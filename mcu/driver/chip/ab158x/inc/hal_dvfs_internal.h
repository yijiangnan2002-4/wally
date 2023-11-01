/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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
#ifndef __HAL_DVFS_INTERNAL_H__
#define __HAL_DVFS_INTERNAL_H__
#include "hal_platform.h"
#ifdef HAL_DVFS_MODULE_ENABLED
#include "hal.h"
#include "memory_attribute.h"
#include "hal_pmu.h"
#include "hal_clock_internal.h"
//#define HAL_DVFS_DEBUG_ENABLE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define DVFS_MODE_NONE (-1)
#define DVFS_NULL_HANDLE 0
typedef struct dvfs_lock_t {
    const char *name;
    unsigned int count;
    uint8_t lock_index;
    uint32_t dvfs_module_index;
    int count_0P55;
    int count_0P65;
    int count_0P8;
} dvfs_lock_t;
typedef enum {
    DVFS_0P55_VOLT_LV,
    DVFS_0P65V_VOLT_LV,
    DVFS_0P8V_VOLT_LV,
    DVFS_CURRENT_SPEED,
    DVFS_ERROR_SPEED,
} dvfs_voltage_mode_t;
typedef struct {
    dvfs_frequency_t opp_num;
#ifdef HAL_PMU_MODULE_ENABLED
    const pmu_power_vcore_voltage_t *vcore_arr; /* DVFS Vore array for each OPP */
#endif
    const uint32_t *cpu_khz_arr; /* DVFS CPU freq array for each OPP */
    hal_dvfs_status_t (*switch_voltage)(unsigned int); /* voltage switch function */
    hal_dvfs_status_t (*switch_frequency)(dvfs_frequency_t); /* frequency switch function */
    /* request_proc() processes whole DVFS request flow (including voltage, freq switch), reference count etc.. */
    hal_dvfs_status_t (*request_proc)(hal_dvfs_lock_parameter_t type, dvfs_frequency_t, dvfs_frequency_t *);
    dvfs_frequency_t max_opp_idx;
    dvfs_frequency_t min_opp_idx;
    dvfs_frequency_t cur_opp_idx;
//    uint32_t dvfs_switch_interval;
    int8_t opp_ref_cnt_arr[HAL_DVFS_OPP_NUM]; /* defined as signed int for easier over/underflow detection */
} dvfs_opp_t;
#define dvfs_enter_privileged_level() \
    do { \
        register uint32_t control = __get_CONTROL(); \
        CONTROL_Type pControl; \
        *(uint32_t *)&pControl = control; \
        if (pControl.b.SPSEL == 1) { \
            /* Alter MSP as stack pointer. */ \
            dvfs_switched_to_privileged = TRUE; \
            pControl.b.SPSEL = 0; \
            control = *(uint32_t *)&pControl; \
            __ISB(); \
            __DSB(); \
            __set_CONTROL(control); \
            __ISB(); \
            __DSB(); \
        } \
    } while(0)
#define dvfs_exit_privileged_level() \
    do { \
        register uint32_t control = __get_CONTROL(); \
        CONTROL_Type pControl; \
        if (dvfs_switched_to_privileged == TRUE) { \
            *(uint32_t *)&pControl = control; \
            dvfs_switched_to_privileged = FALSE; \
            pControl.b.SPSEL = 1; \
            control = *(uint32_t *)&pControl; \
            __ISB(); \
            __DSB(); \
            __set_CONTROL(control); \
            __ISB(); \
            __DSB(); \
        } \
    } while(0)
void dvfs_domain_init(void);
extern bool dvfs_switched_to_privileged;
void dvfs_debug_dump(void);
int dvfs_query_frequency(uint32_t freq, hal_dvfs_freq_relation_t relation);
#endif /* HAL_DVFS_MODULE_ENABLED */
#endif /* __HAL_DVFS_INTERNAL_H__ */
