/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#ifndef __APP_DONGLE_RACE_H__
#define __APP_DONGLE_RACE_H__


#ifdef MTK_RACE_CMD_ENABLE
#include <stdint.h>
#include <stdbool.h>
#include "race_cmd.h"



enum {
    APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_MUTE_MIC,
    APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_RECORD,
    APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_BATTERY,
    APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_TX_VOLUME,
    APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_INVALID = 0xFF,
};
typedef uint8_t app_ull_dongle_race_remote_control_type_t;

#if (defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "bt_ull_service.h"

void app_dongle_race_ull_connect_proc(bool connected, uint16_t conn_handle);

void apps_dongle_race_cmd_on_remote_control_state_change(app_ull_dongle_race_remote_control_type_t control_type, uint8_t state, uint8_t channel_id);

#endif

bool app_dongle_race_cmd_event_proc(uint32_t event_id, void *extra_data, size_t data_len);

bool app_dongle_race_bt_event_proc(uint32_t event_id, void *extra_data, size_t data_len);

void *apps_dongle_race_cmd_handler(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id);

#endif /* #ifdef MTK_RACE_CMD_ENABLE */

#endif /* __APP_DONGLE_RACE_H__ */
