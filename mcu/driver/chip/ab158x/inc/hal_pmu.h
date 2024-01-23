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

#include "hal_platform.h"

#ifndef __HAL_PMU_MODULE_H__

#define __HAL_PMU_MODULE_H__
#ifdef HAL_PMU_MODULE_ENABLED
#define PMIC_SLAVE_ADDR            0x6B
#define INVALID_INTERRUPT_VALUE    0xFF
#define PMU_PRESS_PK_TIME 300000  //300ms
#define ONLY_E1_SETTING
//#define PMU_BC12_DCD_ENABLE
/* Avoid accidentally touching the power key to boot on*/
//#define PMU_AVOID_ACCIDENTAL_BOOTUP

typedef enum {
    PMU_KEY = 0x1,
    PMU_RTCA = 0x2,
    PMU_CHRIN = 0x4,
    PMU_CHROUT = 0x8,
    PMU_RBOOT = 0x10,
    BOOT_MODE = 0x8000,
} pmu_power_on_reason_t;

typedef enum {
    PMU_OFF_MODE = 0,
    PMU_RTC_MODE = 1,
    PMU_WD_RST = 8,
    PMU_REGEN_LPSD = 10,
    PMU_SYS_RST = 13,
    PMU_CAP_LPSD = 14,
} pmu_power_off_reason_t;

typedef enum {
    VSRAM_VOLTAGE_0P5,
    VSRAM_VOLTAGE_0P56,
    VSRAM_VOLTAGE_0P58,
    VSRAM_VOLTAGE_0P6,
    VSRAM_VOLTAGE_0P65,
    VSRAM_VOLTAGE_0P78,
    VSRAM_VOLTAGE_0P8,
    VSRAM_VOLTAGE_0P82,
    VSRAM_VOLTAGE_0P84,
    VSRAM_VOLTAGE_0P86,
    VSRAM_VOLTAGE_0P88,
    VSRAM_VOLTAGE_0P9,
    VSRAM_VOLTAGE_0P92,
    VSRAM_VOLTAGE_0P97,
    VSRAM_VOLTAGE_0P98,
    VSRAM_VOLTAGE_1P0,
} pmu_vsram_voltage_t;

typedef enum {
    VRTC_VOLTAGE_0P65,
    VRTC_VOLTAGE_0P6625,
    VRTC_VOLTAGE_0P675,
    VRTC_VOLTAGE_0P6875,
    VRTC_VOLTAGE_0P7,
    VRTC_VOLTAGE_0P7125,
    VRTC_VOLTAGE_0P725,
    VRTC_VOLTAGE_0P7375,
    VRTC_VOLTAGE_0P75,
    VRTC_VOLTAGE_0P7625,
    VRTC_VOLTAGE_0P775,
    VRTC_VOLTAGE_0P7875,
    VRTC_VOLTAGE_0P8,
    VRTC_VOLTAGE_0P8125,
    VRTC_VOLTAGE_0P825,
    VRTC_VOLTAGE_0P8325,
} pmu_vrtc_voltage_t;

typedef enum {
    PMU_STRUP_LATCH_CON0    = 0,
    PMU_STRUP_LATCH_CON1    = 1,
    PMU_STRUP_LATCH_CON2    = 2,
    PMU_STRUP_RTC_GPIO0  = 3,
    PMU_STRUP_RTC_GPIO1  = 4,
    PMU_STRUP_RTC_GPIO2  = 5,
    PMU_STRUP_RTC_GPIO3  = 6,
    PMU_STRUP_CAPTOUCH      = 7,
} pmu_strup_mux_t;

typedef enum {
    PMU_LOWPOWER_MODE,
    PMU_NORMAL_MODE,
} pmu_buckldo_stage_t;

typedef enum {
    PMU_POWER_STABLE = 0,
    PMU_POWER_TRY_TO_ENABLE,
    PMU_POWER_TRY_TO_DISABLE,
} pmu_power_status_t;

typedef enum {
    PMU_PWROFF,
    PMU_RTC,
    PMU_SLEEP,
    PMU_NORMAL,
    PMU_DVS,
    PMU_RTC_EXCEPTION,
} pmu_power_stage_t;

typedef enum {
    PMU_BUCK_VCORE,
    PMU_BUCK_VIO18,
    PMU_BUCK_VRF,
    PMU_BUCK_VAUD18,
    PMU_BUCK_VPA,
    PMU_LDO_VA18,
    PMU_LDO_VLDO31,
    PMU_LDO_VLDO33,
    PMU_LDO_VSRAM,
    PMU_LDO_VRF,
    PMU_VDIG18,
    PMU_VRTC,
} pmu_power_domain_t;

typedef enum {
    PMU_INVALID = -1,
    PMU_ERROR   = 0,
    PMU_OK    = 1
} pmu_operate_status_t;

typedef enum {
    PMU_OFF    = 0,
    PMU_ON   = 1,
} pmu_power_operate_t;

typedef enum {
    RG_INT_PWRKEY,      //INT_CON0 index :0
    RG_INT_PWRKEY_R,
    RG_INT_EN_AVDD50_OC,
    RG_INT_VBAT_RECHG,
    RG_INT_JEITA_HOT,
    RG_INT_JEITA_WARM,
    RG_INT_JEITA_COOL,
    RG_INT_JEITA_COLD,
    RG_INT_BATOV,
    RG_INT_CHRDET,
    RG_INT_ChgStatInt,
    RG_INT_VBUS_OVP,
    RG_INT_VBUS_UVLO,
    RG_INT_ICHR_ITERM,
    RG_INT_ICHR_CHG_CUR,
    RG_INT_SAFETY_TIMEOUT,
    RG_INT_AD_LBAT_LV,     //INT_CON1 index :16
    RG_INT_THM_OVER40,
    RG_INT_THM_OVER55,
    RG_INT_THM_OVER110,
    RG_INT_THM_OVER125,
    RG_INT_THM_UNDER40,
    RG_INT_THM_UNDER55,
    RG_INT_THM_UNDER110,
    RG_INT_THM_UNDER125,
    RG_INT_ILimInt,
    RG_INT_ThermRegInt,
    RG_INT_VSYS_DPM,
    RG_INT_JEITA_TO_NORMAL,
    PMU_INT_MAX,
} pmu_interrupt_index_t;

typedef enum {
    PMU_STATUS_INVALID_PARAMETER  = -1,     /**< pmu error invalid parameter */
    PMU_STATUS_ERROR              = 0,     /**< pmu undefined error */
    PMU_STATUS_SUCCESS            = 1       /**< pmu function ok */
} pmu_status_t;

typedef enum {
    PMU_LOCK = 0,
    PMU_UNLOCK,
    PMU_TEMP
} pmu_lock_parameter_t;

typedef enum {
    PMU_PK_PRESS    = 0,
    PMU_PK_RELEASE   = 1,
} pmu_pk_operate_t;

typedef enum {
    PMIC_VCORE_0P525_V,
    PMIC_VCORE_0P55_V,
    PMIC_VCORE_0P65_V,
    PMIC_VCORE_0P75_V,
    PMIC_VCORE_0P80_V,
    PMIC_VCORE_0P85_V,
    PMIC_VCORE_FAIL_V,
} pmu_power_vcore_voltage_t;

typedef enum {
    PMIC_DVS_DEFAULT = 0,
    PMIC_RISING_0P55_0P65 = 1,    /* VCORE WI/VSRAM WO wait */
    PMIC_RISING_0P65_0P8 = 2,    /* VCORE WI/VSRAM WI wait */
    PMIC_RISING_0P55_0P8 = 3,    /* VCORE WI/VSRAM WI wait */
    PMIC_FALLING_0P8_0P65 = 4,   /* VCORE WI/VSRAM WI wait */
    PMIC_FALLING_0P65_0P55 = 5,   /* VCORE WO/VSRAM WO wait */
    PMIC_FALLING_0P8_0P55 = 6,   /* VCORE WI/VSRAM WI wait */
} pmu_vsram_dvs_state_t;

typedef enum {
    PMU_VAUD18_VSEL_L,
    PMU_VAUD18_VSEL_M,
    PMU_VAUD18_VSEL_H,
} pmu_vaud18_vsel_t;

typedef enum {
    PMU_VAUD18_0P7_V  = 700,
    PMU_VAUD18_0P8_V  = 800,
    PMU_VAUD18_0P87_V = 870,
    PMU_VAUD18_0P88_V = 880,
    PMU_VAUD18_0P9_V  = 900,
    PMU_VAUD18_1P1_V  = 1100,
    PMU_VAUD18_1P2_V  = 1200,
    PMU_VAUD18_1P3_V  = 1300,
    PMU_VAUD18_1P73_V = 1730,
    PMU_VAUD18_1P75_V = 1750,
    PMU_VAUD18_1P77_V = 1770,
    PMU_VAUD18_1P8_V  = 1800,
	PMU_VAUD18_1P85_V  = 1850,
} pmu_vaud18_voltage_t;

typedef enum {
    PMU_3VVREF_1P8_V,
    PMU_3VVREF_2P78_V,
    PMU_3VVREF_1P9_V,
    PMU_3VVREF_2P0_V,
    PMU_3VVREF_2P1_V,
    PMU_3VVREF_2P2_V,
    PMU_3VVREF_2P4_V,
    PMU_3VVREF_2P5_V,
} pmu_3vvref_voltage_t;

typedef enum {
    PMU_VPA_1p2V,
    PMU_VPA_1p4V,
    PMU_VPA_1p6V,
    PMU_VPA_1p8V,
    PMU_VPA_2p2V,
} pmu_power_vpa_voltage_t;

typedef enum {
    PMU_CLASSAB,
    PMU_CLASSG2,
    PMU_CLASSG3,
    PMU_CLASSD,
} pmu_audio_mode_t;

typedef enum {
    PMU_AUDIO_PINOUT_L,
    PMU_AUDIO_PINOUT_M,
    PMU_AUDIO_PINOUT_H,
} pmu_audio_pinout_t;

typedef enum {
    PMIC_MICBIAS_LDO0,
    PMIC_MICBIAS_LDO1,
    PMIC_MICBIAS_LDO2,
} pmu_micbias_ldo_t;

typedef enum {
    PMIC_MICBIAS0,
    PMIC_MICBIAS1,
    PMIC_MICBIAS2,
    PMIC_MICBIAS_0_1,
    PMIC_MICBIAS_ALL,
    PMIC_MICBIAS0_SHARE_ENABLE,
    PMIC_MICBIAS0_SHARE_1,
    PMIC_MICBIAS0_SHARE_2,
    PMIC_MICBIAS0_UNSHARE_1,
    PMIC_MICBIAS0_UNSHARE_2,
} pmu_micbias_index_t;

typedef enum {
    PMIC_MICBIAS_LP,
    PMIC_MICBIAS_NM,
    PMIC_MICBIAS_HPM,
} pmu_micbias_mode_t;

typedef enum {
    PMIC_WDT_REG,
    PMIC_WDT_WARM,
    PMIC_WDT_COLD,
} pmu_wdtrstb_act_t;

#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_S
typedef enum {
    PMU_AUX_PN_ZCV,
    PMU_AUX_WK_ZCV,
    PMU_AUX_BATSNS,
    PMU_AUX_BAT_RECHARGER,
    PMU_AUX_VBUS,
    PMU_AUX_VBUS_UART,
    PMU_AUX_CHR_THM,
    PMU_AUX_HW_JEITA,
    PMU_AUX_PMIC_AP,
    PMU_AUX_PMIC_AUTO,
    PMU_AUX_MAX,
} pmu_adc_channel_t;
#elif defined AIR_BTA_IC_PREMIUM_G3_TYPE_A
typedef enum {
    PMU_AUX_PN_ZCV,
    PMU_AUX_WK_ZCV,
    PMU_AUX_BATSNS,
    PMU_AUX_BAT_RECHARGER,
    PMU_AUX_VBUS,
    PMU_AUX_VBUS_UART,
    PMU_AUX_CHR_THM,
    PMU_AUX_HW_JEITA,
    PMU_AUX_PMIC_AP,
    PMU_AUX_PMIC_AUTO,
    PMU_AUX_MAX,
} pmu_adc_channel_t;
#elif defined AIR_BTA_IC_PREMIUM_G3_TYPE_P
typedef enum {
    PMU_AUX_VICHG,
    PMU_AUX_VBAT,
    PMU_AUX_VCHG,
    PMU_AUX_LDO_BUCK,
    PMU_AUX_VBAT_CALI,
    PMU_AUX_VIN,
} pmu_adc_channel_t;
#endif

enum {
    SDP_CHARGER = 1,
    CDP_CHARGER,
    DCP_CHARGER,
    SS_TABLET_CHARGER,
    IPAD2_IPAD4_CHARGER,
    IPHONE_5V_1A_CHARGER,
    NON_STD_CHARGER,
    DP_DM_FLOATING,
    UNABLE_TO_IDENTIFY_CHARGER,
    INVALID_CHARGER = 0xFF,
};

typedef enum {
    ICL_ITH_10mA = 0,     //  10   mA
    ICL_ITH_75mA = 1,     //  75   mA
    ICL_ITH_200mA = 2,    //  200  mA
    ICL_ITH_300mA = 3,    //  300  mA
    ICL_ITH_443mA = 4,    //  400  mA
    ICL_ITH_500mA = 5,    //  500  mA
    ICL_ITH_600mA = 6,    //  600  mA
    ICL_ITH_700mA = 7,    //  700  mA
    ICL_ITH_800mA = 8,    //  800  mA
    ICL_ITH_900mA = 9,    //  900  mA
    ICL_ITH_1000mA = 10,  //  1000 mA
} pmu_icl_level_t;

typedef enum {
    RG_ICC_JC_20    = 0,
    RG_ICC_JC_40    = 4,
    RG_ICC_JC_60    = 5,
    RG_ICC_JC_80    = 6,
    RG_ICC_JC_100   = 7,
} pmu_jc_perecnt_level_t;

typedef enum {
    RG_VCV_VOLTAGE_4P05V  = 0,    //4.05V
    RG_VCV_VOLTAGE_4P075V = 1,    //4.05V
    RG_VCV_VOLTAGE_4P10V  = 2,    //4.10V
    RG_VCV_VOLTAGE_4P125V = 3,    //4.10V
    RG_VCV_VOLTAGE_4P15V  = 4,    //4.15V
    RG_VCV_VOLTAGE_4P175V = 5,    //4.15V
    RG_VCV_VOLTAGE_4P20V  = 6,    //4.20V
    RG_VCV_VOLTAGE_4P225V = 7,    //4.20V
    RG_VCV_VOLTAGE_4P25V  = 8,    //4.25V
    RG_VCV_VOLTAGE_4P725V = 9,    //4.20V
    RG_VCV_VOLTAGE_4P30V  = 10,   //4.30V
    RG_VCV_VOLTAGE_4P325V = 11,   //4.20V
    RG_VCV_VOLTAGE_4P35V  = 12,   //4.35V
    RG_VCV_VOLTAGE_4P375V = 13,   //4.20V
    RG_VCV_VOLTAGE_4P40V  = 14,   //4.40V
    RG_VCV_VOLTAGE_4P425V = 15,   //4.20V
    RG_VCV_VOLTAGE_4P45V  = 16,   //4.45V
    RG_VCV_VOLTAGE_4P475V = 17,   //4.20V
    RG_VCV_VOLTAGE_4P50V  = 19,   //4.50V
    RG_VCV_VOLTAGE_4P525V = 20,   //4.20V
    RG_VCV_VOLTAGE_4P55V  = 21,   //4.55V
    RG_VCV_VOLTAGE_4P575V = 22,   //4.20V
    RG_VCV_VOLTAGE_4P60V  = 23,   //4.60V
} pmu_cv_voltage_t;

typedef enum {
    pmu_fastcc_chrcur_0P5mA = 0,
    pmu_fastcc_chrcur_1mA,
    pmu_fastcc_chrcur_1P5mA,
    pmu_fastcc_chrcur_2mA,
    pmu_fastcc_chrcur_2P5mA,
    pmu_fastcc_chrcur_3mA,
    pmu_fastcc_chrcur_3P5mA,
    pmu_fastcc_chrcur_4mA,
    pmu_fastcc_chrcur_4P5mA,
    pmu_fastcc_chrcur_5mA,
    pmu_fastcc_chrcur_10mA,
    pmu_fastcc_chrcur_15mA,
    pmu_fastcc_chrcur_20mA,
    pmu_fastcc_chrcur_25mA,
    pmu_fastcc_chrcur_30mA,
    pmu_fastcc_chrcur_35mA,
    pmu_fastcc_chrcur_40mA,
    pmu_fastcc_chrcur_45mA,
    pmu_fastcc_chrcur_50mA,
    pmu_fastcc_chrcur_55mA,
    pmu_fastcc_chrcur_60mA,
    pmu_fastcc_chrcur_65mA,
    pmu_fastcc_chrcur_70mA,
    pmu_fastcc_chrcur_75mA,
    pmu_fastcc_chrcur_80mA,
    pmu_fastcc_chrcur_85mA,
    pmu_fastcc_chrcur_90mA,
    pmu_fastcc_chrcur_95mA,
    pmu_fastcc_chrcur_100mA,
    pmu_fastcc_chrcur_105mA,
    pmu_fastcc_chrcur_110mA,
    pmu_fastcc_chrcur_115mA,
    pmu_fastcc_chrcur_120mA,
    pmu_fastcc_chrcur_125mA,
    pmu_fastcc_chrcur_130mA,
    pmu_fastcc_chrcur_135mA,
    pmu_fastcc_chrcur_140mA,
    pmu_fastcc_chrcur_145mA,
    pmu_fastcc_chrcur_150mA,
    pmu_fastcc_chrcur_155mA,
    pmu_fastcc_chrcur_160mA,
    pmu_fastcc_chrcur_165mA,
    pmu_fastcc_chrcur_170mA,
    pmu_fastcc_chrcur_175mA,
    pmu_fastcc_chrcur_180mA,
    pmu_fastcc_chrcur_185mA,
    pmu_fastcc_chrcur_190mA,
    pmu_fastcc_chrcur_195mA,
    pmu_fastcc_chrcur_200mA,
    pmu_fastcc_chrcur_205mA,
    pmu_fastcc_chrcur_210mA,
    pmu_fastcc_chrcur_215mA,
    pmu_fastcc_chrcur_220mA,
    pmu_fastcc_chrcur_225mA,
    pmu_fastcc_chrcur_230mA,
    pmu_fastcc_chrcur_235mA,
    pmu_fastcc_chrcur_240mA,
    pmu_fastcc_chrcur_245mA,
    pmu_fastcc_chrcur_250mA,
    pmu_fastcc_chrcur_255mA,
    pmu_fastcc_chrcur_260mA,
    pmu_fastcc_chrcur_265mA,
    pmu_fastcc_chrcur_270mA,
    pmu_fastcc_chrcur_275mA,
    pmu_fastcc_chrcur_280mA,
    pmu_fastcc_chrcur_285mA,
    pmu_fastcc_chrcur_290mA,
    pmu_fastcc_chrcur_295mA,
    pmu_fastcc_chrcur_300mA,
    pmu_fastcc_chrcur_305mA,
    pmu_fastcc_chrcur_310mA,
    pmu_fastcc_chrcur_315mA,
    pmu_fastcc_chrcur_320mA,
    pmu_fastcc_chrcur_325mA,
    pmu_fastcc_chrcur_330mA,
    pmu_fastcc_chrcur_335mA,
    pmu_fastcc_chrcur_340mA,
    pmu_fastcc_chrcur_345mA,
    pmu_fastcc_chrcur_350mA,
    pmu_fastcc_chrcur_355mA,
    pmu_fastcc_chrcur_360mA,
    pmu_fastcc_chrcur_365mA,
    pmu_fastcc_chrcur_370mA,
    pmu_fastcc_chrcur_375mA,
    pmu_fastcc_chrcur_380mA,
    pmu_fastcc_chrcur_385mA,
    pmu_fastcc_chrcur_390mA,
    pmu_fastcc_chrcur_395mA,
    pmu_fastcc_chrcur_400mA,
    pmu_fastcc_chrcur_405mA,
    pmu_fastcc_chrcur_410mA,
    pmu_fastcc_chrcur_415mA,
    pmu_fastcc_chrcur_420mA,
    pmu_fastcc_chrcur_425mA,
    pmu_fastcc_chrcur_430mA,
    pmu_fastcc_chrcur_435mA,
    pmu_fastcc_chrcur_440mA,
    pmu_fastcc_chrcur_445mA,
    pmu_fastcc_chrcur_450mA,
    pmu_fastcc_chrcur_455mA,
    pmu_fastcc_chrcur_460mA,
    pmu_fastcc_chrcur_465mA,
    pmu_fastcc_chrcur_470mA,
    pmu_fastcc_chrcur_475mA,
    pmu_fastcc_chrcur_480mA,
    pmu_fastcc_chrcur_485mA,
    pmu_fastcc_chrcur_490mA,
    pmu_fastcc_chrcur_495mA,
    pmu_fastcc_chrcur_500mA,
    pmu_fastcc_chrcur_505mA,
    pmu_fastcc_chrcur_510mA,
    pmu_fastcc_chrcur_515mA,
    pmu_fastcc_chrcur_520mA,
    pmu_fastcc_chrcur_525mA,
    pmu_fastcc_chrcur_530mA,
    pmu_fastcc_chrcur_535mA,
    pmu_fastcc_chrcur_540mA,
    pmu_fastcc_chrcur_545mA,
    pmu_fastcc_chrcur_550mA,
    pmu_fastcc_chrcur_555mA,
    pmu_fastcc_chrcur_560mA,
    pmu_fastcc_chrcur_565mA,
    pmu_fastcc_chrcur_570mA,
    pmu_fastcc_chrcur_575mA,
    pmu_fastcc_chrcur_580mA,
    pmu_fastcc_chrcur_585mA,
    pmu_fastcc_chrcur_590mA,
    pmu_fastcc_chrcur_595mA,
    pmu_fastcc_chrcur_600mA,
    pmu_fastcc_chrcur_605mA,
    pmu_fastcc_chrcur_610mA,
    pmu_fastcc_chrcur_615mA,
    pmu_fastcc_chrcur_620mA,
    pmu_fastcc_chrcur_625mA,
    pmu_fastcc_chrcur_630mA,
    pmu_fastcc_chrcur_635mA,
    pmu_fastcc_chrcur_640mA,
} pmu_fastcc_chrcur_t;

typedef enum {
    pmu_iterm_chrcur_0P5mA = 0,
    pmu_iterm_chrcur_1mA,
    pmu_iterm_chrcur_1P5mA,
    pmu_iterm_chrcur_2mA,
    pmu_iterm_chrcur_2P5mA,
    pmu_iterm_chrcur_3mA,
    pmu_iterm_chrcur_3P5mA,
    pmu_iterm_chrcur_4mA,
    pmu_iterm_chrcur_4P5mA,
    pmu_iterm_chrcur_5mA,
    pmu_iterm_chrcur_5P5mA,
    pmu_iterm_chrcur_6mA,
    pmu_iterm_chrcur_6P5mA,
    pmu_iterm_chrcur_7mA,
    pmu_iterm_chrcur_7P5mA,
    pmu_iterm_chrcur_8mA,
    pmu_iterm_chrcur_9mA,
    pmu_iterm_chrcur_10mA,
    pmu_iterm_chrcur_11mA,
    pmu_iterm_chrcur_12mA,
    pmu_iterm_chrcur_13mA,
    pmu_iterm_chrcur_14mA,
    pmu_iterm_chrcur_15mA,
    pmu_iterm_chrcur_16mA,
    pmu_iterm_chrcur_17mA,
    pmu_iterm_chrcur_18mA,
    pmu_iterm_chrcur_19mA,
    pmu_iterm_chrcur_20mA,
    pmu_iterm_chrcur_21mA,
    pmu_iterm_chrcur_22mA,
    pmu_iterm_chrcur_23mA,
    pmu_iterm_chrcur_24mA,
    pmu_iterm_chrcur_25mA,
    pmu_iterm_chrcur_26mA,
    pmu_iterm_chrcur_27mA,
    pmu_iterm_chrcur_28mA,
    pmu_iterm_chrcur_29mA,
    pmu_iterm_chrcur_30mA,
    pmu_iterm_chrcur_31mA,
    pmu_iterm_chrcur_32mA,
    pmu_iterm_chrcur_34mA,
    pmu_iterm_chrcur_36mA,
    pmu_iterm_chrcur_38mA,
    pmu_iterm_chrcur_40mA,
    pmu_iterm_chrcur_42mA,
    pmu_iterm_chrcur_44mA,
    pmu_iterm_chrcur_46mA,
    pmu_iterm_chrcur_48mA,
    pmu_iterm_chrcur_50mA,
    pmu_iterm_chrcur_52mA,
    pmu_iterm_chrcur_54mA,
    pmu_iterm_chrcur_56mA,
    pmu_iterm_chrcur_58mA,
    pmu_iterm_chrcur_60mA,
    pmu_iterm_chrcur_62mA,
    pmu_iterm_chrcur_64mA,
} pmu_iterm_chrcur_t;
typedef enum {
    HAL_BATTERY_VOLT_03_5000_V = 35000,         /**< define voltage as  3500 mV */
    HAL_BATTERY_VOLT_03_6000_V = 36000,         /**< define voltage as  3600 mV */
    HAL_BATTERY_VOLT_03_7000_V = 37000,         /**< define voltage as  3700 mV */
    HAL_BATTERY_VOLT_03_7750_V = 37750,         /**< define voltage as  3775 mV */
    HAL_BATTERY_VOLT_03_8000_V = 38000,         /**< define voltage as  3800 mV */
    HAL_BATTERY_VOLT_03_8500_V = 38500,         /**< define voltage as  3850 mV */
    HAL_BATTERY_VOLT_03_9000_V = 39000,         /**< define voltage as  3900 mV */
    HAL_BATTERY_VOLT_04_0000_V = 40000,         /**< define voltage as  4000 mV */
    HAL_BATTERY_VOLT_04_0500_V = 40500,         /**< define voltage as  4050 mV */
    HAL_BATTERY_VOLT_04_1000_V = 41000,         /**< define voltage as  4100 mV */
    HAL_BATTERY_VOLT_04_1250_V = 41250,         /**< define voltage as  4125 mV */
    HAL_BATTERY_VOLT_04_1375_V = 41375,         /**< define voltage as  4137.5 mV */
    HAL_BATTERY_VOLT_04_1500_V = 41500,         /**< define voltage as  4150 mV */
    HAL_BATTERY_VOLT_04_1625_V = 41625,         /**< define voltage as  4162.5 mV */
    HAL_BATTERY_VOLT_04_1750_V = 41750,         /**< define voltage as  4175 mV */
    HAL_BATTERY_VOLT_04_1875_V = 41875,         /**< define voltage as  4187.5 mV */
    HAL_BATTERY_VOLT_04_2000_V = 42000,         /**< define voltage as  4200 mV */
    HAL_BATTERY_VOLT_04_2125_V = 42125,         /**< define voltage as  4212 mV */
    HAL_BATTERY_VOLT_04_2250_V = 42250,         /**< define voltage as  4225 mV */
    HAL_BATTERY_VOLT_04_2375_V = 42375,         /**< define voltage as  4237.5 mV */
    HAL_BATTERY_VOLT_04_2500_V = 42500,         /**< define voltage as  4250 mV */
    HAL_BATTERY_VOLT_04_2625_V = 42625,         /**< define voltage as  4262.5 mV */
    HAL_BATTERY_VOLT_04_2750_V = 42750,         /**< define voltage as  4275 mV */
    HAL_BATTERY_VOLT_04_2875_V = 42875,         /**< define voltage as  4287.5 mV */
    HAL_BATTERY_VOLT_04_3000_V = 43000,         /**< define voltage as  4300 mV */
    HAL_BATTERY_VOLT_04_3125_V = 43125,         /**< define voltage as  4312.5 mV */
    HAL_BATTERY_VOLT_04_3250_V = 43250,         /**< define voltage as  4325 mV */
    HAL_BATTERY_VOLT_04_3375_V = 43375,         /**< define voltage as  4337.5 mV */
    HAL_BATTERY_VOLT_04_3500_V = 43500,         /**< define voltage as  4350 mV */
    HAL_BATTERY_VOLT_04_3625_V = 43625,         /**< define voltage as  4362.5 mV */
    HAL_BATTERY_VOLT_04_3750_V = 43750,         /**< define voltage as  4375 mV */
    HAL_BATTERY_VOLT_04_3875_V = 43875,         /**< define voltage as  4387.5 mV */
    HAL_BATTERY_VOLT_04_4000_V = 44000,         /**< define voltage as  4400 mV */
    HAL_BATTERY_VOLT_04_4125_V = 44125,         /**< define voltage as  4412.5 mV */
    HAL_BATTERY_VOLT_04_4375_V = 44375,         /**< define voltage as  4437.5 mV */
    HAL_BATTERY_VOLT_04_4250_V = 44250,         /**< define voltage as  4425 mV */
    HAL_BATTERY_VOLT_04_4500_V = 44500,         /**< define voltage as  4450 mV */
    HAL_BATTERY_VOLT_04_4625_V = 44625,         /**< define voltage as  4462.5 mV */
    HAL_BATTERY_VOLT_04_4750_V = 44750,         /**< define voltage as  4475 mV */
    HAL_BATTERY_VOLT_04_4875_V = 44875,         /**< define voltage as  4487.5 mV */
    HAL_BATTERY_VOLT_04_5000_V = 45000,         /**< define voltage as  4500 mV */
    HAL_BATTERY_VOLT_04_5500_V = 45500,         /**< define voltage as  4550 mV */
    HAL_BATTERY_VOLT_04_6000_V = 46000,         /**< define voltage as  4600 mV */
    HAL_BATTERY_VOLT_06_0000_V = 60000,         /**< define voltage as  6000 mV */
    HAL_BATTERY_VOLT_06_5000_V = 65000,         /**< define voltage as  6500 mV */
    HAL_BATTERY_VOLT_07_0000_V = 70000,         /**< define voltage as  7000 mV */
    HAL_BATTERY_VOLT_07_5000_V = 75000,         /**< define voltage as  7500 mV */
    HAL_BATTERY_VOLT_08_5000_V = 85000,         /**< define voltage as  8500 mV */
    HAL_BATTERY_VOLT_09_5000_V = 95000,         /**< define voltage as  9500 mV */
    HAL_BATTERY_VOLT_10_5000_V = 105000,        /**< define voltage as 10500 mV */
    HAL_BATTERY_VOLT_MAX,
    HAL_BATTERY_VOLT_INVALID
} HAL_BATTERY_VOLTAGE_ENUM;
typedef enum {
    option_setting = 0,
    option4_init = 8,
    option4_exit = 9,
} pmu_eoc_operating_t;

typedef enum {
    ICL_TSETP_0US   = 0,    //  0us
    ICL_TSETP_16US  = 1,    //  16us (default)
    ICL_TSETP_128uS = 2,    //  128us
    ICL_TSETP_256US = 3,    //  256us
} pmu_icl_tstep_t;

typedef enum {
    RG_ICC_20P    = 0,
    RG_ICC_40P    = 4,
    RG_ICC_60P    = 5,
    RG_ICC_80P    = 6,
    RG_ICC_100P   = 7,
} pmu_jeita_perecnt_level_t;
typedef enum {
    TST_W_KEY_SW_MODE  = 0,
    TST_W_KEY_HW_MODE  = 1,
} pmu_protect_mode_t;

typedef enum {
    NORMAL = 0,
    PMU,
    RTC,
    CAP,
} pmu_debug_mode_t;
typedef enum {
    pmu_eoc_option1 = 1,
    pmu_eoc_option4 = 4,
} pmu_eoc_option_t;
typedef void (*pmu_callback_t)(void);

typedef struct {
    void (*pmu_callback)(void);
    void *user_data;
    bool init_status;
    bool isMask;
} pmu_function_t;

typedef struct {
    pmu_callback_t callback1; //press callback
    void *user_data1;
    pmu_callback_t callback2; //release callback
    void *user_data2;
} pmu_pwrkey_config_t;

typedef enum {
    PMU_DEBOUNCE_PWRKEY,
    PMU_RELEASE_PWRKEY,
    PMU_REPRESS_PWRKEY,
    PMU_RESET_DEFAULT,
} pmu_lpsd_scenario_t;

typedef enum {
    PMU_DURATION_8S,
    PMU_DURATION_10S,
    PMU_DURATION_15S,
    PMU_DURATION_20S,
} pmu_lpsd_time_t;
/*=====APMIC LP==*/
typedef enum {
    RG_ADC_SW_TRIG_FLAG,
    RG_ADC_HW_TRIG_FLAG,
    RG_CHG_IN_INT_FLAG,
    RG_CHG_OUT_INT_FLAG,
    RG_CHG_PRE_COMPLETE_INT_FLAG,
    RG_CHG_COMPLETE_INT_FLAG,
    RG_CHG_RECHG_INT_FLAG,
    RG_REGEN_IRQ_RISE_FLAG,
    RG_REGEN_IRQ_FALL_FLAG,
    PMU_INT_MAX_LP,
} pmu_interrupt_index_lp_t;

typedef enum {
    PMU_LOW_LEVEL,
    PMU_NORMAL_LEVEL,
    PMU_HIGH_LEVEL,
} pmu_slt_mode_t;

typedef enum {
    PMU_PRECC,
    PMU_FASTCC,
} pmu_safety_timer_t;

typedef enum {
    PMU_RIPPLE_OFF     = 0,
    PMU_RIPPLE_ON      = 1,
} pmu_ripple_operate_t;

typedef enum {
    PMU_RSTPAT_SRC_VBUS         = 0,
    PMU_RSTPAT_SRC_VBUS_UART    = 1,
} pmu_rstpat_src_t;

typedef struct {
    uint16_t vcore_val;
    uint16_t vio18_val;
    uint16_t vaud18_val;
    uint16_t vrf_val;
    uint32_t vpa_val;
} pmu_dummy_load_str;

/***************************** 1wire *******************************/
typedef enum {
    PMU_1WIRE_LOW_BAT,
    PMU_1WIRE_NORM_BAT,
} pmu_1wire_bat_t;

typedef enum {
    PMU_1WIRE_INIT_TO_OUT_OF_CASE_LOW_BAT,
    PMU_1WIRE_INIT_TO_OUT_OF_CASE_NORM_BAT,
    PMU_1WIRE_INIT_TO_CHG_IN,
    PMU_1WIRE_OUT_OF_CASE_TO_CHG_IN_NORM_BAT,
    PMU_1WIRE_OUT_OF_CASE_TO_CHG_IN_LOW_BAT,
    PMU_1WIRE_COM_TO_OUT_OF_CASE,
    PMU_1WIRE_CHG_IN_TO_COM_NORM_BAT,
    PMU_1WIRE_CHG_IN_TO_COM_LOW_BAT,
    PMU_1WIRE_COM_TO_CHG_IN_NORM_BAT,
    PMU_1WIRE_COM_TO_CHG_IN_LOW_BAT,
    PMU_1WIRE_COM_TO_PWR_SAVE,
    PMU_1WIRE_ENTER_TO_LOG_NORM_BAT,
    PMU_1WIRE_ENTER_TO_LOG_LOW_BAT,
    PMU_1WIRE_COM_TO_COM_END,
    PMU_1WIRE_MAX,
} pmu_1wire_operate_t;

void pmu_1wire_cfg(pmu_1wire_operate_t state);
uint32_t pmu_1wire_get_vbus_uart_volt(void);

//#define PMU_BOOTLOADER_ENABLE_CHARGER
#ifdef PMU_BOOTLOADER_ENABLE_CHARGER
#define PMU_BL_PRECC pmu_fastcc_chrcur_5mA
#define PMU_BL_CC pmu_fastcc_chrcur_50mA
#define PMU_BL_CV 6
#endif

/* Dynamic Debug Print Flags */
typedef union {
    struct {
        uint8_t vcore    :1;
        uint8_t other    :1;
        uint8_t reserved :6;
    } b;
    uint8_t val;
} pmu_dynamic_debug_t;

extern pmu_dynamic_debug_t pmu_dynamic_debug;

/*==========[Basic function]==========*/
//#define PMU_SLT_ENV
void pmu_init(void);
void pmu_config(void);
/*-------------------------------------------[Common PMU]----------------------------------------------------*/
void pmu_select_vsram_vosel(pmu_power_stage_t mode, pmu_vsram_voltage_t val);
void pmu_latch_power_key_for_bootloader(void);

/**
 * @brief Get the version of pmu.
 * @return
 *   0    - Can't read nvkey.
 *   0xFF - OTP is not written, only heppened in LP.
 *   1 for E1, 2 for E2, 3 for E3, and others...
 */
uint8_t pmu_get_pmic_version(void);
uint8_t pmu_get_power_on_reason(void);
uint8_t pmu_get_power_off_reason(void);
void pmu_set_rstpat(pmu_power_operate_t oper, pmu_rstpat_src_t src);
pmu_operate_status_t pmu_pwrkey_normal_key_init(pmu_pwrkey_config_t *config);
bool pmu_get_pwrkey_state(void);
void pmu_enable_captouch(pmu_power_operate_t oper);
void pmu_enable_lpsd(pmu_lpsd_time_t tmr, pmu_power_operate_t oper);
void pmu_enable_micbias(pmu_micbias_ldo_t ldo, pmu_micbias_index_t index, pmu_micbias_mode_t mode, pmu_power_operate_t operate);
void pmu_set_micbias_ldo_mode(pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode);
pmu_operate_status_t pmu_set_micbias_vout(pmu_micbias_index_t index, pmu_3vvref_voltage_t vol);
#ifndef AIR_BTA_IC_PREMIUM_G3_TYPE_D
uint32_t pmu_auxadc_get_channel_value(pmu_adc_channel_t Channel);
#endif
void pmu_set_dummy_load(pmu_power_domain_t domain, uint32_t loading_value);
/*-------------------------------------------[D-die PMU]----------------------------------------------------*/
pmu_operate_status_t pmu_set_register_value_ddie(uint32_t address, uint32_t mask, uint32_t shift, uint32_t value);
uint32_t pmu_get_register_value_ddie(uint32_t address, uint32_t mask, uint32_t shift);
uint8_t pmu_enable_usb_power(pmu_power_operate_t oper);

/*-------------------------------------------[A-die PMU]----------------------------------------------------*/
pmu_operate_status_t pmu_set_register_value(uint32_t address, uint32_t mask, uint32_t shift, uint32_t value);
uint32_t pmu_get_register_value(uint32_t address, uint32_t mask, uint32_t shift);
pmu_power_vcore_voltage_t pmu_lock_vcore(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol, pmu_lock_parameter_t lock);
void pmu_enable_power(pmu_power_domain_t pmu_pdm, pmu_power_operate_t operate);
pmu_operate_status_t pmu_set_vaud18_vout(pmu_vaud18_voltage_t lv, pmu_vaud18_voltage_t mv, pmu_vaud18_voltage_t hv);
void pmu_set_vpa_voltage(pmu_power_vpa_voltage_t oper);
void pmu_set_pre_charger_current(pmu_fastcc_chrcur_t cur);
void pmu_set_audio_mode(pmu_audio_mode_t mode, pmu_power_operate_t operate);
int32_t pmu_auxadc_get_pmic_temperature(void);
bool pmu_get_chr_detect_value(void);
void pmu_set_cap_wo_vbus(pmu_power_operate_t oper);
void pmu_power_off_sequence(pmu_power_stage_t stage);
uint8_t pmu_get_power_status(pmu_power_domain_t pmu_pdm);
void pmu_set_vsram_voltage(pmu_vsram_voltage_t val);
uint32_t pmu_get_charger_state(void);
void pmu_select_eoc_option_operating(pmu_eoc_option_t opt, pmu_eoc_operating_t oper);
void pmu_set_icl_by_type(uint8_t port);
void pmu_set_charger_current(pmu_fastcc_chrcur_t cur);
void pmu_set_iterm_current_irq(pmu_iterm_chrcur_t cur);
bool pmu_set_rechg_voltage(uint8_t rechargeVoltage);
void pmu_charger_init(uint16_t precc_cur, uint16_t cv_termination);
bool pmu_set_hw_jeita_enable(uint8_t value);
void pmu_charger_check_faston(void);
pmu_operate_status_t pmu_set_jeita_voltage(uint32_t auxadcVolt, uint8_t JeitaThreshold);
void pmu_lock_va18(int oper);
void pmu_set_charger_current_limit(pmu_icl_level_t icl_value);
bool pmu_select_cv_voltage(uint8_t voltage);
bool pmu_enable_recharger(bool isEnableRecharge);
uint8_t pmu_get_hw_jeita_status(void);
void pmu_hw_jeita_init(void);
bool pmu_set_extend_charger_time(uint8_t timeMins);
uint8_t pmu_enable_charger(uint8_t isEnableCharging);
uint8_t pmu_get_usb_input_status(void);
void pmu_set_jeita_state_setting(uint8_t state, pmu_jeita_perecnt_level_t ICC_JC, pmu_cv_voltage_t vol);
void pmu_select_wdt_mode(pmu_wdtrstb_act_t sel);
void pmu_dump_otp(void);
void pmu_dump_nvkey(void);
void pmu_dump_rg(void);
void pmu_vrf_keep_nm(void);
void pmu_check_rg_timer(uint16_t start, uint16_t end, uint16_t timer);
void hal_pmu_buck_thd_test(void);
void hal_pmu_buck_thd_test_wo_pg(void);
#endif /* HAL_PMU_MODULE_ENABLED */
#endif
