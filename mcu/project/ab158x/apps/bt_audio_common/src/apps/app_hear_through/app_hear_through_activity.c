/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#include "app_hear_through_activity.h"
#include "app_hear_through_race_cmd_handler.h"
#include "app_hear_through_adv.h"
#include "apps_config_event_list.h"
#include "apps_events_event_group.h"
#include "app_hear_through_storage.h"
#include "apps_aws_sync_event.h"
#include "bt_connection_manager.h"
#include "bt_device_manager.h"
#include "bt_device_manager_power.h"
#include "apps_events_bt_event.h"
#include "ui_shell_manager.h"
#include "apps_debug.h"
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#include "audio_anc_psap_control.h"
#endif
#if defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)
#include "audio_anc_vivid_pt_api.h"
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */
#include "race_cmd.h"
#include "race_event.h"
#include "stddef.h"
#include "bt_system.h"
#include "hal_platform.h"
#include "hal_gpt.h"
#include "voice_prompt_api.h"
#include "apps_events_interaction_event.h"
#ifdef AIR_TWS_ENABLE
#include "bt_aws_mce_srv.h"
#endif /* AIR_TWS_ENABLE */
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#include "app_hearing_aid_activity.h"
#include "app_hearing_aid_utils.h"
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
#ifdef MTK_ANC_ENABLE
#include "app_anc_service.h"
#include "anc_control_api.h"
#endif /* MTK_ANC_ENABLE */

#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#include "apps_events_battery_event.h"
#endif /* MTK_BATTERY_MANAGEMENT_ENABLE */
#ifdef AIR_SMART_CHARGER_ENABLE
#include "app_smcharger.h"
#endif /* AIR_SMART_CHARGER_ENABLE */

#include "app_power_save_utils.h"

#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "bt_sink_srv_le_cap_stream.h"
#endif /* AIR_LE_AUDIO_BIS_ENABLE */

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
#include "leakage_detection_control.h"
#endif /* MTK_LEAKAGE_DETECTION_ENABLE */

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE

#define APP_HEAR_THROUGH_ACT_TAG        "[HearThrough][Activity]"

/**
 * @brief Add feature option to support shut down system when HT is not enabled.
 */
#define AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED        1

#define APP_HEAR_THROUGH_SYNC_EVENT_DEFAULT_TIMEOUT             200 // 200ms

#define APP_HEAR_THROUGH_MODE_LENGTH                            5
#define APP_HEAR_THROUGH_OP_PARAMETER_INDEX                     3

#define APP_HEARING_AID_CONFIG_TYPE_MAXIMUM                     0x1000
#define APP_HEAR_THROUGH_CONFIG_TYPE_MAXIMUM                    0x2000

#define APP_HEAR_THROUGH_CONFIG_BASE                            0x1000
#define APP_HEAR_THROUGH_CONFIG_TYPE_SWITCH                     0x1001
#define APP_HEAR_THROUGH_CONFIG_TYPE_MODE                       0x1002

#define APP_HEAR_THROUGH_VIVID_CONFIG_BASE                      0x2000
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
#define APP_VIVID_PT_CONFIG_TYPE_AFC_SWITCH                     0x2001
#define APP_VIVID_PT_CONFIG_TYPE_LDNR_SWITCH                    0x2002
#define APP_VIVID_PT_CONFIG_TYPE_BY_PASS_SWITCH                 0x2003
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */

#ifdef AIR_TWS_ENABLE
#define APP_HEAR_THROUGH_AWS_EVENT_ID_BASE                      0x3000
#define APP_HEAR_THROUGH_AWS_EVENT_ID_CONFIG_SYNC_REQUEST       0x3001
#define APP_HEAR_THROUGH_AWS_EVENT_ID_CONFIG_SYNC               0x3002
#define APP_HEAR_THROUGH_AWS_EVENT_ID_AMBIENT_CONTROL           0x3003
#define APP_HEAR_THROUGH_AWS_EVENT_ID_ANC_SWITCHED              0x3004
#define APP_HEAR_THROUGH_AWS_EVENT_ID_HEAR_THROUGH_KEY_TO_OFF   0x3005
#endif /* AIR_TWS_ENABLE */

#define APP_HEAR_THROUGH_MODE_SWITCH_INDEX_OFF                  0x00
#define APP_HEAR_THROUGH_MODE_SWITCH_INDEX_HEAR_THROUGH         0x01
#define APP_HEAR_THROUGH_MODE_SWITCH_INDEX_ANC                  0x02

typedef struct {
#ifdef MTK_RACE_EVENT_ID_ENABLE
    int32_t                             race_register_id;
#endif /* MTK_RACE_EVENT_ID_ENABLE */
    bool                                is_hear_through_enabled;
    bool                                is_power_on_vp_playing;
    bool                                is_power_on_vp_played;
    bool                                is_power_on_ht_executed;
    bool                                is_powering_off;
    bool                                is_anc_changed;
    bool                                trigger_from_key;
    bool                                init_done;
    bool                                hear_through_key_to_off;
    uint8_t                             mode_index;
    bool                                is_charger_in;
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
    bool                                is_hear_through_enabled_before_ota;
    bool                                is_ota_ongoing;
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */
} app_hear_through_context_t;

static app_hear_through_context_t app_hear_through_ctx;

typedef struct {
    llf_type_t type;
    llf_control_event_t event;
    llf_status_t result;
} app_hear_through_control_callback_parameter_t;

#ifdef AIR_TWS_ENABLE
typedef struct {
    bt_clock_t          target_clock;
    uint32_t            delay_ms;
    uint16_t            op_data_len;
    uint8_t             op_data[0];
} __attribute__((packed)) app_hear_through_aws_operate_command_t;

typedef struct {
    uint8_t             mode_index;
} __attribute__((packed)) app_hear_through_sync_ambient_control_switch_t;

typedef struct {
    bool                from_key;
    bool                is_anc_changed; // Fix issue - 47152
    bool                is_need_sync_operate;
    uint8_t             hear_through_switch;
    uint8_t             hear_through_mode;
    uint8_t             ambient_control_index;
} __attribute__((packed)) app_hear_through_sync_configuration_t;

#endif /* AIR_TWS_ENABLE */

static bool app_hear_through_switch_on_off(bool need_store, bool enable);
static bool app_hear_through_is_aws_connected();
static bool app_hear_through_activity_is_out_case();

static uint8_t app_hear_through_map_middleware_type_to_app_mode(uint8_t type)
{
    if (type == LLF_TYPE_HEARING_AID) {
        return APP_HEAR_THROUGH_MODE_HA_PSAP;
    } else if (type == LLF_TYPE_VIVID_PT) {
        return APP_HEAR_THROUGH_MODE_VIVID_PT;
    } else {
        return 0xFF;
    }
}

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
static app_power_saving_target_mode_t app_hear_through_activity_get_power_saving_mode_callback(void)
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_get_power_saving_mode_callback] Hear through enabled : %d",
                        1,
                        app_hear_through_ctx.is_hear_through_enabled);

    if (app_hear_through_ctx.is_hear_through_enabled == true) {
        return APP_POWER_SAVING_TARGET_MODE_NORMAL;
    }
    return APP_POWER_SAVING_TARGET_MODE_SYSTEM_OFF;
}

static void app_hear_through_activity_notify_power_saving_mode_changed()
{
    app_power_save_utils_notify_mode_changed(false, app_hear_through_activity_get_power_saving_mode_callback);
}
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */

#ifdef AIR_TWS_ENABLE
static bool app_hear_through_is_aws_connected()
{
    bt_aws_mce_srv_link_type_t link_type = bt_aws_mce_srv_get_link_type();
    if (link_type == BT_AWS_MCE_SRV_LINK_NONE) {
        return false;
    }
    return true;
}
#endif /* AIR_TWS_ENABLE */

static void app_hear_through_activity_send_set_cmd_response(uint16_t type, bool result)
{
#ifdef AIR_TWS_ENABLE
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
#endif /* AIR_TWS_ENABLE */
        app_hear_through_race_cmd_send_set_response(type, result);
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
}

static void app_hear_through_update_ambient_control_mode()
{
    if (app_hear_through_storage_get_hear_through_switch() == true) {
        app_hear_through_ctx.mode_index = APP_HEAR_THROUGH_MODE_SWITCH_INDEX_HEAR_THROUGH;
    } else {
        app_hear_through_ctx.is_anc_changed = true;
        bool is_anc_enabled = app_anc_service_is_anc_enabled();
        if (is_anc_enabled == true) {
            app_hear_through_ctx.mode_index = APP_HEAR_THROUGH_MODE_SWITCH_INDEX_ANC;
        } else {
            app_hear_through_ctx.mode_index = APP_HEAR_THROUGH_MODE_SWITCH_INDEX_OFF;
        }
    }
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_update_ambient_control_mode] ambient control mode_index : %d", 1, app_hear_through_ctx.mode_index);
}

static void app_hear_through_notify_switch_state_change(bool on)
{
#ifdef AIR_TWS_ENABLE
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_notify_switch_state_change] enable : %d, role : 0x%02x",
                        2,
                        on,
                        role);

    if (role == BT_AWS_MCE_ROLE_AGENT) {
        app_hear_through_race_cmd_send_notification(APP_HEAR_THROUGH_CONFIG_TYPE_SWITCH,
                                                (uint8_t *)&on,
                                                sizeof(uint8_t));
    }
#else
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_notify_switch_state_change] switch_on : %d",
                        1,
                        on);

    app_hear_through_race_cmd_send_notification(APP_HEAR_THROUGH_CONFIG_TYPE_SWITCH,
                                                (uint8_t *)&on,
                                                sizeof(uint8_t));
#endif /* AIR_TWS_ENABLE */
}

static bool app_hear_through_activity_handle_switch_set_cmd(void *extra_data, uint32_t data_len)
{
    bool ret_value = false;
    bool is_out_case = app_hear_through_activity_is_out_case();
    uint8_t *parameter = (uint8_t *)extra_data;
    app_hear_through_mode_t running_mode = app_hear_through_storage_get_hear_through_mode();

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_switch_set_cmd] out_case : %d, running_mode : %d, trigger_from : %d, switch change to : %d, is_ota_ongoing : %d",
                        5,
                        is_out_case,
                        running_mode,
                        app_hear_through_ctx.trigger_from_key,
                        parameter[0],
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
                        app_hear_through_ctx.is_ota_ongoing
#else
                        0
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */
                        );

    if (parameter[0] == true) {
        app_hear_through_ctx.is_anc_changed = false;
    }

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    if ((running_mode == APP_HEAR_THROUGH_MODE_HA_PSAP)
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
            && (app_hear_through_ctx.is_ota_ongoing == false)
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */
        ) {
        app_hearing_aid_activity_set_user_switch(false, parameter[0]);
    }
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

    app_hear_through_ctx.trigger_from_key = false;
    app_hear_through_ctx.hear_through_key_to_off = false;

#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
    if (app_hear_through_ctx.is_ota_ongoing == true) {
        app_hear_through_storage_set_hear_through_switch(parameter[0]);
        app_hear_through_ctx.is_hear_through_enabled_before_ota = parameter[0];

        app_hear_through_activity_send_set_cmd_response(APP_HEAR_THROUGH_CONFIG_TYPE_SWITCH, true);

        return true;
    }
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */

    ret_value = app_hear_through_switch_on_off(true, parameter[0]);

    app_hear_through_update_ambient_control_mode();

    app_hear_through_activity_send_set_cmd_response(APP_HEAR_THROUGH_CONFIG_TYPE_SWITCH, ret_value);

    return ret_value;
}

static bool app_hear_through_activity_handle_mode_set_cmd(void *extra_data, uint32_t data_len)
{
    bool ret_value = false;
    bool is_out_case = app_hear_through_activity_is_out_case();
    uint8_t *parameter = (uint8_t *)extra_data;
    bool switch_state = app_hear_through_storage_get_hear_through_switch();

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_mode_set_cmd] out_case : %d, switch_state : %d, mode_to_be : %d, is_ota_ongoing : %d",
                        4,
                        is_out_case,
                        switch_state,
                        parameter[0],
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
                        app_hear_through_ctx.is_ota_ongoing
#else
                        0
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */
                        );

    if ((parameter[0] != APP_HEAR_THROUGH_MODE_VIVID_PT)
        && (parameter[0] != APP_HEAR_THROUGH_MODE_HA_PSAP)) {
        app_hear_through_activity_send_set_cmd_response(APP_HEAR_THROUGH_CONFIG_TYPE_MODE, false);
        return false;
    }

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
    if (app_hear_through_ctx.is_ota_ongoing == false) {
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */
        if (parameter[0] == APP_HEAR_THROUGH_MODE_HA_PSAP) {
            app_hearing_aid_activity_set_user_switch(false, switch_state);
        } else {
            app_hearing_aid_activity_set_user_switch(false, false);
        }
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
    }
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

    app_hear_through_storage_set_hear_through_mode(parameter[0]);

    app_hear_through_activity_send_set_cmd_response(APP_HEAR_THROUGH_CONFIG_TYPE_MODE, true);

    return ret_value;
}

#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
static bool app_hear_through_activity_handle_vivid_afc_set_cmd(void *extra_data, uint32_t data_len)
{
    bool ret_value = false;

    uint8_t *parameter = (uint8_t *)extra_data;

    audio_vivid_pt_status_t afc_switch_result = audio_anc_vivid_pt_control_afc_set_switch(parameter[0]);
    ret_value = (afc_switch_result == AUDIO_VIVID_PT_STATUS_SUCCESS) ? true : false;

    app_hear_through_activity_send_set_cmd_response(APP_VIVID_PT_CONFIG_TYPE_AFC_SWITCH, ret_value);

    return ret_value;
}

static bool app_hear_through_activity_handle_vivid_ldnr_set_cmd(void *extra_data, uint32_t data_len)
{
    bool ret_value = false;

    uint8_t *parameter = (uint8_t *)extra_data;

    audio_vivid_pt_status_t nr_switch_result = audio_anc_vivid_pt_control_nr_set_switch(parameter[0]);
    ret_value = (nr_switch_result == AUDIO_VIVID_PT_STATUS_SUCCESS) ? true : false;

    app_hear_through_activity_send_set_cmd_response(APP_VIVID_PT_CONFIG_TYPE_LDNR_SWITCH, ret_value);

    return ret_value;
}

static bool app_hear_through_activity_handle_vivid_by_pass_set_cmd(void *extra_data, uint32_t data_len)
{
    bool ret_value = false;

    uint8_t *parameter = (uint8_t *)extra_data;

    audio_vivid_pt_status_t by_pass_switch_result = audio_anc_vivid_pt_control_bypass_set_switch(parameter[0]);
    ret_value = (by_pass_switch_result == AUDIO_VIVID_PT_STATUS_SUCCESS) ? true : false;

    app_hear_through_activity_send_set_cmd_response(APP_VIVID_PT_CONFIG_TYPE_BY_PASS_SWITCH, ret_value);

    return ret_value;
}
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */

static bool app_hear_through_activity_handle_set_cmd(uint16_t config_type, void *extra_data, uint32_t data_len)
{
    bool ret_value = false;

    if (config_type == APP_HEAR_THROUGH_CONFIG_TYPE_SWITCH) {
        ret_value = app_hear_through_activity_handle_switch_set_cmd(extra_data, data_len);
    } else if (config_type == APP_HEAR_THROUGH_CONFIG_TYPE_MODE) {
        ret_value = app_hear_through_activity_handle_mode_set_cmd(extra_data, data_len);
    }
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
    else if (config_type == APP_VIVID_PT_CONFIG_TYPE_AFC_SWITCH) {
        ret_value = app_hear_through_activity_handle_vivid_afc_set_cmd(extra_data, data_len);
    } else if (config_type == APP_VIVID_PT_CONFIG_TYPE_LDNR_SWITCH) {
        ret_value = app_hear_through_activity_handle_vivid_ldnr_set_cmd(extra_data, data_len);
    } else if (config_type == APP_VIVID_PT_CONFIG_TYPE_BY_PASS_SWITCH) {
        ret_value = app_hear_through_activity_handle_vivid_by_pass_set_cmd(extra_data, data_len);
    }
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */

    return ret_value;
}

static void app_hear_through_activity_handle_get_cmd(uint16_t config_type)
{
    uint8_t state = 0;
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
    audio_vivid_pt_status_t status = AUDIO_VIVID_PT_STATUS_SUCCESS;
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */

    uint8_t response[APP_HEAR_THROUGH_MODE_LENGTH] = {0};
    uint8_t response_len = 0;
    bool ret_result = true;

    if (config_type == APP_HEAR_THROUGH_CONFIG_TYPE_SWITCH) {
        state = app_hear_through_storage_get_hear_through_switch();
    } else if (config_type == APP_HEAR_THROUGH_CONFIG_TYPE_MODE) {
        state = app_hear_through_storage_get_hear_through_mode();
    }
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
    else if (config_type == APP_VIVID_PT_CONFIG_TYPE_AFC_SWITCH) {
        status = audio_anc_vivid_pt_control_afc_get_switch((bool *)&state);
    } else if (config_type == APP_VIVID_PT_CONFIG_TYPE_LDNR_SWITCH) {
        status = audio_anc_vivid_pt_control_nr_get_switch((bool *)&state);
    } else if (config_type == APP_VIVID_PT_CONFIG_TYPE_BY_PASS_SWITCH) {
        status = audio_anc_vivid_pt_control_bypass_get_switch((bool *)&state);
    }
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */
    else {
        return;
    }

    response[0] = state;

#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
    ret_result = (status == AUDIO_VIVID_PT_STATUS_SUCCESS) ? true : false;
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */

    if (config_type != APP_HEAR_THROUGH_CONFIG_TYPE_MODE) {
        response_len = 1;
    } else {
        response_len = APP_HEAR_THROUGH_MODE_LENGTH;
    }

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_get_cmd] Get configure type : 0x%04x, result : %d, response_len : %d",
                        3,
                        config_type,
                        ret_result,
                        response_len);

    if (ret_result == true) {
        app_hear_through_race_cmd_send_get_response(config_type, response, response_len);
    } else {
        app_hear_through_race_cmd_send_get_response(config_type, NULL, 0);
    }
}

#if defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE) || defined(AIR_HW_VIVID_PT_ENABLE)
static bool app_hear_through_control_vivid_pt(bool enable)
{
    audio_anc_control_result_t         control_ret;

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_control_vivid_pt] Enable : %d", 1, enable);

    if (enable == true) {
        audio_anc_control_filter_id_t      target_filter_id;
        audio_anc_control_type_t           target_anc_type;
        audio_anc_control_misc_t           local_misc = {0};

#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
        uint32_t mic_path = 0;
        audio_anc_vivid_pt_get_mic_input_path(&mic_path);

        local_misc.type_mask_param.ANC_path_mask = app_hear_through_storage_get_anc_path_mask_for_vivid_pt();
        target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_SW_VIVID | mic_path;
        target_filter_id     = AUDIO_ANC_CONTROL_SW_VIVID_PASS_THRU_FILTER_DEFAULT; //1~4
#else
        local_misc.type_mask_param.ANC_path_mask = 0;
        target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_HW_VIVID;
        target_filter_id     = AUDIO_ANC_CONTROL_HW_VIVID_PASS_THRU_FILTER_DEFAULT;
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */

        control_ret = audio_anc_control_enable(target_filter_id, target_anc_type, &local_misc);
    } else {
        control_ret = audio_anc_control_disable(NULL);
    }

    return (control_ret == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) ? true : false;
}
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE || AIR_HW_VIVID_PT_ENABLE */

static bool app_hear_through_switch_on_off(bool need_store, bool enable)
{
    bool ret_value = false;

    app_hear_through_mode_t running_mode = app_hear_through_storage_get_hear_through_mode();
    bool ht_switch = app_hear_through_storage_get_hear_through_switch();
    bool is_out_case = app_hear_through_activity_is_out_case();

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_switch_on_off] enable : %d, running_mode : %d, ht_switch : %d, out_case : %d, trigger_from_key : %d, powering_off : %d",
                        6,
                        enable,
                        running_mode,
                        ht_switch,
                        is_out_case,
                        app_hear_through_ctx.trigger_from_key,
                        app_hear_through_ctx.is_powering_off);

    if (enable == true) {
        app_anc_service_set_hear_through_enabled(enable);
    }

    if (need_store == true) {
        app_hear_through_storage_set_hear_through_switch(enable);
    }

#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
    if ((app_hear_through_ctx.is_ota_ongoing == true) && (enable == true)) {
        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_switch_on_off] OTA is ongoing, do not enable Hear Through", 0);
        return false;
    }
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */

    if ((is_out_case == false) && (enable == true)) {
        return false;
    }

    if (app_hear_through_ctx.is_powering_off == true) {
        return false;
    }

    if (app_hear_through_ctx.trigger_from_key == true) {
        if (ht_switch != enable) {
            app_hear_through_notify_switch_state_change(enable);
        }
    }

    if (running_mode == APP_HEAR_THROUGH_MODE_VIVID_PT) {
#if defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE) || defined(AIR_HW_VIVID_PT_ENABLE)
        ret_value = app_hear_through_control_vivid_pt(enable);
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE || AIR_HW_VIVID_PT_ENABLE */

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
        app_hearing_aid_activity_set_open_fwk_done(false);
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

    } else if (running_mode == APP_HEAR_THROUGH_MODE_HA_PSAP) {
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
        if (enable == true) {
            ret_value = app_hearing_aid_activity_open_hearing_aid_fwk();
        } else {
            ret_value = app_hearing_aid_activity_disable_hearing_aid(app_hear_through_ctx.trigger_from_key);
        }
#else
        assert(false && "Not enable AIR_HEARING_AID_ENABLE feature option");
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
    }

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_switch_on_off] result : %d",
                        1,
                        ret_value);

    return ret_value;
}

static void app_hearing_through_activity_leave_hear_through_mode()
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hearing_through_activity_leave_hear_through_mode] Leave hear through mode", 0);

    app_hear_through_storage_set_hear_through_switch(false);

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    app_hearing_aid_activity_set_user_switch(false, false);
    app_hearing_aid_activity_set_open_fwk_done(false);
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

    app_hear_through_ctx.is_hear_through_enabled = false;

    app_anc_service_set_hear_through_enabled(false);

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
    app_hear_through_activity_notify_power_saving_mode_changed();
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */

    app_hear_through_notify_switch_state_change(false);
}

static void app_hear_through_activity_handle_ambient_control_switch()
{
    /**
     * @brief Make the anc_changed to be true firstly
     */
    app_hear_through_ctx.is_anc_changed = true;

    switch (app_hear_through_ctx.mode_index) {
        case APP_HEAR_THROUGH_MODE_SWITCH_INDEX_OFF: {
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_ambient_control_switch] OFF", 0);
            app_hear_through_ctx.trigger_from_key = false;
            app_hearing_through_activity_leave_hear_through_mode();
            app_anc_service_disable_without_notify_hear_through();
        }
        break;
        case APP_HEAR_THROUGH_MODE_SWITCH_INDEX_HEAR_THROUGH: {
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_ambient_control_switch] Enable Hear Through", 0);
		// richard for customer UI spec.
		voice_prompt_play_vp_hearing_through();
		
            app_hear_through_ctx.trigger_from_key = true;
            /**
             * @brief if change to hear through mode, make it to be false.
             */
            app_hear_through_ctx.is_anc_changed = false;

            app_hear_through_ctx.hear_through_key_to_off = false;

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
            uint8_t hear_through_mode = app_hear_through_storage_get_hear_through_mode();
            if ((hear_through_mode == APP_HEAR_THROUGH_MODE_HA_PSAP)
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
                && (app_hear_through_ctx.is_ota_ongoing == false)
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */
                ) {
                app_hearing_aid_activity_set_user_switch(false, true);
            }
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

            app_hear_through_switch_on_off(true, true);
        }
        break;
        case APP_HEAR_THROUGH_MODE_SWITCH_INDEX_ANC: {
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_ambient_control_switch] Enable ANC", 0);
		// richard for customer UI spec.
		voice_prompt_play_vp_anc_on();

            app_hearing_through_activity_leave_hear_through_mode();
            app_anc_service_reset_hear_through_anc(true);
        }
        break;
        default:
        return;
    }
}

static void app_hear_through_activity_handle_anc_state_changed()
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_anc_state_changed] ANC state changed", 0);

    app_hear_through_ctx.is_anc_changed = true;
    app_hear_through_ctx.is_hear_through_enabled = false;

    app_hear_through_ctx.mode_index = APP_HEAR_THROUGH_MODE_SWITCH_INDEX_ANC;

    app_anc_service_set_hear_through_enabled(false);

    app_hear_through_storage_set_hear_through_switch(false);

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    app_hearing_aid_activity_set_user_switch(false, false);
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
}

static bool app_hear_through_activity_handle_middleware_control_callback(void *extra_data, uint32_t extra_data_len)
{
    if ((extra_data == NULL) || (extra_data_len == 0)) {
        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_middleware_control_callback] parameter error, extra_data : 0x%x, data_len : %d",
                            2,
                            extra_data,
                            extra_data_len);
        return false;
    }

    app_hear_through_control_callback_parameter_t *parameter = (app_hear_through_control_callback_parameter_t *)extra_data;

    if (app_hear_through_ctx.init_done == false) {
        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_middleware_control_callback] Not init_done", 0);
        return false;
    }

    uint8_t type_to_mode = app_hear_through_map_middleware_type_to_app_mode(parameter->type);
    uint8_t running_mode = app_hear_through_storage_get_hear_through_mode();

    llf_type_t type = parameter->type;
    llf_control_event_t event = parameter->event;
    llf_status_t result = parameter->result;

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_middleware_control_callback] event : %d, type : %d, result : %d, type_to_mode : %d, running_mode : %d",
                        5,
                        event,
                        type,
                        result,
                        type_to_mode,
                        running_mode);

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_middleware_control_callback] is_hear_through_enabled : %d, anc_changed : %d, trigger_from_key : %d, hear_through_key_to_off : %d",
                        4,
                        app_hear_through_ctx.is_hear_through_enabled,
                        app_hear_through_ctx.is_anc_changed,
                        app_hear_through_ctx.trigger_from_key,
                        app_hear_through_ctx.hear_through_key_to_off);

    if ((app_hear_through_ctx.is_powering_off == true)
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
            && (app_hearing_aid_is_mp_test_mode() == false)
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
        ) {
        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_middleware_control_callback] powering off, ignore", 0);
        return true;
    }

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
    bool need_notify_power_off_state = false;
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */

    if ((result == LLF_STATUS_SUCCESS)
        && ((event == LLF_CONTROL_EVENT_ON) || (event == LLF_CONTROL_EVENT_OFF))) {

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
        if (type == LLF_TYPE_HEARING_AID) {
            if (event == LLF_CONTROL_EVENT_ON) {

                APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_middleware_control_callback] Open HA/PSAP FWK Done, is_anc_changed : %d",
                                    1,
                                    app_hear_through_ctx.is_anc_changed);

                app_hear_through_ctx.trigger_from_key = false;

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
                need_notify_power_off_state = true;
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */

                if (app_hear_through_ctx.is_anc_changed == true) {
                    app_hearing_aid_activity_set_open_fwk_done(false);
                    app_hear_through_ctx.is_hear_through_enabled = false;
                    app_hearing_aid_activity_set_power_on_played();
                } else {
                    app_hearing_aid_activity_set_open_fwk_done(true);
                    app_hear_through_ctx.is_hear_through_enabled = true;

                    if (app_hearing_aid_is_need_enable_ha() == true) {
                        app_hearing_aid_activity_enable_hearing_aid(app_hear_through_ctx.trigger_from_key);
                    }
                }
            }
            if (event == LLF_CONTROL_EVENT_OFF) {
                APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_middleware_control_callback] Close HA/PSAP FWK Done", 0);

                app_hearing_aid_activity_set_open_fwk_done(false);
                app_hear_through_ctx.is_hear_through_enabled = false;

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
                need_notify_power_off_state = true;
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */
            }
        } else {
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
            if (type ==  LLF_TYPE_VIVID_PT) {

                APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_middleware_control_callback] Control Vivid result : %d",
                                    1,
                                    ((event == LLF_CONTROL_EVENT_ON) ? true : false));

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
                bool old_ht_enabled = app_hear_through_ctx.is_hear_through_enabled;
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */

                app_hear_through_ctx.is_hear_through_enabled = ((event == LLF_CONTROL_EVENT_ON) ? true : false);

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
                if (old_ht_enabled != app_hear_through_ctx.is_hear_through_enabled) {
                    need_notify_power_off_state = true;
                }
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */
            }
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
        }
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
    }

    /**
     * @brief When Hear through disabled, resume ANC.
     * Only reset the anc state, if the modification is not caused by the anc hanged.
     */
    if ((app_hear_through_ctx.is_hear_through_enabled == false)
            && (app_hear_through_ctx.is_anc_changed == false)
            && (type_to_mode == running_mode)
            && (app_hear_through_ctx.hear_through_key_to_off == false)
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
            && (app_hearing_aid_activity_is_fwk_opening() == false)
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
            ) {
        app_anc_service_reset_hear_through_anc(false);
    }

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
    if (need_notify_power_off_state == true) {
        app_hear_through_activity_notify_power_saving_mode_changed();
    }
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */

    bool hear_through_switch = app_hear_through_storage_get_hear_through_switch();
    if (hear_through_switch == false) {
        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_middleware_control_callback] update ANC service hear through enabled to be false", 0);
        app_anc_service_set_hear_through_enabled(false);
    }

    if (result == LLF_STATUS_FAIL) {
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
        if ((type == LLF_TYPE_HEARING_AID) && (event == LLF_CONTROL_EVENT_ON)) {
            app_hearing_aid_activity_set_open_fwk_done(false);
        }
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
    }

    return true;
}


static void app_hear_through_activity_handle_ht_enable(bool enable)
{
    uint32_t turn_on_hear_through_timeout = app_hear_through_storage_get_hear_through_turn_on_after_boot_up_timeout();
    bool turn_on_hear_through = app_hear_through_storage_get_hear_through_switch();

    app_hear_through_ctx.is_powering_off = false;

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_ht_enable] enable : %d, ht_turn_on_config : %d, timeout : %d, power_on_vp_played : %d",
                        4,
                        enable,
                        turn_on_hear_through,
                        turn_on_hear_through_timeout,
                        app_hear_through_ctx.is_power_on_vp_played);

    if (enable == true) {
        if (app_hear_through_ctx.is_power_on_vp_played == true) {
            if (turn_on_hear_through == true) {
                ui_shell_send_event(false,
                                    EVENT_PRIORITY_MIDDLE,
                                    EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                    APP_HEAR_THROUGH_EVENT_ID_POWER_ON_TO_OPERATE_HT,
                                    NULL,
                                    0,
                                    NULL,
                                    turn_on_hear_through_timeout);
            } else {
                app_hear_through_ctx.is_power_on_ht_executed = true;

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
                app_hearing_aid_activity_set_power_on_played();
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
            }
        }
    } else {
        app_hear_through_switch_on_off(false, false);
    }
}

#ifdef AIR_TWS_ENABLE
static void app_hear_through_send_configuration(bool need_sync_operate, bool is_key_event, bool switch_status, uint8_t mode)
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_send_configuration] send CONFIGURATION_SYNC event, need_sync_operate : %d, is_key_event : %d, switch_status : %d, running_mode : %d, ambient_control_mode : %d, is_anc_changed : %d",
                        6,
                        need_sync_operate,
                        is_key_event,
                        switch_status,
                        mode,
                        app_hear_through_ctx.mode_index,
                        app_hear_through_ctx.is_anc_changed);

    app_hear_through_sync_configuration_t configuration;

    configuration.is_need_sync_operate = need_sync_operate;
    configuration.from_key = is_key_event;
    configuration.hear_through_switch = switch_status;
    configuration.hear_through_mode = mode;
    configuration.ambient_control_index = app_hear_through_ctx.mode_index;
    /**
     * @brief Fix issue - 47152
     * Sync anc changed to partner side.
     */
    configuration.is_anc_changed = app_hear_through_ctx.is_anc_changed;

    apps_aws_sync_send_future_sync_event(false,
                                            EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                            APP_HEAR_THROUGH_AWS_EVENT_ID_CONFIG_SYNC,
                                            need_sync_operate,
                                            (uint8_t *)&configuration,
                                            sizeof(app_hear_through_sync_configuration_t),
                                            APP_HEAR_THROUGH_SYNC_EVENT_DEFAULT_TIMEOUT);
}
#endif /* AIR_TWS_ENABLE */

/**
 * @brief Handle hear through advertising timeout to stop advertising.
 *
 * @param data
 * @param data_len
 */
static bool app_hear_through_activity_handle_advertising_timeout(void *extra_data, uint32_t extra_data_len)
{
    UNUSED(extra_data);
    UNUSED(extra_data_len);

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_advertising_timeout] Advertising timeout, stop advertising", 0);
#ifdef AIR_TWS_ENABLE
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
#endif /* AIR_TWS_ENABLE */
        app_hear_through_adv_stop();
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */

    return true;
}

/**
 * @brief Handle the RACE connected with SmartPhone.
 * If connected, need stop the ADV.
 *
 * @param data
 * @param data_len
 */
static bool app_hear_through_activity_handle_race_connected(void *extra_data, uint32_t extra_data_len)
{
    UNUSED(extra_data);
    UNUSED(extra_data_len);

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_race_connected] Race connected, stop advertising", 0);
#ifdef AIR_TWS_ENABLE
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
#endif /* AIR_TWS_ENABLE */
        app_hear_through_adv_stop();
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */

    return true;
}

/**
 * @brief Handle the RACE disconnected with SmartPhone.
 * If disconnected, need start the ADV.
 *
 * @param data
 * @param data_len
 */
static bool app_hear_through_activity_handle_race_disconnected(void *extra_data, uint32_t extra_data_len)
{
    UNUSED(extra_data);
    UNUSED(extra_data_len);

    bool is_out_case = app_hear_through_activity_is_out_case();
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_race_disconnected] Race disconnected, start advertising, out_case : %d",
                        1,
                        is_out_case);
#ifdef AIR_TWS_ENABLE
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
#endif /* AIR_TWS_ENABLE */
        if (is_out_case == true) {
            app_hear_through_adv_start();
        }
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */

    return true;
}

static bool app_hear_through_activity_handle_vp_streaming_begin(void *extra_data, uint32_t extra_data_len)
{
    UNUSED(extra_data);
    UNUSED(extra_data_len);

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    app_hearing_aid_activity_proc_vp_streaming_state_change(true);
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

    return true;
}

static bool app_hear_through_activity_handle_vp_streaming_end(void *extra_data, uint32_t extra_data_len)
{
    UNUSED(extra_data);
    UNUSED(extra_data_len);

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_vp_streaming_end] power_on_vp_played : %d, power_on_vp_playing : %d, charger_in : %d, powering_off : %d",
                        4,
                        app_hear_through_ctx.is_power_on_vp_played,
                        app_hear_through_ctx.is_power_on_vp_playing,
                        app_hear_through_ctx.is_charger_in,
                        app_hear_through_ctx.is_powering_off);

    /**
     * @brief Fix issue - 48041
     * When power on vp played, should make the power on vp playing flag to be cleared.
     */
    if (app_hear_through_ctx.is_power_on_vp_playing == true) {
        app_hear_through_ctx.is_power_on_vp_playing = false;
    }

    if (app_hear_through_ctx.is_power_on_vp_played == false) {
        app_hear_through_ctx.is_power_on_vp_played = true;

        if ((app_hear_through_ctx.is_charger_in == false)
            && (app_hear_through_ctx.is_powering_off == false)) {
            app_hear_through_activity_handle_ht_enable(true);
        }
    }

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    app_hearing_aid_activity_proc_vp_streaming_state_change(false);
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

    return true;
}

#ifdef MTK_RACE_CMD_ENABLE
#ifdef MTK_RACE_EVENT_ID_ENABLE
static RACE_ERRCODE app_hear_through_activity_race_event_handler(int32_t id, race_event_type_enum event_type, void *param, void *user_data)
#else
static RACE_ERRCODE app_hear_through_activity_race_event_handler(race_event_type_enum event_type, void *param, void *user_data)
#endif /* MTK_RACE_EVENT_ID_ENABLE */
{
#ifdef MTK_RACE_EVENT_ID_ENABLE
    if (id == app_hear_through_ctx.race_register_id) {
#endif /* MTK_RACE_EVENT_ID_ENABLE */
        switch (event_type) {
            case RACE_EVENT_TYPE_CONN_BLE_CONNECT:
            case RACE_EVENT_TYPE_CONN_BLE_1_CONNECT:
            case RACE_EVENT_TYPE_CONN_BLE_2_CONNECT:
            case RACE_EVENT_TYPE_CONN_SPP_CONNECT:
            case RACE_EVENT_TYPE_CONN_IAP2_CONNECT: {
                ui_shell_send_event(false,
                                    EVENT_PRIORITY_MIDDLE,
                                    EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                    APP_HEAR_THROUGH_EVENT_ID_RACE_CONNECTED,
                                    NULL,
                                    0,
                                    NULL,
                                    0);
            }
            break;

            case RACE_EVENT_TYPE_CONN_BLE_DISCONNECT:
            case RACE_EVENT_TYPE_CONN_BLE_1_DISCONNECT:
            case RACE_EVENT_TYPE_CONN_BLE_2_DISCONNECT:
            case RACE_EVENT_TYPE_CONN_SPP_DISCONNECT:
            case RACE_EVENT_TYPE_CONN_IAP2_DISCONNECT: {
                ui_shell_send_event(false,
                                    EVENT_PRIORITY_MIDDLE,
                                    EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                    APP_HEAR_THROUGH_EVENT_ID_RACE_DISCONNECTED,
                                    NULL,
                                    0,
                                    NULL,
                                    0);
            }
            break;
        }
#ifdef MTK_RACE_EVENT_ID_ENABLE
    }
#endif /* MTK_RACE_EVENT_ID_ENABLE */
    return RACE_ERRCODE_SUCCESS;
}

void app_hear_through_activity_init_race_handler()
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_init_race_handler] Init race event handler", 0);
#ifdef MTK_RACE_EVENT_ID_ENABLE
    race_event_register(&(app_hear_through_ctx.race_register_id), app_hear_through_activity_race_event_handler, NULL);
#else
    race_event_register(app_hear_through_activity_race_event_handler, NULL);
#endif /* MTK_RACE_EVENT_ID_ENABLE */
}
#endif /* MTK_RACE_CMD_ENABLE */

static void app_hear_through_control_callback(llf_type_t type, llf_control_event_t event, llf_status_t result)
{
    app_hear_through_control_callback_parameter_t *parameter = (app_hear_through_control_callback_parameter_t *)pvPortMalloc(sizeof(app_hear_through_control_callback_parameter_t));
    if (parameter == NULL) {
        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_control_callback] Failed to allocate buffer for callback parameter", 0);
        return;
    }

    memset(parameter, 0, sizeof(app_hear_through_control_callback_parameter_t));

    parameter->event = event;
    parameter->type = type;
    parameter->result = result;

    ui_shell_send_event(false,
                        EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                        APP_HEAR_THROUGH_EVENT_ID_MIDDLEWARE_CONTROL_CALLBACK,
                        parameter,
                        sizeof(app_hear_through_control_callback_parameter_t),
                        NULL,
                        100);
}

static void app_hear_through_activity_initialization()
{
    if (app_hear_through_ctx.init_done == false) {
        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_initialization] Enter", 0);

        memset(&app_hear_through_ctx, 0, sizeof(app_hear_through_context_t));

        app_hear_through_ctx.trigger_from_key = true;

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
        app_power_save_utils_register_get_mode_callback(app_hear_through_activity_get_power_saving_mode_callback);
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */

        app_hear_through_storage_load_const_configuration();
        app_hear_through_storage_load_user_configuration();

        llf_control_register_callback((llf_control_callback_t)app_hear_through_control_callback,
                                                LLF_CONTROL_EVENT_ON | LLF_CONTROL_EVENT_OFF,
                                                LLF_MAX_CALLBACK_LEVEL_ALL);

        app_hear_through_ctx.is_charger_in = true; //!is_out_case;

#ifdef MTK_RACE_CMD_ENABLE
        app_hear_through_activity_init_race_handler();
#endif /* MTK_RACE_CMD_ENABLE */

        app_hear_through_adv_init();

        app_hear_through_ctx.init_done = true;

        /**
         * @brief Fix issue
         * Set hear through switch to be the stored value.
         */
        app_anc_service_set_hear_through_enabled(app_hear_through_storage_get_hear_through_switch());
        app_hear_through_update_ambient_control_mode();

#ifdef AIR_TWS_ENABLE
        if (app_hear_through_is_aws_connected() == true) {
            bt_aws_mce_role_t aws_role = bt_device_manager_aws_local_info_get_role();
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_initialization] Role : 0x%02x", 1, aws_role);
            if (aws_role == BT_AWS_MCE_ROLE_AGENT) {
                app_hear_through_mode_t hear_through_mode = app_hear_through_storage_get_hear_through_mode();
                bool hear_through_switch = app_hear_through_storage_get_hear_through_switch();

                app_hear_through_send_configuration(false, false, hear_through_switch, hear_through_mode);
            } else if (aws_role == BT_AWS_MCE_ROLE_PARTNER) {

                apps_aws_sync_send_future_sync_event(false,
                                                        EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                                        APP_HEAR_THROUGH_AWS_EVENT_ID_CONFIG_SYNC_REQUEST,
                                                        false,
                                                        NULL,
                                                        0,
                                                        APP_HEAR_THROUGH_SYNC_EVENT_DEFAULT_TIMEOUT);
            }
        }
#endif /* AIR_TWS_ENABLE */
    }
}

static void app_hear_through_activity_de_initialization()
{
    if (app_hear_through_ctx.init_done == true) {

        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_de_initialization] Enter, hear_through_enabled : %d",
                        1,
                        app_hear_through_ctx.is_hear_through_enabled);

        if (app_hear_through_ctx.is_hear_through_enabled == true) {
            app_hear_through_activity_handle_ht_enable(false);
        }

        app_anc_service_save_into_flash();
        app_anc_service_suspend();

        llf_control_register_callback(NULL,
                                        LLF_CONTROL_EVENT_ON | LLF_CONTROL_EVENT_OFF,
                                        LLF_MAX_CALLBACK_LEVEL_ALL);
        bool is_charger_in = app_hear_through_ctx.is_charger_in;

        memset(&app_hear_through_ctx, 0, sizeof(app_hear_through_context_t));

        app_hear_through_ctx.is_charger_in = is_charger_in;
    }
}

bool app_hear_through_proc_ui_shell_system_event(int32_t event_id, void *extra_data, size_t data_len)
{
    if (event_id == EVENT_ID_SHELL_SYSTEM_ON_CREATE) {

        // bool is_out_case = app_hear_through_activity_is_out_case();
        // APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_ui_shell_system_event] Activity create, out_case : %d",
        //                     1,
        //                     is_out_case);

        app_hear_through_activity_initialization();
    }

    if (event_id == EVENT_ID_SHELL_SYSTEM_ON_DESTROY) {
        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_ui_shell_system_event] Activity Destroy", 0);

        app_hear_through_activity_de_initialization();
    }
    return true;
}

/**
 * @brief Process connection manager event.
 * If a2dp connected, need start Hear through advertising.
 *
 * @param event_id
 * @param extra_data
 * @param data_len
 * @return true
 * @return false
 */
static bool app_hear_through_activity_proc_cm_event(uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    if (event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE) {
        bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;

        if (remote_update->pre_connected_service == remote_update->connected_service) {
            return false;
        }

        if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->pre_connected_service)
            && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->connected_service)) {
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_proc_cm_event] A2DP connected", 0);
            app_hear_through_adv_set_connected_remote_address(&(remote_update->address));
            app_hear_through_adv_start();
        }

#ifdef AIR_TWS_ENABLE

        if (app_hear_through_ctx.init_done == false) {
            APPS_LOG_MSGID_E(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_cm_event] Hear through not init done", 0);
            return false;
        }

        bt_aws_mce_role_t device_aws_role = bt_device_manager_aws_local_info_get_role();
        if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
            && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
            // AWS connected handler
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_cm_event] AWS Connected, Role : 0x%02x", 1, device_aws_role);

            if (device_aws_role == BT_AWS_MCE_ROLE_AGENT) {
                app_hear_through_mode_t hear_through_mode = app_hear_through_storage_get_hear_through_mode();
                bool hear_through_switch = app_hear_through_storage_get_hear_through_switch();

                app_hear_through_send_configuration(false, false, hear_through_switch, hear_through_mode);
            }
        } else if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)
                    && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)) {
            // AWS disconnected handler
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_cm_event] AWS Disconnected", 0);
        }
#endif /* AIR_TWS_ENABLE */
    }
    return false;
}

static bool app_hear_through_activity_handle_race_cmd(void *extra_data, uint32_t extra_data_len)
{
    if ((extra_data == NULL) || (extra_data_len == 0)) {
        APPS_LOG_MSGID_E(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_race_cmd] extra data is NULL", 0);
        return true;
    }

    app_hear_through_request_t *request = (app_hear_through_request_t *)extra_data;

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    if ((audio_anc_leakage_compensation_get_status() == true
            && (request->op_code == APP_HEAR_THROUGH_CMD_OP_CODE_SET))) {
        APPS_LOG_MSGID_E(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_race_cmd] leakage detection is ongoing, cannot execute set command", 0);
        app_hear_through_race_cmd_send_set_response(request->op_type, false);
        return false;
    }
#endif /* MTK_LEAKAGE_DETECTION_ENABLE */

    /**
     * @brief Handle Hearing Aid RACE CMD
     */
    if (request->op_type < APP_HEARING_AID_CONFIG_TYPE_MAXIMUM) {
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
        app_hearing_aid_activity_process_race_cmd(extra_data, extra_data_len);
        return true;
#else
        APPS_LOG_MSGID_E(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_race_cmd] Not enable AIR_HEARING_AID_ENABLE feature option", 0);
        if (request->op_code == APP_HEAR_THROUGH_CMD_OP_CODE_GET) {
            app_hear_through_race_cmd_send_get_response(request->op_type, NULL, 0);
        } else {
            app_hear_through_race_cmd_send_set_response(request->op_type, false);
        }
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
        return true;
    }

    /**
     * @brief Handle Hear Through RACE CMD
     */
    if (request->op_code == APP_HEAR_THROUGH_CMD_OP_CODE_SET) {

        if (request->op_type >= APP_HEAR_THROUGH_VIVID_CONFIG_BASE) {
#ifndef AIR_HEARTHROUGH_VIVID_PT_ENABLE
            app_hear_through_activity_send_set_cmd_response(request->op_type, false);
            return true;
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */
        }

#ifdef AIR_TWS_ENABLE
        if (app_hear_through_is_aws_connected() == true) {

            apps_aws_sync_send_future_sync_event(false,
                                                    EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                                    request->op_type,
                                                    true,
                                                    request->op_parameter,
                                                    extra_data_len - APP_HEAR_THROUGH_OP_PARAMETER_INDEX,
                                                    APP_HEAR_THROUGH_SYNC_EVENT_DEFAULT_TIMEOUT);
        } else {
#endif /* AIR_TWS_ENABLE */
            app_hear_through_activity_handle_set_cmd(request->op_type,
                                                        request->op_parameter,
                                                        extra_data_len - APP_HEAR_THROUGH_OP_PARAMETER_INDEX);
#ifdef AIR_TWS_ENABLE
        }
#endif /* AIR_TWS_ENABLE */
    } else if (request->op_code == APP_HEAR_THROUGH_CMD_OP_CODE_GET) {
        app_hear_through_activity_handle_get_cmd(request->op_type);
    }

    return true;
}

static bool app_hear_through_activity_handle_power_on_ht(void *extra_data, uint32_t extra_data_len)
{
    UNUSED(extra_data);
    UNUSED(extra_data_len);

    app_hear_through_mode_t hear_through_mode = app_hear_through_storage_get_hear_through_mode();
    bool hear_through_switch = app_hear_through_storage_get_hear_through_switch();

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_power_on_ht] mode : %d, ht_switch : %d",
                        2,
                        hear_through_mode,
                        hear_through_switch);

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    bool sync = false;
    bool enable = false;

#ifdef AIR_TWS_ENABLE
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
        sync = true;
    }
#endif /* AIR_TWS_ENABLE */

    if (hear_through_mode == APP_HEAR_THROUGH_MODE_HA_PSAP) {
        enable = hear_through_switch;
    } else {
        enable = false;
    }

    app_hearing_aid_activity_set_user_switch(sync, enable);
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

    app_hear_through_switch_on_off(false, hear_through_switch);

    app_hear_through_ctx.is_power_on_ht_executed = true;

    return true;
}

#ifdef AIR_TWS_ENABLE
static bool app_hear_through_activity_handle_aws_config_sync_request(void *extra_data, uint32_t extra_data_len)
{
    UNUSED(extra_data);
    UNUSED(extra_data_len);

    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
        app_hear_through_mode_t hear_through_mode = app_hear_through_storage_get_hear_through_mode();
        bool hear_through_switch = app_hear_through_storage_get_hear_through_switch();

        app_hear_through_send_configuration(false, false, hear_through_switch, hear_through_mode);
    }
    return true;
}

static bool app_hear_through_activity_handle_aws_config_sync(void *extra_data, uint32_t extra_data_len)
{
    bool is_out_case = app_hear_through_activity_is_out_case();
    app_hear_through_sync_configuration_t *configuration = (app_hear_through_sync_configuration_t *)extra_data;

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_aws_config_sync] CONFIGURATION_SYNC event, is_key_event : %d, is_need_sync_operate : %d, hear_through_switch : %d, hear_through_mode : %d, ambient_control_index : %d, is_anc_changed : %d",
                        6,
                        configuration->from_key,
                        configuration->is_need_sync_operate,
                        configuration->hear_through_switch,
                        configuration->hear_through_mode,
                        configuration->ambient_control_index,
                        configuration->is_anc_changed);

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_aws_config_sync] CONFIGURATION_SYNC event, power_on_ht_executed : %d, is_ota_ongoing : %d",
                        2,
                        app_hear_through_ctx.is_power_on_ht_executed,
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
                        app_hear_through_ctx.is_ota_ongoing
#else
                        0
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */
                        );

    app_hear_through_ctx.mode_index = configuration->ambient_control_index;
    /**
     * @brief Fix issue - 47152
     * Configure from agent side to make sure that do not reset ANC
     */
    app_hear_through_ctx.is_anc_changed = configuration->is_anc_changed;

    /**
     * @brief Fix issue - 47393
     * Fix issue that the ANC changed, no need to do switch on-off HT operation.
     */
    if (app_hear_through_ctx.is_anc_changed == true) {
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
        app_hearing_aid_activity_set_user_switch(false, false);
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

        app_hear_through_storage_set_hear_through_mode(configuration->hear_through_mode);
        app_hear_through_storage_set_hear_through_switch(configuration->hear_through_switch);

        return true;
    }

    if ((configuration->hear_through_mode == APP_HEAR_THROUGH_MODE_VIVID_PT)
        || (configuration->hear_through_mode == APP_HEAR_THROUGH_MODE_HA_PSAP)) {

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
        if (app_hear_through_ctx.is_ota_ongoing == false) {
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */
            if (configuration->hear_through_mode == APP_HEAR_THROUGH_MODE_HA_PSAP) {
                if (configuration->is_need_sync_operate == true) {
                    app_hearing_aid_activity_set_need_play_locally(false);
                } else {
                    app_hearing_aid_activity_set_need_play_locally(true);
                }
                app_hearing_aid_activity_set_user_switch(false, configuration->hear_through_switch);
            } else {
                app_hearing_aid_activity_set_need_play_locally(false);
                app_hearing_aid_activity_set_user_switch(false, false);
            }
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
        }
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

        app_hear_through_storage_set_hear_through_mode(configuration->hear_through_mode);

        if (configuration->hear_through_switch == true) {
            app_hear_through_ctx.hear_through_key_to_off = false;
        }

        if ((app_hear_through_ctx.is_power_on_ht_executed == true) && (is_out_case == true)) {
            app_hear_through_ctx.trigger_from_key = configuration->from_key;
            app_hear_through_switch_on_off(true, configuration->hear_through_switch);
        } else {
            app_hear_through_storage_set_hear_through_switch(configuration->hear_through_switch);
        }
    }
    return true;
}

static bool app_hear_through_activity_handle_aws_ambient_control(void *extra_data, uint32_t extra_data_len)
{
    app_hear_through_sync_ambient_control_switch_t *control_switch = (app_hear_through_sync_ambient_control_switch_t *)extra_data;
    app_hear_through_ctx.mode_index = control_switch->mode_index;

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_aws_ambient_control] ambient control, mode_index : %d",
                        1,
                        app_hear_through_ctx.mode_index);

    app_hear_through_activity_handle_ambient_control_switch();

    return true;
}

static bool app_hear_through_activity_handle_aws_anc_switched(void *extra_data, uint32_t extra_data_len)
{
    UNUSED(extra_data);
    UNUSED(extra_data_len);

    app_hear_through_activity_handle_anc_state_changed();

    return true;
}

static bool app_hear_through_activity_handle_aws_hear_through_key_to_off(void *extra_data, uint32_t extra_data_len)
{
    UNUSED(extra_data);
    UNUSED(extra_data_len);

    app_hear_through_ctx.hear_through_key_to_off = true;

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    app_hear_through_mode_t hear_through_mode = app_hear_through_storage_get_hear_through_mode();
    if (hear_through_mode == APP_HEAR_THROUGH_MODE_HA_PSAP) {
        app_hearing_aid_activity_set_user_switch(false, false);
    }
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

    app_hear_through_storage_set_hear_through_switch(false);

    app_anc_service_reset_hear_through_anc(false);

    app_hear_through_notify_switch_state_change(false);

    return true;
}
#endif /* AIR_TWS_ENABLE */

typedef bool (*hear_through_event_handler)(void *extra_data, uint32_t extra_data_len);

static hear_through_event_handler hear_through_event_ext_handler_list[] = {
    NULL,                                                           // 0x0000
    app_hear_through_activity_handle_race_cmd,                      // 0x0001
    app_hear_through_activity_handle_power_on_ht,                   // 0x0002
    app_hear_through_activity_handle_advertising_timeout,           // 0x0003
    app_hear_through_activity_handle_race_connected,                // 0x0004
    app_hear_through_activity_handle_race_disconnected,             // 0x0005
    app_hear_through_activity_handle_vp_streaming_begin,            // 0x0006
    app_hear_through_activity_handle_vp_streaming_end,              // 0x0007
    app_hear_through_activity_handle_middleware_control_callback,   // 0x0008
};

#ifdef AIR_TWS_ENABLE
static hear_through_event_handler hear_through_event_ht_cmd_handler_list[] = {
    NULL,                                                           // 0x1000
    app_hear_through_activity_handle_switch_set_cmd,                // 0x1001
    app_hear_through_activity_handle_mode_set_cmd,                  // 0x1002
};

#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
static hear_through_event_handler hear_through_event_vivid_cmd_handler_list[] = {
    NULL,                                                           // 0x2000
    app_hear_through_activity_handle_vivid_afc_set_cmd,             // 0x2001
    app_hear_through_activity_handle_vivid_ldnr_set_cmd,            // 0x2002
    app_hear_through_activity_handle_vivid_by_pass_set_cmd,         // 0x2003
};
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */

static hear_through_event_handler hear_through_event_aws_event_handler_list[] = {
    NULL,                                                           // 0x3000
    app_hear_through_activity_handle_aws_config_sync_request,       // 0x3001
    app_hear_through_activity_handle_aws_config_sync,               // 0x3002
    app_hear_through_activity_handle_aws_ambient_control,           // 0x3003
    app_hear_through_activity_handle_aws_anc_switched,              // 0x3004
    app_hear_through_activity_handle_aws_hear_through_key_to_off,   // 0x3005
};
#endif /* AIR_TWS_ENABLE */

bool app_hear_through_proc_hear_through_event(int32_t event_id, void *extra_data, size_t data_len)
{
    bool is_out_case = app_hear_through_activity_is_out_case();

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_hear_through_event] event : 0x%04x, out_case : %d",
                        2,
                        event_id,
                        is_out_case);

    void *temp_extra_data = extra_data;
    uint32_t temp_extra_data_len = data_len;

    if ((event_id > APP_HEAR_THROUGH_EVENT_ID_BASE)
            && (event_id < APP_HEAR_THROUGH_CONFIG_BASE)) {
        if (hear_through_event_ext_handler_list[event_id] != NULL) {
            hear_through_event_ext_handler_list[event_id](temp_extra_data, temp_extra_data_len);
        }
        return true;
    }

#ifdef AIR_TWS_ENABLE

    if (extra_data != NULL) {
        apps_aws_sync_future_event_local_event_t *local_event = (apps_aws_sync_future_event_local_event_t *)extra_data;
        temp_extra_data = local_event->extra_data;
        temp_extra_data_len = local_event->extra_data_len;
    }

    if ((event_id > APP_HEAR_THROUGH_CONFIG_BASE)
            && (event_id < APP_HEAR_THROUGH_VIVID_CONFIG_BASE)) {
        if (hear_through_event_ht_cmd_handler_list[event_id - APP_HEAR_THROUGH_CONFIG_BASE] != NULL) {
            hear_through_event_ht_cmd_handler_list[event_id - APP_HEAR_THROUGH_CONFIG_BASE](temp_extra_data, temp_extra_data_len);
        }
        return true;
    }

#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
    if ((event_id > APP_HEAR_THROUGH_VIVID_CONFIG_BASE)
            && (event_id < APP_HEAR_THROUGH_AWS_EVENT_ID_BASE)) {
        if (hear_through_event_vivid_cmd_handler_list[event_id - APP_HEAR_THROUGH_VIVID_CONFIG_BASE] != NULL) {
            hear_through_event_vivid_cmd_handler_list[event_id - APP_HEAR_THROUGH_VIVID_CONFIG_BASE](temp_extra_data, temp_extra_data_len);
        }
        return true;
    }
#endif /* AIR_HEARTHROUGH_VIVID_PT_ENABLE */

    if (event_id > APP_HEAR_THROUGH_AWS_EVENT_ID_BASE) {
        if (hear_through_event_aws_event_handler_list[event_id - APP_HEAR_THROUGH_AWS_EVENT_ID_BASE] != NULL) {
            hear_through_event_aws_event_handler_list[event_id - APP_HEAR_THROUGH_AWS_EVENT_ID_BASE](temp_extra_data, temp_extra_data_len);
        }
        return true;
    }
#endif /* AIR_TWS_ENABLE */

    return false;
}

static bool app_hear_through_activity_proc_dm_event(uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    bt_device_manager_power_event_t evt;
    bt_device_manager_power_status_t status;
    bt_event_get_bt_dm_event_and_status(event_id, &evt, &status);

    if (status == BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS) {
        if (evt == BT_DEVICE_MANAGER_POWER_EVT_ACTIVE_COMPLETE) {
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_proc_dm_event] BT Powering ON, is_charger_in : %d, hear_through_enabled : %d, power_on_vp_played : %d, power_on_vp_playing : %d",
                                4,
                                app_hear_through_ctx.is_charger_in,
                                app_hear_through_ctx.is_hear_through_enabled,
                                app_hear_through_ctx.is_power_on_vp_played,
                                app_hear_through_ctx.is_power_on_vp_playing);

            app_hear_through_ctx.is_powering_off = false;

            app_anc_service_resume();

            if (app_hear_through_ctx.is_power_on_vp_played == false) {
                if (app_hear_through_ctx.is_power_on_vp_playing == false) {
                    voice_prompt_play_vp_power_on();
                }
            } else {
                if ((app_hear_through_ctx.is_charger_in == false)
                        && (app_hear_through_ctx.is_hear_through_enabled == false)) {
                    app_hear_through_activity_handle_ht_enable(true);
                }
            }

        } else if(evt == BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE) {
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_proc_dm_event] BT Powering off, hear_through_enabled : %d",
                                1,
                                app_hear_through_ctx.is_hear_through_enabled);

            app_hear_through_storage_save_user_configuration();

            if (app_hear_through_ctx.is_hear_through_enabled == true) {
                app_hear_through_activity_handle_ht_enable(false);
            }

            app_anc_service_save_into_flash();
            app_anc_service_suspend();

            /**
             * @brief Move here to make sure to disable HA/PSAP/VIVID correctly.
             */
            app_hear_through_ctx.is_powering_off = true;

            app_hear_through_ctx.is_hear_through_enabled = false;
            app_hear_through_ctx.is_power_on_vp_played = false;
            app_hear_through_ctx.is_power_on_ht_executed = false;

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
            app_hearing_aid_activity_set_open_fwk_done(false);
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
        }
    }

    return false;
}

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
bool app_hear_through_proc_hearing_aid_event(int32_t event_id, void *extra_data, size_t data_len)
{
    if ((event_id == APP_HEARING_AID_EVENT_ID_HA_ON)
        || (event_id == APP_HEARING_AID_EVENT_ID_HA_OFF)) {

#if 0
        if ((app_hear_through_ctx.is_hear_through_enabled == false)
            && (event_id == APP_HEARING_AID_EVENT_ID_HA_ON)) {
        }
#endif
        if (event_id == APP_HEARING_AID_EVENT_ID_HA_OFF) {
            bool anc_enabled = app_anc_service_is_anc_enabled();
            bool is_a2dp_drc_on = false;
            bool is_sco_drc_on = false;
            bool is_drc_on = false;

            bool a2dp_streaming = false;

            bt_sink_srv_state_t sink_srv_state = bt_sink_srv_get_state();

            if (sink_srv_state == BT_SINK_SRV_STATE_STREAMING) {
                a2dp_streaming = true;
            }
#ifdef AIR_LE_AUDIO_BIS_ENABLE
            if (bt_sink_srv_cap_stream_is_broadcast_streaming() == true) {
                a2dp_streaming = true;
            }
#endif /* AIR_LE_AUDIO_BIS_ENABLE */

            if (a2dp_streaming == true) {
                app_hearing_aid_utils_is_drc_on(APP_HEARING_AID_DRC_A2DP, &is_a2dp_drc_on);
                is_drc_on = is_a2dp_drc_on;
            }

            if (app_hearing_aid_activity_is_sco_ongoing() == true) {
                app_hearing_aid_utils_is_drc_on(APP_HEARING_AID_DRC_SCO, &is_sco_drc_on);
                is_drc_on = is_sco_drc_on;
            }

            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_hearing_aid_event] HA/PSAP disabled, anc_enabled : %d, is_a2dp_drc_on : %d, is_sco_drc_on : %d, is_drc_on : %d",
                                4,
                                anc_enabled,
                                is_a2dp_drc_on,
                                is_sco_drc_on,
                                is_drc_on);

            if ((anc_enabled == false) && (is_drc_on == false)) {
                app_hearing_aid_activity_open_hearing_aid_fwk_with_zero_path();
            }
        }
    }

    return false;
}
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

static void app_hear_through_handle_charger_out()
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_handle_charger_out] Charger OUT, power_on_vp_played : %d, power_on_vp_playing : %d, powering_off : %d",
                        3,
                        app_hear_through_ctx.is_power_on_vp_played,
                        app_hear_through_ctx.is_power_on_vp_playing,
                        app_hear_through_ctx.is_powering_off);

    app_hear_through_ctx.is_charger_in = false;
    app_hear_through_ctx.trigger_from_key = true;
    app_hear_through_ctx.is_power_on_ht_executed = false;

    app_hear_through_update_ambient_control_mode();

    app_anc_service_resume();

    /**
     * Fix issue - 47608
     * If out of case happen, need check is powering off or not, if is not powering off
     * need play VP to make HT on.
     */
    if (app_hear_through_ctx.is_powering_off == false) {
        if (app_hear_through_ctx.is_power_on_vp_played == false) {
            if (app_hear_through_ctx.is_power_on_vp_playing == false) {
                voice_prompt_play_vp_power_on();
            }
        } else {
            app_hear_through_activity_handle_ht_enable(true);
        }
    }
}

static void app_hear_through_handle_charger_in()
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_handle_charger_in] Charger IN",0);

    app_hear_through_ctx.is_charger_in = true;
    app_hear_through_ctx.is_power_on_vp_played = false;
    app_hear_through_ctx.is_power_on_vp_playing = false;

    app_hear_through_storage_save_user_configuration();

    if (app_hear_through_ctx.is_hear_through_enabled == true) {
        app_hear_through_activity_handle_ht_enable(false);
    }

    app_anc_service_save_into_flash();
    app_anc_service_suspend();
}


#ifdef AIR_SMART_CHARGER_ENABLE
bool app_hear_through_proc_smart_charger_case_event(int32_t event_id, void *extra_data, size_t data_len)
{
    if (event_id == SMCHARGER_EVENT_NOTIFY_ACTION) {
        app_smcharger_public_event_para_t *para = (app_smcharger_public_event_para_t *)extra_data;
        if (para->action == SMCHARGER_CHARGER_OUT_ACTION) {
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_smart_charger_case_event][SmartCharger] Charger out",0);

            app_hear_through_handle_charger_out();
        }
        if (para->action == SMCHARGER_CHARGER_IN_ACTION) {
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_smart_charger_case_event][SmartCharger] Charger in, Turn off hear through", 0);

            app_hear_through_handle_charger_in();
        }
    }
    return false;
}
#else
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
bool app_hear_through_proc_battery_event(int32_t event_id, void *extra_data, size_t data_len)
{
    if (event_id == APPS_EVENTS_BATTERY_CHARGER_EXIST_CHANGE) {
        bool charger_in = (bool)extra_data;
        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_battery_event], charger_exist change %d->%d, hear_through_enabled : %d",
                            3,
                            app_hear_through_ctx.is_charger_in,
                            charger_in,
                            app_hear_through_ctx.is_hear_through_enabled);

        if ((app_hear_through_ctx.is_charger_in == false) && (charger_in == true)) {
            app_hear_through_handle_charger_in();
        }
        if ((app_hear_through_ctx.is_charger_in == true) && (charger_in == false)) {
            app_hear_through_handle_charger_out();
        }
    }
    return false;
}
#endif /* MTK_BATTERY_MANAGEMENT_ENABLE */
#endif /* AIR_SMART_CHARGER_ENABLE */

static bool app_hear_through_proc_key_event(uint32_t event_id,
                                            void *extra_data,
                                            size_t data_len,
                                            bool from_aws)
{
    uint16_t key_action = *(uint16_t *)extra_data;

    if (key_action == KEY_HEAR_THROUGH_TOGGLE) {

        if (app_hear_through_ctx.init_done == false) {
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_key_event] Not init done", 0);
            return false;
        }

        if (app_hear_through_ctx.mode_index != APP_HEAR_THROUGH_MODE_SWITCH_INDEX_HEAR_THROUGH) {
            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_key_event] Not Hear through mode", 0);
            return false;
        }
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
        if (app_hear_through_ctx.is_ota_ongoing == true) {
            APPS_LOG_MSGID_W(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_key_event] OTA is ongoing, do not support to control Hear Through", 0);
            return false;
        }
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */

        bool hear_through_switch_to_be = false;
        app_hear_through_mode_t hear_through_mode = app_hear_through_storage_get_hear_through_mode();
        bool hear_through_switch = app_hear_through_storage_get_hear_through_switch();

        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_key_event] running_mode : %d, hear_through_switch : %d, trigger_from_key : %d, power_on_ht_executed : %d, charger_in : %d",
                            5,
                            hear_through_mode,
                            hear_through_switch,
                            app_hear_through_ctx.trigger_from_key,
                            app_hear_through_ctx.is_power_on_ht_executed,
                            app_hear_through_ctx.is_charger_in);

        app_hear_through_ctx.trigger_from_key = true;

        if (hear_through_switch == true) {
            hear_through_switch_to_be = false;
        } else {
            hear_through_switch_to_be = true;
        }

        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_key_event] hear_through_switch_to_be : %d",
                            1,
                            hear_through_switch_to_be);

        /**
         * @brief Fix issue - 46584
         * When hear through switch to off, just need reset to anc directly, no need
         * to disable HA/PSAP/VIVID firstly, then reset.
         * This will reduce the ANC off message to SP APP.
         */
        if (hear_through_switch_to_be == false) {
#ifdef AIR_TWS_ENABLE
            if (app_hear_through_is_aws_connected() == true) {
                apps_aws_sync_send_future_sync_event(false,
                                                        EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                                        APP_HEAR_THROUGH_AWS_EVENT_ID_HEAR_THROUGH_KEY_TO_OFF,
                                                        true,
                                                        NULL,
                                                        0,
                                                        APP_HEAR_THROUGH_SYNC_EVENT_DEFAULT_TIMEOUT);
            } else {
#endif /* AIR_TWS_ENABLE */
                app_hear_through_activity_handle_aws_hear_through_key_to_off(NULL, 0);
#ifdef AIR_TWS_ENABLE
            }
#endif /* AIR_TWS_ENABLE */
            return true;
        }

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
        if (hear_through_mode == APP_HEAR_THROUGH_MODE_HA_PSAP) {
            app_hearing_aid_activity_set_user_switch(false, true);
        }
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

#ifdef AIR_TWS_ENABLE
        if (app_hear_through_is_aws_connected() == true) {
            app_hear_through_send_configuration(true, true, true, hear_through_mode);
        } else {
#endif /* AIR_TWS_ENABLE */

            app_hear_through_ctx.hear_through_key_to_off = false;

            if ((app_hear_through_ctx.is_power_on_ht_executed == true)
                    && (app_hear_through_ctx.is_charger_in == false)) {
                app_hear_through_switch_on_off(true, true);
            } else {
                app_hear_through_storage_set_hear_through_switch(true);
            }
#ifdef AIR_TWS_ENABLE
        }
#endif /* AIR_TWS_ENABLE */
        return true;
    }
    return false;
}

#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
/**
 * @brief Workaround for issue - 42461, when OTA start, write OTA bin to flash will make interrupt
 * open/close very frequently, then will make audio MIC work un-normally.
 */
static bool app_hear_through_proc_ota_event(uint32_t event_id,
                                            void *extra_data,
                                            size_t data_len)
{
    if (event_id == RACE_EVENT_TYPE_FOTA_START) {
        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_ota_event] OTA beginning, hear_through_enabled : %d",
                            1,
                            app_hear_through_ctx.is_hear_through_enabled);

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
        app_hearing_aid_activity_set_user_switch(false, false);
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

        app_hear_through_ctx.is_hear_through_enabled_before_ota = app_hear_through_ctx.is_hear_through_enabled;
        app_hear_through_ctx.is_ota_ongoing = true;

        if (app_hear_through_ctx.is_hear_through_enabled_before_ota == true) {
            app_hear_through_activity_handle_ht_enable(false);
        }
    } else if ((event_id == RACE_EVENT_TYPE_FOTA_CANCEL)
                || (event_id == RACE_EVENT_TYPE_FOTA_TRANSFER_COMPLETE)) {

        app_hear_through_mode_t hear_through_mode = app_hear_through_storage_get_hear_through_mode();

        APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_ota_event] OTA finished, hear_through_enabled_before_ota : %d, mode : %d, charger_in : %d, powering_off : %d",
                            4,
                            app_hear_through_ctx.is_hear_through_enabled_before_ota,
                            hear_through_mode,
                            app_hear_through_ctx.is_charger_in,
                            app_hear_through_ctx.is_powering_off);

        app_hear_through_ctx.is_ota_ongoing = false;

        if ((app_hear_through_ctx.is_hear_through_enabled_before_ota == true)
                && (app_hear_through_ctx.is_charger_in == false)
                && (app_hear_through_ctx.is_powering_off == false)) {

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
            if (hear_through_mode == APP_HEAR_THROUGH_MODE_HA_PSAP) {
                app_hearing_aid_activity_set_user_switch(false, true);
            }
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

            app_hear_through_activity_handle_ht_enable(true);
        }

        app_hear_through_update_ambient_control_mode();
    }

    return false;
}
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */

#if defined(MTK_ANC_ENABLE) && defined(AIR_HW_VIVID_PT_ENABLE)
static bool app_hear_through_proc_anc_event(uint32_t event_id,
                                            void *extra_data,
                                            size_t data_len)
{
    if ((extra_data != NULL) && (data_len != 0)) {

        app_anc_srv_result_t *anc_srv_result = (app_anc_srv_result_t *)extra_data;

        if ((anc_srv_result->cur_type == AUDIO_ANC_CONTROL_TYPE_PT_HW_VIVID)
                && (anc_srv_result->cur_filter_id == AUDIO_ANC_CONTROL_HW_VIVID_PASS_THRU_FILTER_DEFAULT)) {

            app_hear_through_mode_t running_mode = app_hear_through_storage_get_hear_through_mode();
            bool hear_through_switch = app_hear_through_storage_get_hear_through_switch();

            APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_proc_anc_event] Control HW Vivid result : %d, running_mode : %d, hear_through_switch : %d, ht_enable : %d, anc_changed : %d, key_to_off : %d",
                                    6,
                                    event_id,
                                    running_mode,
                                    hear_through_switch,
                                    app_hear_through_ctx.is_hear_through_enabled,
                                    app_hear_through_ctx.is_anc_changed,
                                    app_hear_through_ctx.hear_through_key_to_off);

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
            bool old_ht_enabled = app_hear_through_ctx.is_hear_through_enabled;
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */

            if (event_id == AUDIO_ANC_CONTROL_EVENT_ON) {
                app_hear_through_ctx.is_hear_through_enabled = true;
            } else if (event_id == AUDIO_ANC_CONTROL_EVENT_OFF) {
                app_hear_through_ctx.is_hear_through_enabled = false;
            }

#if AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED
            if (old_ht_enabled != app_hear_through_ctx.is_hear_through_enabled) {
                app_hear_through_activity_notify_power_saving_mode_changed();
            }
#endif /* AIR_APP_HEAR_THROUGH_SYSTEM_OFF_WHEN_HT_DISABLED */

            if ((app_hear_through_ctx.is_hear_through_enabled == false)
                    && (app_hear_through_ctx.is_anc_changed == false)
                    && (APP_HEAR_THROUGH_MODE_VIVID_PT == running_mode)
                    && (app_hear_through_ctx.hear_through_key_to_off == false)) {
                app_anc_service_reset_hear_through_anc(false);
            }

            if (hear_through_switch == false) {
                app_anc_service_set_hear_through_enabled(false);
            }
        }
    }

    return false;
}
#endif /* MTK_ANC_ENABLE && AIR_HW_VIVID_PT_ENABLE */

bool app_hear_through_activity_proc(ui_shell_activity_t *self,
                                    uint32_t event_group,
                                    uint32_t event_id,
                                    void *extra_data,
                                    size_t data_len)
{
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            return app_hear_through_proc_ui_shell_system_event(event_id, extra_data, data_len);
        }
        break;

        case EVENT_GROUP_UI_SHELL_HEAR_THROUGH: {
            return app_hear_through_proc_hear_through_event(event_id, extra_data, data_len);
        }
        break;

        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            return app_hear_through_activity_proc_cm_event(event_id, extra_data, data_len);
        }
        break;

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
        case EVENT_GROUP_UI_SHELL_HEARING_AID: {
            return app_hear_through_proc_hearing_aid_event(event_id, extra_data, data_len);
        }
        break;
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

#ifdef AIR_SMART_CHARGER_ENABLE
        case EVENT_GROUP_UI_SHELL_CHARGER_CASE: {
            app_hear_through_proc_smart_charger_case_event(event_id, extra_data, data_len);
        }
        break;
#else
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
        case EVENT_GROUP_UI_SHELL_BATTERY: {
            app_hear_through_proc_battery_event(event_id, extra_data, data_len);
        }
        break;
#endif /* MTK_BATTERY_MANAGEMENT_ENABLE */
#endif /* AIR_SMART_CHARGER_ENABLE */

        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER: {
            app_hear_through_activity_proc_dm_event(event_id, extra_data, data_len);
        }
        break;

        case EVENT_GROUP_UI_SHELL_KEY:{
            return app_hear_through_proc_key_event(event_id, extra_data, data_len, false);
        }
        break;
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
        case EVENT_GROUP_UI_SHELL_FOTA: {
            app_hear_through_proc_ota_event(event_id, extra_data, data_len);
        }
        break;
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */

#if defined(MTK_ANC_ENABLE) && defined(AIR_HW_VIVID_PT_ENABLE)
        case EVENT_GROUP_UI_SHELL_AUDIO_ANC: {
            app_hear_through_proc_anc_event(event_id, extra_data, data_len);
        }
        break;
#endif /* MTK_ANC_ENABLE && AIR_HW_VIVID_PT_ENABLE */
    }

    return false;
}

static bool app_hear_through_activity_is_out_case()
{
    return (app_hear_through_ctx.is_charger_in == true) ? false : true;
}

static void app_hear_through_activity_handle_mode_index_changed()
{

#ifdef AIR_TWS_ENABLE
    if (app_hear_through_is_aws_connected() == true) {
        app_hear_through_sync_ambient_control_switch_t mode_switch;
        mode_switch.mode_index = app_hear_through_ctx.mode_index;

        apps_aws_sync_send_future_sync_event(false,
                                                EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                                APP_HEAR_THROUGH_AWS_EVENT_ID_AMBIENT_CONTROL,
                                                true,
                                                (uint8_t *)&mode_switch,
                                                sizeof(app_hear_through_sync_ambient_control_switch_t),
                                                APP_HEAR_THROUGH_SYNC_EVENT_DEFAULT_TIMEOUT);
    } else {
#endif /* AIR_TWS_ENABLE */
        app_hear_through_activity_handle_ambient_control_switch();
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
}

void app_hear_through_activity_switch_to_hear_through()
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_switch_to_hear_through] Enter", 0);

    app_hear_through_ctx.mode_index = APP_HEAR_THROUGH_MODE_SWITCH_INDEX_HEAR_THROUGH;
    app_hear_through_activity_handle_mode_index_changed();
}

void app_hear_through_activity_switch_ambient_control()
{
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
        if (app_hear_through_ctx.is_ota_ongoing == true) {
            APPS_LOG_MSGID_W(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_switch_ambient_control] OTA is ongoing, do not support to switch ambient control", 0);
            return;
        }
#endif /* MTK_FOTA_ENABLE && MTK_FOTA_VIA_RACE_CMD */

    uint8_t old_mode_index = app_hear_through_ctx.mode_index;
    app_hear_through_ctx.mode_index ++;
    if (app_hear_through_ctx.mode_index == 3) {
        app_hear_through_ctx.mode_index = 0;
    }

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_switch_ambient_control] mode_index change %d -> %d",
                        2,
                        old_mode_index,
                        app_hear_through_ctx.mode_index);

    app_hear_through_activity_handle_mode_index_changed();
}

void app_hear_through_activity_handle_anc_switched(bool need_sync, bool anc_enable)
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ACT_TAG"[app_hear_through_activity_handle_anc_switched] need_sync : %d, anc_enable : %d",
                        2,
                        need_sync,
                        anc_enable);

    app_anc_service_set_hear_through_enabled(false);

#ifdef AIR_TWS_ENABLE
    if ((app_hear_through_is_aws_connected() == true) && (need_sync == true)) {
        apps_aws_sync_send_future_sync_event(false,
                                                EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                                APP_HEAR_THROUGH_AWS_EVENT_ID_ANC_SWITCHED,
                                                true,
                                                NULL,
                                                0,
                                                0);
    } else {
#endif /* AIR_TWS_ENABLE */
        app_hear_through_activity_handle_anc_state_changed();
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
}

void app_hear_through_activity_power_on_vp_start_to_play()
{
    app_hear_through_ctx.is_power_on_vp_playing = true;
}

#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */


