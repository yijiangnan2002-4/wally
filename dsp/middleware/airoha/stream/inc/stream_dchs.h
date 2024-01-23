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
#include "stream_mixer.h"

/******************************************************************************
 *
 * Public Macro and Variable Declaration
 *
 ******************************************************************************/
#define DCHS_DL_UART_BUF_SIZE           (20 * 1024)
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
#define MUX_UART_BUF_SLICE              (1)
#else
#define MUX_UART_BUF_SLICE              (4)
#endif
#define AUDIO_DURATION_TIME             (5)

#define DCHS_DL_SINK_PREFILL_MS         (10) //ms
#define DCHS_DL_SINK_THRESHOLD_MASTER   (7) //ms
#define DCHS_DL_SINK_THRESHOLD_SLAVE    (3) //ms
#define DCHS_DL_USB_IN_PREFILL_FOR_DCHS (9) //ms if = 10, 96k prefil=11520byte(10+10),usb total buf length=11520, so prefill will be 0

typedef struct {
    uint32_t config_operation;
    int32_t gain[8];
} ul_sw_gain_config_param_t, *ul_sw_gain_config_param_p;
/******************************************************************************
 * 
 * Public Function Declaration
 * 
 ******************************************************************************/
//dl func
extern BUFFER_INFO dl_uart_buf_info;
void dchs_dl_set_play_en(U32 play_en_clk, U16 play_en_phase);
void dchs_dl_uart_relay_play_en_info(U32 play_en_clk, U16 play_en_phase);
void dchs_dl_set_hfp_play_en(void);
void dchs_send_unlock_sleep_msg(bool is_dchs_dl);
void dchs_dl_uart_buf_clear();
void dchs_dl_uart_buf_init(source_ch_type_t ch_type);
void update_hwsrc_input_wrpnt(U8 hwsrc_id, U32 iwo);
void DCHS_TransBT2NativeClk(BTCLK CurrCLK, BTPHASE CurrPhase, BTCLK *pNativeBTCLK, BTPHASE *pNativePhase, BT_CLOCK_OFFSET_SCENARIO type);
void dchs_dl_copy_uart_data_2_source_buf();

//ul func
bool stream_function_dchs_uplink_tx_initialize(void *para);
bool stream_function_dchs_uplink_tx_process(void *para);
bool stream_function_dchs_uplink_sw_buffer_slave_initialize(void *para);
bool stream_function_dchs_uplink_sw_buffer_slave_process(void *para);
bool stream_function_dchs_uplink_sw_buffer_master_initialize(void *para);
bool stream_function_dchs_uplink_sw_buffer_master_process(void *para);
void dps_uart_relay_ul_mem_sync_info(uint32_t delay_time,S32 cur_native_bt_clk, S32 cur_native_bt_phase);
void dchs_ul_set_bt_clk(S32 play_bt_clk, S32 play_bt_phase);
void dchs_ul_sw_gain_init(SOURCE source);
void dchs_ul_sw_gain_deinit(void);
void dchs_ul_sw_gain_config(void *config_param, SOURCE source, SINK sink);
#endif

