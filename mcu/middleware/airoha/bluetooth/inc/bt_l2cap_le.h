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
/* Airoha restricted information */

#ifndef __BT_L2CAP_LE_H__
#define __BT_L2CAP_LE_H__

#include "bt_type.h"

/**
 * @addtogroup Bluetooth
 * @{
 * @addtogroup BluetoothL2CAP L2CAP
 * @{
 * This section describes the logical link control and adaptation protocol (L2CAP) APIs for Bluetooth with Low Energy (LE).
 * The L2CAP provides connection oriented data services to upper layer protocols with protocol multiplexing capability.
 * The L2CAP layer provides logical channels, named L2CAP channels, which are multiplexed over one or more logical links.
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                  |
 * |------------------------------|-------------------------------------------------------------------------|
 * |\b L2CAP                      | Logical link control and adaptation protocol. For more information, please refer to <a href="https://en.wikipedia.org/wiki/List_of_Bluetooth_protocols#Logical_link_control_and_adaptation_protocol_(L2CAP)">Wikipedia</a>. |
 * |\b PSM                        | Protocol/Service Multiplexer. It is used to connect the l2cap dynamic channel with the remote device. |
 * |\b CBFCM                      | LE Credit Based Flow Control Mode. It is used for LE L2CAP connection-oriented channels with flow control using a credit based scheme for L2CAP data. |
 * |\b ECBFCM                     | Enhanced Credit Based Flow Control Mode. It is used for L2CAP connection-oriented channels on LE and BR/EDR with flow control using a credit-based scheme for L2CAP data. |
 *
 * @section bt_l2cap_le_api_usage How to use this module
 * - The customized L2CAP LE profile must firstly define its PSM value, and register the configuration and event callback function of L2CAP LE.
 *   Then, the L2CAP LE APIs can be used to connect or disconnect the profile and transmit data. The corresponding L2CAP LE events will also be reported in the L2CAP LE event callback.
 *
 *  - Register example
 *      @code
 *          void l2cap_le_callback(bt_l2cap_le_event_type_t event, const void *parameter);
 *
 *
 *          bt_l2cap_le_profile_config_info_t profile_info = {
 *          l2cap_le_callback,
 *          0x99,
 *          BT_L2CAP_CHANNEL_MODE_ECBFCM,
 *          };
 *
 *          bt_status status = bt_l2cap_le_profile_register(&profile_info);
 *          if (status != BT_STATUS_SUCCESS) {
 *              BT_ASSERT(0 && "profile register failed");
 *          }
 *      @endcode
 *
 *  - Event callback example
 *      @code
 *          void l2cap_le_callback(bt_l2cap_le_event_type_t event, const void *parameter)
 *          {
 *              if (NULL == parameter) {
 *                    return;
 *              }
 *
 *              switch (event) {
 *                  case BT_L2CAP_LE_CONNECTED_IND: {
 *                      ...
 *                      break;
 *                  }
 *                  case BT_L2CAP_LE_CONNECTION_REQUEST_FAILED_IND: {
 *                      ...
 *                      break;
 *                  }
 *                  case BT_L2CAP_LE_DISCONNECTED_IND: {
 *                       ...
 *                      break;
 *                  }
 *                  case BT_L2CAP_LE_DATA_IN_IND: {
 *                      ...
 *                      break;
 *                 }
 *             }
 *         }
 *      @endcode
 **/

/**
 * @defgroup Bluetoothbt_l2cap_le_define Define
 * @{
 * Define L2CAP LE data types and values.
 */


/**
 *  @brief Defines the L2CAP LE channel mode.
 */
#define BT_L2CAP_CHANNEL_MODE_CBFCM         0x05   /**< Credit base flow control mode.*/
#define BT_L2CAP_CHANNEL_MODE_ECBFCM        0x06   /**< Enhanced credit base flow control mode.*/
typedef uint8_t bt_l2cap_le_channel_mode;   /**< The type of the L2CAP LE channel mode.*/


/**
 *  @brief Defines the L2CAP LE events.
 */
#define BT_L2CAP_LE_CONNECTED_IND                       0x00   /**< This event indicates the connection-oriented channel connected #bt_l2cap_le_connected_ind_t. */
#define BT_L2CAP_LE_CONNECTION_REQUEST_FAILED_IND       0x01   /**< This event indicates the connection-oriented channel connection request failed #bt_l2cap_le_connected_ind_t. */
#define BT_L2CAP_LE_DISCONNECTED_IND                    0x02   /**< This event indicates the connection-oriented channel disconnected #bt_l2cap_le_disconnected_ind_t. */
#define BT_L2CAP_LE_DATA_IND                            0x03   /**< This event indicates the connection-oriented channel data in #bt_l2cap_le_data_ind_t. */
typedef uint8_t bt_l2cap_le_event_type_t;    /**< The type of L2CAP LE events.*/


/**
 *  @brief Defines the reason why the connection request failed.
 */
#define BT_L2CAP_LE_CONNECTION_RESULT_SUCCESS                                  0x00    /**< SUCCESS.*/
#define BT_L2CAP_LE_CONNECTION_RESULT_INVALID_RESPONSE_FROM_REMOTE             0x01    /**< Failed due to an invalid response from remote.*/
#define BT_L2CAP_LE_CONNECTION_RESULT_REMOTE_PSM_NOT_SUPPORT                   0x02    /**< Refused by remote; PSM is not support.*/
#define BT_L2CAP_LE_CONNECTION_RESULT_REMOTE_INSUFFICIENT_AUTHENTICAIOTN       0x05    /**< Refused by remote; Insufficient authentication.*/
#define BT_L2CAP_LE_CONNECTION_RESULT_REMOTE_INSUFFICIENT_AUTHORIZATION        0x06    /**< Refused by remote; Insufficient authorization.*/
#define BT_L2CAP_LE_CONNECTION_RESULT_REMOTE_INSUFFICIENT_ENC_KEY_SIZE         0x07    /**< Refused by remote; Insufficient encryption key size.*/
#define BT_L2CAP_LE_CONNECTION_RESULT_REMOTE_INSUFFICIENT_ENC                  0x08    /**< Refused by remote; Insufficient encryption.*/
#define BT_L2CAP_LE_CONNECTION_RESULT_REMOTE_UNACCEPTABLE_PARAMETERS           0x0B    /**< Refused by remote; Unacceptable parameters.*/
typedef uint8_t bt_l2cap_le_connection_result_t;    /**< The type of reason why the L2CAP LE connection request failed.*/


/**
 *  @brief Defines the disconnected reason.
 */
#define BT_L2CAP_LE_DISCONNECTED_REASON_LOCAL_REQUEST                    0x00    /**< Local request.*/
#define BT_L2CAP_LE_DISCONNECTED_REASON_DUPLICATED_CID                   0x01    /**< Duplicated CID.*/
#define BT_L2CAP_LE_DISCONNECTED_REASON_CREDIT_OVERFLOW                  0x02    /**< Credit overflow.*/
#define BT_L2CAP_LE_DISCONNECTED_REASON_CREDIT_UNDERFLOW                 0x03    /**< Credit underflow.*/
#define BT_L2CAP_LE_DISCONNECTED_REASON_INVALID_MTU_CONFIGURATION        0x04    /**< Invalid MTU configuration.*/
#define BT_L2CAP_LE_DISCONNECTED_REASON_REMOTE_REQUEST                   0x05    /**< Remote request.*/
typedef uint8_t bt_l2cap_le_disconnect_reason_t;     /**< The type of L2CAP LE disconnected reason.*/

/**
 * @}
 */


/**
 * @defgroup Bluetoothbt_l2cap_le_struct Struct
 * @{
 * This section defines data structures for the L2CAP LE.
 */

/**
 *  @brief Defines the L2CAP LE callback function.
 */
typedef void (* bt_l2cap_le_callback_t)(bt_l2cap_le_event_type_t event, const void *parameter);


/**
 *  @brief This structure defines the #bt_l2cap_le_connect parameter type.
 */
typedef struct {
    bt_addr_t *address;                                     /**< Remote device address. */
    uint16_t psm;                                           /**< PSM valume of the current channel. */
    uint8_t channel_num;                                    /**< Connection channel number. */
} bt_l2cap_le_connect_parameters_t;

/**
 *  @brief This structure defines the parameter data type for event #BT_L2CAP_LE_CONNECTED_IND / #BT_L2CAP_LE_CONNECTION_REQUEST_FAILED_IND.
 */
typedef struct {
    uint32_t                            handle;                 /**< Connection handle. */
    bt_l2cap_le_connection_result_t     result;                 /**< Connection request failed reason. */
    uint16_t                            psm;                    /**< PSM value of the current connection. */
    bt_addr_t                           *address;               /**< Remote device address. */
} bt_l2cap_le_connected_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BT_L2CAP_LE_DISCONNECTED_IND.
 */
typedef struct {
    uint32_t                            handle;                /**< Connection handle. */
    bt_l2cap_le_disconnect_reason_t     reason;                /**< Disconnected reason. */
} bt_l2cap_le_disconnected_ind_t;


/**
 *  @brief This structure defines the parameter data type for event #BT_L2CAP_LE_DATA_IND.
 */
typedef struct {
    uint32_t                            handle;             /**< Connection handle. */
    uint16_t                            length;             /**< Data length. */
    uint8_t                             *data;              /**< Data. */
} bt_l2cap_le_data_ind_t;


/**
 * @brief This structure defines the #bt_l2cap_le_get_customer_profile_config parameter type.
 */
typedef struct {
    bt_l2cap_le_callback_t callback;                   /**< Callback to invoke whenever an event occurs. */
    uint16_t psm;                                      /**< PSM value of the current connection. */
    bt_l2cap_le_channel_mode mode;                     /**< Mode support for this PSM, such as: BT_L2CAP_CHANNEL_MODE_CBFCM. */
} bt_l2cap_le_profile_config_info_t;

/**
 * @}
 */


/**
 * @brief                                         This function is used to register a profile.
 * @param[in] instance                            is the settings to initialize.
 * @return                                        #BT_STATUS_SUCCESS, the operation completed successfully, otherwise it failed.
 */
bt_status_t bt_l2cap_le_register_profile(const bt_l2cap_le_profile_config_info_t *instance);


/**
 * @brief                                         This function is used to connect L2CAP LE channel.
 * @param[out] handle                             is the handle of the channel.
 * @param[in] parameters                          is the parameters of the channel.
 * @return                                        #BT_STATUS_SUCCESS, the operation completed successfully, otherwise it failed.
 */
bt_status_t bt_l2cap_le_connect(uint32_t *handle,const bt_l2cap_le_connect_parameters_t *parameters);


/**
 * @brief                                         This function is used to disconnect L2CAP LE channel.
 * @param[out] handle                             is the handle of the current channel.
 * @return                                        #BT_STATUS_SUCCESS, the operation completed successfully, otherwise it failed.
 */
bt_status_t bt_l2cap_le_disconnect(uint32_t handle);


/**
 * @brief                                         This function is used to send L2CAP LE data.
 * @param[in] handle                              is the handle of the current channel.
 * @param[in] data                                is the data to send.
 * @param[in] length                              is the data's length.
 * @return                                        #BT_STATUS_SUCCESS, the operation completed successfully, otherwise it failed.
 */
bt_status_t bt_l2cap_le_send_data(uint32_t handle,const void *data, uint32_t length);


/**
 * @brief     This is a user-defined callback and called by the L2CAP module. It should be implemented by the application to provide the customer with a config and profile number.
 * @param[in] config_table   is a pointer to the config table. If there is no customized profile, the config_table should be NULL and return 0.
 * @return    the number of profiles.
 */
uint8_t bt_l2cap_le_get_customer_profile_config(const bt_l2cap_le_profile_config_info_t ***config_table);

/**
 * @}
 * @}
 */
#endif/*__BT_L2CAP_LE_H__*/
