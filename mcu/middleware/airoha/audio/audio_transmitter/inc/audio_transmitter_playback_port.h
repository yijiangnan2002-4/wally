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
#ifndef __AUDIO_TRANSMITTER_PLAYBACK_PORT_H__
#define __AUDIO_TRANSMITTER_PLAYBACK_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "hal_audio_message_struct.h"
#include "audio_transmitter_control.h"
#include "audio_transmitter_control_port.h"
#include "audio_transmitter_internal.h"

#include "scenario_gsensor.h"
#include "scenario_multi_mic_stream.h"
#include "scenario_gaming_mode.h"
#include "scenario_anc_monitor_stream.h"
#include "scenario_test.h"
#include "scenario_tdm.h"
#include "scenario_wired_audio.h"
#include "scenario_ble_audio.h"
#include "scenario_audio_hw_loopback.h"
#include "audio_transmitter_internal.h"
#include "scenario_adaptive_eq_monitor_stream.h"
#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE
#include "scenario_advanced_passthrough.h"
#endif
#include "scenario_ull_audio_v2.h"
#include "scenario_wireless_mic_rx.h"
#include "scenario_dchs.h"
#include "scenario_advanced_record.h"

typedef void (*open_playback_t)(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param);
typedef void (*start_playback_t)(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param);
typedef audio_transmitter_status_t (*set_runtime_config_playback_t)(audio_transmitter_config_t *config, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config, mcu2dsp_audio_transmitter_runtime_config_param_t *runtime_config_param);

typedef struct {
    open_playback_t open_playback;
    start_playback_t start_playback;
    set_runtime_config_playback_t set_runtime_config_playback;
} audio_transmitter_playback_func_t;

#define BLK_HEADER_SIZE sizeof(audio_transmitter_block_header_t)

extern audio_transmitter_playback_func_t audio_transmitter_playback_func[AUDIO_TRANSMITTER_SCENARIO_TYPE_MAX];

typedef struct {
    audio_transmitter_scenario_type_t scenario_type;
    uint8_t scenario_sub_id;
    audio_scenario_type_t clock_setting_type;
} audio_transmitter_clock_setting_type_t;
extern audio_transmitter_clock_setting_type_t audio_transmitter_clock_setting_type[];
extern uint16_t audio_transmitter_clock_setting_type_count;

extern void audio_transmitter_reset_share_info_by_block(n9_dsp_share_info_t *p_share_info, uint32_t buffer_start_address, uint32_t buffer_length, uint32_t max_payload_size);
extern void audio_transmitter_modify_share_info_by_block(n9_dsp_share_info_t *p_share_info, uint32_t max_payload_size);

typedef audio_transmitter_status_t (*read_data_handler_t)(uint32_t scenario_sub_id, uint8_t *data, uint32_t *length);
typedef audio_transmitter_status_t (*write_data_handler_t)(uint32_t scenario_sub_id, uint8_t *data, uint32_t *length);
typedef struct {
    read_data_handler_t  read_data_handler;
    write_data_handler_t write_data_handler;
} audio_transmitter_read_write_handler_t;
extern audio_transmitter_read_write_handler_t audio_transmitter_read_write_handler[];


#ifdef __cplusplus
}
#endif

#endif/*__AUDIO_TRANSMITTER_PLAYBACK_PORT_H__*/
