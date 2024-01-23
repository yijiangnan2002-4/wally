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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include "mux.h"
#include "mux_port.h"
#include "mux_port_device.h"

#include "mux_ll_uart.h"

#include "hal_resource_assignment.h"
#include "hal_pdma_internal.h"
#include "hal_uart_internal.h"

extern UART_REGISTER_T *const g_uart_regbase[];
volatile mux_ll_config_t *g_mux_ll_config = NULL;

vdma_channel_t uart_tx_ch[3] = {VDMA_UART0TX, VDMA_UART1TX, VDMA_UART2TX};
vdma_channel_t uart_rx_ch[3] = {VDMA_UART0RX, VDMA_UART1RX, VDMA_UART2RX};

/* define mux handle magic symbol */
#define handle_to_port(handle) ((handle) & 0xFF)
#define handle_to_user_id(handle) (((handle) >> 8) & 0xFF)
#define user_id_to_handle(user_id, port) ((MUX_LL_HANDLE_MAGIC_NUMBER << 16) | (user_id << 8) | (port))
#define MUX_LL_CONFIG_ADDRESS *(volatile uint32_t*)HW_SYSRAM_PRIVATE_MEMORY_LLMUX_VAR_PORT_START

mux_status_t mux_open_ll(mux_port_t port, const char *user_name, mux_handle_t *p_handle, mux_callback_t callback, void *user_data)
{
    uint32_t uid;

    mux_ll_user_config_t *mux_ll_user_config = NULL;

    if (g_mux_ll_config == NULL) {
        g_mux_ll_config = (volatile mux_ll_config_t *)hal_memview_infrasys_to_dsp0(MUX_LL_CONFIG_ADDRESS);
    }

    for (uid = 0; uid < g_mux_ll_config->user_count; uid++) {
        mux_ll_user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];
        if (!strcmp(mux_ll_user_config->user_name, user_name) && ((HAL_CORE_DSP0 == mux_ll_user_config->tx_scid) || (HAL_CORE_DSP0 == mux_ll_user_config->rx_dcid))) {
            break;
        }
    }

    if (uid == g_mux_ll_config->user_count) {
        LOG_E(MUX_PORT_DSP, "[mux_open_ll] port=%d, [%s] user not found", port, user_name);
        return MUX_STATUS_ERROR;
    }

    *p_handle = user_id_to_handle(uid, port);
    mux_ll_user_config->callback[GET_CURRENT_CPU_ID()].entry = callback;
    mux_ll_user_config->callback[GET_CURRENT_CPU_ID()].user_data = user_data;
    RB_LOG_I("[mux_open_ll] port=%d handle=0x%x", 2, port, *p_handle);
    return MUX_STATUS_OK;
}

mux_status_t mux_close_ll(mux_handle_t handle)
{
    uint32_t uid;
    uint32_t per_cpu_irq_mask;
    mux_ll_user_config_t *mux_ll_user_config = NULL;

    uid = handle_to_user_id(handle);
    mux_ll_user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];

    port_mux_cross_local_enter_critical(&per_cpu_irq_mask);
    mux_ll_user_config->callback[GET_CURRENT_CPU_ID()].entry = NULL;
    mux_ll_user_config->callback[GET_CURRENT_CPU_ID()].user_data = NULL;
    port_mux_cross_local_exit_critical(per_cpu_irq_mask);
    return MUX_STATUS_OK;
}

void mux_reset_user_fifo(mux_handle_t handle)
{
    uint32_t uid;
    mux_ll_user_config_t *user_config;

    uid = handle_to_user_id(handle);

    user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];
    mux_ringbuffer_reset((mux_ringbuffer_t *)&user_config->rxbuf);
}

ATTR_TEXT_IN_FAST_MEM mux_status_t mux_rx_ll(mux_handle_t handle, mux_buffer_t *buffer, uint32_t *receive_done_data_len)
{
    uint32_t uid;
    mux_ll_user_config_t *user_config;
    mux_ringbuffer_t *rxbuf;
    uint32_t vd_len = 0;

    uid = handle_to_user_id(handle);

    if (g_mux_ll_config == NULL) {
        return MUX_STATUS_ERROR;
    }

    user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];
    rxbuf = &user_config->rxbuf;
    if (rxbuf->capacity == 0) {
        RB_LOG_E("[mux_rx_ll] RX BUF NOT INIT, uid=%02x ", 1, rxbuf->uid);
        return MUX_STATUS_ERROR_NOT_INIT;
    }
    *receive_done_data_len = mux_ringbuffer_read(rxbuf, buffer->p_buf, buffer->buf_size);
    rxbuf->access_count++;
    rxbuf->data_amount += *receive_done_data_len;

    vdma_get_available_receive_bytes(uart_rx_ch[MUX_LL_PORT_2_UART_PORT(handle_to_port(handle))], &vd_len);

    RB_LOG_I("[mux_rx_ll] [%02x] expect size=%d real size=%d, ud_len=%d, vd_len=%d acc_cnt=%u damount=%u uwm=%u bt_clk=%uus", 9, rxbuf->uid, buffer->buf_size, *receive_done_data_len,\
        mux_ringbuffer_data_length(rxbuf), vd_len, rxbuf->access_count, rxbuf->data_amount, rxbuf->water_mark, mux_get_bt_clock_count());

    return MUX_STATUS_OK;
}

ATTR_TEXT_IN_FAST_MEM bool mux_ll_uart_notify_mcu(mux_ll_message_t message, void * param)
{
    hal_ccni_status_t status;
    bool norify_status = true;
    uint32_t count = 0;
    uint32_t msg_array[2] = {0};
    msg_array[0] = message;
    msg_array[1] = (uint32_t)param;
    do {
        status = hal_ccni_set_event(CCNI_DSP0_TO_CM4_LL_UART, (hal_ccni_message_t*)msg_array);
        if(status == HAL_CCNI_STATUS_BUSY) {
            hal_gpt_delay_us(100);
            count++;
        }
        if (count > 10) {
            norify_status = false;
            break;   //wait a while and exit if it is overtime.
        }
    } while(status == HAL_CCNI_STATUS_BUSY);

    return norify_status;
}

ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_notify_mcu_ready_to_write(mux_ll_user_config_t * user_config, uint32_t write_len)
{
    uint32_t param;
    uint32_t ud_len = mux_ringbuffer_data_length(&user_config->txbuf);

    param = (user_config->txbuf.uid << 16) | ud_len;
    if (!mux_ll_uart_notify_mcu(MUX_LL_MSG_READY_TO_WRITE, (void*)param)) {
        RB_LOG_D("DSP CCNI send fail, uid=%02x ud_len=%u write_len=%u", 3, \
            user_config->txbuf.uid, ud_len, write_len);
    }
}

ATTR_TEXT_IN_FAST_MEM bool mux_ll_uart_is_tx_blocking(mux_ringbuffer_t *txbuf)
{
    return ((g_uart_regbase[txbuf->port_idx]->MCR_UNION.MCR & 0x10000) == 0x10000);
}

ATTR_TEXT_IN_FAST_MEM mux_status_t mux_tx_ll_with_silence(mux_handle_t handle, const mux_buffer_t buffers[], uint32_t buffers_counter, uint32_t *send_done_data_len)
{
    mux_ringbuffer_t *txbuf;
    mux_ll_user_config_t *user_config;
    mux_status_t mux_status = MUX_STATUS_OK;
    uint32_t uid;
    uint32_t i;
    uint32_t start_tick;
    uint32_t elapse_tick = 0;
    uint32_t payload_size = 0;
    uint32_t silence_write_done;
    uid = handle_to_user_id(handle);
    user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];
    txbuf = &user_config->txbuf;

    if (user_config->tx_silence_enable) {
        //step1: Judge whether need add silence
        if (txbuf->drop_count > 0) {
            silence_write_done = mux_ringbuffer_write_silence_st(txbuf, txbuf->drop_count);
            RB_LOG_W("[mux_tx_ll] [%02x] add silence data %u bytes, total=%u ", 3, txbuf->uid, silence_write_done, txbuf->drop_count);
            if (silence_write_done != txbuf->drop_count) {
                if (mux_ll_uart_is_tx_blocking(txbuf)) {
                    RB_LOG_W("[mux_tx_ll] [%02x] the other side maybe crash!", 1, txbuf->uid);
                } else {
                    RB_LOG_W("[mux_tx_ll] [%02x] tx silence fail, silence data=%u", 2, txbuf->uid, txbuf->drop_count);
                    // assert(0 && "tx silience fail!!");
                }
                txbuf->drop_count -= silence_write_done;
            } else {
                txbuf->drop_count = 0;
            }
        }

        //step2: while loop to send with timeout
        /* Calculate total size of payload */
        for (i = 0; i < buffers_counter; i++) {
            payload_size += buffers[i].buf_size;
        }
        start_tick = mux_get_gpt_tick_count();
        while (user_config->tx_silence_timeout > elapse_tick) {
            if(mux_ringbuffer_free_space(txbuf) >= payload_size) {
                break;
            }
            elapse_tick = mux_get_tick_elapse(start_tick);
        }

        if (user_config->tx_silence_timeout > elapse_tick) {//not timeout
            mux_status = mux_tx_ll(handle, buffers, buffers_counter, send_done_data_len);
            txbuf->drop_count += (payload_size - *send_done_data_len);
        } else {
            txbuf->drop_count += payload_size;
        }
    } else {
        return mux_tx_ll(handle, buffers, buffers_counter, send_done_data_len);
    }
    return mux_status;
}
ATTR_TEXT_IN_FAST_MEM mux_status_t mux_tx_ll(mux_handle_t handle, const mux_buffer_t buffers[], uint32_t buffers_counter, uint32_t *send_done_data_len)
{
    mux_ringbuffer_t *txbuf;
    uint32_t send_len = 0;
    mux_ll_user_config_t *user_config;
    uint32_t uid;
    uint32_t i;
    uint32_t vd_len = 0;
    uint32_t payload_size = 0;

#ifdef MUX_LL_UART_RB_DUMP_ENABLE
    uint32_t expected_total_size = 0;
#endif
    const mux_buffer_t *pbuf;
    uid = handle_to_user_id(handle);

    if (g_mux_ll_config == NULL) {
        return MUX_STATUS_ERROR;
    }

    user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];

    txbuf = &user_config->txbuf;
    if (txbuf->capacity == 0) {
        RB_LOG_E("[mux_tx_ll] TX BUF NOT INIT, uid=%02x ", 1, txbuf->uid);
        return MUX_STATUS_ERROR_NOT_INIT;
    }

    /* Calculate total size of payload */
    for (i = 0; i < buffers_counter; i++) {
        payload_size += buffers[i].buf_size;
    }

    vdma_get_available_receive_bytes(uart_tx_ch[MUX_LL_PORT_2_UART_PORT(handle_to_port(handle))], &vd_len);

    if (g_mux_ll_config->device_mode == DCHS_MODE_SINGLE) {
        RB_LOG_W("[mux_tx_ll] bypass, uid=%02x send_len=%d ud_len=%d vd_len=%d dev_mode=%u", 5, txbuf->uid, payload_size, mux_ringbuffer_data_length(txbuf), vd_len, g_mux_ll_config->device_mode);
        *send_done_data_len = payload_size;
        return MUX_STATUS_OK;
    }
    for (i = 0; i < buffers_counter; i++) {
        pbuf = &buffers[i];
            send_len += mux_ringbuffer_write(txbuf, pbuf->p_buf, pbuf->buf_size);
#ifdef MUX_LL_UART_RB_DUMP_ENABLE
        expected_total_size += pbuf->buf_size;
#endif
    }
#ifdef MUX_LL_UART_RB_DUMP_ENABLE
    if (send_len != expected_total_size) {
        mux_ringbuffer_hexdump(txbuf, false);
    }
#endif
    txbuf->access_count++;
    txbuf->data_amount += send_len;
    *send_done_data_len = send_len;

    mux_ll_uart_notify_mcu_ready_to_write(user_config, send_len);

    RB_LOG_I("[mux_tx_ll] [%02x] write_len=%d ud_len=%d vd_len=%d MCR=0x%x acc_cnt=%u damount=%u uwm=%u bt_clk=%uus", 9, txbuf->uid, send_len, mux_ringbuffer_data_length(txbuf), \
    vd_len, g_uart_regbase[1]->MCR_UNION.MCR, txbuf->access_count, txbuf->data_amount, txbuf->water_mark, mux_get_bt_clock_count());

    return MUX_STATUS_OK;
}

ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_rx_event_from_mcu_handler(hal_ccni_event_t event, void * msg)
{
    hal_ccni_status_t status;
    uint32_t data_len;
    uint32_t duration_count;
    uint8_t uid;
    static uint32_t cbts_idx = 0;
    mux_ll_user_config_t *ll_user_config = NULL;
    mux_ll_user_timestamp_t *cbts;
    (void)msg;

    if (g_mux_ll_config == NULL) {
        g_mux_ll_config = (volatile mux_ll_config_t *)hal_memview_infrasys_to_dsp0(MUX_LL_CONFIG_ADDRESS);
    }

    status = hal_ccni_clear_event(event);  // clear the event.
    if (status != HAL_CCNI_STATUS_OK) {
        RB_LOG_E("MCU CCNI clear event: 0x%x something wrong, return is %d", 2, event, status);
    }

    status = hal_ccni_unmask_event(event); // unmask the event.
    if (status != HAL_CCNI_STATUS_OK) {
        RB_LOG_E("MCU CCNI unmask event: 0x%x something wrong, return is %d", 2, event, status);
    }

    for (uid = 0; uid < g_mux_ll_config->user_count; uid++) {
        ll_user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];
        if (ll_user_config->rx_dcid != HAL_CORE_DSP0) {
            continue;
        }
        data_len = mux_ringbuffer_data_length(&ll_user_config->rxbuf);
        if ((data_len > 0) && ll_user_config->callback[GET_CURRENT_CPU_ID()].entry) {
            cbts = &ll_user_config->callback_timestamp[cbts_idx++ % MUX_LL_TIMESTAMP_BUFFER_MAX];
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &cbts->start_timestamp);
            ll_user_config->callback[GET_CURRENT_CPU_ID()].entry(user_id_to_handle(uid, (uint32_t)ll_user_config->rxbuf.port_idx + MUX_LL_UART_0), MUX_EVENT_READY_TO_READ, data_len, ll_user_config->callback[GET_CURRENT_CPU_ID()].user_data);
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &cbts->end_timestamp);
            hal_gpt_get_duration_count(cbts->start_timestamp, cbts->end_timestamp, &duration_count);
            if (duration_count > 10) {
                RB_LOG_W("RDY_TO_READ CCNI uid:%u callback_duration:%u", 2, uid, duration_count);
            }
        }
    }
}

mux_status_t mux_control_ll(mux_handle_t handle, mux_ctrl_cmd_t command, mux_ctrl_para_t *para)
{
    uint32_t uid;
    mux_ll_user_config_t *user_config;
    mux_ll_user_timeout_with_silence_t * timeout_with_silence;
    uid = handle_to_user_id(handle);

    user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];
    switch(command) {
    case MUX_CMD_GET_LL_USER_RX_BUFFER_DATA_SIZE:
        para->mux_ll_user_rx_data_len = mux_ringbuffer_data_length(&user_config->rxbuf);
        if (para->mux_ll_user_rx_data_len == 0) {
            RB_LOG_W("[mux_control_ll] uid[%02u] rx_ud_len:%u", 2, uid, para->mux_ll_user_rx_data_len);
        }
        break;
    case MUX_CMD_GET_LL_USER_TX_BUFFER_FREE_SIZE:
        para->mux_ll_user_tx_free_len = mux_ringbuffer_free_space(&user_config->txbuf);
        RB_LOG_I("[mux_control_ll] uid[%02u] tx_ud_len:%u", 2, uid, para->mux_ll_user_tx_free_len);
        break;
    case MUX_CMD_SET_LL_USER_TX_PKT_LEN:
        break;
    case MUX_CMD_SET_LL_USER_TIMEOUT_WITH_SILENCE:
        RB_LOG_W("[mux_control_ll] uid[%02u] set time with silence", 1, uid);
        timeout_with_silence = (mux_ll_user_timeout_with_silence_t*)para;
        user_config->tx_silence_timeout = timeout_with_silence->tx_timeout;
        user_config->rx_silence_enable = timeout_with_silence->rx_enable;
        user_config->tx_silence_enable = timeout_with_silence->tx_enable;
        user_config->rx_silence_drop_max = timeout_with_silence->rx_drop_max;
        break;
    default:
        break;
    }
    return MUX_STATUS_OK;
}

mux_status_t mux_query_ll_user_handle(mux_port_t port, const char *user_name, mux_handle_t *p_handle)
{
    uint32_t i;
    uint32_t ll_user_count;
    mux_ll_user_config_t *mux_ll_user_config = NULL;

    *p_handle = 0xdeadbeef;

    if (user_name == NULL) {
        return MUX_STATUS_ERROR_PARAMETER;
    }

    ll_user_count = g_mux_ll_config->user_count;

    for (i = 0; i < ll_user_count; i++) {
        mux_ll_user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[i];
        if (!strcmp(mux_ll_user_config->user_name, user_name) && ((HAL_CORE_DSP0 == mux_ll_user_config->tx_scid) || (HAL_CORE_DSP0 == mux_ll_user_config->rx_dcid))) {
            *p_handle = user_id_to_handle(i, port);
            return MUX_STATUS_OK;
        }
    }

    return MUX_STATUS_ERROR;
}

mux_ringbuffer_t * mux_query_ll_user_buffer(mux_handle_t handle, bool is_rx)
{
    uint32_t uid;
    mux_ringbuffer_t *rb = NULL;

    if (is_mux_ll_handle(handle)) {
        uid = handle_to_user_id(handle);
        if (is_rx) {
            rb = (mux_ringbuffer_t *)&g_mux_ll_config->user_config[uid].rxbuf;
        } else {
            rb = (mux_ringbuffer_t *)&g_mux_ll_config->user_config[uid].txbuf;
        }
    }
    return rb;
}

void mux_clear_ll_user_buffer(mux_handle_t handle, bool is_rx)
{
    uint32_t uid = handle_to_user_id(handle);
    mux_ll_user_config_t *user_config = NULL;
    mux_ringbuffer_t *rb;
    uint32_t mask;
    uint32_t count = 0;
    uint32_t local_msg_status = 0;
    uint32_t data_size;

    user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];
    rb = mux_query_ll_user_buffer(handle, is_rx);
    if (!rb) {
        RB_LOG_E("[mux_clear_ll_user_buffer] Handle error:0x%x", 1, handle);
        return;
    }
    data_size = mux_ringbuffer_data_length(rb);
    if (data_size == 0) {
        LOG_MSGID_I(common, "[mux_clear_ll_user_buffer] DSP trigger clear: handle=0x%x is_rx=%d, buffer empty", 3, handle, is_rx);
        return;
    }

    local_msg_status = (is_rx) ? (1) : (1 << 1);

    port_mux_cross_local_enter_critical(&mask);
    user_config->msg_status |= local_msg_status;
    port_mux_cross_local_exit_critical(mask);

    LOG_MSGID_I(common, "[mux_clear_ll_user_buffer] DSP trigger clear: handle=0x%x is_rx=%d, msg_status=0x%x", 3, handle, is_rx, user_config->msg_status);

    if (!mux_ll_uart_notify_mcu(MUX_LL_MSG_CLEAR_USER_BUFFER, (void*)(local_msg_status | (uid << 8)))) {
        RB_LOG_D("DSP CCNI send fail, uid=%02x clear [%d] buffer fail", 2, uid, is_rx);
    }

    while ((user_config->msg_status & local_msg_status) != 0) {
        hal_gpt_delay_us(10);
        if (count++ > 200) {
            assert(0 && "mux_clear_ll_user_buffer timeout");
            break;
        }
    }
}

mux_status_t mux_peek_ll(mux_handle_t handle, mux_buffer_t *buffer, uint32_t *receive_done_data_len)
{
    uint32_t uid;
    mux_ll_user_config_t *user_config;

    uid = handle_to_user_id(handle);

    if (g_mux_ll_config == NULL) {
        return MUX_STATUS_ERROR;
    }

    user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];

    *receive_done_data_len = mux_ringbuffer_peek((mux_ringbuffer_t *)&user_config->rxbuf, buffer->p_buf, buffer->buf_size);

    RB_LOG_D("[mux_peek_ll] [%02x] rxbuf[%s] expect size=%d real size=%d", 3, uid, buffer->buf_size, *receive_done_data_len);
    return MUX_STATUS_OK;
}

void mux_hexdump_ll(mux_handle_t handle, const char* funcname, bool is_dump_data, bool is_txbuff)
{
    uint32_t uid;
    mux_ll_user_config_t *user_config;
    mux_ringbuffer_t *rb;

    if (!is_mux_ll_handle(handle)) {
        return;
    }
    uid = handle_to_user_id(handle);
    user_config = (mux_ll_user_config_t *)&g_mux_ll_config->user_config[uid];

    rb = is_txbuff ? &user_config->txbuf : &user_config->rxbuf;
    mux_ringbuffer_dump(rb, funcname, is_dump_data);
}

dchs_mode_t dchs_get_device_mode(void)
{
    uint32_t timeout;
    uint32_t start_count = mux_get_gpt_tick_count();

    if (g_mux_ll_config == NULL) {
        g_mux_ll_config = (volatile mux_ll_config_t *)hal_memview_infrasys_to_dsp0(MUX_LL_CONFIG_ADDRESS);
    }
#if 0
    do {
        timeout = mux_get_tick_elapse(start_count);
    } while ((g_mux_ll_config->device_mode == DCHS_MODE_ERROR) && (timeout < 3000000));//3s timeout

    if (g_mux_ll_config->device_mode == DCHS_MODE_ERROR) {
        //device sync timer may be blocked in MCU side!!
        assert(0);
    }
#else
    while (g_mux_ll_config->device_mode == DCHS_MODE_ERROR) {
        timeout = mux_get_tick_elapse(start_count);
        if (timeout > 100000) {
            RB_LOG_W("[device sync] waiting for sync", 0);
            start_count = mux_get_gpt_tick_count();
        }
    }
#endif
    if ((g_mux_ll_config->device_mode != DCHS_MODE_LEFT) && (g_mux_ll_config->device_mode != DCHS_MODE_RIGHT) && (g_mux_ll_config->device_mode != DCHS_MODE_SINGLE)) {
        assert(0 && "device mode error");
    }
    return g_mux_ll_config->device_mode;
}
