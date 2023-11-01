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

#include "hal_dvfs.h"

#ifdef HAL_DVFS_MODULE_ENABLED
#include "hal.h"
#include "hal_dvfs_internal.h"
#include "hal_nvic_internal.h"
#include "hal_clock.h"
extern dvfs_opp_t dvfs_domain;

#if defined(AIR_BOOT_SYS_CLK_HIGH)
    #if !defined(AIR_MAX_SYS_CLK_HIGH)
        #error "AIR DVFS MAX/MIN Level Define Conflict"
    #endif
#elif defined(AIR_BOOT_SYS_CLK_MID)
    #if defined(AIR_MAX_SYS_CLK_LOW)
        #error "AIR DVFS MAX/MIN Level Define Conflict"
    #endif
#endif

/* Return a DVFS opp index that best matches the conditions specified
 *   request_cpu_khz: target cpu freq in KHz
 *   relation: lower or upper bound
 */
int dvfs_query_frequency(uint32_t request_cpu_khz, hal_dvfs_freq_relation_t relation)
{
    log_hal_msgid_info("dvfs_query_frequency() cpu :%uKHz, relation:%d\r\n", 2, request_cpu_khz, relation);
    for (dvfs_frequency_t opp_idx = dvfs_domain.min_opp_idx; opp_idx < HAL_DVFS_OPP_NUM; opp_idx++) {
        if (dvfs_domain.cpu_khz_arr[opp_idx] == request_cpu_khz) {
            log_hal_msgid_info("Chosen DVFS OPP idx %d\r\n", 1, opp_idx);
            return opp_idx;
        }
    }
    log_hal_msgid_info("Currently only support exact DVFS OPP cpu freq\r\n", 0);
    log_hal_msgid_info("Warning, requested cpu khz not found, set to lowest DVFS opp idx\r\n", 0);
    return dvfs_domain.min_opp_idx;
}
/* Due to critical section shouldn't print logs
 *
 * Caller should print related log (Ex: actual opp_idx after dvfs switch)
 * */
static hal_dvfs_status_t dvfs_target_frequency(hal_dvfs_lock_parameter_t type, dvfs_frequency_t request_opp_idx, uint32_t xLinkRegAddr)
{
    hal_dvfs_status_t ret = HAL_DVFS_STATUS_OK;
    dvfs_frequency_t actual_opp_idx;
    ret = dvfs_domain.request_proc(type, request_opp_idx, &actual_opp_idx);
    /* Actual request idx, may not be the same as actual DVFS OPP idx after DVFS
     * Due to this reason, print actual_opp_idx
     * - DVFS unlock
     * - DVFS lock (but is currently locked to a higher OPP)
     */
    if (type == HAL_DVFS_LOCK) {
        log_hal_msgid_info("[DVFS] LOCK   done,  caller[0x%x], current OPP[%d], CPU[%d]KHz\r\n", 3, xLinkRegAddr, actual_opp_idx, dvfs_domain.cpu_khz_arr[actual_opp_idx]);
    } else if (type == HAL_DVFS_UNLOCK) {
        log_hal_msgid_info("[DVFS] UNLOCK done,  caller[0x%x], current OPP[%d], CPU[%d]KHz\r\n", 3, xLinkRegAddr, actual_opp_idx, dvfs_domain.cpu_khz_arr[actual_opp_idx]);
    }
    return ret;
}
/* hal_dvfs_lock_control: only process 1 request at a time */
hal_dvfs_status_t hal_dvfs_lock_control(dvfs_frequency_t freq_idx, hal_dvfs_lock_parameter_t lock)
{
    uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);

    if((freq_idx > dvfs_domain.max_opp_idx) || (freq_idx < dvfs_domain.min_opp_idx)) {
        log_hal_msgid_error("[DVFS] Invalid param FreqIdx %d (Valid %d~%d), Lock %d, Caller 0x%08X", 5, freq_idx, dvfs_domain.min_opp_idx, dvfs_domain.max_opp_idx, lock, xLinkRegAddr);
        return HAL_DVFS_STATUS_INVALID_PARAM;
    }

    if (lock == HAL_DVFS_LOCK) {
        log_hal_msgid_info("[DVFS] LOCK   start, caller[0x%x], request OPP[%d], CPU[%d]KHz\r\n", 3, xLinkRegAddr, freq_idx, dvfs_domain.cpu_khz_arr[freq_idx]);
    } else if (lock == HAL_DVFS_UNLOCK) {
        log_hal_msgid_info("[DVFS] UNLOCK start, caller[0x%x], request OPP[%d], CPU[%d]KHz\r\n", 3, xLinkRegAddr, freq_idx, dvfs_domain.cpu_khz_arr[freq_idx]);
    }
    switch (lock) {
        case HAL_DVFS_LOCK:
        case HAL_DVFS_UNLOCK:
            dvfs_target_frequency(lock, freq_idx, xLinkRegAddr);
            break;
        default:
            log_hal_msgid_info("Invalid hal_dvfs_lock_parameter %d\r\n", 1, (int32_t)lock);
            return HAL_DVFS_STATUS_INVALID_PARAM;
    }
    return HAL_DVFS_STATUS_OK;
}
uint32_t hal_dvfs_get_cpu_frequency(void)
{
    return dvfs_domain.cpu_khz_arr[dvfs_domain.cur_opp_idx];
}
hal_dvfs_status_t hal_dvfs_get_cpu_frequency_list(const uint32_t **list, uint32_t *list_num)
{
    *list = dvfs_domain.cpu_khz_arr;
    *list_num = dvfs_domain.opp_num;
    return HAL_DVFS_STATUS_OK;
}
hal_dvfs_status_t hal_dvfs_target_cpu_frequency(uint32_t target_freq, hal_dvfs_freq_relation_t relation)
{
    uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);
    dvfs_frequency_t next_idx = (dvfs_frequency_t)dvfs_query_frequency(target_freq, relation);
    return dvfs_target_frequency(HAL_DVFS_LOCK, next_idx, xLinkRegAddr);
}
/* hal_dvfs_init()
 * - switches to minimum defined dvfs opp (including voltage switch)
 */
hal_dvfs_status_t hal_dvfs_init(void)
{
    log_hal_msgid_info("hal_dvfs_init()\r\n", 0);
    //dvfs_domain_init();

    /* Lock Vcore for the first time (assume Vcore is the highest) */
    dvfs_domain.cur_opp_idx = HAL_DVFS_OPP_HIGH;
    pmu_lock_vcore(PMU_NORMAL, dvfs_domain.vcore_arr[dvfs_domain.cur_opp_idx], PMU_LOCK);

    /* lock dvfs for the 1st time */
    hal_dvfs_lock_control(dvfs_domain.min_opp_idx, HAL_DVFS_LOCK);

    return HAL_DVFS_STATUS_OK;
}
void dvfs_debug_dump(void) {}

#endif /* HAL_DVFS_MODULE_ENABLED */

