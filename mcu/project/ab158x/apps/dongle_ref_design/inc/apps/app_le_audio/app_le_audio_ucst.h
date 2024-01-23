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

#ifndef __APP_LE_AUDIO_UCST_H__
#define __APP_LE_AUDIO_UCST_H__

#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_gattc.h"
#include "bt_gap_le.h"
#include "bt_gap_le_audio.h"
#include "app_le_audio_csip_set_coordinator.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
    /* Uncomment to support mixing the uplinks of different peer devices. */
#define APP_LE_AUDIO_UCST_UPLINK_MIX_ENABLE

#define APP_LE_AUDIO_UCST_CIS_MAX_NUM       2

    /* ASE location number */
#define APP_LE_AUDIO_UCST_LOCATION_NUM_1    1
#define APP_LE_AUDIO_UCST_LOCATION_NUM_2    2

#define APP_LE_AUDIO_UCST_MAX_CCID_LIST_SIZE 2

    /* ASE releated */
#define APP_LE_AUDIO_UCST_ASE_IDX_0         0
#define APP_LE_AUDIO_UCST_ASE_IDX_1         1
#define APP_LE_AUDIO_UCST_ASE_IDX_2         2
#define APP_LE_AUDIO_UCST_ASE_IDX_3         3
#define APP_LE_AUDIO_UCST_ASE_IDX_4         4
#define APP_LE_AUDIO_UCST_ASE_IDX_5         5
#define APP_LE_AUDIO_UCST_ASE_MAX_NUM       6

#define AIR_LE_AUDIO_AIRD_MIC_VOLUME_CONTROL_ENABLE 0 /*Feature option for mic volume control on earphone*/


/**************************************************************************************************
* Public function
**************************************************************************************************/

bool app_le_audio_ucst_is_streaming(void);

void app_le_audio_ucst_start(void);

bool app_le_audio_ucst_stop(bool restart);

bool app_le_audio_ucst_pause_ex(app_le_audio_ucst_pause_stream_t mode);

void app_le_audio_ucst_pause(void);

void app_le_audio_ucst_resume_ex(app_le_audio_ucst_pause_stream_t mode);

void app_le_audio_ucst_resume(void);

bt_status_t app_le_audio_ucst_create_cis(void);

void app_le_audio_ucst_init(void);

bool app_le_audio_ucst_check_pause_stream(void);

void app_le_audio_ucst_open_audio_transmitter_cb(void);

void app_le_audio_ucst_close_audio_transmitter_cb(void);

void app_le_audio_ucst_delete_device(bt_addr_t *addr);

bt_status_t app_le_audio_ucst_config_codec(bt_handle_t handle);

bt_status_t app_le_audio_ucst_update_metadata(bt_handle_t handle);

bt_status_t app_le_audio_ucst_release_ase(bt_handle_t handle);

bt_status_t app_le_audio_ucst_set_receiver_start_ready(bt_handle_t handle);

bt_status_t app_le_audio_ucst_set_receiver_stop_ready(bt_handle_t handle);

bt_status_t app_le_audio_ucst_disable_ase(bt_handle_t handle);

void app_le_audio_ucst_set_mic_channel(void);

uint8_t app_le_audio_ucst_get_streaming_port(void);

uint8_t app_le_audio_ucst_get_max_link_num(void);
uint8_t app_le_audio_ucst_get_link_num_ex(void);
uint8_t app_le_audio_ucst_get_group_link_num_ex(uint8_t group_id);

void app_le_audio_ucst_handle_scan_cnf(bt_status_t ret);

void app_le_audio_ucst_handle_set_white_list_cnf(bt_status_t ret);

void app_le_audio_ucst_handle_bonding_complete_ind(bt_status_t ret, bt_gap_le_bonding_complete_ind_t *ind);

void app_le_audio_ucst_handle_exchange_mtu_rsp(bt_status_t ret, bt_gatt_exchange_mtu_rsp_t *rsp);

void app_le_audio_ucst_handle_set_cig_parameter_cnf(bt_status_t ret, bt_gap_le_set_cig_params_cnf_t *cnf);

void app_le_audio_ucst_handle_cis_established_ind(bt_status_t ret, bt_gap_le_cis_established_ind_t *ind);

void app_le_audio_ucst_handle_setup_iso_data_path_cnf(bt_status_t ret, bt_gap_le_setup_iso_data_path_cnf_t *cnf);

void app_le_audio_ucst_handle_cis_terminated_ind(bt_status_t ret, bt_gap_le_cis_terminated_ind_t *ind);

void app_le_audio_ucst_handle_remove_cig_cnf(bt_status_t ret, bt_gap_le_remove_cig_cnf_t *cnf);

void app_le_audio_ucst_reset_param(void);

void app_le_audio_ucst_reset_all_cis_info(void);

uint8_t app_le_audio_ucst_get_cis_num(void);

uint16_t app_le_audio_ucst_get_audio_context_type(void);

app_le_audio_ase_codec_t * app_le_audio_ucst_get_ase_codec_config(uint16_t context_type, bt_le_audio_direction_t direction);

bool app_le_audio_ucst_check_close_audio_stream(void);

#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_UCST_H__ */

