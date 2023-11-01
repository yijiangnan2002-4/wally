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


/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM4F port.
 *----------------------------------------------------------*/

#include "hal.h"
#include "FreeRTOS.h"
#include "port_tick.h"
#include "hal_clock.h"
#include "hal_nvic.h"
#include "timers.h"
#include "hal_gpt_internal.h"
#include "hal_resource_assignment.h"
#include <assert.h>

#if configUSE_TICKLESS_IDLE == 2
#include "task.h"
#include "memory_attribute.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_sleep_manager_platform.h"
#include "hal_gpt.h"
#include "hal_log.h"
#include "hal_eint.h"
#include "hal_spm.h"

#endif

#ifdef AIR_ICE_DEBUG_ENABLE
#include "hal_ice_debug.h"
#endif

#define MaximumIdleTime 20  //ms
#define DEEP_SLEEP_SW_BACKUP_RESTORE_TIME 2
#define REMAINDER_LIMIT 100*1000 //100ms

float RTC_Freq = 32.768; /* RTC 32.768KHz Freq*/
uint32_t nvic_mask;
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
extern uint32_t eint_get_status(uint32_t);
uint32_t wakeup_eint, wakeup_eint1, wakeup_eint2;
#endif

volatile uint32_t systick_change_period = 0, systick_change_period_dsp = 0;

//#define TICKLESS_DEEBUG_ENABLE
#ifdef  TICKLESS_DEEBUG_ENABLE
#define log_debug_tickless(_message,...) printf(_message, ##__VA_ARGS__)
#else
#define log_debug_tickless(_message,...)
#endif

#if configUSE_TICKLESS_IDLE == 2
static uint32_t hw_latency_time_ms = 0;
volatile uint32_t tick_remainder = 0;
#endif
static uint32_t count_idle_time;
static uint32_t count_sleep_time;

#include "hal_nvic.h"
#include <assert.h>
#include <xtensa/hal.h>

GPT_REGISTER_T *os_gpt1 = OS_GPT1;
OS_GPT_REGISTER_GLOABL_T *os_gpt_glb = OS_GPTGLB;
#define OST_COUNT_CLEAR         0x1
#define OST_COUNT_START         0x1
#define OST_IRQ_ENABLE          0x1
#define OST_IRQ_FLAG_ACK        0x1
#define OST_CLOCK_32KHZ         (0x10)
#define OST_CLOCK_13MHZ         (0x00)

typedef enum {
    OS_GPT_TIMER_TYPE_ONE_SHOT = 0,                /**< Set the GPT oneshot mode.  */
    OS_GPT_TIMER_TYPE_REPEAT   = 1                 /**< Set the GPT repeat  mode.  */
} os_gpt_timer_type_t;

typedef enum {
    OS_GPT_CLOCK_SOURCE_32K = 0,            /**< Set the GPT clock source to 32kHz, 1 tick = 1/32768 seconds. */
    OS_GPT_CLOCK_SOURCE_1M  = 1             /**< Set the GPT clock source to 1MHz, 1 tick = 1 microsecond.*/
} os_gpt_clock_source_t;

void os_gpt_interrupt_handle(hal_nvic_irq_t irq_number);
void os_gpt_init(void *callback)
{
    hal_nvic_disable_irq(OS_GPT_IRQn);
    hal_nvic_register_isr_handler(OS_GPT_IRQn, callback);
}

uint32_t port_enter_pure_wfi_mode()
{

#ifdef AIR_ICE_DEBUG_ENABLE
    if (hal_ice_debug_is_enabled() == true) {
        return 0;
    }
#endif

    if (hal_sleep_manager_is_sleep_locked() != 0)
    {
        __asm__ __volatile__(" dsync             \n"
                             " waiti 0           \n"
                             " isync             \n");
        return 1;
    }
    return 0;
}

void os_gpt_start(uint32_t time_out_us , os_gpt_timer_type_t timer_type , os_gpt_clock_source_t clock_source , uint32_t rtc_freq)
{
    os_gpt_timer_type_t ost_type = timer_type;
    os_gpt_clock_source_t ost_source = clock_source;
    uint32_t time_out_us_reload;

    time_out_us_reload = (ost_type == OS_GPT_TIMER_TYPE_ONE_SHOT) ? time_out_us : (time_out_us - 1);

    if ((time_out_us_reload / 1000) > HAL_GPT_MAXIMUM_MS_TIMER_TIME) {
        assert(0);
    }
    //  os_gpt1->GPT_IRQ_ACK = 0x1;                   //clear interrupt status
    os_gpt1->GPT_CON_UNION.GPT_CON |= (1 << 16);  // disabled the clock source

    /* set 13 divided with 13M source */
    if (ost_source == OS_GPT_CLOCK_SOURCE_32K) {       //set clock source
        os_gpt1->GPT_CON_UNION.GPT_CON |= (1 << 16);   // disable clock before config
        os_gpt1->GPT_CLK = 0x10;      //set 32k divide 1
        os_gpt1->GPT_CON_UNION.GPT_CON &= ~(1 << 16);   // enable clock
        //workround:
        time_out_us_reload = time_out_us_reload / 1000;
        os_gpt1->GPT_COMPARE = (uint32_t)(time_out_us_reload * 32 + (7 * time_out_us_reload) / 10 + (6 * time_out_us_reload) / 100 + (8 * time_out_us_reload) / 1000);
        (void)rtc_freq;

        //os_gpt1->GPT_COMPARE = (uint32_t)(((float)time_out_us_reload/1000.0) *rtc_freq);
        while (os_gpt_glb->OS_GPT_WCOMPSTA & (1 << 1));
        os_gpt1->GPT_CLR      = 0x1;                    // clear the count
        while (os_gpt_glb->OS_GPT_CLRSTA & (1 << 1));
    } else {
        os_gpt1->GPT_CON_UNION.GPT_CON |= (1 << 16);   // disable clock before config
        os_gpt1->GPT_CLK = 0xc;     //set  13M divide 13
        os_gpt1->GPT_CON_UNION.GPT_CON &= ~(1 << 16);   // enable clock

        os_gpt1->GPT_COMPARE = time_out_us_reload;
        while (os_gpt_glb->OS_GPT_WCOMPSTA & (1 << 1));
        os_gpt1->GPT_CLR      = 0x1;                    // clear the count
        while (os_gpt_glb->OS_GPT_CLRSTA & (1 << 1));
        (void)rtc_freq;
    }

    if (ost_type == OS_GPT_TIMER_TYPE_ONE_SHOT) {
        os_gpt1->GPT_CON_UNION.GPT_CON &= ~(3 << 8);    //set one-shot mode
        os_gpt_glb->OS_GPT_IRQMSK_DSP &= 0x1;           //DSP IRQ enable
        os_gpt_glb->OS_GPT_WAKEUPMSK_DSP &= 0x1;        //DSP wakeup enable
    } else {
        os_gpt1->GPT_CON_UNION.GPT_CON |= 0x101;        // set repeat mode
        os_gpt_glb->OS_GPT_IRQMSK_DSP &= 0x1;           //DSP IRQ enable
        os_gpt_glb->OS_GPT_WAKEUPMSK_DSP &= 0x1;        //DSP wakeup enable
    }

    /* enable IRQ */
    os_gpt1->GPT_IRQ_EN = 0x1;

    /* register and enable IRQ */
    hal_nvic_enable_irq(OS_GPT_IRQn);

    os_gpt1->GPT_CON_UNION.GPT_CON |= 0x01;
    return;
}

uint32_t os_gpt_stop()
{
    /*diable interrupt*/
    os_gpt1->GPT_IRQ_EN = ~OST_IRQ_ENABLE  ;
    /* stop timer */
    os_gpt1->GPT_CON_UNION.GPT_CON_CELLS.EN &= ~OST_COUNT_START;
    os_gpt1->GPT_IRQ_ACK = OST_IRQ_FLAG_ACK ;                    /* clean interrupt status */
    os_gpt1->GPT_CON_UNION.GPT_CON = 0;            /* disable timer     */

    //NVIC_DisableIRQ(OS_GPT_IRQn);
    hal_nvic_disable_irq(OS_GPT_IRQn);
    //NVIC_ClearPendingIRQ(OS_GPT_IRQn);
    xthal_set_intclear((0x1) << OS_GPT_IRQn);

    return os_gpt1->GPT_COUNT;
}

extern BaseType_t xPortSysTickHandler(void);
void os_gpt_interrupt_handle(hal_nvic_irq_t irq_number)
{
    (void)irq_number;

    os_gpt1->GPT_IRQ_ACK = 0x01;

    if (systick_change_period_dsp != 0) {
        systick_change_period_dsp = 0;
        os_gpt_start(portTICK_PERIOD_MS * 1000 , OS_GPT_TIMER_TYPE_REPEAT , OS_GPT_CLOCK_SOURCE_1M , RTC_Freq);
    }
    /* Run FreeRTOS tick handler*/
    xPortSysTickHandler();
}

#if configUSE_TICKLESS_IDLE == 2
ATTR_TEXT_IN_IRAM void tickless_handler(uint32_t xExpectedIdleTime)
{
    static uint32_t after_idle_time;
    static uint32_t ulCompleteTickPeriods, before_idle_time, remain, ulAST_Reload_ms, last_remain;
    static uint32_t after_sleep_time, before_sleep_time, sleep_time = 0;
    static uint32_t before_time[2], after_time[2], duration_time[2];
    static float calculation;


    /* critical section start */
    nvic_mask = (uint32_t)XTOS_SET_INTLEVEL(4);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&before_time[0]);

#ifdef AIR_ICE_DEBUG_ENABLE
    if (hal_ice_debug_is_enabled() == true) {
        XTOS_RESTORE_INTLEVEL(nvic_mask);
        return;
    }
#endif

    /* If a context switch is pending or a task is waiting for the scheduler
    to be unsuspended then abandon the low power entry. */
    if (eTaskConfirmSleepModeStatus() != eAbortSleep) {
        if ((xExpectedIdleTime > (MaximumIdleTime / (1000 / configTICK_RATE_HZ))) && (hal_sleep_manager_is_sleep_locked() == 0) && (((*SPM_PCM_RESERVE2 >> 31) & 0x01) != 1)) {
            /* Tickless SLEEP mode*/

            if (systick_change_period_dsp != 0) {
                systick_change_period_dsp = 0;
                tick_remainder += os_gpt_stop();
                tick_remainder += last_remain;
            } else {
                tick_remainder += os_gpt_stop();
            }

            /* Calculate total idle time to ms */
            ulAST_Reload_ms = (xExpectedIdleTime - 1) * (1000 / configTICK_RATE_HZ);
            ulAST_Reload_ms = ulAST_Reload_ms - DEEP_SLEEP_SW_BACKUP_RESTORE_TIME - hw_latency_time_ms;

            os_gpt_start(ulAST_Reload_ms * 1000 , OS_GPT_TIMER_TYPE_ONE_SHOT , OS_GPT_CLOCK_SOURCE_32K , RTC_Freq);
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &before_sleep_time);
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&after_time[0]);

            /* Enter Sleep mode */
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);

            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&before_time[1]);
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &after_sleep_time);
            /* stop GPT */
            os_gpt_stop();

            hal_gpt_get_duration_count(before_sleep_time, after_sleep_time, &sleep_time);

            sleep_time *= 1000;
            calculation = (float)sleep_time;
            calculation /= RTC_Freq;/*us*/

            sleep_time = (uint32_t)calculation;
            ulCompleteTickPeriods = (sleep_time / 1000) / ((1000 / configTICK_RATE_HZ));
            remain = sleep_time % (portTICK_PERIOD_MS * 1000);
            tick_remainder += remain;

            hal_gpt_get_duration_count(before_time[0], after_time[0], &duration_time[0]);
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&after_time[1]);
            hal_gpt_get_duration_count(before_time[1], after_time[1], &duration_time[1]);
            tick_remainder += duration_time[0] + duration_time[1];

            /* Limit OS Tick Compensation Value */
            if (ulCompleteTickPeriods >= (xExpectedIdleTime)) {
                ulCompleteTickPeriods = xExpectedIdleTime;
            } else {
                if (tick_remainder >= (portTICK_PERIOD_MS * 1000)) {
                    tick_remainder -= (portTICK_PERIOD_MS * 1000);
                    ulCompleteTickPeriods++;
                }
            }

            if (tick_remainder > 1000) {
                systick_change_period_dsp = 1;
                last_remain = (tick_remainder % (portTICK_PERIOD_MS * 1000));
                tick_remainder -= last_remain;
                remain = (portTICK_PERIOD_MS * 1000) - last_remain;
                os_gpt_start(remain, OS_GPT_TIMER_TYPE_REPEAT , OS_GPT_CLOCK_SOURCE_1M , RTC_Freq);
            } else {
                systick_change_period_dsp = 0;
                os_gpt_start(portTICK_PERIOD_MS * 1000, OS_GPT_TIMER_TYPE_REPEAT , OS_GPT_CLOCK_SOURCE_1M , RTC_Freq);
            }

            if (tick_remainder >= REMAINDER_LIMIT) {
                tick_remainder = REMAINDER_LIMIT;
            }

            vTaskStepTick(ulCompleteTickPeriods);

            /* Re-enable interrupts (critical section end) */
            XTOS_RESTORE_INTLEVEL(nvic_mask);
            count_sleep_time = sleep_time / 1000;
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
            wakeup_eint = eint_get_status(0);
            wakeup_eint1 = eint_get_status(1);
            wakeup_eint2 = eint_get_status(2);
#endif
            //sleep_management_dump_debug_log(SLEEP_MANAGEMENT_DEBUG_LOG_DUMP);

#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
#ifdef  SLEEP_MANAGEMENT_DEBUG_SLEEP_WAKEUP_LOG_ENABLE
            sleep_management_dump_wakeup_source(sleep_management_dsp_status.wakeup_source, wakeup_eint);
#endif
#endif
            log_debug_tickless("\r\nEIT=%u\r\n"  , (unsigned int)xExpectedIdleTime);
            log_debug_tickless("RL=%u\r\n"       , (unsigned int)ulAST_Reload_ms);
            return;
        } else {
            /* pure IDLE mode*/
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &before_idle_time);
            __asm__ __volatile__(" dsync             \n"
                                 " waiti 0           \n"
                                 " isync             \n");
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &after_idle_time);

            /* Re-enable interrupts (critical section end) */
            XTOS_RESTORE_INTLEVEL(nvic_mask);

            count_idle_time += (after_idle_time - before_idle_time) / 1000;
            return;
        }
    }

    /* Re-enable interrupts (critical section end) */
    XTOS_RESTORE_INTLEVEL(nvic_mask);
}
#endif


TimerHandle_t xTimerofTest;
void log_cup_resource_callback(TimerHandle_t pxTimer);

/*
A peripheral General Purpose Timer is used for OS timer, and the interrupt is handled in GPT handler.
*/
#if configGENERATE_RUN_TIME_STATS == 1
uint32_t task_information_period = 5000;
uint32_t task_information_period_old = 0;
#endif

void _frxt_tick_timer_init(void)
{
    uint32_t freq_32K;

    //get rtc real frequency from share memory
    freq_32K = *((volatile uint32_t *)HW_SYSRAM_PRIVATE_MEMORY_RTC_FREQ_START);

    //printf("\r\ndsp hal_rtc_get_f32k_frequency:%d\r\n", freq_32K);
    RTC_Freq = ((float)freq_32K) / 1000;

    os_gpt_init(os_gpt_interrupt_handle);
    os_gpt_start(portTICK_PERIOD_MS * 1000 , OS_GPT_TIMER_TYPE_REPEAT , OS_GPT_CLOCK_SOURCE_1M , RTC_Freq);

#if configUSE_TICKLESS_IDLE == 2
    int hw_latency_time_32k = spm_latency_time_checking(); //SPM Latency Time
    hw_latency_time_ms = (uint32_t)((((double)hw_latency_time_32k) * (1/32.768))) + 1;
#endif

#ifdef AIR_CPU_MCPS_PRIORING_ENABLE
    task_information_period_old = task_information_period;
    xTimerofTest = xTimerCreate("TimerofTest", (task_information_period / portTICK_PERIOD_MS), pdTRUE, NULL, log_cup_resource_callback);
    //xTimerStart(xTimerofTest, 0);
#endif
}


#ifdef AIR_CPU_MCPS_PRIORING_ENABLE
uint32_t pxTaskMaxTimeArray[MAX_TASK_TIME_COUNT] = {0};
static uint32_t copy_task_name_flag = TRUE;
dsp_task_info_t *dsp_task_info = NULL;
extern dsp_task_info_t *mcu_to_dsp_info;
uint8_t *task_name_array[MAX_TASK_COUNT];
option_task_maxtime_t task_maxtime_value_b = TASK_MAXTIME_DISABLE;
uint32_t gpt_32k_freq = 0;

extern uint32_t total_irq_interrupt_time_us;
//#include "hal_rtc.h"
//extern hal_rtc_status_t hal_rtc_get_f32k_frequency(uint32_t *frequency);
void log_cup_resource_callback(TimerHandle_t pxTimer)
{
    TaskStatus_t *pxTaskStatusArray;
    UBaseType_t uxArraySize, x;
    uint32_t ulTotalTime, ulStatsAsPercentage;
    static uint32_t ulTotalTime_now = 0, ulTotalTime_last = 0;
    float Percentage;
    float sleep_percentage;
    float idle_percentage;
    float busy_percentage;
    ulTotalTime = 0;
    uint32_t task_number = 0;
	//hal_rtc_get_f32k_frequency(&gpt_32k_freq);
	
    /* Optionally do something if the pxTimer parameter is NULL. */
    configASSERT(pxTimer);

    /* mcu have send cmd to config task proifling */
    if (mcu_to_dsp_info != NULL) {
        dsp_task_info = (dsp_task_info_t *)mcu_to_dsp_info;      
	}

    uxArraySize = uxTaskGetNumberOfTasks();
    if (uxArraySize >= MAX_TASK_COUNT) {
        log_hal_msgid_info("[ERROR]task maxtime is not support\r\n", 0);
    }


    pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray == NULL) {
        return;
    }
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalTime);
    if (copy_task_name_flag && (uxArraySize < MAX_TASK_COUNT)) {
        for (x = 0; x < uxArraySize; x++) {
			uint32_t task_name_len = strlen(pxTaskStatusArray[x].pcTaskName);
			/* task_name_len < configMAX_TASK_NAME_LEN */
            strncpy(dsp_task_info->task_info[pxTaskStatusArray[x].xTaskNumber - 1].task_name, pxTaskStatusArray[x].pcTaskName, task_name_len);
			dsp_task_info->task_info[pxTaskStatusArray[x].xTaskNumber - 1].task_name[task_name_len] = '\0';
        }
        copy_task_name_flag = FALSE;

    }
    ulTotalTime_now = ulTotalTime;

    if (ulTotalTime_now > ulTotalTime_last) {
        ulTotalTime = ulTotalTime_now - ulTotalTime_last;
    } else {
        ulTotalTime = ulTotalTime_now;
    }
    ulTotalTime_last = ulTotalTime_now;

    if (ulTotalTime > 0UL) {
        /*ulTotalTime is 32k counter*/
		uint32_t ulTotalTime_ms = (uint32_t)(ulTotalTime*GPT_32K_SWITCH_TO_MS);
        log_hal_msgid_info("ulTotalTimeL:%lu ms\r\n", 1, ulTotalTime_ms);
        log_hal_msgid_info("----------------------DSP Dump OS Task Info-----------------------------\r\n", 0);

        option_task_maxtime_t task_maxtime_value = dsp_task_info->TaskMaxTime;

        /* if is clear task maxtime value cmd ,*/
        if ((dsp_task_info != NULL ) && (TASK_MAXTIME_CLEAR == task_maxtime_value)) {
            dsp_task_info->TaskMaxTime = task_maxtime_value_b;
        }

        task_maxtime_value_b = task_maxtime_value;

        for (x = 0; x < uxArraySize; x++) {

            /* store task number */
            task_number =   pxTaskStatusArray[x].xTaskNumber - 1;

            /* store max run time */
            if (pxTaskMaxTimeArray[task_number] <  pxTaskStatusArray[x].ulRunTimeCounter) {
                pxTaskMaxTimeArray[task_number] = pxTaskStatusArray[x].ulRunTimeCounter;
            }

            /* clear task max time */
            if ((dsp_task_info != NULL ) && (TASK_MAXTIME_CLEAR == task_maxtime_value)) {
                pxTaskMaxTimeArray[x] = 0;
            }

            Percentage = ((float)pxTaskStatusArray[x].ulRunTimeCounter) / ((float)ulTotalTime);
            ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalTime;

            if (((dsp_task_info->option_task_info.task_number == task_number) && (dsp_task_info->task_info[dsp_task_info->option_task_info.task_number].isSelect))
                    || ((dsp_task_info->task_info[task_number].isSelect) && (x != dsp_task_info->option_task_info.task_number))) {
                if ((dsp_task_info != NULL ) && ((TASK_MAXTIME_ENABLE == task_maxtime_value) || (TASK_MAXTIME_CLEAR == task_maxtime_value))) {
					/* msgid donnot support print %% */
                    log_hal_msgid_info("Number %d  State[%lu] Percentage[%lu.%lu] MinStack[%lu]byte RunTime[%lu]msec  MaxRunTime[%lu]msec\r\n",7
								 , pxTaskStatusArray[x].xTaskNumber - 1
                                 //, pxTaskStatusArray[x].pcTaskName
                                 , pxTaskStatusArray[x].eCurrentState
                                 , (uint32_t)(Percentage * 100)
                                 , ((uint32_t)(Percentage * 1000)) % 10
                                 , pxTaskStatusArray[x].usStackHighWaterMark
                                 , (uint32_t)(pxTaskStatusArray[x].ulRunTimeCounter * GPT_32K_SWITCH_TO_MS)
                                 , (uint32_t)(pxTaskMaxTimeArray[task_number]*GPT_32K_SWITCH_TO_MS));
                } else {
					/* msgid donnot support print %% */
                    log_hal_msgid_info("Number %d  State[%lu] Percentage[%lu.%lu] MinStack[%lu]byte RunTime[%lu]msec\r\n",6
								 , pxTaskStatusArray[x].xTaskNumber - 1
                                 //, pxTaskStatusArray[x].pcTaskName
                                 , pxTaskStatusArray[x].eCurrentState
                                 , (uint32_t)(Percentage * 100)
                                 , ((uint32_t)(Percentage * 1000)) % 10
                                 , pxTaskStatusArray[x].usStackHighWaterMark
                                 , (uint32_t)(pxTaskStatusArray[x].ulRunTimeCounter * GPT_32K_SWITCH_TO_MS));
                }

            }
            if ( (TRUE == dsp_task_info->isEnableTaskThreshold)) {
                if ( (pxTaskStatusArray[x].ulRunTimeCounter * GPT_32K_SWITCH_TO_MS >= dsp_task_info->task_info[task_number].task_threshold)
                        && (dsp_task_info->task_info[task_number].task_threshold != 0) ) {
                    log_hal_msgid_warning("Number %d  task has exceeded threshold,runtime:%dmsec,threshold:%dmsec\r\n"
                                          , 3
                                          , (pxTaskStatusArray[x].xTaskNumber - 1)
                                          , (uint32_t)(pxTaskStatusArray[x].ulRunTimeCounter * GPT_32K_SWITCH_TO_MS)
                                          , dsp_task_info->task_info[task_number].task_threshold);
                }
            }
        }
		log_hal_msgid_info("Irq total time : %d ms\r\n", 1, total_irq_interrupt_time_us / 1000);		
#if configUSE_TICKLESS_IDLE == 2		
        log_hal_msgid_info("Idle mode time : %d ms\r\n", 1, count_idle_time);
        log_hal_msgid_info("Sleep mode time : %d ms\r\n", 1, count_sleep_time);
        idle_percentage = (float)count_idle_time  / (float)ulTotalTime_ms;
        sleep_percentage =  (float)(count_sleep_time) / (float)ulTotalTime_ms;
        busy_percentage = 1 - idle_percentage - sleep_percentage;

        log_hal_msgid_info("Idle mode time percentage: [%lu.%lu]\r\n", 2, (uint32_t)(idle_percentage * 100),((uint32_t)(idle_percentage * 1000))%10);
        log_hal_msgid_info("Sleep mode time percentage: [%lu.%lu]\r\n", 2, (uint32_t)(sleep_percentage * 100),((uint32_t)(sleep_percentage * 1000))%10);
        log_hal_msgid_info("Busy time percentage: [%lu.%lu]\r\n", 2, (uint32_t)(busy_percentage * 100),((uint32_t)(busy_percentage * 1000)) %10);
        count_idle_time = 0;
        count_sleep_time = 0;
#endif 

        log_hal_info("----------------------------------------------------------------------\r\n");

    }

    vPortFree(pxTaskStatusArray);

    vTaskClearTaskRunTimeCounter();

	total_irq_interrupt_time_us = 0;
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
    sleep_management_debug_dump_lock_sleep_time();
#endif

}
#endif
