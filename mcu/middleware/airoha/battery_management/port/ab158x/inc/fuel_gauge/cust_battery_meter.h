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

#ifndef __CUST_BATTERY_METER_H__
#define __CUST_BATTERY_METER_H__


#ifdef __cplusplus
extern "C" {
#endif

#define SOC_BY_SW_FG /* Enable Fuel gauge*/

#define TEMPERATURE_T0             110
#define TEMPERATURE_T1             0
#define TEMPERATURE_T2             25
#define TEMPERATURE_T3             50
#define TEMPERATURE_T              255  /* This should be fixed, never change the value */

/*Fuel gauge behavior setting*/
#define CHARGE_TRACKING_TIME         60
#define DISCHARGE_TRACKING_TIME      10
#define RECHARGE_TOLERANCE           5
#define BATTERYPSEUDO100             95
#define BATTERYPSEUDO1               4

/* Qmax for battery  */
#define Q_MAX_POS_50    83
#define Q_MAX_POS_25    84
#define Q_MAX_POS_0     79
#define Q_MAX_NEG_10    75

#define Q_MAX_POS_50_H_CURRENT    83
#define Q_MAX_POS_25_H_CURRENT    84
#define Q_MAX_POS_0_H_CURRENT     79
#define Q_MAX_NEG_10_H_CURRENT    75

/*Note : Keep Default Value
 * battery meter parameter */


#define CHANGE_TRACKING_POINT
#define CUST_TRACKING_POINT     0
#define AGING_TUNING_VALUE      100
#define CAR_TUNE_VALUE          100
#define FG_METER_RESISTANCE     0
#define OCV_BOARD_COMPESATE     0

#define DIFFERENCE_HWOCV_RTC         30
#define DIFFERENCE_HWOCV_SWOCV       0
#define DIFFERENCE_SWOCV_RTC         10
#define MAX_SWOCV                    3
/* * * * * * * * * * * * * * */

#define NORMAL_WAKEUP_PERIOD         2700   /* 45 * 60 = 45 min */


#define SHUTDOWN_GAUGE1_MINS         60

//#define SHUTDOWN_GAUGE0
//#define SHUTDOWN_GAUGE1_XMINS
#define BATTERY_CATEGORY 1

/*Disable Fuel gauge command : FG_CMD_SET_POWEROFF*/
#define DISABLE_FG_POWER_OFF

#ifdef __cplusplus
}
#endif

#endif /*__CUST_BATTERY_METER_H__*/

