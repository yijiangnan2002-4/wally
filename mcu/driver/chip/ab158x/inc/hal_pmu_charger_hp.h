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


#ifndef __HAL_PMU_CHARGER_H__
#define __HAL_PMU_CHARGER_H__
#ifdef HAL_PMU_MODULE_ENABLED

#define DIGITAL_THERMAL_FUNCTION 1
#define HW_JEITA_HOT_STAGE 0xF
#define HW_JEITA_WARM_STAGE 0xE
#define HW_JEITA_NORMAL_STAGE 0xC
#define HW_JEITA_COOL_STAGE 0x8
#define HW_JEITA_COLD_STAGE 0
/*==========[Basic function]==========*/
void pmu_charger_init_hp(uint16_t precc_cur,uint16_t cv_termination);
void pmu_trim_ic_init(void);
bool pmu_get_chr_detect_value_hp(void);
void pmu_charger_check_faston_hp(void);
void pmu_enable_safety_timer_hp(pmu_safety_timer_t tmr, pmu_power_operate_t oper);
void pmu_set_safety_timer_hp(pmu_safety_timer_t tmr, uint8_t index);
/*==========[charger function]==========*/
uint8_t pmu_vsys_shift(uint8_t temp_trim);
uint8_t pmu_enable_charger_hp(uint8_t isEnableCharging);
bool pmu_enable_sysldo(bool isEnableSysdo);
uint32_t pmu_get_charger_state_hp(void);
void pmu_set_icl_tstep(pmu_icl_tstep_t val);
void pmu_set_icl_by_type_hp(uint8_t port);
void pmu_set_charger_current_limit_hp(pmu_icl_level_t icl_value);
bool pmu_set_rechg_voltage_hp(uint8_t rechargeVoltage);
bool pmu_enable_recharger_hp(bool isEnableRecharge);
bool pmu_set_extend_charger_time_hp(uint8_t timeMins);
/*==========[Battery parameter]==========*/
bool pmu_select_cv_voltage_hp(uint8_t voltage);
void pmu_select_precc_voltage_hp(uint8_t voltage);
bool pmu_select_cc_safety_timer_hp(uint8_t timeMHrs);
void pmu_set_icc_gain(pmu_fastcc_chrcur_t cur);
void pmu_set_pre_charger_current_hp(pmu_fastcc_chrcur_t cur);
void pmu_set_charger_current_hp(pmu_fastcc_chrcur_t cur);
void pmu_set_iterm_current_irq_hp(pmu_iterm_chrcur_t cur);
void pmu_set_iterm_current_hp(pmu_iterm_chrcur_t cur);
int pmu_get_charger_current_index(void);
/*==========[HW-JEITA]==========*/
bool pmu_set_hw_jeita_enable_hp(uint8_t value);
void pmu_hw_jeita_init_hp(void);
pmu_operate_status_t pmu_set_jeita_voltage_hp(uint32_t auxadcVolt, uint8_t JeitaThreshold);
void pmu_set_jeita_state_setting_hp(uint8_t state,pmu_jeita_perecnt_level_t ICC_JC,pmu_cv_voltage_t vol);
uint8_t pmu_get_hw_jeita_status_hp(void);
void pmu_select_eoc_option_operating_hp(pmu_eoc_option_t opt, pmu_eoc_operating_t oper);
void pmu_charger_eoc4_setting(void);
void pmu_charger_eoc4_exit(void);
void pmu_eoc_option_setting(pmu_eoc_option_t opt);
void pmu_control_pmic_protect(uint8_t tstWKeymode);
uint32_t pmu_get_faston_flag(void);
uint32_t pmu_get_vsys_dpm_status(void);
uint32_t pmu_get_vbus_dpm_status(void);
/*==========[Other]==========*/
void pmu_set_vbus_debounce_time_hp(uint8_t value);
void pmu_enable_powerhold_hp(uint8_t value);
void pmu_ovp_debug(void);
#endif /* End of HAL_PMU_MODULE_ENABLED */
#endif /* End of __HAL_PMU_CHARGER_H__*/

