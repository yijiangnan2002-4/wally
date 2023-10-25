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
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_events_bt_event.h"
#include "ui_shell_manager.h"
#include "app_dongle_race.h"
#include "apps_dongle_sync_event.h"

#if (defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "bt_ull_le_service.h"
#include "app_ull_dongle_le.h"
#if (defined AIR_WIRELESS_MIC_ENABLE)
#include "apps_events_mic_control_event.h"
#endif
#endif

#define LOG_TAG        "[app_dongle_race]"

typedef struct {
    uint8_t channel;
    race_pkt_t race_pkt;
} PACKED app_dongle_race_event_ui_data_t;

typedef struct {
    uint8_t connect;
    bt_addr_t addr;
} PACKED app_dongle_race_connect_cmd_t;

typedef struct {
    uint8_t status;
    app_dongle_race_connect_cmd_t cmd;
} PACKED app_dongle_race_connect_rsp_t;

typedef struct {
    uint8_t status;
    uint8_t connected;
    bt_addr_t addr;
} PACKED app_ull_dongle_connect_notify_t;

typedef struct {
    uint8_t control_type;
    uint8_t target_value;
    bt_addr_t addr;
} PACKED app_dongle_race_control_remote_cmd_t;

typedef struct {
    uint8_t status;
    app_dongle_race_control_remote_cmd_t cmd;
} PACKED app_dongle_race_control_remote_rsp_t;

typedef struct {
    uint8_t status;
    uint8_t control_type;
    uint8_t current_value;
    bt_addr_t addr;
} PACKED app_dongle_race_control_remote_notify_t;

typedef struct {
    bt_addr_t address;
} PACKED get_remote_rssi_cmd_t;
typedef struct {
    uint8_t status;
    bt_addr_t address;
} PACKED get_remote_rssi_rsp_t;

typedef struct {
    uint8_t status;
    bt_addr_t address;
    int8_t rssi;
} PACKED get_remote_rssi_notify_t;

#if (defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) && (defined AIR_WIRELESS_MIC_ENABLE)

#define APP_ULL_DONGLE_RACE_CONNECT_EVENT_CONNECTED             (0x01)
#define APP_ULL_DONGLE_RACE_CONNECT_EVENT_DISCONNECTED          (0x00)
#define APP_ULL_DONGLE_RACE_GET_STATUS_NOT_CONNECTED_STATUS     (0xFF)
#define APP_ULL_DONGLE_RACE_NOTIFY_CHANNCEL                 RACE_SERIAL_PORT_TYPE_UART

RACE_ERRCODE app_dongle_race_send_race_notify(void *noti)
{
    RACE_ERRCODE race_notify_ret = RACE_ERRCODE_FAIL;
    if (noti != NULL) {
        race_notify_ret = race_noti_send(noti, APP_ULL_DONGLE_RACE_NOTIFY_CHANNCEL, false);
        if (race_notify_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)noti);
        }
    }
    return race_notify_ret;
}

void app_dongle_race_ull_connect_proc(bool connected, uint16_t conn_handle)
{
    RACE_ERRCODE race_notify_ret = RACE_ERRCODE_FAIL;
    app_ull_dongle_le_link_info_t *link_info = app_ull_dongle_le_get_link_info(conn_handle);
    app_ull_dongle_connect_notify_t *noti = NULL;
    if (link_info) {
        noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                (uint16_t)RACE_ULL_DONGLE_CONNECT,
                                (uint16_t)(sizeof(app_ull_dongle_connect_notify_t)),
                                APP_ULL_DONGLE_RACE_NOTIFY_CHANNCEL);
        if (noti != NULL) {
            noti->status = 0; /* Currently not useful. */
            noti->connected = APP_ULL_DONGLE_RACE_CONNECT_EVENT_DISCONNECTED;
            memcpy(&noti->addr, &link_info->addr, sizeof(bt_addr_t));
            if (connected) {
                noti->connected = APP_ULL_DONGLE_RACE_CONNECT_EVENT_CONNECTED;
            } else {
                noti->connected = APP_ULL_DONGLE_RACE_CONNECT_EVENT_DISCONNECTED;
            }
            app_dongle_race_send_race_notify(noti);
        }
    }

    APPS_LOG_MSGID_I(LOG_TAG"notify connected %x resule: 0x%X.", 2, connected, race_notify_ret);
}

void apps_dongle_race_cmd_on_remote_control_state_change(app_ull_dongle_race_remote_control_type_t control_type, uint8_t state, uint8_t channel_id)
{
    RACE_ERRCODE race_notify_ret = RACE_ERRCODE_FAIL;
    bt_bd_addr_t *addr = race_get_bt_connection_addr(channel_id);
    app_dongle_race_control_remote_notify_t *noti = NULL;
    if (addr) {
        noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                (uint16_t)RACE_DONGLE_CONTROL_REMOTE,
                                (uint16_t)(sizeof(app_dongle_race_control_remote_notify_t)),
                                APP_ULL_DONGLE_RACE_NOTIFY_CHANNCEL);
        if (noti != NULL) {
            noti->status = 0; /* Currently not useful. */
            noti->control_type = control_type;
            noti->current_value = state;
            memcpy(&noti->addr.addr, addr, sizeof(bt_bd_addr_t));
            noti->addr.type = BT_ADDR_PUBLIC;
            app_dongle_race_send_race_notify(noti);
        }
    }
    APPS_LOG_MSGID_I(LOG_TAG"notify control state(0x%x) to 0x%X change to addr %x resule: 0x%X.", 4, control_type, state, addr, race_notify_ret);
}

static bool app_ull2_dongle_race_cmd_event_proc(app_dongle_race_event_ui_data_t *race_data)
{
    bool ret = false;
    RACE_ERRCODE race_ret;
    bt_status_t bt_status = BT_STATUS_FAIL;
    void *rsp_msg = NULL;

    switch (race_data->race_pkt.hdr.id) {
        case RACE_ULL_DONGLE_CONNECT: {
            app_dongle_race_connect_cmd_t *race_conn_data = (app_dongle_race_connect_cmd_t *) & (race_data->race_pkt.payload);
            bt_addr_t *address = &race_conn_data->addr;
            if (race_conn_data->connect) {
                bt_status = app_ull_dongle_le_connect(address);
            } else {
                bt_status = app_ull_dongle_le_disconnect_device(address);
            }
            APPS_LOG_MSGID_I(LOG_TAG" app_ull2_dongle_race_cmd_event_proc, connect(%d), bt_ret = 0x%x",
                                2, race_conn_data->connect, bt_status);
            app_dongle_race_connect_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                        (uint16_t)RACE_ULL_DONGLE_CONNECT,
                                                                        (uint16_t)(sizeof(app_dongle_race_connect_rsp_t)),
                                                                        race_data->channel);
            if (response != NULL) {
                memcpy(&response->cmd, race_conn_data, sizeof(app_dongle_race_connect_cmd_t));
                response->status = (bt_status == BT_STATUS_SUCCESS) ? 0 : 0xFF;
                rsp_msg = response;
            }
            ret = true;
            break;
        }
        case RACE_DONGLE_CONTROL_REMOTE: {
            app_dongle_race_control_remote_cmd_t *remote_control = (app_dongle_race_control_remote_cmd_t *) & (race_data->race_pkt.payload);
            uint32_t event_id = 0xFFFFFFFF;
            if (APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_RECORD == remote_control->control_type) {
                event_id = APPS_EVENTS_MIC_CONTROL_LOCAL_RECORDER;
            } else if (APP_ULL_DONGLE_RACE_REMOTE_CONTROL_TYPE_MUTE_MIC == remote_control->control_type) {
                event_id = APPS_EVENTS_MIC_CONTROL_MUTE;
            }
            if (event_id != 0xFFFFFFFF) {
                bt_status = apps_dongle_sync_event_send_extra_by_address(EVENT_GROUP_UI_SHELL_WIRELESS_MIC, event_id,
                                                            &remote_control->target_value, sizeof(remote_control->target_value),
                                                            &remote_control->addr.addr);
            }
            APPS_LOG_MSGID_I(LOG_TAG" app_ull2_dongle_race_cmd_event_proc, control remote(0x%x), value:0x%x, bt_ret = 0x%x",
                             3, remote_control->control_type, remote_control->target_value, bt_status);
            app_dongle_race_control_remote_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                        (uint16_t)RACE_DONGLE_CONTROL_REMOTE,
                                                                        (uint16_t)(sizeof(app_dongle_race_control_remote_rsp_t)),
                                                                        race_data->channel);
            if (response) {
                memcpy(&response->cmd, remote_control, sizeof(app_dongle_race_control_remote_cmd_t));
                response->status = (bt_status == BT_STATUS_SUCCESS) ? 0 : 0xFF;
                rsp_msg = response;
            }
            break;
        }
        default:
            break;
    }

    if (rsp_msg != NULL) {
        race_ret = race_noti_send(rsp_msg, race_data->channel, false);
        APPS_LOG_MSGID_I(LOG_TAG"send rsp of event: 0x%x resule: 0x%X.", 2, race_data->race_pkt.hdr.id, race_ret);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)rsp_msg);
        }
    }

    return ret;
}

#endif

bool app_dongle_race_bt_event_proc(uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
#if (defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) && (defined AIR_WIRELESS_MIC_ENABLE)
    if (BT_GAP_LE_READ_RSSI_CNF == event_id && extra_data) {
        APPS_LOG_MSGID_I(LOG_TAG"BT_GAP_LE_READ_RSSI_CNF, extra_data = %x", 1, extra_data);
        apps_bt_event_data_t *bt_event_data = (apps_bt_event_data_t *)extra_data;
        if (bt_event_data->buffer) {
            
            RACE_ERRCODE race_notify_ret = RACE_ERRCODE_FAIL;
            bt_hci_evt_cc_read_rssi_t *read_rssi_result = (bt_hci_evt_cc_read_rssi_t *)(bt_event_data->buffer);
            APPS_LOG_MSGID_I(LOG_TAG"BT_GAP_LE_READ_RSSI_CNF, buffer = %x, handle = %d", 2, bt_event_data->buffer, read_rssi_result->handle);
            bt_addr_t *addr = app_ull_dongle_le_get_bt_addr_by_conn_handle(read_rssi_result->handle);
            if (addr) {
                get_remote_rssi_notify_t *noti;
                noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                    (uint16_t)RACE_DONGLE_GET_REMOTE_RSSI,
                                    (uint16_t)(sizeof(get_remote_rssi_notify_t)),
                                    APP_ULL_DONGLE_RACE_NOTIFY_CHANNCEL);
                if (noti != NULL) {
                    noti->status = BT_HCI_STATUS_SUCCESS == read_rssi_result->status ? 0 : 0xFF; /* Currently not useful. */
                    memcpy(&noti->address, addr, sizeof(bt_addr_t));
                    noti->rssi = read_rssi_result->rssi;
                    race_notify_ret = app_dongle_race_send_race_notify(noti);
                }
            }
            APPS_LOG_MSGID_I(LOG_TAG"notify rssi= %d resule: 0x%X.", 2, read_rssi_result->rssi, race_notify_ret);
        }
    }
#endif
    return ret;
}

bool app_dongle_race_cmd_event_proc(uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

#if (defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) && (defined AIR_WIRELESS_MIC_ENABLE)
    if (extra_data && data_len >= sizeof(app_dongle_race_event_ui_data_t)) {
        app_dongle_race_event_ui_data_t *race_data = (app_dongle_race_event_ui_data_t *)extra_data;
        if (race_data->race_pkt.hdr.id >= RACE_ULL_DONGLE_CONNECT && race_data->race_pkt.hdr.id <= RACE_DONGLE_CONTROL_REMOTE) {
            ret = app_ull2_dongle_race_cmd_event_proc(race_data);
        }
    }
#endif
    return ret;
}

void *apps_dongle_race_cmd_handler(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id)
{
    void *response = NULL;
    if (p_race_package->hdr.id >= RACE_ULL_DONGLE_CONNECT && p_race_package->hdr.id <= RACE_DONGLE_CONTROL_REMOTE) {
        uint32_t data_len = sizeof(app_dongle_race_event_ui_data_t) + p_race_package->hdr.length - sizeof(p_race_package->hdr.id);
        app_dongle_race_event_ui_data_t *extra_data = (app_dongle_race_event_ui_data_t *)pvPortMalloc(data_len);
        if (extra_data) {
            memcpy(&extra_data->race_pkt, p_race_package, sizeof(race_pkt_t) + p_race_package->hdr.length - sizeof(p_race_package->hdr.id));
            extra_data->channel = channel_id;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_DONGLE_RACE,
                                0, (void *)extra_data, data_len, NULL, 0);
        }
    }
#if (defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) && (defined AIR_WIRELESS_MIC_ENABLE)
    else if (p_race_package->hdr.id >= RACE_DONGLE_GET_REMOTE_RSSI) {
        get_remote_rssi_cmd_t *cmd = (get_remote_rssi_cmd_t *)&p_race_package->payload;
        get_remote_rssi_rsp_t *get_rssi_rsp = RACE_ClaimPacketAppID(p_race_package->hdr.pktId.field.app_id,
                                        RACE_TYPE_RESPONSE,
                                        RACE_DONGLE_GET_REMOTE_RSSI,
                                        sizeof(get_remote_rssi_rsp_t),
                                        channel_id);
        if (get_rssi_rsp) {
            uint16_t handle = app_ull_dongle_le_get_conn_handle_by_addr(&cmd->address);
            memcpy(&get_rssi_rsp->address, &cmd->address, sizeof(bt_addr_t));
            if (BT_HANDLE_INVALID != handle) {
                bt_hci_cmd_read_rssi_t read_rssi = {
                    .handle = handle,
                };
                if (BT_STATUS_SUCCESS == bt_gap_le_read_rssi(&read_rssi)) {
                    get_rssi_rsp->status = 0;
                } else {
                    get_rssi_rsp->status = 0xFF;
                }
            } else {
                get_rssi_rsp->status = 0xFE;
            }
            response = get_rssi_rsp;
            APPS_LOG_MSGID_I(LOG_TAG"RACE_DONGLE_GET_REMOTE_RSSI, handle = %d", 1, handle);
        }
    }
#endif
    return response;
}

#endif