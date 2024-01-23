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

#include "hal_resource_assignment.h"
#include "hal_pdma_internal.h"
#include "hal_uart_internal.h"

#if !defined(AIR_DCHS_MODE_MASTER_ENABLE) && !defined(AIR_DCHS_MODE_SLAVE_ENABLE)
#error "Master or Slave must be defined!!"
#endif
#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#endif/*HAL_SLEEP_MANAGER_ENABLED*/

typedef struct {
    uint16_t id;
    uint16_t latch_counter;
    uint32_t native_clock;
    uint32_t native_phase;
    uint32_t dl_src_clock_offset;
    uint32_t dl_src_phase_offset;
    uint32_t valid;
    uint16_t init_capid;
    uint16_t current_capid;
} mux_ll_uart_latch_info_t;

typedef struct {
    uint32_t tx_lock_start_count;
    uint32_t tx_lock_max_duration;
    uint32_t tx_lock_duration;
    uint32_t latch_start_count;
    uint32_t latch_duration_count;
    uint32_t latch_lock_sleep_duration;
    uint32_t clock_offset_us;
    uint32_t clock_offset_us_for_deviation_check;
    uint32_t clock_offset_us_out_of_range_counter;
    bool     is_clock_offset_us_out_of_range;
} mux_ll_uart_latch_timestamp_t;

typedef enum {
    LATCH_STATE_LATCH_INIT               = 0,
    LATCH_STATE_WAIT_LATCH_TIMER_EXPIRE  = 1,
    LATCH_STATE_LATCH_START              = 2,
    LATCH_STATE_WAIT_BT_LOCK_SLEEP_DONE  = 3,
    LATCH_STATE_LATCH_REQUEST_PENDING    = 4,
    LATCH_STATE_WAIT_LATCH_RESPONSE      = 5,
    LATCH_STATE_WAIT_LATCH_INFO          = 6,
    LATCH_STATE_WAIT_LATCH_ACTION        = 7,
    LATCH_STATE_MAX                      = 8,
} mux_ll_uart_latch_state_t;

typedef struct {
    mux_ll_uart_latch_state_t latch_state;
    bool bt_clock_synced;
#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
    uint32_t latch_counter;
    uint32_t monitor_timer_handle;
#endif
    uint32_t target_clk_offset_us;
    int32_t phone_master_current_clk_offset_us;
    int32_t phone_master_target_clk_offset_us;
    uint16_t init_capid;
    uint16_t native_capid;
    uint16_t peer_dev_capid;
} mux_ll_uart_latch_context_t;

mux_handle_t uart_latch_handle;

mux_ll_uart_latch_timestamp_t latch_ts = {0};

#define INIT_DEFAULT_CAPID 0xFFFF

mux_ll_uart_latch_context_t latch_context = {
    .latch_state = LATCH_STATE_LATCH_INIT,
    .bt_clock_synced = false,
    .target_clk_offset_us = 0,
    .phone_master_current_clk_offset_us = 0,
    .phone_master_target_clk_offset_us = 0,
    .init_capid = INIT_DEFAULT_CAPID,
    .native_capid = INIT_DEFAULT_CAPID,
    .peer_dev_capid = INIT_DEFAULT_CAPID,
#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
    .latch_counter = 0,
#endif
};

static void mux_ll_uart_latch_callback(uint8_t *buff, uint32_t len);

#define DCHS_ID1_S2M_LATCH_REQ         0x1000
#define DCHS_ID3_S2M_LATCH_ACTION      0x1001
#define DCHS_ID5_S2M_LATCH_INFO        0x1002
#define DCHS_ID5_S2M_LATCH_END         0x1003

#define DCHS_ID2_M2S_LATCH_RSP         0x2000
#define DCHS_ID4_M2S_LATCH_INFO        0x2001

#define __ABS(x) ((x) < 0 ? -(x) : x)

typedef int (*capid_calc_callback_t)(uint32_t *capid_addr, uint32_t init_capid, int32_t curent_clk_offset_us, int32_t target_clk_offset_us);

extern void DCHS_pka_lock_bt_sleep(void (*lock_sleep_done_callback)(void* user_data), void* user_data);
extern void DCHS_pka_unlock_bt_sleep(void);
extern void DCHS_bt_pka_get_uart_latch_info(mux_ll_uart_latch_info_t *);
extern void DCHS_bt_pka_set_audio_clk_offset(bool, mux_ll_uart_latch_info_t *, uint32_t latch_counter, capid_calc_callback_t capid_calc_callback);
extern uint32_t get_capid(void);

static void mux_ll_uart_bt_sleep_lock_done_callback(void* user_data);

#define DCHS_LOCK_BT_SLEEP()       do {\
    RB_LOG_I("[uart latch] lock sleep", 0);\
    uint32_t start_count = mux_get_gpt_tick_count();\
    uint32_t elapse_count;\
    DCHS_pka_lock_bt_sleep(mux_ll_uart_bt_sleep_lock_done_callback, NULL);\
    elapse_count = mux_get_tick_elapse(start_count);\
    if (elapse_count > 1000) {\
        RB_LOG_W("[uart latch] lock sleep cost %uus", 1, elapse_count);\
    }\
} while (0)

#define DCHS_UNLOCK_BT_SLEEP()     do {\
    DCHS_pka_unlock_bt_sleep();\
    RB_LOG_I("[uart latch] unlock sleep", 0);\
} while (0)

static bool dchs_uart_latch_enable = false;
#define DCHS_LATCH_MODE_ENABLE()    do {dchs_uart_latch_enable = true;} while (0)
#define DCHS_LATCH_MODE_DISABLE()   do {dchs_uart_latch_enable = false;} while (0)
#define DCHS_LATCH_MODE_CHECK_ENABLE() (dchs_uart_latch_enable == true)

typedef void (*mux_ll_uart_latch_timer_callback_t)(void*);
static bool mux_ll_uart_send_latch_req(uint16_t id, uint16_t tag);

#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
    extern void DCHS_bt_pka_enter_uart_tx_latch_mode(void);
#elif defined(AIR_DCHS_MODE_MASTER_ENABLE)
    extern void DCHS_bt_pka_enter_uart_rx_latch_mode(void);
#else
#error "ERROR DEVICE!!!"
#endif

typedef struct {
    mux_ll_uart_latch_timer_callback_t callback;
    void *user_data;
    uint32_t timer_handle;
    uint32_t timeout;
    bool is_one_shot;
    uint8_t counter;
} mux_ll_uart_latch_timer_t;

static void mux_ll_uart_latch_period_tx_callback(void * user_data);

static mux_ll_uart_latch_timer_t g_latch_timer = {.timeout = 0xFFFFFFFF};

int dchs_slave_capid_calc_func(uint32_t *capid_addr, uint32_t init_dcxo_capid, int32_t slave_master_curent_clk_offset_us, int32_t slave_master_target_clk_offset_us)
{
    (void)capid_addr;
    (void)init_dcxo_capid;
    (void)slave_master_curent_clk_offset_us;
    (void)slave_master_target_clk_offset_us;
    int32_t d_clk_off_in_us;
    uint32_t dcxo_capid = *capid_addr;

    RB_LOG_I("[uart latch] dchs_slave_capid_calc_func,capid_addr:%u slave_master_curent_clk_offset_us:%d slave_master_target_clk_offset_us:%d native_capid=%u peer_dev_capid=%u, diff=%d", 6, \
    *capid_addr, slave_master_curent_clk_offset_us, slave_master_target_clk_offset_us, latch_context.native_capid, latch_context.peer_dev_capid, latch_context.peer_dev_capid - latch_context.native_capid);

	d_clk_off_in_us = slave_master_curent_clk_offset_us - slave_master_target_clk_offset_us;
    if (d_clk_off_in_us == 0) {
        return 0;
    }

    if (d_clk_off_in_us < -3) {
        if (dcxo_capid < 511) {
            dcxo_capid += 3;
            if (dcxo_capid > 511) {
                dcxo_capid = 511;
            }
        }
    } else if (d_clk_off_in_us > 3) {
        if (dcxo_capid > 50 ) {
            dcxo_capid -= 3;
        }
    } else {
        dcxo_capid = latch_context.peer_dev_capid;
    }

    *capid_addr = dcxo_capid;
    return 0;
}

int dchs_master_capid_calc_func(uint32_t *capid_addr, uint32_t init_dcxo_capid, int32_t phone_master_current_clk_offset_us, int32_t phone_master_target_clk_offset_us)
{
    (void)capid_addr;
    (void)init_dcxo_capid;
    (void)phone_master_target_clk_offset_us;
    (void)phone_master_current_clk_offset_us;
    int32_t d_clk_off_in_us;

    latch_context.phone_master_current_clk_offset_us = phone_master_current_clk_offset_us;
    latch_context.phone_master_target_clk_offset_us = phone_master_target_clk_offset_us;
    d_clk_off_in_us = phone_master_current_clk_offset_us - phone_master_target_clk_offset_us;
    RB_LOG_I("[uart latch] dchs_master_capid_calc_func,capid_addr:%u phone_master_target_clk_offset_us:%d phone_master_current_clk_offset_us:%d phone_master_clk_diff_us:%d", 4, \
        *capid_addr, phone_master_target_clk_offset_us, phone_master_current_clk_offset_us, d_clk_off_in_us);
#if 1
	return -1;
#else

	uint32_t dcxo_capid = *capid_addr;
    if (d_clk_off_in_us < -3) {
        if (dcxo_capid < 450) {
            dcxo_capid += 1;
        }
    } else if (d_clk_off_in_us > 3) {
        if (dcxo_capid > 70) {
            dcxo_capid -= 1;
        }
    }

    *capid_addr = dcxo_capid;

    return 0;
#endif
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_timer_callback(void * user_data)
{
    mux_ll_uart_latch_timer_t *latch_timer = (mux_ll_uart_latch_timer_t *)user_data;
    if (latch_context.latch_state != LATCH_STATE_WAIT_LATCH_TIMER_EXPIRE) {
        RB_LOG_W("[uart latch] s0: Error state:%u, it should be %u", 2, latch_context.latch_state, LATCH_STATE_WAIT_LATCH_TIMER_EXPIRE);
        DCHS_UNLOCK_BT_SLEEP();
        DCHS_UNLOCK_UART_TX();
    }

    latch_context.latch_state = LATCH_STATE_LATCH_START;
    latch_timer->callback(latch_timer->user_data);

    if (!latch_timer->is_one_shot) {
        hal_gpt_sw_start_timer_ms(latch_timer->timer_handle, latch_timer->timeout, mux_ll_uart_latch_timer_callback, user_data);
    }
}


void mux_ll_latch_timer_start(bool is_one_shot, uint32_t timeout)
{
    LOG_MSGID_I(common, "[uart latch] s0 [mux_ll_latch_timer_start] %u:%u", 2, is_one_shot, timeout);
    if (g_latch_timer.counter == 0) {
        hal_gpt_sw_get_timer(&g_latch_timer.timer_handle);
    }
    latch_context.latch_state = LATCH_STATE_WAIT_LATCH_TIMER_EXPIRE;
    g_latch_timer.is_one_shot = is_one_shot;
    if (timeout < g_latch_timer.timeout)
        g_latch_timer.timeout = timeout;
    g_latch_timer.counter++;
    g_latch_timer.callback = mux_ll_uart_latch_period_tx_callback;
    g_latch_timer.user_data = NULL;
    hal_gpt_sw_start_timer_ms(g_latch_timer.timer_handle, 0, mux_ll_uart_latch_timer_callback, (void*)&g_latch_timer);

}

void mux_ll_latch_timer_stop(void)
{
    if (g_latch_timer.counter && (--g_latch_timer.counter == 0)) {
        hal_gpt_sw_stop_timer_ms(g_latch_timer.timer_handle);
        hal_gpt_sw_free_timer(g_latch_timer.timer_handle);
    }
    latch_ts.clock_offset_us_for_deviation_check = 0;
    latch_ts.is_clock_offset_us_out_of_range = false;
#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
    latch_context.latch_counter = 0;
#endif
    latch_context.bt_clock_synced = false;
    mux_ll_uart_send_latch_req(DCHS_ID5_S2M_LATCH_END, 0x2);
    LOG_MSGID_I(common, "[uart latch] [mux_ll_latch_timer_stop] counter=%u", 1, g_latch_timer.counter);
}

static ATTR_TEXT_IN_TCM void mux_ll_uart_latch_cmd_callback(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data)
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
                mux_ll_uart_latch_callback(buffer.p_buf, read_done_size);
            } else {
                RB_LOG_E("[uart wakeup] [cmd_callback] mux_rx read_done_size = 0", 0);
            }
        }
        break;
    case MUX_EVENT_READY_TO_WRITE:
        RB_LOG_I("[uart latch] [cmd_callback] MUX_EVENT_READY_TO_WRITE", 0);
        break;
    default:
        RB_LOG_I("[uart latch] [cmd_callback] event %d no task", 1, event);
    }
}

void mux_ll_uart_latch_init(void)
{
    mux_status_t status;

    status = mux_open(MUX_LL_UART_PORT, "LATCH", &uart_latch_handle, mux_ll_uart_latch_cmd_callback, NULL);
    if(MUX_STATUS_OK != status) {
        assert(0 && "[uart latch] mux_open fail");
    } else {
        RB_LOG_I("[uart latch] mux_open ok handle=0x%x", 1, uart_latch_handle);
    }
    if (latch_context.init_capid == INIT_DEFAULT_CAPID) {
        latch_context.init_capid = get_capid();
    }
    DCHS_LOCK_UART_TX();
#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
    hal_gpt_sw_get_timer(&latch_context.monitor_timer_handle);
    /* The first latch action should not start too early, in case of the another chip has not been bootup. */
    // mux_ll_latch_timer_start(true, 500);
#endif
}

static ATTR_TEXT_IN_FAST_MEM bool mux_ll_uart_latch_send_cmd_data(uint8_t *buf, uint32_t len)
{
    // RB_LOG_I("[uart latch] [mux_ll_uart_latch_send_cmd_data] uart_latch_handle=0x%x", 1, uart_latch_handle);

    mux_buffer_t buffer = {buf, len};
    mux_status_t status;
    uint32_t send_done_len = 0;
    status = mux_tx(uart_latch_handle, &buffer, 1, &send_done_len);
    if (status != MUX_STATUS_OK) {
        assert(0 && "mux_tx fail");
    }
    if (len != send_done_len) {
        RB_LOG_I("[uart latch] [mux_ll_uart_latch_send_cmd_data] tx buffer full", 0);
    }
    // mux_hexdump_ll(uart_latch_handle, __func__, true, true);
    return true;
}

static ATTR_TEXT_IN_FAST_MEM bool mux_ll_uart_send_latch_req(uint16_t id, uint16_t tag)
{
    uint16_t buf[2] = {id, tag};

    return mux_ll_uart_latch_send_cmd_data((uint8_t*)&buf[0], sizeof(buf));
}

static ATTR_TEXT_IN_FAST_MEM bool mux_ll_uart_send_latch_info(mux_ll_uart_latch_info_t *latch_info, uint16_t id)
{
    latch_info->id = id;
    return mux_ll_uart_latch_send_cmd_data((uint8_t*)latch_info, sizeof(mux_ll_uart_latch_info_t));
}

ATTR_TEXT_IN_FAST_MEM uint32_t mux_ll_uart_get_clock_offset_us(void)
{
    return latch_ts.clock_offset_us;
}
ATTR_TEXT_IN_FAST_MEM bool mux_ll_uart_is_bt_clock_synced(void)
{
    return latch_context.bt_clock_synced;
}

/* This is used to start uart latch request */
static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_period_tx_callback(void * user_data)
{
    if (latch_context.latch_state == LATCH_STATE_LATCH_START) {
        latch_context.latch_state = LATCH_STATE_WAIT_BT_LOCK_SLEEP_DONE;
        RB_LOG_I("[uart latch] s0 [latch_timer_cb] vd_len:%u", 1, mux_ringbuffer_data_length(&g_port_config.txbuf));
        latch_ts.latch_start_count = mux_get_gpt_tick_count();
        DCHS_LOCK_BT_SLEEP();
        DCHS_LATCH_MODE_ENABLE();
    } else {
        RB_LOG_W("[uart latch] s0: Error state:%u, it should be %u", 2, latch_context.latch_state, LATCH_STATE_LATCH_START);
    }
}

#ifdef AIR_DCHS_MODE_MASTER_ENABLE
static ATTR_TEXT_IN_FAST_MEM uint32_t mux_ll_uart_latch_check(mux_ll_uart_latch_info_t *info_from_slv, mux_ll_uart_latch_info_t *info_to_slv)
{
    uint32_t clock_offset_us;
    int32_t start_count;
    int32_t end_count;
    mux_ll_uart_latch_info_t *info_bkp;

    static mux_ll_uart_latch_info_t latch_info_to_slv_bkp = {0};

    info_bkp = &latch_info_to_slv_bkp;
    start_count = NATIVE_CLK_PHASE_2_US(info_from_slv->native_clock, info_from_slv->native_phase);
    end_count = NATIVE_CLK_PHASE_2_US(info_to_slv->native_clock, info_to_slv->native_phase);
    clock_offset_us = DCHS_bt_get_native_clock_us_duration(start_count, end_count);

    if ((info_bkp->native_clock == info_to_slv->native_clock) && (info_bkp->native_phase == info_to_slv->native_phase)) {
        assert(0 && "[uart latch] latch fail");
    }
    memcpy(info_bkp, info_to_slv, sizeof(mux_ll_uart_latch_info_t));
    return clock_offset_us;
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_do_latch_req(void)
{
    if (latch_context.latch_state == LATCH_STATE_LATCH_START) {
        latch_context.latch_state = LATCH_STATE_WAIT_BT_LOCK_SLEEP_DONE;
        latch_ts.latch_start_count = mux_get_gpt_tick_count();
        RB_LOG_I("[uart latch] m1<-s2", 0);
        DCHS_LOCK_BT_SLEEP();
    } else {
        RB_LOG_W("[uart latch] m2: Error state:%u, it should be %u", 2, latch_context.latch_state, LATCH_STATE_LATCH_START);
    }
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_do_lock_sleep_done(void)
{
    if (latch_context.latch_state == LATCH_STATE_WAIT_BT_LOCK_SLEEP_DONE) {
        latch_context.latch_state = LATCH_STATE_WAIT_LATCH_ACTION;
        latch_ts.latch_lock_sleep_duration = mux_get_tick_elapse(latch_ts.latch_start_count);
        RB_LOG_I("[uart latch] m2: lock_sleep_duration=%d", 1, latch_ts.latch_lock_sleep_duration);

        DCHS_bt_pka_enter_uart_rx_latch_mode();
        mux_ll_uart_send_latch_req(DCHS_ID2_M2S_LATCH_RSP, 0x2); //send latch rsp, let master know that slave has already entered latch mode
    } else {
        RB_LOG_W("[uart latch] m2: Error state:%u, it should be %u", 2, latch_context.latch_state, LATCH_STATE_WAIT_BT_LOCK_SLEEP_DONE);
    }
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_do_latch_action(mux_ll_uart_latch_info_t *info_to_slv)
{
    //master has already been lathched when slave receive this cb
    DCHS_bt_pka_get_uart_latch_info(info_to_slv);
    DCHS_UNLOCK_BT_SLEEP();
    info_to_slv->current_capid = get_capid();
    latch_context.native_capid = info_to_slv->current_capid;
    RB_LOG_W("[uart latch] m3<-s3: native_phase=%d native_clock=%d dl_src_clock_offset=%d dl_src_phase_offset=%d native_capid=%u peer_dev_capid=%u", 6, \
    info_to_slv->native_phase, info_to_slv->native_clock, (int)info_to_slv->dl_src_clock_offset, (int)info_to_slv->dl_src_phase_offset, latch_context.native_capid, latch_context.peer_dev_capid);
    mux_ll_uart_send_latch_info(info_to_slv, DCHS_ID4_M2S_LATCH_INFO);
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_do_recv_slv_latch_info(mux_ll_uart_latch_info_t *info_from_slv, mux_ll_uart_latch_info_t *info_to_slv)
{
    uint32_t clock_offset_us;
    uint32_t current_capid;
    static uint32_t target_clk_offset_us = 0;
    latch_ts.latch_duration_count = mux_get_tick_elapse(latch_ts.latch_start_count);

#ifdef MUX_LL_UART_BTCLK_SYNC_P34_ENABLE
    DCHS_bt_pka_set_audio_clk_offset(TRUE, info_from_slv, info_from_slv->latch_counter, dchs_master_capid_calc_func);
#endif
    current_capid = latch_context.native_capid;
    clock_offset_us = mux_ll_uart_latch_check(info_from_slv, info_to_slv);
    if (info_from_slv->latch_counter == 5) {
        target_clk_offset_us = clock_offset_us;
        latch_context.target_clk_offset_us = target_clk_offset_us;
    }
    latch_context.peer_dev_capid = info_from_slv->current_capid;

    LOG_MSGID_I(common, "[uart latch] m4<-s4: latch_cost:%4dus, mst_clock:%7d, mst_phase:%4d, slv_clk:%7d, slv_phase:%4d, co_diff:%dus latch_counter:%u, ms_clk_offset:(target:%dus current:%dus diff:%d), pm_clk_offset(target:%dus current:%dus diff:%d), init_capid(m:%u s:%u), curr_capid(m:%u mmdiff:%d s:%u ssdiff:%d smdiff:%d)", 20, \
        latch_ts.latch_duration_count, info_to_slv->native_clock, info_to_slv->native_phase, info_from_slv->native_clock, info_from_slv->native_phase, \
        (int)(clock_offset_us - latch_ts.clock_offset_us), info_from_slv->latch_counter, (int)target_clk_offset_us, (int)(clock_offset_us), \
        (int)(__ABS(clock_offset_us)) - (int)__ABS(target_clk_offset_us), latch_context.phone_master_target_clk_offset_us, latch_context.phone_master_current_clk_offset_us, \
        latch_context.phone_master_current_clk_offset_us - latch_context.phone_master_target_clk_offset_us, latch_context.init_capid, info_from_slv->init_capid, current_capid, \
        (int)(current_capid - latch_context.init_capid), info_from_slv->current_capid, (int)(info_from_slv->current_capid - info_from_slv->init_capid), \
        (int)(info_from_slv->current_capid - current_capid));

    latch_ts.clock_offset_us = (int32_t)clock_offset_us;
    latch_context.bt_clock_synced = true;
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_callback(uint8_t *buff, uint32_t len)
{
    mux_ll_uart_latch_info_t latch_info_from_slv;
    static mux_ll_uart_latch_info_t latch_info_to_slv;

    memcpy(&latch_info_from_slv, buff, len);

    switch(latch_info_from_slv.id) {
    case DCHS_ID1_S2M_LATCH_REQ   :
        if (latch_context.latch_state != LATCH_STATE_LATCH_INIT) {
            DCHS_UNLOCK_BT_SLEEP();
        }
        latch_context.latch_state = LATCH_STATE_LATCH_START;
        mux_ll_uart_latch_do_latch_req();
        break;
    case DCHS_ID3_S2M_LATCH_ACTION:
        if (latch_context.latch_state == LATCH_STATE_WAIT_LATCH_ACTION) {
            latch_context.latch_state = LATCH_STATE_WAIT_LATCH_INFO;
            mux_ll_uart_latch_do_latch_action(&latch_info_to_slv);
        } else {
            RB_LOG_W("[uart latch] m3: Error state:%u, it should be %u", 2, latch_context.latch_state, LATCH_STATE_WAIT_LATCH_ACTION);
        }
        break;
    case DCHS_ID5_S2M_LATCH_INFO  :
        if (latch_context.latch_state == LATCH_STATE_WAIT_LATCH_INFO) {
            latch_context.latch_state = LATCH_STATE_LATCH_INIT;
            mux_ll_uart_latch_do_recv_slv_latch_info(&latch_info_from_slv, &latch_info_to_slv);
        } else {
            RB_LOG_W("[uart latch] m3: Error state:%u, it should be %u", 2, latch_context.latch_state, LATCH_STATE_WAIT_LATCH_INFO);
        }
        break;
    case DCHS_ID5_S2M_LATCH_END  :
        latch_context.latch_state = LATCH_STATE_LATCH_INIT;
        latch_context.bt_clock_synced = false;
        RB_LOG_W("[uart latch] latch end", 0);
        break;
    default:
        RB_LOG_E("[uart latch] Error id:0x%x!!", 1, latch_info_from_slv.id);
        break;
    }
}

#endif //AIR_DCHS_MODE_MASTER_ENABLE

#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_monitor_timer_callback(void * user_data)
{
    latch_context.latch_state = LATCH_STATE_LATCH_INIT;
    if (DCHS_IS_UART_TX_LOCKED()) {
        DCHS_UNLOCK_UART_TX();
        DCHS_LATCH_MODE_DISABLE();
        // assert(0 && "[uart latch] s2: last latch req maybe fail, unlock uart tx");
    }
    RB_LOG_W("[uart latch] s2: lock uart tx too long, latch_state:%u latch_mode:%u is_tx_locked:%u", 3, \
        latch_context.latch_state, DCHS_LATCH_MODE_CHECK_ENABLE(), DCHS_IS_UART_TX_LOCKED());
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_check_clock_deviation(uint32_t clock_offset_us)
{
    if (latch_ts.clock_offset_us_for_deviation_check != 0) {
        /* 26M 200ppm  1s offset 200us. BT lock sync every 100ms ,BT native clock offset must not be beyond 20us. Go easy on the restrictions, we set 100us. */
        if (__ABS((int)(clock_offset_us - latch_ts.clock_offset_us_for_deviation_check)) > 100) {
            if (latch_ts.is_clock_offset_us_out_of_range && (latch_ts.clock_offset_us_out_of_range_counter <= 1)) {
                //assert(0 && "[uart latch] co_diff out of range");
                RB_LOG_E("[uart latch] co_diff out of range 1", 0);
                latch_ts.clock_offset_us_out_of_range_counter++;
            } else if (latch_ts.is_clock_offset_us_out_of_range && (latch_ts.clock_offset_us_out_of_range_counter == 2)) {
                assert(0 && "[uart latch] co_diff out of range");
            } else {
                RB_LOG_E("[uart latch] co_diff out of range 2", 0);
            }
            latch_ts.is_clock_offset_us_out_of_range = true;
        } else {
            latch_ts.clock_offset_us_out_of_range_counter = 0;
        }
    }

    latch_ts.clock_offset_us_for_deviation_check = clock_offset_us;
}
static ATTR_TEXT_IN_FAST_MEM uint32_t mux_ll_uart_latch_check(mux_ll_uart_latch_info_t *info_from_mst, mux_ll_uart_latch_info_t *info_to_mst)
{
    uint32_t clock_offset_us;
    int32_t start_count;
    int32_t end_count;
    mux_ll_uart_latch_info_t *info_bkp;

    static mux_ll_uart_latch_info_t latch_info_to_mst_bkp = {0};

    info_bkp = &latch_info_to_mst_bkp;
    start_count = NATIVE_CLK_PHASE_2_US(info_from_mst->native_clock, info_from_mst->native_phase);
    end_count = NATIVE_CLK_PHASE_2_US(info_to_mst->native_clock, info_to_mst->native_phase);
    clock_offset_us = DCHS_bt_get_native_clock_us_duration(start_count, end_count);

    mux_ll_uart_latch_check_clock_deviation(clock_offset_us);

    latch_ts.clock_offset_us = (int32_t)clock_offset_us;

    if ((info_bkp->native_clock == info_to_mst->native_clock) && (info_bkp->native_phase == info_to_mst->native_phase)) {
        assert(0 && "[uart latch] latch fail");
    }
    memcpy(info_bkp, info_to_mst, sizeof(mux_ll_uart_latch_info_t));
    return clock_offset_us;
}


static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_start_latch_req(void)
{
    hal_gpt_status_t gpt_status;

    latch_ts.latch_lock_sleep_duration = mux_get_tick_elapse(latch_ts.latch_start_count);
    latch_ts.tx_lock_start_count = mux_get_gpt_tick_count();

    RB_LOG_I("[uart latch] s2: lock_sleep_duration=%d", 1, latch_ts.latch_lock_sleep_duration);

    gpt_status = hal_gpt_sw_start_timer_ms(latch_context.monitor_timer_handle, 20, mux_ll_uart_latch_monitor_timer_callback, NULL);
    DCHS_LOCK_UART_TX_CLR_REQ();
    mux_ll_uart_send_latch_req(DCHS_ID1_S2M_LATCH_REQ, 0x1);//send latch req cmd to slave, let slave enter latch mode
    if (HAL_GPT_STATUS_OK != gpt_status) {
        RB_LOG_E("[uart latch] s2: start timer fail, gpt_status=%d", 1, gpt_status);
    }
}

ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_check_latch_request(void)
{
    if (DCHS_IS_UART_TX_BLOCK_REQ() && DCHS_IS_UART_TX_BUFFER_EMPTY()) {
        if (latch_context.latch_state == LATCH_STATE_LATCH_REQUEST_PENDING) {
            latch_context.latch_state = LATCH_STATE_WAIT_LATCH_RESPONSE;
            RB_LOG_I("[uart latch] s2: ready to write, start latch req", 0);
            DCHS_LOCK_UART_TX();
            mux_ll_uart_start_latch_req();
        } else {
            RB_LOG_W("[uart latch] s2: Error state:%u, it should be %u", 2, latch_context.latch_state, LATCH_STATE_LATCH_REQUEST_PENDING);
        }
    }
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_do_lock_sleep_done(void)
{
    uint32_t mask;

    if (latch_context.latch_state == LATCH_STATE_WAIT_BT_LOCK_SLEEP_DONE) {
        // in case of DSP trigger ccni to send data to uart fifo.
        hal_nvic_save_and_set_interrupt_mask(&mask);
        if (!DCHS_IS_UART_TX_BUFFER_EMPTY()) {
            DCHS_LOCK_UART_TX_SET_REQ();
            latch_context.latch_state = LATCH_STATE_LATCH_REQUEST_PENDING;
            hal_nvic_restore_interrupt_mask(mask);
            LOG_MSGID_W(common, "[uart latch] s1: lock uart tx req", 0);
        } else {
            DCHS_LOCK_UART_TX();
            latch_context.latch_state = LATCH_STATE_WAIT_LATCH_RESPONSE;
            hal_nvic_restore_interrupt_mask(mask);
            LOG_MSGID_I(common, "[uart latch] s1: lock uart tx", 0);
            mux_ll_uart_start_latch_req();
        }
    } else {
        RB_LOG_W("[uart latch] s1: Error state:%u, it should be %u", 2, latch_context.latch_state, LATCH_STATE_WAIT_BT_LOCK_SLEEP_DONE);
    }
}


static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_do_latch_response(void)
{
    hal_gpt_status_t gpt_status;

    //when master receive this callback, it means slave have already entered latch mode
    latch_ts.tx_lock_duration = mux_get_tick_elapse(latch_ts.tx_lock_start_count);
    if (DCHS_LATCH_MODE_CHECK_ENABLE()) {
        DCHS_bt_pka_enter_uart_tx_latch_mode();
        DCHS_UNLOCK_UART_TX();
        gpt_status = hal_gpt_sw_stop_timer_ms(latch_context.monitor_timer_handle);
        if (HAL_GPT_STATUS_OK != gpt_status) {
            RB_LOG_E("[uart latch] s3: stop timer fail, gpt_status=%d", 1, gpt_status);
        }

        if (latch_ts.tx_lock_duration > 3000) {
            RB_LOG_E("[uart latch] s3: lock uart tx cost too much time: %uus", 1, latch_ts.tx_lock_duration);
        }
        latch_ts.tx_lock_max_duration = RB_MAX(latch_ts.tx_lock_duration, latch_ts.tx_lock_max_duration);
        RB_LOG_I("[uart latch] s3<-m2: unlock uart tx, tx lock duration=%d, max=%d", 2, latch_ts.tx_lock_duration, latch_ts.tx_lock_max_duration);

        mux_ll_uart_send_latch_req(DCHS_ID3_S2M_LATCH_ACTION, 0x3); //send a latch cmd to let slave know that master has already entered latch mode
    } else {
        RB_LOG_E("[uart latch] s3: response cost %uus, latch cancel", 1, latch_ts.tx_lock_duration);
    }
}

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_do_recv_mst_latch_info(mux_ll_uart_latch_info_t *latch_info)
{
    uint32_t clock_offset_us;
    mux_ll_uart_latch_info_t latch_info_to_master = {0};

    latch_ts.latch_duration_count = mux_get_tick_elapse(latch_ts.latch_start_count);

    DCHS_bt_pka_get_uart_latch_info(&latch_info_to_master);
    latch_context.latch_counter++;
    latch_info_to_master.latch_counter = latch_context.latch_counter;
    latch_info_to_master.init_capid = latch_context.init_capid;
    latch_info_to_master.current_capid = get_capid();
    latch_context.native_capid = latch_info_to_master.current_capid;
    latch_context.peer_dev_capid = latch_info->current_capid;

    DCHS_bt_pka_set_audio_clk_offset(FALSE, latch_info, latch_context.latch_counter, dchs_slave_capid_calc_func);
    mux_ll_uart_send_latch_info(&latch_info_to_master, DCHS_ID5_S2M_LATCH_INFO);

    clock_offset_us = mux_ll_uart_latch_check(latch_info, &latch_info_to_master);
    if (latch_context.latch_counter == 5) {
        latch_context.target_clk_offset_us = clock_offset_us;
    }
    LOG_MSGID_I(common, "[uart latch] s4<-m3: latch_cost:%4dus, tx_lock:%4u lock_max:%4u, latch_counter:%u clock_offset:%dus, co_diff:%dus, mst_clk:%7d, mst_phase:%4d, slv_clock:%7d, slv_phase:%4d, dl_src_clock_offset=%d dl_src_phase_offset=%d", 12, \
        latch_ts.latch_duration_count, latch_ts.tx_lock_duration, latch_ts.tx_lock_max_duration, latch_context.latch_counter, (int)(clock_offset_us), (int)(clock_offset_us - latch_ts.clock_offset_us), latch_info->native_clock, \
        latch_info->native_phase, latch_info_to_master.native_clock, latch_info_to_master.native_phase, (int)latch_info_to_master.dl_src_clock_offset, (int)latch_info_to_master.dl_src_phase_offset);

    DCHS_UNLOCK_BT_SLEEP();

    latch_context.bt_clock_synced = true;
}


static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_latch_callback(uint8_t *buff, uint32_t len)
{
    mux_ll_uart_latch_info_t latch_info_from_mst;

    memcpy(&latch_info_from_mst, buff, len);

    switch(latch_info_from_mst.id) {
    case DCHS_ID2_M2S_LATCH_RSP   :
        if (latch_context.latch_state == LATCH_STATE_WAIT_LATCH_RESPONSE) {
            latch_context.latch_state = LATCH_STATE_WAIT_LATCH_INFO;
            mux_ll_uart_latch_do_latch_response();
        } else {
            RB_LOG_W("[uart latch] Error state:%u, it should be %u", 2, latch_context.latch_state, LATCH_STATE_WAIT_LATCH_RESPONSE);
        }
        break;
    case DCHS_ID4_M2S_LATCH_INFO  :
        if (latch_context.latch_state == LATCH_STATE_WAIT_LATCH_INFO) {
            latch_context.latch_state = LATCH_STATE_WAIT_LATCH_TIMER_EXPIRE;
            mux_ll_uart_latch_do_recv_mst_latch_info(&latch_info_from_mst);
        } else {
            RB_LOG_W("[uart latch] Error state:%u, it should be %u", 2, latch_context.latch_state, LATCH_STATE_WAIT_LATCH_INFO);
        }
        break;
    default:
        RB_LOG_E("[uart latch] Error id:0x%x!!", 1, latch_info_from_mst.id);
        break;
    }
}
#endif //AIR_DCHS_MODE_SLAVE_ENABLE

static ATTR_TEXT_IN_FAST_MEM void mux_ll_uart_bt_sleep_lock_done_callback(void* user_data)
{
    (void)user_data;
    RB_LOG_I("[uart latch] lock sleep done", 0);
    mux_ll_uart_latch_do_lock_sleep_done();
}
