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

#ifndef __APP_BT_CONN_MANAGER_H__
#define __APP_BT_CONN_MANAGER_H__

/**
 * File: app_bt_conn_manager.h
 *
 * Description: This file defines the interface of app_bt_conn_manager.c.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include "bt_type.h"
#include "ui_shell_activity.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_BT_CONN_MAX_CONN_NUM                        2

typedef enum {
    APP_CONN_MGR_EVENT_SYNC_APP_CONN_EVENT = 0,
    APP_CONN_MGR_EVENT_RECONNECT_TIME_OUT,
    APP_CONN_MGR_EVENT_PARTNER_RECONNECT_LEA
} app_conn_manager_event_t;

bool app_bt_conn_mgr_is_dongle(uint8_t *addr);

bool app_bt_conn_mgr_is_support_emp(void);

bool app_bt_conn_manager_allow_le_adv(void);

bool app_bt_conn_manager_check_exist_link(bool check_edr, uint8_t *addr);

void app_bt_conn_mgr_reconnect_edr(void);
bt_status_t app_bt_conn_mgr_disconnect_edr(uint8_t *addr);
void app_bt_conn_mgr_enable_edr(bool enable);
void app_bt_conn_mgr_enable_edr_profile(bool enable);

#ifdef APP_CONN_MGR_RECONNECT_CONTROL
void app_bt_conn_mgr_active_reconnect_edr(void);
bool app_bt_conn_mgr_is_reconnecting(void);
void app_bt_conn_mgr_lea_restart_reconnect_adv(void);
#endif

/**
* @brief      This function is the interface of the app_bt_conn_manager_activity, and is only called by ui_shell framework when events are sent.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_group, the current event group to be handled.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_bt_conn_manager_activity_proc(struct _ui_shell_activity *self,
                                       uint32_t event_group,
                                       uint32_t event_id,
                                       void *extra_data,
                                       size_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* __APP_BT_CONN_MANAGER_H__ */
