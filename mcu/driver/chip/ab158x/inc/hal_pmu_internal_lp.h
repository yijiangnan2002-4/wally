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

extern pmu_function_t pmu_function_table_lp[PMU_INT_MAX_LP];

/*==========[power]==========*/
pmu_power_vcore_voltage_t pmu_lock_vcore_lp(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol, pmu_lock_parameter_t lock);
pmu_power_vcore_voltage_t pmu_get_vcore_setting_index(uint16_t vcore);
pmu_power_vcore_voltage_t pmu_get_vcore_voltage_lp(void);
void pmu_ddie_sram_setting(pmu_vsram_dvs_state_t ste,pmu_power_vcore_voltage_t vol);
void pmu_enable_power_lp(pmu_power_domain_t pmu_pdm, pmu_power_operate_t operate);
void pmu_switch_power(pmu_power_domain_t pmu_pdm, pmu_buckldo_stage_t mode, pmu_power_operate_t operate);
void pmu_vcore_ipeak(int vol_index);
pmu_operate_status_t pmu_select_vcore_voltage_lp(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol);
void pmu_set_vaud18_voltage_lp(pmu_vaud18_vsel_t vsel, uint16_t volt);
void pmu_set_vsram_voltage_lp(pmu_vsram_voltage_t val);
uint32_t pmu_get_vsram_voltage_lp(void);

/*==========[audio]==========*/
pmu_operate_status_t pmu_set_micbias_vout_lp(pmu_micbias_index_t index,pmu_3vvref_voltage_t vol);
pmu_operate_status_t pmu_set_vaud18_vout_lp(pmu_vaud18_voltage_t lv, pmu_vaud18_voltage_t mv, pmu_vaud18_voltage_t hv);
void pmu_enable_micbias_inrush(pmu_micbias_index_t index,uint8_t operate);
void pmu_set_micbias_ldo_vout(pmu_micbias_index_t index);
void pmu_set_micbias_pulllow(pmu_micbias_index_t index,pmu_power_operate_t oper);
void pmu_enable_micbias_ldo(pmu_micbias_ldo_t ldo,pmu_micbias_mode_t mode);
void pmu_enable_micbias_mode(pmu_micbias_ldo_t ldo,pmu_micbias_mode_t mode);
void pmu_enable_micbias_lp(pmu_micbias_ldo_t ldo, pmu_micbias_index_t index, pmu_micbias_mode_t mode,pmu_power_operate_t operate);
void pmu_set_audio_mode_lp(pmu_audio_mode_t mode,pmu_power_operate_t operate);
void pmu_set_audio_mode_init(pmu_audio_mode_t mode);
void pmu_set_micbias_ref_voltage(pmu_3vvref_voltage_t val);
void pmu_set_micbias_ldo_mode_lp(pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode);
/*==========[eint]==========*/
void pmu_eint_init_lp(void);
void pmu_eint_handler_lp(void *parameter);
void pmu_pwrkey_hdlr(uint16_t pwrkey_flag);
void pmu_clear_all_intr(void);
pmu_status_t pmu_register_callback_lp(pmu_interrupt_index_lp_t pmu_int_ch, pmu_callback_t callback, void *user_data);
pmu_status_t pmu_deregister_callback_lp(pmu_interrupt_index_lp_t pmu_int_ch);

/*==========[pwrkey, captouch, usb]==========*/
pmu_operate_status_t pmu_pwrkey_normal_key_init_lp(pmu_pwrkey_config_t *config);
void pmu_latch_power_key_for_bootloader_lp(void);
bool pmu_get_pwrkey_state_lp(void);
void pmu_enable_pk_lpsd_lp(pmu_lpsd_time_t tmr,pmu_power_operate_t oper);
void pmu_enable_cap_lpsd_lp(pmu_power_operate_t oper);
void pmu_set_cap_duration_time_lp(pmu_lpsd_time_t tmr);
void pmu_set_cap_wo_vbus_lp(pmu_power_operate_t oper);
uint8_t pmu_get_usb_input_status_lp(void) ;
//void pmu_press_pk_time(void);

/*==========[Basic]==========*/
void pmu_config_lp(void);
void pmu_init_lp(void);
void pmu_power_off_sequence_lp(pmu_power_stage_t stage);
uint8_t pmu_get_power_off_reason_lp(void);
uint8_t pmu_get_power_on_reason_lp(void);
uint8_t pmu_get_pmic_version_lp(void);
void pmu_fast_buffer_disable(void);
void pmu_safety_confirmation_lp(void);
void pmu_sleep_suspend_lp(void);
void pmu_sleep_resume_lp(void);
void pmu_set_rstpat_lp(pmu_power_operate_t oper, pmu_rstpat_src_t src);
void pmu_dump_rg_lp(void);
uint32_t pmu_d2d_i2c_read(unsigned char *ptr_send, unsigned char *ptr_read, int type);
uint32_t pmu_get_register_value_lp(uint32_t address, uint32_t mask, uint32_t shift);
pmu_operate_status_t pmu_set_register_value_lp(uint32_t address, uint32_t mask, uint32_t shift, uint32_t value);
pmu_operate_status_t pmu_force_set_register_value_lp(uint32_t address, uint32_t value);
void pmic_i2c_init(void);
void pmic_i2c_deinit(void);
//void pmu_get_pmic_info(void);
//void pmu_get_lock_info(void);
void pmu_set_vaud18_pinout(pmu_audio_pinout_t mode);
void pmu_switch_performance_level(pmu_slt_mode_t mode);
void pmu_set_dummy_load_lp(pmu_power_domain_t domain, uint32_t loading_value);
void pmu_vrf_keep_nm_lp(void);
void pmu_check_rg_timer_lp(uint16_t start, uint16_t end, uint16_t timer);
void hal_pmu_buck_thd_test_lp(void);
void hal_pmu_buck_thd_test_wo_pg_lp(void);
#endif /* End of HAL_PMU_MODULE_ENABLED */
#endif /* End of __HAL_PMU_INTERNAL_H__*/
