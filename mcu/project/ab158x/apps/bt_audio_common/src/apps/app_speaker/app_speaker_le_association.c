/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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


#include "app_speaker_le_association.h"

#include "bt_aws_mce_le_association.h"

#include "bt_app_common.h"
#include "bt_aws_mce_srv.h"
#include "bt_callback_manager.h"
#include "bt_connection_manager.h"
#include "bt_device_manager_internal.h"
#include "bt_gattc_discovery.h"
#include "bt_gap_le.h"
#include "bt_timer_external.h"
#include "bt_app_common.h"

#include "apps_aws_sync_event.h"
#include "apps_debug.h"
#include "apps_events_bt_event.h"
#include "apps_events_event_group.h"

#include "ui_shell_manager.h"
#include "multi_ble_adv_manager.h"

#ifdef AIR_LE_AUDIO_ENABLE
#include "ble_csis.h"
#include "app_lea_service.h"
#include "bt_sink_srv_le_cap_stream.h"

extern bt_status_t ble_csis_get_sirk(bt_key_t *sirk);
extern void ble_csis_write_nvkey_sirk(bt_key_t *sirk);
#endif



/**================================================================================*/
/**                                Define & Structure                              */
/**================================================================================*/
extern bt_status_t bt_gap_le_srv_set_extended_scan(bt_gap_le_srv_set_extended_scan_parameters_t *param,
                                                   bt_gap_le_srv_set_extended_scan_enable_t *enable,
                                                   void *callback);
#define LOG_TAG                 "[LE_ASS]"

#define APP_SPEAKER_LE_ASS_TIMER            (30 * 1000)

/* Customer configure option: set manufacturer name and version, <manufacturer>(string, max 10 bytes) + <version>(1 byte). */
#define APP_SPEAKER_LE_ASS_MANUFACTURER     ("AIR")
#define APP_SPEAKER_LE_ASS_VERSION          (1)

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

typedef struct {
    bt_addr_t                   agent_le_addr;
    bt_bd_addr_t                peer_agent_edr_addr;
    bt_handle_t                 handle;
    bt_aws_mce_role_t           target_role;
    bt_aws_mce_srv_mode_t       target_mode;
    bool                        association_done;
} PACKED bt_app_association_context;

static bt_key_t app_speaker_le_ass_aws_key = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                              0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00
                                             };

bt_aws_mce_le_association_custom_data_t app_le_ass_custom_data = {0};

static bt_app_association_context app_le_ass_ctx      = {0};

static bool app_le_ass_ungroup_time_start             = FALSE;
static bool app_le_ass_wait_start_adv                 = FALSE;
static bool app_le_ass_wait_start_scan                = FALSE;
static bool app_le_ass_scan_enabled                   = FALSE;



/**================================================================================*/
/**                                    GATT Define                                 */
/**================================================================================*/
static const ble_uuid_t uuid_aws_le_association_service = {
    .type = BLE_UUID_TYPE_128BIT,
    .uuid.uuid = ASSOCIATION_SERVICE_UUID,
};

static const ble_uuid_t uuid_aws_le_association_agent_addr = {
    .type = BLE_UUID_TYPE_128BIT,
    .uuid.uuid = ASSOCIATION_AGENT_ADDR_CHAR_UUID,
};

static const ble_uuid_t uuid_aws_le_association_key = {
    .type = BLE_UUID_TYPE_128BIT,
    .uuid.uuid = ASSOCIATION_KEY_CHAR_UUID,
};

static const ble_uuid_t uuid_aws_le_association_client_addr = {
    .type = BLE_UUID_TYPE_128BIT,
    .uuid.uuid = ASSOCIATION_CLIENT_ADDR_CHAR_UUID,
};

static const ble_uuid_t uuid_aws_le_association_audio_lat = {
    .type = BLE_UUID_TYPE_128BIT,
    .uuid.uuid = ASSOCIATION_AUDIO_LAT_CHAR_UUID,
};

static const ble_uuid_t uuid_aws_le_association_voice_lat = {
    .type = BLE_UUID_TYPE_128BIT,
    .uuid.uuid = ASSOCIATION_VOICE_LAT_CHAR_UUID,
};

static const ble_uuid_t uuid_aws_le_association_number = {
    .type = BLE_UUID_TYPE_128BIT,
    .uuid.uuid = ASSOCIATION_NUMBER_CHAR_UUID,
};

static const ble_uuid_t uuid_aws_le_custom_read_data = {
    .type = BLE_UUID_TYPE_128BIT,
    .uuid.uuid = ASSOCIATION_CUSTOM_READ_CHAR_UUID,
};

static const ble_uuid_t uuid_aws_le_custom_write_data = {
    .type = BLE_UUID_TYPE_128BIT,
    .uuid.uuid = ASSOCIATION_CUSTOM_WRITE_CHAR_UUID,
};

static bt_gattc_discovery_characteristic_t      app_le_ass_char[BT_AWS_MCE_LE_ASSOCIATION_MAX_NUMBER] = {0};
static bt_gattc_discovery_service_t             app_le_ass_gatt_srv = {0};

static bt_aws_mce_le_association_char_type_t app_speaker_le_ass_get_char_type_by_uuid(ble_uuid_t *uuid)
{
    uint16_t len = sizeof(ble_uuid_t);
    bt_aws_mce_le_association_char_type_t type = BT_AWS_MCE_LE_ASSOCIATION_CHAR_INVALID;
    if (memcmp(uuid, &uuid_aws_le_association_agent_addr, len) == 0) {
        type = BT_AWS_MCE_LE_ASSOCIATION_CHAR_AGENT_ADDR;
    } else if (memcmp(uuid, &uuid_aws_le_association_key, len) == 0) {
        type = BT_AWS_MCE_LE_ASSOCIATION_CHAR_SECRET_KEY;
    } else if (memcmp(uuid, &uuid_aws_le_association_client_addr, len) == 0) {
        type = BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_ADDR;
    } else if (memcmp(uuid, &uuid_aws_le_association_audio_lat, len) == 0) {
        type = BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_AUDIO_LATENCY;
    } else if (memcmp(uuid, &uuid_aws_le_association_voice_lat, len) == 0) {
        type = BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_VOICE_LATENCY;
    } else if (memcmp(uuid, &uuid_aws_le_association_number, len) == 0) {
        type = BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_NO;
    } else if (memcmp(uuid, &uuid_aws_le_custom_read_data, len) == 0) {
        type = BT_AWS_MCE_LE_ASSOCIATION_CHAR_CUSTOM_READ_DATA;
    } else if (memcmp(uuid, &uuid_aws_le_custom_write_data, len) == 0) {
        type = BT_AWS_MCE_LE_ASSOCIATION_CHAR_CUSTOM_WRITE_DATA;
    }
    return type;
}

static bt_status_t app_speaker_le_ass_compose_service(bt_aws_mce_le_association_service_t *service,
                                                      bt_gattc_discovery_service_t *origin_service)
{
    if (service == NULL || origin_service == NULL) {
        return BT_STATUS_LE_ASSOCIATION_PARAMETER_ERR;
    }

    for (int i = 0; i < origin_service->char_count_found; i++) {
        LOG_HEXDUMP_I(common, "uuid", &origin_service->charateristics[i].char_uuid, sizeof(ble_uuid_t));
        bt_aws_mce_le_association_char_type_t type = app_speaker_le_ass_get_char_type_by_uuid(&origin_service->charateristics[i].char_uuid);
        if (type == BT_AWS_MCE_LE_ASSOCIATION_CHAR_INVALID) {
            APPS_LOG_MSGID_E(LOG_TAG" compose_service, error type", 0);
            return BT_STATUS_LE_ASSOCIATION_PARAMETER_ERR;
        }
        service->chara[i].type = type;
        service->chara[i].value_handle = origin_service->charateristics[i].value_handle;
    }

    service->char_count = origin_service->char_count_found;
    return BT_STATUS_SUCCESS;
}

static void app_speaker_le_ass_compose_client(bt_aws_mce_le_association_client_info_t *client_info)
{
    if (client_info == NULL) {
        return;
    }

    bt_bd_addr_t *addr = bt_connection_manager_device_local_info_get_local_address();
    memcpy(&client_info->address, addr, sizeof(bt_bd_addr_t));
    client_info->audio_latency = 0x96;
    client_info->voice_latency = 0x32;
}

/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
static void app_speaker_le_ass_timer_callback(uint32_t timer_id, uint32_t data);
static bt_status_t app_speaker_le_ass_disconnect_le(bt_handle_t handle);

static void app_speaker_le_ass_reset()
{
    memset(&app_le_ass_ctx, 0x00, sizeof(bt_app_association_context));
    app_le_ass_ctx.handle = BT_HANDLE_INVALID;
}

static void app_speaker_le_ass_discovery_event_handler(bt_gattc_discovery_event_t *event)
{
    switch (event->event_type) {
        case BT_GATTC_DISCOVERY_EVENT_COMPLETE: {
            APPS_LOG_MSGID_I(LOG_TAG" discovery_event_handler, role=%02X complete", 1, app_le_ass_ctx.target_role);
            if (app_le_ass_ctx.target_role == BT_AWS_MCE_ROLE_AGENT) {
                break;
            }

            bt_gattc_discovery_service_t *service = event->params.discovered_db;
            if (service == NULL || service != &app_le_ass_gatt_srv) {
                APPS_LOG_MSGID_E(LOG_TAG" discovery_event_handler, service not found", 0);
                break;
            } else if (memcmp(&service->service_uuid, &uuid_aws_le_association_service, sizeof(ble_uuid_t)) != 0) {
                APPS_LOG_MSGID_E(LOG_TAG" discovery_event_handler, service UUID error", 0);
                break;
            } else if (service->char_count_found > BT_AWS_MCE_LE_ASSOCIATION_MAX_NUMBER
                       || service->char_count_found == 0) {
                APPS_LOG_MSGID_E(LOG_TAG" discovery_event_handler, char count %d error", 1, service->char_count_found);
                break;
            }

            bt_aws_mce_le_association_service_t aws_service = {0};
            bt_status_t bt_status  = app_speaker_le_ass_compose_service(&aws_service, service);
            if (bt_status != BT_STATUS_SUCCESS) {
                APPS_LOG_MSGID_E(LOG_TAG" discovery_event_handler, compose_service error bt_status=0x%08X", 1, bt_status);
                break;
            }

            bt_aws_mce_le_association_pair_agent_req_t req = {0};
            bt_aws_mce_le_association_client_info_t client_info = {0};
            req.handle = event->conn_handle;
            req.mode = BT_AWS_MCE_LE_ASSOCIATION_PAIR_MODE_NUMBERED;
            req.service = &aws_service;
            req.client = &client_info;
            app_speaker_le_ass_compose_client(req.client);

            bt_status = bt_aws_mce_le_association_client_start_pairing(&req);
            APPS_LOG_MSGID_I(LOG_TAG" discovery_event_handler, start_pairing bt_status=0x%08X", 1, bt_status);
            break;
        }

        case BT_GATTC_DISCOVERY_EVENT_FAIL: {
            APPS_LOG_MSGID_E(LOG_TAG" discovery_event_handler, fail", 0);
            break;
        }
    }
}

static void app_speaker_le_ass_gattc_register(void)
{
    app_le_ass_gatt_srv.characteristic_count = BT_AWS_MCE_LE_ASSOCIATION_MAX_NUMBER;
    app_le_ass_gatt_srv.charateristics = &app_le_ass_char[0];

    bt_gattc_discovery_user_data_t discovery_data = {
        .uuid.type = BLE_UUID_TYPE_128BIT,
        .uuid.uuid.uuid = ASSOCIATION_SERVICE_UUID,
        .need_cache = FALSE,
        .srv_info = &app_le_ass_gatt_srv,
        .handler = app_speaker_le_ass_discovery_event_handler
    };
    bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_LE_ASSOCIATION, &discovery_data);
}

static void app_speaker_le_ass_get_adv_data(bt_gap_le_set_ext_advertising_data_t *data)
{
    uint8_t data_context[31] = {0};
    /* uint16_t * of the 2nd parameters of bt_aws_mce_le_association_service_build_advertising_data
     * is just for extended adv. */
    bt_aws_mce_le_association_service_build_advertising_data((void *)data_context,
                                                             (uint16_t *) & (data->data_length),
                                                             app_le_ass_ctx.target_mode);
    memcpy(data->data, data_context, data->data_length);
}

static void app_speaker_le_ass_get_adv_param(bt_hci_le_set_ext_advertising_parameters_t *adv_param)
{
    adv_param->advertising_event_properties = 0b10011;
    adv_param->primary_advertising_interval_min = 0x30;
    adv_param->primary_advertising_interval_max = 0x30;
    adv_param->primary_advertising_channel_map = 0b111;
    adv_param->own_address_type = BT_ADDR_RANDOM;
    memset(&(adv_param->peer_address), 0, sizeof(bt_addr_t));
    adv_param->advertising_filter_policy = 0;
    adv_param->advertising_tx_power = 0x00;
    adv_param->primary_advertising_phy = 0x01;
    adv_param->secondary_advertising_phy = 0x01;
    adv_param->secondary_advertising_max_skip = 0;
    adv_param->scan_request_notify_enable = 0;
}

static uint32_t app_speaker_le_ass_get_adv_info(multi_ble_adv_info_t *adv_info)
{
    if (adv_info == NULL) {
        return 0;
    }
    if (adv_info->adv_data) {
        app_speaker_le_ass_get_adv_data(adv_info->adv_data);
    }
    if (adv_info->adv_param) {
        app_speaker_le_ass_get_adv_param(adv_info->adv_param);
    }
    if (adv_info->scan_rsp) {
        adv_info->scan_rsp->data_length = 0;
    }
    return 0;
}

static void app_speaker_le_ass_start_adv(bool start)
{
    APPS_LOG_MSGID_I(LOG_TAG" start_adv, start=%d", 1, start);
    if (start) {
        multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_SPK_ASS, app_speaker_le_ass_get_adv_info);
        multi_ble_adv_manager_add_ble_adv(MULTI_ADV_INSTANCE_SPK_ASS, app_speaker_le_ass_get_adv_info, 1);
        multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_SPK_ASS);
        bt_timer_ext_start(BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_ADV, 0, APP_SPEAKER_LE_ASS_TIMER, app_speaker_le_ass_timer_callback);
    } else {
        multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_SPK_ASS, app_speaker_le_ass_get_adv_info);
        multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_SPK_ASS);
        bt_timer_ext_stop(BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_ADV);
    }
}

static void app_speaker_le_ass_check_and_adv(void)
{
    APPS_LOG_MSGID_I(LOG_TAG" check_and_adv, handle=0x%04X", 1, app_le_ass_ctx.handle);
    if (BT_HANDLE_INVALID == app_le_ass_ctx.handle) {
        app_speaker_le_ass_start_adv(TRUE);
    } else {
        app_le_ass_wait_start_adv = TRUE;

        bt_status_t bt_status = app_speaker_le_ass_disconnect_le(app_le_ass_ctx.handle);
        APPS_LOG_MSGID_I(LOG_TAG" check_and_adv, disconnect bt_status=0x%08X", 1, bt_status);

        if (bt_status == BT_STATUS_CONNECTION_NOT_FOUND) {
            app_le_ass_ctx.handle = BT_HANDLE_INVALID;
            app_le_ass_wait_start_adv = FALSE;
            app_speaker_le_ass_start_adv(TRUE);
        }
    }
}

static bt_status_t app_speaker_le_ass_disconnect_le(bt_handle_t handle)
{
    bt_hci_cmd_disconnect_t disc_param = {0};
    disc_param.reason = BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION;
    disc_param.connection_handle = handle;
    bt_status_t bt_status = bt_gap_le_disconnect(&disc_param);
    return bt_status;
}

static bool app_speaker_le_ass_connect_le(bt_addr_t *peer_addr)
{
    bt_hci_cmd_le_create_connection_t conn_param = {
        .le_scan_interval = 0x30,
        .le_scan_window = 0x30,
        .initiator_filter_policy = 0x0,
        .own_address_type = BT_ADDR_RANDOM,
        .conn_interval_min = 0x18,
        .conn_interval_max = 0x18,
        .conn_latency = 0,
        .supervision_timeout = 0x1F4,
        .minimum_ce_length = 0x40,
        .maximum_ce_length = 0x100,
    };
    memcpy(&conn_param.peer_address, peer_addr, sizeof(bt_addr_t));
    uint8_t *addr = peer_addr->addr;
    bt_status_t bt_status = bt_gap_le_connect((const bt_hci_cmd_le_create_connection_t *)&conn_param);

    APPS_LOG_MSGID_I(LOG_TAG" connect_le, addr=%02X:%02X:%02X:%02X:%02X:%02X bt_status=0x%08X",
                     7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status);
    return (bt_status == BT_STATUS_SUCCESS);
}

static bool app_speaker_le_ass_stop_scan(void)
{
    bt_hci_cmd_le_set_extended_scan_enable_t enable = {
        .enable = BT_HCI_DISABLE,
        .filter_duplicates = BT_HCI_DISABLE,
        .duration = 0,
        .period = 0
    };
    bt_status_t bt_status = bt_gap_le_srv_set_extended_scan(NULL, &enable, NULL);

    app_le_ass_scan_enabled = FALSE;

    bt_timer_ext_stop(BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_SCAN);

    APPS_LOG_MSGID_I(LOG_TAG" stop_scan, bt_status=0x%08X", 1, bt_status);
    return (bt_status == BT_STATUS_SUCCESS);
}

static bool app_speaker_le_ass_start_scan(void)
{
#ifdef AIR_LE_AUDIO_BIS_ENABLE
     if (bt_sink_srv_cap_stream_is_scanning_broadcast_source()) {
         APPS_LOG_MSGID_I(LOG_TAG" start_scan, BIS is scanning", 0);
         bt_sink_srv_cap_stream_stop_scanning_broadcast_source();
         app_le_ass_wait_start_scan = TRUE;
         return false;
     }
#endif

    le_ext_scan_item_t ext_scan_1M_item = {
        .scan_type = BT_HCI_SCAN_TYPE_PASSIVE,
        .scan_interval = 0x50,
        .scan_window = 0x18
    };
    bt_hci_le_set_ext_scan_parameters_t param = {
        .own_address_type = BT_HCI_SCAN_ADDR_RANDOM,
        .scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS,
        .scanning_phys_mask = BT_HCI_LE_PHY_MASK_1M,
        .params_phy_1M = &ext_scan_1M_item,
        .params_phy_coded = NULL
    };
    bt_hci_cmd_le_set_extended_scan_enable_t enable = {
        .enable = BT_HCI_ENABLE,
        .filter_duplicates = BT_HCI_ENABLE,
        .duration = APP_SPEAKER_LE_ASS_TIMER + 5 * 1000,
        .period = 0
    };
    app_le_ass_scan_enabled = TRUE;
    bt_timer_ext_start(BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_SCAN, 0, APP_SPEAKER_LE_ASS_TIMER, app_speaker_le_ass_timer_callback);
    bt_status_t bt_status = bt_gap_le_srv_set_extended_scan(&param, &enable, NULL);
    if (bt_status != BT_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(LOG_TAG" start_scan, bt_status=0x%08X", 1, bt_status);
        app_le_ass_scan_enabled = FALSE;
    }

    APPS_LOG_MSGID_I(LOG_TAG" start_scan, success", 0);
    return (bt_status == BT_STATUS_SUCCESS);
}

static void app_speaker_le_ass_check_and_scan(void)
{
    APPS_LOG_MSGID_I(LOG_TAG" check_and_scan, handle=0x%04X", 1, app_le_ass_ctx.handle);

    if (BT_HANDLE_INVALID == app_le_ass_ctx.handle) {
        app_speaker_le_ass_start_scan();
    } else {
        app_le_ass_wait_start_scan = TRUE;

        bt_status_t bt_status = app_speaker_le_ass_disconnect_le(app_le_ass_ctx.handle);
        APPS_LOG_MSGID_I(LOG_TAG" check_and_scan, disconnect bt_status=0x%08X", 1, bt_status);

        if (bt_status == BT_STATUS_CONNECTION_NOT_FOUND) {
            app_le_ass_ctx.handle = BT_HANDLE_INVALID;
            app_le_ass_wait_start_scan = FALSE;
            app_speaker_le_ass_start_scan();
        }
    }
}

static bool app_speaker_le_ass_ungroup(void)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    APPS_LOG_MSGID_I(LOG_TAG" ungroup, role=%02X ungroup_time_start=%d", 2, role, app_le_ass_ungroup_time_start);

    if (role == BT_AWS_MCE_ROLE_AGENT) {
        bt_status = apps_aws_sync_event_send_for_broadcast(TRUE, EVENT_GROUP_UI_SHELL_LE_ASSOCIATION,
                                                           LE_ASSOCIATION_EVENT_AWS_MCE_UNGROUP, NULL, 0);
    } else if (role == BT_AWS_MCE_ROLE_PARTNER) {
        /* Only agent can send urgent aws data*/
        bt_status = apps_aws_sync_event_send_for_broadcast(FALSE, EVENT_GROUP_UI_SHELL_LE_ASSOCIATION,
                                                           LE_ASSOCIATION_EVENT_AWS_MCE_UNGROUP, NULL, 0);
    }
    APPS_LOG_MSGID_I(LOG_TAG" ungroup, bt_status=0x%08X", 1, bt_status);

    if (!app_le_ass_ungroup_time_start) {
        bt_timer_ext_stop(BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_UNGROUP);
        bt_status = bt_timer_ext_start(BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_UNGROUP, 0,
                                       200, app_speaker_le_ass_timer_callback);
        if (bt_status == BT_STATUS_SUCCESS) {
            app_le_ass_ungroup_time_start = TRUE;
        }
    }
    return (bt_status == BT_STATUS_SUCCESS);
}

static void app_speaker_le_ass_timer_callback(uint32_t timer_id, uint32_t data)
{
    if (timer_id == BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_SCAN) {
        app_speaker_le_ass_stop_scan();
        app_speaker_le_ass_reset();
        ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_LE_ASSOCIATION,
                            LE_ASSOCIATION_EVENT_SCAN_TIMEOUT, NULL, 0, NULL, 0);
        APPS_LOG_MSGID_W(LOG_TAG" timer_callback, SCAN timeout", 0);
    } else if (timer_id == BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_ADV) {
        APPS_LOG_MSGID_W(LOG_TAG" timer_callback, ADV timeout %d", 1, app_le_ass_ctx.association_done);
        if (!app_le_ass_ctx.association_done) {
            app_speaker_le_ass_start_adv(FALSE);
            app_speaker_le_ass_reset();
            ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_LE_ASSOCIATION,
                                LE_ASSOCIATION_EVENT_ADV_TIMEOUT, NULL, 0, NULL, 0);
        }
    } else if (timer_id == BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_UNGROUP) {
        APPS_LOG_MSGID_W(LOG_TAG" timer_callback, UNGROUP timeout", 0);
        app_le_ass_ungroup_time_start = FALSE;
        app_speaker_le_ass_switch_to_single();
    } else if (timer_id == BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_RESET) {
        APPS_LOG_MSGID_W(LOG_TAG" timer_callback, RESET timeout", 0);

        bt_status_t bt_status = BT_STATUS_FAIL;
        bt_aws_mce_srv_mode_t target_mode = app_le_ass_ctx.target_mode;
        bt_aws_mce_role_t target_role = app_le_ass_ctx.target_role;
        bt_aws_mce_srv_mode_t cur_mode = bt_aws_mce_srv_get_mode();
        bt_aws_mce_role_t cur_role = bt_connection_manager_device_local_info_get_aws_role();
        bt_bd_addr_t *peer_addr = bt_connection_manager_device_local_info_get_peer_aws_address();
        bt_aws_mce_srv_mode_switch_t param = {0};
        param.role = target_role;
        memcpy(&(param.addr), &(app_le_ass_ctx.peer_agent_edr_addr), sizeof(bt_bd_addr_t));
        APPS_LOG_MSGID_W(LOG_TAG" timer_callback, RESET mode=%d->%d role=%02X->%02X",
                         4, cur_mode, target_mode, cur_role, target_role);

        if (cur_mode != app_le_ass_ctx.target_mode || cur_role != app_le_ass_ctx.target_role) {
            bt_status = bt_aws_mce_srv_switch_mode(target_mode, &param);
        } else if (cur_role == BT_AWS_MCE_ROLE_PARTNER || cur_role == BT_AWS_MCE_ROLE_CLINET) {
            if (memcmp(app_le_ass_ctx.peer_agent_edr_addr, peer_addr, sizeof(bt_bd_addr_t)) != 0) {
                // Partner/Client, need to attach new agent
                APPS_LOG_MSGID_W(LOG_TAG" timer_callback, RESET attach new agent addr", 0);
                bt_status = bt_aws_mce_srv_switch_mode(target_mode, &param);
            } else {
                APPS_LOG_MSGID_W(LOG_TAG" timer_callback, RESET reconnect", 0);
                bt_status = BT_STATUS_SUCCESS;
            }
        } else {
            // Agent
            APPS_LOG_MSGID_W(LOG_TAG" timer_callback, RESET no switch", 0);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_AWS,
                                BT_AWS_MCE_SRV_EVENT_MODE_CHANGED_IND, NULL, 0, NULL, 0);
            bt_status = BT_STATUS_SUCCESS;
        }

        APPS_LOG_MSGID_W(LOG_TAG" timer_callback, RESET bt_status=0x%08X", 1, bt_status);
        if (bt_status == BT_STATUS_SUCCESS) {
            app_speaker_le_ass_reset();
        }
    }
}

static bt_status_t app_speaker_le_ass_stop_wbscan(void)
{
    /* Disable wbscan:BTA-42985. */
    bt_status_t bt_status;
    bt_cm_connect_t disc_param = {{0}, 0};
    bt_bd_addr_t bd_addr = {0};
    uint32_t connecting_number = 1;
    connecting_number = bt_cm_get_connecting_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                                                     &bd_addr, connecting_number);

    memcpy(disc_param.address, bd_addr, sizeof(bt_bd_addr_t));
    disc_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
    bt_status = bt_cm_disconnect(&disc_param);
    return bt_status ;
}

static bt_status_t app_speaker_le_ass_bt_event_handler(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_status_t bt_status = BT_STATUS_SUCCESS;
    switch (msg) {
        case BT_GAP_LE_EXT_ADVERTISING_REPORT_IND : {
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_ext_advertising_report_ind_t *ind = (bt_gap_le_ext_advertising_report_ind_t *)buffer;
                bt_status = bt_aws_mce_le_association_client_check_adv_data(ind->data, ind->data_length, app_le_ass_ctx.target_mode);
                if (bt_status == BT_STATUS_SUCCESS) {
                    APPS_LOG_MSGID_I(LOG_TAG" bt_event, scan adv match", 0);
                    app_speaker_le_ass_stop_scan();
                    app_speaker_le_ass_stop_wbscan();
                    bool success = app_speaker_le_ass_connect_le(&ind->address);
                    if (success) {
                        memcpy(&app_le_ass_ctx.agent_le_addr, &ind->address, sizeof(bt_addr_t));
                        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_ASSOCIATION, LE_ASSOCIATION_EVENT_TIMER_ASSOCIATION);
                        ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_LE_ASSOCIATION,
                                            LE_ASSOCIATION_EVENT_TIMER_ASSOCIATION, NULL, 0, NULL, APP_SPEAKER_LE_ASS_TIMER);
                    }
                }
            }

            break;
        }
        case BT_GAP_LE_SET_EXTENDED_SCAN_CNF: {
            if (status == BT_STATUS_SUCCESS && app_le_ass_wait_start_scan) {
                app_le_ass_wait_start_scan = FALSE;
                app_speaker_le_ass_start_scan();
            }
            break;
        }
        case BT_GAP_LE_CONNECT_IND: {
            bt_gap_le_connection_ind_t *conn_ind = (bt_gap_le_connection_ind_t *)buffer;
            if (conn_ind != NULL) {
                bt_bd_addr_t adv_addr = {0};
                uint8_t *local_addr = (uint8_t *)conn_ind->local_addr.addr;
                bool update_interval = true;
                if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_SPK_ASS, &adv_addr, NULL)
                    && memcmp(&adv_addr, local_addr, BT_BD_ADDR_LEN) == 0) {
                    // SPK_ASS link
                } else if (conn_ind->role == BT_ROLE_MASTER) {
                    APPS_LOG_MSGID_I(LOG_TAG" bt_event, LE_CONNECT_IND master", 0);
                } else {
                    APPS_LOG_MSGID_I(LOG_TAG" bt_event, LE_CONNECT_IND not LE ASS link", 0);
                    update_interval = false;
                    break;
                }

                bt_status_t interval_ret = BT_STATUS_SUCCESS;
                if (update_interval) {
                    bt_hci_cmd_le_connection_update_t param;
                    param.connection_handle = conn_ind->connection_handle;
                    param.conn_interval_min = 0x18;
                    param.conn_interval_max = 0x18;
                    param.conn_latency = 0;
                    param.supervision_timeout = 0x0258;/** TBC: 6000ms : 600 * 10 ms. */
                    interval_ret = bt_gap_le_update_connection_parameter(&param);
                }

                app_le_ass_ctx.handle = conn_ind->connection_handle;
                APPS_LOG_MSGID_I(LOG_TAG" bt_event, LE_CONNECT_IND handle=0x%04X, interval_ret=%d", 2,
                    app_le_ass_ctx.handle, interval_ret);

                if (conn_ind->role == BT_ROLE_MASTER) {
                    bt_app_common_bond_le_ass(app_le_ass_ctx.handle);
                }
            }
            break;
        }

        case BT_GAP_LE_BONDING_COMPLETE_IND: {
            bt_gap_le_bonding_complete_ind_t *ind = (bt_gap_le_bonding_complete_ind_t *)buffer;
            if (status == BT_STATUS_SUCCESS && ind->handle != BT_HANDLE_INVALID && app_le_ass_ctx.handle == ind->handle) {
                bt_status_t bt_status = bt_gattc_discovery_start(BT_GATTC_DISCOVERY_USER_LE_ASSOCIATION, ind->handle, FALSE);
                APPS_LOG_MSGID_I(LOG_TAG" bt_event, BONDING_COMPLETE_IND discovery bt_status=0x%08X", 1, bt_status);
            }
            break;
        }

        case BT_GAP_LE_DISCONNECT_IND: {
            bt_gap_le_disconnect_ind_t *ind = (bt_gap_le_disconnect_ind_t *)buffer;
            if (ind == NULL) {
                break;
            }

            APPS_LOG_MSGID_I(LOG_TAG" bt_event, LE_DISCONNECT_IND handle=0x%04X 0x%04X",
                             2, app_le_ass_ctx.handle, ind->connection_handle);
            if (app_le_ass_ctx.handle == ind->connection_handle) {
                app_le_ass_ctx.handle = BT_HANDLE_INVALID;


                APPS_LOG_MSGID_I(LOG_TAG" bt_event, LE_DISCONNECT_IND wait_adv=%d wait_scan=%d association=%d target_role=%02X",
                                 4, app_le_ass_wait_start_adv, app_le_ass_wait_start_scan,
                                 app_le_ass_ctx.association_done, app_le_ass_ctx.target_role);
                if (app_le_ass_wait_start_adv) {
                    app_le_ass_wait_start_adv = FALSE;
                    app_speaker_le_ass_start_adv(TRUE);
                } else if (app_le_ass_wait_start_scan) {
                    app_le_ass_wait_start_scan = FALSE;
                    app_speaker_le_ass_start_scan();
                } else if (app_le_ass_ctx.association_done) {
                    app_le_ass_ctx.association_done = FALSE;
                    app_speaker_le_ass_start_adv(FALSE);
                    ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_LE_ASSOCIATION,
                                        LE_ASSOCIATION_EVENT_ASSOCIATION_DONE, NULL, 0, NULL, 0);
                    bt_timer_ext_start(BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_RESET, 0, 1, app_speaker_le_ass_timer_callback);
                } else {
                    /* For 0x3e reason, retry connect. */
                    if (app_le_ass_ctx.target_role == BT_AWS_MCE_ROLE_PARTNER || app_le_ass_ctx.target_role == BT_AWS_MCE_ROLE_CLINET) {
                        if (ind->reason == BT_HCI_STATUS_CONNECTION_FAILED_TO_BE_ESTABLISHED) {
                            app_speaker_le_ass_connect_le(&app_le_ass_ctx.agent_le_addr);
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    return bt_status;
}



/**================================================================================*/
/**                                Middleware Interface                            */
/**================================================================================*/
static uint16_t app_le_ass_assign_num = 0;
uint16_t bt_aws_mce_le_association_service_get_assign_number(void)
{
    app_le_ass_assign_num++;
    APPS_LOG_MSGID_I(LOG_TAG" get_assign_number, num=%d", 1, app_le_ass_assign_num);
    return app_le_ass_assign_num;
}

bool bt_aws_mce_le_association_service_get_custom_data(bt_aws_mce_le_association_custom_data_t *data)
{
#ifdef AIR_LE_AUDIO_ENABLE
    // For server get custom data
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_key_t sirk = {0};
#if 0
    if (role == BT_AWS_MCE_ROLE_PARTNER || role == BT_AWS_MCE_ROLE_CLINET) {
        bt_app_common_generate_random_key((uint8_t *)sirk, sizeof(bt_key_t));
        ble_csis_set_sirk(sirk);
        ble_csis_write_nvkey_sirk(&sirk);
    } else {
        ble_csis_get_sirk(&sirk);
    }
#else
    ble_csis_get_sirk(&sirk);
#endif
    memcpy(data->data, sirk, sizeof(bt_key_t));
    data->len = sizeof(bt_key_t);
    APPS_LOG_MSGID_I(LOG_TAG" get_custom_data, [%02X] SIRK=%02X:%02X:%02X:%02X len=%d",
                     6, role, sirk[0], sirk[1], sirk[2], sirk[3], data->len);
#else
    uint8_t temp_data[4] = {6, 1, 6, 5};
    memcpy(data->data, &temp_data, 4);
    data->len = 4;
#endif
    return TRUE;
}

void bt_aws_mce_le_association_event_callback(bt_aws_mce_le_association_event_t event, bt_status_t status, void *buffer, uint16_t length)
{
    APPS_LOG_MSGID_I(LOG_TAG" event_callback, event=%d status=0x%08X", 2, event, status);
    if (status != BT_STATUS_SUCCESS) {
        return;
    }

    bt_status_t bt_status = BT_STATUS_FAIL;
    if (event == BT_AWS_MCE_LE_ASSOCIATION_EVENT_CLIENT_PAIR_IND) {
        // For Agent
        bt_aws_mce_le_association_client_pair_ind_t *ind = (bt_aws_mce_le_association_client_pair_ind_t *)buffer;
        uint8_t *addr = ind->info.address;
        APPS_LOG_MSGID_I(LOG_TAG" event_callback, CLIENT_PAIR_IND num=%d handle=0x%04X audio_latency=%d voice_latency=%d addr=%08X%04X",
                         6, ind->number, ind->handle, ind->info.audio_latency,
                         ind->info.voice_latency, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
        app_le_ass_ctx.association_done = TRUE;
    } else if (event == BT_AWS_MCE_LE_ASSOCIATION_EVENT_AGENT_PAIR_CNF) {
        // For Client
        bt_aws_mce_le_association_agent_pair_cnf_t *ind = (bt_aws_mce_le_association_agent_pair_cnf_t *) buffer;
        uint8_t *addr = ind->info.address;
        APPS_LOG_MSGID_I(LOG_TAG" event_callback, AGENT_PAIR_CNF num=%d addr=%08X%04X",
                         3, ind->number, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
        LOG_HEXDUMP_I(common, "secret_key", ind->info.secret_key, 16);

        memcpy(&(app_le_ass_ctx.peer_agent_edr_addr), &ind->info.address, sizeof(bt_bd_addr_t));
        bt_device_manager_aws_local_info_store_key(&ind->info.secret_key);
        app_le_ass_ctx.association_done = TRUE;

        bt_status = bt_aws_mce_le_association_read_custom_data(ind->handle);
        APPS_LOG_MSGID_I(LOG_TAG" event_callback, read_custom_data bt_status=0x%08X", 1, bt_status);
    } else if (event == BT_AWS_MCE_LE_ASSOCIATION_EVENT_READ_CUSTOM_DATA_CNF) {
        // For Client
        bt_aws_mce_le_association_read_custom_data_cnf_t *cnf = (bt_aws_mce_le_association_read_custom_data_cnf_t *)buffer;
        if (cnf != NULL) {
            APPS_LOG_MSGID_I(LOG_TAG" event_callback, READ_CUSTOM_DATA_CNF, len=%d data=%02X:%02X:%02X:%02X",
                             5, cnf->info.len, cnf->info.data[0], cnf->info.data[1], cnf->info.data[2], cnf->info.data[3]);
#ifdef AIR_LE_AUDIO_ENABLE
            bt_key_t sirk = {0};
            ble_csis_get_sirk(&sirk);
            if (memcmp(sirk, cnf->info.data, sizeof(bt_key_t)) == 0) {
                APPS_LOG_MSGID_I(LOG_TAG" event_callback, READ_CUSTOM_DATA_CNF same SIRK", 0);
            } else {
                memcpy(sirk, cnf->info.data, sizeof(bt_key_t));
                ble_csis_set_sirk(sirk);
                ble_csis_write_nvkey_sirk(&sirk);
            }
#endif
            /*only For test write flow, User can choose write operation or not in this point*/
            uint8_t data[4] = {2, 3, 4, 5};
            bt_aws_mce_le_association_write_custom_data(cnf->handle, data, 4);
            APPS_LOG_MSGID_I(LOG_TAG" event_callback, write_custom_data data=%02X:%02X:%02X:%02X",
                             4, data[0], data[1], data[2], data[3]);
        }
    } else if (event == BT_AWS_MCE_LE_ASSOCIATION_EVENT_WRITE_CUSTOM_DATA_CNF) {
        // For Client
        APPS_LOG_MSGID_I(LOG_TAG" event_callback, WRITE_CUSTOM_DATA_CNF", 0);
        bt_aws_mce_le_association_write_custom_data_cnf_t *cnf = (bt_aws_mce_le_association_write_custom_data_cnf_t *)buffer;
        if (cnf == NULL) {
            return;
        }

        if (app_le_ass_ctx.handle == cnf->handle) {
            bt_status = app_speaker_le_ass_disconnect_le(app_le_ass_ctx.handle);
            APPS_LOG_MSGID_I(LOG_TAG" event_callback, WRITE_CUSTOM_DATA_CNF disconnect", 0);
        }
        bt_gattc_discovery_continue(app_le_ass_ctx.handle);
    } else if (event == BT_AWS_MCE_LE_ASSOCIATION_EVENT_WRITE_CUSTOM_DATA_IND) {
        // For Server
        bt_aws_mce_le_association_write_custom_data_ind_t *ind = (bt_aws_mce_le_association_write_custom_data_ind_t *)buffer;
        uint8_t temp_data[BT_AWS_MCE_LE_ASSOCIATION_MAX_CUSTOM_DATA_LEN] = {0};
        if (ind == NULL) {
            return;
        }
        if (ind->info.len >= BT_AWS_MCE_LE_ASSOCIATION_MAX_CUSTOM_DATA_LEN) {
            ind->info.len = BT_AWS_MCE_LE_ASSOCIATION_MAX_CUSTOM_DATA_LEN;
        }
        app_le_ass_custom_data.len = ind->info.len;
        memcpy(temp_data, ind->info.data, app_le_ass_custom_data.len);
        app_le_ass_custom_data.data = temp_data;
        APPS_LOG_MSGID_I(LOG_TAG" event_callback, WRITE_CUSTOM_DATA_IND data=%02X:%02X:%02X:%02X",
                         4, app_le_ass_custom_data.data[0], app_le_ass_custom_data.data[1],
                         app_le_ass_custom_data.data[2], app_le_ass_custom_data.data[3]);
    }
}


/**================================================================================*/
/**                                   Public API                                   */
/**================================================================================*/
void app_speaker_le_ass_init()
{
    app_speaker_le_ass_reset();
    /* Customer configure option: set manufacturer name and version. */
    bt_aws_mce_le_association_set_manufacturer_version(APP_SPEAKER_LE_ASS_MANUFACTURER, APP_SPEAKER_LE_ASS_VERSION);

    // Init LE ASS service info
    bt_aws_mce_le_association_agent_info_t info = {0};
    bt_key_t *key = bt_device_manager_aws_local_info_get_key();
    if (key != NULL) {
        memcpy(&info.secret_key, key, 16);
    }
    bt_bd_addr_t *addr = bt_connection_manager_device_local_info_get_local_address();
    memcpy(&info.address, addr, sizeof(bt_bd_addr_t));
    bt_aws_mce_le_association_service_info_set(&info);

    // If previous target_mode is broadcast before reboot, set to single before BT power on.
    bt_aws_mce_srv_mode_t curr_mode = bt_aws_mce_srv_get_mode();
    if (curr_mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
        APPS_LOG_MSGID_I(LOG_TAG" init, none role for broadcast before BT power on", 0);
        bt_device_manager_aws_local_info_store_mode(BT_AWS_MCE_SRV_MODE_SINGLE);
        bt_device_manager_aws_local_info_store_role(BT_AWS_MCE_ROLE_AGENT);
        bt_bd_addr_t peer_addr = {0};
        bt_device_manager_aws_local_info_store_peer_address(&peer_addr);
    }

    // Register BT callback for middleware and APP
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_SYSTEM),
                                          (void *)bt_aws_mce_le_association_event_handler);
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_SYSTEM),
                                          (void *)app_speaker_le_ass_bt_event_handler);
    app_speaker_le_ass_gattc_register();

    bt_timer_ext_init();
    APPS_LOG_MSGID_I(LOG_TAG" init", 0);
}

void app_speaker_le_ass_action(app_speaker_le_ass_action_t action, void *buff)
{
    APPS_LOG_MSGID_I(LOG_TAG" action, action=%d target_mode=%d target_role=%02X",
                     3, action, app_le_ass_ctx.target_mode, app_le_ass_ctx.target_role);
    switch (action) {
        case BT_AWS_MCE_LE_ASSOCIATION_APP_START_PAIR: {
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            if (role == BT_AWS_MCE_ROLE_CLINET) {
                app_speaker_le_ass_check_and_scan();
            } else if (role == BT_AWS_MCE_ROLE_AGENT) {
                app_speaker_le_ass_check_and_adv();
            }
            break;
        }
        case BT_AWS_MCE_LE_ASSOCIATION_APP_START_SCAN: {
            app_speaker_le_ass_check_and_scan();
            break;
        }
        case BT_AWS_MCE_LE_ASSOCIATION_APP_START_ADV: {
            app_speaker_le_ass_check_and_adv();
            break;
        }
        case BT_AWS_MCE_LE_ASSOCIATION_APP_UNGROUP: {
            app_speaker_le_ass_ungroup();
            break;
        }
        default:
            break;
    }
}

void app_speaker_le_ass_set_param(bt_aws_mce_role_t role, bt_aws_mce_srv_mode_t mode)
{
    app_le_ass_ctx.target_role = role;
    app_le_ass_ctx.target_mode = mode;
}

bt_aws_mce_role_t app_speaker_le_ass_get_target_role(void)
{
    return app_le_ass_ctx.target_role;
}

bool app_speaker_le_ass_switch_to_single()
{
    bt_aws_mce_srv_mode_t mode = bt_aws_mce_srv_get_mode();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    app_speaker_le_ass_reset();
    app_speaker_le_ass_start_adv(FALSE);
    app_speaker_le_ass_stop_scan();

    bt_aws_mce_srv_mode_t target_mode = BT_AWS_MCE_SRV_MODE_SINGLE;
    bt_aws_mce_srv_mode_switch_t param = {0};
    bt_status_t bt_status = bt_aws_mce_srv_switch_mode(target_mode, &param);
    APPS_LOG_MSGID_I(LOG_TAG" switch_to_single, mode=%d role=%02X bt_status=0x%08X",
                     3, mode, role, bt_status);
    return (bt_status == BT_STATUS_SUCCESS);
}

void app_speaker_le_ass_get_aws_key(bt_key_t *aws_key)
{
    memcpy(aws_key, &app_speaker_le_ass_aws_key, sizeof(bt_key_t));
}

void app_speaker_le_ass_set_aws_key(bt_key_t aws_key)
{
    memcpy(&app_speaker_le_ass_aws_key, aws_key, sizeof(bt_key_t));
}
