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

#ifdef AIR_WIRELESS_MIC_ENABLE

#include "app_wireless_mic_idle_activity.h"
#include "app_wireless_mic_fatfs.h"
#include "app_wireless_mic_realtime_task.h"
#include "app_wireless_mic_volume_det.h"
#include "apps_config_event_list.h"
#include "apps_config_key_remapper.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_events_key_event.h"
#include "apps_events_mic_control_event.h"
#include "apps_events_battery_event.h"
#include "apps_events_usb_event.h"
#include "apps_debug.h"
#include "app_fota_idle_activity.h"
#include "nvkey.h"
#include "nvkey_id_list.h"

#ifdef MTK_RACE_CMD_ENABLE
#include "apps_dongle_sync_event.h"
#endif

#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
#include "app_power_save_utils.h"
#endif

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
#include "apps_detachable_mic.h"
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "app_ull_idle_activity.h"
#endif

#include "bt_sink_srv_ami.h"
#include "bt_ull_le_service.h"
#include "bt_ull_service.h"

#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#endif

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_control.h"
#endif
#include "audio_src_srv.h"
#include "audio_src_srv_resource_manager_config.h"
#include "audio_transmitter_mcu_dsp_common.h"
#include "audio_transmitter_control.h"
#include "audio_dump.h"

#include "record_control.h"

#ifdef MTK_RACE_CMD_ENABLE
#include "race_event.h"
#endif

#include "hal_audio_message_struct.h"
#ifdef HAL_DVFS_MODULE_ENABLED
#include "hal_dvfs.h"
#include "hal_dvfs_internal.h"
#endif

#ifdef AIR_USB_ENABLE
#include "usb.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_le_service.h"
#endif
#include "bt_hsp.h"

#define LOG_TAG "[APP_WIRELESS_MIC][IDLE]"


typedef enum {
    APP_WIRELESS_MIC_RECORDER_STATE_STOPPED = 0,
    APP_WIRELESS_MIC_RECORDER_STATE_STARTING,
    APP_WIRELESS_MIC_RECORDER_STATE_STARTED,
    APP_WIRELESS_MIC_RECORDER_STATE_STOPPING
} app_wireless_mic_recorder_state_t;

typedef enum {
    APP_WIRELESS_MIC_DVFS_MASK_NONE = 0,
    APP_WIRELESS_MIC_DVFS_MASK_AUDIO,
} app_wireless_mic_dvfs_mask_t;

typedef struct {
    uint8_t bit_mask;
    bool    dvfs_lock;
} app_wireless_mic_dvfs_ctrl_t;

typedef struct {
    bool                                    is_mute;                        /**< If the microphone is muted */
    bool                                    is_first_open;
    bool                                    is_ready_power_off;
    record_id_t                             record_id;
    app_wireless_mic_dvfs_ctrl_t            dvfs_ctl;
    app_wireless_mic_recorder_state_t       recorder_state;
    audio_transmitter_id_t                  transmit_id;                    /**< transmitter id */
    bool                                    is_transmitter_start;           /**< transmitter is start or not */
    bool                                    is_request_transmitter_start;   /**< user request transmitter start or stop */
    uint8_t                                 battery_percent;
    bool                                    is_write_done;
    uint32_t                                data_count;
#ifdef AIR_USB_ENABLE
    app_usb_mode_t                          usb_mode;
    bool                                    usb_plug_in;
#endif
    uint8_t                                 safety_mode;
    bool                                    is_wireless_start;
    bool                                    is_volume_det_start;
    audio_scenario_type_t                   volume_type;
} app_wireless_mic_context_t;

static app_wireless_mic_context_t s_app_wireless_mic_context;

#ifdef APP_WIRELESS_MIC_DEBUG
#include "atci.h"
static atci_status_t app_wireless_mic_atci_handler(atci_parse_cmd_param_t *parse_cmd);
static atci_cmd_hdlr_item_t app_wireless_mic_atci_cmd[] = {
    {
        .command_head = "AT+WMIC",
        .command_hdlr = app_wireless_mic_atci_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};
#endif

#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
#ifdef HAL_DVFS_MODULE_ENABLED
static void app_wireless_mic_dvfs_lock(uint8_t mask)
{
    s_app_wireless_mic_context.dvfs_ctl.bit_mask |= mask;
    if (s_app_wireless_mic_context.dvfs_ctl.bit_mask != APP_WIRELESS_MIC_DVFS_MASK_NONE
        && !s_app_wireless_mic_context.dvfs_ctl.dvfs_lock) {
        s_app_wireless_mic_context.dvfs_ctl.dvfs_lock = TRUE;
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_status_t status = HAL_DVFS_STATUS_ERROR;
#if defined(HAL_DVFS_312M_SOURCE)       // 155x
        status = hal_dvfs_lock_control(DVFS_156M_SPEED, HAL_DVFS_LOCK);
#elif defined(HAL_DVFS_416M_SOURCE)     // 156x
        status = hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_LOCK);
#endif
        APPS_LOG_MSGID_I(LOG_TAG" dvfs_lock: status=%d", 1, status);
#endif
    }
}

static void app_wireless_mic_dvfs_unlock(uint8_t mask)
{
    s_app_wireless_mic_context.dvfs_ctl.bit_mask &= (~mask);
    if (s_app_wireless_mic_context.dvfs_ctl.bit_mask == APP_WIRELESS_MIC_DVFS_MASK_NONE
        && s_app_wireless_mic_context.dvfs_ctl.dvfs_lock) {
        s_app_wireless_mic_context.dvfs_ctl.dvfs_lock = FALSE;
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_status_t status = HAL_DVFS_STATUS_ERROR;
#if defined(HAL_DVFS_312M_SOURCE)
        status = hal_dvfs_lock_control(DVFS_156M_SPEED, HAL_DVFS_UNLOCK);
#elif defined(HAL_DVFS_416M_SOURCE)
        status = hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_UNLOCK);
#endif
        APPS_LOG_MSGID_I(LOG_TAG" dvfs_unlock: status=%d", 1, status);
#endif
    }
}
#endif

static void app_wireless_mic_audio_transmitter_init(void);
static void app_wireless_mic_audio_transmitter_deinit(void);
static void app_wireless_mic_audio_transmitter_reinit(void);


static audio_transmitter_status_t app_wireless_mic_start_audio_transmitter(void)
{
    audio_transmitter_status_t status = AUDIO_TRANSMITTER_STATUS_FAIL;
    app_wireless_mic_dvfs_lock(APP_WIRELESS_MIC_DVFS_MASK_AUDIO);
#ifdef AIR_USB_ENABLE
    app_usb_mode_t  usb_mode = apps_usb_event_get_current_usb_mode();
#endif
    if (AUD_ID_INVALID == s_app_wireless_mic_context.transmit_id
#ifdef AIR_USB_ENABLE
        || usb_mode == APPS_USB_MODE_MSC
#endif
        || app_fota_get_ota_ongoing()
       ) {
#ifdef AIR_USB_ENABLE
        APPS_LOG_MSGID_W(LOG_TAG" start_audio_transmitter fail: id=%d, usb_mode=%d.",
                         2, s_app_wireless_mic_context.transmit_id, usb_mode);
#endif
        goto start_exit;
    }

    FRESULT fatfs_ret     = FR_OK;
    if (!app_wireless_mic_fatfs_get_wav_file_status()) {
        fatfs_ret = app_wireless_mic_fatfs_creat_wav_file();
        if (fatfs_ret != FR_OK) {
            goto start_exit;
        }
    } else {
        goto start_exit;
    }

    if (s_app_wireless_mic_context.is_transmitter_start == false) {
        app_wireless_mic_audio_transmitter_reinit();
        status = audio_transmitter_start(s_app_wireless_mic_context.transmit_id);
        if (status == AUDIO_TRANSMITTER_STATUS_SUCCESS) {
            s_app_wireless_mic_context.is_request_transmitter_start = true;

        }
    }

start_exit:
    APPS_LOG_MSGID_I(LOG_TAG" start_audio_transmitter: status=0x%02x, id=%d.", 2, status, s_app_wireless_mic_context.transmit_id);
    if (status != AUDIO_TRANSMITTER_STATUS_SUCCESS) {
        app_wireless_mic_dvfs_unlock(APP_WIRELESS_MIC_DVFS_MASK_AUDIO);
    }

    return status;
}


static audio_transmitter_status_t app_wireless_mic_stop_audio_transmitter(void)
{
    audio_transmitter_status_t status = AUDIO_TRANSMITTER_STATUS_FAIL;
    if (AUD_ID_INVALID != s_app_wireless_mic_context.transmit_id) {
        s_app_wireless_mic_context.is_request_transmitter_start = false;
#ifdef APP_WIRELESS_MIC_VOLUME_DET_ENABLE
        if (s_app_wireless_mic_context.is_volume_det_start
            && s_app_wireless_mic_context.volume_type == AUDIO_SCENARIO_TYPE_ADVANCED_RECORD_N_MIC) {
            app_wireless_mic_volume_det_stop();
        }
#endif
        if (s_app_wireless_mic_context.is_transmitter_start) {
            status = audio_transmitter_stop(s_app_wireless_mic_context.transmit_id);
        }
    }

    APPS_LOG_MSGID_I(LOG_TAG" stop_audio_transmitter: status=0x%x, id=%d",
                     2, status, s_app_wireless_mic_context.transmit_id);
    s_app_wireless_mic_context.data_count = 0;
    //ui_shell_remove_event(EVENT_GROUP_UI_SHELL_WIRELESS_MIC, APPS_EVENTS_INTERACTION_WIRELESS_MIC_AUDIO_DATA);
    return status;
}

#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
static app_power_saving_target_mode_t app_wireless_mic_get_power_saving_target_mode(void);
#endif

static void app_wireless_mic_audio_transmitter_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    APPS_LOG_MSGID_I(LOG_TAG" audio_transmitter_callback: event=0x%x.", 1, event);

    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            s_app_wireless_mic_context.is_transmitter_start = true;
            APPS_LOG_MSGID_I(LOG_TAG" audio_transmitter_callback: is_request_transmitter_start=%d.", 1, s_app_wireless_mic_context.is_request_transmitter_start);
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
            app_power_save_utils_notify_mode_changed(false, app_wireless_mic_get_power_saving_target_mode);
#endif
            app_wireless_mic_realtime_send_msg(APP_WIRELESS_MIC_REALTIME_MSG_TYPE_RECORDER_DATA, 0, NULL);
            if (AUD_ID_INVALID != s_app_wireless_mic_context.transmit_id
                && false == s_app_wireless_mic_context.is_request_transmitter_start) {
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_stop(s_app_wireless_mic_context.transmit_id)) {
                    APPS_LOG_MSGID_I(LOG_TAG" audio_transmitter_callback: trans_id=0x%x stop fail.", 1, s_app_wireless_mic_context.transmit_id);
                }
            }
#ifdef MTK_RACE_CMD_ENABLE
            app_wireless_mic_idle_send_tx_status(APPS_EVENTS_TX_RECORDER_STATUS, &s_app_wireless_mic_context.is_transmitter_start, sizeof(s_app_wireless_mic_context.is_transmitter_start));
#endif
#ifdef APP_WIRELESS_MIC_VOLUME_DET_ENABLE
            if (s_app_wireless_mic_context.is_volume_det_start && !s_app_wireless_mic_context.is_wireless_start) {
                s_app_wireless_mic_context.volume_type = AUDIO_SCENARIO_TYPE_ADVANCED_RECORD_N_MIC;
                app_wireless_mic_volume_det_start(s_app_wireless_mic_context.volume_type);
            }
#endif
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL:
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            s_app_wireless_mic_context.is_transmitter_start = false;

            app_wireless_mic_realtime_send_msg(APP_WIRELESS_MIC_REALTIME_MSG_TYPE_STOP_RECORDER, 0, NULL);
            APPS_LOG_MSGID_I(LOG_TAG" audio_transmitter_callback: is_request_transmitter_start=%d.", 1, s_app_wireless_mic_context.is_request_transmitter_start);
            //app_wireless_mic_audio_transmitter_deinit();
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
            app_power_save_utils_notify_mode_changed(false, app_wireless_mic_get_power_saving_target_mode);
#endif
            if (AUD_ID_INVALID != s_app_wireless_mic_context.transmit_id
                && s_app_wireless_mic_context.is_request_transmitter_start) {
                //audio_transmitter_start(s_app_wireless_mic_context.transmit_id);
                app_wireless_mic_realtime_send_msg(APP_WIRELESS_MIC_REALTIME_MSG_TYPE_START_RECORDER, 0, NULL);
            }
#ifdef MTK_RACE_CMD_ENABLE
            app_wireless_mic_idle_send_tx_status(APPS_EVENTS_TX_RECORDER_STATUS, &s_app_wireless_mic_context.is_transmitter_start, sizeof(s_app_wireless_mic_context.is_transmitter_start));
#endif
            app_wireless_mic_dvfs_unlock(APP_WIRELESS_MIC_DVFS_MASK_AUDIO);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION: {
            s_app_wireless_mic_context.data_count++;
            //app_wireless_mic_realtime_send_msg(APP_WIRELESS_MIC_REALTIME_MSG_TYPE_RECORDER_DATA, 0, NULL);
            break;
        }
        default:
            break;
    }
}

#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
void bt_ull_wireless_mic_stop_volume_monitor(void)
{
    app_wireless_mic_volume_det_stop();
}
#endif

static void app_wireless_mic_audio_transmitter_init(void)
{
    audio_transmitter_config_t config;
#ifdef AIR_AUDIO_ULD_CODEC_ENABLE
    uint32_t out_channel_mode = 1;
#else
    uint32_t out_channel_mode = (s_app_wireless_mic_context.safety_mode == 0) ? ami_get_stream_in_channel_num(AUDIO_SCENARIO_TYPE_ADVANCED_RECORD_N_MIC) : 1;
#endif

    memset((void *)&config, 0, sizeof(audio_transmitter_config_t));
    config.scenario_type   = AUDIO_TRANSMITTER_ADVANCED_RECORD;
    config.scenario_sub_id = AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC;
    config.msg_handler     = app_wireless_mic_audio_transmitter_callback;
    config.user_data = NULL;

    config.scenario_config.advanced_record_config.n_mic_config.input_codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
    config.scenario_config.advanced_record_config.n_mic_config.input_codec_param.pcm.sample_rate = APP_WIRELESS_MIC_LOCAL_RECORDER_SAMPLE_RATE;
    config.scenario_config.advanced_record_config.n_mic_config.input_codec_param.pcm.channel_mode = ami_get_stream_in_channel_num(AUDIO_SCENARIO_TYPE_ADVANCED_RECORD_N_MIC);
    config.scenario_config.advanced_record_config.n_mic_config.input_codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
    config.scenario_config.advanced_record_config.n_mic_config.input_codec_param.pcm.frame_interval = APP_WIRELESS_MIC_LOCAL_RECORDER_FRAME_INTERVAL; //us

    config.scenario_config.advanced_record_config.n_mic_config.output_codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
    config.scenario_config.advanced_record_config.n_mic_config.output_codec_param.pcm.sample_rate = APP_WIRELESS_MIC_LOCAL_RECORDER_SAMPLE_RATE;
    config.scenario_config.advanced_record_config.n_mic_config.output_codec_param.pcm.channel_mode = out_channel_mode;
    config.scenario_config.advanced_record_config.n_mic_config.output_codec_param.pcm.format = HAL_AUDIO_PCM_FORMAT_S24_LE;
    config.scenario_config.advanced_record_config.n_mic_config.output_codec_param.pcm.frame_interval = APP_WIRELESS_MIC_LOCAL_RECORDER_FRAME_INTERVAL; //us

    if (AUD_ID_INVALID == s_app_wireless_mic_context.transmit_id) {
        s_app_wireless_mic_context.transmit_id = audio_transmitter_init(&config);
    }

    APPS_LOG_MSGID_I(LOG_TAG" audio_transmitter_init: id=%d.", 1, s_app_wireless_mic_context.transmit_id);
}

static void app_wireless_mic_audio_transmitter_deinit(void)
{
    if (AUD_ID_INVALID != s_app_wireless_mic_context.transmit_id) {
        audio_transmitter_deinit(s_app_wireless_mic_context.transmit_id);
        s_app_wireless_mic_context.transmit_id = AUD_ID_INVALID;
    }
    APPS_LOG_MSGID_I(LOG_TAG" audio_transmitter_deinit.", 0);
}

static void app_wireless_mic_audio_transmitter_reinit(void)
{
    app_wireless_mic_audio_transmitter_deinit();
    app_wireless_mic_audio_transmitter_init();

}

#endif

#ifdef AIR_USB_ENABLE
bool app_wireless_mic_is_usb_mass_storage_mode()
{
    return (apps_usb_event_get_current_usb_mode() == APPS_USB_MODE_MSC);
}

#endif


#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
static app_power_saving_target_mode_t app_wireless_mic_get_power_saving_target_mode(void)
{
    app_power_saving_target_mode_t target_mode = APP_POWER_SAVING_TARGET_MODE_SYSTEM_OFF;
    if (s_app_wireless_mic_context.is_transmitter_start) {
        target_mode =  APP_POWER_SAVING_TARGET_MODE_NORMAL;
    }

    APPS_LOG_MSGID_I(LOG_TAG" [POWER_SAVING] target_mode=%d", 1, target_mode);
    return target_mode;
}
#endif

static void app_wireless_mic_update_mic_channel_select(void)
{
#ifdef AIR_AUDIO_ULD_CODEC_ENABLE
    audio_channel_selection_t temp_select = (s_app_wireless_mic_context.safety_mode == 0) ? AUDIO_CHANNEL_SELECTION_MIC1 : AUDIO_CHANNEL_SELECTION_MIC2;
#else
    audio_channel_selection_t temp_select = AUDIO_CHANNEL_SELECTION_MIC_DEFAULE;
#endif
    extern void bt_sink_srv_ami_set_tx_mic_channel_select(audio_channel_selection_t channel);
    bt_sink_srv_ami_set_tx_mic_channel_select(temp_select);
}


static bool app_wireless_mic_idle_proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(LOG_TAG" create current activity : 0x%x", 1, (uint32_t)self);
            self->local_context = &s_app_wireless_mic_context;
            //s_app_wireless_mic_context.transmit_id = AUD_ID_INVALID;
            s_app_wireless_mic_context.is_mute            = false;
            s_app_wireless_mic_context.record_id          = 0;
            s_app_wireless_mic_context.recorder_state     = APP_WIRELESS_MIC_RECORDER_STATE_STOPPED;
            s_app_wireless_mic_context.dvfs_ctl.bit_mask  = 0;
            s_app_wireless_mic_context.dvfs_ctl.dvfs_lock = false;
            s_app_wireless_mic_context.transmit_id        = AUD_ID_INVALID;
            s_app_wireless_mic_context.is_write_done      = true;
            s_app_wireless_mic_context.data_count         = 0;
#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
            app_wireless_mic_audio_transmitter_init();
#endif
            app_wireless_mic_fatfs_init();
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
            app_power_save_utils_register_get_mode_callback(app_wireless_mic_get_power_saving_target_mode);
#endif
#ifdef APP_WIRELESS_MIC_DEBUG
            atci_register_handler(app_wireless_mic_atci_cmd, sizeof(app_wireless_mic_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
#endif
#ifdef AIR_USB_ENABLE
            s_app_wireless_mic_context.usb_mode = apps_usb_event_get_current_usb_mode();
#endif
#ifdef AIR_WIRELESS_MIC_ENABLE
            /* Disable BT profile SDP. */
            bt_hsp_enable_service_record(false);
            bt_hfp_enable_service_record(false);
            bt_a2dp_enable_service_record(false);
            bt_avrcp_disable_sdp(true);
#endif
            uint8_t safety_mode = 0;
            uint32_t safety_mode_size = sizeof(safety_mode);
            nvkey_read_data(NVID_APP_WM_SAFETY_MODE, &safety_mode, &safety_mode_size);
            s_app_wireless_mic_context.safety_mode = safety_mode;
            app_wireless_mic_update_mic_channel_select();
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY: {
            APPS_LOG_MSGID_I(LOG_TAG" destroy", 0);
            app_wireless_mic_fatfs_save_file_num();
            break;
        }
        default:
            break;
    }
    return ret;
}


/**
* @brief      This function is used to handle the key action.
* @param[in]  state, the current sink_state.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_wireless_mic_idle_proc_key_event_group(ui_shell_activity_t *self,
                                                       uint32_t event_id,
                                                       void *extra_data,
                                                       size_t data_len)
{
    bool ret = false;
    app_wireless_mic_context_t *wireless_mic_context = (app_wireless_mic_context_t *)self->local_context;

    uint8_t key_id;
    airo_key_event_t key_event;
    apps_config_key_action_t action;

    /* Decode event_id to key_id and key_event. */
    app_event_key_event_decode(&key_id, &key_event, event_id);

    if (extra_data) {
        action = *(uint16_t *)extra_data;
    } else {
        action = apps_config_key_event_remapper_map_action(key_id, key_event);
    }

    switch (action) {
        case KEY_MUTE_MIC: {
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
            if (!app_ull_is_le_ull_connected()) {
                /* Can not mute/un-mute during the disconnected states*/
                break;
            }
#endif
            wireless_mic_context->is_mute = !wireless_mic_context->is_mute;
            /*Not do: mute key was processed at ull activity. */
#ifdef MTK_RACE_CMD_ENABLE
            app_wireless_mic_idle_send_tx_status(APPS_EVENTS_TX_MIC_STATUS, &wireless_mic_context->is_mute, sizeof(wireless_mic_context->is_mute));
#endif
            break;
        }
#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
        case KEY_WM_CONTROL_LOCAL_RECORDER: {
            if (!wireless_mic_context->is_transmitter_start
                && !wireless_mic_context->is_request_transmitter_start) {
                app_wireless_mic_start_audio_transmitter();
            } else if (wireless_mic_context->is_transmitter_start) {
                app_wireless_mic_stop_audio_transmitter();
            } else {
                APPS_LOG_MSGID_W(LOG_TAG" The recorder operation is not over yet: is_start=%d, is_request_start=%d ",
                                 2, wireless_mic_context->is_transmitter_start, wireless_mic_context->is_request_transmitter_start);
            }
            break;
        }
#endif
        case KEY_WM_SWITCH_USB_TYPE: {
#ifdef AIR_USB_ENABLE
            if (APPS_USB_MODE_MSC  == apps_usb_event_get_current_usb_mode()) {
                s_app_wireless_mic_context.usb_mode = APPS_USB_MODE_WIRELESS_MIC_TX;
                app_wireless_mic_fatfs_update_free_size();
            } else if (APPS_USB_MODE_WIRELESS_MIC_TX == apps_usb_event_get_current_usb_mode()) {
#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
                if (s_app_wireless_mic_context.is_transmitter_start) {
                    app_wireless_mic_stop_audio_transmitter();
                }
#endif
                s_app_wireless_mic_context.usb_mode = APPS_USB_MODE_MSC;
            }
//            usb_drv_disable();
//            usb_drv_enable();
            app_wireless_mic_fatfs_save_file_num();
            apps_usb_event_set_usb_mode(s_app_wireless_mic_context.usb_mode);
            APPS_LOG_MSGID_I(LOG_TAG" switch usb type: type=%d",
                             1, s_app_wireless_mic_context.usb_mode);
#endif
            break;
        }
#ifdef APP_WIRELESS_MIC_VOLUME_DET_ENABLE
        case KEY_WIRELESS_MIC_VOLUME_DET_SWITCH: {
            if (s_app_wireless_mic_context.is_transmitter_start || s_app_wireless_mic_context.is_wireless_start) {
                s_app_wireless_mic_context.is_volume_det_start = !s_app_wireless_mic_context.is_volume_det_start;
                if (s_app_wireless_mic_context.is_volume_det_start) {
                    s_app_wireless_mic_context.volume_type = s_app_wireless_mic_context.is_wireless_start ? AUDIO_SCENARIO_TYPE_BLE_UL : AUDIO_SCENARIO_TYPE_ADVANCED_RECORD_N_MIC;
                    app_wireless_mic_volume_det_start(s_app_wireless_mic_context.volume_type);
                } else {
                    app_wireless_mic_volume_det_stop();
                }
            }
            break;
        }
#endif
        default:
            break;
    }


    APPS_LOG_MSGID_I(LOG_TAG" proc_key_event_group: action=0x%x, ret=%d", 2, action, ret);

    return ret;
}

#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
static bool app_wireless_mic_write_fatfs(uint8_t *data, uint32_t len)
{
    bool ret = true;
    FRESULT fatfs_ret     = FR_OK;

    fatfs_ret = app_wireless_mic_fatfs_write_wav_data(data, len);
    if (fatfs_ret != FR_OK) {
        //app_wireless_mic_fatfs_close_wav_file();
        ret = false;
    }

    return ret;
}

void app_wireless_mic_update_read_point(uint32_t len)
{
    audio_transmitter_read_done(s_app_wireless_mic_context.transmit_id, len);
    return;
}

bool app_wireless_mic_idle_proc_audio_data()
{
    bool ret = true;
    bool write = false;
    uint8_t *data1 = NULL;
    uint32_t len   = 0;
    audio_transmitter_status_t status = AUDIO_TRANSMITTER_STATUS_FAIL;

    do {
        status = audio_transmitter_get_read_information(s_app_wireless_mic_context.transmit_id, &data1, &len);
        if (status == AUDIO_TRANSMITTER_STATUS_SUCCESS) {
            if (len > 0) {
                //LOG_AUDIO_DUMP(data1, len, VOICE_TX_MIC_0);
                {
                    write = app_wireless_mic_write_fatfs(data1, len);
                    if (!write) {
                        app_wireless_mic_stop_audio_transmitter();
                    }
                }
                //update read point at hal_sd_read_blocks_dma_blocking
                //audio_transmitter_read_done(s_app_wireless_mic_context.transmit_id, len);
#ifdef APP_WIRELESS_MIC_DEBUG
                if((len = audio_transmitter_get_available_data_size(s_app_wireless_mic_context.transmit_id)) != 0){
                    APPS_LOG_MSGID_W(LOG_TAG" proc_audio_data: get data size 0x%x", 1,len);
                }
#endif
            }
        } else {
            APPS_LOG_MSGID_W(LOG_TAG" audio_transmitter_callback: get data fail", 0);
        }
        if (s_app_wireless_mic_context.data_count > 0) {
            s_app_wireless_mic_context.data_count--;
        }
    } while (s_app_wireless_mic_context.is_transmitter_start);

    return ret;
}
#endif

/**
* @brief      This function is used to handle the app internal events.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_wireless_mic_idle_proc_app_internal_events(struct _ui_shell_activity *self,
                                                           uint32_t event_id,
                                                           void *extra_data,
                                                           size_t data_len)
{
    bool ret = false;


    switch (event_id) {
#if AIR_AUDIO_DETACHABLE_MIC_ENABLE
        case APPS_EVENTS_INTERACTION_SWITCH_MIC: {
            APPS_LOG_MSGID_I(LOG_TAG": received mic type switch", 0);
#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
            if (s_app_wireless_mic_context.is_transmitter_start) {
                audio_transmitter_status_t stop_ret = app_wireless_mic_stop_audio_transmitter();
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS == stop_ret) {
                    s_app_wireless_mic_context.is_request_transmitter_start = true;
                }
            }
#endif
            break;
        }
#endif
        case APPS_EVENTS_INTERACTION_WIRELESS_MIC_VOLUME_DET: {
#ifdef APP_WIRELESS_MIC_VOLUME_DET_ENABLE
            app_wireless_mic_volume_det_send_data();
#endif
            break;
        }
        default : {
            break;
        }
    }


    return ret;
}

#ifdef MTK_RACE_CMD_ENABLE
static bool app_wireless_mic_idle_proc_rx_control_events(struct _ui_shell_activity *self,
                                                         uint32_t event_id,
                                                         void *extra_data,
                                                         size_t data_len)
{
    bool ret             = false;
    uint32_t event_group = 0;
    void *data           = NULL;


    apps_dongle_event_sync_info_t *pkg = (apps_dongle_event_sync_info_t *)extra_data;
    if (pkg == NULL) {
        return ret;
    }
    APPS_LOG_MSGID_I(LOG_TAG" rx_control_events,group=%d ev=%d.", 2, pkg->event_group, pkg->event_id);
    /* EVENT_GROUP_UI_SHELL_WIRELESS_MIC is 50 and it must be same on dongle and earbuds!!! */
    if (pkg->event_group == EVENT_GROUP_UI_SHELL_WIRELESS_MIC) {
        APPS_LOG_MSGID_I(LOG_TAG" rx_control_events, ev=%d.", 1, pkg->event_id);
        switch (pkg->event_id) {
            /* The Teams application handshake with dongle done. */
            case APPS_EVENTS_MIC_CONTROL_MUTE: {
                event_group = EVENT_GROUP_UI_SHELL_KEY;
                event_id    = INVALID_KEY_EVENT_ID;
                uint16_t *key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
                if (key_action != NULL) {
                    *key_action = (uint16_t)KEY_MUTE_MIC;
                    data = (void *)key_action;
                    data_len = sizeof(uint16_t);
                }
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH, event_group, event_id, data, data_len, NULL, 0);
                break;
            }
            /* It will not be received now. */
            case APPS_EVENTS_MIC_CONTROL_LOCAL_RECORDER: {
                event_group = EVENT_GROUP_UI_SHELL_KEY;
                event_id    = INVALID_KEY_EVENT_ID;
                uint16_t *key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
                if (key_action != NULL) {
                    *key_action = (uint16_t)KEY_WM_CONTROL_LOCAL_RECORDER;
                    data = (void *)key_action;
                    data_len = sizeof(uint16_t);
                }
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH, event_group, event_id, data, data_len, NULL, 0);
                break;
            }
            case APPS_EVENTS_MIC_CONTROL_SAFETY_MODE: {
                uint8_t received_safety_mode = pkg->data[0];
                if (s_app_wireless_mic_context.safety_mode != received_safety_mode) {
                    s_app_wireless_mic_context.safety_mode = received_safety_mode;
                    nvkey_write_data(NVID_APP_WM_SAFETY_MODE, &s_app_wireless_mic_context.safety_mode, sizeof(s_app_wireless_mic_context.safety_mode));
                    app_wireless_mic_update_mic_channel_select();
                }
                break;
            }
            default:
                break;

        }
    }

    return ret;
}

void app_wireless_mic_idle_send_tx_status(app_mic_tx_status_event_t type, void *data, uint32_t data_len)
{
    if (data == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" send_tx_status: data is NULL.", 0);
        return;
    }

    APPS_LOG_MSGID_I(LOG_TAG" send_tx_status: type=%d, data_len=%d.", 2, type, data_len);

    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_WIRELESS_MIC, type, data, data_len);
}

#endif

#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
static bool app_wireless_mic_idle_proc_battery_event_group(ui_shell_activity_t *self,
                                                           uint32_t event_id,
                                                           void *extra_data,
                                                           size_t data_len)
{
    bool ret = false;
    app_wireless_mic_context_t *wireless_mic_context = (app_wireless_mic_context_t *)self->local_context;
    if (wireless_mic_context == NULL) {
        return ret;
    }

    switch (event_id) {
        case APPS_EVENTS_BATTERY_PERCENT_CHANGE: {
            /* Report it to remote device when battery percent changed. */
            uint8_t pre_battery  = wireless_mic_context->battery_percent;
            uint8_t curr_battery = (int32_t)extra_data;

            APPS_LOG_MSGID_I(LOG_TAG", pre_bat: %d, curr_bat = %d", 2, pre_battery, curr_battery);

            if (curr_battery != pre_battery) {
                wireless_mic_context->battery_percent = curr_battery;
#ifdef MTK_RACE_CMD_ENABLE
                app_wireless_mic_idle_send_tx_status(APPS_EVENTS_TX_BATTERY_STATUS, &wireless_mic_context->battery_percent, sizeof(wireless_mic_context->battery_percent));
#endif
            }
            break;
        }
        default : {
            break;
        }
    }

    return ret;
}
#endif


static bool app_wireless_mic_idle_proc_wireless_event_group(struct _ui_shell_activity *self,
                                                            uint32_t event_id,
                                                            void *extra_data,
                                                            size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        case APPS_EVENTS_INTERACTION_WIRELESS_MIC_AUDIO_DATA: {
            APPS_LOG_MSGID_I(LOG_TAG" receive wireless mic audio data ", 0);
#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
            ret = app_wireless_mic_idle_proc_audio_data();
#endif
            break;
        }
        default : {
            break;
        }
    }

    return ret;
}

static bool app_wireless_mic_idle_proc_ull_event(ui_shell_activity_t *self,
                                                 uint32_t event_id,
                                                 void *extra_data,
                                                 size_t data_len)
{
    bool ret = false;

    app_wireless_mic_context_t *wireless_mic_context = (app_wireless_mic_context_t *)self->local_context;
    if (wireless_mic_context == NULL) {
        return ret;
    }

    switch (event_id) {

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        case BT_ULL_EVENT_LE_CONNECTED: {
            bt_ull_le_connected_info_t *con_info = (bt_ull_le_connected_info_t*)extra_data;
            if (con_info->status != BT_STATUS_SUCCESS) {
                break;
            }
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
            wireless_mic_context->battery_percent = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE)
            wireless_mic_context->battery_percent = apps_events_get_optimal_battery();
#endif

#ifdef MTK_RACE_CMD_ENABLE
            app_wireless_mic_idle_send_tx_status(APPS_EVENTS_TX_BATTERY_STATUS, &wireless_mic_context->battery_percent, sizeof(wireless_mic_context->battery_percent));
            app_wireless_mic_idle_send_tx_status(APPS_EVENTS_TX_MIC_STATUS, &wireless_mic_context->is_mute, sizeof(wireless_mic_context->is_mute));
            app_wireless_mic_idle_send_tx_status(APPS_EVENTS_TX_RECORDER_STATUS, &wireless_mic_context->is_transmitter_start, sizeof(wireless_mic_context->is_transmitter_start));
#endif
            break;
        }
        case BT_ULL_EVENT_LE_STREAMING_START_IND:
        case BT_ULL_EVENT_LE_STREAMING_STOP_IND:
            s_app_wireless_mic_context.is_wireless_start = (event_id == BT_ULL_EVENT_LE_STREAMING_START_IND) ? true : false;
#ifdef APP_WIRELESS_MIC_VOLUME_DET_ENABLE
            if (s_app_wireless_mic_context.is_volume_det_start) {
                if (event_id == BT_ULL_EVENT_LE_STREAMING_START_IND
                    && s_app_wireless_mic_context.volume_type != AUDIO_SCENARIO_TYPE_BLE_UL) {
                    app_wireless_mic_volume_det_stop();
                    s_app_wireless_mic_context.volume_type = AUDIO_SCENARIO_TYPE_BLE_UL;
                    app_wireless_mic_volume_det_start(s_app_wireless_mic_context.volume_type);
                }
                if (event_id == BT_ULL_EVENT_LE_STREAMING_STOP_IND
                    && s_app_wireless_mic_context.volume_type == AUDIO_SCENARIO_TYPE_BLE_UL) {
                    //app_wireless_mic_volume_det_stop();
                    if (s_app_wireless_mic_context.is_transmitter_start) {
                        s_app_wireless_mic_context.volume_type = AUDIO_SCENARIO_TYPE_ADVANCED_RECORD_N_MIC;
                        app_wireless_mic_volume_det_start(s_app_wireless_mic_context.volume_type);
                    }
                }
            }
#endif
            break;
#endif
        default:
            break;
    }

    return ret;
}

#ifdef MTK_RACE_CMD_ENABLE
static bool app_wireless_mic_idle_proc_fota_group(struct _ui_shell_activity *self,
                                                  uint32_t event_id,
                                                  void *extra_data,
                                                  size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        case RACE_EVENT_TYPE_FOTA_START: {
#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
            app_wireless_mic_stop_audio_transmitter();
#endif
            break;
        }
        default:
            break;
    }
    return ret;
}
#endif

bool app_wireless_mic_idle_activity_proc(ui_shell_activity_t *self,
                                         uint32_t event_group,
                                         uint32_t event_id,
                                         void *extra_data,
                                         size_t data_len)
{
    bool ret = false;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = app_wireless_mic_idle_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_KEY: {
            /* Key event. */
            ret = app_wireless_mic_idle_proc_key_event_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            /* Events come from app interation. */
            ret = app_wireless_mic_idle_proc_app_internal_events(self, event_id, extra_data, data_len);
            break;
        }
#ifdef MTK_RACE_CMD_ENABLE
        case EVENT_GROUP_UI_SHELL_DONGLE_DATA: {
            ret = app_wireless_mic_idle_proc_rx_control_events(self, event_id, extra_data, data_len);
            break;
        }
#endif
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
        case EVENT_GROUP_UI_SHELL_BATTERY: {
            /* Battery event proc.*/
            ret  = app_wireless_mic_idle_proc_battery_event_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_WIRELESS_MIC: {
            ret  = app_wireless_mic_idle_proc_wireless_event_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY: {
            ret = app_wireless_mic_idle_proc_ull_event(self, event_id, extra_data, data_len);
            break;
        }
#ifdef MTK_RACE_CMD_ENABLE
        case EVENT_GROUP_UI_SHELL_FOTA: {
            /* FOTA events sent by race_cmd_fota. */
            ret = app_wireless_mic_idle_proc_fota_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
        /*
        case EVENT_GROUP_UI_SHELL_USB_AUDIO: {
            ret = app_wireless_mic_idle_proc_usb_audio_event(self, event_id, extra_data, data_len);
            break;
        }
        */
        default:
            break;
    }
    return ret;
}

bool app_wireless_mic_is_safety_mode(void)
{
    return (s_app_wireless_mic_context.safety_mode != 0) ? true : false;
}

bool app_wireless_mic_is_ready_power_off(void)
{
    bool ret = true;
#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
    if (!s_app_wireless_mic_context.is_transmitter_start) {
        return ret;
    } else {
        app_wireless_mic_stop_audio_transmitter();
        s_app_wireless_mic_context.is_ready_power_off = true;
        ret = false;
    }
#endif
    APPS_LOG_MSGID_I(LOG_TAG" is_ready_power_off=%d", 1, ret);
    return ret;
}

void app_wireless_mic_stop_local_recorder(void)
{
#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
    app_wireless_mic_stop_audio_transmitter();
#endif
}

void app_wireless_mic_start_local_recorder(void)
{
#if defined(AIR_RECORD_ADVANCED_ENABLE) && defined(AIR_AUDIO_TRANSMITTER_ENABLE) && defined(AIR_LOCAL_RECORDER_ENABLE)
    app_wireless_mic_start_audio_transmitter();
#endif
}


bool app_wireless_mic_get_ready_power_off_state(void)
{
    return s_app_wireless_mic_context.is_ready_power_off;
}

void app_wireless_mic_set_ready_power_off_state(bool is_ready)
{
    s_app_wireless_mic_context.is_ready_power_off = is_ready;
}

#ifdef APP_WIRELESS_MIC_DEBUG
static atci_status_t app_wireless_mic_atci_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            char *atcmd = parse_cmd->string_ptr + parse_cmd->name_len + 1;
            char cmd[20] = {0};
            uint32_t copy_len = strlen(atcmd) - 2;
            memcpy(cmd, atcmd, (copy_len > 19 ? 19 : copy_len));
            APPS_LOG_I(LOG_TAG" AT+WMIC=%s", cmd);
            if (strstr(cmd, "record") > 0) {
                app_wireless_mic_recorder_state_t type = 0;
                sscanf(cmd, "record,%d", (int *)&type);
                s_app_wireless_mic_context.recorder_state = type;
                switch (s_app_wireless_mic_context.recorder_state) {
                    case APP_WIRELESS_MIC_RECORDER_STATE_STARTED: {
                        app_wireless_mic_fatfs_creat_wav_file();
                        break;
                    }
                    case APP_WIRELESS_MIC_RECORDER_STATE_STOPPED: {
                        app_wireless_mic_fatfs_close_wav_file();
                        break;
                    }
                    default:
                        break;
                }
                APPS_LOG_MSGID_I(LOG_TAG" record_type=%d", 1, s_app_wireless_mic_context.recorder_state);
            } else if (strstr(cmd, "fatfs") > 0) {
                if (s_app_wireless_mic_context.recorder_state == APP_WIRELESS_MIC_RECORDER_STATE_STARTED
                    && app_wireless_mic_fatfs_get_wav_file_status()) {
                    app_wireless_mic_fatfs_write_wav_data((uint8_t *)cmd, copy_len);
                }
            } else if (strstr(cmd, "header") > 0) {
                app_wireless_mic_fatfs_creat_wav_file();
                app_wireless_mic_fatfs_close_wav_file();
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" invalid wireless mic AT-CMD", 0);
            }
            memset(response.response_buf, 0, ATCI_UART_TX_FIFO_BUFFER_SIZE);
            snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "OK - %s\r\n", atcmd);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}
#endif

#endif //AIR_WIRELESS_MIC_ENABLE
