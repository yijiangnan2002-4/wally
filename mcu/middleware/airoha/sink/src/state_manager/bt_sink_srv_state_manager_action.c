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

#define BT_SINK_SRV_STATE_MANAGER_CALL_ACTION_MASK          \
    (SINK_MODULE_MASK_HFP | SINK_MODULE_MASK_HSP)

#define BT_SINK_SRV_STATE_MANAGER_MEDIA_ACTION_MASK         \
    (SINK_MODULE_MASK_A2DP | SINK_MODULE_MASK_AVRCP)

#define BT_SINK_SRV_STATE_MANAGER_DISPATCH_TABLE_SIZE       \
    (sizeof(g_bt_sink_srv_state_manager_dispatch_table) /   \
     sizeof(bt_sink_srv_state_manager_dispatch_table_t))

#define BT_SINK_SRV_STATE_MANAGER_EDR_ACTION_TABLE_SIZE     \
    (sizeof(bt_sink_srv_action_callback_table) /            \
     sizeof(bt_sink_srv_action_callback_table_t))

#define BT_SINK_SRV_STATE_MANAGER_IS_CALL_ACTION(mask)      \
    ((mask) & BT_SINK_SRV_STATE_MANAGER_CALL_ACTION_MASK)

#define BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_ACTION(mask)     \
    ((mask) & BT_SINK_SRV_STATE_MANAGER_MEDIA_ACTION_MASK)

extern const bt_sink_srv_action_callback_table_t bt_sink_srv_action_callback_table[BT_SINK_SRV_MAX_ACTION_TABLE_SIZE];

static const bt_sink_srv_state_manager_dispatch_table_t g_bt_sink_srv_state_manager_dispatch_table[] = {
#ifdef AIR_MS_TEAMS_UE_ENABLE
    /* 1 TWC_INCOMING*/
    {
        BT_SINK_SRV_STATE_HELD_ACTIVE,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,//hold judgement
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_ANSWER,
        true
    },
    /* 1 TWC_INCOMING*/
    {
        BT_SINK_SRV_STATE_HELD_ACTIVE,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        BT_SINK_SRV_ACTION_REJECT,
        false
    },
    /* 1 TWC_OUTGOING*/
    {
        BT_SINK_SRV_STATE_HELD_ACTIVE,
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_ACTION_HANG_UP,
        0,
        BT_SINK_SRV_ACTION_HANG_UP,
        false
    },

    /*2 TWC_INCOMING*/
    {
        BT_SINK_SRV_STATE_TWC_INCOMING,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        0,
        false
    },
    /*2 TWC_INCOMING*/
    {
        BT_SINK_SRV_STATE_TWC_INCOMING,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        false
    },
    /*2 TWC_OUTGOING*/
    {
        BT_SINK_SRV_STATE_TWC_OUTGOING,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_SINK_SRV_ACTION_HANG_UP,
        0,
        false
    },

    /*3 TWC_INCOMING*/
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_TWC_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        true
    },
    /*3 TWC_INCOMING*/
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_TWC_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        false
    },
    /*3 TWC_OUTGOING*/
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_TWC_OUTGOING,
        BT_SINK_SRV_ACTION_HANG_UP,
        0,
        BT_SINK_SRV_ACTION_HANG_UP,
        false
    },
#endif

    /* INCOMING + OUTGOING. */
    {
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_ANSWER,
        0,
        false,
        false
    },
    {
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_SINK_SRV_ACTION_REJECT,
        0,
        true,
        false
    },
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_ACTION_HANG_UP,
        0,
        BT_SINK_SRV_ACTION_HANG_UP,
        false,
        false
    },
#endif
    /* INCOMING + HELD_REMAINING. */
    {
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_ANSWER,
        0,
        false,
        false
    },
    {
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_SINK_SRV_ACTION_REJECT,
        0,
        true,
        false
    },
    /* OUTGOING + INCOMING. */
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_SINK_SRV_ACTION_ANSWER,
        true,
        false
    },
#else
    {
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_HANG_UP,
        0,
        true,
        false
    },
#endif
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_ANSWER,
        0,
        BT_SINK_SRV_ACTION_ANSWER,
        true,
        false
    },
    {
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        BT_SINK_SRV_ACTION_REJECT,
        false,
        false
    },
    {
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_REJECT,
        0,
        BT_SINK_SRV_ACTION_REJECT,
        false,
        false
    },
#endif
    /* OUTGOING + HELD_REMAINING. */
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        false,
        false
    },
#endif
    /* ACTIVE + INCOMING. */
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_ANSWER,
        true,
        false
    },
#else
#ifdef AIR_MS_TEAMS_UE_ENABLE
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        0,
        true
    },
#else
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_HANG_UP,
        0,
        true,
        false
    },
#endif
#endif
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        BT_SINK_SRV_ACTION_REJECT,
        false,
        false
    },
#else

#ifdef AIR_MS_TEAMS_UE_ENABLE
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        BT_SINK_SRV_ACTION_REJECT,
        false
    },
#else
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        0,
        false,
        false
    },
#endif

#endif
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_ANSWER,
        0,
        BT_SINK_SRV_ACTION_ANSWER,
        true,
        false
    },
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_REJECT,
        0,
        BT_SINK_SRV_ACTION_REJECT,
        false,
        false
    },
#endif
    /* ACTIVE + HELD_REMAINING. */
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        false,
        false
    },
#else
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_SINK_SRV_ACTION_HANG_UP,
        0,
        true,
        false
    },
#endif
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        true,
        false
    },
#else
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        0,
        false,
        false
    },
#endif
    /* HELD_REMAINING + INCOMING. */
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        0,
        BT_SINK_SRV_ACTION_ANSWER,
        true,
        false
    },
#else
    {
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        true,
        false
    },
#endif
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        BT_SINK_SRV_ACTION_REJECT,
        false
    },
#else
    {
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        0,
        false,
        false
    },
#endif
    /* HELD_REMAINING + OUTGOING. */
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    {
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_ACTION_HANG_UP,
        0,
        BT_SINK_SRV_ACTION_HANG_UP,
        false,
        false
    },
#else
    {
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        0,
        true,
        false
    },
#endif
};

static bt_status_t bt_sink_srv_state_manager_action_handler_internal(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter);

static bt_status_t bt_sink_srv_state_manager_call_action_handler(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter);

static bt_status_t bt_sink_srv_state_manager_media_action_handler(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter);

static bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_call_action_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_t other_state,
    bool find_last_device);

static bt_status_t bt_sink_srv_state_manager_le_action_handler(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter);

#if defined(MTK_AWS_MCE_ENABLE)
static void bt_sink_srv_state_manager_sync_focus(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_focus_device_t type,
    bt_sink_srv_state_manager_device_t *device);
#endif

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_MULTI_POINT_ENABLE)
static void bt_sink_srv_state_manager_sync_action(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter);
#endif

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_MULTI_POINT_ENABLE)
static uint32_t bt_sink_srv_state_manager_get_parameter_length(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter);
#endif

bt_status_t bt_sink_srv_state_manager_action_handler(bt_sink_srv_action_t action, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_module_mask_t mask = SINK_MODULE_MASK_OFFSET(action);
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    bt_sink_srv_mutex_lock();

    if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_ACTION(mask)) {
        status = bt_sink_srv_state_manager_call_action_handler(context, action, parameter);
    } else if (BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_ACTION(mask)) {
        status = bt_sink_srv_state_manager_media_action_handler(context, action, parameter);
    } else {
        status = bt_sink_srv_state_manager_action_handler_internal(context, action, parameter);
    }

    bt_sink_srv_mutex_unlock();

    return status;
}

static bt_status_t bt_sink_srv_state_manager_action_handler_internal(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter)
{
    bt_status_t status = bt_sink_srv_state_manager_le_action_handler(context, action, parameter);

    if (BT_STATUS_SUCCESS != status) {
        status = bt_sink_srv_state_manager_edr_action_handler(context, action, parameter);
    }

    return status;
}

static bt_status_t bt_sink_srv_state_manager_call_action_handler(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter)
{
    bool swap_focus = false;
    bt_status_t status = BT_STATUS_FAIL;

    bt_sink_srv_state_manager_device_t *other_device = NULL;
    bt_sink_srv_state_t focus_state = BT_SINK_SRV_STATE_NONE;

    /* 1. If no device is playing, trigger LE device at first. */
    if (NULL == context->focus_device) {
        return bt_sink_srv_state_manager_action_handler_internal(context, action, parameter);
    }

    /* 2. Get focus state. */
    focus_state = context->focus_device->call_state;

    /* 3. Handle action by dispatch table. */
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_DISPATCH_TABLE_SIZE; i++) {
        if (g_bt_sink_srv_state_manager_dispatch_table[i].action == action &&
            g_bt_sink_srv_state_manager_dispatch_table[i].focus_state == focus_state) {
            other_device = bt_sink_srv_state_manager_get_call_action_device(
                               context,
                               g_bt_sink_srv_state_manager_dispatch_table[i].other_state,
                               g_bt_sink_srv_state_manager_dispatch_table[i].find_last_other);
        }

        if (NULL != other_device) {
            bt_sink_srv_report_id("[Sink][StaMgr]call action handler, focus_action:0x%x other_action:0x%x", 2,
                                  g_bt_sink_srv_state_manager_dispatch_table[i].focus_action,
                                  g_bt_sink_srv_state_manager_dispatch_table[i].other_action);

            /* 3.1. Check if need to swap focus device. */
            if (g_bt_sink_srv_state_manager_dispatch_table[i].swap_focus) {
                swap_focus = true;
#if defined(MTK_AWS_MCE_ENABLE)
                if (NULL != other_device &&
                    !(BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == context->focus_device->type &&
                      BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == other_device->type)) {
                    bt_sink_srv_state_manager_sync_focus(
                        context, BT_SINK_SRV_STATE_MANAGER_FOCUS_DEVICE_CALL, other_device);
                }
#endif
            }

            /* 3.2. If focus device & other device type are same, send action to focus device. */
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
            if (NULL == other_device || context->focus_device->type == other_device->type) {
                break;
            }
#endif

            /* 3.3. Send focus action & other action. */
            if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == context->focus_device->type) {
                status = bt_sink_srv_state_manager_edr_action_handler(
                             context,
                             g_bt_sink_srv_state_manager_dispatch_table[i].focus_action,
                             parameter);
            } else {
                status = bt_sink_srv_state_manager_le_action_handler(
                             context,
                             g_bt_sink_srv_state_manager_dispatch_table[i].focus_action,
                             parameter);
            }

            if (NULL != other_device && BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == other_device->type) {
                status = bt_sink_srv_state_manager_edr_action_handler(
                             context,
                             g_bt_sink_srv_state_manager_dispatch_table[i].other_action,
                             parameter);
            } else {
                status = bt_sink_srv_state_manager_le_action_handler(
                             context,
                             g_bt_sink_srv_state_manager_dispatch_table[i].other_action,
                             parameter);
            }

            if (swap_focus) {
                bt_sink_srv_state_manager_set_focus_device(context, other_device, true);
            }

            return status;
        }
    }

    if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == context->focus_device->type) {
#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_MULTI_POINT_ENABLE)
        if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role()) {
            status = BT_STATUS_SUCCESS;
            bt_sink_srv_state_manager_sync_action(context, action, parameter);
        } else {
            status = bt_sink_srv_state_manager_edr_action_handler(context, action, parameter);
        }
#else
        status = bt_sink_srv_state_manager_edr_action_handler(context, action, parameter);
#endif
    } else {
        status = bt_sink_srv_state_manager_le_action_handler(context, action, parameter);
    }

#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
    if (swap_focus) {
        bt_sink_srv_state_manager_set_focus_device(context, other_device, true);
    }
#endif

    return status;
}

static bt_status_t bt_sink_srv_state_manager_media_action_handler(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;

    /* 1. If no device is playing, trigger LE device at first. */
    if (NULL == context->focus_device) {
        return bt_sink_srv_state_manager_action_handler_internal(context, action, parameter);
    }

    /* 2. Trigger the playing device. */
    if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == context->focus_device->type) {
        status = bt_sink_srv_state_manager_edr_action_handler(context, action, parameter);
    } else {
        status = bt_sink_srv_state_manager_le_action_handler(context, action, parameter);
    }

    return status;
}

static bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_call_action_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_t other_state,
    bool find_last_device)
{
    bt_sink_srv_state_manager_device_t *device = NULL;

    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
        if (0 == bt_sink_srv_memcmp(
                context->focus_device->address, context->devices[i].address, sizeof(bt_bd_addr_t))) {
            continue;
        }
#else
        if (&context->devices[i] == context->focus_device) {
            continue;
        }
#endif

        if (0 != (context->devices[i].call_state & other_state)) {
            device = &context->devices[i];

            if (!find_last_device) {
                break;
            }
        }
    }

    return device;
}

bt_status_t bt_sink_srv_state_manager_edr_action_handler(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_module_mask_t mask = SINK_MODULE_MASK_OFFSET(action);
    bt_sink_srv_report_id("[Sink][StaMgr]edr action handler, action: 0x%x", 1, action);

    if (0 == action) {
        return BT_STATUS_SUCCESS;
    }

    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_EDR_ACTION_TABLE_SIZE; i++) {
        if ((bt_sink_srv_action_callback_table[i].module & mask) &&
            (NULL != bt_sink_srv_action_callback_table[i].callback)) {
            status = bt_sink_srv_action_callback_table[i].callback(action, parameter);
        }
    }

    return status;
}

static bt_status_t bt_sink_srv_state_manager_le_action_handler(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_srv_report_id("[Sink][StaMgr]le action handler, action: 0x%x", 1, action);

    if (0 == action) {
        return BT_STATUS_SUCCESS;
    }

#ifdef AIR_LE_AUDIO_ENABLE
    status = le_sink_srv_send_action(action, parameter);
#endif
    return status;
}

#if defined(MTK_AWS_MCE_ENABLE)
static void bt_sink_srv_state_manager_sync_focus(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_focus_device_t type,
    bt_sink_srv_state_manager_device_t *device)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_srv_state_manager_sync_focus_t sync_focus = {{0}};

    bt_aws_mce_report_info_t report_info = {0};
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    /* 1. Check parameters. */
    if (NULL == device) {
        bt_sink_srv_report_id("[Sink][StaMgr]sync focus, NULL device", 0);
        return;
    }

    /* 2. Fill data. */
    sync_focus.header.type = BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_FOCUS;
    sync_focus.header.length = sizeof(bt_sink_srv_state_manager_sync_focus_t);

    if (BT_AWS_MCE_ROLE_AGENT == role) {
        sync_focus.header.direction = BT_SINK_SRV_STATE_MANAGER_SYNC_DIRECTION_PARTNER;
    } else {
        sync_focus.header.direction = BT_SINK_SRV_STATE_MANAGER_SYNC_DIRECTION_AGENT;
    }

    sync_focus.focus_type = type;
    sync_focus.device_type = device->type;
    bt_sink_srv_memcpy(sync_focus.address, device->address, sizeof(bt_bd_addr_t));

    /* 3. Fill report info. */
    report_info.module_id = BT_AWS_MCE_REPORT_MODULE_SINK_STAMGR;
    report_info.param = (void *)&sync_focus;
    report_info.param_len = sizeof(bt_sink_srv_state_manager_sync_focus_t);

    if (BT_AWS_MCE_ROLE_AGENT == role) {
        status = bt_aws_mce_report_send_urgent_event(&report_info);
    } else {
        status = bt_aws_mce_report_send_event(&report_info);
    }

    (void)status;
    bt_sink_srv_report_id("[Sink][StaMgr]sync focus, status: 0x%x", 1, status);
}
#endif

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_MULTI_POINT_ENABLE)
static void bt_sink_srv_state_manager_sync_action(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_aws_mce_report_info_t report_info = {0};

    uint32_t parameter_length
        = bt_sink_srv_state_manager_get_parameter_length(context, action, parameter);

    bt_sink_srv_state_manager_sync_action_t *sync_action
        = bt_sink_srv_memory_alloc(sizeof(bt_sink_srv_state_manager_sync_action_t) + parameter_length);

    /* 1. Fill data. */
    if (NULL == sync_action) {
        return;
    }

    sync_action->header.type = BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_ACTION;
    sync_action->header.length = sizeof(bt_sink_srv_state_manager_sync_action_t);
    sync_action->header.direction = BT_SINK_SRV_STATE_MANAGER_SYNC_DIRECTION_AGENT;

    sync_action->action = action;
    sync_action->parameter_length = parameter_length;
    bt_sink_srv_memcpy(sync_action->parameter, parameter, parameter_length);

    /* 2. Fill report info. */
    report_info.module_id = BT_AWS_MCE_REPORT_MODULE_SINK_STAMGR;
    report_info.param = (void *)sync_action;
    report_info.param_len = sizeof(bt_sink_srv_state_manager_sync_action_t);

    if (BT_AWS_MCE_ROLE_PARTNER == bt_connection_manager_device_local_info_get_aws_role()) {
        status = bt_aws_mce_report_send_event(&report_info);
    }

    (void)status;
    bt_sink_srv_memory_free(sync_action);
    bt_sink_srv_report_id("[Sink][StaMgr]sync action, status: 0x%x", 1, status);
}
#endif

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_MULTI_POINT_ENABLE)
static uint32_t bt_sink_srv_state_manager_get_parameter_length(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter)
{
    (void)context;
    uint32_t parameter_length = 0;

    if (NULL == parameter) {
        return 0;
    }

    switch (action) {
        case BT_SINK_SRV_ACTION_VOICE_RECOGNITION_ACTIVATE:
        case BT_SINK_SRV_ACTION_HF_ECNR_ACTIVATE: {
            parameter_length = sizeof(bool);
            break;
        }

        case BT_SINK_SRV_ACTION_3WAY_RELEASE_SPECIAL:
        case BT_SINK_SRV_ACTION_3WAY_HOLD_SPECIAL:
        case BT_SINK_SRV_ACTION_REPORT_BATTERY:
        case BT_SINK_SRV_ACTION_REPORT_BATTERY_EXT:
        case BT_SINK_SRV_ACTION_CALL_SET_VOLUME: {
            parameter_length = sizeof(uint8_t);
            break;
        }

        case BT_SINK_SRV_ACTION_QUERY_CALL_LIST:
        case BT_SINK_SRV_ACTION_SWITCH_AUDIO_PATH: {
            parameter_length = sizeof(bt_bd_addr_t);
            break;
        }

        case BT_SINK_SRV_ACTION_DTMF: {
            parameter_length = sizeof(bt_sink_srv_send_dtmf_t);
            break;
        }

        case BT_SINK_SRV_ACTION_DIAL_NUMBER_EXT: {
            parameter_length = sizeof(bt_sink_srv_dial_number_t);
            break;
        }

        case BT_SINK_SRV_ACTION_DIAL_NUMBER: {
            parameter_length = bt_sink_srv_strlen((char *)parameter);
            break;
        }

        default: {
            break;
        }
    }

    bt_sink_srv_report_id("[Sink][StaMgr]get parameter length: %d", 1, parameter_length);
    return parameter_length;
}
#endif
