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

/**
 * File: app_fast_pair.h
 *
 * Description: This file defines the interface of app_fast_pair_idle_activity.c.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf and https://developers.google.com/nearby/fast-pair/spec for more detail.
 *
 */

#ifndef __APP_FAST_PAIR_H__
#define __APP_FAST_PAIR_H__

#include "bt_platform.h"
#include "ui_shell_activity.h"
#if defined(AIR_BT_FAST_PAIR_SASS_ENABLE) && defined(AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE)
#include "bt_sink_srv.h"
#endif

/**
* @brief      This function is the interface of the app_fm_activity, and is only called by ui_shell when events are sent.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_group, the current event group to be handled.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_fast_pair_idle_activity_proc(ui_shell_activity_t *self,
                                      uint32_t event_group,
                                      uint32_t event_id,
                                      void *extra_data,
                                      size_t data_len);

/**
* @brief      This function set the tx power in fast pair adv data.
* @param[in]  tx_power, measure a tx power value when the distance between smart phone and device is 1 meter.
*/
void app_fast_pair_set_tx_power_level(int8_t tx_power);

/**
* @brief      This function is used to enable/disable GFP advertising.
*/
void app_fast_pair_enable_advertising(bool enable);

bool app_fast_pair_get_is_waiting_connect(void);

uint32_t app_fast_pair_get_model_id(void);

#ifdef AIR_SPOT_ENABLE
bool app_fast_pair_spot_in_adv(void);
#endif
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
void app_fast_pair_connection_state_change(void);

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
bt_sink_srv_allow_result_t app_fast_pair_multi_link_get_allow_play(bt_sink_srv_device_state_t *current_device, bt_sink_srv_device_state_t *coming_device);
#endif
#endif

#endif /* __APP_FAST_PAIR_H__ */
