/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

/*==========[Battery Charger]==========*/
#define BATTERY_WO_BC12_ICL ICL_ITH_443mA
#define BATTERY_PRECC_TIMER 0
#define BATTERY_FASTCC_TIMER 1
#define BATTERY_EXTERNAL_TIMER 0
//#define AIR_BATTERY_EOC_ENTER_RTC
//#define EXTERNAL_CHARGER
/*2CC feature*/
#define BATTERY_S0_VOLTAGE 4     /*index : 4 means 3000V ; 000: 2.40V ;001: 2.55V;010: 2.70V;011: 2.85V;100: 3.00V (default);101: 3.15V;110: 3.30V;111: 3.45V */
#define BATTERY_S0_CHR_CUR pmu_fastcc_chrcur_100mA
#define BATTERY_S1_VOLTAGE 3700
#define BATTERY_S1_CHR_CUR pmu_fastcc_chrcur_100mA
#define BATTERY_S2_VOLTAGE 3900
#define BATTERY_S2_CHR_CUR pmu_fastcc_chrcur_100mA
/*2CC feature end*/
#define BATTERY_PRECC_CURRENT pmu_fastcc_chrcur_5mA
#define BATTERY_ITERM_ITH pmu_iterm_chrcur_5mA
#define BATTERY_ITERM_CURRENT_IRQ pmu_iterm_chrcur_50mA
/*2CV Feature*/
#define FULL_BATTERY_CV1 4200 //mA
#define CV1_ITERM_IRQ pmu_iterm_chrcur_50mA
#define FULL_BATTERY_CV2 4400 //mA
#define CV2_ITERM_IRQ pmu_iterm_chrcur_5mA
/*2CV Feature end*/
/*==========[Battery parameter]==========*/
#define BATTERY_CATEGORY 1
#define INITIAL_BAT 3000 //mA
#define SW_SHUT_DOWN 3300 //mA
#define FULL_BATTERY 4200 //mA (CV)
#define EOC_RECHARGER_VOLTAGE 4100 //mA
#define BATTERY_CAPACITY_OFFSET 50 //mA
#define MAX_BATTERY_VOLTAGE_OVERLAY 5500
#define EXTEND_TIME ITERM_TIME_SETTING_DISABLE
#define BATTERY_VOLTAGE_REFERENCE_POINTS 9
#define RECHARGER_VOLTAGE RECHARGER_MARK_100
#define BATTERY_VBUS_DEBOUNCE_TIME 0 /*00:147(default) ; 1:18.9ms; 2:35ms*/
/*==========[Charger parameter]==========*/
#define CHARGER_REGULAR_TIME 120 // second
#ifdef AIR_SMART_CHARGER_ENABLE
#define BATTERY_MANAGER_DEFAULT_CHARGER_OPTION 1 /*If smart charger case enable need choose option 1*/
#else
#ifdef AIR_BATTERY_EOC_ENTER_RTC
#define BATTERY_MANAGER_DEFAULT_CHARGER_OPTION 4
#else
#define BATTERY_MANAGER_DEFAULT_CHARGER_OPTION 1
#endif
#endif
#define MTK_BATTERY_ULTRA_LOW_BAT 1 /*Set the lowest showed SOC*/
/*==========[Charger parameter : JEITA]==========*/
#define HW_JEITA_HOT_THRESHOLD  593   /*45 celsius */
#define HW_JEITA_WARM_THRESHOLD 635   /*42 celsius */
#define HW_JEITA_COOL_THRESHOLD 1270  /* 3 celsius */
#define HW_JEITA_COLD_THRESHOLD 1316  /* 0 celsius */
#define BATTERY_COOL_CC RG_ICC_JC_20
#define BATTERY_COOL_CV 4200
#define BATTERY_WARM_CC RG_ICC_JC_100
#define BATTERY_WARM_CV 4050
/*==========[Feature Option]==========*/
#define BATTERY_FEATURE_2CC BATTERY_OPERATE_ON
#define BATTERY_FEATURE_2CV BATTERY_OPERATE_OFF
#ifdef AIR_SMART_CHARGER_ENABLE
#define BATTERY_FEATURE_BC12 BATTERY_OPERATE_OFF  /*This is for BC1.2 feature*/
#else
#define BATTERY_FEATURE_BC12 BATTERY_OPERATE_OFF  /*This is for BC1.2 feature*/
#endif
#define BATTERY_FEATURE_JEITA BATTERY_OPERATE_ON
#define BATTERY_FEATURE_JEITA_WARM_COOL BATTERY_OPERATE_ON /*Disable HW-JEITA WARM and COOOL stage detect.*/
#define BATTERY_FEATURE_PREECC_TIMER BATTERY_OPERATE_OFF
#define BATTERY_FEATURE_FASTCC_TIMER BATTERY_OPERATE_OFF
#define BATTERY_FEATURE_POWERHOLD BATTERY_OPERATE_ON

/*==========[Feature 1]==========*/
/* The first times in EOC state ,
 * It will be waits for one minute to execute EOC behavior.
 * BATTERY_EOC_CHECK_TIME unit is second.
 * EOC_CHECK_ON set BATTERY_OPERATE_ON is feature on.
 * */
#define BATTERY_EOC_CHECK_TIME 60
#define EOC_CHECK_ON BATTERY_OPERATE_ON
/*==========[Feature 2]==========*/
/*
 * It will check battery voltage when boot up
 * When the battery voltage is lower than the set voltage,
 * it will enter RTC mode
 * */
#define BATTERY_CHECK_BATTERY_VOLTAGE

/*==========[Feature 3]==========*/
/*
 *PMIC_AWAKE_TO_CHARGER ,This feature is for low power charger.
 *When the feature on, CM4 can enter sleep in charging*/
//#define PMIC_AWAKE_TO_CHARGER

/*==========[Feature 4]==========*/
/* Avoid unexpected problems caused by irq shake,
 * Confirm HW status after irq specified time, and send notification if there is any problem*/
#define CHARGER_CALIBRATION
#ifdef CHARGER_CALIBRATION
#define CALIBRATION_TIME 1000 //Millisecond
#endif

/*==========[Feature 5]==========*/
/*Avoid VBUS shaking, ensure the Irq data is consistent this time*/
#define BATTERY_AVOID_SHAKING

/*==========[Feature 6]==========*/
/*When Use external charger IC ,Disable internal chager ,need to enable*/
//#define BATTERY_DISABLE_CHARGER

/*==========[Feature 7]==========*/
/* Customized settings for customers to use
 * Used in init setting.
 * Default off , this api just for test and development*/
//#define BATTERY_CUSTOMER_SETTING
//#define MTK_BATTERY_CHARGER_STEP

/*==========[Feature 8]==========*/
/*Fuel gauge used,when xx�XC used deputy table
 * */
//#define FUEL_GAUGE_DEPUTY_TABLE_ENABLE
#ifdef FUEL_GAUGE_DEPUTY_TABLE_ENABLE
#define FUEL_GAUGE_DEPUTY_TEMP_H 15
#define FUEL_GAUGE_DEPUTY_TEMP_L 13
#endif
/*==========[Battery parameter: linear gauge calculation]==========*/
#define GAUGE_TOLERANCE_PERCENT 95
#define CHARGER_TOLERANCE_TIME 13 //minute
/*==========[Battery parameter: linear gauge]==========*/
#define BATTERY_LINEAR_GAUGE
#define BATTERY_LINEAR_SMOOTH_SOC
#define BATTERY_AVOID_EVBUS_RAISE_VBAT_VOL
#define LINEAR_GAUGE_SIZE 18 /*Define linear gauge array size*/
#define LINEAR_REGULAR_GAUGE_TIME 10000
#define BATTERY_STABLE_CHARGING_VOLTAGE 90
#define BATTERY_REMOVE_IR_SOC_MARGIN 3
//#define BATTERY_DECREASE_RESISTANCE
#ifdef BATTERY_CUSTOMER_SETTING
typedef struct {
    uint32_t addr;
    uint32_t value;
} battery_customer_setting_t;
#endif
