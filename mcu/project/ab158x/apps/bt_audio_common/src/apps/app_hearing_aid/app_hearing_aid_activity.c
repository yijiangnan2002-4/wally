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

#include "app_hearing_aid_activity.h"
#include "app_hear_through_race_cmd_handler.h"
#include "app_hearing_aid_utils.h"
#include "app_hearing_aid_config.h"
#include "app_hearing_aid_storage.h"
#include "apps_events_event_group.h"
#include "apps_config_event_list.h"
#include "apps_debug.h"
#include "bt_aws_mce.h"
#include "bt_device_manager.h"
#include "bt_device_manager_power.h"
#include "bt_connection_manager.h"
#include "apps_aws_sync_event.h"
#include "stdlib.h"
#include "race_cmd.h"
#include "race_event.h"
#include "ui_shell_manager.h"
#include "voice_prompt_api.h"
#include "voice_prompt_aws.h"
#include "bt_callback_manager.h"
#include "bt_gap.h"
#include "bt_sink_srv_call.h"
#include "apps_config_vp_index_list.h"
#include "app_hearing_aid_key_handler.h"
#include "apps_events_interaction_event.h"
#include "apps_events_bt_event.h"
#include "app_hear_through_storage.h"
#include "app_anc_service.h"
#ifdef AIR_TWS_ENABLE
#include "app_hearing_aid_aws.h"
#endif /* AIR_TWS_ENABLE */
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_utils.h"
#endif /* MTK_IN_EAR_FEATURE_ENABLE */
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#include "apps_events_battery_event.h"
#endif /* MTK_BATTERY_MANAGEMENT_ENABLE */
#ifdef AIR_SMART_CHARGER_ENABLE
#include "app_smcharger.h"
#endif /* AIR_SMART_CHARGER_ENABLE */
#include "app_hear_through_activity.h"
#include "app_anc_service.h"

#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_lea_service_event.h"
#include "bt_sink_srv_le_cap_stream.h"
#endif /* AIR_LE_AUDIO_BIS_ENABLE */

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "app_music_utils.h"
#include "bt_ull_service.h"
#include "bt_ull_le_service.h"
#endif /* AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE */

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
#include "leakage_detection_control.h"
#endif /* MTK_LEAKAGE_DETECTION_ENABLE */
#include "apps_customer_config.h"

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)

#include "audio_anc_psap_control.h"
#include "app_customer_common_activity.h"

#define APP_HA_ACTIVITY_TAG        "[HearingAid][ACTIVITY]"


#define APP_HA_CONTROL_HEARING_AID_BY_A2DP_TIMEOUT_UNIT  (1000) // Unit : 1s
#define APP_HA_CONTROL_HEARING_AID_BY_A2DP_TIMEOUT  (3 * 1000) // 3s
#define APP_HA_CONTROL_HEARING_AID_BY_SCO_TIMEOUT  (3 * 1000) // 3s
#define APP_HA_REQUEST_POWER_OFF_DELAY              (300) // 500ms

#define APP_HA_STATE_DISABLED       0
#define APP_HA_STATE_FWK_OPEN       1
#define APP_HA_STATE_ENABLE         2

/**
 * @brief When power on device, start to play power on VP after HA open framework about the
 * following delay.
 */
#define APP_HA_POWER_ON_VP_DELAY_TIME               50 // 50ms

/*==============================================================================*/
/*                      INTERNAL FUNCTION DECLARATION                           */
/*==============================================================================*/
static void app_hearing_aid_activity_initialization();
static void app_hearing_aid_activity_de_initialization();
static void app_hearing_aid_activity_remove_ha_event();
static bool app_hearing_aid_activity_proc_app_interaction(uint32_t event_id,
                                                          void *extra_data,
                                                          size_t data_len);
static void app_hearing_aid_activity_operate_dvfs(bool lock);
static void app_hearing_aid_activity_update_sco_side_tone_status();
static void app_hearing_aid_activity_handle_sco_status_change(bool sco_start);
static void app_hearing_aid_activity_handle_music_status_change(bool music_start);
extern uint32_t sub_chip_version_get();
uint8_t	need_disable_irsenser=0;
static const uint8_t app_hearing_aid_mode_vp_index_list[] = {
  #if 1 //harry for vp 20240513
    VP_INDEX_AWARE,
    VP_INDEX_SPEECH,
    VP_INDEX_COMFORT,
  #else
    VP_INDEX_HEARING_AID_MODE_1,
    VP_INDEX_HEARING_AID_MODE_2,
    VP_INDEX_HEARING_AID_MODE_3,
    VP_INDEX_HEARING_AID_MODE_4,
    VP_INDEX_HEARING_AID_MODE_5,
    VP_INDEX_HEARING_AID_MODE_6,
    #endif
    VP_INDEX_HEARING_AID_MODE_7,
    VP_INDEX_HEARING_AID_MODE_7,
    VP_INDEX_HEARING_AID_MODE_7,
    VP_INDEX_HEARING_AID_MODE_7,
    VP_INDEX_HEARING_AID_MODE_8,
};

#if 0
typedef struct {
    uint8_t                 sco_connected;
} __attribute__((packed)) app_hearing_aid_app_sync_info_t;
#endif

typedef struct {
    bool                    inited;
    bool                    power_on_ha_executed;
    bool                    vp_streaming;
    bool                    is_open_fwk_done;
    bool                    is_opening_fwk;
    bool                    is_powering_off;
    bool                    is_closing_fwk;
    bool                    anc_path_mask_enable;
    bool                    enter_mp_test_mode;
    bool                    is_anc_suspended_before_enter_mp_test_mode;
    bool                    need_re_open_fwk;
    bool                    is_need_disable_anc;
    bool                    is_anc_disable_done;
    bool                    is_need_play_locally;
    bool                    is_mode_index_vp_played;
    uint8_t                 ha_state;
    uint8_t                 ha_state_before_enter_mp_test_mode;
    int8_t                  partner_rssi;
    uint8_t                 ha_open_caused_by_which_reason;
    uint8_t                 sco_connected;
    bool                    is_charger_in;
    bool                    is_dvfs_locked;
#ifdef MTK_IN_EAR_FEATURE_ENABLE
    bool                    is_in_ear;
    bool                    elder_is_in_ear_switch;
#endif /* MTK_IN_EAR_FEATURE_ENABLE */
} app_hearing_aid_activity_context_t;

app_hearing_aid_activity_context_t  app_ha_activity_context;

bool app_feedback_detection_directly = false;
extern void key_anc_ha_display_proc(void);

bool app_hearing_aid_activity_is_out_case()
{
    return (app_ha_activity_context.is_charger_in == true) ? false : true;
}

bool app_hearing_aid_activity_is_power_off()
{
    return app_ha_activity_context.is_powering_off;
}

void app_hearing_aid_activity_play_vp(uint8_t vp_index, bool need_sync)
{
#ifdef AIR_TWS_ENABLE
    bt_aws_mce_role_t aws_role = bt_device_manager_aws_local_info_get_role();
    bool is_aws_connected = app_hearing_aid_aws_is_connected();

    APPS_LOG_MSGID_I("[app_hearing_aid_activity_play_vp] aws_connected : %d, aws_role : 0x%02x, vp_index : %d, need_sync : %d",
                        4,
                        is_aws_connected,
                        aws_role,
                        vp_index,
                        need_sync);
#else
    APPS_LOG_MSGID_I("[app_hearing_aid_activity_play_vp] vp_index : %d, need_sync : %d",
                        2,
                        vp_index,
                        need_sync);
#endif /* AIR_TWS_ENABLE */

    voice_prompt_param_t vp = {0};
    vp.vp_index = vp_index;
    if (need_sync == true) {
#ifdef AIR_TWS_ENABLE
        if (is_aws_connected == false) {
            vp.control = VOICE_PROMPT_CONTROL_MASK_NONE;
        } else {
            if (aws_role == BT_AWS_MCE_ROLE_AGENT) {
                vp.control = VOICE_PROMPT_CONTROL_MASK_SYNC;
            } else {
                vp.control = VOICE_PROMPT_CONTROL_MASK_NONE;
            }
        }
#else
        vp.control = VOICE_PROMPT_CONTROL_MASK_NONE;
#endif /* AIR_TWS_ENABLE */
    } else {
        vp.control = VOICE_PROMPT_CONTROL_MASK_NONE;
    }
    if(vp_index==VP_INDEX_HEARING_AID_MODE_1)
   {
    vp.delay_time = VOICE_PROMPT_SYNC_DELAY_MIN;
   }
    else
   {
    vp.delay_time = VOICE_PROMPT_SYNC_DELAY_MIN;
   }

#ifdef AIR_TWS_ENABLE
    if ((need_sync == true) && (aws_role == BT_AWS_MCE_ROLE_PARTNER) && (is_aws_connected == true)) {
        app_hearing_aid_aws_sync_vp_play_t play = {0};
        play.vp_index = vp_index;
        app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_PLAY_SYNC_VP,
                                                    (uint8_t *)&play,
                                                    sizeof(app_hearing_aid_aws_sync_vp_play_t),
                                                    false,
                                                    0);
    } else {
#endif /* AIR_TWS_ENABLE */
        voice_prompt_play(&vp, NULL);
#ifdef AIR_TWS_ENABLE
    }
    if(vp_index==42||vp_index==47||vp_index==51)  // harry add for case ui display
    {
		//anc_ha_mode_disp_proc();
		key_anc_ha_display_proc();
    }
#endif /* AIR_TWS_ENABLE */
}

#if 0
void app_hearing_aid_activity_play_mode_index_vp(uint8_t index, bool need_sync)
{
    bt_aws_mce_role_t aws_role = bt_device_manager_aws_local_info_get_role();
    bool is_aws_connected = app_hearing_aid_aws_is_connected();
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_play_mode_index_vp] need_sync : %d,",1,need_sync);
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_play_mode_index_vp] harry aws_role : %d, is_aws_connected : %d,anc_eastech_spec=%d",3,aws_role,is_aws_connected,anc_eastech_spec);
    if((anc_eastech_spec==1)&&(is_aws_connected == true)&&(aws_role == BT_AWS_MCE_ROLE_PARTNER))
    {
    return;
        //app_hearing_aid_activity_play_vp(app_hearing_aid_mode_vp_index_list[index], false);
    }

    else
    {
        app_hearing_aid_activity_play_vp(app_hearing_aid_mode_vp_index_list[index], need_sync);
    }
}
#else
extern uint8_t prompt_no_play_flag;	// richard for UI
void app_hearing_aid_activity_play_mode_index_vp(uint8_t index, bool need_sync)
{
    APPS_LOG_MSGID_I("app_hearing_aid_activity_play_mode_index_vp  prompt_no_play_flag=%d,index=%d,need_sync=%d",3,prompt_no_play_flag,index,need_sync);
	if(prompt_no_play_flag==0)   // 在切HA的时候，如果prompt_no_play_flag=1，已经播放过VP了。在这禁止再播放。harry mask for play vp bug 0708
	{
    		app_hearing_aid_activity_play_vp(app_hearing_aid_mode_vp_index_list[index], need_sync);
	}
}
#endif

void app_hearing_aid_activity_play_ha_on_vp(bool enable, bool need_mode_vp, bool need_sync_play)
{

    uint8_t mode_index = 0;
    bool module_result = false;
    bool need_mode_vp_switch = app_hear_through_storage_get_ha_mode_on_vp_switch();

    APPS_LOG_MSGID_I("[app_hearing_aid_activity_play_ha_on_vp] enable : %d, need_mode_vp : %d, need_mode_vp_switch : %d, need_sync_play : %d",
                        4,
                        enable,
                        need_mode_vp,
                        need_mode_vp_switch,
                        need_sync_play);

    if ((need_mode_vp == true) && (need_mode_vp_switch == true)) {
        module_result = app_hearing_aid_utils_get_mode_index_simple(&mode_index);
    }

#ifdef AIR_TWS_ENABLE

    bt_aws_mce_role_t aws_role = bt_device_manager_aws_local_info_get_role();
    bool is_aws_connected = app_hearing_aid_aws_is_connected();

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_play_ha_on_vp] role : 0x%02x, is_aws_connected : %d",
                        2,
                        aws_role,
                        is_aws_connected);

    if ((is_aws_connected == true) && (need_sync_play == true)) {
        if (aws_role == BT_AWS_MCE_ROLE_AGENT) {
            //app_hearing_aid_activity_play_vp(VP_INDEX_DOORBELL, true);
            if (module_result == true) {
                app_hearing_aid_activity_play_mode_index_vp(mode_index, true);
            }
        }
    } else {
        //app_hearing_aid_activity_play_vp(VP_INDEX_DOORBELL, false);
        if (module_result == true) {
            app_hearing_aid_activity_play_mode_index_vp(mode_index, false);
        }
    }
#else
    app_hearing_aid_activity_play_vp(VP_INDEX_DOORBELL, false);
    if (module_result == true) {
        app_hearing_aid_activity_play_mode_index_vp(mode_index, false);
    }
#endif /* AIR_TWS_ENABLE */
	prompt_no_play_flag=0;    // 恢复提示音开

}

void app_hearing_aid_activity_pre_proc_operate_ha(uint8_t which, bool on, bool need_aws_sync)
{
    bool boot_up_switch_ha = true;
    bool internal_is_origin_on = app_hearing_aid_utils_is_ha_running();
    bool is_out_case = app_hearing_aid_activity_is_out_case();
    bool internal_mix_table_to_enable = false;
    bool internal_drc_to_enable = false;
    bool internal_need_execute = true;
    bool trigger_from_key = false;
    int8_t config_rssi = app_hear_through_storage_get_ha_rssi_threshold();
    app_hearing_aid_state_table_t table;

    bool user_switch_on = app_hearing_aid_utils_is_ha_user_switch_on();
    if (user_switch_on == false) {
        APPS_LOG_MSGID_W(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_pre_proc_operate_ha] current user switch is off state", 0);
        return;
    }

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    if (audio_anc_leakage_compensation_get_status() == true) {
        APPS_LOG_MSGID_W(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_pre_proc_operate_ha] leakage detection is ongoing, cannot execute HA operation", 0);
        return;
    }
#endif /* MTK_LEAKAGE_DETECTION_ENABLE */

#ifdef AIR_TWS_ENABLE
    bool is_aws_connected = app_hearing_aid_aws_is_connected();
    bt_aws_mce_role_t aws_role = bt_device_manager_aws_local_info_get_role();
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_pre_proc_operate_ha] enter, aws_connected : %d, aws_role : 0x%02x",
                        2,
                        is_aws_connected,
                        aws_role);
#endif /* AIR_TWS_ENABLE */
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_pre_proc_operate_ha] enter, which : 0x%02x, on : %d, need_aws_sync : %d, key_triggered : %d",
                        4,
                        which,
                        on,
                        need_aws_sync,
                        trigger_from_key);
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_pre_proc_operate_ha] enter, power_on_executed : %d, powering_off : %d, boot_up_to_enable_ha : %d, out_case : %d, RSSI (%d - %d)",
                        6,
                        app_ha_activity_context.power_on_ha_executed,
                        app_ha_activity_context.is_powering_off,
                        boot_up_switch_ha,
                        is_out_case,
                        app_ha_activity_context.partner_rssi,
                        config_rssi);

    if ((app_ha_activity_context.is_powering_off == true) && (app_hearing_aid_is_mp_test_mode() == false)) {
        return;
    }

    if ((is_out_case == false) && (app_hearing_aid_is_mp_test_mode() == false)) {
        return;
    }

    if (app_hearing_aid_is_mp_test_mode() == true) {
        if (is_out_case == false) {
            app_anc_service_resume();
        }
    }

    if ((app_ha_activity_context.power_on_ha_executed == false)
        && (which != APP_HEARING_AID_CHANGE_CAUSE_POWER_ON)) {
        return;
    }

    if ((which == APP_HEARING_AID_CHANGE_CAUSE_POWER_ON) && (boot_up_switch_ha == false)) {
        app_hearing_aid_utils_set_user_switch(false);
        return;
    }

    if (which == APP_HEARING_AID_CHANGE_CAUSE_BUTTON) {
        trigger_from_key = true;
    }

    {

        memset(&table, 0, sizeof(app_hearing_aid_state_table_t));

        bt_sink_srv_state_t sink_srv_state = bt_sink_srv_get_state();

        if (sink_srv_state == BT_SINK_SRV_STATE_STREAMING) {
            table.a2dp_streaming = true;
        }
        /**
         * @brief Fix issue - 47112
         * Need check BIS streaming or not.
         */
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        if (bt_sink_srv_cap_stream_is_broadcast_streaming() == true) {
            table.a2dp_streaming = true;
        }
#endif /* AIR_LE_AUDIO_BIS_ENABLE */

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        if (app_music_get_ull_is_streaming() == true) {
            table.a2dp_streaming = true;
        }
#endif /* AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE */

        if (app_ha_activity_context.sco_connected != 0) {
            table.sco_streaming = true;
        }

        table.vp_streaming = app_ha_activity_context.vp_streaming;

#ifdef AIR_TWS_ENABLE
        if (is_aws_connected == false) {
            if (config_rssi != 0) {
                table.less_than_threshold = true;
            }
        } else {
            if (config_rssi == 0) {
                table.less_than_threshold = false;
            } else {
                if ((app_ha_activity_context.partner_rssi == 0)
                    || ((app_ha_activity_context.partner_rssi < config_rssi) && (config_rssi != 0))) {
                    table.less_than_threshold = true;
                }
            }
        }
#else
        if (config_rssi != 0) {
            table.less_than_threshold = true;
        } else {
            table.less_than_threshold = false;
        }
#endif

#ifdef MTK_IN_EAR_FEATURE_ENABLE
        bool in_ear_switch = app_hearing_aid_storage_get_in_ear_detection_switch();
        bool anc_suspended = app_anc_service_is_suspended();

        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_pre_proc_operate_ha] enter, in_ear_switch : %d, in_ear_state : %d, anc_suspended : %d",
                            3,
                            in_ear_switch,
                            app_ha_activity_context.is_in_ear,
                            anc_suspended);

        if ((in_ear_switch == false)
                || ((in_ear_switch == true) && (app_ha_activity_context.is_in_ear == true))) {
            if (anc_suspended == true) {
                app_anc_service_resume();
            }
        }

        if (app_ha_activity_context.is_in_ear == true) {
            table.in_ear = true;
        }
#endif /* MTK_IN_EAR_FEATURE_ENABLE */

        internal_mix_table_to_enable = app_hearing_aid_utils_mix_table_to_enable(&table, which, &internal_need_execute);

        internal_drc_to_enable = app_hearing_aid_utils_is_drc_enable(table.a2dp_streaming, table.sco_streaming, table.vp_streaming);

        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_pre_proc_operate_ha] Origin_enable : %d, mix_table_to_enable : %d, drc_to_enable : %d, need_execute : %d, open_fwk_done : %d, is_opening : %d",
                            6,
                            internal_is_origin_on,
                            internal_mix_table_to_enable,
                            internal_drc_to_enable,
                            internal_need_execute,
                            app_ha_activity_context.is_open_fwk_done,
                            app_ha_activity_context.is_opening_fwk);
    }

        if (internal_need_execute == true) {
            if ((table.sco_streaming == true) || (table.a2dp_streaming == true)) {
                app_hearing_aid_activity_operate_dvfs(true);
            } else {
                app_hearing_aid_activity_operate_dvfs(false);
            }
        } else {
            app_hearing_aid_activity_operate_dvfs(false);
        }

        bool operate_ha_result = app_hearing_aid_activity_operate_ha(trigger_from_key,
                                                                        which,
                                                                        internal_need_execute,
                                                                        internal_is_origin_on,
                                                                        internal_mix_table_to_enable,
                                                                        internal_drc_to_enable);

        if (operate_ha_result == false) {
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_pre_proc_operate_ha] Operate HA failed", 0);
            return;
        }
}

bool app_hearing_aid_activity_process_race_cmd(void *race_data, size_t race_data_len)
{
    if (app_ha_activity_context.inited == false) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_process_race_cmd] Not inited", 0);
        return false;
    }

    // app_hearing_aid_activity_handle_race_cmd(race_data, race_data_len);
    if ((race_data == NULL) || (race_data_len == 0)) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_process_race_cmd] Data error (0x%04x, %d)", 2, race_data, race_data_len);
        return false;
    }

    uint8_t execute_where = APP_HEARING_AID_EXECUTE_NONE;
    bool need_sync_execute = false;

    app_hear_through_request_t *request = (app_hear_through_request_t *)race_data;

    if ((request->op_code != APP_HEAR_THROUGH_CMD_OP_CODE_SET)
        && (request->op_code != APP_HEAR_THROUGH_CMD_OP_CODE_GET)) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_process_race_cmd] Unknown code : %d",
                            1,
                            request->op_code);
        return false;
    }

    execute_where = app_hearing_aid_config_get_where_to_execute(request->op_code, request->op_type);
    need_sync_execute = app_hearing_aid_config_get_need_execute_set_cmd_sync(request->op_type);

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_process_race_cmd] handle code : %s, type : %s, where : %s, need_sync_execute : %d,op_parameter[0]=%d",
                        5,
                        app_hearing_aid_command_string[request->op_code],
                        app_hearing_aid_type_string[request->op_type],
                        app_hearing_aid_execute_where_string[execute_where],
                        need_sync_execute,request->op_parameter[0]);
#if 1 // harry for app ha vp 20240904
		if(request->op_code == APP_HEAR_THROUGH_CMD_OP_CODE_SET&&request->op_type== 0x0007)
		{
                app_hearing_aid_activity_play_mode_index_vp(request->op_parameter[0], true);

		}
	
		if(request->op_code == APP_HEAR_THROUGH_CMD_OP_CODE_SET&&request->op_type== 0x001A)
		{
			if(request->op_parameter[0]==0)
			{
		            need_disable_irsenser=1;
			}
			else
			{
		            need_disable_irsenser=0;
			}
                APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_process_race_cmd] need_disable_irsenser=%d, request->op_parameter[0]=%d", 2,need_disable_irsenser,request->op_parameter[0]);

		}

#endif
#ifdef AIR_TWS_ENABLE
    if ((app_hearing_aid_aws_is_connected() == true) && (execute_where == APP_HEARING_AID_EXECUTE_ON_BOTH)) {

        uint32_t delay_ms = 0;

        if (need_sync_execute == true) {
            delay_ms = APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT;
        } else {
            delay_ms = 0;
        }

        app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_RACE_CMD_REQUEST,
                                                    race_data,
                                                    race_data_len,
                                                    true,
                                                    delay_ms);
    } else {
#endif /* AIR_TWS_ENABLE */

    if (request->op_code == APP_HEAR_THROUGH_CMD_OP_CODE_GET) {
        uint8_t get_response[APP_HEARING_AID_RESPONSE_MAX_LEN] = {0};
        uint16_t get_response_len = 0;

        if (request->op_type == APP_HEARING_AID_CONFIG_TYPE_FEEDBACK_DETECTION) {
            app_feedback_detection_directly = true;
        }

        app_hearing_aid_activity_handle_get_race_cmd(race_data, race_data_len, get_response, &get_response_len);
        if (request->op_type == APP_HEARING_AID_CONFIG_TYPE_MIC_CALIBRATION_DATA) {
            /**
             * @brief If it's get mic calibration data and AWS is not connected
             * Need fill in the get response data according to current audio channel.
             */
            uint8_t *rsp_data = (uint8_t *)pvPortMalloc(app_hearing_aid_utils_get_combine_response_length(request->op_type));
            if (rsp_data == NULL) {
                APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_process_race_cmd] Failed to allocate response buffer", 0);
                return false;
            }

            memset(rsp_data, 0, APP_HEARING_AID_RESPONSE_MAX_LEN * 2);

            audio_psap_device_role_t role = app_hearing_aid_utils_get_role();
            if (role == AUDIO_PSAP_DEVICE_ROLE_LEFT) {
                memcpy(rsp_data, get_response, get_response_len);
            } else if (role == AUDIO_PSAP_DEVICE_ROLE_RIGHT) {
                memcpy(rsp_data + APP_HEARING_AID_RESPONSE_MAX_LEN, get_response, get_response_len);
            } else {
                APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_process_race_cmd] Unknown device role", 0);
                return false;
            }
            app_hear_through_race_cmd_send_get_response(request->op_type, RACE_ERRCODE_SUCCESS, rsp_data, APP_HEARING_AID_RESPONSE_MAX_LEN * 2);

            vPortFree(rsp_data);
            rsp_data = NULL;
        } else {
            app_hear_through_race_cmd_send_get_response(request->op_type, RACE_ERRCODE_SUCCESS, get_response, get_response_len);
        }
    } else {
        bool set_result = app_hearing_aid_activity_handle_set_race_cmd(race_data, race_data_len);
        app_hear_through_race_cmd_send_set_response(request->op_type, ((set_result == true) ? RACE_ERRCODE_SUCCESS : RACE_ERRCODE_FAIL));
    }

#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
    return true;
}

bool app_hearing_aid_activity_handle_set_race_cmd(uint8_t *race_data, uint16_t race_data_len)
{
    bool set_result = false;
    app_hear_through_request_t *request = (app_hear_through_request_t *)race_data;

    if (request->op_code != APP_HEAR_THROUGH_CMD_OP_CODE_SET) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_set_race_cmd] Operate code is not SET : 0x%02x",
                            1,
                            request->op_code);
        return false;
    }

    app_hearing_aid_utils_handle_set_race_cmd(request, &set_result);

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_set_race_cmd] SET type : 0x%04x (%s), set result : %d",
                        3,
                        request->op_type,
                        app_hearing_aid_type_string[request->op_type],
                        set_result);

    return set_result;
}

void app_hearing_aid_activity_handle_get_race_cmd(uint8_t *race_data, uint16_t race_data_len, uint8_t *get_response, uint16_t *get_response_len)
{
    app_hear_through_request_t *request = (app_hear_through_request_t *)race_data;

    if (request->op_code != APP_HEAR_THROUGH_CMD_OP_CODE_GET) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_get_race_cmd] Operate code is not GET : 0x%02x",
                            1,
                            request->op_code);
        return;
    }

    app_hearing_aid_utils_handle_get_race_cmd(request, get_response, get_response_len);

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_get_race_cmd] GET type : 0x%04x, get length : %d",
                        2,
                        request->op_type,
                        *get_response_len);
}
extern uint8_t charge_out_flag;

bool app_hearing_aid_activity_open_hearing_aid_fwk()
{

    if (app_anc_service_is_suspended() == true) {
	if(charge_out_flag==1)
	{
           APPS_LOG_MSGID_W(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_open_hearing_aid_fwk] just change out,so resume anc", 0);
           app_anc_service_resume();
           charge_out_flag=0;
	}
	else
	{
           APPS_LOG_MSGID_W(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_open_hearing_aid_fwk] ANC has been suspended", 0);
	   charge_out_flag=0;
           return false;
	}
    }
	charge_out_flag=0;
	
    if (app_ha_activity_context.inited == false) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_open_hearing_aid_fwk] Not inited", 0);
        return false;
    }

    if (app_hearing_aid_utils_is_ha_user_switch_on() == false) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_open_hearing_aid_fwk] User switch is not ON", 0);
        return false;
    }

    if (app_ha_activity_context.is_open_fwk_done == true) {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_open_hearing_aid_fwk] Framework already opened", 0);
        return true;
    } else {
        if (app_ha_activity_context.is_opening_fwk == true) {
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_open_hearing_aid_fwk] Framework is opening", 0);
            return true;
        }
    }

    if (app_ha_activity_context.is_closing_fwk == true) {
        APPS_LOG_MSGID_W(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_open_hearing_aid_fwk] Current is closing FWK", 0);
        app_ha_activity_context.need_re_open_fwk = true;
        return false;
    }

    bool fwk_result = app_hearing_aid_utils_control_fwk(true, app_ha_activity_context.anc_path_mask_enable);
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_open_hearing_aid_fwk] Enable framework result : %d, anc_path_mask_enable : %d",
                        2,
                        fwk_result,
                        app_ha_activity_context.anc_path_mask_enable);

    if (fwk_result == false) {
        return false;
    }

    app_ha_activity_context.is_open_fwk_done = false;
    app_ha_activity_context.is_opening_fwk = true;
    return true;
}

void app_hearing_aid_activity_open_hearing_aid_fwk_with_zero_path()
{
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_open_hearing_aid_fwk_with_zero_path] is_opening_fwk : %d, is_closing_fwk : %d",
                        2,
                        app_ha_activity_context.is_opening_fwk,
                        app_ha_activity_context.is_closing_fwk);

    app_ha_activity_context.anc_path_mask_enable = false;

    if (app_ha_activity_context.is_closing_fwk == true) {
        app_ha_activity_context.need_re_open_fwk = true;
        return;
    }

    if (app_ha_activity_context.is_opening_fwk == true) {
        app_ha_activity_context.need_re_open_fwk = true;
    } else {
        app_ha_activity_context.is_open_fwk_done = false;
        app_hearing_aid_activity_open_hearing_aid_fwk();
    }
}

void app_hearing_aid_activity_set_open_fwk_done(bool result)
{
    if (app_ha_activity_context.inited == false) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_set_open_fwk_done] Not inited", 0);
        return;
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_set_open_fwk_done] Result : %d, need_disable_anc : %d, anc_disable_done : %d, mp_test_mode : %d, need_reopen_fwk : %d, opening_fwk : %d, closing_fwk : %d",
                        7,
                        result,
                        app_ha_activity_context.is_need_disable_anc,
                        app_ha_activity_context.is_anc_disable_done,
                        app_ha_activity_context.enter_mp_test_mode,
                        app_ha_activity_context.need_re_open_fwk,
                        app_ha_activity_context.is_opening_fwk,
                        app_ha_activity_context.is_closing_fwk);

    if (result == true) {

        /**
         * @brief Workaround - when MIC changed, need disable ANC and then enable HA FWK again
         * If is opening framework before, need disable ANC.
         */
        if (app_ha_activity_context.is_need_disable_anc == true) {
            app_ha_activity_context.need_re_open_fwk = false;
            app_ha_activity_context.is_opening_fwk = false;
            app_ha_activity_context.is_open_fwk_done = false;
            app_anc_service_disable_without_notify_hear_through();
            return;
        }

        /**
         * @brief If need re-open fwk, need open fwk again - For MP test mode.
         */
        if (app_ha_activity_context.need_re_open_fwk == true) {
            app_ha_activity_context.is_open_fwk_done = false;
            app_ha_activity_context.is_opening_fwk = false;
            app_ha_activity_context.need_re_open_fwk = false;
            app_hearing_aid_activity_open_hearing_aid_fwk();
            return;
        }

        app_ha_activity_context.is_open_fwk_done = true;
        app_ha_activity_context.is_opening_fwk = false;

        app_ha_activity_context.ha_state = APP_HA_STATE_FWK_OPEN;

        if (app_ha_activity_context.enter_mp_test_mode == true) {
            app_hearing_aid_utils_enable_mp_test_mode(true);
        }
    } else {
        app_ha_activity_context.is_open_fwk_done = false;
        app_ha_activity_context.is_opening_fwk = false;
        app_ha_activity_context.is_closing_fwk = false;
#if 0
        if ((app_ha_activity_context.enter_mp_test_mode == true)
                && (app_ha_activity_context.is_need_disable_anc == false)) {
            app_hearing_aid_utils_enable_mp_test_mode(false);
            app_ha_activity_context.enter_mp_test_mode = false;
        }
#endif
        app_hearing_aid_activity_operate_dvfs(false);

        /**
         * @brief Workaround - when MIC changed, need disable ANC and then enable HA FWK again
         * If HA framework disable done, check ANC disable done or not, if done, need open HA FWK.
         */
        if ((app_ha_activity_context.is_need_disable_anc == false)
                && (app_ha_activity_context.is_anc_disable_done == true)
                && (app_ha_activity_context.need_re_open_fwk == false)) {
            app_ha_activity_context.is_anc_disable_done = false;
            app_hearing_aid_activity_open_hearing_aid_fwk();
        }

        if (app_ha_activity_context.need_re_open_fwk == true) {
            app_ha_activity_context.is_open_fwk_done = false;
            app_ha_activity_context.is_opening_fwk = false;
            app_ha_activity_context.need_re_open_fwk = false;
            app_hearing_aid_activity_open_hearing_aid_fwk();
        }
    }
}

/**
 * @brief Fix issue - 49144
 * When power on partner, agent send configuration to partner, but partner do not play HA on VP.
 */
void app_hearing_aid_activity_set_need_play_locally(bool play_locally)
{
    app_ha_activity_context.is_need_play_locally = play_locally;
}

void app_hearing_aid_activity_set_mode_vp_played(bool played)
{
    app_ha_activity_context.is_mode_index_vp_played = played;
}

bool app_hearing_aid_activity_is_open_fwk_done()
{
    return app_ha_activity_context.is_open_fwk_done;
}

bool app_hearing_aid_activity_is_fwk_opening()
{
    return app_ha_activity_context.is_opening_fwk;
}

bool app_hearing_aid_activity_is_sco_ongoing()
{
    return ((app_ha_activity_context.sco_connected > 0) ? true : false);
}

bool app_hearing_aid_activity_is_need_reopen_fwk()
{
    return app_ha_activity_context.need_re_open_fwk;
}

bool app_hearing_aid_activity_enable_hearing_aid(bool from_key)
{
    if (app_ha_activity_context.inited == false) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_enable_hearing_aid] Not inited", 0);
        return false;
    }

    if (app_hearing_aid_utils_is_ha_user_switch_on() == false) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_enable_hearing_aid] User switch is not ON", 0);
        /**
         * @brief Fix issue - 47410
         * If user switch is off, should close HA FWK
         */
        app_hearing_aid_utils_control_fwk(false, false);
        app_ha_activity_context.is_closing_fwk = true;
        app_ha_activity_context.ha_state = APP_HA_STATE_DISABLED;
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_enable_hearing_aid] HA is only FWK open state, mp_test_mode : %d, ha_state_before_mp_test_mode : %d, closing_fwk : %d",
                        3,
                        app_ha_activity_context.enter_mp_test_mode,
                        app_ha_activity_context.ha_state_before_enter_mp_test_mode,
                        app_ha_activity_context.is_closing_fwk);

    if (app_ha_activity_context.is_closing_fwk == true) {
        return false;
    }

    if ((app_ha_activity_context.enter_mp_test_mode == false)
        && (app_ha_activity_context.ha_state_before_enter_mp_test_mode != APP_HA_STATE_ENABLE)) {
        app_ha_activity_context.ha_state_before_enter_mp_test_mode = APP_HA_STATE_ENABLE;
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_enable_hearing_aid] from_key : %d, FWK open done : %d, power_on_ha_executed : %d, stored_open_caused_by_which : 0x%02x, anc_path_mask_enable : %d",
                        5,
                        from_key,
                        app_ha_activity_context.is_open_fwk_done,
                        app_ha_activity_context.power_on_ha_executed,
                        app_ha_activity_context.ha_open_caused_by_which_reason,
                        app_ha_activity_context.anc_path_mask_enable);

    if ((app_ha_activity_context.is_open_fwk_done == false)
            || (app_ha_activity_context.anc_path_mask_enable == false)) {
        return false;
    }

    if (app_ha_activity_context.power_on_ha_executed == false) {
        app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_POWER_ON, true, false);
        app_ha_activity_context.power_on_ha_executed = true;

#ifdef AIR_TWS_ENABLE
        app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner();
        app_hearing_aid_aws_send_middleware_configuration_sync_request();
#endif /* AIR_TWS_ENABLE */

    } else {
        if (from_key == true) {
            app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_BUTTON, true, false);
        } else {
            if (app_ha_activity_context.ha_open_caused_by_which_reason != 0x00) {
                app_hearing_aid_activity_pre_proc_operate_ha(app_ha_activity_context.ha_open_caused_by_which_reason, true, false);
                app_ha_activity_context.ha_open_caused_by_which_reason = 0x00;
            } else {
                app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_RACE_CMD, true, false);
            }
        }
    }

    return true;
}

void app_hearing_aid_activity_set_power_on_played()
{
    app_ha_activity_context.power_on_ha_executed = true;
}

bool app_hearing_aid_activity_disable_hearing_aid(bool need_vp)
{
    if (app_ha_activity_context.inited == false) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_disable_hearing_aid] Not inited", 0);
        return false;
    }

    if (app_ha_activity_context.is_open_fwk_done == true) {
        bool disable_ha_result = app_hearing_aid_utils_control_ha(false);

        bool fwk_result = app_hearing_aid_utils_control_fwk(false, false);
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_disable_hearing_aid] enter_mp_mode : %d, disable_ha_result : %d, close framework result : %d, need_vp : %d",
                            4,
                            app_ha_activity_context.enter_mp_test_mode,
                            disable_ha_result,
                            fwk_result,
                            need_vp);

        if ((disable_ha_result == true) && (fwk_result == true)) {
            app_ha_activity_context.ha_state = APP_HA_STATE_DISABLED;
        }
        if (fwk_result == true) {
            app_ha_activity_context.is_closing_fwk = true;
        }
    } else {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_disable_hearing_aid] Hearing Aid Already closed", 0);
    }

    app_hearing_aid_activity_remove_ha_event();

    /**
     * @brief Play VP to notify HA is off.
     */
    if ((need_vp == true) && (app_ha_activity_context.is_powering_off == false)) {
        app_hearing_aid_activity_play_ha_on_vp(false, false, true);
    }

    return true;
}

bool app_hearing_aid_activity_is_hearing_aid_on()
{
    return app_hearing_aid_utils_is_ha_running();
}

bool app_hearing_aid_activity_set_user_switch(bool need_sync, bool enable)
{
    if (app_ha_activity_context.inited == false) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_set_user_switch] Not inited", 0);
        return false;
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_set_user_switch] need_sync : %d, enable : %d, power_on_ha_executed : %d",
                        3,
                        need_sync,
                        enable,
                        app_ha_activity_context.power_on_ha_executed);

    app_hearing_aid_utils_set_user_switch(enable);

    if (enable == true) {
        app_ha_activity_context.anc_path_mask_enable = true;
    }

    /**
     * @brief Fix issue - 47552
     * New agent already change to ANC, when partner power on, if the power_on_ha_executed is false
     * need mark to be true, otherwise, when switch ANC to HA, there will play mode index vp.
     */
    if ((enable == false) && (app_ha_activity_context.power_on_ha_executed == false)) {
        app_ha_activity_context.power_on_ha_executed = true;
    }

#ifdef AIR_TWS_ENABLE
    if (enable == false) {
        app_hearing_aid_aws_reset_middleware_configuration();
    }

    bool is_aws_connected = app_hearing_aid_aws_is_connected();
    if ((is_aws_connected == true) && (need_sync == true)) {
        app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_SET_USER_SWITCH,
                                                    (uint8_t *)(&enable),
                                                    sizeof(bool),
                                                    false,
                                                    0);
    }
#endif /* AIR_TWS_ENABLE */
    return true;
}

bool app_hearing_aid_activity_operate_ha(bool trigger_from_key, uint8_t which, bool mix_table_need_execute, bool is_origin_on, bool mix_table_to_enable, bool drc_to_enable)
{
    if (app_ha_activity_context.inited == false) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_operate_ha] Not inited", 0);
        return false;
    }

    if ((app_hearing_aid_activity_is_out_case() == false)
            && (app_hearing_aid_is_mp_test_mode() == false)) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_operate_ha] Current is NOT out case", 0);
        return false;
    }

    bool user_switch_on = app_hearing_aid_utils_is_ha_user_switch_on();
    if (user_switch_on == false) {
        APPS_LOG_MSGID_W(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_operate_ha] current user switch is off state", 0);
        return false;
    }

    bool need_notify = false;
    bool notify_result = false;
    bool need_vp = false;
    // bool is_opening = app_ha_activity_context.is_opening_fwk;

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_operate_ha] which : 0x%02x, need_execute : %d, is_origin_on : %d, mix_table_to_enable : %d, drc_to_enable : %d",
                        5,
                        which,
                        mix_table_need_execute,
                        is_origin_on,
                        mix_table_to_enable,
                        drc_to_enable);

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_operate_ha] fwk_done : %d, opening_fwk : %d, closing_fwk : %d, is_key_triggered : %d, anc_path_mask_enable : %d",
                        5,
                        app_ha_activity_context.is_open_fwk_done,
                        app_ha_activity_context.is_opening_fwk,
                        app_ha_activity_context.is_closing_fwk,
                        trigger_from_key,
                        app_ha_activity_context.anc_path_mask_enable);

    if (((mix_table_to_enable == true) && (mix_table_need_execute == true))
        || (drc_to_enable == true)) {

        if (app_ha_activity_context.is_closing_fwk == true) {
            app_ha_activity_context.ha_open_caused_by_which_reason = which;
            app_ha_activity_context.anc_path_mask_enable = true;
            app_ha_activity_context.need_re_open_fwk = true;
            return true;
        }

        if (app_ha_activity_context.is_open_fwk_done == false) {
            app_ha_activity_context.anc_path_mask_enable = true;
            app_ha_activity_context.ha_open_caused_by_which_reason = which;

            if (app_ha_activity_context.is_opening_fwk == false) {
                app_hearing_aid_activity_open_hearing_aid_fwk();
            } else {
                app_ha_activity_context.need_re_open_fwk = true;
            }
        } else {
            if (mix_table_need_execute == true) {

                bool control_ha_result = false;

                if ((mix_table_to_enable == true) && (app_ha_activity_context.anc_path_mask_enable == false)) {
                    app_ha_activity_context.anc_path_mask_enable = true;
                    app_ha_activity_context.is_open_fwk_done = false;

                    app_ha_activity_context.ha_open_caused_by_which_reason = which;

                    if (app_ha_activity_context.is_opening_fwk == false) {
                        app_hearing_aid_activity_open_hearing_aid_fwk();
                    } else {
                        app_ha_activity_context.need_re_open_fwk = true;
                    }
                    return true;
                } else {
                    control_ha_result = app_hearing_aid_utils_control_ha(mix_table_to_enable);
                    app_ha_activity_context.is_need_play_locally = false;
                }

                if (control_ha_result == false) {
                    APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_operate_ha] Failed to control HA (%d)", 1, mix_table_to_enable);

                    app_hearing_aid_utils_control_fwk(false, false);
                    app_ha_activity_context.is_closing_fwk = true;
                    app_ha_activity_context.ha_state = APP_HA_STATE_DISABLED;

                    /**
                     * @brief Fix issue - 49332
                     * Workaround, need disable side tone if SCO is ongoing and PSAP is on.
                     */
                    app_hearing_aid_activity_update_sco_side_tone_status();
                    return false;
                }

                need_notify = true;

                if (mix_table_to_enable == true) {
                    app_ha_activity_context.ha_state = APP_HA_STATE_ENABLE;
                } else {
                    app_ha_activity_context.ha_state = APP_HA_STATE_FWK_OPEN;
                }

                if (mix_table_to_enable == true) {
                    notify_result = true;
                } else {
                    notify_result = false;
                }

                need_vp = true;
            }
        }
    }

    /**
     * @brief Fix issue
     * If is opening FWK, but later is ready to disable, need disable FWk.
     */
    if ((mix_table_to_enable == false) && (mix_table_need_execute == true) && (drc_to_enable == false)) {

        app_hearing_aid_activity_disable_hearing_aid(false);

        app_ha_activity_context.is_need_play_locally = false;

        need_notify = true;
        need_vp = true;
        notify_result = false;
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_operate_ha] need_vp : %d, need_notify : %d, notify_result : %d, need_play_locally : %d",
                        4,
                        need_vp,
                        need_notify,
                        notify_result,
                        app_ha_activity_context.is_need_play_locally);

    if (need_vp == true) {
        if ((which == APP_HEARING_AID_CHANGE_CAUSE_BUTTON)
                || (which == APP_HEARING_AID_CHANGE_CAUSE_POWER_ON)
                || (which == APP_HEARING_AID_CHANGE_CAUSE_RACE_CMD)
                || (which == APP_HEARING_AID_CHANGE_CAUSE_MIX_TABLE_SWITCH)
                || (trigger_from_key == true)) {

            bool notify_ha_enable = false;

            if (mix_table_to_enable == true) {
                if (which == APP_HEARING_AID_CHANGE_CAUSE_POWER_ON) {
                    notify_ha_enable = true;
                    app_hearing_aid_activity_play_ha_on_vp(notify_result, true, false);
                    app_ha_activity_context.is_mode_index_vp_played = true;
                } else {
                    if (is_origin_on == false) {
                        notify_ha_enable = true;

                        bool need_sync_play = (app_ha_activity_context.is_need_play_locally == false) ? true : false;
                        bool need_play_mode_index = (app_ha_activity_context.is_mode_index_vp_played == false) ? true : false;

                        app_hearing_aid_activity_play_ha_on_vp(notify_result, need_play_mode_index, need_sync_play);

                        app_ha_activity_context.is_mode_index_vp_played = true;
                    }
                    app_ha_activity_context.is_need_play_locally = false;
                }
            }

            if ((is_origin_on == true) && (mix_table_to_enable == false)) {
                notify_ha_enable = true;
                app_hearing_aid_activity_play_ha_on_vp(false, false, true);
            }

            if (notify_ha_enable == true) {
                app_hear_through_race_cmd_send_notification(APP_HEARING_AID_CONFIG_TYPE_HA_SWITCH,
                                                            (uint8_t *)(&notify_result),
                                                            sizeof(bool));
            }
        }
    }

    app_hearing_aid_activity_update_sco_side_tone_status();

#ifdef AIR_TWS_ENABLE
    /**
     * @brief Fix issue : 45276
     * Root cause : When agent connected with partner, agent read the HA disable parameter to partner,
     *              but partner is playing VP, after VP finished, the partner will disable HA.
     * Solution : Sync the middleware configuration to partner side after the HA changed.
     * Limitation : There are some timing delay between agent and partner side.
     */
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
        app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner();
    }
#endif /* AIR_TWS_ENABLE */

    if (need_notify == true) {
        if (notify_result == true) {
            ui_shell_send_event(false,
                                EVENT_PRIORITY_MIDDLE,
                                EVENT_GROUP_UI_SHELL_HEARING_AID,
                                APP_HEARING_AID_EVENT_ID_HA_ON,
                                NULL,
                                0,
                                NULL,
                                0);
        } else {
            if ((is_origin_on == true) && (mix_table_to_enable == false)) {
                ui_shell_send_event(false,
                                    EVENT_PRIORITY_MIDDLE,
                                    EVENT_GROUP_UI_SHELL_HEARING_AID,
                                    APP_HEARING_AID_EVENT_ID_HA_OFF,
                                    NULL,
                                    0,
                                    NULL,
                                    0);
            }
        }
    }

    return true;
}

void app_hearing_aid_activity_proc_vp_streaming_state_change(bool streaming)
{
    bool user_switch_on = app_hearing_aid_utils_is_ha_user_switch_on();

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_vp_streaming_state_change] VP streaming : %d -> %d, inited : %d, user_switch_on : %d",
                        4,
                        app_ha_activity_context.vp_streaming,
                        streaming,
                        app_ha_activity_context.inited,
                        user_switch_on);

    app_ha_activity_context.vp_streaming = streaming;

    if ((app_ha_activity_context.inited == false) || (user_switch_on == false)) {
#if APP_HEARING_AID_DEBUG_ENABLE
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_vp_streaming_state_change] Not ready to process VP state change event, inited : %d, user_switch : %d",
                            2,
                            app_ha_activity_context.inited,
                            user_switch_on);
#endif /* APP_HEARING_AID_DEBUG_ENABLE */
        return;
    }

#ifdef AIR_TWS_ENABLE
    app_hearing_aid_aws_set_vp_streaming_state(streaming);
#endif /* AIR_TWS_ENABLE */

#if 0
    bool on = app_hearing_aid_storage_get_vp_mix_switch();
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_vp_streaming_state_change] VP mix switch on : %d",
                        1,
                        on);

#ifdef AIR_TWS_ENABLE
    // if ((bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT)
    //         || (app_hearing_aid_aws_is_connected() == false)) {
#endif /* AIR_TWS_ENABLE */
        app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_VP, false, true);
#ifdef AIR_TWS_ENABLE
    // }
#endif /* AIR_TWS_ENABLE */
#endif /* 0 */
}

bool app_hearing_aid_is_mp_test_mode()
{
    return app_ha_activity_context.enter_mp_test_mode;
}

bool app_hearing_aid_is_need_enable_ha()
{
    return ((app_ha_activity_context.enter_mp_test_mode == false) && (app_ha_activity_context.is_need_disable_anc == false));
}

/**
 * @brief Fix issue - 49332
 * Workaround, need disable side tone if SCO is ongoing and PSAP is on.
 */
bool app_hearing_aid_is_ready_to_enable_side_tone()
{
    // bool is_sco_mix_mode_on = app_hearing_aid_utils_is_sco_mix_mode_on();
    bool is_ha_running = app_hearing_aid_utils_is_ha_running();
    bool is_user_switch_on = app_hearing_aid_utils_is_ha_user_switch_on();
    bool is_out_case = app_hearing_aid_activity_is_out_case();

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_is_ready_to_enable_side_tone] inited : %d, user_switch_on : %d, ha_running : %d, out_case : %d, fwk_open status : %d - %d",
                        6,
                        app_ha_activity_context.inited,
                        is_user_switch_on,
                        is_ha_running,
                        is_out_case,
                        app_ha_activity_context.is_open_fwk_done,
                        app_ha_activity_context.is_opening_fwk);

    /**
     * @brief Fix issue - 51169
     * When in charger case, no need to enable side tone while calling.
     */
    if (is_out_case == false) {
        return false;
    }

    if ((app_ha_activity_context.inited == true)
            && (is_user_switch_on == true)
            // && (is_sco_mix_mode_on == true)
            && ((is_ha_running == true)
                || (app_ha_activity_context.is_open_fwk_done == true)
                || (app_ha_activity_context.is_opening_fwk == true))) {
        return false;
    }

    return true;
}

bool app_hearing_aid_is_supported_cmd(uint16_t cmd_type)
{
    if ((cmd_type == APP_HEARING_AID_CONFIG_TYPE_NONE)
        || (cmd_type >= APP_HEARING_AID_CONFIG_TYPE_MAX)) {
        return false;
    }
    return true;
}

#ifdef AIR_TWS_ENABLE
void app_hearing_aid_activity_handle_rssi_operation(int8_t rssi)
{

    app_ha_activity_context.partner_rssi = rssi;

    int8_t config_rssi = app_hear_through_storage_get_ha_rssi_threshold();
    if (config_rssi == 0x00) {
        return;
    }

    bool on = false;
    bool result = app_hearing_aid_utils_is_rssi_mix_switch_on(&on);
    uint32_t power_off_timeout = app_hear_through_storage_get_ha_rssi_power_off_timeout();

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_rssi_operation] rssi_read_result : %d, rssi_mix_table_enabled : %d, config_rssi : %d, partner_rssi : %d, power_off_timeout : %d",
                        5,
                        result,
                        on,
                        config_rssi,
                        app_ha_activity_context.partner_rssi,
                        power_off_timeout);

    if (result == false || on == false) {
        return;
    }

    app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_RSSI, false, false);

    if (power_off_timeout > 0) {
        if (app_ha_activity_context.partner_rssi > config_rssi) {
            ui_shell_send_event(false,
                                EVENT_PRIORITY_MIDDLE,
                                EVENT_GROUP_UI_SHELL_HEARING_AID,
                                APP_HEARING_AID_EVENT_ID_RSSI_POWER_OFF,
                                NULL,
                                0,
                                NULL,
                                power_off_timeout);
        } else {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_RSSI_POWER_OFF);
        }
    }
}
#endif /* AIR_TWS_ENABLE */

static void app_hearing_aid_activity_handle_music_status_change(bool music_start)
{
    bool user_switch = app_hearing_aid_utils_is_ha_user_switch_on();
    bool is_out_case = app_hearing_aid_activity_is_out_case();
    bool music_operate_delay_switch = false;
    uint8_t music_operate_delay_timeout = 0;

    app_hearing_aid_storage_get_music_operate_ha_configuration(&music_operate_delay_switch, &music_operate_delay_timeout);

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_music_status_change] user_switch : %d, inited : %d, music_start : %d, music_operate_delay_switch : %d, delay_timeout : %d, out_case : %d",
                        6,
                        user_switch,
                        app_ha_activity_context.inited,
                        music_start,
                        music_operate_delay_switch,
                        music_operate_delay_timeout,
                        is_out_case);

    if ((user_switch == true)
            && (app_ha_activity_context.inited == true)
            && (is_out_case == true)) {
        /**
         * @brief remove the request to control HA event
         *  if received the A2DP started again in 3s to avoid execute operate HA multiple times.
         *  if receive A2DP stopped in 3s to avoid execute operate HA again.
         */
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA);
        bool need_aws_sync = true;

        if (music_start == true) {

            uint32_t delay_timeout = APP_HA_CONTROL_HEARING_AID_BY_A2DP_TIMEOUT;

            /**
             * @brief if music_operate_delay_switch is false and music_operate_delay_timeout is 0, need
             * delay to control HA according to the macro APP_HA_CONTROL_HEARING_AID_BY_A2DP_TIMEOUT.
             * otherwise, if music_operate_delay_switch is true and music_operate_delay_timeout > 0, then need
             * delay the music_operate_delay_timeout * APP_HA_CONTROL_HEARING_AID_BY_A2DP_TIMEOUT_UNIT to control HA.
             */
            if ((music_operate_delay_switch == true) || (music_operate_delay_timeout > 0)) {
                delay_timeout = 0;
                if ((music_operate_delay_switch == true) && (music_operate_delay_timeout > 0)) {
                    delay_timeout = music_operate_delay_timeout * APP_HA_CONTROL_HEARING_AID_BY_A2DP_TIMEOUT_UNIT;
                }
            }

            ui_shell_send_event(false,
                                EVENT_PRIORITY_MIDDLE,
                                EVENT_GROUP_UI_SHELL_HEARING_AID,
                                APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA,
                                (void *)need_aws_sync,
                                0,
                                NULL,
                                delay_timeout);
        } else {
            ui_shell_send_event(false,
                                EVENT_PRIORITY_MIDDLE,
                                EVENT_GROUP_UI_SHELL_HEARING_AID,
                                APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA,
                                (void *)need_aws_sync,
                                0,
                                NULL,
                                0);
        }
    }
}

static void app_hearing_aid_activity_handle_sco_status_change(bool sco_start)
{
    bool user_switch = app_hearing_aid_utils_is_ha_user_switch_on();
    bool is_out_case = app_hearing_aid_activity_is_out_case();
    uint8_t old_sco_connected = app_ha_activity_context.sco_connected;
    bool need_aws_sync = true;
    if (sco_start == true) {
        app_ha_activity_context.sco_connected += 1;
    } else {
        if (app_ha_activity_context.sco_connected > 0) {
            app_ha_activity_context.sco_connected -= 1;
        }
    }

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA);

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_sco_status_change] SCO state change from : %d -> %d, out_case : %d",
                        3,
                        old_sco_connected,
                        app_ha_activity_context.sco_connected,
                        is_out_case);

    if ((old_sco_connected != app_ha_activity_context.sco_connected)
            && (user_switch == true)
            && (app_ha_activity_context.inited == true)
            && (is_out_case == true)) {
        if (app_ha_activity_context.sco_connected > 0) {
            audio_anc_psap_control_senario_notification(LLF_SCENARIO_CHANGE_UL_CALL, true);
        } else {
            audio_anc_psap_control_senario_notification(LLF_SCENARIO_CHANGE_UL_CALL, false);
        }

        app_hearing_aid_activity_update_sco_side_tone_status();
#ifdef EASTECH_SCO_DELAY_TO_PROCESS_HA  // harry for hfp pop noise 20240415
errrrrrrrrrrrr
            ui_shell_send_event(false,
                                EVENT_PRIORITY_MIDDLE,
                                EVENT_GROUP_UI_SHELL_HEARING_AID,
                                APP_HEARING_AID_EVENT_ID_REQUEST_SCO_CONTROL_HA,
                                (void *)need_aws_sync,
                                0,
                                NULL,
                                APP_HA_CONTROL_HEARING_AID_BY_SCO_TIMEOUT);

#else
        app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_SCO, false, false);
#endif
    }
}

/**
 * @brief Fix issue - 49332
 * Workaround, need disable side tone if SCO is ongoing and PSAP is on.
 */
static void app_hearing_aid_activity_update_sco_side_tone_status()
{
    bool ready_to_enable_side_tone = app_hearing_aid_is_ready_to_enable_side_tone();

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_update_sco_side_tone_status] ready_to_enable_side_tone : %d, sco_connected : %d",
                        2,
                        ready_to_enable_side_tone,
                        app_ha_activity_context.sco_connected);

    bt_sink_srv_call_sidetone_config_change_notify(ready_to_enable_side_tone);

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
        bt_ull_client_sidetone_switch_t ull_sidetone = {
            .sidetone_enable = ready_to_enable_side_tone,
        };
        bt_ull_action(BT_ULL_ACTION_SET_CLIENT_SIDETONE_SWITCH, &ull_sidetone, sizeof(ull_sidetone));
#endif /* AIR_BT_ULTRA_LOW_LATENCY_ENABLE || AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE */
}

static void app_hearing_aid_activity_handle_utils_notify_notification(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len)
{
    uint16_t which_msg = type & 0x0000FFFF;

#ifdef AIR_TWS_ENABLE
    bt_aws_mce_role_t aws_role = bt_device_manager_aws_local_info_get_role();

    if (app_hearing_aid_aws_is_connected() == false) {
        app_hear_through_race_cmd_send_notification(which_msg, notify_data, notify_data_len);
    } else {
        if (aws_role == BT_AWS_MCE_ROLE_AGENT) {
            uint8_t combine_notify_data[APP_HEARING_AID_NOTIFY_MAX_LEN] = {0};
            uint16_t combine_notify_data_len = 0;
            bool ret_value = app_hearing_aid_utils_handle_notify(role,
                                                                    which_msg,
                                                                    notify_data,
                                                                    notify_data_len,
                                                                    combine_notify_data,
                                                                    &combine_notify_data_len);

            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_utils_notify_notification] Notify result : %d, length : %d",
                                2,
                                ret_value,
                                combine_notify_data_len);

            if ((ret_value == true) && (combine_notify_data_len > 0)) {
                app_hear_through_race_cmd_send_notification(which_msg, combine_notify_data, combine_notify_data_len);
            }
        } else if (aws_role == BT_AWS_MCE_ROLE_PARTNER) {
            if (which_msg == APP_HEARING_AID_CONFIG_TYPE_FEEDBACK_DETECTION) {
                if (app_feedback_detection_directly == true) {
                    app_hear_through_race_cmd_send_notification(which_msg, notify_data, notify_data_len);
                    return;
                }
                app_feedback_detection_directly = false;
            }
            if (app_hearing_aid_config_get_notify_sync(which_msg) == true) {
                app_hearing_aid_aws_send_notification(role,
                                                        type,
                                                        notify_data,
                                                        notify_data_len);
            }
        }
    }
#else
    app_hear_through_race_cmd_send_notification(which_msg, notify_data, notify_data_len);
#endif /* AIR_TWS_ENABLE */
}

static void app_hearing_aid_activity_handle_utils_notify_to_control_ha(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len)
{
    if (app_hearing_aid_utils_is_ha_user_switch_on() == false) {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_utils_notify_to_control_ha] HA user switch is OFF", 0);
        return;
    }

    bool need_aws_sync = false;
    ui_shell_send_event(false,
                        EVENT_PRIORITY_MIDDLE,
                        EVENT_GROUP_UI_SHELL_HEARING_AID,
                        APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA,
                        (void *)need_aws_sync,
                        0,
                        NULL,
                        0);
}

static void app_hearing_aid_activity_handle_utils_notify_to_update_mode_index(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len)
{
    uint8_t *mode_index = (uint8_t *)pvPortMalloc(sizeof(uint8_t) * notify_data_len);
    if (mode_index == NULL) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_utils_notify_to_update_mode_index] Failed to allocate buffer for mode index", 0);
        return;
    }

    memset(mode_index, 0, sizeof(uint8_t) * notify_data_len);
    memcpy(mode_index, notify_data, notify_data_len);

    ui_shell_send_event(false,
                        EVENT_PRIORITY_MIDDLE,
                        EVENT_GROUP_UI_SHELL_HEARING_AID,
                        APP_HEARING_AID_EVENT_ID_MODIFY_MODE_INDEX,
                        mode_index,
                        notify_data_len,
                        NULL,
                        0);
}

static void app_hearing_aid_activity_handle_utils_notify_to_play_aea_off_vp(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len)
{
    ui_shell_send_event(false,
                        EVENT_PRIORITY_MIDDLE,
                        EVENT_GROUP_UI_SHELL_HEARING_AID,
                        APP_HEARING_AID_EVENT_ID_AEA_OFF_VP,
                        NULL,
                        0,
                        NULL,
                        0);
}

static void app_hearing_aid_activity_handle_utils_notify_change_in_ear_detection(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len)
{
    if (app_hearing_aid_utils_is_ha_user_switch_on() == false) {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_utils_notify_change_in_ear_detection] HA user switch is OFF", 0);
        return;
    }

#ifdef MTK_IN_EAR_FEATURE_ENABLE
    bool is_on = app_in_ear_get_own_state();
    app_hearing_aid_activity_proc_app_interaction(APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT,
                                                    (void *)(&is_on),
                                                    sizeof(bool));
#endif /* MTK_IN_EAR_FEATURE_ENABLE */
}

static void app_hearing_aid_activity_handle_utils_notify_to_enter_mp_test_mode(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len)
{
    if (app_hearing_aid_utils_is_ha_user_switch_on() == false) {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_utils_notify_to_enter_mp_test_mode] HA user switch is OFF", 0);
        return;
    }

    app_ha_activity_context.enter_mp_test_mode = true;

    app_ha_activity_context.is_anc_suspended_before_enter_mp_test_mode = app_anc_service_is_suspended();
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_utils_notify_to_enter_mp_test_mode] ENTER MP TEST MODE, anc_suspended : %d, ha_state : %d, open_fwk_done : %d, opening_fwk : %d",
                        4,
                        app_ha_activity_context.is_anc_suspended_before_enter_mp_test_mode,
                        app_ha_activity_context.ha_state,
                        app_ha_activity_context.is_open_fwk_done,
                        app_ha_activity_context.is_opening_fwk);

    app_ha_activity_context.ha_state_before_enter_mp_test_mode = app_ha_activity_context.ha_state;

    if (app_ha_activity_context.is_anc_suspended_before_enter_mp_test_mode == true) {
        app_anc_service_resume();
    }

    app_hearing_aid_activity_open_hearing_aid_fwk_with_zero_path();
}

static void app_hearing_aid_activity_handle_utils_notify_to_exit_mp_test_mode(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len)
{
    if (app_hearing_aid_utils_is_ha_user_switch_on() == false) {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_utils_notify_to_exit_mp_test_mode] HA user switch is OFF", 0);
        return;
    }

    app_ha_activity_context.enter_mp_test_mode = false;
    bool is_out_case = app_hearing_aid_activity_is_out_case();
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_utils_notify_to_exit_mp_test_mode] exit MP test mode, out_case : %d, ha_state_before_mp_mode : %d, anc_state_before_mp_mode : %d, opening_fwk : %d, open_fwk_done : %d",
                                5,
                                is_out_case,
                                app_ha_activity_context.ha_state_before_enter_mp_test_mode,
                                app_ha_activity_context.is_anc_suspended_before_enter_mp_test_mode,
                                app_ha_activity_context.is_opening_fwk,
                                app_ha_activity_context.is_open_fwk_done);

    app_ha_activity_context.anc_path_mask_enable = true;

    switch (app_ha_activity_context.ha_state_before_enter_mp_test_mode) {
        case APP_HA_STATE_DISABLED: {
            app_hearing_aid_activity_disable_hearing_aid(false);
        }
        break;

        case APP_HA_STATE_FWK_OPEN: {
            app_hearing_aid_utils_enable_mp_test_mode(false);
            app_hearing_aid_utils_control_ha(false);

            /**
             * @brief Reset the ANC path mask
             */
            app_hearing_aid_activity_open_hearing_aid_fwk();
        }
        break;

        case APP_HA_STATE_ENABLE: {
            app_hearing_aid_utils_enable_mp_test_mode(false);

            /**
             * @brief If HA is enabled before MP test mode,
             * need disable HA firstly, then open FWk with configured ANC path mask.
             */
            app_hearing_aid_utils_control_ha(false);

            app_ha_activity_context.anc_path_mask_enable = true;
            app_ha_activity_context.is_open_fwk_done = false;

            /**
             * @brief Reset the ANC path mask
             */
            if (app_ha_activity_context.is_opening_fwk == false) {
                app_hearing_aid_activity_open_hearing_aid_fwk();
            } else {
                app_ha_activity_context.need_re_open_fwk = true;
            }
        }
        break;
    }

    if (app_ha_activity_context.is_anc_suspended_before_enter_mp_test_mode == true) {
        app_anc_service_suspend();
    }
}

static void app_hearing_aid_activity_handle_utils_notify_to_control_fwk(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len)
{
    if (app_hearing_aid_utils_is_ha_user_switch_on() == false) {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_utils_notify_to_control_fwk] HA user switch is OFF", 0);
        return;
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_utils_notify_to_control_fwk] opening_fwk : %d",
                        1,
                        app_ha_activity_context.is_opening_fwk);

    app_ha_activity_context.ha_open_caused_by_which_reason = APP_HEARING_AID_CHANGE_CAUSE_MASTER_MIC_CHANNEL_SWITCH;
    app_ha_activity_context.is_need_disable_anc = true;
    app_ha_activity_context.is_anc_disable_done = false;

    if (app_ha_activity_context.is_opening_fwk == false) {
        // app_ha_activity_context.is_open_fwk_done = false;
        // app_hearing_aid_activity_open_hearing_aid_fwk();
        app_anc_service_disable_without_notify_hear_through();
    } else {
        // app_ha_activity_context.need_re_open_fwk = true;
    }
}

typedef void (*utils_notify_handler)(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len);

static utils_notify_handler app_ha_activity_utils_notify_handlers[] = {
    NULL,
    app_hearing_aid_activity_handle_utils_notify_notification,              // 0x01
    app_hearing_aid_activity_handle_utils_notify_to_control_ha,             // 0x02
    app_hearing_aid_activity_handle_utils_notify_to_update_mode_index,      // 0x03
    app_hearing_aid_activity_handle_utils_notify_to_play_aea_off_vp,        // 0x04
    app_hearing_aid_activity_handle_utils_notify_change_in_ear_detection,   // 0x05
    app_hearing_aid_activity_handle_utils_notify_to_enter_mp_test_mode,     // 0x06
    app_hearing_aid_activity_handle_utils_notify_to_exit_mp_test_mode,      // 0x07
    app_hearing_aid_activity_handle_utils_notify_to_control_fwk,            // 0x08
};

void app_hearing_aid_activity_ha_utils_notify_handler(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len)
{
    uint16_t notify_event = (type >> 16) & 0x0000FFFF;
    uint16_t which_msg = type & 0x0000FFFF;

#ifdef AIR_TWS_ENABLE
    bt_aws_mce_role_t aws_role = bt_device_manager_aws_local_info_get_role();
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_ha_utils_notify_handler] Aws Role : 0x%02x, role : %d, event : 0x%04x, msg : %s, data : 0x%x, data_len : %d",
                        6,
                        aws_role,
                        role,
                        notify_event,
                        app_hearing_aid_type_string[which_msg],
                        notify_data,
                        notify_data_len);

#else
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_ha_utils_notify_handler] Role : %d, event : 0x%04x, msg : %s, data : 0x%x, data_len : %d",
                        5,
                        role,
                        notify_event,
                        app_hearing_aid_type_string[which_msg],
                        notify_data,
                        notify_data_len);
#endif /* AIR_TWS_ENABLE */

    if (app_ha_activity_utils_notify_handlers[notify_event] != NULL) {
        app_ha_activity_utils_notify_handlers[notify_event](role, type, notify_data, notify_data_len);
    }
}

/*===================================================================================*/
/*                     HA INTERNAL FUNCTION IMPLEMENTATION                           */
/*===================================================================================*/
static void app_hearing_aid_activity_operate_dvfs(bool lock)
{
#ifdef AIR_BTA_IC_STEREO_HIGH_G3
#ifdef HAL_DVFS_MODULE_ENABLED
    hal_dvfs_status_t dvfs_status = HAL_DVFS_STATUS_OK;
    bool pre_dvfs_locked = app_ha_activity_context.is_dvfs_locked;

    if (lock == true) {
        if (app_ha_activity_context.is_dvfs_locked == false) {
            dvfs_status = hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
            if (dvfs_status == HAL_DVFS_STATUS_OK) {
                app_ha_activity_context.is_dvfs_locked = true;
            }
        }
    } else {
        if (app_ha_activity_context.is_dvfs_locked == true) {
            dvfs_status = hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_UNLOCK);
            if (dvfs_status == HAL_DVFS_STATUS_OK) {
                app_ha_activity_context.is_dvfs_locked = false;
            }
        }
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_operate_dvfs] lock : %d, dvfs_locked : %d -> %d",
                        3,
                        lock,
                        pre_dvfs_locked,
                        app_ha_activity_context.is_dvfs_locked);
#endif
#endif /* AIR_BTA_IC_STEREO_HIGH_G3 */
}

static void app_hearing_aid_activity_remove_ha_event()
{
    /**
     * @brief remove hearing AID event.
     */
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA);
    // ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_VP_STREAMING_BEGIN);
    // ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_VP_STREAMING_END);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_AEA_OFF_VP);
    // ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_RSSI_POWER_OFF);
}

static void app_hearing_aid_activity_initialization()
{
    if (app_ha_activity_context.inited == true) {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_initialization] Already inited", 0);
        return;
    }

#ifdef AIR_HEARING_AID_ENABLE
    /**
     * @brief Read the chip information.
     * TODO maybe need modify the judgement point.
     */
    uint32_t chip_info = sub_chip_version_get();
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_initialization] chip_info : 0x%02x", 1, chip_info);
    if ((chip_info != 0x0A) && (chip_info != 0x0D)) {
        assert(false && "Not supported chip");
    }
#endif /* AIR_HEARING_AID_ENABLE */

    memset(&app_ha_activity_context, 0, sizeof(app_hearing_aid_activity_context_t));
    app_ha_activity_context.inited = true;

    app_ha_activity_context.anc_path_mask_enable = true;
    app_ha_activity_context.is_need_play_locally = true;

    app_ha_activity_context.is_charger_in = true;//!is_out_case;

    /**
     * @brief Set the ha_state_before_mp_test_mode to be enabled, then when initialization
     * to make sure that HA should be enabled when power on.
     */
    app_ha_activity_context.ha_state_before_enter_mp_test_mode = APP_HA_STATE_ENABLE;

    app_hearing_aid_storage_load();

#ifdef MTK_IN_EAR_FEATURE_ENABLE
    app_ha_activity_context.is_in_ear = app_in_ear_get_own_state();
    app_ha_activity_context.elder_is_in_ear_switch = app_hearing_aid_storage_get_in_ear_detection_switch();
#endif /* MTK_IN_EAR_FEATURE_ENABLE */

    app_hearing_aid_utils_init(app_hearing_aid_activity_ha_utils_notify_handler);

    app_hearing_aid_aws_init();
}

static void app_hearing_aid_activity_de_initialization()
{
    if (app_ha_activity_context.inited == false) {
        return;
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_de_initialization] Enter", 0);

    app_hearing_aid_activity_disable_hearing_aid(false);

    app_hearing_aid_utils_deinit();

    app_hearing_aid_aws_deinit();

    bool is_charger_in = app_ha_activity_context.is_charger_in;

    memset(&app_ha_activity_context, 0, sizeof(app_hearing_aid_activity_context_t));

    app_ha_activity_context.is_charger_in = is_charger_in;

    app_hearing_aid_activity_remove_ha_event();
    app_hearing_aid_aws_remove_rssi_reading_event();
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_RSSI_POWER_OFF);
}

/**
 * @brief Process the system event.
 *
 * @param event_id
 * @param extra_data
 * @param data_len
 * @return true
 * @return false
 */
static bool app_hearing_aid_activity_proc_sys_event(uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    if (event_id == EVENT_ID_SHELL_SYSTEM_ON_CREATE) {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_sys_event] Activity Create", 0);

        app_hearing_aid_activity_initialization();

    } else if (event_id == EVENT_ID_SHELL_SYSTEM_ON_DESTROY) {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_sys_event] Activity Destroy", 0);
        app_hearing_aid_activity_de_initialization();
    }

    return true;
}

/**
 * @brief Process the key event
 *
 * @param event_id
 * @param extra_data
 * @param data_len
 * @return true
 * @return false
 */
 #if 0
static bool app_hearing_aid_activity_proc_key_event(uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    uint16_t key_id = *(uint16_t *)extra_data;
    uint8_t level_max_count = 0;
    uint8_t mode_max_count = 0;
    uint8_t vol_max_count = 0;
    uint8_t mode_index = 0;

    if ((key_id < KEY_HEARING_AID_BEGIN) || (key_id > KEY_HEARING_AID_END)) {
        return false;
    }

    audio_psap_status_t mode_index_status = audio_anc_psap_control_get_mode_index(&mode_index);
    audio_psap_status_t mode_max_count_status = audio_anc_psap_control_get_level_mode_max_count(&level_max_count, &mode_max_count, &vol_max_count);

    APPS_LOG_MSGID_I("app_hearing_aid_activity_proc_key_event  current mode index:%d, mode max:%d,anc_key_count=%d,vol_max_count=%d",
                     4,
                     mode_index,
                     mode_max_count,
                     anc_key_count,
                     vol_max_count
                     );


    
        if((mode_index+1)==mode_max_count&&anc_key_count==0)
        {
          APPS_LOG_MSGID_I("app_hearing_aid_activity_proc_key_event  set user_switch=1",0);
          app_hearing_aid_activity_set_user_switch(true, true);
        }


    bool user_switch = app_hearing_aid_utils_is_ha_user_switch_on();
    if ((user_switch == false) || (app_ha_activity_context.inited == false)) {
#if APP_HEARING_AID_DEBUG_ENABLE
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_key_event] Not ready to process key event, %d - %d",
                            2,
                            app_ha_activity_context.inited,
                            user_switch);
#endif /* APP_HEARING_AID_DEBUG_ENABLE */
        return false;
    }

    /**
     * @brief Handle the key event locally.
     */
    return app_hearing_aid_key_handler_processing(key_id);
}
#else
static bool app_hearing_aid_activity_proc_key_event(uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    uint16_t key_id = *(uint16_t *)extra_data;

    if ((key_id < KEY_HEARING_AID_BEGIN) || (key_id > KEY_HEARING_AID_END)) {
        return false;
    }

    bool user_switch = app_hearing_aid_utils_is_ha_user_switch_on();
    if ((user_switch == false) || (app_ha_activity_context.inited == false)) {
#if APP_HEARING_AID_DEBUG_ENABLE
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_key_event] Not ready to process key event, %d - %d",
                            2,
                            app_ha_activity_context.inited,
                            user_switch);
#endif /* APP_HEARING_AID_DEBUG_ENABLE */
        return false;
    }

    /**
     * @brief Handle the key event locally.
     */
    return app_hearing_aid_key_handler_processing(key_id);
}
#endif

/**
 * @brief Process BT Sink Service event
 * Check A2DP/SCO streaming or not to determine need start HA or not.
 *
 * @param event_id
 * @param extra_data
 * @param data_len
 * @return true
 * @return false
 */
static bool app_hearing_aid_activity_proc_bt_sink_event(uint32_t event_id,
                                                        void *extra_data,
                                                        size_t data_len)
{
    if (event_id == BT_SINK_SRV_EVENT_STATE_CHANGE) {
        bt_sink_srv_event_param_t *event = (bt_sink_srv_event_param_t *)extra_data;
        if (event != NULL) {
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_bt_sink_event] current state : 0x%04x, pre state : 0x%04x",
                                2,
                                event->state_change.current,
                                event->state_change.previous);
            bool a2dp_changed = false;

            if (((event->state_change.current == BT_SINK_SRV_STATE_STREAMING) && (event->state_change.previous != BT_SINK_SRV_STATE_STREAMING))
                || ((event->state_change.current != BT_SINK_SRV_STATE_STREAMING) && (event->state_change.previous == BT_SINK_SRV_STATE_STREAMING))) {
                a2dp_changed = true;
            }

            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_bt_sink_event] a2dp_changed : %d",
                                1,
                                a2dp_changed);

            if (a2dp_changed == true) {
                if ((event->state_change.current == BT_SINK_SRV_STATE_STREAMING) && (event->state_change.previous != BT_SINK_SRV_STATE_STREAMING)) {
                    app_hearing_aid_activity_handle_music_status_change(true);
                } else {
                    app_hearing_aid_activity_handle_music_status_change(false);
                }
            }
        }
    }

    if ((event_id == BT_SINK_SRV_EVENT_HF_SCO_STATE_UPDATE)
            || (event_id == BT_SINK_SRV_EVENT_LE_BIDIRECTION_LEA_UPDATE)) {

        if (extra_data == NULL) {
            return false;
        }

        bool sco_start = false;

        if (event_id == BT_SINK_SRV_EVENT_HF_SCO_STATE_UPDATE) {
            bt_sink_srv_sco_state_update_t *sco_state = (bt_sink_srv_sco_state_update_t *)extra_data;
            if (sco_state->state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED) {
                sco_start = true;
            }
        } else {
            bt_sink_srv_bidirection_lea_state_update_t *lea_state = (bt_sink_srv_bidirection_lea_state_update_t *)extra_data;
            if (lea_state->state == BT_SINK_SRV_BIDIRECTION_LEA_STATE_ENABLE) {
                sco_start = true;
            }
        }

        app_hearing_aid_activity_handle_sco_status_change(sco_start);
    }

    return false;
}

/**
 * @brief
 *
 * @param event_id
 * @param extra_data
 * @param data_len
 * @return true
 * @return false
 */
static bool app_hearing_aid_activity_proc_app_interaction(uint32_t event_id,
                                                          void *extra_data,
                                                          size_t data_len)
{
    bool user_switch = app_hearing_aid_utils_is_ha_user_switch_on();

#ifdef MTK_IN_EAR_FEATURE_ENABLE
    if (event_id == APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT) {
        bool on = app_hearing_aid_storage_get_in_ear_detection_switch();
        uint32_t timeout = app_hear_through_storage_get_ha_in_ear_det_turn_on_delay_time();
        bool is_in_ear = ((uint8_t *)extra_data)[0];
        bool anc_suspended = app_anc_service_is_suspended();

        uint8_t caused_by_which = APP_HEARING_AID_CHANGE_CAUSE_RACE_CMD;

        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_app_interaction] in_ear_detection_switch : %d -> %d, is_in_ear state : %d -> %d, anc_suspended : %d, timeout : %d",
                            6,
                            app_ha_activity_context.elder_is_in_ear_switch,
                            on,
                            app_ha_activity_context.is_in_ear,
                            is_in_ear,
                            anc_suspended,
                            timeout);

        app_ha_activity_context.is_in_ear = is_in_ear;
      APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_app_interaction] user switch=%d,inited=%d",2,user_switch , app_ha_activity_context.inited);
        if ((user_switch == false) || (app_ha_activity_context.inited == false)) {
		if(is_in_ear==true)	{
		if(anc_suspended==true)
		{
      			app_anc_service_resume();
		}

		}
		else
		{
			//app_anc_service_suspend();
		}
		
            return false;
        }

        /**
         * @brief If the in ear detection switch is changed, need operate the HA by race_cmd to play VP
         * if the switch is not changed, but in_ear state changed, need operate HA by in_ear, then will not play VP.
         */
        if (on == app_ha_activity_context.elder_is_in_ear_switch) {
            caused_by_which = APP_HEARING_AID_CHANGE_CAUSE_IN_EAR;
        }
        app_ha_activity_context.elder_is_in_ear_switch = on;

        if (on == false) {
            if (anc_suspended == true) {
                app_anc_service_resume();
            }
            app_hearing_aid_activity_pre_proc_operate_ha(caused_by_which, true, false);
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_app_interaction] on == false return flase",0);
            return false;
        }

        app_ha_activity_context.ha_open_caused_by_which_reason = caused_by_which;

        if (is_in_ear == true) {
            /**
             * @brief Fix issue - 45772, send resume ANC event to resume ANC.
             */
            ui_shell_send_event(false,
                                EVENT_PRIORITY_MIDDLE,
                                EVENT_GROUP_UI_SHELL_HEARING_AID,
                                APP_HEARING_AID_EVENT_ID_REQUEST_TO_RESUME_ANC,
                                NULL,
                                0,
                                NULL,
                                timeout);
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_app_interaction] sent APP_HEARING_AID_EVENT_ID_REQUEST_TO_RESUME_ANC",0);
        } else {
            /**
             * @brief Fix issue - 45772, do not disable HA, but suspend ANC.
             */
            app_anc_service_suspend();
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_app_interaction]  action app_anc_service_suspend",0);
            // app_hearing_aid_activity_disable_hearing_aid(false);
        }
    }
#endif /* MTK_IN_EAR_FEATURE_ENABLE */

#ifdef AIR_TWS_ENABLE
    if ((event_id == APPS_EVENTS_INTERACTION_RHO_END)
        || (event_id == APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT)) {

        if ((user_switch == false) || (app_ha_activity_context.inited == false)) {
            return false;
        }

        if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
            app_hearing_aid_aws_send_rssi_reading_event();
        } else if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
            app_hearing_aid_aws_remove_rssi_reading_event();
        }
    }
#endif /* AIR_TWS_ENABLE */

    return false;
}

#ifdef AIR_LE_AUDIO_BIS_ENABLE
static bool app_hearing_aid_activity_proc_le_audio_event(uint32_t event_id,
                                                            void *extra_data,
                                                            size_t data_len)
{

    if ((event_id == EVENT_ID_LE_AUDIO_BIS_START_STREAMING)
        || (event_id == EVENT_ID_LE_AUDIO_BIS_STOP_STREAMING)) {

        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA);
        bool need_aws_sync = true;

        if (event_id == EVENT_ID_LE_AUDIO_BIS_START_STREAMING) {
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_le_audio_event] BIS Start streaming", 0);

            ui_shell_send_event(false,
                                EVENT_PRIORITY_MIDDLE,
                                EVENT_GROUP_UI_SHELL_HEARING_AID,
                                APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA,
                                (void *)need_aws_sync,
                                0,
                                NULL,
                                APP_HA_CONTROL_HEARING_AID_BY_A2DP_TIMEOUT);
        } else  {
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_le_audio_event] BIS Stop streaming", 0);
            ui_shell_send_event(false,
                                EVENT_PRIORITY_MIDDLE,
                                EVENT_GROUP_UI_SHELL_HEARING_AID,
                                APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA,
                                (void *)need_aws_sync,
                                0,
                                NULL,
                                0);
        }
    }

    return false;
}
#endif /* AIR_LE_AUDIO_BIS_ENABLE */

/*===================================================================================*/
/*                          HA EVENT HANDLING                                        */
/*===================================================================================*/

static void app_hearing_aid_activity_handle_request_to_control_ha(void *data, size_t data_len)
{
    bool need_aws_sync = (bool)data;

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_request_to_control_ha] Request to control HA, need_aws_sync : %d",
                        1,
                        need_aws_sync);

    app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_REQUEST, false, need_aws_sync);
}


#ifdef EASTECH_SCO_DELAY_TO_PROCESS_HA
static void app_hearing_aid_activity_handle_request_sco_control_ha(void *data, size_t data_len)
{
    bool need_aws_sync = (bool)data;

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_request_sco_control_ha] Request to control HA, need_aws_sync : %d",
                        1,
                        need_aws_sync);

    app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_SCO, false, false);
}
#endif
#if 0
static void app_hearing_aid_activity_handle_vp_streaming_begin(void *data, size_t data_len)
{
    app_hearing_aid_activity_proc_vp_streaming_state_change(true);
}

static void app_hearing_aid_activity_handle_vp_streaming_end(void *data, size_t data_len)
{
    app_hearing_aid_activity_proc_vp_streaming_state_change(false);
}
#endif

static void app_hearing_aid_activity_handle_rssi_reading(void *data, size_t data_len)
{
#ifdef AIR_TWS_ENABLE
    bool user_switch = app_hearing_aid_utils_is_ha_user_switch_on();
    if ((user_switch == false) || (app_ha_activity_context.inited == false)) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_rssi_reading] Not ready to process RSSI event, %d - %d",
                            2,
                            app_ha_activity_context.inited,
                            user_switch);
        return;
    }

    if (app_hearing_aid_aws_is_connected() == false) {
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_rssi_reading] AWS disconnected state, NO need to handle RSSI event", 0);
        return;
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_rssi_reading] Start to read RSSI", 0);

    bt_bd_addr_t *local_addr = bt_device_manager_aws_local_info_get_local_address();//bt_device_manager_aws_local_info_get_peer_address();
    bt_bd_addr_t gap_addr = {0};
    memcpy(&gap_addr, local_addr, BT_BD_ADDR_LEN);
    bt_gap_connection_handle_t handle = bt_cm_get_gap_handle(gap_addr);
    bt_status_t status = bt_gap_read_raw_rssi(handle);
    if (status != BT_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_rssi_reading] Failed to get RAW RSSI, 0x%04x",
                            1,
                            status);
    }

    app_hearing_aid_aws_send_rssi_reading_event();
#endif /* AIR_TWS_ENABLE */
}

static void app_hearing_aid_activity_handle_mode_index_modification_req(void *data, size_t data_len)
{
    if ((data == NULL) || (data_len != sizeof(uint8_t))) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_mode_index_modification_req] Data error, 0x%04x, %d",
                            2,
                            data,
                            data_len);
        return;
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_mode_index_modification_req] Modify mode index to : %d",
                        1,
                        ((uint8_t *)data)[0]);

    bool ret = false;

#ifdef AIR_TWS_ENABLE
    bool is_aws_connected = app_hearing_aid_aws_is_connected();
    if (is_aws_connected == true) {
        ret = app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_MODIFY_MODE_INDEX_REQ,
                                                        (uint8_t *)data,
                                                        data_len,
                                                        true,
                                                        APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT);
    } else {
#endif /* AIR_TWS_ENABLE */
        ret = app_hearing_aid_utils_set_mode_index((uint8_t *)data);
        /**
         * @brief Fix issue - 47207
         * Send the mode index notification when set succeed.
         */
        if (ret == true) {
            app_hear_through_race_cmd_send_notification(APP_HEARING_AID_CONFIG_TYPE_MODE_INDEX, data, data_len);
        }
#ifdef AIR_TWS_ENABLE
    }
#endif /* AIR_TWS_ENABLE */
}

static void app_hearing_aid_activity_handle_aea_off_vp(void *data, size_t data_len)
{
    bool user_switch = app_hearing_aid_utils_is_ha_user_switch_on();
    if ((user_switch == false) || (app_ha_activity_context.inited == false)) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_aea_off_vp] Not ready to process AEA OFF VP event, %d - %d",
                            2,
                            app_ha_activity_context.inited,
                            user_switch);
        return;
    }

    app_hearing_aid_activity_play_vp(VP_INDEX_DOORBELL, true);
}

static void app_hearing_aid_activity_handle_rssi_power_off(void *data, size_t data_len)
{
    bool user_switch = app_hearing_aid_utils_is_ha_user_switch_on();
    if ((user_switch == false) || (app_ha_activity_context.inited == false)) {
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_rssi_power_off] Not ready to process RSS power off event, %d - %d",
                            2,
                            app_ha_activity_context.inited,
                            user_switch);
        return;
    }

    int8_t config_rssi = app_hear_through_storage_get_ha_rssi_threshold();

    if (config_rssi == 0x00) {
        return;
    }

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_rssi_power_off] Handle RSSI power off, config_rssi : %d, partner_rssi : %d",
                        2,
                        config_rssi,
                        app_ha_activity_context.partner_rssi);

    if (app_ha_activity_context.partner_rssi > config_rssi) {

        app_ha_activity_context.is_powering_off = true;
        voice_prompt_play_vp_power_off(VOICE_PROMPT_CONTROL_MASK_NONE);

        ui_shell_send_event(false,
                            EVENT_PRIORITY_HIGHEST,
                            EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF,
                            NULL,
                            0,
                            NULL,
                            APP_HA_REQUEST_POWER_OFF_DELAY);
    }
}

static void app_hearing_aid_activity_handle_request_to_resume_anc(void *data, size_t data_len)
{
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_request_to_resume_anc] Request to resume ANC, ha_state : %d",
                        1,
                        app_ha_activity_context.ha_state);

    app_anc_service_resume();

    /**
     * @brief Fix issue - 46758
     * When out-of-ear happen, need check current status and confirm need enable HA or not.
     */
    app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_REQUEST, true, false);
}

/*===================================================================================*/
/*                              HANDLE SYNC EVENT                                    */
/*===================================================================================*/
typedef void (*ha_event_handler)(void *data, size_t data_len);

static ha_event_handler app_hearing_aid_ha_event_handler[] = {
    NULL,                                                           // 0x00
    NULL, // app_hearing_aid_activity_handle_race_connected,         // 0x01
    NULL, // app_hearing_aid_activity_handle_race_disconnected,      // 0x02
    NULL, // app_hearing_aid_activity_handle_advertising_timeout,    // 0x03
    app_hearing_aid_activity_handle_request_to_control_ha,           // 0x04
    NULL, // app_hearing_aid_activity_handle_vp_streaming_begin,             // 0x05
    NULL, // app_hearing_aid_activity_handle_vp_streaming_end,               // 0x06
    app_hearing_aid_activity_handle_rssi_reading,                   // 0x07
    app_hearing_aid_activity_handle_mode_index_modification_req,    // 0x08
    app_hearing_aid_activity_handle_aea_off_vp,                     // 0x09
    NULL, // app_hearing_aid_activity_handle_init_to_control_ha,    // 0x0A
    NULL, // app_hearing_aid_activity_handle_init_to_play_power_on_vp,       // 0x0B
    app_hearing_aid_activity_handle_rssi_power_off,                 // 0x0C
    app_hearing_aid_activity_handle_request_to_resume_anc,          // 0x0D
#ifdef EASTECH_SCO_DELAY_TO_PROCESS_HA
    app_hearing_aid_activity_handle_request_sco_control_ha,           // 0x0E
    #endif
};

/**
 * @brief Process HA event (internal)
 *
 * @param event_id
 * @param extra_data
 * @param data_len
 * @return true
 * @return false
 */
static bool app_hearing_aid_activity_proc_ha_event(uint32_t event_id, void *extra_data, size_t data_len)
{
    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_ha_event] event id : 0x%04x",
                        1,
                        event_id);

    if (event_id < sizeof(app_hearing_aid_ha_event_handler)) {
        if (app_hearing_aid_ha_event_handler[event_id] != NULL) {
            app_hearing_aid_ha_event_handler[event_id](extra_data, data_len);
        }
    }

#ifdef AIR_TWS_ENABLE
    if (event_id >= APP_HEARING_AID_EVENT_SYNC_BASE) {
        app_hearing_aid_aws_handle_sync_execute_event(event_id, extra_data, data_len);
    }
#endif /* AIR_TWS_ENABLE */

    return true;
}

/**
 * @brief Process connection manager event.
 * Handle AWS connected or disconnected.
 * When AWS connected, agent need sync the stored nvkey to the partner side.
 *
 * @param event_id
 * @param extra_data
 * @param data_len
 * @return true
 * @return false
 */
static bool app_hearing_aid_activity_proc_cm_event(uint32_t event_id,
                                                   void *extra_data,
                                                   size_t data_len)
{
     bool user_switch = app_hearing_aid_utils_is_ha_user_switch_on();
    if ((user_switch == false) || (app_ha_activity_context.inited == false)) {
#if APP_HEARING_AID_DEBUG_ENABLE
        APPS_LOG_MSGID_E(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_cm_event] Not ready to process CM event, %d - %d, event_id : 0x%04x",
                            3,
                            app_ha_activity_context.inited,
                            user_switch,
                            event_id);
#endif /* APP_HEARING_AID_DEBUG_ENABLE */
        return false;
    }

    if (event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE) {

        bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;

        if (remote_update->pre_connected_service == remote_update->connected_service) {
            return false;
        }

#ifdef AIR_TWS_ENABLE
        bt_aws_mce_role_t device_aws_role = bt_device_manager_aws_local_info_get_role();
        if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
            && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
            // AWS connected handler

            bool user_switch_on = app_hearing_aid_utils_is_ha_user_switch_on();

            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_cm_event] AWS Connected, Role : 0x%02x, power_on_executed : %d, user_switch_on : %d",
                                3,
                                device_aws_role,
                                app_ha_activity_context.power_on_ha_executed,
                                user_switch_on);

            if (device_aws_role == BT_AWS_MCE_ROLE_AGENT) {

                if (user_switch_on == true) {
                    app_hearing_aid_aws_handle_connected(app_ha_activity_context.power_on_ha_executed);
                }
                // harry for anc key // harry for anc+ha common key
  	            //apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_CUSTOMER_COMMON, EVENT_ID_ANC_KEY_SYNC,(void*)&anc_key_count ,1);

                /**
                 * @brief Fix issue - 44600
                 * When AWS connected, need sync app information to partner side.
                 * Then partner side need execute HA mix table flow.
                 *
                 * Fix issue - 50008
                 * No need to sync the sco info to partner.
                 */
#if 0
                app_hearing_aid_app_sync_info_t info = {0};
                info.sco_connected = app_ha_activity_context.sco_connected;
                app_hearing_aid_aws_sync_agent_app_info_to_partner((uint8_t *)&info, sizeof(app_hearing_aid_app_sync_info_t));
#endif
            }

        } else if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)
                    && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)) {
            // AWS disconnected handler
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_cm_event] AWS Disconnected", 0);
            app_hearing_aid_aws_handle_disconnected();
        }
#endif /* AIR_TWS_ENABLE */
    }

    return false;
}

static bool app_hearing_aid_activity_proc_dm_event(uint32_t event_id,
                                                   void *extra_data,
                                                   size_t data_len)
{
    bt_device_manager_power_event_t evt;
    bt_device_manager_power_status_t status;
    bt_event_get_bt_dm_event_and_status(event_id, &evt, &status);

    if (status == BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS) {
        if (evt == BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE) {
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_dm_event] BT Powering off", 0);

            app_hearing_aid_utils_save_user_settings();
            app_hearing_aid_storage_save_configuration();

            app_ha_activity_context.is_powering_off = true;
            app_ha_activity_context.need_re_open_fwk = false;
            app_ha_activity_context.is_open_fwk_done = false;
            app_ha_activity_context.is_opening_fwk = false;
            app_ha_activity_context.is_closing_fwk = false;

        } else if (evt == BT_DEVICE_MANAGER_POWER_EVT_ACTIVE_COMPLETE) {
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_dm_event] BT Powering ON", 0);

            app_ha_activity_context.is_powering_off = false;
        }
    }

    return false;
}


#ifdef AIR_TWS_ENABLE
/**
 * @brief Process the AWS data.
 *
 * @param event_id
 * @param extra_data
 * @param data_len
 * @return true
 * @return false
 */
static bool app_hearing_aid_activity_proc_aws_data(uint32_t event_id,
                                                   void *extra_data,
                                                   size_t data_len)
{
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;

    uint32_t extra_event_group;
    uint32_t extra_event_id;

    void *aws_extra_data = NULL;
    uint32_t aws_extra_data_len = 0;

    apps_aws_sync_event_decode_extra(aws_data_ind, &extra_event_group, &extra_event_id, &aws_extra_data, &aws_extra_data_len);

    if (extra_event_group == EVENT_GROUP_UI_SHELL_HEARING_AID) {
        app_hearing_aid_aws_process_data(extra_event_id, (uint8_t *)aws_extra_data, aws_extra_data_len);
        return true;
    } else if (extra_event_group == EVENT_GROUP_UI_SHELL_KEY) {
        return app_hearing_aid_key_handler_processing(extra_event_id);
    }

    return false;
}

#if 0
void app_hearing_aid_activity_handle_app_info_sync(uint8_t *data, uint32_t data_len)
{
    /**
     * @brief Fix issue - 44600
     * When AWS connected, need sync app information to partner side.
     * Then partner side need execute HA mix table flow.
     */
    app_hearing_aid_app_sync_info_t *info = (app_hearing_aid_app_sync_info_t *)data;

    APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_handle_app_info_sync] sco_connected : %d -> %d",
                    2,
                    app_ha_activity_context.sco_connected,
                    info->sco_connected);

    if (app_ha_activity_context.sco_connected != info->sco_connected) {
        app_ha_activity_context.sco_connected = info->sco_connected;

        app_hearing_aid_activity_pre_proc_operate_ha(APP_HEARING_AID_CHANGE_CAUSE_SCO, false, false);
    }
}
#endif

void app_hearing_aid_activity_set_powering_off()
{
    app_ha_activity_context.is_powering_off = true;
}

void app_hearing_aid_activity_set_fbd_directly()
{
    app_feedback_detection_directly = true;
}

#endif /* AIR_TWS_ENABLE */

#ifdef AIR_SMART_CHARGER_ENABLE
static bool app_hearing_aid_activity_proc_smart_charger_case_event(uint32_t event_id,
                                                                   void *extra_data,
                                                                   size_t data_len)
{
    if (event_id == SMCHARGER_EVENT_NOTIFY_ACTION) {
        app_smcharger_public_event_para_t *para = (app_smcharger_public_event_para_t *)extra_data;
        if (para->action == SMCHARGER_CHARGER_OUT_ACTION) {
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_smart_charger_case_event][SmartCharger] Charger OUT", 0);

            app_ha_activity_context.is_charger_in = false;
            app_ha_activity_context.power_on_ha_executed = false;
#ifdef MTK_IN_EAR_FEATURE_ENABLE
            app_ha_activity_context.is_in_ear = app_in_ear_get_own_state();
            app_ha_activity_context.elder_is_in_ear_switch = app_hearing_aid_storage_get_in_ear_detection_switch();
#endif /* MTK_IN_EAR_FEATURE_ENABLE */
            app_ha_activity_context.is_powering_off = false;
        }
        if (para->action == SMCHARGER_CHARGER_IN_ACTION) {
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_smart_charger_case_event][SmartCharger] Charger IN", 0);
            app_ha_activity_context.is_charger_in = true;

            app_hearing_aid_utils_save_user_settings();
            app_hearing_aid_storage_save_configuration();
        }
    }
    return false;
}

#else

#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
static bool app_hearing_aid_activity_proc_battery_event(uint32_t event_id,
                                                        void *extra_data,
                                                        size_t data_len)
{
    if (event_id == APPS_EVENTS_BATTERY_CHARGER_EXIST_CHANGE) {
        bool charger_in = (bool)extra_data;
        APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_battery_event], charger_exist change %d->%d, inited : %d",
                            3,
                            app_ha_activity_context.is_charger_in,
                            charger_in,
                            app_ha_activity_context.inited);

        if ((app_ha_activity_context.is_charger_in == false) && (charger_in == true)) {
            app_ha_activity_context.is_charger_in = true;
            // ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_INIT_TO_PLAY_POWER_ON_VP);

            // if (app_ha_activity_context.inited == true) {
            //     app_hearing_aid_activity_de_initialization();
            // }
            app_hearing_aid_utils_save_user_settings();
            app_hearing_aid_storage_save_configuration();
        }
        if ((app_ha_activity_context.is_charger_in == true) && (charger_in == false)) {
            app_ha_activity_context.is_charger_in = false;
            app_ha_activity_context.power_on_ha_executed = false;
            app_ha_activity_context.is_powering_off = false;
            // if (app_ha_activity_context.inited == false) {
            //     app_hearing_aid_activity_initialization();
            // }

#ifdef MTK_IN_EAR_FEATURE_ENABLE
            app_ha_activity_context.is_in_ear = app_in_ear_get_own_state();
            app_ha_activity_context.elder_is_in_ear_switch = app_hearing_aid_storage_get_in_ear_detection_switch();
#endif /* MTK_IN_EAR_FEATURE_ENABLE */
            /**
             * @brief When out of case, play power on VP with some delay to avoid HA initialization.
             */
            // ui_shell_send_event(false,
            //                     EVENT_PRIORITY_MIDDLE,
            //                     EVENT_GROUP_UI_SHELL_HEARING_AID,
            //                     APP_HEARING_AID_EVENT_ID_INIT_TO_PLAY_POWER_ON_VP,
            //                     NULL,
            //                     0,
            //                     NULL,
            //                     APP_HA_POWER_ON_VP_DELAY_TIME);
        }
    }
    return false;
}
#endif /* MTK_BATTERY_MANAGEMENT_ENABLE */
#endif /* AIR_SMART_CHARGER_ENABLE */

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
static bool app_hearing_aid_activity_proc_ultra_low_latency_event(uint32_t event_id,
                                                                    void *extra_data,
                                                                    size_t data_len)
{
    if ((event_id == BT_ULL_EVENT_LE_STREAMING_START_IND)
            || (event_id == BT_ULL_EVENT_LE_STREAMING_STOP_IND)) {

        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_REQUEST_TO_CONTROL_HA);
        bool need_aws_sync = true;

        bt_ull_le_streaming_start_ind_t *ind = (bt_ull_le_streaming_start_ind_t *)extra_data;

        if (event_id == BT_ULL_EVENT_LE_STREAMING_START_IND) {
            if (ind->stream_mode == BT_ULL_LE_STREAM_MODE_UPLINK) {
                APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_ultra_low_latency_event], ULL - UL Start Streaming", 0);
                app_hearing_aid_activity_handle_sco_status_change(true);
            } else {
                APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_ultra_low_latency_event], ULL - DL Start Streaming", 0);
                app_hearing_aid_activity_handle_music_status_change(true);
            }
        } else {
            if (ind->stream_mode == BT_ULL_LE_STREAM_MODE_UPLINK) {
                APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_ultra_low_latency_event], ULL - UL Stop Streaming", 0);
                app_hearing_aid_activity_handle_sco_status_change(false);
            } else {
                APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_ultra_low_latency_event], ULL - DL Stop Streaming", 0);
                app_hearing_aid_activity_handle_music_status_change(false);
            }
        }
    }

    return false;	// richard for compiling error
}
#endif /* AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE */

#ifdef MTK_ANC_ENABLE
static bool app_hearing_aid_activity_proc_anc_service_event(uint32_t event_id,
                                                            void *extra_data,
                                                            size_t data_len)
{
    if (event_id == AUDIO_ANC_CONTROL_EVENT_OFF) {
        /**
         * @brief Workaround - when MIC changed, need disable ANC and then enable HA FWK again
         * If ANC disable done, start HA FWK again.
         */
        if (app_ha_activity_context.is_need_disable_anc == true) {
            APPS_LOG_MSGID_I(APP_HA_ACTIVITY_TAG"[app_hearing_aid_activity_proc_anc_service_event] ANC disabled, is_open_fwk_done : %d",
                            1,
                            app_ha_activity_context.is_open_fwk_done);

            app_ha_activity_context.is_need_disable_anc = false;
            app_ha_activity_context.is_anc_disable_done = true;

            if (app_ha_activity_context.is_open_fwk_done == false) {
                app_hearing_aid_activity_open_hearing_aid_fwk();
            }
        }
    }

    return false;
}
#endif /* MTK_ANC_ENABLE */

typedef struct {
    uint32_t            event_group;
    bool (*handler)(uint32_t event_id, void *extra_data, size_t data_len);
} app_ha_activity_event_handler_t;

const app_ha_activity_event_handler_t app_ha_activity_event_handler[] = {
    {
        .event_group = EVENT_GROUP_UI_SHELL_SYSTEM,
        .handler = app_hearing_aid_activity_proc_sys_event,
    },
#ifdef AIR_TWS_ENABLE
    {
        .event_group = EVENT_GROUP_UI_SHELL_AWS_DATA,
        .handler = app_hearing_aid_activity_proc_aws_data,
    },
#endif /* AIR_TWS_ENABLE */
    {
        .event_group = EVENT_GROUP_UI_SHELL_HEARING_AID,
        .handler = app_hearing_aid_activity_proc_ha_event,
    },
    {
        .event_group = EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER,
        .handler = app_hearing_aid_activity_proc_cm_event,
    },
    {
        .event_group = EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER,
        .handler = app_hearing_aid_activity_proc_dm_event,
    },
    {
        .event_group = EVENT_GROUP_UI_SHELL_KEY,
        .handler = app_hearing_aid_activity_proc_key_event,
    },
    {
        .event_group = EVENT_GROUP_UI_SHELL_BT_SINK,
        .handler = app_hearing_aid_activity_proc_bt_sink_event,
    },
    {
        .event_group = EVENT_GROUP_UI_SHELL_APP_INTERACTION,
        .handler = app_hearing_aid_activity_proc_app_interaction,
    },
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    {
        .event_group = EVENT_GROUP_UI_SHELL_LE_AUDIO,
        .handler = app_hearing_aid_activity_proc_le_audio_event,
    },
#endif /* AIR_LE_AUDIO_BIS_ENABLE */
#ifdef AIR_SMART_CHARGER_ENABLE
    {
        .event_group = EVENT_GROUP_UI_SHELL_CHARGER_CASE,
        .handler = app_hearing_aid_activity_proc_smart_charger_case_event,
    },
#else
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
    {
        .event_group = EVENT_GROUP_UI_SHELL_BATTERY,
        .handler = app_hearing_aid_activity_proc_battery_event,
    },
#endif /* MTK_BATTERY_MANAGEMENT_ENABLE */
#endif /* AIR_SMART_CHARGER_ENABLE */
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
    {
        .event_group = EVENT_GROUP_BT_ULTRA_LOW_LATENCY,
        .handler = app_hearing_aid_activity_proc_ultra_low_latency_event,
    },
#endif /* AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE */
#ifdef MTK_ANC_ENABLE
    {
        .event_group = EVENT_GROUP_UI_SHELL_AUDIO_ANC,
        .handler = app_hearing_aid_activity_proc_anc_service_event,
    },
#endif /* MTK_ANC_ENABLE */
};

#define app_ha_activity_event_handler_COUNT      (sizeof(app_ha_activity_event_handler) / sizeof(app_ha_activity_event_handler_t))

bool app_hearing_aid_activity_proc(ui_shell_activity_t *self,
                                   uint32_t event_group,
                                   uint32_t event_id,
                                   void *extra_data,
                                   size_t data_len)
{
    uint8_t event_handler_index = 0;

    for (event_handler_index = 0; event_handler_index < app_ha_activity_event_handler_COUNT; event_handler_index++) {
        if ((app_ha_activity_event_handler[event_handler_index].event_group == event_group)
            && (app_ha_activity_event_handler[event_handler_index].handler != NULL)) {
            return app_ha_activity_event_handler[event_handler_index].handler(event_id, extra_data, data_len);
        }
    }
    return false;
}


#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */


