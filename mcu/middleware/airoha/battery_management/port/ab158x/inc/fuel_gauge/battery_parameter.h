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

#ifndef __BATTERY_PARAMETER_H__
#define __BATTERY_PARAMETER_H__

#include "cust_battery_meter.h"
#include "cust_battery_meter_bat2.h"
#include "cust_battery_meter_deputy.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_CHARGING_TIME
#define MAX_CHARGING_TIME               24*60*60 /* 24hr */
#endif

#ifndef MAX_CV_CHARGING_TIME
#define MAX_CV_CHARGING_TIME            3*60*60  /* 3hr */
#endif

#ifndef MAX_TOP_OFF_CHARGING_TIME
#define MAX_TOP_OFF_CHARGING_TIME       30*60    /* 30min */
#endif

#define BAT_TASK_PERIOD                     10   /* 10sec */

#ifndef OVER_TEMPERATURE_THRESHOLD
#define OVER_TEMPERATURE_THRESHOLD 60
#endif

#define TEMP_POS_60_THRESHOLD  50
#define TEMP_POS_60_THRES_MINUS_X_DEGREE 47

#define TEMP_POS_45_THRESHOLD  45
#define TEMP_POS_45_THRES_MINUS_X_DEGREE 39

#define TEMP_POS_10_THRESHOLD  10
#define TEMP_POS_10_THRES_PLUS_X_DEGREE 16

#define TEMP_POS_0_THRESHOLD  0
#define TEMP_POS_0_THRES_PLUS_X_DEGREE 6

#define TEMP_NEG_10_THRESHOLD  0
#define TEMP_NEG_10_THRES_PLUS_X_DEGREE  0


#ifdef  BAT_SLEEP_FORCE_WAKEUP_DISABLE
#ifndef BAT_SLEEP_WAKE_UP_PERIOD
#define BAT_SLEEP_WAKE_UP_PERIOD        BAT_TASK_PERIOD
#else
#undef BAT_SLEEP_WAKE_UP_PERIOD
#define BAT_SLEEP_WAKE_UP_PERIOD        BAT_TASK_PERIOD
#endif
#else
#ifndef BAT_SLEEP_WAKE_UP_PERIOD
#define BAT_SLEEP_WAKE_UP_PERIOD    10   /* 10sec or 20sec*/
#else
#if !((BAT_SLEEP_WAKE_UP_PERIOD == 10) || (BAT_SLEEP_WAKE_UP_PERIOD == 20))
#warning "BAT_SLEEP_WAKE_UP_PERIOD define invalid, use default 10 sec"
#undef BAT_SLEEP_WAKE_UP_PERIOD
#define BAT_SLEEP_WAKE_UP_PERIOD    10   /* 10sec */
#endif
#endif
#endif

#ifndef Q_MAX_SYS_VOLTAGE
//#define Q_MAX_SYS_VOLTAGE 3300
#define Q_MAX_SYS_VOLTAGE 1300
#endif

#ifndef CHARGE_TRACKING_TIME
#define CHARGE_TRACKING_TIME        60
#endif

#ifndef DISCHARGE_TRACKING_TIME
#define DISCHARGE_TRACKING_TIME        10
#endif

#ifndef RECHARGE_TOLERANCE
#define RECHARGE_TOLERANCE    10
#endif




#ifndef SUSPEND_WAKEUP_TIME
#define SUSPEND_WAKEUP_TIME 300
#endif

#ifndef FG_CURRENT_INIT_VALUE
//#define FG_CURRENT_INIT_VALUE 300
#define FG_CURRENT_INIT_VALUE 10
#endif

#ifndef FG_MIN_CHARGING_SMOOTH_TIME
//#define FG_MIN_CHARGING_SMOOTH_TIME 40
#define FG_MIN_CHARGING_SMOOTH_TIME 10
#endif


#ifndef AP_SLEEP_CAR
#define AP_SLEEP_CAR 20
#endif

#ifndef APSLEEP_BATTERY_VOLTAGE_COMPENSATE
#define APSLEEP_BATTERY_VOLTAGE_COMPENSATE 150
#endif


#ifdef __cplusplus
}
#endif

#endif /*__BATTERY_PARAMETER_H__*/

