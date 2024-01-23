
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

#include "app_ms_teams_led.h"
#include "app_bt_state_service.h"
#include "app_ms_teams_idle_activity.h"
#include "app_ms_teams_activity.h"
#include "app_ms_teams_utils.h"
#include "stdint.h"
#define TAG "TEAMS LED"

bool app_ms_teams_set_background_led(usb_hid_srv_event_t call_event, ms_teams_notif_sub_event_t notify_event, app_ms_teams_connection_status_t teams_sta)
{
    bool ret = false;
    do {
#ifndef AIR_HEADSET_ENABLE
        app_bt_state_service_status_t *bt_sta = (app_bt_state_service_status_t *)app_bt_connection_service_get_current_status();
        if (bt_sta->connection_state == APP_BT_CONNECTION_SERVICE_BT_STATE_BT_OFF) {
            APPS_LOG_MSGID_I(TAG"APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN received, BT off, do not set the led.", 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                NULL, 1000);
            break;
        }
#endif
        /* Call state has high priority. */
#if defined(AIR_USB_HID_CALL_CTRL_ENABLE)
        if (call_event < USB_HID_SRV_EVENT_CALL_END) {
            APPS_LOG_MSGID_I(TAG"APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN received, call state=%d.", 1, call_event);
            switch (call_event) {
                case USB_HID_SRV_EVENT_CALL_ACTIVATE:
                case USB_HID_SRV_EVENT_CALL_UNHOLD:
                    apps_config_set_background_led_pattern(LED_INDEX_CALL_ACTIVE, false, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
                    break;
                case USB_HID_SRV_EVENT_CALL_INCOMING:
                    apps_config_set_background_led_pattern(LED_INDEX_INCOMING_CALL, false, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
                    break;
                case USB_HID_SRV_EVENT_CALL_HOLD:
                    apps_config_set_background_led_pattern(LED_INDEX_HOLD_CALL, false, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
                    break;
            }
            ret = true;
        }
#else
        if (call_event < USB_HID_SRV_EVENT_CALL_END) {
            APPS_LOG_MSGID_I(TAG"APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN received, call state=%d.", 1, call_event);
        }
#endif
        /* The the teams notification has second priority */
        else if (notify_event != MS_TEAMS_NOTIF_EVENT_NONE) {
            if (notify_event == MS_TEAMS_NOTIF_EVENT_UPCOMING_SCHEDULED_MEETING) {
                /* todo */
            }
            APPS_LOG_MSGID_I(TAG"APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN received, but notify sta still is %d.", 1, notify_event);
            /* Use the incoming call's led UI to indicate these event. */
            apps_config_set_background_led_pattern(LED_INDEX_TEAMS_NOTIFICATION, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            ret = true;
        } else {
            APPS_LOG_MSGID_I(TAG"APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN received, connect state=%d.", 1, teams_sta);
            if (teams_sta == APP_MS_TEAMS_CONNECTION_STA_CONNECTED) {
                apps_config_set_background_led_pattern(LED_INDEX_TEAMS_CONNECTED, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            } else if (teams_sta == APP_MS_TEAMS_CONNECTION_STA_CONNECTING) {
                apps_config_set_background_led_pattern(LED_INDEX_TEAMS_NOT_CONNECTED, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            } else {
                apps_config_set_background_led_pattern(LED_INDEX_IDLE, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            }
            /* Teams will take over the basic LED control. */
            ret = true;
        }
    } while(0);

    return ret;
}


void app_ms_teams_set_foreground_led(app_ms_teams_ui_event_t event)
{
    if (event == APP_MS_TEAMS_UI_EVENT_INVOKE_FAIL) {
#ifdef AIR_HEADSET_ENABLE
        apps_config_set_foreground_led_pattern(LED_INDEX_TEAMS_INVOKE_FAIL, 15, false);
#else
        apps_config_set_foreground_led_pattern(LED_INDEX_TEAMS_INVOKE_FAIL, 15 + 6, true);
#endif
    }
}


