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
#include "mux_ll_uart_config.h"
#include "mux_ll_uart_latch.h"
#include "mux_ll_uart_sync.h"
#include "mux_ll_uart_user_config.h"
#include "mux_ringbuffer.h"
#include "hal_gpt.h"
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


extern void DCHS_bt_pka_get_native_clk(uint32_t *NativeClk, uint16_t *NativePhase);

// phase: 0~2499, 12-bit, unit is 0.5us
// clk:0~(0xFFFFFFE), 28-bit, unit is 312.5us. NativeClk & 0xFFFFFFFC;lsb 2bit must be 0
// phase from 0 to 2499, add 1, then, phase=0, clk[27:2]+=1
//

#define NATIVE_CLK_PHASE_2_US(NativeClk, NativePhase) (((NativeClk) * 625 + (NativePhase)) >> 1)

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ uint32_t DCHS_bt_get_native_clock_us(void)
{
    uint32_t NativeClk = 0;
    uint16_t NativePhase = 0;
    DCHS_bt_pka_get_native_clk(&NativeClk, &NativePhase);
    // (NativeClk * 3125 + NativePhase * 5) / 10;
    // RB_LOG_I("NativeClk=%u NativePhase=%u", 2, NativeClk & 0xFFFFFFFC, NativePhase);
    return NATIVE_CLK_PHASE_2_US(NativeClk & 0xFFFFFFFC, NativePhase); // us, max = (0xFFFFFFE * 625 + 2499) >> 1 = 0x8000270
}


ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ int32_t DCHS_bt_get_native_clock_us_duration(uint32_t start_count, uint32_t end_count)
{
    int32_t duration;
    if (end_count > start_count) {
        duration = (int32_t)(end_count - start_count);
    } else {
        if ((start_count - end_count) < 0x4000000) {
            duration = (int32_t)(end_count - start_count);
        } else {
            duration =  - ((0x8000270 - (start_count - end_count)) + 1);
        }
    }

    return duration;
}

ATTR_TEXT_IN_FAST_MEM static __FORCE_INLINE__ uint32_t mux_get_bt_clock_count(void)
{
    uint32_t clock_us = 0;
    if (mux_ll_uart_is_bt_clock_synced()) {

        clock_us = DCHS_bt_get_native_clock_us();

    #ifdef AIR_DCHS_MODE_SLAVE_ENABLE
        clock_us -= mux_ll_uart_get_clock_offset_us();
    #endif
    }
    return clock_us;
}

#define MUX_LL_UART_PKT_HEAD 0x55

#ifdef MUX_LL_UART_SEQ_ENABLE
#define MUX_SEQ_LEN 1
#else
#define MUX_SEQ_LEN 0
#endif

#ifdef MUX_LL_UART_HEADER_CHECK_ENABLE
#define MUX_HEADER_CHECK_LEN 2
#else
#define MUX_HEADER_CHECK_LEN 0
#endif

#ifdef MUX_LL_UART_CRC_CHECK_ENABLE
#define MUX_DATA_CHECK_LEN 2
#else
#define MUX_DATA_CHECK_LEN 0
#endif

#ifdef MUX_LL_UART_TIMESTAMP_ENABLE
#define MUX_TIMESTAMP_LEN 4
#else
#define MUX_TIMESTAMP_LEN 0
#endif

#define MUX_PACKAGE_HEAD_LEN (4 + MUX_TIMESTAMP_LEN + MUX_SEQ_LEN + MUX_HEADER_CHECK_LEN)

#define MUX_PACKAGE_HEAD_TAIL_LEN (MUX_PACKAGE_HEAD_LEN + MUX_DATA_CHECK_LEN)

#define MUX_HEADER_HEAD_IDX         0
#define MUX_HEADER_ID_IDX           1
#define MUX_HEADER_LOW_LEN_IDX      2
#define MUX_HEADER_HIGH_LEN_IDX     3
#define MUX_HEADER_SEQ_IDX          4

#ifndef MUX_LL_UART_CUSTOMIZATION_USER_COUNT_MAX
#define MUX_LL_UART_CUSTOMIZATION_USER_COUNT_MAX 0
#endif

#ifndef MUX_DEF_CUSTOMIZATION_USER_BUFFER
#define MUX_DEF_CUSTOMIZATION_USER_BUFFER
#endif

#ifndef MUX_DEF_CUSTOMIZATION_USER_ATTR
#define MUX_DEF_CUSTOMIZATION_USER_ATTR
#endif

#define MAX_MUX_LL_USER_COUNT               (6 + MUX_LL_INTERNAL_USER_COUNT_MAX + MUX_LL_UART_CUSTOMIZATION_USER_COUNT_MAX)

extern mux_ll_uart_user_configure_t g_mux_ll_uart_user_configure_table[MAX_MUX_LL_USER_COUNT];

#define foreach_node(node, head) \
        for (node = head; node; node = node->next)

#define last_node(node, head) ({\
    for (node = head; node && node->next; node = node->next);\
    node;\
})

#define foreach_array(type, item, array, len, ...) \
    for (uint32_t idx = 0; idx != len; idx++) {\
        type item = (type)array[idx];\
        __VA_ARGS__\
    }
#define foreach_index(idx, start, end) \
    for (uint32_t idx = start; idx != end; idx++)

typedef struct {
    uint8_t head;
    uint8_t id;
    uint16_t length;
#ifdef MUX_LL_UART_TIMESTAMP_ENABLE
    uint32_t timestamp;
#endif
#ifdef MUX_LL_UART_SEQ_ENABLE
    uint8_t seq;
#endif
#ifdef MUX_LL_UART_HEADER_CHECK_ENABLE
    uint16_t crc;
#endif
} __attribute__((packed)) mux_ll_header_t;

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
    mux_user_attr_t uattr;

    uint32_t tx_bt_timestamp;
    uint32_t tx_gpt_timestamp;
    uint32_t rx_timestamp;
#ifdef MUX_LL_UART_SEQ_ENABLE
    uint8_t tx_pkt_current_seq;
    uint8_t rx_pkt_current_seq;
    uint8_t rx_pkt_seq;
    uint8_t tx_pkt_seq;
#endif
} mux_ll_user_context_t;

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

typedef struct mux_user_fifo_list {
    struct mux_user_fifo_list *next;
    mux_ll_user_config_t *p_user_config;
} mux_user_fifo_node_t;

typedef enum {
    MUX_TX_TRIGGER_OWNER_MCU_TX = 1,
    MUX_TX_TRIGGER_OWNER_RDYW   = 2,
    MUX_TX_TRIGGER_OWNER_DSP_TX = 3,
    MUX_TX_TRIGGER_OWNER_WAKEUP = 4,
} mux_tx_trigger_owner_t;

#define MUX_SET_TX_FLAGS(uid, debug) (g_port_config.tx_flags = ((((uid) & 0xFF) << 8) | ((debug) & 0xFF)))
#define MUX_GET_TX_FLAGS() (g_port_config.tx_flags)

typedef struct {
    volatile uint32_t wptr;
    volatile uint32_t rptr;
    volatile uint32_t fifo_cnt;
    volatile uint32_t fifo_sta;
    volatile uint32_t altlen;
    volatile uint32_t fifo_size;
} mux_ll_uart_hw_info_t;

typedef struct {
    mux_ringbuffer_t txbuf;
    mux_ringbuffer_t rxbuf;
    mux_port_setting_t *p_user_setting;             //  user hardware port assignment and port setting.
    uint32_t tx_buf_addr;                           //  user tx hardware buffer base address.
    uint32_t tx_buf_size;                           //  user tx hardware buffer total size.
    uint32_t rx_buf_addr;                           //  user rx hardware buffer base address.
    uint32_t rx_buf_size;
    mux_ll_uart_hw_info_t *ll_uart_hw_info[2];
    mux_protocol_t protocol_cpu;
    mux_user_fifo_node_t *user_fifo_list_head[MUX_LL_UART_PRIORITY_MAX];   //user fifo list head
    uint32_t tx_flags;
    bool is_bypass_tx;
} mux_ll_port_config_t;

typedef struct {
    uint32_t config_head;
    uint32_t user_count;
    uint16_t uart_tx_threshold;
    uint16_t uart_rx_threshold;
    uint32_t tx_pkt_head_tail_len;
    dchs_mode_t device_mode;
    uint32_t notify_counter;
    mux_ll_user_totify_t dsp_notify[MUX_LL_NOTIFY_BUFFER_MAX];
    mux_ll_user_config_t user_config[MAX_MUX_LL_USER_COUNT];
} mux_ll_config_t;

extern mux_ll_port_config_t g_port_config;
extern ATTR_SHARE_ZIDATA mux_ll_config_t mux_ll_config;
extern mux_ll_user_context_t g_mux_ll_user_context[MAX_MUX_LL_USER_COUNT];

extern mux_status_t mux_ll_uart_normal_init(uint8_t port_index, mux_ll_port_config_t *p_setting);
extern mux_status_t mux_init_ll(mux_port_t port, mux_port_setting_t *setting, mux_protocol_t *protocol_callback);
extern mux_status_t mux_deinit_ll(mux_port_t port);
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
extern void mux_ll_uart_format_header(mux_ll_header_t *header, uint8_t uid, uint32_t pkt_len);
extern uint32_t mux_ll_uart_send_to_vfifo(uint8_t *pdata, uint32_t length, bool is_critical);
#ifdef MUX_LL_UART_RDWR_PTR_CHECK_ENABLE
extern uint32_t mux_uart_get_hw_wptr(struct mux_ringbuffer *rb);
extern uint32_t mux_uart_get_hw_rptr(struct mux_ringbuffer *rb);
#endif
#endif