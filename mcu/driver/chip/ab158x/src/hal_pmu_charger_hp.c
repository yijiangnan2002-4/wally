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
#include "hal_pmu.h"
#include "hal_pmu_internal_hp.h"
#include "hal_pmu_charger_hp.h"
#include "hal_pmu_auxadc_hp.h"
#include "hal_pmu_hp_platform.h"
#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_sleep_manager_internal.h"
#include "hal_sleep_manager.h"
#endif
#ifdef HAL_RTC_MODULE_ENABLED
#include "hal_rtc_internal.h"
#endif
uint8_t pmu_charger_sleep_handle;                                                                   /*DATA : sleep handle for PMIC*/
/*DATA : charger current in CC value*/
uint16_t pmu_cc_cur[137] = {0x0, 0x10, 0x50, 0x90, 0x130, 0x150, 0x170, 0x190, 0x220, 0x230,
                            0x310, 0x350, 0x390, 0x430, 0x450, 0x470, 0x490, 0x520, 0x530, 0x540,
                            0x550, 0x560, 0x570, 0x580, 0x590, 0x614, 0x620, 0x624, 0x630, 0x634,
                            0x640, 0x644, 0x650, 0x654, 0x660, 0x664, 0x670, 0x674, 0x680, 0x684,
                            0x690, 0x694, 0x714, 0x716, 0x720, 0x722, 0x724, 0x726, 0x730, 0x732,
                            0x734, 0x736, 0x740, 0x742, 0x744, 0x746, 0x750, 0x752, 0x754, 0x756,
                            0x760, 0x762, 0x764, 0x766, 0x770, 0x772, 0x774, 0x776, 0x780, 0x782,
                            0x784, 0x786, 0x790, 0x811, 0x812, 0x813, 0x814, 0x815, 0x816, 0x817,
                            0x820, 0x821, 0x822, 0x823, 0x824, 0x825, 0x826, 0x827, 0x830, 0x831,
                            0x832, 0x833, 0x834, 0x835, 0x836, 0x837, 0x840, 0x841, 0x842, 0x843,
                            0x844, 0x845, 0x846, 0x847, 0x850, 0x851, 0x852, 0x853, 0x854, 0x855,
                            0x856, 0x857, 0x860, 0x861, 0x862, 0x863, 0x864, 0x865, 0x866, 0x867,
                            0x870, 0x871, 0x872, 0x873, 0x874, 0x875, 0x876, 0x877, 0x880, 0x881,
                            0x882, 0x883, 0x884, 0x885, 0x886, 0x887, 0x890
                           };
/*DATA : charger current in iterm value*/
uint16_t pmu_iterm[56] = {0x0, 0x10, 0x50, 0x90, 0x130, 0x150, 0x170, 0x190, 0x220, 0x230,
                          0x240, 0x250, 0x260, 0x270, 0x280, 0x290, 0x320, 0x330, 0x340, 0x350,
                          0x360, 0x370, 0x380, 0x390, 0x414, 0x420, 0x424, 0x430, 0x434, 0x440,
                          0x444, 0x450, 0x454, 0x460, 0x464, 0x470, 0x474, 0x480, 0x484, 0x490,
                          0x514, 0x520, 0x524, 0x530, 0x534, 0x540, 0x544, 0x550, 0x554, 0x560,
                          0x564, 0x570, 0x574, 0x580, 0x584, 0x590
                         };
/*==========[Basic function]==========*/
void pmu_charger_init_hp(uint16_t precc_cur, uint16_t cv_termination)
{
    pmu_trim_ic_init();
    pmu_set_pre_charger_current_hp(precc_cur);
    pmu_set_iterm_current_hp(cv_termination);
    pmu_thermal_parameter_init_hp(); //wait efuse ready
}
void pmu_trim_ic_init(void)
{
    /*check Reserved efuse bit*/
}
bool pmu_get_chr_detect_value_hp(void)
{
    return pmu_get_register_value_hp(PMU_CHR_AO_DBG0, DA_QI_CHR_REF_EN_MASK, DA_QI_CHR_REF_EN_SHIFT);
}
void pmu_charger_check_faston_hp(void)
{
    uint32_t rg_658 = pmu_get_register_value_hp(PMU_LCHR_DIG_DEBUG1, 0xFFFF, 0);
    if (rg_658 & 0x8100) {
        log_hal_msgid_info("[PMU_CHG]FAST_ON : %x charger state : %d", 2, rg_658, pmu_get_charger_state_hp());
        pmu_enable_sysldo(0);
        hal_gpt_delay_ms(300);
        pmu_enable_sysldo(1);
        log_hal_msgid_info("[PMU_CHG]After SYSLDO off/on charger state : %d", 1, pmu_get_charger_state_hp());
    }
}

void pmu_enable_safety_timer_hp(pmu_safety_timer_t tmr, pmu_power_operate_t oper)
{
    switch (tmr) {
        case PMU_PRECC:
            pmu_set_register_value_hp(PMU_LCHR_DIG_CON4, RG_EN_SAFETY_TMR_PRECC_MASK, RG_EN_SAFETY_TMR_PRECC_SHIFT, oper);
            break;
        case PMU_FASTCC:
            pmu_set_register_value_hp(PMU_LCHR_DIG_CON4, RG_EN_SAFETY_TMR_CC_MASK, RG_EN_SAFETY_TMR_CC_SHIFT, oper);
            break;
    }
}
void pmu_set_safety_timer_hp(pmu_safety_timer_t tmr, uint8_t index)
{
    switch (tmr) {
        case PMU_PRECC:
            pmu_set_register_value_hp(PMU_LCHR_DIG_CON5, RG_PRECC_TMR_SEL_MASK, RG_PRECC_TMR_SEL_SHIFT, index);
            log_hal_msgid_info("[PMU_CHG]precc safety timer : %x", 1, pmu_get_register_value_hp(PMU_LCHR_DIG_CON5, RG_PRECC_TMR_SEL_MASK, RG_PRECC_TMR_SEL_SHIFT));
            break;
        case PMU_FASTCC:
            pmu_set_register_value_hp(PMU_LCHR_DIG_CON5, RG_CC_TMR_SEL_MASK, RG_CC_TMR_SEL_SHIFT, index);
            log_hal_msgid_info("[PMU_CHG]fastcc safety timer : %x", 1, pmu_get_register_value_hp(PMU_LCHR_DIG_CON5, RG_CC_TMR_SEL_MASK, RG_CC_TMR_SEL_SHIFT));
            break;
    }
}
/*==========[charger function]==========*/
uint8_t pmu_vsys_shift(uint8_t temp_trim)
{
    if (temp_trim <= 3) {
        temp_trim = temp_trim + 0xC;
    } else if ((temp_trim >= 0x8) && (temp_trim <= 0xA)) {
        temp_trim = temp_trim - 0x3;
    } else if (temp_trim == 0xB) {
        temp_trim = 0;
    } else {
        temp_trim = 0x4;
    }
    return temp_trim;
}
uint8_t pmu_enable_charger_hp(uint8_t isEnableCharging)
{
    uint8_t value = 0;
    if (pmu_get_pmic_version() == 0) {
        pmu_select_cv_voltage_hp(0);//4.05V
        uint32_t temp_trim = pmu_get_register_value_hp(PMU_CORE_CORE_ELR_2, RG_EFUSE_SYSLDO_RSV_TRIM_MASK, RG_EFUSE_SYSLDO_RSV_TRIM_SHIFT);
        temp_trim = pmu_vsys_shift(temp_trim);
        pmu_set_register_value_hp(PMU_CORE_CORE_ELR_2, RG_EFUSE_SYSLDO_RSV_TRIM_MASK, RG_EFUSE_SYSLDO_RSV_TRIM_SHIFT, temp_trim);
    }
    pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON9, RG_LOOP_CHRLDO_SB_DIS_MASK, RG_LOOP_CHRLDO_SB_DIS_SHIFT, 0x80);
    pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON8, RG_SYSDPM_STATUS_SEL_MASK, RG_SYSDPM_STATUS_SEL_SHIFT, 0);
    //pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON8, RG_BUSDPM_DELTA_VTH_MASK, RG_BUSDPM_DELTA_VTH_SHIFT, 0x1);/*Set BUSDPM 200mV*/
#ifndef AIR_PMU_DISABLE_CHARGER
    value = pmu_set_register_value_hp(PMU_LCHR_DIG_CON4, RG_EN_CHR_MASK, RG_EN_CHR_SHIFT, isEnableCharging);
#endif
    return value;
}
/*SYSLDO enable bit*/
bool pmu_enable_sysldo(bool isEnableSysdo)
{
    return pmu_set_register_value_hp(PMU_CHR_AO_SYSLDO, RG_SYSLDO_EN_MASK, RG_SYSLDO_EN_SHIFT, isEnableSysdo);
}
/*The charging state status of linear charger
 * 3'b000: CHR_OFF
 * 3'b001: PRECC
 * 3'b010: FASTCC
 * 3'b011: EXTENSION
 * 3'b100: EOC
 * 3'b101: THR
 * 3'b110: VBAT_OVP
 * 3'b111: PRECC or CC  SAFETY timer time out
 * */
uint32_t pmu_get_charger_state_hp(void)
{
    return pmu_get_register_value_hp(PMU_LCHR_DIG_CON9, RGS_CHR_STATE_SW_MASK, RGS_CHR_STATE_SW_SHIFT);
}
/*
 * 2'b00: 0us
 * 2'b01: 16us (default)
 * 2'b10: 128us
 * 2'b11: 256us*/
void pmu_set_icl_tstep(pmu_icl_tstep_t val)
{
    pmu_set_register_value_hp(PMU_CHR_AO_ICL_0, RG_TSTEP_ICL_MASK, RG_TSTEP_ICL_SHIFT, val);
}

void pmu_set_icl_by_type_hp(uint8_t port)
{
    switch (port) {
        case SDP_CHARGER:
            pmu_set_charger_current_limit_hp(ICL_ITH_443mA);
            break;
        case CDP_CHARGER:
            pmu_set_charger_current_limit_hp(ICL_ITH_600mA);
            break;
        case DCP_CHARGER:
            pmu_set_charger_current_limit_hp(ICL_ITH_800mA);
            break;
        default:
            pmu_set_charger_current_limit_hp(ICL_ITH_443mA);
            break;
    }
    log_hal_msgid_info("[PMU_CHG]Get ICL Current After setting:%d\r\n", 1, pmu_get_register_value_hp(PMU_CHR_AO_ICL_0, RG_ICL_ITH_MASK, RG_ICL_ITH_SHIFT));
}

void pmu_set_charger_current_limit_hp(pmu_icl_level_t icl_value)
{
    /*ICL use 443 mA, set RG_ICL_TRIM_SEL=0. Other ICL current level , set  RG_ICL_TRIM_SEL=1. */
    if (icl_value == ICL_ITH_443mA) {
        pmu_set_register_value_hp(PMU_CORE_CORE_ELR_0, RG_ICL_TRIM_SEL_MASK, RG_ICL_TRIM_SEL_SHIFT, 0x0);
    } else {
        pmu_set_register_value_hp(PMU_CORE_CORE_ELR_0, RG_ICL_TRIM_SEL_MASK, RG_ICL_TRIM_SEL_SHIFT, 0x1);
    }
    pmu_set_register_value_hp(PMU_CHR_AO_ICL_0, RG_ICL_ITH_MASK, RG_ICL_ITH_SHIFT, icl_value);
}

/*
 * recharging voltage select
 * 2'b00:VCV-50mV
 * 2'b01:VCV-100mV(default)
 * 2'b10:VCV-150mV
 * 2'b11:VCV-200mV
 * */
bool pmu_set_rechg_voltage_hp(uint8_t rechargeVoltage)
{
    return pmu_set_register_value_hp(PMU_LCHR_DIG_CON7, RG_VRECHG_MASK, RG_VRECHG_SHIFT, rechargeVoltage);
}

bool pmu_enable_recharger_hp(bool isEnableRecharge)
{
    return pmu_set_register_value_hp(PMU_LCHR_DIG_CON7, RG_EN_RECHG_MASK, RG_EN_RECHG_SHIFT, isEnableRecharge);
}

/*iterm timing setting
 * 2'b00:dsiable iterm
 * 2'b01:15mins
 * 2'b10:30mins
 * 2'b11:60mins
*/
bool pmu_set_extend_charger_time_hp(uint8_t timeMins)
{
    return pmu_set_register_value_hp(PMU_CHR_AO_ITERM, RG_T_ITERM_EXT_MASK, RG_T_ITERM_EXT_SHIFT, timeMins);
}

/*==========[Battery parameter]==========*/
/*select vcv voltage
* 5'b00000: 4.05V
* 5'b00001: 4.075V
* 5'b00010: 4.1V
* 5'b00011: 4.125V
* 5'b00100: 4.15V
* 5'b00101: 4.175V
* 5'b00110: 4.2V (Default)
* 5'b00111: 4.225V
* 5'b01000: 4.25V
* 5'b01001: 4.275V
* 5'b01010: 4.3V
* 5'b01011: 4.325V
* 5'b01100: 4.35V
* 5'b01101: 4.375V
* 5'b01110: 4.4V
* 5'b01111: 4.425V
* 5'b10000: 4.45V
* 5'b10001: 4.475V
* 5'b10010: 4.5V
* 5'b10011: 4.525V
* 5'b10100: 4.55V
* 5'b10101: 4.575V
* 5'b10110~5'b11111: 4.6V*/
bool pmu_select_cv_voltage_hp(uint8_t voltage)
{
    return pmu_set_register_value_hp(PMU_CHR_AO_VCV, RG_VCV_VTH_MASK, RG_VCV_VTH_SHIFT, voltage);
}

/*Select the threshold of CC mode. (VBAT>THR enter CC mode)
 *3'b000: 2.40V
 *3'b001: 2.55V
 *3'b010: 2.70V
 *3'b011: 2.85V
 *3'b100: 3.00V (default)
 *3'b101: 3.15V
 *3'b110: 3.30V
 *3'b111: 3.45V*/
void pmu_select_precc_voltage_hp(uint8_t voltage)
{
    pmu_control_pmic_protect(TST_W_KEY_SW_MODE);
    pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON12, RG_PRECC_DET_MASK, RG_PRECC_DET_SHIFT, voltage);
    pmu_control_pmic_protect(TST_W_KEY_HW_MODE);
    log_hal_msgid_info("[PMU_CHR] : Precc voltage[%x]", 1, pmu_get_register_value_hp(PMU_CORE_CORE_ANA_CON12, RG_PRECC_DET_MASK, RG_PRECC_DET_SHIFT));
}

/*CC safety timer selection, if change time out target, timer will be reset and re-count
 * 2'b00:3h
 * 2'b01:3h (default)
 * 2'b10:4.5h
 * 2'b11:9h
 */
bool pmu_select_cc_safety_timer_hp(uint8_t timeMHrs)
{
    return pmu_set_register_value_hp(PMU_LCHR_DIG_CON5, RG_CC_TMR_SEL_MASK, RG_CC_TMR_SEL_SHIFT, timeMHrs);
}

/*[Charger current/Iterm]*/
void pmu_set_icc_gain(pmu_fastcc_chrcur_t cur)
{
    if (cur < pmu_fastcc_chrcur_50mA) {
        pmu_set_register_value_hp(PMU_CORE_CORE_ELR_7, RG_EFUSE_ICC_GAIN_SEL_MASK, RG_EFUSE_ICC_GAIN_SEL_SHIFT, 0x0);
    } else if ((cur >= pmu_fastcc_chrcur_50mA) && (cur < pmu_fastcc_chrcur_200mA)) {
        pmu_set_register_value_hp(PMU_CORE_CORE_ELR_7, RG_EFUSE_ICC_GAIN_SEL_MASK, RG_EFUSE_ICC_GAIN_SEL_SHIFT, 0x1);
    } else { //>=pmu_fastcc_chrcur_200mA
        pmu_set_register_value_hp(PMU_CORE_CORE_ELR_7, RG_EFUSE_ICC_GAIN_SEL_MASK, RG_EFUSE_ICC_GAIN_SEL_SHIFT, 0x2);
    }
    //log_hal_msgid_info("pmu_set_icc_gain : [%d][%x]",2,cur,pmu_get_register_value_hp(PMU_CORE_CORE_ELR_7, RG_EFUSE_ICC_GAIN_SEL_MASK, RG_EFUSE_ICC_GAIN_SEL_SHIFT));
}
void pmu_set_pre_charger_current_hp(pmu_fastcc_chrcur_t cur)
{
    pmu_set_icc_gain(cur);
    pmu_set_register_value_hp(PMU_LCHR_DIG_CON14, 0xfff, 0, pmu_cc_cur[cur]);
    log_hal_msgid_info("[PMU_CHG]pree-cc charger current : [%d][%x]", 2, cur, pmu_cc_cur[cur]);
}

void pmu_set_charger_current_hp(pmu_fastcc_chrcur_t cur)
{
    pmu_set_icc_gain(cur);
    pmu_set_register_value_hp(PMU_LCHR_DIG_CON15, 0xfff, 0, pmu_cc_cur[cur]);
    log_hal_msgid_info("[PMU_CHG]fast-cc charger current : [%d][%x]", 2, cur, pmu_cc_cur[cur]);
}

void pmu_set_iterm_current_irq_hp(pmu_iterm_chrcur_t cur)
{
    pmu_set_register_value_hp(PMU_LCHR_DIG_CON16, 0x7ff, 0, pmu_iterm[cur]);
    log_hal_msgid_info("[PMU_CHG]iterm current irq : [%d][%x]", 2, cur, pmu_iterm[cur]);
}

void pmu_set_iterm_current_hp(pmu_iterm_chrcur_t cur)
{
    pmu_set_register_value_hp(PMU_LCHR_DIG_CON17, 0x7ff, 0, pmu_iterm[cur]);
    log_hal_msgid_info("[PMU_CHG]iterm current : [%d][%x]", 2, cur, pmu_iterm[cur]);
}

int pmu_get_charger_current_index(void)
{
    int i = 0;
    int cc_index = 0;
    uint32_t temp_value =  pmu_get_register_value_hp(PMU_LCHR_DIG_CON15, 0xfff, 0);
    for (i = 0; i < 137; i++) {
        if (temp_value == pmu_cc_cur[i]) {
            cc_index = i;
            break;
        }
    }
    log_hal_msgid_info("[PMU_CHG]get charger current index: %d temp_v:%x ,arr_v:%x", 3, cc_index, temp_value, pmu_cc_cur[cc_index]);
    return cc_index;
}

/*==========[HW-JEITA]==========*/
/* Enable the HW-JEITA hot cold control */
bool pmu_set_hw_jeita_enable_hp(uint8_t value)
{
    pmu_set_register_value_hp(PMU_AUXADC_AD_JEITA_0, AUXADC_JEITA_IRQ_EN_MASK, AUXADC_JEITA_IRQ_EN_SHIFT, value);
    return pmu_set_register_value_hp(PMU_LCHR_DIG_CON0, RG_EN_HWJEITA_MASK, RG_EN_HWJEITA_SHIFT, value);
}

void pmu_hw_jeita_init_hp(void)
{
    pmu_set_register_value_hp(PMU_CHR_AO_ICC_0, RG_TSTEP_ICC_MASK, RG_TSTEP_ICC_SHIFT, 0x2); //ICC soft start step time.default 16ms
    if (DIGITAL_THERMAL_FUNCTION) { //digital thermal function needs to be enabled
        log_hal_msgid_info("[PMU_JEITA]Enable Digital Thermal Function\r\n", 0);
        /*Enable the digital thermal regulation for ICC*/
        pmu_set_register_value_hp(PMU_LCHR_DIG_CON2, RG_EN_DIG_THR_MASK, RG_EN_DIG_THR_SHIFT, 0x1);
    }
}

pmu_operate_status_t pmu_set_jeita_voltage_hp(uint32_t auxadcVolt, uint8_t JeitaThreshold)
{
    switch (JeitaThreshold) {
        case HW_JEITA_HOT_STAGE:
            return pmu_set_register_value_hp(PMU_AUXADC_AD_JEITA_1, AUXADC_JEITA_VOLT_HOT_MASK, AUXADC_JEITA_VOLT_HOT_SHIFT, auxadcVolt);
            break;

        case HW_JEITA_WARM_STAGE:
            return pmu_set_register_value_hp(PMU_AUXADC_AD_JEITA_2, AUXADC_JEITA_VOLT_WARM_MASK, AUXADC_JEITA_VOLT_WARM_SHIFT, auxadcVolt);
            break;

        case HW_JEITA_COOL_STAGE:
            return pmu_set_register_value_hp(PMU_AUXADC_AD_JEITA_3, AUXADC_JEITA_VOLT_COOL_MASK, AUXADC_JEITA_VOLT_COOL_SHIFT, auxadcVolt);
            break;

        case HW_JEITA_COLD_STAGE:
            return pmu_set_register_value_hp(PMU_AUXADC_AD_JEITA_4, AUXADC_JEITA_VOLT_COLD_MASK, AUXADC_JEITA_VOLT_COLD_SHIFT, auxadcVolt);
            break;
    }
    return PMU_OK;
}

/*WARM/COOL flag will not referenced by LCHR which decided by RG_DISWARMCOOL (default 1).
 * Set RG_DISWARMCOOL=0 when the user needs to use JEITA/WARM COOL*/
void pmu_set_jeita_state_setting_hp(uint8_t state, pmu_jeita_perecnt_level_t ICC_JC, pmu_cv_voltage_t vol)
{
    switch (state) {
        case HW_JEITA_NORMAL_STAGE:
            log_hal_msgid_info("[PMU_JEITA]Normal State will not be change", 0);
            break;
        case HW_JEITA_WARM_STAGE:
            pmu_set_register_value_hp(PMU_LCHR_DIG_CON1, RG_VCV_JW_MASK, RG_VCV_JW_SHIFT, vol);
            hal_gpt_delay_us(50);
            pmu_set_register_value_hp(PMU_CHR_AO_ICC_0, RG_ICC_JW_MASK, RG_ICC_JW_SHIFT, ICC_JC);
            log_hal_msgid_info("[PMU_JEITA]HW JEITA Warm Setting", 0);
            break;
        case HW_JEITA_COOL_STAGE:
            pmu_set_register_value_hp(PMU_LCHR_DIG_CON1, RG_VCV_JC_MASK, RG_VCV_JC_SHIFT, vol);
            hal_gpt_delay_us(50);
            pmu_set_register_value_hp(PMU_CHR_AO_ICC_0, RG_ICC_JC_MASK, RG_ICC_JC_SHIFT, ICC_JC);
            log_hal_msgid_info("[PMU_JEITA]HW JEITA Cool Setting\r\n", 0);
            break;
        case HW_JEITA_HOT_STAGE:
            log_hal_msgid_info("[PMU_JEITA]HW JEITA HOT Setting,Charger off\r\n", 0);
            break;
        case HW_JEITA_COLD_STAGE:
            log_hal_msgid_info("[PMU_JEITA]HW JEITA Cold Setting,Charger off\r\n", 0);
            break;
        default :
            log_hal_msgid_info("[PMU_JEITA]state error\r\n", 0);
            break;
    }
}

/* Get HW JEITA stage
 *
 * HW_JEITA_HOT_STAGE 0xF
 * HW_JEITA_WARM_STAGE 0xE
 * HW_JEITA_NORMAL_STAGE 0xC
 * HW_JEITA_COOL_STAGE 0x8
 * HW_JEITA_COLD_STAGE 0
 *
 * */
uint8_t pmu_get_hw_jeita_status_hp(void)
{
    uint8_t jeita_status = 0;
    jeita_status |= (pmu_get_register_value_hp(PMU_AUXADC_AD_JEITA_1, AUXADC_JEITA_HOT_IRQ_MASK, AUXADC_JEITA_HOT_IRQ_SHIFT) << 0);
    jeita_status |= (pmu_get_register_value_hp(PMU_AUXADC_AD_JEITA_2, AUXADC_JEITA_WARM_IRQ_MASK, AUXADC_JEITA_WARM_IRQ_SHIFT) << 1);
    jeita_status |= (pmu_get_register_value_hp(PMU_AUXADC_AD_JEITA_3, AUXADC_JEITA_COOL_IRQ_MASK, AUXADC_JEITA_COOL_IRQ_SHIFT) << 2);
    jeita_status |= (pmu_get_register_value_hp(PMU_AUXADC_AD_JEITA_4, AUXADC_JEITA_COLD_IRQ_MASK, AUXADC_JEITA_COLD_IRQ_SHIFT) << 3);
    return jeita_status;
}

/*==========[ECO operating]==========*/
void pmu_select_eoc_option_operating_hp(pmu_eoc_option_t opt, pmu_eoc_operating_t oper)
{
    log_hal_msgid_info("[PMU_CHG]pmu_select_eoc_option_operating[%d][%d]", 2, opt, oper);
    switch (oper) {
        case option_setting:
            pmu_eoc_option_setting(opt);
            break;
        case option4_init:
            pmu_charger_eoc4_setting();
            break;
        case option4_exit:
            pmu_charger_eoc4_exit();
            break;
    }
}

void pmu_charger_eoc4_setting(void)
{
    pmu_control_pmic_protect(TST_W_KEY_SW_MODE);
    pmu_set_register_value_hp(PMU_CHR_AO_CON0, RG_CHR_PLUGIN_DB_SW_SEL_MASK, RG_CHR_PLUGIN_DB_SW_SEL_SHIFT, 0x1);
    hal_gpt_delay_ms(400);
    if (pmu_get_faston_flag() == 1) {
        pmu_set_register_value_hp(PMU_TPO_CON4, RG_LOW_IBUS_EN_MASK, RG_LOW_IBUS_EN_SHIFT, 0x1);
        pmu_set_register_value_hp(PMU_TPO_CON4, RG_LOW_IBUS_EN_LATCH_MASK, RG_LOW_IBUS_EN_LATCH_SHIFT, 0x1);
        pmu_set_register_value_hp(PMU_TPO_CON4, RG_LOW_IBUS_EN_LATCH_MASK, RG_LOW_IBUS_EN_LATCH_SHIFT, 0x0);
    } else {
        log_hal_msgid_info("[PMU_CHG]EOC3 EOC setting fail\r\n", 0);
    }
    pmu_set_register_value_hp(PMU_CORE_CORE_ANA_AO_CON0, RG_PPFET_CTRL_LP_MODE_MASK, RG_PPFET_CTRL_LP_MODE_SHIFT, 0x1);
    hal_gpt_delay_ms(2);
    pmu_set_register_value_hp(PMU_TPO_CON6, RG_EOC_RTC_EN_MASK, RG_EOC_RTC_EN_SHIFT, 0x1);
#ifdef HAL_RTC_MODULE_ENABLED
    hal_rtc_enter_rtc_mode();
#endif
}

void pmu_charger_eoc4_exit(void)
{
    if (pmu_get_register_value_hp(PMU_PONSTS, STS_CHROUT_MASK, STS_CHROUT_SHIFT)) {
        pmu_set_register_value_hp(PMU_TPO_CON5, RG_LOW_IBUS_EN_MASK, RG_LOW_IBUS_EN_SHIFT, 0);
        pmu_set_register_value_hp(PMU_TPO_CON5, RG_LOW_IBUS_EN_LATCH_MASK, RG_LOW_IBUS_EN_LATCH_SHIFT, 0x1);
        pmu_set_register_value_hp(PMU_TPO_CON5, RG_LOW_IBUS_EN_LATCH_MASK, RG_LOW_IBUS_EN_LATCH_SHIFT, 0);
        log_hal_msgid_info("[PMU_CHG]EOC RTC MODE alarm", 0);
    }
}
void pmu_eoc_option_setting(pmu_eoc_option_t opt)
{
    if (opt == pmu_eoc_option1) {
    } else if (opt == pmu_eoc_option4) {
    } else {
        log_hal_msgid_info("[PMU_CHG]EOC option setting fail  %d \r\n", 0);
    }

}
void pmu_control_pmic_protect(uint8_t tstWKeymode)
{
    switch (tstWKeymode) {
        case TST_W_KEY_HW_MODE:
            pmu_set_register_value_hp(PMU_TESTKEY, TST_W_KEY_MASK, TST_W_KEY_SHIFT, 0);
            break;
        case TST_W_KEY_SW_MODE:
            pmu_set_register_value_hp(PMU_TESTKEY, TST_W_KEY_MASK, TST_W_KEY_SHIFT, 0x4936);
            break;
        default:
            break;
    }
}
uint32_t pmu_get_faston_flag(void)
{
    return pmu_get_register_value_hp(PMU_LCHR_DIG_DEBUG1, DD_QI_FASTON_FLAG_DB_MASK, DD_QI_FASTON_FLAG_DB_SHIFT);
}
uint32_t pmu_get_vsys_dpm_status(void)
{
    return pmu_get_register_value_hp(PMU_LCHR_DIG_DEBUG1, AD_QI_SYSDPM_MODE_MASK, AD_QI_SYSDPM_MODE_SHIFT);
}
uint32_t pmu_get_vbus_dpm_status(void)
{
    return pmu_get_register_value_hp(PMU_LCHR_DIG_DEBUG1, AD_QI_VBUSDPM_MODE_MASK, AD_QI_VBUSDPM_MODE_SHIFT);
}
void pmu_set_vbus_debounce_time_hp(uint8_t value)
{
    pmu_set_register_value(PMU_CORE_CORE_ANA_CON14, RG_EFUSE_CHGIN_DEB_T_MASK, RG_EFUSE_CHGIN_DEB_T_SHIFT, value);
    pmu_set_register_value(PMU_CORE_CORE_ANA_CON14, RG_CHGIN_DEB_T_LAT_RELOAD_SW_MASK, RG_CHGIN_DEB_T_LAT_RELOAD_SW_SHIFT, 0x0);
    pmu_set_register_value(PMU_CORE_CORE_ANA_CON14, RG_CHGIN_DEB_T_LAT_RELOAD_SW_MASK, RG_CHGIN_DEB_T_LAT_RELOAD_SW_SHIFT, 0x1);
    pmu_set_register_value(PMU_CORE_CORE_ANA_CON14, RG_CHGIN_DEB_T_LAT_RELOAD_SW_MASK, RG_CHGIN_DEB_T_LAT_RELOAD_SW_SHIFT, 0x0);
}
void pmu_enable_powerhold_hp(uint8_t value)
{
    pmu_set_register_value(PMU_CORE_CORE_ANA_CON14, RG_EFUSE_PWRHOLD_CHK_EN_MASK, RG_EFUSE_PWRHOLD_CHK_EN_SHIFT, value);
    pmu_set_register_value(PMU_CORE_CORE_ANA_CON14, RG_PWRHOLD_CHK_EN_LAT_RELOAD_SW_MASK, RG_PWRHOLD_CHK_EN_LAT_RELOAD_SW_SHIFT, 0x0);
    pmu_set_register_value(PMU_CORE_CORE_ANA_CON14, RG_PWRHOLD_CHK_EN_LAT_RELOAD_SW_MASK, RG_PWRHOLD_CHK_EN_LAT_RELOAD_SW_SHIFT, 0x1);
    pmu_set_register_value(PMU_CORE_CORE_ANA_CON14, RG_PWRHOLD_CHK_EN_LAT_RELOAD_SW_MASK, RG_PWRHOLD_CHK_EN_LAT_RELOAD_SW_SHIFT, 0x0);
}


void pmu_ovp_debug(void)
{
    uint32_t rg_658 = pmu_get_register_value(0x658, 0xffff, 0);
    log_hal_msgid_info("[PMU_CHG][0x63e:%x][0x748:%x][0x74a:%x][0x74c:%x][0x10e:%x]", 5,
                       rg_658,
                       pmu_get_register_value(0x63e, 0xffff, 0),
                       pmu_get_register_value(0x656, 0xffff, 0),
                       pmu_get_register_value(0x65a, 0xffff, 0),
                       pmu_get_register_value(0x10e, 0xffff, 0));
    if (rg_658 & 0x1) {
        log_hal_msgid_info("[PMU_CHG]OVP : VBUSDPM", 0);
    } else if (rg_658 & 0x2) {
        log_hal_msgid_info("[PMU_CHG]OVP : VSYSDPM", 0);
    } else if (rg_658 & 0x800) {
        log_hal_msgid_info("[PMU_CHG]OVP : BATTERY OVER VOLTAGE ", 0);
    } else {
        log_hal_msgid_info("[PMU_CHG]OVP : CHECK FASTON ", 0);
    }
}
#endif /* HAL_PMU_MODULE_ENABLED */
