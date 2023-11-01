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

#ifndef __HAL_PMU_INTERNAL_H__
#define __HAL_PMU_INTERNAL_H__
#ifdef HAL_PMU_MODULE_ENABLED
/*==========[BUCK/LDO]==========*/
pmu_power_vcore_voltage_t pmu_lock_vcore_hp(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol, pmu_lock_parameter_t lock);
pmu_power_vcore_voltage_t pmu_get_vcore_setting_index(uint16_t vcore);
pmu_power_vcore_voltage_t pmu_get_vcore_voltage_hp(void);
void pmu_ddie_sram_setting(pmu_vsram_dvs_state_t ste, pmu_power_vcore_voltage_t vol);
void pmu_enable_power_hp(pmu_power_domain_t pmu_pdm, pmu_power_operate_t operate);
void pmu_switch_power(pmu_power_domain_t pmu_pdm, pmu_buckldo_stage_t mode, pmu_power_operate_t operate);
uint8_t pmu_get_power_status_hp(pmu_power_domain_t pmu_pdm);
void pmu_vcore_ipeak(int vol_index);
pmu_operate_status_t pmu_select_vcore_voltage_hp(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol);
void pmu_set_vaud18_voltage_hp(pmu_vaud18_vsel_t vsel, uint16_t volt);
void pmu_set_vpa_voltage_hp(pmu_power_vpa_voltage_t oper);
void pmu_lock_va18_hp(int oper);
void pmu_set_vsram_voltage_hp(pmu_vsram_voltage_t val);
uint32_t pmu_get_vsram_voltage_hp(void);
/*==========[Audio/MICBIAS]==========*/
pmu_operate_status_t pmu_set_micbias_vout_hp(pmu_micbias_index_t index, pmu_3vvref_voltage_t vol);
pmu_operate_status_t pmu_set_vaud18_vout_hp(pmu_vaud18_voltage_t lv, pmu_vaud18_voltage_t mv, pmu_vaud18_voltage_t hv);
void pmu_enable_micbias_inrush(pmu_micbias_index_t index, uint8_t operate);
void pmu_set_micbias_ldo_vout(pmu_micbias_index_t index);
void pmu_set_micbias_pulllow(pmu_micbias_index_t index, pmu_power_operate_t oper);
void pmu_enable_micbias_ldo(pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode);
void pmu_enable_micbias_mode(pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode);
void pmu_enable_micbias_hp(pmu_micbias_ldo_t ldo, pmu_micbias_index_t index, pmu_micbias_mode_t mode, pmu_power_operate_t operate);
void pmu_set_audio_mode_hp(pmu_audio_mode_t mode, pmu_power_operate_t operate);
void pmu_set_audio_mode_init(pmu_audio_mode_t mode);
void pmu_set_micbias_ref_voltage(pmu_3vvref_voltage_t val);
void pmu_set_micbias_ldo_mode_hp(pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode);
/*==========[Interrupt]==========*/
pmu_status_t pmu_register_callback(pmu_interrupt_index_t pmu_int_ch, pmu_callback_t callback, void *user_data);
pmu_status_t pmu_deregister_callback(pmu_interrupt_index_t pmu_int_ch);
void pmu_get_all_int_status(void);
int pmu_get_status_interrupt(pmu_interrupt_index_t int_channel);
pmu_status_t pmu_mask_interrupt(pmu_interrupt_index_t int_channel, int isEnable);
void pmu_irq_count(int int_channel);
void pmu_scan_interrupt_status(void);
void pmu_irq_init(void);
pmu_status_t pmu_enable_interrupt(pmu_interrupt_index_t int_channel, int isEnable);
pmu_status_t pmu_clear_interrupt(pmu_interrupt_index_t int_channel) ;
pmu_operate_status_t pmu_pk_filter(uint8_t pk_sta);
void pmu_eint_handler(void *parameter);
void pmu_eint_init(void);
/*==========[Power key & Cap touch]==========*/
pmu_operate_status_t pmu_pwrkey_normal_key_init_hp(pmu_pwrkey_config_t *config);
void pmu_enable_pk_lpsd_hp(pmu_lpsd_time_t tmr, pmu_power_operate_t oper);
pmu_operate_status_t pmu_lpsd_function_sel(pmu_lpsd_scenario_t oper);
void pmu_enable_cap_lpsd_hp(pmu_power_operate_t oper);
void pmu_set_cap_duration_time_hp(pmu_lpsd_time_t tmr);
void pmu_set_cap_wo_vbus_hp(pmu_power_operate_t oper);
bool pmu_get_pwrkey_state_hp(void);
/*==========[Efuse]==========*/
void pmu_efuse_enable_reading(void);
void pmu_efuse_disable_reading(void);
/*==========[Basic]==========*/
void pmu_power_off_sequence_hp(pmu_power_stage_t stage);
void pmu_latch_power_key_for_bootloader_hp(void);
uint8_t pmu_get_power_on_reason_hp(void);
uint8_t pmu_get_power_off_reason_hp(void);
bool pmu_get_efuse_status(void);
void pmu_press_pk_time(void);
uint8_t pmu_get_usb_input_status_hp(void) ;
void hal_pmu_sleep_suspend(void);
void hal_pmu_sleep_resume(void);
void pmu_set_rstpat_hp(pmu_power_operate_t oper, pmu_rstpat_src_t src);
void pmu_safety_confirmation(void);
uint8_t pmu_get_pmic_version_hp(void);
void pmu_init_hp(void);
void pmu_config_hp(void);
void pmu_efuse_init_hp(void);
uint32_t pmu_d2d_i2c_read(unsigned char *ptr_send, unsigned char *ptr_read, int type);
uint32_t pmu_get_register_value_hp(uint32_t address, uint32_t mask, uint32_t shift);
pmu_operate_status_t pmu_set_register_value_hp(uint32_t address, uint32_t mask, uint32_t shift, uint32_t value);
void pmic_i2c_init(void);
void pmu_select_wdt_mode_hp(pmu_wdtrstb_act_t sel);
void pmu_get_pmic_info(void);
void pmu_get_lock_info(void);
void pmu_ripple_init_hp(pmu_power_operate_t oper);
void pmu_select_buck_ripple(pmu_power_domain_t pmu_pdm, pmu_power_operate_t oper);
void pmu_set_dummy_load_hp(pmu_power_domain_t domain, uint32_t loading_value);
void pmu_set_vaud18_mode(uint8_t val, uint8_t mode);
uint32_t pmu_modify_4bit_trim_hp(pmu_slt_mode_t oper, uint32_t buck_value, uint32_t divv_value);
uint32_t pmu_modify_5bit_trim_hp(pmu_slt_mode_t oper, uint32_t buck_value, uint32_t divv_value);
void pmu_switch_output_mode_hp(pmu_power_domain_t pmu_pdm, uint32_t divv_value, pmu_slt_mode_t oper);
void pmu_performance_level(pmu_slt_mode_t mode, uint32_t divv_value);
void pmu_dump_otp_hp(void);

#ifdef AIR_NVDM_ENABLE
pmu_status_t pmu_get_nvkey(uint16_t id, uint8_t *ptr, uint32_t size);
void pmu_dump_nvkey_hp(void);
#endif

void pmu_dump_rg_hp(void);
void pmu_vrf_keep_nm_hp(void);
void pmu_check_rg_timer_hp(uint16_t start, uint16_t end, uint16_t timer);
#endif /* End of HAL_PMU_MODULE_ENABLED */
#endif /* End of __HAL_PMU_INTERNAL_H__*/
