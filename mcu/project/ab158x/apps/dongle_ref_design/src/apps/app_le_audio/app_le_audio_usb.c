/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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
#ifdef AIR_LE_AUDIO_ENABLE
#include "usbaudio_drv.h"
#include "apps_events_usb_event.h"

#include "app_le_audio.h"
#include "app_le_audio_usb.h"
#include "app_le_audio_utillity.h"
#include "app_preproc_activity.h"
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
#include "app_le_audio_ucst.h"
#include "app_le_audio_aird.h"
#include "app_le_audio_ucst_utillity.h"
#include "app_le_audio_ccp_call_control_server.h"
#include "app_le_audio_vcp_volume_controller.h"
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_le_audio_bcst.h"
#endif

#include "bt_le_audio_msglog.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
/* USB releated */
#define APP_LE_AUDIO_USB_PORT_NUM_0 0
#define APP_LE_AUDIO_USB_PORT_NUM_1 1

/* USB volume value information */
#define APP_LE_AUDIO_VOL_VALUE_MIN      0
#define APP_LE_AUDIO_VOL_VALUE_MAX      100
#define APP_LE_AUDIO_VOL_VALUE_DEFAULT  30

#define APP_LE_AUDIO_VOL_RANGE_MAX_15        (15)
#define APP_LE_AUDIO_VOL_RANGE_MAX_255       (255)


typedef enum {
    APP_LE_AUDIO_USB_EVENT_ENABLE,
    APP_LE_AUDIO_USB_EVENT_DISABLE,
    APP_LE_AUDIO_USB_EVENT_SAMPLE_SIZE_OBTAIN, /* Obtain a valid value when the corresponding steram port is not initialized. */
    APP_LE_AUDIO_USB_EVENT_SAMPLE_SIZE_CHANGE, /* Changed for an enabled USB port */
    APP_LE_AUDIO_USB_EVENT_SAMPLE_RATE_OBTAIN,
    APP_LE_AUDIO_USB_EVENT_SAMPLE_RATE_CHANGE,
    APP_LE_AUDIO_USB_EVENT_CHANNEL_OBTAIN,
    APP_LE_AUDIO_USB_EVENT_CHANNEL_CHANGE,
    APP_LE_AUDIO_USB_EVENT_MAX = 0xFF
} app_le_audio_usb_event_enum;

/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct {
    uint8_t is_streaming;                   /**< USB audio streaming state */
    uint8_t mute;                           /**< USB audio mute state: MUTE or UNMUTE */
    uint8_t vol_left;                       /**< USB audio volume L, value: 0 ~ 100 */
    uint8_t vol_right;                      /**< USB audio volume R, value: 0 ~ 100 */
    app_le_audio_usb_config_info_t config;  /**< USB sample rate and sample size and usb_channel */
} app_le_audio_usb_info_t;


/**************************************************************************************************
* Variable
**************************************************************************************************/
//const uint32_t sample_rate_table[] = {16000,16000,32000,44100,48000,96000};
const uint32_t sample_size_table[] =
{
    HAL_AUDIO_PCM_FORMAT_S16_LE, //default
    HAL_AUDIO_PCM_FORMAT_S8,
    HAL_AUDIO_PCM_FORMAT_S16_LE,
    HAL_AUDIO_PCM_FORMAT_S24_LE,
    HAL_AUDIO_PCM_FORMAT_S32_LE
};

app_le_audio_usb_info_t g_lea_usb_info[APP_LE_AUDIO_USB_PORT_MAX];

/**************************************************************************************************
* Prototype
**************************************************************************************************/

/**************************************************************************************************
* Static Functions
**************************************************************************************************/

static uint8_t app_le_audio_map_vol_level(uint8_t volume, uint16_t range_max)
{
    float local_level_f = 0;
    float vcp_level_f = volume;

    if(volume == 0){
        return 0;
    }

    local_level_f = (vcp_level_f * range_max) / 100 + 0.5f;
    return (uint8_t)local_level_f;
}

static app_le_audio_usb_port_t app_le_audio_usb_get_port(app_usb_audio_port_t port_type, uint8_t port_num)
{
    if((port_type == APP_USB_AUDIO_SPK_PORT)&&(port_num <= APP_LE_AUDIO_USB_PORT_NUM_1)){
        return (APP_LE_AUDIO_USB_PORT_SPK_0 + (port_num - APP_LE_AUDIO_USB_PORT_NUM_0));
    }
    else if((port_type == APP_USB_AUDIO_MIC_PORT)&&(port_num == APP_LE_AUDIO_USB_PORT_NUM_0)){
        return APP_LE_AUDIO_USB_PORT_MIC_0;
    }

    //LE_AUDIO_MSGLOG_I("[APP][USB] get_port, unknown port_type:%x port_num:%x", 2, port_type, port_num);
    return APP_LE_AUDIO_USB_PORT_MAX;
}

static app_le_audio_usb_info_t *app_le_audio_usb_get_info_by_port(app_usb_audio_port_t port_type, uint8_t port_num)
{
    app_le_audio_usb_port_t usb_port = app_le_audio_usb_get_port(port_type, port_num);

    if (APP_LE_AUDIO_USB_PORT_MAX <= usb_port) {
        return NULL;
    }

    return &g_lea_usb_info[usb_port];
}

static bool app_le_audio_usb_check_port_ready(app_le_audio_usb_port_t port)
{
    uint32_t port_num = 0;

    bool ret = FALSE;

    if (APP_LE_AUDIO_USB_PORT_MAX <= port) {
        return FALSE;
    }

    if(APP_LE_AUDIO_USB_PORT_SPK_1 == port) {
        port_num = 1;//for port_type
    }
    else {
        port_num = 0;
    }

    if ((APP_LE_AUDIO_USB_PORT_SPK_0 == port ||
         APP_LE_AUDIO_USB_PORT_SPK_1 == port) &&
        g_lea_usb_info[port].is_streaming &&
        (g_lea_usb_info[port].config.usb_sample_rate = USB_Audio_Get_RX_Sample_Rate(port_num)) &&
        (g_lea_usb_info[port].config.usb_sample_size = USB_Audio_Get_RX_Sample_Size(port_num)) &&
        (g_lea_usb_info[port].config.usb_channel = USB_Audio_Get_RX_Channel(port_num))) {
        ret = TRUE;
    }
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    else if ((APP_LE_AUDIO_USB_PORT_MIC_0 == port) &&
        g_lea_usb_info[port].is_streaming &&
        (g_lea_usb_info[port].config.usb_sample_rate = USB_Audio_Get_TX_Sample_Rate(port_num)) &&
        (g_lea_usb_info[port].config.usb_sample_size = USB_Audio_Get_TX_Sample_Size(port_num)) &&
        (g_lea_usb_info[port].config.usb_channel = USB_Audio_Get_TX_Channel(port_num))) {
        ret = TRUE;
    }
#endif

    LE_AUDIO_MSGLOG_I("[APP][USB] ckeck_port_ready port:%x ret:%d", 2, port, ret);
    return ret;
}

static void app_le_audio_usb_process_usb_event(app_le_audio_usb_event_enum event, app_usb_audio_port_t port_type, uint8_t port_num)
{
    app_le_audio_mode_t mode = app_le_audio_get_current_mode();
    app_le_audio_usb_port_t port = app_le_audio_usb_get_port(port_type, port_num);
    app_le_audio_stream_port_t stream_port = APP_LE_AUDIO_CONVERT_USB_PORT(port);

    LE_AUDIO_MSGLOG_I("[APP][USB] handle_usb_event event:%d port: %x_%x", 3, event, port_type, port_num);

    if (APP_LE_AUDIO_USB_EVENT_SAMPLE_SIZE_OBTAIN == event ||
        APP_LE_AUDIO_USB_EVENT_SAMPLE_RATE_OBTAIN == event ||
        APP_LE_AUDIO_USB_EVENT_CHANNEL_OBTAIN == event ||
        APP_LE_AUDIO_USB_EVENT_ENABLE == event) {
        if (app_le_audio_usb_check_port_ready(port)) {
            /* All USB info is obtained. Try to start */
            app_le_audio_start_streaming_port(stream_port);
        }
    } else if (APP_LE_AUDIO_USB_EVENT_SAMPLE_SIZE_CHANGE == event ||
              APP_LE_AUDIO_USB_EVENT_SAMPLE_RATE_CHANGE == event ||
              APP_LE_AUDIO_USB_EVENT_CHANNEL_CHANGE == event) {
        /* USB info changes. Restart */
        if (!app_le_audio_usb_check_port_ready(port)) {
            return;
        }
        // TODO: no need to restart. Just stop and restart
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        if (APP_LE_AUDIO_MODE_UCST == mode) {
            app_le_audio_ucst_stop(true);
        }
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        if (APP_LE_AUDIO_MODE_BCST == mode) {
            if (port_type != APP_USB_AUDIO_MIC_PORT) {
                app_le_audio_bcst_stop(true);
            }
        }
#endif
    }
}

static bool app_le_audio_usb_check_usb_config_changed(app_events_usb_port_t *p_rate)
{
    app_le_audio_usb_info_t *p_usb_info = NULL;
    app_le_audio_usb_port_t usb_port;
    app_le_audio_stream_port_t stream_port;
    uint32_t sample_rate = 0, rate = 0;
    uint8_t sample_size = 0;
    uint8_t channel = 0;
    app_le_audio_usb_event_enum usb_event = APP_LE_AUDIO_USB_EVENT_MAX;

    /* Ignore value of 0 when it is reported for usb port disable. */
    if (NULL == p_rate) {
        return false;
    }
    uint8_t interface_id = apps_event_usb_get_interface_id_from_port_info(p_rate);
    const apps_usb_interface_enable_app_task_recorder_t *app_record = app_preproc_activity_get_usb_interface_info(interface_id);
    if (app_record == NULL || app_record->sample_rate == 0) {
        return false;
    }

    usb_port = app_le_audio_usb_get_port(p_rate->port_type, p_rate->port_num);
    stream_port = APP_LE_AUDIO_CONVERT_USB_PORT(usb_port);

    p_usb_info = app_le_audio_usb_get_info_by_port(p_rate->port_type, p_rate->port_num);
    if (NULL == p_usb_info) {
        return false;
    }
    //check sample_size changed
    sample_size = app_le_audio_get_usb_sample_size_in_use(stream_port);
    LE_AUDIO_MSGLOG_I("[APP][USB] [%d]SAMPLE_SIZE_%x_%x, usb size:%d stream size:%d", 5, interface_id, p_rate->port_type, p_rate->port_num, app_record->sample_size, sample_size);
    if (!sample_size) {
        usb_event = APP_LE_AUDIO_USB_EVENT_SAMPLE_SIZE_OBTAIN;
    } else if (sample_size != app_record->sample_size) {
        usb_event = APP_LE_AUDIO_USB_EVENT_SAMPLE_SIZE_CHANGE;
    }
    //check channel changed
    channel = app_le_audio_get_usb_channel_in_use(stream_port);
    LE_AUDIO_MSGLOG_I("[APP][USB] [%d]CHANNEL_%x_%x, usb channel:%d stream channel:%d", 5, interface_id, p_rate->port_type, p_rate->port_num, app_record->channel, channel);
    if (!channel) {
        usb_event = APP_LE_AUDIO_USB_EVENT_CHANNEL_OBTAIN;
    } else if (channel != app_record->channel) {
        usb_event = APP_LE_AUDIO_USB_EVENT_CHANNEL_CHANGE;
    }

    //check sample_rate changed
    sample_rate = app_le_audio_get_usb_sample_rate_in_use(stream_port);
    rate = app_record->sample_rate;
    LE_AUDIO_MSGLOG_I("[APP][USB] [%d]SAMPLE_RATE_%x_%x, usb rate:%d stream rate:%d", 5, interface_id, p_rate->port_type, p_rate->port_num, rate, sample_rate);
    if (!sample_rate) {
        usb_event = APP_LE_AUDIO_USB_EVENT_SAMPLE_RATE_OBTAIN;
    } else if (sample_rate != rate) {
        usb_event = APP_LE_AUDIO_USB_EVENT_SAMPLE_RATE_CHANGE;
    }

    if(usb_event != APP_LE_AUDIO_USB_EVENT_MAX){
        app_le_audio_usb_process_usb_event(usb_event, p_rate->port_type, p_rate->port_num);
        return true;
    }

    return false;
}


static bool app_le_audio_usb_handle_play_event(app_events_usb_port_t *p_port)
{
    app_le_audio_usb_port_t port = APP_LE_AUDIO_USB_PORT_MAX;
    uint8_t interface_id;

    if (NULL == p_port) {
        return false;
    }

    LE_AUDIO_MSGLOG_I("[APP][USB] PLAY_%x_%x", 2, p_port->port_type, p_port->port_num);

    port = app_le_audio_usb_get_port(p_port->port_type, p_port->port_num);
    interface_id = apps_event_usb_get_interface_id_from_port_info(p_port);
    if (APP_LE_AUDIO_USB_PORT_MAX <= port ||
        g_lea_usb_info[port].is_streaming || interface_id >= APPS_USB_EVENTS_INTERFACE_MAX) {
        return true;
    }

    g_lea_usb_info[port].is_streaming = true;
    if(!app_le_audio_usb_check_usb_config_changed(p_port)) {
        app_le_audio_usb_process_usb_event(APP_LE_AUDIO_USB_EVENT_ENABLE, p_port->port_type, p_port->port_num);
    }
    return true;
}

static bool app_le_audio_usb_handle_stop_event(app_events_usb_port_t *p_port)
{
    app_le_audio_usb_port_t port;

    if (NULL == p_port) {
        return false;
    }

    LE_AUDIO_MSGLOG_I("[APP][USB] STOP_%x_%x", 2, p_port->port_type, p_port->port_num);

    if (APP_LE_AUDIO_USB_PORT_MAX == (port = app_le_audio_usb_get_port(p_port->port_type, p_port->port_num))) {
        return false;
    }
    g_lea_usb_info[port].is_streaming = false;
    g_lea_usb_info[port].config.usb_channel = 0;
    g_lea_usb_info[port].config.usb_sample_rate = 0;
    g_lea_usb_info[port].config.usb_sample_size = 0;

    app_le_audio_stop_streaming_port(port);
    return true;
}

static bool app_le_audio_usb_handle_volume_event(app_events_usb_volume_t *p_vol)
{
    app_le_audio_usb_info_t *p_usb_info = NULL;
    app_le_audio_usb_port_t port;
    bool adjust_on_remote_device = false;

    if (NULL == p_vol) {
        //LE_AUDIO_MSGLOG_I("[APP][USB] volume_event, invalid data", 0);
        return true;
    }

    if (APP_LE_AUDIO_USB_PORT_MAX == (port = app_le_audio_usb_get_port(p_vol->port_type, p_vol->port_num))) {
        return true;
    }

    p_usb_info = &g_lea_usb_info[port];
    LE_AUDIO_MSGLOG_I("[APP][USB] VOLUME_%x_%x, vol:%x %x->%x %x", 6, p_vol->port_type, p_vol->port_num,
                      p_usb_info->vol_left, p_usb_info->vol_right,
                      p_vol->left_volume, p_vol->right_volume);

    p_vol->left_volume = (p_vol->left_volume > APP_LE_AUDIO_VOL_VALUE_MAX)?APP_LE_AUDIO_VOL_VALUE_MAX:p_vol->left_volume;
    p_vol->right_volume = (p_vol->right_volume > APP_LE_AUDIO_VOL_VALUE_MAX)?APP_LE_AUDIO_VOL_VALUE_MAX:p_vol->right_volume;

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    // PC wake up from sleep,app has not initialized completely. USB volume change will cause AM volume is not max when APP_LE_AUDIO_MODE_NONE.
    if(APP_LE_AUDIO_MODE_UCST == app_le_audio_get_current_mode()) {
#ifndef AIR_VOLUME_CONTROL_BY_DONGLE
        if (APP_USB_AUDIO_SPK_PORT == p_vol->port_type) {
            ble_vcs_volume_t volume;
            adjust_on_remote_device = true;
            if (p_usb_info->vol_left == p_vol->left_volume) {
                return true;
            }
            volume = app_le_audio_map_vol_level(p_vol->left_volume, APP_LE_AUDIO_VOL_RANGE_MAX_255);
            LE_AUDIO_MSGLOG_I("[APP][USB] volume:%d -> %x", 2, p_vol->left_volume, volume);

            app_le_audio_vcp_control_active_device_volume(APP_LEA_VCP_OPERATE_SET_ABSOLUTE_VOLUME, &volume);
        }
#endif

#if AIR_LE_AUDIO_AIRD_MIC_VOLUME_CONTROL_ENABLE
        if (APP_USB_AUDIO_MIC_PORT == p_vol->port_type) {
            adjust_on_remote_device = true;
            if (p_usb_info->vol_left != p_vol->left_volume) {
                app_le_audio_aird_notify_volume_change(p_vol->port_type,
                                                       p_vol->port_num,
                                                       p_vol->left_volume);

            } else if (p_usb_info->vol_right != p_vol->right_volume) {
                app_le_audio_aird_notify_volume_change(p_vol->port_type,
                                                       p_vol->port_num,
                                                       p_vol->right_volume);
            }
        }
#endif
    }
#endif

    if (!adjust_on_remote_device) {
        #ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        if ((p_usb_info->vol_left != p_vol->left_volume) &&
            ((APP_LE_AUDIO_VOL_VALUE_MAX == p_vol->left_volume) || (APP_LE_AUDIO_VOL_VALUE_MIN == p_vol->left_volume))) {
            app_le_audio_aird_notify_volume_change(p_vol->port_type,
                                                   p_vol->port_num,
                                                   p_vol->left_volume);

        } else if ((p_usb_info->vol_right != p_vol->right_volume) &&
            ((APP_LE_AUDIO_VOL_VALUE_MAX == p_vol->right_volume) || (APP_LE_AUDIO_VOL_VALUE_MIN == p_vol->right_volume))) {
            app_le_audio_aird_notify_volume_change(p_vol->port_type,
                                                   p_vol->port_num,
                                                   p_vol->right_volume);
        }
        #endif
        if ((p_usb_info->vol_right != p_vol->right_volume) || (p_usb_info->vol_left != p_vol->left_volume)) {
            app_le_audio_set_audio_transmitter_volume_level(port,
                app_le_audio_map_vol_level(p_vol->left_volume, APP_LE_AUDIO_VOL_RANGE_MAX_15),
                app_le_audio_map_vol_level(p_vol->right_volume, APP_LE_AUDIO_VOL_RANGE_MAX_15));
        }
    }
    p_usb_info->vol_left = p_vol->left_volume;
    p_usb_info->vol_right = p_vol->right_volume;
    return true;
}

static bool app_le_audio_usb_handle_mute_event(app_events_usb_mute_t *p_mute)
{
    app_le_audio_usb_info_t *p_usb_info;
    app_le_audio_usb_port_t port;
    bool adjust_on_remote_device = false;

    if (NULL == p_mute) {
        //LE_AUDIO_MSGLOG_I("[APP][USB] mute_event, invalid data", 0);
        return true;
    }

    if (APP_LE_AUDIO_USB_PORT_MAX == (port = app_le_audio_usb_get_port(p_mute->port_type, p_mute->port_num))) {
        return true;
    }

    p_usb_info = &g_lea_usb_info[port];

    LE_AUDIO_MSGLOG_I("[APP][USB] MUTE_%x_%x, mute:%x->%x", 4, p_mute->port_type,p_mute->port_num,
                          p_usb_info->mute, p_mute->is_mute);

    if (p_mute->is_mute != p_usb_info->mute) {
        p_usb_info->mute = p_mute->is_mute;
    }
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    if(APP_LE_AUDIO_MODE_UCST == app_le_audio_get_current_mode()) {
#ifndef AIR_VOLUME_CONTROL_BY_DONGLE
        if (APP_USB_AUDIO_SPK_PORT == p_mute->port_type) {
            adjust_on_remote_device = true;
            if (p_usb_info->mute) {
                app_le_audio_vcp_control_active_device_volume(APP_LEA_VCP_OPERATE_MUTE, NULL);
            } else {
                app_le_audio_vcp_control_active_device_volume(APP_LEA_VCP_OPERATE_UNMUTE, NULL);
            }
        }
#endif

#if AIR_LE_AUDIO_AIRD_MIC_VOLUME_CONTROL_ENABLE
        if (APP_USB_AUDIO_MIC_PORT == p_mute->port_type) {
            adjust_on_remote_device = true;
            app_le_audio_ucst_notify_mic_mute(p_usb_info->mute);
        }
#endif
    }
#endif

    if (!adjust_on_remote_device) {
        if (p_mute->is_mute) {
            app_le_audio_mute_audio_transmitter(port);
        } else {
            app_le_audio_unmute_audio_transmitter(port);
        }
    }
    return true;
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void app_le_audio_usb_init(void)
{
    uint8_t vol_level = APP_LE_AUDIO_VOL_LEVEL_DEFAULT;
    app_le_audio_usb_port_t port;
#ifndef AIR_VOLUME_CONTROL_BY_DONGLE
    vol_level = APP_LE_AUDIO_VOL_LEVEL_MAX;
#endif
    memset(&g_lea_usb_info, 0, sizeof(app_le_audio_usb_info_t)*APP_LE_AUDIO_USB_PORT_MAX);
    for (port = 0; port < APP_LE_AUDIO_USB_PORT_MAX; port++){
        g_lea_usb_info[port].vol_left = vol_level;
        g_lea_usb_info[port].vol_right = vol_level;
        app_le_audio_set_audio_transmitter_volume_level(port, g_lea_usb_info[port].vol_left, g_lea_usb_info[port].vol_right);
    }
}

bool app_le_audio_idle_usb_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    uint32_t usb_data = (uint32_t)extra_data;
    LE_AUDIO_MSGLOG_I("[APP][USB] usb_event_proc, event_id:%x usb_data:%x", 2, event_id,usb_data);

    switch (event_id) {
        case APPS_EVENTS_USB_AUDIO_UNPLUG:
        case APPS_EVENTS_USB_AUDIO_PLAY:
        case APPS_EVENTS_USB_AUDIO_STOP: {
            app_events_usb_port_t *usb_port = (app_events_usb_port_t *) & (usb_data);
            if (usb_port) {
#if (defined(AIR_USB_AUDIO_ENABLE) && ! defined(APPS_USB_AUDIO_SUPPORT))|| defined (AIR_USB_AUDIO_1_MIC_ENABLE)
                uint8_t interface_id = apps_event_usb_get_interface_id_from_port_info(usb_port);
                if (interface_id < APPS_USB_EVENTS_INTERFACE_MAX) {
                    const apps_usb_interface_enable_app_task_recorder_t *app_recorder = app_preproc_activity_get_usb_interface_info(interface_id);
                    if (app_recorder) {
                        if (app_recorder->enabled) {
                            app_le_audio_usb_handle_play_event((app_events_usb_port_t *) & (usb_data));
                        } else {
                            app_le_audio_usb_handle_stop_event((app_events_usb_port_t *) & (usb_data));
                        }
                    }
                }
#endif
            }
            break;
        }
        case APPS_EVENTS_USB_AUDIO_VOLUME: {
            app_le_audio_usb_handle_volume_event((app_events_usb_volume_t *)extra_data);
            break;
        }
        case APPS_EVENTS_USB_AUDIO_MUTE: {
            app_le_audio_usb_handle_mute_event((app_events_usb_mute_t *) & (usb_data));
            break;
        }
        case APPS_EVENTS_USB_AUDIO_SAMPLE_RATE: {
            app_le_audio_usb_check_usb_config_changed((app_events_usb_port_t *) & (usb_data));
            break;
        }
        default:
            break;
    }

    /* Always return ture to let the event continure to be processed by the following registers. */
    return true;
}

bool app_le_audio_usb_is_port_ready(app_le_audio_usb_port_t port, app_le_audio_usb_config_info_t *p_usb_config_info)
{
    uint32_t usb_sample_rate = 0;
    uint8_t usb_sample_size = 0, usb_channel = 0;

    bool ret = FALSE;

    if (APP_LE_AUDIO_USB_PORT_MAX <= port) {
        return FALSE;
    }

    if ((APP_LE_AUDIO_USB_PORT_SPK_0 == port ||
         APP_LE_AUDIO_USB_PORT_SPK_1 == port) &&
        g_lea_usb_info[port].is_streaming &&
        (usb_sample_rate = g_lea_usb_info[port].config.usb_sample_rate) &&
        (usb_sample_size = g_lea_usb_info[port].config.usb_sample_size) &&
        (usb_channel = g_lea_usb_info[port].config.usb_channel)) {
        ret = TRUE;
    }
    else if ((APP_LE_AUDIO_USB_PORT_MIC_0 == port) &&
        g_lea_usb_info[port].is_streaming &&
        (usb_sample_rate = g_lea_usb_info[port].config.usb_sample_rate) &&
        (usb_sample_size = g_lea_usb_info[port].config.usb_sample_size) &&
        (usb_channel = g_lea_usb_info[port].config.usb_channel)) {
        ret = TRUE;
    }

    if ((NULL != p_usb_config_info) && (TRUE == ret)) {
        p_usb_config_info->usb_sample_rate = usb_sample_rate;
        p_usb_config_info->usb_sample_size = usb_sample_size;
        p_usb_config_info->usb_channel = usb_channel;
    }
    LE_AUDIO_MSGLOG_I("[APP][USB] is_port_ready port:%x ret:%d", 2, port, ret);
    return ret;
}


uint32_t app_le_audio_usb_convert_sample_size(uint8_t sample_size)
{
    if (sample_size >= (sizeof(sample_size_table) / sizeof(uint32_t))) {
        sample_size = 0;
    }
    return sample_size_table[sample_size];
}

/*
uint32_t app_le_audio_usb_convert_sample_rate(uint8_t sample_rate)
{
    if (sample_rate >= (sizeof(sample_rate_table) / sizeof(uint32_t))) {
        sample_rate = 0;
    }
    return sample_rate_table[sample_rate];
}
*/

app_le_audio_usb_port_mask_t app_le_audio_usb_get_streaming_port(void)
{
    uint8_t state = 0, i;

    i = APP_LE_AUDIO_USB_PORT_MAX;
    while (i > 0) {
        i --;
        if (g_lea_usb_info[i].is_streaming) {
            state |= (1 << i);
        }
    }

    return state;
}


void app_le_audio_usb_refresh_volume(void)
{
#ifndef AIR_VOLUME_CONTROL_BY_DONGLE
    uint8_t vol_level = APP_LE_AUDIO_VOL_LEVEL_MAX;
    uint8_t mute = 0;
    app_le_audio_usb_port_t port;
    app_le_audio_mode_t mode = app_le_audio_get_current_mode();
    for (port = 0; port < APP_LE_AUDIO_USB_PORT_MAX; port++){
        if(APP_LE_AUDIO_MODE_BCST == mode) {
            vol_level = g_lea_usb_info[port].vol_left;
            mute = g_lea_usb_info[port].mute;
        }
        app_le_audio_set_audio_transmitter_volume_level(port, vol_level, vol_level);
        if (mute) {
            app_le_audio_mute_audio_transmitter(port);
        } else {
            app_le_audio_unmute_audio_transmitter(port);
        }
    }

#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
    //Have not support control volume currently, so BIS <-->CIS no need refresh volume
    //app_le_audio_set_audio_transmitter_volume_level(APP_LE_AUDIO_STREAM_PORT_LINE_IN, vol_level, vol_level);
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
    //Have not support control volume currently
    //app_le_audio_set_audio_transmitter_volume_level(APP_LE_AUDIO_STREAM_PORT_I2S_IN, vol_level, vol_level);
#endif
#endif
}

bt_status_t app_le_audio_usb_get_volume(app_le_audio_usb_port_t usb_port, uint8_t *volume, uint8_t *mute)
{
    if ((APP_LE_AUDIO_USB_PORT_MAX <= usb_port) || (NULL == volume) || (NULL == mute)) {
        return BT_STATUS_FAIL;
    }

    *volume = app_le_audio_map_vol_level(g_lea_usb_info[usb_port].vol_left, APP_LE_AUDIO_VOL_RANGE_MAX_255);
    *mute = g_lea_usb_info[usb_port].mute;
    return BT_STATUS_SUCCESS;
}

#endif  /* AIR_LE_AUDIO_ENABLE */

