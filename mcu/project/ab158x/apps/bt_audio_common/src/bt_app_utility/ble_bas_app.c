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

#include "ble_bas_app.h"

#include "apps_debug.h"
#include "apps_events_battery_event.h"
#include "apps_events_event_group.h"

#include "bt_callback_manager.h"
#include "bt_gap_le.h"
#include "ble_bas.h"
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#endif

#define LOG_TAG           "[BAS][APP]"

/**================================================================================*/
/**                                Define/Structure                                */
/**================================================================================*/
#define BLE_BAS_MAX_COUNT                   5

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

typedef struct {
    uint16_t                                handle;
    bool                                    cccd_enable;
} PACKED ble_bas_app_item_t;

typedef struct {
    ble_bas_app_item_t                      list[BLE_BAS_MAX_COUNT];
    uint8_t                                 battery;
} PACKED ble_bas_app_context_t;

static ble_bas_app_context_t                ble_bas_ctx = {0};



/**================================================================================*/
/**                                  Internal API                                  */
/**================================================================================*/
static void ble_bas_app_notify_battery(uint8_t battery)
{
    if (ble_bas_ctx.battery == battery || battery > 100) {
        APPS_LOG_MSGID_E(LOG_TAG" notify_battery, invalid battery %d %d", 2, ble_bas_ctx.battery, battery);
        return;
    }

    ble_bas_ctx.battery = battery;
    for (int i = 0; i < BLE_BAS_MAX_COUNT; i++) {
        if (ble_bas_ctx.list[i].handle != BT_HANDLE_INVALID && ble_bas_ctx.list[i].cccd_enable) {
            bt_status_t status = ble_bas_notify_battery_level(ble_bas_ctx.list[i].handle, battery);
            APPS_LOG_MSGID_I(LOG_TAG" notify_battery, [%d] handle=0x%04X battery=%d status=0x%08X",
                             4, i, ble_bas_ctx.list[i].handle, battery, status);
        }
    }
}

static void ble_bas_app_update_conn_handle(bool connect, uint16_t handle)
{
    for (int i = 0; i < BLE_BAS_MAX_COUNT; i++) {
        if (connect && ble_bas_ctx.list[i].handle == BT_HANDLE_INVALID) {
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
            uint8_t battery = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);
#else
            uint8_t battery = 100;
#endif
            ble_bas_ctx.list[i].handle = handle;
            ble_bas_ctx.list[i].cccd_enable = FALSE;
            ble_bas_ctx.battery = battery;
            APPS_LOG_MSGID_I(LOG_TAG" update_conn_handle, [%d] connect handle=0x%04X battery=%d",
                             3, i, handle, battery);
            break;
        } else if (!connect && ble_bas_ctx.list[i].handle == handle) {
            ble_bas_ctx.list[i].handle = BT_HANDLE_INVALID;
            ble_bas_ctx.list[i].cccd_enable = FALSE;
            APPS_LOG_MSGID_I(LOG_TAG" update_conn_handle, [%d] disconnect handle=0x%04X", 2, i, handle);
            break;
        }
    }
}



/**================================================================================*/
/**                                  Callback API                                  */
/**================================================================================*/
uint8_t ble_bas_read_callback(ble_bas_event_t event, bt_handle_t conn_handle)
{
    bool found = FALSE;
    for (int i = 0; i < BLE_BAS_MAX_COUNT; i++) {
        if (conn_handle == ble_bas_ctx.list[i].handle) {
            APPS_LOG_MSGID_I(LOG_TAG" read_callback, [%d] event=%d handle=0x%04X battery=%d cccd=%d",
                             5, i, event, conn_handle, ble_bas_ctx.battery, ble_bas_ctx.list[i].cccd_enable);
            if (event == BLE_BAS_EVENT_BATTRY_LEVEL_READ) {
                return ble_bas_ctx.battery;
            } else if (event == BLE_BAS_EVENT_CCCD_READ) {
                return ble_bas_ctx.list[i].cccd_enable;
            }
            found = TRUE;
            return 0;
        }
    }

    if (!found) {
        APPS_LOG_MSGID_E(LOG_TAG" read_callback, not found handle=0x%04X", 1, conn_handle);
    }
    return 0;
}

void ble_bas_write_callback(ble_bas_event_t event, bt_handle_t conn_handle, void *data)
{
    bool found = FALSE;
    for (int i = 0; i < BLE_BAS_MAX_COUNT; i++) {
        if (conn_handle == ble_bas_ctx.list[i].handle && event == BLE_BAS_EVENT_CCCD_WRITE) {
            uint16_t cccd_value = *((uint16_t *)data);
            APPS_LOG_MSGID_I(LOG_TAG" write_callback, [%d] event=%d handle=0x%04X cccd_value=%d",
                             4, i, event, conn_handle, cccd_value);
            ble_bas_ctx.list[i].cccd_enable = cccd_value;
            found = TRUE;

            if (cccd_value) {
                bt_status_t status = ble_bas_notify_battery_level(conn_handle, ble_bas_ctx.battery);
                APPS_LOG_MSGID_I(LOG_TAG" write_callback, handle=0x%04X notify_battery=%d status=0x%08X",
                                 3, conn_handle, ble_bas_ctx.battery, status);
            }
            break;
        }
    }

    if (!found) {
        APPS_LOG_MSGID_E(LOG_TAG" write_callback, not found event=%d handle=0x%04X", 2, event, conn_handle);
    }
}

static bt_status_t ble_bas_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    if (status != BT_STATUS_SUCCESS) {
        return BT_STATUS_SUCCESS;
    }

    switch (msg) {
        case BT_GAP_LE_CONNECT_IND: {
            bt_gap_le_connection_ind_t *conn_ind = (bt_gap_le_connection_ind_t *)buffer;
            if (conn_ind == NULL || BT_HANDLE_INVALID == conn_ind->connection_handle) {
                break;
            }

            bt_handle_t conn_handle = conn_ind->connection_handle;
            ble_bas_app_update_conn_handle(TRUE, conn_handle);
            break;
        }

        case BT_GAP_LE_DISCONNECT_IND: {
            bt_hci_evt_disconnect_complete_t *disconn_ind = (bt_hci_evt_disconnect_complete_t *)buffer;
            if (disconn_ind == NULL || BT_HANDLE_INVALID == disconn_ind->connection_handle) {
                break;
            }

            bt_handle_t conn_handle = disconn_ind->connection_handle;
            ble_bas_app_update_conn_handle(FALSE, conn_handle);
            break;
        }

        case BT_POWER_OFF_CNF: {
            APPS_LOG_MSGID_I(LOG_TAG" BT_POWER_OFF_CNF", 0);
            memset(&ble_bas_ctx, 0, sizeof(ble_bas_app_context_t));
            for (int i = 0; i < BLE_BAS_MAX_COUNT; i++) {
                ble_bas_ctx.list[i].handle = BT_HANDLE_INVALID;
            }
            break;
        }

        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}



/**================================================================================*/
/**                                   Public API                                   */
/**================================================================================*/
void ble_bas_app_init(void)
{
    APPS_LOG_MSGID_I(LOG_TAG" init", 0);
    bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_SYSTEM | MODULE_MASK_GAP,
                                          (void *)ble_bas_app_event_callback);

    memset(&ble_bas_ctx, 0, sizeof(ble_bas_app_context_t));
    for (int i = 0; i < BLE_BAS_MAX_COUNT; i++) {
        ble_bas_ctx.list[i].handle = BT_HANDLE_INVALID;
    }
}

void ble_bas_app_handle_event(uint32_t event_group, uint32_t event_id, void *extra_data, size_t data_len)
{
    if (event_group == EVENT_GROUP_UI_SHELL_BATTERY && event_id == APPS_EVENTS_BATTERY_PERCENT_CHANGE) {
        uint8_t battery = (uint8_t)(int32_t)extra_data;
        ble_bas_app_notify_battery(battery);
    }
}
