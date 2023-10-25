/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */
#include "bt_source_srv.h"
#include "bt_source_srv_hfp.h"
#include "bt_source_srv_utils.h"
#include "bt_source_srv_call_pseduo_dev.h"
#include "bt_source_srv_call_audio.h"
#include "bt_source_srv_hfp_call_manager.h"
#include "bt_source_srv_internal.h"
#include "bt_source_srv_common.h"
#include "bt_source_srv_call.h"
#ifdef MTK_BT_CM_SUPPORT
#include "bt_connection_manager.h"
#endif
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
#include "bt_timer_external.h"
#endif
#include "bt_iot_device_white_list.h"
#include "bt_hsp.h"

typedef struct {
    uint32_t                       handle;
    bt_source_srv_hfp_flag_t       flags;
    bt_hfp_hf_feature_t            support_feature;
    void                           *device;
    bt_source_srv_linknode_t       oom_wl;                   /* out of memory waiting list */
    bt_bd_addr_t                   remote_address;
    bt_hfp_ag_indicators_t         ag_indicators;
    bt_hfp_ag_hold_feature_t       hold_feature;
} bt_source_srv_hfp_context_t;

typedef struct {
    bt_source_srv_hfp_context_t                *highlight_device;
    bt_source_srv_battery_level_t              battery_level;
    bt_source_srv_voice_recognition_status_t   va_status;
    bt_source_srv_hfp_bqb_flag_t               bqb_flag;
    bt_source_srv_hfp_common_flag_t            common_flags;
    uint8_t                                    audio_source_speaker_volume;
} bt_source_srv_hfp_common_context_t;

/* OOM information */
#define BT_SOURCE_SRV_HFP_OOM_WL_ACTION_CMD                0x00
#define BT_SOURCE_SRV_HFP_OOM_WL_ACTION_PHONE_STATUS       0x01
#define BT_SOURCE_SRV_HFP_OOM_WL_ACTION_ESCO               0x02
typedef uint8_t bt_source_srv_hfp_oom_wl_action_t;
typedef struct {
    bt_hfp_ag_indicators_t  indicator_type;
    uint32_t                value;
} bt_source_srv_hfp_phone_status_t;

typedef union {
    uint8_t                                       cmd[BT_SOURCE_SRV_HFP_CMD_LENGTH];
    bt_hfp_audio_direction_t                      audio_direction;
    bt_source_srv_hfp_phone_status_t              phone_status;
} bt_source_srv_hfp_oom_wl_action_parameter_t;

typedef struct {
    bt_source_srv_linknode_t                        node;
    bt_source_srv_hfp_oom_wl_action_t               wl_action;
    bt_source_srv_hfp_oom_wl_action_parameter_t     action_parameter;
} bt_source_srv_hfp_oom_wl_node_t;

static bt_source_srv_hfp_context_t g_source_srv_hfp_context[BT_SOURCE_SRV_HFP_LINK_NUM] = {{0}};

static bt_source_srv_hfp_common_context_t g_hfp_common_context = {0};

static const bt_source_srv_hfp_clcc_call_state_t g_source_srv_hfp_clcc_call_state_map_table[BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_MAX] = {
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_INVAILD,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_INCOMMING,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_DIALING,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_ALERTING,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_ACTIVE,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_LOCAL_HELD,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_WAITING,
};

static const bt_source_srv_call_audio_play_t g_source_srv_play_type_table[] = {
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_CODEC,
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL,
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL,
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL,
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN,
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_I2S_IN
};


static void bt_source_srv_hfp_ring_alerting_start(void);
static bt_status_t bt_source_srv_hfp_send_result_cmd(bt_source_srv_hfp_context_t *context, uint8_t *cmd);
static bt_status_t bt_source_srv_hfp_add_oom_wl(bt_source_srv_hfp_oom_wl_action_t action, bt_source_srv_hfp_context_t *context,
        bt_source_srv_hfp_oom_wl_action_parameter_t *action_param);
static void bt_source_srv_hfp_clear_oom_wl(bt_source_srv_hfp_context_t *context);
static bt_status_t bt_source_srv_hfp_audio_transfer_with_call(bt_source_srv_hfp_context_t *context,
        bt_source_srv_call_state_t new_state, bt_status_t call_transfer_result);
static void bt_source_srv_hfp_sync_volume_with_audio_source(bt_source_srv_hfp_context_t *context, bool is_resume_volume);

/* call action handler function */
typedef bt_status_t (*bt_source_srv_hfp_handle_action_t)(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_new_call(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_call_state_change(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_service_avaliability_change(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_signal_strength_change(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_roaming_status_change(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_battery_level_change(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_voice_recognition_state_change(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_switch_audio_path(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_send_custom_result_code(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_volume_change_by_internal(bt_source_srv_hfp_context_t *context, bt_source_srv_call_audio_volume_t new_volume, bt_source_srv_call_gain_t call_gain_type);

static const bt_source_srv_hfp_handle_action_t g_handle_action_table[] = {
    NULL,
    bt_source_srv_hfp_handle_new_call,
    bt_source_srv_hfp_handle_call_state_change,
    bt_source_srv_hfp_handle_service_avaliability_change,
    bt_source_srv_hfp_handle_signal_strength_change,
    bt_source_srv_hfp_handle_roaming_status_change,
    bt_source_srv_hfp_handle_battery_level_change,
    bt_source_srv_hfp_handle_voice_recognition_state_change,
    bt_source_srv_hfp_handle_switch_audio_path,
    bt_source_srv_hfp_handle_send_custom_result_code,
};

/* common action handler function */
static bt_status_t bt_source_srv_hfp_handle_mute(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_unmute(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_volume_up(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_volume_down(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_hfp_handle_volume_change(void *parameter, uint32_t length);

static const bt_source_srv_hfp_handle_action_t g_handle_common_action_table[] = {
    bt_source_srv_hfp_handle_mute,
    bt_source_srv_hfp_handle_unmute,
    bt_source_srv_hfp_handle_volume_up,
    bt_source_srv_hfp_handle_volume_down,
    bt_source_srv_hfp_handle_volume_change
};

/* call state transfer function */
static bt_status_t bt_source_srv_hfp_get_call_transfer_info_by_idle(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info);
static bt_status_t bt_source_srv_hfp_get_call_transfer_info_by_incomming(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info);
static bt_status_t bt_source_srv_hfp_get_call_transfer_info_by_ougoing(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info);
static bt_status_t bt_source_srv_hfp_get_call_transfer_info_by_active(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info);
static bt_status_t bt_source_srv_hfp_get_call_transfer_info_by_held(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info);
typedef bt_status_t (*bt_source_srv_hfp_call_get_transfer_info_t)(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info);
static const bt_source_srv_hfp_call_get_transfer_info_t g_get_call_transfer_info_handle[] = {
    bt_source_srv_hfp_get_call_transfer_info_by_idle,
    bt_source_srv_hfp_get_call_transfer_info_by_incomming,
    bt_source_srv_hfp_get_call_transfer_info_by_ougoing,
    bt_source_srv_hfp_get_call_transfer_info_by_ougoing,
    bt_source_srv_hfp_get_call_transfer_info_by_active,
    bt_source_srv_hfp_get_call_transfer_info_by_held,
    bt_source_srv_hfp_get_call_transfer_info_by_incomming
};

/* hfp event handler function */
typedef bt_status_t (*bt_source_srv_hfp_common_event_handler_t)(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_connect_request(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_slc_connected(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_disconnected(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_brsf_feature(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_call_hold_feature(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_audio_connected(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_audio_status_change(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_audio_disconnected(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_ag_indicators_enable(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_operator_format(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_query_operator(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_error_code_enable(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_codec_connection(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_answer_call(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_terminate_call(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_dial_number(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_dial_memory(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_dial_last(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_call_waiting_enable(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_caller_id_enable(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_dtmf_code(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_query_subscriber_number(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_query_call_list(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_hold_call(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_sync_volume_speaker_gain(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_volume_sync_mic_gain(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_voice_recognition(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_custom_command_result(bt_msg_type_t msg, bt_status_t status, void *parameter);
static bt_status_t bt_source_srv_hfp_handle_ready_to_send(bt_msg_type_t msg, bt_status_t status, void *parameter);
typedef struct {
    bt_msg_type_t                                 msg;
    bt_source_srv_hfp_common_event_handler_t      handler;
} bt_source_srv_hfp_handle_common_event_t;

static const bt_source_srv_hfp_handle_common_event_t g_handle_common_event_table[] = {
    {BT_HFP_CONNECT_REQUEST_IND,           bt_source_srv_hfp_handle_connect_request},
    {BT_HFP_SLC_CONNECTED_IND,             bt_source_srv_hfp_handle_slc_connected},
    {BT_HFP_DISCONNECT_IND,                bt_source_srv_hfp_handle_disconnected},
    {BT_HFP_BRSF_FEATURES_IND,             bt_source_srv_hfp_handle_brsf_feature},
    {BT_HFP_CALL_HOLD_FEATURES_IND,        bt_source_srv_hfp_handle_call_hold_feature},
    {BT_HFP_AUDIO_CONNECT_IND,             bt_source_srv_hfp_handle_audio_connected},
    {BT_HFP_AUDIO_STATUS_CHANGE_IND,       bt_source_srv_hfp_handle_audio_status_change},
    {BT_HFP_AUDIO_DISCONNECT_IND,          bt_source_srv_hfp_handle_audio_disconnected},
    {BT_HFP_AG_INDICATORS_ENABLE_IND,      bt_source_srv_hfp_handle_ag_indicators_enable},
    {BT_HFP_OPERATOR_FORMAT_IND,           bt_source_srv_hfp_handle_operator_format},
    {BT_HFP_QUERY_OPERATOR_IND,            bt_source_srv_hfp_handle_query_operator},
    {BT_HFP_ERROR_CODE_ENABLE_IND,         bt_source_srv_hfp_handle_error_code_enable},
    {BT_HFP_CODEC_CONNECTION_IND,          bt_source_srv_hfp_handle_codec_connection},
    {BT_HFP_ANSWER_CALL_IND,               bt_source_srv_hfp_handle_answer_call},
    {BT_HFP_TERMINATE_CALL_IND,            bt_source_srv_hfp_handle_terminate_call},
    {BT_HFP_DIAL_NUMBER_IND,               bt_source_srv_hfp_handle_dial_number},
    {BT_HFP_DIAL_MEMORY_IND,               bt_source_srv_hfp_handle_dial_memory},
    {BT_HFP_DIAL_LAST_IND,                 bt_source_srv_hfp_handle_dial_last},
    {BT_HFP_CALL_WAITING_ENABLE_IND,       bt_source_srv_hfp_handle_call_waiting_enable},
    {BT_HFP_CALLER_ID_ENABLE_IND,          bt_source_srv_hfp_handle_caller_id_enable},
    {BT_HFP_DTMF_CODE_IND,                 bt_source_srv_hfp_handle_dtmf_code},
    {BT_HFP_QUERY_SUBSCRIBER_NUMBER_IND,   bt_source_srv_hfp_handle_query_subscriber_number},
    {BT_HFP_QUERY_CALL_LIST_IND,           bt_source_srv_hfp_handle_query_call_list},
    {BT_HFP_HOLD_CALL_IND,                 bt_source_srv_hfp_handle_hold_call},
    {BT_HFP_VOLUME_SYNC_SPEAKER_GAIN_IND,  bt_source_srv_hfp_handle_sync_volume_speaker_gain},
    {BT_HFP_VOLUME_SYNC_MIC_GAIN_IND,      bt_source_srv_hfp_handle_volume_sync_mic_gain},
    {BT_HFP_VOICE_RECOGNITION_IND,         bt_source_srv_hfp_handle_voice_recognition},
    {BT_HFP_CUSTOM_COMMAND_RESULT_IND,     bt_source_srv_hfp_handle_custom_command_result},
    {BT_HFP_READY_TO_SEND_IND,             bt_source_srv_hfp_handle_ready_to_send}
};

#define BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context)            bt_source_srv_hfp_send_result_cmd(context, (uint8_t *)"OK")
#define BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context)         bt_source_srv_hfp_send_result_cmd(context, (uint8_t *)"ERROR")
#define BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd)     bt_source_srv_hfp_send_result_cmd(context, cmd)

static bt_source_srv_hfp_context_t *bt_source_srv_get_free_context(void)
{
    bt_source_srv_hfp_context_t *context = NULL;
    for (uint32_t i = 0; i < BT_SOURCE_SRV_HFP_LINK_NUM; i++) {
        if (g_source_srv_hfp_context[i].handle == BT_HFP_INVALID_HANDLE) {
            context = &g_source_srv_hfp_context[i];
            break;
        }
    }
    LOG_MSGID_I(source_srv, "[HFP][AG] get free context = %02x", 1, context);
    return context;
}

static bt_source_srv_hfp_context_t *bt_source_srv_hfp_find_context_by_handle(uint32_t handle)
{
    bt_source_srv_hfp_context_t *context = NULL;
    for (uint32_t i = 0; i < BT_SOURCE_SRV_HFP_LINK_NUM; i++) {
        if (g_source_srv_hfp_context[i].handle == handle) {
            context = &g_source_srv_hfp_context[i];
            break;
        }
    }
    LOG_MSGID_I(source_srv, "[HFP][AG] find context = %02x by handle = %02x", 2, context, handle);
    return context;
}

static bt_source_srv_hfp_context_t *bt_source_srv_hfp_get_highlight_device(void)
{
    return g_hfp_common_context.highlight_device;
}

static void bt_source_srv_hfp_set_highlight_device(bt_source_srv_hfp_context_t *context)
{
    g_hfp_common_context.highlight_device = context;
}

static bt_source_srv_hfp_context_t *bt_source_srv_hfp_find_context_by_device(void *device)
{
    bt_source_srv_hfp_context_t *context = NULL;
    for (uint32_t i = 0; i < BT_SOURCE_SRV_HFP_LINK_NUM; i++) {
        if ((g_source_srv_hfp_context[i].handle != BT_HFP_INVALID_HANDLE) && (g_source_srv_hfp_context[i].device == device)) {
            context = &g_source_srv_hfp_context[i];
            break;
        }
    }
    LOG_MSGID_I(source_srv, "[HFP][AG] find context = %02x by device = %02x", 2, context, device);
    return context;
}

static void bt_source_srv_hfp_reset_context(bt_source_srv_hfp_context_t *context)
{
    LOG_MSGID_W(source_srv, "[HFP][AG] reset context = %02x", 1, context);
    bt_source_srv_call_psd_free_device(context->device);
    bt_source_srv_hfp_clear_oom_wl(context);
    bt_source_srv_memset(context, 0, sizeof(bt_source_srv_hfp_context_t));
    g_hfp_common_context.common_flags &= ~BT_SOURCE_SRV_HFP_FLAG_SLC_GET_CALL_INFO_COMPLETE;
}

static bt_source_srv_hfp_context_t *bt_source_srv_hfp_find_context_by_address(bt_bd_addr_t *address)
{
    bt_source_srv_hfp_context_t *context = NULL;
    for (uint32_t i = 0; i < BT_SOURCE_SRV_HFP_LINK_NUM; i++) {
        if (bt_source_srv_memcmp(&g_source_srv_hfp_context[i].remote_address, address, sizeof(bt_bd_addr_t)) == 0) {
            context = &g_source_srv_hfp_context[i];
            break;
        }
    }
    LOG_MSGID_I(source_srv, "[HFP][AG] find context = %02x by address:%02x-%02x-%02x-%02x-%02x-%02x", 7, context, (uint8_t *)address[0],
                (uint8_t *)address[2], (uint8_t *)address[3], (uint8_t *)address[4], (uint8_t *)address[5], (uint8_t *)address[6]);
    return context;
}

static bt_status_t bt_source_srv_hfp_transfer_phone_status(bt_hfp_ag_indicators_t indicator_type, uint32_t value)
{
    bt_status_t status = BT_STATUS_FAIL;
    for (uint32_t i = 0; i < BT_SOURCE_SRV_HFP_LINK_NUM; i++) {
        bt_source_srv_hfp_context_t *context = &g_source_srv_hfp_context[i];
        if ((context->handle == BT_HFP_INVALID_HANDLE) || (!bt_source_srv_call_psd_is_ready((void *)context->device))) {
            continue;
        }

        /**
         * type:source issue
         * root cause:phone status send to sink after deactive individual indicators.
         * solution:ingnore indicator for deactive status.
         */
        if ((!BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_CALL_AG_INDICATOR_ENABLED)) ||
                ((!(indicator_type & (BT_HFP_AG_INDICATORS_CALL | BT_HFP_AG_INDICATORS_CALL_SETUP | BT_HFP_AG_INDICATORS_CALL_HELD))) &&
                 (!(context->ag_indicators & indicator_type)))) {
            LOG_MSGID_E(source_srv, "[HFP][AG] context = %02x indicator status = %02x transfer fail by type = %02x", 3, context, context->ag_indicators, indicator_type);
            continue;
        }

        status = bt_hfp_send_status_indication(context->handle, indicator_type, value);
        LOG_MSGID_I(source_srv, "[HFP][AG] phone status transfer context = %02x status = %02x", 2, context, status);
        if (status == BT_STATUS_OUT_OF_MEMORY) {
            bt_source_srv_hfp_oom_wl_action_parameter_t action_parameter = {
                .phone_status.indicator_type = indicator_type,
                .phone_status.value = value
            };
            bt_source_srv_hfp_add_oom_wl(BT_SOURCE_SRV_HFP_OOM_WL_ACTION_PHONE_STATUS, context, &action_parameter);
        }
        if ((status != BT_STATUS_SUCCESS) && (status != BT_STATUS_OUT_OF_MEMORY)) {
            LOG_MSGID_E(source_srv, "[HFP][AG] phone status transfer context = %02x, indicator = %02x, value = %02x, status = %02x", 4,
                        context, indicator_type, value, status);
            break;
        }
    }
    return status;
}

static bt_status_t bt_source_srv_hfp_audio_transfer(bt_source_srv_hfp_context_t *context, bt_hfp_audio_direction_t audio_direction)
{
    bt_status_t status = BT_STATUS_FAIL;
    status = bt_hfp_audio_transfer(context->handle, audio_direction);
    LOG_MSGID_I(source_srv, "[HFP][AG] audio transfer context = %02x, direction = %02x, status = %02x", 3, context, audio_direction, status);
    if ((status == BT_STATUS_SUCCESS) && (audio_direction == BT_HFP_AUDIO_TO_HF)) {
        BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING);
    }
    return status;
}


static bt_status_t bt_source_srv_hfp_add_oom_wl(bt_source_srv_hfp_oom_wl_action_t action, bt_source_srv_hfp_context_t *context,
        bt_source_srv_hfp_oom_wl_action_parameter_t *action_param)
{
    bt_source_srv_hfp_oom_wl_node_t *cmd_node = (bt_source_srv_hfp_oom_wl_node_t *)bt_source_srv_memory_alloc(sizeof(bt_source_srv_hfp_oom_wl_node_t));
    if (cmd_node == NULL) {
        LOG_MSGID_E(source_srv, "[HFP][AG] add oom wl allocate fail", 0);
        return BT_STATUS_FAIL;
    }

    cmd_node->wl_action = action;
    LOG_MSGID_I(source_srv, "[HFP][AG] add oom wl context = %02x, action = %02x cmd node = %02x", 3, context, action, cmd_node);

    /* if wl esco action exist, will ingore new esco action, avoid repeated esco connection */
    bt_source_srv_hfp_oom_wl_node_t *wl_node = (bt_source_srv_hfp_oom_wl_node_t *)context->oom_wl.front;
    while (wl_node != NULL) {
        if ((wl_node->wl_action == BT_SOURCE_SRV_HFP_OOM_WL_ACTION_ESCO) && (action == BT_SOURCE_SRV_HFP_OOM_WL_ACTION_ESCO)) {
            LOG_MSGID_W(source_srv, "[HFP][AG] add oom wl action = %02x had exist", 1, action);
            return BT_STATUS_SUCCESS;
        }
        wl_node = (bt_source_srv_hfp_oom_wl_node_t *)wl_node->node.front;
    }

    bt_source_srv_memcpy(&cmd_node->action_parameter, action_param, sizeof(bt_source_srv_hfp_oom_wl_action_parameter_t));

    bt_source_srv_linknode_insert_node(&context->oom_wl, (bt_source_srv_linknode_t *)cmd_node, BT_SOURCE_SRV_NODE_BACK);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_run_oom_wl(bt_source_srv_hfp_context_t *context)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_source_srv_hfp_oom_wl_node_t *wl_node = (bt_source_srv_hfp_oom_wl_node_t *)context->oom_wl.front;
    bt_source_srv_hfp_oom_wl_node_t *free_wl_node = NULL;
    while (wl_node != NULL) {
        LOG_MSGID_I(source_srv, "[HFP][AG] run oom wl context = %02x, action = %02x cmd node = %02x", 3, context, wl_node->wl_action, wl_node);
        switch (wl_node->wl_action) {
            case BT_SOURCE_SRV_HFP_OOM_WL_ACTION_CMD: {
                status = bt_hfp_send_command(context->handle, (uint8_t *)wl_node->action_parameter.cmd, (uint16_t)bt_source_srv_strlen((char *)wl_node->action_parameter.cmd));
            }
            break;
            case BT_SOURCE_SRV_HFP_OOM_WL_ACTION_PHONE_STATUS: {
                bt_source_srv_hfp_phone_status_t *phone_status = (bt_source_srv_hfp_phone_status_t *)&wl_node->action_parameter.phone_status;
                status = bt_hfp_send_status_indication(context->handle, phone_status->indicator_type, phone_status->value);
            }
            break;
            case BT_SOURCE_SRV_HFP_OOM_WL_ACTION_ESCO: {
                status = bt_source_srv_hfp_audio_transfer(context, wl_node->action_parameter.audio_direction);
            }
            break;
            default:
                break;
        }
        if (status != BT_STATUS_SUCCESS) {
            LOG_MSGID_I(source_srv, "[HFP][AG] run oom wl status = %02x", 1, status);
            return BT_STATUS_FAIL;
        }
        free_wl_node = wl_node;
        wl_node = (bt_source_srv_hfp_oom_wl_node_t *)wl_node->node.front;
        bt_source_srv_linknode_remove_node(&context->oom_wl, (bt_source_srv_linknode_t *)free_wl_node);
        bt_source_srv_memory_free((void *)free_wl_node);
    }
    return status;
}

static void bt_source_srv_hfp_clear_oom_wl(bt_source_srv_hfp_context_t *context)
{
    bt_source_srv_hfp_oom_wl_node_t *wl_node = (bt_source_srv_hfp_oom_wl_node_t *)context->oom_wl.front;
    while (wl_node != NULL) {
        bt_source_srv_linknode_remove_node(&context->oom_wl, (bt_source_srv_linknode_t *)wl_node);
        bt_source_srv_memory_free((void *)wl_node);
        wl_node = (bt_source_srv_hfp_oom_wl_node_t *)wl_node->node.front;
    }
}

static bool bt_source_srv_hfp_is_wl_node_exist(bt_source_srv_hfp_context_t *context, bt_source_srv_hfp_oom_wl_action_t action)
{
    bt_source_srv_hfp_oom_wl_node_t *wl_node = (bt_source_srv_hfp_oom_wl_node_t *)context->oom_wl.front;
    while (wl_node != NULL) {
        if (wl_node->wl_action == action) {
            return true;
        }
        wl_node = (bt_source_srv_hfp_oom_wl_node_t *)wl_node->node.front;
    }
    return false;
}

static bt_status_t bt_source_srv_hfp_handle_result_cmd_oom(bt_source_srv_hfp_context_t *context, uint8_t *cmd)
{
    if (bt_source_srv_strlen((char *)cmd) > BT_SOURCE_SRV_HFP_CMD_LENGTH) {
        return BT_STATUS_OUT_OF_MEMORY;
    }

    bt_source_srv_hfp_oom_wl_action_parameter_t action_parameter = {0};
    bt_source_srv_memcpy(action_parameter.cmd, cmd, bt_source_srv_strlen((char *)cmd));
    /* out of memory */
    bt_source_srv_hfp_add_oom_wl(BT_SOURCE_SRV_HFP_OOM_WL_ACTION_CMD, context, &action_parameter);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_send_result_cmd(bt_source_srv_hfp_context_t *context, uint8_t *cmd)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (bt_source_srv_hfp_is_wl_node_exist(context, BT_SOURCE_SRV_HFP_OOM_WL_ACTION_CMD)) {
        LOG_MSGID_W(source_srv, "[HFP][AG] send result cmd wl mode exist, need arrange", 0);
        return bt_source_srv_hfp_handle_result_cmd_oom(context, cmd);
    }

    status = bt_hfp_send_command(context->handle, cmd, (uint16_t)bt_source_srv_strlen((char *)cmd));
    if (status == BT_STATUS_OUT_OF_MEMORY) {
        return bt_source_srv_hfp_handle_result_cmd_oom(context, cmd);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_handle_new_call(void *parameter, uint32_t length)
{
    bt_source_srv_new_call_t *new_call = (bt_source_srv_new_call_t *)parameter;
    bt_source_srv_call_index_t call_index = bt_source_srv_hfp_create_new_call(&new_call->hfp_new_call);
    /* notify user call index */
    bt_source_srv_call_index_ind_t call_index_ind = {
        .type = BT_SOURCE_SRV_TYPE_HFP,
        .index = call_index
    };

    /**
     * type:BQB workaround
     * root cause:handle the HFP/AG/TCA/BV-04-I case, use AT cmd to make a virtual call, so there is no need to report the call index.
     * solution:there is no need to report the call index.
     */
    if (bt_source_srv_hfp_get_bqb_flag() & BT_SOURCE_SRV_HFP_BQB_FLAG_VIRTUAL_CALL) {
        LOG_MSGID_W(source_srv, "[HFP][AG] virtual new call", 0);
        return BT_STATUS_SUCCESS;
    }
    bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_CALL_INDEX_IND, &call_index_ind, sizeof(bt_source_srv_call_index_ind_t));
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_handle_call_state_change(void *parameter, uint32_t length)
{
    bt_source_srv_call_state_change_t *call_state_change = (bt_source_srv_call_state_change_t *)parameter;
    return bt_source_srv_hfp_call_state_change(&call_state_change->hfp_call_change);
}

static bt_status_t bt_source_srv_hfp_handle_service_avaliability_change(void *parameter, uint32_t length)
{
    bt_source_srv_service_availability_change_t *service_availability = (bt_source_srv_service_availability_change_t *)parameter;
    return bt_source_srv_hfp_transfer_phone_status(BT_HFP_AG_INDICATORS_SERVICE, service_availability->service_ability);
}

static bt_status_t bt_source_srv_hfp_handle_signal_strength_change(void *parameter, uint32_t length)
{
    bt_source_srv_signal_strength_change_t *signal_strength = (bt_source_srv_signal_strength_change_t *)parameter;
    return bt_source_srv_hfp_transfer_phone_status(BT_HFP_AG_INDICATORS_SIGNAL, signal_strength->signal_strength);
}

static bt_status_t bt_source_srv_hfp_handle_roaming_status_change(void *parameter, uint32_t length)
{
    bt_source_srv_roaming_status_change_t *roaming_status = (bt_source_srv_roaming_status_change_t *)parameter;
    return bt_source_srv_hfp_transfer_phone_status(BT_HFP_AG_INDICATORS_ROAM, roaming_status->roaming_state);
}

static bt_status_t bt_source_srv_hfp_handle_battery_level_change(void *parameter, uint32_t length)
{
    bt_source_srv_battery_level_change_t *battery = (bt_source_srv_battery_level_change_t *)parameter;
    return bt_source_srv_hfp_transfer_phone_status(BT_HFP_AG_INDICATORS_BATTERY, battery->battery_level);
}

static bt_status_t bt_source_srv_hfp_handle_voice_recognition_state_change(void *parameter, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_source_srv_voice_recognition_state_change_t *va_change = (bt_source_srv_voice_recognition_state_change_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_get_highlight_device();
    LOG_MSGID_I(source_srv, "[HFP][AG] va state change status = %02x", 1, va_change->status);
    g_hfp_common_context.va_status = va_change->status;

    if (context == NULL) {
        LOG_MSGID_W(source_srv, "[HFP][AG] va state change highlight is NULL", 0);
        return BT_STATUS_SUCCESS;
    }

    bt_source_srv_hfp_feature_config_t hfp_feature_config = {0};
    if (bt_source_srv_get_feature_config(BT_SOURCE_SRV_TYPE_HFP, &hfp_feature_config) != BT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[HFP][AG] va get feature config fail", 0);
        return BT_STATUS_FAIL;
    }

    if (!((hfp_feature_config.feature & BT_SOURCE_SRV_HFP_FEATURE_VOICE_RECOGNITION) && (context->support_feature & BT_HFP_HF_FEATURE_VOICE_RECOGNITION))) {
        LOG_MSGID_E(source_srv, "[HFP][AG] va not support local feature = %02x remote feature = %02x ", 2,
                    hfp_feature_config.feature, context->support_feature);
        return BT_STATUS_FAIL;
    }

    bt_hfp_audio_direction_t audio_direction = BT_HFP_AUDIO_TO_HF;
    if ((va_change->status == BT_SOURCE_SRV_VOICE_RECOGNITION_STATUS_DISABLE) && (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED))) {
        audio_direction = BT_HFP_AUDIO_TO_AG;
    }

    uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = {0};
    snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+BVRA:%d", va_change->status);
    status = BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
    if (status == BT_STATUS_OUT_OF_MEMORY) {
        bt_source_srv_hfp_oom_wl_action_parameter_t action_parameter = {
            .audio_direction = audio_direction
        };
        bt_source_srv_hfp_add_oom_wl(BT_SOURCE_SRV_HFP_OOM_WL_ACTION_ESCO, context, &action_parameter);
        return BT_STATUS_SUCCESS;
    }

    if (status != BT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[HFP][AG] va state change send cmd fail status = %02x", 1, status);
        return status;
    }

    status = bt_source_srv_hfp_audio_transfer(context, audio_direction);

    LOG_MSGID_I(source_srv, "[HFP][AG] va state change create sco  status = %02x", 1, status);
    return status;
}

static bt_status_t bt_source_srv_hfp_handle_switch_audio_path(void *parameter, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_get_highlight_device();
    bt_source_srv_switch_audio_path_t *audio_path = (bt_source_srv_switch_audio_path_t *)parameter;
    if (context == NULL) {
        LOG_MSGID_W(source_srv, "[HFP][AG] switch audio path highlight is NULL", 0);
        if ((NULL != audio_path) && (BT_SOURCE_SRV_AUDIO_TRANSFER_TO_HF == audio_path->audio_transfer)) {
            g_hfp_common_context.common_flags |= BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECT_WITH_SLC;
        } else if ((NULL != audio_path) && (BT_SOURCE_SRV_AUDIO_TRANSFER_TO_AG == audio_path->audio_transfer)) {
            g_hfp_common_context.common_flags &= ~BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECT_WITH_SLC;
        }
        return BT_STATUS_FAIL;
    }

    if (NULL == parameter) {
        if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED)) {
            return bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_AG);
        } else if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING)) {
            return BT_STATUS_SUCCESS;
        }
        return bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_HF);
    }

    if ((BT_SOURCE_SRV_AUDIO_TRANSFER_TO_HF == audio_path->audio_transfer) &&
            (!BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED | BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING))) {
        status = bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_HF);
    } else if ((BT_SOURCE_SRV_AUDIO_TRANSFER_TO_AG == audio_path->audio_transfer) &&
               (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED))) {
        status = bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_AG);
    } else {
        LOG_MSGID_W(source_srv, "[HFP][AG] switch audio path context flag = %02x direction = %02x", 2, context->flags, audio_path->audio_transfer);
    }
    return status;
}

static bt_status_t bt_source_srv_hfp_handle_send_custom_result_code(void *parameter, uint32_t length)
{
    bt_source_srv_send_custom_result_code_t *cuntom_result_code = (bt_source_srv_send_custom_result_code_t *)parameter;

    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_get_highlight_device();
    if (context == NULL) {
        LOG_MSGID_W(source_srv, "[HFP][AG] send custom result code highlight is NULL", 0);
        return BT_STATUS_FAIL;
    }

    return BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, (uint8_t *)cuntom_result_code->result_code);
}

static bt_status_t bt_source_srv_hfp_handle_mute(void *parameter, uint32_t length)
{
    bt_source_srv_audio_mute_t *mute = (bt_source_srv_audio_mute_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_get_highlight_device();
    if (context == NULL) {
        LOG_MSGID_W(source_srv, "[HFP][AG] audio mute highlight is NULL", 0);
        return BT_STATUS_SUCCESS;
    }

    if (mute->port == BT_SOURCE_SRV_PORT_MIC) {
        /* sync mic gain to headset */
        uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = {0};
        snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+VGM:0");
        BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_MIC_MUTED);
        return BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
    }
    uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = {0};
    snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+VGS:0");
    bt_source_srv_call_psd_audio_mute(context->device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL);
    BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SPEAKER_MUTED);
    return BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
}

static bt_status_t bt_source_srv_hfp_handle_unmute(void *parameter, uint32_t length)
{
    bt_source_srv_audio_mute_t *unmute = (bt_source_srv_audio_mute_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_get_highlight_device();
    if (context == NULL) {
        LOG_MSGID_W(source_srv, "[HFP][AG] audio unmute highlight is NULL", 0);
        return BT_STATUS_SUCCESS;
    }

    if (unmute->port == BT_SOURCE_SRV_PORT_MIC) {
        bt_source_srv_call_audio_volume_t volume = bt_source_srv_call_get_audio_gain_level(BT_SOURCE_SRV_CALL_GAIN_MIC, &context->remote_address);
        /* sync mic gain to headset */
        uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = {0};
        snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+VGM:%d", volume);
        BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_MIC_MUTED);
        return BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
    }
    bt_source_srv_call_audio_volume_t volume = bt_source_srv_call_get_audio_gain_level(BT_SOURCE_SRV_CALL_GAIN_SPEAKER, &context->remote_address);
    /* sync mic gain to headset */
    uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = {0};
    snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+VGS:%d", volume);
    BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SPEAKER_MUTED);
    bt_source_srv_call_psd_audio_unmute(context->device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL);
    return BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
}

static bt_status_t bt_source_srv_hfp_handle_volume_up(void *parameter, uint32_t length)
{
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_handle_volume_down(void *parameter, uint32_t length)
{
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_handle_volume_change(void *parameter, uint32_t length)
{
    bt_source_srv_volume_change_t *volume_change  = (bt_source_srv_volume_change_t *)parameter;
    bt_source_srv_call_audio_volume_t old_volume = 0;
    bt_source_srv_call_audio_volume_t new_volume = 0;

    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_get_highlight_device();
    if (context == NULL) {
        if (BT_SOURCE_SRV_PORT_CHAT_SPEAKER == volume_change->port) {
            g_hfp_common_context.audio_source_speaker_volume = volume_change->volume_value;
        }
        LOG_MSGID_W(source_srv, "[HFP][AG] volume change highlight is NULL", 0);
        return BT_STATUS_FAIL;
    }

    if (volume_change->port == BT_SOURCE_SRV_PORT_MIC) {
        old_volume = bt_source_srv_call_get_audio_gain_level(BT_SOURCE_SRV_CALL_GAIN_MIC, &context->remote_address);
    } else {
        old_volume = bt_source_srv_call_get_audio_gain_level(BT_SOURCE_SRV_CALL_GAIN_SPEAKER, &context->remote_address);
    }

    /* action volume value is 0~100, call volume is 0~15 */
    new_volume = (bt_source_srv_call_audio_volume_t)bt_source_srv_call_convert_volume(BT_SOURCE_SRV_CALL_VOLUME_SCALE_15, volume_change->volume_value, true);
    LOG_MSGID_I(source_srv, "[HFP][AG] volume change port = %02x, volume absolute value = %02x, old volume = %02x, new volume = %02x", 4,
                volume_change->port, volume_change->volume_value, old_volume, new_volume);
    /* volume is not change */
    if (old_volume == new_volume) {
        return BT_STATUS_SUCCESS;
    }
    return bt_source_srv_hfp_handle_volume_change_by_internal(context, new_volume, (volume_change->port == BT_SOURCE_SRV_PORT_MIC) ? BT_SOURCE_SRV_CALL_GAIN_MIC : BT_SOURCE_SRV_CALL_GAIN_SPEAKER);
}


static bt_status_t bt_source_srv_hfp_handle_volume_change_by_internal(bt_source_srv_hfp_context_t *context, bt_source_srv_call_audio_volume_t new_volume, bt_source_srv_call_gain_t call_gain_type)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = {0};

    switch (call_gain_type) {
        case BT_SOURCE_SRV_CALL_GAIN_SPEAKER: {
            /* send speaker volume to remote */
            snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+VGS:%d", new_volume);
        }
        break;
        case BT_SOURCE_SRV_CALL_GAIN_MIC: {
            snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+VGM:%d", new_volume);
        }
        break;
        default:
            break;
    }

    if ((!bt_source_srv_call_psd_is_playing(context->device)) && (BT_SOURCE_SRV_CALL_GAIN_SPEAKER == call_gain_type)) {
        LOG_MSGID_W(source_srv, "[HFP][AG] volume change = %02x when is not playing", 1, new_volume);
        g_hfp_common_context.audio_source_speaker_volume = new_volume;
        bt_source_srv_call_audio_gain_update(BT_SOURCE_SRV_CALL_GAIN_AUDIO_SOURCE_SPEAKER, &context->remote_address, context->device, new_volume);
        return status;
    }

    status = BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
    if ((status == BT_STATUS_SUCCESS) || (status == BT_STATUS_OUT_OF_MEMORY)) {
        bt_source_srv_call_audio_gain_update(call_gain_type, &context->remote_address, context->device, new_volume);
    }
    LOG_MSGID_I(source_srv, "[HFP][AG] volume change type = %02x send status = %02x", 2, call_gain_type, status);
    return status;
}

static bt_status_t bt_source_srv_hfp_get_call_transfer_info_by_idle(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info)
{
    switch (new_state) {
        case BT_SOURCE_SRV_CALL_STATE_INCOMING:
        case BT_SOURCE_SRV_CALL_STATE_WAITING: {
            transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALLSETUP;
            transfer_info->callsetup_status = BT_HFP_CIEV_CALL_SETUP_STATE_INCOMING;
        }
        break;
        case BT_SOURCE_SRV_CALL_STATE_DIALING:
        case BT_SOURCE_SRV_CALL_STATE_ALERTING: {
            transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALLSETUP;
            transfer_info->callsetup_status = (new_state == BT_SOURCE_SRV_CALL_STATE_DIALING) ? BT_HFP_CIEV_CALL_SETUP_STATE_OUTGOING : BT_HFP_CIEV_CALL_SETUP_STATE_REMOTE_ALERT;
        }
        break;
        case BT_SOURCE_SRV_CALL_STATE_ACTIVE: {
            transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALL;
            transfer_info->call_status = BT_HFP_CIEV_CALL_SETUP_STATE_INCOMING;
        }
        break;
        case BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD: {
            transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD;
            transfer_info->held_status = BT_HFP_CIEV_CALL_HOLD_STATE_ALL;
            if (bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_ACTIVE)) {
                transfer_info->held_status = BT_HFP_CIEV_CALL_HOLD_STATE_SOME;
            }
        }
        break;
        default:
            return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_get_call_transfer_info_by_incomming(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info)
{
    switch (new_state) {
        case BT_SOURCE_SRV_CALL_STATE_NONE: {
            transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALLSETUP;
            transfer_info->callsetup_status = BT_HFP_CIEV_CALL_SETUP_STATE_NONE;
        }
        break;
        case BT_SOURCE_SRV_CALL_STATE_ACTIVE: {
            transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALL | BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALLSETUP;
            transfer_info->callsetup_status = BT_HFP_CIEV_CALL_SETUP_STATE_NONE;
            transfer_info->call_status = BT_HFP_CIEV_CALL_STATE_CALL;
            if (bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD)) {
                transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD_ACTIVE_SWAPPED;
                transfer_info->held_status = BT_HFP_CIEV_CALL_HOLD_STATE_SOME;
            }
        }
        break;
        default:
            return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_get_call_transfer_info_by_ougoing(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info)
{
    switch (new_state) {
        case BT_SOURCE_SRV_CALL_STATE_NONE: {
            transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALLSETUP;
            transfer_info->callsetup_status = BT_HFP_CIEV_CALL_SETUP_STATE_NONE;
        }
        break;
        case BT_SOURCE_SRV_CALL_STATE_ACTIVE: {
            transfer_info->call_transfer |=  BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALLSETUP;
            transfer_info->callsetup_status = BT_HFP_CIEV_CALL_SETUP_STATE_NONE;

            if (bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD)) {
                transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD_ACTIVE_SWAPPED;
                transfer_info->held_status = BT_HFP_CIEV_CALL_HOLD_STATE_SOME;
            } else {
                transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALL;
                transfer_info->call_status = BT_HFP_CIEV_CALL_STATE_CALL;
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_STATE_ALERTING: {
            transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALLSETUP;
            transfer_info->callsetup_status = BT_HFP_CIEV_CALL_SETUP_STATE_REMOTE_ALERT;
        }
        break;
        default:
            return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_get_call_transfer_info_by_active(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info)
{
    switch (new_state) {
        case BT_SOURCE_SRV_CALL_STATE_NONE: {
            if (bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD)) {
                transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD;
                transfer_info->held_status = BT_HFP_CIEV_CALL_HOLD_STATE_ALL;
            } else {
                transfer_info->call_transfer |=  BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALL;
                transfer_info->call_status = BT_HFP_CIEV_CALL_STATE_NONE;
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD: {
            transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD;
            if (bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_ACTIVE)) {
                transfer_info->held_status = BT_HFP_CIEV_CALL_HOLD_STATE_SOME;
            } else {
                transfer_info->held_status = BT_HFP_CIEV_CALL_HOLD_STATE_ALL;
            }

            /* active call + held call, active call->held call when receive AT+CHLD=2 */
            bt_source_srv_hfp_call_context_t call_context[2] = {0};
            if (bt_source_srv_hfp_call_get_context_by_state(BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD, call_context, 2) > 1) {
                transfer_info->call_transfer = BT_SOURCE_SRV_HFP_CALL_TRANSFER_NONE;
            }
        }
        break;
        default:
            return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_get_call_transfer_info_by_held(bt_source_srv_call_state_t new_state, bt_source_srv_hfp_call_transfer_info_t *transfer_info)
{
    switch (new_state) {
        case BT_SOURCE_SRV_CALL_STATE_NONE: {
            transfer_info->call_transfer |=  BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALL | BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD;
            transfer_info->held_status = BT_HFP_CIEV_CALL_HOLD_STATE_NONE;
            transfer_info->call_status = BT_HFP_CIEV_CALL_STATE_NONE;
            if (bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_ACTIVE)) {
                transfer_info->call_status = BT_HFP_CIEV_CALL_STATE_CALL;
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_STATE_ACTIVE: {
            transfer_info->call_transfer |= BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD;
            transfer_info->call_status = BT_HFP_CIEV_CALL_STATE_CALL;
            if (bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD)) {
                transfer_info->held_status = BT_HFP_CIEV_CALL_HOLD_STATE_SOME;
                transfer_info->call_transfer &= ~BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALL;
            } else {
                transfer_info->held_status = BT_HFP_CIEV_CALL_HOLD_STATE_NONE;
            }
        }
        break;
        default:
            return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_set_audio_status(uint32_t handle, bt_hfp_audio_status_t status)
{
    return bt_hfp_set_audio_status(handle, status);
}

static bt_status_t bt_source_srv_hfp_psd_callback(void *device, bt_source_srv_call_psd_user_event_t event_id, void *parameter)
{
    LOG_MSGID_I(source_srv, "[HFP][AG] psd callback event = %02x, device = %02x", 2, event_id, device);
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_device(device);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }
    switch (event_id) {
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_DEINIT: {
            /* reset context */
            bt_source_srv_hfp_reset_context(context);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_ACTIVATE_SCO: {
            bt_source_srv_hfp_set_audio_status(context->handle, BT_HFP_AUDIO_STATUS_ACTIVE);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_DEACTIVATE_SCO: {
            bt_source_srv_hfp_set_audio_status(context->handle, BT_HFP_AUDIO_STATUS_INACTIVE);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_SUSPEND: {
            bt_source_srv_call_psd_event_notify(device, BT_SOURCE_SRV_CALL_PSD_EVENT_SUSPEND_REQ, NULL);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_MIC_LOCATION: {
            bt_source_srv_call_psd_mic_location_t *mic_location = (bt_source_srv_call_psd_mic_location_t *)parameter;
            bt_source_srv_hfp_feature_config_t hfp_feature_config = {0};
            if (bt_source_srv_get_feature_config(BT_SOURCE_SRV_TYPE_HFP, &hfp_feature_config) != BT_STATUS_SUCCESS) {
                LOG_MSGID_E(source_srv, "[HFP][AG] get feature config fail", 0);
                return BT_STATUS_FAIL;
            }

            if (hfp_feature_config.custom_feature & BT_SOURCE_SRV_HFP_CUSTOM_FEATURE_LOCAL_MIC) {
                mic_location->location = BT_SOURCE_SRV_CALL_PSD_LOCATION_LOCAL;
            } else {
                mic_location->location = BT_SOURCE_SRV_CALL_PSD_LOCATION_REMOTE;
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_REMOTE_ADDRESS: {
            bt_source_srv_call_psd_remote_address_t *p_remote_address = (bt_source_srv_call_psd_remote_address_t *)parameter;
            bt_source_srv_memcpy(&p_remote_address->remote_address, &context->remote_address, sizeof(bt_bd_addr_t));
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_ESCO_STATE: {
            bt_source_srv_psd_get_esco_state_t *p_esco_state = (bt_source_srv_psd_get_esco_state_t *)parameter;
            p_esco_state->state = BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED) ? \
                                  BT_SOURCE_SRV_CALL_PSD_ESCO_CONNECTED : BT_SOURCE_SRV_CALL_PSD_ESCO_DISCONNECTED;

        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_CALL_STATE: {
            bt_source_srv_psd_get_call_state_t *p_call_state = (bt_source_srv_psd_get_call_state_t *)parameter;
            p_call_state->state = (bt_source_srv_hfp_call_is_exist()) ? BT_SOURCE_SRV_CALL_PSD_CALL_EXISTENCE : BT_SOURCE_SRV_CALL_PSD_CALL_NON_EXISTENCE;
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_SLIENCE_DETECTION_SUSPEND: {
            //bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_AUDIO_DATA_DETECTION, NULL, 0);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_AUDIO_PLAY_COMPLETE: {
            /* sync esco volume to pc */
            bt_source_srv_hfp_sync_volume_with_audio_source(context, false);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_USER_EVENT_AUDIO_STOP_COMPLETE: {
            /* resume PC volume */
            bt_source_srv_hfp_sync_volume_with_audio_source(context, true);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

#ifdef MTK_BT_CM_SUPPORT
static void bt_source_srv_hfp_disconnect(bt_source_srv_hfp_context_t *context)
{
    bt_status_t status = bt_hfp_disconnect(context->handle);
    if ((status == BT_STATUS_SUCCESS) && (context->handle != BT_HFP_INVALID_HANDLE)) {
        if (bt_source_srv_call_psd_is_connecting(context->device)) {
            bt_bd_addr_t remote_address = {0};
            bt_source_srv_memcpy(remote_address, context->remote_address, sizeof(bt_bd_addr_t));
            bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED, NULL);
            bt_cm_profile_service_status_notify(BT_CM_PROFILE_SERVICE_HFP_AG, remote_address, BT_CM_PROFILE_SERVICE_STATE_DISCONNECTED, BT_STATUS_SUCCESS);
        } else {
            bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECT_REQ, NULL);
        }
    } else {
        LOG_MSGID_E(source_srv, "[HFP][AG] disconnect status = %02x", 1, status);
    }
}

static bt_status_t bt_source_srv_hfp_cm_callback(bt_cm_profile_service_handle_t type, void *data)
{
    bt_status_t status = BT_STATUS_FAIL;
    LOG_MSGID_I(source_srv, "[HFP][AG] cm callback event = %02x", 1, type);
    switch (type) {
        case BT_CM_PROFILE_SERVICE_HANDLE_POWER_ON: {
            /* init context */
            bt_source_srv_memset(g_source_srv_hfp_context, 0, sizeof(bt_source_srv_hfp_context_t) * BT_SOURCE_SRV_HFP_LINK_NUM);
            bt_source_srv_call_psd_init();
            bt_source_srv_hfp_set_highlight_device(NULL);
        }
        break;
        case BT_CM_PROFILE_SERVICE_HANDLE_POWER_OFF: {
            bt_source_srv_call_psd_deinit();
            bt_source_srv_hfp_set_highlight_device(NULL);
        }
        break;
        case BT_CM_PROFILE_SERVICE_HANDLE_CONNECT: {
            bt_bd_addr_t *p_bd_address = (bt_bd_addr_t *)data;
            bt_source_srv_hfp_context_t *context = bt_source_srv_get_free_context();
            if (context != NULL) {
                status = bt_hfp_connect_with_role(&context->handle, p_bd_address, BT_HFP_ROLE_HF);
                if (status == BT_STATUS_SUCCESS) {
                    bt_source_srv_memcpy(&context->remote_address, p_bd_address, sizeof(bt_bd_addr_t));
                    context->device = bt_source_srv_call_psd_alloc_device(&context->remote_address, bt_source_srv_hfp_psd_callback);
                    bt_source_srv_assert(context->device && "alloc pseduo device is NULL");
                    /* connect HF device */
                    bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECT_REQ, NULL);
                } else {
                    LOG_MSGID_E(source_srv, "[HFP][AG] connect fail status = %02x", 1, status);
                    bt_source_srv_memset(context, 0, sizeof(bt_source_srv_hfp_context_t));
                }
            }
        }
        break;
        case BT_CM_PROFILE_SERVICE_HANDLE_DISCONNECT: {
            bt_bd_addr_t *p_bd_address = (bt_bd_addr_t *)data;
            bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_address(p_bd_address);
            if (context != NULL) {
                if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED)) {
                    /* sco is connected, first disconenct sco, then disconnect profile */
                    status = bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_AG);
                    if (status == BT_STATUS_SUCCESS) {
                        BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_PROFILE_DISCONNECT);
                    }
                } else {
                    bt_source_srv_hfp_disconnect(context);
                }
            }
        }
        break;
        default:
            break;
    }
    return status;
}
#endif

static bt_status_t bt_source_srv_hfp_send_ring_alerting(bt_source_srv_hfp_call_context_t *call_context)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_get_highlight_device();
    /* ring notify */
    uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = "RING";
    status = BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
    if (status != BT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[HFP][AG] ring notify send fail status = %02x, handle = %02x", 2, status, context->handle);
    }

    /* calling line identification notify */
    if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_CLI_ENABLED)) {
        bt_source_srv_memset(cmd_buffer, 0, BT_SOURCE_SRV_HFP_CMD_LENGTH);
        uint8_t phone_number_type = (call_context->iac == BT_SOURCE_SRV_HFP_CALL_IAC_WITHOUT) ? BT_SOURCE_SRV_HFP_PHONE_NUMBER_NATIONAL : BT_SOURCE_SRV_HFP_PHONE_NUMBER_INTERNATIONAL;
        snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+CLIP: \"%s\",%d", \
                 call_context->phone_number, phone_number_type);
        status = BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
        if (status != BT_STATUS_SUCCESS) {
            LOG_MSGID_E(source_srv, "[HFP][AG] calling line identification notify send fail status = %02x", 1, status);
        }
    }
    return status;
}

#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
static void bt_source_srv_hfp_ring_alerting_timeout_handle(uint32_t timer_id, uint32_t data)
{
    bt_source_srv_hfp_call_context_t call_context = {0};
    /* if no incomming call exist, return status */
    if (bt_source_srv_hfp_call_get_context_by_state(BT_SOURCE_SRV_CALL_STATE_INCOMING, &call_context, 1) == 0) {
        return;
    }

    bt_source_srv_hfp_send_ring_alerting(&call_context);
    /* restart timer */
    bt_timer_ext_status_t timer_status = bt_timer_ext_start(BT_SOURCE_SRV_RING_ALERTING_TIMER_ID, 0,
                                         BT_SOURCE_SRV_HFP_RING_ALERTING_TIMEOUT, bt_source_srv_hfp_ring_alerting_timeout_handle);
    if (timer_status != BT_TIMER_EXT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[HFP][AG] timeout start timer notify ring alerting fail = %d, context = %02x", 2, timer_status, call_context);
    }
}
#endif


static void bt_source_srv_hfp_ring_alerting_start(void)
{
    bt_source_srv_hfp_call_context_t call_context = {0};
    /* if no incomming call exist, return status */
    if (bt_source_srv_hfp_call_get_context_by_state(BT_SOURCE_SRV_CALL_STATE_INCOMING, &call_context, 1) == 0) {
        return;
    }

    bt_source_srv_hfp_send_ring_alerting(&call_context);
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
    /* start timer to notify ring and calling line identification(option) */
    bt_timer_ext_status_t timer_status = bt_timer_ext_start(BT_SOURCE_SRV_RING_ALERTING_TIMER_ID, 0,
                                         BT_SOURCE_SRV_HFP_RING_ALERTING_TIMEOUT, bt_source_srv_hfp_ring_alerting_timeout_handle);
    if (timer_status != BT_TIMER_EXT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[HFP][AG] start timer notify ring alerting fail = %d, context = %02x", 2, timer_status, &call_context);
    }
#endif
}

static void bt_source_srv_hfp_ring_alerting_stop(void)
{
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
    if (bt_timer_ext_find(BT_SOURCE_SRV_RING_ALERTING_TIMER_ID) == NULL) {
        return;
    }
    /* stop timer to notify ring and calling line identification(option) */
    bt_timer_ext_status_t timer_status = bt_timer_ext_stop(BT_SOURCE_SRV_RING_ALERTING_TIMER_ID);
    if (timer_status != BT_TIMER_EXT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[HFP][AG] stop timer notify ring alerting fail = %d", 1, timer_status);
    }
#endif
}

static void bt_source_srv_hfp_trigger_action_with_call_state_change(bt_source_srv_hfp_context_t *context,
        bt_source_srv_call_state_t previous_state, bt_source_srv_call_state_t new_state)
{
    /* this hold to stop audio is fix hold UL no sound */
    if ((previous_state == BT_SOURCE_SRV_CALL_STATE_ACTIVE) && (new_state == BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD)) {
        bt_source_srv_call_psd_audio_stop_req_t audio_stop_req = {
            .type = BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL,
        };
        bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_STOP_REQ, &audio_stop_req);
    }

    if ((previous_state == BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD) && (new_state == BT_SOURCE_SRV_CALL_STATE_ACTIVE)) {
        bt_source_srv_call_psd_audio_replay_req_t audio_replay_req = {
            .type = BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL,
        };
        if (bt_source_srv_common_audio_port_is_valid(BT_SOURCE_SRV_PORT_CHAT_SPEAKER) && (bt_source_srv_call_psd_is_ready(context->device))) {
            bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_REPLAY_REQ, &audio_replay_req);
        }
    }

    /* device event notify */
    if ((previous_state == BT_SOURCE_SRV_CALL_STATE_NONE) && (new_state != BT_SOURCE_SRV_CALL_STATE_NONE)) {
        bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_CALL_START, NULL);
    }

    if ((previous_state != BT_SOURCE_SRV_CALL_STATE_NONE) && (new_state == BT_SOURCE_SRV_CALL_STATE_NONE) && (!bt_source_srv_hfp_call_is_exist())) {
        bt_source_srv_call_psd_call_end_t call_end = {
            .is_allow_audio_stop = bt_source_srv_common_audio_port_is_valid(BT_SOURCE_SRV_PORT_MIC) ? false : true
        };
        bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_CALL_END, &call_end);
    }

    /* incomming ring alerting */
    if ((previous_state == BT_SOURCE_SRV_CALL_STATE_NONE) && (new_state == BT_SOURCE_SRV_CALL_STATE_INCOMING) &&
            (!bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_ACTIVE)) && (!bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD))) {
        bt_source_srv_hfp_feature_config_t hfp_feature_config = {0};
        if ((bt_source_srv_get_feature_config(BT_SOURCE_SRV_TYPE_HFP, &hfp_feature_config) == BT_STATUS_SUCCESS) &&
                (hfp_feature_config.feature & BT_SOURCE_SRV_HFP_FEATURE_IN_BAND_RING) && (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED) == 0)) {
            /* if support inband ringtone, the ring alerting start after esco transfer complete */
            BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_RING_START);
        } else {
            bt_source_srv_hfp_ring_alerting_start();
        }
    } else if ((previous_state == BT_SOURCE_SRV_CALL_STATE_NONE) && (new_state == BT_SOURCE_SRV_CALL_STATE_WAITING) &&
               (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_CALL_WAITING_ENABLED) > 0)) {
        /* call waiting notification */
        uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = {0};
        bt_source_srv_hfp_call_context_t call_context = {0};
        bt_source_srv_hfp_call_get_context_by_state(BT_SOURCE_SRV_CALL_STATE_WAITING, &call_context, 1);
        uint8_t phone_number_type = (call_context.iac == BT_SOURCE_SRV_HFP_CALL_IAC_WITHOUT) ? BT_SOURCE_SRV_HFP_PHONE_NUMBER_NATIONAL : BT_SOURCE_SRV_HFP_PHONE_NUMBER_INTERNATIONAL;
        snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+CCWA: \"%s\",%d", \
                 call_context.phone_number, phone_number_type);
        BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
    }

    if ((previous_state == BT_SOURCE_SRV_CALL_STATE_INCOMING) && (new_state != BT_SOURCE_SRV_CALL_STATE_INCOMING)) {
        BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_RING_START);
        bt_source_srv_hfp_ring_alerting_stop();
    }
}

static bt_status_t bt_source_srv_hfp_transfer_call_state_int(bt_source_srv_call_state_t previous_state, bt_source_srv_call_state_t new_state)
{
    bt_status_t status = BT_STATUS_FAIL;
    /* call state transfer */
    bt_source_srv_hfp_call_transfer_info_t transfer_info = {0};
    if (g_get_call_transfer_info_handle[previous_state](new_state, &transfer_info) != BT_STATUS_SUCCESS) {
        return status;
    }
    LOG_MSGID_I(source_srv, "[HFP][AG] call transfer type = %02x", 1, transfer_info.call_transfer);

    if (transfer_info.call_transfer & BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD) {
        status = bt_source_srv_hfp_transfer_phone_status(BT_HFP_AG_INDICATORS_CALL_HELD, transfer_info.held_status);
    }

    if (transfer_info.call_transfer & BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALL) {
        status = bt_source_srv_hfp_transfer_phone_status(BT_HFP_AG_INDICATORS_CALL, transfer_info.call_status);
    }

    if (transfer_info.call_transfer & BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALLSETUP) {
        status = bt_source_srv_hfp_transfer_phone_status(BT_HFP_AG_INDICATORS_CALL_SETUP, transfer_info.callsetup_status);
    }

    if (transfer_info.call_transfer & BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD_ACTIVE_SWAPPED) {
        status = bt_source_srv_hfp_transfer_phone_status(BT_HFP_AG_INDICATORS_CALL_HELD, transfer_info.held_status);
    }

    return status;
}

static void bt_source_srv_hfp_trigger_action_with_connected(bt_source_srv_hfp_context_t *context)
{
    /* call or va exist, trigger start play*/
    bt_status_t status = BT_STATUS_FAIL;
    bt_source_srv_hfp_call_context_t call_list[BT_SOURCE_SRV_HFP_CALL_MAX] = {{0}};
    uint32_t call_count = bt_source_srv_hfp_call_get_call_list(call_list, BT_SOURCE_SRV_HFP_CALL_MAX);

    for (uint32_t i = 0; i < call_count; i++) {
        if (!(g_hfp_common_context.common_flags & BT_SOURCE_SRV_HFP_FLAG_SLC_GET_CALL_INFO_COMPLETE)) {
            status = bt_source_srv_hfp_transfer_call_state_int(BT_SOURCE_SRV_CALL_STATE_NONE, call_list[i].call_state);
        }

        bt_source_srv_hfp_trigger_action_with_call_state_change(context, BT_SOURCE_SRV_CALL_STATE_NONE, call_list[i].call_state);

        /**
         * type:IOT issue
         * root cause:when the HFP reconnect and call is exist, headset will block esco connection initiated by the dongle.
         * solution:dongle doesn't create esco connect and will wait headset initiate AT+BCC.
         */
        if (bt_iot_device_white_list_check_iot_case(&context->remote_address, BT_IOT_CALL_HEADSET_BLOCK_ESCO_CONNECTION_WITH_RECONNECT)) {
            LOG_MSGID_W(source_srv, "[HFP][AG] IOT issue reconnect block esco connection", 0);
            continue;
        }

        bt_source_srv_hfp_audio_transfer_with_call(context, call_list[i].call_state, status);
    }

    if ((g_hfp_common_context.va_status == BT_SOURCE_SRV_VOICE_RECOGNITION_STATUS_ENABLE) && ((bt_source_srv_hfp_get_highlight_device() == context))) {
        bt_source_srv_voice_recognition_state_change_t state_change = {
            .status = BT_SOURCE_SRV_VOICE_RECOGNITION_STATUS_ENABLE
        };
        bt_source_srv_hfp_handle_voice_recognition_state_change(&state_change, sizeof(bt_source_srv_voice_recognition_state_change_t));
    }

    if ((g_hfp_common_context.common_flags & BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECT_WITH_SLC) &&
            (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED | BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING) == 0) &&
            (call_count == 0)) {
        bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_HF);
        LOG_MSGID_W(source_srv, "[HFP][AG] trigger esco connect with SLC", 0);
    }
    g_hfp_common_context.common_flags &= ~BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECT_WITH_SLC;
}

static bt_status_t bt_source_srv_hfp_handle_connect_request(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_connect_request_ind_t *message = (bt_hfp_connect_request_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_get_free_context();

    if (message->role == BT_HFP_ROLE_AG) {
        LOG_MSGID_W(source_srv, "[HFP][AG] connect request role = %02x", 1, message->role);
        return BT_STATUS_FAIL;
    }

    if (context == NULL) {
        return bt_hfp_connect_response(message->handle, false);
    }

    result = bt_hfp_connect_response(message->handle, true);
    if (result == BT_STATUS_SUCCESS) {
        context->handle = message->handle;
        bt_source_srv_memcpy(&context->remote_address, message->address, sizeof(bt_bd_addr_t));
        /* alloc pseduo device */
        context->device = bt_source_srv_call_psd_alloc_device(&context->remote_address, bt_source_srv_hfp_psd_callback);
        bt_source_srv_assert(context->device && "alloc pseduo device is NULL");
        /* notify pseduo device state machine */
        bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECT_REQ_IND, NULL);
    } else {
        LOG_MSGID_E(source_srv, "[HFP][AG] connect response fail status = %02x", 1, result);
    }
    return result;
}

static bt_status_t bt_source_srv_hfp_handle_slc_connected(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_slc_connected_ind_t *message = (bt_hfp_slc_connected_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    LOG_MSGID_I(source_srv, "[HFP][AG] connect complete context = %02x, handle = %02x, device = %02x", 3,
                context, context->handle, context->device);

    if (BT_STATUS_SUCCESS != status) {
        return BT_STATUS_FAIL;
    }

#ifdef MTK_BT_CM_SUPPORT
    /* notify cm AG connect success */
    bt_cm_profile_service_status_notify(BT_CM_PROFILE_SERVICE_HFP_AG, context->remote_address, BT_CM_PROFILE_SERVICE_STATE_CONNECTED, status);
#endif
    bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECTED, NULL);
    if (bt_source_srv_hfp_get_highlight_device() == NULL) {
        bt_source_srv_hfp_set_highlight_device(context);
    }

    bt_source_srv_call_audio_gain_init(&context->remote_address, context->device);

    if (g_hfp_common_context.audio_source_speaker_volume != 0) {
        bt_source_srv_call_audio_gain_update(BT_SOURCE_SRV_CALL_GAIN_AUDIO_SOURCE_SPEAKER, &context->remote_address, context->device, g_hfp_common_context.audio_source_speaker_volume);
    }

    /* handle call/va had exist before HFP connected*/
    bt_source_srv_hfp_trigger_action_with_connected(context);
    g_hfp_common_context.common_flags &= ~BT_SOURCE_SRV_HFP_FLAG_SLC_GET_CALL_INFO_COMPLETE;
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_handle_disconnected(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_disconnect_ind_t *message = (bt_hfp_disconnect_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    bt_source_srv_call_psd_set_codec_type(context->device, BT_HFP_CODEC_TYPE_NONE);
    BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED);
#ifdef MTK_BT_CM_SUPPORT
    /* notify cm AG connect success */
    bt_cm_profile_service_status_notify(BT_CM_PROFILE_SERVICE_HFP_AG, context->remote_address, BT_CM_PROFILE_SERVICE_STATE_DISCONNECTED, status);
#endif
    bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED, NULL);
    bt_source_srv_hfp_ring_alerting_stop();
    if (bt_source_srv_hfp_get_highlight_device() == context) {
        /* because there is no EMP, so the highlight is set to NULL */
        bt_source_srv_hfp_set_highlight_device(NULL);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_handle_brsf_feature(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_hf_feature_ind_t *message = (bt_hfp_hf_feature_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context != NULL) {
        context->support_feature = message->hf_feature;
        LOG_MSGID_I(source_srv, "[HFP][AG] support feature = %02x", 1, context->support_feature);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_handle_call_hold_feature(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_hold_feature_ind_t *message = (bt_hfp_hold_feature_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context != NULL) {
        context->hold_feature = message->feature;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_handle_audio_connected(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_audio_connect_ind_t *message = (bt_hfp_audio_connect_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED);
    BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING);

    if (BT_STATUS_SUCCESS == status) {
        bt_source_srv_call_psd_set_codec_type(context->device, message->codec);
        bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_CONNECTED, NULL);

        bt_source_srv_esco_state_update_t esco_state = {
            .state = BT_SOURCE_SRV_ESCO_CONNECTION_STATE_CONNECTED,
            .codec_type = message->codec,
        };
        bt_source_srv_memcpy(esco_state.peer_address.addr, context->remote_address, sizeof(bt_bd_addr_t));
        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_ESCO_STATE_UPDATE, &esco_state, sizeof(bt_source_srv_esco_state_update_t));

        if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_DISCONNECT)) {
            bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_AG);
            BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SCO_DISCONNECT);
        }
#ifdef AIR_FEATURE_SOURCE_MHDT_SUPPORT
        bt_source_srv_common_switch_mhdt(&context->remote_address, false);
#endif
    }

    /* start ring alerting when incomming call esco conencted */
    if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_RING_START)) {
        BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_RING_START);
        bt_source_srv_hfp_ring_alerting_start();
    }

    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_handle_audio_status_change(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_audio_status_change_ind_t *message = (bt_hfp_audio_status_change_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    if (message->audio_status == BT_HFP_AUDIO_STATUS_ACTIVE) {
        bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_ACTIVATED, NULL);
    } else {
        bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DEACTIVATED, NULL);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_handle_audio_disconnected(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_audio_disconnect_ind_t *message = (bt_hfp_audio_disconnect_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    if ((BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_CANCEL_SCO_TO_RECONNECT)) && (status == BT_STATUS_FAIL)) {
        BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_CANCEL_SCO_TO_RECONNECT);
        LOG_MSGID_I(source_srv, "[HFP][AG] context = %02x reconnect sco after cancel", 1, context);
        return bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_HF);
    }

    bt_source_srv_call_psd_set_codec_type(context->device, BT_HFP_CODEC_TYPE_NONE);

#ifdef AIR_FEATURE_SOURCE_MHDT_SUPPORT
    bt_source_srv_common_switch_mhdt(&context->remote_address, true);
#endif

    BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED);
    BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING);
    bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DISCONNECTED, NULL);

    bt_source_srv_esco_state_update_t esco_state = {
        .state = BT_SOURCE_SRV_ESCO_CONNECTION_STATE_DISCONNECTED,
        .codec_type = BT_HFP_CODEC_TYPE_NONE,
    };
    bt_source_srv_memcpy(esco_state.peer_address.addr, context->remote_address, sizeof(bt_bd_addr_t));
    bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_ESCO_STATE_UPDATE, &esco_state, sizeof(bt_source_srv_esco_state_update_t));

    if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_PROFILE_DISCONNECT)) {
        BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_PROFILE_DISCONNECT);
        result = bt_hfp_disconnect(context->handle);
        if (result == BT_STATUS_SUCCESS) {
            bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECT_REQ, NULL);
        }
    }

    /* IOT issue handle, transfer esco when call active */
    if ((bt_iot_device_white_list_check_iot_case(&context->remote_address, BT_IOT_CALL_ESCO_DISCONNECT_WITH_CALL_ACTIVE_RECONNECT)) &&
            (bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_ACTIVE))) {
        LOG_MSGID_W(source_srv, "[HFP][AG] handle IOT issue to transfer esco", 0);
        bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_HF);
    }
    return result;
}

static bt_status_t bt_source_srv_hfp_handle_ag_indicators_enable(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_ag_indicators_enable_ind_t *message = (bt_hfp_ag_indicators_enable_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    LOG_MSGID_I(source_srv, "[HFP][AG] ag indicator = %02x status = %02x", 2, message->ag_indicators, message->status);
    if (message->ag_indicators == BT_HFP_AG_INDICATORS_ALL) {
        if (message->status == BT_HFP_INDICATOR_ON) {
            BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_CALL_AG_INDICATOR_ENABLED);
            context->ag_indicators = BT_HFP_AG_INDICATORS_ALL;
        } else if (message->status == BT_HFP_INDICATOR_OFF) {
            BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_CALL_AG_INDICATOR_ENABLED);
            context->ag_indicators = 0;
        }
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    } else {
        if (message->status == BT_HFP_INDICATOR_ON) {
            context->ag_indicators |= message->ag_indicators;
        } else if (message->status == BT_HFP_INDICATOR_OFF) {
            context->ag_indicators &= ~message->ag_indicators;
        }
    }
    return result;
}

static bt_status_t bt_source_srv_hfp_handle_operator_format(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_operator_format_ind_t *message = (bt_hfp_operator_format_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context != NULL) {
        return BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    }
    return BT_STATUS_FAIL;
}

static bt_status_t bt_source_srv_hfp_handle_query_operator(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_query_operator_ind_t *message = (bt_hfp_query_operator_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    bt_source_srv_phone_card_info_t phone_crad_info = {0};
    if (bt_source_srv_get_phone_card_information(BT_SOURCE_SRV_TYPE_HFP, &phone_crad_info, 1) != 0) {
        uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = {0};
        snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+COPS:%d,0,\"%s\"", \
                 phone_crad_info.operator_mode, phone_crad_info.operator_information);
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
        if (result != BT_STATUS_SUCCESS) {
            LOG_MSGID_E(source_srv, "[HFP][AG] query operator send cmd fail status = %02x", 1, result);
        }
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    } else {
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context);
    }
    return result;
}

static bt_status_t bt_source_srv_hfp_handle_error_code_enable(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_error_code_enable_ind_t *message = (bt_hfp_error_code_enable_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context != NULL) {
        return BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context);
    }
    return BT_STATUS_FAIL;
}

static bt_status_t bt_source_srv_hfp_handle_codec_connection(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_codec_connection_ind_t *message = (bt_hfp_codec_connection_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING)) {
        LOG_MSGID_W(source_srv, "[HFP][AG] context = %02x codec connection esco is connecting", 1, context);
        BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_CANCEL_SCO_TO_RECONNECT);
        return bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_AG);
    }

    if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED)) {
        LOG_MSGID_W(source_srv, "[HFP][AG] context = %02x codec connection esco had connected", 1, context);
        BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_CANCEL_SCO_TO_RECONNECT);
        return bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_AG);
    }

    /* initiate sco connect */
    result = bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_HF);
    if (result != BT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[HFP][AG] initiate sco connect status = %02x by HF", 1, result);
    }
    return result;
}

static bt_status_t bt_source_srv_hfp_handle_answer_call(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_answer_call_ind_t *message = (bt_hfp_answer_call_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    bt_source_srv_hfp_call_context_t call_context = {0};
    uint32_t call_count = bt_source_srv_hfp_call_get_context_by_state(BT_SOURCE_SRV_CALL_STATE_INCOMING, &call_context, 1);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    bt_source_srv_accept_call_t accept_call_ind = {
        .type = BT_SOURCE_SRV_TYPE_HFP,
    };

    if (0 != call_count) {
        accept_call_ind.index = call_context.index;
        /**
         * type:source issue
         * root cause:send alerting after receive ATA cause HFP/AG/ICA/BV-04-I fail.
         * solution:stop alerting after receive ATA.
         */
        bt_source_srv_hfp_ring_alerting_stop();
    } else {
        /* fix IOT issue: notify app to teams play window and response ok when call is does not exist. */
        accept_call_ind.index = BT_SOURCE_SRV_CALL_INVALID_INDEX;
    }
    bt_source_srv_memcpy(&accept_call_ind.peer_address.addr, &context->remote_address, sizeof(bt_bd_addr_t));
    result = BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_ACCEPT_CALL, &accept_call_ind, sizeof(bt_source_srv_accept_call_t));
    return result;
}

static bt_status_t bt_source_srv_hfp_handle_terminate_call(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_terminate_call_ind_t *message = (bt_hfp_terminate_call_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    uint32_t i = 0;
    bt_source_srv_call_state_t call_state_table[] = {
        BT_SOURCE_SRV_CALL_STATE_ACTIVE,
        BT_SOURCE_SRV_CALL_STATE_INCOMING,
        BT_SOURCE_SRV_CALL_STATE_WAITING,
        BT_SOURCE_SRV_CALL_STATE_DIALING,
        BT_SOURCE_SRV_CALL_STATE_ALERTING
    };

    for (i = 0; i < sizeof(call_state_table); i++) {
        bt_source_srv_hfp_call_context_t call_context = {0};
        uint32_t call_count = bt_source_srv_hfp_call_get_context_by_state(call_state_table[i], &call_context, 1);
        if (0 != call_count) {
            bt_source_srv_terminate_call_t terminate_call_ind = {
                .type = BT_SOURCE_SRV_TYPE_HFP,
                .index = call_context.index,
            };
            bt_source_srv_memcpy(&terminate_call_ind.peer_address.addr, &context->remote_address, sizeof(bt_bd_addr_t));
            result = BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
            bt_source_srv_event_t event_id = (((i == 1) || (i == 2))) ? BT_SOURCE_SRV_EVENT_REJECT_CALL : BT_SOURCE_SRV_EVENT_TERMINATE_CALL;
            if ((i == 1) || (i == 2)) {
                /**
                 * type:source issue
                 * root cause:send alerting after receive reject.
                 * solution:stop alerting after receive CHUP.
                 */
                bt_source_srv_hfp_ring_alerting_stop();
            }
            bt_source_srv_hfp_event_notify(event_id, &terminate_call_ind, sizeof(bt_source_srv_terminate_call_t));
            break;
        }
    }

    if (i == sizeof(call_state_table)) {
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context);
    }

    return result;
}

static bt_status_t bt_source_srv_hfp_handle_dial_number(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_dial_number_ind_t *message = (bt_hfp_dial_number_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    bt_source_srv_dial_number_t dail_number_ind = {
        .type = BT_SOURCE_SRV_TYPE_HFP,
        .number = (uint8_t *)message->number,
        .number_length = message->number_length,
    };
    bt_source_srv_memcpy(&dail_number_ind.peer_address.addr, &context->remote_address, sizeof(bt_bd_addr_t));
    result = BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_DIAL_NUMBER, &dail_number_ind, sizeof(bt_source_srv_dial_number_t));

    return result;
}

static bt_status_t bt_source_srv_hfp_handle_dial_memory(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_dial_memory_ind_t *message = (bt_hfp_dial_memory_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context != NULL) {

        /**
         * type:BQB workaround
         * root cause:handle the HFP/AG/OCM/BV-02-I, no dialing memory, but BQB need test empty memory.
         * solution:IUT response error when BQB need dailing empty memory.
         */
        if (bt_source_srv_hfp_get_bqb_flag() & BT_SOURCE_SRV_HFP_BQB_FLAG_DIALING_CALL_ERROR) {
            LOG_MSGID_W(source_srv, "[HFP][AG] dialing empty memory", 0);
            return BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context);
        }

        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_DIAL_MEMORY, NULL, 0);
        return BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    }
    return BT_STATUS_FAIL;
}

static bt_status_t bt_source_srv_hfp_handle_dial_last(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_dial_last_ind_t *message = (bt_hfp_dial_last_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context != NULL) {
        /**
         * type:BQB workaround
         * root cause:handle the HFP/AG/OCL/BV-02-I, no dialing memory, but BQB need to dial last.
         * solution:IUT response error when BQB need dailing last.
         */
        if (bt_source_srv_hfp_get_bqb_flag() & BT_SOURCE_SRV_HFP_BQB_FLAG_DIALING_CALL_ERROR) {
            LOG_MSGID_W(source_srv, "[HFP][AG] dialing last error", 0);
            return BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context);
        }

        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_DIAL_LAST_NUMBER, NULL, 0);
        return BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    }
    return BT_STATUS_FAIL;
}

static bt_status_t bt_source_srv_hfp_handle_call_waiting_enable(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_call_waiting_enable_ind_t *message = (bt_hfp_call_waiting_enable_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context != NULL) {
        if (message->enable) {
            BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_CALL_WAITING_ENABLED);
        } else {
            BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_CALL_WAITING_ENABLED);
        }
        return BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    }
    return BT_STATUS_FAIL;
}

static bt_status_t bt_source_srv_hfp_handle_caller_id_enable(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_caller_id_enable_ind_t *message = (bt_hfp_caller_id_enable_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context != NULL) {
        LOG_MSGID_I(source_srv, "[HFP][AG] clip receive status = %02x", 1, message->enable);
        if (message->enable) {
            BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_CLI_ENABLED);
        } else {
            BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_CLI_ENABLED);
        }
        return BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    }
    return BT_STATUS_FAIL;
}

static bt_status_t bt_source_srv_hfp_handle_dtmf_code(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_dtmf_code_ind_t *message = (bt_hfp_dtmf_code_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context != NULL) {
        bt_source_srv_dtmf_t dtmf_ind = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
            .dtmf_value = message->code,
        };
        bt_source_srv_memcpy(&dtmf_ind.peer_address.addr, &context->remote_address, sizeof(bt_bd_addr_t));
        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_CALL_INDEX_IND, &dtmf_ind, sizeof(bt_source_srv_dtmf_t));
        return BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    }
    return BT_STATUS_FAIL;
}

static bt_status_t bt_source_srv_hfp_handle_query_subscriber_number(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_query_subscriber_number_ind_t *message = (bt_hfp_query_subscriber_number_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    bt_source_srv_phone_card_info_t phone_crad_info = {0};
    if (bt_source_srv_get_phone_card_information(BT_SOURCE_SRV_TYPE_HFP, &phone_crad_info, 1) != 0) {
        uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = {0};
        snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+CNUM: ,\"%s\",%d,,%d", \
                 phone_crad_info.own_number, phone_crad_info.own_number_type, phone_crad_info.own_number_service);
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
        if (result != BT_STATUS_SUCCESS) {
            LOG_MSGID_E(source_srv, "[HFP][AG] get subscriber number send cmd fail status = %02x", 1, result);
        }
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    } else {
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context);
    }

    return result;
}

static bt_status_t bt_source_srv_hfp_handle_query_call_list(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_query_call_list_ind_t *message = (bt_hfp_query_call_list_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    uint8_t cmd_buffer[BT_SOURCE_SRV_HFP_CMD_LENGTH] = {0};
    bt_source_srv_hfp_call_context_t call_list[BT_SOURCE_SRV_HFP_CALL_MAX] = {{0}};
    uint32_t call_count = bt_source_srv_hfp_call_get_call_list(call_list, BT_SOURCE_SRV_HFP_CALL_MAX);
    if (call_count == 0) {
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context);
        return BT_STATUS_FAIL;
    }

    for (uint32_t i = 0; i < call_count; i++) {
        bt_source_srv_hfp_call_context_t *call_context = &call_list[i];
        bt_source_srv_hfp_clcc_call_state_t hfp_call_state = g_source_srv_hfp_clcc_call_state_map_table[call_context->call_state];
        snprintf((char *)cmd_buffer, BT_SOURCE_SRV_HFP_CMD_LENGTH, "+CLCC:%d,%d,%d,%d,%d,\"%s\",%d", \
                 call_context->index, call_context->dir, hfp_call_state, call_context->mode, call_context->mpty, call_context->phone_number, call_context->iac);
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_CODE(context, cmd_buffer);
        if (result != BT_STATUS_SUCCESS) {
            LOG_MSGID_E(source_srv, "[HFP][AG] get call list send cmd fail status = %02x, call index = %02x", 2, result, call_list[i].index);
        }
    }
    return BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
}

static bt_status_t bt_source_srv_hfp_handle_hold_call(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_hold_call_ind_t *message = (bt_hfp_hold_call_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    bt_source_srv_hfp_feature_config_t hfp_feature_config = {0};
    if (bt_source_srv_get_feature_config(BT_SOURCE_SRV_TYPE_HFP, &hfp_feature_config) != BT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[HFP][AG] get feature config fail", 0);
        return BT_STATUS_FAIL;
    }

    LOG_MSGID_I(source_srv, "[HFP][AG] hold call local feature = %02x remote feature = %02x hold feature = %02x", 3,
                hfp_feature_config.feature, context->support_feature, hfp_feature_config.hold_feature);
    if ((hfp_feature_config.feature & BT_SOURCE_SRV_HFP_FEATURE_3_WAY) && (context->support_feature & BT_HFP_HF_FEATURE_3_WAY)) {
        if (!(hfp_feature_config.hold_feature & (1 << (message->type)))) {
            /* not support this hold feature */
            result = BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context);
            return BT_STATUS_SUCCESS;
        }
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
        bt_source_srv_hfp_call_handle_multiparty(message->type, message->index, &context->remote_address);
    } else {
        result = BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context);
    }

    return result;
}

static bt_status_t bt_source_srv_hfp_handle_sync_volume_speaker_gain(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_volume_sync_speaker_gain_ind_t *message = (bt_hfp_volume_sync_speaker_gain_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    bt_source_srv_volume_change_ind_t volume_change = {0};
    bt_source_srv_common_audio_port_context_t audio_port = {0};
    if (bt_source_srv_common_audio_find_port_context(BT_SOURCE_SRV_PORT_CHAT_SPEAKER, &audio_port) == BT_STATUS_SUCCESS) {
        volume_change.port = BT_SOURCE_SRV_PORT_CHAT_SPEAKER;
    } else if (bt_source_srv_common_audio_find_port_context(BT_SOURCE_SRV_PORT_GAMING_SPEAKER, &audio_port) == BT_STATUS_SUCCESS) {
        volume_change.port = BT_SOURCE_SRV_PORT_GAMING_SPEAKER;
    }

    uint32_t old_volume = bt_source_srv_call_get_audio_gain_level(BT_SOURCE_SRV_CALL_GAIN_SPEAKER, &context->remote_address);
    LOG_MSGID_I(source_srv, "[HFP][AG] speaker old volume %02x new volume = %02x", 2, old_volume, message->data);
    if (volume_change.port != BT_SOURCE_SRV_PORT_NONE) {
        volume_change.step_type = (message->data > old_volume) ? BT_SOURCE_SRV_VOLUME_STEP_TYPE_UP : BT_SOURCE_SRV_VOLUME_STEP_TYPE_DOWN;
        volume_change.volume_step = bt_source_srv_call_convert_volume_step(message->data < old_volume ? message->data : old_volume, message->data > old_volume ? message->data : old_volume);
        if (0 == message->data) {
            volume_change.mute_state = BT_SOURCE_SRV_MUTE_STATE_ENABLE;
            bt_source_src_call_audio_mute(BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL);
            BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SPEAKER_MUTED);
        } else if ((0 != message->data) && (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SPEAKER_MUTED))) {
            BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SPEAKER_MUTED);
            bt_source_src_call_audio_unmute(BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL);
            volume_change.mute_state = BT_SOURCE_SRV_MUTE_STATE_DISABLE;
        }
        bt_source_srv_memcpy(&volume_change.peer_address.addr, &context->remote_address, sizeof(bt_bd_addr_t));

        LOG_MSGID_I(source_srv, "[HFP][AG] speaker volume port = %02x step type = %02x volume step = %02x mute state = %02x", 4,
                    volume_change.port, volume_change.step_type, volume_change.volume_step, volume_change.mute_state);

        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_VOLUME_CHANGE, &volume_change, sizeof(bt_source_srv_volume_change_ind_t));
    }

    if (0 != message->data) {
        bt_source_srv_call_audio_gain_update(BT_SOURCE_SRV_CALL_GAIN_SPEAKER, &context->remote_address, context->device, message->data);
    }
    return BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
}

static bt_status_t bt_source_srv_hfp_handle_volume_sync_mic_gain(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_volume_sync_mic_gain_ind_t *message = (bt_hfp_volume_sync_mic_gain_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    bt_source_srv_volume_change_ind_t volume_change = {0};
    bt_source_srv_common_audio_port_context_t audio_port = {0};
    uint32_t old_volume = bt_source_srv_call_get_audio_gain_level(BT_SOURCE_SRV_CALL_GAIN_MIC, &context->remote_address);
    LOG_MSGID_I(source_srv, "[HFP][AG] mic old volume %02x new volume = %02x", 2, old_volume, message->data);
    if (bt_source_srv_common_audio_find_port_context(BT_SOURCE_SRV_PORT_MIC, &audio_port) == BT_STATUS_SUCCESS) {
        volume_change.port = BT_SOURCE_SRV_PORT_MIC;
        volume_change.step_type = (message->data > old_volume) ? BT_SOURCE_SRV_VOLUME_STEP_TYPE_UP : BT_SOURCE_SRV_VOLUME_STEP_TYPE_DOWN;
        volume_change.volume_step = bt_source_srv_call_convert_volume_step(message->data < old_volume ? message->data : old_volume, message->data > old_volume ? message->data : old_volume);
        if (0 == message->data) {
            volume_change.mute_state = BT_SOURCE_SRV_MUTE_STATE_ENABLE;
            BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_MIC_MUTED);
        } else if ((0 != message->data) && (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_MIC_MUTED))) {
            BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_MIC_MUTED);
            volume_change.mute_state = BT_SOURCE_SRV_MUTE_STATE_DISABLE;
        }
        bt_source_srv_memcpy(&volume_change.peer_address.addr, &context->remote_address, sizeof(bt_bd_addr_t));

        LOG_MSGID_I(source_srv, "[HFP][AG] mic volume port = %02x step type = %02x volume step = %02x mute state= %02x", 4,
                    volume_change.port, volume_change.step_type, volume_change.volume_step, volume_change.mute_state);

        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_VOLUME_CHANGE, &volume_change, sizeof(bt_source_srv_volume_change_ind_t));
    }

    if (0 != message->data) {
        bt_source_srv_call_audio_gain_update(BT_SOURCE_SRV_CALL_GAIN_MIC, &context->remote_address, context->device, message->data);
    }
    return BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
}

static bt_status_t bt_source_srv_hfp_handle_voice_recognition(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_status_t result = BT_STATUS_FAIL;
    bt_hfp_voice_recognition_ind_t *message = (bt_hfp_voice_recognition_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    /* VA status notify app, so transfer esco will wait app trigger */
#if 0
    if ((message->enable) && (!BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED))) {
        result = bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_HF);
    } else if ((!message->enable) && (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED))) {
        result = bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_AG);
    }
#endif

    result = BT_SOURCE_SRV_HFP_CMD_RESULT_OK(context);
    bt_source_srv_voice_recognition_activation_t va_activation = {
        .type = BT_SOURCE_SRV_TYPE_HFP,
        .status = (uint8_t)message->enable
    };
    bt_source_srv_memcpy(&va_activation.peer_address.addr, &context->remote_address, sizeof(bt_bd_addr_t));
    bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_VOICE_RECOGNITION_ACTIVATION, &va_activation, sizeof(bt_source_srv_voice_recognition_activation_t));
    return result;
}

static bt_status_t bt_source_srv_hfp_handle_custom_command_result(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_custom_command_result_ind_t *message = (bt_hfp_custom_command_result_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    bt_status_t result = BT_STATUS_FAIL;

    if (NULL == context) {
        return result;
    }

    LOG_MSGID_I(source_srv, "[HFP][AG] custom command result length = %02x", 1, strlen(message->result));
    result = BT_SOURCE_SRV_HFP_CMD_RESULT_ERROR(context);

    if ((bt_source_srv_memcmp(message->result, "AT+NREC", strlen("AT+NREC")) == 0) &&
            (bt_iot_device_white_list_check_iot_case(&context->remote_address, BT_IOT_CALL_HEADSET_BLOCK_ESCO_CONNECTION_WITH_RECONNECT)) &&
            (!BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED | BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING)) &&
            (bt_source_srv_hfp_get_highlight_device() == context)) {

        /* get call inforamtion */
        bt_source_srv_hfp_call_context_t call_list[BT_SOURCE_SRV_HFP_CALL_MAX] = {{0}};
        uint32_t call_count = bt_source_srv_hfp_call_get_call_list(call_list, BT_SOURCE_SRV_HFP_CALL_MAX);
        if (call_count == 0) {
            return result;
        }

        bt_source_srv_hfp_feature_config_t hfp_feature_config = {0};
        if ((call_count == 1) && (bt_source_srv_hfp_call_is_exist_by_state(BT_SOURCE_SRV_CALL_STATE_INCOMING)) &&
                (bt_source_srv_get_feature_config(BT_SOURCE_SRV_TYPE_HFP, &hfp_feature_config) == BT_STATUS_SUCCESS) &&
                (!(hfp_feature_config.feature & BT_SOURCE_SRV_HFP_FEATURE_IN_BAND_RING))) {
            return result;
        }

        result = bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_HF);
        LOG_MSGID_I(source_srv, "[HFP][AG] IOT issue create esco with call exist status = %02x", 1, result);
    }

    return result;
}

static bt_status_t bt_source_srv_hfp_handle_ready_to_send(bt_msg_type_t msg, bt_status_t status, void *parameter)
{
    bt_hfp_ready_to_send_ind_t *message = (bt_hfp_ready_to_send_ind_t *)parameter;
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_find_context_by_handle(message->handle);
    if (context != NULL) {
        bt_source_srv_hfp_run_oom_wl(context);
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_FAIL;
}

static bt_status_t bt_source_srv_hfp_audio_transfer_with_call(bt_source_srv_hfp_context_t *context,
        bt_source_srv_call_state_t new_state, bt_status_t call_transfer_result)
{
    bool is_audio_transfer = false;
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_hfp_audio_direction_t audio_direction = BT_HFP_AUDIO_TO_HF;

    /* esco logic processing */
    if ((new_state == BT_SOURCE_SRV_CALL_STATE_NONE) && (!bt_source_srv_hfp_call_is_exist())) {
        if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED)) {
            /* disconnect esco */
            is_audio_transfer = true;
            audio_direction = BT_HFP_AUDIO_TO_AG;
        } else if ((BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING)) && (!bt_source_srv_common_audio_port_is_valid(BT_SOURCE_SRV_PORT_MIC))) {
            /* because esco is connecting, so disconnect esco after esco connect complete. */
            BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SCO_DISCONNECT);
        }
    } else if (!BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED | BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING)) {
        if (new_state == BT_SOURCE_SRV_CALL_STATE_INCOMING) {
            bt_source_srv_hfp_feature_config_t hfp_feature_config = {0};
            if (bt_source_srv_get_feature_config(BT_SOURCE_SRV_TYPE_HFP, &hfp_feature_config) != BT_STATUS_SUCCESS) {
                LOG_MSGID_E(source_srv, "[HFP][AG] get feature config fail", 0);
            }

            if (hfp_feature_config.feature & BT_SOURCE_SRV_HFP_FEATURE_IN_BAND_RING) {
                is_audio_transfer = true;
            }
        } else if ((new_state != BT_SOURCE_SRV_CALL_STATE_NONE)) {
            is_audio_transfer = true;
        }
    } else {
        LOG_MSGID_W(source_srv, "[HFP][AG] call transfer esco create conetxt = %02x flag = %02x", 2, context, context->flags);
    }

    if (is_audio_transfer) {
        if (call_transfer_result == BT_STATUS_OUT_OF_MEMORY) {
            bt_source_srv_hfp_oom_wl_action_parameter_t action_parameter = {
                .audio_direction = audio_direction
            };
            bt_source_srv_hfp_add_oom_wl(BT_SOURCE_SRV_HFP_OOM_WL_ACTION_ESCO, context, &action_parameter);
            return BT_STATUS_OUT_OF_MEMORY;
        }
        status = bt_source_srv_hfp_audio_transfer(context, audio_direction);
    }
    return status;
}

static void bt_source_srv_hfp_sync_volume_with_audio_source(bt_source_srv_hfp_context_t *context, bool is_resume_volume)
{
    bt_source_srv_volume_change_ind_t volume_change = {0};
    bt_source_srv_common_audio_port_context_t audio_port = {0};
    uint32_t new_volume = 0;
    uint32_t old_volume = 0;

    if (is_resume_volume) {
        new_volume = bt_source_srv_call_get_audio_gain_level(BT_SOURCE_SRV_CALL_GAIN_AUDIO_SOURCE_SPEAKER, &context->remote_address);
        old_volume = bt_source_srv_call_get_audio_gain_level(BT_SOURCE_SRV_CALL_GAIN_SPEAKER, &context->remote_address);;
    } else {
        new_volume = bt_source_srv_call_get_audio_gain_level(BT_SOURCE_SRV_CALL_GAIN_SPEAKER, &context->remote_address);
        old_volume = g_hfp_common_context.audio_source_speaker_volume;
    }

    if (bt_source_srv_common_audio_find_port_context(BT_SOURCE_SRV_PORT_CHAT_SPEAKER, &audio_port) == BT_STATUS_SUCCESS) {
        volume_change.port = BT_SOURCE_SRV_PORT_CHAT_SPEAKER;
    } else if (bt_source_srv_common_audio_find_port_context(BT_SOURCE_SRV_PORT_GAMING_SPEAKER, &audio_port) == BT_STATUS_SUCCESS) {
        volume_change.port = BT_SOURCE_SRV_PORT_GAMING_SPEAKER;
    }

    volume_change.step_type = (new_volume > old_volume) ? BT_SOURCE_SRV_VOLUME_STEP_TYPE_UP : BT_SOURCE_SRV_VOLUME_STEP_TYPE_DOWN;
    volume_change.volume_step = bt_source_srv_call_convert_volume_step(old_volume < new_volume ? old_volume : new_volume, old_volume > new_volume ? old_volume : new_volume);
    volume_change.mute_state = BT_SOURCE_SRV_MUTE_STATE_DISABLE;
    bt_source_srv_memcpy(&volume_change.peer_address.addr, &context->remote_address, sizeof(bt_bd_addr_t));

    LOG_MSGID_I(source_srv, "[HFP][AG] sync speaker volume port = %02x old volume = %02x new volume = %02x step type = %02x volume step = %02x mute state = %02x", 6,
                volume_change.port, old_volume, new_volume, volume_change.step_type, volume_change.volume_step, volume_change.mute_state);

    bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_VOLUME_CHANGE, &volume_change, sizeof(bt_source_srv_volume_change_ind_t));
}

#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
static void bt_source_srv_hfp_audio_stop_handle(uint32_t timer_id, uint32_t data)
{
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_get_highlight_device();
    if (context == NULL) {
        LOG_MSGID_W(source_srv, "[HFP][AG] audio stop highlight is NULL", 0);
        return;
    }

    bt_source_srv_port_t port = (bt_source_srv_port_t)data;
    bt_source_srv_call_psd_audio_stop_req_t audio_stop_req = {
        .type = g_source_srv_play_type_table[port],
        .port_action = BT_SOURCE_SRV_CALL_PSD_CALL_ACTION_CLOSE,
    };
    bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_STOP_REQ, &audio_stop_req);
}
#endif

bt_status_t bt_source_srv_hfp_audio_port_update(bt_source_srv_port_t audio_port, bt_source_srv_call_port_action_t action)
{
    LOG_MSGID_I(source_srv, "[HFP][AG] audio port = %02x update action = %02x", 2, audio_port, action);

    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_get_highlight_device();
    if (context == NULL) {
        LOG_MSGID_W(source_srv, "[HFP][AG] audio port update highlight is NULL", 0);
        if ((BT_SOURCE_SRV_PORT_MIC == audio_port) && (!bt_source_srv_hfp_call_is_exist()) && (BT_SOURCE_SRV_CALL_PORT_ACTION_OPEN == action)) {
            g_hfp_common_context.common_flags |= BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECT_WITH_SLC;
        } else if ((BT_SOURCE_SRV_PORT_MIC == audio_port) && (!bt_source_srv_hfp_call_is_exist()) && (BT_SOURCE_SRV_CALL_PORT_ACTION_CLOSE == action)) {
            g_hfp_common_context.common_flags &= ~BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECT_WITH_SLC;
        }
        return BT_STATUS_FAIL;
    }

    switch (action) {
        case BT_SOURCE_SRV_CALL_PORT_ACTION_OPEN: {
            if ((BT_SOURCE_SRV_PORT_MIC == audio_port) && (!bt_source_srv_hfp_call_is_exist()) &&
                    (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED | BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING) == 0)) {
                bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_HF);
            }

            if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED)) {
                bt_source_srv_call_psd_audio_replay_req_t audio_replay_req = {
                    .type = g_source_srv_play_type_table[audio_port],
                    .port_action = ((BT_SOURCE_SRV_PORT_CHAT_SPEAKER == audio_port) && bt_source_srv_common_audio_port_is_valid(BT_SOURCE_SRV_PORT_MIC)) ? \
                    BT_SOURCE_SRV_CALL_PSD_CALL_ACTION_UPDATE : BT_SOURCE_SRV_CALL_PSD_CALL_ACTION_OPEN, /* AG force open DL when only MIC*/
                };
                bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_REPLAY_REQ, &audio_replay_req);
            }
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
            bt_timer_ext_t *timer_ext = bt_timer_ext_find(BT_SOURCE_SRV_CALL_AUDIO_STOP_TIMER_ID);
            if ((NULL != timer_ext) && (timer_ext->data == audio_port)) {
                bt_timer_ext_status_t timer_status = bt_timer_ext_stop(BT_SOURCE_SRV_CALL_AUDIO_STOP_TIMER_ID);
                LOG_MSGID_I(source_srv, "[HFP][AG] stop audio stop timer status = %d", 1, timer_status);
            }
#endif
        }
        break;
        case BT_SOURCE_SRV_CALL_PORT_ACTION_CLOSE: {

            if ((BT_SOURCE_SRV_PORT_MIC == audio_port) && (!bt_source_srv_hfp_call_is_exist())) {
                if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED) > 0)  {
                    bt_source_srv_hfp_audio_transfer(context, BT_HFP_AUDIO_TO_AG);
                } if (BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING) > 0) {
                    BT_SOURCE_SRV_HFP_SET_FLAG(context, BT_SOURCE_SRV_HFP_FLAG_SCO_DISCONNECT);
                }
            }

            /**
            * type:source issue
            * root cause:active call + incomming call, NB will colse MIC port when end active call, therefore the second incomming call ringtone no sound.
            * solution:HFP cannot notify audio stop request when MIC port close and call exist.
            */
            if ((BT_SOURCE_SRV_PORT_MIC == audio_port) && (bt_source_srv_hfp_call_is_exist())) {
                break;
            }

            /**
            * type:workaround
            * root cause:open speaker port->close speaker port HFP will stop audio when MIC port open.
            * solution:judge MIC port state when speaker port close.
            */
            if ((BT_SOURCE_SRV_PORT_CHAT_SPEAKER == audio_port) && (bt_source_srv_common_audio_port_is_valid(BT_SOURCE_SRV_PORT_MIC))) {
                break;
            }

            if (bt_source_srv_call_psd_is_playing(context->device)) {
                /* start timer to stop audio */
                /**
                 * type:source issue
                 * root cause:because source receive call end(stop audio) after port closed, so audio will send null packet to headset cause noise.
                 * solution:source stop audio when port colse, start timer:port anti-shake processing.
                 */
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
                if (NULL != bt_timer_ext_find(BT_SOURCE_SRV_CALL_AUDIO_STOP_TIMER_ID)) {
                    break;
                }

                bt_timer_ext_status_t timer_status = bt_timer_ext_start(BT_SOURCE_SRV_CALL_AUDIO_STOP_TIMER_ID, audio_port,
                                                     BT_SOURCE_SRV_HFP_AUDIO_STOP_TIMEOUT, bt_source_srv_hfp_audio_stop_handle);
                if (timer_status != BT_TIMER_EXT_STATUS_SUCCESS) {
                    LOG_MSGID_E(source_srv, "[HFP][AG] start timer aduio stop fail = %d", 1, timer_status);
                }
#endif
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_PORT_ACTION_UPDATE: {
            /* ingonre device is not playing case */
            if (bt_source_srv_call_psd_is_playing(context->device)) {
                bt_source_srv_call_psd_audio_replay_req_t audio_replay_req = {
                    .type = g_source_srv_play_type_table[audio_port],
                    .port_action = BT_SOURCE_SRV_CALL_PSD_CALL_ACTION_UPDATE,
                };
                bt_source_srv_call_psd_event_notify(context->device, BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_REPLAY_REQ, &audio_replay_req);
            }
        }
        break;
    }
    return BT_STATUS_SUCCESS;
}


bt_status_t bt_source_srv_hfp_transfer_call_state(bt_source_srv_call_state_t previous_state, bt_source_srv_call_state_t new_state)
{
    bt_status_t status = BT_STATUS_FAIL;
    LOG_MSGID_I(source_srv, "[HFP][AG] transfer call state previous state = %02x, new state = %02x", 2, previous_state, new_state);

    /* Check whether the HFP is connected */
    bt_source_srv_hfp_context_t *highlight_context = bt_source_srv_hfp_get_highlight_device();
    if (highlight_context == NULL) {
        LOG_MSGID_W(source_srv, "[HFP][AG] transfer call state highlight is NULL, SCL call flag = %02x", 1, g_hfp_common_context.common_flags);
        if (g_hfp_common_context.common_flags & BT_SOURCE_SRV_HFP_FLAG_SLC_GET_CALL_INFO_COMPLETE) {
            g_hfp_common_context.common_flags &= ~BT_SOURCE_SRV_HFP_FLAG_SLC_GET_CALL_INFO_COMPLETE;
        }
        return BT_STATUS_SUCCESS;
    }

    status = bt_source_srv_hfp_transfer_call_state_int(previous_state, new_state);

    bt_source_srv_hfp_trigger_action_with_call_state_change(highlight_context, previous_state, new_state);

    status = bt_source_srv_hfp_audio_transfer_with_call(highlight_context, new_state, status);
    LOG_MSGID_I(source_srv, "[HFP][AG] transfer call state status = %02x", 1, status);
    return status;
}

void bt_source_srv_hfp_event_notify(bt_source_srv_event_t event_id, void *parameter, uint32_t length)
{
    if (bt_source_srv_hfp_get_bqb_flag() & BT_SOURCE_SRV_HFP_BQB_FLAG_TWC) {
        bt_source_srv_bqb_event_callback(event_id, parameter, length);
        return;
    }

    bt_source_srv_event_callback(event_id, parameter, length);
}

bt_status_t bt_source_srv_hfp_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint32_t module = (action & BT_SOURCE_MODULE_MASK);
    switch (module) {
        case BT_SOURCE_MODULE_CALL: {
            uint8_t action_index = (uint8_t)(action & BT_SOURCE_SRV_HFP_ACTION_MASK);
            if (action_index > (sizeof(g_handle_action_table) >> 2)) {
                return status;
            }
            status = g_handle_action_table[action_index](parameter, length);
        }
        break;
        case BT_SOURCE_MODULE_COMMON: {
            uint8_t action_index = (uint8_t)(action - BT_SOURCE_SRV_ACTION_MUTE);
            if (action_index > (sizeof(g_handle_common_action_table) >> 2)) {
                return status;
            }
            status = g_handle_common_action_table[action_index](parameter, length);
        };
        default:
            break;
    }
    return ((status == BT_STATUS_OUT_OF_MEMORY) ? BT_STATUS_SUCCESS : status);
}



bt_status_t bt_source_srv_hfp_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    LOG_MSGID_I(source_srv, "[HFP][AG] common event = %02x, status = %02x", 2, msg, status);
    for (uint32_t i = 0; i < (sizeof(g_handle_common_event_table) >> 3); i++) {
        if (g_handle_common_event_table[i].msg == msg) {
            return g_handle_common_event_table[i].handler(msg, status, buffer);
        }
    }
    return BT_STATUS_FAIL;
}

bt_status_t bt_source_srv_hfp_init(bt_source_srv_hfp_init_parameter_t *hfp_init_param)
{
    g_hfp_common_context.battery_level = hfp_init_param->battery_level;
#ifdef MTK_BT_CM_SUPPORT
    bt_cm_profile_service_register(BT_CM_PROFILE_SERVICE_HFP_AG, bt_source_srv_hfp_cm_callback);
#endif
    /* enable HFP AG SDP record */
    bt_hfp_enable_ag_service_record(true);
    bt_hfp_enable_service_record(false);
    bt_hsp_enable_service_record(false);
    return BT_STATUS_SUCCESS;
}

/* Implement the HFP init parameter function */
bt_status_t bt_hfp_ag_get_init_params(bt_hfp_ag_init_param_t *init_params)
{
    bt_source_srv_assert(init_params && "HFP init param is NULL");
    bt_source_srv_hfp_feature_config_t hfp_feature_config = {0};
    if (bt_source_srv_get_feature_config(BT_SOURCE_SRV_TYPE_HFP, &hfp_feature_config) != BT_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[HFP][AG] get feature config fail", 0);
        return BT_STATUS_FAIL;
    }

    init_params->support_features = hfp_feature_config.feature;
    init_params->support_hold_feature = hfp_feature_config.hold_feature;
    bt_source_srv_phone_card_info_t phone_crad_info = {0};
    if (bt_source_srv_get_phone_card_information(BT_SOURCE_SRV_TYPE_HFP, &phone_crad_info, 1) == 0) {
        LOG_MSGID_E(source_srv, "[HFP][AG] get phone crad information fail", 0);
        return BT_STATUS_FAIL;
    }

    if (init_params->call_indicator_value == BT_HFP_CALL_STATUS_UNKNOWN) {
        g_hfp_common_context.common_flags |= BT_SOURCE_SRV_HFP_FLAG_SLC_GET_CALL_INFO_COMPLETE;
        LOG_MSGID_I(source_srv, "[HFP][AG] get call information init params", 0);
    }

    init_params->support_codec = hfp_feature_config.codec_type;

    bt_source_srv_hfp_call_transfer_info_t transfer_info = {0};

    bt_source_srv_hfp_call_context_t call_context[BT_SOURCE_SRV_HFP_CALL_MAX] = {{0}};
    uint32_t real_call_count = bt_source_srv_hfp_call_get_call_list(call_context, BT_SOURCE_SRV_HFP_CALL_MAX);
    for (uint32_t i = 0; i < real_call_count; i++) {
        g_get_call_transfer_info_handle[BT_SOURCE_SRV_CALL_STATE_NONE](call_context[i].call_state, &transfer_info);
    }

    init_params->service_indicator_value = phone_crad_info.service_ability;
    init_params->call_indicator_value = transfer_info.call_status;
    init_params->call_setup_indicator_value = transfer_info.callsetup_status;
    init_params->call_held_indicator_value = transfer_info.held_status;
    init_params->signal_indicator_value = phone_crad_info.signal_strength;
    init_params->roam_indicator_value = phone_crad_info.roaming_state;
    init_params->battery_indicator_value = g_hfp_common_context.battery_level / 20;

    LOG_MSGID_I(source_srv, "[HFP][AG] get init phone card service ability = %02x, roaming state = %02x, battery level = %02x, signal strength = %02x", 4,
                phone_crad_info.service_ability, phone_crad_info.roaming_state, g_hfp_common_context.battery_level, phone_crad_info.signal_strength);
    LOG_MSGID_I(source_srv, "[HFP][AG] get init call state = %02x, call setup = %02x, held status = %02x", 3,
                transfer_info.call_status, transfer_info.callsetup_status, transfer_info.held_status);
    return BT_STATUS_SUCCESS;
}

bt_source_srv_hfp_codec_t bt_source_srv_hfp_get_playing_device_codec(void)
{
    bt_source_srv_hfp_context_t *context = bt_source_srv_hfp_get_highlight_device();
    if (NULL == context) {
        return BT_SOURCE_SRV_HFP_CODEC_TYPE_NONE;
    }
    return bt_source_srv_call_psd_get_codec_type(context->device);
}

/* for AG BQB test use */
bt_status_t bt_source_srv_hfp_set_bqb_flag(bt_source_srv_hfp_bqb_flag_t flag)
{
    g_hfp_common_context.bqb_flag |= flag;
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_srv_hfp_remove_bqb_flag(bt_source_srv_hfp_bqb_flag_t flag)
{
    g_hfp_common_context.bqb_flag &= ~flag;
    return BT_STATUS_SUCCESS;
}

bt_source_srv_hfp_bqb_flag_t bt_source_srv_hfp_get_bqb_flag(void)
{
    return g_hfp_common_context.bqb_flag;
}

uint32_t bt_hfp_get_bqb_custom_config(uint32_t handle)
{
    if (bt_source_srv_hfp_get_bqb_flag() & BT_SOURCE_SRV_HFP_BQB_FLAG_CODEC_CONNECTION) {
        return (uint32_t)BT_SOURCE_SRV_HFP_BQB_FLAG_CODEC_CONNECTION;
    }
    return 0;
}
