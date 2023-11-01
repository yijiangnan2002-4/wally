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
#ifndef __HAL_PMU_CAL_LP_H__
#define __HAL_PMU_CAL_LP_H__
#ifdef HAL_PMU_MODULE_ENABLED

#include "hal_pmu_nvkey_struct.h"


#define NO_CAL    0
#define OTP_OK    1
#define CAL_DONE  3

#define OTP_BASE_ADDR                 (0x0000)
#define OTP_VBAT_ADDR                 (OTP_BASE_ADDR + 0)
#define OTP_BUCK_VOLT_ADDR            (OTP_BASE_ADDR + 13)
#define OTP_BUCK_IPEAK_ADDR           (OTP_BASE_ADDR + 26)
#define OTP_BUCK_FREQ_ADDR            (OTP_BASE_ADDR + 29)
#define OTP_BUCK_DL_ADDR              (OTP_BASE_ADDR + 34)
#define OTP_LDO_VSRAM_ADDR            (OTP_BASE_ADDR + 47)
#define OTP_LDO_VDIG18_ADDR           (OTP_BASE_ADDR + 49)
#define OTP_LDO_VDD33_REG_ADDR        (OTP_BASE_ADDR + 56)
#define OTP_LDO_VDD33_RET_ADDR        (OTP_BASE_ADDR + 63)
#define OTP_HPBG_ADDR                 (OTP_BASE_ADDR + 70)
#define OTP_LPBG_ADDR                 (OTP_BASE_ADDR + 77)
#define OTP_CHG_DAC_ADDR              (OTP_BASE_ADDR + 84)
#define OTP_VSYS_LDO_ADDR             (OTP_BASE_ADDR + 91)
#define OTP_OCP_ADDR                  (OTP_BASE_ADDR + 98)
#define OTP_CHG_4V2_CURR_ADDR         (OTP_BASE_ADDR + 101)
#define OTP_CHG_4V35_CURR_ADDR        (OTP_BASE_ADDR + 115)
#define OTP_VICHG_ADC_VAL_ADDR        (OTP_BASE_ADDR + 129)
#define OTP_LPO32_ADDR                (OTP_BASE_ADDR + 136)
#define OTP_SIDO_1_ADDR               (OTP_BASE_ADDR + 140)
#define OTP_SIDO_2_ADDR               (OTP_BASE_ADDR + 144)
#define OTP_SIDO_3_ADDR               (OTP_BASE_ADDR + 147)
#define OTP_SIDO_4_ADDR               (OTP_BASE_ADDR + 160)
#define OTP_ADIE_VER_ADDR             (OTP_BASE_ADDR + 163)


/************  global stur  **************/
typedef struct {
    uint16_t volt1;
    uint16_t adc1;
    uint16_t volt2;
    uint16_t adc2;
    uint8_t volt_sel;
    uint8_t two_step_en;
} bat_cfg_t;

typedef struct {
    VOLT_DAC dac_4v35;
    VOLT_DAC dac_4v2;
} volt_dac_t;

typedef struct {
    CURR_VAL bat_4v35[2];
    CURR_VAL bat_4v2[2];
} curr_val_t;

typedef struct {
    uint16_t cc1_curr;
    uint16_t cc2_curr;
} cc_curr_t;

typedef struct {
    uint16_t adc_4v35;
    uint16_t adc_4v2;
    uint16_t adc_4v05;
    uint16_t cv_stop_curr_perc;
} vichg_adc_t;

/****************************** OTP stru ******************************/
typedef struct {
    uint8_t kflag;
    VOLT_ADC bat_4v35;
    VOLT_ADC bat_4v2;
    VOLT_ADC bat_3v;
} PACKED otp_vbat_t;

typedef struct {
    uint8_t kflag;
    VOLT_SEL vio_nm;
    VOLT_SEL vio_lpm;
    VOLT_SEL vcore_nm;
    VOLT_SEL vcore_lpm;
} PACKED otp_buck_volt_t;

typedef struct {
    uint8_t kflag;
    uint8_t vio_sel;
    uint8_t vcore_sel;
} PACKED otp_buck_ipeak_t;

typedef struct
{
    uint8_t freq;
    uint8_t sel;
}PACKED FREQ_SEL;

typedef struct {
    uint8_t kflag;
    FREQ_SEL vio;
    FREQ_SEL vcore;
} PACKED otp_buck_freq_t;

typedef struct {
    uint8_t kflag;
    CURR_VAL vio18_dl_k[2];
    CURR_VAL vcore_dl_k[2];
} PACKED otp_buck_dl_t;

typedef struct {
    uint8_t kflag;
    uint8_t vsram_votrim;
} PACKED otp_ldo_vsram_t;

typedef struct {
    uint8_t kflag;
    VOLT_SEL data[2];
} PACKED otp_ldo_vdig18_t;

typedef struct {
    uint8_t kflag;
    VOLT_SEL data[2];
} PACKED otp_ldo_vdd33_t;

typedef struct {
    uint8_t kflag;
    VOLT_SEL data[2];
} PACKED otp_hpbg_t;

typedef struct {
    uint8_t kflag;
    VOLT_SEL data[2];
} PACKED otp_lpbg_t;

typedef struct {
    uint8_t kflag;
    uint16_t dac_4v35;
    uint16_t dac_4v2;
    uint16_t dac_4v05;
} PACKED otp_chg_dac_t;

typedef struct {
    uint8_t kflag;
    VOLT_SEL data[2];
} PACKED otp_vsys_ldo_t;

typedef struct {
    uint8_t kflag;
    uint8_t load_switch_ocp_trim;
    uint8_t vsys_ldo_ocp_trim;
} PACKED otp_ocp_t;

typedef struct {
    uint8_t kflag;
    CURR_VAL data[4];
} PACKED otp_chg_4v2_curr_t;

typedef struct {
    uint8_t kflag;
    CURR_VAL data[4];
} PACKED otp_chg_4v35_curr_t;

typedef struct {
    uint8_t kflag;
    uint16_t adc_4v35;
    uint16_t adc_4v2;
    uint16_t adc_4v05;
} PACKED otp_vichg_adc_val_t;

typedef struct {
    uint8_t kflag;
    uint16_t ftune;
    uint8_t ctune;
} PACKED otp_lpo32_t;

typedef struct {
    uint8_t kflag;
    uint8_t vaud18_nm_k;
    uint8_t vrf_nm_k;
    uint8_t vrf_lpm_k;
} PACKED otp_sido_1_t;

typedef struct {
    uint8_t kflag;
    uint8_t vrf_ipeak_k;
    uint8_t vaud18_ipeak_k;
} PACKED otp_sido_2_t;

typedef struct {
    uint8_t kflag;
    CURR_VAL vrf_dl_k[2];
    CURR_VAL vaud18_dl_k[2];
} PACKED otp_sido_3_t;

typedef struct {
    uint8_t kflag;
    uint8_t vrf_freq;
    uint8_t vaud18_freq;
} PACKED otp_sido_4_t;

typedef struct {
    uint8_t version;
} PACKED otp_aide_version;


extern vbat_volt_cfg_t pmu_vbat_volt;
extern bool pmu_otp_failed;

/******************************************extern API****************************************/
uint16_t pmu_lerp(uint16_t volt1, uint16_t adc1, uint16_t volt2, uint16_t adc2, uint16_t volt_val);
void pmu_cal_init(void);
void pmu_set_init(void);
void pmu_dump_otp_lp(void);
void pmu_dump_nvkey_lp(void);
pmu_status_t pmu_get_nvkey(uint16_t id, uint8_t *ptr, uint32_t size);
pmu_status_t pmu_set_nvkey(uint16_t id, uint8_t *ptr, uint32_t size);
pmu_status_t pmu_get_otp(uint16_t addr, uint8_t *ptr, uint32_t size);

#endif  /* HAL_PMU_MODULE_ENABLED */
#endif  /* __HAL_PMU_CAL_LP_H__ */
