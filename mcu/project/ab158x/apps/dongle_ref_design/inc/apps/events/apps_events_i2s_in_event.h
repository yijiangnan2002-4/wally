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

/**
 * File: app_events_i2s_in_event.h
 *
 * Description: This file defines the common structure and functions of i2s in event.
 */


#ifndef __APP_EVENTS_I2S_IN_EVENT_H__
#define __APP_EVENTS_I2S_IN_EVENT_H__

#include "ui_shell_manager.h"
#include "ui_shell_activity.h"

typedef enum {
    APPS_EVENTS_I2S_IN_STATUS_CHANGE,  /**< This event is used for broadcast the status of i2s in. */
    APPS_EVENTS_I2S_IN_VOLUME_CHANGE, /**< This event is used for broadcast the volume of i2s in. */
} apps_i2s_in_events_t;

typedef enum {
    APP_I2S_IN_VOL_PORT_0,       /**< I2S in port0 volume. */
    APP_I2S_IN_VOL_PORT_1,       /**<  I2S in port1 volume. */
} apps_i2s_in_vol_port_t;

typedef enum {
    APP_I2S_IN_VOL_UP,            /**< Up i2s in volume level. */
    APP_I2S_IN_VOL_DOWN,          /**< Down i2s in volume level. */
    APP_I2S_IN_VOL_SET,           /**< Set i2s in volume level. */
} apps_i2s_in_vol_action_t;

typedef struct {
#ifdef AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
    uint8_t                i2s_port;
#endif
    uint8_t                i2s_state;
    uint16_t               i2s_device;
    uint16_t               i2s_interface;
    uint32_t               i2s_sample_rate;
    uint32_t               i2s_famart;
    uint32_t               i2s_word_len;
} app_i2s_in_det_t;

typedef struct {
    apps_i2s_in_vol_port_t      vol_port;
    apps_i2s_in_vol_action_t    vol_action;
    uint8_t                     vol_level;
} app_i2s_in_vol_t;

/**
 * @brief      Initialize the line in event.
 */
void app_events_i2s_in_init(void);


#endif /* __APP_EVENTS_I2S_IN_EVENT_H__ */
