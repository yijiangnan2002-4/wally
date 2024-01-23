
/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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
 * File: app_ms_teams_activity.c
 *
 * Description:
 * This file is ms_teams idle activity. This activity is used for teams status management
 *
 */


#ifdef AIR_MS_TEAMS_ENABLE
#include "app_ms_teams_led.h"
#include "app_ms_teams_telemetry.h"
#include "app_ms_teams_idle_activity.h"
#include "app_ms_teams_activity.h"
#include "app_ms_teams_utils.h"
#include "bt_connection_manager.h"
#include "app_bt_state_service.h"
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio_utillity.h"
#include "app_le_audio_ucst_utillity.h"
#include "usb_hid_srv.h"
#else
#define USB_HID_SRV_EVENT_CALL_MAX 0xff
#endif

#include "stdlib.h"
#include "task.h"
#include "apps_events_usb_event.h"
#include "apps_events_battery_event.h"
#include "bt_spp.h"

#ifdef AIR_MS_TEAMS_SPECIAL_CLIENT_ENABLE
#include "bt_source_srv.h"
#include "app_bt_source_event.h"
extern void app_ms_teams_connect_to_special_client(bt_bd_addr_t *addr);
extern void app_ms_teams_special_client_init(void);

#endif

#include "bt_hci.h"
#include "apps_events_bt_event.h"
#include "bt_app_common.h"

#define MS_TEAMS_NOTIFY_EV_ALIVE_TIME (5 * 60 *1000)

#define TAG "[MS TEAMS] idle_activity "

/**
 *  @brief This enumeration defines the in ear detection state.
 */
typedef enum {
    APP_IN_EAR_STA_BOTH_IN,         /**<  The agent and partner are in the ear. */
    APP_IN_EAR_STA_BOTH_OUT,        /**<  The agent and partner are not in the ear. */
    APP_IN_EAR_STA_AIN_POUT,        /**<  Only agent is in the ear. */
    APP_IN_EAR_STA_AOUT_PIN,        /**<  Only partner is in the ear. */
} app_in_ear_sta_t;


/**
 *  @brief This structure defines the state information.
 */
typedef struct {
    app_in_ear_sta_t previous;      /**<  The previous state of earbuds. */
    app_in_ear_sta_t current;       /**<  The current state of earbuds. */
} app_in_ear_sta_info_t;

bool s_teams_connected = false;
/* TODO: udpate this flag under the case of HFP + Teams. */
static bool s_hfp_connected = false;
static ms_teams_notif_sub_event_t s_last_notify_ev = MS_TEAMS_NOTIF_EVENT_NONE;
/* 0 is not call state. */
static uint32_t s_last_hid_call_state_event = USB_HID_SRV_EVENT_CALL_MAX;

static bool app_ms_teams_idle_teams_ev_proc(struct _ui_shell_activity *self, uint32_t ev, uint8_t *data, uint32_t data_len);

#include "atci.h"
#ifdef MS_TEAMS_TEST
extern void ms_teams_conn_test(uint32_t test_type, uint32_t package_type);

static atci_status_t app_ms_teams_atci_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    char *param = NULL;
    uint32_t p1 = 0, p2 = 0;

    param = parse_cmd->string_ptr + parse_cmd->name_len + 1;
    param = strtok(param, ",");

    p1 = atoi(param);
    param = strtok(NULL, ",");
    p2 = atoi(param);
    APPS_LOG_MSGID_I(TAG"at commnd: %d,%d", 2, p1, p2);
    ms_teams_conn_test(p1, p2);

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_cmd_hdlr_item_t app_ms_teams_atci_cmd_debug[] = {
    {
        .command_head = "AT+TEAMD_TEST",
        .command_hdlr = app_ms_teams_atci_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

static void app_ms_teams_test_init()
{
    atci_register_handler(app_ms_teams_atci_cmd_debug, sizeof(app_ms_teams_atci_cmd_debug) / sizeof(atci_cmd_hdlr_item_t));
}
#endif

static atci_status_t app_ms_teams_common_atci_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    char *param = NULL;
    uint32_t p1 = 0, p2 = 0, p3 = 0;

    param = parse_cmd->string_ptr + parse_cmd->name_len + 1;
    param = strtok(param, ",");

    p1 = atoi(param);
    param = strtok(NULL, ",");
    p2 = atoi(param);
    param = strtok(NULL, ",");
    p3 = atoi(param);
    APPS_LOG_MSGID_I(TAG"common at commnd: %d,%d", 2, p1, p2);
    if (p1 == 0) {
        app_ms_teams_idle_teams_ev_proc(NULL, p2, NULL, 0);
    } else if (p1 == 1) {
        if (p2 == 1) {
            app_ms_teams_set_hardmute_lock(p3 > 0);
        } else if (p2 == 2) {
            app_ms_teams_set_headset_worn(p3 > 0);
        } else if (p2 == 3) {
            app_ms_teams_set_button_press_info_hook(p3 > 0);
        } else if (p2 == 4) {
            app_ms_teams_set_button_press_info_mute(p3 > 0);
        } else if (p2 == 5) {
            app_ms_teams_set_button_press_info_flash(p3 > 0);
        }  else if (p2 == 6) {
            app_ms_teams_set_error_message((uint8_t*)"unknown errors.", strlen("unknown errors."));
        }
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_cmd_hdlr_item_t app_ms_teams_common_atci_cmdg[] = {
    {
        .command_head = "AT+TEAMS",
        .command_hdlr = app_ms_teams_common_atci_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

static void app_ms_teams_common_atci_init()
{
    atci_register_handler(app_ms_teams_common_atci_cmdg, sizeof(app_ms_teams_common_atci_cmdg) / sizeof(atci_cmd_hdlr_item_t));
}


static bool app_ms_teams_ev_notify_handler(ms_teams_notif_sub_event_t ev, uint8_t *data, uint32_t data_len)
{
    APPS_LOG_MSGID_I(TAG"teams notify event=0x%x.", 1, ev);
    uint32_t shell_event = 0;
    switch (ev) {
        case MS_TEAMS_NOTIF_EVENT_NONE:
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                NULL, 0);
            s_last_notify_ev = MS_TEAMS_NOTIF_EVENT_NONE;
            break;
        case MS_TEAMS_NOTIF_EVENT_MISSED_CALL:
        case MS_TEAMS_NOTIF_EVENT_UPCOMING_SCHEDULED_MEETING:
        case MS_TEAMS_NOTIF_EVENT_UNCHECKE_VOICE_MAIL: {
            s_last_notify_ev = ev;
            /* Clear the notify state */
            shell_event = ((MS_TEAMS_EVENT_NOTIFY << 16) & 0xFFFF0000) | 0xFFFF;
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_MS_TEAMS, shell_event);
            ui_shell_send_event(true, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_MS_TEAMS,
                                shell_event,
                                NULL, 0, NULL, MS_TEAMS_NOTIFY_EV_ALIVE_TIME);
            /* Use the incoming call's led UI to indicate these event. */
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_MS_TEAMS_SPECIAL_CLIENT_ENABLE)
            if (s_last_hid_call_state_event == USB_HID_SRV_EVENT_CALL_END || s_last_hid_call_state_event == USB_HID_SRV_EVENT_CALL_MAX)
#else
            if (s_last_hid_call_state_event == USB_HID_SRV_EVENT_CALL_MAX)
#endif
            {
                app_ms_teams_set_background_led(USB_HID_SRV_EVENT_CALL_END, s_last_notify_ev, APP_MS_TEAMS_CONNECTION_STA_CONNECTED);
            }
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                NULL, MS_TEAMS_NOTIFY_EV_ALIVE_TIME);
            break;
            default:
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN);
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                    NULL, 0);
                s_last_notify_ev = MS_TEAMS_NOTIF_EVENT_NONE;
                break;
            }
    }

    return true;
}

extern const bt_bd_addr_t *bt_app_common_get_local_random_addr(void);
static bool app_ms_teams_idle_teams_ev_proc(struct _ui_shell_activity *self, uint32_t ev, uint8_t *data, uint32_t data_len)
{
    ms_teams_event_t event = ((ev >> 16) & 0xFFFF);

    APPS_LOG_MSGID_I(TAG"teams event proc, 0x%x, 0x%x.", 2, ev, event);
    switch (event) {
        case MS_TEAMS_CONNECTED: {
            s_teams_connected = true;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                NULL, 0);
            bt_bd_addr_t* addr = bt_device_manager_get_local_address();
            apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, ev, addr, sizeof(bt_bd_addr_t));
            break;
        }
        case MS_TEAMS_DISCONNECTED:
            s_teams_connected = false;
            apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_MS_TEAMS, ev);
            break;
        case MS_TEAMS_EVENT_LOCALE:
            break;
        case MS_TEAMS_EVENT_NOTIFY: {
#ifdef MTK_AWS_MCE_ENABLE
            bt_aws_mce_role_t role;
            bt_status_t ret = BT_STATUS_SUCCESS;
            role = bt_device_manager_aws_local_info_get_role();
            if (role == BT_AWS_MCE_ROLE_AGENT) {
                /* If rho switched and the "None" notification not sent, the new Agent will use this event to keep a timer to update the state. */
                ret = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, event, data, data_len);
                APPS_LOG_MSGID_I(TAG"partner notify event to partner ret=%d.", 1, ret);
            }
#endif
            app_ms_teams_ev_notify_handler((ms_teams_notif_sub_event_t)(ev & 0xFFFF), data, data_len);
            apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_MS_TEAMS, ev);
            break;
        }
        case MS_TEAMS_EVENT_CALL_STATE: {
            break;
        }
        default:
            break;
    }

    return true;
}

static bool _proc_ui_shell_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            app_ms_teams_load_default_telemetry_setting();
            app_ms_teams_init();
#ifdef MS_TEAMS_TEST
            app_ms_teams_test_init();
#endif
            app_ms_teams_common_atci_init();
            /* Connection timeout, set LED off. */
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                NULL, 10 * 1000);
            #ifdef AIR_MS_TEAMS_SPECIAL_CLIENT_ENABLE
            app_ms_teams_special_client_init();
            #endif
            break;
        }
        default:
            break;
    }
    return ret;
}

static bool _proc_key_action(apps_config_key_action_t action, uint32_t event_id, void *extra_data, size_t data_len)
{
    static bool is_teams_btn_down = false;
    bool ret = false;
    switch (action) {
        case KEY_REDIAL_LAST_CALL: {
            ms_teams_send_action(MS_TEAMS_ACTION_CALL_DIAL_OUT, NULL, 0);
            ret = true;
        }
        break;
        case KEY_MS_TEAMS_BTN_LONG_PRESS: {
            ms_teams_btn_press_type_t type = MS_TEAMS_BTN_PRESS_TYPE_LONG_PRESS;
            ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
            ret = true;
            break;
        }
        case KEY_MS_TEAMS_BTN_INVOKE: {
            is_teams_btn_down = true;
            if (app_ms_teams_is_dongle_connected()) {
                apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_KEY, event_id, extra_data, data_len);
            } else {
                ms_teams_btn_press_type_t type = MS_TEAMS_BTN_PRESS_TYPE_SHORT_PRESS;
                ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
            }
            ret = true;
            break;
        }
        case KEY_MS_TEAMS_BTN_RELEASE: {
            if (!is_teams_btn_down) {
                break;
            }
            is_teams_btn_down = false;
            if (app_ms_teams_is_dongle_connected()) {
                apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_KEY, event_id, (void *)&action, sizeof(apps_config_key_action_t));
            } else {
                ms_teams_btn_press_type_t type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
                ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_RELEASE, &type, sizeof(ms_teams_btn_press_type_t));
            }
            ret = true;
            break;
        }
        default:
            break;
    }

    return ret;
}

static bool _proc_key_event(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    apps_config_key_action_t action;
    uint8_t key_id;
    airo_key_event_t key_event;

    /* Only KEY_REDIAL_LAST_CALL will handle in idle state, if HFP profile connected, redial through HFP */
    if (!s_teams_connected || s_hfp_connected) {
        //return false;
    }

    app_event_key_event_decode(&key_id, &key_event, event_id);
    if (extra_data) {
        action = *(uint16_t *)extra_data;
    } else {
        return false;
    }

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role;
    role = bt_device_manager_aws_local_info_get_role();
    if (role == BT_AWS_MCE_ROLE_PARTNER) {
        if (action == KEY_REDIAL_LAST_CALL || action == KEY_MS_TEAMS_BTN_INVOKE || action == KEY_MS_TEAMS_BTN_RELEASE) {
            ret = apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action);
            APPS_LOG_MSGID_I(TAG"partner sync key action=%d to agent ret=%d.", 2, action, ret);
            return true;
        }
    }
#endif
    ret = _proc_key_action(action, event_id, extra_data, data_len);
    return ret;
}

static bool _proc_bt_cm_event(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bt_cm_remote_info_update_ind_t *info = (bt_cm_remote_info_update_ind_t *)extra_data;
    if (event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE) {
#if 0
        if (info->pre_acl_state != BT_CM_ACL_LINK_CONNECTED && info->acl_state == BT_CM_ACL_LINK_CONNECTED) {
#else
        if ((!(info->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL)))
            && (info->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL))) {
#endif
            APPS_LOG_MSGID_I(TAG"update client connection 0x%x.", 1, info->address);
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
                ms_teams_update_client_connection((const bt_bd_addr_t *)&info->address);
            }
#else
            ms_teams_update_client_connection((const bt_bd_addr_t *)info->address);
#endif
        }
    }
    return false;
}

#ifdef MTK_AWS_MCE_ENABLE
static bool _proc_aws_data(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
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

        if (event_group == EVENT_GROUP_UI_SHELL_KEY) {
            APPS_LOG_MSGID_I(TAG"receive partner key action: %d.", 1, action);
            ret = _proc_key_action(action, event_id, extra_data, data_len);
        } else if (event_group == EVENT_GROUP_UI_SHELL_MS_TEAMS) {
            ms_teams_event_t event = ((event_id >> 16) & 0xFFFF);
            if (event == MS_TEAMS_EVENT_NOTIFY) {
                app_ms_teams_ev_notify_handler((ms_teams_notif_sub_event_t)(event_id & 0xFFFF), p_extra_data, extra_data_len);
            }
        }
    }

    /* TODO: handle the teams event come from Agent. */
    return ret;
}
#endif

#if 0
static void app_ms_teams_get_spp_rsi(uint32_t spp_handle)
{
    const bt_bd_addr_t *addr = bt_spp_get_bd_addr_by_handle(spp_handle);
    if (!addr) {
        return;
    }

    bt_gap_connection_handle_t gap_handle = bt_gap_get_handle_by_address((const bt_bd_addr_t*) addr);
    bt_gap_read_rssi(gap_handle);
}
#endif

/**
 * @brief The activity event handler
 *
 * @param self
 * @param event_group
 * @param event_id
 * @param extra_data
 * @param data_len
 */
bool app_ms_teams_idle_activity_proc(struct _ui_shell_activity *self,
                                     uint32_t event_group,
                                     uint32_t event_id,
                                     void *extra_data,
                                     size_t data_len)
{

    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;

        case EVENT_GROUP_UI_SHELL_KEY:
            /* key event. */
            ret = _proc_key_event(self, event_id, extra_data, data_len);
            break;

        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER:
            /* The event come from bt connection manager, indicates the power state of BT. */
            ret = _proc_bt_cm_event(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_USB_AUDIO: {
            if (event_id == APPS_EVENTS_USB_CONFIG_DONE) {
                ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_TEAMS_DELAY_TO_RECONNECT,
                                NULL, 0, NULL, 3000);
            }
            break;
        }
        case EVENT_GROUP_UI_SHELL_MS_TEAMS:
            ret = app_ms_teams_idle_teams_ev_proc(self, event_id, extra_data, data_len);
            break;
        #ifdef AIR_MS_TEAMS_SPECIAL_CLIENT_ENABLE
        case EVENT_GROUP_UI_SHELL_BT_SOURCE: {
            if (event_id == BT_SOURCE_SRV_EVENT_ACCEPT_CALL || event_id == BT_SOURCE_SRV_EVENT_VOICE_RECOGNITION_ACTIVATION)
            {
                APPS_LOG_MSGID_I(TAG"receive special client key event=0x%x.", 1, event_id);
                bt_source_srv_accept_call_t *call_info = (bt_source_srv_accept_call_t*)extra_data;
                if (!call_info || call_info->index != BT_SOURCE_SRV_CALL_INVALID_INDEX) {
                    break;
                }
                ms_teams_btn_press_type_t type;
                type = event_id == BT_SOURCE_SRV_EVENT_ACCEPT_CALL ? MS_TEAMS_BTN_PRESS_TYPE_SHORT_PRESS : MS_TEAMS_BTN_PRESS_TYPE_LONG_PRESS;
                ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
                type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
                ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_RELEASE, &type, sizeof(ms_teams_btn_press_type_t));
            }
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_SOURCE_APP: {
            static bool bt_source_client_connected = false;
            if (event_id == APP_BT_SOURCE_EVENT_NOTIFY_CONN_CONNECTED) {
                if (!bt_source_client_connected) {
                    app_ms_teams_connect_to_special_client((bt_bd_addr_t *)extra_data);
                }
                bt_source_client_connected = true;
            } else if (event_id == APP_BT_SOURCE_EVENT_NOTIFY_CONN_DISCONNECTED) {
                bt_source_client_connected = false;
            }
            break;
        }
        #endif
#ifdef MTK_AWS_MCE_ENABLE
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            /* The event come from partner. */
            ret = _proc_aws_data(self, event_id, extra_data, data_len);
            break;
#endif
        case EVENT_GROUP_UI_SHELL_BT: {
            if (event_id == BT_GAP_LE_READ_RSSI_CNF) {
                apps_bt_event_data_t *bt_event_data = (apps_bt_event_data_t *)extra_data;
                if (bt_event_data && bt_event_data->buffer) {
                    bt_hci_evt_cc_read_rssi_t *read_rssi_result = (bt_hci_evt_cc_read_rssi_t *)(bt_event_data->buffer);
                    if (read_rssi_result->rssi < 100) {
                        app_ms_teams_set_link_quality(0x02);
                    }
                }
            } else if (event_id == BT_GAP_READ_RSSI_CNF) {
                apps_bt_event_data_t *bt_event_data = (apps_bt_event_data_t *)extra_data;
                if (bt_event_data && bt_event_data->buffer) {
                    bt_gap_read_rssi_cnf_t *read_rssi_result = (bt_gap_read_rssi_cnf_t *)(bt_event_data->buffer);
                    if (read_rssi_result->rssi < 100) {
                        app_ms_teams_set_link_quality(0x02);
                    }
                }
            }
            break;
        }
        case EVENT_GROUP_UI_SHELL_DONGLE_DATA: {
            apps_dongle_event_sync_info_t *pkg = (apps_dongle_event_sync_info_t *)extra_data;
            if (pkg->event_group == EVENT_GROUP_UI_SHELL_KEY) {
                apps_config_key_action_t action = *((uint16_t *)&pkg->data);
                switch (action) {
                    case KEY_MS_TEAMS_BTN_LONG_PRESS: {
                        ms_teams_btn_press_type_t type = MS_TEAMS_BTN_PRESS_TYPE_LONG_PRESS;
                        //ms_teams_error_code_t ret = ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
                        ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
                        type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
                        ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_RELEASE, &type, sizeof(ms_teams_btn_press_type_t));
                        ret = true;
                        break;
                    }
                    case KEY_MS_TEAMS_BTN_INVOKE: {
                        if (!s_teams_connected) {
                            APPS_LOG_MSGID_I(TAG"TEAMS not connected, btn invoke will not work.", 0);
                            app_ms_teams_set_foreground_led(APP_MS_TEAMS_UI_EVENT_INVOKE_FAIL);
                            break;
                        }
                        ms_teams_btn_press_type_t type = MS_TEAMS_BTN_PRESS_TYPE_SHORT_PRESS;
                        ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
                        type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
                        ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_RELEASE, &type, sizeof(ms_teams_btn_press_type_t));
                        ret = true;
                        break;
                    }
                }
            } else if (pkg->event_group == EVENT_GROUP_UI_SHELL_MS_TEAMS) {
                APPS_LOG_MSGID_I(TAG"client connected, update basic info, datalen=%d,byte0=%d.", 2, pkg->extra_data_len, pkg->data[0]);
                uint32_t teams_event = (pkg->event_id >> 16) & 0xffff;
                if (teams_event == MS_TEAMS_EVENT_VER_SYNC) {
                    uint8_t *product_info = (uint8_t *)&pkg->data;
                    uint32_t str_len = strnlen((const char*)product_info, pkg->extra_data_len);
                    app_ms_teams_set_endpoint_fw_version(product_info, str_len);
                    product_info = &product_info[str_len + 1];
                    str_len = strnlen((const char*)product_info, pkg->extra_data_len - str_len);
                    app_ms_teams_set_endpoint_device_model_id(product_info, str_len);
                    product_info = &product_info[str_len + 1];
                    str_len = strnlen((const char*)product_info, pkg->extra_data_len - str_len);
                    app_ms_teams_set_endpoint_device_SN(product_info, str_len);
                    app_ms_teams_set_link_quality(APP_MS_TEAMS_TELEMETRY_LINK_QUALITY_HIGH);
#ifdef AIR_LE_AUDIO_ENABLE
                    static uint8_t s_last_group_id = 0xff;
                    uint32_t group_idx = 0;
                    for (; group_idx < APP_LE_AUDIO_UCST_LINK_MAX_NUM; group_idx++) {
                        app_le_audio_ucst_link_info_t *link_info = app_le_audio_ucst_get_link_info_by_idx(group_idx);
                        if (link_info) {
                            if (link_info->group_id == 0xff) {
                                continue;
                            }
                        }
                        if (link_info && link_info->group_id != s_last_group_id) {
                            app_ms_teams_set_connected_wireless_device_changed(true);
                            s_last_group_id = link_info->group_id;
                            break;
                        }
                        bt_hci_cmd_read_rssi_t read_rssi = {
                            .handle = link_info->handle,
                        };
                        bt_gap_le_read_rssi(&read_rssi);
                    }
                    if (group_idx >= APP_LE_AUDIO_UCST_LINK_MAX_NUM) {
                        app_ms_teams_set_connected_wireless_device_changed(false);
                    }
                    app_ms_teams_set_local_reference_count(app_le_audio_ucst_get_link_num());
#endif
                    app_ms_teams_set_battery_level(APP_MS_TEAMS_TELEMETRY_BATTERY_HIGH);
                    app_ms_teams_set_device_ready(true);
                } else if (teams_event == MS_TEAMS_EVENT_IN_EAR_STA) {
                    uint8_t worn = *(uint8_t *)pkg->data;
                    app_ms_teams_set_headset_worn(worn);
                } else if (teams_event == MS_TEAMS_EVENT_SIDETONE_LEVEL_SYNC) {
                    float32_t *level_diff = (float32_t *)pkg->data;
                    app_ms_teams_set_sidetone_level(*level_diff);
                } else if (teams_event == MS_TEAMS_EVENT_DSP_EFFECT_UPDATE) {
                    uint32_t effects = *(uint32_t *)pkg->data;
                    app_ms_teams_set_dsp_effect_mask(effects);
                } else if (teams_event == MS_TEAMS_EVENT_AUDIO_CODEC_CHANGED) {
                    uint8_t codec = *(uint8_t *)pkg->data;
                    app_ms_teams_set_audio_codec_used((app_ms_teams_telemetry_audio_codec_type_t)codec);
                } else if (teams_event == MS_TEAMS_EVENT_HARD_MUTE_LOCK_CHANGED) {
                    uint8_t lock = *(uint8_t *)pkg->data;
                    app_ms_teams_set_hardmute_lock(lock);
                } else if (teams_event == MS_TEAMS_EVENT_VAD_STA) {
                    uint8_t *sta = (uint8_t *)pkg->data;
                    APPS_LOG_MSGID_I(TAG"MS_TEAMS_EVENT_VAD_STA:%d.", 1, *sta);
                    app_ms_teams_set_voice_detected(*sta);
                } else if (teams_event == MS_TEAMS_EVENT_BATTERY_LEVEL_CHANGED) {
                    uint8_t level = *(uint8_t *)pkg->data;
                    app_ms_teams_set_battery_level((app_ms_teams_telemetry_battery_level_t)level);
                } else if (teams_event == MS_TEAMS_EVENT_BUTTON_PRESS_INFO_CHANGED) {
                    uint16_t data = *(uint16_t *)pkg->data;
                    ms_teams_telemetry_report_uint16_value(MS_TEAMS_TELEMETRY_BTN_PRESS_INFO, data,true); 
                }
            }
            break;
        }
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            static int32_t s_teams_race_link_nums = 0;
            if (event_id == APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN) {
                uint32_t time = 0;
                time = xTaskGetTickCount() / portTICK_PERIOD_MS / 1000;
                bool connecting = false;
                if (time < 10) {
                    connecting = true;
                }
                ret = app_ms_teams_set_background_led(s_last_hid_call_state_event,s_last_notify_ev,
                        s_teams_connected ? APP_MS_TEAMS_CONNECTION_STA_CONNECTED : (connecting ? APP_MS_TEAMS_CONNECTION_STA_CONNECTING : APP_MS_TEAMS_CONNECTION_STA_DISCONNECTED));
            } else if (event_id == APPS_EVENTS_INTERACTION_LE_AUDIO_RACE_READY) {
                APPS_LOG_MSGID_I(TAG"APPS_EVENTS_INTERACTION_LE_AUDIO_RACE_READY received, s_teams_race_link_nums = %d.", 1, s_teams_race_link_nums);
                s_teams_race_link_nums += 1;
                s_teams_race_link_nums = s_teams_race_link_nums > 2 ? 2 : s_teams_race_link_nums;
                ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_CLIENT_CONNECTED, NULL, 0);
                if (s_teams_connected) {
                    bt_bd_addr_t* addr = (bt_bd_addr_t*)bt_device_manager_get_local_address();
                    apps_dongle_sync_event_send_extra_by_address(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_CONNECTED << 16) & 0xFFFF0000, addr, sizeof(bt_bd_addr_t), (bt_bd_addr_t*)extra_data);
                    //apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_CONNECTED << 16) & 0xFFFF0000, addr, sizeof(bt_bd_addr_t));
                } else {
                    apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_DISCONNECTED << 16) & 0xFFFF0000);
                }
            } else if (event_id == APPS_EVENTS_INTERACTION_LE_AUDIO_RACE_DISCONNECTED) {
#ifdef AIR_LE_AUDIO_ENABLE
                APPS_LOG_MSGID_I(TAG"APPS_EVENTS_INTERACTION_LE_AUDIO_RACE_DISCONNECTED received, s_teams_race_link_nums = %d, link_nums=%d.", 2,
                    s_teams_race_link_nums, app_le_audio_ucst_get_link_num());
                s_teams_race_link_nums -= 1;
                if (app_le_audio_ucst_get_link_num() == 0) {
                    ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_CLIENT_DISCONNECTED, NULL, 0);
                    app_ms_teams_set_battery_level(APP_MS_TEAMS_TELEMETRY_BATTERY_OFF);
                    app_ms_teams_set_link_quality(APP_MS_TEAMS_TELEMETRY_LINK_QUALITY_OFF);
                    app_ms_teams_set_device_ready(false);
                    app_ms_teams_set_local_reference_count(app_le_audio_ucst_get_link_num());
                }
#endif
            }
            #ifdef AIR_MS_TEAMS_SPECIAL_CLIENT_ENABLE
            else if (event_id == APPS_EVENTS_INTERACTION_TEAMS_SPECIAL_CLIENT_CONNECTED) {
                extern void app_ms_teams_special_client_connect_process(void);
                app_ms_teams_special_client_connect_process();
            }
            #endif
            else if (event_id == APPS_EVENTS_INTERACTION_TEAMS_DELAY_TO_RECONNECT && !s_teams_connected) {
                extern uint32_t ms_teams_porting_usb_tx(uint8_t *buf, uint32_t len);
                uint8_t send_data[] = {0x9B, 0x01};
                ms_teams_porting_usb_tx(send_data, 2);
                send_data[1] = 0x0;
                ms_teams_porting_usb_tx(send_data, 2);
            } else if (event_id == APPS_EVENTS_INTERACTION_ESCO_CRC_RATE) {
                uint32_t quality = (uint32_t)extra_data;
                uint8_t esco_quality = (uint8_t)(quality >> 8);
                uint8_t temp_quality = esco_quality >= 254 ? 0x03 : 0x02;
                app_ms_teams_set_link_quality(temp_quality);
            } else if (event_id == APPS_EVENTS_INTERACTION_LINK_QUALITY_MONITOR) {
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_LINK_QUALITY_MONITOR, NULL, 0,
                                NULL, 2000);
                bt_app_common_read_link_quality(BT_APP_COMMON_LINK_QUALITY_BT_CRC, NULL);
            }
            break;
        }
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_MS_TEAMS_SPECIAL_CLIENT_ENABLE)
        case EVENT_GROUP_UI_SHELL_USB_HID_CALL: {
            switch (event_id) {
                case USB_HID_SRV_EVENT_CALL_UNHOLD:
                    if (s_last_hid_call_state_event == USB_HID_SRV_EVENT_CALL_END) {
                        break;
                    }
                case USB_HID_SRV_EVENT_CALL_INCOMING:
                case USB_HID_SRV_EVENT_CALL_ACTIVATE:
                    if (event_id == USB_HID_SRV_EVENT_CALL_ACTIVATE) {
                        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_LINK_QUALITY_MONITOR, NULL, 0,
                                NULL, 2000);
                    }
                case USB_HID_SRV_EVENT_CALL_END:
                case USB_HID_SRV_EVENT_CALL_HOLD:
                    if (s_last_hid_call_state_event == USB_HID_SRV_EVENT_CALL_ACTIVATE && event_id == USB_HID_SRV_EVENT_CALL_INCOMING) {
                        APPS_LOG_MSGID_I(TAG"do not update state in second incoming call.", 0);
                        break;
                    }
                    s_last_hid_call_state_event = event_id;
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                        NULL, 0);
                    if (event_id != USB_HID_SRV_EVENT_CALL_ACTIVATE) {
                        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LINK_QUALITY_MONITOR);
                    }
                    APPS_LOG_MSGID_I(TAG"s_last_hid_call_state_event updated to %d.", 1, event_id);
                    break;
            }
            break;
        }
#endif
#if 0
        case EVENT_GROUP_UI_SHELL_LE_AUDIO: {
            if (event_id == APP_LE_AUDIO_EVENT_ACTIVE_DEVICE_CHANGED) {
                app_ms_teams_set_connected_wireless_device_changed(true);
                break;
            }
            break;
        }
#endif
    }
    return ret;
}

#endif /* AIR_MS_TEAMS_ENABLE */

