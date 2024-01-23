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
  * File: app_bolt_poc_data.h
  *
  * Description: This file defines the interface of app_bolt_poc_data.c.
  *
  * Note: todo.
  *
  */

#ifndef __APP_BOLT_POC_DATA_H__
#define __APP_BOLT_POC_DATA_H__

#include "bt_hogp_client.h"
#include "bt_type.h"

#define APP_WHITE_LIST_NUM (8)
#define HOGP_REPORT_DATA_LENGTH (7)

void app_bolt_poc_connect_ind(bt_status_t status, const bt_addr_t *addr,  bt_handle_t handle);
void app_bolt_poc_disconn_ind(bt_handle_t handle);
void app_bolt_poc_connect_cancel_cnf();
void app_bolt_poc_bonding_complete_ind(bt_handle_t handle);
void app_bolt_poc_adv_report_ind(void *buffer);
void app_bolt_poc_power_on_cnf();
void app_bolt_poc_set_white_list_cnf(bt_status_t status);
void app_bolt_poc_event_input_report_ind(const bt_hogp_client_para_t *para, const uint8_t *buffer, uint16_t length);
void app_bolt_poc_pop_handle_process(bt_handle_t handle);

void app_bolt_poc_scan_list_init();
void app_bolt_poc_start_new_conn(const bt_addr_t *addr);
void app_bolt_poc_set_new_wl_address(const bt_addr_t *addr);
const bt_addr_t *app_bolt_poc_find_address_by_handle(bt_handle_t handle);
const bt_addr_t *app_bolt_poc_get_scan_addr(int index);
const bt_addr_t *app_bolt_poc_get_white_list_addr(int index);
const bool app_bolt_poc_get_dev_conn_status(int index);

#endif

