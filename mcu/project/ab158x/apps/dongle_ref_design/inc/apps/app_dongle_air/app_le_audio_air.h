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

#ifndef __APP_LE_AUDIO_AIR_H__
#define __APP_LE_AUDIO_AIR_H__


#include "bt_type.h"
#include "bt_aws_mce.h"

#define APP_LE_AUDIO_AIR_ROLE_INVALID   0xFF

#define APP_LE_AUDIO_AIR_EVENT_ENABLE_SERVICE_COMPLETE   1
#define APP_LE_AUDIO_AIR_EVENT_RHO                       2
typedef uint8_t app_le_audio_air_event_t;

#define APP_LE_AUDIO_AIR_STATE_IDLE                      0
#define APP_LE_AUDIO_AIR_STATE_SRV_DISCOVERY_COMPLETE    1
#define APP_LE_AUDIO_AIR_STATE_READ_ROLE                 2
#define APP_LE_AUDIO_AIR_STATE_SET_CCCD_TX               3
#define APP_LE_AUDIO_AIR_STATE_SET_CCCD_ROLE             4
#define APP_LE_AUDIO_AIR_STATE_READY                     5
#define APP_LE_AUDIO_AIR_STATE_MAX                       6
typedef uint8_t app_le_audio_air_state_t;

typedef struct {
    bt_handle_t handle;
    bt_status_t status;
    uint8_t role;
} app_le_audio_air_event_enable_service_complete_t;

typedef struct {
    bt_handle_t handle;
    uint8_t role;
} app_le_audio_air_event_rho_t;

typedef struct {
    bt_handle_t conn_handle;
    bt_addr_t   addr;
    bt_handle_t att_handle_rx;
    bt_handle_t att_handle_tx;
    bt_handle_t att_handle_tx_cccd;
    bt_handle_t att_handle_role;
    bt_handle_t att_handle_role_cccd;
    app_le_audio_air_state_t state;
    bt_aws_mce_role_t role;
} app_le_audio_air_info_t;

typedef void (*app_le_audio_air_callback_t)(app_le_audio_air_event_t event, void *buff);

bt_status_t app_le_audio_air_register_callback(app_le_audio_air_callback_t callback);

void app_le_audio_air_event_handler(bt_msg_type_t msg, bt_status_t status, void *buff);

void app_le_audio_air_start_pre_action(bt_handle_t handle, bt_status_t status);

app_le_audio_air_info_t *app_le_audio_air_get_info(bt_handle_t handle);

bt_aws_mce_role_t app_le_audio_air_get_role(bt_addr_t *addr);

void app_le_audio_air_reset_info(uint8_t link_idx);

void app_le_audio_air_init(void);

void ble_audio_air_main(void);


#endif /* __APP_LE_AUDIO_AIR_H__ */

