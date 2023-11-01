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

#ifndef __APP_LE_AUDIO_VCP_VOLUME_CONTROLLER_H__
#define __APP_LE_AUDIO_VCP_VOLUME_CONTROLLER_H__

#ifdef AIR_LE_AUDIO_ENABLE

#include "bt_type.h"
#include "ble_vcs_def.h"
#include "ble_vcp_enhance.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_LEA_VCP_OPERATE_IDLE                              0x00        /**< Idle. */
#define APP_LEA_VCP_OPERATE_RELATIVE_VOLUME_DOWN              0x01        /**< Relative volume down. */
#define APP_LEA_VCP_OPERATE_RELATIVE_VOLUME_UP                0x02        /**< Relative volume up. */
#define APP_LEA_VCP_OPERATE_UNMUTE_RELATIVE_VOLUME_DOWN       0x03        /**< Unmute and relative volume down. */
#define APP_LEA_VCP_OPERATE_UNMUTE_RELATIVE_VOLUME_UP         0x04        /**< Unmute and relative volume up. */
#define APP_LEA_VCP_OPERATE_SET_ABSOLUTE_VOLUME               0x05        /**< Set absolute volume. */
#define APP_LEA_VCP_OPERATE_UNMUTE                            0x06        /**< Unmute. */
#define APP_LEA_VCP_OPERATE_MUTE                              0x07        /**< Mute. */
typedef uint8_t app_le_audio_vcp_operate_t;

typedef struct {
    uint8_t volume_flags;
    ble_vcs_volume_t volume_setting;                /**< The volume value. */
    ble_vcs_mute_t mute;                            /**< The mute state. */
    uint8_t change_counter;                         /**< The change counter value, range: 0~255. */
    app_le_audio_vcp_operate_t current_operate;     /**< The volume operate. */
    app_le_audio_vcp_operate_t next_operate;        /**< The volume operate. */
    ble_vcs_volume_t target_volume_setting;         /**< The volume value. */
    ble_vcs_mute_t target_mute;                     /**< The mute state. */
} app_le_audio_vcs_info_t;

#if 0
typedef struct {
    uint8_t volume_offset;
    uint8_t change_counter;
    uint32_t audio_location;
    uint8_t *audio_output_description;
    //TBD: add other info
}app_le_audio_vocs_info_t;

typedef struct {
    int8_t gain_setting;
    uint8_t mute;
    uint8_t gain_mode;
    uint8_t change_counter;
    uint8_t gain_setting_units;
    int8_t gain_setting_min;
    int8_t gain_setting_max;
    uint8_t audio_input_type;
    uint8_t audio_input_status;
    uint8_t *audio_input_description;
    //TBD: add other info
}app_le_audio_aics_info_t;

#endif

typedef struct {
    app_le_audio_vcs_info_t vcs_info;
    //app_le_audio_vocs_info_t vocs_info[2];
    //app_le_audio_aics_info_t aics_info[2];
} app_le_audio_vcp_info_t;

/**************************************************************************************************
* Public function
**************************************************************************************************/
void app_le_audio_vcp_handle_evt(ble_vcp_enhance_event_t event, void *msg);
bt_status_t app_le_audio_vcp_handle_retry_event(uint32_t vcp_retry);
bt_status_t app_le_audio_vcp_relative_volume_down(bt_handle_t handle, bool unmute);
bt_status_t app_le_audio_vcp_relative_volume_up(bt_handle_t handle, bool unmute);
bt_status_t app_le_audio_vcp_set_absolute_volume(bt_handle_t handle, ble_vcs_volume_t volume_setting);
bt_status_t app_le_audio_vcp_unmute(bt_handle_t handle);
bt_status_t app_le_audio_vcp_mute(bt_handle_t handle);
bt_status_t app_le_audio_vcp_set_volume_state(bt_handle_t handle, ble_vcs_volume_t volume_setting, ble_vcs_mute_t mute);
bt_status_t app_le_audio_vcp_control_active_device_volume(app_le_audio_vcp_operate_t operate, void *parameter);
bt_status_t app_le_audio_vcp_control_device_volume(app_le_audio_vcp_operate_t operate, void *parameter);

#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_VCP_VOLUME_CONTROLLER_H__ */

