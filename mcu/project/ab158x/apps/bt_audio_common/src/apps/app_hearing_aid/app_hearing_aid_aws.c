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

#include "app_hearing_aid_aws.h"
#include "app_hearing_aid_utils.h"
#include "app_hearing_aid_config.h"
#include "app_hearing_aid_activity.h"
#include "app_hearing_aid_key_handler.h"
#include "app_hear_through_storage.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "apps_debug.h"
#include "FreeRTOS.h"
#include "bt_device_manager.h"
#include "bt_connection_manager.h"
#include "bt_callback_manager.h"
#include "apps_aws_sync_event.h"
#include "apps_events_event_group.h"
#include "app_hear_through_race_cmd_handler.h"
#include "ui_shell_manager.h"
#include "apps_events_interaction_event.h"
#include "bt_system.h"
#include "hal_platform.h"
#include "hal_gpt.h"
#include "app_hearing_aid_storage.h"
#ifdef AIR_TWS_ENABLE
#include "bt_aws_mce_srv.h"
#endif /* AIR_TWS_ENABLE */

#if (defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)) && defined(AIR_TWS_ENABLE)

#define APP_HA_AWS_TAG        "[HearingAid][AWS]"

#define APP_HA_AWS_COMMAND_LENGTH           sizeof(uint8_t)
#define APP_HA_RSSI_READ_TIMEOUT                    (5 * 1000) // 5s
typedef struct {
    uint8_t             op_code;
    uint8_t             from_which_role;
    bool                need_sync_operate;
    bt_clock_t          target_clock;
    uint32_t            delay_ms;
    uint16_t            op_data_len;
    uint8_t             op_data[0];
} __attribute__((packed)) app_hearing_aid_aws_operate_command_t;

typedef struct {
    uint8_t             notify_role;
    uint32_t            notify_code;
    uint16_t            notify_data_len;
    uint8_t             notify_data[0];
} __attribute__((packed)) app_hearing_aid_aws_notification_t;

typedef struct {
    bool                is_vp_streaming;
    uint8_t             *configuration_parameter;
    uint32_t            configuration_parameter_len;
} app_hearing_aid_aws_configuration_sync_info_t;

app_hearing_aid_aws_configuration_sync_info_t app_ha_aws_config = {
    .is_vp_streaming = false,
    .configuration_parameter = NULL,
    .configuration_parameter_len = 0,
};

static void app_hearing_aid_aws_handle_middleware_configuration_sync_request(uint8_t *data, uint32_t data_len)
{
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
        app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner();
    }
}

/**
 * @brief Partner side handle the received middleware configuration from agent side.
 *
 * @param data
 * @param data_len
 */
static void app_hearing_aid_aws_handle_agent_middleware_configuration_sync(uint8_t *data, uint32_t data_len)
{
    if ((data == NULL) || (data_len == 0)) {
        return;
    }

    if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_PARTNER) {
        return;
    }

    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_handle_agent_middleware_configuration_sync] is_vp_streaming : %d, agent configuration length : %d",
                        2,
                        app_ha_aws_config.is_vp_streaming,
                        data_len);

    if (app_ha_aws_config.is_vp_streaming == true) {
        /**
         * @brief If VP is streaming, Do not reload configuration directly
         * When VP play finished, then reload configuration.
         */
        app_hearing_aid_aws_reset_middleware_configuration();

        app_ha_aws_config.configuration_parameter = (uint8_t *)pvPortMalloc(sizeof(uint8_t) * data_len);
        if (app_ha_aws_config.configuration_parameter == NULL) {
            APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_handle_agent_middleware_configuration_sync] Failed to allocate buffer for agent configuration : %d",
                                1,
                                data_len);
            return;
        }

        memset(app_ha_aws_config.configuration_parameter, 0, sizeof(uint8_t) * data_len);
        app_ha_aws_config.configuration_parameter_len = data_len;
        memcpy(app_ha_aws_config.configuration_parameter, data, data_len);
    } else {
        app_hearing_aid_utils_sync_runtime_parameters(data, data_len);
        //app_hearing_aid_utils_reload_configuration();
    }
}

extern void app_hearing_aid_activity_ha_utils_notify_handler(uint8_t role, uint32_t type, uint8_t *notify_data, uint16_t notify_data_len);

static void app_hearing_aid_aws_handle_notification(uint8_t *data, uint32_t data_len)
{
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_handle_notification] data : 0x%x, data_len : %d",
                     2,
                     data,
                     data_len);
    if ((data == NULL) || (data_len < sizeof(app_hearing_aid_aws_notification_t))) {
        return;
    }

    app_hearing_aid_aws_notification_t *notification = (app_hearing_aid_aws_notification_t *)data;
    app_hearing_aid_activity_ha_utils_notify_handler(notification->notify_role,
                                                     notification->notify_code,
                                                     notification->notify_data,
                                                     notification->notify_data_len);
}

static void app_hearing_aid_aws_handle_agent_user_configuration_sync(uint8_t *data, uint32_t data_len)
{
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_handle_agent_user_configuration_sync] data : 0x%x, data_len : %d",
                     2,
                     data,
                     data_len);

    if ((data == NULL) || (data_len == 0)) {
        return;
    }

    app_hearing_aid_storage_sync_user_configuration(data, data_len);
}

#if 0
static void app_hearing_aid_aws_handle_app_info_sync(uint8_t *data, uint32_t data_len)
{
    if ((data == NULL) || (data_len == 0)) {
        return;
    }

    app_hearing_aid_activity_handle_app_info_sync(data, data_len);
}
#endif

static bt_status_t app_hearing_aid_aws_gap_event_handler(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    if ((status == BT_STATUS_SUCCESS) && (msg == BT_GAP_READ_RAW_RSSI_CNF)) {
        bt_gap_read_rssi_cnf_t *rssi_cnf = (bt_gap_read_rssi_cnf_t *)buff;

#ifdef AIR_TWS_ENABLE
        if (app_hearing_aid_aws_is_connected() == true) {
            app_hearing_aid_aws_rssi_operate_t rssi_op = {0};
            rssi_op.new_rssi = rssi_cnf->rssi;
            app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_RSSI_OPERATION,
                                                        (uint8_t *)&(rssi_op),
                                                        sizeof(app_hearing_aid_aws_rssi_operate_t),
                                                        true,
                                                        APP_HEARING_AID_SYNC_EVENT_DEFAULT_TIMEOUT);
        } else {
#endif /* AIR_TWS_ENABLE */
            app_hearing_aid_activity_handle_rssi_operation(rssi_cnf->rssi);
#ifdef AIR_TWS_ENABLE
        }
#endif /* AIR_TWS_ENABLE */
    }
    return status;
}

void app_hearing_aid_aws_init()
{
    bt_status_t status = bt_callback_manager_register_callback(bt_callback_type_app_event,
                                                                MODULE_MASK_GAP,
                                                                (void *)app_hearing_aid_aws_gap_event_handler);

    if (status != BT_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_init] Failed to register RSSI GAP callback : 0x%x",
                            1,
                            status);
    }
}

void app_hearing_aid_aws_deinit()
{
    bt_status_t status = bt_callback_manager_deregister_callback(bt_callback_type_app_event,
                                                                    app_hearing_aid_aws_gap_event_handler);

    if (status != BT_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_deinit] Failed to deregister RSSI GAP callback : 0x%x",
                            1,
                            status);
    }
}

/**
 * @brief The AWS data handler
 *
 */
typedef void (*hearing_aid_aws_data_handler)(uint8_t *data, uint32_t data_len);

/**
 * @brief The AWS data handler table.
 *
 */
static const hearing_aid_aws_data_handler aws_data_handler_list[] = {
    app_hearing_aid_aws_handle_middleware_configuration_sync_request,
    app_hearing_aid_aws_handle_agent_middleware_configuration_sync,
    NULL, // app_hearing_aid_aws_handle_race_cmd_request,
    NULL, // app_hearing_aid_aws_handle_race_cmd_response,
    app_hearing_aid_aws_handle_notification,
    app_hearing_aid_aws_handle_agent_user_configuration_sync,
    // app_hearing_aid_aws_handle_app_info_sync,
};

void app_hearing_aid_aws_process_data(uint32_t aws_id, uint8_t *aws_data, uint32_t aws_data_len)
{
    // if (aws_data == NULL || aws_data_len == 0) {
    //     return;
    // }

    uint8_t handler_index = aws_id - APP_HEARING_AID_EVENT_ID_AWS_BEGIN;
    if (aws_data_handler_list[handler_index] != NULL) {
        aws_data_handler_list[handler_index](aws_data, aws_data_len);
    }
}

void app_hearing_aid_aws_send_middleware_configuration_sync_request()
{
    if ((bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER)
            && (app_hearing_aid_aws_is_connected() == true)
            && (app_hearing_aid_activity_is_out_case() == true)) {
        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_HEARING_AID,
                                        APP_HEARING_AID_EVENT_ID_AWS_MIDDLEWARE_CONFIGURATION_SYNC_REQUEST,
                                        NULL,
                                        0);
    }
}

/**
 * @brief Agent side send the middleware configuration to the partner side.
 *
 */
void app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner()
{
    /**
     * @brief Send the agent middleware configuration to partner side.
     */

    if (app_hearing_aid_aws_is_connected() == false) {
        // APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner] AWS not connected", 0);
        return;
    }

    if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_AGENT) {
        // APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner] Current is not agent role", 0);
        return;
    }

    /**
     * @brief If agent is in charger case should ignore the request
     */
    if (app_hearing_aid_activity_is_out_case() == false) {
        APPS_LOG_MSGID_W(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner] Ignore the sync request cause in charger case", 0);
        return;
    }

    uint16_t sync_buf_len = 0;
    uint8_t *sync_buf = audio_anc_psap_control_get_runtime_sync_parameter(&sync_buf_len);

    if (sync_buf == NULL || sync_buf_len == 0) {
        APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner] get runtime sync parameter error, %d - %d",
                            2,
                            sync_buf,
                            sync_buf_len);
        return;
    }

    bt_status_t status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_HEARING_AID,
                                                        APP_HEARING_AID_EVENT_ID_AWS_SYNC_MIDDLEWARE_CONFIGURATION,
                                                        sync_buf,
                                                        sync_buf_len);

    vPortFree(sync_buf);
    sync_buf = NULL;

    if (BT_STATUS_SUCCESS != status) {
        APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner] Send to partner failed", 0);
    }
}

void app_hearing_aid_aws_sync_agent_user_configuration_to_partner()
{
    uint8_t config_len = 0;
    uint8_t *config = app_hearing_aid_storage_get_user_configuration(&config_len);

    bt_status_t status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_HEARING_AID,
                                                        APP_HEARING_AID_EVENT_ID_AWS_SYNC_USER_CONFIGURATION,
                                                        config,
                                                        config_len);

    if (BT_STATUS_SUCCESS != status) {
        APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_agent_user_configuration_to_partner] Send to partner failed", 0);
    }
}

void app_hearing_aid_aws_reset_middleware_configuration()
{
    if (app_ha_aws_config.configuration_parameter != NULL) {
        vPortFree(app_ha_aws_config.configuration_parameter);
        app_ha_aws_config.configuration_parameter = NULL;
    }

    app_ha_aws_config.configuration_parameter_len = 0;
}

bool app_hearing_aid_aws_send_operate_command(uint8_t code,
                                                uint8_t *buf,
                                                uint16_t buf_len,
                                                bool need_execute_locally,
                                                uint32_t delay_ms)
{

    apps_aws_sync_send_future_sync_event(false,
                                            EVENT_GROUP_UI_SHELL_HEARING_AID,
                                            (APP_HEARING_AID_EVENT_SYNC_BASE + code),
                                            need_execute_locally,
                                            buf,
                                            buf_len,
                                            delay_ms);

    return true;
}

bool app_hearing_aid_aws_send_notification(uint8_t role, uint32_t code, uint8_t *notify_data, uint16_t notify_data_len)
{
    if (app_hearing_aid_aws_is_connected() == false) {
        return false;
    }

    uint32_t allocate_len = sizeof(app_hearing_aid_aws_notification_t) + notify_data_len;
    app_hearing_aid_aws_notification_t *notification = (app_hearing_aid_aws_notification_t *)pvPortMalloc(allocate_len);
    if (notification == NULL) {
        APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_send_notification] [ERROR] Failed to allocate notification buffer", 0);
        return true;
    }
    memset(notification, 0, allocate_len);
    notification->notify_role = role;
    notification->notify_code = code;
    notification->notify_data_len = notify_data_len;
    if ((notify_data_len > 0) && (notify_data != NULL)) {
        memcpy(notification->notify_data, notify_data, notify_data_len);
    }
    bt_status_t status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_HEARING_AID,
                                                        APP_HEARING_AID_EVENT_ID_AWS_NOTIFICATION,
                                                        (void *)notification,
                                                        allocate_len);

    vPortFree(notification);
    notification = NULL;

    if (status != BT_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_send_notification] [ERROR] Failed to send notification to agent", 0);
        return false;
    }

    return true;
}

bool app_hearing_aid_aws_is_connected()
{
    bt_aws_mce_srv_link_type_t link_type = bt_aws_mce_srv_get_link_type();
    if (link_type == BT_AWS_MCE_SRV_LINK_NONE) {
        return false;
    }
    return true;
}

bool app_hearing_aid_aws_set_vp_streaming_state(bool streaming)
{
    /**
     * @brief Fix issue
     * When partner power to reconnect to agent, agent sync the configuration to partner side.
     * partner side will deinitialize and initialize to make the configuration to be enabled, so
     * this will make power-on VP be cut, cannot hear all of the power-on VP.
     *
     */
    if (app_ha_aws_config.is_vp_streaming != streaming) {
        app_ha_aws_config.is_vp_streaming = streaming;

        APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_set_vp_streaming_state] role : 0x%02x, parameter_len : %d, parameter : 0x%x, streaming : %d",
                         4,
                         bt_device_manager_aws_local_info_get_role(),
                         app_ha_aws_config.configuration_parameter_len,
                         app_ha_aws_config.configuration_parameter,
                         streaming);

        if ((app_ha_aws_config.is_vp_streaming == false)
            && (app_ha_aws_config.configuration_parameter_len > 0)
            && (app_ha_aws_config.configuration_parameter != NULL)
            && (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER)) {
            app_hearing_aid_utils_sync_runtime_parameters(app_ha_aws_config.configuration_parameter, app_ha_aws_config.configuration_parameter_len);

            app_hearing_aid_aws_reset_middleware_configuration();
        }
    }

    return true;
}

#if 0
void app_hearing_aid_aws_sync_agent_app_info_to_partner(uint8_t *data, uint32_t data_len)
{
    if ((data != NULL) && (data_len != 0)) {
        bt_status_t status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_HEARING_AID,
                                                            APP_HEARING_AID_EVENT_ID_AWS_SYNC_APP_INFO,
                                                            (void *)data,
                                                            data_len);

        if (BT_STATUS_SUCCESS != status) {
            APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_agent_app_info_to_partner] Send to partner failed", 0);
        }
    }
}
#endif

static void app_hearing_aid_aws_sync_handle_control_ha(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_control_ha][SYNC] Handle HA control, from where : 0x%02x, current_role : 0x%02x",
                        2,
                        from_which_role,
                        current_role);

    app_hearing_aid_aws_sync_operate_ha_t *operate_ha = (app_hearing_aid_aws_sync_operate_ha_t *)data;

    app_hearing_aid_activity_operate_ha(operate_ha->from_key,
                                        operate_ha->which,
                                        operate_ha->mix_table_need_execute,
                                        operate_ha->is_origin_on,
                                        operate_ha->mix_table_to_enable,
                                        operate_ha->drc_to_enable);
}

static void app_hearing_aid_aws_sync_handle_bf_switch(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    bool enable = ((uint8_t *)data)[0];
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_bf_switch][SYNC] Handle BF switch, from where : 0x%02x, current_role : 0x%02x, enable : %d",
                        3,
                        from_which_role,
                        current_role,
                        enable);

    app_hearing_aid_key_handler_bf_mode_switch(enable, ((current_role == BT_AWS_MCE_ROLE_AGENT) ? true : false));
}

static void app_hearing_aid_aws_sync_handle_aea_switch(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    bool enable = ((uint8_t *)data)[0];
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_aea_switch][SYNC] Handle AEA switch, from where : 0x%02x, current_role : 0x%02x, enable : %d",
                        3,
                        from_which_role,
                        current_role,
                        enable);

    app_hearing_aid_key_handler_aea_switch(enable, ((current_role == BT_AWS_MCE_ROLE_AGENT) ? true : false));
}

static void app_hearing_aid_aws_sync_handle_master_channel_switch(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    uint8_t channel = ((uint8_t *)data)[0];
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_master_channel_switch][SYNC] Handle Mater MIC channel switch, from where : 0x%02x, current_role : 0x%02x, channel : %d",
                        3,
                        from_which_role,
                        current_role,
                        channel);
    app_hearing_aid_utils_master_mic_channel_switch_toggle(channel);
}

static void app_hearing_aid_aws_sync_handle_tunning_mode_switch(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_tunning_mode_switch][SYNC] Handle tunning mode switch, from_where : 0x%02x, current_role : 0x%02x",
                        2,
                        from_which_role,
                        current_role);

    if (current_role == from_which_role) {
        app_hearing_aid_utils_hearing_tuning_mode_toggle(false);
    } else {
        app_hearing_aid_utils_hearing_tuning_mode_toggle(true);
    }
}

static void app_hearing_aid_aws_sync_handle_change_level(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    app_hearing_aid_aws_lr_index_change_t *change = (app_hearing_aid_aws_lr_index_change_t *)data;
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_change_level][SYNC] Handle level change, from where : 0x%02x, current_role : 0x%02x, l_index : %d, r_index : %d",
                        4,
                        from_which_role,
                        current_role,
                        change->l_index,
                        change->r_index);

    app_hearing_aid_key_handler_adjust_level(change->l_index, change->r_index, ((current_role == BT_AWS_MCE_ROLE_AGENT) ? true : false));
}

static void app_hearing_aid_aws_sync_handle_change_volume(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    app_hearing_aid_aws_lr_index_with_direction_change_t *change = (app_hearing_aid_aws_lr_index_with_direction_change_t *)data;
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_change_volume][SYNC] Handle volume change, from where : 0x%02x, current_role : 0x%02x, l_index : %d, r_index : %d, up : %d",
                        5,
                        from_which_role,
                        current_role,
                        change->l_index,
                        change->r_index,
                        change->up);

    app_hearing_aid_key_handler_adjust_volume(change->l_index, change->r_index, change->up, ((current_role == BT_AWS_MCE_ROLE_AGENT) ? true : false));
}

static void app_hearing_aid_aws_sync_handle_change_mode(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    uint8_t target_mode = ((uint8_t *)data)[0];
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_change_mode][SYNC] Handle mode change, from where : 0x%02x, current_role : 0x%02x, target_mode : %d",
                        3,
                        from_which_role,
                        current_role,
                        target_mode);

    app_hearing_aid_key_handler_adjust_mode(target_mode, ((current_role == BT_AWS_MCE_ROLE_AGENT) ? true : false));
}

static void app_hearing_aid_aws_sync_handle_change_mode_index(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    uint8_t target_mode = ((uint8_t *)data)[0];
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_change_mode_index][SYNC] Handle mode index change, from where : 0x%02x, current_role : 0x%02x, target_mode : %d",
                        3,
                        from_which_role,
                        current_role,
                        target_mode);
    bool ret = app_hearing_aid_utils_set_mode_index(&target_mode);

    /**
     * @brief Fix issue - 47207
     * Send mode index changed notification when changed locally
     * to make sure the mode index has been effective in the middleware.
     */
    if ((ret == true) && (current_role == BT_AWS_MCE_ROLE_AGENT)) {
        app_hear_through_race_cmd_send_notification(APP_HEARING_AID_CONFIG_TYPE_MODE_INDEX, data, data_len);
    }
}

static void app_hearing_aid_aws_sync_handle_set_user_switch(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    bool enable = ((uint8_t *)data)[0];
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_set_user_switch][SYNC] Handle user switch, from where : 0x%02x, current_role : 0x%02x, enable : %d",
                        3,
                        from_which_role,
                        current_role,
                        enable);

    app_hearing_aid_utils_set_user_switch(enable);
}

static void app_hearing_aid_aws_sync_handle_sync_vp_play(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    app_hearing_aid_aws_sync_vp_play_t *sync_vp_play = (app_hearing_aid_aws_sync_vp_play_t *)data;

    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_sync_vp_play][SYNC] Handle VP play, from where : 0x%02x, current_role : 0x%02x, vp_index : %d",
                        3,
                        from_which_role,
                        current_role,
                        sync_vp_play->vp_index);

    app_hearing_aid_activity_play_vp(sync_vp_play->vp_index, true);
}

static void app_hearing_aid_aws_sync_handle_power_off_request(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_power_off_request][SYNC] Handle power off request, from where : 0x%02x, current_role : 0x%02x",
                        2,
                        from_which_role,
                        current_role);

    app_hearing_aid_activity_set_powering_off();

    ui_shell_send_event(false,
                        EVENT_PRIORITY_HIGHEST,
                        EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF,
                        NULL,
                        0,
                        NULL,
                        0);
}

static void app_hearing_aid_aws_handle_combine_get_response(uint16_t op_type,
                                                            bool local,
                                                            uint8_t *response,
                                                            uint16_t response_len)
{
    uint16_t combine_response_len = app_hearing_aid_utils_get_combine_response_length(op_type);
    uint8_t *combine_response = NULL;

    if (combine_response_len != 0) {
        combine_response = (uint8_t *)pvPortMalloc(combine_response_len);
        if (combine_response == NULL) {
            APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_handle_combine_get_response] Handle get command, failed to allocate buffer for type : %s, combine_response_len : %d",
                                2,
                                app_hearing_aid_type_string[op_type],
                                combine_response_len);
            return;
        }

        bool result = app_hearing_aid_utils_handle_get_combine_response(op_type,
                                                                        local,
                                                                        response,
                                                                        response_len,
                                                                        combine_response,
                                                                        &combine_response_len);

        if (result == true) {
            app_hear_through_race_cmd_send_get_response(op_type, RACE_ERRCODE_SUCCESS, combine_response, combine_response_len);
        }

        vPortFree(combine_response);
        combine_response = NULL;
    } else {
        APPS_LOG_MSGID_W(APP_HA_AWS_TAG"[app_hearing_aid_aws_handle_combine_get_response] Handle get command, Failed to get combine response length for type : %s",
                            1,
                            app_hearing_aid_type_string[op_type]);
    }
}

static void app_hearing_aid_aws_sync_handle_race_cmd_request(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    app_hear_through_request_t *request = (app_hear_through_request_t *)data;
    uint8_t where_to_execute = app_hearing_aid_config_get_where_to_execute(request->op_code, request->op_type);

    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_race_cmd_request][SYNC] Handle race cmd, from_where : 0x%02x, current_role : 0x%02x, code : %s, type : %s, where_execute : %s",
                        5,
                        from_which_role,
                        current_role,
                        app_hearing_aid_command_string[request->op_code],
                        app_hearing_aid_type_string[request->op_type],
                        app_hearing_aid_execute_where_string[where_to_execute]);

    if (request->op_code == APP_HEAR_THROUGH_CMD_OP_CODE_GET) {
        uint8_t get_response[APP_HEARING_AID_RESPONSE_MAX_LEN] = {0};
        uint16_t get_response_len = 0;

        if (request->op_type == APP_HEARING_AID_CONFIG_TYPE_FEEDBACK_DETECTION) {
            if (from_which_role == BT_AWS_MCE_ROLE_PARTNER) {
                app_hearing_aid_activity_set_fbd_directly();
            }
            app_hearing_aid_activity_handle_get_race_cmd(data, data_len, get_response, &get_response_len);
        } else {
            /**
             * @brief Get command response firstly.
             * If no need to execute on both side, just send the get response to SP directly.
             * If need to execute on both sides, need to store the data in the temp buffer to wait the remote complete the get response data.
             */
            app_hearing_aid_activity_handle_get_race_cmd(data, data_len, get_response, &get_response_len);

            if (from_which_role == current_role) {
                /**
                 * @brief Execute the race command locally.
                 */
                if (where_to_execute != APP_HEARING_AID_EXECUTE_ON_BOTH) {
                    app_hear_through_race_cmd_send_get_response(request->op_type, RACE_ERRCODE_SUCCESS, get_response, get_response_len);
                } else {
                    app_hearing_aid_aws_handle_combine_get_response(request->op_type, true, get_response, get_response_len);
                }
            } else {
                /**
                 * @brief Execute the race command remote.
                 * If need execute on both side, send the get response data to remote side, the remote
                 * side will combine the get response data in one packet, then relay to SP.
                 */
                if (where_to_execute == APP_HEARING_AID_EXECUTE_ON_BOTH) {
                    app_hearing_aid_aws_race_cmd_op_response_t *cmd = (app_hearing_aid_aws_race_cmd_op_response_t *)pvPortMalloc(sizeof(app_hearing_aid_aws_race_cmd_op_response_t) + get_response_len);

                    if (cmd == NULL) {
                        APPS_LOG_MSGID_W(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_race_cmd_request] Handle get command, Failed to allocate buffer for race cmd response", 0);
                        return;
                    }

                    cmd->cmd_op_code = request->op_code;
                    cmd->cmd_op_type = request->op_type;
                    cmd->cmd_op_data_len = get_response_len;
                    memcpy(cmd->cmd_op_data, get_response, get_response_len);

                    app_hearing_aid_aws_send_operate_command(APP_HEARING_AID_OP_COMMAND_RACE_CMD_RESPONSE,
                                                                (uint8_t *)cmd,
                                                                sizeof(app_hearing_aid_aws_race_cmd_op_response_t) + get_response_len,
                                                                false,
                                                                0);

                    vPortFree(cmd);
                    cmd = NULL;
                }
            }
        }
    } else {
        bool result = app_hearing_aid_activity_handle_set_race_cmd(data, data_len);

        if (from_which_role == current_role) {
            app_hear_through_race_cmd_send_set_response(request->op_type, ((result == true) ? RACE_ERRCODE_SUCCESS : RACE_ERRCODE_FAIL));
        }
    }
}

static void app_hearing_aid_aws_sync_handle_race_cmd_response(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    if (from_which_role != current_role) {
        app_hearing_aid_aws_race_cmd_op_response_t *cmd = (app_hearing_aid_aws_race_cmd_op_response_t *)data;

        if (cmd->cmd_op_code == APP_HEAR_THROUGH_CMD_OP_CODE_GET) {
            app_hearing_aid_aws_handle_combine_get_response(cmd->cmd_op_type,
                                                            false,
                                                            cmd->cmd_op_data,
                                                            cmd->cmd_op_data_len);
        }
    }
}

static void app_hearing_aid_aws_sync_handle_rssi_operation(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len)
{
    app_hearing_aid_aws_rssi_operate_t *rssi_op = (app_hearing_aid_aws_rssi_operate_t *)data;

    if (rssi_op != NULL) {
        app_hearing_aid_activity_handle_rssi_operation(rssi_op->new_rssi);
    } else {
        APPS_LOG_MSGID_E(APP_HA_AWS_TAG"[app_hearing_aid_aws_sync_handle_rssi_operation][SYNC] RSSI operation data is NULL", 0);
    }
}

typedef void (*ha_sync_execute_event_handler)(uint8_t from_which_role, uint8_t current_role, void *data, size_t data_len);

static ha_sync_execute_event_handler app_hearing_aid_sync_execute_event_handler[] = {
    NULL,                                                      // 0x00
    app_hearing_aid_aws_sync_handle_control_ha,                // 0x01
    app_hearing_aid_aws_sync_handle_bf_switch,                 // 0x02
    app_hearing_aid_aws_sync_handle_aea_switch,                // 0x03
    app_hearing_aid_aws_sync_handle_master_channel_switch,     // 0x04
    app_hearing_aid_aws_sync_handle_tunning_mode_switch,       // 0x05
    app_hearing_aid_aws_sync_handle_change_level,              // 0x06
    app_hearing_aid_aws_sync_handle_change_volume,             // 0x07
    app_hearing_aid_aws_sync_handle_change_mode,               // 0x08
    app_hearing_aid_aws_sync_handle_change_mode_index,         // 0x09
    app_hearing_aid_aws_sync_handle_set_user_switch,           // 0x0A
    app_hearing_aid_aws_sync_handle_sync_vp_play,              // 0x0B
    app_hearing_aid_aws_sync_handle_power_off_request,         // 0x0C
    app_hearing_aid_aws_sync_handle_race_cmd_request,          // 0x0D
    app_hearing_aid_aws_sync_handle_race_cmd_response,         // 0x0E
    app_hearing_aid_aws_sync_handle_rssi_operation,            // 0x0F
};

void app_hearing_aid_aws_handle_sync_execute_event(uint32_t event_id,
                                                    void *extra_data,
                                                    uint32_t data_len)
{
    uint32_t event = event_id - APP_HEARING_AID_EVENT_SYNC_BASE;

    APPS_LOG_MSGID_I(APP_HA_AWS_TAG"[app_hearing_aid_aws_handle_sync_execute_event][SYNC] event : 0x%02x, extra_data : 0x%04x, data_len : %d",
                        3,
                        event,
                        extra_data,
                        data_len);

    if (event_id < APP_HEARING_AID_EVENT_SYNC_BASE) {
        return;
    }

    apps_aws_sync_future_event_local_event_t *local_event = (apps_aws_sync_future_event_local_event_t *)extra_data;
    bt_aws_mce_role_t cur_role = bt_device_manager_aws_local_info_get_role();
    if (app_hearing_aid_sync_execute_event_handler[event_id - APP_HEARING_AID_EVENT_SYNC_BASE] != NULL) {
        app_hearing_aid_sync_execute_event_handler[event_id - APP_HEARING_AID_EVENT_SYNC_BASE](local_event->from_while_role,
                                                                                                cur_role,
                                                                                                local_event->extra_data,
                                                                                                local_event->extra_data_len);
    }
}

void app_hearing_aid_aws_handle_connected(bool need_middleware_sync)
{
    app_hearing_aid_aws_sync_agent_user_configuration_to_partner();

    if (need_middleware_sync == true) {
        app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner();
    }

    app_hearing_aid_aws_remove_rssi_reading_event();
    app_hearing_aid_aws_send_rssi_reading_event();
}

void app_hearing_aid_aws_handle_disconnected()
{
    app_hearing_aid_aws_remove_rssi_reading_event();

    app_hearing_aid_aws_reset_middleware_configuration();
}

void app_hearing_aid_aws_send_rssi_reading_event()
{
    int8_t config_rssi = app_hear_through_storage_get_ha_rssi_threshold();
    bool is_aws_connected = app_hearing_aid_aws_is_connected();
    if ((bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT)
            && (config_rssi != 0)
            && (is_aws_connected == true)) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_RSSI_READ);
        ui_shell_send_event(false,
                            EVENT_PRIORITY_MIDDLE,
                            EVENT_GROUP_UI_SHELL_HEARING_AID,
                            APP_HEARING_AID_EVENT_ID_RSSI_READ,
                            NULL,
                            0,
                            NULL,
                            APP_HA_RSSI_READ_TIMEOUT);
    }
}

void app_hearing_aid_aws_remove_rssi_reading_event()
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEARING_AID, APP_HEARING_AID_EVENT_ID_RSSI_READ);
}

#endif /* (AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE) && AIR_TWS_ENABLE */

