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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "task_def.h"

/* hal includes */
#include "hal.h"
#include "sys_init.h"
#include "syslog.h"
#include "memory_map.h"
#include "hal_spm.h"
#include "apps_init.h"
#include "app_home_screen_idle_activity.h"
#include "string.h"
#include "hal_resource_assignment.h"

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
#include "systemhang_tracer.h"
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */

#ifdef MTK_MEMORY_MONITOR_ENABLE
#include "memory_monitor.h"
#endif /* MTK_MEMORY_MONITOR_ENABLE */

#ifdef MTK_FOTA_VIA_RACE_CMD
#include "fota.h"
#include "fota_flash.h"
#endif

#ifdef MTK_RACE_CMD_ENABLE
#include "race_xport.h"
#include "race_bt.h"
#include "race_fota.h"
#include "race_app_race_event_hdl.h"
#include "race_app_bt_event_hdl.h"
#endif

#ifdef HAL_TIME_CHECK_ENABLED
#include "exception_handler.h"
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
#include "audio_source_control.h"
#endif


#include "bt_app_common.h"
#include "bt_system.h"
#include "bt_power_on_config.h"
#ifdef MTK_LAYOUT_PARTITION_ENABLE
#include "layout_partition.h"
#endif

#ifdef AIR_BTA_IC_PREMIUM_G3
#ifdef AIR_CPU_IN_SECURITY_MODE
#if (__ARM_FEATURE_CMSE & 1) == 0
#error "Need ARMv8-M security extensions"
#elif (__ARM_FEATURE_CMSE & 2) == 0
#error "Compile with --cmse"
#endif
#endif

#if !defined(AIR_LIMIT_TZ_ENABLE) && !defined(AIR_CPU_IN_SECURITY_MODE)
#error "All in security no ns world"
#endif
#endif
#ifdef AIR_VA_MODEL_MANAGER_ENABLE
#include "va_model_manager.h"
#endif /* AIR_VA_MODEL_MANAGER_ENABLE */

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
#if (defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3))
#include "audio_source_control.h"
#endif
#endif

// richard for customer UI spec.
#include "app_customer_nvkey_operation.h"
#include "bsp_px31bf.h"
#include "bsp_hall_sensor.h"

//log_create_module_variant(BTIF, DEBUG_LOG_ON, PRINT_LEVEL_INFO);
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#if BOOT_METHOD == 0
/* RAM Booting */
#define DSP_START_ADDRESS 0x04700000
#elif BOOT_METHOD == 1
/* Flash Booting */
#ifdef MTK_LAYOUT_PARTITION_ENABLE
#define DSP_START_ADDRESS lp_get_begin_address(PARTITION_DSP0)
#else
#define DSP_START_ADDRESS 0x1801F000
#endif
#else
#define DSP_START_ADDRESS 0x04700000
#endif
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* It will be called in platform_assert function.
 */
void exception_platform_assert_hook()
{
    /* Note:Donnot call assert again in this function to avoid function recursion */
    return;
}

#if (PRODUCT_VERSION != 1552) && (PRODUCT_VERSION != 2552)
/* It will be called in light_assert function.
 */
void exception_light_assert_hook()
{
    /* Note:Donnot call assert again in this function to avoid function recursion */
    return;
}
#endif

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
void vApplicationMallocFailedHook()
{
    extern void vDumpHeapStatus();
    vDumpHeapStatus();
    configASSERT(0);
}
#endif

/* Create the log control block for different modules.
 * Please refer to the log dev guide under /doc folder for more details.
 */
log_create_module(main, PRINT_LEVEL_INFO);

void vApplicationIdleHook(void)
{
    /* log to flash feature, dynamic enable by config log port */
    log_offline_dump_idle_hook();

#ifdef MTK_NVDM_ENABLE
    log_save_filter();
#endif

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
    systemhang_set_safe_duration(SYSTEMHANG_USER_CONFIG_COUNT_TOTAL - 1, SYSTEMHANG_IDLE_MONITOR_TIME);

#if (configUSE_TICKLESS_IDLE != 2)
    /* If tickless mode is not enabled, sometimes the idle task will run more than 1 min */
    /* For avoiding the wdt timeout in this case, feed wdt in every idle task's loop */
    systemhang_wdt_feed_in_task_switch();
#endif /* configUSE_TICKLESS_IDLE != 2 */
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */
    app_home_screen_check_power_off_and_reboot();
}

#ifdef MTK_MEMORY_MONITOR_ENABLE
int memorymonitor_memory_check(void)
{
    // hal_dwt_request_watchpoint(MEMORYMONITOR_DWT_CHANNEL, ((uint32_t)0x0), 0x2, WDE_DATA_WO);
    // if ( *((int *)0x0 + 4) > 0x10000 )
    // {
    //     return -1;
    // }

    return 0;
}
#endif /* MTK_MEMORY_MONITOR_ENABLE */
#if defined(MTK_NVDM_ENABLE) && defined(MTK_FOTA_VIA_RACE_CMD)
extern void reserved_nvdm_item_list_check(void);
#endif
int main(void)
{
#ifdef AIR_TEMP_FEATURE_ENABLE
    pmu_set_register_value(0x132, 0xffff, 0, 0x2088);
    pmu_set_register_value(0x134, 0xffff, 0, 0x82);
#endif
    /* Do system initialization, eg: hardware, nvdm. */
    system_init();
#if (defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3))
#ifdef AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE
    extern const char BSP_DUAL_CHIP_TRIGGER_POWER_KEY_PIN;
    hal_pinmux_set_function(BSP_DUAL_CHIP_TRIGGER_POWER_KEY_PIN, 0);
    hal_gpio_set_direction(BSP_DUAL_CHIP_TRIGGER_POWER_KEY_PIN, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(BSP_DUAL_CHIP_TRIGGER_POWER_KEY_PIN, HAL_GPIO_DATA_HIGH);
    hal_gpt_delay_ms(2000);
    hal_gpio_set_output(BSP_DUAL_CHIP_TRIGGER_POWER_KEY_PIN, HAL_GPIO_DATA_LOW);
#endif
#elif defined(AIR_BTA_IC_PREMIUM_G3)
#if defined(AIR_DCHS_MODE_1BATT_ENABLE) && defined(AIR_DCHS_MODE_MASTER_ENABLE)
    extern const char BSP_DUAL_CHIP_TRIGGER_POWER_KEY_PIN; /*for single battery mode, use one GPIO control slave power on.*/
    if (BSP_DUAL_CHIP_TRIGGER_POWER_KEY_PIN != 0xff) {
        hal_gpio_set_output(BSP_DUAL_CHIP_TRIGGER_POWER_KEY_PIN, HAL_GPIO_DATA_HIGH);
        LOG_MSGID_I(common, "[DCHS]1batt trigger slave poweron", 0);
    }
#endif
#endif
    /* FOTA init must be put just after system_init() */
#ifdef MTK_FOTA_VIA_RACE_CMD
    fota_init_flash();
#if FOTA_STORE_IN_EXTERNAL_FLASH
    fota_flash_config_fota_partition_in_external_flash(FOTA_EXT_RESERVED_BASE_DEFAULT,
                                                       FOTA_EXT_RESERVED_LENGTH_DEFAULT);
#endif
#endif
#if defined(MTK_NVDM_ENABLE) && defined(MTK_FOTA_VIA_RACE_CMD)
    reserved_nvdm_item_list_check();
#endif

	// richard for customer UI spec.
	app_nvkey_setting_init();
	bsp_component_px31bf_init();
	bsp_component_HALL_init();

    bt_power_on_config_init();


#ifdef MTK_RACE_CMD_ENABLE
    race_init();
#if (defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3))
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    audio_source_control_init();
#endif
#endif
    race_app_init();
#ifdef MTK_FOTA_VIA_RACE_CMD
    race_fota_set_transmit_interval(RACE_FOTA_DEFAULT_SPP_TRANSMIT_INTERVAL_IN_MS,
                                    RACE_FOTA_DEFAULT_BLE_TRANSMIT_INTERVAL_IN_MS);

#endif
#endif


#ifdef AIR_VA_MODEL_MANAGER_ENABLE
    /**
     * @brief Add the code for va model manager for switch language model.
     */
#ifdef AIR_BTA_IC_PREMIUM_G3
    va_model_manager_partition_info_t info[] = {
        {LM_GVA_BASE, LM_GVA_LENGTH},
        {LM_AMA_BASE, LM_AMA_LENGTH},
    };
#elif (defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3))
    va_model_manager_partition_info_t info[1] = {
        {LM_BASE, LM_LENGTH}
    };
#endif
    va_model_manager_init(info, sizeof(info) / sizeof(va_model_manager_partition_info_t), ROM_BASE);
#endif /* AIR_VA_MODEL_MANAGER_ENABLE */

#ifdef HAL_TIME_CHECK_ENABLED
    extern exception_config_mode_t exception_config_mode;
    if (exception_config_mode.exception_mode_t.mask_irq_check_assert) {
        hal_time_check_enable();
    }
    memcpy((uint8_t *)(HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_START + HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_LEN - sizeof(exception_config_mode_t)), &exception_config_mode, sizeof(exception_config_mode_t));
#endif

    /* Create tasks*/
    task_def_init();
    task_def_create();
    bt_app_common_init();
    apps_init();
	
    /* Call this function to indicate the system initialize done. */
    hal_core_status_write(HAL_CORE_MCU, HAL_CORE_ACTIVE);
	
    /* Start the scheduler. */
    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}

