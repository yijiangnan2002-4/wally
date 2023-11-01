/*
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

#ifndef __BLE_TBS_H__
#define __BLE_TBS_H__

#include "ble_tbs_def.h"

/**
 *  @brief Define events of TBS and GTBS.
 */
#define BLE_TBS_EVENT_INCOMING_CALL     0x0001          /**< The incoming call event with #ble_tbs_event_incoming_call_t as the payload. */
#define BLE_TBS_EVENT_DIALING           0x0002          /**< The dialing Call (Outgoing call) event with #ble_tbs_event_dialing_t as the payload. */
#define BLE_TBS_EVENT_ALERTING          0x0003          /**< The alerting (Outgoing call) event with #ble_tbs_event_parameter_t as the payload. */
#define BLE_TBS_EVENT_ACTIVE            0x0004          /**< The call active event with #ble_tbs_event_parameter_t as the payload. */
#define BLE_TBS_EVENT_CALL_ENDED        0x0005          /**< The call ended event with #ble_tbs_event_parameter_t as the payload. */
#define BLE_TBS_EVENT_CALL_HELD         0x0006          /**< The call held event with #ble_tbs_event_parameter_t as the payload. */
#define BLE_TBS_EVENT_ACCEPT_CALL       0x0010          /**< The accept call event with #ble_tbs_event_parameter_t as the payload. */
#define BLE_TBS_EVENT_TERMINATE_CALL    0x0011          /**< The terminate call event with #ble_tbs_event_parameter_t as the payload. */
#define BLE_TBS_EVENT_HOLD_CALL         0x0012          /**< The hold call event with #ble_tbs_event_parameter_t as the payload. */
#define BLE_TBS_EVENT_UNHOLD_CALL       0x0013          /**< The unhold call event with #ble_tbs_event_parameter_t as the payload. */
typedef uint8_t ble_tbs_event_t;                        /**< The type of the TBS events. */

/**
 *  @brief This structure defines the parameter data type for event #BLE_TBS_EVENT_INCOMING_CALL and #BLE_TBS_EVENT_DIALING.
 */
typedef struct {
    uint8_t service_idx;    /**< Service index. */
    uint8_t call_idx;       /**< Call index. */
    uint8_t uri_len;        /**< The length of URI. */
    uint8_t *uri;           /**< URI. */
    uint8_t name_len;        /**< The length of call friendly name. */
    uint8_t *name;           /**< call friendly name. */
} ble_tbs_event_incoming_call_t, ble_tbs_event_dialing_t;

/**
 *  @brief This structure defines the parameter data type for events.
 */
typedef struct {
    uint8_t service_idx;    /**< Service index. */
    uint8_t call_idx;       /**< Call index. */
} ble_tbs_event_parameter_t;

/**
 * @brief                       This function get the GTBS service index.
 * @return                      is the service index of GTBS.
 */
uint8_t ble_tbs_get_gtbs_service_idx(void);
/**
 * @brief                       This function is ued to judge the connection handle whether is an active device.
 * @param[in] handle            is the connection handle.
 * @return                      is an active device.
 */
bool ble_tbs_is_active_group_by_handle(bt_handle_t handle);


#endif  /* __BLE_TBS_H__ */

