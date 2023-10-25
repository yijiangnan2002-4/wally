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
 * File: app_preproc_activity.c
 *
 * Description: This activity is used to pre-process events before other activities.
 *
 * Note: See doc/AB158X_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */

#include "app_preproc_activity.h"
#include "apps_config_led_manager.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "bt_app_common.h"
#include "apps_debug.h"
#include "apps_config_key_remapper.h"
#include "apps_events_key_event.h"
#include "app_preproc_sys_pwr.h"
#include "app_bt_state_service.h"
#include "apps_customer_config.h"

//#if defined(AIR_ULL_DONGLE_LINE_IN_ENABLE) || defined(AIR_ULL_DONGLE_LINE_OUT_ENABLE)
#include "apps_events_line_in_event.h"
//#endif
#if defined(AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE)
#include "apps_events_i2s_in_event.h"
#endif

#ifdef AIRO_KEY_EVENT_ENABLE
static const uint8_t captouch_keys[] = APPS_CAPTOUCH_KEY_IDS;
#endif

#define LOG_TAG "[pre-proc]"

#if (defined(AIR_USB_AUDIO_ENABLE) && ! defined(APPS_USB_AUDIO_SUPPORT))|| defined (AIR_USB_AUDIO_1_MIC_ENABLE)
#include "apps_events_usb_event.h"
/* Work around to fix too many messages. Use a ping-pong solution. */
apps_usb_interface_enable_app_task_recorder_t s_usb_enable_recorder_last_processed[APPS_USB_EVENTS_INTERFACE_MAX];
apps_usb_interface_enable_app_task_recorder_t s_usb_enable_recorder_new[APPS_USB_EVENTS_INTERFACE_MAX];
#endif

static bool _proc_ui_shell_group(ui_shell_activity_t *self,
                                 uint32_t event_id,
                                 void *extra_data,
                                 size_t data_len)
{
    bool ret = true;

    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE:
//#if defined(AIR_ULL_DONGLE_LINE_IN_ENABLE) || defined(AIR_ULL_DONGLE_LINE_OUT_ENABLE)
            app_events_line_in_det_init();
//#endif
            break;
        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY:
            APPS_LOG_MSGID_I("app_preproc_activity destroy", 0);
            break;
        case EVENT_ID_SHELL_SYSTEM_ON_RESUME:
            APPS_LOG_MSGID_I("app_preproc_activity resume", 0);
            break;
        case EVENT_ID_SHELL_SYSTEM_ON_PAUSE:
            APPS_LOG_MSGID_I("app_preproc_activity pause", 0);
            break;
        case EVENT_ID_SHELL_SYSTEM_ON_REFRESH:
            APPS_LOG_MSGID_I("app_preproc_activity refresh", 0);
            break;
        case EVENT_ID_SHELL_SYSTEM_ON_RESULT:
            APPS_LOG_MSGID_I("app_preproc_activity result", 0);
            break;
        default:
            break;
    }
    return ret;
}

#ifdef AIRO_KEY_EVENT_ENABLE
static bool pre_proc_key_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    if (extra_data) {
        uint8_t key_id;
        airo_key_event_t key_event;
        bool is_captouch = false;
        /* The pointer is from p_key_action in apps_events_key_event.c, content is same as s_press_from_power_on. */
        uint16_t *p_key_action = (uint16_t *)extra_data;
        if (INVALID_KEY_EVENT_ID == event_id) {
            /* Key event from CMD, not real key. */
            APPS_LOG_MSGID_I("Receive CMD key event, action: %04x", 1, *p_key_action);
            return false;
        }
        /* The key is from power on, ignore it. */
        if (*p_key_action) {
            APPS_LOG_MSGID_I("The key pressed from power on, do special %04x", 1, event_id);
            return true;
        }

        /* The extra_data in the key event is valid key_action. */
        app_event_key_event_decode(&key_id, &key_event, event_id);
        for (uint8_t i = 0; i < sizeof(captouch_keys); i++) {
            if (captouch_keys[i] == key_id) {
                is_captouch = true;
                break;
            }
        }

        if (is_captouch) {
            /* For QA testing, use power key table to implement . */
            key_id = DEVICE_KEY_POWER;
        }
        *p_key_action = apps_config_key_event_remapper_map_action(key_id, key_event);
        #if defined(AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE)
        if (*p_key_action == KEY_I2S_IN_SWITCH) {
            static bool s_i2s_in_sta = false;
            s_i2s_in_sta = !s_i2s_in_sta;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_I2S_IN,
                APPS_EVENTS_I2S_IN_STATUS_CHANGE, (void *)&s_i2s_in_sta, 0, NULL, 0);
        }
        #endif
    }
    return false;
}
#endif

static bool pre_proc_app_interaction_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        /* Increase BLE ADV interval and restart advertise, see bt_app_common_ble_adv_timer_hdlr in bt_app_common.c. */
        case APPS_EVENTS_INTERACTION_INCREASE_BLE_ADV_INTERVAL:
            bt_app_common_trigger_increase_ble_adv();
            ret = true;
            break;
        /* Reload key_remaper when NVKEY changed, see bt_race_reload_nvkey_event_callback in apps_events_bt_event.c. */
        case APPS_EVENTS_INTERACTION_RELOAD_KEY_ACTION_FROM_NVKEY:
            apps_config_key_remaper_init_configurable_table();
            ret = true;
            break;
        case APPS_EVENTS_INTERACTION_SIMULATE_TIMER: {
            if (extra_data) {
                typedef void (*simulate_timer_callback_t)(void);
                simulate_timer_callback_t timer_callback = (simulate_timer_callback_t)extra_data;
                timer_callback();
            }
            ret = true;
        }
        default:
            break;
    }

    return ret;
}


static bool pre_proc_led_manager_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        /* Check and disable foreground LED when timeout, see _led_fg_time_out_callback in apps_config_led_manager.c. */
        case APPS_EVENTS_LED_FG_PATTERN_TIMEOUT:
            apps_config_check_foreground_led_pattern();
            ret = true;
            break;
#ifdef MTK_AWS_MCE_ENABLE
        /* Sync LED pattern info from Agent or Partner, then handle in UI Shell task, see app_led_sync_callback in apps_config_led_manager.c. */
        case APPS_EVENTS_LED_SYNC_LED_PATTERN:
            app_config_led_sync(extra_data);
            ret = true;
            break;
#endif
        default:
            break;
    }

    return ret;
}

#if (defined(AIR_USB_AUDIO_ENABLE) && ! defined(APPS_USB_AUDIO_SUPPORT))|| defined (AIR_USB_AUDIO_1_MIC_ENABLE)
static bool pre_proc_usb_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    uint32_t extra_value = (uint32_t)extra_data;

    if (APPS_EVENTS_USB_AUDIO_PLAY == event_id || APPS_EVENTS_USB_AUDIO_STOP == event_id || APPS_EVENTS_USB_AUDIO_UNPLUG == event_id) {
        app_events_usb_port_t *p_port = (app_events_usb_port_t *) & extra_value;
        if (p_port) {
            uint8_t interface = apps_event_usb_get_interface_id_from_port_info(p_port);
            if (interface < APPS_USB_EVENTS_INTERFACE_MAX) {
                uint8_t enabled;
                uint8_t sample_size;
                uint8_t channel;
                uint32_t sample_rate;
                enabled = apps_event_usb_get_speaker_enable_info(interface, &sample_size, &channel);
                sample_rate = apps_event_usb_get_speaker_sameple_rate(interface, false);
                if (enabled == s_usb_enable_recorder_new[interface].enabled) {
                    ret = true;
                }
                s_usb_enable_recorder_new[interface].enabled = enabled;
                s_usb_enable_recorder_new[interface].sample_size = sample_size;
                s_usb_enable_recorder_new[interface].channel = channel;
                s_usb_enable_recorder_new[interface].sample_rate = sample_rate;
            }
            APPS_LOG_MSGID_I(LOG_TAG"usb_event_proc[%d], event_id = %d, ret = %d", 3, interface, event_id, ret);
        }
    } else if (APPS_EVENTS_USB_AUDIO_SAMPLE_RATE == event_id) {
        app_events_usb_sample_size_t *p_sample_size = (app_events_usb_sample_size_t *) & extra_value;
        uint8_t interface = apps_event_usb_get_interface_id_from_port_info(&p_sample_size->port);
        if (interface < APPS_USB_EVENTS_INTERFACE_MAX) {
            uint32_t sample_rate = apps_event_usb_get_speaker_sameple_rate(interface, true);
            s_usb_enable_recorder_new[interface].sample_rate = sample_rate;
            if (!s_usb_enable_recorder_new[interface].enabled) {
                ret = true;
            }
        }
        APPS_LOG_MSGID_I(LOG_TAG"usb_event_proc[%d], sample_rate, ret = %d", 2, interface, ret);
    } else if (APPS_EVENTS_USB_ENABLE_CONTROL_DETECT_TIMEOUT == event_id || APPS_EVENTS_USB_ENABLE_CONTROL_FORCE_STOP_TIMEOUT == event_id) {
        uint8_t interface = (uint8_t)(uint32_t)extra_data;
        apps_event_usb_enable_control_event(event_id, interface);
    } else if (APPS_EVENTS_USB_CONFIG_DONE == event_id) {
        /* To-od */
    }

    return ret;
}

const apps_usb_interface_enable_app_task_recorder_t *app_preproc_activity_get_usb_interface_info(uint8_t interface_id)
{
    if (interface_id < APPS_USB_EVENTS_INTERFACE_MAX) {
        return &s_usb_enable_recorder_new[interface_id];
    } else {
        return NULL;
    }
}
#endif


bool app_preproc_activity_proc(ui_shell_activity_t *self,
                               uint32_t event_group,
                               uint32_t event_id,
                               void *extra_data,
                               size_t data_len)
{
    bool ret = false;

    /* APPS_LOG_MSGID_I("pre-proc receive event_group=%d event_id=0x%08x", 2, event_group, event_id); */

    switch (event_group) {
        /* UI Shell internal events. */
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        /* UI Shell APP interaction events. */
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            ret = pre_proc_app_interaction_event_proc(self, event_id, extra_data, data_len);
            break;
#ifdef AIRO_KEY_EVENT_ENABLE
        /* UI Shell key events. */
        case EVENT_GROUP_UI_SHELL_KEY:
            ret = pre_proc_key_event_proc(self, event_id, extra_data, data_len);
            break;
#endif
        /* UI Shell BT Sink events, see bt_app_common.c. */
        case EVENT_GROUP_UI_SHELL_BT_SINK:
            ret = bt_app_common_sink_event_proc(event_id, extra_data, data_len);
            break;
        /* UI Shell BT CM events, see bt_app_common.c. */
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER:
            ret = bt_app_common_cm_event_proc(event_id, extra_data, data_len);
            break;
        /* UI Shell GSound events, see bt_app_common.c. */
        case EVENT_GROUP_UI_SHELL_GSOUND:
            ret = bt_app_common_gsound_event_proc(event_id, extra_data, data_len);
            break;
        /* UI Shell LED manager events. */
        case EVENT_GROUP_UI_SHELL_LED_MANAGER:
            ret = pre_proc_led_manager_event_proc(self, event_id, extra_data, data_len);
            break;
        /* UI Shell system_power events. */
        case EVENT_GROUP_UI_SHELL_SYSTEM_POWER:
            //ret = sys_pwr_component_event_proc(self, event_id, extra_data, data_len);
            break;
#if defined(AIR_USB_AUDIO_ENABLE)
        case EVENT_GROUP_UI_SHELL_USB_AUDIO:
            ret = pre_proc_usb_event_proc(self, event_id, extra_data, data_len);
            break;
#endif
        default:
            break;
    }
    if (!ret) {
        /* Handle again if ret is not TRUE, see app_bt_state_service.c. */
        ret = app_bt_state_service_process_events(event_group, event_id, extra_data, data_len);
    }
    return ret;
}
