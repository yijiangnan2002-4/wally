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
        context->focus_device,
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

    if (device == context->focus_device) {
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
    else if (0 == bt_sink_srv_memcmp(device->address, context->focus_device->address, sizeof(bt_bd_addr_t))) {
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
        if (is_play) {
            bt_sink_srv_event_hf_ring_ind_t ring_ind = {{0}};
            bt_sink_srv_memcpy(&ring_ind.address, address, sizeof(bt_bd_addr_t));
            bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_HF_RING_IND, &ring_ind, sizeof(ring_ind));
        }
    } else {
        bt_sink_srv_event_hf_twc_ring_ind_t twc_ring_ind = {0};
        twc_ring_ind.play_vp = is_play;
        bt_sink_srv_memcpy(&twc_ring_ind.address, address, sizeof(bt_bd_addr_t));
        bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_HF_TWC_RING_IND, &twc_ring_ind, sizeof(twc_ring_ind));
    }
}

void bt_sink_srv_state_manager_update_ring_ind(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    bt_sink_srv_state_t previous_state)
{


    /* 1. INCOMING + INCOMING. */
    /* 1.1. The 1st incoming is answered, keep TWC RING. */
    /* 1.2. The 1st incoming is rejected, stop TWC RING and resume 2nd RING. */
    /* 1.3. The 2nd incoming is answered, stop TWC RING and resume 1st RING. */
    /* 1.4. The 2nd incoming is rejected, stop TWC RING and resume 1st RING. */

    /* 2. INCOMING + ONCALL. */
    /* 2.1. The 1st incoming is answered, stop RING. */
    /* 2.2. The 1st incoming is rejected, stop RING. */
    /* 2.3. The 2nd oncall is ended, keep RING. */

    /* 3. ONCALL + INCOMING. */
    /* 3.1. The 1st oncall is ended, stop TWC RING and resume RING. */
    /* 3.2. The 2nd incoming is answered, stop TWC RING. */
    /* 3.3. The 2nd incoming is rejected, stop TWC RING. */

    bt_sink_srv_state_manager_device_t *other_device = NULL;
    bool support_inband = false;
    bt_sink_srv_report_id("[Sink][StaMgr]update ring ind", 0);

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
    if (bt_sink_srv_state_manager_is_rho_update(context)) {
        return;
    }
#endif

    if (BT_SINK_SRV_STATE_INCOMING == previous_state &&
        BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(device->call_state)) {
        /* 1.1. The 1st incoming is answered, keep TWC RING.         no action*/
        /* 2.1. The 1st incoming is answered, APP auto stop RING ?.    no action? */


        other_device = bt_sink_srv_state_manager_get_device_by_call_state(context, BT_SINK_SRV_STATE_INCOMING, device);
        if (NULL != other_device && other_device == context->focus_device) {
            /* 1.3. The 2nd incoming is answered, stop TWC RING and resume 1st RING. */
            bt_sink_srv_state_manager_notify_ring_ind_internal(context, &device->address, false, true);   /*stop TWC RING*/
            /* Check 1st incoming in-band support. */
            if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == other_device->type) {
                bt_sink_srv_state_manager_call_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT,
                    &other_device->address,
                    (void *)&support_inband);

            } else {
                bt_sink_srv_state_manager_le_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT,
                    &other_device->address,
                    (void *)&support_inband);
            }

            if (!support_inband) {
                bt_sink_srv_state_manager_notify_ring_ind_internal(context, &other_device->address, true, false);   /*resume RING*/
            }
        } else if (!other_device) {
            other_device = bt_sink_srv_state_manager_get_device_by_call_state(context, BT_SINK_SRV_STATE_ACTIVE, device);
            /* 3.2. The 2nd incoming is answered, stop TWC RING. */
            if (other_device == context->focus_device) {
                bt_sink_srv_state_manager_notify_ring_ind_internal(context, &device->address, false, true);   /*stop TWC RING*/
            } else if (!other_device) {
                /* 2.1. The 1st incoming is answered, stop RING. */
                /* 2nd active call auto hold, there is no other  active call */
                bt_sink_srv_state_manager_notify_ring_ind_internal(context, &device->address, false, false);   /*stop TWC RING*/
            }
        }
    } else if (BT_SINK_SRV_STATE_INCOMING == previous_state &&
               BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE(device->call_state)) {

        other_device = bt_sink_srv_state_manager_get_device_by_call_state(context, BT_SINK_SRV_STATE_INCOMING, device);

        if (other_device != NULL &&
            other_device != context->focus_device) {
            /* 1.2. The 1st incoming is rejected, stop TWC RING and resume 2nd RING. */
            /*found 2nd incoming device*/

            bt_sink_srv_state_manager_notify_ring_ind_internal(context, &other_device->address, false, true);   /*stop TWC RING*/
            /* Check 2nd incoming in-band support. */
            if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == other_device->type) {
                bt_sink_srv_state_manager_call_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT,
                    &other_device->address,
                    (void *)&support_inband);

            } else {
                bt_sink_srv_state_manager_le_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT,
                    &other_device->address,
                    (void *)&support_inband);
            }

            if (!support_inband) {
                bt_sink_srv_state_manager_notify_ring_ind_internal(context, &other_device->address, true, false);   /*resume RING*/
            }
        } else if (NULL != other_device && other_device == context->focus_device) {

            /* 1.4. The 2nd incoming is rejected, stop TWC RING and resume 1st RING. */
            bt_sink_srv_state_manager_notify_ring_ind_internal(context, &other_device->address, false, true);   /*stop TWC RING*/
            /* Check 1nd incoming in-band support. */
            if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == other_device->type) {
                bt_sink_srv_state_manager_call_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT,
                    &other_device->address,
                    (void *)&support_inband);

            } else {
                bt_sink_srv_state_manager_le_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT,
                    &other_device->address,
                    (void *)&support_inband);
            }

            if (!support_inband) {
                bt_sink_srv_state_manager_notify_ring_ind_internal(context, &other_device->address, true, false);   /*resume RING*/
            }

        } else if (!other_device) {
            other_device = bt_sink_srv_state_manager_get_device_by_call_state(context, BT_SINK_SRV_STATE_ACTIVE, device);
            /* 3.3. The 2nd incoming is rejected, stop TWC RING. */
            if (other_device == context->focus_device) {
                bt_sink_srv_state_manager_notify_ring_ind_internal(context, &device->address, false, true);/*stop TWC RING*/
            } else if (!other_device) {
                /* 2.2. The 1st incoming is rejected, stop RING. */
                bt_sink_srv_state_manager_notify_ring_ind_internal(context, &device->address, false, false);/*stop RING*/
            }
        }
    } else if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(previous_state) &&
               BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE(device->call_state)) {
        /* 2.3. The 2nd oncall is ended, keep RING. no need action*/
        /* 3.1. The 1st oncall is ended, stop TWC RING and resume RING. */
        other_device = bt_sink_srv_state_manager_get_device_by_call_state(context, BT_SINK_SRV_STATE_INCOMING, device);
        if (other_device != context->focus_device) {
            bt_sink_srv_state_manager_notify_ring_ind_internal(context, &device->address, false, true);   /*stop TWC RING*/
            if (other_device) {
                other_device->flag |= BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_SEND_RING;
            }

        }
    }
}
