/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio.h"
#include "app_le_audio_utillity.h"
#include "app_le_audio_ucst.h"
#include "bt_le_audio_msglog.h"
#include "bt_le_audio_source.h"
#include "usb_hid_srv.h"
#include "app_le_audio_transmitter.h"
/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_LE_AUDIO_DONGLE_LOWEST_PRIORITY    0


/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/
#if defined(AIR_MS_TEAMS_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE)
ble_tbs_call_index_t g_curr_call_idx = BLE_TBS_INVALID_CALL_INDEX;
static ble_tbs_call_index_t g_pre_call_idx = BLE_TBS_INVALID_CALL_INDEX;
#ifdef AIR_LE_AUDIO_BA_ENABLE
static bool g_le_audio_usb_hid_switch_bcst_mode_flag = false;
#endif
#endif

/**************************************************************************************************
* Prototype
**************************************************************************************************/
#ifdef AIR_LE_AUDIO_DO_NOT_STOP_CALL_MODE_WHEN_CALL_EXIST
extern void app_le_audio_ucst_handle_call_end_event(void);
#endif

#if defined(AIR_MS_TEAMS_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE)
/* for PTS test */
extern bt_status_t ble_tbs_set_call_state_remotely_unheld(uint8_t service_idx, ble_tbs_call_index_t call_idx);
extern bt_status_t ble_tbs_set_call_state_remotely_held(uint8_t service_idx, ble_tbs_call_index_t call_idx);
extern bool g_lea_ucst_pts_remote_call_service;
#endif
extern void app_le_audio_ucst_notify_mic_mute(bool mic_mute);

/**************************************************************************************************
* Static Functions
**************************************************************************************************/
#ifdef AIR_MS_TEAMS_ENABLE
static bt_status_t app_le_audio_usb_hid_terminate_previous_call(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_le_audio_source_call_ended_t param;

    if (g_lea_ucst_pts_remote_call_service) {
        return ret;
    }

    param.service_index = ble_tbs_get_gtbs_service_idx();
    param.call_index = g_pre_call_idx;

    if(g_pre_call_idx != BLE_TBS_INVALID_CALL_INDEX){
        LE_AUDIO_MSGLOG_I("[APP][USB][HID] terminate_previous_call, call_idx:%x", 1, g_pre_call_idx);
        g_pre_call_idx = BLE_TBS_INVALID_CALL_INDEX;
        ret = bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_ENDED, &param);
    }

    return ret;
}
/**************************************************************************************************
* Public Functions
**************************************************************************************************/

bool app_le_audio_usb_hid_handle_incoming_call(void)
{
    uint8_t uri[8] = {'m', 's', 't', 'e', 'a', 'm', 's', ':'};
    ble_tbs_call_index_t call_idx;

    //app_le_audio_usb_hid_terminate_previous_call();
    call_idx = bt_le_audio_source_call_set_incoming_call(ble_tbs_get_gtbs_service_idx(), uri, 8, NULL, 0);
    LE_AUDIO_MSGLOG_I("[APP][USB][HID] handle_incoming_call, new_call_idx:%x curr_call_idx", 2, call_idx, g_curr_call_idx);

    if (BLE_TBS_INVALID_CALL_INDEX == call_idx) {
        return false;
    }

    if(g_curr_call_idx != BLE_TBS_INVALID_CALL_INDEX) {
        g_pre_call_idx = g_curr_call_idx;
    }
    else{
#ifdef AIR_LE_AUDIO_BA_ENABLE
        if (APP_LE_AUDIO_MODE_BCST == app_le_audio_get_current_mode()) {
            app_le_audio_start_unicast();
            g_le_audio_usb_hid_switch_bcst_mode_flag = true;
        } else
#endif
        {
            // 1st Incoming call
            app_le_audio_ucst_start();
        }
    }
    g_curr_call_idx = call_idx;

    return true;
}

bool app_le_audio_usb_hid_handle_outgoing_call(void)
{
    uint8_t uri[8] = {'m', 's', 't', 'e', 'a', 'm', 's', ':'};

    g_curr_call_idx = bt_le_audio_source_call_originate(ble_tbs_get_gtbs_service_idx(), uri, 8);
    LE_AUDIO_MSGLOG_I("[APP][USB][HID] handle_outgoing_call, call_idx:%x", 1, g_curr_call_idx);

    if (BLE_TBS_INVALID_CALL_INDEX == g_curr_call_idx) {
        return false;
    }

    bt_le_audio_source_call_accept_t alerting = {
        .service_index = ble_tbs_get_gtbs_service_idx(),
        .call_index = g_curr_call_idx
    };

    if (BT_STATUS_SUCCESS != bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_ALERTING, &alerting)) {
        return false;
    }
    if (BT_STATUS_SUCCESS != bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_ACTIVE, &alerting)) {
        return false;
    }
#ifdef AIR_LE_AUDIO_BA_ENABLE
    if (APP_LE_AUDIO_MODE_BCST == app_le_audio_get_current_mode()) {
        app_le_audio_start_unicast();
        g_le_audio_usb_hid_switch_bcst_mode_flag = true;
    }
#endif

    return true;
}

bool app_le_audio_usb_hid_handle_call_active(void)
{
    if (BLE_TBS_INVALID_CALL_INDEX == g_curr_call_idx) {
        app_le_audio_usb_hid_handle_outgoing_call();
    }
    else{
        bt_le_audio_source_call_active_t param;
        //ble_tbs_state_t call_state = bt_le_audio_source_call_get_state(ble_tbs_get_gtbs_service_idx(), g_curr_call_idx);
        param.service_index = ble_tbs_get_gtbs_service_idx();
        param.call_index = g_curr_call_idx;

        app_le_audio_usb_hid_terminate_previous_call();
        bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_ACTIVE, &param);
    }
    //LE_AUDIO_MSGLOG_I("[APP][USB][HID] handle_call_active, ret:%x call_status:%x", 2, ret, p_info->call_status);

    return true;
}

bool app_le_audio_usb_hid_handle_call_end(void)
{
    bt_le_audio_source_call_ended_t le_param;
    le_param.service_index = ble_tbs_get_gtbs_service_idx();
    le_param.call_index = g_curr_call_idx;

    app_le_audio_usb_hid_terminate_previous_call();
    bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_ENDED, &le_param);
    g_curr_call_idx = BLE_TBS_INVALID_CALL_INDEX;

    //LE_AUDIO_MSGLOG_I("[APP][USB][HID] handle_call_end, ret:%x call_status:%x", 2, ret, p_info->call_status);
#ifdef AIR_LE_AUDIO_BA_ENABLE
    if (g_le_audio_usb_hid_switch_bcst_mode_flag) {
        app_le_audio_start_broadcast();
        g_le_audio_usb_hid_switch_bcst_mode_flag = false;
    }
#endif

    return true;
}

bool app_le_audio_usb_hid_handle_call_hold(void)
{
    bt_le_audio_source_call_hold_t param;
    if (BLE_TBS_INVALID_CALL_INDEX == g_curr_call_idx) {
        app_le_audio_usb_hid_handle_incoming_call();
    }
    param.service_index = ble_tbs_get_gtbs_service_idx();
    param.call_index = g_curr_call_idx;

    app_le_audio_usb_hid_terminate_previous_call();
    bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_HOLD, &param);
    //LE_AUDIO_MSGLOG_I("[APP][USB][HID] handle_call_active, ret:%x call_status:%x", 2, ret, p_info->call_status);
    return true;
}

bool app_le_audio_usb_hid_handle_call_unhold(void)
{
    bt_le_audio_source_call_unhold_t param;
    param.service_index = ble_tbs_get_gtbs_service_idx();
    param.call_index = g_curr_call_idx;

    app_le_audio_usb_hid_terminate_previous_call();
    bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_UNHOLD, &param);
    //LE_AUDIO_MSGLOG_I("[APP][USB][HID] handle_call_active, ret:%x call_status:%x", 2, ret, p_info->call_status);

    return true;
}

/* for PTS test to simulate alerting from USB HID */
bool app_le_audio_usb_hid_handle_call_alert(void)
{
    bt_le_audio_source_call_alerting_t param;
    param.service_index = ble_tbs_get_gtbs_service_idx();
    param.call_index = 2;

    app_le_audio_usb_hid_terminate_previous_call();
    bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_ALERTING, &param);

    return true;
}

/* for PTS test to simulate remotely hold from USB HID */
bool app_le_audio_usb_hid_handle_call_remotely_hold(void)
{
    uint8_t service_index = ble_tbs_get_gtbs_service_idx();
    ble_tbs_call_index_t call_index = g_curr_call_idx;

    LE_AUDIO_MSGLOG_I("[APP][USB][HID] handle_call_remotely_hold, curr_call_idx:0x%x", 1, g_curr_call_idx);

    ble_tbs_set_call_state_remotely_held(service_index, call_index);

    return true;
}

/* for PTS test to simulate remotely unhold from USB HID */
bool app_le_audio_usb_hid_handle_call_remotely_unhold(void)
{
    uint8_t service_index = ble_tbs_get_gtbs_service_idx();
    ble_tbs_call_index_t call_index = g_curr_call_idx;

    LE_AUDIO_MSGLOG_I("[APP][USB][HID] handle_call_remotely_unhold, curr_call_idx:0x%x", 1, g_curr_call_idx);

    ble_tbs_set_call_state_remotely_unheld(service_index, call_index);

    return true;
}

void app_le_audio_usb_hid_handle_busy_call(bool busy)
{
#if APP_LE_AUDIO_DONGLE_LOWEST_PRIORITY
    uint8_t i;
    ble_tbs_state_t call_state = bt_le_audio_source_call_get_state(ble_tbs_get_gtbs_service_idx(), g_curr_call_idx);
    if (busy) {
        switch (call_state) {
            case BLE_TBS_STATE_INCOMING:
                /* auto reject */
                LE_AUDIO_MSGLOG_I("[APP][USB][HID] auto reject", 0);
                usb_hid_srv_send_action(USB_HID_SRV_ACTION_REJECT_CALL, NULL);
                break;
            case BLE_TBS_STATE_DIALING:
            case BLE_TBS_STATE_ALERTING:
            case BLE_TBS_STATE_ACTIVE:
                LE_AUDIO_MSGLOG_I("[APP][USB][HID] auto hold", 0);
                /* auto hold */
                usb_hid_srv_send_action(USB_HID_SRV_ACTION_HOLD_CALL, NULL);
                break;
            default:
                break;
        }
    } else {
        switch (call_state) {
            case BLE_TBS_STATE_LOCALLY_HELD:
                LE_AUDIO_MSGLOG_I("[APP][USB][HID] auto unhold", 0);
                /* auto hold */
                usb_hid_srv_send_action(USB_HID_SRV_ACTION_UNHOLD_CALL, NULL);
                break;
            default:
                break;
        }

    }

#endif
}

bool app_le_audio_usb_hid_call_existing(void)
{
    if (g_curr_call_idx != BLE_TBS_INVALID_CALL_INDEX) {
        return true;
    }
    return false;
}

#endif

bool app_le_audio_handle_idle_usb_hid_call_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* Do not return true due to other apps will listen these events. */
    bool ret = false;

    switch (event_id) {
#ifdef AIR_MS_TEAMS_ENABLE
        case USB_HID_SRV_EVENT_CALL_INCOMING: {
            LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: INCOMING_CALL", 0);
            app_le_audio_usb_hid_handle_incoming_call();

            //if (APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL <= app_le_audio_ucst_get_pause_stream_flag()) {
            //    app_le_audio_usb_hid_handle_busy_call(true);
            //}
            break;
        }
        case USB_HID_SRV_EVENT_CALL_ACTIVATE: {
            LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: CALL_ACTIVATE", 0);
            app_le_audio_usb_hid_handle_call_active();
            break;
        }
        case USB_HID_SRV_EVENT_CALL_REMOTE_HOLD: {
            LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: CALL_REMOTE_HOLD", 0);
            //app_le_audio_usb_hid_handle_call_remotely_hold();
            break;
        }
        case USB_HID_SRV_EVENT_CALL_HOLD: {
            LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: CALL_HOLD", 0);
            app_le_audio_usb_hid_handle_call_hold();
            break;
        }
        case USB_HID_SRV_EVENT_CALL_UNHOLD: {
            LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: CALL_UNHOLD", 0);
            app_le_audio_usb_hid_handle_call_unhold();
            break;
        }
        case USB_HID_SRV_EVENT_CALL_END: {
            LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: CALL_ENDED", 0);
            //Zoom Terminate Call will not auto unmute firstly(Teams will)
            //app_le_audio_unmute_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_MIC_0);
#ifdef AIR_LE_AUDIO_DO_NOT_STOP_CALL_MODE_WHEN_CALL_EXIST
            if (app_le_audio_usb_hid_handle_call_end()) {
                app_le_audio_ucst_handle_call_end_event();
            }
#else
            app_le_audio_usb_hid_handle_call_end();
#endif
            break;
        }
#endif
        case USB_HID_SRV_EVENT_CALL_MIC_MUTE: {
            LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: MIC_MUTE", 0);
#if !AIR_LE_AUDIO_AIRD_MIC_VOLUME_CONTROL_ENABLE
            app_le_audio_mute_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_MIC_0);
#endif
            app_le_audio_ucst_notify_mic_mute(true);
            break;
        }
        case USB_HID_SRV_EVENT_CALL_MIC_UNMUTE: {
            LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: MIC_UNMUTE", 0);
#if !AIR_LE_AUDIO_AIRD_MIC_VOLUME_CONTROL_ENABLE
            app_le_audio_unmute_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_MIC_0);
#endif
            app_le_audio_ucst_notify_mic_mute(false);
            break;
        }
        default:
            break;
    }

    return ret;
}
#endif

