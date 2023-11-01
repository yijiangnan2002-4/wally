/*
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


#ifndef __BLE_HRS_H__
#define __BLE_HRS_H__
#include "bt_type.h"
#include "bt_system.h"
#include "bt_att.h"
#include "bt_gatt.h"
#include "bt_gatts.h"
#include "bt_gap_le.h"
#include "bt_platform.h"
#include "ble_hrs_feature.h"


BT_EXTERN_C_BEGIN


/**
 * @defgroup BluetoothServices_HRS_define Define
 * @{
 * This section defines the HRS event types.
 */
#define BLE_HRS_EVENT_READ_BODY_SENSOR_LOCATION                     0x00      /**< Read Body Sensor Location. */
#define BLE_HRS_EVENT_READ_HEART_RATE_MEASUREMENT_CCCD              0x01      /**< Read Client Characteristic Configuration descriptor of Heart Rate Measurement. */
#define BLE_HRS_EVENT_WRITE_HEART_RATE_MEASUREMENT_CCCD             0x02      /**< Write Client Characteristic Configuration descriptor of Heart Rate Measurement. */
#define BLE_HRS_EVENT_WRITE_HEART_RATE_CONTROL_POINT                0x03      /**< Write Opcode to Heart Rate Control Point. */
typedef uint16_t ble_hrs_event_t;   /**< The event type of HRS. */

#define BLE_HRS_ERRCODE_SUCCESS                             0x00        /**< Response if successfully operated. */
#define BLE_HRS_ERRCODE_HRCP_VALUE_NOT_SUPPORTED            0x80        /**< Response if Control Point value is not supported. */

/**
 * @}
 */

/**
 * @defgroup BluetoothServices_HRS_define Define
 * @{
 * This section defines macros about the SIG HRS Profile.
 */

#define BT_SIG_UUID16_HEART_RATE_SERVICE                0x180D    /**< Heart Rate Service. */

#define BT_SIG_UUID16_HEART_RATE_MEASUREMENT            0x2A37    /**< Heart Rate Measurement. */
#define BT_SIG_UUID16_BODY_SENSOR_LOCATION              0x2A38    /**< Body Sensor Location. */
#define BT_SIG_UUID16_HEART_RATE_CONTROL_POINT          0x2A39    /**< Heart Rate Control Point. */


#define BLE_HRS_HRCP_OPCODE_RESET_ENERGY_EXPENDED        0x01    /**< Reset Energy Expended. */

#define BLE_HRS_BODY_SENSOR_LOCATION_OTHER        0x00    /**< Other. */
#define BLE_HRS_BODY_SENSOR_LOCATION_CHEST        0x01    /**< Chest. */
#define BLE_HRS_BODY_SENSOR_LOCATION_WRIST        0x02    /**< Wrist. */
#define BLE_HRS_BODY_SENSOR_LOCATION_FINGER       0x03    /**< Finger. */
#define BLE_HRS_BODY_SENSOR_LOCATION_HAND         0x04    /**< Hand. */
#define BLE_HRS_BODY_SENSOR_LOCATION_EAR_LOBE     0x05    /**< Ear Lobe. */
#define BLE_HRS_BODY_SENSOR_LOCATION_FOOT         0x06    /**< Foot. */
#define BLE_HRS_BODY_SENSOR_LOCATION_RFU          0x07    /**< Reserved for Future Use :0x07~0xFF. */

/**
 * @}
 */


/**
 * @brief An HRS Service.
 */
extern const bt_gatts_service_t ble_hrs_service;

/**
 * @brief   This function is a user-defined callback for the application to listen to the events of HRS Service.
 * @param[in] event           is the event of type @ref ble_hrs_event_t.
 * @param[in] handle          is the connection handle.
 * @param[in/out] data        is the data of write event or the buffer of read event from a remote device.
 * @param[in] size            is the write event's data length or the read event's buffer length,specially 0 is
 *                            only used to get the Characteristic Value's Length.
 * @param[in] offset          is the offset of data for long characteristic.
 * @return    BT_GATTS_ASYNC_RESPONSE,it means the ATT response has sent in this function,so ATT bearer will not
 *                                    send ATT response again.
 *            size,for write event,it means how long the length of data has written to characteristic,specially
 *                 if returns 0,ATT bearer will send ATT response with error code @ref BT_ATT_ERRCODE_INVALID_ATTRIBUTE_VALUE_LENGTH.
 *            size,for read event,it means the length of Characteristic Value.
 */
uint32_t ble_hrs_event_handler(ble_hrs_event_t event, bt_handle_t handle, void *data, uint16_t size, uint16_t offset);

/**
 * @brief   This function is used to send an ATT response to a peer device about the ATT write request.
 * @param[in] handle            is the connection handle.
 * @param[in] bt_sig_uuid16     is Characteristic UUID,@ref BT_SIG_UUID16_HEART_RATE_CONTROL_POINT.
 * @param[in] error_code        is the error code.
 * @return    BT_STATUS_SUCCESS, the ATT response is sent successfully.
 *            BT_STATUS_FAIL, the ATT response is failed to send.
 */
bt_status_t ble_hrs_send_response(bt_handle_t handle, uint16_t bt_sig_uuid16, uint8_t error_code);

/**
 * @brief   This function is used to send a notification to a peer device about the requested data.
 * @param[in] handle           is the connection handle.
 * @param[in] bt_sig_uuid16    is Characteristic UUID,@ref BT_SIG_UUID16_HEART_RATE_CONTROL_POINT.
 * @param[in] data             is the data to be notified.
 * @param[in] length           is length of data.
 * @return    BT_STATUS_SUCCESS, the notification is sent successfully.
 *            BT_STATUS_FAIL, the notification is failed to send.
 */
bt_status_t ble_hrs_send_notification(bt_handle_t handle, uint16_t bt_sig_uuid16, uint8_t *data, uint8_t length);

BT_EXTERN_C_END
/**
 * @}
 * @}
 */

#endif /*__BLE_HRS_H__*/


