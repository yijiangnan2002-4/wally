/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#include "app_hearing_aid_utils.h"
#include "app_hearing_aid_config.h"
#include "app_hearing_aid_storage.h"
#include "app_hear_through_race_cmd_handler.h"
#include "app_hearing_aid_activity.h"
#include "app_hear_through_storage.h"
#include "bt_sink_srv_ami.h"
#include "audio_anc_psap_control.h"
#include "apps_debug.h"
#include "bt_aws_mce.h"
#include "bt_aws_mce_srv.h"
#include "bt_device_manager.h"
#include "bt_connection_manager.h"
#include "bt_type.h"
#include "anc_control_api.h"
#include "app_hear_through_storage.h"
#ifdef AIR_DAC_MODE_RUNTIME_CHANGE
#include "hal_audio_internal.h"
#include "audio_set_driver.h"
#endif /* AIR_DAC_MODE_RUNTIME_CHANGE */

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)

#define APP_HA_UTILS_TAG        "[HearingAid][UTILS]"
#define APP_HA_COMMAND_DEBUG

#define APP_HA_BIT_COMPARE_VALUE_1                          0x01
#define APP_HA_BIT_COMPARE_VALUE_2                          0x03
#define APP_HA_BIT_COMPARE_VALUE_3                          0x07
#define APP_HA_BIT_COMPARE_VALUE_4                          0x0F

#define APP_HA_MODE_TABLE_MFA_SWITCH_L_OFFSET               0
#define APP_HA_MODE_TABLE_MFA_SWITCH_R_OFFSET               1
#define APP_HA_MODE_TABLE_LOW_CUT_SWITCH_L_OFFSET           2
#define APP_HA_MODE_TABLE_LOW_CUT_SWITCH_R_OFFSET           3
#define APP_HA_MODE_TABLE_NR_SWITCH_OFFSET                  4
#define APP_HA_MODE_TABLE_NR_LEVEL_OFFSET                   5
#define APP_HA_MODE_TABLE_BF_SWITCH_OFFSET                  0

#define APP_HA_AEA_SWITCH_OFFSET                            0
#define APP_HA_AES_NR_SWITCH_OFFSET                         1
#define APP_HA_AEA_NR_LEVEL_OFFSET                          2

#define APP_HA_BF_SETTINGS_BF_SWITCH_OFFSET                 0
#define APP_HA_BF_SETTINGS_BF_MODE_CONTROL_SWITCH_OFFSET    1

#define APP_HA_AFC_SWITCH_L_OFFSET                          0
#define APP_HA_AFC_SWITCH_R_OFFSET                          1
#define APP_HA_AFC_GR_SWITCH_L_OFFSET                       2
#define APP_HA_AFC_GR_SWITCH_R_OFFSET                       3

#define APP_HA_INR_SWITCH_OFFSET                            0
#define APP_HA_INR_INSENSITIVITY_OFFSET                     1
#define APP_HA_INR_GAIN_INDEX_OFFSET                        5
#define APP_HA_INR_INTENSITY_OFFSET                         0

#define APP_HA_USER_EQ_SWITCH_L_OFFSET                      0
#define APP_HA_USER_EQ_SWITCH_R_OFFSET                      0

#define APP_HA_MIX_MODE_SETTING_A2DP_SWITCH                 0
#define APP_HA_MIX_MODE_SETTING_A2DP_WDRC_SWITCH_L          3
#define APP_HA_MIX_MODE_SETTING_A2DP_WDRC_SWITCH_R          4
#define APP_HA_MIX_MODE_SETTING_A2DP_MFA_SWITCH_L           5
#define APP_HA_MIX_MODE_SETTING_A2DP_MFA_SWITCH_R           6

#define APP_HA_MIX_MODE_SETTING_SCO_SWITCH                  0
#define APP_HA_MIX_MODE_SETTING_SCO_WDRC_SWITCH_L           1
#define APP_HA_MIX_MODE_SETTING_SCO_WDRC_SWITCH_R           2
#define APP_HA_MIX_MODE_SETTING_SCO_MFA_SWITCH_L            3
#define APP_HA_MIX_MODE_SETTING_SCO_MFA_SWITCH_R            4

#define APP_HA_MIX_MODE_SETTING_VP_SWITCH                   0
#define APP_HA_MIX_MODE_SETTING_VP_WDRC_SWITCH_L            1
#define APP_HA_MIX_MODE_SETTING_VP_WDRC_SWITCH_R            2
#define APP_HA_MIX_MODE_SETTING_VP_MFA_SWITCH_L             3
#define APP_HA_MIX_MODE_SETTING_VP_MFA_SWITCH_R             4

#define APP_HA_CHANNEL_L                                    0x01
#define APP_HA_CHANNEL_R                                    0x02
#define APP_HA_CHANNEL_STEREO                               0x03

#define APP_HA_MP_TEST_MODE_SWITCH_ON                       0x03
#define APP_HA_MP_TEST_MODE_SWITCH_OFF                      0x00

#define APP_HA_USER_EQ_GAIN_TABLE_LENGTH                    17

#define APP_HA_FEEDBACK_DETECTION_NOTIFY_LENGTH             7
#define APP_HA_FEEDBACK_DETECTION_NOTIFY_CALLBACK_DATA_LEN  3

#define APP_HA_MIC_CALIBRATION_DATA_COMBINE_LEN             (APP_HEARING_AID_RESPONSE_MAX_LEN * 2)

#define APP_HA_SPEAKER_REFERENCE_COMBINE_LEN                (sizeof(psap_test_spk_ref_t))

#define APP_HA_NOTIFY_MSG_COMBINER(event, msg)              ((((uint32_t)event) << 16) | msg)


typedef struct {
    uint8_t                 l_switch: 1;
    uint8_t                 r_switch: 1;
    uint8_t                 reserved: 6;
    uint16_t                l_freq;
    uint8_t                 l_dbfs;
    uint16_t                r_freq;
    uint8_t                 r_dbfs;
} __attribute__((packed)) app_hearing_aid_pure_tone_generator_t;

typedef struct {
    uint8_t             l_level_index;
    uint8_t             r_level_index;
} __attribute__((packed)) app_hearing_aid_level_index_t;

typedef struct {
    uint8_t             l_vol_index;
    uint8_t             r_vol_index;
} __attribute__((packed)) app_hearing_aid_vol_index_t;

typedef struct {
    uint8_t             modex_index;
} __attribute__((packed)) app_hearing_aid_mode_index_t;

typedef struct {
    uint8_t             l_passthrough_switch;
    uint8_t             r_passthrough_switch;
} __attribute__((packed)) app_hearing_aid_passthrough_switch_t;

typedef struct {
    uint8_t             l_switch: 1;
    uint8_t             r_switch: 1;
    uint8_t             reserved: 6;
} __attribute__((packed)) app_hearing_aid_hearing_tuning_mode_t;

typedef struct {
    uint8_t             channel;
    uint8_t             left_val[3];
    uint8_t             right_val[3];
} __attribute__((packed)) app_hearing_aid_feedback_detection_notify_t;

typedef struct {
    uint8_t             l_mute;
    uint8_t             r_mute;
} __attribute__((packed)) app_hearing_aid_ha_mute_t;

typedef struct {
    U8 a2dp_mix_mode_switch  : 1;                          /**< @Value 0 @Desc 1 */
    U8 reserved_3            : 2;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_drc_switch_l     : 1;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_drc_switch_r     : 1;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_mfa_switch_l     : 1;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_mfa_switch_r     : 1;                          /**< @Value 0 @Desc 1 */
    U8 reserved_4            : 1;                          /**< @Value 0 @Desc 1 */
    S8 a2dp_mix_mode_psap_gain_l;                            /**< @Value 0 @Desc 1 */
    S8 a2dp_mix_mode_psap_gain_r;                            /**< @Value 0 @Desc 1 */

    U8 sco_mix_mode_switch   : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_drc_switch_l      : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_drc_switch_r      : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_mfa_switch_l      : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_mfa_switch_r      : 1;                          /**< @Value 0 @Desc 1 */
    U8 reserved_5            : 3;                          /**< @Value 0 @Desc 1 */
    S8 sco_mix_mode_psap_gain_l;                             /**< @Value 0 @Desc 1 */
    S8 sco_mix_mode_psap_gain_r;                             /**< @Value 0 @Desc 1 */

    U8 vp_mix_mode_switch   : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_drc_switch_l      : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_drc_switch_r      : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_mfa_switch_l      : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_mfa_switch_r      : 1;                           /**< @Value 0 @Desc 1 */
    U8 reserved_6           : 3;                           /**< @Value 0 @Desc 1 */
    S8 vp_mix_mode_psap_gain_l;                              /**< @Value 0 @Desc 1 */
    S8 vp_mix_mode_psap_gain_r;                              /**< @Value 0 @Desc 1 */

} __attribute__((packed)) app_mix_mode_t;


typedef struct {
    uint8_t             count;
    app_hearing_aid_feedback_detection_notify_t stored_info;
} app_hearing_aid_feedback_detection_notify_info_t;

typedef struct {
    bool                init_done;
    bool                user_switch;
#ifdef AIR_DAC_MODE_RUNTIME_CHANGE
    bool                is_hearing_test_mode;
    uint32_t            dac_mode_before_hearing_test_mode;
#endif /* AIR_DAC_MODE_RUNTIME_CHANGE */
    notify              notify_handler;
    uint8_t             feedback_detection_channel;
    uint32_t            stored_mic_input_path;
    app_mix_mode_t      ha_mix_mode;
    app_hearing_aid_feedback_detection_notify_info_t  fb_det_notify_info;
} app_hearing_aid_utils_context_t;

app_hearing_aid_utils_context_t app_ha_utils_context;

typedef struct {
    bool                rsp_exist;
    uint8_t             response[APP_HEARING_AID_RESPONSE_MAX_LEN];
} app_hearing_aid_utils_mic_calibration_data_combine_response_t;

static app_hearing_aid_utils_mic_calibration_data_combine_response_t app_ha_mic_calibration_data_combine_response;

typedef struct {
    bool                    is_rsp_exist;
    bool                    is_local;
    psap_test_spk_ref_t     spk_ref;
} app_hearing_aid_utils_speaker_reference_combine_response_t;

static app_hearing_aid_utils_speaker_reference_combine_response_t app_ha_speaker_reference_combine_response;

ATTR_LOG_STRING_LIB app_hearing_aid_command_string[][APP_HEARING_AID_CHAR_LOG_MAX_LEN] = {
    "None",
    "Set",
    "Get"
};

ATTR_LOG_STRING_LIB app_hearing_aid_type_string[][APP_HEARING_AID_CHAR_LOG_MAX_LEN] = {
    "None",                         // 0x0000
    "HA_SWITCH",                    // 0x0001
    "HA_LEVEL_INDEX",               // 0x0002
    "HA_LEVEL_SYNC_SWITCH",         // 0x0003
    "HA_LEVEL_MODE_MAXCOUNT",       // 0x0004
    "HA_VOLUME_INDEX",              // 0x0005
    "HA_VOLUME_SYNC_SWITCH",        // 0x0006
    "HA_MODE_INDEX",                // 0x0007
    "HA_SPECIFIC_MODE_TABLE",       // 0x0008
    "HA_AEA_CONFIGURATION",         // 0x0009
    "HA_WNR_SWITCH",                // 0x000A
    "HA_BEAMFORMING_SETTINGS",      // 0x000B
    "HA_AFC_CONFIG",                // 0x000C
    "HA_INR_CONFIG",                // 0x000D
    "HA_USEREQ_SWITCH",             // 0x000E
    "HA_USEREQ_GAIN",               // 0x000F
    "HA_SPEAKER_REFERENCE",         // 0x0010
    "HA_PURETONE_GENERATOR",        // 0x0011
    "HA_MIXMODE_TOTALSETTING",      // 0x0012
    "HA_HEARINGTUNINGMODE_SWITCH",  // 0x0013
    "HA_MPTESTMODE_SWITCH",         // 0x0014
    "HA_RESTORE_SETTING",           // 0x0015
    "HA_FEEDBACK_DETECTION",        // 0x0016
    "HA_MUTE",                      // 0x0017
    "HA_HOWLING_DETECTION",         // 0x0018
    "HA_MPO_ADJUSTMENT",            // 0x0019
    "HA_INEAR_DETECTION",           // 0x001A
    "HA_PASSTHROUGH_SWITCH",        // 0x001B
    "HA_MASTER_MIC_CHANNEL",        // 0x001C
    "HA_TRIAL_RUN",                 // 0x001D
    "HA_MIC_CALIBRATION_MODE",      // 0x001E
    "HA_MIC_CALIBRATION_DATA",      // 0x001F
    "HA_HEARING_TEST_MODE",         // 0x0020
};

ATTR_LOG_STRING_LIB app_hearing_aid_execute_where_string[][APP_HEARING_AID_CHAR_LOG_MAX_LEN] = {
    "0 - None",
    "1 - AGENT",
    "2 - PARTNER",
    "3 - BOTH",
};

extern audio_psap_status_t audio_anc_psap_control_trial_run(ha_trial_run_event_t event, U32 data_len, void* data);

static uint8_t APP_HA_BIT_VALUE(uint8_t value, uint8_t offset, uint8_t c_value)
{
    return ((value >> offset) & c_value);
}

static bool APP_HA_IS_ENABLE(uint8_t value, uint8_t offset)
{
    return APP_HA_BIT_VALUE(value, offset, APP_HA_BIT_COMPARE_VALUE_1);
}

static void app_hearing_aid_utils_notify(uint16_t event, uint16_t msg, uint8_t *data, uint16_t data_len)
{
    uint32_t value = APP_HA_NOTIFY_MSG_COMBINER(event, msg);
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_notify] value : 0x%08x, event : 0x%04x, msg : 0x%04x", 3,
                     value, event, msg);
    if (app_ha_utils_context.notify_handler != NULL) {
        app_ha_utils_context.notify_handler(app_hearing_aid_utils_get_role(), value, data, data_len);
    }
}

static void app_hearing_aid_utils_check_master_mic_channel_changed(bool need_reload_config)
{
    uint32_t mic_input_path = 0;
    audio_anc_psap_control_get_mic_input_path(&mic_input_path);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_check_master_mic_channel_changed] Check, need_reload_config : %d, mic_path : %d -> %d",
                        3,
                        need_reload_config,
                        app_ha_utils_context.stored_mic_input_path,
                        mic_input_path);

    if (app_ha_utils_context.stored_mic_input_path != mic_input_path) {
        app_ha_utils_context.stored_mic_input_path = mic_input_path;

        app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_TO_CONTROL_FWK,
                                        0,
                                        NULL,
                                        0);
    } else {
        if (need_reload_config == true) {
            app_hearing_aid_utils_reload_configuration();
        }
    }
}

static void app_hearing_aid_utils_feedback_detection_callback_handler(uint8_t *result)
{
    /**
     *   B0: GRS channel
     *   B1~B3 : Left channel result
     *   B4~B6 : Right channel result
     */
    uint8_t notify_data[APP_HA_FEEDBACK_DETECTION_NOTIFY_LENGTH] = {0};
    app_hearing_aid_feedback_detection_notify_t *det_notify = (app_hearing_aid_feedback_detection_notify_t *)notify_data;

    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        det_notify->channel = APP_HA_CHANNEL_L;
        memcpy(det_notify->left_val, result, APP_HA_FEEDBACK_DETECTION_NOTIFY_CALLBACK_DATA_LEN);
    } else {
        det_notify->channel = APP_HA_CHANNEL_R;
        memcpy(det_notify->right_val, result, APP_HA_FEEDBACK_DETECTION_NOTIFY_CALLBACK_DATA_LEN);
    }

    app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_EVENT_NOTIFICATION,
                                 APP_HEARING_AID_CONFIG_TYPE_FEEDBACK_DETECTION,
                                 notify_data,
                                 APP_HA_FEEDBACK_DETECTION_NOTIFY_LENGTH);
}

static void app_hearing_aid_utils_ha_callback_handler(psap_noti_event_t event, uint8_t *extra_data, uint16_t extra_data_len)
{
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_ha_callback_handler] Event : %d, data : 0x%x, data_len : %d",
                     3,
                     event,
                     extra_data,
                     extra_data_len);

    if (event == PSAP_NOTI_EVENT_FB_DETECT) {
        if (extra_data_len == APP_HA_FEEDBACK_DETECTION_NOTIFY_CALLBACK_DATA_LEN) {
            app_hearing_aid_utils_feedback_detection_callback_handler(extra_data);
        }
    } else if (event == PSAP_NOTI_EVENT_AEA_CHANGE_MODE) {
        app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_TO_UPDATE_MODE_INDEX,
                                     0,
                                     extra_data,
                                     extra_data_len);
    }
}

void app_hearing_aid_utils_init(notify handler)
{
    if (app_ha_utils_context.init_done == true) {
        return;
    }

    memset(&app_ha_utils_context, 0, sizeof(app_hearing_aid_utils_context_t));
    memset(&app_ha_mic_calibration_data_combine_response, 0, sizeof(app_hearing_aid_utils_mic_calibration_data_combine_response_t));

    audio_psap_status_t init_status = audio_anc_psap_control_init();
    audio_psap_status_t init_detect_fb_status = audio_anc_psap_control_register_notification_callback(app_hearing_aid_utils_ha_callback_handler);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_init] init status : %d, detect_fb : %d",
                     2,
                     init_status,
                     init_detect_fb_status);

    if ((init_status != AUDIO_PSAP_STATUS_SUCCESS)
        || (init_detect_fb_status != AUDIO_PSAP_STATUS_SUCCESS)) {
        return;
    }

    app_ha_utils_context.init_done = true;
    app_ha_utils_context.notify_handler = handler;

    uint16_t rsp_len = 0;
    app_hearing_aid_utils_get_mix_mode_total_setting(NULL, (uint8_t *)(&(app_ha_utils_context.ha_mix_mode)), &rsp_len);

    audio_anc_psap_control_get_mic_input_path(&(app_ha_utils_context.stored_mic_input_path));

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_init] mic_input_path : %d", 1, app_ha_utils_context.stored_mic_input_path);
}

void app_hearing_aid_utils_deinit()
{
    if (app_ha_utils_context.init_done == false) {
        return;
    }

    // audio_anc_psap_control_close_framework();
    memset(&app_ha_utils_context, 0, sizeof(app_hearing_aid_utils_context_t));
}

audio_psap_device_role_t app_hearing_aid_utils_get_role()
{
    audio_channel_t channel = ami_get_audio_channel();
        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_role] channel : %d", 1, channel);
    if (channel == AUDIO_CHANNEL_L) {
        return AUDIO_PSAP_DEVICE_ROLE_LEFT;
    } else if (channel == AUDIO_CHANNEL_R) {
        return AUDIO_PSAP_DEVICE_ROLE_RIGHT;
    } else {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_role] Unknown channel : %d", 1, channel);
        return AUDIO_PSAP_DEVICE_ROLE_MAX;
    }
}


void app_hearing_aid_utils_handle_get_race_cmd(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    if (app_ha_exe_handler_list[request->op_type].ha_cmd_get_handler != NULL) {
        app_ha_exe_handler_list[request->op_type].ha_cmd_get_handler(request, response, response_len);
    }
}

void app_hearing_aid_utils_handle_set_race_cmd(app_hear_through_request_t *request, bool *set_result)
{
    bool execute_result = false;

    if (app_ha_exe_handler_list[request->op_type].ha_cmd_set_handler != NULL) {
        execute_result = app_ha_exe_handler_list[request->op_type].ha_cmd_set_handler(request->op_parameter);
    }

    *set_result = execute_result;
}

static bool app_hearing_aid_utils_get_combine_response_checker(uint8_t *response, uint16_t response_len,
                                                                uint8_t *combine_response, uint16_t *combine_response_len)
{
    if ((response == NULL)
            || (response_len == 0)
            || (combine_response == NULL)
            || (combine_response_len == NULL)) {
        return false;
    }

    return true;
}

bool app_hearing_aid_utils_handle_get_combine_response(uint16_t type,
                                                       bool is_local_response,
                                                       uint8_t *response,
                                                       uint16_t response_len,
                                                       uint8_t *combine_response,
                                                       uint16_t *combine_response_len)
{
    if (app_hearing_aid_utils_get_combine_response_checker(response, response_len, combine_response, combine_response_len) == false) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[get_combine_response] Parameter error, type : 0x%04x, response : 0x%x, response_len : %d, combine_response : 0x%x, combine_response_len : 0x%x",
                            5,
                            type,
                            response,
                            response_len,
                            combine_response,
                            combine_response_len);
        return false;
    }

    if (app_ha_exe_handler_list[type].ha_cmd_get_combine_handler != NULL) {
        return app_ha_exe_handler_list[type].ha_cmd_get_combine_handler(is_local_response,
                                                                        response,
                                                                        response_len,
                                                                        combine_response,
                                                                        combine_response_len);
    } else {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[get_combine_response] combine_handler is NULL and this should not happen, 0x%04x", 1, type);

        return false;
    }
}

uint32_t app_hearing_aid_utils_get_combine_response_length(uint16_t type)
{
    if (app_ha_exe_handler_list[type].ha_cmd_get_combine_response_len != NULL) {
        return app_ha_exe_handler_list[type].ha_cmd_get_combine_response_len();
    }

    return 0;
}

bool app_hearing_aid_utils_handle_notify(uint8_t role, uint16_t type, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
    if (data == NULL || data_len == 0 || notify_data_len == NULL || notify_data == NULL) {
        return false;
    }

    if (app_ha_exe_handler_list[type].ha_notify != NULL) {
        return app_ha_exe_handler_list[type].ha_notify(role,
                                                       data,
                                                       data_len,
                                                       notify_data,
                                                       notify_data_len);
    } else {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_handle_notify] Notify handler is NULL and this should not happen, 0x%04x", 1, type);
    }

    return false;
}


/**=====================================================================================*/
/** Get function handler                                                                */
/**=====================================================================================*/
bool app_hearing_aid_utils_get_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{

    response[0] = app_ha_utils_context.user_switch;
    *response_len = 1;
    return true;
}

bool app_hearing_aid_utils_get_level_index(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    uint8_t l_index = 0;
    uint8_t r_index = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_level_index(&l_index, &r_index);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;

    if (ret_value == true) {
        app_hearing_aid_level_index_t *level_index = (app_hearing_aid_level_index_t *)response;
        level_index->l_level_index = l_index;
        level_index->r_level_index = r_index;

        *response_len = sizeof(app_hearing_aid_level_index_t);
    }
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_get_level_sync_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    UNUSED(request);
    uint8_t enable = app_hearing_aid_storage_get_level_sync_switch();

    response[0] = enable;
    *response_len = sizeof(uint8_t);

    return true;
}

bool app_hearing_aid_utils_get_mode_max_count(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    UNUSED(request);
    uint8_t level_max_count = 0;
    uint8_t mode_max_count = 0;
    uint8_t vol_max_count = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_level_mode_max_count(&level_max_count, &mode_max_count, &vol_max_count);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
    *response_len = 0;
    if (ret_value == true) {
        response[0] = level_max_count;
        response[1] = mode_max_count;
        response[2] = vol_max_count;
        *response_len = 3;
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_volume_index(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    uint8_t l_vol_index = 0;
    uint8_t r_vol_index = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_volume_index(&l_vol_index, &r_vol_index);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
    *response_len = 0;
    if (ret_value == true) {
        response[0] = l_vol_index;
        response[1] = r_vol_index;
        *response_len = 2;
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_volume_sync_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    bool enable = app_hearing_aid_storage_get_volume_sync_switch();

    response[0] = enable;
    *response_len = 1;
    return true;
}

bool app_hearing_aid_utils_get_mode_index(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    uint8_t mode_index = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_mode_index(&mode_index);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        app_hearing_aid_mode_index_t *mode = (app_hearing_aid_mode_index_t *)response;

        mode->modex_index = mode_index;
        *response_len = sizeof(app_hearing_aid_mode_index_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_specific_mode_table(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    psap_mode_table_t mode_table = {0};
    uint8_t mode_index = request->op_parameter[0];
    audio_psap_status_t status = audio_anc_psap_control_get_specific_mode_table(mode_index, &mode_table);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        response[0] = mode_index;
        memcpy(response + 1, &mode_table, sizeof(psap_mode_table_t));
        *response_len = sizeof(psap_mode_table_t) + 1;
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_aea_configuration(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    psap_aea_config_t config = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_aea_configuration(&config);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        memcpy(response, &config, sizeof(psap_aea_config_t));
        *response_len = sizeof(psap_aea_config_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_wnr_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    bool enable = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_wnr_switch(&enable);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        response[0] = enable;
        *response_len = 1;
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_beam_forming_setting(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    psap_bf_config_t config = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_beamforming_setting(&config);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        memcpy(response, &config, sizeof(psap_bf_config_t));
        *response_len = sizeof(psap_bf_config_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_afc_config(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    psap_afc_config_t config = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_afc_configuration(&config);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        memcpy(response, &config, sizeof(psap_afc_config_t));
        *response_len = sizeof(psap_afc_config_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_inr_config(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    ha_inr_config_t config = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_inr_configuration(&config);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        memcpy(response, &config, sizeof(ha_inr_config_t));
        *response_len = sizeof(ha_inr_config_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_user_eq_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    psap_user_eq_switch_t config = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_user_eq_switch(&config);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        memcpy(response, &config, sizeof(psap_user_eq_switch_t));
        *response_len = sizeof(psap_user_eq_switch_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_user_eq_gain(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    psap_usr_eq_para_t eq_para = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_user_eq_gain(&eq_para);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        memcpy(response, &eq_para, sizeof(psap_usr_eq_para_t));
        *response_len = sizeof(psap_usr_eq_para_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_speaker_reference(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    psap_test_spk_ref_t spk_ref = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_speaker_reference(&spk_ref);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        memcpy(response, &spk_ref, sizeof(psap_test_spk_ref_t));
        *response_len = sizeof(psap_test_spk_ref_t);
    }

    return ret_value;
}

bool app_hearing_aid_utils_get_pure_tone_generator(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{

    app_hearing_aid_pure_tone_generator_t *resp = (app_hearing_aid_pure_tone_generator_t *)response;

    bool l_enable = 0;
    bool r_enable = 0;
    uint16_t l_freq = 0;
    uint16_t r_freq = 0;
    int8_t l_dbfs = 0;
    int8_t r_dbfs = 0;

    audio_psap_status_t status = AUDIO_PSAP_STATUS_SUCCESS;

    status = audio_anc_psap_control_get_puretone_generator(&l_enable, &l_freq, &l_dbfs, &r_enable, &r_freq, &r_dbfs);

    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {

        resp->l_switch = (l_enable == true ? true : false);
        resp->r_switch = (r_enable == true ? true : false);
        resp->l_freq = l_freq;
        resp->r_freq = r_freq;
        resp->l_dbfs = l_dbfs;
        resp->r_dbfs = r_dbfs;

#if 0
        if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
            resp->l_switch = (enable == true ? true : false);
            resp->l_freq = freq;
            resp->l_dbfs = dbfs;
        } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
            resp->r_switch = (enable == true ? true : false);
            resp->r_freq = freq;
            resp->r_dbfs = dbfs;
        } else {
            return false;
        }
#endif
        *response_len = sizeof(app_hearing_aid_pure_tone_generator_t);
    }

    return ret_value;
}

bool app_hearing_aid_utils_get_mix_mode_total_setting(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    psap_scenario_mix_mode_t mix_mode = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_mix_mode(&mix_mode);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    app_mix_mode_t app_mix_mode;
    memcpy(&app_mix_mode, &mix_mode, sizeof(app_mix_mode_t));

    app_mix_mode.a2dp_mix_mode_switch = app_hearing_aid_storage_get_a2dp_mix_switch();
    app_mix_mode.sco_mix_mode_switch = app_hearing_aid_storage_get_sco_mix_switch();
    app_mix_mode.vp_mix_mode_switch = app_hearing_aid_storage_get_vp_mix_switch();

    *response_len = 0;
    if (ret_value == true) {
        memcpy(response, &app_mix_mode, sizeof(psap_scenario_mix_mode_t));
        *response_len = sizeof(psap_scenario_mix_mode_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_hearing_tuning_mode_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    uint8_t mode_switch = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_tuning_mode(&mode_switch);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        response[0] = mode_switch;
        *response_len = sizeof(uint8_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_mp_test_mode_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    uint8_t mp_mode = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_mp_test_mode(&mp_mode);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        response[0] = mp_mode;
        *response_len = sizeof(uint8_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_feedback_detection(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    uint8_t channel = request->op_parameter[0];
    bool l_enable = APP_HA_IS_ENABLE(channel, 0);
    bool r_enable = APP_HA_IS_ENABLE(channel, 1);
    audio_psap_status_t get_fb_status = AUDIO_PSAP_STATUS_SUCCESS;

    app_ha_utils_context.feedback_detection_channel = 0x00;
    *response_len = 0;

    if (app_hearing_aid_utils_is_mp_test_mode() == false) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_feedback_detection] Not MP test mode", 0);
        return false;
    }

    bool enable = false;

    if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (l_enable == true))
        || ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (r_enable == true))) {
        enable = true;
    }

    if (enable == true) {
        get_fb_status = audio_anc_psap_control_detect_feedback();
    }
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_feedback_detection] role : %d, channel : 0x%02x, l_enable : %d, r_enable : %d, enable : %d, status : %d",
                        6,
                        role,
                        channel,
                        l_enable,
                        r_enable,
                        enable,
                        get_fb_status);

    if ((enable == true) && (get_fb_status == AUDIO_PSAP_STATUS_SUCCESS)) {
        app_ha_utils_context.feedback_detection_channel = channel;
    }

    response[0] = channel;
    *response_len = sizeof(uint8_t);

    return true;
}

bool app_hearing_aid_utils_get_mute(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    bool l_mute = 0;
    bool r_mute = 0;
    app_hearing_aid_ha_mute_t *rsp = (app_hearing_aid_ha_mute_t *)response;
    audio_psap_status_t status = audio_anc_psap_control_get_mute(&l_mute, &r_mute);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        rsp->l_mute = l_mute;
        rsp->r_mute = r_mute;

        *response_len = sizeof(app_hearing_aid_ha_mute_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_howling_detection(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    psap_how_det_t det = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_howling_detection(&det);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        memcpy(response, &det, sizeof(psap_how_det_t));
        *response_len = sizeof(psap_how_det_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_mpo_adjustment(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    ha_mpo_adjust_t adjust = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_mpo_adjustment(&adjust);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    *response_len = 0;
    if (ret_value == true) {
        memcpy(response, &adjust, sizeof(ha_mpo_adjust_t));
        *response_len = sizeof(ha_mpo_adjust_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_in_ear_detection_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    bool enable = app_hearing_aid_storage_get_in_ear_detection_switch();
    response[0] = enable;
    *response_len = sizeof(uint8_t);
    return true;
}

bool app_hearing_aid_utils_get_passthrough_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    bool l_enable = 0;
    bool r_enable = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_passthrough_switch(&l_enable, &r_enable);
    bool ret_value = (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    app_hearing_aid_passthrough_switch_t *switch_response = (app_hearing_aid_passthrough_switch_t *)response;

    *response_len = 0;
    if (ret_value == true) {
        switch_response->l_passthrough_switch = l_enable;
        switch_response->r_passthrough_switch = r_enable;
        *response_len = sizeof(app_hearing_aid_passthrough_switch_t);
    }
    return ret_value;
}

bool app_hearing_aid_utils_get_mic_channel(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    uint8_t channel = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_mic_channel(&channel);

    *response_len = 0;
    if (status == AUDIO_PSAP_STATUS_SUCCESS) {
        response[0] = channel;
        *response_len = sizeof(uint8_t);
        return true;
    }
    return false;
}

extern audio_psap_status_t audio_anc_psap_control_get_mic_cal_mode(U8 *mic_cal_mode, U8 *mic_cal_mode_len);

bool app_hearing_aid_utils_get_mic_calibration_mode(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    audio_psap_status_t psap_status = audio_anc_psap_control_get_mic_cal_mode(response, (uint8_t *)response_len);
    return (psap_status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

extern audio_psap_status_t audio_anc_psap_control_get_mic_cal_data(U32 *len, U8* data);

bool app_hearing_aid_utils_get_mic_calibration_data(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    audio_psap_status_t psap_status = audio_anc_psap_control_get_mic_cal_data((uint32_t *)response_len, response);
    return (psap_status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

#ifdef AIR_DAC_MODE_RUNTIME_CHANGE
bool app_hearing_aid_utils_get_hearing_test_mode(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len)
{
    response[0] = app_ha_utils_context.is_hearing_test_mode;
    *response_len = sizeof(uint8_t);
    return true;
}
#endif /* AIR_DAC_MODE_RUNTIME_CHANGE */

/**=====================================================================================*/
/** Set function handler                                                                */
/**=====================================================================================*/

bool app_hearing_aid_utils_set_switch(uint8_t *parameter)
{
    app_hearing_aid_activity_set_user_switch(false, parameter[0]);
    app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_RACE_CMD, parameter[0], false);
    return true;
}

bool app_hearing_aid_utils_set_level_index(uint8_t *parameter)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    if (role == AUDIO_PSAP_DEVICE_ROLE_MAX) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_level_index(parameter[0], parameter[1], role);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_level_sync_switch(uint8_t *parameter)
{
    app_hearing_aid_storage_set_level_sync_switch((bool)(parameter[0]));
    return true;
}

bool app_hearing_aid_utils_set_volume_index(uint8_t *parameter)
{

    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    if (role == AUDIO_PSAP_DEVICE_ROLE_MAX) {
        return false;
    }
    app_hearing_aid_vol_index_t *index = (app_hearing_aid_vol_index_t *)parameter;

    audio_psap_status_t status = audio_anc_psap_control_set_volume_index(index->l_vol_index, index->r_vol_index, role);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_volume_sync_switch(uint8_t *parameter)
{
    bool enable = (bool)(parameter[0]);
    app_hearing_aid_storage_set_volume_sync_switch(enable);
    return true;
}

bool app_hearing_aid_utils_set_mode_index(uint8_t *parameter)
{
    if ((app_hearing_aid_utils_is_hearing_tuning_mode() == true)
        || (app_hearing_aid_utils_is_mp_test_mode() == true)) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_mode_index(parameter[0]);

    if (status == AUDIO_PSAP_STATUS_SUCCESS) {
        app_hearing_aid_utils_check_master_mic_channel_changed(false);
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_set_specific_mode_table(uint8_t *parameter)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    if (role == AUDIO_PSAP_DEVICE_ROLE_MAX) {
        return false;
    }

    if ((app_hearing_aid_utils_is_hearing_tuning_mode() == true)
        || (app_hearing_aid_utils_is_mp_test_mode() == true)) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_specific_mode_table(parameter[0], (psap_mode_table_t *)(parameter + 1), role);

    if (status == AUDIO_PSAP_STATUS_SUCCESS) {
        app_hearing_aid_utils_check_master_mic_channel_changed(false);
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_set_aea_configuration(uint8_t *parameter)
{
    if ((app_hearing_aid_utils_is_hearing_tuning_mode() == true)
        || (app_hearing_aid_utils_is_mp_test_mode() == true)) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_aea_configuration((psap_aea_config_t *)parameter);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    return true;
}

bool app_hearing_aid_utils_set_wnr_switch(uint8_t *parameter)
{
    if ((app_hearing_aid_utils_is_hearing_tuning_mode() == true)
        || (app_hearing_aid_utils_is_mp_test_mode() == true)) {
        return false;
    }

    bool enable = (bool)(parameter[0]);
    audio_psap_status_t status = audio_anc_psap_control_set_wnr_switch(enable);

    if (status == AUDIO_PSAP_STATUS_SUCCESS) {
        app_hearing_aid_utils_check_master_mic_channel_changed(false);
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_set_beam_forming_setting(uint8_t *parameter)
{
    if ((app_hearing_aid_utils_is_hearing_tuning_mode() == true)
        || (app_hearing_aid_utils_is_mp_test_mode() == true)) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_beamforming_setting((psap_bf_config_t *)parameter);

    if (status == AUDIO_PSAP_STATUS_SUCCESS) {
        app_hearing_aid_utils_check_master_mic_channel_changed(false);
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_set_afc_config(uint8_t *parameter)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    if (role == AUDIO_PSAP_DEVICE_ROLE_MAX) {
        return false;
    }

    if ((app_hearing_aid_utils_is_hearing_tuning_mode() == true)
        || (app_hearing_aid_utils_is_mp_test_mode() == true)) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_afc_configuration((psap_afc_config_t *)parameter, role);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_inr_config(uint8_t *parameter)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    if (role == AUDIO_PSAP_DEVICE_ROLE_MAX) {
        return false;
    }

    if ((app_hearing_aid_utils_is_hearing_tuning_mode() == true)
        || (app_hearing_aid_utils_is_mp_test_mode() == true)) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_inr_configuration((ha_inr_config_t *)parameter, role);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_user_eq_switch(uint8_t *parameter)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    if (role == AUDIO_PSAP_DEVICE_ROLE_MAX) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_user_eq_switch((psap_user_eq_switch_t *)parameter, role);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_user_eq_gain(uint8_t *parameter)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    if (role == AUDIO_PSAP_DEVICE_ROLE_MAX) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_user_eq_gain((psap_usr_eq_para_t *)parameter, role);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_pure_tone_generator(uint8_t *parameter)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    if (role == AUDIO_PSAP_DEVICE_ROLE_MAX) {
        return false;
    }

    app_hearing_aid_pure_tone_generator_t *config = (app_hearing_aid_pure_tone_generator_t *)parameter;

    audio_psap_status_t set_pure_status = AUDIO_PSAP_STATUS_SUCCESS;
    // audio_psap_status_t enable_status = AUDIO_PSAP_STATUS_SUCCESS;

#if 0
    if (enable == true) {
        enable_status = audio_anc_psap_control_enable();
    }
#endif
#if 0
    if (((enable == true)/* && (enable_status == AUDIO_PSAP_STATUS_SUCCESS)*/)
        || (enable == false)) {
        set_pure_status = audio_anc_psap_control_set_puretone_generator(enable, freq, dbfs);
    }
#endif

    set_pure_status = audio_anc_psap_control_set_puretone_generator(config->l_switch,
                                                                   config->l_freq,
                                                                   config->l_dbfs,
                                                                   config->r_switch,
                                                                   config->r_freq,
                                                                   config->r_dbfs);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_pure_tone_generator] set status : %d",
                     1,
                     set_pure_status);

#if 0
    if ((enable == false) && (set_pure_status == AUDIO_PSAP_STATUS_SUCCESS)) {
        app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_TO_CONTROL_HA,
                                     false,
                                     NULL,
                                     0);
    }
#endif
    return (set_pure_status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_mix_mode_total_setting(uint8_t *parameter)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    if (role == AUDIO_PSAP_DEVICE_ROLE_MAX) {
        return false;
    }

    if (app_hearing_aid_utils_is_mp_test_mode() == true) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_mix_mode((psap_scenario_mix_mode_t *)parameter, role);

#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
    hal_audio_status_send_update_dac_mode_event_to_am(HAL_AUDIO_HA_DAC_FLAG_A2DP_MIX_MODE, ((psap_scenario_mix_mode_t *)parameter)->a2dp_mix_mode_switch);
    hal_audio_status_send_update_dac_mode_event_to_am(HAL_AUDIO_HA_DAC_FLAG_SCO_MIX_MODE, ((psap_scenario_mix_mode_t *)parameter)->sco_mix_mode_switch);
#endif

    bool mix_mode_changed = false;
    app_mix_mode_t *mix_mode = (app_mix_mode_t *)parameter;

    bool a2dp_changed = app_hearing_aid_storage_set_a2dp_mix_switch(mix_mode->a2dp_mix_mode_switch);
    bool sco_changed = app_hearing_aid_storage_set_sco_mix_switch(mix_mode->sco_mix_mode_switch);
    bool vp_changed = app_hearing_aid_storage_set_vp_mix_switch(mix_mode->vp_mix_mode_switch);

    uint8_t switch_changed = false;

    if ((a2dp_changed == true)
            || (sco_changed == true)
            || (vp_changed == true)
            || (app_ha_utils_context.ha_mix_mode.a2dp_drc_switch_l != mix_mode->a2dp_drc_switch_l)
            || (app_ha_utils_context.ha_mix_mode.a2dp_drc_switch_r != mix_mode->a2dp_drc_switch_r)
            || (app_ha_utils_context.ha_mix_mode.sco_drc_switch_l != mix_mode->sco_drc_switch_l)
            || (app_ha_utils_context.ha_mix_mode.sco_drc_switch_r != mix_mode->sco_drc_switch_r)
            || (app_ha_utils_context.ha_mix_mode.vp_drc_switch_l != mix_mode->vp_drc_switch_l)
            || (app_ha_utils_context.ha_mix_mode.vp_drc_switch_r != mix_mode->vp_drc_switch_r)) {
        mix_mode_changed = true;

        if ((a2dp_changed == true)
            || (sco_changed == true)
            || (vp_changed == true)) {
            switch_changed = true;
        }
    }

    memcpy(&(app_ha_utils_context.ha_mix_mode), parameter, sizeof(psap_scenario_mix_mode_t));

    if ((status == AUDIO_PSAP_STATUS_SUCCESS) && (mix_mode_changed == true)) {
        if (switch_changed == true) {
            app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_MIX_TABLE_SWITCH, false, false);
        } else {
            app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_MIX_TABLE_DRC, false, false);
        }
    }

    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_hearing_tuning_mode_switch(uint8_t *parameter)
{
#if 0
    if (app_hearing_aid_utils_is_mp_test_mode() == true) {
        return false;
    }
#endif
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    if (role == AUDIO_PSAP_DEVICE_ROLE_MAX) {
        return false;
    }

    uint8_t mode_value = parameter[0];

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_hearing_tuning_mode_switch] mode : %d, role : %d",
                     2,
                     mode_value,
                     role);

    audio_psap_status_t status = AUDIO_PSAP_STATUS_SUCCESS;
    if ((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (APP_HA_IS_ENABLE(mode_value, 0))) {
        status = audio_anc_psap_control_set_tuning_mode(mode_value);
    }
    if ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (APP_HA_IS_ENABLE(mode_value, 1))) {
        status = audio_anc_psap_control_set_tuning_mode(mode_value);
    }

    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_mp_test_mode_switch(uint8_t *parameter)
{
    uint8_t enable = parameter[0];

    if (app_hearing_aid_utils_is_hearing_tuning_mode() == true) {
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_mp_test_mode_switch] enable parameter : %d",
                        1,
                        enable);

    app_hearing_aid_utils_notify((enable == APP_HA_MP_TEST_MODE_SWITCH_ON) ? APP_HEARING_AID_UTILS_NOTIFY_TO_ENTER_MP_TEST_MODE : APP_HEARING_AID_UTILS_NOTIFY_TO_EXIT_MP_TEST_MODE,
                                    0,
                                    NULL,
                                    0);

    return true;

#if 0
    audio_psap_status_t set_status = AUDIO_PSAP_STATUS_FAIL;
    audio_psap_status_t enable_status = AUDIO_PSAP_STATUS_FAIL;

    if (enable == APP_HA_MP_TEST_MODE_SWITCH_ON) {
        enable_status = audio_anc_psap_control_enable();
    }

    if (((enable == APP_HA_MP_TEST_MODE_SWITCH_ON) && (enable_status == AUDIO_PSAP_STATUS_SUCCESS))
        || (enable == APP_HA_MP_TEST_MODE_SWITCH_OFF)) {
        set_status = audio_anc_psap_control_set_mp_test_mode(enable);
    }
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_mp_test_mode_switch] enable parameter : %d, enable HA status : %d, set MP test mode status : %d",
                     3,
                     enable,
                     enable_status,
                     set_status);

    if ((enable == APP_HA_MP_TEST_MODE_SWITCH_OFF) && (set_status == AUDIO_PSAP_STATUS_SUCCESS)) {
        app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_TO_CONTROL_HA,
                                     0,
                                     NULL,
                                     0);
    }

    return (set_status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
#endif
}

bool app_hearing_aid_utils_set_restore_setting(uint8_t *parameter)
{
    if ((app_hearing_aid_utils_is_hearing_tuning_mode() == true)
        || (app_hearing_aid_utils_is_mp_test_mode() == true)) {
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_restore_setting] restore all settings", 0);

    audio_psap_status_t status = audio_anc_psap_control_set_restore_setting(parameter[0]);

    bool need_update = false;
    app_hearing_aid_storage_restore(&need_update);

    if (need_update == true) {
        app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_RACE_CMD, true, false);
    }

    app_hearing_aid_utils_check_master_mic_channel_changed(true);

    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_mute(uint8_t *parameter)
{
    audio_psap_status_t status = audio_anc_psap_control_set_mute(parameter[0], parameter[1]);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_howling_detection(uint8_t *parameter)
{
    if ((app_hearing_aid_utils_is_hearing_tuning_mode() == true)
        || (app_hearing_aid_utils_is_mp_test_mode() == true)) {
        return false;
    }

    audio_psap_status_t status = audio_anc_psap_control_set_howling_detection((psap_how_det_t *)parameter);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_mpo_adjustment(uint8_t *parameter)
{
    audio_psap_status_t status = audio_anc_psap_control_set_mpo_adjustment((ha_mpo_adjust_t *)parameter);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_in_ear_detection_switch(uint8_t *parameter)
{
    app_hearing_aid_storage_set_in_ear_detection_switch((bool)(parameter[0]));

    app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_IN_EAR_DETECTION_SWITCH_CHANGE, 0, NULL, 0);
    return true;
}

bool app_hearing_aid_utils_set_passthrough_switch(uint8_t *parameter)
{
    audio_psap_status_t status = audio_anc_psap_control_set_passthrough_switch(parameter[0], parameter[1]);
    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_set_mic_channel(uint8_t *parameter)
{
    audio_psap_status_t status = audio_anc_psap_control_set_mic_channel(parameter[0]);

    if (status == AUDIO_PSAP_STATUS_SUCCESS) {
        app_hearing_aid_utils_check_master_mic_channel_changed(false);
        return true;
    }

    return false;
}

typedef struct {
    uint8_t                 channel;
    uint16_t                trial_run_id;
    uint16_t                trial_run_param_len;
} __attribute__((packed)) app_hearing_aid_trial_run_payload_t;

bool app_hearing_aid_utils_set_trial_run(uint8_t *parameter)
{
    audio_psap_status_t status = AUDIO_PSAP_STATUS_SUCCESS;
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    app_hearing_aid_trial_run_payload_t *trial_run_payload = (app_hearing_aid_trial_run_payload_t *)parameter;

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_trial_run] role : %d, target_role : %d, ID : 0x%04x, length : 0x%04x",
                        4,
                        role,
                        trial_run_payload->channel,
                        trial_run_payload->trial_run_id,
                        trial_run_payload->trial_run_param_len);

    if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (trial_run_payload->channel == APP_HA_CHANNEL_L))
        || ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (trial_run_payload->channel == APP_HA_CHANNEL_R))
        || (trial_run_payload->channel == 0x03)) {
        status = audio_anc_psap_control_trial_run(trial_run_payload->trial_run_id,
                                                    trial_run_payload->trial_run_param_len,
                                                    parameter + sizeof(app_hearing_aid_trial_run_payload_t));
    }

    return (status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

extern audio_psap_status_t audio_anc_psap_control_set_mic_cal_mode(U8 *mic_cal_mode);

bool app_hearing_aid_utils_set_mic_calibration_mode(uint8_t *parameter)
{
    audio_psap_status_t psap_status = audio_anc_psap_control_set_mic_cal_mode(parameter);

    return (psap_status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;
}

//#if (defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE))
#ifdef AIR_DAC_MODE_RUNTIME_CHANGE
bool app_hearing_aid_utils_set_hearing_test_mode(uint8_t *parameter)
{
#ifdef AIR_DAC_MODE_RUNTIME_CHANGE
    bool enable = parameter[0];

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_hearing_test_mode] control hearing test mode : %d, current mode : %d",
                        2,
                        enable,
                        app_ha_utils_context.is_hearing_test_mode);

    if (enable == app_ha_utils_context.is_hearing_test_mode) {
        return true;
    }

    app_ha_utils_context.is_hearing_test_mode = enable;
#endif
#if 0
    if (enable == true) {
        // enable hearing test mode

        app_ha_utils_context.dac_mode_before_hearing_test_mode = hal_audio_status_get_dac_mode();
        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_hearing_test_mode] Enter hearing test mode, DAC mode : 0x%02x",
                            1,
                            app_ha_utils_context.dac_mode_before_hearing_test_mode);

        if (app_ha_utils_context.dac_mode_before_hearing_test_mode == 0x04) {
            bool change_dac_result = hal_audio_status_change_dac_mode(0x02);
            if (change_dac_result == true) {
                app_ha_utils_context.is_hearing_test_mode = true;
            } else {
                APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_hearing_test_mode] Enter hearing test mode failed", 0);
                return false;
            }
        } else {
            app_ha_utils_context.is_hearing_test_mode = true;
        }
    } else {
        // disable hearing test mode
        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_hearing_test_mode] Exit hearing test mode, DAC mode : 0x%02x",
                            1,
                            app_ha_utils_context.dac_mode_before_hearing_test_mode);

        if (app_ha_utils_context.dac_mode_before_hearing_test_mode == 0x04) {
            bool change_dac_result = hal_audio_status_change_dac_mode(0x04);
            if (change_dac_result == true) {
                app_ha_utils_context.is_hearing_test_mode = false;
            } else {
                APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_set_hearing_test_mode] Exit hearing test mode failed", 0);
                return false;
            }
        } else {
            app_ha_utils_context.is_hearing_test_mode = false;
        }
    }
#endif

//#ifdef AIR_DAC_MODE_RUNTIME_CHANGE
    hal_audio_status_send_update_dac_mode_event_to_am(HAL_AUDIO_HA_DAC_FLAG_HEARING_TEST, app_ha_utils_context.is_hearing_test_mode);
//#endif /* AIR_DAC_MODE_RUNTIME_CHANGE */

    return true;
}
#endif

uint32_t app_hearing_aid_utils_get_mic_calibration_data_combine_response_len()
{
    return APP_HA_MIC_CALIBRATION_DATA_COMBINE_LEN;
}

uint32_t app_hearing_aid_utils_get_speaker_reference_combine_response_len()
{
    return APP_HA_SPEAKER_REFERENCE_COMBINE_LEN;
}

#if 0
bool app_hearing_aid_utils_get_level_index_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    app_hearing_aid_level_index_t *local_index = (app_hearing_aid_level_index_t *)local;
    app_hearing_aid_level_index_t *remote_index = (app_hearing_aid_level_index_t *)remote;
    app_hearing_aid_level_index_t *response_index = (app_hearing_aid_level_index_t *)response;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        response_index->l_level_index = local_index->l_level_index;
        response_index->r_level_index = remote_index->r_level_index;
    } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
        response_index->l_level_index = remote_index->l_level_index;
        response_index->r_level_index = local_index->r_level_index;
    } else {
        *response_len = 0;
        return false;
    }
    *response_len = 2;
    return true;
}

bool app_hearing_aid_utils_get_volume_index_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    app_hearing_aid_vol_index_t *local_index = (app_hearing_aid_vol_index_t *)local;
    app_hearing_aid_vol_index_t *remote_index = (app_hearing_aid_vol_index_t *)remote;
    app_hearing_aid_vol_index_t *response_index = (app_hearing_aid_vol_index_t *)response;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        response_index->l_vol_index = local_index->l_vol_index;
        response_index->r_vol_index = remote_index->r_vol_index;
    } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
        response_index->l_vol_index = remote_index->l_vol_index;
        response_index->r_vol_index = local_index->r_vol_index;
    } else {
        *response_len = 0;
        return false;
    }
    *response_len = 2;
    return true;
}

bool app_hearing_aid_utils_get_specific_mode_table_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    psap_mode_table_t *local_mode = (psap_mode_table_t *)(local + 1);
    psap_mode_table_t *remote_mode = (psap_mode_table_t *)(remote + 1);
    psap_mode_table_t *rsp_mode = (psap_mode_table_t *)(response + 1);

    response[0] = local[0];

    rsp_mode->BeamForming_switch = local_mode->BeamForming_switch;
    rsp_mode->NR_level = local_mode->NR_level;
    rsp_mode->NR_switch = local_mode->NR_switch;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        rsp_mode->MFA_switch_L = local_mode->MFA_switch_L;
        rsp_mode->MFA_switch_R = remote_mode->MFA_switch_R;
        rsp_mode->LowCut_switch_L = local_mode->LowCut_switch_L;
        rsp_mode->LowCut_switch_R = remote_mode->LowCut_switch_R;
    } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
        rsp_mode->MFA_switch_L = remote_mode->MFA_switch_L;
        rsp_mode->MFA_switch_R = local_mode->MFA_switch_R;
        rsp_mode->LowCut_switch_L = remote_mode->LowCut_switch_L;
        rsp_mode->LowCut_switch_R = local_mode->LowCut_switch_R;
    } else {
        *response_len = 0;
        return false;
    }
    *response_len = 1 + sizeof(psap_mode_table_t);
    return true;
}

bool app_hearing_aid_utils_get_afc_config_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    psap_afc_config_t *local_config = (psap_afc_config_t *)local;
    psap_afc_config_t *remote_config = (psap_afc_config_t *)remote;
    psap_afc_config_t *rsp_config = (psap_afc_config_t *)response;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        rsp_config->afc_ctrl_switch_l = local_config->afc_ctrl_switch_l;
        rsp_config->afc_ctrl_switch_r = remote_config->afc_ctrl_switch_r;
    } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
        rsp_config->afc_ctrl_switch_l = remote_config->afc_ctrl_switch_l;
        rsp_config->afc_ctrl_switch_r = local_config->afc_ctrl_switch_r;
    } else {
        *response_len = 0;
        return false;
    }

    *response_len = sizeof(psap_afc_config_t);
    return true;
}

bool app_hearing_aid_utils_get_inr_config_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    ha_inr_config_t *local_config = (ha_inr_config_t *)local;
    ha_inr_config_t *remote_config = (ha_inr_config_t *)remote;
    ha_inr_config_t *rsp_config = (ha_inr_config_t *)response;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        rsp_config->INR_switch_l = local_config->INR_switch_l;
        rsp_config->INR_sensitivity_l = local_config->INR_sensitivity_l;
        rsp_config->INR_strength_l = local_config->INR_strength_l;

        rsp_config->INR_switch_r = remote_config->INR_switch_r;
        rsp_config->INR_sensitivity_r = remote_config->INR_sensitivity_r;
        rsp_config->INR_strength_r = remote_config->INR_strength_r;
    } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
        rsp_config->INR_switch_l = remote_config->INR_switch_l;
        rsp_config->INR_sensitivity_l = remote_config->INR_sensitivity_l;
        rsp_config->INR_strength_l = remote_config->INR_strength_l;

        rsp_config->INR_switch_r = local_config->INR_switch_r;
        rsp_config->INR_sensitivity_r = local_config->INR_sensitivity_r;
        rsp_config->INR_strength_r = local_config->INR_strength_r;
    } else {
        *response_len = 0;
        return false;
    }
    *response_len = sizeof(ha_inr_config_t);
    return true;
}

bool app_hearing_aid_utils_get_user_eq_switch_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    psap_user_eq_switch_t *local_config = (psap_user_eq_switch_t *)local;
    psap_user_eq_switch_t *remote_config = (psap_user_eq_switch_t *)remote;
    psap_user_eq_switch_t *rsp_config = (psap_user_eq_switch_t *)response;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        rsp_config->ha_user_eq_switch_l = local_config->ha_user_eq_switch_l;
        rsp_config->ha_user_eq_switch_r = remote_config->ha_user_eq_switch_r;
    } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
        rsp_config->ha_user_eq_switch_l = remote_config->ha_user_eq_switch_l;
        rsp_config->ha_user_eq_switch_r = local_config->ha_user_eq_switch_r;
    } else {
        *response_len = 0;
        return false;
    }
    *response_len = sizeof(psap_user_eq_switch_t);
    return true;
}

bool app_hearing_aid_utils_get_user_eq_gain_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    psap_usr_eq_para_t *local_config = (psap_usr_eq_para_t *)local;
    psap_usr_eq_para_t *remote_config = (psap_usr_eq_para_t *)remote;
    psap_usr_eq_para_t *rsp_config = (psap_usr_eq_para_t *)response;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        rsp_config->psap_user_eq_overall_l = local_config->psap_user_eq_overall_l;
        memcpy(rsp_config->psap_user_eq_band_l, local_config->psap_user_eq_band_l, HA_BAND_NUM);
        rsp_config->psap_user_eq_overall_r = remote_config->psap_user_eq_overall_r;
        memcpy(rsp_config->psap_user_eq_band_r, remote_config->psap_user_eq_band_r, HA_BAND_NUM);
    } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
        rsp_config->psap_user_eq_overall_l = remote_config->psap_user_eq_overall_l;
        memcpy(rsp_config->psap_user_eq_band_l, remote_config->psap_user_eq_band_l, HA_BAND_NUM);
        rsp_config->psap_user_eq_overall_r = local_config->psap_user_eq_overall_r;
        memcpy(rsp_config->psap_user_eq_band_r, local_config->psap_user_eq_band_r, HA_BAND_NUM);
    } else {
        *response_len = 0;
        return false;
    }
    *response_len = sizeof(psap_usr_eq_para_t);
    return true;
}
#endif

bool app_hearing_aid_utils_combine_speaker_reference_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    psap_test_spk_ref_t *local_config = (psap_test_spk_ref_t *)local;
    psap_test_spk_ref_t *remote_config = (psap_test_spk_ref_t *)remote;
    psap_test_spk_ref_t *rsp_config = (psap_test_spk_ref_t *)response;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        rsp_config->test_spk_ref_L_64       =  local_config->test_spk_ref_L_64;
        rsp_config->test_spk_ref_L_125      =  local_config->test_spk_ref_L_125;
        rsp_config->test_spk_ref_L_250      =  local_config->test_spk_ref_L_250;
        rsp_config->test_spk_ref_L_500      =  local_config->test_spk_ref_L_500;
        rsp_config->test_spk_ref_L_1000     =  local_config->test_spk_ref_L_1000;
        rsp_config->test_spk_ref_L_2000     =  local_config->test_spk_ref_L_2000;
        rsp_config->test_spk_ref_L_4000     =  local_config->test_spk_ref_L_4000;
        rsp_config->test_spk_ref_L_6000     =  local_config->test_spk_ref_L_6000;
        rsp_config->test_spk_ref_L_8000     =  local_config->test_spk_ref_L_8000;
        rsp_config->test_spk_ref_L_12000    =  local_config->test_spk_ref_L_12000;

        rsp_config->test_spk_ref_R_64       =  remote_config->test_spk_ref_L_64;
        rsp_config->test_spk_ref_R_125      =  remote_config->test_spk_ref_L_125;
        rsp_config->test_spk_ref_R_250      =  remote_config->test_spk_ref_L_250;
        rsp_config->test_spk_ref_R_500      =  remote_config->test_spk_ref_L_500;
        rsp_config->test_spk_ref_R_1000     =  remote_config->test_spk_ref_L_1000;
        rsp_config->test_spk_ref_R_2000     =  remote_config->test_spk_ref_L_2000;
        rsp_config->test_spk_ref_R_4000     =  remote_config->test_spk_ref_L_4000;
        rsp_config->test_spk_ref_R_6000     =  remote_config->test_spk_ref_L_6000;
        rsp_config->test_spk_ref_R_8000     =  remote_config->test_spk_ref_L_8000;
        rsp_config->test_spk_ref_R_12000    =  remote_config->test_spk_ref_L_12000;
#if 0
        memcpy(rsp_config, local_config, 10);
        memcpy(rsp_config + 10, remote_config, 10);
#endif
    } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
#if 0
        memcpy(rsp_config, remote_config, 10);
        memcpy(rsp_config + 10, local_config, 10);
#endif
        rsp_config->test_spk_ref_L_64       =  remote_config->test_spk_ref_L_64;
        rsp_config->test_spk_ref_L_125      =  remote_config->test_spk_ref_L_125;
        rsp_config->test_spk_ref_L_250      =  remote_config->test_spk_ref_L_250;
        rsp_config->test_spk_ref_L_500      =  remote_config->test_spk_ref_L_500;
        rsp_config->test_spk_ref_L_1000     =  remote_config->test_spk_ref_L_1000;
        rsp_config->test_spk_ref_L_2000     =  remote_config->test_spk_ref_L_2000;
        rsp_config->test_spk_ref_L_4000     =  remote_config->test_spk_ref_L_4000;
        rsp_config->test_spk_ref_L_6000     =  remote_config->test_spk_ref_L_6000;
        rsp_config->test_spk_ref_L_8000     =  remote_config->test_spk_ref_L_8000;
        rsp_config->test_spk_ref_L_12000    =  remote_config->test_spk_ref_L_12000;

        rsp_config->test_spk_ref_R_64       =  local_config->test_spk_ref_L_64;
        rsp_config->test_spk_ref_R_125      =  local_config->test_spk_ref_L_125;
        rsp_config->test_spk_ref_R_250      =  local_config->test_spk_ref_L_250;
        rsp_config->test_spk_ref_R_500      =  local_config->test_spk_ref_L_500;
        rsp_config->test_spk_ref_R_1000     =  local_config->test_spk_ref_L_1000;
        rsp_config->test_spk_ref_R_2000     =  local_config->test_spk_ref_L_2000;
        rsp_config->test_spk_ref_R_4000     =  local_config->test_spk_ref_L_4000;
        rsp_config->test_spk_ref_R_6000     =  local_config->test_spk_ref_L_6000;
        rsp_config->test_spk_ref_R_8000     =  local_config->test_spk_ref_L_8000;
        rsp_config->test_spk_ref_R_12000    =  local_config->test_spk_ref_L_12000;
    } else {
        *response_len = 0;
        return false;
    }
    *response_len = sizeof(psap_test_spk_ref_t);
    return true;

}

#if 0
bool app_hearing_aid_utils_get_mix_mode_total_setting_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{

    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    psap_scenario_mix_mode_t *local_config = (psap_scenario_mix_mode_t *)local;
    psap_scenario_mix_mode_t *remote_config = (psap_scenario_mix_mode_t *)remote;
    psap_scenario_mix_mode_t *rsp_config = (psap_scenario_mix_mode_t *)response;

    rsp_config->a2dp_mix_mode_switch = local_config->a2dp_mix_mode_switch;
    rsp_config->sco_mix_mode_switch = local_config->sco_mix_mode_switch;
    rsp_config->vp_mix_mode_switch = local_config->vp_mix_mode_switch;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        rsp_config->a2dp_drc_switch_l = local_config->a2dp_drc_switch_l;
        rsp_config->a2dp_mfa_switch_l = local_config->a2dp_mfa_switch_l;
        rsp_config->a2dp_mix_mode_psap_gain_l = local_config->a2dp_mix_mode_psap_gain_l;
        rsp_config->sco_drc_switch_l = local_config->sco_drc_switch_l;
        rsp_config->sco_mfa_switch_l = local_config->sco_mfa_switch_l;
        rsp_config->sco_mix_mode_psap_gain_l = local_config->sco_mix_mode_psap_gain_l;
        rsp_config->vp_drc_switch_l = local_config->vp_drc_switch_l;
        rsp_config->vp_mfa_switch_l = local_config->vp_mfa_switch_l;
        rsp_config->vp_mix_mode_psap_gain_l = local_config->vp_mix_mode_psap_gain_l;

        rsp_config->a2dp_drc_switch_r = remote_config->a2dp_drc_switch_r;
        rsp_config->a2dp_mfa_switch_r = remote_config->a2dp_mfa_switch_r;
        rsp_config->a2dp_mix_mode_psap_gain_r = remote_config->a2dp_mix_mode_psap_gain_r;
        rsp_config->sco_drc_switch_r = remote_config->sco_drc_switch_r;
        rsp_config->sco_mfa_switch_r = remote_config->sco_mfa_switch_r;
        rsp_config->sco_mix_mode_psap_gain_r = remote_config->sco_mix_mode_psap_gain_r;
        rsp_config->vp_drc_switch_r = remote_config->vp_drc_switch_r;
        rsp_config->vp_mfa_switch_r = remote_config->vp_mfa_switch_r;
        rsp_config->vp_mix_mode_psap_gain_r = remote_config->vp_mix_mode_psap_gain_r;
    } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
        rsp_config->a2dp_drc_switch_l = remote_config->a2dp_drc_switch_l;
        rsp_config->a2dp_mfa_switch_l = remote_config->a2dp_mfa_switch_l;
        rsp_config->a2dp_mix_mode_psap_gain_l = remote_config->a2dp_mix_mode_psap_gain_l;
        rsp_config->sco_drc_switch_l = remote_config->sco_drc_switch_l;
        rsp_config->sco_mfa_switch_l = remote_config->sco_mfa_switch_l;
        rsp_config->sco_mix_mode_psap_gain_l = remote_config->sco_mix_mode_psap_gain_l;
        rsp_config->vp_drc_switch_l = remote_config->vp_drc_switch_l;
        rsp_config->vp_mfa_switch_l = remote_config->vp_mfa_switch_l;
        rsp_config->vp_mix_mode_psap_gain_l = remote_config->vp_mix_mode_psap_gain_l;

        rsp_config->a2dp_drc_switch_r = local_config->a2dp_drc_switch_r;
        rsp_config->a2dp_mfa_switch_r = local_config->a2dp_mfa_switch_r;
        rsp_config->a2dp_mix_mode_psap_gain_r = local_config->a2dp_mix_mode_psap_gain_r;
        rsp_config->sco_drc_switch_r = local_config->sco_drc_switch_r;
        rsp_config->sco_mfa_switch_r = local_config->sco_mfa_switch_r;
        rsp_config->sco_mix_mode_psap_gain_r = local_config->sco_mix_mode_psap_gain_r;
        rsp_config->vp_drc_switch_r = local_config->vp_drc_switch_r;
        rsp_config->vp_mfa_switch_r = local_config->vp_mfa_switch_r;
        rsp_config->vp_mix_mode_psap_gain_r = local_config->vp_mix_mode_psap_gain_r;
    } else {
        *response_len = 0;
        return false;
    }
    *response_len = sizeof(psap_scenario_mix_mode_t);
    return true;

}
#endif

bool app_hearing_aid_utils_get_hearing_tuning_mode_switch_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
    app_hearing_aid_hearing_tuning_mode_t *local_config = (app_hearing_aid_hearing_tuning_mode_t *)local;
    app_hearing_aid_hearing_tuning_mode_t *remote_config = (app_hearing_aid_hearing_tuning_mode_t *)remote;
    app_hearing_aid_hearing_tuning_mode_t *rsp_config = (app_hearing_aid_hearing_tuning_mode_t *)response;

    *response_len = 0;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        rsp_config->l_switch = local_config->l_switch;
        rsp_config->r_switch = remote_config->r_switch;
    } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
        rsp_config->l_switch = remote_config->l_switch;
        rsp_config->r_switch = local_config->r_switch;
    } else {
        return false;
    }
    *response_len = sizeof(app_hearing_aid_hearing_tuning_mode_t);

    return true;
}

bool app_hearing_aid_utils_get_mic_calibration_data_combine_response(bool is_local_response,
                                                                        uint8_t *response,
                                                                        uint16_t response_len,
                                                                        uint8_t *combine_response,
                                                                        uint16_t *combine_response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();

    if (*combine_response_len < APP_HA_MIC_CALIBRATION_DATA_COMBINE_LEN) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_mic_calibration_data_combine_response] combine_response_len error : %d - %d",
                            2,
                            *combine_response_len,
                            APP_HA_MIC_CALIBRATION_DATA_COMBINE_LEN);
        return false;
    }

    if (response_len != APP_HEARING_AID_RESPONSE_MAX_LEN) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_mic_calibration_data_combine_response] response_len error : %d - %d",
                            2,
                            response_len,
                            APP_HA_MIC_CALIBRATION_DATA_COMBINE_LEN);
        return false;
    }

    // APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_mic_calibration_data_combine_response] role : %d, is_local_response : %d, rsp_exist : %d",
    //                          3,
    //                          role,
    //                          is_local_response,
    //                          app_ha_mic_calibration_data_combine_response.rsp_exist);

    if (app_ha_mic_calibration_data_combine_response.rsp_exist == false) {
        app_ha_mic_calibration_data_combine_response.rsp_exist = true;
        memcpy(app_ha_mic_calibration_data_combine_response.response, response, response_len);

        return false;
    } else {
        if ((role == AUDIO_PSAP_DEVICE_ROLE_LEFT)
                || (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT)) {
            if (((role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (is_local_response == true))
                || ((role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (is_local_response == false))) {
                memcpy(combine_response, response, response_len);

                memcpy(combine_response + APP_HEARING_AID_RESPONSE_MAX_LEN,
                            app_ha_mic_calibration_data_combine_response.response,
                            APP_HEARING_AID_RESPONSE_MAX_LEN);
            } else {
                memcpy(combine_response,
                            app_ha_mic_calibration_data_combine_response.response,
                            APP_HEARING_AID_RESPONSE_MAX_LEN);

                memcpy(combine_response + APP_HEARING_AID_RESPONSE_MAX_LEN, response, response_len);
            }
        } else {
            memset(&app_ha_mic_calibration_data_combine_response, 0, sizeof(app_hearing_aid_utils_mic_calibration_data_combine_response_t));
            return false;
        }
    }

    *combine_response_len = APP_HA_MIC_CALIBRATION_DATA_COMBINE_LEN;
    memset(&app_ha_mic_calibration_data_combine_response, 0, sizeof(app_hearing_aid_utils_mic_calibration_data_combine_response_t));

    return true;
}

bool app_hearing_aid_utils_get_speaker_reference_combine_response(bool is_local_response, uint8_t *response, uint16_t response_len, uint8_t *combine_response, uint16_t *combine_response_len)
{

    if (*combine_response_len < APP_HA_SPEAKER_REFERENCE_COMBINE_LEN) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_speaker_reference_combine_response] combine_response_len error : %d - %d",
                            2,
                            *combine_response_len,
                            APP_HA_SPEAKER_REFERENCE_COMBINE_LEN);
        return false;
    }

    if (response_len != APP_HA_SPEAKER_REFERENCE_COMBINE_LEN) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_speaker_reference_combine_response] response_len error : %d - %d",
                            2,
                            response_len,
                            APP_HA_SPEAKER_REFERENCE_COMBINE_LEN);
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_speaker_reference_combine_response] is_rsp_exist : %d, is_local_response : %d",
                            2,
                            app_ha_speaker_reference_combine_response.is_rsp_exist,
                            is_local_response);

    if (app_ha_speaker_reference_combine_response.is_rsp_exist == false) {
        app_ha_speaker_reference_combine_response.is_rsp_exist = true;
        app_ha_speaker_reference_combine_response.is_local = is_local_response;
        memcpy(&app_ha_speaker_reference_combine_response.spk_ref, response, response_len);

        return false;
    } else {
        if (app_ha_speaker_reference_combine_response.is_local == true) {
            app_hearing_aid_utils_combine_speaker_reference_response((uint8_t *)(&(app_ha_speaker_reference_combine_response.spk_ref)),
                                                                        APP_HA_SPEAKER_REFERENCE_COMBINE_LEN,
                                                                        response,
                                                                        response_len,
                                                                        combine_response,
                                                                        combine_response_len);
        } else {
            app_hearing_aid_utils_combine_speaker_reference_response(response,
                                                                        response_len,
                                                                        (uint8_t *)(&(app_ha_speaker_reference_combine_response.spk_ref)),
                                                                        APP_HA_SPEAKER_REFERENCE_COMBINE_LEN,
                                                                        combine_response,
                                                                        combine_response_len);
        }

        app_ha_speaker_reference_combine_response.is_local = false;
        app_ha_speaker_reference_combine_response.is_rsp_exist = false;
        memset(&app_ha_speaker_reference_combine_response.spk_ref, 0, APP_HA_SPEAKER_REFERENCE_COMBINE_LEN);
    }

    return true;
}

#if 0
bool app_hearing_aid_utils_get_feedback_detection_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    *response = app_ha_utils_context.feedback_detection_channel;
    *response_len = sizeof(uint8_t);
    return true;
}

bool app_hearing_aid_utils_get_pure_tone_generator_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();

    app_hearing_aid_pure_tone_generator_t *local_response = (app_hearing_aid_pure_tone_generator_t *)local;
    app_hearing_aid_pure_tone_generator_t *remote_response = (app_hearing_aid_pure_tone_generator_t *)remote;
    app_hearing_aid_pure_tone_generator_t *combine_response = (app_hearing_aid_pure_tone_generator_t *)response;

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        combine_response->l_switch = local_response->l_switch;
        combine_response->l_freq = local_response->l_freq;
        combine_response->l_dbfs = local_response->l_dbfs;

        combine_response->r_switch = remote_response->r_switch;
        combine_response->r_freq = remote_response->r_freq;
        combine_response->r_dbfs = remote_response->r_freq;
    } else {
        combine_response->l_switch = remote_response->l_switch;
        combine_response->l_freq = remote_response->l_freq;
        combine_response->l_dbfs = remote_response->l_dbfs;

        combine_response->r_switch = local_response->r_switch;
        combine_response->r_freq = local_response->r_freq;
        combine_response->r_dbfs = local_response->r_freq;
    }

    *response_len = sizeof(app_hearing_aid_pure_tone_generator_t);

    return true;
}

bool app_hearing_aid_utils_get_passthrough_switch_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len)
{
    app_hearing_aid_passthrough_switch_t *switch_response = (app_hearing_aid_passthrough_switch_t *)response;
    app_hearing_aid_passthrough_switch_t *local_response = (app_hearing_aid_passthrough_switch_t *)local;
    app_hearing_aid_passthrough_switch_t *remote_response = (app_hearing_aid_passthrough_switch_t *)remote;
    audio_psap_device_role_t role = app_hearing_aid_utils_get_role();

    if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
        switch_response->l_passthrough_switch = local_response->l_passthrough_switch;
        switch_response->r_passthrough_switch = remote_response->r_passthrough_switch;
    } else {
        switch_response->l_passthrough_switch = remote_response->l_passthrough_switch;
        switch_response->r_passthrough_switch = local_response->r_passthrough_switch;
    }
    *response_len = sizeof(app_hearing_aid_passthrough_switch_t);

    return true;
}
#endif

static void app_hearing_aid_utils_common_notify_handler(uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
    memcpy(notify_data, data, data_len);
    *notify_data_len = data_len;
}

bool app_hearing_aid_utils_ha_switch_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
#if 0
    notify_data[0] = data[0];
    *notify_data_len = sizeof(uint8_t);
#endif
    app_hearing_aid_utils_common_notify_handler(data, data_len, notify_data, notify_data_len);
    return true;
}

bool app_hearing_aid_utils_level_index_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
    app_hearing_aid_level_index_t *level_index = (app_hearing_aid_level_index_t *)notify_data;
    app_hearing_aid_level_index_t *data_index = (app_hearing_aid_level_index_t *)data;
    audio_psap_device_role_t local_role = app_hearing_aid_utils_get_role();

    uint8_t l_index = 0;
    uint8_t r_index = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_level_index(&l_index, &r_index);

    if (role == local_role) {
        memcpy(level_index, data_index, sizeof(app_hearing_aid_level_index_t));
    } else {
        if (status != AUDIO_PSAP_STATUS_SUCCESS) {
            APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_level_index_notify] Failed to get local level index : %d",
                             1,
                             status);
            return false;
        }
        if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
            level_index->l_level_index = data_index->l_level_index;
            level_index->r_level_index = r_index;
        } else {
            level_index->l_level_index = l_index;
            level_index->r_level_index = data_index->r_level_index;
        }
    }
    *notify_data_len = sizeof(app_hearing_aid_level_index_t);
    return true;
}

bool app_hearing_aid_utils_volume_index_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
    app_hearing_aid_vol_index_t *level_index = (app_hearing_aid_vol_index_t *)notify_data;
    app_hearing_aid_vol_index_t *data_index = (app_hearing_aid_vol_index_t *)data;
    audio_psap_device_role_t local_role = app_hearing_aid_utils_get_role();

    uint8_t l_index = 0;
    uint8_t r_index = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_volume_index(&l_index, &r_index);

    if (role == local_role) {
        memcpy(level_index, data_index, sizeof(app_hearing_aid_vol_index_t));
    } else {
        if (status != AUDIO_PSAP_STATUS_SUCCESS) {
            APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_volume_index_notify] Failed to get local volume index : %d",
                             1,
                             status);
            return false;
        }
        if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
            level_index->l_vol_index = data_index->l_vol_index;
            level_index->r_vol_index = r_index;
        } else {
            level_index->l_vol_index = l_index;
            level_index->r_vol_index = data_index->r_vol_index;
        }
    }
    *notify_data_len = sizeof(app_hearing_aid_level_index_t);
    return true;
}

bool app_hearing_aid_utils_mode_index_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
#if 0
    notify_data[0] = data[0];
    *notify_data_len = sizeof(uint8_t);
#endif
    app_hearing_aid_utils_common_notify_handler(data, data_len, notify_data, notify_data_len);
    return true;
}

bool app_hearing_aid_utils_aea_configuration_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
#if 0
    memcpy(notify_data, data, sizeof(psap_aea_config_t));
    *notify_data_len = sizeof(psap_aea_config_t);
#endif
    app_hearing_aid_utils_common_notify_handler(data, data_len, notify_data, notify_data_len);
    return true;
}

bool app_hearing_aid_utils_beam_forming_settings_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
#if 0
    memcpy(notify_data, data, sizeof(psap_bf_config_t));
    *notify_data_len = sizeof(psap_bf_config_t);
#endif
    app_hearing_aid_utils_common_notify_handler(data, data_len, notify_data, notify_data_len);
    return true;
}

bool app_hearing_aid_utils_speaker_reference_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
    psap_test_spk_ref_t *data_spk_ref = (psap_test_spk_ref_t *)data;
    psap_test_spk_ref_t local_spk_ref = {0};
    audio_psap_device_role_t local_role = app_hearing_aid_utils_get_role();

    if (role == local_role) {
        memcpy(notify_data, data_spk_ref, sizeof(psap_test_spk_ref_t));
        *notify_data_len = sizeof(psap_test_spk_ref_t);
    } else {
        audio_psap_status_t status = audio_anc_psap_control_get_speaker_reference(&local_spk_ref);

        if (status != AUDIO_PSAP_STATUS_SUCCESS) {
            APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_speaker_reference_notify] Failed to get local speaker reference : %d",
                                1,
                                status);
            return false;
        }
        app_hearing_aid_utils_combine_speaker_reference_response((uint8_t *)&local_spk_ref,
                                                                     sizeof(psap_test_spk_ref_t),
                                                                     data,
                                                                     sizeof(psap_test_spk_ref_t),
                                                                     notify_data,
                                                                     notify_data_len);
    }

    return true;
}

bool app_hearing_aid_utils_hearing_tuning_mode_switch_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
    app_hearing_aid_hearing_tuning_mode_t *data_spk_ref = (app_hearing_aid_hearing_tuning_mode_t *)data;
    audio_psap_device_role_t local_role = app_hearing_aid_utils_get_role();

    if (role == local_role) {
        memcpy(notify_data, data_spk_ref, sizeof(app_hearing_aid_hearing_tuning_mode_t));
        *notify_data_len = sizeof(app_hearing_aid_hearing_tuning_mode_t);
    } else {
        uint8_t mode = 0;
        audio_psap_status_t status = audio_anc_psap_control_get_tuning_mode(&mode);

        if (status != AUDIO_PSAP_STATUS_SUCCESS) {
            APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_hearing_tuning_mode_switch_notify] Failed to get local hearing tuning mode : %d",
                             1,
                             status);
            return false;
        }

        app_hearing_aid_utils_get_hearing_tuning_mode_switch_combine_response((uint8_t *)&mode,
                                                                              sizeof(uint8_t),
                                                                              data,
                                                                              sizeof(uint8_t),
                                                                              notify_data,
                                                                              notify_data_len);
    }
    return true;
}

bool app_hearing_aid_utils_mp_test_mode_switch_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
#if 0
    notify_data[0] = data[0];
    *notify_data_len = sizeof(uint8_t);
#endif
    app_hearing_aid_utils_common_notify_handler(data, data_len, notify_data, notify_data_len);
    return true;
}

bool app_hearing_aid_utils_feedback_detection_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
    app_ha_utils_context.fb_det_notify_info.count ++;

    app_hearing_aid_feedback_detection_notify_t *fb_det_notify = (app_hearing_aid_feedback_detection_notify_t *)notify_data;
    app_hearing_aid_feedback_detection_notify_t *data_notify = (app_hearing_aid_feedback_detection_notify_t *)data;

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_feedback_detection_notify] Stored count : %d, channel : %d, notify channel : %d, data_len : %d",
                     4,
                     app_ha_utils_context.fb_det_notify_info.count,
                     app_ha_utils_context.feedback_detection_channel,
                     data_notify->channel,
                     data_len);

    /**
     * @brief ONLY execute FB detection on agent or partner side.
     * If execute on agent side, the channel should be L or R
     * If execute on partner side, the channel should be 0.
     * So when the notify received, need notify to SP directly.
     */
    if ((app_ha_utils_context.feedback_detection_channel == APP_HA_CHANNEL_L)
        || (app_ha_utils_context.feedback_detection_channel == APP_HA_CHANNEL_R)
        || (app_ha_utils_context.feedback_detection_channel == 0x00)) {
        memcpy(fb_det_notify, data_notify, sizeof(app_hearing_aid_feedback_detection_notify_t));
        *notify_data_len = sizeof(app_hearing_aid_feedback_detection_notify_t);

        memset(&(app_ha_utils_context.fb_det_notify_info), 0, sizeof(app_hearing_aid_feedback_detection_notify_info_t));

        return true;
    }

    if (app_ha_utils_context.fb_det_notify_info.count == 1) {
        memcpy(&(app_ha_utils_context.fb_det_notify_info.stored_info), data_notify, sizeof(app_hearing_aid_feedback_detection_notify_t));
        return false;
    }

    if (app_ha_utils_context.fb_det_notify_info.count == 2) {
        fb_det_notify->channel = APP_HA_CHANNEL_STEREO;

        if (data_notify->channel == APP_HA_CHANNEL_L) {
            memcpy(fb_det_notify->left_val, data_notify->left_val, 3);
        } else {
            memcpy(fb_det_notify->right_val, data_notify->right_val, 3);
        }
        if (app_ha_utils_context.fb_det_notify_info.stored_info.channel == APP_HA_CHANNEL_L) {
            memcpy(fb_det_notify->left_val, app_ha_utils_context.fb_det_notify_info.stored_info.left_val, 3);
        } else {
            memcpy(fb_det_notify->right_val, app_ha_utils_context.fb_det_notify_info.stored_info.right_val, 3);
        }

        *notify_data_len = sizeof(app_hearing_aid_feedback_detection_notify_t);
        memset(&(app_ha_utils_context.fb_det_notify_info), 0, sizeof(app_hearing_aid_feedback_detection_notify_info_t));

        return true;
    }

    return false;
}

bool app_hearing_aid_utils_mic_channel_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len)
{
#if 0
    notify_data[0] = data[0];
    *notify_data_len = sizeof(uint8_t);
#endif
    app_hearing_aid_utils_common_notify_handler(data, data_len, notify_data, notify_data_len);
    return true;
}

bool app_hearing_aid_utils_is_init_done()
{
    return app_ha_utils_context.init_done;
}

bool app_hearing_aid_utils_is_ha_user_switch_on()
{
    return app_ha_utils_context.user_switch;
}

bool app_hearing_aid_utils_is_ha_running()
{
    audio_psap_status_t status = AUDIO_PSAP_STATUS_SUCCESS;
    bool ha_enable_status = false;
    status = audio_anc_psap_control_get_status(&ha_enable_status);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_is_ha_running] status : %d, enable_status : %d",
                        2,
                        status,
                        ha_enable_status);

    if ((status == AUDIO_PSAP_STATUS_SUCCESS) && (ha_enable_status == true)) {
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_is_hearing_tuning_mode()
{
    uint8_t mode = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_tuning_mode(&mode);
    audio_psap_device_role_t local_role = app_hearing_aid_utils_get_role();
    app_hearing_aid_hearing_tuning_mode_t *mode_struct = (app_hearing_aid_hearing_tuning_mode_t *)&mode;

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_is_hearing_tuning_mode] status : %d, local role : %d, mode : 0x%02x, l_switch : %d, r_switch : %d",
                     5,
                     status,
                     local_role,
                     mode,
                     mode_struct->l_switch,
                     mode_struct->r_switch);

    if (status == AUDIO_PSAP_STATUS_SUCCESS) {
        if (((local_role == AUDIO_PSAP_DEVICE_ROLE_LEFT) && (mode_struct->l_switch == true))
            || ((local_role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) && (mode_struct->r_switch == true))) {
            return true;
        }
    }

    return false;
}

bool app_hearing_aid_utils_is_mp_test_mode()
{
    uint8_t mp_mode = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_mp_test_mode(&mp_mode);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_is_mp_test_mode] Status : %d, mp_mode : 0x%02x",
                     2,
                     status,
                     mp_mode);

    if ((status == AUDIO_PSAP_STATUS_SUCCESS) && (mp_mode == APP_HA_MP_TEST_MODE_SWITCH_ON)) {
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_is_beam_forming_enable()
{
    psap_bf_config_t bf_config = {0};
    audio_psap_status_t status = audio_anc_psap_control_get_beamforming_setting(&bf_config);

    if (status != AUDIO_PSAP_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_is_beam_forming_enable] Failed to get beam forming, result : %d", 1, status);
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_is_beam_forming_enable] Get BF switch : %d",
                     1,
                     bf_config.bf_switch);

    return bf_config.bf_switch;
}

bool app_hearing_aid_utils_is_aea_configure_enable()
{
    psap_aea_config_t current_config = {0};
    audio_psap_status_t get_status = audio_anc_psap_control_get_aea_configuration(&current_config);

    if (get_status != AUDIO_PSAP_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_is_aea_configure_enable] Failed to get AEA configuration, result : %d", 1, get_status);
        return false;
    }

    return current_config.aea_switch;
}

bool app_hearing_aid_utils_is_level_sync_on(bool *on)
{
    bool level_sync = app_hearing_aid_storage_get_level_sync_switch();

    *on = level_sync;

    return true;
}

bool app_hearing_aid_utils_is_rssi_mix_switch_on(bool *on)
{
    int8_t p_sensor_rssi_threshold = app_hear_through_storage_get_ha_rssi_threshold();
    bool p_sensor_rssi_switch = (p_sensor_rssi_threshold == 0) ? false : true;

    *on = p_sensor_rssi_switch;

    return true;
}

uint8_t app_hearing_aid_utils_get_master_mic_channel()
{
    uint8_t channel = 0;
    audio_psap_status_t get_status = audio_anc_psap_control_get_mic_channel(&channel);

    if (get_status != AUDIO_PSAP_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_master_mic_channel] Failed to get master MIC channel, result : %d", 1, get_status);
        return 0x00;
    }

    return channel;
}

bool app_hearing_aid_utils_get_mode_index_simple(uint8_t *out_mode_index)
{
    uint8_t mode_index = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_mode_index(&mode_index);

    *out_mode_index = 0;

    if (status == AUDIO_PSAP_STATUS_SUCCESS) {
        *out_mode_index = mode_index;
        return true;
    }

    return false;
}

typedef enum {
    HA_STATE_DISABLE,
    HA_STATE_ENABLE,
    HA_STATE_NONE,
} ha_state;

#if 0
typedef struct {
    bool a2dp_mix_switch;
    bool sco_mix_switch;
    bool vp_mix_switch;
    bool in_ear_switch;
    bool rssi_switch;
    ha_state a2dp_streaming;
    ha_state sco_streaming;
    ha_state vp_streaming;
    ha_state in_ear;
    ha_state less_than_threshold;
} ha_mix_table;

static const ha_mix_table ha_mix_table_list[] = {
    {true,  true,   true,   true,   true,   HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {true,  true,   true,   true,   false,  HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_ENABLE,    HA_STATE_NONE},
    {true,  true,   true,   false,  true,   HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_ENABLE},
    {true,  true,   true,   false,  false,  HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE},
    {true,  true,   false,  true,   true,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {true,  true,   false,  true,   true,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {true,  true,   false,  true,   true,   HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {true,  true,   false,  true,   false,  HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE},
    {true,  true,   false,  true,   false,  HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE},
    {true,  true,   false,  true,   false,  HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE},
    {true,  true,   false,  false,  true,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_ENABLE},
    {true,  true,   false,  false,  true,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_ENABLE},
    {true,  true,   false,  false,  true,   HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_ENABLE},
    {true,  true,   false,  false,  false,  HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_NONE},
    {true,  true,   false,  false,  false,  HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_NONE},
    {true,  true,   false,  false,  false,  HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE},
    {true,  false,  true,   true,   true,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {true,  false,  true,   true,   true,   HA_STATE_NONE,      HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {true,  false,  true,   true,   false,  HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE},
    {true,  false,  true,   true,   false,  HA_STATE_NONE,      HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_ENABLE,    HA_STATE_NONE},
    {true,  false,  true,   false,  true,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_ENABLE},
    {true,  false,  true,   false,  true,   HA_STATE_NONE,      HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_ENABLE},
    {true,  false,  true,   false,  false,  HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_NONE},
    {true,  false,  true,   false,  false,  HA_STATE_NONE,      HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE},
    {false, true,   true,   true,   true,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {false, true,   true,   true,   true,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {false, true,   true,   true,   false,  HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE},
    {false, true,   true,   true,   false,  HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE},
    {false, true,   true,   false,  true,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_ENABLE},
    {false, true,   true,   false,  true,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_ENABLE},
    {false, true,   true,   false,  false,  HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_NONE},
    {false, true,   true,   false,  false,  HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE,      HA_STATE_NONE},
    {true,  false,  false,  true,   true,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {true,  false,  false,  true,   true,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {true,  false,  false,  true,   true,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {true,  false,  false,  true,   false,  HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE},
    {true,  false,  false,  true,   false,  HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE},
    {true,  false,  false,  true,   false,  HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE},
    {true,  false,  false,  false,  true,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_ENABLE},
    {true,  false,  false,  false,  true,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_ENABLE},
    {true,  false,  false,  false,  true,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_ENABLE},
    {true,  false,  false,  false,  false,  HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE},
    {true,  false,  false,  false,  false,  HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_NONE},
    {true,  false,  false,  false,  false,  HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE},
    {false, true,   false,  true,   true,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {false, true,   false,  true,   true,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {false, true,   false,  true,   true,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {false, true,   false,  true,   false,  HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE},
    {false, true,   false,  true,   false,  HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE},
    {false, true,   false,  true,   false,  HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE},
    {false, true,   false,  false,  true,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_ENABLE},
    {false, true,   false,  false,  true,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_ENABLE},
    {false, true,   false,  false,  true,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_ENABLE},
    {false, true,   false,  false,  false,  HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE},
    {false, true,   false,  false,  false,  HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_NONE},
    {false, true,   false,  false,  false,  HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE},
    {false, false,  true,   true,   true,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {false, false,  true,   true,   true,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {false, false,  true,   true,   true,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {false, false,  true,   true,   true,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {false, false,  true,   true,   false,  HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE},
    {false, false,  true,   true,   false,  HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE},
    {false, false,  true,   true,   false,  HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE},
    {false, false,  true,   true,   false,  HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE},
    {false, false,  true,   false,  true,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_ENABLE},
    {false, false,  true,   false,  true,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_ENABLE},
    {false, false,  true,   false,  true,   HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_ENABLE},
    {false, false,  true,   false,  true,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_ENABLE},
    {false, false,  true,   false,  false,  HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_NONE},
    {false, false,  true,   false,  false,  HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_NONE},
    {false, false,  true,   false,  false,  HA_STATE_ENABLE,    HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE,      HA_STATE_NONE},
    {false, false,  true,   false,  false,  HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE},
    {false, false,  false,  true,   true,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_ENABLE},
    {false, false,  false,  true,   false,  HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_ENABLE,    HA_STATE_NONE},
    {false, false,  false,  false,  true,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_ENABLE},
    {false, false,  false,  false,  false,  HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_DISABLE,   HA_STATE_NONE,      HA_STATE_NONE},
};

#else
typedef struct {
    bool a2dp_mix_switch;
    bool sco_mix_switch;
    bool in_ear_switch;
    bool rssi_switch;
    ha_state a2dp_streaming;
    ha_state sco_streaming;
    ha_state in_ear;
    ha_state less_than_threshold;
} ha_mix_table;


static const ha_mix_table ha_mix_table_list[] = {
    {true, true, true, true, HA_STATE_NONE, HA_STATE_NONE, HA_STATE_ENABLE, HA_STATE_ENABLE},
    {true, true, true, false, HA_STATE_NONE, HA_STATE_NONE, HA_STATE_ENABLE, HA_STATE_NONE},
    {true, true, false, true, HA_STATE_NONE, HA_STATE_NONE, HA_STATE_NONE, HA_STATE_ENABLE},
    {true, true, false, false, HA_STATE_NONE, HA_STATE_NONE, HA_STATE_NONE, HA_STATE_NONE},
    {true, false, true, true, HA_STATE_NONE, HA_STATE_DISABLE, HA_STATE_ENABLE, HA_STATE_ENABLE},
    {true, false, true, false, HA_STATE_NONE, HA_STATE_DISABLE, HA_STATE_ENABLE, HA_STATE_NONE},
    {true, false, false, true, HA_STATE_NONE, HA_STATE_DISABLE, HA_STATE_NONE, HA_STATE_ENABLE},
    {true, false, false, false, HA_STATE_NONE, HA_STATE_DISABLE, HA_STATE_NONE, HA_STATE_NONE},
    {false, true, true, true, HA_STATE_DISABLE, HA_STATE_NONE, HA_STATE_ENABLE, HA_STATE_ENABLE},
    {false, true, true, false, HA_STATE_DISABLE, HA_STATE_NONE, HA_STATE_ENABLE, HA_STATE_NONE},
    {false, true, false, true, HA_STATE_DISABLE, HA_STATE_NONE, HA_STATE_NONE, HA_STATE_ENABLE},
    {false, true, false, false, HA_STATE_DISABLE, HA_STATE_NONE, HA_STATE_NONE, HA_STATE_NONE},
    {false, false, true, true, HA_STATE_DISABLE, HA_STATE_DISABLE, HA_STATE_ENABLE, HA_STATE_ENABLE},
    {false, false, true, false, HA_STATE_DISABLE, HA_STATE_DISABLE, HA_STATE_ENABLE, HA_STATE_NONE},
    {false, false, false, true, HA_STATE_DISABLE, HA_STATE_DISABLE, HA_STATE_NONE, HA_STATE_ENABLE},
    {false, false, false, false, HA_STATE_DISABLE, HA_STATE_DISABLE, HA_STATE_NONE, HA_STATE_NONE},
};

#endif

#define APP_HEARING_AID_UTILS_MIX_TABLE_LIST_SIZE (sizeof(ha_mix_table_list) / sizeof(ha_mix_table))

bool app_hearing_aid_utils_mix_table_to_enable(app_hearing_aid_state_table_t *table, app_hearing_aid_change_cause_item where, bool *need_execute)
{
    uint8_t get_response[APP_HEARING_AID_RESPONSE_MAX_LEN] = {0};
    uint16_t get_response_len = 0;
    bool ha_switch = app_ha_utils_context.user_switch;
    bool pure_tone_generator = false;
    bool mp_test_mode = false;
    bool a2dp_mix_switch = false;
    bool sco_mix_switch = false;
    // bool vp_mix_switch = false;
    bool in_ear_det_switch = false;
    bool p_sensor_rssi_switch = false;
    int8_t p_sensor_rssi_threshold = 0;

    bool ret_value = false;

    bool get_result = app_hearing_aid_utils_get_pure_tone_generator(NULL, get_response, &get_response_len);
    if (get_result == true) {
        pure_tone_generator = get_response[0];
    } else {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_mix_table_to_enable] Failed to get pure tone generator state", 0);
        *need_execute = false;
        return false;
    }

    in_ear_det_switch = app_hearing_aid_storage_get_in_ear_detection_switch();

    mp_test_mode = app_hearing_aid_utils_is_mp_test_mode();

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_mix_table_to_enable] ha_switch : %d, pure_tone_generator : %d, mp_test_mode : %d, where : %d",
                        4,
                        ha_switch,
                        pure_tone_generator,
                        mp_test_mode,
                        where);

    if ((ha_switch == false) && (mp_test_mode == false)) {
        *need_execute = true;
        return false;
    }

    a2dp_mix_switch = app_hearing_aid_storage_get_a2dp_mix_switch();
    sco_mix_switch = app_hearing_aid_storage_get_sco_mix_switch();
    // vp_mix_switch = app_hearing_aid_storage_get_vp_mix_switch();
    p_sensor_rssi_threshold = app_hear_through_storage_get_ha_rssi_threshold();
    p_sensor_rssi_switch = (p_sensor_rssi_threshold == 0) ? false : true;

#if 0
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_mix_table_to_enable] mix switch : a2dp(%d), sco(%d), vp(%d), in_ear(%d), rssi thresshold(%d), rssi (%d)",
                     6,
                     a2dp_mix_switch,
                     sco_mix_switch,
                     vp_mix_switch,
                     in_ear_det_switch,
                     p_sensor_rssi_threshold,
                     p_sensor_rssi_switch);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_mix_table_to_enable] mix state : a2dp(%d), sco(%d), vp(%d), in_ear(%d), rssi(%d)",
                     5,
                     table->a2dp_streaming,
                     table->sco_streaming,
                     table->vp_streaming,
                     table->in_ear,
                     table->less_than_threshold);
#else
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_mix_table_to_enable] mix switch : a2dp(%d), sco(%d), in_ear(%d), rssi thresshold(%d), rssi (%d)",
                     5,
                     a2dp_mix_switch,
                     sco_mix_switch,
                     in_ear_det_switch,
                     p_sensor_rssi_threshold,
                     p_sensor_rssi_switch);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_mix_table_to_enable] mix state : a2dp(%d), sco(%d), in_ear(%d), rssi(%d)",
                     4,
                     table->a2dp_streaming,
                     table->sco_streaming,
                     table->in_ear,
                     table->less_than_threshold);
#endif
    if (pure_tone_generator == true) {
        *need_execute = true;
        return true;
    }

    /**
     * @brief mp test mode is on, and A2DP/SCO/VP is not streaming.
     */
    if ((mp_test_mode == true)
        && (table->a2dp_streaming == false)
        && (table->sco_streaming == false)
        /* && (table->vp_streaming == false) */) {
        *need_execute = true;
        return true;
    }

    bool a2dp_enable = false;
    bool sco_enable = false;
    // bool vp_enable = false;
    bool in_ear_enable = false;
    bool rssi_enable = false;

    unsigned char index = 0;
    for (index = 0; index < APP_HEARING_AID_UTILS_MIX_TABLE_LIST_SIZE; index ++) {
        a2dp_enable = false;
        sco_enable = false;
        // vp_enable = false;
        in_ear_enable = false;
        rssi_enable = false;

        if (ha_mix_table_list[index].a2dp_mix_switch == a2dp_mix_switch
            && ha_mix_table_list[index].sco_mix_switch == sco_mix_switch
            // && ha_mix_table_list[index].vp_mix_switch == vp_mix_switch
            && ha_mix_table_list[index].in_ear_switch == in_ear_det_switch
            && ha_mix_table_list[index].rssi_switch == p_sensor_rssi_switch) {

            if ((ha_mix_table_list[index].a2dp_streaming == HA_STATE_NONE)
                || ((ha_mix_table_list[index].a2dp_streaming != HA_STATE_NONE) && (ha_mix_table_list[index].a2dp_streaming == table->a2dp_streaming))) {
                a2dp_enable = true;
            }

            if ((ha_mix_table_list[index].sco_streaming == HA_STATE_NONE)
                || ((ha_mix_table_list[index].sco_streaming != HA_STATE_NONE) && (ha_mix_table_list[index].sco_streaming == table->sco_streaming))) {
                sco_enable = true;
            }
#if 0
            if ((ha_mix_table_list[index].vp_streaming == HA_STATE_NONE)
                || ((ha_mix_table_list[index].vp_streaming != HA_STATE_NONE) && (ha_mix_table_list[index].vp_streaming == table->vp_streaming))) {
                vp_enable = true;
            }
#endif
            if ((ha_mix_table_list[index].in_ear == HA_STATE_NONE)
                || ((ha_mix_table_list[index].in_ear != HA_STATE_NONE) && (ha_mix_table_list[index].in_ear == table->in_ear))) {
                in_ear_enable = true;
            }

            if ((ha_mix_table_list[index].less_than_threshold == HA_STATE_NONE)
                || ((ha_mix_table_list[index].less_than_threshold != HA_STATE_NONE) && (ha_mix_table_list[index].less_than_threshold == table->less_than_threshold))) {
                rssi_enable = true;
            }

            if ((a2dp_enable == true)
                && (sco_enable == true)
                // && (vp_enable == true)
                && (in_ear_enable == true)
                && (rssi_enable == true)) {
                ret_value = true;
                break;
            }
        }
    }

    return ret_value;

}

bool app_hearing_aid_utils_set_user_switch(bool enable)
{
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_control_ha] user_switch enable= : %d",
                     1,
                     enable);
    if (app_ha_utils_context.user_switch != enable) {
        app_ha_utils_context.user_switch = enable;
    }

    return true;
}

bool app_hearing_aid_utils_control_ha(bool enable)
{
    audio_psap_status_t status = AUDIO_PSAP_STATUS_SUCCESS;
    bool ha_enable_status = false;
    status = audio_anc_psap_control_get_status(&ha_enable_status);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_control_ha] get_status : %d, ha_enable_status : %d, enable : %d",
                     3,
                     status,
                     ha_enable_status,
                     enable);

    // if (enable != ha_enable_status) {
        if (enable == true) {
            status = audio_anc_psap_control_enable();
        } else {
            status = audio_anc_psap_control_disable();
        }
        return ((status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false);
    // }

    // return true;
}

/**=====================================================================================*/
/** Combine function handler                                                            */
/**=====================================================================================*/
bool app_hearing_aid_utils_reload_configuration()
{
    audio_anc_psap_control_suspend();
    audio_anc_psap_control_resume();

    return true;
}

bool app_hearing_aid_utils_enable_mp_test_mode(bool enable)
{
    audio_psap_status_t set_status = audio_anc_psap_control_set_mp_test_mode((enable == true) ? APP_HA_MP_TEST_MODE_SWITCH_ON : APP_HA_MP_TEST_MODE_SWITCH_OFF);
    if (set_status != AUDIO_PSAP_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_enable_mp_test_mode] Failed to control MP test mode: %d, result %d",
                            2,
                            enable,
                            set_status);
        return false;
    }
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_enable_mp_test_mode] Succeed to control MP test mode : %d", 1, enable);
    return true;
}

bool app_hearing_aid_utils_adjust_level(uint8_t l_index, uint8_t r_index)
{
    audio_psap_device_role_t device_role = app_hearing_aid_utils_get_role();

    audio_psap_status_t set_result = audio_anc_psap_control_set_level_index(l_index, r_index, device_role);
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_adjust_level] Adjust level to, l_index : %d, r_index : %d, result : %d",
                     3,
                     l_index,
                     r_index,
                     set_result);

    if (set_result == AUDIO_PSAP_STATUS_SUCCESS) {
        app_hearing_aid_level_index_t level_index = {0};
        level_index.l_level_index = l_index;
        level_index.r_level_index = r_index;
        app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_EVENT_NOTIFICATION,
                                     APP_HEARING_AID_CONFIG_TYPE_LEVEL_INDEX,
                                     (uint8_t *)(&level_index),
                                     sizeof(app_hearing_aid_level_index_t));
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_adjust_volume(uint8_t l_index, uint8_t r_index)
{
    audio_psap_device_role_t device_role = app_hearing_aid_utils_get_role();

    audio_psap_status_t set_result = audio_anc_psap_control_set_volume_index(l_index, r_index, device_role);
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_adjust_volume] Adjust volume to, l_index : %d, r_index : %d, result : %d",
                     3,
                     l_index,
                     r_index,
                     set_result);

    if (set_result == AUDIO_PSAP_STATUS_SUCCESS) {
        app_hearing_aid_vol_index_t vol_index = {0};
        vol_index.l_vol_index = l_index;
        vol_index.r_vol_index = r_index;
        app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_EVENT_NOTIFICATION,
                                     APP_HEARING_AID_CONFIG_TYPE_VOLUME_INDEX,
                                     (uint8_t *)(&vol_index),
                                     sizeof(app_hearing_aid_vol_index_t));
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_adjust_mode(uint8_t target_mode_index)
{
    bool aea_disable_change = false;

    if (app_hearing_aid_utils_is_aea_configure_enable() == true) {
        bool aea_op_result = app_hearing_aid_utils_aea_switch_toggle(false);
        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_adjust_mode] AEA enabled, disable AEA firstly, result : %d",
                         1,
                         aea_op_result);
        if (aea_op_result == true) {
            aea_disable_change = true;
        }
    }

    audio_psap_status_t mode_set_status = audio_anc_psap_control_set_mode_index(target_mode_index);
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_adjust_mode] Set mode index : %d, result : %d, disabled AEA before : %d",
                     3,
                     target_mode_index,
                     mode_set_status,
                     aea_disable_change);

    if (mode_set_status != AUDIO_PSAP_STATUS_SUCCESS) {
        if (aea_disable_change == true) {
            app_hearing_aid_utils_aea_switch_toggle(true);
        }
        return false;
    }

    app_hearing_aid_utils_check_master_mic_channel_changed(false);

    app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_EVENT_NOTIFICATION,
                                 APP_HEARING_AID_CONFIG_TYPE_MODE_INDEX,
                                 &target_mode_index,
                                 sizeof(uint8_t));

    return true;
}


typedef struct {
    bool                    up;
    bool                    circular;
    bool                    sync;
    uint8_t                 l_index;
    uint8_t                 r_index;
    uint8_t                 max_count;
} app_ha_lr_change_value_t;

bool app_hearing_aid_utils_get_lr_change_value(app_ha_lr_change_value_t *value, app_hearing_aid_change_value_t *change_value)
{
    if ((value == NULL) || (change_value == NULL)) {
        return false;
    }

    audio_psap_device_role_t device_role = app_hearing_aid_utils_get_role();
    uint8_t dif_value = 0;
    memset(change_value, 0, sizeof(app_hearing_aid_change_value_t));

    change_value->sync = value->sync;
    change_value->l_index = value->l_index;
    change_value->r_index = value->r_index;

    if (value->up == true) {
        if (value->circular == true) {
            if (value->sync == true) {
                change_value->sync = true;

                if ((value->l_index == (value->max_count - 2)) || (value->r_index == (value->max_count - 2))) {
                    change_value->max = true;
                }

                if ((value->l_index == (value->max_count - 1)) || (value->r_index == (value->max_count - 1))) {
                    if (value->l_index > value->r_index) {
                        dif_value = value->l_index - value->r_index;
                    } else {
                        dif_value = value->r_index - value->l_index;
                    }

                    if (value->l_index == (value->max_count - 1)) {
                        change_value->r_index = 0;
                        change_value->l_index = change_value->r_index + dif_value;
                    }
                    if (value->r_index == (value->max_count - 1)) {
                        change_value->l_index = 0;
                        change_value->r_index = change_value->l_index + dif_value;
                    }
                } else {
                    change_value->l_index = value->l_index + 1;
                    change_value->r_index = value->r_index + 1;
                }
            } else {
                if (device_role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
                    if (value->l_index == (value->max_count - 2)) {
                        change_value->max = true;
                    }
                    if (value->l_index == (value->max_count - 1)) {
                        change_value->l_index = 0;
                    } else {
                        change_value->l_index = value->l_index + 1;
                    }
                } else {
                    if (value->r_index == (value->max_count - 2)) {
                        change_value->max = true;
                    }
                    if (value->r_index == (value->max_count - 1)) {
                        change_value->r_index = 0;
                    } else {
                        change_value->r_index = value->r_index + 1;
                    }
                }
            }
        } else {
            if (value->sync == true) {
                change_value->sync = true;
                if ((value->l_index == (value->max_count - 1)) || (value->r_index == (value->max_count - 1))) {
                    change_value->max = true;
                } else {
                    change_value->l_index = value->l_index + 1;
                    change_value->r_index = value->r_index + 1;
                }
            } else {
                if (device_role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
                    if (value->l_index == (value->max_count - 1)) {
                        change_value->max = true;
                    } else {
                        change_value->l_index = value->l_index + 1;
                    }
                } else {
                    if (value->r_index == (value->max_count - 1)) {
                        change_value->max = true;
                    } else {
                        change_value->r_index = value->r_index + 1;
                    }
                }
            }
        }
    } else {
        if (value->sync == true) {
            change_value->sync = true;
            if (value->l_index == 0 || value->r_index == 0) {
                change_value->min = true;
            } else {
                change_value->l_index = value->l_index - 1;
                change_value->r_index = value->r_index - 1;
            }
        } else {
            if (device_role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
                if (value->l_index == 0) {
                    change_value->min = true;
                } else {
                    change_value->l_index = value->l_index - 1;
                }
            } else {
                if (value->r_index == 0) {
                    change_value->min = true;
                } else {
                    change_value->r_index = value->r_index - 1;
                }
            }
        }
    }

    return true;
}

bool app_hearing_aid_utils_get_level_change_value(bool up, bool circular, app_hearing_aid_change_value_t *change_value)
{
    uint8_t l_level_index = 0;
    uint8_t r_level_index = 0;
    uint8_t level_max_count = 0;
    uint8_t mode_max_count = 0;
    uint8_t vol_max_count = 0;
    uint8_t level_sync_switch = 0;

    audio_psap_status_t get_level_index_status = audio_anc_psap_control_get_level_index(&l_level_index, &r_level_index);
    audio_psap_status_t level_max_count_status = audio_anc_psap_control_get_level_mode_max_count(&level_max_count, &mode_max_count, &vol_max_count);
    level_sync_switch = app_hearing_aid_storage_get_level_sync_switch();

    if ((get_level_index_status != AUDIO_PSAP_STATUS_SUCCESS)
        || (level_max_count_status != AUDIO_PSAP_STATUS_SUCCESS)) {
        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_level_change_value] get status : %d, max status : %d",
                         2,
                         get_level_index_status,
                         level_max_count_status);
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_level_change_value] up : %d, circular : %d, l_index : %d, r_index : %d, level_max : %d, sync switch ; %d",
                     6,
                     up,
                     circular,
                     l_level_index,
                     r_level_index,
                     level_max_count,
                     level_sync_switch);

    if ((up == false) && (circular == true)) {
        return false;
    }

    app_ha_lr_change_value_t ori_value = {0};
    ori_value.circular = circular;
    ori_value.l_index = l_level_index;
    ori_value.r_index = r_level_index;
    ori_value.up = up;
    ori_value.max_count = level_max_count;
    ori_value.sync = level_sync_switch;

    bool ret = app_hearing_aid_utils_get_lr_change_value(&ori_value, change_value);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_level_change_value] After change, ret : %d, sync : %d, max : %d, min : %d, l_index : %d, r_index : %d",
                     6,
                     ret,
                     change_value->sync,
                     change_value->max,
                     change_value->min,
                     change_value->l_index,
                     change_value->r_index);

    return ret;

}

bool app_hearing_aid_utils_get_volume_change_value(bool up, bool circular, app_hearing_aid_change_value_t *change_value)
{
    uint8_t l_vol_index = 0;
    uint8_t r_vol_index = 0;
    uint8_t level_max_count = 0;
    uint8_t mode_max_count = 0;
    uint8_t vol_max_count = 0;
    uint8_t vol_sync_switch = 0;

    audio_psap_status_t get_vol_index_status = audio_anc_psap_control_get_volume_index(&l_vol_index, &r_vol_index);
    audio_psap_status_t vol_max_count_status = audio_anc_psap_control_get_level_mode_max_count(&level_max_count, &mode_max_count, &vol_max_count);
    vol_sync_switch = app_hearing_aid_storage_get_volume_sync_switch();

    if ((get_vol_index_status != AUDIO_PSAP_STATUS_SUCCESS)
        || (vol_max_count_status != AUDIO_PSAP_STATUS_SUCCESS)) {
        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_volume_change_value] get status : %d, max status : %d, vol sync status : %d",
                         2,
                         get_vol_index_status,
                         vol_max_count_status);
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_volume_change_value] up : %d, circular : %d, l_index : %d, r_index : %d, vol_max : %d, sync switch : %d",
                     6,
                     up,
                     circular,
                     l_vol_index,
                     r_vol_index,
                     vol_max_count,
                     vol_sync_switch);

    if ((up == false) && (circular == true)) {
        return false;
    }

    app_ha_lr_change_value_t ori_value = {0};
    ori_value.circular = circular;
    ori_value.l_index = l_vol_index;
    ori_value.r_index = r_vol_index;
    ori_value.up = up;
    ori_value.max_count = vol_max_count;
    ori_value.sync = vol_sync_switch;

    bool ret = app_hearing_aid_utils_get_lr_change_value(&ori_value, change_value);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_volume_change_value] After change, ret : %d, sync : %d, max : %d, min : %d, l_index : %d, r_index : %d",
                     6,
                     ret,
                     change_value->sync,
                     change_value->max,
                     change_value->min,
                     change_value->l_index,
                     change_value->r_index);

    return ret;
}

bool app_hearing_aid_utils_get_mode_change_value(bool up, bool circular, uint8_t *out_mode, bool *max, bool *min)
{
    uint8_t mode_index = 0;
    uint8_t level_max_count = 0;
    uint8_t mode_max_count = 0;
    uint8_t vol_max_count = 0;

    audio_psap_status_t mode_index_status = audio_anc_psap_control_get_mode_index(&mode_index);
    audio_psap_status_t mode_max_count_status = audio_anc_psap_control_get_level_mode_max_count(&level_max_count, &mode_max_count, &vol_max_count);

    if ((out_mode == NULL) || (max == NULL) || (min == NULL)) {
        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_mode_change_value] Pointer parameter error, %x, %x, %x", 3,
                         out_mode, max, min);
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_mode_change_value] mode index status : %d, mode max count : %d, up : %d, circular : %d, current mode index : %d, mode max count : %d",
                     6,
                     mode_index_status,
                     mode_max_count_status,
                     up,
                     circular,
                     mode_index,
                     mode_max_count);

    if ((mode_index_status != AUDIO_PSAP_STATUS_SUCCESS) || (mode_max_count_status != AUDIO_PSAP_STATUS_SUCCESS)) {
        return false;
    }

    *out_mode = 0;
    *max = false;
    *min = false;

    if (up == false && circular == true) {
        return false;
    }

    if (up == true) {
        if (circular == false) {
            if (mode_index == mode_max_count - 1) {
                *max = true;
            } else {
                mode_index += 1;
            }
        } else {
            if (mode_index == mode_max_count - 2) {
                *max = true;
            }
            if (mode_index == mode_max_count - 1) {
                mode_index = 0;
            } else {
                mode_index += 1;
            }
        }
    } else {
        if (mode_index == 0) {
            *min = true;
        } else {
            mode_index -= 1;
        }
    }

    *out_mode = mode_index;

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_get_mode_change_value] Out mode : %d, max : %d, min : %d",
                        3,
                        *out_mode,
                        *max,
                        *min);

    if (((*max == true) && (circular == false)) || (*min == true)) {
        return false;
    }

    return true;
}

bool app_hearing_aid_utils_hearing_tuning_mode_toggle(bool from_remote)
{
    uint8_t mode = 0;
    audio_psap_status_t status = audio_anc_psap_control_get_tuning_mode(&mode);

    union {
        uint8_t uint8_t_mode;
        app_hearing_aid_hearing_tuning_mode_t struct_mode;
    } temp_tuning_mode_t;

    if (status == AUDIO_PSAP_STATUS_SUCCESS) {
        app_hearing_aid_hearing_tuning_mode_t *tuning_mode = (app_hearing_aid_hearing_tuning_mode_t *)&mode;
        audio_psap_device_role_t device_role = app_hearing_aid_utils_get_role();

        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_hearing_tuning_mode_toggle] remote : %d, device role : %d, l_switch : %d, r_switch : %d",
                         4,
                         from_remote,
                         device_role,
                         tuning_mode->l_switch,
                         tuning_mode->r_switch);

        temp_tuning_mode_t.struct_mode.l_switch = tuning_mode->l_switch;
        temp_tuning_mode_t.struct_mode.r_switch = tuning_mode->r_switch;
        temp_tuning_mode_t.struct_mode.reserved = tuning_mode->reserved;

        if (device_role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
            if (from_remote == true) {
                temp_tuning_mode_t.struct_mode.r_switch = !temp_tuning_mode_t.struct_mode.r_switch;
            } else {
                temp_tuning_mode_t.struct_mode.l_switch = !temp_tuning_mode_t.struct_mode.l_switch;
            }
        } else if (device_role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
            if (from_remote == true) {
                temp_tuning_mode_t.struct_mode.l_switch = !temp_tuning_mode_t.struct_mode.l_switch;
            } else {
                temp_tuning_mode_t.struct_mode.r_switch = !temp_tuning_mode_t.struct_mode.r_switch;
            }
        } else {
            return false;
        }

        status = audio_anc_psap_control_set_tuning_mode(temp_tuning_mode_t.uint8_t_mode);
        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_hearing_tuning_mode_toggle] Set tuning mode to be : %d (L : %d, R : %d), result : %d",
                         4,
                         temp_tuning_mode_t.uint8_t_mode,
                         temp_tuning_mode_t.struct_mode.l_switch,
                         temp_tuning_mode_t.struct_mode.r_switch,
                         status);
        if (status != AUDIO_PSAP_STATUS_SUCCESS) {
            return false;
        }

        app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_EVENT_NOTIFICATION,
                                     APP_HEARING_AID_CONFIG_TYPE_TUNNING_MODE_SWITCH,
                                     &(temp_tuning_mode_t.uint8_t_mode),
                                     sizeof(temp_tuning_mode_t));
    } else {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_hearing_tuning_mode_toggle] Failed to get tuning mode, result : %d", 1, status);
        return false;
    }

    return true;
}

bool app_hearing_aid_utils_beam_forming_switch_toggle(bool enable)
{
    psap_bf_config_t bf_config = {0};
    audio_psap_status_t status = AUDIO_PSAP_STATUS_SUCCESS;
    bool bf_enable = app_hearing_aid_utils_is_beam_forming_enable();

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_beam_forming_switch_toggle] enable : %d, Get BF switch : %d",
                     2,
                     enable,
                     bf_enable);

    if (bf_enable == enable) {
        return true;
    }

    bf_config.bf_switch_mode_ctrl = false;
    bf_config.bf_switch = enable;

    status = audio_anc_psap_control_set_beamforming_setting(&bf_config);
    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_beam_forming_switch_toggle] Set BF switch : %d, result : %d",
                     2,
                     bf_config.bf_switch,
                     status);

    if (status == AUDIO_PSAP_STATUS_SUCCESS) {

        app_hearing_aid_utils_check_master_mic_channel_changed(false);

        app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_EVENT_NOTIFICATION,
                                     APP_HEARING_AID_CONFIG_TYPE_BF_SETTINGS,
                                     (uint8_t *)(&bf_config),
                                     sizeof(psap_bf_config_t));
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_aea_switch_toggle(bool enable)
{
    psap_aea_config_t current_config = {0};
    audio_psap_status_t get_status = audio_anc_psap_control_get_aea_configuration(&current_config);
    audio_psap_status_t set_status = AUDIO_PSAP_STATUS_SUCCESS;

    if (get_status != AUDIO_PSAP_STATUS_SUCCESS) {
        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_aea_switch_toggle] Failed to get AEA status, %d", 1, get_status);
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_aea_switch_toggle] enable : %d, Get AEA switch current status : %d",
                     2,
                     enable,
                     current_config.aea_switch);

    if (current_config.aea_switch == enable) {
        return true;
    }

    current_config.aea_switch = enable;

    set_status = audio_anc_psap_control_set_aea_configuration(&current_config);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_aea_switch_toggle] Set AEA switch to : %d, result : %d",
                     2,
                     current_config.aea_switch,
                     set_status);

    if (set_status == AUDIO_PSAP_STATUS_SUCCESS) {
        app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_EVENT_NOTIFICATION,
                                     APP_HEARING_AID_CONFIG_TYPE_AEA_CONFIGURATION,
                                     (uint8_t *)(&current_config),
                                     sizeof(psap_aea_config_t));

        if (enable == false) {
            app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_TO_PLAY_AEA_OFF_VP, 0, NULL, 0);
        }
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_master_mic_channel_switch_toggle(uint8_t target_channel)
{
    uint8_t channel = app_hearing_aid_utils_get_master_mic_channel();
    audio_psap_status_t set_status = AUDIO_PSAP_STATUS_SUCCESS;

    if ((channel != APP_HEARING_AID_MASTER_MIC_CHANNEL_1) && (channel != APP_HEARING_AID_MASTER_MIC_CHANNEL_2)) {
        APPS_LOG_MSGID_E(APP_HA_UTILS_TAG"[app_hearing_aid_utils_master_mic_channel_switch_toggle] Failed to get mic channel, channel : 0x%02x",
                         1,
                         channel);
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_master_mic_channel_switch_toggle] target channel : 0x%02x, Get mic channel : 0x%02x",
                     2,
                     target_channel,
                     channel);

    if (target_channel == channel) {
        return true;
    }

    channel = target_channel;

    set_status = audio_anc_psap_control_set_mic_channel(channel);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_master_mic_channel_switch_toggle] Set mic channel : 0x%02x, result : %d",
                     2,
                     channel,
                     set_status);

    if (set_status == AUDIO_PSAP_STATUS_SUCCESS) {

        app_hearing_aid_utils_check_master_mic_channel_changed(false);

        app_hearing_aid_utils_notify(APP_HEARING_AID_UTILS_NOTIFY_EVENT_NOTIFICATION,
                                     APP_HEARING_AID_CONFIG_TYPE_MIC_CONTROL,
                                     (uint8_t *)(&channel),
                                     sizeof(uint8_t));
        return true;
    }

    return false;
}


bool app_hearing_aid_utils_save_user_settings()
{
    audio_anc_psap_control_save_setting();
    return true;
}

bool app_hearing_aid_utils_control_fwk(bool enable, bool with_anc_path)
{

    audio_anc_control_result_t control_ret = AUDIO_ANC_CONTROL_EXECUTION_FAIL;

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_control_fwk] enable : %d, with_anc_path : %d",
                        2,
                        enable,
                        with_anc_path);

    if (enable == true) {
        audio_anc_control_filter_id_t      target_filter_id;
        audio_anc_control_type_t           target_anc_type;
        audio_anc_control_misc_t           local_misc = {0};

        if (with_anc_path == false) {
            local_misc.type_mask_param.ANC_path_mask = 0;
        } else {
            local_misc.type_mask_param.ANC_path_mask = app_hear_through_storage_get_anc_path_mask_for_ha_psap();
        }

        uint32_t mic_input_path = 0;
        audio_anc_psap_control_get_mic_input_path(&mic_input_path);

        target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_HA_PSAP | mic_input_path;
        target_filter_id     = AUDIO_ANC_CONTROL_HA_PSAP_FILTER_DEFAULT; //1~4

        APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_control_fwk] anc_path_mask : %d, anc_type : %d",
                            2,
                            local_misc.type_mask_param.ANC_path_mask,
                            target_anc_type);

        control_ret = audio_anc_control_enable(target_filter_id, target_anc_type, &local_misc);
    } else {
        control_ret = audio_anc_control_disable(NULL);
    }

    return (control_ret == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) ? true : false;
}

bool app_hearing_aid_utils_is_drc_on(app_hearing_aid_drc_type_t type, bool *status)
{
    psap_scenario_mix_mode_t mix_mode = {0};
    audio_psap_status_t psap_status = audio_anc_psap_control_get_mix_mode(&mix_mode);
    bool ret_value = (psap_status == AUDIO_PSAP_STATUS_SUCCESS) ? true : false;

    if (ret_value == false) {
        *status = false;
        return false;
    }

    *status = false;

    switch (type) {
        case APP_HEARING_AID_DRC_A2DP: {
            if ((mix_mode.a2dp_drc_switch_l == true) || (mix_mode.a2dp_drc_switch_r == true)) {
                *status = true;
            }
        }
        break;
        case APP_HEARING_AID_DRC_SCO: {
            if ((mix_mode.sco_drc_switch_l == true) || (mix_mode.sco_drc_switch_r == true)) {
                *status = true;
            }
        }
        break;
        case APP_HEARING_AID_DRC_VP: {
            if ((mix_mode.vp_drc_switch_l == true) || (mix_mode.vp_drc_switch_r == true)) {
                *status = true;
            }
        }
        break;
        default: {
            *status = false;
            return false;
        }
    }

    return true;
}

bool app_hearing_aid_utils_is_drc_enable(bool a2dp_streaming, bool sco_streaming, bool vp_streaming)
{
    bool a2dp_drc_on = false;
    bool sco_drc_on = false;
    bool vp_drc_on = false;

    bool a2dp_drc_on_result = app_hearing_aid_utils_is_drc_on(APP_HEARING_AID_DRC_A2DP, &a2dp_drc_on);
    bool sco_drc_on_result = app_hearing_aid_utils_is_drc_on(APP_HEARING_AID_DRC_SCO, &sco_drc_on);
    bool vp_drc_on_result = app_hearing_aid_utils_is_drc_on(APP_HEARING_AID_DRC_VP, &vp_drc_on);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_is_drc_enable] a2dp streaming : %d, sco_streaming : %d, vp_streaming : %d",
                     3,
                     a2dp_streaming,
                     sco_streaming,
                     vp_streaming);

    APPS_LOG_MSGID_I(APP_HA_UTILS_TAG"[app_hearing_aid_utils_is_drc_enable] DRC state : a2dp (%d - %d), sco (%d - %d), vp (%d - %d)",
                     6,
                     a2dp_drc_on_result,
                     a2dp_drc_on,
                     sco_drc_on_result,
                     sco_drc_on,
                     vp_drc_on_result,
                     vp_drc_on);

    if ((a2dp_drc_on_result == false)
        || (sco_drc_on_result == false)
        || (vp_drc_on_result == false)) {
        return false;
    }

    if (((a2dp_streaming == true) && (a2dp_drc_on == true))
        || ((sco_streaming == true) && (sco_drc_on == true))
        || ((vp_streaming == true) && (vp_drc_on == true))) {
        return true;
    }

    return false;
}

bool app_hearing_aid_utils_is_sco_mix_mode_on()
{
    return app_ha_utils_context.ha_mix_mode.sco_mix_mode_switch;
}

bool app_hearing_aid_utils_is_music_mix_mode_on()
{
    return app_ha_utils_context.ha_mix_mode.a2dp_mix_mode_switch;
}

void app_hearing_aid_utils_sync_runtime_parameters(uint8_t *parameter, uint16_t parameter_len)
{
    if ((parameter == NULL) || (parameter_len == 0)) {
        return;
    }

    audio_anc_psap_control_set_runtime_sync_parameter(parameter_len, parameter);

    app_hearing_aid_utils_check_master_mic_channel_changed(false);
}


#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */


