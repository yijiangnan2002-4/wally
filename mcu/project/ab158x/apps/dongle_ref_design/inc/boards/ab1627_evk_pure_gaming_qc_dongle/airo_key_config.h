/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef __AIRO_KEY_CONFIG_H__
#define __AIRO_KEY_CONFIG_H__

#ifdef __cplusplus
extern "C"
{
#endif

/****** feature control ****************************************************/
#include "hal_feature_config.h"

/*if want to use powerkey as normal key, please uncomment the below definition*/
#ifdef HAL_PMU_MODULE_ENABLED
#define AIRO_KEY_FEATRURE_POWERKEY
#endif

/*If user needs a press event of powerkey during startup, please uncomment the following definition. */
#ifdef AIRO_KEY_FEATRURE_POWERKEY
#define AIRO_KEY_FEATRURE_POWERKEY_POWERON_PRESS_EVENT
#endif

/*If user needs a press event of eintkey during startup, please uncomment the following definition. */
//#ifdef MTK_EINT_KEY_ENABLE
//#define AIRO_KEY_FEATRURE_EINTKEY_POWERON_PRESS_EVENT
//#endif

/*********inlcuding********************************************************/
#include "airo_key_define.h"
#include "hal_keypad_table.h"
#include "hal.h"

/****default configuration defined ****************************************/

#define CFG_SUPPORT_PRESS_RELEASE           1

#define CFG_NO_SUPPORT_PRESS_RELEASE        0

#define CFG_NO_SUPPORT_SLONG                0
#define CFG_SUPPORT_SLONG                   1
#define CFG_NO_SUPPORT_SLONG_REPEAT         0
#define CFG_SUPPORT_SLONG_REPEAT            1

#define CFG_NO_SUPPORT_MULTIPLE_CLICK       0
#define CFG_SUPPORT_MULTIPLE_CLICK_SHORT    1
#define CFG_SUPPORT_MULTIPLE_CLICK_DOUBLE   2
#define CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE   3

#define CFG_NO_SUPPORT_LONG_PRESS           0
#define CFG_SUPPORT_LONG_PRESS_1            1
#define CFG_SUPPORT_LONG_PRESS_2            2
#define CFG_SUPPORT_LONG_PRESS_3            3

#define CFG_NO_SUPPORT_LONG_REPEAT          0
#define CFG_SUPPORT_LONG_REPEAT             1

/*the default time,uint:ms*/
#define default_t_p                         300   //the multiple click press time
#define default_t_r                         200   //the multiple click release time for waiting next click
#define default_t_end                       300   //the silence time when only  support someone click
#define default_t_repeat                    200   //the repeat time inlcude slong repeat and long repeat
#define default_t_l_p1                      500   //the longpress 1 time               , long1 = default_t_l_p1
#define default_t_l_p2                      1000  //the longpress 1 -> longpress 2 time, long2 = default_t_l_p1 + default_t_l_p2
#define default_t_l_p3                      1500  //the longpress 2 -> longpress 3 time, long3 = default_t_l_p1 + default_t_l_p2 + default_t_l_p3
#define default_t_slong                     500   //the slong time

/*************** User defined *************************************************/

/********* key definition ************/
/*
Definition format:

key_data:       key name value definition
t_prss:         multiple press time
t_release:      multiple release time for next click
t_silence:      multiple silence time
t_repeat:       repeat time
t_long_press_1: long1 press time
t_long_press_2: long2 press time
t_long_press_3: long3 press time
t_slong:        slong time
press_release:  supported press and release event or not
slong:          supported song event or not
slong_repeat:   supported song repeat or not
long_level:     supported longpress level
long_repeat:    supported longpress repeat or not
multiple_click: supported multiple clicks or not*/

#define AIRO_CONFIG_KEY_0 \
    EINT_KEY_0,\
    {default_t_p,\
    default_t_r,\
    default_t_end,\
    default_t_repeat,\
    default_t_l_p1,\
    default_t_l_p2,\
    default_t_l_p3,\
    default_t_slong},\
    {CFG_SUPPORT_PRESS_RELEASE,\
    CFG_SUPPORT_SLONG,\
    CFG_SUPPORT_SLONG_REPEAT,\
    CFG_SUPPORT_LONG_PRESS_3,\
    CFG_SUPPORT_LONG_REPEAT,\
    CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE}

#define AIRO_CONFIG_KEY_1 \
    EINT_KEY_1,\
    {default_t_p,\
    default_t_r,\
    default_t_end,\
    default_t_repeat,\
    default_t_l_p1,\
    default_t_l_p2,\
    default_t_l_p3,\
    default_t_slong},\
    {CFG_SUPPORT_PRESS_RELEASE,\
    CFG_SUPPORT_SLONG,\
    CFG_SUPPORT_SLONG_REPEAT,\
    CFG_SUPPORT_LONG_PRESS_3,\
    CFG_SUPPORT_LONG_REPEAT,\
    CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE}


#define AIRO_CONFIG_KEY_2 \
    EINT_KEY_2,\
    {default_t_p,\
    default_t_r,\
    default_t_end,\
    default_t_repeat,\
    default_t_l_p1,\
    default_t_l_p2,\
    default_t_l_p3,\
    default_t_slong},\
    {CFG_SUPPORT_PRESS_RELEASE,\
    CFG_SUPPORT_SLONG,\
    CFG_SUPPORT_SLONG_REPEAT,\
    CFG_SUPPORT_LONG_PRESS_3,\
    CFG_SUPPORT_LONG_REPEAT,\
    CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE}


#define AIRO_CONFIG_KEY_3 \
    EINT_KEY_3,\
    {default_t_p,\
    default_t_r,\
    default_t_end,\
    default_t_repeat,\
    default_t_l_p1,\
    default_t_l_p2,\
    default_t_l_p3,\
    default_t_slong},\
    {CFG_SUPPORT_PRESS_RELEASE,\
    CFG_SUPPORT_SLONG,\
    CFG_SUPPORT_SLONG_REPEAT,\
    CFG_SUPPORT_LONG_PRESS_3,\
    CFG_SUPPORT_LONG_REPEAT,\
    CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE}

#define AIRO_CONFIG_KEY_4 \
    EINT_KEY_4,\
    {default_t_p,\
    default_t_r,\
    default_t_end,\
    default_t_repeat,\
    default_t_l_p1,\
    default_t_l_p2,\
    default_t_l_p3,\
    default_t_slong},\
    {CFG_SUPPORT_PRESS_RELEASE,\
    CFG_SUPPORT_SLONG,\
    CFG_SUPPORT_SLONG_REPEAT,\
    CFG_SUPPORT_LONG_PRESS_3,\
    CFG_SUPPORT_LONG_REPEAT,\
    CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE}


#define AIRO_CONFIG_KEY_5 \
    DEVICE_KEY_POWER,\
    {default_t_p,\
    default_t_r,\
    default_t_end,\
    default_t_repeat,\
    default_t_l_p1,\
    default_t_l_p2,\
    default_t_l_p3,\
    default_t_slong},\
    {CFG_SUPPORT_PRESS_RELEASE,\
    CFG_SUPPORT_SLONG,\
    CFG_SUPPORT_SLONG_REPEAT,\
    CFG_SUPPORT_LONG_PRESS_3,\
    CFG_SUPPORT_LONG_REPEAT,\
    CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE}


#define AIRO_CONFIG_KEY_6 \
    DEVICE_KEY_0,\
    {default_t_p,\
    default_t_r,\
    default_t_end,\
    default_t_repeat,\
    default_t_l_p1,\
    default_t_l_p2,\
    default_t_l_p3,\
    default_t_slong},\
    {CFG_SUPPORT_PRESS_RELEASE,\
    CFG_SUPPORT_SLONG,\
    CFG_SUPPORT_SLONG_REPEAT,\
    CFG_SUPPORT_LONG_PRESS_3,\
    CFG_SUPPORT_LONG_REPEAT,\
    CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE}


#define AIRO_CONFIG_KEY_7 \
    DEVICE_KEY_1,\
    {default_t_p,\
    default_t_r,\
    default_t_end,\
    default_t_repeat,\
    default_t_l_p1,\
    default_t_l_p2,\
    default_t_l_p3,\
    default_t_slong},\
    {CFG_SUPPORT_PRESS_RELEASE,\
    CFG_SUPPORT_SLONG,\
    CFG_SUPPORT_SLONG_REPEAT,\
    CFG_SUPPORT_LONG_PRESS_3,\
    CFG_SUPPORT_LONG_REPEAT,\
    CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE}


#define AIRO_CONFIG_KEY_8 \
    DEVICE_KEY_2,\
    {default_t_p,\
    default_t_r,\
    default_t_end,\
    default_t_repeat,\
    default_t_l_p1,\
    default_t_l_p2,\
    default_t_l_p3,\
    default_t_slong},\
    {CFG_SUPPORT_PRESS_RELEASE,\
    CFG_SUPPORT_SLONG,\
    CFG_SUPPORT_SLONG_REPEAT,\
    CFG_SUPPORT_LONG_PRESS_3,\
    CFG_SUPPORT_LONG_REPEAT,\
    CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE}

#define AIRO_CONFIG_KEY_9 \
    DEVICE_KEY_3,\
    {default_t_p,\
    default_t_r,\
    default_t_end,\
    default_t_repeat,\
    default_t_l_p1,\
    default_t_l_p2,\
    default_t_l_p3,\
    default_t_slong},\
    {CFG_SUPPORT_PRESS_RELEASE,\
    CFG_SUPPORT_SLONG,\
    CFG_SUPPORT_SLONG_REPEAT,\
    CFG_SUPPORT_LONG_PRESS_3,\
    CFG_SUPPORT_LONG_REPEAT,\
    CFG_SUPPORT_MULTIPLE_CLICK_TRIPLE}

#ifdef MTK_EINT_KEY_ENABLE
#define EINT_KEY_MAPPING                     \
                        {AIRO_CONFIG_KEY_0}, \
                        {AIRO_CONFIG_KEY_1}, \
                        {AIRO_CONFIG_KEY_2}, \
                        {AIRO_CONFIG_KEY_3}, \
                        {AIRO_CONFIG_KEY_4},
#else
#define EINT_KEY_MAPPING
#endif

#ifdef AIRO_KEY_FEATRURE_POWERKEY
#define POWERKEY_KEY_MAPPING                 \
                        {AIRO_CONFIG_KEY_5},
#else
#define POWERKEY_KEY_MAPPING
#endif

#ifdef HAL_CAPTOUCH_MODULE_ENABLED
#define CAPTOUCH_KEY_MAPPING                 \
                        {AIRO_CONFIG_KEY_6}, \
                        {AIRO_CONFIG_KEY_7}, \
                        {AIRO_CONFIG_KEY_8}, \
                        {AIRO_CONFIG_KEY_9},
#else
#define CAPTOUCH_KEY_MAPPING
#endif

#define AIRO_KEY_MAPPING {     \
        POWERKEY_KEY_MAPPING   \
        EINT_KEY_MAPPING       \
        CAPTOUCH_KEY_MAPPING   \
}

#ifdef GSENSOR_KEY_ENABLE
/*
Definition format:

key_data:       key name value definition
multiple_click: supported multiple clicks or not
t_interval:     The interval timer between two short click
t_silence:      The silence time after the max click, 0:not support
*/
#define AIRO_CONFIG_GSENSOR_KEY \
	DEVICE_KEY_MENU,\
	CFG_SUPPORT_MULTIPLE_CLICK_DOUBLE,\
	default_t_p,\
	default_t_end


#define AIRO_GSENSOR_KEY_MAPPING {AIRO_CONFIG_GSENSOR_KEY}
#define AIRO_GSENSOR_KEY_I2C_PORT HAL_I2C_MASTER_2
#endif

/**************User define end**********************************************/



#ifdef __cplusplus
}
#endif

#endif /* __AIRO_KEY_CONFIG_H__ */
