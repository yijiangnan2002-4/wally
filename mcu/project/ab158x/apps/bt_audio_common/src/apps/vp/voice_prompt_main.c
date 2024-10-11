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
 * File: app_voice_prompt.c
 *
 * Description: This file provide implementation of voice prompt management and control.
 *
 */
#include "voice_prompt_internal.h"
#include "voice_prompt_main.h"
#include "voice_prompt_api.h"
#include "voice_prompt_aws.h"
#include "voice_prompt_local.h"
#include "voice_prompt_queue.h"
#include "voice_prompt_nvdm.h"
#include "ui_realtime_task.h"
#include "bt_device_manager.h"
#include "hal_gpt.h"
#include <assert.h>
#include "task.h"
#ifdef AIR_VOICE_PROMPT_COSYS_ENABLE
#include "voice_prompt_cosys.h"
#endif

typedef struct {
    voice_prompt_state_t state;
    voice_prompt_play_callback_t common_callback;
    bool cleanup;

} voice_prompt_context_t;

static voice_prompt_context_t g_voice_prompt_ctx = {0};

#define LOG_TAG  "VP_MAIN"

static void voice_prompt_main_set_state(voice_prompt_state_t state)
{
    VP_LOG_MSGID_I(LOG_TAG" set state %d", 1, state);
    g_voice_prompt_ctx.state = state;
}

/* Init. */
voice_prompt_status_t voice_prompt_init(voice_prompt_play_callback_t common_callback)
{
#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" init, VP feature option not enable!", 0);
    return VP_STATUS_FAIL;
#endif

    if (g_voice_prompt_ctx.state != VOICE_PROMPT_STAT_NONE) {
        //VP_LOG_MSGID_E(LOG_TAG" already inited !", 0);
        return VP_STATUS_FAIL;
    }

    VP_LOG_MSGID_I(LOG_TAG" init", 0);
    voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
    g_voice_prompt_ctx.common_callback = common_callback;

    voice_prompt_queue_init();
#ifdef VOICE_PROMPT_SYNC_ENABLE
    voice_prompt_aws_init();
#endif
    /* Init VP file. */
    voice_prompt_nvdm_init();

#ifdef AIR_VOICE_PROMPT_COSYS_ENABLE
    voice_prompt_cosys_init();
#endif
    return VP_STATUS_SUCCESS;
}



static inline void voice_prompt_main_noti(voice_prompt_play_callback_t callback, uint32_t index, voice_prompt_event_t event)
{
    if (g_voice_prompt_ctx.common_callback != NULL) {
        VP_LOG_MSGID_I(LOG_TAG" noti common vp_index %d, event %d", 2, index, event);
        g_voice_prompt_ctx.common_callback(index, event);
    }
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
    extern void app_usb_audio_vp_play_end_notify();
    if (event == VP_EVENT_PLAY_END || event == VP_EVENT_FAIL) {
        app_usb_audio_vp_play_end_notify();
    }
#endif
    if (callback != NULL) {
        VP_LOG_MSGID_I(LOG_TAG" noti item vp_index %d, event %d", 2, index, event);
        callback(index, event);
    }
}

static void voice_prompt_main_local_play(voice_prompt_list_item_t *item, uint32_t tar_gpt)
{
    voice_prompt_status_t status;

    if (item == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" item null", 0);
        assert(0);
        return;
    }

    item->state = VP_ITEM_STATE_PLAYING;
    voice_prompt_main_set_state(VOICE_PROMPT_STAT_PLAYING);

    status = voice_prompt_local_play(item->vp_index, tar_gpt);
    if (status == VP_STATUS_FILE_NOT_FOUND) {
        voice_prompt_main_noti(item->callback, item->vp_index, VP_EVENT_FILE_NO_FOUND);
    } else if (status == VP_STATUS_FAIL) {
        voice_prompt_main_noti(item->callback, item->vp_index, VP_EVENT_FAIL);
    }
    if (status != VP_STATUS_SUCCESS) {
        VP_LOG_MSGID_E(LOG_TAG" play item fail status %d, delete", 1, status);
        voice_prompt_queue_delete(0);
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
        ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_PROC_NEXT, NULL);
    }
}

#ifdef VOICE_PROMPT_SYNC_ENABLE
static void voice_prompt_main_sync_play(voice_prompt_list_item_t *item, bool first_time)
{
    voice_prompt_status_t status;
    voice_prompt_param_t param;
    uint32_t tar_gpt = 0;
    uint32_t cur_gpt;
    uint16_t delay;

    if (item == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" item null", 0);
        assert(0);
        return;
    }

    if ((item->control & VOICE_PROMPT_CONTROL_MASK_LOOP) && first_time) {
        VP_LOG_MSGID_I(LOG_TAG" sync_play RT 1st time", 0);
        delay = VOICE_PROMPT_RT_DEFAULT_DELAY;
    } else {
        delay = item->delay_time;
    }

    param.vp_index = item->vp_index;
    param.control = item->control;
    param.delay_time = item->delay_time;
    param.callback = NULL;
    param.target_gpt = 0;

    status = voice_prompt_aws_sync_play(&param, delay, &tar_gpt);

    if (status == VP_STATUS_AWS_IF_LOCKED) {
        VP_LOG_MSGID_I(LOG_TAG" sync_play IF locked", 0);
        if (first_time) {
            item->state = VP_ITEM_STATE_WAIT_SYNC_PLAY;
        } else {
            item->state = VP_ITEM_STATE_WAIT_SYNC_LOOP;
        }
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_WAIT_IF_UNLOCK);
        return;
    }

    if (status == VP_STATUS_AWS_DISCONNECT) {
        voice_prompt_main_noti(item->callback, item->vp_index, VP_EVENT_SYNC_FAIL);
        if (item->control & VOICE_PROMPT_CONTROL_MASK_SYNC_FAIL_NOT_PLAY) {
            VP_LOG_MSGID_I(LOG_TAG" sync_play fail not local play", 0);
            voice_prompt_queue_delete(0);
            voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
            ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_PROC_NEXT, NULL);
            return;
        }
    }

    if (tar_gpt == 0 && (item->control & VOICE_PROMPT_CONTROL_MASK_LOOP)) {
        VP_LOG_MSGID_I(LOG_TAG" sync_play ensure loop delay", 0);
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &cur_gpt);
        tar_gpt = cur_gpt + delay * 1000;
    }

    voice_prompt_main_local_play(item, tar_gpt);
}
#endif

static void voice_prompt_main_proc_item(voice_prompt_list_item_t *item, bool first_time)
{
#ifdef VOICE_PROMPT_SYNC_ENABLE
    bool sync = false;
#endif
    uint32_t cur_gpt, tar_gpt = 0;

    if (item == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" item null", 0);
        assert(0);
        return;
    }

    voice_prompt_main_noti(item->callback, item->vp_index, VP_EVENT_READY);

    if (g_voice_prompt_ctx.state == VOICE_PROMPT_STAT_PLAYING || g_voice_prompt_ctx.state == VOICE_PROMPT_STAT_STOPPING) {
        VP_LOG_MSGID_E(LOG_TAG" proc item, state wrong %d", 1, g_voice_prompt_ctx.state);
        assert(0);
    }

#ifdef VOICE_PROMPT_SYNC_ENABLE
    if (!first_time && (item->control & VOICE_PROMPT_CONTROL_MASK_SYNC)) {
        /* loop play sync VP, only Agent goes here. */
        sync = true;
    } else if (first_time && (item->control & VOICE_PROMPT_CONTROL_MASK_SYNC) && !(item->sync_by_peer)) {
        /* First play VP, sync due to flag to avoid RHO corner case. Except for sync_by_peer. */
        sync = true;
    }

    if (sync) {
        if (g_voice_prompt_ctx.state == VOICE_PROMPT_STAT_SNIFF_EXITING || g_voice_prompt_ctx.state == VOICE_PROMPT_STAT_WAIT_IF_UNLOCK) {
            VP_LOG_MSGID_I(LOG_TAG" proc item sync, state pending %d", 1, g_voice_prompt_ctx.state);
            item->state = VP_ITEM_STATE_WAIT_SYNC_PLAY;
            return;
        } else {
            voice_prompt_aws_state_t aws_state = voice_prompt_aws_get_state();
            //VP_LOG_MSGID_I(LOG_TAG" proc item sync, aws state %d", 1, aws_state);
            if (aws_state == VOICE_PROMPT_AWS_STATE_SNIFF) {
                if (VP_STATUS_SNIFF_EXITING == voice_prompt_aws_exit_sniff()) {
                    item->state = VP_ITEM_STATE_WAIT_SYNC_PLAY;
                    voice_prompt_main_set_state(VOICE_PROMPT_STAT_SNIFF_EXITING);
                    return;
                }
            }
            voice_prompt_main_sync_play(item, first_time);
        }
    } else
#endif
    {
        if (!first_time && item->delay_time != 0) {
            VP_LOG_MSGID_I(LOG_TAG" local_play ensure loop delay", 0);
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &cur_gpt);
            tar_gpt = cur_gpt + item->delay_time * 1000;
        } else {
            tar_gpt = item->target_gpt;
        }
        voice_prompt_main_local_play(item, tar_gpt);
    }

}

static void voice_prompt_main_local_stop(uint8_t idx, voice_prompt_list_item_t *item, uint32_t tar_gpt)
{
    VP_LOG_MSGID_I(LOG_TAG" voice_prompt_main_local_stop item->sta=%d", 1, item->state);
    if (item->state == VP_ITEM_STATE_STOPPING) {
        /* The state already in stopping */;
    } else if (item->state < VP_ITEM_STATE_PLAYING) {
        voice_prompt_queue_delete(idx);
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
        ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_PROC_NEXT, NULL);
    } else {
        item->state = VP_ITEM_STATE_STOPPING;
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_STOPPING);
        voice_prompt_local_stop(tar_gpt);
    }

}

#ifdef VOICE_PROMPT_SYNC_ENABLE
static void voice_prompt_main_sync_stop(uint8_t idx, voice_prompt_list_item_t *item, bool preempted)
{
    voice_prompt_status_t status;
    uint32_t tar_gpt = 0;

    if (VOICE_PROMPT_STAT_WAIT_IF_UNLOCK == g_voice_prompt_ctx.state) {
        VP_LOG_MSGID_I(LOG_TAG" stop_item sync IF locked", 0);
        item->state = VP_ITEM_STATE_WAIT_SYNC_STOP;
    }
    status = voice_prompt_aws_sync_stop(item->vp_index, preempted, &tar_gpt);
    if (status == VP_STATUS_AWS_IF_LOCKED) {
        item->state = VP_ITEM_STATE_WAIT_SYNC_STOP;
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_WAIT_IF_UNLOCK);
        return;
    } else if (status != VP_STATUS_SUCCESS) {
        voice_prompt_main_noti(item->callback, item->vp_index, VP_EVENT_SYNC_STOP_FAIL);
    }

    voice_prompt_main_local_stop(idx, item, tar_gpt);

}
#endif

static void voice_prompt_main_stop_item(uint8_t idx, voice_prompt_list_item_t *item, bool preempted, bool sync)
{
    if (item == NULL || idx > (voice_prompt_queue_get_current_length() - 1)) {
        //VP_LOG_MSGID_E(LOG_TAG" item null, idx %d", 1, idx);
        assert(0);
        return;
    }

    VP_LOG_MSGID_I(LOG_TAG" stop_item, idx %d, item state %d, preempted %d", 3, idx, item->state, preempted);
    if (preempted) {
        voice_prompt_main_noti(item->callback, item->vp_index, VP_EVENT_PREEMPTED);
    }

    if (item != voice_prompt_queue_get_current_item()) {
        VP_LOG_MSGID_I(LOG_TAG" stop_item not in the head, just delete", 0);
        voice_prompt_queue_delete(idx);
        return;
    }

    if (item->state == VP_ITEM_STATE_WAIT_SYNC_PLAY || item->state == VP_ITEM_STATE_WAIT_SYNC_LOOP) {
        VP_LOG_MSGID_I(LOG_TAG" stop_item id wait sync, just delete", 0);
        voice_prompt_queue_delete(idx);
        ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_PROC_NEXT, NULL);
        return;
    }

    if (item->state == VP_ITEM_STATE_STOPPING || item->state == VP_ITEM_STATE_WAIT_SYNC_STOP) {
        //VP_LOG_MSGID_W(LOG_TAG" already in stop progress", 0);
        return;
    }

#ifdef VOICE_PROMPT_SYNC_ENABLE
    if (sync) {
        if (g_voice_prompt_ctx.state == VOICE_PROMPT_STAT_SNIFF_EXITING || g_voice_prompt_ctx.state == VOICE_PROMPT_STAT_WAIT_IF_UNLOCK) {
            VP_LOG_MSGID_I(LOG_TAG" stop_item sync, state pending %d", 1, g_voice_prompt_ctx.state);
            item->state = VP_ITEM_STATE_WAIT_SYNC_STOP;
            return;
        } else {
            voice_prompt_aws_state_t aws_state = voice_prompt_aws_get_state();
            VP_LOG_MSGID_I(LOG_TAG" proc item sync, aws state %d", 1, aws_state);
            if (aws_state == VOICE_PROMPT_AWS_STATE_SNIFF) {
                if (VP_STATUS_SNIFF_EXITING == voice_prompt_aws_exit_sniff()) {
                    item->state = VP_ITEM_STATE_WAIT_SYNC_STOP;
                    voice_prompt_main_set_state(VOICE_PROMPT_STAT_SNIFF_EXITING);
                    return;
                }
            }
            voice_prompt_main_sync_stop(idx, item, preempted);
        }

    } else
#endif
    {
        voice_prompt_main_local_stop(idx, item, 0);
    }

}

static uint8_t voice_prompt_main_find_preempted_idx(voice_prompt_list_item_t *tail_item, uint8_t len)
{
    uint8_t idx = VOICE_PROMPT_QUEUE_MAX;
    int8_t i = len - 1;
    voice_prompt_list_item_t *item_after = tail_item;

    if (len < 2) {
        VP_LOG_MSGID_E(LOG_TAG" find_preempted_idx len %d < 2", 1, len);
        return idx;
    }

    while (i >= 0) {
        /* Search from tail, should insert after the first not allow preempted item. */
        if (item_after->control & VOICE_PROMPT_CONTROL_MASK_NO_PREEMPTED) {
            idx = i;
            break;
        }
        item_after --;
        i --;
    }

    return idx;;
}

static void voice_prompt_main_all_play_end()
{
#ifdef VOICE_PROMPT_SYNC_ENABLE
    voice_prompt_aws_unlock_sleep();
    voice_prompt_aws_enable_sniff(true);
#endif
    voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
}

static void voice_prompt_msg_hdl_proc_next()
{
    if (voice_prompt_queue_get_current_length() != 0) {
        voice_prompt_main_proc_item(voice_prompt_queue_get_current_item(), true);
    } else {
        VP_LOG_MSGID_I(LOG_TAG" hdl_proc_next, queue empty", 0);
        voice_prompt_main_all_play_end();
    }
}

static void voice_prompt_main_new_vp(voice_prompt_msg_set_vp_t *vp, bool synced)
{
    voice_prompt_list_item_t *cur_item = voice_prompt_queue_get_current_item();
    voice_prompt_list_item_t *tail_item = voice_prompt_queue_get_tail_item();
    uint8_t cur_num = voice_prompt_queue_get_current_length();
    uint8_t after_idx;

    if (vp == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" new_vp, input NULL", 0);
        assert(0);
    }

    VP_LOG_MSGID_I(LOG_TAG" new_vp, index %d, control 0x%x, delay %d, gpt 0x%08x, cb %d, id %d", 6,
                   vp->param.vp_index, vp->param.control, vp->param.delay_time, vp->param.target_gpt, (vp->param.callback == NULL) ? 1 : 0, vp->play_id);

    if (g_voice_prompt_ctx.cleanup) {
        VP_LOG_MSGID_W(LOG_TAG" new_vp, already cleanup", 0);
        voice_prompt_main_noti(vp->param.callback, vp->param.vp_index, VP_EVENT_FAIL);
        return;
    }

    if (vp->param.control & VOICE_PROMPT_CONTROL_MASK_CLEANUP) {
        //VP_LOG_MSGID_I(LOG_TAG" new_vp, set clean up", 0);
        g_voice_prompt_ctx.cleanup = true;
    }

    if (!(g_voice_prompt_ctx.cleanup) &&
        tail_item && (tail_item->control & VOICE_PROMPT_CONTROL_MASK_SKIP_OTHER) && (tail_item->state < VP_ITEM_STATE_WAIT_SYNC_STOP)) {
        VP_LOG_MSGID_W(LOG_TAG" new_vp, skip", 0);
        voice_prompt_main_noti(vp->param.callback, vp->param.vp_index, VP_EVENT_FAIL);
        return;
    }

    VP_LOG_MSGID_I(LOG_TAG" new_vp, cur_num %d, cur_state %d", 2, cur_num, g_voice_prompt_ctx.state);

    if (cur_num == 0) {
        if (VOICE_PROMPT_STAT_INIT != g_voice_prompt_ctx.state) {
            assert(0);
        }
        /* Put in queue and play the incoming. */
        VP_LOG_MSGID_I(LOG_TAG" new_vp, just play", 0);
        if (voice_prompt_queue_push(&(vp->param), vp->play_id, synced) == VP_QUEUE_SUCCESS) {
            voice_prompt_main_proc_item(voice_prompt_queue_get_current_item(), true);
        }
    } else {
        if (synced && (vp->param.control & VOICE_PROMPT_CONTROL_MASK_SYNCED_FAIL_NOT_PLAY)) {
            /* Sync VP set sync_fail_not_play flag, means if partner has current playing VP, should ignore the sync VP. */
            VP_LOG_MSGID_W(LOG_TAG" synced fail not play", 0);
            return;
        }

        if (g_voice_prompt_ctx.cleanup) {
            VP_LOG_MSGID_I(LOG_TAG" new_vp, clean up all to play", 0);
            voice_prompt_queue_delete_excp(0);
            voice_prompt_queue_push(&(vp->param), vp->play_id, synced);
            voice_prompt_main_stop_item(0, cur_item, true, false);
            return;
        }

        if (cur_num == 1) {
            if (vp->param.control & VOICE_PROMPT_CONTROL_MASK_PREEMPT) {
                /* The incoming vp want to preempted. */
                if ((cur_item->control & VOICE_PROMPT_CONTROL_MASK_NO_PREEMPTED) || (cur_item->state == VP_ITEM_STATE_STOPPING)) {
                    /* If the current item not allow preempted, or it is already in stopping, just put the incoming in queue. */
                    VP_LOG_MSGID_I(LOG_TAG" new_vp, cur not allow preempted or stopping, put in queue", 0);
                    voice_prompt_queue_push(&(vp->param), vp->play_id, synced);
                } else {
                    /* Stop current item, and put in queue to wait play after stop success. */
                    VP_LOG_MSGID_I(LOG_TAG" new_vp, stop current", 0);
                    if (voice_prompt_queue_push(&(vp->param), vp->play_id, synced) == VP_QUEUE_SUCCESS) {
                        voice_prompt_main_stop_item(0, cur_item, true, (cur_item->control & VOICE_PROMPT_CONTROL_MASK_SYNC) ? true : false);
                    }
                }
            } else {
                /* Don't want to preempt, just put in queue. */
                VP_LOG_MSGID_I(LOG_TAG" new_vp, put in queue", 0);
                voice_prompt_queue_push(&(vp->param), vp->play_id, synced);
            }
        } else {
            /* Current VP item number > 1, need check the current and the tail item. */
            if (vp->param.control & VOICE_PROMPT_CONTROL_MASK_PREEMPT) {
                after_idx = voice_prompt_main_find_preempted_idx(tail_item, cur_num);
                if (tail_item == NULL) {
                    //VP_LOG_MSGID_E(LOG_TAG" new_vp, tail_item NULL", 0);
                    assert(0);
                    return;
                }
                if (after_idx != VOICE_PROMPT_QUEUE_MAX) {
                    /* Find a index to insert after, just put in queue. */
                    VP_LOG_MSGID_I(LOG_TAG" new_vp, insert after idx %d", 1, after_idx);
                    voice_prompt_queue_insert_after(&(vp->param), vp->play_id, synced, after_idx);
                } else {
                    /* All the items in queue allow preempted, just stop current and put after the stopping one. */
                    VP_LOG_MSGID_I(LOG_TAG" new_vp, insert after current", 0);
                    if (voice_prompt_queue_insert_after(&(vp->param), vp->play_id, synced, 0) == VP_QUEUE_SUCCESS) {

                        if (cur_item->state != VP_ITEM_STATE_STOPPING) {
                            /* Insert to the head, so stop the current item. */
                            VP_LOG_MSGID_I(LOG_TAG" new_vp, stop current", 0);
                            voice_prompt_main_stop_item(0, cur_item, true, (cur_item->control & VOICE_PROMPT_CONTROL_MASK_SYNC));
                        }

                    }
                }
            } else {
                /* Don't want to preempt, just put in queue. */
                VP_LOG_MSGID_I(LOG_TAG" new_vp, put in queue", 0);
                voice_prompt_queue_push(&(vp->param), vp->play_id, synced);
            }
        }
    }

}

void voice_prompt_msg_hdl_set_vp(voice_prompt_msg_set_vp_t *vp)
{
    voice_prompt_main_new_vp(vp, false);

    vPortFree((void *)vp);
}


static void voice_prompt_msg_hdl_update_control(voice_prompt_msg_set_vp_t *vp)
{
    voice_prompt_list_item_t *item = NULL;

    if (vp == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_update, input NULL", 0);
        assert(0);
    }

    item = voice_prompt_queue_get_item_by_id(vp->play_id);
    if (item == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_update, not found item with id %d", 1, vp->play_id);
        vPortFree((void *)vp);
        return;
    }

    if (vp->param.vp_index != item->vp_index) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_update, VP index not match %d -> %d", 2, item->vp_index, vp->param.vp_index);
        vPortFree((void *)vp);
        return;
    }

    item->callback = vp->param.callback;
    item->control = vp->param.control;
    item->delay_time = vp->param.delay_time;
    item->target_gpt = vp->param.target_gpt;

    vPortFree((void *)vp);
}

#ifdef VOICE_PROMPT_SYNC_ENABLE
static void voice_prompt_msg_hdl_play_peer(voice_prompt_param_t *param)
{
    if (param == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_play_peer, input NULL", 0);
        assert(0);
    }

    if (voice_prompt_aws_play_peer(param) == VP_STATUS_AWS_IF_LOCKED) {
#if 0
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_WAIT_IF_UNLOCK);
#endif
    }

    vPortFree((void *)param);
}
#endif

static void voice_prompt_msg_hdl_stop_vp(voice_prompt_stop_t *data)
{
    uint8_t idx;
    voice_prompt_list_item_t *item = NULL;
    voice_prompt_list_item_t *cur_item = voice_prompt_queue_get_current_item();

    if (data == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_stop_vp, input NULL", 0);
        assert(0);
    }

    VP_LOG_MSGID_I(LOG_TAG" hdl_stop_vp, id %d, index %d, on_peer %d, sync %d", 4,
                   data->play_id, data->vp_index, data->on_peer, data->sync);

    if (cur_item == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_stop_vp, current no item", 0);
        vPortFree((void *)data);
        return;
    }

#ifdef VOICE_PROMPT_SYNC_ENABLE
    if (data->on_peer) {
        if (voice_prompt_aws_stop_peer(data->vp_index, data->sync) == VP_STATUS_AWS_IF_LOCKED) {
#if 0
            voice_prompt_main_set_state(VOICE_PROMPT_STAT_WAIT_IF_UNLOCK);
#endif
        }
    } else
#endif
    {
        if (data->play_id > VOICE_PROMPT_ID_MAX || data->play_id == VOICE_PROMPT_ID_INVALID) {
            VP_LOG_MSGID_I(LOG_TAG" hdl_stop_vp, ID=%d, id=%d", 2, cur_item->vp_index, data->vp_index);
            if (cur_item->vp_index != data->vp_index) {
                voice_prompt_queue_delete_by_vp_index(data->vp_index);
            } else {
                voice_prompt_queue_delete_by_vp_index_skip_cur(data->vp_index);
                voice_prompt_main_stop_item(0, cur_item, false, data->sync);
            }
        } else {
            if (voice_prompt_queue_get_idx_by_id(data->play_id, &idx) != VP_QUEUE_SUCCESS) {
                //VP_LOG_MSGID_E(LOG_TAG" hdl_stop_vp, queue idx not found", 0);
                vPortFree((void *)data);
                return;
            }
            item = voice_prompt_queue_get_item_by_id(data->play_id);
            if (item == NULL) {
                //VP_LOG_MSGID_E(LOG_TAG" hdl_stop_vp, item not found", 0);
                vPortFree((void *)data);
                return;
            }
            voice_prompt_main_stop_item(idx, item, false, data->sync);
        }
    }

    vPortFree((void *)data);
}

static void voice_prompt_msg_hdl_clear_all()
{
    bool sync = false;
    voice_prompt_list_item_t *cur_item = voice_prompt_queue_get_current_item();
    if (cur_item == NULL) {
        //VP_LOG_MSGID_W(LOG_TAG" hdl_clear_all, current no item", 0);
        return;
    }

#ifdef VOICE_PROMPT_SYNC_ENABLE
    sync = (cur_item->control & VOICE_PROMPT_CONTROL_MASK_SYNC) ? true : false;
#endif

    voice_prompt_queue_delete_excp(0);
    voice_prompt_main_stop_item(0, cur_item, false, sync);
}

void voice_prompt_msg_hdl_lang_set(voice_prompt_set_lang_t *data)
{
    if (data == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_lang_set, input NULL", 0);
        assert(0);
    }

#ifdef VOICE_PROMPT_SYNC_ENABLE
    //voice_prompt_status_t status;
    if (data->sync) {
        voice_prompt_aws_sync_language(data->lang_idx, data->codec);
#if 0
        if (status ==  VP_STATUS_AWS_IF_LOCKED) {
            voice_prompt_main_set_state(VOICE_PROMPT_STAT_WAIT_IF_UNLOCK);
        }
#endif
    }
#endif

    voice_prompt_nvdm_set_current_lang(data->lang_idx);

    vPortFree((void *)data);
}

static void voice_prompt_msg_hdl_play_start()
{
    voice_prompt_list_item_t *cur_item = voice_prompt_queue_get_current_item();

    if (cur_item == NULL) {
        VP_LOG_MSGID_E(LOG_TAG" hdl_play_start, current no item", 0);
        return;
    }

    voice_prompt_main_noti(cur_item->callback, cur_item->vp_index, VP_EVENT_START_PLAY);
}

static void voice_prompt_msg_hdl_play_end()
{
    voice_prompt_list_item_t *cur_item = voice_prompt_queue_get_current_item();
#ifdef VOICE_PROMPT_SYNC_ENABLE
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
#endif
    bool loop = false;

    if (cur_item == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_play_end, current no item", 0);
#ifdef VOICE_PROMPT_SYNC_ENABLE
        voice_prompt_aws_unlock_sleep();
#endif
        return;
    }

    if (cur_item->control & VOICE_PROMPT_CONTROL_MASK_LOOP) {
#ifdef VOICE_PROMPT_SYNC_ENABLE
        if ((cur_item->control & VOICE_PROMPT_CONTROL_MASK_SYNC) && role == BT_AWS_MCE_ROLE_AGENT) {
            /* Only Agent loop sync VP. */
#else
        if ((cur_item->control & VOICE_PROMPT_CONTROL_MASK_SYNC)) {
#endif
            loop = true;
        } else if (!(cur_item->control & VOICE_PROMPT_CONTROL_MASK_SYNC)) {
            /* Allow all roles loop play local VP. */
            loop = true;
        }
    }

    VP_LOG_MSGID_I(LOG_TAG" hdl_play_end, loop=%d, sta=%d play index=%d", 3,
                   loop, cur_item->state, cur_item->vp_index);
    if (!loop || cur_item->state > VP_ITEM_STATE_PLAYING) {
        voice_prompt_main_noti(cur_item->callback, cur_item->vp_index, VP_EVENT_PLAY_END);
        voice_prompt_queue_delete(0);
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
        voice_prompt_msg_hdl_proc_next();
    } else {
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
        /* Delay to ensure partner play end. */
        if (cur_item->control & VOICE_PROMPT_CONTROL_MASK_SYNC) {
            vTaskDelay(50);
        }
        voice_prompt_main_proc_item(cur_item, false);
    }

}

#ifdef VOICE_PROMPT_SYNC_ENABLE
void voice_prompt_msg_hdl_sniff_change(voice_prompt_sniff_change_t *data)
{
    voice_prompt_list_item_t *cur_item = voice_prompt_queue_get_current_item();

    if (data == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_sniff_change, input NULL", 0);
        assert(0);
    }

    voice_prompt_aws_exit_sniff_cnf((void *)data);

    if (cur_item == NULL) {
        //VP_LOG_MSGID_W(LOG_TAG" hdl_sniff_change, queue NULL", 0);
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
        vPortFree((void *)data);
        return;
    }

    if (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_PLAY) {
        voice_prompt_main_sync_play(cur_item, true);
    } else if (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_LOOP) {
        voice_prompt_main_sync_play(cur_item, false);
    } else if (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_STOP) {
        voice_prompt_main_sync_stop(0, cur_item, false);
    } else if (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_PREEMPT) {
        voice_prompt_main_sync_stop(0, cur_item, true);
    }

    vPortFree((void *)data);
}

void voice_prompt_msg_hdl_synced_play(voice_prompt_param_t *data)
{
    voice_prompt_msg_set_vp_t set_vp = {0};
    if (data == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_synced_play, input NULL", 0);
        assert(0);
    }

    memcpy((void *) & (set_vp.param), (void *)data, sizeof(voice_prompt_param_t));
    set_vp.play_id = VOICE_PROMPT_ID_SYNCED;
    voice_prompt_main_new_vp(&set_vp, true);

    vPortFree((void *)data);
}

void voice_prompt_msg_hdl_synced_stop(voice_prompt_synced_stop_t *data)
{
    voice_prompt_list_item_t *cur_item = voice_prompt_queue_get_current_item();

    if (data == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" hdl_synced_stop, input NULL", 0);
        assert(0);
    }

    if (cur_item == NULL) {
        //VP_LOG_MSGID_W(LOG_TAG" hdl_synced_stop, curr no item", 0);
        vPortFree((void *)data);
        return;
    }

    VP_LOG_MSGID_I(LOG_TAG" hdl_synced_stop, ID1=%d, ID2=%d", 2, cur_item->vp_index, data->vp_index);
    if (cur_item->vp_index != data->vp_index) {
        voice_prompt_queue_delete_by_vp_index(data->vp_index);
    } else {
        voice_prompt_queue_delete_by_vp_index_skip_cur(data->vp_index);
        voice_prompt_main_local_stop(0, cur_item, data->tar_gpt);
    }

    vPortFree((void *)data);
}

static void voice_prompt_msg_hdl_if_unlock()
{
    voice_prompt_list_item_t *cur_item = voice_prompt_queue_get_current_item();

    voice_prompt_aws_hdl_if_unlock();

    if (cur_item == NULL) {
        //VP_LOG_MSGID_W(LOG_TAG" hdl_if_unlock, curr no item", 0);
        return;
    }

    VP_LOG_MSGID_I(LOG_TAG" IF unlock, curr_state %d, cur_item_state %d", 2, g_voice_prompt_ctx.state, cur_item->state);

    if (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_PLAY || cur_item->state == VP_ITEM_STATE_WAIT_SYNC_LOOP) {
        if (VP_STATUS_SNIFF_EXITING == voice_prompt_aws_exit_sniff()) {
            voice_prompt_main_set_state(VOICE_PROMPT_STAT_SNIFF_EXITING);
            return;
        }
        voice_prompt_main_sync_play(cur_item, (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_PLAY) ? true : false);
    } else if (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_STOP || cur_item->state == VP_ITEM_STATE_WAIT_SYNC_PREEMPT) {
        if (VP_STATUS_SNIFF_EXITING == voice_prompt_aws_exit_sniff()) {
            voice_prompt_main_set_state(VOICE_PROMPT_STAT_SNIFF_EXITING);
            return;
        }
        voice_prompt_main_sync_stop(0, cur_item, (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_PREEMPT) ? true : false);
    }
}

static void voice_prompt_msg_hdl_if_timeout()
{
    voice_prompt_list_item_t *cur_item = voice_prompt_queue_get_current_item();
    uint32_t tar_gpt = 0;
    uint32_t cur_gpt;

    if (cur_item == NULL) {
        //VP_LOG_MSGID_W(LOG_TAG" hdl_if_timeout, curr no item", 0);
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
        return;
    }

    VP_LOG_MSGID_I(LOG_TAG" if_timeout, cur_state %d, cur_item_state %d", 2, g_voice_prompt_ctx.state, cur_item->state);

    if (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_PLAY || cur_item->state == VP_ITEM_STATE_WAIT_SYNC_LOOP) {
        voice_prompt_main_noti(cur_item->callback, cur_item->vp_index, VP_EVENT_SYNC_FAIL);

        VP_LOG_MSGID_I(LOG_TAG" if_timeout fail control=%d", 1, cur_item->control);
        if (cur_item->control & VOICE_PROMPT_CONTROL_MASK_SYNC_FAIL_NOT_PLAY) {
            /* Delete. */
            voice_prompt_queue_delete(0);
            voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
            ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_PROC_NEXT, NULL);
        } else {
            /* Local play. */
            if (cur_item->control & VOICE_PROMPT_CONTROL_MASK_LOOP) {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &cur_gpt);
                if (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_LOOP) {
                    tar_gpt = cur_gpt + cur_item->delay_time * 1000;
                } else {
                    tar_gpt = cur_gpt + VOICE_PROMPT_RT_DEFAULT_DELAY * 1000;
                }
            }
            voice_prompt_main_local_play(cur_item, tar_gpt);
        }
    } else if (cur_item->state == VP_ITEM_STATE_WAIT_SYNC_STOP || cur_item->state == VP_ITEM_STATE_WAIT_SYNC_PREEMPT) {
        voice_prompt_main_noti(cur_item->callback, cur_item->vp_index, VP_EVENT_SYNC_STOP_FAIL);
        voice_prompt_main_local_stop(0, cur_item, 0);
    } else {
        //voice_prompt_main_set_state(VOICE_PROMPT_STAT_INIT);
    }
}

static void voice_prompt_msg_hdl_remote_disc()
{
    voice_prompt_aws_hdl_remote_disconnect();

    if (g_voice_prompt_ctx.state == VOICE_PROMPT_STAT_SNIFF_EXITING) {
        /* When smart phone all disconnect, AWS will try to switch to special, so set state to wait IF unlock. */
        voice_prompt_main_set_state(VOICE_PROMPT_STAT_WAIT_IF_UNLOCK);
    }
}
#endif

void voice_prompt_msg_handle(uint16_t msg_id, void *data)
{
#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" msg_hdl, VP feature option not enable!", 0);
    return;
#endif

    if (g_voice_prompt_ctx.state == VOICE_PROMPT_STAT_NONE) {
        //VP_LOG_MSGID_E(LOG_TAG" msg_hdl, VP has not inited!", 0);
        return;
    }

    VP_LOG_MSGID_I(LOG_TAG" handle msg %d", 1, msg_id);

    switch (msg_id) {
        case UI_REALTIME_MSG_SET_VP: {
            voice_prompt_msg_hdl_set_vp((voice_prompt_msg_set_vp_t *)data);
            break;
        }
        case UI_REALTIME_MSG_UPDATE_CONTROL: {
            voice_prompt_msg_hdl_update_control((voice_prompt_msg_set_vp_t *)data);
            break;
        }
        case UI_REALTIME_MSG_STOP_VP: {
            voice_prompt_msg_hdl_stop_vp((voice_prompt_stop_t *)data);
            break;
        }
        case UI_REALTIME_MSG_CLEAR_ALL: {
            voice_prompt_msg_hdl_clear_all();
            break;
        }
        case UI_REALTIME_MSG_LANGUAGE_SET: {
            voice_prompt_msg_hdl_lang_set((voice_prompt_set_lang_t *)data);
            break;
        }
        case UI_REALTIME_MSG_PLAY_START: {
            voice_prompt_msg_hdl_play_start();
            break;
        }
        case UI_REALTIME_MSG_PLAY_END: {
            voice_prompt_msg_hdl_play_end();
            break;
        }
        case UI_REALTIME_MSG_PROC_NEXT: {
            voice_prompt_msg_hdl_proc_next();
            break;
        }
#ifdef VOICE_PROMPT_SYNC_ENABLE
        case UI_REALTIME_MSG_PLAY_PEER: {
            voice_prompt_msg_hdl_play_peer((voice_prompt_param_t *)data);
            break;
        }
        case UI_REALTIME_MSG_SNIFF_CHANGE: {
            voice_prompt_msg_hdl_sniff_change((voice_prompt_sniff_change_t *)data);
            break;
        }
        case UI_REALTIME_MSG_SYNCED_PLAY: {
            voice_prompt_msg_hdl_synced_play((voice_prompt_param_t *)data);
            break;
        }
        case UI_REALTIME_MSG_SYNCED_STOP: {
            voice_prompt_msg_hdl_synced_stop((voice_prompt_synced_stop_t *)data);
            break;
        }
        case UI_REALTIME_MSG_IF_UNLOCK: {
            voice_prompt_msg_hdl_if_unlock();
            break;
        }
        case UI_REALTIME_MSG_IF_TIMEOUT: {
            voice_prompt_msg_hdl_if_timeout();
            break;
        }
        case UI_REALTIME_MSG_REMOTE_DISCONNECT: {
            voice_prompt_msg_hdl_remote_disc();
            break;
        }
#endif
        default: {
            //VP_LOG_MSGID_E(LOG_TAG" msg ID wrong", 0);
            break;
        }
    }
}


void voice_prompt_remote_disconnect()
{
#ifdef AIR_PROMPT_SOUND_ENABLE
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_REMOTE_DISCONNECT, NULL);
#endif
}

void voice_prompt_IF_unlock()
{
#ifdef AIR_PROMPT_SOUND_ENABLE
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_IF_UNLOCK, NULL);
#endif
}
