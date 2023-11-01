/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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
#ifdef AIR_USB_HID_ENABLE
#include "FreeRTOS.h"
#include "timers.h"
#include "syslog.h"
#include "usbaudio_drv.h"
#include "usb_hid_srv.h"

log_create_module(USB_HID_SRV_CALL, PRINT_LEVEL_INFO);

/**************************************************************************************************
* Define
**************************************************************************************************/
#define USB_HID_SRV_CALL_MSGLOG_I(msg, cnt, arg...) LOG_MSGID_I(USB_HID_SRV_CALL, msg, cnt, ##arg)

#define USB_HID_SRV_CALL_CMD_LEN                   0x02

#define USB_HID_SRV_CALL_REPORT_ID_IDX             0
#define USB_HID_SRV_CALL_STATE_IDX                 0x01

#define USB_HID_SRV_CALL_RX_FLAG_NONE              0x00
#define USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK          0x01
#define USB_HID_SRV_CALL_RX_FLAG_MUTE              0x02
#define USB_HID_SRV_CALL_RX_FLAG_RING              0x04
#define USB_HID_SRV_CALL_RX_FLAG_HOLD              0x08
#define USB_HID_SRV_CALL_RX_FLAG_MIC               0x10
#define USB_HID_SRV_CALL_RX_FLAG_LINE              0x20
#define USB_HID_SRV_CALL_RX_FLAG_RINGER            0x40

#define USB_HID_SRV_CALL_TX_FLAG_NONE              0x00
#define USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH       0x01
#define USB_HID_SRV_CALL_TX_FLAG_PHONE_MUTE        0x02
#define USB_HID_SRV_CALL_TX_FLAG_REDIAL            0x04
#define USB_HID_SRV_CALL_TX_FLAG_FLASH             0x08
#define USB_HID_SRV_CALL_TX_FLAG_LINE_BUSY_TONE    0x10
#define USB_HID_SRV_CALL_TX_FLAG_LINE              0x20
#define USB_HID_SRV_CALL_TX_FLAG_SPEED_DIAL        0x40
#define USB_HID_SRV_CALL_TX_FLAG_HEADSET_ARRAY     0x80

#define USB_HID_SRV_CALL_END_DELAY                 (2500)
#define USB_HID_SRV_CALL_MUTE_DELAY                (8)
#define USB_HID_SRV_CALL_TERMINATE_DELAY           (3)
#define USB_HID_SRV_CALL_ACCEPT_DELAY              (500)
#define USB_HID_SRV_CALL_REJECT_DELAY              (3)
#define USB_HID_SRV_CALL_HOLD_DELAY                (3)
#define USB_HID_SRV_CALL_UNHOLD_DELAY              (3)

enum {
    USB_HID_SRV_CALL_TIMER_UNHOLD_TERMINATE,
    USB_HID_SRV_CALL_TIMER_TERMINATE_DELAY,
    USB_HID_SRV_CALL_TIMER_ACCEPT_DELAY,
    USB_HID_SRV_CALL_TIMER_REJECT_DELAY,
    USB_HID_SRV_CALL_TIMER_HOLD_DELAY,
    USB_HID_SRV_CALL_TIMER_UNHOLD_DELAY,
    USB_HID_SRV_CALL_TIMER_MUTE,
    USB_HID_SRV_CALL_TIMER_CHECK_UNHOLD_OR_TERMINATE,
    USB_HID_SRV_CALL_TIMER_MAX
};

/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef bt_status_t (*usb_hid_srv_call_action_handler_t)(void *params);

/**************************************************************************************************
* Prototype
**************************************************************************************************/
static void usb_hid_srv_call_start_timer(uint32_t type, uint32_t timeout_ms);
static bool usb_hid_srv_call_stop_timer(uint32_t type);
static void usb_hid_srv_call_send_cmd(uint8_t cmd, uint8_t length);
static void usb_hid_srv_call_send_event(usb_hid_srv_event_t event);
static bt_status_t usb_hid_srv_call_accept_call(void *params);
static bt_status_t usb_hid_srv_call_reject_call(void *params);
static bt_status_t usb_hid_srv_call_hold_call(void *params);
static bt_status_t usb_hid_srv_call_unhold_call(void *params);
static bt_status_t usb_hid_srv_call_terminate_call(void *params);
static bt_status_t usb_hid_srv_call_toggle_mic_mute(void *params);

/**************************************************************************************************
* Variable
**************************************************************************************************/
extern usb_hid_srv_event_cb_t g_usb_hid_srv_event_cb;

static uint8_t g_usb_hid_srv_call_rx_event = USB_HID_SRV_CALL_RX_FLAG_NONE;
static uint8_t g_usb_hid_srv_call_tx_cmd = USB_HID_SRV_CALL_TX_FLAG_NONE;
static TimerHandle_t g_usb_hid_srv_call_timer[USB_HID_SRV_CALL_TIMER_MAX] = {0};

static const usb_hid_srv_call_action_handler_t g_usb_hid_srv_call_action_table[USB_HID_SRV_ACTION_CALL_MAX] = {
    usb_hid_srv_call_accept_call,
    usb_hid_srv_call_reject_call,
    usb_hid_srv_call_hold_call,
    usb_hid_srv_call_unhold_call,
    usb_hid_srv_call_terminate_call,
    usb_hid_srv_call_toggle_mic_mute
};

/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static uint32_t usb_hid_srv_call_get_timer_type(TimerHandle_t timer_id)
{
    uint32_t i = 0;
    for (i = 0; i < USB_HID_SRV_CALL_TIMER_MAX; i++) {
        if (g_usb_hid_srv_call_timer[i] == timer_id) {
            return i;
        }
    }
    return USB_HID_SRV_CALL_TIMER_MAX;
}

static void usb_hid_srv_call_handle_timeout(TimerHandle_t timer_id)
{
    uint32_t timer_type = usb_hid_srv_call_get_timer_type(timer_id);
    USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] timeout! type %d", 1, timer_type);

    switch (timer_type) {
        case USB_HID_SRV_CALL_TIMER_UNHOLD_TERMINATE: {
            //app_le_audio_usb_hid_terminate_call(1);
            if (g_usb_hid_srv_call_tx_cmd & USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH) {
                /* Terminate Call case I, II, III(1st time): normal case, use 05 00 00 to terminate call */
                usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_NONE, USB_HID_SRV_CALL_CMD_LEN);
            } else {
                /* Terminate Call case III(2nd time): if multidevice, use 05 01 00--500ms-->05 00 00 to terminate call */
                usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH, USB_HID_SRV_CALL_CMD_LEN);
                usb_hid_srv_call_start_timer(USB_HID_SRV_CALL_TIMER_TERMINATE_DELAY, 500);
            }
            break;
        }
        case USB_HID_SRV_CALL_TIMER_ACCEPT_DELAY: {
            if(g_usb_hid_srv_call_tx_cmd & USB_HID_SRV_CALL_TX_FLAG_FLASH){
                usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_NONE, USB_HID_SRV_CALL_CMD_LEN);
            }
            else{
                usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH, USB_HID_SRV_CALL_CMD_LEN);
            }
            break;
        }
        case USB_HID_SRV_CALL_TIMER_TERMINATE_DELAY:
        case USB_HID_SRV_CALL_TIMER_REJECT_DELAY: {
            usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_NONE, USB_HID_SRV_CALL_CMD_LEN);
            break;
        }
        case USB_HID_SRV_CALL_TIMER_HOLD_DELAY:
        case USB_HID_SRV_CALL_TIMER_UNHOLD_DELAY: {
            usb_hid_srv_call_send_cmd((g_usb_hid_srv_call_tx_cmd & USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH), USB_HID_SRV_CALL_CMD_LEN);
            break;
        }
        case USB_HID_SRV_CALL_TIMER_MUTE: {
            usb_hid_srv_call_send_cmd((g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK), USB_HID_SRV_CALL_CMD_LEN);
            break;
        }
        case USB_HID_SRV_CALL_TIMER_CHECK_UNHOLD_OR_TERMINATE: {
            /* Call end */
            usb_hid_srv_call_send_event(USB_HID_SRV_EVENT_CALL_END);
            break;
        }

    }
}

static void usb_hid_srv_call_start_timer(uint32_t type, uint32_t timeout_ms)
{
    uint8_t result = 1;
    if (type >= USB_HID_SRV_CALL_TIMER_MAX) {
        return;
    }

    if (!g_usb_hid_srv_call_timer[type]) {
        g_usb_hid_srv_call_timer[type] = xTimerCreate("g_usb_hid_srv_call_timer",
                                                 (timeout_ms * portTICK_PERIOD_MS),
                                                 pdFALSE, /* Repeat timer */
                                                 NULL,
                                                 usb_hid_srv_call_handle_timeout);
    }

    if (g_usb_hid_srv_call_timer[type]) {
        if (pdFALSE == xTimerIsTimerActive(g_usb_hid_srv_call_timer[type])) {
            xTimerStart(g_usb_hid_srv_call_timer[type], 0);
            result = 0;
        }
    }

    USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] start_timer %d for %d ms, fail:%d", 3, type, timeout_ms, result);
}

static bool usb_hid_srv_call_stop_timer(uint32_t type)
{
    if (type >= USB_HID_SRV_CALL_TIMER_MAX) {
        return false;
    }

    if (!g_usb_hid_srv_call_timer[type]) {
        return false;
    }

    if (pdTRUE != xTimerIsTimerActive(g_usb_hid_srv_call_timer[type])) {
        return false;
    }

    xTimerStop(g_usb_hid_srv_call_timer[type], 0);
    USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] stop_timer %d, done", 1, type);
    return true;
}

static void usb_hid_srv_call_send_cmd(uint8_t cmd, uint8_t length)
{
    USB_HID_Status_t ret;
    uint8_t buf[5] = {USB_HID_SRV_CALL_STATE_REPORT_ID, 0, 0, 0, 0};

    buf[1] = cmd;
    ret = USB_HID_TX_SendData(buf, length);

    USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] send_call_cmd, cmd:%x length:%x ret:%x", 3, cmd, length, ret);
    g_usb_hid_srv_call_tx_cmd = cmd;
}

static void usb_hid_srv_call_send_event(usb_hid_srv_event_t event)
{
    if((event < USB_HID_SRV_EVENT_CALL_MAX)&&(g_usb_hid_srv_event_cb != NULL)) {
        USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] send event %x", 1, event);
        g_usb_hid_srv_event_cb(event);
    }
}

static bt_status_t usb_hid_srv_call_accept_call(void *params)
{
    if (g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_RING) {
        if ((g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK)||
            (g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_HOLD)) {
            // 2nd Incoming Call
            usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_FLASH, USB_HID_SRV_CALL_CMD_LEN);
            usb_hid_srv_call_start_timer(USB_HID_SRV_CALL_TIMER_ACCEPT_DELAY, 10);
        }
        else {
            // 1st Incoming Call
            if (!(g_usb_hid_srv_call_tx_cmd & USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH)){
                usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH, USB_HID_SRV_CALL_CMD_LEN);
            }
            else{
                usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_NONE, USB_HID_SRV_CALL_CMD_LEN);
                usb_hid_srv_call_start_timer(USB_HID_SRV_CALL_TIMER_ACCEPT_DELAY, USB_HID_SRV_CALL_ACCEPT_DELAY);
            }
        }
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_FAIL;
}

static bt_status_t usb_hid_srv_call_reject_call(void *params)
{
    if (g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_RING) {
        //Reject incoming call
        usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_HEADSET_ARRAY, USB_HID_SRV_CALL_CMD_LEN);
        usb_hid_srv_call_start_timer(USB_HID_SRV_CALL_TIMER_REJECT_DELAY, USB_HID_SRV_CALL_REJECT_DELAY);
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_FAIL;
}

static bt_status_t usb_hid_srv_call_hold_call(void *params)
{
    if (!(g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_HOLD)) {
        /* 05 09, 05 01, (wait 05 09 -> 05 08), 05 00 */
        uint8_t hook_switch_state = g_usb_hid_srv_call_tx_cmd & USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH;
        usb_hid_srv_call_send_cmd(hook_switch_state | USB_HID_SRV_CALL_TX_FLAG_FLASH, USB_HID_SRV_CALL_CMD_LEN);
        usb_hid_srv_call_start_timer(USB_HID_SRV_CALL_TIMER_HOLD_DELAY, USB_HID_SRV_CALL_HOLD_DELAY);
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_FAIL;
}

static bt_status_t usb_hid_srv_call_unhold_call(void *params)
{
    if (g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_HOLD) {
        /* 05 08, 05 00, (wait 05 00 -> 05 01), 05 01 */
        uint8_t hook_switch_state = g_usb_hid_srv_call_tx_cmd & USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH;
        usb_hid_srv_call_send_cmd(hook_switch_state | USB_HID_SRV_CALL_TX_FLAG_FLASH, USB_HID_SRV_CALL_CMD_LEN);
        usb_hid_srv_call_start_timer(USB_HID_SRV_CALL_TIMER_UNHOLD_DELAY, USB_HID_SRV_CALL_UNHOLD_DELAY);
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_FAIL;
}

static bt_status_t usb_hid_srv_call_terminate_call(void *params)
{
    if (g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_HOLD) {
        USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] call held, unhold firstly", 0);
        /* unhold firstly and then terminate */
        usb_hid_srv_call_unhold_call(params);
        usb_hid_srv_call_start_timer(USB_HID_SRV_CALL_TIMER_UNHOLD_TERMINATE, USB_HID_SRV_CALL_END_DELAY);
    }
    else if (g_usb_hid_srv_call_rx_event){
        /* if dongle hook-switch status:
           0: send 05 01 00 -> 500ms -> 05 00 00
           1: send 05 00 00
         */
        if (g_usb_hid_srv_call_tx_cmd & USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH) {
            /* Terminate Call case I, II, III(1st time): normal case, use 05 00 00 to terminate call */
            usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_NONE, USB_HID_SRV_CALL_CMD_LEN);
        } else {
            /* Terminate Call case III(2nd time): if multidevice, use 05 01 00--500ms-->05 00 00 to terminate call */
            usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH, USB_HID_SRV_CALL_CMD_LEN);
            usb_hid_srv_call_start_timer(USB_HID_SRV_CALL_TIMER_TERMINATE_DELAY, 500);
        }

    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t usb_hid_srv_call_toggle_mic_mute(void *params)
{
    if (g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_HOLD) {
        //USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] call held, do not change mic mute state", 0);
        return BT_STATUS_FAIL;
    }
    usb_hid_srv_call_send_cmd((g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK) | USB_HID_SRV_CALL_TX_FLAG_PHONE_MUTE, USB_HID_SRV_CALL_CMD_LEN);
    usb_hid_srv_call_start_timer(USB_HID_SRV_CALL_TIMER_MUTE, USB_HID_SRV_CALL_MUTE_DELAY);
    return BT_STATUS_SUCCESS;
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void usb_hid_srv_call_rx_callback(uint8_t *p_data, uint32_t size)
{
    uint8_t call_event;
    uint8_t rx_event;
    usb_hid_srv_event_t event_id = USB_HID_SRV_EVENT_CALL_MAX;

    if (NULL == p_data) {
        //USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] rx_callback ERROR, invalid param", 0);
        return;
    }

    if ((USB_HID_SRV_CALL_STATE_REPORT_LENGTH != size) || (USB_HID_SRV_CALL_STATE_REPORT_ID != p_data[USB_HID_SRV_CALL_REPORT_ID_IDX])) {
        //USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] rx_callback ERROR, invalid report_id:%x size:%x", 2, p_data[USB_HID_SRV_CALL_REPORT_ID_IDX], size);
        return;
    }
    //QA or MTBF Check Log:"rx_callback, data:"
    USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] rx_callback, data:%x %x call_event:%x", 3, p_data[0], p_data[1], g_usb_hid_srv_call_rx_event);

    rx_event = p_data[USB_HID_SRV_CALL_STATE_IDX];
    call_event = rx_event;

    if (g_usb_hid_srv_call_rx_event == rx_event) {
        return;
    }

    // Step 1.Process mic mute
    if ((call_event & USB_HID_SRV_CALL_RX_FLAG_MUTE) && (!(g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_MUTE))) {
        //Mute 0->1
        /* MIC MUTE  */
        event_id = USB_HID_SRV_EVENT_CALL_MIC_MUTE;
    }
    else if ((!(call_event & USB_HID_SRV_CALL_RX_FLAG_MUTE)) && (g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_MUTE)) {
        //Mute 1->0
        /* MIC UNMUTE  */
        event_id = USB_HID_SRV_EVENT_CALL_MIC_UNMUTE;
    }

    usb_hid_srv_call_send_event(event_id);
    call_event = call_event & (~USB_HID_SRV_CALL_RX_FLAG_MUTE);
    event_id = USB_HID_SRV_EVENT_CALL_MAX;

    // Step 2.Process call state
    if (!(USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK & call_event) && (USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK & g_usb_hid_srv_call_rx_event)) {
        //USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] rx_callback off-hook 1->0", 0);
        /* Off-hook from 1 to 0, send 05 00 00 */
        usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_NONE, USB_HID_SRV_CALL_CMD_LEN);

    } else if ((USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK & call_event) && !(USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK & g_usb_hid_srv_call_rx_event)) {
        //USB_HID_SRV_CALL_MSGLOG_I("[USB][HID][CALL] rx_callback off-hook 0->1", 0);
        /* Off-hook from 0 to 1, send 05 01 00 */
        usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH, USB_HID_SRV_CALL_CMD_LEN);
   }

    if ((call_event & USB_HID_SRV_CALL_RX_FLAG_RING) &&
        (!(g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_RING))) {
        //Ring 0->1
        /* Inoming call */
        event_id = USB_HID_SRV_EVENT_CALL_INCOMING;
    }
    else if (!(call_event & USB_HID_SRV_CALL_RX_FLAG_RING) &&
                (g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_RING)) {
        //Ring 1->0
        if((call_event & USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK) && !(call_event & USB_HID_SRV_CALL_RX_FLAG_HOLD)){
            //Ring 1->0 & Off-Hook x->1 & Hold x->0
            //Call Accept
            event_id = USB_HID_SRV_EVENT_CALL_ACTIVATE;
        }
        else if(!(call_event & USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK) && (call_event & USB_HID_SRV_CALL_RX_FLAG_HOLD)){
            // 1st call is held,2nd is incoming =>Don't accept 2nd call time out
            //Ring 1->0 & Off-Hook x->0 & Hold x->1
            event_id = USB_HID_SRV_EVENT_CALL_HOLD;
        }
        else if(call_event == USB_HID_SRV_CALL_RX_FLAG_NONE){
            //Don't accept call time out
            event_id = USB_HID_SRV_EVENT_CALL_END;
        }
    }
    else if ((call_event & USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK) &&
                !(call_event & USB_HID_SRV_CALL_RX_FLAG_RING) &&
               (!(g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_OFF_HOOK))) {
        //Off-Hook 0->1
        if (usb_hid_srv_call_stop_timer(USB_HID_SRV_CALL_TIMER_CHECK_UNHOLD_OR_TERMINATE)) {
                /* Call unhold */
                event_id = USB_HID_SRV_EVENT_CALL_UNHOLD;
        }
        else if(!(call_event & USB_HID_SRV_CALL_RX_FLAG_HOLD)){
            /* Call active */
            event_id = USB_HID_SRV_EVENT_CALL_ACTIVATE;
        }
    }
    else if (!(call_event & USB_HID_SRV_CALL_RX_FLAG_HOLD) &&
                (g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_HOLD)) {
        //Hold 1->0
        if(call_event == USB_HID_SRV_CALL_RX_FLAG_NONE){//Terminate by PC (05 08,05 00)or Normal unhold call but will receive 05 01 soon(05 08,05 00,05 01)
            usb_hid_srv_call_start_timer(USB_HID_SRV_CALL_TIMER_CHECK_UNHOLD_OR_TERMINATE, 300);
        }
        else{
            /* Call local unhold */
            event_id = USB_HID_SRV_EVENT_CALL_UNHOLD;
        }
    }
    else if ((call_event & USB_HID_SRV_CALL_RX_FLAG_HOLD)&&
                (!(g_usb_hid_srv_call_rx_event & USB_HID_SRV_CALL_RX_FLAG_HOLD))) {
        //Hold 0->1
        /* Call local hold */
        event_id = USB_HID_SRV_EVENT_CALL_HOLD;
    }
    else if (call_event == USB_HID_SRV_CALL_RX_FLAG_NONE) {
        /* Call end */
        event_id = USB_HID_SRV_EVENT_CALL_END;
    }

    if((event_id == USB_HID_SRV_EVENT_CALL_END) && (g_usb_hid_srv_call_tx_cmd & USB_HID_SRV_CALL_TX_FLAG_HOOK_SWITCH)) {
        //Rx:05 04
        //Tx:05 01(Accept)
        //Rx:No Response
        //Rx:05 00(End by PC)
        //Tx:05 00(if don't send,will cause next incoming call will not be accepted)
        usb_hid_srv_call_send_cmd(USB_HID_SRV_CALL_TX_FLAG_NONE, USB_HID_SRV_CALL_CMD_LEN);
    }

    g_usb_hid_srv_call_rx_event = rx_event;
    usb_hid_srv_call_send_event(event_id);
}

bt_status_t usb_hid_srv_call_handle_action(usb_hid_srv_action_t action,void *param)
{
    if(action >= USB_HID_SRV_ACTION_CALL_MAX){
        return BT_STATUS_FAIL;
    }
    return g_usb_hid_srv_call_action_table[action](param);
}

#endif

