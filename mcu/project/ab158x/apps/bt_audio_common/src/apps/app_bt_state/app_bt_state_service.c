/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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
 * File: app_bt_state_service.c
 *
 * Description: This file provides many utility function to control BT switch and visibility.
 *
 */

#include "app_bt_state_service.h"
#include "apps_events_event_group.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_adapt.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager.h"
#include "bt_device_manager_power.h"
#include "bt_app_common.h"
#include "ui_shell_manager.h"
#include "voice_prompt_api.h"
#include "apps_config_vp_index_list.h"
#include "apps_events_bt_event.h"
#include "apps_customer_config.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#include "apps_aws_sync_event.h"
#include "apps_config_features_dynamic_setting.h"
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "bt_ull_service.h"
#include "bt_hci.h"
#endif
#ifdef MTK_RACE_CMD_ENABLE
#include "race_bt.h"
#endif
#include "app_rho_idle_activity.h"
#include "apps_events_interaction_event.h"
#include "apps_debug.h"
#ifdef AIR_MCSYNC_SHARE_ENABLE
#include "bt_mcsync_share.h"
#endif
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "app_lea_service.h"
#endif
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le.h"
#ifdef AIR_LE_AUDIO_MULTIPOINT_ENABLE
#include "bt_sink_srv_le_cap.h"
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_le_audio_bis.h"
#include "bt_sink_srv_le_cap_stream.h"
#endif
#endif
#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#endif
#ifdef AIR_APP_A2DP_LBB_VENDOR_CODEC_LIMIT
#include "bt_sink_srv_a2dp.h"
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE)
#include "apps_race_cmd_co_sys_event.h"
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "bt_ull_service.h"
extern void app_ull_take_over_enable_visible(void);
extern void app_ull_take_over_disable_visible(void);
#endif

#ifdef AIR_BT_TAKEOVER_ENABLE
#include "app_bt_takeover_service.h"
#endif
#ifdef AIR_SWIFT_PAIR_ENABLE
#include "app_swift_cust_pair.h"
#endif
#include "bt_hci.h"
#include "bt_init.h"

#ifdef AIR_WIRELESS_MIC_ENABLE
#include "app_wireless_mic_idle_activity.h"
#endif

#ifdef AIR_SPEAKER_ENABLE
#include "app_speaker_le_association.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_le_service.h"
#endif

//#include "app_multi_va_idle_activity.h"


#define LOG_TAG     "[app_bt_state_service]"

#define RETRY_TIMEOUT_BT_POWER_OFF_FAILED       (500)

/**
 *  @brief This structure defines the pending request of BT visibility.
 */
typedef struct {
    bool bt_visible;                        /**<  Request enable/disable BT visibility. */
    bool need_wait_aws;                     /**<  Whether must wait AWS. */
    uint32_t timeout;                       /**<  BT visibility duration time. */
} app_bt_state_service_bt_visible_request_t;

/* Current BT visibility pending request. */
app_bt_state_service_bt_visible_request_t s_visible_pending_request = {
    false,
    false,
    0
};

/* The flag for system off. */
static bool s_for_system_off = false;
#ifdef MTK_AWS_MCE_ENABLE
static uint32_t s_visible_timeout_time;
#endif

/* Current status/context of BT state service. */
app_bt_state_service_status_t s_current_status = {
    .connection_state = APP_BT_CONNECTION_SERVICE_BT_STATE_BT_OFF,
    .current_power_state = APP_BT_STATE_POWER_STATE_DISABLED,
    .target_power_state = APP_BT_STATE_POWER_STATE_NONE_ACTION,
#ifdef AIR_LE_AUDIO_ENABLE
    .ble_audio_state = APP_BLE_AUDIO_STATE_BT_OFF,
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    .ble_ull_state = APP_BLE_ULL_STATE_BT_OFF,
#endif
    .aws_connected = false,
    .bt_visible = false,
#ifdef MTK_AWS_MCE_ENABLE
    .in_air_pairing = false,
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    .in_ull_pairing = false,
#endif
    .reason = BT_HCI_STATUS_CONNECTION_TIMEOUT,
};

static bool app_bt_state_service_pending_bt_on_off(void)
{
#if defined(MTK_AWS_MCE_ENABLE)
    if (s_current_status.in_air_pairing) {
        return true;
    }
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    if (s_current_status.in_ull_pairing) {
        return true;
    }
#endif
    return false;
}

static void app_bt_state_service_notify_bt_visible(bool visible)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_NOTIFY);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_BT_VISIBLE_NOTIFY, (void *)visible, 0,
                        NULL, 0);
}

/**
* @brief      This function is used to control BT on/off via BT power API.
* @param[in]  try_rho, whether need to trigger RHO before turn off BT.
*/
static void app_bt_state_service_check_and_do_bt_enable_disable(bool try_rho)
{
    APPS_LOG_MSGID_I(LOG_TAG" app_bt_state_service_check_and_do_bt_enable_disable target status ? %d, current statue: %d",
                     2, s_current_status.target_power_state, s_current_status.current_power_state);

    if (app_bt_state_service_pending_bt_on_off()) {
        return;
    }

    if (APP_BT_STATE_POWER_STATE_DISABLING == s_current_status.current_power_state
        || APP_HOME_SCREEN_BT_POWER_CLASSIC_DISABLING == s_current_status.current_power_state
        || APP_BT_STATE_POWER_STATE_ENABLING == s_current_status.current_power_state) {
        return;
    } else if (s_current_status.current_power_state == s_current_status.target_power_state
               || (APP_BT_STATE_POWER_STATE_DISABLED == s_current_status.current_power_state && APP_HOME_SCREEN_BT_POWER_CLASSIC_DISABLED == s_current_status.target_power_state)) {
        /* Enable target state is the same as s_current_status.current_power_state, do not call bt function. */
        s_current_status.target_power_state = APP_BT_STATE_POWER_STATE_NONE_ACTION;
        return;
    } else if (APP_BT_STATE_POWER_STATE_ENABLED == s_current_status.target_power_state) {
        /* Enable BT when s_current_status.target_power_state is APP_BT_STATE_POWER_STATE_ENABLED. */
        bt_device_manager_power_active(BT_DEVICE_TYPE_LE | BT_DEVICE_TYPE_CLASSIC);
        s_current_status.current_power_state = APP_BT_STATE_POWER_STATE_ENABLING;
        s_current_status.target_power_state = APP_BT_STATE_POWER_STATE_NONE_ACTION;
    } else if (APP_BT_STATE_POWER_STATE_DISABLED == s_current_status.target_power_state
               || APP_HOME_SCREEN_BT_POWER_CLASSIC_DISABLED == s_current_status.target_power_state) {
#ifdef APPS_AUTO_TRIGGER_RHO
        /* Need to check RHO before disable BT. */
        if (apps_config_features_is_auto_rho_enabled()
            && bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
            && try_rho) {
            APPS_LOG_MSGID_I(LOG_TAG" app_bt_state_service_check_and_do_bt_enable_disable do RHO before disable bt", 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_TRIGGER_RHO, NULL, 0,
                                NULL, 0);
        } else
#endif
        {
#ifdef AIR_LE_AUDIO_BIS_ENABLE
            bool is_bis_streaming = bt_sink_srv_cap_stream_is_broadcast_streaming();
            if (APP_BT_STATE_POWER_STATE_DISABLED == s_current_status.target_power_state
                && is_bis_streaming) {
                bool success = app_le_audio_bis_stop_streaming(FALSE);
                if (success) {
                    APPS_LOG_MSGID_I(LOG_TAG" [LEA][BIS] app_bt_state_service_check_and_do_bt_enable_disable, OFF pending due to BIS streaming", 0);
                    return;
                }
            }
#endif

#ifdef AIR_BT_INTEL_EVO_ENABLE
#if defined(MTK_AWS_MCE_ENABLE)
        bt_status_t bt_ret = BT_STATUS_FAIL;
        bt_sink_srv_device_state_t device_state;
#if defined(AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE)
        bt_ret = bt_sink_srv_get_playing_device_state(&device_state);
#endif
        if (bt_aws_mce_srv_get_link_type() == BT_AWS_MCE_SRV_LINK_NONE
            || (bt_ret == BT_STATUS_SUCCESS && BT_SINK_SRV_DEVICE_LE != device_state.type && bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT && !try_rho))
#endif
        {
            bt_sink_srv_state_t sink_state = bt_sink_srv_get_state();
            if (BT_SINK_SRV_STATE_STREAMING == sink_state) {
                bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PAUSE, NULL);
            }
        }
#endif

#ifdef APP_CONN_MGR_RECONNECT_CONTROL
            extern void app_bt_conn_manager_save(void);
            app_bt_conn_manager_save();
#endif
            APPS_LOG_MSGID_I(LOG_TAG" app_bt_state_service_check_and_do_bt_enable_disable start set BT enable: %d", 1, s_current_status.target_power_state);
#ifdef AIR_SPEAKER_ENABLE
            if (bt_aws_mce_srv_get_mode() == BT_AWS_MCE_SRV_MODE_BROADCAST &&
                bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_AGENT) {
                /* Notify client to switch to single through urgent channel. */
                apps_aws_sync_event_send_for_broadcast(TRUE, EVENT_GROUP_UI_SHELL_LE_ASSOCIATION,
                                                       LE_ASSOCIATION_EVENT_AWS_MCE_UNGROUP, NULL, 0);
            }
#endif
            /* Disable BT directly when role is partner, no need RHO or AWS not connected. */
            bt_status_t bt_state;
            if (APP_BT_STATE_POWER_STATE_DISABLED == s_current_status.target_power_state) {
                bt_state = bt_device_manager_power_standby(BT_DEVICE_TYPE_LE | BT_DEVICE_TYPE_CLASSIC);
            } else {
                bt_state = bt_device_manager_power_standby(BT_DEVICE_TYPE_CLASSIC);
            }
            if (BT_STATUS_SUCCESS == bt_state) {
                if (APP_BT_STATE_POWER_STATE_DISABLED == s_current_status.target_power_state) {
                    s_current_status.current_power_state = APP_BT_STATE_POWER_STATE_DISABLING;
                } else {
                    s_current_status.current_power_state = APP_HOME_SCREEN_BT_POWER_CLASSIC_DISABLING;
                }
                s_current_status.target_power_state = APP_BT_STATE_POWER_STATE_NONE_ACTION;
            } else {
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_BT_RETRY_POWER_ON_OFF, NULL, 0,
                                    NULL, RETRY_TIMEOUT_BT_POWER_OFF_FAILED);
            }
        }
    }
}

/**
* @brief      This function is used to refresh BT visibility duration time.
* @param[in]  timeout, updated BT visibility duration time.
*/
#if !(defined(AIR_WIRELESS_MIC_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE))
static void app_bt_state_service_refresh_visible_timeout(uint32_t timeout)
{
    APPS_LOG_MSGID_I(LOG_TAG" app_bt_state_service_refresh_visible_timeout: %d", 1, timeout);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
    if (timeout > BT_VISIBLE_TIMEOUT_INVALID) {
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT, NULL, 0,
                            NULL, timeout);
#ifdef MTK_AWS_MCE_ENABLE
        s_visible_timeout_time = xTaskGetTickCount() * portTICK_PERIOD_MS + timeout;
    } else {
        s_visible_timeout_time = BT_VISIBLE_TIMEOUT_INVALID;
#endif
    }
}
#endif

/**
* @brief      This function is used to check and turn on BT visibility after BT enabled or AWS connected.
*/
static void app_bt_state_service_check_and_do_bt_visible(void)
{
    if (s_visible_pending_request.bt_visible
#ifdef MTK_AWS_MCE_ENABLE
        && bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
#endif
       ) {
        APPS_LOG_MSGID_I(LOG_TAG" app_bt_state_service_check_and_do_bt_visible s_current_status.current_power_state: %d, wait_aws: %d - %d", 3,
                         s_current_status.current_power_state, s_visible_pending_request.need_wait_aws, s_current_status.aws_connected);
        /* Enable BT visibility when BT enabled and AW connected or no need wait AWS.*/
        if (s_current_status.current_power_state == APP_BT_STATE_POWER_STATE_ENABLED) {
            if (!s_visible_pending_request.need_wait_aws || s_current_status.aws_connected) {
                s_visible_pending_request.bt_visible = false;
                bt_cm_discoverable(true);
            }
        }
    }
}

/******************************************************************************
******************************* Public APIs ***********************************
******************************************************************************/
void app_bt_state_service_set_bt_on_off(bool on, bool classic_off, bool need_do_rho, bool for_system_off)
{
    APPS_LOG_MSGID_I(LOG_TAG" app_bt_state_service_set_bt_on_off ? %d, classic_off: %d, try_rho : %d, for_system_off: %d", 4,
                     on, classic_off, need_do_rho, for_system_off);

    /* Should turn off BT and power off if current s_for_system_off is TRUE. */
    if (for_system_off) {
        s_for_system_off = for_system_off;
    }
    if (s_for_system_off) {
        s_current_status.target_power_state = APP_BT_STATE_POWER_STATE_DISABLED;
        /* No need to enable BT visibility if target BT state is off. */
        s_visible_pending_request.bt_visible = false;
    } else if (!on) {
        if (classic_off) {
            s_current_status.target_power_state = APP_HOME_SCREEN_BT_POWER_CLASSIC_DISABLED;
        } else {
            s_current_status.target_power_state = APP_BT_STATE_POWER_STATE_DISABLED;
        }
#ifdef AIR_WIRELESS_MIC_ENABLE
        app_wireless_mic_stop_local_recorder();
#endif
    } else {
        s_current_status.target_power_state = APP_BT_STATE_POWER_STATE_ENABLED;
        /* No need RHO if target BT state is on. */
        need_do_rho = false;
    }
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE)
    if (!s_for_system_off) {
        bool on_off = (s_current_status.target_power_state == APP_BT_STATE_POWER_STATE_ENABLED);
        app_race_cmd_co_sys_send_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT,
                                       &on_off, sizeof(on_off), false);
    }
#endif
#ifdef MTK_AWS_MCE_ENABLE
    if (!on && s_current_status.in_air_pairing) {
        bt_aws_mce_srv_air_pairing_stop();
    }
#endif
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_RETRY_POWER_ON_OFF);
    app_bt_state_service_check_and_do_bt_enable_disable(need_do_rho);
}

bool app_bt_state_service_set_bt_visible(bool enable_visible, bool wait_aws_connect, uint32_t timeout)
{
#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
    if (enable_visible && bt_aws_mce_srv_get_link_type() == BT_AWS_MCE_SRV_LINK_NONE) {
        APPS_LOG_MSGID_E(LOG_TAG"[SWIFT_PAIR] set_bt_visible, cannot visible when AWS disconnected", 0);
        return FALSE;
    }
#endif

#if defined(AIR_WIRELESS_MIC_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
    s_current_status.bt_visible = enable_visible;
    app_bt_state_service_notify_bt_visible(enable_visible);

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_AUTO_START_BT_VISIBLE);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
    if (enable_visible && timeout > BT_VISIBLE_TIMEOUT_INVALID) {
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT, NULL, 0,
                            NULL, timeout);
    }
    APPS_LOG_MSGID_I(LOG_TAG"[WIRELESS_MIC] set_bt_visible enable, visible=%d wait_aws=%d timeout=%d",
                     3, enable_visible, wait_aws_connect, timeout);
    return TRUE;

#else
    bool ret = true;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    APPS_LOG_MSGID_I(LOG_TAG" app_bt_state_service_set_bt_visible, [%02X] enable=%d need_wait_aws=%d current=%d",
                     4, role, enable_visible, wait_aws_connect, s_current_status.bt_visible);

#ifdef AIR_TWS_ENABLE
    if (s_current_status.in_air_pairing) {
        s_visible_pending_request.bt_visible = false;
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
        return ret;
    }
#endif
#ifdef AIR_MCSYNC_SHARE_ENABLE
    bt_mcsync_share_role_t share_role = bt_mcsync_share_get_role();
    if ((share_role & BT_MCSYNC_SHARE_ROLE_FOLLOWER) == BT_MCSYNC_SHARE_ROLE_FOLLOWER) {
        return true;
    }
#endif

#ifdef AIR_SPEAKER_ENABLE
    if (role == BT_AWS_MCE_ROLE_CLINET) {
        return FALSE;
    } else if (BT_AWS_MCE_SRV_MODE_DOUBLE != bt_aws_mce_srv_get_mode()) {
        wait_aws_connect = false;
    }
#endif

    /* No action if wanted BT visibility and current BT status is same. */
    if (enable_visible == s_current_status.bt_visible
#ifdef MTK_AWS_MCE_ENABLE
        && (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
            || bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_NONE)
#endif
       ) {
        if (enable_visible) {
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
            app_ull_take_over_enable_visible();
#endif
            /* Refresh BT visibility duration if enable_visible is TRUE. */
            app_bt_state_service_refresh_visible_timeout(timeout);
#ifdef AIR_SWIFT_PAIR_ENABLE
            app_swift_pair_restart_adv();
#endif
        }
        return true;
    }
    if (enable_visible) {
        /* No action if current BT is off and target BT state isn't on or no need to wait AWS connection. */
        if (APP_BT_STATE_POWER_STATE_DISABLING >= s_current_status.current_power_state
            && (APP_BT_STATE_POWER_STATE_ENABLED != s_current_status.target_power_state || !wait_aws_connect)) {
            return false;
        } else if (APP_BT_STATE_POWER_STATE_ENABLED == s_current_status.current_power_state) {
#ifdef MTK_AWS_MCE_ENABLE
            /* Partner send BT visibility request to Agent when BT on and AWS connected. */
            if ((!wait_aws_connect) && s_current_status.aws_connected && bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
                app_bt_state_service_bt_visible_request_t aws_data = {
                    .bt_visible = TRUE,
                    .need_wait_aws = wait_aws_connect,
                    .timeout = timeout
                };
                if (BT_STATUS_SUCCESS != apps_aws_sync_event_send_extra(
                        EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_SET_BT_VISIBLE,
                        &aws_data,
                        sizeof(aws_data))) {
                    ret = false;
                } else {
                    s_visible_pending_request.bt_visible = FALSE;
                    app_bt_state_service_refresh_visible_timeout(timeout);
                    ret = true;
                }
            } else if (wait_aws_connect && !s_current_status.aws_connected) {
                /* Enable BT visibility pending request when BT on and AWS not connected. */
                s_visible_pending_request.bt_visible = TRUE;
                s_visible_pending_request.need_wait_aws = wait_aws_connect;
                s_visible_pending_request.timeout = timeout;
                app_bt_state_service_refresh_visible_timeout(timeout);
            } else if (wait_aws_connect
                       && bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_AGENT
                       && bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_NONE) {
                s_visible_pending_request.bt_visible = FALSE;
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
            } else
#endif
            {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_DISCOVERABLE, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_DISCOVER);
                app_ull_take_over_enable_visible();
                if (!wait_aws_connect) {
                    bt_app_common_apply_ultra_low_latency_retry_count();
                }
#endif
                /* Enable BT visibility, disable BT visibility pending request. */
                bt_cm_discoverable(true);
                app_bt_state_service_refresh_visible_timeout(timeout);
                s_visible_pending_request.bt_visible = FALSE;
                APPS_LOG_MSGID_I(LOG_TAG" Set discoverable", 0);
            }
        } else {
            /* Enable BT visibility pending request when BT off. */
            s_visible_pending_request.bt_visible = TRUE;
            s_visible_pending_request.need_wait_aws = wait_aws_connect;
            app_bt_state_service_refresh_visible_timeout(timeout);
        }
    } else {
#ifdef MTK_AWS_MCE_ENABLE
        /* Partner send BT visibility(off) request to Agent. */
        if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
            app_bt_state_service_bt_visible_request_t aws_data = {
                .bt_visible = FALSE,
                .need_wait_aws = 0,
                .timeout = 0
            };
            if (BT_STATUS_SUCCESS != apps_aws_sync_event_send_extra(
                    EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                    APPS_EVENTS_INTERACTION_SET_BT_VISIBLE,
                    &aws_data,
                    sizeof(aws_data))) {
                ret = false;
            }
        } else if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_CLINET)
#endif
        {
            /* Agent stopped BT visibility. */
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_DISCOVERABLE, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
            app_ull_take_over_disable_visible();
            bt_app_common_apply_ultra_low_latency_retry_count();
#endif
            bt_cm_discoverable(false);
        }
        s_visible_pending_request.bt_visible = false;
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
    }

    return ret;
#endif
}

const app_bt_state_service_status_t *app_bt_connection_service_get_current_status(void)
{
    return &s_current_status;
}

#ifdef MTK_AWS_MCE_ENABLE
void app_bt_state_service_set_air_pairing_doing(bool doing)
{
    APPS_LOG_MSGID_I(LOG_TAG" app_bt_state_service_set_air_pairing_doing %d -> %d", 2, s_current_status.in_air_pairing, doing);
    if (s_current_status.in_air_pairing && !doing) {
        s_current_status.in_air_pairing = doing;
        app_bt_state_service_check_and_do_bt_enable_disable(false);
    } else {
        if (doing) {
            s_visible_pending_request.bt_visible = false;
        }
        s_current_status.in_air_pairing = doing;
    }
}
#endif

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
void app_bt_state_service_set_ull_air_pairing_doing(bool doing)
{
    APPS_LOG_MSGID_I(LOG_TAG" app_bt_state_service_set_ull_air_pairing_doing %d -> %d", 2, s_current_status.in_ull_pairing, doing);
    if (s_current_status.in_ull_pairing && !doing) {
        s_current_status.in_ull_pairing = doing;
        app_bt_state_service_check_and_do_bt_enable_disable(false);
    } else {
        s_current_status.in_ull_pairing = doing;
    }
}
#endif

/******************************************************************************
****************************** Process events *********************************
******************************************************************************/
static bool app_bt_state_service_process_interaction_events(uint32_t event_id,
                                                            void *extra_data,
                                                            size_t data_len)
{
    bool ret = false;
    switch (event_id) {
        /* The old Agent will switch to new Partner if RHO successfully. */
        case APPS_EVENTS_INTERACTION_RHO_END:
            /* The new partner need to check and turn off BT (no need try to trigger RHO). */
            app_bt_state_service_check_and_do_bt_enable_disable(false);
            break;
        case APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT:
            APPS_LOG_MSGID_I(LOG_TAG" received BT visible timeout", 0);
#if defined(AIR_WIRELESS_MIC_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
            s_current_status.bt_visible = FALSE;
            app_bt_state_service_notify_bt_visible(FALSE);
#else
#ifdef MTK_AWS_MCE_ENABLE
            /* The Agent should disable BT visibility when BT visibility duration timeout. */
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT
                || bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_NONE)
#endif
            {
                s_visible_pending_request.bt_visible = false;
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_DISCOVERABLE, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
                app_ull_take_over_disable_visible();
                bt_app_common_apply_ultra_low_latency_retry_count();
#endif
                if (s_current_status.bt_visible) {
                    bt_cm_discoverable(false);
                }
              voice_prompt_param_t vp = {0};
              vp.vp_index = VP_INDEX_EN_Pairing_out;   
              vp.control = VOICE_PROMPT_CONTROL_MASK_SYNC;
              vp.delay_time = 300;
              voice_prompt_play(&vp, NULL);
              APPS_LOG_MSGID_I(LOG_TAG" VP_INDEX_EN_Pairing_out BT visible timeout 111", 0);
            }

#endif
            ret = true;
            break;
        case APPS_EVENTS_INTERACTION_BT_RETRY_POWER_ON_OFF:
            app_bt_state_service_check_and_do_bt_enable_disable(false);
            ret = true;
            break;
#ifdef AIR_MULTI_POINT_ENABLE
        case APPS_EVENTS_INTERACTION_MULTI_POINT_ENABLE: {
            bool enable = (bool)extra_data;
            if (app_bt_emp_is_enable() != enable) {
                APPS_LOG_MSGID_I(LOG_TAG" switch multi point to %d, current state current: %d, target: %d", 3, enable, s_current_status.target_power_state, s_current_status.current_power_state);
#ifdef AIR_APP_A2DP_LBB_VENDOR_CODEC_LIMIT
                bt_sink_srv_a2dp_enable_vendor_codec(!enable);
#endif
                app_bt_emp_enable(enable, TRUE);
                if (s_current_status.current_power_state == APP_BT_STATE_POWER_STATE_ENABLED
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
                    && BT_AWS_MCE_SRV_LINK_NORMAL == bt_aws_mce_srv_get_link_type()
#endif
                   ) {
                    if (s_current_status.target_power_state == APP_BT_STATE_POWER_STATE_NONE_ACTION || s_current_status.target_power_state == APP_BT_STATE_POWER_STATE_ENABLED) {
                        s_current_status.target_power_state = APP_BT_STATE_POWER_STATE_DISABLED;
                        app_bt_state_service_check_and_do_bt_enable_disable(false);
                        s_current_status.target_power_state = APP_BT_STATE_POWER_STATE_ENABLED;
                    }
                }
            }
            break;
        }
#endif
        default:
            break;
    }

    return ret;
}

static bool app_bt_state_service_process_bt_cm_events(uint32_t event_id,
                                                      void *extra_data,
                                                      size_t data_len)
{
    bool ret = false;
    switch (event_id) {
        case BT_CM_EVENT_POWER_STATE_UPDATE: {
            bt_cm_power_state_update_ind_t *power_update = (bt_cm_power_state_update_ind_t *)extra_data;
            if (NULL == power_update) {
                break;
            }

            APPS_LOG_MSGID_I(LOG_TAG" power_update Power %x", 1, power_update->power_state);
            if (BT_CM_POWER_STATE_ON == power_update->power_state) {
                /* BT off switch to on. */
                s_current_status.connection_state = APP_BT_CONNECTION_SERVICE_BT_STATE_DISCONNECTED;

                app_bt_state_service_check_and_do_bt_visible();
                start_default_ble_adv();
            } else if (BT_CM_POWER_STATE_OFF == power_update->power_state) {
                /* BT on switch to off. */
                s_current_status.bt_visible = false;
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
                s_current_status.connection_state = APP_BT_CONNECTION_SERVICE_BT_STATE_BT_OFF;
                s_current_status.aws_connected = false;
            }
            break;
        }
        case BT_CM_EVENT_VISIBILITY_STATE_UPDATE: {
            bt_cm_visibility_state_update_ind_t *visible_update = (bt_cm_visibility_state_update_ind_t *)extra_data;
            if (NULL == visible_update) {
                break;
            }
            APPS_LOG_MSGID_I(LOG_TAG" visibility_state: %d", 1, visible_update->visibility_state);
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
                break;
            }
#endif
            /* Update BT visibility state. */
            s_current_status.bt_visible = visible_update->visibility_state;
            app_bt_state_service_notify_bt_visible(s_current_status.bt_visible);
            if (!s_current_status.bt_visible) {
                /* No need visibility_timeout event if BT visibility disabled. */
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
            } else {
                /* Disable BT visibility pending request if BT visibility enabled.*/
                s_visible_pending_request.bt_visible = false;
            }
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
                bt_status_t send_aws_status;
                app_bt_state_service_visible_state_notification_t notification_data = {
                    .bt_visible = s_current_status.bt_visible,
                    .timeout = (s_current_status.bt_visible && s_visible_timeout_time != BT_VISIBLE_TIMEOUT_INVALID) ?
                    s_visible_timeout_time - xTaskGetTickCount() * portTICK_PERIOD_MS : BT_VISIBLE_TIMEOUT_INVALID,
                };
                if (notification_data.timeout > 0x8FFFFFFF) {
                    notification_data.timeout = BT_VISIBLE_TIMEOUT_INVALID;
                }
                send_aws_status = apps_aws_sync_event_send_extra(
                                      EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_BT_VISIBLE_STATE_CHANGE,
                                      &notification_data,
                                      sizeof(notification_data));
                if (BT_STATUS_SUCCESS != send_aws_status) {
                    APPS_LOG_MSGID_I(LOG_TAG"Fail to send bt visible change to partner : %d", 1, visible_update->visibility_state);
                }
            }
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            uint32_t latency = APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK;
            extern bool app_ull_is_multi_link_mode(void);
            if (app_ull_is_multi_link_mode()) {
                if (visible_update->visibility_state) {
                    latency = BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_DISCOVER;
                } else {
                    app_ull_take_over_disable_visible();
                }
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_DISCOVERABLE, latency);
//                default_ble_adv_update();
            }
#endif
                default_ble_adv_update();
            break;
        }
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
#ifdef MTK_AWS_MCE_ENABLE
            bt_event_suffix_data_t *suffix_data = get_bt_event_suffix_data(extra_data, sizeof(bt_cm_remote_info_update_ind_t));
            bt_aws_mce_role_t role = suffix_data->aws_role;
#endif
            if (NULL == remote_update) {
                break;
            }

            APPS_LOG_MSGID_I(LOG_TAG" REMOTE_INFO_UPDATE, addr [%02X:%02X:%02X:%02X:%02X:%02X]", 6,
                             remote_update->address[5], remote_update->address[4], remote_update->address[3],
                             remote_update->address[2], remote_update->address[1], remote_update->address[0]);
            APPS_LOG_MSGID_I(LOG_TAG" REMOTE_INFO_UPDATE, role: %x, acl_state(0x%x)->(0x%x), connected_service(0x%x)->(0x%x), reason:0x%x", 6,
#ifdef MTK_AWS_MCE_ENABLE
                             role,
#else
                             0,
#endif
                             remote_update->pre_acl_state, remote_update->acl_state,
                             remote_update->pre_connected_service, remote_update->connected_service,
                             remote_update->reason);

#ifdef MTK_AWS_MCE_ENABLE
            if (BT_AWS_MCE_ROLE_AGENT == role || BT_AWS_MCE_ROLE_NONE == role)
#endif
            {
                if (BT_CM_ACL_LINK_ENCRYPTED != remote_update->pre_acl_state
                    && BT_CM_ACL_LINK_ENCRYPTED == remote_update->acl_state
                    && 0 != memcmp(remote_update->address, bt_device_manager_get_local_address(), sizeof(bt_bd_addr_t))) {
                    bt_cm_discoverable(false);
                    s_visible_pending_request.bt_visible = false;
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
                }
                if (!(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                    && (~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    /* Update BT state as PROFILE_CONNECTED when Agent connected first non-AWS profile. */
                    APPS_LOG_MSGID_I(LOG_TAG" Agent Connected", 0);
                    s_current_status.connection_state = APP_BT_CONNECTION_SERVICE_BT_STATE_PROFILE_CONNECTED;
                } else if (BT_CM_ACL_LINK_DISCONNECTED != remote_update->pre_acl_state
                           && BT_CM_ACL_LINK_DISCONNECTED == remote_update->acl_state) {
                    /* BT ACL disconnection happen. */
                    if (APP_BT_CONNECTION_SERVICE_BT_STATE_ACL_CONNECTED <= s_current_status.connection_state
                        && bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), NULL, 0) == 0) {
                        /* Update BT state as DISCONNECTED if Agent all non-AWS profile disconnected. */
                        APPS_LOG_MSGID_I(LOG_TAG" Agent Disconnected", 0);
                        s_current_status.connection_state = APP_BT_CONNECTION_SERVICE_BT_STATE_DISCONNECTED;
                    }
                } else if (BT_CM_ACL_LINK_CONNECTED != remote_update->pre_acl_state
                           && BT_CM_ACL_LINK_CONNECTED == remote_update->acl_state
                           && 0 != memcmp(remote_update->address, bt_device_manager_get_local_address(), sizeof(bt_bd_addr_t))) {
                    APPS_LOG_MSGID_I(LOG_TAG" Agent ACL Connected", 0);
                    /* Update BT state as ACL_CONNECTED if Agent ACL connected. */
                    if (s_current_status.connection_state < APP_BT_CONNECTION_SERVICE_BT_STATE_ACL_CONNECTED) {
                        s_current_status.connection_state = APP_BT_CONNECTION_SERVICE_BT_STATE_ACL_CONNECTED;
                    }
                }

#ifdef MTK_AWS_MCE_ENABLE
                if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                    && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    /* Agent update AWS connection state when AWS connected. */
                    s_current_status.aws_connected = true;
                    s_current_status.reason = remote_update->reason;
                    APPS_LOG_MSGID_I(LOG_TAG" aws connected reason=%d", 1, s_current_status.reason);
                    app_bt_state_service_visible_state_notification_t notification_data = {
                        .bt_visible = s_current_status.bt_visible,
                        .timeout = (s_current_status.bt_visible && s_visible_timeout_time != BT_VISIBLE_TIMEOUT_INVALID) ?
                                   s_visible_timeout_time - xTaskGetTickCount() * portTICK_PERIOD_MS : BT_VISIBLE_TIMEOUT_INVALID,
                    };
                    if (notification_data.timeout > 0x8FFFFFFF) {
                        notification_data.timeout = BT_VISIBLE_TIMEOUT_INVALID;
                    }
                    bt_status_t send_aws_status = apps_aws_sync_event_send_extra(
                                                      EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                      APPS_EVENTS_INTERACTION_BT_VISIBLE_STATE_CHANGE,
                                                      &notification_data,
                                                      sizeof(notification_data));
                    if (BT_STATUS_SUCCESS != send_aws_status) {
                        APPS_LOG_MSGID_I(LOG_TAG"Fail to send bt visible change to partner when aws connected", 0);
                    }

                    /* Check and enable BT visibility when Agent AWS connected. */
                    app_bt_state_service_check_and_do_bt_visible();
                    APPS_LOG_MSGID_I(LOG_TAG" Partner Attached.", 0);
#ifdef MTK_RACE_CMD_ENABLE
                    /* Agent send AWS state to APP via RACE command. */
                    race_bt_notify_aws_state(1);
#endif
                    start_default_ble_adv();
                } else if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                           && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    /* Agent update AWS connection state when AWS disconnected. */
                    s_current_status.aws_connected = false;
                    s_current_status.reason = remote_update->reason;
                    APPS_LOG_MSGID_I(LOG_TAG"Partner Detached. aws disconnected reason=%d", 1, remote_update->reason);
#ifdef MTK_RACE_CMD_ENABLE
                    race_bt_notify_aws_state(0);
#endif
                }
#endif
            }
#ifdef MTK_AWS_MCE_ENABLE
            else if (role == BT_AWS_MCE_ROLE_PARTNER || role == BT_AWS_MCE_ROLE_CLINET) {
                if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                    && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    /* Partner update AWS connection state when AWS connected. */
                    s_current_status.aws_connected = true;
                    s_current_status.reason = remote_update->reason;
                    APPS_LOG_MSGID_I(LOG_TAG" aws connected reason=%d", 1, s_current_status.reason);
                    s_visible_pending_request.bt_visible = false;
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
                    if (BT_AWS_MCE_SRV_LINK_NORMAL == bt_aws_mce_srv_get_link_type()) {
                        /* Partner connected SP if AWS connected and link type is normal. */
                        s_current_status.connection_state = APP_BT_CONNECTION_SERVICE_BT_STATE_PROFILE_CONNECTED;
                    }
#ifdef MTK_RACE_CMD_ENABLE
                    race_bt_notify_aws_state(true);
#endif
                } else if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                           && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    if (APP_BT_CONNECTION_SERVICE_BT_STATE_ACL_CONNECTED <= s_current_status.connection_state) {
                        s_current_status.connection_state = APP_BT_CONNECTION_SERVICE_BT_STATE_DISCONNECTED;
                    }
                    s_current_status.aws_connected = false;
                    s_current_status.reason = remote_update->reason;
                    s_current_status.bt_visible = false;
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
                    APPS_LOG_MSGID_I(LOG_TAG" aws disconnected reason=%d", 1, s_current_status.reason);
#ifdef MTK_RACE_CMD_ENABLE
                    race_bt_notify_aws_state(false);
#endif
                }
            }
#endif
        }
        break;

        default:
            break;
    }
    return ret;
}

static bool app_bt_state_service_process_bt_device_manager_events(uint32_t event_id,
                                                                  void *extra_data,
                                                                  size_t data_len)
{
    bool ret = false;
    bt_device_manager_power_event_t evt;
    bt_device_manager_power_status_t status;
    bt_event_get_bt_dm_event_and_status(event_id, &evt, &status);
    APPS_LOG_MSGID_I(LOG_TAG" BT_DM Power. status: %x, evt: %x, current : %x", 3, status, evt, s_current_status.current_power_state);
    switch (evt) {
        case BT_DEVICE_MANAGER_POWER_EVT_PREPARE_ACTIVE:
            break;
        case BT_DEVICE_MANAGER_POWER_EVT_PREPARE_STANDBY:
            break;
        case BT_DEVICE_MANAGER_POWER_EVT_ACTIVE_COMPLETE:
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
                uint8_t mode = true;
                bt_hci_send_vendor_cmd(0xFE0C, (uint8_t *)&mode, sizeof(uint8_t));
#endif
                if (APP_BT_STATE_POWER_STATE_ENABLING == s_current_status.current_power_state) {
                    s_current_status.current_power_state = APP_BT_STATE_POWER_STATE_ENABLED;
                }
                app_bt_state_service_check_and_do_bt_enable_disable(false);
            }
            break;
        case BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE:
            if (BT_DEVICE_MANAGER_POWER_RESET_TYPE_NORMAL == status) {
                if (APP_HOME_SCREEN_BT_POWER_CLASSIC_DISABLING == s_current_status.current_power_state) {
                    s_current_status.current_power_state = APP_HOME_SCREEN_BT_POWER_CLASSIC_DISABLED;
                } else if (APP_BT_STATE_POWER_STATE_DISABLING == s_current_status.current_power_state) {
                    s_current_status.current_power_state = APP_BT_STATE_POWER_STATE_DISABLED;
                }
                app_bt_state_service_check_and_do_bt_enable_disable(false);
                s_visible_pending_request.bt_visible = false;
            }
            break;
        case BT_DEVICE_MANAGER_POWER_EVT_CLASSIC_ACTIVE_COMPLETE:
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
#if defined(AIR_WIRELESS_MIC_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
                bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_DISABLE);
                APPS_LOG_MSGID_I(LOG_TAG"[WIRELESS_MIC] disable page/inquiry scan when BT power on", 0);
#endif
                if (APP_BT_STATE_POWER_STATE_ENABLING == s_current_status.current_power_state) {
                    s_current_status.current_power_state = APP_BT_STATE_POWER_STATE_ENABLED;
                }
                app_bt_state_service_check_and_do_bt_enable_disable(false);
            }
            break;
        case BT_DEVICE_MANAGER_POWER_EVT_CLASSIC_STANDBY_COMPLETE:
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
                if (APP_HOME_SCREEN_BT_POWER_CLASSIC_DISABLING == s_current_status.current_power_state) {
                    s_current_status.current_power_state = APP_HOME_SCREEN_BT_POWER_CLASSIC_DISABLED;
                }
                app_bt_state_service_check_and_do_bt_enable_disable(false);
                s_visible_pending_request.bt_visible = false;
            }
            break;
        default:
            break;
    }
    return ret;
}

static bool app_bt_state_service_process_bt_sink_events(uint32_t event_id,
                                                        void *extra_data,
                                                        size_t data_len)
{
    bool ret = false;
#ifdef AIR_LE_AUDIO_ENABLE
    switch (event_id) {
        case LE_SINK_SRV_EVENT_REMOTE_INFO_UPDATE: {
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            bt_le_sink_srv_event_remote_info_update_t *update_ind = (bt_le_sink_srv_event_remote_info_update_t *)extra_data;
            if (update_ind == NULL) {
                break;
            }
            APPS_LOG_MSGID_I(LOG_TAG" [%02X] ble_audio_state=%d Link=%d->%d SRV=%d->%d",
                             6, role, s_current_status.ble_audio_state,
                             update_ind->pre_state, update_ind->state,
                             update_ind->pre_connected_service, update_ind->connected_service);
            if (update_ind->pre_state == BT_BLE_LINK_DISCONNECTED
                && update_ind->state == BT_BLE_LINK_CONNECTED) {
                if (s_current_status.ble_audio_state < APP_BLE_AUDIO_STATE_CONNECTED) {
                    s_current_status.ble_audio_state = APP_BLE_AUDIO_STATE_CONNECTED;
                }
            } else if (update_ind->pre_state == BT_BLE_LINK_CONNECTED
                       && update_ind->state == BT_BLE_LINK_DISCONNECTED) {
                if (APP_BLE_AUDIO_STATE_CONNECTED <= s_current_status.ble_audio_state) {
#ifdef AIR_LE_AUDIO_MULTIPOINT_ENABLE
                    bt_handle_t le_handle = bt_sink_srv_cap_check_links_state(BT_SINK_SRV_CAP_STATE_CONNECTED);
                    APPS_LOG_MSGID_I(LOG_TAG" BLE Link Disconnect, remain le_handle=0x%04X",
                                     1, le_handle);
                    if (BT_HANDLE_INVALID == le_handle) {
                        /* No any BLE link is connected. */
                        s_current_status.ble_audio_state = APP_BLE_AUDIO_STATE_DISCONNECTED;
                    }
#else
                    s_current_status.ble_audio_state = APP_BLE_AUDIO_STATE_DISCONNECTED;
#endif
                }
            } else if (update_ind->connected_service > 0) {
                s_current_status.ble_audio_state = APP_BLE_AUDIO_STATE_PROFILE_CONNECTED;
            }
        }
        break;
        default:
            break;
    }
#endif
    return ret;
}

static bool app_bt_state_service_process_bt_events(uint32_t event_id,
                                                   void *extra_data,
                                                   size_t data_len)
{
    bool ret = false;
    switch (event_id) {
        case BT_POWER_ON_CNF: {
#ifdef AIR_LE_AUDIO_ENABLE
            s_current_status.ble_audio_state = APP_BLE_AUDIO_STATE_DISCONNECTED;
#endif
            break;
        }
        case BT_POWER_OFF_CNF: {
#ifdef AIR_LE_AUDIO_ENABLE
            s_current_status.ble_audio_state = APP_BLE_AUDIO_STATE_BT_OFF;
#endif
            break;
        }
        default:
            break;
    }
    return ret;
}

#ifdef MTK_AWS_MCE_ENABLE
static bool app_bt_state_service_process_aws_data_events(uint32_t event_id,
                                                         void *extra_data,
                                                         size_t data_len)
{
    bool ret = false;
    uint32_t aws_event_group;
    uint32_t aws_event_id;
    void *p_extra_data = NULL;
    uint32_t extra_data_len = 0;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;

    if (!aws_data_ind || aws_data_ind->module_id != BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        return ret;
    }

    apps_aws_sync_event_decode_extra(aws_data_ind, &aws_event_group, &aws_event_id,
                                     &p_extra_data, &extra_data_len);
    switch (aws_event_group) {
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            switch (aws_event_id) {
                /* Update Partner BT visibility when Agent visibility changed or AWS connected. */
                case APPS_EVENTS_INTERACTION_BT_VISIBLE_STATE_CHANGE: {
                    app_bt_state_service_visible_state_notification_t visible_notification;
                    if (BT_AWS_MCE_ROLE_PARTNER == bt_device_manager_aws_local_info_get_role()
                        && p_extra_data && extra_data_len == sizeof(visible_notification)) {
                        memcpy(&visible_notification, p_extra_data, extra_data_len);
                        APPS_LOG_MSGID_I(LOG_TAG"Received bt_visible from agent : %d", 1,
                                         visible_notification.bt_visible);
                        if (s_current_status.bt_visible != visible_notification.bt_visible) {
                            bt_cm_discoverable(visible_notification.bt_visible);
                            if (visible_notification.bt_visible) {
                                if (visible_notification.timeout != BT_VISIBLE_TIMEOUT_INVALID) {
                                    app_bt_state_service_refresh_visible_timeout(visible_notification.timeout);
                                }
                            } else {
                                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
                            }
                            s_current_status.bt_visible = visible_notification.bt_visible;
                            app_bt_state_service_notify_bt_visible(s_current_status.bt_visible);
                        }
                    }
                }
                break;
                /* Agent trigger BT visibility on/off when Agent received SET_BT_VISIBLE event. */
                case APPS_EVENTS_INTERACTION_SET_BT_VISIBLE: {
                    app_bt_state_service_bt_visible_request_t *aws_data;
                    if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()
                        && p_extra_data && extra_data_len == sizeof(app_bt_state_service_bt_visible_request_t)) {

                        if (s_current_status.in_air_pairing) {
                            APPS_LOG_MSGID_I(LOG_TAG" app_bt_state_service_set_bt_visible but in air pairing", 0);
                            s_visible_pending_request.bt_visible = false;
                            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
                            ret = true;
                            break;
                        }

                        aws_data = (app_bt_state_service_bt_visible_request_t *)p_extra_data;
                        APPS_LOG_MSGID_I(LOG_TAG"Received set bt_visible from partner, enable: %d - timeout: %d", 2,
                                         aws_data->bt_visible, aws_data->timeout);
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                        bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_DISCOVERABLE, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_DISCOVER);
#endif
                        if (aws_data->bt_visible) {
                            voice_prompt_param_t vp = {0};
                            vp.vp_index = VP_INDEX_PAIRING;
                            voice_prompt_play(&vp, NULL);
                            APPS_LOG_MSGID_I("VP_INDEX_PAIRING 111", 0);
                           
                            //apps_config_set_vp(VP_INDEX_PAIRING, false, 100, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                        }

                        bt_cm_discoverable(aws_data->bt_visible);
                        app_bt_state_service_refresh_visible_timeout(aws_data->timeout);
                        s_visible_pending_request.bt_visible = false;
                    }
                    ret = true;
                }
                break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
    return ret;
}
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
static bool app_bt_state_proc_ull_event(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case BT_ULL_EVENT_LE_CONNECTED: {
            bt_ull_le_connected_info_t *con_info = (bt_ull_le_connected_info_t*)extra_data;
            if (con_info->status != BT_STATUS_SUCCESS) {
                break;
            }
            s_current_status.ble_ull_state = APP_BLE_ULL_STATE_CONNECTED;
            break;
        }
        case BT_ULL_EVENT_LE_DISCONNECTED:
            s_current_status.ble_ull_state = APP_BLE_ULL_STATE_DISCONNECTED;
            break;
        default:
            break;
    }
    return false;
}
#endif

bool app_bt_state_service_process_events(uint32_t event_group,
                                         uint32_t event_id,
                                         void *extra_data,
                                         size_t data_len)
{
    bool ret = false;
    switch (event_group) {
        /* UI Shell APP_INTERACTION events. */
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            ret = app_bt_state_service_process_interaction_events(event_id, extra_data, data_len);
            break;
        /* UI Shell BT Connection Manager events. */
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER:
            ret = app_bt_state_service_process_bt_cm_events(event_id, extra_data, data_len);
            break;
        /* UI Shell BT device Manager events. */
        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER:
            ret = app_bt_state_service_process_bt_device_manager_events(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_BT_SINK:
            ret = app_bt_state_service_process_bt_sink_events(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_BT: {
            ret = app_bt_state_service_process_bt_events(event_id, extra_data, data_len);
            break;
        }
#ifdef MTK_AWS_MCE_ENABLE
        /* UI Shell BT AWS_DATA events. */
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            ret = app_bt_state_service_process_aws_data_events(event_id, extra_data, data_len);
            break;
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY: {
            ret = app_bt_state_proc_ull_event(event_id, extra_data, data_len);
            break;
        }
#endif
        default:
            break;
    }

#ifdef AIR_BT_TAKEOVER_ENABLE
    app_bt_takeover_proc_ui_shell_event(event_group, event_id, extra_data, data_len);
#endif
    return ret;
}

bool app_bt_service_is_visible()
{
    return (s_current_status.bt_visible);
}

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
void app_bt_state_service_cancel_discoverable_mode(void)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (s_current_status.bt_visible && (role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_NONE)) {
        APPS_LOG_MSGID_I(LOG_TAG"[LEA] cancel discoverable mode", 0);
#if defined(AIR_WIRELESS_MIC_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
        app_bt_state_service_set_bt_visible(FALSE, FALSE, 0);
#else
        bt_cm_discoverable(FALSE);
        s_visible_pending_request.bt_visible = FALSE;
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_VISIBLE_TIMEOUT);
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_AUTO_START_BT_VISIBLE);
#endif
    }
}
#endif

uint32_t app_bt_state_service_get_connected_exclude_aws(bt_bd_addr_t *addr_list, uint32_t list_num)
{
    uint32_t ret = 0;
#ifdef MTK_AWS_MCE_ENABLE
    bt_bd_addr_t local_list[BT_MAX_LINK_NUM];
    uint32_t local_list_count = BT_MAX_LINK_NUM;
    uint32_t i;
    local_list_count = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE, local_list, local_list_count);

    for (i = 0; i < local_list_count && (addr_list == NULL || ret < list_num); i++) {
        if (memcmp(local_list[i], bt_device_manager_get_local_address(), sizeof(bt_bd_addr_t)) != 0) {
            if (addr_list) {
                memcpy(addr_list[ret], local_list[i], sizeof(bt_bd_addr_t));
            }
            ret++;
        }
    }
#else
    ret = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE, addr_list, list_num);
#endif
    return ret;
}

bool app_bt_state_client_connect_aws(bool connect)
{
    bool success = FALSE;
#if defined(AIR_SPEAKER_ENABLE) && defined(MTK_AWS_MCE_ENABLE)
    bt_status_t bt_status = BT_STATUS_FAIL;
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    uint32_t aws_link = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), NULL, 0);
    if (mode != BT_AWS_MCE_SRV_MODE_BROADCAST || role != BT_AWS_MCE_ROLE_CLINET) {
        APPS_LOG_MSGID_E(LOG_TAG"[APP_SPK] client_connect_aws, FAIL mode=%d role=%02X connect=%d",
                         3, mode, role, connect);
        return success;
    }

    if (connect) {
        bt_bd_addr_t *agent_addr = bt_connection_manager_device_local_info_get_peer_aws_address();
        bt_cm_connect_t connect_param = {{0}, 0};
        memcpy(connect_param.address, agent_addr, sizeof(bt_bd_addr_t));
        connect_param.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS);
        bt_status = bt_cm_connect(&connect_param);
    } else if (aws_link > 0) {
        bt_cm_connect_t disc_param = {{0}, 0};
        bt_bd_addr_t addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        memcpy(disc_param.address, &addr, sizeof(bt_bd_addr_t));
        disc_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
        bt_status = bt_cm_disconnect(&disc_param);
    }

    success = (bt_status == BT_STATUS_SUCCESS);
    APPS_LOG_MSGID_I(LOG_TAG"[APP_SPK] client_connect_aws, mode=%d role=%02X aws_link=%d connect=%d bt_status=0x%08X",
                     5, mode, role, aws_link, connect, bt_status);
#endif
    return success;
}

// richard for customer UI spec
bool app_bt_service_is_air_pairing()
{
    APPS_LOG_MSGID_I(LOG_TAG"[XIAOAI] app_bt_service_is_air_pairing %d",
                     1, s_current_status.in_air_pairing);
    return (s_current_status.in_air_pairing);
}

