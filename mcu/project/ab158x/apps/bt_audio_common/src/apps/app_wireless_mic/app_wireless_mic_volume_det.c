/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
#include "app_wireless_mic_volume_det.h"
#include "app_wireless_mic_idle_activity.h"
#include "apps_config_event_list.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_events_key_event.h"
#include "apps_debug.h"

#include "bt_sink_srv_ami.h"

#define LOG_TAG "[APP_WIRELESS_MIC][VOL]"

typedef struct {
    bool                  is_start;
    bool                  is_threshold;
    int32_t               volume_threshold;
    audio_scenario_type_t volume_type;
    uint32_t              volume_len;
} app_wireless_mic_volume_det_cnt;

static app_wireless_mic_volume_det_cnt s_wireless_mic_volume_det_cnt;

void app_wireless_mic_volume_det_start(audio_scenario_type_t type)
{
    if (s_wireless_mic_volume_det_cnt.is_start) {
        APPS_LOG_MSGID_I(LOG_TAG" volume_det_started", 0);
        return;
    }
    s_wireless_mic_volume_det_cnt.is_start = true;
    s_wireless_mic_volume_det_cnt.volume_type = type;
    s_wireless_mic_volume_det_cnt.volume_len  = 1;
    s_wireless_mic_volume_det_cnt.volume_threshold = audio_volume_monitor_get_effective_threshold_db();
    //audio_spectrum_meter_start(type, s_wireless_mic_volume_det_cnt.volume_len, NULL);
    audio_volume_monitor_param_t param = {0};
    param.cb = NULL;
    param.volume_len = 1;
    param.ch         = 1;
    param.user_data  = NULL;
    audio_volume_monitor_start(type, &param);
    app_wireless_mic_volume_det_send_data();
    APPS_LOG_MSGID_I(LOG_TAG" volume_det_start", 0);
}

void app_wireless_mic_volume_det_stop(void)
{
    if (!s_wireless_mic_volume_det_cnt.is_start) {
        APPS_LOG_MSGID_I(LOG_TAG" volume_det_stopped", 0);
        return;
    }
    s_wireless_mic_volume_det_cnt.is_start = false;
    audio_volume_monitor_stop(s_wireless_mic_volume_det_cnt.volume_type);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_WIRELESS_MIC_VOLUME_DET);
    if (s_wireless_mic_volume_det_cnt.is_threshold) {
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0, NULL, 0);
        s_wireless_mic_volume_det_cnt.is_threshold = false;
    }
    APPS_LOG_MSGID_I(LOG_TAG" volume_det_stop", 0);
}

void app_wireless_mic_volume_det_send_data(void)
{
    uint32_t volume_data = 0;
    uint32_t data_len    = 1;
    int32_t send_volume_data = 0;
    audio_volume_monitor_get_data(s_wireless_mic_volume_det_cnt.volume_type, &volume_data, &data_len);
    if (data_len == 1) {
        send_volume_data = *(volatile int32_t *)volume_data;
        s_wireless_mic_volume_det_cnt.volume_len = data_len;
#ifdef MTK_RACE_CMD_ENABLE
        app_wireless_mic_idle_send_tx_status(APPS_EVENTS_TX_VOLUME_STATUS, &send_volume_data, sizeof(send_volume_data));
#endif
        if (send_volume_data > s_wireless_mic_volume_det_cnt.volume_threshold) {
            if (!s_wireless_mic_volume_det_cnt.is_threshold) {
                apps_config_set_background_led_pattern(APP_WIRELESS_MIC_VOLUME_DET_THRESHOLD_LED, false, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
                s_wireless_mic_volume_det_cnt.is_threshold = true;
            }
        } else {
            if (s_wireless_mic_volume_det_cnt.is_threshold) {
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0, NULL, 0);
                s_wireless_mic_volume_det_cnt.is_threshold = false;
            }
        }
    }

    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST,
                        EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_WIRELESS_MIC_VOLUME_DET,
                        NULL, 0, NULL, APP_WIRELESS_MIC_VOLUME_DET_INTERVAL);
    APPS_LOG_MSGID_I(LOG_TAG" volume_det_send_data: %d", 1, send_volume_data);
}
#endif
