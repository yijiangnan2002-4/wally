
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
 * File: app_le_audio_conn_mgr.c
 *
 * Description: This file provides API for LE Audio connection manage.
 *
 */

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)

#include "app_lea_service_conn_mgr.h"

#include "app_lea_service.h"
#include "app_lea_service_adv_mgr.h"
#include "app_lea_service_event.h"
#include "app_lea_service_sync_info.h"

#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio.h"
#include "app_le_audio_aird_client.h"
#endif

#include "apps_debug.h"
#include "apps_events_bt_event.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#include "apps_aws_sync_event.h"
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
#include "app_rho_idle_activity.h"
#endif
#endif

#include "bt_app_common.h"
#include "bt_aws_mce_srv.h"
#include "bt_customer_config.h"
#include "bt_device_manager.h"
#include "bt_device_manager_le.h"
#include "bt_gattc_discovery.h"
#include "bt_gap_le.h"
#include "bt_gap_le_service.h"
#include "bt_sink_srv_le.h"

#include "FreeRTOS.h"
#include "task.h"
#include "task_def.h"
#include "multi_ble_adv_manager.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "ui_shell_manager.h"
#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#endif
#ifdef AIR_BT_TAKEOVER_ENABLE
#include "app_bt_takeover_service.h"
#endif
#ifdef AIR_GATT_SRV_CLIENT_ENABLE
#include "bt_gatt_service_client.h"
#endif
#ifdef MTK_FOTA_ENABLE
#include "fota.h"
#endif



#define LOG_TAG             "[LEA][CONN]"

#define APP_LEA_NORMAL_CONN_INTERVAL                       (0x0030) // 0x30, 60ms=48*1.25

#define APP_LEA_CENTRAL_ADDRESS_RESOLUTION_MASK            (0x03)   // bit0~1
#define APP_LEA_CENTRAL_ADDRESS_RESOLUTION_UNKNOWN         (0x0)    // bit0~1, 0b00
#define APP_LEA_CENTRAL_ADDRESS_RESOLUTION_ENABLE          (0x1)    // bit0~1, 0b01
#define APP_LEA_CENTRAL_ADDRESS_RESOLUTION_DISABLE         (0x2)    // bit0~1, 0b10

#define APP_LEA_VALUE_LEA_DONGLE_MASK                      (0x80)   // bit8

typedef enum {
    APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED = 0,        // Default, Target announcement flag
    APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED,                 // General announcement flag
    APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV,              // Link Lost, Direct ADV for Intel EVO
    APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED,               // Already Connected
    APP_LEA_CONN_ACTIVE_RECONNECT_TEMP_NO_NEED,            // NO_NEED temporarily, but restore to DEFAULT_NEED on some cases
} app_lea_conn_active_reconnect_type_t;

typedef enum {
    APP_LEA_CONN_SPECIAL_PAIRING_NONE                       = 0,
    APP_LEA_CONN_SPECIAL_PAIRING_GFP,
    APP_LEA_CONN_SPECIAL_PAIRING_SWIFT,
} app_lea_conn_special_pairing_type_t;

#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
static uint16_t                            app_lea_conn_gfp_standby_handle = 0;
#endif
#ifdef APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE
static uint16_t                            app_lea_conn_swift_standby_handle = 0;
#endif

#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
typedef struct {
    uint8_t              addr_type;
    uint8_t              ida[BT_BD_ADDR_LEN];
    uint8_t              irk[BT_KEY_SIZE];
} PACKED app_lea_conn_sync_ida_info_t;

typedef enum {
    APP_LEA_CONN_IDA_STATE_NONE                       = 0,
    APP_LEA_CONN_IDA_STATE_READY_TO_ADD,
    APP_LEA_CONN_IDA_STATE_ADD,
} app_lea_conn_ida_state_t;

typedef struct {
    uint8_t              addr_type;
    uint8_t              ida[BT_BD_ADDR_LEN];
    uint8_t              irk[BT_KEY_SIZE];
    uint8_t              state;
} PACKED app_lea_conn_ida_info_item_t;

#define APP_LEA_MAX_IDA_INFO_NUM          (APP_LEA_MAX_BOND_NUM + 3)
static app_lea_conn_ida_info_item_t       app_lea_conn_ida_info_list[APP_LEA_MAX_IDA_INFO_NUM] = {0};
static app_lea_conn_ida_info_item_t       app_lea_conn_cur_ida_info = {0};

#endif

typedef struct {
    bool                        used;
    uint8_t                     addr_type;
    uint8_t                     addr[BT_BD_ADDR_LEN];
} PACKED app_lea_conn_unacive_addr_t;

typedef struct {
    bt_handle_t                 conn_handle;
    app_lea_conn_type_t         conn_type;
    uint8_t                     peer_random_addr[BT_BD_ADDR_LEN];
    uint8_t                     peer_type;
    uint8_t                     peer_id_addr[BT_BD_ADDR_LEN];
#ifdef AIR_LE_AUDIO_ENABLE
    uint16_t                    conn_interval;
    uint16_t                    conn_latency;
    uint16_t                    supervision_timeout;
    uint8_t                     special_pairing;

    uint8_t                     tbs_discovery_done      : 1;
    uint8_t                     tbs_char_num            : 1;
    uint8_t                     gtbs_discovery_done     : 1;
    uint8_t                     gtbs_char_num           : 1;
    uint8_t                     mcs_discovery_done      : 1;
    uint8_t                     mcs_char_num            : 1;
    uint8_t                     gmcs_discovery_done     : 1;
    uint8_t                     gmcs_char_num           : 1;
#ifdef AIR_TWS_ENABLE
    uint8_t                     irk[BT_KEY_SIZE];
#endif
#endif
} PACKED app_lea_conn_info_t;

static app_lea_conn_info_t                 app_lea_conn_info_list[APP_LEA_MAX_CONN_NUM] = {0};

#define APP_LEA_BOND_INFO_LENGTH           (sizeof(app_lea_bond_info_t) * APP_LEA_MAX_BOND_NUM)
static app_lea_bond_info_t                 app_lea_bond_info_list[APP_LEA_MAX_BOND_NUM] = {0};  // LEA/ULL2 Device Info List

static app_lea_conn_mgr_connection_cb_t    app_lea_connection_cb = NULL;



/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
static void app_lea_conn_mgr_save_bond_info(bool adjust);
#ifdef AIR_LE_AUDIO_ENABLE
static void app_lea_conn_mgr_handle_discovery_result(uint32_t info);
#endif

#define APP_LEA_IS_EMPTY_ADDR(X, EMPTY)        (memcmp((X), (EMPTY), BT_BD_ADDR_LEN) == 0)

static uint8_t app_lea_conn_mgr_get_link_count(void)
{
    uint8_t conn_num = 0;
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (app_lea_conn_info_list[i].conn_handle != BT_HANDLE_INVALID) {
            conn_num++;
        }
    }
    return conn_num;
}

static void app_lea_conn_mgr_clear_conn_info(void)
{
    memset(&app_lea_conn_info_list[0], 0, APP_LEA_MAX_CONN_NUM * sizeof(app_lea_conn_info_t));
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        app_lea_conn_info_list[i].conn_handle = BT_HANDLE_INVALID;
    }
}

static void app_lea_conn_mgr_print_bond_info(bool now)
{
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        const uint8_t *addr = app_lea_bond_info_list[i].bond_addr;
        if (now) {
            APPS_LOG_MSGID_I(LOG_TAG" update_bond_info, [now][%d] used=%d conn_type=%d addr_type=%d reconnect=%d addr=%08X%04X value=0x%08X",
                             8, i, app_lea_bond_info_list[i].used, app_lea_bond_info_list[i].conn_type,
                             app_lea_bond_info_list[i].addr_type, app_lea_bond_info_list[i].reconnect_type,
                             *((uint32_t *)(addr + 2)), *((uint16_t *)addr), app_lea_bond_info_list[i].value);
        } else {
            APPS_LOG_MSGID_I(LOG_TAG" update_bond_info, [before][%d] used=%d conn_type=%d addr_type=%d reconnect=%d addr=%08X%04X value=0x%08X",
                             8, i, app_lea_bond_info_list[i].used, app_lea_bond_info_list[i].conn_type,
                             app_lea_bond_info_list[i].addr_type, app_lea_bond_info_list[i].reconnect_type,
                             *((uint32_t *)(addr + 2)), *((uint16_t *)addr), app_lea_bond_info_list[i].value);
        }
    }
}

static void app_lea_conn_mgr_remove_le_bond_info(uint8_t addr_type, uint8_t *addr)
{
    // Need to remove LE Bond ?
    /*
    bt_addr_t remove_bt_addr = {0};
    remove_bt_addr.type = addr_type;
    memcpy(remove_bt_addr.addr, addr, BT_BD_ADDR_LEN);
    bt_device_manager_le_remove_bonded_device(&remove_bt_addr);
    APPS_LOG_MSGID_W(LOG_TAG" remove_le_bond_info, addr=%08X%04X", 2, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
     */
}

static void app_lea_conn_mgr_adjust_reconnect_type(void)
{
    app_lea_conn_mgr_print_bond_info(TRUE);

#if defined(AIR_LE_AUDIO_USE_DIRECT_ADV_TO_ACTIVE_RECONNECT) && !defined(AIR_BT_INTEL_EVO_ENABLE)
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        uint8_t conn_type = app_lea_bond_info_list[i].conn_type;
        uint8_t reconnect_type = app_lea_bond_info_list[i].reconnect_type;
        if (conn_type == APP_LEA_CONN_TYPE_LE_AUDIO
            && reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV) {
            APPS_LOG_MSGID_W(LOG_TAG" adjust_reconnect_type, [%d] DIRECT_ADV reset to DEFAULT_NEED", 1, i);
            app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED;
        }
    }
#endif

    // Change ULL2 reconnect type (Default_need or no_need) to avoiding ADV rotation
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    uint8_t adv_mode = app_lea_adv_mgr_get_adv_mode();
    // If ADV mode is general (no whitelist), always set ULL2 as RECONNECT_NO_NEED
    if (adv_mode == APP_LEA_ADV_MODE_GENERAL) {
        for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
            if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_ULL
                && app_lea_bond_info_list[i].reconnect_type != APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED) {
                APPS_LOG_MSGID_W(LOG_TAG" adjust_reconnect_type, [%d] set ULL2 to NO_NEED when General ADV", 1, i);
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED;
            }
        }
    } else {
        uint8_t lea_num = 0;
        uint8_t ull2_num = 0;
        bool is_exist_direct_adv = 0;
        uint8_t lea_active_reconnect_num = 0;
        uint8_t lea_no_need_num = 0;
        for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
            uint8_t conn_type = app_lea_bond_info_list[i].conn_type;
            uint8_t reconnect_type = app_lea_bond_info_list[i].reconnect_type;
            if (reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED) {
                continue;
            }
            if (conn_type == APP_LEA_CONN_TYPE_LE_AUDIO) {
                lea_num++;
                if (reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED) {
                    lea_active_reconnect_num++;
                } else if (reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED
                           || reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_TEMP_NO_NEED) {
                    lea_no_need_num++;
                }
            } else if (conn_type == APP_LEA_CONN_TYPE_LE_ULL) {
                ull2_num++;
            }
            if (reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV) {
                is_exist_direct_adv = TRUE;
            }
        }

        if (!is_exist_direct_adv && lea_num > 0 && ull2_num > 0) {
            if (lea_active_reconnect_num == lea_num && lea_no_need_num == 0) {
                for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
                    if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_ULL
                        && app_lea_bond_info_list[i].reconnect_type != APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED) {
                        APPS_LOG_MSGID_W(LOG_TAG" adjust_reconnect_type, [%d] set ULL2 to DEFAULT_NEED", 1, i);
                        app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED;
                    }
                }
            } else if (lea_no_need_num == lea_num && lea_active_reconnect_num == 0) {
                for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
                    if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_ULL
                        && app_lea_bond_info_list[i].reconnect_type != APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED) {
                        APPS_LOG_MSGID_W(LOG_TAG" adjust_reconnect_type, [%d] set ULL2 to NO_NEED", 1, i);
                        app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED;
                    }
                }
            }
        }
    }
#endif

#ifdef AIR_LE_AUDIO_USE_DIRECT_ADV_TO_ACTIVE_RECONNECT
    // When earbuds need to start LEA ADV and rotation each 500ms,
    // LEA ADV with targeted announcement flag for phone A (as whitelist) and general announcement flag for phone B (as whitelist)
    // But Phone B could found the LEA ADV with targeted announcement flag and initiate connection request.
    // In order to avoid the case, start Direct ADV to replace whitelist ADV with targeted announcement flag.
    // But Direct ADV only cover one phone, many Direct ADV will reduce LEA connection performance.
#ifdef APP_LE_AUDIO_ADV_ACTIVE_RECONNECT_TYPE_ALWAYS_USE_DIRECT_ADV
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO
            && app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED
            && (app_lea_bond_info_list[i].value & APP_LEA_CENTRAL_ADDRESS_RESOLUTION_MASK) > 0) {
            APPS_LOG_MSGID_W(LOG_TAG" adjust_reconnect_type, [%d] always set DEFAULT_NEED to DIRECT_ADV", 1, i);
            app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV;
        }
    }
#else
    uint8_t lea1_active_reconnect_num = 0;
    uint8_t lea1_no_need_num = 0;
    uint8_t lea1_connected_num = 0;
    uint8_t ull2_num1 = 0;
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO) {
            if (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED) {
                lea1_active_reconnect_num++;
            } else if (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED) {
                lea1_no_need_num++;
            } else if (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED) {
                lea1_connected_num++;
            }
        } else if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_ULL) {
            ull2_num1++;
        }
    }
    if (lea1_active_reconnect_num > 0 && (lea1_no_need_num > 0 || lea1_connected_num > 0)) {
        for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
            uint8_t conn_type = app_lea_bond_info_list[i].conn_type;
            uint8_t reconnect_type = app_lea_bond_info_list[i].reconnect_type;
            if (conn_type == APP_LEA_CONN_TYPE_LE_AUDIO
                && reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED
                && (app_lea_bond_info_list[i].value & APP_LEA_CENTRAL_ADDRESS_RESOLUTION_MASK) > 0) {
                APPS_LOG_MSGID_W(LOG_TAG" adjust_reconnect_type, [%d] set DEFAULT_NEED to DIRECT_ADV", 1, i);
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV;
            }

            if (ull2_num1 > 0 && conn_type == APP_LEA_CONN_TYPE_LE_ULL
                && app_lea_bond_info_list[i].reconnect_type != APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED) {
                APPS_LOG_MSGID_W(LOG_TAG" adjust_reconnect_type, [%d] reset ULL2 to NO_NEED", 1, i);
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED;
            }
        }
    }
#endif
#endif
}

void app_lea_conn_mgr_update_reconnect_type(const uint8_t *bd_addr, uint8_t reconnect_type)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};

    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        const uint8_t *addr = app_lea_bond_info_list[i].bond_addr;
        uint8_t conn_type = app_lea_bond_info_list[i].conn_type;
        uint8_t addr_type = app_lea_bond_info_list[i].addr_type;
        uint8_t old_reconnect_type = app_lea_bond_info_list[i].reconnect_type;

        if (!app_lea_bond_info_list[i].used || (memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0)) {
            continue;
        }

        // Don't support direct adv for LEA Dongle / ULL2 Dongle
        if (reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV
            && (addr_type == BT_ADDR_RANDOM || conn_type == APP_LEA_CONN_TYPE_LE_ULL)) {
            reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED;
        }

        if (memcmp(addr, bd_addr, BT_BD_ADDR_LEN) == 0 && old_reconnect_type != reconnect_type) {
            app_lea_bond_info_list[i].reconnect_type = reconnect_type;
            APPS_LOG_MSGID_W(LOG_TAG" update_reconnect_type, [%d] conn_type=%d addr_type=%d reconnect=%d addr=%08X%04X",
                             6, i, conn_type, addr_type, reconnect_type,
                             *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
        }
    }

    app_lea_conn_mgr_adjust_reconnect_type();
}

static bool app_lea_conn_mgr_update_bond_info(uint8_t addr_type, uint8_t *bd_addr, uint8_t conn_type)
{
    bool duplicate = FALSE;
    int duplicate_index = -1;
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};

    if (APP_LEA_IS_EMPTY_ADDR(bd_addr, empty_addr) || addr_type > BT_ADDR_RANDOM_IDENTITY) {
        APPS_LOG_MSGID_I(LOG_TAG" update_bond_info, error addr/type", 0);
        return FALSE;
    }

    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        const uint8_t *addr = app_lea_bond_info_list[i].bond_addr;
        if (memcmp(addr, bd_addr, BT_BD_ADDR_LEN) == 0) {
            duplicate = TRUE;
            duplicate_index = i;
            break;
        }
    }

    if (duplicate) {
        APPS_LOG_MSGID_I(LOG_TAG" update_bond_info, duplicate_index=%d", 1, duplicate_index);
        app_lea_conn_mgr_print_bond_info(FALSE);
        if (duplicate_index != 0) {
            app_lea_bond_info_t temp = {0};
            memcpy(&temp, &app_lea_bond_info_list[duplicate_index], sizeof(app_lea_bond_info_t));
            for (int i = duplicate_index - 1; i >= 0; i--) {
                memcpy(&app_lea_bond_info_list[i + 1], &app_lea_bond_info_list[i], sizeof(app_lea_bond_info_t));
            }
            memcpy(&app_lea_bond_info_list[0], &temp, sizeof(app_lea_bond_info_t));
            app_lea_conn_mgr_print_bond_info(TRUE);
        }
        return FALSE;
    }

    app_lea_bond_info_t remove_item = {0};
    // Prevent the dongle info (last position) be remove; ignore ULL2 + LEA dongle co-exist case;
    bool is_last_dongle = FALSE;
    app_lea_bond_info_t last_item = app_lea_bond_info_list[APP_LEA_MAX_BOND_NUM - 1];
    if (last_item.used) {
        remove_item = last_item;
        bool is_last_lea_dongle = (last_item.conn_type == APP_LEA_CONN_TYPE_LE_AUDIO
                                   && (last_item.value & APP_LEA_VALUE_LEA_DONGLE_MASK) > 0);
#ifdef AIR_BLE_ULL_REMOVE_OLD_RECORD_ENABLE
        // ULL2->ULL2 via AIR_BLE_ULL_REMOVE_OLD_RECORD_ENABLE, only handle LEA->ULL2
        bool is_last_ull2_dongle = (conn_type == APP_LEA_CONN_TYPE_LE_AUDIO && last_item.conn_type == APP_LEA_CONN_TYPE_LE_ULL);
#else
        bool is_last_ull2_dongle = (last_item.conn_type == APP_LEA_CONN_TYPE_LE_ULL);
#endif
        is_last_dongle = (is_last_ull2_dongle || is_last_lea_dongle);
    }

    app_lea_conn_mgr_print_bond_info(FALSE);
    for (int i = (APP_LEA_MAX_BOND_NUM - 1 - 1); i >= 0; i--) {
        if (app_lea_bond_info_list[i].used) {
            memcpy(&app_lea_bond_info_list[i + 1], &app_lea_bond_info_list[i], sizeof(app_lea_bond_info_t));
        }
    }
    app_lea_bond_info_list[0].used = TRUE;
    app_lea_bond_info_list[0].conn_type = conn_type;
    app_lea_bond_info_list[0].addr_type = addr_type;
    app_lea_bond_info_list[0].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED;
    app_lea_bond_info_list[0].value = 0;
    memcpy(app_lea_bond_info_list[0].bond_addr, bd_addr, BT_BD_ADDR_LEN);

    if (is_last_dongle && conn_type != APP_LEA_CONN_TYPE_LE_ULL) {
        APPS_LOG_MSGID_W(LOG_TAG" update_bond_info, must keep last link", 0);
        remove_item = app_lea_bond_info_list[APP_LEA_MAX_BOND_NUM - 1];
        app_lea_bond_info_list[APP_LEA_MAX_BOND_NUM - 1] = last_item;
    }

    if (remove_item.used) {
        if (remove_item.reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED) {
            app_lea_conn_mgr_disconnect(remove_item.bond_addr, BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
        }
        app_lea_conn_mgr_remove_le_bond_info(remove_item.addr_type, remove_item.bond_addr);
    }

#ifdef AIR_BLE_ULL_REMOVE_OLD_RECORD_ENABLE
    if (conn_type == APP_LEA_CONN_TYPE_LE_ULL) {
        for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
            uint8_t conn_type = app_lea_bond_info_list[i].conn_type;
            uint8_t *addr = app_lea_bond_info_list[i].bond_addr;
            if (conn_type == APP_LEA_CONN_TYPE_LE_ULL && memcmp(addr, bd_addr, BT_BD_ADDR_LEN) != 0) {
                uint8_t addr_type = app_lea_bond_info_list[i].addr_type;
                APPS_LOG_MSGID_W(LOG_TAG" update_bond_info, remove first ULL2 addr", 0);
                app_lea_conn_mgr_remove_bond_info(TRUE, addr_type, addr);
                break;
            }
        }
    }
#endif

    app_lea_conn_mgr_print_bond_info(TRUE);
    app_lea_conn_mgr_save_bond_info(FALSE);

#ifdef AIR_LE_AUDIO_BOTH_SYNC_INFO
    app_lea_sync_info_send();
#endif

    return TRUE;
}

static void app_lea_conn_mgr_save_bond_info(bool adjust)
{
    if (adjust) {
        uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
        for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
            if (!app_lea_bond_info_list[i].used || (memcmp(app_lea_bond_info_list[i].bond_addr, empty_addr, BT_BD_ADDR_LEN) == 0)) {
                continue;
            }

            uint8_t conn_type = app_lea_bond_info_list[i].conn_type;
            uint8_t reconnect_type = app_lea_bond_info_list[i].reconnect_type;
            if (conn_type == APP_LEA_CONN_TYPE_LE_ULL && reconnect_type != APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED) {
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED;
            }
            if (conn_type == APP_LEA_CONN_TYPE_LE_AUDIO && app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV) {
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED;
            }
#ifdef APP_LE_AUDIO_ADV_RESTORE_TEMP_NO_NEED_TO_ACTIVE_RECONNECT
            if (reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_TEMP_NO_NEED) {
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED;
            }
#else
            if (reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_TEMP_NO_NEED) {
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED;
            }
#endif
#ifdef APP_LE_AUDIO_ADV_CLEAR_LEA_ACTIVE_RECONNECT_TYPE_AFTER_POWER_ON
            if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO) {
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED;
                APPS_LOG_MSGID_E(LOG_TAG" save_bond_info, [%d] CLEAR_LEA_ACTIVE_RECONNECT_TYPE_AFTER_POWER_ON",
                                 1, i);
            }
#endif
        }
    }

    nvkey_status_t status = nvkey_write_data(NVID_APP_LE_AUDIO_CONN_FLAG,
                                             (const uint8_t *)&app_lea_bond_info_list[0],
                                             APP_LEA_BOND_INFO_LENGTH);
    APPS_LOG_MSGID_I(LOG_TAG" save_bond_info, status=%d", 1, status);
}

static void app_lea_conn_mgr_restore_bond_info(void)
{
#ifdef MTK_FOTA_ENABLE
    bool ota_result = FALSE;
    fota_get_upgrade_result(&ota_result);
    if (ota_result) {
        uint8_t old_item_len = sizeof(app_lea_old_bond_info_t);         // 9 * 8 = 72
        uint8_t old_item1_len = sizeof(app_lea_old_bond_info1_t);       // 10 * 7 = 70
        uint8_t new_item_len = sizeof(app_lea_bond_info_t);             // 14 * 7 = 98
        uint32_t size = 0;
        nvkey_status_t status = nvkey_data_item_length(NVID_APP_LE_AUDIO_CONN_FLAG, &size);
        APPS_LOG_MSGID_E(LOG_TAG" restore_bond_info, FOTA status=%d item_len=%d/%d-%d size=%d-%d",
                         6, status, old_item_len, old_item1_len,
                         new_item_len, size, APP_LEA_BOND_INFO_LENGTH);
        if (status == NVKEY_STATUS_OK && size > 0 && size != APP_LEA_BOND_INFO_LENGTH && (size % old_item_len) == 0) {
            uint8_t old_item_num = size / old_item_len;
            memset(&app_lea_bond_info_list[0], 0, APP_LEA_BOND_INFO_LENGTH);
            app_lea_old_bond_info_t *temp_list = (app_lea_old_bond_info_t *)pvPortMalloc(sizeof(app_lea_old_bond_info_t) * old_item_num);
            if (temp_list == NULL) {
                APPS_LOG_MSGID_E(LOG_TAG" restore_bond_info, malloc fail", 0);
                return;
            }
            nvkey_read_data(NVID_APP_LE_AUDIO_CONN_FLAG, (uint8_t *)&temp_list[0], &size);

            for (int i = 0; i < old_item_num; i++) {
                app_lea_bond_info_list[i].used = temp_list[i].used;
                app_lea_bond_info_list[i].conn_type = temp_list[i].conn_type;
                app_lea_bond_info_list[i].addr_type = temp_list[i].addr_type;
                memcpy(app_lea_bond_info_list[i].bond_addr, temp_list[i].bond_addr, BT_BD_ADDR_LEN);
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED;
                app_lea_bond_info_list[i].value = 0;
                if (i >= (APP_LEA_MAX_BOND_NUM - 1)) {
                    break;
                }
            }
            app_lea_conn_mgr_print_bond_info(TRUE);
            app_lea_conn_mgr_save_bond_info(TRUE);
            vPortFree(temp_list);
            return;
        } else if (status == NVKEY_STATUS_OK && size > 0 && size != APP_LEA_BOND_INFO_LENGTH && (size % old_item1_len) == 0) {
            uint8_t old_item_num = size / old_item1_len;
            memset(&app_lea_bond_info_list[0], 0, APP_LEA_BOND_INFO_LENGTH);
            app_lea_old_bond_info1_t *temp_list = (app_lea_old_bond_info1_t *)pvPortMalloc(sizeof(app_lea_old_bond_info1_t) * old_item_num);
            if (temp_list == NULL) {
                APPS_LOG_MSGID_E(LOG_TAG" restore_bond_info, malloc fail", 0);
                return;
            }
            nvkey_read_data(NVID_APP_LE_AUDIO_CONN_FLAG, (uint8_t *)&temp_list[0], &size);

            for (int i = 0; i < old_item_num; i++) {
                app_lea_bond_info_list[i].used = temp_list[i].used;
                app_lea_bond_info_list[i].conn_type = temp_list[i].conn_type;
                app_lea_bond_info_list[i].addr_type = temp_list[i].addr_type;
                memcpy(app_lea_bond_info_list[i].bond_addr, temp_list[i].bond_addr, BT_BD_ADDR_LEN);
                app_lea_bond_info_list[i].reconnect_type = temp_list[i].reconnect_type;
                app_lea_bond_info_list[i].value = 0;
                if (i >= (APP_LEA_MAX_BOND_NUM - 1)) {
                    break;
                }
            }
            app_lea_conn_mgr_print_bond_info(TRUE);
            app_lea_conn_mgr_save_bond_info(TRUE);
            vPortFree(temp_list);
            return;
        }
    }
#endif

    uint32_t size = APP_LEA_BOND_INFO_LENGTH;
    nvkey_status_t status = nvkey_read_data(NVID_APP_LE_AUDIO_CONN_FLAG,
                                            (uint8_t *)&app_lea_bond_info_list[0],
                                            &size);
    APPS_LOG_MSGID_I(LOG_TAG" restore_bond_info, read status=%d", 1, status);
    if (status == NVKEY_STATUS_OK) {
        app_lea_conn_mgr_print_bond_info(TRUE);
        for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
            uint8_t reconnect_type = app_lea_bond_info_list[i].reconnect_type;
            // For HW Reset or unexpected crash
            if (reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED
                || reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV
                || reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_TEMP_NO_NEED
                || app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_ULL) {
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED;
            }
#ifdef APP_LE_AUDIO_ADV_CLEAR_LEA_ACTIVE_RECONNECT_TYPE_AFTER_POWER_ON
            if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO) {
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED;
                APPS_LOG_MSGID_E(LOG_TAG" restore_bond_info, [%d] CLEAR_LEA_ACTIVE_RECONNECT_TYPE_AFTER_POWER_ON",
                                 1, i);
            }
#endif
        }
    } else if (status == NVKEY_STATUS_ITEM_NOT_FOUND) {
        memset(&app_lea_bond_info_list[0], 0, APP_LEA_BOND_INFO_LENGTH);
        nvkey_write_data(NVID_APP_LE_AUDIO_CONN_FLAG,
                         (const uint8_t *)&app_lea_bond_info_list[0],
                         APP_LEA_BOND_INFO_LENGTH);
    } else {
        memset(&app_lea_bond_info_list[0], 0, APP_LEA_BOND_INFO_LENGTH);
    }
}

static void app_lea_conn_mgr_clear_lea_bond_info_keep_ull2(bool restore)
{
    if (restore) {
        app_lea_conn_mgr_restore_bond_info();
    } else {
        app_lea_conn_mgr_save_bond_info(FALSE);
    }

    bool found_ull2 = FALSE;
    app_lea_bond_info_t ull2_info = {0};
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_ULL) {
            found_ull2 = TRUE;
            ull2_info = app_lea_bond_info_list[i];
            APPS_LOG_MSGID_E(LOG_TAG" keep_ull2_bond_info", 0);
            break;
        }
    }

    memset(&app_lea_bond_info_list[0], 0, APP_LEA_BOND_INFO_LENGTH);
    if (found_ull2) {
        memcpy(&app_lea_bond_info_list[0], &ull2_info, sizeof(app_lea_bond_info_t));
    }
    app_lea_conn_mgr_save_bond_info(FALSE);
}

#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
static void app_lea_conn_clear_ida_info_list(void)
{
    APPS_LOG_MSGID_I(LOG_TAG" clear_ida_info_list", 0);
    memset(&app_lea_conn_ida_info_list[0], 0, APP_LEA_MAX_IDA_INFO_NUM * sizeof(app_lea_conn_ida_info_item_t));
}

static bool app_lea_conn_compare_ida_info_list(uint8_t *ida, uint8_t *irk)
{
    bool same = FALSE;
    for (int i = 0; i < APP_LEA_MAX_IDA_INFO_NUM; i++) {
        app_lea_conn_ida_info_item_t *item = &app_lea_conn_ida_info_list[i];
        if (memcmp(item->ida, ida, BT_BD_ADDR_LEN) == 0
            && memcmp(item->irk, irk, BT_KEY_SIZE) == 0) {
            same = TRUE;
            break;
        }
    }
    return same;
}

static void app_lea_conn_update_ida_info_list(bool added, uint8_t addr_type, uint8_t *ida, uint8_t *irk)
{
    bool duplicate_ida = FALSE;
    for (int i = 0; i < APP_LEA_MAX_IDA_INFO_NUM; i++) {
        app_lea_conn_ida_info_item_t *item = &app_lea_conn_ida_info_list[i];
        if (memcmp(item->ida, ida, BT_BD_ADDR_LEN) == 0) {
            duplicate_ida = TRUE;
            if (memcmp(item->irk, irk, BT_KEY_SIZE) == 0) {
                item->state = APP_LEA_CONN_IDA_STATE_ADD;
            } else if (!added) {
                item->state = APP_LEA_CONN_IDA_STATE_READY_TO_ADD;
                memcpy(item->irk, irk, BT_KEY_SIZE);
            }
            APPS_LOG_MSGID_W(LOG_TAG" update_ida_info_list, [%d] same_ida added=%d addr=%08X%04X irk=%02X:%02X:%02X",
                             7, i, added, *((uint32_t *)(ida + 2)), *((uint16_t *)ida), irk[0], irk[1], irk[2]);
            break;
        }
    }

    bool is_found = FALSE;
    int added_index = -1;
    if (!duplicate_ida) {
        for (int i = 0; i < APP_LEA_MAX_IDA_INFO_NUM; i++) {
            app_lea_conn_ida_info_item_t *item = &app_lea_conn_ida_info_list[i];
            if (item->state == APP_LEA_CONN_IDA_STATE_NONE) {
                item->addr_type = addr_type;
                memcpy(item->ida, ida, BT_BD_ADDR_LEN);
                memcpy(item->irk, irk, BT_KEY_SIZE);
                item->state = (added ? APP_LEA_CONN_IDA_STATE_ADD : APP_LEA_CONN_IDA_STATE_READY_TO_ADD);
                APPS_LOG_MSGID_W(LOG_TAG" update_ida_info_list, [%d] added=%d addr=%08X%04X irk=%02X:%02X:%02X",
                                 7, i, added, *((uint32_t *)(ida + 2)), *((uint16_t *)ida), irk[0], irk[1], irk[2]);
                is_found = TRUE;
                break;
            } else if (added_index == -1 && item->state == APP_LEA_CONN_IDA_STATE_ADD) {
                added_index = i;
            }
        }

        if (!is_found) {
            APPS_LOG_MSGID_E(LOG_TAG" update_ida_info_list, not found - use [%d] added=%d addr=%08X%04X irk=%02X:%02X:%02X",
                             7, added_index, added, *((uint32_t *)(ida + 2)), *((uint16_t *)ida), irk[0], irk[1], irk[2]);
            if (added_index != -1) {
                app_lea_conn_ida_info_item_t *item = &app_lea_conn_ida_info_list[added_index];
                item->addr_type = addr_type;
                memcpy(item->ida, ida, BT_BD_ADDR_LEN);
                memcpy(item->irk, irk, BT_KEY_SIZE);
                item->state = (added ? APP_LEA_CONN_IDA_STATE_ADD : APP_LEA_CONN_IDA_STATE_READY_TO_ADD);
            }
        }
    }
}

static void app_lea_conn_add_ida_info(uint8_t addr_type, uint8_t *ida, uint8_t *irk)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    uint8_t *local_irk = bt_app_common_get_ble_local_irk();
    bt_hci_cmd_le_add_device_to_resolving_list_t param = {0};
    param.peer_identity_address.type = addr_type;
    memcpy(param.peer_identity_address.addr, ida, BT_BD_ADDR_LEN);
    memcpy(param.peer_irk, irk, BT_KEY_SIZE);
    memcpy(param.local_irk, local_irk, BT_KEY_SIZE);
    bt_status_t bt_status = bt_gap_le_set_resolving_list(BT_GAP_LE_ADD_TO_RESOLVING_LIST, &param);
    APPS_LOG_MSGID_W(LOG_TAG" add_ida_info, [%02X] addr=%08X%04X irk=%02X:%02X:%02X bt_status=0x%08X",
                     8, role, *((uint32_t *)(ida + 2)), *((uint16_t *)ida), irk[0], irk[1], irk[2], bt_status);

    if (bt_status == BT_STATUS_SUCCESS) {
        app_lea_conn_cur_ida_info.addr_type = addr_type;
        memcpy(app_lea_conn_cur_ida_info.ida, ida, BT_BD_ADDR_LEN);
        memcpy(app_lea_conn_cur_ida_info.irk, irk, BT_KEY_SIZE);
        app_lea_conn_cur_ida_info.state = APP_LEA_CONN_IDA_STATE_ADD;

        bt_status = bt_gap_le_set_address_resolution_enable(TRUE);
        if (bt_status != BT_STATUS_SUCCESS) {
            APPS_LOG_MSGID_E(LOG_TAG" add_ida_info, set_address_resolution_enable bt_status=0x%08X", 1, bt_status);
        }

        bt_hci_cmd_le_set_privacy_mode_t param = {0};
        param.privacy_mode = BT_HCI_PRIVACY_MODE_DEVICE;
        param.peer_address.type = addr_type;
        memcpy(param.peer_address.addr, ida, BT_BD_ADDR_LEN);
        if (param.peer_address.type >= BT_ADDR_PUBLIC_IDENTITY) {
            param.peer_address.type = param.peer_address.type - BT_ADDR_PUBLIC_IDENTITY;
        }
        bt_status = bt_gap_le_set_privacy_mode(&param);

        app_lea_conn_mgr_update_bond_info(addr_type, ida, APP_LEA_CONN_TYPE_LE_AUDIO);
        app_lea_conn_update_ida_info_list(TRUE, addr_type, ida, irk);

        uint8_t adv_mode = app_lea_adv_mgr_get_adv_mode();
        APPS_LOG_MSGID_I(LOG_TAG" add_ida_info, [%02X] bt_status=0x%08X adv_mode=%d", 3, role, bt_status, adv_mode);
        if (adv_mode != APP_LEA_ADV_MODE_GENERAL) {
            app_lea_service_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, FALSE, 0);
        }
    }
}

static void app_lea_conn_check_ida_irk(void)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    uint8_t num = 0;
    app_lea_conn_sync_ida_info_t sync_ida_info[APP_LEA_MAX_CONN_NUM] = {0};
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    uint8_t empty_irk[BT_KEY_SIZE] = {0};

    //APPS_LOG_MSGID_W(LOG_TAG" check_ida_irk, [%02X] is_primary=%d", 2, role, app_le_audio_is_primary_earbud());
    if (role != BT_AWS_MCE_ROLE_AGENT && !app_le_audio_is_primary_earbud()) {
        return;
    }

    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (app_lea_conn_info_list[i].conn_handle != BT_HANDLE_INVALID
            && app_lea_conn_info_list[i].peer_type != BT_ADDR_RANDOM
            && app_lea_conn_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO) {
            uint8_t *ida = app_lea_conn_info_list[i].peer_id_addr;
            uint8_t *irk = app_lea_conn_info_list[i].irk;
            bool is_edr_bonded = bt_device_manager_is_paired(ida);
            uint8_t special_pairing = app_lea_conn_info_list[i].special_pairing;

            if (memcmp(ida, empty_addr, BT_BD_ADDR_LEN) == 0
                || memcmp(irk, empty_irk, BT_KEY_SIZE) == 0) {
                continue;
            }

            if (is_edr_bonded || special_pairing != APP_LEA_CONN_SPECIAL_PAIRING_NONE) {
                APPS_LOG_MSGID_I(LOG_TAG" check_ida_irk, [%d] is_edr_bonded=%d special_pairing=%d addr=%08X%04X irk=%02X:%02X:%02X",
                                 8, i, is_edr_bonded, special_pairing, *((uint32_t *)(ida + 2)), *((uint16_t *)ida),
                                 irk[0], irk[1], irk[2]);
                sync_ida_info[num].addr_type = app_lea_conn_info_list[i].peer_type;
                memcpy(sync_ida_info[num].ida, ida, BT_BD_ADDR_LEN);
                memcpy(sync_ida_info[num].irk, irk, BT_KEY_SIZE);
                num++;
            }
        }
    }

    if (num > 0 && aws_link_type != BT_AWS_MCE_SRV_LINK_NONE) {
        bt_status_t bt_status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                                               EVENT_ID_LEA_SYNC_IDA_INFO,
                                                               &sync_ida_info[0],
                                                               sizeof(app_lea_conn_sync_ida_info_t) * APP_LEA_MAX_CONN_NUM);
        APPS_LOG_MSGID_I(LOG_TAG" check_ida_irk, [%02X] send num=%d bt_status=0x%08X",
                         3, role, num, bt_status);
    }
}

static void app_lea_conn_add_rsl_imp(uint8_t addr_type, uint8_t *ida, uint8_t *irk)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_status_t bt_status = bt_gap_le_srv_prepare_set_rsl();
    if (bt_status == BT_STATUS_SUCCESS) {
        app_lea_conn_add_ida_info(addr_type, ida, irk);
    } else if (bt_status == BT_STATUS_BUSY) {
        app_lea_conn_update_ida_info_list(FALSE, addr_type, ida, irk);
        APPS_LOG_MSGID_E(LOG_TAG" handle_peer_ida_info, [%02X] RSL Busy addr=%08X%04X irk=%02X:%02X:%02X",
                         6, role, *((uint32_t *)(ida + 2)), *((uint16_t *)ida), irk[0], irk[1], irk[2]);
    }
}

static void app_lea_conn_handle_peer_ida_info(app_lea_conn_sync_ida_info_t *sync_ida_info)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    uint8_t empty_irk[BT_KEY_SIZE] = {0};

    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        uint8_t addr_type = sync_ida_info[i].addr_type;
        uint8_t *ida = sync_ida_info[i].ida;
        uint8_t *irk = sync_ida_info[i].irk;
        if (memcmp(ida, empty_addr, BT_BD_ADDR_LEN) == 0
            || memcmp(irk, empty_irk, BT_KEY_SIZE) == 0) {
            continue;
        }

        bool is_connected = FALSE;
        bool added_rsl = app_lea_conn_compare_ida_info_list(ida, irk);
        // Still need to add IDA/IRK (maybe new IRK) even if LE_GAP_SRV exist the bond info of the IDA
        for (int j = 0; j < APP_LEA_MAX_CONN_NUM; j++) {
            if (memcmp(ida, app_lea_conn_info_list[j].peer_id_addr, BT_BD_ADDR_LEN) == 0
                || memcmp(ida, app_lea_conn_info_list[j].peer_random_addr, BT_BD_ADDR_LEN) == 0) {
                is_connected = TRUE;
                break;
            }
        }

        if (is_connected || added_rsl) {
            continue;
        } else {
            app_lea_conn_add_rsl_imp(addr_type, ida, irk);
        }
    }
}
#endif

static void app_lea_conn_mgr_le_srv_event_callback(bt_gap_le_srv_event_t event, void *data)
{
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
    if (event == BT_GAP_LE_SRV_EVENT_RSL_SET_PREPARED_COMPLETE_IND) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_ALLOW_SET_RSL);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                            EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_ALLOW_SET_RSL,
                            NULL, 0, NULL, 0);
    }
#endif
}

#ifdef AIR_LE_AUDIO_ENABLE
static void app_lea_conn_mgr_handle_aird_ready(bt_handle_t handle)
{
    uint8_t *addr = app_lea_conn_mgr_get_addr_by_handle(handle);
    if (handle == BT_HANDLE_INVALID || addr == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" handle_aird_ready, handle=0x%04X addr=0x%08X",
                         2, handle, addr);
        return;
    }
    APPS_LOG_MSGID_I(LOG_TAG" handle_aird_ready, handle=0x%04X addr=%08X%04X",
                     3, handle, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));

    bool update_lea_dongle_flag = FALSE;
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO
            && ((app_lea_bond_info_list[i].value & APP_LEA_VALUE_LEA_DONGLE_MASK) == 0)
            && memcmp(addr, app_lea_bond_info_list[i].bond_addr, BT_BD_ADDR_LEN) == 0) {
            update_lea_dongle_flag = TRUE;
            APPS_LOG_MSGID_W(LOG_TAG" update_bond_info, addr=%08X%04X update flag",
                             2, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
            app_lea_bond_info_list[i].value |= APP_LEA_VALUE_LEA_DONGLE_MASK;
            break;
        }
    }

    if (update_lea_dongle_flag) {
        // Only keep one LEA Dongle bond info, remove other LEA dongle
        for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
            if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO
                && ((app_lea_bond_info_list[i].value & APP_LEA_VALUE_LEA_DONGLE_MASK) > 0)
                && memcmp(addr, app_lea_bond_info_list[i].bond_addr, BT_BD_ADDR_LEN) != 0) {
                APPS_LOG_MSGID_W(LOG_TAG" update_bond_info, remove old LEA Dongle addr", 0);
                app_lea_conn_mgr_remove_bond_info(TRUE, app_lea_bond_info_list[i].addr_type,
                                                  app_lea_bond_info_list[i].bond_addr);
                break;
            }
        }

        app_lea_conn_mgr_print_bond_info(TRUE);
        app_lea_conn_mgr_save_bond_info(FALSE);
    }
}

static void app_lea_conn_mgr_start_reconnect_timer(void)
{
    // Cancel all active_reconnect flag (direct adv / targeted announcement flag) after APP_LE_AUDIO_ACTIVE_RECONNECT_TIME
    // Similar to EDR cancel reconnect from headset after power on or link lost TIME_TO_STOP_RECONNECTION
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CANCEL_ADV_RECONNECT_FLAG);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                        EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CANCEL_ADV_RECONNECT_FLAG,
                        NULL, 0, NULL, APP_LE_AUDIO_ACTIVE_RECONNECT_TIME);
}
#endif

#ifdef AIR_LE_AUDIO_DIRECT_ADV
static bool app_lea_conn_mgr_is_check_addr_resolution_feature(uint8_t *addr)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    if (addr == NULL || memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
        return FALSE;
    }

    bool ret = FALSE;
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (memcmp(app_lea_bond_info_list[i].bond_addr, addr, BT_BD_ADDR_LEN) == 0) {
            uint32_t value = app_lea_bond_info_list[i].value;
            uint8_t addr_resolution = (value & APP_LEA_CENTRAL_ADDRESS_RESOLUTION_MASK);
            ret = (addr_resolution != APP_LEA_CENTRAL_ADDRESS_RESOLUTION_UNKNOWN);
            break;
        }
    }
    return ret;
}

static void app_lea_conn_mgr_update_addr_resolution_feature(uint8_t *addr, uint8_t value)
{
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (memcmp(app_lea_bond_info_list[i].bond_addr, addr, BT_BD_ADDR_LEN) == 0) {
            uint32_t old_value = app_lea_bond_info_list[i].value;
            app_lea_bond_info_list[i].value |= ((value == 1) ? APP_LEA_CENTRAL_ADDRESS_RESOLUTION_ENABLE : APP_LEA_CENTRAL_ADDRESS_RESOLUTION_DISABLE);
            if (old_value != app_lea_bond_info_list[i].value) {
                app_lea_conn_mgr_save_bond_info(FALSE);
            }
            break;
        }
    }
}

static bt_status_t app_lea_conn_mgr_client_event_callback(bt_gatt_srv_client_event_t event,
                                                          bt_status_t status, void *parameter)
{
    APPS_LOG_MSGID_I(LOG_TAG" client_event_callback, event=%d bt_status=0x%08X",
                     2, event, status);
    if (event == BT_GATT_SRV_CLIENT_EVENT_READ_VALUE_COMPLTETE && status == BT_STATUS_SUCCESS) {
        bt_gatt_srv_client_read_value_complete_t *read_value = (bt_gatt_srv_client_read_value_complete_t *)parameter;
        APPS_LOG_MSGID_I(LOG_TAG" client_event_callback, type=%d handle=0x%04X length=%d",
                         3, read_value->type, read_value->connection_handle, read_value->length);
        if (read_value->type == BT_GATT_SRV_CLIENT_CENTRAL_ADDRESS_RESOLUTION && read_value->length == 1) {
            uint8_t value = *((uint8_t *)read_value->data);
            uint8_t *addr = app_lea_conn_mgr_get_addr_by_handle(read_value->connection_handle);
            char *cur_task = pcTaskGetName(NULL);
            APPS_LOG_MSGID_W(LOG_TAG" client_event_callback, addr=%08X%04X resolution=%d",
                             3, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), value);
            if (strstr(UI_SHELL_TASK_NAME, cur_task) != 0) {
                app_lea_conn_mgr_update_addr_resolution_feature(addr, value);
            } else {
                uint32_t info = ((read_value->connection_handle << 16) | value);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                    EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_GET_ADDR_RESOLUTION_INFO,
                                    (void *)(int)info, 0, NULL, 0);
            }
        }
    }
    return BT_STATUS_SUCCESS;
}

static void app_lea_conn_mgr_get_address_resolution(bt_handle_t conn_handle)
{
    uint8_t *addr = app_lea_conn_mgr_get_addr_by_handle(conn_handle);
    if (addr == NULL || app_lea_conn_mgr_is_check_addr_resolution_feature(addr)) {
        return;
    }

    bt_gatt_srv_client_action_read_value_t read_param = {0};
    read_param.type = BT_GATT_SRV_CLIENT_CENTRAL_ADDRESS_RESOLUTION;
    read_param.connection_handle = conn_handle;
    read_param.callback = app_lea_conn_mgr_client_event_callback;
    bt_status_t bt_status = bt_gatt_srv_client_send_action(BT_GATT_SRV_CLIENT_ACTION_READ_VALUE,
                                                           &read_param, sizeof(read_param));
    APPS_LOG_MSGID_I(LOG_TAG" get_address_resolution, addr=%08X%04X handle=0x%04X bt_status=0x%08X",
                     4, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), conn_handle, bt_status);
}

#endif



/**================================================================================*/
/**                                 APP Event Handler                              */
/**================================================================================*/
static void app_lea_conn_mgr_bt_cm_event_group(uint32_t event_id, void *extra_data, size_t data_len)
{
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (NULL == remote_update) {
                break;
            }

            if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service) &&
                (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                app_lea_conn_check_ida_irk();
            }
            break;
        }
    }
#endif
}

static void app_lea_conn_mgr_bt_dm_event_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    bt_device_manager_power_event_t evt;
    bt_device_manager_power_status_t status;
    bt_event_get_bt_dm_event_and_status(event_id, &evt, &status);
    switch (evt) {
        case BT_DEVICE_MANAGER_POWER_EVT_ACTIVE_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
                //APPS_LOG_MSGID_I(LOG_TAG" BT_DM Power ON", 0);
                app_lea_conn_mgr_clear_conn_info();
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
                app_lea_conn_clear_ida_info_list();
#endif
            }
            break;
        }

        case BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_RESET_TYPE_NORMAL == status) {
                //APPS_LOG_MSGID_I(LOG_TAG" BT_DM POWER OFF", 0);
                app_lea_conn_mgr_clear_conn_info();
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
                app_lea_conn_clear_ida_info_list();
#endif
                app_lea_conn_mgr_save_bond_info(TRUE);
            }
            break;
        }
    }
}

static void app_lea_conn_mgr_interaction_event_group(uint32_t event_id, void *extra_data, size_t data_len)
{
#ifdef AIR_BT_TAKEOVER_ENABLE
    if (event_id == APPS_EVENTS_INTERACTION_LE_TAKEOVER_ADDR) {
        uint8_t *le_addr = app_bt_takeover_get_disconnect_le_addr();
        if (le_addr != NULL && app_lea_conn_mgr_is_connected(le_addr)) {
            // After takeover service reject new_lea_addr connection, headset should disconnect the LEA.
            APPS_LOG_MSGID_I(LOG_TAG" interaction LE_TAKEOVER_ADDR, disconnect lea_addr for takeover", 0);
            app_lea_service_disconnect(FALSE, APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT, le_addr,
                                       BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
            app_bt_takeover_clear_disconnect_le_addr();
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" interaction LE_TAKEOVER_ADDR, error lea_addr for takeover", 0);
        }
    }
#endif
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
    if (event_id == APPS_EVENTS_INTERACTION_RHO_END || event_id == APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT) {
        app_rho_result_t rho_ret = (app_rho_result_t)extra_data;
        if (APP_RHO_RESULT_SUCCESS == rho_ret) {
            app_lea_conn_clear_ida_info_list();
        }
    }
#endif
}

#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
static void app_lea_conn_mgr_proc_aws_data(void *extra_data, size_t data_len)
{
    uint32_t aws_event_group;
    uint32_t aws_event_id;
    void *p_extra_data = NULL;
    uint32_t extra_data_len = 0;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
    if (aws_data_ind == NULL || aws_data_ind->module_id != BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        return;
    }

    apps_aws_sync_event_decode_extra(aws_data_ind, &aws_event_group, &aws_event_id,
                                     &p_extra_data, &extra_data_len);
    if (aws_event_group == EVENT_GROUP_UI_SHELL_LE_AUDIO && aws_event_id == EVENT_ID_LEA_SYNC_IDA_INFO) {
        app_lea_conn_sync_ida_info_t *sync_ida_info = (app_lea_conn_sync_ida_info_t *)p_extra_data;
        app_lea_conn_handle_peer_ida_info(sync_ida_info);
    }
    if (aws_event_group == EVENT_GROUP_UI_SHELL_LE_AUDIO && aws_event_id == EVENT_ID_LEA_SYNC_RECONNECT_TARGETED_ADDR) {
        uint8_t *addr = (uint8_t *)p_extra_data;
        app_lea_conn_mgr_set_reconnect_targeted_addr(FALSE, addr);
    }
}
#endif

static void app_lea_conn_mgr_lea_event_group(uint32_t event_id, void *extra_data, size_t data_len)
{
#ifdef AIR_LE_AUDIO_ENABLE
#ifdef AIR_TWS_ENABLE
    if (event_id == EVENT_ID_LEA_ALLOW_SET_RSL) {
        bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
        APPS_LOG_MSGID_I(LOG_TAG" lea_event_group, [%02X] ALLOW_SET_RSL", 1, role);
        for (int i = 0; i < APP_LEA_MAX_IDA_INFO_NUM; i++) {
            app_lea_conn_ida_info_item_t *item = &app_lea_conn_ida_info_list[i];
            if (item->state == APP_LEA_CONN_IDA_STATE_READY_TO_ADD) {
                app_lea_conn_add_ida_info(item->addr_type, item->ida, item->irk);
            }
        }
    }
#endif

    if (event_id == EVENT_ID_LEA_HANDLE_AIRD_READY) {
        uint16_t handle = (uint16_t)(int)extra_data;
        app_lea_conn_mgr_handle_aird_ready(handle);
    } else if (event_id == EVENT_ID_LEA_DISCOVERY_INFO) {
        uint32_t info = (uint32_t)(int)extra_data;
        app_lea_conn_mgr_handle_discovery_result(info);
    }
#ifdef AIR_LE_AUDIO_DIRECT_ADV
    else if (event_id == EVENT_ID_LEA_GET_ADDR_RESOLUTION_INFO) {
        uint32_t info = (uint32_t)(int)extra_data;
        bt_handle_t conn_handle = (uint16_t)((info & 0xFFFF0000) >> 16);
        uint8_t value = (uint8_t)(info & 0xFF);
        uint8_t *addr = app_lea_conn_mgr_get_addr_by_handle(conn_handle);
        if (addr == NULL) {
            APPS_LOG_MSGID_E(LOG_TAG" lea_event_group, GET_ADDR_RESOLUTION_INFO addr=NULL", 0);
            return;
        }
        APPS_LOG_MSGID_W(LOG_TAG" lea_event_group, GET_ADDR_RESOLUTION_INFO conn_handle=0x%04X addr=%08X%04X resolution=%d",
                         4, conn_handle, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), value);
        app_lea_conn_mgr_update_addr_resolution_feature(addr, value);
    }
#endif
#endif
}

#ifdef AIR_LE_AUDIO_ENABLE
void app_lea_conn_mgr_notify_aird_ready(bt_handle_t handle)
{
    // Enhance for LEA Dongle, set value bit8 as 1
    APPS_LOG_MSGID_I(LOG_TAG" notify_aird_ready, handle=0x%04X", 1, handle);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_HANDLE_AIRD_READY);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                        EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_HANDLE_AIRD_READY,
                        (void *)(int)handle, 0, NULL, 0);
}

static void app_lea_conn_mgr_handle_discovery_result(uint32_t info)
{
/*
    bt_handle_t conn_handle = (uint16_t)((info & 0xFFFF0000) >> 16);
    uint8_t char_num = (uint8_t)((info & 0xFF00) >> 8);
    bool tbs = (bool)((info & 0x02) > 0);
    bool general = (bool)((info & 0x01) > 0);
//    APPS_LOG_MSGID_E(LOG_TAG" handle_discovery_result, info=%08X conn_handle=0x%02X char_num=%d tbs=%d general=%d",
//                     5, info, conn_handle, char_num, tbs, general);
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (app_lea_conn_info_list[i].conn_handle == conn_handle
            && app_lea_conn_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO) {
            if (tbs) {
                if (general) {
                    app_lea_conn_info_list[i].gtbs_discovery_done = TRUE;
                    app_lea_conn_info_list[i].gtbs_char_num = (char_num > 0);
                } else {
                    app_lea_conn_info_list[i].tbs_discovery_done = TRUE;
                    app_lea_conn_info_list[i].tbs_char_num = (char_num > 0);
                }
            } else {
                if (general) {
                    app_lea_conn_info_list[i].gmcs_discovery_done = TRUE;
                    app_lea_conn_info_list[i].gmcs_char_num = (char_num > 0);
                } else {
                    app_lea_conn_info_list[i].mcs_discovery_done = TRUE;
                    app_lea_conn_info_list[i].mcs_char_num = (char_num > 0);
                }
            }

//            APPS_LOG_MSGID_E(LOG_TAG" handle_discovery_result, %d %d %d %d %d %d %d %d",
//                             8, app_lea_conn_info_list[i].gtbs_discovery_done, app_lea_conn_info_list[i].gtbs_char_num,
//                             app_lea_conn_info_list[i].tbs_discovery_done, app_lea_conn_info_list[i].tbs_char_num,
//                             app_lea_conn_info_list[i].gmcs_discovery_done, app_lea_conn_info_list[i].gmcs_char_num,
//                             app_lea_conn_info_list[i].mcs_discovery_done, app_lea_conn_info_list[i].mcs_char_num);
            if (app_lea_conn_info_list[i].gtbs_discovery_done
                && app_lea_conn_info_list[i].tbs_discovery_done
                && app_lea_conn_info_list[i].gmcs_discovery_done
                && app_lea_conn_info_list[i].mcs_discovery_done
                && app_lea_conn_info_list[i].gtbs_char_num == 0
                && app_lea_conn_info_list[i].tbs_char_num == 0
                && app_lea_conn_info_list[i].gmcs_char_num == 0
                && app_lea_conn_info_list[i].mcs_char_num == 0) {
                uint8_t *addr = NULL;
                if (app_lea_conn_info_list[i].peer_type == BT_ADDR_RANDOM) {
                    addr = app_lea_conn_info_list[i].peer_random_addr;
                } else {
                    addr = app_lea_conn_info_list[i].peer_id_addr;
                }
                APPS_LOG_MSGID_E(LOG_TAG" handle_discovery_result, conn_handle=0x%02X addr=%08X%04X no any service",
                                 3, app_lea_conn_info_list[i].conn_handle,
                                 *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
                app_lea_conn_mgr_remove_bond_info(FALSE, app_lea_conn_info_list[i].peer_type, addr);
            }
            break;
        }
    }
*/
}

void app_lea_conn_mgr_update_discovery_result(bt_handle_t conn_handle, bool tbs, bool general, uint8_t char_num)
{
/*
    uint32_t info = ((conn_handle << 16) | (char_num << 8) | (tbs ? 0x02 : 0) | (general ? 0x01 : 0));
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                        EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_DISCOVERY_INFO,
                        (void *)(int)info, 0, NULL, 0);
*/
}
#endif

static void app_lea_conn_mgr_bt_event_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    apps_bt_event_data_t *bt_event_data = (apps_bt_event_data_t *)extra_data;
    if (bt_event_data == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" BT event, bt_event_data is NULL", 0);
        return;
    }

#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
    bool is_enable_dual_mode = app_lea_service_is_enable_dual_mode();
#endif
    switch (event_id) {
#if defined(AIR_LE_AUDIO_ENABLE) && (defined(AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE) || defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE))
        case BT_GAP_LE_ADVERTISING_SET_TERMINATED_IND: {
            if (!app_lea_service_is_enabled_lea()) {
                break;
            }
            bt_gap_le_advertising_set_terminated_ind_t *ind = (bt_gap_le_advertising_set_terminated_ind_t *)bt_event_data->buffer;
            bt_gap_le_advertising_handle_t adv_handle = 0;
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
            if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_FAST_PAIR, NULL, &adv_handle)
                && adv_handle == ind->handle) {
                app_lea_conn_gfp_standby_handle = ind->connection_handle;
                APPS_LOG_MSGID_I(LOG_TAG"[GFP] ADVERTISING_SET_TERMINATED_IND, GFP adv_handle=%d conn_handle=0x%04X",
                                 2, adv_handle, app_lea_conn_gfp_standby_handle);
            }
#endif
#ifdef APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE
            if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_SWIFT_PAIR, NULL, &adv_handle)
                && adv_handle == ind->handle) {
                app_lea_conn_swift_standby_handle = ind->connection_handle;
                APPS_LOG_MSGID_I(LOG_TAG"[SWIFT] ADVERTISING_SET_TERMINATED_IND, SWIFT adv_handle=%d conn_handle=0x%04X",
                                 2, adv_handle, app_lea_conn_swift_standby_handle);
            }
#endif
            break;
        }
#endif

        case BT_GAP_LE_CONNECT_IND: {
            bt_gap_le_connection_ind_t *conn_ind = (bt_gap_le_connection_ind_t *)bt_event_data->buffer;
            if (conn_ind == NULL || BT_HANDLE_INVALID == conn_ind->connection_handle) {
                break;
            }
#ifdef AIR_SPEAKER_ENABLE
            if (conn_ind->role == BT_ROLE_MASTER) {
                APPS_LOG_MSGID_I(LOG_TAG" BT_LE_CONNECT_IND, master", 0);
                break;
            }
#endif

#ifdef AIR_LE_AUDIO_ENABLE
            bool is_fast_pairing = FALSE;
            bool is_swift_pairing = FALSE;
#endif
            bool need_check = TRUE;
            uint8_t *local_addr = (uint8_t *)conn_ind->local_addr.addr;
            bt_handle_t conn_handle = conn_ind->connection_handle;
            uint8_t *addr = (uint8_t *)conn_ind->peer_addr.addr;
            bt_addr_type_t peer_type = conn_ind->peer_addr.type;
            bt_bd_addr_t adv_addr = {0};

#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
            if (app_lea_conn_gfp_standby_handle == conn_handle) {
                app_lea_conn_gfp_standby_handle = BT_HANDLE_INVALID;
                need_check = FALSE;
                is_fast_pairing = TRUE;
                APPS_LOG_MSGID_W(LOG_TAG"[GFP] BT_LE_CONNECT_IND, GFP conn_handle matched", 0);
            }
#endif
#ifdef APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE
            if (app_lea_conn_swift_standby_handle == conn_handle) {
                app_lea_conn_swift_standby_handle = BT_HANDLE_INVALID;
                need_check = FALSE;
                is_swift_pairing = TRUE;
                APPS_LOG_MSGID_W(LOG_TAG"[SWIFT] BT_LE_CONNECT_IND, SWIFT conn_handle matched", 0);
            }
#endif
#ifdef AIR_LE_AUDIO_DIRECT_ADV
            if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, &adv_addr, NULL)
                && memcmp(&adv_addr, local_addr, BT_BD_ADDR_LEN) == 0) {
                need_check = FALSE;
                APPS_LOG_MSGID_W(LOG_TAG"[DIRECT ADV] BT_LE_CONNECT_IND, conn_handle matched", 0);
            }
#endif

#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
            if (is_enable_dual_mode) {
#if defined(MTK_AWS_MCE_ENABLE) && !defined(MTK_BT_SPEAKER_DISABLE_DOUBLE_LEA_CIS)
                uint8_t *edr_addr = (uint8_t *)bt_device_manager_aws_local_info_get_fixed_address();
#else
                uint8_t *edr_addr = (uint8_t *)bt_device_manager_get_local_address();
#endif
                bool is_public = (memcmp(local_addr, edr_addr, BT_BD_ADDR_LEN) == 0);
                if (need_check && !is_public) {
                    APPS_LOG_MSGID_I(LOG_TAG" BT_LE_CONNECT_IND, not public edr_addr", 0);
                    break;
                }
            } else {
                if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, &adv_addr, NULL)
                    && memcmp(&adv_addr, local_addr, BT_BD_ADDR_LEN) == 0) {
                    // le audio link
                } else if (need_check) {
                    APPS_LOG_MSGID_I(LOG_TAG" BT_LE_CONNECT_IND, not le audio link", 0);
                    break;
                }
            }
#else
            if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, &adv_addr, NULL)
                && memcmp(&adv_addr, local_addr, BT_BD_ADDR_LEN) == 0) {
                // le audio link
            } else if (need_check) {
                APPS_LOG_MSGID_I(LOG_TAG" BT_LE_CONNECT_IND, not le audio link", 0);
                break;
            }
#endif

#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            bool is_ull2_link = bt_gap_le_check_remote_features(conn_handle, BT_GAP_LE_ULL2_0);
            if (!is_ull2_link && app_lea_feature_mode == APP_LEA_FEATURE_MODE_OFF) {
                APPS_LOG_MSGID_E(LOG_TAG" BT_LE_CONNECT_IND, not ULL2 link when APP_LEA_FEATURE_MODE_OFF", 0);
                break;
            }
#endif

            int index = -1;
            for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
                if (app_lea_conn_info_list[i].conn_handle == BT_HANDLE_INVALID) {
                    APPS_LOG_MSGID_I(LOG_TAG" BT_LE_CONNECT_IND, lea_conn_info[%d] handle=0x%04X peer_addr=%08X%04X peer_type=%d",
                                     5, i, conn_handle, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), peer_type);
                    app_lea_conn_info_list[i].conn_handle = conn_handle;
                    if (peer_type == BT_ADDR_RANDOM) {
                        memcpy(app_lea_conn_info_list[i].peer_random_addr, addr, BT_BD_ADDR_LEN);
                        memset(app_lea_conn_info_list[i].peer_id_addr, 0, BT_BD_ADDR_LEN);
                        app_lea_conn_info_list[i].peer_type = BT_ADDR_RANDOM;
                    } else {
                        memset(app_lea_conn_info_list[i].peer_random_addr, 0, BT_BD_ADDR_LEN);
                        memcpy(app_lea_conn_info_list[i].peer_id_addr, addr, BT_BD_ADDR_LEN);
                        app_lea_conn_info_list[i].peer_type = peer_type;
                    }
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                    bool ull2_link = bt_gap_le_check_remote_features(conn_handle, BT_GAP_LE_ULL2_0);
                    extern bool app_lea_adv_add_ull_data;
                    if (ull2_link && app_lea_adv_add_ull_data) {
                        app_lea_conn_info_list[i].conn_type = APP_LEA_CONN_TYPE_LE_ULL;
                    } else {
                        app_lea_conn_info_list[i].conn_type = APP_LEA_CONN_TYPE_LE_AUDIO;
                    }
                    APPS_LOG_MSGID_I(LOG_TAG" BT_LE_CONNECT_IND, ull2_link=%d adv_ull=%d lea_conn_type=%d",
                                     3, ull2_link, app_lea_adv_add_ull_data, app_lea_conn_info_list[i].conn_type);
#elif defined(AIR_LE_AUDIO_ENABLE) && !defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                    app_lea_conn_info_list[i].conn_type = APP_LEA_CONN_TYPE_LE_AUDIO;
#elif !defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                    app_lea_conn_info_list[i].conn_type = APP_LEA_CONN_TYPE_LE_ULL;
#endif
#ifdef AIR_LE_AUDIO_ENABLE
                    app_lea_conn_info_list[i].conn_interval = conn_ind->conn_interval;
                    app_lea_conn_info_list[i].conn_latency = conn_ind->conn_latency;
                    app_lea_conn_info_list[i].supervision_timeout = conn_ind->supervision_timeout;
                    if (is_fast_pairing) {
                        app_lea_conn_info_list[i].special_pairing = APP_LEA_CONN_SPECIAL_PAIRING_GFP;
                    } else if (is_swift_pairing) {
                        app_lea_conn_info_list[i].special_pairing = APP_LEA_CONN_SPECIAL_PAIRING_SWIFT;
                    }
#endif
                    index = i;

                    if (app_lea_connection_cb != NULL) {
                        app_lea_connection_cb(APP_LEA_CONN_CB_TYPE_CONNECTED, index, peer_type, addr, 0x00);

                    }
                    break;
                }
            }
            if (index == -1) {
                APPS_LOG_MSGID_E(LOG_TAG" BT_LE_CONNECT_IND, lea_conn_info full", 0);
            }
            break;
        }

        case BT_GAP_LE_DISCONNECT_IND: {
            bt_hci_evt_disconnect_complete_t *disconn_ind = (bt_hci_evt_disconnect_complete_t *)bt_event_data->buffer;
            if (disconn_ind == NULL || BT_HANDLE_INVALID == disconn_ind->connection_handle) {
                break;
            }

            bt_handle_t conn_handle = disconn_ind->connection_handle;
            APPS_LOG_MSGID_I(LOG_TAG" BT_LE_DISCONNECT_IND, lea_conn_info handle=0x%04X", 1, conn_handle);
            int index = -1;
            for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
                if (app_lea_conn_info_list[i].conn_handle == conn_handle) {
                    APPS_LOG_MSGID_I(LOG_TAG" BT_LE_DISCONNECT_IND, lea_conn_info[%d] handle=0x%04X reason=0x%02X",
                                     3, i, conn_handle, disconn_ind->reason);
                    uint8_t conn_type = app_lea_conn_info_list[i].conn_type;
                    uint8_t peer_type = app_lea_conn_info_list[i].peer_type;
                    uint8_t peer_addr[BT_BD_ADDR_LEN] = {0};
                    bt_hci_disconnect_reason_t reason = disconn_ind->reason;

                    if (peer_type == BT_ADDR_RANDOM) {
                        memcpy(peer_addr, app_lea_conn_info_list[i].peer_random_addr, BT_BD_ADDR_LEN);
                    } else {
                        memcpy(peer_addr, app_lea_conn_info_list[i].peer_id_addr, BT_BD_ADDR_LEN);
                    }
                    memset(&app_lea_conn_info_list[i], 0, sizeof(app_lea_conn_info_t));
                    app_lea_conn_info_list[i].conn_handle = BT_HANDLE_INVALID;

                    if (conn_type == APP_LEA_CONN_TYPE_LE_AUDIO) {
                        if (reason == BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION
                            || reason == BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES
                            || reason == BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST) {
                            // As active disconnect, no need to active reconnect
                            app_lea_conn_mgr_update_reconnect_type(peer_addr, APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED);
#if defined(AIR_BT_INTEL_EVO_ENABLE) && defined(AIR_LE_AUDIO_DIRECT_ADV)
                        } else if (reason == BT_HCI_STATUS_CONNECTION_TIMEOUT) {
                            // As link lost, need to active reconnect (direct adv for Intel EVO)
                            bool check_addr_resolution = app_lea_conn_mgr_is_check_addr_resolution_feature(peer_addr);
                            if (!check_addr_resolution) {
                                APPS_LOG_MSGID_E(LOG_TAG" BT_LE_DISCONNECT_IND, EVO Link Lost but not check addr resolution", 0);
                                app_lea_conn_mgr_update_reconnect_type(peer_addr, APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED);
                            } else {
                                app_lea_conn_mgr_update_reconnect_type(peer_addr, APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV);
                            }
                            app_lea_conn_mgr_start_reconnect_timer();
#endif
                        } else {
                            // Need to active reconnect
                            app_lea_conn_mgr_update_reconnect_type(peer_addr, APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED);
#ifdef AIR_LE_AUDIO_ENABLE
                            app_lea_conn_mgr_start_reconnect_timer();
#endif
                        }
                    } else if (conn_type == APP_LEA_CONN_TYPE_LE_ULL) {
                        app_lea_conn_mgr_update_reconnect_type(peer_addr, APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED);
                    }

                    if (app_lea_connection_cb != NULL) {
                        app_lea_connection_cb(APP_LEA_CONN_CB_TYPE_DISCONNECTED, i, peer_type, peer_addr, disconn_ind->reason);
                    }
                    index = i;

#ifdef AIR_SPEAKER_ENABLE
                    if (app_lea_conn_mgr_get_conn_num() == 0) {
                        ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                            EVENT_ID_LE_AUDIO_DISCONNECT_DONE, NULL, 0, NULL, 0);
                    }
#endif
                    break;
                }
            }
            if (index == -1) {
                APPS_LOG_MSGID_I(LOG_TAG" BT_LE_DISCONNECT_IND, no match item", 0);
            }
            break;
        }

        case BT_GAP_LE_BONDING_COMPLETE_IND: {
            bt_gap_le_bonding_complete_ind_t *ind = (bt_gap_le_bonding_complete_ind_t *)bt_event_data->buffer;
            if (ind == NULL || BT_HANDLE_INVALID == ind->handle) {
                break;
            } else if (bt_event_data->status != BT_STATUS_SUCCESS) {
                APPS_LOG_MSGID_E(LOG_TAG" BT_BONDING_COMPLETE_IND, error status=0x%08X", 1, bt_event_data->status);
                break;
            }

            bt_handle_t conn_handle = ind->handle;
            APPS_LOG_MSGID_I(LOG_TAG" BT_BONDING_COMPLETE_IND, handle=0x%04X", 1, conn_handle);
            for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
                if (app_lea_conn_info_list[i].conn_handle == conn_handle) {
                    uint8_t *addr = app_lea_conn_info_list[i].peer_random_addr;
                    APPS_LOG_MSGID_I(LOG_TAG" BT_BONDING_COMPLETE_IND, lea_conn_info[%d] handle=0x%04X peer_type=%d conn_type=%d",
                                     4, i, conn_handle, app_lea_conn_info_list[i].peer_type, app_lea_conn_info_list[i].conn_type);

                    bt_device_manager_le_bonded_info_t *bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)addr);
                    if (bonded_info != NULL) {
                        uint8_t type = (uint8_t)bonded_info->info.identity_addr.address.type;
                        uint8_t *bd_addr = (uint8_t *)bonded_info->info.identity_addr.address.addr;
                        APPS_LOG_MSGID_I(LOG_TAG" BT_BONDING_COMPLETE_IND, lea_conn_info[%d] type=%d bd_addr=%02X:%02X:%02X:%02X:%02X:%02X",
                                         8, i, type, bd_addr[5], bd_addr[4], bd_addr[3], bd_addr[2], bd_addr[1], bd_addr[0]);

                        uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
                        if (type == 0xFF || APP_LEA_IS_EMPTY_ADDR(bd_addr, empty_addr)) {
                            // Not RPA addr, cannot get IDA
                            bd_addr = app_lea_conn_info_list[i].peer_random_addr;
                            type = app_lea_conn_info_list[i].peer_type;
                            memset(app_lea_conn_info_list[i].peer_id_addr, 0, BT_BD_ADDR_LEN);
                            APPS_LOG_MSGID_I(LOG_TAG" BT_BONDING_COMPLETE_IND, NON-RPA use type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                                             7, type, bd_addr[5], bd_addr[4], bd_addr[3], bd_addr[2], bd_addr[1], bd_addr[0]);
                        } else {
                            memcpy(app_lea_conn_info_list[i].peer_id_addr, bd_addr, BT_BD_ADDR_LEN);
                            app_lea_conn_info_list[i].peer_type = type;
                        }

                        app_lea_conn_mgr_update_bond_info(type, bd_addr, app_lea_conn_info_list[i].conn_type);
                        app_lea_conn_mgr_update_reconnect_type(bd_addr, APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED);
                    } else if (app_lea_conn_info_list[i].peer_type == BT_ADDR_PUBLIC
                               || app_lea_conn_info_list[i].peer_type == BT_ADDR_PUBLIC_IDENTITY
                               || app_lea_conn_info_list[i].peer_type == BT_ADDR_RANDOM_IDENTITY) {
                        app_lea_conn_mgr_update_bond_info(app_lea_conn_info_list[i].peer_type,
                                                          app_lea_conn_info_list[i].peer_id_addr,
                                                          app_lea_conn_info_list[i].conn_type);
                        app_lea_conn_mgr_update_reconnect_type(app_lea_conn_info_list[i].peer_id_addr,
                                                               APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED);
                    }

#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
                    if (app_lea_conn_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO) {
                        multi_ble_adv_manager_switch_le_link_to_another_instance(app_lea_conn_info_list[i].conn_handle, MULTI_ADV_INSTANCE_NOT_RHO);
                    }
#endif

                    if (app_lea_conn_info_list[i].peer_type == BT_ADDR_PUBLIC
                        || app_lea_conn_info_list[i].peer_type == BT_ADDR_PUBLIC_IDENTITY
                        || app_lea_conn_info_list[i].peer_type == BT_ADDR_RANDOM_IDENTITY) {
                        app_lea_connection_cb(APP_LEA_CONN_CB_TYPE_BONDED, i,
                                              app_lea_conn_info_list[i].peer_type, app_lea_conn_info_list[i].peer_id_addr, 0);
                    } else if (app_lea_conn_info_list[i].peer_type == BT_ADDR_RANDOM) {
                        app_lea_connection_cb(APP_LEA_CONN_CB_TYPE_BONDED, i,
                                              BT_ADDR_RANDOM, app_lea_conn_info_list[i].peer_random_addr, 0);
                    }

                    // After takeover service reject new_lea_addr connection, peer headset should reject it when the new_lea_addr connected.
#ifdef AIR_LE_AUDIO_ENABLE
                    bool need_discovery = TRUE;
#endif
#ifdef AIR_BT_TAKEOVER_ENABLE
                    uint8_t *le_addr = app_bt_takeover_get_disconnect_le_addr();
                    if (le_addr != NULL) {
                        if ((memcmp(le_addr, app_lea_conn_info_list[i].peer_random_addr, BT_BD_ADDR_LEN) == 0
                             || memcmp(le_addr, app_lea_conn_info_list[i].peer_id_addr, BT_BD_ADDR_LEN) == 0)
                            && app_lea_conn_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO) {
                            APPS_LOG_MSGID_W(LOG_TAG" BT_BONDING_COMPLETE_IND, disconnect lea_addr for takeover", 0);
                            app_lea_service_disconnect(FALSE, APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT, le_addr,
                                                       BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
                            app_bt_takeover_clear_disconnect_le_addr();
#ifdef AIR_LE_AUDIO_ENABLE
                            need_discovery = FALSE;
#endif
                        }
                    }
#endif

#ifdef AIR_LE_AUDIO_ENABLE
                    if (app_lea_conn_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO) {
#ifdef AIR_LE_AUDIO_DIRECT_ADV
                        app_lea_conn_mgr_get_address_resolution(conn_handle);
#endif
                        if (need_discovery) {
                            bt_status_t bt_status = bt_gattc_discovery_start(BT_GATTC_DISCOVERY_USER_LE_AUDIO,
                                                                             conn_handle, FALSE);

                            if (bt_status != BT_STATUS_SUCCESS) {
                                APPS_LOG_MSGID_E(LOG_TAG" BT_BONDING_COMPLETE_IND, discovery error status=0x%08X",
                                                 1, bt_status);
                            }
                        }
#ifdef AIR_TWS_ENABLE
                        if (app_lea_conn_info_list[i].peer_type == BT_ADDR_PUBLIC
                            || app_lea_conn_info_list[i].peer_type == BT_ADDR_PUBLIC_IDENTITY) {
                            bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)app_lea_conn_info_list[i].peer_id_addr);
                            if (bonded_info != NULL) {
                                uint8_t *irk = (uint8_t *)bonded_info->info.identity_info.irk;
                                memcpy(app_lea_conn_info_list[i].irk, irk, BT_KEY_SIZE);
                            }
                            app_lea_conn_check_ida_irk();
                        }
#endif
                    }
#endif
                    break;
                }
            }
            break;
        }

#ifdef AIR_LE_AUDIO_ENABLE
#ifndef AIR_HEAD_TRACKER_ENABLE
        case BT_GAP_LE_CIS_ESTABLISHED_IND: {
            for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
                if (app_lea_conn_info_list[i].conn_handle != BT_HANDLE_INVALID
                    && app_lea_conn_info_list[i].conn_interval < APP_LEA_NORMAL_CONN_INTERVAL) {
                    bt_hci_cmd_le_connection_update_t param = {0};
                    param.connection_handle =  app_lea_conn_info_list[i].conn_handle;
                    param.conn_interval_min = APP_LEA_NORMAL_CONN_INTERVAL;
                    param.conn_interval_max = APP_LEA_NORMAL_CONN_INTERVAL;
                    param.conn_latency = app_lea_conn_info_list[i].conn_latency;
                    param.supervision_timeout = app_lea_conn_info_list[i].supervision_timeout;
                    bt_status_t bt_status = bt_gap_le_update_connection_parameter(&param);
                    APPS_LOG_MSGID_I(LOG_TAG" LEA update connection interval, handle=0x%04X conn_interval=0x%04X->0x%04X bt_status=0x%08X",
                                     4, app_lea_conn_info_list[i].conn_handle, app_lea_conn_info_list[i].conn_interval, APP_LEA_NORMAL_CONN_INTERVAL, bt_status);
                    if (bt_status == BT_STATUS_SUCCESS) {
                        app_lea_conn_info_list[i].conn_interval = APP_LEA_NORMAL_CONN_INTERVAL;
                    }
                    // No need to break, update all LEA low connection interval when CIS establish
                }
            }
            break;
        }
#endif
        case BT_GAP_LE_CONNECTION_UPDATE_IND: {
            bt_gap_le_connection_update_ind_t *ind = (bt_gap_le_connection_update_ind_t *)bt_event_data->buffer;
            if (ind == NULL || BT_HANDLE_INVALID == ind->conn_handle) {
                break;
            } else if (bt_event_data->status != BT_STATUS_SUCCESS) {
                APPS_LOG_MSGID_E(LOG_TAG" BT_GAP_LE_CONNECTION_UPDATE_IND, error status=0x%08X", 1, bt_event_data->status);
                break;
            }

            for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
                if (app_lea_conn_info_list[i].conn_handle == ind->conn_handle) {
                    app_lea_conn_info_list[i].conn_interval = ind->conn_interval;
                    app_lea_conn_info_list[i].conn_latency = ind->conn_latency;
                    app_lea_conn_info_list[i].supervision_timeout = ind->supervision_timeout;
                    APPS_LOG_MSGID_I(LOG_TAG" BT_GAP_LE_CONNECTION_UPDATE_IND, handle=0x%04X conn_interval=0x%04X",
                                     2, ind->conn_handle, ind->conn_interval);
                    break;
                }
            }
            break;
        }

#ifdef AIR_TWS_ENABLE
        case BT_GAP_LE_SET_RESOLVING_LIST_CNF: {
            if (bt_event_data->status == BT_HCI_STATUS_MEMORY_CAPACITY_EXCEEDED) {
                APPS_LOG_MSGID_E(LOG_TAG" SET_RESOLVING_LIST_CNF, MEMORY_CAPACITY_EXCEEDED", 0);
                extern bt_status_t bt_device_manager_le_remove_old_bonded_device(void);
                bt_device_manager_le_remove_old_bonded_device();
                app_lea_conn_add_rsl_imp(app_lea_conn_cur_ida_info.addr_type,
                                         app_lea_conn_cur_ida_info.ida,
                                         app_lea_conn_cur_ida_info.irk);
            } else {
                memset(&app_lea_conn_cur_ida_info, 0, sizeof(app_lea_conn_ida_info_item_t));
            }
            break;
        }
#endif
#endif
    }
}



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
void app_lea_conn_mgr_register_connection_cb(app_lea_conn_mgr_connection_cb_t cb)
{
    app_lea_connection_cb = cb;
}

bool app_lea_conn_mgr_is_ever_connected(void)
{
    bool ret = FALSE;
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (app_lea_bond_info_list[i].used) {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

bool app_lea_conn_mgr_is_connected(const uint8_t *addr)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    if (addr == NULL || memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
        return FALSE;
    }

    bool ret = FALSE;
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (app_lea_conn_info_list[i].conn_handle != BT_HANDLE_INVALID
            && (memcmp(app_lea_conn_info_list[i].peer_random_addr, addr, BT_BD_ADDR_LEN) == 0
                || memcmp(app_lea_conn_info_list[i].peer_id_addr, addr, BT_BD_ADDR_LEN) == 0)) {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

bool app_lea_conn_mgr_is_le_bond(const uint8_t *addr)
{
    bt_device_manager_le_bonded_info_t *bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)addr);
    return (bonded_info != NULL);
}

bool app_lea_conn_mgr_get_ida(const uint8_t *random_addr, uint8_t *type, uint8_t *id_addr)
{
    bool ret = FALSE;
    if (type == NULL || id_addr == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" get_ida, error type=0x%08X id_addr=0x%08X", 2, type, id_addr);
        return ret;
    }

    bt_device_manager_le_bonded_info_t *bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)random_addr);
    if (bonded_info != NULL) {
        *type = (uint8_t)bonded_info->info.identity_addr.address.type;
        uint8_t *bd_addr = (uint8_t *)bonded_info->info.identity_addr.address.addr;
        memcpy(id_addr, bd_addr, BT_BD_ADDR_LEN);
        ret = TRUE;
    }

    return ret;
}

bt_handle_t app_lea_conn_mgr_get_handle(uint8_t index)
{
    return (index < APP_LEA_MAX_CONN_NUM ? app_lea_conn_info_list[index].conn_handle : BT_HANDLE_INVALID);
}

bt_handle_t app_lea_conn_mgr_get_dongle_handle(app_lea_conn_type_t type)
{
    bt_handle_t handle = BT_HANDLE_INVALID;
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (app_lea_conn_info_list[i].conn_handle != BT_HANDLE_INVALID
            && app_lea_conn_info_list[i].conn_type == type) {
#ifdef AIR_LE_AUDIO_ENABLE
            if (type == APP_LEA_CONN_TYPE_LE_AUDIO
                && app_le_audio_aird_client_is_support(app_lea_conn_info_list[i].conn_handle)) {
                handle = app_lea_conn_info_list[i].conn_handle;
                break;
            }
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
            if (type == APP_LEA_CONN_TYPE_LE_ULL) {
                handle = app_lea_conn_info_list[i].conn_handle;
                break;
            }
#endif
        }
    }
    return handle;
}

bt_handle_t app_lea_conn_mgr_get_handle_by_addr(const uint8_t *addr)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    if (addr == NULL || memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
        return BT_HANDLE_INVALID;
    }

    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if ((memcmp(addr, app_lea_conn_info_list[i].peer_random_addr, BT_BD_ADDR_LEN) == 0) ||
            (memcmp(addr, app_lea_conn_info_list[i].peer_id_addr, BT_BD_ADDR_LEN) == 0)) {
            return app_lea_conn_info_list[i].conn_handle;
        }
    }
    return BT_HANDLE_INVALID;
}

uint8_t *app_lea_conn_mgr_get_addr_by_handle(bt_handle_t handle)
{
    uint8_t *addr = NULL;
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (app_lea_conn_info_list[i].conn_handle == handle && handle != BT_HANDLE_INVALID) {
            if (app_lea_conn_info_list[i].peer_type == BT_ADDR_RANDOM) {
                addr = app_lea_conn_info_list[i].peer_random_addr;
            } else {
                addr = app_lea_conn_info_list[i].peer_id_addr;
            }
            break;
        }
    }
    return addr;
}

uint8_t *app_lea_conn_mgr_get_unify_addr(const uint8_t *addr)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    if (addr == NULL || memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
        return NULL;
    }

    uint8_t *unify_addr = NULL;
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (memcmp(app_lea_conn_info_list[i].peer_random_addr, addr, BT_BD_ADDR_LEN) == 0
            || memcmp(app_lea_conn_info_list[i].peer_id_addr, addr, BT_BD_ADDR_LEN) == 0) {
            if (app_lea_conn_info_list[i].peer_type == BT_ADDR_RANDOM) {
                unify_addr = app_lea_conn_info_list[i].peer_random_addr;
            } else {
                unify_addr = app_lea_conn_info_list[i].peer_id_addr;
            }
            break;
        }
    }
    return unify_addr;
}

bool app_lea_conn_mgr_is_bond_link(const uint8_t *addr)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    if (addr == NULL || memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
        return FALSE;
    }

    bool ret = FALSE;
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (memcmp(addr, app_lea_bond_info_list[i].bond_addr, BT_BD_ADDR_LEN) == 0) {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

bool app_lea_conn_mgr_is_ull_link(const uint8_t *addr)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    if (addr == NULL || memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
        return FALSE;
    }

    bool ret = FALSE;
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if ((memcmp(addr, app_lea_conn_info_list[i].peer_random_addr, BT_BD_ADDR_LEN) == 0) ||
            (memcmp(addr, app_lea_conn_info_list[i].peer_id_addr, BT_BD_ADDR_LEN) == 0)) {
            ret = (app_lea_conn_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_ULL);
            break;
        }
    }
#endif
    return ret;
}

uint8_t* app_lea_conn_mgr_get_ull_dongle_addr(void)
{
    uint8_t *addr = NULL;
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (app_lea_bond_info_list[i].used
            && app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_ULL) {
            addr = app_lea_bond_info_list[i].bond_addr;
            break;
        }
    }
    return addr;
}

bool app_lea_conn_mgr_is_lea_dongle_link(const uint8_t *addr)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    if (addr == NULL || memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
        return FALSE;
    }

    bool ret = FALSE;
#ifdef AIR_LE_AUDIO_ENABLE
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO
            && memcmp(app_lea_bond_info_list[i].bond_addr, addr, BT_BD_ADDR_LEN) == 0
            && ((app_lea_bond_info_list[i].value & APP_LEA_VALUE_LEA_DONGLE_MASK) > 0)) {
            ret = TRUE;
            break;
        }
    }
#endif
    return ret;
}

uint8_t *app_lea_conn_mgr_get_lea_dongle_addr(void)
{
    uint8_t *addr = NULL;
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO
            && ((app_lea_bond_info_list[i].value & APP_LEA_VALUE_LEA_DONGLE_MASK) > 0)
            && memcmp(app_lea_bond_info_list[i].bond_addr, empty_addr, BT_BD_ADDR_LEN) != 0) {
            addr = app_lea_bond_info_list[i].bond_addr;
            break;
        }
    }
    return addr;
}

void app_lea_conn_mgr_reset_lea_dongle(void)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_AUDIO
            && ((app_lea_bond_info_list[i].value & APP_LEA_VALUE_LEA_DONGLE_MASK) > 0)
            && memcmp(app_lea_bond_info_list[i].bond_addr, empty_addr, BT_BD_ADDR_LEN) != 0) {
            uint8_t *addr = app_lea_bond_info_list[i].bond_addr;
            APPS_LOG_MSGID_W(LOG_TAG" reset_lea_dongle, addr=%08X%04X",
                             2, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
            app_lea_conn_mgr_remove_bond_info(TRUE, app_lea_bond_info_list[i].addr_type,
                                              app_lea_bond_info_list[i].bond_addr);
            break;
        }
    }
}

uint8_t app_lea_conn_mgr_get_conn_type(bt_handle_t handle)
{
    if (handle == BT_HANDLE_INVALID) {
        return APP_LEA_CONN_TYPE_NONE;
    }

    uint8_t conn_type = APP_LEA_CONN_TYPE_NONE;
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (app_lea_conn_info_list[i].conn_handle == handle) {
            conn_type = app_lea_conn_info_list[i].conn_type;
            break;
        }
    }
    return conn_type;
}

uint8_t app_lea_conn_mgr_get_conn_type_by_addr(const uint8_t *addr)
{
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    if (addr == NULL || memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0) {
        return APP_LEA_CONN_TYPE_NONE;
    }

    bt_handle_t handle = app_lea_conn_mgr_get_handle_by_addr(addr);
    uint8_t conn_type = app_lea_conn_mgr_get_conn_type(handle);
    if (conn_type == APP_LEA_CONN_TYPE_NONE) {
        for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
            const uint8_t *bond_addr = app_lea_bond_info_list[i].bond_addr;
            if (memcmp(addr, bond_addr, BT_BD_ADDR_LEN) == 0) {
                conn_type = app_lea_bond_info_list[i].conn_type;
                break;
            }
        }
    }
    return conn_type;
}

uint8_t app_lea_conn_mgr_get_index(bt_handle_t handle)
{
    int i = 0;
    for (i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (app_lea_conn_info_list[i].conn_handle == handle && handle != BT_HANDLE_INVALID) {
            break;
        }
    }
    return i;
}

uint8_t app_lea_conn_mgr_get_conn_num(void)
{
    return app_lea_conn_mgr_get_link_count();
}

void app_lea_conn_mgr_get_conn_info(uint8_t *num, bt_addr_t *bdaddr)
{
    if (num == NULL || bdaddr == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" get_conn_info, error num=0x%08X bdaddr=0x%08X", 2, num, bdaddr);
        return;
    }

    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    int conn_num = app_lea_conn_mgr_get_conn_num();
    if (conn_num == 0) {
        //APPS_LOG_MSGID_I(LOG_TAG" get_conn_info, num=0", 0);
        return;
    }

    int index = 0;
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (app_lea_conn_info_list[i].conn_handle != BT_HANDLE_INVALID) {
            uint8_t peer_type = app_lea_conn_info_list[i].peer_type;
            const uint8_t *peer_addr = NULL;
            if (peer_type == BT_ADDR_RANDOM) {
                peer_addr = app_lea_conn_info_list[i].peer_random_addr;
            } else {
                peer_addr = app_lea_conn_info_list[i].peer_id_addr;
            }

            if (!APP_LEA_IS_EMPTY_ADDR(peer_addr, empty_addr)) {
                bdaddr[index].type = peer_type;
                memcpy(bdaddr[index].addr, peer_addr, BT_BD_ADDR_LEN);
                index++;
                *num = index;

                APPS_LOG_MSGID_I(LOG_TAG" get_conn_info, lea_num=%d addr_type=%d %02X:%02X:%02X:%02X:%02X:%02X",
                                 8, *num, peer_type,
                                 peer_addr[5], peer_addr[4], peer_addr[3], peer_addr[2], peer_addr[1], peer_addr[0]);
                if (index == APP_LEA_MAX_CONN_NUM) {
                    break;
                }
            }
        }
    }
}

uint8_t app_lea_conn_mgr_get_support_max_conn_num(void)
{
#if !defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    return 1;
#else
    bool support_emp = FALSE;
#ifdef AIR_MULTI_POINT_ENABLE
    support_emp = app_bt_emp_is_enable();
#endif

    if (support_emp) {
        return APP_LEA_MAX_CONN_NUM;
    } else {
#ifdef AIR_BT_TAKEOVER_ENABLE
        return 1 + 1;
#else
        return 1;
#endif
    }
#endif
}

app_lea_bond_info_t *app_lea_conn_mgr_get_bond_info(void)
{
    return &app_lea_bond_info_list[0];
}

uint8_t app_lea_conn_mgr_get_bond_num(void)
{
    uint8_t bond_num = 0;
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        const uint8_t *addr = app_lea_bond_info_list[i].bond_addr;
        if (app_lea_bond_info_list[i].used && !APP_LEA_IS_EMPTY_ADDR(addr, empty_addr)) {
            bond_num++;
        }
    }
    return bond_num;
}

void app_lea_conn_mgr_reset_bond_info(void)
{
    APPS_LOG_MSGID_I(LOG_TAG" reset_bond_info", 0);
    memset(&app_lea_bond_info_list[0], 0, APP_LEA_BOND_INFO_LENGTH);
    app_lea_conn_mgr_save_bond_info(FALSE);
}

bool app_lea_conn_mgr_add_bond_info(uint8_t addr_type, uint8_t *addr, uint8_t conn_type)
{
    APPS_LOG_MSGID_W(LOG_TAG" add_bond_info, addr=%d-%02X:%02X:%02X:%02X:%02X:%02X conn_type=%d",
                     8, addr_type, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], conn_type);
    return app_lea_conn_mgr_update_bond_info(addr_type, addr, conn_type);
}

bool app_lea_conn_mgr_remove_bond_info(bool disconnect, uint8_t addr_type, uint8_t *addr)
{
    bool ret = FALSE;
    int index = -1;
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};

    if (memcmp(addr, empty_addr, BT_BD_ADDR_LEN) == 0 || addr_type > BT_ADDR_RANDOM_IDENTITY) {
        return FALSE;
    }

    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        const uint8_t *bond_addr = app_lea_bond_info_list[i].bond_addr;
        uint8_t conn_type = app_lea_bond_info_list[i].conn_type;
        if (memcmp(addr, bond_addr, BT_BD_ADDR_LEN) == 0
            && ((conn_type == APP_LEA_CONN_TYPE_LE_ULL && addr_type == app_lea_bond_info_list[i].addr_type)
                || conn_type == APP_LEA_CONN_TYPE_LE_AUDIO)) {
            // Need to consider ULL/BTA Dongle with own public addr
            ret = TRUE;
            app_lea_conn_mgr_print_bond_info(FALSE);
            index = i;
            APPS_LOG_MSGID_W(LOG_TAG" remove_bond_info, disconnect=%d addr_type=%d addr=%08X%04X",
                             4, disconnect, addr_type, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));

            if (disconnect) {
                app_lea_conn_mgr_disconnect(addr, BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
            }

            app_lea_conn_mgr_remove_le_bond_info(addr_type, addr);
            break;
        }
    }

    if (ret && index >= 0) {
        for (int i = index; i <= APP_LEA_MAX_BOND_NUM - 1 - 1; i++) {
            memcpy(&app_lea_bond_info_list[i], &app_lea_bond_info_list[i + 1], sizeof(app_lea_bond_info_t));
        }
        memset(&app_lea_bond_info_list[APP_LEA_MAX_BOND_NUM - 1], 0, sizeof(app_lea_bond_info_t));
        app_lea_conn_mgr_print_bond_info(TRUE);

        app_lea_service_refresh_advertising(0);
    }

    return ret;
}

void app_lea_conn_mgr_reset_keep_ull2_bond_info(void)
{
    APPS_LOG_MSGID_E(LOG_TAG" reset_keep_ull2_bond_info", 0);
    app_lea_conn_mgr_clear_lea_bond_info_keep_ull2(TRUE);
}

void app_lea_conn_mgr_clear_lea_keep_ull2_bond_info(void)
{
    APPS_LOG_MSGID_E(LOG_TAG" clear_lea_keep_ull2_bond_info", 0);
    app_lea_conn_mgr_clear_lea_bond_info_keep_ull2(FALSE);
}

bool app_lea_conn_mgr_is_need_reconnect_adv(void)
{
    bool need = FALSE;
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (!app_lea_bond_info_list[i].used || (memcmp(app_lea_bond_info_list[i].bond_addr, empty_addr, BT_BD_ADDR_LEN) == 0)) {
            continue;
        }

        if (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV
            || app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED
            || app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED
            || app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_TEMP_NO_NEED) {
            need = TRUE;
            break;
        }
    }
    return need;
}

void app_lea_conn_mgr_get_reconnect_info(bool adjust, uint8_t *direct_num, uint8_t *active_num, uint8_t *inactive_num)
{
    if (adjust) {
        app_lea_conn_mgr_adjust_reconnect_type();
    }

    uint8_t temp_direct_num = 0;
    uint8_t temp_active_num = 0;
    uint8_t temp_inactive_num = 0;
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (!app_lea_bond_info_list[i].used || (memcmp(app_lea_bond_info_list[i].bond_addr, empty_addr, BT_BD_ADDR_LEN) == 0)) {
            continue;
        }

        if (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV) {
            temp_direct_num++;
        } else if (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED) {
            temp_active_num++;
        } else if (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED
                   || app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_TEMP_NO_NEED) {
            temp_inactive_num++;
        }
    }
    if (direct_num != NULL) {
        *direct_num = temp_direct_num;
    }
    if (active_num != NULL) {
        *active_num = temp_active_num;
    }
    if (inactive_num != NULL) {
        *inactive_num = temp_inactive_num;
    }

    if (adjust) {
        APPS_LOG_MSGID_W(LOG_TAG"[SUB_MODE] get_reconnect_info, direct_num=%d active_num=%d inactive_num=%d",
                         3, temp_direct_num, temp_active_num, temp_inactive_num);
    }
}

void app_lea_conn_mgr_get_reconnect_addr(uint8_t sub_mode, bt_addr_t *addr_list, uint8_t *list_num)
{
    if (addr_list == NULL || list_num == NULL || *list_num == 0) {
        return;
    }

    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    int buffer_num = *list_num;
    int index = 0;
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (!app_lea_bond_info_list[i].used || (memcmp(app_lea_bond_info_list[i].bond_addr, empty_addr, BT_BD_ADDR_LEN) == 0)) {
            continue;
        }

        bool match = FALSE;
        if (sub_mode == APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT
            && (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED
                || (app_lea_bond_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_ULL && app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_NO_NEED))) {
            match = TRUE;
        } else if (sub_mode == APP_LEA_ADV_SUB_MODE_INACTIVE
                   && app_lea_bond_info_list[i].reconnect_type != APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED) {
            match = TRUE;
        } else if (sub_mode == APP_LEA_ADV_SUB_MODE_DIRECT
                   && app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV) {
            match = TRUE;
        }

        if (match) {
            memcpy(addr_list[index].addr, app_lea_bond_info_list[i].bond_addr, BT_BD_ADDR_LEN);
            addr_list[index].type = app_lea_bond_info_list[i].addr_type;
            index++;
            if (index > buffer_num) {
                break;
            }
        }
    }

    *list_num = index;
}

bool app_lea_conn_mgr_is_support_addr_resolution(uint8_t *addr)
{
    bool ret = FALSE;
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (memcmp(app_lea_bond_info_list[i].bond_addr, addr, BT_BD_ADDR_LEN) == 0) {
            uint32_t value = app_lea_bond_info_list[i].value;
            uint8_t addr_resolution = (value & APP_LEA_CENTRAL_ADDRESS_RESOLUTION_MASK);
            ret = (addr_resolution == APP_LEA_CENTRAL_ADDRESS_RESOLUTION_ENABLE);
            break;
        }
    }
    return ret;
}

void app_lea_conn_mgr_disconnect(const uint8_t *addr, uint8_t reason)
{
    if (addr != NULL) {
        APPS_LOG_MSGID_I(LOG_TAG" disconnect, addr=%02X:%02X:%02X:%02X:%02X:%02X reason=0x%02X",
                         7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], reason);
        uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
        if (APP_LEA_IS_EMPTY_ADDR(addr, empty_addr)) {
            //APPS_LOG_MSGID_E(LOG_TAG" disconnect, empty addr", 0);
            return;
        }

        for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
            if (memcmp(addr, app_lea_conn_info_list[i].peer_id_addr, BT_BD_ADDR_LEN) == 0
                || memcmp(addr, app_lea_conn_info_list[i].peer_random_addr, BT_BD_ADDR_LEN) == 0) {
                app_lea_conn_mgr_disconnect_by_handle(app_lea_conn_info_list[i].conn_handle,
                                                      reason);
                break;
            }
        }
    } else {
        APPS_LOG_MSGID_I(LOG_TAG" disconnect all, reason=0x%02X", 1, reason);
        for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
            if (BT_HANDLE_INVALID != app_lea_conn_info_list[i].conn_handle) {
                app_lea_conn_mgr_disconnect_by_handle(app_lea_conn_info_list[i].conn_handle,
                                                      reason);
            }
        }
    }
}

bool app_lea_conn_mgr_disconnect_by_handle(bt_handle_t handle,
                                           bt_hci_disconnect_reason_t reason)
{
    bt_hci_cmd_disconnect_t disconnect_para;
    disconnect_para.connection_handle = handle;
    disconnect_para.reason = reason;

    bt_status_t bt_status = bt_gap_le_disconnect(&disconnect_para);
    APPS_LOG_MSGID_I(LOG_TAG" disconnect_by_handle, handle=0x%04X reason=0x%02X bt_status=0x%08X",
                     3, handle, reason, bt_status);
    return (bt_status == BT_STATUS_SUCCESS);
}

void app_lea_conn_mgr_disconnect_dongle(void)
{
    uint8_t disconnect_num = 0;
    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        if (BT_HANDLE_INVALID != app_lea_conn_info_list[i].conn_handle) {
            bool is_dongle = FALSE;
#ifdef AIR_LE_AUDIO_ENABLE
            if (app_le_audio_aird_client_is_support(app_lea_conn_info_list[i].conn_handle)) {
                is_dongle = TRUE;
            }
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
            if (app_lea_conn_info_list[i].conn_type == APP_LEA_CONN_TYPE_LE_ULL) {
                is_dongle = TRUE;
            }
#endif
            if (is_dongle) {
                // Use BT_LOCAL_HOST for Headset wired_audio & LE Audio.
                // Dongle will reconnect headset successfully when remove wired_audio and headset restart ADV
                // Use low_resource, the dongle cannot reconnect Headset again.
                app_lea_conn_mgr_disconnect_by_handle(app_lea_conn_info_list[i].conn_handle,
                                                      BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST);
                disconnect_num++;
            }
        }
    }
    APPS_LOG_MSGID_I(LOG_TAG" disconnect_dongle, num=%d", 1, disconnect_num);
}

void app_lea_conn_mgr_set_reconnect_targeted_addr(bool sync, const uint8_t *addr)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bool already_connected = FALSE;
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (app_lea_bond_info_list[i].used
            && memcmp(addr, app_lea_bond_info_list[i].bond_addr, BT_BD_ADDR_LEN) == 0) {
            already_connected = (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_CONNECTED);
            break;
        }
    }
    // Update reconnect mode, should be on the LEA Bond List, start adv with targeted announcement flag
    if (!already_connected) {
        app_lea_conn_mgr_update_reconnect_type(addr, APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED);
#ifdef AIR_LE_AUDIO_ENABLE
        app_lea_conn_mgr_start_reconnect_timer();
#endif
    }

#ifdef AIR_TWS_ENABLE
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    if (sync && aws_link_type != BT_AWS_MCE_SRV_LINK_NONE) {
        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                       EVENT_ID_LEA_SYNC_RECONNECT_TARGETED_ADDR,
                                       addr, BT_BD_ADDR_LEN);
    }
    APPS_LOG_MSGID_I(LOG_TAG" set_targeted_addr, [%02X] aws_link_type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     8, role, aws_link_type, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
#else
    APPS_LOG_MSGID_I(LOG_TAG" set_targeted_addr, [%02X] addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     7, role, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
#endif
}

void app_lea_conn_mgr_control_temp_reconnect_type(bool enable)
{
    app_lea_conn_mgr_print_bond_info(FALSE);

    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    uint8_t update_num = 0;
    for (int i = 0; i < APP_LEA_MAX_BOND_NUM; i++) {
        if (!app_lea_bond_info_list[i].used || (memcmp(app_lea_bond_info_list[i].bond_addr, empty_addr, BT_BD_ADDR_LEN) == 0)) {
            continue;
        }

        if (enable) {
            if (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_TEMP_NO_NEED) {
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED;
                update_num++;
            }
        } else {
            if (app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DIRECT_ADV
                || app_lea_bond_info_list[i].reconnect_type == APP_LEA_CONN_ACTIVE_RECONNECT_DEFAULT_NEED) {
                app_lea_bond_info_list[i].reconnect_type = APP_LEA_CONN_ACTIVE_RECONNECT_TEMP_NO_NEED;
                update_num++;
            }
        }
    }

    APPS_LOG_MSGID_W(LOG_TAG" control_temp_reconnect_type, enable=%d update_num=%d", 2, enable, update_num);
    if (update_num > 0) {
        app_lea_conn_mgr_print_bond_info(TRUE);
        app_lea_service_refresh_advertising(0);
    }
}

void app_lea_conn_mgr_enable_multi_conn(bool enable)
{
#ifdef AIR_LE_AUDIO_MULTIPOINT_ENABLE
    if (enable) {
        multi_ble_adv_manager_set_le_connection_max_count(MULTI_ADV_INSTANCE_NOT_RHO, APP_LEA_MAX_CONN_NUM);
    } else {
        // Add more one for takeover when EMP OFF
        multi_ble_adv_manager_set_le_connection_max_count(MULTI_ADV_INSTANCE_NOT_RHO, 1 + 1);
    }
#elif !defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    if (enable) {
        APPS_LOG_MSGID_E(LOG_TAG" enable_multi_conn, only support ULL2", 0);
    }
    multi_ble_adv_manager_set_le_connection_max_count(MULTI_ADV_INSTANCE_NOT_RHO, 1);
#else
    if (enable) {
        APPS_LOG_MSGID_E(LOG_TAG" enable_multi_conn, cannot support multi point feature", 0);
    }
    multi_ble_adv_manager_set_le_connection_max_count(MULTI_ADV_INSTANCE_NOT_RHO, 1);
#endif
    APPS_LOG_MSGID_I(LOG_TAG" enable_multi_conn, enable=%d", 1, enable);
}

void app_lea_conn_mgr_init(void)
{
    app_lea_conn_mgr_clear_conn_info();

    app_lea_conn_mgr_restore_bond_info();
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
    app_lea_conn_gfp_standby_handle = BT_HANDLE_INVALID;
#endif
#ifdef APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE
    app_lea_conn_swift_standby_handle = BT_HANDLE_INVALID;
#endif
    bt_gap_le_srv_register_event_callback(app_lea_conn_mgr_le_srv_event_callback);
}

void app_lea_conn_mgr_proc_ui_shell_event(uint32_t event_group,
                                          uint32_t event_id,
                                          void *extra_data,
                                          size_t data_len)
{
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            app_lea_conn_mgr_bt_cm_event_group(event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER:
            app_lea_conn_mgr_bt_dm_event_group(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            app_lea_conn_mgr_interaction_event_group(event_id, extra_data, data_len);
            break;
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_TWS_ENABLE)
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            app_lea_conn_mgr_proc_aws_data(extra_data, data_len);
            break;
#endif
        case EVENT_GROUP_UI_SHELL_LE_AUDIO: {
            app_lea_conn_mgr_lea_event_group(event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT:
            app_lea_conn_mgr_bt_event_group(event_id, extra_data, data_len);
            break;
    }
}

#endif /* AIR_LE_AUDIO_ENABLE */
