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

#ifdef AIR_ADAPTIVE_EQ_ENABLE
#include "peq_setting.h"
#include "app_adaptive_eq_idle_activity.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_aws_sync_event.h"
#ifdef MTK_ANC_ENABLE
#include "app_anc_service.h"
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_le_audio_bis.h"
#endif

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_control.h"
#endif
#include "audio_src_srv.h"
#include "audio_src_srv_resource_manager_config.h"
#include "audio_nvdm_common.h"

#include "bt_system.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_ami.h"
#include "bt_connection_manager_adapt.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "bt_ull_service.h"
#endif

#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le_cap_audio_manager.h"
#include "bt_sink_srv_le_cap_stream.h"
#include "bt_gap_le_service.h"
#endif


#ifdef HAL_AUDIO_MODULE_ENABLED
#include "hal_audio_message_struct.h"
#endif

#ifdef MTK_RACE_CMD_ENABLE
#include "race_event.h"
#include "race_cmd_dsprealtime.h"
#endif

#include "ui_shell_manager.h"

extern uint8_t g_adaptive_eq_golden_index;

//#define APP_AEQ_AUDIO_RESOURCE_MUTEX
#define AUD_ID_INVALID  -1
//#define APP_ADAPTIVE_EQ_TEST

#define LOG_TAG "[APP_ADAPTIVE_EQ][IDLE]"

typedef struct {
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    audio_transmitter_id_t                  transmit_id;                    /**< transmitter id */
#endif
    uint8_t                                 race_channel_id;                /**< race channel id */
    bool                                    is_transmitter_start;           /**< transmitter is start or not */
    bool                                    is_request_transmitter_start;   /**< user request transmitter start or stop */
    bool                                    is_need_resume;                 /**< If need to resume the adaptive eq transmitter */
    audio_src_srv_resource_manager_handle_t *resource_handle;               /**< need to take resource before start transmitter */
} app_adaptive_eq_context_t;

static app_adaptive_eq_context_t s_app_aeq_context;

#ifdef APP_ADAPTIVE_EQ_TEST
#include "atci.h"
static atci_status_t app_aeq_atci_handler(atci_parse_cmd_param_t *parse_cmd);
static atci_cmd_hdlr_item_t app_aeq_atci_cmd[] = {
    {
        .command_head = "AT+AEQ",
        .command_hdlr = app_aeq_atci_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};
#endif

int16_t s_agent_adaptive_eq_index = 1;
int16_t s_partner_adaptive_eq_index = 1;
extern uint8_t adaptive_eq_channel_id;

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
static void app_events_audio_event_adaptive_eq_notify_callback(int16_t eq_index)
{
#ifdef AIR_TWS_ENABLE
    int16_t partner_adaptive_eq_index = eq_index;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
        if (role == BT_AWS_MCE_ROLE_PARTNER) {
            APPS_LOG_MSGID_I(LOG_TAG"Received partner adaptive eq index %d -> %d", 2, s_partner_adaptive_eq_index, eq_index);
            s_partner_adaptive_eq_index = eq_index;
            partner_adaptive_eq_index = eq_index;
            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                           APPS_EVENTS_INTERACTION_SYNC_AEQ_INDEX_TO_AGENT,
                                           (void *)&partner_adaptive_eq_index, sizeof(int16_t));
            APPS_LOG_MSGID_I(LOG_TAG" race_control[%02x] sync to agent", 1, role);
        }
    } else
#endif
    {
        APPS_LOG_MSGID_I(LOG_TAG"Received agent adaptive eq index %d -> %d,adaptive_eq_channel_id:%d", 3, s_agent_adaptive_eq_index, eq_index, adaptive_eq_channel_id);
        s_agent_adaptive_eq_index = eq_index;
        if (adaptive_eq_channel_id != 0) {
            race_dsprealtime_adaptive_eq_index_response(s_agent_adaptive_eq_index, s_partner_adaptive_eq_index, adaptive_eq_channel_id);
        }
    }
}
#endif

#ifdef APP_AEQ_AUDIO_RESOURCE_MUTEX
static void app_adaptive_eq_audio_resource_callback(audio_src_srv_resource_manager_handle_t *handle, audio_src_srv_resource_manager_event_t event)
{
    APPS_LOG_MSGID_I(LOG_TAG" audio_resource_callback: event=0x%x, handle=0x%x, eq_handle=0x%x.",
                     3, event, handle, s_app_aeq_context.resource_handle);

    if (handle != s_app_aeq_context.resource_handle) {
        return;
    }

    switch (event) {
        case AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS: {
            app_adaptive_eq_start_audio_transmitter();
            break;
        }

        case AUDIO_SRC_SRV_EVENT_TAKE_REJECT: {
            audio_src_srv_resource_manager_add_waiting_list(s_app_aeq_context.resource_handle);
            break;
        }
        default: {
            break;
        }
    }
}

static void app_adaptive_eq_audio_resource_init(void)
{
    s_app_aeq_context.resource_handle  = audio_src_srv_resource_manager_construct_handle(
                                             AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_ADAPTIVE, AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_ADAPTIVE_USER_EQ);
    if (s_app_aeq_context.resource_handle != NULL) {
        s_app_aeq_context.resource_handle->callback_func = app_adaptive_eq_audio_resource_callback;
        s_app_aeq_context.resource_handle->priority = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_ADAPTIVE_USER_EQ_PRIORITY;
    }
    APPS_LOG_MSGID_I(LOG_TAG" audio_resource_init state=%d.", 1, (s_app_aeq_context.resource_handle != NULL) ? true : false);
}
#endif //APP_AEQ_MUTEX_AUDIO

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
static bool app_aeq_check_ull_streaming_status(void)
{
    bt_ull_streaming_info_t info = {0};
    bt_status_t ret = BT_STATUS_FAIL;
    bt_status_t ret2 = BT_STATUS_FAIL;
    bt_ull_streaming_t streaming = {
        .streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER,
        .port = 0,
    };
    bt_ull_streaming_t streaming2 = {
        .streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER,
        .port = 1,
    };
    ret  = bt_ull_get_streaming_info(streaming, &info);
    ret2 = bt_ull_get_streaming_info(streaming2, &info);

    if (BT_STATUS_SUCCESS == ret || BT_STATUS_SUCCESS == ret2) {
        return info.is_playing;
    }
    return FALSE;
}
#endif

static bool app_aeq_get_current_streaming_state(void)
{
    bool is_bis_streaming = false;
    bool is_ull_streaming = false;
    bt_sink_srv_state_t bt_sink_state = bt_sink_srv_get_state();
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    is_bis_streaming = app_le_audio_bis_is_streaming();
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    if (app_aeq_check_ull_streaming_status()) {
        is_ull_streaming = true;
    }
#endif

    return (bt_sink_state == BT_SINK_SRV_STATE_STREAMING || is_bis_streaming || is_ull_streaming);
}

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
static audio_transmitter_status_t app_adaptive_eq_start_audio_transmitter(void)
{
    audio_transmitter_status_t status = AUDIO_TRANSMITTER_STATUS_SUCCESS;
    aeq_detect_status_t aeq_detect_status = RACE_AEQ_DETECT_STATUS_UNKNOWN;
    aeq_detect_status = app_aeq_get_detect_status();

    if (AUD_ID_INVALID == s_app_aeq_context.transmit_id) {
        return AUDIO_TRANSMITTER_STATUS_FAIL;
    }

    if (aeq_detect_status == RACE_AEQ_DETECT_STATUS_NOT_RUNNING) {
        status = audio_transmitter_start(s_app_aeq_context.transmit_id);
        if (status == AUDIO_TRANSMITTER_STATUS_SUCCESS) {
            s_app_aeq_context.is_request_transmitter_start = true;
        }
    } else {
        if (s_app_aeq_context.race_channel_id != 0) {
            race_dsprealtime_adaptive_eq_detect_status_response(false, false, aeq_detect_status, s_app_aeq_context.race_channel_id);
        }
    }

#if 0
    if (!s_app_aeq_context.is_transmitter_start && eq_status && (aeq_detect_status == 1) && is_streaming
#ifdef MTK_ANC_ENABLE
        && !anc_enable
#endif
       ) {
        status = audio_transmitter_start(s_app_aeq_context.transmit_id);
        if (status == AUDIO_TRANSMITTER_STATUS_SUCCESS) {
            s_app_aeq_context.is_request_transmitter_start = true;
        }
    }
#endif
    APPS_LOG_MSGID_I(LOG_TAG" start_audio_transmitter: status=0x%02x, id=%d.", 2, status, s_app_aeq_context.transmit_id);

    return status;
}


static audio_transmitter_status_t app_adaptive_eq_stop_audio_transmitter(void)
{
    audio_transmitter_status_t status = AUDIO_TRANSMITTER_STATUS_SUCCESS;

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_AEQ_START_TIMER);
    if (AUD_ID_INVALID != s_app_aeq_context.transmit_id) {
        s_app_aeq_context.is_request_transmitter_start = false;

        if (s_app_aeq_context.is_transmitter_start) {
            status = audio_transmitter_stop(s_app_aeq_context.transmit_id);
        }
    }

    APPS_LOG_MSGID_I(LOG_TAG" stop_audio_transmitter: status=0x%x, id=%d.", 2, status, s_app_aeq_context.transmit_id);

    return status;
}

static void app_adaptive_eq_audio_transmitter_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    APPS_LOG_MSGID_I(LOG_TAG" audio_transmitter_callback: is_request_transmitter_start=%d.", 1, s_app_aeq_context.is_request_transmitter_start);
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            s_app_aeq_context.is_transmitter_start = true;
            s_app_aeq_context.is_need_resume       = false;
            if (s_app_aeq_context.race_channel_id != 0) {
                race_dsprealtime_adaptive_eq_detect_status_response(false, s_app_aeq_context.is_transmitter_start, RACE_AEQ_DETECT_STATUS_RUNNING, s_app_aeq_context.race_channel_id);
            }
            if (AUD_ID_INVALID != s_app_aeq_context.transmit_id
                && false == s_app_aeq_context.is_request_transmitter_start) {
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_stop(s_app_aeq_context.transmit_id)) {
                    APPS_LOG_MSGID_W(LOG_TAG" audio_transmitter_callback: trans_id=0x%x stop fail.", 1, s_app_aeq_context.transmit_id);
                }
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL:
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            s_app_aeq_context.is_transmitter_start = false;
            if (s_app_aeq_context.race_channel_id != 0) {
                race_dsprealtime_adaptive_eq_detect_status_response(false, s_app_aeq_context.is_transmitter_start, RACE_AEQ_DETECT_STATUS_NOT_RUNNING, s_app_aeq_context.race_channel_id);
            }
            if (AUD_ID_INVALID != s_app_aeq_context.transmit_id
                && s_app_aeq_context.is_request_transmitter_start) {
                audio_transmitter_start(s_app_aeq_context.transmit_id);
            } else {
#ifdef APP_AEQ_AUDIO_RESOURCE_MUTEX
                audio_src_srv_resource_manager_give(s_app_aeq_context.resource_handle);
#endif
            }
            break;
        }
        default:
            break;
    }
}

static void app_adaptive_eq_audio_transmitter_init(void)
{
    audio_transmitter_config_t config;

    memset((void *)&config, 0, sizeof(audio_transmitter_config_t));
    config.scenario_type   = AUDIO_TRANSMITTER_ADAPTIVE_EQ_MONITOR_STREAM;
    config.scenario_sub_id = 0;
    config.msg_handler     = app_adaptive_eq_audio_transmitter_callback;
    config.user_data = NULL;

    if (AUD_ID_INVALID == s_app_aeq_context.transmit_id) {
        s_app_aeq_context.transmit_id = audio_transmitter_init(&config);
    }

    adaptive_eq_notify_callback_register(app_events_audio_event_adaptive_eq_notify_callback);

    APPS_LOG_MSGID_I(LOG_TAG" audio_transmitter_init: id=%d.", 1, s_app_aeq_context.transmit_id);
}
#endif //AIR_AUDIO_TRANSMITTER_ENABLE

static bool app_aeq_idle_proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            self->local_context = &s_app_aeq_context;
#ifdef APP_AEQ_AUDIO_RESOURCE_MUTEX
            app_adaptive_eq_audio_resource_init();
#endif
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
            s_app_aeq_context.transmit_id = AUD_ID_INVALID;
            app_adaptive_eq_audio_transmitter_init();
#endif

#ifdef APP_ADAPTIVE_EQ_TEST
            atci_status_t ret = atci_register_handler(app_aeq_atci_cmd, sizeof(app_aeq_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
            APPS_LOG_MSGID_I(LOG_TAG" init atci ret=%d", 1, ret);
#endif
            break;
        }
        default:
            break;
    }
    return ret;
}

static bool app_aeq_idle_proc_bt_sink_event_group(ui_shell_activity_t *self,
                                                  uint32_t event_id,
                                                  void *extra_data,
                                                  size_t data_len)
{
    app_adaptive_eq_context_t *local_context = (app_adaptive_eq_context_t *)self->local_context;

    if (event_id == BT_SINK_SRV_EVENT_STATE_CHANGE) {
        bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *)extra_data;

        APPS_LOG_MSGID_I(LOG_TAG" proc_bt_sink_event: param->pre=0x%x, param->now=0x%x, is_transmitter_start=%d",
                         3, param->previous, param->current, local_context->is_transmitter_start);

        if (param->current == BT_SINK_SRV_STATE_STREAMING) {
#ifdef APP_AEQ_AUDIO_RESOURCE_MUTEX
            audio_src_srv_resource_manager_take(local_context->resource_handle);
#elif defined(AIR_AUDIO_TRANSMITTER_ENABLE)
#ifdef AIR_LE_AUDIO_ENABLE
        bt_sink_srv_device_state_t curr_state = {0};
        if (bt_sink_srv_get_playing_device_state(&curr_state) == BT_STATUS_SUCCESS) {
            if (curr_state.type == BT_SINK_SRV_DEVICE_LE) {
                if (bt_sink_srv_cap_am_is_dsp_streaming_with_conversational_context()) {
                    APPS_LOG_MSGID_W(LOG_TAG" LEA conversation", 0);
                    return false;
                }
            }
        }
#endif
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_AEQ_START_TIMER, NULL, 0, NULL, 200);
#endif

        } else if (param->current != BT_SINK_SRV_STATE_STREAMING) {
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
            //app_adaptive_eq_stop_audio_transmitter();
#endif
        }
    }

    return false;
}

#ifdef MTK_ANC_ENABLE
static bool app_aeq_idle_proc_audio_anc_event_group(ui_shell_activity_t *self,
                                                    uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    bool ret = false;
    app_adaptive_eq_context_t *local_context = (app_adaptive_eq_context_t *)self->local_context;
    APPS_LOG_MSGID_I(LOG_TAG" proc_audio_anc_event: audio_anc_event=0x%x", 1, event_id);
    switch (event_id) {
        case AUDIO_ANC_CONTROL_EVENT_OFF:
        case AUDIO_ANC_CONTROL_EVENT_FORCE_OFF: {
            if (local_context->is_need_resume) {
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
                app_adaptive_eq_start_audio_transmitter();
#endif
            }
            break;
        }
        case AUDIO_ANC_CONTROL_EVENT_ON: {
            if (local_context->is_transmitter_start) {
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
                app_adaptive_eq_stop_audio_transmitter();
                local_context->is_need_resume = true;
#endif
            }
            break;
        }
        default:
            break;
    }
    return ret;
}
#endif

#ifdef MTK_RACE_CMD_ENABLE

static aeq_control_status_t app_aeq_race_control_aeq_feature(bt_aws_mce_role_t role, uint8_t action, uint8_t set_mode, uint32_t bt_clk)
{
    aeq_control_status_t status = RACE_AEQ_STATUS_SUCCESS;
    uint32_t change_result = AUD_EXECUTION_SUCCESS;

    if (action == RACE_AEQ_ENABLE) {
        change_result = race_dsprt_peq_change_mode_data(0, set_mode, bt_clk, action, 1, AM_AUDIO_AEQ);
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
        app_adaptive_eq_start_audio_transmitter();
#endif
    } else if (action == 0) {
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
        app_adaptive_eq_stop_audio_transmitter();
#endif
        change_result = race_dsprt_peq_change_mode_data(0, set_mode, bt_clk, action, PEQ_SOUND_MODE_FORCE_DRC, AM_AUDIO_AEQ);
    }

    if (change_result != AUD_EXECUTION_SUCCESS) {
        status = RACE_AEQ_STATUS_UNKNOWN_ERROR;
    }

    return status;
}

static aeq_control_status_t app_aeq_race_control_aeq_detect(bt_aws_mce_role_t role, uint8_t action, uint8_t set_mode, uint32_t bt_clk)
{
    aeq_control_status_t status = RACE_AEQ_STATUS_SUCCESS;
    uint8_t eq_status = aud_peq_get_peq_status(PEQ_AUDIO_PATH_ADAPTIVE_EQ, 0);
    uint32_t change_mode_ret = 0;


#ifdef MTK_ANC_ENABLE
    bool anc_enable = false;
#endif

    if (!eq_status) {
        status = RACE_AEQ_STATUS_AEQ_NOT_ON;
        return status;
    }

#ifdef MTK_ANC_ENABLE
    anc_enable = app_anc_service_is_enable();
    if (anc_enable) {
        status = RACE_AEQ_STATUS_ANC_ON;
        return status;
    }
#endif

    uint8_t aeq_detect_status = action;
    sysram_status_t status_nv;
    uint32_t length = sizeof(uint8_t);

    status_nv = flash_memory_write_nvdm_data(NVKEY_DSP_PARA_AEQ_MISC, &aeq_detect_status, length);
    if (status_nv != NVDM_STATUS_NAT_OK) {
        APPS_LOG_MSGID_I(LOG_TAG "aeq detect failed to write 0x%x from nvdm - err(%d)\r\n", 2, NVKEY_DSP_PARA_AEQ_MISC, status_nv);
        status = RACE_AEQ_STATUS_UNKNOWN_ERROR;
        return status;
    }

    audio_transmitter_status_t trans_status = AUDIO_TRANSMITTER_STATUS_SUCCESS;
    switch (action) {
        case RACE_AEQ_FREEZE: {
            trans_status = app_adaptive_eq_stop_audio_transmitter();
            break;
        }
        case RACE_AEQ_ENABLE: {
            trans_status = app_adaptive_eq_start_audio_transmitter();
            break;
        }
        case RACE_AEQ_DISABLE: {
            trans_status = app_adaptive_eq_stop_audio_transmitter();
            change_mode_ret = race_dsprt_peq_change_mode_data(0, set_mode, bt_clk, RACE_AEQ_ENABLE, g_adaptive_eq_golden_index, AM_AUDIO_AEQ);
            break;
        }
        default:
            break;
    }
    if (trans_status != AUDIO_TRANSMITTER_STATUS_SUCCESS || change_mode_ret != 0) {
        status = RACE_AEQ_STATUS_UNKNOWN_ERROR;
    }

    return status;
}

static void app_aeq_race_query_detection_status(uint8_t channel_id)
{
    aeq_detect_status_t aeq_detect_status = RACE_AEQ_DETECT_STATUS_UNKNOWN;
    aeq_detect_status = app_aeq_get_detect_status();

    race_dsprealtime_adaptive_eq_detect_status_response(true, s_app_aeq_context.is_transmitter_start, aeq_detect_status, channel_id);
}

static void app_aeq_race_control(aeq_control_param_t *param)
{
    bt_aws_mce_role_t role      = bt_connection_manager_device_local_info_get_aws_role();
    aeq_control_status_t status = RACE_AEQ_STATUS_SUCCESS;

    uint8_t type       = param->type;
    uint8_t action     = param->action;
    uint8_t set_mode   = param->setting_mode;
    uint32_t bt_clk    = param->target_bt_clk;
    s_app_aeq_context.race_channel_id = param->channel_id;
    APPS_LOG_MSGID_I(LOG_TAG" race_control[%02x]: type=0x%x, action=%d, set_mode=%d, bt_clk=0x%x, channel_id=%d",
                     6, role, type, action, set_mode, bt_clk, s_app_aeq_context.race_channel_id);

    if (type == PEQ_AEQ_DETECT_STATUS
#ifdef MTK_AWS_MCE_ENABLE
        && role == BT_AWS_MCE_ROLE_AGENT
#endif
       ) {
        app_aeq_race_query_detection_status(s_app_aeq_context.race_channel_id);
        return;
    }

#ifdef MTK_AWS_MCE_ENABLE
    if (role == BT_AWS_MCE_ROLE_AGENT && BT_AWS_MCE_SRV_LINK_NONE == bt_aws_mce_srv_get_link_type()) {
        APPS_LOG_MSGID_W(LOG_TAG" race_control[%02x]:  Agent not AWS Connected", 0);
        race_dsprealtime_adaptive_eq_control_response(RACE_AEQ_STATUS_AWS_NOT_CONNTECT, type, s_app_aeq_context.race_channel_id);
        return;
    }
#endif

    switch (type) {
        case PEQ_AEQ_CONTROL: {
            status = app_aeq_race_control_aeq_feature(role, action, set_mode, bt_clk);
            break;
        }
        case PEQ_AEQ_DETECT_CONTROL: {
            status = app_aeq_race_control_aeq_detect(role, action, set_mode, bt_clk);
            break;
        }
        default:
            break;
    }
#ifdef AIR_TWS_ENABLE
    if (role == BT_AWS_MCE_ROLE_AGENT) {
        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                       APPS_EVENTS_INTERACTION_SYNC_AEQ_CONTROL_TO_PARTNER,
                                       (void *)param, sizeof(aeq_control_param_t));
        APPS_LOG_MSGID_I(LOG_TAG" race_control[%02x] sync to partner", 1, role);
        race_dsprealtime_adaptive_eq_control_response(status, type, s_app_aeq_context.race_channel_id);
    }
#else
    race_dsprealtime_adaptive_eq_control_response(status, type, s_app_aeq_context.race_channel_id);
#endif
}

static bool app_aeq_idle_proc_race_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool anc_enable = false;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
#ifdef MTK_ANC_ENABLE
    anc_enable = app_anc_service_is_enable();
#endif
    uint8_t eq_status = aud_peq_get_peq_status(PEQ_AUDIO_PATH_A2DP, 0);
    APPS_LOG_MSGID_I(LOG_TAG" [ANC_GAIN_RACE event] [%02X] event_id=%d, anc_enable=%d, eq_status=%d",
                     4, role, event_id, anc_enable, eq_status);

    switch (event_id) {
        case RACE_EVENT_TYPE_AEQ_CONTROL: {
            aeq_control_param_t *param = (aeq_control_param_t *)extra_data;
            app_aeq_race_control(param);
            break;
        }
    }
    return true;
}

#endif

#ifdef MTK_AWS_MCE_ENABLE
static bool app_aeq_proc_aws_data_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;

    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group = 0;
        uint32_t action_id = 0;
        void    *extra = NULL;
        uint32_t extra_len = 0;

        apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &action_id, (void *)&extra, &extra_len);
        if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
            && action_id == APPS_EVENTS_INTERACTION_SYNC_AEQ_CONTROL_TO_PARTNER) {
            aeq_control_param_t param = *((aeq_control_param_t *)extra);
            app_aeq_race_control(&param);
            ret = TRUE;
        } else if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
                   && action_id == APPS_EVENTS_INTERACTION_SYNC_AEQ_INDEX_TO_AGENT) {
            s_partner_adaptive_eq_index = *((int16_t *)extra);
            if (adaptive_eq_channel_id != 0) {
                race_dsprealtime_adaptive_eq_index_response(s_agent_adaptive_eq_index, s_partner_adaptive_eq_index, adaptive_eq_channel_id);
            }
            ret = TRUE;
        }
    }
    return ret;
}
#endif

/**
* @brief      This function is used to handle the app internal events.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_aeq_proc_app_internal_events(ui_shell_activity_t *self,
                                             uint32_t event_id,
                                             void *extra_data,
                                             size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        /* Update LED display when the state of HFP updated. */
        case APPS_EVENTS_INTERACTION_AEQ_START_TIMER: {
            app_adaptive_eq_start_audio_transmitter();
            break;
        }
        default : {
            break;
        }
    }

    return ret;
}


bool app_adaptive_eq_idle_activity_proc(ui_shell_activity_t *self,
                                        uint32_t event_group,
                                        uint32_t event_id,
                                        void *extra_data,
                                        size_t data_len)
{
    bool ret = false;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = app_aeq_idle_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_SINK: {
            /* Bt sink srv event. */
            ret = app_aeq_idle_proc_bt_sink_event_group(self, event_id, extra_data, data_len);
            break;
        }
#ifdef MTK_ANC_ENABLE
        case EVENT_GROUP_UI_SHELL_AUDIO_ANC: {
            ret = app_aeq_idle_proc_audio_anc_event_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
#ifdef MTK_RACE_CMD_ENABLE
        case EVENT_GROUP_UI_SHELL_AEQ_RACE: {
            ret = app_aeq_idle_proc_race_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
#ifdef MTK_AWS_MCE_ENABLE
        case EVENT_GROUP_UI_SHELL_AWS_DATA: {
            ret = app_aeq_proc_aws_data_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            /* Events come from app interation. */
            ret = app_aeq_proc_app_internal_events(self, event_id, extra_data, data_len);
            break;
        }
        default:
            break;
    }
    return ret;
}

aeq_detect_status_t app_aeq_get_detect_status(void)
{
    aeq_detect_status_t aeq_detect_status = RACE_AEQ_DETECT_STATUS_UNKNOWN;
    uint8_t eq_status = aud_peq_get_peq_status(PEQ_AUDIO_PATH_ADAPTIVE_EQ, 0);
    uint8_t aeq_detect_nvdm = 0;
    bool anc_enable = false;
    bool streaming_status = app_aeq_get_current_streaming_state();

#ifdef MTK_ANC_ENABLE
    anc_enable = app_anc_service_is_enable();
#endif

    sysram_status_t status_nv;
    uint32_t length = sizeof(uint8_t);
    status_nv = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_AEQ_MISC, (uint8_t *)&aeq_detect_nvdm, &length);
    if (status_nv != NVDM_STATUS_NAT_OK) {
        APPS_LOG_MSGID_I(LOG_TAG " get_detect_status: read aeq detect nvkey-err(%d)", 1, status_nv);
    }
    if (!eq_status) {
        aeq_detect_status = RACE_AEQ_DETECT_STATUS_AEQ_NOT_ON;
    } else if ((aeq_detect_nvdm == 0) || (aeq_detect_nvdm == 2)) {
        aeq_detect_status = RACE_AEQ_DETECT_STATUS_DETECT_NOT_ON;
    } else if (!streaming_status) {
        aeq_detect_status =  RACE_AEQ_DETECT_STATUS_A2DP_NOT_ON;
    } else if (anc_enable) {
        aeq_detect_status =  RACE_AEQ_DETECT_STATUS_ANC_ON;
    } else if (s_app_aeq_context.is_transmitter_start) {
        aeq_detect_status =  RACE_AEQ_DETECT_STATUS_RUNNING;
    } else if (!s_app_aeq_context.is_transmitter_start) {
        aeq_detect_status =  RACE_AEQ_DETECT_STATUS_NOT_RUNNING;
    }
    APPS_LOG_MSGID_I(LOG_TAG " query_detection_status: eq_status=%d, detect_status=%d, streaming_status=%d, ANC_status=%d",
                     4, eq_status, aeq_detect_nvdm, streaming_status, anc_enable);
    return aeq_detect_status;
}

bool app_aeq_get_aeq_is_started(void)
{
    bool ret = false;

    if (s_app_aeq_context.is_transmitter_start) {
        ret = true;
    }

    APPS_LOG_MSGID_I(LOG_TAG" get_aeq_is_started: ret=%d", 1, ret);
    return ret;
}

/* Weak symbol*/
int32_t bt_sink_srv_stop_am_notify(audio_src_srv_pseudo_device_t device_type)
{
    APPS_LOG_MSGID_I(LOG_TAG" received stop notify: type=%d", 1, device_type);
    if (AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP || AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE) {
        app_adaptive_eq_stop_audio_transmitter();
    }
    return 0;
}

#ifdef APP_ADAPTIVE_EQ_TEST
static atci_status_t app_aeq_atci_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            char *atcmd = parse_cmd->string_ptr + parse_cmd->name_len + 1;
            char cmd[20] = {0};
            uint8_t copy_len = strlen(atcmd) - 2;
            memcpy(cmd, atcmd, (copy_len > 19 ? 19 : copy_len));
            APPS_LOG_I(LOG_TAG" AT CMD=%s", cmd);
            if (strstr(cmd, "set_eq") > 0) {
                aeq_control_param_t aeq_control = {0};
                aeq_control.type = PEQ_AEQ_CONTROL;
                sscanf(cmd, "set_eq,%d", &aeq_control.action);
                app_aeq_race_control(&aeq_control);
            } else if (strstr(cmd, "set_det") > 0) {
                aeq_control_param_t aeq_control = {0};
                aeq_control.type = PEQ_AEQ_DETECT_CONTROL;
                sscanf(cmd, "set_eq,%d", &aeq_control.action);
                app_aeq_race_control(&aeq_control);
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" invalid ANC_GAIN AT-CMD", 0);
            }
            memset(response.response_buf, 0, ATCI_UART_TX_FIFO_BUFFER_SIZE);
            snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "OK - %s\r\n", atcmd);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}
#endif

#endif
