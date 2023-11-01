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
 * File: app_gsound_battery_ohd.h
 *
 * Description: This file defines the interface of app_gsound_battery_ohd.c.
 */

#ifndef __APP_GSOUND_BATTERY_OHD_H__
#define __APP_GSOUND_BATTERY_OHD_H__

#ifdef AIR_GSOUND_ENABLE

#include "gsound_api.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    APP_GSOUND_BATTERY_INFO_LOCAL_PERCENT,
    APP_GSOUND_BATTERY_INFO_LOCAL_IS_CHARGING,
    APP_GSOUND_BATTERY_INFO_PEER_PERCENT,
    APP_GSOUND_BATTERY_INFO_PEER_IS_CHARGING,
    APP_GSOUND_BATTERY_INFO_CASE_PERCENT,
    APP_GSOUND_BATTERY_INFO_CASE_IS_CHARGING
};

void app_gsound_battery_set_info(uint8_t info_type, uint8_t data);

uint8_t app_gsound_battery_get_info(uint8_t info_type);

void app_gsound_battery_handle_peer_battery(uint8_t peer_data);

void app_gsound_battery_init(void);

gsound_ohd_state app_gsound_ohd_get_state();

#ifdef __cplusplus
}
#endif

#endif /* AIR_GSOUND_ENABLE */

#endif /* __APP_GSOUND_BATTERY_OHD_H__ */

