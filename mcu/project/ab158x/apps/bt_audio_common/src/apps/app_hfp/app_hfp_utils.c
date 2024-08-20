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
 * File: app_hfp_activity.c
 *
 * Description: this file provide common functions for hfp_app.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */


#include "app_hfp_utils.h"
#include "app_hfp_activity.h"
#include "app_hfp_idle_activity.h"
#include "apps_config_key_remapper.h"
#include "voice_prompt_api.h"
#include "apps_config_vp_index_list.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_events_key_event.h"
#include "apps_aws_sync_event.h"
#include "bt_device_manager.h"
#include "bt_sink_srv_ami.h"
#include "bt_aws_mce_srv.h"
#include "app_home_screen_idle_activity.h"
#include "app_rho_idle_activity.h"
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#include "battery_management_core.h"
#endif
#include "nvkey_id_list.h"
#include "nvkey.h"
#include "bt_sink_srv_hf.h"

#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_utils.h"
#endif
#include "bt_hci.h"
#include "app_bt_state_service.h"

#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_le_audio_util.h"
#include "app_le_audio_aird_client.h"
#include "bt_sink_srv_le_volume.h"
#include "bt_sink_srv_le_cap.h"
#include "bt_device_manager_le.h"
#endif

#if defined(AIR_MCSYNC_SHARE_ENABLE) && defined(MTK_AWS_MCE_ENABLE)
#include "app_share_idle_activity.h"
#endif
#include "bt_sink_srv_state_manager.h"
#include "bt_device_manager_link_record.h"

#ifdef AIR_MS_TEAMS_ENABLE
#include "app_ms_teams_telemetry.h"
#endif
#include "app_smcharger_utils.h"

#ifdef MTK_IN_EAR_FEATURE_ENABLE
uint8_t g_app_hfp_auto_accept = APP_HFP_AUTO_ACCEPT_NONE;  /**<  Record the hfp auto accept incoming call config. */
#endif
//app_hfp_incoming_call_vp_status_t  g_app_hfp_incoming_call_vp_status;
bool app_hfp_get_active_device_addr(bt_bd_addr_t *active_addr)
{
    if (NULL == active_addr) {
        return false;
    }
    bt_sink_srv_state_manager_played_device_t list = {0};
    uint32_t list_num = 0;
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    list_num = bt_sink_srv_state_manager_get_played_device_list(&list, 1);
#endif
    const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();
    if (link_info == NULL || (link_info != NULL && link_info->connected_num == 0)) {
        return false;
    }
    if (list_num == 0) {
        /* Use the last of address. */
        memcpy((*active_addr), link_info->connected_device[0].remote_addr, sizeof(bt_bd_addr_t));
    } else if (list_num == 1) {
        memcpy((*active_addr), list.address, sizeof(bt_bd_addr_t));
    }
#ifdef AIR_LE_AUDIO_ENABLE
    if (link_info->connected_device[0].link_type == BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE) {
        bt_device_manager_le_bonded_info_t *le_bond_info = bt_device_manager_le_get_bonding_info_by_addr_ext(active_addr);
        if (le_bond_info != NULL) {
            memcpy((*active_addr), le_bond_info->bt_addr.addr, sizeof(bt_bd_addr_t));
        }
    }
#endif
    APPS_LOG_MSGID_I(APP_HFP_UTILS" app_hfp_get_active_device_addr: =%02X:%02X:%02X:%02X:%02X:%02X", 6
                     , (*active_addr)[0], (*active_addr)[1], (*active_addr)[2], (*active_addr)[3], (*active_addr)[4], (*active_addr)[5]);

    APPS_LOG_MSGID_I(APP_HFP_UTILS"list_num=%d, conn_num=%d, addr_type=%x",
                     3, list_num, link_info->connected_num, link_info->connected_device[0].link_type);

    return true;

}

uint8_t app_hfp_get_active_device_type()
{
    bt_sink_srv_device_t type = BT_SINK_SRV_DEVICE_INVALID;
    bt_sink_srv_state_manager_played_device_t list = {0};
    uint32_t list_num = 0;
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    list_num = bt_sink_srv_state_manager_get_played_device_list(&list, 1);
#endif
    const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();
    if (link_info == NULL || (link_info != NULL && link_info->connected_num == 0)) {
        return type;
    }
    if (list_num == 0) {
        /* Use the last of address. */
        type = (link_info->connected_device[0].link_type == BT_DEVICE_MANAGER_LINK_TYPE_LE) ? BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE : BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR;
    } else if (list_num == 1) {
        type          = list.type;
    }

    APPS_LOG_MSGID_I(APP_HFP_UTILS" app_hfp_get_active_device_type: list_num=%d, type=%d", 2, list_num, type);
    return type;
}

bool app_hfp_get_va_active_device(bt_bd_addr_t *active_addr)
{
    bool ret = false;
    if (NULL == active_addr) {
        return ret;
    }

    const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();
    if (link_info == NULL || (link_info != NULL && link_info->connected_num == 0)) {
        return ret;
    }

    bt_sink_srv_state_manager_played_device_t played_list[3] = {0};
    uint32_t play_list_num = 0;
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    play_list_num = bt_sink_srv_state_manager_get_played_device_list(played_list, 3);
#endif
    if (play_list_num != 0) {
        for (int i=0; i<play_list_num; i++) {
            if (played_list[i].type == BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR) {
                memcpy((*active_addr), played_list[i].address, sizeof(bt_bd_addr_t));
                ret = true;
                break;
            }
        }
    } else {
        for (int j=0; j<link_info->connected_num; j++) {
            if (link_info->connected_device[j].link_type == BT_DEVICE_MANAGER_LINK_TYPE_EDR) {
                memcpy((*active_addr), link_info->connected_device[j].remote_addr, sizeof(bt_bd_addr_t));
                ret = true;
                break;
            }
        }
    }


    APPS_LOG_MSGID_I(APP_HFP_UTILS" get_va_active_device: ret=%d, list_num=%d, connected_num=%d", 3, ret, play_list_num, link_info->connected_num);
    APPS_LOG_MSGID_I(APP_HFP_UTILS" get_va_active_device: addr=%02X:%02X:%02X:%02X:%02X:%02X", 6
                     , (*active_addr)[0], (*active_addr)[1], (*active_addr)[2], (*active_addr)[3], (*active_addr)[4], (*active_addr)[5]);
    return ret;
}

apps_config_state_t app_hfp_get_config_status_by_state(bt_sink_srv_state_t state)
{
    apps_config_state_t status = APP_TOTAL_STATE_NO;
    /*APPS_LOG_MSGID_I(APP_HFP_UTILS", hfp_state: %x", 1, state);*/

    switch (state) {
        case BT_SINK_SRV_STATE_INCOMING: {
            /* There is an incoming call. */
            status = APP_HFP_INCOMING;
            break;
        }
        case BT_SINK_SRV_STATE_OUTGOING: {
            /* There is an outgoing call. */
            status = APP_HFP_OUTGOING;
            break;
        }
        case BT_SINK_SRV_STATE_ACTIVE: {
            /* There is an active call only.*/
            status = APP_HFP_CALL_ACTIVE;
            break;
        }
        case BT_SINK_SRV_STATE_TWC_INCOMING: {
            /* There is an active call and a waiting incoming call. */
            status = APP_HFP_TWC_INCOMING;
            break;
        }
        case BT_SINK_SRV_STATE_TWC_OUTGOING: {
            /* There is a held call and a outgoing call. */
            status = APP_HFP_TWC_OUTGOING;
            break;
        }
        case BT_SINK_SRV_STATE_HELD_ACTIVE: {
            /* There is an active call and a held call. */
            status = APP_STATE_HELD_ACTIVE;
            break;
        }
        case BT_SINK_SRV_STATE_HELD_REMAINING: {
            /* There is a held call only. */
            status = APP_HFP_CALL_ACTIVE_WITHOUT_SCO;
            break;
        }
        case BT_SINK_SRV_STATE_MULTIPARTY: {
            /* There is a conference call. */
            status = APP_HFP_MULTIPARTY_CALL;
            break;
        }
    }
    /*APPS_LOG_MSGID_I(APP_HFP_UTILS", hfp_status: %x", 1, status);*/
    return status;
}

void app_hfp_stop_vp(void)
{
    voice_prompt_stop(VP_INDEX_INCOMING_CALL, VOICE_PROMPT_ID_INVALID, true);
    voice_prompt_stop(VP_INDEX_TWC_INCOMING_CALL, VOICE_PROMPT_ID_INVALID, true);
}

bool app_hfp_send_call_action(apps_config_key_action_t action)
{
    bt_sink_srv_action_t sink_action  = BT_SINK_SRV_ACTION_NONE;
    bool ret = true;

#if defined(AIR_MCSYNC_SHARE_ENABLE) && defined(MTK_AWS_MCE_ENABLE)
    /* Share mode need to process the key event. */
    if (app_share_get_pairing_result() && bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_AGENT) {
        APPS_LOG_MSGID_I(APP_HFP_UTILS", set ret flag to false", 0);
        ret = false;
    }
#endif

    /* Map key action to sink service action. */
    switch (action) {
        case KEY_ACCEPT_CALL: {
#ifdef AIR_MS_TEAMS_ENABLE
            app_ms_teams_set_button_press_info_hook(true);
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_AUTO_ACCEPT_INCOMING_CALL);
#endif
            sink_action = BT_SINK_SRV_ACTION_ANSWER;
            break;
        }
        case KEY_REJCALL: {
#ifdef MTK_IN_EAR_FEATURE_ENABLE
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_AUTO_ACCEPT_INCOMING_CALL);
#endif
            sink_action = BT_SINK_SRV_ACTION_REJECT;
#ifdef MTK_AWS_MCE_ENABLE
            bt_aws_mce_role_t aws_role = bt_device_manager_aws_local_info_get_role();
            if (aws_role == BT_AWS_MCE_ROLE_AGENT || aws_role == BT_AWS_MCE_ROLE_NONE)
#endif
            {
                app_hfp_stop_vp();
                voice_prompt_param_t vp = {0};
                vp.vp_index = VP_INDEX_CALL_REJECTED;
                vp.control = VOICE_PROMPT_CONTROL_MASK_SYNC;
                vp.delay_time = 200;
                voice_prompt_play(&vp, NULL);
            }
#if defined(MTK_AWS_MCE_ENABLE) && !defined(AIR_SPEAKER_ENABLE)
            else {
                uint32_t event_group = EVENT_GROUP_UI_SHELL_KEY;
                apps_aws_sync_event_send(event_group, KEY_REJCALL);
            }
#endif
            break;
        }
        case KEY_ONHOLD_CALL: {
            sink_action = BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER;
            break;
        }
        case KEY_END_CALL: {
#ifdef AIR_MS_TEAMS_ENABLE
            app_ms_teams_set_button_press_info_hook(false);
#endif
            sink_action = BT_SINK_SRV_ACTION_HANG_UP;
            break;
        }
        case KEY_VOICE_UP: {
            sink_action = BT_SINK_SRV_ACTION_CALL_VOLUME_UP;
            break;
        }
        case KEY_VOICE_DN: {
            sink_action = BT_SINK_SRV_ACTION_CALL_VOLUME_DOWN;
            break;
        }
        case KEY_REJCALL_SECOND_PHONE:  {
#ifdef AIR_MS_TEAMS_ENABLE
            app_ms_teams_set_button_press_info_flash(false);
#endif
            sink_action = BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD;
            break;
        }
        case KEY_SWITCH_AUDIO_PATH:
            sink_action = BT_SINK_SRV_ACTION_SWITCH_AUDIO_PATH;
            break;
        case KEY_3WAY_HOLD_ACTIVE_ACCEPT_OTHER: {
#ifdef AIR_MS_TEAMS_ENABLE
            app_ms_teams_set_button_press_info_flash(true);
#endif
            sink_action = BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER;
            break;
        }
        default: {
            ret = false;
            break;
        }
    }

#ifdef AIR_SPEAKER_ENABLE
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
    if (ret && (role == BT_AWS_MCE_ROLE_PARTNER || role == BT_AWS_MCE_ROLE_CLIENT)) {
        APPS_LOG_MSGID_E(APP_HFP_UTILS" send_call_action, return role=%02X", 1, role);
        return ret;
    }
#endif

#ifdef AIR_LE_AUDIO_ENABLE
    if (KEY_VOICE_UP == action || KEY_VOICE_DN == action) {
        bt_handle_t handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(bt_sink_srv_cap_am_get_current_mode());
        if (handle != BT_HANDLE_INVALID && (app_le_audio_aird_client_is_support_hid_call(handle) || app_le_audio_aird_client_is_support(handle))) {
            app_le_audio_aird_action_t aird_action = (action == KEY_VOICE_UP ? APP_LE_AUDIO_AIRD_ACTION_SET_STREAMING_VOLUME_UP : APP_LE_AUDIO_AIRD_ACTION_SET_STREAMING_VOLUME_DOWN);
            app_le_audio_aird_action_set_streaming_volume_t param;
            param.streaming_interface = APP_LE_AUDIO_AIRD_STREAMING_INTERFACE_SPEAKER;
            param.streaming_port = APP_LE_AUDIO_AIRD_STREAMING_PORT_0;
            param.channel = APP_LE_AUDIO_AIRD_CHANNEL_DUAL;
            param.volume = 1;   /* The delta value. */

            app_le_audio_aird_client_send_action(handle, aird_action,
                                                 &param, sizeof(app_le_audio_aird_action_set_streaming_volume_t));
            return ret;
        } else {
            APPS_LOG_MSGID_I(APP_HFP_UTILS" [LEA][CIS] VOLUME condition not met: handle=0x%x", 1, handle);
        }
    }
#endif

    APPS_LOG_MSGID_I(APP_HFP_UTILS", ret: %x, sink_action : %x", 2, ret, sink_action);

    if (BT_SINK_SRV_ACTION_NONE != sink_action) {
        bt_sink_srv_send_action(sink_action, NULL);
    }
    return ret;
}

bool app_hfp_update_led_bg_pattern(ui_shell_activity_t *self, bt_sink_srv_state_t now, bt_sink_srv_state_t pre)
{
    bool ret = true;

    /* Update LED display by current MMI state. */
    switch (now) {
        case BT_SINK_SRV_STATE_INCOMING: {
            apps_config_set_background_led_pattern(LED_INDEX_INCOMING_CALL, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            break;
        }
        case BT_SINK_SRV_STATE_OUTGOING: {
            apps_config_set_background_led_pattern(LED_INDEX_OUTGOING_CALL, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            break;
        }
        case BT_SINK_SRV_STATE_ACTIVE: {
            apps_config_set_background_led_pattern(LED_INDEX_CALL_ACTIVE, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            break;
        }
        case BT_SINK_SRV_STATE_HELD_REMAINING: {
            apps_config_set_background_led_pattern(LED_INDEX_HOLD_CALL, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            break;
        }
        case BT_SINK_SRV_STATE_HELD_ACTIVE: {
            apps_config_set_background_led_pattern(LED_INDEX_CALL_ACTIVE, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            break;
        }
        case BT_SINK_SRV_STATE_TWC_INCOMING: {
            apps_config_set_background_led_pattern(LED_INDEX_INCOMING_CALL, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            break;
        }
        case BT_SINK_SRV_STATE_TWC_OUTGOING: {
            apps_config_set_background_led_pattern(LED_INDEX_OUTGOING_CALL, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            break;
        }
        case BT_SINK_SRV_STATE_MULTIPARTY: {
            /* There is a conference call. */
            apps_config_set_background_led_pattern(LED_INDEX_CALL_ACTIVE, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            break;
        }
        default: {
            ret = false;
            break;
        }
    }

    return ret;
}

/**
* @brief      This function is used to record the "incoming call" vp sync state.
* @return     The state of "incoming call" vp, 1 means the vp is playing, 0 means not.
*/
static void app_hfp_incoming_call_vp_callback(uint32_t idx, voice_prompt_event_t err)
{
#if defined(AIR_PROMPT_SOUND_ENABLE) && defined(MTK_AWS_MCE_ENABLE)
    if (idx == VP_INDEX_TWC_INCOMING_CALL || idx == VP_INDEX_INCOMING_CALL) {
        APPS_LOG_MSGID_I(APP_HFP_UTILS" [RING_IND] incoming call vp sync state=%d.", 1, err);
    }
#endif
}

void app_hfp_report_battery_to_remote(int32_t bat_val, int32_t pre_val,uint8_t formstate)
{
    /* Report current battery level to remote device, such as smartphone. */
    int32_t peer_bat_level,current_bat_level,bal_val1,bal_val2;
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
    APPS_LOG_MSGID_I(APP_HFP_UTILS", bat_val : %d, pre_val : %d,role=%d,formstate=%d", 4, bat_val, pre_val,role,formstate);
    // harry for battery level display.
    bal_val1=(bat_val & 0x7F);
    if (bal_val1>100)
    {
         APPS_LOG_MSGID_E(APP_HFP_UTILS",BATT>100 error, bat_val1 : %d,bat_val : %d,",2,bal_val1,bat_val); 
    }
    peer_bat_level=((app_get_smcharger_context()->peer_battery_percent) & 0x7F);
    current_bat_level=((app_get_smcharger_context()->battery_percent) & 0x7F);
    APPS_LOG_MSGID_I(APP_HFP_UTILS", bat_val : origine battery_percent : %d,peer_battery_percent=%d", 2, app_get_smcharger_context()->battery_percent,app_get_smcharger_context()->peer_battery_percent);
    APPS_LOG_MSGID_I(APP_HFP_UTILS", bat_val : %d,bal_val1=%d, peer_bat_level : %d, current_bat_level : %d", 4, bat_val,bal_val1, peer_bat_level,current_bat_level);
    if(bat_val>=128)
    {
      if(formstate==WHEN_CONNECTING)
      {
         APPS_LOG_MSGID_I(APP_HFP_UTILS", bat_val : %d,hfp conecting local bat in charging",1,bat_val); 
      }
      else if(formstate==PEER_REPORT)
      {
         APPS_LOG_MSGID_I(APP_HFP_UTILS", bat_val : %d,peer bat in charging",1,bat_val); 
      }
      else if(formstate==LOCAL_REPORT)
      {
         APPS_LOG_MSGID_I(APP_HFP_UTILS", bat_val : %d,local bat in charging",1,bat_val); 
      }
    }
    if (pre_val != bat_val) 
    {
        // harry for battery level display
        if(BT_AWS_MCE_SRV_LINK_NONE!=bt_aws_mce_srv_get_link_type())
        {
          if ((role == BT_AWS_MCE_ROLE_AGENT))
          {
            if(formstate==WHEN_CONNECTING)
              {
                bal_val2=(bal_val1<peer_bat_level)? bal_val1:peer_bat_level;
               APPS_LOG_MSGID_I(APP_HFP_UTILS",aws sent battery level %d to phone,when hfp connecting,peer_bat_level=%d,bat_val=%d,bal_val1=%d",4,bal_val2,peer_bat_level,bal_val1,bat_val); 
               bt_sink_srv_send_action(BT_SINK_SRV_ACTION_REPORT_BATTERY_EXT, &bal_val2);
              }
            else if(formstate==PEER_REPORT)
              {
                if((bal_val1&0x7f)<current_bat_level)
                {
                 APPS_LOG_MSGID_I(APP_HFP_UTILS",aws sent battery level %d to phone,when PEER BATT SMALL,bat_val=%d",2,bal_val1,bat_val); 
                 bt_sink_srv_send_action(BT_SINK_SRV_ACTION_REPORT_BATTERY_EXT, &bal_val1);
                }
              }
            else if(formstate==LOCAL_REPORT)
              {
                if((bal_val1&0x7f)<peer_bat_level)
                  {
                 APPS_LOG_MSGID_I(APP_HFP_UTILS",aws sent battery level %d to phone,when local BATT SMALL,bat_val=%d",2,bal_val1,bat_val); 
                 bt_sink_srv_send_action(BT_SINK_SRV_ACTION_REPORT_BATTERY_EXT, &bal_val1);
                  }
              }
         }
          else
          {
              // aws is connected ,partner no need report to phone by hfp
          }
        }
        else
        {
          if (bal_val1<=100)
            {
          APPS_LOG_MSGID_I(APP_HFP_UTILS",single sent battery level %d to phone,bal_val=%d",2,bal_val1,bat_val); 
          bt_sink_srv_send_action(BT_SINK_SRV_ACTION_REPORT_BATTERY_EXT, &bal_val1);
            }
          else
            {
          APPS_LOG_MSGID_I(APP_HFP_UTILS",single sent battery level  nooooo sent  to phone,because bal_val%d,bal_val=%d",2,bal_val1,bat_val); 
            }
        }
    }
}

#ifdef MTK_IN_EAR_FEATURE_ENABLE
uint8_t app_hfp_is_auto_accept_incoming_call()
{
    nvkey_status_t status = NVKEY_STATUS_OK;
    uint8_t isEnable = 0;
    uint32_t data_size = sizeof(uint8_t);
    if (APP_HFP_AUTO_ACCEPT_NONE != g_app_hfp_auto_accept) {
        isEnable =  g_app_hfp_auto_accept;
    } else {
        status = nvkey_read_data(NVID_APP_IN_EAR_HFP_ABILITY, &isEnable, &data_size);
        if (NVKEY_STATUS_ITEM_NOT_FOUND == status) {
            isEnable = APP_HFP_AUTO_ACCEPT_DISABLE;
            status = nvkey_write_data(NVID_APP_IN_EAR_HFP_ABILITY, &isEnable, sizeof(uint8_t));
            if (NVKEY_STATUS_OK == status) {
                g_app_hfp_auto_accept = isEnable;
            }
        }
    }
    APPS_LOG_MSGID_I(APP_HFP_UTILS" [AUTO ACCEPT] get in ear control state: status=%d isEnable=%d.", 2, status, isEnable);
    return isEnable;
}

bool app_hfp_set_auto_accept_incoming_call(uint8_t auto_accept, bool sync)
{
    bool ret = false;
    nvkey_status_t status = NVKEY_STATUS_OK;
    status = nvkey_write_data(NVID_APP_IN_EAR_HFP_ABILITY, &auto_accept, sizeof(uint8_t));
    if (status == NVKEY_STATUS_OK) {
        ret = true;
        g_app_hfp_auto_accept = auto_accept;
#ifdef MTK_AWS_MCE_ENABLE
        if (sync) {
            app_hfp_notify_state_to_peer();
        }
#endif
    }
    APPS_LOG_MSGID_I(APP_HFP_UTILS" [AUTO ACCEPT] set in ear control value: status=%d, isEnable=%d, sync=%d.",
                     3, status, auto_accept, sync);
    return ret;
}

void app_hfp_notify_state_to_peer()
{
#ifdef MTK_AWS_MCE_ENABLE
    uint8_t auto_accept = app_hfp_is_auto_accept_incoming_call();
    if (TRUE == app_home_screen_idle_activity_is_aws_connected()) {
        bt_status_t send_state = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                                APPS_EVENTS_INTERACTION_SYNC_AUTO_ACCEPT_STATUS, &auto_accept, sizeof(uint8_t));
        APPS_LOG_MSGID_I(APP_HFP_UTILS" [AUTO ACCEPT] sync auto accept status to peer: state=%d.", 1, send_state);
    }
#endif
}

#endif

#if defined(AIR_MULTI_POINT_ENABLE) && defined(AIR_EMP_AUDIO_INTER_STYLE_ENABLE)
void app_hfp_emp_music_process(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    static bt_bd_addr_t device_in_music = {0};
    static bt_bd_addr_t empt_addr = {0};
    bt_status_t sta = BT_STATUS_SUCCESS;
    uint32_t conn_nums;
    bt_bd_addr_t conn_addrs[2];
    uint32_t idx = 0;
    uint8_t *p_t = NULL;
    if (event_id == BT_SINK_SRV_EVENT_STATE_CHANGE) {
        bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *) extra_data;
        switch (param->current) {
            /* If music started, member the device's addr */
            case BT_SINK_SRV_STATE_STREAMING: {
                conn_nums = bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), conn_addrs, 2);
                for (idx = 0; idx < conn_nums; idx++) {
                    bt_sink_srv_device_state_t dev_sta = {{0}, 0, 0, 0, 0};
                    bt_sink_srv_get_device_state((const bt_bd_addr_t *)&conn_addrs[idx], &dev_sta, 1);
                    if (dev_sta.music_state == BT_SINK_SRV_STATE_STREAMING) {
                        memcpy(&device_in_music, &conn_addrs[idx], sizeof(bt_bd_addr_t));
                        p_t = (uint8_t *)&conn_addrs[idx];
                        APPS_LOG_MSGID_I(APP_HFP_UTILS" MUSIC ADDR: %x,%x,%x,%x.", 4, p_t[0], p_t[1], p_t[2], p_t[3]);
                    }
                }
                break;
            }

            /* If HFP start, try to disconnect the second phone's A2DP and AVRCP connection. */
            case BT_SINK_SRV_STATE_INCOMING:
            case BT_SINK_SRV_STATE_OUTGOING:
            case BT_SINK_SRV_STATE_ACTIVE: {
                const bt_bd_addr_t *hfp_addr = NULL;
                conn_nums = bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), conn_addrs, 2);
                if (conn_nums < 2) {
                    break;
                }

                hfp_addr = bt_sink_srv_hf_get_last_operating_device();
                if (hfp_addr == NULL) {
                    break;
                }
                /* If another phone is not in playing state, disconnect it's A2DP and avrcp connection. */
                if ((memcmp(&device_in_music, &empt_addr, sizeof(bt_bd_addr_t)) == 0) ||
                    (memcmp(&device_in_music, hfp_addr, sizeof(bt_bd_addr_t)) == 0)) {
                    /* Find idle device's addr. */
                    bt_bd_addr_t *idle_addr = NULL;
                    if (memcmp(hfp_addr, &conn_addrs[0], sizeof(bt_bd_addr_t)) == 0) {
                        idle_addr = &conn_addrs[1];
                    } else {
                        idle_addr = &conn_addrs[0];
                    }
                    /* Disconnect */
                    p_t = (uint8_t *)hfp_addr;
                    APPS_LOG_MSGID_I(APP_HFP_UTILS" HFP ADDR: %x,%x,%x,%x.", 4, p_t[0], p_t[1], p_t[2], p_t[3]);
                    p_t = (uint8_t *)idle_addr;
                    APPS_LOG_MSGID_I(APP_HFP_UTILS" idle ADDR: %x,%x,%x,%x.", 4, p_t[0], p_t[1], p_t[2], p_t[3]);
                    bt_cm_connect_t dis_conn = {{0}, 0};
                    memcpy(&dis_conn.address, idle_addr, sizeof(bt_bd_addr_t));
                    dis_conn.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SOURCE);
                    sta = bt_cm_disconnect(&dis_conn);
                    dis_conn.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP);
                    sta = bt_cm_disconnect(&dis_conn);
                    APPS_LOG_MSGID_I(APP_HFP_UTILS" disconnect the A2DP/AVRCP profile sta: %d.", 1, sta);
                    /* Clear music info. */
                    memset(&device_in_music, 0, sizeof(bt_bd_addr_t));
                }
                break;
            }

            /**/
            default: {
                /* Clear music info. */
                memset(&device_in_music, 0, sizeof(bt_bd_addr_t));
                break;
            }
        }

        /* try to resume connection */
        if (param->current <= BT_SINK_SRV_STATE_STREAMING) {
            bt_cm_profile_service_mask_t music_mask = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP);
            music_mask |= BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SOURCE);
            bt_cm_profile_service_mask_t conn_mask = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS);
            conn_nums = bt_cm_get_connected_devices(~conn_mask, conn_addrs, 2);
            for (idx = 0; idx < conn_nums; idx++) {
                bt_cm_profile_service_mask_t mask = bt_cm_get_connected_profile_services(&conn_addrs[idx]);
                if ((mask & music_mask) == 0) {
                    bt_cm_connect_t conn = {{0}, 0};
                    p_t = (uint8_t *)&conn_addrs[idx];
                    APPS_LOG_MSGID_I(APP_HFP_UTILS" connected with out A2DP addr: %x,%x,%x,%x.", 4, p_t[0], p_t[1], p_t[2], p_t[3]);
                    memcpy(&conn.address, &conn_addrs[idx], sizeof(bt_bd_addr_t));
                    conn.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP);
                    sta = bt_cm_connect(&conn);
                    conn.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SOURCE);
                    sta = bt_cm_connect(&conn);
                    APPS_LOG_MSGID_I(APP_HFP_UTILS" connect the A2DP/AVRCP profile sta: %d.", 1, sta);
                }
            }
        }
    }
}

bool app_hfp_proc_conflict_vp_event(ui_shell_activity_t *self,
                                    uint32_t event_group,
                                    uint32_t event_id,
                                    void *extra_data,
                                    size_t data_len)
{
    bool ret = false;
    /* Play the conflict incoming call VP when two smart phones incoming call at the same time. */
    hfp_context_t *hfp_context = (hfp_context_t *)(self->local_context);
    bt_sink_srv_event_call_state_conflict_t *update_ring_ind = (bt_sink_srv_event_call_state_conflict_t *)extra_data;
    if (update_ring_ind == NULL) {
        return ret;
    }

    if (update_ring_ind->call_state == BT_SINK_SRV_STATE_INCOMING) {
        if (APP_HFP_INCOMING_CALL_VP_LONG == app_hfp_get_incoming_call_vp_flag()) {
            app_hfp_incoming_call_vp_process(self, FALSE, FALSE);
        } else {
            app_hfp_incoming_call_vp_process(self, TRUE, FALSE);
        }
    } else {
        /* Two incoming call -->  one incoming call (one of incoming was rejected or accepted)*/

        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_UPDATE_MMI_STATE, NULL, 0, NULL, 0);

        bt_bd_addr_t *actived_addr = NULL;
        uint8_t *p_t = NULL;
        bt_sink_srv_device_state_t actived_addr_state_list = {{0}, 0, 0, 0, 0};
        uint32_t conn_nums;
        bt_bd_addr_t conn_addrs[2];
        uint32_t idx = 0;
        conn_nums = bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), conn_addrs, 2);

        for (idx = 0; idx < conn_nums; idx++) {
            if (memcmp(&update_ring_ind->address, &conn_addrs[idx], sizeof(bt_bd_addr_t)) != 0) {
                //memcpy(&device_actived, &conn_addrs[idx], sizeof(bt_bd_addr_t));
                actived_addr = &conn_addrs[idx];
            }
        }
        p_t = (uint8_t *)actived_addr;
        APPS_LOG_MSGID_I(APP_HFP_UTILS",[CONFLICT VP] actived_addr:%02x %02x %02x %02x %02x %02x.",
                         6, p_t[0], p_t[1], p_t[2], p_t[3], p_t[4], p_t[5]);
        bt_sink_srv_get_device_state((const bt_bd_addr_t *)actived_addr, &actived_addr_state_list, 1);
        if (actived_addr_state_list.is_inband_supported == TRUE) {
            if (VP_INDEX_INCOMING_CALL == voice_prompt_get_current_index()) {
                app_hfp_stop_vp();
            }
        } else {
            app_hfp_incoming_call_vp_process(self, TRUE, FALSE);
        }
    }

    return ret;
}
#endif

void app_hfp_incoming_call_vp_process(ui_shell_activity_t *self, bool isLongVp, bool isTwcIncoming)
{
    hfp_context_t *hfp_context = (hfp_context_t *)(self->local_context);
    if (hfp_context == NULL) {
        return;
    }

    uint32_t vp_delay_time = (isLongVp == TRUE) ? APP_HFP_INCOMING_CALL_VP_LONG_DELAY_TIME : APP_HFP_INCOMING_CALL_VP_SHORT_DELAY_TIME;
    uint32_t vp_index      = (isTwcIncoming == TRUE) ? VP_INDEX_TWC_INCOMING_CALL : VP_INDEX_INCOMING_CALL;
    uint32_t stop_vp_index = (isTwcIncoming == FALSE) ? VP_INDEX_TWC_INCOMING_CALL : VP_INDEX_INCOMING_CALL;
    APPS_LOG_MSGID_I(APP_HFP_UTILS",[RING_IND] set vp: vp_delay_time=%d, vp_index=%d, aws_link_state=%d",
                     3, vp_delay_time, vp_index, hfp_context->aws_link_state);

    if (vp_index == voice_prompt_get_current_index()) {
        return;
    }

    voice_prompt_param_t vp = {0};
    vp.vp_index   = vp_index;
    vp.control    = VOICE_PROMPT_CONTROL_LOOPRT;
    vp.delay_time = vp_delay_time;
    vp.callback   = app_hfp_incoming_call_vp_callback;
    voice_prompt_stop(stop_vp_index, VOICE_PROMPT_ID_INVALID, true);
    voice_prompt_play(&vp, NULL);
}

#if defined(AIR_MULTI_POINT_ENABLE) && defined(MTK_AWS_MCE_ENABLE) && !defined(AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE)
void app_hfp_sync_sink_state_to_peer(ui_shell_activity_t *self)
{
    hfp_context_t *hfp_context = (hfp_context_t *)(self->local_context);
    if (hfp_context == NULL) {
        return;
    }
    bt_sink_srv_state_change_t param;
    bt_status_t send_state = BT_STATUS_FAIL;

    param.current = hfp_context->curr_state;
    param.previous = hfp_context->pre_state;

    if (TRUE == hfp_context->aws_link_state) {
        send_state = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                    APPS_EVENTS_INTERACTION_SYNC_HFP_STATE_TO_PARTNER, &param, sizeof(bt_sink_srv_state_change_t));
    }

    APPS_LOG_MSGID_I(APP_HFP_UTILS", agent send hfp_state_change to partner: send_state=%x, param->previous: %x,param->current: %x",
                     3, send_state, hfp_context->pre_state, hfp_context->curr_state);
}
#endif

/**
  * @brief                            This function is used to mute or unmute microphone.
  * @param[in]  type            True is mute mic, flase is unmute mic.
  */
bool app_hfp_mute_mic(bool mute)
{
    bool ret = false;

    /* Mute or unmute mic.*/
    bt_status_t bt_status = BT_STATUS_FAIL;

#ifdef AIR_LE_AUDIO_ENABLE
    bt_handle_t handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(bt_sink_srv_cap_am_get_current_mode());
    if (handle != BT_HANDLE_INVALID && app_le_audio_aird_client_is_support_hid_call(handle)) {
        app_le_audio_aird_client_send_action(handle, APP_LE_AUDIO_AIRD_ACTION_MUTE_MIC, NULL, 0);
      #if 0  //harry for new vp
      if (mute==TRUE)
      {
        APPS_LOG_MSGID_I(APP_HFP_UTILS" [LEA] AIRD_Client MUTE mic leaudio : handle=0x%x", 1, handle);
        voice_prompt_play_sync_vp_mute();
      }
      #endif
        return TRUE;
    } else {
        APPS_LOG_MSGID_I(APP_HFP_UTILS" [LEA] AIRD_Client MUTE condition not met: handle=0x%x", 1, handle);
    }

    bt_status = bt_sink_srv_le_volume_set_mute_ex(BT_SINK_SRV_LE_STREAM_TYPE_IN, mute, true);
    if (bt_status == BT_STATUS_SUCCESS) {
        ret = TRUE;
    }
#endif

    bt_status = bt_sink_srv_set_mute(BT_SINK_SRV_MUTE_MICROPHONE, mute);
    if (bt_status == BT_STATUS_SUCCESS) {
        ret = TRUE;
    }
    if (ret) {
        /* Sync mute or unmute operation to peer when aws mce enable.*/
#ifdef MTK_AWS_MCE_ENABLE
        if (TRUE == app_home_screen_idle_activity_is_aws_connected()) {
            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                           APPS_EVENTS_INTERACTION_SYNC_MIC_MUTE_STATUS,
                                           &mute, sizeof(bool));
        }
#endif
      #if 0  //harry for new vp
      if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()&&(mute==TRUE))
      {
        voice_prompt_play_sync_vp_mute();
      }
      #else

#ifdef MTK_AWS_MCE_ENABLE
        if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role())
#endif
        {
            voice_prompt_play_sync_vp_succeed();
        }
      #endif
    }

    APPS_LOG_MSGID_I(APP_HFP_UTILS" [MUTE] mute_mic=%d, bt_status=0x%08X, ret=%d",
                     3, mute, bt_status, ret);
    return ret;
}

#ifdef MTK_AWS_MCE_ENABLE
bool app_hfp_get_aws_link_is_switching(void)
{
    bool ret = false;
    /* Get current aws link state and connected/disconnected reason. */
    const app_bt_state_service_status_t *curr_state = app_bt_connection_service_get_current_status();
    if (!curr_state->aws_connected && curr_state->reason == BT_HCI_STATUS_SUCCESS) {
        ret = true;
    }
    APPS_LOG_MSGID_I(APP_HFP_UTILS" get_aws_link_is_switching: ret=%d", 1, ret);

    return ret;
}
#endif

bool app_hfp_set_va_enable(bool enable)
{
    bool ret = FALSE;
    bool trigger_ret = FALSE;
    bt_status_t bt_status =  BT_STATUS_FAIL;
    bt_sink_srv_action_voice_recognition_activate_ext_t va_trigger;
    hfp_context_t *hfp_context = app_hfp_get_context();
    if (hfp_context == NULL || hfp_context->voice_assistant == enable) {
        return ret;
    }


    va_trigger.activate = enable;
    /* Get trigger address.*/
    va_trigger.type = BT_SINK_SRV_DEVICE_EDR;
    //trigger_ret = app_hfp_get_active_device_addr(&(va_trigger.address));
    trigger_ret = app_hfp_get_va_active_device(&(va_trigger.address));
    
    if (trigger_ret == TRUE) {
        bt_status = bt_sink_srv_send_action(BT_SINK_SRV_ACTION_VOICE_RECOGNITION_ACTIVATE_EXT,
                                            (void *)&va_trigger);
    } 

    if (BT_STATUS_SUCCESS == bt_status) {
        ret = TRUE;
    }

    APPS_LOG_MSGID_I(APP_HFP_UTILS"  set_va_enable to %d, ret=%d", 2, enable, ret);

    return ret;
}
