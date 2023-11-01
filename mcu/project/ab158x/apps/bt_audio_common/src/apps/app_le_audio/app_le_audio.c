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

#ifdef AIR_LE_AUDIO_ENABLE

#include "app_le_audio.h"

#include "app_ccp.h"
#include "app_mcp.h"
#include "app_le_audio_aird_client.h"
#ifdef AIR_LE_AUDIO_MTK_HFP_AT_CMD
#include "app_le_audio_hfp_at_cmd.h"
#endif
#include "app_lea_service.h"
#include "app_lea_service_conn_mgr.h"
#include "app_lea_service_event.h"
#include "app_lea_service_sync_info.h"

#include "bt_type.h"
#include "bt_gap_le_audio.h"
#include "bt_gap_le.h"

#include "bt_le_audio_sink.h"

#include "bt_sink_srv.h"
#include "bt_sink_srv_le.h"
#include "bt_sink_srv_le_cap.h"
#include "bt_sink_srv_le_cap_audio_manager.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#include "apps_aws_sync_event.h"
#endif
#include "apps_debug.h"

#include "multi_ble_adv_manager.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "app_bt_state_service.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_events_bt_event.h"
#include "apps_config_features_dynamic_setting.h"
#include "bt_callback_manager.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_customer_config.h"
#include "bt_device_manager.h"
#include "bt_init.h"
#include "ui_shell_manager.h"
#include "bt_aws_mce_role_recovery.h"
#include "bt_gattc.h"
#include "bt_le_audio_sink.h"
#include "ble_pbp.h"

#ifdef AIR_TWS_ENABLE
#include "app_mps.h"
#endif
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
#include "app_power_save_utils.h"
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_le_audio_bis.h"
#endif
#ifdef AIR_LE_AUDIO_HAPS_ENABLE
#include "ble_haps.h"
#endif

#include "bt_avm.h"
#include "bt_device_manager_le.h"

/**************************************************************************************************
 * Define
**************************************************************************************************/
#define LOG_TAG     "[LEA][APP]"

/**************************************************************************************************
 * Enum
**************************************************************************************************/

/**************************************************************************************************
 * Variable
**************************************************************************************************/
static bool app_le_audio_clear_adv = false;

/**************************************************************************************************
 * Prototype
**************************************************************************************************/
extern void app_le_audio_dhss_sdp_init(void);
extern void bt_app_common_sync_le_audio_info(void);
extern bool bt_le_audio_sink_load_cccd(bt_handle_t handle);
extern void app_le_audio_dhss_proc_ui_shell_event(uint32_t event_group, uint32_t event_id, void *extra_data, size_t data_len);
/**************************************************************************************************
 * Static Functions
**************************************************************************************************/
#ifdef AIR_TWS_ENABLE
static void app_le_audio_send_battery_level_notification(void)
{
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        bt_handle_t handle = app_lea_conn_mgr_get_handle(i);
        if (handle != BT_HANDLE_INVALID) {
            app_le_audio_mps_send_battery(handle);
        }
    }
}
#endif

static bt_status_t app_le_audio_set_sniff(bool enable)
{
    bt_bd_addr_t aws_device;
    bt_status_t status = BT_STATUS_SUCCESS;
    uint32_t num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), &aws_device, 1);

    if (num > 0) {
        bt_gap_connection_handle_t gap_handle = bt_cm_get_gap_handle(aws_device);
        bt_gap_link_policy_setting_t setting;
        APPS_LOG_MSGID_I(LOG_TAG" set_sniff, enable=%d handle=0x%08X", 2, enable, gap_handle);
        if (!enable) {
            status = bt_gap_exit_sniff_mode(gap_handle);
        }

        setting.sniff_mode = enable ? BT_GAP_LINK_POLICY_ENABLE : BT_GAP_LINK_POLICY_DISABLE;
        status = bt_gap_write_link_policy(gap_handle, &setting);
    }
    return status;
}

static bt_status_t app_le_audio_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_GAP_LE_CIS_ESTABLISHED_IND: {
            bt_gap_le_cis_established_ind_t *ind = (bt_gap_le_cis_established_ind_t *) buff;
            APPS_LOG_MSGID_I(LOG_TAG" BT_GAP_LE_CIS_ESTABLISHED_IND, handle=0x%04X", 1, ind->connection_handle);
            //app_le_audio_set_sniff(FALSE);
            break;
        }

        case BT_GAP_LE_CIS_TERMINATED_IND: {
            bt_gap_le_cis_terminated_ind_t *ind = (bt_gap_le_cis_terminated_ind_t *) buff;
            APPS_LOG_MSGID_I(LOG_TAG" BT_GAP_LE_CIS_TERMINATED_IND, handle=0x%04X", 1, ind->connection_handle);
            app_le_audio_set_sniff(TRUE);
            break;
        }

        case BT_GAP_LE_EXT_ADVERTISING_REPORT_IND: {
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_ext_advertising_report_ind_t *ind = (bt_gap_le_ext_advertising_report_ind_t *)buff;
                uint16_t total_len = ind->data_length;
                uint8_t *data = ind->data;
                uint16_t uuid;
                uint16_t idx = 0;

                while (idx < total_len) {
                    if (data[idx] >= 3 && data[idx + 1] == BT_GAP_LE_AD_TYPE_SERVICE_DATA) {
                        uuid = (uint16_t) data[idx + 3] << 8 | data[idx + 2];
                        if (uuid == BT_PBP_UUID16_PUBLIC_BROADCAST_ANNOUNCEMENTS) {
                            APPS_LOG_MSGID_I(LOG_TAG"[PBP] PUBLIC_BROADCAST_ANNOUNCEMENTS find UUID=0x%04X SID=%02X type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X feature=%d",
                                             10, uuid, ind->advertising_SID, ind->address.type, ind->address.addr[5], ind->address.addr[4], ind->address.addr[3],
                                             ind->address.addr[2], ind->address.addr[1], ind->address.addr[0], data[idx + 4]);
                        }
                    }
#ifdef AIR_LE_AUDIO_PBP_ENABLE
                    else if (data[idx + 1] == BT_PBP_BROADCAST_NAME) {
                        uint8_t val = data[idx + data[idx] + 1];
                        data[idx + data[idx] + 1] = 0;
                        APPS_LOG_I(LOG_TAG"[PBP] BROADCAST_NAME = [%s]", &data[idx + 2]);
                        data[idx + data[idx] + 1] = val;
                    }
#endif
                    idx += (data[idx] + 1);
                }
            }
            break;
        }

        case BT_GATTC_DISCOVER_PRIMARY_SERVICE:
        case BT_GATTC_DISCOVER_PRIMARY_SERVICE_BY_UUID:
        case BT_GATTC_FIND_INCLUDED_SERVICES:
        case BT_GATTC_DISCOVER_CHARC:
        case BT_GATTC_DISCOVER_CHARC_DESCRIPTOR:
        case BT_GATTC_READ_CHARC:
        case BT_GATTC_READ_LONG_CHARC:
        case BT_GATTC_READ_USING_CHARC_UUID:
        case BT_GATTC_READ_MULTI_CHARC_VALUES:
        case BT_GATTC_WRITE_CHARC:
        case BT_GATTC_WRITE_LONG_CHARC:
        case BT_GATTC_RELIABLE_WRITE_CHARC:
        case BT_GATTC_CHARC_VALUE_NOTIFICATION: {
            app_le_audio_aird_client_event_handler(msg, status, buff);
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bool app_le_audio_proc_bt_cm_group(struct _ui_shell_activity *self,
                                          uint32_t event_id,
                                          void *extra_data,
                                          size_t data_len)
{
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (NULL == remote_update) {
                break;
            }

#if defined(MTK_AWS_MCE_ENABLE) || defined(AIR_INFORM_CONNECTION_STATUS_ENABLE)
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
#endif

#ifdef AIR_INFORM_CONNECTION_STATUS_ENABLE
            if (role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_NONE) {
                bt_bd_addr_t *local_addr = bt_device_manager_get_local_address();
                bool phone_related = (memcmp(remote_update->address, local_addr, sizeof(bt_bd_addr_t)) != 0);
                if (remote_update->pre_acl_state < BT_CM_ACL_LINK_CONNECTED
                    && remote_update->acl_state >= BT_CM_ACL_LINK_CONNECTED
                    && phone_related) {
                    app_le_audio_aird_client_infom_connection_status(TRUE);
                } else if (remote_update->pre_acl_state != BT_CM_ACL_LINK_DISCONNECTED
                           && remote_update->acl_state == BT_CM_ACL_LINK_DISCONNECTED
                           && phone_related) {
                    app_le_audio_aird_client_infom_connection_status(FALSE);
                }
            }
#endif
#ifdef AIR_TWS_ENABLE
            if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service) &&
                (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                //APPS_LOG_MSGID_I(LOG_TAG" BT_CM event, [%02X] AWS Connected reason=0x%02X", 2, role, remote_update->reason);
                app_le_audio_mps_set_battery(APP_MPS_CHANNEL_PEER, APP_MPS_BATTERY_LEVEL_VALUE_MAX);
                app_le_audio_send_battery_level_notification();
            } else if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                       && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                //APPS_LOG_MSGID_I(LOG_TAG" BT_CM event, [%02X] AWS Disconnected reason=0x%02X", 2, role, remote_update->reason);
                if (remote_update->reason != BT_HCI_STATUS_ROLE_SWITCH_PENDING && remote_update->reason != BT_HCI_STATUS_SUCCESS) {
                    app_le_audio_mps_set_battery(APP_MPS_CHANNEL_PEER, APP_MPS_BATTERY_LEVEL_VALUE_MIN);
                    app_le_audio_send_battery_level_notification();
                }
            }
#endif
            break;
        }
    }
    return FALSE;
}

static bool app_le_audio_proc_bt_dm_group(struct _ui_shell_activity *self,
                                          uint32_t event_id,
                                          void *extra_data,
                                          size_t data_len)
{
    bt_device_manager_power_event_t evt;
    bt_device_manager_power_status_t status;
    bt_event_get_bt_dm_event_and_status(event_id, &evt, &status);
    switch (evt) {
        case BT_DEVICE_MANAGER_POWER_EVT_ACTIVE_COMPLETE:
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
                bt_sink_srv_cap_am_init();
#ifdef AIR_LE_AUDIO_MTK_HFP_AT_CMD
                app_le_audio_hfp_at_cmd_register(TRUE);
#endif
            }
            break;
        case BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE:
            if (BT_DEVICE_MANAGER_POWER_RESET_TYPE_NORMAL == status) {
                bt_sink_srv_cap_am_deinit();
#ifdef AIR_LE_AUDIO_MTK_HFP_AT_CMD
                app_le_audio_hfp_at_cmd_register(FALSE);
#endif
            }
            break;
        default:
            break;
    }
    return FALSE;
}

static bool app_le_audio_proc_bt_sink_group(struct _ui_shell_activity *self,
                                            uint32_t event_id,
                                            void *extra_data,
                                            size_t data_len)
{
    switch (event_id) {
        case LE_SINK_SRV_EVENT_REMOTE_INFO_UPDATE: {
#if defined(AIR_LE_AUDIO_CIS_ENABLE) && defined(APPS_SLEEP_AFTER_NO_CONNECTION)
            bt_le_sink_srv_event_remote_info_update_t *update_ind = (bt_le_sink_srv_event_remote_info_update_t *)extra_data;
            if (update_ind == NULL) {
                break;
            }

            if (update_ind->pre_state == BT_BLE_LINK_DISCONNECTED
                && update_ind->state == BT_BLE_LINK_CONNECTED) {
                app_power_save_utils_notify_mode_changed(FALSE, NULL);
            } else if (update_ind->pre_state == BT_BLE_LINK_CONNECTED
                       && update_ind->state == BT_BLE_LINK_DISCONNECTED) {
                app_power_save_utils_notify_mode_changed(FALSE, NULL);
            }
#endif
            break;
        }
    }
    return FALSE;
}



/**************************************************************************************************
 * Public function
**************************************************************************************************/
#if 0
void bt_sink_srv_le_media_state_change_callback(uint16_t event_id, bt_handle_t handle, bool is_resume)
{
    // Use BT_SINK_SRV LE Audio Callback directly, not switch to APP task for low latency
    APPS_LOG_MSGID_I(LOG_TAG" le_media_state_change_callback, event_id=0x%04X handle=0x%04X is_resume=%d",
                     3, event_id, handle, is_resume);
    if (event_id == BT_LE_AUDIO_SINK_EVENT_MEDIA_SUSPEND) {
        app_le_audio_aird_client_notify_dongle_media_state(TRUE, handle, NULL, 0);
    } else if (event_id == BT_LE_AUDIO_SINK_EVENT_MEDIA_RESUME) {
        app_le_audio_aird_client_notify_dongle_media_state(FALSE, handle, NULL, 0);
    }
}
#else
void bt_sink_srv_edr_state_change_callback(bt_sink_srv_state_t previous, bt_sink_srv_state_t now)
{
    APPS_LOG_MSGID_I(LOG_TAG" edr_state_change, bt_sink_state %04X->%04X", 2, previous, now);
    if (((!(previous & BT_SINK_SRV_STATE_INCOMING)) && (now & BT_SINK_SRV_STATE_INCOMING)) ||
        ((!(previous & BT_SINK_SRV_STATE_OUTGOING)) && (now & BT_SINK_SRV_STATE_OUTGOING)) ||
        //((previous & BT_SINK_SRV_STATE_HELD_REMAINING) && (!(now & BT_SINK_SRV_STATE_HELD_REMAINING))) ||
        ((!(previous & BT_SINK_SRV_STATE_ACTIVE)) && (now & BT_SINK_SRV_STATE_ACTIVE)) ||
        ((!(previous & BT_SINK_SRV_STATE_STREAMING)) && (now & BT_SINK_SRV_STATE_STREAMING))) {
        /* HFP incoming call */
        /* HFP outgoing call */
        /* HFP cal unheld */
        /* HFP call active */
        /* A2DP Streaming start */
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_RESET_DONGLE_BUSY_EVENT);
        app_le_audio_aird_client_notify_dongle_media_state(TRUE, NULL, 0);
    } else if (((previous & BT_SINK_SRV_STATE_INCOMING) && (!(now & BT_SINK_SRV_STATE_INCOMING))) ||
               ((previous & BT_SINK_SRV_STATE_OUTGOING) && (!(now & BT_SINK_SRV_STATE_OUTGOING))) ||
               //((!(previous & BT_SINK_SRV_STATE_HELD_REMAINING)) && (now & BT_SINK_SRV_STATE_HELD_REMAINING)) ||
               ((previous & BT_SINK_SRV_STATE_ACTIVE) && (!(now & BT_SINK_SRV_STATE_ACTIVE))) ||
               ((previous & BT_SINK_SRV_STATE_STREAMING) && (!(now & BT_SINK_SRV_STATE_STREAMING)))) {
        /* HFP reject call (incoming) */
        /* HFP reject call (outgoing) */
        /* HFP cal held */
        /* HFP call end */
        /* A2DP Streaming stop */
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_RESET_DONGLE_BUSY_EVENT);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                            EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_RESET_DONGLE_BUSY_EVENT,
                            NULL, 0, NULL, APP_LE_AUDIO_RESET_DEVICE_BUSY_TIMEOUT);
    }
}
#endif

bool app_le_audio_idle_activity_proc(struct _ui_shell_activity *self, uint32_t event_group, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = FALSE;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            ret = app_le_audio_proc_bt_cm_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_SINK: {
            ret = app_le_audio_proc_bt_sink_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER: {
            ret = app_le_audio_proc_bt_dm_group(self, event_id, extra_data, data_len);
            break;
        }
        default:
            break;
    }

#ifdef AIR_TWS_ENABLE
    app_le_audio_dhss_proc_ui_shell_event(event_group, event_id, extra_data, data_len);
#endif
    app_le_audio_aird_client_proc_ui_shell_event(event_group, event_id, extra_data, data_len);
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    app_le_audio_bis_proc_ui_shell_event(self, event_group, event_id, extra_data, data_len);
#endif
#ifdef AIR_LE_AUDIO_MTK_HFP_AT_CMD
    app_le_audio_hfp_handle_event(event_group, event_id, extra_data, data_len);
#endif
    return ret;
}

void app_le_audio_init(void)
{
    APPS_LOG_MSGID_I(LOG_TAG" app_le_audio_init", 0);
#ifdef AIR_TWS_ENABLE
    app_le_audio_dhss_sdp_init();
#endif

    le_sink_srv_init(APP_LEA_MAX_CONN_NUM);
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    le_audio_set_device_type(LE_AUDIO_DEVICE_TYPE_HEADSET);
#endif

    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          MODULE_MASK_GAP | MODULE_MASK_SYSTEM | MODULE_MASK_GATT,
                                          (void *)app_le_audio_event_callback);

    app_le_audio_aird_client_init();
    app_le_audio_ccp_init();
    app_le_audio_mcp_init();

#ifdef AIR_LE_AUDIO_BIS_ENABLE
    app_le_audio_bis_init();
#endif

#ifdef AIR_TWS_ENABLE
    app_le_audio_mps_init();
#endif
#ifdef AIR_LE_AUDIO_HAPS_ENABLE
    ble_haps_init_server(APP_LEA_MAX_CONN_NUM);
#endif
}

void app_le_audio_disconnect_edr()
{
    bt_cm_connect_t cm_param;
    bt_bd_addr_t connected_address[APP_LEA_MAX_CONN_NUM];
    uint32_t connected_num = APP_LEA_MAX_CONN_NUM;
    uint32_t i;

    connected_num = bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                                                connected_address, connected_num);

    APPS_LOG_MSGID_W(LOG_TAG" connected_num %d", 1, connected_num);

    for (i = 0; i < connected_num; i++) {
        cm_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;

        memcpy(cm_param.address, connected_address[i], sizeof(bt_bd_addr_t));
        bt_cm_disconnect(&cm_param);
    }

}

bool app_le_audio_set_feature_mode(uint8_t feature_mode)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (app_lea_feature_mode == feature_mode) {
        APPS_LOG_MSGID_W(LOG_TAG" set_feature_mode, [%02X] same %d", 2, role, feature_mode);
        return TRUE;
    }
#ifndef AIR_LE_AUDIO_DUALMODE_ENABLE
    if (feature_mode == APP_LEA_FEATURE_MODE_DUAL_MODE) {
        APPS_LOG_MSGID_E(LOG_TAG" set_feature_mode, [%02X] cannot enable dual_mode", 1, role);
        return FALSE;
    }
#endif
#ifdef AIR_TWS_ENABLE
    if (bt_aws_mce_srv_get_link_type() == BT_AWS_MCE_SRV_LINK_NONE) {
        APPS_LOG_MSGID_E(LOG_TAG" set_feature_mode, [%02X] error AWS Disconnected feature_mode=%d",
                         2, role, feature_mode);
        return FALSE;
    }

    if (BT_AWS_MCE_ROLE_AGENT == role) {
        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                       EVENT_ID_LEA_SYNC_FEATURE_STATE,
                                       &feature_mode,
                                       sizeof(feature_mode));
    }
#endif

    app_lea_feature_mode = feature_mode;
    nvkey_write_data(NVID_APP_LEA_FEATURE_STATE, &app_lea_feature_mode, sizeof(app_lea_feature_mode_t));
    APPS_LOG_MSGID_W(LOG_TAG" set_feature_mode, [%02X] feature_mode=%d", 2, role, feature_mode);

    bt_device_manager_unpair_all();
    bt_device_manager_le_clear_all_bonded_info();
    app_lea_conn_mgr_reset_bond_info();

    app_le_audio_disconnect_edr();

    if (app_lea_feature_mode == APP_LEA_FEATURE_MODE_OFF) {
        app_lea_service_disconnect(FALSE, APP_LE_AUDIO_DISCONNECT_MODE_ALL,
                                   NULL, BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST);
        app_lea_service_stop_advertising(FALSE);
    } else if (app_lea_feature_mode == APP_LEA_FEATURE_MODE_ON
               || app_lea_feature_mode == APP_LEA_FEATURE_MODE_DUAL_MODE) {
        app_lea_service_disconnect(FALSE, APP_LE_AUDIO_DISCONNECT_MODE_ALL,
                                   NULL, BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST);
        if (role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_NONE) {
            app_bt_state_service_set_bt_visible(TRUE, FALSE, APP_LE_AUDIO_ADV_TIME);
        }
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_FORCE_UPDATE_ADV);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                            EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_FORCE_UPDATE_ADV,
                            NULL, 0, NULL, 0);
#ifdef AIR_TWS_ENABLE
        extern void app_le_audio_dhss_set_local_le_addr(bt_addr_type_t type, bt_bd_addr_t addr);
#endif

        if (app_lea_feature_mode == APP_LEA_FEATURE_MODE_DUAL_MODE) {

            APPS_LOG_MSGID_I(LOG_TAG" set_feature_mode, COD = 0x%x", 1, LE_AUDIO_COD);

            bt_avm_change_local_cod(LE_AUDIO_COD);

#ifdef AIR_TWS_ENABLE
            app_le_audio_dhss_set_local_le_addr(BT_ADDR_PUBLIC, *bt_device_manager_aws_local_info_get_fixed_address());
#endif
        } else {

            APPS_LOG_MSGID_I(LOG_TAG" set_feature_mode, COD =0x%x", 1, LE_AUDIO_COD_DISABLE_LEA);

            bt_avm_change_local_cod(LE_AUDIO_COD_DISABLE_LEA);
            app_le_audio_clear_adv = true;

#ifdef AIR_TWS_ENABLE
            app_le_audio_dhss_set_local_le_addr(BT_ADDR_RANDOM, *multi_ble_adv_get_instance_address(MULTI_ADV_INSTANCE_NOT_RHO));
#endif
        }
    }
    return TRUE;
}

void app_le_audio_reset(void)
{
    APPS_LOG_MSGID_I(LOG_TAG" reset", 0);
}

bool app_le_audio_get_addr(bool local_or_peer, uint8_t *type, uint8_t *addr)
{
    if (type == NULL || addr == NULL) {
        return FALSE;
    }

    bool is_enable_dual_mode = app_lea_service_is_enable_dual_mode();
#ifdef MTK_AWS_MCE_ENABLE
    if (local_or_peer) {
        if (is_enable_dual_mode) {
            uint8_t *edr_addr = (uint8_t *)bt_device_manager_aws_local_info_get_fixed_address();
            memcpy(addr, edr_addr, BT_BD_ADDR_LEN);
        } else {
            bt_bd_addr_t adv_addr = {0};
            if (!multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, &adv_addr, NULL)) {
                return FALSE;
            }
            memcpy(addr, &adv_addr, BT_BD_ADDR_LEN);
        }
    } else {
        uint32_t size = 1 + BT_BD_ADDR_LEN;
        uint8_t peer_addr[1 + BT_BD_ADDR_LEN] = {0};
        nvkey_status_t status = nvkey_read_data(NVID_APP_LEA_DHSS_PAIR_LE_ADDR, peer_addr, &size);
        if (NVKEY_STATUS_OK != status || size != 1 + BT_BD_ADDR_LEN) {
            APPS_LOG_MSGID_E(LOG_TAG" get_addr, peer NVKEY status=%d size=%d", 2, status, size);
            return FALSE;
        }
        for (int i = 0; i < BT_BD_ADDR_LEN; i++) {
            addr[i] = peer_addr[BT_BD_ADDR_LEN - i];
        }
    }
#else   /* Headset */
    if (is_enable_dual_mode) {
        uint8_t *edr_addr = (uint8_t *)bt_device_manager_get_local_address();
        memcpy(addr, edr_addr, BT_BD_ADDR_LEN);
    } else {
        bt_bd_addr_t adv_addr = {0};
        if (!multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, &adv_addr, NULL)) {
            return FALSE;
        }
        memcpy(addr, &adv_addr, BT_BD_ADDR_LEN);
    }
#endif

    if (is_enable_dual_mode) {
        *type = BT_ADDR_PUBLIC;
    } else {
        *type = BT_ADDR_RANDOM;
    }

    APPS_LOG_MSGID_I(LOG_TAG" get_addr, local=%d type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     8, local_or_peer, *type, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    return TRUE;
}

bool app_le_audio_is_connected(void)
{
    bt_handle_t le_handle = bt_sink_srv_cap_check_links_state(BT_SINK_SRV_CAP_STATE_CONNECTED);
    return (le_handle != BT_HANDLE_INVALID);
}

bool app_le_audio_is_clear_adv_data(void)
{
    bool is_clear_adv_data = app_le_audio_clear_adv;

    app_le_audio_clear_adv = false;

    APPS_LOG_MSGID_I(LOG_TAG" app_le_audio_clear_adv = %d is_clear_adv_data = %d ",
                     2, app_le_audio_clear_adv, is_clear_adv_data);

    return is_clear_adv_data;
}

#ifdef MTK_AWS_MCE_ENABLE
bool app_le_audio_is_primary_earbud(void)
{
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();

    if (app_lea_feature_mode == APP_LEA_FEATURE_MODE_DUAL_MODE) {
        bt_bd_addr_t *edr_addr = NULL;
        bt_bd_addr_t *le_addr = bt_device_manager_aws_local_info_get_fixed_address();

        if (BT_AWS_MCE_ROLE_PARTNER == role) {
            edr_addr = bt_device_manager_aws_local_info_get_peer_address();
        } else {
            edr_addr = bt_device_manager_get_local_address();
        }

        return (0 == memcmp(edr_addr, le_addr, BT_BD_ADDR_LEN));
    } else
        return (BT_AWS_MCE_ROLE_AGENT == role);
}
#endif

#endif  /* AIR_LE_AUDIO_ENABLE */
