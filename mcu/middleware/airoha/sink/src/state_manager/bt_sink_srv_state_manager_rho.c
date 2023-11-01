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

#include "bt_sink_srv_state_manager.h"
#include "bt_sink_srv_state_manager_internal.h"

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
uint8_t bt_sink_srv_state_manager_rho_get_len(const bt_bd_addr_t *address)
{
    uint8_t len = 0;
    bt_sink_srv_state_manager_device_t *device = NULL;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    if (NULL != address) {
        device = bt_sink_srv_state_manager_get_device(
                context,
                BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR,
                (bt_bd_addr_t *)address);

        if (NULL != device) {
            if (device == context->focus_call_device || device == context->focus_media_device) {
                len = sizeof(bt_sink_srv_state_manager_rho_data_t);
            }
        }
    }

    bt_sink_srv_report_id("[Sink][StaMgr]RHO get len, %d", 1, len);
    return len;
}

bt_status_t bt_sink_srv_state_manager_rho_get_data(const bt_bd_addr_t *address, void *data)
{
    bt_sink_srv_state_manager_device_t *device = NULL;
    bt_sink_srv_state_manager_rho_data_t *rho_data = (bt_sink_srv_state_manager_rho_data_t *)data;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    if (NULL == rho_data) {
        bt_sink_srv_report_id("[Sink][StaMgr]RHO get data, data is NULL!", 0);
        return BT_STATUS_FAIL;
    }

    if (NULL != address) {
        device = bt_sink_srv_state_manager_get_device(
                context,
                BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR,
                (bt_bd_addr_t *)address);

        if (NULL != device) {
            if (device == context->focus_call_device) {
                rho_data->focus_type |= BT_SINK_SRV_STATE_MANAGER_FOCUS_DEVICE_CALL;
            }

            if (device == context->focus_media_device) {
                rho_data->focus_type |= BT_SINK_SRV_STATE_MANAGER_FOCUS_DEVICE_MEDIA;
            }
        }
    }

    bt_sink_srv_report_id("[Sink][StaMgr]RHO get data, focus_type: 0x%x", 1, rho_data->focus_type);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_sink_srv_state_manager_rho_update(bt_role_handover_update_info_t *info)
{
    bt_sink_srv_state_manager_rho_data_t *rho_data = NULL;
    bt_sink_srv_state_manager_device_t *device = NULL;
    bt_sink_srv_state_manager_device_t new_device = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    if (NULL == info) {
        bt_sink_srv_report_id("[Sink][StaMgr]RHO update, info is NULL!", 0);
        return BT_STATUS_FAIL;
    }

    if (BT_AWS_MCE_ROLE_PARTNER == info->role) {
        rho_data = (bt_sink_srv_state_manager_rho_data_t *)info->data;
        if (NULL != rho_data) {
            bt_sink_srv_report_id("[Sink][StaMgr]RHO update, focus_type: 0x%x", 1, rho_data->focus_type);
            if (rho_data->focus_type & BT_SINK_SRV_STATE_MANAGER_FOCUS_DEVICE_CALL) {
                device = bt_sink_srv_state_manager_get_device(
                        context,
                        BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR,
                        (bt_bd_addr_t *)info->addr);

                if (NULL == device) {
                    new_device.type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR;
                    bt_sink_srv_memcpy(&new_device.address, info->addr, sizeof(bt_bd_addr_t));
                    device = bt_sink_srv_state_manager_add_device(context, &new_device);
                }

                bt_sink_srv_state_manager_set_focus_call_device(context, device, true);
            }

            if (rho_data->focus_type & BT_SINK_SRV_STATE_MANAGER_FOCUS_DEVICE_MEDIA) {
                device = bt_sink_srv_state_manager_get_device(
                        context,
                        BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR,
                        (bt_bd_addr_t *)info->addr);

                if (NULL == device) {
                    new_device.type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR;
                    bt_sink_srv_memcpy(&new_device.address, info->addr, sizeof(bt_bd_addr_t));
                    device = bt_sink_srv_state_manager_add_device(context, &new_device);
                }

                bt_sink_srv_state_manager_set_focus_media_device(context, device, true);
            }
        }
    }

    return BT_STATUS_SUCCESS;
}

void bt_sink_srv_state_manager_rho_status(
        const bt_bd_addr_t *address,
        bt_aws_mce_role_t role,
        bt_role_handover_event_t event,
        bt_status_t status)
{
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();
    bt_sink_srv_report_id("[Sink][StaMgr]RHO status, event: 0x%x, status: 0x%x", 2, event, status);

    switch (event) {
        case BT_ROLE_HANDOVER_COMPLETE_IND: {
            if (BT_STATUS_SUCCESS == status) {
                bt_sink_srv_state_manager_update_edr_devices(context);
            }
            break;
        }

        default: {
            break;
        }
    }
}
#endif

