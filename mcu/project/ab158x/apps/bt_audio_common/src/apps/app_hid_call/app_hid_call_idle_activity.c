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
/* Airoha restricted information */
#include "app_hid_call_idle_activity.h"
#include "ui_shell_activity.h"
#include "apps_config_event_list.h"
#include "apps_config_state_list.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "FreeRTOS.h"
#ifdef AIR_AIRO_KEY_ENABLE
#include "airo_key_event.h"
#endif
#include "apps_events_key_event.h"
#include "apps_events_interaction_event.h"
#include "apps_config_key_remapper.h"
#include "apps_config_led_manager.h"
#include "apps_config_led_index_list.h"
#include "usbaudio_drv.h"
#include "apps_customer_config.h"
#include "usb_hid_srv.h"

#define TAG "APP HID CALL"
#ifdef AIR_MS_TEAMS_ENABLE
#include "app_ms_teams_utils.h"
#include "ms_teams.h"
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) && 0
#include "bt_ull_le_call_service.h"
#endif
#ifdef AIR_MS_TEAMS_ENABLE
#include "app_ms_teams_telemetry.h"
#endif
#ifdef AIR_USB_AUDIO_OUT_ENABLE
#include "app_usb_audio_idle_activity.h"
#endif

static bool s_muted = false;
static usb_hid_srv_event_t s_last_call_state = USB_HID_SRV_EVENT_CALL_END;
static apps_config_key_action_t s_key_action_pre_parse_result = KEY_ACTION_INVALID;

static bool _proc_ui_shell_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(TAG"create", 0);
            break;
        }
        default:
            break;
    }
    return ret;
}

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) && 0
void app_hid_call_send_action(uint8_t action)
{
    static bt_ull_le_srv_call_event_t remap_table[] = {
        BT_ULL_LE_SRV_CALL_ACTION_ANSWER,
        BT_ULL_LE_SRV_CALL_ACTION_REJECT,
        BT_ULL_LE_SRV_CALL_ACTION_NONE,
        BT_ULL_LE_SRV_CALL_ACTION_NONE,
        BT_ULL_LE_SRV_CALL_ACTION_TERMINATE,
        BT_ULL_LE_SRV_CALL_ACTION_MUTE,
        BT_ULL_LE_SRV_CALL_ACTION_UNMUTE,
    };
    bt_ull_le_srv_call_event_t send_action = (bt_ull_le_srv_call_event_t)remap_table[action];
    bt_status_t sta = bt_ull_le_call_srv_send_action(send_action);
    APPS_LOG_MSGID_I(TAG"send ull2 call action=%d, raw_action=%d, ret=%d", 3, action, send_action, sta);
}
#else
void app_hid_call_send_action(uint8_t action)
{
    usb_hid_srv_send_action((usb_hid_srv_action_t)action, NULL);
}
#endif

static bool _proc_key_event(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = true;
    apps_config_state_t mmi_sta = apps_config_key_get_mmi_state();
    if (mmi_sta >= APP_HFP_INCOMING && mmi_sta <= APP_HFP_MULTIPARTY_CALL) {
        ;/* The key event will remap by app_hid_call_coexist_hfp in hfp activity. */
    } else {
        app_hid_call_coexist_hfp(event_id, KEY_ACTION_INVALID);
    }

    if (s_key_action_pre_parse_result != KEY_ACTION_INVALID) {
        APPS_LOG_MSGID_I(TAG"_proc_key_event, pre parse_result=%d", 1, s_key_action_pre_parse_result);
    }

    if (s_key_action_pre_parse_result == KEY_END_CALL ||
        s_key_action_pre_parse_result == KEY_CANCEL_OUT_GOING_CALL) {
#ifdef AIR_MS_TEAMS_ENABLE
        app_ms_teams_set_button_press_info_hook(false);
#endif
        app_hid_call_send_action(USB_HID_SRV_ACTION_TERMINATE_CALL);
    } else if (s_key_action_pre_parse_result == KEY_REJCALL) {
#ifdef AIR_MS_TEAMS_ENABLE
        app_ms_teams_set_button_press_info_hook(false);
#endif
        app_hid_call_send_action(USB_HID_SRV_ACTION_REJECT_CALL);
    } else if (s_key_action_pre_parse_result == KEY_ACCEPT_CALL) {
#ifdef AIR_MS_TEAMS_ENABLE
        app_ms_teams_set_button_press_info_hook(true);
#endif
        app_hid_call_send_action(USB_HID_SRV_ACTION_ACCEPT_CALL);
    } else if (s_key_action_pre_parse_result == KEY_ONHOLD_CALL) {
        if (s_last_call_state == USB_HID_SRV_EVENT_CALL_HOLD || s_last_call_state == USB_HID_SRV_EVENT_CALL_REMOTE_HOLD) {
#ifdef AIR_MS_TEAMS_ENABLE
            app_ms_teams_set_button_press_info_flash(false);
#endif
            app_hid_call_send_action(USB_HID_SRV_ACTION_UNHOLD_CALL);
        } else {
#ifdef AIR_MS_TEAMS_ENABLE
            app_ms_teams_set_button_press_info_flash(true);
#endif
            app_hid_call_send_action(USB_HID_SRV_ACTION_HOLD_CALL);
        }
    } else {
        ret = false;
    }
    s_key_action_pre_parse_result = KEY_ACTION_INVALID;

    if (extra_data != NULL) {
        apps_config_key_action_t action = *(uint16_t *)extra_data;
        switch (action) {
            case KEY_MUTE_MIC:
#ifdef AIR_MS_TEAMS_ENABLE
                if (s_muted) {
                    app_ms_teams_set_button_press_info_mute(false);
                } else {
                    app_ms_teams_set_button_press_info_mute(true);
                }
#endif
                app_hid_call_send_action(USB_HID_SRV_ACTION_TOGGLE_MIC_MUTE);
                break;
        }
    }

    return ret;
}

bool app_hid_call_idle_activity_proc(struct _ui_shell_activity *self,
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
        case EVENT_GROUP_UI_SHELL_USB_HID_CALL:
            if (event_id <= USB_HID_SRV_EVENT_CALL_END) {
                s_last_call_state = event_id;
            }
            if (event_id == USB_HID_SRV_EVENT_CALL_MIC_MUTE) {
                s_muted = true;
            } else if (event_id == USB_HID_SRV_EVENT_CALL_MIC_UNMUTE) {
                s_muted = false;
            } else if (event_id <= USB_HID_SRV_EVENT_CALL_ACTIVATE || event_id == USB_HID_SRV_EVENT_CALL_END) {
                s_muted = false;
            }
#ifdef AIR_USB_AUDIO_OUT_ENABLE
            app_usb_out_mute(s_muted);
#endif
            break;
        case EVENT_GROUP_UI_SHELL_KEY:
            /* key event. */
            ret = _proc_key_event(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            if (event_id == APPS_EVENTS_INTERACTION_USB_PLUG_STATE) {
                bool plug_in = (bool)extra_data;
                APPS_LOG_MSGID_I(TAG"APPS_EVENTS_INTERACTION_USB_PLUG_STATE received, usb state=%d.", 1, plug_in);
                /* TODO */
            }
            break;
        }
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) && 0
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY: {
            if (event_id == BT_ULL_EVENT_LE_CALL_STATE) {
                static usb_hid_srv_event_t convert_tbl[] = {
                    USB_HID_SRV_EVENT_CALL_INCOMING,
                    USB_HID_SRV_EVENT_CALL_ACTIVATE,
                    USB_HID_SRV_EVENT_CALL_ACTIVATE,
                    USB_HID_SRV_EVENT_CALL_ACTIVATE,
                    USB_HID_SRV_EVENT_CALL_HOLD,
                    USB_HID_SRV_EVENT_CALL_REMOTE_HOLD,
                    USB_HID_SRV_EVENT_CALL_HOLD,
                    USB_HID_SRV_EVENT_CALL_END,
                    USB_HID_SRV_EVENT_CALL_MIC_MUTE,
                    USB_HID_SRV_EVENT_CALL_MIC_UNMUTE
                };
                bt_ull_le_srv_call_status_t *sta = (bt_ull_le_srv_call_status_t *)extra_data;
                APPS_LOG_MSGID_I(TAG"update ull2 call state=%d, remap to=%d.", 2, *sta, convert_tbl[*sta]);
                s_last_call_state = convert_tbl[*sta];
            }
        }
#endif
    }

    return ret;
}

apps_config_key_action_t app_hid_call_coexist_hfp(uint32_t event_id, apps_config_key_action_t raw_action)
{
    apps_config_key_action_t hfp_action = KEY_ACTION_INVALID;

    apps_config_state_t mmi_sta = apps_config_key_get_mmi_state();
    apps_config_state_t remap_mmi_sta = apps_config_key_get_mmi_state();

    if (event_id == 0xffff || s_last_call_state == USB_HID_SRV_EVENT_CALL_END) {
        return raw_action;
    }

    /* remap mmi state */
    if (mmi_sta >= APP_HFP_INCOMING && mmi_sta <= APP_HFP_MULTIPARTY_CALL) {
        switch (mmi_sta) {
            case APP_HFP_INCOMING:
            case APP_HFP_TWC_INCOMING:
                if (s_last_call_state != USB_HID_SRV_EVENT_CALL_END) {
                    remap_mmi_sta = APP_HFP_TWC_INCOMING;
                }
                break;
            case APP_HFP_OUTGOING:
            case APP_HFP_TWC_OUTGOING:
                if (s_last_call_state != USB_HID_SRV_EVENT_CALL_END) {
                    remap_mmi_sta = APP_HFP_TWC_OUTGOING;
                }
                break;
            case APP_STATE_HELD_ACTIVE:
            case APP_HFP_CALL_ACTIVE:
            case APP_HFP_CALL_ACTIVE_WITHOUT_SCO:
            case APP_HFP_MULTIPARTY_CALL:
                switch (s_last_call_state) {
                    case USB_HID_SRV_EVENT_CALL_END:
                        break;
                    case USB_HID_SRV_EVENT_CALL_INCOMING:
                        remap_mmi_sta = APP_HFP_TWC_INCOMING;
                        break;
#if 0
                    case APP_HID_CALL_CALL_STATUS_OUTGOING:
                        remap_mmi_sta = APP_HFP_TWC_OUTGOING;
                        break;
#endif
                    default:
                        remap_mmi_sta = APP_HFP_MULTIPARTY_CALL;
                        break;
                }
                break;
        }
    } else {
        switch (s_last_call_state) {
            case USB_HID_SRV_EVENT_CALL_END:
                break;
            case USB_HID_SRV_EVENT_CALL_INCOMING:
                remap_mmi_sta = APP_HFP_INCOMING;
                break;
#if 0
            case APP_HID_CALL_CALL_STATUS_OUTGOING:
                remap_mmi_sta = APP_HFP_TWC_OUTGOING;
                break;
#endif
            default:
                remap_mmi_sta = APP_HFP_CALL_ACTIVE;
                break;
        }
    }

    /* remap key action by remaped mmi state. */
    uint8_t key_id;
    airo_key_event_t key_event;
    app_event_key_event_decode(&key_id, &key_event, event_id);

#ifdef AIRO_KEY_EVENT_ENABLE
    static const uint8_t captouch_keys[] = APPS_CAPTOUCH_KEY_IDS;
    for (uint8_t i = 0; i < sizeof(captouch_keys); i++) {
        if (captouch_keys[i] == key_id) {
            key_id = DEVICE_KEY_POWER;
            break;
        }
    }
#endif

    apps_config_key_action_t remap_action = apps_config_key_event_remapper_map_action_in_temp_state(key_id, key_event, remap_mmi_sta);
    APPS_LOG_MSGID_I(TAG"app_hid_call_coexist_hfp mmi_sta=%d, hid_call_sta=%d, remap_mmi=%d, remap_key_action=%d", 4,
                     mmi_sta, s_last_call_state, remap_mmi_sta, remap_action);

    /* Decide hfp call and hid call action. */
    switch (remap_action) {
        /* Just one call exist, find which one. */
        case KEY_REDIAL_LAST_CALL:
        case KEY_CANCEL_OUT_GOING_CALL:
        case KEY_REJCALL:
        case KEY_ACCEPT_CALL:
        /* Common case. */
        case KEY_VOICE_UP:
        case KEY_VOICE_DN:
        case KEY_MUTE_MIC:
        /* If just one call exist, find which one. If at least two call exist, hfp call is higher prority. */
        case KEY_END_CALL:
        case KEY_ONHOLD_CALL:
            if (mmi_sta >= APP_HFP_INCOMING && mmi_sta <= APP_HFP_MULTIPARTY_CALL) {
                hfp_action = remap_action;
            } else if (s_last_call_state != USB_HID_SRV_EVENT_CALL_END) {
                s_key_action_pre_parse_result = remap_action;
            } else {
                APPS_LOG_MSGID_E(TAG"app_hid_call_coexist_hfp, unexpected status.", 0);
            }
            break;
        /* If just one call exist, find which one. If at least two call exist, hid call is higher prority. */
        case KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER:
            if (s_last_call_state == USB_HID_SRV_EVENT_CALL_INCOMING) {
                s_key_action_pre_parse_result = KEY_ACCEPT_CALL;
                if (mmi_sta == APP_HFP_INCOMING) {
                    hfp_action = KEY_REJCALL;
                } else if (mmi_sta > APP_HFP_INCOMING) {
                    hfp_action = KEY_END_CALL;
                }
            } else if (mmi_sta == APP_HFP_INCOMING) {
                hfp_action = KEY_ACCEPT_CALL;
                if (s_last_call_state == USB_HID_SRV_EVENT_CALL_INCOMING) {
                    s_key_action_pre_parse_result = KEY_REJCALL;
                } else if (s_last_call_state > USB_HID_SRV_EVENT_CALL_INCOMING) {
                    s_key_action_pre_parse_result = KEY_END_CALL;
                }
            } else {
                APPS_LOG_MSGID_E(TAG"app_hid_call_coexist_hfp, unexpected status in multipart call case.", 0);
            }
            break;
        /* hid call is higher priority. */
        case KEY_REJCALL_SECOND_PHONE:
            if (mmi_sta == APP_HFP_INCOMING) {
                hfp_action = KEY_REJCALL;
            } else if (s_last_call_state == USB_HID_SRV_EVENT_CALL_INCOMING) {
                s_key_action_pre_parse_result = KEY_REJCALL;
            } else {
                APPS_LOG_MSGID_E(TAG"app_hid_call_coexist_hfp, unexpected status in twc incoming call case.", 0);
            }
            break;
    }

    APPS_LOG_MSGID_I(TAG"app_hid_call_coexist_hfp, return hfp_action=%d, pre parse key action=%d.", 2,
                     hfp_action, s_key_action_pre_parse_result);
    return hfp_action;
}

bool app_hid_call_existing(void)
{
    return (s_last_call_state < USB_HID_SRV_EVENT_CALL_END);
}

