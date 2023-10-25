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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "syslog.h"
#include "mux.h"
#include "mux_port_common.h"
#include "hal_uart.h"

#ifdef FREERTOS_ENABLE
#include "FreeRTOS.h"
#include "task.h"
#endif

/*********port layer*****************/
#define PORT_SYSLOG_UNUSED(a) (void)a

#define NO_INLINE __attribute__((noinline))

/* MSGID log string start address define */
#define PORT_SYSLOG_MSG_ADDR_OFFSET 0x06000000

/* syslog length limitation */
#define PORT_SYSLOG_MAX_ONE_LOG_SIZE (160)
#define PORT_SYSLOG_MAX_ONE_PLAIN_LOG_SIZE (256)

/* module and filter define */
#define PORT_SYSLOG_MODULE_FILTER_TOTAL_NUMBER 200
#define PORT_SYSLOG_MODULE_FILTER_STATUS_SIZE (MTK_MAX_CPU_NUMBER * 2 + PORT_SYSLOG_MODULE_FILTER_TOTAL_NUMBER)
#define PORT_SYSLOG_MAX_MODULE_FILTER_STATUS_SIZE (PORT_SYSLOG_MAX_CPU_NUMBER * 2 + PORT_SYSLOG_MODULE_FILTER_TOTAL_NUMBER)

/* max drop log buffer size define */
#define MAX_DROP_BUFFER_SIZE                    64

/* syslog status define */
#define SYSLOG_INIT_NONE                        0x00
#define SYSLOG_EARLY_INIT                       0x01
#define SYSLOG_FULL_INIT                        0x02
#define SYSLOG_EXCEPTION_INIT                   0x03
#define SYSLOG_INIT_PHASE_MAX                   0x04
/* RACE HEAD */
#define RACE_HEAD_CHANNEL_BYTE                  0x05
/* RACE TYPE */
#define RACE_TYPE_CMD_WITH_RESPONCE             0x5A
#define RACE_TYPE_RESPONCE                      0x5B
#define RACE_TYPE_CMD_WITHOUT_RESPONCE          0x5C
#define RACE_TYPE_NOTIFICATION                  0x5D
/* RACE PROTOCOL ID */
#define RACE_PROTOCAL_SYSLOG_BEGIN              (0x0F00 | 0x10)
#define RACE_PROTOCAL_EXCEPTION_STRING_LOG      (0x0F00 | 0x12)
#define RACE_PROTOCAL_EXCEPTION_BINARY_LOG      (0x0F00 | 0x13)
#define RACE_PROTOCAL_ACTIVE_ASSERT             (0x0F00 | 0x14)
#define RACE_PROTOCAL_QUERY_VERSION_BUILDTIME   (0x0F00 | 0x15)
#define RACE_PROTOCAL_GET_LOG_FILTER_INFO       (0x0F00 | 0x16)
#define RACE_PROTOCAL_SET_LOG_FILTER_INFO       (0x0F00 | 0x17)
#define RACE_PROTOCAL_SAVE_LOG_SETTING          (0x0F00 | 0x18)
#define RACE_PROTOCAL_QUERY_CPU_FILTER_INFO     (0x0F00 | 0x19)
#define RACE_PROTOCAL_SET_CPU_FILTER_INFO       (0x0F00 | 0x20)
#define RACE_PROTOCAL_EXCEPTION_MSGID           (0x0F00 | 0x1A)
#define RACE_PROTOCAL_TLV_LOG                   (0x0F00 | 0x00)
#define RACE_PROTOCAL_STRING_LOG                (0x0F00 | 0x40)
#define RACE_PROTOCAL_MSGID_LOG                 (0x0F00 | 0x41)
#define RACE_PROTOCAL_SYSLOG_END                (0x0F00 | 0x4F)

/* RACE USER PROTOCOL ID */
#define RACE_PROTOCAL_GPS_BEGIN                 (0x0F91)
#define RACE_PROTOCAL_GPS_END                   (0x0F91)
#define RACE_PROTOCAL_ATCI_BEGIN                (0x0F92)
#define RACE_PROTOCAL_ATCI_END                  (0x0F92)

/* syslog stack monitor */
#define SYSLOG_LOCAL_STACK_USE                  (0xC0)      /* 192byte print_module_log + query_stack_space */
#define SYSLOG_STACK_MONITOR_RESERVE            (0x120)     /* 288byte reserve + irq_stack */
#define SYSLOG_TEXT_RESERVE_STACK_SPACE         (860 - SYSLOG_LOCAL_STACK_USE + SYSLOG_STACK_MONITOR_RESERVE)
#define SYSLOG_MSGID_RESERVE_STACK_SPACE        (692 - SYSLOG_LOCAL_STACK_USE + SYSLOG_STACK_MONITOR_RESERVE)

/* syslog pack head info struct */
typedef struct {
    uint8_t     race_pktId;
    uint8_t     race_type;
    uint16_t    race_length;
    uint16_t    race_id;
} __attribute__((packed)) syslog_race_t;

typedef struct {
    uint8_t     log_cpu_id;
    uint8_t     log_seq_num;
    uint8_t     log_irq_num;
    uint8_t     log_task_num;
    uint8_t     log_reserve[2];
    uint32_t    log_timestamp;
} __attribute__((packed)) syslog_info_t;

typedef struct {
    syslog_info_t   log_info;
    uint32_t        msg_offset;
} __attribute__((packed)) syslog_msgid_t;

typedef struct {
    uint8_t     log_cpu_id;
    uint8_t     log_reserve[5];
    uint32_t    msg_offset;
} __attribute__((packed)) syslog_exc_info_t;

typedef struct {
    syslog_race_t log_race;
    syslog_info_t log_info;
    uint8_t data[0];
} __attribute__((packed)) syslog_pack_t;

/* SET MODULE FILTER END FLAG (SILM ASSERT API)*/
__attribute__ ((__section__(".log_filter_end"))) __attribute__((used)) static const char fiter_end_flag[] = "[MODULE_FILTER_END]";

uint32_t query_current_stack(uint32_t *p_data)
{
    __asm__ __volatile__("s32i.n a1, a2, 0 \n");
    return *p_data;
}

/* syslog stack monitor */
extern unsigned int _stack_start[];
uint32_t syslog_port_query_curent_stack_space(void)
{
#ifdef AIR_SYSLOG_STACK_MONITOR_ENABLE
    uint32_t stack_base, current_stack;

#ifdef FREERTOS_ENABLE
    TaskStatus_t pxTaskStatus;
    uint32_t int_level;
    if ((HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) && (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)) {
        int_level = (uint32_t)XTOS_SET_INTLEVEL(4); // to get intlevel
        XTOS_RESTORE_INTLEVEL(int_level);
        if (int_level == 0) {    // !__get_BASEPRI()
            vTaskGetInfo(xTaskGetCurrentTaskHandle(), &pxTaskStatus, pdFALSE, eInvalid);
            query_current_stack(&current_stack);
            stack_base = (uint32_t)pxTaskStatus.pxStackBase;
            if (current_stack <= stack_base) {
                assert(0 && "Asserted by syslog monitor os, sp < sp_base");
            }
        } else {
            query_current_stack(&current_stack);
            stack_base = (uint32_t)_stack_start;
            if (current_stack <= stack_base) {
                assert(0 && "Asserted by syslog monitor os->irq, sp < sp_base");
            }
        }
    } else {
        query_current_stack(&current_stack);
        stack_base = (uint32_t)_stack_start;
        if (current_stack <= stack_base) {
            assert(0 && "Asserted by syslog monitor irq, sp < sp_base");
        }
    }
#else
    query_current_stack(&current_stack);
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

/* dummy function,
    when disable syslog, some local variable will cause unused error
*/
void syslog_port_dummy(const char *message, ...)
{
    (void)(message);
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM static uint32_t port_syslog_get_current_timestamp(void)
{
    uint32_t count = 0;
    uint64_t count64 = 0;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    count64 = ((uint64_t)count) * 1000 / 32768;

    return (unsigned int)count64;
}

/* log port nest count */
uint32_t g_syslog_nest_count;
extern uint32_t mux_query_nest_count(mux_handle_t handle);

#define DROP_PROMPT_LOG             ">>> log drop "
#define DROP_PROMPT_HOLE_LOG        ">>> log hole "
#define DROP_PROMPT_LOG_FIX_SIZE    (sizeof(DROP_PROMPT_LOG) - 1) /* substract \0 */

static uint32_t uint_to_str(uint32_t number, uint8_t *str_array)
{
    uint32_t i, value, bits;
    uint8_t byte_array[16];

    bits = 0;
    value = number;
    do {
        byte_array[bits++] = value % 10;
        value /= 10;
    } while (value);

    for (i = 0; i < bits; i++) {
        str_array[i] = byte_array[bits - i - 1] + '0';
    }

    return bits;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM uint32_t port_syslog_drop_prompt_length(uint32_t drop_log_count)
{
    uint32_t drop_bit;

    drop_bit = 0;
    while (drop_log_count / 10) {
        drop_log_count /= 10;
        drop_bit++;
    }
    if (drop_log_count) {
        drop_bit++;
    }

#ifdef MTK_DEBUG_PLAIN_LOG_ENABLE
    return DROP_PROMPT_LOG_FIX_SIZE + drop_bit + 2; /* string + drop + /r/n */
#else
    return 16 + DROP_PROMPT_LOG_FIX_SIZE + drop_bit; /* race header + string + drop */
#endif
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM void port_syslog_drop_prompt(uint32_t timestamp, uint32_t drop_log_size, uint32_t drop_log_count, uint8_t *drop_prompt)
{
    uint32_t bits;
    uint8_t array[16];
    uint8_t *p_curr_drop_prompt = drop_prompt;
    uint32_t per_cpu_irq_mask;

#ifndef MTK_DEBUG_PLAIN_LOG_ENABLE
    syslog_pack_t *p_syslog_pack = (syslog_pack_t*)&array[0];
    p_syslog_pack->log_race.race_pktId      = RACE_HEAD_CHANNEL_BYTE;
    p_syslog_pack->log_race.race_type       = RACE_TYPE_NOTIFICATION;
    p_syslog_pack->log_race.race_length     = drop_log_size - 4;
    p_syslog_pack->log_race.race_id         = RACE_PROTOCAL_STRING_LOG;
    p_syslog_pack->log_info.log_cpu_id          = GET_CURRENT_CPU_ID();
    p_syslog_pack->log_info.log_seq_num         = 0x0;
    p_syslog_pack->log_info.log_irq_num         = 0x0;
    p_syslog_pack->log_info.log_task_num        = 0x0;
    p_syslog_pack->log_info.log_reserve[0]      = 0x0;
    p_syslog_pack->log_info.log_reserve[1]      = 0x0;
    p_syslog_pack->log_info.log_timestamp       = timestamp;
    memcpy(p_curr_drop_prompt, array, sizeof(syslog_pack_t));
    p_curr_drop_prompt += sizeof(syslog_pack_t);
#endif

    /* drop log prefix */
    hal_nvic_save_and_set_interrupt_mask(&per_cpu_irq_mask);
    if (g_syslog_nest_count != 0) {
        /* hole limit cause */
        memcpy(p_curr_drop_prompt, DROP_PROMPT_HOLE_LOG, DROP_PROMPT_LOG_FIX_SIZE);
    } else {
        /* through put limit cause */
        memcpy(p_curr_drop_prompt, DROP_PROMPT_LOG, DROP_PROMPT_LOG_FIX_SIZE);
    }
    hal_nvic_restore_interrupt_mask(per_cpu_irq_mask);
    p_curr_drop_prompt += DROP_PROMPT_LOG_FIX_SIZE;

    /* drop log count */
    bits = uint_to_str(drop_log_count, array);
    memcpy(p_curr_drop_prompt, array, bits);
    p_curr_drop_prompt += bits;

    /* add \r\n */
#ifdef MTK_DEBUG_PLAIN_LOG_ENABLE
    memcpy(p_curr_drop_prompt, "\r\n", 2);
#endif
}

extern unsigned port_xSchedulerRunning;

/* init stage nest flag */
static bool g_syslog_user_init_contex;
/* os stage nest flag */
static bool g_syslog_user_os_nest_is_running;
static uint32_t g_syslog_user_task_number;
static uint32_t g_syslog_user_task_max_number;
static TaskHandle_t *p_syslog_user_task_context;
/* irq stage nest flag */
static uint32_t g_syslog_user_irq_context[(IRQ_NUMBER_MAX + 31) / 32];

bool port_syslog_anti_nest_check_begin(void)
{
    uint32_t i, j, cpu_irq_mask;
    TaskHandle_t task_context;

    hal_nvic_save_and_set_interrupt_mask(&cpu_irq_mask);

    if ((xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) || (port_xSchedulerRunning == false)) {
        /* Init context */
        if (g_syslog_user_init_contex != false) {
            assert(0 && "Asserted by syslog init nest check");
        }
        g_syslog_user_init_contex = true;
    } else if (HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) {
        /* Init task context table */
        if ((g_syslog_user_os_nest_is_running == false) && (p_syslog_user_task_context == NULL)) {
            g_syslog_user_task_max_number = (uxTaskGetNumberOfTasks() + 6); // reserve 3
            p_syslog_user_task_context = malloc(sizeof(TaskHandle_t) * g_syslog_user_task_max_number);
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

    hal_nvic_save_and_set_interrupt_mask(&cpu_irq_mask);

    if ((xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) || (port_xSchedulerRunning == false)) {
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

#if !defined(MTK_DEBUG_LEVEL_PRINTF)

/* share syslog variable define */
#define PORT_SYSLOG_MAX_CPU_NUMBER 16
/*
 * Global share variable shared by all CPUs, make sure every member keep align access.
 * [cpu_module_filter_status]:
 *      cpu1_id                   1 byte
 *      cpu1_module_number        1 byte
 *      cpu2_id                   1 byte
 *      cpu2_module_number        1 byte
 *        ..                        ..
 *      cpuN_id                   1 byte
 *      cpuN_module_number        1 byte
 *      valid_cpu1_module1_log_switch   1 byte (high 4 bit)
 *      valid_cpu1_module1_print_level  1 byte (low 4 bit)
 *        ..                              ..
 *      valid_cpu1_moduleM_log_switch   1 byte (high 4 bit)
 *      valid_cpu1_moduleM_print_level  1 byte (low 4 bit)
 *      valid_cpu2_module1_log_switch   1 byte (high 4 bit)
 *      valid_cpu2_module1_print_level  1 byte (low 4 bit)
 *        ..                              ..
 *      valid_cpu2_moduleM_log_switch   1 byte (high 4 bit)
 *      valid_cpu2_moduleM_print_level  1 byte (low 4 bit)
 * [cpu_module_filter]:
 *      per-cpu needs to do address transfer to CM4 view for access by CM4 side.
 */
typedef struct {
    mux_handle_t handle;
    uint32_t one_wire_active;
    uint32_t init_phase;
    uint32_t sequence[2];
    uint32_t drop_count[2];
    uint8_t cpu_log_switch[PORT_SYSLOG_MAX_CPU_NUMBER];
    uint8_t cpu_log_print_level[PORT_SYSLOG_MAX_CPU_NUMBER];
    uint8_t cpu_module_filter_status[PORT_SYSLOG_MAX_MODULE_FILTER_STATUS_SIZE]; /* log filter setting read from NVDM to let per-cpu do initialization with it's log filter array. */
    void *cpu_module_filter[PORT_SYSLOG_MAX_CPU_NUMBER]; /* Init to per-cpu's log filter array when per-cpu call log_set_filter() in it's init phase. */
    uint32_t debug_flag;
} syslog_share_variable_t;

#include "hal_resource_assignment.h"
#define PORT_SYSLOG_SHARE_VARIABLE_BEGIN HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VAR_START
static volatile syslog_share_variable_t *g_syslog_share_variable = (volatile syslog_share_variable_t *)(PORT_SYSLOG_SHARE_VARIABLE_BEGIN); /* 344byte */

/* log filters collection */
#if defined(__GNUC__)
extern uint8_t _log_filter_start[];
extern uint8_t _log_filter_end[];
#define LOG_FILTER_START _log_filter_start
#define LOG_FILTER_END _log_filter_end
#endif /* __GNUC__ */

// #define port_syslog_memory_remap_to_primary_cpu(cpu_id, address) address
/* Memory remap across CPUs (other CPU remap to primary CPU) */
static uint32_t port_syslog_memory_remap_to_primary_cpu(uint32_t cpu_id, uint32_t address)
{
    if (cpu_id == 1) {
        return hal_memview_dsp0_to_cm4(address);
    } else {
        return address;
    }
}

/* SDK version and build time */
#include "memory_attribute.h"
#define DUMMY_SDK_VERSION_STRING "DUMMY_SDK_VERSION"
static const char sw_verno_str[] = DUMMY_SDK_VERSION_STRING;
ATTR_LOG_VERSION log_bin_sw_verno_str[] = DUMMY_SDK_VERSION_STRING;
extern char build_date_time_str[];

#define PORT_SYSLOG_SDK_VERSION_BEGIN HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VERSION_VAR_START
#define PORT_SYSLOG_SDK_VERSION_LENGTH 48
#define PORT_SYSLOG_BUILD_TIME_BEGIN HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_BUILD_TIME_VAR_START
#define PORT_SYSLOG_BUILD_TIME_LENGTH 48
static void port_syslog_build_time_sdk_version_copy(uint32_t cpu_id)
{
    strncpy((char *)(PORT_SYSLOG_SDK_VERSION_BEGIN + cpu_id * PORT_SYSLOG_SDK_VERSION_LENGTH), sw_verno_str, PORT_SYSLOG_SDK_VERSION_LENGTH);
    strncpy((char *)(PORT_SYSLOG_BUILD_TIME_BEGIN + cpu_id * PORT_SYSLOG_BUILD_TIME_LENGTH), build_date_time_str, PORT_SYSLOG_BUILD_TIME_LENGTH);
}

/* 1-wire log mode, share with MCU */
typedef enum {
    SMCHG_1WIRE_NORM,
    SMCHG_1WIRE_OUT,
    SMCHG_1WIRE_LOG,
    SMCHG_1WIRE_CHG,
    SMCHG_1WIRE_COM,
    SMCHG_1WIRE_RACE,
    SMCHG_1WIRE_ATCI,
    SMCHG_1WIRE_MAX,
} smchg_1wire_mode_t;

static volatile smchg_1wire_mode_t *p_1wire_mode = (volatile smchg_1wire_mode_t *)HW_SYSRAM_PRIVATE_MEMORY_1WIRE_START;

smchg_1wire_mode_t smchg_1wire_get_mode_status(void)
{
    if (p_1wire_mode == NULL) {
        return SMCHG_1WIRE_NORM;
    }

    return *p_1wire_mode;
}

bool log_port_get_one_wire_active(void)
{
    if (g_syslog_share_variable->one_wire_active != 0x0) {
        return true;
    }
    return false;
}

void race_tx_protocol_callback(mux_handle_t handle, const mux_buffer_t payload[], uint32_t buffers_counter,
                               mux_buffer_t *head, mux_buffer_t *tail, void *user_data)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(payload);
    PORT_SYSLOG_UNUSED(buffers_counter);
    PORT_SYSLOG_UNUSED(user_data);
    smchg_1wire_mode_t onw_wire_mode_status;

    if (log_port_get_one_wire_active() == true) {
        onw_wire_mode_status = smchg_1wire_get_mode_status();
        if ((onw_wire_mode_status != SMCHG_1WIRE_NORM) && (onw_wire_mode_status != SMCHG_1WIRE_LOG)) {
            /* Ignore tx except syslog */
            /* "Wire" = 0x57697265 */
            head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 0] = 'e';
            head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 1] = 'r';
            head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 2] = 'i';
            head->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 3] = 'W';

            tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 0] = 'e';
            tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 1] = 'r';
            tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 2] = 'i';
            tail->p_buf[TX_PROTOCOL_CB_HEAD_BUFFER_MAX_LEN + 3] = 'W';
        }
    }

    head->p_buf = NULL;
    tail->p_buf = NULL;
    head->buf_size = 0;
    tail->buf_size = 0;
    return;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM static uint32_t port_syslog_send(uint32_t id, bool drop_flag, mux_buffer_t *p_buf_info)
{
    mux_status_t status;
    uint8_t race_header[sizeof(syslog_race_t)];
    uint32_t i, total_size = 0;
    uint32_t counter, cpu_id, per_cpu_irq_mask;

    p_buf_info[0].p_buf = race_header;
    p_buf_info[0].buf_size = sizeof(syslog_race_t);

    for (i = 0; p_buf_info[i].p_buf != NULL; i++) {
        total_size += p_buf_info[i].buf_size;
    }
    counter = i;

    /* Insert the race header here */
    syslog_race_t *p_head_race = (syslog_race_t*)&race_header[0];
    p_head_race->race_pktId     = RACE_HEAD_CHANNEL_BYTE;
    p_head_race->race_type      = RACE_TYPE_NOTIFICATION;
    p_head_race->race_length    = total_size - 4;
    p_head_race->race_id        = id;

    cpu_id = GET_CURRENT_CPU_ID();

#if 1 /* This feature have risk, need review */
    if (g_syslog_share_variable->drop_count[cpu_id] != 0) {
        uint32_t tx_size = 0, tx_free_size = 0, drop_package_len = 0;
        uint8_t *p_drop_package;
        mux_buffer_t drop_buf;
        p_drop_package = (uint8_t *)malloc(MAX_DROP_BUFFER_SIZE);
        if (p_drop_package == NULL) {
            return 0;
        }
        drop_package_len = port_syslog_drop_prompt_length(g_syslog_share_variable->drop_count[cpu_id]);
        tx_size = total_size + drop_package_len;
        mux_control((g_syslog_share_variable->handle & 0xFF), MUX_CMD_GET_TX_AVAIL, (mux_ctrl_para_t *)&tx_free_size);
        if (tx_free_size >= tx_size) {
            memset(p_drop_package, 0, MAX_DROP_BUFFER_SIZE);
            port_syslog_drop_prompt(port_syslog_get_current_timestamp(), drop_package_len, g_syslog_share_variable->drop_count[cpu_id], (uint8_t *)p_drop_package);
            drop_buf.p_buf = p_drop_package;
            drop_buf.buf_size = drop_package_len;
            mux_tx(g_syslog_share_variable->handle, (mux_buffer_t*)&drop_buf, 1, &tx_size);
            hal_nvic_save_and_set_interrupt_mask(&per_cpu_irq_mask);
            g_syslog_share_variable->drop_count[cpu_id] = 0;
            g_syslog_nest_count = 0;
            hal_nvic_restore_interrupt_mask(per_cpu_irq_mask);
        } else {
            hal_nvic_save_and_set_interrupt_mask(&per_cpu_irq_mask);
            g_syslog_share_variable->drop_count[cpu_id] += 1;
            g_syslog_nest_count = mux_query_nest_count(g_syslog_share_variable->handle);
            hal_nvic_restore_interrupt_mask(per_cpu_irq_mask);
            free(p_drop_package);
            return 0;
        }
        free(p_drop_package);
    }
#endif

    do {
        status = mux_tx(g_syslog_share_variable->handle, p_buf_info, counter, &total_size);
    } while ((drop_flag == false) && (total_size == 0));

    if (status == MUX_STATUS_OK) {
        total_size = total_size - sizeof(race_header);
    } else {
        hal_nvic_save_and_set_interrupt_mask(&per_cpu_irq_mask);
        g_syslog_share_variable->drop_count[cpu_id] += 1;
        g_syslog_nest_count = mux_query_nest_count(g_syslog_share_variable->handle);
        hal_nvic_restore_interrupt_mask(per_cpu_irq_mask);
        total_size = 0;
    }
    return total_size;
}

typedef struct {
    uint8_t cpu_id;
    uint8_t module_number;
} module_filter_info_t;

#define SYSLOG_FILTER_VALID_MARK 0x80

/* TCM is mainly for Optimize log printing time */
NO_INLINE ATTR_TEXT_IN_IRAM volatile uint8_t *find_start_of_cpu_log_filters(uint32_t cpu_id)
{
    uint32_t offset, i;
    module_filter_info_t *p_filter_info;

    offset = 2 * MTK_MAX_CPU_NUMBER;
    p_filter_info = (module_filter_info_t *)(g_syslog_share_variable->cpu_module_filter_status);
    for (i = 0; i < MTK_MAX_CPU_NUMBER; i++) {
        if ((p_filter_info[i].cpu_id & SYSLOG_FILTER_VALID_MARK) == 0) {
            break;
        }
        if ((p_filter_info[i].cpu_id & (~SYSLOG_FILTER_VALID_MARK)) == cpu_id) {
            break;
        }
        offset += p_filter_info[i].module_number;
    }

    return &(g_syslog_share_variable->cpu_module_filter_status[offset]);
}

#if !defined(MTK_DEBUG_LEVEL_NONE)

bool log_port_init(void)
{
    uint8_t cpu_id;
    mux_protocol_t protocol_callback;

    cpu_id = GET_CURRENT_CPU_ID();

    /* register user call callback for dsp */
    protocol_callback.rx_protocol_callback = NULL;
    protocol_callback.tx_protocol_callback = race_tx_protocol_callback;
    protocol_callback.user_data = NULL; //user_data

    extern void mux_register_uaser_callback(mux_handle_t handle, uint8_t cpu_id, mux_protocol_t *protocol);
    mux_register_uaser_callback(g_syslog_share_variable->handle, cpu_id, &protocol_callback);

    return true;
}

uint32_t query_syslog_port(void)
{
    uint32_t temp_value = 0;
    if (g_syslog_share_variable->init_phase != SYSLOG_FULL_INIT) {
        return 0;
    }
    temp_value= g_syslog_share_variable->handle;
    temp_value = temp_value&0xFF;
    
    return temp_value;
}

bool log_set_filter(void)
{
    uint32_t i, cpu_id, offset;
    uint32_t runtime_filter_number;
    module_filter_info_t *p_filter_info;
    volatile uint8_t *p_filters;
    log_control_block_t *entries;

    PORT_SYSLOG_UNUSED(fiter_end_flag);

    if (g_syslog_share_variable->init_phase != SYSLOG_FULL_INIT) {
        return false;
    }

    cpu_id = GET_CURRENT_CPU_ID();

    log_port_init();

    port_syslog_build_time_sdk_version_copy(cpu_id);

    /* Calculate the number of runtime module filter */
    runtime_filter_number = (LOG_FILTER_END - LOG_FILTER_START) / sizeof(log_control_block_t);
    /* As this variable need to be by accessed by master CPU, so maybe need to do remap */
    entries = (log_control_block_t *)LOG_FILTER_START;
    g_syslog_share_variable->cpu_module_filter[cpu_id] = (void *)port_syslog_memory_remap_to_primary_cpu(cpu_id, (uint32_t)entries);

    p_filter_info = (module_filter_info_t *)g_syslog_share_variable->cpu_module_filter_status;

    for (i = 0; i < MTK_MAX_CPU_NUMBER; i++) {
        if (!(p_filter_info[i].cpu_id & SYSLOG_FILTER_VALID_MARK)) {
            break;
        }
        if ((p_filter_info[i].cpu_id & (~SYSLOG_FILTER_VALID_MARK)) == cpu_id) {
            if (p_filter_info[i].module_number != runtime_filter_number) {
                assert(0);
                return false;
            }
            return true;
        }
    }
    offset = i;

    p_filters = find_start_of_cpu_log_filters(cpu_id);
    for (i = 0; i < runtime_filter_number; i++) {
        p_filters[i] = ((uint8_t)(entries[i].log_switch) << 4) | (uint8_t)(entries[i].print_level);
    }
    p_filter_info[offset].cpu_id = cpu_id | SYSLOG_FILTER_VALID_MARK;
    p_filter_info[offset].module_number = runtime_filter_number;

    return true;
}

void filter_config_print_switch(void *handle, log_switch_t log_switch)
{
    uint32_t index, cpu_id;
    volatile uint8_t *p_filters;

    cpu_id = GET_CURRENT_CPU_ID();
    if (g_syslog_share_variable->cpu_module_filter[cpu_id] == NULL) {
        return;
    }

    index = ((uint32_t)handle - (uint32_t)LOG_FILTER_START) / sizeof(log_control_block_t);
    p_filters = find_start_of_cpu_log_filters(cpu_id);

    p_filters[index] &= 0x0F;
    p_filters[index] |= log_switch << 4;
}

void filter_config_print_level(void *handle, print_level_t log_level)
{
    uint32_t index, cpu_id;
    volatile uint8_t *p_filters;

    cpu_id = GET_CURRENT_CPU_ID();
    if (g_syslog_share_variable->cpu_module_filter[cpu_id] == NULL) {
        return;
    }

    index = ((uint32_t)handle - (uint32_t)LOG_FILTER_START) / sizeof(log_control_block_t);
    p_filters = find_start_of_cpu_log_filters(cpu_id);

    p_filters[index] &= 0xF0;
    p_filters[index] |= log_level;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM static bool filter_runtime_check(const void *is_module_control, log_control_block_t *context, print_level_t level)
{
    uint32_t offset;
    volatile uint8_t *p_filters;

    /* If current CPU debug level is turn off, bypass the log. */
    if (g_syslog_share_variable->cpu_log_switch[GET_CURRENT_CPU_ID()] == DEBUG_LOG_OFF) {
        return false;
    }

    /* Check the address range to detect the seperate build log filter */
    if (((uint32_t)context >= (uint32_t)LOG_FILTER_END) || ((uint32_t)context < (uint32_t)LOG_FILTER_START)) {
        if ((is_module_control != NULL) &&
            ((context->log_switch == DEBUG_LOG_OFF) ||
             (level < g_syslog_share_variable->cpu_log_print_level[GET_CURRENT_CPU_ID()]) ||
             (level < context->print_level))) {
            return false;
        }

        return true;
    }

    /* For LOG_*()/LOG_MSGID_*()/LOG_TLVDUMP_*(), ignore it if
     * 1. If module's debug level is turn off or
     * 2. If current log level is lower than current CPU's debug level.
     * 3. If current log level is lower than module's debug level.
     */
    offset = ((uint32_t)context - (uint32_t)(g_syslog_share_variable->cpu_module_filter[GET_CURRENT_CPU_ID()])) / sizeof(log_control_block_t);
    p_filters = find_start_of_cpu_log_filters(GET_CURRENT_CPU_ID());
    if ((is_module_control != NULL) &&
        (((p_filters[offset] >> 4) == DEBUG_LOG_OFF) ||
         (level < g_syslog_share_variable->cpu_log_print_level[GET_CURRENT_CPU_ID()]) ||
         (level < (p_filters[offset] & 0x0F)))) {
        return false;
    }

    return true;
}

static const char *print_level_table[] = {"debug", "info", "warning", "error"};

#define change_level_to_string(level) \
  ((level) - PRINT_LEVEL_DEBUG <= PRINT_LEVEL_ERROR) ? print_level_table[level] : "debug"

log_create_module(common, PRINT_LEVEL_INFO);
log_create_module(MPLOG, PRINT_LEVEL_WARNING);
log_create_module(printf, PRINT_LEVEL_INFO);

/* TCM is mainly for Optimize log printing time */
NO_INLINE ATTR_TEXT_IN_IRAM static bool check_log_control(const void *is_module_control, log_control_block_t *context, print_level_t level)
{
    /* check whether syslog is initialized. */
    if (g_syslog_share_variable->init_phase == SYSLOG_INIT_NONE) {
        return false;
    }

    /* check whether syslog is exception mode. */
    if (g_syslog_share_variable->init_phase == SYSLOG_EXCEPTION_INIT) {
        return false;
    }

    /* check whether CPU control when not full initialize. */
    if (g_syslog_share_variable->init_phase == SYSLOG_EARLY_INIT) {
        if (GET_CURRENT_CPU_ID() != 0) {
            return false;
        }
    }

    /* check whether debug level control when full initialize */
    if (g_syslog_share_variable->init_phase == SYSLOG_FULL_INIT) {
        if (filter_runtime_check(is_module_control, context, level) == false) {
            return false;
        }
    }

    return true;
}

extern unsigned port_xSchedulerRunning;

#ifndef AIR_BTA_IC_PREMIUM_G2
uint32_t syslog_port_query_debug_flag(void)
{
    return g_syslog_share_variable->debug_flag;
}
#endif

uint32_t syslog_port_query_init_number(void)
{
    uint32_t log_int_level;
    log_int_level = (uint32_t)XTOS_SET_INTLEVEL(4);
    XTOS_RESTORE_INTLEVEL(log_int_level);

#ifdef FREERTOS_ENABLE
    if ((xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) || (port_xSchedulerRunning == 0)) {
        log_int_level = 0xF;
    }

#ifndef AIR_BTA_IC_PREMIUM_G2
    if (((log_int_level & 0xF) == 0x4) && (g_syslog_share_variable->debug_flag == 0x1)) {
        assert(0);
    }
#endif

#endif

    return log_int_level;
}

#ifdef MTK_SYSLOG_SUB_FEATURE_STRING_LOG_SUPPORT
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ NO_INLINE static void string_log_handler(void *handle,
                                         const char *func,
                                         int line,
                                         print_level_t level,
                                         const char *message,
                                         va_list list,
                                         const void *data,
                                         int data_len)
{
    /* race + log_info + [data * 1] + NULL */
    mux_buffer_t log_buf_info[4];
    char *p_frame_header;
    char frame_header[PORT_SYSLOG_MAX_ONE_LOG_SIZE];
    int32_t step_size, log_size, max_size;
    uint32_t per_cpu_irq_mask;

    PORT_SYSLOG_UNUSED(line);

#ifdef FREERTOS_ENABLE
    TaskStatus_t pxTaskStatus;
#endif

    /* anti-nest check start */
    if (port_syslog_anti_nest_check_begin() == false) {
        return;
    }

    /* check log control */
    if (check_log_control(func, (log_control_block_t*)handle, level) == false) {
        /* anti-nest check stop */
        port_syslog_anti_nest_check_end();
        return;
    }

#ifdef MTK_DEBUG_PLAIN_LOG_ENABLE
    max_size = sizeof(frame_header) - 2; /* reserved for \r\n */
#else
    max_size = sizeof(frame_header) - sizeof(syslog_info_t); /* reserved for TLV header */
#endif

    if (data) {
        max_size -= 1;
    }

    /* Format the log header and calculate the size */
#ifdef MTK_DEBUG_PLAIN_LOG_ENABLE
    p_frame_header = frame_header;
#else
    p_frame_header = &frame_header[sizeof(syslog_info_t)];
#endif
    if ((func) && (strcmp(((log_control_block_t *)handle)->module_name, "printf") != 0)) {
        log_size = snprintf(p_frame_header, max_size,
                            "[M:%s C:%s F: L:]: ",
                            ((log_control_block_t *)handle)->module_name,
                            change_level_to_string(level));
        if (log_size < 0) {
            /* anti-nest check stop */
            port_syslog_anti_nest_check_end();
            return;
        }
        if (log_size >= max_size) {
            log_size = max_size - 1;
        }
    } else {
        log_size = 0;
    }

    /* Format the log string/arguments and calculate the size */
    max_size -= log_size;
    p_frame_header += log_size;
    step_size = vsnprintf(p_frame_header, max_size, message, list);
    if (step_size < 0) {
        /* anti-nest check stop */
        port_syslog_anti_nest_check_end();
        return;
    }
    if (step_size >= max_size) {
        step_size = max_size - 1;
    }
    log_size += step_size;

#ifndef MTK_DEBUG_PLAIN_LOG_ENABLE
    syslog_info_t *p_syslog_info = (syslog_info_t*)&frame_header[0];
    hal_nvic_save_and_set_interrupt_mask(&per_cpu_irq_mask);
    p_syslog_info->log_cpu_id       = GET_CURRENT_CPU_ID();
    p_syslog_info->log_seq_num      = g_syslog_share_variable->sequence[1]++;
    hal_nvic_restore_interrupt_mask(per_cpu_irq_mask);
    p_syslog_info->log_irq_num      = HAL_NVIC_QUERY_EXCEPTION_NUMBER;
#ifdef FREERTOS_ENABLE
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        vTaskGetInfo(xTaskGetCurrentTaskHandle(), &pxTaskStatus, pdFALSE, eInvalid);
        p_syslog_info->log_task_num = pxTaskStatus.xTaskNumber;
    } else {
        p_syslog_info->log_task_num = 0;
    }
#else
    p_syslog_info->log_task_num = 0;
#endif
    p_syslog_info->log_reserve[0]   = (syslog_port_query_init_number() & 0xF);
    p_syslog_info->log_reserve[1]   = 0;
    p_syslog_info->log_timestamp    = port_syslog_get_current_timestamp();
#else
    /* add \r\n in the end */
    frame_header[log_size] = '\r';
    frame_header[log_size + 1] = '\n';
#endif

    log_buf_info[1].p_buf = (uint8_t *)frame_header;
#ifndef MTK_DEBUG_PLAIN_LOG_ENABLE
    log_buf_info[1].buf_size = sizeof(syslog_info_t) + log_size;
#else
    log_buf_info[1].buf_size = 2 + log_size;
#endif

    if (data) {
        /* filling a 0 */
        frame_header[log_buf_info[1].buf_size] = 0x0;
        log_buf_info[1].buf_size += 1;
        /* load user data */
        log_buf_info[2].p_buf = (uint8_t *)data;
        log_buf_info[2].buf_size = data_len;
        log_buf_info[3].p_buf = NULL;
    } else {
        log_buf_info[2].p_buf = NULL;
    }

    port_syslog_send(RACE_PROTOCAL_STRING_LOG, true, log_buf_info);

    /* anti-nest check stop */
    port_syslog_anti_nest_check_end();
}

void vprint_module_log(void *handle,
                       const char *func,
                       int line,
                       print_level_t level,
                       const char *message,
                       va_list list)
{
    string_log_handler(handle, func, line, level, message, list, NULL, 0);
}

/* For LOG_*() */
/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM void print_module_log(void *handle,
                                        const char *func,
                                        int line,
                                        print_level_t level,
                                        const char *message, ...)
{
    va_list ap;

    if (g_syslog_share_variable->cpu_log_switch[GET_CURRENT_CPU_ID()] == DEBUG_LOG_OFF) {
        return;
    }

    if (syslog_port_query_curent_stack_space() < SYSLOG_TEXT_RESERVE_STACK_SPACE) {
        /* stack reserce space is not enough */
        assert(0);
    }

    va_start(ap, message);
    vprint_module_log(handle, func, line, level, message, ap);
    va_end(ap);
}

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

int __wrap_puts(const char *format, ...)
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

void vdump_module_buffer(void *handle,
                         const char *func,
                         int line,
                         print_level_t level,
                         const void *data,
                         int length,
                         const char *message,
                         va_list list)
{
    string_log_handler(handle, func, line, level, message, list, data, length);
}

/* For LOG_HEXDUMP_*() */
/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM void dump_module_buffer(void *handle,
                                          const char *func,
                                          int line,
                                          print_level_t level,
                                          const void *data,
                                          int length,
                                          const char *message, ...)
{
    va_list ap;

    if (g_syslog_share_variable->cpu_log_switch[GET_CURRENT_CPU_ID()] == DEBUG_LOG_OFF) {
        return;
    }

    if (syslog_port_query_curent_stack_space() < SYSLOG_TEXT_RESERVE_STACK_SPACE) {
        /* stack reserce space is not enough */
        assert(0);
    }

    va_start(ap, message);
    vdump_module_buffer(handle, func, line, level, data, length, message, ap);
    va_end(ap);
}
#else /* MTK_SYSLOG_SUB_FEATURE_STRING_LOG_SUPPORT */
void vprint_module_log(void *handle,
                       const char *func,
                       int line,
                       print_level_t level,
                       const char *message,
                       va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(list);
}
void print_module_log(void *handle,
                      const char *func,
                      int line,
                      print_level_t level,
                      const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
}

int __wrap_printf(const char *format, ...)
{
    PORT_SYSLOG_UNUSED(format);

    return 0;
}

int __wrap_puts(const char *format, ...)
{
    PORT_SYSLOG_UNUSED(format);

    return 0;
}

void vdump_module_buffer(void *handle,
                         const char *func,
                         int line,
                         print_level_t level,
                         const void *data,
                         int length,
                         const char *message,
                         va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(list);
}
void dump_module_buffer(void *handle,
                        const char *func,
                        int line,
                        print_level_t level,
                        const void *data,
                        int length,
                        const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
}
#endif /* MTK_SYSLOG_SUB_FEATURE_STRING_LOG_SUPPORT */

#ifdef MTK_SYSLOG_SUB_FEATURE_BINARY_LOG_SUPPORT

#define MAX_TLV_DUMP_DATA_SIZE              2048
#define MAX_USER_DUMP_BUFFER_COUNT          3
#define MAX_SYSLOG_MSG_ID_PARAMETER_NUMBER  20

/* TCM is mainly for Optimize log printing time */
NO_INLINE ATTR_TEXT_IN_IRAM static int32_t binary_log_handler(void *handle,
                                            print_level_t level,
                                            log_type_t type,
                                            const char *message,
                                            mux_buffer_t *p_buf_info)
{
    uint8_t tlv_header[sizeof(syslog_msgid_t)];
    uint32_t per_cpu_irq_mask, tx_length = 0;

#ifdef FREERTOS_ENABLE
    TaskStatus_t pxTaskStatus;
#endif

#ifdef MTK_DEBUG_PLAIN_LOG_ENABLE
    return 0;
#endif

    /* anti-nest check start */
    if (port_syslog_anti_nest_check_begin() == false) {
        return 0;
    }

    /* check log control */
    if (check_log_control(p_buf_info, (log_control_block_t*)handle, level) == false) {
        /* anti-nest check stop */
        port_syslog_anti_nest_check_end();
        return 0;
    }

    /* Initialize the TLV header */
    if (type == LOG_TYPE_MSG_ID_LOG) {
        syslog_msgid_t *p_syslog_msgid = (syslog_msgid_t*)&tlv_header[0];
        hal_nvic_save_and_set_interrupt_mask(&per_cpu_irq_mask);
        p_syslog_msgid->log_info.log_cpu_id       = GET_CURRENT_CPU_ID();
        p_syslog_msgid->log_info.log_seq_num      = g_syslog_share_variable->sequence[1]++;
        hal_nvic_restore_interrupt_mask(per_cpu_irq_mask);
        p_syslog_msgid->log_info.log_irq_num      = HAL_NVIC_QUERY_EXCEPTION_NUMBER;
#ifdef FREERTOS_ENABLE
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            vTaskGetInfo(xTaskGetCurrentTaskHandle(), &pxTaskStatus, pdFALSE, eInvalid);
            p_syslog_msgid->log_info.log_task_num = pxTaskStatus.xTaskNumber;
        } else {
            p_syslog_msgid->log_info.log_task_num = 0;
        }
#else
        p_syslog_msgid->log_info.log_task_num = 0;
#endif
        p_syslog_msgid->log_info.log_reserve[0]   = (syslog_port_query_init_number() & 0xF);
        p_syslog_msgid->log_info.log_reserve[1]   = 0;
        p_syslog_msgid->log_info.log_timestamp    = port_syslog_get_current_timestamp();
        p_syslog_msgid->msg_offset                = (uint32_t)message - PORT_SYSLOG_MSG_ADDR_OFFSET;
        /* syslog header */
        p_buf_info[1].p_buf = (uint8_t *)tlv_header;
        p_buf_info[1].buf_size = sizeof(syslog_msgid_t);
        tx_length = port_syslog_send(RACE_PROTOCAL_MSGID_LOG, true, p_buf_info);
    } else if ((type > LOG_TYPE_COMMON_LOG_END) && (type < LOG_TYPE_SPECIAL_LOG_END)) {
        p_buf_info[1].p_buf = (uint8_t *)tlv_header;
        p_buf_info[1].buf_size = 0;
        tx_length = port_syslog_send(RACE_PROTOCAL_TLV_LOG | type, true, p_buf_info);
    } else {
        tx_length = 0;
    }

    /* anti-nest check stop */
    port_syslog_anti_nest_check_end();

    return tx_length;
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM void log_print_msgid(void *handle,
                     print_level_t level,
                     const char *message,
                     uint32_t arg_cnt,
                     va_list list)
{
    /* race + log_info + [parm] + NULL */
    mux_buffer_t log_buf_info[4];
    uint32_t i, buffer[MAX_SYSLOG_MSG_ID_PARAMETER_NUMBER];

#ifdef MTK_DEBUG_PLAIN_LOG_ENABLE
    const char *fun = " ";
    vprint_module_log(handle, fun, 0, level, message, list);
    return;
#endif

    if (arg_cnt > MAX_SYSLOG_MSG_ID_PARAMETER_NUMBER) {
        return;
    }

    for (i = 0; i < arg_cnt; i++) {
        buffer[i] = va_arg(list, uint32_t);
    }

    log_buf_info[2].p_buf = (uint8_t*)buffer;
    log_buf_info[2].buf_size = arg_cnt * sizeof(uint32_t);
    log_buf_info[3].p_buf = NULL;

    binary_log_handler(handle, level, LOG_TYPE_MSG_ID_LOG, message, log_buf_info);
}

/* For LOG_MSGID_*() */
/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM void print_module_msgid_log(void *handle,
                                              print_level_t level,
                                              const char *message,
                                              uint32_t arg_cnt, ...)
{
    va_list list;

    if (g_syslog_share_variable->cpu_log_switch[GET_CURRENT_CPU_ID()] == DEBUG_LOG_OFF) {
        return;
    }

    if (syslog_port_query_curent_stack_space() < SYSLOG_MSGID_RESERVE_STACK_SPACE) {
        /* stack reserce space is not enough */
        assert(0);
    }

    va_start(list, arg_cnt);
    log_print_msgid(handle, level, message, arg_cnt, list);
    va_end(list);
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM void print_module_msgid_log_debug(void *handle,
                                                const char *message,
                                                uint32_t arg_cnt, ...)
{
    va_list list;

    if (g_syslog_share_variable->cpu_log_switch[GET_CURRENT_CPU_ID()] == DEBUG_LOG_OFF) {
        return;
    }

    if (syslog_port_query_curent_stack_space() < SYSLOG_MSGID_RESERVE_STACK_SPACE) {
        /* stack reserce space is not enough */
        assert(0);
    }

    va_start(list, arg_cnt);
    log_print_msgid(handle, PRINT_LEVEL_DEBUG, message, arg_cnt, list);
    va_end(list);
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM void print_module_msgid_log_info(void *handle,
                                                const char *message,
                                                uint32_t arg_cnt, ...)
{
    va_list list;

    if (g_syslog_share_variable->cpu_log_switch[GET_CURRENT_CPU_ID()] == DEBUG_LOG_OFF) {
        return;
    }

    if (syslog_port_query_curent_stack_space() < SYSLOG_MSGID_RESERVE_STACK_SPACE) {
        /* stack reserce space is not enough */
        assert(0);
    }

    va_start(list, arg_cnt);
    log_print_msgid(handle, PRINT_LEVEL_INFO, message, arg_cnt, list);
    va_end(list);
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM void print_module_msgid_log_warning(void *handle,
                                                const char *message,
                                                uint32_t arg_cnt, ...)
{
    va_list list;

    if (g_syslog_share_variable->cpu_log_switch[GET_CURRENT_CPU_ID()] == DEBUG_LOG_OFF) {
        return;
    }

    if (syslog_port_query_curent_stack_space() < SYSLOG_MSGID_RESERVE_STACK_SPACE) {
        /* stack reserce space is not enough */
        assert(0);
    }

    va_start(list, arg_cnt);
    log_print_msgid(handle, PRINT_LEVEL_WARNING, message, arg_cnt, list);
    va_end(list);
}

/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM void print_module_msgid_log_error(void *handle,
                                                const char *message,
                                                uint32_t arg_cnt, ...)
{
    va_list list;

    if (g_syslog_share_variable->cpu_log_switch[GET_CURRENT_CPU_ID()] == DEBUG_LOG_OFF) {
        return;
    }

    if (syslog_port_query_curent_stack_space() < SYSLOG_MSGID_RESERVE_STACK_SPACE) {
        /* stack reserce space is not enough */
        assert(0);
    }

    va_start(list, arg_cnt);
    log_print_msgid(handle, PRINT_LEVEL_ERROR, message, arg_cnt, list);
    va_end(list);
}

/* For LOG_TLVDUMP_*() */
/* TCM is mainly for Optimize log printing time */
ATTR_TEXT_IN_IRAM uint32_t dump_module_tlv_buffer(void *handle,
                                                  print_level_t level,
                                                  log_type_t type,
                                                  const void **p_data,
                                                  uint32_t *p_length)
{
    /* race + log_info + [p_data * MAX_USER_DUMP_BUFFER_COUNT] + NULL */
    mux_buffer_t log_buf_info[MAX_USER_DUMP_BUFFER_COUNT + 3];
    uint32_t i, payload_size;

    if (g_syslog_share_variable->cpu_log_switch[GET_CURRENT_CPU_ID()] == DEBUG_LOG_OFF) {
        return 0;
    }

    if (syslog_port_query_curent_stack_space() < SYSLOG_MSGID_RESERVE_STACK_SPACE) {
        /* stack reserce space is not enough */
        assert(0);
    }

    /* Limit the max size of dump data when calling LOG_TLVDUMP_*() */
    for (payload_size = 0, i = 0; p_data[i] != NULL; i++) {
        if (i >= MAX_USER_DUMP_BUFFER_COUNT) {
            return 0;
        }
        payload_size += p_length[i];
        log_buf_info[i + 2].p_buf = (uint8_t*)p_data[i];
        log_buf_info[i + 2].buf_size = p_length[i];
    }
    log_buf_info[i + 2].p_buf = NULL;

    if (g_syslog_share_variable->init_phase == SYSLOG_FULL_INIT) {
        if (payload_size > MAX_TLV_DUMP_DATA_SIZE) {
            return 0;
        }
    }

    return binary_log_handler(handle, level, type, NULL, log_buf_info);
}
#else /* MTK_SYSLOG_SUB_FEATURE_BINARY_LOG_SUPPORT */
void log_print_msgid(void *handle,
                     print_level_t level,
                     const char *message,
                     uint32_t arg_cnt,
                     va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
    PORT_SYSLOG_UNUSED(list);
}
void print_module_msgid_log(void *handle,
                            print_level_t level,
                            const char *message,
                            uint32_t arg_cnt, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
}

uint32_t dump_module_tlv_buffer(void *handle,
                                print_level_t level,
                                log_type_t type,
                                const void **p_data,
                                uint32_t *p_length)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(type);
    PORT_SYSLOG_UNUSED(p_data);
    PORT_SYSLOG_UNUSED(p_length);

    return 0;
}

#endif /* MTK_SYSLOG_SUB_FEATURE_BINARY_LOG_SUPPORT */

#else /* !MTK_DEBUG_LEVEL_NONE */

#ifndef AIR_BTA_IC_PREMIUM_G2
uint32_t syslog_port_query_debug_flag(void)
{
    return 0;
}
#endif

uint32_t query_syslog_port(void)
{
    return 0xFF;
}

bool log_set_filter(void)
{
    /* don't need to actually call the functions to avoid the warnings */
    (void)&port_syslog_memory_remap_to_primary_cpu;
    (void)&port_syslog_build_time_sdk_version_copy;
    (void)&port_syslog_send;
    return false;
}

void filter_config_print_switch(void *handle, log_switch_t log_switch)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(log_switch);
}

void filter_config_print_level(void *handle, print_level_t log_level)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(log_level);
}

void vprint_module_log(void *handle,
                       const char *func,
                       int line,
                       print_level_t level,
                       const char *message,
                       va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(list);
}

void print_module_log(void *handle,
                      const char *func,
                      int line,
                      print_level_t level,
                      const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
}

int __wrap_printf(const char *format, ...)
{
    PORT_SYSLOG_UNUSED(format);

    return 0;
}

int __wrap_puts(const char *format, ...)
{
    PORT_SYSLOG_UNUSED(format);

    return 0;
}

void vdump_module_buffer(void *handle,
                         const char *func,
                         int line,
                         print_level_t level,
                         const void *data,
                         int length,
                         const char *message,
                         va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(list);
}

void dump_module_buffer(void *handle,
                        const char *func,
                        int line,
                        print_level_t level,
                        const void *data,
                        int length,
                        const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
}

void log_print_msgid(void *handle,
                     print_level_t level,
                     const char *message,
                     uint32_t arg_cnt,
                     va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
    PORT_SYSLOG_UNUSED(list);
}

void print_module_msgid_log(void *handle,
                            print_level_t level,
                            const char *message,
                            uint32_t arg_cnt, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
}

uint32_t dump_module_tlv_buffer(void *handle,
                                print_level_t level,
                                log_type_t type,
                                const void **p_data,
                                uint32_t *p_length)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(type);
    PORT_SYSLOG_UNUSED(p_data);
    PORT_SYSLOG_UNUSED(p_length);

    return 0;
}

#endif /* !MTK_DEBUG_LEVEL_NONE */

#else /* !MTK_DEBUG_LEVEL_PRINTF */

typedef struct {
    uint8_t port_index;
} syslog_share_variable_t;

#ifdef MTK_SINGLE_CPU_ENV
ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN static volatile syslog_share_variable_t syslog_share_variable;
static volatile syslog_share_variable_t *g_syslog_share_variable = &syslog_share_variable;
#else
#include "hal_resource_assignment.h"
#define PORT_SYSLOG_SHARE_VARIABLE_BEGIN HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VAR_START
static volatile syslog_share_variable_t *g_syslog_share_variable = (volatile syslog_share_variable_t *)(PORT_SYSLOG_SHARE_VARIABLE_BEGIN);
#endif /* MTK_SINGLE_CPU_ENV */

log_create_module(common, PRINT_LEVEL_INFO);
log_create_module(MPLOG, PRINT_LEVEL_WARNING);
log_create_module(printf, PRINT_LEVEL_INFO);



bool log_set_filter(void)
{
    return false;
}

void filter_config_print_switch(void *handle, log_switch_t log_switch)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(log_switch);
}

void filter_config_print_level(void *handle, print_level_t log_level)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(log_level);
}

void vprint_module_log(void *handle,
                       const char *func,
                       int line,
                       print_level_t level,
                       const char *message,
                       va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(list);
}

void print_module_log(void *handle,
                      const char *func,
                      int line,
                      print_level_t level,
                      const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ int __wrap_printf(const char *format, ...)
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

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
#ifndef MTK_SINGLE_CPU_ENV
    while (hal_hw_semaphore_take(HW_SEMAPHORE_SYSLOG) != HAL_HW_SEMAPHORE_STATUS_OK);
#endif
    uart_send_polling(g_syslog_share_variable->port_index, (uint8_t *)frame_header, log_size);
#ifndef MTK_SINGLE_CPU_ENV
    while (hal_hw_semaphore_give(HW_SEMAPHORE_SYSLOG) != HAL_HW_SEMAPHORE_STATUS_OK);
#endif
    hal_nvic_restore_interrupt_mask(irq_mask);

    return log_size;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ int __wrap_puts(const char *format, ...)
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

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
#ifndef MTK_SINGLE_CPU_ENV
    while (hal_hw_semaphore_take(HW_SEMAPHORE_SYSLOG) != HAL_HW_SEMAPHORE_STATUS_OK);
#endif
    uart_send_polling(g_syslog_share_variable->port_index, (uint8_t *)frame_header, log_size);
#ifndef MTK_SINGLE_CPU_ENV
    while (hal_hw_semaphore_give(HW_SEMAPHORE_SYSLOG) != HAL_HW_SEMAPHORE_STATUS_OK);
#endif
    hal_nvic_restore_interrupt_mask(irq_mask);

    return log_size;
}

void vdump_module_buffer(void *handle,
                         const char *func,
                         int line,
                         print_level_t level,
                         const void *data,
                         int length,
                         const char *message,
                         va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(list);
}

void dump_module_buffer(void *handle,
                        const char *func,
                        int line,
                        print_level_t level,
                        const void *data,
                        int length,
                        const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
}

void log_print_msgid(void *handle,
                     print_level_t level,
                     const char *message,
                     uint32_t arg_cnt,
                     va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
    PORT_SYSLOG_UNUSED(list);
}


void print_module_msgid_log(void *handle,
                            print_level_t level,
                            const char *message,
                            uint32_t arg_cnt, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
}

uint32_t dump_module_tlv_buffer(void *handle,
                                print_level_t level,
                                log_type_t type,
                                const void **p_data,
                                uint32_t *p_length)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(type);
    PORT_SYSLOG_UNUSED(p_data);
    PORT_SYSLOG_UNUSED(p_length);

    return 0;
}

#endif /* !MTK_DEBUG_LEVEL_PRINTF */

