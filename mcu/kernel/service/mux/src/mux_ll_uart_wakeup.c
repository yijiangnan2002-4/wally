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
#include "mux_ll_uart_wakeup.h"

#ifdef AIR_LL_MUX_WAKEUP_ENABLE
#include "timers.h"
mux_handle_t wakeup_handle;
static TimerHandle_t mux_uart_sleep_lock_timer = NULL;
static TimerHandle_t mux_uart_sleep_monitor_timer = NULL;
static uint8_t mux_uart_rx_sleep_handle = 0xFF;
uint8_t mux_uart_tx_sleep_handle = 0xFF;
static bool mux_sleep_lock_timer_alive = false;
static bool mux_sleep_monitor_timer_alive = false;

volatile bool other_side_has_lock_sleep = true; /*record the other side sleep status.*/
volatile bool has_send_wakeup_cmd = false; /*record has sent wakeup cmd, but not received response.*/
volatile bool is_recieved_wakeup_cmd = false; /*record has  received wakeup cmd, set to true after received wakeup cmd, and clear after send wakeup response..*/
volatile bool mux_sleep_lock_timer_need_restart = false;  /*record if need restart 10s timer, if any cmd received before lock sleep timer timeout, it will be set to 1.*/
volatile bool mux_sleep_monitor_timer_need_restart = false; /*record if any cmd send before monitor sleep timer timeout.*/

typedef struct {
    mux_ll_header_t header;
    uint16_t data;
#ifdef MUX_LL_UART_CRC_CHECK_ENABLE
    uint16_t crc;
#endif
} __attribute__((packed)) mux_uart_wakeup_t;

uint8_t g_wakeup_uid = 0xFF;

ATTR_TEXT_IN_FAST_MEM static void mux_uart_sleep_lock_timer_callback(TimerHandle_t xtimer)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t ret;

    if (mux_sleep_lock_timer_need_restart) { /*received any cmd, restart timer*/
        mux_sleep_lock_timer_need_restart = false;
        ret = xTimerStartFromISR(mux_uart_sleep_lock_timer, &xHigherPriorityTaskWoken);
        LOG_MSGID_I(common, "[uart wakeup] sleep_lock_timer_callback, re-start ret %d", 1, ret);
    } else {
        mux_sleep_lock_timer_alive = false;
        if (hal_sleep_manager_is_sleep_handle_alive(mux_uart_rx_sleep_handle) == true) {
            hal_sleep_manager_unlock_sleep(mux_uart_rx_sleep_handle);
            LOG_MSGID_I(common, "[uart wakeup] sleep_lock_timer_callback, unlock sleep!", 0);
        }
    }
}

ATTR_TEXT_IN_FAST_MEM static void mux_uart_sleep_monitor_timer_callback(TimerHandle_t xtimer)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t ret;

    if (has_send_wakeup_cmd == true) { /*if send cmd and not recived response, then send wakeup cmd again.*/
        RB_LOG_W("[uart wakeup] retry wakeup, has_send_wakeup_cmd %d", 1, has_send_wakeup_cmd);
        mux_ll_uart_send_wakeup_cmd();
        return;
    }
    if (mux_sleep_monitor_timer_need_restart) { /*if any cmd is sent in gpt alive, restart timer*/
        mux_sleep_monitor_timer_need_restart = false;
        ret = xTimerStartFromISR(mux_uart_sleep_monitor_timer, &xHigherPriorityTaskWoken);
        LOG_MSGID_I(common, "[uart wakeup] sleep_monitor_timer_callback, re-start ret %d", 1, ret);
    } else {
        mux_sleep_monitor_timer_alive = false;
        has_send_wakeup_cmd = false;
        other_side_has_lock_sleep = false;

        ret = xTimerChangePeriodFromISR(mux_uart_sleep_monitor_timer, (10 / portTICK_PERIOD_MS), &xHigherPriorityTaskWoken);
        ret = xTimerStopFromISR(mux_uart_sleep_monitor_timer, &xHigherPriorityTaskWoken);
        LOG_MSGID_I(common, "[uart wakeup] sleep_monitor_timer_callback, change timer period to 10ms, stop timer. ret %d", 1, ret);
    }

    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void mux_uart_wakeup_timer_init(void)
{
    if (mux_uart_rx_sleep_handle == 0xFF) {
        mux_uart_rx_sleep_handle = hal_sleep_manager_set_sleep_handle("mux_ll");
        LOG_MSGID_I(common, "[uart wakeup] wakeup_timer_init, handle[%d]", 1, mux_uart_rx_sleep_handle);
    }


    if (mux_uart_tx_sleep_handle == 0xFF) {
        mux_uart_tx_sleep_handle = hal_sleep_manager_set_sleep_handle("mux_ll_tx");
        LOG_MSGID_I(common, "[uart wakeup] wakeup_timer_init, handle[%d]", 1, mux_uart_tx_sleep_handle);
    }

    if (mux_uart_sleep_lock_timer == NULL) {
        mux_uart_sleep_lock_timer = xTimerCreate("mux_ll_timer", (10 * 1000 / portTICK_PERIOD_MS), pdFALSE, NULL, mux_uart_sleep_lock_timer_callback);
        if (mux_uart_sleep_lock_timer == NULL) {
            if (hal_sleep_manager_is_sleep_handle_alive(mux_uart_rx_sleep_handle) == true) {
                hal_sleep_manager_unlock_sleep(mux_uart_rx_sleep_handle);
            }
            LOG_MSGID_I(common, "[uart wakeup] wakeup_timer_init fail", 0);
            return;
        }
    }

    if (mux_uart_sleep_monitor_timer == NULL) {
        mux_uart_sleep_monitor_timer = xTimerCreate("mux_ll_monitor_timer", (9 * 500 / portTICK_PERIOD_MS), pdFALSE, NULL, mux_uart_sleep_monitor_timer_callback);
        if (mux_uart_sleep_monitor_timer == NULL) {
            LOG_MSGID_I(common, "[uart wakeup] sleep_monitor_timer_init fail", 0);
            return;
        }
    }
}

ATTR_TEXT_IN_FAST_MEM void mux_uart_start_sleep_lock_timer(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t ret;

    if (!mux_sleep_lock_timer_alive) {
        if (HAL_NVIC_QUERY_EXCEPTION_NUMBER != 0) {
            ret = xTimerStartFromISR(mux_uart_sleep_lock_timer, &xHigherPriorityTaskWoken);
        } else {
            ret = xTimerStart(mux_uart_sleep_lock_timer, 0);
        }
        LOG_MSGID_I(common, "[uart wakeup] sleep_lock_timer start, ret %d", 1, ret);
    } else {
        if (HAL_NVIC_QUERY_EXCEPTION_NUMBER != 0) {
            ret = xTimerResetFromISR(mux_uart_sleep_lock_timer, &xHigherPriorityTaskWoken);
        } else {
            ret = xTimerReset(mux_uart_sleep_lock_timer, 0);
        }
        LOG_MSGID_I(common, "[uart wakeup] sleep_lock_timer restart, ret %d", 1, ret);
    }

    if (ret == pdTRUE) {
        mux_sleep_lock_timer_alive = true;
    }

    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    /*lock self is to prevent enter sleep and loss response.*/
    if (hal_sleep_manager_is_sleep_handle_alive(mux_uart_rx_sleep_handle) == true) {
        hal_sleep_manager_unlock_sleep(mux_uart_rx_sleep_handle);
    }
    hal_sleep_manager_lock_sleep(mux_uart_rx_sleep_handle);
}

ATTR_TEXT_IN_FAST_MEM void mux_uart_start_sleep_monitor_timer(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t ret;

    if (!mux_sleep_monitor_timer_alive) {
        if (HAL_NVIC_QUERY_EXCEPTION_NUMBER != 0) {
            ret = xTimerStartFromISR(mux_uart_sleep_monitor_timer, &xHigherPriorityTaskWoken);
        } else {
            ret = xTimerStart(mux_uart_sleep_monitor_timer, 0);
        }
        LOG_MSGID_I(common, "[uart wakeup] sleep_monitor_timer start, ret %d", 1, ret);
    } else {
        if (HAL_NVIC_QUERY_EXCEPTION_NUMBER != 0) {
            ret = xTimerResetFromISR(mux_uart_sleep_monitor_timer, &xHigherPriorityTaskWoken);
        } else {
            ret = xTimerReset(mux_uart_sleep_monitor_timer, 0);
        }
        LOG_MSGID_I(common, "[uart wakeup] sleep_monitor_timer restart, ret %d", 1, ret);
    }

    if (ret == pdTRUE) {
        mux_sleep_monitor_timer_alive = true;
    }

    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_parse_wakeup_cmd(void)
{
    uint32_t uid = 0xF1;

    if (is_recieved_wakeup_cmd == false) {
        LOG_MSGID_I(common, "[uart wakeup] recv wakeup cmd, lock sleep", 0);
        is_recieved_wakeup_cmd = true;
        mux_uart_start_sleep_lock_timer(); /*received cmd then start timer and lock sleep 10s.*/
        MUX_SET_TX_FLAGS(uid, MUX_TX_TRIGGER_OWNER_WAKEUP);
        hal_nvic_set_pending_irq(SW_IRQn);
    } else {
        LOG_MSGID_I(common, "[uart wakeup] wakeup cmd have already received.", 0);
    }
}

ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_wakeup_restore(void)
{
    if (g_wakeup_uid == 0xFF) {
        assert(0 && "wakeup uid not init!!");
    }
    // if (g_mux_ll_config->device_mode != DCHS_MODE_SINGLE) {
    g_mux_ll_user_context[g_wakeup_uid].rx_pkt_seq++;
    mux_ll_uart_parse_wakeup_cmd();
    // }
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_parse_wakeup_cmd_response(void)
{
    uint32_t uid = 0xF2;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t ret;

    LOG_MSGID_I(common, "[uart wakeup] recv wakeup response, other_side_has_lock_sleep=%u", 1, other_side_has_lock_sleep);
    if (other_side_has_lock_sleep == true) { /*if this flag == ture, means the other side wakeup from sleep, and 2 times responses be sent(uart sleep restore & rx deliver)*/
        return;
    } else {
        other_side_has_lock_sleep = true;
        has_send_wakeup_cmd = false; /*have received response, set has_send_wakeup_cmd to false, trigger tx limiter to send pending data.*/

        //mux_uart_start_sleep_monitor_timer();

        /*if wakeup sync done, then unlock sleep.*/
        if (hal_sleep_manager_is_sleep_handle_alive(mux_uart_tx_sleep_handle) == true) {
            hal_sleep_manager_unlock_sleep(mux_uart_tx_sleep_handle);
        }

        ret = xTimerChangePeriodFromISR(mux_uart_sleep_monitor_timer, (9 * 500 / portTICK_PERIOD_MS), &xHigherPriorityTaskWoken);
        LOG_MSGID_I(common, "[uart wakeup] parse_wakeup_cmd_response, change timer period to 4.5s, ret %d, unlock", 1, ret);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

        MUX_SET_TX_FLAGS(uid, MUX_TX_TRIGGER_OWNER_WAKEUP);
        hal_nvic_set_pending_irq(SW_IRQn);
    }
}

ATTR_TEXT_IN_TCM void dchs_uart_wakeup_callback(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data)
{
    mux_buffer_t buffer;
    mux_status_t mux_status;
    uint32_t read_done_size = 0;
    uint8_t user_rx_fifo[256];
    #define handle_to_user_id(handle) ((handle >> 8) & 0xFF)

    buffer.p_buf = user_rx_fifo;
    buffer.buf_size = sizeof(user_rx_fifo);

    switch (event) {
    case MUX_EVENT_READY_TO_READ:
        mux_status = mux_rx(handle, &buffer, &read_done_size);
        if (mux_status != MUX_STATUS_OK) {
            RB_LOG_E("[uart wakeup] [cmd_callback] mux_rx fail:%d", 1, mux_status);
            assert(0);
        } else {
            if (read_done_size > 0) {
                // RB_LOG_I("[uart latch] [cmd_callback][MCU] dlen=%d uid=%d", 2, read_done_size, handle_to_user_id(handle));
                uint16_t wakeup_cmd = *(uint16_t *)user_rx_fifo;
                if (wakeup_cmd == MUX_UART_WAKEUP_CMD) {
                    mux_ll_uart_parse_wakeup_cmd();
                    return;
                } else if (wakeup_cmd == MUX_UART_WAKEUP_CMD_RESPONSE) {
                    mux_ll_uart_parse_wakeup_cmd_response();
                    return;
                } else {
                    RB_LOG_E("[uart wakeup] [cmd_callback] error wakeup command:0x%x", 1, wakeup_cmd);
                }
            }
        }

        break;
    case MUX_EVENT_READY_TO_WRITE:
        RB_LOG_I("[uart wakeup] [cmd_callback] MUX_EVENT_READY_TO_WRITE", 0);
        break;
    default:
        RB_LOG_W("[uart wakeup] [cmd_callback] event %d no task", 1, event);
    }
}

ATTR_TEXT_IN_FAST_MEM  void mux_ll_uart_send_wakeup_data(uint32_t data)
{
    mux_ll_user_context_t *user_context = &g_mux_ll_user_context[g_wakeup_uid];
    mux_uart_wakeup_t wakeup_data;
    uint32_t tx_len;

    mux_ll_uart_format_header(&wakeup_data.header, g_wakeup_uid, 2);

    wakeup_data.data = data;
#ifdef MUX_LL_UART_CRC_CHECK_ENABLE
    wakeup_data.crc = 0;
    BYTE_CRC16_NR(wakeup_data.crc, data & 0xff);
    BYTE_CRC16_NR(wakeup_data.crc, (data & 0xff00) >> 8);
#endif
    tx_len = mux_ll_uart_send_to_vfifo((uint8_t*)&wakeup_data, sizeof(wakeup_data), true);
    if (tx_len == 0) {
        RB_LOG_W("[send vfifo] vfifo full", 0);
    }
    LOG_MSGID_I(common, "[uart wakeup][mux_tx_vfifo] uid[%x], data=%x tx_vd_len=%u, rx_vd_len=%u, vwm=%u, bt_ts=%u, gpt_ts=%u", 7,\
        g_wakeup_uid|0x80, data, mux_ringbuffer_data_length(&g_port_config.txbuf), mux_ringbuffer_data_length(&g_port_config.rxbuf),\
        g_port_config.txbuf.water_mark, user_context->tx_bt_timestamp, user_context->tx_gpt_timestamp);
}

ATTR_TEXT_IN_FAST_MEM  void mux_ll_uart_send_wakeup_response(void)
{
    mux_ll_uart_send_wakeup_data(MUX_UART_WAKEUP_CMD_RESPONSE);
    is_recieved_wakeup_cmd = false;
}

ATTR_TEXT_IN_FAST_MEM  void mux_ll_uart_send_wakeup_cmd(void)
{
    mux_ll_uart_send_wakeup_data(MUX_UART_WAKEUP_CMD);

    /*lock self after send wakeup cmd, otherwise enter deepsleep after send wakeup cmd, then will miss receive wakeup response.*/
    if (hal_sleep_manager_is_sleep_handle_alive(mux_uart_tx_sleep_handle) == true) {
        hal_sleep_manager_unlock_sleep(mux_uart_tx_sleep_handle);
    }
    hal_sleep_manager_lock_sleep(mux_uart_tx_sleep_handle);

    mux_uart_start_sleep_monitor_timer();
}


ATTR_TEXT_IN_FAST_MEM bool mux_ll_uart_wakeup_sync(void)
{
    uint32_t irq_save = 0;

    hal_nvic_save_and_set_interrupt_mask(&irq_save);
    if (is_recieved_wakeup_cmd == true) { /*if received wakeup cmd, send response first*/
        hal_nvic_restore_interrupt_mask(irq_save);
        mux_ll_uart_send_wakeup_response();
        return false;
    }

    if (other_side_has_lock_sleep == false) { /*after receive wakeup response, set other_side_has_lock_sleep to record other side wakeup status*/
        if (has_send_wakeup_cmd == false) { /*has send wakeup cmd, but not received response.*/
            has_send_wakeup_cmd = true;
            hal_nvic_restore_interrupt_mask(irq_save);
            mux_ll_uart_send_wakeup_cmd();
        } else {
            hal_nvic_restore_interrupt_mask(irq_save);
        }
        return false;
    }

    if (!mux_sleep_monitor_timer_need_restart) {
        mux_sleep_monitor_timer_need_restart = true;
    }
    hal_nvic_restore_interrupt_mask(irq_save);

    return true;
}

ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_reset_lock_sleep_timer(void)
{
    uint32_t irq_save = 0;
    hal_nvic_save_and_set_interrupt_mask(&irq_save);
    if (!mux_sleep_lock_timer_need_restart) {
        mux_sleep_lock_timer_need_restart = true;
    }
    hal_nvic_restore_interrupt_mask(irq_save);
}

void mux_ll_uart_wakeup_init(void)
{
    mux_status_t status;

    status = mux_open(MUX_LL_UART_PORT, "WAKEUP", &wakeup_handle, dchs_uart_wakeup_callback, NULL);
    if(MUX_STATUS_OK != status) {
        RB_LOG_E("[uart wakeup] mux_open fail, status=%d handle=0x%x", 2, status, wakeup_handle);
        assert(0);
        return;
    } else {
        RB_LOG_I("[uart wakeup] mux_open ok handle=0x%x", 1, wakeup_handle);
    }
    for (uint32_t i = 0; i < MAX_MUX_LL_USER_COUNT; i++) {
        if (!strcmp(g_mux_ll_uart_user_configure_table[i].name, "WAKEUP")) {
            g_wakeup_uid = (uint8_t)i;
        }
    }

    if (g_wakeup_uid == 0xFF) {
        assert(0 && "[uart wakeup] can not find wakeup uid!!");
    }
    mux_uart_wakeup_timer_init();

    mux_uart_start_sleep_lock_timer();
    mux_uart_start_sleep_monitor_timer();
}
#endif
