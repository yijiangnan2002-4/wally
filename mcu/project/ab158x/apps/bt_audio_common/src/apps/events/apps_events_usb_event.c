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
#ifdef APPS_USB_AUDIO_SUPPORT
#include "app_usb_audio_idle_activity.h"
#endif /* APPS_USB_AUDIO_SUPPORT */
#include "usbaudio_drv.h"
#include "usb.h"
#include "ui_shell_manager.h"
#include "ui_shell_activity.h"
#include "apps_debug.h"
#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
#include "usb_hid_srv.h"
#endif
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "apps_customer_config.h"
#include <string.h>
#include <stdlib.h>

#define USB_EVENT_LOG_I(msg, ...)     APPS_LOG_MSGID_I("[USB_EVENT]"msg, ##__VA_ARGS__)
#define USB_EVENT_LOG_E(msg, ...)     APPS_LOG_MSGID_E("[USB_EVENT]"msg, ##__VA_ARGS__)
#define USB_EVENT_LOG_D(msg, ...)     APPS_LOG_MSGID_D("[USB_EVENT]"msg, ##__VA_ARGS__)

#define USB_PORT_MAXIMUM_NUMBER 3

typedef struct {
    app_usb_audio_port_t port_type;         /* spk/mic */
    uint8_t port_num;                       /* range:0 ~ 1; 0:chat,1:gaming */
    uint8_t left_volume;                    /* 0 ~ 100 */
    uint8_t right_volume;                   /* 0 ~ 100 */
    int32_t left_db;
    int32_t right_db;
} app_usb_volume_data_t;

static app_usb_volume_data_t app_usb_volume[USB_PORT_MAXIMUM_NUMBER]; /* 0:chat,1:gaming,2:mic */
static app_usb_volume_data_t app_usb_volume_cache[USB_PORT_MAXIMUM_NUMBER]; /* 0:chat,1:gaming,2:mic */
/* for sample rate filter */
static uint32_t app_usb_sample_rate[USB_PORT_MAXIMUM_NUMBER]; /* 0:chat,1:gaming,2:mic */
static uint32_t app_usb_sample_rate_cache[USB_PORT_MAXIMUM_NUMBER]; /* 0:chat,1:gaming,2:mic */

/* flag to check volume delay timer is start or not */
static volatile bool app_is_delay_timer_start = false;
static app_usb_mode_t s_usb_mode;

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
#endif
static void apps_event_usb_suspend_callback(void);
static void apps_event_usb_resume_callback(void);
static void apps_event_usb_reset_callback(void);

#if defined(AIR_USB_AUDIO_ENABLE)
static void app_event_usb_sampling_size_callback(uint8_t ep_number, uint8_t sampling_size);
static void app_event_usb_set_channel_callback(uint8_t ep_number, uint8_t channel);

static app_usb_audio_sample_rate_t apps_event_usb_sample_rate_exchange(uint32_t sample_rate);
static void apps_event_usb_simulater_callback(void);
#endif

#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
static void app_usb_hid_call_event_callback(usb_hid_srv_event_t event)
{
    if (event < USB_HID_SRV_EVENT_CALL_MAX) {
        ui_shell_send_event(true,
                            EVENT_PRIORITY_HIGH,
                            EVENT_GROUP_UI_SHELL_USB_HID_CALL,
                            event,
                            NULL, 0, NULL, 0);
    }
}
#endif

static app_usb_mode_t app_dongle_common_idle_verify_mode(app_usb_mode_t mode)
{
    if (mode >= APPS_USB_MODE_MAX) {
        mode = APPS_DEFAULT_USB_MODE;
    }
    return mode;
}

app_usb_mode_t app_dongle_common_idle_read_mode(void)
{
    uint32_t read_size = sizeof(s_usb_mode);
    if (NVKEY_STATUS_OK != nvkey_read_data(NVID_APP_DONGLE_USB_MODE, &s_usb_mode, &read_size)) {
        s_usb_mode = APPS_DEFAULT_USB_MODE;
    }
    s_usb_mode = app_dongle_common_idle_verify_mode(s_usb_mode);
    return s_usb_mode;
}

void apps_event_usb_event_init()
{
#if defined(AIR_USB_AUDIO_ENABLE)
#if (!defined(AIR_DCHS_MODE_ENABLE)) && (!defined(AIR_USB_AUDIO_2_SPK_ENABLE))
    /* Headset project no needs to use RX ALT2 */
    /**
     * Workaround for feature_68_headset_ull2.mk
     * Only feature_68_headset_ull2 and feature_68_headset_va use 2nd spk.
     * Open it temporarily.
     */
    USB_Aduio_Set_RX1_Alt2(false, 0, 0, 0, (uint32_t *)NULL);
#endif

    /* register usb audio in0 */
    USB_Audio_Register_SetInterface_Callback(0, apps_event_usb_setinterface_cb);
    USB_Audio_Register_Unplug_Callback(0, apps_event_usb_chat_unplug_cb);
    USB_Audio_Register_VolumeChange_Callback(0, apps_event_usb_volumechange_cb);
    USB_Audio_Register_Mute_Callback(0, apps_event_usb_mute_cb);
    USB_Audio_Register_SetSamplingRate_Callback(0, apps_event_usb_sample_rate_cb);
    USB_Audio_Register_SetSampleSize_Callback(0, app_event_usb_sampling_size_callback);
    USB_Audio_Register_SetChannel_Callback(0, app_event_usb_set_channel_callback);

#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
    /* register usb audio in1 */
    USB_Audio_Register_SetInterface_Callback(1, apps_event_usb_setinterface_cb);
    USB_Audio_Register_Unplug_Callback(1, apps_event_usb_gaming_unplug_cb);
    USB_Audio_Register_VolumeChange_Callback(1, apps_event_usb_volumechange_cb);
    USB_Audio_Register_Mute_Callback(1, apps_event_usb_mute_cb);
    USB_Audio_Register_SetSamplingRate_Callback(1, apps_event_usb_sample_rate_cb);
    USB_Audio_Register_SetSampleSize_Callback(1, app_event_usb_sampling_size_callback);
    USB_Audio_Register_SetChannel_Callback(1, app_event_usb_set_channel_callback);
#endif

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    /* register default usb audio out. */
    USB_Audio_Register_Mic_SetInterface_Callback(0, apps_event_usb_setinterface_cb);
    USB_Audio_Register_Mic_Unplug_Callback(0, apps_event_usb_mic_unplug_cb);
    USB_Audio_Register_Mic_VolumeChange_Callback(0, apps_event_usb_volumechange_cb);
    USB_Audio_Register_Mic_Mute_Callback(0, apps_event_usb_mute_cb);
    USB_Audio_Register_Mic_SetSamplingRate_Callback(0, apps_event_usb_sample_rate_cb);
    USB_Audio_Register_Mic_SetSampleSize_Callback(0, app_event_usb_sampling_size_callback);
    USB_Audio_Register_Mic_SetChannel_Callback(0, app_event_usb_set_channel_callback);
#endif

#endif
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
    memcpy(app_usb_volume_cache, app_usb_volume, sizeof(app_usb_volume_data_t)*USB_PORT_MAXIMUM_NUMBER);
    memset(app_usb_sample_rate, 0x00, sizeof(uint32_t)*USB_PORT_MAXIMUM_NUMBER);
    memset(app_usb_sample_rate_cache, 0x00, sizeof(uint32_t)*USB_PORT_MAXIMUM_NUMBER);
#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
    usb_hid_srv_init(app_usb_hid_call_event_callback);
#endif
    USB_EVENT_LOG_I("apps_event_usb_event_init done.", 0);
}

#if defined(AIR_USB_AUDIO_ENABLE)
/*========================================================================================*/
/*                          USB Audio evet callback                                       */
/*========================================================================================*/

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

    USB_EVENT_LOG_I("usb_audio_setinterface_cb, if: 0x%x, value: 0x%x", 2, interface_number, alternate_set);

    uint8_t chat_if = 0xFF, gaming_if = 0xFF, mic_if = 0xFF;
    uint8_t chat_ep = 0xFF, gaming_ep = 0xFF, mic_ep = 0xFF;
    USB_Audio_Get_Game_Info(&gaming_if, &gaming_ep);
    USB_Audio_Get_Chat_Info(&chat_if, &chat_ep, &mic_if, &mic_ep);
    if ((0xFF != gaming_if) || (0xFF != chat_if) || (0xFF != mic_if)) {
        if (gaming_if == interface_number) {
            p_port->port_type = APP_USB_AUDIO_SPK_PORT;
            p_port->port_num = 1; /* gaming streaming port */
        } else if (chat_if == interface_number) {
            p_port->port_type = APP_USB_AUDIO_SPK_PORT;
            p_port->port_num = 0; /* chat streaming port */
        } else if (mic_if == interface_number) {
            p_port->port_type = APP_USB_AUDIO_MIC_PORT;
            p_port->port_num = 0;
        } else {
            p_port->port_type = APP_USB_AUDIO_UNKNOWN_PORT;
            USB_EVENT_LOG_E("[Error] unknown interface, gaming_if:0x%x, chat_if:0x%x, mic_if:0x%x", 3,
                            gaming_if, chat_if, mic_if);
        }
    }

    if (APP_USB_AUDIO_UNKNOWN_PORT != p_port->port_type) {
        if (0x01 == alternate_set) {
            event = APPS_EVENTS_USB_AUDIO_PLAY;
        } else {
            event = APPS_EVENTS_USB_AUDIO_STOP;
        }
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                            event, (void *)event_data, 0, NULL, 0);
    }
}

/**
 * @brief   Callback when USB unplugged.
 *
 * @return  None.
 */
static void apps_event_usb_chat_unplug_cb(void)
{
    uint32_t event_data = 0;
    app_events_usb_port_t *p_port = (app_events_usb_port_t *)&event_data;
    USB_EVENT_LOG_I("USB Chat unpluged", 0);

    p_port->port_type = APP_USB_AUDIO_SPK_PORT;
    p_port->port_num = 0;
    app_usb_volume[0].left_volume = 0xFF;
    app_usb_volume[0].right_volume = 0xFF;
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_UNPLUG,
                        (void *)event_data, 0, NULL, 0);
}

#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
static void apps_event_usb_gaming_unplug_cb(void)
{
    uint32_t event_data = 0;
    app_events_usb_port_t *p_port = (app_events_usb_port_t *)&event_data;
    USB_EVENT_LOG_I("USB Gaming unpluged", 0);

    p_port->port_type = APP_USB_AUDIO_SPK_PORT;
    p_port->port_num = 1;
    app_usb_volume[1].left_volume = 0xFF;
    app_usb_volume[1].right_volume = 0xFF;
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_UNPLUG,
                        (void *)event_data, 0, NULL, 0);
}
#endif

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
static void apps_event_usb_mic_unplug_cb(void)
{
    uint32_t event_data = 0;
    app_events_usb_port_t *p_port = (app_events_usb_port_t *)&event_data;
    USB_EVENT_LOG_I("USB MIC unpluged", 0);

    app_usb_volume[2].left_volume = 0xFF;
    app_usb_volume[2].right_volume = 0xFF;
    p_port->port_type = APP_USB_AUDIO_MIC_PORT;
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_UNPLUG,
                        (void *)event_data, 0, NULL, 0);
}
#endif

#ifdef AIR_DCHS_MODE_MASTER_ENABLE
static int32_t s_8ch_volumes[8];
int32_t *apps_event_usb_8ch_volumes()
{
    return s_8ch_volumes;
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
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
        if (chat_ep == ep_number) {
            if (channel == 0x00) {
                for (int32_t i = 0; i < 8; i++) {
                    s_8ch_volumes[i] = db;
                }
            } else if (channel < 8) {
                s_8ch_volumes[channel - 1] = db;
            }
        }
#endif
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
    if (sample_rate >= (sizeof(sample_rates) / sizeof(uint32_t))) {
        return 0;
    }
    return sample_rates[sample_rate];
}

static void apps_event_usb_sample_rate_cb(uint8_t ep_number, uint32_t sampling_rate)
{
    uint32_t event_data = 0;
    app_events_usb_sample_rate_t *p_rate = (app_events_usb_sample_rate_t *)&event_data;
    USB_EVENT_LOG_I("usb_sample_rate_cb, ep:0x%x, sample_rate:%d", 2, ep_number, sampling_rate);

    uint8_t chat_if = 0xFF, gaming_if = 0xFF, mic_if = 0xFF;
    uint8_t chat_ep = 0xFF, gaming_ep = 0xFF, mic_ep = 0xFF;
    USB_Audio_Get_Game_Info(&gaming_if, &gaming_ep);
    USB_Audio_Get_Chat_Info(&chat_if, &chat_ep, &mic_if, &mic_ep);
    if ((0xFF != gaming_ep) || (0xFF != chat_ep) || (0xFF != mic_ep)) {
        if (gaming_ep == ep_number) {
            p_rate->port_type = APP_USB_AUDIO_SPK_PORT;
            p_rate->port_num = 1; /* gaming streaming port */
            app_usb_sample_rate[0] = sampling_rate;
        } else if (chat_ep == ep_number) {
            p_rate->port_type = APP_USB_AUDIO_SPK_PORT;
            p_rate->port_num = 0; /* chat streaming port */
            app_usb_sample_rate[1] = sampling_rate;
        } else if (mic_ep == ep_number) {
            p_rate->port_type = APP_USB_AUDIO_MIC_PORT;
            p_rate->port_num = 0;
            app_usb_sample_rate[2] = sampling_rate;
        } else {
            p_rate->port_type = APP_USB_AUDIO_UNKNOWN_PORT;
            USB_EVENT_LOG_E("[Error] sample_rate_cb unknown ep, gaming_ep:0x%x, chat_ep:0x%x, mic_ep:0x%x", 3,
                            gaming_ep, chat_ep, mic_ep);
        }
    }

    if (APP_USB_AUDIO_UNKNOWN_PORT != p_rate->port_type) {
        /* usb will be notify sample rate before interface enable every time */
        if (app_usb_sample_rate_cache[0] != app_usb_sample_rate[0]
            || app_usb_sample_rate_cache[1] != app_usb_sample_rate[1]
            || app_usb_sample_rate_cache[2] != app_usb_sample_rate[2]) {
            p_rate->rate = apps_event_usb_sample_rate_exchange(sampling_rate);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                                APPS_EVENTS_USB_AUDIO_SAMPLE_RATE,
                                (void *)event_data, 0, NULL, 0);
            app_usb_sample_rate_cache[0] = app_usb_sample_rate[0];
            app_usb_sample_rate_cache[1] = app_usb_sample_rate[1];
            app_usb_sample_rate_cache[2] = app_usb_sample_rate[2];
        }
    }
}

static void get_port_type_by_ep_number(uint8_t ep_number, uint8_t *port_type, uint8_t *port_num)
{
    uint8_t chat_if = 0xFF, gaming_if = 0xFF, mic_if = 0xFF;
    uint8_t chat_ep = 0xFF, gaming_ep = 0xFF, mic_ep = 0xFF;
    USB_Audio_Get_Game_Info(&gaming_if, &gaming_ep);
    USB_Audio_Get_Chat_Info(&chat_if, &chat_ep, &mic_if, &mic_ep);
    if ((0xFF != gaming_ep) || (0xFF != chat_ep) || (0xFF != mic_ep)) {
        if (gaming_ep == ep_number) {
            *port_type = APP_USB_AUDIO_SPK_PORT;
            *port_num = 1; /* gaming streaming port */
        } else if (chat_ep == ep_number) {
            *port_type = APP_USB_AUDIO_SPK_PORT;
            *port_num = 0; /* chat streaming port */
        } else if (mic_ep == ep_number) {
            *port_type = APP_USB_AUDIO_MIC_PORT;
            *port_num = 0;
        } else {
            USB_EVENT_LOG_E("[Error] sample_rate_cb unknown ep, gaming_ep:0x%x, chat_ep:0x%x, mic_ep:0x%x", 3,
                            gaming_ep, chat_ep, mic_ep);
        }
    }
}

static void app_event_usb_sampling_size_callback(uint8_t ep_number, uint8_t sampling_size)
{
    uint32_t event_data = 0;
    app_events_usb_sample_size_t *size = (app_events_usb_sample_size_t *)&event_data;
    get_port_type_by_ep_number(ep_number, &size->port_type, &size->port_num);
    size->size = sampling_size;

    USB_EVENT_LOG_I("usb_sample_size_cb, ep:0x%x, sample_rate:%d", 2, ep_number, sampling_size);
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_SAMPLE_SIZE,
                        (void *)event_data, 0, NULL, 0);
}

static void app_event_usb_set_channel_callback(uint8_t ep_number, uint8_t channel)
{
    uint32_t event_data = 0;
    app_events_usb_set_channel_t *p_channel = (app_events_usb_set_channel_t *)&event_data;
    get_port_type_by_ep_number(ep_number, &p_channel->port_type, &p_channel->port_num);
    p_channel->channel_nums = channel;

    USB_EVENT_LOG_I("usb_set_channel_cb, ep:0x%x, channel:%d", 2, ep_number, channel);
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_SET_CHANNELS,
                        (void *)event_data, 0, NULL, 0);
}

#endif

static void apps_event_usb_suspend_callback(void)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_SUSPEND);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESUME);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESET);
    ui_shell_send_event(true, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_SUSPEND,
                        NULL, 0, NULL, 0);
}

static void apps_event_usb_resume_callback(void)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_SUSPEND);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESUME);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESET);
    ui_shell_send_event(true, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_RESUME,
                        NULL, 0, NULL, 0);
}

static void apps_event_usb_reset_callback(void)
{
    USB_EVENT_LOG_I("apps_event_usb_reset_callback", 0);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_SUSPEND);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESUME);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_AUDIO_RESET);
    apps_event_usb_event_init();
    ui_shell_send_event(true, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                        APPS_EVENTS_USB_AUDIO_RESET,
                        NULL, 0, NULL, 0);
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

    memcpy(app_usb_volume_cache, app_usb_volume, sizeof(app_usb_volume_data_t)*USB_PORT_MAXIMUM_NUMBER);
    app_is_delay_timer_start = false;    /* reinit volume delay timer flag */
}
#endif

void apps_usb_event_set_usb_mode(uint32_t mode)
{
    mode = app_dongle_common_idle_verify_mode(mode);
    if (mode != s_usb_mode) {
        USB_EVENT_LOG_E("apps_usb_event_set_usb_mode usb mode change from %d to %d", 2, s_usb_mode, mode);
        s_usb_mode = mode;
        nvkey_write_data(NVID_APP_DONGLE_USB_MODE, (uint8_t *) &s_usb_mode, sizeof(s_usb_mode));
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_REQUEST_REBOOT,
                            (void *)0, 0, NULL, 0);
    }
}

app_usb_mode_t apps_usb_event_get_current_usb_mode(void)
{
    return s_usb_mode;
}

/**************************************************************************************************/
/*************************************** USB init callbacks ***************************************/
/**************************************************************************************************/
#if defined(AIR_USB_AUDIO_ENABLE)
static const int32_t apps_usb_ent_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_ent_mic_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_game_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_game_mic_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_wmtx_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_wmtx_mic_sample_rate[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};


/* for 16/24 bits 2ch */
static const int32_t apps_usb_8ch_sample_rate_alt12[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

/* for 8ch */
static const int32_t apps_usb_8ch_sample_rate_alt3[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_mic_8ch_sample_rate_alt12[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_32K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_dchs_custom_sample_rate_alt12[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_mic_dchs_custom_sample_rate_alt12[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_16K,
    USB_AUDIO_SAMPLE_RATE_32K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

/* for MIC_192K_1CH */
static const int32_t apps_usb_192k_1ch_sample_rate_alt1[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_mic_192k_1ch_sample_rate_alt1[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_SAMPLE_RATE_192K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

static const int32_t apps_usb_mic_192k_1ch_sample_rate_alt2[USB_AUDIO_DSCR_MAX_FREQ_NUM] = {
    USB_AUDIO_SAMPLE_RATE_48K,
    USB_AUDIO_SAMPLE_RATE_96K,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL,
    USB_AUDIO_NULL
};

#endif

void apps_events_usb_event_usb_pulg_in_cb(usb_evt_t event, void *usb_data, void *user_data)
{
    switch (s_usb_mode) {
        case APPS_USB_MODE_GAMING: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_GAMING[%d]", 1, s_usb_mode);
            usb_set_device_type(APPS_USB_GAME_DEV_TYPE);
            usb_custom_set_speed(APPS_USB_GAME_HS_ENABLE);
            usb_custom_set_product_info(APPS_USB_GAME_VID, APPS_USB_GAME_PID, APPS_USB_GAME_VER);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset Gaming");
            break;
        }
        case APPS_USB_MODE_GAMING_ULL2: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_GAMING_ULL2[%d]", 1, s_usb_mode);
            usb_set_device_type(APPS_USB_GAME_DEV_TYPE);
            usb_custom_set_speed(APPS_USB_GAME_HS_ENABLE);
            usb_custom_set_product_info(APPS_USB_GAME_VID, 0x0821, APPS_USB_GAME_VER);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset Gaming ULL1");
            break;
        }
#ifdef AIR_USB_XBOX_ENABLE
        case APPS_USB_MODE_XBOX: {
            usb_set_device_type(APPS_USB_XBOX_DEV_TYPE);
            usb_custom_set_speed(APPS_USB_XBOX_HS_ENABLE);
            /* Xbox don't have product info */
            break;
        }
#endif
        case APPS_USB_MODE_ENTERPRISE: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_ENTERPRISE[%d]", 1, s_usb_mode);
            usb_set_device_type(APPS_USB_ENT_DEV_TYPE);
            usb_custom_set_speed(APPS_USB_ENT_HS_ENABLE);
            usb_custom_set_product_info(APPS_USB_ENT_VID, APPS_USB_ENT_PID, APPS_USB_ENT_VER);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset Enterprise");
            break;
        }
        case APPS_USB_MODE_WIRELESS_MIC_TX: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_WIRELESS_MIC_TX[%d]", 1, s_usb_mode);
            usb_set_device_type(USB_AUDIO);
            usb_custom_set_speed(false);
            usb_custom_set_product_info(0x0E8D, 0x080D, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Wireless Mic TX");
            break;
        }
        case APPS_USB_MODE_8CH: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_8CH[%d]", 1, s_usb_mode);
            usb_set_device_type(USB_AUDIO);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x080F, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset 8CH");
            break;
        }
        case APPS_USB_MODE_IOT: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_IOT[%d]", 1, s_usb_mode);
            usb_set_device_type(USB_AUDIO);
            usb_custom_set_speed(false);
            usb_custom_set_product_info(0x0E8D, 0x0811, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset IOT");
            break;
        }
        case APPS_USB_MODE_DCHS_CUSTOM: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_DCHS_CUSTOM[%d]", 1, s_usb_mode);
            usb_set_device_type(USB_AUDIO);
            usb_custom_set_speed(false);
            usb_custom_set_product_info(0x0E8D, 0x0813, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset DCHS Custom");
            break;
        }
        case APPS_USB_MODE_MIC_192K_1CH: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_MIC_192K_1CH[%d]", 1, s_usb_mode);
            usb_set_device_type(USB_AUDIO);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0815, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset MIC_192K_1CH");
            break;
        }
        case APPS_USB_MODE_AUDIO_CDC: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_AUDIO_CDC[%d]", 1, s_usb_mode);
            usb_set_device_type(USB_AUDIO_CDC);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0817, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset Audio CDC");
            break;
        }
        case APPS_USB_MODE_MSC: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_MSC[%d]", 1, s_usb_mode);
            usb_set_device_type(USB_MASS_STORAGE);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0823, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset Mass Storage");
            break;
        }
        case APPS_USB_MODE_CDC: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_CDC[%d]", 1, s_usb_mode);
            usb_set_device_type(USB_CDC_ACM);
            usb_custom_set_speed(true);
            usb_custom_set_product_info(0x0E8D, 0x0827, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset CDC");
            break;
        }
        case APPS_USB_MODE_HID: {
            USB_EVENT_LOG_I("USB Mode: APPS_USB_MODE_HID[%d]", 1, s_usb_mode);
            usb_set_device_type(USB_HID);
            usb_custom_set_speed(false);
            usb_custom_set_product_info(0x0E8D, 0x0829, 0x0100);
            usb_custom_set_string(USB_STRING_USAGE_PRODUCT, "Airoha Headset HID");
            break;
        }
        default:
            break;
    }
}

void apps_events_usb_event_usb_pulg_out_cb(usb_evt_t event, void *usb_data, void *user_data)
{
#ifdef APPS_USB_AUDIO_SUPPORT
    app_usb_audio_report_usb_plug_status(false);
#endif /* APPS_USB_AUDIO_SUPPORT */
}

static void apps_events_usb_event_usb_clear_tx_rx()
{
    USB_EVENT_LOG_I("apps_events_usb_event_usb_clear_tx_rx start clear", 1, s_usb_mode);
#if defined(AIR_USB_AUDIO_ENABLE)
    USB_Aduio_Set_RX1_Alt2(false, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL);
    USB_Aduio_Set_RX1_Alt3(false, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL);
    USB_Aduio_Set_RX2_Alt2(false, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL);
    USB_Aduio_Set_TX1_Alt2(false, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL, USB_AUDIO_NULL);
#endif
    USB_EVENT_LOG_I("apps_events_usb_event_usb_clear_tx_rx end clear", 1, s_usb_mode);
}

void apps_events_usb_event_usb_device_init_cb(usb_evt_t event, void *usb_data, void *user_data)
{
    USB_EVENT_LOG_I("apps_events_usb_event_usb_device_init_cb usb_mode:%d", 1, s_usb_mode);

    /* clear RX/TX alt2, alt3 setting */
    apps_events_usb_event_usb_clear_tx_rx();

    switch (s_usb_mode) {
        case APPS_USB_MODE_GAMING:
        case APPS_USB_MODE_GAMING_ULL2: {
#if defined(AIR_USB_AUDIO_ENABLE)
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, true, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_NULL);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_spk_channels(USB_AUDIO_2_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_RX2_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_RX2_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_game_mic_sample_rate);
#endif

            usb_hid_set_dscr_enable(
            (usb_hid_report_dscr_type_t[3]) {
                USB_REPORT_DSCR_TYPE_MUX,
                USB_REPORT_DSCR_TYPE_AC,
                USB_REPORT_DSCR_TYPE_TELEPHONY,
            },
            3);
            break;
        }
#ifdef AIR_USB_XBOX_ENABLE
        case APPS_USB_MODE_XBOX: {
            break;
        }
#endif
        case APPS_USB_MODE_ENTERPRISE: {
#if defined(AIR_USB_AUDIO_ENABLE)
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_HEADSET, USB_AUDIO_TERMT_HEADSET);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_1,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_ent_sample_rate);

            USB_Aduio_Set_RX1_Alt2(false,
                                   USB_AUDIO_NULL,
                                   USB_AUDIO_NULL,
                                   USB_AUDIO_NULL,
                                   USB_AUDIO_NULL);

            USB_Aduio_Set_RX2_Alt2(false,
                                   USB_AUDIO_NULL,
                                   USB_AUDIO_NULL,
                                   USB_AUDIO_NULL,
                                   USB_AUDIO_NULL);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_ent_mic_sample_rate);
#endif
            usb_hid_set_dscr_enable(
            (usb_hid_report_dscr_type_t[4]) {
                USB_REPORT_DSCR_TYPE_MUX,
                USB_REPORT_DSCR_TYPE_AC,
                USB_REPORT_DSCR_TYPE_TEAMS,
                USB_REPORT_DSCR_TYPE_TELEPHONY,
            },
            4);
            break;
        }
        case APPS_USB_MODE_WIRELESS_MIC_TX: {
#if defined(AIR_USB_AUDIO_ENABLE)
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_1,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_wmtx_sample_rate);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_1,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_wmtx_mic_sample_rate);
#endif

            usb_hid_set_dscr_enable(
            (usb_hid_report_dscr_type_t[2]) {
                USB_REPORT_DSCR_TYPE_MUX,
                USB_REPORT_DSCR_TYPE_AC,
            },
            2);
            break;
        }
        case APPS_USB_MODE_8CH: {
#if defined(AIR_USB_AUDIO_ENABLE)
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_NULL, USB_AUDIO_TERMT_NULL);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_8, USB_AUDIO_CHANNEL_CONBINE_8CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_8ch_sample_rate_alt12);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_8ch_sample_rate_alt12);

            USB_Aduio_Set_RX1_Alt3(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_1,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_8,
                                   (uint32_t *)apps_usb_8ch_sample_rate_alt3);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_mic_8ch_sample_rate_alt12);

            USB_Aduio_Set_TX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_mic_8ch_sample_rate_alt12);
#endif

            usb_hid_set_dscr_enable(
            (usb_hid_report_dscr_type_t[3]) {
                USB_REPORT_DSCR_TYPE_MUX,
                USB_REPORT_DSCR_TYPE_AC,
                USB_REPORT_DSCR_TYPE_TELEPHONY,
            },
            3);
            break;
        }
        case APPS_USB_MODE_IOT: {
#if defined(AIR_USB_AUDIO_ENABLE)
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);
#endif

            /* TODO: set sample rate */

            usb_hid_set_dscr_enable(
            (usb_hid_report_dscr_type_t[2]) {
                USB_REPORT_DSCR_TYPE_MUX,
                USB_REPORT_DSCR_TYPE_AC,
            },
            2);
            break;
        }
        case APPS_USB_MODE_DCHS_CUSTOM: {
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_HEADSET, USB_AUDIO_TERMT_HEADSET);
            usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_NULL, USB_AUDIO_TERMT_NULL);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_dchs_custom_sample_rate_alt12);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_dchs_custom_sample_rate_alt12);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_mic_dchs_custom_sample_rate_alt12);

            USB_Aduio_Set_TX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_mic_dchs_custom_sample_rate_alt12);

            usb_hid_set_dscr_enable(
            (usb_hid_report_dscr_type_t[3]) {
                USB_REPORT_DSCR_TYPE_MUX,
                USB_REPORT_DSCR_TYPE_AC,
                USB_REPORT_DSCR_TYPE_TELEPHONY,
            },
            3);
            break;
        }
        case APPS_USB_MODE_MIC_192K_1CH: {
#if defined(AIR_USB_AUDIO_ENABLE)
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_terminal_type(USB_AUDIO_2_PORT, USB_AUDIO_TERMT_NULL, USB_AUDIO_TERMT_NULL);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_192k_1ch_sample_rate_alt1);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_3,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_mic_192k_1ch_sample_rate_alt1);

            USB_Aduio_Set_TX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_mic_192k_1ch_sample_rate_alt2);

#endif

            usb_hid_set_dscr_enable(
            (usb_hid_report_dscr_type_t[3]) {
                USB_REPORT_DSCR_TYPE_MUX,
                USB_REPORT_DSCR_TYPE_AC,
                USB_REPORT_DSCR_TYPE_TELEPHONY,
            },
            3);
            break;
        }
        case APPS_USB_MODE_AUDIO_CDC: {
#if defined(AIR_USB_AUDIO_ENABLE)
            usb_audio_set_audio_card(USB_AUDIO_1_PORT, true, true);
            usb_audio_set_audio_card(USB_AUDIO_2_PORT, false, false);
            usb_audio_set_terminal_type(USB_AUDIO_1_PORT, USB_AUDIO_TERMT_SPEAKER, USB_AUDIO_TERMT_MICROPHONE);
            usb_audio_set_spk_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_2, USB_AUDIO_CHANNEL_CONBINE_2CH, USB_AUDIO_VC_INDIVIUAL);
            usb_audio_set_mic_channels(USB_AUDIO_1_PORT, USB_AUDIO_CHANNEL_1, USB_AUDIO_CHANNEL_CONBINE_MONO, USB_AUDIO_VC_MASTER);

            USB_Aduio_Set_RX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_RX1_Alt2(true,
                                   USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_24BIT,
                                   USB_AUDIO_CHANNEL_2,
                                   (uint32_t *)apps_usb_game_sample_rate);

            USB_Aduio_Set_TX1_Alt1(USB_AUDIO_SAMPLE_RATE_NUM_2,
                                   USB_AUDIO_SAMPLE_SIZE_16BIT,
                                   USB_AUDIO_CHANNEL_1,
                                   (uint32_t *)apps_usb_game_mic_sample_rate);
#endif

            usb_hid_set_dscr_enable(
            (usb_hid_report_dscr_type_t[3]) {
                USB_REPORT_DSCR_TYPE_MUX,
                USB_REPORT_DSCR_TYPE_AC,
                USB_REPORT_DSCR_TYPE_TELEPHONY,
            },
            3);
            break;
        }
        case APPS_USB_MODE_MSC: {
            break;
        }
        case APPS_USB_MODE_CDC: {
            break;
        }
        case APPS_USB_MODE_HID: {
            usb_hid_set_dscr_enable(
                (usb_hid_report_dscr_type_t[2]){
                    USB_REPORT_DSCR_TYPE_MUX,
                    USB_REPORT_DSCR_TYPE_AC,},
                2);
            break;
        }
        default:
            break;
    }
}
