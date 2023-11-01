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

#ifndef __BLE_IAC_H__
#define __BLE_IAC_H__

#include "bt_gattc_discovery.h"

/**
 *  @brief Defines the IAC events.
 */
#define BLE_IAC_EVT_DISCOVERY_COMPLETE          (0x0001)      /**< discover service complete. */
typedef uint32_t ble_iac_event_t;                             /**< The type of IAC events.*/

/**
 *  @brief Defines the IAS UUID.
 */
#define BLE_UUID16_IAS_SERVICE                  (0x1802)      /**< 16-bit UUID of Immediate Alert Service. */
#define BLE_UUID16_IAS_ALERT_LEVEL              (0x2A06)      /**< 16-bit UUID of Alert Level Characteristic. */

/**
 *  @brief Defines the Alert Level.
 */
#define BLE_IAS_NO_ALERT                        0             /**< No Alert. */
#define BLE_IAS_MILD_ALERT                      1             /**< Mild Alert. */
#define BLE_IAS_HIGH_ALERT                      2             /**< High Alert. */

/**
 *  @brief This structure defines IAC event BLE_IAC_EVT_DISCOVERY_COMPLETE detail.
 */
typedef struct
{
    bt_handle_t conn_hdl;                       /**< link connection handle */
} ble_iac_evt_discovery_complete_t;

/**
 * @brief The IAC event callback.
 */
typedef void (*ble_iac_callback_t)(ble_iac_event_t event, void *msg);

/**
 * @brief                       This function is used to initialize IAC Client.
 * @param[in] callback          IAC event callback.
 * @param[in] max_link_num      is the max number of Bluetooth link.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_iac_init(ble_iac_callback_t callback, uint8_t max_link_num);

/**
 * @brief                       This function is used to initialize IAC Client.
 * @param[in] callback          IAC event callback.
 * @param[in] alert_level       is alert level.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_iac_set_alert_level(bt_handle_t conn_hdl, uint8_t alert_level);

/**
 * @brief                       This function sets the service attribute of IAS.
 * @param[in] event             the attribute information of IAS.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_iac_set_service_attribute(bt_gattc_discovery_event_t *event);

#endif
