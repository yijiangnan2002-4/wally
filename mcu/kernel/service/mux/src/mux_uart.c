/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#include "mux.h"
#include "mux_port.h"
#include "mux_port_device.h"
#include "hal_uart.h"
#include "hal_uart_internal.h"
#include "syslog.h"
#include "syslog_port.h"
#include "serial_port_assignment.h"
#ifdef AIR_1WIRE_ENABLE
#include "smchg_1wire.h"
#endif

#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_spm.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
static sleep_management_lock_request_t uart_lock_sleep[HAL_UART_MAX] = {SLEEP_LOCK_UART0, SLEEP_LOCK_UART1, SLEEP_LOCK_UART2};
static volatile bool mux_uart_lock_sleep_handle[HAL_UART_MAX] = {false, false, false};
static hal_uart_port_t uart_log_running_port = HAL_UART_MAX;
#endif/*HAL_SLEEP_MANAGER_ENABLED*/

/*Static global variables for CONNECT and DISCONNECT service*/
static volatile uint32_t g_uart_record_wptr_legnth[3] = {0};
static volatile bool g_uart_on_off_status[3] = {true, true, true};
extern UART_REGISTER_T *const g_uart_regbase[];
extern bool g_uart_disable_irq[HAL_UART_MAX];

#ifndef MTK_SINGLE_CPU_ENV  /*multi core user*/
static volatile mux_port_config_t *g_uart_port_configs = (volatile mux_port_config_t *)HW_SYSRAM_PRIVATE_MEMORY_MUX_VAR_PORT_START; //80 * N
#endif

/*MUX operate DMA hardware pointer begin*/
/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM uint32_t port_mux_uart_get_hw_rptr(uint8_t port_index, bool is_rx)
{
    return uart_get_hw_rptr(port_index, is_rx);
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM uint32_t port_mux_uart_get_hw_wptr(uint8_t port_index, bool is_rx)
{
    return uart_get_hw_wptr(port_index, is_rx);
}

/**When Receive data, MUX will call this API to move Rx Ring buffer Read point.
    for MUX, firstly ,should copy data out. Then call this API to move Rx Ring buffer Read point. **/
void port_mux_uart_set_rx_hw_rptr(uint8_t port_index, uint32_t move_bytes)
{
    uart_set_sw_move_byte(port_index, 1, move_bytes);
}

/**When Send data, MUX will call this API to move Tx Ring buffer Write point.
    for MUX, firstly ,should copy data in. Then call this API to move Tx Ring buffer Write point. **/
/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM void port_mux_uart_set_tx_hw_wptr(uint8_t port_index, uint32_t move_bytes)
{
    if (g_uart_on_off_status[port_index] == true) {
        uart_set_sw_move_byte(port_index, 0, move_bytes);
        g_uart_record_wptr_legnth[port_index] = 0;
    } else {
        g_uart_record_wptr_legnth[port_index] = move_bytes;
    }
}
/*MUX operate DMA hardware pointer end*/

#ifdef HAL_UART_FEATURE_FLOWCONTROL_CALLBACK
static uint32_t uart_gpt_handle[HAL_UART_MAX] = {0};                                      //set gpt handle
static volatile uint32_t g_uart_get_rptr_when_start_timer[HAL_UART_MAX] = {0};            //record read pointer when user start gpt timer.
static volatile bool g_uart_already_set_gpt_timer[HAL_UART_MAX] = {false, false, false};  //uart set gpt timer when receive flow control
static uint32_t g_uart_blocking_port = HAL_UART_MAX;                                      //indicate which port is blocking by flow control.
#define MAX_FLOW_CONTROL_TIME 60000                                                       //indicate max time that trigger assert more than this time.

void uart_debug_flow_control_callback(void *user_data)
{
    uint32_t  port_index = (uint32_t)user_data;
    uint32_t  hw_tx_rptr;
    uint32_t  log_port;
    UART_REGISTER_T    *uartx = g_uart_regbase[port_index];

    hal_gpt_sw_stop_timer_ms(uart_gpt_handle[port_index]);
    hal_gpt_sw_free_timer(uart_gpt_handle[port_index]);
    g_uart_already_set_gpt_timer[port_index] = false;
    hw_tx_rptr = port_mux_uart_get_hw_rptr(port_index, false);
   // MUX_PORT_MSGID_I("UART_flow_control_callback free timer ! port=%d MCR=%08x\r\n", 2, port_index, uartx->MCR_UNION.MCR);

    /*  assert if two condition true:
        1. DMA tx channel read pointer haven't moved during this period.
        2. UART MCR register show that xoff received or RTS signal received.
    */
    if ((g_uart_get_rptr_when_start_timer[port_index] == hw_tx_rptr) && \
        ((uartx->MCR_UNION.MCR & UART_MCR_XOFF_STATUS_MASK) || uartx->MCR_UNION.MCR & UART_MCR_RTS_MASK)) {
        MUX_PORT_MSGID_I("UART_flow_control_callback receive xoff more than 60s! port=%d MCR=%08x\r\n", 2, port_index, uartx->MCR_UNION.MCR);
        g_uart_blocking_port = port_index;

        log_port = query_syslog_port();
        if (log_port == port_index) {
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
            uart_clear_xoff_status(port_index);
#endif
            // hal_gpt_delay_ms(100);
            // assert(0);
        }
    }
}
#endif /*HAL_UART_FEATURE_FLOWCONTROL_CALLBACK*/

#ifdef MTK_CPU_NUMBER_0

static mux_irq_handler_t g_mux_irq_handler;
#define UART_PORT_INDEX_TO_MUX_PORT(port_index) (port_index + MUX_UART_0)

static void mux_uart_callback(hal_uart_callback_event_t event, void *user_data)
{
    uint32_t  port_index = (uint32_t)user_data;
#ifdef HAL_SLEEP_MANAGER_ENABLED
    uint32_t  hw_tx_rptr, hw_tx_wptr;
    uint32_t  mask_irq;
#endif /*HAL_SLEEP_MANAGER_ENABLED*/

    if (event == HAL_UART_EVENT_READY_TO_WRITE) {
        g_mux_irq_handler(UART_PORT_INDEX_TO_MUX_PORT(port_index), MUX_EVENT_READY_TO_WRITE, user_data);
    } else if (event == HAL_UART_EVENT_READY_TO_READ) {
        g_mux_irq_handler(UART_PORT_INDEX_TO_MUX_PORT(port_index), MUX_EVENT_READY_TO_READ, user_data);
    } else if (event == HAL_UART_EVENT_WAKEUP_SLEEP) {
        g_mux_irq_handler(UART_PORT_INDEX_TO_MUX_PORT(port_index), MUX_EVENT_WAKEUP_FROM_SLEEP, user_data);
    } else if (event == HAL_UART_EVENT_TRANSMISSION_DONE) {
        g_mux_irq_handler(UART_PORT_INDEX_TO_MUX_PORT(port_index), MUX_EVENT_TRANSMISSION_DONE, user_data);
#ifdef HAL_SLEEP_MANAGER_ENABLED

        /* condition of unlock sleep
           1, This hardware uart port has been locked.
           2, DMA hw_write_point equals to hw_read_point that means DMA empty.
           3, mux_uart_lock_sleep_handle is true means this port is locked by mux_uart.
           4, g_uart_record_wptr_legnth must equal to 0 to avoid race condition.
                (i). For connection,g_uart_record_wptr_legnth is always zero,
                (ii).For disconnection,g_uart_record_wptr_legnth record bytes(not zero) that restored in vfifo.
                     but both DMA&UART are empty(IDLE) may lead to enter sleep and reinitialize hardware and lose data.
        */
        if (port_index != uart_log_running_port) {
            port_mux_local_cpu_enter_critical(&mask_irq);   // mask_irq begin
            hw_tx_rptr = port_mux_uart_get_hw_rptr(port_index, false);
            hw_tx_wptr = port_mux_uart_get_hw_wptr(port_index, false);
            if ((hal_sleep_manager_is_sleep_handle_alive(uart_lock_sleep[port_index]) == true) && \
                (g_uart_record_wptr_legnth[port_index] == 0) && (hw_tx_rptr == hw_tx_wptr) && \
                (mux_uart_lock_sleep_handle[port_index] == true)) {
                hal_sleep_manager_unlock_sleep(uart_lock_sleep[port_index]);
                mux_uart_lock_sleep_handle[port_index] = false;
            }
            port_mux_local_cpu_exit_critical(mask_irq);    // mask_irq end
        }
#endif /*HAL_SLEEP_MANAGER_ENABLED*/
    }
#ifdef HAL_UART_FEATURE_FLOWCONTROL_CALLBACK
    else if (event == HAL_UART_EVENT_SW_FLOW_CTRL) {
        UART_REGISTER_T    *uartx = g_uart_regbase[port_index];
        PORT_MUX_UNUSED(uartx); //avoid build warning/error
#ifdef HAL_SLEEP_MANAGER_ENABLED
        if (port_index != uart_log_running_port) {
            MUX_PORT_MSGID_I("UART_SW_FLOW_CTRL! port=%d MCR=%08x\r\n", 2, port_index, uartx->MCR_UNION.MCR);
        }
#endif /*HAL_SLEEP_MANAGER_ENABLED*/
        /*Every time flow control occurs, the timer will restart*/
        if (g_uart_already_set_gpt_timer[port_index] == false) {
            MUX_PORT_MSGID_I("UART_SW_FLOW_CTRL start %d(ms) timer! port=%d MCR=%08x\r\n", 3, MAX_FLOW_CONTROL_TIME, port_index, uartx->MCR_UNION.MCR);

            if (HAL_GPT_STATUS_OK != hal_gpt_sw_get_timer(&uart_gpt_handle[port_index])) {
                return;
            }
            if (HAL_GPT_STATUS_OK != hal_gpt_sw_start_timer_ms(uart_gpt_handle[port_index], MAX_FLOW_CONTROL_TIME, uart_debug_flow_control_callback, (void *)(intptr_t)port_index)) {
                hal_gpt_sw_free_timer(uart_gpt_handle[port_index]);
                return;
            }
            g_uart_get_rptr_when_start_timer[port_index] = port_mux_uart_get_hw_rptr(port_index, false);
            g_uart_already_set_gpt_timer[port_index] = true;        //set true to control start timer.

        } else if (g_uart_already_set_gpt_timer[port_index] == true) {
            // MUX_PORT_MSGID_I("UART_SW_FLOW_CTRL restart %d(ms) timer! port=%d MCR=%08x\r\n",3,MAX_FLOW_CONTROL_TIME,port_index,uartx->MCR_UNION.MCR);

            hal_gpt_sw_stop_timer_ms(uart_gpt_handle[port_index]);
            hal_gpt_sw_free_timer(uart_gpt_handle[port_index]);

            if (HAL_GPT_STATUS_OK != hal_gpt_sw_get_timer(&uart_gpt_handle[port_index])) {
                return;
            }
            if (HAL_GPT_STATUS_OK != hal_gpt_sw_start_timer_ms(uart_gpt_handle[port_index], MAX_FLOW_CONTROL_TIME, uart_debug_flow_control_callback, (void *)(intptr_t)port_index)) {
                hal_gpt_sw_free_timer(uart_gpt_handle[port_index]);
                return;
            }
            g_uart_get_rptr_when_start_timer[port_index] = port_mux_uart_get_hw_rptr(port_index, false);
        }
    } else if (event == HAL_UART_EVENT_HW_FLOW_CTRL) {
        UART_REGISTER_T    *uartx = g_uart_regbase[port_index];
        PORT_MUX_UNUSED(uartx); //avoid build warning/error
        /*Every time flow control occurs, the timer will restart*/
        if (g_uart_already_set_gpt_timer[port_index] == false) {
            MUX_PORT_MSGID_I("UART_HW_FLOW_CTRL start %d(ms) timer! port=%d MCR=%08x\r\n", 3, MAX_FLOW_CONTROL_TIME, port_index, uartx->MCR_UNION.MCR);
            if (HAL_GPT_STATUS_OK != hal_gpt_sw_get_timer(&uart_gpt_handle[port_index])) {
                return;
            }
            if (HAL_GPT_STATUS_OK != hal_gpt_sw_start_timer_ms(uart_gpt_handle[port_index], MAX_FLOW_CONTROL_TIME, uart_debug_flow_control_callback, (void *)(intptr_t)port_index)) {
                hal_gpt_sw_free_timer(uart_gpt_handle[port_index]);
                return;
            }
            g_uart_get_rptr_when_start_timer[port_index] = port_mux_uart_get_hw_rptr(port_index, false);
            g_uart_already_set_gpt_timer[port_index] = true;        //set true to control start timer.

        } else if (g_uart_already_set_gpt_timer[port_index] == true) {
          //  MUX_PORT_MSGID_I("UART_HW_FLOW_CTRL restart %d(ms) timer! port=%d MCR=%08x\r\n", 3, MAX_FLOW_CONTROL_TIME, port_index, uartx->MCR_UNION.MCR);
            hal_gpt_sw_stop_timer_ms(uart_gpt_handle[port_index]);
            hal_gpt_sw_free_timer(uart_gpt_handle[port_index]);

            if (HAL_GPT_STATUS_OK != hal_gpt_sw_get_timer(&uart_gpt_handle[port_index])) {
                return;
            }
            if (HAL_GPT_STATUS_OK != hal_gpt_sw_start_timer_ms(uart_gpt_handle[port_index], MAX_FLOW_CONTROL_TIME, uart_debug_flow_control_callback, (void *)(intptr_t)port_index)) {
                hal_gpt_sw_free_timer(uart_gpt_handle[port_index]);
                return;
            }
            g_uart_get_rptr_when_start_timer[port_index] = port_mux_uart_get_hw_rptr(port_index, false);
        }
    }
#endif /*HAL_UART_FEATURE_FLOWCONTROL_CALLBACK*/
    else if(event == HAL_UART_EVENT_TRANSACTION_ERROR ){
        MUX_PORT_MSGID_E("uart[%d] receive error data", 1, port_index);
    }
    else if(event == HAL_UART_EVENT_RECEIVE_BREAK_SIGNAL){
        MUX_PORT_MSGID_I("uart[%d] receive break signal", 1, port_index);
#ifdef AIR_1WIRE_ENABLE
        g_mux_irq_handler(UART_PORT_INDEX_TO_MUX_PORT(port_index), MUX_EVENT_BREAK_SIGNAL, user_data);
#endif
    }
}

mux_status_t port_mux_uart_normal_init(uint8_t port_index, mux_port_config_t *p_setting, mux_irq_handler_t irq_handler)
{
    hal_uart_config_t uart_config;
    hal_uart_dma_config_t dma_config;

    g_mux_irq_handler = irq_handler;

    uart_config.baudrate     = p_setting->p_user_setting->dev_setting.uart.uart_config.baudrate;    //CONFIG_SYSLOG_BAUDRATE;
    uart_config.parity       = p_setting->p_user_setting->dev_setting.uart.uart_config.parity;      //HAL_UART_PARITY_NONE;
    uart_config.stop_bit     = p_setting->p_user_setting->dev_setting.uart.uart_config.stop_bit;    //HAL_UART_STOP_BIT_1;
    uart_config.word_length  = p_setting->p_user_setting->dev_setting.uart.uart_config.word_length; //HAL_UART_WORD_LENGTH_8;

    dma_config.send_vfifo_buffer      = (uint8_t *)(uint32_t *)p_setting->tx_buf_addr;
    dma_config.send_vfifo_buffer_size = p_setting->tx_buf_size;
    dma_config.receive_vfifo_buffer   = (uint8_t *)(uint32_t *)p_setting->rx_buf_addr;
    dma_config.receive_vfifo_buffer_size = p_setting->rx_buf_size;
    dma_config.send_vfifo_threshold_size = dma_config.send_vfifo_buffer_size / 8;
    dma_config.receive_vfifo_threshold_size   = dma_config.receive_vfifo_buffer_size / 2;
    dma_config.receive_vfifo_alert_size       = 12;

    hal_uart_deinit(port_index);
    /*assign logging port and record in uart driver*/
    mux_port_t syslog_port_is_runing = query_syslog_port();
    uart_backup_log_para_to_share_buf(syslog_port_is_runing);
    if (HAL_UART_STATUS_OK != hal_uart_init(port_index, &uart_config)) {
        return MUX_STATUS_ERROR_INIT_FAIL;
    }

    if (HAL_UART_STATUS_OK != hal_uart_set_dma(port_index, &dma_config)) {
        return MUX_STATUS_ERROR_INIT_FAIL;
    }

    if (p_setting->p_user_setting->dev_setting.uart.flowcontrol_type == MUX_UART_NONE_FLOWCONTROL) {
        hal_uart_disable_flowcontrol(port_index);
    } else if (p_setting->p_user_setting->dev_setting.uart.flowcontrol_type == MUX_UART_SW_FLOWCONTROL) {
        hal_uart_set_software_flowcontrol(port_index, 0x11, 0x13, 0x77);
    } else if (p_setting->p_user_setting->dev_setting.uart.flowcontrol_type == MUX_UART_HW_FLOWCONTROL) {
        hal_uart_set_hardware_flowcontrol(port_index);
    } else {
        assert(0);
    }

    if (HAL_UART_STATUS_OK != hal_uart_register_callback(port_index, mux_uart_callback, (void *)(intptr_t)port_index)) {
        return MUX_STATUS_ERROR_INIT_FAIL;
    }
    /*In signal core ,this function restore logging port parameter in uart driver.
      In multi core,  this function restore hardware(DMA&UART) parameter of logging port to share buffer*/
    uart_backup_log_para_to_share_buf(syslog_port_is_runing);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    uart_log_running_port = syslog_port_is_runing;
#endif

    return MUX_STATUS_OK;
}

mux_status_t port_mux_uart_deinit(uint8_t port_index)
{
    if (HAL_UART_STATUS_OK != hal_uart_deinit(port_index)) {
        return MUX_STATUS_ERROR_DEINIT_FAIL;
    } else {
        return MUX_STATUS_OK;
    }
}

void port_mux_uart_exception_init(uint8_t port_index)
{
    hal_uart_config_t uart_config;

    hal_uart_deinit(port_index);
    uart_config.baudrate = syslog_port_query_uart_baudrate();
    uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    hal_uart_init(port_index, &uart_config);
#ifndef MTK_DEBUG_PLAIN_LOG_ENABLE
    hal_uart_set_software_flowcontrol(port_index, 0x11, 0x13, 0x77);
#endif
}

void port_mux_uart_exception_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    uart_exception_send_polling(port_index, buffer, size);
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM bool port_mux_uart_buf_is_full(uint8_t port_index, bool is_rx)
{
    return uart_get_buf_full_status(port_index, is_rx);
}

#endif /* MTK_CPU_NUMBER_0 */

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM mux_status_t port_mux_uart_phase1_send(uint8_t port_index)
{
#ifdef HAL_SLEEP_MANAGER_ENABLED
    /*if the port is not for logging, mux_uart will lock sleep until data transfer done.
       as far as logging port, SPM just waits all system log sent out(TX empty) then power off infra_domain.

       condition of tx lock sleep:
       1, Nobody lock sleep for this hardware port.
       2, because hal uart interface may lock sleep using uart's handle,so it is necessary to judge true or false of mux_uart_lock_sleep_handle.
    */
    if (port_index != uart_log_running_port) {
        uint32_t mask_irq_phase1;
        port_mux_local_cpu_enter_critical(&mask_irq_phase1);   // mask_irq begin  these run 15 us.
        if ((hal_sleep_manager_is_sleep_handle_alive(uart_lock_sleep[port_index]) == false)
            && (mux_uart_lock_sleep_handle[port_index] == false)
            && (g_uart_disable_irq[port_index] == false)) {
            mux_uart_lock_sleep_handle[port_index] = true;
            port_mux_local_cpu_exit_critical(mask_irq_phase1);     // mask_irq end  these run 15 us.
            hal_sleep_manager_lock_sleep(uart_lock_sleep[port_index]);
        } else {
            port_mux_local_cpu_exit_critical(mask_irq_phase1);     // mask_irq end  these run 15 us.
        }

    }
#else
    PORT_MUX_UNUSED(port_index);
#endif
    return MUX_STATUS_OK;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM mux_status_t port_mux_uart_phase2_send(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
    return MUX_STATUS_OK;
}

void port_mux_uart_clear_fifo(uint8_t port_index, mux_ctrl_cmd_t cmd)
{
    hal_uart_status_t status;

    if(cmd == MUX_CMD_CLEAN_TX_VIRUTUAL) {
        status = uart_clear_vfifo_and_fifo(port_index, 0);   //tx
    } else if(cmd == MUX_CMD_CLEAN_RX_VIRUTUAL) {
        status = uart_clear_vfifo_and_fifo(port_index, 1);   //rx
    } else {
        status = uart_clear_vfifo_and_fifo(port_index, 0);
        status = uart_clear_vfifo_and_fifo(port_index, 1);
    }

    if (HAL_UART_STATUS_OK != status) {
        MUX_PORT_MSGID_I("port_mux_uart_clear_fifo_failed!\r\n", 0);
    }
}

#ifdef AIR_1WIRE_ENABLE
extern smchg_1wire_mode_t smchg_1wire_get_mode_status(void);
#endif

mux_status_t port_mux_uart_control(uint8_t port_index, mux_ctrl_cmd_t command, mux_ctrl_para_t *para)
{
    uint32_t mask;
    bool isTxIdle, isRxIdle;
    uint32_t rxLength;
#ifdef HAL_SLEEP_MANAGER_ENABLED
    uint32_t mask_irq;
#endif
#ifdef AIR_1WIRE_ENABLE
    hal_uart_status_t uart_status;
#endif
    UART_REGISTER_T    *uartx = g_uart_regbase[port_index];
    PORT_MUX_UNUSED(uartx); //avoid build warning/error

    switch (command) {
        case MUX_CMD_CONNECT: {

#ifndef MTK_SINGLE_CPU_ENV  /*multi core user*/
            volatile mux_port_config_t *log_x = &g_uart_port_configs[port_index];
            /*this port_is_connect will restore in share buffer,if dsp is existed,dsp could know currently status*/
            log_x->port_is_connect = MUX_PORT_CONNECT;
#endif

            MUX_PORT_MSGID_I("port_mux_uart_control()--> CONNECT ! record_size = %d\r\n", 1, g_uart_record_wptr_legnth[port_index]);
            hal_nvic_save_and_set_interrupt_mask(&mask);
            g_uart_on_off_status[port_index] = true;
            if (g_uart_record_wptr_legnth[port_index] != 0) {
                port_mux_uart_set_tx_hw_wptr(port_index, g_uart_record_wptr_legnth[port_index]);
            }
            g_uart_record_wptr_legnth[port_index] = 0;
            hal_nvic_restore_interrupt_mask(mask);
            return MUX_STATUS_OK;
        }
        break;

        case MUX_CMD_DISCONNECT: {

            g_uart_on_off_status[port_index] = false;
            MUX_PORT_MSGID_I("port_mux_uart_control()--> DISCONNECT !\r\n", 0);

#ifndef MTK_SINGLE_CPU_ENV  /*multi core user*/
            volatile mux_port_config_t *log_x = &g_uart_port_configs[port_index];
            /*this port_is_connect will restore in share buffer,if dsp is existed,dsp could know currently status*/
            log_x->port_is_connect = MUX_PORT_DISCONNECT;
#endif

            return MUX_STATUS_OK;
        }
        break;

        case MUX_CMD_CLEAN:
        case MUX_CMD_CLEAN_TX_VIRUTUAL:
        case MUX_CMD_CLEAN_RX_VIRUTUAL:
            MUX_PORT_MSGID_I("port_mux_uart mcr_status = 0x%08x,\r\n", 1, uartx->MCR_UNION.MCR);
            port_mux_uart_clear_fifo(port_index, command);
            MUX_PORT_MSGID_I("port_mux_uart_control()--> CLEAN !, command = %d\r\n", 1, command);
            return MUX_STATUS_OK;
        break;

#ifdef AIR_1WIRE_ENABLE
        case MUX_CMD_UART_TX_ENABLE: {
          //  MUX_PORT_MSGID_I("port_mux_uart_control()--> MUX_CMD_UART_TX_ENABLE !\r\n", 0);
            if ((smchg_cfg.uart_sel == SMCHG_UART1_2GPIO) || (smchg_cfg.uart_sel == SMCHG_UART0_2GPIO)) {
                /* TX UART mode */
                hal_pinmux_set_function(smchg_1wire_gpio_cfg.tx_pin, smchg_1wire_gpio_cfg.tx_uart);
                hal_gpio_set_direction(smchg_1wire_gpio_cfg.tx_pin, HAL_GPIO_DIRECTION_OUTPUT);
                /* RX GIOP mode */
                hal_pinmux_set_function(smchg_1wire_gpio_cfg.rx_pin, smchg_1wire_gpio_cfg.rx_gpio);
                hal_gpio_set_direction(smchg_1wire_gpio_cfg.rx_pin, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_disable_pull(smchg_1wire_gpio_cfg.rx_pin);
                //hal_gpio_pull_down(smchg_1wire_gpio_cfg.rx_pin);
            } else if ((smchg_cfg.uart_sel == SMCHG_UART1_1GPIO) || (smchg_cfg.uart_sel == SMCHG_UART0_1GPIO)) {
                /* RX UART mode */
                hal_pinmux_set_function(smchg_1wire_gpio_cfg.trx_pin, smchg_1wire_gpio_cfg.tx_uart);
                hal_gpio_set_direction(smchg_1wire_gpio_cfg.trx_pin, HAL_GPIO_DIRECTION_OUTPUT);
            }
            return MUX_STATUS_OK;
        }
        break;

        case MUX_CMD_UART_RX_ENABLE: {
         //   MUX_PORT_MSGID_I("port_mux_uart_control()--> MUX_CMD_UART_RX_ENABLE !\r\n", 0);
            if ((smchg_cfg.uart_sel == SMCHG_UART1_2GPIO) || (smchg_cfg.uart_sel == SMCHG_UART0_2GPIO)) {
                /* TX GPIO mode */
                hal_gpio_set_direction(smchg_1wire_gpio_cfg.tx_pin, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_disable_pull(smchg_1wire_gpio_cfg.tx_pin);
                hal_pinmux_set_function(smchg_1wire_gpio_cfg.tx_pin, smchg_1wire_gpio_cfg.tx_gpio);
                /* RX UART mode */
                hal_pinmux_set_function(smchg_1wire_gpio_cfg.rx_pin, smchg_1wire_gpio_cfg.rx_uart);
            } else if ((smchg_cfg.uart_sel == SMCHG_UART1_1GPIO) || (smchg_cfg.uart_sel == SMCHG_UART0_1GPIO)) {
                /* RX UART mode */
                hal_pinmux_set_function(smchg_1wire_gpio_cfg.trx_pin, smchg_1wire_gpio_cfg.rx_uart);
            }
            return MUX_STATUS_OK;
        }
        break;
#if 0
        case MUX_CMD_UART_TX_RX_ENABLE: {
            MUX_PORT_MSGID_I("port_mux_uart_control()--> MUX_CMD_UART_TX_RX_ENABLE !\r\n", 0);
            /* TX UART mode */
            hal_pinmux_set_function(smchg_1wire_gpio_cfg.tx_pin, smchg_1wire_gpio_cfg.tx_uart);
            hal_gpio_set_direction(smchg_1wire_gpio_cfg.tx_pin, HAL_GPIO_DIRECTION_OUTPUT);
            /* RX UART mode */
            hal_pinmux_set_function(smchg_1wire_gpio_cfg.rx_pin, smchg_1wire_gpio_cfg.rx_uart);
            return MUX_STATUS_OK;
        }
        break;
#endif
        case MUX_CMD_UART_TX_RX_DISABLE: {
          //  MUX_PORT_MSGID_I("port_mux_uart_control()--> MUX_CMD_UART_TX_RX_DISABLE !\r\n", 0);
            if ((smchg_cfg.uart_sel == SMCHG_UART1_2GPIO) || (smchg_cfg.uart_sel == SMCHG_UART0_2GPIO)) {
                /* TX GPIO mode */
                hal_pinmux_set_function(smchg_1wire_gpio_cfg.tx_pin, smchg_1wire_gpio_cfg.tx_gpio);
                hal_gpio_set_direction(smchg_1wire_gpio_cfg.tx_pin, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_disable_pull(smchg_1wire_gpio_cfg.tx_pin);
                /* RX GPIO mode */
                hal_pinmux_set_function(smchg_1wire_gpio_cfg.rx_pin, smchg_1wire_gpio_cfg.rx_gpio);
                if (smchg_1wire_get_mode_status() == SMCHG_1WIRE_OUT) {
#if (defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_BTA_PMIC_LP)) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
                    hal_gpio_set_direction(smchg_1wire_gpio_cfg.rx_pin, HAL_GPIO_DIRECTION_OUTPUT);
                    hal_gpio_set_output(smchg_1wire_gpio_cfg.rx_pin, HAL_GPIO_DATA_LOW);
                    hal_gpt_delay_ms(2);
#endif
                    hal_gpio_set_direction(smchg_1wire_gpio_cfg.rx_pin, HAL_GPIO_DIRECTION_INPUT);
                    hal_gpio_pull_down(smchg_1wire_gpio_cfg.rx_pin); //out of case
                } else if (smchg_1wire_get_mode_status() == SMCHG_1WIRE_CHG) {
                    hal_gpio_set_direction(smchg_1wire_gpio_cfg.rx_pin, HAL_GPIO_DIRECTION_INPUT);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_BTA_PMIC_HP)
                    hal_gpio_pull_down(smchg_1wire_gpio_cfg.rx_pin); //chg mode
#else
                    hal_gpio_pull_up(smchg_1wire_gpio_cfg.rx_pin); //chg mode
#endif
                } else {
                    assert(0);
                }
            } else if ((smchg_cfg.uart_sel == SMCHG_UART1_1GPIO) || (smchg_cfg.uart_sel == SMCHG_UART0_1GPIO)) {
                /* RX GPIO mode */
                hal_pinmux_set_function(smchg_1wire_gpio_cfg.trx_pin, smchg_1wire_gpio_cfg.trx_gpio);
                if (smchg_1wire_get_mode_status() == SMCHG_1WIRE_OUT) {
#if (defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_BTA_PMIC_LP)) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
                    hal_gpio_set_direction(smchg_1wire_gpio_cfg.trx_pin, HAL_GPIO_DIRECTION_OUTPUT);
                    hal_gpio_set_output(smchg_1wire_gpio_cfg.trx_pin, HAL_GPIO_DATA_LOW);
                    hal_gpt_delay_ms(2);
#endif
                    hal_gpio_set_direction(smchg_1wire_gpio_cfg.trx_pin, HAL_GPIO_DIRECTION_INPUT);
                    hal_gpio_pull_down(smchg_1wire_gpio_cfg.trx_pin);
                } else if (smchg_1wire_get_mode_status() == SMCHG_1WIRE_CHG) {
                    hal_gpio_set_direction(smchg_1wire_gpio_cfg.trx_pin, HAL_GPIO_DIRECTION_INPUT);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_BTA_PMIC_HP)
                    hal_gpio_pull_down(smchg_1wire_gpio_cfg.trx_pin); //chg mode
#else
                    hal_gpio_pull_up(smchg_1wire_gpio_cfg.trx_pin); //chg mode
#endif
                } else {
                    assert(0);
                }
            }
            return MUX_STATUS_OK;
        }
        break;

        case MUX_CMD_CHANGE_UART_PARAM: {
            if (para == NULL) {
                MUX_PORT_MSGID_E("MUX_CMD_CHANGE_UART_PARAM NULL point error \r\n", 0);
                return MUX_STATUS_ERROR;
            }
            uart_status = hal_uart_set_baudrate(port_index, para->mux_set_config_uart_param.baudrate);
            MUX_PORT_MSGID_I("MUX_CMD_CHANGE_UART_PARAM port=%d br=%d status=%d \r\n", 3, port_index, para->mux_set_config_uart_param.baudrate, uart_status);
            if (uart_status == HAL_UART_STATUS_OK) {
                return MUX_STATUS_OK;
            }
        }
        break;

        case MUX_CMD_CHANGE_UART_TX_INT: {
            if (para == NULL) {
                MUX_PORT_MSGID_E("MUX_CMD_CHANGE_UART_INT NULL point error \r\n", 0);
                return MUX_STATUS_ERROR;
            }
            MUX_PORT_MSGID_I("MUX_CMD_CHANGE_UART_INT port=%d en=%d\r\n", 2, port_index, para->mux_set_config_uart_param.int_enable);
            if (para->mux_set_config_uart_param.int_enable == true) {
                uart_unmask_send_interrupt(uartx);
            } else {
                uart_mask_send_interrupt(uartx);
            }

            return MUX_STATUS_OK;
        }
        break;
#endif

        case MUX_CMD_GET_TX_PORT_IDLE: {
            //MUX_PORT_MSGID_I("port_mux_uart_control()--> MUX_CMD_GET_TX_PORT_IDLE !\r\n", 0);
            isTxIdle = uart_query_tx_empty(port_index);
            if (isTxIdle == true) {
#ifdef HAL_SLEEP_MANAGER_ENABLED
                port_mux_local_cpu_enter_critical(&mask_irq);  // mask_irq start
                if ((hal_sleep_manager_is_sleep_handle_alive(uart_lock_sleep[port_index]) == true) && \
                (mux_uart_lock_sleep_handle[port_index] == true)) {
                    hal_sleep_manager_unlock_sleep(uart_lock_sleep[port_index]);
                    mux_uart_lock_sleep_handle[port_index] = false;
                }
                port_mux_local_cpu_exit_critical(mask_irq);    // mask_irq end
#endif /*HAL_SLEEP_MANAGER_ENABLED*/
                return MUX_STATUS_OK;
            } else {
                MUX_PORT_MSGID_I("MUX_CMD_GET_TX_PORT_IDLE port=%d isTxIdle=%d \r\n", 2, port_index, isTxIdle);
                return MUX_STATUS_ERROR_BUSY;
            }
        }
        break;

        case MUX_CMD_GET_RX_PORT_IDLE: {
            //MUX_PORT_MSGID_I("port_mux_uart_control()--> MUX_CMD_GET_RX_PORT_IDLE !\r\n", 0);
            isRxIdle = uart_query_rx_empty(port_index);
            rxLength = hal_uart_get_available_receive_bytes(port_index);
            if (rxLength || !isRxIdle) {
                MUX_PORT_MSGID_I("MUX_CMD_GET_RX_PORT_IDLE port=%d isRxIdle=%d  rxLength=%d \r\n", 3, port_index, isRxIdle, rxLength);
                return MUX_STATUS_ERROR_BUSY;
            } else {
                return MUX_STATUS_OK;
            }
        }
        break;

        case MUX_CMD_GET_TRX_PORT_IDLE: {
            //MUX_PORT_MSGID_I("port_mux_uart_control()--> MUX_CMD_GET_TRX_PORT_IDLE !\r\n", 0);
            isRxIdle = uart_query_rx_empty(port_index);
            isTxIdle = uart_query_tx_empty(port_index);
            rxLength = hal_uart_get_available_receive_bytes(port_index);
            if (rxLength || !isTxIdle || !isRxIdle) {
                MUX_PORT_MSGID_I("MUX_CMD_GET_TRX_PORT_IDLE port=%d isTxIdle=%d isRxIdle=%d  rxLength=%d \r\n", 4, port_index, isTxIdle, isRxIdle, rxLength);
                return MUX_STATUS_ERROR_BUSY;
            } else {
                return MUX_STATUS_OK;
            }
        }
        break;

        default: {
           // MUX_PORT_MSGID_I("MUX UART error control cmd:0x%x\r\n", 1, command);
        }
        break;
    }

    return MUX_STATUS_ERROR;
}

port_mux_device_ops_t g_port_mux_uart_ops = {
#ifdef MTK_CPU_NUMBER_0
    port_mux_uart_normal_init,
    port_mux_uart_deinit,
    port_mux_uart_exception_init,
    port_mux_uart_exception_send,
    port_mux_uart_buf_is_full,
#endif
    port_mux_uart_get_hw_rptr,
    port_mux_uart_set_rx_hw_rptr,
    port_mux_uart_get_hw_wptr,
    port_mux_uart_set_tx_hw_wptr,
    port_mux_uart_phase1_send,
    port_mux_uart_phase2_send,
    port_mux_uart_control,
    NULL,
    NULL,
};



