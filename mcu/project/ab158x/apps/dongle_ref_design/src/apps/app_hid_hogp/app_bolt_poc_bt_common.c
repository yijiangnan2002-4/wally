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
  * File: app_bolt_poc_bt_common.c
  *
  * Description: This file is the bt common api for bolt poc.
  *
  * Note: todo.
  *
  */

#include "app_bolt_poc_bt_common.h"
#include "app_bolt_poc_data.h"
#include "bt_gap_le_service.h"
#include "FreeRTOS.h"
#include "apps_debug.h"

extern bt_status_t bt_gap_le_srv_set_extended_scan(bt_gap_le_srv_set_extended_scan_parameters_t *param, bt_gap_le_srv_set_extended_scan_enable_t *enable, void *callback);

void app_bolt_poc_conn_dev(const bt_addr_t *addr)
{
    bt_hci_cmd_le_create_connection_t conn_param;

    conn_param.le_scan_interval = 0x10;
    conn_param.le_scan_window = 0x10;
    conn_param.own_address_type = BT_ADDR_PUBLIC;
    conn_param.minimum_ce_length = 0x0002;
    conn_param.maximum_ce_length = 0x0004;
    conn_param.conn_interval_min = 0x0006;
    conn_param.conn_interval_max = 0x0006;
    conn_param.conn_latency = 0x0000;
    conn_param.supervision_timeout = 0x01F4;

    if (addr == NULL) {
        conn_param.initiator_filter_policy = BT_HCI_CONN_FILTER_WHITE_LIST_ONLY;
    } else {
        conn_param.initiator_filter_policy = BT_HCI_CONN_FILTER_ASSIGNED_ADDRESS;
        memcpy(&conn_param.peer_address, addr, sizeof(bt_addr_t));
    }

    bt_status_t ret = bt_gap_le_srv_connect(&conn_param);
    if (BT_STATUS_SUCCESS != ret) {
        APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_conn_dev fail(0x%X)", 1, ret);
    }
}

void app_bolt_poc_disconn_dev(bt_handle_t handle)
{
    bt_hci_cmd_disconnect_t discon;
    discon.reason = BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION;
    discon.connection_handle = handle;

    bt_status_t ret = bt_gap_le_disconnect(&discon);
    if (BT_STATUS_SUCCESS != ret) {
        APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_disconn_dev fail(0x%X)", 1, ret);
    }
}

void app_bolt_poc_add_white_list(const bt_addr_t *bt_addr)
{
    app_bolt_poc_set_new_wl_address(bt_addr);

    bt_status_t ret = bt_gap_le_srv_operate_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, bt_addr, NULL);
    if (BT_STATUS_SUCCESS != ret) {
        APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_add_white_list fail(0x%X)", 1, ret);
    }
}

void app_bolt_poc_remove_white_list(const bt_addr_t *bt_addr)
{
    bt_status_t ret = bt_gap_le_srv_operate_white_list(BT_GAP_LE_REMOVE_FROM_WHITE_LIST, bt_addr, NULL);
    if (BT_STATUS_SUCCESS != ret) {
        APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_remove_white_list fail(0x%X)", 1, ret);
    }
}

void app_bolt_poc_stop_scan()
{
    bt_hci_cmd_le_set_extended_scan_enable_t enable = {
        .enable = BT_HCI_DISABLE,
        .filter_duplicates = BT_HCI_DISABLE,
        .duration = 0,
        .period = 0
    };

    bt_status_t ret = bt_gap_le_srv_set_extended_scan(NULL, &enable, NULL);
    if (BT_STATUS_SUCCESS != ret) {
        APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_stop_scan fail(0x%X)", 1, ret);
    }
}

void app_bolt_poc_start_scan()
{
    app_bolt_poc_stop_scan();

    le_ext_scan_item_t *ext_scan_1M_item;
    if (NULL == (ext_scan_1M_item = (le_ext_scan_item_t *)pvPortMalloc(sizeof(le_ext_scan_item_t)))) {
        return;
    }
    memset(ext_scan_1M_item, 0, sizeof(le_ext_scan_item_t));
    ext_scan_1M_item->scan_type = BT_HCI_SCAN_TYPE_PASSIVE;
    ext_scan_1M_item->scan_interval = 0x90;
    ext_scan_1M_item->scan_window = 0x1B;   // ED: decrease(0x24->0x1B) window to avoid mutilink+(BLE scan) cause too much events(host OOM)

    bt_hci_le_set_ext_scan_parameters_t params = {
        .own_address_type = BT_HCI_SCAN_ADDR_RANDOM,
        .scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS,
        .scanning_phys_mask = BT_HCI_LE_PHY_MASK_1M,
        .params_phy_1M = ext_scan_1M_item,
        .params_phy_coded = NULL,
    };
    
    bt_hci_cmd_le_set_extended_scan_enable_t enable = {
        .enable = BT_HCI_ENABLE,
        .filter_duplicates = BT_HCI_ENABLE,
        .duration = 0,
        .period = 0
    };
 
    bt_status_t ret = bt_gap_le_srv_set_extended_scan(&params, &enable, NULL);
    if (BT_STATUS_SUCCESS != ret) {
        APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_start_scan fail(0x%X)", 1, ret);
    }

    vPortFree(ext_scan_1M_item);
}


