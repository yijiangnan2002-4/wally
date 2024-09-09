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
 * File: app_music_utils.c
 *
 * Description: this file provide common functions for music_app.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */


#include "app_music_utils.h"
#include "app_music_activity.h"
#include "app_home_screen_idle_activity.h"
#include "bt_sink_srv_music.h"
#include "bt_device_manager.h"
#include "apps_debug.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_ami.h"
#include "apps_events_key_event.h"
#include "apps_events_interaction_event.h"
#include "apps_events_event_group.h"
#ifdef AIR_GSOUND_ENABLE
#include "app_gsound_service.h"
#endif
#if defined(MTK_AWS_MCE_ENABLE)
#include "bt_aws_mce_srv.h"
#include "bt_aws_mce_report.h"
#include "apps_aws_sync_event.h"
#endif
#include "bt_connection_manager.h"

#ifdef AIR_SMART_CHARGER_ENABLE
#include "app_smcharger_idle_activity.h"
#include "app_smcharger_utils.h"
#include "app_smcharger.h"
#elif defined(MTK_BATTERY_MANAGEMENT_ENABLE)
#include "app_battery_transient_activity.h"
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "bt_ull_service.h"
#include "bt_ull_le_utility.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "app_ull_idle_activity.h"
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_utils.h"
#include "nvkey_id_list.h"
#include "nvkey.h"
#endif
#ifdef AIR_MCSYNC_SHARE_ENABLE
#include "app_share_idle_activity.h"
#endif
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio_aird_client.h"
#include "app_le_audio_bis.h"
#include "audio_src_srv.h"
#include "bt_sink_srv_le.h"
#include "bt_sink_srv_le_cap.h"
#include "bt_sink_srv_le_cap_stream.h"
#include "bt_device_manager_le.h"

extern bt_le_sink_srv_music_active_handle g_music_active_handle;
#endif

#ifdef APP_MUSIC_ADJUST_VOLUME_VIA_ADDRESS_ENABLE
#include "bt_device_manager_link_record.h"
#include "bt_sink_srv_state_manager.h"
#endif

#ifdef AIR_MS_GIP_ENABLE
#include "app_dongle_service.h"
#include "apps_dongle_sync_event.h"
#endif
#include "apps_customer_config.h"
#include "app_bt_conn_manager.h"

#ifdef MTK_IN_EAR_FEATURE_ENABLE
uint8_t g_music_in_ear_config = APP_MUSIC_IN_EAR_NONE;        /**<  Record the music in ear config. */
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
bool app_music_ull2_uplink_enable = false;
bool app_music_ull2_downlink_enable = false;
#endif

static app_music_avrcp_status_t s_app_music_avrcp_status = {0};

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
static bt_status_t app_music_set_ull_link_volume(bt_ull_streaming_interface_t interface, bool volume_up, uint32_t volume);
#endif

bool app_music_get_active_device(uint8_t *active_type, bt_bd_addr_t *active_addr)
{
    if (NULL == active_addr || active_type == NULL) {
        APPS_LOG_MSGID_W(APP_MUSIC_ACTI" parameter is NULL", 0);
        return false;
    }
    bt_sink_srv_state_manager_played_device_t list = {0};
    uint32_t list_num = 0;
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    list_num = bt_sink_srv_state_manager_get_played_device_list(&list, 1);
#endif
    const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();

    if (list_num == 1) {
        *active_type = list.type;
        memcpy((*active_addr), list.address, sizeof(bt_bd_addr_t));
    } else if (list_num == 0) {
        /* Use the last of invalid address. */
        if (link_info == NULL || (link_info != NULL && link_info->connected_num == 0)) {
            APPS_LOG_MSGID_W(APP_MUSIC_ACTI" get_active_device: get connected link info fail!", 0);
            return false;
        }
        for (int i=0; i<link_info->connected_num; i++) {
            if (link_info->connected_device[i].link_type == BT_DEVICE_MANAGER_LINK_TYPE_EDR) {
                bt_bd_addr_t temp_addr = {0};
                memcpy(temp_addr, link_info->connected_device[i].remote_addr, sizeof(bt_bd_addr_t));
                bt_cm_profile_service_mask_t music_mask = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK);
                bt_cm_profile_service_mask_t mask = bt_cm_get_connected_profile_services(temp_addr);
                if ((mask & music_mask) == 0) {
                    continue;
                } else {
                    *active_type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR;
                    memcpy((*active_addr), link_info->connected_device[i].remote_addr, sizeof(bt_bd_addr_t));
                    break;
                }
            } else {
            	uint8_t *addr = link_info->connected_device[i].remote_addr;
		bool isdongle=app_bt_conn_mgr_is_dongle(addr);
		if(!isdongle)
		{
                *active_type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE;
                memcpy((*active_addr), link_info->connected_device[i].remote_addr, sizeof(bt_bd_addr_t));
                break;
		}
            }
        }
    }
#ifdef AIR_LE_AUDIO_ENABLE
    if (*active_type == BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE) {
        bt_device_manager_le_bonded_info_t *le_bond_info = bt_device_manager_le_get_bonding_info_by_addr_ext(active_addr);
        if (le_bond_info != NULL) {
            memcpy((*active_addr), le_bond_info->bt_addr.addr, sizeof(bt_bd_addr_t));
        }
    }
#endif
    APPS_LOG_MSGID_I(APP_MUSIC_ACTI" get_active_device: type=%d, list_num=%d, connected_num=%d", 3, *active_type, list_num, link_info->connected_num);
    APPS_LOG_MSGID_I(APP_MUSIC_ACTI" get_active_device: addr=%02X:%02X:%02X:%02X:%02X:%02X", 6
                     , (*active_addr)[0], (*active_addr)[1], (*active_addr)[2], (*active_addr)[3], (*active_addr)[4], (*active_addr)[5]);
    return true;

}

bool app_music_add_avrcp_status(bt_sink_srv_event_param_t *sink_event)
{
    bool ret = false;
    if (sink_event == NULL || s_app_music_avrcp_status.avrcp_num >= APP_MUSIC_AVRCP_RECORD_MAX_NUM) {
        return ret;
    }

    uint8_t i = 0;

    for (i = 0; i < APP_MUSIC_AVRCP_RECORD_MAX_NUM; i++) {
        if (0 == memcmp(s_app_music_avrcp_status.avrcp_device[i].remote_addr, sink_event->avrcp_status_change.address, sizeof(bt_bd_addr_t))) {
            APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_add_avrcp_status already added.", 0);
            return ret;
        }
    }

    for (i = 0; i < APP_MUSIC_AVRCP_RECORD_MAX_NUM; i++) {
        if (!s_app_music_avrcp_status.avrcp_device[i].is_playing) {
            memcpy(s_app_music_avrcp_status.avrcp_device[i].remote_addr, sink_event->avrcp_status_change.address, sizeof(bt_bd_addr_t));
            s_app_music_avrcp_status.avrcp_device[i].is_playing = true;
            s_app_music_avrcp_status.avrcp_num++;
            ret = true;
            APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_add_avrcp_status: avrcp_addr-%02x:%02x:%02x:%02x:%02x:%02x",
                             6, sink_event->avrcp_status_change.address[5], sink_event->avrcp_status_change.address[4], sink_event->avrcp_status_change.address[3],
                             sink_event->avrcp_status_change.address[2], sink_event->avrcp_status_change.address[1], sink_event->avrcp_status_change.address[0]);

            break;
        }
    }

    return ret;
}

bool app_music_remove_avrcp_status(bt_bd_addr_t *addr)
{
    bool ret = false;
    if (addr == NULL || s_app_music_avrcp_status.avrcp_num == 0) {
        return ret;
    }

    uint8_t i = 0;
    for (i = 0; i < APP_MUSIC_AVRCP_RECORD_MAX_NUM; i++) {
        if (s_app_music_avrcp_status.avrcp_device[i].is_playing
            && (0 == memcmp(s_app_music_avrcp_status.avrcp_device[i].remote_addr, *addr, sizeof(bt_bd_addr_t)))) {
            memset(s_app_music_avrcp_status.avrcp_device[i].remote_addr, 0, sizeof(bt_bd_addr_t));
            s_app_music_avrcp_status.avrcp_device[i].is_playing = false;
            s_app_music_avrcp_status.avrcp_num--;
            ret = true;
            APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_remove_avrcp_status: avrcp_addr-%02x:%02x:%02x:%02x:%02x:%02x",
                             6, (*addr)[5], (*addr)[4], (*addr)[3],
                             (*addr)[2], (*addr)[1], (*addr)[0]);
            break;
        }
    }

    return ret;
}

app_music_avrcp_status_t *app_music_get_avrcp_status(void)
{
    return &s_app_music_avrcp_status;
}

bt_status_t app_music_send_actions_by_address(bt_sink_srv_action_t action)
{
    bt_status_t bt_status = BT_STATUS_FAIL;

    if (BT_SINK_SRV_ACTION_PLAY == action
        || BT_SINK_SRV_ACTION_PLAY_PAUSE == action) {
        bt_sink_srv_action_play_t play_device = {0};
        if (app_music_get_active_device(&(play_device.type), &(play_device.address))) {
            bt_status =  bt_sink_srv_send_action(action, &play_device);
        } else {
            bt_status = bt_sink_srv_send_action(action, NULL);
        }
    } else {
        bt_status = bt_sink_srv_send_action(action, NULL);
    }

    return bt_status;
}

static bool app_music_do_music_actions(bool from_aws_data, ui_shell_activity_t *self, apps_config_key_action_t action)
{
    bt_sink_srv_action_t sink_action = BT_SINK_SRV_ACTION_NONE;
    apps_music_local_context_t *local_context = (apps_music_local_context_t *)self->local_context;
    bt_status_t bt_status = BT_STATUS_FAIL;
    bool op_valid = false;
    bt_sink_srv_avrcp_operation_state_t op = BT_SINK_SRV_AVRCP_OPERATION_PRESS;

    bool ret = true;

    /* Map key action to sink service action. */
    switch (action) {
        case KEY_AVRCP_PLAY:
        case KEY_AVRCP_PAUSE: {
            sink_action = BT_SINK_SRV_ACTION_PLAY_PAUSE;
            break;
        }
        case KEY_VOICE_UP: {
            sink_action = BT_SINK_SRV_ACTION_VOLUME_UP;
            break;
        }
        case KEY_VOICE_DN: {
            sink_action = BT_SINK_SRV_ACTION_VOLUME_DOWN;
            break;
        }
        case KEY_AVRCP_BACKWARD: {
            sink_action = BT_SINK_SRV_ACTION_PREV_TRACK;
            break;
        }
        case KEY_AVRCP_FORWARD: {
            sink_action = BT_SINK_SRV_ACTION_NEXT_TRACK;
            break;
        }
        case KEY_AVRCP_FAST_FORWARD_PRESS: {
            if (local_context->avrcp_op_sta == AVRCP_OPERATION_STA_IDLE) {
                op = BT_SINK_SRV_AVRCP_OPERATION_PRESS;
                op_valid = true;
                sink_action = BT_SINK_SRV_ACTION_FAST_FORWARD;
                local_context->avrcp_op_sta = AVRCP_OPERATION_STA_FAST_FORWARD_PRESS;
            } else {
                ret = false;
            }
            break;
        }
        case KEY_AVRCP_FAST_FORWARD_RELEASE: {
            if (local_context->avrcp_op_sta == AVRCP_OPERATION_STA_FAST_FORWARD_PRESS) {
                op = BT_SINK_SRV_AVRCP_OPERATION_RELEASE;
                op_valid = true;
                sink_action = BT_SINK_SRV_ACTION_FAST_FORWARD;
                local_context->avrcp_op_sta = AVRCP_OPERATION_STA_IDLE;
            } else {
                ret = false;
            }
            break;
        }
        case KEY_AVRCP_FAST_REWIND_PRESS: {
            if (local_context->avrcp_op_sta == AVRCP_OPERATION_STA_IDLE) {
                op = BT_SINK_SRV_AVRCP_OPERATION_PRESS;
                op_valid = true;
                sink_action = BT_SINK_SRV_ACTION_REWIND;
                local_context->avrcp_op_sta = AVRCP_OPERATION_STA_FAST_REWIND_PRESS;
            } else {
                ret = false;
            }
            break;
        }
        case KEY_AVRCP_FAST_REWIND_RELEASE: {
            if (local_context->avrcp_op_sta == AVRCP_OPERATION_STA_FAST_REWIND_PRESS) {
                op = BT_SINK_SRV_AVRCP_OPERATION_RELEASE;
                op_valid = true;
                sink_action = BT_SINK_SRV_ACTION_REWIND;
                local_context->avrcp_op_sta = AVRCP_OPERATION_STA_IDLE;
            } else {
                ret = false;
            }
            break;
        }
        default: {
            ret = false;
            break;
        }
    }

#if defined(MTK_AWS_MCE_ENABLE)
    if ((BT_AWS_MCE_ROLE_PARTNER == bt_device_manager_aws_local_info_get_role())
        && (TRUE == app_home_screen_idle_activity_is_aws_connected())
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        && !app_le_audio_bis_is_streaming()
#endif
        && (BT_SINK_SRV_ACTION_PLAY_PAUSE == sink_action
            || BT_SINK_SRV_ACTION_PREV_TRACK == sink_action
            || BT_SINK_SRV_ACTION_NEXT_TRACK == sink_action
            || BT_SINK_SRV_ACTION_VOLUME_UP == sink_action
            || BT_SINK_SRV_ACTION_VOLUME_DOWN == sink_action)) {
        apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action);
        return ret;
    }
#endif

#if 0	// richard for bug64(from Airoha) defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    uint32_t a2dp_conn_num = 0;
    uint32_t ull_conn_num = 0;
    a2dp_conn_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK),
                                                NULL, 0);
    ull_conn_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL),
                                               NULL, 0);
    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_do_music_actions: a2dp_conn_num=%d, ull_conn_num=%d, mmi_state=%d",
                     3, a2dp_conn_num, ull_conn_num, apps_config_key_get_mmi_state());

    if (app_music_get_ull_is_streaming()
        || (ull_conn_num  != 0 && a2dp_conn_num == 0)
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        || (app_ull_is_le_ull_connected() && apps_config_key_get_mmi_state() == APP_CONNECTED)
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        || (app_ull_is_le_hid_connected() && apps_config_key_get_mmi_state() == APP_CONNECTED)
#endif
       ) {
        if (BT_SINK_SRV_ACTION_VOLUME_UP == sink_action
            || BT_SINK_SRV_ACTION_VOLUME_DOWN == sink_action) {
#ifdef AIR_MS_GIP_ENABLE
            if (app_dongle_service_get_dongle_mode() == APP_DONGLE_SERVICE_DONGLE_MODE_XBOX) {
#if defined(MTK_AWS_MCE_ENABLE)
                if (BT_AWS_MCE_ROLE_PARTNER == bt_device_manager_aws_local_info_get_role()) {
                    apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action);
                } else {
                    apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action);
                }
#else
                apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action);
#endif
            } else {
                app_music_set_ull_volume((sink_action == BT_SINK_SRV_ACTION_VOLUME_UP), 1);
            }
#else
            bool volume_up = (sink_action == BT_SINK_SRV_ACTION_VOLUME_UP) ? true : false;
            app_music_set_ull_volume(volume_up, 1);
#endif
            return ret;
        } else if (BT_SINK_SRV_ACTION_PLAY_PAUSE == sink_action
                   || BT_SINK_SRV_ACTION_PREV_TRACK == sink_action
                   || BT_SINK_SRV_ACTION_NEXT_TRACK == sink_action) {
            bt_ull_usb_hid_control_t control_param;
            if (BT_SINK_SRV_ACTION_PLAY_PAUSE == sink_action) {
                control_param = BT_ULL_USB_HID_PLAY_PAUSE_TOGGLE;
            } else if (BT_SINK_SRV_ACTION_PREV_TRACK == sink_action) {
                control_param = BT_ULL_USB_HID_PREVIOUS_TRACK;
            } else if (BT_SINK_SRV_ACTION_NEXT_TRACK == sink_action) {
                control_param = BT_ULL_USB_HID_NEXT_TRACK;
            }
            bt_ull_action(BT_ULL_ACTION_USB_HID_CONTROL, &control_param, sizeof(control_param));
            return ret;
        }
    }
#endif

#ifdef AIR_LE_AUDIO_BIS_ENABLE
    if (KEY_VOICE_UP == action || KEY_VOICE_DN == action) {
        bool is_bis_streaming = app_le_audio_bis_is_streaming();
        bt_sink_srv_am_volume_level_out_t volume = bt_sink_srv_cap_stream_get_broadcast_volume();

        if (is_bis_streaming) {
#ifdef MTK_AWS_MCE_ENABLE
            if (!from_aws_data && bt_aws_mce_srv_get_link_type() != BT_AWS_MCE_SRV_LINK_NONE) {
                apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action);
            }
#endif
            if (KEY_VOICE_UP == action) {
                volume += 1;
                if (volume > AUD_VOL_OUT_LEVEL15) {
                    volume = AUD_VOL_OUT_LEVEL15;
                }
            } else if (KEY_VOICE_DN == action) {
                if (volume == AUD_VOL_OUT_LEVEL0) {
                    // ignore
                } else if (volume >= AUD_VOL_OUT_LEVEL15) {
                    volume = AUD_VOL_OUT_LEVEL14;
                } else {
                    volume -= 1;
                }
            }
            bt_sink_srv_cap_stream_set_broadcast_volume(volume);
            return ret;
        }
    }
#endif

#if defined(AIR_SPEAKER_ENABLE) && defined(MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR)
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    if (mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
        APPS_LOG_MSGID_E(APP_MUSIC_UTILS"[APP_SPK] do_music_actions, Disallow Music action by SPK Broadcast", 0);
        return false;
    }
#endif

#ifdef AIR_LE_AUDIO_ENABLE
    if (KEY_VOICE_UP == action
        || KEY_VOICE_DN == action
        || KEY_AVRCP_FORWARD == action
        || KEY_AVRCP_BACKWARD == action
        || KEY_AVRCP_PLAY == action
        || KEY_AVRCP_PAUSE == action) {
        bool send_ret = app_music_send_le_audio_aird_action(action, 1);
        if (send_ret) {
            return ret;
        }
    }
#endif

    if (ret) {
#ifdef AIR_GSOUND_ENABLE
        bool gsound_ret = app_gsound_srv_handle_bt_sink_action(sink_action);
        if (gsound_ret) {
            /* Gsound app may intercept this action, refer to gsound app for more details. */
            ret = false;
        } else
#endif
        {
            if (op_valid) {
                bt_status = bt_sink_srv_send_action(sink_action, &op);
            } else {
#ifdef APP_MUSIC_ADJUST_VOLUME_VIA_ADDRESS_ENABLE
                if (sink_action == BT_SINK_SRV_ACTION_VOLUME_UP
                    || sink_action == BT_SINK_SRV_ACTION_VOLUME_DOWN) {
#if defined(MTK_AWS_MCE_ENABLE)
                    /* Partner can not send volume address action to middleware for control music, so relay to agent handle.*/
                    if ((BT_AWS_MCE_ROLE_PARTNER == bt_device_manager_aws_local_info_get_role())
                        && (TRUE == app_home_screen_idle_activity_is_aws_connected())) {
                        apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action);
                        return ret;
                    } else
#endif
                    {
                        bt_sink_srv_action_volume_t volume_addr = {0};
                        bool get_volume_addr_ret = app_music_get_active_device((uint8_t *)&(volume_addr.type), &(volume_addr.address));
                        if (get_volume_addr_ret) {
#ifdef AIR_LE_AUDIO_ENABLE
                            if (volume_addr.type == BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE) {
                                bt_handle_t le_conn_handle = bt_gap_le_srv_get_conn_handle_by_address(&volume_addr.address);
                                bool is_support = app_le_audio_aird_client_is_support(le_conn_handle);
                                APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_do_music_action: le_conn_handle=0x%x, is_support=%d",
                                                 2, le_conn_handle, is_support);
                                if (le_conn_handle != BT_HANDLE_INVALID && is_support) {
                                    app_le_audio_aird_action_t aird_action = (sink_action == BT_SINK_SRV_ACTION_VOLUME_UP ? APP_LE_AUDIO_AIRD_ACTION_SET_STREAMING_VOLUME_UP : APP_LE_AUDIO_AIRD_ACTION_SET_STREAMING_VOLUME_DOWN);
                                    app_le_audio_aird_action_set_streaming_volume_t param;
                                    param.streaming_interface = APP_LE_AUDIO_AIRD_STREAMING_INTERFACE_SPEAKER;
                                    param.streaming_port = APP_LE_AUDIO_AIRD_STREAMING_PORT_0;
                                    param.channel = APP_LE_AUDIO_AIRD_CHANNEL_DUAL;
                                    param.volume = 1;   /* The delta value. */

                                    app_le_audio_aird_client_send_action(le_conn_handle, aird_action,
                                                                         &param, sizeof(app_le_audio_aird_action_set_streaming_volume_t));
                                    return ret;
                                }
                            }
#endif
                            bt_sink_srv_send_action(sink_action, &volume_addr);
                            return ret;
                        }
                    }

                }
#endif
                bt_status = app_music_send_actions_by_address(sink_action);
            }
        }
    }
    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_do_music_action ret=%d, avrcp_op_sta=%d, op=0x%x, sink_action=0x%08X, bt_status=0x%x",
                     5, ret, local_context->avrcp_op_sta, op, sink_action, bt_status);
    return ret;
}

#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
extern bool app_hid_call_existing(void);
#endif
apps_config_key_action_t app_music_utils_proc_key_events(ui_shell_activity_t *self,
                                                         uint32_t event_id,
                                                         void *extra_data,
                                                         size_t data_len)
{
    uint8_t key_id;
    airo_key_event_t key_event;
    bool ret = false;

    /* Decode event_id to key_id and key_event. */
    app_event_key_event_decode(&key_id, &key_event, event_id);

    apps_config_key_action_t action;

    if (extra_data) {
        action = *(uint16_t *)extra_data;
    } else {
        action = apps_config_key_event_remapper_map_action(key_id, key_event);
    }

#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
    if (app_hid_call_existing()
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        && (action != KEY_VOICE_UP) && (action != KEY_VOICE_DN)
#endif
       ) {
        APPS_LOG_MSGID_I(APP_MUSIC_UTILS" hid call exist, do not process key event.", 0);
        action = KEY_ACTION_INVALID;
    }
#endif

    if (!app_music_get_curr_link_is_connected()) {
        action = KEY_ACTION_INVALID;
    }

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    if (action == KEY_ULL_UL_VOL_UP || action == KEY_ULL_UL_VOL_DN) {
        if (true == app_music_ull2_uplink_enable) {
            bt_status_t bt_status = BT_STATUS_FAIL;
            bool volume_up = (action == KEY_ULL_UL_VOL_UP) ? true : false;
            if (BT_ULL_MIC_CLIENT ==  bt_ull_le_srv_get_client_type()) {
                bt_status = app_music_set_ull_link_volume(BT_ULL_STREAMING_INTERFACE_LINE_OUT, volume_up, 1);
            } else {
                bt_status = app_music_set_ull_link_volume(BT_ULL_STREAMING_INTERFACE_MICROPHONE, volume_up, 1);
            }
            if (bt_status == BT_STATUS_SUCCESS) {
                 return action;
            }
        } else {
            action = KEY_ACTION_INVALID;
        }
    }
#endif

    if (KEY_ACTION_INVALID != action) {
        ret = app_music_do_music_actions(FALSE, self, action);
    }

    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" process key event: action=0x%x, ret=%d.", 2, action, ret);

    if (ret) {
        return action;
    } else {
        return KEY_ACTION_INVALID;
    }
}

bool app_music_idle_proc_bt_sink_events(ui_shell_activity_t *self, uint32_t event_id,
                                        void *extra_data, size_t data_len)
{
    bool ret = false;
    apps_music_local_context_t *local_context = (apps_music_local_context_t *)self->local_context;
    if (local_context == NULL) {
        return ret;
    }

#ifdef AIR_LE_AUDIO_BIS_ENABLE
    /* Don't process sink state and avrcp state during BIS streaming. */
    if (app_le_audio_bis_is_streaming()) {
        return ret;
    }
#endif

    if (event_id == BT_SINK_SRV_EVENT_STATE_CHANGE) {
        bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *) extra_data;
        APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_idle_proc_bt_sink_events param->pre=0x%x,  param->now=0x%x, is_playing=%d, isAutoPaused=%d",
                         4, param->previous, param->current, local_context->music_playing, local_context->isAutoPaused);
        /* Try to start app_music_activity when the music starts playing. */
        if ((param->previous != BT_SINK_SRV_STATE_STREAMING) && (param->current == BT_SINK_SRV_STATE_STREAMING)) {
            /* If music_playing is true, it indicates that the app_music_activity already exists. */
//            if (!local_context->music_playing) {
//                local_context->music_playing = true;
//                local_context->isAutoPaused = false;
//                ui_shell_start_activity(self, app_music_activity_proc, ACTIVITY_PRIORITY_MIDDLE, local_context, 0);
//            }
            local_context->music_streaming_state |= APP_MUSIC_STEAMING_STATE_SINK_STATE;
        } else if ((param->previous == BT_SINK_SRV_STATE_STREAMING) && (param->current != BT_SINK_SRV_STATE_STREAMING)) {
//            local_context->music_playing = false;
            local_context->music_streaming_state &= ~APP_MUSIC_STEAMING_STATE_SINK_STATE;
        }
    }
#if defined(MTK_IN_EAR_FEATURE_ENABLE)
    else if (event_id == BT_SINK_SRV_EVENT_AVRCP_STATUS_CHANGE) {
        bt_sink_srv_event_param_t *event = (bt_sink_srv_event_param_t *)extra_data;
        bt_sink_srv_state_t bt_sink_state = bt_sink_srv_get_state();
        bt_avrcp_status_t avrcp_status = event->avrcp_status_change.avrcp_status;
        APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_idle_proc_bt_sink_events: avrcp_status=0x%x, sink_state=0x%x",
                         2, avrcp_status, bt_sink_state);
        if (BT_AVRCP_STATUS_PLAY_PLAYING == avrcp_status) {
            app_music_add_avrcp_status(event);
//            if ((!local_context->music_playing) && (bt_sink_state < BT_SINK_SRV_STATE_INCOMING)) {
//                local_context->isAutoPaused = false;
//                local_context->music_playing = true;
//                ui_shell_start_activity(self, app_music_activity_proc, ACTIVITY_PRIORITY_MIDDLE, local_context, 0);
//            }
            local_context->music_streaming_state |= APP_MUSIC_STEAMING_STATE_AVRCP_STATE;
        } else if ((BT_AVRCP_STATUS_PLAY_PAUSED == avrcp_status) || (BT_AVRCP_STATUS_PLAY_STOPPED == avrcp_status)) {
            app_music_remove_avrcp_status(&(event->avrcp_status_change.address));
            app_music_avrcp_status_t *local_avrcp_status = app_music_get_avrcp_status();
            if (local_avrcp_status->avrcp_num == 0) {
                //local_context->music_playing = false;
                local_context->music_streaming_state &= ~APP_MUSIC_STEAMING_STATE_AVRCP_STATE;
            }
        }
#ifdef MTK_AWS_MCE_ENABLE
        if (BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()
            && BT_AWS_MCE_ROLE_AGENT == bt_connection_manager_device_local_info_get_aws_role()) {
            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                           APPS_EVENTS_INTERACTION_SYNC_BT_AVRCP_STATUS_TO_PEER,
                                           event, sizeof(bt_sink_srv_event_param_t));
        }
#endif
    }
#endif
    if (event_id == BT_SINK_SRV_EVENT_STATE_CHANGE
        || event_id == BT_SINK_SRV_EVENT_AVRCP_STATUS_CHANGE) {
        APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_idle_proc_bt_sink_events music_streaming_state=0x%x,music_playing=0x%x",
                         2, local_context->music_streaming_state,local_context->music_playing);
        if (local_context->music_streaming_state != 0 && !local_context->music_playing) {
            local_context->music_playing = true;
            local_context->isAutoPaused = false;
            APPS_LOG_MSGID_W(APP_MUSIC_UTILS" app_music_idle_proc_bt_sink_events ui_shell_start_activity 11 app_music_activity_proc",0);
            ui_shell_start_activity(self, app_music_activity_proc, ACTIVITY_PRIORITY_MIDDLE, local_context, 0);
        } else if (local_context->music_streaming_state == 0) {
            local_context->music_playing = false;
        }
    }

    return ret;
}

bool app_music_idle_proc_bt_cm_events(ui_shell_activity_t *self, uint32_t event_id,
                                      void *extra_data, size_t data_len)
{
    bool ret = false;
#ifdef MTK_AWS_MCE_ENABLE
    apps_music_local_context_t *local_context = (apps_music_local_context_t *)self->local_context;
    bt_aws_mce_role_t role;
    role = bt_device_manager_aws_local_info_get_role();
#endif

    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
#ifdef MTK_AWS_MCE_ENABLE
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (NULL == local_context || NULL == remote_update) {
                break;
            }

            if (BT_AWS_MCE_ROLE_AGENT == role || BT_AWS_MCE_ROLE_NONE == role) {
                if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                    && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    /* Update the partner connection state when AWS is disconnected. */
                    local_context->isPartnerConnected = false;
                    local_context->isPartnerCharging = false;
#if !defined(AIR_SPEAKER_ENABLE)
                    app_music_checkAudioState(local_context);
#endif
                } else if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                           && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    /* Update the partner connection state when AWS is connected. */
                    local_context->isPartnerConnected = true;
#ifdef MTK_IN_EAR_FEATURE_ENABLE
                    app_music_notify_state_to_peer();
#endif
                }
                if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP) & remote_update->pre_connected_service)
                    && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP) & remote_update->connected_service)) {
                    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_proc_bt_cm_event avrcp disconnect", 0);
                    app_music_remove_avrcp_status(&(remote_update->address));
                    app_music_avrcp_status_t *local_avrcp_status = app_music_get_avrcp_status();
                    if (local_avrcp_status->avrcp_num == 0) {
                        local_context->music_streaming_state &= ~APP_MUSIC_STEAMING_STATE_AVRCP_STATE;
                    }
                }
            }
#ifndef AIR_SPEAKER_ENABLE
            else if (BT_AWS_MCE_ROLE_PARTNER == role) {
                if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                    && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    /* For partner, set music mixing mode to STEREO when AWS is connected. */
                    am_dynamic_change_channel(AUDIO_CHANNEL_SELECTION_STEREO);
                    local_context->currMixState = MUSIC_STEREO;
                }
                if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                    && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
#ifdef AIR_MCSYNC_SHARE_ENABLE
                    if (app_share_get_pairing_result()) {
                        return ret;
                    }
#endif
                    am_dynamic_change_channel(AUDIO_CHANNEL_SELECTION_MONO);
                    local_context->currMixState = MUSIC_MONO;
                }
            }
#endif
#endif
        }
        break;
        default:
            break;
    }
    return ret;
}

#ifdef MTK_AWS_MCE_ENABLE
bool app_music_idle_proc_aws_data_events(ui_shell_activity_t *self, uint32_t event_id,
                                         void *extra_data, size_t data_len)
{
    bool ret = false;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
    apps_music_local_context_t *local_context = (apps_music_local_context_t *)self->local_context;
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();

#ifdef MTK_IN_EAR_FEATURE_ENABLE
    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group;
        uint32_t action;
        void *p_extra_data = NULL;
        uint32_t extra_data_len = 0;

        apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &action,
                                         &p_extra_data, &extra_data_len);
        if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
            && action == APPS_EVENTS_INTERACTION_SYNC_MUSIC_IN_EAR_CFG) {
            uint8_t music_in_ear_cfg = *(uint8_t *)p_extra_data;
            app_music_set_in_ear_control(music_in_ear_cfg, false);
        } else if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
                   && action == APPS_EVENTS_INTERACTION_SYNC_BT_AVRCP_STATUS_TO_PEER) {
            bt_sink_srv_event_param_t *avrcp_data = (bt_sink_srv_event_param_t *)p_extra_data;
            if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role()) {
        APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_proc_aws_data_events call app_music_idle_proc_bt_sink_events", 0);
                app_music_idle_proc_bt_sink_events(self, BT_SINK_SRV_EVENT_AVRCP_STATUS_CHANGE, avrcp_data, extra_data_len);
            }
        } else if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
                   && action == APPS_EVENTS_INTERACTION_SYNC_RECORDED_AVRCP_STATUS_TO_PEER) {
            app_music_avrcp_status_t *record_avrcp_data = (app_music_avrcp_status_t *)p_extra_data;
            if (record_avrcp_data == NULL) {
                return ret;
            }
            if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role()) {
                memcpy(&s_app_music_avrcp_status, record_avrcp_data, sizeof(app_music_avrcp_status_t));
                if (s_app_music_avrcp_status.avrcp_num != 0) {
                    bt_sink_srv_state_t bt_sink_state = bt_sink_srv_get_state();
                    if ((!local_context->music_playing) && (bt_sink_state < BT_SINK_SRV_STATE_INCOMING && bt_sink_state >= BT_SINK_SRV_STATE_CONNECTED)) {
                        for (uint8_t i = 0; i < APP_MUSIC_AVRCP_RECORD_MAX_NUM; i++) {
                            if (s_app_music_avrcp_status.avrcp_device[i].is_playing) {
                                local_context->isAutoPaused = false;
                                local_context->music_playing = true;
            //APPS_LOG_MSGID_W(APP_MUSIC_UTILS" app_music_idle_proc_bt_sink_events app_music_idle_proc_aws_data_events 22 app_music_activity_proc",0);
            APPS_LOG_MSGID_W(APP_MUSIC_UTILS"  app_music_idle_proc_aws_data_events ui_shell_start_activity 22 app_music_activity_proc",0);
                                ui_shell_start_activity(self, app_music_activity_proc, ACTIVITY_PRIORITY_MIDDLE, local_context, 0);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
#endif

    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group;
        uint32_t action;
        apps_aws_sync_event_decode(aws_data_ind, &event_group, &action);
        if (event_group == EVENT_GROUP_UI_SHELL_KEY) {
            if (role == BT_AWS_MCE_ROLE_AGENT
#ifdef AIR_LE_AUDIO_BIS_ENABLE
                || (app_le_audio_bis_is_streaming() && (KEY_VOICE_UP == action || KEY_VOICE_DN == action))
#endif
               ) {
                ret = app_music_do_music_actions(TRUE, self, action);
            }
        }
    }

    uint8_t battery = *(uint8_t *)aws_data_ind->param;
    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_BATTERY
        && role == BT_AWS_MCE_ROLE_AGENT) {
        /* Partner's battery app will report this event when AWS is connected. */
        APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_proc_aws_data_events Partner charge percent=%d.", 1, battery);

        local_context->isPartnerConnected = true;
#if 0
        app_smcharger_in_out_t is_charging = APP_SMCHARGER_NONE;
        is_charging = app_smcharger_peer_is_charging();
        local_context->isPartnerCharging = (is_charging == APP_SMCHARGER_IN) ? true : false;
#elif defined(MTK_BATTERY_MANAGEMENT_ENABLE)
        local_context->isPartnerCharging = battery & PARTNER_BATTERY_CHARGING ? true : false;
#endif
        /* Update music mixing mode when partner's battery info updated. */
#if !defined(AIR_SPEAKER_ENABLE)
        app_music_checkAudioState(local_context);
#endif
    }
    return ret;
}

#if !defined(AIR_SPEAKER_ENABLE)
void app_music_checkAudioState(apps_music_local_context_t *cntx)
{

    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_checkAudioState, currMixState=%d, isPartner_conn=%d, isChagring=%d,isSame=%d",
                     4, cntx->currMixState, cntx->isPartnerConnected, cntx->isPartnerCharging, cntx->isSame);

#ifdef AIR_MCSYNC_SHARE_ENABLE
    if (app_share_get_pairing_result()) {
        return;
    }
#endif

    if (cntx->isPartnerConnected && !cntx->isPartnerCharging
#if  defined(MTK_IN_EAR_FEATURE_ENABLE)
        && cntx->isSame
#endif
       ) {
        /* Set music mixing mode to STEREO when left and right earbuds are in ear. */
        if (cntx->currMixState != MUSIC_STEREO) {
            am_dynamic_change_channel(AUDIO_CHANNEL_SELECTION_STEREO);
            cntx->currMixState = MUSIC_STEREO;
        }
    } else {
        /* Set music mixing mode to MONO when the partner may not be in the ear. */
        if (cntx->currMixState != MUSIC_MONO) {
            am_dynamic_change_channel(AUDIO_CHANNEL_SELECTION_MONO);
            cntx->currMixState = MUSIC_MONO;
        }
    }
}
#endif /*AIR_SPEAKER_ENABLE*/
#endif /*MTK_AWS_MCE_ENABLE*/

#ifdef AIR_GSOUND_ENABLE
bool app_music_idle_proc_gsound_reject_action(ui_shell_activity_t *self, bt_sink_srv_action_t sink_action)
{
    apps_music_local_context_t *local_ctx = (apps_music_local_context_t *)self->local_context;
    bool ret = false;
    bt_status_t bt_status = BT_STATUS_SUCCESS;

    /* Handle the actions that were intercepted by gsound but not processed. */
    switch (sink_action) {
        case BT_SINK_SRV_ACTION_PLAY_PAUSE:
            bt_status = app_music_send_actions_by_address(sink_action);

//            bt_status = bt_sink_srv_send_action(sink_action, NULL);
            if (bt_status == BT_STATUS_SUCCESS) {
                if (!local_ctx->music_playing) {
                    local_ctx->isAutoPaused = false;
            APPS_LOG_MSGID_W(APP_MUSIC_UTILS" app_music_idle_proc_gsound_reject_action ui_shell_start_activity 33 app_music_activity_proc",0);
                    ui_shell_start_activity(self, app_music_activity_proc, ACTIVITY_PRIORITY_MIDDLE, local_ctx, 0);
                    local_ctx->music_playing = true;
                }
            }
            ret = true;
            break;
        case BT_SINK_SRV_ACTION_NEXT_TRACK:
        case BT_SINK_SRV_ACTION_PREV_TRACK:
            bt_sink_srv_send_action(sink_action, NULL);
            ret = true;
            break;
        default:
            break;
    }

    return ret;
}
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
/**
* @brief      This function is used to resume music when the earbud was put into the ear.
* @param[in]  self, the context pointer of the activity.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_music_idle_check_and_start_music(struct _ui_shell_activity *self, void *extra_data)
{
    apps_music_local_context_t *ctx = (apps_music_local_context_t *)self->local_context;
    app_in_ear_sta_info_t *sta_info = (app_in_ear_sta_info_t *)extra_data;
    uint8_t temp_music_in_ear_config = app_music_get_in_ear_control_state();
    bool avrcp_is_playing = (ctx->music_streaming_state & APP_MUSIC_STEAMING_STATE_AVRCP_STATE) ? true : false;

    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" check_and_start_music, cur=%d, pre=%d, isAutoPaused=%d, avrcp_is_playing=%d, in_ear_config=%d",
                     5, sta_info->current, sta_info->previous, ctx->isAutoPaused, avrcp_is_playing, temp_music_in_ear_config);

    /* Resume music when the earbud was put into the ear. */
    #ifdef EASTECH_IRSENSER_ADVANCED_CONTROL
    if (sta_info->previous != APP_IN_EAR_STA_BOTH_IN && sta_info->current != APP_IN_EAR_STA_BOTH_OUT) 
    #else
    if (ctx->isAutoPaused && sta_info->previous != APP_IN_EAR_STA_BOTH_IN && sta_info->current != APP_IN_EAR_STA_BOTH_OUT) 
    #endif
      {
        //if (!avrcp_is_playing) {
            if (APP_MUSIC_IN_EAR_AUTO_PAUSE_RESUME == temp_music_in_ear_config) {
                bt_status_t bt_status = app_music_send_actions_by_address(BT_SINK_SRV_ACTION_PLAY);
                if (bt_status == BT_STATUS_SUCCESS) {
                    ctx->isAutoPaused = false;
                    //ui_shell_start_activity(self, app_music_activity_proc, ACTIVITY_PRIORITY_MIDDLE, ctx, 0);
                    APPS_LOG_MSGID_I(APP_MUSIC_UTILS"  auto resume music", 0);
                    //ctx->music_playing = true;
                }
            }
        //}
    }

    /*
     * this is a special case. it is means that the old Agent send the play action and start the shell activity, but the new Agent
     * still in pause status because of the SINK_SRV not report the event about the music to play status.
     */
    if (sta_info->current != APP_IN_EAR_STA_BOTH_IN && sta_info->previous != APP_IN_EAR_STA_BOTH_OUT) {
        if (avrcp_is_playing) {
            if ((APP_MUSIC_IN_EAR_AUTO_PAUSE_RESUME == temp_music_in_ear_config)
                || (APP_MUSIC_IN_EAR_ONLY_AUTO_PAUSE == temp_music_in_ear_config)) {
                bt_status_t bt_status = bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PAUSE, NULL);
                if (bt_status == BT_STATUS_SUCCESS) {
                    //ctx->music_playing = false;
                    ctx->isAutoPaused = true;
                    APPS_LOG_MSGID_I(APP_MUSIC_UTILS"  isAutoPaused", 0);
                }
            }
        }
    }
    return false;
}
#endif /*MTK_IN_EAR_FEATURE_ENABLE*/

bool app_music_idle_proc_apps_internal_events(ui_shell_activity_t *self,
                                              uint32_t event_id,
                                              void *extra_data,
                                              size_t data_len)
{
    bool ret = false;

    switch (event_id) {
#ifdef MTK_IN_EAR_FEATURE_ENABLE
        case APPS_EVENTS_INTERACTION_IN_EAR_UPDATE_STA: {
            /* The event come from in ear detection app. */
            app_in_ear_sta_info_t *sta_info = (app_in_ear_sta_info_t *)extra_data;
            if (sta_info->previous != sta_info->current) {
                app_music_in_ear_update_mix_state(self, extra_data);
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                if (app_music_get_ull_is_streaming()) {
                    break;
                } else
#endif
                    ret = app_music_idle_check_and_start_music(self, extra_data);
            }
            break;
        }
        case APPS_EVENTS_INTERACTION_IN_EAR_CFG_UPDATE: {
            uint8_t *music_in_ear_cfg = (uint8_t *)extra_data;
            if (music_in_ear_cfg == NULL) {
                break;
            }
            if (0 == *music_in_ear_cfg) {
                app_music_set_in_ear_control(APP_MUSIC_IN_EAR_DISABLE, false);
            } else if (1 == *music_in_ear_cfg) {
                app_music_set_in_ear_control(APP_MUSIC_IN_EAR_AUTO_PAUSE_RESUME, false);
            }
            break;
        }
#endif
#ifdef AIR_GSOUND_ENABLE
        case APPS_EVENTS_INTERACTION_GSOUND_ACTION_REJECTED:
            /* Handle actions related to music control that were intercepted by gsound but not processed. */
            if (!app_music_get_curr_link_is_connected()) {
                break;
            }
            ret = app_music_idle_proc_gsound_reject_action(self, (bt_sink_srv_action_t)extra_data);
            break;
#endif
        default:
            break;
    }

    return ret;
}

#ifdef MTK_IN_EAR_FEATURE_ENABLE
/**
* @brief      This function is used to obtain the status of the function of controlling music through in ear detection.
* @return     Return the configuration of music in ear.
*/
uint8_t app_music_get_in_ear_control_state()
{
    nvkey_status_t status = NVKEY_STATUS_OK;
    uint8_t temp_in_ear_cfg;
    uint32_t data_size = sizeof(uint8_t);
    if (APP_MUSIC_IN_EAR_NONE != g_music_in_ear_config) {
        temp_in_ear_cfg =  g_music_in_ear_config;
    } else {
        status = nvkey_read_data(NVID_APP_IN_EAR_MUSIC_ABILITY, &temp_in_ear_cfg, &data_size);
        if (NVKEY_STATUS_ITEM_NOT_FOUND == status) {
            temp_in_ear_cfg = APP_MUSIC_IN_EAR_AUTO_PAUSE_RESUME;
            status = nvkey_write_data(NVID_APP_IN_EAR_MUSIC_ABILITY, &temp_in_ear_cfg, sizeof(uint8_t));
            if (NVKEY_STATUS_OK == status) {
                g_music_in_ear_config = temp_in_ear_cfg;
            }
        }
    }
    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" [IN EAR] get music in ear default cfg success:status=%d, temp_in_ear_cfg=%d",
                     2, status, temp_in_ear_cfg);
    return temp_in_ear_cfg;
}


/**
* @brief      This function is used to set the function of controlling music through ear detection.
* @param[in]  music_in_ear_cfg, music in ear configuration.
* @param[in]  isSync, whether sync music_in_ear_cfg to other side. false for RACE CMD, true for the other application.
* @return     If return true, set successfully.
*/
bool app_music_set_in_ear_control(uint8_t music_in_ear_cfg, bool isSync)
{
    bool ret = false;
    nvkey_status_t status = NVKEY_STATUS_OK;
    status = nvkey_write_data(NVID_APP_IN_EAR_MUSIC_ABILITY, &music_in_ear_cfg, sizeof(uint8_t));
    if (status == NVKEY_STATUS_OK) {
        ret = true;
        g_music_in_ear_config = music_in_ear_cfg;
#ifdef MTK_AWS_MCE_ENABLE
        if ((TRUE == isSync) && (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role())) {
            app_music_notify_state_to_peer();
        }
#endif
    }
    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" [IN EAR] set in ear control: ret=%d, isSync=%d, status=%d, music_in_ear_cfg=%d",
                     4, ret, isSync, status, music_in_ear_cfg);
    return ret;
}

void app_music_notify_state_to_peer()
{
#ifdef MTK_AWS_MCE_ENABLE
    uint8_t music_in_ear_cfg = app_music_get_in_ear_control_state();
    if (TRUE == app_home_screen_idle_activity_is_aws_connected()) {
        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                       APPS_EVENTS_INTERACTION_SYNC_MUSIC_IN_EAR_CFG,
                                       &music_in_ear_cfg, sizeof(uint8_t));
        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                       APPS_EVENTS_INTERACTION_SYNC_RECORDED_AVRCP_STATUS_TO_PEER,
                                       &s_app_music_avrcp_status, sizeof(app_music_avrcp_status_t));
    }
#endif
}

bool app_music_in_ear_update_mix_state(struct _ui_shell_activity *self, void *extra_data)
{
#ifdef MTK_AWS_MCE_ENABLE
    apps_music_local_context_t *ctx = (apps_music_local_context_t *)self->local_context;
    app_in_ear_sta_info_t *sta_info = (app_in_ear_sta_info_t *)extra_data;
    if (sta_info->current == APP_IN_EAR_STA_AIN_POUT || sta_info->current == APP_IN_EAR_STA_AOUT_PIN) {
        ctx->isSame = false;
    } else {
        ctx->isSame = true;
    }
    //if (sta_info->current != APP_IN_EAR_STA_BOTH_OUT) {
#if !defined(AIR_SPEAKER_ENABLE)
        app_music_checkAudioState(ctx);
#endif
    //}
#endif
    return false;
}
#endif  /*MTK_IN_EAR_FEATURE_ENABLE*/

bool app_music_set_volume_value(bool isUp, uint32_t value)
{
    bool ret = FALSE;
    bt_status_t bt_status = BT_STATUS_FAIL;
    uint8_t volume = 0;

    bt_sink_srv_state_t bt_sink_state = bt_sink_srv_get_state();
    if (bt_sink_state >= BT_SINK_SRV_STATE_INCOMING) {
        volume = bt_sink_srv_get_volume(NULL, BT_SINK_SRV_VOLUME_HFP);
        if (isUp == TRUE) {
            if (volume + value < AUD_VOL_OUT_MAX) {
                volume += value;
            } else {
                volume = AUD_VOL_OUT_LEVEL15;
            }
        } else {
            if (volume > value) {
                volume -= value;
            } else {
                volume = 0;
            }
        }
        bt_status = bt_sink_srv_send_action(BT_SINK_SRV_ACTION_CALL_SET_VOLUME, &volume);
    } else {
        volume = bt_sink_srv_get_volume(NULL, BT_SINK_SRV_VOLUME_A2DP);
        if (isUp == TRUE) {
            if (volume + value < bt_sink_srv_ami_get_a2dp_max_volume_level()) {
                volume += value;
            } else {
                volume = bt_sink_srv_ami_get_a2dp_max_volume_level();
            }
        } else {
            if (volume > value) {
                volume -= value;
            } else {
                volume = 0;
            }
        }
        bt_status = bt_sink_srv_send_action(BT_SINK_SRV_ACTION_SET_VOLUME, &volume);
    }

    if (bt_status == BT_STATUS_SUCCESS) {
        ret = TRUE;
    }
    APPS_LOG_MSGID_I(APP_MUSIC_UTILS"app_music_set_volume_value: ret=%d, isUp=%d, value=%d, volume_level=%d, bt_sink_state=%x",
                     5, ret, isUp, value, volume, bt_sink_state);
    return ret;
}

bool app_music_get_curr_link_is_connected(void)
{
    bool ret = true;

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
    if (app_ull_is_le_ull_connected()) {
        APPS_LOG_MSGID_I(APP_MUSIC_UTILS" ull le connected.", 0);
        return ret;
    }
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
    if (app_ull_is_le_hid_connected()) {
        APPS_LOG_MSGID_I(APP_MUSIC_UTILS" ull le hid connected.", 0);
        return ret;
    }
#endif
#ifndef MTK_AWS_MCE_ENABLE
    if ((0 == bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL), NULL, 0))) {
#ifdef AIR_LE_AUDIO_ENABLE
        bt_handle_t le_handle = bt_sink_srv_cap_check_links_state(BT_SINK_SRV_CAP_STATE_CONNECTED);
        bool is_bis_streaming = bt_sink_srv_cap_stream_is_broadcast_streaming();
        if (!is_bis_streaming && BT_HANDLE_INVALID == le_handle) {
            APPS_LOG_MSGID_I(APP_MUSIC_UTILS" NOT connected A2DP/ULL/CIS/BIS, ignore KEY Action", 0);
            return false;
        }
#else
        APPS_LOG_MSGID_I(APP_MUSIC_UTILS" NOT connected A2DP or ULL, ignore KEY Action", 0);
        return false;
#endif
    }
#endif

    return ret;
}


bool app_music_get_ull_is_streaming(void)
{
    bool ret = false;

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    if (app_music_ull2_uplink_enable || app_music_ull2_downlink_enable) {
        ret = true;
    }
#endif
    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" get_ull_is_streaming: ret=%d", 1, ret);
    return ret;
}



#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
static bt_status_t app_music_set_ull_link_volume(bt_ull_streaming_interface_t interface, bool volume_up, uint32_t volume)
{
    bt_status_t ret              = BT_STATUS_FAIL;
    bt_ull_volume_t volume_param;
    volume_param.streaming.streaming_interface = interface;
    volume_param.streaming.port = 0;
    volume_param.channel = BT_ULL_VOLUME_CHANNEL_DUEL;
    volume_param.volume = volume; /* The delta value. */
    if (volume_up) {
        volume_param.action = BT_ULL_VOLUME_ACTION_SET_UP;
    } else {
        volume_param.action = BT_ULL_VOLUME_ACTION_SET_DOWN;
    }

    ret = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &volume_param, sizeof(bt_ull_volume_t));

    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" set_ull_link_volume: interface=0x%x, volume_up=%d, volume=%d, ret=%d",
                     4, interface, volume_up, volume, ret);
    return ret;
}
#endif

bool app_music_set_ull_volume(bool volume_up, uint32_t volume)
{
    bool ret              = false;
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    bt_status_t bt_status = BT_STATUS_FAIL;

    bt_status = app_music_set_ull_link_volume(BT_ULL_STREAMING_INTERFACE_SPEAKER, volume_up, volume);

    if (bt_status == BT_STATUS_SUCCESS) {
        ret = true;
    }
#endif
    return ret;
}

bool app_music_send_le_audio_aird_action(apps_config_key_action_t action, uint32_t volume)
{
    bool ret = false;
#ifdef AIR_LE_AUDIO_ENABLE
    bt_handle_t handle = BT_HANDLE_INVALID;
    if (KEY_AVRCP_PLAY == action) {
        handle = g_music_active_handle.handle;
    } else {
        handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(bt_sink_srv_cap_am_get_current_mode());
    }

    bool is_support = app_le_audio_aird_client_is_support(handle);
    app_le_audio_aird_action_t aird_action;
    if (handle != BT_HANDLE_INVALID && is_support) {
        if (KEY_VOICE_UP == action || KEY_VOICE_DN == action) {
            app_le_audio_aird_action_set_streaming_volume_t param;
            param.streaming_interface = APP_LE_AUDIO_AIRD_STREAMING_INTERFACE_SPEAKER;
            param.streaming_port      = APP_LE_AUDIO_AIRD_STREAMING_PORT_0;
            param.channel             = APP_LE_AUDIO_AIRD_CHANNEL_DUAL;
            param.volume              = volume;   /* The delta value. */
            aird_action = (action == KEY_VOICE_UP) ? APP_LE_AUDIO_AIRD_ACTION_SET_STREAMING_VOLUME_UP : APP_LE_AUDIO_AIRD_ACTION_SET_STREAMING_VOLUME_DOWN;
            app_le_audio_aird_client_send_action(handle, aird_action,
                                                 &param, sizeof(app_le_audio_aird_action_set_streaming_volume_t));
            ret = true;
        } else if (KEY_AVRCP_FORWARD == action || KEY_AVRCP_BACKWARD == action) {
            aird_action = (action == KEY_AVRCP_FORWARD ? APP_LE_AUDIO_AIRD_ACTION_NEXT_TRACK : APP_LE_AUDIO_AIRD_ACTION_PREVIOUS_TRACK);
            app_le_audio_aird_client_send_action(handle, aird_action, NULL, 0);
            ret = true;
        } else if (KEY_AVRCP_PLAY == action || KEY_AVRCP_PAUSE == action) {
            const audio_src_srv_handle_t *current_device = audio_src_srv_get_runing_pseudo_device();
            if (current_device == NULL
                || (current_device->type != AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP && current_device->type != AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP)) {
                APPS_LOG_MSGID_I(APP_MUSIC_UTILS" [LEA] AIRD_Client TOGGLE PLAY, curr_type=%d", 1, (current_device == NULL) ? 0xFF : current_device->type);
                app_le_audio_aird_client_send_action(handle, APP_LE_AUDIO_AIRD_ACTION_TOGGLE_PLAY,
                                                     NULL, 0);
                ret = true;
            }
        }
    }
    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" send_le_audio_aird_action: handle=0x%x, is_support=%d",
                     2, handle, is_support);
#endif
    return ret;
}

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE

bool app_music_get_ull2_is_call()
{
    return ((true == app_music_ull2_uplink_enable) && (true == app_music_ull2_downlink_enable));
}

bool app_music_get_ull2_is_music()
{
    return ((false == app_music_ull2_uplink_enable) && (true == app_music_ull2_downlink_enable));
}

void app_music_clear_ull2_link_state()
{
    app_music_ull2_uplink_enable = false;
    app_music_ull2_downlink_enable = false;
}


void app_music_update_ull2_link_state(uint32_t event_id, bt_ull_le_streaming_start_ind_t *bt_ull2_link_type)
{
    if (bt_ull2_link_type == NULL) {
        APPS_LOG_MSGID_I(APP_MUSIC_UTILS" app_music_update_ull2_link_state: bt_ull2_link_type is NULL", 0);
        return;
    }
    if (BT_ULL_EVENT_LE_STREAMING_START_IND == event_id) {
        if (BT_ULL_LE_STREAM_MODE_UPLINK == bt_ull2_link_type->stream_mode) {
            app_music_ull2_uplink_enable = true;
        }
        if (BT_ULL_LE_STREAM_MODE_DOWNLINK == bt_ull2_link_type->stream_mode) {
            app_music_ull2_downlink_enable = true;
        }
    }
    if (BT_ULL_EVENT_LE_STREAMING_STOP_IND == event_id) {
        if (BT_ULL_LE_STREAM_MODE_UPLINK == bt_ull2_link_type->stream_mode) {
            app_music_ull2_uplink_enable = false;
        }
        if (BT_ULL_LE_STREAM_MODE_DOWNLINK == bt_ull2_link_type->stream_mode) {
            app_music_ull2_downlink_enable = false;
        }
    }
    APPS_LOG_MSGID_I(APP_MUSIC_UTILS" event_id=%d ull2_link_type=%d uplink_enable=%d downlink_enable=%d"
                     , 4, event_id, bt_ull2_link_type->stream_mode, app_music_ull2_uplink_enable, app_music_ull2_downlink_enable);
}


#endif

#ifdef AIR_ROTARY_ENCODER_ENABLE
bool app_music_proc_rotary_event_group(ui_shell_activity_t *self,
                                       uint32_t event_id,
                                       void *extra_data,
                                       size_t data_len)
{
    bool ret = false;
    bsp_rotary_encoder_port_t port;
    bsp_rotary_encoder_event_t event;
    uint32_t rotary_data;
    if (!extra_data) {
        return ret;
    }
    apps_config_key_action_t key_action = *(uint16_t *)extra_data;
    app_event_rotary_event_decode(&port, &event, &rotary_data, event_id);
    switch (key_action) {
        case KEY_VOICE_UP:
        case KEY_VOICE_DN: {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            if (app_music_get_ull_is_streaming()) {
                bool volume_up = (key_action == KEY_VOICE_UP) ? true : false;
                ret = app_music_set_ull_volume(volume_up, rotary_data);
            } else
#endif
            {
                if (app_music_send_le_audio_aird_action(key_action, rotary_data)) {
                } else {
                    uint8_t volume = bt_sink_srv_get_volume(NULL, BT_SINK_SRV_VOLUME_A2DP);
                    if (KEY_VOICE_UP == key_action) {
                        if (volume + rotary_data < bt_sink_srv_ami_get_a2dp_max_volume_level()) {
                            volume += rotary_data;
                        } else {
                            volume = bt_sink_srv_ami_get_a2dp_max_volume_level();
                        }
                    } else {
                        if (volume > rotary_data) {
                            volume -= rotary_data;
                        } else {
                            volume = 0;
                        }
                    }
                    APPS_LOG_MSGID_I(APP_MUSIC_ACTI" app_music_proc_rotary_event_group, music volume : %d", 1, volume);
                    bt_sink_srv_send_action(BT_SINK_SRV_ACTION_SET_VOLUME, &volume);
                }
            }
            ret = true;
            break;
        }
        default:
            break;
    }

    return ret;
}
#endif
