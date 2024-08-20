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
#include "voice_prompt_aws.h"
#include "voice_prompt_queue.h"
#include "voice_prompt_nvdm.h"
#include "voice_prompt_aws.h"
#include "ui_realtime_task.h"
#include "bt_connection_manager.h"
#include "bt_sink_srv_utils.h"
#include "bt_app_common.h"
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
#include "bt_role_handover.h"
#endif
#include "bt_connection_manager_internal.h"
#include "bt_connection_manager.h"
#include "bt_gap.h"
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#include "bt_callback_manager.h"
#include "bt_device_manager.h"
#include "voice_prompt_local.h"
#include "hal_sleep_manager.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "hal_gpt.h"
#include "app_customer_common_activity.h"
#include "apps_config_event_list.h"
#include "apps_events_event_group.h"

#define LOG_TAG "VP_AWS"
#define VP_AWS_IF_TIMER_MS  (3 * 1000)

typedef struct {
    voice_prompt_aws_state_t aws_state;
    uint8_t slp_handle;                      /**<  Lock sleep handle. */
    bool slp_locked;                         /**<  Flag to indicate whether sleep is locked. */
    TimerHandle_t IF_timer;
    bool IF_timer_active;
} voice_prompt_aws_ctx_t;

static voice_prompt_aws_ctx_t g_voice_prompt_aws_ctx = {0};

static void voice_prompt_aws_sync_cb(bt_aws_mce_report_info_t *info);
static bt_status_t voice_prompt_aws_gap_evt_cb(bt_msg_type_t msg, bt_status_t status, void *buffer);
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
static void voice_prompt_aws_rho_cb(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status);
#endif

static void voice_prompt_aws_if_timer_callback(TimerHandle_t xtimer)
{
    //VP_LOG_MSGID_I(LOG_TAG" IF timer callback", 0);
    g_voice_prompt_aws_ctx.IF_timer_active = false;
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_IF_TIMEOUT, NULL);
}

static void voice_prompt_aws_if_timer_start()
{
    BaseType_t ret;

    if (g_voice_prompt_aws_ctx.IF_timer == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" IF timer start, not create", 0);
        return;
    }

    if (g_voice_prompt_aws_ctx.IF_timer_active) {
        ret = xTimerReset(g_voice_prompt_aws_ctx.IF_timer, 0);
    } else {
        g_voice_prompt_aws_ctx.IF_timer_active = true;
        ret = xTimerStart(g_voice_prompt_aws_ctx.IF_timer, 0);
    }
    VP_LOG_MSGID_I(LOG_TAG" IF timer restart ret %d", 1, ret);

    if (ret == pdFAIL) {
        g_voice_prompt_aws_ctx.IF_timer_active = false;
    }
}

static void voice_prompt_aws_if_timer_stop()
{
    BaseType_t ret;

    if (g_voice_prompt_aws_ctx.IF_timer == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" IF timer stop, not create", 0);
        return;
    }

    ret = xTimerStop(g_voice_prompt_aws_ctx.IF_timer, 0);
    VP_LOG_MSGID_I(LOG_TAG" IF timer stop ret %d", 1, ret);

    g_voice_prompt_aws_ctx.IF_timer_active = false;
}

void voice_prompt_aws_init()
{
    /* Set hal sleep handle, will lock sleep in sync play to ensure dsp get accurate gpt count. */
    g_voice_prompt_aws_ctx.slp_handle = hal_sleep_manager_set_sleep_handle("app_vp");
    g_voice_prompt_aws_ctx.slp_locked = false;


#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
    bt_role_handover_callbacks_t role_callbacks = {NULL, NULL, NULL, NULL, voice_prompt_aws_rho_cb};
#endif

    g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_SNIFF;

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
    bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_VP_APP, &role_callbacks);
#endif

    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_VP, voice_prompt_aws_sync_cb);
    bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_GAP, (void *)voice_prompt_aws_gap_evt_cb);

    g_voice_prompt_aws_ctx.IF_timer_active = false;
    g_voice_prompt_aws_ctx.IF_timer = xTimerCreate("vp_aws_timer", (VP_AWS_IF_TIMER_MS / portTICK_PERIOD_MS),
                                                   pdFALSE, NULL, voice_prompt_aws_if_timer_callback);

    if (g_voice_prompt_aws_ctx.IF_timer == NULL) {
        VP_LOG_MSGID_E(LOG_TAG" IF timer create fail", 0);
    }
}

void voice_prompt_aws_lock_sleep()
{
    if (!(g_voice_prompt_aws_ctx.slp_locked)) {
        VP_LOG_MSGID_I(LOG_TAG" lock sleep", 0);
        hal_sleep_manager_lock_sleep(g_voice_prompt_aws_ctx.slp_handle);
        g_voice_prompt_aws_ctx.slp_locked = true;
    }
}

void voice_prompt_aws_unlock_sleep()
{
    if (g_voice_prompt_aws_ctx.slp_locked) {
        VP_LOG_MSGID_I(LOG_TAG" unlock sleep", 0);
        hal_sleep_manager_unlock_sleep(g_voice_prompt_aws_ctx.slp_handle);
        g_voice_prompt_aws_ctx.slp_locked = false;
    }
}

voice_prompt_aws_state_t voice_prompt_aws_get_state()
{
    return g_voice_prompt_aws_ctx.aws_state;
}

static bool voice_prompt_aws_check_connect()
{
    if (BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
        return true;
    } else {
        return false;
    }
}

static bool voice_prompt_aws_check_switching()
{
    /* todo: BT new API. */
    return false;
}

static voice_prompt_status_t voice_prompt_aws_get_remote_addr(bt_bd_addr_t *addr)
{
    uint32_t num = 0;
    num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), addr, 1);
#if 0
    uint8_t *addr_log = (uint8_t *)addr;
    VP_LOG_MSGID_I(LOG_TAG" Get AWS SP addr num %d, %02X:%02X:%02X:%02X:%02X:%02X", 7, num, *(addr_log + 5), *(addr_log + 4),
                   *(addr_log + 3), *(addr_log + 2), *(addr_log + 1), *(addr_log + 0));
#endif

    if (num != 1) {
        return VP_STATUS_FAIL;
    } else {
        return VP_STATUS_SUCCESS;
    }
}

static bt_status_t voice_prompt_aws_exit_sniff_mode(bt_bd_addr_t remote_addr)
{
    bt_status_t status;
    bt_gap_connection_handle_t bt_handle = bt_cm_get_gap_handle(remote_addr);

    status = bt_gap_exit_sniff_mode(bt_handle);
    VP_LOG_MSGID_I(LOG_TAG" exit sniff mode status %x", 1, status);
    return status;
}

bt_status_t voice_prompt_aws_enable_sniff(bool enable)
{
    bt_bd_addr_t addr = {0};
    bt_gap_connection_handle_t conn_handle = 0;
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (voice_prompt_aws_get_remote_addr(&addr) != VP_STATUS_SUCCESS) {
        //VP_LOG_MSGID_I(LOG_TAG" Error! can't get sp addr,", 0);
        g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_ACTIVE;
        return BT_STATUS_FAIL;
    }
    conn_handle = bt_cm_get_gap_handle(addr);

    if (0 != conn_handle) {
        bt_gap_link_policy_setting_t setting;
        if (enable) {
            g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_SNIFF;
            setting.sniff_mode = BT_GAP_LINK_POLICY_ENABLE;
        } else {
            g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_ACTIVE;
            setting.sniff_mode = BT_GAP_LINK_POLICY_DISABLE;
        }
        ret = bt_gap_write_link_policy(conn_handle, &setting);
    } else {
        g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_ACTIVE;
        ret = BT_STATUS_FAIL;
    }

    if (ret != BT_STATUS_SUCCESS) {
        //VP_LOG_MSGID_I(LOG_TAG" handle 0x%x, enable(%d), bt sniff fail. ret: 0x%08x", 3, conn_handle, enable, ret);
    } else {
        VP_LOG_MSGID_I(LOG_TAG" handle 0x%x, enable(%d), bt sniff success.", 2, conn_handle, enable);
    }
    return ret;
}

voice_prompt_status_t voice_prompt_aws_exit_sniff()
{
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
    bt_bd_addr_t bd_addr = {0};
    bt_status_t status = BT_STATUS_SUCCESS;

    if (role != BT_AWS_MCE_ROLE_AGENT || !voice_prompt_aws_check_connect()) {
        //VP_LOG_MSGID_I(LOG_TAG" exit sniff wrong, role %d", 1, role);
        return VP_STATUS_FAIL;
    }

    if (g_voice_prompt_aws_ctx.aws_state != VOICE_PROMPT_AWS_STATE_SNIFF) {
        VP_LOG_MSGID_I(LOG_TAG" already exited, state %d", 1, g_voice_prompt_aws_ctx.aws_state);
        return VP_STATUS_SUCCESS;
    }

    /* always disable sniff first due to host will never return BT_CONNECTION_MANAGER_STATUS_STATE_ALREADY_EXIST */
    voice_prompt_aws_enable_sniff(false);
    if (voice_prompt_aws_get_remote_addr(&bd_addr) != BT_STATUS_SUCCESS) {
        //VP_LOG_MSGID_I(LOG_TAG" Error! can't get sp addr,", 0);
        status = BT_STATUS_FAIL;
    } else {
        status = voice_prompt_aws_exit_sniff_mode(bd_addr);
    }

    VP_LOG_MSGID_I(LOG_TAG" exiting sniff mode ret=0x%x", 1, status);
    if (status == BT_STATUS_SUCCESS || status == BT_STATUS_PENDING) {
        /* Waiting for exit sniff mode. */
        g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_SNIFF_EXITING;
        return VP_STATUS_SNIFF_EXITING;
    } else if (status == BT_CONNECTION_MANAGER_STATUS_STATE_ALREADY_EXIST) {
        /* Disable sniff mode while VP playing. */
        voice_prompt_aws_enable_sniff(false);
        return VP_STATUS_SUCCESS;
    } else {
        g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_ACTIVE;
        return VP_STATUS_SUCCESS;
    }
}

void voice_prompt_aws_exit_sniff_cnf(void *msg_data)
{
    voice_prompt_aws_sniff_change_t *data = (voice_prompt_aws_sniff_change_t *)msg_data;

    VP_LOG_MSGID_E(LOG_TAG" voice_prompt_aws_exit_sniff_cnf, cur_aws_state=%d", 1, g_voice_prompt_aws_ctx.aws_state);
    if (msg_data == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" exit_sniff_cnf msg data null", 0);
        return;
    }

    if (data->status != BT_STATUS_SUCCESS) {
        /* Exit sniff mode fail, treat as sniff, play anyway. */
        g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_SNIFF;
    } else {
        if (data->ind.sniff_status == BT_GAP_LINK_SNIFF_TYPE_ACTIVE) {
            /* already disabled when exit. */
            if (g_voice_prompt_aws_ctx.aws_state != VOICE_PROMPT_AWS_STATE_SNIFF_EXITING) {
                /* Disable sniff mode while vp is playing. */
                voice_prompt_aws_enable_sniff(false);
            }
        } else {
            g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_SNIFF;
        }
    }

}


static bt_status_t voice_prompt_aws_gap_evt_cb(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    switch (msg) {
        case BT_GAP_SNIFF_MODE_CHANGE_IND: {
            bt_gap_sniff_mode_changed_ind_t *ind = (bt_gap_sniff_mode_changed_ind_t *)buffer;
            VP_LOG_MSGID_I(LOG_TAG" sniff mode change ind status: 0x%x, bt_state: %d, sniff status: %d", 3,
                           status, g_voice_prompt_aws_ctx.aws_state, ind ? ind->sniff_status : 0xff);
            if (g_voice_prompt_aws_ctx.aws_state == VOICE_PROMPT_AWS_STATE_SNIFF_EXITING) {
                voice_prompt_sniff_change_t *data = (voice_prompt_sniff_change_t *)pvPortMalloc(sizeof(voice_prompt_sniff_change_t));
                if (data == NULL) {
                    VP_LOG_MSGID_E(LOG_TAG" sniff mode change ind status: data malloc fail", 0);
                    break;
                }
                data->status = status;
                memcpy(&(data->ind), ind, sizeof(bt_gap_sniff_mode_changed_ind_t));
                /* Send msg to ui_realtime task to implement aws_play. */
                ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_SNIFF_CHANGE, data);
            }
            break;
        }
        case BT_GAP_WRITE_LINK_POLICY_CNF: {
            bt_gap_write_link_policy_cnf_t *cfn = (bt_gap_write_link_policy_cnf_t *)buffer;
            VP_LOG_MSGID_I(LOG_TAG" sniff status change cnf, status: 0x%x, sniff mode: 0x%x", 2, status, cfn->sniff_mode);
            break;
        }
        default:
            break;
    }

    return BT_STATUS_SUCCESS;
}


#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
static void voice_prompt_aws_rho_cb(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status)
{
    VP_LOG_MSGID_I(LOG_TAG" voice_prompt_aws_rho_cb voice prompt rho, role: 0x%x, state: %d,event=0x%x,ocean_cnt=%d", 4, role, g_voice_prompt_aws_ctx.aws_state,event,ocean_cnt);

    if (BT_ROLE_HANDOVER_COMPLETE_IND == event && status == BT_STATUS_SUCCESS) {
        /* Update sniff status. */
        if (role == BT_AWS_MCE_ROLE_AGENT) {
            g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_SNIFF;
        } else if (role == BT_AWS_MCE_ROLE_PARTNER) {
            //g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_ACTIVE;
            g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_SNIFF;
			#if 0
	     if(ocean_cnt)
	     {
		uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t)); 

		*p_key_action = KEY_TRIGER_OCEAN_VP;
		
    		VP_LOG_MSGID_I(LOG_TAG" voice_prompt_aws_rho_cb when rho,if ocean_cnt=1 than (new agent earbud) resent key voice_prompt_aws_rho_cb ", 0);

		if (p_key_action)
		{
	        	ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_KEY, 0xFFFF, p_key_action, sizeof(uint16_t), NULL, 1000);
	  	}
	  	else
	  	{
	      		vPortFree(p_key_action);
	  	}		
	  }
		 #endif
        }
    }
}
#endif


voice_prompt_status_t voice_prompt_aws_sync_play(voice_prompt_param_t *vp, uint16_t delay, uint32_t *tar_gpt)
{
    bt_status_t bt_ret;
    bt_aws_mce_report_info_t info;
    uint8_t sync_ctx[sizeof(voice_prompt_aws_sync_param_t) + sizeof(voice_prompt_aws_sync_play_t)] = {0};
    voice_prompt_aws_sync_param_t *sync_param = (voice_prompt_aws_sync_param_t *)&sync_ctx;
    voice_prompt_aws_sync_play_t *sync_play = (voice_prompt_aws_sync_play_t *) & (sync_param->param);

    /*
    if (!voice_prompt_aws_check_connect() && !voice_prompt_aws_check_switching()) {
        //VP_LOG_MSGID_W( LOG_TAG" syn_play, AWS disconnected", 0);
        return VP_STATUS_AWS_DISCONNECT;
    }
    */

    sync_param->type = VOICE_PROMPT_SYNC_TYPE_PLAY;

    info.module_id = BT_AWS_MCE_REPORT_MODULE_VP;
    info.sync_time = vp->delay_time;
    info.param_len = sizeof(voice_prompt_aws_sync_param_t) + sizeof(voice_prompt_aws_sync_play_t);

    /* Lock sleep before get bt_clock and gpt_count to ensure accurate. */
    voice_prompt_aws_lock_sleep();

    /* Calculate target bt_clock to play.
     * Use delay to calculate play_clock, and send vp->delay_time to partner to store. */
    bt_ret = bt_sink_srv_bt_clock_addition(&(sync_play->play_clock), 0, delay * 1000);
    sync_play->delay_ms = delay;
    memcpy((void *) & (sync_play->vp), (void *)vp, sizeof(voice_prompt_param_t));

    info.param = (void *)&sync_ctx;

    VP_LOG_MSGID_I(LOG_TAG" sync_play vp_index %d, delay %d, bt_clk 0x%08x 0x%08x", 4,
                   vp->vp_index, delay, sync_play->play_clock.nclk, sync_play->play_clock.nclk_intra);

    bt_ret = bt_aws_mce_report_send_urgent_event(&info);
    if (bt_ret != BT_STATUS_SUCCESS) {
        VP_LOG_MSGID_I(LOG_TAG" sync_play, send aws report fail, ret 0x%08x", 1, bt_ret);
        *tar_gpt = 0;
        if (bt_ret == BT_STATUS_BUSY) {
            /* IF lock. */
            voice_prompt_aws_if_timer_start();
            return VP_STATUS_AWS_IF_LOCKED;
        } else {
            return VP_STATUS_AWS_DISCONNECT;
        }
    }

    /* Convert target bt_clock to target gpt_count. */
    if (bt_sink_srv_convert_bt_clock_2_gpt_count((const bt_clock_t *) & (sync_play->play_clock), tar_gpt) == BT_STATUS_SUCCESS) {
        VP_LOG_MSGID_I(LOG_TAG" sync_play, tar_gpt 0x%08x", 1, *tar_gpt);
    } else {
        //VP_LOG_MSGID_I(LOG_TAG" sync_play, bt_clk to gpt fail", 0);
        /* Convert fail due to AWS disconnected, play without delay. */
        *tar_gpt = 0;
    }

    return VP_STATUS_SUCCESS;
}

voice_prompt_status_t voice_prompt_aws_sync_stop(uint32_t vp_index, bool preempted, uint32_t *tar_gpt)
{
    bt_status_t bt_ret;
    bt_aws_mce_report_info_t info;
    uint8_t sync_ctx[sizeof(voice_prompt_aws_sync_param_t) + sizeof(voice_prompt_aws_sync_stop_t)] = {0};
    voice_prompt_aws_sync_param_t *sync_param = (voice_prompt_aws_sync_param_t *)&sync_ctx;
    voice_prompt_aws_sync_stop_t *sync_stop = (voice_prompt_aws_sync_stop_t *) & (sync_param->param);

    /*
    if (!voice_prompt_aws_check_connect() && !voice_prompt_aws_check_switching()) {
        //VP_LOG_MSGID_W( LOG_TAG" sync_stop, AWS disconnected", 0);
        return VP_STATUS_AWS_DISCONNECT;
    }
    */

    sync_param->type = VOICE_PROMPT_SYNC_TYPE_STOP;

    info.module_id = BT_AWS_MCE_REPORT_MODULE_VP;
    info.sync_time = VOICE_PROMPT_SYNC_DELAY_MIN;
    info.param_len = sizeof(voice_prompt_aws_sync_param_t) + sizeof(voice_prompt_aws_sync_stop_t);

    /* Lock sleep before get bt_clock and gpt_count to ensure accurate. */
    voice_prompt_aws_lock_sleep();

    /* Calculate target bt_clock to play. */
    bt_ret = bt_sink_srv_bt_clock_addition(&(sync_stop->stop_clock), 0, VOICE_PROMPT_SYNC_DELAY_MIN * 1000);
    sync_stop->vp_index = vp_index;
    sync_stop->preempted = preempted;

    info.param = (void *)&sync_ctx;

    VP_LOG_MSGID_I(LOG_TAG" sync_stop index %d, bt_clk 0x%08x 0x%08x", 3,
                   vp_index, sync_stop->stop_clock.nclk, sync_stop->stop_clock.nclk_intra);

    bt_ret = bt_aws_mce_report_send_urgent_event(&info);
    if (bt_ret != BT_STATUS_SUCCESS) {
        VP_LOG_MSGID_W(LOG_TAG" sync_stop, send aws report fail, ret %d", 1, bt_ret);
        *tar_gpt = 0;
        if (bt_ret == BT_STATUS_BUSY) {
            /* IF lock. */
            voice_prompt_aws_if_timer_start();
            return VP_STATUS_AWS_IF_LOCKED;
        } else {
            return VP_STATUS_FAIL;
        }
    }

    /* Convert target bt_clock to target gpt_count. */
    if (bt_sink_srv_convert_bt_clock_2_gpt_count((const bt_clock_t *) & (sync_stop->stop_clock), tar_gpt) == BT_STATUS_SUCCESS) {
        VP_LOG_MSGID_I(LOG_TAG" sync_stop, tar_gpt 0x%08x", 1, tar_gpt);
    } else {
        //VP_LOG_MSGID_W(LOG_TAG" sync_stop, bt_clk to gpt fail", 0);
        /* Convert fail due to AWS disconnected, play without delay. */
        *tar_gpt = 0;
    }

    return VP_STATUS_SUCCESS;
}

voice_prompt_status_t voice_prompt_aws_play_peer(voice_prompt_param_t *vp)
{
    bt_status_t bt_ret;
    bt_aws_mce_report_info_t info;
    uint8_t sync_ctx[sizeof(voice_prompt_aws_sync_param_t) + sizeof(voice_prompt_param_t)] = {0};
    voice_prompt_aws_sync_param_t *sync_param = (voice_prompt_aws_sync_param_t *)&sync_ctx;
    voice_prompt_param_t *sync_play = (voice_prompt_param_t *) & (sync_param->param);

    if (!voice_prompt_aws_check_connect() && !voice_prompt_aws_check_switching()) {
        VP_LOG_MSGID_W(LOG_TAG" play_peer, AWS disconnected", 0);
        return VP_STATUS_AWS_DISCONNECT;
    }

    sync_param->type = VOICE_PROMPT_SYNC_TYPE_PLAY_PEER;

    info.module_id = BT_AWS_MCE_REPORT_MODULE_VP;
    info.sync_time = VOICE_PROMPT_SYNC_DELAY_MIN;
    info.param_len = sizeof(voice_prompt_aws_sync_param_t) + sizeof(voice_prompt_param_t);

    memcpy((void *)sync_play, (void *)vp, sizeof(voice_prompt_param_t));
    info.param = (void *)&sync_ctx;

    bt_ret = bt_aws_mce_report_send_urgent_event(&info);
    VP_LOG_MSGID_I(LOG_TAG" play_peer vp_index %d, send aws report ret %d", 2, vp->vp_index, bt_ret);
    if (bt_ret != BT_STATUS_SUCCESS) {
#if 0
        if (bt_ret == BT_STATUS_BUSY) {
            /* IF lock. */
            voice_prompt_aws_if_timer_start();
            return VP_STATUS_AWS_IF_LOCKED;
        }
#endif
        return VP_STATUS_FAIL;

    }

    return VP_STATUS_SUCCESS;
}

voice_prompt_status_t voice_prompt_aws_stop_peer(uint32_t vp_index, bool sync)
{
    bt_status_t bt_ret;
    bt_aws_mce_report_info_t info;
    uint8_t sync_ctx[sizeof(voice_prompt_aws_sync_param_t) + sizeof(voice_prompt_aws_stop_peer_t)] = {0};
    voice_prompt_aws_sync_param_t *sync_param = (voice_prompt_aws_sync_param_t *)&sync_ctx;
    voice_prompt_aws_stop_peer_t *sync_stop = (voice_prompt_aws_stop_peer_t *) & (sync_param->param);

    if (!voice_prompt_aws_check_connect() && !voice_prompt_aws_check_switching()) {
        VP_LOG_MSGID_W(LOG_TAG" stop_peer, AWS disconnected", 0);
        return VP_STATUS_AWS_DISCONNECT;
    }

    sync_param->type = VOICE_PROMPT_SYNC_TYPE_STOP_PEER;

    info.module_id = BT_AWS_MCE_REPORT_MODULE_VP;
    info.sync_time = VOICE_PROMPT_SYNC_DELAY_MIN;
    info.param_len = sizeof(voice_prompt_aws_sync_param_t) + sizeof(voice_prompt_aws_stop_peer_t);

    sync_stop->vp_index = vp_index;
    sync_stop->sync = sync;
    info.param = (void *)&sync_ctx;

    bt_ret = bt_aws_mce_report_send_urgent_event(&info);
    VP_LOG_MSGID_I(LOG_TAG" stop_peer vp_index %d, send aws report ret %d", 2, vp_index, bt_ret);
    if (bt_ret != BT_STATUS_SUCCESS) {
#if 0
        if (bt_ret == BT_STATUS_BUSY) {
            /* IF lock. */
            return VP_STATUS_AWS_IF_LOCKED;
        }
#endif
        return VP_STATUS_FAIL;
    }

    return VP_STATUS_SUCCESS;
}

voice_prompt_status_t voice_prompt_aws_sync_language(uint8_t lang_idx, voice_prompt_lang_codec_t codec)
{
    bt_status_t bt_ret;
    bt_aws_mce_report_info_t info;
    uint8_t sync_ctx[sizeof(voice_prompt_aws_sync_param_t) + sizeof(voice_prompt_aws_sync_lang_t)] = {0};
    voice_prompt_aws_sync_param_t *sync_param = (voice_prompt_aws_sync_param_t *)&sync_ctx;
    voice_prompt_aws_sync_lang_t *sync_lang = (voice_prompt_aws_sync_lang_t *) & (sync_param->param);

    if (!voice_prompt_aws_check_connect() && !voice_prompt_aws_check_switching()) {
        VP_LOG_MSGID_W(LOG_TAG" sync_language, AWS disconnected", 0);
        return VP_STATUS_AWS_DISCONNECT;
    }

    sync_param->type = VOICE_PROMPT_SYNC_TYPE_LANG;

    info.module_id = BT_AWS_MCE_REPORT_MODULE_VP;
    info.sync_time = VOICE_PROMPT_SYNC_DELAY_MIN;
    info.param_len = sizeof(voice_prompt_aws_sync_param_t) + sizeof(voice_prompt_aws_sync_lang_t);

    sync_lang->lang_idx = lang_idx;
    sync_lang->codec = codec;
    info.param = (void *)&sync_ctx;

    bt_ret = bt_aws_mce_report_send_urgent_event(&info);
    VP_LOG_MSGID_I(LOG_TAG" sync_language lang_idx %d, codec %x, send aws report ret %d", 3, lang_idx, codec, bt_ret);
    if (bt_ret != BT_STATUS_SUCCESS) {
#if 0
        if (bt_ret == BT_STATUS_BUSY) {
            /* IF lock. */
            return VP_STATUS_AWS_IF_LOCKED;
        }
#endif
        return VP_STATUS_FAIL;
    }

    return VP_STATUS_SUCCESS;
}

static void voice_prompt_aws_sync_cb_play(void *sync_param)
{
    voice_prompt_aws_sync_play_t *param = (voice_prompt_aws_sync_play_t *)sync_param;
    uint32_t cur_gpt;

    VP_LOG_MSGID_I(LOG_TAG" sync_cb play vp_index %d, bt_clk (0x%08x 0x%08x)", 3,
                   param->vp.vp_index, param->play_clock.nclk, param->play_clock.nclk_intra);

    voice_prompt_param_t *vp = (voice_prompt_param_t *)pvPortMalloc(sizeof(voice_prompt_param_t));
    if (vp == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" sync_cb, play data malloc fail", 0);
        return;
    }
    memcpy((void *)vp, (void *) & (param->vp), sizeof(voice_prompt_param_t));
    vp->callback = NULL;

    /* Lock sleep before play sync VP to ensure DSP get the accurate gpt_count. */
    voice_prompt_aws_lock_sleep();

    /* Convert received target bt_clock to target gpt_count. */
    if (bt_sink_srv_convert_bt_clock_2_gpt_count((const bt_clock_t *) & (param->play_clock), &(vp->target_gpt)) == BT_STATUS_SUCCESS) {
        VP_LOG_MSGID_I(LOG_TAG" sync_cb, play tar_gpt 0x%08x", 1, vp->target_gpt);
    } else {
        //VP_LOG_MSGID_I(LOG_TAG" sync_cb, play bt_clk to gpt fail, count gpt by delay_ms %d", 1, param->delay_ms);
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &cur_gpt);
        vp->target_gpt = cur_gpt + param->delay_ms * 1000;
    }

    /* Send msg to ui_realtime task to play sync VP. */
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_SYNCED_PLAY, (void *)vp);
}

static void voice_prompt_aws_sync_cb_stop(void *sync_param)
{
    voice_prompt_aws_sync_stop_t *param = (voice_prompt_aws_sync_stop_t *)sync_param;
    VP_LOG_MSGID_I(LOG_TAG" sync_cb stop vp_index %d, bt_clk (0x%08x 0x%08x)", 3,
                   param->vp_index, param->stop_clock.nclk, param->stop_clock.nclk_intra);

    voice_prompt_synced_stop_t *stop_vp = (voice_prompt_synced_stop_t *)pvPortMalloc(sizeof(voice_prompt_synced_stop_t));
    if (stop_vp == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" sync_cb, stop data malloc fail", 0);
        return;
    }
    stop_vp->vp_index = param->vp_index;
    stop_vp->preempted = param->preempted;

    /* Lock sleep before play sync VP to ensure DSP get the accurate gpt_count. */
    voice_prompt_aws_lock_sleep();

    /* Convert received target bt_clock to target gpt_count. */
    if (bt_sink_srv_convert_bt_clock_2_gpt_count((const bt_clock_t *) & (param->stop_clock), &(stop_vp->tar_gpt)) == BT_STATUS_SUCCESS) {
        VP_LOG_MSGID_I(LOG_TAG" sync_cb, stop tar_gpt 0x%08x", 1, stop_vp->tar_gpt);
    } else {
        //VP_LOG_MSGID_I(LOG_TAG" sync_cb, stop bt_clk to gpt fail", 0);
        stop_vp->tar_gpt = 0;
    }

    /* Send msg to ui_realtime task to play sync VP. */
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_SYNCED_STOP, (void *)stop_vp);
}

static void voice_prompt_aws_sync_cb_lang(void *sync_param)
{
    voice_prompt_aws_sync_lang_t *param = (voice_prompt_aws_sync_lang_t *)sync_param;
    VP_LOG_MSGID_I(LOG_TAG" sync_cb lang_index %d, codec", 2, param->lang_idx, param->codec);

    voice_prompt_set_lang_t *lang = (voice_prompt_set_lang_t *)pvPortMalloc(sizeof(voice_prompt_set_lang_t));
    if (lang == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" sync_cb, lang data malloc fail", 0);
        return;
    }
    lang->lang_idx = param->lang_idx;
    lang->codec = param->codec;
    lang->sync = false;

    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_LANGUAGE_SET, (void *)lang);
}

static void voice_prompt_aws_sync_cb_play_peer(void *sync_param)
{
    voice_prompt_param_t *param = (voice_prompt_param_t *)sync_param;
    VP_LOG_MSGID_I(LOG_TAG" sync_cb play_peer index %d", 1, param->vp_index);

    voice_prompt_msg_set_vp_t *msg_data = (voice_prompt_msg_set_vp_t *)pvPortMalloc(sizeof(voice_prompt_msg_set_vp_t));
    if (msg_data) {
        memcpy((void *) & (msg_data->param), (void *)param, sizeof(voice_prompt_param_t));
        msg_data->play_id = VOICE_PROMPT_ID_INVALID;
    } else {
        //VP_LOG_MSGID_E(LOG_TAG" sync_cb play_peer malloc fail", 0);
        return;
    }

    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_SET_VP, (void *)msg_data);
}

static void voice_prompt_aws_sync_cb_stop_peer(void *sync_param)
{
    voice_prompt_aws_stop_peer_t *param = (voice_prompt_aws_stop_peer_t *)sync_param;
    VP_LOG_MSGID_I(LOG_TAG" sync_cb stop_peer index %d, sync %d", 2, param->vp_index, param->sync);

    voice_prompt_stop_t *msg_data = (voice_prompt_stop_t *)pvPortMalloc(sizeof(voice_prompt_stop_t));
    if (msg_data) {
        msg_data->on_peer = false;
        msg_data->play_id = VOICE_PROMPT_ID_INVALID;
        msg_data->sync = param->sync;
        msg_data->vp_index = param->vp_index;
    } else {
        //VP_LOG_MSGID_E(LOG_TAG" sync_cb stop_peer malloc fail", 0);
        return;
    }

    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_STOP_VP, (void *)msg_data);
}

typedef void (*voice_prompt_aws_func)(void *param);
static voice_prompt_aws_func vp_aws_funcs[] = {
    voice_prompt_aws_sync_cb_play,
    voice_prompt_aws_sync_cb_stop,
    voice_prompt_aws_sync_cb_lang,
    voice_prompt_aws_sync_cb_play_peer,
    voice_prompt_aws_sync_cb_stop_peer
};
static void voice_prompt_aws_sync_cb(bt_aws_mce_report_info_t *info)
{
    if (info == NULL || info->param == NULL) {
        //VP_LOG_MSGID_E(LOG_TAG" sync_cb, info or param NULL", 0);
        return;
    }

    bt_aws_mce_report_module_id_t owner = info->module_id;
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();

    if ((owner != BT_AWS_MCE_REPORT_MODULE_VP) || ((role != BT_AWS_MCE_ROLE_AGENT) && (role != BT_AWS_MCE_ROLE_PARTNER))) {
        VP_LOG_MSGID_E(LOG_TAG" sync_cb owner not correct: 0x%2x, role 0x%2x", 2, owner, role);
        return;
    }

    voice_prompt_aws_sync_param_t *sync_ctx = (voice_prompt_aws_sync_param_t *)info->param;

    if (sync_ctx->type <= VOICE_PROMPT_SYNC_TYPE_STOP_PEER) {
        vp_aws_funcs[sync_ctx->type](sync_ctx->param);
    }

    return;
}

void voice_prompt_aws_hdl_if_unlock()
{
    voice_prompt_aws_if_timer_stop();
    g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_SNIFF;
}

void voice_prompt_aws_hdl_remote_disconnect()
{
    g_voice_prompt_aws_ctx.aws_state = VOICE_PROMPT_AWS_STATE_SNIFF;
}
