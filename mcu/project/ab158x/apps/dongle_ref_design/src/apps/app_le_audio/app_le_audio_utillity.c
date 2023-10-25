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
#include "bt_le_audio_def.h"
#include "app_le_audio.h"
#include "app_le_audio_usb.h"
#include "app_le_audio_utillity.h"
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
#include "app_le_audio_ucst.h"
#include "app_le_audio_ucst_utillity.h"
#include "app_le_audio_silence_detection.h"
#include "app_le_audio_ccp_call_control_server.h"
#include "app_le_audio_vcp_volume_controller.h"
#include "app_le_audio_micp_micophone_controller.h"
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_le_audio_bcst.h"
#include "app_le_audio_bcst_utillity.h"
#endif

#include "bt_le_audio_source.h"

#include "apps_events_event_group.h"
#include "ui_shell_manager.h"


#include "hal_usb.h"

#include "bt_le_audio_msglog.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
#define APP_LE_AUDIO_DONGLE_SIMULATE_MEDIA_STATE 1
#else
#define APP_LE_AUDIO_DONGLE_SIMULATE_MEDIA_STATE 0
#endif

/* IN_SAMPLE_RATE */
#define IN_SAMPLE_RATE_8KHZ     8
#define IN_SAMPLE_RATE_16KHZ    16
#define IN_SAMPLE_RATE_24KHZ    24
#define IN_SAMPLE_RATE_32KHZ    32
#define IN_SAMPLE_RATE_44_1KHZ  44
#define IN_SAMPLE_RATE_48KHZ    48

/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/
app_le_audio_ctrl_t g_lea_ctrl;

const uint32_t g_lea_sdu_interval_tbl[] = {
/*  sdu_interval (us)   app_le_audio_sdu_interval_t */
    7500,               /* SDU_INTERVAL_7P5_MS */
    10000,              /* SDU_INTERVAL_10_MS */
};

app_le_audio_timer_info_struct g_lea_timer_info[APP_LE_AUDIO_TIMER_MAX_NUM];

uint8_t g_streaming_port;

/**************************************************************************************************
* Prototype
**************************************************************************************************/

/**************************************************************************************************
* Static Functions
**************************************************************************************************/
uint8_t app_le_audio_get_sample_freq(uint8_t in_smaple_rate)
{
    switch (in_smaple_rate) {
        case IN_SAMPLE_RATE_8KHZ:
            return CODEC_CONFIGURATION_SAMPLING_FREQ_8KHZ;
        case IN_SAMPLE_RATE_16KHZ:
            return CODEC_CONFIGURATION_SAMPLING_FREQ_16KHZ;
        case IN_SAMPLE_RATE_24KHZ:
            return CODEC_CONFIGURATION_SAMPLING_FREQ_24KHZ;
        case IN_SAMPLE_RATE_32KHZ:
            return CODEC_CONFIGURATION_SAMPLING_FREQ_32KHZ;
        case IN_SAMPLE_RATE_44_1KHZ:
            return CODEC_CONFIGURATION_SAMPLING_FREQ_44_1KHZ;
        case IN_SAMPLE_RATE_48KHZ:
            return CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ;
    }
    return APP_LE_AUDIO_SAMPLING_FREQ_INVALID;
}

uint32_t app_le_audio_convert_sample_freq(uint8_t sampling_freq)
{
    switch (sampling_freq) {
        case CODEC_CONFIGURATION_SAMPLING_FREQ_8KHZ:
            return 8000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_16KHZ:
            return 16000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_24KHZ:
            return 24000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_32KHZ:
            return 32000;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_44_1KHZ:
            return 44100;
        case CODEC_CONFIGURATION_SAMPLING_FREQ_48KHZ:
            return 48000;
        default:
            break;
    }
    return 0;
}

app_le_audio_mode_t app_le_audio_get_current_mode(void)
{
    return g_lea_ctrl.curr_mode;
}

bt_status_t app_le_audio_setup_iso_data_path(bt_handle_t handle, bt_gap_le_iso_data_path_direction_t direction, bt_gap_le_iso_data_path_id_t id)
{
    bt_status_t ret;
    bt_gap_le_setup_iso_data_path_t param = {
        .handle = handle,
        .direction = direction,
        .data_path_id = id, /* should use AVM buffer#1 instead of BT_GAP_LE_ISO_DATA_PATH_ID_HCI */
        .codec_format = 0x00,
        .company_id = 0x0000,
        .vendor_codec_id = 0x0000,
        .controller_delay = 0,
        .codec_configuration_length = 0,
        .codec_configuration = NULL
    };

    ret = bt_gap_le_setup_iso_data_path(&param);

    LE_AUDIO_MSGLOG_I("[APP] setup_iso_data_path, handle:%x direction:%x id:%x ret:%x", 4, handle, direction, id, ret);

    return ret;
}

bt_status_t app_le_audio_remove_iso_data_path(bt_handle_t handle, uint8_t direction)
{
    bt_status_t ret;
    bt_gap_le_remove_iso_data_path_t param = {
        .handle = handle,
        .data_path_direction = direction,
    };

    ret = bt_gap_le_remove_iso_data_path(&param);

    LE_AUDIO_MSGLOG_I("[APP] remove_iso_data_path, handle:%x direction:%x ret:%x", 3, handle, direction, ret);

    return ret;
}




void app_le_audio_timer_reset_timer_info(app_le_audio_timer_info_struct *timer_info)
{
    if (timer_info) {
        memset(timer_info, 0, sizeof(app_le_audio_timer_info_struct));
    }
}


app_le_audio_timer_info_struct *app_le_audio_timer_get_timer_info_by_timer_handle(TimerHandle_t timer_handle)
{
    uint32_t i = 0;

    if (!timer_handle) {
        return NULL;
    }

    for (; APP_LE_AUDIO_TIMER_MAX_NUM > i; i++) {
        if (timer_handle == g_lea_timer_info[i].timer_handle) {
            return &g_lea_timer_info[i];
        }
    }

    return NULL;
}


app_le_audio_timer_info_struct *app_le_audio_timer_get_empty_timer_info(void)
{
    uint32_t i = 0;

    for (; APP_LE_AUDIO_TIMER_MAX_NUM > i; i++) {
        if (NULL == g_lea_timer_info[i].timer_handle) {
            return &g_lea_timer_info[i];
        }
    }

    return NULL;
}


void app_le_audio_timer_handle_timer_expired_event(TimerHandle_t timer_handle)
{
    app_le_audio_timer_info_struct *timer_info = app_le_audio_timer_get_timer_info_by_timer_handle(timer_handle);

    if (timer_info) {
        if (timer_info->callback) {
            timer_info->callback(timer_handle, timer_info->user_data);
        }

        xTimerDelete(timer_info->timer_handle, 0);
        app_le_audio_timer_reset_timer_info(timer_info);
    }
}


void app_le_audio_timer_callback(TimerHandle_t timer_handle)
{
    ui_shell_send_event(TRUE,
                        EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_LE_AUDIO,
                        APP_LE_AUDIO_EVENT_TIMER_EXPIRED,
                        timer_handle,
                        0,
                        NULL,
                        0);
}


/* timer_period is in ms. The timer is identified by timer_handle only. The user should manage the allocating and the freeing of user_data. */
bt_status_t app_le_audio_timer_start(TimerHandle_t *timer_handle, uint32_t timer_period, app_le_audio_timer_callback_t callback, void *user_data)
{
    app_le_audio_timer_info_struct *timer_info = NULL;

    if (!timer_handle || !timer_period || !callback) {
        return BT_STATUS_FAIL;
    }

    *timer_handle = NULL;

    timer_info = app_le_audio_timer_get_empty_timer_info();
    if (!timer_info) {
        /* No empty timer info. */
        return BT_STATUS_TIMER_FULL;
    }

    timer_info->timer_handle = xTimerCreate("g_lea_timer",
                                            (timer_period / portTICK_PERIOD_MS),
                                            pdFALSE,
                                            NULL,
                                            app_le_audio_timer_callback);

    if (timer_info->timer_handle) {
        *timer_handle = timer_info->timer_handle;
        timer_info->callback = callback;
        timer_info->user_data = user_data;
        xTimerStart(timer_info->timer_handle, 0);
        LE_AUDIO_MSGLOG_I("[APP] start timer. handle:%x period:%x", 2, timer_info->timer_handle, timer_period);
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}


void app_le_audio_timer_stop(TimerHandle_t timer_handle)
{
    app_le_audio_timer_info_struct *timer_info = app_le_audio_timer_get_timer_info_by_timer_handle(timer_handle);

    if (timer_info) {
        LE_AUDIO_MSGLOG_I("[APP] stop timer. handle:%x", 1, timer_handle);
        xTimerStop(timer_info->timer_handle, 0);
        xTimerDelete(timer_info->timer_handle, 0);
        app_le_audio_timer_reset_timer_info(timer_info);
    }
}


bool app_le_audio_handle_idle_le_audio_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* Do not return true due to other apps will listen these events. */
    bool ret = false;

    switch (event_id) {
        case APP_LE_AUDIO_EVENT_TIMER_EXPIRED: {
            app_le_audio_timer_handle_timer_expired_event((TimerHandle_t) extra_data);
            break;
        }
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        case APP_LE_AUDIO_EVENT_VCP_RETRY: {
            app_le_audio_vcp_handle_retry_event((uint32_t)extra_data);
            break;
        }
        case APP_LE_AUDIO_EVENT_MICP_RETRY: {
            app_le_audio_micp_handle_retry_event((uint32_t)extra_data);
            break;
        }
#endif
        default:
            break;
    }

    return ret;
}

void app_le_audio_set_streaming_port_mask(app_le_audio_stream_port_mask_t mask)
{
    g_streaming_port |= mask;
}

void app_le_audio_clear_streaming_port_mask(app_le_audio_stream_port_mask_t mask)
{
    g_streaming_port &= ~mask;
}

uint8_t app_le_audio_get_streaming_port(void)
{
    uint8_t usb_straming_port;
    usb_straming_port = app_le_audio_usb_get_streaming_port();
    g_streaming_port = (g_streaming_port & 0xf8) | usb_straming_port;

    return g_streaming_port;
}

bt_status_t app_le_audio_start_streaming_port(app_le_audio_stream_port_t port)
{
    app_le_audio_mode_t mode = app_le_audio_get_current_mode();

#if APP_LE_AUDIO_DONGLE_SIMULATE_MEDIA_STATE
    uint8_t streaming_port = app_le_audio_get_streaming_port();

    uint8_t streaming_port_cnt = 0;

    // check if SPK from stop to play, if yes, send MediaState=playing
    if (APP_LE_AUDIO_STREAM_PORT_MIC_0 != port) {
        for(uint8_t i = 0; i < APP_LE_AUDIO_STREAM_PORT_MAX; i++) {
            if (i == APP_LE_AUDIO_STREAM_PORT_MIC_0) {
                continue;
            }
            if ((streaming_port>>i) & 0x01) {
                streaming_port_cnt++;
            }
        }
#if AIR_MS_TEAMS_ENABLE
        LE_AUDIO_MSGLOG_I("[APP][USB] PLAY, streaming_port(map):%x call_exist:%d", 2, streaming_port, app_le_audio_usb_hid_call_existing());
        if (streaming_port_cnt == 1) {
            //Only any one of SPK0 & SPK1 & Line-In & I2S-Inis become play firstly,set mcs state.
            if (!app_le_audio_usb_hid_call_existing()) {
                bt_le_audio_source_action_param_t le_param;
                le_param.service_index = ble_mcs_get_gmcs_service_idx();
                bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_MEDIA_PLAY, &le_param);
            }
        }
#else
        LE_AUDIO_MSGLOG_I("[APP][USB] PLAY, streaming_port(map):%x call_exist:%d", 2, streaming_port, false);
        if (streaming_port_cnt == 1) {
            bt_le_audio_source_action_param_t le_param;
            le_param.service_index = ble_mcs_get_gmcs_service_idx();
            bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_MEDIA_PLAY, &le_param);
        }
#endif
    }
#endif

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    app_le_audio_ucst_ctrl_t *ucst_ctrl = app_le_audio_ucst_get_ctrl();

    if (APP_LE_AUDIO_MODE_UCST == mode) {
        if (APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL <= app_le_audio_ucst_get_pause_stream_flag()) {
            return BT_STATUS_SUCCESS;
        }

        if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port ||
            APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE != ucst_ctrl->curr_target ||
            ((APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == ucst_ctrl->curr_target) && //Error handle: start call fail
            (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == ucst_ctrl->curr_stream_state &&
             APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == ucst_ctrl->next_stream_state))) {
            /* MIC port only controls the call mode. */
            app_le_audio_ucst_stop_delay_disconnect_timer();
            app_le_audio_ucst_start();
            return BT_STATUS_SUCCESS;
        }

        if ((APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == ucst_ctrl->curr_target ||
             APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == ucst_ctrl->curr_target) &&
            APP_LE_AUDIO_UCST_TARGET_NONE == ucst_ctrl->next_target) {
            if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(port)) {
                app_le_audio_start_audio_transmitter(port);
            }
        }
    }
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    if (APP_LE_AUDIO_MODE_BCST == mode) {
        if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) {
            return BT_STATUS_FAIL;
        }

        if (app_le_audio_bcst_gat_curr_state() == APP_LE_AUDIO_BCST_STATE_IDLE) {
            app_le_audio_bcst_start();
            return BT_STATUS_SUCCESS;
        }

        if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(port)) {
            app_le_audio_start_audio_transmitter(port);
        }
    }
#endif
    return BT_STATUS_SUCCESS;
}

bt_status_t app_le_audio_stop_streaming_port(app_le_audio_stream_port_t port)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE

#if APP_LE_AUDIO_DONGLE_SIMULATE_MEDIA_STATE
    uint8_t streaming_port = app_le_audio_get_streaming_port();

    // check if SPK from play to stop, if yes, send MediaState=paused
    if (APP_LE_AUDIO_STREAM_PORT_MIC_0 != port) {
#if AIR_MS_TEAMS_ENABLE
        LE_AUDIO_MSGLOG_I("[APP][USB] STOP, streaming_port(map):%x call_exist:%d", 2, streaming_port, app_le_audio_usb_hid_call_existing());
        if (!(streaming_port & (~APP_LE_AUDIO_STREAM_PORT_MASK_MIC_0))) {
            if (!app_le_audio_usb_hid_call_existing()) {
                bt_le_audio_source_action_param_t le_param;
                le_param.service_index = ble_mcs_get_gmcs_service_idx();
                bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_MEDIA_PAUSE, &le_param);
            }
        }
#else
        LE_AUDIO_MSGLOG_I("[APP][USB] STOP, streaming_port(map):%x call_exist:%d", 2, streaming_port, false);
        if (!(streaming_port & (~APP_LE_AUDIO_STREAM_PORT_MASK_MIC_0))) {
            bt_le_audio_source_action_param_t le_param;
            le_param.service_index = ble_mcs_get_gmcs_service_idx();
            bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_MEDIA_PAUSE, &le_param);
        }
#endif
    }
#endif

    if (APP_LE_AUDIO_MODE_UCST == app_le_audio_get_current_mode()) {
        app_le_audio_ucst_target_t curr_target = app_le_audio_ucst_get_curr_target();

        LE_AUDIO_MSGLOG_I("[APP][USB][U] STOP, curr_target:%x port%x streaming_port:%x", 3, curr_target, port, streaming_port);

        if ((APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == curr_target) ||
            (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == curr_target)) {

            if ((!streaming_port) || (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port)) {//Music or Call need to stop,but wait for 3s if reopen
                app_le_audio_ucst_start_delay_disconnect_timer();
            }
            app_le_audio_stop_audio_transmitter(port);
        }
#ifdef AIR_SILENCE_DETECTION_ENABLE
        else if (APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE == curr_target) {
            app_le_audio_silence_detection_handle_event(APP_LE_AUDIO_SILENCE_DETECTION_EVENT_PORT_DISABLED, (void *)port);
        }
#endif

    }
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    if (APP_LE_AUDIO_MODE_BCST == app_le_audio_get_current_mode()) {
        if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) {
            return BT_STATUS_SUCCESS;
        }
        app_le_audio_stop_audio_transmitter(port);
    }
#endif
    return BT_STATUS_SUCCESS;

}

#endif  /* AIR_LE_AUDIO_ENABLE */

