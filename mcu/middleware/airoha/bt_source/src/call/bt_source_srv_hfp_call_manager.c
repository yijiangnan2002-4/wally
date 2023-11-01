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

#include "bt_source_srv_hfp_call_manager.h"
#include "bt_source_srv_utils.h"
#include "bt_source_srv_hfp.h"
#include "bt_source_srv.h"


static bt_source_srv_hfp_call_context_t g_source_srv_hfp_call_context[BT_SOURCE_SRV_HFP_CALL_MAX] = {{0}};

typedef bt_status_t (*bt_source_srv_hfp_call_handle_multiparty_t)(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address);
static bt_status_t bt_source_srv_hfp_call_handle_release_all_held(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address);
static bt_status_t bt_source_srv_hfp_call_handle_release_active_accept_other(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address);
static bt_status_t bt_source_srv_hfp_call_handle_release_special(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address);
static bt_status_t bt_source_srv_hfp_call_handle_hold_active_accept_other(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address);
static bt_status_t bt_source_srv_hfp_call_handle_hold_special(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address);
static bt_status_t bt_source_srv_hfp_call_handle_add_held_to_conversation(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address);
static bt_status_t bt_source_srv_hfp_call_handle_explicit_transfer(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address);
static const bt_source_srv_hfp_call_handle_multiparty_t g_hfp_call_handle_multiparty[] = {
    bt_source_srv_hfp_call_handle_release_all_held,
    bt_source_srv_hfp_call_handle_release_active_accept_other,
    bt_source_srv_hfp_call_handle_release_special,
    bt_source_srv_hfp_call_handle_hold_active_accept_other,
    bt_source_srv_hfp_call_handle_hold_special,
    bt_source_srv_hfp_call_handle_add_held_to_conversation,
    bt_source_srv_hfp_call_handle_explicit_transfer
};

static void bt_source_srv_hfp_call_reset_context(bt_source_srv_hfp_call_context_t *context)
{
    bt_source_srv_memset(context, 0, sizeof(bt_source_srv_hfp_call_context_t));
}

static bt_source_srv_hfp_call_context_t *bt_source_srv_hfp_call_get_free_context(void)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_HFP_CALL_MAX; i++) {
        bt_source_srv_hfp_call_context_t *context = &g_source_srv_hfp_call_context[i];
        if (context->index == BT_SOURCE_SRV_CALL_INVALID_INDEX) {
            bt_source_srv_hfp_call_reset_context(context);
            /* Array subscript plus 1 as index value */
            context->index = i + 1;
            LOG_MSGID_I(source_srv, "[HFP][AG][CALL] get free call context index = %02x", 1, context->index);
            return context;
        }
    }
    LOG_MSGID_I(source_srv, "[HFP][AG][CALL] get free call context fail", 0);
    return NULL;
}

static bt_source_srv_hfp_call_context_t *bt_source_srv_hfp_call_find_context_by_index(bt_source_srv_call_index_t index)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_HFP_CALL_MAX; i++) {
        bt_source_srv_hfp_call_context_t *context = &g_source_srv_hfp_call_context[i];
        if (context->index == index) {
            LOG_MSGID_I(source_srv, "[HFP][AG][CALL] find call context index = %02x", 1, context->index);
            return context;
        }
    }
    LOG_MSGID_I(source_srv, "[HFP][AG][CALL] find call context fail by index = %02x", 1, index);
    return NULL;
}

/* attention: this API cannot find all call held call */
static bt_source_srv_hfp_call_context_t *bt_source_srv_hfp_call_find_context_by_state(bt_source_srv_call_state_t call_state)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_HFP_CALL_MAX; i++) {
        bt_source_srv_hfp_call_context_t *context = &g_source_srv_hfp_call_context[i];
        if (context->call_state == call_state) {
            return context;
        }
    }
    LOG_MSGID_W(source_srv, "[HFP][AG] cannot find call context by state = %02x", 1, call_state);
    return NULL;
}

static bt_status_t bt_source_srv_hfp_call_handle_release_all_held(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_source_srv_hfp_call_context_t *waiting_call_context = bt_source_srv_hfp_call_find_context_by_state(BT_SOURCE_SRV_CALL_STATE_WAITING);

    if (NULL == waiting_call_context) {
        waiting_call_context = bt_source_srv_hfp_call_find_context_by_state(BT_SOURCE_SRV_CALL_STATE_INCOMING);
    }

    for (uint32_t i = 0; i < BT_SOURCE_SRV_HFP_CALL_MAX; i++) {
        bt_source_srv_hfp_call_context_t *call_context = &g_source_srv_hfp_call_context[i];
        if (call_context->call_state == BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD) {
            bt_source_srv_terminate_call_t terminate_call_ind = {
                .type = BT_SOURCE_SRV_TYPE_HFP,
                .index = call_context->index,
            };
            bt_source_srv_memcpy(&terminate_call_ind.peer_address.addr, remote_address, sizeof(bt_bd_addr_t));
            bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_TERMINATE_CALL, &terminate_call_ind, sizeof(bt_source_srv_terminate_call_t));
            status = BT_STATUS_SUCCESS;
        }
    }

    if (NULL != waiting_call_context) {
        bt_source_srv_reject_call_t reject_call_ind = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
            .index = waiting_call_context->index,
        };
        bt_source_srv_memcpy(&reject_call_ind.peer_address.addr, remote_address, sizeof(bt_bd_addr_t));
        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_REJECT_CALL, &reject_call_ind, sizeof(bt_source_srv_accept_call_t));
    }

    return status;
}

static bt_status_t bt_source_srv_hfp_call_handle_release_active_accept_other(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address)
{
    bt_source_srv_hfp_call_context_t *active_call_context = bt_source_srv_hfp_call_find_context_by_state(BT_SOURCE_SRV_CALL_STATE_ACTIVE);
    bt_source_srv_hfp_call_context_t *held_call_context = bt_source_srv_hfp_call_find_context_by_state(BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD);
    bt_source_srv_hfp_call_context_t *waiting_call_context = bt_source_srv_hfp_call_find_context_by_state(BT_SOURCE_SRV_CALL_STATE_WAITING);

    if (NULL == waiting_call_context) {
        waiting_call_context = bt_source_srv_hfp_call_find_context_by_state(BT_SOURCE_SRV_CALL_STATE_INCOMING);
    }

    if ((NULL == active_call_context) && (NULL == held_call_context) && (NULL == waiting_call_context)) {
        return BT_STATUS_FAIL;
    }

    /* 1.release active call */
    if (NULL != active_call_context) {
        bt_source_srv_terminate_call_t terminate_call_ind = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
            .index = active_call_context->index,
        };
        bt_source_srv_memcpy(&terminate_call_ind.peer_address.addr, remote_address, sizeof(bt_bd_addr_t));
        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_TERMINATE_CALL, &terminate_call_ind, sizeof(bt_source_srv_terminate_call_t));
    }

    /* 2.accept held call or waiting call */
    if (NULL != held_call_context) {
        bt_source_srv_unhold_t unhold_call_ind = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
            .index = held_call_context->index,
        };
        bt_source_srv_memcpy(&unhold_call_ind.peer_address.addr, remote_address, sizeof(bt_bd_addr_t));
        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_UNHOLD, &unhold_call_ind, sizeof(bt_source_srv_unhold_t));
    }

    if (NULL != waiting_call_context) {
        bt_source_srv_accept_call_t accept_call_ind = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
            .index = waiting_call_context->index,
        };
        bt_source_srv_memcpy(&accept_call_ind.peer_address.addr, remote_address, sizeof(bt_bd_addr_t));
        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_ACCEPT_CALL, &accept_call_ind, sizeof(bt_source_srv_accept_call_t));
    }

    return BT_STATUS_SUCCESS;

}

static bt_status_t bt_source_srv_hfp_call_handle_release_special(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address)
{
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_call_handle_hold_active_accept_other(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address)
{
    bt_source_srv_hfp_call_context_t *active_call_context = bt_source_srv_hfp_call_find_context_by_state(BT_SOURCE_SRV_CALL_STATE_ACTIVE);
    bt_source_srv_hfp_call_context_t *held_call_context = bt_source_srv_hfp_call_find_context_by_state(BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD);
    bt_source_srv_hfp_call_context_t *waiting_call_context = bt_source_srv_hfp_call_find_context_by_state(BT_SOURCE_SRV_CALL_STATE_WAITING);

    if ((NULL == active_call_context) && (NULL == held_call_context) && (NULL == waiting_call_context)) {
        return BT_STATUS_FAIL;
    }

    /* 1.place active call to hold */
    if (NULL != active_call_context) {
        bt_source_srv_hold_t hold_call_ind = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
            .index = active_call_context->index,
        };
        bt_source_srv_memcpy(&hold_call_ind.peer_address.addr, remote_address, sizeof(bt_bd_addr_t));
        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_HOLD, &hold_call_ind, sizeof(bt_source_srv_hold_t));
    }

    /* 2.accept held call or waiting call */
    if (NULL != held_call_context) {
        bt_source_srv_unhold_t unhold_call_ind = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
            .index = held_call_context->index,
        };
        bt_source_srv_memcpy(&unhold_call_ind.peer_address.addr, remote_address, sizeof(bt_bd_addr_t));
        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_UNHOLD, &unhold_call_ind, sizeof(bt_source_srv_unhold_t));
    }

    if (NULL != waiting_call_context) {
        bt_source_srv_accept_call_t accept_call_ind = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
            .index = waiting_call_context->index,
        };
        bt_source_srv_memcpy(&accept_call_ind.peer_address.addr, remote_address, sizeof(bt_bd_addr_t));
        bt_source_srv_hfp_event_notify(BT_SOURCE_SRV_EVENT_ACCEPT_CALL, &accept_call_ind, sizeof(bt_source_srv_accept_call_t));
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_call_handle_hold_special(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address)
{
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_call_handle_add_held_to_conversation(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address)
{
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_hfp_call_handle_explicit_transfer(bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address)
{
    return BT_STATUS_SUCCESS;
}

bt_source_srv_call_index_t bt_source_srv_hfp_create_new_call(bt_source_srv_hfp_new_call_t *new_call)
{
    bt_source_srv_hfp_call_context_t *context = bt_source_srv_hfp_call_get_free_context();
    if ((context == NULL) || (new_call->state > BT_SOURCE_SRV_CALL_STATE_WAITING)) {
        LOG_MSGID_E(source_srv, "[HFP][AG][CALL] new call state = %02x create fail", 1, new_call->state);
        return BT_SOURCE_SRV_CALL_INVALID_INDEX;
    }

    LOG_MSGID_I(source_srv, "[HFP][AG][CALL] create new call index = %02x, state = %02x, mode = %02x, mpty = %02x, iac = %02x", 5,
                context->index, new_call->state, new_call->mode, new_call->mpty, new_call->iac);

    bt_source_srv_call_state_t previous_state = context->call_state;
    context->call_state = new_call->state;
    context->mode = new_call->mode;
    context->mpty = new_call->mpty;
    context->iac = new_call->iac;
    if ((context->call_state == BT_SOURCE_SRV_CALL_STATE_INCOMING) || (context->call_state == BT_SOURCE_SRV_CALL_STATE_WAITING)) {
        context->dir = BT_SOURCE_SRV_HFP_CALL_DIR_MT;
    }

    if (new_call->number_length <= BT_SOURCE_SRV_PHONE_NUMBER_LENGTH) {
        bt_source_srv_memcpy(context->phone_number, new_call->number, new_call->number_length);
    } else {
        LOG_MSGID_W(source_srv, "[HFP][AG][CALL] new call handle number_length over %02x", 1, BT_SOURCE_SRV_PHONE_NUMBER_LENGTH);
        bt_source_srv_hfp_call_reset_context(context);
        return BT_SOURCE_SRV_CALL_INVALID_INDEX;
    }
    /* notify HF call state */
    bt_source_srv_hfp_transfer_call_state(previous_state, new_call->state);
    return context->index;
}

bt_status_t bt_source_srv_hfp_call_state_change(bt_source_srv_hfp_call_state_change_t *call_change)
{
    bt_source_srv_hfp_call_context_t *context = bt_source_srv_hfp_call_find_context_by_index(call_change->index);
    if ((context == NULL) || (call_change->state > BT_SOURCE_SRV_CALL_STATE_WAITING)) {
        LOG_MSGID_E(source_srv, "[HFP][AG][CALL] call cstate = %02x change fail", 1, call_change->state);
        return BT_STATUS_FAIL;
    }

    LOG_MSGID_I(source_srv, "[HFP][AG][CALL] call change index  = %02x, state = %02x, mpty = %02x", 3,
                call_change->index, call_change->state,  call_change->mpty);

    bt_source_srv_call_state_t previous_state = context->call_state;
    if (call_change->state == BT_SOURCE_SRV_CALL_STATE_NONE) {
        bt_source_srv_hfp_call_reset_context(context);
    } else {
        context->call_state = call_change->state;
    }
    context->mpty = call_change->mpty;
    /* notify HF call state */
    bt_source_srv_hfp_transfer_call_state(previous_state, call_change->state);
    return BT_STATUS_SUCCESS;
}

uint32_t bt_source_srv_hfp_call_get_call_list(bt_source_srv_hfp_call_context_t *call_list, uint32_t list_count)
{
    uint32_t i = 0;
    uint32_t real_count = 0;
    for (i = 0; i < BT_SOURCE_SRV_HFP_CALL_MAX; i++) {
        bt_source_srv_hfp_call_context_t *context = &g_source_srv_hfp_call_context[i];
        if (context->index != BT_SOURCE_SRV_CALL_INVALID_INDEX) {
            bt_source_srv_memcpy(&call_list[real_count], context, sizeof(bt_source_srv_hfp_call_context_t));
            real_count++;
            if (real_count >= list_count) {
                break;
            }
        }
    }
    return real_count;
}

uint32_t bt_source_srv_hfp_call_get_context_by_state(bt_source_srv_call_state_t call_state,
        bt_source_srv_hfp_call_context_t *call_list, uint32_t list_count)
{
    uint32_t real_count = 0;
    for (uint32_t i = 0; i < BT_SOURCE_SRV_HFP_CALL_MAX; i++) {
        bt_source_srv_hfp_call_context_t *context = &g_source_srv_hfp_call_context[i];
        if (context->call_state == call_state) {
            bt_source_srv_memcpy(&call_list[real_count], context, sizeof(bt_source_srv_hfp_call_context_t));
            real_count++;
            if (real_count >= list_count) {
                break;
            }
        }
    }
    LOG_MSGID_I(source_srv, "[HFP][AG] get call context number = %02x by state = %02x", 2, real_count, call_state);
    return real_count;
}

bool bt_source_srv_hfp_call_is_exist(void)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_HFP_CALL_MAX; i++) {
        bt_source_srv_hfp_call_context_t *context = &g_source_srv_hfp_call_context[i];
        if ((context->call_state != BT_SOURCE_SRV_CALL_STATE_NONE) && (context->index != BT_SOURCE_SRV_CALL_INVALID_INDEX)) {
            return true;
        }
    }
    return false;
}

bool bt_source_srv_hfp_call_is_exist_by_state(bt_source_srv_call_state_t call_state)
{
    for (uint32_t i = 0; i < BT_SOURCE_SRV_HFP_CALL_MAX; i++) {
        bt_source_srv_hfp_call_context_t *context = &g_source_srv_hfp_call_context[i];
        if ((context->call_state == call_state) && (context->index != BT_SOURCE_SRV_CALL_INVALID_INDEX)) {
            return true;
        }
    }
    return false;
}

bt_status_t bt_source_srv_hfp_call_handle_multiparty(bt_hfp_hold_call_t hold_type,
        bt_source_srv_call_index_t call_index, bt_bd_addr_t *remote_address)
{
    LOG_MSGID_I(source_srv, "[HFP][AG] handle multiparty type = %02x, call index = %02x", 2, hold_type, call_index);
    if (hold_type > BT_HFP_HOLD_CALL_EXPLICIT_TRANSFER) {
        return BT_STATUS_FAIL;
    }
    return g_hfp_call_handle_multiparty[hold_type](call_index, remote_address);
}
