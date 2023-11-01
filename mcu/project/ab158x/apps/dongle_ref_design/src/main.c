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
#include "bt_power_on_config.h"
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

#include "bt_app_common.h"
#include "bt_system.h"

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

#define mainCHECK_DELAY ( ( portTickType ) 1000 / portTICK_RATE_MS )

#if 0
/**
* @brief       Audio Task
* @param[in]   pvParameters: Pointer that will be used as the parameter for the task being created.
* @return      None.
*/
static void vAudioTask(void *pvParameters)
{
    /* CM33 reset DSP0 */
    LOG_MSGID_I(main, "\r\n#### CM33 Begin to Reset DSP0... ####\r\n", 0);
    //spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_ENABLE);
    hal_dsp_core_reset(HAL_CORE_DSP0, DSP0_BASE);

    vTaskDelete(NULL);
}
#endif

void vApplicationIdleHook(void)
{
    /* log to flash feature, dynamic enable by config log port */
    log_offline_dump_idle_hook();

#ifdef MTK_NVDM_ENABLE
    log_save_filter();
#endif

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
    systemhang_set_safe_duration(SYSTEMHANG_USER_CONFIG_COUNT_TOTAL - 1, 60 * 5);

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

/*
#define mainCHECK_DELAY ( ( portTickType ) 1000 / portTICK_RATE_MS )
static void vTestTask(void *pvParameters)
{
    portTickType xLastExecutionTime, xDelayTime;

    xLastExecutionTime = xTaskGetTickCount();
    xDelayTime = mainCHECK_DELAY;

    while (1) {
        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
        LOG_I(common, "[CM33]Hello World from task %d \r\n", (unsigned int)xTaskGetTickCount());
    }
}
*/

#if defined(MTK_NVDM_ENABLE) && defined(MTK_FOTA_VIA_RACE_CMD)
extern void reserved_nvdm_item_list_check(void);
#endif
int main(void)
{

    /* Do system initialization, eg: hardware, nvdm. */
    system_init();
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

    apps_pre_init();
    bt_power_on_config_init();


#ifdef MTK_RACE_CMD_ENABLE
    race_init();
    race_app_init();
#ifdef MTK_FOTA_VIA_RACE_CMD
    race_fota_set_transmit_interval(RACE_FOTA_DEFAULT_SPP_TRANSMIT_INTERVAL_IN_MS,
                                    RACE_FOTA_DEFAULT_BLE_TRANSMIT_INTERVAL_IN_MS);

#endif
#endif

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

