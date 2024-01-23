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

#include "app_speaker_idle_activity.h"

#include "bt_app_common.h"
#include "bt_aws_mce.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_customer_config.h"
#include "bt_device_manager.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_a2dp.h"
#include "bt_aws_mce_srv.h"

#include "apps_aws_sync_event.h"
#include "app_bt_state_service.h"
#include "apps_config_key_remapper.h"
#include "apps_config_vp_index_list.h"
#include "apps_debug.h"
#include "apps_events_bt_event.h"
#include "apps_events_event_group.h"
#include "apps_events_key_event.h"
#include "apps_events_interaction_event.h"
#include "app_home_screen_idle_activity.h"
#include "app_speaker_le_association.h"
#include "app_speaker_srv.h"

#include "atci.h"
#include "FreeRTOS.h"
#include "voice_prompt_api.h"
#include "ui_shell_manager.h"
#include "multi_ble_adv_manager.h"
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_lea_service.h"
#include "app_lea_service_conn_mgr.h"
#include "app_lea_service_event.h"
#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_le_audio_bis.h"
#include "bt_sink_srv_le_cap_stream.h"
#endif
#include "ble_csis.h"
#include "ble_pacs.h"
#endif
#ifdef APPS_LINE_IN_SUPPORT
#include "app_line_in_idle_activity.h"
#endif
#ifdef APPS_USB_AUDIO_SUPPORT
#include "app_usb_audio_idle_activity.h"
#endif
#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#include "app_bt_takeover_service.h"
#endif
#include "app_bt_conn_manager.h"
#include "apps_config_led_manager.h"
#include "apps_config_led_index_list.h"


#define LOG_TAG             "[APP_SPK]"

#ifdef MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR
#define APP_SPEAKER_RECONNECT_OTHER_PROFILE_TIMER           500
#endif

typedef enum {
    APP_SPEAKER_SWITCH_STATE_NONE = 0,
    APP_SPEAKER_SWITCH_STATE_DISCONNECTING,
    APP_SPEAKER_SWITCH_STATE_PAIRING,
    APP_SPEAKER_SWITCH_STATE_ASSOCIATION_DONE,
    APP_SPEAKER_SWITCH_STATE_MODE_SWITCHED,
    APP_SPEAKER_SWITCH_STATE_FAIL,
} app_speaker_switch_state_t;

typedef struct {
    bt_aws_mce_srv_mode_t                       target_mode;
    bt_aws_mce_role_t                           target_role;
    app_speaker_switch_state_t                  switch_state;
} app_speaker_context_t;

bool g_app_wait_to_scan = FALSE;
static app_speaker_context_t                    app_speaker_ctx = {0};



/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
#define STRNCPY(dest, source) strncpy(dest, source, strlen(source)+1);

static void app_speaker_play_vp(uint32_t vp_index)
{
    voice_prompt_param_t vp = {0};
    vp.vp_index = vp_index;
    voice_prompt_play(&vp, NULL);
}

static bool app_speaker_check_single_mode(bt_aws_mce_srv_mode_t mode)
{
    return (mode == BT_AWS_MCE_SRV_MODE_NORMAL || mode == BT_AWS_MCE_SRV_MODE_SINGLE);
}

static void app_speaker_switch_to_single(void)
{
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    bool success = app_speaker_le_ass_switch_to_single();
    app_speaker_ctx.target_mode = BT_AWS_MCE_SRV_MODE_SINGLE;
    app_speaker_ctx.target_role = BT_AWS_MCE_ROLE_NONE;

    if (success && (mode == BT_AWS_MCE_SRV_MODE_NORMAL || mode == BT_AWS_MCE_SRV_MODE_SINGLE)) {
        APPS_LOG_MSGID_I(LOG_TAG" switch_to_single, already SINGLE continue", 0);
        app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_NONE;
#ifdef AIR_LE_AUDIO_ENABLE
        app_lea_conn_mgr_control_temp_reconnect_type(TRUE);
        app_lea_service_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, FALSE, 0);
#endif
    }
}

static void app_speaker_switch_start(void)
{
    if (app_speaker_ctx.switch_state == APP_SPEAKER_SWITCH_STATE_NONE) {
        APPS_LOG_MSGID_I(LOG_TAG" app_speaker_switch_start, not in switch status", 0);
        return;
    }
    app_speaker_le_ass_set_param(app_speaker_ctx.target_role, app_speaker_ctx.target_mode);
    if (app_speaker_ctx.target_role == BT_AWS_MCE_ROLE_PARTNER || app_speaker_ctx.target_role == BT_AWS_MCE_ROLE_CLINET) {
        app_speaker_le_ass_action(BT_AWS_MCE_LE_ASSOCIATION_APP_START_SCAN, NULL);
    } else {
        app_speaker_le_ass_action(BT_AWS_MCE_LE_ASSOCIATION_APP_START_ADV, NULL);
    }
}

#ifdef MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR
static void app_speaker_control_music_play(bt_aws_mce_srv_mode_t pre_mode, bt_aws_mce_role_t pre_role)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    //uint32_t a2dp = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK), NULL, 0);
    if (role == BT_AWS_MCE_ROLE_AGENT) {
        if ((pre_mode == BT_AWS_MCE_SRV_MODE_DOUBLE || pre_mode == BT_AWS_MCE_SRV_MODE_SINGLE || pre_role != BT_AWS_MCE_ROLE_AGENT) && mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
            bt_sink_srv_music_audio_switch(FALSE, BT_SINK_SRV_ACTION_PAUSE);
        } else if (pre_mode == BT_AWS_MCE_SRV_MODE_BROADCAST && (mode == BT_AWS_MCE_SRV_MODE_SINGLE || mode == BT_AWS_MCE_SRV_MODE_DOUBLE)) {
            bt_sink_srv_music_audio_switch(TRUE, BT_SINK_SRV_ACTION_PLAY);
        }
    }
}

static void app_speaker_control_broadcast_edr(void)
{
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    if (mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
        app_bt_conn_mgr_enable_edr_profile(FALSE);
    } else {
        app_bt_conn_mgr_enable_edr_profile(TRUE);
    }
}
#endif

#ifdef AIR_LE_AUDIO_ENABLE
extern void ble_csis_write_nvkey_sirk(bt_key_t *sirk);

static void app_speaker_lea_change_sirk(void)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_key_t sirk = {0};
    bt_app_common_generate_random_key((uint8_t *)sirk, sizeof(bt_key_t));
    ble_csis_set_sirk(sirk);
    ble_csis_write_nvkey_sirk(&sirk);
    APPS_LOG_MSGID_W(LOG_TAG" lea_change_sirk, partner->[%02X] SIRK=%02X:%02X:%02X:%02X",
                     5, role, sirk[0], sirk[1], sirk[2], sirk[3]);
}

static void app_speaker_lea_switch_channel(void)
{
    extern bt_status_t ble_csis_set_coordinated_set_size(uint8_t size);
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    APPS_LOG_MSGID_W(LOG_TAG"[LEA] switch_channel, mode=%d role=%02X", 2, mode, role);

    if (mode == BT_AWS_MCE_SRV_MODE_DOUBLE) {
        ble_csis_set_coordinated_set_size(2);
        ble_pacs_switch_pac_record(1);
        if (role == BT_AWS_MCE_ROLE_AGENT) {
            ble_pacs_set_audio_location(AUDIO_DIRECTION_SINK, AUDIO_LOCATION_FRONT_LEFT);
            ble_pacs_set_audio_location(AUDIO_DIRECTION_SOURCE, AUDIO_LOCATION_FRONT_LEFT);
        } else if (role == BT_AWS_MCE_ROLE_PARTNER) {
            ble_pacs_set_audio_location(AUDIO_DIRECTION_SINK, AUDIO_LOCATION_FRONT_RIGHT);
            ble_pacs_set_audio_location(AUDIO_DIRECTION_SOURCE, AUDIO_LOCATION_FRONT_RIGHT);
        }
        ble_pacs_send_sink_location_notify(0xFFFF);
        ble_pacs_send_source_location_notify(0xFFFF);
    } else {
        ble_csis_set_coordinated_set_size(1);
        ble_pacs_switch_pac_record(2);
        ble_pacs_set_audio_location(AUDIO_DIRECTION_SINK, AUDIO_LOCATION_FRONT_LEFT | AUDIO_LOCATION_FRONT_RIGHT);
        ble_pacs_set_audio_location(AUDIO_DIRECTION_SOURCE, AUDIO_LOCATION_FRONT_LEFT);
        ble_pacs_send_sink_location_notify(0xFFFF);
        ble_pacs_send_source_location_notify(0xFFFF);
    }
    // bt_sink_srv_am_result_t am_dynamic_change_channel(audio_channel_selection_t change_channel)
}
#endif

static bool app_speaker_check_control_emp(void)
{
    bool success = FALSE;
#ifdef AIR_MULTI_POINT_ENABLE
    bool enable = FALSE;
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_link_type_t link_type = bt_aws_mce_srv_get_link_type();
    if (mode == BT_AWS_MCE_SRV_MODE_SINGLE && (role == BT_AWS_MCE_ROLE_NONE || role == BT_AWS_MCE_ROLE_AGENT)) {
        enable = TRUE;
        success = app_bt_emp_switch_enable(enable);
    } else if (mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
        enable = FALSE;
        success = app_bt_emp_switch_enable(enable);
        app_bt_takeover_service_disconnect_one();
    } else if (mode == BT_AWS_MCE_SRV_MODE_DOUBLE) {
        enable = TRUE;
        success = app_bt_emp_switch_enable(enable);
    }
    if (success) {
        APPS_LOG_MSGID_I(LOG_TAG" check_control_emp, [%02X] mode=%d link_type=%d enable=%d",
                         4, role, mode, link_type, enable);
    }
#endif
    return success;
}

static void app_speaker_check_disconnect_done(void)
{
    // EDR Keep connection when switch mode, but partner/client need to disconnect AWS
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (app_speaker_ctx.switch_state == APP_SPEAKER_SWITCH_STATE_DISCONNECTING
        && (role == BT_AWS_MCE_ROLE_NONE || role == BT_AWS_MCE_ROLE_AGENT || (bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE, NULL, 0) == 0))
#ifdef AIR_LE_AUDIO_ENABLE
        && app_lea_conn_mgr_get_conn_num() == 0
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        && !bt_sink_srv_cap_stream_is_broadcast_streaming()
        && !bt_sink_srv_cap_stream_is_scanning_broadcast_source()
#endif
#endif
       ) {
        APPS_LOG_MSGID_I(LOG_TAG" disconnect_done, target_mode=%d target_role=%02X",
                         2, app_speaker_ctx.target_mode, app_speaker_ctx.target_role);
        if (app_speaker_ctx.target_mode == BT_AWS_MCE_SRV_MODE_SINGLE) {
            app_speaker_switch_to_single();
        } else {
            app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_PAIRING;
            app_speaker_switch_start();
        }
    }
}

static bool app_speaker_disconnect_all(void)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    // EDR Keep connection when switch mode, but partner/client need to disconnect AWS
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    uint32_t aws_link = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), NULL, 0);
    bool need_disconnect_edr = FALSE;
    if ((role == BT_AWS_MCE_ROLE_PARTNER || role == BT_AWS_MCE_ROLE_CLINET) && aws_link > 0) {
        need_disconnect_edr = TRUE;
    }

    if (need_disconnect_edr) {
        bt_status = app_bt_conn_mgr_disconnect_edr(NULL, FALSE);
    }

    // For LEA, both disconnect all LEA Link
#ifdef AIR_LE_AUDIO_ENABLE
    app_lea_service_quick_stop_adv();
    if (app_lea_conn_mgr_get_conn_num() > 0) {
        app_lea_service_disconnect(FALSE, APP_LE_AUDIO_DISCONNECT_MODE_ALL, NULL,
                                   BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_POWER_OFF);
        bt_status = BT_STATUS_SUCCESS;
    }

#ifdef AIR_LE_AUDIO_BIS_ENABLE
    bool success = app_le_audio_bis_start(FALSE);
    if (bt_status == BT_STATUS_FAIL) {
        bt_status = (success ? BT_STATUS_SUCCESS : BT_STATUS_FAIL);
    }
#endif
#endif

    APPS_LOG_MSGID_I(LOG_TAG" disconnect_all, [%02X] aws_link=%d bt_status=0x%08X",
                     3, role, aws_link, bt_status);
    return (bt_status == BT_STATUS_SUCCESS);
}

static void app_speaker_print_mode(bt_aws_mce_srv_mode_t mode, bt_aws_mce_role_t role)
{
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        return;
    }
    memset(response, 0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;

    if (mode == BT_AWS_MCE_SRV_MODE_SINGLE) {
        STRNCPY((char *)response->response_buf, "\r\n+Mode Changed: Single mode, ");
    } else if (mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
        STRNCPY((char *)response->response_buf, "\r\n+Mode Changed: Broadcast mode, ");
    } else if (mode == BT_AWS_MCE_SRV_MODE_DOUBLE) {
        STRNCPY((char *)response->response_buf, "\r\n+Mode Changed: Double mode, ");
    }

    if (role == BT_AWS_MCE_ROLE_NONE) {
        strcat((char *)response->response_buf, "None role.\r\n");
    } else if (role == BT_AWS_MCE_ROLE_AGENT) {
        strcat((char *)response->response_buf, "Agent role.\r\n");
    } else if (role == BT_AWS_MCE_ROLE_CLINET) {
        strcat((char *)response->response_buf, "Client role.\r\n");
    } else if (role == BT_AWS_MCE_ROLE_PARTNER) {
        strcat((char *)response->response_buf, "Partner role.\r\n");
    }

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
}

static void app_speaker_print_ignore_switch_mode(bool ignore_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_URC_FORMAT};
    if (ignore_cmd) {
        STRNCPY((char *)response.response_buf, "switch_mode, ignore CMD\r\n");
    } else {
        STRNCPY((char *)response.response_buf, "Already in the target mode, ignore cmd.\r\n");
        APPS_LOG_MSGID_E(LOG_TAG" key_event, Already in the target mode", 0);
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
}

static bool app_speaker_ignore_switch_key_action(void)
{
    bool is_ignore = FALSE;
    apps_config_state_t mmi_state = apps_config_key_get_mmi_state();
    if (app_speaker_ctx.switch_state != APP_SPEAKER_SWITCH_STATE_NONE) {
        APPS_LOG_MSGID_E(LOG_TAG" ignore_switch_key_action, switch ongoing", 0);
        is_ignore = TRUE;
    } else if (mmi_state >= APP_HFP_INCOMING && mmi_state <= APP_HFP_MULTIPARTY_CALL) {
        APPS_LOG_MSGID_E(LOG_TAG" ignore_switch_key_action, call ongoing", 0);
        is_ignore = TRUE;
    }
#ifdef APPS_LINE_IN_SUPPORT
    else if (app_line_in_activity_get_current_audio_path() == APP_AUDIO_PATH_LINE_IN) {
        APPS_LOG_MSGID_E(LOG_TAG" ignore_switch_key_action, LINE IN", 0);
        is_ignore = TRUE;
    }
#endif
#ifdef APPS_USB_AUDIO_SUPPORT
    else if (app_usb_in_is_open()) {
        APPS_LOG_MSGID_E(LOG_TAG" ignore_switch_key_action, USB IN", 0);
        is_ignore = TRUE;
    }
#endif

    if (is_ignore) {
        app_speaker_print_ignore_switch_mode(TRUE);
    }

    return is_ignore;
}



/**================================================================================*/
/**                                 APP Event Handler                              */
/**================================================================================*/
static bool app_speaker_proc_ui_shell_event(uint32_t event_id,
                                            void *extra_data,
                                            size_t data_len)
{
    bool ret = TRUE; // UI shell internal event must process by this activity, so default is TRUE

    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
            APPS_LOG_MSGID_I(LOG_TAG"  CREATE, mode=%d role=%02X", 2, mode, role);
#ifndef MTK_BATTERY_MANAGEMENT_ENABLE
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT, (void *)TRUE, 0, NULL, 0);
#endif
            break;
        }
        default:
            break;
    }
    return ret;
}

static bool app_speaker_proc_key_event(uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = FALSE;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    uint8_t key_id = 0;
    airo_key_event_t key_event = 0;
    apps_config_key_action_t action = 0;

    if (extra_data) {
        action = *(uint16_t *)extra_data;
    } else {
        app_event_key_event_decode(&key_id, &key_event, event_id);
        action = apps_config_key_event_remapper_map_action(key_id, key_event);
    }

    if (action == KEY_BROADCAST_AGENT || action == KEY_BROADCAST_CLIENT || action == KEY_DOUBLE_AGENT
        || action == KEY_DOUBLE_PARTNER || action == KEY_SPK_SINGLE || action == KEY_SPK_UNGROUP) {
        ret = TRUE;
        if ((action == KEY_BROADCAST_AGENT || action == KEY_BROADCAST_CLIENT
             || action == KEY_DOUBLE_AGENT || action == KEY_DOUBLE_PARTNER) && app_speaker_ignore_switch_key_action()) {
            APPS_LOG_MSGID_E(LOG_TAG" key_event, ignore action=0x%08X", 1, action);
            goto exit;
        }
    } else {
        goto exit;
    }

    switch (action) {
        case KEY_BROADCAST_AGENT: {
            APPS_LOG_MSGID_I(LOG_TAG" key_event, KEY_BROADCAST_AGENT mode=%d role=%02X", 2, mode, role);
            app_speaker_play_vp(VP_INDEX_PAIRING);
            app_speaker_ctx.target_mode = BT_AWS_MCE_SRV_MODE_BROADCAST;
            app_speaker_ctx.target_role = BT_AWS_MCE_ROLE_AGENT;
            app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_PAIRING;

            if ((role == BT_AWS_MCE_ROLE_AGENT && mode == BT_AWS_MCE_SRV_MODE_BROADCAST)
                || (role == BT_AWS_MCE_ROLE_NONE && apps_config_key_get_mmi_state() < APP_CONNECTED)) {
                // do nothing
            } else {
                if (app_speaker_disconnect_all()) {
                    app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_DISCONNECTING;
                    break;
                }
            }

            app_speaker_le_ass_set_param(BT_AWS_MCE_ROLE_AGENT, BT_AWS_MCE_SRV_MODE_BROADCAST);
            app_speaker_le_ass_action(BT_AWS_MCE_LE_ASSOCIATION_APP_START_ADV, NULL);
            break;
        }

        case KEY_BROADCAST_CLIENT: {
            APPS_LOG_MSGID_I(LOG_TAG" key_event, KEY_BROADCAST_CLIENT mode=%d role=%02X aws_link=%d",
                             3, mode, role, aws_link_type);
            if (mode == BT_AWS_MCE_SRV_MODE_BROADCAST
                && role == BT_AWS_MCE_ROLE_CLINET
                && BT_AWS_MCE_SRV_LINK_NONE != aws_link_type) {
                app_speaker_print_ignore_switch_mode(FALSE);
                break;
            } else {
                app_speaker_play_vp(VP_INDEX_PAIRING);
                app_speaker_ctx.target_mode = BT_AWS_MCE_SRV_MODE_BROADCAST;
                app_speaker_ctx.target_role = BT_AWS_MCE_ROLE_CLINET;
                app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_PAIRING;

                if (role != BT_AWS_MCE_ROLE_NONE || apps_config_key_get_mmi_state() >= APP_CONNECTED) {
                    if (app_speaker_disconnect_all()) {
                        app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_DISCONNECTING;
                        break;
                    }
                }

                app_speaker_le_ass_set_param(BT_AWS_MCE_ROLE_CLINET, BT_AWS_MCE_SRV_MODE_BROADCAST);
                app_speaker_le_ass_action(BT_AWS_MCE_LE_ASSOCIATION_APP_START_SCAN, NULL);
            }
            break;
        }

        case KEY_DOUBLE_AGENT: {
            APPS_LOG_MSGID_I(LOG_TAG" key_event, KEY_DOUBLE_AGENT mode=%d role=%02X", 2, mode, role);
            if (mode == BT_AWS_MCE_SRV_MODE_DOUBLE && role == BT_AWS_MCE_ROLE_AGENT) {
                app_speaker_print_ignore_switch_mode(FALSE);
                break;
            } else {
                app_speaker_play_vp(VP_INDEX_PAIRING);
                app_speaker_ctx.target_mode = BT_AWS_MCE_SRV_MODE_DOUBLE;
                app_speaker_ctx.target_role = BT_AWS_MCE_ROLE_AGENT;
                app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_PAIRING;

                if (role != BT_AWS_MCE_ROLE_NONE || apps_config_key_get_mmi_state() >= APP_CONNECTED) {
                    if (app_speaker_disconnect_all()) {
                        app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_DISCONNECTING;
                        break;
                    }
                }

                app_speaker_le_ass_set_param(BT_AWS_MCE_ROLE_AGENT, BT_AWS_MCE_SRV_MODE_DOUBLE);
                app_speaker_le_ass_action(BT_AWS_MCE_LE_ASSOCIATION_APP_START_ADV, NULL);
            }
            break;
        }

        case KEY_DOUBLE_PARTNER: {
            APPS_LOG_MSGID_I(LOG_TAG" key_event, KEY_DOUBLE_PARTNER mode=%d role=%02X aws_link=%d",
                             3, mode, role, aws_link_type);
            if (mode == BT_AWS_MCE_SRV_MODE_DOUBLE
                && role == BT_AWS_MCE_ROLE_PARTNER
                && BT_AWS_MCE_SRV_LINK_NONE != aws_link_type) {
                app_speaker_print_ignore_switch_mode(FALSE);
                break;
            } else {
                app_speaker_play_vp(VP_INDEX_PAIRING);
                app_speaker_ctx.target_mode = BT_AWS_MCE_SRV_MODE_DOUBLE;
                app_speaker_ctx.target_role = BT_AWS_MCE_ROLE_PARTNER;
                app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_PAIRING;

                if (role != BT_AWS_MCE_ROLE_NONE || apps_config_key_get_mmi_state() >= APP_CONNECTED) {
                    if (app_speaker_disconnect_all()) {
                        app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_DISCONNECTING;
                        break;
                    }
                }

                app_speaker_le_ass_set_param(BT_AWS_MCE_ROLE_PARTNER, BT_AWS_MCE_SRV_MODE_DOUBLE);
                app_speaker_le_ass_action(BT_AWS_MCE_LE_ASSOCIATION_APP_START_SCAN, NULL);
            }
            break;
        }

        case KEY_SPK_SINGLE: {
            APPS_LOG_MSGID_I(LOG_TAG" key_event, KEY_SPK_SINGLE mode=%d role=%02X", 2, mode, role);
            if (app_speaker_check_single_mode(mode)) {
                app_speaker_print_ignore_switch_mode(FALSE);
                break;
            }

            if ((mode == BT_AWS_MCE_SRV_MODE_DOUBLE || mode == BT_AWS_MCE_SRV_MODE_BROADCAST)
                && apps_config_key_get_mmi_state() >= APP_CONNECTED) {
                if (app_speaker_disconnect_all()) {
                    app_speaker_ctx.target_mode = BT_AWS_MCE_SRV_MODE_SINGLE;
                    app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_DISCONNECTING;
                    break;
                }
            }

            app_speaker_switch_to_single();
            break;
        }

        case KEY_SPK_UNGROUP: {
            APPS_LOG_MSGID_I(LOG_TAG" key_event, KEY_SPK_UNGROUP mode=%d role=%02X", 2, mode, role);
            if (app_speaker_check_single_mode(mode)) {
                app_speaker_print_ignore_switch_mode(FALSE);
                break;
            }

            app_speaker_le_ass_action(BT_AWS_MCE_LE_ASSOCIATION_APP_UNGROUP, NULL);
            app_speaker_ctx.target_mode = BT_AWS_MCE_SRV_MODE_SINGLE;
            app_speaker_ctx.target_role = BT_AWS_MCE_ROLE_NONE;
            break;
        }
    }

exit:
    return ret;
}

static bool app_speaker_proc_aws_event(uint32_t event_id, void *extra_data, size_t data_len)
{
    bool need_reconnect = FALSE;
    if (BT_AWS_MCE_SRV_EVENT_MODE_CHANGED_IND == event_id) {
        bt_aws_mce_srv_mode_changed_ind_t *ind = (bt_aws_mce_srv_mode_changed_ind_t *)extra_data;
        if (ind == NULL) {
            APPS_LOG_MSGID_W(LOG_TAG" AWS event, MODE_CHANGED_IND no switch", 0);
            app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_NONE;
            return FALSE;
        }

        bt_aws_mce_srv_mode_t mode = ind->mode;
        bt_aws_mce_role_t role = ind->role;
        APPS_LOG_MSGID_W(LOG_TAG" AWS event, MODE_CHANGED_IND mode=%d->%d role=%02X->%02X target_mode=%d",
                         5, ind->pre_mode, mode, ind->pre_role, role, app_speaker_ctx.target_mode);
        app_speaker_print_mode(mode, role);
        if (ind->pre_mode != mode || ind->pre_role != role) {
            app_speaker_check_control_emp();
        }

        if (mode == BT_AWS_MCE_SRV_MODE_SINGLE && app_speaker_ctx.target_mode == BT_AWS_MCE_SRV_MODE_SINGLE) {
            if (app_speaker_ctx.switch_state != APP_SPEAKER_SWITCH_STATE_FAIL) {
                app_speaker_play_vp(VP_INDEX_SUCCEED);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                    NULL, 0);
            }
            app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_NONE;
            // Switch to single mode successfully for partner/client role, try to reconnect first EDR link
            if (ind->pre_role != BT_AWS_MCE_ROLE_AGENT) {
                need_reconnect = TRUE;
            }
        } else if (mode == BT_AWS_MCE_SRV_MODE_SINGLE && app_speaker_ctx.target_mode != BT_AWS_MCE_SRV_MODE_SINGLE) {
            APPS_LOG_MSGID_W(LOG_TAG" AWS event, MODE_CHANGED_IND mode=%d target_mode=%d Retry", 2, mode, app_speaker_ctx.target_mode);
            app_speaker_switch_start();
        } else {
            if (role == BT_AWS_MCE_ROLE_AGENT) {
                app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_NONE;
                if (mode == BT_AWS_MCE_SRV_MODE_DOUBLE || mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
                    apps_config_set_background_led_pattern(LED_INDEX_CHARGING_FULL, FALSE, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOW);
                }
            } else {
                app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_MODE_SWITCHED;
            }
        }

        if (need_reconnect && (role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_NONE)) {
            APPS_LOG_MSGID_I(LOG_TAG" AWS event, reconnect_edr for SINGLE", 0);
            app_bt_conn_mgr_reconnect_edr();
        } else if ((mode == BT_AWS_MCE_SRV_MODE_DOUBLE
#ifndef MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR
                    || mode == BT_AWS_MCE_SRV_MODE_BROADCAST
#endif
                   ) && ind->pre_role != BT_AWS_MCE_ROLE_AGENT && role == BT_AWS_MCE_ROLE_AGENT) {
            APPS_LOG_MSGID_I(LOG_TAG" AWS event, reconnect_edr for switch Agent", 0);
            app_bt_conn_mgr_reconnect_edr();
        }

#ifdef MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR
        if (ind->pre_mode != mode || ind->pre_role != role) {
            app_speaker_control_music_play(ind->pre_mode, ind->pre_role);
            app_speaker_control_broadcast_edr();
        }
#endif

#ifdef AIR_LE_AUDIO_ENABLE
        if ((ind->pre_role == BT_AWS_MCE_ROLE_PARTNER || ind->pre_role == BT_AWS_MCE_ROLE_CLINET)
            && mode == BT_AWS_MCE_SRV_MODE_SINGLE) {
            app_speaker_lea_change_sirk();
        }

        // For LEA, switch channel, both start TARGET_ALL ADV
        app_speaker_lea_switch_channel();

        bool visible = app_bt_service_is_visible();
        if (mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
            app_lea_service_stop_advertising(FALSE);
        } else if (visible) {
            app_lea_conn_mgr_control_temp_reconnect_type(TRUE);
            app_lea_service_start_advertising(APP_LEA_ADV_MODE_GENERAL, FALSE, APP_LE_AUDIO_ADV_TIME);
        } else if (mode == BT_AWS_MCE_SRV_MODE_DOUBLE && role == BT_AWS_MCE_ROLE_PARTNER) {
            app_lea_conn_mgr_control_temp_reconnect_type(TRUE);
            app_lea_service_start_advertising(APP_LEA_ADV_MODE_GENERAL, FALSE, APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME);
        } else {
            app_lea_conn_mgr_control_temp_reconnect_type(TRUE);
            app_lea_service_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, FALSE, 0);
        }
#endif
    }

    return FALSE;
}

static bool app_speaker_proc_le_association_event(uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = FALSE;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    switch (event_id) {
        case LE_ASSOCIATION_EVENT_ASSOCIATION_DONE: {
            APPS_LOG_MSGID_W(LOG_TAG" LE_ASS event, ASSOCIATION_DONE", 0);
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_ASSOCIATION, LE_ASSOCIATION_EVENT_TIMER_ASSOCIATION);
            app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_ASSOCIATION_DONE;
            break;
        }

        case LE_ASSOCIATION_EVENT_SCAN_TIMEOUT: {
            APPS_LOG_MSGID_W(LOG_TAG" LE_ASS event, SCAN_TIMEOUT", 0);
            app_speaker_play_vp(VP_INDEX_FAILED);

            app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_FAIL;
            app_speaker_switch_to_single();
            break;
        }

        case LE_ASSOCIATION_EVENT_ADV_TIMEOUT: {
            APPS_LOG_MSGID_W(LOG_TAG" LE_ASS event, ADV_TIMEOUT", 0);
            app_speaker_play_vp(VP_INDEX_FAILED);
            if (role != BT_AWS_MCE_ROLE_AGENT || mode != BT_AWS_MCE_SRV_MODE_BROADCAST) {
                app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_FAIL;
                app_speaker_switch_to_single();
            } else {
                app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_NONE;
            }
            break;
        }

        case LE_ASSOCIATION_EVENT_TIMER_ASSOCIATION: {
            APPS_LOG_MSGID_W(LOG_TAG" LE_ASS event, TIMER_ASSOCIATION", 0);
            if (app_speaker_ctx.target_role != BT_AWS_MCE_ROLE_AGENT) {
                app_speaker_play_vp(VP_INDEX_FAILED);
                if (role != BT_AWS_MCE_ROLE_AGENT || mode != BT_AWS_MCE_SRV_MODE_BROADCAST) {
                    app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_FAIL;
                    app_speaker_switch_to_single();
                } else {
                    app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_NONE;
                }
            }
            break;
        }

        default:
            break;
    }

    return ret;
}

static bool app_speaker_proc_bt_cm_event(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (NULL == remote_update) {
                break;
            }

            if (BT_CM_ACL_LINK_DISCONNECTED != remote_update->pre_acl_state && BT_CM_ACL_LINK_DISCONNECTED == remote_update->acl_state) {
                app_speaker_check_disconnect_done();
            } else if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                       && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                // only for partner/client
                if (app_speaker_ctx.switch_state == APP_SPEAKER_SWITCH_STATE_MODE_SWITCHED) {
                    app_speaker_play_vp(VP_INDEX_SUCCEED);
                    app_speaker_ctx.switch_state = APP_SPEAKER_SWITCH_STATE_NONE;
                }
                app_speaker_check_control_emp();
            }

#if defined(AIR_SPEAKER_ENABLE) && defined(MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR)
            bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            uint8_t *addr = (uint8_t *)remote_update->address;
            bool reconnect_profile_event = FALSE;

            if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AIR) & remote_update->pre_connected_service)
                && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AIR) & remote_update->connected_service)) {
                if (mode == BT_AWS_MCE_SRV_MODE_BROADCAST && role == BT_AWS_MCE_ROLE_AGENT) {
                    app_bt_conn_mgr_disconnect_edr(addr, FALSE);
                }
            } else if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->pre_connected_service)
                       && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->connected_service)) {
                bt_cm_profile_service_state_t avrcp = bt_cm_get_profile_service_state(addr, BT_CM_PROFILE_SERVICE_AVRCP);
                bt_cm_profile_service_state_t hfp = bt_cm_get_profile_service_state(addr, BT_CM_PROFILE_SERVICE_HFP);
                if (avrcp && !hfp) {
                    reconnect_profile_event = TRUE;
                }
            } else if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP) & remote_update->pre_connected_service)
                       && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP) & remote_update->connected_service)) {
                bt_cm_profile_service_state_t a2dp = bt_cm_get_profile_service_state(addr, BT_CM_PROFILE_SERVICE_A2DP_SINK);
                bt_cm_profile_service_state_t hfp = bt_cm_get_profile_service_state(addr, BT_CM_PROFILE_SERVICE_HFP);
                if (a2dp && !hfp) {
                    reconnect_profile_event = TRUE;
                }
            } else if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->pre_connected_service)
                       && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->connected_service)) {
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SPEAKER, APP_SPEAKER_EVENT_RECONNECT_OTHER_PROFILE);
            }

            if (reconnect_profile_event && mode != BT_AWS_MCE_SRV_MODE_BROADCAST && role == BT_AWS_MCE_ROLE_AGENT) {
                uint8_t *reconnect_addr = (uint8_t *)pvPortMalloc(BT_BD_ADDR_LEN);
                if (reconnect_addr == NULL) {
                    break;
                }
                memcpy(reconnect_addr, addr, BT_BD_ADDR_LEN);
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SPEAKER, APP_SPEAKER_EVENT_RECONNECT_OTHER_PROFILE);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_SPEAKER,
                                    APP_SPEAKER_EVENT_RECONNECT_OTHER_PROFILE, (void *)reconnect_addr,
                                    BT_BD_ADDR_LEN, NULL, APP_SPEAKER_RECONNECT_OTHER_PROFILE_TIMER);
            }
#endif
            break;
        }

        default:
            break;
    }

    return FALSE;
}

static bool app_speaker_proc_bt_dm_event(uint32_t event_id, void *extra_data, size_t data_len)
{
    bt_device_manager_power_event_t event = 0;
    bt_device_manager_power_status_t status = 0;
    bt_event_get_bt_dm_event_and_status(event_id, &event, &status);
    if (event == BT_DEVICE_MANAGER_POWER_EVT_ACTIVE_COMPLETE && status == BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS) {
        app_speaker_check_control_emp();
#ifdef AIR_LE_AUDIO_ENABLE
        app_speaker_lea_switch_channel();
#endif
#ifdef MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR
        app_speaker_control_broadcast_edr();
#endif
    }
    return FALSE;
}

static bool app_speaker_proc_bt_lea_event(uint32_t event_id, void *extra_data, size_t data_len)
{
#ifdef AIR_LE_AUDIO_ENABLE
    if (event_id == EVENT_ID_LE_AUDIO_DISCONNECT_DONE) {
        APPS_LOG_MSGID_I(LOG_TAG" LEA event, DISCONNECT_DONE", 0);
        app_speaker_check_disconnect_done();
    } else if (event_id == EVENT_ID_LE_AUDIO_BIS_STOP_STREAMING
               || event_id == EVENT_ID_LE_AUDIO_BIS_SCAN_STOPPED) {
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        APPS_LOG_MSGID_I(LOG_TAG" LEA event, BIS_STOP_STREAMING or BIS_SCAN_STOPPED", 0);
        app_speaker_check_disconnect_done();
#endif
    }
#endif
    return FALSE;
}

static bool app_speaker_proc_speaker_event(uint32_t event_id, void *extra_data, size_t data_len)
{
#if defined(AIR_SPEAKER_ENABLE) && defined(MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR)
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (event_id == APP_SPEAKER_EVENT_RECONNECT_OTHER_PROFILE
        && mode != BT_AWS_MCE_SRV_MODE_BROADCAST && role == BT_AWS_MCE_ROLE_AGENT) {
        uint8_t *addr = (uint8_t *)extra_data;
        uint32_t edr_profile = bt_customer_config_app_get_cm_config()->power_on_reconnect_profile;
        uint32_t other_profiles = edr_profile & ~((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK)
                                                   | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP)
                                                   | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AIR)
                                                   | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)));
        bt_cm_connect_t conn_param = {{0}, other_profiles};
        memcpy(conn_param.address, addr, sizeof(bt_bd_addr_t));
        bt_status_t bt_status = bt_cm_connect(&conn_param);
        APPS_LOG_MSGID_W(LOG_TAG" RECONNECT_OTHER_PROFILE, addr=%08X%04X other_profiles=0x%08X bt_status=0x%08X",
                         4, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), other_profiles, bt_status);
    }
#endif
    return FALSE;
}

static bool app_speaker_proc_aws_data_event(void *extra_data, size_t data_len)
{
    bool ret = FALSE;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;

    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group = 0;
        uint32_t event_id = 0;
        apps_aws_sync_event_decode(aws_data_ind, &event_group, &event_id);
        switch (event_group) {
            case EVENT_GROUP_UI_SHELL_LE_ASSOCIATION: {
                switch (event_id) {
                    case LE_ASSOCIATION_EVENT_AWS_MCE_UNGROUP:
                        APPS_LOG_MSGID_I(LOG_TAG" AWS_MCE event, UNGROUP", 0);
                        app_speaker_switch_to_single();
                        break;
                    default:
                        break;
                }
                ret = TRUE;
                break;
            }

            default:
                break;
        }
    }
    return ret;
}

/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
bool app_speaker_is_switch_mode_ongoing(void)
{
    return (app_speaker_ctx.switch_state == APP_SPEAKER_SWITCH_STATE_DISCONNECTING
            || app_speaker_ctx.switch_state == APP_SPEAKER_SWITCH_STATE_PAIRING);
}

bool app_speaker_idle_activity_proc(ui_shell_activity_t *self,
                                    uint32_t event_group,
                                    uint32_t event_id,
                                    void *extra_data,
                                    size_t data_len)
{
    bool ret = FALSE;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            ret = app_speaker_proc_ui_shell_event(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_KEY:
            ret = app_speaker_proc_key_event(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_AWS:
            ret = app_speaker_proc_aws_event(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_LE_ASSOCIATION:
            ret = app_speaker_proc_le_association_event(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            ret = app_speaker_proc_aws_data_event(extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER:
            ret = app_speaker_proc_bt_cm_event(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER:
            ret = app_speaker_proc_bt_dm_event(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_LE_AUDIO:
            ret = app_speaker_proc_bt_lea_event(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_SPEAKER:
            ret = app_speaker_proc_speaker_event(event_id, extra_data, data_len);
            break;

        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
#ifdef APPS_USB_AUDIO_SUPPORT
            if (event_id == APPS_EVENTS_INTERACTION_USB_PLUG_STATE) {
                bool plug_in = (bool)(uint32_t)extra_data;
                if (plug_in && (bt_aws_mce_srv_get_mode() != BT_AWS_MCE_SRV_MODE_SINGLE)
                    && (bt_aws_mce_srv_get_mode() != BT_AWS_MCE_SRV_MODE_NORMAL)) {
                    APPS_LOG_MSGID_I(LOG_TAG" usb in, switch to single", 0);
                    app_speaker_le_ass_action(BT_AWS_MCE_LE_ASSOCIATION_APP_UNGROUP, NULL);

                    app_speaker_ctx.target_mode = BT_AWS_MCE_SRV_MODE_SINGLE;
                    app_speaker_ctx.target_role = BT_AWS_MCE_ROLE_NONE;
                }
            }
#endif
#ifdef APPS_LINE_IN_SUPPORT
            if (event_id == APPS_EVENTS_INTERACTION_LINE_IN_PLUG_STATE) {
                bool plug_in = (bool)(uint32_t)extra_data;
                if (plug_in
                    && (bt_aws_mce_srv_get_mode() != BT_AWS_MCE_SRV_MODE_SINGLE)
                    && (bt_aws_mce_srv_get_mode() != BT_AWS_MCE_SRV_MODE_NORMAL)) {
                    APPS_LOG_MSGID_I(LOG_TAG" line in, switch to single", 0);
                    app_speaker_le_ass_action(BT_AWS_MCE_LE_ASSOCIATION_APP_UNGROUP, NULL);

                    app_speaker_ctx.target_mode = BT_AWS_MCE_SRV_MODE_SINGLE;
                    app_speaker_ctx.target_role = BT_AWS_MCE_ROLE_NONE;
                }
            }
#endif
            break;
        default:
            break;
    }
    return ret;
}
