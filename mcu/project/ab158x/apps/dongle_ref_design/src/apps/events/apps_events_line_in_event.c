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
 * File: apps_events_line_in_event.c
 *
 * Description: This file is used to detect line in.
 *
 */
#include "apps_events_line_in_event.h"
#include "apps_events_interaction_event.h"
#include "apps_events_event_group.h"
#include "apps_race_cmd_event.h"
#include "apps_debug.h"
#include "atci.h"
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
#include "app_ull_dongle_idle_activity.h"
#endif

#include "bt_sink_srv_ami.h"

#include "hal_eint.h"
#include "hal_gpio.h"
#include "hal_audio_internal.h"

#include "ui_shell_manager.h"
#include "ui_shell_activity.h"

#include "stdlib.h"

#define  TAG  "LINE_IN_DET"

extern const unsigned char BSP_LINE_IN_EINT;
extern const unsigned char BSP_LINE_IN_DET_PIN;

#define APPS_EVENTS_LINE_IN_EVENT_DEBUG

#define LINE_IN_PLUG_IN_LEVEL HAL_GPIO_DATA_LOW

bool s_curr_line_in_status = false;

static void line_in_detect_callback(void *user_data) {
    hal_gpio_data_t current_gpio_status = 0;
    hal_eint_mask(BSP_LINE_IN_EINT);
    hal_gpio_get_input(BSP_LINE_IN_DET_PIN, &current_gpio_status);
    APPS_LOG_MSGID_I(TAG" get line in gpio sta: %d", 1, current_gpio_status);
    if (current_gpio_status == LINE_IN_PLUG_IN_LEVEL) {
        s_curr_line_in_status = true;
    } else {
        s_curr_line_in_status = false;
    }
    ui_shell_send_event(true, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_LINE_IN,
            APPS_EVENTS_INTERACTION_LINE_IN_STATUS, (void *)s_curr_line_in_status, 0, NULL, 0);
    hal_eint_unmask(BSP_LINE_IN_EINT);
}

static void app_events_line_in_init_set() {
    hal_gpio_data_t current_gpio_status = 0;
    hal_gpio_get_input(BSP_LINE_IN_DET_PIN, &current_gpio_status);
    APPS_LOG_MSGID_I(TAG" the line in pin status is: %d", 1, current_gpio_status);
    if (current_gpio_status == LINE_IN_PLUG_IN_LEVEL) {
        s_curr_line_in_status = true;
    } else {
        s_curr_line_in_status = false;
    }
    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_LINE_IN,
            APPS_EVENTS_INTERACTION_LINE_IN_STATUS, (void *)s_curr_line_in_status, 0, NULL, 0);
}

#ifdef APPS_EVENTS_LINE_IN_EVENT_DEBUG
static atci_status_t _line_in_det_atci(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    uint32_t value = 0;

    switch (parse_cmd->mode) {
    case ATCI_CMD_MODE_EXECUTION: {
        value= atoi(parse_cmd->string_ptr + parse_cmd->name_len + 1);
        APPS_LOG_MSGID_I(TAG" _line_in_det_atci mode = %d, value=%d", 2, parse_cmd->mode, value);
        if (value == 1) {
            s_curr_line_in_status = true;
        } else if(value == 0) {
            s_curr_line_in_status = false;
        }
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_LINE_IN,
                APPS_EVENTS_INTERACTION_LINE_IN_STATUS, (void *)s_curr_line_in_status, 0, NULL, 0);

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

static atci_status_t _line_in_vol_atci(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    char *param = NULL;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            param = parse_cmd->string_ptr + parse_cmd->name_len + 1;
            app_line_in_volume_t *vol = (app_line_in_volume_t *)pvPortMalloc(sizeof(app_line_in_volume_t));
            if (vol == NULL)
            {
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                break;
            }
            memset(vol, 0, sizeof(app_line_in_volume_t));
            if(0 == memcmp("up", param, strlen("up")))
            {
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                app_ull_dongle_change_linein_volume_level(true);
#endif
                vol->vol_action = APP_LINE_IN_VOL_UP;
                param = strchr(param, ',');
                param++;
                //sscanf(param, "%lu,", &vol->vol_level);
                vol->vol_level = (uint8_t)strtoul(param, NULL, 10);
                param = strchr(param, ',');
                param++;
                vol->vol_src = (uint8_t)strtoul(param, NULL, 10);
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (0 == memcmp("down",param, strlen("down"))) {
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
                app_ull_dongle_change_linein_volume_level(false);
#endif
                vol->vol_action = APP_LINE_IN_VOL_DOWN;
                param = strchr(param, ',');
                param++;
                //sscanf(param, "%lu,", &vol->vol_level);
                vol->vol_level = (uint8_t)strtoul(param, NULL, 10);
                param = strchr(param, ',');
                param++;
                vol->vol_src = (uint8_t)strtoul(param, NULL, 10);
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (0 == memcmp("level",param, strlen("level"))) {
                vol->vol_action = APP_LINE_IN_VOL_SET;
                param = strchr(param, ',');
                param++;
                vol->vol_level = (uint8_t)strtoul(param, NULL, 10);
                param = strchr(param, ',');
                param++;
                vol->vol_src = (uint8_t)strtoul(param, NULL, 10);
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                vPortFree(vol);
                vol = NULL;
            }
            if (response.response_flag == ATCI_RESPONSE_FLAG_APPEND_OK) {
                APPS_LOG_MSGID_I(TAG" _line_in_vol_atci: action:%d, level=%d,src=%d.", 3, vol->vol_action, vol->vol_level, vol->vol_src);
                ui_shell_status_t status = ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_LINE_IN,
                                                               APPS_EVENTS_INTERACTION_LINE_IN_VOLUME, (void *)vol, sizeof(app_line_in_volume_t), NULL, 0);
                if (UI_SHELL_STATUS_OK != status) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    vPortFree(vol);
                    vol = NULL;
                }
            }

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

#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
static atci_status_t _line_in_vol_values_atci(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    uint32_t value = 0;

    switch (parse_cmd->mode) {
    case ATCI_CMD_MODE_EXECUTION: {
        value= atoi(parse_cmd->string_ptr + parse_cmd->name_len + 1);
        if (value <= 100) {
            APPS_LOG_MSGID_I(TAG" _line_in_vol_values_atci: volume_values=%d.", 1, value);
            app_ull_dongle_set_linein_volume_value(value);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else {
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
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
#endif //AIR_ULL_DONGLE_LINE_IN_ENABLE

static atci_cmd_hdlr_item_t app_events_line_in_det_atci_cmd_debug[] = {
    {
        .command_head = "AT+LINE_IN_DET",
        .command_hdlr = _line_in_det_atci,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },

    {
       .command_head = "AT+LINE_IN_VOL",
       .command_hdlr = _line_in_vol_atci,
       .hash_value1 = 0,
       .hash_value2 = 0,
    },
#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
    {
       .command_head = "AT+LINE_IN_VOL_VALUES",
       .command_hdlr = _line_in_vol_values_atci,
       .hash_value1 = 0,
       .hash_value2 = 0,
    },
#endif
};

static void app_events_line_in_det_atci_debug_init(void)
{
    atci_status_t ret;

    ret = atci_register_handler(app_events_line_in_det_atci_cmd_debug, sizeof(app_events_line_in_det_atci_cmd_debug) / sizeof(atci_cmd_hdlr_item_t));
    if (ret == ATCI_STATUS_OK) {
        APPS_LOG_MSGID_I(TAG" atci_register_handler register success ", 0);
    } else {
        APPS_LOG_MSGID_I(TAG" atci_register_handler  register fail", 0);
    }
}
#endif //APPS_EVENTS_LINE_IN_EVENT_DEBUG

void app_events_line_in_det_init(void) {
    hal_eint_config_t config;
    hal_eint_status_t sta;

#ifdef APPS_EVENTS_LINE_IN_EVENT_DEBUG
    app_events_line_in_det_atci_debug_init();
#endif

    /* For falling and rising detect. */
    config.trigger_mode = HAL_EINT_EDGE_FALLING_AND_RISING;
    config.debounce_time = 300;

    hal_gpio_init(BSP_LINE_IN_DET_PIN);
    hal_eint_mask(BSP_LINE_IN_EINT);

    sta = hal_eint_init(BSP_LINE_IN_EINT, &config);
    if (sta != HAL_EINT_STATUS_OK) {
        APPS_LOG_MSGID_E(TAG" init line in eint failed: %d", 1, sta);
        hal_eint_unmask(BSP_LINE_IN_EINT);
        return;
    }

    sta = hal_eint_register_callback(BSP_LINE_IN_EINT, line_in_detect_callback, NULL);
    if (sta != HAL_EINT_STATUS_OK) {
        APPS_LOG_MSGID_E(TAG" registe line in eint callback failed: %d", 1, sta);
        hal_eint_unmask(BSP_LINE_IN_EINT);
        hal_eint_deinit(BSP_LINE_IN_EINT);
        return;
    }
    hal_eint_unmask(BSP_LINE_IN_EINT);

    app_events_line_in_init_set();

    APPS_LOG_MSGID_I(TAG" init line in success", 0);
}

