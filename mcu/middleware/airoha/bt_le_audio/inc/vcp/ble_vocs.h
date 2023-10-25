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

#ifndef __BLE_VOCS_H__
#define __BLE_VOCS_H__

#include "bt_type.h"
#include "bt_le_audio_def.h"

/**
 * @brief                           This function is used to set volume offset.
 * @param[in] handle                is the connection handle of the Bluetooth link.
 * @param[in] channel               is the audio channel.
 * @param[in] offset                is the volume offset value.
 * @return                          #BT_STATUS_SUCCESS, the operation completed successfully.
 *                                  #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vocs_set_volume_offset(bt_handle_t handle, uint8_t channel, int16_t offset);

/**
 * @brief                           This function is used to set audio output description.
 * @param[in] handle                is the connection handle of the Bluetooth link.
 * @param[in] channel               is the audio channel.
 * @param[in] output_description    is the output description string.
 * @param[in] length                is the length of input description string.
 * @return                          #BT_STATUS_SUCCESS, the operation completed successfully.
 *                                  #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vocs_set_audio_output_description(bt_handle_t handle, uint8_t channel, uint8_t *output_description, uint8_t length);

/**
 * @brief                           This function is used to set audio output description by channel.
 * @param[in] channel               is the audio channel.
 * @param[in] output_description    is the input description string.
 * @param[in] length                is the length of input description string.
 * @return                          #BT_STATUS_SUCCESS, the operation completed successfully.
 *                                  #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vocs_set_audio_output_description_by_channel(uint8_t channel, uint8_t *output_description, uint8_t length);

/**
 * @brief                           This function is used to set audio location by channel.
 * @param[in] channel               is the audio channel.
 * @param[in] audio_location        is the audio location.
 * @return                          #BT_STATUS_SUCCESS, the operation completed successfully.
 *                                  #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t ble_vocs_set_audio_location_by_channel (uint8_t channel, bt_le_audio_location_t audio_location);

#endif  /* __BLE_VOCS_H__ */

