/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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
#ifndef __BT_ULL_LE_HID_CONN_SERVICE_H__
#define __BT_ULL_LE_HID_CONN_SERVICE_H__
#include "bt_ull_le_utility.h"
#include "bt_ull_le_hid_utility.h"

#define BT_ULL_LE_HID_CONN_SRV_MSG_CIG_CREATED_IND             0x10
#define BT_ULL_LE_HID_CONN_SRV_MSG_CIG_REMOVED_IND             0x11
#define BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CONNECTED_IND           0x12
#define BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CANCEL_CREATE_IND       0x13
#define BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CANCEL_SYNC_IND         0x14
#define BT_ULL_LE_HID_CONN_SRV_MSG_CIS_DISCONNECTED_IND        0x15
#define BT_ULL_LE_HID_CONN_SRV_MSG_CIS_ACTIVE_STREAMING_IND    0x16
#define BT_ULL_LE_HID_CONN_SRV_MSG_CIS_INACTIVE_STREAMING_IND  0x17
#define BT_ULL_LE_HID_CONN_SRV_MSG_SCENARIO_CHANGED_IND        0x18
#define BT_ULL_LE_HID_CONN_SRV_MSG_MAX                         0xFF
typedef uint8_t bt_ull_le_hid_conn_srv_msg_t;

typedef void (*bt_ull_le_hid_conn_srv_callback_t)(bt_ull_le_hid_conn_srv_msg_t msg, void *data);

typedef struct {
    bt_handle_t acl_handle;
    uint8_t device_type;
    uint8_t rr_level;
    bt_addr_t peer_addr;
} bt_ull_le_hid_conn_srv_msg_cis_connected_t;

typedef struct {
    bt_handle_t acl_handle;
    uint8_t     device_type;
    uint8_t     reason;
} bt_ull_le_hid_conn_srv_msg_cis_disconnected_t;

typedef struct {
    bt_handle_t acl_handle;
    uint8_t     device_type;
} bt_ull_le_hid_conn_srv_cis_active_streaming_t,
bt_ull_le_hid_conn_srv_cis_deactive_streaming_t;

typedef struct {
    uint8_t device_type;
} bt_ull_le_hid_conn_srv_msg_cancel_create_t,
bt_ull_le_hid_conn_srv_msg_cancel_sync_t;

typedef struct {
    bt_ull_le_hid_srv_conn_params_t *hs;
    bt_ull_le_hid_srv_conn_params_t *kb;
    bt_ull_le_hid_srv_conn_params_t *ms;
} bt_ull_le_hid_conn_srv_msg_scenario_changed_t;

typedef struct {
    bt_status_t status;                                                     /*BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_PARAMS_REMOVED_IND and BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_REMOVED_IND only have status*/
    union {
        bt_ull_le_hid_conn_srv_msg_cis_connected_t       cis_connected;     /*BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND*/
        bt_ull_le_hid_conn_srv_msg_cis_disconnected_t    cis_disconnected;
        bt_ull_le_hid_conn_srv_msg_cancel_create_t       cancel_create;
        bt_ull_le_hid_conn_srv_msg_cancel_sync_t         cancel_sync;
        bt_ull_le_hid_conn_srv_cis_active_streaming_t    streaming_active;
        bt_ull_le_hid_conn_srv_cis_deactive_streaming_t  streaming_deactive;
        bt_ull_le_hid_conn_srv_msg_scenario_changed_t    scenario_changed;
    };
} bt_ull_le_hid_conn_srv_msg_ind_t;

bt_status_t bt_ull_le_hid_conn_srv_init(bt_ull_role_t role, bt_ull_le_hid_conn_srv_callback_t cb);
void bt_ull_le_hid_conn_srv_deinit(void);
bt_status_t bt_ull_le_hid_conn_srv_change_scenario(bt_ull_le_hid_srv_app_scenario_t scenario);
bt_status_t bt_ull_le_hid_conn_srv_remove_air_cig(void);
bt_status_t bt_ull_le_hid_conn_srv_create_air_cis(bt_ull_le_hid_srv_conn_params_t *hs, \
    bt_ull_le_hid_srv_conn_params_t *kb, bt_ull_le_hid_srv_conn_params_t *ms);
bt_status_t bt_ull_le_hid_conn_srv_cancel_create_air_cis(bt_ull_le_hid_srv_device_t device_type);
bt_status_t bt_ull_le_hid_conn_srv_sync_air_cis(bt_ull_le_hid_srv_device_t type, bt_addr_t *addr);
bt_status_t bt_ull_le_hid_conn_srv_cancel_sync_air_cis(bt_ull_le_hid_srv_device_t device_type);
bt_status_t bt_ull_le_hid_conn_srv_disconnect_air_cis(bt_handle_t acl_handle, uint8_t reason);
bt_status_t bt_ull_le_hid_conn_srv_set_report_rate(uint8_t rr_level);
bt_status_t bt_ull_le_hid_conn_srv_active_streaming(bt_handle_t acl_handle);
bt_status_t bt_ull_le_hid_conn_srv_deactive_streaming(bt_handle_t acl_handle);
bt_status_t bt_ull_le_hid_conn_srv_unmute_air_cis(bt_handle_t acl_handle);
bt_status_t bt_ull_le_hid_conn_srv_set_idle_time(bt_ull_le_hid_srv_idle_time_t idle_time);
void bt_ull_le_hid_conn_srv_set_cis_connection_timeout(uint16_t conn_timeout);

#endif

