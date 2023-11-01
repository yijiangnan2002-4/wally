
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

/**
 * File: app_bt_source_conn_mgr.c
 *
 * Description: This file provides API for BT Source Scan/Connection.
 *
 */

#ifdef AIR_BT_SOURCE_ENABLE

#include "app_bt_source_conn_mgr.h"

#include "apps_debug.h"
#include "app_dongle_connection_common.h"
#include "apps_events_bt_event.h"
#include "apps_events_event_group.h"
#include "bt_callback_manager.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager.h"
#include "bt_device_manager_internal.h"
#include "bt_gap.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "ui_shell_manager.h"

#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
#include "app_dongle_le_race.h"
#endif



#define LOG_TAG             "[BT_SRC][CONN]"

#define APP_BT_SOURCE_IS_EMPTY_ADDR(X, EMPTY)        (memcmp((X), (EMPTY), BT_BD_ADDR_LEN) == 0)
#define APP_BT_SOURCE_MIN_RSSI                       (-127)
#define APP_BT_SOURCE_INVALID_RSSI                   (128)

#define APP_BT_SOURCE_SCAN_TIME                      (0x30)         // Time = N * 1.28sec.
#define APP_BT_SOURCE_SCAN_MAX_NUM                   10

#define APP_BT_SOURCE_CONN_MAX_NUM                   1
#define APP_BT_SOURCE_CONN_INVAILD_INDEX             0xFF

#define APP_BT_SOURCE_BOND_MAX_NUM                   4

typedef enum {
    APP_BT_SOURCE_CONN_STATE_IDLE                    = 0,
    APP_BT_SOURCE_CONN_STATE_SCANNING,
    APP_BT_SOURCE_CONN_STATE_CONNECTING,
    APP_BT_SOURCE_CONN_STATE_CONNECTED,
    APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT,
    APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT
} app_bt_source_conn_mgr_state_t;

typedef enum {
    APP_BT_SOURCE_CONN_EVENT_START_SCAN              = 0,
    APP_BT_SOURCE_CONN_EVENT_START_RECONNECT,
    APP_BT_SOURCE_CONN_EVENT_SCAN_READY,
    APP_BT_SOURCE_CONN_EVENT_CONNECTED,
    APP_BT_SOURCE_CONN_EVENT_LINK_LOST,
    APP_BT_SOURCE_CONN_EVENT_RECONNECT_TIMEOUT,
    APP_BT_SOURCE_CONN_EVENT_CONNECT_FAIL,
    APP_BT_SOURCE_CONN_EVENT_DISCONNECT,
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
    APP_BT_SOURCE_CONN_EVENT_START_CONNECT,
#endif
} app_bt_source_conn_mgr_event_t;

typedef void (*app_bt_source_conn_mgr_exit_action_t)(uint8_t index);
typedef void (*app_bt_source_conn_mgr_do_action_t)(uint8_t index);

typedef struct {
    app_bt_source_conn_mgr_state_t              cur_state;
    app_bt_source_conn_mgr_event_t              event;
    app_bt_source_conn_mgr_state_t              next_state;
    app_bt_source_conn_mgr_exit_action_t        exit_action;
    app_bt_source_conn_mgr_do_action_t          do_action;
} app_bt_source_conn_mgr_transform_t;

#if (APP_BT_SOURCE_SCAN_POLICY != APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
static void app_bt_source_conn_mgr_state_scanning_do_action(uint8_t index);
#endif
static void app_bt_source_conn_mgr_state_power_on_reconnect_do_action(uint8_t index);
static void app_bt_source_conn_mgr_state_connecting_do_action(uint8_t index);
static void app_bt_source_conn_mgr_state_connected_do_action(uint8_t index);
static void app_bt_source_conn_mgr_state_link_lost_reconnect_do_action(uint8_t index);
static void app_bt_source_conn_mgr_state_idle_do_action(uint8_t index);

static app_bt_source_conn_mgr_transform_t       app_bt_source_conn_mgr_transform_table[] = {
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
    {APP_BT_SOURCE_CONN_STATE_IDLE,         APP_BT_SOURCE_CONN_EVENT_START_CONNECT,     APP_BT_SOURCE_CONN_STATE_CONNECTING,              NULL,   app_bt_source_conn_mgr_state_connecting_do_action},
    {APP_BT_SOURCE_CONN_STATE_IDLE,         APP_BT_SOURCE_CONN_EVENT_START_RECONNECT,   APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT,      NULL,   app_bt_source_conn_mgr_state_power_on_reconnect_do_action},
    {APP_BT_SOURCE_CONN_STATE_IDLE,         APP_BT_SOURCE_CONN_EVENT_CONNECTED,         APP_BT_SOURCE_CONN_STATE_CONNECTED,               NULL,   app_bt_source_conn_mgr_state_connected_do_action},
    {APP_BT_SOURCE_CONN_STATE_CONNECTING,   APP_BT_SOURCE_CONN_EVENT_CONNECTED,         APP_BT_SOURCE_CONN_STATE_CONNECTED,               NULL,   app_bt_source_conn_mgr_state_connected_do_action},
    {APP_BT_SOURCE_CONN_STATE_CONNECTING,   APP_BT_SOURCE_CONN_EVENT_CONNECT_FAIL,      APP_BT_SOURCE_CONN_STATE_IDLE,                    NULL,   app_bt_source_conn_mgr_state_idle_do_action},
    {APP_BT_SOURCE_CONN_STATE_CONNECTED,    APP_BT_SOURCE_CONN_EVENT_LINK_LOST,         APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT,     NULL,   app_bt_source_conn_mgr_state_link_lost_reconnect_do_action},
    {APP_BT_SOURCE_CONN_STATE_CONNECTED,    APP_BT_SOURCE_CONN_EVENT_DISCONNECT,        APP_BT_SOURCE_CONN_STATE_IDLE,                    NULL,   app_bt_source_conn_mgr_state_idle_do_action},
    {APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT,    APP_BT_SOURCE_CONN_EVENT_CONNECTED,     APP_BT_SOURCE_CONN_STATE_CONNECTED,         NULL,   app_bt_source_conn_mgr_state_connected_do_action},
    {APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT,    APP_BT_SOURCE_CONN_EVENT_RECONNECT_TIMEOUT,     APP_BT_SOURCE_CONN_STATE_IDLE,      NULL,   app_bt_source_conn_mgr_state_idle_do_action},
    {APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT,    APP_BT_SOURCE_CONN_EVENT_START_CONNECT,     APP_BT_SOURCE_CONN_STATE_CONNECTING,    NULL,   app_bt_source_conn_mgr_state_connecting_do_action},
    {APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT,     APP_BT_SOURCE_CONN_EVENT_CONNECTED,     APP_BT_SOURCE_CONN_STATE_CONNECTED,         NULL,   app_bt_source_conn_mgr_state_connected_do_action},
    {APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT,     APP_BT_SOURCE_CONN_EVENT_RECONNECT_TIMEOUT,     APP_BT_SOURCE_CONN_STATE_IDLE,      NULL,   app_bt_source_conn_mgr_state_idle_do_action},
    {APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT,     APP_BT_SOURCE_CONN_EVENT_START_CONNECT,     APP_BT_SOURCE_CONN_STATE_CONNECTING,    NULL,   app_bt_source_conn_mgr_state_connecting_do_action},
#else
    {APP_BT_SOURCE_CONN_STATE_IDLE,         APP_BT_SOURCE_CONN_EVENT_START_SCAN,        APP_BT_SOURCE_CONN_STATE_SCANNING,                NULL,   app_bt_source_conn_mgr_state_scanning_do_action},
    {APP_BT_SOURCE_CONN_STATE_IDLE,         APP_BT_SOURCE_CONN_EVENT_START_RECONNECT,   APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT,      NULL,   app_bt_source_conn_mgr_state_power_on_reconnect_do_action},
    {APP_BT_SOURCE_CONN_STATE_IDLE,         APP_BT_SOURCE_CONN_EVENT_CONNECTED,         APP_BT_SOURCE_CONN_STATE_CONNECTED,               NULL,   app_bt_source_conn_mgr_state_connected_do_action},
    {APP_BT_SOURCE_CONN_STATE_SCANNING,     APP_BT_SOURCE_CONN_EVENT_SCAN_READY,        APP_BT_SOURCE_CONN_STATE_CONNECTING,              NULL,   app_bt_source_conn_mgr_state_connecting_do_action},
    {APP_BT_SOURCE_CONN_STATE_SCANNING,     APP_BT_SOURCE_CONN_EVENT_CONNECTED,         APP_BT_SOURCE_CONN_STATE_CONNECTED,               NULL,   app_bt_source_conn_mgr_state_connected_do_action},
    {APP_BT_SOURCE_CONN_STATE_CONNECTING,   APP_BT_SOURCE_CONN_EVENT_CONNECTED,         APP_BT_SOURCE_CONN_STATE_CONNECTED,               NULL,   app_bt_source_conn_mgr_state_connected_do_action},
    {APP_BT_SOURCE_CONN_STATE_CONNECTING,   APP_BT_SOURCE_CONN_EVENT_CONNECT_FAIL,      APP_BT_SOURCE_CONN_STATE_SCANNING,                NULL,   app_bt_source_conn_mgr_state_scanning_do_action},
    {APP_BT_SOURCE_CONN_STATE_CONNECTED,    APP_BT_SOURCE_CONN_EVENT_DISCONNECT,        APP_BT_SOURCE_CONN_STATE_IDLE,                    NULL,   app_bt_source_conn_mgr_state_idle_do_action},
    {APP_BT_SOURCE_CONN_STATE_CONNECTED,    APP_BT_SOURCE_CONN_EVENT_LINK_LOST,         APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT,     NULL,   app_bt_source_conn_mgr_state_link_lost_reconnect_do_action},
    {APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT,    APP_BT_SOURCE_CONN_EVENT_CONNECTED,     APP_BT_SOURCE_CONN_STATE_CONNECTED,         NULL,   app_bt_source_conn_mgr_state_connected_do_action},
    {APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT,    APP_BT_SOURCE_CONN_EVENT_RECONNECT_TIMEOUT,     APP_BT_SOURCE_CONN_STATE_IDLE,      NULL,   NULL},
    {APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT,     APP_BT_SOURCE_CONN_EVENT_CONNECTED,     APP_BT_SOURCE_CONN_STATE_CONNECTED,         NULL,   app_bt_source_conn_mgr_state_connected_do_action},
    {APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT,     APP_BT_SOURCE_CONN_EVENT_RECONNECT_TIMEOUT,     APP_BT_SOURCE_CONN_STATE_SCANNING,  NULL,   app_bt_source_conn_mgr_state_scanning_do_action},
#endif
};

#define APP_BT_SOURCE_CONN_MGR_TRANSFORM_TABLE_SIZE  (sizeof(app_bt_source_conn_mgr_transform_table)/sizeof(app_bt_source_conn_mgr_transform_t))

typedef struct {
    app_bt_source_conn_mgr_state_t              pre_state;
    app_bt_source_conn_mgr_state_t              state;
    bt_bd_addr_t                                addr;
} PACKED app_bt_source_conn_device_t;

#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)

#define APP_BT_SOURCE_READ_NAME
#define APP_BT_SOURCE_READ_NAME_TIMER                 (5 * 1000)

typedef enum {
    APP_BT_SOURCE_SCAN_STATE_NONE                    = 0,
    APP_BT_SOURCE_SCAN_STATE_NEED_READ,
    APP_BT_SOURCE_SCAN_STATE_READING,
    APP_BT_SOURCE_SCAN_STATE_FOUND,
    APP_BT_SOURCE_SCAN_STATE_SEND
} app_bt_source_conn_mgr_scan_state_t;

typedef struct {
    uint8_t                                     scan_state;
    uint8_t                                     addr[BT_BD_ADDR_LEN];
    uint8_t                                     rssi;
    char                                        name[BT_GAP_MAX_DEVICE_NAME_LENGTH];
} PACKED app_bt_source_scan_item_t;

static app_bt_source_scan_item_t                app_bt_source_scan_list[APP_BT_SOURCE_SCAN_TOOL_MAX_NUM];

#endif


typedef struct {
    app_bt_source_conn_device_t                 device[APP_BT_SOURCE_CONN_MAX_NUM];

    bool                                        disabled;

    uint8_t                                     scan_num;
    int8_t                                      scan_rssi;
    bt_bd_addr_t                                scan_addr;      // selected_addr

    bool                                        is_bonded;
    bt_bd_addr_t                                bond_addr[APP_BT_SOURCE_BOND_MAX_NUM];
    uint8_t                                     active_index;
} PACKED app_bt_source_conn_mgr_context_t;

static app_bt_source_conn_mgr_context_t         app_bt_source_conn_mgr_ctx = {0};



/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
static void app_bt_source_conn_mgr_print_bonded_device()
{
    uint8_t index = 0;
    for (index = 0; index < APP_BT_SOURCE_BOND_MAX_NUM; index ++) {
        APPS_LOG_MSGID_W(LOG_TAG" bonded_device[%d] = 0x%02X:%02X:%02X:%02X:%02X:%02X",
                            7,
                            index,
                            app_bt_source_conn_mgr_ctx.bond_addr[index][0],
                            app_bt_source_conn_mgr_ctx.bond_addr[index][1],
                            app_bt_source_conn_mgr_ctx.bond_addr[index][2],
                            app_bt_source_conn_mgr_ctx.bond_addr[index][3],
                            app_bt_source_conn_mgr_ctx.bond_addr[index][4],
                            app_bt_source_conn_mgr_ctx.bond_addr[index][5]);
    }
}

static void app_bt_source_conn_mgr_notify_dongle_cm(bool connected)
{
#ifndef APP_BT_SOURCE_ONLY_MODE
    app_dongle_cm_event_t event = (connected ? APP_DONGLE_CM_EVENT_SOURCE_STARTED : APP_DONGLE_CM_EVENT_SOURCE_END);
    bt_status_t bt_status = app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_BTA, event, BT_STATUS_SUCCESS, NULL);
    if (bt_status != BT_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(LOG_TAG" notify_dongle_cm, error connected=%d bt_status=0x%08X",
                         2, connected, bt_status);
    }
#endif
}

#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)

#define APP_BT_SOURCE_EIR_MAX_LENGTH            240
#define APP_BT_SOURCE_EIR_SHORT_NAME_TAG        0x08
#define APP_BT_SOURCE_EIR_NAME_TAG              0x09

static void app_bt_source_conn_mgr_get_name(const uint8_t *eir, uint8_t **name, uint8_t *name_len)
{
    int index = 0;
    uint8_t length = 0;
    uint8_t tag = 0;
    uint8_t *value = NULL;

    while (index < APP_BT_SOURCE_EIR_MAX_LENGTH - 1) {
        length = eir[index++];
        tag = eir[index++];
        value = (uint8_t *)(eir + index);

        if (tag == APP_BT_SOURCE_EIR_SHORT_NAME_TAG || tag == APP_BT_SOURCE_EIR_NAME_TAG) {
            *name = value;
            *name_len = length - 1;
            break;
        } else {
            index += length - 1;
        }
    }
}

#ifdef APP_BT_SOURCE_READ_NAME
static void app_bt_source_conn_mgr_check_read_name(void)
{
    for (int i = 0; i < APP_BT_SOURCE_SCAN_TOOL_MAX_NUM; i++) {
        app_bt_source_scan_item_t *item = &app_bt_source_scan_list[i];
        if (item->scan_state == APP_BT_SOURCE_SCAN_STATE_NEED_READ) {
            APPS_LOG_MSGID_I(LOG_TAG" check_read_name, need read_name again", 0);
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_READ_NAME);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_READ_NAME,
                                NULL, 0, NULL, 0);
            break;
        }
    }
}

static bool app_bt_source_conn_mgr_is_reading_name(void)
{
    bool ret = FALSE;
    for (int i = 0; i < APP_BT_SOURCE_SCAN_TOOL_MAX_NUM; i++) {
        app_bt_source_scan_item_t *item = &app_bt_source_scan_list[i];
        if (item->scan_state == APP_BT_SOURCE_SCAN_STATE_READING) {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

static void app_bt_source_conn_mgr_cancel_reading(void)
{
    for (int i = 0; i < APP_BT_SOURCE_SCAN_TOOL_MAX_NUM; i++) {
        app_bt_source_scan_item_t *item = &app_bt_source_scan_list[i];
        if (item->scan_state == APP_BT_SOURCE_SCAN_STATE_NEED_READ
            || item->scan_state == APP_BT_SOURCE_SCAN_STATE_READING) {
            const uint8_t *addr = item->addr;
            if (item->scan_state == APP_BT_SOURCE_SCAN_STATE_READING) {
                bt_status_t bt_status = bt_gap_cancel_name_request((const bt_bd_addr_t *)addr);
                APPS_LOG_MSGID_W(LOG_TAG" cancel_reading, bt_status=0x%08X", 1, bt_status);
            }
            APPS_LOG_MSGID_W(LOG_TAG" cancel_reading, addr=%02X:%02X:%02X:%02X:%02X:%02X",
                             6, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
#ifdef MTK_RACE_CMD_ENABLE
            app_dongle_le_race_reply_scan_report(APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC, BT_ADDR_PUBLIC,
                                                 item->addr, item->rssi, item->name, strlen(item->name));
#endif
            item->scan_state = APP_BT_SOURCE_SCAN_STATE_SEND;
        }
    }
}
#endif

static void app_bt_source_conn_mgr_update_scan_list(const uint8_t *addr, uint8_t rssi, const uint8_t *eir)
{
    bool exist_name = FALSE;
    uint8_t *name = NULL;
    uint8_t name_len = 0;
    if (eir != NULL) {
        app_bt_source_conn_mgr_get_name(eir, &name, &name_len);
        if (name != NULL && name_len > 0) {
            exist_name = TRUE;
        }
    }

    bool is_exist = FALSE;
    for (int i = 0; i < APP_BT_SOURCE_SCAN_TOOL_MAX_NUM; i++) {
        app_bt_source_scan_item_t *item = &app_bt_source_scan_list[i];
        if (memcmp(item->addr, addr, BT_BD_ADDR_LEN) == 0) {
            is_exist = TRUE;
            item->rssi = rssi;

            if (exist_name && strlen(item->name) == 0) {
                APPS_LOG_MSGID_W(LOG_TAG" update_scan_list, UPDATE addr=%02X:%02X:%02X:%02X:%02X:XX name_len=%d",
                                 7, addr[5], addr[4], addr[3], addr[2], addr[1], name_len);
                if (item->scan_state == APP_BT_SOURCE_SCAN_STATE_READING) {
                    bt_status_t bt_status = bt_gap_cancel_name_request((const bt_bd_addr_t *)addr);
                    APPS_LOG_MSGID_W(LOG_TAG" update_scan_list, UPDATE cancel bt_status=0x%08X", 1, bt_status);
                    app_bt_source_conn_mgr_check_read_name();
                }

                memset(item->name, 0, BT_GAP_MAX_DEVICE_NAME_LENGTH);
                memcpy(item->name, name, (name_len >= BT_GAP_MAX_DEVICE_NAME_LENGTH ? BT_GAP_MAX_DEVICE_NAME_LENGTH - 1 : name_len));
                item->scan_state = APP_BT_SOURCE_SCAN_STATE_FOUND;
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_SCANNED);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_SCANNED,
                                    NULL, 0, NULL, 0);
            }
            break;
        }
    }

    if (!is_exist) {
        bool need_read = FALSE;
        bool found = FALSE;
        for (int i = 0; i < APP_BT_SOURCE_SCAN_TOOL_MAX_NUM; i++) {
            app_bt_source_scan_item_t *item = &app_bt_source_scan_list[i];
            if (item->scan_state == APP_BT_SOURCE_SCAN_STATE_NONE) {
#ifdef APP_BT_SOURCE_READ_NAME
                item->scan_state = APP_BT_SOURCE_SCAN_STATE_NEED_READ;
#else
                item->scan_state = APP_BT_SOURCE_SCAN_STATE_FOUND;
#endif
                memcpy(item->addr, addr, BT_BD_ADDR_LEN);
                item->rssi = rssi;
                memset(item->name, 0, BT_GAP_MAX_DEVICE_NAME_LENGTH);

                if (exist_name) {
                    memcpy(item->name, name, (name_len >= BT_GAP_MAX_DEVICE_NAME_LENGTH ? BT_GAP_MAX_DEVICE_NAME_LENGTH - 1 : name_len));
                    item->scan_state = APP_BT_SOURCE_SCAN_STATE_FOUND;
                    found = TRUE;
                }

                if (item->scan_state == APP_BT_SOURCE_SCAN_STATE_NEED_READ) {
                    need_read = TRUE;
                }

                APPS_LOG_MSGID_I(LOG_TAG" update_scan_list, addr=%02X:%02X:%02X:%02X:%02X:XX name_len=%d found=%d need_read=%d",
                                 8, addr[5], addr[4], addr[3], addr[2], addr[1], name_len, found, need_read);
#ifndef APP_BT_SOURCE_READ_NAME
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_SCANNED);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_SCANNED,
                                    NULL, 0, NULL, 0);
#endif
                break;
            }
        }

#ifdef APP_BT_SOURCE_READ_NAME
        if (found) {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_SCANNED);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_SCANNED,
                                NULL, 0, NULL, 0);
        }
        if (need_read) {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_READ_NAME);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_READ_NAME,
                                NULL, 0, NULL, 0);
        }
#endif
    }
}
#endif

static bool app_bt_source_conn_mgr_check_empty_addr(const uint8_t *addr)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    return (memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0);
}

static uint8_t app_bt_source_conn_mgr_get_unused_index(void)
{
    uint8_t index = APP_BT_SOURCE_CONN_INVAILD_INDEX;
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    for (int i = 0; i < APP_BT_SOURCE_CONN_MAX_NUM; i++) {
        if (memcmp(app_bt_source_conn_mgr_ctx.device[i].addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
            index = i;
            break;
        }
    }

    return index;
}

static uint8_t app_bt_source_conn_mgr_get_index_by_addr(const uint8_t *addr)
{
    uint8_t index = APP_BT_SOURCE_CONN_INVAILD_INDEX;
    if (app_bt_source_conn_mgr_check_empty_addr(addr)) {
        return index;
    }

    for (int i = 0; i < APP_BT_SOURCE_CONN_MAX_NUM; i++) {
        if (memcmp(app_bt_source_conn_mgr_ctx.device[i].addr, addr, BT_BD_ADDR_LEN) == 0) {
            index = i;
            break;
        }
    }

    return index;
}

static uint8_t app_bt_source_conn_mgr_find_and_update_index_by_addr(const uint8_t *addr)
{
    uint8_t index = app_bt_source_conn_mgr_get_index_by_addr(addr);

    if (index == APP_BT_SOURCE_CONN_INVAILD_INDEX) {
        APPS_LOG_MSGID_I(LOG_TAG" find_and_update_index, not conn_device addr", 0);

        if (bt_device_manager_is_paired(addr)) {
            index = app_bt_source_conn_mgr_get_unused_index();
            APPS_LOG_MSGID_I(LOG_TAG" find_and_update_index, find unused_index[%d]", 1, index);

            if (index != APP_BT_SOURCE_CONN_INVAILD_INDEX) {
                memset(&app_bt_source_conn_mgr_ctx.device[index], 0, sizeof(app_bt_source_conn_device_t));
                memcpy(app_bt_source_conn_mgr_ctx.device[index].addr, addr, BT_BD_ADDR_LEN);
            } else {
                for (int i = 0; i < APP_BT_SOURCE_CONN_MAX_NUM; i++) {
                    if (app_bt_source_conn_mgr_ctx.device[i].state != APP_BT_SOURCE_CONN_STATE_CONNECTING
                        && app_bt_source_conn_mgr_ctx.device[i].state != APP_BT_SOURCE_CONN_STATE_CONNECTED) {
                        memset(&app_bt_source_conn_mgr_ctx.device[i], 0, sizeof(app_bt_source_conn_device_t));
                        memcpy(app_bt_source_conn_mgr_ctx.device[i].addr, addr, BT_BD_ADDR_LEN);
                        index = i;
                        break;
                    }
                }
            }
        }
    }

    return index;
}

static uint8_t app_bt_source_conn_mgr_get_connected_num(void)
{
    uint8_t num = 0;
    for (int i = 0; i < APP_BT_SOURCE_CONN_MAX_NUM; i++) {
        if (app_bt_source_conn_mgr_ctx.device[i].state == APP_BT_SOURCE_CONN_STATE_CONNECTED) {
            num++;
        }
    }
    return num;
}

static void app_bt_source_conn_mgr_notify_conn_event(bool connect, uint8_t *addr)
{
    uint8_t *addr_param = (uint8_t *)pvPortMalloc(BT_BD_ADDR_LEN);
    if (addr_param == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" notify_conn_event, malloc error", 0);
        return;
    }

    memcpy(addr_param, addr, BT_BD_ADDR_LEN);

    if (connect) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_NOTIFY_CONN_CONNECTED);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                            EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_NOTIFY_CONN_CONNECTED,
                            (void *)addr_param, BT_BD_ADDR_LEN, NULL, 0);
    } else {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_NOTIFY_CONN_DISCONNECTED);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                            EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_NOTIFY_CONN_DISCONNECTED,
                            (void *)addr_param, BT_BD_ADDR_LEN, NULL, 0);
    }
}

static void app_bt_source_conn_mgr_enable_page_scan(bool enable)
{
    if (app_bt_source_conn_mgr_ctx.disabled) {
        APPS_LOG_MSGID_E(LOG_TAG" enable_page_scan, disabled", 0);
        return;
    }

    bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW,
                          (enable ? BT_CM_COMMON_TYPE_ENABLE : BT_CM_COMMON_TYPE_DISABLE));
    APPS_LOG_MSGID_I(LOG_TAG" enable_page_scan, enable=%d", 1, enable);
}

static bool app_bt_source_conn_mgr_enable_scan(bool enable)
{
    bool success = FALSE;
#if (APP_BT_SOURCE_SCAN_POLICY != APP_BT_SOURCE_SCAN_POLICY_BY_AT_CMD)
    bt_status_t bt_status = BT_STATUS_SUCCESS;
    if (enable) {
        bt_status = bt_gap_inquiry(APP_BT_SOURCE_SCAN_TIME, APP_BT_SOURCE_SCAN_MAX_NUM);
    } else {
        bt_status = bt_gap_cancel_inquiry();
    }
    APPS_LOG_MSGID_I(LOG_TAG" enable_scan, enable=%d bt_stauts=0x%08X", 2, enable, bt_status);
    success = (bt_status == BT_STATUS_SUCCESS);
#endif
    return success;
}

#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
static void app_bt_source_conn_mgr_disconnect_addr(uint8_t *addr)
{
    bt_cm_connect_t connect_param = {{0}, BT_CM_PROFILE_SERVICE_MASK_ALL};
    memcpy(connect_param.address, addr, sizeof(bt_bd_addr_t));
    bt_status_t bt_status = bt_cm_disconnect(&connect_param);
    APPS_LOG_MSGID_I(LOG_TAG" disconnect_addr, addr=%02X:%02X:%02X:%02X:%02X:%02X bt_stauts=0x%08X",
                     7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status);
}
#endif

static void app_bt_source_conn_mgr_disconnect(uint8_t index)
{
    uint8_t *addr = app_bt_source_conn_mgr_ctx.device[index].addr;
    bt_cm_connect_t connect_param = {{0}, BT_CM_PROFILE_SERVICE_MASK_ALL};
    memcpy(connect_param.address, addr, sizeof(bt_bd_addr_t));
    bt_status_t bt_status = bt_cm_disconnect(&connect_param);
    APPS_LOG_MSGID_I(LOG_TAG" disconnect, [%d] addr=%02X:%02X:%02X:%02X:%02X:%02X bt_status=0x%08X",
                     8, index, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status);
    // Re-initiate connection request after EDR fail with BT_HCI_STATUS_PIN_OR_KEY_MISSING reason
    // No need the below workaround (BTA-40287)
//    if (bt_status == BT_STATUS_SUCCESS) {
//        app_bt_source_conn_mgr_remove_record(addr);
//    }
}

static bool app_bt_source_conn_mgr_connect(const uint8_t *addr)
{
    bt_cm_connect_t connect_param = {{0}, BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP_AG)
                                        | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SOURCE)};
    memcpy(connect_param.address, addr, sizeof(bt_bd_addr_t));
    bt_status_t bt_status = bt_cm_connect(&connect_param);
    APPS_LOG_MSGID_I(LOG_TAG" connect, addr=%02X:%02X:%02X:%02X:%02X:%02X bt_stauts=0x%08X",
                     7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status);
    return (bt_status == BT_STATUS_SUCCESS);
}

static uint8_t app_bt_source_conn_mgr_cancel_connect(void)
{
    uint8_t cancel_num = 0;
    bt_bd_addr_t bd_addr[2];
    uint32_t connecting_number = 2;

    connecting_number = bt_cm_get_connecting_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                                                     bd_addr, connecting_number);
    bt_cm_connect_t connect_param = {{0}, ~(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))};
    for (int i = 0; i < connecting_number; i++) {
        memcpy(connect_param.address, bd_addr[i], sizeof(bt_bd_addr_t));
        bt_status_t bt_status = bt_cm_disconnect(&connect_param);
        uint8_t *addr = (uint8_t *)bd_addr[i];
        APPS_LOG_MSGID_I(LOG_TAG" cancel_connect, addr=%02X:%02X:%02X:%02X:%02X:%02X bt_status=0x%08X",
                         7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status);
        cancel_num++;
    }
    return cancel_num;
}



/**================================================================================*/
/**                                   State Machine                                */
/**================================================================================*/
#if (APP_BT_SOURCE_SCAN_POLICY != APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
static void app_bt_source_conn_mgr_state_scanning_do_action(uint8_t index)
{
    APPS_LOG_MSGID_I(LOG_TAG" state_machine [SCANNING][%d], do_action", 1, index);
    app_bt_source_conn_mgr_enable_scan(TRUE);
}
#endif

static void app_bt_source_conn_mgr_state_power_on_reconnect_do_action(uint8_t index)
{
    APPS_LOG_MSGID_I(LOG_TAG" state_machine [POWER_ON_REONNECT][%d], do_action", 1, index);
    app_bt_source_conn_mgr_connect(app_bt_source_conn_mgr_ctx.device[index].addr);

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT + index);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT + index,
                        NULL, 0, NULL, APP_BT_SOURCE_POWER_ON_RECONNECT_TIME);
}

static void app_bt_source_conn_mgr_state_connecting_do_action(uint8_t index)
{
    APPS_LOG_MSGID_I(LOG_TAG" state_machine [CONNECTING][%d], do_action", 1, index);
    memcpy(app_bt_source_conn_mgr_ctx.device[index].addr,
           app_bt_source_conn_mgr_ctx.scan_addr, BT_BD_ADDR_LEN);
    app_bt_source_conn_mgr_connect(app_bt_source_conn_mgr_ctx.scan_addr);
    memset(app_bt_source_conn_mgr_ctx.scan_addr, 0, BT_BD_ADDR_LEN);
}

static void app_bt_source_conn_mgr_state_connected_do_action(uint8_t index)
{
    APPS_LOG_MSGID_I(LOG_TAG" state_machine [CONNECTED][%d], do_action", 1, index);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT + index);

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CHECK_RSSI_TIMEOUT);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CONNECT_DEVICE);
    app_bt_source_conn_mgr_enable_scan(FALSE);

    app_bt_source_conn_mgr_ctx.is_bonded = TRUE;

    // Cancel connecting and disable page_scan when connected
    if (app_bt_source_conn_mgr_get_connected_num() == APP_BT_SOURCE_CONN_MAX_NUM) {
        app_bt_source_conn_mgr_cancel_connect();
        app_bt_source_conn_mgr_enable_page_scan(FALSE);
    }

    // Notify BT Source connection event to other APP
    app_bt_source_conn_mgr_notify_conn_event(TRUE, app_bt_source_conn_mgr_ctx.device[index].addr);

    // Shift bond_addr list and put latest item to first(index=0)
    if (memcmp(app_bt_source_conn_mgr_ctx.bond_addr[0], app_bt_source_conn_mgr_ctx.device[index].addr, BT_BD_ADDR_LEN) == 0) {
        return;
    }

    bool found = FALSE;
    uint8_t found_index = 0;
    for (int i = 0; i < APP_BT_SOURCE_BOND_MAX_NUM; i++) {
        if (memcmp(app_bt_source_conn_mgr_ctx.bond_addr[i], app_bt_source_conn_mgr_ctx.device[index].addr, BT_BD_ADDR_LEN) == 0) {
            found = TRUE;
            found_index = i;
            break;
        }
    }
    if (found && (found_index != APP_BT_SOURCE_BOND_MAX_NUM - 1)) {
        for (int i = (found_index - 1); i >= 0; i--) {
            memcpy(app_bt_source_conn_mgr_ctx.bond_addr[i + 1], app_bt_source_conn_mgr_ctx.bond_addr[i], BT_BD_ADDR_LEN);
        }
    } else {
        for (int i = (APP_BT_SOURCE_BOND_MAX_NUM - 1 - 1); i >= 0; i--) {
            memcpy(app_bt_source_conn_mgr_ctx.bond_addr[i + 1], app_bt_source_conn_mgr_ctx.bond_addr[i], BT_BD_ADDR_LEN);
        }
    }
    memcpy(app_bt_source_conn_mgr_ctx.bond_addr[0], app_bt_source_conn_mgr_ctx.device[index].addr, BT_BD_ADDR_LEN);

    app_bt_source_conn_mgr_print_bonded_device();

    // Saved bond_addr to NVKEY
    uint32_t size = BT_BD_ADDR_LEN * APP_BT_SOURCE_BOND_MAX_NUM;
    nvkey_status_t status = nvkey_write_data(NVID_APP_BT_SOURCE_DEVICE_INFO,
                                                (const uint8_t *)app_bt_source_conn_mgr_ctx.bond_addr,
                                                size);
    if (status != NVKEY_STATUS_OK) {
        APPS_LOG_MSGID_E(LOG_TAG" state_machine [CONNECTED][%d], save nvkey status=%d", 2, index, status);
    }
}

static void app_bt_source_conn_mgr_state_link_lost_reconnect_do_action(uint8_t index)
{
    APPS_LOG_MSGID_I(LOG_TAG" state_machine [LINK_LOST][%d], do_action", 1, index);
    // Note: No need to BT Source APP trigger reconnect, wait headset or BT CM middleware to reconnect
    // app_bt_source_conn_mgr_connect(app_bt_source_conn_mgr_ctx.device[index].addr);

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT + index);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT + index,
                        NULL, 0, NULL, APP_BT_SOURCE_LINK_LOST_RECONNECT_TIME);

    app_bt_source_conn_mgr_notify_conn_event(FALSE, app_bt_source_conn_mgr_ctx.device[index].addr);

    app_bt_source_conn_mgr_enable_page_scan(TRUE);
}

static void app_bt_source_conn_mgr_state_idle_do_action(uint8_t index)
{
    APPS_LOG_MSGID_I(LOG_TAG" state_machine [IDLE][%d], do_action", 1, index);
    if (app_bt_source_conn_mgr_ctx.device[index].pre_state == APP_BT_SOURCE_CONN_STATE_CONNECTED) {
        app_bt_source_conn_mgr_notify_conn_event(FALSE, app_bt_source_conn_mgr_ctx.device[index].addr);
        app_bt_source_conn_mgr_enable_page_scan(TRUE);
    } else if (app_bt_source_conn_mgr_ctx.device[index].pre_state == APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT
            || app_bt_source_conn_mgr_ctx.device[index].pre_state == APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT) {
        app_bt_source_conn_mgr_cancel_connect();
    }

    memset(&app_bt_source_conn_mgr_ctx.device[index], 0, sizeof(app_bt_source_conn_device_t));
}

static void app_bt_source_conn_mgr_state_machine_run(uint8_t index, uint8_t event)
{
    if (app_bt_source_conn_mgr_ctx.disabled && event != APP_BT_SOURCE_CONN_EVENT_LINK_LOST && event != APP_BT_SOURCE_CONN_EVENT_DISCONNECT) {
        APPS_LOG_MSGID_E(LOG_TAG" state_machine_run, [%d] DISABLE state - event=%d", 2, index, event);
        return;
    } else if (index >= APP_BT_SOURCE_CONN_MAX_NUM) {
        APPS_LOG_MSGID_E(LOG_TAG" state_machine_run, invalid index=%d", 1, index);
        return;
    }

    app_bt_source_conn_mgr_transform_t *transform = NULL;
    app_bt_source_conn_mgr_state_t state = app_bt_source_conn_mgr_ctx.device[index].state;
    for (int i = 0; i < APP_BT_SOURCE_CONN_MGR_TRANSFORM_TABLE_SIZE; i++) {
        app_bt_source_conn_mgr_transform_t *temp = &app_bt_source_conn_mgr_transform_table[i];
        if (temp->cur_state == state && temp->event == event) {
            transform = temp;
            break;
        }
    }

    if (transform == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" state_machine_run, [%d] invalid state=%d event=%d",
                         3, index, state, event);
        return;
    }

    app_bt_source_conn_mgr_state_t next_state = transform->next_state;
    APPS_LOG_MSGID_I(LOG_TAG" state_machine_run, [%d] state=%d->%d event=%d exit_action=0x%08X do_action=0x%08X",
                     6, index, state, next_state, event, transform->exit_action, transform->do_action);
    if (transform->exit_action != NULL) {
        transform->exit_action(index);
    }
    app_bt_source_conn_mgr_ctx.device[index].pre_state = app_bt_source_conn_mgr_ctx.device[index].state;
    app_bt_source_conn_mgr_ctx.device[index].state = next_state;
    if (transform->do_action != NULL) {
        transform->do_action(index);
    }
}



/**================================================================================*/
/**                                 APP Event Handler                              */
/**================================================================================*/
static bt_status_t app_bt_source_conn_mgr_bt_event_callback(bt_msg_type_t msg, bt_status_t status, void *buf)
{
    switch (msg) {
        case BT_GAP_INQUIRY_CNF: {
            APPS_LOG_MSGID_I(LOG_TAG" BT event callback, BT_GAP_INQUIRY_CNF status=0x%08X", 1, status);
            if (status == BT_STATUS_SUCCESS) {
                app_bt_source_conn_mgr_ctx.scan_num = 0;
                app_bt_source_conn_mgr_ctx.scan_rssi = APP_BT_SOURCE_MIN_RSSI;
                memset(app_bt_source_conn_mgr_ctx.scan_addr, 0, sizeof(bt_bd_addr_t));

#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_MAX_RSSI)
                uint8_t index = app_bt_source_conn_mgr_get_unused_index();
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CHECK_RSSI_TIMEOUT);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CHECK_RSSI_TIMEOUT,
                                    (void *)(uint32_t)index, 0, NULL, APP_BT_SOURCE_SCAN_RSSI_CHECK_TIME);
#endif
            }
            break;
        }

        case BT_GAP_CANCEL_INQUIRY_CNF: {
            APPS_LOG_MSGID_I(LOG_TAG" BT event callback, BT_GAP_CANCEL_INQUIRY_CNF status=0x%08X", 1, status);
            app_bt_source_conn_mgr_ctx.scan_num = 0;
            app_bt_source_conn_mgr_ctx.scan_rssi = APP_BT_SOURCE_MIN_RSSI;
            break;
        }

        case BT_GAP_INQUIRY_IND: {
            bt_gap_inquiry_ind_t* ind = (bt_gap_inquiry_ind_t*)buf;
            uint8_t *addr = (uint8_t *)ind->address;
            int8_t rssi = ((ind->rssi != NULL ? (*ind->rssi) : APP_BT_SOURCE_INVALID_RSSI));
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_FIRST_DEVICE) || (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_MAX_RSSI)
            bool cancel_scan = FALSE;
            bool send_device = FALSE;
#endif

            APPS_LOG_MSGID_I(LOG_TAG" BT event callback, BT_GAP_INQUIRY_IND rssi=%d addr=%02X:%02X:%02X:%02X:%02X:%02X eir=0x%08X",
                             8, rssi, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], ind->eir);
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
            app_bt_source_conn_mgr_update_scan_list(addr, rssi, ind->eir);
#endif

#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_FIRST_DEVICE)
            cancel_scan = TRUE;
            send_device = TRUE;
            memcpy(app_bt_source_conn_mgr_ctx.scan_addr, addr, BT_BD_ADDR_LEN);
#elif (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_MAX_RSSI)
            if (rssi < APP_BT_SOURCE_SCAN_RSSI_THRESHOLD) {
                APPS_LOG_MSGID_I(LOG_TAG" BT event callback, BT_GAP_INQUIRY_IND rssi lower than threshold %d",
                                 1, APP_BT_SOURCE_SCAN_RSSI_THRESHOLD);
                break;
            } else {
                if (rssi > app_bt_source_conn_mgr_ctx.scan_rssi) {
                    app_bt_source_conn_mgr_ctx.scan_rssi = rssi;
                    memcpy(app_bt_source_conn_mgr_ctx.scan_addr, addr, BT_BD_ADDR_LEN);
                }
                app_bt_source_conn_mgr_ctx.scan_num++;
                if (app_bt_source_conn_mgr_ctx.scan_num == APP_BT_SOURCE_SCAN_RSSI_MAX_NUM) {
                    cancel_scan = TRUE;
                    send_device = TRUE;
                }
            }
#endif
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_FIRST_DEVICE) || (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_MAX_RSSI)
            if (cancel_scan) {
                app_bt_source_conn_mgr_enable_scan(FALSE);
            }
            if (send_device) {
                uint8_t index = app_bt_source_conn_mgr_get_unused_index();
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CHECK_RSSI_TIMEOUT);
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CONNECT_DEVICE);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CONNECT_DEVICE,
                                    (void *)(uint32_t)index, 0, NULL, 0);
            }
#endif
            break;
        }

        case BT_GAP_READ_REMOTE_NAME_COMPLETE_IND: {
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
#ifdef APP_BT_SOURCE_READ_NAME
            bt_gap_read_remote_name_complete_ind_t *ind = (bt_gap_read_remote_name_complete_ind_t *)buf;
            const uint8_t *addr1 = (const uint8_t *)ind->address;
            uint8_t name_len = (ind->name != NULL ? strlen(ind->name) : 0);
            APPS_LOG_MSGID_I(LOG_TAG" BT_NAME_COMPLETE_IND, addr=%02X:%02X:%02X:%02X:%02X:%02X name_len=%d",
                             7, addr1[5], addr1[4], addr1[3], addr1[2], addr1[1], addr1[0], name_len);

            for (int i = 0; i < APP_BT_SOURCE_SCAN_TOOL_MAX_NUM; i++) {
                app_bt_source_scan_item_t *item = &app_bt_source_scan_list[i];
                if (memcmp(item->addr, ind->address, BT_BD_ADDR_LEN) == 0) {
                    uint8_t *addr = item->addr;
                    APPS_LOG_MSGID_I(LOG_TAG" BT_NAME_COMPLETE_IND, scan_state=%d addr=%02X:%02X:%02X:%02X:%02X:%02X name_len=%d",
                                     8, item->scan_state, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], name_len);

                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_READ_NAME_TIMEOUT);
                    if (name_len > 0 && item->scan_state == APP_BT_SOURCE_SCAN_STATE_READING && strlen(item->name) == 0) {
                        item->scan_state = APP_BT_SOURCE_SCAN_STATE_FOUND;
                        memcpy(item->name, ind->name, name_len);
                        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_SCANNED);
                        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_BT_SOURCE_APP,
                                            APP_BT_SOURCE_EVENT_TOOL_SCANNED, NULL, 0, NULL, 0);
                    }
                    app_bt_source_conn_mgr_check_read_name();
                    break;
                }
            }
#endif
#endif
            break;
        }

        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bool app_bt_source_conn_mgr_proc_bt_cm_group(uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (remote_update != NULL) {
                uint8_t *addr = (uint8_t *)remote_update->address;
                uint8_t index = APP_BT_SOURCE_CONN_INVAILD_INDEX;
                bt_status_t reason = remote_update->reason;
                APPS_LOG_MSGID_I(LOG_TAG" BT_CM event, addr=%08X%04X acl=%d->%d srv=0x%04X->0x%04X reason=%02X",
                                 7, *((uint32_t *)(addr + 2)), *((uint16_t *)addr),
                                 remote_update->pre_acl_state, remote_update->acl_state,
                                 remote_update->pre_connected_service, remote_update->connected_service,
                                 reason);

#if 0
                if (remote_update->pre_acl_state < BT_CM_ACL_LINK_CONNECTED
                    && remote_update->acl_state >= BT_CM_ACL_LINK_CONNECTED) {
                    index = app_bt_source_conn_mgr_find_and_update_index_by_addr(addr);
                    if (index == APP_BT_SOURCE_CONN_INVAILD_INDEX && !app_bt_source_conn_mgr_ctx.is_bonded) {
                        // workaround for PTS
                        APPS_LOG_MSGID_E(LOG_TAG" BT_CM event, Connected by peer", 0);
                        memcpy(app_bt_source_conn_mgr_ctx.device[0].addr, addr, BT_BD_ADDR_LEN);
                        memset(app_bt_source_conn_mgr_ctx.scan_addr, 0, BT_BD_ADDR_LEN);
                    }
                }
#endif

                // Workaround for BTA-41952
                if (remote_update->pre_acl_state == BT_CM_ACL_LINK_CONNECTED
                    && remote_update->acl_state == BT_CM_ACL_LINK_DISCONNECTED
                    && reason == BT_HCI_STATUS_PIN_OR_KEY_MISSING) {
                    index = app_bt_source_conn_mgr_get_index_by_addr(addr);
                    if (index != APP_BT_SOURCE_CONN_INVAILD_INDEX) {
                        uint8_t state = app_bt_source_conn_mgr_ctx.device[index].state;
                        if (state == APP_BT_SOURCE_CONN_STATE_CONNECTING
                            || state == APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT) {
                            APPS_LOG_MSGID_E(LOG_TAG" BT_CM event, reconnect again for KEY_MISSING", 0);
                            app_bt_source_conn_mgr_connect(addr);
                        }
                    }
                    break;
                }

                if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SOURCE) & remote_update->pre_connected_service) == 0
                    && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SOURCE) & remote_update->connected_service) > 0) {
                    index = app_bt_source_conn_mgr_find_and_update_index_by_addr(addr);
                    APPS_LOG_MSGID_I(LOG_TAG" BT_CM event, [%d] A2DP Connected", 1, index);

                    if (index != APP_BT_SOURCE_CONN_INVAILD_INDEX) {
                        bt_cm_connect_t connect_param = {
                            .profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AIR),
                        };
                        memcpy(&connect_param.address, remote_update->address, sizeof(bt_bd_addr_t));
                        bt_cm_connect(&connect_param);
                        APPS_LOG_MSGID_I(LOG_TAG" BT_CM event, [%d] connect SPP AIR prifle", 1, index);
                    }

                    app_bt_source_conn_mgr_notify_dongle_cm(TRUE);
                    app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_CONNECTED);
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
                    if (index != APP_BT_SOURCE_CONN_INVAILD_INDEX) {
#ifdef MTK_RACE_CMD_ENABLE
                        app_dongle_le_race_notify_connect_event(0, APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC, TRUE,
                                                                BT_ADDR_PUBLIC, addr, APP_DONGLE_DEVICE_ID_BASE, 0);
#endif
                    } else {
                        APPS_LOG_MSGID_E(LOG_TAG" BT_CM event, [%d] disconnect the unexpected", 1, index);
                        app_bt_source_conn_mgr_disconnect_addr(addr);
                    }
#endif
                } else if ((remote_update->pre_acl_state == BT_CM_ACL_LINK_DISCONNECTING
                            || remote_update->pre_acl_state == BT_CM_ACL_LINK_CONNECTED
                            || remote_update->pre_acl_state == BT_CM_ACL_LINK_ENCRYPTED)
                        && remote_update->acl_state == BT_CM_ACL_LINK_DISCONNECTED) {
                    index = app_bt_source_conn_mgr_get_index_by_addr(addr);
                    APPS_LOG_MSGID_I(LOG_TAG" BT_CM event, [%d] A2DP Disconnected", 1, index);
                    if (BT_HCI_STATUS_PAGE_TIMEOUT == reason
                        || BT_HCI_STATUS_CONNECTION_TIMEOUT == reason
                        || BT_HCI_STATUS_CONTROLLER_BUSY == reason
                        || BT_HCI_STATUS_LMP_RESPONSE_TIMEOUT_OR_LL_RESPONSE_TIMEOUT == reason
                        || BT_HCI_STATUS_CONNECTION_LIMIT_EXCEEDED == reason
                        || BT_CM_STATUS_ROLE_RECOVERY == reason
                        || BT_HCI_STATUS_VENDOR_REMOTE_CONNECTION_EXIST == reason
                        || BT_HCI_STATUS_CONNECTION_REJECTED_DUE_TO_LIMITED_RESOURCES == reason) {
                        app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_LINK_LOST);
                    } else {
                        app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_DISCONNECT);
                    }

#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL) && defined(MTK_RACE_CMD_ENABLE)
                    if (index != APP_BT_SOURCE_CONN_INVAILD_INDEX) {
                        app_dongle_le_race_notify_connect_event(0, APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC, FALSE,
                                                                BT_ADDR_PUBLIC, addr, 0xFF, 0xFF);
                    }
#endif
                    // Note: notify dongle_cm when disabled and disconnected.
                    if (app_bt_source_conn_mgr_ctx.disabled) {
                        app_bt_source_conn_mgr_notify_dongle_cm(FALSE);
                    }
                }

                // Need to continue to connect after BT CM connect timeout until BT Source TIME_OUT
                if (remote_update->pre_acl_state == BT_CM_ACL_LINK_CONNECTING
                    && remote_update->acl_state == BT_CM_ACL_LINK_DISCONNECTED) {
                    uint8_t index = app_bt_source_conn_mgr_get_index_by_addr(addr);
                    if (index != APP_BT_SOURCE_CONN_INVAILD_INDEX) {
                        if (app_bt_source_conn_mgr_ctx.device[index].state == APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT
                            || app_bt_source_conn_mgr_ctx.device[index].state == APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT) {
                            APPS_LOG_MSGID_W(LOG_TAG" BT_CM event, [%d] continue to reconnect", 1, index);
                            app_bt_source_conn_mgr_connect(addr);
                        } else if (app_bt_source_conn_mgr_ctx.device[index].state == APP_BT_SOURCE_CONN_STATE_CONNECTING) {
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
                            // Note: No "Connect Fail" response to notify PC Tool
#endif
                            APPS_LOG_MSGID_W(LOG_TAG" BT_CM event, [%d] exit connecting", 1, index);
                            app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_CONNECT_FAIL);
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    return FALSE;
}

static bool app_bt_source_conn_mgr_proc_bt_dm_group(uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    bt_device_manager_power_event_t event = 0;
    bt_device_manager_power_status_t status = 0;
    bt_event_get_bt_dm_event_and_status(event_id, &event, &status);
    switch (event) {
        case BT_DEVICE_MANAGER_POWER_EVT_ACTIVE_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
                APPS_LOG_MSGID_I(LOG_TAG" BT DM event, POWER ON", 0);
#ifdef APP_BT_SOURCE_ONLY_MODE
                app_bt_source_conn_mgr_enable(NULL);
#endif
            }
            break;
        }

        case BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_RESET_TYPE_NORMAL == status) {
                APPS_LOG_MSGID_I(LOG_TAG" BT DM event, POWER OFF", 0);
#ifdef APP_BT_SOURCE_ONLY_MODE
                app_bt_source_conn_mgr_disable();
#endif
            }
            break;
        }
    }
    return FALSE;
}

static bool app_bt_source_conn_mgr_proc_bt_source_app_group(uint32_t event_id,
                                                            void *extra_data,
                                                            size_t data_len)
{
    switch (event_id) {
        case APP_BT_SOURCE_EVENT_CONNECT_DEVICE: {
            uint8_t index = (uint8_t)(uint32_t)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" BT SOURCE event, [%d] CONNECT_DEVICE", 1, index);
            app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_SCAN_READY);
            break;
        }

#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_MAX_RSSI)
        case APP_BT_SOURCE_EVENT_CHECK_RSSI_TIMEOUT: {
            uint8_t index = (uint8_t)(uint32_t)extra_data;
            uint8_t scan_num = app_bt_source_conn_mgr_ctx.scan_num;
            APPS_LOG_MSGID_I(LOG_TAG" BT SOURCE event, [%d] CHECK_RSSI_TIMEOUT scan_num=%d", 2, index, scan_num);
            if (scan_num > 0) {
                app_bt_source_conn_mgr_enable_scan(FALSE);
                app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_SCAN_READY);
            } else {
                app_bt_source_conn_mgr_enable_scan(TRUE);
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CHECK_RSSI_TIMEOUT);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CHECK_RSSI_TIMEOUT,
                                    (void *)(uint32_t)index, 0, NULL, APP_BT_SOURCE_SCAN_RSSI_CHECK_TIME);
            }
            break;
        }
#endif

        case APP_BT_SOURCE_EVENT_RESET: {
            APPS_LOG_MSGID_I(LOG_TAG" BT SOURCE event, RESET", 0);
            app_bt_source_conn_mgr_reset();
            break;
        }

        case APP_BT_SOURCE_EVENT_RESET_RESTART_SCAN: {
            uint8_t index = app_bt_source_conn_mgr_get_unused_index();
            APPS_LOG_MSGID_I(LOG_TAG" BT SOURCE event, [%d] RESET_RESTART_SCAN", 1, index);
            app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_START_SCAN);
            break;
        }

#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
        case APP_BT_SOURCE_EVENT_TOOL_SCAN_TIMER: {
            APPS_LOG_MSGID_I(LOG_TAG" BT SOURCE event, TOOL_SCAN_TIMER", 0);
            app_bt_source_conn_mgr_control_scan(FALSE, 0);
            break;
        }

        case APP_BT_SOURCE_EVENT_TOOL_SCANNED: {
            APPS_LOG_MSGID_I(LOG_TAG" BT SOURCE event, TOOL_SCANNED", 0);
            for (int i = 0; i < APP_BT_SOURCE_SCAN_TOOL_MAX_NUM; i++) {
                app_bt_source_scan_item_t *item = &app_bt_source_scan_list[i];
                if (item->scan_state == APP_BT_SOURCE_SCAN_STATE_FOUND) {
#ifdef MTK_RACE_CMD_ENABLE
                    app_dongle_le_race_reply_scan_report(APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC, BT_ADDR_PUBLIC,
                                                         item->addr, item->rssi, item->name, strlen(item->name));
#endif
                    item->scan_state = APP_BT_SOURCE_SCAN_STATE_SEND;
                }
            }
            break;
        }

#ifdef APP_BT_SOURCE_READ_NAME
        case APP_BT_SOURCE_EVENT_TOOL_READ_NAME: {
            bool is_reading = app_bt_source_conn_mgr_is_reading_name();
            APPS_LOG_MSGID_I(LOG_TAG" BT SOURCE event, TOOL_READ_NAME %d", 1, is_reading);
            for (int i = 0; i < APP_BT_SOURCE_SCAN_TOOL_MAX_NUM; i++) {
                app_bt_source_scan_item_t *item = &app_bt_source_scan_list[i];
                if (item->scan_state == APP_BT_SOURCE_SCAN_STATE_NEED_READ && !is_reading) {
                    uint8_t *addr = item->addr;
                    bt_status_t bt_status = bt_gap_read_remote_name((const bt_bd_addr_t *)addr);
                    APPS_LOG_MSGID_I(LOG_TAG" BT SOURCE event, read_name addr=%02X:%02X:%02X:%02X:%02X:%02X bt_status=0x%08X",
                                     7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status);
                    if (bt_status == BT_STATUS_SUCCESS) {
                        item->scan_state = APP_BT_SOURCE_SCAN_STATE_READING;
                        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_READ_NAME_TIMEOUT);
                        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                            EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_READ_NAME_TIMEOUT,
                                            NULL, 0, NULL, APP_BT_SOURCE_READ_NAME_TIMER);
                    } else {
#ifdef MTK_RACE_CMD_ENABLE
                        app_dongle_le_race_reply_scan_report(APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC, BT_ADDR_PUBLIC,
                                                             item->addr, item->rssi, item->name, strlen(item->name));
#endif
                        item->scan_state = APP_BT_SOURCE_SCAN_STATE_SEND;

                        app_bt_source_conn_mgr_check_read_name();
                    }
                    break;
                }
            }
            break;
        }

        case APP_BT_SOURCE_EVENT_TOOL_READ_NAME_TIMEOUT: {
            APPS_LOG_MSGID_I(LOG_TAG" BT SOURCE event, READ_NAME_TIMEOUT", 0);
            for (int i = 0; i < APP_BT_SOURCE_SCAN_TOOL_MAX_NUM; i++) {
                app_bt_source_scan_item_t *item = &app_bt_source_scan_list[i];
                if (item->scan_state == APP_BT_SOURCE_SCAN_STATE_READING) {
                    const uint8_t *addr = item->addr;
                    bt_status_t bt_status = bt_gap_cancel_name_request((const bt_bd_addr_t *)addr);
                    APPS_LOG_MSGID_W(LOG_TAG" cancel_reading, addr=%02X:%02X:%02X:%02X:%02X:%02X bt_status=0x%08X",
                                     7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status);
#ifdef MTK_RACE_CMD_ENABLE
                    app_dongle_le_race_reply_scan_report(APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC, BT_ADDR_PUBLIC,
                                                         item->addr, item->rssi, item->name, strlen(item->name));
#endif
                    item->scan_state = APP_BT_SOURCE_SCAN_STATE_SEND;
                    break;
                }
            }
            app_bt_source_conn_mgr_check_read_name();
            break;
        }
#endif

#endif
        default:
            break;
    }

    if (event_id >= APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT && event_id < APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT_END) {
        uint8_t index = event_id - APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT;
        APPS_LOG_MSGID_I(LOG_TAG" BT SOURCE event, [%d] RECONNECT_TIMEOUT - cancel", 1, index);
        app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_RECONNECT_TIMEOUT);
    }

    return FALSE;
}



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
void app_bt_source_conn_mgr_init(void)
{
    memset(&app_bt_source_conn_mgr_ctx, 0, sizeof(app_bt_source_conn_mgr_context_t));
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          MODULE_MASK_GAP | MODULE_MASK_SYSTEM,
                                          (void *)app_bt_source_conn_mgr_bt_event_callback);

    uint32_t size = BT_BD_ADDR_LEN * APP_BT_SOURCE_BOND_MAX_NUM;
    nvkey_status_t status = nvkey_read_data(NVID_APP_BT_SOURCE_DEVICE_INFO,
                                            (uint8_t *)app_bt_source_conn_mgr_ctx.bond_addr, &size);
    APPS_LOG_MSGID_I(LOG_TAG" init, read status=%d size=%d", 2, status, size);

    if (status == NVKEY_STATUS_OK && size == BT_BD_ADDR_LEN * APP_BT_SOURCE_BOND_MAX_NUM) {
        uint8_t *addr = NULL;
        for (int i = 0; i < APP_BT_SOURCE_BOND_MAX_NUM; i++) {
            addr = (uint8_t *)app_bt_source_conn_mgr_ctx.bond_addr[i];
            if (!app_bt_source_conn_mgr_ctx.is_bonded
                && !app_bt_source_conn_mgr_check_empty_addr(addr)) {
                app_bt_source_conn_mgr_ctx.is_bonded = TRUE;
            }
        }

        app_bt_source_conn_mgr_print_bonded_device();
    }
}

void app_bt_source_conn_mgr_enable(uint8_t *addr)
{
    if (addr == NULL) {
        memset(app_bt_source_conn_mgr_ctx.device, 0, sizeof(app_bt_source_conn_device_t) * APP_BT_SOURCE_CONN_MAX_NUM);
    }
    app_bt_source_conn_mgr_ctx.disabled = FALSE;

    // ToDo, only restore first device to reconnect or start_scan now
    const uint8_t index = 0;
    app_bt_source_conn_mgr_ctx.active_index = index;

    if (addr != NULL) {
        app_bt_source_conn_mgr_control_connect(TRUE, addr);
    } else if (app_bt_source_conn_mgr_ctx.is_bonded) {
        memcpy(app_bt_source_conn_mgr_ctx.device[index].addr, app_bt_source_conn_mgr_ctx.bond_addr[0], BT_BD_ADDR_LEN);

        app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_START_RECONNECT);
    } else {
        APPS_LOG_MSGID_I(LOG_TAG" enable, no bonded addr", 0);
#if (APP_BT_SOURCE_SCAN_POLICY != APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
        app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_START_SCAN);
#endif
    }

    app_bt_source_conn_mgr_enable_page_scan(TRUE);
}

void app_bt_source_conn_mgr_disable(void)
{
    bool cancel_scan = FALSE;
    bool cancel_connect = FALSE;
    for (int i = 0; i < APP_BT_SOURCE_CONN_MAX_NUM; i++) {
        uint8_t state = app_bt_source_conn_mgr_ctx.device[i].state;
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT + i);

        APPS_LOG_MSGID_I(LOG_TAG" disable, [%d] state=%d cancel_scan=%d cancel_connect=%d",
                         4, i, state, cancel_scan, cancel_connect);
        if (state == APP_BT_SOURCE_CONN_STATE_CONNECTED) {
            app_bt_source_conn_mgr_disconnect(i);
        } else if (!cancel_scan && state == APP_BT_SOURCE_CONN_STATE_SCANNING) {
            app_bt_source_conn_mgr_enable_scan(FALSE);
            cancel_scan = TRUE;
        } else if (!cancel_connect && state != APP_BT_SOURCE_CONN_STATE_IDLE) {
            app_bt_source_conn_mgr_cancel_connect();
            cancel_connect = TRUE;
        }
    }

    // Disable page_scan Explicitly, but still keep page_scan if link_lost;
    // Still keep page_scan off if Disconnect link (stop_source) via UT APP actively.
    app_bt_source_conn_mgr_enable_page_scan(FALSE);
    app_bt_source_conn_mgr_ctx.disabled = TRUE;
}

void app_bt_source_conn_mgr_reset(void)
{
    APPS_LOG_MSGID_I(LOG_TAG" reset", 0);
    app_bt_source_conn_mgr_disable();

    for (int i = 0; i < APP_BT_SOURCE_BOND_MAX_NUM; i++) {
        uint8_t *addr = (uint8_t *)app_bt_source_conn_mgr_ctx.bond_addr[i];
        if (!app_bt_source_conn_mgr_check_empty_addr(addr)) {
            bt_device_manager_delete_paired_device(addr);
        }
    }
    nvkey_delete_data_item(NVID_APP_BT_SOURCE_DEVICE_INFO);

    memset(&app_bt_source_conn_mgr_ctx, 0, sizeof(app_bt_source_conn_mgr_context_t));
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_FIRST_DEVICE) || (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_MAX_RSSI)
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_RESET_RESTART_SCAN);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_RESET_RESTART_SCAN,
                        NULL, 0, NULL, APP_BT_SOURCE_RESET_RESTART_SCAN_TIME);
#endif
}

bool app_bt_source_conn_mgr_is_connected(void)
{
    return (app_bt_source_conn_mgr_get_conn_num() > 0);
}

uint8_t app_bt_source_conn_mgr_get_conn_num(void)
{
    uint8_t conn_num = 0;
    for (int i = 0; i < APP_BT_SOURCE_CONN_MAX_NUM; i++) {
        uint8_t state = app_bt_source_conn_mgr_ctx.device[i].state;
        if (state == APP_BT_SOURCE_CONN_STATE_CONNECTED) {
            conn_num++;
        }
    }
    return conn_num;
}

void app_bt_source_conn_mgr_get_conn_info(uint8_t *conn_num, bt_addr_t *list)
{
    int index = 0;
    *conn_num = 0;
    for (int i = 0; i < APP_BT_SOURCE_CONN_MAX_NUM; i++) {
        uint8_t *addr = app_bt_source_conn_mgr_ctx.device[i].addr;
        uint8_t state = app_bt_source_conn_mgr_ctx.device[i].state;
        if (state == APP_BT_SOURCE_CONN_STATE_CONNECTED) {
            list[index].type = BT_ADDR_PUBLIC;
            memcpy(list[index].addr, addr, BT_BD_ADDR_LEN);
            index++;

            *conn_num += 1;

            if (index == 2) {
                break;
            }
        }
    }
}

uint8_t* app_bt_source_conn_mgr_get_active_device(void)
{
    uint8_t *addr = NULL;
    uint8_t index = app_bt_source_conn_mgr_ctx.active_index;
    if (index < APP_BT_SOURCE_CONN_MAX_NUM) {
        addr = app_bt_source_conn_mgr_ctx.device[index].addr;
    }
    return addr;
}

void app_bt_source_conn_mgr_set_active_index(uint8_t index)
{
    APPS_LOG_MSGID_I(LOG_TAG" set_active_index, %d->%d",
                     2, app_bt_source_conn_mgr_ctx.active_index, index);
    app_bt_source_conn_mgr_ctx.active_index = index;
}

uint8_t app_bt_source_conn_mgr_get_active_index(void)
{
    return app_bt_source_conn_mgr_ctx.active_index;
}

void app_bt_source_conn_mgr_connect_addr_bt_atcmd(uint8_t *addr)
{
#if (APP_BT_SOURCE_SCAN_POLICY != APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
    app_bt_source_conn_mgr_enable_scan(FALSE);

    memcpy(app_bt_source_conn_mgr_ctx.scan_addr, addr, BT_BD_ADDR_LEN);
    app_bt_source_conn_mgr_ctx.scan_num = 1;

    uint8_t index = app_bt_source_conn_mgr_get_unused_index();
    if (index == APP_BT_SOURCE_CONN_INVAILD_INDEX) {
        index = 0;
    }

    app_bt_source_conn_mgr_state_t state = app_bt_source_conn_mgr_ctx.device[index].state;
    APPS_LOG_MSGID_I(LOG_TAG" connect_addr_bt_atcmd, [%d] state=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     8, index, state, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);

    if (state == APP_BT_SOURCE_CONN_STATE_IDLE) {
        app_bt_source_conn_mgr_state_machine_run(index, APP_BT_SOURCE_CONN_EVENT_START_SCAN);
    }

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CHECK_RSSI_TIMEOUT);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_CONNECT_DEVICE);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_BT_SOURCE_APP,
                        APP_BT_SOURCE_EVENT_CONNECT_DEVICE, (void *) (uint32_t) index, 0, NULL, 0);

    if (state == APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT) {
        vTaskDelay(1000);
        app_bt_source_conn_mgr_connect(addr);
    }
#else
    APPS_LOG_MSGID_E(LOG_TAG" ignore connect_addr_bt_atcmd for PC Tool", 0);
    //app_bt_source_conn_mgr_control_connect(TRUE, addr);
#endif
}

void app_bt_source_conn_mgr_remove_record(uint8_t *addr)
{
    bt_status_t bt_status = bt_device_manager_delete_paired_device(addr);
    APPS_LOG_MSGID_I(LOG_TAG" remove_record, addr=%02X:%02X:%02X:%02X:%02X:%02X bt_status=0x%08X",
                        7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status);

    if (bt_status == BT_STATUS_SUCCESS) {
        uint8_t index = 0;
        uint8_t found_index = 0xFF;
        for (index = 0; index < APP_BT_SOURCE_BOND_MAX_NUM; index ++) {
            if (memcmp(addr, app_bt_source_conn_mgr_ctx.bond_addr[index], BT_BD_ADDR_LEN) == 0) {
                found_index = index;
                break;
            }
        }

        if (found_index != 0xFF) {
            /**
             * @brief If found the bonded device, need remove it from application layer
             * And write back to the nvkey.
             */
            APPS_LOG_MSGID_I(LOG_TAG" remove_record, remove bonded device at index : %d", 1, found_index);
            if (found_index < APP_BT_SOURCE_BOND_MAX_NUM - 1) {
                for (index = found_index; index < APP_BT_SOURCE_BOND_MAX_NUM - 1; index ++) {
                    memcpy(app_bt_source_conn_mgr_ctx.bond_addr[index], app_bt_source_conn_mgr_ctx.bond_addr[index + 1], BT_BD_ADDR_LEN);
                }
            }

            memset(app_bt_source_conn_mgr_ctx.bond_addr[APP_BT_SOURCE_BOND_MAX_NUM - 1], 0, BT_BD_ADDR_LEN);

            app_bt_source_conn_mgr_print_bonded_device();

            nvkey_status_t nvkey_write_status = nvkey_write_data(NVID_APP_BT_SOURCE_DEVICE_INFO,
                                                                    (const uint8_t *)app_bt_source_conn_mgr_ctx.bond_addr,
                                                                    BT_BD_ADDR_LEN * APP_BT_SOURCE_BOND_MAX_NUM);
            if (nvkey_write_status != NVKEY_STATUS_OK) {
                APPS_LOG_MSGID_E(LOG_TAG" remove_record, write nvkey failed : %d", 1, nvkey_write_status);
            }
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" remove_record, failed to found bonded device", 0);
        }
    }
}

bt_status_t app_bt_source_conn_mgr_find_name(uint8_t *addr, char *name)
{
    bt_status_t bt_status = bt_device_manager_remote_find_name(addr, name, BT_GAP_MAX_DEVICE_NAME_LENGTH);
    APPS_LOG_MSGID_I(LOG_TAG" find_name, addr=%02X:%02X:%02X:%02X:%02X:%02X bt_status=0x%08X name_len=%d",
                     8, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status, strlen(name));

//    bt_device_manager_paired_infomation_t info[3] = {0};
//    uint32_t read_number = 0;
//    bt_device_manager_get_paired_list(info, &read_number);
//    for (int i = 0; i < read_number; i++) {
//        uint8_t *addr1 = info[i].address;
//        char *name = info[i].name;
//        APPS_LOG_MSGID_I(LOG_TAG" find_name, addr1=%02X:%02X:%02X:%02X:%02X:%02X name_len=%d",
//                         7, addr1[5], addr1[4], addr1[3], addr1[2], addr1[1], addr1[0], strlen(name));
//    }
    return bt_status;
}

bt_status_t app_bt_source_conn_mgr_control_scan(bool start, uint32_t time_ms)
{
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
    bool success = TRUE;
    app_dongle_cm_link_mode_t link_mode = app_dongle_cm_get_link_mode();
    if ((link_mode & APP_DONGLE_CM_LINK_MODE_BTA) == 0) {
        APPS_LOG_MSGID_E(LOG_TAG" control_scan, not support BTA mode link_mode=0x%02X", 1, link_mode);
        return BT_STATUS_FAIL;
    }

    if (app_bt_source_conn_mgr_ctx.disabled) {
        APPS_LOG_MSGID_E(LOG_TAG" control_scan, disabled", 0);
        success = FALSE;
    }

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_SCAN_TIMER);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_SCANNED);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_READ_NAME);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_READ_NAME_TIMEOUT);

#ifdef APP_BT_SOURCE_READ_NAME
    // Cancel all reading state and send unknown name
    app_bt_source_conn_mgr_cancel_reading();
#endif

    memset(&app_bt_source_scan_list, 0, APP_BT_SOURCE_SCAN_TOOL_MAX_NUM * sizeof(app_bt_source_scan_item_t));

    if (start) {
        success = app_bt_source_conn_mgr_enable_scan(TRUE);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                            EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_TOOL_SCAN_TIMER,
                            NULL, 0, NULL, time_ms);
    } else {
        success = app_bt_source_conn_mgr_enable_scan(FALSE);
    }

    APPS_LOG_MSGID_I(LOG_TAG" control_scan, start=%d time=%d success=%d", 3, start, time_ms, success);
    return (success ? BT_STATUS_SUCCESS : BT_STATUS_FAIL);
#else
    return BT_STATUS_SUCCESS;
#endif
}

bt_status_t app_bt_source_conn_mgr_control_connect(bool conn_or_disconn, uint8_t *addr)
{
#if (APP_BT_SOURCE_SCAN_POLICY == APP_BT_SOURCE_SCAN_POLICY_BY_TOOL)
    bool success = FALSE;
    if (app_bt_source_conn_mgr_ctx.disabled) {
        APPS_LOG_MSGID_E(LOG_TAG" control_connect, disabled", 0);
        goto exit;
    }

    if (conn_or_disconn) {
#ifdef APP_BT_SOURCE_READ_NAME
        app_bt_source_conn_mgr_cancel_reading();
#endif

        if (APP_BT_SOURCE_CONN_MAX_NUM == 1) {
            // Single link case
            app_bt_source_conn_mgr_control_scan(FALSE, 0);

            app_bt_source_conn_mgr_state_t state = app_bt_source_conn_mgr_ctx.device[0].state;
            APPS_LOG_MSGID_I(LOG_TAG" control_connect, [0] state=%d", 1, state);
            if (state == APP_BT_SOURCE_CONN_STATE_IDLE
                || state == APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT
                || state == APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT) {
                if ((state == APP_BT_SOURCE_CONN_STATE_LINK_LOST_RECONNECT || state == APP_BT_SOURCE_CONN_STATE_POWER_ON_RECONNECT)
                    && memcmp(app_bt_source_conn_mgr_ctx.device[0].addr, addr, BT_BD_ADDR_LEN) != 0) {
                    APPS_LOG_MSGID_E(LOG_TAG" control_connect, [0] use new addr - cancel connecting", 0);
                    uint8_t cancel_num = app_bt_source_conn_mgr_cancel_connect();
                    if (cancel_num > 0) {
                        // wait cancel connecting the connect other
                        vTaskDelay(300);
                    }
                }

                memset(app_bt_source_conn_mgr_ctx.device[0].addr, 0, BT_BD_ADDR_LEN);
                memcpy(app_bt_source_conn_mgr_ctx.scan_addr, addr, BT_BD_ADDR_LEN);
                app_bt_source_conn_mgr_ctx.scan_num = 1;
                app_bt_source_conn_mgr_state_machine_run(0, APP_BT_SOURCE_CONN_EVENT_START_CONNECT);
                success = TRUE;
            } else if (state == APP_BT_SOURCE_CONN_STATE_CONNECTING
                    || state == APP_BT_SOURCE_CONN_STATE_CONNECTED) {
                // Reply fail status via RACE when connecting/connected/power_on_reconnet state
                goto exit;
            } else {
                configASSERT(0);
            }
        } else {
            // ToDo, support multi-link
        }
    } else {
        int index = -1;
        for (int i = 0; i < APP_BT_SOURCE_CONN_MAX_NUM; i++) {
            if (memcmp(addr, app_bt_source_conn_mgr_ctx.device[i].addr, BT_BD_ADDR_LEN) == 0) {
                index = i;
                app_bt_source_conn_mgr_disconnect(index);
                success = TRUE;
                break;
            }
        }

        if (index == -1) {
            APPS_LOG_MSGID_E(LOG_TAG" control_connect, disconnect invalid index", 0);
            app_bt_source_conn_mgr_disconnect_addr(addr);
            success = TRUE;
        }
    }

exit:
    APPS_LOG_MSGID_I(LOG_TAG" control_connect, conn_or_disconn=%d addr=%02X:%02X:%02X:%02X:%02X:%02X success=%d",
                     8, conn_or_disconn, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], success);
    return (success ? BT_STATUS_SUCCESS : BT_STATUS_FAIL);
#else
    return BT_STATUS_SUCCESS;
#endif
}

bool app_bt_source_conn_mgr_proc_ui_shell_event(uint32_t event_group,
                                                uint32_t event_id,
                                                void *extra_data,
                                                size_t data_len)
{
    bool ret = FALSE;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            ret = app_bt_source_conn_mgr_proc_bt_cm_group(event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER: {
            ret = app_bt_source_conn_mgr_proc_bt_dm_group(event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_SOURCE_APP: {
            ret = app_bt_source_conn_mgr_proc_bt_source_app_group(event_id, extra_data, data_len);
            break;
        }
        default:
            break;
    }
    return ret;
}

#endif /* AIR_BT_SOURCE_ENABLE */
