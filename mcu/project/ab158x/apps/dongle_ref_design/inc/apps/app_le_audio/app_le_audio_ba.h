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

#ifndef __APP_LE_AUDIO_BA_H__
#define __APP_LE_AUDIO_BA_H__

#ifdef AIR_LE_AUDIO_ENABLE

#include "app_le_audio_ba_utillity.h"
#include "app_le_audio_bcst_utillity.h"
#include "ble_bap_client.h"

/**************************************************************************************************
* Define
**************************************************************************************************/

/**************************************************************************************************
* Public function
**************************************************************************************************/
void app_le_audio_ba_handle_bap_client_evt(ble_bap_event_t event, void *msg);

void app_le_audio_ba_handle_sync_transfer_cnf(bt_msg_type_t msg, void *buff);

void app_le_audio_ba_handle_connect_ind(bt_status_t ret, bt_gap_le_connection_ind_t *ind);

void app_le_audio_ba_handle_disconnect_ind(bt_status_t ret, bt_gap_le_disconnect_ind_t *ind);

void app_le_audio_ba_read_broadcast_receive_state(bt_handle_t handle);

void app_le_audio_ba_play_all(uint8_t is_sync_pa, bool is_sync_bis, uint32_t bis_index);

void app_le_audio_ba_play_by_link(uint8_t link_idx);

void app_le_audio_ba_pause_all(uint8_t is_sync_pa, bool is_sync_bis, uint32_t bis_index);

bt_addr_t app_le_audio_ba_get_pa(void);

void app_le_audio_ba_set_broadcast_code(uint8_t link_idx, uint8_t source_id,const uint8_t *param);

void app_le_audio_ba_start(uint8_t ba_mode);

void app_le_audio_ba_stop_assistant(void);

void app_le_audio_ba_init(void);

#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_BA_H__ */

