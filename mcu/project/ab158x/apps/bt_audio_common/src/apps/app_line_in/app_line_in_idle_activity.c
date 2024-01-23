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
 * File: app_line_in_idle_activity.c
 *
 * Description:
 * This file is the activity to process the play or stop of line-in. When line-in is plugin,
 * this activity will play line-in audio, and stop line-in audio when plugout.
 */

#include "app_audio_trans_mgr.h"
#include "app_line_in_idle_activity.h"
#include "hal_gpio.h"
#include "ui_shell_manager.h"
#include "ui_shell_activity.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "hal_gpio.h"
#include "hal_eint.h"
#include "hal_nvic.h"
#include "hal_rtc.h"
#include "stdlib.h"
#include "hal_pinmux_define.h"
#include "audio_source_control.h"
#ifdef MTK_RACE_CMD_ENABLE
#include "race_cmd_hostaudio.h"
#endif
#include "apps_config_event_list.h"
#include "apps_race_cmd_event.h"
#include "apps_debug.h"
#include "project_config.h"
#ifdef AIR_ROTARY_ENCODER_ENABLE
#include "bsp_rotary_encoder.h"
#include "apps_events_key_event.h"
#endif
#include "bt_sink_srv_ami.h"
#ifdef AIR_LINE_OUT_ENABLE
#include "bt_sink_srv.h"
#endif

#define LINE_IN_LOG_I(msg, ...)     APPS_LOG_MSGID_I("[LINE_IN_MMI]"msg, ##__VA_ARGS__)
#define LINE_IN_LOG_E(msg, ...)     APPS_LOG_MSGID_E("[LINE_IN_MMI]"msg, ##__VA_ARGS__)
#define LINE_IN_LOG_D(msg, ...)     APPS_LOG_MSGID_D("[LINE_IN_MMI]"msg, ##__VA_ARGS__)

/**
 * The port used to detect the line-in plugin or plugout.
 * For high value means this is no line-in.
 * For low value means line-in already exist.
 */
extern const char BSP_LINE_IN_DET_PIN;
extern const unsigned char BSP_LINE_IN_EINT;
extern const char BSP_LINE_IN_SWITCH_PIN;
#define LINE_IN_DETECT_PORT             BSP_LINE_IN_DET_PIN
#define LINE_IN_DETECT_EINT_NUM         BSP_LINE_IN_EINT

#ifdef LINE_IN_DET_WITH_HIGH_LEVEL
#define LINE_IN_ALREADY_PLUG_IN         HAL_GPIO_DATA_HIGH
#define LINE_IN_ALREADY_PLUG_OUT        HAL_GPIO_DATA_LOW
#else
#define LINE_IN_ALREADY_PLUG_IN         HAL_GPIO_DATA_LOW
#define LINE_IN_ALREADY_PLUG_OUT        HAL_GPIO_DATA_HIGH
#endif

#define AUDIO_PATH_ANALOG_SW_PORT       BSP_LINE_IN_SWITCH_PIN
#if 0
#ifdef LINE_IN_ENABLE_WITH_LOW_LEVEL
#define AUDIO_PATH_ANALOG_MIC_IN        HAL_GPIO_DATA_HIGH
#define AUDIO_PATH_ANALOG_LINE_IN       HAL_GPIO_DATA_LOW
#else
#define AUDIO_PATH_ANALOG_MIC_IN        HAL_GPIO_DATA_LOW
#define AUDIO_PATH_ANALOG_LINE_IN       HAL_GPIO_DATA_HIGH
#endif
#endif

#define LINE_IN_SCENARIO_TYPE_COMMON 1
#define LINE_IN_SCENARIO_TYPE_MIX 0

#define AUDIO_PATH_ANALOG_MIC_IN        HAL_GPIO_DATA_HIGH
#define AUDIO_PATH_ANALOG_LINE_IN       HAL_GPIO_DATA_LOW

#define APPS_EVENTS_INTERACTION_LINE_IN_PLUG_IN 1
#define APPS_EVENTS_INTERACTION_LINE_IN_PLUG_OUT 2

#define DEFAULT_VOLUME 15

#ifdef MTK_RACE_CMD_ENABLE
static void line_in_sp_app_pull_request(uint8_t *current_audio_path);
static void line_in_sp_app_control_request(uint8_t new_audio_path, uint8_t *control_result);
static void line_in_sp_app_push_response(uint8_t status);
#endif

static bool line_in_with_rtc_pin = false;
static uint8_t s_line_in_type = 0;
#ifdef AIR_LINE_IN_ENABLE
static void *s_line_in_handle = NULL;
#endif
#ifdef AIR_LINE_OUT_ENABLE
static void *s_line_out_handle = NULL;
static bool s_line_out_mute = false;
#endif
app_audio_path_t current_audio_path_value = APP_AUDIO_PATH_UNKNOWN;

/*========================================================================================*/
/*                          APP INTERNAL FUNCTIONS                                        */
/*========================================================================================*/
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
#include "app_power_save_utils.h"
static app_power_saving_target_mode_t line_in_get_power_saving_target_mode(void)
{
    app_power_saving_target_mode_t target_mode = APP_POWER_SAVING_TARGET_MODE_SYSTEM_OFF;
    if (!line_in_with_rtc_pin) {
        hal_gpio_data_t current_gpio_status = 0;
        hal_gpio_get_input(LINE_IN_DETECT_PORT, &current_gpio_status);
        LINE_IN_LOG_I("power saving callback, gpio=%d", 1, current_gpio_status);
        if (APP_AUDIO_PATH_LINE_IN == current_audio_path_value && LINE_IN_ALREADY_PLUG_IN == current_gpio_status) {
            target_mode = APP_POWER_SAVING_TARGET_MODE_NORMAL;
        }
    } else {
        bool rtc_level = HAL_RTC_GPIO_DATA_LOW;
        hal_rtc_gpio_get_input(LINE_IN_DETECT_PORT, &rtc_level);
        LINE_IN_LOG_I("power saving callback, rtc gpio=%d", 1, rtc_level);
        if (rtc_level == LINE_IN_ALREADY_PLUG_IN) {
            target_mode =  APP_POWER_SAVING_TARGET_MODE_NORMAL;
        }
    }
    LINE_IN_LOG_I("[POWER_SAVING] target_mode=%d", 1, target_mode);
    return target_mode;
}
#endif

/* workaround over */
#ifdef AIR_LINE_OUT_ENABLE
static bool s_line_out_suspend = false;
extern void app_audio_trans_mgr_stop_audio_unsafe(void *usr);
void app_line_out_unsafe_suspend_by_music()
{
    if (current_audio_path_value == APP_AUDIO_PATH_LINE_IN && !s_line_out_suspend) {
        app_audio_trans_mgr_stop_audio_unsafe(s_line_out_handle);
        LINE_IN_LOG_I("line out do suspend", 0);
    }
    s_line_out_suspend = true;
}

void app_line_out_resume_by_music()
{
    if (s_line_out_suspend && current_audio_path_value == APP_AUDIO_PATH_LINE_IN) {
        app_audio_trans_mgr_start_audio(s_line_out_handle);
        LINE_IN_LOG_I("line out resume", 0);
    }
    s_line_out_suspend = false;
}

#if 0
bool app_line_out_handle_sink_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    if (event_id == BT_SINK_SRV_EVENT_STATE_CHANGE) {
        bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *) extra_data;
        if ((param->previous >= BT_SINK_SRV_STATE_STREAMING) && (param->current <= BT_SINK_SRV_STATE_CONNECTED)) {
            if (s_line_out_suspend && current_audio_path_value == APP_AUDIO_PATH_LINE_IN) {
                app_audio_trans_mgr_start_audio(s_line_out_handle);
                LINE_IN_LOG_I("line out resume", 0);
            }
            s_line_out_suspend = false;
        }
    }
    return false;
}
#endif
#endif
/* workaround end */

static void app_line_in_switch(app_audio_path_t path)
{
    if (path == APP_AUDIO_PATH_LINE_IN) {
#ifdef AIR_LINE_IN_ENABLE
        app_audio_trans_mgr_start_audio(s_line_in_handle);
#endif
#ifdef AIR_LINE_OUT_ENABLE
        if (!s_line_out_suspend) {
            app_audio_trans_mgr_start_audio(s_line_out_handle);
        }
#endif
    } else {
#ifdef AIR_LINE_IN_ENABLE
        app_audio_trans_mgr_stop_audio(s_line_in_handle);
#endif
#ifdef AIR_LINE_OUT_ENABLE
        if (!s_line_out_suspend) {
            app_audio_trans_mgr_stop_audio(s_line_out_handle);
        }
#endif
    }
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
    if (path == APP_AUDIO_PATH_LINE_IN) {
        /* In order to avoid the system enter sleep state. */
        app_power_save_utils_notify_mode_changed(true, line_in_get_power_saving_target_mode);
    }
#endif
}

static void broadcast_line_in_plug_state(bool from_isr, bool plug_in)
{
    ui_shell_send_event(from_isr, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_LINE_IN_PLUG_STATE,
                        (void *)plug_in, 0, NULL, 0);
}

static void store_current_audio_path()
{
    nvkey_status_t status = NVKEY_STATUS_OK;
    uint8_t buf[2] = {0};
    uint32_t buf_len = 2;
    snprintf((char *)buf, 2, "%d", current_audio_path_value);

    status = nvkey_write_data(NVID_APP_LINE_IN_AUDIO_PATH, (const uint8_t *)&buf, buf_len);

    LINE_IN_LOG_I("Store audio path (%d) result : %d\n", 2, current_audio_path_value, status);
}

static void switch_audio_path(app_audio_path_t path, bool storage)
{
    LINE_IN_LOG_I("switch audio path: %d to %d\n", 2, current_audio_path_value, path);
    current_audio_path_value = path;
    if (storage) {
        store_current_audio_path();
    }
}

static void app_line_in_cfg_callback(app_audio_trans_mgr_usr_type_t type, app_audio_trans_mgr_usr_cfg_t *cfg)
{
    cfg->trans_cfg.scenario_type = AUDIO_TRANSMITTER_WIRED_AUDIO;
    audio_codec_pcm_t *codec_param = NULL;
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    if (type == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
        cfg->trans_cfg.scenario_type = AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK;
        cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK_I2S0_TO_DAC;
#else
        cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER;
#endif
        cfg->resource_type = AUDIO_SRC_SRV_RESOURCE_TYPE_WIRED_AUDIO;
        cfg->priority = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_RECORD_PRIORITY;
        cfg->source_ctrl_type = AUDIO_SOURCE_CONTROL_USR_LINE_IN;
        codec_param = &cfg->trans_cfg.scenario_config.wired_audio_config.line_in_config.codec_param.pcm;
    } else {
        cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER;
        cfg->resource_type = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC;
        cfg->priority = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_RECORD_PRIORITY;
        cfg->source_ctrl_type = AUDIO_SOURCE_CONTROL_USR_LINE_OUT;
        cfg->trans_cfg.scenario_config.wired_audio_config.line_out_config.is_with_swb = false;
        codec_param = &cfg->trans_cfg.scenario_config.wired_audio_config.line_out_config.codec_param.pcm;
    }
#else
    if (type == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
        cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN;
        cfg->resource_type = AUDIO_SRC_SRV_RESOURCE_TYPE_WIRED_AUDIO;
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
        cfg->priority = 8;
#else
        cfg->priority = AUDIO_SRC_SRV_PRIORITY_MIDDLE;
#endif
        cfg->pseudo_type = AUDIO_SRC_SRV_PSEUDO_DEVICE_LINE_IN;
        codec_param = &cfg->trans_cfg.scenario_config.wired_audio_config.line_in_config.codec_param.pcm;
    } else {
        cfg->trans_cfg.scenario_sub_id = AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT;
        cfg->resource_type = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC;
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
        cfg->priority = 8;
#else
        cfg->priority = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_ULL_UL_PRIORITY;
#endif
        cfg->pseudo_type = 0;
        cfg->trans_cfg.scenario_config.wired_audio_config.line_out_config.is_with_swb = false;
        codec_param = &cfg->trans_cfg.scenario_config.wired_audio_config.line_out_config.codec_param.pcm;
    }
#endif
    codec_param->channel_mode = 2; // stereo
#if 0
    if (type == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
        codec_param->sample_rate = 48000; // 48K
        codec_param->format = HAL_AUDIO_PCM_FORMAT_S32_LE; // 32bit
    } else {
        codec_param->sample_rate = 16000; // 48K
        codec_param->format = HAL_AUDIO_PCM_FORMAT_S16_LE; // 32bit
    }
#else
#ifdef AIR_LINE_IN_SAMPLE_96K
    if (type == APP_AUDIO_TRANS_MGR_USR_LINE_IN) {
        codec_param->sample_rate = 96000;
    } else {
        codec_param->sample_rate = 48000;
    }
#else
    codec_param->sample_rate = 48000; // 48K
#endif
    codec_param->format = HAL_AUDIO_PCM_FORMAT_S24_LE; // 24bit
#endif
}

static void app_line_in_init()
{
    app_audio_trans_mgr_usr_t line_in_cfg;
    memset(&line_in_cfg, 0, sizeof(app_audio_trans_mgr_usr_t));

    line_in_cfg.cfg_callback = app_line_in_cfg_callback;
    line_in_cfg.volume_l = DEFAULT_VOLUME;
    line_in_cfg.volume_r = DEFAULT_VOLUME;
    line_in_cfg.mix_ratio = 100;
    line_in_cfg.mute = false;
    line_in_cfg.en_side_tone = false;
    line_in_cfg.usr_data = NULL;
#ifdef AIR_LINE_IN_ENABLE
    line_in_cfg.volume_l = line_in_cfg.volume_r = bt_sink_srv_ami_get_lineIN_max_volume_level();
    line_in_cfg.type = APP_AUDIO_TRANS_MGR_USR_LINE_IN;
#ifndef AIR_LINE_IN_MIX_ENABLE
    line_in_cfg.ctrl_type = APP_AUDIO_TRANS_MGR_CTRL_TYPE_PSEUDO_DEVICE;
#else
    line_in_cfg.ctrl_type = APP_AUDIO_TRANS_MGR_CTRL_TYPE_RESOURCE_CTRL;
#endif
    line_in_cfg.rt_config_type = WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_LINEIN;
    line_in_cfg.name = "line_in";
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
    line_in_cfg.ctrl_type = APP_AUDIO_TRANS_MGR_CTRL_TYPE_AUD_SRC_CTRL;
#endif
    s_line_in_handle = app_audio_trans_mgr_register(&line_in_cfg);
    app_audio_trans_mgr_init_audio(s_line_in_handle);
#endif

#ifdef AIR_LINE_OUT_ENABLE
    line_in_cfg.type = APP_AUDIO_TRANS_MGR_USR_LINE_OUT;
#ifndef AIR_BTA_IC_STEREO_HIGH_G3
    line_in_cfg.en_side_tone = true;
#else
    line_in_cfg.en_side_tone = false;
#endif
    line_in_cfg.rt_config_type = WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_VOICE_DUL;
    line_in_cfg.ctrl_type = APP_AUDIO_TRANS_MGR_CTRL_TYPE_RESOURCE_CTRL;
    line_in_cfg.name = "line_out";
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
    line_in_cfg.ctrl_type = APP_AUDIO_TRANS_MGR_CTRL_TYPE_AUD_SRC_CTRL;
#endif
    s_line_out_handle = app_audio_trans_mgr_register(&line_in_cfg);
    app_audio_trans_mgr_init_audio(s_line_out_handle);
#endif
}

/**
 * @brief Init the current audio path according to the NVDM data and GPIO status.
 *
 * 1. Read configuration from NVDM.
 * 2. If read value is error (not exist), if the GPIO (line-in) is plugin, set current audio path to be line-in model.
 *    Otherwise, set to be BT.
 * 3. If read value is OK, set to be the saved value.
 */
static void init_audio_path()
{
    nvkey_status_t read_status = 0;
    uint8_t buffer[2] = {0};
    hal_gpio_data_t current_gpio_status = 0;
    bool rtc_level = false;
    app_audio_path_t temp_path = APP_AUDIO_PATH_UNKNOWN;

    uint32_t read_len = sizeof(buffer);

    /* Read the configuration from the NVDM which stored the last configured audio path. */
    read_status = nvkey_read_data(NVID_APP_LINE_IN_AUDIO_PATH, (uint8_t *)buffer, &read_len);

    LINE_IN_LOG_I("init_audio_path -> read common/audio_path from NVDM result : %d", 1, read_status);

    if (read_status != NVKEY_STATUS_OK) {
        temp_path = APP_AUDIO_PATH_LINE_IN;
    } else {
        /* Set the current audio path to be the saved one. */
        LINE_IN_LOG_I("init_audio_path -> read length : %d, value : %d %d", 3, read_len, buffer[0], buffer[1]);
        temp_path = atoi((char *)buffer);
    }

    if (temp_path != APP_AUDIO_PATH_BT) {
        /**
         * If the stored configuration is NONE:
         * Check the current line-in GPIO value, if the line-in already plug-in, switch current audio path to be line-in mode.
         * Otherwise, switch to current audio path to be BT mode.
         */
        if (!line_in_with_rtc_pin) {
            hal_gpio_status_t gpio_status = hal_gpio_get_input(LINE_IN_DETECT_PORT, &current_gpio_status);
            if ((current_gpio_status == LINE_IN_ALREADY_PLUG_IN) && (gpio_status == HAL_GPIO_STATUS_OK)) {
                temp_path = APP_AUDIO_PATH_LINE_IN;
            } else {
                temp_path = APP_AUDIO_PATH_BT;
            }
        } else {
            hal_rtc_status_t rtc_gpio_status = hal_rtc_gpio_get_input(LINE_IN_DETECT_PORT, &rtc_level);
            /* Base on hardware design, high level means line in plug in. */
            if (((hal_rtc_gpio_data_t)rtc_level == (hal_rtc_gpio_data_t)LINE_IN_ALREADY_PLUG_IN) && (rtc_gpio_status == HAL_RTC_STATUS_OK)) {
                temp_path = APP_AUDIO_PATH_LINE_IN;
            } else {
                temp_path = APP_AUDIO_PATH_BT;
            }
#ifdef CS47L50C_DSP_ENABLE
            bool rtc_level = 0;

            hal_rtc_eint_config_t rtc_eint_config;
            rtc_eint_config.is_enable_debounce = true;
            rtc_eint_config.rtc_gpio = LINE_IN_DETECT_PORT;
            rtc_eint_config.is_enable_rtc_eint = true;
            hal_rtc_gpio_get_input(LINE_IN_DETECT_PORT, &rtc_level);
            rtc_eint_config.is_falling_edge_active = rtc_level; //changer trigger edge
            hal_rtc_eint_init(&rtc_eint_config);
#endif
        }
        LINE_IN_LOG_I("read path from gpio : %d", 1, temp_path == APP_AUDIO_PATH_LINE_IN);
        broadcast_line_in_plug_state(false, temp_path == APP_AUDIO_PATH_LINE_IN ? true : false);
    }

    switch_audio_path(temp_path, false);
    if (temp_path == APP_AUDIO_PATH_LINE_IN) {
        app_line_in_switch(temp_path);
    }

    LINE_IN_LOG_I("init_audio_path -> init current audio path to be : %d", 1, current_audio_path_value);
}

static void line_in_detect_callback(void *user_data)
{
    if (!line_in_with_rtc_pin) {
        hal_gpio_data_t current_gpio_status = 0;
        hal_eint_mask(LINE_IN_DETECT_EINT_NUM);
        hal_gpio_get_input(LINE_IN_DETECT_PORT, &current_gpio_status);
        LINE_IN_LOG_I("detect_callback -> detect callback with value : %d", 1, current_gpio_status);

        ui_shell_send_event(true, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_SWITCH_AUDIO_PATH,
                            current_gpio_status == LINE_IN_ALREADY_PLUG_IN ? APPS_EVENTS_INTERACTION_LINE_IN_PLUG_IN : APPS_EVENTS_INTERACTION_LINE_IN_PLUG_OUT,
                            NULL, 0, NULL, 0);

        hal_eint_unmask(LINE_IN_DETECT_EINT_NUM);
    } else {
        /* If the RTC pin used for line in, need debounce. */
        ui_shell_remove_event(EVENT_GROUP_SWITCH_AUDIO_PATH, APPS_EVENTS_INTERACTION_LINE_IN_RTC_PLUG_EV);
        ui_shell_send_event(true, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_SWITCH_AUDIO_PATH,
                            APPS_EVENTS_INTERACTION_LINE_IN_RTC_PLUG_EV,
                            NULL, 0, NULL, 300);
    }
}

static void init_line_in_detect_interrupt()
{
    if (!line_in_with_rtc_pin) {
        hal_eint_config_t config;

        /* For falling and rising detect. */
        config.trigger_mode = HAL_EINT_EDGE_FALLING_AND_RISING;
        config.debounce_time = 300;

        hal_gpio_init(LINE_IN_DETECT_PORT);
        hal_eint_mask(LINE_IN_DETECT_EINT_NUM);
        if (hal_eint_init(LINE_IN_DETECT_EINT_NUM, &config) != HAL_EINT_STATUS_OK) {
            LINE_IN_LOG_E("ini_detect_interrupt -> init eint failed", 0);
            hal_eint_unmask(LINE_IN_DETECT_EINT_NUM);
            return;
        }

        if (hal_eint_register_callback(LINE_IN_DETECT_EINT_NUM, line_in_detect_callback, NULL) != HAL_EINT_STATUS_OK) {
            LINE_IN_LOG_E("ini_detect_interrupt -> eint register callback failed", 0);
            hal_eint_unmask(LINE_IN_DETECT_EINT_NUM);
            hal_eint_deinit(LINE_IN_DETECT_EINT_NUM);
            return;
        }

        hal_eint_unmask(LINE_IN_DETECT_EINT_NUM);
    } else {
        hal_rtc_gpio_config_t rtc_pin_config;
        hal_rtc_eint_config_t rtc_eint_config;
        bool rtc_level = false;

        rtc_pin_config.rtc_gpio = LINE_IN_DETECT_PORT;
        rtc_pin_config.is_analog = false;
        rtc_pin_config.is_input = true;
#ifdef CS47L50C_DSP_ENABLE
        rtc_pin_config.is_pull_up = false;
#else
        rtc_pin_config.is_pull_up = true;
#endif
        rtc_pin_config.is_pull_down = false;
        hal_rtc_gpio_init(&rtc_pin_config);
        hal_rtc_gpio_get_input(LINE_IN_DETECT_PORT, &rtc_level);

        rtc_eint_config.is_enable_debounce = true;
        rtc_eint_config.rtc_gpio = LINE_IN_DETECT_PORT;
        rtc_eint_config.is_enable_rtc_eint = true;
        rtc_eint_config.is_falling_edge_active = rtc_level;
        hal_rtc_eint_init(&rtc_eint_config);

        hal_rtc_eint_register_callback(LINE_IN_DETECT_PORT, line_in_detect_callback, NULL);
    }
}

/*========================================================================================*/
/*                          SP APP RACE CMD HANDLER                                       */
/*========================================================================================*/
#ifdef MTK_RACE_CMD_ENABLE
/**
 * @brief Handle the SP APP RACE CMD of the pull request.
 *
 * @param current_audio_path Output for current audio path.
 */
static void line_in_sp_app_pull_request(uint8_t *current_audio_path)
{
    if (current_audio_path == NULL) {
        LINE_IN_LOG_E("pull_request -> current audio path is NULL", 0);
        return;
    }
    if (current_audio_path_value == APP_AUDIO_PATH_UNKNOWN) {
        *current_audio_path = APP_AUDIO_PATH_BT;
        LINE_IN_LOG_E("pull_request -> current is unknown audio path, return BT audio path", 0);
        return;
    }
    *current_audio_path = current_audio_path_value;
}

static void line_in_sp_app_control_request(uint8_t new_audio_path, uint8_t *control_result)
{
    app_audio_path_t path = (app_audio_path_t)new_audio_path;
    if (control_result == NULL) {
        LINE_IN_LOG_E("control_request -> control result pointer is NULL", 0);
        return;
    }
    if (new_audio_path != APP_AUDIO_PATH_BT && new_audio_path != APP_AUDIO_PATH_LINE_IN) {
        LINE_IN_LOG_E("control_request -> unknown new audio path : %d", 1, new_audio_path);
        return;
    }

    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_SWITCH_AUDIO_PATH, APPS_EVENTS_INTERACTION_AUDIO_PATH_UI_CTRL,
                        (void *)path, 0, NULL, 0);
    *control_result = 0;
}

static void line_in_sp_app_push_response(uint8_t status)
{
    LINE_IN_LOG_I("push_response -> push result : %d", 1, status);
}

static void init_line_in_sp_app_callback()
{
    line_in_app_callback_t callback;
    callback.control_request = line_in_sp_app_control_request;
    callback.pull_request = line_in_sp_app_pull_request;
    callback.push_response = line_in_sp_app_push_response;
    race_cmd_hostaudio_set_app_line_in_callback(&callback);
}
#endif
/*========================================================================================*/
/*                          APP LINE HANDLE FUNCTIONS                                     */
/*========================================================================================*/
static bool app_line_in_handle_system_event(uint32_t event_id,
                                            void *extra_data,
                                            size_t data_len)
{
    bool ret = true;
    uint32_t nvkey_size = sizeof(uint8_t);

    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            /**
             * TODO
             * 1. read the configuration from NVDM.
             * 2. configure the GPIO to register the line-in HW operation.
             * 3. register the BT operation/race command.
             * 4. register the callback to the line-in playback middleware.
             * 5. read the line-in GPIO value.
             */
            if (LINE_IN_DETECT_PORT != 0xFF && LINE_IN_DETECT_EINT_NUM == 0xff) {
                /**
                 * If line in detect pin is a RTC pin, should call different API to operate GPIO.
                 *
                 * Notice!
                 * Base 65_evk default HW design, BSP_LINE_IN_SWITCH_PIN is required.
                 * Base on others HW design, BSP_LINE_IN_SWITCH_PIN may not required.
                 * For customer HW design, these logic must be update to check that whether the RTC pin was used for Line in!!!
                 */
                line_in_with_rtc_pin = true;
            }
            LINE_IN_LOG_I("det_pin=%d, eint=%d", 2, LINE_IN_DETECT_PORT, LINE_IN_DETECT_EINT_NUM);
            nvkey_status_t sta = nvkey_read_data((uint16_t)NVID_APP_LINE_IN_SCENARIO, &s_line_in_type, &nvkey_size);
#if AIR_LINE_IN_MIX_ENABLE
            s_line_in_type = LINE_IN_SCENARIO_TYPE_MIX;
#endif
            LINE_IN_LOG_I("get line in scenario type 0x%x, ret %d", 2, s_line_in_type, sta);
            app_line_in_init();
            init_line_in_detect_interrupt();
#ifdef MTK_RACE_CMD_ENABLE
            init_line_in_sp_app_callback();
#endif
            init_audio_path();
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
            app_power_save_utils_register_get_mode_callback(line_in_get_power_saving_target_mode);
#endif
        }
        break;
    }
    return ret;
}

static uint8_t line_in_volume = DEFAULT_VOLUME;
/**
 * @brief Handle the EINT key event.
 *
 * @param event_id      The event ID of the key.
 * @param extra_data    The extra data of the event.
 * @param data_len      The extra data length.
 * @return              If return true, the current key event cannot be handled by the next activity.
 */
static bool app_line_in_handle_key_event(uint32_t event_id,
                                         void *extra_data,
                                         size_t data_len)
{
    /**
     * TODO
     * Need to switch the line-in state according to the current status.
     * If current is line-in, switch to BT mode.
     * If current is BT, switch to line-in mode.
     * ATTENTION:
     * Need to notify SP application that mode has been changed.
     */
    uint16_t action_id = 0;
    app_audio_path_t path = APP_AUDIO_PATH_UNKNOWN;

    action_id = *(uint16_t *)extra_data;
    if (action_id == KEY_LINE_IN_SWITCH) {
        LINE_IN_LOG_I("key_event -> switch the audio path : (current %d)", 1, current_audio_path_value);
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_SWITCH_AUDIO_PATH, APPS_EVENTS_INTERACTION_AUDIO_PATH_UI_CTRL,
                            (void *)path, 0, NULL, 0);
        return true;
    } else if (action_id == KEY_MUTE_MIC) {
#ifdef AIR_LINE_OUT_ENABLE
        LINE_IN_LOG_I("line out mute, now=%d", 1, s_line_out_mute);
        s_line_out_mute = !s_line_out_mute;
        app_audio_trans_mgr_set_mute(s_line_out_handle, s_line_out_mute);
#endif
    } else if (action_id == KEY_VOICE_UP || action_id == KEY_VOICE_DN) {
        if (current_audio_path_value == APP_AUDIO_PATH_LINE_IN) {
            uint8_t volume = line_in_volume;
            if (KEY_VOICE_UP == action_id) {
                if (volume + 1 < bt_sink_srv_ami_get_lineIN_max_volume_level()) {
                    volume += 1;
                } else {
                    volume = bt_sink_srv_ami_get_lineIN_max_volume_level();
                }
            } else {
                if (volume > 1) {
                    volume -= 1;
                } else {
                    volume = 0;
                }
            }
            LINE_IN_LOG_I(" change volume to %d", 1, volume);
            /* TO-DO: support line in change volume. */
#ifdef AIR_LINE_IN_ENABLE
            app_audio_trans_mgr_set_volume(s_line_in_handle, volume, volume);
#endif
            line_in_volume = volume;
        }
    }
    return false;
}

#ifdef AIR_ROTARY_ENCODER_ENABLE
/**
 * @brief Handle the rotary encoder event.
 *
 * @param event_id      The event ID of the key.
 * @param extra_data    The extra data of the event.
 * @param data_len      The extra data length.
 * @return              If return true, the current key event cannot be handled by the next activity.
 */
static bool app_line_in_handle_rotary_event(ui_shell_activity_t *self,
                                            uint32_t event_id,
                                            void *extra_data,
                                            size_t data_len)
{
    bool ret = false;
    bsp_rotary_encoder_port_t port;
    bsp_rotary_encoder_event_t event;
    uint32_t rotary_data;
    if (!extra_data) {
        return ret;
    }
    apps_config_key_action_t key_action = *(uint16_t *)extra_data;
    app_event_rotary_event_decode(&port, &event, &rotary_data, event_id);

    switch (key_action) {
        case KEY_VOICE_UP:
        case KEY_VOICE_DN: {
            if (current_audio_path_value == APP_AUDIO_PATH_LINE_IN) {
                uint8_t volume = line_in_volume;
                if (KEY_VOICE_UP == key_action) {
                    if (volume + rotary_data < bt_sink_srv_ami_get_lineIN_max_volume_level()) {
                        volume += rotary_data;
                    } else {
                        volume = bt_sink_srv_ami_get_lineIN_max_volume_level();
                    }
                } else {
                    if (volume > rotary_data) {
                        volume -= rotary_data;
                    } else {
                        volume = 0;
                    }
                }
                LINE_IN_LOG_I("rotary change volume to %d", 1, volume);
                /* TO-DO: support line in change volume. */
#ifdef AIR_LINE_IN_ENABLE
                app_audio_trans_mgr_set_volume(s_line_in_handle, volume, volume);
#endif
                line_in_volume = volume;
            }
            break;
        }
        default:
            break;
    }
    return ret;
}
#endif

static bool app_line_in_handle_line_in_event(ui_shell_activity_t *self,
                                             uint32_t event_id,
                                             void *extra_data,
                                             size_t data_len)
{
    switch (event_id) {
        /* Line in plug in or plug out. */
        case APPS_EVENTS_INTERACTION_LINE_IN_PLUG_IN:
        case APPS_EVENTS_INTERACTION_LINE_IN_PLUG_OUT: {
            /* JACK sta: line in plug in. */
            int8_t jack_sta = event_id == APPS_EVENTS_INTERACTION_LINE_IN_PLUG_IN ? 1 : 0;
            app_audio_path_t path = event_id == APPS_EVENTS_INTERACTION_LINE_IN_PLUG_IN ? APP_AUDIO_PATH_LINE_IN : APP_AUDIO_PATH_BT;
            broadcast_line_in_plug_state(false, jack_sta == 1 ? true : false);
            switch_audio_path(path, false);
#ifdef MTK_RACE_CMD_ENABLE
            app_race_send_notify(APPS_RACE_CMD_CONFIG_TYPE_LINE_IN_JACK_STA, &jack_sta, sizeof(uint8_t));
#endif
            app_line_in_switch(path);
        }
        break;
        /* Switch audio path through key pad or phone's app. */
        case APPS_EVENTS_INTERACTION_AUDIO_PATH_UI_CTRL: {
            app_audio_path_t new_path = (app_audio_path_t)extra_data;
            LINE_IN_LOG_I("audio path ctrl: %d", 1, new_path);
            if (new_path == APP_AUDIO_PATH_UNKNOWN) {
                new_path = current_audio_path_value == APP_AUDIO_PATH_BT ? APP_AUDIO_PATH_LINE_IN : APP_AUDIO_PATH_BT;
            }

            broadcast_line_in_plug_state(false, new_path == APP_AUDIO_PATH_LINE_IN ? true : false);
            switch_audio_path(new_path, true);
            app_line_in_switch(new_path);
        }
        break;
        case APPS_EVENTS_INTERACTION_LINE_IN_RTC_PLUG_EV: {
            bool rtc_level = 0;
            hal_rtc_eint_config_t rtc_eint_config;
            rtc_eint_config.is_enable_debounce = true;
            rtc_eint_config.rtc_gpio = LINE_IN_DETECT_PORT;
            rtc_eint_config.is_enable_rtc_eint = true;

            hal_rtc_gpio_get_input(LINE_IN_DETECT_PORT, &rtc_level);
            rtc_eint_config.is_falling_edge_active = rtc_level; //changer trigger edge
            hal_rtc_eint_init(&rtc_eint_config);
            hal_rtc_eint_register_callback(LINE_IN_DETECT_PORT, line_in_detect_callback, NULL);

            LINE_IN_LOG_I("get line in sta(RTC): %d", 1, rtc_level);
            ui_shell_send_event(true, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_SWITCH_AUDIO_PATH,
                                rtc_level == LINE_IN_ALREADY_PLUG_IN ? APPS_EVENTS_INTERACTION_LINE_IN_PLUG_IN : APPS_EVENTS_INTERACTION_LINE_IN_PLUG_OUT,
                                NULL, 0, NULL, 0);
        }
        break;
    }
    return false;
}

/*========================================================================================*/
/*                          IDLE ACTIVITY PRO FUNCTION                                    */
/*========================================================================================*/
bool app_line_in_idle_activity_proc(struct _ui_shell_activity *self,
                                    uint32_t event_group,
                                    uint32_t event_id,
                                    void *extra_data,
                                    size_t data_len)
{
    bool ret = false;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            /* ui_shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = app_line_in_handle_system_event(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_KEY:
            /* key event. */
            ret = app_line_in_handle_key_event(event_id, extra_data, data_len);
            break;
#ifdef AIR_ROTARY_ENCODER_ENABLE
        case EVENT_GROUP_UI_SHELL_ROTARY_ENCODER:
            /**< group for rotary encoder events */
            ret = app_line_in_handle_rotary_event(self, event_id, extra_data, data_len);
            break;
#endif
        case EVENT_GROUP_SWITCH_AUDIO_PATH: {
            ret = app_line_in_handle_line_in_event(self, event_id, extra_data, data_len);
            break;
        }
            /* workaround: the usb audio should resume after a2dp streaming stoped. */
#if 0
#ifdef AIR_LINE_OUT_ENABLE
        case EVENT_GROUP_UI_SHELL_BT_SINK: {
            ret = app_line_out_handle_sink_event(self, event_id, extra_data, data_len);
            break;
        }
#endif
#endif
        /* workaround end */
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            /* interaction events. */
            if (event_id == APPS_EVENTS_INTERACTION_POWER_OFF) {
                /* Close line-in audio before system power off. */
                current_audio_path_value = APP_AUDIO_PATH_BT;
                app_line_in_switch(APP_AUDIO_PATH_BT);
            }
            break;
    }

    return ret;
}

app_audio_path_t app_line_in_activity_get_current_audio_path()
{
    uint32_t mask;
    app_audio_path_t cur_path;

    hal_nvic_save_and_set_interrupt_mask(&mask);
    cur_path = current_audio_path_value;
    hal_nvic_restore_interrupt_mask(mask);

    return cur_path;
}

bool app_line_in_is_plug_in()
{
    bool plug_in = false;

    if (!line_in_with_rtc_pin) {
        hal_gpio_data_t current_gpio_status = 0;
        hal_gpio_get_input(LINE_IN_DETECT_PORT, &current_gpio_status);
        plug_in = current_gpio_status == LINE_IN_ALREADY_PLUG_IN;
    } else {
        bool rtc_level = false;
        hal_rtc_gpio_get_input(LINE_IN_DETECT_PORT, &rtc_level);
        plug_in = ((hal_rtc_gpio_data_t)rtc_level == (hal_rtc_gpio_data_t)LINE_IN_ALREADY_PLUG_IN);
    }

    LINE_IN_LOG_I("get line in plug sta: %d", 1, plug_in);
    return plug_in;
}

