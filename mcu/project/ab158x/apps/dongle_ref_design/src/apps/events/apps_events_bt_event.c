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
 * File: apps_events_bt_event.c
 *
 * Description: This file defines callback of BT and send events to APPs
 *
 */

#include "ui_shell_manager.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "apps_events_bt_event.h"
#include "apps_events_key_event.h"
#include "apps_events_interaction_event.h"
//#include "multi_va_event_id.h"
#include "bt_gap_le.h"
#include "bt_sink_srv.h"
#include "bt_connection_manager.h"
#include "bt_device_manager_power.h"
#include "bt_aws_mce_srv.h"
#include "bt_device_manager.h"
#ifdef MTK_RACE_CMD_ENABLE
#include "race_cmd_feature.h"
#include "race_event.h"
#endif
#if defined AIR_BT_ULTRA_LOW_LATENCY_ENABLE
#include "apps_race_cmd_event.h"
#endif
#include "bt_app_common.h"
#ifdef __BT_FAST_PAIR_ENABLE__
#include "bt_fast_pair.h"
#endif

#include "bt_callback_manager.h"
#include "FreeRTOS.h"
#include "bt_power_on_config.h"
#include "nvkey_id_list.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#include "apps_config_led_manager.h"
#endif
#include "bt_init.h"

#ifdef MTK_AMA_ENABLE
#include "Ama_Interface.h"
//#include "app_ama_audio.h"
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#include "bt_ull_service.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
#include "bt_ull_le_hid_service.h"
#endif

#include "bt_app_common.h"
#ifdef MTK_RACE_CMD_ENABLE
#include "apps_race_cmd_event.h"
#endif
#define LOG_TAG         "[bt_evt]"

bt_event_suffix_data_t *get_bt_event_suffix_data(void *event_params, size_t param_len)
{
    if (event_params) {
        /* Because the param is 4 bytes align. */
        size_t fixed_len = ((param_len + 3) >> 2) << 2;
        return (bt_event_suffix_data_t *)((uint8_t *)event_params + fixed_len);
    } else {
        return NULL;
    }
}

/**
 * @brief      The implementation of the weak symbol bt_sink_srv_event_callback which declared in bt_sink_srv.h.
 */
void bt_sink_srv_event_callback(bt_sink_srv_event_t event_id, void *param, uint32_t param_len)
{
    void *event_params = NULL;
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
#endif
    APPS_LOG_MSGID_I("[UI_SHELL], Srv_event_callback :%x", 1, event_id);

    /*
        if (bt_power_on_get_config_type() == BT_POWER_ON_DUT && event_id == BT_CM_EVENT_VISIBILITY_STATE_UPDATE) {
            bt_bd_addr_t empty_addr = {0};
            bt_cm_visibility_state_update_ind_t *visible_update = (bt_cm_visibility_state_update_ind_t *)param;
            if (!memcmp(bt_device_manager_aws_local_info_get_peer_address(), &empty_addr, sizeof(bt_bd_addr_t))
                    && !visible_update->visibility_state) {
                bt_cm_discoverable(true);
            }
        }
        */

    /* bt_app_common need process the callback in BT task. */
    bt_app_common_on_bt_sink_event_callback(event_id, param, param_len);

    if (NULL != param) {
        /* Calculate the total length, it's 4 bytes align. */
        size_t total_len = (((param_len + 3) >> 2) << 2) + sizeof(bt_event_suffix_data_t);
        event_params = pvPortMalloc(total_len);
        if (NULL != event_params) {
            /* Copy the data into event_params from the data of callback. */
            memcpy(event_params, param, param_len);
#ifdef MTK_AWS_MCE_ENABLE
            /* Append the suffix data. */
            bt_event_suffix_data_t *suffix_data = get_bt_event_suffix_data(event_params, param_len);
            suffix_data->aws_role = role;
#endif
        } else {
            APPS_LOG_MSGID_I("malloc fail", 0);
        }
    }
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_BT_SINK, event_id, event_params,
                        param_len, NULL, 0);
}

/**
 * @brief      The implementation of the weak symbol bt_cm_event_callback which declared in bt_connection_manager.h.
 */
void bt_cm_event_callback(bt_cm_event_t event_id, void *params, uint32_t params_len)
{
    void *event_params = NULL;
    APPS_LOG_MSGID_I(LOG_TAG"bt_cm_event_callback() id: %x", 1, event_id);
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
    if (BT_CM_EVENT_REMOTE_INFO_UPDATE == event_id) {
        bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)params;
        if (remote_update) {
            /* Notify config_led_manager aws status change. */
            if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                && BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service) {
                apps_config_led_manager_on_aws_attached(role, true);
            } else if (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service
                       && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                apps_config_led_manager_on_aws_attached(role, false);
            }
        }
    } else if (BT_CM_EVENT_POWER_STATE_UPDATE == event_id) {
        bt_cm_power_state_update_ind_t *power_update = (bt_cm_power_state_update_ind_t *)params;
        if (power_update && BT_AWS_MCE_ROLE_AGENT == role) {
            if (BT_CM_POWER_STATE_OFF == power_update->power_state) {
                /* Notify config_led_manager aws status change. */
                apps_config_led_manager_on_aws_attached(role, false);
            }
        }
    }
#endif

    if (NULL != params && params_len) {
        event_params = pvPortMalloc(params_len);
        if (event_params) {
            memcpy(event_params, params, params_len);
        } else {
            APPS_LOG_MSGID_I("malloc fail", 0);
            return;
        }
    }
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER, event_id, event_params,
                        params_len, NULL, 0);
}

#ifdef MTK_AWS_MCE_ENABLE
/**
 * @brief      The implementation of the weak symbol bt_cm_event_callback which declared in bt_aws_mce_srv.h.
 */
void bt_aws_mce_srv_event_callback(bt_aws_mce_srv_event_t event_id, void *params, uint32_t params_len)
{
    void *event_params = NULL;
    if (NULL != params && params_len) {
        event_params = pvPortMalloc(params_len);
        if (event_params) {
            memcpy(event_params, params, params_len);
            ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_AWS, event_id, event_params,
                                params_len, NULL, 0);
        } else {
            APPS_LOG_MSGID_I("bt_aws_mce_srv_event_callback malloc fail", 0);
        }
    }
}
#endif

/**
 * @brief      The implementation of the bt event callback, refer to bt_callback_manager.h.
 * @param[in]  msg, the message type of the callback event.
 * @param[in]  status, the status, refer to bt_status_t.
 * @param[in]  buffer, the data buffer.
 */
static void registered_bt_event_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    apps_bt_event_data_t *bt_data = NULL;
    if (BT_GAP_LE_ADVERTISING_REPORT_IND == msg || BT_GAP_LE_EXT_ADVERTISING_REPORT_IND == msg) {
        // Because the BT module sends a large amounts of the messages, must ignore it here. If customer want to receive the message, must check it here.
    } else {
        uint32_t buffer_size;
        switch (msg) {
            /* This is an example. If the user wants to use the buffer in app, the user must copy the buffer to a heap memory; */
            case BT_GAP_LE_DISCONNECT_IND:
                buffer_size = sizeof(bt_gap_le_disconnect_ind_t);
                break;
            case BT_GAP_LE_CONNECT_IND:
                buffer_size = sizeof(bt_gap_le_connection_ind_t);
                break;
            case BT_GAP_LE_READ_RSSI_CNF: {
                buffer_size = sizeof(bt_hci_evt_cc_read_rssi_t);
#ifdef MTK_RACE_CMD_ENABLE
                bt_hci_evt_cc_read_rssi_t *read_rssi_result = (bt_hci_evt_cc_read_rssi_t *)buffer;
                if (read_rssi_result) {
                    uint16_t data = BT_APP_COMMON_LINK_QUALITY_BLE_RSSI | ((uint8_t)read_rssi_result->rssi) << 8;
                    APPS_LOG_MSGID_I("BT_GAP_LE_READ_RSSI_CNF rssi=%x, report=%d", 2, read_rssi_result->rssi, data);
                    app_race_send_notify_to_hid(APPS_RACE_CMD_CONFIG_TYPE_LINK_QUALITY, (int8_t *)&data, sizeof(uint16_t));
                }
#endif
                break;
            }
            case BT_GAP_READ_RSSI_CNF: {
                buffer_size = sizeof(bt_gap_read_rssi_cnf_t);
#ifdef MTK_RACE_CMD_ENABLE
                bt_gap_read_rssi_cnf_t *read_rssi_result = (bt_gap_read_rssi_cnf_t *)buffer;
                if (read_rssi_result) {
                    uint16_t data = BT_APP_COMMON_LINK_QUALITY_SPP_RSSI | ((uint8_t)read_rssi_result->rssi) << 8;
                    APPS_LOG_MSGID_I("BT_GAP_READ_RSSI_CNF rssi=%x, report=%d", 2, read_rssi_result->rssi, data);
                    app_race_send_notify_to_hid(APPS_RACE_CMD_CONFIG_TYPE_LINK_QUALITY, (int8_t *)&data, sizeof(uint16_t));
                }
#endif
                break;
            }
            default:
                buffer_size = 0;
                break;
        }
        /* To decrease the times of calling malloc, append buffer after bt_data */
        bt_data = (apps_bt_event_data_t *)pvPortMalloc(sizeof(apps_bt_event_data_t) + buffer_size);
        if (bt_data) {
            bt_data->status = status;
            bt_data->buffer = NULL;
            if (buffer_size) {
                memcpy(((uint8_t *)bt_data) + sizeof(apps_bt_event_data_t), buffer, buffer_size);
                bt_data->buffer = ((uint8_t *)bt_data) + sizeof(apps_bt_event_data_t);
            }
            ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_BT,
                                msg, bt_data, sizeof(apps_bt_event_data_t), NULL, 0);
        }

    }
}

static bt_status_t bt_device_manager_callback(bt_device_manager_power_event_t evt, bt_device_manager_power_status_t status, void *data, uint32_t data_len)
{
    void *extra_data = NULL;

    if (data && data_len > 0) {
        extra_data = pvPortMalloc(data_len);
        if (extra_data) {
            memcpy(extra_data, data, data_len);
        } else {
            APPS_LOG_MSGID_E("bt_device_manager_callback malloc fail", 0);
            return BT_STATUS_FAIL;
        }
    }
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER,
                        (evt & 0xFFFF) | ((status << 16) & 0xFFFF0000), extra_data, data_len, NULL, 0);
    return BT_STATUS_SUCCESS;
}

void bt_event_get_bt_dm_event_and_status(uint32_t event_id, bt_device_manager_power_event_t *p_evt, bt_device_manager_power_status_t *p_status)
{
    if (p_evt) {
        *p_evt = event_id & 0xFFFF;
    }
    if (p_status) {
        *p_status = (event_id >> 16) & 0xFFFF;
    }
}

#ifdef AIR_HID_BT_HOGP_ENABLE
#include "bt_hogp_client.h"
#include "app_bolt_poc_data.h"

static bt_status_t app_dongle_hid_bt_hogp_report_callback(bt_hogp_client_event_t event, bt_hogp_client_para_t *para, bt_status_t status, void *buffer, uint16_t length)
{
    switch (event) {
        case BT_HOGP_CLIENT_EVENT_REPORT_MAP_IND:
#ifdef AIR_BOLT_POC_MULTI_VENDOR_SUPPORT
            APPS_LOG_DUMP_I(" [BOLT_POC] descriptor report:", buffer, length);
            bt_hogp_client_set_rediscovery_flag(para->conn_handle, false);
            app_bolt_poc_ri_parse(para->conn_handle, (uint8_t *)buffer, length);
#endif
            break;
        case BT_HOGP_CLIENT_EVENT_INPUT_REPORT_IND:
            app_bolt_poc_event_input_report_ind(para, (const uint8_t *)buffer, length);
            break;
        case BT_HOGP_CLIENT_EVENT_CONNECT_IND:
            //APPS_LOG_MSGID_I(" [BOLT_POC] app_dongle_hid_bt_hogp_report_callback BT_HOGP_CLIENT_EVENT_CONNECT_IND", 0);
            app_bolt_poc_pop_handle_process(para->conn_handle);
            break;
    }
    return 0;
}

static void app_dongle_hid_bt_hogp_event_callback(bt_msg_type_t event, bt_status_t status, void *buffer)
{
    switch (event) {
        case BT_POWER_ON_CNF:
            APPS_LOG_MSGID_I(" [BOLT_POC] bt_bolt_poc_event_callback BT_POWER_ON_CNF", 0);
            app_bolt_poc_power_on_cnf();
            break;
        case BT_POWER_OFF_CNF:
            APPS_LOG_MSGID_I(" [BOLT_POC] bt_bolt_poc_event_callback BT_POWER_OFF_CNF", 0);
            break;
        case BT_GAP_LE_SET_WHITE_LIST_CNF:
            app_bolt_poc_set_white_list_cnf(status);
            break;
        case BT_GAP_LE_CONNECT_IND: {
                const bt_gap_le_connection_ind_t *conn_ind = (const bt_gap_le_connection_ind_t *)buffer;
                if (conn_ind == NULL) {
                    return;
                }
                app_bolt_poc_connect_ind(status, &conn_ind->peer_addr, conn_ind->connection_handle);
                APPS_LOG_MSGID_I(" [BOLT_POC] bt_bolt_poc_event_callback: connect handle(0x%X), connected dev num(0x%X)", 2, conn_ind->connection_handle, bt_gap_le_srv_get_connected_dev_num());
            }
            break;
        case BT_GAP_LE_DISCONNECT_IND: {
                const bt_hci_evt_disconnect_complete_t *disconn_ind = (const bt_hci_evt_disconnect_complete_t *)buffer;
                if (disconn_ind == NULL) {
                    return;
                }
                app_bolt_poc_disconn_ind(disconn_ind->connection_handle);
                APPS_LOG_MSGID_I(" [BOLT_POC] bt_bolt_poc_event_callback: disconnect handle(0x%X), connected dev num(0x%X)", 2, disconn_ind->connection_handle, bt_gap_le_srv_get_connected_dev_num());

                extern void app_bolt_poc_race_report_dev_list();
                app_bolt_poc_race_report_dev_list();
            }
            break;
        case BT_GAP_LE_CONNECT_CANCEL_CNF: {
                if (status == BT_HCI_STATUS_COMMAND_DISALLOWED) {
                    APPS_LOG_MSGID_I(" [BOLT_POC] bt_bolt_poc_event_callback BT_GAP_LE_CONNECT_CANCEL_CNF cancel", 0);
                    app_bolt_poc_connect_cancel_cnf();
                }
            }
            break;
        case BT_GAP_LE_EXT_ADVERTISING_REPORT_IND: {
                APPS_LOG_MSGID_I(" [BOLT_POC] BT_GAP_LE_EXT_ADVERTISING_REPORT_IND evt!", 0);
                app_bolt_poc_adv_report_ind(buffer);
            }
            break;
        case BT_GAP_LE_BONDING_COMPLETE_IND: {
#ifdef AIR_BOLT_POC_MULTI_VENDOR_SUPPORT
                const bt_gap_le_bonding_complete_ind_t *bond_ind = (const bt_gap_le_bonding_complete_ind_t *)buffer;
                if (bond_ind == NULL) {
                    return;
                }
                app_bolt_poc_bonding_complete_ind(bond_ind->handle);
#endif
            }
            break;
        default:
            break;
    }
}

void app_dongle_hid_bt_hogp_init()
{
    bt_hogp_client_init(app_dongle_hid_bt_hogp_report_callback);
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_SYSTEM),
                                          app_dongle_hid_bt_hogp_event_callback);
}
#endif

void apps_events_bt_event_init(void)
{
    /* Only care a part of events, so the second parameter is not all modules. */
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_SYSTEM),
                                          (void *)registered_bt_event_callback);
    bt_device_manager_register_callback(bt_device_manager_callback);
}

#ifdef AIR_BT_FAST_PAIR_ENABLE
/**
 * @brief      The implementation of the weak symbol of bt_fast_pair_app_event_callback which is declared in bt_fast_pair.h.
 */
bt_fast_pair_status_t bt_fast_pair_app_event_callback(bt_fast_pair_app_event_t evt, bt_bd_addr_t *addr, void *buf, uint32_t buf_len)
{
    app_bt_event_fast_pair_callback_data_t *msg_buf = NULL;
    uint32_t data_len = 0;
    APPS_LOG_MSGID_I("[UI_SHELL], fast_pair_event_callback :%x", 1, buf);

    if (BT_FAST_PAIR_APP_EVENT_NEED_STORE_ACCOUNT_KEY == evt) {
        /* The buffer of this event is a number but on a pointer. */
        ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                            evt, buf, 0, NULL, 0);
        return BT_FAST_PAIR_STATUS_SUCCESS;
    }
    if (buf != NULL || addr != NULL) {
        data_len += sizeof(app_bt_event_fast_pair_callback_data_t);   /* Event addr is NULL, set 0 as address. */
        data_len += (buf != NULL) ? buf_len : 0;
        if (data_len > 0) {
            msg_buf = (app_bt_event_fast_pair_callback_data_t *)pvPortMalloc(data_len);
            if (msg_buf) {
                memset(msg_buf, 0, data_len);
                if (addr) {
                    memcpy(msg_buf->addr, *addr, sizeof(bt_bd_addr_t));
                }
                if (buf && buf_len > 0) {
                    memcpy(msg_buf->data, buf, buf_len);
                }
            } else {
                APPS_LOG_MSGID_E("fast pair send event malloc fail", 0);
                return BT_FAST_PAIR_STATUS_FAIL;
            }
        }
    }
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                        evt,
                        msg_buf,
                        data_len,
                        NULL, 0);
    return BT_FAST_PAIR_STATUS_SUCCESS;
}
#endif

#ifdef MTK_RACE_CMD_ENABLE
/**
 * @brief      Process the fota type race cmd, send event to APPs.
 * @param[in]  event_type, the events type defined in race_event_type_enum.
 * @param[in]  param, parameter of the event.
 * @param[in]  user_data, The same user_data input in race_event_register().
 * @return RACE_ERRCODE_SUCCESS succeed; otherwise, fail.
 */
static RACE_ERRCODE bt_race_fota_app_event_callback(race_event_type_enum event_type, void *param, void *user_data)
{
    APPS_LOG_MSGID_I("[UI_SHELL], FOTA_APP_event_callback :%x", 1, param);

    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_FOTA,
                        event_type,
                        param,
                        0,
                        NULL, 0);
    return RACE_ERRCODE_SUCCESS;
}


#ifdef RACE_FIND_ME_ENABLE
#include "app_fm_activity.h"
/**
 * @brief      Process the find me type race cmd, send event to APPs.
 * @param[in]  event_type, the events type defined in race_event_type_enum.
 * @param[in]  param, parameter of the event.
 * @param[in]  user_data, The same user_data input in race_event_register().
 * @return RACE_ERRCODE_SUCCESS succeed; otherwise, fail.
 */
static RACE_ERRCODE bt_race_findme_app_event_callback(race_event_type_enum event_type, void *param, void *user_data)
{
    APPS_LOG_MSGID_I("[UI_SHELL], FOTA_findme_event_callback :%x", 1, param);
    if (RACE_EVENT_TYPE_FIND_ME == event_type && param) {
        app_find_me_param_struct *find_me_param = (app_find_me_param_struct *)pvPortMalloc(sizeof(app_find_me_param_struct));
        if (find_me_param) {
            find_me_param->blink = ((race_event_find_me_param_struvt *)param)->is_blink;
            find_me_param->tone = ((race_event_find_me_param_struvt *)param)->is_tone;
            find_me_param->duration_seconds = 0;
            ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_FINDME,
                                APP_FIND_ME_EVENT_ID_TRIGGER,
                                find_me_param,
                                sizeof(app_find_me_param_struct),
                                NULL, 0);
        }
    }
    return RACE_ERRCODE_SUCCESS;
}

#endif

/**
 * @brief      Process the reload nvkey type race cmd.
 * @param[in]  param, parameter of the event.
 * @param[in]  user_data, The same user_data input in race_event_register().
 * @return RACE_ERRCODE_SUCCESS succeed; otherwise, fail.
 */
static RACE_ERRCODE bt_race_reload_nvkey_event_callback(void *param, void *user_data)
{
    uint16_t nvkey_id = *(uint16_t *)param;
    APPS_LOG_MSGID_I("[UI_SHELL], bt_race_reload_nvkey_event_callback :%x", 1, nvkey_id);
    /* Need reload NVID_APP_CUSTOMER_KEY_CFG
    , send event to refresh key remapping table. */
    if (NVID_APP_CUSTOMER_KEY_CFG
        == nvkey_id) {
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_RELOAD_KEY_ACTION_FROM_NVKEY, NULL, 0,
                            NULL, 0);
        return RACE_ERRCODE_SUCCESS;
    } else {
        return RACE_ERRCODE_NOT_SUPPORT;
    }
}

/**
 * @brief      Process the simulate key event race cmd.
 * @param[in]  param, parameter of the event, means the simulated key action.
 * @param[in]  user_data, The same user_data input in race_event_register().
 * @return RACE_ERRCODE_SUCCESS succeed; otherwise, fail.
 */
static RACE_ERRCODE bt_race_key_app_event_callback(void *param, void *user_data)
{
    uint16_t *msg_data = (uint16_t *)param;
    APPS_LOG_MSGID_I("[UI_SHELL], race_key_event_callback :%x", 1, *msg_data);
    uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
    if (p_key_action) {
        *p_key_action = *msg_data;
        /* The extra_data of the event is key action. */
        ui_shell_send_event(false,
                            EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY,
                            INVALID_KEY_EVENT_ID,
                            p_key_action, sizeof(uint16_t), NULL, 0);
    }
    return RACE_ERRCODE_SUCCESS;
}

/**
 * @brief      Process the power testing request race cmd.
 * @param[in]  param, parameter of the event.
 * @param[in]  user_data, The same user_data input in race_event_register().
 * @return RACE_ERRCODE_SUCCESS succeed; otherwise, fail.
 */
static RACE_ERRCODE bt_race_system_power_event_callback(void *param, void *user_data)
{
    uint8_t *msg_data = (uint8_t *)param;
    APPS_LOG_MSGID_I("[UI_SHELL], race_system_power_event_callback :%x", 1, *msg_data);

    switch (*msg_data) {
        case RACE_SYSTEM_POWER_PMUOFF:
            APPS_LOG_MSGID_I("[UI_SHELL], race_system_power_event_callback: send event OFF mode", 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_SYSTEM_POWER,
                                APPS_EVENTS_SYSTEM_POWER_PMUOFF, NULL, 0,
                                NULL, 0);
            break;
        case RACE_SYSTEM_POWER_RTC:
            APPS_LOG_MSGID_I("[UI_SHELL], race_system_power_event_callback: send event RTC mode", 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_SYSTEM_POWER,
                                APPS_EVENTS_SYSTEM_POWER_RTC, NULL, 0,
                                NULL, 0);
            break;
        case RACE_SYSTEM_POWER_SLEEP:
            APPS_LOG_MSGID_I("[UI_SHELL], race_system_power_event_callback: send event SLEEP mode", 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_SYSTEM_POWER,
                                APPS_EVENTS_SYSTEM_POWER_SLEEP, NULL, 0,
                                NULL, 0);
            break;
        default:
            break;
    }

    return RACE_ERRCODE_SUCCESS;
}


#ifdef MTK_LEAKAGE_DETECTION_ENABLE
/**
 * @brief      Process the leakage detection voice prompt request race cmd.
 * @param[in]  param, parameter of the event.
 * @param[in]  user_data, The same user_data input in race_event_register().
 * @return RACE_ERRCODE_SUCCESS succeed; otherwise, fail.
 */
static RACE_ERRCODE bt_race_cmd_vp_callback(void *param, void *user_data)
{
    race_cmd_vp_struct *vp_param = (race_cmd_vp_struct *)param;
    bool *p_play_flag = NULL;

    switch (vp_param->vp_type) {
        case RACE_CMD_VP_LEAKAGE_DETECTION:
            p_play_flag = (bool *)pvPortMalloc(sizeof(bool));
            *p_play_flag = vp_param->play_flag;
            ui_shell_status_t status = ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                           APPS_EVENTS_INTERACTION_LEAKAGE_DETECTION_VP_TRIGGER,
                                                           p_play_flag, sizeof(bool), NULL, 0);
            if (UI_SHELL_STATUS_OK != status) {
                vPortFree(p_play_flag);
                APPS_LOG_MSGID_I("UI_SHELL send race cmd vp trigger event failed.", 0);
            }
            break;
        default:
            break;
    }

    return RACE_ERRCODE_SUCCESS;
}
#endif

/**
 * @brief      Process the app race events.
 * @param[in]  id, The same register_id obtained when calling race_event_register().
 * @param[in]  event_type, The type of the event occurs
 * @param[in]  param, The parameter of the event
 * @param[in]  user_data, The same user_data input in race_event_register().
 * @return RACE_ERRCODE_SUCCESS succeed; otherwise, fail.
 */
static RACE_ERRCODE bt_race_app_event_callback(int32_t id, race_event_type_enum event_type, void *param, void *user_data)
{
    switch (event_type) {
        case RACE_EVENT_TYPE_CONN_SPP_CONNECT:
#if defined AIR_BT_ULTRA_LOW_LATENCY_ENABLE
            apps_race_cmd_event_on_race_spp_connected();
#endif
            break;
        case RACE_EVENT_SYSTEM_POWER:
            return bt_race_system_power_event_callback(param, user_data);
        case RACE_EVENT_RELOAD_NVKEY_TO_RAM:
            return bt_race_reload_nvkey_event_callback(param, user_data);
        case RACE_EVENT_TYPE_KEY:
            return bt_race_key_app_event_callback(param, user_data);
#ifdef RACE_FIND_ME_ENABLE
        case RACE_EVENT_TYPE_FIND_ME:
            return bt_race_findme_app_event_callback(event_type, param, user_data);
#endif
        case RACE_EVENT_TYPE_FOTA_START:
        case RACE_EVENT_TYPE_FOTA_CANCELLING:
        case RACE_EVENT_TYPE_FOTA_CANCEL:
        case RACE_EVENT_TYPE_FOTA_NEED_REBOOT:
            return bt_race_fota_app_event_callback(event_type, param, user_data);
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
        case RACE_EVENT_TYPE_CMD_VP:
            return bt_race_cmd_vp_callback(param, user_data);
#endif
        default:
            break;
    }
    return RACE_ERRCODE_NOT_SUPPORT;
}

/**
 * @brief      Initialize race app callback.
 */
void bt_race_app_event_init()
{
    int32_t register_id = 0;

    APPS_LOG_MSGID_I("[UI_SHELL], bt_race_app_event_init", 0);

    race_event_register(&register_id, bt_race_app_event_callback, NULL);


}

#ifdef MTK_AMA_ENABLE

uint32_t bt_ama_app_event_callback(Handler handler, uint16_t event_type, void *param, uint32_t len);

static AMA_HandlerData g_app_ama_handler = { bt_ama_app_event_callback };

/**
 * @brief      callback of AMA middleware.
 * @param[in]  handler, The same handler obtained when calling AMA_Target_StartService().
 * @param[in]  event_type, the event type, which is defined in AMA_MSG_E.
 * @param[in]  param, the pointer to the parameter.
 * @param[in]  len, the length of the parameter.
 */
uint32_t bt_ama_app_event_callback(Handler handler, uint16_t event_type, void *param, uint32_t len)
{
    void *event_param = NULL;
    APPS_LOG_MSGID_I("[AMA], bt_ama_app_event_callback :%x, len = %x, event_type : %x", 3, param, len, event_type);
    if (event_type <= AMA_MSG_MAX && len) {
        event_param = pvPortMalloc(len);
        if (event_param) {
            memcpy(event_param, param, len);
            ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_BT_AMA,
                                event_type,
                                event_param,
                                len,
                                NULL, 0);
        }
    }
    return RACE_ERRCODE_SUCCESS;
}

/**
 * @brief      initialize AMA.
 */
void app_ama_init(void)
{
    AMAStatus status = AMA_Target_StartService(&g_app_ama_handler, 1);//one link
    APPS_LOG_MSGID_I("[AMA], app_ama_init :status: %x", 1, status);
    //ama_audio_init();/*only init once*/
}
#endif
#endif

void apps_bt_events_le_service_callback(bt_gap_le_srv_event_t event, void *data)
{
    uint32_t data_len = 0;
    void *extra_data = NULL;
    APPS_LOG_MSGID_I(LOG_TAG"apps_bt_events_le_service_callback() event: %x", 1, event);
    switch (event) {
        case BT_GAP_LE_SRV_EVENT_ADV_COMPLETE:
            data_len = sizeof(bt_gap_le_srv_adv_complete_t);
            break;
        case BT_GAP_LE_SRV_EVENT_ADV_CLEARED:
        case BT_GAP_LE_SRV_EVENT_CONN_CLEARED:
        case BT_GAP_LE_SRV_EVENT_BLE_DISABLED:
            data_len = sizeof(bt_gap_le_srv_common_result_t);
            break;
        case BT_GAP_LE_SRV_EVENT_CONN_UPDATED:
            data_len = sizeof(bt_gap_le_srv_conn_update_t);
            break;
        default:
            return;
    }
    extra_data = pvPortMalloc(data_len);
    if (extra_data) {
        memcpy(extra_data, data, data_len);
    } else {
        return;
    }
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_LE_SERVICE,
                        event,
                        extra_data,
                        data_len,
                        NULL, 0);
}

#if defined(MTK_AWS_MCE_ENABLE)
/**
 * @brief      callback of received aws data.
 * @param[in]  para, The parameter of the event.
 */
void app_aws_report_event_callback(bt_aws_mce_report_info_t *para)
{
    APPS_LOG_MSGID_I("[UI_SHELL], app_aws_report_event_callback: send aws report to app: module 0x%0x", 1, para->module_id);
    /* Malloc the bt_aws_mce_report_info_t and the param of bt_aws_mce_report_info_t in a whole memory. */
    bt_aws_mce_report_info_t *extra_data = pvPortMalloc(sizeof(bt_aws_mce_report_info_t) + para->param_len);
    if (extra_data == NULL) {
        APPS_LOG_MSGID_I("[UI_SHELL], app_aws_report_event_callback: malloc fail", 0);
        return;
    }

    memcpy(extra_data, para, sizeof(bt_aws_mce_report_info_t));
    extra_data->param = (uint8_t *)extra_data + sizeof(bt_aws_mce_report_info_t);
    /* Append the content of the para->param to extra_data->param. */
    memcpy(extra_data->param, para->param, para->param_len);

    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_AWS_DATA,
                        0, extra_data, sizeof(bt_aws_mce_report_info_t) + para->param_len,
                        NULL, 0);
}

/**
 * @brief      initialize the aws report.
 * @param[in]  para, The parameter of the event.
 */
void app_aws_report_event_init()
{
    APPS_LOG_MSGID_I("[UI_SHELL], app_aws_report_event_init", 0);

    /* Only care a part of events, so only register a part of BT_AWS_MCE_REPORT_MODULE */
    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_BATTERY, app_aws_report_event_callback);
    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_APP_ACTION, app_aws_report_event_callback);
#ifdef MTK_SMART_CHARGER_ENABLE
    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_SMCHARGER, app_aws_report_event_callback);
#endif

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_IN_EAR, app_aws_report_event_callback);
#endif
}


#endif

#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
void bt_ulla_callback(bt_ull_event_t event, void *param, uint32_t param_len)
{
    void *extra_data = NULL;
    uint32_t malloc_size = param_len;
    APPS_LOG_MSGID_I(LOG_TAG" set app_le_ull_callback event=%d.", 1, event);
    if (param) {
        if (BT_ULL_EVENT_USER_DATA_IND == event
        #ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
            || BT_ULL_EVENT_LE_HID_INPUT_REPORT_IND == event
            || BT_ULL_EVENT_LE_HID_RACE_DATA_IND == event
        #endif
        ) {
            malloc_size += ((bt_ull_user_data_t *)param)->user_data_length;
        }
        extra_data = pvPortMalloc(malloc_size);
        if (extra_data) {
            memcpy(extra_data, param, param_len);
            if (BT_ULL_EVENT_USER_DATA_IND == event
            #ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
                || BT_ULL_EVENT_LE_HID_INPUT_REPORT_IND == event
                || BT_ULL_EVENT_LE_HID_RACE_DATA_IND == event
            #endif
            ) {
                ((bt_ull_user_data_t *)extra_data)->user_data = ((uint8_t *)extra_data) + sizeof(bt_ull_user_data_t);
                memcpy(((bt_ull_user_data_t *)extra_data)->user_data, ((bt_ull_user_data_t *)param)->user_data, ((bt_ull_user_data_t *)param)->user_data_length);
            }
        } else {
            APPS_LOG_MSGID_E(LOG_TAG"bt_ulla_callback: malloc fail", 0);
            return;
        }
    }
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_BT_ULTRA_LOW_LATENCY,
                        event, extra_data, param_len, NULL, 0);

}
#endif
