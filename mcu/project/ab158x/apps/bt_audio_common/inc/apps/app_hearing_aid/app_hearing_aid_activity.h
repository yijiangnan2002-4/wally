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

#ifndef __APP_HEARING_AID_ACTIVITY_H__
#define __APP_HEARING_AID_ACTIVITY_H__

#include "ui_shell_activity.h"
#include "stdint.h"

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)

#include "app_hearing_aid_aws.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool app_hearing_aid_activity_is_out_case();

void app_hearing_aid_activity_play_vp(uint8_t vp_index, bool need_sync);

void app_hearing_aid_activity_play_ha_on_vp(bool enable, bool need_mode_vp, bool need_sync_play);

void app_hearing_aid_activity_play_mode_index_vp(uint8_t index, bool need_sync);

void app_hearing_aid_activity_pre_proc_operate_ha(uint8_t which, bool on, bool need_aws_sync);

bool app_hearing_aid_activity_process_race_cmd(void *race_data, size_t race_data_len);

void app_hearing_aid_activity_handle_get_race_cmd(uint8_t *race_data, uint16_t race_data_len, uint8_t *get_response, uint16_t *get_response_len);

bool app_hearing_aid_activity_handle_set_race_cmd(uint8_t *race_data, uint16_t race_data_len);

bool app_hearing_aid_activity_open_hearing_aid_fwk();

void app_hearing_aid_activity_open_hearing_aid_fwk_with_zero_path();

void app_hearing_aid_activity_set_open_fwk_done(bool result);

void app_hearing_aid_activity_set_need_play_locally(bool play_locally);

bool app_hearing_aid_activity_is_open_fwk_done();

bool app_hearing_aid_activity_is_fwk_opening();

bool app_hearing_aid_activity_is_sco_ongoing();

bool app_hearing_aid_activity_is_need_reopen_fwk();

bool app_hearing_aid_activity_enable_hearing_aid(bool from_key);

void app_hearing_aid_activity_set_power_on_played();

bool app_hearing_aid_activity_disable_hearing_aid(bool need_vp);

bool app_hearing_aid_activity_is_hearing_aid_on();

bool app_hearing_aid_activity_set_user_switch(bool need_sync, bool enable);

bool app_hearing_aid_activity_operate_ha(bool trigger_from_key, uint8_t which, bool mix_table_need_execute, bool is_origin_on, bool mix_table_to_enable, bool drc_to_enable);

void app_hearing_aid_activity_proc_vp_streaming_state_change(bool streaming);

bool app_hearing_aid_is_mp_test_mode();

bool app_hearing_aid_is_need_enable_ha();

bool app_hearing_aid_is_ready_to_enable_side_tone();

bool app_hearing_aid_is_supported_cmd(uint16_t cmd_type);

bool app_hearing_aid_activity_proc(ui_shell_activity_t *self,
                                   uint32_t event_group,
                                   uint32_t event_id,
                                   void *extra_data,
                                   size_t data_len);

#ifdef AIR_TWS_ENABLE
void app_hearing_aid_activity_handle_app_info_sync(uint8_t *data, uint32_t data_len);

void app_hearing_aid_activity_set_powering_off();

void app_hearing_aid_activity_set_fbd_directly();

void app_hearing_aid_activity_handle_rssi_operation(int8_t rssi);
#endif /* AIR_TWS_ENABLE */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

#endif /* __APP_HEARING_AID_ACTIVITY_H__ */

