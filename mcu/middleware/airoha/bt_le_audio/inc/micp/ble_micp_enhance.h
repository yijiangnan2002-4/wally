/*
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

#ifndef __BLE_MICP_ENHANCE_H__
#define __BLE_MICP_ENHANCE_H__

/**
 * @addtogroup Bluetooth
 * @{
 * @addtogroup BluetoothMICP MICP
 * @{
 * This section introduces the MICP operation codes, request and response structures, API prototypes.
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                  |
 * |------------------------------|-------------------------------------------------------------------------|
 * |\b MICP                        | Microphone Control Profile. This profile enables a client to control and obtain the status of a microphone through the MICS. |
 * |\b MICS                        | Microphone Control Service. This service enables a device to expose the mute control and state of one or more microphones. |
 * |\b AICS                       | Audio Input Control Service. This service enables a device to expose the control and state of an audio input. |
 *
 */

#include "bt_type.h"
#include "ble_mics_def.h"
#include "ble_aics_def.h"

/**
 * @defgroup Bluetoothbt_MICP_define Struct
 * @{
 * This section defines basic data structures for the MICP.
 */
#define BLE_MICP_ENHANCE_IDLE                                                        0x00
#define BLE_MICP_ENHANCE_MICS_READ_MUTE_CFM                                          0x01
#define BLE_MICP_ENHANCE_MICS_WRITE_MUTE_CFM                                         0x02
#define BLE_MICP_ENHANCE_MICS_WRITE_MUTE_NOTIFICATION_CFM                            0x03
#define BLE_MICP_ENHANCE_MICS_MUTE_NOTIFY                                            0x04
#define BLE_MICP_ENHANCE_AICS_READ_AUDIO_INPUT_STATE_CFM                             0x05
#define BLE_MICP_ENHANCE_AICS_READ_GAIN_SETTING_PROPERTIES_CFM                       0x06
#define BLE_MICP_ENHANCE_AICS_READ_AUDIO_INPUT_TYPE_CFM                              0x07
#define BLE_MICP_ENHANCE_AICS_READ_AUDIO_INPUT_STATUS_CFM                            0x08
#define BLE_MICP_ENHANCE_AICS_READ_AUDIO_INPUT_DESCRIPTION_CFM                       0x09
#define BLE_MICP_ENHANCE_AICS_WRITE_AUDIO_INPUT_CONTROL_POINT_CFM                    0x0A
#define BLE_MICP_ENHANCE_AICS_WRITE_AUDIO_INPUT_STATE_NOTIFICATION_CFM               0x0B
#define BLE_MICP_ENHANCE_AICS_WRITE_AUDIO_INPUT_STATUS_NOTIFICATION_CFM              0x0C
#define BLE_MICP_ENHANCE_AICS_WRITE_AUDIO_INPUT_DESCRIPTION_NOTIFICATION_CFM         0x0D
#define BLE_MICP_ENHANCE_AICS_AUDIO_INPUT_STATE_NOTIFY                               0x0E
#define BLE_MICP_ENHANCE_AICS_AUDIO_INPUT_STATUS_NOTIFY                              0x0F
#define BLE_MICP_ENHANCE_AICS_AUDIO_INPUT_DESCRIPTION_NOTIFY                         0x10
#define BLE_MICP_ENHANCE_DISCOVER_SERVICE_COMPLETE_NOTIFY                            0x11
typedef uint8_t ble_micp_enhance_event_t;      /**< The type of MICP_ENHANCE events.*/


typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    bt_status_t status;                     /**< Event status. */
    uint8_t number_of_aics;                 /**< The number of AICS instances. */
} ble_micp_enhance_discover_service_complete_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    ble_mics_mute_state_t mute;                    /**< The mute state. */
} ble_micp_enhance_read_mute_cfm_t;

typedef ble_micp_enhance_read_mute_cfm_t ble_micp_enhance_mute_notify_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    int8_t gain_setting;                    /**< The gain setting, Gain = Gain_Setting * Gain_Setting_Units. */
    ble_aics_mute_state_t mute;             /**< The mute state. */
    ble_aics_gain_mode_t gain_mode;         /**< The gain mode. */
    uint8_t change_counter;                 /**< The change counter value, range: 0~255. */
} ble_micp_enhance_read_audio_input_state_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    uint8_t gain_setting_units;             /**< The gain getting value in 0.1 decibel units. */
    int8_t gain_setting_min;                /**< The minimum allowable value of the Gain_Setting field. */
    int8_t gain_setting_max;                /**< The maximum allowable value of the Gain_Setting field. */
} ble_micp_enhance_read_gain_setting_properties_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    ble_aics_audio_input_type_t audio_input_type;/**< The audio input type. */
} ble_micp_enhance_read_audio_input_type_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    ble_aics_audio_input_status_t audio_input_status;/**< The audio input status. */
} ble_micp_enhance_read_audio_input_status_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    uint8_t description_length;             /**< The length of audio input description. */
    uint8_t* audio_input_description;       /**< The audio input description string. */
} ble_micp_enhance_read_audio_input_description_cfm_t;

typedef ble_micp_enhance_read_audio_input_state_cfm_t ble_micp_enhance_audio_input_state_notify_t;
typedef ble_micp_enhance_read_audio_input_status_cfm_t ble_micp_enhance_audio_input_status_notify_t;
typedef ble_micp_enhance_read_audio_input_description_cfm_t ble_micp_enhance_audio_input_description_notify_t;

typedef struct {
    ble_aics_control_opcode_t opcode;       /**< The AICS control point opcode. */
    uint8_t change_counter;                 /**< The change counter value, range: 0~255. */
    int8_t gain_setting;                    /**< The gain setting, Gain = Gain_Setting * Gain_Setting_Units. */
} ble_micp_enhance_audio_input_control_param_t;

typedef struct {
    bt_status_t status;                     /**< The result of write request. */
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
} ble_micp_enhance_write_cfm_t;

/**
 * @brief This structure defines the event callback for MICP.
 * @param[in] event             is the event id.
 * @param[in] msg               is the event message.
 */
typedef void (*ble_micp_enhance_callback_t)(ble_micp_enhance_event_t event, void *msg);

/**
 * @}
 */

/**
 * @brief                   This function initializes the MICP.
 * @param[in] callback      is the callback function of receiving the MICP events.
 * @param[in] max_link_num  is the maximum number of link.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_init(ble_micp_enhance_callback_t callback, uint8_t max_link_num);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of volume state.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_mics_set_mute_notification_req(bt_handle_t handle, bool enable);

/**
 * @brief                   This function is used to read the value of volume state.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_mics_read_mute(bt_handle_t handle);

/**
 * @brief                   This function is used to write the mute state of the MICS.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] mute          is the mute state to be written.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_mics_write_mute(bt_handle_t handle, ble_mics_mute_state_t mute);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of audio input state.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_aics_set_audio_input_state_notification_req(bt_handle_t handle, uint8_t service_index, bool enable);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of audio input status.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_aics_set_audio_input_status_notification_req(bt_handle_t handle, uint8_t service_index, bool enable);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of audio input description.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_aics_set_audio_input_description_notification_req(bt_handle_t handle, uint8_t service_index, bool enable);

/**
 * @brief                   This function is used to read the value of audio input state.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_aics_read_audio_input_state(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to read the value of gain setting properties.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_aics_read_gain_setting_properties(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to read the value of audio input type.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_aics_read_audio_input_type(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to read the value of audio input status.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_aics_read_audio_input_status(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to read the value of audio input description.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_aics_read_audio_input_description(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to start an audio input control procedure.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] param         is the parameters of the audio input control procedure.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_aics_write_audio_input_control_point(bt_handle_t handle, uint8_t service_index, ble_micp_enhance_audio_input_control_param_t *param);

/**
 * @brief                   This function is used to write the value of audio input description.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] description   is the audio input description string.
 * @param[in] length        is the length of the audio input description string.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_micp_enhance_aics_write_audio_input_description(bt_handle_t handle, uint8_t service_index, uint8_t *description, uint8_t length);

/**
 * @}
 * @}
 */

#endif  /* __BLE_MICP_ENHANCE_H__ */


