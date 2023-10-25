
/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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
 * File: app_ms_cfu_idle_activity.c
 *
 * Description:
 * This file is ms_cfu idle activity. This activity is used for cfu status management
 *
 */


#ifdef AIR_CFU_ENABLE

#include "apps_debug.h"
#include "app_ms_cfu_idle_activity.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_config_event_list.h"
#include "app_home_screen_idle_activity.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "apps_aws_sync_event.h"
#endif
#include "bt_connection_manager.h"
#include "bt_device_manager.h"
#include "bt_callback_manager.h"
#include "bt_sink_srv.h"
#include "bt_gap_le.h"
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le_cap_audio_manager.h"
#include "app_le_audio_air.h"
#include "app_le_audio.h"
#include "bt_le_audio_source.h"
#include "ble_mcs.h"
#endif


#include "cfu.h"

#define TAG "[CFU] idle_activity "

/* Init firmware version and product info.*/
#define  CFU_FW_VESION          (0x01000100)        /* 0.1.0.0.0.1.0.0 (initial version). */
#define  CFU_PRODUCT_INFO       (0x00000100)        /* 01 is dongle component id.*/


typedef struct {
    bool                 speaker_mute;   /**<  Record if the speaker is muted. */
    bt_sink_srv_state_t  curr_state;     /**<  Record the current sink_state. */
    bt_handle_t          sub_handle;     /**<  Record the handle of sub-component.*/
} app_ms_cfu_context_t;

static app_ms_cfu_context_t s_app_ms_cfu_context;

#ifdef AIR_CFU_ACTIVE_MODE
static void app_ms_cfu_handle_music(bool is_pause, bool is_mute)
{
    uint16_t  temp_key_action = KEY_ACTION_INVALID;
    if (is_pause) {
        temp_key_action = KEY_AVRCP_PAUSE;
    }
    uint16_t *key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
    if (key_action != NULL) {
        memcpy(key_action, &temp_key_action, sizeof(uint16_t));
        ui_shell_send_event(true, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY,
                            0, key_action, sizeof(uint16_t), NULL, 0);

        /* Mute or unmute mic.*/
        bool mute = is_mute;
        s_app_ms_cfu_context.speaker_mute = mute;
        bt_status_t bt_status = bt_sink_srv_set_mute(BT_SINK_SRV_MUTE_SPEAKER, mute);
        if (BT_STATUS_SUCCESS == bt_status) {
            /* Sync mute or unmute operation to peer when aws mce enable.*/
#ifdef MTK_AWS_MCE_ENABLE
            if (TRUE == app_home_screen_idle_activity_is_aws_connected()) {
                bt_status_t send_state = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                                        APPS_EVENTS_INTERACTION_SYNC_SPEAKER_MUTE_STATUS, &mute, sizeof(bool));
                if (BT_STATUS_SUCCESS == send_state) {
                    APPS_LOG_MSGID_I(TAG" [MUTE] sync speaker to peer success.", 0);
                } else if (BT_STATUS_FAIL == send_state) {
                    APPS_LOG_MSGID_I(TAG" [MUTE] sync speaker to peer fail.", 0);
                }
            }
#endif
        }
    }
}
#endif

/**
 * @brief      The CFU event function is callback from CFU middleware, need to send event and run in UI Shell task.
 * @param[in]  event, CFU event types.
 * @param[in]  param, event parameter.
 */
static void app_ms_cfu_event_callback(cfu_event_type_enum event, void *param)
{
    ui_shell_status_t ret = UI_SHELL_STATUS_OK;
    switch (event) {
        case CFU_EVENT_TYPE_NEED_REBOOT: {
            uint32_t delay = (uint32_t)param;
            APPS_LOG_MSGID_I(TAG" app_ms_cfu_event_callback - reboot", 0);
            ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                      EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_REQUEST_REBOOT,
                                      NULL, 0, NULL, delay);
            break;
        }

        case CFU_EVENT_TYPE_OTA_START: {
            APPS_LOG_MSGID_I(TAG" app_ms_cfu_event_callback - start ota.", 0);

            uint8_t curr_component_id = cfu_get_component_id();
            if (curr_component_id != CFU_COMPONENT_ID_DONGLE) {
#ifdef AIR_CFU_ACTIVE_MODE
                /* Get current streaming and call status.*/
                if (s_app_ms_cfu_context.curr_state == BT_SINK_SRV_STATE_STREAMING) {
                    app_ms_cfu_handle_music(true, true);
                }
#endif
            } else if (curr_component_id == CFU_COMPONENT_ID_DONGLE) {
#if 0//def AIR_LE_AUDIO_ENABLE
                app_le_audio_ucst_mode_t mode;
                bool ret = app_le_audio_is_unicast_streaming(&mode);
                APPS_LOG_MSGID_I(TAG" app_ms_cfu_event_callback - start ota: ret=%d,mode=0x%x",
                                 2, ret, mode);
                if (ret) {
                    if (mode != APP_LE_AUDIO_UCST_MODE_CHAT) {
#ifdef AIR_CFU_ACTIVE_MODE
                        bt_le_audio_source_action_param_t le_param;
                        le_param.service_index = ble_mcs_get_gmcs_service_idx();
                        bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_MEDIA_PAUSE, &le_param);
#endif
                    } else if (mode == APP_LE_AUDIO_UCST_MODE_CHAT) {
                        cfu_set_device_state(true);
                    }
                }
#endif //AIR_LE_AUDIO_ENABLE
            }
            break;
        }
        case CFU_EVENT_TYPE_OTA_END: {
            APPS_LOG_MSGID_I(TAG" app_ms_cfu_event_callback - end ota.", 0);
            uint8_t curr_component_id = cfu_get_component_id();
            if (curr_component_id != CFU_COMPONENT_ID_DONGLE) {
#ifdef AIR_CFU_ACTIVE_MODE
                if (s_app_ms_cfu_context.speaker_mute) {
                    app_ms_cfu_handle_music(false, false);
                }
#endif
            } else if (curr_component_id == CFU_COMPONENT_ID_DONGLE) {
                cfu_set_device_state(false);
            }

            break;
        }

        case CFU_EVENT_TYPE_NEED_PAUSE_MUSIC: {
            APPS_LOG_MSGID_I(TAG" app_ms_cfu_event_callback -pause_music.", 0);
#ifdef AIR_LE_AUDIO_ENABLE
            bt_le_audio_source_action_param_t le_param;
            le_param.service_index = ble_mcs_get_gmcs_service_idx();
            bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_MEDIA_PAUSE, &le_param);
#endif
            break;
        }

        default:
            break;
    }
    APPS_LOG_MSGID_I(TAG" app_ms_cfu_event_callback: event=%d ret=%d", 2, event, ret);
}

#ifdef AIR_LE_AUDIO_ENABLE

static void app_ms_cfu_get_curr_streaming_mode(uint8_t *mode)
{
    /*app_le_audio_ucst_mode_t temp_mode = APP_LE_AUDIO_UCST_MODE_MAX;
    bool ret = app_le_audio_is_unicast_streaming(&temp_mode);
    APPS_LOG_MSGID_I(TAG" app_ms_cfu_get_curr_streaming_mode: ret=%d,temp_mode=0x%x",
            2, ret, temp_mode);
    *mode = temp_mode;*/
}

static bt_status_t app_ms_cfu_handle_le_audio_callback_evt(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_GAP_LE_DISCONNECT_IND: {
            bt_gap_le_disconnect_ind_t *ind = (bt_gap_le_disconnect_ind_t *)buff;
            if (ind == NULL || ind->connection_handle == BT_HANDLE_INVALID) {
                break;
            }
            APPS_LOG_MSGID_I(TAG" bt_event LE_DISCONNECT_IND, handle=0x%4X ind=0x%4X",
                             2, s_app_ms_cfu_context.sub_handle, ind->connection_handle);
            if (s_app_ms_cfu_context.sub_handle == ind->connection_handle) {
                cfu_set_sub_component_connection_state(false);
            }
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static void app_ms_cfu_le_audio_air_callback(app_le_audio_air_event_t event, void *buff)
{
    if (buff == NULL) {
        APPS_LOG_MSGID_I(TAG" le_audio_air_callback buff is NULL.", 0);
        return;
    }
    APPS_LOG_MSGID_I(TAG" le_audio_callback_evt: event=%d", 1, event);

    switch (event) {
        case APP_LE_AUDIO_AIR_EVENT_ENABLE_SERVICE_COMPLETE: {
            app_le_audio_air_event_enable_service_complete_t *evt = (app_le_audio_air_event_enable_service_complete_t *)buff;
            if (evt == NULL || evt->handle == BT_HANDLE_INVALID) {
                break;
            }

            APPS_LOG_MSGID_I(TAG" le_audio_air_callback SERVICE_COMPLETE, [%02X] handle=0x%4X status=0x%8X",
                             3, evt->role, evt->handle, evt->status);
            if (evt->status == BT_STATUS_SUCCESS && (evt->role == BT_AWS_MCE_ROLE_AGENT || evt->role == BT_AWS_MCE_ROLE_NONE)) {
                APPS_LOG_MSGID_I(TAG" le_audio_callback_evt: LE-CONNECT sub-component!", 0);
                s_app_ms_cfu_context.sub_handle = evt->handle;
                cfu_set_sub_component_connection_state(true);
            }
            break;
        }
        case APP_LE_AUDIO_AIR_EVENT_RHO: {
            app_le_audio_air_event_rho_t *rho_evt = (app_le_audio_air_event_rho_t *)buff;
            if (rho_evt == NULL || rho_evt->handle == BT_HANDLE_INVALID) {
                break;
            }

            APPS_LOG_MSGID_I(TAG" le_audio_air_callback AIR_EVENT_RHO, [%02X] handle=0x%4X",
                             2, rho_evt->role, rho_evt->handle);
            if (rho_evt->role == BT_AWS_MCE_ROLE_AGENT) {
                APPS_LOG_MSGID_I(TAG" le_audio_callback_rho_evt: LE-CONNECT sub-component!", 0);
                s_app_ms_cfu_context.sub_handle = rho_evt->handle;
                cfu_set_sub_component_connection_state(true);
            }
            break;
        }
        default:
            break;
    }
}
#endif //AIR_LE_AUDIO_ENABLE

/**
 * @brief      The cfu APP init function, this function will be called when BT power on.
 */
static void app_ms_cfu_init()
{
    APPS_LOG_MSGID_I(TAG" init cfu", 0);

    cfu_init_para_t init_para = {0};

    /* Important reminder! Customer need to modify these parameters. */
    init_para.component_id         = CFU_COMPONENT_ID_DONGLE;  /* Customer configure component id. */
    init_para.firmware_version     = CFU_FW_VESION;            /* Customer configure option: 1.0.0.0.0.1.0.0 (initial version). */
    init_para.product_info         = CFU_PRODUCT_INFO;         /* Customer configure product information. */
    init_para.event_fun            = app_ms_cfu_event_callback;
#ifdef AIR_LE_AUDIO_ENABLE
    init_para.get_le_audio_streaming_mode   = app_ms_cfu_get_curr_streaming_mode;
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_SYSTEM),
                                          (void *)app_ms_cfu_handle_le_audio_callback_evt);
    app_le_audio_air_register_callback(app_ms_cfu_le_audio_air_callback);
#endif

    cfu_init(&init_para);
}


static bool _proc_ui_shell_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(TAG" create", 0);
            memset(&s_app_ms_cfu_context, 0, sizeof(app_ms_cfu_context_t));
            app_ms_cfu_init();
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY: {
            APPS_LOG_MSGID_I(TAG" destroy", 0);
            break;
        }
        default:
            break;
    }
    return ret;
}

static bool app_ms_cfu_bt_sink_event_proc(ui_shell_activity_t *self,
                                          uint32_t event_group,
                                          uint32_t event_id,
                                          void *extra_data,
                                          size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        case BT_SINK_SRV_EVENT_STATE_CHANGE: {
            bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *)extra_data;
            bool device_is_busy = false;
            if (param == NULL) {
                APPS_LOG_MSGID_I(TAG", bt_sink_state_change :param is null ", 0);
                return ret;
            }
            s_app_ms_cfu_context.curr_state = param->current;
            APPS_LOG_MSGID_I(TAG", bt_sink_state_change: param_previous: %x, param_now: %x", 2, param->previous, param->current);
            /* Check the call status, if there are no HFP related states, finish current activity. */
            if ((param->current < BT_SINK_SRV_STATE_STREAMING) && (param->previous >= BT_SINK_SRV_STATE_STREAMING)) {
                /* Busy is end */
                device_is_busy = false;
            } else {
                /*Device is busy*/
                if (param->current > BT_SINK_SRV_STATE_STREAMING) {
                    device_is_busy = true;
                } else if (param->current == BT_SINK_SRV_STATE_STREAMING
                           && cfu_is_running()) {
#ifdef AIR_CFU_ACTIVE_MODE
#ifdef MTK_AWS_MCE_ENABLE
                    if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role())
#endif
                    {
                        app_ms_cfu_handle_music(true, true);
                    }
#endif
                }
            }
            APPS_LOG_MSGID_I(TAG", bt_sink_state_change: device_is_busy=%d", 1, device_is_busy);
            /* Set device state. */
            cfu_set_device_state(device_is_busy);
            break;
        }
        default: {
            break;
        }
    }
    return ret;
}


static bool app_ms_cfu_bt_cm_event_proc(struct _ui_shell_activity *self,
                                        uint32_t event_id,
                                        void *extra_data,
                                        size_t data_len)
{
    bt_cm_remote_info_update_ind_t *info = (bt_cm_remote_info_update_ind_t *)extra_data;
    if (event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE) {
        if (!(info->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL))
            && (info->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL))) {
            APPS_LOG_MSGID_I(TAG" sub-component connected.", 0);
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
                cfu_set_sub_component_connection_state(true);
            }
#else
            cfu_set_sub_component_connection_state(true);
#endif
        } else if ((info->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL))
                   && (!(info->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL)))) {
            APPS_LOG_MSGID_I(TAG" sub-component disconnected.", 0);
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
                cfu_set_sub_component_connection_state(false);
            }
#else
            cfu_set_sub_component_connection_state(false);
#endif
        }
    }
    return false;
}

/**
 * @brief The activity event handler
 *
 * @param self
 * @param event_group
 * @param event_id
 * @param extra_data
 * @param data_len
 */
bool app_ms_cfu_idle_activity_proc(struct _ui_shell_activity *self,
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
        case EVENT_GROUP_UI_SHELL_BT_SINK: {
            /* BT sink events, notify cfu middle. */
            ret = app_ms_cfu_bt_sink_event_proc(self, event_group, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            /* Bt connection manager event, indicates the power state of BT. */
            ret = app_ms_cfu_bt_cm_event_proc(self, event_id, extra_data, data_len);
            break;
        }
#if defined(MTK_AWS_MCE_ENABLE)
        case EVENT_GROUP_UI_SHELL_AWS_DATA: {
            bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;

            if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
                uint32_t event_group;
                uint32_t action;
                void *p_extra_data = NULL;
                uint32_t extra_data_len = 0;

                apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &action, &p_extra_data, &extra_data_len);

                if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
                    && action == APPS_EVENTS_INTERACTION_SYNC_SPEAKER_MUTE_STATUS) {
                    bool mute = *(bool *)p_extra_data;
                    s_app_ms_cfu_context.speaker_mute = mute;
                    APPS_LOG_MSGID_I(TAG" [MUTE] received speaker mute_status=%d", 1, mute);
                    bt_sink_srv_set_mute(BT_SINK_SRV_MUTE_SPEAKER, mute);
                    return true;
                }
            }
            break;
        }
#endif
        default: {
            break;
        }
    }
    return ret;
}

#endif /* AIR_CFU_ENABLE */

