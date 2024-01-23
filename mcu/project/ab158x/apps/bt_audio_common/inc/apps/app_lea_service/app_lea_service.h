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

#ifndef __APP_LEA_SERVICE_H__
#define __APP_LEA_SERVICE_H__

/**
 * File: app_lea_service.h
 *
 * Description: This file defines the public API for LE Audio.
 *
 */
#include "app_lea_service_config.h"

#include "bt_hci.h"
#include "ui_shell_activity.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_lea_service_init(void);

bool app_lea_service_set_feature_mode(uint8_t feature_mode);
bool app_lea_service_is_enable_dual_mode(void);

void app_lea_service_enable_multi_conn(bool enable);

void app_lea_service_start_advertising(uint8_t mode, bool sync, uint32_t timeout);
void app_lea_service_stop_advertising(bool sync);
void app_lea_service_quick_stop_adv(void);
void app_lea_service_refresh_advertising(uint32_t delay);

void app_lea_service_start_reconnect_adv(const uint8_t *addr);

bool app_lea_service_is_enabled_lea(void);

void app_lea_service_reset_lea_dongle(void);

#define APP_LEA_SERVICE_ADV_DATA_LEA        0
#define APP_LEA_SERVICE_ADV_DATA_ULL        1
bool app_lea_service_control_adv_data(uint8_t adv_type, bool enable);

typedef enum {
    APP_LE_AUDIO_DISCONNECT_MODE_ALL,
    APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT,
    APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT_ULL,
    APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT_LEA,
    APP_LE_AUDIO_DISCONNECT_MODE_KEEP,
} app_le_audio_disconnect_mode_t;

void app_lea_service_disconnect(bool sync,
                                app_le_audio_disconnect_mode_t mode,
                                const uint8_t *addr,
                                bt_hci_disconnect_reason_t reason);

bool app_lea_service_activity_proc(struct _ui_shell_activity *self,
                                   uint32_t event_group,
                                   uint32_t event_id,
                                   void *extra_data,
                                   size_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* __APP_LEA_SERVICE_H__ */
