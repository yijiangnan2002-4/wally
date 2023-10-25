/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef __BT_FAST_PAIR_H__
#define __BT_FAST_PAIR_H__

/**
 * @addtogroup Bluetooth_Services_Group Bluetooth Services
 * @{
 * @addtogroup BluetoothServices_FAST_PAIR BT FAST PAIR
 * @{
 * This section describes the Google fast pair service (GFPS) APIs and events.
 * Google fast pair utilizes Bluetooth Low Energy in Android system to detect audio sink devices, eg. headphones, speakers and carkits, which are nearby and ready for pairing.
 * This profile defines two roles: Fast Pair Seeker and Fast Pair Provider. The APIs only support the provider role.
 * Before you use this service, you must complete the model registration with google.
 * Then, google will distribute a Model ID and Anti-Spoofing Private Key to you.
 * Besides, you can provide your tx power level value to google during model registration.
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                  |
 * |------------------------------|-------------------------------------------------------------------------|
 * |\b GFPS                        | Google Fast Pair Service. This service provides a fast method to establish EDR link . |
 *
 * @section bt_fast_pair_api_usage How to use this module
 * - Step 1. Add @ref bt_fast_pair_service to the application's GATT service database. Then return the bt_gatts_service_t pointer to GATTS stack by implementing the user-defined callback bt_get_gatt_server().
 *  - Sample Code:
 *     @code
 *               // The GATTS collects all services.
 *               const bt_gatts_service_t * ble_gatt_server[] = {
 *                   &ble_gap_service,  // handle range: 0x0001 to 0x0009.
 *                   &ble_gatt_service, // handle range: 0x0011 to 0x0015.
 *                   &bt_fast_pair_service,  // handle range: 0x0080 to 0x0089.
 *                   NULL
 *                };
 *                const bt_gatts_service_t** bt_get_gatt_server()
 *                {
 *                    return ble_gatt_server;
 *                }
 *     @endcode
 *
 * - Step 2. Implement the user-defined callback @ref bt_fast_pair_app_event_callback() to listen to the event from fast pair module.
 *  - Sample Code:
 *     @code
 *                bt_fast_pair_status_t bt_fast_pair_app_event_callback(bt_fast_pair_app_event_t evt, void *buf)
 *                {
 *                    switch(evt) {
 *                        case BT_FAST_PAIR_APP_EVENT_NEED_STORE_ACCOUNT_KEY: {
 *                            bt_fast_pair_account_key_list_t *key = (bt_fast_pair_account_key_list_t *)buf;
 *                            // Save the account key to flash or other nonvolatile memory.
 *                        }
 *                            break;
 *                        case BT_FAST_PAIR_APP_EVENT_NEED_DISCOVERABLE: {
 *                            // Open the EDR discoverable.
 *                        }
 *                            break;
 *                        BT_FAST_PAIR_APP_EVENT_PAIR_STARTED: {
 *                            bt_fast_pair_started_t *start_evt = (bt_fast_pair_started_t *)buf;
 *                            // Do the fast pair started flow.
 *                        }
 *                            break;
 *                        BT_FAST_PAIR_APP_EVENT_PAIR_COMPLETE: {
 *                            bt_fast_pair_complete_t *start_evt = (bt_fast_pair_complete_t *)buf;
 *                            // Do the fast pair complete flow.
 *                        }
 *                            break;
 *                        default:
 *                            return BT_FAST_PAIR_STATUS_FAIL;
 *                    }
 *                    return BT_FAST_PAIR_STATUS_SUCCESS;
 *                }
 *     @endcode
 *
 * - Step 3. Implement the user-defined function @ref bt_fast_pair_get_account_key_list() to return the account key buffer to fast pair module.
 *   It is recommended that this buffer be kept during the whole BT power life cycle. The buffer should not be released before advertising stops or fast pair flow completes.
 *  - Sample Code:
 *     @code
 *                static bt_fast_pair_account_key_list_t *g_fast_pair_acc_key_list = pvPortmalloc(sizeof(bt_fast_pair_account_key_list_t) + sizeof(bt_fast_pair_account_key_t) * (BT_FAST_PAIR_ACCOUNT_MAX_KEY_NUM - 1));
 *                // Load previous account key lists to this buffer if possible.
 *                bt_fast_pair_account_key_list_t *bt_fast_pair_get_account_key_list()
 *                {
 *                    return g_fast_pair_acc_key_list;
 *                }
 *     @endcode
 *
 * - Step 4. Initialize fast pair module via @ref bt_fast_pair_init().
 *  - Sample code:
 *     @code
 *                int8_t  fast_pair_app_tx_power_level = 0xD8;
 *                uint32_t fast_pair_app_model_id = 0xaabbcc;      //From model registration.
 *                uint8_t  fast_pair_app_private_key[32] = {...};        //From model registration, no need a global variable.
 *                bt_fast_pair_init_parameters_t init_data = {
 *                    .model_id = fast_pair_app_model_id,
 *                    .private_key = fast_pair_app_private_key,
 *                    .tx_power_level = &fast_pair_app_tx_power_level    //If you had provided tx power level to google by model registration, this can be NULL.
 *                }
 *                bt_fast_pair_init(&init_data);
 *     @endcode
 *
 * - Step 5. Start BLE advertising with the advertising data from @ref bt_fast_pair_get_advertising_data().
 *  - Sample code:
 *     @code
 *                bt_status_t status = BT_STATUS_SUCCESS;
 *                bt_hci_cmd_le_set_advertising_enable_t adv_enable = {
 *                    .advertising_enable = BT_HCI_ENABLE,
 *                };
 *                bt_hci_cmd_le_set_advertising_parameters_t adv_para = {
 *                    .advertising_interval_min = BT_FAST_PAIR_ADV_MIN_INTERVAL,
 *                    .advertising_interval_max = BT_FAST_PAIR_DISCOVERABLE_ADV_MAX_INTERVAL,
 *                    .advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED,
 *                    .own_address_type = BT_ADDR_RANDOM,
 *                    .advertising_channel_map = 7,
 *                    .advertising_filter_policy = 0
 *                };
 *                bt_hci_cmd_le_set_advertising_data_t adv_data= {0};
 *                uint8_t adv_length = sizeof(adv_data.advertising_data);
 *
 *                //if (EDR is discoverable now) {
 *                    //fast_pair advertising data field.
 *                    bt_fast_pair_get_advertising_data(adv_data.advertising_data, &adv_length, BT_FAST_PAIR_ADVERTISING_DATA_MODEL_ID);
 *                //} else {
 *                    adv_para.advertising_interval_max = BT_FAST_PAIRF_NOT_DISCOVERABLE_ADV_MAX_INTERVAL;
 *                    //fast_pair advertising data field.
 *                    bt_fast_pair_get_advertising_data(adv_data.advertising_data, &adv_length, BT_FAST_PAIR_ADVERTISING_DATA_ACCOUNT);
 *                //}
 *                //ohter advertising data field.
 *                // Todo: add other advertising data.
 *
 *                if (BT_STATUS_SUCCESS != (status = bt_gap_le_set_advertising(&adv_enable, &adv_para, &adv_data, &adv_data))) {
 *                    //error handle.
 *                }
 *     @endcode
 * - Step 6. Stop BLE advertising if necessary.
 *  - Sample code:
 *     @code
 *               bt_status_t status = BT_STATUS_SUCCESS;
 *               bt_hci_cmd_le_set_advertising_enable_t adv_enable = {
 *                   .advertising_enable = BT_HCI_DISABLE
 *               };
 *
 *               if (BT_STATUS_SUCCESS != (status = bt_gap_le_set_advertising(&adv_enable, NULL, NULL, NULL))) {
 *                   //error handle.
 *               }
 *     @endcode
 */

#include "bt_type.h"
#include "bt_system.h"
#include "bt_platform.h"
#include "bt_gatts.h"

BT_EXTERN_C_BEGIN

#ifndef PACKED
#define PACKED  __attribute__((packed))         /**< The shorthand of __attribute__((packed)), means the format is not 4-bytes align. */
#endif

/**
 * @defgroup BluetoothServices_FAST_PAIR_define Define
 * @{
 * Define Bluetooth FAST_PAIR data types and values.
 */
#define BT_FAST_PAIR_ACCOUNT_MAX_KEY_NUM                (12)    /**< The maximum number of account key. */
#define BT_FAST_PAIR_ACCOUNT_KEY_SIZE                   (16)    /**< The size of account key. */
#define BT_FAST_PAIR_ADV_MIN_INTERVAL                   (0xA0)  /**< Define the minimum value of BLE advertising interval as 100ms. */
#define BT_FAST_PAIR_DISCOVERABLE_ADV_MAX_INTERVAL      (0xA0)  /**< Define the maximum value of BLE advertising interval as 100ms when EDR is discoverable. */
#define BT_FAST_PAIR_NOT_DISCOVERABLE_ADV_MAX_INTERVAL  (0x190) /**< Define the maximum value of BLE advertising interval as 250ms when EDR is not discoverable. */
#define BT_FAST_PAIR_COMPONENT_MAXIMUM                  (0x0A)  /**< Define the maximum components number, this will be used to battery notiry and ring request and other features. */
#define BT_FAST_PAIR_ADDITIONAL_DATA_MAXIMUM_LEN        (128)   /**< Define the maximum additional data length. */
#define BT_FAST_PAIR_EVENT_BUFFER_MAXIMUM_LENGTH        (136)   /**< Define the maximum fast pair event buffer length. */

/**
 *  @brief      FAST_PAIR status definitions.
 */
#define BT_FAST_PAIR_STATUS_SUCCESS                     (0x00)  /**< FAST_PAIR success. */
#define BT_FAST_PAIR_STATUS_INVALID_PRIVATE_KEY         (0x01)  /**< FAST_PAIR fail due to invalid private key. */
#define BT_FAST_PAIR_STATUS_INVALID_LENGTH              (0x02)  /**< FAST_PAIR fail due to invalid length. */
#define BT_FAST_PAIR_STATUS_INVALID_KEY_BUFFER          (0x03)  /**< FAST_PAIR fail due to invalid key buffer. */
#define BT_FAST_PAIR_STATUS_INVALID_PARAMETERS          (0x04)  /**< FAST_PAIR general invalid parameters. */
#define BT_FAST_PAIR_STATUS_NOT_CONNECTED               (0x05)  /**< FAST_PAIR sersvice not connected. */
#define BT_FAST_PAIR_STATUS_FAIL                        (~(0U)) /**< FAST_PAIR general fail. */
typedef uint32_t bt_fast_pair_status_t;             /**< Define the FAST_PAIR status type, returned from the APIs, and defined as BT_FAST_PAIR_STATUS_XXX. */

#define BT_FAST_PAIR_ADVERTISING_DATA_MODEL_ID              (0x00)  /**< Advertising data of model id. */
#define BT_FAST_PAIR_ADVERTISING_DATA_ACCOUNT               (0x01)  /**< Advertising data of account key filter. */
#define BT_FAST_PAIR_ADVERTISING_DATA_ACCOUNT_AND_BATTERY   (0x02)  /**< Advertising data of account key filter and battery values. */
typedef uint32_t bt_fast_pair_advertising_data_t;  /**< The FAST_PAIR advertising data type. */

#define BT_FAST_PAIR_APP_EVENT_NEED_STORE_ACCOUNT_KEY   (0x00)  /**< FAST_PAIR event of need store account key, with #bt_fast_pair_account_key_list_t payload. */
#define BT_FAST_PAIR_APP_EVENT_NEED_DISCOVERABLE        (0x01)  /**< FAST_PAIR event of need discoverable, with null payload. */
#define BT_FAST_PAIR_APP_EVENT_PAIR_STARTED             (0x02)  /**< FAST_PAIR event of pair flow started, with #bt_fast_pair_started_t payload. */
#define BT_FAST_PAIR_APP_EVENT_PAIR_COMPLETE            (0x03)  /**< FAST_PAIR event of pair flow complete, with #bt_fast_pair_complete_t payload. */
#define BT_FAST_PAIR_APP_EVENT_NEED_INITIAL_CONNECT     (0x04)  /**< FAST_PAIR event of user need initial connection, with remote address bt_bd_addr_t payload. */
#define BT_FAST_PAIR_APP_EVENT_SERVICE_CONNECTED        (0x05)  /**< FAST_PAIR event of service connected, with remote address bt_bd_addr_t payload. */
#define BT_FAST_PAIR_APP_EVENT_SERVICE_DISCONNECTED     (0x06)  /**< FAST_PAIR event of service disconnected, with remote address bt_bd_addr_t payload. */
#define BT_FAST_PAIR_APP_EVENT_RING_REQUEST             (0x07)  /**< FAST_PAIR event of ring request from remote device, with #bt_fast_pair_ring_request_t payload, user need reply #bt_fast_pair_send_acknowledgements() within 1 second. */
#define BT_FAST_PAIR_APP_EVENT_MESSAGE_STREAM           (0x08)  /**< FAST_PAIR event of message stream from remote device, with #bt_fast_pair_message_stream_t playload. */
#define BT_FAST_PAIR_APP_EVENT_ADDITIONAL_DATA          (0x09)  /**< FAST_PAIR event received additional data from remote fast pair connected device, with #bt_fast_pair_additional_data_t payload. */
#define BT_FAST_PAIR_APP_EVENT_ACTIVE_COMPONENTS_REQ    (0x0A)  /**< FAST_PAIR event received active components request from remote fast pair connected device, without payload. */
#define BT_FAST_PAIR_APP_EVENT_WAIT_CONNECTION          (0x0B)  /**< FAST_PAIR event received pair or reconnect flow started, with null payload. */
#define BT_FAST_PAIR_APP_EVENT_SASS_MSG_RECEIVED        (0x0C)  /**< FAST_PAIR event received SASS message. */
#define BT_FAST_PAIR_APP_EVENT_SASS_UPDATE_IN_USE_ACCOUNT (0xD) /**< FAST_PAIR event received in use account key ID changed. */
#define BT_FAST_PAIR_APP_EVENT_SASS_UPDATE_CONNECTED_RECORD (0xE) /**< FAST_PAIR event received connected record changed. */
#define BT_FAST_PAIR_APP_EVENT_SASS_PEER_STATE_RECEIVED (0xF)    /**< FAST_PAIR event received peer SASS state from AWS data. */
typedef uint32_t bt_fast_pair_app_event_t;              /**< The FAST_PAIR event type. */

#define BT_FAST_PAIR_ADDITIONAL_DATA_ID_PERSONALIZED_NAME   (0x01)  /**< FAST_PAIR additional data id of personalized name. */
typedef uint8_t bt_fast_pair_additional_data_id_t;    /**< FAST_PAIR additional data id. */

#define BT_FAST_PAIR_MESSAGE_GROUP_BLUETOOTH_EVENT      (0x01)  /**< FAST_PAIR bluetooth event message group id . */
#define BT_FAST_PAIR_MESSAGE_GROUP_COMPANION_APP_EVENT  (0x02)  /**< FAST_PAIR companion app event message group id . */
#define BT_FAST_PAIR_MESSAGE_GROUP_DEVICE_INFO_EVENT    (0x03)  /**< FAST_PAIR device information event message group id . */
#define BT_FAST_PAIR_MESSAGE_GROUP_DEVICE_ACTION_EVENT  (0x04)  /**< FAST_PAIR device action event message group id . */
#define BT_FAST_PAIR_MESSAGE_GROUP_DEVICE_RUNTIME_CONFIG_EVENT (0x05)  /**< FAST_PAIR device runtime configuration event message group id . */
#define BT_FAST_PAIR_MESSAGE_GROUP_DEVICE_CAP_SYNC_EVENT (0x06)  /**< FAST_PAIR device capability sync event message group id . */
#define BT_FAST_PAIR_MESSAGE_GROUP_SMART_AUDIO_SOURCE_SWITCHING (0x07) /**< FAST_PAIR smart audio source switching message group id . */
#define BT_FAST_PAIR_MESSAGE_GROUP_ACKNOWLEDGEMENT      (0xFF)  /**< FAST_PAIR acknowledgement message group id . */
typedef uint8_t bt_fast_pair_message_group_t;       /**< The FAST_PAIR message group id. */

#define BT_FAST_PAIR_MESSAGE_CODE_BLUETOOTH_EVENT_ENABLE_SILENCE_MODE       (0x01)  /**< FAST_PAIR enable silence message code id in bluetooth event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_BLUETOOTH_EVENT_DISABLE_SILENCE_MODE      (0x02)  /**< FAST_PAIR disable silence message code id in bluetooth event message group. */
typedef uint8_t bt_fast_pair_message_code_bluetooth_evt_t;      /**< FAST_PAIR message code id in bluetooth event message group. */

#define BT_FAST_PAIR_MESSAGE_CODE_COMPANION_APP_EVENT_LOG_BUFFER_FULL       (0x01)  /**< FAST_PAIR log buffer full message code id in companion event message group. */
typedef uint8_t bt_fast_pair_message_code_companion_app_evt_t;  /**< FAST_PAIR message code id in companion event message group. */

#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_MODEL_ID                (0x01)  /**< FAST_PAIR model id message code id in device informaiton event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_BLE_ADDR_UPDATE         (0x02)  /**< FAST_PAIR ble address update message code id in device informaiton event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_BATTERY_UPDATED         (0x03)  /**< FAST_PAIR battery update message code id in device informaiton event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_REMAINING_BATTERY_TIME  (0x04)  /**< FAST_PAIR remaining battery time message code id in device informaiton event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_ACTIVE_COMPONENTS_REQ   (0x05)  /**< FAST_PAIR active components request message code id in device informaiton event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_ACTIVE_COMPONENTS_RSP   (0x06)  /**< FAST_PAIR active components response message code id in device informaiton event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_CAPABILITIES            (0x07)  /**< FAST_PAIR capabilities message code id in device informaiton event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_PLATFORM_TYPE           (0x08)  /**< FAST_PAIR platform type message code id in device informaiton event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_FIRMWARE_REVISION       (0x09)  /**< FAST_PAIR firmware revision message code id in device informaiton event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_SESSION_NONCE           (0x0A)  /**< FAST_PAIR session nonce messsage code in in device informaiton event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_CUR_EDDYSTONE_IDENTIFIER (0x0B)  /**< FAST_PAIR eddystone identifier code in in device informaiton event message group. */
typedef uint8_t bt_fast_pair_message_code_device_info_evt_t;    /**< FAST_PAIR message code id in device informaiton event message group. */

#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_ACTION_EVENT_RING                  (0x01)  /**< FAST_PAIR ring request message code id in device action event message group. */
typedef uint8_t bt_fast_pair_message_code_device_action_evt_t;  /**< FAST_PAIR message code id in device action event message group. */

#define BT_FAST_PAIR_MESSAGE_DEVICE_RUNTIME_CONFIG_EVENT_DYNAMIC_BUFFER_SIZING   (0x01)  /**< FAST_PAIR dynamic buffer sizing message code id in device runtime configuration event message group. */
typedef uint8_t bt_fast_pair_message_code_device_runtime_configuration_evt_t;  /**< FAST_PAIR message code id in device runtime configuration event message group. */

#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_CAP_SYNC_EVENT_REQUEST_CAP_UPDATE   (0x01)  /**< FAST_PAIR request capability update message code id in device capability event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_CAP_SYNC_EVENT_DYNAMIC_BUFFER_SIZE  (0x02)  /**< FAST_PAIR dynamic buffer size message code id in device capability event message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_DEVICE_CAP_SYNC_EVENT_EDDYSTONE_TRACKING   (0x03)  /**< Eddystone tracing(spot) message code id in device capability event message group. */
typedef uint8_t bt_fast_pair_message_code_device_capability_sync_evt_t; /**< FAST_PAIR message code id in device capability event message group. */

#define BT_FAST_PAIR_MESSAGE_CODE_SASS_GET_CAPABILITY                       (0x10)  /**< FAST_PAIR get capability of SASS in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_NOTIFY_CAPABILITY                    (0x11)  /**< FAST_PAIR notify capability of SASS in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_SET_MULTIPOINT_STATE                 (0x12)  /**< FAST_PAIR set multipoint state in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_SET_SWITCHING_PREFERENCE             (0x20)  /**< FAST_PAIR set switching preference in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_GET_SWITCHING_PREFERENCE             (0x21)  /**< FAST_PAIR get switching preference in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_NOTIFY_SWITCHING_PREFERENCE          (0x22)  /**< FAST_PAIR notify switching preference in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_ACTIVE_AUDIO_SOURCE           (0x30)  /**< FAST_PAIR switch active audio source (to connected device) in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_BACK                          (0x31)  /**< FAST_PAIR switch back (to disconnected device) in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_NOTIFY_MULTIPOINT_SWITCH             (0x32)  /**< FAST_PAIR notify multipoint-switch event in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_GET_CONNECTION_STATUS                (0x33)  /**< FAST_PAIR get connection status in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_NOTIFY_CONNECTION_STATUS             (0x34)  /**< FAST_PAIR notify connection status in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_NOTIFY_SASS_INITIATED_CONNECTION     (0x40)  /**< FAST_PAIR notify SASS initiated connection in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_INDICATE_IN_USE_ACCOUNT_KEY          (0x41)  /**< FAST_PAIR indicate in use account key in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_SEND_CUSTOM_DATA                     (0x42)  /**< FAST_PAIR send custom data in smart audio source switching message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_SASS_SET_DROP_CONNECTION_TARGET           (0x43)  /**< FAST_PAIR set drop connection target in smart audio source switching message group. */
typedef uint8_t bt_fast_pair_message_code_sass_evt_t; /**< FAST_PAIR message code id in smart audio source switching message group. */

#define BT_FAST_PAIR_MESSAGE_CODE_ACKNOWLEDGEMENT_ACK                       (0x01)  /**< FAST_PAIR ack message code id in acknowledgement message group. */
#define BT_FAST_PAIR_MESSAGE_CODE_ACKNOWLEDGEMENT_NAK                       (0x02)  /**< FAST_PAIR nak message code id in acknowledgement message group. */
typedef uint8_t bt_fast_pair_message_code_acknowledgement_t;    /**< FAST_PAIR message code id in acknowledgement message group. */

#define BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_NOT_SUPPORTED                 (0x00)  /**< FAST_PAIR message acknowledgement nak reason of not supported. */
#define BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_DEVICE_BUSY                   (0x01)  /**< FAST_PAIR message acknowledgement nak reason of device busy. */
#define BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_CURRENT_STATE                 (0x02)  /**< FAST_PAIR message acknowledgement nak reason of not allowed due to current state. */
#define BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_INCORRECT_MAC                 (0x03)  /**< FAST_PAIR message acknowledgement nak reason of not allowed due to incorrect message authentication code. */
#define BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_REDUNDANT_DEVICE_ACTION       (0x04)  /**< FAST_PAIR message acknowledgement nak reason of the action is redundant. */
typedef uint8_t bt_fast_pair_acknowledgement_nak_reason_t;       /**< FAST_PAIR message acknowledgement nak reason.  */
#define BT_FAST_PAIR_ACKNOLEGEMENT_NAK_REASON_NOT_SUPPORTED                   BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_NOT_SUPPORTED     /**< Wrong typo of BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_NOT_SUPPORTED. */
#define BT_FAST_PAIR_ACKNOLEGEMENT_NAK_REASON_DEVICE_BUSY                     BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_DEVICE_BUSY     /**< Wrong typo of BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_DEVICE_BUSY. */
#define BT_FAST_PAIR_ACKNOLEGEMENT_NAK_REASON_CURRENT_STATE                   BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_CURRENT_STATE     /**< Wrong typo of BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_CURRENT_STATE. */

typedef uint8_t bt_fast_pair_account_key_t[BT_FAST_PAIR_ACCOUNT_KEY_SIZE];    /**< Define the account key type. */

#define BT_FAST_PAIR_REMAINING_TIME_INVALID_VALUE                           (0xFFFF)    /**< FAST_PAIR invalid or unkown remaining time. */

#define BT_FAST_PAIR_FIRMWARE_REVISION_MAX_LENGTH                           (40)    /**< The maximum length of firmware revision. */

#define BT_FAST_PAIR_MESSAGE_NONCE_LENGTH                                   (8) /**< The length of message nonce. */

#define BT_FAST_PAIR_MESSAGE_AUTH_CODE_LENGTH                               (8) /**< The length of message authentication code. */


#define BT_FAST_PAIR_SASS_ENCRYPTED_CONNECTION_STATUS_MAX_LENGTH            (6) /**< The maximum length of encrypted_connection_status. */
/**
 *  @brief      SPOT ring req type define.
 */
#define BT_FAST_PAIR_SPOT_RING_REQ_RING (0x00)                  /**< SPOT request to ring the device, reference bt_fast_pair_spot_ring_type_t for details. */
#define BT_FAST_PAIR_SPOT_RING_REQ_GET_RING_CAPABLE (0x01)      /**< SPOT request to get the ring capable, reference bt_fast_pair_spot_ring_capable_t for details. */
#define BT_FAST_PAIR_SPOT_RING_REQ_GET_RING_TYPE (0x02)         /**< SPOT request to get the ring type, reference bt_fast_pair_spot_ring_state_t for details. */
typedef uint8_t bt_fast_pair_spot_ring_req_type_t;          /**< define the ring request type. */

/**
 *  @brief      SPOT ring type define.
 */
#define BT_FAST_PAIR_SPOT_RING_TYPE_NONE    (0x00)              /**< SPOT request to stop the ring. */
#define BT_FAST_PAIR_SPOT_RING_TYPE_LEFT    (0x01)              /**< SPOT request to ring the L side. */
#define BT_FAST_PAIR_SPOT_RING_TYPE_RIGHT   (0x02)              /**< SPOT request to ring the R side. */
#define BT_FAST_PAIR_SPOT_RING_TYPE_CASE    (0x04)              /**< SPOT request to ring the charger case. */
#define BT_FAST_PAIR_SPOT_RING_TYPE_ALL     (0xFF)              /**< SPOT request to ring all devices. */
typedef uint8_t bt_fast_pair_spot_ring_type_t;              /**< define the ring type. */

/**
 *  @brief      SPOT ring capable define.
 */
#define BT_FAST_PAIR_SPOT_RING_CAPABLE_NONE          (0x00)     /**< The ringing is not supported. */
#define BT_FAST_PAIR_SPOT_RING_CAPABLE_SINGLE        (0x01)     /**< Only one device is supported, for headset. */
#define BT_FAST_PAIR_SPOT_RING_CAPABLE_LR            (0x02)     /**< L&R are supported, for earbuds. */
#define BT_FAST_PAIR_SPOT_RING_CAPABLE_LR_AND_CASE   (0x03)     /**< L&R and charger case are supported, for earbuds and charger case. */
typedef uint8_t bt_fast_pair_spot_ring_capable_t;           /**< define the capable type. */

/**
 *  @brief      SPOT ring state define.
 */
#define BT_FAST_PAIR_SPOT_RING_STA_STARTED           (0x00)     /**< The ringing is already started. */
#define BT_FAST_PAIR_SPOT_RING_STA_FAIL              (0x01)     /**< Something wrong happend while ringing. */
#define BT_FAST_PAIR_SPOT_RING_STOPPED_BY_TIMEOUT    (0x02)     /**< The ringing was already stopped due to timeout. */
#define BT_FAST_PAIR_SPOT_RING_STOPPED_BY_BTN        (0x03)     /**< The ringing was stopped by user. */
#define BT_FAST_PAIR_SPOT_RING_STOPPED_BY_GATT       (0x04)     /**< The ringing was stopped by SPOT client. */
typedef uint8_t bt_fast_pair_spot_ring_state_t;             /**< Define the ring state. */

/**
 *  @brief      SPOT event define.
 */
typedef enum {
    BT_FAST_PAIR_SPOT_EVENT_START_ADV = 0x1000,                 /**< SPOT request to start the advertising. */
    BT_FAST_PAIR_SPOT_EVENT_STOP_ADV,                           /**< SPOT request to stop the advertising. */
    BT_FAST_PAIR_SPOT_EVENT_EID_READY,                          /**< Notify that the EID is ready. */
    BT_FAST_PAIR_SPOT_EVENT_EID_UPDATED,                        /**< Notify that the EID is updated. */
} bt_fast_pair_spot_adv_event;

typedef void (*bt_fast_pair_spot_adv_cb_t)(bt_fast_pair_spot_adv_event event); /**< The SPOT adv callback. */


#define BT_FAST_PAIR_SASS_CAPABILITY_STATE_BIT                      (7) /**< The bit of SASS capability state, the MSB 0 of the 1st Byte. */
#define BT_FAST_PAIR_SASS_CAPABILITY_MULTIPOINT_CONFIG_BIT          (6) /**< The bit of SASS capability multipoint configurability, the MSB 1 of the 1st Byte. */
#define BT_FAST_PAIR_SASS_CAPABILITY_MULTIPOINT_STATE_BIT           (5) /**< The bit of SASS capability multipoint current state, the MSB 2 of the 1st Byte. */
#define BT_FAST_PAIR_SASS_CAPABILITY_ON_HEAD_DETECTION_BIT          (4) /**< The bit of SASS capability support on head detection, the MSB 3 of the 1st Byte. */
#define BT_FAST_PAIR_SASS_CAPABILITY_ON_HEAD_DETECTION_CURRENT_BIT  (3) /**< The bit of SASS capability on head detection current state, the MSB 3 of the 1st Byte. */
typedef uint8_t bt_fast_pair_sass_capability_bit_t;      /**< Define the flag of SASS capability. */

#define BT_FAST_PAIR_SASS_SWITCH_ACTIVE_AUDIO_SOURCE_SWITCH_TO_THIS_BIT     (7) /**< 1 switch to this device, 0 switch to second connected device. */
#define BT_FAST_PAIR_SASS_SWITCH_ACTIVE_AUDIO_SOURCE_RESUME_PLAYING_BIT     (6) /**< 1 resume playing on switch to device after switching, 0 otherwise. */
#define BT_FAST_PAIR_SASS_SWITCH_ACTIVE_AUDIO_SOURCE_REJECT_SCO             (5) /**< 1 reject SCO on switched away device, 0 otherwise. */
#define BT_FAST_PAIR_SASS_SWITCH_ACTIVE_AUDIO_SOURCE_DISCONNECT             (4) /**< 1 disconnect Bluetooth on switch away device, 0 otherwise. */
typedef uint8_t bt_fast_pair_switch_active_audio_source_bit_t;      /**< Define the flag of multi point switch active audio source. */

#define BT_FAST_PAIR_SASS_MULTIPOINT_SWITCH_A_A_BIT         (7) /**< The bit of SASS multipoint switching preference A2DP vs A2DP. */
#define BT_FAST_PAIR_SASS_MULTIPOINT_SWITCH_H_H_BIT         (6) /**< The bit of SASS multipoint switching preference HFP vs HFP. */
#define BT_FAST_PAIR_SASS_MULTIPOINT_SWITCH_A_H_BIT         (5) /**< The bit of SASS multipoint switching preference A2DP vs HFP. */
#define BT_FAST_PAIR_SASS_MULTIPOINT_SWITCH_H_A_BIT         (4) /**< The bit of SASS multipoint switching preference HFP vs A2DP. */
typedef uint8_t bt_fast_pair_sass_switch_preference_bit_t;          /**< Define the flag of multi point switch preference. */

#define BT_FAST_PAIR_SASS_SWITCH_BACK_EVENT_SWITCH_BACK     (0x01)  /**< The additional data of switch back message is switch back only. */
#define BT_FAST_PAIR_SASS_SWITCH_BACK_EVENT_RESUME_PLAYING  (0x02)  /**< The additional data of switch back message is switch back and resume playing. */

#define SPOT_COUNTER_SAVE_CYCLE_DEF         (4 * 60 * 60 * 1000)    /**< The eid counter save sycle. */

#define FAST_PAIR_SPP_MAXIMUM                       (0x03)      /**< The maximum SPP connection number. */

#define BT_FAST_PAIR_REVERSE_UINT16(X)          ((X) << 8 | ((X) >> 8))     /**< Change the UINT16 data from little-endian to big-endian. */

#define BT_FAST_PAIR_REVERSE_UINT32(X)      ((X) << 24 | ((X) << 8 & 0xFF0000) | ((X) >> 8 & 0xFF00) | (X) >> 24)  /**< Change the UINT32 data from little-endian to big-endian. */

#define BT_FAST_PAIR_REVERSE_UINT24(X)      (((X) << 16 & 0xFF0000) | ((X) & 0xFF00) | ((X) >> 16 & 0xFF))  /**< Change the UINT24(type is UINT32) data from little-endian to big-endian. */

/** @brief
 * This enum defines the connection state of the connection status data.
 */
enum {
    BT_FAST_PAIR_CONNECTION_STATE_NO_CONNECTION = 0x00,                         /**< Current provider is not connected state or connecting. */
    BT_FAST_PAIR_CONNECTION_STATE_PAGING,                                       /**< Current provider is actively connecting to seekers. */
    BT_FAST_PAIR_CONNECTION_STATE_CONNECTED,                                    /**< Current provider is connected with at least one seeker. */
    BT_FAST_PAIR_CONNECTION_STATE_NON_AUDIO_DATA_TRANSFERRING,                  /**< Current provider is connected and is transfering non audio data. */
    BT_FAST_PAIR_CONNECTION_STATE_A2DP_STREAMING,                               /**< Current provider is playing a2dp music but without avrcp profile. */
    BT_FAST_PAIR_CONNECTION_STATE_A2DP_STREAMING_WITH_AVRCP,                    /**< Current provider is playing a2dp music with avrcp profile. */
    BT_FAST_PAIR_CONNECTION_STATE_HFP,                                          /**< Current provider is in HFP calling. */
    BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_MEDIA_STREAMING,                     /**< Current provider is playing LE audio media. */
    BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_MEDIA_STREAMING_WITH_CONTROL,        /**< Current provider is playing LE audio media and can control it. */
    BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_CALL,                                /**< Current provider is in LE audio call status. */
    BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_BIS,                                 /**< Current provider is in LE audio BIS status. */
    BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_DISABLE_CONNECTION_SWITCH = 0x0F,    /**< Current provider is disabled connecting switch. */
};
typedef uint8_t bt_fast_pair_connection_state_t;            /**< Define the connection state. */

/** @brief
 * This enum defines the bit index in connection flags of the connection status data.
 */
enum {
    BT_FAST_PAIR_CONNECTION_FLAGS_AUTO_RECONNECTED = 4,         /**< the bit is 1, means the current connection is auto-reconnected by the provider. */
    BT_FAST_PAIR_CONNECTION_FLAGS_FOCUS_MODE = 5,               /**< the bit is 1, means the provider is in focused mode now, connection switching is not allowed for media usage. */
    BT_FAST_PAIR_CONNECTION_FLAGS_CONNECTION_AVALIABILITY = 6,  /**< the bit is 1, means there is an available connection. */
    BT_FAST_PAIR_CONNECTION_FLAGS_ON_HEAD_DETECTION = 7,        /**< the bit is 1, means the provider is on head now. */
};
typedef uint8_t bt_fast_pair_connection_flags_t;            /**< Define the connection flags. */

/** @brief
 * This enum defines the switch reason of notify multipoint-switch event.
 */
enum {
    BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_UNSPECIFIED,   /**< This reason is unspecified. */
    BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_A2DP,          /**< This reason is a2dp playing. */
    BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_HFP,           /**< This reason is hfp callng. */
};
typedef uint8_t bt_fast_pair_sass_multipoint_audio_src_switch_reason_t; /**< Define the audio source switch reason. */

/** @brief
 * This enum defines the target device flag of notify multipoint-switch event.
 */
enum {
    BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_TARGET_DEVICE_THIS_DEVICE,    /**< This device is active after switching. */
    BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_TARGET_DEVICE_OTHER_DEVICE    /**< This other connected device is active after switching. */
};
typedef uint8_t bt_fast_pair_sass_multipoint_audio_src_target_device_t; /**< Define audio source switch target device flag. */

/**
 * @}
 */

/**
 * @defgroup BluetoothServices_FAST_PAIR_struct Struct
 * @{
 * Defines fast pair structures for associated APIs and events.
 */

/**
 * @brief This structure defines the account keys list. Please refer to #bt_fast_pair_get_account_key_list().
 */
typedef struct {
    uint8_t max_key_number;                         /**< The number of account keys, The range is from 0x05 to 0x0C. */
    bt_fast_pair_account_key_t account_key_list[1]; /**< The account keys list. */
} bt_fast_pair_account_key_list_t;

/**
 * @brief This structure defines the associated parameter type in the callback for #BT_FAST_PAIR_APP_EVENT_PAIR_STARTED event.
 */
typedef struct {
    bt_handle_t conn_handle;                        /**< The BLE connection handle of fas pair. */
} bt_fast_pair_started_t;

/**
  * @brief This structure defines the associated parameter type in the callback for #BT_FAST_PAIR_APP_EVENT_PAIR_COMPLETE event.
 */
typedef struct {
    bt_handle_t             conn_handle;    /**< The BLE connection handle of fas pair. */
    bt_fast_pair_status_t   status;         /**< The fast pair complete status. */
} bt_fast_pair_complete_t;

/**
 * @brief This structure defines the initialization parameters of SASS feature.
 */
typedef struct {
    uint16_t        sass_version_code;  /**< The SASS version code. */
    uint16_t        sass_capability_flags;  /**< The SASS capability flags. */
    uint8_t         switching_preference_flags; /**< The SASS Switching preference flags. */
} bt_fast_pair_sass_inti_param_t;

/**
 * @brief This structure defines the initialization parameters of LE feature.
 */
typedef struct {
    bool            le_support;             /**< LE if support . */
    uint32_t        le_passkey;            /**< The pass key of LE. */
} bt_fast_pair_le_init_param_t;

/**
 * @brief This structure defines the initialization parameters. Please refer to #bt_fast_pair_init().
 */
typedef struct {
    bool            aws_device;         /**< If the device is an aws device. */
    uint32_t        model_id;           /**< The model id is distributed from google model registration. */
    uint8_t         *private_key;       /**< The private_key is distributed from google model registration. */
    int8_t          *tx_power_level;    /**< The tx power level is that received signal strength at source(0 meters). If you had registered with google model registration this could be NULL. */
    bt_addr_type_t  addr_type;          /**< The fast pair advertising own address type. */
    uint8_t         component_num;      /**< The product component number, this will be used for every components battery value notify by fast pair. */
    const char      *personalized_name; /**< The personalized name string, the end of the array should be \0, the string length not large than 128 Bytes. */
    const char      *firmware_revision; /**< The firmware revision string. The strlen of it must be smaller than BT_FAST_PAIR_FIRMWARE_REVISION_MAX_LENGTH. */
    bt_fast_pair_sass_inti_param_t sass_param;  /**< The parameters for the SASS feature. */
    bt_fast_pair_le_init_param_t   le_param;    /**< The parameters for the LE feature. */
} bt_fast_pair_init_parameters_t;

/**
 * @brief This structure defines the fast pair update parameters. Please refer to #bt_fast_pair_update_parameters().
 */
typedef struct {
    int8_t          *tx_power_level;    /**< The tx power level is that received signal strength at source(0 meters). If you had registered with google model registration this could be NULL. */
    bt_addr_type_t  addr_type;          /**< The fast pair advertising own address type. */
    const char      *personalized_name; /**< The personalized name string, the end of the array should be \0, the string length not large than 128 Bytes. */
    bool            le_support;         /**< The configuration of LE Audio is supported. */
} bt_fast_pair_update_parameters_t;

/**
  * @brief This structure defines the message stream format used in #BT_FAST_PAIR_APP_EVENT_MESSAGE_STREAM event and #bt_fast_pair_send_message_stream().
  * This structure is defined from google fast pair spec, please refer to https://developers.google.cn/nearby/fast-pair/spec for more detail information.
 */

BT_PACKED(
typedef struct {
    bt_fast_pair_message_group_t    group_ID;   /**< The Fast pair message group ID. */
    uint8_t     code_ID;                        /**< The Fast pair message code ID. */
    uint16_t    data_length;                    /**< The Fast pair message additional data length. */
    uint8_t     data[1];                        /**< The Fast pair message additional data. */
}) bt_fast_pair_message_stream_t;

/**
  * @brief This structure used to record every component's battery in #bt_fast_pair_battery_t.
 */
typedef struct {
    uint8_t     charging;               /**< The component charging status. */
    uint8_t     battery_value;          /**< The component battery value, 0xFF means invalid or unkown battery value. */
} bt_fast_pair_battery_item_t;

/**
  * @brief This structure used to notify battery value to remote device by API #bt_fast_pair_update_battery;
 */
typedef struct {
    bool               ui_show;         /**< The ui show. */
    uint16_t           remaining_time;  /**< The remaining time, 0xFFFF means invalid or unkown remaining time. */
    uint8_t            component_num;   /**< The component num of battery notify. */
    bt_fast_pair_battery_item_t battery[BT_FAST_PAIR_COMPONENT_MAXIMUM];    /**< The every component's battery. */
} bt_fast_pair_battery_t;

/**
  * @brief This structure defines the ring request from remote device in the event callback for #BT_FAST_PAIR_APP_EVENT_RING_REQUEST event.
 */
typedef struct {
    bool    ring_status[BT_FAST_PAIR_COMPONENT_MAXIMUM];    /**< The ring request of every componet. */
    uint8_t ring_duration;                                  /**< The ring requested duration. */
} bt_fast_pair_ring_request_t;

/**
  * @brief This structure used to acknowledge action from remote device by API #bt_fast_pair_send_acknowledgements();
 */
typedef struct {
    bt_fast_pair_message_code_acknowledgement_t action;     /**< The acknowledge action ID. */
    bt_fast_pair_message_group_t                groupID;    /**< The message group ID of acknowledgements. */
    uint8_t                                     codeID;     /**< The message code ID of acknowledgements. */
    bt_fast_pair_acknowledgement_nak_reason_t    reason; /**< The nak reason. It's only valid when action ID setted to #BT_FAST_PAIR_MESSAGE_CODE_ACKNOWLEDGEMENT_NAK. */
} bt_fast_pair_acknowledgements_t;

/**
  * @brief This structure defines the fast pair additional data format for #BT_FAST_PAIR_APP_EVENT_ADDITIONAL_DATA event.
 */
BT_PACKED(
typedef struct {
    bt_fast_pair_additional_data_id_t   data_id;            /**< additional data ID. */
    uint8_t                             data_length;        /**< additional data length. */
    uint8_t                             data[1];            /**< additional data. */
}) bt_fast_pair_additional_data_t;

/**
  * @brief This structure defines additional data format of the notify session nonce message.
 */
typedef struct {
    uint8_t session_nonce[BT_FAST_PAIR_MESSAGE_NONCE_LENGTH];   /* The session nonce. */
} PACKED bt_fast_pair_notify_session_nonce_t;

/**
  * @brief This structure defines the received data format of notify capability.
 */
typedef struct {
    uint16_t sass_version_code;     /**< SASS version code, default is 0x0101. */
    uint16_t sass_capability_flag;  /**< SASS capability flags, refer to #bt_fast_pair_sass_capability_bit_t. */
} PACKED bt_fast_pair_sass_notify_capability_data_t;

/**
  * @brief This structure defines the received data format of setting multipoint state from seeker.
 */
typedef struct {
    uint8_t switch_on;     /**< The switching preference flags. */
} PACKED bt_fast_pair_sass_set_multipoint_state_data_t;

/**
  * @brief This structure defines the received data format of setting switching preference from seeker.
 */
typedef struct {
    uint8_t switching_preference_flags;     /**< The switching preference flags. */
    uint8_t advanced_switching_settings;    /**< The advanced switching settings. */
} PACKED bt_fast_pair_sass_set_switching_preference_data_t;

/**
  * @brief This structure defines the data format of the notify switching preference.
 */
typedef struct {
    uint8_t switching_preference_flags;     /**< The switching preference flags. */
    uint8_t advanced_switching_settings;    /**< The advanced switching settings. */
} PACKED bt_fast_pair_sass_notify_switching_preference_data_t;

/**
  * @brief This structure defines the data format of the notify multipoint-switch event.
 */
typedef struct {
    bt_fast_pair_sass_multipoint_audio_src_switch_reason_t switching_reason;    /**< The switch reason. */
    bt_fast_pair_sass_multipoint_audio_src_target_device_t target_device;       /**< The flag to indicate which seeker is active after switching. */
    char target_device_name[0];                                                 /**< The name of the seeker active after switching. */
} PACKED bt_fast_pair_sass_notify_multipoint_switch_event_data_t;

/**
  * @brief This structure defines the data format of the notify connection status message.
 */
typedef struct {
    uint8_t active_device_flag;                                 /**< 0x00: this Seeker is not the active device; 0x01: means this Seeker is the active device. */
    uint8_t encrypted_connection_status[0];                     /**< The encrypted data of connection status, the length is variable. */
    uint8_t message_nonce[BT_FAST_PAIR_MESSAGE_NONCE_LENGTH];   /**< The message nonce which is generated by random. */
} PACKED bt_fast_pair_sass_notify_connection_status_data_t;

/**
  * @brief This structure defines the data format of RHO old agent send to new agent.
 */
typedef struct {
    uint8_t session_nonce[BT_FAST_PAIR_MESSAGE_NONCE_LENGTH];   /**< The session of the seeker. */
    uint8_t account_index;                                      /**< The index of the account key in account key list of the seeker. */
    uint8_t bitmap_index;                                       /**< The position of the seeker in the bitmap. */
} PACKED bt_fast_pair_sass_edr_addr_sync_data_t;

/**
  * @brief This structure defines one item of address to session nonce, account key id and bitmap index.
 */
typedef struct {
    bt_bd_addr_t addr;                                          /**< The address of the connected seeker. */
    uint8_t session_nonce[BT_FAST_PAIR_MESSAGE_NONCE_LENGTH];   /**< The session of the seeker. */
    uint8_t account_index;                                      /**< The index of the account key in account key list of the seeker. */
    uint8_t bitmap_index;                                       /**< The position of the seeker in the bitmap. */
    uint8_t flag_playing : 1;                                   /**< One bit of flag means the device is playing. */
} PACKED bt_fast_pair_edr_addr_account_map_t;

/**
  * @brief This structure defines the data need store in NVDM in SASS.
 */
typedef struct {
    uint8_t inused_account_key_id;              /**< The index of in use or recently in used account key in account key list. */
    uint8_t connected_record_length;            /**< The count of the connected record devices. */
    bt_bd_addr_t *connected_record_array;       /**< The record of ever connected address list. */
} bt_fast_pair_sass_recorded_data_t;

/**
  * @brief This structure defines the data not need store in NVDM in SASS.
 */
typedef struct {
    uint8_t custom_data;                                                            /**< The custom data. */
    bt_fast_pair_edr_addr_account_map_t addr_account_map[FAST_PAIR_SPP_MAXIMUM];    /**< The address mapping to its session nonce, account key, bitmap index. */
} bt_fast_pair_sass_temp_data_t;

/**
  * @brief This structure defines the format of SASS MAC verify data.
 */
typedef struct {
    uint8_t nonce[BT_FAST_PAIR_MESSAGE_NONCE_LENGTH];       /**< The message nonce. */
    uint8_t mac[BT_FAST_PAIR_MESSAGE_AUTH_CODE_LENGTH];     /**< The MAC data. */
} PACKED bt_fast_pair_sass_mac_verify_data_t;

/**
 * @}
 */

/**
 * @brief gatt service database of fast pair module.
 */
extern const bt_gatts_service_t bt_fast_pair_service;

/**
 * @brief   This function is a user-defined callback for the application to listen to the events from fast pair.
 * @param[in] evt     is the callback event type.
 * @param[in] addr    is the address of the remote device trigger the event, NULL if the event is from internal.
 * @param[in] buf     is the payload of the callback event.
 * @param[in] buf_len is the length of the buf.
 * @return            The status of this operation returned from the callback.
 */
bt_fast_pair_status_t   bt_fast_pair_app_event_callback(bt_fast_pair_app_event_t evt, bt_bd_addr_t *addr, void *buf, uint32_t buf_len);

/**
 * @brief     This is a user-defined function that returns the account key list buffer to fast pair module.
 * @return    A pointer to the account key buffer containing the account key buffer numbers and account key buffer. The pointer should not be NULL and should point to a global variable.
 */
bt_fast_pair_account_key_list_t *bt_fast_pair_get_account_key_list();

/**
 * @brief     This function is to get the advertising data according to the data type.
 * @param[in,out] buffer   the buffer that filled to advertising data.
 * @param[in,out] length   the buffer length, return the actual data length. The input length should be more than 7 bytes.
 * @param[in] data_type   is the advertising data type. Please refer to #bt_fast_pair_advertising_data_t.
 * @return    #BT_FAST_PAIR_STATUS_SUCCESS, the advertising data filled in buffer successfully.
 *            #BT_FAST_PAIR_STATUS_INVALID_LENGTH, the buffer length not enough.
 *            #BT_FAST_PAIR_STATUS_INVALID_KEY_BUFFER, the buffer is invalid.
 *            #BT_FAST_PAIR_STATUS_INVALID_PARAMETERS, the data_type is invalid.
 */
bt_fast_pair_status_t   bt_fast_pair_get_advertising_data(uint8_t *buffer, uint8_t *length, bt_fast_pair_advertising_data_t data_type);

/**
 * @brief     This function initializes the fast pair module with initialization parameters.
 * @param[in] init_param is the initialization parameters. Please refer to #bt_fast_pair_init_parameters_t.
 * @return    #BT_FAST_PAIR_STATUS_SUCCESS, the initialization is successful.
 *            #BT_FAST_PAIR_STATUS_INVALID_PARAMETERS, the init_param is invalid.
 *            #BT_FAST_PAIR_STATUS_INVALID_PRIVATE_KEY, the private key is invalid.
 */
bt_fast_pair_status_t   bt_fast_pair_init(bt_fast_pair_init_parameters_t *init_param);

/**
 * @brief     This function send a message stream to remote device.
 * @param[in] addr the remote device's bluetooth address, set to NULL means send message stream to all fast pair connected device.
 * @param[in] message the message send to remote device , should not be null.
 * @return    #BT_FAST_PAIR_STATUS_SUCCESS, the send action success.
 *            #BT_FAST_PAIR_STATUS_NOT_CONNECTED, the fast pair service not connected.
 *            #BT_FAST_PAIR_STATUS_INVALID_PARAMETERS, the parameters is invalid.
 *            #BT_FAST_PAIR_STATUS_FAIL, the general fail.
 */
bt_fast_pair_status_t   bt_fast_pair_send_message_stream(bt_bd_addr_t *addr, bt_fast_pair_message_stream_t *message);

/**
 * @brief     This function update every component's battery value to fast pair moudle. Fast pair will send it to remote fast pair connected device,
               And fast pair will also broadcast it to around device.
 * @param[in] battery the battery value set, please refer to #bt_fast_pair_battery_t.
 * @return    #BT_FAST_PAIR_STATUS_SUCCESS, the action success.
 *            #BT_FAST_PAIR_STATUS_INVALID_PARAMETERS, the parameters is invalid.
 *            #BT_FAST_PAIR_STATUS_FAIL, the general fail.
 */
bt_fast_pair_status_t   bt_fast_pair_update_battery(bt_fast_pair_battery_t *battery);

/**
 * @brief     This function send a silence request to remote fast pair connected device.
 * @param[in] addr the remote device's bluetooth address, set to NULL means send request to all fast pair connected device.
 * @param[in] enable the disable or enable the silence mode.
 * @return    #BT_FAST_PAIR_STATUS_SUCCESS, the send action success.
 *            #BT_FAST_PAIR_STATUS_NOT_CONNECTED, the fast pair service not connected.
 *            #BT_FAST_PAIR_STATUS_INVALID_PARAMETERS, the parameters is invalid.
 *            #BT_FAST_PAIR_STATUS_FAIL, the general fail.
 */
bt_fast_pair_status_t   bt_fast_pair_set_silence_mode(bt_bd_addr_t *addr, bool enable);

/**
 * @brief     This function send a acknowledgement to remote fast pair connected device.
 * @param[in] addr the remote device's bluetooth address, set to NULL means send to all fast pair connected device.
 * @param[in] ack the ack parameters.
 * @param[in] current_state the array to indicate the current state, it's only useful when it's an ACK not NAK, normally it is the same as received additional data.
 * @param[in] state_length the length of current_state
 * @return    #BT_FAST_PAIR_STATUS_SUCCESS, the send action success.
 *            #BT_FAST_PAIR_STATUS_NOT_CONNECTED, the fast pair service not connected.
 *            #BT_FAST_PAIR_STATUS_INVALID_PARAMETERS, the parameters is invalid.
 *            #BT_FAST_PAIR_STATUS_FAIL, the general fail.
 */
bt_fast_pair_status_t   bt_fast_pair_send_acknowledgements(bt_bd_addr_t *addr, bt_fast_pair_acknowledgements_t *ack, uint8_t *current_state, uint16_t state_length);

/**
 * @brief     This function update the fast pair parameters.
 * @param[in] param is the update parameters. Please refer to #bt_fast_pair_update_parameters_t.
 * @return    #BT_FAST_PAIR_STATUS_SUCCESS, the initialization is successful.
 *            #BT_FAST_PAIR_STATUS_INVALID_PARAMETERS, the init_param is invalid.
 *            #BT_FAST_PAIR_STATUS_INVALID_PRIVATE_KEY, the private key is invalid.
 */
bt_fast_pair_status_t   bt_fast_pair_update_parameters(bt_fast_pair_update_parameters_t *param);

/**
 * @brief     This function used to terminate a fast pair service indicated by address.
 * @param[in] addr the remote device's bluetooth address, set to NULL means terminate all fast pair service.
 * @return    #BT_FAST_PAIR_STATUS_SUCCESS, the send action success.
 *            #BT_FAST_PAIR_STATUS_NOT_CONNECTED, the fast pair service not connected.
 *            #BT_FAST_PAIR_STATUS_INVALID_PARAMETERS, the parameters is invalid.
 *            #BT_FAST_PAIR_STATUS_FAIL, the general fail.
 */
bt_fast_pair_status_t   bt_fast_pair_terminate(bt_bd_addr_t *addr);

/**
 * @brief     This function used to set the random ble address, call it when system on or received BT_GAP_LE_RPA_ROTAION_IND.
 * @param[in] need_update is true means need send stream immediately.
 * @param[in] random_addr is the random_addr.
 */
void bt_fast_pair_set_random_ble_address(bool need_update, bt_bd_addr_t *random_addr);

/**
 * @brief     This function used to start ring or get ring capable or get ring status.
 * @param[in] req_type BT_FAST_PAIR_SPOT_RING_REQ_RING or BT_FAST_PAIR_SPOT_RING_REQ_GET_RING_CAPABLE or BT_FAST_PAIR_SPOT_RING_REQ_GET_RING_STATE.
 * @param[in] data if req_type is BT_FAST_PAIR_SPOT_RING_REQ_RING, *data means the current ring type.
 *            if req_type is BT_FAST_PAIR_SPOT_RING_REQ_GET_RING_CAPABLE, should assign the current ring capable to *data
 *            if req_type is BT_FAST_PAIR_SPOT_RING_REQ_GET_RING_TYPE, should assign the current ring type to *data
 * @param[in] remain_time assign the remain time of ring to *remain_time, if not in ringing state, the value should be 0x0. The unit is deciseconds.
 * @return    #BT_FAST_PAIR_STATUS_SUCCESS, the send action success.
 *            #BT_FAST_PAIR_STATUS_NOT_CONNECTED, the fast pair service not connected.
 *            #BT_FAST_PAIR_STATUS_INVALID_PARAMETERS, the parameters is invalid.
 *            #BT_FAST_PAIR_STATUS_FAIL, the general fail.
 */
bt_fast_pair_status_t bt_fast_pair_spot_ring_callback(bt_fast_pair_spot_ring_req_type_t req_type, uint8_t *data, uint16_t *remain_time);

/**
 * @brief     This function is used to get the time clock which used to gen EID data. (APP layer must be implement)
 * @return    The clock value (seconds)
 */
uint32_t bt_fast_pair_spot_get_eid_clock_callback(void);

/**
 * @brief     This function is used to get the spot adv data, including Eddystone-E2EE-EID frame header and 20bytes EID.
 * @param[in] re_calculate re_calculate EID to adv data, it will block current task 1~3 seconds.
 * @param[in] buf the buf used to storage the adv data.
 * @param[in] len the buffer length and output spot_adv_data length.
 * @return    TRUE, get spot_adv_data successfully
 *            FALSE, get fail - not exist EIK/EID, buf is NULL, buf length less than 28.
 */
bool bt_fast_pair_get_spot_adv_data(bool re_calculate, uint8_t *buf, uint16_t *len);

/**
 * @brief     This function is used to trigger the spot adv
 *            Notice that the bt_fast_pair_spot_eid_advertising_callback will be called while this function called.
 * @param[in] enable enable or disable the advertising, true means it is enabled.
 * @return    The clock value (seconds)
 */
void bt_fast_pair_spot_set_advertising(bool enable);

/**
 * @brief     This function is used to set adv callback function by APP layer
 *            Must call it before bt_fast_pair_init.
 * @param[in] cb bt_fast_pair_spot_adv_cb_t.
 */
void bt_fast_pair_spot_set_adv_callback(bt_fast_pair_spot_adv_cb_t cb);

/**
 * @brief     This function is used to update ring statue to SPOT client, must be called every time ring starts or stops by any reason.
 * @param[in] sta current ring state.
 * @param[in] ring_type current ring type.
 * @param[in] remain_time the remain time of ringing, if currentily not in ringing, this value must be 0.
 * @return    #BT_FAST_PAIR_STATUS_SUCCESS the send action is successfull.
 *            #BT_FAST_PAIR_STATUS_NOT_CONNECTED the fast pair service is not connected.
 *            #BT_FAST_PAIR_STATUS_INVALID_PARAMETERS the parameters are invalid.
 *            #BT_FAST_PAIR_STATUS_FAIL the general fail.
 */
bt_fast_pair_status_t bt_fast_pair_spot_ring_state_update(bt_fast_pair_spot_ring_state_t sta, bt_fast_pair_spot_ring_type_t ring_type, uint16_t remain_time);

/**
 * @brief     This function used to update the pairing mode state to SPOT.
 * @param[in] in_pairing_mode indicate that whether the device in pairing mode.
 * @return    None
 */
void bt_fast_pair_spot_update_pairing_mode_state(bool in_pairing_mode);

/**
 * @brief     This function is notify capability changed.
 * @param[in] capability the capability flags.
 * @param[in] need_notify true means need use fast pair message to notify all of connected seekers.
 */
void bt_fast_pair_sass_update_capability(uint16_t capability, bool need_notify);

/**
 * @brief     This function is notify the connect state is changed.
 * @param[in] preference the configuration of switch preference for notifying.
 * @param[in] need_notify true means need use fast pair message to notify all of connected seekers.
 */
void bt_fast_pair_sass_update_multipoint_switching_preference(bt_fast_pair_sass_notify_switching_preference_data_t *preference, bool need_notify);

/**
 * @brief     This function is notify the connect state is changed.
 * @param[in] need_notify true means need use fast pair message to notify all of connected seekers.
 */
void bt_fast_pair_sass_update_connection_state(bool need_notify);

/**
 * @brief     This function is update the address of active device.
 * @param[in] active_addr the point to the address of the active device.
 * @param[in] reason the reason of switching.
 */
void bt_fast_pair_sass_update_multipoint_audio_source_switch(bt_bd_addr_t *active_addr, bt_fast_pair_sass_multipoint_audio_src_switch_reason_t reason);

/**
 * @brief     This function is the weak symbol that must be implemented by the project.
 * @param[out] addr the implementation need fill the address of the active device.
 * @return    True means get active device successfully.
 */
bool bt_fast_pair_sass_get_active_device_address(bt_bd_addr_t *addr);

/**
 * @brief     This function is the weak symbol that must be implemented by the project. It should return the connection state.
 * @return    Current connection state value. The value refer to bt_fast_pair_connection_state_t.
 */
uint8_t bt_fast_pair_sass_get_connection_state(void);

/**
 * @brief     This function is the weak symbol that must be implemented by the project.
 * @param[in] local_or_peer if yes, get the local device of device.
 * @param[out] type the implementation that must fill the address type of the LE device.
 * @param[out] addr the implementation that must fill the address of the LE device.
 * @return    True means get LE device successfully.
 */
bool bt_fast_pair_get_le_addr(bool local_or_peer, uint8_t *type, uint8_t *addr);

/**
 * @brief     This function is the weak symbol that must be implemented by the project.
 * @param[in] is_display_yesno If display yes or no.
 * @return    True means it set the LE IO capability successfully.
 */
bool bt_fast_pair_set_le_io_capability(bool is_display_yesno);

/**
 * @brief     This function is get the index of the account key in the account key list.
 * @param[in] addr the pointer to the BT address.
 * @return    The index of the account key in the account key list. If the address is not in the list, it returns 0xFF.
 */
uint8_t bt_fast_pair_sass_get_account_id_by_addr(const bt_bd_addr_t *addr);

/**
 * @brief     This function is get the internal data, contains temp data and recoreded data.
 * @param[out] p_temp_data the pointer to the temporary data, the temporary data means this data which will be released when power off.
 * @param[out] p_recorded_data the pointer to the recoreded data, the recoreded data means this data which will be stored in NVDM when power off.
 */
void bt_fast_pair_sass_get_internal_data(const bt_fast_pair_sass_temp_data_t **p_temp_data, const bt_fast_pair_sass_recorded_data_t **p_recorded_data);

/**
 * @brief     This function is set the custom data. Only be called in fast pair middleware. Only be called in fast pair middleware.
 * @param[in] custom_data the custom data which is send from seeker.
 * @return    Currently always return true.
 */
bool bt_fast_pair_sass_set_internal_custom_data(uint8_t custom_data);

/**
 * @brief     This function is set one mapping item of address - bt_fast_pair_edr_addr_account_map_t data. Only be called in fast pair middleware.
 * @param[in] addr the pointer to the BT address.
 * @param[in] map the pointer to the information map item.
 * @return    When parameter is not correct or mapping table is full, it returns false.
 */
bool bt_fast_pair_sass_set_internal_addr_account_map(const bt_bd_addr_t *addr, bt_fast_pair_sass_edr_addr_sync_data_t *map);

/**
 * @brief     This function is clear the temporary data, the data format is bt_fast_pair_sass_temp_data_t. Only be called in fast pair middleware.
 * @return    Currently always return true.
 */
bool bt_fast_pair_sass_set_internal_clear_temp_data(void);

/**
 * @brief     This function is set the in use account key.
 * @param[in] account_key_id the index of account key in accout key id list.
 * @return    Currently always return true.
 */
bool bt_fast_pair_sass_set_in_used_account_key_id(uint8_t account_key_id);

/**
 * @brief     This function is set the connected devices address record.
 * @param[in] array_length the length of address array.
 * @param[in] addr_array the address array.
 * @return    When the parameter is not correct or the status have some fatal error, it return false.
 */
bool bt_fast_pair_sass_set_connected_record(uint8_t array_length, bt_bd_addr_t *addr_array);

/**
 * @brief     This function is called when AWS is connected in an earbuds project.
 * @param[in] connected true means connected, false means disconnected.
 */
void bt_fast_pair_sass_on_aws_state_change(bool connected);

BT_EXTERN_C_END

/**
 * @}
 * @}
 */

#endif /*__BT_FAST_PAIR_H__*/

