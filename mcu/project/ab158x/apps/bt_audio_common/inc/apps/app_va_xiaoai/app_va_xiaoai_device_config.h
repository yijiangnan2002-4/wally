
/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

/**
 * File: app_va_xiaoai_device_config.h
 *
 * Description: This file defines the interface of app_va_xiaoai_device_config.c.
 *
 */

#ifndef __APP_VA_XIAOAI_DEVICE_CONFIG_H__
#define __APP_VA_XIAOAI_DEVICE_CONFIG_H__

#ifdef AIR_XIAOAI_ENABLE

#include "stdbool.h"
#include "stdint.h"
#include "bt_customer_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XIAOAI_BT_NAME_LEN                  (BT_GAP_LE_MAX_DEVICE_NAME_LENGTH)

typedef enum {
    XIAOAI_MMA_TYPE_AUDIO_MODE                                 = 0x0001,
    XIAOAI_MMA_TYPE_CUSTOM_KEY                                 = 0x0002,
    XIAOAI_MMA_TYPE_AUTO_ACCEPT_INCOMING_CALL                  = 0x0003,
    XIAOAI_MMA_TYPE_MULTI_POINT_ENABLE                         = 0x0004,
    XIAOAI_MMA_TYPE_START_FIT_DETECTION                        = 0x0005,
    XIAOAI_MMA_TYPE_HEADSET_FIT                                = 0x0006,
    XIAOAI_MMA_TYPE_EQ_MODE                                    = 0x0007,
    XIAOAI_MMA_TYPE_DEVICE_NAME                                = 0x0008,
    XIAOAI_MMA_TYPE_FIND_ME                                    = 0x0009,
    XIAOAI_MMA_TYPE_SWITCH_ANC_MODE                            = 0x000A,
    XIAOAI_MMA_TYPE_SELECT_ANC_LEVEL                           = 0x000B,
    XIAOAI_MMA_TYPE_ANTI_LOST                                  = 0x000C,
    XIAOAI_MMA_TYPE_CHAT_FREE_MODE                             = 0x000D,
    XIAOAI_MMA_TYPE_HOST_APP_PACKAGE_NAME                      = 0x000E,

    XIAOAI_MMA_TYPE_BLE_ALL_ADV                                = 0x0010,
    XIAOAI_MMA_TYPE_BLE_MIUI_CONNECT_ADV                       = 0x0011,
    XIAOAI_MMA_TYPE_BLE_ADV_DATA                               = 0x0012,
    XIAOAI_MMA_TYPE_BLE_ADV_SCAN_RSP                           = 0x0013,
    XIAOAI_MMA_TYPE_HEADSET_NOTIFY_BASIC_INFO                  = 0x0014,
    XIAOAI_MMA_TYPE_PHONE_GET_BASIC_INFO                       = 0x0015,
    XIAOAI_MMA_TYPE_PHONE_MODIFY_HEADSET_NAME                  = 0x0016,
    XIAOAI_MMA_TYPE_HEADSET_NAME_MODIFY_RESULT                 = 0x0017,
    XIAOAI_MMA_TYPE_SET_HEADSET_CANNOT_RECONNECT               = 0x0018,
    XIAOAI_MMA_TYPE_PHONE_MODIFY_HEADSET_ACCOUNT_KEY           = 0x0019,
    XIAOAI_MMA_TYPE_HEADSET_ACCOUNT_KEY_MODIFY_RESULT          = 0x001A,
    XIAOAI_MMA_TYPE_PHONE_MODIFY_HEADSET_LE_AUDIO              = 0x001B,
    XIAOAI_MMA_TYPE_HEADSET_LE_AUDIO_STATUS                    = 0x001C,
    XIAOAI_MMA_TYPE_PHONE_MODIFY_HEADSET_GYRO                  = 0x001D,
    XIAOAI_MMA_TYPE_HEADSET_MODIFY_GYRO_RESULT                 = 0x001E,
    XIAOAI_MMA_TYPE_PHONE_MODIFY_HEADSET_NOISE_DIS             = 0x001F,
    XIAOAI_MMA_TYPE_HEADSET_NOISE_DIS_RESULT                   = 0x0020,

    XIAOAI_MMA_TYPE_UNKNOWN                                    = 0xFFFF,
} xiaoai_mma_type;

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

/* ANC handle event. */
void     app_va_xiaoai_handle_anc_event(bool on_event, void *extra_data);

/* Get ANC state for MIUI HFP AT CMD. */
uint8_t app_va_xiaoai_get_anc_state_for_miui();

/* Get WWE state. */
bool app_va_xiaoai_is_enable_wwe();

/* Get voice_recognition_state. */
uint8_t app_va_xiaoai_get_voice_recognition_state();

/* Get eq state. */
uint8_t app_va_xiaoai_get_eq_mode();

/* Get game mode for MIUI HFP AT CMD. */
uint8_t app_va_xiaoai_get_game_mode_for_miui();

/* Get anti_lost state. */
uint8_t app_va_xiaoai_get_anti_lost_state();

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
void    app_va_xiaoai_notify_leakage_detectable(bool detectable);
void    app_va_xiaoai_agent_handle_partner_leakage_result(uint8_t leakage_result);
bool    app_va_xiaoai_is_ld_ongoing();
#endif

bool    app_va_xiaoai_own_set_device_name(uint8_t *name, uint8_t len);
void    app_va_xiaoai_peer_set_device_name(uint8_t *name, uint8_t len);

void    app_va_xiaoai_partner_handle_saving_config(uint8_t *data);

void    app_va_xiaoai_handle_chat_event(bool detect_chat);

/* Device Config SET - 0x08/0xF2. */
void    app_va_xiaoai_set_device_config(bool sync_reply, void *param);

/* Device Config GET - 0x09/0xF3. */
uint32_t app_va_xiaoai_get_device_config(uint8_t type);

/* Device Config Notify - 0xF4. */
void     app_va_xiaoai_notify_device_config(uint8_t type, uint8_t *data, uint8_t len);

void     app_va_xiaoai_init_device_config();



bool     app_va_xiaoai_need_run_in_agent(uint8_t type);
typedef struct {
    uint8_t opcode;
    uint8_t rsp_status;
    uint8_t app_type;
    uint8_t opcode_sn;
} PACKED xiaoai_device_config_reply_result_t;
void     app_va_xiaoai_reply_device_config_result(xiaoai_device_config_reply_result_t *result);


typedef struct {
    uint8_t type;
    uint8_t len;
    uint8_t data[1];
} PACKED xiaoai_device_config_notify_t;

typedef struct {
    uint16_t mma_type;
    uint32_t atcmd_len;
    uint8_t  atcmd[1];
} PACKED xiaoai_at_cmd_param_t;


typedef enum {
    XIAOAI_APP_NOTIFY_AWS_STATUS = 0,
    XIAOAI_APP_NOTIFY_EDR_STATUS,
    XIAOAI_APP_NOTIFY_ANC_STATUS,
    XIAOAI_APP_NOTIFY_BATTERY_STATUS
} xiaoai_app_notify_status;

typedef struct {
    uint8_t  notify_type;    // 07/0E
    uint8_t  data;
} PACKED xiaoai_notify_sp_param_t;

bool xiaoai_notify_sp_status(uint8_t notify_type, uint8_t data);


typedef struct {
    uint8_t    left_result;
    uint8_t    right_result;
} PACKED xiaoai_leakage_result_param_t;
void xiaoai_set_ld_result(uint8_t left_result, uint8_t right_result);

#ifdef __cplusplus
}
#endif

#endif /* AIR_XIAOAI_ENABLED */

#endif /* __APP_VA_XIAOAI_DEVICE_CONFIG_H__ */

