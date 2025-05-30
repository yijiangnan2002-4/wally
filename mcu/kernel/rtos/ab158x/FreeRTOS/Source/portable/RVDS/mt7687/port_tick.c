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

#include "FreeRTOS.h"
#include "hal_platform.h"

#if configUSE_TICKLESS_IDLE == 2
#include "task.h"
#include "port_tick.h"
#include "hal_gpt.h"
#include "gpt.h"
#include "timer.h"
#include "hal_log.h"
#include "type_def.h"
#include "hal_lp.h"
#include "core_cm4.h"
#include "top.h"
#include "connsys_driver.h"
#include "hal_sleep_driver.h"
#include <string.h>
#include "hal.h"
#include "timers.h"
#include "hal_rtc.h"
#endif /* configUSE_TICKLESS_IDLE == 2 */

#ifndef configSYSTICK_CLOCK_HZ
#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
/* Ensure the SysTick is clocked at the same frequency as the core. */
#define portNVIC_SYSTICK_CLK_BIT    ( 1UL << 2UL )
#else
/* The way the SysTick is clocked is not modified in case it is not the same
as the core. */
#define portNVIC_SYSTICK_CLK_BIT    ( 0 )
#endif


#if configUSE_TICKLESS_IDLE != 0
static uint32_t ulTimerCountsForOneTick = 0;
static uint32_t ulStoppedTimerCompensation = 0;
static uint32_t xMaximumPossibleSuppressedTicks = 0;
/* A fiddle factor to estimate the number of SysTick counts that would have
occurred while the SysTick counter is stopped during tickless idle
calculations. */
#define portMISSED_COUNTS_FACTOR                        ( 45UL )
#endif /* configUSE_TICKLESS_IDLE != 0 */

#if configUSE_TICKLESS_IDLE == 2
#define PlatformIdleOverhead        (2)
#define LegacySleepResumeOverhead   (5)  // mt7687 worst case is 5ms (ext 32k/N9 sleep/CM4 sleep)
#define LegacySleepCriteria         (10)

void tickless_log_timestamp(void);

extern hal_gpt_port_t wakeup_gpt_port;

unsigned char AST_TimeOut_flag = 1;

#define AST_CLOCK 32768 //32.768 kHz
#define xMaximumPossibleASTTicks (0xFFFFFFFF / (AST_CLOCK/configTICK_RATE_HZ))

#define TICKLESS_DEBUG                          0
#define RTC_CALIBRATION_ENABLE                  0
#define GIVE_OWN_BIT                            1
#define TICKLESS_TIME_ACCURACY_DEBUG            0
#define TICKLESS_WAKE_UP_FREQUENCY_DEBUG        1

#if TICKLESS_DEBUG == 1
uint32_t bAbort = 0;
uint32_t workaround = 0;
#endif

#if RTC_CALIBRATION_ENABLE
uint32_t rtc_clock;
uint32_t rtc_clock_now;
#endif

#if TICKLESS_WAKE_UP_FREQUENCY_DEBUG
uint32_t pre_wakeup_gpttick = 0;
uint32_t cur_wakeup_gpttick = 0;
uint32_t busy_wakeup_num    = 0;
uint32_t show_wic_info_num  = 0;
#endif

volatile uint8_t ticklessStatus = 0;


uint32_t ticklessLSCount   = 0;
uint32_t ticklessWFICount  = 0;
uint32_t ticklessIdleCount = 0;
uint32_t ticklessTryCount  = 0;
extern void (*ptr_tickless_cb)(void*);

uint32_t ulAST_Reload_ms = 0;
uint32_t ulSleepTickPeriods;
uint32_t ulReloadValue;
uint32_t ulCompletedCountDecrements;
uint32_t ulCompletedTickDecrements;
uint32_t ulPassedSystick;

static uint32_t TimeStampCounter;
static uint32_t TimeStampSystick;

float RTC_Freq = 32.768f;

#if TICKLESS_TIME_ACCURACY_DEBUG
#define TICKLESS_DEBUG_TICKS  1000
extern uint32_t ulAST_Current_ms;
extern uint32_t ulReloadValue;
extern uint32_t ulCompleteTickPeriods;
extern uint32_t ulCompletedSysTickDecrements;
extern uint32_t ulAST_CurrentCount;
static TimerHandle_t timer1 = NULL;
static TimerHandle_t timer2 = NULL;


static void tickless_debug_timer_callback(TimerHandle_t expiredTimer)
{
    printf("tick:%u ls:%u wfi:%u idle:%u, lock:%u\n",
        (unsigned int)xTaskGetTickCount(),
        ticklessLSCount,
        ticklessWFICount,
        ticklessIdleCount,
        hal_sleep_manager_get_lock_status());
}

static void tickless_dummy_debug_timer_callback(TimerHandle_t expiredTimer)
{
}
#endif


void AST_vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
#if RTC_CALIBRATION_ENABLE
    static bool calibration_done = false;
#endif

    TickType_t xModifiableIdleTime;
    uint32_t nowCount;
    uint32_t nowTick;
    hal_sleep_manager_status_t ret;
    hal_sleep_mode_t mode;

    ticklessStatus = 30;

#if RTC_CALIBRATION_ENABLE
    if (calibration_done == false) {
        hal_rtc_get_f32k_frequency(&rtc_clock_now);
        if (rtc_clock_now == rtc_clock) {
            calibration_done = true;
            RTC_Freq =  ((float)(rtc_clock)/1000);
            printf("calibration done, %u, %f\n", rtc_clock, RTC_Freq);
        }
        rtc_clock = rtc_clock_now; 
    }
#endif

    AST_TimeOut_flag = 0;

    //need ues AST
    /* Calculate the reload value required to wait xExpectedIdleTime
    tick periods.  -1 is used because this code will execute part way
    through one of the tick periods. */
    ulReloadValue = SysTick->VAL;
    if (ulReloadValue > ulStoppedTimerCompensation) {
        ulReloadValue -= ulStoppedTimerCompensation;
    }

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
    method as that will mask interrupts that should exit sleep mode. */
    __asm volatile("cpsid i");

    ticklessStatus = 31;

    /* If a context switch is pending or a task is waiting for the scheduler
    to be unsuspended then abandon the low power entry. */
    if (eTaskConfirmSleepModeStatus() == eAbortSleep) {
        ticklessStatus = 32;
#if TICKLESS_DEBUG == 1
        bAbort = 1;
#endif
        /* Restart from whatever is left in the count register to complete
        this tick period. */
        SysTick->LOAD = SysTick->VAL;

        /* Restart SysTick. */
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

        /* Reset the reload register to the value required for normal tick
        periods. */
        SysTick->LOAD = ulTimerCountsForOneTick - 1UL;

        /* Re-enable interrupts - see comments above the cpsid instruction()
        above. */
        __asm volatile("cpsie i");

        ticklessStatus = 33;

        return;
    } else {
        ticklessStatus = 35;

        //Calculate total idle time to ms
        ulAST_Reload_ms = ((xExpectedIdleTime - 1) * (1000 / configTICK_RATE_HZ)) - LegacySleepResumeOverhead;
        ret = hal_sleep_manager_set_sleep_time(ulAST_Reload_ms);

        if (ret == HAL_SLEEP_MANAGER_ERROR) {
            /* Restart from whatever is left in the count register to complete
            this tick period. */
            SysTick->LOAD = SysTick->VAL;
            /* Restart SysTick. */
            SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
            /* Reset the reload register to the value required for normal tick
            periods. */
            SysTick->LOAD = ulTimerCountsForOneTick - 1UL;
            /* Re-enable interrupts - see comments above the cpsid instruction()
            above. */
            __asm volatile("cpsie i");
            printf("hal_sleep_manager_set_sleep_time error, abort sleep\n");
            assert(0);
            return;
        }

        ticklessStatus = 36;

        /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
        set its parameter to 0 to indicate that its implementation contains
        its own wait for interrupt or wait for event instruction, and so wfi
        should not be executed again.  However, the original expected idle
        time variable must remain unmodified, so a copy is taken. */
        xModifiableIdleTime = xExpectedIdleTime;

        configPRE_SLEEP_PROCESSING(xModifiableIdleTime);
        if (xModifiableIdleTime > 0) {
            ticklessStatus = 37;

#if GIVE_OWN_BIT
            /* Enable FW_OWN_BACK_INT interrupt */
            if (hal_lp_connsys_get_own_enable_int() != 0) {
                printf("hal_lp_connsys_get_own_enable_int fail\n");
            }

            /* Give connsys ownership to N9 */
            if (hal_lp_connsys_give_n9_own() != 0) {
                printf("hal_lp_connsys_give_n9_own fail\n");
            }
#endif

            ticklessStatus = 38;

#ifdef HAL_WDT_PROTECTION_ENABLED
            hal_wdt_feed(HAL_WDT_FEED_MAGIC);
#endif

            ticklessStatus = 39;

            mode = sleepdrv_get_sleep_mode();
            hal_sleep_manager_enter_sleep_mode(mode);
            ticklessStatus = 43;

            if (mode == HAL_SLEEP_MODE_LEGACY_SLEEP) {
                ticklessLSCount++;
            } else if (mode == HAL_SLEEP_MODE_SLEEP) {
                ticklessWFICount++;
            } else {
                printf("unknown sleep mode %d\n", mode);
            }

            ticklessStatus = 44;

#ifdef HAL_WDT_PROTECTION_ENABLED
            hal_wdt_feed(HAL_WDT_FEED_MAGIC);
#endif

#if GIVE_OWN_BIT
            /* re-init connsys for handling inband-cmd response */
            if (hal_lp_connsys_get_ownership() != TRUE) {
                log_hal_info("connsys_get_ownership fail\n");
            }
#endif
            ticklessStatus = 45;
        }
        configPOST_SLEEP_PROCESSING(xExpectedIdleTime);

        ticklessStatus = 46;
        hal_gpt_stop_timer(wakeup_gpt_port);
        ticklessStatus = 47;

        //calculate time(systick) to jump
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &nowCount);

#if TICKLESS_WAKE_UP_FREQUENCY_DEBUG
        if (busy_wakeup_num > 5) {
            show_wic_info_num = 3;
        }

        if (show_wic_info_num > 0) {
            printf("WIC status: 0x%x 0x%x 0x%x\n", *((volatile unsigned int*)0x8300C334),
                                                   *((volatile unsigned int*)0x8300C338),
                                                   *((volatile unsigned int*)0x8300C33C));
            printf("WIC Mask:   0x%x 0x%x 0x%x\n", *((volatile unsigned int*)0x8300C348),
                                                   *((volatile unsigned int*)0x8300C34C),
                                                   *((volatile unsigned int*)0x8300C350));
            show_wic_info_num--;
            printf("busy_wakeup_num %u, show_wic_info_num %u\n", (unsigned int)busy_wakeup_num, (unsigned int)show_wic_info_num);
        }

        cur_wakeup_gpttick = nowCount;
        if ((sleepdrv_get_sleep_mode() != HAL_SLEEP_MODE_SLEEP) && 
            ((cur_wakeup_gpttick - pre_wakeup_gpttick) < 100)) { // less than about 3ms
            busy_wakeup_num++;
        } else {
            busy_wakeup_num = 0;
        }
        pre_wakeup_gpttick = cur_wakeup_gpttick;
#endif

        ticklessStatus = 48;
        nowTick = (uint32_t)xTaskGetTickCount();
        // get counter distance from last record
        if (nowCount >= TimeStampCounter) {
            ulCompletedCountDecrements = nowCount - TimeStampCounter;
        } else {
            ulCompletedCountDecrements = nowCount + (0xFFFFFFFF - TimeStampCounter);
        }
        // get systick distance from last record
        if (nowTick >= TimeStampSystick) {
            ulCompletedTickDecrements = nowTick - TimeStampSystick;
        } else {
            ulCompletedTickDecrements = nowTick + (0xFFFFFFFF - TimeStampSystick);
        }

        // get systick distance for this sleep
        ulPassedSystick = (uint32_t)(((float)ulCompletedCountDecrements)/RTC_Freq);
        // calculate ticks for jumpping
        ulSleepTickPeriods = ulPassedSystick - ulCompletedTickDecrements;

        //Limit OS Tick Compensation Value
        if (ulSleepTickPeriods > (xExpectedIdleTime - 1)) {
            ulSleepTickPeriods = xExpectedIdleTime - 1;
        }

        ticklessStatus = 49;

        SysTick->LOAD = ulReloadValue;
        SysTick->VAL = 0UL;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
        vTaskStepTick(ulSleepTickPeriods);
        SysTick->LOAD = ulTimerCountsForOneTick - 1UL;

        ticklessStatus = 50;
        __asm volatile("cpsie i");
    }

    ticklessStatus = 51;

#if TICKLESS_DEBUG == 1
    printf("xExpectedIdleTime = %d ms\n", xExpectedIdleTime);
    printf("bAbort = %d\n", bAbort);
    printf("workaround = %d\n", workaround);
    printf("AST_TimeOut_flag = %d\n", AST_TimeOut_flag);
    printf("ulAST_Reload_ms = %d ms\n", ulAST_Reload_ms);
    printf("ulSleepTickPeriods = %d ms\n", ulSleepTickPeriods);
    printf("ulReloadValue = %u\n\n", ulReloadValue);
    bAbort = 0;
    workaround = 0;
#endif
}

void tickless_GPT_CB(void* data)
{
    AST_TimeOut_flag = 1;
}

static void tickless_log_timestamp_callback(TimerHandle_t expiredTimer)
{
    tickless_log_timestamp();
}

void tickless_init()
{
    TimerHandle_t timer = NULL;
    ptr_tickless_cb = tickless_GPT_CB;

    tickless_log_timestamp();

    timer = xTimerCreate("tickless_log_timestamp_timer",
                         1000*60*60*12,  //12hours
                         true,
                         NULL,
                         tickless_log_timestamp_callback);

    if (timer == NULL) {
        printf("timer create fail\n");
    } else {
        if (xTimerStart(timer, 0) != pdPASS) {
            printf("xTimerStart fail\n");
        }
    }

#if TICKLESS_TIME_ACCURACY_DEBUG
    timer1 = xTimerCreate("tickless_debug_timer",
                         TICKLESS_DEBUG_TICKS,
                         true,
                         NULL,
                         tickless_debug_timer_callback);

    if (timer1 == NULL) {
        printf("tickless_debug_timer create fail\n");
    } else {
        if (xTimerStart(timer1, 0) != pdPASS) {
            printf("tickless_debug_timer fail\n");
        } else {
            printf("tickless_debug_timer start\n");
        }
    }

#if 0
    timer2 = xTimerCreate("tickless_dummy_timer",
                         TICKLESS_DEBUG_TICKS/40,
                         true,
                         NULL,
                         tickless_dummy_debug_timer_callback);

    if (timer2 == NULL) {
        printf("tickless_dummy_timer create fail\n");
    } else {
        if (xTimerStart(timer2, 0) != pdPASS) {
            printf("tickless_dummy_timer fail\n");
        } else {
            printf("tickless_dummy_timer start\n");
        }
    }
#endif
#endif

#if 0
    uint8_t tickless_test_sleep_handle;
    tickless_test_sleep_handle = hal_sleep_manager_set_sleep_handle("tickless_test");
    printf("tickless_test_sleep_handle %d\n", tickless_test_sleep_handle);
    hal_sleep_manager_lock_sleep(tickless_test_sleep_handle);
#endif
}

void tickless_handler(TickType_t xExpectedIdleTime)
{
    uint32_t ulReloadValue, ulCompleteTickPeriods;
    TickType_t xModifiableIdleTime;
    static long unsigned int before_sleep_time, after_sleep_time, sleep_time, SystickCompensation;
    static uint32_t StoppedSystickCompensation;
    hal_sleep_manager_status_t ret;

    if (sleepdrv_get_sleep_mode() == HAL_SLEEP_MODE_NONE) {
        return;
    }

    ticklessTryCount++;

    ticklessStatus = 1;

    /* Stop the SysTick momentarily.  The time the SysTick is stopped for
    is accounted for as best it can be, but using the tickless mode will
    inevitably result in some tiny drift of the time maintained by the
    kernel with respect to calendar time. */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&before_sleep_time);
    
    StoppedSystickCompensation = (SysTick->LOAD - SysTick->VAL);

#ifdef HAL_WDT_PROTECTION_ENABLED
    if (xExpectedIdleTime >= ((HAL_WDT_TIMEOUT_VALUE-1)*1000)) {
        xExpectedIdleTime = (HAL_WDT_TIMEOUT_VALUE-1)*1000;
    }
#endif

    /* Make sure the SysTick reload value does not overflow the counter. */
    if ((xExpectedIdleTime > LegacySleepCriteria) && (!hal_sleep_manager_is_sleep_locked())) {
        ticklessStatus = 2;
        //xExpectedIdleTime = xMaximumPossibleSuppressedTicks;

        /* Make sure the AST reload value does not overflow the counter. */
        if (xExpectedIdleTime > xMaximumPossibleASTTicks) {
            xExpectedIdleTime = xMaximumPossibleASTTicks;
        }

        //hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &time_before_sleep);
        AST_vPortSuppressTicksAndSleep(xExpectedIdleTime);
        //hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &time_after_sleep);
        //printf("%u, %u, %u, %u\n", time_before_sleep, time_after_sleep, time_after_sleep - time_before_sleep, ulAST_Reload_ms);
        ticklessStatus = 3;
        return;
    }

    ticklessStatus = 4;

    if (xExpectedIdleTime > xMaximumPossibleSuppressedTicks) {
        xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
    }

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
    method as that will mask interrupts that should exit sleep mode. */
    __asm volatile("cpsid i");

    ticklessStatus = 5;

    /* If a context switch is pending or a task is waiting for the scheduler
    to be unsuspended then abandon the low power entry. */
    if (eTaskConfirmSleepModeStatus() == eAbortSleep) {
        ticklessStatus = 6;
        /* Restart from whatever is left in the count register to complete
        this tick period. */
        SysTick->LOAD = SysTick->VAL;

        /* Restart SysTick. */
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

        /* Reset the reload register to the value required for normal tick
        periods. */
        SysTick->LOAD = ulTimerCountsForOneTick - 1UL;

        /* Re-enable interrupts - see comments above the cpsid instruction()
        above. */
        __asm volatile("cpsie i");
        ticklessStatus = 7;
    } else {
        ticklessStatus = 8;
        ulReloadValue = ((xExpectedIdleTime - 1UL) * (1000 / configTICK_RATE_HZ));
        ret = hal_sleep_manager_set_sleep_time((uint32_t)ulReloadValue);
        if (ret == HAL_SLEEP_MANAGER_ERROR) {
            printf("tickless: set_sleep_time %u error in idle\n", (unsigned int)ulReloadValue);
            /* Restart from whatever is left in the count register to complete
            this tick period. */
            SysTick->LOAD = SysTick->VAL;

            /* Restart SysTick. */
            SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

            /* Reset the reload register to the value required for normal tick
            periods. */
            SysTick->LOAD = ulTimerCountsForOneTick - 1UL;

            /* Re-enable interrupts - see comments above the cpsid instruction()
            above. */
            __asm volatile("cpsie i");

            assert(0);
            return;
        }
        ticklessStatus = 9;

        xModifiableIdleTime = xExpectedIdleTime;
        configPRE_SLEEP_PROCESSING(xModifiableIdleTime);

        if (xModifiableIdleTime > 0) {
            ticklessStatus = 10;

            HAL_REG_32(MCU_CFG_NVIC_BASE + 0xD10) &= ~(BIT(2));
            HAL_REG_32(TOP_CFG_CM4_PWR_CTL_CR)    &= ~(BIT(30));

            __asm volatile("dsb");

            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");

            __asm volatile("wfi");

            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");
            __asm volatile("nop");

            __asm volatile("isb");
        }
        ticklessStatus = 11;
        configPOST_SLEEP_PROCESSING(xExpectedIdleTime);
        ticklessIdleCount++;

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&after_sleep_time);
        ticklessStatus = 12;

        if (after_sleep_time >= before_sleep_time) {
            sleep_time = after_sleep_time - before_sleep_time;
        } else {
            sleep_time = after_sleep_time + (0xFFFFFFFF - before_sleep_time);
        }

        ulCompleteTickPeriods = (sleep_time / 1000) / (1000 / configTICK_RATE_HZ);

        sleep_time -= (ulCompleteTickPeriods * 1000) * (1000 / configTICK_RATE_HZ);
        sleep_time = (1000 * (1000 / configTICK_RATE_HZ)) - sleep_time;

        StoppedSystickCompensation = StoppedSystickCompensation / (configSYSTICK_CLOCK_HZ / 1000000);

        if (sleep_time >= (StoppedSystickCompensation)) {
            sleep_time = sleep_time - (StoppedSystickCompensation);
        } else {
            ulCompleteTickPeriods++;
            sleep_time = (1000 * (1000 / configTICK_RATE_HZ)) - ((StoppedSystickCompensation) - sleep_time);
        }

        if (sleep_time <= 5) {
            SystickCompensation = ulTimerCountsForOneTick - (((sleep_time) * ulTimerCountsForOneTick) / 1000 / (1000 / configTICK_RATE_HZ));
            ulCompleteTickPeriods++;
        } else {
            SystickCompensation = sleep_time * (ulTimerCountsForOneTick / 1000 / (1000 / configTICK_RATE_HZ));
        }

        //Limit OS Tick Compensation Value
        if (ulCompleteTickPeriods >= (xExpectedIdleTime)) {
            ulCompleteTickPeriods = xExpectedIdleTime;
        }        

        ticklessStatus = 13;

        SysTick->LOAD = SystickCompensation;
        SysTick->VAL = 0UL;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;        
        vTaskStepTick(ulCompleteTickPeriods);
        SysTick->LOAD = ulTimerCountsForOneTick - 1UL;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

        ticklessStatus = 14;
        /* Re-enable interrupts - see comments above the cpsid instruction() above. */
        __asm volatile("cpsie i");
        ticklessStatus = 15;
    }

    ticklessStatus = 16;
}

void tickless_log_timestamp()
{
#if RTC_CALIBRATION_ENABLE
    uint32_t rtc_clock;
#endif
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &TimeStampCounter);
    TimeStampSystick = (uint32_t)xTaskGetTickCount() * (1000 / configTICK_RATE_HZ);
#if RTC_CALIBRATION_ENABLE
    hal_rtc_get_f32k_frequency(&rtc_clock);
    RTC_Freq = ((float)(rtc_clock)/1000);
#endif
}
#endif /* configUSE_TICKLESS_IDLE == 2 */

void vPortSetupTimerInterrupt(void)
{
    /* Calculate the constants required to configure the tick interrupt. */
#if configUSE_TICKLESS_IDLE != 0
    {
        ulTimerCountsForOneTick = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ);
        xMaximumPossibleSuppressedTicks = SysTick_LOAD_RELOAD_Msk / ulTimerCountsForOneTick;
        ulStoppedTimerCompensation = portMISSED_COUNTS_FACTOR / (configCPU_CLOCK_HZ / configSYSTICK_CLOCK_HZ);
    }
#endif /* configUSE_TICKLESS_IDLE != 0 */

    /* Configure SysTick to interrupt at the requested rate. */
    SysTick->LOAD = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

