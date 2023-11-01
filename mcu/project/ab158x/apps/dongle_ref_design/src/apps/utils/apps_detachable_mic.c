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
 * File: app_detachable_mic.c
 *
 * Description: This activity is used to detect and switch the detachable MIC.
 *
 */
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
#include "apps_detachable_mic.h"
#include "apps_events_interaction_event.h"
#include "apps_events_event_group.h"
#include "apps_debug.h"
#include "apps_race_cmd_event.h"
#include "atci.h"

#include "bt_sink_srv_ami.h"
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_le_service.h"
#endif

#include "hal_eint.h"
#include "hal_gpio.h"

#include "ui_shell_manager.h"
#include "ui_shell_activity.h"

#include "stdlib.h"

#define LOG_TAG  "[DETACHABLE_MIC]"

#define APPS_EVENTS_DETACHABLE_EVENT_DEBUG

extern const unsigned char BSP_DETACHABLE_MIC_EINT;
extern const unsigned char BSP_DETACHABLE_MIC_DET_PIN;

static voice_mic_type_t s_current_mic = VOICE_MIC_TYPE_FIXED;  /* Indicate that which MIC in using now. */

#define DETACHABLE_MIC_PLUG_IN_LEVEL HAL_GPIO_DATA_LOW
#define DETACHABLE_MIC_PLUG_ANTI_SHAKE_TIME    (500)  //500ms

static void detachable_mic_detect_callback(void *user_data)
{
    hal_gpio_data_t current_gpio_status = 0;
    hal_eint_mask(BSP_DETACHABLE_MIC_EINT);
    hal_gpio_get_input(BSP_DETACHABLE_MIC_DET_PIN, &current_gpio_status);
    if (current_gpio_status == DETACHABLE_MIC_PLUG_IN_LEVEL) {
        s_current_mic = VOICE_MIC_TYPE_DETACHABLE;
    } else {
        s_current_mic = VOICE_MIC_TYPE_FIXED;
    }
    APPS_LOG_MSGID_I(LOG_TAG" get detachable mic gpio sta: %d", 1, current_gpio_status);
#ifdef AIR_DONGLE_ENABLE
    ami_set_voice_mic_type(s_current_mic);
#else
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_SWITCH_MIC);
    ui_shell_send_event(true, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_SWITCH_MIC, NULL, 0, NULL, DETACHABLE_MIC_PLUG_ANTI_SHAKE_TIME);
#endif
    hal_eint_unmask(BSP_DETACHABLE_MIC_EINT);
}

/* Notice that this api can not be called in ISR. */
#define DETACH_MIC_IN 1
#define DETACH_MIC_OUT 0
void app_detachable_mic_switch()
{
    voice_mic_type_t c_mic = s_current_mic;
    //uint8_t detach_mic_sta = DETACH_MIC_OUT;
    bt_sink_srv_am_result_t result = ami_set_voice_mic_type(c_mic);
    APPS_LOG_MSGID_I(LOG_TAG" mic_switch: type=%d, result=%d", 2, c_mic, result);
    //detach_mic_sta = c_mic == VOICE_MIC_TYPE_DETACHABLE ? DETACH_MIC_IN : DETACH_MIC_OUT;
    //app_race_send_notify(APPS_RACE_CMD_CONFIG_TYPE_DETACH_MIC_JACK_STA, (int8_t *)&detach_mic_sta, sizeof(uint8_t));

}

static void detachable_mic_init_set()
{
    hal_gpio_data_t current_gpio_status = 0;
    hal_gpio_get_input(BSP_DETACHABLE_MIC_DET_PIN, &current_gpio_status);
    APPS_LOG_MSGID_I(LOG_TAG" the detachable det pin status is: %d", 1, current_gpio_status);
    if (current_gpio_status == DETACHABLE_MIC_PLUG_IN_LEVEL) {
        s_current_mic = VOICE_MIC_TYPE_DETACHABLE;
    } else {
        s_current_mic = VOICE_MIC_TYPE_FIXED;
    }
    app_detachable_mic_switch();
}

#ifdef APPS_EVENTS_DETACHABLE_EVENT_DEBUG
static atci_status_t _detachale_mic_det_atci(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    uint32_t value = 0;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            value = atoi(parse_cmd->string_ptr + parse_cmd->name_len + 1);
            APPS_LOG_MSGID_I(LOG_TAG" _detachale_mic_det_atci mode = %d, value=%d, curr_mic=%d",
                             3, parse_cmd->mode, value, s_current_mic);
            if (value != s_current_mic) {
                s_current_mic = (value != 0) ? VOICE_MIC_TYPE_DETACHABLE : VOICE_MIC_TYPE_FIXED;
#ifdef AIR_DONGLE_ENABLE
                ami_set_voice_mic_type(s_current_mic);
#else
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_SWITCH_MIC);
                ui_shell_send_event(true, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_SWITCH_MIC, NULL, 0, NULL, DETACHABLE_MIC_PLUG_ANTI_SHAKE_TIME);
#endif
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

static atci_cmd_hdlr_item_t app_events_detachable_mic_det_atci_cmd_debug[] = {
    {
        .command_head = "AT+MIC_DET",
        .command_hdlr = _detachale_mic_det_atci,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};
#endif

void app_detachable_mic_det_init()
{
    hal_eint_config_t config;
    hal_eint_status_t sta;

#ifdef APPS_EVENTS_DETACHABLE_EVENT_DEBUG
    atci_register_handler(app_events_detachable_mic_det_atci_cmd_debug, sizeof(app_events_detachable_mic_det_atci_cmd_debug) / sizeof(atci_cmd_hdlr_item_t));
#endif

    /* For falling and rising detect. */
    config.trigger_mode = HAL_EINT_EDGE_FALLING_AND_RISING;
    config.debounce_time = 300;

    hal_gpio_init(BSP_DETACHABLE_MIC_DET_PIN);
    hal_eint_mask(BSP_DETACHABLE_MIC_EINT);

    sta = hal_eint_init(BSP_DETACHABLE_MIC_EINT, &config);
    if (sta != HAL_EINT_STATUS_OK) {
        APPS_LOG_MSGID_E(LOG_TAG" init detachable mic eint failed: %d", 1, sta);
        hal_eint_unmask(BSP_DETACHABLE_MIC_EINT);
        return;
    }

    sta = hal_eint_register_callback(BSP_DETACHABLE_MIC_EINT, detachable_mic_detect_callback, NULL);
    if (sta != HAL_EINT_STATUS_OK) {
        APPS_LOG_MSGID_E(LOG_TAG" registe detachable mic eint callback failed: %d", 1, sta);
        hal_eint_unmask(BSP_DETACHABLE_MIC_EINT);
        hal_eint_deinit(BSP_DETACHABLE_MIC_EINT);
        return;
    }
    hal_eint_unmask(BSP_DETACHABLE_MIC_EINT);

    detachable_mic_init_set();

    APPS_LOG_MSGID_I(LOG_TAG" init detachable mic success", 0);
}
void detachable_mic_set_mic_type(voice_mic_type_t mic)
{
    APPS_LOG_MSGID_I(LOG_TAG" mic_switch by atci: ori=%d, cur=%d", 2, s_current_mic, mic);
    s_current_mic = mic;
}

#endif

