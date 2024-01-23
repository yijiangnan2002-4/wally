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
 * File: app_usb_audio_idle_activity.c
 *
 * Description:
 * This file is the activity to process the play or stop of USB audio. This app accepts
 * events from the USB host to play or stop USB audio, set/mute/unmute the volume of USB
 * audio. It will also resume USB audio after HFP ends.
 */

//#include "app_usb_audio_utils.h"
#include "usbaudio_drv.h"
#include "apps_events_key_event.h"
#include "apps_events_usb_event.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "app_audio_trans_mgr.h"
#include "ui_shell_manager.h"
#include "ui_shell_activity.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_config_event_list.h"
#include "apps_config_state_list.h"
#include "apps_config_key_remapper.h"
#include "apps_debug.h"
#include "stdlib.h"
#include "bt_sink_srv.h"
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
#include "voice_prompt_api.h"
#endif
#include "atci.h"
#include "project_config.h"

#define USB_AUDIO_LOG_I(msg, ...)     APPS_LOG_MSGID_I("[USB_AUDIO_MMI]"msg, ##__VA_ARGS__)
#define USB_AUDIO_LOG_E(msg, ...)     APPS_LOG_MSGID_E("[USB_AUDIO_MMI]"msg, ##__VA_ARGS__)
#define USB_AUDIO_LOG_D(msg, ...)     APPS_LOG_MSGID_D("[USB_AUDIO_MMI]"msg, ##__VA_ARGS__)

#define DEFAULT_VOLUME 13
#define USB_AUDIO_MAX_LEVEL 15
#define USB_AUDIO_MAX_VOLUME 100

#define ULL_MIX_RATIO_GAME_MAX_LEVEL    (0)     /* Gaming is 100%, Chat is 0% */
#define ULL_MIX_RATIO_CHAT_MAX_LEVEL    (20)    /* Gaming is 0%, Chat is 100% */
#define ULL_MIX_RATIO_BALANCED_LEVEL    (10)    /* Gaming is 100%, Chat is 100% */

#define USB_AUDIO_SCENARIO_TYPE_COMMON_USB_IN 1
#define USB_AUDIO_SCENARIO_TYPE_MIX_OUT 0

static volatile bool s_usb_plugged_in = false;
#ifdef AIR_USB_AUDIO_IN_ENABLE
static void *s_usb_in1_handle = NULL;
static void *s_usb_in2_handle = NULL;
#endif
#ifdef AIR_USB_AUDIO_OUT_ENABLE
static void *s_usb_out_handle = NULL;
#endif
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
static void *s_usb_in_out_mix_handle = NULL;
static bool s_usb_in_out_mix_started = false;
#endif
#define APP_USB_AUDIO_IDX_AUDIO_IN1 0
#define APP_USB_AUDIO_IDX_AUDIO_IN2 1
#define APP_USB_AUDIO_IDX_AUDIO_OUT 2
#define APP_USB_AUDIO_IDX_AUDIO_IN_OUT_MIX 3
static uint32_t s_samples[] = {48000, 48000, 48000};
#ifndef AIR_ALWAYS_KEEP_USB_AUDIO_STREAMING_ENABLE
static uint8_t s_sample_sizes[] = {2, 2, 2};
#else
static uint8_t s_sample_sizes[] = {2, 2, 3};
#endif
static uint8_t s_channels[] = {2, 2, 1};
static bool s_started[3] = {false};
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
static uint8_t s_usb_out_vol_l, s_usb_out_vol_r;
#endif
static bool s_usb_audio_mode = true;

#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
static void app_usb_audio_in_out_mix(bool en);
#endif
static void app_usb_audio_out_ctrl_by_cmd(bool en);
static atci_status_t app_usb_audio_atci_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    char *pChar = NULL;
    atci_response_t *response = NULL;

    response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    pChar = parse_cmd->string_ptr + parse_cmd->name_len + 1;
    if (0 == memcmp(pChar, "OUT_CTRL", 8)) {
        pChar = strchr(pChar, ',');
        pChar++;
        USB_AUDIO_LOG_I("OUT_CTRL", 0);
        if (0 == memcmp(pChar, "ON", 2)) {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            app_usb_audio_out_ctrl_by_cmd(true);
        } else if (0 == memcmp(pChar, "OFF", 3)) {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            app_usb_audio_out_ctrl_by_cmd(false);
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    }
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
    else if (0 == memcmp(pChar, "IN_OUT_MIX_VOLUME", 17)) {
        pChar = strchr(pChar, ',');
        pChar++;

        int32_t volume = atoi(pChar);
        if (volume > 0) {
            app_audio_trans_mgr_set_volume(s_usb_in_out_mix_handle, volume, volume);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
#ifdef AIR_WIRED_AUDIO_SUB_STREAM_ENABLE
    } else if (0 == memcmp(pChar, "IN_OUT_MIX_SUBSTREAM", 20)) {
        extern void bt_sink_srv_ami_set_wired_audio_substream(bool is_enable);
        pChar = strchr(pChar, ',');
        pChar++;
        USB_AUDIO_LOG_I("[SUB STREAM] IN_OUT_MIX_SUBSTREAM %d", 0);
        if (0 == memcmp(pChar, "ON", 2)) {
            bt_sink_srv_ami_set_wired_audio_substream(true);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "OFF", 3)) {
            bt_sink_srv_ami_set_wired_audio_substream(false);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
#endif
    } else if (0 == memcmp(pChar, "IN_OUT_MIX", 9)) {
        pChar = strchr(pChar, ',');
        pChar++;

        if (0 == memcmp(pChar, "ON", 2)) {
            app_usb_audio_in_out_mix(true);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "OFF", 3)) {
            app_usb_audio_in_out_mix(false);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    }
#endif
    else {
        response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        goto atci_exit;
    }

atci_exit:
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}


static atci_cmd_hdlr_item_t app_usb_audio_at_cmd[] = {
    {
        .command_head = "AT+USBAUDIO",       /**< AT command string. */
        .command_hdlr = app_usb_audio_atci_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

static void app_usb_audio_init_atci()
{
    atci_status_t ret = atci_register_handler(app_usb_audio_at_cmd, sizeof(app_usb_audio_at_cmd) / sizeof(atci_cmd_hdlr_item_t));
    USB_AUDIO_LOG_I("register atci handler ret=%d.", 1, ret);
}

static bool is_plug_in = false;
static void report_usb_plug_status(bool plug_in)
{
    if (is_plug_in != plug_in) {
        is_plug_in = plug_in;
        if (s_usb_audio_mode) {
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_USB_PLUG_STATE,
                                (void *)plug_in, 0, NULL, 0);
        }
#ifdef AIR_ALWAYS_KEEP_USB_AUDIO_STREAMING_ENABLE
        /* usb is the last init one. */
        if (!s_started[APP_USB_AUDIO_IDX_AUDIO_OUT] && plug_in) {
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
            app_usb_audio_in_out_mix(true);
#endif
            app_usb_audio_out_ctrl_by_cmd(true);
        } else if (!plug_in) {
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
            app_usb_audio_in_out_mix(false);
#endif
            app_usb_audio_out_ctrl_by_cmd(false);
        }
        USB_AUDIO_LOG_I("start usb audio mix and mix as default.", 0);
#endif
    }
}

void app_usb_audio_report_usb_plug_status(bool plug_in)
{
    report_usb_plug_status(plug_in);
}

bool app_usb_audio_usb_plug_in()
{
    return is_plug_in;
}

uint8_t app_usb_audio_scenario_type(void)
{
    static uint8_t s_usb_scenario_type = 0xFF;
    if (s_usb_scenario_type == 0xFF) {
        uint32_t nvkey_size = sizeof(uint8_t);
        nvkey_status_t sta = nvkey_read_data((uint16_t)NVID_APP_USB_AUDIO_SCENARIO, &s_usb_scenario_type, &nvkey_size);
        if (sta == NVKEY_STATUS_ITEM_NOT_FOUND) {
            s_usb_scenario_type = USB_AUDIO_SCENARIO_TYPE_MIX_OUT;
        }
        USB_AUDIO_LOG_I("get usb audio scenario type 0x%x, ret %d", 2, s_usb_scenario_type, sta);
    }

    return s_usb_scenario_type;
}

/*========================================================================================*/
/*                               AT COMMOND SUPPORT                                       */
/*========================================================================================*/
#include "atci.h"
#include "stdlib.h"
#include "string.h"
static atci_status_t app_usb_audio_atci_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    char *param = NULL;
    uint32_t p1 = 0, p2 = 0;

    param = parse_cmd->string_ptr + parse_cmd->name_len + 1;
    param = strtok(param, ",");

    p1 = atoi(param);
    param = strtok(NULL, ",");
    p2 = atoi(param);
    USB_AUDIO_LOG_I("at command: %d,%d", 2, p1, p2);
#ifdef AIR_USB_AUDIO_IN_ENABLE
    app_audio_trans_mgr_set_mix_ratio(s_usb_in1_handle, p1);
    app_audio_trans_mgr_set_mix_ratio(s_usb_in2_handle, p2);
#endif
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_cmd_hdlr_item_t app_usb_audio_atci_cmd[] = {
    {
        .command_head = "AT+USBAUDIORATIO",
        .command_hdlr = app_usb_audio_atci_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

static void app_usb_audio_atci_init()
{
    atci_status_t ret;

    ret = atci_register_handler(app_usb_audio_atci_cmd, sizeof(app_usb_audio_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
    USB_AUDIO_LOG_I("atci register result %d", 1, ret);
}

#if 0
static uint32_t get_sample_rate(app_usb_audio_sample_rate_t sample_rate)
{
    static uint32_t sample_table[] = {};
    return sample_table[sample_rate];
}
#endif

static void app_usb_audio_cfg_callback(app_audio_trans_mgr_usr_type_t type, app_audio_trans_mgr_usr_cfg_t *cfg)
{
    cfg->trans_cfg.scenario_type = AUDIO_TRANSMITTER_WIRED_AUDIO;
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
    if (APP_AUDIO_TRANS_MGR_USR_USB_IN_OUT_MIX == type) {
        cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM;
        return;
    }
#endif
    audio_codec_pcm_t *codec_param = NULL;
    uint8_t param_idx = 0;
    cfg->trans_cfg.scenario_config.wired_audio_config.usb_in_config.codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
    if (type == APP_AUDIO_TRANS_MGR_USR_USB_OUT) {
#ifndef AIR_WIRELESS_MIC_ENABLE
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
        cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT;
        cfg->trans_cfg.scenario_config.wired_audio_config.usb_out_config.is_with_ecnr = false;
#else
        cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT;
        cfg->trans_cfg.scenario_config.wired_audio_config.usb_out_config.is_with_ecnr = true;
#endif
#else
        cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT;
        cfg->trans_cfg.scenario_config.wired_audio_config.usb_out_config.is_with_ecnr = false;

#endif
        cfg->trans_cfg.scenario_config.wired_audio_config.usb_out_config.is_with_swb = false;
        cfg->resource_type = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC;
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
        cfg->priority = 7;
#else
        cfg->priority = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_RECORD_PRIORITY;
#endif
        cfg->pseudo_type = 0;
        codec_param = &cfg->trans_cfg.scenario_config.wired_audio_config.usb_out_config.codec_param.pcm;
#ifdef AIR_WIRELESS_MIC_ENABLE
        codec_param->frame_interval = 5000;
#endif
        param_idx = 2;//usb out
    } else {
        if (type == APP_AUDIO_TRANS_MGR_USR_USB_IN1) {
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
            cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0;
#else
            cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0;
#endif
            param_idx = 0;//usb in 1
        } else {
            cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1;
            param_idx = 1;//usb in 2
        }
        cfg->resource_type = AUDIO_SRC_SRV_RESOURCE_TYPE_WIRED_AUDIO;
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
        cfg->priority = 7;
#else
        cfg->priority = AUDIO_SRC_SRV_PRIORITY_ABOVE_NORMAL;
#endif
        cfg->pseudo_type = AUDIO_SRC_SRV_PSEUDO_DEVICE_USB_AUDIO;
        codec_param = &cfg->trans_cfg.scenario_config.wired_audio_config.usb_in_config.codec_param.pcm;
    }
    codec_param->sample_rate = s_samples[param_idx];
    codec_param->channel_mode = s_channels[param_idx];
    codec_param->format = s_sample_sizes[param_idx] == 3 ? HAL_AUDIO_PCM_FORMAT_S24_LE : HAL_AUDIO_PCM_FORMAT_S16_LE;
    USB_AUDIO_LOG_I("app_usb_audio_cfg_callback type=%d, sample_rate=%d, size=%d, channel=%d.", 4,
                    type, s_samples[param_idx], s_sample_sizes[param_idx], s_channels[param_idx]);
}

static void app_usb_audio_init()
{
    app_audio_trans_mgr_usr_t usb_audio_cfg;
    memset(&usb_audio_cfg, 0, sizeof(app_audio_trans_mgr_usr_t));
    usb_audio_cfg.cfg_callback = app_usb_audio_cfg_callback;
    usb_audio_cfg.volume_l = DEFAULT_VOLUME;
    usb_audio_cfg.volume_r = DEFAULT_VOLUME;
    usb_audio_cfg.volume_db_l = 0;
    usb_audio_cfg.volume_db_r = 0;
    usb_audio_cfg.mix_ratio = 100;
    usb_audio_cfg.mute = false;
    usb_audio_cfg.en_side_tone = false;
    usb_audio_cfg.usr_data = NULL;
#ifdef AIR_USB_AUDIO_IN_ENABLE
    usb_audio_cfg.type = APP_AUDIO_TRANS_MGR_USR_USB_IN1;
#ifndef AIR_USB_AUDIO_IN_MIX_ENABLE
    usb_audio_cfg.ctrl_type = APP_AUDIO_TRANS_MGR_CTRL_TYPE_PSEUDO_DEVICE;
#else
    usb_audio_cfg.ctrl_type = APP_AUDIO_TRANS_MGR_CTRL_TYPE_RESOURCE_CTRL;
#endif
    usb_audio_cfg.rt_config_type = WIRED_AUDIO_CONFIG_OP_VOL_DB_MUSIC;
    usb_audio_cfg.name = "usb_in";
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    usb_audio_cfg.ctrl_type = APP_AUDIO_TRANS_MGR_CTRL_TYPE_RESOURCE_CTRL;
#endif
    s_usb_in1_handle = app_audio_trans_mgr_register(&usb_audio_cfg);
    app_audio_trans_mgr_init_audio(s_usb_in1_handle);
    usb_audio_cfg.type = APP_AUDIO_TRANS_MGR_USR_USB_IN2;
    s_usb_in2_handle = app_audio_trans_mgr_register(&usb_audio_cfg);
    app_audio_trans_mgr_init_audio(s_usb_in2_handle);
#endif

#ifdef AIR_USB_AUDIO_OUT_ENABLE
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    s_usb_out_vol_l = s_usb_out_vol_r = DEFAULT_VOLUME;
#endif
    usb_audio_cfg.type = APP_AUDIO_TRANS_MGR_USR_USB_OUT;
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
    usb_audio_cfg.en_side_tone = false;
#else
    usb_audio_cfg.en_side_tone = true;
#endif
    usb_audio_cfg.rt_config_type = WIRED_AUDIO_CONFIG_OP_VOL_DB_VOICE;
    usb_audio_cfg.ctrl_type = APP_AUDIO_TRANS_MGR_CTRL_TYPE_RESOURCE_CTRL;
    usb_audio_cfg.name = "usb_out";
    s_usb_out_handle = app_audio_trans_mgr_register(&usb_audio_cfg);
    app_audio_trans_mgr_init_audio(s_usb_out_handle);
#endif

#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
    usb_audio_cfg.rt_config_type = WIRED_AUDIO_CONFIG_OP_VOL_DB_MUSIC;
    usb_audio_cfg.type = APP_AUDIO_TRANS_MGR_USR_USB_IN_OUT_MIX;
    usb_audio_cfg.en_side_tone = false;
    usb_audio_cfg.ctrl_type = APP_AUDIO_TRANS_MGR_CTRL_TYPE_NONE;
    usb_audio_cfg.name = "usb_in_out";
    s_usb_in_out_mix_handle = app_audio_trans_mgr_register(&usb_audio_cfg);
    app_audio_trans_mgr_init_audio(s_usb_in_out_mix_handle);
#endif
}

static bool app_usb_audio_handle_system_event(ui_shell_activity_t *self,
                                              uint32_t event_id,
                                              void *extra_data,
                                              size_t data_len)
{
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            //#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
            app_usb_audio_init_atci();
            //#endif
            app_usb_audio_init();
            app_usb_audio_atci_init();
            USB_AUDIO_LOG_I("app_usb_audio_idle_activity CREATE.", 0);
            break;
        }
        default:
            break;
    }
    return true;
}

static void *get_handle_by_port(app_usb_audio_port_t port_type, uint8_t port_num)
{
#ifdef AIR_USB_AUDIO_IN_ENABLE
    if (port_type == APP_USB_AUDIO_SPK_PORT) {
        return (port_num == 0 ? s_usb_in1_handle : s_usb_in2_handle);
    }
#endif
#ifdef AIR_USB_AUDIO_OUT_ENABLE
    if (port_type == APP_USB_AUDIO_MIC_PORT) {
        return s_usb_out_handle;
    }
#endif
    return NULL;
}

static void *get_handle_by_idx(uint32_t idx)
{
#ifdef AIR_USB_AUDIO_IN_ENABLE
    if (idx == APP_USB_AUDIO_IDX_AUDIO_IN1) {
        return s_usb_in1_handle;
    } else if (idx == APP_USB_AUDIO_IDX_AUDIO_IN2) {
        return s_usb_in2_handle;
    }
#endif
#ifdef AIR_USB_AUDIO_OUT_ENABLE
    if (idx == APP_USB_AUDIO_IDX_AUDIO_OUT) {
        return s_usb_out_handle;
    }
#endif
    return NULL;
}

/* workaround start */
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
static bool usb_in_suspend_by_vp = false;
void app_usb_audio_vp_play_end_notify()
{
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_USB_IN_RESUME_AFTER_VP, NULL, 0, NULL, 100);
}
#endif

#ifdef AIR_USB_AUDIO_OUT_ENABLE
static bool usb_out_suspend = false;
extern void app_audio_trans_mgr_stop_audio_unsafe(void *usr);
void app_usb_out_unsafe_suspend_by_music()
{
    if (s_started[APP_USB_AUDIO_IDX_AUDIO_OUT] && !usb_out_suspend) {
        app_audio_trans_mgr_stop_audio_unsafe(s_usb_out_handle);
        USB_AUDIO_LOG_I("usb out do suspend", 0);
    }
    usb_out_suspend = true;
    USB_AUDIO_LOG_I("usb out do suspend = 1", 0);
}

void app_usb_out_resume_by_music()
{
    if (usb_out_suspend && s_started[APP_USB_AUDIO_IDX_AUDIO_OUT]) {
        app_audio_trans_mgr_start_audio(s_usb_out_handle);
        USB_AUDIO_LOG_I("usb out resume", 0);
    }
    usb_out_suspend = false;
}

#if 0
bool app_usb_audio_handle_sink_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    if (event_id == BT_SINK_SRV_EVENT_STATE_CHANGE) {
        bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *) extra_data;
        if ((param->previous >= BT_SINK_SRV_STATE_STREAMING) && (param->current <= BT_SINK_SRV_STATE_CONNECTED)) {
            if (usb_out_suspend && s_started[APP_USB_AUDIO_IDX_AUDIO_OUT]) {
                app_audio_trans_mgr_start_audio(s_usb_out_handle);
                USB_AUDIO_LOG_I("usb out resume", 0);
            }
            usb_out_suspend = false;
            USB_AUDIO_LOG_I("usb out do suspend = 0", 0);
        }
    }
    return false;
}
#endif
#endif
/* workaround end */

static bool _proc_apps_internal_events(struct _ui_shell_activity *self,
                                       uint32_t event_id,
                                       void *extra_data,
                                       size_t data_len)
{
    switch (event_id) {
        case APPS_EVENTS_INTERACTION_POWER_OFF: {
            /* Stop USB audio before system power off. */
            USB_AUDIO_LOG_I("USB Audio stop when power off.", 0);
#ifdef AIR_USB_AUDIO_IN_ENABLE
            app_audio_trans_mgr_stop_audio(s_usb_in1_handle);
            app_audio_trans_mgr_stop_audio(s_usb_in2_handle);
#endif
        }
        break;
#ifdef AIR_USB_HID_MEDIA_CTRL_ENABLE
#if defined(GSOUND_LIBRARY_ENABLE) || defined(AIR_USB_AUDIO_IN_ENABLE)
        case APPS_EVENTS_INTERACTION_GSOUND_ACTION_REJECTED: {
            apps_config_state_t sta = apps_config_key_get_mmi_state();
            USB_AUDIO_LOG_I("USB Audio process play pause due to gsound rejected", 0);
            if (sta <= APP_CONNECTABLE) {
                USB_HID_PlayPause();
            }
        }
        break;
#endif
#endif
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
        case APPS_EVENTS_INTERACTION_SWITCH_MIC: {
            if (s_started[APP_USB_AUDIO_IDX_AUDIO_OUT]) {
                void *handle = get_handle_by_idx(APP_USB_AUDIO_IDX_AUDIO_OUT);
                app_audio_trans_mgr_set_volume(handle, 0, 0);
                app_audio_trans_mgr_stop_audio(handle);
                app_audio_trans_mgr_start_audio(handle);
                app_audio_trans_mgr_set_volume(handle, s_usb_out_vol_l, s_usb_out_vol_r);
            }
        }
        break;
#endif
            /* workaround start */
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
        case APPS_EVENTS_INTERACTION_USB_IN_RESUME_AFTER_VP: {
            if (voice_prompt_get_current_index() == VOICE_PROMPT_VP_INDEX_INVALID) {
                if (s_started[APP_USB_AUDIO_IDX_AUDIO_IN1] && s_usb_audio_mode) {
                    app_audio_trans_mgr_start_audio(s_usb_in1_handle);
                }
                if (s_started[APP_USB_AUDIO_IDX_AUDIO_IN2] && s_usb_audio_mode) {
                    app_audio_trans_mgr_start_audio(s_usb_in2_handle);
                }
                usb_in_suspend_by_vp = false;
            }
        }
        break;
#endif
        case APPS_EVENTS_INTERACTION_USB_AUDIO_EN: {
            bool en = (bool)extra_data;
            if (s_usb_audio_mode != en) {
                USB_AUDIO_LOG_I("APPS_EVENTS_INTERACTION_USB_AUDIO_EN %d->%d", 2, s_usb_audio_mode, en);
                s_usb_audio_mode = en;
                for (uint32_t idx = 0; idx < 3; idx++) {
                    if (s_started[idx]) {
                        if (en) {
                            app_audio_trans_mgr_start_audio(get_handle_by_idx(idx));
                        } else {
                            app_audio_trans_mgr_stop_audio(get_handle_by_idx(idx));
                        }
                    }
                }
                if (is_plug_in) {
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_USB_PLUG_STATE,
                                        (void *)en, 0, NULL, 0);
                }
            }
        }
        break;
            /* workaround end */
    }
    return false;
}

#ifdef AIR_ROTARY_ENCODER_ENABLE
#ifdef AIR_USB_AUDIO_IN_ENABLE
#define MIX_RATIO_GAME_MAX_LEVEL    (0)     /* Gaming is 100%, Chat is 0% */
#define MIX_RATIO_CHAT_MAX_LEVEL    (20)    /* Gaming is 0%, Chat is 100% */
#define MIX_RATIO_BALANCED_LEVEL    (10)    /* Gaming is 100%, Chat is 100% */
void app_usb_audio_mix_rotary_change(uint8_t mix_arg)
{
    uint8_t mix_ratio = (mix_arg <= MIX_RATIO_BALANCED_LEVEL) ?
                        100 : 100 * (MIX_RATIO_CHAT_MAX_LEVEL - mix_arg) / (MIX_RATIO_CHAT_MAX_LEVEL - MIX_RATIO_BALANCED_LEVEL);
    app_audio_trans_mgr_set_mix_ratio(s_usb_in1_handle, mix_ratio);
    mix_ratio = (mix_arg >= MIX_RATIO_BALANCED_LEVEL) ?
                100 : 100 * (mix_arg - MIX_RATIO_GAME_MAX_LEVEL) / (MIX_RATIO_BALANCED_LEVEL - MIX_RATIO_GAME_MAX_LEVEL);
    app_audio_trans_mgr_set_mix_ratio(s_usb_in2_handle, mix_ratio);
}
#endif

/**
 * @brief Handle the rotary encoder event.
 *
 * @param event_id      The event ID of the key.
 * @param extra_data    The extra data of the event.
 * @param data_len      The extra data length.
 * @return              If return true, the current key event cannot be handled by the next activity.
 */
static bool app_usb_handle_rotary_event(
    ui_shell_activity_t *self,
    uint32_t event_id,
    void *extra_data,
    size_t data_len)
{
    bool ret = false;
#ifdef AIR_USB_AUDIO_IN_ENABLE
    static uint8_t s_ratio = ULL_MIX_RATIO_BALANCED_LEVEL;
#endif
    bsp_rotary_encoder_port_t port;
    bsp_rotary_encoder_event_t event;
    uint32_t rotary_data;
    if (!extra_data) {
        return ret;
    }
    apps_config_key_action_t key_action = *(uint16_t *)extra_data;
    app_event_rotary_event_decode(&port, &event, &rotary_data, event_id);

    switch (key_action) {
#ifdef AIR_USB_AUDIO_IN_ENABLE
        case KEY_AUDIO_MIX_RATIO_GAME_ADD: {
            if (ULL_MIX_RATIO_GAME_MAX_LEVEL + rotary_data <= s_ratio) {
                s_ratio -= rotary_data;
                app_usb_audio_mix_rotary_change(s_ratio);
            }
            break;
        }
        case KEY_AUDIO_MIX_RATIO_CHAT_ADD: {
            if (s_ratio + rotary_data <= ULL_MIX_RATIO_CHAT_MAX_LEVEL) {
                s_ratio += rotary_data;
                app_usb_audio_mix_rotary_change(s_ratio);
            }
            break;
        }
#endif
#ifdef AIR_USB_HID_MEDIA_CTRL_ENABLE
        case KEY_VOICE_UP: {
            USB_HID_VolumeUp(rotary_data);
            USB_AUDIO_LOG_I("rotary volume up:%d", 1, rotary_data);
            break;
        }
        case KEY_VOICE_DN: {
            USB_HID_VolumeDown(rotary_data);
            USB_AUDIO_LOG_I("rotary volume dn:%d", 1, rotary_data);
            break;
        }
#endif
        default:
            break;
    }
    return ret;
}
#endif

#ifdef AIR_USB_HID_MEDIA_CTRL_ENABLE
static bool _proc_key_event_group(
    struct _ui_shell_activity *self,
    uint32_t event_id,
    void *extra_data,
    size_t data_len)
{
    if (!is_plug_in) {
        return false;
    }
    bool ret = false;
    apps_config_key_action_t action;
    uint8_t key_id;
    airo_key_event_t key_event;
    app_event_key_event_decode(&key_id, &key_event, event_id);
    action = apps_config_key_event_remapper_map_action_in_temp_state(key_id, key_event, APP_WIRED_MUSIC_PLAY);

    if (action != KEY_AVRCP_PLAY && action != KEY_AVRCP_PAUSE && !s_started[APP_USB_AUDIO_IDX_AUDIO_IN1] && !s_started[APP_USB_AUDIO_IDX_AUDIO_IN2]) {
        /* in this case, the key action is not for usb audio due to usb speaker not in streaming. */
        return ret;
    }
    switch (action) {
        case KEY_AVRCP_PLAY:
        case KEY_AVRCP_PAUSE: {
            apps_config_state_t sta = apps_config_key_get_mmi_state();
            bool wired_audio_playing = app_audio_trans_mgr_wired_audio_playing();
            USB_AUDIO_LOG_I("idle activity play pause, mmi_sta = %d, wired_audio_playing = %d", 2, sta, wired_audio_playing);
            if (sta <= APP_CONNECTED || (wired_audio_playing && sta >= APP_WIRED_MUSIC_PLAY)) {
                USB_HID_PlayPause();
                ret = true;
            }
            break;
        }
        case KEY_VOICE_UP: {
            USB_HID_VolumeUp(1);
            break;
        }
        case KEY_VOICE_DN: {
            USB_HID_VolumeDown(1);
            break;
        }
        case KEY_AVRCP_BACKWARD:
            USB_HID_ScanPreviousTrack();
            ret = true;
            break;
        case KEY_AVRCP_FORWARD:
            USB_HID_ScanNextTrack();
            ret = true;
            break;
    }
    return ret;
}
#endif

static uint32_t get_idx_by_port(app_usb_audio_port_t port_type, uint8_t port_num)
{
    if (port_type == APP_USB_AUDIO_SPK_PORT) {
        return (port_num == 0 ? 0 : 1);
    } else {
        return 2;
    }
}

static void __check_and_update_config(uint32_t idx, void* handle)
{
    do {
        if (s_started[idx] && s_usb_audio_mode) {
            /* workaround start */
#ifdef AIR_USB_AUDIO_OUT_ENABLE
            if (idx == APP_USB_AUDIO_IDX_AUDIO_OUT && usb_out_suspend) {
                app_audio_trans_mgr_deinit_audio(handle);
                app_audio_trans_mgr_init_audio(handle);
                break;
            }
#endif
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
            if (idx != APP_USB_AUDIO_IDX_AUDIO_OUT && usb_in_suspend_by_vp) {
                app_audio_trans_mgr_deinit_audio(handle);
                app_audio_trans_mgr_init_audio(handle);
                break;
            }
#endif
            /* workaround end */
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
            if (s_usb_in_out_mix_started && idx == APP_USB_AUDIO_IDX_AUDIO_OUT) {
                app_audio_trans_mgr_stop_audio(s_usb_in_out_mix_handle);
            }
#endif
            app_audio_trans_mgr_stop_audio(handle);
            app_audio_trans_mgr_deinit_audio(handle);
            app_audio_trans_mgr_init_audio(handle);
            app_audio_trans_mgr_start_audio(handle);
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
            if (s_usb_in_out_mix_started && idx == APP_USB_AUDIO_IDX_AUDIO_OUT) {
                app_audio_trans_mgr_start_audio(s_usb_in_out_mix_handle);
            }
#endif
        } else {
            app_audio_trans_mgr_deinit_audio(handle);
            app_audio_trans_mgr_init_audio(handle);
        }
    }while (0);
}

static bool _proc_usb_audio_event_group(
    struct _ui_shell_activity *self,
    uint32_t event_id,
    void *extra_data,
    size_t data_len)
{
    //USB_AUDIO_LOG_I("_proc_usb_audio_event_group event id = %d", 1, event_id);
    switch (event_id) {
        case APPS_EVENTS_USB_AUDIO_UNPLUG:
            report_usb_plug_status(false);
        case APPS_EVENTS_USB_AUDIO_RESET: {
            for (uint32_t idx = 0; idx < 3; idx++) {
#ifdef AIR_ALWAYS_KEEP_USB_AUDIO_STREAMING_ENABLE
                if (idx == APP_USB_AUDIO_IDX_AUDIO_OUT) {
                    continue;
                }
#endif
                ui_shell_remove_event(EVENT_GROUP_SWITCH_USB_AUDIO, idx);
                if (s_started[idx]) {
                    s_started[idx] = false;
                    app_audio_trans_mgr_stop_audio(get_handle_by_idx(idx));
                }
            }
            if (event_id == APPS_EVENTS_USB_AUDIO_RESET) {
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_CONFIG_CHECKER);
                ui_shell_send_event(false,
                                    EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_USB_AUDIO,
                                    APPS_EVENTS_USB_CONFIG_CHECKER,
                                    NULL,
                                    0,
                                    NULL,
                                    100);
            }
            break;
        }
        case APPS_EVENTS_USB_AUDIO_RESUME: {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_USB_AUDIO, APPS_EVENTS_USB_CONFIG_CHECKER);
            ui_shell_send_event(false,
                                EVENT_PRIORITY_HIGH,
                                EVENT_GROUP_UI_SHELL_USB_AUDIO,
                                APPS_EVENTS_USB_CONFIG_CHECKER,
                                NULL,
                                0,
                                NULL,
                                0);
            break;
        }
        case APPS_EVENTS_USB_AUDIO_PLAY: {
            report_usb_plug_status(true);

            app_events_usb_port_t *port = (app_events_usb_port_t *)&extra_data;
            uint32_t idx = get_idx_by_port(port->port_type, port->port_num);
            ui_shell_remove_event(EVENT_GROUP_SWITCH_USB_AUDIO, idx);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_SWITCH_USB_AUDIO,
                                idx, (void *)true, 0, NULL, 100);
            break;
        }
        case APPS_EVENTS_USB_AUDIO_STOP: {
            report_usb_plug_status(true);

            app_events_usb_port_t *port = (app_events_usb_port_t *)&extra_data;
            uint32_t idx = get_idx_by_port(port->port_type, port->port_num);
#ifdef AIR_ALWAYS_KEEP_USB_AUDIO_STREAMING_ENABLE
            if (idx == APP_USB_AUDIO_IDX_AUDIO_OUT) {
                break;
            }
#endif
            ui_shell_remove_event(EVENT_GROUP_SWITCH_USB_AUDIO, idx);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_SWITCH_USB_AUDIO,
                                idx, (void *)false, 0, NULL, 100);
            break;
        }
        case APPS_EVENTS_USB_AUDIO_VOLUME: {
            app_events_usb_volume_t *vol = (app_events_usb_volume_t *)extra_data;
            app_audio_trans_mgr_set_db(get_handle_by_port(vol->port_type, vol->port_num), vol->left_db, vol->right_db);
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            /* back usb out volume due to it will be reset. */
            if (get_idx_by_port(vol->port_type, vol->port_num) == APP_USB_AUDIO_IDX_AUDIO_OUT) {
                s_usb_out_vol_l = vol->left_volume;
                s_usb_out_vol_r = vol->right_volume;
            }
#endif
            break;
        }
        case APPS_EVENTS_USB_AUDIO_MUTE: {
            app_events_usb_mute_t *mute_info = (app_events_usb_mute_t *)&extra_data;
            app_audio_trans_mgr_set_mute(get_handle_by_port(mute_info->port_type, mute_info->port_num), mute_info->is_mute);
            break;
        }
        case APPS_EVENTS_USB_AUDIO_SAMPLE_RATE: {
            app_events_usb_sample_rate_t *sr_info = (app_events_usb_sample_rate_t *)&extra_data;
            uint32_t idx = get_idx_by_port(sr_info->port_type, sr_info->port_num);
#ifdef AIR_USB_AUDIO_ENABLE
            uint32_t new_rate = apps_event_usb_event_sample_rate_convert(sr_info->rate);
#else
            uint32_t new_rate = 48000;
#endif
            if (s_samples[idx] != new_rate) {
                USB_AUDIO_LOG_I("usr=%d set sample rate to=%d, old=%d", 3, idx, new_rate, s_samples[idx]);
                s_samples[idx] = new_rate;
                void *handle = get_handle_by_port(sr_info->port_type, sr_info->port_num);
                __check_and_update_config(idx, handle);
            }
            break;
        }
        case APPS_EVENTS_USB_AUDIO_SAMPLE_SIZE: {
            app_events_usb_sample_size_t *size_info = (app_events_usb_sample_size_t *)&extra_data;
            uint32_t idx = get_idx_by_port(size_info->port_type, size_info->port_num);
            if (size_info->size > 0 && s_sample_sizes[idx] != size_info->size) {
                s_sample_sizes[idx] = size_info->size;
                USB_AUDIO_LOG_I("usr=%d set sample size to=%d", 2, idx, s_sample_sizes[idx]);
                void *handle = get_handle_by_port(size_info->port_type, size_info->port_num);
                __check_and_update_config(idx, handle);
            }
            break;
        }
        case APPS_EVENTS_USB_AUDIO_SET_CHANNELS: {
            app_events_usb_set_channel_t *channel_info = (app_events_usb_set_channel_t *)&extra_data;
            uint32_t idx = get_idx_by_port(channel_info->port_type, channel_info->port_num);

            if (channel_info->channel_nums > 0 && s_channels[idx] != channel_info->channel_nums) {
                s_channels[idx] = channel_info->channel_nums;
                USB_AUDIO_LOG_I("usr=%d set channel to=%d", 2, idx, s_channels[idx]);
                /* set sample size -> set channels -> start. */
                void *handle = get_handle_by_port(channel_info->port_type, channel_info->port_num);
                __check_and_update_config(idx, handle);
            }
            break;
        }
        case APPS_EVENTS_USB_CONFIG_CHECKER: {
            USB_DEVICE_STATE state = USB_Get_Device_State();
            USB_AUDIO_LOG_I("USB configure state : %d", 1, state);
            if (state == DEVSTATE_CONFIG) {
                ui_shell_send_event(false,
                                    EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_USB_AUDIO,
                                    APPS_EVENTS_USB_CONFIG_DONE,
                                    NULL,
                                    0,
                                    NULL,
                                    0);
            } else {
                ui_shell_send_event(false,
                                    EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_USB_AUDIO,
                                    APPS_EVENTS_USB_CONFIG_CHECKER,
                                    NULL,
                                    0,
                                    NULL,
                                    100);
            }
            break;
        }
        case APPS_EVENTS_USB_CONFIG_DONE: {
            /* USB has enumerated */
            report_usb_plug_status(true);
            break;
        }
    }
    return false;
}

/*========================================================================================*/
/*                          IDLE ACTIVITY PRO FUNCTION                                    */
/*========================================================================================*/
bool app_usb_audio_idle_activity_proc(struct _ui_shell_activity *self,
                                      uint32_t event_group,
                                      uint32_t event_id,
                                      void *extra_data,
                                      size_t data_len)
{
    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            /* ui_shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = app_usb_audio_handle_system_event(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            /* interaction events. */
            ret = _proc_apps_internal_events(self, event_id, extra_data, data_len);
            break;
#ifdef AIR_USB_HID_MEDIA_CTRL_ENABLE
        case EVENT_GROUP_UI_SHELL_KEY: {
            ret = _proc_key_event_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
#ifdef AIR_ROTARY_ENCODER_ENABLE
        case EVENT_GROUP_UI_SHELL_ROTARY_ENCODER:
            /**< group for rotary encoder events */
            ret = app_usb_handle_rotary_event(self, event_id, extra_data, data_len);
            break;
#endif
        case EVENT_GROUP_UI_SHELL_USB_AUDIO:
            ret = _proc_usb_audio_event_group(self, event_id, extra_data, data_len);
            break;

            /* workaround: the usb audio should resume after a2dp streaming stoped. */
#if 0
#ifdef AIR_USB_AUDIO_OUT_ENABLE
        case EVENT_GROUP_UI_SHELL_BT_SINK: {
            ret = app_usb_audio_handle_sink_event(self, event_id, extra_data, data_len);
            break;
        }
#endif
#endif
        /* workaround end */
        case EVENT_GROUP_SWITCH_USB_AUDIO: {
            uint32_t idx = event_id;
            bool play = (bool)extra_data;
            if (idx == APP_USB_AUDIO_IDX_AUDIO_IN_OUT_MIX) {
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
                extern void wired_audio_set_usb_out_mix_usb_in_state(bool en);
                if (s_usb_in_out_mix_started != play) {
                    USB_AUDIO_LOG_I("usb in out mix switch to=%d", 1, play);
                    wired_audio_set_usb_out_mix_usb_in_state(play);
                    s_usb_in_out_mix_started = play;
                    // for (uint32_t idx = 0; idx < 3; idx++) {
                    //     if (s_started[idx]) {
                    //         app_audio_trans_mgr_stop_audio(get_handle_by_idx(idx));
                    //         app_audio_trans_mgr_start_audio(get_handle_by_idx(idx));
                    //     }
                    // }
                    if (play) {
                        if (s_started[APP_USB_AUDIO_IDX_AUDIO_OUT]) {
                            app_audio_trans_mgr_start_audio(s_usb_in_out_mix_handle);
                        }
                    } else {
                        app_audio_trans_mgr_stop_audio(s_usb_in_out_mix_handle);
                    }
                }
#endif
                break;
            }
            /* workaround: reset suspend flag */
#ifdef AIR_USB_AUDIO_OUT_ENABLE
            if (idx == APP_USB_AUDIO_IDX_AUDIO_OUT) {
                USB_AUDIO_LOG_I("usb out op=%d,sta=%d,%d,%d", 4, play, usb_out_suspend, s_started[APP_USB_AUDIO_IDX_AUDIO_OUT], s_usb_audio_mode);
                if (play) {
                    if (!usb_out_suspend && !s_started[APP_USB_AUDIO_IDX_AUDIO_OUT]) {
                        if (s_usb_audio_mode) {
                            app_audio_trans_mgr_start_audio(s_usb_out_handle);
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
                            if (s_usb_in_out_mix_started) {
                                app_audio_trans_mgr_start_audio(s_usb_in_out_mix_handle);
                            }
#endif
                        }
                    }
                    s_started[APP_USB_AUDIO_IDX_AUDIO_OUT] = true;
                } else {
                    if (!usb_out_suspend && s_started[APP_USB_AUDIO_IDX_AUDIO_OUT]) {
                        if (s_usb_audio_mode) {
#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
                            if (s_usb_in_out_mix_started) {
                                app_audio_trans_mgr_stop_audio(s_usb_in_out_mix_handle);
                            }
#endif
                            app_audio_trans_mgr_stop_audio(s_usb_out_handle);
                        }
                    }
                    s_started[APP_USB_AUDIO_IDX_AUDIO_OUT] = false;
                }
                break;
            }
#endif
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
            if (idx != APP_USB_AUDIO_IDX_AUDIO_OUT) {
                if (play) {
                    if (voice_prompt_get_current_index() != VOICE_PROMPT_VP_INDEX_INVALID) {
                        usb_in_suspend_by_vp = true;
                        s_started[idx] = true;
                        break;
                    }
                } else {
                    if (usb_in_suspend_by_vp) {
                        usb_in_suspend_by_vp = false;
                        s_started[idx] = false;
                        break;
                    }
                }
            }
#endif
            /* workaround end */

            if (play) {
                if (s_started[idx] == true) { /* Host may sent many play cmds during sample rate change. */
                    break;
                }
                s_started[idx] = true;
                if (s_usb_audio_mode) {
                    app_audio_trans_mgr_start_audio(get_handle_by_idx(idx));
                }
            } else {
                if (s_started[idx] == false) { /* to filter the stop request when usb plug in. */
                    break;
                }
                s_started[idx] = false;
                if (s_usb_audio_mode) {
                    app_audio_trans_mgr_stop_audio(get_handle_by_idx(idx));
                }
            }

            break;
        }
        default:
            break;
    }

    return ret;
}

bool app_usb_in_is_open()
{
#ifdef AIR_USB_AUDIO_IN_ENABLE
    if (s_started[0] || s_started[1]) {
        return true;
    }
#endif
    return false;
}

bool app_usb_out_is_open()
{
#ifdef AIR_USB_AUDIO_OUT_ENABLE
    return s_started[2];
#endif
    return false;
}

void app_usb_audio_enable(bool en)
{
    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_USB_AUDIO_EN,
                        (void *)en, 0, NULL, 0);
}

#ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
static void app_usb_audio_in_out_mix(bool en)
{
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_SWITCH_USB_AUDIO,
                        APP_USB_AUDIO_IDX_AUDIO_IN_OUT_MIX, (void *)en, 0, NULL, 100);
}
#endif

static void app_usb_audio_out_ctrl_by_cmd(bool en)
{
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_SWITCH_USB_AUDIO,
                        APP_USB_AUDIO_IDX_AUDIO_OUT, (void *)en, 0, NULL, 0);
}

#ifdef AIR_USB_AUDIO_OUT_ENABLE
bool app_usb_out_mute(bool en)
{
    if (s_usb_out_handle) {
        app_audio_trans_mgr_set_mute(s_usb_out_handle, en);
        return true;
    }
    return false;
}
#endif

