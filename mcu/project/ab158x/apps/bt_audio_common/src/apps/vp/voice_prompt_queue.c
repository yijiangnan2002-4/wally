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

#include "voice_prompt_queue.h"
#include "voice_prompt_local.h"

static voice_prompt_list_item_t g_voice_prompt_queue[VOICE_PROMPT_QUEUE_MAX] = {0};
static uint8_t g_voice_prompt_queue_len = 0;
static uint16_t g_voice_prompt_curr_id = 0;

#define LOG_TAG   "VP_QUEUE"

void voice_prompt_queue_init()
{
    uint8_t i = 0;

    //VP_LOG_MSGID_I(LOG_TAG" reinit, queue max len %d", 1, VOICE_PROMPT_QUEUE_MAX);

    for (i = 0; i < VOICE_PROMPT_QUEUE_MAX; i ++) {
        g_voice_prompt_queue[i].id = VOICE_PROMPT_ID_INVALID;
    }
    g_voice_prompt_queue_len = 0;

    return;
}

uint8_t voice_prompt_queue_get_current_length()
{
    return g_voice_prompt_queue_len;
}

uint16_t voice_prompt_queue_generate_id()
{
    g_voice_prompt_curr_id ++;

    if (VOICE_PROMPT_ID_MAX <= g_voice_prompt_curr_id) {
        /* Start from 1. */
        g_voice_prompt_curr_id = 1;
    }

    VP_LOG_MSGID_I(LOG_TAG" gen id %d", 1, g_voice_prompt_curr_id);
    return g_voice_prompt_curr_id;
}

static void voice_prompt_queue_fill_item(uint8_t fill_idx, uint16_t id, voice_prompt_param_t *param, bool sync_by_peer)
{
    g_voice_prompt_queue[fill_idx].vp_index = param->vp_index;
    g_voice_prompt_queue[fill_idx].control = param->control;
    g_voice_prompt_queue[fill_idx].callback = param->callback;
    g_voice_prompt_queue[fill_idx].delay_time = param->delay_time;
    g_voice_prompt_queue[fill_idx].target_gpt = param->target_gpt;

    g_voice_prompt_queue[fill_idx].sync_by_peer = sync_by_peer;

    g_voice_prompt_queue[fill_idx].state = VP_ITEM_STATE_QUEUE;
    g_voice_prompt_queue[fill_idx].id = id;
}

voice_prompt_queue_status_t voice_prompt_queue_insert_before(voice_prompt_param_t *param, uint16_t id, bool sync_by_peer, uint8_t before_index)
{
    uint8_t i;

    if (g_voice_prompt_queue_len >= VOICE_PROMPT_QUEUE_MAX) {
        VP_LOG_MSGID_E(LOG_TAG" insert_before, queue full! curr %d >= max %d", 2, g_voice_prompt_queue_len, VOICE_PROMPT_QUEUE_MAX);
        //assert(0);
        return VP_QUEUE_FULL;
    }

    if (before_index >= g_voice_prompt_queue_len - 1) {
        //VP_LOG_MSGID_E(LOG_TAG" insert_before, param wrong! before %d, len %d", 2, before_index, g_voice_prompt_queue_len);
        return VP_QUEUE_PARAM_WRONG;
    }

    VP_LOG_MSGID_I(LOG_TAG" insert_before idx %d, synced %d, vp_index %d, curr_len %d, id %d", 5,
                   before_index, sync_by_peer, param->vp_index, g_voice_prompt_queue_len, id);

    for (i = g_voice_prompt_queue_len; i > before_index; i --) {
        g_voice_prompt_queue[i] = g_voice_prompt_queue[i - 1];
    }

    voice_prompt_queue_fill_item(before_index, id, param, sync_by_peer);
    g_voice_prompt_queue_len ++;

    return VP_QUEUE_SUCCESS;
}

voice_prompt_queue_status_t voice_prompt_queue_insert_after(voice_prompt_param_t *param, uint16_t id, bool sync_by_peer, uint8_t after_index)
{
    uint8_t i;

    if (g_voice_prompt_queue_len >= VOICE_PROMPT_QUEUE_MAX) {
        VP_LOG_MSGID_E(LOG_TAG" insert_after, queue full! curr %d >= max %d", 2, g_voice_prompt_queue_len, VOICE_PROMPT_QUEUE_MAX);
        //assert(0);
        return VP_QUEUE_FULL;
    }

    if (after_index > g_voice_prompt_queue_len - 1) {
        //VP_LOG_MSGID_E(LOG_TAG" insert_after, param wrong! after %d, len %d", 2, after_index, g_voice_prompt_queue_len);
        return VP_QUEUE_PARAM_WRONG;
    }

    VP_LOG_MSGID_I(LOG_TAG" insert_after idx %d, synced %d, vp_index %d, curr_len %d, id %d", 5,
                   after_index, sync_by_peer, param->vp_index, g_voice_prompt_queue_len, id);

    for (i = g_voice_prompt_queue_len; i > after_index + 1; i --) {
        g_voice_prompt_queue[i] = g_voice_prompt_queue[i - 1];
    }

    voice_prompt_queue_fill_item(after_index + 1, id, param, sync_by_peer);
    g_voice_prompt_queue_len ++;

    return VP_QUEUE_SUCCESS;
}

voice_prompt_queue_status_t voice_prompt_queue_replace(voice_prompt_param_t *param, uint16_t id, bool sync_by_peer, uint8_t replace_idx)
{
    if (replace_idx >= g_voice_prompt_queue_len - 1) {
        //VP_LOG_MSGID_E(LOG_TAG" replace idx wrong %d, len %d", 2, replace_idx, g_voice_prompt_queue_len);
        return VP_QUEUE_PARAM_WRONG;
    }

    VP_LOG_MSGID_I(LOG_TAG" replace idx %d, synced %d, vp_index %d, curr_len %d, id %d", 5,
                   replace_idx, sync_by_peer, param->vp_index, g_voice_prompt_queue_len, id);
    voice_prompt_queue_fill_item(replace_idx, id, param, sync_by_peer);

    return VP_QUEUE_SUCCESS;
}

voice_prompt_queue_status_t voice_prompt_queue_push(voice_prompt_param_t *param, uint16_t id, bool sync_by_peer)
{
    if (g_voice_prompt_queue_len >= VOICE_PROMPT_QUEUE_MAX) {
        VP_LOG_MSGID_E(LOG_TAG" push, queue full! curr %d >= max %d", 2, g_voice_prompt_queue_len, VOICE_PROMPT_QUEUE_MAX);
        //assert(0);
        return VP_QUEUE_FULL;
    }

    VP_LOG_MSGID_I(LOG_TAG" push VP, synced %d, vp_index %d, curr_len %d, id %d", 4,
                   sync_by_peer, param->vp_index, g_voice_prompt_queue_len, id);

    voice_prompt_queue_fill_item(g_voice_prompt_queue_len, id, param, sync_by_peer);
    g_voice_prompt_queue_len ++;
    return VP_QUEUE_SUCCESS;
}

voice_prompt_queue_status_t voice_prompt_queue_delete(uint8_t list_idx)
{
    uint8_t i = list_idx;

    if (list_idx >= g_voice_prompt_queue_len) {
        //VP_LOG_MSGID_E(LOG_TAG" delete idx wrong %d, curr_len %d", 2, list_idx, g_voice_prompt_queue_len);
        return VP_QUEUE_PARAM_WRONG;
    }

    VP_LOG_MSGID_I(LOG_TAG" delete idx %d, curr_len %d", 2, list_idx, g_voice_prompt_queue_len);

    while (i < g_voice_prompt_queue_len - 1) {
        g_voice_prompt_queue[i] = g_voice_prompt_queue[i + 1];
        i ++;
    }
    g_voice_prompt_queue[g_voice_prompt_queue_len - 1].id = VOICE_PROMPT_ID_INVALID;
    g_voice_prompt_queue_len --;

    return VP_QUEUE_SUCCESS;
}

voice_prompt_queue_status_t voice_prompt_queue_delete_by_vp_index_start(uint32_t vp_index, uint8_t start_idx)
{
    uint8_t i = 0, valid = 0;

    i = start_idx;
    valid = start_idx;

    VP_LOG_MSGID_I(LOG_TAG" delete VP_index %d, curr_len %d, start_idx %d", 3, vp_index, g_voice_prompt_queue_len, start_idx);

    if (start_idx > g_voice_prompt_queue_len - 1) {
        return VP_QUEUE_FAIL;
    }

    while (i < g_voice_prompt_queue_len) {
        if (g_voice_prompt_queue[i].vp_index != vp_index) {
            if (valid != i) {
                g_voice_prompt_queue[valid] = g_voice_prompt_queue[i];
            }
            valid ++;
        }
        i ++;
    }

    g_voice_prompt_queue_len = valid;
    VP_LOG_MSGID_I(LOG_TAG" delete done, curr_len %d", 1, g_voice_prompt_queue_len);

    return VP_QUEUE_SUCCESS;
}

voice_prompt_queue_status_t voice_prompt_queue_delete_by_vp_index(uint32_t vp_index)
{
    return voice_prompt_queue_delete_by_vp_index_start(vp_index, 0);
}

voice_prompt_queue_status_t voice_prompt_queue_delete_by_vp_index_expcur(uint32_t vp_index)
{
    return voice_prompt_queue_delete_by_vp_index_start(vp_index, 1);
}

voice_prompt_queue_status_t voice_prompt_queue_delete_all()
{
    VP_LOG_MSGID_I(LOG_TAG" delete all, curr_len %d", 1, g_voice_prompt_queue_len);
    voice_prompt_queue_init();
    return VP_QUEUE_SUCCESS;
}

voice_prompt_queue_status_t voice_prompt_queue_delete_excp(uint8_t list_idx)
{
    voice_prompt_list_item_t item = g_voice_prompt_queue[list_idx];

    if (list_idx >= g_voice_prompt_queue_len) {
        VP_LOG_MSGID_E(LOG_TAG" delete_excp idx wrong %d, curr_len %d", 2, list_idx, g_voice_prompt_queue_len);
        return VP_QUEUE_PARAM_WRONG;
    }

    VP_LOG_MSGID_I(LOG_TAG" delete_excp idx %d, curr_len %d", 2, list_idx, g_voice_prompt_queue_len);

    voice_prompt_queue_delete_all();
    g_voice_prompt_queue[0] = item;
    g_voice_prompt_queue_len = 1;

    return VP_QUEUE_SUCCESS;
}

voice_prompt_queue_status_t voice_prompt_queue_get_idx_by_id(uint16_t id, uint8_t *idx)
{
    uint8_t i = 0;

    if (id > VOICE_PROMPT_ID_MAX || id == VOICE_PROMPT_ID_INVALID) {
        //VP_LOG_MSGID_E(LOG_TAG" get_idx_by_id %d param wrong!", 1, id);
        return VP_QUEUE_PARAM_WRONG;
    }

    while (i < g_voice_prompt_queue_len) {
        if (g_voice_prompt_queue[i].id == id) {
            break;
        } else {
            i ++;
        }
    }

    if (i >= g_voice_prompt_queue_len) {
        VP_LOG_MSGID_E(LOG_TAG" get_idx_by_id %d not found", 1, id);
        return VP_QUEUE_NOT_FOUND;
    }

    *idx = i;
    return VP_QUEUE_SUCCESS;
}

voice_prompt_queue_status_t voice_prompt_queue_delete_by_id(uint16_t id)
{
    uint8_t idx;

    if (voice_prompt_queue_get_idx_by_id(id, &idx) == VP_QUEUE_SUCCESS) {
        voice_prompt_queue_delete(idx);
        return VP_QUEUE_SUCCESS;
    } else {
        return VP_QUEUE_PARAM_WRONG;
    }
}

voice_prompt_list_item_t *voice_prompt_queue_get_item_by_id(uint16_t id)
{
    uint8_t idx;
    if (voice_prompt_queue_get_idx_by_id(id, &idx) == VP_QUEUE_SUCCESS) {
        return &g_voice_prompt_queue[idx];
    } else {
        return NULL;
    }
}


voice_prompt_list_item_t *voice_prompt_queue_get_current_item()
{
    if (g_voice_prompt_queue[0].id == VOICE_PROMPT_ID_INVALID) {
        return NULL;
    } else {
        return &g_voice_prompt_queue[0];
    }
}

voice_prompt_list_item_t *voice_prompt_queue_get_tail_item()
{
    if (g_voice_prompt_queue_len == 0) {
        return NULL;
    } else {
        return &g_voice_prompt_queue[g_voice_prompt_queue_len - 1];
    }
}
