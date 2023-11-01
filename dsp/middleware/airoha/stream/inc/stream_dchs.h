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
#ifndef _STREAM_UART_H_
#define _STREAM_UART_H_
/******************************************************************************
 *
 * Include
 *
 ******************************************************************************/
#include "source.h"
#include "hal_audio_control.h"
#include "hal_audio.h"
#include "mux_ll_uart.h"
#include "bt_interface.h"
#include "common.h"

/******************************************************************************
 *
 * Public Macro and Variable Declaration
 *
 ******************************************************************************/
#define DCHS_DL_FIX_SAMPLE_RATE         (48000)    //48k
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
#define MUX_UART_BUF_SLICE              (1)
#else
#define MUX_UART_BUF_SLICE              (4)
#endif
#define AUDIO_DURATION_TIME             (5)
#define DCHS_DL_PLAY_EN_DELAY_MS        (30)  //ms [hard code]
#define DCHS_DL_PLAY_EN_USB_IN_DELAY_MS (30)  //ms [hard code]
#define DCHS_DL_HFP_DELAY_TIME          (7.5*3) //ms
#define USB_IN_8_CHANNEL                (8)
#define DCHS_DL_SINK_PREFILL_MS         (7) //ms
#define DCHS_DL_SINK_LEAUDIO_PREFILL_MS (15) //ms
#define DCHS_DL_SINK_THRESHOLD_MASTER   (3) //ms
#define DCHS_DL_SINK_THRESHOLD_SLAVE    (1) //ms
#define DCHS_DL_USB_IN_PREFILL_FOR_DCHS (9) //ms if = 10, 96k prefil=11520byte(10+10),usb total buf length=11520, so prefill will be 0

 typedef enum {
    HWSRC_NONE = 0,
    HWSRC_1,
    HWSRC_2,
    HWSRC_3,
    HWSRC_MAX = HWSRC_3
} hwsrc_id_t;

typedef enum
{
    LOCAL_CHIP,
    OTHER_CHIP,
    MAX_CHIP,
    MAX_VALUE = 0xFFFFFFFF,
}dchs_dl_chip_role_t;
typedef enum{
    STOP_MIX,
    WAITING_MIX,
}dchs_dl_mix_type_t;

typedef enum {
    LOCAL_SCENARIO_1, //other scneario for master
    LOCAL_SCENARIO_2, //vp scneario for master
    UART_SCENARIO,
    DCHS_DL_SINK,
    CH_MAX,
}buf_ch_type_t;


typedef struct{
    hwsrc_id_t hwsrc_id;
    bool       is_enable;
}hwsrc_control_status_t;

typedef struct{
    U8 channel_num;
    U32 frame_size;
    audio_scenario_type_t data_scenario_type;
    U32 format_bytes;
    BUFFER_INFO_PTR sink_buf_info;
    U32 sample_rate;
    hwsrc_control_status_t hwsrc_info;
    bool is_running;
    U32 prefill_size;
    U32 mix_point;
    dchs_dl_mix_type_t mix_type;
    bool waiting_set_volume;
    int32_t  vol_gain;
}dl_data_scenario_msg_t;

typedef struct
{
    audio_scenario_type_t scenario_type;
    U32 play_en_clk;
    U16 play_en_phase;
    bool waiting_to_set;
} dchs_dl_play_en_info_t;

/******************************************************************************
 * 
 * Public Function Declaration
 * 
 ******************************************************************************/
//dl func
SOURCE SourceInit_DCHS_DL(SOURCE source);
extern bool dchs_play_en_timeout_flag;
extern dl_data_scenario_msg_t dchs_dl_ch_scenario_msg[CH_MAX];
extern bool g_dchs_dl_open_done_flag;
extern dchs_dl_play_en_info_t g_dchs_dl_play_en_info;
extern uint32_t g_dchs_dl_data_mix_count;
extern uint32_t g_dchs_dl_process_count;
extern void dchs_dl_copy_uart_data_2_source_buf();

void dchs_dl_set_timer_do_sw_mix_early(SOURCE source);
void dchs_dl_source_ch_hwsrc_cfg(buf_ch_type_t ch_type, hwsrc_id_t hwsrc_id);
void dchs_dl_source_ch_hwsrc_driver_control(buf_ch_type_t ch_type, bool control);
bool dchs_dl_check_scenario_exist(buf_ch_type_t ch_type);
bool dchs_dl_check_hwsrc_enable(buf_ch_type_t ch_type);
void dchs_dl_set_scenario_exist(buf_ch_type_t ch_type, bool is_running);
void dchs_dl_update_hwsrc_input_wrpnt(buf_ch_type_t ch_type, U32 input_wo);
afe_mem_asrc_id_t dchs_dl_get_hwsrc_id(buf_ch_type_t ch_type);
void dchs_dl_set_play_en(U32 play_en_clk, U16 play_en_phase, audio_scenario_type_t data_scenario_type);
void dchs_dl_play_en_disable(hal_audio_agent_t agent);
void dchs_dl_uart_relay_play_en_info(U32 play_en_clk, U16 play_en_phase, audio_scenario_type_t data_scenario_type);
void dchs_dl_set_play_en_ms(U32 later_time_ms, audio_scenario_type_t data_scenario_type);
void dchs_dl_set_scenario_play_en_exist(hal_audio_agent_t agent, bool is_enable);
bool dchs_dl_check_scenario_play_en_exist(hal_audio_agent_t agent);
U32  dchs_get_cur_native_clk(void);
void DCHS_TransBT2NativeClk(BTCLK CurrCLK, BTPHASE CurrPhase, BTCLK *pNativeBTCLK, BTPHASE *pNativePhase, BT_CLOCK_OFFSET_SCENARIO type);
void dchs_dl_set_hfp_play_en(void);
void dchs_dl_resume_dchs_task(void);
void dchs_dl_count_mix_point(U32 play_en_clk, U16 play_en_phase, audio_scenario_type_t data_scenario_type, buf_ch_type_t ch_type);
void dchs_send_unlock_sleep_msg(bool is_dchs_dl);
void dchs_dl_uart_buf_clear();
//ul func
bool stream_function_dchs_uplink_tx_initialize(void *para);
bool stream_function_dchs_uplink_tx_process(void *para);
bool stream_function_dchs_uplink_sw_buffer_slave_initialize(void *para);
bool stream_function_dchs_uplink_sw_buffer_slave_process(void *para);
bool stream_function_dchs_uplink_sw_buffer_master_initialize(void *para);
bool stream_function_dchs_uplink_sw_buffer_master_process(void *para);
void dps_uart_relay_ul_mem_sync_info(uint32_t delay_time,S32 cur_native_bt_clk, S32 cur_native_bt_phase);
void dchs_ul_set_bt_clk(S32 play_bt_clk, S32 play_bt_phase);
#endif

