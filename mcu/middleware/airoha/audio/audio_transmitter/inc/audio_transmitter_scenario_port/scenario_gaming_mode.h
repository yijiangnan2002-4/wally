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
#ifndef __SCENARIO_GAMING_MODE_H__
#define __SCENARIO_GAMING_MODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)

#include "audio_log.h"
#include "FreeRTOS.h"
#include "bt_sink_srv_ami.h"

#include "audio_nvdm_coef.h"
#include "audio_nvdm_common.h"
#include "hal_audio_message_struct.h"

extern void gaming_mode_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param);
extern void gaming_mode_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param);
extern audio_transmitter_status_t gaming_mode_set_runtime_config_playback(audio_transmitter_config_t *config, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config, mcu2dsp_audio_transmitter_runtime_config_param_t *runtime_config_param);
extern audio_transmitter_status_t gaming_mode_get_runtime_config(uint8_t scenario_type, uint8_t scenario_sub_id, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config);
extern void gaming_mode_state_started_handler(uint8_t scenario_sub_id);
extern void gaming_mode_state_idle_handler(uint8_t scenario_sub_id);
extern void gaming_mode_state_starting_handler(uint8_t scenario_sub_id);
extern void gaming_mode_state_stoping_handler(uint8_t scenario_sub_id);
#endif

#ifdef __cplusplus
}
#endif

#endif/*__SCENARIO_GAMING_MODE_H__*/
