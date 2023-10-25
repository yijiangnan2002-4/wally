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
 *
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

#ifndef __HAL_PMU_DDIE_H__
#define __HAL_PMU_DDIE_H__
#ifdef HAL_PMU_MODULE_ENABLED


/*=========[CAPTOUCH]=========*/
void pmu_cap_touch_config(pmu_strup_mux_t strup_mux,int cap_rstb , int cap2core_iso_en, int ana_pwr_en_cpt, int lsh_en_cpt, int cap_in_en);
uint8_t pmu_enable_captouch_power_ddie(pmu_power_operate_t oper);
/*=========[USB : VBUS]=========*/
void pmu_select_vbus_vosel(uint32_t val);
uint8_t pmu_select_vbus_vosel_ddie(void);
uint8_t pmu_enable_usb_power_ddie(pmu_power_operate_t oper);
/*=========[Power]=========*/
void pmu_select_vsram_vosel_ddie(pmu_power_stage_t mode , pmu_vsram_voltage_t val);
uint32_t pmu_get_vsram_vosel_ddie(pmu_power_stage_t mode);
void pmu_enable_power_ddie(pmu_power_domain_t domain, pmu_power_operate_t oper);
void pmu_set_power_state_ddie(pmu_power_stage_t ste);
void pmu_set_bgrbuf(int vref06_pl_dis, int bgr_buf_forceen, int hpbg_chop_en, int lpf_rsel, int bypass_mode_en, int zero_comp_en);
/*=======[BASIC]========*/
void pmu_strup_latch_register(uint16_t data, pmu_strup_mux_t sel);
uint8_t pmu_get_power_off_reason_ddie(void);
uint8_t pmu_get_power_on_reason_ddie(void);
void pmu_eint_isr(void);
void pmu_ddie_eint_init(void);
void pmu_init_ddie(void);
/*==========[PMU Debug GPIO]==========*/
void pmu_latch_con2(int rg_mon_sel, int rg_mon_en, int rg_dbg_high_byte_sel, int stup_lvsh_en);
pmu_operate_status_t pmu_set_register_ddie(uint32_t address, short int mask, short int shift, short int value);
uint32_t pmu_get_register_ddie(uint32_t address, short int mask, short int shift);
void pmu_set_rtc_gpio(pmu_strup_mux_t gpio_index , pmu_power_operate_t oper);
void pmu_rtc_gpio_config(pmu_strup_mux_t strup_mux,uint8_t data);
uint32_t pmu_get_rtc_gpio_status(pmu_strup_mux_t strup_mux);
void pmu_select_rtc_voltage_ddie(pmu_vrtc_voltage_t vol);
void pmu_set_debug_mode (int gpio0_mode, int gpio1_mode, int gpio2_mode, int gpio3_mode);
/*==========[Efuse]==========*/
void pmu_efuse_init_ddie(void);
/*==========[BC 1.2 behavior]==========*/
void pmu_bc12_init(void);
void pmu_bc12_end(void);
uint8_t pmu_bc12_dcd(void);
uint8_t pmu_bc12_primary_detction(void);
uint8_t pmu_bc12_secondary_detection(void);
uint8_t pmu_bc12_check_DCP(void);
uint8_t pmu_bc12_check_dp(void);
uint8_t pmu_bc12_check_dm(void);
uint8_t pmu_bc12_check_floating(void);
uint8_t pmu_get_bc12_charger_type(void);
void pmu_dump_rg_ddie(void);


#endif /* end of HAL_PMU_MODULE_ENABLED */
#endif /* end of __HAL_PMU_DDIE_H__  */
