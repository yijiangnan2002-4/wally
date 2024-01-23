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

#if defined(AIR_SWIFT_PAIR_ENABLE) && defined(AIR_CUST_PAIR_ENABLE)

#include "app_cust_pair_idle_activity.h"

#include "app_swift_cust_pair.h"
#include "cust_pair.h"

#include "app_bt_state_service.h"
#include "apps_customer_config.h"
#include "apps_events_battery_event.h"
#include "apps_events_bt_event.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_debug.h"

#include "bt_app_common.h"
#include "battery_management.h"
#include "battery_management_core.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_customer_config.h"
#include "bt_device_manager.h"
#include "bt_gap_le.h"
#include "bt_app_common.h"
#include "multi_ble_adv_manager.h"
#include "ui_shell_manager.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#endif
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
#include "app_rho_idle_activity.h"
#endif
#ifdef AIR_BT_FAST_PAIR_ENABLE
#include "app_fast_pair.h"
#endif
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio.h"
#include "app_lea_service_adv_mgr.h"
#endif



/**================================================================================*/
/**                              Definition & Structure                            */
/**================================================================================*/
#define LOG_TAG                             "[CUST_PAIR][APP]"

extern bt_status_t bt_app_common_set_pairing_distribute_ctkd(bt_gap_le_srv_link_t link_type, bool is_ctkd_support);

#define APP_CUST_PAIR_OTHER_ADV_TIMEOUT          (6 * 1000)
#define APP_CUST_PAIR_ADV_TIMEOUT                (VISIBLE_TIMEOUT)

#define APP_CUST_PAIR_TRY_EDR_CONN_TIMEOUT       (3 * 1000)

#define APP_CUST_PAIR_PRODUCT_TYPE               0          // Need customer configure
#define APP_CUST_PAIR_API_VERSION                1          // Need customer configure, current API version number
#define CUST_PAIR_ADV_RESERVE                    0          // Need customer configure, Only for using extended advertising case, default 0
#define CUST_PAIR_ADV_VID                        0          // Need customer configure, Only for using extended advertising case
#define CUST_PAIR_ADV_PID                        0x250A     // Need customer configure, set Actual PRODUCT ID (Same like USB PID)
#define CUST_PAIR_ADV_INTERVAL                   0x0020     // default 20ms = 32 * 0.625
#define CUST_PAIR_ADV_TX_POWER                   (0)        // default 0 or -4/0xFC, customer could adjust the value to tune "pop up distance"

#define APP_CUST_PAIR_ADV_MODE_TIMEOUT           2000

typedef struct {
    cust_pair_adv_mode                           adv_mode;
    uint8_t                                      sp_addr[BT_BD_ADDR_LEN];
} app_cust_pair_context_t;

static app_cust_pair_context_t              app_cust_pair_ctx = {0};



/**================================================================================*/
/**                                Internal Function                               */
/**================================================================================*/
bt_status_t bt_gatts_service_get_gap_device_name_with_handle(bt_handle_t connection_handle, uint8_t *device_name, uint32_t *length)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    const bt_gap_config_t *cust_config = bt_customer_config_get_gap_config();
    const char *bt_name = cust_config->device_name;
    uint8_t bt_name_len = strlen(bt_name);
    bool is_exist = cust_pair_is_exist_conn(connection_handle);

    if (device_name == NULL || length == NULL || bt_name_len == 0) {
        APPS_LOG_MSGID_E(LOG_TAG" get_gap_device_name_with_handle, name=0x%08X length=0x%08X bt_name_len=%d",
                         3, device_name, length, bt_name_len);
        return bt_status;
    }

    if (is_exist) {
        memcpy(device_name, bt_name, bt_name_len);              // Need customer configure, such as "123456" as LE name, "123456 BT" as classic name
        *length = bt_name_len;
        bt_status = BT_STATUS_SUCCESS;
    }

    if (bt_status == BT_STATUS_SUCCESS) {
        APPS_LOG_I(LOG_TAG" get_gap_device_name_with_handle, handle=0x%04X device_name=%s %d",
                   connection_handle, device_name, bt_name_len);
    }
    return bt_status;
}

static bool cust_pair_disconnect_edr(const uint8_t *addr)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_cm_connect_t disconn_param = {0};
    disconn_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
    memcpy(&(disconn_param.address), addr, sizeof(bt_bd_addr_t));
    bt_status_t bt_status = bt_cm_disconnect(&disconn_param);
    APPS_LOG_MSGID_I(LOG_TAG" disconnect_edr, [%02X] addr=%08X%04X bt_status=0x%08X",
                     4, role, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), bt_status);
    return (bt_status == BT_STATUS_SUCCESS);
}

static void cust_pair_connect_edr(const uint8_t *addr)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    uint32_t default_edr_profile = (bt_customer_config_app_get_cm_config()->power_on_reconnect_profile
                                    & (~(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))));
    bt_cm_connect_t connect_param = {{0}, 0};
    memcpy(connect_param.address, addr, sizeof(bt_bd_addr_t));
    connect_param.profile = default_edr_profile;
    bt_status_t bt_status = bt_cm_connect(&connect_param);
    APPS_LOG_MSGID_I(LOG_TAG" connect_edr, [%02X] addr=%08X%04X profile=0x%08X bt_status=0x%08X",
                     5, role, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), default_edr_profile, bt_status);
}

bool cust_pair_update_ctkd(bool ctkd_support)
{
    APPS_LOG_MSGID_I(LOG_TAG" update_ctkd, ctkd_support=%d", 1, ctkd_support);
    bt_status_t bt_status = bt_app_common_set_pairing_distribute_ctkd(BT_GAP_LE_SRV_LINK_TYPE_CUST_PAIR, ctkd_support);
    return (bt_status == BT_STATUS_SUCCESS);
}

bool cust_pair_check_le_link(const uint8_t *le_local_addr)
{
    bool ret = FALSE;
#ifdef APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE
#if defined(AIR_TWS_ENABLE)
    uint8_t *edr_addr = (uint8_t *)bt_device_manager_aws_local_info_get_fixed_address();
#else
    uint8_t *edr_addr = (uint8_t *)bt_device_manager_get_local_address();
#endif
    ret = (memcmp(le_local_addr, edr_addr, BT_BD_ADDR_LEN) == 0);
    APPS_LOG_MSGID_I(LOG_TAG" check_ble_link, [Public] ret=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     7, ret, edr_addr[5], edr_addr[4], edr_addr[3], edr_addr[2], edr_addr[1], edr_addr[0]);
#else
    bt_bd_addr_t adv_addr = {0};
    if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_SWIFT_PAIR, &adv_addr, NULL)) {
        ret = (memcmp(&adv_addr, le_local_addr, BT_BD_ADDR_LEN) == 0);
        APPS_LOG_MSGID_I(LOG_TAG" check_ble_link, [Random] ret=%d adv_addr=%02X:%02X:%02X:%02X:%02X:%02X",
                         7, ret, adv_addr[5], adv_addr[4], adv_addr[3], adv_addr[2], adv_addr[1], adv_addr[0]);
    }
#endif
    return ret;
}

void cust_pair_start_le_conn_timer(bool start, uint16_t conn_handle, uint32_t timeout)
{
#if 0
    APPS_LOG_MSGID_I(LOG_TAG" start_le_conn_timer, start=%d conn_handle=0x%04X timeout=%d",
                     3, start, conn_handle, timeout);
    if (start) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                              APP_SWIFT_PAIR_EVENT_CUST_CONN_TIMER_BASE + conn_handle);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                            APP_SWIFT_PAIR_EVENT_CUST_CONN_TIMER_BASE + conn_handle,
                            NULL, 0, NULL, timeout);
    } else {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                              APP_SWIFT_PAIR_EVENT_CUST_CONN_TIMER_BASE + conn_handle);
    }
#endif
}

static uint32_t app_cust_pair_get_adv_info(multi_ble_adv_info_t *adv_info)
{
    if (adv_info == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" get_adv_info, null adv_info", 0);
        return 0;
    }

    if (adv_info->adv_param != NULL) {
        bt_app_common_generate_default_adv_data(adv_info->adv_param, NULL, NULL, NULL, 0);
    }

    cust_pair_get_adv_info(app_cust_pair_ctx.adv_mode, adv_info->adv_param, adv_info->adv_data, adv_info->scan_rsp);
    return 0;
}

static void app_cust_pair_update_adv(void)
{
#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
    bool visible = app_bt_service_is_visible();
    bt_aws_mce_srv_link_type_t aws_link = bt_aws_mce_srv_get_link_type();
    bool is_primary = app_le_audio_is_primary_earbud();
    if (!visible || !is_primary || aws_link == BT_AWS_MCE_SRV_LINK_NONE) {
        APPS_LOG_MSGID_E(LOG_TAG" update_adv, visible=%d is_primary=%d aws_link=%d fail",
                         3, visible, is_primary, aws_link);
        return;
    }
#endif

    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_cust_pair_get_adv_info);
    multi_ble_adv_manager_add_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_cust_pair_get_adv_info, 1);
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_SWIFT_PAIR);
}

static void app_cust_pair_switch_adv_type(void)
{
    uint8_t old_adv_mode = app_cust_pair_ctx.adv_mode;
    if (old_adv_mode == CUST_PAIR_ADV_MODE_STD) {
        app_cust_pair_ctx.adv_mode = CUST_PAIR_ADV_MODE_CUST;
    } else if (old_adv_mode == CUST_PAIR_ADV_MODE_CUST) {
        app_cust_pair_ctx.adv_mode = CUST_PAIR_ADV_MODE_STD;
    }

    app_cust_pair_update_adv();
    APPS_LOG_MSGID_I(LOG_TAG" switch_adv_type, adv_mode=%d->%d", 2, old_adv_mode, app_cust_pair_ctx.adv_mode);

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_CUST_SWITCH_TYPE);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                        APP_SWIFT_PAIR_EVENT_CUST_SWITCH_TYPE, NULL, 0, NULL, APP_CUST_PAIR_ADV_MODE_TIMEOUT);
}

static bool app_cust_pair_start_adv(void)
{
    app_cust_pair_ctx.adv_mode = CUST_PAIR_ADV_MODE_STD;

    app_cust_pair_update_adv();

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_CUST_SWITCH_TYPE);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_CUST_ADV_TIMEOUT);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                        APP_SWIFT_PAIR_EVENT_CUST_SWITCH_TYPE, NULL, 0, NULL, APP_CUST_PAIR_ADV_MODE_TIMEOUT);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                        APP_SWIFT_PAIR_EVENT_CUST_ADV_TIMEOUT, NULL, 0, NULL, APP_CUST_PAIR_ADV_TIMEOUT);

    APPS_LOG_MSGID_I(LOG_TAG" start_adv, adv_mode=%d time=%d", 2, app_cust_pair_ctx.adv_mode, APP_CUST_PAIR_ADV_MODE_TIMEOUT);
    return TRUE;
}

static bool app_cust_pair_stop_adv(void)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_CUST_SWITCH_TYPE);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_CUST_ADV_TIMEOUT);

    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_cust_pair_get_adv_info);
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_SWIFT_PAIR);
    APPS_LOG_MSGID_I(LOG_TAG" stop_adv", 0);
    return TRUE;
}

static bool app_cust_pair_proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            const bt_gap_config_t *cust_config = bt_customer_config_get_gap_config();
            const char *bt_name = cust_config->device_name;
            uint8_t battery = (uint8_t)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);

            multi_ble_adv_manager_set_le_connection_max_count(MULTI_ADV_INSTANCE_SWIFT_PAIR, CUST_PAIR_MAX_CONN_NUM);
            cust_pair_init(bt_name, battery);
            // Need customer configure
            cust_pair_adv_config_parameter(APP_CUST_PAIR_PRODUCT_TYPE, APP_CUST_PAIR_API_VERSION,
                                           CUST_PAIR_ADV_RESERVE, CUST_PAIR_ADV_VID, CUST_PAIR_ADV_PID,
                                           CUST_PAIR_ADV_INTERVAL, CUST_PAIR_ADV_TX_POWER);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

static bool app_cust_pair_proc_battery_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case APPS_EVENTS_BATTERY_PERCENT_CHANGE: {
            uint8_t battery = (uint8_t)(int32_t)extra_data;
            //APPS_LOG_MSGID_I(LOG_TAG" battery event, [%02X] battery=%d", 2, role, battery);
            cust_pair_update_battery(battery);
            break;
        }
        default:
            break;
    }
    return FALSE;
}

static bool app_cust_pair_proc_bt_cm_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            /* Stop Swift pair ADV when connected. */
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (NULL == remote_update) {
                //APPS_LOG_MSGID_E(LOG_TAG" BT_CM event, null remote_update", 0);
                break;
            }

            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            bt_bd_addr_t *local_addr = bt_device_manager_get_local_address();
            bool phone_related = (memcmp(remote_update->address, local_addr, sizeof(bt_bd_addr_t)) != 0);
            uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
            bool visible = app_bt_service_is_visible();
            bool is_primary = app_le_audio_is_primary_earbud();
            bool aws_conntected = (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                                   && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service));
            bool aws_disconntected = ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                                      && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service));
#endif

            if (BT_AWS_MCE_ROLE_AGENT == role || BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_FOLLOWER_1 == role) {
                if (remote_update->pre_acl_state != BT_CM_ACL_LINK_CONNECTED
                    && remote_update->acl_state == BT_CM_ACL_LINK_CONNECTED
                    && phone_related) {
#if !defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) || !defined(AIR_TWS_ENABLE)
                    APPS_LOG_MSGID_I(LOG_TAG" BT_CM event, Remote connected -> stop ADV", 0);
                    app_cust_pair_stop_adv();
#endif
                    // After swift/cust pairing with public addr & support LEA, No need the workaround feature
#ifndef APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE
                    memset(app_cust_pair_ctx.sp_addr, 0, BT_BD_ADDR_LEN);
                    extern bool cust_pair_srv_le_bond_flag;
                    if (cust_pair_srv_le_bond_flag) {
                        cust_pair_srv_le_bond_flag = FALSE;
                        uint8_t *addr = (uint8_t *)pvPortMalloc(BT_BD_ADDR_LEN);
                        if (addr != NULL) {
                            memcpy(addr, remote_update->address, BT_BD_ADDR_LEN);
                            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_CUST_TRY_EDR_CONN);
                            ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                                                APP_SWIFT_PAIR_EVENT_CUST_TRY_EDR_CONN,
                                                addr, BT_BD_ADDR_LEN, NULL, APP_CUST_PAIR_TRY_EDR_CONN_TIMEOUT);
                        }
                    }
#endif
                } else if (remote_update->pre_acl_state != BT_CM_ACL_LINK_DISCONNECTED
                        && remote_update->acl_state == BT_CM_ACL_LINK_DISCONNECTED
                        && phone_related) {
                    if (memcmp(app_cust_pair_ctx.sp_addr, empty_addr, BT_BD_ADDR_LEN) != 0
                        && memcmp(app_cust_pair_ctx.sp_addr, remote_update->address, BT_BD_ADDR_LEN) == 0) {
                        cust_pair_connect_edr(app_cust_pair_ctx.sp_addr);
                        memset(app_cust_pair_ctx.sp_addr, 0, BT_BD_ADDR_LEN);
                    }
                }

                if (remote_update->acl_state == BT_CM_ACL_LINK_ENCRYPTED
                    && remote_update->pre_connected_service == 0
                    && remote_update->connected_service > 0) {
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_CUST_TRY_EDR_CONN);
                }
            }

#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
            if (aws_conntected || aws_disconntected) {
                APPS_LOG_MSGID_I(LOG_TAG" BT_CM AWS event, [%02X] aws_conn=%d visible=%d is_primary=%d",
                                 4, role, aws_conntected, visible, is_primary);
            }
            if (aws_conntected && is_primary && visible) {
                app_cust_pair_start_adv();
            } else if (aws_disconntected) {
                app_cust_pair_stop_adv();
            }
#endif
            break;
        }

#if !defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) || !defined(AIR_TWS_ENABLE)
        case BT_CM_EVENT_VISIBILITY_STATE_UPDATE: {
            bt_cm_visibility_state_update_ind_t *visible_update = (bt_cm_visibility_state_update_ind_t *)extra_data;
            if (NULL == visible_update) {
                break;
            }

            bool visible = visible_update->visibility_state;
            APPS_LOG_MSGID_I(LOG_TAG" BT_CM event, visibility_state=%d", 1, visible);

            if (visible) {
                app_cust_pair_start_adv();
            } else {
                app_cust_pair_stop_adv();
            }
            break;
        }
#endif
        default:
            break;
    }
    return FALSE;
}

static bool app_cust_pair_proc_interaction_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    switch (event_id) {
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
        case APPS_EVENTS_INTERACTION_RHO_END: {
            app_rho_result_t rho_result = (app_rho_result_t)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" interaction event, RHO (Agent->Partner) - %d", 1, rho_result);
            if (APP_RHO_RESULT_SUCCESS == rho_result) {
                cust_pair_role_switch(FALSE);
            }
            break;
        }
        /* The old Partner will switch to new Agent if RHO successfully. */
        case APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT: {
            app_rho_result_t rho_result = (app_rho_result_t)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" interaction event, RHO (Partner->Agent) - %d", 1, rho_result);
            if (APP_RHO_RESULT_SUCCESS == rho_result) {
                cust_pair_role_switch(TRUE);
            }
            break;
        }
#endif
#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
        case APPS_EVENTS_INTERACTION_BT_VISIBLE_NOTIFY: {
            bool visible = (bool)extra_data;
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            bt_aws_mce_srv_link_type_t aws_link = bt_aws_mce_srv_get_link_type();
            bool is_primary = app_le_audio_is_primary_earbud();
            APPS_LOG_MSGID_I(LOG_TAG" BT_VISIBLE_NOTIFY event, [%02X] visible=%d aws_link=%d is_primary=%d",
                             4, role, visible, aws_link, is_primary);
            if (visible && aws_link != BT_AWS_MCE_SRV_LINK_NONE && is_primary) {
                app_cust_pair_start_adv();
            } else if (!visible) {
                app_cust_pair_stop_adv();
            }
            break;
        }
#endif
    }
    return ret;
}

static bool app_cust_pair_proc_swift_pair_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case APP_SWIFT_PAIR_EVENT_RESTART_ADV: {
            bool is_visible = app_bt_service_is_visible();
            APPS_LOG_MSGID_I(LOG_TAG" cust_pair event, restart_adv visible=%d", 1, is_visible);
            if (is_visible) {
                app_cust_pair_start_adv();
            }
            break;
        }

        case APP_SWIFT_PAIR_EVENT_CUST_SWITCH_TYPE: {
            //APPS_LOG_MSGID_I(LOG_TAG" cust_pair event, switch_adv_type", 0);
            app_cust_pair_switch_adv_type();
            break;
        }

        case APP_SWIFT_PAIR_EVENT_CUST_ADV_TIMEOUT: {
            APPS_LOG_MSGID_I(LOG_TAG" cust_pair event, timeout", 0);
            app_cust_pair_stop_adv();
            break;
        }

        case APP_SWIFT_PAIR_EVENT_CUST_ADV_STOP: {
            app_cust_pair_stop_adv();
            break;
        }

        case APP_SWIFT_PAIR_EVENT_CUST_GFP_ADV_RESTORE: {
#ifdef AIR_BT_FAST_PAIR_ENABLE
            app_fast_pair_enable_advertising(TRUE);
#endif
            break;
        }

        case APP_SWIFT_PAIR_EVENT_CUST_LEA_ADV_ADJUST: {
#ifdef AIR_LE_AUDIO_ENABLE
            app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_S, APP_LE_AUDIO_ADV_INTERVAL_MAX_S);
#endif
            break;
        }

        case APP_SWIFT_PAIR_EVENT_CUST_TRY_EDR_CONN: {
            uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
            uint8_t *addr = (uint8_t *)extra_data;
            APPS_LOG_MSGID_W(LOG_TAG" cust_pair event, TRY_EDR_CONN save addr=%02X:%02X:%02X:%02X:%02X:%02X",
                             6, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
            if (memcmp(addr, empty_addr, BT_BD_ADDR_LEN) != 0) {
                bt_cm_profile_service_mask_t mask = bt_cm_get_connected_profile_services(addr);
                if (mask == 0 || mask == BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)) {
                    bool success = cust_pair_disconnect_edr(addr);
                    if (success) {
                        memcpy(app_cust_pair_ctx.sp_addr, addr, BT_BD_ADDR_LEN);
                    }
                    break;
                }
            }
            break;
        }

        default:
            break;
    }

    if (event_id > APP_SWIFT_PAIR_EVENT_CUST_CONN_TIMER_BASE && event_id < APP_SWIFT_PAIR_EVENT_CUST_DISCONNECT_BASE) {
        uint16_t conn_handle = event_id - APP_SWIFT_PAIR_EVENT_CUST_CONN_TIMER_BASE;
        APPS_LOG_MSGID_W(LOG_TAG" cust_pair event, LE_CONN_TIMEOUT conn_handle=0x%04X", 1, conn_handle);
        cust_pair_disconnect(conn_handle);
    } else if (event_id >= APP_SWIFT_PAIR_EVENT_CUST_DISCONNECT_BASE) {
        uint16_t conn_handle = event_id - APP_SWIFT_PAIR_EVENT_CUST_DISCONNECT_BASE;
        APPS_LOG_MSGID_W(LOG_TAG"[BAS] cust_pair event, DISCONNECT conn_handle=0x%04X", 1, conn_handle);
        cust_pair_disconnect(conn_handle);
    }

    return TRUE;
}



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
bool cust_pair_notify_event(uint8_t event)
{
    if (event == CUST_PAIR_EVENT_LE_PAIRING_START) {
        APPS_LOG_MSGID_I(LOG_TAG" notify_event, LE_PAIRING_START", 0);
        app_cust_pair_stop_adv();
#ifdef AIR_BT_FAST_PAIR_ENABLE
        app_fast_pair_enable_advertising(FALSE);
#endif
#ifdef AIR_LE_AUDIO_ENABLE
        app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_L, APP_LE_AUDIO_ADV_INTERVAL_MAX_L);
#endif
    } else if (event == CUST_PAIR_EVENT_LE_DISCONNECTED
            || event == CUST_PAIR_EVENT_LE_BONDED) {
        bool is_visible = app_bt_service_is_visible();
        APPS_LOG_MSGID_I(LOG_TAG" notify_event, LE_DISCONNECTED/LE_BONDED visible=%d", 1, is_visible);
#ifdef AIR_BT_FAST_PAIR_ENABLE
        if (event == CUST_PAIR_EVENT_LE_BONDED) {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_CUST_GFP_ADV_RESTORE);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                                APP_SWIFT_PAIR_EVENT_CUST_GFP_ADV_RESTORE,
                                NULL, 0, NULL, APP_CUST_PAIR_OTHER_ADV_TIMEOUT);
        }
#endif
#ifdef AIR_LE_AUDIO_ENABLE
        if (event == CUST_PAIR_EVENT_LE_DISCONNECTED) {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_CUST_LEA_ADV_ADJUST);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                                APP_SWIFT_PAIR_EVENT_CUST_LEA_ADV_ADJUST,
                                NULL, 0, NULL, APP_CUST_PAIR_OTHER_ADV_TIMEOUT);
        }
#endif
    }
    return TRUE;
}

bool app_cust_pair_idle_activity_proc(struct _ui_shell_activity *self,
                                      uint32_t event_group,
                                      uint32_t event_id,
                                      void *extra_data,
                                      size_t data_len)
{
    bool ret = FALSE;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            ret = app_cust_pair_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BATTERY: {
            ret = app_cust_pair_proc_battery_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            ret = app_cust_pair_proc_bt_cm_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            ret = app_cust_pair_proc_interaction_group(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_SWIFT_PAIR: {
            ret = app_cust_pair_proc_swift_pair_group(self, event_id, extra_data, data_len);
            break;
        }
        default:
            break;
    }
    return ret;
}

#endif
