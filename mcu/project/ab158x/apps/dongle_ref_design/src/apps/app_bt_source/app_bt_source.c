
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
 * File: app_bt_source.c
 *
 * Description: This file provides API for BT Source APP.
 *
 */

#ifdef AIR_BT_SOURCE_ENABLE

#include "app_bt_source.h"

#include "app_bt_source_conn_mgr.h"
#include "app_bt_source_music.h"
#include "app_bt_source_call.h"

#include "apps_debug.h"
#include "app_dongle_connection_common.h"
#include "apps_events_bt_event.h"
#include "apps_events_event_group.h"
#include "apps_events_usb_event.h"
#include "app_preproc_activity.h"
#include "atci.h"
#include "bt_connection_manager.h"
#include "bt_device_manager.h"
#include "bt_source_srv.h"
#include "FreeRTOS.h"
#include "usbaudio_drv.h"
#include "usb_hid_srv.h"
#include "ui_shell_manager.h"
#include "bt_customer_config.h"
#include "bt_sink_srv_ami.h"
#ifdef AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
#include "apps_events_i2s_in_event.h"
#endif
#ifdef AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE
#include "apps_events_line_in_event.h"
#include "apps_events_interaction_event.h"
#endif

#include "app_preproc_activity.h"

#define LOG_TAG             "[BT_SRC][APP]"


#define APP_BT_SOURCE_USB_STREAMING_NONE            0x00
#define APP_BT_SOURCE_USB_STREAMING_MUSIC           0x01
#define APP_BT_SOURCE_USB_STREAMING_CALL            0x02

typedef struct {
    bool                                            chat_spk_port_enable;
    bool                                            game_spk_port_enable;
    bool                                            mic_port_enable;
    bool                                            line_in_port_enable;
    bool                                            i2s_0_in_port_enable;
    bool                                            i2s_1_in_port_enable;
} PACKED app_bt_source_context_t;

static app_bt_source_context_t                      app_bt_source_ctx = {0};

/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
static atci_status_t app_bt_source_atcmd_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            char *atcmd = parse_cmd->string_ptr + parse_cmd->name_len + 1;
            char cmd[20] = {0};
            memcpy(cmd, atcmd, strlen(atcmd) - 2);
            bool correct_cmd_flag = TRUE;
            uint32_t event_id = 0;
            APPS_LOG_I(LOG_TAG" APP ATCMD string=%s", cmd);

            if (strstr(cmd, "RESET") > 0) {
                event_id = APP_BT_SOURCE_EVENT_RESET;
            } else {
                correct_cmd_flag = FALSE;
            }

            memset(response.response_buf, 0, ATCI_UART_TX_FIFO_BUFFER_SIZE);
            response.response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
            if (correct_cmd_flag) {
                snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "OK - %s", atcmd);
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, event_id);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, event_id,
                                    NULL, 0, NULL, 0);
            } else {
                snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "ERROR - %s", atcmd);
            }
            break;
        }
        default: {
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
        }
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_cmd_hdlr_item_t app_bt_source_atci_cmd[] = {
    {
        .command_head = "AT+BTSRC",
        .command_hdlr = app_bt_source_atcmd_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

static bt_source_srv_port_t app_bt_source_covert_port(app_usb_audio_port_t port_type, uint8_t port_num)
{
    bt_source_srv_port_t port = BT_SOURCE_SRV_PORT_NONE;
    if (port_type == APP_USB_AUDIO_SPK_PORT) {
        if (port_num == 0) {
            port = BT_SOURCE_SRV_PORT_CHAT_SPEAKER;
        } else if (port_num == 1) {
            port = BT_SOURCE_SRV_PORT_GAMING_SPEAKER;
        }
    } else if (port_type == APP_USB_AUDIO_MIC_PORT) {
        port = BT_SOURCE_SRV_PORT_MIC;
    }

//    APPS_LOG_MSGID_I(LOG_TAG" covert_port, port_type=%d port_num=%d bt_source_port=%d",
//                     3, port_type, port_num, port);
    return port;
}

static bool app_bt_source_activity_is_port_ready_to_disable(bt_source_srv_port_t port)
{
#ifdef AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
    if ((port == BT_SOURCE_SRV_PORT_I2S_IN)
        && (app_bt_source_ctx.i2s_0_in_port_enable == false)) {
        return false;
    }

    if ((port == BT_SOURCE_SRV_PORT_I2S_IN_1)
        && (app_bt_source_ctx.i2s_1_in_port_enable == false)) {
        return false;
    }
#endif /* AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE */

#ifdef AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE
    if ((port == BT_SOURCE_SRV_PORT_LINE_IN)
        && (app_bt_source_ctx.line_in_port_enable == false)) {
        return false;
    }
#endif /* AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE */

    if ((port == BT_SOURCE_SRV_PORT_MIC)
        && (app_bt_source_ctx.mic_port_enable == false)) {
        return false;
    }

    if ((port == BT_SOURCE_SRV_PORT_GAMING_SPEAKER)
        && (app_bt_source_ctx.game_spk_port_enable == false)) {
        return false;
    }

    if ((port == BT_SOURCE_SRV_PORT_CHAT_SPEAKER)
        && (app_bt_source_ctx.chat_spk_port_enable == false)) {
        return false;
    }

    return true;
}

#if 0
static uint32_t app_bt_source_covert_sample_rate(app_usb_audio_sample_rate_t rate)
{
    uint32_t rate_integer = 48000;
    switch (rate) {
        case APP_USB_AUDIO_SAMPLE_RATE_16K:
            rate_integer = 16000;
            break;
        case APP_USB_AUDIO_SAMPLE_RATE_24K:
            rate_integer = 24000;
            break;
        case APP_USB_AUDIO_SAMPLE_RATE_32K:
            rate_integer = 32000;
            break;
        case APP_USB_AUDIO_SAMPLE_RATE_44_1K:
            rate_integer = 44100;
            break;
        case APP_USB_AUDIO_SAMPLE_RATE_48K:
            rate_integer = 48000;
            break;
        case APP_USB_AUDIO_SAMPLE_RATE_96K:
            rate_integer = 96000;
            break;
        case APP_USB_AUDIO_SAMPLE_RATE_192K:
            rate_integer = 192000;
            break;
        default:
            break;
    }
    return rate_integer;
}
#endif

bool app_bt_source_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length)
{
    if (action == BT_SOURCE_SRV_ACTION_SWITCH_AUDIO_PATH) {
        // do nothing, need to send action to Call Source
    } else if (action >= BT_SOURCE_MODULE_CALL_ACTION && !app_bt_source_conn_mgr_is_connected()) {
        APPS_LOG_MSGID_E(LOG_TAG" send_action, error disconnected action=0x%08X", 1, action);
        return FALSE;
    }

    bt_status_t bt_status = bt_source_srv_send_action(action, parameter, length);
    APPS_LOG_MSGID_I(LOG_TAG" send_action, action=0x%08X bt_status=0x%08X", 2, action, bt_status);
    return (bt_status == BT_STATUS_SUCCESS);
}

#if defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
static void app_bt_source_handle_line_in_volume(void *extra_data)
{
    if (extra_data == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" LINE_IN_VOLUME, extra_data is NULL", 0);
        return;
    }

    app_line_in_volume_t *line_in_vol = (app_line_in_volume_t *)extra_data;
    bool success = false;

    switch (line_in_vol->vol_action) {
        case APP_LINE_IN_VOL_UP: {
            bt_source_srv_action_t action = BT_SOURCE_SRV_ACTION_VOLUME_UP;
            bt_source_srv_volume_up_t volume_up = {0};
            volume_up.port = BT_SOURCE_SRV_PORT_LINE_IN;
            success = app_bt_source_send_action(action, (void*)&volume_up, sizeof(bt_source_srv_volume_up_t));
            break;
        }
        case APP_LINE_IN_VOL_DOWN: {
            bt_source_srv_action_t action = BT_SOURCE_SRV_ACTION_VOLUME_DOWN;
            bt_source_srv_volume_down_t volume_down = {0};
            volume_down.port = BT_SOURCE_SRV_PORT_LINE_IN;
            success = app_bt_source_send_action(action, (void*)&volume_down, sizeof(bt_source_srv_volume_down_t));
            break;
        }
        case APP_LINE_IN_VOL_SET: {
            bt_source_srv_action_t action = BT_SOURCE_SRV_ACTION_VOLUME_CHANGE;
            bt_source_srv_volume_change_t volume_change = {0};
            volume_change.port           = BT_SOURCE_SRV_PORT_LINE_IN;
            volume_change.volume_value   = line_in_vol->vol_level;
            if (line_in_vol->vol_src == APP_LINE_IN_VOL_SRC) {
                volume_change.is_local = BT_SOURCE_SRV_SET_VOLUME_TO_LOCAL;
            } else {
                volume_change.is_local = BT_SOURCE_SRV_SET_VOLUME_TO_REMOTE;
            }
            success = app_bt_source_send_action(action, (void*)&volume_change, sizeof(bt_source_srv_volume_change_t));
            break;
        }
        default:
            break;
    }

    APPS_LOG_MSGID_I(LOG_TAG" LINE_IN_VOLUME:action : %d, success : %d", 2, line_in_vol->vol_action, success);
}
#endif /* AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE */

#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE)
static void app_bt_source_handle_i2s_in_volume(void *extra_data)
{
    if (extra_data == NULL) {
        return;
    }

    app_i2s_in_vol_t *i2s_in_vol = (app_i2s_in_vol_t *)extra_data;
    bool success = false;

    switch (i2s_in_vol->vol_action) {
        case APP_I2S_IN_VOL_UP: {
            bt_source_srv_action_t action = BT_SOURCE_SRV_ACTION_VOLUME_UP;
            bt_source_srv_volume_up_t volume_up = {0};
            volume_up.port = BT_SOURCE_SRV_PORT_I2S_IN;
            success = app_bt_source_send_action(action, (void*)&volume_up, sizeof(bt_source_srv_volume_up_t));
            break;
        }
        case APP_I2S_IN_VOL_DOWN: {
            bt_source_srv_action_t action = BT_SOURCE_SRV_ACTION_VOLUME_DOWN;
            bt_source_srv_volume_down_t volume_down = {0};
            volume_down.port = BT_SOURCE_SRV_PORT_I2S_IN;
            success = app_bt_source_send_action(action, (void*)&volume_down, sizeof(bt_source_srv_volume_down_t));
            break;
        }
        case APP_I2S_IN_VOL_SET: {
            bt_source_srv_action_t action = BT_SOURCE_SRV_ACTION_VOLUME_CHANGE;
            bt_source_srv_volume_change_t volume_change = {0};
            volume_change.port           = BT_SOURCE_SRV_PORT_I2S_IN;
            volume_change.volume_value   = i2s_in_vol->vol_level;
            if (i2s_in_vol->vol_src == APP_I2S_IN_VOL_SRC) {
                volume_change.is_local = BT_SOURCE_SRV_SET_VOLUME_TO_LOCAL;
            } else {
                volume_change.is_local = BT_SOURCE_SRV_SET_VOLUME_TO_REMOTE;
            }
            success = app_bt_source_send_action(action, (void*)&volume_change, sizeof(bt_source_srv_volume_change_t));
            break;
        }
        default:
            break;
    }

    APPS_LOG_MSGID_I(LOG_TAG" I2S_IN_VOLUME:action : %d, success : %d", 2, i2s_in_vol->vol_action, success);
}
#endif /* AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE */

static void app_bt_source_handle_audio_port_change(bt_source_srv_port_t port, bool enable)
{
    bt_source_srv_audio_port_t port_param = {0};
    port_param.port = port;
    port_param.state = (enable ? BT_SOURCE_SRV_AUDIO_PORT_STATE_ENABLE : BT_SOURCE_SRV_AUDIO_PORT_STATE_DISABLE);
    bool success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_AUDIO_PORT, &port_param, sizeof(port_param));

    bool exist_call = app_bt_source_call_exist_call();

    APPS_LOG_MSGID_I(LOG_TAG" AUDIO_PORT_CHANGE, port : 0x%02x, enable : %d, success : %d, exist_call : %d, mic_port_enable : %d",
                        5,
                        port,
                        enable,
                        success,
                        exist_call,
                        app_bt_source_ctx.mic_port_enable);

    if (success == true) {
        if ((port == BT_SOURCE_SRV_PORT_GAMING_SPEAKER)
                || (port == BT_SOURCE_SRV_PORT_CHAT_SPEAKER)
                || (port == BT_SOURCE_SRV_PORT_MIC)) {
            if ((exist_call == false) && (app_bt_source_ctx.mic_port_enable == false)) {
                if ((app_bt_source_ctx.chat_spk_port_enable == true) || (app_bt_source_ctx.game_spk_port_enable == true)) {
                    app_bt_source_music_start_streaming(true, true);
                } else {
                    app_bt_source_music_start_streaming(false, true);
                }
            }
        } else {
            if (port != BT_SOURCE_SRV_PORT_MIC) {
                app_bt_source_music_start_streaming(enable, false);
            }
        }
    }
}

static uint8_t app_bt_source_get_usb_streaming_state(void)
{
    uint8_t state = APP_BT_SOURCE_USB_STREAMING_NONE;
    if (app_bt_source_ctx.mic_port_enable) {
        state = APP_BT_SOURCE_USB_STREAMING_CALL;
    } else if (app_bt_source_ctx.game_spk_port_enable || app_bt_source_ctx.chat_spk_port_enable) {
        state = APP_BT_SOURCE_USB_STREAMING_MUSIC;
    }
    return state;
}

static void app_bt_source_handle_usb_info(bt_source_srv_port_t port, uint8_t channel,
                                          uint8_t sample_size, uint32_t sample_rate)
{
    bt_source_srv_audio_sample_channel_t channel_param = {0};
    channel_param.port = port;
    channel_param.channel = channel;
    app_bt_source_send_action(BT_SOURCE_SRV_ACTION_AUDIO_SAMPLE_CHANNEL, &channel_param, sizeof(channel_param));

    bt_source_srv_audio_sample_size_t size_param = {0};
    size_param.port = port;
    size_param.sample_size = sample_size;
    app_bt_source_send_action(BT_SOURCE_SRV_ACTION_AUDIO_SAMPLE_SIZE, &size_param, sizeof(size_param));

    bt_source_srv_audio_sample_rate_t rate_param = {0};
    rate_param.port = port;
    rate_param.sample_rate = sample_rate;
    app_bt_source_send_action(BT_SOURCE_SRV_ACTION_AUDIO_SAMPLE_RATE, &rate_param, sizeof(rate_param));
}

static void app_bt_source_handle_usb_streaming(bt_source_srv_port_t port, bool enable)
{
    if (port == BT_SOURCE_SRV_PORT_NONE) {
        APPS_LOG_MSGID_E(LOG_TAG" handle_usb_streaming, error port", 0);
        return;
    }

    // bool exist_call = app_bt_source_call_exist_call();
    if (enable) {
        APPS_LOG_MSGID_W(LOG_TAG" handle_usb_streaming, AUDIO_PLAY port=%d", 1, port);
    } else {
        APPS_LOG_MSGID_W(LOG_TAG" handle_usb_streaming, AUDIO_STOP port=%d", 1, port);

        if ((enable == false)
            && (app_bt_source_activity_is_port_ready_to_disable(port) == false)) {
            APPS_LOG_MSGID_W(LOG_TAG" handle_usb_streaming, is not ready to disable, port : %d, game_spk_enable : %d, chat_spk_enable : %d, mic_enable : %d",
                            4,
                            port,
                            app_bt_source_ctx.game_spk_port_enable,
                            app_bt_source_ctx.chat_spk_port_enable,
                            app_bt_source_ctx.mic_port_enable);
            return;
        }
    }

    // Update app_bt_source context
    if (port == BT_SOURCE_SRV_PORT_GAMING_SPEAKER) {
        app_bt_source_ctx.game_spk_port_enable = enable;
    } else if (port == BT_SOURCE_SRV_PORT_CHAT_SPEAKER) {
        app_bt_source_ctx.chat_spk_port_enable = enable;
    } else if (port == BT_SOURCE_SRV_PORT_MIC) {
        app_bt_source_ctx.mic_port_enable = enable;
    }
    APPS_LOG_MSGID_I(LOG_TAG" handle_usb_streaming, game=%d chat=%d mic=%d line_in=%d i2s_0_in=%d i2s_1_in%d",
                     6, app_bt_source_ctx.game_spk_port_enable, app_bt_source_ctx.chat_spk_port_enable,
                     app_bt_source_ctx.mic_port_enable, app_bt_source_ctx.line_in_port_enable,
                     app_bt_source_ctx.i2s_0_in_port_enable, app_bt_source_ctx.i2s_1_in_port_enable);

    app_bt_source_handle_audio_port_change(port, enable);

#if 0
    // Notify port state to BT_SOURCE_SRV
    bt_source_srv_audio_port_t port_param = {0};
    port_param.port = port;
    port_param.state = (enable ? BT_SOURCE_SRV_AUDIO_PORT_STATE_ENABLE : BT_SOURCE_SRV_AUDIO_PORT_STATE_DISABLE);
    app_bt_source_send_action(BT_SOURCE_SRV_ACTION_AUDIO_PORT, &port_param, sizeof(port_param));

    // Do BT_SOURCE_SRV action
    if (port == BT_SOURCE_SRV_PORT_GAMING_SPEAKER || port == BT_SOURCE_SRV_PORT_CHAT_SPEAKER) {
        if (!exist_call && !app_bt_source_ctx.mic_port_enable) {
            app_bt_source_music_start_streaming(enable, TRUE);
        }
    }
#endif
}

static void app_bt_source_handle_usb_reset(void)
{
    app_bt_source_handle_usb_streaming(BT_SOURCE_SRV_PORT_CHAT_SPEAKER, FALSE);
#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
    app_bt_source_handle_usb_streaming(BT_SOURCE_SRV_PORT_GAMING_SPEAKER, FALSE);
#endif
    app_bt_source_handle_usb_streaming(BT_SOURCE_SRV_PORT_MIC, FALSE);
}

/**================================================================================*/
/**                             Dongle CM Handler for co-exist                     */
/**================================================================================*/
bt_status_t app_bt_source_activity_start_source(const bt_addr_t addr, app_dongle_cm_start_source_param_t param)
{
    const uint8_t *addr1 = addr.addr;
    app_dongle_cm_lea_mode_t bta_mode = param.bta_mode;
    APPS_LOG_MSGID_I(LOG_TAG" start_source, addr=%02X:%02X:%02X:%02X:%02X:%02X bta_mode=%d",
                     7, addr1[5], addr1[4], addr1[3], addr1[2], addr1[1], addr1[0], bta_mode);

    if (bta_mode == APP_DONGLE_CM_BTA_MODE_CONNECT_ASSIGN) {
        app_bt_source_conn_mgr_enable((uint8_t *)addr1);
    } else {
        app_bt_source_conn_mgr_enable(NULL);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t app_bt_source_activity_stop_source(const bt_addr_t addr, app_dongle_cm_stop_source_param_t param)
{
    const uint8_t *addr1 = addr.addr;
    APPS_LOG_MSGID_I(LOG_TAG" stop_source, addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     6, addr1[5], addr1[4], addr1[3], addr1[2], addr1[1], addr1[0]);
    app_bt_source_conn_mgr_disable();
    return BT_STATUS_SUCCESS;
}

bt_status_t app_bt_source_activity_pre_check(app_dongle_cm_precheck_data_t *check_data)
{
    APPS_LOG_MSGID_I(LOG_TAG" pre_check", 0);
    return BT_STATUS_SUCCESS;
}



/**================================================================================*/
/**                                 APP Event Handler                              */
/**================================================================================*/
static bool app_bt_source_activity_proc_ui_shell_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(LOG_TAG" CREATE", 0);
#ifndef APP_BT_SOURCE_ONLY_MODE
            app_dongle_cm_handle_t cm_handle_callback = {
                .start_source = app_bt_source_activity_start_source,
                .stop_source = app_bt_source_activity_stop_source,
                .precheck = app_bt_source_activity_pre_check,
            };
            app_dongle_cm_register_handle(APP_DONGLE_CM_SOURCE_BTA, &cm_handle_callback);
#endif

            bt_source_srv_init_parameter_t init_param = {0};
            init_param.hfp_init_parameter.battery_level = 100;
            bt_source_srv_init(&init_param);

            app_bt_source_conn_mgr_init();
            app_bt_source_music_init();
            app_bt_source_call_init();

            atci_register_handler(app_bt_source_atci_cmd, sizeof(app_bt_source_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
        }
        break;
    }
    return TRUE;
}

static bool app_bt_source_activity_proc_bt_source_group(uint32_t event_id,
                                                        void *extra_data,
                                                        size_t data_len)
{
    USB_HID_Status_t status = USB_HID_IS_NOT_READY;
    switch (event_id) {
        case BT_SOURCE_SRV_EVENT_VOLUME_UP: {
            bt_source_srv_volume_up_ind_t *ind = (bt_source_srv_volume_up_ind_t *)extra_data;
            status = USB_HID_VolumeUp(1);
            APPS_LOG_MSGID_I(LOG_TAG" VOLUME_UP port=%d hid_status=%d", 2, ind->port, status);
            break;
        }

        case BT_SOURCE_SRV_EVENT_VOLUME_DOWN: {
            bt_source_srv_volume_down_ind_t *ind = (bt_source_srv_volume_down_ind_t *)extra_data;
            status = USB_HID_VolumeDown(1);
            APPS_LOG_MSGID_I(LOG_TAG" VOLUME_DOWN port=%d hid_status=%d", 2, ind->port, status);
            break;
        }

        case BT_SOURCE_SRV_EVENT_VOLUME_CHANGE: {
            bt_source_srv_volume_change_ind_t *ind = (bt_source_srv_volume_change_ind_t *)extra_data;
            if (ind->port == BT_SOURCE_SRV_PORT_GAMING_SPEAKER || ind->port == BT_SOURCE_SRV_PORT_CHAT_SPEAKER) {
                if (ind->step_type == BT_SOURCE_SRV_VOLUME_STEP_TYPE_UP) {
                    status = USB_HID_VolumeUp(ind->volume_step);
                } else if (ind->step_type == BT_SOURCE_SRV_VOLUME_STEP_TYPE_DOWN) {
                    status = USB_HID_VolumeDown(ind->volume_step);
                }
            } else if (ind->port == BT_SOURCE_SRV_PORT_MIC) {
                if (ind->mute_state == BT_SOURCE_SRV_MUTE_STATE_ENABLE
                    || ind->mute_state == BT_SOURCE_SRV_MUTE_STATE_DISABLE) {
                    status = usb_hid_srv_send_action(USB_HID_SRV_ACTION_TOGGLE_MIC_MUTE, NULL);
                }
            }

            APPS_LOG_MSGID_I(LOG_TAG" VOLUME_CHANGE port=%d type=%d step=%d mute=%d hid_status=%d",
                             5, ind->port, ind->step_type, ind->volume_step, ind->mute_state, status);
            break;
        }
    }

    return FALSE;
}

static bool app_bt_source_activity_proc_bt_source_app_group(uint32_t event_id,
                                                            void *extra_data,
                                                            size_t data_len)
{
    bool ret = TRUE;
    switch (event_id) {
        case APP_BT_SOURCE_EVENT_ENABLE: {
            app_bt_source_conn_mgr_enable(NULL);
            break;
        }

        case APP_BT_SOURCE_EVENT_DISABLE: {
            app_bt_source_conn_mgr_disable();
            break;
        }

        case APP_BT_SOURCE_EVENT_NOTIFY_CONN_CONNECTED: {
            uint8_t state = app_bt_source_get_usb_streaming_state();
            APPS_LOG_MSGID_I(LOG_TAG" NOTIFY_CONN_CONNECTED event, usb_streaming_state=%d", 1, state);
            if (state == APP_BT_SOURCE_USB_STREAMING_MUSIC) {
                app_bt_source_music_start_streaming(TRUE, FALSE);
            } else if (state == APP_BT_SOURCE_USB_STREAMING_CALL) {
                // Note: no need to new_active_call when MIC port enable then BT Connected
                // app_bt_source_call_new_active_call();
                // Note: When MIC port open then HFP reconnect, even if not call state, Source (call) middleware will create eSCO
            }
            break;
        }

        case APP_BT_SOURCE_EVENT_NOTIFY_CONN_DISCONNECTED: {
            break;
        }
    }

    if (event_id >= APP_BT_SOURCE_EVENT_PUBLIC_EVENT_BASE) {
        ret = FALSE;
    }
    return ret;
}

static bool app_bt_source_activity_proc_usb_group(uint32_t event_id,
                                                  void *extra_data,
                                                  size_t data_len)
{
    bool success = FALSE;
    uint32_t usb_data = (uint32_t)extra_data;
    //APPS_LOG_MSGID_I(LOG_TAG" USB event, event_id=%d extra_data=0x%08X", 2, event_id, extra_data);
    switch (event_id) {
        case APPS_EVENTS_USB_AUDIO_PLAY:
        case APPS_EVENTS_USB_AUDIO_STOP: {
            app_events_usb_port_t *usb_port = (app_events_usb_port_t *)&usb_data;
            if (usb_port != NULL) {
                bt_source_srv_port_t port = app_bt_source_covert_port(usb_port->port_type, usb_port->port_num);
                uint8_t interface_id = apps_event_usb_get_interface_id_from_port_info(usb_port);
                const apps_usb_interface_enable_app_task_recorder_t *usb_info = app_preproc_activity_get_usb_interface_info(interface_id);
                APPS_LOG_MSGID_W(LOG_TAG" USB event, AUDIO_PLAY/STOP port=%d enable=%d channel=%d size=%d rate=%d",
                                 5, port, usb_info->enabled, usb_info->channel, usb_info->sample_size, usb_info->sample_rate);
                app_bt_source_handle_usb_info(port, usb_info->channel, usb_info->sample_size, usb_info->sample_rate);
                app_bt_source_handle_usb_streaming(port, usb_info->enabled);
                success = TRUE;
            }
            break;
        }

        case APPS_EVENTS_USB_AUDIO_VOLUME: {
            app_events_usb_volume_t *usb_volume = (app_events_usb_volume_t *)extra_data;
            if (usb_volume != NULL) {
                bt_source_srv_volume_change_t volume_change = {0};
                const uint8_t *device_addr = app_bt_source_conn_mgr_get_active_device();
                bool support_absolute_volume = bt_source_srv_get_remote_absolute_volume_information((const bt_bd_addr_t *)device_addr);
                bt_source_srv_port_t port = app_bt_source_covert_port(usb_volume->port_type, usb_volume->port_num);
                APPS_LOG_MSGID_W(LOG_TAG" USB event, AUDIO_VOLUME support_absolute_volume=%d port=%d volume=%d %d, db=%d %d",
                                 6, support_absolute_volume, port,
                                 usb_volume->left_volume, usb_volume->right_volume,
                                 usb_volume->left_db, usb_volume->right_db);

                volume_change.port = port;
                volume_change.volume_value = bt_customer_config_get_volume_by_gain(port, usb_volume->left_db > usb_volume->right_db ? usb_volume->left_db : usb_volume->right_db);
                volume_change.is_local = BT_SOURCE_SRV_SET_VOLUME_TO_REMOTE;
                success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_VOLUME_CHANGE,
                                                    (void*)&volume_change, sizeof(bt_source_srv_volume_change_t));
            }
            break;
        }

        case APPS_EVENTS_USB_AUDIO_MUTE: {
            app_events_usb_mute_t *usb_mute = (app_events_usb_mute_t *)&usb_data;
            if (usb_mute != NULL) {
                bt_source_srv_port_t port = app_bt_source_covert_port(usb_mute->port_type, usb_mute->port_num);
                bt_source_srv_audio_mute_t mute_param = {.port = port};
                APPS_LOG_MSGID_W(LOG_TAG" USB event, USB_AUDIO_MUTE port=%d is_mute=%d",
                                 2, port, usb_mute->is_mute);
                if (usb_mute->is_mute) {
                    success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_MUTE, &mute_param, sizeof(bt_source_srv_audio_mute_t));
                } else {
                    success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_UNMUTE, &mute_param, sizeof(bt_source_srv_audio_mute_t));
                }
            } else {
                APPS_LOG_MSGID_E(LOG_TAG", received APPS_EVENTS_USB_AUDIO_MUTE, but data not correct", 0);
            }
            break;
        }

        case APPS_EVENTS_USB_AUDIO_SAMPLE_RATE: {
            app_events_usb_sample_rate_t *usb_rate = (app_events_usb_sample_rate_t *)&usb_data;
            if (usb_rate != NULL) {
                bt_source_srv_port_t port = app_bt_source_covert_port(usb_rate->port.port_type, usb_rate->port.port_num);
                uint8_t interface_id = apps_event_usb_get_interface_id_from_port_info(&usb_rate->port);
                const apps_usb_interface_enable_app_task_recorder_t *interface_status = app_preproc_activity_get_usb_interface_info(interface_id);

                if (interface_status != NULL) {
                    bt_source_srv_audio_sample_rate_t rate_param = {0};
                    rate_param.port = port;
                    rate_param.sample_rate = interface_status->sample_rate;
                    APPS_LOG_MSGID_W(LOG_TAG" USB event, SAMPLE_RATE port=%d rate : %d",
                                        2, port, rate_param.sample_rate);
                    success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_AUDIO_SAMPLE_RATE, &rate_param, sizeof(rate_param));
                } else {
                    APPS_LOG_MSGID_E(LOG_TAG" USB event, SAMPLE_RATE, interface is NULL", 0);
                }
#if 0
                bt_source_srv_port_t port = app_bt_source_covert_port(usb_rate->port.port_type, usb_rate->port.port_num);
                uint32_t rate = app_bt_source_covert_sample_rate(usb_rate->rate);
                APPS_LOG_MSGID_W(LOG_TAG" USB event, SAMPLE_RATE port=%d rate=%d->%d",
                                 3, port, usb_rate->rate, rate);
                bt_source_srv_audio_sample_rate_t rate_param = {0};
                rate_param.port = port;
                rate_param.sample_rate = rate;
                success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_AUDIO_SAMPLE_RATE, &rate_param, sizeof(rate_param));
#endif
            }
            break;
        }
/*
        case APPS_EVENTS_USB_AUDIO_SAMPLE_SIZE: {
            app_events_usb_sample_size_t *usb_size = (app_events_usb_sample_size_t *)&usb_data;
            if (usb_size != NULL) {
                bt_source_srv_port_t port = app_bt_source_covert_port(usb_size->port.port_type, usb_size->port.port_num);
                APPS_LOG_MSGID_W(LOG_TAG" USB event, SAMPLE_SIZE port=%d size=%d",
                                 2, port, usb_size->size);
                bt_source_srv_audio_sample_size_t size = {0};
                size.port = port;
                size.sample_size = usb_size->size;
                success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_AUDIO_SAMPLE_SIZE, &size, sizeof(size));
            }
            break;
        }

        case APPS_EVENTS_USB_AUDIO_CHANNEL: {
            app_events_usb_channel_t *usb_channel = (app_events_usb_channel_t *)&usb_data;
            if (usb_channel != NULL) {
                bt_source_srv_port_t port = app_bt_source_covert_port(usb_channel->port.port_type, usb_channel->port.port_num);
                APPS_LOG_MSGID_W(LOG_TAG" USB event, CHANNEL port=%d channel=%d", 2, port, usb_channel->channel);
                bt_source_srv_audio_sample_channel_t channel = {0};
                channel.port = port;
                channel.channel = usb_channel->channel;
                success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_AUDIO_SAMPLE_CHANNEL, &channel, sizeof(channel));
            }
            break;
        }
*/
        case APPS_EVENTS_USB_AUDIO_SUSPEND: {
            APPS_LOG_MSGID_W(LOG_TAG" USB event, SUSPEND", 0);
            success = TRUE;
            break;
        }

        case APPS_EVENTS_USB_AUDIO_RESUME: {
            APPS_LOG_MSGID_W(LOG_TAG" USB event, RESUME", 0);
            success = TRUE;
            break;
        }

        case APPS_EVENTS_USB_AUDIO_RESET: {
            APPS_LOG_MSGID_W(LOG_TAG" USB event, RESET", 0);
            app_bt_source_handle_usb_reset();
            success = TRUE;
            break;
        }

        default:
            success = TRUE;
            break;
    }

    if (!success) {
        APPS_LOG_MSGID_E(LOG_TAG" USB event, event_id=%d error", 1, event_id);
    }
    return FALSE;
}

#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
static void app_bt_source_set_afe_in_default_volume(bt_source_srv_port_t port)
{
    bt_source_srv_volume_change_t volume_change = {0};
    volume_change.port = port;
    volume_change.volume_value = bt_sink_srv_ami_get_lineIN_default_volume_level();
    volume_change.is_local = BT_SOURCE_SRV_SET_VOLUME_TO_REMOTE;
    bool success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_VOLUME_CHANGE,
                                            (void*)&volume_change,
                                            sizeof(bt_source_srv_volume_change_t));

    APPS_LOG_MSGID_I(LOG_TAG" AFE In set default volume, port : 0x%02x, volume : %d, result : %d",
                        3,
                        port,
                        volume_change.volume_value,
                        success);
}
#endif /* AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE || AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE */

#ifdef AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
bool app_bt_source_activity_proc_i2s_in_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    if (extra_data != NULL) {
        if (event_id == APPS_EVENTS_I2S_IN_STATUS_CHANGE) {
            app_i2s_in_det_t *i2s_in_param = (app_i2s_in_det_t *)extra_data;
            bool i2s_in = (i2s_in_param->i2s_state == 1);
            bt_source_srv_port_t i2s_port = BT_SOURCE_SRV_PORT_NONE;

            if (i2s_in_param->i2s_port == APP_I2S_IN_VOL_PORT_0) {
                i2s_port = BT_SOURCE_SRV_PORT_I2S_IN;
            } else if (i2s_in_param->i2s_port == APP_I2S_IN_VOL_PORT_1) {
                i2s_port = BT_SOURCE_SRV_PORT_I2S_IN_1;
            } else {
                return false;
            }

            if ((i2s_in == false)
                && (app_bt_source_activity_is_port_ready_to_disable(i2s_port) == false)) {
                APPS_LOG_MSGID_W(LOG_TAG" proc_i2s_in_group, I2S is not ready to disable, port : %d, i2s enable status : %d %d",
                                    3,
                                    i2s_in_param->i2s_port,
                                    app_bt_source_ctx.i2s_0_in_port_enable,
                                    app_bt_source_ctx.i2s_1_in_port_enable);
                return false;
            }

            if (i2s_in_param->i2s_port == APP_I2S_IN_VOL_PORT_0) {
                app_bt_source_ctx.i2s_0_in_port_enable = i2s_in;
            } else {
                app_bt_source_ctx.i2s_1_in_port_enable = i2s_in;
            }

#if 0
    //        if (app_bt_source_conn_mgr_is_disabled()) {
    //            APPS_LOG_MSGID_E(LOG_TAG" I2S_IN event, i2s_state=%d but BT Source disabled",
    //                             1, i2s_in_param->i2s_state);
    //            return FALSE;
    //        }

            app_bt_source_music_start_streaming(i2s_in, FALSE);

            bt_source_srv_audio_port_t port_param = {0};
            port_param.port = BT_SOURCE_SRV_PORT_I2S_IN;
            port_param.state = (i2s_in ? BT_SOURCE_SRV_AUDIO_PORT_STATE_ENABLE : BT_SOURCE_SRV_AUDIO_PORT_STATE_DISABLE);
            bool success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_AUDIO_PORT, &port_param, sizeof(port_param));
            APPS_LOG_MSGID_W(LOG_TAG" I2S_IN event, i2s_state=%d success=%d", 2, i2s_in_param->i2s_state, success);
#endif
            if (i2s_port != BT_SOURCE_SRV_PORT_NONE) {
                app_bt_source_handle_audio_port_change(i2s_port, i2s_in);
                app_bt_source_set_afe_in_default_volume(i2s_port);
            }
        } else if (event_id == APPS_EVENTS_I2S_IN_VOLUME_CHANGE) {
            app_bt_source_handle_i2s_in_volume(extra_data);
        }
    }
    return FALSE;
}
#endif /* AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE */

#ifdef AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE
bool app_bt_source_activity_proc_line_in_group(uint32_t event_id, void *extra_data, size_t data_len)
{

    if (event_id == APPS_EVENTS_INTERACTION_LINE_IN_STATUS) {
        bool line_in = (bool)extra_data;

        if ((line_in == false)
            && (app_bt_source_activity_is_port_ready_to_disable(BT_SOURCE_SRV_PORT_LINE_IN) == false)) {
            APPS_LOG_MSGID_W(LOG_TAG" proc_line_in_group, Line-in is not ready to disable", 0);
            return false;
        }

        app_bt_source_ctx.line_in_port_enable = line_in;

        app_bt_source_handle_audio_port_change(BT_SOURCE_SRV_PORT_LINE_IN, line_in);

        app_bt_source_set_afe_in_default_volume(BT_SOURCE_SRV_PORT_LINE_IN);
    } else if (event_id == APPS_EVENTS_INTERACTION_LINE_IN_VOLUME) {
        app_bt_source_handle_line_in_volume(extra_data);
    }

    return FALSE;
}
#endif /* AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE */

/**================================================================================*/
/**                             BT Source Service Callback                         */
/**================================================================================*/
void bt_source_srv_event_callback(bt_source_srv_event_t event_id, void *parameter, uint32_t length)
{
    APPS_LOG_MSGID_I(LOG_TAG" bt_source_srv callback, event_id=0x%08X parameter=0x%08X length=%d",
                     3, event_id, parameter, length);

    if ((BT_SOURCE_SRV_EVENT_PROFILE_CONNECTED == event_id) || (BT_SOURCE_SRV_EVENT_PROFILE_DISCONNECTED == event_id)) {
        extern bt_status_t app_dongle_session_manager_handle_edr_session_type(bt_source_srv_event_t event,void *parameter);
        app_dongle_session_manager_handle_edr_session_type(event_id, parameter);
        return;
    }

    void *extra_data = NULL;
    if (parameter != NULL && length > 0) {
        extra_data = pvPortMalloc(length);
        if (extra_data != NULL) {
            memcpy(extra_data, parameter, length);
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" bt_source_srv callback, malloc fail", 0);
            return;
        }
    }

    ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_BT_SOURCE, event_id,
                        extra_data, length, NULL, 0);
}



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
void app_bt_source_enable(bool enable)
{
    APPS_LOG_MSGID_I(LOG_TAG" enable, enable=%d", 1, enable);
    if (enable) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_ENABLE);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                            EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_ENABLE,
                            NULL, 0, NULL, 0);
    } else {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_DISABLE);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                            EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_DISABLE,
                            NULL, 0, NULL, 0);
    }
}

bool app_bt_source_activity_proc(struct _ui_shell_activity *self,
                                 uint32_t event_group,
                                 uint32_t event_id,
                                 void *extra_data,
                                 size_t data_len)
{
    bool ret = FALSE;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            ret = app_bt_source_activity_proc_ui_shell_group(event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_SOURCE: {
            ret = app_bt_source_activity_proc_bt_source_group(event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_SOURCE_APP: {
            ret = app_bt_source_activity_proc_bt_source_app_group(event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_USB_AUDIO: {
            ret = app_bt_source_activity_proc_usb_group(event_id, extra_data, data_len);
            break;
        }
#ifdef AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
        case EVENT_GROUP_UI_SHELL_I2S_IN: {
            ret = app_bt_source_activity_proc_i2s_in_group(event_id, extra_data, data_len);
            break;
        }
#endif
#ifdef AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE
        case EVENT_GROUP_UI_SHELL_LINE_IN: {
            ret = app_bt_source_activity_proc_line_in_group(event_id, extra_data, data_len);
            break;
        }
#endif
        default:
            break;
    }

    app_bt_source_conn_mgr_proc_ui_shell_event(event_group, event_id, extra_data, data_len);
    app_bt_source_music_proc_ui_shell_event(event_group, event_id, extra_data, data_len);
    app_bt_source_call_proc_ui_shell_event(event_group, event_id, extra_data, data_len);

    return ret;
}

#endif /* AIR_BT_SOURCE_ENABLE */
