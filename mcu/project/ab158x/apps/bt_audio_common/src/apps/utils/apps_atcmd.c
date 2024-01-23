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
 * File: apps_atcmd.c
 *
 * Description: This file is used to create APP log module.
 *
 */

#include "apps_atcmd.h"

#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_events_key_event.h"
#include "atci.h"
#include "ui_shell_manager.h"
#include "FreeRTOS.h"
#include "voice_prompt_api.h"
#include "bt_type.h"
#if defined(APPS_LINE_IN_SUPPORT) || defined(AIR_LINE_IN_MIX_ENABLE) || defined(AIR_LINE_OUT_ENABLE)
#include "app_line_in_idle_activity.h"
#endif
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_lea_service.h"
#include "app_lea_service_conn_mgr.h"
#include "app_le_audio_aird_client.h"
#endif
#include "apps_config_led_manager.h"

#define LOG_TAG           "[APP_ATCMD]"

/**
 * Power_off - AT+APP=Action,5\0d\0a
 * Reboot - AT+APP=Action,7\0d\0a
 * RHO - AT+APP=Action,0\0d\0a
 *
 * KEY_DISCOVERABLE - AT+APP=Key,0002\0d\0a
 * KEY_AVRCP_PLAY - AT+APP=Key,0053\0d\0a
 * KEY_AVRCP_PAUSE - AT+APP=Key,0055\0d\0a
 * KEY_AVRCP_FORWARD - AT+APP=Key,005A\0d\0a
 * KEY_AVRCP_BACKWARD - AT+APP=Key,005B\0d\0a
 * KEY_VOICE_UP - AT+APP=Key,000A\0d\0a
 * KEY_VOICE_DN - AT+APP=Key,000B\0d\0a
 * KEY_AIR_PAIRING - AT+APP=Key,0097\0d\0a
 * KEY_SWITCH_ANC_AND_PASSTHROUGH - AT+APP=Key,0092\0d\0a
 *
 * HW KEY Data/Event - AT+APP=HW_KEY,18,01\0d\0a
 * DEVICE_KEY_POWER - 24, 0x18
 * airo_key_event_t - 0x01 short, 0x02 double, 0x03 triple, 0x21 LONG_PRESS_1
 */

static void app_atcmd_action(uint32_t event_group, uint32_t event_id, void *data, size_t data_len)
{
    ui_shell_status_t ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                                event_group, event_id,
                                                data, data_len, NULL, 0);
    APPS_LOG_MSGID_I(LOG_TAG" send, group=%d event=%d(%04X) ret=%d",
                     4, event_group, event_id, event_id, ret);
}

static atci_status_t app_atcmd_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            char *atcmd = parse_cmd->string_ptr + parse_cmd->name_len + 1;
            char cmd[20] = {0};
            memcpy(cmd, atcmd, strlen(atcmd) - 2);
            bool correct_cmd_flag = TRUE;
            APPS_LOG_I(LOG_TAG" at_cmd=%s", cmd);

            uint32_t event_group = 0;
            uint32_t event_id = 0;
            void *data = NULL;
            size_t data_len = 0;
            int cmd_id = 0;
            if (strstr(cmd, "Action") > 0) {
                sscanf(cmd, "Action,%d", &cmd_id);
                event_group = EVENT_GROUP_UI_SHELL_APP_INTERACTION;
                event_id = (uint32_t)cmd_id;
            } else if (strstr(cmd, "Key") > 0) {
                // Send key action directly
                sscanf(cmd, "Key,%04x", &cmd_id);
                event_group = EVENT_GROUP_UI_SHELL_KEY;
                event_id = INVALID_KEY_EVENT_ID;
                uint16_t *key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
                if (key_action != NULL) {
                    *key_action = (uint16_t)cmd_id;
                    data = (void *)key_action;
                    data_len = sizeof(uint16_t);
                }
            } else if (strstr(cmd, "HW_KEY") > 0) {
                // Only send HW key event
                uint32_t key_data = 0;
                uint32_t key_event = 0;
                sscanf(cmd, "HW_KEY,%02x,%02x", (unsigned int *)&key_data, (unsigned int *)&key_event);
                event_group = EVENT_GROUP_UI_SHELL_KEY;
                event_id = (key_data & 0xFF) | ((key_event & 0xFF) << 8);
                uint16_t *key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
                if (key_action != NULL) {
                    *key_action = FALSE;
                    data = (void *)key_action;
                    data_len = sizeof(uint16_t);
                }
            }
#if 0
            else if (strstr(cmd, "VP") > 0) {
                if (strstr(cmd, "CLEAR") > 0) {
                    voice_prompt_clear_all();
                } else {
                    uint32_t vp_index;
                    uint32_t sync;
                    uint32_t loop;
                    if (sscanf(cmd, "VP,%d,%d,%d", (unsigned int *)&vp_index, (unsigned int *)&sync, (unsigned int *)&loop) == 3) {
                        voice_prompt_param_t vp = {0};
                        vp.vp_index = vp_index;
                        if (sync) {
                            vp.control |= VOICE_PROMPT_CONTROL_MASK_SYNC;
                        }
                        if (loop) {
                            vp.control |= VOICE_PROMPT_CONTROL_MASK_LOOP;
                        }
                        voice_prompt_play(&vp, NULL);
                    } else {
                        //APPS_LOG_MSGID_I(LOG_TAG" VP cmd format error", 0);
                        correct_cmd_flag = FALSE;
                    }
                }
            }
#endif
#ifdef AIR_LE_AUDIO_ENABLE
            else if (strstr(cmd, "SWITCH") > 0) {
                sscanf(cmd, "SWITCH,%d", &cmd_id);
                bt_handle_t handle = app_lea_conn_mgr_get_handle(cmd_id);
                APPS_LOG_MSGID_I(LOG_TAG"[LEA] switch_device, cmd_id=%d handle=0x%04X", 2, cmd_id, handle);
                app_le_audio_aird_client_switch_device(handle, 1);
            }
            else if (strstr(cmd, "ADD_BOND") > 0) {
                int addr_id = 0;
                int type = 0;
                sscanf(cmd, "ADD_BOND,%d,%d", &addr_id, &type);
                uint8_t addr[BT_BD_ADDR_LEN] = {0};
                memset(addr, (addr_id & 0xFF), BT_BD_ADDR_LEN);
                APPS_LOG_MSGID_I(LOG_TAG"[LEA] ADD_BOND, addr_id=%02X type=%d", 2, addr_id, type);
                app_lea_conn_mgr_add_bond_info(BT_ADDR_PUBLIC, addr, APP_LEA_CONN_TYPE_LE_AUDIO);
                extern void app_lea_conn_mgr_update_reconnect_type(const uint8_t *bd_addr, uint8_t reconnect_type);
                app_lea_conn_mgr_update_reconnect_type(addr, type);
                app_lea_service_refresh_advertising(1000);
            }
            else if (strstr(cmd, "Disconnect") > 0) {
                int reason = 0;
                sscanf(cmd, "Disconnect,%02X", &reason);
                APPS_LOG_MSGID_I(LOG_TAG"[LEA] Disconnect, reason=0x%02X", 1, reason);
                app_lea_service_disconnect(FALSE, APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT_LEA,
                                           NULL, reason);
            }
#endif
#if defined(APPS_LINE_IN_SUPPORT) || defined(AIR_LINE_IN_MIX_ENABLE) || defined(AIR_LINE_OUT_ENABLE)
            else if (strstr(cmd, "GET_LINE_IN") > 0) {
                bool line_in = app_line_in_is_plug_in();
                if (line_in) {
                    atcmd = "LINE_IN - ON";
                } else {
                    atcmd = "LINE_IN - OFF";
                }
            }
#endif
#if LED_ENABLE
            else if (strstr(cmd, "BG_LED") > 0) {
                sscanf(cmd, "BG_LED,%d", &cmd_id);
                apps_config_set_background_led_pattern(cmd_id, false, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOWEST);
            } else if (strstr(cmd, "FG_LED") > 0) {
                sscanf(cmd, "FG_LED,%d", &cmd_id);
                apps_config_set_foreground_led_pattern(cmd_id, 30, false);
            }
#endif
            else {
                //APPS_LOG_MSGID_I(LOG_TAG" invalid APP ATCMD", 0);
                correct_cmd_flag = FALSE;
            }

            memset(response.response_buf, 0, ATCI_UART_TX_FIFO_BUFFER_SIZE);
            response.response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
            if (correct_cmd_flag) {
                snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "OK - %s", atcmd);
                app_atcmd_action(event_group, event_id, data, data_len);
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

static atci_cmd_hdlr_item_t app_atci_cmd[] = {
    {
        .command_head = "AT+APP",
        .command_hdlr = app_atcmd_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

void app_atcmd_init()
{
    atci_register_handler(app_atci_cmd, sizeof(app_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
    //APPS_LOG_MSGID_I(LOG_TAG" init, ret=%d", 1, ret);
}
