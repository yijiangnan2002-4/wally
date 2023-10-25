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
#include "syslog.h"
#include "usbaudio_drv.h"
#include "usb_hid_srv.h"


/**************************************************************************************************
* Define
**************************************************************************************************/
#define USB_HID_SRV_MSGLOG_I(msg, cnt, arg...) LOG_MSGID_I(USB_HID_SRV_CALL, msg, cnt, ##arg)

/**************************************************************************************************
* Structure
**************************************************************************************************/


/**************************************************************************************************
* Prototype
**************************************************************************************************/
extern void usb_hid_srv_call_rx_callback(uint8_t *p_data, uint32_t size);
extern bt_status_t usb_hid_srv_call_handle_action(usb_hid_srv_action_t index,void *param);

/**************************************************************************************************
* Variable
**************************************************************************************************/
usb_hid_srv_event_cb_t g_usb_hid_srv_event_cb = NULL;


/**************************************************************************************************
* Static Functions
**************************************************************************************************/


/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void usb_hid_srv_init(usb_hid_srv_event_cb_t callback)
{
    int32_t ret = 0;
    g_usb_hid_srv_event_cb = callback;

    ret = usb_hid_handler_rx_register(USB_HID_SRV_CALL_STATE_REPORT_ID, USB_HID_SRV_CALL_STATE_REPORT_LENGTH, usb_hid_srv_call_rx_callback);
    USB_HID_SRV_MSGLOG_I("[USB][HID][CALL] init, usb_hid_handler_rx_register:%x", 1, ret);
}

bt_status_t usb_hid_srv_send_action(usb_hid_srv_action_t action, void *param)
{
    bt_status_t ret = BT_STATUS_FAIL;
    if(action < USB_HID_SRV_ACTION_CALL_MAX){
        ret = usb_hid_srv_call_handle_action(action, param);
    }

    return ret;
}
#endif

