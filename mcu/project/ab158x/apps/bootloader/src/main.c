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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "memory_map.h"
#include "sys_init.h"
#include "bl_common.h"
#include "bl_fota.h"
#include "bl_boot_time_analyze.h"
#if defined(CORE_CM4)
#include "core_cm4.h"
#elif defined(CORE_CM33)
#include "core_cm33.h"
#endif

/* hal includes */
#include "hal_uart.h"
#include "hal_flash.h"
#include "hal_clock_internal.h"
#include "hal_pmu.h"
#include "hal_gpio.h"
#include "hal_usb_internal.h"
#include "hal_wdt_internal.h"
#include "hal_gpt.h"

/* middleware includes */
#ifdef MTK_SECURE_BOOT_ENABLE
#include "secure_boot.h"
#endif
#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#include "nvkey.h"
#endif
#ifdef AIR_ANTI_ROLLBACK_ENABLE
#include "anti_rollback.h"
#endif
#ifdef AIR_BL_DFU_ENABLE
#include "bl_dfu.h"
#include "dfu_util.h"
#endif

/* Define -------------------------------------------------------------------*/

/* Typedef -----------------------------------------------------------------*/


/* Variables ----------------------------------------------------------------*/
#ifdef HAL_FLASH_MODULE_ENABLED
extern int sfi_index;
extern uint8_t nor_id[4];
const partition_t protect_partition[] = {
    PARTITION_BL,
    PARTITION_SECURITY_HEADER
};
#endif

#ifdef AIR_ANTI_ROLLBACK_ENABLE
uint8_t g_antrbk_en = 0;
#endif

/* Functions ---------------------------------------------------------------*/
ATTR_TEXT_IN_TCM void bl_system_init()
{
	/* Enable FPU. Set CP10 and CP11 Full Access.  bl_print_internal in keil uses FPU.*/
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));
}

ATTR_TEXT_IN_TCM void bl_hardware_init()
{
    /*config wdt to cold reset for fix hw limitation*/
#ifndef AIR_BTA_IC_STEREO_HIGH_G3
    bl_latch_start_gpt(BOOT_TIME_WDT_RST);
    hal_wdt_set_reset_mode(true);
    bl_latch_end_gpt(BOOT_TIME_WDT_RST);
#endif

#ifdef AIR_BL_CACHE_ENABLE
    cache_init();
#endif

#ifdef HAL_DCXO_MODULE_ENABLED
    bl_latch_start_gpt(BOOT_TIME_DCXO_INT);
    hal_dcxo_init();
    bl_latch_end_gpt(BOOT_TIME_DCXO_INT);
#endif

    bl_latch_start_gpt(BOOT_TIME_CLOCK);
    hal_clock_init();
    bl_latch_end_gpt(BOOT_TIME_CLOCK);

    /* UART init */
    bl_latch_start_gpt(BOOT_TIME_UART);
    hal_uart_config_t uart_config;
#ifdef AIR_BL_DFU_ENABLE
    uart_config.baudrate = HAL_UART_BAUDRATE_921600;
#else
    uart_config.baudrate = HAL_UART_BAUDRATE_115200;
#endif
    uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    hal_uart_init(HAL_UART_0, &uart_config);
    /* print log */
    bl_set_debug_level(LOG_DEBUG);
    bl_print(LOG_DEBUG, "bl_uart_init\r\n");
    bl_latch_end_gpt(BOOT_TIME_UART);

    /* SFI init required for SFC clock change. */
    bl_latch_start_gpt(BOOT_TIME_FLASH);
    custom_setSFIExt();
    hal_flash_init();
#ifdef HAL_FLASH_MODULE_ENABLED
    bl_print(LOG_DEBUG, "flash ID =0x%x, 0x%x, 0x%x \r\n", nor_id[0], nor_id[1], nor_id[2]);
    bl_print(LOG_DEBUG, "sfi_index=%d\r\n", sfi_index);
#endif
    bl_print(LOG_DEBUG, "NOR_init\r\n");
    bl_latch_end_gpt(BOOT_TIME_FLASH);

    /* latch power key */
#ifdef HAL_PMU_MODULE_ENABLED
    bl_latch_start_gpt(BOOT_TIME_PWR_KEY);
    extern void pmu_latch_power_key_for_bootloader();
    pmu_latch_power_key_for_bootloader();
    bl_print(LOG_DEBUG, "hal_latch_power_key\r\n");
    bl_print(LOG_DEBUG, "Power off[%d] on [%d]\r\n", pmu_get_power_off_reason(), pmu_get_power_on_reason());
    bl_latch_end_gpt(BOOT_TIME_PWR_KEY);
#endif

    /* partition table init */
    bl_latch_start_gpt(BOOT_TIME_LP);
    lp_init();
    lp_register_readonly_partitions(protect_partition, sizeof(protect_partition) / sizeof(partition_t));
    bl_latch_end_gpt(BOOT_TIME_LP);

    /* nvdm init */
#ifdef MTK_NVDM_ENABLE
    bl_latch_start_gpt(BOOT_TIME_NVDM);
    bl_print(LOG_DEBUG, "nvdm_init\r\n");
    nvdm_init();
    bl_latch_end_gpt(BOOT_TIME_NVDM);
#if defined(LPOSC_FRQ_IN_NVKEY) && !defined(FPGA_ENV)
    hal_clock_bootloader_init();
#endif
#endif

    /* SF STT and Disturbance Test*/
#ifdef __SERIAL_FLASH_STT_EN__
    bl_latch_start_gpt(BOOT_TIME_STT);
    extern void stt_main(void);
    stt_main();
    bl_latch_end_gpt(BOOT_TIME_STT);
#endif

     /* deinit brom usb */
#ifdef AIR_BTA_IC_PREMIUM_G2
    bl_latch_start_gpt(BOOT_TIME_USB_DEINT);
    hal_usb_deinit_brom();
    bl_latch_end_gpt(BOOT_TIME_USB_DEINT);
#endif
}


#if defined(MTK_SECURE_BOOT_ENABLE)
void secure_boot_entry(void)
{
    sboot_status_t    sboot_ret  = SBOOT_STATUS_OK;
    uint32_t sechdrAddr = bl_custom_header_start_address();

    bl_print(LOG_DEBUG, "bl_sec_header_start_address = %x\r\n", sechdrAddr);

    sboot_ret = sboot_secure_boot_check((uint8_t *)sechdrAddr, NULL, SBOOT_IOTHDR_V2, SBOOT_EFUSE_HASH_KEY_A);
    if (sboot_ret != SBOOT_STATUS_OK) {
        if(sboot_ret == SBOOT_STATUS_NOT_ENABLE) {
            bl_print(LOG_DEBUG, "secure boot disabled\r\n");
        } else {
            bl_print(LOG_DEBUG, "secure boot check failed. system halt (%x)\r\n", sboot_ret);
            while(1);
        }
    } else {
        bl_print(LOG_DEBUG, "secure boot check pass\r\n");
    }
}

#if defined(AIR_ANTI_ROLLBACK_ENABLE)
void anti_rollback_entry(void)
{
    antrbk_status_t   antrbk_ret = ANTRBK_STATUS_OK;
    bool              antrbk_auto_update_en = true;
    uint32_t sechdrAddr = bl_custom_header_start_address();

    antrbk_ret = anti_rollback_check((uint8_t *)sechdrAddr, antrbk_auto_update_en);
    if(antrbk_ret != ANTRBK_STATUS_OK) {
        if(antrbk_ret == ANTRBK_STATUS_NOT_ENABLE) {
            bl_print(LOG_DEBUG, "anti_rollback is disabled\r\n");
        } else {
            bl_print(LOG_DEBUG, "anti_rollback check failed. system halt (%d)\r\n", antrbk_ret);
            while(1);
        }
    } else {
        bl_print(LOG_DEBUG, "anti_rollback check pass\r\n");
    }
}
#endif /*AIR_ANTI_ROLLBACK_ENABLE*/
#endif /*MTK_SECURE_BOOT_ENABLE*/


void bl_start_user_code()
{
    bl_latch_start_gpt(BOOT_TIME_USER_CODE);
    uint32_t targetAddr = bl_custom_mcu_start_address();

#if defined(MTK_SECURE_BOOT_ENABLE)
    secure_boot_entry();
#if defined(AIR_ANTI_ROLLBACK_ENABLE)
    anti_rollback_entry();
#endif
#endif

#ifdef AIR_BL_CACHE_ENABLE
    cache_deinit();
#endif

    bl_latch_end_gpt(BOOT_TIME_USER_CODE);
    bl_latch_end_gpt(BOOT_TIME_TOTAL);
    bl_gpt_print_all();

    bl_print(LOG_DEBUG, "Jump to addr %x\r\n\r\n", targetAddr);
    JumpCmd(targetAddr);
}


int main()
{
    bl_hardware_init();

#ifdef AIR_BL_DFU_ENABLE
    bl_dfu_entry();
#endif

#ifdef BL_FOTA_ENABLE
    bl_latch_start_gpt(BOOT_TIME_FOTA);
    bl_fota_process();
    bl_latch_end_gpt(BOOT_TIME_FOTA);
#endif

    bl_start_user_code();

    return 0;
}

