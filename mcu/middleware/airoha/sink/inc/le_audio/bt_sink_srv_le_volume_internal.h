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


#ifndef __LE_SINK_SRV_VOLUME_INTERNAL_H__
#define __LE_SINK_SRV_VOLUME_INTERNAL_H__

#include "bt_type.h"
#include "bt_sink_srv_le_cap_audio_manager.h"

#define BT_SINK_LE_MIC_VOLUME_ADJUST_ENABLE 0

bt_status_t bt_sink_srv_le_volume_init(void);

bt_status_t bt_sink_srv_le_volume_vcp_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);

bt_status_t bt_sink_srv_le_volume_micp_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);

bt_status_t bt_sink_srv_le_volume_set_volume(bt_sink_srv_le_stream_type_t type, uint8_t volume);

bt_sink_srv_le_volume_state_t bt_sink_srv_le_volume_get_state(bt_handle_t handle, bt_sink_srv_cap_am_mode mode);

bt_sink_srv_le_volume_state_t bt_sink_srv_le_volume_get_mic_volume_state(bt_handle_t handle);

bt_status_t bt_sink_srv_le_volume_set_mute_and_volume_level(int8_t aid, bt_sink_srv_le_stream_type_t type, bool mute, uint8_t vol_level);

void bt_sink_srv_le_volume_reset_mic_volume_state(uint8_t link_idx);

uint8_t bt_sink_srv_le_mapping_am_volume(uint8_t vcs_volume, bt_sink_srv_cap_am_mode mode);

#endif  /* __LE_SINK_SRV_VOLUME_INTERNAL_H__ */

