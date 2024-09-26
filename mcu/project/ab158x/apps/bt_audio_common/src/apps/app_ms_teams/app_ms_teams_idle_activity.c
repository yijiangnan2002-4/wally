
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
#ifdef MTK_ANC_ENABLE
#include "anc_control_api.h"
#endif
#ifdef AIR_MS_TEAMS_ENABLE
#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
#include "usb_hid_srv.h"
#endif
#include "app_ms_teams_led.h"
#include "app_ms_teams_telemetry.h"
#include "app_ms_teams_idle_activity.h"
#include "app_ms_teams_activity.h"
#include "app_ms_teams_utils.h"
#include "bt_connection_manager.h"
#include "app_bt_state_service.h"
#include "apps_config_vp_index_list.h"
#include "voice_prompt_api.h"
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le.h"
#include "app_le_audio.h"
#endif
#include "stdlib.h"
#include "task.h"
#ifdef AIR_HEADSET_ENABLE
#include "apps_events_usb_event.h"
#endif
#include "apps_events_battery_event.h"
#include "bt_sink_srv.h"
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le.h"
#include "app_le_audio_aird_client.h"
#include "app_lea_service_conn_mgr.h"
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_utils.h"
#endif

#ifdef MTK_RACE_CMD_ENABLE
#include "race_cmd.h"
#include "race_cmd_relay_cmd.h"
#include "race_noti.h"
#include "race_bt.h"
#endif
#include "bt_app_common.h"
#include "apps_events_bt_event.h"
#include "bt_device_manager_link_record.h"

#define MS_TEAMS_NOTIFY_EV_ALIVE_TIME (5 * 60 *1000)

#define MS_TEAMS_MAX_CONNECTION_NUMS 3

#ifndef AIR_USB_HID_CALL_CTRL_ENABLE
#define USB_HID_SRV_EVENT_CALL_MAX 0xFF
#endif

#define TAG "[MS TEAMS] idle_activity "

typedef struct {
    bool    teams_connected;
    bool    call_active;
    /* Make sure the last one is active one, so it will be changed when connected/disconnected/streaming... */
    ms_teams_channel_t connected_clients[MS_TEAMS_MAX_CONNECTION_NUMS];
    uint32_t usb_connected_time;
    ms_teams_notif_sub_event_t last_notify_ev;
    uint32_t last_hid_call_state_event;
} teams_app_context_t;

static teams_app_context_t s_ctx;

#ifdef MTK_AWS_MCE_ENABLE
static bool s_aws_connected;
#endif

bool app_ms_teams_connected() {
    return s_ctx.teams_connected;
}

static bool app_ms_teams_idle_teams_ev_proc(struct _ui_shell_activity *self, uint32_t ev, uint8_t *data, uint32_t data_len, bool from_aws);

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

static void app_ms_teams_move_host_to_active(bt_bd_addr_t *addr, uint8_t type)
{
    APPS_LOG_MSGID_I(TAG"try to set type=%d to active", 1, type);
    uint8_t match_type = 0;
    uint32_t selected_one = MS_TEAMS_MAX_CONNECTION_NUMS;
    if (addr) {
        for (uint32_t i = 0; i < MS_TEAMS_MAX_CONNECTION_NUMS; i++) {
            if (memcmp(s_ctx.connected_clients[i].address, addr, sizeof(bt_bd_addr_t)) == 0) {
                selected_one = i;
                goto do_swap;
            }
        }
        return;
    }

    for (uint32_t i = 0; i < MS_TEAMS_MAX_CONNECTION_NUMS; i++) {
        if (s_ctx.connected_clients[i].channel_type == type) {
            selected_one = i;
            match_type = 1;
            break;
        }
    }
    if (selected_one == MS_TEAMS_MAX_CONNECTION_NUMS) {
        return;
    }

do_swap:
    for(uint32_t j = MS_TEAMS_MAX_CONNECTION_NUMS - 1; j > 0; j--) {
        if (j == selected_one) {
            APPS_LOG_MSGID_I(TAG"%d selected, break", 1, selected_one);
        }
        if (s_ctx.connected_clients[j].channel_type != MS_TEAMS_LINK_CHANNEL_INVALID) {
            ms_teams_channel_t temp;
            memcpy(&temp, &s_ctx.connected_clients[selected_one], sizeof(ms_teams_channel_t));
            memcpy(&s_ctx.connected_clients[selected_one], &s_ctx.connected_clients[j], sizeof(ms_teams_channel_t));
            memcpy(&s_ctx.connected_clients[j], &temp, sizeof(ms_teams_channel_t));
            APPS_LOG_MSGID_I(TAG"swap active host, old=%d, new=%d, match_type=%d", 3, j, selected_one, match_type);
            break;
        }
    }
}

static void app_ms_teams_remove_host(bt_bd_addr_t *addr, uint8_t type)
{
    bool has_connected = false;
    APPS_LOG_MSGID_I(TAG"try remove host, type=%d", 1, type);
    if (addr) {
        uint8_t *p_addr = (uint8_t*)addr;
        APPS_LOG_MSGID_I(TAG" addr [%02X:%02X:%02X:%02X:%02X:%02X]", 6, p_addr[5], p_addr[4], p_addr[3], p_addr[2], p_addr[1], p_addr[0]);
    }
    for (uint32_t i = 0; i < MS_TEAMS_MAX_CONNECTION_NUMS; i++) {
        if (addr) {
            if ((s_ctx.connected_clients[i].channel_type == type) && (memcmp(s_ctx.connected_clients[i].address, addr, sizeof(bt_bd_addr_t)) == 0)) {
                APPS_LOG_MSGID_I(TAG"remove host by addr, type=%d, idx=%d", 2, type, i);
                memset(&s_ctx.connected_clients[i], 0, sizeof(ms_teams_channel_t));
                s_ctx.connected_clients[i].channel_type = MS_TEAMS_LINK_CHANNEL_INVALID;
            }
        } else if (s_ctx.connected_clients[i].channel_type == type) {
            s_ctx.connected_clients[i].channel_type = MS_TEAMS_LINK_CHANNEL_INVALID;
        }
        if (s_ctx.connected_clients[i].channel_type != MS_TEAMS_LINK_CHANNEL_INVALID) {
            has_connected = true;
        }
    }

    /* make sure the incoming one always listed at the end of list. */
    for (uint32_t i = 0; i < MS_TEAMS_MAX_CONNECTION_NUMS - 1; i++) {
        if (s_ctx.connected_clients[i].channel_type == MS_TEAMS_LINK_CHANNEL_INVALID) {
            memcpy(&s_ctx.connected_clients[i], &s_ctx.connected_clients[i + 1], sizeof(ms_teams_channel_t));
            s_ctx.connected_clients[i + 1].channel_type = MS_TEAMS_LINK_CHANNEL_INVALID;
        }
    }

    s_ctx.teams_connected = has_connected;
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN);
    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                        NULL, 0);
}

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
        app_ms_teams_idle_teams_ev_proc(NULL, p2, NULL, 0, true);
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
        } else if (p2 == 6) {
            app_ms_teams_set_error_message((uint8_t*)"unknown errors.", strlen("unknown errors."));
        }
    } else if (p1 == 2) {
         ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_USB_PLUG_STATE,
                                (void *)p2, 0, NULL, 0);
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
            s_ctx.last_notify_ev = MS_TEAMS_NOTIF_EVENT_NONE;
            break;
        case MS_TEAMS_NOTIF_EVENT_MISSED_CALL:
        case MS_TEAMS_NOTIF_EVENT_UPCOMING_SCHEDULED_MEETING:
        case MS_TEAMS_NOTIF_EVENT_UNCHECKE_VOICE_MAIL: {
#ifdef AIR_HEADSET_ENABLE
            voice_prompt_play_vp_succeed();
#else
            voice_prompt_play_sync_vp_succeed();
                    APPS_LOG_MSGID_I(", harrtdbg VP_INDEX_SUCCEED 21 ", 0);
#endif
            s_ctx.last_notify_ev = ev;
            /* Clear the notify state */
            shell_event = ((MS_TEAMS_EVENT_NOTIFY << 16) & 0xFFFF0000) | 0xFFFF;
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_MS_TEAMS, shell_event);
            ui_shell_send_event(true, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_MS_TEAMS,
                                shell_event,
                                NULL, 0, NULL, MS_TEAMS_NOTIFY_EV_ALIVE_TIME);
            /* Use the incoming call's led UI to indicate these event. */
#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
            if (s_ctx.last_hid_call_state_event == USB_HID_SRV_EVENT_CALL_END || s_ctx.last_hid_call_state_event == USB_HID_SRV_EVENT_CALL_MAX)
#else
            if (s_ctx.last_hid_call_state_event == USB_HID_SRV_EVENT_CALL_MAX)
#endif
            {
                APPS_LOG_MSGID_I(TAG"not in call state, update led to notification event.", 0);
                app_ms_teams_set_background_led(USB_HID_SRV_EVENT_CALL_END, s_ctx.last_notify_ev, APP_MS_TEAMS_CONNECTION_STA_CONNECTED);
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
                s_ctx.last_notify_ev = MS_TEAMS_NOTIF_EVENT_NONE;
                break;
            }
    }

    return true;
}

static bool app_ms_teams_idle_teams_ev_proc(struct _ui_shell_activity *self, uint32_t ev, uint8_t *data, uint32_t data_len, bool from_aws)
{
    ms_teams_event_t event = ((ev >> 16) & 0xFFFF);

    APPS_LOG_MSGID_I(TAG"teams event proc, 0x%x, 0x%x.", 2, ev, event);
#ifdef MTK_AWS_MCE_ENABLE
    if ((event == MS_TEAMS_CONNECTED || event == MS_TEAMS_DISCONNECTED) && !from_aws) {
        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, ev, data, data_len);
    }
#endif

    switch (event) {
        case MS_TEAMS_CONNECTED: {
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                NULL, 0);
            ms_teams_channel_t *con_ch = (ms_teams_channel_t*)data;
            APPS_LOG_MSGID_I(TAG"channel=%d connected.", 1, con_ch->channel_type);
            uint32_t i = 0;
            uint32_t connected_nums = 0;
            for (i = 0; i < MS_TEAMS_MAX_CONNECTION_NUMS; i++) {
                if (s_ctx.connected_clients[i].channel_type != MS_TEAMS_LINK_CHANNEL_INVALID) {
                    connected_nums += 1;
                }
                if (s_ctx.connected_clients[i].channel_type == MS_TEAMS_LINK_CHANNEL_HID && con_ch->channel_type == MS_TEAMS_LINK_CHANNEL_HID) {
                    if (!s_ctx.call_active) {
                        app_ms_teams_move_host_to_active(NULL, MS_TEAMS_LINK_CHANNEL_HID);
                    }
                }
                #if 0
                else if (s_ctx.connected_clients[i].channel_type == MS_TEAMS_LINK_CHANNEL_RACE && con_ch->channel_type == MS_TEAMS_LINK_CHANNEL_RACE) {
                    if (!s_ctx.call_active) {
                        app_ms_teams_move_host_to_active(NULL, MS_TEAMS_LINK_CHANNEL_RACE);
                    }
                
                }
                #endif
                else if (s_ctx.connected_clients[i].channel_type == MS_TEAMS_LINK_CHANNEL_GATT || s_ctx.connected_clients[i].channel_type == MS_TEAMS_LINK_CHANNEL_RACE) {
                    if (memcmp(&s_ctx.connected_clients[i].address, &con_ch->address, sizeof(bt_bd_addr_t)) == 0) {
                        APPS_LOG_MSGID_I(TAG"duplicated connection=%d.", 1, i);
                        break;
                    }
                } else if (s_ctx.connected_clients[i].channel_type == MS_TEAMS_LINK_CHANNEL_INVALID) {
                    if (s_ctx.call_active && i > 0) {
                        ms_teams_channel_t temp;
                        memcpy(&temp, &s_ctx.connected_clients[i - 1], sizeof(ms_teams_channel_t));
                        memcpy(&s_ctx.connected_clients[i - 1], con_ch, sizeof(ms_teams_channel_t));
                        memcpy(&s_ctx.connected_clients[i], &temp, sizeof(ms_teams_channel_t));
                    } else {
                        memcpy(&s_ctx.connected_clients[i], con_ch, sizeof(ms_teams_channel_t));
                        APPS_LOG_MSGID_I(TAG"new connection put to pos=%d.", 1, i);
                        APPS_LOG_MSGID_I(TAG" addr [%02X:%02X:%02X:%02X:%02X:%02X]", 6,
                             con_ch->address[5], con_ch->address[4], con_ch->address[3],
                             con_ch->address[2], con_ch->address[1], con_ch->address[0]);
                    }
                    connected_nums += 1;
                    s_ctx.teams_connected = true;
                    break;
                }
            }
            if (i >= MS_TEAMS_MAX_CONNECTION_NUMS) {
                APPS_LOG_MSGID_I(TAG"connection full=%d.", 1, i);
            }
            break;
        }
        case MS_TEAMS_DISCONNECTED: {
            uint32_t connected_nums = 0;
            ms_teams_channel_t *dis_ch = (ms_teams_channel_t*)data;
            for (uint32_t i = 0; i < MS_TEAMS_MAX_CONNECTION_NUMS; i++) {
                if (s_ctx.connected_clients[i].channel_type != MS_TEAMS_LINK_CHANNEL_INVALID) {
                    connected_nums += 1;
                }
            }
            APPS_LOG_MSGID_I(TAG"dis_ch=%d disconnected, now_links=%d.", 2, dis_ch->channel_type, connected_nums);
            app_ms_teams_remove_host(&dis_ch->address, dis_ch->channel_type);
            break;
        }
        case MS_TEAMS_EVENT_LOCALE:
            break;
        case MS_TEAMS_EVENT_NOTIFY: {
#ifdef MTK_AWS_MCE_ENABLE
            bt_aws_mce_role_t role;
            bt_status_t ret = BT_STATUS_SUCCESS;
            role = bt_device_manager_aws_local_info_get_role();
            if (role == BT_AWS_MCE_ROLE_PARTNER) {
                /* If rho switched and the "None" notification not sent, the new Agent will use this event to keep a timer to update the state. */
                ret = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, ev, data, data_len);
                APPS_LOG_MSGID_I(TAG"partner notify event to agent ret=%d.", 1, ret);
                return true;
            }
#endif
            app_ms_teams_ev_notify_handler((ms_teams_notif_sub_event_t)(ev & 0xFFFF), data, data_len);
            //apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_MS_TEAMS, ev); /* Do not send status to other side on headset/earbuds. */
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

#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
void app_ms_teams_mute_speaking_callback(bool speaking)
{
    //APPS_LOG_MSGID_I(TAG"speaking callback=%d", 1, speaking);
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_VOICE_ACTIVE_WHILE_MUTED_UPDATE,
                                (void*)speaking, 0, NULL, 0);
}

#include "hal_audio_message_struct.h"
extern void audio_mute_speaking_detection_callback_register(mute_speaking_detection_callback_t FunPtr);
#endif
static bool _proc_ui_shell_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    if (event_id == EVENT_ID_SHELL_SYSTEM_ON_CREATE) {
        app_ms_teams_load_default_telemetry_setting();
        app_ms_teams_init();
#ifdef MS_TEAMS_TEST
        app_ms_teams_test_init();
#endif
        app_ms_teams_common_atci_init();
            s_ctx.last_hid_call_state_event = USB_HID_SRV_EVENT_CALL_MAX;
            for (uint32_t i = 0; i < MS_TEAMS_MAX_CONNECTION_NUMS; i++) {
                s_ctx.connected_clients[i].channel_type = MS_TEAMS_LINK_CHANNEL_INVALID;
            }
        /* Connection timeout, set LED off. */
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_UPDATE_TEAMS_LED_BG_PATTERN, NULL, 0,
                            NULL, 10 * 1000);
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
        audio_mute_speaking_detection_callback_register(app_ms_teams_mute_speaking_callback);
#endif
    }
    return true;
}

void ms_teams_send_telemetry_update_notify_to_active_host()
{
    /* find active one. */
    uint32_t client_idx = MS_TEAMS_MAX_CONNECTION_NUMS - 1;
    for (; client_idx > 0; client_idx--) {
        if (s_ctx.connected_clients[client_idx].channel_type != MS_TEAMS_LINK_CHANNEL_INVALID) {
            break;
        }
    }
    APPS_LOG_MSGID_I(TAG"send telemetry notify to active channel, type=%d, index=%d.", 2, s_ctx.connected_clients[client_idx].channel_type, client_idx);
    ms_teams_btn_press_type_t type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
    ms_teams_send_action_to_client(&s_ctx.connected_clients[client_idx], MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
    type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
    ms_teams_send_action_to_client(&s_ctx.connected_clients[client_idx], MS_TEAMS_ACTION_TEAMS_BTN_RELEASE, &type, sizeof(ms_teams_btn_press_type_t));
}

static bool _proc_key_action(apps_config_key_action_t action, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    if (action == KEY_MS_TEAMS_BTN_LONG_PRESS || action == KEY_MS_TEAMS_BTN_INVOKE || action == KEY_MS_TEAMS_BTN_RELEASE) {
        if (!s_ctx.teams_connected) {
            APPS_LOG_MSGID_I(TAG"TEAMS not connected, btn invoke will not work.", 0);
            apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_KEY, event_id, extra_data, data_len);
            app_ms_teams_set_foreground_led(APP_MS_TEAMS_UI_EVENT_INVOKE_FAIL);
            return false;
        }
    }

    /* find active one. */
    uint32_t client_idx = MS_TEAMS_MAX_CONNECTION_NUMS - 1;
    for (; client_idx > 0; client_idx--) {
        if (s_ctx.connected_clients[client_idx].channel_type != MS_TEAMS_LINK_CHANNEL_INVALID) {
            break;
        }
    }
    APPS_LOG_MSGID_I(TAG"find active channel, type=%d, index=%d.", 2, s_ctx.connected_clients[client_idx].channel_type, client_idx);

    switch (action) {
        case KEY_REDIAL_LAST_CALL: {
            ms_teams_send_action(MS_TEAMS_ACTION_CALL_DIAL_OUT, NULL, 0);
            ret = true;
        }
        break;
        case KEY_MS_TEAMS_BTN_LONG_PRESS: {
            if (s_ctx.connected_clients[client_idx].channel_type != MS_TEAMS_LINK_CHANNEL_RACE) {
                ms_teams_btn_press_type_t type = MS_TEAMS_BTN_PRESS_TYPE_LONG_PRESS;
                ms_teams_send_action_to_client(&s_ctx.connected_clients[client_idx], MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
                type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
                ms_teams_send_action_to_client(&s_ctx.connected_clients[client_idx], MS_TEAMS_ACTION_TEAMS_BTN_RELEASE, &type, sizeof(ms_teams_btn_press_type_t));
            } else {
                apps_dongle_sync_event_send_extra_by_address(EVENT_GROUP_UI_SHELL_KEY, event_id, extra_data, data_len, &s_ctx.connected_clients[client_idx].address);
            }
            ret = true;
            break;
        }
        case KEY_MS_TEAMS_BTN_INVOKE: {
            if (s_ctx.connected_clients[client_idx].channel_type == MS_TEAMS_LINK_CHANNEL_RACE) {
                apps_dongle_sync_event_send_extra_by_address(EVENT_GROUP_UI_SHELL_KEY, event_id, extra_data, data_len, &s_ctx.connected_clients[client_idx].address);
            } else {
                ms_teams_btn_press_type_t type = MS_TEAMS_BTN_PRESS_TYPE_SHORT_PRESS;
                ms_teams_send_action_to_client(&s_ctx.connected_clients[client_idx], MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
                type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
                ms_teams_send_action_to_client(&s_ctx.connected_clients[client_idx], MS_TEAMS_ACTION_TEAMS_BTN_RELEASE, &type, sizeof(ms_teams_btn_press_type_t));
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
    if (!s_ctx.teams_connected) {
        ;
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
#ifdef AIR_LE_AUDIO_ENABLE
        if (app_le_audio_is_connected() && !s_aws_connected) {
            APPS_LOG_MSGID_I(TAG"partner will process key event during le audio connect but aws not.", 0);
        } else {
            if (action == KEY_REDIAL_LAST_CALL || action == KEY_MS_TEAMS_BTN_INVOKE || action == KEY_MS_TEAMS_BTN_RELEASE) {
                ret = apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action);
                APPS_LOG_MSGID_I(TAG"partner sync key action=%d to agent ret=%d.", 2, action, ret);
                return true;
            }
        }
#else
        if (action == KEY_REDIAL_LAST_CALL || action == KEY_MS_TEAMS_BTN_INVOKE || action == KEY_MS_TEAMS_BTN_RELEASE) {
            ret = apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action);
            APPS_LOG_MSGID_I(TAG"partner sync key action=%d to agent ret=%d.", 2, action, ret);
            return true;
        }
#endif
    }
#endif
    ret = _proc_key_action(action, event_id, extra_data, data_len);
    return ret;
}

static bool _proc_bt_cm_event(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
#ifdef MTK_AWS_MCE_ENABLE
    bt_cm_remote_info_update_ind_t *info = (bt_cm_remote_info_update_ind_t *)extra_data;
    if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & info->pre_connected_service)
        && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & info->connected_service)) {
        s_aws_connected = false;
    } else if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & info->pre_connected_service)
               && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & info->connected_service)) {
        s_aws_connected = true;
    }
#endif
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_TEAMS_DELAY_GET_CONN_NUMS);
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_TEAMS_DELAY_GET_CONN_NUMS,
                                    NULL, 0, NULL, 1000);
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
            apps_config_key_action_t key_action = (apps_config_key_action_t)action;
            APPS_LOG_MSGID_I(TAG"receive partner key action: %d.", 1, key_action);
            ret = _proc_key_action(action, event_id, &key_action, sizeof(apps_config_key_action_t));
        } else if (event_group == EVENT_GROUP_UI_SHELL_MS_TEAMS) {
            app_ms_teams_idle_teams_ev_proc(self, action, p_extra_data, extra_data_len, true);
        }
    }

    /* TODO: handle the teams event come from Agent. */
    return ret;
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
#ifdef AIR_USB_ENABLE
        case EVENT_GROUP_UI_SHELL_USB_AUDIO: {
            if (event_id == APPS_EVENTS_USB_CONFIG_DONE) {
                ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_TEAMS_DELAY_TO_RECONNECT,
                                    NULL, 0, NULL, 3000);
            } else if (event_id == APPS_EVENTS_USB_AUDIO_PLAY) {
                app_ms_teams_move_host_to_active(NULL, MS_TEAMS_LINK_CHANNEL_HID);
            } else if (event_id == APPS_EVENTS_USB_AUDIO_UNPLUG) {
                app_ms_teams_remove_host(NULL, MS_TEAMS_LINK_CHANNEL_HID);
            }
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_MS_TEAMS:
            ret = app_ms_teams_idle_teams_ev_proc(self, event_id, extra_data, data_len, false);
            break;

#ifdef MTK_AWS_MCE_ENABLE
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            /* The event come from partner. */
            ret = _proc_aws_data(self, event_id, extra_data, data_len);
            break;
#endif
        case EVENT_GROUP_UI_SHELL_BT_SINK: {
            #if AIR_BT_HFP_ENABLE
            extern bool app_hfp_get_active_device_addr(bt_bd_addr_t *active_addr);
            #endif
            if (event_id == BT_SINK_SRV_EVENT_STATE_CHANGE) {
                bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *) extra_data;
                if (param->previous < BT_SINK_SRV_STATE_ACTIVE && param->current >= BT_SINK_SRV_STATE_ACTIVE) {
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_LINK_QUALITY_MONITOR, NULL, 0,
                                NULL, 2000);
                } else if (param->previous >= BT_SINK_SRV_STATE_ACTIVE && param->current < BT_SINK_SRV_STATE_ACTIVE) {
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LINK_QUALITY_MONITOR);
                }
                if (param->current < BT_SINK_SRV_STATE_STREAMING) {
                    break;
                }
                #if AIR_BT_HFP_ENABLE
                /* get active one */
                bt_bd_addr_t addr;
                if (app_hfp_get_active_device_addr(&addr)) {
                    app_ms_teams_move_host_to_active(&addr, MS_TEAMS_LINK_CHANNEL_RACE);
                }
                #endif
                if (param->current >= BT_SINK_SRV_STATE_ACTIVE) {
                    s_ctx.call_active = true;
                } else {
                    s_ctx.call_active = false;
                }
            }
#ifdef AIR_LE_AUDIO_ENABLE
            else if (event_id == LE_SINK_SRV_EVENT_REMOTE_INFO_UPDATE) {
                /* get active one */
                #if 0
                bt_bd_addr_t addr;
                if (app_hfp_get_active_device_addr(&addr)) {
                    app_ms_teams_move_host_to_active(&addr, MS_TEAMS_LINK_CHANNEL_RACE);
                }
                #endif
                bt_le_sink_srv_event_remote_info_update_t *update_ind = (bt_le_sink_srv_event_remote_info_update_t *)extra_data;
                #if 0
                if (update_ind->connected_service & BT_SINK_SRV_PROFILE_CALL) {
                    s_ctx.call_active = true;
                } else {
                    s_ctx.call_active = false;
                }
                #endif
                /* check disconnect information */
                if (update_ind->pre_state == BT_BLE_LINK_CONNECTED && update_ind->state == BT_BLE_LINK_DISCONNECTED) {
                    bt_handle_t handle = app_lea_conn_mgr_get_handle_by_addr((uint8_t*)&update_ind->address.addr);
                    if (app_le_audio_aird_client_is_support(handle)) {
                        app_ms_teams_remove_host(&update_ind->address.addr, MS_TEAMS_LINK_CHANNEL_RACE);
                    }
                }
                #if 0
                else if (update_ind->state == BT_BLE_LINK_CONNECTED && update_ind->pre_state == BT_BLE_LINK_DISCONNECTED) {
                    bt_handle_t handle = app_lea_conn_mgr_get_handle_by_addr((uint8_t*)&update_ind->address.addr);
                    if (app_le_audio_aird_client_is_support(handle)) {
                        for (uint32_t i = 0; i < MS_TEAMS_MAX_CONNECTION_NUMS - 1; i++) {
                            if (s_ctx.connected_clients[i].channel_type == MS_TEAMS_LINK_CHANNEL_RACE) {
                                memcpy(&s_ctx.connected_clients[i].address, &update_ind->address.addr, sizeof(bt_bd_addr_t));
                            }
                        }
                    }
                }
                #endif
            }
#endif
            break;
        }
        case EVENT_GROUP_UI_SHELL_BATTERY: {
            if (event_id == APPS_EVENTS_BATTERY_PERCENT_CHANGE) {
                int32_t battery = (int32_t)extra_data;
                app_ms_teams_telemetry_battery_level_t level;
                if (battery > 70) {
                    level = APP_MS_TEAMS_TELEMETRY_BATTERY_HIGH;
                } else if (battery > 30) {
                    level = APP_MS_TEAMS_TELEMETRY_BATTERY_MIDDLE;
                } else {
                    level = APP_MS_TEAMS_TELEMETRY_BATTERY_CRITICALLY_LOW;
                }
                app_ms_teams_set_battery_level(level);
            }
            break;
        }
#ifdef MTK_ANC_ENABLE
        case EVENT_GROUP_UI_SHELL_AUDIO_ANC: {
            if (event_id == AUDIO_ANC_CONTROL_EVENT_ON
                || event_id == AUDIO_ANC_CONTROL_EVENT_OFF) {
                app_teams_telemetry_info *info = app_ms_teams_get_telemetry_info();
                if (event_id == AUDIO_ANC_CONTROL_EVENT_ON) {
                    info->dsp_effect |= 0x4;
                } else {
                    info->dsp_effect &= (~0x4);
                }
                app_ms_teams_set_dsp_effect_mask(info->dsp_effect);
            }
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_DONGLE_DATA: {
            apps_dongle_event_sync_info_t *pkg = (apps_dongle_event_sync_info_t *)extra_data;
            /* EVENT_GROUP_UI_SHELL_MS_TEAMS is 30 and it must be same on dongle and earbuds!!! */
            if (pkg->event_group == EVENT_GROUP_UI_SHELL_MS_TEAMS) {
                ms_teams_event_t teams_event_type = ((pkg->event_id >> 16) & 0xFFFF);
                APPS_LOG_MSGID_I(TAG"dongle teams event received, ev=%d.", 1, pkg->event_id);
                switch (teams_event_type) {
                    /* The Teams application handshake with dongle done. */
                    case MS_TEAMS_CONNECTED: {
                        ms_teams_channel_t race_channel = {0};
                        if (pkg->extra_data_len) {
                            memcpy(&race_channel.address, &pkg->data, sizeof(bt_bd_addr_t));
                        } else {
                            uint8_t channel_id = apps_dongle_sync_event_get_channel_id(extra_data, data_len);
                            bt_bd_addr_t *dongle_addr = race_get_bt_connection_addr(channel_id);
                            if (dongle_addr) {
                                memcpy(&race_channel.address, dongle_addr, sizeof(bt_bd_addr_t));
                            } else {
                                break;
                            }
                        }
                        race_channel.channel_type = MS_TEAMS_LINK_CHANNEL_RACE;
                        app_ms_teams_idle_teams_ev_proc(self, MS_TEAMS_CONNECTED << 16, (uint8_t*)&race_channel, sizeof(ms_teams_channel_t), false);
                        /* Sync the fw version and SN and model id to dongle */
                        const uint8_t *fw = (const uint8_t *)app_ms_teams_get_telemetry_info()->endpoint_fw;
                        const uint8_t *model_id = (const uint8_t *)app_ms_teams_get_telemetry_info()->endpoint_mode_id;
                        const uint8_t *sn = (const uint8_t *)app_ms_teams_get_telemetry_info()->endpoint_sn;
                        uint32_t len1 = strnlen((const char *)fw, 32);
                        uint32_t len2 = strnlen((const char *)model_id, 32);
                        uint32_t len3 = strnlen((const char *)sn, 32);
                        uint32_t sync_len = len1 + len2 + len3 + 3;
                        uint8_t *sync_info = pvPortMalloc(sync_len);
                        if (sync_info == NULL) {
                            break;
                        }
                        memset(sync_info, 0, sync_len);
                        memcpy(sync_info, fw, len1);
                        memcpy(&sync_info[len1 + 1], model_id, len2);
                        memcpy(&sync_info[len1 + len2 + 2], sn, len3);
                        apps_dongle_sync_event_send_extra_by_address(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_EVENT_VER_SYNC << 16) & 0xFFFF0000,
                                                          sync_info, sync_len, &race_channel.address);
                        vPortFree(sync_info);
                        break;
                    }
                    case MS_TEAMS_EVENT_NOTIFY: {
                        app_ms_teams_idle_teams_ev_proc(self, pkg->event_id, pkg->data, pkg->extra_data_len, false);
                        break;
                    }
                }
            }
            break;
        }

        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            if (event_id == APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN || event_id == APPS_EVENTS_INTERACTION_UPDATE_TEAMS_LED_BG_PATTERN) {
                if (event_id == APPS_EVENTS_INTERACTION_UPDATE_TEAMS_LED_BG_PATTERN && s_ctx.teams_connected) {
                    /* Do not update LED again if Teams already connected, due to maybe already in call state. */
                    break;
                }
                uint32_t time = 0;
                time = xTaskGetTickCount() / portTICK_PERIOD_MS / 1000;
                bool connecting = false;
                if (time < 10) {
                    connecting = true;
                }
                ret = app_ms_teams_set_background_led(
#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
                    s_ctx.last_hid_call_state_event
#else
                    USB_HID_SRV_EVENT_CALL_MAX
#endif
                    , s_ctx.last_notify_ev,
                        s_ctx.teams_connected ? APP_MS_TEAMS_CONNECTION_STA_CONNECTED : (connecting ? APP_MS_TEAMS_CONNECTION_STA_CONNECTING : APP_MS_TEAMS_CONNECTION_STA_DISCONNECTED));
            } else if (event_id == APPS_EVENTS_INTERACTION_VOICE_ACTIVE_WHILE_MUTED_UPDATE) {
                uint8_t det = (uint8_t)(uint32_t)extra_data;
                APPS_LOG_MSGID_I(TAG"vad sta updated:%d.", 1, det);
                app_ms_teams_set_voice_detected(det);
            }
#ifdef AIR_HEADSET_ENABLE
            else if (event_id == APPS_EVENTS_INTERACTION_USB_PLUG_STATE) {
                bool plug_in = (bool)extra_data;
                APPS_LOG_MSGID_I(TAG"APPS_EVENTS_INTERACTION_USB_PLUG_STATE received, usb state=%d.", 1, plug_in);
                if (!plug_in) {
                    ms_teams_channel_t usb_channel;
                    memset(&usb_channel, 0, sizeof(usb_channel));
                    usb_channel.channel_type = MS_TEAMS_LINK_CHANNEL_HID;
                    app_ms_teams_idle_teams_ev_proc(self, MS_TEAMS_DISCONNECTED << 16, (uint8_t*)&usb_channel, sizeof(ms_teams_channel_t), false);
                    s_ctx.last_notify_ev = MS_TEAMS_NOTIF_EVENT_NONE;
                    /* usb plug out, set LED off. */
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                        NULL, 0);
                } else {
                    /* set to usb type even if teams not connected due to le-audio will be disconnected when usb plug in. */
                    s_ctx.usb_connected_time = xTaskGetTickCount() / portTICK_PERIOD_MS / 1000;
                    /* usb plug in, set LED to connection state. */
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                        NULL, 0);
                    /* Connection timeout, set LED off. */
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                        NULL, 10 * 1000);
                }
            }
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
            else if (event_id == APPS_EVENTS_INTERACTION_IN_EAR_UPDATE_STA) {
                app_in_ear_sta_info_t *sta_info = (app_in_ear_sta_info_t *)extra_data;
                if (sta_info->current == APP_IN_EAR_STA_BOTH_OUT) {
                    app_ms_teams_set_headset_worn(false);
                } else {
                    app_ms_teams_set_headset_worn(true);
                }
            }
#endif
            else if (event_id == APPS_EVENTS_INTERACTION_TEAMS_DELAY_TO_RECONNECT && !s_ctx.teams_connected) {
                extern uint32_t ms_teams_porting_usb_tx(uint8_t *buf, uint32_t len);
                uint8_t send_data[] = {0x9B, 0x01};
                ms_teams_porting_usb_tx(send_data, 2);
                send_data[1] = 0x0;
                ms_teams_porting_usb_tx(send_data, 2);
            } else if (event_id == APPS_EVENTS_INTERACTION_LINK_QUALITY_MONITOR) {
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_LINK_QUALITY_MONITOR, NULL, 0,
                                NULL, 2000);
                bt_app_common_read_link_quality(BT_APP_COMMON_LINK_QUALITY_BT_CRC, NULL);
            } else if (event_id == APPS_EVENTS_INTERACTION_ESCO_CRC_RATE) {
                uint32_t quality = (uint32_t)extra_data;
                uint8_t esco_quality = (uint8_t)(quality >> 8);
                uint8_t temp_quality = esco_quality >= 254 ? 0x03 : 0x02;
                app_ms_teams_set_link_quality(temp_quality);
            } else if (event_id == APPS_EVENTS_INTERACTION_TEAMS_DELAY_GET_CONN_NUMS) {
                const bt_device_manager_link_record_t *link_record = bt_device_manager_link_record_get_connected_link();
                if (link_record) {
                    app_ms_teams_set_local_reference_count(link_record->connected_num);
                }
            }
            break;
        }
#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
        case EVENT_GROUP_UI_SHELL_USB_HID_CALL: {
            switch (event_id) {
                case USB_HID_SRV_EVENT_CALL_UNHOLD:
                    if (s_ctx.last_hid_call_state_event == USB_HID_SRV_EVENT_CALL_END) {
                        break;
                    }
                case USB_HID_SRV_EVENT_CALL_INCOMING:
                case USB_HID_SRV_EVENT_CALL_ACTIVATE:
                case USB_HID_SRV_EVENT_CALL_END:
                case USB_HID_SRV_EVENT_CALL_HOLD:
                    if (s_ctx.last_hid_call_state_event == USB_HID_SRV_EVENT_CALL_ACTIVATE && event_id == USB_HID_SRV_EVENT_CALL_INCOMING) {
                        APPS_LOG_MSGID_I(TAG"do not update state in second incoming call.", 0);
                        break;
                    }
                    s_ctx.last_hid_call_state_event = event_id;
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                        NULL, 0);
                    APPS_LOG_MSGID_I(TAG"s_last_hid_call_state_event updated to %d.", 1, event_id);
                    if (event_id == USB_HID_SRV_EVENT_CALL_ACTIVATE) {
                        app_ms_teams_move_host_to_active(NULL, MS_TEAMS_LINK_CHANNEL_HID);
                    }
                    break;
            }
            break;
        }
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
    }
    return ret;
}

#endif /* AIR_MS_TEAMS_ENABLE */

