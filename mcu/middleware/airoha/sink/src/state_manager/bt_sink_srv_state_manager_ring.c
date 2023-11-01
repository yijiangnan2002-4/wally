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

static void bt_sink_srv_state_manager_notify_ring_ind_internal(
        bt_sink_srv_state_manager_context_t *context,
        bt_bd_addr_t *address,
        bool is_play,
        bool is_twc);

void bt_sink_srv_state_manager_notify_ring_ind(
        bt_sink_srv_state_manager_device_type_t type,
        bt_bd_addr_t *address,
        bool play)
{
    bool support_inband = false;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();
    bt_sink_srv_state_manager_device_t *device = bt_sink_srv_state_manager_get_device(context, type, address);

    bt_sink_srv_report_id(
            "[Sink][StaMgr]notify ring ind, call_focus_device:0x%x device:0x%x",
            2,
            context->focus_call_device,
            device);

    if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE == type) {
        if (NULL == device) {
            bt_sink_srv_state_manager_device_t new_device = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE;

            new_device.type = type;
            new_device.flag |= BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_SEND_RING;
            bt_sink_srv_memcpy(&new_device.address, address, sizeof(bt_bd_addr_t));

            device = bt_sink_srv_state_manager_add_device(context, &new_device);
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
            bt_sink_srv_state_manager_alloc_psedev(context, device);
#endif
            return;
        } else {
            if (BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE(device->call_state)) {
                device->flag |= BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_SEND_RING;
                return;
            }
        }
    } else {
        if (NULL == device) {
            bt_sink_srv_state_manager_device_t new_device = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE;

            new_device.type = type;
            new_device.flag |= BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_SEND_RING;
            bt_sink_srv_memcpy(&new_device.address, address, sizeof(bt_bd_addr_t));

            device = bt_sink_srv_state_manager_add_device(context, &new_device);
        } else {
            if (!BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(context->previous_state)) {
                device->flag |= BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_SEND_RING;
                return;
            }
        }
    }

    if (device == context->focus_call_device) {
        /* 1.1. Check in-band support. */
        if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == device->type) {
            bt_sink_srv_state_manager_call_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT,
                    address,
                    (void *)&support_inband);
        } else {
            bt_sink_srv_state_manager_le_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT,
                    address,
                    (void *)&support_inband);
        }

        /* 1.2. Notify RING IND. */
        if (play && !support_inband) {
            device->flag |= BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_RING;
            bt_sink_srv_state_manager_notify_ring_ind_internal(context, address, play, false);
        }
    }
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    else if (0 == bt_sink_srv_memcmp(device->address, context->focus_call_device->address, sizeof(bt_bd_addr_t))) {
        bt_sink_srv_report_id("[Sink][StaMgr]notify ring ind, skip same address", 0);
        return;
    }
#endif
    else {
        /* 2.1. Notify TWC RING IND. */
        if (play) {
            device->flag |= BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_TWC_RING;
        }

        bt_sink_srv_state_manager_notify_ring_ind_internal(context, address, play, true);
    }
}

static void bt_sink_srv_state_manager_notify_ring_ind_internal(
        bt_sink_srv_state_manager_context_t *context,
        bt_bd_addr_t *address,
        bool is_play,
        bool is_twc)
{
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_link_type_t link = bt_aws_mce_srv_get_link_type();
    bt_sink_srv_report_id("[Sink][StaMgr]notify ring ind, role: 0x%x link_type: 0x%x", 2, role, link);

    if (BT_AWS_MCE_ROLE_PARTNER == role && BT_AWS_MCE_SRV_LINK_NONE != link) {
        return;
    }
#endif

    if (!is_twc) {
        bt_sink_srv_event_hf_ring_ind_t ring_ind = {{0}};
        bt_sink_srv_memcpy(&ring_ind.address, address, sizeof(bt_bd_addr_t));
        bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_HF_RING_IND, &ring_ind, sizeof(ring_ind));
    } else {
        bt_sink_srv_event_hf_twc_ring_ind_t twc_ring_ind = {0};
        twc_ring_ind.play_vp = is_play;
        bt_sink_srv_memcpy(&twc_ring_ind.address, address, sizeof(bt_bd_addr_t));
        bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_HF_TWC_RING_IND, &twc_ring_ind, sizeof(twc_ring_ind));
    }
}

