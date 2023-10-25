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
 * File: app_usb_audio_idle_utils.c
 *
 * Description:
 * This file define the common function of usb audio.
 */

#include "apps_events_usb_event.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "usbaudio_drv.h"
#include "usb.h"
#include "ui_shell_manager.h"
#include "ui_shell_activity.h"
#include "apps_debug.h"
#include <string.h>
#include <stdlib.h>
#ifdef AIR_USB_HID_ENABLE
#include "usb_hid_srv.h"
#endif
#include "hal_nvic.h"

#define USB_EVENT_LOG_I(msg, ...)     APPS_LOG_MSGID_I("[USB_EVENT]"msg, ##__VA_ARGS__)
#define USB_EVENT_LOG_E(msg, ...)     APPS_LOG_MSGID_E("[USB_EVENT]"msg, ##__VA_ARGS__)
#define USB_EVENT_LOG_D(msg, ...)     APPS_LOG_MSGID_D("[USB_EVENT]"msg, ##__VA_ARGS__)

#define USB_PORT_MAXIMUN_NUMBER 3

typedef struct {
    app_usb_audio_port_t port_type;         /* spk/mic */
    uint8_t port_num;                       /* range:0 ~ 1; 0:chat,1:gaming */
    uint8_t left_volume;                    /* 0 ~ 100 */
    uint8_t right_volume;                   /* 0 ~ 100 */
    int32_t left_db;
    int32_t right_db;
} app_usb_volume_data_t;

static app_usb_volume_data_t app_usb_volume[USB_PORT_MAXIMUN_NUMBER]; /* 0:chat,1:gaming,2:mic */
static app_usb_volume_data_t app_usb_volume_cache[USB_PORT_MAXIMUN_NUMBER]; /* 0:chat,1:gaming,2:mic */

/* flag to check volume delay timer is start or not */
static volatile bool app_is_delay_timer_start = false;

#if defined(AIR_USB_AUDIO_ENABLE)
static void apps_event_usb_setinterface_cb(uint8_t interface_number, uint8_t alternate_set);
static void apps_event_usb_chat_unplug_cb(void);
#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
static void apps_event_usb_gaming_unplug_cb(void);
#endif
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
static void apps_event_usb_mic_unplug_cb(void);
#endif
static void apps_event_usb_volumechange_cb(uint8_t ep_number, uint8_t channel, uint32_t volume, int32_t db);
static void apps_event_usb_mute_cb(uint8_t ep_number, usb_audio_mute_t mute);
static void apps_event_usb_sample_rate_cb(uint8_t ep_number, uint32_t sampling_rate);
static void apps_event_usb_sample_size_cb(uint8_t ep_number, uint8_t sampling_size);
static void apps_event_usb_channel_cb(uint8_t ep_number, uint8_t channel);
#endif
static void apps_event_usb_suspend_callback(void);
static void apps_event_usb_resume_callback(void);
static void apps_event_usb_reset_callback(void);
static void app_event_usb_device_config_cb(usb_evt_t event, void* usb_data, void* user_data);

static app_usb_audio_sample_rate_t apps_event_usb_sample_rate_exchange(uint32_t sample_rate);
static void apps_event_usb_simulater_callback(void);

#ifdef AIR_USB_HID_ENABLE
static void apps_event_usb_hid_srv_event_callback(usb_hid_srv_event_t event)
{
    if(event < USB_HID_SRV_EVENT_CALL_MAX){
        ui_shell_send_event(true,
                            EVENT_PRIORITY_HIGH,
                            EVENT_GROUP_UI_SHELL_USB_HID_CALL,
                            event,
                            NULL, 0, NULL, 0);
    }
}
#endif

#if defined (AIR_PURE_GAMING_ENABLE)
bool g_usb_config_done = false;
#endif

void apps_event_usb_event_init()
{
#if defined(AIR_USB_AUDIO_ENABLE)
    /* register usb audio in0 */
    USB_Audio_Register_SetInterface_Callback(0, apps_event_usb_setinterface_cb);
    USB_Audio_Register_Unplug_Callback(0, apps_event_usb_chat_unplug_cb);
    USB_Audio_Register_VolumeChange_Callback(0, apps_event_usb_volumechange_cb);
    USB_Audio_Register_Mute_Callback(0, apps_event_usb_mute_cb);
    USB_Audio_Register_SetSamplingRate_Callback(0, apps_event_usb_sample_rate_cb);
    USB_Audio_Register_SetSampleSize_Callback(0, apps_event_usb_sample_size_cb);
    USB_Audio_Register_SetChannel_Callback(0, apps_event_usb_channel_cb);
#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
    /* register usb audio in1 */
    USB_Audio_Register_SetInterface_Callback(1, apps_event_usb_setinterface_cb);
    USB_Audio_Register_Unplug_Callback(1, apps_event_usb_gaming_unplug_cb);
    USB_Audio_Register_VolumeChange_Callback(1, apps_event_usb_volumechange_cb);
    USB_Audio_Register_Mute_Callback(1, apps_event_usb_mute_cb);
    USB_Audio_Register_SetSamplingRate_Callback(1, apps_event_usb_sample_rate_cb);
    USB_Audio_Register_SetSampleSize_Callback(1, apps_event_usb_sample_size_cb);
    USB_Audio_Register_SetChannel_Callback(1, apps_event_usb_channel_cb);
#endif

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    /* register default usb audio out. */
    USB_Audio_Register_Mic_SetInterface_Callback(0, apps_event_usb_setinterface_cb);
    USB_Audio_Register_Mic_Unplug_Callback(0, apps_event_usb_mic_unplug_cb);
    USB_Audio_Register_Mic_VolumeChange_Callback(0, apps_event_usb_volumechange_cb);
    USB_Audio_Register_Mic_Mute_Callback(0, apps_event_usb_mute_cb);
    USB_Audio_Register_Mic_SetSamplingRate_Callback(0, apps_event_usb_sample_rate_cb);
    USB_Audio_Register_Mic_SetSampleSize_Callback(0, apps_event_usb_sample_size_cb);
    USB_Audio_Register_Mic_SetChannel_Callback(0, apps_event_usb_channel_cb);
#endif

#endif /* #if defined(AIR_USB_AUDIO_ENABLE) */
    USB_Register_Suspend_Callback(apps_event_usb_suspend_callback);
    USB_Register_Resume_Callback(apps_event_usb_resume_callback);
    USB_Register_Reset_Callback(apps_event_usb_reset_callback);

    /* init all usb port volume cache data */
    app_usb_volume[0].port_type = APP_USB_AUDIO_SPK_PORT;
    app_usb_volume[0].port_num = 0;         /* chat */
    app_usb_volume[0].left_volume = 0xFF;   /* invalid volume */
    app_usb_volume[0].right_volume = 0xFF;  /* invalid volume */
    app_usb_volume[1].port_type = APP_USB_AUDIO_SPK_PORT;
    app_usb_volume[1].port_num = 1;         /* gaming */
    app_usb_volume[1].left_volume = 0xFF;   /* invalid volume */
    app_usb_volume[1].right_volume = 0xFF;  /* invalid volume */
    app_usb_volume[2].port_type = APP_USB_AUDIO_MIC_PORT;
    app_usb_volume[2].port_num = 0;         /* mic */
    app_usb_volume[2].left_volume = 0xFF;   /* invalid volume */
    app_usb_volume[2].right_volume = 0xFF;  /* invalid volume */
    /* init cache volume */
    memcpy(app_usb_volume_cache, app_usb_volume, sizeof(app_usb_volume_data_t)*USB_PORT_MAXIMUN_NUMBER);
#ifdef AIR_USB_HID_ENABLE
    usb_hid_srv_init(apps_event_usb_hid_srv_event_callback);
#endif

    usb_evt_register_cb(USB_USER_APP, USB_EVT_CONFIG_DONE, app_event_usb_device_config_cb, NULL);

#if defined (AIR_PURE_GAMING_ENABLE)
    //g_usb_config_done = false;
#endif

    USB_EVENT_LOG_I("apps_event_usb_event_init done.", 0);
}

#if defined(AIR_USB_AUDIO_ENABLE)

static void apps_event_usb_get_usb_port_info(app_events_usb_port_t *p_port, uint8_t *p_interface_id, uint8_t number, bool number_is_interface)
{
    if (p_port == NULL || p_interface_id == NULL) {
        return;
    }

    uint8_t chat_if = 0xFF, gaming_if = 0xFF, mic_if = 0xFF;
    uint8_t chat_ep = 0xFF, gaming_ep = 0xFF, mic_ep = 0xFF;
    USB_Audio_Get_Game_Info(&gaming_if, &gaming_ep);
    USB_Audio_Get_Chat_Info(&chat_if, &chat_ep, &mic_if, &mic_ep);
    if ((number_is_interface && gaming_if == number) || (!number_is_interface && gaming_ep == number)) {
        p_port->port_type = APP_USB_AUDIO_SPK_PORT;
        p_port->port_num = 1; /* gaming streaming port */
#if defined(AIR_USB_AUDIO_2_SPK_ENABLE) && defined(AIR_USB_AUDIO_ENABLE)
        *p_interface_id = APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING;
#endif
    } else if ((number_is_interface && chat_if == number) || (!number_is_interface && chat_ep == number)) {
        p_port->port_type = APP_USB_AUDIO_SPK_PORT;
        p_port->port_num = 0; /* chat streaming port */
#if defined(AIR_USB_AUDIO_ENABLE)
        *p_interface_id = APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT;
#endif
    } else if ((number_is_interface && mic_if == number) || (!number_is_interface && mic_ep == number)) {
        p_port->port_type = APP_USB_AUDIO_MIC_PORT;
        p_port->port_num = 0;
#if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
        *p_interface_id = APPS_USB_EVENTS_INTERFACE_MIC;
#endif
    } else {
        p_port->port_type = APP_USB_AUDIO_UNKNOWN_PORT;
        *p_interface_id = APPS_USB_EVENTS_INTERFACE_MAX;
        USB_EVENT_LOG_E("[Error] unknown interface, gaming_if:0x%x, chat_if:0x%x, mic_if:0x%x", 3,
                        gaming_if, chat_if, mic_if);
    }
}

uint8_t apps_event_usb_get_interface_id_from_port_info(const app_events_usb_port_t *p_port)
{
#if defined(AIR_USB_AUDIO_ENABLE)
    if (APP_USB_AUDIO_SPK_PORT == p_port->port_type) {
#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
        if (p_port->port_num == 1) {
            return APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING;
        }
#endif
        return APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT;
    }
#endif
#if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
    if (p_port->port_type == APP_USB_AUDIO_MIC_PORT) {
        return APPS_USB_EVENTS_INTERFACE_MIC;
    }
#endif
    return APPS_USB_EVENTS_INTERFACE_MAX;
}

void apps_event_usb_get_port_info_from_interface_id(app_events_usb_port_t *p_port, uint8_t interface_id)
{
#if defined(AIR_USB_AUDIO_ENABLE)
#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
    if (APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING == interface_id) {
        p_port->port_type = APP_USB_AUDIO_SPK_PORT;
        p_port->port_num = 1;
    }
#endif
    if (APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT == interface_id) {
        p_port->port_type = APP_USB_AUDIO_SPK_PORT;
        p_port->port_num = 0;
    }
#endif
#if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
    if (APPS_USB_EVENTS_INTERFACE_MIC == interface_id) {
        p_port->port_type = APP_USB_AUDIO_MIC_PORT;
        p_port->port_num = 0;
    }
#endif
}

/*========================================================================================*/
/*                          USB Audio evet callback                                       */
/*========================================================================================*/
#define APPS_EVENT_USB_ABNORMAL_DETECT_TIME         (2 * 1000)
#define APPS_EVENT_USB_ABNORMAL_COUNT               (9)
#define APPS_EVENT_USB_FORCE_STOP_TIME              (1000)

typedef enum {
    APPS_EVENT_USB_ENABLE_CONTROL_STATE_IDLE,
    APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_DETECTING,
    APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_DETECTED,
    APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_FORCE_STOPPED,
} apps_event_usb_enable_control_state_t;

/* Work around to fix too many messages. Use a ping-pong solution. */
typedef struct  {
    uint8_t enabled;
    uint8_t sample_size;
    uint8_t channel;
    uint32_t sample_rate;
} apps_event_usb_interface_enable_app_task_recorder_t;

typedef struct  {
    uint8_t enable_msg_pending;
    uint8_t sample_rate_msg_pending;
    uint8_t enabled;
    uint8_t sample_size;
    uint8_t channel;
    uint32_t sample_rate;
    uint32_t enable_callback_count;
    uint32_t sample_rate_callback_count;
    apps_event_usb_enable_control_state_t enable_event_control_state;
    uint32_t last_state_count;
} apps_event_usb_interface_enable_control_data_t;

static apps_event_usb_interface_enable_control_data_t s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_MAX];

uint8_t apps_event_usb_get_speaker_enable_info(uint8_t interface_id, uint8_t *sample_size, uint8_t *channel)
{
    uint8_t enabled;
    bool need_send_msg = false;
    uint32_t count;
    uint32_t mask;
    if (interface_id >= APPS_USB_EVENTS_INTERFACE_MAX || sample_size == NULL || channel == NULL) {
        return 0;
    }
    hal_nvic_save_and_set_interrupt_mask(&mask);
    *sample_size = s_speaker_enable_control[interface_id].sample_size;
    *channel = s_speaker_enable_control[interface_id].channel;
    if (APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_DETECTED == s_speaker_enable_control[interface_id].enable_event_control_state
            || APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_FORCE_STOPPED == s_speaker_enable_control[interface_id].enable_event_control_state) {
        if (APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_DETECTED == s_speaker_enable_control[interface_id].enable_event_control_state) {
            need_send_msg = true;
            s_speaker_enable_control[interface_id].enable_event_control_state = APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_FORCE_STOPPED;
        }
        enabled = 0;
    } else {
        enabled = s_speaker_enable_control[interface_id].enabled;
    }
    count = s_speaker_enable_control[interface_id].enable_callback_count;
    s_speaker_enable_control[interface_id].enable_msg_pending = false;
    hal_nvic_restore_interrupt_mask(mask);
    if (need_send_msg) {
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                            APPS_EVENTS_USB_ENABLE_CONTROL_FORCE_STOP_TIMEOUT, (void *)(uint32_t)interface_id, 0, NULL, APPS_EVENT_USB_FORCE_STOP_TIME);
    }
    USB_EVENT_LOG_I("get_speaker_enable_info[%d], enable: %d, sample_size: %d, channel: %d, acount: %d, enable_event_control_state: %d", 6, interface_id, enabled, *sample_size, *channel, count, s_speaker_enable_control[interface_id].enable_event_control_state);
    return enabled;
}

uint32_t apps_event_usb_get_speaker_sameple_rate(uint8_t interface_id, bool set_msg_pending)
{
    uint32_t sample_rate;
    uint32_t count;
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    sample_rate = s_speaker_enable_control[interface_id].sample_rate;
    count = s_speaker_enable_control[interface_id].sample_rate_callback_count;
    if (set_msg_pending) {
        s_speaker_enable_control[interface_id].sample_rate_msg_pending = false;
    }
    hal_nvic_restore_interrupt_mask(mask);
    USB_EVENT_LOG_I("get_speaker_sample_rate[%d], sample_rate: %d, set_msg_pending: %d, count: %d", 4, interface_id, sample_rate, set_msg_pending, count);
    return sample_rate;
}

/**
 * @brief USB interface callback
 *
 * @param interface_number      1 means speaker interface.
 *                              2 means microphone interface.
 * @param alternate_set         0 means the audio stream stoped.
 *                              1 means the audio stream started.
 * @return
 */
static void apps_event_usb_setinterface_cb(uint8_t interface_number, uint8_t alternate_set)
{
    uint32_t event_data = 0;
    app_events_usb_port_t *p_port = (app_events_usb_port_t *)&event_data;
    app_usb_audio_event_t event = APPS_EVENTS_USB_AUDIO_UNPLUG;
    uint8_t interface_id = APPS_USB_EVENTS_INTERFACE_MAX;

    USB_EVENT_LOG_I("usb_audio_setinterface_cb, if: 0x%x, value: 0x%x", 2, interface_number, alternate_set);

    apps_event_usb_get_usb_port_info(p_port, &interface_id, interface_number, true);

    if (APP_USB_AUDIO_UNKNOWN_PORT != p_port->port_type && interface_id < APPS_USB_EVENTS_INTERFACE_MAX) {
        if (0x01 == alternate_set) {
            event = APPS_EVENTS_USB_AUDIO_PLAY;
        } else {
            event = APPS_EVENTS_USB_AUDIO_STOP;
        }
        bool has_pending_msg = false;
        bool refuse_send_event_by_control = false;
        bool send_detect_timeout_event = false;
        uint32_t mask;
        hal_nvic_save_and_set_interrupt_mask(&mask);
        s_speaker_enable_control[interface_id].enabled = alternate_set == 0x01 ? 1 : 0;
        if (APPS_EVENT_USB_ENABLE_CONTROL_STATE_IDLE == s_speaker_enable_control[interface_id].enable_event_control_state) {
            send_detect_timeout_event = true;
            s_speaker_enable_control[interface_id].enable_event_control_state = APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_DETECTING;
            s_speaker_enable_control[interface_id].last_state_count = s_speaker_enable_control[interface_id].enable_callback_count;
        } else if (APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_DETECTING == s_speaker_enable_control[interface_id].enable_event_control_state) {
            if (s_speaker_enable_control[interface_id].enable_callback_count > s_speaker_enable_control[interface_id].last_state_count + APPS_EVENT_USB_ABNORMAL_COUNT) {
                s_speaker_enable_control[interface_id].enable_event_control_state = APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_DETECTED;
                s_speaker_enable_control[interface_id].last_state_count = s_speaker_enable_control[interface_id].enable_callback_count;
                /* After detected, allow send the last time msg. */
            }
        } else if (APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_DETECTED == s_speaker_enable_control[interface_id].enable_event_control_state
            || APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_FORCE_STOPPED == s_speaker_enable_control[interface_id].enable_event_control_state) {
            refuse_send_event_by_control = true;
        }
        has_pending_msg = s_speaker_enable_control[interface_id].enable_msg_pending;
        if (!refuse_send_event_by_control) {
            s_speaker_enable_control[interface_id].enable_msg_pending = true;
        }
        s_speaker_enable_control[interface_id].enable_callback_count ++;
        hal_nvic_restore_interrupt_mask(mask);
        if (send_detect_timeout_event) {
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_ENABLE_CONTROL_DETECT_TIMEOUT, (void *)(uint32_t)interface_id, 0, NULL, APPS_EVENT_USB_ABNORMAL_DETECT_TIME);
        }
        USB_EVENT_LOG_I("usb_audio_setinterface_cb[%d], value: %d, has_pending_msg: %d, count: %d", 4, interface_id, alternate_set, has_pending_msg, s_speaker_enable_control[interface_id].enable_callback_count);
        if (!has_pending_msg && !refuse_send_event_by_control) {
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                                event, (void *)event_data, 0, NULL, 0);
        }
    }
}

void apps_event_usb_enable_control_event(app_usb_audio_event_t event, uint8_t interface_id)
{
    if (interface_id >= APPS_USB_EVENTS_INTERFACE_MAX) {
        return;
    }
    bool has_pending_msg = false;
    bool force_stop_end = false;
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (APPS_EVENTS_USB_ENABLE_CONTROL_DETECT_TIMEOUT == event) {
        if (APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_DETECTING == s_speaker_enable_control[interface_id].enable_event_control_state) {
            s_speaker_enable_control[interface_id].enable_event_control_state = APPS_EVENT_USB_ENABLE_CONTROL_STATE_IDLE;
        }
    } else if (APPS_EVENTS_USB_ENABLE_CONTROL_FORCE_STOP_TIMEOUT == event) {
        if (APPS_EVENT_USB_ENABLE_CONTROL_STATE_ABNORMAL_FORCE_STOPPED == s_speaker_enable_control[interface_id].enable_event_control_state) {
            if (s_speaker_enable_control[interface_id].last_state_count == s_speaker_enable_control[interface_id].enable_callback_count) {
                s_speaker_enable_control[interface_id].enable_event_control_state = APPS_EVENT_USB_ENABLE_CONTROL_STATE_IDLE;
                has_pending_msg = s_speaker_enable_control[interface_id].enable_msg_pending;
                force_stop_end =true;
            } else {
                s_speaker_enable_control[interface_id].last_state_count = s_speaker_enable_control[interface_id].enable_callback_count;
                force_stop_end = false;
            }
        }
    }
    hal_nvic_restore_interrupt_mask(mask);

    if (APPS_EVENTS_USB_ENABLE_CONTROL_FORCE_STOP_TIMEOUT == event) {
        if (force_stop_end) {
            uint32_t event_data = 0;
            app_events_usb_port_t *p_port = (app_events_usb_port_t *)&event_data;
            apps_event_usb_get_port_info_from_interface_id(p_port, interface_id);
            if (!has_pending_msg) {
                ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                                    APPS_EVENTS_USB_AUDIO_PLAY, (void *)event_data, 0, NULL, 0);
            }
        } else {
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                                APPS_EVENTS_USB_ENABLE_CONTROL_FORCE_STOP_TIMEOUT, (void *)(uint32_t)interface_id, 0, NULL, APPS_EVENT_USB_FORCE_STOP_TIME);
        }
    }

}

/**
 * @brief   Callback when USB unplugged.
 *
 * @return  None.
 */
static void apps_event_usb_chat_unplug_event(bool from_isr)
{
    uint32_t event_data = 0;
    app_events_usb_port_t *p_port = (app_events_usb_port_t *)&event_data;
    USB_EVENT_LOG_I("USB Chat unpluged", 0);

    p_port->port_type = APP_USB_AUDIO_SPK_PORT;
    p_port->port_num = 0;
    app_usb_volume[0].left_volume = 0xFF;
    app_usb_volume[0].right_volume = 0xFF;
    bool has_pending_msg = false;
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT].enabled = 0;
    has_pending_msg = s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT].enable_msg_pending;
    s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT].enable_msg_pending = true;
    s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT].enable_callback_count ++;
    hal_nvic_restore_interrupt_mask(mask);
    USB_EVENT_LOG_I("USB Chat unpluged[%d], value: %d, has_pending_msg: %d, count: %d", 4, APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT, 0, has_pending_msg, s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_SPEAKER_CHAT].enable_callback_count);
    if (!has_pending_msg) {
        ui_shell_send_event(from_isr, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_UNPLUG,
                        (void *)event_data, 0, NULL, 0);
    }
}

static void apps_event_usb_chat_unplug_cb(void)
{
    apps_event_usb_chat_unplug_event(false);
}

#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
static void apps_event_usb_gaming_unplug_event(bool from_isr)
{
    uint32_t event_data = 0;
    app_events_usb_port_t *p_port = (app_events_usb_port_t *)&event_data;
    USB_EVENT_LOG_I("USB Gaming unpluged", 0);

    p_port->port_type = APP_USB_AUDIO_SPK_PORT;
    p_port->port_num = 1;
    app_usb_volume[1].left_volume = 0xFF;
    app_usb_volume[1].right_volume = 0xFF;
    bool has_pending_msg = false;
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING].enabled = 0;
    has_pending_msg = s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING].enable_msg_pending;
    s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING].enable_msg_pending = true;
    s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING].enable_callback_count ++;
    hal_nvic_restore_interrupt_mask(mask);
    USB_EVENT_LOG_I("USB Gaming unpluged[%d], value: %d, has_pending_msg: %d, count: %d", 4, APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING, 0, has_pending_msg, s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_SPEAKER_GAMING].enable_callback_count);
    if (!has_pending_msg) {
        ui_shell_send_event(from_isr, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_UNPLUG,
                        (void *)event_data, 0, NULL, 0);
    }
}

static void apps_event_usb_gaming_unplug_cb(void)
{
    apps_event_usb_gaming_unplug_event(false);
}
#endif

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
static void apps_event_usb_mic_unplug_event(bool from_isr)
{
    uint32_t event_data = 0;
    app_events_usb_port_t *p_port = (app_events_usb_port_t *)&event_data;
    USB_EVENT_LOG_I("USB MIC unpluged", 0);

    app_usb_volume[2].left_volume = 0xFF;
    app_usb_volume[2].right_volume = 0xFF;
    p_port->port_type = APP_USB_AUDIO_MIC_PORT;
    bool has_pending_msg = false;
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_MIC].enabled = 0;
    has_pending_msg = s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_MIC].enable_msg_pending;
    s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_MIC].enable_msg_pending = true;
    s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_MIC].enable_callback_count ++;
    hal_nvic_restore_interrupt_mask(mask);
    USB_EVENT_LOG_I("USB Gaming unpluged[%d], value: %d, has_pending_msg: %d, count: %d", 4, APPS_USB_EVENTS_INTERFACE_MIC, 0, has_pending_msg, s_speaker_enable_control[APPS_USB_EVENTS_INTERFACE_MIC].enable_callback_count);
    if (!has_pending_msg) {
        ui_shell_send_event(from_isr, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_UNPLUG,
                        (void *)event_data, 0, NULL, 0);
    }
}

static void apps_event_usb_mic_unplug_cb(void)
{
    apps_event_usb_mic_unplug_event(false);
}
#endif

static void apps_event_usb_volumechange_cb(uint8_t ep_number, uint8_t channel, uint32_t volume, int32_t db)
{
    app_usb_volume_data_t *p_vol = NULL;

    USB_EVENT_LOG_I("usb_audio_volumechange_cb, ep: 0x%x, channel:0x%x, vol:%d, db:%d", 4, ep_number, channel, volume, db);

    uint8_t chat_if = 0xFF, gaming_if = 0xFF, mic_if = 0xFF;
    uint8_t chat_ep = 0xFF, gaming_ep = 0xFF, mic_ep = 0xFF;
    USB_Audio_Get_Game_Info(&gaming_if, &gaming_ep);
    USB_Audio_Get_Chat_Info(&chat_if, &chat_ep, &mic_if, &mic_ep);
    if ((0xFF != gaming_ep) || (0xFF != chat_ep) || (0xFF != mic_ep)) {
        if (gaming_ep == ep_number) {
            p_vol = &(app_usb_volume[1]);  /* gaming */
        } else if (chat_ep == ep_number) {
            p_vol = &(app_usb_volume[0]);  /* chat */
        } else if (mic_ep == ep_number) {
            p_vol = &(app_usb_volume[2]);  /* mic */
        } else {
            USB_EVENT_LOG_E("[Error] volume_cb unknown ep, gaming_ep:0x%x, chat_ep:0x%x, mic_ep:0x%x", 3,
                            gaming_ep, chat_ep, mic_ep);
        }
    }

    if (NULL != p_vol) {
        if (0x00 == channel) {
            /* Master Channel (0) Set both L and R */
            p_vol->left_volume = (volume > 100) ? 100 : volume;
            p_vol->left_db = db;
            p_vol->right_volume = p_vol->left_volume;
            p_vol->right_db = db;
        } else if (0x01 == channel) {
            p_vol->left_volume = (volume > 100) ? 100 : volume;
            p_vol->left_db = db;
        } else if (0x02 == channel) {
            p_vol->right_volume = (volume > 100) ? 100 : volume;
            p_vol->right_db = db;
        }
        /* Send 100ms delay message to avoid to too fast volume notify lead to heap OOM */
        if (!app_is_delay_timer_start) {
            app_is_delay_timer_start = true;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_SIMULATE_TIMER,
                                apps_event_usb_simulater_callback, 0, NULL, 100);
        }
    }
}

static void apps_event_usb_mute_cb(uint8_t ep_number, usb_audio_mute_t mute)
{
    uint32_t event_data = 0;
    app_events_usb_mute_t *p_mute = (app_events_usb_mute_t *)&event_data;

    USB_EVENT_LOG_I("usb_audio_mute_cb, ep:0x%x, mute:0x%x", 2, ep_number, mute);

    uint8_t chat_if = 0xFF, gaming_if = 0xFF, mic_if = 0xFF;
    uint8_t chat_ep = 0xFF, gaming_ep = 0xFF, mic_ep = 0xFF;
    USB_Audio_Get_Game_Info(&gaming_if, &gaming_ep);
    USB_Audio_Get_Chat_Info(&chat_if, &chat_ep, &mic_if, &mic_ep);
    if ((0xFF != gaming_ep) || (0xFF != chat_ep) || (0xFF != mic_ep)) {
        if (gaming_ep == ep_number) {
            p_mute->port_type = APP_USB_AUDIO_SPK_PORT;
            p_mute->port_num = 1; /* gaming streaming port */
        } else if (chat_ep == ep_number) {
            p_mute->port_type = APP_USB_AUDIO_SPK_PORT;
            p_mute->port_num = 0; /* chat streaming port */
        } else if (mic_ep == ep_number) {
            p_mute->port_type = APP_USB_AUDIO_MIC_PORT;
            p_mute->port_num = 0;
        } else {
            p_mute->port_type = APP_USB_AUDIO_UNKNOWN_PORT;
            USB_EVENT_LOG_E("[Error] mute_cb unknown ep, gaming_ep:0x%x, chat_ep:0x%x, mic_ep:0x%x", 3,
                            gaming_ep, chat_ep, mic_ep);
        }
    }

    if (APP_USB_AUDIO_UNKNOWN_PORT != p_mute->port_type) {
        if (USB_AUDIO_MUTE_ON == mute) {
            p_mute->is_mute = 0x01;
        } else {
            p_mute->is_mute = 0x00;
        }
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                            APPS_EVENTS_USB_AUDIO_MUTE,
                            (void *)event_data, 0, NULL, 0);
    }
}

uint32_t apps_event_usb_event_sample_rate_convert(app_usb_audio_sample_rate_t sample_rate)
{
    static uint32_t sample_rates[] = {
        0,      //#define APP_USB_AUDIO_SAMPLE_RATE_UNKNOWN                0x00     /**< Unknown. */
        16000,  //#define APP_USB_AUDIO_SAMPLE_RATE_16K                    0x01     /**< 16KHz. */
        32000,  //#define APP_USB_AUDIO_SAMPLE_RATE_32K                    0x02     /**< 32KHz. */
        44100,  //#define APP_USB_AUDIO_SAMPLE_RATE_44_1K                  0x03     /**< 44.1KHz. */
        48000,  //#define APP_USB_AUDIO_SAMPLE_RATE_48K                    0x04     /**< 48KHz. */
        96000,  //#define APP_USB_AUDIO_SAMPLE_RATE_96K                    0x05     /**< 96KHz. */
        192000, //#define APP_USB_AUDIO_SAMPLE_RATE_192K                   0x06     /**< 192KHz. */
        24000,  //#define APP_USB_AUDIO_SAMPLE_RATE_24K                    0x07     /**< 24KHz. */
    };
    if (sample_rate >= sizeof(sample_rates)/sizeof(uint32_t)) {
        return 0;
    }
    return sample_rates[sample_rate];
}

static void apps_event_usb_sample_rate_cb(uint8_t ep_number, uint32_t sampling_rate)
{
    uint32_t event_data = 0;
    app_events_usb_sample_rate_t *p_rate = (app_events_usb_sample_rate_t *)&event_data;
    uint8_t interface_id = APPS_USB_EVENTS_INTERFACE_MAX;
    USB_EVENT_LOG_I("usb_sample_rate_cb, ep:0x%x, sample_rate:%d", 2, ep_number, sampling_rate);

    apps_event_usb_get_usb_port_info(&p_rate->port, &interface_id, ep_number, false);

    if (APP_USB_AUDIO_UNKNOWN_PORT != p_rate->port.port_type && interface_id < APPS_USB_EVENTS_INTERFACE_MAX) {
        bool has_pending_msg = false;
        uint32_t mask;
        hal_nvic_save_and_set_interrupt_mask(&mask);
        s_speaker_enable_control[interface_id].sample_rate = sampling_rate;
        has_pending_msg = s_speaker_enable_control[interface_id].sample_rate_msg_pending;
        s_speaker_enable_control[interface_id].sample_rate_msg_pending = true;
        s_speaker_enable_control[interface_id].sample_rate_callback_count ++;
        hal_nvic_restore_interrupt_mask(mask);
        USB_EVENT_LOG_I("usb_sample_rate_cb[%d], sample_rate: %d, has_pending_msg: %d, count: %d", 4, interface_id, sampling_rate, has_pending_msg, s_speaker_enable_control[interface_id].sample_rate_callback_count);
        if (!has_pending_msg) {
        p_rate->rate = apps_event_usb_sample_rate_exchange(sampling_rate);
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                            APPS_EVENTS_USB_AUDIO_SAMPLE_RATE,
                            (void *)event_data, 0, NULL, 0);
    }
    }
}

static void apps_event_usb_sample_size_cb(uint8_t ep_number, uint8_t sampling_size)
{
    uint32_t event_data = 0;
    app_events_usb_sample_size_t *p_size = (app_events_usb_sample_size_t *)&event_data;
    uint8_t interface_id = APPS_USB_EVENTS_INTERFACE_MAX;
    USB_EVENT_LOG_I("usb_sample_size_cb, ep:0x%x, sample_size:%d", 2, ep_number, sampling_size);
    apps_event_usb_get_usb_port_info(&p_size->port, &interface_id, ep_number, false);

    if (APP_USB_AUDIO_UNKNOWN_PORT != p_size->port.port_type && interface_id < APPS_USB_EVENTS_INTERFACE_MAX) {
        /* usb will be notify sample size before interface enable every time */
        uint32_t mask;
        hal_nvic_save_and_set_interrupt_mask(&mask);
        s_speaker_enable_control[interface_id].sample_size = sampling_size;
        hal_nvic_restore_interrupt_mask(mask);
        USB_EVENT_LOG_I("usb_sample_size_cb[%d], sample_size: %d, count: %d", 3, interface_id, sampling_size, s_speaker_enable_control[interface_id].enable_callback_count);
        /*
        p_size->size = sampling_size;
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                            APPS_EVENTS_USB_AUDIO_SAMPLE_SIZE,
                            (void *)event_data, 0, NULL, 0);
                            */
    }
}

static void apps_event_usb_channel_cb(uint8_t ep_number, uint8_t channel)
{
    uint32_t event_data = 0;
    app_events_usb_channel_t *p_channel = (app_events_usb_channel_t *)&event_data;
    uint8_t interface_id = APPS_USB_EVENTS_INTERFACE_MAX;
    USB_EVENT_LOG_I("usb_channel_cb, ep:0x%x, channel:%d", 2, ep_number, channel);

    apps_event_usb_get_usb_port_info(&p_channel->port, &interface_id, ep_number, false);
    if (APP_USB_AUDIO_UNKNOWN_PORT != p_channel->port.port_type && interface_id < APPS_USB_EVENTS_INTERFACE_MAX) {
        /* usb will be notify channel before interface enable every time */
        uint32_t mask;
        hal_nvic_save_and_set_interrupt_mask(&mask);
        s_speaker_enable_control[interface_id].channel = channel;
        hal_nvic_restore_interrupt_mask(mask);
        USB_EVENT_LOG_I("usb_channel_cb[%d], channel: %d, count: %d", 3, interface_id, channel, s_speaker_enable_control[interface_id].enable_callback_count);
        /*
        p_channel->channel = channel;
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                            APPS_EVENTS_USB_AUDIO_CHANNEL,
                            (void *)event_data, 0, NULL, 0);
        */
    }
}

#endif

void apps_event_usb_hid_keyboard_led_control_cb(uint32_t led_control)
{
    uint32_t event_data = led_control;
    USB_EVENT_LOG_I("usb_hid_keyboard_led_control_cb, led_control:%d", 1, led_control);
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_HID_GENARIC_DATA,
        APPS_EVENT_USB_HID_LED_CONTROL, (void *)event_data, 0, NULL, 0);
}
static void apps_event_usb_suspend_callback(void)
{
    USB_EVENT_LOG_I("apps_event_usb_suspend_callback", 0);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_SUSPEND);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESUME);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESET);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_CONFIG_DONE);
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
#if defined(AIR_USB_AUDIO_ENABLE)
    apps_event_usb_chat_unplug_event(true);
#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
    apps_event_usb_gaming_unplug_event(true);
#endif
#endif

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    apps_event_usb_mic_unplug_event(true);
#endif
#endif /* #if defined(AIR_WIRELESS_MIC_RX_ENABLE)*/
    ui_shell_send_event(true, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_SUSPEND,
                        NULL, 0, NULL, 500);
}

static void apps_event_usb_resume_callback(void)
{
    USB_EVENT_LOG_I("apps_event_usb_resume_callback", 0);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_SUSPEND);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESUME);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESET);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_CONFIG_DONE);
    ui_shell_send_event(true, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_RESUME,
                        NULL, 0, NULL, 1500);
}

static void apps_event_usb_reset_callback(void)
{
    USB_EVENT_LOG_I("apps_event_usb_reset_callback", 0);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_SUSPEND);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESUME);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESET);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_CONFIG_DONE);
    apps_event_usb_event_init();
    ui_shell_send_event(true, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_RESET,
                        NULL, 0, NULL, 0);
}

static void app_event_usb_device_config_cb(usb_evt_t event, void* usb_data, void* user_data)
{
    USB_EVENT_LOG_I("apps_event_usb_config_done_callback", 0);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_SUSPEND);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESUME);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESET);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_CONFIG_DONE);
    ui_shell_send_event(true, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_CONFIG_DONE,
                        NULL, 0, NULL, 0);

#if defined (AIR_PURE_GAMING_ENABLE)
    g_usb_config_done = true;
#endif					
}

#if defined(AIR_USB_AUDIO_ENABLE)
static app_usb_audio_sample_rate_t apps_event_usb_sample_rate_exchange(uint32_t sample_rate)
{
    /* default 48KHz*/
    app_usb_audio_sample_rate_t rate = APP_USB_AUDIO_SAMPLE_RATE_48K;
    switch (sample_rate) {
        case 16000:
            rate = APP_USB_AUDIO_SAMPLE_RATE_16K;
            break;
        case 24000:
            rate = APP_USB_AUDIO_SAMPLE_RATE_24K;
            break;
        case 32000:
            rate = APP_USB_AUDIO_SAMPLE_RATE_32K;
            break;
        case 44100:
            rate = APP_USB_AUDIO_SAMPLE_RATE_44_1K;
            break;
        case 48000:
            rate = APP_USB_AUDIO_SAMPLE_RATE_48K;
            break;
        case 96000:
            rate = APP_USB_AUDIO_SAMPLE_RATE_96K;
            break;
        case 192000:
            rate = APP_USB_AUDIO_SAMPLE_RATE_192K;
            break;
        default:
            break;
    }
    return rate;
}

static void apps_event_usb_simulater_callback(void)
{
    USB_EVENT_LOG_I("apps_event_usb_simulater_callback", 0);
    /* Chat: compare cached volume to decide volume change */
    if (app_usb_volume[0].left_volume != app_usb_volume_cache[0].left_volume
        || app_usb_volume[0].right_volume != app_usb_volume_cache[0].right_volume) {
        app_events_usb_volume_t *p_vol = (app_events_usb_volume_t *)malloc(sizeof(app_events_usb_volume_t));
        if (p_vol == NULL) {
            USB_EVENT_LOG_E("apps_event_usb_simulater_callback malloc fail", 0);
            return;
        }
        p_vol->port_type = APP_USB_AUDIO_SPK_PORT;
        p_vol->port_num = 0;
        p_vol->left_volume = app_usb_volume[0].left_volume;
        p_vol->right_volume = app_usb_volume[0].right_volume;
        p_vol->left_db = app_usb_volume[0].left_db;
        p_vol->right_db = app_usb_volume[0].right_db;
        if (p_vol->left_volume == 0xFF) {
            p_vol->left_volume = p_vol->right_volume;
            p_vol->left_db = p_vol->right_db;
        }

        if (p_vol->right_volume == 0xFF) {
            p_vol->right_volume = p_vol->left_volume;
            p_vol->right_db = p_vol->left_db;
        }
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                            APPS_EVENTS_USB_AUDIO_VOLUME,
                            (void *)p_vol, sizeof(app_events_usb_volume_t), NULL, 0);
    }

    /* Gaming: compare cached volume to decide volume change */
    if (app_usb_volume[1].left_volume != app_usb_volume_cache[1].left_volume
        || app_usb_volume[1].right_volume != app_usb_volume_cache[1].right_volume) {
        app_events_usb_volume_t *p_vol = (app_events_usb_volume_t *)malloc(sizeof(app_events_usb_volume_t));
        if (p_vol == NULL) {
            USB_EVENT_LOG_E("apps_event_usb_simulater_callback malloc fail", 0);
            return;
        }
        p_vol->port_type = APP_USB_AUDIO_SPK_PORT;
        p_vol->port_num = 1;
        p_vol->left_volume = app_usb_volume[1].left_volume;
        p_vol->right_volume = app_usb_volume[1].right_volume;
        p_vol->left_db = app_usb_volume[1].left_db;
        p_vol->right_db = app_usb_volume[1].right_db;
        if (p_vol->left_volume == 0xFF) {
            p_vol->left_volume = p_vol->right_volume;
            p_vol->left_db = p_vol->right_db;
        }

        if (p_vol->right_volume == 0xFF) {
            p_vol->right_volume = p_vol->left_volume;
            p_vol->right_db = p_vol->left_db;
        }
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                            APPS_EVENTS_USB_AUDIO_VOLUME,
                            (void *)p_vol, sizeof(app_events_usb_volume_t), NULL, 0);
    }

    /* Mic: compare cached volume to decide volume change */
    if (app_usb_volume[2].left_volume != app_usb_volume_cache[2].left_volume
        || app_usb_volume[2].right_volume != app_usb_volume_cache[2].right_volume) {
        app_events_usb_volume_t *p_vol = (app_events_usb_volume_t *)malloc(sizeof(app_events_usb_volume_t));
        if (p_vol == NULL) {
            USB_EVENT_LOG_E("apps_event_usb_simulater_callback malloc fail", 0);
            return;
        }
        p_vol->port_type = APP_USB_AUDIO_MIC_PORT;
        p_vol->port_num = 0;
        p_vol->left_volume = app_usb_volume[2].left_volume;
        p_vol->right_volume = app_usb_volume[2].right_volume;
        p_vol->left_db = app_usb_volume[2].left_db;
        p_vol->right_db = app_usb_volume[2].right_db;
        if (p_vol->left_volume == 0xFF) {
            p_vol->left_volume = p_vol->right_volume;
            p_vol->left_db = p_vol->right_db;
        }

        if (p_vol->right_volume == 0xFF) {
            p_vol->right_volume = p_vol->left_volume;
            p_vol->right_db = p_vol->left_db;
        }
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                            APPS_EVENTS_USB_AUDIO_VOLUME,
                            (void *)p_vol, sizeof(app_events_usb_volume_t), NULL, 0);
    }

    memcpy(app_usb_volume_cache, app_usb_volume, sizeof(app_usb_volume_data_t)*USB_PORT_MAXIMUN_NUMBER);
    app_is_delay_timer_start = false;    /* reinit volume delay timer flag */
}
#endif

