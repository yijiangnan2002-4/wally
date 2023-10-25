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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "timers.h"

#include "hal_rtc.h"
#include "hal_gpt.h"
#include "hal_sleep_manager.h"


#include "battery_management_core.h"
#ifdef MTK_FUEL_GAUGE
#include "fuelgauge_interface.h"
#include "battery_common.h"
#include "cust_battery_meter_table.h"
#include "cust_battery_meter_table_bat2.h"
#include "cust_battery_meter_table_deputy.h"
#include "fuelgauge_interface.h"
#endif

/*************************************
*           global variable
**************************************/
extern uint8_t battery_init_setting_flag;
extern battery_basic_data bm_cust;                          /*Data : restore battery basic data*/

static int32_t g_I_SENSE_offset = 0;

static int32_t gFG_15_vlot = 3700;
static int32_t gFG_voltage_init = 0;
static int32_t gFG_current_init = 0;
static bool gFG_Is_Charging_init = false;
static int32_t g_sw_vbat_temp = 0;

uint32_t sleep_total_time = NORMAL_WAKEUP_PERIOD;

//static int32_t gFG_plugout_status = 0;
static int32_t g_tracking_point = CUST_TRACKING_POINT;
static int32_t gFG_voltage = 0;
static int32_t gFG_hwocv = 0;
static int32_t gFG_capacity_by_c = -1;


/* SW FG */
static int32_t oam_v_ocv;
static int32_t oam_r;
static int32_t swfg_ap_suspend_time = 0;
static int32_t ap_suspend_car = 0;

static int32_t is_hwocv_update = false;
static int32_t shutdown_system_voltage = SW_SHUT_DOWN;
//static int32_t shutdown_system_voltage = bm_cust.shutdown_bat;
static int32_t charge_tracking_time = CHARGE_TRACKING_TIME;
static int32_t discharge_tracking_time = DISCHARGE_TRACKING_TIME;

uint32_t wake_up_smooth_time = 0; /* sec */
bool g_suspend_timeout = false;
uint32_t ap_suspend_time;
uint32_t battery_suspend_time;
uint32_t battery_thread_time;
#define ZCV_ENABLE 1

struct battery_meter_custom_data batt_meter_cust_data;
struct battery_meter_table_custom_data batt_meter_table_cust_data;
extern struct battery_custom_data batt_cust_data;

extern PMU_ChargerStruct BMT_status;
HAL_BATTERY_VOLTAGE_ENUM g_cv_voltage = HAL_BATTERY_VOLT_04_2000_V;
extern bool g_battery_soc_ready;

int32_t battery_meter_init_custom_meter_data_bat2(void)
{
    batt_meter_table_cust_data.battery_profile_t0_size = sizeof(bat2_battery_profile_t0) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t0, &bat2_battery_profile_t0, sizeof(bat2_battery_profile_t0));

    batt_meter_table_cust_data.battery_profile_t1_size = sizeof(bat2_battery_profile_t1) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t1, &bat2_battery_profile_t1, sizeof(bat2_battery_profile_t1));

    batt_meter_table_cust_data.battery_profile_t2_size = sizeof(bat2_battery_profile_t2) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t2, &bat2_battery_profile_t2, sizeof(bat2_battery_profile_t2));

    batt_meter_table_cust_data.battery_profile_t3_size = sizeof(bat2_battery_profile_t3) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t3, &bat2_battery_profile_t3, sizeof(bat2_battery_profile_t3));

    batt_meter_table_cust_data.r_profile_t0_size = sizeof(bat2_r_profile_t0) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t0, &bat2_r_profile_t0, sizeof(bat2_r_profile_t0));

    batt_meter_table_cust_data.r_profile_t1_size = sizeof(bat2_r_profile_t1) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t1, &bat2_r_profile_t1, sizeof(bat2_r_profile_t1));

    batt_meter_table_cust_data.r_profile_t2_size = sizeof(bat2_r_profile_t2) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t2, &bat2_r_profile_t2, sizeof(bat2_r_profile_t2));

    batt_meter_table_cust_data.r_profile_t3_size = sizeof(bat2_r_profile_t3) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t3, &bat2_r_profile_t3, sizeof(bat2_r_profile_t3));

    /* cust_battery_meter.h */
    batt_meter_cust_data.soc_flow = SW_FG;
    batt_meter_cust_data.temperature_t0 = BAT2_TEMPERATURE_T0;
    batt_meter_cust_data.temperature_t1 = BAT2_TEMPERATURE_T1;
    batt_meter_cust_data.temperature_t2 = BAT2_TEMPERATURE_T2;
    batt_meter_cust_data.temperature_t3 = BAT2_TEMPERATURE_T3;
    batt_meter_cust_data.temperature_t = BAT2_TEMPERATURE_T;

    batt_meter_cust_data.fg_meter_resistance = FG_METER_RESISTANCE;

    /* Qmax for battery  */
    batt_meter_cust_data.q_max_pos_50 = BAT2_Q_MAX_POS_50;
    batt_meter_cust_data.q_max_pos_25 = BAT2_Q_MAX_POS_25;
    batt_meter_cust_data.q_max_pos_0 = BAT2_Q_MAX_POS_0;
    batt_meter_cust_data.q_max_neg_10 = BAT2_Q_MAX_NEG_10;
    batt_meter_cust_data.q_max_pos_50_h_current = BAT2_Q_MAX_POS_50_H_CURRENT;
    batt_meter_cust_data.q_max_pos_25_h_current = BAT2_Q_MAX_POS_25_H_CURRENT;
    batt_meter_cust_data.q_max_pos_0_h_current = BAT2_Q_MAX_POS_0_H_CURRENT;
    batt_meter_cust_data.q_max_neg_10_h_current = BAT2_Q_MAX_NEG_10_H_CURRENT;

#if defined(CHANGE_TRACKING_POINT)
    batt_meter_cust_data.change_tracking_point = 1;
#else
    batt_meter_cust_data.change_tracking_point = 0;
#endif
    batt_meter_cust_data.cust_tracking_point = BAT2_CUST_TRACKING_POINT;
    g_tracking_point = CUST_TRACKING_POINT;
    batt_meter_cust_data.aging_tuning_value = BAT2_AGING_TUNING_VALUE;
    batt_meter_cust_data.ocv_board_compesate = BAT2_OCV_BOARD_COMPESATE;
    batt_meter_cust_data.car_tune_value = BAT2_CAR_TUNE_VALUE;


    batt_meter_cust_data.difference_hwocv_rtc = BAT2_DIFFERENCE_HWOCV_RTC;
    batt_meter_cust_data.difference_hwocv_swocv = BAT2_DIFFERENCE_HWOCV_SWOCV;
    batt_meter_cust_data.difference_swocv_rtc = BAT2_DIFFERENCE_SWOCV_RTC;
    batt_meter_cust_data.max_swocv = BAT2_MAX_SWOCV;

    batt_meter_cust_data.shutdown_system_voltage = bm_cust.shutdown_bat;
    batt_meter_cust_data.recharge_tolerance = BAT2_RECHARGE_TOLERANCE;

    batt_meter_cust_data.batterypseudo100 = BAT2_BATTERYPSEUDO100;
    batt_meter_cust_data.batterypseudo1 = BAT2_BATTERYPSEUDO1;

    batt_meter_cust_data.q_max_by_current = 0;
    batt_meter_cust_data.q_max_sys_voltage = Q_MAX_SYS_VOLTAGE;

#if defined(SHUTDOWN_GAUGE0)
    batt_meter_cust_data.shutdown_gauge0 = 1;
#else
    batt_meter_cust_data.shutdown_gauge0 = 0;
#endif
#if defined(SHUTDOWN_GAUGE1_XMINS)
    batt_meter_cust_data.shutdown_gauge1_xmins = 1;
#else
    batt_meter_cust_data.shutdown_gauge1_xmins = 0;
#endif

    batt_meter_cust_data.shutdown_gauge1_mins = BAT2_SHUTDOWN_GAUGE1_MINS;

    batt_meter_cust_data.min_charging_smooth_time = FG_MIN_CHARGING_SMOOTH_TIME;

    batt_meter_cust_data.apsleep_battery_voltage_compensate = APSLEEP_BATTERY_VOLTAGE_COMPENSATE;

    return 0;
}

int32_t battery_meter_init_custom_meter_data(void)
{
    /* cust_battery_meter_table.h */
    LOG_MSGID_I(battery_management, "[BM_FG]Fuel gauge load zcv table", 0);
    batt_meter_table_cust_data.battery_profile_t0_size = sizeof(battery_profile_t0) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t0, &battery_profile_t0, sizeof(battery_profile_t0));

    batt_meter_table_cust_data.battery_profile_t1_size = sizeof(battery_profile_t1) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t1, &battery_profile_t1, sizeof(battery_profile_t1));

    batt_meter_table_cust_data.battery_profile_t2_size = sizeof(battery_profile_t2) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t2, &battery_profile_t2, sizeof(battery_profile_t2));

    batt_meter_table_cust_data.battery_profile_t3_size = sizeof(battery_profile_t3) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t3, &battery_profile_t3, sizeof(battery_profile_t3));

    batt_meter_table_cust_data.r_profile_t0_size = sizeof(r_profile_t0) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t0, &r_profile_t0, sizeof(r_profile_t0));

    batt_meter_table_cust_data.r_profile_t1_size = sizeof(r_profile_t1) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t1, &r_profile_t1, sizeof(r_profile_t1));

    batt_meter_table_cust_data.r_profile_t2_size = sizeof(r_profile_t2) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t2, &r_profile_t2, sizeof(r_profile_t2));

    batt_meter_table_cust_data.r_profile_t3_size = sizeof(r_profile_t3) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t3, &r_profile_t3, sizeof(r_profile_t3));

    /* cust_battery_meter.h */
    batt_meter_cust_data.soc_flow = SW_FG;
    batt_meter_cust_data.temperature_t0 = TEMPERATURE_T0;
    batt_meter_cust_data.temperature_t1 = TEMPERATURE_T1;
    batt_meter_cust_data.temperature_t2 = TEMPERATURE_T2;
    batt_meter_cust_data.temperature_t3 = TEMPERATURE_T3;
    batt_meter_cust_data.temperature_t = TEMPERATURE_T;

    batt_meter_cust_data.fg_meter_resistance = FG_METER_RESISTANCE;

    /* Qmax for battery  */
    batt_meter_cust_data.q_max_pos_50 = Q_MAX_POS_50;
    batt_meter_cust_data.q_max_pos_25 = Q_MAX_POS_25;
    batt_meter_cust_data.q_max_pos_0 = Q_MAX_POS_0;
    batt_meter_cust_data.q_max_neg_10 = Q_MAX_NEG_10;
    batt_meter_cust_data.q_max_pos_50_h_current = Q_MAX_POS_50_H_CURRENT;
    batt_meter_cust_data.q_max_pos_25_h_current = Q_MAX_POS_25_H_CURRENT;
    batt_meter_cust_data.q_max_pos_0_h_current = Q_MAX_POS_0_H_CURRENT;
    batt_meter_cust_data.q_max_neg_10_h_current = Q_MAX_NEG_10_H_CURRENT;

#if defined(CHANGE_TRACKING_POINT)
    batt_meter_cust_data.change_tracking_point = 1;
#else
    batt_meter_cust_data.change_tracking_point = 0;
#endif
    batt_meter_cust_data.cust_tracking_point = CUST_TRACKING_POINT;
    g_tracking_point = CUST_TRACKING_POINT;
    batt_meter_cust_data.aging_tuning_value = AGING_TUNING_VALUE;
    batt_meter_cust_data.ocv_board_compesate = OCV_BOARD_COMPESATE;
    batt_meter_cust_data.car_tune_value = CAR_TUNE_VALUE;


    batt_meter_cust_data.difference_hwocv_rtc = DIFFERENCE_HWOCV_RTC;
    batt_meter_cust_data.difference_hwocv_swocv = DIFFERENCE_HWOCV_SWOCV;
    batt_meter_cust_data.difference_swocv_rtc = DIFFERENCE_SWOCV_RTC;
    batt_meter_cust_data.max_swocv = MAX_SWOCV;

    batt_meter_cust_data.shutdown_system_voltage = bm_cust.shutdown_bat;
    batt_meter_cust_data.recharge_tolerance = RECHARGE_TOLERANCE;

    batt_meter_cust_data.batterypseudo100 = BATTERYPSEUDO100;
    batt_meter_cust_data.batterypseudo1 = BATTERYPSEUDO1;

    batt_meter_cust_data.q_max_by_current = 0;
    batt_meter_cust_data.q_max_sys_voltage = Q_MAX_SYS_VOLTAGE;

#if defined(SHUTDOWN_GAUGE0)
    batt_meter_cust_data.shutdown_gauge0 = 1;
#else
    batt_meter_cust_data.shutdown_gauge0 = 0;
#endif
#if defined(SHUTDOWN_GAUGE1_XMINS)
    batt_meter_cust_data.shutdown_gauge1_xmins = 1;
#else
    batt_meter_cust_data.shutdown_gauge1_xmins = 0;
#endif

    batt_meter_cust_data.shutdown_gauge1_mins = SHUTDOWN_GAUGE1_MINS;

    batt_meter_cust_data.min_charging_smooth_time = FG_MIN_CHARGING_SMOOTH_TIME;

    batt_meter_cust_data.apsleep_battery_voltage_compensate = APSLEEP_BATTERY_VOLTAGE_COMPENSATE;

    return 0;
}

int32_t battery_meter_init_deputy_meter_data(void)
{
    /* cust_battery_meter_table.h */
    LOG_MSGID_I(battery_management, "[BM_FG]Fuel gauge load deputy zcv table", 0);
    batt_meter_table_cust_data.battery_profile_t0_size = sizeof(battery_profile_deputy_t0) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t0, &battery_profile_deputy_t0, sizeof(battery_profile_deputy_t0));

    batt_meter_table_cust_data.battery_profile_t1_size = sizeof(battery_profile_deputy_t1) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t1, &battery_profile_deputy_t1, sizeof(battery_profile_deputy_t1));

    batt_meter_table_cust_data.battery_profile_t2_size = sizeof(battery_profile_deputy_t2) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t2, &battery_profile_deputy_t2, sizeof(battery_profile_deputy_t2));

    batt_meter_table_cust_data.battery_profile_t3_size = sizeof(battery_profile_deputy_t3) / sizeof(BATTERY_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.battery_profile_t3, &battery_profile_deputy_t3, sizeof(battery_profile_deputy_t3));

    batt_meter_table_cust_data.r_profile_t0_size = sizeof(r_deputy_profile_t0) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t0, &r_deputy_profile_t0, sizeof(r_deputy_profile_t0));

    batt_meter_table_cust_data.r_profile_t1_size = sizeof(r_deputy_profile_t1) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t1, &r_deputy_profile_t1, sizeof(r_deputy_profile_t1));

    batt_meter_table_cust_data.r_profile_t2_size = sizeof(r_deputy_profile_t2) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t2, &r_deputy_profile_t2, sizeof(r_deputy_profile_t2));

    batt_meter_table_cust_data.r_profile_t3_size = sizeof(r_deputy_profile_t3) / sizeof(R_PROFILE_STRUC);
    memcpy(&batt_meter_table_cust_data.r_profile_t3, &r_deputy_profile_t3, sizeof(r_deputy_profile_t3));
    batt_meter_cust_data.soc_flow = SW_FG;
    batt_meter_cust_data.temperature_t0 = DEPUTY_TEMPERATURE_T0;
    batt_meter_cust_data.temperature_t1 = DEPUTY_TEMPERATURE_T1;
    batt_meter_cust_data.temperature_t2 = DEPUTY_TEMPERATURE_T2;
    batt_meter_cust_data.temperature_t3 = DEPUTY_TEMPERATURE_T3;
    batt_meter_cust_data.temperature_t = DEPUTY_TEMPERATURE_T;

    batt_meter_cust_data.fg_meter_resistance = DEPUTY_FG_METER_RESISTANCE;

    /* Qmax for battery  */
    batt_meter_cust_data.q_max_pos_50 = DEPUTY_Q_MAX_POS_50;
    batt_meter_cust_data.q_max_pos_25 = DEPUTY_Q_MAX_POS_25;
    batt_meter_cust_data.q_max_pos_0 = DEPUTY_Q_MAX_POS_0;
    batt_meter_cust_data.q_max_neg_10 = DEPUTY_Q_MAX_NEG_10;
    batt_meter_cust_data.q_max_pos_50_h_current = DEPUTY_Q_MAX_POS_50_H_CURRENT;
    batt_meter_cust_data.q_max_pos_25_h_current = DEPUTY_Q_MAX_POS_25_H_CURRENT;
    batt_meter_cust_data.q_max_pos_0_h_current = DEPUTY_Q_MAX_POS_0_H_CURRENT;
    batt_meter_cust_data.q_max_neg_10_h_current = DEPUTY_Q_MAX_NEG_10_H_CURRENT;

#if defined(CHANGE_TRACKING_POINT)
    batt_meter_cust_data.change_tracking_point = 1;
#else
    batt_meter_cust_data.change_tracking_point = 0;
#endif
    batt_meter_cust_data.cust_tracking_point = DEPUTY_CUST_TRACKING_POINT;
    g_tracking_point = CUST_TRACKING_POINT;
    batt_meter_cust_data.aging_tuning_value = DEPUTY_AGING_TUNING_VALUE;
    batt_meter_cust_data.ocv_board_compesate = DEPUTY_OCV_BOARD_COMPESATE;
    batt_meter_cust_data.car_tune_value = DEPUTY_CAR_TUNE_VALUE;


    batt_meter_cust_data.difference_hwocv_rtc = DEPUTY_DIFFERENCE_HWOCV_RTC;
    batt_meter_cust_data.difference_hwocv_swocv = DEPUTY_DIFFERENCE_HWOCV_SWOCV;
    batt_meter_cust_data.difference_swocv_rtc = DEPUTY_DIFFERENCE_SWOCV_RTC;
    batt_meter_cust_data.max_swocv = DEPUTY_MAX_SWOCV;

    batt_meter_cust_data.shutdown_system_voltage = bm_cust.shutdown_bat;
    batt_meter_cust_data.recharge_tolerance = DEPUTY_RECHARGE_TOLERANCE;

    batt_meter_cust_data.batterypseudo100 = DEPUTY_BATTERYPSEUDO100;
    batt_meter_cust_data.batterypseudo1 = DEPUTY_BATTERYPSEUDO1;

    batt_meter_cust_data.q_max_by_current = 0;
    batt_meter_cust_data.q_max_sys_voltage = Q_MAX_SYS_VOLTAGE;

#if defined(SHUTDOWN_GAUGE0)
    batt_meter_cust_data.shutdown_gauge0 = 1;
#else
    batt_meter_cust_data.shutdown_gauge0 = 0;
#endif
#if defined(SHUTDOWN_GAUGE1_XMINS)
    batt_meter_cust_data.shutdown_gauge1_xmins = 1;
#else
    batt_meter_cust_data.shutdown_gauge1_xmins = 0;
#endif

    batt_meter_cust_data.shutdown_gauge1_mins = DEPUTY_SHUTDOWN_GAUGE1_MINS;

    batt_meter_cust_data.min_charging_smooth_time = FG_MIN_CHARGING_SMOOTH_TIME;

    batt_meter_cust_data.apsleep_battery_voltage_compensate = APSLEEP_BATTERY_VOLTAGE_COMPENSATE;

    return 0;
}

int32_t battery_meter_init_custom_data(void)
{
    battery_meter_init_custom_meter_data();
    return 0;
}
int32_t battery_meter_force_get_tbat(bool update)
{

    int32_t bat_temperature_val = 0;
    static int32_t pre_bat_temperature_val = -100;
    if (update == true || pre_bat_temperature_val == -100) {
        bat_temperature_val = battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE);
        pre_bat_temperature_val = bat_temperature_val;
    } else {
        bat_temperature_val = pre_bat_temperature_val;
    }
    return bat_temperature_val;
}

int32_t battery_meter_get_battery_voltage(bool update)
{
    int32_t val = 5;
    static int32_t pre_val = -1;
    if (update == true || pre_val == -1) {
        val = 5;
        //hal_charger_meter_get_battery_voltage_sense(&val);
        val = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
        pre_val = val;
    } else {
        val = pre_val;
    }
    g_sw_vbat_temp = val;
    return val;
}

#if defined(SOC_BY_SW_FG)
void battery_meter_get_fuel_gauge_init_data(void)
{
    bool charging_enable = false;

    /*stop charging for vbat measurement*/
    //hal_charger_enable(charging_enable);
    battery_set_enable_charger(charging_enable);

    hal_gpt_delay_ms(50);
    /* 1. Get Raw Data */
    gFG_voltage_init = battery_meter_get_battery_voltage(true);
    gFG_current_init = FG_CURRENT_INIT_VALUE;
    gFG_Is_Charging_init = false;

    charging_enable = true;
    battery_set_enable_charger(charging_enable);
    LOG_MSGID_I(battery_management, "[BM_FG][battery_meter_get_fuel_gauge_init_data](gFG_voltage_init %d, gFG_current_init %d, gFG_Is_Charging_init %d)", 3,
                (int)gFG_voltage_init, (int)gFG_current_init, (int)gFG_Is_Charging_init);
}
#endif



int32_t battery_meter_get_battery_temperature(void)
{
    return battery_meter_force_get_tbat(true);
}

int32_t battery_meter_get_battery_capacity_level(int32_t percentage)
{
    int32_t capacity_level = 0;

    if (percentage >= BATTERY_CAPACITY_LEVEL_5) {
        capacity_level = 5;
    } else if (percentage >= BATTERY_CAPACITY_LEVEL_4) {
        capacity_level = 4;
    } else if (percentage >= BATTERY_CAPACITY_LEVEL_3) {
        capacity_level = 3;
    } else if (percentage >= BATTERY_CAPACITY_LEVEL_2) {
        capacity_level = 2;
    } else if (percentage >= BATTERY_CAPACITY_LEVEL_1) {
        capacity_level = 1;
    } else {
        capacity_level = 0;
    }

    return capacity_level;

}

uint32_t battery_meter_fuel_gauge_command_callback(FG_CTRL_CMD cmd, void *parameter, void *data)
{
    if (cmd < FG_CMD_NUMBER) {
        switch (cmd) {
            case FG_CMD_GET_RTC_SPARE_FG_VALUE: {
                hal_rtc_status_t ret;
                char rtc_value = 0;
                ret = hal_rtc_get_data(0, &rtc_value, 1);
                if (HAL_RTC_STATUS_OK == ret) {
                    *((uint32_t *)data) = rtc_value;
                }
            }
            break;
            case FG_CMD_IS_CHARGER_EXIST:
                *((bool *)data) = battery_core_get_charger_status();
                break;
            case FG_CMD_GET_SHUTDOWN_SYSTEM_VOLTAGE:
                *((int32_t *)data) = shutdown_system_voltage;
                break;
            case FG_CMD_GET_BATTERY_INIT_VOLTAGE:
                *((int32_t *)data) = gFG_voltage_init;
                break;
            case FG_CMD_GET_HW_FG_INIT_CURRENT:
                *((int32_t *)data) = gFG_current_init;
                break;
            case FG_CMD_GET_TEMPERTURE: {
                int32_t temperture = 0;
                temperture = battery_meter_force_get_tbat(*((bool *)parameter));
                *((int32_t *)data) = temperture;
            }
            break;
            case FG_CMD_GET_CUSTOM_TABLE: {

                ((struct battery_meter_table_custom_data_p *)data)->battery_profile_t0_size = batt_meter_table_cust_data.battery_profile_t0_size;
                ((struct battery_meter_table_custom_data_p *)data)->battery_profile_t1_size = batt_meter_table_cust_data.battery_profile_t1_size;
                ((struct battery_meter_table_custom_data_p *)data)->battery_profile_t2_size = batt_meter_table_cust_data.battery_profile_t2_size;
                ((struct battery_meter_table_custom_data_p *)data)->battery_profile_t3_size = batt_meter_table_cust_data.battery_profile_t3_size;
                ((struct battery_meter_table_custom_data_p *)data)->battery_profile_temperature_size = batt_meter_table_cust_data.battery_profile_temperature_size;

                ((struct battery_meter_table_custom_data_p *)data)->r_profile_t0_size = batt_meter_table_cust_data.r_profile_t0_size;
                ((struct battery_meter_table_custom_data_p *)data)->r_profile_t1_size = batt_meter_table_cust_data.r_profile_t1_size;
                ((struct battery_meter_table_custom_data_p *)data)->r_profile_t2_size = batt_meter_table_cust_data.r_profile_t2_size;
                ((struct battery_meter_table_custom_data_p *)data)->r_profile_t3_size = batt_meter_table_cust_data.r_profile_t3_size;
                ((struct battery_meter_table_custom_data_p *)data)->r_profile_temperature_size = batt_meter_table_cust_data.r_profile_temperature_size;

                ((struct battery_meter_table_custom_data_p *)data)->cust_battery_profile_t0 = &batt_meter_table_cust_data.battery_profile_t0[0];
                ((struct battery_meter_table_custom_data_p *)data)->cust_battery_profile_t1 = &batt_meter_table_cust_data.battery_profile_t1[0];
                ((struct battery_meter_table_custom_data_p *)data)->cust_battery_profile_t2 = &batt_meter_table_cust_data.battery_profile_t2[0];
                ((struct battery_meter_table_custom_data_p *)data)->cust_battery_profile_t3 = &batt_meter_table_cust_data.battery_profile_t3[0];
                ((struct battery_meter_table_custom_data_p *)data)->cust_battery_profile_temperature = &batt_meter_table_cust_data.battery_profile_temperature[0];

                ((struct battery_meter_table_custom_data_p *)data)->cust_r_profile_t0 = &batt_meter_table_cust_data.r_profile_t0[0];
                ((struct battery_meter_table_custom_data_p *)data)->cust_r_profile_t1 = &batt_meter_table_cust_data.r_profile_t1[0];
                ((struct battery_meter_table_custom_data_p *)data)->cust_r_profile_t2 = &batt_meter_table_cust_data.r_profile_t2[0];
                ((struct battery_meter_table_custom_data_p *)data)->cust_r_profile_t3 = &batt_meter_table_cust_data.r_profile_t3[0];
                ((struct battery_meter_table_custom_data_p *)data)->cust_r_profile_temperature = &batt_meter_table_cust_data.r_profile_temperature[0];
                LOG_MSGID_I(battery_management, "[BM_FG]cust_r_profile_t0  size = %d ", 1, (int)((struct battery_meter_table_custom_data_p *)data)->r_profile_t0_size);
                LOG_MSGID_I(battery_management, "[BM_FG]cust_r_profile_t1  size = %d ", 1, (int)((struct battery_meter_table_custom_data_p *)data)->r_profile_t1_size);
                LOG_MSGID_I(battery_management, "[BM_FG]cust_r_profile_t2  size = %d ", 1, (int)((struct battery_meter_table_custom_data_p *)data)->r_profile_t2_size);
                LOG_MSGID_I(battery_management, "[BM_FG]cust_r_profile_t3  size = %d ", 1, (int)((struct battery_meter_table_custom_data_p *)data)->r_profile_t3_size);
                LOG_MSGID_I(battery_management, "[BM_FG]cust_r_profile_temperature  size = %d ", 1, (int)((struct battery_meter_table_custom_data_p *)data)->r_profile_temperature_size);
            }
            break;
            case FG_CMD_GET_FG_CUSTOM_DATA:
                memcpy(data, &batt_meter_cust_data, sizeof(batt_meter_cust_data));
                break;
            case FG_CMD_GET_HW_OCV: {
                int32_t voltage = 0;
                if (battery_init_setting_flag) {
                    voltage = pmu_auxadc_get_channel_value(PMU_AUX_PN_ZCV);
                } else {
                    voltage = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
                }
                *((int32_t *) data) = voltage;
                gFG_hwocv = voltage;
                (void) gFG_hwocv;
            }
            break;
            case FG_CMD_GET_BATTERY_PLUG_STATUS: {
                //hal_charger_meter_get_battery_plug_out_status((int32_t *)data);
                //gFG_plugout_status = *((int32_t *)data);
                //(void)gFG_plugout_status;
            }
            break;
            case FG_CMD_IS_BATTERY_FULL: {
                *((bool *)data) = BMT_status.bat_full;
            }
            break;
            case FG_CMD_GET_CV_VALUE: {
                *((uint32_t *)data) = g_cv_voltage;
            }
            break;
            case FG_CMD_GET_SUSPEND_CAR: {
                int32_t  car = ap_suspend_car / 3600;
                *((int32_t *)data) = car;
                ap_suspend_car = ap_suspend_car % 3600;
            }
            break;
            case FG_CMD_GET_SUSPEND_TIME: {
                *((int32_t *)data) = swfg_ap_suspend_time;
                swfg_ap_suspend_time = 0;
            }
            break;
            case FG_CMD_GET_DURATION_TIME: {
                int32_t duration_time = 0;
                BATTERY_TIME_ENUM duration_type;
                duration_type = *((BATTERY_TIME_ENUM *)parameter);
                duration_time = battery_core_get_duration_time(duration_type);
                *((int32_t *)data) = duration_time;
            }
            break;
            case FG_CMD_GET_BATTERY_VOLTAGE: {
                bool update;
                int32_t voltage = 0;
                update = *((bool *)parameter);
                if (update == true) {
                    voltage = battery_meter_get_battery_voltage(true);
                } else {
                    voltage = BMT_status.bat_vol;
                }
                *((int32_t *)data) = voltage;
            }
            break;
            case FG_CMD_IS_HW_OCV_UPDATE: {
                *((int32_t *)data) = is_hwocv_update;
                is_hwocv_update = false;
            }
            break;
            case FG_CMD_GET_CHARGE_TRACKING_TIME: {
                *((int32_t *)data) = charge_tracking_time;
            }
            break;
            case FG_CMD_GET_DISCHARGE_TRACKING_TIME: {
                *((int32_t *)data) = discharge_tracking_time;
            }
            break;
            case FG_CMD_SET_SWOCV:
                gFG_voltage = *((int32_t *)data);
                break;
            case FG_CMD_SET_SOC: {
                gFG_capacity_by_c = *((int32_t *)data);
                BMT_status.SOC = gFG_capacity_by_c;
            }
            break;
            case FG_CMD_SET_WAKEUP_SMOOTH_TIME: {
                wake_up_smooth_time = *((uint32_t *)data);
            }
            break;
            case FG_CMD_SET_BATTERY_FULL: {
                BMT_status.bat_full = *((bool *)data);
            }
            break;
            case FG_CMD_SET_UI_SOC: {
                BMT_status.UI_SOC = *((int32_t *)data);
            }
            break;
            case FG_CMD_SET_UI_SOC2: {
                BMT_status.UI_SOC2 = *((int32_t *)data);
                BMT_status.UI_SOC2_LEVEL = battery_meter_get_battery_capacity_level(BMT_status.UI_SOC2);
                if (!g_battery_soc_ready) {
                    g_battery_soc_ready = true;
                }
            }
            break;
            case FG_CMD_SET_RTC: {
                hal_rtc_status_t ret;
                char rtc_value = (char) * ((int32_t *)data);
                ret = hal_rtc_set_data(0, &rtc_value, 1);
                if (HAL_RTC_STATUS_OK != ret) {
                    LOG_MSGID_I(battery_management, "[BM_FG]Set RTC data error", 0);
                }
            }
            break;
            case FG_CMD_SET_OAM_V_OCV: {
                oam_v_ocv = *((int32_t *)data);
                (void)oam_v_ocv;
            }
            break;
            case FG_CMD_SET_OAM_R: {
                oam_r = *((int32_t *)data);
                (void)oam_r;
            }
            break;
            case FG_CMD_SET_POWEROFF: {
#ifndef DISABLE_FG_POWER_OFF
                LOG_MSGID_I(battery_management, "[BM_FG]system need power off, FG_CMD_SET_POWEROFF power request!!", 0);
                hal_rtc_enter_rtc_mode();
#endif
            }
            break;

            default:

                break;
        }
        return 1;
    }
    return 0;
}

extern log_control_block_t log_control_block_bmt;
void battery_meter_fuel_gauge_log(const char *format, va_list args)
{
#ifndef MTK_DEBUG_LEVEL_NONE
    vprint_module_log(&log_control_block_bmt, __FUNCTION__, __LINE__, PRINT_LEVEL_INFO, format, args);
#endif
}

int32_t battery_meter_initial(void)
{

    static bool meter_initilized = false;

    if (meter_initilized == false) {
#if defined(SOC_BY_SW_FG)
        battery_meter_get_fuel_gauge_init_data();
#endif
        meter_initilized = true;
    }

    fgauge_register_debug_message_callback(battery_meter_fuel_gauge_log);
    fgauge_register_callback(battery_meter_fuel_gauge_command_callback);
    fgauge_initialization();

    return 0;
}

int32_t battery_meter_sync_current_sense_offset(int32_t bat_i_sense_offset)
{
    g_I_SENSE_offset = bat_i_sense_offset;
    return 0;
}

int32_t battery_meter_get_battery_zcv(void)
{
    return gFG_voltage;
}

int32_t battery_meter_get_battery_nPercent_zcv(void)
{
    return gFG_15_vlot;    /* 15% zcv,  15% can be customized by 100-g_tracking_point */
}

int32_t battery_meter_get_battery_nPercent_UI_SOC(void)
{
    return g_tracking_point;    /* tracking point */
}

int32_t battery_meter_get_voltage_sense(void)
{
    int32_t val = 0;

    val = 1;
    //hal_charger_meter_get_voltage_current_sense(&val);
    return val;
}

void battery_meter_init(void)
{
    battery_meter_init_custom_data();
}
const char *table_name[3] = { "","Normal","Deputy"};
void battery_dump_zcv_table(void){
    int i = 0 ;

    LOG_I(battery_management, "[BM_FG][%s][Percentage:Voltage:Resistance]", table_name[BMT_status.meter_table]);
    LOG_I(battery_management, "[BM_FG]===========[%s:t0]==============", table_name[BMT_status.meter_table]);
    for(i=0;i<51;i++){
        LOG_MSGID_I(battery_management, "[BM_FG]|%d|[%d:%d:%d]", 4,i,
                batt_meter_table_cust_data.battery_profile_t0[i].percentage,
                batt_meter_table_cust_data.battery_profile_t0[i].voltage,
                batt_meter_table_cust_data.r_profile_t0[i].resistance);
    }
    LOG_I(battery_management, "[BM_FG]===========[%s:t1]==============", table_name[BMT_status.meter_table]);
    for(i=0;i<51;i++){
        LOG_MSGID_I(battery_management, "[BM_FG]|%d|[%d:%d:%d]", 4,i,
                batt_meter_table_cust_data.battery_profile_t1[i].percentage,
                batt_meter_table_cust_data.battery_profile_t1[i].voltage,
                batt_meter_table_cust_data.r_profile_t1[i].resistance);
    }
    LOG_I(battery_management, "[BM_FG]===========[%s:t2]==============", table_name[BMT_status.meter_table]);
    for(i=0;i<51;i++){
        LOG_MSGID_I(battery_management, "[BM_FG]|%d|[%d:%d:%d]", 4,i,
                batt_meter_table_cust_data.battery_profile_t2[i].percentage,
                batt_meter_table_cust_data.battery_profile_t2[i].voltage,
                batt_meter_table_cust_data.r_profile_t2[i].resistance);
    }
    LOG_I(battery_management, "[BM_FG]===========[%s:t3]==============", table_name[BMT_status.meter_table]);
    for(i=0;i<51;i++){
        LOG_MSGID_I(battery_management, "[BM_FG]|%d|[%d:%d:%d]", 4,i,
                batt_meter_table_cust_data.battery_profile_t3[i].percentage,
                batt_meter_table_cust_data.battery_profile_t3[i].voltage,
                batt_meter_table_cust_data.r_profile_t3[i].resistance);
    }
    LOG_MSGID_I(battery_management, "[BM_FG]==============Dump End=============", 0);
}
