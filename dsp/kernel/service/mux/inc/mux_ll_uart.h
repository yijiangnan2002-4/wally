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

#ifndef __MUX_LL_UART_H__
#define __MUX_LL_UART_H__
#include <stdint.h>
#include <stdbool.h>
#include "mux_ringbuffer.h"
#include "hal_gpt.h"
#include "bt_types.h"
extern void MCE_GetBtClk(BTCLK *pCurrCLK, BTPHASE *pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);

#define NATIVE_CLK_PHASE_2_US(NativeClk, NativePhase) (((NativeClk) * 625 + (NativePhase)) >> 1)

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ uint32_t mux_get_gpt_tick_count(void)
{
    uint32_t count;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count);
    return count;
}

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ uint32_t mux_get_tick_elapse_with_end(uint32_t start_count, uint32_t end_count)
{
    uint32_t duration_count;
    hal_gpt_get_duration_count(start_count, end_count, &duration_count);
    return duration_count;
}

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ uint32_t mux_get_tick_elapse(uint32_t start_count)
{
    uint32_t end_count;
    uint32_t duration_count;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &end_count);
    hal_gpt_get_duration_count(start_count, end_count, &duration_count);
    return duration_count;
}

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ uint32_t mux_get_bt_clock_count(void)
{
    BTCLK clk;
    BTPHASE phase;
    MCE_GetBtClk(&clk, &phase, DCHS_CLK_Offset);
    return NATIVE_CLK_PHASE_2_US(clk, phase);
}

typedef enum {
    DCHS_MODE_ERROR, /* Not sync done */
    DCHS_MODE_LEFT,
    DCHS_MODE_RIGHT,
    DCHS_MODE_SINGLE,
} dchs_mode_t;

#define MUX_LL_PORT_2_UART_PORT(port) (is_mux_ll_port(port) ? (port - MUX_LL_UART_0) : port)

typedef struct {
    uint32_t tx_pkt_len; //package length to UART VFIFO
    uint8_t uid;
    uint8_t tx_priority;
    uint8_t flags;
} mux_user_attr_t;

#define MUX_LL_USER_RX_DATA_CNT_MAX 8
typedef struct {
    uint32_t wptr;
    uint32_t rptr;
    uint32_t is_empty;
    uint8_t *p_data[MUX_LL_USER_RX_DATA_CNT_MAX];
    uint32_t data_size[MUX_LL_USER_RX_DATA_CNT_MAX];
    uint32_t drop_count;
} buffer_fifo_t;

typedef struct {
    uint32_t start_timestamp;
    uint32_t end_timestamp;
} mux_ll_user_timestamp_t;

typedef struct {
    uint32_t timestamp;
    uint32_t status;
} mux_ll_user_totify_t;

typedef struct {
    mux_callback_t entry;
    void *user_data;
} mux_ll_callback_t;

#define MUX_LL_UART_MAX_USER_NAME 16
#define MUX_LL_TIMESTAMP_BUFFER_MAX 8
#define MUX_LL_NOTIFY_BUFFER_MAX 8
typedef struct {
    char user_name[MUX_LL_UART_MAX_USER_NAME];
    mux_ringbuffer_t txbuf;
    mux_ringbuffer_t rxbuf;
    mux_user_attr_t uattr;
    buffer_fifo_t buffer_fifo;
    mux_ll_callback_t callback[2];
    mux_ll_user_timestamp_t callback_timestamp[MUX_LL_TIMESTAMP_BUFFER_MAX];
    hal_core_id_t tx_scid;
    hal_core_id_t rx_dcid;
} mux_ll_user_config_t;

typedef struct {
    uint32_t config_head;
    uint32_t user_count;
    uint16_t uart_tx_threshold;
    uint16_t uart_rx_threshold;
    uint32_t tx_pkt_head_tail_len;
    dchs_mode_t device_mode;
    uint32_t notify_counter;
    mux_ll_user_totify_t dsp_notify[MUX_LL_NOTIFY_BUFFER_MAX];
    mux_ll_user_config_t user_config[0];//user count is maintained by mcu.
} mux_ll_config_t;

extern mux_status_t mux_rx_ll(mux_handle_t handle, mux_buffer_t *buffer, uint32_t *receive_done_data_len);
extern mux_status_t mux_peek_ll(mux_handle_t handle, mux_buffer_t *buffer, uint32_t *receive_done_data_len);
extern mux_status_t mux_tx_ll(mux_handle_t handle, const mux_buffer_t buffers[], uint32_t buffers_counter, uint32_t *send_done_data_len);
extern mux_status_t mux_open_ll(mux_port_t port, const char *user_name, mux_handle_t *p_handle, mux_callback_t callback, void *user_data);
extern mux_status_t mux_close_ll(mux_handle_t handle);
extern mux_status_t mux_control_ll(mux_handle_t handle, mux_ctrl_cmd_t command, mux_ctrl_para_t *para);
extern void mux_hexdump_ll(mux_handle_t handle, const char* funcname, bool is_dump_data, bool is_txbuff);
extern mux_status_t mux_query_ll_user_handle(mux_port_t port, const char *user_name, mux_handle_t *p_handle);
extern mux_ringbuffer_t* mux_query_ll_user_buffer(mux_handle_t handle, bool is_rx);
extern void mux_clear_ll_user_buffer(mux_handle_t handle, bool is_rx);

extern dchs_mode_t dchs_get_device_mode(void);

#endif
