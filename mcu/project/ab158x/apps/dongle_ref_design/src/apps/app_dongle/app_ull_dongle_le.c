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

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "bt_ull_service.h"
#include "bt_ull_le_service.h"
#include "bt_ull_le_utility.h"
#include "app_dongle_service.h"
#include "race_cmd_feature.h"
#include "bt_system.h"
#include "bt_type.h"
#include "bt_hci.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "apps_events_event_group.h"
#include "app_ull_dongle_le.h"
#include "apps_race_cmd_event.h"
#include "apps_debug.h"
#include "app_le_audio_air.h"
#include "apps_customer_config.h"
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
#include "app_power_save_utils.h"
#endif
#if defined (MTK_RACE_CMD_ENABLE) && defined (AIR_WIRELESS_MIC_ENABLE)
#include "app_dongle_race.h"
#endif

#include "bt_gap.h"
#include "bt_gap_le.h"
#include "bt_gattc.h"
#include "bt_connection_manager.h"
#include "bt_callback_manager.h"
#include "bt_device_manager_le.h"
#include "app_dongle_connection_common.h"
#include "bt_gap_le_service.h"
#include "race_cmd.h"
#include "race_xport.h"
#include "race_noti.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_ULL_DONGLE_DISABLE 0
#define APP_ULL_DONGLE_ENABLE  1
#define APP_ULL_LOG_TAG     "[app_ull_dongle] "

#define APP_ULL_DONGLE_LE_INVALID_LINK_IDX       0xFF
#define APP_ULL_DONGLE_NVKEY_CSIS_DATA_LEN    18      /* NVID_BT_LEA_CSIS_DATA: | SIRK (16 bytes) | reserved (2 byte) | */

#define APP_ULL_DONGLE_GATT_CLIENT_MTU 512
#define APP_ULL_DONGLE_AIR_PAIRING_CLEAR_BOND_INFO

/**************************************************************************************************
* Structure
**************************************************************************************************/
/* scan related */
#define APP_ULL_DONGLE_LE_SCAN_NONE                 0   /* scan disabled */
#define APP_ULL_DONGLE_LE_SCAN_ENABLING             1   /* scan enabling */
#define APP_ULL_DONGLE_LE_SCAN_ENABLED              2   /* scan enabled */
#define APP_ULL_DONGLE_LE_SCAN_DISABLING            3   /* scan disabling */
#define APP_ULL_DONGLE_LE_SCAN_CS_ENABLING          4   /* scan coordinated set enabling */
#define APP_ULL_DONGLE_LE_SCAN_CS_ENABLED           5   /* scan coordinated set enabled */
#define APP_ULL_DONGLE_LE_SCAN_CS_DISABLING         6   /* scan coordinated set disabling */
typedef uint8_t app_ull_dongle_le_scan_state_t;

/* create connection related, conntection type */
#define APP_ULL_DONGLE_LE_CONN_NONE                     0
#define APP_ULL_DONGLE_LE_CONN_NEW_DEVICE               1   /* connect new device */
#define APP_ULL_DONGLE_LE_CONN_BONDED_DEVICE            2   /* connect bonded device */
#define APP_ULL_DONGLE_LE_CONN_COORDINATED_SET_BY_SIRK  3   /* connect coordinated set by SIRK. */
#define APP_ULL_DONGLE_LE_CONN_RECONNECT_DEVICE         4   /* connect coordinated set by SIRK. */
typedef uint8_t app_ull_dongle_le_conn_t;

#define APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_NONE      0
#define APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_ADD_ON_GOING     1
#define APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_REMOVE_ON_GOING  2
#define APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_COMPLETE         3
typedef uint8_t app_ull_dongle_le_set_white_list_state_t;

#define APP_ULL_DONGLE_LE_STATE_DISABLED      0
#define APP_ULL_DONGLE_LE_STATE_ENABLING      1
#define APP_ULL_DONGLE_LE_STATE_ENABLED       2
#define APP_ULL_DONGLE_LE_STATE_DISABLING     3
typedef uint8_t app_ull_dongle_le_state_t;

#define APP_ULL_DONGLE_LE_CONN_PARAM_DEFAULT_SPEED      0x0
#define APP_ULL_DONGLE_LE_CONN_PARAM_HIGH_SPEED         0x1
typedef uint8_t app_ull_dongle_le_conn_param_t;

/*Bonding information Context. */
typedef struct {
    uint8_t num;                /* bonded device number */
    bt_bd_addr_t *addr;         /* bonded device address list */
} app_ull_dongle_le_bonded_info_t;

/*Set White List Context. */
typedef struct {
    uint8_t curr_idx;
    app_ull_dongle_le_set_white_list_state_t state;
} app_ull_dongle_le_set_white_list_t;

/*Control Context. */
typedef struct {
    app_ull_dongle_le_scan_state_t curr_scan;
    app_ull_dongle_le_scan_state_t next_scan;
    app_ull_dongle_le_conn_t curr_conn;
    bool is_creating_connection;
    uint8_t set_size;
    bt_ull_client_t client_type;
    bt_key_t sirk; /* little endian */
    app_ull_dongle_le_state_t ull_state;
} app_ull_dongle_le_ctrl_t;

/*Black List Context. */
typedef struct {
    bt_addr_t addr;   /* peer device address */
} app_ull_dongle_le_black_list_t;

/**************************************************************************************************
* Variable
**************************************************************************************************/
static app_ull_dongle_le_bonded_info_t g_app_ull_le_bonded_info = {0, NULL};
static app_ull_dongle_le_link_info_t g_app_ull_le_link_info[BT_ULL_LE_MAX_LINK_NUM];
static app_ull_dongle_le_ctrl_t g_app_ull_le_ctrl;
static app_ull_dongle_le_set_white_list_t g_app_ull_le_set_white_list;
static app_ull_dongle_le_black_list_t g_app_ull_le_bl_info[BT_ULL_LE_MAX_LINK_NUM];
static app_dongle_le_race_event_callback_t g_ull_dongle_le_race_callback = NULL;
static uint8_t g_connect_idx = 0;
#ifdef APPS_ULL_V2_128_BIT_UUID
static uint8_t g_app_ull_dongle_le_uuid128_default[16] = APPS_ULL_V2_128_BIT_UUID;
#else
static uint8_t g_app_ull_dongle_le_uuid128_default[16] = { \
    0x45, 0x4C, 0x42, 0x61, 0x68, 0x6F, 0x72, 0x69,\
    0x41, 0x07, 0xAB, 0x2D, 0x4D, 0x49, 0x52, 0x50 \
};
#endif

/**************************************************************************************************
* Prototype
**************************************************************************************************/
static void app_ull_dongle_le_reset_param(void);
static bt_status_t app_ull_dongle_le_disconnect(bt_handle_t conn_handle);
static void app_ull_dongle_le_update_bonded_info(void);
static bt_status_t app_ull_dongle_le_start_scan_device(app_ull_dongle_le_scan_t scan_type, bool use_white_list);
static bt_status_t app_ull_dongle_le_stop_scan_device(app_ull_dongle_le_scan_t scan_type);
static bool app_ull_dongle_le_is_connected_device(bt_addr_t *addr);
static bt_status_t app_ull_dongle_le_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff);
static bt_status_t app_ull_dongle_le_enable(const bt_addr_t addr, app_dongle_cm_start_source_param_t param);
static bt_status_t app_ull_dongle_le_disable(const bt_addr_t addr, app_dongle_cm_stop_source_param_t param);
static bt_status_t app_ull_dongle_le_pre_check(app_dongle_cm_precheck_data_t *check_data);
extern void bt_app_common_at_cmd_print_report(char *string);
static void app_ull_dongle_gap_le_srv_event_callback(bt_gap_le_srv_event_t event, void *data);
bt_status_t app_ull_dongle_le_cancel_create_connection(void);
extern bt_gap_le_srv_link_t bt_device_manager_le_get_link_type_by_addr(bt_bd_addr_t *remote_addr);

/**************************************************************************************************
* Functions
**************************************************************************************************/
static void app_ull_dongle_le_print_adv_addr(bt_addr_t *addr)
{
#ifdef APP_ULL_DONGLE_LE_DEBUG
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"ULL ADV found! addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                                                        addr->type,
                                                        addr->addr[5],
                                                        addr->addr[4],
                                                        addr->addr[3],
                                                        addr->addr[2],
                                                        addr->addr[1],
                                                        addr->addr[0]);
#endif
}

#if 0
static bt_status_t app_ull_dongle_le_set_sniff(bool enable)
{
    bt_bd_addr_t aws_device;
    bt_status_t status = BT_STATUS_SUCCESS;
    uint32_t num = 0;
    num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), &aws_device, 1);

    if (num > 0) {
        bt_gap_connection_handle_t gap_handle = bt_cm_get_gap_handle(aws_device);
        bt_gap_link_policy_setting_t setting;

        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_set_sniff, enable:%d, conn_handle:0x%x", 2, enable, gap_handle);
        if (!enable) {
            status = bt_gap_exit_sniff_mode(gap_handle);
        }

        setting.sniff_mode = enable ? BT_GAP_LINK_POLICY_ENABLE : BT_GAP_LINK_POLICY_DISABLE;
        status = bt_gap_write_link_policy(gap_handle, &setting);
    }
    return status;
}
#endif

static void app_ull_dongle_le_reset_link_info(uint8_t link_idx)
{
    memset(&g_app_ull_le_link_info[link_idx], 0, sizeof(app_ull_dongle_le_link_info_t));
    g_app_ull_le_link_info[link_idx].handle = BT_HANDLE_INVALID;
}

static void app_ull_dongle_le_clear_black_list(uint8_t link_idx)
{
    memset(&g_app_ull_le_bl_info[link_idx], 0, sizeof(app_ull_dongle_le_black_list_t));
}

static void app_ull_dongle_le_reset_param(void)
{
    uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
    memset(&g_app_ull_le_ctrl, 0, sizeof(app_ull_dongle_le_ctrl_t));
    memset(&g_app_ull_le_set_white_list, 0, sizeof(app_ull_dongle_le_set_white_list_t));

    /** reset link info */
    while (i > 0) {
        i--;
        app_ull_dongle_le_reset_link_info(i);
        app_ull_dongle_le_clear_black_list(i);
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_reset_param Success", 0);
}

static bool app_ull_dongle_le_sirk_is_null(void)
{
    bt_key_t invalid_sirk = {0};

    if (0 == memcmp(&(g_app_ull_le_ctrl.sirk), &invalid_sirk, sizeof(bt_key_t))) {
        APPS_LOG_MSGID_W(APP_ULL_LOG_TAG"Sirk is NULL!\r\n", 0);
        return true;
    }
    return false;
}

static bt_status_t app_ull_dongle_le_save_link_info(bt_gap_le_connection_ind_t *ind)
{
    uint8_t link_idx = 0;

    for (link_idx = 0; link_idx < BT_ULL_LE_MAX_LINK_NUM; link_idx++) {
        if (BT_HANDLE_INVALID == g_app_ull_le_link_info[link_idx].handle) {
            break;
        }
    }

    if (BT_ULL_LE_MAX_LINK_NUM == link_idx) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"LE_CONNECT_IND link full, handle:%x", 1, ind->connection_handle);
        return BT_STATUS_OUT_OF_MEMORY;
    }

    app_ull_dongle_le_reset_link_info(link_idx);
    g_app_ull_le_link_info[link_idx].handle = ind->connection_handle;
    g_app_ull_le_link_info[link_idx].conn_interval = ind->conn_interval;
    memcpy(&(g_app_ull_le_link_info[link_idx].addr), &(ind->peer_addr), sizeof(bt_addr_t));

#ifdef APP_ULL_DONGLE_LE_DEBUG
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"LE_CONNECT_IND, handle[%x]:%x addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 9,
                                                                link_idx,
                                                                ind->connection_handle,
                                                                g_app_ull_le_link_info[link_idx].addr.type,
                                                                g_app_ull_le_link_info[link_idx].addr.addr[5],
                                                                g_app_ull_le_link_info[link_idx].addr.addr[4],
                                                                g_app_ull_le_link_info[link_idx].addr.addr[3],
                                                                g_app_ull_le_link_info[link_idx].addr.addr[2],
                                                                g_app_ull_le_link_info[link_idx].addr.addr[1],
                                                                g_app_ull_le_link_info[link_idx].addr.addr[0]);
#endif
    return BT_STATUS_SUCCESS;
}

bt_status_t app_ull_dongle_le_add_black_list(bt_addr_t *addr)
{
    uint8_t i;
    bt_addr_t empty_addr = {0};

    for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
        if (0 == memcmp(&(g_app_ull_le_bl_info[i].addr), &empty_addr, sizeof(bt_addr_t))) {
            memcpy(&(g_app_ull_le_bl_info[i].addr), addr, sizeof(bt_addr_t));
            #ifdef APP_ULL_DONGLE_LE_DEBUG
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_add_black_list, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                                                                g_app_ull_le_bl_info[i].addr.type,
                                                                g_app_ull_le_bl_info[i].addr.addr[5],
                                                                g_app_ull_le_bl_info[i].addr.addr[4],
                                                                g_app_ull_le_bl_info[i].addr.addr[3],
                                                                g_app_ull_le_bl_info[i].addr.addr[2],
                                                                g_app_ull_le_bl_info[i].addr.addr[1],
                                                                g_app_ull_le_bl_info[i].addr.addr[0]);
            #endif
            return BT_STATUS_SUCCESS;
        }
    }
    APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"app_ull_dongle_le_add_black_list Failed!", 0);
    return BT_STATUS_FAIL;
}

static bt_status_t app_ull_dongle_le_remove_black_list(bt_addr_t *addr)
{
    uint8_t i = 0;

    for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
        if (0 == memcmp(&(g_app_ull_le_bl_info[i].addr), addr, sizeof(bt_addr_t))) {
            #ifdef APP_ULL_DONGLE_LE_DEBUG
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_remove_black_list, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                                                                g_app_ull_le_bl_info[i].addr.type,
                                                                g_app_ull_le_bl_info[i].addr.addr[5],
                                                                g_app_ull_le_bl_info[i].addr.addr[4],
                                                                g_app_ull_le_bl_info[i].addr.addr[3],
                                                                g_app_ull_le_bl_info[i].addr.addr[2],
                                                                g_app_ull_le_bl_info[i].addr.addr[1],
                                                                g_app_ull_le_bl_info[i].addr.addr[0]);
            #endif
            memset(&g_app_ull_le_bl_info[i], 0, sizeof(app_ull_dongle_le_black_list_t));
            return BT_STATUS_SUCCESS;
        }
    }
    APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"app_ull_dongle_le_remove_black_list Failed!", 0);
    return BT_STATUS_FAIL;
}

static void app_ull_dongle_le_update_bonded_info(void)
{
    if (NULL != g_app_ull_le_bonded_info.addr) {
        vPortFree(g_app_ull_le_bonded_info.addr);
    }
    memset(&g_app_ull_le_bonded_info, 0, sizeof(app_ull_dongle_le_bonded_info_t));
    if (0 == (g_app_ull_le_bonded_info.num = bt_device_manager_le_get_bonded_number())) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"update_bonded_info fail, no bonded info!", 0);
        return;
    }

    g_app_ull_le_bonded_info.addr = (bt_bd_addr_t *)pvPortMalloc(g_app_ull_le_bonded_info.num * sizeof(bt_bd_addr_t));
    if (NULL == g_app_ull_le_bonded_info.addr) {
        g_app_ull_le_bonded_info.num = 0;
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"update_bonded_info, malloc failed!", 0);
        return;
    }

    bt_device_manager_le_get_bonded_list(g_app_ull_le_bonded_info.addr, &g_app_ull_le_bonded_info.num);
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"update_bonded_info, bonded num is %d", 1, g_app_ull_le_bonded_info.num);

    /* Check real bonded list num */
    if (0 == g_app_ull_le_bonded_info.num) {
        vPortFree(g_app_ull_le_bonded_info.addr);
        g_app_ull_le_bonded_info.addr = NULL;
        return;
    }

#ifdef APP_ULL_DONGLE_LE_DEBUG   /* just For test */
    {
        uint8_t *tempaddr = NULL;
        uint8_t i;
        i = g_app_ull_le_bonded_info.num;
        while (i > 0) {
            i--;
            tempaddr = (uint8_t *)&g_app_ull_le_bonded_info.addr[i];
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"bonded_list[%x], addr:%02x:%02x:%02x:%02x:%02x:%02x", 7, i, tempaddr[5], tempaddr[4],
                                                                                                tempaddr[3], tempaddr[2],
                                                                                                tempaddr[1], tempaddr[0]);
        }
    }
#endif
}

static void app_ull_dongle_le_add_bonded_device_to_white_list(uint8_t idx)
{
    if (0 == g_app_ull_le_bonded_info.num) {
        APPS_LOG_MSGID_W(APP_ULL_LOG_TAG"[APP][U] add_bonded_device_to_white_list, bonded_list is empty", 0);
        return;
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP] add_bonded_device_to_white_list, total_num:%x idx:%x state:%x", 3,
                      g_app_ull_le_bonded_info.num,
                      g_app_ull_le_set_white_list.curr_idx,
                      g_app_ull_le_set_white_list.state);


    if (APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_ADD_ON_GOING == g_app_ull_le_set_white_list.state) {
        if (g_app_ull_le_bonded_info.num <= idx) {
            memset(&g_app_ull_le_set_white_list, 0, sizeof(app_ull_dongle_le_set_white_list_t));
            g_app_ull_le_set_white_list.state = APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_COMPLETE;
            app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE);
            return;
        }
    }

    if (idx < g_app_ull_le_bonded_info.num) {
        bt_addr_t device;
        bt_status_t ret;

        device.type = BT_ADDR_PUBLIC;
        memcpy(device.addr, (g_app_ull_le_bonded_info.addr + idx), sizeof(bt_bd_addr_t));
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] white_list[%x], addr:%02x:%02x:%02x:%02x:%02x:%02x", 7, idx, device.addr[5], device.addr[4],
                          device.addr[3], device.addr[2],
                          device.addr[1], device.addr[0]);

        g_app_ull_le_set_white_list.curr_idx = idx;
        g_app_ull_le_set_white_list.state = APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_ADD_ON_GOING;

        ret = bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &device);

        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] add_white_list, ret:%x", 1, ret);
    }
}

static uint8_t app_ull_dongle_le_get_link_idx(bt_handle_t handle)
{
    uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
    if (BT_HANDLE_INVALID == handle) {
        return APP_ULL_DONGLE_LE_INVALID_LINK_IDX;
    }

    while (i > 0) {
        i--;
        if (handle == g_app_ull_le_link_info[i].handle) {
            return i;
        }
    }
    return APP_ULL_DONGLE_LE_INVALID_LINK_IDX;
}

void app_ull_dongle_le_set_sirk(bt_key_t *sirk, bool update_nvkey)
{
    memcpy((uint8_t *)&g_app_ull_le_ctrl.sirk, (uint8_t *)sirk, sizeof(bt_key_t));

    if (update_nvkey) {
        uint8_t data[APP_ULL_DONGLE_NVKEY_CSIS_DATA_LEN] = {0};
        nvkey_status_t ret = NVKEY_STATUS_ERROR;
        uint32_t size = APP_ULL_DONGLE_NVKEY_CSIS_DATA_LEN;

        /* NVID_BT_LEA_CSIS_DATA: | SIRK (16 bytes) | reserved (2 byte) | */
        if (NVKEY_STATUS_OK == nvkey_read_data(NVID_BT_LEA_CSIS_DATA, data, &size)) {
            memcpy(data, (uint8_t *)sirk, sizeof(bt_key_t));
            size = APP_ULL_DONGLE_NVKEY_CSIS_DATA_LEN;
            ret = nvkey_write_data(NVID_BT_LEA_CSIS_DATA, data, size);
        }
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] set_sirk, nvkey write ret:%x", 1, ret);
    }
}

bt_key_t *app_ull_dongle_le_get_sirk(bool from_nvkey)
{
    if (from_nvkey) {
        uint8_t data[APP_ULL_DONGLE_NVKEY_CSIS_DATA_LEN] = {0};
        uint32_t size = APP_ULL_DONGLE_NVKEY_CSIS_DATA_LEN;

        /* NVID_BT_LEA_CSIS_DATA: | SIRK (16 bytes) | reserved (2 byte) | */
        if (NVKEY_STATUS_OK == nvkey_read_data(NVID_BT_LEA_CSIS_DATA, data, &size)) {
            memcpy((uint8_t *)&g_app_ull_le_ctrl.sirk, data, sizeof(bt_key_t));
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] get_sirk, nvkey read success", 0);
            bt_ull_le_srv_set_sirk(g_app_ull_le_ctrl.sirk);
            return &g_app_ull_le_ctrl.sirk;
        }
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"[APP][U] get_sirk, nvkey read failed", 0);
        return NULL;
    }
    return &g_app_ull_le_ctrl.sirk;
}

app_ull_dongle_le_link_info_t *app_ull_dongle_le_get_link_info(bt_handle_t handle)
{
    if (BT_HANDLE_INVALID != handle) {
        uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
        while (i > 0) {
            i--;
            if (handle == g_app_ull_le_link_info[i].handle) {
                return &g_app_ull_le_link_info[i];
            }
        }
    }
    return NULL;
}

uint8_t app_ull_dongle_le_get_link_num(void)
{
    uint8_t i = 0, count = 0;

    i = BT_ULL_LE_MAX_LINK_NUM;
    while (i > 0) {
        i--;
        if (BT_HANDLE_INVALID != g_app_ull_le_link_info[i].handle) {
            count++;
        }
    }
    return count;
}

static bool app_ull_dongle_le_is_ull_adv(bt_gap_le_ext_advertising_report_ind_t *adv_report)
{
    bt_bd_addr_t empty_addr = {0};
    uint16_t i = 0;
    uint8_t *uuid = &g_app_ull_dongle_le_uuid128_default[0];

    if (!(adv_report->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_CONNECTABLE)) {
        return false;
    }

    if ((!(adv_report->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_DIRECTED) &&
         (0 == memcmp(adv_report->address.addr, empty_addr, sizeof(bt_bd_addr_t)))) ||
        ((adv_report->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_DIRECTED) &&
         (0 == memcmp(adv_report->direct_address.addr, empty_addr, sizeof(bt_bd_addr_t))))) {
        return false;
    }


    while (i < adv_report->data_length) {
        if ((adv_report->data[i] >= 17) &&
            (adv_report->data[i+1] == BT_GAP_LE_AD_TYPE_128_BIT_UUID_DATA) &&
            (0 == memcmp(&(adv_report->data[i+2]), uuid, 16))) {
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] is_ull_adv, Yes!", 0);
            return true;
        }
        i += (adv_report->data[i] + 1);
    }

    return false;
}

static bool app_ull_dongle_le_is_bonded_device(bt_addr_t *addr)
{
    uint8_t i;

    if (NULL == g_app_ull_le_bonded_info.addr || !addr) {
        return false;
    }

#ifdef APP_ULL_DONGLE_LE_DEBUG
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP] is_bonded_device, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                                                                                addr->type,
                                                                                addr->addr[5],
                                                                                addr->addr[4],
                                                                                addr->addr[3],
                                                                                addr->addr[2],
                                                                                addr->addr[1],
                                                                                addr->addr[0]);
#endif

    i = g_app_ull_le_bonded_info.num;

    while (i > 0) {
        i--;
        if (0 == memcmp((g_app_ull_le_bonded_info.addr+i), addr->addr, BT_BD_ADDR_LEN)) {
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] is_bonded_device, YES!", 0);
            return true;
        }
    }

    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] is_bonded_device, NO!", 0);
    return false;
}

static bool app_ull_dongle_le_is_in_black_list(bt_addr_t *addr)
{
    uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
    while (i > 0) {
        i--;
        if (0 == memcmp(&g_app_ull_le_bl_info[i].addr, addr, sizeof(bt_addr_t))) {
            return true;
        }
    }
    return false;
}


static bt_status_t app_ull_dongle_le_update_conn_param(bt_handle_t handle, app_ull_dongle_le_conn_param_t type)
{
    bt_status_t status;
    bt_hci_cmd_le_connection_update_t conn_params;
    conn_params.supervision_timeout = 0x01F4;            /** TBC: 6000ms : 600 * 10 ms. */
    conn_params.connection_handle = handle;
    conn_params.minimum_ce_length = 0x06;
    conn_params.maximum_ce_length = 0x06;

    switch (type) 
    {
        case APP_ULL_DONGLE_LE_CONN_PARAM_DEFAULT_SPEED:
        {
            conn_params.conn_interval_min = 0x0018;/** TBC: 7.5ms : 6 * 1.25 ms. */
            conn_params.conn_interval_max = 0x0018;/** TBC: 12.5ms : 10 * 1.25 ms. */
            conn_params.conn_latency = 0;
            break;
        }
        case APP_ULL_DONGLE_LE_CONN_PARAM_HIGH_SPEED:
        {
            conn_params.conn_interval_min = 0x0006;/** TBC: 7.5ms : 6 * 1.25 ms. */
            conn_params.conn_interval_max = 0x0006;/** TBC: 12.5ms : 10 * 1.25 ms. */
            conn_params.conn_latency = 0;
            break;
        }
    }
    status = bt_gap_le_update_connection_parameter(&conn_params);
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_update_conn_param, status: %x, type: %d", 2, status, type);

    return status;
}

static bool app_ull_dongle_le_is_connected_device(bt_addr_t *addr)
{
    uint8_t i = BT_ULL_LE_MAX_LINK_NUM;

    while (i > 0) {
        i--;
        if (BT_HANDLE_INVALID == g_app_ull_le_link_info[i].handle) {
            continue;
        }

        if (0 == memcmp(&g_app_ull_le_link_info[i].addr, addr, sizeof(bt_addr_t))) {
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP] is_connected_device, YES!", 0);
            return true;
        }
    }

    APPS_LOG_MSGID_W(APP_ULL_LOG_TAG"[APP] is_connected_device, NO!", 0);
    return false;
}

bt_status_t app_ull_dongle_le_connect(bt_addr_t *addr)
{
    bt_status_t status;

    if (g_app_ull_le_ctrl.is_creating_connection) {
        APPS_LOG_MSGID_W(APP_ULL_LOG_TAG"[APP][U] connect_device, busy", 0);
        return BT_STATUS_BUSY;
    }

    if (BT_ULL_LE_MAX_LINK_NUM == app_ull_dongle_le_get_link_num()) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"[APP][U] connect_device, link full", 0);
        return BT_STATUS_FAIL;
    }

    bt_hci_cmd_le_create_connection_t param = {
        .le_scan_interval = 0x10,
        .le_scan_window = 0x10,
        .initiator_filter_policy = BT_HCI_CONN_FILTER_ASSIGNED_ADDRESS,
        .own_address_type = BT_ADDR_RANDOM,
        .conn_interval_min = 0x0006,
        .conn_interval_max = 0x0006,
        .conn_latency = 0x0000,
        .supervision_timeout = 0x01F4,
        .minimum_ce_length = 0x0002,
        .maximum_ce_length = 0x0002,
    };

    /** stop all scan when connect any device. */
    app_ull_dongle_le_stop_scan();
    memcpy(&(param.peer_address), addr, sizeof(bt_addr_t));
    app_ull_dongle_le_remove_black_list(addr);
#ifdef APP_ULL_DONGLE_LE_DEBUG
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] connect_device, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      param.peer_address.type,
                      param.peer_address.addr[5],
                      param.peer_address.addr[4],
                      param.peer_address.addr[3],
                      param.peer_address.addr[2],
                      param.peer_address.addr[1],
                      param.peer_address.addr[0]);

#endif

    g_app_ull_le_ctrl.is_creating_connection = true;
    if (BT_STATUS_SUCCESS != (status = bt_gap_le_connect(&param))) {
        g_app_ull_le_ctrl.is_creating_connection = false;
    }

    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] connect_device, status is 0x%4x", 1, status);
    return status;
}

static bt_status_t app_ull_dongle_le_start_scan_device(app_ull_dongle_le_scan_t scan_type, bool use_white_list)
{
    bt_status_t status;
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"start_scan_device begain, scan_type: %d, use_wl: %d, curr_scan:%x next_scan:%x", 4, scan_type,
        use_white_list, g_app_ull_le_ctrl.curr_scan, g_app_ull_le_ctrl.next_scan);

    switch (g_app_ull_le_ctrl.curr_scan) {
        case APP_ULL_DONGLE_LE_SCAN_ENABLING:
        case APP_ULL_DONGLE_LE_SCAN_DISABLING:
        case APP_ULL_DONGLE_LE_SCAN_CS_ENABLING:
        case APP_ULL_DONGLE_LE_SCAN_CS_DISABLING: {
            g_app_ull_le_ctrl.next_scan = (APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK == scan_type)? APP_ULL_DONGLE_LE_SCAN_CS_ENABLING: APP_ULL_DONGLE_LE_SCAN_ENABLING;
            return BT_STATUS_SUCCESS;
        }
        case APP_ULL_DONGLE_LE_SCAN_ENABLED: {
            /* stop_scan--> start_scan */
            g_app_ull_le_ctrl.next_scan = (APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK == scan_type)? APP_ULL_DONGLE_LE_SCAN_CS_ENABLING: APP_ULL_DONGLE_LE_SCAN_ENABLING;
            if (BT_STATUS_SUCCESS != (status = app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE))) {
                g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
            }
            return BT_STATUS_SUCCESS;
        }
        case APP_ULL_DONGLE_LE_SCAN_CS_ENABLED: {
            /* stop_scan cs--> start_scan */
            g_app_ull_le_ctrl.next_scan = (APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK == scan_type)? APP_ULL_DONGLE_LE_SCAN_CS_ENABLING: APP_ULL_DONGLE_LE_SCAN_ENABLING;
            if (BT_STATUS_SUCCESS != (status = app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK))) {
                g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
            }
            return BT_STATUS_SUCCESS;
        }
        default:
            break;
    }
    uint8_t duplicate_filter_enable = use_white_list ? BT_HCI_DISABLE : BT_HCI_ENABLE;
    /** curr_scan: APP_ULL_DONGLE_LE_SCAN_NONE, start scan */
    le_ext_scan_item_t ext_scan_1M_item = {
        .scan_type = BT_HCI_SCAN_TYPE_PASSIVE,
        .scan_interval = 0x90,
        .scan_window = 0x30,
    };
    bt_hci_cmd_le_set_extended_scan_enable_t enable = {
        .enable = BT_HCI_ENABLE,
        .filter_duplicates = duplicate_filter_enable,
        .duration = 0,
        .period = 0
    };

    bt_hci_le_set_ext_scan_parameters_t param = {
        .own_address_type = BT_HCI_SCAN_ADDR_RANDOM,
        .scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS,
        .scanning_phys_mask = BT_HCI_LE_PHY_MASK_1M,
        .params_phy_1M = &ext_scan_1M_item,
        .params_phy_coded = NULL,
    };

    param.scanning_filter_policy = use_white_list ? BT_HCI_SCAN_FILTER_ACCEPT_ONLY_ADVERTISING_PACKETS_IN_WHITE_LIST : BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS;
    g_app_ull_le_ctrl.curr_scan = (APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK == scan_type)? APP_ULL_DONGLE_LE_SCAN_CS_ENABLING: APP_ULL_DONGLE_LE_SCAN_ENABLING;

    if (BT_STATUS_SUCCESS != (status = bt_gap_le_set_extended_scan(&param, &enable))) {
        g_app_ull_le_ctrl.curr_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"start scan device failed, status:%x", 1,  status);
        if (BT_STATUS_OUT_OF_MEMORY == status) {
            assert(0);
        }
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"start_scan_device End, curr_scan:%x next_scan:%x", 2,  g_app_ull_le_ctrl.curr_scan, g_app_ull_le_ctrl.next_scan);
    return status;
}

static bt_status_t app_ull_dongle_le_stop_scan_device(app_ull_dongle_le_scan_t scan_type)
{
    bt_status_t status;
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"stop_scan, scan_type:%d, curr_scan:%x next_scan:%x", 3, scan_type, g_app_ull_le_ctrl.curr_scan, g_app_ull_le_ctrl.next_scan);

    switch (g_app_ull_le_ctrl.curr_scan) {
        case APP_ULL_DONGLE_LE_SCAN_NONE:
        case APP_ULL_DONGLE_LE_SCAN_DISABLING:
        case APP_ULL_DONGLE_LE_SCAN_CS_DISABLING: {
            g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
            return BT_STATUS_SUCCESS;
        }

        case APP_ULL_DONGLE_LE_SCAN_CS_ENABLING: {
            if (APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK == scan_type) {
                g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_CS_DISABLING;
                return BT_STATUS_SUCCESS;
            }
            return BT_STATUS_FAIL;
        }

        case APP_ULL_DONGLE_LE_SCAN_CS_ENABLED: {
            if (APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK != scan_type) {
                return BT_STATUS_FAIL;
            }
        } break;

        case APP_ULL_DONGLE_LE_SCAN_ENABLING: {
             if (APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK != scan_type) {
                 g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_DISABLING;
                 return BT_STATUS_SUCCESS;
            }
             return BT_STATUS_FAIL;
        }

        case APP_ULL_DONGLE_LE_SCAN_ENABLED: {
            if (APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK == scan_type) {
                return BT_STATUS_FAIL;
        }
        } break;

        default:
            break;
    }

    /** curr_scan: APP_ULL_DONGLE_LE_SCAN_ENABLED or APP_ULL_DONGLE_LE_SCAN_CS_ENABLED*/
    bt_hci_cmd_le_set_extended_scan_enable_t enable = {
        .enable = BT_HCI_DISABLE,
        .filter_duplicates = BT_HCI_DISABLE,
        .duration = 0,
        .period = 0
    };

    /**To Stop Scan. */
    g_app_ull_le_ctrl.curr_scan = (APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK == scan_type) ? APP_ULL_DONGLE_LE_SCAN_CS_DISABLING : APP_ULL_DONGLE_LE_SCAN_DISABLING;

    if (BT_STATUS_SUCCESS != (status = bt_gap_le_set_extended_scan(NULL, &enable))) {
        g_app_ull_le_ctrl.curr_scan = (APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK == scan_type) ? APP_ULL_DONGLE_LE_SCAN_CS_ENABLED : APP_ULL_DONGLE_LE_SCAN_ENABLED;
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"stop_scan failed, status:%x", 1,  status);
    } else {
        char conn_string[50] = {0};
        snprintf((char *)conn_string, 50, "Stop scan:\r\n");
        bt_app_common_at_cmd_print_report(conn_string);
    }
    return status;
}

bt_status_t app_ull_dongle_le_scan_new_device(void)
{
    return app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_NEW_DEVICE);
}

bt_status_t app_ull_dongle_le_scan_device(app_ull_dongle_le_scan_t scan_type)
{
    bt_status_t status = BT_STATUS_FAIL;
    char conn_string[50] = {0};

    if (BT_ULL_LE_MAX_LINK_NUM == app_ull_dongle_le_get_link_num()) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"[APP][U] scan device by type(%d) fail, link full", 1, scan_type);
        return BT_STATUS_OUT_OF_MEMORY;
    }
    snprintf((char *)conn_string, 50, "Scanning device:\r\n");
    bt_app_common_at_cmd_print_report(conn_string);

    switch (scan_type) {
        case APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK: {
            if (app_ull_dongle_le_sirk_is_null()) {
                APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"[APP][U] scan cs fail, SIRK is NULL", 0);
                return BT_STATUS_FAIL;
            }
            g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_COORDINATED_SET_BY_SIRK;
            if (BT_STATUS_SUCCESS != (status = app_ull_dongle_le_start_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK, false))) {
                g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_NONE;
            }
        } break;
        case APP_ULL_DONGLE_LE_SCAN_RECONNECT_DEVICE:
        case APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE: {
            if (0 == g_app_ull_le_bonded_info.num) {
                APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"[APP][U] scan bobded device, but no bonded device found", 0);
                return BT_STATUS_CONNECTION_NOT_FOUND;
            }

            if (APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_ADD_ON_GOING == g_app_ull_le_set_white_list.state) {
                APPS_LOG_MSGID_W(APP_ULL_LOG_TAG"[APP][U] scan bobded device, but set white list ongoing, busy", 0);
                return BT_STATUS_BUSY;
            }

            g_app_ull_le_ctrl.curr_conn = (scan_type == APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE) ? APP_ULL_DONGLE_LE_CONN_BONDED_DEVICE : APP_ULL_DONGLE_LE_CONN_RECONNECT_DEVICE;
            if (BT_STATUS_SUCCESS != (status = app_ull_dongle_le_start_scan_device(scan_type, true))) {
                g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_NONE;
            }

        } break;

        case APP_ULL_DONGLE_LE_SCAN_NEW_DEVICE: {
            g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_NEW_DEVICE;
            /** start scan new device. */
            if (BT_STATUS_SUCCESS != (status = app_ull_dongle_le_start_scan_device(APP_ULL_DONGLE_LE_SCAN_NEW_DEVICE, false))) {
                 g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_NONE;
            }
        } break;

        default:
            break;
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] scan device, scan_type: %d, curr_scan:%x, next_scan:%x, conn:%x", 4, scan_type,
        g_app_ull_le_ctrl.curr_scan, g_app_ull_le_ctrl.next_scan, g_app_ull_le_ctrl.curr_conn);
    return status;
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
static void app_ull_dongle_le_handle_bt_power_on_cnf(void)
{
    bt_status_t status;
    uint8_t mode = APP_ULL_DONGLE_ENABLE;//ull dongle mode,not headset; and ull feature is enable

    if (APP_DONGLE_CM_LINK_MODE_ULL_V2 & app_dongle_cm_get_link_mode()) {
        /** Notify controller that it's in dongle mode. */
        bt_hci_send_vendor_cmd(0xFDCC, (uint8_t *)&mode, sizeof(uint8_t));

        /** Notify controller that ULL feature set bit is enable */
        status = bt_hci_send_vendor_cmd(0xFE0C, (uint8_t *)&mode, sizeof(uint8_t));
    } else {
        /** Notify controller that ULL feature set bit is disable */
        mode = APP_ULL_DONGLE_DISABLE;
        status = bt_hci_send_vendor_cmd(0xFE0C, (uint8_t *)&mode, sizeof(uint8_t));
    }
    app_ull_dongle_le_get_sirk(true);
    app_ull_dongle_le_update_bonded_info();
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"BT_POWER_ON_CNF, set ULL feature bit, status is 0x%x, bit value is %d", 2, status, mode);
#if 0
    /** get the bonded device info firstly. */
    app_ull_dongle_le_update_bonded_info();
    app_ull_dongle_le_add_bonded_device_to_white_list(0);
    app_ull_dongle_le_start_scan();
#endif
}

static void app_ull_dongle_le_handle_bt_power_off_cnf(void)
{
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP] BT_POWER_OFF_CNF", 0);
    app_ull_dongle_le_reset_param();
}

static void app_ull_dongle_le_handle_scan_cnf(bt_status_t status)
{
    char conn_string[50] = {0};

    if (APP_ULL_DONGLE_LE_STATE_DISABLED == g_app_ull_le_ctrl.ull_state) {
        return;
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"LE_SET_SCAN_CNF Begain, status:0x%4x scan:%x->%x conn:%x", 4, status,
                                                                g_app_ull_le_ctrl.curr_scan, g_app_ull_le_ctrl.next_scan,
                                                                g_app_ull_le_ctrl.curr_conn);
    if (BT_STATUS_SUCCESS != status) {
        /** handle error case */
        switch (g_app_ull_le_ctrl.curr_scan) {
            case APP_ULL_DONGLE_LE_SCAN_ENABLING:
            case APP_ULL_DONGLE_LE_SCAN_CS_ENABLING: {
                g_app_ull_le_ctrl.curr_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
                break;
            }
            case APP_ULL_DONGLE_LE_SCAN_DISABLING: {
                g_app_ull_le_ctrl.curr_scan = APP_ULL_DONGLE_LE_SCAN_ENABLED;
                break;
            }
            case APP_ULL_DONGLE_LE_SCAN_CS_DISABLING: {
                g_app_ull_le_ctrl.curr_scan = APP_ULL_DONGLE_LE_SCAN_CS_ENABLED;
                break;
            }
            default:
                break;
        }
        g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
        return;
    }

    if (APP_ULL_DONGLE_LE_STATE_DISABLING == g_app_ull_le_ctrl.ull_state) {/**< stop scan successfully.*/
        g_app_ull_le_ctrl.ull_state = APP_ULL_DONGLE_LE_STATE_DISABLED;
        app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V2, APP_DONGLE_CM_EVENT_SOURCE_END, status, NULL);
    }

    if (g_app_ull_le_ctrl.next_scan == g_app_ull_le_ctrl.curr_scan) {
        g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
    }

    /** update curr_scan state */
    switch (g_app_ull_le_ctrl.curr_scan) {
        case APP_ULL_DONGLE_LE_SCAN_ENABLING: {
            g_app_ull_le_ctrl.curr_scan = APP_ULL_DONGLE_LE_SCAN_ENABLED;
            snprintf((char *)conn_string, 50, "Start scanning:\r\n");
            bt_app_common_at_cmd_print_report(conn_string);
            if ((APP_ULL_DONGLE_LE_SCAN_CS_ENABLING == g_app_ull_le_ctrl.next_scan) ||
                (APP_ULL_DONGLE_LE_SCAN_DISABLING == g_app_ull_le_ctrl.next_scan)) {
                if (BT_STATUS_SUCCESS != (status = app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE))) {
                    g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
                }
                return;
            }
            break;
        }
        case APP_ULL_DONGLE_LE_SCAN_CS_ENABLING: {
            g_app_ull_le_ctrl.curr_scan = APP_ULL_DONGLE_LE_SCAN_CS_ENABLED;
            snprintf((char *)conn_string, 50, "Start scanning:\r\n");
            bt_app_common_at_cmd_print_report(conn_string);
            if ((APP_ULL_DONGLE_LE_SCAN_ENABLING == g_app_ull_le_ctrl.next_scan) ||
                (APP_ULL_DONGLE_LE_SCAN_CS_DISABLING == g_app_ull_le_ctrl.next_scan)) {
                if (BT_STATUS_SUCCESS != (status = app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK))) {
                    g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
                }
                return;
            }
            break;
        }

        case APP_ULL_DONGLE_LE_SCAN_DISABLING:
        case APP_ULL_DONGLE_LE_SCAN_CS_DISABLING: {
            g_app_ull_le_ctrl.curr_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
            snprintf((char *)conn_string, 50, "Stop scanning.\r\n");
            bt_app_common_at_cmd_print_report(conn_string);

            if (APP_ULL_DONGLE_LE_SCAN_ENABLING == g_app_ull_le_ctrl.next_scan) {
                bool use_white_list = (APP_ULL_DONGLE_LE_CONN_BONDED_DEVICE == g_app_ull_le_ctrl.curr_conn || APP_ULL_DONGLE_LE_CONN_RECONNECT_DEVICE == g_app_ull_le_ctrl.curr_conn) ? true : false;
                app_ull_dongle_le_scan_t scan_type = APP_ULL_DONGLE_LE_CONN_NEW_DEVICE;
                switch (g_app_ull_le_ctrl.curr_conn) {
                    case APP_ULL_DONGLE_LE_CONN_BONDED_DEVICE: {
                        scan_type = APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE;
                    } break;
                    case APP_ULL_DONGLE_LE_CONN_RECONNECT_DEVICE: {
                        scan_type = APP_ULL_DONGLE_LE_SCAN_RECONNECT_DEVICE;
                    } break;
                    case APP_ULL_DONGLE_LE_CONN_NEW_DEVICE: {
                        scan_type = APP_ULL_DONGLE_LE_SCAN_NEW_DEVICE;
                    } break;
                    default: 
                        break;
                }
                if (BT_STATUS_SUCCESS != (status = app_ull_dongle_le_start_scan_device(scan_type, use_white_list))) {
                    g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
                }
                return;
            }
            if (APP_ULL_DONGLE_LE_SCAN_CS_ENABLING == g_app_ull_le_ctrl.next_scan) {
                if (BT_STATUS_SUCCESS != (status = app_ull_dongle_le_start_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK, false))) {
                    g_app_ull_le_ctrl.next_scan = APP_ULL_DONGLE_LE_SCAN_NONE;
                }
                return;
            }
            break;
        }
        default:
            break;
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"LE_SET_SCAN_CNF End, scan status:%x->%x conn:%x", 3, g_app_ull_le_ctrl.curr_scan,
        g_app_ull_le_ctrl.next_scan, g_app_ull_le_ctrl.curr_conn);
}

static void app_ull_dongle_le_handle_device_adv(bt_gap_le_ext_advertising_report_ind_t *ind)
{
    uint8_t link_num;

    if (BT_ULL_LE_MAX_LINK_NUM == (link_num = app_ull_dongle_le_get_link_num())) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"[APP][U] handle_adv_report_ind, link full", 0);
        app_ull_dongle_le_scan_t scan_type = (APP_ULL_DONGLE_LE_CONN_BONDED_DEVICE == g_app_ull_le_ctrl.curr_conn) ? APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE : APP_ULL_DONGLE_LE_SCAN_NEW_DEVICE;
        g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_NONE;
        app_ull_dongle_le_stop_scan_device(scan_type);
        return;
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] app_ull_dongle_le_handle_device_adv, curr_conn is %d", 1, g_app_ull_le_ctrl.curr_conn);

    switch (g_app_ull_le_ctrl.curr_conn) {
        case APP_ULL_DONGLE_LE_CONN_BONDED_DEVICE: {
            /* connect device */
            if ((app_ull_dongle_le_is_bonded_device(&ind->address) && (!app_ull_dongle_le_is_connected_device(&ind->address)))) {
                app_ull_dongle_le_print_adv_addr(&ind->address);
                app_ull_dongle_le_connect(&ind->address);
            }
            break;
        }
        case APP_ULL_DONGLE_LE_CONN_NEW_DEVICE: {
            /* connect device */
            if (!app_ull_dongle_le_is_connected_device(&ind->address)) {
                app_ull_dongle_le_print_adv_addr(&ind->address);
                app_ull_dongle_le_connect(&ind->address);
            }
            break;
        }

        default:
            app_ull_dongle_le_print_adv_addr(&ind->address);
            break;
    }
}

static void app_ull_dongle_le_handle_new_cs_adv(bt_gap_le_ext_advertising_report_ind_t *ind)
{
    uint8_t link_num;
    uint16_t length = 0, index = 0;
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_handle_new_cs_adv", 0);

    if (app_ull_dongle_le_is_connected_device(&ind->address)) {
        APPS_LOG_MSGID_W(APP_ULL_LOG_TAG"app_ull_dongle_le_handle_new_cs_adv, addr had been connected", 0);
        return;
    }

    if (BT_ULL_LE_MAX_LINK_NUM == (link_num = app_ull_dongle_le_get_link_num())) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"[APP][CSIP] DISCOVER_COORDINATED_SET_CNF, link full", 0);
        g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_NONE;
        app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
        return;
    }

    /** Searched coordinated set device. */
    while (index < ind->data_length) {
        length = ind->data[index];
        if (0 == length) {
            break;
        }
        /**<Headset adv format is different with earbuds ULL adv.*/
        if ((length >= 7) && ((ind->data[index+1] == 0xF0) || (ind->data[index+1] == 0x2E))) { //have ull dongle special RSI info
            bt_status_t result = bt_ull_le_srv_verify_rsi(&ind->data[index+2]);
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"ADV report from Earbuds: %02x:%02x:%02x:%02x:%02x:%02x", 6, ind->address.addr[0], ind->address.addr[1], ind->address.addr[2], ind->address.addr[3], ind->address.addr[4], ind->address.addr[5]);
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"RSI is: %02x %02x %02x %02x %02x %02x", 6, ind->data[index+2], ind->data[index+3], ind->data[index+4], ind->data[index+5], ind->data[index+6], ind->data[index+7]);
            if ((BT_STATUS_SUCCESS == result) &&
                (APP_ULL_DONGLE_LE_CONN_COORDINATED_SET_BY_SIRK == g_app_ull_le_ctrl.curr_conn)) {
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] handle_adv_report_ind, is CS adv", 0);
                //app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
                app_ull_dongle_le_connect(&ind->address);
            }
            break;
        }

        if ((length >= 10) && ((ind->data[index + 1] == 0xFF) && (ind->data[index + 2] == 0x94) && (ind->data[index + 5] == 0x2E))) {
            bt_status_t result = bt_ull_le_srv_verify_rsi(&ind->data[index + 6]);
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"ADV report from Headset: %02x:%02x:%02x:%02x:%02x:%02x", 6, ind->address.addr[0], ind->address.addr[1], ind->address.addr[2], ind->address.addr[3], ind->address.addr[4], ind->address.addr[5]);
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"RSI is: %02x %02x %02x %02x %02x %02x", 6, ind->data[index+2], ind->data[index+3], ind->data[index+4], ind->data[index+5], ind->data[index+6], ind->data[index+7]);
            if ((BT_STATUS_SUCCESS == result) &&
                (APP_ULL_DONGLE_LE_CONN_COORDINATED_SET_BY_SIRK == g_app_ull_le_ctrl.curr_conn)) {
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] handle_adv_report_ind, is CS adv", 0);
                //app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
                app_ull_dongle_le_connect(&ind->address);
            }
            break;
        }
        index = index + length + 1;
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"index [%d] length [%d]", 2, index, length);
    }
}

static void app_ull_dongle_le_handle_reconnect_adv(bt_gap_le_ext_advertising_report_ind_t *ind)
{
    uint8_t link_num;
    uint16_t length = 0, index = 0;
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_handle_reconnect_adv", 0);

    if (app_ull_dongle_le_is_connected_device(&ind->address)) {
        APPS_LOG_MSGID_W(APP_ULL_LOG_TAG"app_ull_dongle_le_handle_reconnect_adv, addr had been connected", 0);
        return;
    }

    if (BT_ULL_LE_MAX_LINK_NUM == (link_num = app_ull_dongle_le_get_link_num())) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"app_ull_dongle_le_handle_reconnect_adv, link full", 0);
        g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_NONE;
        app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_RECONNECT_DEVICE);
        return;
    }
    while (index < ind->data_length) {
        length = ind->data[index];
        if (length >= 0x3 \
            && BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC == ind->data[index + 1] \
            && (ind->data[index + 2] == (APP_DONGLE_CM_RECONNECT_MODE_ADV_DATA & 0xFF)) \
            && (ind->data[index + 3] == ((APP_DONGLE_CM_RECONNECT_MODE_ADV_DATA & 0xFF00)) >> 8)) {
            if (APP_ULL_DONGLE_LE_CONN_RECONNECT_DEVICE == g_app_ull_le_ctrl.curr_conn) {
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_handle_reconnect_adv, is ull reconnect adv", 0);
                //app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
                app_ull_dongle_le_connect(&ind->address);
            }
            break;
        }
        index = index + length + 1;
    }
}
static void app_ull_dongle_le_handle_adv_report_ind(bt_status_t status, bt_gap_le_ext_advertising_report_ind_t *ind)
{
    if (APP_ULL_DONGLE_LE_STATE_ENABLED != g_app_ull_le_ctrl.ull_state) {
        return;
    }

    if ((BT_STATUS_SUCCESS != status) || (!ind)) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"adv report fail, status is 0x%x!", 1, status);
        return;
    }

    if (APP_ULL_DONGLE_LE_CONN_RECONNECT_DEVICE < g_app_ull_le_ctrl.curr_conn) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"adv report fail, invalid conn type: 0x%x!", 1, g_app_ull_le_ctrl.curr_conn);
        return;
    }

    if (app_ull_dongle_le_is_in_black_list(&(ind->address))) {
        APPS_LOG_MSGID_W(APP_ULL_LOG_TAG"adv report fail, device is in black list!", 0);
        return;
    }

    if (!app_ull_dongle_le_is_ull_adv(ind)) {
        return;
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"ULL Adv report, current conn type is %d", 1, g_app_ull_le_ctrl.curr_conn);
    switch (g_app_ull_le_ctrl.curr_conn) {
        case APP_ULL_DONGLE_LE_CONN_BONDED_DEVICE:
        case APP_ULL_DONGLE_LE_CONN_NEW_DEVICE: {
            app_ull_dongle_le_handle_device_adv(ind);
            break;
        }
        case APP_ULL_DONGLE_LE_CONN_RECONNECT_DEVICE: {
            app_ull_dongle_le_handle_reconnect_adv(ind);
            break;
        }
        case APP_ULL_DONGLE_LE_CONN_COORDINATED_SET_BY_SIRK: {
            app_ull_dongle_le_handle_new_cs_adv(ind);
            break;
        }
        default:
            break;
    }
}

static void app_ull_dongle_le_handle_connect_ind(bt_status_t status, bt_gap_le_connection_ind_t *ind)
{
    uint8_t link_num = app_ull_dongle_le_get_link_num();
    APPS_LOG_MSGID_W(APP_ULL_LOG_TAG"app_ull_dongle_le_handle_connect_ind! %d, %d, %d, %x", 4, \
        g_app_ull_le_ctrl.ull_state, g_app_ull_le_ctrl.is_creating_connection, link_num, status);
    if (APP_ULL_DONGLE_LE_STATE_DISABLED == g_app_ull_le_ctrl.ull_state || !g_app_ull_le_ctrl.is_creating_connection) {
        return;
    }
    if (APP_ULL_DONGLE_LE_STATE_DISABLING == g_app_ull_le_ctrl.ull_state \
        && BT_STATUS_SUCCESS != status \
        && g_app_ull_le_ctrl.is_creating_connection \
        && link_num == 0) { /*cancel create connection*/
        g_app_ull_le_ctrl.ull_state = APP_ULL_DONGLE_LE_STATE_DISABLED;
        app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V2, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, NULL);
        g_app_ull_le_ctrl.is_creating_connection = false;
        return;
    }
    BT_GATTC_NEW_EXCHANGE_MTU_REQ(req, APP_ULL_DONGLE_GATT_CLIENT_MTU);
    bt_gattc_exchange_mtu(ind->connection_handle, &req);

    g_app_ull_le_ctrl.is_creating_connection = false;
    if ((BT_STATUS_SUCCESS != status) || (BT_HANDLE_INVALID == ind->connection_handle)) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"LE_CONNECT_IND failed, handle:%x status:0x%x", 2, ind->connection_handle, status);
        /** Connect Fail, re-start scan and connect. */
        switch (g_app_ull_le_ctrl.curr_conn) {
            case APP_ULL_DONGLE_LE_CONN_BONDED_DEVICE: {
                app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE);
                break;
            }
            case APP_ULL_DONGLE_LE_CONN_RECONNECT_DEVICE: {
                app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_RECONNECT_DEVICE);
                break;
            }
            case APP_ULL_DONGLE_LE_CONN_NEW_DEVICE: {
                app_ull_dongle_le_scan_new_device();
                break;
            }
            case APP_ULL_DONGLE_LE_CONN_COORDINATED_SET_BY_SIRK: {
                app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
                break;
            }
            default:
                break;
        }
        bt_bd_addr_t zero_addr = {0};
        if (memcmp(&zero_addr, &(ind->peer_addr.addr), sizeof(bt_bd_addr_t))) {
            if (g_ull_dongle_le_race_callback) {
                app_dongle_le_race_connect_ind_t connect_ind;
                connect_ind.ret = status;
                connect_ind.group_id = 0xFF;
                connect_ind.group_size = 0;
                memcpy(&(connect_ind.peer_addr), &(ind->peer_addr), sizeof(bt_addr_t));
                g_ull_dongle_le_race_callback(APP_DONGLE_LE_RACE_EVT_CONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2, &connect_ind);
            }
        }


        return;
    }
    bool is_pairing = app_dongle_cm_is_enter_pairing_mode();
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"LE_CONNECT_IND, handle:0x%x status:0x%x, is pairing: %d", 3, ind->connection_handle, status, is_pairing);
#ifdef APP_ULL_DONGLE_AIR_PAIRING_CLEAR_BOND_INFO
    if (is_pairing) {
        //app_ull_dongle_le_reset_bonded_info();
        bt_gap_le_set_white_list(BT_GAP_LE_CLEAR_WHITE_LIST, NULL);
        if (NULL != g_app_ull_le_bonded_info.addr && g_app_ull_le_bonded_info.num != 0) {
            bt_addr_t tempaddr = {0};
            uint8_t *ptempaddr = NULL;
            uint8_t i = g_app_ull_le_bonded_info.num;
            while (i > 0) {
                i--;
                ptempaddr = (uint8_t *)&g_app_ull_le_bonded_info.addr[i];
                tempaddr.type = 0x0;
                memcpy(&(tempaddr.addr), ptempaddr, sizeof(bt_bd_addr_t));
                bt_gap_le_srv_link_t link_type_mask = bt_device_manager_le_get_link_type_by_addr(&tempaddr.addr);
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"air pairing , delete bonded_list[%x], link type: %x, addr:%02x:%02x:%02x:%02x:%02x:%02x", 8, i, link_type_mask, ptempaddr[5], ptempaddr[4],
                                                                                                    ptempaddr[3], ptempaddr[2],
                                                                                                    ptempaddr[1], ptempaddr[0]);
                /* rm from bonded list */
                if (0 != memcmp(&(tempaddr.addr), &(ind->peer_addr.addr), sizeof(bt_bd_addr_t))) {
                    if (link_type_mask & BT_GAP_LE_SRV_LINK_TYPE_ULL_V2)
                    {
                        bt_device_manager_le_remove_bonded_device(&tempaddr);
                    }
                }
                /* update bonded info */
            }
            app_ull_dongle_le_update_bonded_info();
        }
    }
#endif
    /** Connect Successful, save connection info. */
    g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_NONE;
    app_ull_dongle_le_save_link_info(ind);
}

static void app_ull_dongle_le_handle_conn_update_ind(bt_status_t status, bt_gap_le_connection_update_ind_t *ind)
{
    app_ull_dongle_le_link_info_t *p_info = NULL;

    if ((!ind) || (NULL == (p_info = app_ull_dongle_le_get_link_info(ind->conn_handle)))) {
        return;
    }
    p_info->conn_interval = ind->conn_interval;

    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"LE_CONN_UPDATE_IND, interval:%d", 1, p_info->conn_interval);
}

static void app_ull_dongle_le_handle_set_white_list_cnf(bt_status_t status)
{
    if (APP_ULL_DONGLE_LE_STATE_ENABLED != g_app_ull_le_ctrl.ull_state) {
        return;
    }

    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] LE_SET_WHITE_LIST_CNF status:0x%x, curr_state:%d", 2, status, g_app_ull_le_set_white_list.state);
    if (APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_ADD_ON_GOING == g_app_ull_le_set_white_list.state) {
        app_ull_dongle_le_add_bonded_device_to_white_list(g_app_ull_le_set_white_list.curr_idx + 1);
    } else if (APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_REMOVE_ON_GOING == g_app_ull_le_set_white_list.state) {
        g_app_ull_le_set_white_list.state = APP_ULL_DONGLE_LE_SET_WHITE_LIST_STATE_COMPLETE;
    }
}

static void app_ull_dongle_le_handle_bonding_complete_ind(bt_status_t status, bt_gap_le_bonding_complete_ind_t *ind)
{
    app_ull_dongle_le_link_info_t *p_info = NULL;
    if (!ind) {
        return;
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"LE_BONDING_COMPLETE_IND, handle:%x ret:%x", 2, ind->handle, status);
    if ((NULL == (p_info = app_ull_dongle_le_get_link_info(ind->handle))) || (BT_STATUS_SUCCESS != status)) {
        return;
    }
    app_ull_dongle_le_update_bonded_info();

    bt_ull_le_find_cs_info_t cs_req = {
            .duration = APPS_ULL_LE_FIND_CS_DURATION,//30s
            .conn_handle = ind->handle,
    };

    if (BT_STATUS_SUCCESS == bt_ull_action(BT_ULL_ACTION_FIND_CS_INFO, &cs_req, sizeof(bt_ull_le_find_cs_info_t))) {
        /** waiting for ULL Pairing Complete IND.*/
        /**Waiting ULL Connected ind and start read CSS and SIRK.*/
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Finding CS info", 0);
    } else {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"Finding CS info FAIL", 0);
    }
}

void app_ull_dongle_le_reverse_copy(uint8_t *dst, uint8_t *src, uint32_t len)
{
    uint8_t i = 0;

    for (i = 0; i < len; i++) {
        dst[i] = src[len - 1 - i];
    }
}

static void app_ull_dongle_le_handle_ull_connect_ind(bt_ull_le_connected_info_t *ind)
{
    app_ull_dongle_le_link_info_t *p_info = NULL;
    char conn_string[60] = {0};
    uint8_t link_num;
    if (!ind) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"ULL Connect IND, Null Pointer!", 0);
        return ;
    }

    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Group device addr:type: %d, ull_state: %d, addr: %x-%x-%x-%x-%x-%x", 8, ind->group_device_addr[0].type,
        g_app_ull_le_ctrl.ull_state,
        ind->group_device_addr[0].addr[0],
        ind->group_device_addr[0].addr[1],
        ind->group_device_addr[0].addr[2],
        ind->group_device_addr[0].addr[3],
        ind->group_device_addr[0].addr[4],
        ind->group_device_addr[0].addr[5]
        );
    if (BT_HANDLE_INVALID == ind->conn_handle) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"ULL Connect IND, invalid Params!", 0);
        return ;
    }

    if (NULL == (p_info = (app_ull_dongle_le_get_link_info(ind->conn_handle)))) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"ULL Connect IND, Get link info fail!", 0);
        return;
    }

    if (BT_STATUS_SUCCESS != ind->status) {
        link_num = app_ull_dongle_le_get_link_num();
        if (link_num == 0x00 && g_app_ull_le_ctrl.ull_state == APP_ULL_DONGLE_LE_STATE_ENABLING) {
            APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"ULL Source Start Fail!!", 0);
            g_app_ull_le_ctrl.ull_state = APP_ULL_DONGLE_LE_STATE_DISABLED;
            app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V2, APP_DONGLE_CM_EVENT_SOURCE_STARTED, BT_STATUS_FAIL, NULL);
            app_ull_dongle_le_stop_scan();
            return;
        } else if (link_num == 0x00 && g_app_ull_le_ctrl.ull_state == APP_ULL_DONGLE_LE_STATE_ENABLED) {
#ifdef AIR_WIRELESS_MIC_ENABLE
            APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"Wireless mic Connect fail, reconnect!!", 0);
            app_ull_dongle_le_start_scan();
            return;
#endif
        } else {
            APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"ULL Connect fail!, status is 0x%x, to disconnect LE ACL", 1, ind->status);
            if (g_ull_dongle_le_race_callback) {
                app_dongle_le_race_connect_ind_t connect_ind;
                connect_ind.ret = BT_STATUS_FAIL;
                connect_ind.group_id = 0xFF;
                connect_ind.group_size = 0;
                memcpy(&(connect_ind.peer_addr), &(p_info->addr), sizeof(bt_addr_t));
                g_ull_dongle_le_race_callback(APP_DONGLE_LE_RACE_EVT_CONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2, &connect_ind);
            }
            
            /** To disconnect the current link. */
            app_ull_dongle_le_disconnect(ind->conn_handle);
            return;
        }
    }

    if (BT_STATUS_SUCCESS == ind->status) {
        if ((0 != g_app_ull_le_ctrl.set_size) && (g_app_ull_le_ctrl.set_size != ind->set_size)) {// not the fisrt CS, need have same CS size
            APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"ULL Connect fail! CS size error! To disconnect LE ACL", 0);
            if (g_ull_dongle_le_race_callback) {
                app_dongle_le_race_connect_ind_t connect_ind;
                connect_ind.ret = BT_STATUS_FAIL;
                connect_ind.group_id = 0xFF;
                connect_ind.group_size = 0;
                memcpy(&(connect_ind.peer_addr), &(p_info->addr), sizeof(bt_addr_t));
                g_ull_dongle_le_race_callback(APP_DONGLE_LE_RACE_EVT_CONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2, &connect_ind);
            }

            /** To disconnect the current link. */
            app_ull_dongle_le_disconnect(ind->conn_handle);
            return;
        }
#ifdef AIR_WIRELESS_MIC_ENABLE
        if ((!app_ull_dongle_le_sirk_is_null()) &&
            (0 != memcmp((uint8_t *)&g_app_ull_le_ctrl.sirk, &(ind->sirk), sizeof(bt_key_t)))) {
            APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"ULL Connect fail! CS Sirk error! To disconnect LE ACL", 0);
            if (g_ull_dongle_le_race_callback) {
                app_dongle_le_race_connect_ind_t connect_ind;
                connect_ind.ret = BT_STATUS_FAIL;
                connect_ind.group_id = 0xFF;
                connect_ind.group_size = 0;
                memcpy(&(connect_ind.peer_addr), &(p_info->addr), sizeof(bt_addr_t));
                g_ull_dongle_le_race_callback(APP_DONGLE_LE_RACE_EVT_CONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2, &connect_ind);
            }

            /** To disconnect the current link. */
            app_ull_dongle_le_disconnect(ind->conn_handle);
            return;
        }
#endif        
        if (g_ull_dongle_le_race_callback) {
            app_dongle_le_race_connect_ind_t connect_ind;
            connect_ind.ret = ind->status;
            connect_ind.group_id = 0x01;
            connect_ind.group_size = ind->set_size;
            memcpy(&(connect_ind.peer_addr), &(p_info->addr), sizeof(bt_addr_t));
            g_ull_dongle_le_race_callback(APP_DONGLE_LE_RACE_EVT_CONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2, &connect_ind);
        }
        p_info->is_ull_connected = true;
        g_app_ull_le_ctrl.client_type = ind->client_type;
        g_app_ull_le_ctrl.set_size = ind->set_size;
        memcpy((uint8_t *)&g_app_ull_le_ctrl.sirk, (uint8_t *)&(ind->sirk), sizeof(bt_key_t));
        app_ull_dongle_le_update_conn_param(ind->conn_handle, APP_ULL_DONGLE_LE_CONN_PARAM_DEFAULT_SPEED);
        /* add to white list */
        if (app_ull_dongle_le_is_bonded_device(&(p_info->addr))) {
            bt_status_t ret = bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &(p_info->addr));
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] add_white_list, ret:%x", 1, ret);
        }
#if defined (MTK_RACE_CMD_ENABLE) && defined (AIR_WIRELESS_MIC_ENABLE)
        app_dongle_race_ull_connect_proc(true, ind->conn_handle);
#endif
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"SIRK: %2x-%2x-%2x-%2x-%2x-%2x-%2x-%2x", 8, g_app_ull_le_ctrl.sirk[0], g_app_ull_le_ctrl.sirk[1], g_app_ull_le_ctrl.sirk[2], g_app_ull_le_ctrl.sirk[3],
            g_app_ull_le_ctrl.sirk[4], g_app_ull_le_ctrl.sirk[5], g_app_ull_le_ctrl.sirk[6], g_app_ull_le_ctrl.sirk[7]);
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"SIRK: %2x-%2x-%2x-%2x-%2x-%2x-%2x-%2x", 8, g_app_ull_le_ctrl.sirk[8], g_app_ull_le_ctrl.sirk[9], g_app_ull_le_ctrl.sirk[10], g_app_ull_le_ctrl.sirk[11],
            g_app_ull_le_ctrl.sirk[12], g_app_ull_le_ctrl.sirk[13], g_app_ull_le_ctrl.sirk[14], g_app_ull_le_ctrl.sirk[15]);

        snprintf((char *)conn_string, 60, "Coordinated set size[%d], handle:%x", ind->set_size, ind->conn_handle);
        bt_app_common_at_cmd_print_report(conn_string);
        link_num = app_ull_dongle_le_get_link_num();
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"ULL_CONNECT_IND, handle:%x conn_type:%x link_num:%x set_size:%x", 4, ind->conn_handle, g_app_ull_le_ctrl.curr_conn, link_num, g_app_ull_le_ctrl.set_size);

        if (1 == link_num && g_app_ull_le_ctrl.ull_state == APP_ULL_DONGLE_LE_STATE_ENABLING) {/**the  firsrt connection always trigger by app dongle cm module, so notify it the result */
            g_app_ull_le_ctrl.ull_state = APP_ULL_DONGLE_LE_STATE_ENABLED;
            app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V2, APP_DONGLE_CM_EVENT_SOURCE_STARTED, BT_STATUS_SUCCESS, NULL);
        }

        if ((BT_ULL_LE_MAX_LINK_NUM == link_num) || (g_app_ull_le_ctrl.set_size == link_num)) {
            g_connect_idx = 0;
            g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_NONE;
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"ALL CS device have been connected, Good!", 0);

            if (APP_ULL_DONGLE_LE_SCAN_NONE != g_app_ull_le_ctrl.curr_scan) {
                /** stop all scan when connection num is MAX. */
                app_ull_dongle_le_stop_scan();
            }
            return;
        }

        /* search next cs device */
        if (g_app_ull_le_ctrl.set_size != link_num) {
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Scan more CS device, %d devices waiting for connect", 1, g_app_ull_le_ctrl.set_size - link_num);
#if defined (AIR_WIRELESS_MIC_ENABLE)
            app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
#else
            bt_bd_addr_t zero_addr = {0};
            if (!memcmp((uint8_t *)&ind->group_device_addr[g_connect_idx].addr, (uint8_t *)&zero_addr, sizeof(bt_bd_addr_t))) {
                APPS_LOG_MSGID_E(APP_ULL_LOG_TAG", app_ull_le_set_group_device_addr, addr is zero", 0);
                app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
                return;
            } else if (g_app_ull_le_ctrl.set_size > 1) {
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Start create connection,idx: %d, type: %d, addr: %x-%x-%x-%x-%x-%x", 8, g_connect_idx, \
                    ind->group_device_addr[g_connect_idx].type, ind->group_device_addr[g_connect_idx].addr[0], ind->group_device_addr[g_connect_idx].addr[1], ind->group_device_addr[g_connect_idx].addr[2],
                    ind->group_device_addr[g_connect_idx].addr[3], ind->group_device_addr[g_connect_idx].addr[4], ind->group_device_addr[g_connect_idx].addr[5]);
                bt_addr_t connect_addr = {
                    .type = ind->group_device_addr[g_connect_idx].type,
                    .addr = {0}
                };

                app_ull_dongle_le_reverse_copy((uint8_t *)&connect_addr.addr, (uint8_t *)&ind->group_device_addr[g_connect_idx].addr, sizeof(bt_bd_addr_t));
                if (!app_ull_dongle_le_connect(&connect_addr)) {
                    g_connect_idx ++;
                    if (g_connect_idx >= (BT_ULL_LE_MAX_LINK_NUM - 1)) {
                        g_connect_idx = 0;
                    }
                } else {
                    app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
                }
            }

#endif
        }
    }

}

bt_status_t app_ull_dongle_le_cancel_create_connection(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG" app_ull_dongle_le_cancel_create_connection, %x", 1, g_app_ull_le_ctrl.is_creating_connection);
    if (g_app_ull_le_ctrl.is_creating_connection) {
        status = bt_gap_le_cancel_connection();
    }
    return status;
}
static void app_ull_dongle_le_handle_disconnect_ind(bt_status_t status, bt_gap_le_disconnect_ind_t *ind)
{
    uint8_t link_idx, link_num;
    bt_addr_t remote_addr = {0};
    app_dongle_le_race_disconnect_ind_t dis_ind;

    if (!ind) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"LE_DISCONNECT_IND Fail, Null pointer", 0);
        return;
    }
    if (BT_STATUS_SUCCESS != status) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"LE_DISCONNECT_IND Fail, handle:%x status:%x", 2, ind->connection_handle, status);
        return;
    }
    if (APP_ULL_DONGLE_LE_INVALID_LINK_IDX == (link_idx = app_ull_dongle_le_get_link_idx(ind->connection_handle))) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"LE_DISCONNECT_IND, link not exist (hdl:%x)", 1, ind->connection_handle);
        return;
    }
    dis_ind.ret = status;
    memcpy(&(dis_ind.peer_addr), &(g_app_ull_le_link_info[link_idx].addr), sizeof(bt_addr_t));
    dis_ind.group_id = 0x01;
    dis_ind.group_size = g_app_ull_le_ctrl.set_size;

    if (app_ull_dongle_le_get_link_info(ind->connection_handle)) {
        memcpy(&remote_addr, &(app_ull_dongle_le_get_link_info(ind->connection_handle)->addr), sizeof(bt_addr_t));
    }

    app_ull_dongle_le_reset_link_info(link_idx);

    if (APP_ULL_DONGLE_LE_STATE_DISABLED == g_app_ull_le_ctrl.ull_state) {
        return;
    }

#if defined (MTK_RACE_CMD_ENABLE) && defined (AIR_WIRELESS_MIC_ENABLE)
        app_dongle_race_ull_connect_proc(false, ind->connection_handle);
#endif

    if (g_ull_dongle_le_race_callback) {
        g_ull_dongle_le_race_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2, &dis_ind);
    }

    link_num = app_ull_dongle_le_get_link_num();
    if (link_num == 0x00) {
        g_connect_idx = 0;
    }
#ifndef AIR_WIRELESS_MIC_ENABLE
    /* Reconnect the disconnected device */
    if ((BT_HCI_STATUS_CONNECTION_TIMEOUT == ind->reason || BT_HCI_STATUS_LMP_RESPONSE_TIMEOUT_OR_LL_RESPONSE_TIMEOUT == ind->reason)\
        && BT_ULL_EARBUDS_CLIENT == g_app_ull_le_ctrl.client_type \
        && link_num == 0x01) {
        if (!app_ull_dongle_le_connect(&remote_addr)) {
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Linkloss, Reconnect-1, Start Connect", 0);
            return;
        }
    }
#endif
    if (app_ull_dongle_le_is_bonded_device(&remote_addr)) {
    #ifndef AIR_WIRELESS_MIC_ENABLE
        if (BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES == ind->reason) {
            //app_ull_dongle_le_add_black_list(&remote_addr);

            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Disconnect due low resource(0x14), remove device from White List, Please Plug in again to reconnect", 0);
            if (0 == link_num) {
                app_ull_dongle_le_cancel_create_connection();
                app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_RECONNECT_DEVICE);

/*
                if (APP_ULL_DONGLE_LE_SCAN_NONE != g_app_ull_le_ctrl.curr_scan) {
                    g_app_ull_le_ctrl.ull_state = APP_ULL_DONGLE_LE_STATE_DISABLING;
                    app_ull_dongle_le_stop_scan();
                } else {
                    g_app_ull_le_ctrl.ull_state = APP_ULL_DONGLE_LE_STATE_DISABLED;
                    app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V2, APP_DONGLE_CM_EVENT_SOURCE_END, status, NULL);
                }
*/
            }
            return;
        }
    #endif
        if ((BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST == ind->reason) ||
            (BT_HCI_STATUS_LIMIT_REACHED == ind->reason)) {//for BTA-36375
#ifdef AIR_WIRELESS_MIC_ENABLE
            app_ull_dongle_le_add_black_list(&remote_addr);
#endif
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Disconnect by local host(0x16), remove device from White List, Please Plug in again to reconnect, %x, %x, %x", 3, \
                g_app_ull_le_ctrl.ull_state, link_num, g_app_ull_le_ctrl.curr_scan);
            if ((APP_ULL_DONGLE_LE_STATE_DISABLING == g_app_ull_le_ctrl.ull_state) && (0 == link_num)) {
                if (APP_ULL_DONGLE_LE_SCAN_NONE != g_app_ull_le_ctrl.curr_scan) {
                    /** stop all scan. */
                    app_ull_dongle_le_stop_scan();
                } else {
                    g_app_ull_le_ctrl.ull_state = APP_ULL_DONGLE_LE_STATE_DISABLED;
                    app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V2, APP_DONGLE_CM_EVENT_SOURCE_END, status, NULL);
                }
            }
            return;
        }
#ifdef AIR_WIRELESS_MIC_ENABLE
        app_ull_dongle_le_start_scan();
#else
        if (BT_ULL_EARBUDS_CLIENT == g_app_ull_le_ctrl.client_type && link_num == 0x01) {
            if (!app_ull_dongle_le_connect(&remote_addr)) {
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Linkloss, Reconnect-2, Start Connect", 0);
            } else {
               APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Linkloss, Reconnect-3, Start Scan", 0);
                app_ull_dongle_le_cancel_create_connection();
                app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE);
            }
        } else {
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Linkloss, Reconnect-4, Start Scan", 0);
            app_ull_dongle_le_cancel_create_connection();
            app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE);
        }
        
#endif
    } else if (g_app_ull_le_ctrl.set_size > link_num) {
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Linkloss, Reconnect-5!!", 0);
        app_ull_dongle_le_cancel_create_connection();
        app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
    } else {
#ifdef AIR_WIRELESS_MIC_ENABLE
        app_ull_dongle_le_start_scan();
#else

        app_ull_dongle_le_cancel_create_connection();
        app_ull_dongle_le_start_scan();
#endif
    }
    //}
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] LE_DISCONNECT_IND, Normal! handle:%x reason:%x, link num:%d, cs:%d, ct: %d", 5, \
        ind->connection_handle, ind->reason, link_num, g_app_ull_le_ctrl.set_size, g_app_ull_le_ctrl.client_type);

    /*TBD, switch Mic channel.*/
    //if ((0 != link_num) && (0 != cis_num)) {

    //}
}

static bt_status_t app_ull_dongle_le_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    //APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP] app_ull_dongle_le_event_callback, msg is 0x%4x, status is 0x%4x", 2, msg, status);

    switch (msg) {
        case BT_POWER_ON_CNF: {
            app_ull_dongle_le_handle_bt_power_on_cnf();
            break;
        }
        case BT_POWER_OFF_CNF: {
            app_ull_dongle_le_handle_bt_power_off_cnf();
            break;
        }
        //case BT_GAP_LE_SET_SCAN_CNF:
        case BT_GAP_LE_SET_EXTENDED_SCAN_CNF: {
            app_ull_dongle_le_handle_scan_cnf(status);
            break;
        }
        case BT_GAP_LE_EXT_ADVERTISING_REPORT_IND: {
            app_ull_dongle_le_handle_adv_report_ind(status, (bt_gap_le_ext_advertising_report_ind_t *)buff);
            break;
        }
        case BT_GAP_LE_SET_WHITE_LIST_CNF: {
            app_ull_dongle_le_handle_set_white_list_cnf(status);
            break;
        }
        case BT_GAP_LE_CONNECT_IND: {
            bt_gap_le_connection_ind_t *ind = (bt_gap_le_connection_ind_t *)buff;
            if (ind) {
                app_ull_dongle_le_handle_connect_ind(status, (bt_gap_le_connection_ind_t *)buff);
            }
            /** Do it in bt_app_common.c */
            //bt_gap_le_srv_bond(connection_ind->connection_handle, &pairing_config);
            break;
        }
        case BT_GAP_LE_CONNECTION_UPDATE_IND: {
            app_ull_dongle_le_handle_conn_update_ind(status, (bt_gap_le_connection_update_ind_t *)buff);
            break;
        }
        case BT_GAP_LE_BONDING_COMPLETE_IND: {
            app_ull_dongle_le_handle_bonding_complete_ind(status, (bt_gap_le_bonding_complete_ind_t *)buff);
            break;
        }
         case BT_GAP_LE_DISCONNECT_IND: {
            app_ull_dongle_le_handle_disconnect_ind(status, (bt_gap_le_disconnect_ind_t *)buff);
            break;
        }

        case BT_GAP_LE_CONNECT_CANCEL_CNF: {
            if ((BT_STATUS_SUCCESS == status) && (APP_ULL_DONGLE_LE_STATE_DISABLED != g_app_ull_le_ctrl.ull_state)) {
                //g_app_ull_le_ctrl.is_creating_connection = false;
            }
            break;
        }
        default :
            break;
    }

    return BT_STATUS_SUCCESS;
}

void app_ull_dongle_le_srv_event_callback(bt_ull_event_t event, void *param, uint32_t param_len)
{
    switch (event) {
        case BT_ULL_EVENT_LE_CONNECTED: {
            if (param && param_len) {
                bt_ull_le_connected_info_t *conn_info = (bt_ull_le_connected_info_t *)param;
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"ULL Connected", 0);
                app_ull_dongle_le_handle_ull_connect_ind(conn_info);
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
                app_power_save_utils_notify_mode_changed(FALSE, NULL);
#endif
            }
            break;
        }

        case BT_ULL_EVENT_LE_DISCONNECTED: {
            if (param && param_len) {
                bt_ull_le_disconnect_info_t *conn_info = (bt_ull_le_disconnect_info_t *)param;
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"ULL Disconnected, handle is 0x%2x", 1, conn_info->conn_handle);
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
            app_power_save_utils_notify_mode_changed(FALSE, NULL);
#endif
            }
            break;
        }

        default:
            break;
    }
}

static bt_status_t app_ull_dongle_le_enable(const bt_addr_t addr, app_dongle_cm_start_source_param_t param)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_addr_t conn_addr;
    memcpy(&conn_addr, &addr, sizeof(bt_addr_t));

    /* connect ULL device */
    if (!app_ull_dongle_le_is_connected_device(&(conn_addr))) {
        app_ull_dongle_le_print_adv_addr(&(conn_addr));
        status = app_ull_dongle_le_connect(&(conn_addr));
    }
    g_app_ull_le_ctrl.ull_state = APP_ULL_DONGLE_LE_STATE_ENABLING;
    g_app_ull_le_ctrl.curr_conn = (BT_STATUS_SUCCESS == status) ? APP_ULL_DONGLE_LE_CONN_NEW_DEVICE : APP_ULL_DONGLE_LE_CONN_NONE;
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_start status is 0x%x", 1, status);
    return status;
}

static bt_status_t app_ull_dongle_le_disable(const bt_addr_t addr, app_dongle_cm_stop_source_param_t param)
{
    bt_status_t status = BT_STATUS_FAIL;
    app_ull_dongle_le_state_t temp_state = g_app_ull_le_ctrl.ull_state;
    g_app_ull_le_ctrl.ull_state = APP_ULL_DONGLE_LE_STATE_DISABLING;
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_disable, %d, %d, %d", 3, \
        g_app_ull_le_ctrl.is_creating_connection, app_ull_dongle_le_is_connected(), g_app_ull_le_ctrl.curr_scan);
    /**< Disconnet all connection and cancel all connection on going. */
    if (g_app_ull_le_ctrl.is_creating_connection) {
        status = bt_gap_le_cancel_connection();
    } else if (app_ull_dongle_le_is_connected()) {
        status = app_ull_dongle_le_disconnect_all_device();
    } else if (APP_ULL_DONGLE_LE_SCAN_NONE != g_app_ull_le_ctrl.curr_scan) {
        status = app_ull_dongle_le_stop_scan();
    } else {/**< no running procedure. */
        g_app_ull_le_ctrl.ull_state = APP_ULL_DONGLE_LE_STATE_DISABLED;
        app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V2, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, NULL);
        return BT_STATUS_SUCCESS;
    }

    if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != status) {
        g_app_ull_le_ctrl.ull_state = temp_state;
    }
    return status;
}

static bt_status_t app_ull_dongle_le_pre_check(app_dongle_cm_precheck_data_t *check_data)
{
    bt_status_t result = BT_STATUS_FAIL;
    uint16_t length = 0, index = 0;

    if (app_ull_dongle_le_is_in_black_list(&(check_data->adv_ind.address))) {
        APPS_LOG_MSGID_W(APP_ULL_LOG_TAG"app_ull_dongle_le_pre_check fail, device is in black list!", 0);
        return BT_STATUS_FAIL;
    }

    /** Searched coordinated set device. */
    while (index < check_data->adv_ind.data_length) {
        length = check_data->adv_ind.data[index];
        if (0 == length) {
            break;
        }
        /**<Headset adv format is different with earbuds ULL adv.*/
        if ((length >= 7) && ((check_data->adv_ind.data[index+1] == 0xF0) || (check_data->adv_ind.data[index+1] == 0x2E))) { //have ull dongle special RSI info
            result = bt_ull_le_srv_verify_rsi(&check_data->adv_ind.data[index + 2]);
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"ADV report from Earbuds: %02x:%02x:%02x:%02x:%02x:%02x", 6, check_data->adv_ind.address.addr[0], check_data->adv_ind.address.addr[1],
                check_data->adv_ind.address.addr[2], check_data->adv_ind.address.addr[3], check_data->adv_ind.address.addr[4], check_data->adv_ind.address.addr[5]);
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"RSI is: %02x %02x %02x %02x %02x %02x", 6, check_data->adv_ind.data[index+2], check_data->adv_ind.data[index+3],
                check_data->adv_ind.data[index + 4], check_data->adv_ind.data[index+5], check_data->adv_ind.data[index+6], check_data->adv_ind.data[index+7]);
            if (BT_STATUS_SUCCESS == result) {
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] app_ull_dongle_le_pre_check, ULL RSI is correct", 0);
                return BT_STATUS_SUCCESS;
            }
            break;
        }

        if ((length >= 10) && ((check_data->adv_ind.data[index + 1] == 0xFF) && (check_data->adv_ind.data[index + 2] == 0x94) && (check_data->adv_ind.data[index + 5] == 0x2E))) {
            result = bt_ull_le_srv_verify_rsi(&check_data->adv_ind.data[index + 6]);
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"ADV report from Headset: %02x:%02x:%02x:%02x:%02x:%02x", 6, check_data->adv_ind.address.addr[0], check_data->adv_ind.address.addr[1],
                check_data->adv_ind.address.addr[2], check_data->adv_ind.address.addr[3], check_data->adv_ind.address.addr[4], check_data->adv_ind.address.addr[5]);
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"RSI is: %02x %02x %02x %02x %02x %02x", 6, check_data->adv_ind.data[index + 6], check_data->adv_ind.data[index + 7],
                check_data->adv_ind.data[index + 8], check_data->adv_ind.data[index + 9], check_data->adv_ind.data[index + 10], check_data->adv_ind.data[index + 11]);
            if (BT_STATUS_SUCCESS == result) {
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] app_ull_dongle_le_pre_check, ULL RSI is correct", 0);
                return BT_STATUS_SUCCESS;
            }
            break;
        }
        index = index + length + 1;
        //APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"index [%d] length [%d]", 2, index, length);
    }
    return result;
}

void app_ull_dongle_le_init(void)
{
    app_ull_dongle_le_reset_param();
    if (APP_DONGLE_CM_LINK_MODE_ULL_V2 & app_dongle_cm_get_link_mode()) {
        /**< Registe app dongle connection manager handler table. */
        app_dongle_cm_handle_t app_ull_le_cm_handler = {
            app_ull_dongle_le_enable,       /*Start connect function.*/
            app_ull_dongle_le_disable,      /*Stop all connect and scan function.*/
            app_ull_dongle_le_pre_check,    /*Check adv data RSI info function.*/
        };
        app_dongle_cm_register_handle(APP_DONGLE_CM_SOURCE_ULL_V2, &app_ull_le_cm_handler);
        /** register host event callback */
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                        (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_SYSTEM),
                                        (void *)app_ull_dongle_le_event_callback);
        bt_gap_le_srv_register_event_callback(&app_ull_dongle_gap_le_srv_event_callback);

#ifdef AIR_MS_TEAMS_ENABLE
    //app_ull_dongle_le_usb_hid_init();
#endif
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_init Success", 0);
    } else {
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_init Fail, ULL not support, link mode is 0x%x", 1, app_dongle_cm_get_link_mode());
    }
}

static void app_ull_dongle_gap_le_srv_event_callback(bt_gap_le_srv_event_t event, void *data)
{
    switch (event) {
        case BT_GAP_LE_SRV_GET_LINK_INFO: {
            bt_gap_le_srv_link_info_t *link_info = (bt_gap_le_srv_link_info_t *)data;
            bt_addr_t addr;
            memcpy(&addr, &link_info->remote_address, sizeof(bt_addr_t));
            bt_handle_t handle = app_ull_dongle_le_get_conn_handle_by_addr(&addr);
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_gap_le_srv_event_callback, event: %d, connection: %d, handle: 0x%x",
                3, event, g_app_ull_le_ctrl.is_creating_connection, handle);
            if (g_app_ull_le_ctrl.is_creating_connection || BT_HANDLE_INVALID != handle) {
                link_info->link_type = BT_GAP_LE_SRV_LINK_TYPE_ULL_V2;
            }
        } break;
        default:
            break;
    }
}

bt_status_t app_ull_dongle_le_start_scan(void)
{
#ifdef APP_ULL_DONGLE_LE_DEBUG
    app_ull_dongle_le_get_sirk(true);
#endif

    if (!app_ull_dongle_le_sirk_is_null()) {
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"scan cs device", 0);
        return app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
    } else {
        APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"scan new device", 0);
        return app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_NEW_DEVICE);
    }
    APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"Can not scan device!", 0);
    return BT_STATUS_FAIL;
}

bt_status_t app_ull_dongle_le_stop_scan(void)
{
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"stop_scan_device, scan:%x->%x conn:%x", 3, g_app_ull_le_ctrl.curr_scan, g_app_ull_le_ctrl.next_scan,
                                                                                g_app_ull_le_ctrl.curr_conn);
    if ((APP_ULL_DONGLE_LE_SCAN_ENABLING <= g_app_ull_le_ctrl.curr_scan) &&
        (APP_ULL_DONGLE_LE_SCAN_DISABLING >= g_app_ull_le_ctrl.curr_scan)) {
        return app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE);
    } else if ((APP_ULL_DONGLE_LE_SCAN_CS_ENABLING <= g_app_ull_le_ctrl.curr_scan) &&
        (APP_ULL_DONGLE_LE_SCAN_CS_DISABLING >= g_app_ull_le_ctrl.curr_scan)) {
        return app_ull_dongle_le_stop_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
    }
    return BT_STATUS_SUCCESS;
}

bool app_ull_dongle_le_is_connected(void)
{
    uint8_t i= BT_ULL_LE_MAX_LINK_NUM;
    while (i > 0) {
        i--;
        if (BT_HANDLE_INVALID == g_app_ull_le_link_info[i].handle) {
            continue;
        }

        if (g_app_ull_le_link_info[i].is_ull_connected) {
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP] exist connected_device, YES!", 0);
            return true;
        }
    }

    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP] have not connected_device, NO!", 0);
    return false;
}

/* user can remove the Bonding information by atcmd*/
/* AT+BTULL=BOND,<ACTION> */
/* <ACTION>: RM */
void app_ull_dongle_le_reset_bonded_info(void)
{
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] app_ull_dongle_le_reset_bonded_info", 0);

    bt_device_manager_le_clear_all_bonded_info();
    app_ull_dongle_le_update_bonded_info();
    bt_gap_le_set_white_list(BT_GAP_LE_CLEAR_WHITE_LIST, NULL);

    g_app_ull_le_ctrl.curr_conn = APP_ULL_DONGLE_LE_CONN_NONE;
    app_ull_dongle_le_stop_scan();
}

uint16_t app_ull_dongle_le_get_conn_handle_by_addr(bt_addr_t *addr)
{
    uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
    while (0 != i) {
        i--;
        if ((BT_HANDLE_INVALID != g_app_ull_le_link_info[i].handle) &&
            (0 == memcmp(&(g_app_ull_le_link_info[i].addr), addr, sizeof(bt_addr_t)))) {
            APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][ULL] get_device_addr, handle:%x", 1, g_app_ull_le_link_info[i].handle);
            return g_app_ull_le_link_info[i].handle;
        }
    }
    return BT_HANDLE_INVALID;
}

bt_addr_t *app_ull_dongle_le_get_bt_addr_by_conn_handle(uint16_t conn_handle)
{
    uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
    while (0 != i) {
        i--;
        if ((BT_HANDLE_INVALID != conn_handle) && (conn_handle == g_app_ull_le_link_info[i].handle)) {
            return &(g_app_ull_le_link_info[i].addr);
        }
    }

    return NULL;
}

/** To disconnect LE link, when LE link connected, but ULL le connected fail. */
static bt_status_t app_ull_dongle_le_disconnect(bt_handle_t conn_handle)
{
    bt_status_t status;
    bt_hci_cmd_disconnect_t param;

    if (BT_HANDLE_INVALID == conn_handle) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"[APP][U] disconnect, invalid handle", 0);
        return BT_STATUS_FAIL;
    }

    param.connection_handle = conn_handle;
    param.reason = BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST;
    status = bt_gap_le_disconnect(&param);
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"[APP][U] disconnect, handle:%x ret:%x", 2, param.connection_handle, status);

    return status;
}

bt_status_t app_ull_dongle_le_disconnect_device(bt_addr_t *addr)
{
    bt_handle_t conn_handle;

    if (BT_HANDLE_INVALID == (conn_handle = app_ull_dongle_le_get_conn_handle_by_addr(addr))) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"[APP][U] disconnect device by addr, invalid handle", 0);
        return BT_STATUS_FAIL;
    }

    return app_ull_dongle_le_disconnect(conn_handle);
}

/** To disconnect LE link, when LE link connected, but ULL le connected fail. */
bt_status_t app_ull_dongle_le_disconnect_all_device(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t i = BT_ULL_LE_MAX_LINK_NUM;

    while (0 != i) {
        i--;
        if (BT_HANDLE_INVALID != g_app_ull_le_link_info[i].handle) {
            status = app_ull_dongle_le_disconnect(g_app_ull_le_link_info[i].handle);
            if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != status) {
                status = BT_STATUS_FAIL;
            } else {
                status = BT_STATUS_SUCCESS;
            }
        }
    }
    return status;
}

bool app_ull_dongle_le_is_streaming(void)
{
    uint8_t i, playing_mask = 0;
    bt_ull_streaming_info_t info = {0};
    bt_status_t ret = BT_STATUS_FAIL;

    bt_ull_streaming_t streaming[3] = {
        {//streaming_mic
            .streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE,
            .port = 0,
        },
        {//streaming_chat
            .streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER,
            .port = 0,
        },
        {//streaming_spk
            .streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER,
            .port = 1,
        }
    };

    for (i = 0; i < 3; i++) {
        ret = bt_ull_le_srv_get_streaming_info(streaming[i], &info);
        if ((BT_STATUS_SUCCESS == ret) && (info.is_playing)) {
            playing_mask |= 0x01;
        }
        memset(&info, 0x0, sizeof(bt_ull_streaming_info_t));
    }

    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_is_streaming playing_mask: %d", 1, playing_mask);
    if (0 != playing_mask) {
        return true;
    }
    return FALSE;
}

void app_ull_dongle_le_delete_device(bt_addr_t *addr)
{
    /* disconnect device */
    app_ull_dongle_le_disconnect_device(addr);
    /* rm from white list */
    bt_gap_le_set_white_list(BT_GAP_LE_REMOVE_FROM_WHITE_LIST, addr);
    /* rm from bonded list */
    bt_device_manager_le_remove_bonded_device(addr);
    /* update bonded info */
    app_ull_dongle_le_update_bonded_info();
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_delete_device done!", 0);
}

bt_status_t app_ull_dongle_le_get_connected_device_list(bt_addr_t *list, uint8_t *count)
{
    uint8_t i, connected_num = 0;;
    uint8_t buff_size = *count;
    bt_addr_t *p = list;

    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_get_bonded_list, want_read_count is %d", 1, *count);

    if ((NULL == list) || (0 == buff_size)) {
        APPS_LOG_MSGID_E(APP_ULL_LOG_TAG"buffer is empty!", 0);
        return BT_STATUS_FAIL;
    }
    for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
        if (BT_HANDLE_INVALID != g_app_ull_le_link_info[i].handle) {
            memcpy(p, &(g_app_ull_le_link_info[i].addr), sizeof(bt_addr_t));
            connected_num ++;
            if (buff_size == connected_num) {
                break;
            }
            p++;
        }
    }
    *count = connected_num;
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_get_connected_device_list, real_read_count is %d", 1, *count);
    return BT_STATUS_SUCCESS;
}

void app_ull_dongle_le_register_race_callback(app_dongle_le_race_event_callback_t callback)
{
    g_ull_dongle_le_race_callback = callback;
}

#ifdef MTK_RACE_CMD_ENABLE
void app_ull_dongle_le_get_device_status_handler(uint8_t race_channel, app_dongle_le_race_get_device_status_cmd_t *cmd)
{
    RACE_ERRCODE race_ret;
    bt_handle_t handle = app_ull_dongle_le_get_conn_handle_by_addr(&cmd->addr);
    app_ull_dongle_le_link_info_t *link_info = app_ull_dongle_le_get_link_info(handle);
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG" get link_info is 0x%x", 1, link_info);
    app_dongle_le_race_get_device_status_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                           (uint16_t)RACE_DONGLE_LE_GET_DEVICE_STATUS,
                                                                           (uint16_t)(sizeof(app_dongle_le_race_get_device_status_rsp_t)),
                                                                           race_channel);
    if (response != NULL) {
        if (link_info) {
            response->status = 0;
            memcpy(&response->addr, &link_info->addr, sizeof(bt_addr_t));
            response->device_id = race_get_device_id_by_conn_address(&response->addr.addr);
            response->group_id = 0x01;
            response->role = app_le_audio_air_get_role(&response->addr);
        } else {
            response->status = APP_DONGLE_LE_RACE_GET_STATUS_NOT_CONNECTED_STATUS;
            memcpy(&response->addr, &cmd->addr, sizeof(bt_addr_t));
            response->device_id = 0xFF;
            response->group_id = 0xFF;
            response->role = 0xFF;
        }
        race_ret = race_noti_send(response, race_channel, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }

}
void app_ull_dongle_le_get_device_list_handler(uint8_t race_channel)
{
    app_ull_dongle_le_link_info_t *link_info = NULL;
    uint32_t count = app_ull_dongle_le_get_link_num();
    uint32_t position = 0;
    RACE_ERRCODE race_ret;
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG" get link_info list count %d", 1, count);
    app_dongle_le_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                         (uint16_t)RACE_DONGLE_LE_GET_DEVICE_LIST,
                                                                         (uint16_t)(sizeof(app_dongle_le_race_get_device_list_rsp_t) + count * sizeof(app_dongle_le_race_device_status_item_t)),
                                                                         race_channel);
    if (response != NULL) {
        response->status = 0;
        for (uint32_t i = 0; i < BT_ULL_LE_MAX_LINK_NUM && position < count; i++) {
            link_info = &g_app_ull_le_link_info[i];
            if (link_info && BT_HANDLE_INVALID != link_info->handle) {
                memcpy(&response->devices_list[position].addr, &link_info->addr, sizeof(bt_addr_t));
                response->devices_list[position].device_id = race_get_device_id_by_conn_address(&link_info->addr.addr);
                response->devices_list[position].group_id = 0x01;
                response->devices_list[position].role = app_le_audio_air_get_role(&link_info->addr);
                position++;
            }
        }
        race_ret = race_noti_send(response, race_channel, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }

}
void app_ull_dongle_le_switch_active_device_handler(uint8_t race_channel, app_dongle_le_race_switch_active_audio_cmd_t *cmd)
{
    RACE_ERRCODE race_ret;
    app_dongle_le_race_switch_active_audio_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                         (uint16_t)RACE_DONGLE_LE_SWITCH_ACTIVE_DEVICE,
                                                                         (uint16_t)(sizeof(app_dongle_le_race_switch_active_audio_rsp_t)),
                                                                          race_channel);

    if (response) {
        response->set_or_get = cmd->set_or_get;
        response->status = 0x00;
        response->group_id = 0x00;
        race_ret = race_noti_send(response, race_channel, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }
}

static bt_addr_type_t app_ull_dongle_le_get_addr_type_from_bonding_list(bt_bd_addr_t *addr)
{
    bt_device_manager_le_bonded_info_t *bonded_info = NULL;

    bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext(addr);
    if (!bonded_info) {
        return 0;
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"Get addr type: %d", 1, bonded_info->bt_addr.type);
    return  bonded_info->bt_addr.type;

}

static uint8_t app_ull_dongle_le_get_bonding_num(void)
{
    bt_gap_le_srv_link_t link_type_mask = 0;
    uint8_t num = 0;
    uint8_t i = 0;
    for (i = 0; i < g_app_ull_le_bonded_info.num; i ++) {
        link_type_mask = bt_device_manager_le_get_link_type_by_addr(&g_app_ull_le_bonded_info.addr[i]);
        if (link_type_mask & BT_GAP_LE_SRV_LINK_TYPE_ULL_V2) {
            num ++;
        }
    }
    APPS_LOG_MSGID_I(APP_ULL_LOG_TAG" app_ull_dongle_le_get_bonding_num, num: %d", 1, num);
    return num;
}

static void app_ull_dongle_le_get_bonding_list(uint8_t conn_num, bt_addr_t *list)
{
    if (!list || !conn_num) {
        return;
    }
    bt_gap_le_srv_link_t link_type_mask = 0;
    bt_addr_type_t type = 0;
    uint8_t i = 0;
    uint8_t count = 0;
    for (i = 0; i < g_app_ull_le_bonded_info.num; i ++) {
        if (count < conn_num) {
            link_type_mask = bt_device_manager_le_get_link_type_by_addr(&g_app_ull_le_bonded_info.addr[i]);
            if (link_type_mask & BT_GAP_LE_SRV_LINK_TYPE_ULL_V2) {
                type = app_ull_dongle_le_get_addr_type_from_bonding_list(&g_app_ull_le_bonded_info.addr[i]);
                list[count].type = type;
                memcpy(list[count].addr, &g_app_ull_le_bonded_info.addr[i], sizeof(bt_bd_addr_t));
                APPS_LOG_MSGID_I(APP_ULL_LOG_TAG"app_ull_dongle_le_get_bonding_list[%x], addr:%02x:%02x:%02x:%02x:%02x:%02x", 7, count, list[count].addr[5], list[count].addr[4],
                                                                                                    list[count].addr[3], list[count].addr[2],
                                                                                                    list[count].addr[1], list[count].addr[0]);
                count ++;
            }
        } else {
            break;
        }
    }
}

void app_ull_dongle_le_get_paired_device_list_handler(uint8_t race_channel)
{
    RACE_ERRCODE race_ret;
    uint8_t count = app_ull_dongle_le_get_bonding_num();
    bt_addr_t *addr = NULL;
    uint32_t i;
    if (count) {
        addr = (bt_addr_t *)pvPortMalloc(count * sizeof(bt_addr_t));
        if (!addr) {
             APPS_LOG_MSGID_E(APP_ULL_LOG_TAG" app_ull_dongle_le_get_paired_device_list_handler, OOM!!", 0);
             count = 0;
        } else {
            app_ull_dongle_le_get_bonding_list(count, addr);
        }
    }
    APPS_LOG_MSGID_E(APP_ULL_LOG_TAG" app_ull_dongle_le_get_paired_device_list_handler, cunt: %d", 1, count);
    app_dongle_le_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                            (uint16_t)RACE_DONGLE_LE_GET_PAIRED_LIST,
                                                                            (uint16_t)(sizeof(app_dongle_le_race_get_device_list_rsp_t) + count * sizeof(app_dongle_le_race_device_status_item_t)),
                                                                            race_channel);
    if (response != NULL) {
        response->status = 0;
        for (i = 0; i < count; i++) {
            memcpy(&response->devices_list[i].addr, &addr[i], sizeof(bt_addr_t));
            response->devices_list[i].device_id = race_get_device_id_by_conn_address(&addr[i].addr);
            response->devices_list[i].group_id = 0x01;
            response->devices_list[i].role = app_le_audio_air_get_role(&addr[i]);
        }
        race_ret = race_noti_send(response, race_channel, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }
    if (addr) {
       vPortFree(addr); 
    }
}

#endif
#endif

