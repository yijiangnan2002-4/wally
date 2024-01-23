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
#ifndef _STREAM_MIXER_H_
#define _STREAM_MIXER_H_
/******************************************************************************
 *
 * Include
 *
 ******************************************************************************/
#include "source.h"
#include "hal_audio_control.h"
#include "hal_audio.h"
#include "bt_interface.h"
#include "common.h"
#include "dsp_scenario.h"
#include "dsp_play_en.h"

/******************************************************************************
 *
 * Public Macro and Variable Declaration
 *
 ******************************************************************************/
#define MAX_MIXER_NUM                   (3)
#define PLAY_DELAY_MS                   (20)
#define MIXER_STREAM_OUT_CHANNEL        (2)
#define MAX_SCENARIO_NUM                (10)

#ifdef AIR_DCHS_MODE_ENABLE
#define STREAM_PERIOD_MS                (MUX_UART_BUF_SLICE == 4 ? 1.25 : 5) //ms
#define PRE_PROCESS_COUNT               (MUX_UART_BUF_SLICE)
#define MIXER_SINK_PREFILL_MS           (10) //ms
#else
#define STREAM_PERIOD_MS                (5) //ms
#define PRE_PROCESS_COUNT               (1)
#define MIXER_SINK_PREFILL_MS           (5) //ms
#endif

typedef enum{
    NO_MIX,
    WAITING_MIX,
    IS_RUNNING,
    WAITING_COUNT_MIX,
}mix_type_t;

typedef enum{
    SCENARIO_HWSRC, //scenario self already have hwsrc.
    MIXER_ADD_HWSRC, //mixer stream new add hwsrc.
}hwsrc_enable_type_t;

typedef enum {
    MIX_SCENARIO_1,
    MIX_SCENARIO_2,
    MIX_SCENARIO_3,
    MIX_SCENARIO_4,
    MIX_SCENARIO_5,
    MIX_SCENARIO_MAX = MAX_MIXER_NUM,
}source_ch_type_t;

typedef enum
{
    SYNC_SCENARIO_OPEN,
    SYNC_SCENARIO_CLOSE,
    SYNC_SET_GAIN_VALUE,
    //extend here
    SYNC_TYPE_MAX = 0xFFFFFFFF,
}mcu2dsp_sync_type_t;

typedef struct
{
    mcu2dsp_sync_type_t sync_type;
    audio_scenario_type_t scenario_type;
    hal_audio_memory_t memory_agent;
    int32_t  vol_gain;
    U8 format_bytes;
    U8 clkskew_mode;
    bool need_play_en;
    U32 bt_clk;
    U16 bt_phase;
    U32 sample_rate;
    U8  channel_num;
}sceanrio_mcu2dsp_sync_msg_t;

typedef struct{
    bool hwsrc_enable;
    U8 hwsrc_id;
    U8 format_bytes;
    U8 channel_num;
    U8 pre_process_count;
    audio_scenario_type_t scenario_type;
    BUFFER_INFO_PTR sink_buf_info;
    U32 mix_point;
    mix_type_t mix_type;
    hal_audio_memory_t memory_agent;
    U32 prefill_silence_count;
    U32 sample_rate;
    hwsrc_enable_type_t hwsrc_enable_type;
}mixer_scenario_msg_t;

/******************************************************************************
 * 
 * Public Function Declaration
 * 
 ******************************************************************************/
extern U32 g_mixer_stream_mix_count;
extern U32 g_mixer_stream_process_count;
extern U32 g_mixer_stream_timer_handle;
extern mixer_scenario_msg_t  mix_scenarios_msg[MIX_SCENARIO_MAX];

SOURCE dsp_open_stream_in_mixer(mcu2dsp_open_param_p open_param);
void dsp_mixer_stream_open(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_mixer_stream_start(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_mixer_stream_stop(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_mixer_stream_close(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_start_stream_in_mixer(mcu2dsp_start_param_p start_param, SOURCE source);
void mixer_stream_play_en_disable(hal_audio_agent_t agent);
void mixer_stream_mcu2dsp_msg_sync_callback(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void mixer_stream_trigger_debug_timer(SINK_TYPE sink_type);
bool mixer_stream_check_waiting_mix(void);
void mixer_stream_setup_play_en(U32 bt_clk, U16 bt_phase, SOURCE source, audio_scenario_type_t scenario_type);
audio_scenario_type_t mixer_stream_get_scenario_type_by_channel(source_ch_type_t ch_type);
source_ch_type_t mixer_stream_get_source_ch_by_agent(hal_audio_memory_t memory_agent);
U32 mixer_get_ch_data_size(source_ch_type_t ch_type);
void update_hwsrc_input_wrpnt(U8 hwsrc_id, U32 iwo);
void mixer_stream_source_ch_hwsrc_cfg(source_ch_type_t ch_type, afe_mem_asrc_id_t hwsrc_id);
source_ch_type_t mixer_stream_get_source_ch_by_scenario(audio_scenario_type_t scenario_type);
void mixer_stream_source_ch_hwsrc_driver_control(source_ch_type_t ch_type, bool control);

#endif