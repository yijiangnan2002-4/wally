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

#include "hal_dvfs_internal.h"

#ifdef HAL_DVFS_MODULE_ENABLED
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "hal.h"
#include "hal_gpt.h"
#include "hal_clock.h"
#include "hal_pmu.h"
#include "hal_dvfs_internal.h"
#include "hal_clock_internal.h"
#include "hal_nvic_internal.h"
#ifdef  HAL_DVFS_DEBUG_ENABLE
#define dvfs_debug(_message,...) log_hal_info("[DVFS] "_message, ##__VA_ARGS__)
#else
#define dvfs_debug(_message,...)
#endif
/* For the below chip types, DVS is not supported */
#if defined(AIR_BTA_IC_PREMIUM_G3_TYPE_D)
#define DFS_ONLY
#endif

/* If DFS_ONLY is defined, it will only perform DFS */
#ifdef DFS_ONLY
#undef HAL_PMU_MODULE_ENABLED
#endif

#define DVFS_100_MS                                 100000
static hal_dvfs_status_t dvfs_switch_frequency(dvfs_frequency_t next_opp);
static hal_dvfs_status_t dvfs_switch_voltage(unsigned int n_voltage);
hal_dvfs_status_t dvfs_request_proc(hal_dvfs_lock_parameter_t type, dvfs_frequency_t request_opp_idx, dvfs_frequency_t *selected_opp_idx);
/* Expected DVFS CPU freq table */
#if defined(AIR_RFI_SUPPRESS_DISABLE)
typedef enum {
    DVFS_OPP_LOW_CPU_FREQ = 104000,
    DVFS_OPP_MID_CPU_FREQ = 156000,
    DVFS_OPP_HIGH_CPU_FREQ = 260000
} dvfs_opp_cpu_freq_t;
#else
typedef enum {
    DVFS_OPP_LOW_CPU_FREQ = 99750,
    DVFS_OPP_MID_CPU_FREQ = 149625,
    DVFS_OPP_HIGH_CPU_FREQ = 260000
} dvfs_opp_cpu_freq_t;
#endif
//------------[DVFS Variable Declaration]
static const uint32_t dvfs_cpu_khz_arr[HAL_DVFS_OPP_NUM] = {
    DVFS_OPP_LOW_CPU_FREQ,
    DVFS_OPP_MID_CPU_FREQ,
    DVFS_OPP_HIGH_CPU_FREQ
};


#ifdef HAL_PMU_MODULE_ENABLED
ATTR_RWDATA_IN_TCM static pmu_power_vcore_voltage_t dvfs_vcore_arr[HAL_DVFS_OPP_NUM] = {
    PMIC_VCORE_0P55_V,
    PMIC_VCORE_0P65_V,
    PMIC_VCORE_0P80_V
};
#endif
ATTR_RWDATA_IN_TCM dvfs_opp_t dvfs_domain = {
    .opp_num = HAL_DVFS_OPP_NUM,
#ifdef HAL_PMU_MODULE_ENABLED
    .vcore_arr = dvfs_vcore_arr,          // table that stores targeted vcore for each DVFS OPP
#endif
    .cpu_khz_arr = dvfs_cpu_khz_arr,    // table that stores targeted CPU freq for each DVFS OPP
    /* frequency, voltage switching functions */
    .switch_voltage = dvfs_switch_voltage,//function about switch voltage
    .switch_frequency = dvfs_switch_frequency,// function about switch frequency (currently also switches vcore)
    .request_proc = dvfs_request_proc,
    .cur_opp_idx = HAL_DVFS_OPP_HIGH,    /* default set to high, since bootloader should set to highest DVFS opp after bringup */

#if defined(AIR_MAX_SYS_CLK_HIGH)
    .max_opp_idx = HAL_DVFS_OPP_HIGH,
#elif defined(AIR_MAX_SYS_CLK_MID)
    .max_opp_idx = HAL_DVFS_OPP_MID,
#elif defined(AIR_MAX_SYS_CLK_LOW)
    .max_opp_idx = HAL_DVFS_OPP_LOW,
#else
#error "AIR_MAX_SYS_CLK_XXX not defined"
#endif

#if defined(AIR_BOOT_SYS_CLK_HIGH)
    .min_opp_idx = HAL_DVFS_OPP_HIGH,
#elif defined(AIR_BOOT_SYS_CLK_MID)
    .min_opp_idx = HAL_DVFS_OPP_MID,
#elif defined(AIR_BOOT_SYS_CLK_LOW)
    .min_opp_idx = HAL_DVFS_OPP_LOW,
#else
#error "AIR_BOOT_SYS_CLK_XXX not defined"
#endif

//    .dvfs_switch_interval = 0,      // timer for keeping count of dvfs switch caller interval
    // If interval is too small, it may block bluetooth transmission
    .opp_ref_cnt_arr = {0} /* initialize opp ref cnt array to 0 */
};

//#define DVFS_CRITICAL_SECTION_TIME_MEASURE
#ifdef DVFS_CRITICAL_SECTION_TIME_MEASURE
static uint32_t critical_time_start, critical_time_end;
#endif

#if defined(HAL_DVFS_DEBUG_ENABLE)
static void ref_cnt_dbg(void)
{
    for (dvfs_frequency_t opp_idx = 0; opp_idx < HAL_DVFS_OPP_NUM; opp_idx++) {
        log_hal_msgid_info("opp_idx %d: %d \r\n", 2, (int)opp_idx, (int)dvfs_domain.opp_ref_cnt_arr[opp_idx]);
    }
}
#endif

/* dvfs_opp_ref_cnt_proc: increment/decrement the requested DVFS OPP ref count
 * This function should only be called in dvfs_switch_frequency (it is already protected by dvfs_mutex)
 * process reference count
 * - LOCK: opp ref_cnt++
 * - UNLOCK: opp ref_cnt--
 * (Check for over/underflow, and assert if it occurs)
 * ref_cnt_arr defined as signed for easier error detection
 */
static inline hal_dvfs_status_t dvfs_opp_ref_cnt_proc(hal_dvfs_lock_parameter_t request_type, dvfs_frequency_t request_opp_idx)
{
    if (request_type == HAL_DVFS_LOCK) {
        /* Check if reference count is overflow */
        if (dvfs_domain.opp_ref_cnt_arr[request_opp_idx] == 127) {
            //log_hal_msgid_error("[DVFS] LOCK: OppRefCnt overflow err! OppRefCnt[%d] > 127", 1, request_opp_idx);
            return HAL_DVFS_STATUS_ERROR;
        }
        dvfs_domain.opp_ref_cnt_arr[request_opp_idx]++;
    } else { /*HAL_DVFS_UNLOCK */
        /* Check if reference count is smaller than 0 (this means someone ref count call is unbalanced) */
        if (dvfs_domain.opp_ref_cnt_arr[request_opp_idx] == 0) {
            //log_hal_msgid_error("[DVFS] UNLOCK: OppRefCnt err! OppRefCnt[%d] < 0", 1, request_opp_idx);
            return HAL_DVFS_STATUS_ERROR;
        }
        dvfs_domain.opp_ref_cnt_arr[request_opp_idx]--;
    }

#if defined(HAL_DVFS_DEBUG_ENABLE)
    ref_cnt_dbg();
#endif
    return HAL_DVFS_STATUS_OK;
}

/* Loop through dvfs opp reference count, and select/return the highest opp idx with reference count != 0
 * If all reference count is 0: return lowest allowed dvfs instead
 */
static inline hal_dvfs_status_t dvfs_next_opp_sel(dvfs_frequency_t *next_opp_idx)
{
    /* HAL_DVFS_OPP_NUM - 1: should indicate highest dvfs opp index in reference count array */
#if defined(HAL_DVFS_DEBUG_ENABLE)
    log_hal_msgid_info("dvfs_next_opp_sel()\r\n", 0);
    ref_cnt_dbg();
#endif

    for (int8_t opp_sel_idx = dvfs_domain.max_opp_idx; opp_sel_idx >= dvfs_domain.min_opp_idx; opp_sel_idx--) {
        if (dvfs_domain.opp_ref_cnt_arr[opp_sel_idx] != 0) {
            *next_opp_idx = opp_sel_idx;
            return HAL_DVFS_STATUS_OK;
        }
    }

    /* If code flow reaches here, it means all dvfs opp reference count is 0, which is unexpected
     * system locks a defined DVFS_OPP during sys_init() and doesn't unlock itself: at least 1 DVFS ref cnt should be 1
     */
    //log_hal_msgid_info("[DVFS] unexpected err, DVFS opp ref cnt all 0! (unbalanced API usage)\r\n", 1, dvfs_domain.min_opp_idx);

    return HAL_DVFS_STATUS_ERROR;
}

//------------[DVFS basic setting api]
/* dvfs_switch_voltage - function that performs voltage switching */
static ATTR_TEXT_IN_TCM hal_dvfs_status_t dvfs_switch_voltage(unsigned int next_voltage_idx)
{
#ifdef HAL_PMU_MODULE_ENABLED
    pmu_power_vcore_voltage_t vcore_current = dvfs_domain.vcore_arr[dvfs_domain.cur_opp_idx];
    pmu_power_vcore_voltage_t vcore_next = dvfs_domain.vcore_arr[next_voltage_idx];

    if (vcore_current != vcore_next) {
        pmu_lock_vcore(PMU_NORMAL, vcore_next, PMU_LOCK);
        pmu_lock_vcore(PMU_NORMAL, vcore_current, PMU_UNLOCK);
    }
#endif

    return HAL_DVFS_STATUS_OK;
}

#if 0
/* This function is called during FreeRTOS sys_init (non-reentrant), so it is not protected by critical section*/
void dvfs_domain_init(void)
{

    return;
}
#endif

ATTR_TEXT_IN_TCM static hal_dvfs_status_t dvfs_switch_frequency(dvfs_frequency_t next_opp_idx)
{
    dvfs_pre_proc(next_opp_idx); /* Enable clock resources */
    dvfs_switch_clock_freq(next_opp_idx); /* Perform mux switch */
    dvfs_post_proc(); /* Disable unused clock resources */
    return 0;
}

/* actual_opp_idx, used for printing actual_opp_idx in the caller function
 * (since requested DVFS opp may not be the actual opp after DVFS request due to dvfs reference count)
 */
ATTR_TEXT_IN_TCM hal_dvfs_status_t dvfs_request_proc(hal_dvfs_lock_parameter_t request_type, dvfs_frequency_t request_opp_idx, dvfs_frequency_t *selected_opp_idx)
{
    uint32_t irq_mask = 0;//, dvfs_cur_ticks = 0, time_diff_us;
    //uint8_t intv_too_short = 0;

    hal_dvfs_status_t result = HAL_DVFS_STATUS_OK;
#ifdef DVFS_CRITICAL_SECTION_TIME_MEASURE
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &critical_time_start);
#endif
    hal_nvic_save_and_set_interrupt_mask_special(&irq_mask);
    /* ================ Critical Section Start ======================== */
#if 0
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &dvfs_cur_ticks);
    time_diff_us = dvfs_cur_ticks - dvfs_domain.dvfs_switch_interval;
    if (!dvfs_domain.dvfs_switch_interval) {
        /* first dvfs switch */
        dvfs_domain.dvfs_switch_interval = dvfs_cur_ticks;
    } else if (time_diff_us < DVFS_100_MS) {
        /* Don't print log in critical section, use flag to record and print later on */
        intv_too_short = 1;
    }
    dvfs_domain.dvfs_switch_interval = dvfs_cur_ticks;
#endif
    if(dvfs_opp_ref_cnt_proc(request_type, request_opp_idx) != HAL_DVFS_STATUS_OK) {
        hal_nvic_restore_interrupt_mask_special(irq_mask);
        log_hal_msgid_error("[DVFS] LockType %d: OppRefCnt[%d]=%d", 3, request_type, request_opp_idx, dvfs_domain.opp_ref_cnt_arr[request_opp_idx]);
        assert(0);
    }
    if(dvfs_next_opp_sel(selected_opp_idx) != HAL_DVFS_STATUS_OK) {
        hal_nvic_restore_interrupt_mask_special(irq_mask);
        log_hal_msgid_error("[DVFS] OppRefCnt all 0", 0);
        assert(0);
    }
#if defined(MTK_DEBUG_LEVEL_INFO)
    /* for ref cnt debug log usage, Note: debug log is currently hardcoded to assume HAL_DVFS_OPP_NUM == 3
     * value assigned during reference count array backup
     */
    uint8_t ref_cnt_arr_after_proc[HAL_DVFS_OPP_NUM];

    /* backup reference count arr (for dbg log after exiting critical section) */
    for (dvfs_frequency_t opp_idx = 0; opp_idx < HAL_DVFS_OPP_NUM; opp_idx++) {
        ref_cnt_arr_after_proc[opp_idx] = dvfs_domain.opp_ref_cnt_arr[opp_idx];
    }
#endif

    //log_hal_msgid_info("DVFS selected_opp_idx %d, curr_idx %d \r\n", 2, selected_opp_idx, dvfs_domain.cur_opp_idx);
    /* DVFS switch flow is only required when
     * actual DVFS OPP is NOT the same as the current one
     */
    if (*selected_opp_idx > dvfs_domain.cur_opp_idx) {
        /* DVFS OPP ramp up
         * (cpu interrupt is disabled during freq, voltage switch)
         */
        dvfs_switch_voltage(*selected_opp_idx);
        dvfs_switch_frequency(*selected_opp_idx);
        dvfs_domain.cur_opp_idx = *selected_opp_idx;
    } else if (*selected_opp_idx < dvfs_domain.cur_opp_idx) {
        /* DVFS OPP ramp down
         * (cpu interrupt is disabled during freq, voltage switch)
         */
        dvfs_switch_frequency(*selected_opp_idx);
        dvfs_switch_voltage(*selected_opp_idx);
        dvfs_domain.cur_opp_idx = *selected_opp_idx;
    }
    /* ================ Critical Section End ======================== */
    hal_nvic_restore_interrupt_mask_special(irq_mask);
#ifdef DVFS_CRITICAL_SECTION_TIME_MEASURE
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &critical_time_end);
    log_hal_msgid_info("[DVFS] Critical Section Time: %dus", 1, critical_time_end - critical_time_start);
#endif
    /* calling DVFS too frequently may block other HW module interrupt processing, just print warning for now */
    //if (intv_too_short) {
    //    log_hal_msgid_info("DVFS INTERVAL [%d]us, TOO SHORT!\r\n", 1, time_diff_us);
    //}

#if defined(MTK_DEBUG_LEVEL_INFO)
    /* DVFS opp ref cnt debug log: assumes DVFS_OPP_NUM to 3 */
    log_hal_msgid_info("[DVFS] ref cnt after proc [%d, %d, %d]\r\n", 3, ref_cnt_arr_after_proc[0], ref_cnt_arr_after_proc[1], ref_cnt_arr_after_proc[2]);
#endif

    return result;
}

#endif /* HAL_DVFS_MODULE_ENABLED */

