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
#include "hal_pmu.h"

#ifdef AIR_BTA_IC_PREMIUM_G3
#include "hal_pmu_ddie.h"
#include "hal_pmu_ddie_platform.h"
#endif

#ifdef AIR_BTA_PMIC_HP
#include "hal_pmu_internal_hp.h"
#include "hal_pmu_charger_hp.h"
#include "hal_pmu_auxadc_hp.h"
#include "hal_pmu_hp_platform.h"
#endif

#ifdef AIR_BTA_PMIC_LP
#include "hal_pmu_internal_lp.h"
#include "hal_pmu_charger_lp.h"
#include "hal_pmu_auxadc_lp.h"
#include "hal_pmu_cal_lp.h"
#include "hal_pmu_lp_platform.h"
#endif
#ifdef HAL_PMU_MODULE_ENABLED

#include "hal_pmu.h"

/*==========[Basic function]==========*/
void pmu_init(void)
{
    log_hal_msgid_info("[PMU]PMU init", 0);
#ifdef AIR_BTA_IC_PREMIUM_G3
    pmu_init_ddie();
#endif

#ifdef AIR_BTA_PMIC_HP
    pmu_init_hp();
#elif defined AIR_BTA_PMIC_LP
    pmu_cal_init();
    pmu_set_init();
    pmu_init_lp();
#ifndef AIR_PMU_DISABLE_CHARGER
    pmu_chg_init();
    pmu_bat_init();
#endif /* AIR_PMU_DISABLE_CHARGER */
    pmu_rst_pat_init();
    pmu_ntc_init();
#else
#endif
}
void pmu_latch_power_key_for_bootloader(void)
{
#ifndef AIR_BTA_IC_PREMIUM_G3_TYPE_D
#ifdef AIR_BTA_PMIC_HP
    pmu_latch_power_key_for_bootloader_hp();
#elif defined AIR_BTA_PMIC_LP
    pmu_latch_power_key_for_bootloader_lp();
#else
#endif
#endif
}

void pmu_config(void)
{
    //log_hal_msgid_info("PMU config", 0);
#ifdef AIR_BTA_PMIC_HP
    pmu_config_hp();
#elif defined AIR_BTA_PMIC_LP
    pmu_config_lp();
#else
#endif
}

uint8_t pmu_get_pmic_version(void)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_get_pmic_version_hp();
#elif defined AIR_BTA_PMIC_LP
    return pmu_get_pmic_version_lp();
#else
    return 0;
#endif
}

uint8_t pmu_get_power_on_reason(void)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_get_power_on_reason_hp();
#elif defined AIR_BTA_PMIC_LP
    return pmu_get_power_on_reason_lp();
#else
    return pmu_get_power_on_reason_ddie();
#endif
}

uint8_t pmu_get_power_off_reason(void)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_get_power_off_reason_hp();
#elif defined AIR_BTA_PMIC_LP
    return pmu_get_power_off_reason_lp();
#else
    return pmu_get_power_off_reason_ddie();
#endif
}
void pmu_power_off_sequence(pmu_power_stage_t stage)
{
    pmu_set_power_state_ddie(stage);
#ifdef AIR_BTA_PMIC_HP
    pmu_power_off_sequence_hp(stage);
#elif defined AIR_BTA_PMIC_LP
    pmu_power_off_sequence_lp(stage);
#else
#endif
}
pmu_operate_status_t pmu_set_register_value(uint32_t address, uint32_t mask, uint32_t shift, uint32_t value)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_set_register_value_hp(address, mask, shift, value);
#elif defined AIR_BTA_PMIC_LP
    return pmu_set_register_value_lp(address, mask, shift, value);
#else
    return PMU_INVALID;
#endif

}
uint32_t pmu_get_register_value(uint32_t address, uint32_t mask, uint32_t shift)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_get_register_value_hp(address, mask, shift);
#elif defined AIR_BTA_PMIC_LP
    return pmu_get_register_value_lp(address, mask, shift);
#else
    return PMU_INVALID;
#endif
}

pmu_power_vcore_voltage_t pmu_lock_vcore(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol, pmu_lock_parameter_t lock)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_lock_vcore_hp(mode, vol, lock);
#elif defined AIR_BTA_PMIC_LP
    return pmu_lock_vcore_lp(mode, vol, lock);
#else
    return PMU_INVALID;
#endif
}

uint32_t pmu_get_charger_state(void)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_get_charger_state_hp();
#elif defined AIR_BTA_PMIC_LP
    return pmu_get_charger_state_lp();
#else
    return PMU_INVALID;
#endif
}

uint8_t pmu_get_usb_input_status(void)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_get_usb_input_status_hp();
#elif defined AIR_BTA_PMIC_LP
    return pmu_get_chr_detect_value_lp();
#else
    return pmu_select_vbus_vosel_ddie();
#endif
}

void pmu_set_rstpat(pmu_power_operate_t oper, pmu_rstpat_src_t src)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_set_rstpat_hp(oper, src);
#elif defined AIR_BTA_PMIC_LP
    pmu_set_rstpat_lp(oper, src);
#else
#endif
}

bool pmu_get_chr_detect_value(void)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_get_usb_input_status_hp();
#elif defined AIR_BTA_PMIC_LP
    return pmu_get_chr_detect_value_lp();
#else
    return pmu_select_vbus_vosel_ddie();
#endif
}

void pmu_select_wdt_mode(pmu_wdtrstb_act_t sel)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_select_wdt_mode_hp(sel);
#elif defined AIR_BTA_PMIC_LP
#else
    //log_hal_msgid_info("Error pmu_select_wdt_mode", 0);
#endif
}
/*==========[BUCK/LDO Power]==========*/
void pmu_enable_power(pmu_power_domain_t pmu_pdm, pmu_power_operate_t operate)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_enable_power_hp(pmu_pdm, operate);
#elif defined AIR_BTA_PMIC_LP
    pmu_enable_power_lp(pmu_pdm, operate);
#else
#endif
}

void pmu_select_vsram_vosel(pmu_power_stage_t mode, pmu_vsram_voltage_t val)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_set_vsram_voltage_hp(val);
#elif defined AIR_BTA_PMIC_LP
    pmu_set_vsram_voltage_lp(val);
#else
    log_hal_msgid_info("Error pmu_select_vsram_vosel", 0);
#endif
}

uint32_t pmu_get_vsram_vosel(pmu_power_stage_t mode)
{
    uint32_t temp_val = 0;
#ifdef AIR_BTA_PMIC_HP
    temp_val = pmu_get_vsram_voltage_hp();
#elif defined AIR_BTA_PMIC_LP
    temp_val = pmu_get_vsram_voltage_lp();
#else
    log_hal_msgid_info("Error pmu_get_vsram_vosel", 0);
#endif
    return temp_val;
}

void pmu_vrf_keep_nm(void)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_vrf_keep_nm_hp();
#elif defined AIR_BTA_PMIC_LP
    pmu_vrf_keep_nm_lp();
#else
    log_hal_msgid_info("Error pmu_keep_vrf_nm", 0);
#endif
}

/*[Audio]*/
void pmu_set_audio_mode(pmu_audio_mode_t mode, pmu_power_operate_t operate)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_set_audio_mode_hp(mode, operate);
#elif defined AIR_BTA_PMIC_LP
    pmu_set_audio_mode_lp(mode, operate);
#else
    log_hal_msgid_info("Error pmu_select_vsram_vosel", 0);
#endif
}

void pmu_enable_micbias(pmu_micbias_ldo_t ldo, pmu_micbias_index_t index, pmu_micbias_mode_t mode, pmu_power_operate_t operate)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_enable_micbias_hp(ldo, index, mode, operate);
#elif defined AIR_BTA_PMIC_LP
    pmu_enable_micbias_lp(ldo, index, mode, operate);
#else
    log_hal_msgid_info("Error pmu_select_vsram_vosel", 0);
#endif

}

void pmu_set_micbias_ldo_mode (pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_set_micbias_ldo_mode_hp(ldo, mode);
#elif defined AIR_BTA_PMIC_LP
    pmu_set_micbias_ldo_mode_lp(ldo, mode);
#else
    log_hal_msgid_info("Error pmu_set_micbias_ldo_mode", 0);
#endif
}

/*[Captouch]*/
void pmu_enable_captouch(pmu_power_operate_t oper)
{
    log_hal_msgid_info("Captouch Power :%d ", 1, oper);
    pmu_enable_captouch_power_ddie(oper);
}
void pmu_set_cap_wo_vbus(pmu_power_operate_t oper)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_set_cap_wo_vbus_hp(oper);
#elif defined AIR_BTA_PMIC_LP
    pmu_set_cap_wo_vbus_lp(oper);
#else
    log_hal_msgid_info("Error pmu_select_vsram_vosel", 0);
#endif
}
/*==========[Module Function]==========*/
pmu_operate_status_t pmu_pwrkey_normal_key_init(pmu_pwrkey_config_t *config)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_pwrkey_normal_key_init_hp(config);
#elif defined AIR_BTA_PMIC_LP
    return pmu_pwrkey_normal_key_init_lp(config);
#else
    return PMU_STATUS_INVALID_PARAMETER;
#endif
}

bool pmu_get_pwrkey_state(void)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_get_pwrkey_state_hp();
#elif defined AIR_BTA_PMIC_LP
    return pmu_get_pwrkey_state_lp();
#else
    return PMU_STATUS_INVALID_PARAMETER;
#endif
}

void pmu_enable_lpsd(pmu_lpsd_time_t tmr, pmu_power_operate_t oper)
{
#if defined(AIR_DCHS_MODE_1BATT_ENABLE) && defined(AIR_DCHS_MODE_SLAVE_ENABLE)
    oper = PMU_OFF;
    log_hal_msgid_info("[PMU_BASIC] LPSD FORCE OFF BY OPTION", 0);
#endif
    /*Power key set duration time and enable and disable*/
#ifdef AIR_BTA_PMIC_HP
    pmu_enable_pk_lpsd_hp(tmr, oper);
#elif defined AIR_BTA_PMIC_LP
    pmu_enable_pk_lpsd_lp(tmr, oper);
#else
#endif

#ifdef HAL_CAPTOUCH_MODULE_ENABLED
    /*Enable and disable lpsd of cap touch*/
#ifdef AIR_BTA_PMIC_HP
    pmu_enable_cap_lpsd_hp(oper);
    /*Cap touch ,set duration time and enable and disable*/
    pmu_set_cap_duration_time_hp(tmr);
#elif defined AIR_BTA_PMIC_LP
    pmu_enable_cap_lpsd_lp(oper);
    /*Cap touch ,set duration time and enable and disable*/
    pmu_set_cap_duration_time_lp(tmr);
#else
#endif
#endif
}

void pmu_set_vpa_voltage(pmu_power_vpa_voltage_t oper)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_set_vpa_voltage_hp(oper);
#elif defined AIR_BTA_PMIC_LP
#else
#endif
}

void pmu_set_pre_charger_current(pmu_fastcc_chrcur_t cur)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_set_pre_charger_current_hp(cur);
#elif defined AIR_BTA_PMIC_LP
#else
#endif
}
/*==========[Charger]==========*/
uint8_t pmu_enable_charger(uint8_t isEnableCharging)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_enable_charger_hp(isEnableCharging);
#elif defined AIR_BTA_PMIC_LP
    return pmu_enable_charger_lp(isEnableCharging);
#else
    return PMU_STATUS_INVALID_PARAMETER;
#endif

}

uint8_t pmu_get_power_status(pmu_power_domain_t pmu_pdm)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_get_power_status_hp(pmu_pdm);
#elif defined AIR_BTA_PMIC_LP
    return PMU_STATUS_INVALID_PARAMETER;
#else
    return PMU_STATUS_INVALID_PARAMETER;
#endif
}

pmu_operate_status_t pmu_set_vaud18_vout(pmu_vaud18_voltage_t lv, pmu_vaud18_voltage_t mv, pmu_vaud18_voltage_t hv)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_set_vaud18_vout_hp(lv, mv, hv);
#elif defined AIR_BTA_PMIC_LP
    return pmu_set_vaud18_vout_lp(lv, mv, hv);
#else
    return PMU_STATUS_INVALID_PARAMETER;
#endif
}

pmu_operate_status_t pmu_set_micbias_vout(pmu_micbias_index_t index, pmu_3vvref_voltage_t vol)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_set_micbias_vout_hp(index, vol);
#elif defined AIR_BTA_PMIC_LP
    return pmu_set_micbias_vout_lp(index, vol);
#else
    return PMU_STATUS_INVALID_PARAMETER;
#endif
}
#ifndef AIR_BTA_IC_PREMIUM_G3_TYPE_D
uint32_t pmu_auxadc_get_channel_value(pmu_adc_channel_t Channel)
{
#ifdef AIR_BTA_PMIC_HP
    return pmu_auxadc_get_channel_value_hp(Channel);
#elif defined AIR_BTA_PMIC_LP
    return pmu_auxadc_get_channel_value_lp(Channel);
#else
    return PMU_STATUS_INVALID_PARAMETER;
#endif
}
#endif

void pmu_set_dummy_load(pmu_power_domain_t domain, uint32_t loading_value)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_set_dummy_load_hp(domain, loading_value);
#elif defined AIR_BTA_PMIC_LP
    pmu_set_dummy_load_lp(domain, loading_value);
#else
    //log_hal_msgid_info("pmu_set_dummy_load not support", 0);
#endif
}
/*-------------------------------------------[D-die PMU]----------------------------------------------------*/
pmu_operate_status_t pmu_set_register_value_ddie(uint32_t address, uint32_t mask, uint32_t shift, uint32_t value)
{
    return pmu_set_register_ddie(address, mask, shift, value);
}
uint32_t pmu_get_register_value_ddie(uint32_t address, uint32_t mask, uint32_t shift)
{
    return pmu_get_register_ddie(address, mask, shift);
}

uint8_t pmu_enable_usb_power(pmu_power_operate_t oper)
{
    return pmu_enable_usb_power_ddie(oper);
}


/*-------------------------------------------[A-die H PMU]----------------------------------------------------*/
/*Charger*/
#ifdef AIR_BTA_PMIC_HP
void pmu_charger_init(uint16_t precc_cur, uint16_t cv_termination)
{
    pmu_charger_init_hp(precc_cur, cv_termination);
}
void pmu_set_icl_by_type(uint8_t port)
{
    pmu_set_icl_by_type_hp(port);
}

bool pmu_enable_recharger(bool isEnableRecharge)
{
    return pmu_enable_recharger_hp(isEnableRecharge);
}

void pmu_select_eoc_option_operating(pmu_eoc_option_t opt, pmu_eoc_operating_t oper)
{
    pmu_select_eoc_option_operating_hp(opt, oper);
}
void pmu_set_charger_current_limit(pmu_icl_level_t icl_value)
{
    pmu_set_charger_current_limit_hp(icl_value);
}
void pmu_set_iterm_current_irq(pmu_iterm_chrcur_t cur)
{
    pmu_set_iterm_current_irq_hp(cur);
}

bool pmu_set_rechg_voltage(uint8_t rechargeVoltage)
{
    return pmu_set_rechg_voltage_hp(rechargeVoltage);
}

int32_t pmu_auxadc_get_pmic_temperature(void)
{
    return pmu_get_pmic_temperature_hp();
}

bool pmu_set_extend_charger_time(uint8_t timeMins)
{
    return pmu_set_extend_charger_time_hp(timeMins);
}

void pmu_charger_check_faston(void)
{
    pmu_charger_check_faston_hp();
}
void pmu_set_charger_current(pmu_fastcc_chrcur_t cur)
{
    pmu_set_charger_current_hp(cur);
}
bool pmu_select_cv_voltage(uint8_t voltage)
{
    return pmu_select_cv_voltage_hp(voltage);
}
/*[JEITA]*/
uint8_t pmu_get_hw_jeita_status(void)
{
    return pmu_get_hw_jeita_status_hp();
}

void pmu_lock_va18(int oper)
{
    pmu_lock_va18_hp(oper);
}

void pmu_hw_jeita_init(void)
{
    pmu_hw_jeita_init_hp();
}

void pmu_set_jeita_state_setting(uint8_t state, pmu_jeita_perecnt_level_t ICC_JC, pmu_cv_voltage_t vol)
{
    pmu_set_jeita_state_setting_hp(state, ICC_JC, vol);
}

pmu_operate_status_t pmu_set_jeita_voltage(uint32_t auxadcVolt, uint8_t JeitaThreshold)
{
    return pmu_set_jeita_voltage_hp(auxadcVolt, JeitaThreshold);
}

bool pmu_set_hw_jeita_enable(uint8_t value)
{
    return pmu_set_hw_jeita_enable_hp(value);
}
#endif /* AIR_BTA_PMIC_HP */

/* Debug Fuctions */
void pmu_dump_otp(void)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_dump_otp_hp();
#elif defined AIR_BTA_PMIC_LP
    pmu_dump_otp_lp();
#endif
}

void pmu_dump_nvkey(void)
{
#ifdef AIR_NVDM_ENABLE
#ifdef AIR_BTA_PMIC_HP
    pmu_dump_nvkey_hp();
#elif defined AIR_BTA_PMIC_LP
    pmu_dump_nvkey_lp();
#endif
#else  /* AIR_NVDM_ENABLE */
    log_hal_msgid_info("pmu_dump_nvkey, AIR_NVDM_ENABLE option is disable, no nvkey", 0);
#endif
}

void pmu_dump_rg(void)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_dump_rg_hp();
#elif defined AIR_BTA_PMIC_LP
    pmu_dump_rg_lp();
#endif
    pmu_dump_rg_ddie();
}

void pmu_check_rg_timer(uint16_t start, uint16_t end, uint16_t timer)
{
#ifdef AIR_BTA_PMIC_HP
    pmu_check_rg_timer_hp(start, end, timer);
#elif defined AIR_BTA_PMIC_LP
    pmu_check_rg_timer_lp(start, end, timer);
#endif
}

void hal_pmu_buck_thd_test(void)
{
#ifdef AIR_BTA_PMIC_LP
    hal_pmu_buck_thd_test_lp();
#endif
}

void hal_pmu_buck_thd_test_wo_pg(void)
{
#ifdef AIR_BTA_PMIC_LP
    hal_pmu_buck_thd_test_wo_pg_lp();
#endif
}
/***************************** 1wire API *******************************/
uint32_t pmu_1wire_get_vbus_uart_volt(void)
{
    uint32_t volt = 0;

#if defined(AIR_BTA_PMIC_HP)
    volt = pmu_auxadc_get_channel_value(PMU_AUX_VBUS_UART);
#elif defined(AIR_BTA_PMIC_LP)
    volt = pmu_auxadc_get_channel_value(PMU_AUX_VCHG);
#elif defined(AIR_BTA_PMIC_G2_HP)
    volt = pmu_auxadc_get_channel_value(PMU_AUX_VBUS);
#elif defined(AIR_BTA_PMIC_G2_LP)
    volt = pmu_bat_adc_to_volt(pmu_auxadc_get_channel_value(PMU_AUX_VCHG));
#else
    log_hal_msgid_error("[PMU_1WIRE]get_vbus_uart_volt fail", 0);
#endif

    return volt;
}

#if defined(AIR_BTA_PMIC_HP)
static void pmu_1wire_uart_trx_pulldown(uint8_t op)
{
    pmu_set_register_value_hp(0x12E, 1, 7, op);
    //log_hal_msgid_info("[PMU_1WIRE]uart_trx_pulldown[%d]", 1, op);
}
#endif

#ifndef AIR_BTA_IC_PREMIUM_G3_TYPE_D
static void pmu_set_rstpat_to_idle(void){
#if defined(AIR_BTA_PMIC_HP)
    if(pmu_get_register_value_hp(PMU_VBUS_UART_STS1,0x1,1)==1){
        pmu_set_register_value_hp(PMU_VBUS_UART_STS1,RG_RSTPAT_SW_RSTB_MASK,RG_RSTPAT_SW_RSTB_SHIFT,0);
        pmu_set_register_value_hp(PMU_VBUS_UART_STS1,RG_RSTPAT_SW_RSTB_MASK,RG_RSTPAT_SW_RSTB_SHIFT,1);
    }
#elif defined(AIR_BTA_PMIC_LP)
    if (pmu_get_register_value(0x0090,0xffff,0x0) == 0x6){
        pmu_set_register_value(0x08E,0x1,8,0x1);
        hal_gpt_delay_us(100);
        pmu_set_register_value(0x08E,0x1,8,0x0);
        log_hal_msgid_info("[PMU_1WIRE] reset RSTPAT", 0);
    } else {
        log_hal_msgid_info("[PMU_1WIRE] reset RSTPAT not unknown", 0);
    }
#endif
}
#endif

static void pmu_1wire_init_to_out_of_case(pmu_1wire_bat_t bat)
{
#if defined(AIR_BTA_PMIC_HP)
    pmu_set_rstpat_to_idle();
    pmu_1wire_uart_trx_pulldown(PMU_ON);

#elif defined(AIR_BTA_PMIC_LP) || defined(AIR_BTA_PMIC_G2_LP)
    if (bat) {
        pmu_eoc_ctrl(PMU_ON);
        pmu_uart_psw(PMU_OFF);
    } else {
        pmu_eoc_ctrl(PMU_OFF);
        pmu_uart_psw(PMU_ON);
        pmu_uart_psw_cl(PMU_ON);
    }
    pmu_vio18_pull_up(PMU_OFF);
    pmu_set_rstpat_to_idle();
#else
    log_hal_msgid_error("[PMU_1WIRE]init_to_out_of_case fail, pmic error", 0);
#endif
}

static void pmu_1wire_init_to_chg_mode(void)
{
#if defined(AIR_BTA_PMIC_HP)
    pmu_set_rstpat_to_idle();
    pmu_1wire_uart_trx_pulldown(PMU_ON);

#elif defined(AIR_BTA_PMIC_LP) || defined(AIR_BTA_PMIC_G2_LP)
    pmu_vio18_pull_up(PMU_ON);
    pmu_set_rstpat_to_idle();

#else
    log_hal_msgid_error("[PMU_1WIRE]init_to_chg_mode fail, pmic error", 0);
#endif
}

static void pmu_1wire_intr_to_chg_mode(pmu_1wire_bat_t bat)
{
#if defined(AIR_BTA_PMIC_HP)
    pmu_set_rstpat_to_idle();
    pmu_1wire_uart_trx_pulldown(PMU_ON);

#elif defined(AIR_BTA_PMIC_LP) || defined(AIR_BTA_PMIC_G2_LP)
    if (bat) {
        pmu_uart_psw_sequence();
    } else {
        hal_gpt_delay_us(100);
        pmu_uart_psw_cl(PMU_OFF);
    }
    pmu_enable_charger_lp(PMU_OFF);
    pmu_set_register_value(RSTPAT_EN_ADDR,RSTPAT_EN_MASK,RSTPAT_EN_SHIFT,0x0);
    pmu_eoc_ctrl(PMU_OFF);
    hal_gpt_delay_ms(10);
    pmu_set_register_value(RSTPAT_EN_ADDR,RSTPAT_EN_MASK,RSTPAT_EN_SHIFT,0x1);
    pmu_enable_charger_lp(PMU_ON);
    pmu_vio18_pull_up(PMU_ON);
    pmu_set_rstpat_to_idle();

#else
    log_hal_msgid_error("[PMU_1WIRE]intr_to_chg_mode fail, pmic error", 0);
#endif
}

static void pmu_1wire_com_mode_to_out_of_case(void)
{
#if defined(AIR_BTA_PMIC_HP)
    pmu_set_rstpat_to_idle();
    pmu_1wire_uart_trx_pulldown(PMU_ON);

#elif defined(AIR_BTA_PMIC_LP) || defined(AIR_BTA_PMIC_G2_LP)
    pmu_vio18_pull_up(PMU_OFF);
    pmu_set_rstpat_to_idle();

#else
    log_hal_msgid_error("[PMU_1WIRE]com_mode_to_out_of_case fail, pmic error", 0);
#endif
}

static void pmu_1wire_chg_mode_to_com_mode(pmu_1wire_bat_t bat)//init_to_log_mode
{
#if defined(AIR_BTA_PMIC_HP)
    pmu_1wire_uart_trx_pulldown(PMU_OFF);

#elif defined(AIR_BTA_PMIC_LP) || defined(AIR_BTA_PMIC_G2_LP)
    if (bat) {
        pmu_eoc_ctrl(PMU_ON);
        pmu_uart_psw(PMU_OFF);
    } else {
        pmu_eoc_ctrl(PMU_OFF);
        pmu_uart_psw(PMU_ON);
        pmu_uart_psw_cl(PMU_ON);
    }
    pmu_vio18_pull_up(PMU_ON);

#else
    log_hal_msgid_error("[PMU_1WIRE]chg_mode_to_com_mode fail, pmic error", 0);
#endif
}

static void pmu_1wire_pwr_saving_mode(void)
{
#if defined(AIR_BTA_PMIC_HP)
    pmu_1wire_uart_trx_pulldown(PMU_ON);

#elif defined(AIR_BTA_PMIC_LP) || defined(AIR_BTA_PMIC_G2_LP)
    pmu_vio18_pull_up(PMU_OFF);

#else
    log_hal_msgid_error("[PMU_1WIRE]pwr_saving_mode fail, pmic error", 0);
#endif
}

static void pmu_1wire_com_to_com_end(void)
{
#if defined(AIR_BTA_PMIC_HP)
    pmu_set_rstpat_to_idle();
    pmu_1wire_uart_trx_pulldown(PMU_ON);
#elif defined(AIR_BTA_PMIC_LP) || defined(AIR_BTA_PMIC_G2_LP)
#else
    log_hal_msgid_error("[PMU_1WIRE]pmu_1wire_com_to_com_end error", 0);
#endif
}

void pmu_1wire_cfg(pmu_1wire_operate_t state)
{
    switch (state) {
        case PMU_1WIRE_INIT_TO_OUT_OF_CASE_LOW_BAT:
            pmu_1wire_init_to_out_of_case(PMU_1WIRE_LOW_BAT);
            break;
        case PMU_1WIRE_INIT_TO_OUT_OF_CASE_NORM_BAT:
            pmu_1wire_init_to_out_of_case(PMU_1WIRE_NORM_BAT);
            break;
        case PMU_1WIRE_INIT_TO_CHG_IN:
            pmu_1wire_init_to_chg_mode();
            break;
        case PMU_1WIRE_OUT_OF_CASE_TO_CHG_IN_NORM_BAT:
        case PMU_1WIRE_COM_TO_CHG_IN_NORM_BAT:
            pmu_1wire_intr_to_chg_mode(PMU_1WIRE_NORM_BAT);
            break;
        case PMU_1WIRE_OUT_OF_CASE_TO_CHG_IN_LOW_BAT:
        case PMU_1WIRE_COM_TO_CHG_IN_LOW_BAT:
            pmu_1wire_intr_to_chg_mode(PMU_1WIRE_LOW_BAT);
            break;
        case PMU_1WIRE_COM_TO_OUT_OF_CASE:
            pmu_1wire_com_mode_to_out_of_case();
            break;
        case PMU_1WIRE_CHG_IN_TO_COM_NORM_BAT:
        case PMU_1WIRE_ENTER_TO_LOG_NORM_BAT:
            pmu_1wire_chg_mode_to_com_mode(PMU_1WIRE_NORM_BAT);
            break;
        case PMU_1WIRE_CHG_IN_TO_COM_LOW_BAT:
        case PMU_1WIRE_ENTER_TO_LOG_LOW_BAT:
            pmu_1wire_chg_mode_to_com_mode(PMU_1WIRE_LOW_BAT);
            break;
        case PMU_1WIRE_COM_TO_PWR_SAVE:
            pmu_1wire_pwr_saving_mode();
            break;
        case PMU_1WIRE_COM_TO_COM_END:
            pmu_1wire_com_to_com_end();
            break;
        default:
            break;
    }
}

#endif /* HAL_PMU_MODULE_ENABLED */
