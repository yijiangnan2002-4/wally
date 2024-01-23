/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

#include "bt_type.h"
#include "bt_spp.h"
#include "bt_callback_manager.h"
#include "bt_ull_utility.h"
#include "bt_ull_audio_manager.h"
#include "bt_os_layer_api.h"
#include "bt_sink_srv_state_notify.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_ull_aws_mce.h"
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
#include "bt_role_handover.h"
#endif
#endif
#include "bt_timer_external.h"
#include "audio_src_srv.h"
#include "bt_utils.h"


static void bt_ull_am_codec_open_ind(void *param);
static void bt_ull_am_fill_audio_src_callback(audio_src_srv_handle_t *handle);
static void bt_ull_am_play_ind(audio_src_srv_handle_t *handle);
static void bt_ull_am_suspend_ind(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd);
static void bt_ull_am_stop_ind(audio_src_srv_handle_t *handle);
static void bt_ull_am_reject_ind(audio_src_srv_handle_t *handle);
static void bt_ull_am_exception_ind(audio_src_srv_handle_t *handle, int32_t event, void *param);
static void bt_ull_am_set_am_state(bt_ull_am_state_t state);
static bt_ull_am_state_t bt_ull_am_get_am_state(void);
static void bt_ull_am_set_am_substate(bt_ull_am_sub_state_t state);
static bt_ull_am_sub_state_t bt_ull_am_get_am_substate(void);
static void bt_ull_am_ind_state_handle(bt_ull_am_ind_state_t ind_state, bool enable);

#ifdef MTK_AWS_MCE_ENABLE
static void bt_ull_am_agent_send_eir_stop(void);
#endif

static void bt_ull_am_remove_waiting_list(void);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_A2DP_STANDBY_ENABLE
static void bt_ull_am_delay_play_timeout_cb(uint32_t timer_id, uint32_t data);
#endif


audio_src_srv_handle_t *bt_ull_am_init(void)
{
    audio_src_srv_handle_t *handle = NULL;
    ull_report("[ULL][API] bt_ull_am_init ", 0);
    handle = audio_src_srv_construct_handle(AUDIO_SRC_SRV_PSEUDO_DEVICE_ULL_EDR);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_A2DP_STANDBY_ENABLE
    handle->priority = 1; /* lower priority than a2dp */
#else
    handle->priority = AUDIO_SRC_SRV_PRIORITY_ABOVE_NORMAL; /* higher priority than a2dp */
#endif
    bt_ull_am_fill_audio_src_callback(handle);
    /* Update audio source state */
    audio_src_srv_update_state(handle, AUDIO_SRC_SRV_EVT_READY);
    bt_ull_am_set_am_state(BT_ULL_AM_READY);
    return handle;
}

void bt_ull_am_deinit(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();

    bt_ull_am_sub_state_t substate = bt_ull_am_get_am_substate();
    bt_ull_am_state_t state = bt_ull_am_get_am_state();
    ull_report("[ULL][API] bt_ull_am_deinit, am_state: 0x%x, substate:0x%x", 2, state, substate);

    bt_ull_am_remove_waiting_list();
#ifdef AIR_BT_ULTRA_LOW_LATENCY_A2DP_STANDBY_ENABLE
    bt_timer_ext_stop(BT_ULL_DELAY_PLAY_TIMER_ID);
#endif
    /* codec is opening, we just need wait */
    if (BT_ULL_SUB_STATE_PREPARE_CODEC == substate
        || BT_ULL_SUB_STATE_PREPARE_AUDIO_SRC == substate) {
        BT_ULL_SET_FLAG(ctx->flag, BT_ULL_FLAG_WAIT_AMI_STOP_CODEC);
        bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_CLEAR_CODEC);
    } else if (BT_ULL_SUB_STATE_NONE == substate
               && BT_ULL_AM_PLAYING == state) {
        BT_ULL_SET_FLAG(ctx->flag, BT_ULL_FLAG_WAIT_AMI_STOP_CODEC);
        bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_CLEAR_CODEC);
        /* Release audio source */
        audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
    } else {
        /* Do nothing */
        if (ctx->am_handle) {
            audio_src_srv_destruct_handle(ctx->am_handle);
            ctx->am_handle = NULL;
            bt_ull_am_set_am_state(BT_ULL_AM_IDLE);
        }
        /* give and destruct the mic resource handle*/
        if (ctx->ul_microphone.resource_handle != NULL) {
            if ((AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS == ctx->ul_microphone.resource_handle->state)
                || (AUDIO_SRC_SRV_EVENT_SUSPEND == ctx->ul_microphone.resource_handle->state)) {
                audio_src_srv_resource_manager_give(ctx->ul_microphone.resource_handle);
            }
            audio_src_srv_resource_manager_destruct_handle(ctx->ul_microphone.resource_handle);
            ctx->ul_microphone.resource_handle = NULL;
        }
    }
}


void bt_ull_am_play(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_am_sub_state_t substate = bt_ull_am_get_am_substate();
    bt_ull_am_state_t state = bt_ull_am_get_am_state();
    bool is_allow_play = false;
    bt_ull_am_remove_waiting_list();
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        ull_report("[ULL][API] bt_ull_am_play pending due to rho is onging", 0);
        return;
    }
#endif

#ifdef MTK_AWS_MCE_ENABLE
    if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()) {
        bt_role_t role;
        bt_gap_connection_handle_t gap_hd = bt_gap_get_handle_by_address((const bt_bd_addr_t *) & (ctx->bt_addr));
        if ((bt_gap_get_role_sync(gap_hd, &role) == BT_STATUS_SUCCESS) && (role == BT_ROLE_SLAVE)) {
            is_allow_play = true;
        }
    } else {
        /* partner no need check SLAVE role */
        is_allow_play = true;
    }
#else
    bt_role_t role;
    bt_gap_connection_handle_t gap_hd = bt_gap_get_handle_by_address((const bt_bd_addr_t *) & (ctx->bt_addr));
    if ((bt_gap_get_role_sync(gap_hd, &role) == BT_STATUS_SUCCESS) && (role == BT_ROLE_SLAVE)) {
        is_allow_play = true;
    }
#endif

    ull_report("[ULL][API] bt_ull_am_play, is_allow_play:0x%x", 1, is_allow_play);

    if (BT_ULL_AM_READY == state && is_allow_play) {
        if (BT_ULL_SUB_STATE_NONE == substate) {
            /* substate is waiting query audio source callback */
            bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_PREPARE_AUDIO_SRC);
            /* Query audio source play */
            audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
        } else if (BT_ULL_SUB_STATE_CLEAR_CODEC == substate) {
            /* codec is opening, we just need wait */
            bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_PREPARE_CODEC);
        }
    }
}

void bt_ull_am_stop(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_am_sub_state_t substate = bt_ull_am_get_am_substate();
    bt_ull_am_state_t state = bt_ull_am_get_am_state();
    ull_report("[ULL][API] bt_ull_am_stop, am_state: 0x%x, substate:0x%x", 2, state, substate);

    bt_ull_am_remove_waiting_list();
#ifdef AIR_BT_ULTRA_LOW_LATENCY_A2DP_STANDBY_ENABLE
    bt_timer_ext_stop(BT_ULL_DELAY_PLAY_TIMER_ID);
#endif
    /* codec is opening, we just need wait */
    if (BT_ULL_SUB_STATE_PREPARE_CODEC == substate
        || BT_ULL_SUB_STATE_PREPARE_AUDIO_SRC == substate) {
        bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_CLEAR_CODEC);
    } else if (BT_ULL_SUB_STATE_NONE == substate
               && BT_ULL_AM_PLAYING == state) {
        bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_CLEAR_CODEC);
        /* Release audio source */
        audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
    } else {
        /* Do nothing */
    }
}

void bt_ull_am_restart(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_am_sub_state_t substate = bt_ull_am_get_am_substate();
    bt_ull_am_state_t state = bt_ull_am_get_am_state();
    ull_report("[ULL][API] bt_ull_am_restart, am_state: 0x%x, substate:0x%x", 2, state, substate);

    /* codec is opening, we just need wait */
    if (BT_ULL_SUB_STATE_PREPARE_CODEC == substate) {
        BT_ULL_SET_FLAG(ctx->flag, BT_ULL_FLAG_WAIT_AMI_RESTART_CODEC);
        bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_CLEAR_CODEC);
    } else if (BT_ULL_SUB_STATE_NONE == substate
               && BT_ULL_AM_PLAYING == state) {
        BT_ULL_SET_FLAG(ctx->flag, BT_ULL_FLAG_WAIT_AMI_RESTART_CODEC);
        bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_CLEAR_CODEC);
        /* Release audio source */
        audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
    } else {
        /* Do nothing */
    }
}

/**
 * @brief                     Request to mute the current player.
 * @param[in] is_mute             is the action of mute.
 * @return                    The result of set mute.
 */
int32_t bt_ull_am_set_mute(bool is_mute)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    int ret = 0;
    ull_report("[ULL][API] set mute : %d, ull_role:0x%x, am_state:0x%x", 3, is_mute, ctx->ull_role, ctx->am_state);
    if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
        ctx->dl_speaker.is_mute = is_mute;
        if (BT_ULL_AM_PLAYING == ctx->am_state) {
            ret = bt_sink_srv_ami_audio_set_mute(ctx->audio_id, is_mute, STREAM_OUT);
            ull_report("[ULL][API] set mute, am_handle:0x%02x, mute:%d, ret:0x%08x",
                       3, ctx->am_handle, is_mute, ret);
        }
    }
    return ret;
}

int32_t bt_ull_am_set_volume(uint8_t volume)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    int ret = -1;
    ull_report("[ULL][API] set volume : %d, ull_role:0x%x, am_state:0x%x", 3, volume, ctx->ull_role, ctx->am_state);
    if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
        ctx->dl_speaker.volume.vol_left = volume;
        if (0 == ctx->dl_speaker.volume.vol_left) {
            bt_ull_am_set_mute(true);
        } else {
            if (ctx->dl_speaker.is_mute) {
                bt_ull_am_set_mute(false);
            }
        }
        if (BT_ULL_AM_PLAYING == ctx->am_state) {
            ret = bt_sink_srv_ami_audio_set_volume(ctx->audio_id, volume, STREAM_OUT);
            ull_report("[ULL][API] set volume, am_handle:0x%02x, volume:%d, ret:0x%08x",
                       3, ctx->audio_id, volume, ret);
        }
    }
    return ret;
}


void bt_ull_am_add_waiting_list(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL][API] bt_ull_am_add_waiting_list ", 0);

    audio_src_srv_add_waiting_list(ctx->am_handle);
}

void bt_ull_am_remove_waiting_list(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL][API] bt_ull_am_remove_waiting_list ", 0);
    audio_src_srv_del_waiting_list(ctx->am_handle);
}

void bt_ull_am_set_am_state(bt_ull_am_state_t state)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ctx->am_state = state;
    ull_report("[ULL] bt_ull_am_set_am_state : 0x%x", 1,
               state);
}

bt_ull_am_state_t bt_ull_am_get_am_state(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL] bt_ull_am_get_am_state : 0x%x", 1,
               ctx->am_state);
    return ctx->am_state;
}

void bt_ull_am_set_am_substate(bt_ull_am_sub_state_t state)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ctx->am_substate = state;
    ull_report("[ULL] bt_ull_am_set_am_substate : 0x%x", 1,
               state);
}

bt_ull_am_sub_state_t bt_ull_am_get_am_substate(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL] bt_ull_am_get_am_substate : 0x%x", 1,
               ctx->am_substate);
    return ctx->am_substate;
}


void bt_ull_am_callback(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param)
{
    int32_t ret = 0;
    int32_t err_code = 0;
    bt_ull_context_t *ctx = bt_ull_get_context();

    BT_ULL_MUTEX_LOCK();
    if ((ctx->audio_id == aud_id) &&
        (msg_id == AUD_A2DP_PROC_IND) &&
        (sub_msg == AUD_STREAM_EVENT_DATA_REQ ||
         sub_msg == AUD_CODEC_MEDIA_AWS_CHECK_CLOCK_SKEW ||
         sub_msg == AUD_CODEC_MEDIA_AWS_CHECK_UNDERFLOW)) {
        // drop
        ;
    } else {
        ull_report("[ULL] [ami]-aid: %d, aud_id: %d, msg_id: %d, sub_msg: %d, 2nd: 0x%x", 5,
                   ctx->am_handle, aud_id, msg_id, sub_msg, sub_msg);
    }
    if (ctx->audio_id == aud_id) {
        switch (msg_id) {
            case AUD_SINK_OPEN_CODEC: {
                bt_ull_am_codec_open_ind(param);
                break;
            }
            case AUD_SELF_CMD_REQ: {
                if (AUD_CMD_COMPLETE == sub_msg) {
                    /* ull codec suspend/stop will be unlock svfs */
                    bt_ull_am_sub_state_t substate = bt_ull_am_get_am_substate();
                    ull_report("[ULL] [ami] ull codec stop done, substate:0x%x ", 1, substate);
                    if (BT_ULL_SUB_STATE_NONE == substate) {
#ifdef AIR_BTA_IC_PREMIUM_G3
                    bt_ull_dvfs_unlock(HAL_DVFS_OPP_MID);
#else
                        bt_ull_dvfs_unlock(HAL_DVFS_FULL_SPEED_104M);
#endif
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    if (ctx->audio_id == aud_id && msg_id == AUD_A2DP_PROC_IND &&
        (sub_msg == AUD_STREAM_EVENT_DATA_REQ ||
         sub_msg == AUD_CODEC_MEDIA_AWS_CHECK_CLOCK_SKEW ||
         sub_msg == AUD_CODEC_MEDIA_AWS_CHECK_UNDERFLOW)) {
        // drop
        ;
    } else {
        ull_report("[ULL] [ami]-am_callback done, err_code: %d, ret: %d", 2, err_code, ret);
    }
    BT_ULL_MUTEX_UNLOCK();
}


#ifdef MTK_AWS_MCE_ENABLE
static void bt_ull_am_agent_send_eir_stop(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();

    if (BT_AWS_MCE_ROLE_AGENT == role) {
        bt_ull_aws_mce_eir_info_t eir_info = {0};
        eir_info.event = BT_ULL_AWS_MCE_EVT_STOP;
        eir_info.param.aws_stop.dl_is_streaming = ctx->dl_speaker.is_streaming;
        eir_info.param.aws_stop.ul_is_streaming = ctx->ul_microphone.is_streaming;
        bt_ull_aws_mce_send_eir(&eir_info, sizeof(eir_info), true);
        ull_report("[ULL][AWS] agent sync streaming stop", 0);
    }
}
#endif

static void bt_ull_am_ind_state_handle(bt_ull_am_ind_state_t ind_state, bool enable)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL][am_callback] bt_ull_am_ind_state_handle, state:%d, is_enable:%d", 2, ind_state, enable);
#ifdef MTK_AWS_MCE_ENABLE
        bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
        if (BT_AWS_MCE_ROLE_AGENT == role
            || BT_AWS_MCE_ROLE_NONE == role) {
            /* fdcb notify controller enable, controller will be clear music downlink avm buffer */
            uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
            bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_AWS_MCE_AGENT, enable, ctx->dl_latency);

            if(BT_ULL_AM_CODEC_OPEN_IND == ind_state){
                /* enable microphone */
                ull_report("[ULL][am_callbak] bt_ull_am_codec_open_ind, ul_microphone.is_streaming:0x%x", 1, ctx->ul_microphone.is_streaming);
                if (ctx->ul_microphone.is_streaming) {
                    /* start voice path */
                    if (ctx->ul_microphone.resource_handle != NULL) {
                        audio_src_srv_resource_manager_take(ctx->ul_microphone.resource_handle);
                    }
                }
            } else if (BT_ULL_AM_SUSPEND_IND == ind_state) {
              bt_ull_notify_server_play_is_allow(BT_ULL_PLAY_DISALLOW);
              bt_ull_am_agent_send_eir_stop();
            } else if (BT_ULL_AM_STOP_IND == ind_state) {
              bt_ull_am_agent_send_eir_stop();
            } else if (BT_ULL_AM_REJECT_IND == ind_state) {
              bt_ull_notify_server_play_is_allow(BT_ULL_PLAY_DISALLOW);
            } else {
              ull_report("am_ind_state error",0);
            }
        } else {
            /* fdcb notify controller enable, controller will be clear music downlink avm buffer */
            uint32_t aws_handle = bt_aws_mce_srv_get_aws_handle(&(ctx->bt_addr));
            bt_ull_set_music_enable(aws_handle, BT_AVM_ROLE_AWS_MCE_PARTNER, enable, ctx->dl_latency);
        }
#else
        if(BT_ULL_AM_CODEC_OPEN_IND == ind_state) {
            /* enable microphone */
            ull_report("[ULL][am_callback] bt_ull_am_codec_open_ind, ul_microphone.is_streaming:0x%x", 1, ctx->ul_microphone.is_streaming);
            if (ctx->ul_microphone.is_streaming) {
                /* start voice path */
                if (ctx->ul_microphone.resource_handle != NULL) {
                    audio_src_srv_resource_manager_take(ctx->ul_microphone.resource_handle);
                }
            }
        }
        /* fdcb notify controller enable, controller will be clear music downlink avm buffer */
        uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
        bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, enable, ctx->dl_latency);
        if((BT_ULL_AM_SUSPEND_IND == ind_state) || (BT_ULL_AM_REJECT_IND == ind_state)) {
            bt_ull_notify_server_play_is_allow(BT_ULL_PLAY_DISALLOW);
        }
#endif
}

static void bt_ull_am_fill_audio_src_callback(audio_src_srv_handle_t *handle)
{
    bt_utils_assert(handle);
    handle->play = bt_ull_am_play_ind;
    handle->stop = bt_ull_am_stop_ind;
    handle->suspend = bt_ull_am_suspend_ind;
    handle->reject = bt_ull_am_reject_ind;
    handle->exception_handle = bt_ull_am_exception_ind;
}

static void bt_ull_am_play_ind(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_am_audio_capability_t aud_cap = {0};
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_sink_srv_am_result_t am_ret;

    ull_report("[ULL][am_callbak] get audio src succeed--hd: 0x%x, ctx->am_handle: 0x%x, voice transmitter state: 0x%x", 3,
               handle, ctx->am_handle, ctx->ul_microphone.is_transmitter_start);
    BT_ULL_MUTEX_LOCK();
    if (ctx->am_handle && (ctx->am_handle == handle)) {
        bt_ull_am_sub_state_t substate = bt_ull_am_get_am_substate();
        if (BT_ULL_SUB_STATE_NONE == substate
            || BT_ULL_SUB_STATE_PREPARE_AUDIO_SRC == substate) {
            /* special handle for A2DP resume maybe has noise due to ull playing a short time */
            /* audio priority: HFP > A2DP > ULL */
#ifdef AIR_BT_ULTRA_LOW_LATENCY_A2DP_STANDBY_ENABLE
            /* resume from waiting list, then start a timer to wait other high priority audio source interrupt */
            if (BT_ULL_SUB_STATE_NONE == substate) {
#ifdef MTK_AWS_MCE_ENABLE
                if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()) {
                    bt_timer_ext_start(BT_ULL_DELAY_PLAY_TIMER_ID, 0, 800, bt_ull_am_delay_play_timeout_cb);
                }
#else
                bt_timer_ext_start(BT_ULL_DELAY_PLAY_TIMER_ID, 0, 800, bt_ull_am_delay_play_timeout_cb);
#endif
                audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                BT_ULL_MUTEX_UNLOCK();
                return;
            }
#endif
            bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_PREPARE_CODEC);
#ifdef MTK_AWS_MCE_ENABLE
            if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()) {
                bt_ull_notify_server_play_is_allow(BT_ULL_PLAY_ALLOW);
                bt_bd_addr_t aws_device;
                if (bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), &aws_device, 1)) {
                    /* if partner was attached in ull link, then send play IF, otherwise we should switch AWS link first */
                    if (!memcmp(&aws_device, &(ctx->bt_addr), BT_BD_ADDR_LEN)) {
                        bt_ull_aws_mce_eir_info_t eir_info = {0};
                        eir_info.event = BT_ULL_AWS_MCE_EVT_PLAY;
                        eir_info.param.aws_play.dl_streaming.volume.vol_left = ctx->dl_speaker.original_volume.vol_left;
                        eir_info.param.aws_play.dl_streaming.volume.vol_right = ctx->dl_speaker.original_volume.vol_right;
                        eir_info.param.aws_play.dl_streaming.is_mute = ctx->dl_speaker.is_mute;
                        eir_info.param.aws_play.dl_streaming.is_streaming = ctx->dl_speaker.is_streaming;

                        eir_info.param.aws_play.ul_streaming.volume.vol_left = ctx->ul_microphone.original_volume.vol_left;
                        eir_info.param.aws_play.ul_streaming.volume.vol_right = ctx->ul_microphone.original_volume.vol_right;
                        eir_info.param.aws_play.ul_streaming.is_mute = ctx->ul_microphone.is_mute;
                        eir_info.param.aws_play.ul_streaming.is_streaming = ctx->ul_microphone.is_streaming;

                        eir_info.param.aws_play.dl_latency = ctx->dl_latency;
                        eir_info.param.aws_play.ul_latency = ctx->ul_latency;
                        bt_ull_aws_mce_send_eir(&eir_info, sizeof(eir_info), true);
                        ull_report("[ULL][AWS] agent sync streaming start, vol_lev: %d, is_mute:0x%x, dl_latency: %d, ul_latency: %d", 4,
                                   ctx->dl_speaker.volume.vol_left, ctx->dl_speaker.is_mute, ctx->dl_latency, ctx->ul_latency);
                    } else {
                        bt_cm_connect_t conn_req;
                        memcpy(&(conn_req.address), &(ctx->bt_addr), sizeof(bt_bd_addr_t));
                        conn_req.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS);
                        bt_cm_connect(&conn_req);
                        ull_report("[ULL] switch aws link", 0);
                    }
                }
            }
#else
            bt_ull_notify_server_play_is_allow(BT_ULL_PLAY_ALLOW);
#endif
            /* lock dvfs for ull will be playing */
#ifdef AIR_BTA_IC_PREMIUM_G3
            bt_ull_dvfs_lock(HAL_DVFS_OPP_MID);
#else
            bt_ull_dvfs_lock(HAL_DVFS_FULL_SPEED_104M);
#endif

            /* init codec parameter */
            memset(&aud_cap, 0x00, sizeof(bt_sink_srv_am_audio_capability_t));
            aud_cap.type = A2DP;
            aud_cap.codec.a2dp_format.a2dp_codec.role = BT_A2DP_SINK;
            ull_report("[ULL]open codec type:0x%x", 1, ctx->codec_cap.type);
            memcpy(&(aud_cap.codec.a2dp_format.a2dp_codec.codec_cap), &(ctx->codec_cap), sizeof(bt_a2dp_codec_capability_t));
            aud_cap.audio_stream_out.audio_device = HAL_AUDIO_DEVICE_HEADSET;
            aud_cap.audio_stream_out.audio_volume = AUD_VOL_OUT_LEVEL15;    /* ull client using fix volume */
            ull_report("[ULL]open codec init vol is %d, is_mute:0x%x, latency: 0x%x", 3, aud_cap.audio_stream_out.audio_volume, ctx->dl_speaker.is_mute, ctx->dl_latency);
            aud_cap.audio_stream_out.audio_mute = false;
            aud_cap.codec.a2dp_format.a2dp_codec.a2dp_mtu = 0;  /* no used in ull codec */
            BT_ULL_SET_FLAG(ctx->flag, BT_ULL_FLAG_WAIT_AM_OPEN_CODEC);
            /* set latency */
            bt_sink_srv_ami_set_a2dp_sink_latency(ctx->dl_latency * 1000);

            /* open codec in client */
            am_ret = bt_sink_srv_ami_audio_play(ctx->audio_id, &aud_cap);
            if (AUD_EXECUTION_SUCCESS != am_ret) {
                ull_report_error("[ULL][am_callbak] bt_sink_srv_ami_audio_play fail: 0x%x, ", 1, am_ret);
                BT_ULL_REMOVE_FLAG(ctx->flag, BT_ULL_FLAG_WAIT_AM_OPEN_CODEC);
                /* Release audio source */
                bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_NONE);
                audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
            }
        } else if (BT_ULL_SUB_STATE_CLEAR_CODEC == substate) {
            /* Release audio source */
            audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        }
    }
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_am_suspend_ind(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL][am_callbak] audio src suspend ind--hd: 0x%08x, ctx->am_handle: 0x%08x, flag: 0x%x, int_hd->type: %d, ul_microphone.is_transmitter_start:0x%x", 5,
               handle, ctx->am_handle, ctx->flag, int_hd->type, ctx->ul_microphone.is_transmitter_start);
    BT_ULL_MUTEX_LOCK();
    if (ctx->am_handle && (ctx->am_handle == handle)) {

        bt_ull_am_ind_state_handle(BT_ULL_AM_SUSPEND_IND, false);
        bt_ull_am_sub_state_t substate = bt_ull_am_get_am_substate();
        /* waiting codec open, should release after codec open */
        if (BT_ULL_SUB_STATE_PREPARE_CODEC == substate) {
            bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_CLEAR_CODEC);
        } else if (BT_ULL_SUB_STATE_PREPARE_AUDIO_SRC == substate
            ||BT_ULL_SUB_STATE_NONE == substate) {
            if (ctx->ul_microphone.is_streaming) {
                /* stop transmitter */
                am_audio_side_tone_disable();
                bt_ull_stop_transmitter(&ctx->ul_microphone);
            }
            if (BT_ULL_AM_PLAYING == bt_ull_am_get_am_state()) {
                /* stop codec */
                bt_sink_srv_am_media_handle_t *med_hd = &(ctx->med_handle);
                bt_status_t ret = med_hd->stop(ctx->audio_id);
                ull_report("[ULL] codec stop--ret: 0x%x", 1, ret);
            }
            bt_sink_srv_ami_audio_stop(ctx->audio_id);

            bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_NONE);
            bt_ull_am_set_am_state(BT_ULL_AM_READY);
            audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_READY);
            bt_sink_srv_state_reset(BT_SINK_SRV_STATE_STREAMING);
        }
        ull_report("[ULL][am_callbak] audio src suspend ind, is downlink in streaming: 0x%x", 1, ctx->dl_speaker.is_streaming);
        if (ctx->dl_speaker.is_streaming || ctx->ul_microphone.is_streaming) {
            bt_ull_am_add_waiting_list();
        }
    }
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_am_stop_ind(audio_src_srv_handle_t *handle)
{
    bt_ull_context_t *ctx = bt_ull_get_context();

    ull_report("[ULL][am_callbak] audio src stop ind--hd: 0x%x, ctx->am_handle: 0x%x, flag: 0x%x, ul_microphone.is_transmitter_start:0x%x", 4,
               handle, ctx->am_handle, ctx->flag, ctx->ul_microphone.is_transmitter_start);
    BT_ULL_MUTEX_LOCK();
    if (ctx->am_handle && (ctx->am_handle == handle)) {
        bt_ull_am_ind_state_handle(BT_ULL_AM_STOP_IND, false);
        bt_ull_am_sub_state_t substate = bt_ull_am_get_am_substate();
        /* waiting codec open, should release after codec open */
        if (BT_ULL_SUB_STATE_PREPARE_CODEC == substate) {
            bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_CLEAR_CODEC);
        } else if (BT_ULL_SUB_STATE_NONE == substate
            ||BT_ULL_SUB_STATE_CLEAR_CODEC == substate) {
            /* stop transmitter & disable sidetone */
            am_audio_side_tone_disable();
            bt_ull_stop_transmitter(&ctx->ul_microphone);
            if (BT_ULL_AM_PLAYING == bt_ull_am_get_am_state()) {
                /* stop codec */
                bt_sink_srv_am_media_handle_t *med_hd = &(ctx->med_handle);
                bt_status_t ret = med_hd->stop(ctx->audio_id);
                ull_report("[ULL] codec stop--ret: 0x%x", 1, ret);
            }
            bt_sink_srv_ami_audio_stop(ctx->audio_id);

            bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_NONE);
            audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_READY);
            bt_ull_am_set_am_state(BT_ULL_AM_READY);
            bt_sink_srv_state_reset(BT_SINK_SRV_STATE_STREAMING);
            /* deinit pseudo device */
            if (ctx->flag & BT_ULL_FLAG_WAIT_AMI_STOP_CODEC) {
                BT_ULL_REMOVE_FLAG(ctx->flag, BT_ULL_FLAG_WAIT_AMI_STOP_CODEC);
                if (ctx->am_handle) {
                    audio_src_srv_destruct_handle(ctx->am_handle);
                    ctx->am_handle = NULL;
                    bt_ull_am_set_am_state(BT_ULL_AM_IDLE);
                }
                /* destruct the mic resource handle*/
                if (ctx->ul_microphone.resource_handle != NULL) {
                    if ((AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS == ctx->ul_microphone.resource_handle->state)
                        || (AUDIO_SRC_SRV_EVENT_SUSPEND == ctx->ul_microphone.resource_handle->state)) {
                        audio_src_srv_resource_manager_give(ctx->ul_microphone.resource_handle);
                    }
                    audio_src_srv_resource_manager_destruct_handle(ctx->ul_microphone.resource_handle);
                    ctx->ul_microphone.resource_handle = NULL;
                }
            } else if (ctx->flag & BT_ULL_FLAG_WAIT_AMI_RESTART_CODEC) {
                BT_ULL_REMOVE_FLAG(ctx->flag, BT_ULL_FLAG_WAIT_AMI_RESTART_CODEC);
                ull_report("[ULL] restart ull codec, current dl is_streaming: 0x%x", 1, ctx->dl_speaker.is_streaming);
                if (ctx->dl_speaker.is_streaming || ctx->ul_microphone.is_streaming) {
                    bt_ull_am_play();
                }
            } else {
                if (ctx->dl_speaker.is_streaming || ctx->ul_microphone.is_streaming) {
                    bt_ull_am_add_waiting_list();
                }
            }
        }
    }
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_am_reject_ind(audio_src_srv_handle_t *handle)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL][am_callbak] get audio src fail ind--hd: 0x%x, ctx->am_handle: 0x%x, flag: 0x%x, ul_microphone.is_transmitter_start:0x%x", 4,
               handle, ctx->am_handle, ctx->flag, ctx->ul_microphone.is_transmitter_start);
    BT_ULL_MUTEX_LOCK();
    if (ctx->am_handle && (ctx->am_handle == handle)) {
        if (ctx->ul_microphone.is_transmitter_start) {
            /* stop transmitter & disable sidetone */
            am_audio_side_tone_disable();
            bt_ull_stop_transmitter(&ctx->ul_microphone);
        }
        bt_ull_am_ind_state_handle(BT_ULL_AM_REJECT_IND, false);
        bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_NONE);
        if (ctx->dl_speaker.is_streaming || ctx->ul_microphone.is_streaming) {
            bt_ull_am_add_waiting_list();
        }
    }
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_am_exception_ind(audio_src_srv_handle_t *handle, int32_t event, void *param)
{
    ull_report("[ULL][am_callbak] exception_ind--hd: 0x%x, , event: 0x%x", 2,
               handle, event);
}

static void bt_ull_am_codec_open_ind(void *param)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_utils_assert(param);
    ull_report("[ULL] codec_open_ind, ctx->flag = 0x%x", 1, ctx->flag);
    BT_ULL_MUTEX_LOCK();
    bt_ull_am_sub_state_t substate = bt_ull_am_get_am_substate();
    if (BT_ULL_SUB_STATE_PREPARE_CODEC == substate
        || BT_ULL_SUB_STATE_NONE == substate) {
        if (ctx->flag & BT_ULL_FLAG_WAIT_AM_OPEN_CODEC) {
            BT_ULL_REMOVE_FLAG(ctx->flag, BT_ULL_FLAG_WAIT_AM_OPEN_CODEC);
            bt_ull_am_set_am_substate(BT_ULL_SUB_STATE_NONE);
            bt_ull_am_ind_state_handle(BT_ULL_AM_CODEC_OPEN_IND, true);
            /* Save codec handle */
            memcpy(&(ctx->med_handle), param, sizeof(bt_sink_srv_am_media_handle_t));
            /* Set codec volume */
            bt_sink_srv_ami_audio_set_volume(ctx->audio_id, AUD_VOL_OUT_LEVEL15, STREAM_OUT);
            /* codec play */
            bt_sink_srv_am_media_handle_t *med_handle = &(ctx->med_handle);
            bt_status_t ret = med_handle->play(ctx->audio_id);
            ull_report("[ULL] codec play--ret: 0x%x", 1, ret);
            if (BT_CODEC_MEDIA_STATUS_OK == ret) {
                /* update audio device */
                audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_PLAYING);
                bt_ull_am_set_am_state(BT_ULL_AM_PLAYING);
                bt_sink_srv_state_set(BT_SINK_SRV_STATE_STREAMING);
            } else {
                /* Release audio source */
                audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
            }
        }
    } else if (BT_ULL_SUB_STATE_CLEAR_CODEC == substate) {
        BT_ULL_REMOVE_FLAG(ctx->flag, BT_ULL_FLAG_WAIT_AM_OPEN_CODEC);
        /* Release audio source */
        audio_src_srv_update_state(ctx->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
    }
    BT_ULL_MUTEX_UNLOCK();
}

#ifdef AIR_BT_ULTRA_LOW_LATENCY_A2DP_STANDBY_ENABLE
static void bt_ull_am_delay_play_timeout_cb(uint32_t timer_id, uint32_t data)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL] bt_ull_am_delay_play_timeout_cb, speaker:0x%x, mic:0x%x", 2,
               ctx->dl_speaker.is_streaming, ctx->ul_microphone.is_streaming);

    if (ctx->dl_speaker.is_streaming || ctx->ul_microphone.is_streaming) {
        bt_ull_am_play();
    }
}
#endif
