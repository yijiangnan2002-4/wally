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

 /**
  * File: app_bolt_poc_data.c
  *
  * Description: This file is the data flow for bolt poc.
  *
  * Note: todo.
  *
  */

#include "app_bolt_poc_data.h"
#include "app_bolt_poc_bt_common.h"
#include "app_bolt_poc_utility.h"
#include "app_bolt_poc_race.h"
#include "bt_app_common.h"
#include "bt_device_manager_le.h"
#include "bt_init.h"
#include "usbhid_drv.h"
#include "apps_debug.h"

#ifdef AIR_BOLT_POC_MULTI_VENDOR_SUPPORT
#include "app_bolt_poc_report_merge.h"
#endif

#pragma pack(1)
typedef struct {
    bt_addr_t addr_;
} app_conn_dev_info_t;
#pragma pack()

#pragma pack(1)
typedef struct {
    bt_addr_t addr_;
} app_bonding_dev_info_t;
#pragma pack()

#pragma pack(1)
typedef struct {
    bt_handle_t handle_;
    bt_addr_t addr_;
    bool bonding_status_;
} app_white_list_item_info_t;
#pragma pack()

#pragma pack(1)
typedef struct {
    bt_addr_t addr_;
} app_scan_dev_item_info_t;
#pragma pack()

#define APP_MAX_CONN_NUM (BT_LE_CONNECTION_NUM)
#define APP_SCAN_MAX_DEV_NUM (APP_WHITE_LIST_NUM)
#define APP_BONDING_LIST_NUM (APP_WHITE_LIST_NUM)

static volatile uint8_t g_conn_dev_num = 0;
static volatile uint8_t g_power_on_bonded_idx = 0;
static volatile uint8_t g_power_on_bonded_cnt = 0;

static bt_addr_t g_new_conn_addr;
static app_white_list_item_info_t g_new_wl_item;
static app_conn_dev_info_t g_bolt_poc_conn_info[APP_MAX_CONN_NUM];
static app_bonding_dev_info_t g_bolt_poc_bonding_list_info[APP_BONDING_LIST_NUM];
static app_white_list_item_info_t g_bolt_poc_white_list_info[APP_WHITE_LIST_NUM];
static app_scan_dev_item_info_t g_bolt_poc_scan_dev_info[APP_SCAN_MAX_DEV_NUM];


void app_bolt_poc_race_report_dev_list();

static bool app_bolt_poc_conn_list_not_full()
{
    return g_conn_dev_num < APP_MAX_CONN_NUM;
}

static bool app_bolt_poc_conn_list_not_empty()
{
    return g_conn_dev_num > 0;
}

static bool app_bolt_poc_conn_list_full()
{
    return g_conn_dev_num == APP_MAX_CONN_NUM;
}

static void app_bolt_poc_conn_list_move_forward(uint8_t cur_index)
{
    if (app_bolt_poc_conn_list_not_empty()) {
        if (cur_index < g_conn_dev_num - 1) {
            memcpy(&g_bolt_poc_conn_info[cur_index], &g_bolt_poc_conn_info[cur_index + 1], sizeof(app_conn_dev_info_t) * (g_conn_dev_num - 1 - cur_index));
        }
    }
}

static void app_bolt_poc_conn_list_add(const bt_addr_t *addr)
{
    if (addr == NULL) {
        return;
    }

    if (app_bolt_poc_conn_list_not_full()) {
        app_bolt_poc_cpy_addr(&g_bolt_poc_conn_info[g_conn_dev_num].addr_, addr);
        g_conn_dev_num++;
    }
}

static void app_bolt_poc_conn_list_replace(uint8_t cur_index, const bt_addr_t *addr)
{
    if (addr == NULL) {
        return;
    }

    if (app_bolt_poc_conn_list_not_empty()) {
        app_bolt_poc_conn_list_move_forward(cur_index);
        app_bolt_poc_cpy_addr(&g_bolt_poc_conn_info[g_conn_dev_num - 1].addr_, addr);
    }
}

static void app_bolt_poc_conn_list_remove_by_index(uint8_t cur_index)
{
    if (app_bolt_poc_conn_list_not_empty()) {
        app_bolt_poc_conn_list_move_forward(cur_index);
        app_bolt_poc_init_addr(&g_bolt_poc_conn_info[g_conn_dev_num - 1].addr_);
        g_conn_dev_num--;
    }
}

static int8_t app_bolt_poc_find_duplicate_in_conn_info(const bt_addr_t *addr)
{
    if (addr == NULL) {
        return -1;
    }

    uint8_t i = 0;
    for (i = 0; i < g_conn_dev_num; i++) {
        const bt_addr_t *conn_addr = &g_bolt_poc_conn_info[i].addr_;
        if (app_bolt_poc_addr_not_empty(conn_addr)) {
            if (app_bolt_poc_cmp_addr(addr, conn_addr)) {
                return i;
            }
        }
    }
    return -1;
}

static bool app_bolt_poc_conn_list_find_dupli_post_process(const bt_addr_t *addr)
{
    if (addr == NULL) {
        return false;
    }

    int8_t idx = app_bolt_poc_find_duplicate_in_conn_info(addr);
    if (idx >= 0) {
        app_bolt_poc_conn_list_replace(idx, addr);
        return true;
    }
    return false;
}

static void app_bolt_poc_conn_list_remove_by_addr(const bt_addr_t *addr)
{
    if (addr == NULL) {
        return;
    }

    int8_t idx = app_bolt_poc_find_duplicate_in_conn_info(addr);
    if (idx >= 0) {
        app_bolt_poc_conn_list_remove_by_index(idx);
    }
}

static bt_handle_t app_bolt_poc_find_handle_by_address(const bt_addr_t *addr)
{
    if (addr == NULL) {
        return BT_HANDLE_INVALID;
    }

    uint8_t i = 0;
    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        const app_white_list_item_info_t *item = &g_bolt_poc_white_list_info[i];
        if (app_bolt_poc_addr_not_empty(&item->addr_)) {
            if (app_bolt_poc_cmp_addr(addr, &item->addr_)) {
                return item->handle_;
            }
        }
    }
    return BT_HANDLE_INVALID;
}

static bt_handle_t app_bolt_poc_get_pop_handle(bt_handle_t handle)
{
    const bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(handle);
    if (conn_info == NULL) {
        APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_get_pop_handle bt_gap_le_srv_get_conn_info null", 0);
        return BT_HANDLE_INVALID;
    }
    
    if (app_bolt_poc_conn_list_find_dupli_post_process(&conn_info->peer_addr)) {
        return BT_HANDLE_INVALID;
    }

    app_bolt_poc_conn_list_add(&conn_info->peer_addr);

    if (app_bolt_poc_conn_list_full()) {
        //APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_get_pop_handle app_bolt_poc_conn_list_full", 0);
        return app_bolt_poc_find_handle_by_address(&g_bolt_poc_conn_info[0].addr_);
    }

    return BT_HANDLE_INVALID;
}

static uint8_t app_bolt_poc_find_del_index_in_white_list()
{
    uint8_t i = 0;
    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        int8_t idx = app_bolt_poc_find_duplicate_in_conn_info(&g_bolt_poc_white_list_info[i].addr_);
        if (idx < 0) {
            return i;
        }
    }

    return 0;
}

static uint8_t app_bolt_poc_find_del_index_in_scan_list()
{
    uint8_t i = 0;
    for (i = 0; i < APP_SCAN_MAX_DEV_NUM; i++) {
        int8_t idx = app_bolt_poc_find_duplicate_in_conn_info(&g_bolt_poc_scan_dev_info[i].addr_);
        if (idx < 0) {
            return i;
        }
    }

    return 0;
}

const bt_addr_t *app_bolt_poc_find_address_by_handle(bt_handle_t handle)
{
    uint8_t i = 0;
    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        if (handle == g_bolt_poc_white_list_info[i].handle_) {
            return &g_bolt_poc_white_list_info[i].addr_;
        }
    }
    return NULL;
}

static bool app_bolt_poc_clear_handle(bt_handle_t handle)
{
    for (uint8_t i = 0; i < APP_WHITE_LIST_NUM; i++) {
        app_white_list_item_info_t *item = &g_bolt_poc_white_list_info[i];
        if (handle == item->handle_) {
            item->handle_ = BT_HANDLE_INVALID;
            return true;
        }
    }
    return false;
}

static void app_bolt_poc_save_adv_address(const bt_addr_t *addr)
{
    if (addr == NULL) {
        return;
    }

    uint8_t i = 0;
    for (i = 0; i < APP_SCAN_MAX_DEV_NUM; i++) {
        app_scan_dev_item_info_t *item = &g_bolt_poc_scan_dev_info[i];
        if (app_bolt_poc_addr_not_empty(&item->addr_)) {
            if (app_bolt_poc_cmp_addr(&item->addr_, addr)) {
                break;
            }
        } else {
            app_bolt_poc_cpy_addr(&item->addr_, addr);
            break;
        }
    }

    if (i == APP_SCAN_MAX_DEV_NUM) {
        const uint8_t idx = app_bolt_poc_find_del_index_in_scan_list();
        app_scan_dev_item_info_t *item = &g_bolt_poc_scan_dev_info[idx];
        app_bolt_poc_init_addr(&item->addr_);
    }
}

static void app_bolt_poc_clear_wl_item(app_white_list_item_info_t *item)
{
    app_bolt_poc_init_addr(&item->addr_);
    item->handle_ = BT_HANDLE_INVALID;
    item->bonding_status_ = false;
}

static bt_addr_type_t app_bolt_poc_get_addr_type_from_bonding_list(bt_bd_addr_t *addr)
{
    bt_device_manager_le_bonded_info_t *bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext(addr);
    if (!bonded_info) {
        return 0;
    }
    //APPS_LOG_MSGID_I(" [BOLT_POC] Get addr type: %d", 1, bonded_info->bt_addr.type);
    return  bonded_info->bt_addr.type;
}

static bool app_bolt_poc_check_white_list_dev_conn_full()
{
    uint8_t i = 0;
    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        const app_white_list_item_info_t *item = &g_bolt_poc_white_list_info[i];
        if (app_bolt_poc_addr_not_empty(&item->addr_) && (item->handle_ == BT_HANDLE_INVALID)) {
            return false;
        }
    }
    return true;
}

static int8_t app_bolt_poc_find_idx_in_wl_list(const bt_addr_t *addr)
{
    if (addr == NULL) {
        return -1;
    }

    uint8_t i = 0;
    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        const bt_addr_t *bt_addr = &g_bolt_poc_white_list_info[i].addr_;
        if (app_bolt_poc_addr_not_empty(bt_addr)) {
            if (app_bolt_poc_cmp_addr(addr, bt_addr)) {
                return i;
            }
        }
    }
    return -1;
}

static int8_t app_bolt_poc_find_add_idx(const bt_addr_t *addr)
{
    if (addr == NULL) {
        return -1;
    }

    uint8_t i = 0;
    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        const bt_addr_t *bt_addr = &g_bolt_poc_white_list_info[i].addr_;
        if (!app_bolt_poc_addr_not_empty(bt_addr)) {
            return i;
        }
    }
    return -1;
}

static bool app_bolt_poc_add_to_local_white_list(const bt_addr_t *addr, bt_handle_t handle)
{
    const int8_t idx = app_bolt_poc_find_add_idx(addr);
    if (idx < 0) {
        return false;
    }

    app_white_list_item_info_t *item = &g_bolt_poc_white_list_info[idx];
    app_bolt_poc_cpy_addr(&item->addr_, addr);
    item->handle_ = handle;
    app_bolt_poc_race_report_dev_list();
    return true;
}

static void app_bolt_poc_update_local_white_list(int8_t idx, bt_handle_t handle)
{
    app_white_list_item_info_t *item = &g_bolt_poc_white_list_info[idx];
    item->handle_ = handle;
    app_bolt_poc_race_report_dev_list();
}

static void app_bolt_poc_remove_from_local_white_list(uint8_t idx)
{
    app_bolt_poc_clear_wl_item(&g_bolt_poc_white_list_info[idx]);
    app_bolt_poc_race_report_dev_list();
}

static void app_bolt_poc_white_list_full_process()
{
    const uint8_t idx = app_bolt_poc_find_del_index_in_white_list();
    const bt_addr_t *bt_addr = &g_bolt_poc_white_list_info[idx].addr_;

#ifdef AIR_BOLT_POC_MULTI_VENDOR_SUPPORT
    app_bolt_poc_del_dev_info(bt_addr);
#endif

    app_bolt_poc_remove_white_list(bt_addr);
    app_bolt_poc_remove_from_local_white_list(idx);
}

static uint8_t app_bolt_poc_get_bond_info()
{
    uint8_t num = bt_device_manager_le_get_bonded_number();
    if (num == 0) {
        return 0;
    }

    bt_bd_addr_t *bonded_addr = (bt_bd_addr_t *)app_bolt_poc_memory_alloc(num * sizeof(bt_bd_addr_t));
    if (bonded_addr == NULL) {
        return 0;
    }
    bt_device_manager_le_get_bonded_list(bonded_addr, &num);

    uint8_t i;
    for (i = 0; i < num; i++) {
        bt_addr_t bt_addr;
        memcpy(&bt_addr.addr, &bonded_addr[i], sizeof(bt_bd_addr_t));
        bt_addr.type = app_bolt_poc_get_addr_type_from_bonding_list(&bt_addr.addr);
        app_bolt_poc_cpy_addr(&g_bolt_poc_bonding_list_info[i].addr_, &bt_addr);
    }

    app_bolt_poc_memory_free(bonded_addr);

    return num;
}

static void app_bolt_poc_start_reconn()
{
    bt_status_t status = bt_gap_le_srv_cancel_connection();
    if (status != BT_STATUS_SUCCESS) {
        app_bolt_poc_connect_cancel_cnf();
    }
}

static void app_bolt_poc_release_keyboard()
{
    LogiBolt_Keyboard_t keyboard;
    memset(&keyboard, 0, sizeof(LogiBolt_Keyboard_t));
    usb_hid_send_LB_keyboard_report(&keyboard);
}

static bool app_bolt_poc_new_dev()
{
    return app_bolt_poc_addr_not_empty(&g_new_conn_addr);
}

static void app_bolt_poc_connect_new_dev()
{
    app_bolt_poc_conn_dev(&g_new_conn_addr);
}

static void app_bolt_poc_clear_new_conn_address()
{
    app_bolt_poc_init_addr(&g_new_conn_addr);
}

static void app_bolt_poc_clear_new_wl_address()
{
    app_bolt_poc_init_addr(&g_new_wl_item.addr_);
    g_new_wl_item.handle_ = BT_HANDLE_INVALID;
    g_new_wl_item.bonding_status_ = false;
}

static bool app_bolt_poc_add_wl_address_is_valid()
{
    return app_bolt_poc_addr_not_empty(&g_new_wl_item.addr_);
}

static void app_bolt_poc_set_new_wl_handle(bt_handle_t handle)
{
    g_new_wl_item.handle_ = handle;
}

static bt_addr_t *app_bolt_poc_get_add_wl_item_addr()
{
    return &g_new_wl_item.addr_;
}

static bt_handle_t app_bolt_poc_get_add_wl_item_handle()
{
    return g_new_wl_item.handle_;
}

static void app_bolt_poc_data_init()
{
    g_conn_dev_num = 0;
    g_power_on_bonded_idx = 0;
    g_power_on_bonded_cnt = 0;

    app_bolt_poc_clear_new_conn_address();
    app_bolt_poc_clear_new_wl_address();

    uint8_t i = 0;
    for (i = 0; i < APP_MAX_CONN_NUM; i++) {
        app_bolt_poc_init_addr(&g_bolt_poc_conn_info[i].addr_);
    }

    for (i = 0; i < APP_BONDING_LIST_NUM; i++) {
        app_bolt_poc_init_addr(&g_bolt_poc_bonding_list_info[i].addr_);
    }

    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        app_bolt_poc_clear_wl_item(&g_bolt_poc_white_list_info[i]);
    }

    app_bolt_poc_scan_list_init();

#ifdef AIR_BOLT_POC_MULTI_VENDOR_SUPPORT
    app_bolt_poc_init_device_info();
#endif
}

static void app_bolt_poc_set_white_list_process()
{
    if (g_power_on_bonded_idx < g_power_on_bonded_cnt) {
        const bt_addr_t *bt_addr = &g_bolt_poc_bonding_list_info[g_power_on_bonded_idx].addr_;
        app_bolt_poc_add_white_list(bt_addr);
        g_power_on_bonded_idx++;
    } else {
        app_bolt_poc_start_reconn();
    }
}

void app_bolt_poc_set_new_wl_address(const bt_addr_t *addr)
{
    app_bolt_poc_cpy_addr(&g_new_wl_item.addr_, addr);
}

void app_bolt_poc_set_white_list_cnf(bt_status_t status)
{
    #define MEMORY_CAPACITY_EXCEEDED (0x07)
    if (status == MEMORY_CAPACITY_EXCEEDED) {
        app_bolt_poc_white_list_full_process();

        // add new dev to wl again until pass
        if (app_bolt_poc_add_wl_address_is_valid()) {
            const bt_addr_t *bt_addr = app_bolt_poc_get_add_wl_item_addr();
            app_bolt_poc_add_white_list(bt_addr);
        }
        return;
    }

    // add new dev to local wl list
    if (app_bolt_poc_add_wl_address_is_valid()) {
        const bt_addr_t *bt_addr = app_bolt_poc_get_add_wl_item_addr();
        const bt_handle_t handle = app_bolt_poc_get_add_wl_item_handle();
        bool ret = app_bolt_poc_add_to_local_white_list(bt_addr, handle);
        if (ret) {
            app_bolt_poc_clear_new_wl_address();
        } else {
            APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_set_white_list_cnf add local wl fail(full)", 0);
        }
    }

    app_bolt_poc_set_white_list_process();
}

void app_bolt_poc_power_on_cnf()
{
    app_bolt_poc_data_init();

    g_power_on_bonded_cnt = app_bolt_poc_get_bond_info();
    if (g_power_on_bonded_cnt == 0) {
        return;
    }

    app_bolt_poc_set_white_list_process();
}

void app_bolt_poc_adv_report_ind(void *buffer)
{
    const bt_gap_le_ext_advertising_report_ind_t *report = (const bt_gap_le_ext_advertising_report_ind_t *)buffer;
    if (report == NULL) {
        return;
    }

    #define MAX_AD_DATA_LEN (32)
    
    uint8_t count = 0;
    while (count < report->data_length) {
        const uint8_t data_len = report->data[count];
        count++;

        if (data_len > MAX_AD_DATA_LEN - 2 || data_len < 1) {
            break;
        }

        const uint8_t data_type = report->data[count];
        count++;

        const uint8_t buf_len = data_len - 1;
        if (data_type == BT_GAP_LE_AD_TYPE_APPEARANCE) {

            uint8_t buf[MAX_AD_DATA_LEN] = {0};
            memcpy(buf, &report->data[count], buf_len);
            count += buf_len;

            if ((buf[0] == 0xC1 && buf[1] == 0x03) ||
                (buf[0] == 0xC2 && buf[1] == 0x03)) {
                app_bolt_poc_save_adv_address(&report->address);
            }
            break;
        } else {
            count += buf_len;
        }
    }
}

void app_bolt_poc_scan_list_init()
{
    uint8_t i = 0;
    for (i = 0; i < APP_SCAN_MAX_DEV_NUM; i++) {
        app_bolt_poc_init_addr(&g_bolt_poc_scan_dev_info[i].addr_);
    }
}

void app_bolt_poc_pop_handle_process(bt_handle_t handle)
{
    const bt_handle_t pop_handle = app_bolt_poc_get_pop_handle(handle);
    if (pop_handle != BT_HANDLE_INVALID) {
        APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_pop_handle_process pop_handle(%d)", 1, pop_handle);
        app_bolt_poc_disconn_dev(pop_handle);
    }
}

void app_bolt_poc_event_input_report_ind(const bt_hogp_client_para_t *para, const uint8_t *buffer, uint16_t length)
{
#ifdef AIR_BOLT_POC_MULTI_VENDOR_SUPPORT
    int8_t idx = 0;
    const uint8_t dev_id = app_bolt_poc_find_dev_id(para->conn_handle, para->report_id, &idx);
    if (dev_id == GD_Mouse) {
        LogiBolt_Mouse_t mouse;
        memset(&mouse, 0, sizeof(LogiBolt_Mouse_t));
        if (app_bolt_poc_get_mouse_info(&mouse, para->conn_handle, buffer, length, idx)) {
            usb_hid_send_LB_mouse_report(&mouse);
        }
    } else if (dev_id == GD_Keyboard) {
        LogiBolt_Keyboard_t keyboard;
        memset(&keyboard, 0, sizeof(LogiBolt_Keyboard_t));
        if (app_bolt_poc_get_keyboard_info(&keyboard, para->conn_handle, buffer, length, idx)) {
            usb_hid_send_LB_keyboard_report(&keyboard);
        }
    }
#else
    //APPS_LOG_DUMP_I(" [BOLT_POC] data input: ", buffer, length);
    if (para->report_id == BT_HOGP_MOUSE_INPUT_REPORT) {
        LogiBolt_Mouse_t mouse;
        memset(&mouse, 0, sizeof(LogiBolt_Mouse_t));
        memcpy(&mouse.bitmap_Btn, buffer, (length > HOGP_REPORT_DATA_LENGTH) ? HOGP_REPORT_DATA_LENGTH : length);
        usb_hid_send_LB_mouse_report(&mouse);
    } else if (para->report_id == BT_HOGP_KEYBOARD_INPUT_REPORT) {
        LogiBolt_Keyboard_t keyboard;
        memset(&keyboard, 0, sizeof(LogiBolt_Keyboard_t));
        memcpy(keyboard.keymap, buffer, (length > HOGP_REPORT_DATA_LENGTH) ? HOGP_REPORT_DATA_LENGTH : length);
        usb_hid_send_LB_keyboard_report(&keyboard);
    }
#endif

    app_bolt_poc_pop_handle_process(para->conn_handle);
}

void app_bolt_poc_bonding_complete_ind(bt_handle_t handle)
{
    uint8_t i = 0;
    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        app_white_list_item_info_t *item = &g_bolt_poc_white_list_info[i];
        if (app_bolt_poc_addr_not_empty(&item->addr_)) {
            if (handle == item->handle_) {
                if (false == item->bonding_status_) {
                    item->bonding_status_ = true;
                    APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_bonding_complete_ind bt_hogp_client_set_rediscovery_flag", 0);
                    bt_hogp_client_set_rediscovery_flag(handle, true);
                }
                return;
            }
        }
    }
}

void app_bolt_poc_connect_ind(bt_status_t status, const bt_addr_t *addr, bt_handle_t handle)
{
    if (addr == NULL || handle == BT_HANDLE_INVALID) {
        return;
    }

    if (status == BT_HCI_STATUS_UNKNOWN_CONNECTION_IDENTIFIER) { // Cancel pass, need reconnect 
        APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_connect_ind need reconnect by WL", 0);
        app_bolt_poc_conn_dev(NULL);
        return;
    }

    const int8_t idx = app_bolt_poc_find_idx_in_wl_list(addr);
    if (idx >= 0) {
        app_bolt_poc_update_local_white_list(idx, handle);
        if (!app_bolt_poc_check_white_list_dev_conn_full()) {
            app_bolt_poc_start_reconn();
        }
    } else {
        app_bolt_poc_set_new_wl_handle(handle);
        app_bolt_poc_add_white_list(addr);
    }
}

void app_bolt_poc_disconn_ind(bt_handle_t handle)
{
    const bt_addr_t *bt_addr = app_bolt_poc_find_address_by_handle(handle);
    if (bt_addr != NULL) {

#ifdef AIR_BOLT_POC_MULTI_VENDOR_SUPPORT
        if (app_poc_bolt_find_keyboard_dev_id(bt_addr)) {
            APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_disconn_ind keyboard", 0);
            app_bolt_poc_release_keyboard();
        }
#else
        app_bolt_poc_release_keyboard();
#endif
        app_bolt_poc_conn_list_remove_by_addr(bt_addr);

        if (!app_bolt_poc_clear_handle(handle)) {
            APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_clear_handle fail", 0);
        }
    }
    app_bolt_poc_start_reconn();
}

void app_bolt_poc_start_new_conn(const bt_addr_t *addr)
{
    if (addr == NULL) {
        return;
    }
    app_bolt_poc_cpy_addr(&g_new_conn_addr, addr);
    bt_status_t status = bt_gap_le_srv_cancel_connection();
    if (status != BT_STATUS_SUCCESS) {
        app_bolt_poc_connect_cancel_cnf();
    }
}

void app_bolt_poc_connect_cancel_cnf()
{
    if (app_bolt_poc_new_dev()) {
        app_bolt_poc_connect_new_dev();
        app_bolt_poc_clear_new_conn_address();
    } else {
        if (!app_bolt_poc_check_white_list_dev_conn_full()) {
            app_bolt_poc_conn_dev(NULL);
            APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_connect_cancel_cnf: start conn white list", 0);
        }
    }
}

const bt_addr_t *app_bolt_poc_get_scan_addr(int index)
{
    const bt_addr_t *bt_addr = &g_bolt_poc_scan_dev_info[index].addr_;
    if (app_bolt_poc_addr_not_empty(bt_addr)) {
        return bt_addr;
    }
    return NULL;
}

const bt_addr_t *app_bolt_poc_get_white_list_addr(int index)
{
    const bt_addr_t *bt_addr = &g_bolt_poc_white_list_info[index].addr_;
    if (app_bolt_poc_addr_not_empty(bt_addr)) {
        return bt_addr;
    }
    return NULL;
}

const bool app_bolt_poc_get_dev_conn_status(int index)
{
    return g_bolt_poc_white_list_info[index].handle_ != BT_HANDLE_INVALID;
}

