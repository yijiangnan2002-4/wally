
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

#include <assert.h>
#include "app_ms_teams_telemetry.h"
#include "app_ms_teams_utils.h"
#ifdef MTK_FOTA_ENABLE
#include "fota_util.h"
#endif
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "ms_teams_porting.h"
#include "ms_teams_sys_mem.h"
#include "apps_customer_config.h"

#define TAG "[MS TEAMS] idle_activity "

static void app_get_version(uint8_t *ver, uint32_t *len)
{
    if (ver == NULL || len == NULL) {
        return;
    }
    memset(ver, 0, *len);
#ifdef MTK_FOTA_ENABLE
    FOTA_ERRCODE status = fota_version_get(ver, *len, FOTA_VERSION_TYPE_STORED);
    if (status != FOTA_ERRCODE_SUCCESS) {
        assert(0 && "read fw version fail");
    }
    *len = strnlen((const char*)ver, *len);
#else
    assert(0 && "AIROHA FOTA NOT SUPPORTED, UPDATE THIS FUNCTION");
#endif
}

static void app_get_device_sn(uint8_t *sn, uint32_t *len)
{
    if (sn == NULL || len == NULL) {
        return;
    }
    uint32_t buffer_len_temp = *len;
    memset(sn, 0, buffer_len_temp);
    nvkey_status_t status = nvkey_read_data(NVID_TEAMS_DEVICE_SN, sn, len);
    if (status != NVKEY_STATUS_OK) {
        APPS_LOG_MSGID_I(TAG"read fw SN fail, load_default_value", 0);
        memcpy(sn, "SN0001", sizeof("SN0001"));
    }
    *len = strnlen((const char *)sn, buffer_len_temp);
}

#if !defined(AIR_DONGLE_ENABLE)
static void app_get_model_id(uint8_t *sn, uint32_t *len)
{
    if (sn == NULL || len == NULL) {
        return;
    }

    uint8_t temp_str[40] = {0};
    uint32_t read_len = 40;
    nvkey_status_t status = nvkey_read_data(NVID_MODEL_NAME, temp_str, &read_len);
    if (status != NVKEY_STATUS_OK) {
        assert(0 && "read mode name fail");
    }
    *len = strnlen((const char *)&temp_str[20], 20);
    if (*len > 16) {
        assert(0 && "read mode name fail, length invalid");
    }
    memcpy(sn, temp_str, *len);
}
#endif

void app_ms_teams_load_default_telemetry_setting()
{
    uint8_t temp_string[32];
    uint32_t string_len = 32; // buffer size is 32

    ms_teams_memset(app_ms_teams_get_telemetry_info(), 0, sizeof(app_teams_telemetry_info));
    /* Set default endpoint fw version, but this item should be set again while endpoint connected. */
    app_ms_teams_set_endpoint_fw_version((uint8_t*)"v0.0.0", strlen("v0.0.0"));

    /* Load version from SDK common solution. */
    app_get_version(&temp_string[0], &string_len);
    app_ms_teams_set_dongle_fw_version(temp_string, string_len);

    /* Always set it to false */
    app_ms_teams_set_don_to_answer_setting(MS_TEAMS_SUPPORT_DON_TO_ANSWER);

    string_len = 32;
#ifndef AIR_DONGLE_ENABLE
    app_get_model_id(temp_string, &string_len);
    app_ms_teams_set_endpoint_device_model_id(temp_string, string_len);
#else
    /* Set default endpoint model id, but this item should be set again while endpoint connected. */
    app_ms_teams_set_endpoint_device_model_id((uint8_t*)"Airoha_headset", strlen("Airoha_headset"));
#endif

    /* Load SB from SDK common solution. */
    string_len = 32;
    app_get_device_sn(&temp_string[0], &string_len);
    app_ms_teams_set_dongle_device_SN(temp_string, string_len);

    /* Set default endpoint model id, but this item should be set again while endpoint connected. */
    app_ms_teams_set_endpoint_device_SN((uint8_t*)"SN0000", strlen("SN0000"));

    /* Default set to "0.0", if your sidetone support level modification, update the diff db while it changed. */
    app_ms_teams_set_sidetone_level(MS_TEAMS_DEFAULT_SIDETONE_LEVEL_DIFF);

    /* Default set to Narrowband. */
    app_ms_teams_set_audio_codec_used(MS_TEAMS_DEFAULT_AUDIO_CODEC_USED);

    /* Default value set to 0x17 */
    app_ms_teams_set_dsp_effect_mask(MS_TEAMS_DEFAULT_DSP_EFFECT);

    /* follow items are only device triggered item, call follow action while the status changed. */
    //app_ms_teams_set_hardmute_lock(on_off);
    //app_ms_teams_set_headset_worn(on_off);

    /* Default set to battery off, but this item should be set again while endpoint connected or battery updated */
    app_ms_teams_set_battery_level(APP_MS_TEAMS_TELEMETRY_BATTERY_OFF);

    /* Default set to false, but this item should be must to true while endpoint connected */
    app_ms_teams_set_device_ready(false);

    /* Default set to off status, but this item should be set while endpoint connected */
    app_ms_teams_set_link_quality(APP_MS_TEAMS_TELEMETRY_LINK_QUALITY_OFF);

    /* follow items are only device triggered item, call follow action while the status changed. */
    //app_ms_teams_set_error_message((uint8_t*)"None", strlen("None"));
    //app_ms_teams_set_button_press_info_hook(on_off);
    //app_ms_teams_set_button_press_info_mute(on_off);
    //app_ms_teams_set_button_press_info_flash(on_off);
    //app_ms_teams_set_connected_wireless_device_changed(yes_no);
    //app_ms_teams_set_local_reference_coult(count);

    /* follow item is only device triggered item, call follow action while the status changed. */
    //app_ms_teams_set_voice_activity_while_muted(yes_no);
}

static void app_ms_teams_telemetry_string_check(uint8_t *str, uint32_t len)
{
    if (str == NULL || len == 0 || len > 32) {
        APPS_LOG_MSGID_I(TAG"invalid string input, str=0x%x, len=%d.", 2, str, len);
        assert(0 && "TEAMS TELEMETRY INVALID STRING INPUT");
        return;
    }

    APPS_LOG_DUMP_I(TAG"input string is:", str, len);
    for (uint32_t i = 0; i < len; i++) {
        if ((str[i] >= 'A' && str[i] <= 'Z') ||
            (str[i] >= 'a' && str[i] <= 'z') ||
            (str[i] >= '0' && str[i] <= '9') ||
            (str[i] == '.') ||
            (str[i] == '_') ||
            (str[i] == '-') ||
            (str[i] == ' ')) {
            continue;
        } else {
            APPS_LOG_MSGID_I(TAG"invalid character in pos=%d, value=%d.", 2, i, str[i]);
            assert(0 && "TEAMS TELEMETRY INVALID STRING CHARACTER");
        }
    }
}

static void app_ms_teams_telemetry_copy_string(uint8_t** des, uint8_t *str, uint32_t len)
{
    app_ms_teams_telemetry_string_check(str,len);
    if (*des != NULL) {
        ms_teams_free(*des);
    }
    *des = ms_teams_malloc(len + 1);
    if (*des != NULL) {
        ms_teams_memset(*des, 0, len + 1);
        ms_teams_memcpy(*des, str, len);
    } else {
        assert(0 && "INVALID string copy destination");
    }
}

bool app_ms_teams_set_endpoint_fw_version(uint8_t *ver, uint32_t len)
{
    app_ms_teams_telemetry_copy_string(&app_ms_teams_get_telemetry_info()->endpoint_fw, ver, len);
    return true;
}

bool app_ms_teams_set_dongle_fw_version(uint8_t *ver, uint32_t len)
{
    app_ms_teams_telemetry_copy_string(&app_ms_teams_get_telemetry_info()->base_fw, ver, len);
    return true;
}

bool app_ms_teams_set_don_to_answer_setting(bool en)
{
    app_ms_teams_get_telemetry_info()->don_to_seeting = en;
    return true;
}

bool app_ms_teams_set_endpoint_device_model_id(uint8_t *model_id, uint32_t len)
{
    app_ms_teams_telemetry_copy_string(&(app_ms_teams_get_telemetry_info()->endpoint_mode_id), model_id, len);
    return true;
}

bool app_ms_teams_set_dongle_device_SN(uint8_t *sn, uint32_t len)
{
    app_ms_teams_telemetry_copy_string(&(app_ms_teams_get_telemetry_info()->base_sn), sn, len);
    return true;
}

bool app_ms_teams_set_endpoint_device_SN(uint8_t *sn, uint32_t len)
{
    app_ms_teams_telemetry_copy_string(&(app_ms_teams_get_telemetry_info()->endpoint_sn), sn, len);
    return true;
}

bool app_ms_teams_set_sidetone_level(float32_t db_diff)
{
    if (db_diff < 0) {
        db_diff = 0 - db_diff;
    }
#if defined(AIR_DONGLE_ENABLE) || defined(AIR_HEADSET_ENABLE) || defined(AIR_TWS_ENABLE)
    static uint8_t level[16];
    uint32_t len = snprintf((char*)&level[0], 16, "%.2f", (double)db_diff);
    app_ms_teams_telemetry_string_check(level, len);
    app_ms_teams_telemetry_copy_string(&(app_ms_teams_get_telemetry_info()->side_tone_level), level, len);
#endif
#if !defined(AIR_DONGLE_ENABLE)
    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_EVENT_SIDETONE_LEVEL_SYNC << 16) & 0xFFFF0000,
                                                          &db_diff, sizeof(float32_t));
#endif
    return true;
}

bool app_ms_teams_set_audio_codec_used(app_ms_teams_telemetry_audio_codec_type_t type)
{
    uint8_t codec_type = (uint8_t)type;
#if defined(AIR_DONGLE_ENABLE) || defined(AIR_HEADSET_ENABLE) || defined(AIR_TWS_ENABLE)
    app_ms_teams_get_telemetry_info()->audio_codec = (uint8_t)type;
    ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_AUDIO_CODEC_USED, codec_type, true);
#endif
#if !defined(AIR_DONGLE_ENABLE)
    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_EVENT_AUDIO_CODEC_CHANGED << 16) & 0xFFFF0000,
                                                          &codec_type, sizeof(uint8_t));
#endif
    return true;
}

bool app_ms_teams_set_dsp_effect_mask(uint32_t mask)
{
#if defined(AIR_DONGLE_ENABLE) || defined(AIR_HEADSET_ENABLE) || defined(AIR_TWS_ENABLE)
    app_ms_teams_get_telemetry_info()->dsp_effect = mask;
    ms_teams_telemetry_report_uint32_value(MS_TEAMS_TELEMETRY_DSP_EFFECTS_ENABLED, mask, true);
#endif
#if !defined(AIR_DONGLE_ENABLE)
    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_EVENT_DSP_EFFECT_UPDATE << 16) & 0xFFFF0000,
                                                          &mask, sizeof(uint32_t));
#endif
    return true;
}

bool app_ms_teams_set_hardmute_lock(bool on_off)
{
#if defined(AIR_DONGLE_ENABLE) || defined(AIR_HEADSET_ENABLE) || defined(AIR_TWS_ENABLE)
    ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_MUTE_LOCK, on_off ? 0x01 : 0x00, true);
#endif
#if !defined(AIR_DONGLE_ENABLE)
    uint8_t lock = on_off ? 1 : 0;
    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_EVENT_HARD_MUTE_LOCK_CHANGED << 16) & 0xFFFF0000,
                                                          &lock, sizeof(uint8_t));
#endif
    return true;
}

bool app_ms_teams_set_headset_worn(bool on_off)
{
#if defined(AIR_DONGLE_ENABLE) || defined(AIR_HEADSET_ENABLE) || defined(AIR_TWS_ENABLE)
    ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_HEADSET_WORN, on_off ? 0x01 : 0x00, true);
#endif
#if !defined(AIR_DONGLE_ENABLE)
    uint8_t worn = on_off ? 1 : 0;
    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_EVENT_IN_EAR_STA << 16) & 0xFFFF0000,
                                                          &worn, sizeof(uint8_t));
#endif
    return true;
}

bool app_ms_teams_set_battery_level(app_ms_teams_telemetry_battery_level_t level)
{
    uint8_t battery_level = (uint8_t)level;
#if defined(AIR_DONGLE_ENABLE) || defined(AIR_HEADSET_ENABLE) || defined(AIR_TWS_ENABLE)
    app_ms_teams_get_telemetry_info()->battery_level = battery_level;
    ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_BATTERY_LEVEL, battery_level, true);
#endif
#if !defined(AIR_DONGLE_ENABLE)
    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_EVENT_BATTERY_LEVEL_CHANGED << 16) & 0xFFFF0000,
                                                          &battery_level, sizeof(uint8_t));
#endif
    return true;
}

/**
* @brief      This function will be called every time the headset/earbuds connected or disconnected with dongle.
* @param[in]  yes_no true means connected and false disconnected.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_device_ready(bool yes_no)
{
    app_ms_teams_get_telemetry_info()->device_ready = yes_no;
    ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_DEVICE_READY, yes_no, true);
    return true;
}

bool app_ms_teams_set_link_quality(app_ms_teams_telemetry_link_quality_t quality)
{
    app_ms_teams_get_telemetry_info()->radio_link_quality = quality;
    ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_RADIO_LINK_QUALITY, quality, true);
    return true;
}

bool app_ms_teams_set_error_message(uint8_t *msg, uint32_t len)
{
    app_ms_teams_telemetry_copy_string(&(app_ms_teams_get_telemetry_info()->err_msg), msg, len);
    ms_teams_btn_press_type_t type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
    ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_INVOKE, &type, sizeof(ms_teams_btn_press_type_t));
    type = MS_TEAMS_BTN_PRESS_TYPE_NONE;
    ms_teams_send_action(MS_TEAMS_ACTION_TEAMS_BTN_RELEASE, &type, sizeof(ms_teams_btn_press_type_t));
    return true;
}

static bool app_ms_teams_set_button_press_info(uint8_t type, bool press)
{
    uint16_t data = type << 8 | (press ? 0x01 : 0x00);
#if defined(AIR_DONGLE_ENABLE) || defined(AIR_HEADSET_ENABLE) || defined(AIR_TWS_ENABLE)
    ms_teams_telemetry_report_uint16_value(MS_TEAMS_TELEMETRY_BTN_PRESS_INFO, data, true);
#endif
#if !defined(AIR_DONGLE_ENABLE)
    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_EVENT_BUTTON_PRESS_INFO_CHANGED << 16) & 0xFFFF0000,
                                                          &data, sizeof(uint16_t));
#endif
    return true;
}

bool app_ms_teams_set_button_press_info_hook(bool on_off)
{
    return app_ms_teams_set_button_press_info(0x20, on_off);
}

bool app_ms_teams_set_button_press_info_mute(bool on_off)
{
    return app_ms_teams_set_button_press_info(0x2f, on_off);
}

bool app_ms_teams_set_button_press_info_flash(bool on_off)
{
    return app_ms_teams_set_button_press_info(0x21, on_off);
}

bool app_ms_teams_set_connected_wireless_device_changed(bool yes_no)
{
    ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_WIREL_DEVICE_CHANGE, yes_no, true);
    return true;
}

bool app_ms_teams_set_local_reference_count(uint32_t count)
{
    ms_teams_telemetry_report_uint32_value(MS_TEAMS_TELEMETRY_LOCAL_CONFERENCE_CNT, count, true);
    return true;
}

/**
* @brief      This function will be called everty time the voice detected while you muted microphone during call.
* @param[in]  yes_no true means voice detected and false means not detected.
* @return     Success or Failed, true means Success.
*/
bool app_ms_teams_set_voice_detected(bool yes_no)
{
    uint8_t detect = yes_no ? 0x01 : 0x00;
#if defined(AIR_DONGLE_ENABLE) || defined(AIR_HEADSET_ENABLE)
    ms_teams_telemetry_report_uint8_value(MS_TEAMS_TELEMETRY_VOICE_MUTE_ACTIVITY, detect, true);
#endif
#if !defined(AIR_DONGLE_ENABLE)
    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_MS_TEAMS, (MS_TEAMS_EVENT_VAD_STA << 16) & 0xFFFF0000,
                                                          &detect, sizeof(uint8_t));
#endif
    return true;
}

