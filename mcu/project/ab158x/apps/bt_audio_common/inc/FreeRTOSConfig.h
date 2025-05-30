/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

/* Ensure stdint is only used by the compiler, and not the assembler. */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
#include <stdint.h>
#include <stdio.h>
#include "os_port_callback.h"
#include "os_trace_callback.h"

extern uint32_t SystemCoreClock;
#endif


#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             1
#define configUSE_TICK_HOOK             0
#define configCPU_CLOCK_HZ              ( SystemCoreClock )
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES            ( 15 )
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 384 )

#if defined(AIR_TWS_ENABLE)
/* ori:114kb
 * workaround for BTA-25528
 * ori:116kb
 * bisto/gsound worst case need 8.5k
 * ori:125kb
 * workaround for BTA-27416,add 1k
 * ori 126
 * now 129 ull2.0 merge back
 * ori 129
 * now 136 for BTA-41233,add 1k
 * ori 136
 * now 138 for BTA-42671,add 2k
 * ori 139
 * now 143 for BTA-46483,add 4k
 */
#if defined(AIR_BTA_IC_PREMIUM_G3)
#ifdef AIR_SYSLOG_BUFFER_EXPAND_ENABLE
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 154 * 1024 ) )
#else
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 140 * 1024 ) )
#endif 
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
#ifdef AIR_SYSLOG_BUFFER_EXPAND_ENABLE
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 158 * 1024 ) )
#else
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 144 * 1024 ) )
#endif 
#else
#ifdef AIR_SYSLOG_BUFFER_EXPAND_ENABLE
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 147 * 1024 ) )
#else
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 133 * 1024 ) )
#endif
#endif

#elif defined(AIR_SPEAKER_ENABLE)
// Speaker Project Heap configure
/* for use stack,add 1k,115k */
/* ori:131k,add 1k = 132k for bta-30841 */
/* ori:132k,add 1k = 133k for bta-34507 */
/* ori:133k,add 5k = 138k for bta-37749 */
/* ori:138k,add 2k = 140k for bta-40234 */
#if defined (AIR_BTA_IC_PREMIUM_G3)
/* ori:140k,add 2k = 142k for bta-42794 */
/* ori:142k,add 2k = 144k for nvdm */
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 144 * 1024 ) )
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 142 * 1024 ) )
#else
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 135 * 1024 ) )
#endif

#else /* AIR_HEADSET_ENABLE */
/* for use stack,add 1k,115k */
/* ori:131k,add 1k = 132k for bta-30841 */
/* ori:132k,add 1k = 133k for bta-34507 */
/* ori:133k,add 5k = 138k for bta-37749 */
/* ori:138k,add 2k = 140k for bta-40234 */
#if defined (AIR_BTA_IC_PREMIUM_G3)
/* ori:140k,add 2k = 142k for bta-42794 */
/* ori:142k,add 2k = 144k for nvdm */
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 144 * 1024 ) )
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 142 * 1024 ) )
#else
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 135 * 1024 ) )
#endif 

#endif 


#define configMAX_TASK_NAME_LEN         ( 6 )
#define configUSE_TRACE_FACILITY        1
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1
#define configUSE_MUTEXES               1
#define configQUEUE_REGISTRY_SIZE       8
#define configCHECK_FOR_STACK_OVERFLOW  2
#define configUSE_RECURSIVE_MUTEXES     1
#define configUSE_MALLOC_FAILED_HOOK    1
#define configUSE_APPLICATION_TASK_TAG  0
#define configUSE_COUNTING_SEMAPHORES   1
#define configUSE_TICKLESS_IDLE         2
//#define configTICKLESS_DYNAMIC_ENABLE   1

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_STATS_FORMATTING_FUNCTIONS 1

#if defined(MTK_OS_CPU_UTILIZATION_ENABLE)
/* Run time stats gathering definitions. */
void vConfigureTimerForRunTimeStats(void);
uint32_t ulGetRunTimeCounterValue(void);
#define configGENERATE_RUN_TIME_STATS   1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vConfigureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE() ulGetRunTimeCounterValue()
#endif

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES       0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH        10
#define configTIMER_TASK_STACK_DEPTH    ( configMINIMAL_STACK_SIZE * 2 )
/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskCleanUpResources   1
#define INCLUDE_vTaskSuspend            1
#define INCLUDE_vTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_pcTaskGetTaskName 1
#define INCLUDE_eTaskGetState     1
#define INCLUDE_xTaskGetCurrentTaskHandle 1

/* Port specific configuration. */
#define configENABLE_MPU                      0
#define configENABLE_FPU                      1

#ifdef AIR_LIMIT_TZ_ENABLE
#define configENABLE_TRUSTZONE                0 //todo: after the secure bin build with the files under the path mcu/kernel/rtos/FreeRTOS/Source/portable/GCC/ARM_CM33/secure, this option should switch to 1
#define configRUN_FREERTOS_SECURE_ONLY        0
#else
#define configENABLE_TRUSTZONE                0
#define configRUN_FREERTOS_SECURE_ONLY        1
#endif /* AIR_LIMIT_TZ_ENABLE */

#define configMINIMAL_SECURE_STACK_SIZE       ((uint32_t)1024)

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
#define configPRIO_BITS             __NVIC_PRIO_BITS
#else
#define configPRIO_BITS             4      /* 16 priority levels */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         0xff

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    0x1

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY         (( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS)) & 0xFF )

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
//#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); printf("\n%s : %s #%d\n", __FILE__, __FUNCTION__, __LINE__);for( ;; ); }


/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#if MTK_SWLA_ENABLE
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_pcTaskGetTaskName 1
#include "os_trace_callback.h"
#endif /* MTK_SWLA_ENABLE */

#if configUSE_TICKLESS_IDLE == 1
#define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime ) vPortSuppressTicksAndSleep( xExpectedIdleTime )
#elif configUSE_TICKLESS_IDLE == 2
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
extern void tickless_handler(uint32_t xExpectedIdleTime);
#endif /*#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)*/
#define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime ) tickless_handler( xExpectedIdleTime )
#endif

#if (configUSE_TICKLESS_IDLE >= 1)
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
/* restore dwt stack overflow check after deep sleep */
#undef configPOST_SLEEP_PROCESSING
#define configPOST_SLEEP_PROCESSING(x)                              \
{                                                                    \
    extern void hal_dwt_init(void);                                  \
    extern void vPortCurrentTaskStackOverflowCheck(void);            \
    hal_dwt_init();                                                  \
    vPortCurrentTaskStackOverflowCheck();                            \
}
#endif /* (configCHECK_FOR_STACK_OVERFLOW > 0) */
#endif /* (configUSE_TICKLESS_IDLE >= 1) */

/* Irq cannot masked longer than 70us to avoid some real time requirement. */
#define configCheck_CRITICAL_SECTION_LENGTH
#define configMASK_IRQ_DURATION_TIME 70 /* Check 70us, irq cannot masked longer than 70us. */

#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
#include "syslog.h"
#define configASSERT( x ) if( (x) == 0 ) { \
    extern void light_assert(const char *expr, const char *file, int line); \
    ATTR_LOG_STRING exp[] = #x; \
    ATTR_LOG_STRING file[] = __FILE__; \
    light_assert(exp, file, __LINE__); \
}

#endif /*#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)*/

#endif /* FREERTOS_CONFIG_H */

