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

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <assert.h>
#include "exception_handler.h"

#include "hal_uart.h"
#include "hal_uart_internal.h"
#include "hal_nvic.h"
#include "hal_gpt_internal.h"
#include "hal_core_status.h"
#include "hal_hw_semaphore.h"

#include "syslog.h"
#include "syslog_port.h"
#include "syslog_nvkey_struct.h"

#include "mux.h"
#include "mux_port_common.h"

#include "serial_port_assignment.h"
#include "hal_resource_assignment.h"

/* nvdm for filter */
#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#include "nvdm_id_list.h"
#endif

/* 1-wire log feature fucntion */
#if defined (AIR_1WIRE_ENABLE)
#include "smchg_1wire.h"
#endif

#if defined(MTK_NVDM_ENABLE) && !defined(AG3335)
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif

#ifdef SYSTEM_DAEMON_TASK_ENABLE
#include "system_daemon.h"
#endif

#ifdef FREERTOS_ENABLE
#include "FreeRTOS.h"
#include "task.h"
#endif


/* create for self debug module */
log_create_module(syslog, PRINT_LEVEL_INFO);
/* create common debug module */
log_create_module(common, PRINT_LEVEL_INFO);
log_create_module(MPLOG, PRINT_LEVEL_WARNING);
log_create_module(printf, PRINT_LEVEL_INFO);

#define SYSLOG_I(fmt, arg...)               LOG_I(syslog, fmt, ##arg)
#define SYSLOG_W(fmt, arg...)               LOG_W(syslog, fmt, ##arg)
#define SYSLOG_E(fmt, arg...)               LOG_E(syslog, fmt, ##arg)
#define SYSLOG_MSGID_D(fmt, cnt, arg...)    LOG_MSGID_D(syslog, fmt, cnt, ##arg)
#define SYSLOG_MSGID_I(fmt, cnt, arg...)    LOG_MSGID_I(syslog, fmt, cnt, ##arg)
#define SYSLOG_MSGID_W(fmt, cnt, arg...)    LOG_MSGID_W(syslog, fmt, cnt, ##arg)
#define SYSLOG_MSGID_E(fmt, cnt, arg...)    LOG_MSGID_E(syslog, fmt, cnt, ##arg)

const uint32_t PORT_SYSLOG_SHARE_VARIABLE_BEGIN   = HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VAR_START;
const uint32_t PORT_SYSLOG_SDK_VERSION_BEGIN      = HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VERSION_VAR_START;
const uint32_t PORT_SYSLOG_BUILD_TIME_BEGIN       = HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_BUILD_TIME_VAR_START;
#ifndef AIR_UART1_EARLY_LOG_ENABLE
const uint32_t PORT_SYSLOG_INIT_STAGE_PORT        = CONFIG_SYSLOG_INIT_STAGE_PORT;
#else
const uint32_t PORT_SYSLOG_INIT_STAGE_PORT        = HAL_UART_1;
#endif
const uint32_t PORT_SYSLOG_RUNNING_STAGE_PORT     = CONFIG_SYSLOG_RUNNING_STAGE_PORT;

extern uint32_t mux_query_nest_count(mux_handle_t handle);

#define SYSLOG_PORT_UNUSED(a) (void)a

#ifdef AIR_SYSLOG_BUFFER_EXPAND_ENABLE
#define AIR_SYSLOG_TX_BUFFER_SIZE       (32 * 1024)
#define AIR_SYSLOG_RX_BUFFER_SIZE       (1024)
#else
#define AIR_SYSLOG_TX_BUFFER_SIZE       (16 * 1024)
#define AIR_SYSLOG_RX_BUFFER_SIZE       (1024)
#endif

#define PORT_SYSLOG_MAX_ONE_PLAIN_LOG_SIZE (256)

#ifndef AIR_BTA_IC_STEREO_HIGH_G3
/* init stage nest flag */
static bool g_syslog_user_init_contex;
/* os stage nest flag */
static bool g_syslog_user_os_nest_is_running;
static uint32_t g_syslog_user_task_number;
static uint32_t g_syslog_user_task_max_number;
static TaskHandle_t *p_syslog_user_task_context;
/* irq stage nest flag */
static uint32_t g_syslog_user_irq_context[(IRQ_NUMBER_MAX + 31) / 32];
#endif

const g_log_port_mapping_t g_log_port_mapping[] = {
    {MUX_UART_0,    "UART_0"},
    {MUX_UART_1,    "UART_1"},
    {MUX_UART_2,    "UART_2"},
    {MUX_USB_COM_1, "MUX_USB_COM_1/USB_CDC_1"},
    {MUX_USB_COM_2, "MUX_USB_COM_2/USB_CDC_2"},
    {MUX_AIRAPP_0,  "AIR_SPP"},
    {MUX_FLASH,     "MUX_FLASH"},
    {MUX_USB_COM_2, "MUX_USB_COM_2/USB_HID"},
};

/* hal uart internal API for */
extern UART_REGISTER_T *const g_uart_regbase[];
hal_uart_baudrate_t g_uart_baudrate = CONFIG_SYSLOG_BAUDRATE;
mux_port_t g_syslog_nvkey_port = MUX_PORT_END;

#if !defined(MTK_DEBUG_LEVEL_NONE)
ATTR_LOG_STRING_LIB syslog_001[] = LOG_INFO_PREFIX(syslog) "Logging:  PC logging tool command id = 0x%04x";
ATTR_LOG_STRING_LIB syslog_002[] = LOG_INFO_PREFIX(syslog) "filter_cpu_config_save fail";
ATTR_LOG_STRING_LIB syslog_003[] = LOG_INFO_PREFIX(syslog) "filter_module_config_save fail";
ATTR_LOG_STRING_LIB syslog_004[] = LOG_INFO_PREFIX(syslog) "pc tool command data length too long %d";
#else
ATTR_LOG_STRING_LIB syslog_001[] = "";
ATTR_LOG_STRING_LIB syslog_002[] = "";
ATTR_LOG_STRING_LIB syslog_003[] = "";
ATTR_LOG_STRING_LIB syslog_004[] = "";
#endif

#if !defined(MTK_DEBUG_LEVEL_NONE)
void syslog_port_log_msgid_info(const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(&LOG_CONTROL_BLOCK_SYMBOL(syslog), PRINT_LEVEL_INFO, message, arg_cnt, ap);
    va_end(ap);
}

void syslog_port_log_msgid_warning(const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(&LOG_CONTROL_BLOCK_SYMBOL(syslog), PRINT_LEVEL_WARNING, message, arg_cnt, ap);
    va_end(ap);
}

void syslog_port_log_msgid_error(const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(&LOG_CONTROL_BLOCK_SYMBOL(syslog), PRINT_LEVEL_ERROR, message, arg_cnt, ap);
    va_end(ap);
}
#else
void syslog_port_log_msgid_info(const char *message, uint32_t arg_cnt, ...) {}
void syslog_port_log_msgid_warning(const char *message, uint32_t arg_cnt, ...) {}
void syslog_port_log_msgid_error(const char *message, uint32_t arg_cnt, ...) {}
#endif

/* dummy function,
    when disable syslog, some local variable will cause unused error
*/
void syslog_port_dummy(const char *message, ...)
{
    (void)(message);
}

/* deal with early init stage, syslog is update from nvkey */
uint32_t syslog_port_query_running_port(void)
{
    if (g_syslog_nvkey_port != MUX_PORT_END) {
        return g_syslog_nvkey_port;
    }

    return PORT_SYSLOG_INIT_STAGE_PORT;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM uint32_t syslog_port_query_current_timestamp(void)
{
    uint32_t count = 0;
    uint64_t count64 = 0;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    count64 = ((uint64_t)count) * 1000 / 32768;

    return (uint32_t)count64;
}

uint32_t syslog_port_memview_dsp0_to_mcu(uint32_t address)
{
    return hal_memview_dsp0_to_mcu(address);
}

uint32_t syslog_port_query_uart_baudrate(void)
{
    return g_uart_baudrate;
}

bool syslog_port_uart_is_xoff(uint32_t port)
{
    if (port >= HAL_UART_MAX) {
        return false;
    }

    if ((g_uart_regbase[port]->MCR_UNION.MCR & UART_MCR_XOFF_STATUS_MASK) != 0) {
        return true;
    }

    return false;
}

void syslog_port_uart_disable_flowcontrol(uint32_t port)
{
    if (port < HAL_UART_MAX) {
        hal_uart_disable_flowcontrol(port);
    }
}

void syslog_port_uart_disable_irq(uint32_t port)
{
    if (port < HAL_UART_MAX) {
        uart_disable_irq(port);
    }
}

void syslog_port_uart_send_polling(uint32_t port, const uint8_t *data, uint32_t size)
{
    if (port < HAL_UART_MAX) {
        hal_uart_send_polling(port, data, size);
    }
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM int32_t syslog_port_nvic_save_and_set_interrupt_mask(uint32_t *mask)
{
    return hal_nvic_save_and_set_interrupt_mask(mask);
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM int32_t syslog_port_nvic_restore_interrupt_mask(uint32_t mask)
{
    return hal_nvic_restore_interrupt_mask(mask);
}

uint32_t syslog_port_parse_port_number(syslog_port_type_t port_type, uint8_t port_index)
{
    if (port_type == SYSLOG_PORT_TYPE_UART) {
        return MUX_UART_BEGIN + port_index;
    } else if (port_type == SYSLOG_PORT_TYPE_USB) {
        return MUX_USB_BEGIN + port_index;
    } else if (port_type == SYSLOG_PORT_TYPE_FLASH) {
        return MUX_FLASH_BEGIN + port_index;
    }
    return PORT_SYSLOG_RUNNING_STAGE_PORT;
}

/* memory free */
void *syslog_port_malloc(uint32_t size)
{
    return malloc(size);
}

/* memory malloc */
void syslog_port_free(void *pdata)
{
    free(pdata);
}

/* syslog setting read from flash */
bool syslog_port_read_setting(char *name, uint8_t *p_setting, uint32_t size)
{
#ifdef MTK_NVDM_ENABLE
    uint32_t max_size;
    nvkey_status_t nvkey_status;

    if ((name == NULL) || (p_setting ==NULL)) {
        return false;
    }

    max_size = size;
    if (strcmp(name, "cpu_filter") == 0) {
        nvkey_status = nvkey_read_data(NVID_PERI_LOG_SETTING, p_setting, &max_size);
        if ((nvkey_status != NVKEY_STATUS_OK) || (max_size != size)) {
            return false;
        }
    } else if (strcmp(name, "module_filter") == 0) {
        nvkey_status = nvkey_read_data(NVID_PERI_LOG_MODULE_SETTING, p_setting, &max_size);
        if ((nvkey_status != NVKEY_STATUS_OK) || (max_size != size)) {
            return false;
        }
    } else {
        nvkey_status = NVKEY_STATUS_INVALID_PARAMETER;
    }

    if (nvkey_status == NVKEY_STATUS_OK) {
        return true;
    }
#endif

    return false;
}

/* syslog setting write to flash */
bool syslog_port_save_setting(char *name, uint8_t *p_setting, uint32_t size)
{
#ifdef MTK_NVDM_ENABLE
    nvkey_status_t nvkey_status;

    if ((name == NULL) || (p_setting ==NULL)) {
        return false;
    }

    if (strcmp(name, "cpu_filter") == 0) {
        nvkey_status = nvkey_write_data(NVID_PERI_LOG_SETTING, p_setting, size);
    } else if (strcmp(name, "module_filter") == 0) {
        nvkey_status = nvkey_write_data(NVID_PERI_LOG_MODULE_SETTING, p_setting, size);
    } else {
        nvkey_status = NVKEY_STATUS_ERROR;
    }

    if (nvkey_status == NVKEY_STATUS_OK) {
        return true;
    }

#endif

    return false;
}

uint32_t syslog_port_receive(uint32_t handle, uint8_t *p_buf, uint32_t size)
{
    mux_buffer_t buffer;
    uint32_t len;
    buffer.p_buf = p_buf;
    buffer.buf_size = size;
    mux_rx(handle, &buffer, &len);
    return len;
}

bool syslog_port_cmd_event_is_vaild(uint8_t event)
{
    if (event != MUX_EVENT_READY_TO_READ) {
        return false;
    }

    return true;
}

bool syslog_port_is_flash_port(uint32_t port)
{
    if (port == MUX_FLASH) {
        return true;
    }

    return false;
}

bool syslog_port_exception_uart_dump(uint32_t port)
{
    if ((port == MUX_FLASH) || (port == MUX_AIRAPP_0)) {
        return true;
    }

    return false;
}

bool syslog_port_is_usb_port(uint32_t port)
{
    if ((port == MUX_USB_COM_1) || (port == MUX_USB_COM_2)) {
        return true;
    }

    return false;
}

void syslog_port_send_daemo_message(void)
{
#ifdef SYSTEM_DAEMON_TASK_ENABLE
    system_daemon_send_message(SYSTEM_DAEMON_ID_LOGGING_TO_FLASH, NULL);
#endif
}

void syslog_port_send_daemo_message_assert(void)
{
#ifdef SYSTEM_DAEMON_TASK_ENABLE
    system_daemon_send_message(SYSTEM_DAEMON_ID_LOGGING_TO_ASSERT, NULL);
#endif
}

/* TCM is mainly for Optimize log printing time */
uint32_t syslog_port_query_exception_number(void)
{
    return HAL_NVIC_QUERY_EXCEPTION_NUMBER;
}

/* TCM is mainly for Optimize log printing time */
uint32_t syslog_port_query_os_task_number(void)
{
#ifdef FREERTOS_ENABLE
    TaskStatus_t pxTaskStatus;
    if ((xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) && (port_xSchedulerRunning != false)) {
        vTaskGetInfo(xTaskGetCurrentTaskHandle(), &pxTaskStatus, pdFALSE, eInvalid);
        return pxTaskStatus.xTaskNumber;
    } else {
        return 0;
    }
#else
    return 0;
#endif

}

#ifndef AIR_BTA_IC_STEREO_HIGH_G3
bool port_syslog_anti_nest_check_begin(void)
{
    uint32_t i, j, cpu_irq_mask;
    TaskHandle_t task_context;

    hal_nvic_save_and_set_interrupt_mask(&cpu_irq_mask); // __get_BASEPRI() = 0x10

    if (port_xSchedulerRunning == false) {
        /* Init context */
        if (g_syslog_user_init_contex != false) {
            assert(0 && "Asserted by syslog init nest check");
        }
        g_syslog_user_init_contex = true;
    } else if (HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) {
        /* Init task context table */
        if ((g_syslog_user_os_nest_is_running == false) && (p_syslog_user_task_context == NULL)) {
            g_syslog_user_task_max_number = (uxTaskGetNumberOfTasks() + 6); // reserve 3
            p_syslog_user_task_context = syslog_port_malloc(sizeof(TaskHandle_t) * g_syslog_user_task_max_number);
            g_syslog_user_os_nest_is_running = true;
        }
        /* Task context */
        i = 0;
        j = 0;
        task_context = xTaskGetCurrentTaskHandle();
        do {
            if (p_syslog_user_task_context[i] == task_context) {
                assert(0 && "Asserted by syslog os nest check");
            }
            if (p_syslog_user_task_context[i] != NULL) {
                j++;
            }
            i++;
        } while (j < g_syslog_user_task_number);
        for (i = 0; i < g_syslog_user_task_max_number; i++) {
            if (p_syslog_user_task_context[i] == NULL) {
                p_syslog_user_task_context[i] = task_context;
                g_syslog_user_task_number++;
                break;
            }
        }
        assert(i < g_syslog_user_task_max_number);
    } else {
            /* IRQ context */
            if (g_syslog_user_irq_context[HAL_NVIC_QUERY_EXCEPTION_NUMBER / 32] & (1 << (HAL_NVIC_QUERY_EXCEPTION_NUMBER % 32))) {
                assert(0 && "Asserted by syslog irq nest check");
            }
            g_syslog_user_irq_context[HAL_NVIC_QUERY_EXCEPTION_NUMBER / 32] |= 1 << (HAL_NVIC_QUERY_EXCEPTION_NUMBER % 32);
    }

    hal_nvic_restore_interrupt_mask(cpu_irq_mask);

    return true;
}

void port_syslog_anti_nest_check_end(void)
{
    uint32_t i, cpu_irq_mask;
    TaskHandle_t task_context;

    hal_nvic_save_and_set_interrupt_mask(&cpu_irq_mask); // __get_BASEPRI() = 0x10

    if (port_xSchedulerRunning == false) {
        /* Init context */
        g_syslog_user_init_contex = false;
    } else if (HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) {
        /* Task context */
        task_context = xTaskGetCurrentTaskHandle();
        for (i = 0; i < g_syslog_user_task_max_number; i++) {
            if (p_syslog_user_task_context[i] == task_context) {
                p_syslog_user_task_context[i] = NULL;
                g_syslog_user_task_number--;
                break;
            }
        }
    } else {
        /* IRQ context */
        g_syslog_user_irq_context[HAL_NVIC_QUERY_EXCEPTION_NUMBER / 32] &= ~(1 << (HAL_NVIC_QUERY_EXCEPTION_NUMBER % 32));
    }

    hal_nvic_restore_interrupt_mask(cpu_irq_mask);
}
#else

bool port_syslog_anti_nest_check_begin(void)
{
    return true;
}

void port_syslog_anti_nest_check_end(void)
{
    ;
}

#endif

volatile uint32_t syslog_check_assert_flag = 0;

void syslog_port_enable_dis_irq_call_log_assert(void)
{
    syslog_check_assert_flag = 0x1;
    syslog_set_debug_flag();
    SYSLOG_MSGID_I("Logging: enable disable irq call syslog API assert by nvkey", 0);
}

/* TCM is mainly for Optimize log printing time */
uint32_t syslog_port_query_init_number(void)
{
    uint32_t log_int_level;

#ifdef FREERTOS_ENABLE
    /* between system init ~ enable irq call SVC start schedule */
    if (port_xSchedulerRunning == false) {
        return 0xFFFFFFF;
    }
    /* ignore system init stage disable irq */
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return 0xFFFFFFF;
    }
#endif

    /* syslog disable irq call check assert */
    log_int_level = __get_BASEPRI();

#ifndef AIR_BTA_IC_PREMIUM_G2
    if ((log_int_level == 0x10) && (syslog_check_assert_flag == 0x1)) {
        configASSERT(0 && "Asserted by syslog monitor, disable irq call syslog API");
    }
#endif

    return log_int_level;
}

uint32_t syslog_port_query_curent_stack_space(void)
{
#ifdef AIR_SYSLOG_STACK_MONITOR_ENABLE
    uint32_t current_stack, stack_base;

#ifdef FREERTOS_ENABLE
    TaskStatus_t pxTaskStatus;
    if ((HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION)
            && (!__get_BASEPRI()) && (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)) {
        vTaskGetInfo(xTaskGetCurrentTaskHandle(), &pxTaskStatus, pdFALSE, eInvalid);
        current_stack = __get_PSP();
        stack_base = (uint32_t)pxTaskStatus.pxStackBase;
        if (current_stack <= stack_base) {
            configASSERT(0 && "Asserted by syslog monitor(process stack), stack is overflow");
        }
    } else {
        current_stack = __get_MSP();
        stack_base = (uint32_t)_stack_start;
        if (current_stack <= stack_base) {
            configASSERT(0 && "Asserted by syslog monitor(main stack), stack is overflow");
        }
    }
#else
    current_stack = __get_MSP();
    stack_base = (uint32_t)_stack_start;
    if (current_stack <= stack_base) {
        assert(0);
    }
#endif /* FREERTOS_ENABLE */
    return current_stack - stack_base;
#else
    return 0xFFFFFFFF;
#endif /* AIR_SYSLOG_STACK_MONITOR_ENABLE */
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM uint32_t syslog_port_query_nest_count(uint32_t handle)
{
    return mux_query_nest_count(handle);
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM bool syslog_port_query_user_name(uint32_t handle, const char **user_name)
{
    mux_status_t status;
    const char *name = NULL;

    status = mux_query_user_name(handle, &name);
    if (status != MUX_STATUS_OK) {
        return false;
    }

    *user_name = name;

    return true;
}

bool syslog_port_mux_exception_init(uint32_t handle)
{
    mux_status_t status;

    status = mux_exception_init(handle);
    if (status != MUX_STATUS_OK) {
        return false;
    }

    return true;
}

bool syslog_port_user_handle(uint32_t port, const char *user_name, uint32_t *p_handle)
{
    mux_status_t status;

    status = mux_query_user_handle(port, user_name, p_handle);
    if (status != MUX_STATUS_OK) {
        return false;
    }

    return true;
}

bool syslog_port_exception_send(uint32_t handle, uint8_t *buffers, uint32_t buffers_counter)
{
    mux_status_t status;

    status = mux_exception_send(handle, (mux_buffer_t *)buffers, buffers_counter);
    if (status != MUX_STATUS_OK) {
        return false;
    }

    return true;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM bool syslog_port_mux_tx(uint32_t handle, uint8_t *buffers, uint32_t buffers_counter, uint32_t *send_done_data_len)
{
    mux_status_t status;

    status = mux_tx(handle, (mux_buffer_t *)buffers, buffers_counter, send_done_data_len);
    if (status != MUX_STATUS_OK) {
        return false;
    }

    return true;
}

uint32_t syslog_port_cmd_query_tx_buffer_valid_size(uint32_t port)
{
    mux_status_t status;
    uint32_t tx_valid_size;

    status = mux_control(port, MUX_CMD_GET_VIRTUAL_TX_AVAIL_LEN, (mux_ctrl_para_t *)&tx_valid_size);
    if (status != MUX_STATUS_OK) {
        tx_valid_size = 0;
    }

    return tx_valid_size;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM uint32_t syslog_port_cmd_query_tx_buffer_free(uint32_t port)
{
    mux_status_t status;
    uint32_t tx_free_size;

    status = mux_control(port, MUX_CMD_GET_TX_AVAIL, (mux_ctrl_para_t *)&tx_free_size);
    if (status != MUX_STATUS_OK) {
        tx_free_size = 0;
    }

    return tx_free_size;
}

void syslog_port_cmd_tx_buffer_send(uint32_t port)
{
    mux_control(port, MUX_CMD_TX_BUFFER_SEND, NULL);
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM void syslog_port_cmd_clean_tx_buffer(uint32_t port)
{
    if ((port == MUX_USB_COM_1) || (port == MUX_USB_COM_2)) {
        mux_control(port, MUX_CMD_CLEAN, NULL);
    }
}

void syslog_port_excetpion_1wire_check(uint32_t port)
{
#if defined (AIR_1WIRE_ENABLE)
    if (syslog_query_one_wire_log_active() == true) {
        mux_control(port, MUX_CMD_UART_TX_ENABLE, NULL);
        syslog_port_uart_disable_flowcontrol(port);
    }
#endif
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_TCM bool syslog_port_core_is_exception(void)
{
    if (hal_core_status_read(HAL_CORE_MCU) == HAL_CORE_EXCEPTION) {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#if defined (AIR_1WIRE_ENABLE)
extern void smchg_1wire_mux_tx_protocol_cb(mux_handle_t handle, const mux_buffer_t payload[], uint32_t buffers_counter, mux_buffer_t *head, mux_buffer_t *tail, void *user_data);
extern void smchg_1wire_mux_rx_protocol_cb(mux_handle_t *handle, mux_buffer_t buffers[], uint32_t buffers_counter, uint32_t *consume_len, uint32_t *package_len, void *user_data);
#endif

void syslog_port_early_init(void)
{
    hal_uart_config_t uart_config;
    uart_config.baudrate        = syslog_port_query_uart_baudrate();
    uart_config.parity          = HAL_UART_PARITY_NONE;
    uart_config.stop_bit        = HAL_UART_STOP_BIT_1;
    uart_config.word_length     = HAL_UART_WORD_LENGTH_8;
    hal_uart_init(PORT_SYSLOG_INIT_STAGE_PORT, &uart_config);
#ifndef MTK_DEBUG_PLAIN_LOG_ENABLE
    hal_uart_set_software_flowcontrol(PORT_SYSLOG_INIT_STAGE_PORT, 0x11, 0x13, 0x77);
#endif
}

#if defined(MTK_NVDM_ENABLE) && !defined(AG3335)
log_nvkey_status_t syslog_port_form_nvkey_init(uint32_t *port)
{
    syslog_nvkey_cfg_t syslog_nvkey_data;
    uint32_t nvkey_item_size;
    uint8_t  syslog_port;
    nvkey_status_t nvkey_status;

    if (port == NULL) {
        return SYSLOG_NVKEY_STATUS_ERROR;
    }

    /* read port setting from nvkey */
    nvkey_item_size = sizeof(syslog_nvkey_data);
    memset((uint8_t *)(&syslog_nvkey_data), 0, nvkey_item_size);

    nvkey_status = nvkey_read_data(NVID_PERI_LOG_SETTING, (uint8_t *)(&syslog_nvkey_data), &nvkey_item_size);
    if ((nvkey_status != NVKEY_STATUS_OK) || (nvkey_item_size != 4) || (syslog_nvkey_data.magic_number != SYSLOG_NVKEY_MAGIC_NUMBER)) {
        return SYSLOG_NVKEY_STATUS_ERROR;
    }

    SYSLOG_MSGID_D("Logging: Read NVKEY, status=%d [0]=0x%x [1]=0x%x [2]=0x%x [3]=0x%02x", 5, nvkey_status,
                    syslog_nvkey_data.syslog_port, syslog_nvkey_data.syslog_level, syslog_nvkey_data.baudrate, syslog_nvkey_data.magic_number);

    if (syslog_nvkey_data.baudrate.high_br_enable != 0) {
        // TODO: 1wire not support high baudrate.
#ifdef HAL_UART_FEATURE_6M_BAUDRATE
        g_uart_baudrate = HAL_UART_BAUDRATE_6000000;
        SYSLOG_MSGID_I("Logging: Read NVKEY Logging uart enabe 6M baudrate", 0);
#endif
    }

    /* use reserve value bit for disable irq print log check */
#ifndef AIR_BTA_IC_PREMIUM_G2
    /* enable disable irq call log assert and disable irq call os API assert */
    syslog_port_enable_dis_irq_call_log_assert();
#endif

    /* 1wire setting init */
#if defined (AIR_1WIRE_ENABLE)
    smchg_nvkey_init();
    smchg_1wire_gpio_init(); //1wire gpio config

    if (smchg_cfg.one_wire_log) {
        syslog_config_one_wire_log_active();
        if ((smchg_cfg.uart_sel == SMCHG_UART0_2GPIO) || (smchg_cfg.uart_sel == SMCHG_UART0_1GPIO)) {
            syslog_nvkey_data.syslog_port.port = 0; //UART0
        } else {
            syslog_nvkey_data.syslog_port.port = 1; //UART1
        }
        SYSLOG_MSGID_D("1wire, log UART[%d]", 1, syslog_nvkey_data.syslog_port.port);
    }
#endif

    /* load syslog port parameter*/
    syslog_port = syslog_nvkey_data.syslog_port.port;

    if (syslog_port < (sizeof(g_log_port_mapping) / sizeof(g_log_port_mapping_t))) {
            *port = g_log_port_mapping[syslog_port].mux_port;
        SYSLOG_I("Logging: NVKEY port[%d]: %s", syslog_port, g_log_port_mapping[syslog_port].name);
    } else {
        SYSLOG_MSGID_E("Logging: Read NVKEY port:%d error, not support!!!", 1, syslog_port);
        return SYSLOG_NVKEY_STATUS_ERROR;
    }

    return SYSLOG_NVKEY_STATUS_OK;
}
#endif

uint32_t syslog_port_init(uint32_t port)
{
    log_nvkey_status_t nvkey_status;
    uint32_t syslog_port;
    mux_status_t status;
    mux_port_setting_t syslog_setting;
    mux_protocol_t protocol_callback;
    mux_port_buffer_t query_port_buffer;

    syslog_port = port;
    nvkey_status = SYSLOG_NVKEY_STATUS_INVALID;
    SYSLOG_MSGID_I("Logging: Default define runtime log port:%d", 1, syslog_port);

#if defined(MTK_NVDM_ENABLE) && !defined(AG3335)
    nvkey_status = syslog_port_form_nvkey_init(&syslog_port);
#endif /* MTK_NVDM_ENABLE && !AG3335 */

    /* default syslog port setting */
    syslog_setting.tx_buffer_size = AIR_SYSLOG_TX_BUFFER_SIZE;
    syslog_setting.rx_buffer_size = AIR_SYSLOG_RX_BUFFER_SIZE;
    syslog_setting.dev_setting.uart.uart_config.baudrate    = syslog_port_query_uart_baudrate();
    syslog_setting.dev_setting.uart.uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    syslog_setting.dev_setting.uart.uart_config.stop_bit    = HAL_UART_STOP_BIT_1;
    syslog_setting.dev_setting.uart.uart_config.parity      = HAL_UART_PARITY_NONE;
    syslog_setting.dev_setting.uart.flowcontrol_type        = MUX_UART_SW_FLOWCONTROL;
    syslog_setting.portLinkRegAddr = 0x0;

    if (nvkey_status == SYSLOG_NVKEY_STATUS_OK) {
        /* syslog nvkey is valid */
        SYSLOG_MSGID_I("Logging: syslog port read nvkey ok, port[%d]", 1, syslog_port);
        status = mux_query_port_numbers_from_nvdm("SYSLOG", (mux_port_buffer_t *)&query_port_buffer);
        if ((status == MUX_STATUS_OK) && (query_port_buffer.count == 1)) {
            SYSLOG_MSGID_I("Logging: query syslog port from nvdm ok, port[%d]", 1, query_port_buffer.buf[0]);
            if (query_port_buffer.buf[0] != syslog_port) {
                mux_close_delete_from_nvdm(query_port_buffer.buf[0], "SYSLOG");
                SYSLOG_MSGID_I("Logging: nvdm!=nvkey, delete nvdm port and save nvkey port to nvdm", 0);
            }
        } else {
            SYSLOG_MSGID_I("Logging: query syslog port from nvdm fail, nvkey port=%d", 1, syslog_port);
        }
    } else if (nvkey_status == SYSLOG_NVKEY_STATUS_INVALID) {
        /* no syslog nvkey */
        SYSLOG_MSGID_I("Logging: query port read nvkey fail, no syslog nvkey", 0);
        status = mux_query_port_numbers_from_nvdm("SYSLOG", (mux_port_buffer_t *)&query_port_buffer);
        if ((status == MUX_STATUS_OK) && (query_port_buffer.count == 1)) {
            SYSLOG_MSGID_I("Logging: query syslog port from nvdm ok, port[%d]", 1, query_port_buffer.buf[0]);
            syslog_port = query_port_buffer.buf[0];
        } else {
            SYSLOG_MSGID_I("Logging: query syslog port from nvdm fail, default port=%d", 1, syslog_port);
        }
    } else if (nvkey_status == SYSLOG_NVKEY_STATUS_ERROR) {
        /* syslog nvkey value error */
        SYSLOG_MSGID_I("Logging: query port read nvkey fail, syslog nvkey value error", 0);
        status = mux_query_port_numbers_from_nvdm("SYSLOG", (mux_port_buffer_t *)&query_port_buffer);
        if ((status == MUX_STATUS_OK) && (query_port_buffer.count == 1)) {
            SYSLOG_MSGID_I("Logging: query syslog port from nvdm ok, port[%d]", 1, query_port_buffer.buf[0]);
            syslog_port = query_port_buffer.buf[0];
        } else {
            SYSLOG_MSGID_I("Logging: query syslog port from nvdm fail, default port=%d", 1, syslog_port);
        }
    }

    if ((syslog_query_one_wire_log_active() == true) && ((syslog_port == MUX_UART_1) || (syslog_port == MUX_UART_0))) {
        SYSLOG_MSGID_I("Logging: one-wire log is avtive, 1-wire port=%d", 1, syslog_port);
#if defined (AIR_1WIRE_ENABLE)
        syslog_setting.dev_setting.uart.flowcontrol_type = MUX_UART_NONE_FLOWCONTROL;
        protocol_callback.rx_protocol_callback = smchg_1wire_mux_rx_protocol_cb;
        protocol_callback.tx_protocol_callback = smchg_1wire_mux_tx_protocol_cb;
        protocol_callback.user_data = NULL;
#else
        protocol_callback.rx_protocol_callback = NULL;
        protocol_callback.tx_protocol_callback = NULL;
        protocol_callback.user_data = NULL;
#endif
    } else {
        SYSLOG_MSGID_D("Logging: one-wire is invalid, port=%d", 1, syslog_port);
        if ((syslog_port == MUX_AIRAPP_0) || (syslog_port == MUX_FLASH)) {
            protocol_callback.rx_protocol_callback = NULL;
            protocol_callback.tx_protocol_callback = NULL;
            protocol_callback.user_data = NULL;
        } else {
            protocol_callback.rx_protocol_callback = race_rx_protocol_callback; //race_rx_no_packed_callback;
            protocol_callback.tx_protocol_callback = race_tx_protocol_callback; //race_tx_no_packed_callback;
            protocol_callback.user_data = NULL;
        }
    }

    SYSLOG_MSGID_I("Logging: begin mux_init syslog port:%d tx_buf_size:%d rx_buf_size:%d", 3,
                    syslog_port, syslog_setting.tx_buffer_size, syslog_setting.rx_buffer_size);

    /* mask log port uart irq */
    if (syslog_port < MUX_UART_END) {
        syslog_port_uart_disable_irq(syslog_port);
    }

    if ((syslog_port == MUX_USB_COM_1) || (syslog_port == MUX_USB_COM_2)) {
        syslog_debug_uart_init(HAL_UART_0);
    } else if (syslog_query_one_wire_log_active() == true) {
        if (syslog_port == 0) {
            syslog_debug_uart_init(HAL_UART_1);
        } else if (syslog_port == 1) {
            syslog_debug_uart_init(HAL_UART_0);
        }
    }

    /* update running port */
    g_syslog_nvkey_port = syslog_port;

    status = mux_init(syslog_port, &syslog_setting, &protocol_callback);

    if (syslog_port <= MUX_USB_END) {
        mux_open_save_to_nvdm(syslog_port, "SYSLOG");
    }

    return syslog_port;
}

void syslog_port_1wire_port_init(uint32_t syslog_port)
{
#if defined (AIR_1WIRE_ENABLE)
    /* check charger 5v exist */
    if (smchg_1wire_chg_exist()) {
        smchg_1wire_set_mode_status(SMCHG_1WIRE_CHG);
        mux_control(syslog_port, MUX_CMD_UART_TX_RX_DISABLE, NULL);
    } else {
        mux_ctrl_para_t port_config_parm;
        smchg_1wire_set_mode_status(SMCHG_1WIRE_LOG);
        port_config_parm.mux_set_config_uart_param.baudrate = smchg_cfg.log_mode_baud_rate;
        mux_control(syslog_port, MUX_CMD_CHANGE_UART_PARAM, &port_config_parm);
        mux_control(syslog_port, MUX_CMD_UART_TX_ENABLE, NULL);
    }
    hal_uart_disable_flowcontrol(syslog_port);
#else
    SYSLOG_PORT_UNUSED(syslog_port);
#endif
}

void syslog_port_post_init(uint32_t port, uint32_t *handle, void *user_cb_handler)
{
    mux_status_t status;
    if ((port == MUX_AIRAPP_0) || (port == MUX_FLASH)) {
        status = mux_open(port, "SYSLOG", (mux_handle_t *)handle, NULL, NULL);
        if (status != MUX_STATUS_OK) {
            assert(0);
        }
    } else {
        status = mux_open(port, "SYSLOG", (mux_handle_t *)handle, user_cb_handler, NULL);
        if (status != MUX_STATUS_OK) {
            assert(0);
        }
    }
}

#if !defined(MTK_DEBUG_LEVEL_NONE)
#if !defined(MTK_DEBUG_LEVEL_PRINTF)
/* For printf() */
int __wrap_printf(const char *format, ...)
{
    va_list ap;

    /* module fun line level format ap */
    va_start(ap, format);

#ifdef MTK_DEBUG_PLAIN_LOG_ENABLE
    vprint_module_log(&LOG_CONTROL_BLOCK_SYMBOL(printf), NULL, 0, 0, format, ap);
#else
    const char *fun = __FUNCTION__;
    vprint_module_log(&LOG_CONTROL_BLOCK_SYMBOL(printf), fun, __LINE__, PRINT_LEVEL_INFO, format, ap);
#endif

    va_end(ap);

    return 0;
}
#else /* !MTK_DEBUG_LEVEL_PRINTF */
int __wrap_printf(const char *format, ...)
{
    va_list ap;
    int32_t log_size;
    uint32_t irq_mask;
    char frame_header[PORT_SYSLOG_MAX_ONE_PLAIN_LOG_SIZE];

    va_start(ap, format);
    log_size = vsnprintf(frame_header, sizeof(frame_header), format, ap);
    if (log_size < 0) {
        return -1;
    }
    if ((uint32_t)log_size >= (sizeof(frame_header))) {
        log_size = sizeof(frame_header) - 1;
    }
    va_end(ap);

    syslog_port_nvic_save_and_set_interrupt_mask(&irq_mask);
#ifndef MTK_SINGLE_CPU_ENV
    while (hal_hw_semaphore_take(HW_SEMAPHORE_SYSLOG) != HAL_HW_SEMAPHORE_STATUS_OK);
#endif
    syslog_port_uart_send_polling(query_syslog_port(), (uint8_t *)frame_header, log_size);
#ifndef MTK_SINGLE_CPU_ENV
    while (hal_hw_semaphore_give(HW_SEMAPHORE_SYSLOG) != HAL_HW_SEMAPHORE_STATUS_OK);
#endif
    syslog_port_nvic_restore_interrupt_mask(irq_mask);

    return log_size;
}
#endif /* MTK_DEBUG_LEVEL_PRINTF */
#else  /* !MTK_DEBUG_LEVEL_NONE */
int __wrap_printf(const char *format, ...)
{
    SYSLOG_PORT_UNUSED(format);

    return 0;
}
#endif /* MTK_DEBUG_LEVEL_NONE */
