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

#include "voice_prompt_api.h"
#include "voice_prompt_main.h"
#include "voice_prompt_queue.h"
#include "voice_prompt_internal.h"
#include "voice_prompt_aws.h"
#include "voice_prompt_nvdm.h"
#include "ui_realtime_task.h"
#include "bt_device_manager.h"
#include "voice_prompt_local.h"
#include "apps_config_vp_index_list.h"
#include "FreeRTOS.h"
#ifdef AIR_VOICE_PROMPT_COSYS_ENABLE
#include "voice_prompt_cosys.h"
#endif

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
#include "app_hear_through_activity.h"
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#if defined(AIR_DCHS_MODE_SLAVE_ENABLE)
#include "mux_ll_uart_latch.h"
#endif
#include "apps_customer_config.h"
#include "app_bt_state_service.h"
#include "app_in_ear_idle_activity.h"

#define LOG_TAG   "VP_API"

#ifdef AIR_PROMPT_SOUND_ENABLE
static voice_prompt_status_t voice_prompt_check_param(voice_prompt_param_t *vp)
{
    if (VOICE_PROMPT_VP_INDEX_INVALID == vp->vp_index) {
        return VP_STATUS_PARAM_WRONG;
    }

    /*
    if ((vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC) && vp->delay_time == 0) {
        VP_LOG_MSGID_E( LOG_TAG" sync VP must set delay_time !", 0);
        return VP_STATUS_PARAM_WRONG;
    }
    */

    if (((vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC) || (vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC_MUST))
        && vp->target_gpt != 0) {
        VP_LOG_MSGID_E(LOG_TAG" sync VP gpt must be 0!", 0);
        return VP_STATUS_PARAM_WRONG;
    }

#ifdef VOICE_PROMPT_SYNC_ENABLE
    uint32_t role = bt_device_manager_aws_local_info_get_role();
    bt_cm_power_state_t bt_power_state = bt_cm_power_get_state();

    if (((vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC) || (vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC_MUST)) &&
        (role == BT_AWS_MCE_ROLE_NONE || bt_power_state != BT_CM_POWER_STATE_ON)) {
        /* Trigger local VP. */
        VP_LOG_MSGID_I(LOG_TAG" play, change to local", 0);
        vp->control = vp->control & (~VOICE_PROMPT_CONTROL_MASK_SYNC);
        //vp->delay_time = 0;
    } else if (role != BT_AWS_MCE_ROLE_AGENT && (vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC)) {
        /* Ignore on partner/client. */
        VP_LOG_MSGID_I(LOG_TAG" play, partner ignore sync VP", 0);
        return VP_STATUS_ROLE_WRONG;
    }

    if (((vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC) || (vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC_MUST))
        && vp->delay_time < VOICE_PROMPT_SYNC_DELAY_MIN) {
        //VP_LOG_MSGID_W(LOG_TAG" delay time too short %d, enlarge", 1, vp->delay_time);
        vp->delay_time = VOICE_PROMPT_SYNC_DELAY_MIN;
    }

    if (vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC_MUST) {
        VP_LOG_MSGID_I(LOG_TAG" sync must", 0);
        vp->control = vp->control | VOICE_PROMPT_CONTROL_MASK_SYNC;
    }
#else
    vp->control = vp->control & (~VOICE_PROMPT_CONTROL_MASK_SYNC);
    vp->control = vp->control & (~VOICE_PROMPT_CONTROL_MASK_SYNC_MUST);
#endif

    return VP_STATUS_SUCCESS;
}
#endif

voice_prompt_status_t voice_prompt_play(voice_prompt_param_t *vp, uint16_t *play_id)
{
bool visi;
#ifdef EASTECH_SPEC_VP
if(vp->vp_index == VP_INDEX_PAIRING||vp->vp_index == VP_INDEX_PAIRING_LOOP)
{
visi=app_bt_connection_service_get_current_status()->bt_visible;
       VP_LOG_MSGID_I(LOG_TAG" VP_INDEX_PAIRING inear play vp=%d ir_senser_in_ear_statu=%d,visi=%d",3,vp->vp_index,ir_senser_in_ear_statu,visi);
  if((ir_senser_in_ear_statu==1)&&visi)
  {
       VP_LOG_MSGID_I(LOG_TAG" VP_INDEX_PAIRING inear play vp=%d",1,vp->vp_index);
  }
  else
  {
    VP_LOG_MSGID_I(LOG_TAG" VP_INDEX_PAIRING out ear no need play vp=%d",1,vp->vp_index);
    return VP_STATUS_SUCCESS;
  }
}
#else

#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE)
#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
    if (dchs_get_device_mode() != DCHS_MODE_SINGLE) {
        bool ret = voice_prompt_cosys_remote_play(vp, play_id);
        return (ret ? VP_STATUS_SUCCESS : VP_STATUS_FAIL);
    }
#else
    bool ret = voice_prompt_cosys_remote_play(vp, play_id);
    return (ret ? VP_STATUS_SUCCESS : VP_STATUS_FAIL);
#endif
#endif /* #if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE) */

    voice_prompt_msg_set_vp_t *msg_data = NULL;

#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return VP_STATUS_FAIL;
#else

    voice_prompt_status_t status = voice_prompt_check_param(vp);
    if (status != VP_STATUS_SUCCESS) {
        return status;
    }

    msg_data = (voice_prompt_msg_set_vp_t *)pvPortMalloc(sizeof(voice_prompt_msg_set_vp_t));
    if (msg_data) {
        memcpy((void *) & (msg_data->param), (void *)vp, sizeof(voice_prompt_param_t));
        msg_data->play_id = voice_prompt_queue_generate_id();
    } else {
        VP_LOG_MSGID_E(LOG_TAG" play malloc fail", 0);
        return VP_STATUS_FAIL;
    }

    VP_LOG_MSGID_I(LOG_TAG" play index %d, id %d, control 0x%08x", 3, vp->vp_index, msg_data->play_id, vp->control);
    if (play_id) {
        *play_id = msg_data->play_id;
    }
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_SET_VP, (void *)msg_data);
    return VP_STATUS_SUCCESS;

#endif
}

voice_prompt_status_t voice_prompt_update_control(uint16_t play_id, voice_prompt_param_t *vp)
{
    voice_prompt_msg_set_vp_t *msg_data = NULL;

#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return VP_STATUS_FAIL;
#else
    voice_prompt_status_t status = voice_prompt_check_param(vp);
    if (status != VP_STATUS_SUCCESS) {
        return status;
    }

    msg_data = (voice_prompt_msg_set_vp_t *)pvPortMalloc(sizeof(voice_prompt_msg_set_vp_t));
    if (msg_data) {
        memcpy((void *) & (msg_data->param), (void *)vp, sizeof(voice_prompt_param_t));
        msg_data->play_id = play_id;
    } else {
        VP_LOG_MSGID_E(LOG_TAG" update malloc fail", 0);
        return VP_STATUS_FAIL;
    }

    VP_LOG_MSGID_I(LOG_TAG" update play index %d, id %d", 2, vp->vp_index, msg_data->play_id);
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_UPDATE_CONTROL, (void *)msg_data);
    return VP_STATUS_SUCCESS;
#endif
}

voice_prompt_status_t voice_prompt_play_on_peer(bt_aws_mce_role_t target_role, voice_prompt_param_t *vp)
{
#if (defined VOICE_PROMPT_SYNC_ENABLE) && (defined AIR_PROMPT_SOUND_ENABLE)
    voice_prompt_param_t *msg_data = NULL;

    if (VOICE_PROMPT_VP_INDEX_INVALID == vp->vp_index) {
        //VP_LOG_MSGID_E(LOG_TAG" play_on_peer VP index invalid!", 0);
        return VP_STATUS_PARAM_WRONG;
    }

    if (vp->callback != NULL) {
        VP_LOG_MSGID_E(LOG_TAG" play_on_peer cannot sync callback!", 0);
        return VP_STATUS_PARAM_WRONG;
    }

    uint32_t role = bt_device_manager_aws_local_info_get_role();
    bt_cm_power_state_t bt_power_state = bt_cm_power_get_state();
    if (role != BT_AWS_MCE_ROLE_PARTNER || bt_power_state != BT_CM_POWER_STATE_ON) {
        VP_LOG_MSGID_E(LOG_TAG" play_on_peer error role 0x%x, bt_power 0x%x", 2, role, bt_power_state);
        return VP_STATUS_ROLE_WRONG;
    }

    if (((vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC) || (vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC_MUST))
        && vp->delay_time < VOICE_PROMPT_SYNC_DELAY_MIN) {
        //VP_LOG_MSGID_W(LOG_TAG" delay time too short %d, enlarge", 1, vp->delay_time);
        vp->delay_time = VOICE_PROMPT_SYNC_DELAY_MIN;
    }

    if (vp->target_gpt != 0) {
        VP_LOG_MSGID_E(LOG_TAG" play_on_peer not support gpt", 0);
        return VP_STATUS_PARAM_WRONG;
    }

    if (vp->control & VOICE_PROMPT_CONTROL_MASK_SYNC_MUST) {
        vp->control = vp->control | VOICE_PROMPT_CONTROL_MASK_SYNC;
    }

    msg_data = (voice_prompt_param_t *)pvPortMalloc(sizeof(voice_prompt_param_t));
    if (msg_data) {
        memcpy((void *)msg_data, (void *)vp, sizeof(voice_prompt_param_t));
    } else {
        VP_LOG_MSGID_E(LOG_TAG" play_on_peer malloc fail", 0);
        return VP_STATUS_FAIL;
    }

    VP_LOG_MSGID_I(LOG_TAG" play_on_peer index %d", 1, vp->vp_index);
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_PLAY_PEER, (void *)msg_data);
    return VP_STATUS_SUCCESS;

#else
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return VP_STATUS_FAIL;
#endif
}

voice_prompt_status_t voice_prompt_stop(uint32_t vp_index, uint16_t id, bool sync)
{
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE)
#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
    if (dchs_get_device_mode() != DCHS_MODE_SINGLE) {
        bool ret = voice_prompt_cosys_remote_stop(vp_index, id);
        return (ret ? VP_STATUS_SUCCESS : VP_STATUS_FAIL);
    }
#else
    bool ret = voice_prompt_cosys_remote_stop(vp_index, id);
    return (ret ? VP_STATUS_SUCCESS : VP_STATUS_FAIL);
#endif
#endif /* #if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE) */

    voice_prompt_stop_t *msg_data = NULL;

#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return VP_STATUS_FAIL;
#else

    if (VOICE_PROMPT_VP_INDEX_INVALID == vp_index) {
        //VP_LOG_MSGID_E(LOG_TAG" stop VP index invalid!", 0);
        return VP_STATUS_PARAM_WRONG;
    }

#ifdef VOICE_PROMPT_SYNC_ENABLE
    uint32_t role = bt_device_manager_aws_local_info_get_role();
    bt_cm_power_state_t bt_power_state = bt_cm_power_get_state();
    if (sync && (role == BT_AWS_MCE_ROLE_NONE || bt_power_state != BT_CM_POWER_STATE_ON)) {
        VP_LOG_MSGID_W(LOG_TAG" stop sync error role 0x%x, bt_power 0x%x", 2, role, bt_power_state);
        sync = false;
    } else if (sync && role != BT_AWS_MCE_ROLE_AGENT) {
        /* Ignore on partner/client. */
        VP_LOG_MSGID_I(LOG_TAG" stop, partner can't sync stop", 0);
        return VP_STATUS_ROLE_WRONG;
    }
#endif

    msg_data = (voice_prompt_stop_t *)pvPortMalloc(sizeof(voice_prompt_stop_t));
    if (msg_data) {
        msg_data->on_peer = false;
        msg_data->play_id = id;
        msg_data->vp_index = vp_index;
        msg_data->sync = sync;
    } else {
        VP_LOG_MSGID_E(LOG_TAG" stop malloc fail", 0);
        return VP_STATUS_FAIL;
    }

    VP_LOG_MSGID_I(LOG_TAG" stop index %d, id %d, sync %d", 3, vp_index, id, sync);
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_STOP_VP, (void *)msg_data);
    return VP_STATUS_SUCCESS;
#endif
}

voice_prompt_status_t voice_prompt_clear_all(void)
{
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE)
#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
    if (dchs_get_device_mode() != DCHS_MODE_SINGLE) {
        bool ret = voice_prompt_cosys_remote_clear_all();
        return (ret ? VP_STATUS_SUCCESS : VP_STATUS_FAIL);
    }
#else
    bool ret = voice_prompt_cosys_remote_clear_all();
    return (ret ? VP_STATUS_SUCCESS : VP_STATUS_FAIL);
#endif
#endif

#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return VP_STATUS_FAIL;
#else
    VP_LOG_MSGID_I(LOG_TAG" clear all", 0);
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_CLEAR_ALL, NULL);
    return VP_STATUS_SUCCESS;
#endif
}

voice_prompt_status_t voice_prompt_stop_on_peer(uint32_t vp_index, bool sync)
{
#if (defined VOICE_PROMPT_SYNC_ENABLE) && (defined AIR_PROMPT_SOUND_ENABLE)
    voice_prompt_stop_t *msg_data = NULL;

    if (VOICE_PROMPT_VP_INDEX_INVALID == vp_index) {
        //VP_LOG_MSGID_E(LOG_TAG" stop_on_peer VP index invalid!", 0);
        return VP_STATUS_PARAM_WRONG;
    }

    uint32_t role = bt_device_manager_aws_local_info_get_role();
    bt_cm_power_state_t bt_power_state = bt_cm_power_get_state();
    if (role != BT_AWS_MCE_ROLE_PARTNER || bt_power_state != BT_CM_POWER_STATE_ON) {
        VP_LOG_MSGID_E(LOG_TAG" stop_on_peer error role 0x%x, bt_power 0x%x", 2, role, bt_power_state);
        return VP_STATUS_ROLE_WRONG;
    }

    msg_data = (voice_prompt_stop_t *)pvPortMalloc(sizeof(voice_prompt_stop_t));
    if (msg_data) {
        msg_data->on_peer = true;
        msg_data->play_id = VOICE_PROMPT_ID_INVALID;
        msg_data->vp_index = vp_index;
        msg_data->sync = sync;
    } else {
        VP_LOG_MSGID_E(LOG_TAG" stop_on_peer malloc fail", 0);
        return VP_STATUS_FAIL;
    }

    VP_LOG_MSGID_I(LOG_TAG" stop_on_peer index %d, sync %d", 2, vp_index, sync);
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_STOP_VP, (void *)msg_data);
    return VP_STATUS_SUCCESS;
#else
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return VP_STATUS_FAIL;
#endif
}


uint32_t voice_prompt_get_current_index()
{
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE)
#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
    if (dchs_get_device_mode() != DCHS_MODE_SINGLE) {
        return voice_prompt_cosys_get_cur_index();
    }
#else
    return voice_prompt_cosys_get_cur_index();
#endif
#endif

    uint32_t index = VOICE_PROMPT_VP_INDEX_INVALID;
#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return index;
#else
    voice_prompt_list_item_t *item = voice_prompt_queue_get_current_item();

    if (item == NULL) {
        return index;
    }

    if (item->id != VOICE_PROMPT_ID_INVALID) {
        index = item->vp_index;
    }
    return index;
#endif
}

uint8_t voice_prompt_get_current_language()
{
    uint8_t index = VOICE_PROMPT_VP_LANGUAGE_INVALID;

#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return index;
#else

    voice_prompt_nvdm_get_current_lang(&index);
    return index;
#endif
}

static voice_prompt_status_t voice_prompt_set_language_internal(uint8_t lang_idx, voice_prompt_lang_codec_t codec, bool sync)
{
    voice_prompt_set_lang_t *msg_data = NULL;
#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return VP_STATUS_FAIL;
#else

#ifdef VOICE_PROMPT_SYNC_ENABLE
    if (sync && BT_AWS_MCE_SRV_LINK_NONE == bt_aws_mce_srv_get_link_type()) {
        //VP_LOG_MSGID_E(LOG_TAG" set_lang sync fail, aws disconnect", 0);
        return VP_STATUS_FAIL;
    }
#else
    sync = FALSE;
#endif

    msg_data = (voice_prompt_set_lang_t *)pvPortMalloc(sizeof(voice_prompt_set_lang_t));
    if (msg_data) {
        msg_data->lang_idx = lang_idx;
        msg_data->codec = codec;
        msg_data->sync = sync;
    } else {
        VP_LOG_MSGID_E(LOG_TAG" set_lang malloc fail", 0);
        return VP_STATUS_FAIL;
    }

    VP_LOG_MSGID_I(LOG_TAG" set_lang index %d, codec 0x%04x sync %d", 3, lang_idx, codec, sync);
    ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_LANGUAGE_SET, (void *)msg_data);
    return VP_STATUS_SUCCESS;
#endif
}


voice_prompt_status_t voice_prompt_set_language(uint8_t lang_idx, bool sync)
{
    return voice_prompt_set_language_internal(lang_idx, VOICE_PROMPT_LANG_CODEC_INVALID, sync);
}

voice_prompt_status_t voice_prompt_set_language_by_LCID(voice_prompt_lang_codec_t codec, bool sync)
{
    return voice_prompt_set_language_internal(VOICE_PROMPT_VP_LANGUAGE_INVALID, codec, sync);
}

uint16_t voice_prompt_get_support_language_count()
{
    uint16_t count = 0;
#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return count;
#else

    voice_prompt_nvdm_get_lang_cnt(&count);
    return count;
#endif
}

voice_prompt_status_t voice_prompt_get_support_language(uint16_t *buffer, uint16_t buffer_num)
{
#ifndef AIR_PROMPT_SOUND_ENABLE
    //VP_LOG_MSGID_E(LOG_TAG" feature option not enable", 0);
    return VP_STATUS_FAIL;
#else
    if (buffer == NULL || voice_prompt_get_support_language_count() > buffer_num) {
        return VP_STATUS_PARAM_WRONG;
    } else {
        if (voice_prompt_nvdm_get_support_lang(buffer)) {
            return VP_STATUS_SUCCESS;
        } else {
            return VP_STATUS_FAIL;
        }
    }
#endif
}

#if 0
static voice_prompt_status_t voice_prompt_play_vp_x(uint32_t vp_index)
{
    voice_prompt_param_t vp = {0};
    vp.vp_index = vp_index;
    if (vp_index==VP_INDEX_HEARING_AID_AEA_OFF||vp_index==VP_INDEX_HEARING_THROUGH||vp_index==VP_INDEX_ANC_ON)
    {
      vp.delay_time = 200;
    }else
      {
      vp.delay_time = 0;
      }
    return voice_prompt_play(&vp, NULL);
}

static voice_prompt_status_t voice_prompt_play_sync_vp_x(uint32_t vp_index)
{
    voice_prompt_param_t vp = {0};
    vp.vp_index = vp_index;
    vp.control = VOICE_PROMPT_CONTROL_MASK_SYNC;
    if(VP_INDEX_ANC_ON==VP_INDEX_ANC_ON)
      {
    vp.delay_time = VOICE_PROMPT_SYNC_DELAY_MIN+200;
      }
    else
      {
    vp.delay_time = VOICE_PROMPT_SYNC_DELAY_MIN;
      }
    return voice_prompt_play(&vp, NULL);
}
#else
static voice_prompt_status_t voice_prompt_play_vp_x(uint32_t vp_index)
{
    voice_prompt_param_t vp = {0};
    vp.vp_index = vp_index;
    return voice_prompt_play(&vp, NULL);
}

static voice_prompt_status_t voice_prompt_play_sync_vp_x(uint32_t vp_index)
{
    voice_prompt_param_t vp = {0};
    vp.vp_index = vp_index;
    vp.control = VOICE_PROMPT_CONTROL_MASK_SYNC;
    vp.delay_time = VOICE_PROMPT_SYNC_DELAY_MIN;
    return voice_prompt_play(&vp, NULL);
}
#endif

voice_prompt_status_t voice_prompt_play_vp_press()
{
    return voice_prompt_play_vp_x(VP_INDEX_PRESS);
}

voice_prompt_status_t voice_prompt_play_sync_vp_press()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_PRESS);
}

voice_prompt_status_t voice_prompt_play_vp_succeed()
{
    return voice_prompt_play_vp_x(VP_INDEX_SUCCEED);
}

voice_prompt_status_t voice_prompt_play_sync_vp_succeed()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_SUCCEED);
}

voice_prompt_status_t voice_prompt_play_sync_vp_mute()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_MUTE);
}

voice_prompt_status_t voice_prompt_play_vp_failed()
{
    return voice_prompt_play_vp_x(VP_INDEX_FAILED);
}

voice_prompt_status_t voice_prompt_play_sync_vp_failed()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_FAILED);
}

// richard for customer UI spec.
voice_prompt_status_t voice_prompt_play_vp_hearing_through()
{
    return voice_prompt_play_vp_x(VP_INDEX_AWARE);
}
voice_prompt_status_t voice_prompt_play_sync_vp_hearing_through()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_AWARE);
}

#if 0	// for production test
voice_prompt_status_t voice_prompt_play_sync_vp_ull_volume_up()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_HEARING_AID_MODE_5);
}

voice_prompt_status_t voice_prompt_play_sync_vp_ull_volume_down()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_HEARING_AID_MODE_2);
}
voice_prompt_status_t voice_prompt_play_sync_vp_ull_mbutton()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_POWER_ON);
}
#endif
voice_prompt_status_t voice_prompt_play_sync_vp_ha(uint8_t ha_mode_value)
{
	if(ha_mode_value==0)
	{
		return voice_prompt_play_sync_vp_x(VP_INDEX_AWARE);
	}
	else if(ha_mode_value==1)
	{
		return voice_prompt_play_sync_vp_x(VP_INDEX_SPEECH);	
	}
	else if(ha_mode_value==2)
	{
		return voice_prompt_play_sync_vp_x(VP_INDEX_COMFORT);
	}
	return VP_STATUS_SUCCESS;
}

voice_prompt_status_t voice_prompt_play_vp_anc_on()
{
    return voice_prompt_play_vp_x(VP_INDEX_ANC_ON);
}
voice_prompt_status_t voice_prompt_play_sync_vp_anc_on()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_ANC_ON);
}


voice_prompt_status_t voice_prompt_play_vp_anc_off()
{
    return voice_prompt_play_vp_x(VP_INDEX_HEARING_AID_AEA_OFF);
}
voice_prompt_status_t voice_prompt_play_sync_vp_anc_off()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_HEARING_AID_AEA_OFF);
}
voice_prompt_status_t voice_prompt_play_vp_battery_fail()
{
    return voice_prompt_play_vp_x(VP_INDEX_BATTERY_FAIL);
}
voice_prompt_status_t voice_prompt_play_sync_vp_battery_fail()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_BATTERY_FAIL);
}

voice_prompt_status_t voice_prompt_play_vp_volume_up()
{
    return voice_prompt_play_vp_x(VP_INDEX_VOLUME_UP);
}
voice_prompt_status_t voice_prompt_play_sync_vp_volume_up()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_VOLUME_UP);
}

voice_prompt_status_t voice_prompt_play_vp_volume_down()
{
    return voice_prompt_play_vp_x(VP_INDEX_VOLUME_DOWN);
}
voice_prompt_status_t voice_prompt_play_sync_vp_volume_down()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_VOLUME_DOWN);
}

voice_prompt_status_t voice_prompt_play_vp_speech_focus()
{
    return voice_prompt_play_vp_x(VP_INDEX_SPEECH);
}
voice_prompt_status_t voice_prompt_play_sync_vp_speech_focus()
{
    return voice_prompt_play_sync_vp_x(VP_INDEX_SPEECH);
}

voice_prompt_status_t voice_prompt_play_vp_power_off(voice_prompt_control_mask_t mask)
{
    voice_prompt_param_t vp = {0};
    vp.vp_index = VP_INDEX_POWER_OFF;
    vp.control = mask;
    if (mask & VOICE_PROMPT_CONTROL_MASK_SYNC) {
        vp.delay_time = VOICE_PROMPT_SYNC_DELAY_MIN;
    }
    return voice_prompt_play(&vp, NULL);
}

extern uint8_t prompt_no_play_flag;	// richard for UI
voice_prompt_status_t voice_prompt_play_vp_power_on()
{
//	if(prompt_no_play_flag) return 0;   // harry mask 0708
	
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    app_hear_through_activity_power_on_vp_start_to_play();
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

    voice_prompt_param_t vp = {0};
    vp.vp_index = VP_INDEX_POWER_ON;
    vp.control = VOICE_PROMPT_CONTROL_MASK_PREEMPT;
    return voice_prompt_play(&vp, NULL);
}

