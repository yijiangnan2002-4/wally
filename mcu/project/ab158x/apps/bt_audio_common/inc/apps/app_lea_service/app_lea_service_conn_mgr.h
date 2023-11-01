/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef __APP_LEA_SERVICE_CONN_MGR_H__
#define __APP_LEA_SERVICE_CONN_MGR_H__

/**
 * File: app_lea_service_conn_mgr.h
 *
 * Description: This file defines the interface of app_lea_service_conn_mgr.c.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include "bt_hci.h"
#include "bt_type.h"
#include "app_lea_service_config.h"
#include "app_lea_service_nvkey_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

#define APP_LEA_MAX_CONN_NUM              (APP_LE_AUDIO_MAX_LINK_NUM)

typedef enum {
    APP_LEA_CONN_TYPE_NONE                       = 0,
    APP_LEA_CONN_TYPE_LE_AUDIO,
    APP_LEA_CONN_TYPE_LE_ULL,
} app_lea_conn_type_t;

#define APP_LEA_CONN_CB_TYPE_CONNECTED                  0
#define APP_LEA_CONN_CB_TYPE_DISCONNECTED               1
#define APP_LEA_CONN_CB_TYPE_BONDED                     2
typedef void (*app_lea_conn_mgr_connection_cb_t)(uint8_t conn_cb_type, int index, uint8_t addr_type,
                                                 const uint8_t *addr, bt_hci_disconnect_reason_t reason);
void        app_lea_conn_mgr_register_connection_cb(app_lea_conn_mgr_connection_cb_t cb);

bool        app_lea_conn_mgr_is_ever_connected(void);
bool        app_lea_conn_mgr_is_connected(const uint8_t *addr);

bool        app_lea_conn_mgr_is_le_bond(const uint8_t *addr);
bool        app_lea_conn_mgr_get_ida(const uint8_t *random_addr, uint8_t *type, uint8_t *id_addr);

bt_handle_t app_lea_conn_mgr_get_handle(uint8_t index);
bt_handle_t app_lea_conn_mgr_get_dongle_handle(app_lea_conn_type_t type);
bt_handle_t app_lea_conn_mgr_get_handle_by_addr(const uint8_t *addr);
uint8_t    *app_lea_conn_mgr_get_addr_by_handle(bt_handle_t handle);
uint8_t    *app_lea_conn_mgr_get_unify_addr(const uint8_t *addr);

bool        app_lea_conn_mgr_is_bond_link(const uint8_t *addr);
bool        app_lea_conn_mgr_is_ull_link(const uint8_t *addr);
bool        app_lea_conn_mgr_is_lea_dongle_link(const uint8_t *addr);

uint8_t     app_lea_conn_mgr_get_conn_type(bt_handle_t handle);

uint8_t     app_lea_conn_mgr_get_index(bt_handle_t handle);

uint8_t     app_lea_conn_mgr_get_conn_num(void);
void        app_lea_conn_mgr_get_conn_info(uint8_t *num, bt_addr_t *bdaddr);
// Get support max LEA conn num
uint8_t     app_lea_conn_mgr_get_support_max_conn_num(void);

app_lea_bond_info_t *app_lea_conn_mgr_get_bond_info(void);
uint8_t     app_lea_conn_mgr_get_bond_num(void);
void        app_lea_conn_mgr_reset_bond_info(void);
bool        app_lea_conn_mgr_add_bond_info(uint8_t addr_type, uint8_t *addr, uint8_t conn_type);
bool        app_lea_conn_mgr_remove_bond_info(uint8_t addr_type, uint8_t *addr);
void        app_lea_conn_mgr_reset_keep_ull2_bond_info(void);

bool        app_lea_conn_mgr_is_need_reconnect_adv(void);
void        app_lea_conn_mgr_get_reconnect_info(uint8_t *direct_num, uint8_t *active_num, uint8_t *unactive_num);
void        app_lea_conn_mgr_get_reconnect_addr(uint8_t sub_mode, bt_addr_t *addr_list, uint8_t *list_num);

bool        app_lea_conn_mgr_is_support_addr_resolution(uint8_t *addr);

void        app_lea_conn_mgr_disconnect(const uint8_t *addr, uint8_t reason);
bool        app_lea_conn_mgr_disconnect_by_handle(bt_handle_t handle,
                                                  bt_hci_disconnect_reason_t reason);
void        app_lea_conn_mgr_disconnect_dongle(void);

void        app_lea_conn_mgr_set_reconnect_targeted_addr(bool sync, const uint8_t *addr);

void        app_lea_conn_mgr_enable_multi_conn(bool enable);

void        app_lea_conn_mgr_init(void);

void        app_lea_conn_mgr_proc_ui_shell_event(uint32_t event_group,
                                                 uint32_t event_id,
                                                 void *extra_data,
                                                 size_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* __APP_LEA_SERVICE_CONN_MGR_H__ */
