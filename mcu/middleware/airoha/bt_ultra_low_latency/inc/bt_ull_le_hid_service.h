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
#ifndef __BT_ULL_LE_HID_SRV_SERVICE_H__
#define __BT_ULL_LE_HID_SRV_SERVICE_H__

/**
 * @addtogroup Bluetooth_Services_Group Bluetooth Services
 * @{
 * @addtogroup BluetoothServices_ULL Ultra Low Latency LE with HID
 * @{
 * This section describes the Ultra Low Latency (ULL) LE with HID APIs and events.
 * Clients (Ex. Headset/Earbuds/Keyboard/Mouse) with a low latency wireless audio latency and high report rate at the same time when they are connected with a well-matched server (Ex. Dongle).
 * This profile defines two roles: ULL_Client (Ex. Headset/Earbuds/Keyboard/Mouse) and ULL_Server (Ex. Dongle).
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                  |
 * |------------------------------|-------------------------------------------------------------------------|
 * |\b ULL                        | Ultra Low Latency. |
 * |\b ACL                        | Asynchronous Connectionless Link. |
 * |\b HID                         | Human Interface Device. |
 *
 * @section bt_ullle_hid_api_usage How to use this module
 *  - Step1: Initialize Bluetooth Ultra Low Latency LE with HID service during system initialization (Mandatory).
 *   - Sample code in client (Ex. Headset/Earbuds):
 *    @code
 *           bt_ull_le_hid_srv_init(BT_ULL_ROLE_CLIENT, bt_ull_le_hid_event_callback);
 *    @endcode
 *   - Sample code in server (Ex. Dongle):
 *    @code
 *           bt_ull_le_hid_srv_init(BT_ULL_ROLE_SERVER, bt_ull_le_hid_event_callback);
 *    @endcode
 *  - Step2: Mandatory, implement bt_ull_le_hid_event_callback() to handle the ULL LE HID events, such as connection event, user data received, etc.
 *   - Sample code:
 *    @code
 *       void bt_ull_le_hid_event_callback(bt_ull_event_t event, void *param, uint32_t param_len)
 *       {
 *           switch (event)
 *           {
 *              case BT_ULL_EVENT_LE_HID_BONDING_COMPLETE_IND:
 *                  printf("ULL LE HID service has bonded");
 *                  break;
 *              case BT_ULL_EVENT_LE_HID_CONNECTED_IND:
 *                  printf("ULL LE HID asynchronous connectionless link is connected");
 *                  break;
 *              case BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND:
 *                  printf("ULL LE HID service is connected");
 *                  break;
 *              case BT_ULL_EVENT_LE_HID_DISCONNECTED_IND:
 *                  printf("ULL LE HID asynchronous connectionless link and service is disconnected");
 *                  break;
 *              default:
 *                  break;
 *            }
 *        }
 *    @endcode
 *
 *  - Step4: Call the function #bt_ull_le_hid_srv_action() to control the device (Optional).
 *   - Sample code:
 *    @code
 *          bt_ull_volume_t volume_param;
 *          volume_param.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
 *          volume_param.streaming.port = 0;
 *          volume_param.action = BT_ULL_VOLUME_ACTION_SET_UP;
 *          volume_param.channel = BT_ULL_AUDIO_CHANNEL_DUAL;
 *          volume_param.volume = 1;
 *          bt_ull_le_srv_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &volume_param, sizeof(volume_param));
 *    @endcode
 */

#include "bt_type.h"
#include "bt_ull_le_service.h"

/**
 * @defgroup BluetoothServices_ULL_LE_HID_define Define.
 * @{
 * Define Bluetooth ULL LE HID data types and values.
 */

/**
 *  @brief Define the ULL LE HID service callback event.
 */
#define BT_ULL_EVENT_LE_HID_BONDING_COMPLETE_IND                  0x20   /* Notify user that ULL LE HID bond is compeleted, for parameter please refer to structure #bt_ull_le_hid_srv_bonding_complete_ind_t.*/
#define BT_ULL_EVENT_LE_HID_CONNECTED_IND                         0x21   /* Notify user that ULL LE HID asynchronous connectionless link is connected,  for parameter please refer to structure #bt_ull_le_hid_srv_connected_ind_t.*/
#define BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND                 0x22   /* Notify user that ULL LE HID service is connected, for parameter please refer to structure #bt_ull_le_hid_srv_service_connected_ind_t.*/
#define BT_ULL_EVENT_LE_HID_DISCONNECTED_IND                      0x23   /* Notify user that ULL LE HID service and asynchronous connectionless link is disconnected, for parameter please refer to structure #bt_ull_le_hid_srv_disconnected_ind_t.*/
#define BT_ULL_EVENT_LE_HID_SWITCH_LINK_MODE_IND                  0x24   /* Notify user that ULL LE HID service received the indication to switch link mode, for parameter please refer to structure #bt_ull_le_hid_srv_switch_link_mode_ind_t.*/ 
#define BT_ULL_EVENT_LE_HID_INPUT_REPORT_IND                      0x25   /* Notify user that ULL LE HID service received input report data from keyboard or mouse, for parameter please refer to structure #bt_ull_le_hid_srv_input_report_data_t.*/
#define BT_ULL_EVENT_LE_HID_RACE_DATA_IND                         0x26   /* Notify user that ULL LE HID service received race data, for parameter please refer to structure #bt_ull_le_hid_srv_race_data_t.*/

/**
 *  @brief Define the user action.
 */
#define BT_ULL_ACTION_LE_HID_BOND                                 0x20   /*Bond ULL LE HID service,  for parameter please refer to structure #bt_ull_le_hid_srv_bond_t.*/
#define BT_ULL_ACTION_LE_HID_CONNECT                              0x21   /*Connect ULL LE HID asynchronous connectionless link (server),  for parameter please refer to structure #bt_ull_le_hid_srv_connect_t.*/
#define BT_ULL_ACTION_LE_HID_CREATE_SYNC                          0x22   /*Create sync ULL LE HID asynchronous connectionless link (client), for parameter please refer to structure #bt_ull_le_hid_srv_create_sync_t.*/
#define BT_ULL_ACTION_LE_HID_DISCONNECT                           0x23   /*Disconnect ULL LE HID asynchronous connectionless link and service, for parameter please refer to structure #bt_ull_le_hid_srv_disconnect_t.*/
#define BT_ULL_ACTION_LE_HID_CONNECT_SERVICE                      0x24   /*Connect ULL LE HID asynchronous connectionless link and service, for parameter please refer to structure #bt_ull_le_hid_srv_disconnect_t.*/
#define BT_ULL_ACTION_LE_HID_SWITCH_LINK_MODE                     0x25   /*Switch link mode, for parameter please refer to structure #bt_ull_le_hid_srv_switch_link_mode_t.*/
#define BT_ULL_ACTION_LE_HID_TX_RACE_DATA                         0x27   /*Send RACE data to remote device, for parameter please refer to structure #bt_ull_le_hid_srv_race_data_t.*/
#define BT_ULL_ACTION_LE_HID_CONTROL_RGB                          0x28   /*Send indicater led control data to remote device, for parameter please refer to structure #bt_ull_le_hid_srv_control_indicater_led_t.*/
#define BT_ULL_ACTION_LE_HID_CHANGE_REPORT_RATE_LEVEL             0x29   /*Change the report rate about keyboard and mouse, for parameter please refer to structure #bt_ull_le_hid_srv_report_rate_level_t.*/
#define BT_ULL_ACTION_LE_HID_SET_SCENARIO                         0x2A   /*Change the using scenario,  for parameter please refer to structure #bt_ull_le_hid_srv_app_scenario_t.*/
#define BT_ULL_ACTION_LE_HID_CANCEL_CONNECT                       0x2B   /*Cancel create ULL LE HID asynchronous connectionless link (server), for parameter please refer to structure #bt_ull_le_hid_srv_cancel_connect_t.*/
#define BT_ULL_ACTION_LE_HID_CANCEL_CREATE_SYNC                   0x2C   /*Cancel create ULL LE HID asynchronous connectionless link (server),x*/
#define BT_ULL_ACTION_LE_HID_SET_IDLE_TIME                        0x2D   /*Change the idle time, 0~200ms, for parameter please refer to structure #bt_ull_le_hid_srv_idle_time_t.*/

/**
 * @brief Define the device type.
 */
typedef uint8_t bt_ull_le_hid_srv_device_t;
#define BT_ULL_LE_HID_SRV_DEVICE_NONE                             0x00   /**< Invalid device type. */
#define BT_ULL_LE_HID_SRV_DEVICE_HEADSET                          0x01   /**< Headset device. */
#define BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD                         0x02   /**< Keyboard device. */
#define BT_ULL_LE_HID_SRV_DEVICE_MOUSE                            0x03   /**< Mouse device. */
#define BT_ULL_LE_HID_SRV_DEVICE_EARBUDS                          0x04   /**< Earbuds device. */

/**
 * @brief Define link mode.
 */
typedef uint8_t bt_ull_le_hid_srv_link_mode_t;
#define BT_ULL_LE_HID_SRV_LINK_MODE_NONE                          0x00   /**< Invalid link mode. */
#define BT_ULL_LE_HID_SRV_LINK_MODE_NORMAL                        0x01   /**< Normal link mode. */
#define BT_ULL_LE_HID_SRV_LINK_MODE_FOTA                          0x02   /**< FOTA link mode. */

/**
 * @brief Define report rate level.
 */
typedef uint8_t bt_ull_le_hid_srv_report_rate_level_t;
#define BT_ULL_LE_HID_SRV_REPORT_RATE_LEVEL_1                     0x00   /*Report rate level1: 1200 Hz. */
#define BT_ULL_LE_HID_SRV_REPORT_RATE_LEVEL_2                     0x01   /*Report rate level1: 800 Hz. */
#define BT_ULL_LE_HID_SRV_REPORT_RATE_LEVEL_DEFAULT BT_ULL_LE_HID_SRV_REPORT_RATE_LEVEL_1 /*Default report rate level: 1200 Hz. */

/**
 * @brief Define app scenario type.
 */
typedef uint8_t bt_ull_le_hid_srv_app_scenario_t;
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_NONE                       0x00   /**< Invalid scenario type. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_1                          0x01   /**< Ull2.0 headset and 1613 hid device co-exsit scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_2                          0x02   /**< Ull2.0 headset and 1623 hid device co-exsit scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_3                          0x03   /**< Only exist 1613 hid device scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_4                          0x04   /**< Pure gaming mouse 4k report rate scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_5                          0x05   /**< Pure gaming mouse 2k report rate scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_6                          0x06   /**< Pure gaming mouse 1k report rate scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_7                          0x07   /**< Pure gaming mouse 500 report rate scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_8                          0x08   /**< Pure gaming mouse 250 report rate scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_9                          0x09   /**< Pure gaming mouse 125 report rate scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_10                         0x0A   /**< Ull2.0 headset and 1613 hid device co-exsit scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_11                         0x0B   /**< Ull2.0 headset and 1613 hid device co-exsit scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_12                         0x0C   /**<  Pure gaming mouse and gaming kebord co-exsit scenario. */
#define BT_ULL_LE_HID_SRV_APP_SCENARIO_13                         0x0D   /**<  Pure gaming mouse and gaming kebord co-exsit scenario. */

/**
 * @brief Define idle time.
 */
typedef uint16_t bt_ull_le_hid_srv_idle_time_t;

/**
 * @brief Define moudle type.
 */
typedef uint8_t bt_ull_le_hid_srv_module_t;
#define BT_ULL_LE_HID_SRV_MODULE_APP                              0x00   /**< Application module type. */
#define BT_ULL_LE_HID_SRV_MODULE_AIR_SRV                          0x01   /**< Air service module type. */
#define BT_ULL_LE_HID_SRV_MODULE_MAX                              0x02   /**< Invalid module type. */

/**
 * @}
 */

/**
 * @defgroup BluetoothServices_ULL_LE_HID_struct Struct.
 * @{
 * Defines ULL LE HID structures for associated APIs and events.
 */

/**
 * @brief Define the connection parameters.
 */
typedef struct {
    bt_ull_le_hid_srv_device_t             device_type;                  /**< The device type. */
    uint8_t                                list_num;                     /**< The peer addr list number. */
    uint8_t                                peer_addr_list[1];            /**< Set a dynamic buffer for peer bt address list, size: sizeof(bt_addr_t) * list_num. */
} bt_ull_le_hid_srv_conn_params_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_LE_HID_CONNECT.
 *            This structure is used to connect asynchronous connectionless link.
 */
typedef struct {
    bt_ull_le_hid_srv_conn_params_t        *audio_sink;                  /**< The aduio sink (including headset and earbuds) connection parameters pointer to connect. */
    bt_ull_le_hid_srv_conn_params_t        *keyboard;                    /**< Keyboard connection parameters pointer to connect. */
    bt_ull_le_hid_srv_conn_params_t        *mouse;                       /**< Mouse connection parameters pointer to connect. */
} bt_ull_le_hid_srv_connect_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_LE_HID_CREATE_SYNC,
 *            which is used to create sync asynchronous connectionless link.
 */
typedef struct {
    bt_ull_le_hid_srv_device_t             device_type;                  /**< The device type to ceate sync. */
    bt_addr_t                              peer_addr;                    /**< The remote device address to create sync.*/
} bt_ull_le_hid_srv_create_sync_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_LE_HID_CANCEL_CONNECT or BT_ULL_ACTION_LE_HID_CANCEL_CREATE_SYNC,
 *            which is used to create sync asynchronous connectionless link.
 */
typedef struct {
    bt_ull_le_hid_srv_device_t             device_type;                  /**< The device type. */
} bt_ull_le_hid_srv_cancel_connect_t, 
bt_ull_le_hid_srv_cancel_create_sync_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_LE_HID_DISCONNECT,
 *            which is used to disconnect asynchronous connectionless link and service.
 */
typedef struct {
    bt_handle_t                            handle;                       /**< The LE ACL connection handle to disconnect. */
    uint8_t                                reason;                       /**< The reason to disconnect. */
} bt_ull_le_hid_srv_disconnect_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_LE_HID_BOND or BT_ULL_ACTION_LE_HID_CONNECT_SERVICE.
 *
 */
typedef struct {
    bt_handle_t                            handle;                       /**< The LE ACL connection handle. */
} bt_ull_le_hid_srv_bond_t,
bt_ull_le_hid_srv_connect_service_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_LE_HID_SWITCH_LINK_MODE or BT_ULL_EVENT_LE_HID_SWITCH_LINK_MODE_IND,
 *            which is used to switch link mode.
 */
typedef struct {
    bt_handle_t                            handle;                       /**< The LE ACL connection handle. */
    bt_ull_le_hid_srv_link_mode_t          link_mode;                    /**< The link mode to switch. */
} bt_ull_le_hid_srv_switch_link_mode_t,
bt_ull_le_hid_srv_switch_link_mode_ind_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_ACTION_LE_HID_CONTROL_RGB,
 *            which is used to control rgb info.
 */
typedef struct {
    uint8_t                                indicater_led;                /**< The indicater led. */
} bt_ull_le_hid_srv_control_indicater_led_t;

/**
 * @brief This structure defines the parameters of event #BT_ULL_EVENT_LE_HID_CONNECTED_IND,
 *         which indicates the ULL LE HID asynchronous connectionless link is connected.
 */
typedef struct {
    bt_status_t                            status;                       /**< Status. */
    bt_handle_t                            handle;                       /**< The LE ACL connection handle. */
    bt_ull_le_hid_srv_device_t             device_type;                  /**< The device type. */
    bt_addr_t                              peer_addr;                    /**< The remote device address.*/
} bt_ull_le_hid_srv_connected_ind_t;

/**
 * @brief This structure defines the parameters of event #BT_ULL_EVENT_LE_HID_DISCONNECTED_IND,
 *         which indicates the ULL LE HID asynchronous connectionless link is disconnected.
 */
typedef struct {
    bt_status_t                            status;                       /**< Status. */
    bt_handle_t                            handle;                       /**< The LE ACL connection handle. */
    bt_ull_le_hid_srv_device_t             device_type;                  /**< The device type. */
    uint8_t                                reason;                       /**< The reason to disconnect. */
    bt_addr_t                              peer_addr;                    /**< The remote device address.*/
} bt_ull_le_hid_srv_disconnected_ind_t;

/**
 * @brief This structure defines the parameters of event #BT_ULL_EVENT_LE_HID_BONDING_COMPLETE_IND or BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND,
 *         which indicates the ULL LE HID service is bonded or connected.
 */
typedef struct {
    bt_status_t                            status;                       /**< Status. */
    bt_handle_t                            handle;                       /**< The LE ACL connection handle. */
    bt_addr_t                              peer_addr;                    /**< The remote device address.*/
} bt_ull_le_hid_srv_bonding_complete_ind_t,
bt_ull_le_hid_srv_service_connected_ind_t;

/**
 * @brief Define the parameters related to FOTA.
 */
typedef struct {
    bt_ull_le_hid_srv_device_t             device_type;                  /**< The device type. */
    uint16_t                               mtu;                          /**< The max transport unit size about ull hid service. */
    uint16_t                               att_handle_rx;                /**< The attribute handle of air service rx. */
    uint16_t                               att_handle_tx;                /**< The attribute handle of air service tx. */
    uint16_t                               att_handle_cccd;              /**< The attribute handle of air service cccd. */
} bt_ull_le_hid_srv_fota_info;

/**
 * @brief This structure defines the parameters of #BT_ULL_EVENT_LE_HID_INPUT_REPORT_IND,
 *          which is used for user to receive the input report data.
 */
typedef bt_ull_user_data_t bt_ull_le_hid_srv_input_report_data_t;

/**
 * @brief This structure defines the parameters of #BT_ULL_EVENT_LE_HID_RACE_DATA_IND,
 *          which is used for user to receive the RACE data.
 */
typedef bt_ull_user_data_t bt_ull_le_hid_srv_race_data_t;


/**
 * @}
 */

/**
 * @brief   This function is used for application to initialize Ultra Low Latency LE with HID service.
 * @param[in] role        is server or client.
 * @param[in] callback    is callback function.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                             #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_hid_srv_init(bt_ull_role_t role, bt_ull_callback cb);

/**
 * @brief   This function is used to register callback funtion to Ultra Low Latency LE with HID service.
 * @param[in] module is module type.
 * @param[in] cb is callback function.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                             #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_hid_srv_register_callback(bt_ull_le_hid_srv_module_t module, bt_ull_callback cb);


/**
 * @brief   This function is used to control Ultra Low Latency LE with HID service.
 * @param[in] action       is the user request action type.
 * @param[in] param        is the payload of the request action.
 * @param[in] param_len    is the payload length.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                             #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_hid_srv_action(bt_ull_action_t action, const void *param, uint32_t param_len);

/**
 * @brief   This function sets the device information of ULL Client.
 * @param[in] dev_info    is the device information of ULL Client device.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                             #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_hid_srv_set_device_info(bt_ull_le_device_info_t *dev_info);

/**
 * @brief   This function is used get FOTA information.
 * @param[in] addr        is pointer of BT address.
 * @param[in] info          is pointer of fota information.
 * @return                 None
 */
void bt_ull_le_hid_srv_get_fota_info(bt_addr_t *addr, bt_ull_le_hid_srv_fota_info *info);

/**
 * @brief   This function is used to get the number of the bonded devices.
 * @param[in] device_type        is the device type.
 * @return           The number of the bonded devices.
 */
uint8_t bt_ull_le_hid_srv_get_bonded_device_num(bt_ull_le_hid_srv_device_t device_type);

/**
 * @brief   This function is used to get the number of the bonded devices.
 * @param[in] device_type is the device type.
 * @param[in] count            is number of bonded devices.
 * @param[in] list                 is the BT address list of bonded devices.
 * @return           The number of the bonded devices.
 */
void bt_ull_le_hid_srv_get_bonded_device_list(bt_ull_le_hid_srv_device_t device_type, uint8_t count, bt_addr_t *list);

/**
 * @brief   This function is used to get the bonded status of the device specified by the address and device type.
 * @param[in] device_type is the device type.
 * @param[in] addr            is BT address of bonded devices.
 * @return           True, bonded.
 * @                    False, unbonded.
 */
bool bt_ull_le_hid_srv_is_bonded_device(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr);

/**
 * @brief   This function is used to clear bonded info of the device specified by device type.
 * @param[in] device_type is the device type.
 * @return                #BT_STATUS_SUCCESS, the operation completed successfully.
 *                             #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_hid_srv_clear_bonded_list(bt_ull_le_hid_srv_device_t device_type);

/**
 * @brief   This function is used to get role of ULL HID service .
 * @return                #bt_ull_role_t, the role of ULL HID service.
 */
bt_ull_role_t bt_ull_le_hid_srv_get_role(void);

/**
 * @}
 * @}
 */

#endif /* __BT_ULL_LE_HID_SERVICE_H__ */

