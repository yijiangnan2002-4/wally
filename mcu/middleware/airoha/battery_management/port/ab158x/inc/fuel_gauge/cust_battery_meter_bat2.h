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

#ifndef __CUST_BATTERY_METER_BAT2_H__
#define __CUST_BATTERY_METER_BAT2_H__


#ifdef __cplusplus
extern "C" {
#endif

#define SOC_BY_SW_FG /* Enable Fuel gauge*/

#define BAT2_TEMPERATURE_T0             110
#define BAT2_TEMPERATURE_T1             0
#define BAT2_TEMPERATURE_T2             25
#define BAT2_TEMPERATURE_T3             50
#define BAT2_TEMPERATURE_T              255  /* This should be fixed, never change the value */

/*Fuel gauge behavior setting*/
#define BAT2_CHARGE_TRACKING_TIME         60
#define BAT2_DISCHARGE_TRACKING_TIME      10
#define BAT2_RECHARGE_TOLERANCE           5
#define BAT2_BATTERYPSEUDO100             95
#define BAT2_BATTERYPSEUDO1               4

/* Qmax for battery  */
#define BAT2_Q_MAX_POS_50    80
#define BAT2_Q_MAX_POS_25    81
#define BAT2_Q_MAX_POS_0     77
#define BAT2_Q_MAX_NEG_10    72

#define BAT2_Q_MAX_POS_50_H_CURRENT    80
#define BAT2_Q_MAX_POS_25_H_CURRENT    81
#define BAT2_Q_MAX_POS_0_H_CURRENT     77
#define BAT2_Q_MAX_NEG_10_H_CURRENT    72

/*Note : Keep Default Value
 * battery meter parameter */


#define CHANGE_TRACKING_POINT
#define BAT2_CUST_TRACKING_POINT     0
#define BAT2_AGING_TUNING_VALUE      100
#define BAT2_CAR_TUNE_VALUE          100
#define BAT2_FG_METER_RESISTANCE     0
#define BAT2_OCV_BOARD_COMPESATE     0

#define BAT2_DIFFERENCE_HWOCV_RTC         30
#define BAT2_DIFFERENCE_HWOCV_SWOCV       0
#define BAT2_DIFFERENCE_SWOCV_RTC         10
#define BAT2_MAX_SWOCV                    3
/* * * * * * * * * * * * * * */

#define NORMAL_WAKEUP_PERIOD         2700   /* 45 * 60 = 45 min */
/*
 *If defined, UI SOC will keep at 1% and trigger power off after SHUTDOWN_GAUGE1_MINS minutes
 * */
//#define SHUTDOWN_GAUGE1_XMINS
#define BAT2_SHUTDOWN_GAUGE1_MINS         60

/*
 *If defined, system will trigger power off when gauge is 0%
 * */
//#define SHUTDOWN_GAUGE0



#ifdef __cplusplus
}
#endif

#endif /*__CUST_BATTERY_METER_H__*/

