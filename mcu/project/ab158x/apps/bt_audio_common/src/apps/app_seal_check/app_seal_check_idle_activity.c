/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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
 * File: app_seal_check_idle_activity.c
 *
 * Description:
 * This file is the activity to play the VP of leakage detection. When leakage detection
 * starts, the activity will play the VP of leakage detection. And the algorithm in the
 * middleware will check the sealing state of the earbuds according to the playback
 * quality of this VP.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */

#ifdef MTK_LEAKAGE_DETECTION_ENABLE

#include "app_seal_check_idle_activity.h"
#include "app_seal_check_utils.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_config_vp_index_list.h"
#include "voice_prompt_api.h"
#include "bt_sink_srv.h"
#include "bt_aws_mce.h"
#include "bt_device_manager.h"
#include "apps_config_event_list.h"
#include "apps_debug.h"
#include "ui_shell_manager.h"
#include "bt_sink_srv_ami.h"
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le.h"
#endif
#include "leakage_detection_control.h"
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE_V2)
#include "app_advance_passthrough.h"
#endif
#ifdef MTK_AWS_MCE_ENABLE
#include "apps_aws_sync_event.h"
#endif
#ifdef MTK_ANC_ENABLE
#include "app_anc_service.h"
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_idle_activity.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "bt_ull_service.h"
#include "bt_ull_le_service.h"
#endif

#define SC_TAG "app_seal_check "

static app_leakage_detection_context_t s_app_leakage_detection_context;  /* The variable records context. */

static void app_leakage_detection_broadcast_ongoing_status(bool is_from_irq, bool is_ongoing)
{
    APPS_LOG_MSGID_I(SC_TAG" app_leakage_detection_broadcast_status: is_from_irq=%d, is_ongoing=%d.",
                     2, is_from_irq, is_ongoing);
    ui_shell_send_event(is_from_irq, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_STATUS, (void *)is_ongoing, 0, NULL, 0);
}

/**
* @brief      This function is callback from the middleware to trigger the vp of leakage detection.
* @param[in]  leakage_status, the leakage detection status.
* @return     None.
*/
static void app_leakage_detection_trigger_vp_callback(uint16_t leakage_status)
{
    bool *p_play_flag = NULL;

    s_app_leakage_detection_context.seal_checking = true;

    /* Play leakage detection VP with 1500ms delay. */
    p_play_flag = (bool *)pvPortMalloc(sizeof(bool));
    if (p_play_flag == NULL) {
        APPS_LOG_MSGID_E(SC_TAG" leakage_detection_trigger_vp_callback: malloc mem failed.", 0);
        return;
    }
    *p_play_flag = true;

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP_TRIGGER);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP);
    ui_shell_status_t status = ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                   APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP_TRIGGER,
                                                   p_play_flag, sizeof(bool), NULL, 200);
    if (UI_SHELL_STATUS_OK != status) {
        vPortFree(p_play_flag);
    } else {
        app_leakage_detection_broadcast_ongoing_status(false, true);
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE_V2)
        app_advance_passthrough_set_ld_ongoing(TRUE);
#endif
    }
}

static void app_leakage_detection_stop(bool is_vp_over)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP_TRIGGER);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP);
    if (APP_LEAKAGE_DETECTION_VP_INDEX == voice_prompt_get_current_index()) {
        voice_prompt_stop(APP_LEAKAGE_DETECTION_VP_INDEX, VOICE_PROMPT_ID_INVALID, true);
    }
    s_app_leakage_detection_context.vp_cnt = 0;
    s_app_leakage_detection_context.seal_checking = false;

#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE_V2)
    app_advance_passthrough_set_ld_ongoing(FALSE);
#endif
    if (!is_vp_over) {
        audio_anc_leakage_detection_stop();
    }
    app_leakage_detection_broadcast_ongoing_status(false, false);
}

static void app_leakage_detection_vp_callback(uint32_t idx, voice_prompt_event_t err)
{
    /* Stop the leakage detection when the VP of leakage detection is interrupted by others VP. */
    if (idx == APP_LEAKAGE_DETECTION_VP_INDEX && err == VP_EVENT_PREEMPTED) {
        APPS_LOG_MSGID_I(SC_TAG" vp stoped", 0);
        app_leakage_detection_stop(false);
    }
}

/**
* @brief      This function is called by the middleware at the the stopped of leakage detection.
* @param[in]  event_id, the event id of leakage detection.
* @param[in]  result, the result of leakage detection.
* @return     None.
*/
static void app_leakage_detection_event_callback(audio_anc_leakage_detection_control_event_t event_id,
                                                audio_anc_leakage_detection_execution_t result)
{
    bool     anc_enable = false;
    uint8_t  role = 0;
#ifdef MTK_ANC_ENABLE
    anc_enable = app_anc_service_is_enable();
#endif
#ifdef AIR_TWS_ENABLE
    role = bt_connection_manager_device_local_info_get_aws_role();
#endif
    APPS_LOG_MSGID_I(SC_TAG" leakage_detection_event_callback: role=0x%02x, event_id=0x%x, result=0x%x",
                     4, role, event_id, result, anc_enable);
    if (result !=  AUDIO_LEAKAGE_DETECTION_EXECUTION_SUCCESS) {
        return;
    }
    switch (event_id) {
        case AUDIO_LEAKAGE_DETECTION_CONTROL_EVENT_MUSIC_MUTE: {
            if (anc_enable) {
#ifdef MTK_ANC_ENABLE
                app_anc_service_suspend();
                s_app_leakage_detection_context.is_resume_anc = true;
#endif
            } else {
#ifdef AIR_TWS_ENABLE
                if (BT_AWS_MCE_ROLE_AGENT == role) 
#endif
                {
                    audio_anc_leakage_detection_start(NULL);
                }
            }
            break;
        }
        case AUDIO_LEAKAGE_DETECTION_CONTROL_EVENT_STOP: {
#ifdef AIR_PROMPT_SOUND_ENABLE
            if (APP_LEAKAGE_DETECTION_VP_INDEX == voice_prompt_get_current_index()) {
                voice_prompt_stop(APP_LEAKAGE_DETECTION_VP_INDEX, VOICE_PROMPT_ID_INVALID, false);
            }
#endif
            app_leakage_detection_broadcast_ongoing_status(true, false);
            if (s_app_leakage_detection_context.is_resume_anc) {
                app_anc_service_resume();
                s_app_leakage_detection_context.is_resume_anc = false;
            }
            break;
        }
        default:
            break;
    }

    if (event_id == AUDIO_LEAKAGE_DETECTION_CONTROL_EVENT_STOP
        && result ==  AUDIO_LEAKAGE_DETECTION_EXECUTION_SUCCESS) {
#ifdef AIR_PROMPT_SOUND_ENABLE
        if (APP_LEAKAGE_DETECTION_VP_INDEX == voice_prompt_get_current_index()) {
            voice_prompt_stop(APP_LEAKAGE_DETECTION_VP_INDEX, VOICE_PROMPT_ID_INVALID, false);
        }
#endif
        app_leakage_detection_broadcast_ongoing_status(true, false);
    }
}

static bool app_leakage_detection_proc_ui_shell_events(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    app_leakage_detection_context_t *ctx = NULL;

    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            self->local_context = &s_app_leakage_detection_context;
            memset(self->local_context, 0, sizeof(app_leakage_detection_context_t));
            ctx = (app_leakage_detection_context_t *)self->local_context;
            ctx->in_idle = true;
            audio_anc_leakage_detection_register_vp_start_callback(app_leakage_detection_trigger_vp_callback);
            audio_anc_leakage_detection_control_register_callback(app_leakage_detection_event_callback,
                                                                  AUDIO_LEAKAGE_DETECTION_CONTROL_EVENT_STOP,
                                                                  AUDIO_LEAKAGE_DETECTIONs_CONTROL_CB_LEVEL_SUCCESS_ONLY);
            break;
        }
        default:
            break;
    }
    return ret;
}


static bool app_leakage_detection_proc_apps_internal_events(ui_shell_activity_t *self,
                                       uint32_t event_id,
                                       void *extra_data,
                                       size_t data_len)
{
    bool ret = false;
    app_leakage_detection_context_t *ctx = (app_leakage_detection_context_t *)self->local_context;

    switch (event_id) {
        case APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP_TRIGGER:
            ret = true;
            if (ctx && ctx->in_idle) {
                bool *p_play_flag = (bool *)extra_data;
                APPS_LOG_MSGID_I(SC_TAG" trigger vp play, flag:%d", 1, *p_play_flag);
                if (*p_play_flag) {
#ifndef APP_LEAKAGE_DETECTION_VP_REPEAT_INTERVAL
                    voice_prompt_param_t vp = {0};
                    vp.vp_index = APP_LEAKAGE_DETECTION_VP_INDEX;
                    vp.control =  VOICE_PROMPT_CONTROL_SINGLERT;
                    vp.delay_time = 200;
                    vp.callback = app_leakage_detection_vp_callback;
                    voice_prompt_play(&vp, NULL);
                    //apps_config_set_voice(APP_LEAKAGE_DETECTION_VP_INDEX, true, 200, VOICE_PROMPT_PRIO_ULTRA, true, false, app_leakage_detection_vp_callback);
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP);
                    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP,
                                        NULL, 0, NULL,
                                        APP_LEAKAGE_DETECTION_VP_PLAY_TIME);
#else
                    ctx->vp_cnt = APP_LEAKAGE_DETECTION_VP_REPEAT_TIMES - 1;
                    voice_prompt_param_t vp = {0};
                    vp.vp_index = APP_LEAKAGE_DETECTION_VP_INDEX;
                    vp.control = VOICE_PROMPT_CONTROL_SINGLERT;
                    vp.delay_time = 200;
                    vp.callback = app_leakage_detection_vp_callback;
                    voice_prompt_play(&vp, NULL);
                    //apps_config_set_voice(APP_LEAKAGE_DETECTION_VP_INDEX, true, 200, VOICE_PROMPT_PRIO_ULTRA, false, false, app_leakage_detection_vp_callback);
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP);
                    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP,
                                        NULL, 0, NULL,
                                        APP_LEAKAGE_DETECTION_VP_REPEAT_INTERVAL);
#endif
                } else {
                    app_leakage_detection_stop(true);
                }
            } else {
                app_leakage_detection_stop(false);
            }

            break;

        case APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP: {
            ret = true;
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            bool ld_over = FALSE;

            APPS_LOG_MSGID_I(SC_TAG" [%02X] LEAKAGE_DETECTION_VP event", 1, role);
#ifndef APP_LEAKAGE_DETECTION_VP_REPEAT_INTERVAL
            app_leakage_detection_stop(true);
            if (role == BT_AWS_MCE_ROLE_PARTNER) {
                ld_over = TRUE;
            }
#else
            if (ctx && ctx->in_idle && ctx->vp_cnt > 0) {
                APPS_LOG_MSGID_I(SC_TAG" vp play, vp_cnt:%d", 1, ctx->vp_cnt);
                ctx->vp_cnt--;
                voice_prompt_param_t vp = {0};
                vp.vp_index = APP_LEAKAGE_DETECTION_VP_INDEX;
                vp.control = VOICE_PROMPT_CONTROL_SINGLERT;
                vp.delay_time = 200;
                vp.callback = app_leakage_detection_vp_callback;
                voice_prompt_play(&vp, NULL);
                //apps_config_set_voice(APP_LEAKAGE_DETECTION_VP_INDEX, true, 200, VOICE_PROMPT_PRIO_ULTRA, false, false);

                if (ctx->vp_cnt > 0) {
                    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP,
                                        NULL, 0, NULL,
                                        APP_LEAKAGE_DETECTION_VP_REPEAT_INTERVAL);
                }
                if (ctx->vp_cnt == 0 && role == BT_AWS_MCE_ROLE_PARTNER) {
                    ld_over = TRUE;
                }
            }
#endif

            if (ld_over) {
#ifdef MTK_AWS_MCE_ENABLE
                // Notify new Agent
                APPS_LOG_MSGID_I(SC_TAG" notify new agent LD over", 0);
                apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                         APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_STOP);
#endif
            }
            break;
        }
        case APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_STATUS: {
            bool is_ongoing  = (bool)extra_data;
            if (!is_ongoing) {
                ctx->seal_checking = false;
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE_V2)
                app_advance_passthrough_set_ld_ongoing(FALSE);
#endif
            }
            break;
        }
#ifdef MTK_IN_EAR_FEATURE_ENABLE
        case APPS_EVENTS_INTERACTION_IN_EAR_UPDATE_STA: {
            /* The event come from in ear detection app. */
            app_in_ear_sta_info_t *sta_info = (app_in_ear_sta_info_t *)extra_data;
            if (sta_info->current != APP_IN_EAR_STA_BOTH_IN
                && sta_info->previous != APP_IN_EAR_STA_BOTH_OUT
                && ctx->seal_checking) {
                APPS_LOG_MSGID_I(SC_TAG" out ear interrupt LD.", 0);
                app_leakage_detection_stop(false);
            }
            break;
        }
#endif
        default:
            break;
    }
    return ret;
}


static bool app_leakage_detection_proc_bt_state_events(ui_shell_activity_t *self,
                                 uint32_t event_id,
                                 void *extra_data,
                                 size_t data_len)
{
    app_leakage_detection_context_t *ctx = (app_leakage_detection_context_t *)self->local_context;

    switch (event_id) {
        case BT_SINK_SRV_EVENT_STATE_CHANGE: {
            bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *) extra_data;
            /* Leakage detection will stop when the HFP is active. */
            if ((param->previous < BT_SINK_SRV_STATE_INCOMING) && (param->current >= BT_SINK_SRV_STATE_INCOMING)) {
                ctx->in_idle = false;
                if (audio_anc_leakage_compensation_get_status()) {
                    app_leakage_detection_stop(false);
                }
            }

            if (param->current < BT_SINK_SRV_STATE_INCOMING) {
                ctx->in_idle = true;
            }
            break;
        }
#ifdef AIR_LE_AUDIO_ENABLE
        case BT_SINK_SRV_EVENT_LE_BIDIRECTION_LEA_UPDATE: {
            bt_sink_srv_bidirection_lea_state_update_t *event = (bt_sink_srv_bidirection_lea_state_update_t *)extra_data;
            if (event == NULL) {
                return false;
            }
            APPS_LOG_MSGID_I(SC_TAG",[BIDIRECTION_LEA_STATE] le call start:event_state=%d ", 1, event->state);
            if (BT_SINK_SRV_BIDIRECTION_LEA_STATE_ENABLE == event->state) {
                ctx->in_idle = false;
                if (audio_anc_leakage_compensation_get_status()) {
                    app_leakage_detection_stop(false);
                }
            } else if (BT_SINK_SRV_BIDIRECTION_LEA_STATE_DISABLE == event->state) {
                ctx->in_idle = true;
            }
            break;
        }
        case LE_SINK_SRV_EVENT_REMOTE_INFO_UPDATE: {
            bt_le_sink_srv_event_remote_info_update_t *ind = (bt_le_sink_srv_event_remote_info_update_t *)extra_data;
            if (ind == NULL) {
                break;
            }

            if (ind->pre_state == BT_BLE_LINK_CONNECTED
                && ind->state == BT_BLE_LINK_DISCONNECTED) {
#ifdef AIR_PROMPT_SOUND_ENABLE
                if (APP_LEAKAGE_DETECTION_VP_INDEX == voice_prompt_get_current_index()) {
                    voice_prompt_stop(APP_LEAKAGE_DETECTION_VP_INDEX, VOICE_PROMPT_ID_INVALID, true);
                }
#endif
                if (ctx->seal_checking) {
                    APPS_LOG_MSGID_I(SC_TAG" LD terminated LEA disconnected", 0);
                    app_leakage_detection_stop(false);
                }
            }
            break;
        }
#endif
        default:
            break;
    }

    return false;
}

static bool app_leakage_detection_proc_bt_cm_events(ui_shell_activity_t *self,
                                                              uint32_t event_id,
                                                              void *extra_data,
                                                              size_t data_len)
{
    bool ret = false;
    app_leakage_detection_context_t *ctx = (app_leakage_detection_context_t *)self->local_context;
#ifdef MTK_AWS_MCE_ENABLE
    //bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
#endif
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;

            bt_cm_profile_service_mask_t pre_srv = remote_update->pre_connected_service;
            bt_cm_profile_service_mask_t cur_srv = remote_update->connected_service;
            bt_cm_profile_service_mask_t aws_mask = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS);
            if (((pre_srv & ~aws_mask) && !(cur_srv & ~aws_mask)) || ((pre_srv & aws_mask) && (cur_srv == 0))) {
                APPS_LOG_MSGID_I(SC_TAG" apps_config_stop_voice.", 0);
#ifdef AIR_PROMPT_SOUND_ENABLE
                if (APP_LEAKAGE_DETECTION_VP_INDEX == voice_prompt_get_current_index()) {
                    voice_prompt_stop(APP_LEAKAGE_DETECTION_VP_INDEX, VOICE_PROMPT_ID_INVALID, true);
                }
#endif
                if (ctx->seal_checking) {
                    APPS_LOG_MSGID_I(SC_TAG" LD terminated bt disconnected", 0);
                    app_leakage_detection_stop(false);
                }
            }
#ifdef MTK_AWS_MCE_ENABLE
            //if (BT_AWS_MCE_ROLE_PARTNER == role || BT_AWS_MCE_ROLE_CLINET == role) {
                if ((pre_srv & aws_mask) && !(cur_srv & aws_mask)) {
                    /* Stop LD when aws disconnected. */
#ifdef AIR_PROMPT_SOUND_ENABLE
                    if (APP_LEAKAGE_DETECTION_VP_INDEX == voice_prompt_get_current_index()) {
                        voice_prompt_stop(APP_LEAKAGE_DETECTION_VP_INDEX, VOICE_PROMPT_ID_INVALID, false);
                    }
#endif
                    app_leakage_detection_stop(false);
                }
            //}
#endif
        }
        break;
        default:
            break;
    }

    return ret;
}

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
bool app_leakage_detection_proc_ull_events(ui_shell_activity_t *self,
                                    uint32_t event_id,
                                    void *extra_data,
                                    size_t data_len)
{
    bool ret = false;

    app_leakage_detection_context_t *ctx = (app_leakage_detection_context_t *)self->local_context;
    if (ctx == NULL) {
        return ret;
    }
    switch (event_id) {
        case BT_ULL_EVENT_LE_STREAMING_START_IND: {
            bt_ull_le_streaming_start_ind_t *bt_ull2_link_type = (bt_ull_le_streaming_start_ind_t *)extra_data;
            if (BT_ULL_LE_STREAM_MODE_UPLINK == bt_ull2_link_type->stream_mode) {
                ctx->in_idle = false;
#ifdef AIR_PROMPT_SOUND_ENABLE
                if (APP_LEAKAGE_DETECTION_VP_INDEX == voice_prompt_get_current_index()) {
                    voice_prompt_stop(APP_LEAKAGE_DETECTION_VP_INDEX, VOICE_PROMPT_ID_INVALID, true);
                }
#endif
                if (audio_anc_leakage_compensation_get_status()) {
                    app_leakage_detection_stop(false);
                }
            }
            break;
        }
        case BT_ULL_EVENT_LE_STREAMING_STOP_IND: {
            bt_ull_le_streaming_start_ind_t *bt_ull2_link_type = (bt_ull_le_streaming_start_ind_t *)extra_data;
            if (BT_ULL_LE_STREAM_MODE_UPLINK == bt_ull2_link_type->stream_mode) {
                ctx->in_idle = true;
            }
            break;
        }
        case BT_ULL_EVENT_LE_DISCONNECTED: {
#ifdef AIR_PROMPT_SOUND_ENABLE
            if (APP_LEAKAGE_DETECTION_VP_INDEX == voice_prompt_get_current_index()) {
                voice_prompt_stop(APP_LEAKAGE_DETECTION_VP_INDEX, VOICE_PROMPT_ID_INVALID, true);
            }
#endif
            if (ctx->seal_checking) {
                APPS_LOG_MSGID_I(SC_TAG" LD terminated ULL disconnected", 0);
                app_leakage_detection_stop(false);
            }
            break;
        }
    }
    return ret;
}
#endif

#ifdef MTK_ANC_ENABLE
static bool app_leakage_detection_proc_audio_anc_events(ui_shell_activity_t *self,
                                                    uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    bool ret = false;
    app_leakage_detection_context_t *local_context = (app_leakage_detection_context_t *)self->local_context;
    APPS_LOG_MSGID_I(SC_TAG" proc_audio_anc_event: audio_anc_event=0x%x", 1, event_id);
    switch (event_id) {
        case AUDIO_ANC_CONTROL_EVENT_SUSPEND_NOTIFY: {
            if (local_context->is_resume_anc) {
#ifdef AIR_TWS_ENABLE
                if (BT_AWS_MCE_ROLE_AGENT == bt_connection_manager_device_local_info_get_aws_role())
#endif
                {
                    audio_anc_leakage_detection_start(NULL);
                }

            }
            break;
        }
        default:
            break;
    }
    return ret;
}
#endif

bool app_leakage_detection_idle_activity_proc(ui_shell_activity_t *self,
                                              uint32_t event_group,
                                              uint32_t event_id,
                                              void *extra_data,
                                              size_t data_len)
{
    bool ret = false;
    //APPS_LOG_MSGID_I("app_leakage_detection_idle_activity_proc event_group : %x, id: %x", 2, event_group, event_id);
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = app_leakage_detection_proc_ui_shell_events(self, event_id, extra_data, data_len);
            break;

        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            /* Interaction events. */
            ret = app_leakage_detection_proc_apps_internal_events(self, event_id, extra_data, data_len);
            break;

        case EVENT_GROUP_UI_SHELL_BT_SINK:
            /* BT_SINK events, indicates the state of A2DP and HFP. */
            ret = app_leakage_detection_proc_bt_state_events(self, event_id, extra_data, data_len);
            break;

        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER:
            /* Event come from bt connection manager, indicates the connection state of HFP or AWS. */
            ret = app_leakage_detection_proc_bt_cm_events(self, event_id, extra_data, data_len);
            break;
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY: {
            ret = app_leakage_detection_proc_ull_events(self, event_id, extra_data, data_len);
            ret = false;
            break;
        }
#endif
#ifdef MTK_ANC_ENABLE
        case EVENT_GROUP_UI_SHELL_AUDIO_ANC: {
            ret = app_leakage_detection_proc_audio_anc_events(self, event_id, extra_data, data_len);
            break;
        }
#endif
        default:
            break;
    }

    return ret;
}

#endif /* MTK_LEAKAGE_DETECTION_ENABLE */

