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
 * File: app_bt_emp_service.h
 *
 * Description: This file defines the interface of app_bt_emp_service.c.
 *
 */

#ifndef __APP_BT_EMP_SERVICE_H__
#define __APP_BT_EMP_SERVICE_H__

#include "ui_shell_activity.h"
#include "stdint.h"
#include "bt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef AIR_MULTI_POINT_ENABLE

#ifdef AIR_APP_A2DP_LBB_VENDOR_CODEC_LIMIT
#define APP_BT_EMP_SWITCH_INIT_STATE            FALSE
#else
#define APP_BT_EMP_SWITCH_INIT_STATE            TRUE
#endif

typedef enum {
    APP_BT_EMP_SRV_USER_ID_HFP = 0,
    APP_BT_EMP_SRV_USER_ID_XIAOAI,
    APP_BT_EMP_SRV_USER_ID_LEAUDIO,
    APP_BT_EMP_SRV_USER_ID_MAX,
} app_bt_emp_srv_user_id;

typedef bool (*app_bt_emp_switch_allow_cb_t)(bool need_enable, bt_bd_addr_t *keep_phone_addr);

/**
 * @brief      This function is used to register app_bt_emp_switch_allow_cb_t.
 * @param[in]  id, User ID.
 * @param[in]  func, APP callback.
 * @return     TRUE, register successfully.
 */
bool app_bt_emp_srv_user_register(app_bt_emp_srv_user_id user_id, app_bt_emp_switch_allow_cb_t func);

/**
 * @brief      This function is used to deregister app_bt_emp_switch_allow_cb_t.
 * @param[in]  id, User ID.
 * @return     TRUE, deregister successfully.
 */
bool app_bt_emp_srv_user_deregister(app_bt_emp_srv_user_id user_id);

/**
* @brief      This function is the interface of the app_bt_emp_service_activity, and is only called by ui_shell framework when events are sent.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_group, the current event group to be handled.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_bt_emp_service_activity_proc(struct _ui_shell_activity *self,
                                      uint32_t event_group,
                                      uint32_t event_id,
                                      void *extra_data,
                                      size_t data_len);

/**
* @brief      This function is return EMP enable/disable state.
* @return     TRUE - EMP enable.
*/
bool app_bt_emp_is_enable();

/**
* @brief      This function is switch BT EMP feature, not save NVKEY.
* @param[in]  enable, enable/disable BT EMP feature.
* @return     TRUE - success.
*/
bool app_bt_emp_switch_enable(bool enable);

/**
* @brief      This function is switch BT EMP feature.
* @param[in]  enable, enable/disable BT EMP feature.
* @param[in]  disconnect_one_when_off, disconnect one link when EMP disable.
* @return     TRUE - success.
*/
bool app_bt_emp_enable(bool enable, bool disconnect_one_when_off);

#endif /* AIR_MULTI_POINT_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* __APP_BT_EMP_SERVICE_H__ */

