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

 
#ifndef __BT_ULL_LE_CALL_SERVICE_H__
#define __BT_ULL_LE_CALL_SERVICE_H__

#include "bt_type.h"
#include "bt_system.h"
#include "bt_platform.h"
#include "bt_ull_service.h"
#include "bt_ull_le.h"

BT_EXTERN_C_BEGIN

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE

/*server call event*/
typedef uint8_t bt_ull_le_srv_call_event_t;
#define BT_ULL_LE_SRV_CALL_EVENT_INCOMING             (0x00)
#define BT_ULL_LE_SRV_CALL_EVENT_ACTIVE               (0x01)
#define BT_ULL_LE_SRV_CALL_EVENT_REMOTE_HOLD          (0x02)
#define BT_ULL_LE_SRV_CALL_EVENT_HOLD                 (0x03)
#define BT_ULL_LE_SRV_CALL_EVENT_UNHOLD               (0x04)
#define BT_ULL_LE_SRV_CALL_EVENT_END                  (0x05)
#define BT_ULL_LE_SRV_CALL_EVENT_REMOTE_MIC_MUTE      (0x06)
#define BT_ULL_LE_SRV_CALL_EVENT_REMOTE_MIC_UNMUTE    (0x07)
#define BT_ULL_LE_SRV_CALL_EVENT_MAX                  (0xFF)

/*call status*/
typedef uint8_t bt_ull_le_srv_call_status_t;
#define BT_ULL_LE_SRV_CALL_STATE_INCOMING                         (0x00)    /**<  A remote party is calling (incoming call). */
#define BT_ULL_LE_SRV_CALL_STATE_DIALING                          (0x01)    /**<  The process to call the remote party has started on the server but the remote party is not being alerted (outgoing call). */
#define BT_ULL_LE_SRV_CALL_STATE_ALERTING                         (0x02)    /**<  A remote party is being alerted (outgoing call). */
#define BT_ULL_LE_SRV_CALL_STATE_ACTIVE                           (0x03)    /**<  The call is in an active conversation. */
#define BT_ULL_LE_SRV_CALL_STATE_HELD                             (0x04)    /**<  The call is connected but held locally. */
#define BT_ULL_LE_SRV_CALL_STATE_REMOTELY_HELD                    (0x05)    /**<  The call is connected but held remotely. */
#define BT_ULL_LE_SRV_CALL_STATE_LOCALLY_AND_REMOTELY_HELD        (0x06)    /**<  The call is connected but held both locally and remotely. */
#define BT_ULL_LE_SRV_CALL_STATE_STATE_IDLE                       (0x07)    /**<  Call service idle. */
#define BT_ULL_LE_SRV_CALL_STATE_STATE_MIC_MUTE                   (0x08)    /**<  Call service idle. */
#define BT_ULL_LE_SRV_CALL_STATE_STATE_MIC_UNMUTE                 (0x09)    /**<  Call service idle. */

#define BT_ULL_LE_SRV_CALL_STATE_STATE_NOTIFY_ACTIVE_DEVICE       (0xFE)    /**<  Notify call state to active device,not a realy call state. */
#define BT_ULL_LE_SRV_CALL_STATE_STATE_INVALID                    (0xFF)    /**<  Invalid call state. */

/*client call action*/
typedef uint8_t bt_ull_le_srv_call_action_t;
#define BT_ULL_LE_SRV_CALL_ACTION_NONE                (0x00)
#define BT_ULL_LE_SRV_CALL_ACTION_ANSWER              (0x01)
#define BT_ULL_LE_SRV_CALL_ACTION_REJECT              (0x02)
#define BT_ULL_LE_SRV_CALL_ACTION_TERMINATE           (0x03)
#define BT_ULL_LE_SRV_CALL_ACTION_MUTE                (0x04)
#define BT_ULL_LE_SRV_CALL_ACTION_UNMUTE              (0x05)

/*call flags*/
typedef uint8_t bt_ull_le_srv_call_flag_t;
#define BT_ULL_LE_SRV_CALL_FLAG_INCOMING_CALL          (0x01)
#define BT_ULL_LE_SRV_CALL_FLAG_OUTGOING_CALL          (0x02)
/**
 * @brief Defines for call index.
 */
#define BT_ULL_LE_SRV_DEFAULT_MAX_CALL_COUNT       3
#define BT_ULL_LE_SRV_INVALID_CALL_INDEX           0          

typedef uint8_t bt_ull_le_srv_call_index_t;                  /**<  The type of call index.*/

/**
 * @brief   This function is used to control ULL device and only for dongle.
 * @param[in] call_event      is the USB HID Call Event.
 * @param[in] extra_data       is the payload of the request action.
 * @param[in] data_len   is the payload length.
 * @return                BT_STATUS_SUCCESS, the operation completed successfully.
 *                        BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_call_srv_send_event(bt_ull_le_srv_call_event_t call_event, void *extra_data, size_t data_len);

/**
 * @brief   This function is used to send call action(accept, reject, terminate, mute/unmute etc) and only for headset
 * @param[in] call_action  is the action that for a call such as accept, reject, terminate, mute/unmute.
 * @param[in] extra_data       is the payload of the request action.
 * @param[in] data_len   is the payload length.
 * @return                BT_STATUS_SUCCESS, the operation completed successfully.
 *                        BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_call_srv_send_action(bt_ull_le_srv_call_action_t  call_action);

/**
 * @brief   This function is used to init ull call srv
 * @param[in] void
 * @return                BT_STATUS_SUCCESS, the operation completed successfully.
 *                        BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_call_srv_init(void);

bt_status_t bt_ull_le_call_srv_client_handle_call_action(bt_ull_le_srv_call_action_t call_action, void *data);

bt_status_t bt_ull_le_call_srv_client_handle_call_state(bt_ull_le_srv_call_status_t call_state, void *data);

#endif
#endif

