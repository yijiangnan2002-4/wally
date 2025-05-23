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

#ifndef __PORT_TICK_H__
#define __PORT_TICK_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define GPT_32K_SWITCH_TO_MS (1000*(1.0/32768))
#define MAX_TASK_COUNT  32
#define MAX_TASK_TIME_COUNT 32

#if configGENERATE_RUN_TIME_STATS == 1
typedef enum {
    TASK_PERIOD_CONFIG = 0xa0,
    TASK_THRESHOLD_CONFIG,
    TASK_INVAILD_CONFIG = 0xffffffff
} option_selector_t;

typedef struct {
    char task_name[configMAX_TASK_NAME_LEN];
    uint32_t task_threshold;
    uint32_t isSelect;
} task_info_t;

typedef enum {
    TASK_MAXTIME_DISABLE,
    TASK_MAXTIME_ENABLE,
    TASK_MAXTIME_CLEAR,
    TASK_INVAILD_ACTION = 0xffffffff
} option_task_maxtime_t;

typedef struct {
    uint32_t task_number;
    uint32_t isSelect;
    uint32_t threshold;
} option_task_info_t;

typedef struct {
    uint32_t task_information_period;
    task_info_t task_info[MAX_TASK_COUNT];
    option_task_info_t option_task_info;
    uint32_t isEnableTaskThreshold;
    uint32_t isEnableTaskInforPeriod;
    uint32_t isEnablePerfMonitor;
    option_task_maxtime_t TaskMaxTime;
    option_selector_t option_selector;
} dsp_task_info_t;
#endif 

#if configUSE_TICKLESS_IDLE == 2
void tickless_handler(uint32_t xExpectedIdleTime);
void AST_vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime);
#endif

uint32_t port_enter_pure_wfi_mode(void);

#endif /* !__IDLE_TASK_H__ */
