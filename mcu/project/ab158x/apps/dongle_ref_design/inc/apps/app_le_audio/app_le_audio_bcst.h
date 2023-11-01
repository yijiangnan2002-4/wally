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

#ifndef __APP_LE_AUDIO_BCST_H__
#define __APP_LE_AUDIO_BCST_H__

#ifdef AIR_LE_AUDIO_ENABLE

#include "bt_type.h"
#include "ble_bap.h"
#include "app_le_audio_bcst_utillity.h"
/**************************************************************************************************
* Define
**************************************************************************************************/

/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Public function
**************************************************************************************************/
bool app_le_audio_bcst_is_streaming(void);

void app_le_audio_bcst_start(void);

bool app_le_audio_bcst_stop(bool restart);

void app_le_audio_bcst_init(void);

void app_le_audio_bcst_open_audio_transmitter_cb(void);

void app_le_audio_bcst_close_audio_transmitter_cb(void);

void app_le_audio_bcst_handle_setup_iso_data_path_cnf(bt_status_t ret, bt_gap_le_setup_iso_data_path_cnf_t *cnf);

void app_le_audio_bcst_handle_remove_iso_data_path_cnf(bt_status_t ret, bt_gap_le_remove_iso_data_path_cnf_t *cnf);

void app_le_audio_bcst_handle_config_extended_advertising_cnf(bt_status_t ret, bt_gap_le_config_extended_advertising_cnf_t *cnf);

void app_le_audio_bcst_handle_enable_extended_advertising_cnf(bt_status_t ret, bt_gap_le_enable_extended_advertising_cnf_t *cnf);

void app_le_audio_bcst_handle_config_periodic_advertising_cnf_t(bt_status_t ret, bt_gap_le_config_periodic_advertising_cnf_t *cnf);

void app_le_audio_bcst_handle_enable_periodic_advertising_cnf(bt_status_t ret, bt_gap_le_enable_periodic_advertising_cnf_t *cnf);

void app_le_audio_bcst_handle_big_established_ind(bt_status_t ret, bt_gap_le_big_established_ind_t *ind);

void app_le_audio_bcst_handle_big_terminated_ind(bt_status_t ret, bt_gap_le_big_terminated_ind_t *ind);

void app_le_audio_bcst_reset_info_ex(void);

#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_BCST_H__ */

