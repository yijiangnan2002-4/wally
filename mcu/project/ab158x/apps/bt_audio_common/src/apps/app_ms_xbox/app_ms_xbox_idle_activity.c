
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

/**
 * File: app_ms_xbox_idle_activity.c
 *
 * Description: This file is the activity to handle xbox gip event.
 *
 */

#include "app_ms_xbox_idle_activity.h"
#include "apps_events_event_group.h"
#include "apps_config_event_list.h"
#include "apps_dongle_sync_event.h"
#include "apps_debug.h"
#include "bt_connection_manager.h"
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "app_dongle_service.h"
#endif /* AIR_BLE_ULTRA_LOW_LATENCY_ENABLE */
#include "apps_config_vp_index_list.h"
#include "voice_prompt_api.h"
#include "app_preproc_activity.h"
#include "ms_gip_headset.h"

#ifdef AIR_MS_GIP_ENABLE

#define TAG "[MS][GIP][HEADSET][APP]"

typedef struct {
    app_dongle_service_dongle_mode_t           ctx_dongle_mode;
    uint32_t                        ctx_dongle_volume_status;
} app_ms_xbox_context_t;

static void app_ms_xbox_idle_activity_dongle_mode_update(app_dongle_service_dongle_mode_t new_mode);
static void app_ms_xbox_idle_activity_dongle_connected();
static void app_ms_xbox_idle_activity_dongle_disconnected();
static void app_ms_xbox_idle_activity_dongle_volume_status_change(app_srv_volume_change_t *new_volume_status);

static app_ms_xbox_context_t    s_xbox_ctx = {0};

static void app_ms_xbox_idle_activity_register_callback()
{
    app_dongle_service_callback_t callback;

    callback.dongle_connected = app_ms_xbox_idle_activity_dongle_connected;
    callback.dongle_disconnected = app_ms_xbox_idle_activity_dongle_disconnected;
    callback.dongle_mode_changed = app_ms_xbox_idle_activity_dongle_mode_update;
    callback.dongle_off = NULL;
    callback.dongle_reset = NULL;
    callback.dongle_volume_status_changed = app_ms_xbox_idle_activity_dongle_volume_status_change;

    app_dongle_service_register_callback(callback);
}

bool app_ms_xbox_idle_activity_proc(
    struct _ui_shell_activity *self,
    uint32_t event_group,
    uint32_t event_id,
    void *extra_data,
    size_t data_len)
{
    if (event_group == EVENT_GROUP_UI_SHELL_SYSTEM && event_id == EVENT_ID_SHELL_SYSTEM_ON_CREATE) {
        s_xbox_ctx.ctx_dongle_mode = APP_DONGLE_SERVICE_DONGLE_MODE_NONE;
        s_xbox_ctx.ctx_dongle_volume_status = APP_DONGLE_SERVICE_VOLUME_NONE;
        app_ms_xbox_idle_activity_register_callback();

        ms_gip_headset_init();
        return true;
    }
    return false;
}

void app_ms_xbox_idle_activity_dongle_mode_update(app_dongle_service_dongle_mode_t new_mode)
{
    if (s_xbox_ctx.ctx_dongle_mode != new_mode) {
        s_xbox_ctx.ctx_dongle_mode = new_mode;
    }
}

void app_ms_xbox_idle_activity_dongle_connected()
{
    APPS_LOG_MSGID_I(TAG"Dongle connected", 0);
    ms_gip_headset_execute(MS_GIP_HEADSET_ACTION_DONGLE_CONNECTED, NULL, 0);
}

void app_ms_xbox_idle_activity_dongle_disconnected()
{
    APPS_LOG_MSGID_I(TAG"Dongle disconnected", 0);
    ms_gip_headset_execute(MS_GIP_HEADSET_ACTION_DONGLE_DISCONNECTED, NULL, 0);
}

void app_ms_xbox_idle_activity_dongle_volume_status_change(app_srv_volume_change_t *new_volume_status)
{
    APPS_LOG_MSGID_I(TAG"Dongle volume status changed, old : 0x%04x, new : 0x%04x, which : 0x%02x", 3,
                     s_xbox_ctx.ctx_dongle_volume_status, new_volume_status->new_volume, new_volume_status->which_changed);
    voice_prompt_play_vp_press();
    s_xbox_ctx.ctx_dongle_volume_status = new_volume_status->new_volume;
}

#endif /* AIR_MS_GIP_ENABLE */
