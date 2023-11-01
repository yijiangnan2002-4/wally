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

#ifndef __BLE_VCP_ENHANCE_H__
#define __BLE_VCP_ENHANCE_H__

/**
 * @addtogroup Bluetooth
 * @{
 * @addtogroup BluetoothVCP VCP
 * @{
 * This section introduces the VCP operation codes, request and response structures, API prototypes.
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                  |
 * |------------------------------|-------------------------------------------------------------------------|
 * |\b VCP                        | Volume Control Profile. This profile enables a device to adjust the volume of audio devices that expose the Volume Control Service. |
 * |\b VCS                        | Volume Control Service. This service enables a device to expose the controls and state of its audio volume. |
 * |\b VOCS                       | Volume Offset Control Service. This service enables a device to expose the volume offset of an audio output. |
 * |\b AICS                       | Audio Input Control Service. This service enables a device to expose the control and state of an audio input. |
 *
 */

#include "bt_type.h"
#include "ble_vcs_def.h"
#include "ble_vocs_def.h"
#include "ble_aics_def.h"

/**
 * @defgroup Bluetoothbt_VCP_define Struct
 * @{
 * This section defines basic data structures for the VCP.
 */
#define BLE_VCP_ENHANCE_IDLE                                                        0x00
#define BLE_VCP_ENHANCE_VCS_READ_VOLUME_STATE_CFM                                   0x01
#define BLE_VCP_ENHANCE_VCS_READ_VOLUME_FLAGS_CFM                                   0x02
#define BLE_VCP_ENHANCE_VCS_WRITE_VOLUME_CONTROL_POINT_CFM                          0x03
#define BLE_VCP_ENHANCE_VCS_WRITE_VOLUME_STATE_NOTIFICATION_CFM                     0x04
#define BLE_VCP_ENHANCE_VCS_WRITE_VOLUME_FLAGS_NOTIFICATION_CFM                     0x05
#define BLE_VCP_ENHANCE_VCS_VOLUME_STATE_NOTIFY                                     0x06
#define BLE_VCP_ENHANCE_VCS_VOLUME_FLAGS_NOTIFY                                     0x07
#define BLE_VCP_ENHANCE_VOCS_READ_VOLUME_OFFSET_STATE_CFM                           0x08
#define BLE_VCP_ENHANCE_VOCS_READ_AUDIO_LOCATION_CFM                                0x09
#define BLE_VCP_ENHANCE_VOCS_READ_AUDIO_OUTPUT_DESCRIPTION_CFM                      0x0A
#define BLE_VCP_ENHANCE_VOCS_WRITE_VOLUME_OFFSET_CONTROL_POINT_CFM                  0x0B
#define BLE_VCP_ENHANCE_VOCS_WRITE_OFFSET_STATE_NOTIFICATION_CFM                    0x0C
#define BLE_VCP_ENHANCE_VOCS_WRITE_AUDIO_LOCATION_NOTIFICATION_CFM                  0x0D
#define BLE_VCP_ENHANCE_VOCS_WRITE_AUDIO_OUTPUT_DESCRIPTION_NOTIFICATION_CFM        0x0E
#define BLE_VCP_ENHANCE_VOCS_OFFSET_STATE_NOTIFY                                    0x0F
#define BLE_VCP_ENHANCE_VOCS_AUDIO_LOCATION_NOTIFY                                  0x10
#define BLE_VCP_ENHANCE_VOCS_AUDIO_OUTPUT_DESCRIPTION_NOTIFY                        0x11
#define BLE_VCP_ENHANCE_AICS_READ_AUDIO_INPUT_STATE_CFM                             0x12
#define BLE_VCP_ENHANCE_AICS_READ_GAIN_SETTING_PROPERTIES_CFM                       0x13
#define BLE_VCP_ENHANCE_AICS_READ_AUDIO_INPUT_TYPE_CFM                              0x14
#define BLE_VCP_ENHANCE_AICS_READ_AUDIO_INPUT_STATUS_CFM                            0x15
#define BLE_VCP_ENHANCE_AICS_READ_AUDIO_INPUT_DESCRIPTION_CFM                       0x16
#define BLE_VCP_ENHANCE_AICS_WRITE_AUDIO_INPUT_CONTROL_POINT_CFM                    0x17
#define BLE_VCP_ENHANCE_AICS_WRITE_AUDIO_INPUT_STATE_NOTIFICATION_CFM               0x18
#define BLE_VCP_ENHANCE_AICS_WRITE_AUDIO_INPUT_STATUS_NOTIFICATION_CFM              0x19
#define BLE_VCP_ENHANCE_AICS_WRITE_AUDIO_INPUT_DESCRIPTION_NOTIFICATION_CFM         0x1A
#define BLE_VCP_ENHANCE_AICS_AUDIO_INPUT_STATE_NOTIFY                               0x1B
#define BLE_VCP_ENHANCE_AICS_AUDIO_INPUT_STATUS_NOTIFY                              0x1C
#define BLE_VCP_ENHANCE_AICS_AUDIO_INPUT_DESCRIPTION_NOTIFY                         0x1D
#define BLE_VCP_ENHANCE_DISCOVER_SERVICE_COMPLETE_NOTIFY                            0x1E
typedef uint8_t ble_vcp_enhance_event_t;                                           /**< The type of VCP_ENHANCE events.*/


typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    bt_status_t status;                     /**< Event status. */
    uint8_t number_of_vocs;                 /**< The number of VOCS instances. */
    uint8_t number_of_aics;                 /**< The number of AICS instances. */
} ble_vcp_enhance_discover_service_complete_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    ble_vcs_volume_t volume_setting;         /**< The volume value. */
    ble_vcs_mute_t mute;                    /**< The mute state. */
    uint8_t change_counter;                 /**< The change counter value, range: 0~255. */
} ble_vcp_enhance_read_volume_state_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t volume_flags;                   /**< The volume flags, bit 0: Volume_Setting_Persisted(0x00 = Reset Volume Setting, 0x01 = User Set Volume Setting), bit1~7: RFU. */
} ble_vcp_enhance_read_volume_flags_cfm_t;

typedef ble_vcp_enhance_read_volume_state_cfm_t ble_vcp_enhance_volume_state_notify_t;
typedef ble_vcp_enhance_read_volume_flags_cfm_t ble_vcp_enhance_volume_flags_notify_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    uint8_t change_counter;                 /**< The change counter value, range: 0~255. */
    int16_t volume_offset;                  /**< The volume offset value. */
} ble_vcp_enhance_read_volume_offset_state_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    uint32_t audio_location;                /**< The bitmask value of audio location, such as Left or Right. */
} ble_vcp_enhance_read_audio_location_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    uint8_t description_length;             /**< The length of audio output description. */
    uint8_t* audio_output_description;      /**< The audio output description string. */
} ble_vcp_enhance_read_audio_output_description_cfm_t;

typedef ble_vcp_enhance_read_volume_offset_state_cfm_t ble_vcp_enhance_volume_offset_state_notify_t;
typedef ble_vcp_enhance_read_audio_location_cfm_t ble_vcp_enhance_audio_location_notify_t;
typedef ble_vcp_enhance_read_audio_output_description_cfm_t ble_vcp_enhance_audio_output_description_notify_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    int8_t gain_setting;                    /**< The gain setting, Gain = Gain_Setting * Gain_Setting_Units. */
    ble_aics_mute_state_t mute;             /**< The mute state. */
    ble_aics_gain_mode_t gain_mode;         /**< The gain mode. */
    uint8_t change_counter;                 /**< The change counter value, range: 0~255. */
} ble_vcp_enhance_read_audio_input_state_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    uint8_t gain_setting_units;             /**< The gain getting value in 0.1 decibel units. */
    int8_t gain_setting_min;                /**< The minimum allowable value of the Gain_Setting field. */
    int8_t gain_setting_max;                /**< The maximum allowable value of the Gain_Setting field. */
} ble_vcp_enhance_read_gain_setting_properties_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    ble_aics_audio_input_type_t audio_input_type;/**< The audio input type. */
} ble_vcp_enhance_read_audio_input_type_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    ble_aics_audio_input_status_t audio_input_status;/**< The audio input status. */
} ble_vcp_enhance_read_audio_input_status_cfm_t;

typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
    uint8_t description_length;             /**< The length of audio input description. */
    uint8_t* audio_input_description;       /**< The audio input description string. */
} ble_vcp_enhance_read_audio_input_description_cfm_t;

typedef ble_vcp_enhance_read_audio_input_state_cfm_t ble_vcp_enhance_audio_input_state_notify_t;
typedef ble_vcp_enhance_read_audio_input_status_cfm_t ble_vcp_enhance_audio_input_status_notify_t;
typedef ble_vcp_enhance_read_audio_input_description_cfm_t ble_vcp_enhance_audio_input_description_notify_t;

typedef struct {
    ble_vcs_control_opcode_t opcode;        /**< The VCS control point opcode. */
    uint8_t change_counter;                 /**< The change counter value, range: 0~255. */
    uint8_t volume_setting;                 /**< The volume value, range: 0~255. */
} ble_vcp_enhance_volume_control_param_t;

typedef struct {
    ble_vocs_control_opcode_t opcode;       /**< The VOCS control point opcode. */
    uint8_t change_counter;                 /**< The change counter value, range: 0~255. */
    int16_t volume_offset;                  /**< The volume offset value:-255~255. */
} ble_vcp_enhance_volume_offset_control_param_t;

typedef struct {
    ble_aics_control_opcode_t opcode;       /**< The AICS control point opcode. */
    uint8_t change_counter;                 /**< The change counter value, range: 0~255. */
    int8_t gain_setting;                    /**< The gain setting, Gain = Gain_Setting * Gain_Setting_Units. */
} ble_vcp_enhance_audio_input_control_param_t;

typedef struct {
    bt_status_t status;                     /**< The result of write request. */
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_index;                  /**< The service index of multiple instances. */
} ble_vcp_enhance_write_cfm_t;

/**
 * @brief This structure defines the event callback for VCP.
 * @param[in] event             is the event id.
 * @param[in] msg               is the event message.
 */
typedef void (*ble_vcp_enhance_callback_t)(ble_vcp_enhance_event_t event, void *msg);

/**
 * @}
 */

/**
 * @brief                   This function initializes the VCP.
 * @param[in] callback      is the callback function of receiving the VCP events.
 * @param[in] max_link_num  is the maximum number of link.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_init(ble_vcp_enhance_callback_t callback, uint8_t max_link_num);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of volume state.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vcs_set_volume_state_notification_req(bt_handle_t handle, bool enable);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of volume flags.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vcs_set_volume_flags_notification_req(bt_handle_t handle, bool enable);

/**
 * @brief                   This function is used to read the value of volume state.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vcs_read_volume_state(bt_handle_t handle);

/**
 * @brief                   This function is used to read the value of volume flags.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vcs_read_volume_flags(bt_handle_t handle);

/**
 * @brief                   This function is used to start a volume control procedure.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] param         is the parameters of the volume control procedure.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vcs_write_volume_control_point(bt_handle_t handle, ble_vcp_enhance_volume_control_param_t *param);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of volume offset state.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vocs_set_volume_offset_state_notification_req(bt_handle_t handle, uint8_t service_index, bool enable);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of audio location.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vocs_set_audio_location_notification_req(bt_handle_t handle, uint8_t service_index, bool enable);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of audio output description.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vocs_set_audio_output_description_notification_req(bt_handle_t handle, uint8_t service_index, bool enable);

/**
 * @brief                   This function is used to read the value of volume offset state.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vocs_read_volume_offset_state(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to read the value of audio location.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vocs_read_audio_location(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to read the value of audio output description.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vocs_read_audio_output_description(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to start a volume offset control procedure.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] param         is the parameters of the volume offeset control procedure.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vocs_write_volume_offset_control_point(bt_handle_t handle, uint8_t service_index, ble_vcp_enhance_volume_offset_control_param_t *param);

/**
 * @brief                   This function is used to write the value of audio location.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] location      is the audio location of the service instance.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vocs_write_audio_location(bt_handle_t handle, uint8_t service_index, uint32_t location);

/**
 * @brief                   This function is used to write the value of audio output description.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] description   is the audio output description string.
 * @param[in] length        is the length of the audio output description string.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_vocs_write_audio_output_description(bt_handle_t handle, uint8_t service_index, uint8_t *description, uint8_t length);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of audio input state.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_aics_set_audio_input_state_notification_req(bt_handle_t handle, uint8_t service_index, bool enable);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of audio input status.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_aics_set_audio_input_status_notification_req(bt_handle_t handle, uint8_t service_index, bool enable);

/**
 * @brief                   This function is used to config the Client Characteristic Configuration descriptor value of audio input description.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] enable        is the Client Characteristic Configuration descriptor value.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_aics_set_audio_input_description_notification_req(bt_handle_t handle, uint8_t service_index, bool enable);

/**
 * @brief                   This function is used to read the value of audio input state.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_aics_read_audio_input_state(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to read the value of gain setting properties.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_aics_read_gain_setting_properties(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to read the value of audio input type.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_aics_read_audio_input_type(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to read the value of audio input status.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_aics_read_audio_input_status(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to read the value of audio input description.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_aics_read_audio_input_description(bt_handle_t handle, uint8_t service_index);

/**
 * @brief                   This function is used to start an audio input control procedure.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] param         is the parameters of the audio input control procedure.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_aics_write_audio_input_control_point(bt_handle_t handle, uint8_t service_index, ble_vcp_enhance_audio_input_control_param_t *param);

/**
 * @brief                   This function is used to write the value of audio input description.
 * @param[in] handle        is the connection handle of the Bluetooth link.
 * @param[in] service_index is the index of multiple service instances.
 * @param[in] description   is the audio input description string.
 * @param[in] length        is the length of the audio input description string.
 * @return                  #BT_STATUS_SUCCESS, the operation completed successfully.
 *                          #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vcp_enhance_aics_write_audio_input_description(bt_handle_t handle, uint8_t service_index, uint8_t *description, uint8_t length);

/**
 * @}
 * @}
 */

#endif  /* __BLE_VCP_ENHANCE_H__ */
