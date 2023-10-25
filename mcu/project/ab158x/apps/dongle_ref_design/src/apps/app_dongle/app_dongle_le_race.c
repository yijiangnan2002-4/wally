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
#if (defined MTK_RACE_CMD_ENABLE)

#include "race_cmd.h"
#include "race_xport.h"
#include "race_noti.h"
#include "bt_gap_le.h"
#include "apps_race_cmd_event.h"
#include "ble_air_interface.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "bt_type.h"
#include "apps_debug.h"

#include "app_le_audio_air.h"
#include "app_dongle_connection_common.h"
#include "app_dongle_le_race.h"
#if (defined AIR_LE_AUDIO_UNICAST_ENABLE)
#include "app_le_audio_race.h"
#include "app_le_audio_ucst.h"
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "app_ull_dongle_le.h"
#endif
#ifdef AIR_BT_SOURCE_ENABLE
#include "app_bt_source_conn_mgr.h"
#endif
#ifdef AIR_GATT_SRV_CLIENT_ENABLE
#include "bt_gatt_service_client.h"
#endif
#ifdef MTK_BLE_GAP_SRV_ENABLE
#include "bt_gap_le_service.h"
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
#include "app_dongle_ull_le_hid.h"
#include "bt_ull_le_hid_service.h"
#endif

#define LOG_TAG        "[app_dongle_le_race]"

#define APP_DONGLE_SCAN_UNKNOWN_NAME        "unknown_name"
#define APP_DONGLE_SCAN_UNKNOWN_NAME_LEN    (strlen(APP_DONGLE_SCAN_UNKNOWN_NAME))

typedef struct {
    uint8_t channel;
    race_pkt_t race_pkt;
} PACKED app_dongle_le_race_event_ui_data_t;

typedef struct {
    app_dongle_le_race_sink_device_t current_sink_device;
} app_dongle_le_race_contex_t;

static app_dongle_le_race_contex_t g_le_race_ctx = {0};
static app_dongle_le_race_contex_t * app_dongle_le_race_get_ctx(void) {
    return &g_le_race_ctx;
}
static void app_dongle_le_race_event_callback(app_dongle_le_race_event_t event, app_dongle_le_race_sink_device_t sink_type, void *buff)
{
    RACE_ERRCODE race_notify_ret;

    APPS_LOG_MSGID_I(LOG_TAG" app_dongle_le_race_event_callback event: %d, buff : 0x%x, sink type: %d", 3, event, buff, sink_type);

    switch (event) {
        case APP_DONGLE_LE_RACE_EVT_ADV_REPORT: {
            if (buff) {
                bt_gap_le_ext_advertising_report_ind_t *report_ind = (bt_gap_le_ext_advertising_report_ind_t *)buff;
                uint8_t *name = NULL;
                uint32_t name_len = 0;
                uint32_t cursor_position = 0;
                for (; cursor_position + 2 < report_ind->data_length;) {
                    if (report_ind->data[cursor_position + 1] == BT_GAP_LE_AD_TYPE_NAME_COMPLETE) {
                        name = &(report_ind->data[cursor_position + 2]);
                        name_len = report_ind->data[cursor_position] - 1;
                        break;
                    } else {
                        cursor_position += report_ind->data[cursor_position] + 1;
                    }
                }
                app_dongle_le_adv_report_notify_t *noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                                                 (uint16_t)RACE_DONGLE_LE_ADV_REPORT_NOTIFY,
                                                                 (uint16_t)(sizeof(app_dongle_le_adv_report_notify_t) + name_len),
                                                                 RACE_SERIAL_PORT_TYPE_USB);
                if (noti != NULL) {
                    memcpy(&noti->addr, &report_ind->address, sizeof(bt_addr_t));
                    noti->status = 0; /* Currently not useful. */
                    noti->rssi = report_ind->rssi;
                    if (name && name_len > 0) {
                        memcpy(noti->name, name, name_len);
                    }
                    noti->sink_type = sink_type;
                    race_notify_ret = race_noti_send(noti, RACE_SERIAL_PORT_TYPE_USB, false);
                    APPS_LOG_MSGID_I(LOG_TAG"notify_adv_report resule: 0x%X.", 1, race_notify_ret);
                    if (race_notify_ret != RACE_ERRCODE_SUCCESS) {
                        RACE_FreePacket((void *)noti);
                    }
                }
            }
            break;
        }
        case APP_DONGLE_LE_RACE_EVT_CONNECT_IND: {
            if (buff) {
                app_dongle_le_race_connect_ind_t *connect_ind = (app_dongle_le_race_connect_ind_t *)buff;
                app_dongle_le_race_connect_notify_t *noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                                              (uint16_t)RACE_DONGLE_LE_CONNECT_NOTIFY,
                                                              (uint16_t)(sizeof(app_dongle_le_race_connect_notify_t)),
                                                              RACE_SERIAL_PORT_TYPE_USB);
                if (noti != NULL) {
                    noti->status = (connect_ind->ret == BT_STATUS_SUCCESS) ? 0 : 0xFF;; /* Currently not useful. */
                    noti->connected = APP_DONGLE_LE_RACE_CONNECT_EVENT_CONNECTED;
                    memcpy(&noti->addr, &connect_ind->peer_addr, sizeof(bt_addr_t));
                    APPS_LOG_MSGID_I(LOG_TAG"notify_adv_report EVT_CONNECT_IND: sink_type: %d. addr:%02X-%02X-%02X-%02X-%02X-%02X", 7, \
                        sink_type, connect_ind->peer_addr.addr[0], connect_ind->peer_addr.addr[1],connect_ind->peer_addr.addr[2], \
                        connect_ind->peer_addr.addr[3], connect_ind->peer_addr.addr[4], connect_ind->peer_addr.addr[5]);
                    if (APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS <= sink_type && \
                        APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB >= sink_type) {
                        noti->device_id = race_get_device_id_by_conn_address(&connect_ind->peer_addr.addr);
                    } else {
                        noti->device_id = ble_air_get_device_id_by_address(&connect_ind->peer_addr.addr);
                    }

                    noti->group_id = connect_ind->group_id;
                    noti->sink_type = sink_type;
                    race_notify_ret = race_noti_send(noti, RACE_SERIAL_PORT_TYPE_USB, false);
                    APPS_LOG_MSGID_I(LOG_TAG"APP_DONGLE_LE_RACE_EVT_CONNECT_IND resule: 0x%X.", 1, race_notify_ret);
                    if (race_notify_ret != RACE_ERRCODE_SUCCESS) {
                        RACE_FreePacket((void *)noti);
                    }
                }
            }
            break;
        }
        case APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND: {
            if (buff) {
                ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_AUDIO_RACE_DISCONNECTED,
                    NULL, 0, NULL, 0);
                app_dongle_le_race_disconnect_ind_t *disconnect_ind = (app_dongle_le_race_disconnect_ind_t *)buff;
                app_dongle_le_race_connect_notify_t *noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                                              (uint16_t)RACE_DONGLE_LE_CONNECT_NOTIFY,
                                                              (uint16_t)(sizeof(app_dongle_le_race_connect_notify_t)),
                                                              RACE_SERIAL_PORT_TYPE_USB);
                if (noti != NULL) {
                    noti->status = 0; /* Currently not useful. */
                    noti->connected = APP_DONGLE_LE_RACE_CONNECT_EVENT_DISCONNECTED;
                    memcpy(&noti->addr, &disconnect_ind->peer_addr, sizeof(bt_addr_t));
                    noti->device_id = BT_AIR_DEVICE_ID_INVAILD;
                    noti->sink_type = sink_type;
                    race_notify_ret = race_noti_send(noti, RACE_SERIAL_PORT_TYPE_USB, true);
                    APPS_LOG_MSGID_I(LOG_TAG"notify_le_audio_disconnected resule: 0x%X.", 1, race_notify_ret);
                    if (race_notify_ret != RACE_ERRCODE_SUCCESS) {
                        RACE_FreePacket((void *)noti);
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}


static void app_dongle_le_race_reply_empty_device_list(bool device_list_or_pair_list, uint8_t channel_id)
{
    APPS_LOG_MSGID_I(LOG_TAG"reply_empty_device_list %d", 1, device_list_or_pair_list);

    RACE_ERRCODE race_ret;
    app_dongle_le_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                          (uint16_t)(device_list_or_pair_list ? RACE_DONGLE_LE_GET_DEVICE_LIST : RACE_DONGLE_LE_GET_PAIRED_LIST),
                                                                          (uint16_t)(sizeof(app_dongle_le_race_get_device_list_rsp_t)),
                                                                          channel_id);
    if (response) {
        response->status = 0;
        race_ret = race_noti_send(response, channel_id, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }
}

static void app_dongle_le_race_reply_empty_status_handler(uint8_t channel_id)
{
    APPS_LOG_MSGID_I(LOG_TAG"reply_empty_device_list %d", 0);

    RACE_ERRCODE race_ret;
    app_dongle_le_race_get_device_status_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                         (uint16_t)RACE_DONGLE_LE_GET_DEVICE_STATUS,
                                                                         (uint16_t)(sizeof(app_dongle_le_race_get_device_status_rsp_t)),
                                                                          channel_id);
    if (response) {
        response->status = 0xFF;
        race_ret = race_noti_send(response, channel_id, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }

}


#ifdef AIR_BT_SOURCE_ENABLE
static void app_dongle_le_race_reply_bta_device_list(bool device_list_or_pair_list, uint8_t channel_id)
{
    uint8_t conn_num = 0;
    bt_addr_t list[2] = {0};
    app_bt_source_conn_mgr_get_conn_info(&conn_num, list);
    APPS_LOG_MSGID_I(LOG_TAG"[BT_SRC] reply_bta_device_list, device_list=%d conn_num=%d",
                     2, device_list_or_pair_list, conn_num);
    configASSERT(conn_num <= 2);

    RACE_ERRCODE race_ret;
    app_dongle_le_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                          (uint16_t)(device_list_or_pair_list ? RACE_DONGLE_LE_GET_DEVICE_LIST : RACE_DONGLE_LE_GET_PAIRED_LIST),
                                                                          (uint16_t)(sizeof(app_dongle_le_race_get_device_list_rsp_t) + conn_num * sizeof(app_dongle_le_race_device_status_item_t)),
                                                                          channel_id);
    if (response) {
        response->status = 0;
        for (int i = 0; i < conn_num; i++) {
            memcpy(&response->devices_list[i].addr, &list[i], sizeof(bt_addr_t));
            response->devices_list[i].group_id = 0;
            response->devices_list[i].device_id = APP_DONGLE_DEVICE_ID_BASE + i;
            response->devices_list[i].role = 0xFF;
        }

        race_ret = race_noti_send(response, channel_id, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }
}

static void app_dongle_le_race_reply_bta_device_status(uint8_t channel_id)
{
    uint8_t conn_num = 0;
    uint8_t device_id = RACE_TARGET_INVALID_DEVICE;
    bt_addr_t list[2] = {0};
    app_bt_source_conn_mgr_get_conn_info(&conn_num, list);
    if (conn_num > 0) {
        const uint8_t *addr = list[0].addr;
        device_id = race_get_device_id_by_conn_address((bt_bd_addr_t *)addr);
        APPS_LOG_MSGID_I(LOG_TAG"[BT_SRC] reply_bta_device_status, conn_num=%d addr=%02X:%02X:%02X:%02X:%02X:%02X device_id=%02X",
                         8, conn_num, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], device_id);
    } else {
        APPS_LOG_MSGID_W(LOG_TAG"[BT_SRC] reply_bta_device_status, no connection", 0);
    }

    RACE_ERRCODE race_ret;
    app_dongle_le_race_get_device_status_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                           (uint16_t)RACE_DONGLE_LE_GET_DEVICE_STATUS,
                                                                           (uint16_t)(sizeof(app_dongle_le_race_get_device_status_rsp_t)),
                                                                           channel_id);


    if (response != NULL) {
        if (conn_num > 0 && device_id != RACE_TARGET_INVALID_DEVICE) {
            response->status = 0;
            memcpy(&response->addr, &list[0], sizeof(bt_addr_t));
            response->device_id = device_id;
            response->group_id = 0;
            response->role = 0xFF;
        } else {
            response->status = APP_DONGLE_LE_RACE_GET_STATUS_NOT_CONNECTED_STATUS;
            memcpy(&response->addr, &list[0], sizeof(bt_addr_t));
            response->device_id = 0xFF;
            response->group_id = 0xFF;
            response->role = 0xFF;
        }
        race_ret = race_noti_send(response, channel_id, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }
}

#endif

void app_dongle_le_race_reply_scan_report(uint8_t sink_type, uint8_t addr_type, uint8_t *addr, uint8_t rssi,
                                          const char *name, uint8_t name_len)
{
    if (name == NULL || name_len == 0) {
        name = APP_DONGLE_SCAN_UNKNOWN_NAME;
        name_len = strlen(APP_DONGLE_SCAN_UNKNOWN_NAME);
    }

    app_dongle_le_adv_report_notify_t *noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                                               (uint16_t)RACE_DONGLE_LE_ADV_REPORT_NOTIFY,
                                                               (uint16_t)(sizeof(app_dongle_le_adv_report_notify_t) + name_len),
                                                               RACE_SERIAL_PORT_TYPE_USB);
    if (noti != NULL) {
        noti->addr.type = addr_type;
        memcpy(&noti->addr.addr, addr, BT_BD_ADDR_LEN);
        noti->status = 0; /* Currently not useful. */
        noti->rssi = rssi;
        if (name != NULL && name_len > 0) {
            memcpy(noti->name, name, name_len);
        }
        noti->sink_type = sink_type;
        RACE_ERRCODE ret = race_noti_send(noti, RACE_SERIAL_PORT_TYPE_USB, FALSE);
        APPS_LOG_MSGID_I(LOG_TAG"[BT_SRC] reply_scan_report, sink_type=%d addr=%08X%04X rssi=%d ret=%d",
                         5, sink_type, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), rssi, ret);
        if (ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)noti);
        }
    }
}

void app_dongle_le_race_notify_connect_event(uint8_t status, uint8_t sink_type, bool conn_or_disconn,
                                             uint8_t addr_type, uint8_t *addr, uint8_t device_id, uint8_t group_id)
{
    app_dongle_le_race_connect_notify_t *noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                                                 (uint16_t)RACE_DONGLE_LE_CONNECT_NOTIFY,
                                                                 (uint16_t)(sizeof(app_dongle_le_race_connect_notify_t)),
                                                                 RACE_SERIAL_PORT_TYPE_USB);
    if (noti != NULL) {
        noti->status = status;
        noti->connected = (conn_or_disconn ? APP_DONGLE_LE_RACE_CONNECT_EVENT_CONNECTED : APP_DONGLE_LE_RACE_CONNECT_EVENT_DISCONNECTED);
        noti->addr.type = addr_type;
        memcpy(&noti->addr.addr, addr, BT_BD_ADDR_LEN);
        noti->sink_type = sink_type;
        noti->device_id = device_id;
        noti->group_id = group_id;

        RACE_ERRCODE ret = race_noti_send(noti, RACE_SERIAL_PORT_TYPE_USB, false);
        APPS_LOG_MSGID_I(LOG_TAG"[BT_SRC] notify_connect_event, status=%d conn_or_disconn=%d ret=%d",
                         3, status, conn_or_disconn, ret);
        if (ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)noti);
        }
    }
}

#if defined(AIR_LE_AUDIO_UNICAST_ENABLE) && defined(AIR_GATT_SRV_CLIENT_ENABLE)
static bt_status_t app_dongle_le_get_device_name_callback(bt_gatt_srv_client_event_t event, bt_status_t status, void *parameter)
{
    RACE_ERRCODE race_ret;
    APPS_LOG_MSGID_I(LOG_TAG"[BT_SRC] app_dongle_le_get_device_name_callback, event = %02x status = %02x", 2, event, status);
    switch (event) {
        case BT_GATT_SRV_CLIENT_EVENT_READ_VALUE_COMPLTETE: {
            app_dongle_le_race_get_device_name_rsp_t *response = NULL;
            if (status == BT_STATUS_SUCCESS) {
                bt_gatt_srv_client_read_value_complete_t *read_value_complete = (bt_gatt_srv_client_read_value_complete_t *)parameter;
                bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(read_value_complete->connection_handle);
                if (conn_info == NULL) {
                    return BT_STATUS_FAIL;
                }

                response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                            (uint16_t)RACE_DONGLE_LE_GET_DEVICE_NAME,
                                            (uint16_t)(sizeof(app_dongle_le_race_get_device_name_rsp_t) + read_value_complete->length),
                                            RACE_SERIAL_PORT_TYPE_USB);
                if (NULL == response) {
                    return BT_STATUS_FAIL;
                }
                response->status = 0;
                memcpy(&response->addr, &conn_info->peer_addr, sizeof(bt_addr_t));
                memcpy(response->name, read_value_complete->data, read_value_complete->length);
            } else {
                bt_gatt_srv_client_action_error_t *error_event = (bt_gatt_srv_client_action_error_t *)parameter;
                bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(error_event->connection_handle);
                if (conn_info == NULL) {
                    return BT_STATUS_FAIL;
                }

                response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                            (uint16_t)RACE_DONGLE_LE_GET_DEVICE_NAME,
                                            (uint16_t)(sizeof(app_dongle_le_race_get_device_name_rsp_t) + 0),
                                            RACE_SERIAL_PORT_TYPE_USB);
                if (NULL == response) {
                    return BT_STATUS_FAIL;
                }

                response->status = 0XFF;
                memcpy(&response->addr, &conn_info->peer_addr, sizeof(bt_addr_t));
            }

            if (response) {
                race_ret = race_noti_send(response, RACE_SERIAL_PORT_TYPE_USB, false);
                if (race_ret != RACE_ERRCODE_SUCCESS) {
                    RACE_FreePacket((void *)response);
                }
            }
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}
#endif

bool app_dongle_le_race_cmd_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = true;
    RACE_ERRCODE race_ret;
    bt_status_t bt_status = BT_STATUS_FAIL;
    app_dongle_le_race_contex_t *ctx = app_dongle_le_race_get_ctx();
    if (extra_data && data_len >= sizeof(app_dongle_le_race_event_ui_data_t)) {
        app_dongle_le_race_event_ui_data_t *race_data = (app_dongle_le_race_event_ui_data_t *)extra_data;
        APPS_LOG_MSGID_I(LOG_TAG"[BT_SRC] app_dongle_le_race_cmd_event_proc, race_cmd=0x%04X", 1, race_data->race_pkt.hdr.id);
        switch (race_data->race_pkt.hdr.id) {
            case RACE_DONGLE_LE_SET_LINK_TYPE: {
                app_dongle_le_race_set_link_type_cmd_t *set_link_type = (app_dongle_le_race_set_link_type_cmd_t *) & (race_data->race_pkt.payload);
                APPS_LOG_MSGID_I(LOG_TAG" Race set link type, type=%d", 1, set_link_type->type);
                ctx->current_sink_device = set_link_type->type;

                app_dongle_le_race_set_link_type_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                          (uint16_t)RACE_DONGLE_LE_SET_LINK_TYPE,
                                                                          (uint16_t)(sizeof(app_dongle_le_race_set_link_type_rsp_t)),
                                                                          race_data->channel);
                if (response != NULL) {
                    response->status = 0;
                    response->type = set_link_type->type;
                    race_ret = race_noti_send(response, race_data->channel, false);
                    APPS_LOG_MSGID_I(LOG_TAG"send rsp of set link type resule: 0x%X.", 1, race_ret);
                    if (race_ret != RACE_ERRCODE_SUCCESS) {
                        RACE_FreePacket((void *)response);
                    }
                }
                break;
            }
            case RACE_DONGLE_LE_GET_LINK_TYPE: {
                APPS_LOG_MSGID_I(LOG_TAG" Race get link type, type=%d", 1, ctx->current_sink_device);
                app_dongle_le_race_get_link_type_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                          (uint16_t)RACE_DONGLE_LE_GET_LINK_TYPE,
                                                                          (uint16_t)(sizeof(app_dongle_le_race_get_link_type_rsp_t)),
                                                                          race_data->channel);
                if (response != NULL) {
                    response->status = 0;
                    response->type = ctx->current_sink_device;
                    race_ret = race_noti_send(response, race_data->channel, false);
                    APPS_LOG_MSGID_I(LOG_TAG"send rsp of get link type resule: 0x%X.", 1, race_ret);
                    if (race_ret != RACE_ERRCODE_SUCCESS) {
                        RACE_FreePacket((void *)response);
                    }
                }

                break;
            }

            case RACE_DONGLE_LE_SCAN: {
                app_dongle_le_race_scan_cmd_t *scan_cmd_data = (app_dongle_le_race_scan_cmd_t *) & (race_data->race_pkt.payload);
                bt_status_t temp_status = BT_STATUS_FAIL;

#if defined(AIR_BT_SOURCE_ENABLE)
                temp_status = app_bt_source_conn_mgr_control_scan(scan_cmd_data->start_scan, scan_cmd_data->timeout_seconds * 1000);
#endif

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
                if (scan_cmd_data->start_scan) {
                    bt_status = app_dongle_ull_le_hid_start_scan(APP_DONGLE_ULL_LE_HID_SCAN_AUDIO | APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD | APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
                    if (bt_status == BT_STATUS_SUCCESS) {
                        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_SCAN_END);
                        if (scan_cmd_data->timeout_seconds) {
                            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                APPS_EVENTS_INTERACTION_LE_SCAN_END, NULL, 0, NULL, scan_cmd_data->timeout_seconds * 1000);
                        }
                    }

                } else {
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_SCAN_END);
                    bt_status = app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_AUDIO | APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD | APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
                }
#else

                if (scan_cmd_data->start_scan) {
                    bt_status = app_dongle_cm_le_start_scan_device(APP_DONGLE_CM_CONNECTION_USING_FULL_SCAN_FOR_USER);
                    if (bt_status == BT_STATUS_SUCCESS) {
                        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_SCAN_END);
                        if (scan_cmd_data->timeout_seconds) {
                            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                APPS_EVENTS_INTERACTION_LE_SCAN_END, NULL, 0, NULL, scan_cmd_data->timeout_seconds * 1000);
                        }
                    }
                } else {
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_SCAN_END);
                    bt_status = app_dongle_cm_le_stop_scan_device();
                }
#endif
                bt_status = ((temp_status == BT_STATUS_SUCCESS || bt_status == BT_STATUS_SUCCESS) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL);
                APPS_LOG_MSGID_I(LOG_TAG"[BT_SRC] app_dongle_le_race_cmd_event_proc, start scan(%d), bt_ret = 0x%x, timeout = %d",
                                  3, scan_cmd_data->start_scan, bt_status, scan_cmd_data->timeout_seconds);
                app_dongle_le_race_scan_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                          (uint16_t)RACE_DONGLE_LE_SCAN,
                                                                          (uint16_t)(sizeof(app_dongle_le_race_scan_rsp_t)),
                                                                          race_data->channel);
                if (response != NULL) {
                    response->status = (bt_status == BT_STATUS_SUCCESS) ? 0 : 0xFF;
                    race_ret = race_noti_send(response, race_data->channel, false);
                    APPS_LOG_MSGID_I(LOG_TAG"send rsp of RACE_DONGLE_LE_SCAN resule: 0x%X.", 1, race_ret);
                    if (race_ret != RACE_ERRCODE_SUCCESS) {
                        RACE_FreePacket((void *)response);
                    }
                }
                break;
            }
            case RACE_DONGLE_LE_CONNECT: {
                app_dongle_le_race_connect_cmd_t *race_conn_data = (app_dongle_le_race_connect_cmd_t *) & (race_data->race_pkt.payload);
                bt_addr_t address;
                app_dongle_cm_source_t source_type = APP_DONGLE_CM_SOURCE_INVALID;
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
                bt_ull_le_hid_srv_device_t device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
#endif
                memcpy(&address, &race_conn_data->addr, sizeof(bt_addr_t));
                switch (ctx->current_sink_device) {
                    case APP_DONGLE_LE_RACE_SINK_DEVICE_LEA: {
                        source_type = APP_DONGLE_CM_SOURCE_LEA;
                        break;
                    }
                    case APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2: {
                        source_type = APP_DONGLE_CM_SOURCE_ULL_V2;
                        break;
                    }
                    case APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC: {
                        source_type = APP_DONGLE_CM_SOURCE_BTA;
                        break;
                    }
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
                    case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS: {
                        device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
                        break;
                    }
                    case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB: {
                        device_type = BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD;
                        break;
                    }
                    case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS: {
                        device_type = BT_ULL_LE_HID_SRV_DEVICE_MOUSE;
                        break;
                    }
                    case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB: {
                        device_type = BT_ULL_LE_HID_SRV_DEVICE_EARBUDS;
                        break;
                    }
#endif
                    default:
                        break;

                }

                APPS_LOG_MSGID_I(LOG_TAG" app_dongle_le_race_cmd_event_proc, sink type: %d, connect(%d), bt_ret = 0x%x",
                                  3, ctx->current_sink_device, race_conn_data->connect, bt_status);

                if (APP_DONGLE_CM_SOURCE_INVALID != source_type) {
                        if (race_conn_data->connect) {
                            bt_status = app_dongle_cm_le_create_connection(source_type, address);
                        } else {
                            if (APP_DONGLE_LE_RACE_SINK_DEVICE_LEA == ctx->current_sink_device) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
                        bt_status = app_le_audio_ucst_disconnect_device(&address);
#endif
                            } else {
                                bt_status = app_dongle_cm_le_disconnect(source_type, address);
                            }
                        }
                }

 #if defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
                else if (BT_ULL_LE_HID_SRV_DEVICE_NONE != device_type) {
                    if (race_conn_data->connect)
                    {
                        bt_status = app_dongle_ull_le_hid_connect_device(device_type, &address);
                    } else {
                        bt_status = app_dongle_ull_le_hid_disconnect_device(device_type, BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION, &address);

                    }

                }
#endif
                app_dongle_le_race_connect_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                             (uint16_t)RACE_DONGLE_LE_CONNECT,
                                                                             (uint16_t)(sizeof(app_dongle_le_race_connect_rsp_t)),
                                                                             race_data->channel);
                if (response != NULL) {
                    response->connect = race_conn_data->connect;
                    memcpy(&response->addr, &race_conn_data->addr, sizeof(bt_addr_t));
                    response->status = (bt_status == BT_STATUS_SUCCESS) ? 0 : 0xFF;
                    race_ret = race_noti_send(response, race_data->channel, false);
                    APPS_LOG_MSGID_I(LOG_TAG"send rsp of RACE_DONGLE_LE_CONNECT resule: 0x%X.", 1, race_ret);
                    if (race_ret != RACE_ERRCODE_SUCCESS) {
                        RACE_FreePacket((void *)response);
                    }
                }
                break;
            }
            case RACE_DONGLE_LE_GET_DEVICE_STATUS: {
#if (defined AIR_LE_AUDIO_UNICAST_ENABLE) || (defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
                app_dongle_le_race_get_device_status_cmd_t *race_get_status_cmd = (app_dongle_le_race_get_device_status_cmd_t *) & (race_data->race_pkt.payload);
#endif
                APPS_LOG_MSGID_I(LOG_TAG" get device status, sink type: 0x%x", 1, ctx->current_sink_device);
                if (APP_DONGLE_LE_RACE_SINK_DEVICE_LEA == ctx->current_sink_device) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
                    app_le_audio_race_get_device_status_handler(race_data->channel, race_get_status_cmd);
#else
                    app_dongle_le_race_reply_empty_status_handler(race_data->channel);

#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2 == ctx->current_sink_device) {
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
                    app_ull_dongle_le_get_device_status_handler(race_data->channel, race_get_status_cmd);
#else
                    app_dongle_le_race_reply_empty_status_handler(race_data->channel);

#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC == ctx->current_sink_device) {
#ifdef AIR_BT_SOURCE_ENABLE
                    app_dongle_le_race_reply_bta_device_status(race_data->channel);
#else
                    RACE_ERRCODE race_ret;
                    app_dongle_le_race_get_device_status_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                                         (uint16_t)RACE_DONGLE_LE_GET_DEVICE_STATUS,
                                                                                         (uint16_t)(sizeof(app_dongle_le_race_get_device_status_rsp_t)),
                                                                                          race_data->channel);
                    if (response) {
                        response->status = 0xFF;
                        race_ret = race_noti_send(response, race_data->channel, false);
                        if (race_ret != RACE_ERRCODE_SUCCESS) {
                            RACE_FreePacket((void *)response);
                        }
                    }
#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS == ctx->current_sink_device \
                    || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB == ctx->current_sink_device \
                    || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS == ctx->current_sink_device \
                    || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB == ctx->current_sink_device) {
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
                    app_dongle_ull_le_hid_get_device_status_handler(race_data->channel, ctx->current_sink_device, race_get_status_cmd);
#else
                    app_dongle_le_race_reply_empty_status_handler(race_data->channel);
#endif
                } else {
                    app_dongle_le_race_reply_empty_status_handler(race_data->channel);
                }
                break;
            }

            case RACE_DONGLE_LE_GET_DEVICE_LIST: {
                APPS_LOG_MSGID_I(LOG_TAG"[BT_SRC] get device list, sink_type=%d", 1, ctx->current_sink_device);

                if (APP_DONGLE_LE_RACE_SINK_DEVICE_LEA == ctx->current_sink_device) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
                    app_le_audio_race_get_device_list_handler(race_data->channel);
#else
                    app_dongle_le_race_reply_empty_device_list(TRUE, race_data->channel);
#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2 == ctx->current_sink_device) {
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
                    app_ull_dongle_le_get_device_list_handler(race_data->channel);
#else
                    app_dongle_le_race_reply_empty_device_list(TRUE, race_data->channel);
#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC == ctx->current_sink_device) {
#ifdef AIR_BT_SOURCE_ENABLE
                    app_dongle_le_race_reply_bta_device_list(TRUE, race_data->channel);
#else
                    app_dongle_le_race_reply_empty_device_list(TRUE, race_data->channel);
#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS == ctx->current_sink_device \
                    || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB == ctx->current_sink_device \
                    || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS == ctx->current_sink_device \
                    || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB == ctx->current_sink_device) {
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
                    app_dongle_ull_le_hid_get_device_list_handler(race_data->channel, ctx->current_sink_device);
#else
                    app_dongle_le_race_reply_empty_device_list(TRUE, race_data->channel);
#endif
                }
                else {
                    app_dongle_le_race_reply_empty_device_list(TRUE, race_data->channel);
                }
                break;
            }
            case RACE_DONGLE_LE_SWITCH_ACTIVE_DEVICE: {
                app_dongle_le_race_switch_active_audio_cmd_t *race_switch_device_cmd = (app_dongle_le_race_switch_active_audio_cmd_t *) & (race_data->race_pkt.payload);
                APPS_LOG_MSGID_I(LOG_TAG" switch active audio, sink type: 0x%x", 1, ctx->current_sink_device);
                if (APP_DONGLE_LE_RACE_SINK_DEVICE_LEA == ctx->current_sink_device) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
                    app_le_audio_race_switch_active_device_handler(race_data->channel, race_switch_device_cmd);
#else
                    RACE_ERRCODE race_ret;
                    app_dongle_le_race_switch_active_audio_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                                         (uint16_t)RACE_DONGLE_LE_SWITCH_ACTIVE_DEVICE,
                                                                                         (uint16_t)(sizeof(app_dongle_le_race_switch_active_audio_rsp_t)),
                                                                                          race_data->channel);

                    if (response) {
                        response->set_or_get = race_switch_device_cmd->set_or_get;
                        response->status = 0xFF;
                        response->group_id = 0x00;
                        race_ret = race_noti_send(response, race_data->channel, false);
                        if (race_ret != RACE_ERRCODE_SUCCESS) {
                            RACE_FreePacket((void *)response);
                        }
                    }
#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2 == ctx->current_sink_device) {
                    //TODO
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
                    app_ull_dongle_le_switch_active_device_handler(race_data->channel, race_switch_device_cmd);
#else
                    RACE_ERRCODE race_ret;
                    app_dongle_le_race_switch_active_audio_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                                         (uint16_t)RACE_DONGLE_LE_SWITCH_ACTIVE_DEVICE,
                                                                                         (uint16_t)(sizeof(app_dongle_le_race_switch_active_audio_rsp_t)),
                                                                                          race_data->channel);

                    if (response) {
                        response->set_or_get = race_switch_device_cmd->set_or_get;
                        response->status = 0xFF;
                        response->group_id = 0x00;
                        race_ret = race_noti_send(response, race_data->channel, false);
                        if (race_ret != RACE_ERRCODE_SUCCESS) {
                            RACE_FreePacket((void *)response);
                        }
                    }

#endif
                }
                break;
            }
            case RACE_DONGLE_LE_GET_PAIRED_LIST: {
                if (APP_DONGLE_LE_RACE_SINK_DEVICE_LEA == ctx->current_sink_device) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
                    app_le_audio_race_get_paired_device_list_handler(race_data->channel);
#else
                    app_dongle_le_race_reply_empty_device_list(FALSE, race_data->channel);
#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC == ctx->current_sink_device) {
#ifdef AIR_BT_SOURCE_ENABLE
                    app_dongle_le_race_reply_bta_device_list(FALSE, race_data->channel);
#else
                    app_dongle_le_race_reply_empty_device_list(FALSE, race_data->channel);
#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS == ctx->current_sink_device \
                    || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB == ctx->current_sink_device \
                    || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS == ctx->current_sink_device \
                    || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB == ctx->current_sink_device) {
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
                    app_dongle_ull_le_hid_get_paired_list_handler(race_data->channel, ctx->current_sink_device);
#else
                    app_dongle_le_race_reply_empty_device_list(FALSE, race_data->channel);
#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2 == ctx->current_sink_device) {
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
                    app_ull_dongle_le_get_paired_device_list_handler(race_data->channel);
#else
                    app_dongle_le_race_reply_empty_device_list(FALSE, race_data->channel);
#endif
                }
                else {
                    app_dongle_le_race_reply_empty_device_list(FALSE, race_data->channel);
                }
                break;
            }

            case RACE_DONGLE_LE_REMOVE_PAIRED_RECORD: {
                bt_addr_t *p_addr = (bt_addr_t *) & (race_data->race_pkt.payload);
                app_dongle_le_race_remove_paired_record_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                                        (uint16_t)RACE_DONGLE_LE_REMOVE_PAIRED_RECORD,
                                                                                        (uint16_t)(sizeof(app_dongle_le_race_remove_paired_record_rsp_t)),
                                                                                        race_data->channel);
                if (response) {
                    response->status = 0xFF;
                    memcpy(&response->addr, p_addr, sizeof(bt_addr_t));
                    if (APP_DONGLE_LE_RACE_SINK_DEVICE_LEA == ctx->current_sink_device) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
                        app_le_audio_ucst_delete_device(p_addr);
                        response->status = 0;
#endif
                    } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC == ctx->current_sink_device) {
#ifdef AIR_BT_SOURCE_ENABLE
                        app_bt_source_conn_mgr_remove_record(p_addr->addr);
                        response->status = 0;
#endif
                    } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS == ctx->current_sink_device \
                        || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB == ctx->current_sink_device \
                        || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS == ctx->current_sink_device \
                        || APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB == ctx->current_sink_device) {
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
                        app_dongle_ull_le_hid_delete_device(ctx->current_sink_device, p_addr);
                        response->status = 0;
#endif
                    }
                    race_ret = race_noti_send(response, race_data->channel, false);
                    if (race_ret != RACE_ERRCODE_SUCCESS) {
                        RACE_FreePacket((void *)response);
                    }
                }
                break;
            }

            case RACE_DONGLE_LE_GET_DEVICE_NAME: {
                bt_addr_t *p_addr = (bt_addr_t *) &(race_data->race_pkt.payload);
                char *name = (char *)pvPortMalloc(BT_GAP_MAX_DEVICE_NAME_LENGTH);
                uint32_t name_len = 0;
                if (name == NULL) {
                    APPS_LOG_MSGID_E(LOG_TAG" GET_DEVICE_NAME, malloc fail", 0);
                    break;
                }

                memset(name, 0, BT_GAP_MAX_DEVICE_NAME_LENGTH);
                if (APP_DONGLE_LE_RACE_SINK_DEVICE_LEA == ctx->current_sink_device) {
#if defined(AIR_LE_AUDIO_UNICAST_ENABLE) && defined(AIR_GATT_SRV_CLIENT_ENABLE)
                    bt_handle_t conn_handle = bt_gap_le_srv_get_conn_handle_by_address((const bt_bd_addr_t *)p_addr->addr);
                    if (conn_handle != 0) {
                        bt_gatt_srv_client_action_read_value_t read_value = {
                            .type = BT_GATT_SRV_CLIENT_DEVICE_NAME,
                            .connection_handle = conn_handle,
                            .callback = app_dongle_le_get_device_name_callback
                        };
                        bt_gatt_srv_client_send_action(BT_GATT_SRV_CLIENT_ACTION_READ_VALUE, &read_value, sizeof(bt_gatt_srv_client_action_read_value_t));
                        vPortFree(name);
                        break;
                    }
                    /*
                    bt_status = bt_gattc_service_get_device_name(p_addr, name);
                    if (bt_status == BT_STATUS_SUCCESS) {
                        response->status = 0;
                        name_len = strlen(name);
                        name_len = name_len <= 512 ? name_len : 512;
                    }
                    */
                    // ToDo
#endif
                    bt_status = BT_STATUS_SUCCESS;
                    memcpy(name, APP_DONGLE_SCAN_UNKNOWN_NAME, APP_DONGLE_SCAN_UNKNOWN_NAME_LEN);
                    name_len = APP_DONGLE_SCAN_UNKNOWN_NAME_LEN;
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC == ctx->current_sink_device) {
#ifdef AIR_BT_SOURCE_ENABLE
                    bt_status = app_bt_source_conn_mgr_find_name(p_addr->addr, name);
                    if (bt_status == BT_STATUS_SUCCESS) {
                        name_len = strlen(name);
                    }
#endif
                } else if (APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2 == ctx->current_sink_device) {
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
                    // ToDo
                    bt_status = BT_STATUS_SUCCESS;
                    memcpy(name, APP_DONGLE_SCAN_UNKNOWN_NAME, APP_DONGLE_SCAN_UNKNOWN_NAME_LEN);
                    name_len = APP_DONGLE_SCAN_UNKNOWN_NAME_LEN;
#endif
                }

                app_dongle_le_race_get_device_name_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                                        (uint16_t)RACE_DONGLE_LE_GET_DEVICE_NAME,
                                                                                        (uint16_t)(sizeof(app_dongle_le_race_get_device_name_rsp_t) + name_len),
                                                                                        race_data->channel);
                if (response) {
                    response->status = (bt_status == BT_STATUS_SUCCESS ? 0 : 0xFF);
                    memcpy(&response->addr, p_addr, sizeof(bt_addr_t));
                    memcpy(response->name, name, name_len);
                    race_ret = race_noti_send(response, race_data->channel, false);
                    if (race_ret != RACE_ERRCODE_SUCCESS) {
                        RACE_FreePacket((void *)response);
                    }
                }
                vPortFree(name);
                break;
            }
            default:
                break;
        }
    }

    return ret;
}

void *app_dongle_le_race_cmd_handler(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id)
{
    void *response = NULL;
    APPS_LOG_MSGID_I(LOG_TAG" app_dongle_le_race_cmd_handler, Race ID: 0x%x", 1, p_race_package->hdr.id);
    switch (p_race_package->hdr.id) {
        case RACE_DONGLE_LE_SCAN:
        case RACE_DONGLE_LE_ADV_REPORT:
        case RACE_DONGLE_LE_CONNECT:
        case RACE_DONGLE_LE_GET_DEVICE_STATUS:
        case RACE_DONGLE_LE_GET_DEVICE_LIST:
        case RACE_DONGLE_LE_SWITCH_ACTIVE_DEVICE:
        case RACE_DONGLE_LE_GET_PAIRED_LIST:
        case RACE_DONGLE_LE_REMOVE_PAIRED_RECORD:
        case RACE_DONGLE_LE_GET_DEVICE_NAME:
        case RACE_DONGLE_LE_GET_LINK_TYPE:
        case RACE_DONGLE_LE_SET_LINK_TYPE: {
            uint32_t data_len = sizeof(app_dongle_le_race_event_ui_data_t) + p_race_package->hdr.length - sizeof(p_race_package->hdr.id);
            app_dongle_le_race_event_ui_data_t *extra_data = (app_dongle_le_race_event_ui_data_t *)pvPortMalloc(data_len);
            if (extra_data) {
                memcpy(&extra_data->race_pkt, p_race_package, sizeof(race_pkt_t) + p_race_package->hdr.length - sizeof(p_race_package->hdr.id));
                extra_data->channel = channel_id;
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_DONGLE_LE_RACE,
                                    0, (void *)extra_data, data_len, NULL, 0);
            }
            break;
        }
        default:
            break;
    }

    return response;
}

bool app_dongle_le_race_interaction_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    bt_status_t bt_status;
    switch (event_id) {
        case APPS_EVENTS_INTERACTION_LE_SCAN_END: {
            bt_status = app_dongle_cm_le_stop_scan_device();
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
            bt_status = app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_AUDIO | APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD | APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
#endif
            APPS_LOG_MSGID_I(LOG_TAG" timeout, stop scan, bt_ret = 0x%x", 1, bt_status);
            ret = true;
            break;
        }
        default:
            break;
    }

    return ret;
}

void app_dongle_le_race_set_current_sink_device(app_dongle_le_race_sink_device_t sink_device)
{
    APPS_LOG_MSGID_I(LOG_TAG" set current sink device : %d", 1, sink_device);
    g_le_race_ctx.current_sink_device = sink_device;
}

void app_dongle_le_race_init(void)
{
    APPS_LOG_MSGID_I(LOG_TAG" app_dongle_le_race_init", 0);
    //memset(&g_le_race_ctx, 0, sizeof (app_dongle_le_race_contex_t));
    //g_le_race_ctx.current_sink_device = APP_DONGLE_LE_RACE_SINK_DEVICE_LEA;
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    app_le_audio_ucst_register_callback(app_dongle_le_race_event_callback);
#endif
    app_dongle_cm_le_register_race_callback(app_dongle_le_race_event_callback);
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    app_ull_dongle_le_register_race_callback(app_dongle_le_race_event_callback);
#endif

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    app_le_audio_race_init();
#endif

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    app_dongle_ull_le_hid_register_race_callback(app_dongle_le_race_event_callback);
#endif

}

#endif
