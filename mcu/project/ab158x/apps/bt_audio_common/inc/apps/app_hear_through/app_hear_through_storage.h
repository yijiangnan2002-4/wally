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

#ifndef __APP_HEAR_THROUGH_STORAGE_H__
#define __APP_HEAR_THROUGH_STORAGE_H__

#include "stdint.h"
#include "stdbool.h"

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool app_hear_through_storage_read_data(uint16_t nvkey_id, uint8_t expected_len, uint8_t *data_to_fill_in);

void app_hear_through_storage_load_const_configuration();

uint32_t app_hear_through_storage_get_ha_ble_advertising_timeout();

uint32_t app_hear_through_storage_get_hear_through_turn_on_after_boot_up_timeout();

uint32_t app_hear_through_storage_get_ha_in_ear_det_turn_on_delay_time();

int8_t app_hear_through_storage_get_ha_rssi_threshold();

uint32_t app_hear_through_storage_get_ha_rssi_power_off_timeout();

bool app_hear_through_storage_get_ha_mode_on_vp_switch();

bool app_hear_through_storage_get_ha_level_up_circular_max_vp_switch();

bool app_hear_through_storage_get_ha_mode_up_circular_max_vp_switch();

bool app_hear_through_storage_get_ha_vol_up_circular_max_vp_switch();

uint8_t app_hear_through_storage_get_anc_mode_with_vivid_pt();

uint8_t app_hear_through_storage_get_anc_mode_with_ha_psap();

uint32_t app_hear_through_storage_get_anc_path_mask_for_ha_psap();

uint32_t app_hear_through_storage_get_anc_path_mask_for_vivid_pt();

typedef enum {
    APP_HEAR_THROUGH_MODE_VIVID_PT      = 0,
    APP_HEAR_THROUGH_MODE_HA_PSAP       = 1,
} app_hear_through_mode_t;

void app_hear_through_storage_load_user_configuration();

app_hear_through_mode_t app_hear_through_storage_get_hear_through_mode();

bool app_hear_through_storage_set_hear_through_mode(app_hear_through_mode_t new_mode);

bool app_hear_through_storage_get_hear_through_switch();

bool app_hear_through_storage_set_hear_through_switch(bool enable);

bool app_hear_through_storage_save_user_configuration();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#endif /* __APP_HEAR_THROUGH_STORAGE_H__ */
