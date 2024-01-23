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

#ifndef __BT_GATT_SERVICE_CLIENT_H__
#define __BT_GATT_SERVICE_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif
/**
  * @addtogroup Bluetooth_Services_Group Bluetooth Services
  * @{
  * @addtogroup BluetoothServices BT GATT service client
  * @{
  * This section provides API to read/write the GAP/GATT service.
  *
  * @section bt_gatt_srv_client_api_usage how to use this module
  * - Step 1. Initialize the BT GATT service client.
  *  - Sample Code:
  *     @code
  *                bt_gatt_srv_client_init();
  *     @endcode
  * - Step 2. Use BT GATT service client APIs to send action.
  *  - Sample code:
  *     @code
  *           static bt_status_t bt_gatt_srv_client_event_callback(bt_gatt_srv_client_event_t event, bt_status_t status, void *parameter)
  *           {
  *                 switch (event) {
  *                     //This event indicates the read value is complete with #bt_gatt_srv_client_read_value_complete_t as the payload
  *                     case BT_GATT_SRV_CLIENT_EVENT_READ_VALUE_COMPLTETE: {
  *                     }
  *                     break;
  *                     default:
  *                         break;
  *                 }
  *                 return BT_STATUS_SUCCESS;
  *           }
  *
  *           bt_gatt_srv_client_action_read_value_t action_read_value {
  *                 .type = BT_GATT_SRV_CLIENT_DEVICE_NAME,
  *                 .connection_handle = connection_handle,
  *                 .callback = bt_gatt_srv_client_event_callback
  *           };
  *           bt_gatt_srv_client_send_action(BT_GATT_SRV_CLIENT_ACTION_READ_VALUE, &action_read_value);
  *     @endcode
  */
#include "bt_type.h"

/**
 * @defgroup BluetoothServices_GATT_SRV_CLIENT_define Define
 * @{
 * Define Bluetooth BT GATT service client data types and values.
 */

/**
 * @brief Define the GATT service characteristic type.
 */
#define BT_GATT_SRV_CLIENT_DEVICE_NAME                                 0x00         /**< The device name characteristic. */
#define BT_GATT_SRV_CLIENT_APPEARANCE                                  0x01         /**< The appearance characteristic. */
#define BT_GATT_SRV_CLIENT_PPCP                                        0x02         /**< The peripheral preferred connection parameters characteristic. */
#define BT_GATT_SRV_CLIENT_CENTRAL_ADDRESS_RESOLUTION                  0x03         /**< The central address resolution characteristic. */
#define BT_GATT_SRV_CLIENT_RPA_ONLY                                    0x04         /**< The device RPA only characteristic. */
#define BT_GATT_SRV_CLIENT_SERVICE_CHANGE                              0x05         /**< The service change characteristic. */
#define BT_GATT_SRV_CLIENT_SUPPORTED_FEATURE                           0x06         /**< The client supported feature characteristic. */
#define BT_GATT_SRV_CLIENT_DATABASE_HASH                               0x07         /**< The database hash characteristic. */
#define BT_GATT_SRV_CLIENT_SERVER_SUPPORTED_FEATURE                    0x08         /**< The server supported feature characteristic. */
#define BT_GATT_SRV_CLIENT_MAX_NUMBER                                  0x09         /**< The characteristic max number. */
typedef uint8_t bt_gatt_srv_client_t;                                               /**< The characteristic type. */

/**
 * @brief Define the GATT service client action.
 */
#define BT_GATT_SRV_CLIENT_ACTION_READ_VALUE                           0x00         /**< This action sends a request to read the value with #bt_gatt_srv_client_action_read_value_t as the payload. */
#define BT_GATT_SRV_CLIENT_ACTION_WRITE_CCCD                           0x01         /**< This action sends a request to write the CCCD with #bt_gatt_srv_client_action_write_cccd_t as the payload. */
typedef uint8_t bt_gatt_srv_client_action_t;                                        /**< The action type. */

/**
 * @brief Define the GATT service client event.
 */
#define BT_GATT_SRV_CLIENT_EVENT_READ_VALUE_COMPLETE                   0x00         /**< This event indicates the read value is complete with #bt_gatt_srv_client_read_value_complete_t as the payload. */
#define BT_GATT_SRV_CLIENT_EVENT_WRITE_CCCD_COMPLETE                   0x01         /**< This event indicates the write CCCD is complete with payload as NULL. */
typedef uint8_t bt_gatt_srv_client_event_t;                                         /**< The event type. */

/**
 * @deprecated Use #BT_GATT_SRV_CLIENT_EVENT_READ_VALUE_COMPLETE instead.
 */
#define BT_GATT_SRV_CLIENT_EVENT_READ_VALUE_COMPLTETE              (BT_GATT_SRV_CLIENT_EVENT_READ_VALUE_COMPLETE)    /**< This event will be phased out and removed in the next SDK major version. Do not use. */

/**
 * @deprecated Use #BT_GATT_SRV_CLIENT_EVENT_WRITE_CCCD_COMPLETE instead.
 */
#define BT_GATT_SRV_CLIENT_EVENT_WRITE_CCCD_COMPLTETE              (BT_GATT_SRV_CLIENT_EVENT_WRITE_CCCD_COMPLETE)    /**< This event will be phased out and removed in the next SDK major version. Do not use. */


/**
 * @brief Define the enable type.
 */
#define BT_GATT_SRV_CLIENT_DISABLE                                     0x00         /**< The disable type. */
#define BT_GATT_SRV_CLIENT_ENABLE                                      0x01         /**< The enable type. */
typedef uint8_t bt_gatt_srv_client_enable_t;                                        /**< The enable type. */

/**
 * @brief Define the event callback type.
 */
typedef bt_status_t (*bt_gatt_srv_client_event_callback)(bt_gatt_srv_client_event_t event, bt_status_t status, void *parameter);

/**
 * @}
 */

/**
 * @defgroup BluetoothServices_BT_GATT_SRV_CLIENT_struct Struct
 * @{
 * Define structures for the BT GATT service client.
 */

/**
 *  @brief This structure defines the parameter for read value action.
 */
typedef struct {
    bt_gatt_srv_client_t                type;                       /**< The characteristic type. */
    uint32_t                            connection_handle;          /**< The connection handle. */
    bt_gatt_srv_client_event_callback   callback;                   /**< The event callback. */
} bt_gatt_srv_client_action_read_value_t;

/**
 *  @brief This structure defines the parameter for write CCCD action.
 */
typedef struct {
    bt_gatt_srv_client_t                type;                       /**< The characteristic type. */
    uint32_t                            connection_handle;          /**< The connection handle. */
    bt_gatt_srv_client_enable_t         enable;                     /**< The write CCCD is either enabled or disabled */
    bt_gatt_srv_client_event_callback   callback;                   /**< The event callback. */
} bt_gatt_srv_client_action_write_cccd_t;

/**
 *  @brief This structure defines the parameter for read value complete event.
 */
typedef struct {
    bt_gatt_srv_client_t  type;                                     /**< The characteristic type. */
    uint32_t              connection_handle;                        /**< The connection handle. */
    uint32_t              length;                                   /**< The data length. */
    uint8_t               *data;                                    /**< A pointer to the data read from remote side, the length of this data does not exceed the GATT MTU. */
} bt_gatt_srv_client_read_value_complete_t;

/**
 *  @brief This structure defines the parameter for write CCCD complete event.
 */
typedef struct {
    bt_gatt_srv_client_t  type;                                     /**< The characteristic type. */
    uint32_t              connection_handle;                        /**< The connection handle. */
} bt_gatt_srv_client_write_cccd_complete_t;

/**
 *  @brief This structure defines the parameter for action error event.
 */
typedef struct {
    bt_gatt_srv_client_t  type;                                     /**< The characteristic type. */
    uint32_t              connection_handle;                        /**< The connection handle. */
} bt_gatt_srv_client_action_error_t;


/**
 * @}
 */

/**
 * @brief                       This function initializes the GATT service client.
 * @return                      #BT_STATUS_SUCCESS, the function initialization is successful.
 *                              Otherwise, the function initialization failed.
 */
bt_status_t bt_gatt_srv_client_init(void);

/**
 * @brief                       This function sends an action to the GATT service client.
 * @param[in] action            is the GATT service client action.
 * @param[in] parameter         is the parameter of the action.
 * @param[in] length            is the length of the parameter.
 * @return                      #BT_STATUS_SUCCESS, the action is sent successfully.
 *                              Otherwise, the operation failed with the specified status.
 */
bt_status_t bt_gatt_srv_client_send_action(bt_gatt_srv_client_action_t action, void *parameter, uint32_t length);

#ifdef __cplusplus
}
#endif

/**
 * @}
 * @}
 */

#endif
