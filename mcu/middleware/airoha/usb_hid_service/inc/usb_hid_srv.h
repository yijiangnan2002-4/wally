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

#ifndef __USB_HID_SRV_H__
#define __USB_HID_SRV_H__

#include "bt_type.h"

/**
 * @addtogroup USB_HID_Group USB HID Service
 * @{
 * @addtogroup USB_HID_Group_Call USB HID Call
 * @{
 * The USB HID service works on a Bluetooth Sink device or a Bluetooth Source device, it will report
 * call state events and provides many usual functions such as answer or reject incoming call.
 * This section defines the USB HID Service API to use all USB HID Call functions.
 * @{
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b SRV                        |Service, a common service that reports call state events and provides call control API. |
 *
 * @section usb_hid_srv_api_usage How to use this module
 *  - Step1: Mandatory, initialize USB HID service during system initialization and implement apps_event_usb_hid_srv_event_callback() to handle the USB HID service events.
 *   - Sample code:
 *    @code
 *           void apps_event_usb_hid_srv_event_callback(usb_hid_srv_event_t event)
 *           {
 *              if(event < USB_HID_SRV_EVENT_CALL_MAX){
 *                  ui_shell_send_event(true,
 *                                      EVENT_PRIORITY_HIGH,
 *                                      EVENT_GROUP_UI_SHELL_USB_HID_CALL,
 *                                      event,
 *                                      NULL, 0, NULL, 0);
 *              }
 *           }
 *
 *           usb_hid_srv_init(apps_event_usb_hid_srv_event_callback);
 *    @endcode
 *  - Step2: Mandatory, initialize application idle activity procedure during system initialization and implement app_le_audio_idle_activity_proc() and app_le_audio_handle_idle_usb_hid_call_event() to handle the USB HID service events about call.
 *   - Sample code:
 *    @code
 *           bool app_le_audio_handle_idle_usb_hid_call_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
 *           {
 *               bool ret = false;
 *
 *               switch (event_id) {
 *                  case USB_HID_SRV_EVENT_CALL_INCOMING: {
 *                      LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: INCOMING_CALL", 0);
 *                      app_le_audio_usb_hid_handle_incoming_call();
 *                      break;
 *                  }
 *                  case USB_HID_SRV_EVENT_CALL_ACTIVATE: {
 *                      LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: CALL_ACTIVATE", 0);
 *                      app_le_audio_usb_hid_handle_call_active();
 *                      break;
 *                  }
 *                  case USB_HID_SRV_EVENT_CALL_REMOTE_HOLD: {
 *                      LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: CALL_REMOTE_HOLD", 0);
 *                      app_le_audio_usb_hid_handle_call_remotely_hold();
 *                      break;
 *                  }
 *                  case USB_HID_SRV_EVENT_CALL_HOLD: {
 *                      LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: CALL_HOLD", 0);
 *                      app_le_audio_usb_hid_handle_call_hold();
 *                      break;
 *                  }
 *                  case USB_HID_SRV_EVENT_CALL_UNHOLD: {
 *                      LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: CALL_UNHOLD", 0);
 *                      app_le_audio_usb_hid_handle_call_unhold();
 *                      break;
 *                  }
 *                  case USB_HID_SRV_EVENT_CALL_END: {
 *                      LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: CALL_ENDED", 0);
 *                      app_le_audio_usb_hid_handle_call_end();
 *                      break;
 *                  }
 *                  case USB_HID_SRV_EVENT_CALL_MIC_MUTE: {
 *                      LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: MIC_MUTE", 0);
 *                      app_le_audio_mute_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_MIC_0);
 *                      app_le_audio_ucst_notify_mic_mute(true);
 *                      break;
 *                  }
 *                  case USB_HID_SRV_EVENT_CALL_MIC_UNMUTE: {
 *                      LE_AUDIO_MSGLOG_I("[APP][USB][HID] le_audio_evt: MIC_UNMUTE", 0);
 *                      app_le_audio_unmute_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_MIC_0);
 *                      app_le_audio_ucst_notify_mic_mute(false);
 *                      break;
 *                  }
 *                  default:
 *                      break;
 *              }
 *              return ret;
 *          }
 *
 *          bool app_le_audio_idle_activity_proc(ui_shell_activity_t *self,
 *                                               uint32_t event_group,
 *                                               uint32_t event_id,
 *                                               void *extra_data,
 *                                               size_t data_len)
 *          {
 *              bool ret = false;
 *
 *              switch (event_group) {
 *                  case EVENT_GROUP_UI_SHELL_USB_HID_CALL: {
 *                      ret = app_le_audio_handle_idle_usb_hid_call_event(self, event_id, extra_data, data_len);
 *                      break;
 *                  }
 *                  default:
 *                      break;
 *          }
 *          return ret;
 *       }
 *
 *       ui_shell_start_activity(NULL, app_le_audio_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
 *    @endcode
 *  - Step3: Optional, Call the function #usb_hid_srv_send_action() to execute a USB HID service operation, such as answer an incoming call.
 *   - Sample code:
 *    @code
 *           bt_status_t status = usb_hid_srv_send_action(USB_HID_SRV_ACTION_ACCEPT_CALL, NULL);
 *    @endcode
 */


/**
 * @defgroup USB_HID_service_define Define
 * @{
 * Define USB HID service data types and values.
 */

#define USB_HID_SRV_CALL_STATE_REPORT_ID      0x05    /**< USB HID Call state report ID. */
#define USB_HID_SRV_CALL_STATE_REPORT_LENGTH  0x02    /**< USB HID Call state report length.*/

/** @brief
 * This enum defines the USB HID service event type of call state.
 */
typedef enum {
    USB_HID_SRV_EVENT_CALL_INCOMING = 0,        /**< Call incoming event. */
    USB_HID_SRV_EVENT_CALL_ACTIVATE,            /**< Call active event. */
    USB_HID_SRV_EVENT_CALL_REMOTE_HOLD,         /**< Call remotely hold event. */
    USB_HID_SRV_EVENT_CALL_HOLD,                /**< Call locally hold event. */
    USB_HID_SRV_EVENT_CALL_UNHOLD,              /**< Call unhold event. */
    USB_HID_SRV_EVENT_CALL_END,                 /**< Call end event. */
    USB_HID_SRV_EVENT_CALL_MIC_MUTE,            /**< Call mic mute event. */
    USB_HID_SRV_EVENT_CALL_MIC_UNMUTE,          /**< Call mic unmute event. */
    USB_HID_SRV_EVENT_CALL_MAX                  /**< Max call event. */
} usb_hid_srv_event_t;

/** @brief
 * This enum defines the USB HID service action type of call control.
 */
typedef enum {
    USB_HID_SRV_ACTION_ACCEPT_CALL = 0,         /**< Accept call action. */
    USB_HID_SRV_ACTION_REJECT_CALL,             /**< Reject call action. */
    USB_HID_SRV_ACTION_HOLD_CALL,               /**< Hold call action. */
    USB_HID_SRV_ACTION_UNHOLD_CALL,             /**< Unhold call action. */
    USB_HID_SRV_ACTION_TERMINATE_CALL,          /**< Terminate call action. */
    USB_HID_SRV_ACTION_TOGGLE_MIC_MUTE,         /**< Toggle mic mute action. */
    USB_HID_SRV_ACTION_CALL_MAX                 /**< Max call action. */
} usb_hid_srv_action_t;

/**
 * @}
 */

/**
 * @defgroup USB_HID_service_struct Struct
 * @{
 * Define structures for the USB HID service.
 */

/** @brief Define usb hid callback function prototype.
 *  @param[out] event is the value defined in #usb_hid_srv_event_t.
 */
typedef void (*usb_hid_srv_event_cb_t)(usb_hid_srv_event_t event);
/**
 * @}
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief                       This function initializes the USB HID service.
 * @param[in] callback          is the callback of the USB HID service to report the event.
 * @return                      None.
 */
void usb_hid_srv_init(usb_hid_srv_event_cb_t callback);

/**
 * @brief                       This function sends an action to the USB HID service,
 *                              The USB HID service executes an operation to the connected device.
 * @param[in] action            is the request action.
 * @param[in] param             is the parameter of the action.
 * @return                      #BT_STATUS_SUCCESS, the action is successfully sent.
 *                              BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t usb_hid_srv_send_action(usb_hid_srv_action_t action, void *param);
#ifdef __cplusplus
}
#endif

/**
 * @}
 * @}
 * @}
 */
#endif  /* __USB_HID_SRV_H__ */

