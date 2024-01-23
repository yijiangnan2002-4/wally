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
#ifndef __DSP_MUX_UART__
#define __DSP_MUX_UART__

/******************************************************************************
 * Include
 ******************************************************************************/
#include "mux.h"
#include "common.h"

/******************************************************************************
 * Pubilc Global Variables and Macro and Enum
 ******************************************************************************/
extern uint32_t g_dchs_dl_data_mix_count;
extern uint32_t g_dchs_dl_mix_point;
extern bool     g_dchs_dl_dual_mix_flag;

typedef enum{
    UART_DL,
    UART_UL,
    UART_CMD,
    UART_TYPE_MAX,
}uart_type_t;

typedef enum
{
    AUDIO_DCHS_CMD_NONE,
    AUDIO_DCHS_DL_MIX_POINT,
    AUDIO_DCHS_DL_PLAY_EN_INFO,
    AUDIO_DCHS_UL_MEM_SYNC_INFO,
    AUDIO_DCHS_DL_UART_SCENARIO_PREFILL_SIZE,
    //UL cmd type extend here
    AUDIO_DCHS_CMD_MAX = 0xFFFFFFFF,
}uart_cmd_type_t;

typedef struct
{
    uart_cmd_type_t cmd_type;
    uint32_t param_size;
}uart_cmd_header_t;

typedef struct
{
    U32 play_en_clk;
    U16 play_en_phase;
    U16 prefill_size;
    U8  channel_num;
    //dl extend here
} dchs_dl_cmd_param_t;

typedef struct
{
    S32 play_bt_clk;
    S32 play_bt_phase;
} dchs_ul_cmd_param_t;

typedef union{
    dchs_dl_cmd_param_t dchs_dl_param;
    dchs_ul_cmd_param_t dchs_ul_param;
}audio_dchs_cmd_param_t;

typedef struct
{
    uart_cmd_header_t header;
    audio_dchs_cmd_param_t cmd_param;
}audio_dchs_dsp2dsp_cmd_t;

/******************************************************************************
 * Pubilc Function Declarations
 ******************************************************************************/
void dsp_uart_rx_buffer_register(uart_type_t type, BUFFER_INFO_PTR rx_buffer);
void dsp_uart_open(uart_type_t type);
void dsp_uart_tx(uart_type_t type, uint8_t *user_tx_buffer, uint32_t buf_size);
uint32_t dsp_query_uart_rx_buf_remain_size(uart_type_t type);
void dsp_uart_rx(uart_type_t type, U8 * user_rx_buf,U32 buf_size);
mux_handle_t dsp_get_uart_handle(uart_type_t type);
void dsp_uart_ul_open(void);
void dsp_uart_ul_clear_tx_buffer(void);
void dsp_uart_ul_clear_rx_buffer(void);
#endif //__DSP_MUX_UART__
