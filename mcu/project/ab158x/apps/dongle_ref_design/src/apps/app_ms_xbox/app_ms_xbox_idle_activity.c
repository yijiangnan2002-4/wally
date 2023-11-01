
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
#include "apps_events_interaction_event.h"
#include "apps_config_event_list.h"
#include "apps_dongle_sync_event.h"
#include "apps_debug.h"
#include "app_dongle_service.h"
#include "usb_main.h"
#include "bt_connection_manager.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "ui_shell_manager.h"
#include "app_ms_xbox_nvkey_struct.h"
#ifdef AIR_MS_GIP_ENABLE

#include "ms_gip_dongle.h"
#include "apps_events_usb_event.h"

#define TAG "[MS][GIP][DONGLE][APP]"

static bool app_ms_xbox_idle_activity_handle_headset_connected();
static bool app_ms_xbox_idle_activity_handle_headset_disconnected();
static void app_ms_xbox_idle_activity_handle_headset_power_off();
static void app_ms_xbox_idle_activity_handle_battery_level_change(uint32_t new_level);
static void app_ms_xbox_idle_activity_handle_charging_status_change(bool is_charging);

/*******************************************************************************
 * Internal XBOX GIP lib implementation
 *******************************************************************************/
#define APP_XBOX_DELAY_TIME        500

static ms_gip_audio_volume_info_t s_volume_info = {0};
static ms_gip_version_t          g_gip_fw_version   = {0x0001, 0x0000, 0x0001, 0x0001, 0x01, 0x00};

static bool g_headset_connected = false;

static bool app_xbox_load_volume_info_from_nvdm(ms_gip_audio_volume_info_t *info)
{
    ms_xbox_audio_volume_info_t x_box_vi;
    uint32_t size = sizeof(ms_gip_audio_volume_info_t);
    nvkey_status_t sta = nvkey_read_data(NVID_APP_XBOX_VOL_INFO, (uint8_t *)&x_box_vi, &size);
    APPS_LOG_MSGID_I(TAG"app_xbox_load_volume_info_from_nvdm read result=%d", 1, sta);
    info->mic_volume = x_box_vi.mic_volume;
    info->spk_volume = x_box_vi.spk_volume;
    info->sidetone_volume = x_box_vi.sidetone_volume;
    info->gaming_chat_balance = x_box_vi.gaming_chat_balance;
    info->is_mic_mute = x_box_vi.is_mic_mute;
    info->is_spk_mute = x_box_vi.is_spk_mute;
    info->is_sidetone_en = x_box_vi.is_sidetone_en;
    return sta == NVKEY_STATUS_OK;
}

static bool app_xbox_storage_volume_info_to_nvdm(ms_gip_audio_volume_info_t *info)
{
    ms_xbox_audio_volume_info_t x_box_vi;
    x_box_vi.mic_volume = info->mic_volume;
    x_box_vi.spk_volume = info->spk_volume;
    x_box_vi.sidetone_volume = info->sidetone_volume;
    x_box_vi.gaming_chat_balance = info->gaming_chat_balance;
    x_box_vi.is_mic_mute = info->is_mic_mute;
    x_box_vi.is_spk_mute = info->is_spk_mute;
    x_box_vi.is_sidetone_en = info->is_sidetone_en;
    nvkey_status_t sta = nvkey_write_data(NVID_APP_XBOX_VOL_INFO, (const uint8_t *)&x_box_vi, sizeof(ms_gip_audio_volume_info_t));
    APPS_LOG_MSGID_I(TAG"app_xbox_storage_volume_info_to_nvdm write result=%d", 1, sta);
    return sta == NVKEY_STATUS_OK;
}

static void app_xbox_volume_info_cb(ms_gip_audio_volume_info_t *info, uint8_t which)
{
    s_volume_info = *info;

    APPS_LOG_MSGID_I(TAG"app_xbox_volume_info_cb, which : 0x%02x, mic_mute : %d, speaker_mute : %d, sidetone_enable : %d, mic_volume : %d, speaker_volume : %d, sidetone_volume : %d, balance : %d",
                     8, which, s_volume_info.is_mic_mute, s_volume_info.is_spk_mute, s_volume_info.is_sidetone_en,
                     s_volume_info.mic_volume, s_volume_info.spk_volume, s_volume_info.sidetone_volume, s_volume_info.gaming_chat_balance);

    uint32_t new_volume_status = 0;
    uint8_t change_which = 0;

    app_xbox_storage_volume_info_to_nvdm(info);
    if (s_volume_info.is_mic_mute == true) {
        new_volume_status |= APP_DONGLE_SERVICE_MIC_MUTE;
    }
    if (s_volume_info.is_spk_mute == true) {
        new_volume_status |= APP_DONGLE_SERVICE_SPEAKER_MUTE;
    }
    if (s_volume_info.is_sidetone_en == true) {
        new_volume_status |= APP_DONGLE_SERVICE_SIDE_TONE_ENABLE;
    }
    if (s_volume_info.mic_volume == 100) {
        new_volume_status |= APP_DONGLE_SERVICE_MIC_VOLUME_MAX;
    } else if (s_volume_info.mic_volume == 0) {
        new_volume_status |= APP_DONGLE_SERVICE_MIC_VOLUME_MIN;
    }
    if (s_volume_info.spk_volume == 100) {
        new_volume_status |= APP_DONGLE_SERVICE_SPEAKER_VOLUME_MAX;
    } else if (s_volume_info.spk_volume == 0) {
        new_volume_status |= APP_DONGLE_SERVICE_SPEAKER_VOLUME_MIN;
    }
    if (s_volume_info.sidetone_volume == 100) {
        new_volume_status |= APP_DONGLE_SERVICE_SIDE_TONE_MAX;
    } else if (s_volume_info.sidetone_volume == 0) {
        new_volume_status |= APP_DONGLE_SERVICE_SIDE_TONE_MIN;
    }
    if (s_volume_info.gaming_chat_balance == 100) {
        new_volume_status |= APP_DONGLE_SERVICE_GAME_CHAT_BALANCE_MAX;
    } else if (s_volume_info.gaming_chat_balance == 0) {
        new_volume_status |= APP_DONGLE_SERVICE_GAME_CHAT_BALANCE_MIN;
    }

    switch (which) {
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_MIC_UP:
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_MIC_DOWN: {
            change_which = APP_DONGLE_SERVICE_VOLUME_CHANGE_MIC;
        }
        break;
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_SPK_UP:
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_SPK_DOWN: {
            change_which = APP_DONGLE_SERVICE_VOLUME_CHANGE_SPEAKER;
        }
        break;
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_SIDETONE_UP:
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_SIDETONE_DOWN: {
            change_which = APP_DONGLE_SERVICE_VOLUME_CHANGE_SIDETONE;
        }
        break;
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_BALANCE_UP:
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_BALANCE_DOWN: {
            change_which = APP_DONGLE_SERVICE_VOLUME_CHANGE_BALANCE;
        }
        break;
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_MIC_MUTE:
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_MIC_UN_MUTE: {
            change_which = APP_DONGLE_SERVICE_VOLUME_CHANGE_MIC_MUTE;
        }
        break;
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_SPK_MUTE:
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_SPK_UN_MUTE: {
            change_which = APP_DONGLE_SERVICE_VOLUME_CHANGE_SPEAKER_MUTE;
        }
        break;
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_SIDETONE_ENABLE:
        case MS_GIP_AUDIO_VOLUME_CHANGE_EVENT_SIDETONE_DISABLE: {
            change_which = APP_DONGLE_SERVICE_VOLUME_CHANGE_SIDETONE_ENABLE;
        }
        break;
        default: {
            change_which = 0;
        }
        break;
    }

    APPS_LOG_MSGID_I(TAG"app_xbox_volume_info_cb: new_volume_status : 0x%04x, which : %d", 2, new_volume_status, change_which);

    if (new_volume_status != 0) {
        app_dongle_service_update_volume_status(new_volume_status, change_which);
    }
}

static void app_xbox_event_cb(ms_gip_event_t event)
{
    if (event == MS_GIP_EVENT_NOTIFY_STARTUP_FAIL) {
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_XBOX,
                            XBOX_EVENT_NOTIFY_STARTUP_FAIL, NULL, 0, NULL, 0);
    } else if (event == MS_GIP_EVENT_NOTIFY_OFF) {
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_XBOX,
                            XBOX_EVENT_NOTIFY_OFF, NULL, 0, NULL, 0);
    } else {
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_XBOX,
                            XBOX_EVENT_STATE_CHANGE, (void *)event, 0, NULL, 0);
    }
}

static ms_gip_device_battery_level_t app_gip_calculate_battery_level(uint32_t battery_percent)
{
    /* The range of battery_percent is 0~100.*/
    if (battery_percent >= 90) {
        return GIP_DEVICE_BATTERY_LEVEL_FULL;
    } else if (battery_percent >= 50) {
        return GIP_DEVICE_BATTERY_LEVEL_MEDIUM;
    } else if (battery_percent >= 25) {
        return GIP_DEVICE_BATTERY_LEVEL_LOW;
    } else {
        return GIP_DEVICE_BATTERY_LEVEL_CRITICALLY_LOW;
    }
}

static void app_xbox_get_headset_battery_info_cb(ms_gip_headset_battery_info_t *battery_info)
{
    if (app_dongle_service_is_headset_charging()) {
        battery_info->charge_status = GIP_DEVICE_CHARGING;
    }
    uint32_t battery_percent       = app_dongle_service_get_headset_battery_level();
    battery_info->battery_level    = app_gip_calculate_battery_level(battery_percent);
    battery_info->battery_type     = GIP_DEVICE_BATTERY_STANDARD;
    battery_info->power_level      = app_dongle_service_is_power_off() ? GIP_DEVICE_POWER_OFF : GIP_DEVICE_POWER_FULL;
    APPS_LOG_MSGID_I(TAG"app_xbox_get_headset_battery_info_cb: battery_level=%d, battery_type=%d, charge_status=%d, power_level=%d",
                     4, battery_info->battery_level, battery_info->battery_type,
                     battery_info->charge_status, battery_info->power_level);
}

static void app_xbox_init()
{
    ms_gip_init_parameter_t init_param = {
        g_gip_fw_version,
        {
            .uplink_audio_format = MS_GIP_AUDIO_FORMAT_16KHZ_1_CHANNEL,
            .dnlink_audio_format = MS_GIP_AUDIO_FORMAT_48KHZ_2_CHANNELS,
            .uplink_audio_frame_size = 32,
            .dnlink_audio_frame_size = 192,
            .spk_mute = false,
            .mic_mute = false,
            .default_spk_volume = 100,
            .default_mic_volume = 100,
            .default_sidetone_volume = 100,
            .default_gaming_chat_ratio = 50,
            .spk_volume_writeable = true,
            .mic_volume_writeable = false,
            .sidetone_volume_writeable = false,
            .gaming_chat_ratio_writeable = false,
        },
        MS_GIP_BT_ULL_DISCONNECT_TIMER
    };

    ms_gip_audio_volume_info_t vol_info;
    if (app_xbox_load_volume_info_from_nvdm(&vol_info)) {
        init_param.audio_cfg.default_spk_volume = vol_info.spk_volume;
        init_param.audio_cfg.default_mic_volume = vol_info.mic_volume;
        init_param.audio_cfg.default_sidetone_volume = vol_info.sidetone_volume;
        init_param.audio_cfg.default_gaming_chat_ratio = vol_info.gaming_chat_balance;
        init_param.audio_cfg.spk_mute = vol_info.is_spk_mute;
        init_param.audio_cfg.mic_mute = vol_info.is_mic_mute;
    }

    APPS_LOG_MSGID_I(TAG"xbox core dongle init, sidetone volume is=%d", 1, init_param.audio_cfg.default_sidetone_volume);
    ms_gip_callback_t app_callbacks = {
        app_xbox_event_cb,
        app_xbox_volume_info_cb,
        app_xbox_get_headset_battery_info_cb
    };

    uint8_t ret = ms_gip_dongle_init(init_param, app_callbacks);
    APPS_LOG_MSGID_I(TAG"xbox core dongle init ret = %d", 1, ret);
}

static bool _proc_bt_ull_data_event(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = true;

    apps_dongle_event_sync_info_t *pkg = (apps_dongle_event_sync_info_t *)extra_data;
    if (pkg->event_group == EVENT_GROUP_UI_SHELL_KEY) {
        apps_config_key_action_t action = (apps_config_key_action_t)pkg->event_id;
        switch (action) {
            case KEY_VOICE_UP:
            case KEY_VOICE_DN: {
                ms_gip_action_t g_action = ((action == KEY_VOICE_UP) ? MS_GIP_ACTION_SPEAKER_VOLUME_UP : MS_GIP_ACTION_SPEAKER_VOLUME_DOWN);
                uint32_t ret = ms_gip_dongle_execute(g_action, NULL, 0);
                APPS_LOG_MSGID_I(TAG"receive volume ctrl action=%d, do action=%d ret=%d", 3, action, g_action, ret);
                break;
            }

            case KEY_AUDIO_MIX_RATIO_GAME_ADD:
            case KEY_AUDIO_MIX_RATIO_CHAT_ADD: {
                uint8_t new_value = s_volume_info.gaming_chat_balance;
                if (action == KEY_AUDIO_MIX_RATIO_GAME_ADD) {
                    new_value = s_volume_info.gaming_chat_balance >= 100 ? 100 : s_volume_info.gaming_chat_balance + 1;
                } else {
                    new_value = s_volume_info.gaming_chat_balance > 0 ? s_volume_info.gaming_chat_balance - 1 : 0;
                }
                uint32_t ret = ms_gip_dongle_execute(MS_GIP_ACTION_GAMING_CHAT_BALANCE, &new_value, sizeof(uint8_t));
                APPS_LOG_MSGID_I(TAG"receive mix ratio action=%d, new value=%d ret=%d", 3, action, new_value, ret);
                break;
            }

            case KEY_AUDIO_SIDE_TONE_VOLUME_UP:
            case KEY_AUDIO_SIDE_TONE_VOLUME_DOWN: {
                ms_gip_action_t g_action = ((action == KEY_AUDIO_SIDE_TONE_VOLUME_UP) ? MS_GIP_ACTION_SIDETONE_VOLUME_UP : MS_GIP_ACTION_SIDETONE_VOLUME_DOWN);
                uint32_t ret = ms_gip_dongle_execute(g_action, NULL, 0);
                APPS_LOG_MSGID_I(TAG"receive sidetone volume ctrl action=%d, do action=%d ret=%d", 3, action, g_action, ret);
                break;
            }
            default:
                ret = false;
                break;
        }
    } else {
        ret = false;
    }

    return ret;
}

static bool _proc_xbox_core_event(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = true;
    switch (event_id) {
        case XBOX_EVENT_STATE_CHANGE: {
            uint8_t state = (uint8_t)(uint32_t)extra_data;
            switch (state) {
                case MS_GIP_EVENT_STATE_IDLE: {
                    // ToDo, waiting BT ULL connection
                    APPS_LOG_MSGID_I(TAG" XBOX Core IDLE state", 0);
                    // APPS_LOG_MSGID_I(TAG" XBOX Core IDLE state, try BT ON", 0);
                    // ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                    //                           EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                    //                           APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT,
                    //                           (void *)TRUE, 0, NULL, 0);
                    break;
                }

                case MS_GIP_EVENT_STATE_STARTUP: {
                    // ToDo, XBOX LED ON
                    APPS_LOG_MSGID_I(TAG" XBOX Core STARTUP state", 0);
                    break;
                }

                case MS_GIP_EVENT_STATE_READY: {
                    APPS_LOG_MSGID_I(TAG" XBOX Core READY state", 0);
                    app_dongle_service_update_dongle_mode(APP_DONGLE_SERVICE_DONGLE_MODE_XBOX);
                    break;
                }

                case MS_GIP_EVENT_STATE_RESET: {
                    APPS_LOG_MSGID_I(TAG" XBOX Core RESET state", 0);
                    app_dongle_service_update_dongle_mode(APP_DONGLE_SERVICE_DONGLE_MODE_NONE);
                    app_dongle_service_notify_reset_state();
                    ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                              EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                              APPS_EVENTS_INTERACTION_REQUEST_REBOOT,
                                              NULL, 0, NULL, APP_XBOX_DELAY_TIME);
                    break;
                }

                case MS_GIP_EVENT_STATE_USB_SUSPEND: {
                    APPS_LOG_MSGID_I(TAG" XBOX Core USB_SUSPEND state", 0);
                    app_dongle_service_update_dongle_mode(APP_DONGLE_SERVICE_DONGLE_MODE_NONE);
                    // ToDo, power saving?, wait USB resume and connection complete
                    // ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                    //                           EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                    //                           APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT,
                    //                           (void *)FALSE, 0, NULL, 0);
                    break;
                }

                default:
                    break;
            }
            break;
        }

        case XBOX_EVENT_NOTIFY_STARTUP_FAIL: {
            // Show fail foreground LED?, Notify to headset?
            APPS_LOG_MSGID_I(TAG" XBOX event notify STARTUP_FAIL", 0);
            break;
        }

        case XBOX_EVENT_NOTIFY_OFF: {
            // Not dongle power off, received "set_device_state - OFF" from XBOX, need to disconnect ULL??
            app_dongle_service_notify_off_state();
            APPS_LOG_MSGID_I(TAG" XBOX event Notify OFF", 0);
            break;
        }

        default:
            break;
    }
    return ret;
}

#ifdef AIR_USB_ENABLE
static bool app_ms_xbox_proc_usb_event(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case APPS_EVENTS_USB_AUDIO_SUSPEND: {
            ms_gip_dongle_execute(MS_GIP_ACTION_USB_SUSPEND, NULL, 0);
        }
        break;
        case APPS_EVENTS_USB_CONFIG_DONE: {
            APPS_LOG_MSGID_I(TAG" USB configure done, do RESUME action", 0);
            ms_gip_dongle_execute(MS_GIP_ACTION_USB_RESUME, NULL, 0);
        }
        break;
        default:
            break;
    }
    return false;
}
#endif /* AIR_USB_ENABLE */

static void app_ms_xbox_idle_register_app_service_callback()
{
    app_dongle_service_callback_t callback;

    callback.headset_connected = app_ms_xbox_idle_activity_handle_headset_connected;
    callback.headset_disconnected = app_ms_xbox_idle_activity_handle_headset_disconnected;
    callback.headset_battery_charging_changed = app_ms_xbox_idle_activity_handle_charging_status_change;
    callback.headset_battery_level_changed = app_ms_xbox_idle_activity_handle_battery_level_change;
    callback.headset_headset_power_off = app_ms_xbox_idle_activity_handle_headset_power_off;

    app_dongle_service_register_callback(callback);
}

static bool _proc_ui_shell_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(TAG" create", 0);
            app_ms_xbox_idle_register_app_service_callback();
            app_xbox_init();
        }
        break;
        default:
            break;
    }
    return ret;
}

bool app_ms_xbox_idle_activity_proc(ui_shell_activity_t *self,
                                    uint32_t event_group,
                                    uint32_t event_id,
                                    void *extra_data,
                                    size_t data_len)
{
    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_DONGLE_DATA: {
            ret = _proc_bt_ull_data_event(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_XBOX: {
            ret = _proc_xbox_core_event(self, event_id, extra_data, data_len);
            break;
        }
#ifdef AIR_USB_ENABLE
        case EVENT_GROUP_UI_SHELL_USB_AUDIO: {
            ret = app_ms_xbox_proc_usb_event(self, event_id, extra_data, data_len);
        }
        break;
#endif /* AIR_USB_ENABLE */
        case EVENT_GROUP_UI_SHELL_APP_SERVICE: {
            if (event_id == APP_DONGLE_SERVICE_MODE_SWITCH_EVENT) {
                if (g_headset_connected == true) {
                    APPS_LOG_MSGID_I(TAG" Dongle mode switch happen and headset connected, need send power off event", 0);
                    ms_gip_dongle_execute(MS_GIP_ACTION_DONGLE_POWER_OFF, NULL, 0);
                }
            }
        }
        break;
        default: {
            break;
        }
    }

    return ret;
}


bool app_ms_xbox_idle_activity_handle_headset_connected()
{
    /**
     * TODO Handle the headset connected
     * Maybe start handshake with host.
     */
    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
        g_headset_connected = true;
        APPS_LOG_MSGID_I(TAG" Connected with Headset", 0);
        #ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        uint8_t headset_type = app_dongle_service_get_headset_type();
        uint8_t mic_audio_format = 0;
        if (headset_type == APP_DONGLE_SERVICE_HEADSET_TYPE_8X) {
            mic_audio_format = MS_GIP_AUDIO_FORMAT_16KHZ_1_CHANNEL;
        } else {
            mic_audio_format = MS_GIP_AUDIO_FORMAT_16KHZ_1_CHANNEL;
        }
        ms_gip_dongle_execute(MS_GIP_ACTION_MIC_SET_FORMAT, &mic_audio_format, sizeof(uint8_t));
        #endif
        return ms_gip_dongle_execute(MS_GIP_ACTION_BT_HEADSET_CONNECTED, NULL, 0);
    }
    return true;
}

bool app_ms_xbox_idle_activity_handle_headset_disconnected()
{
    /**
     * TODO Handle the headset disconnected
     *
     */
    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
        g_headset_connected = false;
        APPS_LOG_MSGID_I(TAG" Disconnected with Headset", 0);
        return ms_gip_dongle_execute(MS_GIP_ACTION_BT_HEADSET_DISCONNECTED, NULL, 0);
    }
    return true;
}

void app_ms_xbox_idle_activity_handle_headset_power_off()
{
    /**
     * TODO need handle the headset power off flow.
     *
     */
    APPS_LOG_MSGID_I(TAG" Handle Headset power_off", 0);
    ms_gip_dongle_execute(MS_GIP_ACTION_HEADSET_POWER_OFF, NULL, 0);
}

void app_ms_xbox_idle_activity_handle_battery_level_change(uint32_t new_level)
{
    /**
     * TODO handle the headset battery level changed operation
     *
     */
}

void app_ms_xbox_idle_activity_handle_charging_status_change(bool is_charging)
{
    /**
     * TODO handle the headset battery charging status changed operation
     *
     */
}

#endif /* AIR_MS_GIP_ENABLE */

