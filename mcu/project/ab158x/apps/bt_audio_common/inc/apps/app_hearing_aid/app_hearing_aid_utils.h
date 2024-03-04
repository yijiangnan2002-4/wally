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

#ifndef __APP_HEARING_AID_UTILS_H__
#define __APP_HEARING_AID_UTILS_H__

#include "stdint.h"
#include "stdbool.h"
#include "audio_anc_psap_control.h"
#include "app_hear_through_race_cmd_handler.h"

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define APP_HEARING_AID_DEBUG_ENABLE                    0

#define APP_HEARING_AID_CHAR_LOG_MAX_LEN                40

#define APP_HEARING_AID_MASTER_MIC_CHANNEL_1            0x01
#define APP_HEARING_AID_MASTER_MIC_CHANNEL_2            0x02

#define APP_HEARING_AID_EVENT_ID_NONE                   0x0000
// #define APP_HEARING_AID_EVENT_ID_RACE_CONNECTED         0x0001
// #define APP_HEARING_AID_EVENT_ID_RACE_DISCONNECTED      0x0002
// #define APP_HEARING_AID_EVENT_ID_BLE_ADV_TIMEOUT        0x0003
#define APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA  0x0004
// #define APP_HEARING_AID_EVENT_ID_VP_STREAMING_BEGIN     0x0005
// #define APP_HEARING_AID_EVENT_ID_VP_STREAMING_END       0x0006
#define APP_HEARING_AID_EVENT_ID_RSSI_READ              0x0007
#define APP_HEARING_AID_EVENT_ID_MODIFY_MODE_INDEX      0x0008
#define APP_HEARING_AID_EVENT_ID_AEA_OFF_VP             0x0009
// #define APP_HEARING_AID_EVENT_ID_INIT_TO_CONTROL_HA     0x000A
#define APP_HEARING_AID_EVENT_ID_INIT_TO_PLAY_POWER_ON_VP 0x000B
#define APP_HEARING_AID_EVENT_ID_RSSI_POWER_OFF         0x000C
#define APP_HEARING_AID_EVENT_ID_REQUEST_TO_RESUME_ANC  0x000D

#define APP_HEARING_AID_EVENT_ID_HA_ON                  0x1001
#define APP_HEARING_AID_EVENT_ID_HA_OFF                 0x1002

#define APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT      200 // 200ms

#define APP_HEARING_AID_EVENT_SYNC_BASE                 0x2000

#define APP_HEARING_AID_EVENT_ID_AWS_BEGIN                                  0x0020
#define APP_HEARING_AID_EVENT_ID_AWS_MIDDLEWARE_CONFIGURATION_SYNC_REQUEST  APP_HEARING_AID_EVENT_ID_AWS_BEGIN
#define APP_HEARING_AID_EVENT_ID_AWS_SYNC_MIDDLEWARE_CONFIGURATION          (APP_HEARING_AID_EVENT_ID_AWS_BEGIN + 1)
#define APP_HEARING_AID_EVENT_ID_AWS_RACE_CMD_REQUEST                       (APP_HEARING_AID_EVENT_ID_AWS_BEGIN + 2)
#define APP_HEARING_AID_EVENT_ID_AWS_RACE_CMD_RESPONSE                      (APP_HEARING_AID_EVENT_ID_AWS_BEGIN + 3)
#define APP_HEARING_AID_EVENT_ID_AWS_NOTIFICATION                           (APP_HEARING_AID_EVENT_ID_AWS_BEGIN + 4)
#define APP_HEARING_AID_EVENT_ID_AWS_SYNC_USER_CONFIGURATION                (APP_HEARING_AID_EVENT_ID_AWS_BEGIN + 5)
// #define APP_HEARING_AID_EVENT_ID_AWS_SYNC_APP_INFO                          (APP_HEARING_AID_EVENT_ID_AWS_BEGIN + 6)

#define APP_HEARING_AID_RESPONSE_MAX_LEN                104
#define APP_HEARING_AID_NOTIFY_MAX_LEN                  25

typedef enum {
    APP_HEARING_AID_CONFIG_TYPE_NONE                    = 0x0000,
    APP_HEARING_AID_CONFIG_TYPE_HA_SWITCH               = 0x0001,
    APP_HEARING_AID_CONFIG_TYPE_LEVEL_INDEX             = 0x0002,
    APP_HEARING_AID_CONFIG_TYPE_LEVEL_SYNC_SWITCH       = 0x0003,
    APP_HEARING_AID_CONFIG_TYPE_LEVEL_MODE_MAX_COUNT    = 0x0004,
    APP_HEARING_AID_CONFIG_TYPE_VOLUME_INDEX            = 0x0005,
    APP_HEARING_AID_CONFIG_TYPE_VOLUME_SYNC_SWITCH      = 0x0006,
    APP_HEARING_AID_CONFIG_TYPE_MODE_INDEX              = 0x0007,
    APP_HEARING_AID_CONFIG_TYPE_SPECIFIC_MODE_TABLE     = 0x0008,
    APP_HEARING_AID_CONFIG_TYPE_AEA_CONFIGURATION       = 0x0009,
    APP_HEARING_AID_CONFIG_TYPE_WNR_SWITCH              = 0x000A,
    APP_HEARING_AID_CONFIG_TYPE_BF_SETTINGS             = 0x000B,
    APP_HEARING_AID_CONFIG_TYPE_AFC_CONFIGURE           = 0x000C,
    APP_HEARING_AID_CONFIG_TYPE_INR_CONFIG              = 0x000D,
    APP_HEARING_AID_CONFIG_TYPE_USER_EQ_SWITCH          = 0x000E,
    APP_HEARING_AID_CONFIG_TYPE_USER_EQ_GAIN            = 0x000F,
    APP_HEARING_AID_CONFIG_TYPE_SPEAKER_REFERENCE       = 0x0010,
    APP_HEARING_AID_CONFIG_TYPE_PURE_TONE_GENERATOR     = 0x0011,
    APP_HEARING_AID_CONFIG_TYPE_MIX_MODE_TOTAL_SETTING  = 0x0012,
    APP_HEARING_AID_CONFIG_TYPE_TUNNING_MODE_SWITCH     = 0x0013,
    APP_HEARING_AID_CONFIG_TYPE_MP_TEST_MODE_SWITCH     = 0x0014,
    APP_HEARING_AID_CONFIG_TYPE_RESTORE_SETTING         = 0x0015,
    APP_HEARING_AID_CONFIG_TYPE_FEEDBACK_DETECTION      = 0x0016,
    APP_HEARING_AID_CONFIG_TYPE_MUTE                    = 0x0017,
    APP_HEARING_AID_CONFIG_TYPE_HOWLING_DETECTION       = 0x0018,
    APP_HEARING_AID_CONFIG_TYPE_MPO_ADJUST              = 0x0019,
    APP_HEARING_AID_CONFIG_TYPE_IN_EAR_DETECTION        = 0x001A,
    APP_HEARING_AID_CONFIG_TYPE_PASSTHROUGH_SWITCH      = 0x001B,
    APP_HEARING_AID_CONFIG_TYPE_MIC_CONTROL             = 0x001C,
    APP_HEARING_AID_CONFIG_TYPE_TRIAL_RUN               = 0x001D,
    APP_HEARING_AID_CONFIG_TYPE_MIC_CALIBRATION_MODE    = 0x001E,
    APP_HEARING_AID_CONFIG_TYPE_MIC_CALIBRATION_DATA    = 0x001F,
#ifdef AIR_DAC_MODE_RUNTIME_CHANGE
    APP_HEARING_AID_CONFIG_TYPE_HEARING_TEST_MODE       = 0x0020,
#endif /* AIR_DAC_MODE_RUNTIME_CHANGE */
    APP_HEARING_AID_CONFIG_TYPE_MAX,
} app_hearing_aid_config_type_t;

extern const char app_hearing_aid_command_string[][APP_HEARING_AID_CHAR_LOG_MAX_LEN];
extern const char app_hearing_aid_type_string[][APP_HEARING_AID_CHAR_LOG_MAX_LEN];
extern const char app_hearing_aid_execute_where_string[][APP_HEARING_AID_CHAR_LOG_MAX_LEN];

#define APP_HEARING_AID_VP_REQUIRE_NONE                 0x00
#define APP_HEARING_AID_VP_REQUIRE_LOCAL                0x01
#define APP_HEARING_AID_VP_REQUIRE_BOTH                 0x02
typedef uint8_t app_hearing_aid_vp_require_t;

#define APP_HEARING_AID_UTILS_NOTIFY_EVENT_NOTIFICATION     0x0001
#define APP_HEARING_AID_UTILS_NOTIFY_TO_CONTROL_HA          0x0002
#define APP_HEARING_AID_UTILS_NOTIFY_TO_UPDATE_MODE_INDEX   0x0003
#define APP_HEARING_AID_UTILS_NOTIFY_TO_PLAY_AEA_OFF_VP     0x0004
#define APP_HEARING_AID_UTILS_NOTIFY_IN_EAR_DETECTION_SWITCH_CHANGE 0x0005
#define APP_HEARING_AID_UTILS_NOTIFY_TO_ENTER_MP_TEST_MODE  0x0006
#define APP_HEARING_AID_UTILS_NOTIFY_TO_EXIT_MP_TEST_MODE   0x0007
#define APP_HEARING_AID_UTILS_NOTIFY_TO_CONTROL_FWK         0x0008


typedef void (*notify)(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len);

void app_hearing_aid_utils_init(notify handler);
void app_hearing_aid_utils_deinit();
audio_psap_device_role_t app_hearing_aid_utils_get_role();

void app_hearing_aid_utils_handle_get_race_cmd(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
void app_hearing_aid_utils_handle_set_race_cmd(app_hear_through_request_t *request, bool *set_result);
bool app_hearing_aid_utils_handle_get_combine_response(uint16_t type,
                                                       bool is_local_response,
                                                       uint8_t *response,
                                                       uint16_t response_len,
                                                       uint8_t *combine_response,
                                                       uint16_t *combine_response_len);
uint32_t app_hearing_aid_utils_get_combine_response_length(uint16_t type);

bool app_hearing_aid_utils_handle_notify(uint8_t role, uint16_t type, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);

bool app_hearing_aid_utils_get_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_level_index(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_level_sync_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mode_max_count(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_volume_index(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_volume_sync_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mode_index(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_specific_mode_table(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_aea_configuration(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_wnr_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_beam_forming_setting(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_afc_config(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_inr_config(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_user_eq_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_user_eq_gain(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_speaker_reference(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_pure_tone_generator(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mix_mode_total_setting(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_hearing_tuning_mode_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mp_test_mode_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_feedback_detection(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mute(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_howling_detection(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mpo_adjustment(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_in_ear_detection_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_passthrough_switch(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mic_channel(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mic_calibration_mode(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mic_calibration_data(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
#ifdef AIR_DAC_MODE_RUNTIME_CHANGE
bool app_hearing_aid_utils_get_hearing_test_mode(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
#endif /* AIR_DAC_MODE_RUNTIME_CHANGE */

bool app_hearing_aid_utils_set_switch(uint8_t *parameter);
bool app_hearing_aid_utils_set_level_index(uint8_t *parameter);
bool app_hearing_aid_utils_set_level_sync_switch(uint8_t *parameter);
bool app_hearing_aid_utils_set_volume_index(uint8_t *parameter);
bool app_hearing_aid_utils_set_volume_sync_switch(uint8_t *parameter);
bool app_hearing_aid_utils_set_mode_index(uint8_t *parameter);
bool app_hearing_aid_utils_set_specific_mode_table(uint8_t *parameter);
bool app_hearing_aid_utils_set_aea_configuration(uint8_t *parameter);
bool app_hearing_aid_utils_set_wnr_switch(uint8_t *parameter);
bool app_hearing_aid_utils_set_beam_forming_setting(uint8_t *parameter);
bool app_hearing_aid_utils_set_afc_config(uint8_t *parameter);
bool app_hearing_aid_utils_set_inr_config(uint8_t *parameter);
bool app_hearing_aid_utils_set_user_eq_switch(uint8_t *parameter);
bool app_hearing_aid_utils_set_user_eq_gain(uint8_t *parameter);
bool app_hearing_aid_utils_set_pure_tone_generator(uint8_t *parameter);
bool app_hearing_aid_utils_set_mix_mode_total_setting(uint8_t *parameter);
bool app_hearing_aid_utils_set_hearing_tuning_mode_switch(uint8_t *parameter);
bool app_hearing_aid_utils_set_mp_test_mode_switch(uint8_t *parameter);
bool app_hearing_aid_utils_set_restore_setting(uint8_t *parameter);
bool app_hearing_aid_utils_set_mute(uint8_t *parameter);
bool app_hearing_aid_utils_set_howling_detection(uint8_t *response);
bool app_hearing_aid_utils_set_mpo_adjustment(uint8_t *response);
bool app_hearing_aid_utils_set_in_ear_detection_switch(uint8_t *parameter);
bool app_hearing_aid_utils_set_passthrough_switch(uint8_t *parameter);
bool app_hearing_aid_utils_set_mic_channel(uint8_t *parameter);
bool app_hearing_aid_utils_set_trial_run(uint8_t *parameter);
bool app_hearing_aid_utils_set_mic_calibration_mode(uint8_t *parameter);
#ifdef AIR_DAC_MODE_RUNTIME_CHANGE
bool app_hearing_aid_utils_set_hearing_test_mode(uint8_t *parameter);
#endif /* AIR_DAC_MODE_RUNTIME_CHANGE */

uint32_t app_hearing_aid_utils_get_mic_calibration_data_combine_response_len();
uint32_t app_hearing_aid_utils_get_speaker_reference_combine_response_len();

#if 0
bool app_hearing_aid_utils_get_level_index_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_volume_index_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_specific_mode_table_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_afc_config_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_inr_config_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_user_eq_switch_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_user_eq_gain_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mix_mode_total_setting_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_feedback_detection_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_pure_tone_generator_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_passthrough_switch_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
#endif
bool app_hearing_aid_utils_combine_speaker_reference_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_hearing_tuning_mode_switch_combine_response(uint8_t *local, uint16_t local_length, uint8_t *remote, uint16_t remote_len, uint8_t *response, uint16_t *response_len);
bool app_hearing_aid_utils_get_mic_calibration_data_combine_response(bool is_local_response, uint8_t *response, uint16_t response_len, uint8_t *combine_response, uint16_t *combine_response_len);
bool app_hearing_aid_utils_get_speaker_reference_combine_response(bool is_local_response, uint8_t *response, uint16_t response_len, uint8_t *combine_response, uint16_t *combine_response_len);

bool app_hearing_aid_utils_ha_switch_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
bool app_hearing_aid_utils_level_index_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
bool app_hearing_aid_utils_volume_index_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
bool app_hearing_aid_utils_mode_index_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
bool app_hearing_aid_utils_aea_configuration_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
bool app_hearing_aid_utils_beam_forming_settings_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
bool app_hearing_aid_utils_speaker_reference_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
bool app_hearing_aid_utils_hearing_tuning_mode_switch_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
bool app_hearing_aid_utils_mp_test_mode_switch_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
bool app_hearing_aid_utils_feedback_detection_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
bool app_hearing_aid_utils_mic_channel_notify(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);

bool app_hearing_aid_utils_is_init_done();
bool app_hearing_aid_utils_is_ha_user_switch_on();
bool app_hearing_aid_utils_is_ha_running();
bool app_hearing_aid_utils_is_hearing_tuning_mode();
bool app_hearing_aid_utils_is_mp_test_mode();
bool app_hearing_aid_utils_is_beam_forming_enable();
bool app_hearing_aid_utils_is_aea_configure_enable();
bool app_hearing_aid_utils_is_level_sync_on(bool *on);
bool app_hearing_aid_utils_is_rssi_mix_switch_on(bool *on);

uint8_t app_hearing_aid_utils_get_master_mic_channel();
bool app_hearing_aid_utils_get_mode_index_simple(uint8_t *out_mode_index);

typedef struct {
    bool            a2dp_streaming;
    bool            sco_streaming;
    bool            vp_streaming;
    bool            in_ear;
    bool            less_than_threshold;
} app_hearing_aid_state_table_t;

#define APP_HEARING_AID_CHANGE_CAUSE_A2DP                       0x01
#define APP_HEARING_AID_CHANGE_CAUSE_SCO                        0x02
#define APP_HEARING_AID_CHANGE_CAUSE_VP                         0x03
#define APP_HEARING_AID_CHANGE_CAUSE_IN_EAR                     0x04
#define APP_HEARING_AID_CHANGE_CAUSE_BUTTON                     0x05
#define APP_HEARING_AID_CHANGE_CAUSE_RSSI                       0x06
#define APP_HEARING_AID_CHANGE_CAUSE_POWER_ON                   0x07
#define APP_HEARING_AID_CHANGE_CAUSE_REQUEST                    0x08
#define APP_HEARING_AID_CHANGE_CAUSE_RACE_CMD                   0x09
#define APP_HEARING_AID_CHANGE_CAUSE_MIX_TABLE_SWITCH           0x0A
#define APP_HEARING_AID_CHANGE_CAUSE_MIX_TABLE_DRC              0x0B
#define APP_HEARING_AID_CHANGE_CAUSE_REMOTE_USER_SWITCH         0x0C
#define APP_HEARING_AID_CHANGE_CAUSE_MASTER_MIC_CHANNEL_SWITCH  0x0D
typedef uint8_t     app_hearing_aid_change_cause_item;

bool app_hearing_aid_utils_mix_table_to_enable(app_hearing_aid_state_table_t *table, app_hearing_aid_change_cause_item where, bool *need_execute);

bool app_hearing_aid_utils_set_user_switch(bool enable);

bool app_hearing_aid_utils_control_ha(bool enable);

bool app_hearing_aid_utils_reload_configuration();

bool app_hearing_aid_utils_enable_mp_test_mode(bool enable);

typedef struct {
    bool        sync;
    bool        max;
    bool        min;
    uint8_t     l_index;
    uint8_t     r_index;
} app_hearing_aid_change_value_t;

bool app_hearing_aid_utils_adjust_level(uint8_t l_index, uint8_t r_index);
bool app_hearing_aid_utils_adjust_volume(uint8_t l_index, uint8_t r_index);
bool app_hearing_aid_utils_adjust_mode(uint8_t target_mode_index);

bool app_hearing_aid_utils_get_level_change_value(bool up, bool circular, app_hearing_aid_change_value_t *change_value);
bool app_hearing_aid_utils_get_volume_change_value(bool up, bool circular, app_hearing_aid_change_value_t *change_value);
bool app_hearing_aid_utils_get_mode_change_value(bool up, bool circular, uint8_t *out_mode, bool *max, bool *min);

bool app_hearing_aid_utils_hearing_tuning_mode_toggle(bool from_remote);
bool app_hearing_aid_utils_beam_forming_switch_toggle(bool enable);
bool app_hearing_aid_utils_aea_switch_toggle(bool enable);
bool app_hearing_aid_utils_master_mic_channel_switch_toggle(uint8_t target_channel);

bool app_hearing_aid_utils_save_user_settings();

bool app_hearing_aid_utils_control_fwk(bool enable, bool with_anc_path);

#define APP_HEARING_AID_DRC_A2DP            0x01
#define APP_HEARING_AID_DRC_SCO             0x02
#define APP_HEARING_AID_DRC_VP              0x03
typedef uint8_t     app_hearing_aid_drc_type_t;

bool app_hearing_aid_utils_is_drc_on(app_hearing_aid_drc_type_t type, bool *status);

bool app_hearing_aid_utils_is_drc_enable(bool a2dp_streaming, bool sco_streaming, bool vp_streaming);

bool app_hearing_aid_utils_is_sco_mix_mode_on();

bool app_hearing_aid_utils_is_music_mix_mode_on();

void app_hearing_aid_utils_sync_runtime_parameters(uint8_t *parameter, uint16_t parameter_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

#endif /* __APP_HEARING_AID_UTILS_H__ */


