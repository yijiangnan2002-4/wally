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

#ifndef __APP_BT_SOURCE_CONN_MGR_H__
#define __APP_BT_SOURCE_CONN_MGR_H__

/**
 * File: app_bt_source_conn_mgr.h
 *
 * Description: This file defines the interface of app_bt_source_conn_mgr.c.
 *
 */

#include "app_bt_source_event.h"

#include "bt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_bt_source_conn_mgr_init(void);

void app_bt_source_conn_mgr_enable(uint8_t *addr);
void app_bt_source_conn_mgr_disable(void);
void app_bt_source_conn_mgr_reset(void);

bool app_bt_source_conn_mgr_is_connected(void);
uint8_t app_bt_source_conn_mgr_get_conn_num(void);
void app_bt_source_conn_mgr_get_conn_info(uint8_t *conn_num, bt_addr_t *list);

uint8_t* app_bt_source_conn_mgr_get_active_device(void);

void app_bt_source_conn_mgr_set_active_index(uint8_t index);
uint8_t app_bt_source_conn_mgr_get_active_index(void);

void app_bt_source_conn_mgr_connect_addr_bt_atcmd(uint8_t *addr);

void app_bt_source_conn_mgr_remove_record(uint8_t *addr);

bt_status_t app_bt_source_conn_mgr_find_name(uint8_t *addr, char *name);

bt_status_t app_bt_source_conn_mgr_control_scan(bool start, uint32_t time_ms);
bt_status_t app_bt_source_conn_mgr_control_connect(bool conn_or_disconn, uint8_t *addr);

bool app_bt_source_conn_mgr_proc_ui_shell_event(uint32_t event_group,
                                                uint32_t event_id,
                                                void *extra_data,
                                                size_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* __APP_BT_SOURCE_CONN_MGR_H__ */
