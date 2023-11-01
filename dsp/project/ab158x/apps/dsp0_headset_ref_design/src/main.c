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

#include <xtensa/hal.h>
#include <xtensa/xtruntime.h>
#include "hal.h"
#include "sys_init.h"
#include "syslog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "task_def.h"
#include "timers.h"
#include "assert.h"
#include "dsp.h"
#include "memory_map.h"
#include "string.h"
#include "hal_resource_assignment.h"
#include "matchning_table.h"

extern void DSP_Init(void);


////////////////////////////////////////////////////////////////////////////////
// Function Declarations ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define mainCHECK_DELAY ( ( portTickType ) 1000 / portTICK_RATE_MS )//delay 1000ms

#ifdef AIR_BTA_IC_PREMIUM_G3
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
#endif

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
#ifdef AIR_BTA_IC_PREMIUM_G3
void vApplicationMallocFailedHook(void *pucPoolBuffer)
{
#ifdef MTK_SUPPORT_HEAP_DEBUG
    /* dump all heap blocks */
    extern uint8_t ucHeap[];
    extern void mp_dump_status(void *pucPoolBuffer);
    if((uint32_t)pucPoolBuffer == (uint32_t)ucHeap) {
        /* dump all heap blocks */
        // mp_dump_status(&ucHeap);
        configASSERT(0);
    }
#else 
    (void)pucPoolBuffer;
#endif /* MTK_SUPPORT_HEAP_DEBUG */

    //configASSERT(0);
}
#else
void vApplicationMallocFailedHook(void *pucPoolBuffer)
{
#ifdef MTK_SUPPORT_HEAP_DEBUG
    /* dump all heap blocks */
    extern uint8_t ucHeap[];
    extern void mp_dump_status(void *pucPoolBuffer);
    if((uint32_t)pucPoolBuffer == (uint32_t)ucHeap) {
        /* dump all heap blocks */
        //mp_dump_status(&ucHeap);

        configASSERT(0);
    }
    #else
    (void)pucPoolBuffer;
    #endif /* MTK_SUPPORT_HEAP_DEBUG */
}
#endif /* AIR_BTA_IC_PREMIUM_G3*/
#endif

/**
* @brief       Task main function
* @param[in]   pvParameters: Pointer that will be used as the parameter for the task being created.
* @return      None.
*/
void vTestTask(void *pvParameters)
{
    uint32_t idx = (int)pvParameters;
    portTickType xLastExecutionTime, xDelayTime;

    xLastExecutionTime = xTaskGetTickCount();
    xDelayTime = (1 << idx) * mainCHECK_DELAY;

    while (1) {
        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
        LOG_MSGID_I(common,"Hello World from task %d at %d tick", 2, idx, xTaskGetTickCount());
    }
}

#if 0
#ifdef PRELOADER_ENABLE
/**
* SDK Sample code
* @brief       Audio Task
* @param[in]   pvParameters: Pointer that will be used as the parameter for the task being created.
* @return      None.
*/
static void vAudioTask(void *pvParameters)
{
    /* DSP0 reset DSP1 */
#ifdef FLASH_BOOTING
    LOG_MSGID_I(common,"#### DSP0 Begin to Reset DSP1... ####\r\n",0);
   // hal_dsp_core_reset(HAL_CORE_DSP1, DSP1_BASE);
#endif
    while(1){
        vTaskDelete(NULL);
    }
}
#endif

#ifdef FREERTOS_TEST
#include "FreeRTOS_test.h"
void vApplicationTickHook(void)
{
    vFreeRTOS_TestISR();
}
#endif /* FREERTOS_TEST */
#endif

#if( configUSE_IDLE_HOOK == 1 )

void vApplicationIdleHook(void)
{
#ifdef AIR_BTA_IC_PREMIUM_G2
    static uint32_t last_count = 0;
    uint32_t current_count = 0, time_count = 0;

    /* every >1s to dump system info */
    {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &current_count);
        hal_gpt_get_duration_count(last_count, current_count, &time_count);
        if(time_count > 32768 * 1){
            #ifdef AIR_CPU_MCPS_PRIORING_ENABLE
            extern void osStatusInfo(void);
            osStatusInfo();
            #else
            extern void osDumpHeapInfo(void);
            osDumpHeapInfo();
            #endif /* AIR_CPU_MCPS_PRIORING_ENABLE */

            last_count = current_count;
        }
    }
#endif
}

#endif /* configUSE_IDLE_HOOK == 1 */

#ifndef AIR_BTA_IC_PREMIUM_G2
/* In reent.c - init the global reent struct */
extern void _init_reent_bss(void);
#endif

/**
 * The entry of C code
 */
int main(void)
{
    system_init();

#ifndef AIR_BTA_IC_PREMIUM_G2
    _init_reent_bss();   /*init the reent (clib.bss section)*/
#endif

    LOG_MSGID_I(common,"DSP start..\r\n",0);

#ifdef FPGA_ENV
#ifdef FREERTOS_TEST
    LOG_MSGID_I(common,"start to run freertos test.\r\n",0);
    xTaskCreate(vMainTestTask, FREERTOS_EXAMPLE_TASK_NAME, FREERTOS_EXAMPLE_TASK_STACKSIZE / sizeof(portSTACK_TYPE), NULL, FREERTOS_EXAMPLE_TASK_PRIO, NULL);
#else
    /* TODO: Only for FPGA and Early development */
    for (uint32_t idx = 0; idx < 1; idx++) {
        xTaskCreate(vTestTask, FREERTOS_EXAMPLE_TASK_NAME, 0x600 / sizeof(StackType_t), (void *)idx, FREERTOS_EXAMPLE_TASK_PRIO, NULL);
    }
#endif /* FREERTOS_TEST */
#endif



#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    dsp_matching_table_initialize();
#endif
    DSP_Init();

    LOG_MSGID_I(common, "create multi task done.", 0);

    /* Call this function to indicate the system initialize done. */
    //move "write core status as ACTIVE" to middleware aud_msg_init for ccni ready.
    //hal_core_status_write(HAL_CORE_DSP0, HAL_CORE_ACTIVE);
    //LOG_MSGID_I(common,"hal_core_status_write done.", 0);

#ifdef HAL_TIME_CHECK_ENABLED
    extern exception_config_mode_t exception_config_mode;
    memcpy(&exception_config_mode, (uint8_t *)(HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_START + HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_LEN - sizeof(exception_config_mode_t)), sizeof(exception_config_mode_t));
    if (exception_config_mode.exception_mode_t.mask_irq_check_assert) {
        hal_time_check_enable();
        LOG_MSGID_I(common, "DSP hal time check enable.", 0);
    }
#endif
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}

