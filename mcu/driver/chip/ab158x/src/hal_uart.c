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

#include "hal_uart.h"
#include "hal_uart_internal.h"
#ifdef HAL_UART_MODULE_ENABLED

#include "memory_attribute.h"
#include "hal_resource_assignment.h"
#include "core_cm33.h"
#include "hal_pdma_internal.h"
#include "hal_nvic.h"
#include "hal_nvic_internal.h"
#include "hal_clock.h"
#include "hal_log.h"

#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_gpt.h"
#include "hal_spm.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#endif/*HAL_SLEEP_MANAGER_ENABLED*/

#ifdef __cplusplus
extern "C" {
#endif

static bool g_uart_not_wait_empty_status = false;
static bool g_uart_global_data_initialized = false;
static hal_uart_config_t g_uart_config[HAL_UART_MAX];
static hal_uart_baudrate_t g_baudrate[HAL_UART_MAX];
static hal_uart_port_t g_uart_port_for_logging = HAL_UART_0;
static uart_flowcontrol_t g_uart_flowcontrol_status[HAL_UART_MAX] = {UART_FLOWCONTROL_NONE, UART_FLOWCONTROL_NONE, UART_FLOWCONTROL_NONE};
bool g_uart_disable_irq[HAL_UART_MAX] = {false, false, false};

static volatile uart_hwstatus_t g_uart_hwstatus[HAL_UART_MAX];
static uart_callback_t g_uart_callback[HAL_UART_MAX];
static uart_dma_callback_data_t g_uart_dma_callback_data[HAL_UART_MAX * 2];
static hal_uart_dma_config_t g_uart_dma_config[HAL_UART_MAX];
static const uint32_t g_uart_baudrate_map[] = {110, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600, 2000000, 3000000, 3200000, 6000000, 8666000};
VDMA_REGISTER_T *const g_vdma_regbase[HAL_UART_MAX][2] = {{VDMA_RG_UART0_TX_BASE, VDMA_RG_UART0_RX_BASE},
    {VDMA_RG_UART1_TX_BASE, VDMA_RG_UART1_RX_BASE},
    {VDMA_RG_UART2_TX_BASE, VDMA_RG_UART2_RX_BASE}
};
const vdma_channel_t  uart_port_to_dma_map[2][3] = {
    {VDMA_UART0TX, VDMA_UART1TX, VDMA_UART2TX},
    {VDMA_UART0RX, VDMA_UART1RX, VDMA_UART2RX},
};

#define  uart_port_to_dma_channel(uart_port,is_rx)   (uart_port_to_dma_map[is_rx][uart_port])

#ifdef HAL_SLEEP_MANAGER_ENABLED
static bool g_uart_frist_send_complete_interrupt[HAL_UART_MAX];
static bool g_uart_send_lock_status[HAL_UART_MAX];
static uart_sw_flowcontrol_config_t g_uart_sw_flowcontrol_config[HAL_UART_MAX];
static const hal_eint_number_t g_uart_eint_num[HAL_UART_MAX] = {HAL_EINT_UART_0_RX, HAL_EINT_UART_1_RX, HAL_EINT_UART_2_RX};
static sleep_management_lock_request_t uart_sleep_handle[HAL_UART_MAX] = {SLEEP_LOCK_UART0, SLEEP_LOCK_UART1, SLEEP_LOCK_UART2};
static volatile uart_backup_para_for_log_t uart_backup_para[HAL_UART_MAX];
#endif

extern UART_REGISTER_T *const g_uart_regbase[];
extern hal_clock_cg_id g_uart_port_to_pdn[];
extern hal_nvic_irq_t g_uart_port_to_irq_num[];

static uint32_t state_machine_ary[HAL_UART_MAX] = {0, 0, 0};
static uint32_t rx_timeout_irq_count[HAL_UART_MAX] = {0, 0, 0};
extern bool volatile g_uart_receive_break_single_status[];
// extern bool volatile sleep_rx_timeout_flag[];

static bool uart_port_is_valid(hal_uart_port_t uart_port)
{
    return (uart_port < HAL_UART_MAX);
}

static bool uart_baudrate_is_valid(hal_uart_baudrate_t baudrate)
{
    return (baudrate < HAL_UART_BAUDRATE_MAX);
}

static bool uart_config_is_valid(const hal_uart_config_t *config)
{
    return ((config->baudrate < HAL_UART_BAUDRATE_MAX) &&
            (config->word_length <= HAL_UART_WORD_LENGTH_8) &&
            (config->stop_bit <= HAL_UART_STOP_BIT_2) &&
            (config->parity <= HAL_UART_PARITY_EVEN));
}

/* triggered by vfifo dma rx thershold interrupt or UART receive timeout interrupt.
 * 1. When vfifo dma rx thershold interrupt happen,
 * this function is called with is_timeout=false.
 * then call suer's callback to notice that data can be fetched from receive buffer.
 * 2. When UART receive timeout interrupt happen,
 * this function is called with is_timeout=true.
 * then call suer's callback to notice that data can be fetched from receive buffer.
 */
void uart_receive_handler(hal_uart_port_t uart_port, bool is_timeout)
{
    vdma_channel_t channel;
    uint32_t rcv_bytes;
    hal_uart_callback_t callback;
    void *arg;
    UART_REGISTER_T *uartx;
    vdma_status_t status;
    uint8_t vdma_irq_is_enabled;

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_DMA_INITIALIZED) {
        UART_ASSERT();
        return;
    }

    uartx = g_uart_regbase[uart_port];
    channel = uart_port_to_dma_channel(uart_port, 1);

    if (is_timeout == true) {
        uart_clear_timeout_interrupt(uartx);
    }

    status = vdma_get_available_receive_bytes(channel, &rcv_bytes);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return;
    }

    /*  fix uart RX ground issue code */
    if(g_uart_receive_break_single_status[uart_port] == true) {
        if ((rcv_bytes == 0) && (state_machine_ary[uart_port] == 0))  {
            rx_timeout_irq_count[uart_port]++;
            if (rx_timeout_irq_count[uart_port] >= 30) {   // avoid receive  dummy irq,  rx timeout irq must continuous
                /*Enable dma irq & set dma threshold to '0'*/
                vdma_set_threshold(channel, 0);
                vdma_enable_interrupt(channel);
                /*disable uart timeout irq*/
                uart_mask_receive_interrupt(uartx);
                state_machine_ary[uart_port] = 1;
                log_hal_msgid_error("uart[%d] rx ground  \r\n", 1, uart_port);
                return;
            }
            return;
        } else if (state_machine_ary[uart_port] == 1) {
            /*restore dma threshold setting*/
            vdma_set_threshold(channel, g_uart_dma_config[uart_port].receive_vfifo_threshold_size);
            /*enable uart timeout irq*/
            uart_unmask_receive_interrupt(uartx);
            log_hal_msgid_info(" uart[%d] umask rx timeout\r\n", 1, uart_port);
            state_machine_ary[uart_port] = 0;
            rx_timeout_irq_count[uart_port] = 0;   /*  clear not continuous rx timeout irq  count*/
            g_uart_receive_break_single_status[uart_port] = false; /* clear break single irq  status*/
        }
        rx_timeout_irq_count[uart_port] = 0;  /*  clear not continuous rx timeout irq  count*/
    }

    callback = g_uart_callback[uart_port].func;
    arg = g_uart_callback[uart_port].arg;
    if (callback == NULL) {
        UART_ASSERT();
        return;
    }
    /* for callback re-entry issue */
    /* Avoid user didn't get data in the callback, then user will receive one dummy irq*/
    vdma_get_interrupt_status(channel, &vdma_irq_is_enabled);
    if (vdma_irq_is_enabled == true) {
        vdma_disable_interrupt(channel);
    }
    callback(HAL_UART_EVENT_READY_TO_READ, arg);
    if (vdma_irq_is_enabled == true) {
        vdma_enable_interrupt(channel);
    }

    status = vdma_get_available_receive_bytes(channel, &rcv_bytes);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return;
    }

    /*  For rx channel.
            If the amount of data in Vfifo is greater than the threshold,
            driver will disable rx DMA channel IRQ.
    */
#ifdef AIR_DCHS_MODE_ENABLE
    if (uart_port != HAL_UART_1) {
#endif
    if (rcv_bytes >= g_uart_dma_config[uart_port].receive_vfifo_threshold_size) {
        status = vdma_disable_interrupt(channel);
        if (status != VDMA_OK) {
            UART_ASSERT();
            return;
        }
    }
#ifdef AIR_DCHS_MODE_ENABLE
    }
#endif
}

/* triggered by vfifo dma tx thershold interrupt or uart transmit complete interrupt.
 * 1. When vfifo dma tx thershold interrupt happen,
 * this function is called with is_send_complete_trigger=false.
 * then call suer's callback to notice that data can be put in send buffer again.
 * 2. When UART transmit complete interrupt happen,
 * this function is called with is_send_complete_trigger=true.
 * Now all user data has been sent out, so we call hal_sleep_manager_unlock_sleep()
 * to unlock sleep.
 */
void uart_send_handler(hal_uart_port_t uart_port, bool is_send_complete_trigger)
{
    vdma_channel_t channel;
    vdma_status_t status;
    uint32_t compare_space, empty_space;
    hal_uart_callback_t callback;
    void *arg;
#ifdef HAL_SLEEP_MANAGER_ENABLED
    uint32_t irq_status;
#endif

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_DMA_INITIALIZED) {
        UART_ASSERT();
        return;
    }

    channel = uart_port_to_dma_channel(uart_port, 0);
    if (is_send_complete_trigger == true) {
#ifdef HAL_SLEEP_MANAGER_ENABLED
        if (g_uart_frist_send_complete_interrupt[uart_port] == false) {
            hal_nvic_save_and_set_interrupt_mask(&irq_status);
            status = vdma_get_available_send_space(channel, &empty_space);
            if (status != VDMA_OK) {
                hal_nvic_restore_interrupt_mask(irq_status);
                UART_ASSERT();
                return;
            }

            /*when driver discern [is_send_complete_trigger] is true,it means HW fifo empty IRQ triggered
              if driver discerb DMA vfifo empty at the moment, uart tx unlock sleep.
             */
            if (g_uart_dma_config[uart_port].send_vfifo_buffer_size - empty_space == 0) {
                if (g_uart_send_lock_status[uart_port] == true) {
                    if (hal_sleep_manager_is_sleep_handle_alive(uart_sleep_handle[uart_port]) == true) {
                        hal_sleep_manager_unlock_sleep(uart_sleep_handle[uart_port]);
                    }
                    g_uart_send_lock_status[uart_port] = false;
                }
            }
            hal_nvic_restore_interrupt_mask(irq_status);
        } else {
            g_uart_frist_send_complete_interrupt[uart_port] = false;
        }
        return;
#endif/*HAL_SLEEP_MANAGER_ENABLED*/
    } else {
        callback = g_uart_callback[uart_port].func;
        arg = g_uart_callback[uart_port].arg;
        if (callback == NULL) {
            UART_ASSERT();
            return;
        }
        /* for callback re-entry issue */
#ifdef AIR_DCHS_MODE_ENABLE
        if (uart_port != HAL_UART_1)
#endif
        vdma_disable_interrupt(channel);
        callback(HAL_UART_EVENT_READY_TO_WRITE, arg);
#ifdef AIR_DCHS_MODE_ENABLE
        if (uart_port != HAL_UART_1)
#endif
        vdma_enable_interrupt(channel);

        status = vdma_get_available_send_space(channel, &empty_space);
        if (status != VDMA_OK) {
            UART_ASSERT();
            return;
        }

        /*  For tx channel.
            If the free space is greater than Compare_space,which means
            the amount of data in Vfifo is less than the threshold
            driver will disable tx DMA channel IRQ.
        */
        /*   buffer top                     threashold           buffer bottom
             | <           compara-space         > |                     |
        */
        compare_space = g_uart_dma_config[uart_port].send_vfifo_buffer_size
                        - g_uart_dma_config[uart_port].send_vfifo_threshold_size;
#ifdef AIR_DCHS_MODE_ENABLE
        if (uart_port != HAL_UART_1) {
#endif
            if (empty_space >= compare_space) {
                status = vdma_disable_interrupt(channel);
                if (status != VDMA_OK) {
                    UART_ASSERT();
                    return;
                }
            }
#ifdef AIR_DCHS_MODE_ENABLE
        }
#endif
    }
}

/* Only triggered by UART error interrupt */
void uart_error_handler(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;
    hal_uart_callback_t callback;
    void *arg;

    uartx = g_uart_regbase[uart_port];

    if (!uart_verify_error(uartx)) {
        // uart_purge_fifo(uartx, 1);
        // uart_purge_fifo(uartx, 0);
        callback = g_uart_callback[uart_port].func;
        arg = g_uart_callback[uart_port].arg;
        if (callback == NULL) {
            UART_ASSERT();
            return;
        }
        callback(HAL_UART_EVENT_TRANSACTION_ERROR, arg);
    }
}

void uart_transaction_done_handler(hal_uart_port_t uart_port)
{
    hal_uart_callback_t callback;
    void *arg;
    callback = g_uart_callback[uart_port].func;
    arg = g_uart_callback[uart_port].arg;

    /*  If the UART BR is too low, the UART HW has not finished sending data when the TX empty IRQ is trigger,
     so it needs to wait for the HW to finish sending data before notifying the upper layer.    */
    if(g_baudrate[uart_port] <= HAL_UART_BAUDRATE_115200){
        uart_wait_empty(uart_port);
    }

    if (callback != NULL) {
        callback(HAL_UART_EVENT_TRANSMISSION_DONE, arg);
    }
}

#ifdef HAL_UART_FEATURE_FLOWCONTROL_CALLBACK
void uart_hw_flowcontrol_handler(hal_uart_port_t uart_port)
{
    hal_uart_callback_t callback;
    void *arg;
    callback = g_uart_callback[uart_port].func;
    arg = g_uart_callback[uart_port].arg;
    if (callback != NULL) {
        callback(HAL_UART_EVENT_HW_FLOW_CTRL, arg);
    }
}

void uart_sw_flowcontrol_handler(hal_uart_port_t uart_port)
{
    hal_uart_callback_t callback;
    void *arg;
    callback = g_uart_callback[uart_port].func;
    arg = g_uart_callback[uart_port].arg;
    if (callback != NULL) {
        callback(HAL_UART_EVENT_SW_FLOW_CTRL, arg);
    }
}

void uart_enable_software_flowcontrol_irq(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;

    uartx = g_uart_regbase[uart_port];
    uartx->IER_UNION.IER_CELLS.XOFFI = UART_IER_XOFFI_MASK;
}

void uart_disable_software_flowcontrol_irq(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;

    uartx = g_uart_regbase[uart_port];
    uartx->IER_UNION.IER_CELLS.XOFFI = ~UART_IER_XOFFI_MASK;
}

#endif /*HAL_UART_FEATURE_FLOWCONTROL_CALLBACK*/

static void uart_dma_callback_handler(vdma_event_t event, void  *user_data)
{
    /* fix warning */
    vdma_event_t local_event = event;
    local_event = local_event;
    uart_dma_callback_data_t *callback_data = (uart_dma_callback_data_t *)user_data;

    if (callback_data->is_rx == true) {
        uart_receive_handler(callback_data->uart_port, false);
    } else {
        uart_send_handler(callback_data->uart_port, false);
    }
}



#ifdef HAL_SLEEP_MANAGER_ENABLED

static hal_uart_port_t g_uart_eint_port[HAL_UART_MAX];

static void eint_uart_handler(void *parameter)
{
    /*
        uart configures all existing ports in eint mode in backup.
        eint isr is executed after the restore function is completed,
        but only the port triggered by eint is executed and deinit all eint.
    */
    hal_uart_port_t uart_port = *((hal_uart_port_t *)parameter);
    hal_uart_callback_t callback;
    void *arg;
    /*mask eint to avoid irq blast.*/
    hal_eint_mask(g_uart_eint_num[uart_port]);
    /* call user callback .send wakeup event to user */
    callback = g_uart_callback[uart_port].func;
    arg = g_uart_callback[uart_port].arg;
    if (callback != NULL) {
        callback(HAL_UART_EVENT_WAKEUP_SLEEP, arg);
    }
    /*  The UART port that received EINT is responsible for deinit eint for other ports,
        otherwise those ports keep eint mode.
    */
    for (uart_port = HAL_UART_0; uart_port < HAL_UART_MAX; uart_port++) {
        hal_eint_deinit(g_uart_eint_num[uart_port]);
    }
}

void uart_backup_all_registers(void)
{
    bool rx_empty_value;
    uint32_t rx_length;
    hal_uart_port_t uart_port;
    uart_backup_para_for_log_t *log_para;

    log_para = (uart_backup_para_for_log_t *)HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_UART_VAR_START;
    g_uart_port_for_logging = log_para->uart_log_port;

    for (uart_port = HAL_UART_0; uart_port < HAL_UART_MAX; uart_port++) {
        if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_UNINITIALIZED) {
            hal_eint_config_t config_eint;
            g_uart_eint_port[uart_port] = uart_port;
            config_eint.trigger_mode = HAL_EINT_EDGE_FALLING;
            config_eint.debounce_time = 0;
            hal_eint_init(g_uart_eint_num[uart_port], &config_eint);
            hal_eint_register_callback(g_uart_eint_num[uart_port], eint_uart_handler, (void *) &g_uart_eint_port[uart_port]);
            hal_eint_unmask(g_uart_eint_num[uart_port]);

            if (uart_port != g_uart_port_for_logging) {
                /*  When entering sleep, the data in the rx buffer may not be taken away, 
                  so you have to wait until the rx buffer is empty before entering sleep. */
                rx_empty_value = uart_query_rx_empty(uart_port);
                rx_length = hal_uart_get_available_receive_bytes(uart_port);
                // log_hal_msgid_info("udcb  uart[%d] rx_empty_value = %d, rx_length = %d\r\n", 3, uart_port, rx_empty_value, rx_length);
                if((rx_empty_value == false) || (rx_length != 0)) {
                    /* If the rx buffer is not empty, manually trigger wakeup sleep event.  */
                    hal_eint_set_software_trigger(g_uart_eint_num[uart_port]);
                    hal_eint_clear_software_trigger(g_uart_eint_num[uart_port]);
                }

                uart_backup_register(uart_port);
            }
        }
    }
}

extern void mux_restore_callback(); //reset mux software pointer
void uart_restore_all_registers(void)
{
    UART_REGISTER_T *uartx;
    VDMA_REGISTER_T *dma_tx;
    VDMA_REGISTER_T *dma_rx;
    uart_backup_para_for_log_t *log_para;

    log_para = (uart_backup_para_for_log_t *)HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_UART_VAR_START;
    g_uart_port_for_logging = log_para->uart_log_port;

    *(volatile uint32_t *)(0x40080074) |= 0x3; // VDMA_CLOCK enable TX and RX chanel UART0
    *(volatile uint32_t *)(0x4008000C) |= 0x3; // global top interrupt enable
    *(volatile uint32_t *)(0x40090074) |= 0x3; // VDMA_CLOCK enable TX and RX chanel UART1
    *(volatile uint32_t *)(0x4009000C) |= 0x3; // global top interrupt enable
    *(volatile uint32_t *)(0x400A0074) |= 0x3; // VDMA_CLOCK enable TX and RX chanel UART2
    *(volatile uint32_t *)(0x400A000C) |= 0x3; // global top interrupt enable

    for (hal_uart_port_t uart_port = HAL_UART_0; uart_port < HAL_UART_MAX; uart_port++) {
        if (uart_port == g_uart_port_for_logging) {
            while (hal_hw_semaphore_take(HW_SEMAPHORE_SLEEP) != HAL_HW_SEMAPHORE_STATUS_OK);

            uartx  = g_uart_regbase[uart_port];
            dma_tx = g_vdma_regbase[uart_port][0];
            dma_rx = g_vdma_regbase[uart_port][1];

            /*The default value of SPM_INFRA_OFF_FLAG is 0xffff, If infra has been powered off. SPM_INFRA_OFF_FLAG can be changed to 0x1.
               if SPM_INFRA_OFF_FLAG is not equal to 0, which means thar infra has been powered off.
            */
            if (SPM_INFRA_OFF_FLAG != 0) {
                /*set to zero to avoid dsp to modify register when dsp wakeups after cm4.*/
                SPM_INFRA_OFF_FLAG = 0;
                /*infra_reinit_flag indicates that INFRA has done a re-initialization*/
                // log_para->infra_reinit_flag  = 0x1;

                /*  Because, DMA's HW read & write pointer's value are same as MUX's software pointer.
                    But, DMA's HW read & write pointer will reset after UART's operation of deinit&init.
                    So, uart need reset reset software pointer in MUX layer,manually.
                */
                mux_restore_callback(uart_port);
                /*restore UART register from share memory*/
                uartx  = g_uart_regbase[uart_port];
                uartx->DLM_DLL                        = log_para->uart_rg_DL            ;
                uartx->FCR_UNION.FCR                  = log_para->uart_rg_FCR           ;
                uartx->LCR_UNION.LCR                  = log_para->uart_rg_LCR           ;
                uartx->HIGHSPEED                      = log_para->uart_rg_HIGHSPEED     ;
                uartx->SAMPLE_REG_UNION.SAMPLE_REG    = log_para->uart_rg_SAMPLEREG     ;
                uartx->RATEFIX_UNION.RATEFIX          = log_para->uart_rg_RATEFIX       ;
                uartx->GUARD                          = log_para->uart_rg_GUARD         ;
                uartx->SLEEP_REG                      = log_para->uart_rg_SLEEPREG      ;
                uartx->IER_UNION.IER                  = log_para->uart_rg_IER           ;
                uartx->RXTRIG                         = log_para->uart_rg_RXTRIG        ;
                uartx->FRACDIV                        = log_para->uart_rg_FRACDIV       ;

                //uartx->FCR_UNION.FCR |=  UART_FCR_CLRT_MASK | UART_FCR_CLRR_MASK; // clear uart hw fifo
                uartx->EFR_UNION.EFR                  = log_para->uart_rg_EFR           ;
                uartx->XON_XOFF_UNION.XON_XOFF        = log_para->uart_rg_XOFF_XON      ;
                uartx->ESCAPE_REG_UNION.ESCAPE_REG    = log_para->uart_rg_ESCAPEREG     ;

                // Disable related VFIFO TX channel interrupt, ITEN = 0
                dma_tx->VDMA_ACKINT                  |= VDMA_ACKINT_BIT_MASK            ;
                dma_tx->VDMA_COUNT                    = log_para->vdma_rg_tx_COUNT      ;
                dma_tx->VDMA_CON_UNION.VDMA_CON       = log_para->vdma_rg_tx_CON        ;
                dma_tx->VDMA_PGMADDR                  = log_para->vdma_rg_tx_PGMADDR    ;
                dma_tx->VDMA_ALTLEN                   = log_para->vdma_rg_tx_ALTLEN     ;
                dma_tx->VDMA_FFSIZE                   = log_para->vdma_rg_tx_FFSIZE     ;
                dma_tx->VDMA_START                   |= VDMA_START_BIT_MASK             ;

                dma_rx->VDMA_ACKINT                  |= VDMA_ACKINT_BIT_MASK            ;
                dma_rx->VDMA_COUNT                    = log_para->vdma_rg_rx_COUNT      ;
                dma_rx->VDMA_CON_UNION.VDMA_CON       = log_para->vdma_rg_rx_CON        ;
                dma_rx->VDMA_PGMADDR                  = log_para->vdma_rg_rx_PGMADDR    ;
                dma_rx->VDMA_ALTLEN                   = log_para->vdma_rg_rx_ALTLEN     ;
                dma_rx->VDMA_FFSIZE                   = log_para->vdma_rg_rx_FFSIZE     ;
                dma_rx->VDMA_START                   |= VDMA_START_BIT_MASK             ;
                uartx->DMA_CON_UNION.DMA_CON          = log_para->uart_rg_DMACON        ;
                uartx->FCR_UNION.FCR |=  UART_FCR_CLRT_MASK | UART_FCR_CLRR_MASK; // clear uart hw fifo
            }
            while (hal_hw_semaphore_give(HW_SEMAPHORE_SLEEP) != HAL_HW_SEMAPHORE_STATUS_OK);
        } else if (uart_port != g_uart_port_for_logging) {
            dma_tx = g_vdma_regbase[uart_port][0];
            if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_UNINITIALIZED) {
                if (dma_tx->VDMA_WRPTR == 0) {
                    mux_restore_callback(uart_port);
                    uart_restore_register(uart_port);
                }
            }
        }
    }
}
#endif /*HAL_SLEEP_MANAGER_ENABLED*/

hal_uart_status_t hal_uart_set_baudrate(hal_uart_port_t uart_port, hal_uart_baudrate_t baudrate)
{
    uint32_t actual_baudrate, irq_status;
    UART_REGISTER_T *uartx;

    if ((!uart_port_is_valid(uart_port)) ||
        (!uart_baudrate_is_valid(baudrate))) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    if (g_uart_hwstatus[uart_port] == UART_HWSTATUS_UNINITIALIZED) {
        return HAL_UART_STATUS_ERROR_UNINITIALIZED;
    }

    hal_nvic_save_and_set_interrupt_mask(&irq_status);
#ifdef HAL_SLEEP_MANAGER_ENABLED
    g_uart_config[uart_port].baudrate = baudrate;
#endif
    g_baudrate[uart_port] = baudrate;
    hal_nvic_restore_interrupt_mask(irq_status);

    uartx = g_uart_regbase[uart_port];
    actual_baudrate = g_uart_baudrate_map[baudrate];

    uart_set_baudrate(uartx, actual_baudrate);

    return HAL_UART_STATUS_OK;
}

hal_uart_status_t hal_uart_set_format(hal_uart_port_t uart_port,
                                      const hal_uart_config_t *config)
{
    uint32_t irq_status;
    UART_REGISTER_T *uartx;

    if ((!uart_port_is_valid(uart_port)) ||
        (!uart_config_is_valid(config))) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    if (g_uart_hwstatus[uart_port] == UART_HWSTATUS_UNINITIALIZED) {
        return HAL_UART_STATUS_ERROR_UNINITIALIZED;
    }

    hal_nvic_save_and_set_interrupt_mask(&irq_status);
#ifdef HAL_SLEEP_MANAGER_ENABLED
    g_uart_config[uart_port].baudrate = config->baudrate;
    g_uart_config[uart_port].word_length = config->word_length;
    g_uart_config[uart_port].stop_bit = config->stop_bit;
    g_uart_config[uart_port].parity = config->parity;
#endif
    hal_nvic_restore_interrupt_mask(irq_status);

    uartx = g_uart_regbase[uart_port];

    hal_uart_set_baudrate(uart_port, config->baudrate);
    uart_set_format(uartx, config->word_length, config->stop_bit, config->parity);

    return HAL_UART_STATUS_OK;
}

hal_uart_status_t hal_uart_init(hal_uart_port_t uart_port, hal_uart_config_t *uart_config)
{
    UART_REGISTER_T *uartx;
    uint32_t i, actual_baudrate, irq_status;
    hal_clock_status_t clock_status;

    if ((!uart_port_is_valid(uart_port)) ||
        (!uart_config_is_valid(uart_config))) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_UNINITIALIZED) {
        hal_nvic_restore_interrupt_mask(irq_status);
        return HAL_UART_STATUS_ERROR_BUSY;
    }

    if (g_uart_global_data_initialized == false) {
        for (i = 0; i < HAL_UART_MAX; i++) {
            g_uart_hwstatus[i] = UART_HWSTATUS_UNINITIALIZED;
#ifdef HAL_SLEEP_MANAGER_ENABLED
            g_uart_flowcontrol_status[i] = UART_FLOWCONTROL_NONE;
            g_uart_frist_send_complete_interrupt[i] = false;
            g_uart_send_lock_status[i] = false;
#endif
            g_uart_callback[i].arg = NULL;
            g_uart_callback[i].func = NULL;
        }
        g_uart_global_data_initialized = true;
    }
#ifdef HAL_SLEEP_MANAGER_ENABLED
    g_uart_config[uart_port].baudrate = uart_config->baudrate;
    g_uart_config[uart_port].word_length = uart_config->word_length;
    g_uart_config[uart_port].stop_bit = uart_config->stop_bit;
    g_uart_config[uart_port].parity = uart_config->parity;
#endif
    g_uart_hwstatus[uart_port] = UART_HWSTATUS_POLL_INITIALIZED;
    hal_nvic_restore_interrupt_mask(irq_status);

#ifndef FPGA_ENV
    clock_status = hal_clock_enable(g_uart_port_to_pdn[uart_port]);
    if (clock_status != HAL_CLOCK_STATUS_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }
#endif

    uartx = g_uart_regbase[uart_port];
    uart_reset_default_value(uartx);
    actual_baudrate = g_uart_baudrate_map[uart_config->baudrate];
    g_baudrate[uart_port] = uart_config->baudrate;
    uart_set_baudrate(uartx, actual_baudrate);
    uart_set_format(uartx, uart_config->word_length, uart_config->stop_bit, uart_config->parity);
    uart_set_fifo(uartx);
#ifdef HAL_SLEEP_MANAGER_ENABLED
    //uart_set_sleep_mode(uartx);
    //uart_set_sleep_idle_fc_mode(uartx);
    sleep_management_register_all_secure_suspend_callback(SLEEP_BACKUP_RESTORE_UART, (sleep_management_suspend_callback_t)uart_backup_all_registers, NULL);
    sleep_management_register_all_secure_resume_callback(SLEEP_BACKUP_RESTORE_UART, (sleep_management_resume_callback_t)uart_restore_all_registers, NULL);
#endif
    return HAL_UART_STATUS_OK;
}

hal_uart_status_t hal_uart_deinit(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;
    VDMA_REGISTER_T *dmax;
    vdma_channel_t tx_dma_channel, rx_dma_channel;
    uint32_t irq_status;
    vdma_status_t status;
    hal_clock_status_t clock_status;

    if (!uart_port_is_valid(uart_port)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    if (g_uart_hwstatus[uart_port] == UART_HWSTATUS_UNINITIALIZED) {
        return HAL_UART_STATUS_ERROR_UNINITIALIZED;
    }

    uartx = g_uart_regbase[uart_port];
    /* wait all left data sent out before deinit. */

    uart_wait_empty(uart_port);
    /* unregister vdma module */
    if (g_uart_hwstatus[uart_port] == UART_HWSTATUS_DMA_INITIALIZED) {
        tx_dma_channel = uart_port_to_dma_channel(uart_port, 0);
        rx_dma_channel = uart_port_to_dma_channel(uart_port, 1);

        status = vdma_disable_interrupt(tx_dma_channel);
        if (status != VDMA_OK) {
            UART_ASSERT();
            return HAL_UART_STATUS_ERROR;
        }
        status = vdma_disable_interrupt(rx_dma_channel);
        if (status != VDMA_OK) {
            UART_ASSERT();
            return HAL_UART_STATUS_ERROR;
        }

        status = vdma_stop(tx_dma_channel);
        if (status != VDMA_OK) {
            UART_ASSERT();
            return HAL_UART_STATUS_ERROR;
        }
        status = vdma_stop(rx_dma_channel);
        if (status != VDMA_OK) {
            UART_ASSERT();
            return HAL_UART_STATUS_ERROR;
        }

        //ack pending vdma irq
        //dmax = (VDMA_REGISTER_T*)g_uart_dma_base[uart_port]; //TODO
        dmax = (VDMA_REGISTER_T *)g_vdma_regbase[uart_port][0];
        dmax->VDMA_ACKINT = 0x8000;

        status = vdma_deinit(tx_dma_channel);
        if (status != VDMA_OK) {
            UART_ASSERT();
            return HAL_UART_STATUS_ERROR;
        }
        status = vdma_deinit(rx_dma_channel);
        if (status != VDMA_OK) {
            UART_ASSERT();
            return HAL_UART_STATUS_ERROR;
        }
        hal_nvic_disable_irq(g_uart_port_to_irq_num[uart_port]);
    }
    uart_reset_default_value(uartx);

    //clr pending uart irq
    uart_query_interrupt_type(uartx);

#ifndef FPGA_ENV
    clock_status = hal_clock_disable(g_uart_port_to_pdn[uart_port]);
    if (clock_status != HAL_CLOCK_STATUS_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }
#endif

    //g_uart_callback[uart_port].func = NULL;
    //g_uart_callback[uart_port].arg = NULL;

    hal_nvic_save_and_set_interrupt_mask(&irq_status);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    g_uart_frist_send_complete_interrupt[uart_port] = false;
    g_uart_flowcontrol_status[uart_port] = UART_FLOWCONTROL_NONE;
#endif
    g_uart_hwstatus[uart_port] = UART_HWSTATUS_UNINITIALIZED;
    rx_timeout_irq_count[uart_port] = 0;   /*  clear not continuous rx timeout irq  count*/
    g_uart_receive_break_single_status[uart_port] = false; /* clear break single irq  status*/

    hal_nvic_restore_interrupt_mask(irq_status);

    return HAL_UART_STATUS_OK;
}

void hal_uart_put_char(hal_uart_port_t uart_port, char byte)
{
    UART_REGISTER_T *uartx;

    if (!uart_port_is_valid(uart_port)) {
        return;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_POLL_INITIALIZED) {
        return;
    }

    uartx = g_uart_regbase[uart_port];
    uart_put_char_block(uartx, byte);
}

uint32_t hal_uart_send_polling(hal_uart_port_t uart_port, const uint8_t *data, uint32_t size)
{
    uint32_t i = 0;

    if ((!uart_port_is_valid(uart_port)) || (data == NULL)) {
        return 0;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_POLL_INITIALIZED) {
        return 0;
    }

    for (i = 0; i < size; i++) {
        hal_uart_put_char(uart_port, *data);
        data++;
    }

    return size;
}

uint32_t hal_uart_send_dma(hal_uart_port_t uart_port, const uint8_t *data, uint32_t size)
{
    vdma_channel_t channel;
    uint32_t real_count, avail_space;
    vdma_status_t status;
#ifdef HAL_SLEEP_MANAGER_ENABLED
    uint32_t irq_status;
#endif

    if ((!uart_port_is_valid(uart_port)) || (data == NULL) || (size == 0)) {
        return 0;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_DMA_INITIALIZED) {
        return 0;
    }

#ifdef HAL_SLEEP_MANAGER_ENABLED
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    if (g_uart_send_lock_status[uart_port] == false) {
        hal_sleep_manager_lock_sleep(uart_sleep_handle[uart_port]);
        g_uart_send_lock_status[uart_port] = true;
    }
    hal_nvic_restore_interrupt_mask(irq_status);
#endif

    channel = uart_port_to_dma_channel(uart_port, 0);

    status = vdma_get_available_send_space(channel, &avail_space);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return 0;
    }

    if (avail_space >= size) {
        real_count = size;
    } else {
        real_count = avail_space;
    }

    /* Update to multiple byte push operation */
    status = vdma_push_data_multi_bytes(channel, (uint8_t *)data, real_count);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return 0;
    }

    /* If avail space is not enough, turn on TX IRQ
       * so that UART driver can notice user when user's data has been sent out.
       */
    if (real_count == avail_space) {
        status = vdma_enable_interrupt(channel);
        if (status != VDMA_OK) {
            UART_ASSERT();
            return 0;
        }
    }

    return real_count;
}

char hal_uart_get_char(hal_uart_port_t uart_port)
{
    char data;
    UART_REGISTER_T *uartx;

    if (!uart_port_is_valid(uart_port)) {
        return 0;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_POLL_INITIALIZED) {
        return 0;
    }

    uartx = g_uart_regbase[uart_port];

    data = uart_get_char_block(uartx);

    return data;
}

uint32_t hal_uart_get_char_unblocking(hal_uart_port_t uart_port)
{
    uint32_t data;
    UART_REGISTER_T *uartx;

    if (!uart_port_is_valid(uart_port)) {
        return 0;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_POLL_INITIALIZED) {
        return 0;
    }

    uartx = g_uart_regbase[uart_port];

    data = uart_get_char_unblocking(uartx);

    return data;
}

uint32_t hal_uart_receive_polling(hal_uart_port_t uart_port, uint8_t *buffer, uint32_t size)
{
    uint32_t i;
    uint8_t *pbuf = buffer;

    if ((!uart_port_is_valid(uart_port)) ||
        (buffer == NULL)) {
        return 0;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_POLL_INITIALIZED) {
        return 0;
    }

    for (i = 0; i < size; i++) {
        pbuf[i] = hal_uart_get_char(uart_port);
    }

    return size;
}

uint32_t hal_uart_receive_dma(hal_uart_port_t uart_port, uint8_t *buffer, uint32_t size)
{
    vdma_channel_t channel;
    uint32_t receive_count, avail_count;
    vdma_status_t status;

    if ((!uart_port_is_valid(uart_port)) ||
        (buffer == NULL) ||
        (size == 0)) {
        return 0;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_DMA_INITIALIZED) {
        return 0;
    }

    channel = uart_port_to_dma_channel(uart_port, 1);

    status = vdma_get_available_receive_bytes(channel, &avail_count);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return 0;
    }

    if (avail_count < size) {
        receive_count = avail_count;
    } else {
        receive_count = size;
    }

    status = vdma_pop_data_multi_bytes(channel, buffer, receive_count);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return 0;
    }

    /* If avail bytes is not enough, turn on RX IRQ
       * so that UART driver can notice user when new user's data has been received.
       */
    if (receive_count == avail_count) {
        status = vdma_enable_interrupt(channel);
        if (status != VDMA_OK) {
            UART_ASSERT();
            return 0;
        }
    }

    return receive_count;
}

void uart_disable_irq(hal_uart_port_t uart_port)
{
    g_uart_disable_irq[uart_port] = true;
}

static void uart_start_dma_transmission(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;
    vdma_channel_t tx_dma_channel, rx_dma_channel;
    vdma_status_t status;
    uint32_t irq_status;

    uartx = g_uart_regbase[uart_port];
    tx_dma_channel = uart_port_to_dma_channel(uart_port, 0);
    rx_dma_channel = uart_port_to_dma_channel(uart_port, 1);

    status = vdma_enable_interrupt(rx_dma_channel);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return;
    }
    status = vdma_start(tx_dma_channel);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return;
    }
    status = vdma_start(rx_dma_channel);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return;
    }
    uart_enable_dma(uartx);
    uart_set_fifo(uartx); //when DMA is enabled, the UART hw fifo cannot have data, otherwise the RX timeout IRQ cannot be triggered.

    /*mask irq to see if it exit pending irq hear,if true,clear pending irq.*/
    hal_nvic_save_and_set_interrupt_mask(&irq_status);
#ifdef HAL_SLEEP_MANAGER_ENABLED
    if (g_uart_disable_irq[uart_port] == false) {
        uart_unmask_send_interrupt(uartx);
    }

    g_uart_frist_send_complete_interrupt[uart_port] = true;
#endif

    /*  Finally enable IRQ,  Prevent IRQ from triggering during init, which can cause system crash if it cannot be cleared  */
    uart_unmask_receive_interrupt(uartx);

    hal_nvic_enable_irq(g_uart_port_to_irq_num[uart_port]);
    /*when UART is initialized after exitting sleep,one dummy irq will generate.
        it is necessary to clear this irq to avoid app unnecessary processing event and print syslog.
    */
    if (0xFF != hal_nvic_get_pending_irq(g_uart_port_to_irq_num[uart_port])) {
        uartx  = g_uart_regbase[uart_port];
        uart_query_interrupt_type(uartx);
        hal_nvic_clear_pending_irq(g_uart_port_to_irq_num[uart_port]);
    }
    hal_nvic_restore_interrupt_mask(irq_status);

    /* Do not need ready-to-write due to use MUX porting layer*/
    // if (uart_port != g_uart_port_for_logging) {
    //     status = vdma_enable_interrupt(tx_dma_channel);
    //     if (status != VDMA_OK) {
    //         UART_ASSERT();
    //         return;
    //     }
    // }

}

hal_uart_status_t hal_uart_register_callback(hal_uart_port_t uart_port,
                                             hal_uart_callback_t user_callback,
                                             void *user_data)
{
    vdma_channel_t tx_dma_channel, rx_dma_channel;
    uint32_t irq_status;
    vdma_status_t status;
    hal_nvic_status_t nvic_status;

    if ((!uart_port_is_valid(uart_port)) ||
        (user_callback == NULL)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_POLL_INITIALIZED) {
        return HAL_UART_STATUS_ERROR_UNINITIALIZED;
    }

    tx_dma_channel = uart_port_to_dma_channel(uart_port, 0);
    rx_dma_channel = uart_port_to_dma_channel(uart_port, 1);

    g_uart_callback[uart_port].func = user_callback;
    g_uart_callback[uart_port].arg = user_data;

    uart_dma_channel_to_callback_data(tx_dma_channel, &g_uart_dma_callback_data[uart_port * 2]);
    status = vdma_register_callback(tx_dma_channel, uart_dma_callback_handler, &g_uart_dma_callback_data[uart_port * 2]);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }
    uart_dma_channel_to_callback_data(rx_dma_channel, &g_uart_dma_callback_data[(uart_port * 2) + 1]);
    status = vdma_register_callback(rx_dma_channel, uart_dma_callback_handler, &g_uart_dma_callback_data[(uart_port * 2) + 1]);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }

    nvic_status = hal_nvic_register_isr_handler(g_uart_port_to_irq_num[uart_port], uart_interrupt_handler);
    if (nvic_status != HAL_NVIC_STATUS_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }

    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    g_uart_hwstatus[uart_port] = UART_HWSTATUS_DMA_INITIALIZED;
    hal_nvic_restore_interrupt_mask(irq_status);

    uart_start_dma_transmission(uart_port);

    return HAL_UART_STATUS_OK;
}

uint32_t hal_uart_get_available_send_space(hal_uart_port_t uart_port)
{
    vdma_channel_t channel;
    uint32_t roomleft;
    vdma_status_t status;

    if (!uart_port_is_valid(uart_port)) {
        return 0;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_DMA_INITIALIZED) {
        return 0;
    }

    channel = uart_port_to_dma_channel(uart_port, 0);
    status = vdma_get_available_send_space(channel, &roomleft);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return 0;
    }

    return roomleft;
}

uint32_t hal_uart_get_available_receive_bytes(hal_uart_port_t uart_port)
{
    vdma_channel_t channel;
    uint32_t avail;
    vdma_status_t status;

    if (!uart_port_is_valid(uart_port)) {
        return 0;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_DMA_INITIALIZED) {
        return 0;
    }

    channel = uart_port_to_dma_channel(uart_port, 1);
    status = vdma_get_available_receive_bytes(channel, &avail);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return 0;
    }

    return avail;
}

hal_uart_status_t hal_uart_set_hardware_flowcontrol(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;

    if (!uart_port_is_valid(uart_port)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    if (g_uart_hwstatus[uart_port] == UART_HWSTATUS_UNINITIALIZED) {
        return HAL_UART_STATUS_ERROR_UNINITIALIZED;
    }

    uartx = g_uart_regbase[uart_port];

    uart_set_hardware_flowcontrol(uartx);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    g_uart_flowcontrol_status[uart_port] = UART_FLOWCONTROL_HARDWARE;
#endif

    return HAL_UART_STATUS_OK;
}

hal_uart_status_t hal_uart_set_software_flowcontrol(hal_uart_port_t uart_port,
                                                    uint8_t xon,
                                                    uint8_t xoff,
                                                    uint8_t escape_character)
{
    UART_REGISTER_T *uartx;

    if (!uart_port_is_valid(uart_port)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    if (g_uart_hwstatus[uart_port] == UART_HWSTATUS_UNINITIALIZED) {
        return HAL_UART_STATUS_ERROR_UNINITIALIZED;
    }

    uartx = g_uart_regbase[uart_port];

    uart_set_software_flowcontrol(uartx, xon, xoff, escape_character);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    g_uart_flowcontrol_status[uart_port] = UART_FLOWCONTROL_SOFTWARE;
    g_uart_sw_flowcontrol_config[uart_port].xon = xon;
    g_uart_sw_flowcontrol_config[uart_port].xoff = xoff;
    g_uart_sw_flowcontrol_config[uart_port].escape_character = escape_character;
#endif

    return HAL_UART_STATUS_OK;
}

hal_uart_status_t hal_uart_disable_flowcontrol(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;

    if (!uart_port_is_valid(uart_port)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    if (g_uart_hwstatus[uart_port] == UART_HWSTATUS_UNINITIALIZED) {
        return HAL_UART_STATUS_ERROR_UNINITIALIZED;
    }

    uartx = g_uart_regbase[uart_port];

    uart_disable_flowcontrol(uartx);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    g_uart_flowcontrol_status[uart_port] = UART_FLOWCONTROL_NONE;
#endif

    return HAL_UART_STATUS_OK;
}


hal_uart_status_t hal_uart_set_dma(hal_uart_port_t uart_port, const hal_uart_dma_config_t *dma_config)
{
    uint32_t irq_status;
    vdma_config_t internal_dma_config;
    vdma_channel_t tx_dma_channel, rx_dma_channel;
    vdma_status_t status;

    if (!uart_port_is_valid(uart_port)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }
    if ((dma_config->send_vfifo_buffer == NULL) ||
        (dma_config->receive_vfifo_buffer == NULL)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }
    if ((dma_config->send_vfifo_buffer_size >= UART_DMA_MAX_SETTING_VALUE) ||
        (dma_config->send_vfifo_threshold_size >= UART_DMA_MAX_SETTING_VALUE) ||
        (dma_config->send_vfifo_threshold_size > dma_config->send_vfifo_buffer_size)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }
    if ((dma_config->receive_vfifo_buffer_size >= UART_DMA_MAX_SETTING_VALUE) ||
        (dma_config->receive_vfifo_threshold_size >= UART_DMA_MAX_SETTING_VALUE) ||
        (dma_config->receive_vfifo_alert_size >= UART_DMA_MAX_SETTING_VALUE) ||
        (dma_config->receive_vfifo_threshold_size > dma_config->receive_vfifo_buffer_size) ||
        (dma_config->receive_vfifo_alert_size > dma_config->receive_vfifo_buffer_size)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    if (g_uart_hwstatus[uart_port] != UART_HWSTATUS_POLL_INITIALIZED) {
        return HAL_UART_STATUS_ERROR_UNINITIALIZED;
    }

    tx_dma_channel = uart_port_to_dma_channel(uart_port, 0);
    rx_dma_channel = uart_port_to_dma_channel(uart_port, 1);

    status = vdma_init(tx_dma_channel);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }
    internal_dma_config.base_address = (uint32_t)dma_config->send_vfifo_buffer;
    internal_dma_config.size = dma_config->send_vfifo_buffer_size;
    status = vdma_configure(tx_dma_channel, &internal_dma_config);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }
    status = vdma_set_threshold(tx_dma_channel, dma_config->send_vfifo_threshold_size);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }

    status = vdma_init(rx_dma_channel);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }
    internal_dma_config.base_address = (uint32_t)dma_config->receive_vfifo_buffer;
    internal_dma_config.size = dma_config->receive_vfifo_buffer_size;
    status = vdma_configure(rx_dma_channel, &internal_dma_config);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }
    status = vdma_set_threshold(rx_dma_channel, dma_config->receive_vfifo_threshold_size);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }
    status = vdma_set_alert_length(rx_dma_channel, dma_config->receive_vfifo_alert_size);
    if (status != VDMA_OK) {
        UART_ASSERT();
        return HAL_UART_STATUS_ERROR;
    }

    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    g_uart_dma_config[uart_port].send_vfifo_buffer = dma_config->send_vfifo_buffer;
    g_uart_dma_config[uart_port].send_vfifo_buffer_size = dma_config->send_vfifo_buffer_size;
    g_uart_dma_config[uart_port].send_vfifo_threshold_size = dma_config->send_vfifo_threshold_size;
    g_uart_dma_config[uart_port].receive_vfifo_alert_size = dma_config->receive_vfifo_alert_size;
    g_uart_dma_config[uart_port].receive_vfifo_buffer = dma_config->receive_vfifo_buffer;
    g_uart_dma_config[uart_port].receive_vfifo_buffer_size = dma_config->receive_vfifo_buffer_size;
    g_uart_dma_config[uart_port].receive_vfifo_threshold_size = dma_config->receive_vfifo_threshold_size;
    hal_nvic_restore_interrupt_mask(irq_status);

    return HAL_UART_STATUS_OK;
}



#ifdef HAL_UART_FEATURE_VFIFO_DMA_TIMEOUT
hal_uart_status_t hal_uart_set_dma_timeout(hal_uart_port_t uart_port, uint32_t timeout)
{
    UART_REGISTER_T *uartx;

    if (!uart_port_is_valid(uart_port)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }
    if (timeout > HAL_UART_TIMEOUT_VALUE_MAX) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    uartx = g_uart_regbase[uart_port];
    uart_set_timeout_value(uartx, timeout);

    return HAL_UART_STATUS_OK;
}
#endif /*HAL_UART_FEATURE_VFIFO_DMA_TIMEOUT*/

hal_uart_status_t hal_uart_set_auto_baudrate(hal_uart_port_t uart_port, bool is_enable)
{
    UART_REGISTER_T *uartx;
    uint32_t current_baudrate;

    if (!uart_port_is_valid(uart_port)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    uartx = g_uart_regbase[uart_port];
    current_baudrate = g_uart_baudrate_map[g_baudrate[uart_port]];

    uart_set_auto_baudrate(uartx, is_enable, current_baudrate);
    return HAL_UART_STATUS_OK;
}

void uart_wait_empty(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;

    uartx = g_uart_regbase[uart_port];
    /* wait all left data sent out before deinit. */
    while (!((uartx->LSR) & UART_LSR_TEMT_MASK)) {
        /*
            There is a risk that uart receive xoff when MCU is waitting UART fifo empty.
            Add break condition to avoid system hang.(such as disconnection of logging tool/other device ).
            But it is possible to bring about data lost when uart receive flow control signal under normal mode.
        */
        if (g_uart_flowcontrol_status[uart_port] == UART_FLOWCONTROL_SOFTWARE) {
            if (uartx->MCR_UNION.MCR & UART_MCR_XOFF_STATUS_MASK) {
                g_uart_not_wait_empty_status = true;
                break;
            }
        } else if (g_uart_flowcontrol_status[uart_port] == UART_FLOWCONTROL_HARDWARE) {
            if (uartx->MCR_UNION.MCR & UART_MCR_RTS_MASK) {
                g_uart_not_wait_empty_status = true;
                break;
            }
        }
    }
}


/*for logging*/
hal_uart_status_t uart_backup_log_para_to_share_buf(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;
    VDMA_REGISTER_T *dma_tx;
    VDMA_REGISTER_T *dma_rx;
    uart_backup_para_for_log_t *log_para;

    g_uart_port_for_logging = uart_port;
    log_para = (uart_backup_para_for_log_t *)HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_UART_VAR_START;
    uartx  = g_uart_regbase[uart_port];
    dma_tx = g_vdma_regbase[uart_port][0];
    dma_rx = g_vdma_regbase[uart_port][1];

    if (uart_port > HAL_UART_2) {
        log_para->uart_log_port      = g_uart_port_for_logging; //If the log port is not UART, then all UART ports need to be backedup.
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    log_para->uart_log_port      = g_uart_port_for_logging;
    log_para->uart_baudrate      = g_uart_baudrate_map[g_uart_config[uart_port].baudrate];
    log_para->uart_rg_DL         = uartx->DLM_DLL;
    log_para->uart_rg_FCR        = uartx->FCR_UNION.FCR;
    log_para->uart_rg_IER        = uartx->IER_UNION.IER;
    log_para->uart_rg_EFR        = uartx->EFR_UNION.EFR;
    log_para->uart_rg_LCR        = uartx->LCR_UNION.LCR;
    log_para->uart_rg_XOFF_XON   = uartx->XON_XOFF_UNION.XON_XOFF;
    log_para->uart_rg_HIGHSPEED  = uartx->HIGHSPEED;
    log_para->uart_rg_SAMPLEREG  = uartx->SAMPLE_REG_UNION.SAMPLE_REG;
    log_para->uart_rg_RATEFIX    = uartx->RATEFIX_UNION.RATEFIX;
    log_para->uart_rg_GUARD      = uartx->GUARD;
    log_para->uart_rg_ESCAPEREG  = uartx->ESCAPE_REG_UNION.ESCAPE_REG;
    log_para->uart_rg_SLEEPREG   = uartx->SLEEP_REG;
    log_para->uart_rg_DMACON     = uartx->DMA_CON_UNION.DMA_CON;
    log_para->uart_rg_RXTRIG     = uartx->RXTRIG;
    log_para->uart_rg_FRACDIV    = uartx->FRACDIV;
    log_para->vdma_rg_tx_COUNT   = dma_tx->VDMA_COUNT;
    log_para->vdma_rg_tx_CON     = dma_tx->VDMA_CON_UNION.VDMA_CON;
    log_para->vdma_rg_tx_PGMADDR = dma_tx->VDMA_PGMADDR;
    log_para->vdma_rg_tx_ALTLEN  = dma_tx->VDMA_ALTLEN;
    log_para->vdma_rg_tx_FFSIZE  = dma_tx->VDMA_FFSIZE;
    log_para->vdma_rg_rx_COUNT   = dma_rx->VDMA_COUNT;
    log_para->vdma_rg_rx_CON     = dma_rx->VDMA_CON_UNION.VDMA_CON;
    log_para->vdma_rg_rx_PGMADDR = dma_rx->VDMA_PGMADDR;
    log_para->vdma_rg_rx_ALTLEN  = dma_rx->VDMA_ALTLEN;
    log_para->vdma_rg_rx_FFSIZE  = dma_rx->VDMA_FFSIZE;
    log_para->infra_reinit_flag  = 0x1;

    return HAL_UART_STATUS_OK;
}

#ifdef HAL_SLEEP_MANAGER_ENABLED
hal_uart_status_t uart_backup_register(hal_uart_port_t uart_port)
{
    if (uart_port > HAL_UART_2) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    UART_REGISTER_T *uartx;
    VDMA_REGISTER_T *dma_tx;
    VDMA_REGISTER_T *dma_rx;

    uartx  = g_uart_regbase[uart_port];
    dma_tx = g_vdma_regbase[uart_port][0];
    dma_rx = g_vdma_regbase[uart_port][1];

    uart_backup_para[uart_port].uart_log_port      = uart_port;
    uart_backup_para[uart_port].uart_baudrate      = g_uart_baudrate_map[g_uart_config[uart_port].baudrate];
    uart_backup_para[uart_port].uart_rg_DL         = uartx->DLM_DLL;
    uart_backup_para[uart_port].uart_rg_FCR        = uartx->FCR_UNION.FCR;
    uart_backup_para[uart_port].uart_rg_IER        = uartx->IER_UNION.IER;
    uart_backup_para[uart_port].uart_rg_EFR        = uartx->EFR_UNION.EFR;
    uart_backup_para[uart_port].uart_rg_LCR        = uartx->LCR_UNION.LCR;
    uart_backup_para[uart_port].uart_rg_XOFF_XON   = uartx->XON_XOFF_UNION.XON_XOFF;
    uart_backup_para[uart_port].uart_rg_HIGHSPEED  = uartx->HIGHSPEED;
    uart_backup_para[uart_port].uart_rg_SAMPLEREG  = uartx->SAMPLE_REG_UNION.SAMPLE_REG;
    uart_backup_para[uart_port].uart_rg_RATEFIX    = uartx->RATEFIX_UNION.RATEFIX;
    uart_backup_para[uart_port].uart_rg_GUARD      = uartx->GUARD;
    uart_backup_para[uart_port].uart_rg_ESCAPEREG  = uartx->ESCAPE_REG_UNION.ESCAPE_REG;
    uart_backup_para[uart_port].uart_rg_SLEEPREG   = uartx->SLEEP_REG;
    //uart_backup_para[uart_port].uart_rg_DMACON     = uartx->DMA_CON_UNION.DMA_CON;
    uart_backup_para[uart_port].uart_rg_RXTRIG     = uartx->RXTRIG;
    uart_backup_para[uart_port].uart_rg_FRACDIV    = uartx->FRACDIV;

    uart_backup_para[uart_port].vdma_rg_tx_COUNT   = dma_tx->VDMA_COUNT;
    uart_backup_para[uart_port].vdma_rg_tx_CON     = dma_tx->VDMA_CON_UNION.VDMA_CON;
    uart_backup_para[uart_port].vdma_rg_tx_PGMADDR = dma_tx->VDMA_PGMADDR;
    uart_backup_para[uart_port].vdma_rg_tx_ALTLEN  = dma_tx->VDMA_ALTLEN;
    uart_backup_para[uart_port].vdma_rg_tx_FFSIZE  = dma_tx->VDMA_FFSIZE;

    uart_backup_para[uart_port].vdma_rg_rx_COUNT   = dma_rx->VDMA_COUNT;
    uart_backup_para[uart_port].vdma_rg_rx_CON     = dma_rx->VDMA_CON_UNION.VDMA_CON;
    uart_backup_para[uart_port].vdma_rg_rx_PGMADDR = dma_rx->VDMA_PGMADDR;
    uart_backup_para[uart_port].vdma_rg_rx_ALTLEN  = dma_rx->VDMA_ALTLEN;
    uart_backup_para[uart_port].vdma_rg_rx_FFSIZE  = dma_rx->VDMA_FFSIZE;

    uart_backup_para[uart_port].infra_reinit_flag  = 0x1;

    return HAL_UART_STATUS_OK;
}

hal_uart_status_t uart_restore_register(hal_uart_port_t uart_port)
{
    if (uart_port > HAL_UART_2) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    UART_REGISTER_T *uartx;
    VDMA_REGISTER_T *dma_tx;
    VDMA_REGISTER_T *dma_rx;

    uartx  = g_uart_regbase[uart_port];
    dma_tx = g_vdma_regbase[uart_port][0];
    dma_rx = g_vdma_regbase[uart_port][1];

    uartx->DLM_DLL                        = uart_backup_para[uart_port].uart_rg_DL            ;
    uartx->FCR_UNION.FCR                  = uart_backup_para[uart_port].uart_rg_FCR           ;
    uartx->LCR_UNION.LCR                  = uart_backup_para[uart_port].uart_rg_LCR           ;
    uartx->HIGHSPEED                      = uart_backup_para[uart_port].uart_rg_HIGHSPEED     ;
    uartx->SAMPLE_REG_UNION.SAMPLE_REG    = uart_backup_para[uart_port].uart_rg_SAMPLEREG     ;
    uartx->RATEFIX_UNION.RATEFIX          = uart_backup_para[uart_port].uart_rg_RATEFIX       ;
    uartx->GUARD                          = uart_backup_para[uart_port].uart_rg_GUARD         ;
    uartx->SLEEP_REG                      = uart_backup_para[uart_port].uart_rg_SLEEPREG      ;
    uartx->IER_UNION.IER                  = uart_backup_para[uart_port].uart_rg_IER           ;
    uartx->RXTRIG                         = uart_backup_para[uart_port].uart_rg_RXTRIG        ;
    uartx->FRACDIV                        = uart_backup_para[uart_port].uart_rg_FRACDIV       ;

    //uartx->FCR_UNION.FCR |=  UART_FCR_CLRT_MASK | UART_FCR_CLRR_MASK; // clear uart hw fifo
    uartx->EFR_UNION.EFR                  = uart_backup_para[uart_port].uart_rg_EFR           ;
    uartx->XON_XOFF_UNION.XON_XOFF        = uart_backup_para[uart_port].uart_rg_XOFF_XON      ;
    uartx->ESCAPE_REG_UNION.ESCAPE_REG    = uart_backup_para[uart_port].uart_rg_ESCAPEREG     ;

    // Disable related VFIFO TX channel interrupt, ITEN = 0
    dma_tx->VDMA_ACKINT                  |= VDMA_ACKINT_BIT_MASK            ;
    dma_tx->VDMA_COUNT                    = uart_backup_para[uart_port].vdma_rg_tx_COUNT      ;
    dma_tx->VDMA_CON_UNION.VDMA_CON       = uart_backup_para[uart_port].vdma_rg_tx_CON        ;
    dma_tx->VDMA_PGMADDR                  = uart_backup_para[uart_port].vdma_rg_tx_PGMADDR    ;
    dma_tx->VDMA_ALTLEN                   = uart_backup_para[uart_port].vdma_rg_tx_ALTLEN     ;
    dma_tx->VDMA_FFSIZE                   = uart_backup_para[uart_port].vdma_rg_tx_FFSIZE     ;
    dma_tx->VDMA_START                   |= VDMA_START_BIT_MASK             ;

    dma_rx->VDMA_ACKINT                  |= VDMA_ACKINT_BIT_MASK            ;
    dma_rx->VDMA_COUNT                    = uart_backup_para[uart_port].vdma_rg_rx_COUNT      ;
    dma_rx->VDMA_CON_UNION.VDMA_CON       = uart_backup_para[uart_port].vdma_rg_rx_CON        ;
    dma_rx->VDMA_PGMADDR                  = uart_backup_para[uart_port].vdma_rg_rx_PGMADDR    ;
    dma_rx->VDMA_ALTLEN                   = uart_backup_para[uart_port].vdma_rg_rx_ALTLEN     ;
    dma_rx->VDMA_FFSIZE                   = uart_backup_para[uart_port].vdma_rg_rx_FFSIZE     ;
    dma_rx->VDMA_START                   |= VDMA_START_BIT_MASK             ;
    // uartx->DMA_CON_UNION.DMA_CON          = uart_backup_para[uart_port].uart_rg_DMACON        ;

    uart_enable_dma(uartx); // When restoring, reconfigure DMA_CON
    uartx->FCR_UNION.FCR |=  UART_FCR_CLRT_MASK | UART_FCR_CLRR_MASK; // clear uart hw fifo

    return HAL_UART_STATUS_OK;
}
#endif

extern vdma_status_t vdma_get_hw_read_offset(vdma_channel_t channel, uint32_t *read_offset);
extern vdma_status_t vdma_get_hw_write_offset(vdma_channel_t channel, uint32_t *read_offset);

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM uint32_t uart_get_hw_rptr(hal_uart_port_t uart_port, int32_t is_rx)
{
    vdma_channel_t dma_channel;
    uint32_t read_offset;

    dma_channel = uart_port_to_dma_channel(uart_port, is_rx);
    vdma_get_hw_read_offset(dma_channel, &read_offset);

    return read_offset;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM hal_uart_status_t uart_set_sw_move_byte(hal_uart_port_t uart_port, int32_t is_rx, uint16_t sw_move_byte)
{
    vdma_status_t status;
    vdma_channel_t dma_channel;

    dma_channel = uart_port_to_dma_channel(uart_port, is_rx);
    status = vdma_set_sw_move_byte(dma_channel, sw_move_byte);
    if (status != VDMA_OK) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }
    return HAL_UART_STATUS_OK;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM uint32_t uart_get_hw_wptr(hal_uart_port_t uart_port, int32_t is_rx)
{
    vdma_channel_t dma_channel;
    uint32_t write_offset;

    dma_channel = uart_port_to_dma_channel(uart_port, is_rx);
    vdma_get_hw_write_offset(dma_channel, &write_offset);

    return write_offset;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM bool uart_get_buf_full_status(hal_uart_port_t uart_port, int32_t is_rx)
{
    bool ret;
    vdma_channel_t dma_channel;
    uint32_t receive_bytes, available_space;

    dma_channel = uart_port_to_dma_channel(uart_port, is_rx);
    if (is_rx) {
        if (VDMA_OK != vdma_get_available_receive_bytes(dma_channel, &receive_bytes)) {
            ret = true;
        }
        if (receive_bytes == g_uart_dma_config[uart_port].receive_vfifo_buffer_size) {
            ret = true;
        } else {
            ret = false;
        }
    } else {
        if (VDMA_OK != vdma_get_available_send_space(dma_channel, &available_space)) {
            ret = true;
        }
        if (available_space == 0) {
            ret = true;
        } else {
            ret = false;
        }
    }

    return ret;
}

static uint32_t uart_clear_fifo_flag = 0;
hal_uart_status_t  uart_clear_vfifo_and_fifo(hal_uart_port_t uart_port, uint32_t is_rx)
{
    vdma_channel_t   dma_channel;
    vdma_status_t    vdma_status;
    UART_REGISTER_T  *uartx = g_uart_regbase[uart_port];
    dma_channel = uart_port_to_dma_channel(uart_port, is_rx);

    vdma_status = vdma_stop(dma_channel);
    if (VDMA_OK != vdma_status) {
        uart_clear_fifo_flag = 1;
        return HAL_UART_STATUS_ERROR;
    }

    vdma_status = vdma_start(dma_channel);
    if (VDMA_OK != vdma_status) {
        uart_clear_fifo_flag = 2;
        return HAL_UART_STATUS_ERROR;
    }

    uart_purge_fifo(uartx, is_rx);  // rx:1  tx:0

    uart_clear_fifo_flag = 3;

    return HAL_UART_STATUS_OK;
}

uint32_t uart_send_polling(hal_uart_port_t uart_port, const uint8_t *data, uint32_t size)
{
    uint32_t i = 0;
    UART_REGISTER_T *uartx;

    uartx = g_uart_regbase[uart_port];

    for (i = 0; i < size; i++) {
        uart_put_char_block(uartx, *data);
        data++;
    }

    return size;
}

/*other module used this api*/
uint32_t uart_exception_send_polling(hal_uart_port_t uart_port, const uint8_t *data, uint32_t size)
{
    uint32_t i, j;
    uint32_t loop_count, remainder, curr_word;
    uint8_t *p_word;
    uint32_t *p_data;
    UART_REGISTER_T *uartx;

    uartx = g_uart_regbase[uart_port];

    loop_count = size / 4;
    remainder = size % 4;

    /* DSP IRAM must be fetched with WORD unit. */
    p_data = (uint32_t *)data;
    for (i = 0; i < loop_count; i++) {
        curr_word = p_data[i];
        p_word = (uint8_t *)&curr_word;
        for (j = 0; j < 4; j++) {
            uart_put_char_block(uartx, p_word[j]);
            hal_gpt_delay_us(5);
        }
    }
    if (remainder) {
        curr_word = p_data[i];
        p_word = (uint8_t *)&curr_word;
        for (j = 0; j < remainder; j++) {
            uart_put_char_block(uartx, p_word[j]);
            hal_gpt_delay_us(5);
        }
    }

    return size;
}

/*For Crystal Trim using in at command*/
hal_uart_status_t hal_uart_ext_get_uart_config(hal_uart_port_t uart_port, hal_uart_config_t  *uart_config, hal_uart_dma_config_t *dma_config, uint32_t *callback)
{
    if (!uart_port_is_valid(uart_port)) {
        return HAL_UART_STATUS_ERROR_PARAMETER;
    }

    if (g_uart_hwstatus[uart_port] == UART_HWSTATUS_UNINITIALIZED) {
        return HAL_UART_STATUS_ERROR_UNINITIALIZED;
    }

#ifdef HAL_SLEEP_MANAGER_ENABLED
    *uart_config = g_uart_config[uart_port];
    *dma_config = g_uart_dma_config[uart_port];
    *callback = (uint32_t)g_uart_callback[uart_port].func;
#else
    (void)(uart_config);
    (void)(dma_config);
    (void)(callback);
#endif
    return HAL_UART_STATUS_OK;
}


bool hal_uart_ext_is_dma_mode(hal_uart_port_t uart_port)
{
    if (uart_port >= HAL_UART_MAX) {
        return false;
    }
    if (g_uart_hwstatus[uart_port] & UART_HWSTATUS_DMA_INITIALIZED) {
        return true;
    }
    return false;
}

hal_uart_status_t  hal_uart_ext_set_baudrate(hal_uart_port_t uart_port, uint32_t baudrate)
{
    UART_REGISTER_T *uartx = NULL;

    if (uart_port >= HAL_UART_MAX) {
        return HAL_UART_STATUS_ERROR;
    }
    uartx = g_uart_regbase[uart_port];
    /*Set a signal frequency,Baud rate is twice the frequency*/
    uart_set_baudrate(uartx, baudrate * 2);
    uartx->GUARD = 0x00;
    return HAL_UART_STATUS_OK;
}

/* return true means current uart idle  */
bool hal_uart_query_idle(hal_uart_port_t uart_port)
{
    uint32_t isTxIdle;
    uint32_t isRxIdle;
    uint32_t rxLength;

    isRxIdle = uart_query_rx_empty(uart_port);
    isTxIdle = uart_query_tx_empty(uart_port);
    rxLength = hal_uart_get_available_receive_bytes(uart_port);
    if (rxLength || !isTxIdle || !isRxIdle) {
        return false;
    } else {
        return true;
    }
}

ATTR_TEXT_IN_TCM void uart_enable_rx_interrupt(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;

    uartx = g_uart_regbase[uart_port];
    uart_unmask_receive_interrupt(uartx);
}

ATTR_TEXT_IN_TCM void uart_disable_rx_interrupt(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;

    uartx = g_uart_regbase[uart_port];
    uart_mask_receive_interrupt(uartx);
}

ATTR_TEXT_IN_TCM void uart_enable_tx_interrupt(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;

    uartx = g_uart_regbase[uart_port];
    uart_unmask_send_interrupt(uartx);
}

ATTR_TEXT_IN_TCM void uart_disable_tx_interrupt(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;

    uartx = g_uart_regbase[uart_port];
    uart_mask_send_interrupt(uartx);
}

ATTR_TEXT_IN_TCM void uart_enable_dma_interrupt(hal_uart_port_t uart_port, bool is_rx)
{
    vdma_status_t status;
    vdma_channel_t channel;

    channel = uart_port_to_dma_channel(uart_port, is_rx);
    status  = vdma_enable_interrupt(channel);
    if (status != VDMA_OK) {
        assert(0);
    }
}

ATTR_TEXT_IN_TCM void uart_disable_dma_interrupt(hal_uart_port_t uart_port, bool is_rx)
{
    vdma_status_t status;
    vdma_channel_t channel;

    channel = uart_port_to_dma_channel(uart_port, is_rx);
    status  = vdma_disable_interrupt(channel);
    if (status != VDMA_OK) {
        assert(0);
    }
}

ATTR_TEXT_IN_TCM void uart_config_rx_timeout(hal_uart_port_t uart_port, uint32_t count)
{
    UART_REGISTER_T *uartx;
    uartx = g_uart_regbase[uart_port];

    uartx->RX_TO_CON_UNION.RX_TO_CON_CELLS.RX_TO_MODE = 0x1; //enable customize rx timeout mode
    uartx->RX_TOC_DEST = count;  //rx timeout time = count * (1024/26M)
}

ATTR_TEXT_IN_TCM void uart_disable_customize_rx_timeout(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;
    uartx = g_uart_regbase[uart_port];

    uartx->RX_TO_CON_UNION.RX_TO_CON_CELLS.RX_TO_MODE = 0x0; //disable customize rx timeout mode
}

ATTR_TEXT_IN_TCM bool uart_get_tx_irq_status(hal_uart_port_t uart_port)
{
    uint32_t IIR;
    UART_REGISTER_T *uartx;
    bool dma_tx_irq_status = false;

    uartx = g_uart_regbase[uart_port];
    IIR = uartx->IIR;

    if(uart_port == HAL_UART_0) {
        if (UART_DMA_0_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(0)) {
            dma_tx_irq_status = true;
        }
    } else if(uart_port == HAL_UART_1) {
        if (UART_DMA_1_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(0)) {
            dma_tx_irq_status = true;
        }
    } else if(uart_port == HAL_UART_2) {
        if (UART_DMA_2_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(0)) {
            dma_tx_irq_status = true;
        }
    }

    if((dma_tx_irq_status == true) || (IIR & UART_IIR_THRE))
    {
        return true;
    } else {
        return false;
    }
}


#if 0 /* debug dump RG */
void uart_debug_dump_rg(hal_uart_port_t uart_port)
{
    log_hal_msgid_warning("========dump uart %d register=======\r\n", 1, uart_port);
    if (uart_port == g_uart_port_for_logging) {
        /*print share buffer*/
        log_hal_msgid_warning("========logging port is uart %d=======\r\n", 1, uart_port);
    }
    /* dump uart register*/
    UART_REGISTER_T    *uartx = g_uart_regbase[uart_port];
    log_hal_msgid_warning("uart%d [00]>:  xxxxxxxx  xxxxxxxx  %08x  %08x\r\n", 3, uart_port, uartx->DLM_DLL, uartx->IER_UNION.IER);
    log_hal_msgid_warning("uart%d [10]>:  xxxxxxxx  %08x  %08x  %08x\r\n", 4, uart_port, uartx->FCR_UNION.FCR, uartx->EFR_UNION.EFR, uartx->LCR_UNION.LCR);
    log_hal_msgid_warning("uart%d [20]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, uartx->MCR_UNION.MCR, uartx->XON_XOFF_UNION.XON_XOFF, uartx->LSR, uartx->SCR);
    log_hal_msgid_warning("uart%d [30]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, uartx->AUTOBAUD_CON_UNION.AUTOBAUD_CON, uartx->HIGHSPEED, uartx->SAMPLE_REG_UNION.SAMPLE_REG, uartx->AUTOBAUD_REG_UNION.AUTOBAUD_REG);
    log_hal_msgid_warning("uart%d [40]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, uartx->RATEFIX_UNION.RATEFIX, uartx->GUARD, uartx->ESCAPE_REG_UNION.ESCAPE_REG, uartx->SLEEP_REG);
    log_hal_msgid_warning("uart%d [50]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, uartx->DMA_CON_UNION.DMA_CON, uartx->RXTRIG, uartx->FRACDIV, uartx->RX_TO_CON_UNION.RX_TO_CON);
    log_hal_msgid_warning("uart%d [60]>:  %08x  \r\n", 2, uart_port, uartx->RX_TOC_DEST);
    /*dump DMA register*/
    /*if DMA mode enable*/
    uint32_t i;
    if (uartx->DMA_CON_UNION.DMA_CON & 0xFFFFFFFF) {
        for (i = 0; i < 2; i++) {
            VDMA_REGISTER_T *dma_x = g_vdma_regbase[uart_port][i];
            dma_x->VDMA_COUNT = dma_x->VDMA_COUNT;
            if (i == 0) {
                log_hal_msgid_warning("vdma%d-tx [10]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, dma_x->VDMA_COUNT,      dma_x->VDMA_CON_UNION.VDMA_CON, dma_x->VDMA_START, dma_x->VDMA_INTSTA);
                log_hal_msgid_warning("vdma%d-tx [20]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, dma_x->VDMA_ACKINT,     dma_x->DUMMY1_OFFSET, dma_x->VDMA_LIMITER, dma_x->VDMA_PGMADDR);
                log_hal_msgid_warning("vdma%d-tx [30]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, dma_x->VDMA_WRPTR,      dma_x->VDMA_RDPTR, dma_x->VDMA_FFCNT, dma_x->VDMA_FFSTA);
                log_hal_msgid_warning("vdma%d-tx [40]>:  %08x  %08x  xxxxxxxx  xxxxxxxx\r\n", 3, uart_port, dma_x->VDMA_ALTLEN,     dma_x->VDMA_FFSIZE);
                log_hal_msgid_warning("vdma%d-tx [60]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, dma_x->VDMA_SW_MV_BYTE, dma_x->VDMA_BNDRY_ADDR, dma_x->VDMA_BYTE_TO_BNDRY, dma_x->VDMA_BYTE_AVAIL);
            } else {
                log_hal_msgid_warning("vdma%d-rx [10]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, dma_x->VDMA_COUNT,      dma_x->VDMA_CON_UNION.VDMA_CON, dma_x->VDMA_START, dma_x->VDMA_INTSTA);
                log_hal_msgid_warning("vdma%d-rx [20]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, dma_x->VDMA_ACKINT,     dma_x->DUMMY1_OFFSET, dma_x->VDMA_LIMITER, dma_x->VDMA_PGMADDR);
                log_hal_msgid_warning("vdma%d-rx [30]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, dma_x->VDMA_WRPTR,      dma_x->VDMA_RDPTR, dma_x->VDMA_FFCNT, dma_x->VDMA_FFSTA);
                log_hal_msgid_warning("vdma%d-rx [40]>:  %08x  %08x  xxxxxxxx  xxxxxxxx\r\n", 3, uart_port, dma_x->VDMA_ALTLEN,     dma_x->VDMA_FFSIZE);
                log_hal_msgid_warning("vdma%d-rx [60]>:  %08x  %08x  %08x  %08x\r\n", 5, uart_port, dma_x->VDMA_SW_MV_BYTE, dma_x->VDMA_BNDRY_ADDR, dma_x->VDMA_BYTE_TO_BNDRY, dma_x->VDMA_BYTE_AVAIL);
            }
        }
    } else {
        log_hal_msgid_warning("UART %d is not DMA mode\r\n", 1, uart_port);
    }
#if 0
    uint32_t *mux_mem_0 = (uint32_t *)0x0425AC64;
    uint32_t *mux_mem_1 = (uint32_t *)0x0425ACB4;
    uint32_t *mux_mem_2 = (uint32_t *)0x0425AD04;
    uint32_t *mux_mem_ary[HAL_UART_MAX] = {mux_mem_0, mux_mem_1, mux_mem_2};
    mux_mem_ary[0] = mux_mem_ary[0];
    //log_hal_msgid_warning("mux share memory1 %d,%08x,%08x,%08x,%08x\r\n",5,uart_port,*(mux_mem_ary[2]++),*(mux_mem_ary[2]++),*(mux_mem_ary[2]++),*(mux_mem_ary[2]++));
    for (i = 0; i < 5; i++) {
        log_hal_msgid_warning("mux sh-mem %d,%08x,%08x,%08x,%08x\r\n", 5, uart_port, *(mux_mem_ary[uart_port]++), *(mux_mem_ary[uart_port]++), *(mux_mem_ary[uart_port]++), *(mux_mem_ary[uart_port]++));
    }
#endif
}
#endif /* debug dump RG */

#ifdef __cplusplus
}
#endif

#endif /*HAL_UART_MODULE_ENABLED*/


