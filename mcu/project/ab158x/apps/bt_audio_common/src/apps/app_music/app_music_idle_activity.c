/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

/**
 * File: app_music_idle_activity.c
 *
 * Description:
 * This file is the activity to handle the key action or sink service event when music is not playing.
 * When the music starts playing, This activity will start the app_music_activity. In addition, this
 * activity manage the partner connection status, music mixing mode, and data synchronization during RHO.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */


#include "FreeRTOS.h"
#include "app_music_idle_activity.h"
#include "app_music_utils.h"
#include "app_music_activity.h"
#include "apps_events_interaction_event.h"
#include "bt_sink_srv_ami.h"
#include <stdlib.h>
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "bt_sink_srv.h"

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
#include "nvkey_dspalg.h"
#include "apps_aws_sync_event.h"
#include "bt_device_manager.h"
#include "apps_config_vp_index_list.h"
#include "voice_prompt_api.h"
#include "race_cmd.h"
#include "race_cmd_dsprealtime.h"
extern void race_mmi_set_peq_group_id(uint8_t peq_group_id, uint8_t *status, am_feature_type_t audio_path_id);
#endif

#include "apps_config_key_remapper.h"
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_utils.h"
#endif
#include "bt_role_handover.h"
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "bt_ull_service.h"
#endif


static apps_music_local_context_t s_app_music_context;  /* The variable records context */

static bool _proc_ui_shell_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len);
static bool _proc_key_event_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len);

#if defined(MTK_IN_EAR_FEATURE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)

typedef struct {
    uint8_t flag;
    uint8_t streaming_state;
} app_music_rho_data_t;

static uint8_t _role_handover_service_get_data_len_callback(const bt_bd_addr_t *addr)
{
    uint8_t data_len = 0;
#ifdef AIR_MULTI_POINT_ENABLE
    if (NULL == addr)
#endif
    {
        data_len = sizeof(app_music_rho_data_t);
    }
    return data_len;
}

static bt_status_t _role_handover_get_data_callback(const bt_bd_addr_t *addr, void *data)
{
#ifdef AIR_MULTI_POINT_ENABLE
    if (NULL == addr)
#endif
    {
        app_music_rho_data_t rho_data = {};
        //uint8_t flag_data = 0;
        if (s_app_music_context.isAutoPaused) {
            rho_data.flag = rho_data.flag | 0x1;
        }

        if (s_app_music_context.music_playing) {
            rho_data.flag = rho_data.flag | 0x2;
        }
        //memcpy(&rho_data.streaming_state, &s_app_music_context.music_streaming_state, sizeof(uint8_t));
        rho_data.streaming_state = s_app_music_context.music_streaming_state;
        memcpy(data, &rho_data, sizeof(app_music_rho_data_t));
        APPS_LOG_MSGID_I(APP_MUSIC_TAG" [RHO] get_data_callback isAutoPaused: %d", 1, s_app_music_context.isAutoPaused);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t _role_handover_update_data_callback(bt_role_handover_update_info_t *info)
{
    /* When RHO start, the data of the agent needs to be synchronized to the partner. */
#ifdef AIR_MULTI_POINT_ENABLE
    if (NULL == info->addr)
#endif
    {
        if (info->role == BT_AWS_MCE_ROLE_PARTNER) {
            app_music_rho_data_t data = *(app_music_rho_data_t *)info->data;
            bool isAutoPaused = (data.flag & 0x1) == 0x1 ? true : false;
            bool music_playing = (data.flag & 0x2) == 0x2 ? true : false;
            s_app_music_context.music_streaming_state = data.streaming_state;
            APPS_LOG_MSGID_I(APP_MUSIC_TAG" [RHO] update_data_callback, recv role=0x%02X,old[isAutoPaused=%d:isPlaying=%d], new[isAutoPaused=%d:isPlaying=%d], streaming_state=0x%x",
                             6, info->role, s_app_music_context.isAutoPaused, s_app_music_context.music_playing,
                             isAutoPaused, music_playing, data.streaming_state);
            if (s_app_music_context.isAutoPaused != isAutoPaused) {
                s_app_music_context.isAutoPaused = isAutoPaused;
            }
            if (s_app_music_context.music_playing != music_playing) {
                s_app_music_context.music_playing = music_playing;
            }
        }
    }

    return BT_STATUS_SUCCESS;
}

static void _role_handover_service_status_callback(const bt_bd_addr_t *addr,
                                                   bt_aws_mce_role_t role,
                                                   bt_role_handover_event_t event,
                                                   bt_status_t status)
{
    if (BT_ROLE_HANDOVER_COMPLETE_IND == event) {
        APPS_LOG_MSGID_I(APP_MUSIC_TAG" [RHO] service_status_callback: status=%d, role=0x%x",
                         2, status, role);
        if (BT_STATUS_SUCCESS == status) {
            /* always set partner connecting flag to true while rho success. */
            s_app_music_context.isPartnerConnected = true;
#ifdef MTK_IN_EAR_FEATURE_ENABLE
            if (role == BT_AWS_MCE_ROLE_AGENT) {
                am_dynamic_change_channel(AUDIO_CHANNEL_SELECTION_STEREO);
                s_app_music_context.currMixState = MUSIC_STEREO;
            }
#endif
        }
    }
}

#endif

static bool _proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            //APPS_LOG_MSGID_I(APP_MUSIC_TAG" create current activity : 0x%x", 1, (uint32_t)self);
            self->local_context = &s_app_music_context;
            ((apps_music_local_context_t *)self->local_context)->music_playing = false;
#if defined(MTK_AWS_MCE_ENABLE) && !defined(AIR_SPEAKER_ENABLE)
            ((apps_music_local_context_t *)self->local_context)->isPartnerCharging = false;
            ((apps_music_local_context_t *)self->local_context)->isPartnerConnected = false;
            ((apps_music_local_context_t *)self->local_context)->isSame = true;
            ((apps_music_local_context_t *)self->local_context)->currMixState = MUSIC_MONO;
            am_dynamic_change_channel(AUDIO_CHANNEL_SELECTION_MONO);
#endif
#if defined(AIR_SPEAKER_ENABLE)
            ((apps_music_local_context_t *)self->local_context)->currMixState = MUSIC_STEREO;
            am_dynamic_change_channel(AUDIO_CHANNEL_SELECTION_STEREO);
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
            race_dsprt_peq_change_sound_mode(1);
#endif
#endif

#ifdef MTK_IN_EAR_FEATURE_ENABLE
            ((apps_music_local_context_t *)self->local_context)->isAutoPaused = false;
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
            bt_role_handover_callbacks_t role_callbacks = {
                NULL,
                _role_handover_service_get_data_len_callback,
                _role_handover_get_data_callback,
                _role_handover_update_data_callback,
                _role_handover_service_status_callback
            };
            bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_MUSIC, &role_callbacks);
#endif
#endif /* MTK_IN_EAR_FEATURE_ENABLE */
            ((apps_music_local_context_t *)self->local_context)->avrcp_op_sta = AVRCP_OPERATION_STA_IDLE;
            break;
        }
        default:
            break;
    }
    return ret;
}

/**
 * @brief Do PEQ switch operation
 */
static void app_music_idle_activity_handle_peq_switch()
{
#if (defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)) && defined(AIR_RACE_CMD_ENABLE)
    voice_prompt_param_t vp = {0};
    vp.vp_index    = VP_INDEX_DOUBLE;
    vp.control     = VOICE_PROMPT_CONTROL_MASK_SYNC;
    vp.delay_time  = 200;
    voice_prompt_play(&vp, NULL);

#ifdef MTK_AWS_MCE_ENABLE
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
        bt_status_t status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_KEY, KEY_AUDIO_PEQ_SWITCH, NULL, 0);
        if (BT_STATUS_SUCCESS != status) {
            voice_prompt_play_sync_vp_failed();
        }
        return;
    }
#endif /* MTK_AWS_MCE_ENABLE */

    uint8_t status = aud_peq_get_peq_status(PEQ_AUDIO_PATH_A2DP, 0);
    uint8_t current_peq_id = aud_peq_get_current_sound_mode(PEQ_AUDIO_PATH_A2DP, 0);
    int32_t total_peq_num = aud_peq_get_total_mode(PEQ_AUDIO_PATH_A2DP, 0);

    APPS_LOG_MSGID_I(APP_MUSIC_TAG" app_music_idle_activity_handle_peq_switch, get peq: status=%d, total_num=%d, current_ID=%d",
                     3, status, total_peq_num, current_peq_id);

    if (status == 0 || (current_peq_id > total_peq_num)) {
        voice_prompt_play_sync_vp_failed();
        return;
    }

    uint8_t next_peq_id = 0;
    if (current_peq_id < total_peq_num) {
        next_peq_id = (current_peq_id + 1);
    } else if (current_peq_id == total_peq_num) {
        next_peq_id = 1;
    }

    race_mmi_set_peq_group_id(next_peq_id, &status, AM_A2DP_PEQ);

    APPS_LOG_MSGID_I(APP_MUSIC_TAG" app_music_idle_activity_handle_peq_switch, configure next peq ID : %d, result : %d (SUCCESS on 0)",
                     2, next_peq_id, status);

    if (status == RACE_ERRCODE_SUCCESS) {
        voice_prompt_play_sync_vp_succeed();
    } else {
        voice_prompt_play_sync_vp_failed();
    }
#endif /* PEQ_ENABLE || LINE_IN_PEQ_ENABLE */
}

#if defined(AIR_SPEAKER_ENABLE)
static uint8_t s_app_music_bis_audio_channel = 0;

static void app_music_switch_audio_channel_via_key(void)
{
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
    if (s_app_music_bis_audio_channel < 3)
#else
    if (s_app_music_bis_audio_channel < 2)
#endif
    {
        s_app_music_bis_audio_channel += 1;
    } else {
        s_app_music_bis_audio_channel = 0;
    }
    audio_channel_selection_t temp_audio_channel = AUDIO_CHANNEL_SELECTION_STEREO;
    uint8_t temp_peq_mode = 1;
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
    //uint8_t peq_status = 1;
#endif
    switch (s_app_music_bis_audio_channel) {
        case 0: {
            temp_audio_channel = AUDIO_CHANNEL_SELECTION_STEREO;
            temp_peq_mode = 1;
            break;
        }
        case 1: {
            temp_audio_channel = AUDIO_CHANNEL_SELECTION_BOTH_L;
            temp_peq_mode = 1;
            break;
        }
        case 2: {
            temp_audio_channel = AUDIO_CHANNEL_SELECTION_BOTH_R;
            temp_peq_mode = 1;
            break;
        }
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
        case 3: { //subwoofer mode
            temp_audio_channel = AUDIO_CHANNEL_SELECTION_STEREO;
            temp_peq_mode = 3;
            break;
        }
#endif
        default:
            break;
    }
    APPS_LOG_MSGID_I(APP_MUSIC_TAG" switch audio channel: %d, PEQ mode: %d", 2, temp_audio_channel, temp_peq_mode);
    am_dynamic_change_channel(temp_audio_channel);
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
    //race_mmi_set_peq_group_id(temp_peq_mode, &peq_status, AM_A2DP_PEQ);
    race_dsprt_peq_change_sound_mode(temp_peq_mode);
#endif
}
#endif

static bool _proc_key_event_group(ui_shell_activity_t *self,
                                  uint32_t event_id,
                                  void *extra_data,
                                  size_t data_len)
{
    bool ret = false;
    apps_config_key_action_t action = KEY_ACTION_INVALID;
    apps_music_local_context_t *local_ctx = (apps_music_local_context_t *)self->local_context;

    if (*(uint16_t *)extra_data == KEY_AUDIO_PEQ_SWITCH) {
        app_music_idle_activity_handle_peq_switch();
        return true;
    }

#if defined(AIR_SPEAKER_ENABLE)
    if (*(uint16_t *)extra_data == KEY_AUDIO_CHANNEL_SWITCH) {
        app_music_switch_audio_channel_via_key();
        return true;
    }
#endif

    if (local_ctx) {
        action = app_music_utils_proc_key_events(self, event_id, extra_data, data_len);
    }

    if (action != KEY_ACTION_INVALID) {
        ret = true;
    }

    return ret;
}

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE

bool app_music_idle_proc_ull_events(ui_shell_activity_t *self,
                                    uint32_t event_id,
                                    void *extra_data,
                                    size_t data_len)
{
    bool ret = false;

    apps_music_local_context_t *local_ctx = (apps_music_local_context_t *)self->local_context;
    if (local_ctx == NULL) {
        return ret;
    }
    switch (event_id) {
        case BT_ULL_EVENT_LE_STREAMING_START_IND: {
            app_music_update_ull2_link_state(event_id, (bt_ull_le_streaming_start_ind_t *)extra_data);
            if (!local_ctx->music_playing
                && (true == app_music_ull2_uplink_enable || true == app_music_ull2_downlink_enable)) {
                local_ctx->music_streaming_state |= APP_MUSIC_STEAMING_STATE_ULL_STATE;
                local_ctx->isAutoPaused = false;
                local_ctx->music_playing = true;
                ui_shell_start_activity(self, app_music_activity_proc, ACTIVITY_PRIORITY_MIDDLE, local_ctx, 0);
                APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_idle_proc_ull_events-START_IND", 0);
            }
            break;
        }
        case BT_ULL_EVENT_LE_STREAMING_STOP_IND: {
            app_music_update_ull2_link_state(event_id, (bt_ull_le_streaming_start_ind_t *)extra_data);
            APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_idle_proc_ull_events-STOP_IND", 0);
            break;
        }
        case BT_ULL_EVENT_LE_DISCONNECTED: {
            app_music_clear_ull2_link_state();
            APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_proc_ull_events-ULL_LE_DISCONNECTED", 0);
            break;
        }
    }

    return ret;
}
#endif

bool app_music_idle_activity_proc(ui_shell_activity_t *self,
                                  uint32_t event_group,
                                  uint32_t event_id,
                                  void *extra_data,
                                  size_t data_len)
{
    bool ret = false;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_KEY: {
            /* Key event. */
            ret = _proc_key_event_group(self, event_id, extra_data, data_len);
            break;
        }
#ifdef AIR_ROTARY_ENCODER_ENABLE
        case EVENT_GROUP_UI_SHELL_ROTARY_ENCODER: {
            /* Rotary encoder event. */
            ret = app_music_proc_rotary_event_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_BT_SINK: {
            /* BT_SINK events, indicates the state of music. */
            ret = app_music_idle_proc_bt_sink_events(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER:
            /* The event come from bt connection manager. */
            ret = app_music_idle_proc_bt_cm_events(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            /* Interaction events. */
            ret = app_music_idle_proc_apps_internal_events(self, event_id, extra_data, data_len);
            break;
#if defined(MTK_AWS_MCE_ENABLE)
        case EVENT_GROUP_UI_SHELL_AWS_DATA: {

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
            /**
             * @brief Handle the PEQ switch key event from partner.
             */
            bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
            bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();

            if ((aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) && (role == BT_AWS_MCE_ROLE_AGENT)) {
                //uint32_t event_group;
                uint32_t action;
                void *p_extra_data = NULL;
                uint32_t extra_data_len = 0;

                apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &action, &p_extra_data, &extra_data_len);

                if (action == KEY_AUDIO_PEQ_SWITCH) {
                    app_music_idle_activity_handle_peq_switch();
                    return true;
                }
            }
#endif

            /* The event come from peer. */
            ret = app_music_idle_proc_aws_data_events(self, event_id, extra_data, data_len);
        }
        break;
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY: {
            ret = app_music_idle_proc_ull_events(self, event_id, extra_data, data_len);
            ret = false;
            break;
        }
#endif
        default:
            break;
    }
    return ret;
}

