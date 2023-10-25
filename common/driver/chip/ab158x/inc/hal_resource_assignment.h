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

#ifndef __HAL_RESOURCE_ASSIGNMENT_H__
#define __HAL_RESOURCE_ASSIGNMENT_H__

#include "stdio.h"
#include "stdint.h"
#include "stdarg.h"
#include "hal.h"

#ifdef HAL_HW_SEMAPHORE_MODULE_ENABLED
/**
 * @addtogroup HAL
 * @{
 * @addtogroup HW_SEMAPHORE
 * @{
 * @defgroup hal_hw_semaphore_define Define
 * @{
 */

/**
 * @brief  This macro defines the SEMAPHORE for SYSLOG.
 */
#define HW_SEMAPHORE_SYSLOG        0
/**
 * @brief  This macro defines the SEMAPHORE for EXCEPTION.
 */
#define HW_SEMAPHORE_EXCEPTION     1
/**
 * @brief  This macro defines the SEMAPHORE for HB.
 */
#define HW_SEMAPHORE_HB            2
/**
 * @brief  This macro defines the SEMAPHORE for AUDIO_SINK.
 */
#define HW_SEMAPHORE_AUDIO_SINK    3
/**
 * @brief  This macro defines the SEMAPHORE for CIRCULAR_BUFFER.
 */
#define HW_SEMAPHORE_DSP_CIRCULAR_BUFFER 4
/**
 * @brief  This macro defines the SEMAPHORE for DVFS.
 */
#define HW_SEMAPHORE_DVFS 5
/**
 * @brief  This macro defines the DSP0_PLAYBACK.
 */
#define HW_SEMAPHORE_AUDIO_CM4_DSP0_PLAYBACK 6
/**
 * @brief  This macro defines the SEMAPHORE for SLEEP.
 */
#define HW_SEMAPHORE_SLEEP 7
/**
 * @brief  This macro defines the SEMAPHORE for syslog porting layer.
 */
#define HW_SEMAPHORE_SYSLOG_WRAPER 8
/**
 * @brief  This macro defines the SEMAPHORE for mux layer.
 */
#define HW_SEMAPHORE_MUX 9
/**
  * @brief This macro defines the SEMAPHORE for TRNG.
  */
#define HW_SEMAPHORE_TRNG 10
/**
  * @brief This macro defines the SEMAPHORE for PMU.
  */
#define HW_SEMAPHORE_PMU 11
/**
  * @}
  */

/**
 * @}
 * @}
 */

#endif


/*
Total system private memory  4KB

----------------------------------
|   logging variable   --- 360  B
----------------------------------
|   USB                --- 180  B
----------------------------------
|   logging online     --- 80   B
----------------------------------
|   logging flash      --- 100  B
----------------------------------
|   MUX                --- 1624 B
----------------------------------
|   UART log port      --- 112  B
----------------------------------
|   logging version    --- 96   B
----------------------------------
|   logging build time --- 96   B
----------------------------------
|   Exception          --- 512  B
----------------------------------
|   CCNI               --- 128  B
----------------------------------
|   Core status        --- 16   B
----------------------------------
|   RTC                --- 4    B
----------------------------------
|   DSP exception info --- 48   B
----------------------------------
|   NS exception info  --- 256  B
----------------------------------
|   DSP ICE debug      --- 4    B
----------------------------------
|   1 wire             --- 4    B
----------------------------------
|   Low latency MUX    --- 4    B
----------------------------------
|   reserved           --- 462  B
----------------------------------
*/
#if defined(CORE_CM33)
#ifdef AIR_CPU_IN_SECURITY_MODE
#define HW_SYSRAM_PRIVATE_MEMORY_START  0x24208000
#else
#define HW_SYSRAM_PRIVATE_MEMORY_START  0x34208000
#endif
#elif defined (CORE_DSP0)
#ifdef AIR_CPU_IN_SECURITY_MODE
#define HW_SYSRAM_PRIVATE_MEMORY_START  0x64208000

#define AFE_BASE            (0xC0000000)
#define AFE_I2S_SLV0_BASE   (0xC9020000)
#define AFE_I2S_SLV1_BASE   (0xC9030000)
#define AFE_I2S_SLV2_BASE   (0xC9040000)
#define ANC_BASE            (0xC9050000)

#define ABB_BASE            (0x42090000)
#define ABB_CLK_BASE        (0x42010000)
#define XPLL_BASE           (0xA2050000)

#define VAD_BASE            (0x42130000)
#define VOW_CTRL_BASE       (0x44000000)
#define VOW_SRAM_BASE       (0x44010000)

#define VER_CTRL_BASE       (0xA2000000)
#else
#define HW_SYSRAM_PRIVATE_MEMORY_START  0x74208000

#define AFE_BASE            (0xD0000000)
#define AFE_I2S_SLV0_BASE   (0xD9020000)
#define AFE_I2S_SLV1_BASE   (0xD9030000)
#define AFE_I2S_SLV2_BASE   (0xD9040000)
#define ANC_BASE            (0xD9050000)

#define ABB_BASE            (0x52090000)
#define ABB_CLK_BASE        (0x52010000)
#define XPLL_BASE           (0xB2050000)

#define VAD_BASE            (0x52130000)
#define VOW_CTRL_BASE       (0x54000000)
#define VOW_SRAM_BASE       (0x54010000)

#define VER_CTRL_BASE       (0xB2000000)
#endif

#endif

#define HW_SYSRAM_PRIVATE_MEMORY_LEN    0x1000

#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VAR_START HW_SYSRAM_PRIVATE_MEMORY_START
#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_LEN   0x168

#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_USB_VAR_START (HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VAR_START + HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_USB_VAR_LEN   0xB4

#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_ONLINE_VAR_START (HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_USB_VAR_START + HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_USB_VAR_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_ONLINE_VAR_LEN   0x50

#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_FLASH_VAR_START (HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_ONLINE_VAR_START + HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_ONLINE_VAR_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_FLASH_VAR_LEN   0x64

#define HW_SYSRAM_PRIVATE_MEMORY_MUX_VAR_PORT_START (HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_FLASH_VAR_START + HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_FLASH_VAR_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_MUX_VAR_PORT_LEN 0x658

#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_UART_VAR_START (HW_SYSRAM_PRIVATE_MEMORY_MUX_VAR_PORT_START + HW_SYSRAM_PRIVATE_MEMORY_MUX_VAR_PORT_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_UART_VAR_LEN   0x70

#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VERSION_VAR_START (HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_UART_VAR_START + HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_UART_VAR_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VERSION_VAR_LEN   0x60

#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_BUILD_TIME_VAR_START (HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VERSION_VAR_START + HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VERSION_VAR_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_BUILD_TIME_VAR_LEN   0x60

#define HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_START (HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_BUILD_TIME_VAR_START + HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_BUILD_TIME_VAR_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_LEN   0x200

#define HW_SYSRAM_PRIVATE_MEMORY_CCNI_START (HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_START + HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_CCNI_LEN   0x80

#define HW_SYSRAM_PRIVATE_MEMORY_CORE_STATUS_START (HW_SYSRAM_PRIVATE_MEMORY_CCNI_START + HW_SYSRAM_PRIVATE_MEMORY_CCNI_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_STATUS_LEN   0x10

#define HW_SYSRAM_PRIVATE_MEMORY_RTC_FREQ_START (HW_SYSRAM_PRIVATE_MEMORY_CORE_STATUS_START + HW_SYSRAM_PRIVATE_MEMORY_STATUS_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_RTC_FREQ_LEN   0x4

#define HW_SYSRAM_PRIVATE_MEMORY_DSP_EXCEPTION_INFO_START (HW_SYSRAM_PRIVATE_MEMORY_RTC_FREQ_START + HW_SYSRAM_PRIVATE_MEMORY_RTC_FREQ_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_DSP_EXCEPTION_INFO_LEN   0x30

#define HW_SYSRAM_PRIVATE_MEMORY_NS_EXCEPTION_INFO_START (HW_SYSRAM_PRIVATE_MEMORY_DSP_EXCEPTION_INFO_START + HW_SYSRAM_PRIVATE_MEMORY_DSP_EXCEPTION_INFO_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_NS_EXCEPTION_INFO_LEN   0x100

#define HW_SYSRAM_PRIVATE_MEMORY_DSP_ICE_DEBUG_START (HW_SYSRAM_PRIVATE_MEMORY_NS_EXCEPTION_INFO_START + HW_SYSRAM_PRIVATE_MEMORY_NS_EXCEPTION_INFO_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_DSP_ICE_DEBUG_LEN   0x4

#define HW_SYSRAM_PRIVATE_MEMORY_1WIRE_START (HW_SYSRAM_PRIVATE_MEMORY_DSP_ICE_DEBUG_START + HW_SYSRAM_PRIVATE_MEMORY_DSP_ICE_DEBUG_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_1WIRE_LEN   0x4

#define HW_SYSRAM_PRIVATE_MEMORY_LLMUX_VAR_PORT_START (HW_SYSRAM_PRIVATE_MEMORY_1WIRE_START + HW_SYSRAM_PRIVATE_MEMORY_1WIRE_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_LLMUX_VAR_PORT_LEN 0x4

#define HW_SYSRAM_PRIVATE_MEMORY_FLASH_IRQ_STATUS_START (HW_SYSRAM_PRIVATE_MEMORY_LLMUX_VAR_PORT_START + HW_SYSRAM_PRIVATE_MEMORY_LLMUX_VAR_PORT_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_FLASH_IRQ_STATUS_LEN 0x8

#define HW_SYSRAM_PRIVATE_MEMORY_EPT_VAR_BACKUP_START (HW_SYSRAM_PRIVATE_MEMORY_FLASH_IRQ_STATUS_START + HW_SYSRAM_PRIVATE_MEMORY_FLASH_IRQ_STATUS_LEN)
#define HW_SYSRAM_PRIVATE_MEMORY_EPT_VAR_BACKUP_LEN 0x8

#define HW_SYSRAM_PRIVATE_MEMORY_RESERVED_START (HW_SYSRAM_PRIVATE_MEMORY_EPT_VAR_BACKUP_START + HW_SYSRAM_PRIVATE_MEMORY_EPT_VAR_BACKUP_LEN)

#define HW_SYSRAM_PRIVATE_MEMORY_RESERVED_LEN  \
    (HW_SYSRAM_PRIVATE_MEMORY_LEN - (\
    HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_USB_VAR_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_ONLINE_VAR_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_FLASH_VAR_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_MUX_VAR_PORT_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_UART_VAR_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VERSION_VAR_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_BUILD_TIME_VAR_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_CCNI_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_STATUS_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_RTC_FREQ_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_DSP_EXCEPTION_INFO_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_NS_EXCEPTION_INFO_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_DSP_ICE_DEBUG_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_1WIRE_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_LLMUX_VAR_PORT_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_FLASH_IRQ_STATUS_LEN + \
    HW_SYSRAM_PRIVATE_MEMORY_EPT_VAR_BACKUP_LEN))

#if HW_SYSRAM_PRIVATE_MEMORY_RESERVED_LEN < 0
#error Privite memory is overflow
#endif

/* TODO: The related API is not universal, please ask User to change it to a universal API. */
#define    hal_memview_cm4_to_dsp0                hal_memview_mcu_to_dsp0
#define    hal_memview_cm4_to_infrasys            hal_memview_mcu_to_infrasys
#define    hal_memview_dsp0_to_cm4                hal_memview_dsp0_to_mcu
#define    hal_memview_dsp0_to_infrasys           hal_memview_dsp0_to_infrasys
#define    hal_memview_infrasys_to_cm4            hal_memview_infrasys_to_mcu
#define    hal_memview_infrasys_to_dsp0           hal_memview_infrasys_to_dsp0

/* Private memory init */
void private_memory_init(void);

/* Memory View Transform */
uint32_t hal_memview_mcu_to_dsp0(uint32_t mcu_address);
uint32_t hal_memview_mcu_to_infrasys(uint32_t mcu_address);
uint32_t hal_memview_dsp0_to_mcu(uint32_t dsp0_address);
uint32_t hal_memview_dsp0_to_infrasys(uint32_t dsp0_address);
uint32_t hal_memview_infrasys_to_mcu(uint32_t infrasys_address);
uint32_t hal_memview_infrasys_to_dsp0(uint32_t infrasys_address);
/* Memory View Transform end*/


#endif//__HAL_RESOURCE_ASSIGNMENT_H__
