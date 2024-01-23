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
 * File: app_music_activity.c
 *
 * Description:
 * This file is the activity to handle the key action or sink service event when music is playing.
 * When the music starts playing, the app_music_idle_activity start this activity to handle the a2dp
 * state changed events and UI events of music.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */


#include "app_music_activity.h"
#include "app_music_utils.h"
#include "apps_debug.h"
#include "bt_sink_srv.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"

#include "apps_config_key_remapper.h"
#include "apps_events_key_event.h"
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "bt_ull_service.h"
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_idle_activity.h"
#endif
#ifdef AIR_ROTARY_ENCODER_ENABLE
#include "bt_sink_srv_a2dp.h"
#endif

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
#include "app_rho_idle_activity.h"
#endif

#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio_aird_client.h"
#include "app_le_audio_bis.h"
#include "audio_src_srv.h"
#include "bt_sink_srv_le.h"
#include "bt_sink_srv_le_cap.h"
#include "bt_sink_srv_le_cap_stream.h"
#endif

#ifdef MTK_AWS_MCE_ENABLE
#include "apps_aws_sync_event.h"
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#endif
#ifdef AIR_MS_GIP_ENABLE
#include "app_dongle_service.h"
#endif

static bool _proc_ui_shell_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(APP_MUSIC_ACTI", create", 0);
            if (extra_data) {
                self->local_context = extra_data;
                apps_music_local_context_t *local_context = (apps_music_local_context_t *)self->local_context;
                local_context->avrcp_op_sta = AVRCP_OPERATION_STA_IDLE;
            }
            /* Trigger MMI updating. */
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                NULL, 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_MMI_STATE, NULL, 0,
                                NULL, 0);
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY: {
            APPS_LOG_MSGID_I(APP_MUSIC_ACTI", destroy", 0);
            /* This activity would be finished by self when the music stop playing, so trigger MMI updating. */
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                NULL, 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_MMI_STATE, NULL, 0,
                                NULL, 0);
            break;
        }
        default:
            break;
    }
    return ret;
}

static bool app_music_proc_key_event_group(ui_shell_activity_t *self,
                                           uint32_t event_id,
                                           void *extra_data,
                                           size_t data_len)
{
    bool ret = false;
    apps_config_key_action_t action = KEY_ACTION_INVALID;
    apps_music_local_context_t *local_ctx = (apps_music_local_context_t *)self->local_context;

    if (local_ctx) {
        action = app_music_utils_proc_key_events(self, event_id, extra_data, data_len);
    }

    if (action != KEY_ACTION_INVALID) {
        ret = true;
    }

    return ret;
}

static bool app_music_proc_bt_event_when_playing(ui_shell_activity_t *self,
                                                 uint32_t event_id,
                                                 void *extra_data,
                                                 size_t data_len)
{
    apps_music_local_context_t *local_context = (apps_music_local_context_t *)self->local_context;
    if (event_id == BT_SINK_SRV_EVENT_STATE_CHANGE) {
        bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *)extra_data;

        APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_proc_bt_event:param->pre=0x%x, param->now=0x%x, is playing=0x%x, isAutoPaused=%d",
                         4, param->previous, param->current, local_context->music_playing, local_context->isAutoPaused);

        /* Finish current activity when the a2dp streaming stops. */
        if ((param->previous == BT_SINK_SRV_STATE_STREAMING) && (param->current != BT_SINK_SRV_STATE_STREAMING)) {
//            local_context->music_playing = false;
//            local_context->isAutoPaused = false;
//            ui_shell_finish_activity(self, self);
            local_context->music_streaming_state &= ~APP_MUSIC_STEAMING_STATE_SINK_STATE;
        }
    }
#if defined(MTK_IN_EAR_FEATURE_ENABLE)
    else if (event_id == BT_SINK_SRV_EVENT_AVRCP_STATUS_CHANGE) {
        bt_sink_srv_event_param_t *event = (bt_sink_srv_event_param_t *)extra_data;
        bt_avrcp_status_t avrcp_status = event->avrcp_status_change.avrcp_status;
        APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_proc_bt_event: avrcp_status=%d", 1, avrcp_status);
        /* Finish current activity when the avrcp stopped or paused. */
        if (BT_AVRCP_STATUS_PLAY_PAUSED == avrcp_status || BT_AVRCP_STATUS_PLAY_STOPPED == avrcp_status) {
            app_music_remove_avrcp_status(&(event->avrcp_status_change.address));
            app_music_avrcp_status_t *local_avrcp_status = app_music_get_avrcp_status();
            if (local_avrcp_status->avrcp_num == 0) {
//                local_context->music_playing = false;
//                local_context->isAutoPaused = false;
//                ui_shell_finish_activity(self, self);
                local_context->music_streaming_state &= ~APP_MUSIC_STEAMING_STATE_AVRCP_STATE;
            }
        }
    }
#endif
    if ((event_id == BT_SINK_SRV_EVENT_STATE_CHANGE
        || event_id == BT_SINK_SRV_EVENT_AVRCP_STATUS_CHANGE)
        && local_context->music_streaming_state == 0x00) {
        local_context->music_playing = false;
        local_context->isAutoPaused = false;
        ui_shell_finish_activity(self, self);
    }
    return false;
}

#ifdef MTK_IN_EAR_FEATURE_ENABLE
/**
* @brief      This function is used to stop music when the earbud was taken out of the ear.
* @param[in]  self, the context pointer of the activity.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_music_check_and_end_music(struct _ui_shell_activity *self, void *extra_data)
{
    apps_music_local_context_t *ctx = (apps_music_local_context_t *)self->local_context;
    app_in_ear_sta_info_t *sta_info = (app_in_ear_sta_info_t *)extra_data;
    uint8_t temp_music_in_ear_config = app_music_get_in_ear_control_state();
    bool avrcp_is_playing = (ctx->music_streaming_state & APP_MUSIC_STEAMING_STATE_AVRCP_STATE) ? true : false;

    APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_check_and_end_music, cur=%d, pre=%d, isAutoPaused=%d, avrcp_is_playing=%d, in_ear_config=%d",
                     5, sta_info->current, sta_info->previous, ctx->isAutoPaused, avrcp_is_playing, temp_music_in_ear_config);


    /* Stop the music when at least one earbud is taken out of the ear. */
    if (sta_info->current != APP_IN_EAR_STA_BOTH_IN
        && sta_info->previous != APP_IN_EAR_STA_BOTH_OUT
        && !ctx->isAutoPaused
        && avrcp_is_playing) {
        if ((APP_MUSIC_IN_EAR_AUTO_PAUSE_RESUME == temp_music_in_ear_config)
            || (APP_MUSIC_IN_EAR_ONLY_AUTO_PAUSE == temp_music_in_ear_config)) {
            bt_status_t bt_status = bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PAUSE, NULL);
            if (bt_status == BT_STATUS_SUCCESS) {
                ctx->isAutoPaused = true;
                //ctx->music_playing = false;
                ui_shell_finish_activity(self, self);
                APPS_LOG_MSGID_I(APP_MUSIC_ACTI" auto pause music.", 0);
            }
        }
    }

    /*
     * while the phone paused music, the sink server will report this event with at least 3 seconds delay.
     * so, it's possible that this activity will receive the music playing request before the sink event.
     * In this case, may means that the old Agent send pause action and finished the shell activity, but
     * the new Agent not recv the SINK_SRV event about the music paused status.
     */
    if (sta_info->previous != APP_IN_EAR_STA_BOTH_IN && sta_info->current != APP_IN_EAR_STA_BOTH_OUT) {
        if (ctx->isAutoPaused) {
            if (APP_MUSIC_IN_EAR_AUTO_PAUSE_RESUME == temp_music_in_ear_config) {
                bt_status_t bt_status = app_music_send_actions_by_address(BT_SINK_SRV_ACTION_PLAY);
                if (bt_status == BT_STATUS_SUCCESS) {
                    ctx->isAutoPaused = false;
                    ctx->music_playing = true;
                }
            }
        }
    }
    return false;
}

#endif /*MTK_IN_EAR_FEATURE_ENABLE*/

static bool app_music_proc_apps_internal_events(ui_shell_activity_t *self,
                                                uint32_t event_id,
                                                void *extra_data,
                                                size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        case APPS_EVENTS_INTERACTION_UPDATE_MMI_STATE: {
         apps_music_local_context_t *local_context = (apps_music_local_context_t *)self->local_context;
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            if (app_music_get_ull_is_streaming()) {
#ifdef AIR_MS_GIP_ENABLE
                if (app_dongle_service_get_dongle_mode() != APP_DONGLE_SERVICE_DONGLE_MODE_XBOX) {
                    apps_config_key_set_mmi_state(APP_ULTRA_LOW_LATENCY_PLAYING);
                }
#else
                apps_config_key_set_mmi_state(APP_ULTRA_LOW_LATENCY_PLAYING);
#endif
                ret = true;
            } else
#endif
            {
                if (local_context->music_streaming_state != 0x00
#ifdef AIR_LE_AUDIO_BIS_ENABLE
                && !app_le_audio_bis_is_streaming()
#endif
                ) {
                    apps_config_key_set_mmi_state(APP_A2DP_PLAYING);
                    ret = true;
                }
            }
            APPS_LOG_MSGID_I(APP_MUSIC_ACTI" received mmi state, ret=%d", 1, ret);
        }
        break;

#ifdef MTK_IN_EAR_FEATURE_ENABLE
        case APPS_EVENTS_INTERACTION_IN_EAR_UPDATE_STA: {
            /* The event come from in ear detection app. */
            app_in_ear_sta_info_t *sta_info = (app_in_ear_sta_info_t *)extra_data;
            if (sta_info->previous != sta_info->current) {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                if (app_music_get_ull_is_streaming()) {
                    break;
                } else
#endif
                    ret = app_music_check_and_end_music(self, extra_data);
            }
            break;
        }
#endif
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
        case APPS_EVENTS_INTERACTION_RHO_END:
        case APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT: {
            apps_music_local_context_t *local_context = (apps_music_local_context_t *)self->local_context;
            app_rho_result_t rho_result = (app_rho_result_t)extra_data;
            APPS_LOG_MSGID_I(APP_MUSIC_ACTI" RHO done - rho_result=%d, music_playing=%d",
                             2, rho_result, local_context->music_playing);
            if (APP_RHO_RESULT_SUCCESS == rho_result) {
                if (!local_context->music_playing
#ifdef MTK_IN_EAR_FEATURE_ENABLE
                    && APP_IN_EAR_STA_BOTH_OUT == app_in_ear_get_state()
#endif
                   ) {
                    ui_shell_finish_activity(self, self);
                }
            }
            break;
        }
#endif
        default:
            break;
    }

    return ret;
}

#if defined(MTK_AWS_MCE_ENABLE) && defined(MTK_IN_EAR_FEATURE_ENABLE)
bool app_music_proc_aws_data_events(ui_shell_activity_t *self,
                                    uint32_t event_id,
                                    void *extra_data,
                                    size_t data_len)
{
    bool ret = false;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;

    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group;
        uint32_t action;
        void *p_extra_data = NULL;
        uint32_t extra_data_len = 0;

        apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &action,
                                         &p_extra_data, &extra_data_len);
        if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
            && action == APPS_EVENTS_INTERACTION_SYNC_BT_AVRCP_STATUS_TO_PEER) {
            bt_sink_srv_event_param_t *avrcp_data = (bt_sink_srv_event_param_t *)p_extra_data;
            if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role()) {
                app_music_proc_bt_event_when_playing(self, BT_SINK_SRV_EVENT_AVRCP_STATUS_CHANGE, avrcp_data, extra_data_len);
            }
        } else if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
                   && action == APPS_EVENTS_INTERACTION_SYNC_MUSIC_APP_STREAMING_STATE_TO_PEER) {
            app_music_streaming_state_t *music_streaming_state = (app_music_streaming_state_t *)p_extra_data;
            APPS_LOG_MSGID_I(APP_MUSIC_ACTI" aws_data_events, received peer music streaming state: 0x%x", 1, *music_streaming_state);
            if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role()) {
                apps_music_local_context_t *local_ctx = (apps_music_local_context_t *)self->local_context;
                local_ctx->music_streaming_state = *music_streaming_state;
                if (local_ctx->music_streaming_state == 0x00) {
                    local_ctx->music_playing = false;
                    local_ctx->isAutoPaused = false;
                    ui_shell_finish_activity(self, self);
                }
            }
        }
    }

    return ret;
}
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE

static void app_music_finish_music_activity(ui_shell_activity_t *self, apps_music_local_context_t *local_ctx)
{
    if (local_ctx && self) {
        local_ctx->music_playing = false;
        local_ctx->isAutoPaused = false;
        ui_shell_finish_activity(self, self);
    }
}


bool app_music_proc_ull_events(ui_shell_activity_t *self,
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
            // if (true == app_music_ull2_uplink_enable) {
            //     app_music_finish_music_activity(self, local_ctx);
            //     APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_idle_proc_ull_events-START_IND call end music", 0);
            // }
        }
        break;
        case BT_ULL_EVENT_LE_STREAMING_STOP_IND: {
            app_music_update_ull2_link_state(event_id, (bt_ull_le_streaming_start_ind_t *)extra_data);
            if (false == app_music_ull2_downlink_enable && false == app_music_ull2_uplink_enable) {
                local_ctx->music_streaming_state &= ~APP_MUSIC_STEAMING_STATE_ULL_STATE;
                app_music_finish_music_activity(self, local_ctx);
                APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_proc_ull_events-STOP_IND music finish", 0);
            }
            break;
        }
        case BT_ULL_EVENT_LE_DISCONNECTED: {
            //app_music_finish_music_activity(self,local_ctx);
            app_music_clear_ull2_link_state();
            APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_proc_ull_events-ULL_LE_DISCONNECTED end music", 0);
            break;
        }

    }
    return ret;
}
#endif

#if defined(MTK_IN_EAR_FEATURE_ENABLE)
bool app_music_proc_bt_cm_events(ui_shell_activity_t *self, uint32_t event_id,
                                 void *extra_data, size_t data_len)
{
    bool ret = false;

    apps_music_local_context_t *local_context = (apps_music_local_context_t *)self->local_context;

    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {

            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (NULL == local_context || NULL == remote_update) {
                break;
            }

            if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP) & remote_update->pre_connected_service)
                && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP) & remote_update->connected_service)) {
                APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_proc_bt_cm_event avrcp disconnect", 0);
                app_music_remove_avrcp_status(&(remote_update->address));
                app_music_avrcp_status_t *local_avrcp_status = app_music_get_avrcp_status();
                if (local_avrcp_status->avrcp_num == 0) {
                    local_context->music_streaming_state &= ~APP_MUSIC_STEAMING_STATE_AVRCP_STATE;
#ifdef MTK_AWS_MCE_ENABLE
                    if (BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()
                        && BT_AWS_MCE_ROLE_AGENT == bt_connection_manager_device_local_info_get_aws_role()) {
                        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                    APPS_EVENTS_INTERACTION_SYNC_MUSIC_APP_STREAMING_STATE_TO_PEER,
                                                    &local_context->music_streaming_state, sizeof(local_context->music_streaming_state));
                    }
#endif
                
                    if (local_context->music_streaming_state == 0x00) {
                        local_context->music_playing = false;
                        local_context->isAutoPaused = false;
                        ui_shell_finish_activity(self, self);
                    }
                }
            }
        }
        break;
        default:
            break;
    }
    return ret;
}
#endif

bool app_music_activity_proc(ui_shell_activity_t *self,
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
            ret = app_music_proc_key_event_group(self, event_id, extra_data, data_len);
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
            ret = app_music_proc_bt_event_when_playing(self, event_id, extra_data, data_len);
            break;
        }
#if defined(MTK_IN_EAR_FEATURE_ENABLE)
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            /* The event come from bt connection manager. */
            ret = app_music_proc_bt_cm_events(self, event_id, extra_data, data_len);
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            /* Interaction events. */
            ret = app_music_proc_apps_internal_events(self, event_id, extra_data, data_len);
            break;
        }
#if defined(MTK_AWS_MCE_ENABLE) && defined(MTK_IN_EAR_FEATURE_ENABLE)
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            /* The event come from partner. */
            ret = app_music_proc_aws_data_events(self, event_id, extra_data, data_len);
            break;
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY: {
            ret = app_music_proc_ull_events(self, event_id, extra_data, data_len);
            break;
        }
#endif
        default:
            break;
    }
    return ret;
}


