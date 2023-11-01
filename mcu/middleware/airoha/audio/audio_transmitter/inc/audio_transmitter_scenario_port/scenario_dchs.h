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
#ifndef __SCENARIO_DCHS_H__
#define __SCENARIO_DCHS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef AIR_DCHS_MODE_ENABLE

/******************************************************************************
 * Include
 ******************************************************************************/

#include "audio_log.h"
#include "FreeRTOS.h"


#include "audio_nvdm_coef.h"
#include "audio_nvdm_common.h"
#include "hal_audio_message_struct.h"
#include "bt_codec.h"
#include "mux_ll_uart_latch.h"
#include "bt_system.h"

/******************************************************************************
 * Pubilc Global Variables and Macro and Enum
 ******************************************************************************/
typedef enum
{
    UART_DL,
    UART_UL,
    UART_AUDIO_CMD,
    UART_MAX,
}uart_type_t;

typedef enum
{
    LOCAL_CHIP,
    OTHER_CHIP,
    MAX_CHIP,
    CHIP_MAX_VALUE = 0xFFFFFFFF,
}dchs_dl_chip_role_t;

typedef enum
{
    AUDIO_UART_COSYS_CMD_MIN = 0,
    AUDIO_UART_COSYS_DL_OPEN,
    AUDIO_UART_COSYS_DL_START,
    AUDIO_UART_COSYS_DL_CLOSE,
    AUDIO_UART_COSYS_DL_SET_GAIN,
    AUDIO_UART_COSYS_DL_SAMPLE_RATE_SYNC = 5,
    AUDIO_UART_COSYS_UL_OPEN,
    AUDIO_UART_COSYS_UL_START,
    AUDIO_UART_COSYS_UL_CLOSE,
    AUDIO_UART_COSYS_UL_VOLUME,
    AUDIO_UART_COSYS_ANC_CTRL = 10,
    AUDIO_UART_COSYS_RACE_CMD,
    AUDIO_UART_COSYS_CMD_ACK,
    AUDIO_UART_COSYS_CMD_WAITING_ACK,
    //extend here

    AUDIO_UART_COSYS_CMD_MAX,
    AUDIO_UART_COSYS_CMD_RESERVE = 0xFFFFFFFF,
}uart_cmd_type_t;

typedef enum
{
    ACK_NO,
    ACK_NOW,
    ACK_LATER,
    ACK_WAITING,
    ACK_RECEIVED,
    ACK_WAIT_LATER,
    ACK_MAX_VALUE = 0xFFFFFFFF,
}audio_uart_cmd_ack_type_t;

typedef enum
{
    SYNC_NONE,
    SYNC_SUCCESS,
    SYNC_ERROR,
    //extern here

    ACK_CONTEXT_MAX_VALUE = 0xFFFFFFFF,
}audio_uart_cmd_ack_context_t;

typedef struct
{
    uart_cmd_type_t ctrl_type;
    uint32_t param_size;
    audio_uart_cmd_ack_type_t ack_type;
}audio_uart_cmd_header_t;

typedef struct
{
    audio_uart_cmd_header_t header;
    uart_cmd_type_t ctrl_type;
    audio_uart_cmd_ack_context_t ack_context;
} audio_dchs_ack_param_t;

typedef struct
{
    audio_uart_cmd_header_t header;
    audio_scenario_type_t scenario_type;
    bt_a2dp_codec_type_t a2dp_codec_type;
    hal_audio_memory_t in_memory;
    hal_audio_memory_t out_memory;
    hal_audio_interface_t in_interface;
    hal_audio_interface_t out_interface;
    uint32_t sampling_rate;
    uint32_t frame_size; //samples
    uint32_t  context_type;
    uint8_t frame_number;
    hal_audio_format_t format;
    uint8_t  irq_period;
    uint8_t  channel_num;
    dchs_dl_chip_role_t chip_role;
} audio_dchs_dl_open_param_t;

typedef struct
{
    audio_uart_cmd_header_t header;
    audio_scenario_type_t scenario_type;
    dchs_dl_chip_role_t   chip_role;
    uint32_t  context_type;
} audio_dchs_dl_start_param_t;

typedef struct
{
    audio_uart_cmd_header_t header;
    uint32_t operation;
    int32_t  vol_gain;
} audio_dchs_dl_gain_param_t;

typedef struct
{
    audio_uart_cmd_header_t header;
    audio_scenario_type_t scenario_type;
    dchs_dl_chip_role_t   chip_role;
    uint32_t  context_type;
} audio_dchs_dl_close_param_t;

typedef struct
{
    audio_uart_cmd_header_t header;
    audio_scenario_type_t scenario_type;
    uint32_t sampling_rate;
    uint32_t frame_size;
    uint8_t frame_number;
    hal_audio_format_t format;
    uint8_t  irq_period;
} audio_dchs_ul_open_param_t;
typedef struct
{
    audio_uart_cmd_header_t header;
    audio_scenario_type_t scenario_type;
} audio_dchs_ul_start_param_t;
typedef struct {
    audio_uart_cmd_header_t header;
    audio_scenario_type_t scenario_type;
} audio_dchs_ul_close_param_t;
typedef struct {
    audio_uart_cmd_header_t header;
    audio_scenario_type_t scenario_type;
    uint8_t codec;
    uint16_t dev_in;
    uint8_t lev_in;
    uint16_t dev_out;
    uint8_t lev_out;
} audio_dchs_ul_volume_param_t;

typedef struct {
    audio_uart_cmd_header_t header;
    uint8_t channel_id;
    bool is_ack_cmd;
    uint16_t payload_length;
    uint8_t payload[16];    //Reserved 16bytes for the race command
} audio_dchs_relay_cmd_param_t;

typedef struct
{
    audio_uart_cmd_header_t header;
    U32 dchs_dl_sample_rate;
}audio_dchs_dl_sample_rate_param_t;

typedef struct
{
    audio_uart_cmd_header_t header;
    uart_cmd_type_t cmd_type;
}audio_dchs_wait_ack_param_t;

typedef union {
    audio_dchs_dl_open_param_t  dchs_dl_open_param;
    audio_dchs_dl_start_param_t dchs_dl_start_param;
    audio_dchs_dl_close_param_t dchs_dl_close_param;
    audio_dchs_dl_gain_param_t  dchs_dl_gain_param;
    // ul param add here
    audio_dchs_ul_open_param_t  dchs_ul_open_param;
    audio_dchs_ul_start_param_t dchs_ul_start_param;
    audio_dchs_ul_close_param_t dchs_ul_close_param;
    audio_dchs_ul_volume_param_t dchs_ul_volume_param;

    audio_dchs_relay_cmd_param_t dchs_relay_cmd_param;
    audio_dchs_wait_ack_param_t  dchs_wait_ack_param;
}audio_dchs_cosys_ctrl_param_t;

typedef struct
{
    uart_cmd_type_t ctrl_type;
    audio_dchs_cosys_ctrl_param_t ctrl_param;
    bool is_send_am_front;
}audio_dchs_cosys_ctrl_t;


/******************************************************************************
 * Pubilc Function Declaration
 ******************************************************************************/
extern bt_clock_t g_dchs_anc_target_clk;

extern void mcu_uart_open(uart_type_t type);
extern void mcu_uart_tx(uart_type_t type, uint8_t *param_buf, uint32_t buf_size);
extern void mcu_uart_init(void);

extern void dchs_cosys_ctrl_cmd_relay(uart_cmd_type_t ctrl_type, audio_scenario_type_t scenario_type, mcu2dsp_open_param_t *open_param, mcu2dsp_start_param_t * start_param);
extern void dchs_cosys_ctrl_cmd_execute(audio_dchs_cosys_ctrl_t * cosys_ctrl);

extern void    dchs_dl_set_scenario_exist_flag(dchs_dl_chip_role_t chip_role, audio_scenario_type_t data_scenario_type, bool is_running);
extern uint8_t dchs_dl_check_scenario_exist_flag(dchs_dl_chip_role_t chip_role);
extern void dchs_dl_set_audio_sample_rate(uint32_t sample_rate);
void dchs_lock_bt_sleep(bool is_dchs_dl);
#endif

#ifdef __cplusplus
}
#endif

#endif/*__SCENARIO_DCHS_H__*/