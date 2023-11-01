/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

/**
 * File: hal_pmu_nvkey_struct.h
 *
 * Description: This file defines the structure format of NVKey.
 *
 */

#ifndef __HAL_PMU_NVKEY_STRUCT_H__
#define __HAL_PMU_NVKEY_STRUCT_H__

/* may be better to use AIR_NVDM_ENABLE, but need to modify some lp files for 1585, because bootloader will build fail */
#ifdef HAL_PMU_MODULE_ENABLED

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

/*********************    member_stur    *****************************/
typedef struct
{
    uint16_t volt;
    uint16_t adc;
}PACKED VOLT_ADC;

typedef struct
{
    uint16_t volt;
    uint16_t dac;
}PACKED VOLT_DAC;

typedef struct
{
    uint16_t volt;
    uint8_t sel;
}PACKED VOLT_SEL;

typedef struct
{
    uint16_t volt;
    uint8_t sel;
    uint16_t rsv;
}PACKED VOLT_SEL_RSV;

typedef struct
{
    uint16_t curr;
    uint16_t sel;
}PACKED CURR_SEL;

typedef struct
{
    uint16_t perc;
    uint16_t adc;
}PACKED PERC_ADC;

typedef struct
{
    uint16_t curr;
    uint8_t val;
}PACKED CURR_VAL;

//8x only
typedef struct
{
    uint16_t curr;
    uint8_t val;
    uint16_t rsv;
}PACKED CURR_VAL_RSV;

typedef struct
{
    uint16_t volt;
    uint8_t val;
    uint8_t ipeak;
}PACKED VOLT_VAL_IPEAK;

/*********************    nvkey_stur    *****************************/
/**
 *  @brief This structure defines the PMU NVKey.
 */
//0x2000
typedef struct
{
    uint8_t otp_dump;
    uint8_t bat_volt_sel;
    uint8_t two_step_en;
    uint8_t kflag;
    uint16_t bat_volt_sel2;
}chg_cfg_t;

//0x2001
typedef struct
{
    uint16_t cc1_thrd_volt;
    uint16_t cc1_thrd_adc;
    uint16_t rsv1;
    uint16_t rsv2;
    uint16_t cc2_thrd_volt;
    uint16_t cc2_thrd_adc;
    uint16_t rsv3;
    uint16_t rsv4;
    uint16_t cv_thrd_volt;
    uint16_t cv_thrd_adc;
    uint16_t rsv5;
    uint16_t rsv6;
    uint16_t full_bat_volt;
    uint16_t rsv8;
    uint16_t rsv9;
    uint16_t rsv10;
    uint16_t rechg_volt;
    uint16_t rechg_adc;
    uint16_t rsv11;
    uint16_t rsv12;
    uint16_t rsv13;
    uint16_t rsv14;
    uint16_t rsv15;
    uint16_t rsv16;
}chg_adc_cfg_t;

//0x2002
typedef struct
{
    uint16_t tri_curr_dac;
    uint16_t rsv1;
    uint16_t cc1_curr_dac;
    uint16_t rsv2;
    uint16_t cc2_curr_dac;
    uint16_t rsv3;
    uint16_t cv_dac;
    uint16_t rsv4;
}chg_dac_cfg_t;

//0x2003
typedef struct
{
    uint8_t cal_cnt;
    uint8_t sel;
    CURR_SEL data[10];
}chg_tri_curr_cfg_t;

//0x2004
typedef struct
{
    uint8_t cal_cnt;
    uint8_t sel;
    CURR_SEL data[10];
}chg_cc1_curr_cfg_t;

//0x2005
typedef struct
{
    uint8_t cal_cnt;
    uint8_t sel;
    CURR_SEL data[10];
}chg_cc2_curr_cfg_t;

//0x2006
typedef struct
{
    uint8_t cal_cnt;
    uint8_t sel;
    PERC_ADC cv_stop_curr[2];
    uint8_t chg_pre_compl_int_en;
    uint8_t pre_compl_cal_cnt;
    uint8_t pre_compl_sel;
    PERC_ADC pre_compl_curr[2];
}PACKED cv_stop_curr_cfg_t;

//0x2007
typedef struct
{
    uint16_t sysldo_output_volt;
    uint16_t chg_ldo_sel;
}sys_ldo_t;

//0x2008
typedef struct
{
    uint16_t sw_oc_lmt;
    uint16_t i_lim_trim;
}ocp_t;

//0x2009
typedef struct
{
    uint16_t cool_state_curr_perc;
    uint16_t cool_state_dac_dec;
    uint16_t warm_state_curr_perc;
    uint16_t warm_state_dac_dec;
}jeita_t;

//0x200A
typedef struct
{
    uint16_t cc2_thrd_volt;
    uint16_t cc2_thrd_adc;
    uint16_t cv_thrd_volt;
    uint16_t cv_thrd_adc;
    uint16_t rsv1;
    uint16_t rsv2;
    uint16_t rechg_volt;
    uint16_t rechg_adc;
    uint16_t rsv3;
    uint16_t rsv4;
    uint16_t cc1_curr_dac;
    uint16_t cc2_curr_dac;
    uint16_t cv_dac;
    uint16_t cc1_curr;
    uint16_t cc1_rsel;
    uint16_t cc2_curr;
    uint16_t cc2_rsel;
    uint16_t cv_stop_curr_adc;
    uint16_t pre_compl_curr_adc;
}jeita_warm_t;

//0x200B
typedef struct
{
    uint16_t cc2_thrd_volt;
    uint16_t cc2_thrd_adc;
    uint16_t cv_thrd_volt;
    uint16_t cv_thrd_adc;
    uint16_t rsv1;
    uint16_t rsv2;
    uint16_t rechg_volt;
    uint16_t rechg_adc;
    uint16_t rsv3;
    uint16_t rsv4;
    uint16_t cc1_curr_dac;
    uint16_t cc2_curr_dac;
    uint16_t cv_dac;
    uint16_t cc1_curr;
    uint16_t cc1_rsel;
    uint16_t cc2_curr;
    uint16_t cc2_rsel;
    uint16_t cv_stop_curr_adc;
    uint16_t pre_compl_curr_adc;
}jeita_cool_t;

//0x2010
typedef struct
{
    uint8_t vcore_dummy_en;
    CURR_VAL otp[2];
    uint8_t cal_cnt;
    uint8_t sel;
    CURR_VAL_RSV data[8];
}PACKED vcore_dl_t;

//0x2011
typedef struct
{
    uint8_t en;
    CURR_VAL otp[2];
    uint8_t cal_cnt;
    uint8_t sel;
    CURR_VAL_RSV data[8];
}PACKED vio18_dl_t;

//0x2012, 0x2015
typedef struct
{
    uint8_t kflag;
    uint8_t buck_freq;
    uint8_t osc_freqk;
}PACKED buck_freq_65_t;

//0x2013
typedef struct
{
    uint8_t en;
    CURR_VAL otp[2];
    uint8_t cal_cnt;
    uint8_t sel;
    CURR_VAL_RSV data[8];
}PACKED vaud18_dl_t;

//0x2014
typedef struct
{
    uint8_t en;
    CURR_VAL otp[2];
    uint8_t cal_cnt;
    uint8_t sel;
    CURR_VAL_RSV data[8];
}PACKED vrf_dl_t;

//0x2016-0x201C, 0x2030-0x2033
typedef struct
{
    uint8_t kflag;
    VOLT_SEL otp[2];
    uint8_t cal_cnt;
    uint8_t sel;
    VOLT_SEL_RSV data[8];
}PACKED buck_ldo_cfg_t;

//0x201D, 0x201E
typedef struct
{
    uint8_t kflag;
    VOLT_SEL otp[2];
    uint8_t cal_cnt;
    uint8_t sel;
    VOLT_SEL_RSV data[2];
}PACKED bg_cfg_t;

//0x2020
typedef struct
{
    uint8_t kflag;
    uint8_t cal_cnt;
    VOLT_ADC data[10];
}PACKED vbat_adc_cal_t;

//0x2021
typedef struct
{
    uint8_t kflag;
    VOLT_ADC init_bat;
    VOLT_ADC rsv1;
    VOLT_ADC sd_bat;
    VOLT_ADC data[18];
}PACKED vbat_volt_cfg_t;

//0x2022
typedef struct
{
    uint8_t ripple_vcore_en;
    uint8_t ripple_vio18_en;
    uint8_t ripple_vaud18_en;
    uint8_t ripple_vpa_en;
    uint8_t ripple_vrf_en;
}PACKED buck_ripple_ctrl_t;

//0x2023
typedef struct
{
    uint8_t vcore_nm_trim;
    uint8_t vcore_lpm_trim;
    uint8_t vio18_nm_trim;
    uint8_t vio18_lpm_trim;
}PACKED buck_volt_trim_t;

//0x2024
typedef struct
{
    uint8_t vcore_ipeak_trim;
    uint8_t vio18_ipeak_trim;
}PACKED buck_ipeak_t;

//0x2025
typedef struct
{
    uint8_t vcore_freq;
    uint8_t vio18_freq;
}PACKED buck_freq_t;

//0x2026
typedef struct
{
    uint8_t vaud18_nm_trim;
    uint8_t vaud18_lpm_trim;
    uint8_t vrf_nm_trim;
    uint8_t vrf_lpm_trim;
}PACKED sido_volt_cfg_t;

//0x2027
typedef struct
{
    uint8_t vaud18_ipeak_trim;
    uint8_t vrf_ipeak_trim;
}PACKED sido_ipeak_t;

//0x2028
typedef struct
{
    uint8_t vaud18_freq;
    uint8_t vrf_freq;
}PACKED sido_freq_t;

//0x2029
typedef struct
{
    uint8_t kflag;
    VOLT_VAL_IPEAK hv;
    VOLT_VAL_IPEAK mv;
    VOLT_VAL_IPEAK lv;
    VOLT_VAL_IPEAK slp;
    VOLT_VAL_IPEAK rsv;
}PACKED buck_vcore_volt_cfg_t;

//0x202A
typedef struct
{
    uint8_t kflag;
    VOLT_VAL_IPEAK nm;
    VOLT_VAL_IPEAK slp;
    VOLT_VAL_IPEAK rsv[3];
}PACKED buck_vio18_volt_cfg_t;

//0x202B
typedef struct
{
    uint8_t kflag;
    VOLT_VAL_IPEAK nm;
    VOLT_VAL_IPEAK slp;
    VOLT_VAL_IPEAK rsv[3];
}PACKED buck_vrf_volt_cfg_t;

//0x202C
typedef struct
{
    uint8_t kflag;
    VOLT_VAL_IPEAK hv;
    VOLT_VAL_IPEAK mv;
    VOLT_VAL_IPEAK lv;
    VOLT_VAL_IPEAK rsv[2];
}PACKED buck_vaud18_volt_cfg_t;

//0x202E
/*typedef struct
{
    uint8_t vpa_dummy_en;
    CURR_VAL res1[2];
    uint8_t rsv2;
    uint8_t rsv3;
    uint16_t dl;
    uint8_t val;
    uint8_t rsv4;
    VOLT_SEL_RSV rsv5[7];
}PACKED vpa_dummy_load_t;*/

// 0x2034
typedef struct
{
    uint8_t ftune;
    uint8_t ctune;
}PACKED lpo32_cfg_t;

//0x2048
typedef struct
{
    uint16_t volt;
    uint8_t vcore_nm_trim;
}PACKED ldo_vsram_cfg_t;

typedef struct
{
    uint8_t ll_low;
    uint8_t ll_hgh;
    uint8_t hh_low;
    uint8_t hh_hgh;
}PACKED RST_PAT_CFG;

//0x2300
typedef struct
{
    uint8_t enable;
    uint8_t rst_pat;
    uint8_t cfg_en;
    RST_PAT_CFG cfg;
    uint8_t rsv1;
    uint8_t rsv2;
    uint8_t rsv3;
    uint8_t rsv4;
}PACKED pmu_rst_pat_cfg_t;

/*****************    NTC    *****************/
#define NTC_DATA_SIZE               80

//0x20F1
typedef struct
{
    uint8_t enable;
    uint8_t k_flag;
    uint8_t dump;
    uint8_t rsv1[3];
    uint8_t avg_cnt;
    uint8_t rsv2[2];
    uint8_t interval;
    uint8_t rsv3[5];
} PACKED pmu_ntc_cfg1_t;

//0x20F2
typedef struct
{
    int8_t hot;
    int8_t warm;
    int8_t rsv1[2];
    int8_t cool;
    int8_t cold;
    int8_t rsv2[4];
    int8_t burning;
    int8_t rsv3[2];
    int8_t frozen;
    int8_t rsv4[4];
}PACKED pmu_ntc_cfg2_t;

//0x20F3
typedef struct
{
    uint8_t cnt;
    int8_t data[NTC_DATA_SIZE];
}PACKED pmu_ntc_temp_t;

//0x20F4
typedef struct
{
    uint8_t cnt;
    uint16_t data[NTC_DATA_SIZE];
}PACKED pmu_ntc_res_t;

//0x20F5
typedef struct
{
    uint8_t cnt;
    uint32_t data[NTC_DATA_SIZE];
}PACKED pmu_ntc_r2_t;

//0x20F6
typedef struct
{
    uint8_t cnt;
    uint16_t data[NTC_DATA_SIZE];
}PACKED pmu_ntc_ratio_t;


/* ================= A die H NvKey Structure ================= */
/* =========================================================== */
/* NVID_CAL_GENERAL_CFG 0x2049 */
typedef struct {
    uint16_t pwrhold_check;
    uint16_t vbus_debounce;
} PACKED nvid_cal_general_cfg_t;


/* NVID_CAL_CHG_CTL_CFG 0x204A */
typedef struct {
    uint16_t bc12_support;
    uint16_t icl_config;
    uint16_t jeita_en;
    uint16_t warm_cool_en;
    uint16_t precc_timer_en;
    uint16_t precc_safety_time;
    uint16_t fastcc_timer_en;
    uint16_t fastcc_safety_time;
    uint16_t extend_charging;
} PACKED nvid_cal_chg_ctl_cfg_t;

/* NVID_CAL_BAT_MGMT_BASIC 0x20A0 */
#define NVID_CAL_BAT_MGMT_BASIC_FIELD_MAX  12

typedef struct {
    uint16_t field_arr[NVID_CAL_BAT_MGMT_BASIC_FIELD_MAX];
    uint16_t full_bat_voltage;
    uint16_t full_bat_adc;
    uint16_t full_bat_voltage_offset;
    uint16_t full_bat_adc_offset;
    uint16_t recharge_voltage;
    uint16_t recharge_adc;
    uint16_t recharge_voltage_offset;
    uint16_t recharge_adc_offset;
    uint16_t precc_current_setting;
    uint16_t cv_termination_current;
    uint16_t s1_voltage;
    uint16_t s1_multi_level;
    uint16_t s2_voltage;
    uint16_t s2_multi_level;
    uint16_t cool_charger_current;
    uint16_t cool_cv_flag;
    uint16_t warm_charger_current;
    uint16_t warm_cv_flag;
    uint16_t iterm_irq;
    uint16_t s0_voltage;
    uint16_t s0_current;
    uint16_t battery_category;
    uint16_t _2step_iterm_irq1;
    uint16_t _2step_cv1;
    uint16_t _2step_iterm_irq2;
    uint16_t _2step_cv2;
    uint16_t _2step_cv_enable;
    uint16_t _2step_cc_enable;
} PACKED nvid_cal_bat_mgmt_basic_t;

/* NVID_CAL_BAT_MGMT_CHR 0x20D0 */
#define NVID_CAL_BAT_MGMT_CHR_CHECK_POINT_MAX  9

typedef struct
{
    uint16_t check_point;
    uint16_t adc;
} PACKED nvid_cal_bat_mgmt_chr_check_point_t;

typedef struct {
    uint8_t  k_flag;
    uint16_t initial_bat;
    uint16_t initial_bat_adc;
    uint16_t low_bat;
    uint16_t low_bat_adc;
    uint16_t shutdown_bat;
    uint16_t shutdown_bat_adc;
    nvid_cal_bat_mgmt_chr_check_point_t check_point_arr[NVID_CAL_BAT_MGMT_CHR_CHECK_POINT_MAX];
    uint16_t rsv1;
    uint16_t rsv1_adc;
    uint16_t cold;
    uint16_t cool_cc;
    uint16_t cool;
    uint16_t cool_cv;
    uint16_t warm;
    uint16_t warm_cc;
    uint16_t hot;
    uint16_t warm_cv;
    uint16_t rsv6;
    uint16_t rsv6_adc;
    uint16_t rsv7;
    uint16_t rsv7_adc;
    uint16_t rsv8;
    uint16_t rsv8_adc;
    uint16_t rsv9;
    uint16_t rsv9_adc;
} PACKED nvid_cal_bat_mgmt_chr_t;

#endif  /* HAL_PMU_MODULE_ENABLED */

#endif  /* __HAL_PMU_NVKEY_STRUCT_H__ */
