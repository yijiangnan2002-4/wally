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
/*==========[Battery parameter]==========*/
#define BATTERY_VOLTAGE_REFERENCE_POINTS 9
/*==========[Charger parameter]==========*/
#define CHARGER_REGULAR_TIME 120 // second
#define MTK_BATTERY_ULTRA_LOW_BAT 1 /*Set the lowest showed SOC*/
//#define EXTERNAL_CHARGER
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
/*==========[Feature 1]==========*/
/* The first times in EOC state ,
 * It will be waits for one minute to execute EOC behavior.
 * BATTERY_EOC_CHECK_TIME unit is second.
 * EOC_CHECK_ON set BATTERY_OPERATE_ON is feature on.
 * */
#define BATTERY_EOC_CHECK_TIME 60
#define EOC_CHECK_ON BATTERY_OPERATE_ON
