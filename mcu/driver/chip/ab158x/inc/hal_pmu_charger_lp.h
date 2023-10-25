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


#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif


typedef enum {
    CHG_IDLE,
    CHG_TRICKLE,
    CHG_CC1,
    CHG_CC2,
    CHG_CV_INIT,
    CHG_CV,
    CHG_COMPL,
    CHG_RECHG,
} pmu_chg_state_t;

#define EFUSE_NTC_R2_ADDR           0x420C0304
#define EFUSE_NTC_RAGPIO_ADDR       0x420C0300
#define EFUSE_NTC_DEF_R2            11693
#define EFUSE_NTC_DEF_RAGPIO        719


typedef enum {
    PMU_NTC_NORM,
    PMU_NTC_WARM,
    PMU_NTC_COOL,
    PMU_NTC_HOT,
    PMU_NTC_COLD,
    PMU_NTC_PWR_OFF,
} pmu_ntc_state_t;

typedef struct {
    pmu_callback_t callback1; //chg in callback
    void *user_data1;
    pmu_callback_t callback2; //chg out callback
    void *user_data2;
    pmu_callback_t callback3; //chg compl callback
    void *user_data3;
    pmu_callback_t callback4; //chg rechg callback
    void *user_data4;
} pmu_charger_config_t;

typedef enum {
    PMU_CHG_OUT_DEB_0MS,
    PMU_CHG_OUT_DEB_10MS,
    PMU_CHG_OUT_DEB_20MS,
    PMU_CHG_OUT_DEB_40MS,
    PMU_CHG_IN_DEB_0MS = 0,
    PMU_CHG_IN_DEB_16MS,
    PMU_CHG_IN_DEB_40MS,
    PMU_CHG_IN_DEB_128MS,
} pmu_chg_deb_time_t;

typedef enum {
    PMU_CHG_IN_INT_FLAG,
    PMU_CHG_IN_INT_EN,
    PMU_CHG_OUT_INT_FLAG,
    PMU_CHG_OUT_INT_EN,
    PMU_CHG_COMPLETE_INT_FLAG,
    PMU_CHG_COMPLETE_INT_EN,
    PMU_CHG_RECHG_INT_FLAG,
    PMU_CHG_RECHG_INT_EN,
} pmu_chg_int_mask_t;

typedef enum {
    PMU_RSEL_NORM,
    PMU_RSEL_WARM,
    PMU_RSEL_COOL,
    PMU_RSEL_VBAT,
    PMU_RSEL_MAX,
} pmu_rsel_state_t;

typedef enum {
    PMU_PRECC_SETTING,
    PMU_CC_SETTING,
    PMU_ITERM_SETTING,
} pmu_chg_setting_t;

typedef struct {
    uint16_t state;
    uint16_t cc1[PMU_RSEL_MAX];
    uint16_t cc2[PMU_RSEL_MAX];
} pmu_chg_rsel_t;

typedef struct {
    uint16_t cc1_thrd_volt;
    uint16_t cc1_curr;
    uint16_t cc2_thrd_volt;
    uint16_t cc2_curr;
    uint16_t full_bat_volt;
    uint16_t rechg_volt;
    uint16_t cv_stop_curr_adc;
} pmu_chg_info_t;

extern uint8_t g_ntc_state;
extern pmu_chg_rsel_t chg_rsel;
extern pmu_chg_info_t pmu_chg_info;

void pmu_chg_rsel_ctl(pmu_rsel_state_t state);
uint32_t pmu_chg_vchg_to_volt(uint32_t adc);

bool pmu_get_chr_detect_value_lp(void);
uint32_t pmu_get_charger_state_lp(void);
uint8_t pmu_enable_charger_lp(uint8_t isEnableCharging);
void pmu_ntc_update_state(int *ret_temp, pmu_ntc_state_t *curr_state);
void pmu_chg_in_hdlr(void);
void pmu_chg_out_hdlr(void);
void pmu_chg_pre_comp_hdlr(void);
void pmu_chg_compl_hdlr(void);
void pmu_chg_rechg_hdlr(void);
void pmu_chg_hdlr(uint16_t chg_flag);
void pmu_chg_init(void);
void pmu_rst_pat_init(void);

/****  1wire  ****/
void pmu_vio18_pull_up(pmu_power_operate_t en);
void pmu_uart_psw_cl(pmu_power_operate_t en);
void pmu_uart_psw(pmu_power_operate_t en);
void pmu_uart_psw_sequence(void);
void pmu_vchg_dischg_path(pmu_power_operate_t en);
void pmu_eoc_ctrl(pmu_power_operate_t en);

uint8_t pmu_ntc_get_enable_status(void);
uint8_t pmu_ntc_get_interval(void) ;
void pmu_ntc_init(void);
void pmu_ntc_cfg(pmu_ntc_state_t state);
bool pmu_ntc_slt(void);

extern void pmu_bat_init(void);
extern uint8_t pmu_bat_volt_to_perc(uint32_t voltval);
extern uint16_t pmu_bat_volt_to_k_perc(uint32_t volt);
extern uint32_t pmu_bat_volt_to_adc(uint32_t volt);
extern uint32_t pmu_bat_adc_to_volt(uint32_t adcval);
extern void pmu_bat_3v3_proc(uint32_t bat_volt);
extern uint16_t pmu_bat_get_pure_vbat(void);

extern void pmu_chg_set_cc_current(uint16_t set_value);
extern void pmu_chg_set_cc2_ctrl_en(uint8_t enable);
extern void pmu_chg_set_cc2_current(uint16_t set_value);
uint16_t pmu_get_chg_setting(pmu_chg_setting_t chg_setting_type);

#endif /* End of HAL_PMU_MODULE_ENABLED */
#endif /* End of __HAL_PMU_CHARGER_H__*/

