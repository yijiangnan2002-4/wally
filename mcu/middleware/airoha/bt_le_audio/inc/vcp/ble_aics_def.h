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

#ifndef __BLE_AICS_DEF_H__
#define __BLE_AICS_DEF_H__

#include "bt_type.h"

/**
 * @brief The AICS service start handle.
 */
#define BLE_AICS_START_HANDLE                           (0x4001)   /**< AICS Start Handle. */
#define BLE_AICS_VALUE_HANDLE_INPUT_STATE               (0x4003)   /**< Attribute Value Handle of Input State Characteristic. */
#define BLE_AICS_VALUE_HANDLE_GAIN_SETTING_PROPERTIES   (0x4006)   /**< Attribute Value Handle of Gain Setting Properties Characteristic. */
#define BLE_AICS_VALUE_HANDLE_INPUT_TYPE                (0x4008)   /**< Attribute Value Handle of Input Type Characteristic. */
#define BLE_AICS_VALUE_HANDLE_INPUT_STATUS              (0x400A)   /**< Attribute Value Handle of Input Status Characteristic. */
#define BLE_AICS_VALUE_HANDLE_CONTROL_POINT             (0x400D)   /**< Attribute Value Handle of Audio Input Control Point Characteristic. */
#define BLE_AICS_VALUE_HANDLE_AUDIO_INPUT_DESCRIPTION   (0x400F)   /**< Attribute Value Handle of Audio Input Description Characteristic. */
#define BLE_AICS_END_HANDLE                             (0x4010)   /**< AICS End Handle. */

/**
 * @brief The AICS service UUID.
 */
#define BT_SIG_UUID16_AUDIO_INPUT_CONTROL_SERVICE  (0x1843)    /**< Audio input control service. */

/**
 * @brief The AICS characteristic UUID definitions.
 */
#define BT_SIG_UUID16_AUDIO_INPUT_STATE             (0x2B77)    /**< Input State. */
#define BT_SIG_UUID16_GAIN_SETTING_PROPERTIES       (0x2B78)    /**< Gain Setting Properties. */
#define BT_SIG_UUID16_AUDIO_INPUT_TYPE              (0x2B79)    /**< Input Type. */
#define BT_SIG_UUID16_AUDIO_INPUT_STATUS            (0x2B7A)    /**< Input Status. */
#define BT_SIG_UUID16_AUDIO_INPUT_CONTROL_POINT     (0x2B7B)    /**< Audio Input Control Point. */
#define BT_SIG_UUID16_AUDIO_INPUT_DESCRIPTION       (0x2B7C)    /**< Audio Input Description. */

/**
 * @brief The AICS UUID type definitions.
 */
#define BLE_AICS_UUID_TYPE_AUDIO_INPUT_CONTROL_SERVICE   0       /**< Audio input control service. */
#define BLE_AICS_UUID_TYPE_INPUT_STATE                   1       /**< Audio input state. */
#define BLE_AICS_UUID_TYPE_GAIN_SETTING_PROPERTIES       2       /**< Gain setting properties. */
#define BLE_AICS_UUID_TYPE_INPUT_TYPE                    3       /**< Audio input type. */
#define BLE_AICS_UUID_TYPE_INPUT_STATUS                  4       /**< Audio input status. */
#define BLE_AICS_UUID_TYPE_AUDIO_INPUT_CONTROL_POINT     5       /**< Audio input control point. */
#define BLE_AICS_UUID_TYPE_AUDIO_INPUT_DESCRIPTION       6       /**< Audio input description. */
#define BLE_AICS_UUID_TYPE_MAX_NUM                       7       /**< The max number of AICS UUID type.*/
#define BLE_AICS_UUID_TYPE_INVALID                       0xFF    /**< The invalid AICS UUID type.*/
typedef uint8_t ble_aics_uuid_t;                                 /**< UUID type.*/

/**
 * @brief The AICS max number of characteristics.
 */
#define BLE_AICS_MAX_CHARC_NUMBER    (BLE_AICS_UUID_TYPE_MAX_NUM-1)   /**< The number of AICS characteristics.*/

/**
 * @brief The AICS gain mode.
 */
#define BLE_AICS_GAIN_MODE_MANUAL_ONLY                   0x00   /**< Manual only, the server does not support changes to gain mode. */
#define BLE_AICS_GAIN_MODE_AUTOMATIC_ONLY                0x01   /**< Automatic only, the server does not support changes to the gain mode. */
#define BLE_AICS_GAIN_MODE_MANUAL                        0x02   /**< Manual. */
#define BLE_AICS_GAIN_MODE_AUTOMATIC                     0x03   /**< Automatic. */
typedef uint8_t ble_aics_gain_mode_t;

/**
 * @brief The AICS mute state.
 */
#define BLE_AICS_MUTE_STATE_UNMUTE                       0x00   /**< Not Muted. */
#define BLE_AICS_MUTE_STATE_MUTE                         0x01   /**< Muted. */
#define BLE_AICS_MUTE_STATE_DISABLE                      0x02   /**< Disabled. */
typedef uint8_t ble_aics_mute_state_t;

#define BLE_AICS_AUDIO_INPUT_TYPE_UNSPECIFIED            0x00   /**< Unspecified input. */
#define BLE_AICS_AUDIO_INPUT_TYPE_BLUETOOTH              0x01   /**< Bluetooth Audio Stream. */
#define BLE_AICS_AUDIO_INPUT_TYPE_MICROPHONE             0x02   /**< Microphone. */
#define BLE_AICS_AUDIO_INPUT_TYPE_ANALOG                 0x03   /**< Analog Interface. */
#define BLE_AICS_AUDIO_INPUT_TYPE_DIGITAL                0x04   /**< Digital Interface. */
#define BLE_AICS_AUDIO_INPUT_TYPE_RADIO                  0x05   /**< AM/FM/XM/etc. */
#define BLE_AICS_AUDIO_INPUT_TYPE_STREAMING              0x06   /**< Streaming Audio Source. */
typedef uint8_t ble_aics_audio_input_type_t;

/**
 * @brief The AICS audio input status.
 */
#define BLE_AICS_AUDIO_INPUT_STATUS_INACTIVE             0x00   /**< Inactive. */
#define BLE_AICS_AUDIO_INPUT_STATUS_ACTIVE               0x01   /**< Active. */
typedef uint8_t ble_aics_audio_input_status_t;

/**
 * @brief The AICS control point opcode.
 */
#define BLE_AICS_OPCODE_SET_GAIN_SETTING                 0x01        /**< Set Gain Setting, Operand: Change_Counter,Gain_Setting. */
#define BLE_AICS_OPCODE_UNMUTE                           0x02        /**< Unmute, Operand: Change_Counter. */
#define BLE_AICS_OPCODE_MUTE                             0x03        /**< Mute, Operand: Change_Counter. */
#define BLE_AICS_OPCODE_SET_MANUAL_GAIN_MODE             0x04        /**< Set Manual Gain Mode, Operand: Change_Counter. */
#define BLE_AICS_OPCODE_SET_AUTOMATIC_GAIN_MODE          0x05        /**< Set Automatic Gain Mode, Operand: Change_Counter. */
typedef uint8_t ble_aics_control_opcode_t;


/**
 *  @brief This structure defines AICS attribute handle detail.
 */
typedef struct {
    ble_aics_uuid_t uuid_type;                      /**< UUID type */
    uint16_t att_handle;                            /**< Attribute handle */
} ble_aics_attribute_handle_t;

/**
 * @brief The AICS GATT type definitions.
 */
#define BLE_AICS_READ_INPUT_STATE                      0x00 /**< Read input state. */
#define BLE_AICS_READ_INPUT_STATE_CCCD                 0x01 /**< Read input state CCCD. */
#define BLE_AICS_WRITE_INPUT_STATE_CCCD                0x02 /**< Write input state CCCD. */
#define BLE_AICS_READ_GAIN_SETTING_PROPERTIES          0x03 /**< Read gain setting properties. */
#define BLE_AICS_READ_INPUT_TYPE                       0x04 /**< Read input type. */
#define BLE_AICS_READ_INPUT_STATUS                     0x05 /**< Read input status. */
#define BLE_AICS_READ_INPUT_STATUS_CCCD                0x06 /**< Read input status CCCD. */
#define BLE_AICS_WRITE_INPUT_STATUS_CCCD               0x07 /**< Write input status CCCD. */
#define BLE_AICS_WRITE_AUDIO_INPUT_CONTROL_POINT       0x08 /**< Write audio input control point. */
#define BLE_AICS_READ_AUDIO_INPUT_DESCRIPTION          0x09 /**< Read input description. */
#define BLE_AICS_WRITE_AUDIO_INPUT_DESCRIPTION         0x0A /**< Write input description. */
#define BLE_AICS_READ_AUDIO_INPUT_DESCRIPTION_CCCD     0x0B /**< Read input description CCCD. */
#define BLE_AICS_WRITE_AUDIO_INPUT_DESCRIPTION_CCCD    0x0C /**< Write input description CCCD. */
#define BLE_AICS_GATTS_REQ_MAX                         0x0D /**< The maximum number of AICS GATT type. */
typedef uint8_t ble_aics_gatt_request_t;

/**
 * @brief                       This function distributes AICS requests to the corresponding handler from specified remote device.
 * @param[in] req               is the type of AICS request.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] channel           is the channel.
 * @param[in] data              is the data.
 * @param[in] size              is the size of data.
 * @return                      the size of responded data.
 *
 */
uint16_t ble_aics_gatt_request_handler(ble_aics_gatt_request_t req, bt_handle_t handle, uint8_t channel, void *data, uint16_t size);

#endif  /* __BLE_AICS_DEF_H__ */

