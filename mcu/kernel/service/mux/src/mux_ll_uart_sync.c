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

#include "hal_uart_internal.h"
#include "mux_ll_uart.h"
#include "mux_ll_uart_sync.h"

#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#endif/*HAL_SLEEP_MANAGER_ENABLED*/


#define DEVICE_SYNC_TIMEOUT 1000
#define SW1_IRQn (SW_IRQn + 1)
mux_handle_t device_sync_handle;

uint32_t device_sync_timer_handle = 0xFFFFFFFF;
uint32_t device_sync_start_count = 0;

volatile dchs_mode_t g_device_mode = DCHS_MODE_ERROR;
volatile static bool is_other_ready = false;
volatile static bool is_self_ready = false;
static uint16_t detect_seq = 0;

ATTR_TEXT_IN_FAST_MEM bool mux_ll_uart_send_sync_data(uint16_t id, uint16_t seq)
{
    uint16_t buf[2] = {id, seq};

    mux_buffer_t buffer = {(uint8_t*)buf, sizeof(buf)};
    mux_status_t status;
    uint32_t send_done_len = 0;
    status = mux_tx(device_sync_handle, &buffer, 1, &send_done_len);
    if (status != MUX_STATUS_OK) {
        assert(0);
    }
    if (buffer.buf_size != send_done_len) {
        RB_LOG_I("[device sync] tx buffer full", 0);
    }
    // mux_hexdump_ll(device_sync_handle, __func__, true, true);
    return true;
}

static void mux_ll_uart_set_device_mode(dchs_mode_t device_mode)
{
    mux_ll_config.device_mode = device_mode;
    g_device_mode = device_mode;
}

void mux_ll_uart_device_sync_gpt_callback(void *user_data)
{
    hal_gpt_status_t gpt_status;
    mux_ringbuffer_t *tx_vfifo = &g_port_config.txbuf;
    // uint8_t xoff = 0x13;
    uint8_t xon = 0x11;
    uint32_t mask;
    uint32_t tx_len;

    LOG_MSGID_I(common, "[device sync] mux_ll_uart_device_sync_gpt_callback enter. retry time:%u", 1, (uint32_t)user_data);

    hal_nvic_save_and_set_interrupt_mask(&mask);
    hal_uart_disable_flowcontrol(tx_vfifo->port_idx);
    tx_len = mux_ringbuffer_write_st(tx_vfifo, &xon, 1);
    if (tx_len != 0) {
        mux_ringbuffer_write_move_hw_tail_st(tx_vfifo, 1);
    }
    hal_uart_set_software_flowcontrol(tx_vfifo->port_idx, 0x11, 0x13, 0x77);
    hal_nvic_restore_interrupt_mask(mask);
    if (tx_len == 0) {
        RB_LOG_E("[device sync] tx buffer full", 0);
    }
    if (g_device_mode != DCHS_MODE_ERROR ) {
#ifdef HAL_SLEEP_MANAGER_ENABLED
        hal_sleep_manager_unlock_sleep(MUX_LL_UART_SLEEP_LOCK_HANDLE);
#endif
        LOG_MSGID_I(common, "[device sync] sync done [%u], return", 1, g_device_mode);
        return ;
    }
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
    mux_ll_uart_send_sync_data(DCHS_SYNC_DETECT_SLAVE, detect_seq++);
    LOG_MSGID_I(common, "[device sync] timer send DCHS_SYNC_DETECT_SLAVE", 0);
#else
    mux_ll_uart_send_sync_data(DCHS_SYNC_DETECT_MASTER, detect_seq++);
    LOG_MSGID_I(common, "[device sync] timer send DCHS_SYNC_DETECT_MASTER", 0);
#endif
    gpt_status = hal_gpt_sw_start_timer_ms(device_sync_timer_handle, (uint32_t)user_data, mux_ll_uart_device_sync_gpt_callback, user_data);
    if (HAL_GPT_STATUS_OK != gpt_status) {
        RB_LOG_E("[device sync] start timer fail:%d", 1, gpt_status);
    }
}

ATTR_TEXT_IN_FAST_MEM void dchs_device_sync_cb(hal_nvic_irq_t irq_number)
{
    mux_ll_uart_device_sync_gpt_callback((void*)DEVICE_SYNC_TIMEOUT);
}

dchs_mode_t dchs_get_device_mode(void)
{
    uint32_t start_count = mux_get_gpt_tick_count();
    uint32_t timeout;

    do {
        timeout = mux_get_tick_elapse(start_count);
    } while ((device_sync_timer_handle == 0xFFFFFFFF) && (timeout < 3000000));//3s timeout

    if (device_sync_timer_handle == 0xFFFFFFFF) {
        //device sync sw irq may be blocked!!
        assert(0);
    }
#if 0
    start_count = mux_get_gpt_tick_count();
    do {
        timeout = mux_get_tick_elapse(start_count);
    } while ((g_device_mode == DCHS_MODE_ERROR) && (timeout < 4000000));//4s timeout

    if (g_device_mode == DCHS_MODE_ERROR) {
        //device sync timer may be blocked!!
        assert(0);
    }
#else
    start_count = mux_get_gpt_tick_count();
    while (g_device_mode == DCHS_MODE_ERROR) {
        timeout = mux_get_tick_elapse(start_count);
        if (timeout > 500000) {
            RB_LOG_W("[device sync] waiting for sync", 0);
            start_count = mux_get_gpt_tick_count();
        }
    }
#endif
    return g_device_mode;
}

ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_sync_do_recv_detect_cmd_and_send_response(uint16_t response_cmd, dchs_mode_t device_mode, uint16_t rsp_seq)
{
    if (is_self_ready || (g_device_mode == DCHS_MODE_SINGLE)) {
        LOG_MSGID_I(common, "[device sync] [devid:%d] recv detect_cmd, seems abnormal reset, so it should reset to sync, current dev mode = %d, is_other_ready:%d is_self_ready:%d", 4,\
            device_mode, g_device_mode, is_other_ready, is_self_ready);
        hal_wdt_software_reset();
    } else {
        mux_ll_uart_send_sync_data(response_cmd, rsp_seq);
        is_self_ready = true;
        if (is_other_ready) {
            hal_gpt_sw_stop_timer_ms(device_sync_timer_handle);
            hal_gpt_sw_free_timer(device_sync_timer_handle);
            mux_ll_uart_set_device_mode(device_mode);
        }
        LOG_MSGID_I(common, "[device sync] [devid:%d] recv detect_cmd, current dev mode = %d, is_other_ready:%d is_self_ready:%d", 4, device_mode, g_device_mode, is_other_ready, is_self_ready);
    }
}

ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_sync_do_recv_self_detect(uint16_t detect_cmd)
{
    static uint32_t single_mode_confirm_counter = 0;

    if ((0x3 & single_mode_confirm_counter++) == 3) {
        LOG_MSGID_I(common, "[device sync] recv detect_cmd:0x%x, single mode,  is_other_ready:%d is_self_ready:%d", 3, detect_cmd, is_other_ready, is_self_ready);
        mux_ll_uart_set_device_mode(DCHS_MODE_SINGLE);
    } else {
        mux_ll_uart_send_sync_data(detect_cmd, 0x8000|detect_seq++);
        LOG_MSGID_I(common, "[device sync] recv detect_cmd:0x%x, confirm counter=%d,  is_other_ready:%d is_self_ready:%d", 4, detect_cmd, single_mode_confirm_counter, is_other_ready, is_self_ready);
    }
}

ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_sync_do_recv_device_response(dchs_mode_t device_mode)
{
    is_other_ready = true;
    if (is_self_ready) {
        hal_gpt_sw_stop_timer_ms(device_sync_timer_handle);
        hal_gpt_sw_free_timer(device_sync_timer_handle);
        mux_ll_uart_set_device_mode(device_mode);
    }
    LOG_MSGID_I(common, "[device sync] recv response, is_other_ready:%d is_self_ready:%d", 2, is_other_ready, is_self_ready);
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_sync_callback(uint8_t *buff, uint32_t len)
{
    uint16_t *pbuf = (uint16_t*)buff;
    uint16_t id = *pbuf++;
    uint16_t seq = *pbuf;

    switch(id) {
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
    case DCHS_SYNC_DETECT_MASTER: /*CMD from Slave, send response to Slave.*/
        mux_ll_uart_sync_do_recv_detect_cmd_and_send_response(DCHS_SYNC_DETECT_RESPONSE_FROM_MASTER, DCHS_MODE_RIGHT, seq);
        break;
    case DCHS_SYNC_DETECT_SLAVE: /*TX RX connect, FT single mode*/
        mux_ll_uart_sync_do_recv_self_detect(DCHS_SYNC_DETECT_SLAVE);
        break;
    case DCHS_SYNC_DETECT_RESPONSE_FROM_SLAVE:
        mux_ll_uart_sync_do_recv_device_response(DCHS_MODE_RIGHT);
        break;
#endif

#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
    case DCHS_SYNC_DETECT_SLAVE: /*CMD from Master, send response to Master.*/
        mux_ll_uart_sync_do_recv_detect_cmd_and_send_response(DCHS_SYNC_DETECT_RESPONSE_FROM_SLAVE, DCHS_MODE_LEFT, seq);
        break;
    case DCHS_SYNC_DETECT_MASTER: /*TX RX connect, FT single mode*/
        mux_ll_uart_sync_do_recv_self_detect(DCHS_SYNC_DETECT_MASTER);
        break;
    case DCHS_SYNC_DETECT_RESPONSE_FROM_MASTER:
        mux_ll_uart_sync_do_recv_device_response(DCHS_MODE_LEFT);
        break;
#endif
    default:
        RB_LOG_E("[device sync] Error id:0x%x!!", 1, id);
        break;
    }
}

ATTR_TEXT_IN_TCM void mux_ll_uart_sync_cmd_callback(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data)
{
    mux_buffer_t buffer;
    mux_status_t mux_status;
    uint32_t read_done_size = 0;
    uint8_t user_rx_fifo[32];

    buffer.p_buf = user_rx_fifo;
    buffer.buf_size = sizeof(user_rx_fifo);

    switch (event) {
    case MUX_EVENT_READY_TO_READ:
        mux_status = mux_rx(handle, &buffer, &read_done_size);
        if (mux_status != MUX_STATUS_OK) {
            RB_LOG_E("[uart wakeup] [cmd_callback] mux_rx fail:%d", 1, mux_status);
            assert(0);
        } else {
            if (read_done_size != 0) {
                mux_ll_uart_sync_callback(buffer.p_buf, buffer.buf_size);
            } else {
                RB_LOG_E("[uart wakeup] [cmd_callback] mux_rx read_done_size = 0", 0);
            }
        }
        break;
    case MUX_EVENT_READY_TO_WRITE:
        RB_LOG_I("[device sync] [cmd_callback] MUX_EVENT_READY_TO_WRITE", 0);
        break;
    default:
        RB_LOG_I("[device sync] [cmd_callback] event %d no task", 1, event);
    }
}

void mux_ll_uart_sync_init(void)
{
    mux_status_t status;

    status = mux_open(MUX_LL_UART_PORT, "SYNC", &device_sync_handle, mux_ll_uart_sync_cmd_callback, NULL);
    if(MUX_STATUS_OK != status) {
        RB_LOG_I("[device sync] mux_open fail, status=%d handle=0x%x", 2, status, device_sync_handle);
        assert(0);
    } else {
        RB_LOG_I("[device sync] mux_open ok handle=0x%x", 1, device_sync_handle);
    }

    NVIC_SetPriority(SW1_IRQn, DEFAULT_IRQ_PRIORITY + 1);

    hal_nvic_register_isr_handler(SW1_IRQn, dchs_device_sync_cb);
    hal_nvic_enable_irq(SW1_IRQn);
    hal_nvic_set_pending_irq(SW1_IRQn);
    hal_gpt_sw_get_timer(&device_sync_timer_handle);
#ifdef HAL_SLEEP_MANAGER_ENABLED
    hal_sleep_manager_lock_sleep(MUX_LL_UART_SLEEP_LOCK_HANDLE);
#endif
}
