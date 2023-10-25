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
/* Airoha restricted information */

/**
 * File: app_ms_teams_activity.h
 *
 * Description: This file defines the interface of app_ms_teams_activity.c.
 *
 */

#ifndef __APP_MS_TEAMS_UTILS_H__
#define __APP_MS_TEAMS_UTILS_H__

#include "ui_shell_activity.h"
#include "ms_teams.h"
#include "apps_config_event_list.h"
#include "apps_config_state_list.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "FreeRTOS.h"
#ifdef AIR_AIRO_KEY_ENABLE
#include "airo_key_event.h"
#endif
#include "apps_events_key_event.h"
#include "bt_device_manager.h"
#include "apps_aws_sync_event.h"
#include "apps_events_interaction_event.h"
#include "apps_config_key_remapper.h"
#include "apps_config_led_manager.h"
#include "apps_config_led_index_list.h"
#include "apps_dongle_sync_event.h"

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

#define MS_TEAMS_CUSTOMER_EVENT_START 0x10
#define MS_TEAMS_EVENT_VER_SYNC (MS_TEAMS_CUSTOMER_EVENT_START + 0)
#define MS_TEAMS_EVENT_SIDETONE_LEVEL_SYNC (MS_TEAMS_CUSTOMER_EVENT_START + 1)
#define MS_TEAMS_EVENT_IN_EAR_STA (MS_TEAMS_CUSTOMER_EVENT_START + 2)
#define MS_TEAMS_EVENT_DSP_EFFECT_UPDATE (MS_TEAMS_CUSTOMER_EVENT_START + 3)
#define MS_TEAMS_EVENT_VAD_STA (MS_TEAMS_CUSTOMER_EVENT_START + 4)
#define MS_TEAMS_EVENT_AUDIO_CODEC_CHANGED (MS_TEAMS_CUSTOMER_EVENT_START + 5)
#define MS_TEAMS_EVENT_HARD_MUTE_LOCK_CHANGED (MS_TEAMS_CUSTOMER_EVENT_START + 6)
#define MS_TEAMS_EVENT_BATTERY_LEVEL_CHANGED (MS_TEAMS_CUSTOMER_EVENT_START + 7)
#define MS_TEAMS_EVENT_BUTTON_PRESS_INFO_CHANGED (MS_TEAMS_CUSTOMER_EVENT_START + 8)


typedef struct {
    uint8_t type;
    uint8_t data[0];
} PACKED ms_teams_race_pkg_t;

typedef uint8_t telemetry_bool;
typedef struct {
    uint8_t *endpoint_fw;
    uint8_t *base_fw;
    telemetry_bool don_to_seeting;
    uint8_t *endpoint_mode_id;
    uint8_t *endpoint_sn;
    uint8_t *base_sn;
    uint8_t *side_tone_level;
    uint8_t audio_codec;
    uint32_t dsp_effect;
    telemetry_bool mute_lock;
    telemetry_bool headset_worn;
    uint8_t battery_level;
    telemetry_bool device_ready;
    uint8_t radio_link_quality;
    uint8_t *err_msg;
    uint8_t *btn_press_info; // DT
    telemetry_bool wired_device_changed;//DT
    uint32_t local_conference_cnt;
    telemetry_bool voice_mute;
    telemetry_bool dual_purpose_btn;
} app_teams_telemetry_info;

/**
* @brief      This function is used to init ms teams core.
* @return     None
*/
void app_ms_teams_init(void);

/**
* @brief      Check that whether the dongle connected.
* @return     True means the dongle connected.
*/
bool app_ms_teams_is_dongle_connected(void);

void ms_teams_telemetry_report_uint8_value(uint8_t key, uint8_t value, bool sync);

void ms_teams_telemetry_report_uint16_value(uint8_t key, uint16_t value, bool sync);

void ms_teams_telemetry_report_uint32_value(uint8_t key, uint32_t value, bool sync);

void ms_teams_update_product_info(uint8_t *info, uint32_t total_len);

app_teams_telemetry_info *app_ms_teams_get_telemetry_info();

#if 0
/**
* @brief      This function is used to send data to dongle.
* @param[in]  type, the type item.
* @param[in]  data, the data append.
* @param[in]  data_len, the length of data.
* @return     None
*/
void app_ms_teams_send_race_data_to_dongle(uint8_t type, uint8_t *data, uint32_t data_len);

/**
* @brief      This function is used to register event callback on race command.
* @return     None
*/
void app_ms_teams_event_init(void);
#endif
#endif


