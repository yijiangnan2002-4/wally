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

#ifndef __BLE_HAPC_H__
#define __BLE_HAPC_H__

#include "ble_has_def.h"
#include "bt_gattc_discovery.h"

/**
 *  @brief Defines the HAPC events.
 */
#define BLE_HAPC_EVT_DISCOVERY_COMPLETE         (BT_LE_AUDIO_MODULE_HAPC | 0x0001)    /**< discover service complete. */
#define BLE_HAPC_EVT_REQUEST_COMPLETE           (BT_LE_AUDIO_MODULE_HAPC | 0x0002)    /**< The user request is complete. */
#define BLE_HAPC_EVT_HA_FEATURE_IND             (BT_LE_AUDIO_MODULE_HAPC | 0x0003)    /**< Information of hearing aid feature. */
#define BLE_HAPC_EVT_PRESET_INFO_IND            (BT_LE_AUDIO_MODULE_HAPC | 0x0004)    /**< Information of preset. */
#define BLE_HAPC_EVT_PRESET_CHANGE_IND          (BT_LE_AUDIO_MODULE_HAPC | 0x0005)    /**< preset info is changed. */
#define BLE_HAPC_EVT_PRESET_DELETED_IND         (BT_LE_AUDIO_MODULE_HAPC | 0x0006)    /**< preset is deleted. */
#define BLE_HAPC_EVT_PRESET_AVAILABLE_IND       (BT_LE_AUDIO_MODULE_HAPC | 0x0007)    /**< preset is become available. */
#define BLE_HAPC_EVT_PRESET_UNAVAILABLE_IND     (BT_LE_AUDIO_MODULE_HAPC | 0x0008)    /**< preset is become unavailable. */
#define BLE_HAPC_EVT_PRESET_ACTIVE_IND          (BT_LE_AUDIO_MODULE_HAPC | 0x0009)    /**< Preset is active. */
typedef uint32_t ble_hapc_event_t;                                                    /**< The type of HAPC events.*/

/**
 *  @brief Defines ATT APP error code of HAP Profile.
 */
#define HAPC_ATT_ERR_NONE                            0x00   /**< The HAPC request is Success. */
#define HAPC_ATT_ERR_INVALID_OPCODE                  0x80   /**< Thes HAPC request is using invalid opcode. */
#define HAPC_ATT_ERR_WRITE_NAME_NOT_ALLOWED          0x81   /**< Thes HAPC that writes Preset name to HA is disallowed. */
#define HAPC_ATT_ERR_PRESET_SYNC_NOT_SUPPORT         0x82   /**< The HA does not support Preset synchornization. */
#define HAPC_ATT_ERR_PRESET_OPERATION_NOT_POSSIBLE   0x83   /**< The HAPC can not be excuted in this time. */
#define HAPC_ATT_ERR_PRESET_INVALID_PARAM_LEN        0x84   /**< The HAPC requested to write valid opcode but with parameters of invalid length. */
#define HAPC_ATT_ERR_WRITE_REQUEST_REJECTED          0xFC   /**< The HAPC request is reject by HA. */
#define HAPC_ATT_ERR_CCCD_IMPROPERLY_CONFIG          0xFD   /**< The HAPC requested to write valid opcode when CCCD of HA is not improperly configed. */
#define HAPC_ATT_ERR_PROCEDURE_IN_PROGRESS           0xFE   /**< The HAPC requested to write valid opcode when HA is busy. */
#define HAPC_ATT_ERR_OUT_OF_RANGE                    0xFF   /**< The HAPC requested to write valid opcode with invalid Preset index. */
typedef uint8_t ble_hapc_error_t;                           /**< The HAPC request error code.*/

/**
 *  @brief Defines the user request.
 */
#define HAPC_REQ_READ_PRESET                         0x01   /**< To read Preset information of HA. */
#define HAPC_REQ_READ_HA_FEATURE                     0x02   /**< To read Feature of HA.*/
#define HAPC_REQ_READ_ACTIVE_INDEX                   0x03   /**< To read active Preset index of HA. */
#define HAPC_REQ_WRITE_PRESET_NAME                   0x04   /**< To write Preset name of HA. */
#define HAPC_REQ_SET_ACTIVE_PRESET                   0x05   /**< To set active Preset of HA. */
#define HAPC_REQ_SET_NEXT_PRESET                     0x06   /**< To set next index of current active Preset of HA to be active. */
#define HAPC_REQ_SET_PREVIOUS_PRESET                 0x07   /**< To set previous index of current active Preset of HA to be active. */
#define HAPC_REQ_SET_ACTIVE_PRESET_SYNC              0x08   /**< To set active Preset of HA and sync to other HA */
#define HAPC_REQ_SET_NEXT_PRESET_SYNC                0x09   /**< To set next index of current active Preset of HA to be active and sync to other HA. */
#define HAPC_REQ_SET_PREVIOUS_PRESET_SYNC            0x0A   /**< To set previous index of current active Preset of HA to be active and sync to other HA. */
typedef uint8_t ble_hapc_usr_req_t;                         /**< HAPC User request .*/


/**
 *  @brief This structure defines HAPC event BLE_HAPC_EVT_DISCOVERY_COMPLETE detail.
 */
typedef struct
{
    bt_handle_t conn_hdl;                       /**< link connection handle */
    bt_status_t status;                         /**< BT_STATUS_SUCCESS, BT_STATUS_FAIL */
    uint8_t ha_ctrl_pt_support;                 /**< HA support control point characteristic or not */
} ble_hapc_evt_discovery_complete_t;

/**
 *  @brief This structure defines HAPC event BLE_HAPC_EVT_REQUEST_COMPLETE detail.
 */
typedef struct
{
    bt_handle_t conn_hdl;                       /**< link connection handle */
    uint8_t req;                                /**< user request, refer to ble_hapc_usr_req_t */
    uint8_t status;                             /**< status of user request, refer to ble_hapc_error_t */
} ble_hapc_evt_request_complete_t;

/**
 *  @brief This structure defines HAPC event BLE_HAPC_EVT_HA_FEATURE_IND detail.
 */
typedef struct
{
    bt_handle_t conn_hdl;                       /**< link connection handle */
    uint8_t feature;                            /**< hearing aid feature */
} ble_hapc_evt_ha_feature_ind_t;

/**
 *  @brief This structure defines HAPC event BLE_HAPC_EVT_PRESET_INFO_IND detail.
 */
typedef struct
{
    bt_handle_t conn_hdl;                       /**< link connection handle */
    uint8_t index;                              /**< Preset index */
    uint8_t property;                           /**< Preset property */
    uint8_t name_len;                           /**< Preset name length */
    uint8_t name[1];                            /**< Preset name */
} ble_hapc_evt_preset_info_ind_t;

/**
 *  @brief This structure defines HAPC event BLE_HAPC_EVT_PRESET_CHANGE_IND detail.
 */
typedef struct
{
    bt_handle_t conn_hdl;                       /**< link connection handle */
    uint8_t prev_idx;                           /**< Preset previous index */
    uint8_t index;                              /**< Preset index */
    uint8_t property;                           /**< Preset property */
    uint8_t name_len;                           /**< Preset name length */
    uint8_t name[1];                            /**< Preset name */
} ble_hapc_evt_preset_change_ind_t;

/**
 *  @brief This structure defines HAPC event BLE_HAPC_EVT_PRESET_DELETED_IND detail.
 */
typedef struct
{
    bt_handle_t conn_hdl;                       /**< link connection handle */
    uint8_t index;                              /**< Preset index */
} ble_hapc_evt_preset_deleted_ind_t;

/**
 *  @brief This structure defines HAPC event BLE_HAPC_EVT_PRESET_AVAILABLE_IND detail.
 */
typedef struct
{
    bt_handle_t conn_hdl;                       /**< link connection handle */
    uint8_t index;                              /**< Preset index */
} ble_hapc_evt_preset_available_ind_t;

/**
 *  @brief This structure defines HAPC event BLE_HAPC_EVT_PRESET_UNAVAILABLE_IND detail.
 */
typedef struct
{
    bt_handle_t conn_hdl;                       /**< link connection handle */
    uint8_t index;                              /**< Preset index */
} ble_hapc_evt_preset_unavailable_ind_t;

/**
 *  @brief This structure defines HAPC event BLE_HAPC_EVT_PRESET_ACTIVE_IND detail.
 */
typedef struct
{
    bt_handle_t conn_hdl;                       /**< link connection handle */
    uint8_t index;                              /**< Preset index */
} ble_hapc_evt_preset_active_ind_t;

/**
 * @brief The HAPC event callback.
 */
typedef void (*ble_hapc_callback_t)(ble_hapc_event_t event, void *msg);

/**
 * @brief                       This function is used to initialize HAP Client.
 * @param[in] callback          HAPC event callback.
 * @param[in] max_link_num      is the max number of Bluetooth link.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_hapc_init(ble_hapc_callback_t callback, uint8_t max_link_num);

/**
 * @brief                       This function is used to response HAPC to stop GATT operation.
 * @param[in] conn_hdl          link connection handle.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_hapc_discovery_complete_response(bt_handle_t conn_hdl);

/**
 * @brief                       This function is used to get HA Feature.
 * @param[in] conn_hdl          link connection handle.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_BUSY, HAPC is busy to process other request.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_hapc_read_ha_feature(bt_handle_t conn_hdl);

/**
 * @brief                       This function is used to get active preset index of HA.
 * @param[in] conn_hdl          link connection handle.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_BUSY, HAPC is busy to process other request.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_hapc_read_active_index(bt_handle_t conn_hdl);

/**
 * @brief                       This function is used to get active preset index of HA.
 * @param[in] conn_hdl          link connection handle.
 * @param[in] start_index       The start Index of preset that HAPC wants to read.
 * @param[in] count             The start Index of preset that HAPC wants to read.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_BUSY, HAPC is busy to process other request.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_hapc_read_preset(bt_handle_t conn_hdl, uint8_t start_index, uint8_t count);

/**
 * @brief                       This function is used to set the name of Preset by index.
 * @param[in] conn_hdl          link connection handle.
 * @param[in] index             The index of preset that HAPC wants to change the name.
 * @param[in] name_len          The name length of preset that HAPC wants to write.
 * @param[in] name              The name of preset that HAPC wants to write.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_BUSY, HAPC is busy to process other request.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_hapc_write_preset_name(bt_handle_t conn_hdl, uint8_t index, uint8_t name_len, void *name);

/**
 * @brief                       This function is used to set Preset to be active by index.
 * @param[in] conn_hdl          link connection handle.
 * @param[in] index             The index of preset that HAPC wants to set.
 * @param[in] sync              If the type of HA is binaural and the property independent of HAs is TRUE, then
 *                              HA1 will notity HA2 that HAUC has set the active index.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_BUSY, HAPC is busy to process other request.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_hapc_set_active_preset(bt_handle_t conn_hdl, uint8_t index, bool sync);

/**
 * @brief                       This function is used to set next index of active Preset to be active.
 * @param[in] conn_hdl          link connection handle.
 * @param[in] sync              If the type of HA is binaural and the property independent of HAs is TRUE, then
 *                              HA1 will notity HA2 that HAUC has set the active index.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_BUSY, HAPC is busy to process other request.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_hapc_set_next_preset(bt_handle_t conn_hdl, bool sync);

/**
 * @brief                       This function is used to set previous index of active Preset to be active.
 * @param[in] conn_hdl          link connection handle.
 * @param[in] sync              If the type of HA is binaural and the property independent of HAs is TRUE, then
 *                              HA1 will notity HA2 that HAUC has set the active index.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_BUSY, HAPC is busy to process other request.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_hapc_set_previous_preset(bt_handle_t conn_hdl, bool sync);

/**
 * @brief                       This function sets the service attribute of HAPS.
 * @param[in] event             the attribute information of HAPS.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_OUT_OF_MEMORY, out of memory.
 *                              #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_hapc_set_service_attribute(bt_gattc_discovery_event_t *event);

#endif
