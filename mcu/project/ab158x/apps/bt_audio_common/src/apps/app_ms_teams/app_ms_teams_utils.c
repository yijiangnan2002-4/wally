
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
 * File: app_ms_teams_utils.c
 *
 * Description:
 * This file is used to provide the common function for ms_teams
 *
 */

#include "app_ms_teams_utils.h"
#include "ui_shell_activity.h"
#include "ui_shell_manager.h"
#ifdef MTK_RACE_CMD_ENABLE
#include "race_cmd.h"
#include "race_cmd_relay_cmd.h"
#include "race_noti.h"
#include "race_bt.h"
#endif
#include "app_ms_teams_idle_activity.h"
#include "ms_teams_sys_mem.h"
#include <string.h>

#define TAG "[MS TEAMS] utils "

static app_teams_telemetry_info s_telemetry_info;
extern bool app_ms_teams_connected(void);

typedef struct {
    uint8_t key;
    uint8_t val;
} ms_teams_telemetry_key_value_uint8;
typedef struct {
    uint8_t key;
    uint16_t val;
} ms_teams_telemetry_key_value_uint16;
typedef struct {
    uint8_t key;
    uint32_t val;
} ms_teams_telemetry_key_value_uint32;

#define MAX_TELEMETRY_LIST_INT8_MAX_NUMS 16
static ms_teams_telemetry_key_value_uint8 s_telemetry_dt_list[MAX_TELEMETRY_LIST_INT8_MAX_NUMS] = {{0}};
#define MAX_TELEMETRY_LIST_INT16_MAX_NUMS 8
static ms_teams_telemetry_key_value_uint16 s_telemetry_dt_list1[MAX_TELEMETRY_LIST_INT16_MAX_NUMS] = {{0}};
#define MAX_TELEMETRY_LIST_INT32_MAX_NUMS 2
static ms_teams_telemetry_key_value_uint32 s_telemetry_dt_list2[MAX_TELEMETRY_LIST_INT32_MAX_NUMS] = {{0}};

extern void ms_teams_send_telemetry_update_notify_to_active_host(void);
void ms_teams_telemetry_report_uint8_value(uint8_t key, uint8_t value, bool sync)
{
    if (!app_ms_teams_connected()) {
        return;
    }
    uint32_t idx = 0;
    for (idx = 0; idx < MAX_TELEMETRY_LIST_INT8_MAX_NUMS; idx++) {
        if (s_telemetry_dt_list[idx].key == 0 || s_telemetry_dt_list[idx].key == key) {
            s_telemetry_dt_list[idx].val = value;
            s_telemetry_dt_list[idx].key = key;
            break;
        }
    }

    if (idx == MAX_TELEMETRY_LIST_INT8_MAX_NUMS) {
        APPS_LOG_MSGID_E(TAG"no enough memory for uint8 report.", 0);
    }

    if (sync) {
        ms_teams_send_telemetry_update_notify_to_active_host();
    }
}

void ms_teams_telemetry_report_uint16_value(uint8_t key, uint16_t value, bool sync)
{
    if (!app_ms_teams_connected()) {
        return;
    }
    uint32_t idx = 0;
    for (idx = 0; idx < MAX_TELEMETRY_LIST_INT16_MAX_NUMS; idx++) {
        if (s_telemetry_dt_list1[idx].key == 0 || s_telemetry_dt_list1[idx].key == key) {
            s_telemetry_dt_list1[idx].key = key;
            s_telemetry_dt_list1[idx].val = value;
            break;
        }
    }

    if (idx == MAX_TELEMETRY_LIST_INT16_MAX_NUMS) {
        APPS_LOG_MSGID_E(TAG"no enough memory for uint16 report.", 0);
    }

    if (sync) {
        ms_teams_send_telemetry_update_notify_to_active_host();;
    }
}

#if 0
void ms_teams_telemetry_report_initial_value()
{
    ms_teams_btn_press_type_t type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
    ms_teams_error_code_t ret = ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
    type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
    ret = ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_RELEASE, &type, sizeof(ms_teams_btn_press_type_t));
}
#endif

void ms_teams_telemetry_report_uint32_value(uint8_t key, uint32_t value, bool sync)
{
    if (!app_ms_teams_connected()) {
        return;
    }
    uint32_t idx = 0;
    for (idx = 0; idx < MAX_TELEMETRY_LIST_INT32_MAX_NUMS; idx++) {
        if (s_telemetry_dt_list2[idx].key == 0 || s_telemetry_dt_list2[idx].key == key) {
            s_telemetry_dt_list2[idx].key = key;
            s_telemetry_dt_list2[idx].val = value;
            break;
        }
    }

    if (idx == MAX_TELEMETRY_LIST_INT32_MAX_NUMS) {
        APPS_LOG_MSGID_E(TAG"no enough memory for uint32 report.", 0);
    }

    if (sync) {
        ms_teams_send_telemetry_update_notify_to_active_host();;
    }
}

void ms_teams_update_product_info(uint8_t *info, uint32_t total_len)
{
    static uint8_t *product_info = NULL;
    if (product_info != NULL) {
        ms_teams_free(product_info);
        product_info = NULL;
    }
    product_info = ms_teams_malloc(total_len);
    if (product_info == NULL) {
        return;
    }
    ms_teams_memcpy(product_info, info, total_len);
    s_telemetry_info.endpoint_fw = product_info;
    s_telemetry_info.endpoint_mode_id = &product_info[strnlen((const char *)s_telemetry_info.endpoint_fw, total_len) + 1];
    s_telemetry_info.endpoint_sn = &product_info[strnlen((const char *)s_telemetry_info.endpoint_mode_id, total_len) + strnlen((const char *)s_telemetry_info.endpoint_fw, total_len) + 2];
}

static void ms_teams_event_handler(ms_teams_event_t ev, uint32_t sub_event, uint8_t *data, uint32_t data_len)
{
    uint32_t shell_event = 0;
    uint8_t *data_copy = NULL;
    ui_shell_status_t ui_ret = UI_SHELL_STATUS_OK;

    APPS_LOG_MSGID_I(TAG"teams event callback: 0x%x, 0x%x, 0x%x, 0x%x.", 4, ev, sub_event, data, data_len);
#ifdef MTK_AWS_MCE_ENABLE
    /* TODO: send event and data to partner. */
#endif
    shell_event = ((ev << 16) & 0xFFFF0000) | (sub_event & 0xFFFF);
    if (data != NULL && data_len != 0) {
        data_copy = (uint8_t *)pvPortMalloc(data_len);
        if (data_copy == NULL) {
            return;
        }
        memcpy(data_copy, data, data_len);
    } else {
        data_len = 0;
    }

    ui_ret = ui_shell_send_event(true, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_MS_TEAMS,
                                 shell_event,
                                 data_copy,
                                 data_len,
                                 NULL, 0);
    if (ui_ret != UI_SHELL_STATUS_OK && data_len != 0) {
        APPS_LOG_MSGID_E(TAG"teams event handler send event fail.", 0);
        vPortFree(data_copy);
    }
}

/* TODO, update every telemetry item */
static void ms_teams_telemetry_handler(ms_teams_telemetry_type_t *req_list, uint32_t req_len, uint8_t **data, uint32_t *data_len)
{
    static uint8_t invalid_str = 0x0;
    static uint8_t telemetry_rsp_buf[20] = {0};
    uint32_t rsp_len = 0;
    for (uint32_t idx = 0; idx < req_len; idx++) {
        APPS_LOG_MSGID_I(TAG"teams telemetry callback type=0x%x.", 1, req_list[idx]);
        if (idx > 0) {
            /* Teams core will fill the telemetry for the first request by default, the usr should fill the follow request in multiple request. */
            telemetry_rsp_buf[rsp_len] = req_list[idx];
            rsp_len += 1;
        }
        switch (req_list[idx]) {
            case MS_TEAMS_TELEMETRY_ENDPOINT_FW:
                if (s_telemetry_info.endpoint_fw != NULL) {
                    *data = s_telemetry_info.endpoint_fw;
                    *data_len = strnlen((const char *)s_telemetry_info.endpoint_fw, 32);
                } else {
                    *data = &invalid_str;
                    *data_len = 1;
                }
                return;
            case MS_TEAMS_TELEMETRY_FW:
                if (s_telemetry_info.base_fw != NULL) {
                    *data = s_telemetry_info.base_fw;
                    *data_len = strnlen((const char *)s_telemetry_info.base_fw, 32);
                } else {
                    *data = &invalid_str;
                    *data_len = 1;
                }
                return;
            case MS_TEAMS_TELEMETRY_DON_TO_ANS_SETTING:
                telemetry_rsp_buf[rsp_len] = s_telemetry_info.don_to_setting;
                rsp_len += 1;
                break;
            case MS_TEAMS_TELEMETRY_ENDPOINT_MODEL_ID:
                if (s_telemetry_info.endpoint_mode_id != NULL) {
                    *data = s_telemetry_info.endpoint_mode_id;
                    *data_len = strnlen((const char *)s_telemetry_info.endpoint_mode_id, 32);
                } else {
                    *data = &invalid_str;
                    *data_len = 1;
                }
                return;
            case MS_TEAMS_TELEMETRY_ENDPOINT_SN:
                if (s_telemetry_info.endpoint_sn != NULL) {
                    *data = s_telemetry_info.endpoint_sn;
                    *data_len = strnlen((const char *)s_telemetry_info.endpoint_sn, 32);
                } else {
                    *data = &invalid_str;
                    *data_len = 1;
                }
                return;
            case MS_TEAMS_TELEMETRY_SN:
                if (s_telemetry_info.base_sn != NULL) {
                    *data = s_telemetry_info.base_sn;
                    *data_len = strnlen((const char *)s_telemetry_info.base_sn, 32);
                } else {
                    *data = &invalid_str;
                    *data_len = 1;
                }
                return;
            case MS_TEAMS_TELEMETRY_SIDETONE_LEVEL: {
                *data = s_telemetry_info.side_tone_level;
                *data_len = strnlen((const char*)s_telemetry_info.side_tone_level, 16);
            }
            return;
            case MS_TEAMS_TELEMETRY_AUDIO_CODEC_USED:
                telemetry_rsp_buf[rsp_len] = s_telemetry_info.audio_codec;
                rsp_len += 1;
                break;
            case MS_TEAMS_TELEMETRY_DSP_EFFECTS_ENABLED: {
                telemetry_rsp_buf[rsp_len + 3] = s_telemetry_info.dsp_effect & 0xFF;
                telemetry_rsp_buf[rsp_len + 2] = (s_telemetry_info.dsp_effect >> 8) & 0xFF;
                telemetry_rsp_buf[rsp_len + 1] = (s_telemetry_info.dsp_effect >> 16) & 0xFF;
                telemetry_rsp_buf[rsp_len + 0] = (s_telemetry_info.dsp_effect >> 24) & 0xFF;
                rsp_len += 4;
            }
            break;
            case MS_TEAMS_TELEMETRY_MUTE_LOCK:
                telemetry_rsp_buf[rsp_len] = s_telemetry_info.mute_lock;
                rsp_len += 1;
                break;
            case MS_TEAMS_TELEMETRY_HEADSET_WORN:
                telemetry_rsp_buf[rsp_len] = s_telemetry_info.headset_worn;
                rsp_len += 1;
                break;
            case MS_TEAMS_TELEMETRY_BATTERY_LEVEL:
                telemetry_rsp_buf[rsp_len] = s_telemetry_info.battery_level;
                rsp_len += 1;
                break;
            case MS_TEAMS_TELEMETRY_DEVICE_READY:
                telemetry_rsp_buf[rsp_len] = s_telemetry_info.device_ready;
                rsp_len += 1;
                break;
            case MS_TEAMS_TELEMETRY_RADIO_LINK_QUALITY:
                telemetry_rsp_buf[rsp_len] = s_telemetry_info.radio_link_quality;
                rsp_len += 1;
                break;
            case MS_TEAMS_TELEMETRY_ERR_MSG:
                break;
            case MS_TEAMS_TELEMETRY_BTN_PRESS_INFO:
                break;
            case MS_TEAMS_TELEMETRY_WIREL_DEVICE_CHANGE:
                telemetry_rsp_buf[rsp_len] = s_telemetry_info.wired_device_changed;
                rsp_len += 1;
                break;
#if 1
            case MS_TEAMS_TELEMETRY_PEOPLE_CNT:
                telemetry_rsp_buf[rsp_len] = 0xff;
                rsp_len += 1;
                break;
#endif
            case MS_TEAMS_TELEMETRY_LOCAL_CONFERENCE_CNT:
                telemetry_rsp_buf[rsp_len + 3] = s_telemetry_info.local_conference_cnt & 0xFF;
                telemetry_rsp_buf[rsp_len + 2] = (s_telemetry_info.local_conference_cnt >> 8) & 0xFF;
                telemetry_rsp_buf[rsp_len + 1] = (s_telemetry_info.local_conference_cnt >> 16) & 0xFF;
                telemetry_rsp_buf[rsp_len + 0] = (s_telemetry_info.local_conference_cnt >> 24) & 0xFF;
                rsp_len += 4;
                break;
#if 1
            case MS_TEAMS_TELEMETRY_DEVICE_MIC_STA:
            case MS_TEAMS_TELEMETRY_DEVICE_SPK_STA:
            case MS_TEAMS_TELEMETRY_DEVICE_AUD_STREAM_STA:
            case MS_TEAMS_TELEMETRY_DEVICE_VIDEO_STREAM_STA:
            case MS_TEAMS_TELEMETRY_AI_MODEL_AND_VERSION:
            case MS_TEAMS_TELEMETRY_DEVICE_RESOURCE_USAGE:
                *data = &invalid_str;
                *data_len = 1;
                return;
#endif
#if 0
            /* DT item only. */
            case MS_TEAMS_TELEMETRY_VOICE_MUTE_ACTIVITY:
                telemetry_rsp_buf[rsp_len] = s_telemetry_info.voice_mute;
                rsp_len += 1;
                break;
#endif
            case MS_TEAMS_TELEMETRY_DUAL_PURPOS_TEAMS_BTN:
                telemetry_rsp_buf[rsp_len] = s_telemetry_info.dual_purpose_btn;
                rsp_len += 1;
                break;
        }
    }

    if (rsp_len != 0) {
        *data = telemetry_rsp_buf;
        *data_len = rsp_len;
    } else if (req_len == 0) {
        APPS_LOG_MSGID_I(TAG"teams telemetry callback, query context.", 0);
        uint32_t idx = 0;
        for (idx = 0; idx < MAX_TELEMETRY_LIST_INT16_MAX_NUMS; idx++) {
            if (s_telemetry_dt_list1[idx].key != 0) {
                telemetry_rsp_buf[0] = s_telemetry_dt_list1[idx].key;
                s_telemetry_dt_list1[idx].key = 0;
                telemetry_rsp_buf[1] = (s_telemetry_dt_list1[idx].val >> 8) & 0xFF;
                telemetry_rsp_buf[2] = (s_telemetry_dt_list1[idx].val) & 0xFF;
                *data = telemetry_rsp_buf;
                *data_len = 3;
                return;
            }
        }
        uint32_t uint8_nums = 0;
        for (idx = 0; idx < MAX_TELEMETRY_LIST_INT8_MAX_NUMS; idx++) {
            if (s_telemetry_dt_list[idx].key != 0) {
                telemetry_rsp_buf[uint8_nums] = s_telemetry_dt_list[idx].key;
                s_telemetry_dt_list[idx].key = 0;
                telemetry_rsp_buf[uint8_nums + 1] = s_telemetry_dt_list[idx].val;
                uint8_nums += 2;
            }
        }
        if (uint8_nums != 0) {
            *data = telemetry_rsp_buf;
            *data_len = uint8_nums;
            return;
        }
        for (idx = 0; idx < MAX_TELEMETRY_LIST_INT32_MAX_NUMS; idx++) {
            if (s_telemetry_dt_list2[idx].key != 0) {
                telemetry_rsp_buf[0] = s_telemetry_dt_list2[idx].key;
                s_telemetry_dt_list2[idx].key = 0;
                telemetry_rsp_buf[4] = s_telemetry_dt_list2[idx].val & 0xff;
                telemetry_rsp_buf[3] = (s_telemetry_dt_list2[idx].val >> 8) & 0xff;
                telemetry_rsp_buf[2] = (s_telemetry_dt_list2[idx].val >> 16) & 0xff;
                telemetry_rsp_buf[1] = (s_telemetry_dt_list2[idx].val >> 24) & 0xff;
                *data = telemetry_rsp_buf;
                *data_len = 5;
                return;
            }
        }

        if (s_telemetry_info.err_msg) {
            telemetry_rsp_buf[0] = MS_TEAMS_TELEMETRY_ERR_MSG;
            uint32_t len = strlen((const char*)s_telemetry_info.err_msg);
            len = len > 19 ? 19 : len;
            memcpy(&telemetry_rsp_buf[1], s_telemetry_info.err_msg, len);
            *data = telemetry_rsp_buf;
            *data_len = len + 1;
        }
    }
}

app_teams_telemetry_info *app_ms_teams_get_telemetry_info()
{
    return &s_telemetry_info;
}

void app_ms_teams_init()
{
    ms_teams_config_t config = {
#ifdef AIR_HEADSET_ENABLE
#ifndef ULL_DONGLE
        MS_TEAMS_BT_SRV | MS_TEAMS_USB_SRV,
#else
        MS_TEAMS_USB_PROXY
#endif
        ms_teams_event_handler,
        ms_teams_telemetry_handler,
        0x02
#else
#ifndef ULL_DONGLE
        MS_TEAMS_BT_SRV,
#else
        MS_TEAMS_USB_PROXY
#endif
        ms_teams_event_handler,
        ms_teams_telemetry_handler,
        0x01
#endif
    };
    ms_teams_init(&config);
}

bool app_ms_teams_is_dongle_connected()
{
#ifdef AIR_LE_AUDIO_ENABLE
    return true;
#else
    return false;
#endif
}

