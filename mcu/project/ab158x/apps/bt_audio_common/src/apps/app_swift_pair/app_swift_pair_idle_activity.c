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

#ifdef AIR_SWIFT_PAIR_ENABLE

#include "app_swift_pair_idle_activity.h"

#include "app_swift_cust_pair.h"

#include "app_bt_state_service.h"
#include "apps_debug.h"
#include "apps_events_bt_event.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"

#include "bt_app_common.h"
#include "bt_callback_manager.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_customer_config.h"
#include "bt_device_manager.h"
#include "bt_device_manager_le.h"
#include "bt_hci.h"
#include "bt_gap.h"
#include "bt_gap_le.h"
#include "bt_gatt.h"
#include "bt_gatts.h"
#include "bt_system.h"
#include "multi_ble_adv_manager.h"
#include "ui_shell_manager.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#endif
#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
#include "app_le_audio.h"
#endif



/**================================================================================*/
/**                              Definition & Structure                            */
/**================================================================================*/
#define LOG_TAG                             "[SWIFT_PAIR]"

#define APP_SWIFT_PAIR_QUICK_MIN_ADV_INTERVAL         (0x0030)              // 30ms
#define APP_SWIFT_PAIR_QUICK_MAX_ADV_INTERVAL         (0x00A0)              // 100ms
#define APP_SWIFT_PAIR_NORMAL_MIN_ADV_INTERVAL        (0x00A0)              // 100ms
#define APP_SWIFT_PAIR_NORMAL_MAX_ADV_INTERVAL        (0x01E0)              // 300ms

#define APP_SWIFT_PAIR_QUICK_TIMEOUT                  (30 * 1000)
#define APP_SWIFT_PAIR_TIMEOUT                        (2 * 60 * 1000)       // 2min

#define BLE_AD_TYPE_COMPLETE_LOCAL_NAME               0x09

typedef struct {
    bool            is_quick_adv;
} app_swift_pair_context_t;

static app_swift_pair_context_t             app_swift_pair_ctx = {0};



/**================================================================================*/
/**                                Internal Function                               */
/**================================================================================*/
static void app_swift_pair_get_adv_data(bt_gap_le_set_ext_advertising_data_t *data)
{
    if (data == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" get_adv_data, NULL data", 0);
        return;
    }

    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
    uint8_t *addr = (uint8_t *)bt_device_manager_aws_local_info_get_fixed_address();
#else
    uint8_t *addr = (uint8_t *)bt_device_manager_get_local_address();
#endif
    if (addr == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" get_adv_data, NULL local addr", 0);
        return;
    }

#ifdef APP_SWIFT_PAIR_LE_EDR_SECURE_MODE
#define APP_SWIFT_PAIR_MAX_PARAM_LEN        10
#if defined(AIR_TWS_ENABLE) && defined(APP_SWIFT_PAIR_SHOW_TWS_ICON)
#define APP_SWIFT_PAIR_MAX_NAME_LEN         (BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM - APP_SWIFT_PAIR_MAX_PARAM_LEN - 4)
#else
#define APP_SWIFT_PAIR_MAX_NAME_LEN         (BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM - APP_SWIFT_PAIR_MAX_PARAM_LEN)
#endif

    uint8_t data_context[BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM] = {17,                   // Len
                                                                       0xFF, 0x06, 0x00,     // Company <0x0006> Micorsoft
                                                                       0x03, 0x02,           // Type: Swift Pair Beacon
                                                                       0x80,                 // RSSI
#ifdef AIR_SPEAKER_ENABLE
                                                                       0x14, 0x04, 0x20,     // Cod[3]
#else
                                                                       0x18, 0x04, 0x20,     // Cod[3] http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
#endif
                                                                       'A',                  // Display Name
                                                                      };

    const bt_gap_config_t *cust_config = bt_customer_config_get_gap_config();
    if (cust_config != NULL) {
        uint8_t name_length = strlen(cust_config->device_name);
        if (name_length > APP_SWIFT_PAIR_MAX_NAME_LEN) {
            //APPS_LOG_MSGID_E(LOG_TAG" get_adv_data, name length %d > %d", 1, name_length, APP_SWIFT_PAIR_MAX_NAME_LEN);
            name_length = APP_SWIFT_PAIR_MAX_NAME_LEN;
        }
        memcpy(&data_context[APP_SWIFT_PAIR_MAX_PARAM_LEN], cust_config->device_name, name_length);
        data_context[0] = APP_SWIFT_PAIR_MAX_PARAM_LEN - 1 + name_length;
    }

#if defined(AIR_TWS_ENABLE) && defined(APP_SWIFT_PAIR_SHOW_TWS_ICON)
    // <Appearance>
    uint8_t index = data_context[0] + 1;
    data_context[index++] = 3;
    data_context[index++] = BT_GAP_LE_AD_TYPE_APPEARANCE;
    uint16_t appearance = 0x0941;
    memcpy(&data_context[index], &appearance, 2);

    data->data_length = data_context[0] + 1 + 4;
#else
    data->data_length = data_context[0] + 1;
#endif
    data->fragment_preference = 0x00;

    memcpy(data->data, data_context, data->data_length);

#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
    APPS_LOG_MSGID_I(LOG_TAG" get_adv_data [LEA/EDR], [%02X] fixed_addr=%08X%04X len=%d",
                     4, role, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), data->data_length);
#else
    APPS_LOG_MSGID_I(LOG_TAG" get_adv_data [LEA/EDR], [%02X] addr=%08X%04X len=%d",
                     4, role, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), data->data_length);
#endif

#else
#define APP_SWIFT_PAIR_MAX_PARAM_LEN        16
#if defined(AIR_TWS_ENABLE) && defined(APP_SWIFT_PAIR_SHOW_TWS_ICON)
#define APP_SWIFT_PAIR_MAX_NAME_LEN         (BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM - APP_SWIFT_PAIR_MAX_PARAM_LEN - 4)
#else
#define APP_SWIFT_PAIR_MAX_NAME_LEN         (BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM - APP_SWIFT_PAIR_MAX_PARAM_LEN)
#endif

    const bt_gap_config_t *cust_config = bt_customer_config_get_gap_config();
    uint8_t data_context[31] = { 23,                                    // Len
                                 0xFF, 0x06, 0x00,                      // Company <0x0006> Micorsoft
                                 0x03, 0x01,                            // Type: Swift Pair Beacon
                                 0x80,                                  // RSSI
                                 0x33, 0x33, 0x33, 0xFF, 0xFF, 0xFF,    // BD Addr
#ifdef AIR_SPEAKER_ENABLE
                                 0x14, 0x04, 0x20,     // Cod[3]
#else
                                 0x18, 0x04, 0x20,     // Cod[3] http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
#endif
                                 'A', 'I', 'R', 'O', 'H', 'A', 'B', 'T',// Display Name
                                 0
                               };
    // BT Addr, little endian.
    memcpy(&data_context[7], addr, BT_BD_ADDR_LEN);

    if (cust_config != NULL) {
        uint8_t name_length = strlen(cust_config->device_name);
        if (name_length > APP_SWIFT_PAIR_MAX_NAME_LEN) {
            //APPS_LOG_MSGID_E(LOG_TAG" get_adv_data, name length %d > %d", 1, name_length, APP_SWIFT_PAIR_MAX_NAME_LEN);
            name_length = APP_SWIFT_PAIR_MAX_NAME_LEN;
        }
        memcpy(&data_context[APP_SWIFT_PAIR_MAX_PARAM_LEN], cust_config->device_name, name_length);
        data_context[0] = APP_SWIFT_PAIR_MAX_PARAM_LEN - 1 + name_length;
    }

#if defined(AIR_TWS_ENABLE) && defined(APP_SWIFT_PAIR_SHOW_TWS_ICON)
    // <Appearance>
    uint8_t index = data_context[0] + 1;
    data_context[index++] = 3;
    data_context[index++] = BT_GAP_LE_AD_TYPE_APPEARANCE;
    uint16_t appearance = 0x0941;
    memcpy(&data_context[index], &appearance, 2);

    data->data_length = data_context[0] + 1 + 4;
#else
    data->data_length = data_context[0] + 1;
#endif
    data->fragment_preference = 0x00;

    memcpy(data->data, data_context, data->data_length);

    APPS_LOG_MSGID_I(LOG_TAG" get_adv_data [EDR only], [%02X] addr=%08X%04X len=%d",
                     4, role, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), data->data_length);
#endif
}

static void app_swift_pair_get_adv_param(bt_hci_le_set_ext_advertising_parameters_t *adv_param)
{
    if (adv_param == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" get_adv_param, NULL adv_param", 0);
        return;
    }

    bt_app_common_generate_default_adv_data(adv_param, NULL, NULL, NULL, 0);

    APPS_LOG_MSGID_I(LOG_TAG" get_adv_param, is_quick_adv=%d", 1, app_swift_pair_ctx.is_quick_adv);
    if (app_swift_pair_ctx.is_quick_adv) {
        adv_param->primary_advertising_interval_min = APP_SWIFT_PAIR_QUICK_MIN_ADV_INTERVAL;
        adv_param->primary_advertising_interval_max = APP_SWIFT_PAIR_QUICK_MAX_ADV_INTERVAL;
    } else {
        adv_param->primary_advertising_interval_min = APP_SWIFT_PAIR_NORMAL_MIN_ADV_INTERVAL;
        adv_param->primary_advertising_interval_max = APP_SWIFT_PAIR_NORMAL_MAX_ADV_INTERVAL;
    }

    // Customer configure: need to tune Tx Power of Swift pairing ADV
    adv_param->advertising_tx_power = 0;

#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE)
#ifdef AIR_TWS_ENABLE
    adv_param->own_address_type = BT_ADDR_LE_PUBLIC;
#else
    adv_param->own_address_type = BT_ADDR_PUBLIC;
#endif
    memset(&(adv_param->peer_address), 0, sizeof(bt_addr_t));
#else
    adv_param->own_address_type = BT_ADDR_RANDOM;
    memset(&(adv_param->peer_address), 0, sizeof(bt_addr_t));
#endif
}

static void app_swift_pair_get_scan_rsp(bt_gap_le_set_ext_scan_response_data_t *scan_rsp)
{
    if (scan_rsp == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" get_scan_rsp, NULL scan_rsp", 0);
        return;
    }

    const bt_gap_config_t *cust_config = bt_customer_config_get_gap_config();
    if (cust_config == NULL || cust_config->device_name == NULL || strlen(cust_config->device_name) == 0) {
        //APPS_LOG_MSGID_E(LOG_TAG" get_scan_rsp, NULL cust_config or name", 0);
        return;
    }

    uint8_t rsp[BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM] = {0, BLE_AD_TYPE_COMPLETE_LOCAL_NAME, 0};

    const char *name = cust_config->device_name;
    uint8_t max_name_len = strlen(cust_config->device_name);
    max_name_len = (max_name_len >= (BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM - 2) ? (BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM - 2) : max_name_len);
    memcpy(rsp + 2, name, max_name_len);
    rsp[0] = max_name_len + 1;

    scan_rsp->data_length = max_name_len + 2;
    scan_rsp->fragment_preference = 0x00;

    memcpy(scan_rsp->data, rsp, scan_rsp->data_length);
}

static uint32_t app_swift_pair_adv_info(multi_ble_adv_info_t *adv_info)
{
    if (adv_info == NULL) {
        return 0;
    }

    if (adv_info->adv_param != NULL) {
        app_swift_pair_get_adv_param(adv_info->adv_param);
    }
    if (adv_info->adv_data != NULL) {
        app_swift_pair_get_adv_data(adv_info->adv_data);
    }
    if (adv_info->scan_rsp != NULL) {
        app_swift_pair_get_scan_rsp(adv_info->scan_rsp);
        // Swift pairing cannot support scan response
        adv_info->scan_rsp->data_length = 0;
    }
    return 0;
}

#if 0
static uint32_t app_swift_pair_adv_teams_info(multi_ble_adv_info_t *adv_info)
{
    if (adv_info == NULL) {
        return 0;
    }

    if (adv_info->adv_param != NULL) {
        app_swift_pair_get_adv_param(adv_info->adv_param);
    }
    if (adv_info->adv_data != NULL) {
        uint8_t len = 0;
        /* adv_data AD_TYPE_FLAG */
        adv_info->adv_data->data[0] = 2;
        adv_info->adv_data->data[1] = BT_GAP_LE_AD_TYPE_FLAG;
        adv_info->adv_data->data[2] = 0x1A;
        len = 3;
        adv_info->adv_data->data[len] = 0x3;
        adv_info->adv_data->data[len + 1] = 0x2;
        adv_info->adv_data->data[len + 2] = 0x8;
        adv_info->adv_data->data[len + 3] = 0xFE;
        len += 4;

        adv_info->adv_data->data[len] = 0x7;
        adv_info->adv_data->data[len + 1] = 0xFF;
        adv_info->adv_data->data[len + 2] = 0x06;
        adv_info->adv_data->data[len + 3] = 0x00;
#if 1
        adv_info->adv_data->data[len + 4] = 0x04;
        adv_info->adv_data->data[len + 5] = 0x01;
        adv_info->adv_data->data[len + 6] = 0x52;
        adv_info->adv_data->data[len + 7] = app_swift_pair_ctx.is_quick_adv ? 0x02 : 0x00;
        //adv_info->adv_data->data[len+7] = 0x02;
#else
        adv_info->adv_data->data[len + 4] = 0x00;
        adv_info->adv_data->data[len + 5] = 0x00;
        adv_info->adv_data->data[len + 6] = 0x00;
        adv_info->adv_data->data[len + 7] = 0x00;
#endif
        len += 8;

        adv_info->adv_data->data_length = len;
    }
    if (adv_info->scan_rsp != NULL) {
        app_swift_pair_get_scan_rsp(adv_info->scan_rsp);
        adv_info->scan_rsp->data_length = 0;
    }
    return 0;
}

static bool app_swift_pair_update_teams_adv()
{
    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_swift_pair_adv_teams_info);
    multi_ble_adv_manager_add_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_swift_pair_adv_teams_info, 1);
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_SWIFT_PAIR);
    return TRUE;
}
#endif

static bool app_swift_pair_update_adv(void)
{
#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
    bool visible = app_bt_service_is_visible();
    bt_aws_mce_srv_link_type_t aws_link = bt_aws_mce_srv_get_link_type();
    bool is_primary = app_le_audio_is_primary_earbud();
    if (!visible || !is_primary || aws_link == BT_AWS_MCE_SRV_LINK_NONE) {
        APPS_LOG_MSGID_E(LOG_TAG" update_adv, fail", 0);
        return FALSE;
    }
#endif

    APPS_LOG_MSGID_I(LOG_TAG" update_adv, is_quick_adv=%d", 1, app_swift_pair_ctx.is_quick_adv);

    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_swift_pair_adv_info);
    multi_ble_adv_manager_add_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_swift_pair_adv_info, 1);
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_SWIFT_PAIR);

    return TRUE;
}

static bool app_swift_pair_start_adv(void)
{
    app_swift_pair_ctx.is_quick_adv = TRUE;
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_SWIFT_QUICK_TIMEOUT);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                        APP_SWIFT_PAIR_EVENT_SWIFT_QUICK_TIMEOUT, NULL, 0, NULL, APP_SWIFT_PAIR_QUICK_TIMEOUT);

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_SWIFT_ADV_TIMEOUT);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                        APP_SWIFT_PAIR_EVENT_SWIFT_ADV_TIMEOUT, NULL, 0, NULL, APP_SWIFT_PAIR_TIMEOUT);

    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_swift_pair_adv_info);
#if 0
    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_swift_pair_adv_teams_info);
#endif
    multi_ble_adv_manager_add_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_swift_pair_adv_info, 1);
#if 0
    multi_ble_adv_manager_add_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_swift_pair_adv_teams_info, 1);
#endif
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_SWIFT_PAIR);

    APPS_LOG_MSGID_W(LOG_TAG" start_adv", 0);
    return TRUE;
}

static bool app_swift_pair_stop_adv(void)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_SWIFT_QUICK_TIMEOUT);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_SWIFT_ADV_TIMEOUT);

    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_SWIFT_PAIR, app_swift_pair_adv_info);
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_SWIFT_PAIR);

    app_swift_pair_ctx.is_quick_adv = FALSE;

    APPS_LOG_MSGID_W(LOG_TAG" stop_adv", 0);
    return TRUE;
}

static bool app_swift_pair_proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            app_swift_pair_ctx.is_quick_adv = FALSE;
            break;
        }
        default:
            break;
    }
    return TRUE;
}

#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
static bool app_swift_pair_proc_interaction_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    if (event_id == APPS_EVENTS_INTERACTION_BT_VISIBLE_NOTIFY) {
        bool visible = (bool)extra_data;
        bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
        bt_aws_mce_srv_link_type_t aws_link = bt_aws_mce_srv_get_link_type();
        bool is_primary = app_le_audio_is_primary_earbud();
        APPS_LOG_MSGID_I(LOG_TAG" BT_VISIBLE_NOTIFY event, [%02X] visible=%d aws_link=%d is_primary=%d",
                         4, role, visible, aws_link, is_primary);

        if (visible && aws_link != BT_AWS_MCE_SRV_LINK_NONE && is_primary) {
            app_swift_pair_start_adv();
        } else if (!visible) {
            app_swift_pair_stop_adv();
        }
    }
    return FALSE;
}
#endif

static bool app_swift_pair_proc_swift_pair_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case APP_SWIFT_PAIR_EVENT_RESTART_ADV: {
            //APPS_LOG_MSGID_I(LOG_TAG" swift_pair event, restart_adv", 0);
            app_swift_pair_update_adv();
            break;
        }

        case APP_SWIFT_PAIR_EVENT_SWIFT_ADV_TIMEOUT: {
            //APPS_LOG_MSGID_I(LOG_TAG" swift_pair event, adv timeout", 0);
            //app_swift_pair_stop_adv();
            break;
        }

        case APP_SWIFT_PAIR_EVENT_SWIFT_QUICK_TIMEOUT: {
            //APPS_LOG_MSGID_I(LOG_TAG" swift_pair event, quick_adv timeout", 0);
            app_swift_pair_ctx.is_quick_adv = FALSE;
            app_swift_pair_update_adv();
            break;
        }

        default:
            break;
    }
    return TRUE;
}

static bool app_swift_pair_proc_bt_cm_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
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
#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
            bool visible = app_bt_service_is_visible();
            bool is_primary = app_le_audio_is_primary_earbud();
            bool aws_conntected = (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                                   && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service));
            bool aws_disconntected = ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                                      && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service));
#endif

            if (BT_AWS_MCE_ROLE_AGENT == role || BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_FOLLOWER_1 == role) {
                if (remote_update->pre_acl_state != BT_CM_ACL_LINK_ENCRYPTED
                    && remote_update->acl_state == BT_CM_ACL_LINK_ENCRYPTED
                    && phone_related) {
#if !defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) || !defined(AIR_TWS_ENABLE)
                    app_swift_pair_stop_adv();
#endif
#if 0
                    app_swift_pair_update_teams_adv();
#endif
                }
            }

#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
            if (aws_conntected || aws_disconntected) {
                APPS_LOG_MSGID_I(LOG_TAG" BT_CM AWS event, [%02X] aws_conn=%d visible=%d is_primary=%d",
                                 4, role, aws_conntected, visible, is_primary);
            }
            if (aws_conntected && is_primary && visible) {
                app_swift_pair_update_adv();
            } else if (aws_disconntected) {
                app_swift_pair_stop_adv();
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
                app_swift_pair_start_adv();
            } else {
                app_swift_pair_stop_adv();
            }
            break;
        }
#endif
        default:
            break;
    }
    return FALSE;
}



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
void app_swift_pair_restart_adv(void)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_SWIFT_ADV_TIMEOUT);

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_SWIFT_PAIR, APP_SWIFT_PAIR_EVENT_RESTART_ADV);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_SWIFT_PAIR,
                        APP_SWIFT_PAIR_EVENT_RESTART_ADV, NULL, 0, NULL, 0);
    APPS_LOG_MSGID_I(LOG_TAG" restart_adv", 0);
}

bool app_swift_pair_idle_activity_proc(struct _ui_shell_activity *self,
                                       uint32_t event_group,
                                       uint32_t event_id,
                                       void *extra_data,
                                       size_t data_len)
{
    bool ret = FALSE;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            ret = app_swift_pair_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            ret = app_swift_pair_proc_interaction_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_SWIFT_PAIR: {
            ret = app_swift_pair_proc_swift_pair_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            ret = app_swift_pair_proc_bt_cm_group(self, event_id, extra_data, data_len);
            break;
        }
        default:
            break;
    }
    return ret;
}

#endif
