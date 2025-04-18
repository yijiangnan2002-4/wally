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
#ifdef HAL_UART_MODULE_ENABLED
#include "hal_uart_internal.h"
#include "hal_resource_assignment.h"
#include "hal_nvic.h"
#include "hal_clock.h"

#ifdef __cplusplus
extern "C" {
#endif

UART_REGISTER_T *const g_uart_regbase[] = {UART0, UART1, UART2};
const hal_clock_cg_id g_uart_port_to_pdn[] = {HAL_CLOCK_CG_UART0, HAL_CLOCK_CG_UART1, HAL_CLOCK_CG_UART2};
const hal_nvic_irq_t g_uart_port_to_irq_num[] = {UART0_IRQn, UART1_IRQn, UART2_IRQn};
bool volatile g_uart_receive_break_single_status[HAL_UART_MAX] = {false, false, false};
// bool volatile sleep_rx_timeout_flag[HAL_UART_MAX] = {false, false, false};

void uart_dma_channel_to_callback_data(vdma_channel_t dma_channel, uart_dma_callback_data_t *user_data)
{
    switch (dma_channel) {
        case VDMA_UART0RX:
            user_data->is_rx = true;
            user_data->uart_port = HAL_UART_0;
            break;
        case VDMA_UART1RX:
            user_data->is_rx = true;
            user_data->uart_port = HAL_UART_1;
            break;
        case VDMA_UART2RX:
            user_data->is_rx = true;
            user_data->uart_port = HAL_UART_2;
            break;
        case VDMA_UART0TX:
            user_data->is_rx = false;
            user_data->uart_port = HAL_UART_0;
            break;
        case VDMA_UART1TX:
            user_data->is_rx = false;
            user_data->uart_port = HAL_UART_1;
            break;
        case VDMA_UART2TX:
            user_data->is_rx = false;
            user_data->uart_port = HAL_UART_2;
            break;
        default:
            break;
    }
}

void uart_enable_dma(UART_REGISTER_T *uartx)
{
    uartx->DMA_CON_UNION.DMA_CON_CELLS.TX_DMA_EN = UART_DMA_CON_TX_DMA_EN_MASK;
    uartx->DMA_CON_UNION.DMA_CON_CELLS.RX_DMA_EN = UART_DMA_CON_RX_DMA_EN_MASK;
}

void uart_interrupt_handler(hal_nvic_irq_t irq_number)
{
    hal_uart_port_t uart_port;
    uart_interrupt_type_t type;
    UART_REGISTER_T *uartx = UART0;

    switch (irq_number) {
        case UART0_IRQn:
            uart_port = HAL_UART_0;
            break;
        case UART1_IRQn:
            uart_port = HAL_UART_1;
            break;
        case UART2_IRQn:
            uart_port = HAL_UART_2;
            break;
        default:
            uart_port = HAL_UART_0;
    }

    uartx = g_uart_regbase[uart_port];
    type = uart_query_interrupt_type(uartx);

    switch (type) {
        /* received data and timeout happen */
        case UART_INTERRUPT_RECEIVE_TIMEOUT:
            uart_receive_handler(uart_port, true);
            break;
        /* receive line status changed Any of BI/FE/PE/OE becomes set */
        case UART_INTERRUPT_RECEIVE_ERROR:
            uart_error_handler(uart_port);
            break;
        /* received data or received Trigger level reached */
        case UART_INTERRUPT_RECEIVE_AVAILABLE:
            //uart_receive_handler(uart_port, false);
            break;
        /* false interrupt detect */
        case UART_INTERRUPT_NONE:
            // if(sleep_rx_timeout_flag[uart_port] == true) {  //After entering sleep, RX timeout IRQ is cleared
            //     uart_receive_handler(uart_port, true);
            //     sleep_rx_timeout_flag[uart_port] = false;
            // }
            break;
        /* received break signal */
        case UART_INTERRUPT_RECEIVE_BREAK:
            g_uart_receive_break_single_status[uart_port] = true;
            uart_error_handler(uart_port);
            // uart_purge_fifo(uartx, 1);
            break;
        /* TX holding register is empty or the TX FIFO reduce to it's trigger level */
        case UART_INTERRUPT_SEND_AVAILABLE:
            uart_send_handler(uart_port, true);
            uart_transaction_done_handler(uart_port);
            break;
        /* detect hardware flow control request (CTS is high) */
        case UART_INTERRUPT_HARDWARE_FLOWCONTROL:
#ifdef HAL_UART_FEATURE_FLOWCONTROL_CALLBACK
            uart_hw_flowcontrol_handler(uart_port);
#endif
            break;
        /* an XOFF character has been received */
        case UART_INTERRUPT_SOFTWARE_FLOWCONTROL:
#ifdef HAL_UART_FEATURE_FLOWCONTROL_CALLBACK
            uart_sw_flowcontrol_handler(uart_port);
#endif
            break;
        default:
            UART_ASSERT();
    }
}


void uart_set_baudrate(UART_REGISTER_T *uartx, uint32_t actual_baudrate)
{
    uint32_t uart_clock, integer, remainder, fraction;
    uint32_t dll_dlm, sample_count, sample_point;
    uint32_t fraction_mapping[] = {0x00, 0x00, 0x20, 0x90, 0xa8, 0x154, 0x16c, 0x1ba, 0x1f6, 0x1fe};

    uartx->RATEFIX_UNION.RATEFIX = 0x0;
    uartx->FRACDIV = (~UART_FRACDIV_MASK);

    uart_clock = UART_INPUT_CLOCK_DCXO;

    integer = uart_clock / (actual_baudrate * 256);
    remainder = ((uart_clock * 10) / (actual_baudrate * 256)) % 10;
    if ((remainder != 0) || (integer == 0)) {
        integer += 1;
    }
    dll_dlm = integer;
    sample_count = uart_clock / (actual_baudrate * dll_dlm);

    while (sample_count > 256) {
        dll_dlm++;
        sample_count = uart_clock / (actual_baudrate * dll_dlm);
    }

    fraction = ((uart_clock * 10) / (actual_baudrate * dll_dlm)) % 10;

    sample_count -= 1;
    sample_point = (sample_count - 1) >> 1;

    uartx->HIGHSPEED = UART_HIGHSPEED_SPEED_MODE3;

    uartx->DLM_DLL = dll_dlm & 0x0000ffff;

    uartx->SAMPLE_REG_UNION.SAMPLE_REG = (sample_point << UART_SAMPLE_REG_SAMPLE_POINT_OFFSET) | sample_count;

    uartx->FRACDIV = fraction_mapping[fraction];

    if ((actual_baudrate >= 3000000) && (actual_baudrate != 8666000)) {
        uartx->GUARD = 0x12;    /* delay 2 bits per byte. */
    }

    if (actual_baudrate == 8666000) {
        uartx->GUARD = 0x11;    /* delay 1 bits per byte. */
    }

}

void uart_set_format(UART_REGISTER_T *uartx,
                     hal_uart_word_length_t word_length,
                     hal_uart_stop_bit_t stop_bit,
                     hal_uart_parity_t parity)
{
    uint8_t byte = 0;

    /* Setup wordlength */
    switch (word_length) {
        case HAL_UART_WORD_LENGTH_5:
            byte |= UART_LCR_WORD_5BITS;
            break;
        case HAL_UART_WORD_LENGTH_6:
            byte |= UART_LCR_WORD_6BITS;
            break;
        case HAL_UART_WORD_LENGTH_7:
            byte |= UART_LCR_WORD_7BITS;
            break;
        case HAL_UART_WORD_LENGTH_8:
            byte |= UART_LCR_WORD_8BITS;
            break;
        default:
            byte |= UART_LCR_WORD_8BITS;
            break;
    }

    /* setup stop bit */
    switch (stop_bit) {
        case HAL_UART_STOP_BIT_1:
            byte |= UART_LCR_STB_1;
            break;
        case HAL_UART_STOP_BIT_2:
            byte |= UART_LCR_STB_2;
            break;
        default:
            byte |= UART_LCR_STB_1;
            break;
    }

    /* setup parity bit */
    switch (parity) {
        case HAL_UART_PARITY_NONE:
            byte |= UART_LCR_PAR_NONE;
            break;
        case HAL_UART_PARITY_ODD:
            byte |= UART_LCR_PAR_ODD;
            break;
        case HAL_UART_PARITY_EVEN:
            byte |= UART_LCR_PAR_EVEN;
            break;
        default:
            byte |= UART_LCR_PAR_NONE;
            break;
    }

    /* DLAB End */
    uartx->LCR_UNION.LCR_CELLS.PAR_STB_WLS = byte;
}

void uart_put_char_block(UART_REGISTER_T *uartx, uint8_t byte)
{
    uint32_t LSR;

    while (1) {
        LSR = uartx->LSR;
        if (LSR & UART_LSR_THRE_MASK) {
            uartx->THR = byte;
            break;
        }
    }
}

int uart_probe_char(UART_REGISTER_T *uartx)
{
    if (uartx->LSR & UART_LSR_DR_MASK) {
        return (int)uartx->RBR;
    }

    return -1;
}

uint8_t uart_get_char_block(UART_REGISTER_T *uartx)
{
    uint32_t LSR;
    uint8_t byte;

    while (1) {
        LSR = uartx->LSR;
        if (LSR & UART_LSR_DR_MASK) {
            byte = (uint8_t)uartx->RBR;
            break;
        }
    }

    return byte;
}

uint32_t uart_get_char_unblocking(UART_REGISTER_T *uartx)
{
    uint32_t LSR;
    uint32_t value;

    LSR = uartx->LSR;
    if (LSR & UART_LSR_DR_MASK) {
        value = uartx->RBR;
    } else {
        value = 0xffffffff;
    }

    return value;
}

void uart_set_hardware_flowcontrol(UART_REGISTER_T *uartx)
{
    uartx->ESCAPE_REG_UNION.ESCAPE_REG = 0;
    uartx->MCR_UNION.MCR_CELLS.RTS = UART_MCR_RTS_MASK;
    uartx->EFR_UNION.EFR_CELLS.HW_FLOW_CONT = (UART_EFR_HW_TX_FLOWCTRL_MASK |
                                               UART_EFR_HW_RX_FLOWCTRL_MASK);

#ifdef HAL_UART_FEATURE_FLOWCONTROL_CALLBACK
    uartx->IER_UNION.IER_CELLS.CTSI_RTSI = UART_IER_RTSI_MASK | UART_IER_CTSI_MASK;
#endif
}

void uart_set_software_flowcontrol(UART_REGISTER_T *uartx,
                                   uint8_t xon,
                                   uint8_t xoff,
                                   uint8_t escape_character)
{
    uartx->XON_XOFF_UNION.XON_XOFF = (xon << UART_XON_XOFF_XONCHAR_OFFSET) |
                                     (xoff << UART_XON_XOFF_XOFFCHAR_OFFSET);
    uartx->ESCAPE_REG_UNION.ESCAPE_REG = (0x1 << UART_ESCAPE_REG_EN_OFFSET) |
                                         (escape_character << UART_ESCAPE_REG_CHAR_OFFSET);
    uartx->EFR_UNION.EFR_CELLS.SW_FLOW_CONT = (UART_EFR_SW_TX_FLOWCTRL_MASK |
                                               UART_EFR_SW_RX_FLOWCTRL_MASK);

#ifdef HAL_UART_FEATURE_FLOWCONTROL_CALLBACK
    uartx->IER_UNION.IER_CELLS.XOFFI = UART_IER_XOFFI_MASK;
#endif
}

void uart_disable_flowcontrol(UART_REGISTER_T *uartx)
{
    uartx->XON_XOFF_UNION.XON_XOFF = 0x0;
    uartx->ESCAPE_REG_UNION.ESCAPE_REG = 0x0;
    uartx->EFR_UNION.EFR_CELLS.SW_FLOW_CONT = 0x0;
    uartx->EFR_UNION.EFR_CELLS.HW_FLOW_CONT = 0x0;
    uartx->MCR_UNION.MCR_CELLS.RTS = 0x0;
#ifdef HAL_UART_FEATURE_FLOWCONTROL_CALLBACK
    uartx->IER_UNION.IER_CELLS.XOFFI = 0x00;
    uartx->IER_UNION.IER_CELLS.CTSI_RTSI = 0x00;
#endif
}

void uart_set_fifo(UART_REGISTER_T *uartx)
{
    uartx->FCR_UNION.FCR = UART_FCR_TXTRIG_1 |
                           UART_FCR_RXTRIG_12 |
                           UART_FCR_CLRT_MASK |
                           UART_FCR_CLRR_MASK |
                           UART_FCR_FIFOE_MASK;
}

#ifdef HAL_SLEEP_MANAGER_ENABLED
void uart_set_sleep_mode(UART_REGISTER_T *uartx)
{
    uartx->SLEEP_REG |= UART_SLEEP_EN_MASK;
}

void uart_set_sleep_idle_fc_mode(UART_REGISTER_T *uartx)
{
    uartx->SLEEP_REG |= UART_SLEEP_IDLE_FC_EN_MASK;
}
#endif

void uart_unmask_send_interrupt(UART_REGISTER_T *uartx)
{
    uartx->IER_UNION.IER_CELLS.ETBEI = UART_IER_ETBEI_MASK;
}

void uart_mask_send_interrupt(UART_REGISTER_T *uartx)
{
    uartx->IER_UNION.IER_CELLS.ETBEI = ~UART_IER_ETBEI_MASK;
}

void uart_unmask_receive_interrupt(UART_REGISTER_T *uartx)
{
    uartx->IER_UNION.IER_CELLS.ELSI_ERBFI = (UART_IER_ELSI_MASK | UART_IER_ERBFI_MASK);
}

void uart_mask_receive_interrupt(UART_REGISTER_T *uartx)
{
    uartx->IER_UNION.IER_CELLS.ELSI_ERBFI = ~(UART_IER_ELSI_MASK | UART_IER_ERBFI_MASK);
}

void uart_purge_fifo(UART_REGISTER_T *uartx, int32_t is_rx)
{
    uint32_t FCR = 0;

    FCR = UART_FCR_TXTRIG_1 | UART_FCR_RXTRIG_12 | UART_FCR_FIFOE_MASK;

    if (is_rx) {
        FCR |= UART_FCR_CLRR_MASK;
    } else {
        FCR |= UART_FCR_CLRT_MASK;
    }

    uartx->FCR_UNION.FCR = FCR;
}

uart_interrupt_type_t uart_query_interrupt_type(UART_REGISTER_T *uartx)
{
    uint32_t IIR, LSR;
    uart_interrupt_type_t type = UART_INTERRUPT_NONE;

    IIR = uartx->IIR;
#if 0
    if (IIR & UART_IIR_NONE) {
        return type;
    }
#endif
    switch (IIR & UART_IIR_ID_MASK) {
        /* received data and timeout happen */
        case UART_IIR_RDT:
            type = UART_INTERRUPT_RECEIVE_TIMEOUT;
            break;
        /* receive line status changed Any of BI/FE/PE/OE becomes set */
        case UART_IIR_LSR:
            LSR = uartx->LSR;
            if (LSR & UART_LSR_BI_MASK) {
                type = UART_INTERRUPT_RECEIVE_BREAK;
            } else {
                type = UART_INTERRUPT_RECEIVE_ERROR;
            }
            break;
        /* TX Holding Register Empty */
        case UART_IIR_THRE:
            type = UART_INTERRUPT_SEND_AVAILABLE;
            break;
        case UART_IIR_RCVXOFF:
            type = UART_INTERRUPT_SOFTWARE_FLOWCONTROL;
            break;
        case UART_IIR_CTSRTS:
            type = UART_INTERRUPT_HARDWARE_FLOWCONTROL;
            break;
        case UART_IIR_RDR:  /* RX Data Received */
            type = UART_INTERRUPT_RECEIVE_AVAILABLE;
            break;
        default:
            break;
    }

    return type;
}
/* if error happened, return 0*/
int32_t uart_verify_error(UART_REGISTER_T *uartx)
{
    uint32_t LSR;
    int32_t ret = 0;

    LSR = uartx->LSR;
    if (!(LSR & (UART_LSR_OE_MASK | UART_LSR_FE_MASK | UART_LSR_PE_MASK))) {
        ret = -1;
    }

    return ret;
}

void uart_clear_timeout_interrupt(UART_REGISTER_T *uartx)
{
    uint32_t DMA_CON;

    DMA_CON = uartx->DMA_CON_UNION.DMA_CON;
    DMA_CON = DMA_CON;
}

void uart_set_timeout_value(UART_REGISTER_T *uartx, uint32_t value)
{
    uint32_t ticks_per_ms;

    ticks_per_ms = 26 * 1000 / 1024; // critical: do not modify this value  <-.->

    /* If this value is set, the time of RX timeout is the set time instead of the time of 4 char characters.
       if parameter value is 1,which means timeout time is set to 1ms.
    */
    uartx->RX_TO_CON_UNION.RX_TO_CON_CELLS.RX_TO_MODE = 0x1;

    uartx->RX_TOC_DEST = (value * ticks_per_ms);
}

void uart_set_auto_baudrate(UART_REGISTER_T *uartx, bool is_enable, uint32_t current_baudrate)
{
    if (is_enable) {
        /* before enable HW auto baudrate, need configure HIGHSPEED = 0 */
        uartx->HIGHSPEED = 0x0;
        uartx->DLM_DLL = UART_INPUT_CLOCK_DCXO / current_baudrate / 16;
        /* enable auto baudrate detection */
        uartx->RATEFIX_UNION.RATEFIX_CELLS.AUTOBAUD_RATEFIX = 0x0;
        uartx->RATEFIX_UNION.RATEFIX_CELLS.AUTOBAUD_SAMPLE = 0xf;
        uartx->AUTOBAUD_CON_UNION.AUTOBAUD_CON_CELLS.AUTOBAUD_EN = 0x1;
        uartx->AUTOBAUD_CON_UNION.AUTOBAUD_CON_CELLS.AUTOBAUD_SEL = 0x0;
    } else {
        /* Auto baudrate is disabled, use HIGHSPEED = 3 feature for calibration */
        uart_set_baudrate(uartx, current_baudrate);
    }
}

void uart_reset_default_value(UART_REGISTER_T *uartx)
{
    uartx->LCR_UNION.LCR = 0x00;
    uartx->XON_XOFF_UNION.XON_XOFF = 0x00;
    uartx->DLM_DLL = 0x00;
    uartx->IER_UNION.IER = 0x00;
    uartx->FCR_UNION.FCR = 0x00;
    uartx->EFR_UNION.EFR = 0x00;
    uartx->MCR_UNION.MCR = 0x00;
    //uartx->MCR_UNION.MCR_CELLS.RTS = 0x00;
    //uartx->MCR_UNION.MCR_CELLS.LOOP = 0x00;
    uartx->SCR = 0x00;
    //uartx->AUTOBAUD_REG_UNION.AUTOBAUD_REG = 0x00;
    uartx->HIGHSPEED = 0x00;
    uartx->SAMPLE_REG_UNION.SAMPLE_REG = 0x00;
    uartx->RATEFIX_UNION.RATEFIX = 0x00;
    uartx->AUTOBAUD_CON_UNION.AUTOBAUD_CON = 0x00;
    uartx->GUARD = 0x00;
    uartx->ESCAPE_REG_UNION.ESCAPE_REG = 0x00;
    uartx->SLEEP_REG = 0x00;
    uartx->DMA_CON_UNION.DMA_CON = 0x00;
    uartx->RXTRIG = 0x00;
    uartx->FRACDIV = 0x00;
    uartx->RX_TOC_DEST = 0x00;
    uartx->RX_TO_CON_UNION.RX_TO_CON = 0x00;
}

bool uart_query_rx_empty(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;
    uartx = g_uart_regbase[uart_port];
    if ((uartx->LSR) & UART_LSR_DR_MASK) {
        return false;
    } else {
        return true;
    }
}

bool uart_query_tx_empty(hal_uart_port_t uart_port)
{
    UART_REGISTER_T *uartx;
    uartx = g_uart_regbase[uart_port];
    if ((uartx->LSR) & UART_LSR_TEMT_MASK) {
        return true;
    } else {
        return false;
    }
}

void uart_send_xon_xoff(hal_uart_port_t uart_port, bool snd_xon, bool snd_xoff)
{
    UART_REGISTER_T *uartx;
    uartx = g_uart_regbase[uart_port];

    if(snd_xon){
        uartx->EFR_UNION.EFR_CELLS.SW_FLOW_CONT = 0x01;  //disable tx sw flowcontrol
        uartx->EFR_UNION.EFR_CELLS.SEND_XON = UART_EFR_SEND_XON_MASK;
        uartx->EFR_UNION.EFR_CELLS.SW_FLOW_CONT = (UART_EFR_SW_TX_FLOWCTRL_MASK | UART_EFR_SW_RX_FLOWCTRL_MASK);
    } else if(snd_xoff){
        uartx->EFR_UNION.EFR_CELLS.SW_FLOW_CONT = 0x01;  //disable tx sw flowcontrol
        uartx->EFR_UNION.EFR_CELLS.SEND_XOFF = UART_EFR_SEND_XOFF_MASK;
        uartx->EFR_UNION.EFR_CELLS.SW_FLOW_CONT = (UART_EFR_SW_TX_FLOWCTRL_MASK | UART_EFR_SW_RX_FLOWCTRL_MASK);  //After enable tx flowcontrol, it will automatically send a xon.
    }
}

void uart_improve_dma_bus_priority(hal_uart_port_t uart_port)
{
    *(volatile uint32_t*)0x422D0004 |= (0x1  << (9 + uart_port));
}

#ifdef __cplusplus
}
#endif

#endif

