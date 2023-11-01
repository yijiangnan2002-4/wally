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

#include "hal.h"
#ifdef HAL_PMU_MODULE_ENABLED
#include "assert.h"
#include "hal_pmu_internal_lp.h"
#include "hal_pmu_charger_lp.h"
#include "hal_pmu_auxadc_lp.h"
#include "hal_pmu_lp_platform.h"
#include "hal_pmu_cal_lp.h"
#include "hal_hw_semaphore.h"
#include "hal_resource_assignment.h"

#define AUXADC_RDY1_TIMEOUT    10000
#define AUXADC_BUSY_TIMEOUT    200
#define AUXADC_RDY2_TIMEOUT    10000
#define MAX_ERROR_COUNT        10
uint16_t vbat_val = 0, vchg_val = 0;
uint8_t adc_state = 0, sar_state = 0, adc_next_state = 0, sar_next_state = 0;
uint8_t err_cnt = 0;

void pmu_auxadc_init(void)
{
    pmu_set_register_value_lp(ADC_SW_RST_ADDR, ADC_SW_RST_MASK, ADC_SW_RST_SHIFT, 0x1);
    hal_gpt_delay_us(120);
    pmu_set_register_value_lp(PMU_FLANCTER_RST_ADDR, PMU_FLANCTER_RST_MASK,PMU_FLANCTER_RST_SHIFT, 0x1);
    hal_gpt_delay_us(120);
    pmu_set_register_value_lp(ADC_AVG0_ADDR, ADC_AVG0_MASK, ADC_AVG0_SHIFT, 0x3);
    pmu_set_register_value_lp(ADC_ONE_SHOT_START_ADDR, ADC_ONE_SHOT_START_MASK, ADC_ONE_SHOT_START_SHIFT, 1);//set ready bit
}
void auxadc_hang_checker(void)
{
    adc_next_state = pmu_get_register_value_lp(ADC_STATE_ADDR, ADC_STATE_MASK, ADC_STATE_SHIFT);
    sar_next_state = pmu_get_register_value_lp(SAR_STATE_ADDR, SAR_STATE_MASK, SAR_STATE_SHIFT);
    if(adc_next_state == 0 && sar_next_state == 0){
        err_cnt = 0;
        adc_state = adc_next_state;
        sar_state = sar_next_state;
        return;
    }else if(adc_state == adc_next_state && sar_state == sar_next_state){
        err_cnt += 1;
        log_hal_msgid_info("[ADC_STATE] adc_state[%d], sar_state[%d], adc_next[%d], sar_next[%d], err_cnt[%d]", 5, adc_state, sar_state, adc_next_state, sar_next_state, err_cnt);
    }else{
        err_cnt = 0;
        adc_state = adc_next_state;
        sar_state = sar_next_state;
        return;
    }
    if(err_cnt > MAX_ERROR_COUNT){
        log_hal_msgid_error("[PMU_ADC]auxadc hang, trigger ADC_SW_RST, adc_state[%d], sar_state[%d]", 2, adc_state, sar_state);
        pmu_set_register_value_lp(ADC_SW_RST_ADDR, ADC_SW_RST_MASK, ADC_SW_RST_SHIFT, 0x1);
        err_cnt = 0;
        adc_state = 0;
        sar_state = 0;
        hal_gpt_delay_us(120);
        //assert(0);
    }
}
uint32_t pmu_auxadc_get_channel_value_lp(pmu_adc_channel_t Channel)
{
    uint16_t adc_val = 0, err_sta = 0;
    uint32_t ts = 0, t1 = 0, t2 = 0;
#ifdef HAL_HW_SEMAPHORE_MODULE_ENABLED
    if(hal_hw_semaphore_take(HW_SEMAPHORE_PMU)==HAL_HW_SEMAPHORE_STATUS_OK){
#endif
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &ts);
        while (1) {
            if (pmu_get_register_value_lp(ADC_RDY_STS0_ADDR, ADC_RDY_STS0_MASK, ADC_RDY_STS0_SHIFT)) {
                break;
            } else {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &t2);
                if (t2 - ts > AUXADC_RDY1_TIMEOUT) {
                    err_sta |= 0x1;
                    break;
                }
            }
        }
        if(err_sta & 1){
            auxadc_hang_checker();
        }else{
            err_cnt = 0;
        }
        pmu_set_register_value(ADC_CH_EN_ADDR, ADC_CH_EN_MASK, ADC_CH_EN_SHIFT, (1<<Channel));
        if (pmu_get_register_value_lp(ADC_ONE_SHOT_START_ADDR, ADC_ONE_SHOT_START_MASK, ADC_ONE_SHOT_START_SHIFT) == 0) {
            pmu_set_register_value_lp(ADC_ONE_SHOT_START_ADDR, ADC_ONE_SHOT_START_MASK, ADC_ONE_SHOT_START_SHIFT, 1);
            hal_gpt_delay_us(120);
            if(pmu_get_register_value_lp(ADC_ONE_SHOT_START_ADDR, ADC_ONE_SHOT_START_MASK, ADC_ONE_SHOT_START_SHIFT)){
                log_hal_msgid_error("[PMU_ADC]ADC_ONE_SHOT_START stuck, trigger PMU_FLANCTER_RST", 0);
                pmu_set_register_value_lp(PMU_FLANCTER_RST_ADDR, PMU_FLANCTER_RST_MASK,PMU_FLANCTER_RST_SHIFT, 0x1);
                hal_gpt_delay_us(120);
                //assert(0);
            }
        }

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &t1);
        while (1) {
            if (pmu_get_register_value_lp(ADC_RDY_STS0_ADDR, ADC_RDY_STS0_MASK, ADC_RDY_STS0_SHIFT)) {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &t2);
                if (t2 - t1 > AUXADC_BUSY_TIMEOUT) {
                    err_sta |= 0x2;
                    break;
                }
            } else {
                break;
            }

        }
        if(err_sta & 2){
            auxadc_hang_checker();
        }
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &t1);
        while (1) {
            if (pmu_get_register_value_lp(ADC_RDY_STS0_ADDR, ADC_RDY_STS0_MASK, ADC_RDY_STS0_SHIFT)) {
                break;
            } else {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &t2);
                if (t2 - t1 > AUXADC_RDY2_TIMEOUT) {
                    err_sta |= 0x4;
                    break;
                }
            }
        }
        if(err_sta & 4){
            auxadc_hang_checker();
        }else{
            err_cnt = 0;
        }
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &t2);
#ifdef HAL_HW_SEMAPHORE_MODULE_ENABLED
        if(HAL_HW_SEMAPHORE_STATUS_OK !=hal_hw_semaphore_give(HW_SEMAPHORE_PMU)) {
                log_hal_msgid_error("[PMU_ADC]HW Semaphore give failed", 0);
        }
    }
#endif
    switch (Channel) {
        case PMU_AUX_VBAT:
            adc_val = pmu_get_register_value_lp(ADC_VALUE1_ADDR, ADC_VALUE1_MASK, ADC_VALUE1_SHIFT);
#ifdef AIR_NVDM_ENABLE
#ifndef AIR_PMU_DISABLE_CHARGER
            adc_val = pmu_bat_adc_to_volt(adc_val);
#endif
#endif
            log_hal_msgid_info("[PMU_ADC]vbat_volt[%d], err_sta[%d], time[%dus]", 3, adc_val, err_sta, (t2 - ts));
            break;

        case PMU_AUX_VCHG:
            adc_val = pmu_get_register_value_lp(ADC_VALUE2_ADDR, ADC_VALUE2_MASK, ADC_VALUE2_SHIFT);
#ifdef AIR_NVDM_ENABLE
#ifndef AIR_PMU_DISABLE_CHARGER
            adc_val = pmu_chg_vchg_to_volt(adc_val);
#endif
#endif
            log_hal_msgid_info("[PMU_ADC]vchg_volt[%d], err_sta[%d], time[%dus]", 3, adc_val, err_sta, (t2 - ts));
            break;

        default:
            break;
    }

    return adc_val;
}
#endif /* HAL_PMU_MODULE_ENABLED */
