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

#include <string.h>
#include "memory_attribute.h"
#include "hal_resource_assignment.h"
#include "hal_pdma_internal.h"
#include "hal_nvic.h"
#include "hal_clock.h"
#ifdef HAL_CACHE_MODULE_ENABLED
#include "hal_cache.h"
#endif
#include "hal_log.h"

#if 0
#ifndef __UBL__
#include <assert.h>
#else
#define assert(expr) log_hal_error("assert\r\n")
#endif
#endif

#define assert(expr)

#ifndef NULL
#define NULL ((void *)(0))
#endif

// #define DUMP_DMA_REGISTER_ENABLE

/* Full DMA channel list table */
typedef enum {
    UART_DMA_CHANNEL_UART0_BEGIN,
    UART_DMA_CHANNEL_UART0_TX = UART_DMA_CHANNEL_UART0_BEGIN,
    UART_DMA_CHANNEL_UART0_RX,
    UART_DMA_CHANNEL_UART0_END = UART_DMA_CHANNEL_UART0_RX,

    UART_DMA_CHANNEL_UART1_BEGIN,
    UART_DMA_CHANNEL_UART1_TX = UART_DMA_CHANNEL_UART1_BEGIN,
    UART_DMA_CHANNEL_UART1_RX,
    UART_DMA_CHANNEL_UART1_END = UART_DMA_CHANNEL_UART1_RX,

    UART_DMA_CHANNEL_UART2_BEGIN,
    UART_DMA_CHANNEL_UART2_TX = UART_DMA_CHANNEL_UART2_BEGIN,
    UART_DMA_CHANNEL_UART2_RX,
    UART_DMA_CHANNEL_UART2_END = UART_DMA_CHANNEL_UART2_RX,

    I2C_DMA_CHANNEL_I2C0_BEGIN,
    I2C_DMA_CHANNEL_I2C0_TX = I2C_DMA_CHANNEL_I2C0_BEGIN,
    I2C_DMA_CHANNEL_I2C0_RX,
    I2C_DMA_CHANNEL_I2C0_END = I2C_DMA_CHANNEL_I2C0_RX,

    I2C_DMA_CHANNEL_I2C1_BEGIN,
    I2C_DMA_CHANNEL_I2C1_TX = I2C_DMA_CHANNEL_I2C1_BEGIN,
    I2C_DMA_CHANNEL_I2C1_RX,
    I2C_DMA_CHANNEL_I2C1_END = I2C_DMA_CHANNEL_I2C1_RX,

    I2C_DMA_CHANNEL_I2C2_BEGIN,
    I2C_DMA_CHANNEL_I2C2_TX = I2C_DMA_CHANNEL_I2C2_BEGIN,
    I2C_DMA_CHANNEL_I2C2_RX,
    I2C_DMA_CHANNEL_I2C2_END = I2C_DMA_CHANNEL_I2C2_RX,

    I3C_DMA_CHANNEL_I3C0_BEGIN,
    I3C_DMA_CHANNEL_I3C0_TX = I3C_DMA_CHANNEL_I3C0_BEGIN,
    I3C_DMA_CHANNEL_I3C0_RX,
    I3C_DMA_CHANNEL_I3C0_END = I3C_DMA_CHANNEL_I3C0_RX,

    I3C_DMA_CHANNEL_I3C1_BEGIN,
    I3C_DMA_CHANNEL_I3C1_TX = I3C_DMA_CHANNEL_I3C1_BEGIN,
    I3C_DMA_CHANNEL_I3C1_RX,
    I3C_DMA_CHANNEL_I3C1_END = I3C_DMA_CHANNEL_I3C1_RX,

    I2S_DMA_CHANNEL_BEGIN,
    I2S_DMA_CHANNEL_I2S0_TX = I2S_DMA_CHANNEL_BEGIN,
    I2S_DMA_CHANNEL_I2S0_RX,
    I2S_DMA_CHANNEL_I2S1_TX,
    I2S_DMA_CHANNEL_I2S1_RX,
    I2S_DMA_CHANNEL_I2S2_TX,
    I2S_DMA_CHANNEL_I2S2_RX,
    I2S_DMA_CHANNEL_I2S3_TX,
    I2S_DMA_CHANNEL_I2S3_RX,
    I2S_DMA_CHANNEL_I2S4_TX,
    I2S_DMA_CHANNEL_I2S4_RX,
    I2S_DMA_CHANNEL_I2S5_TX,
    I2S_DMA_CHANNEL_I2S5_RX,
    I2S_DMA_CHANNEL_I2S6_TX,
    I2S_DMA_CHANNEL_I2S6_RX,
    I2S_DMA_CHANNEL_I2S7_TX,
    I2S_DMA_CHANNEL_I2S7_RX,
    I2S_DMA_CHANNEL_END = I2S_DMA_CHANNEL_I2S7_RX,
} dma_channel_t;

static void uart_dma_interrupt_hander(hal_nvic_irq_t irq_number);
static void i2c_dma_interrupt_hander(hal_nvic_irq_t irq_number);
static void i3c_dma_interrupt_hander(hal_nvic_irq_t irq_number);
static void i2s_dma_interrupt_hander(hal_nvic_irq_t irq_number);

static hal_nvic_irq_t uart_dma_irq_number[] = {UART_DMA0_IRQn, UART_DMA1_IRQn, UART_DMA2_IRQn};
static hal_nvic_irq_t i2c_dma_irq_number[] = {I2C_DMA0_IRQn, I2C_DMA1_IRQn, I2C_DMA2_IRQn};
static hal_nvic_irq_t i3c_dma_irq_number[] = {I3C_DMA0_IRQn, I3C_DMA1_IRQn};

static void internal_dma_register_irq_callback(dma_channel_t dma_channel)
{
    uint32_t irq_status;
    uint32_t channel_offset;
    static bool uart_dma_register_done[3] = {false, false, false};
    static bool i2c_dma_register_done[3] = {false, false, false};
    static bool i3c_dma_register_done[2] = {false, false};
    static bool i2s_dma_register_done = false;

    hal_nvic_save_and_set_interrupt_mask(&irq_status);
    if (dma_channel <= UART_DMA_CHANNEL_UART2_END) {
        channel_offset = (dma_channel - UART_DMA_CHANNEL_UART0_BEGIN) / 2;
        if (uart_dma_register_done[channel_offset] == false) {
            hal_nvic_register_isr_handler(uart_dma_irq_number[channel_offset], uart_dma_interrupt_hander);
            hal_nvic_enable_irq(uart_dma_irq_number[channel_offset]);
            uart_dma_register_done[channel_offset] = true;
        }
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C0_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C2_END)) {
        channel_offset = (dma_channel - I2C_DMA_CHANNEL_I2C0_BEGIN) / 2;
        if (i2c_dma_register_done[channel_offset] == false) {
            hal_nvic_register_isr_handler(i2c_dma_irq_number[channel_offset], i2c_dma_interrupt_hander);
            hal_nvic_enable_irq(i2c_dma_irq_number[channel_offset]);
            i2c_dma_register_done[channel_offset] = true;
        }
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C0_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C1_END)) {
        channel_offset = (dma_channel - I3C_DMA_CHANNEL_I3C0_BEGIN) / 2;
        if (i3c_dma_register_done[channel_offset] == false) {
            hal_nvic_register_isr_handler(i3c_dma_irq_number[channel_offset], i3c_dma_interrupt_hander);
            hal_nvic_enable_irq(i3c_dma_irq_number[channel_offset]);
            i3c_dma_register_done[channel_offset] = true;
        }
    } else {
        if (i2s_dma_register_done == false) {
            hal_nvic_register_isr_handler(I2S_SLAVE_IRQn, i2s_dma_interrupt_hander);
            hal_nvic_enable_irq(I2S_SLAVE_IRQn);
            i2s_dma_register_done = true;
        }
    }
    hal_nvic_restore_interrupt_mask(irq_status);
}

static void internal_dma_enable_clock(dma_channel_t dma_channel)
{
    uint32_t domain_offset;
    static bool uart_dma_pdn_on[3] = {false, false, false};
    static bool i2c_dma_pdn_on[3] = {false, false, false};
    static bool i3c_dma_pdn_on[2] = {false, false};
    static bool i2s_dma_pdn_on = false;

    if (dma_channel <= UART_DMA_CHANNEL_UART0_END) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART0_BEGIN;
        UART_DMA_0_RG_GLB_CLK_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
        if (uart_dma_pdn_on[0] == false) {
#ifndef FPGA_ENV
            hal_clock_enable(HAL_CLOCK_CG_UART_DMA_0);
#endif
            uart_dma_pdn_on[0] = true;
        }
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART1_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART1_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART1_BEGIN;
        UART_DMA_1_RG_GLB_CLK_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
        if (uart_dma_pdn_on[1] == false) {
#ifndef FPGA_ENV
            hal_clock_enable(HAL_CLOCK_CG_UART_DMA_1);
#endif
            uart_dma_pdn_on[1] = true;
        }
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART2_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART2_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART2_BEGIN;
        UART_DMA_2_RG_GLB_CLK_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
        if (uart_dma_pdn_on[2] == false) {
#ifndef FPGA_ENV
            hal_clock_enable(HAL_CLOCK_CG_UART_DMA_2);
#endif
            uart_dma_pdn_on[2] = true;
        }
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C0_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C0_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C0_BEGIN;
        I2C_DMA_0_RG_GLB_CLK_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
        if (i2c_dma_pdn_on[0] == false) {
#ifndef FPGA_ENV
            hal_clock_enable(HAL_CLOCK_CG_I2C_DMA0);
#endif
            i2c_dma_pdn_on[0] = true;
        }
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C1_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C1_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C1_BEGIN;
        I2C_DMA_1_RG_GLB_CLK_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
        if (i2c_dma_pdn_on[1] == false) {
#ifndef FPGA_ENV
            hal_clock_enable(HAL_CLOCK_CG_I2C_DMA1);
#endif
            i2c_dma_pdn_on[1] = true;
        }
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C2_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C2_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C2_BEGIN;
        I2C_DMA_2_RG_GLB_CLK_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
        if (i2c_dma_pdn_on[2] == false) {
#ifndef FPGA_ENV
            hal_clock_enable(HAL_CLOCK_CG_I2C_DMA2);
#endif
            i2c_dma_pdn_on[2] = true;
        }
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C0_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C0_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C0_BEGIN;
        I3C_DMA_0_RG_GLB_CLK_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
        if (i3c_dma_pdn_on[0] == false) {
#ifndef FPGA_ENV
            hal_clock_enable(HAL_CLOCK_CG_I3C_DMA0);
#endif
            i3c_dma_pdn_on[0] = true;
        }
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C1_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C1_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C1_BEGIN;
        I3C_DMA_1_RG_GLB_CLK_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
        if (i3c_dma_pdn_on[1] == false) {
#ifndef FPGA_ENV
            hal_clock_enable(HAL_CLOCK_CG_I3C_DMA1);
#endif
            i3c_dma_pdn_on[1] = true;
        }
    } else {
        domain_offset = dma_channel - I2S_DMA_CHANNEL_BEGIN;
        I2S_DMA_RG_GLB_CLK_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
        if (i2s_dma_pdn_on == false) {
            i2s_dma_pdn_on = true;
        }
    }
}

static void internal_dma_disable_clock(dma_channel_t dma_channel)
{
    uint32_t domain_offset;

    if (dma_channel <= UART_DMA_CHANNEL_UART0_END) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART0_BEGIN;
        UART_DMA_0_RG_GLB_CLK_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART1_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART1_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART1_BEGIN;
        UART_DMA_1_RG_GLB_CLK_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART2_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART2_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART2_BEGIN;
        UART_DMA_2_RG_GLB_CLK_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C0_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C0_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C0_BEGIN;
        I2C_DMA_0_RG_GLB_CLK_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C1_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C1_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C1_BEGIN;
        I2C_DMA_1_RG_GLB_CLK_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C2_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C2_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C2_BEGIN;
        I2C_DMA_2_RG_GLB_CLK_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C0_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C0_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C0_BEGIN;
        I3C_DMA_0_RG_GLB_CLK_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C1_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C1_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C1_BEGIN;
        I3C_DMA_1_RG_GLB_CLK_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else {
        domain_offset = dma_channel - I2S_DMA_CHANNEL_BEGIN;
        I2S_DMA_RG_GLB_CLK_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    }
}

static void internal_dma_enable_irq(dma_channel_t dma_channel)
{
    uint32_t domain_offset;

    if (dma_channel <= UART_DMA_CHANNEL_UART0_END) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART0_BEGIN;
        UART_DMA_0_RG_GLB_CPU0_INT_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART1_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART1_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART1_BEGIN;
        UART_DMA_1_RG_GLB_CPU0_INT_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART2_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART2_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART2_BEGIN;
        UART_DMA_2_RG_GLB_CPU0_INT_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C0_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C0_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C0_BEGIN;
        I2C_DMA_0_RG_GLB_CPU0_INT_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C1_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C1_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C1_BEGIN;
        I2C_DMA_1_RG_GLB_CPU0_INT_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C2_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C2_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C2_BEGIN;
        I2C_DMA_2_RG_GLB_CPU0_INT_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C0_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C0_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C0_BEGIN;
        I3C_DMA_0_RG_GLB_CPU0_INT_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C1_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C1_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C1_BEGIN;
        I3C_DMA_1_RG_GLB_CPU0_INT_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else {
        domain_offset = dma_channel - I2S_DMA_CHANNEL_BEGIN;
        I2S_DMA_RG_GLB_CPU0_INT_SET = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    }
}

static void internal_dma_disable_irq(dma_channel_t dma_channel)
{
    uint32_t domain_offset;

    if (dma_channel <= UART_DMA_CHANNEL_UART0_END) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART0_BEGIN;
        UART_DMA_0_RG_GLB_CPU0_INT_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART1_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART1_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART1_BEGIN;
        UART_DMA_1_RG_GLB_CPU0_INT_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART2_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART2_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART2_BEGIN;
        UART_DMA_2_RG_GLB_CPU0_INT_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C0_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C0_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C0_BEGIN;
        I2C_DMA_0_RG_GLB_CPU0_INT_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C1_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C1_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C1_BEGIN;
        I2C_DMA_1_RG_GLB_CPU0_INT_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C2_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C2_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C2_BEGIN;
        I2C_DMA_2_RG_GLB_CPU0_INT_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C0_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C0_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C0_BEGIN;
        I3C_DMA_0_RG_GLB_CPU0_INT_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C1_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C1_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C1_BEGIN;
        I3C_DMA_1_RG_GLB_CPU0_INT_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    } else {
        domain_offset = dma_channel - I2S_DMA_CHANNEL_BEGIN;
        I2S_DMA_RG_GLB_CPU0_INT_CLR = (DMA_GLB_IRQ_CFG_BIT_MASK(domain_offset));
    }
}

static bool internal_dma_check_idle_status(dma_channel_t dma_channel)
{
    uint32_t domain_offset;

    if (dma_channel <= UART_DMA_CHANNEL_UART0_END) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART0_BEGIN;
        if ((UART_DMA_0_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) ||
            (UART_DMA_0_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(domain_offset))) {
            return false;
        }
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART1_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART1_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART1_BEGIN;
        if ((UART_DMA_1_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) ||
            (UART_DMA_1_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(domain_offset))) {
            return false;
        }
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART2_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART2_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART2_BEGIN;
        if ((UART_DMA_2_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) ||
            (UART_DMA_2_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(domain_offset))) {
            return false;
        }
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C0_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C0_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C0_BEGIN;
        if ((I2C_DMA_0_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) ||
            (I2C_DMA_0_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(domain_offset))) {
            return false;
        }
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C1_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C1_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C1_BEGIN;
        if ((I2C_DMA_1_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) ||
            (I2C_DMA_1_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(domain_offset))) {
            return false;
        }
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C2_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C2_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C2_BEGIN;
        if ((I2C_DMA_2_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) ||
            (I2C_DMA_2_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(domain_offset))) {
            return false;
        }
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C0_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C0_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C0_BEGIN;
        if ((I3C_DMA_0_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) ||
            (I3C_DMA_0_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(domain_offset))) {
            return false;
        }
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C1_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C1_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C1_BEGIN;
        if ((I3C_DMA_1_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) ||
            (I3C_DMA_1_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(domain_offset))) {
            return false;
        }
    } else {
        domain_offset = dma_channel - I2S_DMA_CHANNEL_BEGIN;
        if ((I2S_DMA_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) ||
            (I2S_DMA_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(domain_offset))) {
            return false;
        }
    }

    return true;
}

static bool internal_dma_check_running_status(dma_channel_t dma_channel)
{
    uint32_t domain_offset;

    if (dma_channel <= UART_DMA_CHANNEL_UART0_END) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART0_BEGIN;
        if (UART_DMA_0_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) {
            return true;
        }
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART1_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART1_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART1_BEGIN;
        if (UART_DMA_1_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) {
            return true;
        }
    } else if ((dma_channel >= UART_DMA_CHANNEL_UART2_BEGIN) && (dma_channel <= UART_DMA_CHANNEL_UART2_END)) {
        domain_offset = dma_channel - UART_DMA_CHANNEL_UART2_BEGIN;
        if (UART_DMA_2_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) {
            return true;
        }
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C0_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C0_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C0_BEGIN;
        if (I2C_DMA_0_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) {
            return true;
        }
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C1_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C1_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C1_BEGIN;
        if (I2C_DMA_1_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) {
            return true;
        }
    } else if ((dma_channel >= I2C_DMA_CHANNEL_I2C2_BEGIN) && (dma_channel <= I2C_DMA_CHANNEL_I2C2_END)) {
        domain_offset = dma_channel - I2C_DMA_CHANNEL_I2C2_BEGIN;
        if (I2C_DMA_2_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) {
            return true;
        }
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C0_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C0_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C0_BEGIN;
        if (I3C_DMA_0_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) {
            return true;
        }
    } else if ((dma_channel >= I3C_DMA_CHANNEL_I3C1_BEGIN) && (dma_channel <= I3C_DMA_CHANNEL_I3C1_END)) {
        domain_offset = dma_channel - I3C_DMA_CHANNEL_I3C1_BEGIN;
        if (I3C_DMA_1_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) {
            return true;
        }
    } else {
        domain_offset = dma_channel - I2S_DMA_CHANNEL_BEGIN;
        if (I2S_DMA_RG_GLB_STA & DMA_GLB_RUNNING_BIT_MASK(domain_offset)) {
            return true;
        }
    }

    return false;
}

/*************************************************************************************************************/
/************************************peripheral  dma function start  line********************************************/
/*************************************************************************************************************/

#define PDMA_INIT    1
#define PDMA_DEINIT  0

#define PDMA_MAX_WPPT 0xFFFF

/* Map table for pdma_channel_t -> dma_channel_t */
static dma_channel_t g_pdma_channel_map_table[] = {
    I2C_DMA_CHANNEL_I2C0_TX,
    I2C_DMA_CHANNEL_I2C0_RX,
    I2C_DMA_CHANNEL_I2C1_TX,
    I2C_DMA_CHANNEL_I2C1_RX,
    I2C_DMA_CHANNEL_I2C2_TX,
    I2C_DMA_CHANNEL_I2C2_RX,
    I3C_DMA_CHANNEL_I3C0_TX,
    I3C_DMA_CHANNEL_I3C0_RX,
    I3C_DMA_CHANNEL_I3C1_TX,
    I3C_DMA_CHANNEL_I3C1_RX,
};

/* PDMA channel base address list */
static PDMA_REGISTER_T *g_pdma_regbase_table[] = {
    PDMA_RG_I2C0_TX_BASE,
    PDMA_RG_I2C0_RX_BASE,
    PDMA_RG_I2C1_TX_BASE,
    PDMA_RG_I2C1_RX_BASE,
    PDMA_RG_I2C2_TX_BASE,
    PDMA_RG_I2C2_RX_BASE,
    PDMA_RG_I3C0_TX_BASE,
    PDMA_RG_I3C0_RX_BASE,
    PDMA_RG_I3C1_TX_BASE,
    PDMA_RG_I3C1_RX_BASE,
};

typedef struct {
    pdma_callback_t func;
    void *argument;
} pdma_user_callback_t;

static pdma_user_callback_t g_pdma_callback[PDMA_END_CHANNEL];
static volatile uint8_t g_pdma_init_status[PDMA_END_CHANNEL];

static void internal_pdma_restore_default_value(pdma_channel_t channel)
{
    PDMA_REGISTER_T *pdma;

    pdma = g_pdma_regbase_table[channel];
    pdma->PDMA_START = 0;
    pdma->PDMA_WPPT = 0;
    pdma->PDMA_WPTO = 0;
    pdma->PDMA_COUNT = 0;
    pdma->PDMA_CON_UNION.PDMA_CON = 0;
    pdma->PDMA_LIMITER = 0;
    pdma->PDMA_PGMADDR = 0;
    pdma->PDMA_ACKINT = 0x8000;
}

pdma_status_t pdma_init(pdma_channel_t channel)
{
    uint32_t saved_mask;

    if (channel >= PDMA_END_CHANNEL) {
        return PDMA_ERROR_CHANNEL;
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (g_pdma_init_status[channel] == PDMA_INIT) {
        hal_nvic_restore_interrupt_mask(saved_mask);
        return PDMA_ERROR;
    }
    g_pdma_init_status[channel] = PDMA_INIT;
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (internal_dma_check_idle_status(g_pdma_channel_map_table[channel]) == false) {
        assert(0);
        return PDMA_ERROR;
    }

    internal_dma_enable_clock(g_pdma_channel_map_table[channel]);
    internal_pdma_restore_default_value(channel);

    return PDMA_OK;
}

pdma_status_t pdma_deinit(pdma_channel_t channel)
{
    uint32_t saved_mask;

    if (channel >= PDMA_END_CHANNEL) {
        return PDMA_ERROR_CHANNEL;
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (g_pdma_init_status[channel] == PDMA_DEINIT) {
        hal_nvic_restore_interrupt_mask(saved_mask);
        return PDMA_ERROR;
    }

    if (internal_dma_check_idle_status(g_pdma_channel_map_table[channel]) == false) {
        assert(0);
        return PDMA_ERROR;
    }

    internal_pdma_restore_default_value(channel);
    internal_dma_disable_irq(g_pdma_channel_map_table[channel]);
    internal_dma_disable_clock(g_pdma_channel_map_table[channel]);

    g_pdma_init_status[channel] = PDMA_DEINIT;
    hal_nvic_restore_interrupt_mask(saved_mask);

    return PDMA_OK;
}

pdma_status_t pdma_set_ring_buffer(pdma_channel_t channel, uint32_t wppt, uint32_t wpto)
{
    PDMA_REGISTER_T *pdma;

    if (channel >= PDMA_END_CHANNEL) {
        return PDMA_ERROR_CHANNEL;
    }
    if (wppt > PDMA_MAX_WPPT) {
        return PDMA_INVALID_PARAMETER;
    }

    pdma = g_pdma_regbase_table[channel];
    pdma->PDMA_WPPT = wppt;
    pdma->PDMA_WPTO = wpto;

    return PDMA_OK;
}

pdma_status_t pdma_configure(pdma_channel_t channel, pdma_config_t *pdma_config)
{
    uint32_t dma_control;
    PDMA_REGISTER_T *pdma;

    if (channel >= PDMA_END_CHANNEL) {
        return PDMA_ERROR_CHANNEL;
    }
    if ((pdma_config == NULL) ||
        (pdma_config->master_type > PDMA_RX_RINGBUFF) ||
        (pdma_config->size > PDMA_WORD)) {
        return PDMA_INVALID_PARAMETER;
    }

    dma_control = 0;
    if (pdma_config->burst_mode == true) {
        dma_control = PDMA_CON_BURST_4_BEAT << PDMA_CON_BURST_OFFSET;
    } else {
        dma_control = PDMA_CON_BURST_SINGLE << PDMA_CON_BURST_OFFSET;
    }
    switch (pdma_config->master_type) {
        case PDMA_TX:
            dma_control |= PDMA_CON_SINC_MASK;
            break;
        case PDMA_RX:
            dma_control |= PDMA_CON_DIR_MASK;
            dma_control |= PDMA_CON_DINC_MASK;
            break;
        case PDMA_TX_RINGBUFF:
            dma_control |= PDMA_CON_SINC_MASK;
            dma_control |= PDMA_CON_WPEN_MASK;
            break;
        case PDMA_RX_RINGBUFF:
            dma_control |= PDMA_CON_DIR_MASK;
            dma_control |= PDMA_CON_SINC_MASK;
            dma_control |= PDMA_CON_WPSD_MASK;
            dma_control |= PDMA_CON_WPEN_MASK;
            break;
        default:
            dma_control |= PDMA_CON_SINC_MASK;
    }
    switch (pdma_config->size) {
        case PDMA_BYTE:
            dma_control |= PDMA_CON_SIZE_BYTE  << PDMA_CON_SIZE_OFFSET;
            break;
        case PDMA_HALF_WORD:
            dma_control |= PDMA_CON_SIZE_HALF_WORD  << PDMA_CON_SIZE_OFFSET;
            break;
        case PDMA_WORD:
            dma_control |= PDMA_CON_SIZE_WORD  << PDMA_CON_SIZE_OFFSET;
            break;
        default:
            dma_control |= PDMA_CON_SIZE_BYTE  << PDMA_CON_SIZE_OFFSET;
    }
    if ((pdma_config->burst_mode == true) && (pdma_config->size == PDMA_BYTE)) {
        dma_control |= PDMA_CON_B2W_MASK;
    }
    dma_control |= PDMA_CON_DREQ_MASK;

    pdma = g_pdma_regbase_table[channel];
    pdma->PDMA_START &= ~PDMA_START_BIT_MASK;
    pdma->PDMA_CON_UNION.PDMA_CON = dma_control;
    pdma->PDMA_COUNT = pdma_config->count;

    return PDMA_OK;
}

pdma_status_t pdma_start_polling(pdma_channel_t channel, uint32_t address)
{
    PDMA_REGISTER_T *pdma;
    pdma_running_status_t running_status;

    if (channel >= PDMA_END_CHANNEL) {
        return PDMA_ERROR_CHANNEL;
    }
    if ((address % 4) > 0) {
        assert(0);
        return PDMA_INVALID_PARAMETER;
    }
#ifdef HAL_CACHE_MODULE_ENABLED
    if (true == hal_cache_is_cacheable(address)) {
        assert(0);
        return PDMA_INVALID_PARAMETER;
    }
#endif

    pdma = g_pdma_regbase_table[channel];
    pdma->PDMA_PGMADDR = hal_memview_mcu_to_infrasys(address);
    pdma->PDMA_CON_UNION.PDMA_CON_CELLS.PDMA_ITEN = 0x0;
    pdma->PDMA_START = PDMA_START_BIT_MASK;

    do {
        pdma_get_running_status(channel, &running_status);
    } while (running_status);
    pdma_stop(channel);

    return PDMA_OK;
}

pdma_status_t pdma_start_interrupt(pdma_channel_t channel, uint32_t address)
{
    PDMA_REGISTER_T *pdma;

    if (channel >= PDMA_END_CHANNEL) {
        return PDMA_ERROR_CHANNEL;
    }
    if ((address % 4) > 0) {
        assert(0);
        return PDMA_INVALID_PARAMETER;
    }
#ifdef HAL_CACHE_MODULE_ENABLED
    if (true == hal_cache_is_cacheable(address)) {
        assert(0);
        return PDMA_INVALID_PARAMETER;
    }
#endif

    pdma = g_pdma_regbase_table[channel];
    pdma->PDMA_PGMADDR = hal_memview_mcu_to_infrasys(address);
    pdma->PDMA_CON_UNION.PDMA_CON_CELLS.PDMA_ITEN = 0x1;
    pdma->PDMA_START = PDMA_START_BIT_MASK;

    return PDMA_OK;
}

pdma_status_t pdma_stop(pdma_channel_t channel)
{
    PDMA_REGISTER_T *pdma;

    if (channel >= PDMA_END_CHANNEL) {
        return PDMA_ERROR_CHANNEL;
    }

    pdma = g_pdma_regbase_table[channel];
    pdma->PDMA_START &= ~PDMA_START_BIT_MASK;
    pdma->PDMA_ACKINT = PDMA_ACKINT_BIT_MASK;

    return PDMA_OK;
}

pdma_status_t pdma_register_callback(pdma_channel_t channel, pdma_callback_t callback, void *user_data)
{
    if (channel >= PDMA_END_CHANNEL) {
        return PDMA_ERROR_CHANNEL;
    }
    if (callback == NULL) {
        return PDMA_INVALID_PARAMETER;
    }

    g_pdma_callback[channel].func = callback;
    g_pdma_callback[channel].argument  = user_data;

    internal_dma_register_irq_callback(g_pdma_channel_map_table[channel]);
    internal_dma_enable_irq(g_pdma_channel_map_table[channel]);

    return PDMA_OK;
}

pdma_status_t pdma_get_running_status(pdma_channel_t channel, pdma_running_status_t *running_status)
{
    if (channel >= PDMA_END_CHANNEL) {
        return PDMA_ERROR_CHANNEL;
    }
    if (running_status == NULL) {
        return PDMA_INVALID_PARAMETER;
    }

    if (internal_dma_check_running_status(g_pdma_channel_map_table[channel]) == true) {
        *running_status = PDMA_BUSY;
    } else {
        *running_status = PDMA_IDLE;
    }

    return PDMA_OK;
}

#ifdef DUMP_DMA_REGISTER_ENABLE
pdma_status_t pdma_dump_reg_list(pdma_channel_t channel)
{
    PDMA_REGISTER_T *pdma;

    if (channel >= PDMA_END_CHANNEL) {
        return PDMA_ERROR_CHANNEL;
    }
    (void)pdma;
    pdma = g_pdma_regbase_table[channel];
    log_hal_msgid_info("PDMA channel %d reg list dump:", 1, channel);
    log_hal_msgid_info("PDMA_WPPT = 0x%08x", 1, pdma->PDMA_WPPT);
    log_hal_msgid_info("PDMA_WPTO = 0x%08x", 1, pdma->PDMA_WPTO);
    log_hal_msgid_info("PDMA_COUNT = 0x%08x", 1, pdma->PDMA_COUNT);
    log_hal_msgid_info("PDMA_CON = 0x%08x", 1, pdma->PDMA_CON_UNION.PDMA_CON);
    log_hal_msgid_info("PDMA_START = 0x%08x", 1, pdma->PDMA_START);
    log_hal_msgid_info("PDMA_INTSTA = 0x%08x", 1, pdma->PDMA_INTSTA);
    log_hal_msgid_info("PDMA_RLCT = 0x%08x", 1, pdma->PDMA_RLCT);
    log_hal_msgid_info("PDMA_LIMITER = 0x%08x", 1, pdma->PDMA_LIMITER);
    log_hal_msgid_info("PDMA_PGMADDR = 0x%08x", 1, pdma->PDMA_PGMADDR);

    return PDMA_OK;
}
#endif

/*************************************************************************************************************/
/************************************peripheral  dma function end line**********************************************/
/*************************************************************************************************************/



/*************************************************************************************************************/
/************************************virtual fifo dma function start line*********************************************/
/*************************************************************************************************************/

#define VDMA_INIT    1
#define VDMA_DEINIT  0

#define VDMA_MAX_THRESHOLD       0xFFFF
#define VDMA_MAX_ALERT_LENGTH    0x3F

/* Map table for vdma_channel_t -> dma_channel_t */
static dma_channel_t g_vdma_channel_map_table[] = {
    UART_DMA_CHANNEL_UART0_TX,
    UART_DMA_CHANNEL_UART0_RX,
    UART_DMA_CHANNEL_UART1_TX,
    UART_DMA_CHANNEL_UART1_RX,
    UART_DMA_CHANNEL_UART2_TX,
    UART_DMA_CHANNEL_UART2_RX,
    I2S_DMA_CHANNEL_I2S0_TX,
    I2S_DMA_CHANNEL_I2S0_RX,
    I2S_DMA_CHANNEL_I2S1_TX,
    I2S_DMA_CHANNEL_I2S1_RX,
    I2S_DMA_CHANNEL_I2S2_TX,
    I2S_DMA_CHANNEL_I2S2_RX,
    I2S_DMA_CHANNEL_I2S3_TX,
    I2S_DMA_CHANNEL_I2S3_RX,
    I2S_DMA_CHANNEL_I2S4_TX,
    I2S_DMA_CHANNEL_I2S4_RX,
    I2S_DMA_CHANNEL_I2S5_TX,
    I2S_DMA_CHANNEL_I2S5_RX,
    I2S_DMA_CHANNEL_I2S6_TX,
    I2S_DMA_CHANNEL_I2S6_RX,
    I2S_DMA_CHANNEL_I2S7_TX,
    I2S_DMA_CHANNEL_I2S7_RX,
};

/* VDMA channel base address list */
static VDMA_REGISTER_T *g_vdma_regbase_table[] = {
    VDMA_RG_UART0_TX_BASE,
    VDMA_RG_UART0_RX_BASE,
    VDMA_RG_UART1_TX_BASE,
    VDMA_RG_UART1_RX_BASE,
    VDMA_RG_UART2_TX_BASE,
    VDMA_RG_UART2_RX_BASE,
    VDMA_RG_I2S0_TX_BASE,
    VDMA_RG_I2S0_RX_BASE,
    VDMA_RG_I2S1_TX_BASE,
    VDMA_RG_I2S1_RX_BASE,
    VDMA_RG_I2S2_TX_BASE,
    VDMA_RG_I2S2_RX_BASE,
    VDMA_RG_I2S3_TX_BASE,
    VDMA_RG_I2S3_RX_BASE,
    VDMA_RG_I2S4_TX_BASE,
    VDMA_RG_I2S4_RX_BASE,
    VDMA_RG_I2S5_TX_BASE,
    VDMA_RG_I2S5_RX_BASE,
    VDMA_RG_I2S6_TX_BASE,
    VDMA_RG_I2S6_RX_BASE,
    VDMA_RG_I2S7_TX_BASE,
    VDMA_RG_I2S7_RX_BASE,
};

typedef struct {
    vdma_callback_t func;
    void *argument;
} vdma_user_callback_t;

static vdma_user_callback_t g_vdma_callback[VDMA_END_CHANNEL];
static volatile uint8_t g_vdma_init_status[VDMA_END_CHANNEL];

static void internal_vdma_restore_default_value(vdma_channel_t channel)
{
    VDMA_REGISTER_T *vdma;

    vdma = g_vdma_regbase_table[channel];
    vdma->VDMA_START = 0;
    vdma->VDMA_CON_UNION.VDMA_CON = 0;
    vdma->VDMA_COUNT = 0;
    vdma->VDMA_INTSTA = 0;
    vdma->VDMA_PGMADDR = 0;
    vdma->VDMA_ALTLEN = 0;
    vdma->VDMA_FFSIZE = 0;
    vdma->VDMA_LIMITER = 0;
    vdma->VDMA_ACKINT = 0x8000;
}

vdma_status_t vdma_init(vdma_channel_t channel)
{
    uint32_t saved_mask;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (g_vdma_init_status[channel] == VDMA_INIT) {
        hal_nvic_restore_interrupt_mask(saved_mask);
        return PDMA_ERROR;
    }
    g_vdma_init_status[channel] = VDMA_INIT;
    hal_nvic_restore_interrupt_mask(saved_mask);

    internal_dma_enable_clock(g_vdma_channel_map_table[channel]);
    //*(volatile uint32_t *)0xa30c0074 |= 0x0f;
    internal_vdma_restore_default_value(channel);

    vdma = g_vdma_regbase_table[channel];
    switch (channel) {
        case VDMA_UART0TX:
        case VDMA_UART1TX:
        case VDMA_UART2TX:
            vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SETTING = 0x1;
            vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SIZE = 0x0;
            break;
        case VDMA_UART0RX:
        case VDMA_UART1RX:
        case VDMA_UART2RX:
            vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_DIRECTION = 0x10;
            vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SETTING = 0x1;
            vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SIZE = 0x0;
            break;
        case VDMA_I2S0TX:
        case VDMA_I2S1TX:
        case VDMA_I2S2TX:
        case VDMA_I2S3TX:
        case VDMA_I2S4TX:
        case VDMA_I2S5TX:
        case VDMA_I2S6TX:
        case VDMA_I2S7TX:
            vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SETTING = 0x1;
            vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SIZE = 0x2;
            break;
        case VDMA_I2S0RX:
        case VDMA_I2S1RX:
        case VDMA_I2S2RX:
        case VDMA_I2S3RX:
        case VDMA_I2S4RX:
        case VDMA_I2S5RX:
        case VDMA_I2S6RX:
        case VDMA_I2S7RX:
            vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_DIRECTION = 0x10;
            vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SETTING = 0x1;
            vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SIZE = 0x2;
            break;
        default:
            return VDMA_ERROR;
    }

    return VDMA_OK;
}

vdma_status_t vdma_deinit(vdma_channel_t channel)
{
    uint32_t saved_mask;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (g_vdma_init_status[channel] == VDMA_DEINIT) {
        hal_nvic_restore_interrupt_mask(saved_mask);
        return VDMA_ERROR;
    }

    if (internal_dma_check_idle_status(g_vdma_channel_map_table[channel]) == false) {
        assert(0);
        return VDMA_ERROR;
    }

    internal_vdma_restore_default_value(channel);
    internal_dma_disable_irq(g_vdma_channel_map_table[channel]);
    internal_dma_disable_clock(g_vdma_channel_map_table[channel]);

    g_vdma_init_status[channel] = VDMA_DEINIT;
    hal_nvic_restore_interrupt_mask(saved_mask);

    return VDMA_OK;
}

vdma_status_t vdma_start(vdma_channel_t channel)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    internal_dma_check_idle_status(g_vdma_channel_map_table[channel]);

    vdma = g_vdma_regbase_table[channel];
    vdma->VDMA_START = VDMA_START_BIT_MASK;

    return VDMA_OK;
}

vdma_status_t vdma_stop(vdma_channel_t channel)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    vdma = g_vdma_regbase_table[channel];
    vdma->VDMA_START = 0 << VDMA_START_BIT_OFFSET;
    vdma->VDMA_SW_MV_BYTE = vdma->VDMA_FFCNT << vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SIZE;

    return VDMA_OK;
}

vdma_status_t vdma_configure(vdma_channel_t channel, vdma_config_t *vdma_config)
{
    VDMA_REGISTER_T *vdma;
    uint32_t base_address;

    if (vdma_config == NULL) {
        assert(0);
        return VDMA_INVALID_PARAMETER;
    }

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    base_address = vdma_config->base_address;
    if ((base_address % 4) > 0) {
        assert(0);
        return VDMA_INVALID_PARAMETER;
    }

#ifdef HAL_CACHE_MODULE_ENABLED
    if (true == hal_cache_is_cacheable(base_address)) {
        assert(0);
        return VDMA_INVALID_PARAMETER;
    }
#endif

    vdma = g_vdma_regbase_table[channel];
    vdma->VDMA_FFSIZE = vdma_config->size;
    vdma->VDMA_PGMADDR = hal_memview_mcu_to_infrasys(base_address);

    return VDMA_OK;
}

vdma_status_t vdma_enable_interrupt(vdma_channel_t channel)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    vdma = g_vdma_regbase_table[channel];
    vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_ITEN = 0x1;

    return VDMA_OK;
}

vdma_status_t vdma_disable_interrupt(vdma_channel_t channel)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    vdma = g_vdma_regbase_table[channel];
    vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_ITEN = 0x0;

    return VDMA_OK;
}

vdma_status_t vdma_get_interrupt_status(vdma_channel_t channel, uint8_t *is_enabled)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    if (is_enabled == NULL) {
        return VDMA_INVALID_PARAMETER;
    }

    vdma = g_vdma_regbase_table[channel];
    *is_enabled = vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_ITEN & 0x1;

    return VDMA_OK;
}

vdma_status_t vdma_set_threshold(vdma_channel_t channel, uint32_t threshold)
{
    uint8_t h_size = 0;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    vdma = g_vdma_regbase_table[channel];
    h_size = vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SIZE;
    threshold = threshold >> h_size;
    if (threshold > VDMA_MAX_THRESHOLD) {
        return VDMA_INVALID_PARAMETER;
    }
    vdma->VDMA_COUNT = threshold;

    return VDMA_OK;
}

vdma_status_t vdma_set_alert_length(vdma_channel_t channel, uint32_t alert_length)
{
    uint8_t h_size = 0;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    vdma = g_vdma_regbase_table[channel];
    h_size = vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SIZE;
    alert_length = alert_length >> h_size;
    if (alert_length > VDMA_MAX_ALERT_LENGTH) {
        return VDMA_INVALID_PARAMETER;
    }
    vdma->VDMA_ALTLEN = alert_length;

    return VDMA_OK;
}

vdma_status_t vdma_push_data(vdma_channel_t channel, uint8_t data)
{
    uint8_t *write_address;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    vdma = g_vdma_regbase_table[channel];
    write_address = (uint8_t *)vdma->VDMA_WRPTR;
    write_address = (uint8_t *)hal_memview_infrasys_to_mcu((uint32_t)((uint32_t *)write_address));
    *write_address = data;

    vdma_set_sw_move_byte(channel, 1);

    return VDMA_OK;
}

vdma_status_t vdma_push_data_4bytes(vdma_channel_t channel, uint32_t data)
{
    uint32_t *write_address;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    vdma = g_vdma_regbase_table[channel];
    write_address = (uint32_t *)vdma->VDMA_WRPTR;
    write_address = (uint32_t *)hal_memview_infrasys_to_mcu((uint32_t)((uint32_t *)write_address));
    *write_address = data;

    vdma_set_sw_move_byte(channel, 4);

    return VDMA_OK;
}

vdma_status_t vdma_push_data_multi_bytes(vdma_channel_t channel, uint8_t *data, uint32_t size)
{
    uint8_t *write_address;
    uint32_t byte_to_bndry;
    uint32_t byte_avail;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    if (data == NULL) {
        return VDMA_INVALID_PARAMETER;
    }
    vdma = g_vdma_regbase_table[channel];
    byte_avail = vdma->VDMA_BYTE_AVAIL;
    if (size > byte_avail) {
        return VDMA_INVALID_PARAMETER;
    }

    write_address = (uint8_t *)vdma->VDMA_WRPTR;
    write_address = (uint8_t *)hal_memview_infrasys_to_mcu((uint32_t)((uint32_t *)write_address));
    byte_to_bndry = vdma->VDMA_BYTE_TO_BNDRY;
    if (size > byte_to_bndry) {
        memcpy(write_address, data, byte_to_bndry);
        write_address = (uint8_t *)vdma->VDMA_PGMADDR;
        write_address = (uint8_t *)hal_memview_infrasys_to_mcu((uint32_t)((uint32_t *)write_address));
        memcpy(write_address, (data + byte_to_bndry), (size - byte_to_bndry));
    } else {
        memcpy(write_address, data, size);
    }
    vdma_set_sw_move_byte(channel, size);

    return VDMA_OK;
}

vdma_status_t vdma_pop_data(vdma_channel_t channel, uint8_t *data)
{
    uint8_t *read_address;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    if (data == NULL) {
        return VDMA_INVALID_PARAMETER;
    }

    vdma = g_vdma_regbase_table[channel];
    read_address = (uint8_t *)vdma->VDMA_RDPTR;
    read_address = (uint8_t *)hal_memview_infrasys_to_mcu((uint32_t)((uint32_t *)read_address));
    *data = *read_address ;

    vdma_set_sw_move_byte(channel, 1);

    return VDMA_OK;
}

vdma_status_t vdma_pop_data_4bytes(vdma_channel_t channel, uint32_t *data)
{
    uint32_t *read_address;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    if (data == NULL) {
        return VDMA_INVALID_PARAMETER;
    }

    vdma = g_vdma_regbase_table[channel];
    read_address = (uint32_t *)vdma->VDMA_RDPTR;
    read_address = (uint32_t *)hal_memview_infrasys_to_mcu((uint32_t)((uint32_t *)read_address));
    *data = *read_address;

    vdma_set_sw_move_byte(channel, 4);

    return VDMA_OK;
}

vdma_status_t vdma_pop_data_multi_bytes(vdma_channel_t channel, uint8_t *data, uint32_t size)
{
    uint8_t h_size;
    uint8_t  *read_address;
    uint32_t ready_for_read;
    uint32_t byte_to_bndry;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    if (data == NULL) {
        return VDMA_INVALID_PARAMETER;
    }
    vdma = g_vdma_regbase_table[channel];
    h_size = vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SIZE;
    ready_for_read = (vdma->VDMA_FFCNT << h_size);
    if (size > ready_for_read) {
        return VDMA_INVALID_PARAMETER;
    }

    byte_to_bndry = vdma->VDMA_BYTE_TO_BNDRY;
    read_address = (uint8_t *)vdma->VDMA_RDPTR;
    read_address = (uint8_t *)hal_memview_infrasys_to_mcu((uint32_t)((uint32_t *)read_address));
    if (size > byte_to_bndry) {
        memcpy(data, read_address, byte_to_bndry);
        read_address = (uint8_t *)vdma->VDMA_PGMADDR;
        read_address = (uint8_t *)hal_memview_infrasys_to_mcu((uint32_t)((uint32_t *)read_address));
        memcpy((data + byte_to_bndry), read_address, (size - byte_to_bndry));
    } else {
        memcpy(data, read_address, size);
    }

    vdma_set_sw_move_byte(channel, size);

    return VDMA_OK;
}

vdma_status_t vdma_get_available_receive_bytes(vdma_channel_t channel, uint32_t *receive_bytes)
{
    uint8_t h_size;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    if (receive_bytes == NULL) {
        return VDMA_INVALID_PARAMETER;
    }

    vdma = g_vdma_regbase_table[channel];
    h_size = vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SIZE;
    *receive_bytes = (vdma->VDMA_FFCNT << h_size);

    return VDMA_OK;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM vdma_status_t vdma_get_available_send_space(vdma_channel_t channel, uint32_t *available_space)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    if (available_space == NULL) {
        return VDMA_INVALID_PARAMETER;
    }

    vdma = g_vdma_regbase_table[channel];
    *available_space = vdma->VDMA_BYTE_AVAIL;

    return VDMA_OK;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM vdma_status_t vdma_get_hw_read_point(vdma_channel_t channel, uint32_t *read_point)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    if (read_point == NULL) {
        return VDMA_INVALID_PARAMETER;
    }

    vdma = g_vdma_regbase_table[channel];
    *read_point = vdma->VDMA_RDPTR;

    return VDMA_OK;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM vdma_status_t vdma_get_hw_read_offset(vdma_channel_t channel, uint32_t *read_offset)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    if (read_offset == NULL) {
        return VDMA_INVALID_PARAMETER;
    }

    vdma = g_vdma_regbase_table[channel];
    *read_offset = vdma->VDMA_RDPTR - vdma->VDMA_PGMADDR;

    return VDMA_OK;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM vdma_status_t vdma_set_sw_move_byte(vdma_channel_t channel, uint16_t sw_move_byte)
{
    uint8_t h_size = 0;
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    vdma = g_vdma_regbase_table[channel];
    h_size = vdma->VDMA_CON_UNION.VDMA_CON_CELLS.VDMA_SIZE;
    switch (h_size) {
        case 0x2:
            if ((sw_move_byte & 0x3) != 0x0) {
                return VDMA_INVALID_PARAMETER;
            }
            break;
        default:
            break;
    }

    vdma->VDMA_SW_MV_BYTE = (sw_move_byte >> h_size);

    return VDMA_OK;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM vdma_status_t vdma_get_hw_write_point(vdma_channel_t channel, uint32_t *write_point)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    if (write_point == NULL) {
        return VDMA_INVALID_PARAMETER;
    }

    vdma = g_vdma_regbase_table[channel];
    *write_point = vdma->VDMA_WRPTR;

    return VDMA_OK;
}

ATTR_TEXT_IN_TCM vdma_status_t vdma_get_hw_write_offset(vdma_channel_t channel, uint32_t *write_offset)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }

    if (write_offset == NULL) {
        return VDMA_INVALID_PARAMETER;
    }

    vdma = g_vdma_regbase_table[channel];
    *write_offset = vdma->VDMA_WRPTR - vdma->VDMA_PGMADDR;

    return VDMA_OK;
}

vdma_status_t vdma_register_callback(vdma_channel_t channel, vdma_callback_t callback, void *user_data)
{
    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    if (callback == NULL) {
        return VDMA_INVALID_PARAMETER;
    }

    g_vdma_callback[channel].func = callback;
    g_vdma_callback[channel].argument = user_data;

    internal_dma_register_irq_callback(g_vdma_channel_map_table[channel]);
    internal_dma_enable_irq(g_vdma_channel_map_table[channel]);

    return VDMA_OK;
}

bool vdma_buffer_is_empty(vdma_channel_t channel)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return false;
    }

    vdma = g_vdma_regbase_table[channel];
    if (vdma->VDMA_RDPTR != vdma->VDMA_WRPTR) {
        return false;
    }

    return true;
}

#ifdef DUMP_DMA_REGISTER_ENABLE
vdma_status_t vdma_dump_reg_list(vdma_channel_t channel)
{
    VDMA_REGISTER_T *vdma;

    if (channel >= VDMA_END_CHANNEL) {
        return VDMA_ERROR_CHANNEL;
    }
    (void)vdma;
    vdma = g_vdma_regbase_table[channel];
    log_hal_msgid_info("VDMA channel %d reg list dump:", 1, channel);
    log_hal_msgid_info("VDMA_COUNT = 0x%08x", 1, vdma->VDMA_COUNT);
    log_hal_msgid_info("VDMA_CON = 0x%08x", 1, vdma->VDMA_CON_UNION.VDMA_CON);
    log_hal_msgid_info("VDMA_START = 0x%08x", 1, vdma->VDMA_START);
    log_hal_msgid_info("VDMA_INTSTA = 0x%08x", 1, vdma->VDMA_INTSTA);
    log_hal_msgid_info("VDMA_LIMITER = 0x%08x", 1, vdma->VDMA_LIMITER);
    log_hal_msgid_info("VDMA_PGMADDR = 0x%08x", 1, vdma->VDMA_PGMADDR);
    log_hal_msgid_info("VDMA_WRPTR = 0x%08x", 1, vdma->VDMA_WRPTR);
    log_hal_msgid_info("VDMA_RDPTR = 0x%08x", 1, vdma->VDMA_RDPTR);
    log_hal_msgid_info("VDMA_FFCNT = 0x%08x", 1, vdma->VDMA_FFCNT);
    log_hal_msgid_info("VDMA_FFSTA = 0x%08x", 1, vdma->VDMA_FFSTA);
    log_hal_msgid_info("VDMA_ALTLEN = 0x%08x", 1, vdma->VDMA_ALTLEN);
    log_hal_msgid_info("VDMA_FFSIZE = 0x%08x", 1, vdma->VDMA_FFSIZE);
    log_hal_msgid_info("VDMA_BNDRY_ADDR = 0x%08x", 1, vdma->VDMA_BNDRY_ADDR);
    log_hal_msgid_info("VDMA_BYTE_TO_BNDRY = 0x%08x", 1, vdma->VDMA_BYTE_TO_BNDRY);
    log_hal_msgid_info("VDMA_BYTE_AVAIL = 0x%08x", 1, vdma->VDMA_BYTE_AVAIL);

    return VDMA_OK;
}
#endif

/*************************************************************************************************************/
/************************************DMA domain IRQ handler start line*********************************************/
/*************************************************************************************************************/

/* domain channel index list */
typedef enum {
    I2S_DMA_DOMAIN_I2S0_TX = 0,
    I2S_DMA_DOMAIN_I2S0_RX,
    I2S_DMA_DOMAIN_I2S1_TX,
    I2S_DMA_DOMAIN_I2S1_RX,
    I2S_DMA_DOMAIN_I2S2_TX,
    I2S_DMA_DOMAIN_I2S2_RX,
    I2S_DMA_DOMAIN_I2S3_TX,
    I2S_DMA_DOMAIN_I2S3_RX,
    I2S_DMA_DOMAIN_I2S4_TX,
    I2S_DMA_DOMAIN_I2S4_RX,
    I2S_DMA_DOMAIN_I2S5_TX,
    I2S_DMA_DOMAIN_I2S5_RX,
    I2S_DMA_DOMAIN_I2S6_TX,
    I2S_DMA_DOMAIN_I2S6_RX,
    I2S_DMA_DOMAIN_I2S7_TX,
    I2S_DMA_DOMAIN_I2S7_RX,
} i2s_dma_domain_channel_t;

/* Map from domain channel index to vdma_channel_t/pdma_channel_t */
static uint32_t g_i2s_dma_domain_channel_index[] = {
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21
};

void i2s_dma_interrupt_hander(hal_nvic_irq_t irq_number)
{
    void *argument;
    vdma_callback_t vdma_callback;
    i2s_dma_domain_channel_t channel = 0;
    VDMA_REGISTER_T *vdma;
    uint32_t vdma_index;

    if (irq_number != I2S_SLAVE_IRQn) {
        assert(0);
    }

    for (channel = I2S_DMA_DOMAIN_I2S0_TX; channel <= I2S_DMA_DOMAIN_I2S7_RX; channel++) {
        vdma_index = g_i2s_dma_domain_channel_index[channel];
        if ((g_vdma_init_status[vdma_index] == VDMA_INIT) && (I2S_DMA_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(channel))) {
            /*clear vdma interrupt*/
            vdma = g_vdma_regbase_table[vdma_index];
            vdma->VDMA_ACKINT = VDMA_ACKINT_BIT_MASK;
            /*call vdma user's callback */
            vdma_callback = g_vdma_callback[vdma_index].func;
            argument = g_vdma_callback[vdma_index].argument;
            if (vdma_callback != NULL) {
                vdma_callback(VDMA_EVENT_TRANSACTION_SUCCESS, argument);
            } else {
                assert(0);
            }
        }
    }
}

/* Map from domain channel index to vdma_channel_t */
static uint32_t uart_vdma_domain_channel_index[] = {
    0,
    1,
    2,
    3,
    4,
    5,
};

typedef enum {
    UART_DMA_DOMAIN_UART0_TX = 0,
    UART_DMA_DOMAIN_UART0_RX,
    UART_DMA_DOMAIN_UART1_TX,
    UART_DMA_DOMAIN_UART1_RX,
    UART_DMA_DOMAIN_UART2_TX,
    UART_DMA_DOMAIN_UART2_RX,
} uart_dma_domain_channel_t;

void uart_dma_interrupt_hander(hal_nvic_irq_t irq_number)
{
    uint32_t vdma_index;
    void *argument;
    VDMA_REGISTER_T *vdma;
    vdma_callback_t vdma_callback;
    uart_dma_domain_channel_t channel = 0;

    if (((irq_number == UART_DMA0_IRQn) || (irq_number == UART_DMA1_IRQn) || (irq_number == UART_DMA2_IRQn)) == false) {
        assert(0);
    }

    if (irq_number == UART_DMA0_IRQn) {
        for (channel = UART_DMA_DOMAIN_UART0_TX; channel <= UART_DMA_DOMAIN_UART0_RX; channel++) {
            vdma_index = uart_vdma_domain_channel_index[channel];
            if ((g_vdma_init_status[vdma_index] == VDMA_INIT)
                && (UART_DMA_0_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(channel - UART_DMA_DOMAIN_UART0_TX))) {
                /*clear vdma interrupt*/
                vdma = g_vdma_regbase_table[vdma_index];
                vdma->VDMA_ACKINT = VDMA_ACKINT_BIT_MASK;
                /*call vdma user's callback */
                vdma_callback = g_vdma_callback[vdma_index].func;
                argument = g_vdma_callback[vdma_index].argument;
                if (vdma_callback != NULL) {
                    vdma_callback(VDMA_EVENT_TRANSACTION_SUCCESS, argument);
                } else {
                    assert(0);
                }
            }
        }
    } else if (irq_number == UART_DMA1_IRQn) {
        for (channel = UART_DMA_DOMAIN_UART1_TX; channel <= UART_DMA_DOMAIN_UART1_RX; channel++) {
            vdma_index = uart_vdma_domain_channel_index[channel];
            if ((g_vdma_init_status[vdma_index] == VDMA_INIT)
                && (UART_DMA_1_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(channel - UART_DMA_DOMAIN_UART1_TX))) {
                /*clear vdma interrupt*/
                vdma = g_vdma_regbase_table[vdma_index];
                vdma->VDMA_ACKINT = VDMA_ACKINT_BIT_MASK;
                /*call vdma user's callback */
                vdma_callback = g_vdma_callback[vdma_index].func;
                argument = g_vdma_callback[vdma_index].argument;
                if (vdma_callback != NULL) {
                    vdma_callback(VDMA_EVENT_TRANSACTION_SUCCESS, argument);
                } else {
                    assert(0);
                }
            }
        }
    } else if (irq_number == UART_DMA2_IRQn) {
        for (channel = UART_DMA_DOMAIN_UART2_TX; channel <= UART_DMA_DOMAIN_UART2_RX; channel++) {
            vdma_index = uart_vdma_domain_channel_index[channel];
            if ((g_vdma_init_status[vdma_index] == VDMA_INIT)
                && (UART_DMA_2_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(channel - UART_DMA_DOMAIN_UART2_TX))) {
                /*clear vdma interrupt*/
                vdma = g_vdma_regbase_table[vdma_index];
                vdma->VDMA_ACKINT = VDMA_ACKINT_BIT_MASK;
                /*call vdma user's callback */
                vdma_callback = g_vdma_callback[vdma_index].func;
                argument = g_vdma_callback[vdma_index].argument;
                if (vdma_callback != NULL) {
                    vdma_callback(VDMA_EVENT_TRANSACTION_SUCCESS, argument);
                } else {
                    assert(0);
                }
            }
        }
    }
}

/* Map from domain channel index to pdma_channel_t */
static uint32_t i2c_pdma_domain_channel_index[] = {
    0,
    1,
    2,
    3,
    4,
    5,
};

typedef enum {
    I2C_DMA_DOMAIN_I2C0_TX = 0,
    I2C_DMA_DOMAIN_I2C0_RX,
    I2C_DMA_DOMAIN_I2C1_TX,
    I2C_DMA_DOMAIN_I2C1_RX,
    I2C_DMA_DOMAIN_I2C2_TX,
    I2C_DMA_DOMAIN_I2C2_RX,
} i2c_dma_domain_channel_t;

void i2c_dma_interrupt_hander(hal_nvic_irq_t irq_number)
{
    uint32_t pdma_index;
    void *argument;
    PDMA_REGISTER_T *pdma;
    pdma_callback_t pdma_callback;
    i2c_dma_domain_channel_t channel;

    if (((irq_number == I2C_DMA0_IRQn) || (irq_number == I2C_DMA1_IRQn) || (irq_number == I2C_DMA2_IRQn)) == false)  {
        assert(0);
    }

    if (irq_number == I2C_DMA0_IRQn) {
        for (channel = I2C_DMA_DOMAIN_I2C0_TX; channel <= I2C_DMA_DOMAIN_I2C0_RX; channel++) {
            pdma_index = i2c_pdma_domain_channel_index[channel];
            if ((g_pdma_init_status[pdma_index] == PDMA_INIT)
                && (I2C_DMA_0_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(channel - I2C_DMA_DOMAIN_I2C0_TX))) {
                /*clear pdma interrupt*/
                pdma = g_pdma_regbase_table[pdma_index];
                pdma->PDMA_ACKINT = PDMA_ACKINT_BIT_MASK;
                /*call pdma user's callback */
                pdma_callback = g_pdma_callback[pdma_index].func;
                argument = g_pdma_callback[pdma_index].argument;
                if (pdma_callback != NULL) {
                    pdma_callback(PDMA_EVENT_TRANSACTION_SUCCESS, argument);
                } else {
                    assert(0);
                }
            }
        }
    } else if (irq_number == I2C_DMA1_IRQn) {
        for (channel = I2C_DMA_DOMAIN_I2C1_TX; channel <= I2C_DMA_DOMAIN_I2C1_RX; channel++) {
            pdma_index = i2c_pdma_domain_channel_index[channel];
            if ((g_pdma_init_status[pdma_index] == PDMA_INIT)
                && (I2C_DMA_1_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(channel - I2C_DMA_DOMAIN_I2C1_TX))) {
                /*clear pdma interrupt*/
                pdma = g_pdma_regbase_table[pdma_index];
                pdma->PDMA_ACKINT = PDMA_ACKINT_BIT_MASK;
                /*call pdma user's callback */
                pdma_callback = g_pdma_callback[pdma_index].func;
                argument = g_pdma_callback[pdma_index].argument;
                if (pdma_callback != NULL) {
                    pdma_callback(PDMA_EVENT_TRANSACTION_SUCCESS, argument);
                } else {
                    assert(0);
                }
            }
        }
    } else if (irq_number == I2C_DMA2_IRQn) {
        for (channel = I2C_DMA_DOMAIN_I2C2_TX; channel <= I2C_DMA_DOMAIN_I2C2_RX; channel++) {
            pdma_index = i2c_pdma_domain_channel_index[channel];
            if ((g_pdma_init_status[pdma_index] == PDMA_INIT) &&
                (I2C_DMA_2_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(channel - I2C_DMA_DOMAIN_I2C2_TX))) {
                /*clear pdma interrupt*/
                pdma = g_pdma_regbase_table[pdma_index];
                pdma->PDMA_ACKINT = PDMA_ACKINT_BIT_MASK;
                /*call pdma user's callback */
                pdma_callback = g_pdma_callback[pdma_index].func;
                argument = g_pdma_callback[pdma_index].argument;
                if (pdma_callback != NULL) {
                    pdma_callback(PDMA_EVENT_TRANSACTION_SUCCESS, argument);
                } else {
                    assert(0);
                }
            }
        }
    }
}

/* Map from domain channel index to pdma_channel_t */
static uint32_t i3c_pdma_domain_channel_index[] = {
    6,
    7,
    8,
    9,
};

typedef enum {
    I3C_DMA_DOMAIN_I3C0_TX = 0,
    I3C_DMA_DOMAIN_I3C0_RX,
    I3C_DMA_DOMAIN_I3C1_TX,
    I3C_DMA_DOMAIN_I3C1_RX,
    I3C_DMA_DOMAIN_I3C2_TX,
    I3C_DMA_DOMAIN_I3C2_RX,
} i3c_dma_domain_channel_t;

void i3c_dma_interrupt_hander(hal_nvic_irq_t irq_number)
{
    uint32_t pdma_index;
    void *argument;
    PDMA_REGISTER_T *pdma;
    pdma_callback_t pdma_callback;
    i3c_dma_domain_channel_t channel = 0;

    if (((irq_number == I3C_DMA0_IRQn) || (irq_number == I3C_DMA1_IRQn)) == false) {
        assert(0);
    }

    if (irq_number == I3C_DMA0_IRQn) {
        for (channel = I3C_DMA_DOMAIN_I3C0_TX; channel <= I3C_DMA_DOMAIN_I3C0_RX; channel++) {
            pdma_index = i3c_pdma_domain_channel_index[channel];
            if ((g_pdma_init_status[pdma_index] == PDMA_INIT)
                && (I3C_DMA_0_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(channel - I3C_DMA_DOMAIN_I3C0_TX))) {
                /*clear pdma interrupt*/
                pdma = g_pdma_regbase_table[pdma_index];
                pdma->PDMA_ACKINT = PDMA_ACKINT_BIT_MASK;
                /*call pdma user's callback */
                pdma_callback = g_pdma_callback[pdma_index].func;
                argument = g_pdma_callback[pdma_index].argument;
                if (pdma_callback != NULL) {
                    pdma_callback(PDMA_EVENT_TRANSACTION_SUCCESS, argument);
                } else {
                    assert(0);
                }
            }
        }
    } else if (irq_number == I3C_DMA1_IRQn) {
        for (channel = I3C_DMA_DOMAIN_I3C1_TX; channel <= I3C_DMA_DOMAIN_I3C1_RX; channel++) {
            pdma_index = i3c_pdma_domain_channel_index[channel];
            if ((g_pdma_init_status[pdma_index] == PDMA_INIT)
                && (I3C_DMA_1_RG_GLB_STA & DMA_GLB_IRQ_STA_BIT_MASK(channel - I3C_DMA_DOMAIN_I3C1_TX))) {
                /*clear pdma interrupt*/
                pdma = g_pdma_regbase_table[pdma_index];
                pdma->PDMA_ACKINT = PDMA_ACKINT_BIT_MASK;
                /*call pdma user's callback */
                pdma_callback = g_pdma_callback[pdma_index].func;
                argument = g_pdma_callback[pdma_index].argument;
                if (pdma_callback != NULL) {
                    pdma_callback(PDMA_EVENT_TRANSACTION_SUCCESS, argument);
                } else {
                    assert(0);
                }
            }
        }
    }
}

