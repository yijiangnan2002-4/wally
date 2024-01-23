/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef __APP_LEA_SERVICE_ADV_MGR_H__
#define __APP_LEA_SERVICE_ADV_MGR_H__

/**
 * File: app_lea_service_adv_mgr.h
 *
 * Description: This file defines the interface of app_lea_service_adv_mgr.c.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include "bt_type.h"
#include "app_lea_service_config.h"

#include "app_lea_service_target_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

void        app_lea_adv_mgr_start_advertising(uint8_t mode, bool sync, uint32_t timeout);
void        app_lea_adv_mgr_stop_advertising(bool sync);
void        app_lea_adv_mgr_quick_stop_adv(void);
void        app_lea_adv_mgr_refresh_advertising(uint32_t delay);
uint8_t     app_lea_adv_mgr_get_adv_mode(void);
bool        app_lea_adv_mgr_update_adv_interval(uint16_t interval_min, uint16_t interval_max);

bool        app_lea_adv_mgr_control_adv_data(uint8_t adv_type, bool enable);

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
bool        app_lea_adv_mgr_enable_ull2_pairing_mode(bool enable);

bool        app_lea_adv_mgr_enable_ull2_reconnect_mode(bool enable);
#endif

void        app_lea_adv_mgr_get_adv_info(uint8_t *mode, uint32_t *timeout);

void        app_lea_adv_mgr_init(void);

void        app_lea_adv_mgr_proc_ui_shell_event(uint32_t event_group,
                                                uint32_t event_id,
                                                void *extra_data,
                                                size_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* __APP_LEA_SERVICE_ADV_MGR_H__ */
