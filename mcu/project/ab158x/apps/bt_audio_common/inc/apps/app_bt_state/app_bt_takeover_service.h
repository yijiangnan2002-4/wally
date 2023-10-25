/* Copyright Statement:
 *
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

/**
 * File: app_bt_state_service.h
 *
 * Description: This file defines the interface of app_bt_state_service.c.
 *
 */

#ifndef __APP_BT_TAKEOVER_SERVICE_H__
#define __APP_BT_TAKEOVER_SERVICE_H__

#include "bt_type.h"

//#define AIR_BT_TAKEOVER_FIRST_DEVICE                    // Select first device to disconnect when both idle, no last_play_device
//#define AIR_BT_TAKEOVER_KEEP_DONGLE                     // Keep dongle not be takeover
//#define AIR_BT_TAKEOVER_DONGLE_MUST_TAKEOVER            // Dongle always takeover successfully
//#define AIR_BT_TAKEOVER_EMP_OFF_ALWAYS_TAKEOVER         // Always disconnect old link when EMP OFF
//#define AIR_BT_TAKEOVER_EMP_OFF_ALWAYS_REJECT_NEW       // Always reject new link when EMP OFF

typedef enum {
    APP_BT_TAKEOVER_ID_BTSINK,
    APP_BT_TAKEOVER_ID_AMA,
    APP_BT_TAKEOVER_ID_BISTO,
    APP_BT_TAKEOVER_ID_XIAOWEI,
    APP_BT_TAKEOVER_ID_XIAOAI,
    APP_BT_TAKEOVER_ID_FASTPAIR,
    APP_BT_TAKEOVER_ID_OTA,
    APP_BT_TAKEOVER_ID_MAX,
} app_bt_takeover_user_id_t;

typedef bool (*app_bt_takeover_service_allow_func_t)(const bt_bd_addr_t remote_addr);

bool app_bt_takeover_service_user_register(app_bt_takeover_user_id_t user_id, app_bt_takeover_service_allow_func_t func);

bool app_bt_takeover_service_user_deregister(app_bt_takeover_user_id_t user_id);

void app_bt_takeover_service_init(void);

typedef struct {
    uint8_t                     addr[BT_BD_ADDR_LEN];
    uint8_t                     addr_type;
    bool                        is_edr;
    bool                        is_ull2;
    bool                        is_dongle;
} app_bt_takeover_device_t;
bool app_bt_takeover_service_get_last_takeover_device(app_bt_takeover_device_t *device);

void app_bt_takeover_service_disconnect_one(void);

uint8_t *app_bt_takeover_get_disconnect_le_addr(void);
void app_bt_takeover_clear_disconnect_le_addr(void);

void app_bt_takeover_proc_ui_shell_event(uint32_t event_group, uint32_t event_id,
                                         void *extra_data, size_t data_len);

#endif /* __APP_BT_TAKEOVER_SERVICE_H__ */
