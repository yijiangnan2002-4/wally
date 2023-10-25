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
 * File: app_gsound_device_action.c
 *
 * Description: This file is used to handle the device action.
 */

#ifdef AIR_GSOUND_ENABLE

#include "app_gsound_device_action.h"

#include "app_gsound_battery_ohd.h"
#include "app_gsound_event.h"
#include "gsound_api.h"
#include "gsound_device_action.pb.h"

#include "apps_aws_sync_event.h"
#ifdef MTK_ANC_ENABLE
#include "app_anc_service.h"
#endif
#include "apps_control_touch_key_status.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#include "bt_sink_srv_ami.h"
#endif
#include "pb_encode.h"
#include "pb_decode.h"

#define LOG_TAG "[GS][APP][DEVICE_ACTION]"

#define DEVICE_ACTION_STATE_KEY_BATTERY             "battery_details"
#define DEVICE_ACTION_STATE_KEY_ALL                 "all"

typedef struct {
    uint8_t *volatile key_all;
    uint8_t *volatile key_battery;
} app_gsound_state_key_t;

static volatile app_gsound_state_key_t gsound_device_action_state_key = {NULL, NULL};

#ifdef MTK_ANC_ENABLE
static audio_anc_control_type_t        gsound_anc_cur_type = AUDIO_ANC_CONTROL_ANC_TYPE_DEFAULT;

int32_t app_gsound_device_action_get_anc_pt_level(audio_anc_control_type_t type)
{
    uint8_t anc_enable = 0;
    audio_anc_control_filter_id_t anc_current_filter_id = AUDIO_ANC_CONTROL_FILTER_FLASH_ID_DUMMY;
    audio_anc_control_type_t anc_current_type = AUDIO_ANC_CONTROL_TYPE_DUMMY;
    int16_t anc_runtime_gain = 0;
    uint8_t support_hybrid_enable = 0;
    audio_anc_control_misc_t control_misc = {0};

    audio_anc_control_get_status(&anc_enable, &anc_current_filter_id, &anc_current_type, &anc_runtime_gain, &support_hybrid_enable, &control_misc);
    GSOUND_LOG_I(LOG_TAG" get_anc_pt_level, input_type=%d, enable=%d, filterId=%d, type=%d, gain=%d dB, hybridEnable=%d, sub_state=%d", 7,
                 type, anc_enable, anc_current_filter_id, anc_current_type, anc_runtime_gain, support_hybrid_enable, control_misc.sub_state);

    if (control_misc.sub_state == AUDIO_ANC_CONTROL_SUB_STATE_WAITING_ENABLE ||
        (anc_enable && control_misc.sub_state != AUDIO_ANC_CONTROL_SUB_STATE_WAITING_DISABLE)) {
        return (type == anc_current_type) ? 1 : 0;
    } else {
        return 0;
    }
}

static void app_gsound_device_action_turn_on_anc(void)
{
    uint8_t anc_enable = 0;
    audio_anc_control_filter_id_t anc_current_filter_id = AUDIO_ANC_CONTROL_FILTER_FLASH_ID_DUMMY;
    audio_anc_control_type_t anc_current_type = AUDIO_ANC_CONTROL_TYPE_DUMMY;
    int16_t anc_runtime_gain = 0;
    uint8_t support_hybrid_enable = 0;
    audio_anc_control_get_status(&anc_enable, &anc_current_filter_id, &anc_current_type,
                                 &anc_runtime_gain, &support_hybrid_enable, NULL);

    audio_anc_control_type_t target_anc_type = AUDIO_ANC_CONTROL_TYPE_DUMMY;
    audio_anc_control_filter_id_t target_anc_filter_id = 0;
    int16_t target_anc_runtime_gain = 0;
    if (support_hybrid_enable) {
        target_anc_type = AUDIO_ANC_CONTROL_ANC_TYPE_DEFAULT;
    } else {
        target_anc_type = AUDIO_ANC_CONTROL_TYPE_FF;
    }
    if (anc_current_type < AUDIO_ANC_CONTROL_TYPE_USER_DEFINED) {
        target_anc_filter_id = anc_current_filter_id;
        target_anc_runtime_gain = anc_runtime_gain;
    } else {
        target_anc_filter_id = AUDIO_ANC_CONTROL_ANC_FILTER_DEFAULT;
        target_anc_runtime_gain = AUDIO_ANC_CONTROL_UNASSIGNED_GAIN;
    }

    gsound_anc_cur_type = target_anc_type;
    app_anc_service_enable(target_anc_filter_id,
                           target_anc_type,
                           target_anc_runtime_gain,
                           NULL);
}

static void app_gsound_device_action_turn_off_anc_pt(void)
{
    app_anc_service_disable();
}

static void app_gsound_device_action_turn_on_pt(void)
{
    app_anc_service_enable(AUDIO_ANC_CONTROL_PASS_THRU_FILTER_DEFAULT,
                           AUDIO_ANC_CONTROL_PASS_THRU_TYPE_DEFAULT,
                           AUDIO_ANC_CONTROL_UNASSIGNED_GAIN,
                           NULL);
}
#endif

static int32_t app_gsound_device_action_get_anc_level(void)
{
#ifdef MTK_ANC_ENABLE
    int32_t level = app_gsound_device_action_get_anc_pt_level(gsound_anc_cur_type);
    return level;
#else
    return 0;
#endif
}

static int32_t app_gsound_device_action_get_pt_level(void)
{
#ifdef MTK_ANC_ENABLE
    int32_t level = app_gsound_device_action_get_anc_pt_level(AUDIO_ANC_CONTROL_PASS_THRU_TYPE_DEFAULT);
    return level;
#else
    return 0;
#endif
}

/*
static bool app_gsound_device_action_decode_battery(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    BatteryDetails *bat = (BatteryDetails *)(*arg);
    if (!nano_pb_decode(stream, BatteryDetails_fields, bat)) {
        GSOUND_LOG_TEXT(LOG_TAG" decode_battery, error=%s", PB_GET_ERROR(stream));
        return FALSE;
    }
    *arg = bat + 1;
    return TRUE;
}

static void app_gsound_device_action_decode_check(const uint8_t *device_actions, int device_actions_size)
{
    BatteryDetails bat[3] = {0};
    CurrentDeviceState state_key_all = CurrentDeviceState_init_default;
    pb_istream_t stream = nano_pb_istream_from_buffer(device_actions, device_actions_size);


    state_key_all.battery_details.funcs.decode = app_gsound_device_action_decode_battery;
    state_key_all.battery_details.arg = bat;

    if (!(nano_pb_decode(&stream, CurrentDeviceState_fields, &state_key_all))) {
        GSOUND_LOG_TEXT(LOG_TAG" decode_check, error=%s", PB_GET_ERROR(&stream));
        return;
    }
}
*/

static bool app_gsound_device_action_battery_encode(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    uint8_t i = 0;
    BatteryDetails *bat = (BatteryDetails *)*arg;
#ifdef MTK_AWS_MCE_ENABLE
    for (i = 0; i < 2; i++)
#else
    i = 0;
#endif
    {
        if (!nano_pb_encode_tag_for_field(stream, field)) {
            GSOUND_LOG_TEXT(LOG_TAG" battery_encode, tag error %d %s", i, PB_GET_ERROR(stream));
            return FALSE;
        }
        if (!nano_pb_encode_submessage(stream, BatteryDetails_fields, bat + i)) {
            GSOUND_LOG_TEXT(LOG_TAG" battery_encode, error %d %s", i, PB_GET_ERROR(stream));
            return FALSE;
        }
    }
    return TRUE;
}

#ifdef AIR_GSOUND_HOTWORD_ENABLE
static bool app_gsound_device_action_is_touch_control_enabled(void)
{
    /* Implementation: Return
       TRUE: if touch control is enabled (device can be triggered by button)
       FALSE: if touch control is disabled (device can only be triggered by hotword)
    */
    uint8_t status = apps_get_touch_control_status();
    return (status == 1);
}
#endif

static uint8_t *app_gsound_device_action_pb_encode(uint32_t *buf_len)
{
    uint8_t *buf = NULL;
    pb_ostream_t stream = PB_OSTREAM_SIZING;
    pb_ostream_t stream_dummy = PB_OSTREAM_SIZING;
    CurrentDeviceState state_key = CurrentDeviceState_init_default;

    uint32_t bat_local_percent = app_gsound_battery_get_info(APP_GSOUND_BATTERY_INFO_LOCAL_PERCENT);
#ifdef MTK_AWS_MCE_ENABLE
    bool is_right = (AUDIO_CHANNEL_R == ami_get_audio_channel() ? TRUE : FALSE);
    uint32_t bat_partner_percent = app_gsound_battery_get_info(APP_GSOUND_BATTERY_INFO_PEER_PERCENT);
    bt_aws_mce_srv_link_type_t aws_status = bt_aws_mce_srv_get_link_type();
#endif

    // 1. set value
#ifdef MTK_AWS_MCE_ENABLE
    BatteryDetails bat[2] = {
        {
            (is_right) ? bat_partner_percent : bat_local_percent,
            (is_right) ? (aws_status != BT_AWS_MCE_SRV_LINK_NONE) : TRUE,
            BatteryDetails_DeviceType_LEFT_EAR_BUD
        },
        {
            (is_right) ? bat_local_percent : bat_partner_percent,
            (is_right) ? TRUE : (aws_status != BT_AWS_MCE_SRV_LINK_NONE),
            BatteryDetails_DeviceType_RIGHT_EAR_BUD
        },
        //{89, TRUE, BatteryDetails_DeviceType_EAR_BUD_CASE},
    };
#else
    BatteryDetails bat[1] = {
        {bat_local_percent, TRUE, BatteryDetails_DeviceType_SINGLE_BATTERY_DEVICE},
    };
#endif

    state_key.battery_details.funcs.encode = app_gsound_device_action_battery_encode;
    state_key.battery_details.arg = bat;
    state_key.noise_cancellation_level = app_gsound_device_action_get_anc_level();
    state_key.ambient_mode_level = app_gsound_device_action_get_pt_level();
#ifdef AIR_GSOUND_HOTWORD_ENABLE
    state_key.touch_control_enabled = app_gsound_device_action_is_touch_control_enabled();
#else
    state_key.touch_control_enabled = TRUE;
#endif

    // 2. get encoded length
    if (!nano_pb_encode(&stream_dummy, CurrentDeviceState_fields, &state_key)) {
        GSOUND_LOG_E(LOG_TAG" pb_encode, error", 0);
        return NULL;
    }
    *buf_len = stream_dummy.bytes_written;

    // 3. allocate buffer memory
    buf = pvPortMalloc(*buf_len);
    if (buf == NULL) {
        GSOUND_LOG_E(LOG_TAG" pb_encode, malloc fail buf_len=%d", 1, buf_len);
        return NULL;
    }

    // 4. set buffer to stream and encode
    stream = nano_pb_ostream_from_buffer(buf, *buf_len);
    if (!nano_pb_encode(&stream, CurrentDeviceState_fields, &state_key)) {
        vPortFree(buf);
        return NULL;
    }
    // test to check the encode is correct
    // app_gsound_device_action_decode_check(buf, *buf_len);
    return buf;
}

static bool app_gsound_device_action_decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    char *str = (char *)(*arg);
    if (!nano_pb_read(stream, (pb_byte_t *)str, stream->bytes_left)) {
        GSOUND_LOG_TEXT(LOG_TAG" decode_string error=%s", PB_GET_ERROR(stream));
        return FALSE;
    }
    GSOUND_LOG_TEXT(LOG_TAG" decode_string success=%s", str);
    return TRUE;
}

static bool app_gsound_device_action_decode_param(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    Execution_ParamsEntry *param = (Execution_ParamsEntry *)(*arg);
    char key[GSOUND_TARGET_DEVICE_ACTIONS_STATE_MAX_LENGTH] = {0};
    param->key.funcs.decode = app_gsound_device_action_decode_string;
    param->key.arg = key;
    if (!nano_pb_decode(stream, Execution_ParamsEntry_fields, param)) {
        return FALSE;
    }
    return TRUE;
}

/**************************************************************************************************
 * Do Action functions
**************************************************************************************************/
static void app_gsound_device_action_noise_cancellation(double value)
{
    // Implementation: Control noise cancellation (ANC)
    // value 0 - Close, >0 - Open with volume, -1: Open with the last time volume
#ifdef MTK_ANC_ENABLE
    if (value != 0) {
        app_gsound_device_action_turn_off_anc_pt();
        app_gsound_device_action_turn_on_anc();
    } else {
        app_gsound_device_action_turn_off_anc_pt();
    }
#endif

    app_gsound_device_action_request_state();
}

static void app_gsound_device_action_ambient_mode(double value)
{
    // Implementation: Control ambient mode (passthrough)
    // value 0 - Close, >0 - Open with volume, -1: Open with the last time volume
#ifdef MTK_ANC_ENABLE
    if (value != 0) {
        app_gsound_device_action_turn_off_anc_pt();
        app_gsound_device_action_turn_on_pt();
    } else {
        app_gsound_device_action_turn_off_anc_pt();
    }
#endif

    app_gsound_device_action_request_state();
}

static void app_gsound_device_action_turn_off(bool on_off)
{
    if (!on_off) {
#if defined(MTK_AWS_MCE_ENABLE)
        apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_GSOUND, APPS_EVENT_GSOUND_POWER_OFF_SYNC);
#endif
        bool *need_rho = (bool *)pvPortMalloc(sizeof(bool));
        if (need_rho == NULL) {
            GSOUND_LOG_E(LOG_TAG" power_off, malloc failed", 0);
            return;
        }
        *need_rho = FALSE;
        ui_shell_status_t status = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                                       EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                       APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF,
                                                       (void *)need_rho, sizeof(bool),
                                                       NULL, 0);
        if (status != UI_SHELL_STATUS_OK) {
            vPortFree(need_rho);
        }
    }
}
static void app_gsound_device_action_touch_control(bool on_off)
{
    /*
       Implementation: Control touch control
       on_off: TRUE: enable (device can be triggered by button)
               FALSE: disable (device can only be triggered by hotword)
    */
    apps_set_touch_control_enable(on_off, TRUE);

    app_gsound_device_action_request_state();
}

static bool app_gsound_device_action_command_handler(char *command, Execution_ParamsEntry *param)
{
    bool success = TRUE;
    if (strcmp(command, "action.devices.commands.headphone.UpdateNoiseCancellation") == 0) {
        app_gsound_device_action_noise_cancellation(param->value.kind.number_value);
    } else if (strcmp(command, "action.devices.commands.headphone.UpdateAmbientMode") == 0) {
        app_gsound_device_action_ambient_mode(param->value.kind.number_value);
    } else if (strcmp(command, "action.devices.commands.OnOff") == 0) {
        app_gsound_device_action_turn_off(param->value.kind.bool_value);
    } else if (strcmp(command, "action.devices.commands.UpdateTouchControl") == 0) {
        app_gsound_device_action_touch_control(param->value.kind.bool_value);
    } else {
        success = FALSE;
    }
    return success;
}



/**================================================================================*/
/**                                  Device Action API                             */
/**================================================================================*/
bool app_gsound_device_action_request_state(void)
{
    uint8_t *state_key_all = NULL;
    uint32_t state_len_all = 0;

    GSOUND_LOG_I(LOG_TAG" app_gsound_device_action_request_state, key_all=0x%08X",
                 1, gsound_device_action_state_key.key_all);

    if (gsound_device_action_state_key.key_all == NULL
        && (state_key_all = app_gsound_device_action_pb_encode(&state_len_all))) {
        gsound_device_action_state_key.key_all = state_key_all;
        if (gsound_device_action_send_state_key((uint8_t *)DEVICE_ACTION_STATE_KEY_ALL,
                                                strlen(DEVICE_ACTION_STATE_KEY_ALL),
                                                state_key_all, state_len_all)) {
            return TRUE;
        } else {
            gsound_device_action_state_key.key_all = NULL;
            vPortFree(state_key_all);
            return FALSE;
        }
    }
    return TRUE;
}

bool app_gsound_device_action_consume(const uint8_t *const action_state_key)
{
    if (strcmp((const char *)action_state_key, DEVICE_ACTION_STATE_KEY_ALL) == 0) {
        if (gsound_device_action_state_key.key_all != NULL) {
            vPortFree(gsound_device_action_state_key.key_all);
            gsound_device_action_state_key.key_all = NULL;
        }
    } else {
        GSOUND_LOG_E(LOG_TAG" app_gsound_device_action_consume, not match", 0);
    }
    return TRUE;
}

bool app_gsound_device_action_do_action(const uint8_t *device_actions, int device_actions_size)
{
    char command[GSOUND_TARGET_DEVICE_ACTIONS_COMMAND_MAX_LENGTH] = {0};
    Execution execution = Execution_init_zero;
    Execution_ParamsEntry param = Execution_ParamsEntry_init_zero;

    pb_istream_t stream = nano_pb_istream_from_buffer(device_actions, device_actions_size);
    execution.command.funcs.decode = app_gsound_device_action_decode_string;
    execution.command.arg = command;
    execution.params.funcs.decode = app_gsound_device_action_decode_param;
    execution.params.arg = &param;

    if (!nano_pb_decode(&stream, Execution_fields, &execution)) {
        return FALSE;
    }

    return app_gsound_device_action_command_handler(command, &param);
}

#endif /* AIR_GSOUND_ENABLE */
