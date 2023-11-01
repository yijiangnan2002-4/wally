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

#include "app_hearing_aid_config.h"
#include "app_hearing_aid_utils.h"
#include "app_hearing_aid_activity.h"
#include "app_hear_through_race_cmd_handler.h"
#include "apps_debug.h"
#include "FreeRTOS.h"
#include "nvkey_id_list.h"
#include "nvkey.h"
#include "string.h"

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)

#define APP_HA_CONFIG_TAG   "[HearingAid][Config]"

const app_hearing_aid_execute_handler_t app_ha_exe_handler_list[] = {
    {
        // 0x0000, Not Used
        .execute_get_where = APP_HEARING_AID_EXECUTE_NONE,
        .execute_set_where = APP_HEARING_AID_EXECUTE_NONE,
        .notify_need_sync = false,
        .need_execute_set_sync = false,
        .ha_cmd_get_handler = NULL,
        .ha_cmd_set_handler = NULL,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x0001, HA_SWITCH, Get, Set, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = false,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_switch,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_switch,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = app_hearing_aid_utils_ha_switch_notify,
    },
    {
        // 0x0002, HA_LEVEL_INDEX, Get, Set, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = true,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_level_index,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_level_index,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_level_index_combine_response,
        .ha_notify = app_hearing_aid_utils_level_index_notify,
    },
    {
        // 0x0003, HA_LEVEL_SYNC_SWITCH, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = false,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_level_sync_switch,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_level_sync_switch,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x0004, HA_LEVEL_MODE_MAXCOUNT, Get
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_NONE,
        .notify_need_sync = false,
        .need_execute_set_sync = false,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_mode_max_count,
        .ha_cmd_set_handler = NULL,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x0005, HA_VOLUME_INDEX, Get, Set, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = true,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_volume_index,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_volume_index,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_volume_index_combine_response,
        .ha_notify = app_hearing_aid_utils_volume_index_notify,
    },
    {
        // 0x0006, HA_VOLUME_SYNC_SWITCH, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = false,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_volume_sync_switch,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_volume_sync_switch,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x0007, HA_MODE_INDEX, Get, Set, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_mode_index,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_mode_index,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = app_hearing_aid_utils_mode_index_notify,
    },
    {
        // 0x0008, HA_SPECIFIC_MODE_TABLE, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_specific_mode_table,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_specific_mode_table,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_specific_mode_table_combine_response,
        .ha_notify = NULL,
    },
    {
        // 0x0009, HA_AEA_CONFIGURATION, Get, Set, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_aea_configuration,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_aea_configuration,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = app_hearing_aid_utils_aea_configuration_notify,
    },
    {
        // 0x000A, HA_WNR_SWITCH, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_wnr_switch,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_wnr_switch,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x000B, BEAMFORMING_SETTINGS, Get, Set, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_beam_forming_setting,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_beam_forming_setting,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = app_hearing_aid_utils_beam_forming_settings_notify,
    },
    {
        // 0x000C, HA_AFC_CONFIG, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_afc_config,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_afc_config,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_afc_config_combine_response,
        .ha_notify = NULL,
    },
    {
        // 0x000D, HA_INR_CONFIG, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_inr_config,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_inr_config,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_inr_config_combine_response,
        .ha_notify = NULL,
    },
    {
        // 0x000E, HA_USEREQ_SWITCH, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_user_eq_switch,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_user_eq_switch,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_user_eq_switch_combine_response,
        .ha_notify = NULL,
    },
    {
        // 0x000F, HA_USEREQ_GAIN, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_user_eq_gain,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_user_eq_gain,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_user_eq_gain_combine_response,
        .ha_notify = NULL,
    },
    {
        // 0x0010, HA_SPEAKER_REFERENCE, Get, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .execute_set_where = APP_HEARING_AID_EXECUTE_NONE,
        .notify_need_sync = true,
        .need_execute_set_sync = false,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_speaker_reference,
        .ha_cmd_set_handler = NULL,
        .ha_cmd_get_combine_response_len = app_hearing_aid_utils_get_speaker_reference_combine_response_len,
        .ha_cmd_get_combine_handler = app_hearing_aid_utils_get_speaker_reference_combine_response,
        .ha_notify = app_hearing_aid_utils_speaker_reference_notify,
    },
    {
        // 0x0011, HA_PURETONE_GENERATOR, Get, Set, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_pure_tone_generator,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_pure_tone_generator,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_pure_tone_generator_combine_response,
        .ha_notify = NULL,
    },
    {
        // 0x0012, HA_MIXMODE_TOTALSETTING, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_mix_mode_total_setting,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_mix_mode_total_setting,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_mix_mode_total_setting_combine_response,
        .ha_notify = NULL,
    },
    {
        // 0x0013, HA_HEARINGTUNINGMODE_SWITCH, Get, Set, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = true,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_hearing_tuning_mode_switch,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_hearing_tuning_mode_switch,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_hearing_tuning_mode_switch_combine_response,
        .ha_notify = app_hearing_aid_utils_hearing_tuning_mode_switch_notify,
    },
    {
        // 0x0014, HA_MPTESTMODE_SWITCH, Get, Set, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_mp_test_mode_switch,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_mp_test_mode_switch,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = app_hearing_aid_utils_mp_test_mode_switch_notify,
    },
    {
        // 0x0015, HA_RESTORE_SETTING, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_NONE,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = NULL,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_restore_setting,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x0016, HA_FEEDBACK_DETECTION, Get, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .execute_set_where = APP_HEARING_AID_EXECUTE_NONE,
        .notify_need_sync = true,
        .need_execute_set_sync = false,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_feedback_detection,
        .ha_cmd_set_handler = NULL,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_feedback_detection_combine_response,
        .ha_notify = app_hearing_aid_utils_feedback_detection_notify,
    },
    {
        // 0x0017, HA_MUTE, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_NONE,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = NULL,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_mute,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x0018, HA_HOWLING_DETECTION, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_howling_detection,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_howling_detection,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x0019, HA_MPO_ADJUSTMENT, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_mpo_adjustment,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_mpo_adjustment,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x001A, HA_INEAR_DETECTION, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_in_ear_detection_switch,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_in_ear_detection_switch,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x001B, HA_PASSTHROUGH_SWITCH, Get, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_passthrough_switch,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_passthrough_switch,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,//app_hearing_aid_utils_get_passthrough_switch_combine_response,
        .ha_notify = NULL,
    },
    {
        // 0x001C, HA_MIC_CHANNEL, Get, Set, Notify
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_mic_channel,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_mic_channel,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = app_hearing_aid_utils_mic_channel_notify,
    },
    {
        // 0x001D, HA_TRIAL_RUN, Set
        .execute_get_where = APP_HEARING_AID_EXECUTE_NONE,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = NULL,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_trial_run,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x001E, HA_MIC_CALIBRATION_MODE, Set,Get
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_AGENT,
        .execute_set_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .notify_need_sync = false,
        .need_execute_set_sync = true,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_mic_calibration_mode,
        .ha_cmd_set_handler = app_hearing_aid_utils_set_mic_calibration_mode,
        .ha_cmd_get_combine_response_len = NULL,
        .ha_cmd_get_combine_handler = NULL,
        .ha_notify = NULL,
    },
    {
        // 0x001F, HA_MIC_CALIBRATION_DATA, Get
        .execute_get_where = APP_HEARING_AID_EXECUTE_ON_BOTH,
        .execute_set_where = APP_HEARING_AID_EXECUTE_NONE,
        .notify_need_sync = false,
        .need_execute_set_sync = false,
        .ha_cmd_get_handler = app_hearing_aid_utils_get_mic_calibration_data,
        .ha_cmd_set_handler = NULL,
        .ha_cmd_get_combine_response_len = app_hearing_aid_utils_get_mic_calibration_data_combine_response_len,
        .ha_cmd_get_combine_handler = app_hearing_aid_utils_get_mic_calibration_data_combine_response,
        .ha_notify = NULL,
    }
};

const uint8_t app_ha_exe_handler_count = sizeof(app_ha_exe_handler_list) / sizeof(app_ha_exe_handler_list[0]);

uint8_t app_hearing_aid_config_get_where_to_execute(uint8_t code, uint16_t type)
{
    uint8_t execute_where = APP_HEARING_AID_EXECUTE_NONE;
    if (code == APP_HEAR_THROUGH_CMD_OP_CODE_GET) {
        execute_where = app_ha_exe_handler_list[type].execute_get_where;
    }

    if (code == APP_HEAR_THROUGH_CMD_OP_CODE_SET) {
        execute_where = app_ha_exe_handler_list[type].execute_set_where;
    }

    return execute_where;
}

bool app_hearing_aid_config_get_notify_sync(uint16_t type)
{
    return app_ha_exe_handler_list[type].notify_need_sync;
}

bool app_hearing_aid_config_get_need_execute_set_cmd_sync(uint16_t type)
{
    return app_ha_exe_handler_list[type].need_execute_set_sync;
}

#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

