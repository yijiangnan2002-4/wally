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
/* Airoha restricted information */

#ifndef __APP_MS_TEAMS_LED_H__
#define __APP_MS_TEAMS_LED_H__

#include <stdint.h>
#include <stdbool.h>
#include "usb_hid_srv.h"
#include "ms_teams.h"

/**
 * Notice!!!
 *
 * For the LED customization, the SDK has the default implementation and we have done the testing 
 * and can guarantee that they can pass the certification test. So we recommend not modifying the 
 * default implementation if there is no special reason.
 */


/**
 * Define the Teams connection status, it's the PC teams client and dongle/headset connections status.
 * Notice that, after the dongle/headset system boot up, we will try to start the connection with the PC
 * Teams, it's the CONNECTING status. But this status has 10s timeout. If teams not connected in this
 * 10s, the status will switch into DISCONNECTED.
 */
typedef enum {
    APP_MS_TEAMS_CONNECTION_STA_DISCONNECTED,
    APP_MS_TEAMS_CONNECTION_STA_CONNECTING,
    APP_MS_TEAMS_CONNECTION_STA_CONNECTED,
} app_ms_teams_connection_status_t;

/**
* @brief      The Teams spec contains the LED UI. This UI contains the call status, teams notification status and
*             teams connection status. The UI priority is that call status > notification status > connection status.
* @param[in]  call_event is the last call event the device received.
* @param[in]  notify_event is the last notification event the device received.
* @param[in]  teams_sta is the current connection status of teams.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_background_led(usb_hid_srv_event_t call_event, ms_teams_notif_sub_event_t notify_event, app_ms_teams_connection_status_t teams_sta);


typedef enum {
    APP_MS_TEAMS_UI_EVENT_INVOKE_FAIL, /* Teams button pressed but teams not connected, flash LED 3 times. */
} app_ms_teams_ui_event_t;

/**
* @brief      This function use to set the Teams foreground LED UI, this UI only show once when some event happens.
* @param[in]  event reference the app_ms_teams_ui_event_t.
* @return     Success or Failed, true means Success.
*/
void app_ms_teams_set_foreground_led(app_ms_teams_ui_event_t event);

#endif /*__APP_MS_TEAMS_LED_H__*/

