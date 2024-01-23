
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
 * File: app_ms_teams_special_client1.c
 *
 * Description:
 * This file is used to provide the common function for special client
 *
 */

#include "app_ms_teams_utils.h"
#include "bt_spp.h"
#include "bt_source_srv.h"
#include "bt_callback_manager.h"
#include "app_ms_teams_idle_activity.h"
#include "app_ms_teams_utils.h"

#define APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_DEVICE_INFO 0x0100
#define APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_FRIEND_NAME 0x0101
#define APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_SIDETONE_LEVEL 0x0200
#define APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_AUDIO_CODEC 0x0202
#define APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_DSP_EFFECT 0x0204
#define APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_MIC_MUTE_STA 0x0206
#define APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_MIC_LOCK_STA 0x0208
#define APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_WEARING_STA 0x0300
#define APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_DON_TO_ANSWER_SET 0x0400
#define APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_BAT_LEVEL 0x0500
typedef uint16_t app_ms_teams_special_client_action;

#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_DEVICE_INFO_RSP 0x0140
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_FRIENDLY_NAME_RSP 0x0141
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_SIDETONE_LEVEL_RSP 0x0240
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_AUDIO_CODEC_RSP 0x0242
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_DSP_EFFECT_RSP 0x0244
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_MIC_MUTE_STA_RSP 0x0246
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_MIC_LOCK_STA_RSP 0x0248
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_WEARING_STA_RSP 0x0340
#define APP_MS_TEAMS_SPECIAL_ClIENT_EVT_DON_TO_ANSWER_SET_RSP 0x0440
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_BAT_LEVEL_RSP 0x0540
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_FRIEND_NAME_CHANGE 0x0182
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_SIDETONE_LEVEL_CHANGE 0x0281
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_AUDIO_CODEC_CHANGE 0x0283
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_DSP_EFFECT_CHANGE 0x0285
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_MIC_MUTE_STA_CHANGE 0x0287
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_MIC_LOCK_STA_CHANGE 0x0289
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_WARING_STA_CHANGE 0x0381
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_DON_TO_ANSWER_SET_CHANGE 0x0481
#define APP_MS_TEAMS_SPECIAL_CLIENT_EVT_BAT_LEVEL_CHANGE 0x0581
typedef uint16_t app_ms_teams_special_client_event;

#define min(a,b) (a > b ? b : a)

#define TAG "[MS TEAMS][SPC]"

#define SPECIAL_CLIENT_SPP_SRV_UUID 0x00,0x00,0x11,0x01,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x80,0x5F,0x9B,0x34,0xFB
static uint32_t s_spp_client_handle = BT_SPP_INVALID_HANDLE;

static void ms_teams_special_client_write_data(uint8_t *buf, uint32_t len)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    if (s_spp_client_handle == BT_SPP_INVALID_HANDLE) {
        APPS_LOG_MSGID_I(TAG"special client not connected = %d.", 0);
        return;
    }

    ret = bt_spp_send(s_spp_client_handle, buf, (uint16_t)len);
    if (ret != BT_STATUS_SUCCESS) {
        APPS_LOG_MSGID_I(TAG"send data to special client fail=%d.", 1, ret);
    }
}

static void app_ms_teams_send_action_to_special_client(app_ms_teams_special_client_action action)
{
    static uint8_t cmd_to_client[] = {0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cmd_to_client[3] = (uint8_t)((action >> 8) & 0xFF); // server id
    cmd_to_client[4] = (uint8_t)(action & 0xFF); // function type

    uint8_t xor = 0x4D;
    for (uint32_t i = 1; i < 7; i++) {
        xor ^= cmd_to_client[i];
    }
    cmd_to_client[7] = xor;

    ms_teams_special_client_write_data(cmd_to_client, sizeof(cmd_to_client));
}

#define MAX_VER_INFO_LEN 64
static void app_ms_teams_event_process(uint8_t *data, uint32_t len)
{
    if (data == NULL) {
        return;
    }

    static uint8_t *s_device_info_bk = NULL;
    app_ms_teams_special_client_event event = (((data[3] << 8) & 0xFF00) | data[4]);
    app_teams_telemetry_info *telemetry_info = app_ms_teams_get_telemetry_info();
    APPS_LOG_MSGID_I(TAG"received notification = 0x%x.", 1, event);
    switch (event) {
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_DEVICE_INFO_RSP: {
            if (s_device_info_bk != NULL) {
                vPortFree(s_device_info_bk);
            }
            if (len < MAX_VER_INFO_LEN) {
                s_device_info_bk = pvPortMalloc(len);
                memcpy(s_device_info_bk, data, len);
            }
            
            break;
        }
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_FRIENDLY_NAME_RSP:
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_FRIEND_NAME_CHANGE: {
            char ver_info[MAX_VER_INFO_LEN];
            if (s_device_info_bk == NULL) {
                APPS_LOG_MSGID_E(TAG"device info is invalid", 0);
                break;
            }
            /* get version */
            uint32_t info_len = snprintf(ver_info, 64, "%d.%d.%d", s_device_info_bk[17], s_device_info_bk[18],
                ((s_device_info_bk[19] << 8) & 0xFF00) | s_device_info_bk[20]);
            if (info_len >= MAX_VER_INFO_LEN) {
                APPS_LOG_MSGID_I(TAG"invalid ver input, len=%d.", 1, info_len);
                break;
            }
            /* get model id */
            uint32_t allowed_model_id_len = min(MAX_VER_INFO_LEN - info_len, data[7]);
            memcpy(&ver_info[info_len], &data[8], allowed_model_id_len);
            info_len += allowed_model_id_len;
            if (info_len >= (MAX_VER_INFO_LEN - 1)) {
                APPS_LOG_MSGID_I(TAG"invalid model_id input, len=%d.", 1, info_len);
                break;
            }
            ver_info[min(info_len, MAX_VER_INFO_LEN - 1)] = 0;
            info_len += 1;
            /* get SN */
            uint32_t allowed_sn_len = min(s_device_info_bk[22], MAX_VER_INFO_LEN - info_len);
            memcpy(&ver_info[info_len], &s_device_info_bk[23], allowed_sn_len);
            info_len += allowed_sn_len;
            if (info_len >= (MAX_VER_INFO_LEN - 1)) {
                APPS_LOG_MSGID_I(TAG"invalid model_id input, len=%d.", 1, info_len);
                break;
            }
            ver_info[min(info_len, MAX_VER_INFO_LEN - 1)] = 0;
            info_len += 1;
            ms_teams_update_product_info((uint8_t*)ver_info, info_len);
            break;
        }
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_SIDETONE_LEVEL_RSP:
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_SIDETONE_LEVEL_CHANGE:
            telemetry_info->side_tone_level = data[8];
            break;
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_AUDIO_CODEC_RSP:
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_AUDIO_CODEC_CHANGE:
            telemetry_info->side_tone_level = data[7];
            ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_AUDIO_CODEC_USED, data[7], true);
            break;
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_DSP_EFFECT_RSP:
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_DSP_EFFECT_CHANGE: {
            uint32_t dsp_effect = (data[7] << 24) | (data[8] << 16) | (data[9] << 8) | data[10];
            telemetry_info->dsp_effect = dsp_effect;
            ms_teams_telemetry_report_uint32_value(MS_TEAMS_TELEMETRY_DSP_EFFECTS_ENABLED, dsp_effect, true);
            break;
        }
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_MIC_MUTE_STA_RSP:
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_MIC_MUTE_STA_CHANGE:
            telemetry_info->voice_mute = data[7];
            ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_DEVICE_MIC_STA, data[7], true);
            break;
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_MIC_LOCK_STA_RSP:
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_MIC_LOCK_STA_CHANGE:
            telemetry_info->mute_lock = data[7];
            ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_MUTE_LOCK, data[7], true);
            break;
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_WEARING_STA_RSP:
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_WARING_STA_CHANGE:
            telemetry_info->headset_worn = data[7];
            ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_HEADSET_WORN, data[7], true);
            break;
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_DON_TO_ANSWER_SET_RSP:
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_DON_TO_ANSWER_SET_CHANGE:
            telemetry_info->don_to_setting = data[7];
            ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_DON_TO_ANS_SETTING, data[7], true);
            break;
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_BAT_LEVEL_RSP:
        case APP_MS_TEAMS_SPECIAL_CLIENT_EVT_BAT_LEVEL_CHANGE:
            telemetry_info->battery_level = data[7];
            ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_BATTERY_LEVEL, data[7], true);
            break;
    }
}

static bt_status_t ms_teams_spp_client_event_handler(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    if (msg == BT_SPP_CONNECT_CNF) {
        bt_spp_connect_cnf_t *con = (bt_spp_connect_cnf_t*) buff;
        if (con->handle == s_spp_client_handle) {
            APPS_LOG_MSGID_I(TAG"connect to special client success.", 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_TEAMS_SPECIAL_CLIENT_CONNECTED, NULL, 0,
                                NULL, 1000);
        }
    } else if (msg == BT_SPP_DATA_RECEIVED_IND) {
        bt_spp_data_received_ind_t *data_ind_p = (bt_spp_data_received_ind_t *)buff;
        uint32_t error = 1;
        if (data_ind_p->handle == s_spp_client_handle) {
            uint8_t *data = data_ind_p->packet;
            uint32_t process_len = 0;
            while (process_len < data_ind_p->packet_length) {
                //process_len = 1;
                /* search SOF(0x4D) */
                if (data[process_len] != 0x4D) {
                    process_len+=1;
                    continue;
                }
                /* get package length */
                if (process_len + 6 > data_ind_p->packet_length) {
                    error = 2;
                    break; /* package is not translate done, drop it. */
                }
                uint32_t len = (data[process_len+5] << 8) + data[process_len+6];
                if (process_len + 7 + len > data_ind_p->packet_length) {
                    error = 3;
                    break; /* package is not translate done, drop it. */
                }
                app_ms_teams_event_process(&data[process_len], data_ind_p->packet_length - process_len);
                process_len += (8 + len);
                error = 0;
            }
            APPS_LOG_MSGID_I(TAG"received data process result=%d.", 1, error);
        }
    }

    return BT_STATUS_SUCCESS;
}

void app_ms_teams_special_client_init()
{
    bt_status_t ret = bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_SPP, (void *)ms_teams_spp_client_event_handler);
    APPS_LOG_MSGID_I(TAG"register bt callback, ret=%d.", 1, ret);
}

void app_ms_teams_connect_to_special_client(bt_bd_addr_t *addr)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    static uint8_t spp_srv_uuid[] = {SPECIAL_CLIENT_SPP_SRV_UUID};

    ret = bt_spp_connect(&(s_spp_client_handle), addr, spp_srv_uuid);
    APPS_LOG_MSGID_I(TAG"connect to special client, ret=%d.", 1, ret);
}

void app_ms_teams_special_client_connect_process()
{
    app_ms_teams_send_action_to_special_client(APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_DEVICE_INFO);
    //app_ms_teams_send_action_to_special_client(APP_MS_TEAMS_SPECIAL_CLIENT_ACT_GET_FRIEND_NAME);
}

const uint8_t *bt_avrcp_get_company_id()
{
    static uint8_t sig_company_id[] = {0x00, 0x04, 0x6d};
    return sig_company_id;
}


