/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "exception_handler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "memory_attribute.h"
#include "at_command.h"
#include "syslog.h"
#include "task_def.h"
#include "os_port_callback.h"
#include "hal_wdt.h"
#include "hal_gpt.h"
#ifdef HAL_CCNI_MODULE_ENABLED
#include "hal_ccni.h"
#include "hal_ccni_config.h"
#endif
#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif
#define LOGE(fmt,arg...)   LOG_E(atcmd, "ATSS: "fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(atcmd, "ATSS: "fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(atcmd ,"ATSS: "fmt,##arg)
#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atcmd, "ATSS: "fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atcmd ,"ATSS: "fmt,cnt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atcmd ,"ATSS: "fmt,cnt,##arg)

#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
#include "hal_resource_assignment.h"
#define MAX_TASK_COUNT  32
typedef enum {
    TASK_PERIOD_CONFIG = 0xa0,
    TASK_THRESHOLD_CONFIG,
    TASK_INVAILD_CONFIG = 0xffffffff
} option_selector_t;

typedef struct {
    char task_name[8];
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
} core_task_info_t;

ATTR_SHARE_ZIDATA core_task_info_t dsp_task_info = {0};
ATTR_SHARE_ZIDATA core_task_info_t mcu_task_info = {0};
ATTR_SHARE_ZIDATA uint32_t core_enable_heap_leak_self_check;
ATTR_SHARE_ZIDATA hal_ccni_message_t hal_ccni_message;
#endif

/*--- Function ---*/
#if defined(MTK_SYSTEM_AT_COMMAND_ENABLE)
extern atci_status_t atci_cmd_hdlr_system(atci_parse_cmd_param_t *parse_cmd);
#endif /* MTK_SYSTEM_AT_COMMAND_ENABLE */
#if defined(MTK_OS_CPU_UTILIZATION_ENABLE)
extern atci_status_t atci_cmd_hdlr_utilization(atci_parse_cmd_param_t *parse_cmd);
#endif /* MTK_OS_CPU_UTILIZATION_ENABLE */
#if defined(MTK_SWLA_ENABLE)
extern atci_status_t atci_cmd_hdlr_swla(atci_parse_cmd_param_t *parse_cmd);
#endif /* MTK_SWLA_ENABLE */
/** <pre>
* <b>[Category]</b>
*    System Service
* <b>[Description]</b>
*    Show system information and trigger system reboot or crash
* <b>[Command]</b>
*    AT+SYSTEM=<module>
* <b>[Parameter]</b>
*    <module> string type
*    task: show all freeRTOS tasks information
*    mem: show heap status
*    crash: trigger system crash to dump system infomation
*    reboot: trigger system to hard-reboot
* <b>[Response]</b>
*    OK with data or ERROR;
* <b>[Example]</b>
* @code
*    Send:
*        AT+SYSTEM=task
*    Response:
*        +SYSTEM:
*        parameter meaning:
*        1: pcTaskName
*        2: cStatus(Ready/Blocked/Suspended/Deleted)
*        3: uxCurrentPriority
*        4: usStackHighWaterMark (unit is sizeof(StackType_t))
*        5: xTaskNumber
*        OK
*        +SYSTEM:
*        ATCI   X   5   594 2
*        IDLE   R   0   223 5
*        Tmr S  B   14  326 6
*        OK
* @endcode
* <b>[Note]</b>
*    MTK_SYSTEM_AT_COMMAND_ENABLE should be defined in y in project's feature.mk
* </pre>
*/

#if ( ( ( defined(MTK_SYSTEM_AT_COMMAND_ENABLE) ) && ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS == 1 ) ) || defined(MTK_OS_CPU_UTILIZATION_ENABLE) )
static void construct_profiling_info(const char *prefix, atci_response_t *presponse, char *task_list_buf)
{
    char *substr = NULL, *saveptr = NULL;
    int32_t pos = 0;

    substr = strtok_r(task_list_buf, "\n", &saveptr);
    pos = snprintf((char *)(presponse->response_buf), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "%s", prefix);
    pos += snprintf((char *)(presponse->response_buf + pos), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "%s\n", substr);
    while (substr) {
        /* LOGI("##substr: \r\n%s##\r\n",substr); */
        substr = strtok_r(NULL, "\n", &saveptr);
        if (substr == NULL) {
            /* on MT2625, 0x0 is protected by mpu with XN attribute, so cannot touch 0x0, even read operation */
            break;
        } else if ((pos + strlen(substr)) <= ATCI_UART_TX_FIFO_BUFFER_SIZE) {
            pos += snprintf((char *)(presponse->response_buf + pos),
                            ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                            "%s\n",
                            substr);
            /*LOGI("##buf: \r\n%s##\r\n",(char *)(presponse->response_buf)); */
        } else {
            /*LOGI("##buf: \r\n%s##\r\n",(char *)(presponse->response_buf)); */
            presponse->response_len = strlen((char *)(presponse->response_buf));
            presponse->response_flag |= ATCI_RESPONSE_FLAG_URC_FORMAT;
            atci_send_response(presponse);
            pos = 0;
            pos += snprintf((char *)(presponse->response_buf + pos),
                            ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                            "%s",
                            prefix);
            pos += snprintf((char *)(presponse->response_buf + pos),
                            ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                            "%s\n",
                            substr);
        }
    }
    if (strlen((char *)(presponse->response_buf)) != 0) {
        /*LOGI("##buf: \r\n%s##\r\n",(char *)(presponse->response_buf)); */
        presponse->response_len = strlen((char *)(presponse->response_buf));
        presponse->response_flag |= ATCI_RESPONSE_FLAG_URC_FORMAT;
        atci_send_response(presponse);
    }
}
#endif /* if ( ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS == 1 ) ) || defined(MTK_OS_CPU_UTILIZATION_ENABLE) ) */

static void stringToLower(char *pString)
{
    uint16_t index;
    uint16_t length = (uint16_t)strlen((char *)pString);

    for (index = 0; index <= length; index++) {
        pString[index] = (char)tolower((unsigned char)pString[index]);
    }
}

#if defined(MTK_SYSTEM_AT_COMMAND_ENABLE)

static void system_show_usage(uint8_t *buf)
{
    int32_t pos = 0;

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+SYSTEM:\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+SYSTEM=<module>\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(<module>=task|mem|crash|reboot)\r\n");
}

#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS == 1 ) )
static void system_show_task_info(atci_response_t *presponse)
{
    char *task_list_buf;
    int32_t buf_size = uxTaskGetNumberOfTasks() * (configMAX_TASK_NAME_LEN + 18);

    if ((task_list_buf = pvPortMalloc(buf_size)) == NULL) {
        strncpy((char *)(presponse->response_buf),
                "+SYSTEM:\r\nheap memory is not enough to do task information profiling\r\n",
                (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
        presponse->response_len = strlen((char *)(presponse->response_buf));
        presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(presponse);
        return;
    }

    strncpy((char *)(presponse->response_buf),
            "+SYSTEM:\r\nparameter meaning:\r\n1: pcTaskName\r\n2: cStatus(Ready/Blocked/Suspended/Deleted)\r\n3: uxCurrentPriority\r\n4: usStackHighWaterMark(unit is sizeof(StackType_t))\r\n5: xTaskNumber\r\n",
            (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
    presponse->response_len = strlen((char *)(presponse->response_buf));
    presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
    atci_send_response(presponse);

    vTaskList(task_list_buf);
    construct_profiling_info("+SYSTEM:\r\n", presponse, task_list_buf);

    vPortFree(task_list_buf);
}
#endif

static void system_query_mem(uint8_t *buf)
{
    int32_t pos = 0;
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+SYSTEM:\r\nheap information: \r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "\ttotal: %d\r\n",
                    configTOTAL_HEAP_SIZE);
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "\tcurrent free: %d\r\n",
                    xPortGetFreeHeapSize());
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "\tmini free: %d\r\n",
                    xPortGetMinimumEverFreeHeapSize());
}

/* AT command handler  */
atci_status_t atci_cmd_hdlr_system(atci_parse_cmd_param_t *parse_cmd)
{
    char *param = NULL, *saveptr = NULL;
    atci_response_t *presponse = pvPortMalloc(sizeof(atci_response_t));
    if (presponse == NULL) {
        //LOGMSGIDE("memory malloced failed.\r\n", 0);
        return ATCI_STATUS_ERROR;
    }

    memset(presponse, 0, sizeof(atci_response_t));
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    presponse->cmd_id = parse_cmd->cmd_id;
#endif /* ATCI_APB_PROXY_ADAPTER_ENABLE */

    /* To support both of uppercase and lowercase */
    stringToLower(parse_cmd->string_ptr);

    presponse->response_flag = 0; /* Command Execute Finish. */
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_READ: /* rec: AT+SYSTEM? */
        case ATCI_CMD_MODE_TESTING: /* rec: AT+SYSTEM=? */
            system_show_usage(presponse->response_buf);
            presponse->response_len = strlen((char *)(presponse->response_buf));
            presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(presponse);
            break;
        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+SYSTEM=<module> */
            param = parse_cmd->string_ptr + parse_cmd->parse_pos;
            param = strtok_r(param, "\n\r", &saveptr);
            //LOGI("##module=%s##\r\n", param);

            if (strcmp(param, "task") == 0) {
#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS == 1 ) )
                system_show_task_info(presponse);
#else
                strncpy((char *)(presponse->response_buf),
                        "+SYSTEM:\r\nplease define configUSE_TRACE_FACILITY = 1 and configUSE_STATS_FORMATTING_FUNCTIONS = 1 in project freeRTOSConfig.h \r\n",
                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                atci_send_response(presponse);
#endif
            } else if (strcmp(param, "mem") == 0) {
                system_query_mem(presponse->response_buf);
                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                atci_send_response(presponse);
            } else if (strcmp(param, "arm,crash") == 0) {
                strncpy((char *)(presponse->response_buf),
                        "+SYSTEM:\r\n system will crash...\r\n",
                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                atci_send_response(presponse);
                configASSERT(0);
            } else if (strcmp(param, "dsp,crash") == 0) {
                strncpy((char *)(presponse->response_buf),
                        "+SYSTEM:\r\n system will crash...\r\n",
                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

#ifdef HAL_CCNI_MODULE_ENABLED
#include "hal_ccni.h"
#include "hal_ccni_config.h"
                //hal_ccni_init();
                LOGMSGIDE("cm4 will send ccni to dsp!!!\r\n", 0);
                hal_ccni_status_t  hal_ccni_status = -1;
                //while(1){
                hal_ccni_status = hal_ccni_set_event(IRQGEN_CM4_TO_DSP0_EVENT9, NULL);
                if (hal_ccni_status != 0) {
                    LOGMSGIDE("hal_ccni_set_event failed!!!,hal_ccni_status = %d\r\n", 1,hal_ccni_status);
                }
                hal_gpt_delay_ms(100);
                // }
#endif
                atci_send_response(presponse);
                while (1);
            } else if (strcmp(param, "reboot") == 0) {
#if 0
                hal_wdt_config_t wdt_config;

                wdt_config.mode = HAL_WDT_MODE_RESET;
                wdt_config.seconds = 1;
                hal_wdt_init(&wdt_config);
                hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
#else
                (void)hal_wdt_software_reset();
#endif
                presponse->response_len = 0;
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                /* command syntax error */
                strncpy((char *)(presponse->response_buf),
                        "+SYSTEM:\r\ncommand syntax error\r\n",
                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                atci_send_response(presponse);
            }
            break;
        default :
            /* others are invalid command format */
            presponse->response_len = strlen((char *)(presponse->response_buf));
            presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(presponse);
            break;
    }

    vPortFree(presponse);
    return ATCI_STATUS_OK;
}
#endif /* MTK_SYSTEM_AT_COMMAND_ENABLE */
/*
AT+UTILIZATION=?
+UTILIZATION:
AT+UTILIZATION=<op> [,<tick,count>])
<op> string type
start: begin os profiling
stop: stop os profiling
duration: enable os profiling with in a period of time
[,<tick,count>] integer type
<tick> integer type, means the profiling duration, the unit is system tick, 1tick is 1ms, only needed when <op>=duration
<count> integer type, means the profiling counts, only needed when <op>=duration
*/

/* <pre>
* <b>[Category]</b>
*    System Service
* <b>[Description]</b>
*    Profile cpu utilization
* <b>[Command]</b>
*    AT+UTILIZATION=<op> [,<tick,count>])
* <b>[Parameter]</b>
*    <op> string type, must be 'start', 'stop' or 'duration'
*    <tick> integer type, means the profiling duration, the unit is system tick, 1tick is 1ms, only needed when <op>=duration
*    <count> integer type, means the profiling counts, only needed when <op>=duration
* <b>[Response]</b>
*    OK with profling data or ERROR;
* <b>[Example]</b>
* @code
*    Send:
*        AT+UTILIZATION= duration,5000,10
*    Response:
*        +UTILIZATION:
*        profiling begin, duration is 5000 tick, the tick's unit is 1/configTICK_RATE_HZ
*        OK
*        +UTILIZATION:
*        parameter meaning:
*        1: pcTaskName
*        2: count(unit is 32k of gpt)
*        3: ratio
*        +UTILIZATION:
*        CPU    0       <1%
*        IDLE   163943  99%
*        Tmr S  89      <1%
*        ATCI   19      <1%
* @endcode
* <b>[Note]</b>
*    MTK_OS_CPU_UTILIZATION_ENABLE should be defined in y in project's feature.mk
* </pre>
*/
#if defined(MTK_OS_CPU_UTILIZATION_ENABLE)
typedef enum {
    CPU_MEASURE_NOT_START = 0,
    CPU_MEASURE_START_MODE = 1,
    CPU_MEASURE_DURATION_MODE = 2,
} _cpu_measure_state_t;

static void utilization_show_usage(uint8_t *buf)
{
    int32_t pos = 0;

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+UTILIZATION:\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+UTILIZATION=<op> [,<tick,count>])\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(<op>=[start|stop|duration]\r\n<tick> means the profiling duration, the unit is system tick. <count> means the profiling counts, <tick> and <count> are only needed when <op>=duration)\r\n");
}

static TaskHandle_t xUtilTaskHandle = NULL;
static uint32_t _cpu_meausre_time;
static uint32_t _cpu_meausre_counts;
static uint32_t _cpu_meausre_mode = CPU_MEASURE_NOT_START;

uint32_t get_count_sleep_time_us(void);
uint32_t get_count_idle_time_us(void);
void tickless_start_count_idle_ratio(void);
void tickless_stop_count_idle_ratio(void);

void _cpu_utilization_task(void *arg)
{
    char *task_list_buf = NULL;
    atci_response_t *presponse = NULL;
    int32_t buf_size = uxTaskGetNumberOfTasks() * (configMAX_TASK_NAME_LEN + 20);
    TickType_t xLastWakeTime;
#if (configUSE_TICKLESS_IDLE == 2) && (defined(AIR_BTA_IC_PREMIUM_G2))
    uint32_t pos = 0;
    uint32_t start_time = 0;
    uint32_t stop_time = 0;
    float duration = 0;
    float sleep_percentage = 0;
    float idle_percentage = 0;
    float busy_percentage = 0;
#endif

    if ((presponse = pvPortMalloc(sizeof(atci_response_t))) != NULL) {
        memset(presponse, 0, sizeof(atci_response_t));

        if ((task_list_buf = pvPortMalloc(buf_size)) == NULL) {
            //LOGMSGIDE("memory malloced failed, and do cpu utilization \r\n", 0);
            strncpy((char *)(presponse->response_buf),
                    "+UTILIZATION:\r\nheap memory is not enough to do cpu utilization\r\n Error\r\n",
                    ATCI_UART_TX_FIFO_BUFFER_SIZE);
            presponse->response_len = strlen((char *)(presponse->response_buf));
            presponse->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
            atci_send_response(presponse);
        } else {
            while (_cpu_meausre_counts) {
                /* none risk condition here, because utilization task priority is always higher than atcmd task */
                _cpu_meausre_counts--;

                xLastWakeTime = xTaskGetTickCount();
                vConfigureTimerForRunTimeStats();
                vTaskClearTaskRunTimeCounter();

                if (_cpu_meausre_time > 0) {
                    //vTaskDelay(_cpu_meausre_time); /* duration mode */
                    vTaskDelayUntil(&xLastWakeTime, _cpu_meausre_time);  /* duration mode */
                    if (!_cpu_meausre_counts) {
                        /* profiling is ended by at+utilization = stop */
                        break;
                    }
                } else {
#if (configUSE_TICKLESS_IDLE == 2) &&  (defined(AIR_BTA_IC_PREMIUM_G2))
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &start_time);
                    tickless_start_count_idle_ratio();
#endif
                    vTaskSuspend(NULL); /* start/stop mode */
                }

                /* after resume */
                vTaskGetRunTimeStats(task_list_buf);

                /* Print idle time ratio */
#if (configUSE_TICKLESS_IDLE == 2) &&  (defined(AIR_BTA_IC_PREMIUM_G2))
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &stop_time);
                duration = ((unsigned int)(((double)(stop_time - start_time)) / 32.768)) / ((1000 / configTICK_RATE_HZ));
                idle_percentage = (float)get_count_idle_time_us() / 1000 / duration;
                sleep_percentage = (float)get_count_sleep_time_us() / 1000 / duration;
                busy_percentage = 1 - idle_percentage - sleep_percentage;
                tickless_stop_count_idle_ratio();

                pos += snprintf((char *)(presponse->response_buf + pos), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "[CM4 IDLE TIME]Duration: %lu\r\n", (uint32_t)duration);
                pos += snprintf((char *)(presponse->response_buf + pos), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "[CM4 IDLE TIME]Idle_time: %lu ms\r\n", (uint32_t)get_count_idle_time_us() / 1000);
                pos += snprintf((char *)(presponse->response_buf + pos), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "[CM4 IDLE TIME]Sleep_time: %lu ms\r\n", (uint32_t)get_count_sleep_time_us() / 1000);
                pos += snprintf((char *)(presponse->response_buf + pos), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "[CM4 IDLE TIME]Idle time percentage: [%lu]\r\n", (uint32_t)(idle_percentage * 100));
                pos += snprintf((char *)(presponse->response_buf + pos), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "[CM4 IDLE TIME]Sleep time percentage: [%lu]\r\n", (uint32_t)(sleep_percentage * 100));
                pos += snprintf((char *)(presponse->response_buf + pos), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "[CM4 IDLE TIME]Busy time percentage: [%lu]\r\n", (uint32_t)(busy_percentage * 100));
                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(presponse);
#endif

                strncpy((char *)(presponse->response_buf),
                        "+UTILIZATION:\r\nparameter meaning:\r\n1: pcTaskName\r\n2: count(unit is 32k of gpt)\r\n3: ratio\r\n",
                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);

                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(presponse);

                construct_profiling_info("+UTILIZATION:\r\n", presponse, task_list_buf);
            }

            vPortFree(task_list_buf);
            vPortFree(presponse);
            _cpu_meausre_mode = CPU_MEASURE_NOT_START;
            vTaskDelete(xTaskGetCurrentTaskHandle());
        }
    } else {
        //LOGMSGIDE("memory malloced failed, and do cpu utilization \r\n", 0);
        vTaskDelete(xTaskGetCurrentTaskHandle());
    }
    while (1);
}

static BaseType_t utilization_cpu_benchmark(uint32_t duration, uint32_t count)
{
    BaseType_t ret = pdPASS;
    _cpu_meausre_time = duration;
    _cpu_meausre_counts = count;
    ret = xTaskCreate(_cpu_utilization_task,
                      MTK_OS_CPU_UTILIZATION_TASK_NAME,
                      MTK_OS_CPU_UTILIZATION_STACKSIZE / sizeof(StackType_t),
                      NULL,
                      MTK_OS_CPU_UTILIZATION_PRIO,
                      &xUtilTaskHandle);
    return ret;
}

atci_status_t atci_cmd_hdlr_utilization(atci_parse_cmd_param_t *parse_cmd)
{
    char *op = NULL, *param = NULL, *saveptr = NULL;
    BaseType_t ret;
#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
    uint32_t idx = 0;
    uint32_t pos = 0;
#endif

    uint32_t tick = 0, counts = 0;

    atci_response_t *presponse = pvPortMalloc(sizeof(atci_response_t));
    if (presponse == NULL) {
       // LOGMSGIDE("memory malloced failed.\r\n", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(presponse, 0, sizeof(atci_response_t));
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    presponse->cmd_id = parse_cmd->cmd_id;
#endif /* ATCI_APB_PROXY_ADAPTER_ENABLE */

    /* To support both of uppercase and lowercase */
    stringToLower(parse_cmd->string_ptr);

    presponse->response_flag = 0; /* Command Execute Finish. */
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_READ: /* rec: AT+UTILIZATION?  */
        case ATCI_CMD_MODE_TESTING: /* rec: AT+UTILIZATION=? */
            utilization_show_usage(presponse->response_buf);
            presponse->response_len = strlen((char *)(presponse->response_buf));
            presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(presponse);
            break;
        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+UTILIZATION=<op> [,<tick,count>]) */
            op = parse_cmd->string_ptr + parse_cmd->parse_pos;
            op = strtok_r(op, "\n\r", &saveptr);
            op = strtok_r(op, ",", &saveptr);
            //LOGI("## op=%s ##\r\n", op);
            if (strcmp(op, "start") == 0) {
                /* start/stop mode */
                switch (_cpu_meausre_mode) {
                    case CPU_MEASURE_NOT_START:
                        _cpu_meausre_mode = CPU_MEASURE_START_MODE;
                        ret = utilization_cpu_benchmark(0, 1);
                        if (ret == pdPASS) {
                            strncpy((char *)(presponse->response_buf),
                                    "+UTILIZATION:\r\ncpu utilization profing begin, please use AT+UTILIZATION=stop to end profiling...\r\n",
                                    (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                            presponse->response_len = strlen((char *)(presponse->response_buf));
                            presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                            atci_send_response(presponse);
                        } else {
                            //LOGMSGIDE("memory malloced failed, and cannot create profiling task\r\n", 0);
                            strncpy((char *)(presponse->response_buf),
                                    "+UTILIZATION:\r\nheap memory is not enough to do cpu utilization\r\n",
                                    (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                            presponse->response_len = strlen((char *)(presponse->response_buf));
                            presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                            atci_send_response(presponse);
                            _cpu_meausre_mode = CPU_MEASURE_NOT_START;
                        }
                        break;
                    case CPU_MEASURE_START_MODE:
                    case CPU_MEASURE_DURATION_MODE:
                        strncpy((char *)(presponse->response_buf),
                                "+UTILIZATION:\r\nutilization profiling already started, please use AT+UTILIZATION=stop to stop profiling\r\n",
                                (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                        presponse->response_len = strlen((char *)(presponse->response_buf));
                        presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                        atci_send_response(presponse);
                        break;
                }
            }
#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
#ifdef AIR_CPU_MCPS_PRIORING_ENABLE
            else if (strcmp(op, "mcu_period") == 0) {
                param = strtok_r(NULL, ",", &saveptr);
                #include "timers.h"
                extern TimerHandle_t xTimerofTest;
                if (param != NULL) {
                    if (strcmp("enable", param) == 0) {
                        xTimerStart(xTimerofTest, 0);
                    }else if (strcmp("disable", param) == 0) {
                        xTimerStop(xTimerofTest, 0);
                    } else{
                        uint32_t period = atoi(param);
                        if((period >= 1000) && (period <= 60000)){
                            xTimerStop(xTimerofTest, 0);
                            xTimerChangePeriod(xTimerofTest,period,0);
                        }else{
                            strncpy((char *)(presponse->response_buf),
                                "+UTILIZATION:\r\nPlease set correct argument!!!\r\n",
                                (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                            presponse->response_len = strlen((char *)(presponse->response_buf));
                            presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                            atci_send_response(presponse);
                            vPortFree(presponse);
							return ATCI_STATUS_OK;
                        }
                    }
                    presponse->response_len = 1;
                    presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                    atci_send_response(presponse);
                }
            }
            else if (strcmp(op, "mcu_get_task_config") == 0) {

                for (idx = 0; idx < MAX_TASK_COUNT; idx++) {
                    if (mcu_task_info.task_info[idx].task_name[0]
                        && mcu_task_info.task_info[idx].task_name[1]
                        && mcu_task_info.task_info[idx].task_name[2]){
                        pos += snprintf((char *)(presponse->response_buf + pos), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                                    "number:%d,%s\r\n",
                                    (int)idx,
                                    mcu_task_info.task_info[idx].task_name);
                        }
                }

                presponse->response_len =  strlen((char *)(presponse->response_buf));
                presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                atci_send_response(presponse);
                break;
            }
#endif 
            else if (strcmp(op, "dsp_period") == 0) {
                param = strtok_r(NULL, ",", &saveptr);

                if (param != NULL) {
                    if (strcmp("enable", param) == 0) {
                        dsp_task_info.isEnableTaskInforPeriod = TRUE;
                    } else if (strcmp("disable", param) == 0) {
                        dsp_task_info.isEnableTaskInforPeriod = FALSE;
                    } else {
                        dsp_task_info.task_information_period = atoi(param);
                        dsp_task_info.option_selector = TASK_PERIOD_CONFIG;
                        dsp_task_info.isEnableTaskInforPeriod = TRUE;

                    }
                    if (((100 <= dsp_task_info.task_information_period) && (6000 >= dsp_task_info.task_information_period) )
                        || (dsp_task_info.option_selector != TASK_PERIOD_CONFIG)) {
                        hal_ccni_message.ccni_message[0] =  hal_memview_mcu_to_infrasys((uint32_t)&dsp_task_info);

                        if (0 != hal_ccni_set_event(CCNI_CM4_TO_DSP0_CONFIG_PROFILING, &hal_ccni_message)) {
                            LOGMSGIDE("mcu send ccni to dsp fail!!!\r\n", 0);
                        }
                    } else {
                        strncpy((char *)(presponse->response_buf),
                                "+UTILIZATION:\r\nPlease set correct task period first!!!\r\n",
                                (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                        presponse->response_len = strlen((char *)(presponse->response_buf));
                        presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                        atci_send_response(presponse);
                        break;

                    }
                    presponse->response_len = 1;
                    presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                    atci_send_response(presponse);
                } else {
                    strncpy((char *)(presponse->response_buf),
                            "+UTILIZATION:\r\n AT+UTILIZATION = dsp_period,<op>!!!\r\n",
                            (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                    presponse->response_len = strlen((char *)(presponse->response_buf));
                    presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    atci_send_response(presponse);
                    break;
                }
            } else if (strcmp(op, "task_select") == 0) {
                param = strtok_r(NULL, ",", &saveptr);
                if (param != NULL) {
                    if (strcmp(param, "all") == 0) {
                        param = strtok_r(NULL, ",", &saveptr);
                        if ((param != NULL) && (strcmp("y", param) == 0 )) {
                            for (uint32_t i = 0; i < MAX_TASK_COUNT; i++) {
                                dsp_task_info.option_task_info.task_number = 0xa5a5a5a5;
                                dsp_task_info.task_info[i].isSelect = 1;
                            }
                        } else if ((param != NULL) && (strcmp("n", param) == 0) ) {
                            for (uint32_t i = 0; i < MAX_TASK_COUNT; i++) {
                                dsp_task_info.option_task_info.task_number = 0x5a5a5a5a;
                                dsp_task_info.task_info[i].isSelect = 0;
                            }
                        } else {

                        }
                    } else {
                        dsp_task_info.option_task_info.task_number = atoi(param);
                        if (dsp_task_info.option_task_info.task_number >= MAX_TASK_COUNT) {
                            strncpy((char *)(presponse->response_buf),
                                    "+UTILIZATION:\r\n Please input correct task index(must < 32)!!!\r\n",
                                    (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                            presponse->response_len = strlen((char *)(presponse->response_buf));
                            presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                            atci_send_response(presponse);
                            break;
                        }

                        param = strtok_r(NULL, ",", &saveptr);
                        if (strcmp("y", param) == 0 ) {
                            dsp_task_info.option_task_info.isSelect = 1;
                            dsp_task_info.task_info[dsp_task_info.option_task_info.task_number].isSelect = 1;
                        } else {
                            dsp_task_info.option_task_info.isSelect = 0;
                            dsp_task_info.task_info[dsp_task_info.option_task_info.task_number].isSelect = 0;
                        }
                    }
                    /* send ccni to dsp */
                    hal_ccni_message.ccni_message[0] =  hal_memview_mcu_to_infrasys((uint32_t)&dsp_task_info);
                    if (0 != hal_ccni_set_event(CCNI_CM4_TO_DSP0_CONFIG_PROFILING, &hal_ccni_message)) {
                        LOGMSGIDE("mcu send ccni to dsp fail!!!\r\n", 0);
                    }
                    presponse->response_len = 1;
                    presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                    atci_send_response(presponse);

                }
            } else if (strcmp(op, "task_threshold") == 0) {
                param = strtok_r(NULL, ",", &saveptr);
                if (param != NULL) {
                    /* param is enable task threshold feature */
                    if (strcmp("enable", param) == 0 ) {
                        dsp_task_info.isEnableTaskThreshold = TRUE;
                    } else if (strcmp("disable", param) == 0) {
                        /* param is disable task threshold feature */
                        dsp_task_info.isEnableTaskThreshold = FALSE;
                    } else {
                        /* you need enable task threshold before you wanto set param */
                        idx = atoi(param);
                        if (idx >= MAX_TASK_COUNT) {
                            strncpy((char *)(presponse->response_buf),
                                    "+UTILIZATION:\r\ntask index need < 32\r\n",
                                    (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                            presponse->response_len = strlen((char *)(presponse->response_buf));
                            presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                            atci_send_response(presponse);
                            break;
                        }
                        param = strtok_r(NULL, ",", &saveptr);
                        dsp_task_info.task_info[idx].task_threshold = atoi(param);
                        dsp_task_info.option_selector = TASK_THRESHOLD_CONFIG;
                        dsp_task_info.isEnableTaskThreshold = TRUE;
                        dsp_task_info.isEnableTaskInforPeriod = TRUE;
                    }
                    /* send ccni to dsp */
                    hal_ccni_message.ccni_message[0] =  hal_memview_mcu_to_infrasys((uint32_t)&dsp_task_info);
                    if (0 != hal_ccni_set_event(CCNI_CM4_TO_DSP0_CONFIG_PROFILING, &hal_ccni_message)) {
                         LOGMSGIDE("mcu send ccni to dsp fail!!!\r\n", 0);
                    }

                    presponse->response_len = 1;
                    presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                    atci_send_response(presponse);
                }
            } else if (strcmp(op, "task_maxtime") == 0) {

                param = strtok_r(NULL, ",", &saveptr);
                if (param != NULL) {
                    /* param is enable task threshold feature */
                    if (strcmp("enable", param) == 0 ) {
                        dsp_task_info.TaskMaxTime = TASK_MAXTIME_ENABLE;
                        dsp_task_info.isEnableTaskInforPeriod = TRUE;
                    } else if (strcmp("disable", param) == 0) {
                        /* param is disable task threshold feature */
                        dsp_task_info.TaskMaxTime = TASK_MAXTIME_DISABLE;
                    } else if (strcmp("clear", param) == 0) {
                        if ( dsp_task_info.TaskMaxTime != TASK_MAXTIME_ENABLE) {
                            strncpy((char *)(presponse->response_buf),
                                    "+UTILIZATION:\r\nplease enable task maxtime first\r\n",
                                    (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                            presponse->response_len = strlen((char *)(presponse->response_buf));
                            presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                            atci_send_response(presponse);
                            break;
                        }
                        /* param is disable task threshold feature */
                        dsp_task_info.TaskMaxTime = TASK_MAXTIME_CLEAR;
                    } else {
                        strncpy((char *)(presponse->response_buf),
                                "+UTILIZATION:\r\ninvaild param\r\n",
                                (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                        presponse->response_len = strlen((char *)(presponse->response_buf));
                        presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                        atci_send_response(presponse);
                    }

                    hal_ccni_message.ccni_message[0] =  hal_memview_mcu_to_infrasys((uint32_t)&dsp_task_info);
                    if (0 != hal_ccni_set_event(CCNI_CM4_TO_DSP0_CONFIG_PROFILING, &hal_ccni_message)) {
                        LOGMSGIDE("mcu send ccni to dsp fail!!!\r\n", 0);
                    }
                    presponse->response_len = 1;
                    presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                    atci_send_response(presponse);
                } else {
                    strncpy((char *)(presponse->response_buf),
                            "+UTILIZATION:\r\ninvaild param\r\n",
                            (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                    presponse->response_len = strlen((char *)(presponse->response_buf));
                    presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    atci_send_response(presponse);
                }
            } else if (strcmp(op, "dsp_get_task_config") == 0) {

                pos += snprintf((char *)(presponse->response_buf), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                                "+UTILIZATION:\r\nisEnableTaskInforPeriod:%d,task period:%dms\r\nisEnableTaskThreshold:%d\r\nisEnableTaskMaxTime:%d\r\n",
                                (int)dsp_task_info.isEnableTaskInforPeriod, (int)dsp_task_info.task_information_period,
                                (int)dsp_task_info.isEnableTaskThreshold, (int)dsp_task_info.TaskMaxTime);

                for (idx = 0; idx < MAX_TASK_COUNT; idx++) {
                    if (dsp_task_info.task_info[idx].task_name[0]
                        && dsp_task_info.task_info[idx].task_name[1]
                        && dsp_task_info.task_info[idx].task_name[2]){
                        pos += snprintf((char *)(presponse->response_buf + pos), (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                                    "number:%d,%s,threshold:%dms,isSelect:%d\r\n",
                                    (int)idx,
                                    dsp_task_info.task_info[idx].task_name,
                                    (int)dsp_task_info.task_info[idx].task_threshold,
                                    (int)dsp_task_info.task_info[idx].isSelect);
                        }
                }

                presponse->response_len =  strlen((char *)(presponse->response_buf));
                presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                atci_send_response(presponse);
                break;
            }
            else if (strcmp(op, "heap_self_check") == 0) {  
                param = strtok_r(NULL, ",", &saveptr);
                extern uint32_t g_tmp_enable_heak_leak;
                if (param != NULL) {
                    if (strcmp("enable", param) == 0) {
                        g_tmp_enable_heak_leak = 1;    
                        core_enable_heap_leak_self_check  = 1;                      
                    } else if (strcmp("disable", param) == 0) {
                        g_tmp_enable_heak_leak = 0;
                        core_enable_heap_leak_self_check = 0;
                    } else {
                        strncpy((char *)(presponse->response_buf),
                        "+UTILIZATION:\r\n command syntax error\r\n",
                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                        presponse->response_len = strlen((char *)(presponse->response_buf));
                        atci_send_response(presponse);
                        break;
                    }  
                    /* send ccni to dsp */
                    hal_ccni_message.ccni_message[0] =  hal_memview_mcu_to_infrasys((uint32_t)&core_enable_heap_leak_self_check);
                    if (0 != hal_ccni_set_event(CCNI_CM4_TO_DSP0_CONFIG_PROFILING, &hal_ccni_message)) {
                         LOGMSGIDE("mcu send ccni to dsp fail!!!\r\n", 0);
                    }                    
                    presponse->response_len = 1;
                    presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                    atci_send_response(presponse);
                }
            }
#endif
            else if (strcmp(op, "duration") == 0) {
                /* duration mode */
                //LOGI("1## param=%s ##\r\n", saveptr);
                param = strtok_r(NULL, ",", &saveptr);
                if (param != NULL) {
                    //LOGI("2## param=%s ##\r\n", param);
                    tick = atoi(param);
                    param = strtok_r(NULL, ",", &saveptr);
                    if (param != NULL) {
                        //LOGI("3## param=%s ##\r\n", param);
                        counts = atoi(param);
                    }
                }

                switch (_cpu_meausre_mode) {
                    case CPU_MEASURE_NOT_START:
                        /* param validity check */
                        if ((tick <= 0) || (counts <= 0)) {
                            strncpy((char *)(presponse->response_buf),
                                    "+UTILIZATION:\r\nparam must be positive integer if <op>=duration, which means profiling duration and profiling counts\r\n",
                                    (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                            presponse->response_len = strlen((char *)(presponse->response_buf));
                            presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                            atci_send_response(presponse);
                        } else {
                            /* start profiling */
                            _cpu_meausre_mode = CPU_MEASURE_DURATION_MODE;
                            ret = utilization_cpu_benchmark(tick, counts); /* duration mode */
                            if (ret == pdPASS) {
                                snprintf((char *)(presponse->response_buf),
                                         ATCI_UART_TX_FIFO_BUFFER_SIZE,
                                         "+UTILIZATION:\r\n profiling begin, duration is %d tick, and profiling %d counts, the tick's unit is 1/configTICK_RATE_HZ\r\n",
                                         (int)tick, (int)counts);
                                presponse->response_len = strlen((char *)(presponse->response_buf));
                                presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                                atci_send_response(presponse);
                            } else {
                                strncpy((char *)(presponse->response_buf),
                                        "+UTILIZATION:\r\nheap memory is not enough to do cpu utilization\r\n",
                                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                                presponse->response_len = strlen((char *)(presponse->response_buf));
                                presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                                atci_send_response(presponse);
                                _cpu_meausre_mode = CPU_MEASURE_NOT_START;
                            }
                        }
                        break;
                    case CPU_MEASURE_START_MODE:
                    case CPU_MEASURE_DURATION_MODE:
                        /* profiling already begin by start mode, duration mode is invalid until profiling stop by AT+UTILIZATION=stop */
                        strncpy((char *)(presponse->response_buf),
                                "+UTILIZATION:\r\nprofiling already begin by AT+UTILIZATION=start, duration mode is invalid until profiling stoped by AT+UTILIZATION=stop\r\n",
                                (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                        presponse->response_len = strlen((char *)(presponse->response_buf));
                        presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                        atci_send_response(presponse);
                        break;
                }
            } else if (strcmp(op, "stop") == 0) {
                switch (_cpu_meausre_mode) {
                    case CPU_MEASURE_NOT_START:
                        strncpy((char *)(presponse->response_buf),
                                "+UTILIZATION:\r\nutilization profiling didn't start, please use AT+UTILIZATION=start or AT+UTILIZATION= duration,tick,count to start firstly\r\n",
                                (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                        presponse->response_len = strlen((char *)(presponse->response_buf));
                        presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                        atci_send_response(presponse);
                        break;
                    case CPU_MEASURE_START_MODE:
                        presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                        presponse->response_len = 0;
                        atci_send_response(presponse);
                        vTaskResume(xUtilTaskHandle);
                        _cpu_meausre_mode = CPU_MEASURE_NOT_START;
                        break;
                    case CPU_MEASURE_DURATION_MODE:
                        /* set the measure count to zero to end the profiling */
                        _cpu_meausre_counts = 0;
                        presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                        presponse->response_len = 0;
                        atci_send_response(presponse);
                        _cpu_meausre_mode = CPU_MEASURE_NOT_START;
                        break;
                }
            } else {
                /* command syntax error */
                strncpy((char *)(presponse->response_buf),
                        "+UTILIZATION:\r\n command syntax error\r\n",
                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                atci_send_response(presponse);
            }
            break;
        default :
            /* others are invalid command format */
            presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            presponse->response_len = strlen((char *)(presponse->response_buf));
            atci_send_response(presponse);
            break;
    }

    vPortFree(presponse);
    return ATCI_STATUS_OK;
}
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
atci_status_t atci_cmd_hdlr_dspperfmon(atci_parse_cmd_param_t *parse_cmd)
{
    char *op = NULL, *saveptr = NULL;

    atci_response_t *presponse = pvPortMalloc(sizeof(atci_response_t));
    if (presponse == NULL) {
       // LOGMSGIDE("memory malloced failed.\r\n", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(presponse, 0, sizeof(atci_response_t));
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    presponse->cmd_id = parse_cmd->cmd_id;
#endif /* ATCI_APB_PROXY_ADAPTER_ENABLE */

    /* To support both of uppercase and lowercase */
    stringToLower(parse_cmd->string_ptr);

    presponse->response_flag = 0; /* Command Execute Finish. */
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_READ: /* rec: AT+DSPPERFMON?  */
        case ATCI_CMD_MODE_TESTING: /* rec: AT+DSPPERFMON=? */
            utilization_show_usage(presponse->response_buf);
            presponse->response_len = strlen((char *)(presponse->response_buf));
            presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(presponse);
            break;
        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+DSPPERFMON=<op> */
            op = parse_cmd->string_ptr + parse_cmd->parse_pos;
            op = strtok_r(op, "\n\r", &saveptr);
            op = strtok_r(op, ",", &saveptr);
            if (strcmp("enable", op) == 0) {
                dsp_task_info.isEnablePerfMonitor = TRUE;
            } else if (strcmp("disable", op) == 0) {
                dsp_task_info.isEnablePerfMonitor = FALSE;
            } else {
                strncpy((char *)(presponse->response_buf),
                        "+DSPPERFMON:\r\n AT+DSPPERFMON=enable/disable\r\n",
                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                atci_send_response(presponse);
                break;
            }
            hal_ccni_message.ccni_message[0] =  hal_memview_mcu_to_infrasys((uint32_t)&dsp_task_info);
            if (0 != hal_ccni_set_event(CCNI_CM4_TO_DSP0_CONFIG_PROFILING, &hal_ccni_message)) {
                LOGMSGIDE("mcu send ccni to dsp fail!!!\r\n", 0);
            }
            presponse->response_len = 1;
            presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(presponse);
            break;
        default :
            /* others are invalid command format */
            presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            presponse->response_len = strlen((char *)(presponse->response_buf));
            atci_send_response(presponse);
            break;
    }
    vPortFree(presponse);
    return ATCI_STATUS_OK;
}
#endif /* AIR_BTA_IC_STEREO_HIGH_G3 */
#endif /* MTK_OS_CPU_UTILIZATION_ENABLE */

/*
AT+SWLA=?
+SWLA:
AT+SWLA=<op>
<op> string type
enable: enable SWLA
disable: disable SWLA
*/

/* <pre>
* <b>[Category]</b>
*    System Service
* <b>[Description]</b>
*    Enable or disable SWLA at runtime
* <b>[Command]</b>
*    AT+SWLA=<op>
* <b>[Parameter]</b>
*    <op> string type, must be 'enable' or 'disable'
* <b>[Response]</b>
*    OK or ERROR;
* <b>[Example]</b>
* @code
*    Send:
*        AT+SWLA= enable
*    Response:
*        OK
* @endcode
* <b>[Note]</b>
*    MTK_SWLA_ENABLE should be defined in y in project's feature.mk
* </pre>
*/
#ifdef MTK_SWLA_ENABLE
static void swla_show_usage(uint8_t *buf)
{
    int32_t pos = 0;

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+SWLA:\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+SWLA=<op>)\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(<op>=[enable|disable]\r\n");
}
/* AT command handler  */
atci_status_t atci_cmd_hdlr_swla(atci_parse_cmd_param_t *parse_cmd)
{
    char *param = NULL, *saveptr = NULL;
    atci_response_t *presponse = pvPortMalloc(sizeof(atci_response_t));
    if (presponse == NULL) {
        //LOGMSGIDE("memory malloced failed.\r\n", 0);
        return ATCI_STATUS_ERROR;
    }

    memset(presponse, 0, sizeof(atci_response_t));
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    presponse->cmd_id = parse_cmd->cmd_id;
#endif /* ATCI_APB_PROXY_ADAPTER_ENABLE */

    /* To support both of uppercase and lowercase */
    stringToLower(parse_cmd->string_ptr);

    presponse->response_flag = 0; /* Command Execute Finish. */
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_READ: /* rec: AT+SWLA? */
        case ATCI_CMD_MODE_TESTING: /* rec: AT+SWLA=? */
            swla_show_usage(presponse->response_buf);
            presponse->response_len = strlen((char *)(presponse->response_buf));
            presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(presponse);
            break;
        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+SWLA=<op> */
            param = parse_cmd->string_ptr + parse_cmd->parse_pos;
            param = strtok_r(param, "\n\r", &saveptr);
           // LOGI("##op=%s##\r\n", param);

            if (strcmp(param, "enable") == 0) {
                /* swla enable, will re-start profiling. */
                SLA_Control(SA_ENABLE);
                presponse->response_len = 0;
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (strcmp(param, "disable") == 0) {
                /* swla disable, will freeze swla profiling. */
                SLA_Control(SA_DISABLE);
                presponse->response_len = 0;
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                /* command syntax error */
                strncpy((char *)(presponse->response_buf),
                        "+SWLA:\r\ncommand syntax error\r\n",
                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            atci_send_response(presponse);
            break;
        default :
            /* others are invalid command format */
            presponse->response_len = strlen((char *)(presponse->response_buf));
            presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(presponse);
            break;
    }

    vPortFree(presponse);
    return ATCI_STATUS_OK;
}
#endif /* MTK_SWLA_ENABLE */