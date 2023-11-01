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
 * File: apps_aws_sync_event.h
 *
 * Description: This file defines the interface of apps_aws_sync_event.c.
 *
 */

#ifndef __APPS_AWS_SYNC_EVENT_H__
#define __APPS_AWS_SYNC_EVENT_H__

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_sink_srv.h"
#include "ui_shell_activity.h"

#include "bt_aws_mce_report.h"

/**
* @brief      This function is used to send UI Shell event with extra_data between Agent and Partner.
* @param[in]  event_group, the current event group to be sent.
* @param[in]  event_id, the current event ID to be sent.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  extra_data_len, the length of the extra data. 0 means extra_data is NULL.
* @return
* #BT_STATUS_SUCCESS, if the operation is successful.
* #BT_STATUS_FAIL, if the event is not sent successfully.
*/
bt_status_t apps_aws_sync_event_send_extra(uint32_t event_group,
                                           uint32_t event_id, const void *extra_data,
                                           uint32_t extra_data_len);

/**
* @brief      This function is used to send UI Shell event without extra_data between Agent and Partner.
* @param[in]  event_group, the current event group to be sent.
* @param[in]  event_id, the current event ID to be sent.
* @return
* #BT_STATUS_SUCCESS, if the operation is successful.
* #BT_STATUS_FAIL, if the event is not sent successfully.
*/
bt_status_t apps_aws_sync_event_send(uint32_t event_group, uint32_t event_id);

/**
* @brief      This function is used to send UI Shell event for broadcast mode.
* @param[in]  is_urgent, urgent channel.
* @param[in]  event_group, the current event group to be sent.
* @param[in]  event_id, the current event ID to be sent.
* @return
* #BT_STATUS_SUCCESS, if the operation is successful.
* #BT_STATUS_FAIL, if the event is not sent successfully.
*/
bt_status_t apps_aws_sync_event_send_for_broadcast(bool is_urgent, uint32_t event_group, uint32_t event_id,
                                                   void *extra_data, uint32_t extra_data_len);

/**
* @brief      This function is used to decode UI Shell event with extra_data from received AWS MCE report info.
* @param[in]  aws_data_ind, received AWS MCE report info.
* @param[out] event_group, received event group.
* @param[out] event_id, received event ID.
* @param[out] extra_data, extra data pointer of received event, NULL means there is no extra data.
* @param[out] extra_data_len, the length of the extra data. 0 means extra_data is NULL.
*/
void apps_aws_sync_event_decode_extra(const bt_aws_mce_report_info_t *aws_data_ind,
                                      uint32_t *event_group, uint32_t *event_id,
                                      void **p_extra_data, uint32_t *p_extra_data_len);

/**
* @brief      This function is used to decode UI Shell event without extra_data from received AWS MCE report info.
* @param[in]  aws_data_ind, received AWS MCE report info.
* @param[out] event_group, received event group.
* @param[out] event_id, received event ID.
*/
void apps_aws_sync_event_decode(const bt_aws_mce_report_info_t *aws_data_ind,
                                uint32_t *event_group, uint32_t *event_id);

#define APPS_AWS_SYNC_FUTURE_SYNC_EVENT_MAGIC_NUM           (0xF8F8F8F8)

/**
 * @brief When local UI shell event received, the extra data should be the following structure.
 */
typedef struct {
    uint8_t                 from_while_role;
    uint32_t                extra_data_len;
    uint8_t                 extra_data[0];
} apps_aws_sync_future_event_local_event_t;

/**
 * @brief       This function is used to send future event with some time delay and execute the event in both agent and partner side at a future time.
 *              Also support do not execute locally.
 * @param[in] from_isr, indicate the function call is from isr or not.
 * @param[in] event_group, Event group.
 * @param[in] event_id, event ID.
 * @param[in] need_execute_locally, need execute the event locally or not, if execute locally, will check the #delay_ms parameter to send the event in future or directly.
 * @param[in] extra_data, The extra data for the event.
 * @param[in] extra_data_len, The extra data length for the event.
 * @param[in] delay_ms, How to to delay the event to execute in future.
 */
bt_status_t apps_aws_sync_send_future_sync_event(bool from_isr,
                                                    uint32_t event_group,
                                                    uint32_t event_id,
                                                    bool need_execute_locally,
                                                    uint8_t *extra_data,
                                                    uint32_t extra_data_len,
                                                    uint32_t delay_ms);

void apps_aws_sync_handle_future_sync_event(uint8_t *data, uint32_t data_len);

#endif /* __APPS_AWS_SYNC_EVENT_H__ */
#endif
