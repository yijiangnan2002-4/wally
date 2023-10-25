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

#include "hal_nvic.h"
#include "hal_gpt.h"

#ifdef HAL_NVIC_MODULE_ENABLED
#include "hal_eint_internal.h"
#include "hal_nvic_internal.h"
//#include "hal_flash_disk_internal.h"
#include "memory_attribute.h"
#include "hal_log.h"
#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif /* MTK_SWLA_ENABLE */

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
#include "systemhang_tracer.h"
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */

#ifdef MTK_MEMORY_MONITOR_ENABLE
#include "memory_monitor.h"
#endif /* MTK_MEMORY_MONITOR_ENABLE */
#endif

#ifdef HAL_TIME_CHECK_ENABLED
#include "hal_time_check.h"
#include "exception_handler.h"
#include "hal_gpt_internal.h"
#endif
#ifndef __UBL__
#include "assert.h"
#endif

#ifdef HAL_ESC_MODULE_ENABLED
#include "esc_device_config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*nvic_callback)(hal_nvic_irq_t irq_number);
} nvic_function_t;

#define HAL_NVIC_MASK_IRQ_VALUE (0x10)    /*((0x01 << (8 - __NVIC_PRIO_BITS)) & 0xFF)*/

#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
ATTR_ZIDATA_IN_TCM uint32_t mask_irq_func = 0;
ATTR_ZIDATA_IN_TCM uint32_t time_check_disbale_irq_start;
ATTR_ZIDATA_IN_TCM uint32_t disable_irq_save_duration_us;
extern bool is_time_check_assert_enabled;
#endif

#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
extern void irq_excute_time_start();
extern void irq_excute_time_end();
#endif 

static const uint32_t defualt_irq_priority[IRQ_NUMBER_MAX] = {
    [OS_GPT_IRQn]         = OS_GPT_IRQ_PRIORITY,
    [MCU_DMA0_IRQn]       = MCU_DMA0_IRQ_PRIORITY,
    [MCU_DMA1_IRQn]       = MCU_DMA1_IRQ_PRIORITY,
    [UART_DMA0_IRQn]      = UART_DMA0_IRQ_PRIORITY,
    [UART_DMA1_IRQn]      = UART_DMA1_IRQ_PRIORITY,
    [UART_DMA2_IRQn]      = UART_DMA2_IRQ_PRIORITY,
    [I2C_DMA0_IRQn]       = I2C_DMA0_IRQ_PRIORITY,
    [I2C_DMA1_IRQn]       = I2C_DMA1_IRQ_PRIORITY,
    [I2C_DMA2_IRQn]       = I2C_DMA2_IRQ_PRIORITY,
    [I3C_DMA0_IRQn]       = I3C_DMA0_IRQ_PRIORITY,
    [I3C_DMA1_IRQn]       = I3C_DMA1_IRQ_PRIORITY,
    [SPI_MST0_IRQn]       = SPI_MST0_IRQ_PRIORITY,
    [SPI_MST1_IRQn]       = SPI_MST1_IRQ_PRIORITY,
    [SPI_MST2_IRQn]       = SPI_MST2_IRQ_PRIORITY,
    [SPI_SLV_IRQn]        = SPI_SLV_IRQ_PRIORITY,
    [SDIO_MST0_IRQn]      = SDIO_MST0_IRQ_PRIORITY,
    [UART0_IRQn]          = UART0_IRQ_PRIORITY,
    [UART1_IRQn]          = UART1_IRQ_PRIORITY,
    [UART2_IRQn]          = UART2_IRQ_PRIORITY,
    [CRYPTO_IRQn]         = CRYPTO_IRQ_PRIORITY,
    [TRNG_IRQn]           = TRNG_IRQ_PRIORITY,
    [I2S_SLAVE_IRQn]      = I2S_SLAVE_IRQ_PRIORITY,
    [I2C0_IRQn]           = I2C0_IRQ_PRIORITY,
    [I2C1_IRQn]           = I2C1_IRQ_PRIORITY,
    [I2C2_IRQn]           = I2C2_IRQ_PRIORITY,
    [I2C_AO_IRQn]         = I2C_AO_IRQ_PRIORITY,
    [I3C0_IRQn]           = I3C0_IRQ_PRIORITY,
    [I3C1_IRQn]           = I3C1_IRQ_PRIORITY,
    [RTC_IRQn]            = RTC_IRQ_PRIORITY,
    [GPT_IRQn]            = GPT_IRQ_PRIORITY,
    [GPT_SEC_IRQn]        = GPT_SEC_IRQ_PRIORITY,
    [SPM_IRQn]            = SPM_IRQ_PRIORITY,
    [WDT_IRQn]            = WDT_IRQ_PRIORITY,
    [EINT_SEC_IRQn]       = EINT_SEC_IRQ_PRIORITY,
    [EINT_IRQn]           = EINT_IRQ_PRIORITY,
    [SFC_IRQn]            = SFC_IRQ_PRIORITY,
    [ESC_IRQn]            = ESC_IRQ_PRIORITY,
    [USB_IRQn]            = USB_IRQ_PRIORITY,
    [DSP0_IRQn]           = DPS0_IRQ_PRIORITY,
    [CAP_TOUCH_IRQn]      = CAP_TOUCH_IRQ_PRIORITY,
    [AUDIOSYS0_IRQn]      = AUDIOSYS0_IRQ_PRIORITY,
    [AUDIOSYS1_IRQn]      = AUDIOSYS1_IRQ_PRIORITY,
    [AUDIOSYS2_IRQn]      = AUDIOSYS2_IRQ_PRIORITY,
    [AUDIOSYS3_IRQn]      = AUDIOSYS3_IRQ_PRIORITY,
    [ANC_IRQn]            = ANC_IRQ_PRIORITY,
    [ANC_RAMP_IRQn]       = ANC_RAMP_IRQ_PRIORITY,
    [ANC_DMA_IRQn]        = ANC_DMA_IRQ_PRIORITY,
    [VAD_IRQn]            = VAD_IRQ_PRIORITY,
    [BT_IRQn]             = BT_IRQ_PRIORITY,
    [BT_AURX_IRQn]        = BT_AURX_IRQ_PRIORITY,
    [BT_AUTX_IRQn]        = BT_AUTX_IRQ_PRIORITY,
    [BT_TIMER_IRQn]       = BT_TIMER_IRQ_PRIORITY,
    [BT_PLAY_EN_IRQn]     = BT_PLAY_EN_IRQ_PRIORITY,
    [VOW_SNR_IRQn]        = VOW_SNR_IRQ_PRIORITY,
    [VOW_FIFO_IRQn]       = VOW_FIFO_IRQ_PRIORITY,
    [SEC_VIOLATION_IRQn]  = SEC_VIOLATION_IRQ_PRIORITY,
    [MEM_ILLEGAL_IRQn]    = MEM_ILLEGAL_IRQ_PRIORITY,
    [BUS_ERR_IRQn]        = BUS_ERR_IRQ_PRIORITY,
    [MBX_TX0_IRQn]        = MBX_TX0_IRQ_PRIORITY,
    [MBX_TX1_IRQn]        = MBX_TX1_IRQ_PRIORITY,
    [MBX_TX2_IRQn]        = MBX_TX2_IRQ_PRIORITY,
    [MBX_TX3_IRQn]        = MBX_TX3_IRQ_PRIORITY,
    [MBX_TX4_IRQn]        = MBX_TX4_IRQ_PRIORITY,
    [MBX_TX5_IRQn]        = MBX_TX5_IRQ_PRIORITY,
    [MBX_RX0_IRQn]        = MBX_RX0_IRQ_PRIORITY,
    [MBX_RX1_IRQn]        = MBX_RX1_IRQ_PRIORITY,
    [MBX_RX2_IRQn]        = MBX_RX2_IRQ_PRIORITY,
    [MBX_RX3_IRQn]        = MBX_RX3_IRQ_PRIORITY,
    [MBX_RX4_IRQn]        = MBX_RX4_IRQ_PRIORITY,
    [MBX_RX5_IRQn]        = MBX_RX5_IRQ_PRIORITY,
    [PMU_IRQn]            = PMU_IRQ_PRIORITY,
    [IRRX_IRQn]           = IRRX_IRQ_PRIORITY,
    [DSP_ERR_IRQn]        = DSP_ERR_IRQ_PRIORITY,
    [SW_IRQn]             = SW_IRQ_PRIORITY,
    [CM33_reserved0_IRQn] = CM33_reserved0_IRQ_PRIORITY,
};

nvic_function_t nvic_function_table[IRQ_NUMBER_MAX];

typedef struct {
    uint32_t callback_addr;
    uint32_t mask_irq_time;
    uint32_t cur_time;
    uint32_t cur_time_32k;
} hal_nvic_debug_t;

#define HAL_NVIC_DEBUG_NUM (20)
ATTR_ZIDATA_IN_TCM hal_nvic_debug_t nvic_debug[HAL_NVIC_DEBUG_NUM];
ATTR_ZIDATA_IN_TCM uint32_t cur_index = 0;

ATTR_TEXT_IN_TCM void hal_nvic_debug_log_save(uint32_t callback_addr, uint32_t time)
{
    uint32_t count = 0;

    nvic_debug[cur_index].callback_addr = callback_addr;
    nvic_debug[cur_index].mask_irq_time = time;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(nvic_debug[cur_index].cur_time));
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    nvic_debug[cur_index].cur_time_32k = (uint32_t)(((uint64_t)count) * 1000 / 32768);

    cur_index ++;
    cur_index = (cur_index < HAL_NVIC_DEBUG_NUM) ? cur_index : 0;
}

hal_nvic_status_t hal_nvic_init(void)
{
    static uint32_t priority_set = 0;
    uint32_t i;
    if (priority_set == 0) {
        /* Set defualt priority only one time */
        for (i = 0; i < IRQ_NUMBER_MAX; i++) {
            if (defualt_irq_priority[i] >= (1 << __NVIC_PRIO_BITS)) {
                log_hal_msgid_info("[NVIC]defualt irq priority config error", 0);
#ifndef __UBL__
                assert(0);
#endif
            }
            NVIC_SetPriority((hal_nvic_irq_t)i, defualt_irq_priority[i]);
        }
        priority_set = 1;
    }
    return HAL_NVIC_STATUS_OK;
}

ATTR_TEXT_IN_TCM hal_nvic_status_t hal_nvic_enable_irq(hal_nvic_irq_t irq_number)
{
#if !(defined(__EXT_BOOTLOADER__) || defined(__EXT_DA__))
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;
    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
        return status;
    } else {
        NVIC_EnableIRQ(irq_number);
        status = HAL_NVIC_STATUS_OK;
        return status;
    }
#else
    (void)irq_number;
#endif
    return HAL_NVIC_STATUS_OK;
}

ATTR_TEXT_IN_TCM hal_nvic_status_t hal_nvic_disable_irq(hal_nvic_irq_t irq_number)
{
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
        return status;
    } else {
        NVIC_DisableIRQ(irq_number);
        status = HAL_NVIC_STATUS_OK;
    }

    return status;
}

uint32_t hal_nvic_get_pending_irq(hal_nvic_irq_t irq_number)
{
    uint32_t ret = 0xFF;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        return ret;
    } else {
        ret = NVIC_GetPendingIRQ(irq_number);
    }

    return ret;
}

hal_nvic_status_t hal_nvic_set_pending_irq(hal_nvic_irq_t irq_number)
{
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
        return status;
    } else {
        NVIC_SetPendingIRQ(irq_number);
        status = HAL_NVIC_STATUS_OK;
    }

    return status;
}

hal_nvic_status_t hal_nvic_clear_pending_irq(hal_nvic_irq_t irq_number)
{
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
        return status;
    } else {
        NVIC_ClearPendingIRQ(irq_number);
        status = HAL_NVIC_STATUS_OK;
    }

    return status;
}

hal_nvic_status_t hal_nvic_set_priority(hal_nvic_irq_t irq_number, uint32_t priority)
{
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
        return status;
    } else {
        NVIC_SetPriority(irq_number, priority);
        status = HAL_NVIC_STATUS_OK;
    }

    return status;
}

uint32_t hal_nvic_get_priority(hal_nvic_irq_t irq_number)
{
    uint32_t ret = 0xff;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        return ret;
    } else {
        ret = NVIC_GetPriority(irq_number);
    }

    return ret;
}

ATTR_TEXT_IN_TCM uint32_t get_current_irq()
{
    return (((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) >> SCB_ICSR_VECTACTIVE_Pos) - 16);
}

#ifdef HAL_FLASH_MODULE_ENABLED
extern void Flash_ReturnReady(void);
#endif

#if defined(HAL_ESC_MODULE_ENABLED) && defined(HAL_ESC_WITH_FLASH)
extern void esc_flash_return_ready(void);
#endif
ATTR_TEXT_IN_TCM hal_nvic_status_t isrC_main()
{
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;
    hal_nvic_irq_t irq_number;

#ifdef HAL_FLASH_MODULE_ENABLED
    Flash_ReturnReady();
#endif

#if defined(HAL_ESC_MODULE_ENABLED) && defined(HAL_ESC_WITH_FLASH)
    esc_flash_return_ready();
#endif

#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
    irq_excute_time_start();
#endif 

    irq_number = (hal_nvic_irq_t)(get_current_irq());

#if 0
    if (irq_number == SEC_VIOLATION_IRQn) {
#include "assert.h"
        assert(0);
    }
#endif

#ifdef MTK_SWLA_ENABLE_O2
    uart_debug_print_byte(UART_LA_PORT, 0x14 + irq_number);
#endif

#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
#if defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2)
    systemhang_tracer_t systemhang_irq_tracer;

    systemhang_interrupt_enter_trace((uint32_t)irq_number, &systemhang_irq_tracer);
#endif
#endif /* defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2) */
#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
#ifdef MTK_MEMORY_MONITOR_ENABLE
    memorymonitor_interrupt_enter_trace(irq_number);
#endif
#endif /* MTK_MEMORY_MONITOR_ENABLE */
#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
#ifdef MTK_SWLA_ENABLE
    if (irq_number != 0) {
        SLA_RamLogging((uint32_t)(IRQ_START | irq_number));
    }
#endif
#endif /* MTK_SWLA_ENABLE */

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;

#ifdef MTK_SWLA_ENABLE_O2
        uart_debug_print_byte(UART_LA_PORT, '*');
#endif

#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
        irq_excute_time_end();
#endif 

        return status;
    } else if (nvic_function_table[irq_number].nvic_callback == NULL) {
        status = HAL_NVIC_STATUS_ERROR_NO_ISR;
#ifdef READY
        log_hal_msgid_error("ERROR: no IRQ handler! \n", 0);
#endif

#ifdef MTK_SWLA_ENABLE_O2
        uart_debug_print_byte(UART_LA_PORT, '*');
#endif

#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
        irq_excute_time_end();
#endif 		
        return status;
    } else {
        nvic_function_table[irq_number].nvic_callback(irq_number);
        status = HAL_NVIC_STATUS_OK;
    }
#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
#ifdef MTK_SWLA_ENABLE
    if (irq_number != 0) {
        SLA_RamLogging((uint32_t)IRQ_END);
    }
#endif
#endif /* MTK_SWLA_ENABLE */
#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
#ifdef MTK_MEMORY_MONITOR_ENABLE
    memorymonitor_interrupt_exit_trace(irq_number);
#endif
#endif /* MTK_MEMORY_MONITOR_ENABLE */
#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
#if defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2)
    systemhang_interrupt_exit_trace((uint32_t)irq_number, &systemhang_irq_tracer);
#endif
#endif /* defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O1) || defined(MTK_SYSTEM_HANG_TRACER_ENABLE_O2) */

#ifndef __UBL__
    if (__get_BASEPRI() != 0) {
        log_hal_msgid_error("irq_number = %d \n", 1, irq_number);
        assert(0);
    }
#endif

#ifdef MTK_SWLA_ENABLE_O2
    uart_debug_print_byte(UART_LA_PORT, '*');
#endif

#if !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__)
    irq_excute_time_end();
#endif 

    return status;
}

hal_nvic_status_t hal_nvic_register_isr_handler(hal_nvic_irq_t irq_number, hal_nvic_isr_t callback)
{
    uint32_t mask;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX || callback == NULL) {
        return HAL_NVIC_STATUS_INVALID_PARAMETER;
    }

    hal_nvic_save_and_set_interrupt_mask(&mask);
    NVIC_ClearPendingIRQ(irq_number);
    nvic_function_table[irq_number].nvic_callback = callback;
    hal_nvic_restore_interrupt_mask(mask);

    return HAL_NVIC_STATUS_OK;
}

ATTR_TEXT_IN_TCM hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask(uint32_t *mask)
{
#if !(defined(__EXT_BOOTLOADER__) || defined(__EXT_DA__))
#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);
#endif

    *mask = __get_BASEPRI();
    if (*mask == 0x10) {
        return HAL_NVIC_STATUS_OK;
    }

    __set_BASEPRI(HAL_NVIC_MASK_IRQ_VALUE);  //set base priority 1, fault can be handled first as priority is default zero.
    __DMB();
    __ISB();

#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    if ((*mask == 0) && (hal_core_status_read(HAL_CORE_MCU) == HAL_CORE_ACTIVE)) {
        time_check_disbale_irq_start = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
        mask_irq_func = xLinkRegAddr;
    }
#endif
#else
    (void)*mask;
#endif
    return HAL_NVIC_STATUS_OK;
}

ATTR_TEXT_IN_TCM hal_nvic_status_t hal_nvic_restore_interrupt_mask(uint32_t mask)
{
#if !(defined(__EXT_BOOTLOADER__) || defined(__EXT_DA__))
#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    uint32_t temp_time_end, temp_duration_us;
    if ((mask == 0) && (HAL_NVIC_MASK_IRQ_VALUE == __get_BASEPRI()) && (hal_core_status_read(HAL_CORE_MCU) == HAL_CORE_ACTIVE)) {
        temp_time_end = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
        temp_duration_us = temp_time_end - time_check_disbale_irq_start;
        if ((temp_duration_us > TIME_CHECK_DISABLE_IRQ_TIME) && (is_time_check_assert_enabled == true)) {
            disable_irq_save_duration_us = temp_duration_us;
            ram_assert(0); /*flash mask irq too long need use ram assert*/
        }
        mask_irq_func = 0x0;
    } else if ((mask != 0) && (__get_BASEPRI() == 0)) {
        ram_assert(0);
    }
#endif
    __set_BASEPRI(mask);
    __DMB();
    __ISB();
#else
    (void)mask;
#endif
    return HAL_NVIC_STATUS_OK;
}

ATTR_TEXT_IN_TCM hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask)
{
#if !(defined(__EXT_BOOTLOADER__) || defined(__EXT_DA__))
#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);
#endif

    *mask = __get_BASEPRI();
    if (*mask == 0x10) {
        return HAL_NVIC_STATUS_OK;
    }

    __set_BASEPRI(HAL_NVIC_MASK_IRQ_VALUE); //set base priority 1, fault can be handled first as priority is default zero.
    __DMB();
    __ISB();

#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    if ((*mask == 0) && (hal_core_status_read(HAL_CORE_MCU) == HAL_CORE_ACTIVE)) {
#ifdef MTK_SWLA_ENABLE
        SLA_RamLogging(0xAAAAFFFF);
#endif 
        time_check_disbale_irq_start = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
        mask_irq_func = xLinkRegAddr;
    }
#endif
#else
    (void)*mask;
#endif
    return HAL_NVIC_STATUS_OK;
}

ATTR_TEXT_IN_TCM hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask)
{
#if !(defined(__EXT_BOOTLOADER__) || defined(__EXT_DA__))
#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    uint32_t temp_time_end, temp_duration_us;
    if ((mask == 0) && (HAL_NVIC_MASK_IRQ_VALUE == __get_BASEPRI()) && (hal_core_status_read(HAL_CORE_MCU) == HAL_CORE_ACTIVE)) {
        temp_time_end = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
        temp_duration_us = temp_time_end - time_check_disbale_irq_start;
        #ifdef MTK_SWLA_ENABLE
            SLA_RamLogging(0xAAAAAAAA);
        #endif 
        if ((temp_duration_us > TIME_CHECK_DISABLE_IRQ_TIME_SPECIAL) && (is_time_check_assert_enabled == true)) {
            disable_irq_save_duration_us = temp_duration_us;
            ram_assert(0); /*flash mask irq too long need use ram assert*/
        }
        mask_irq_func = 0x0;
    } else if ((mask != 0) && (__get_BASEPRI() == 0)) {
        ram_assert(0);
    }
#endif

    __set_BASEPRI(mask);
    __DMB();
    __ISB();
#else
    (void)mask;
#endif
    return HAL_NVIC_STATUS_OK;
}


#ifdef __cplusplus
}
#endif

#endif /* HAL_NVIC_MODULE_ENABLED */

