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
 * File: voice_prompt_api.h
 *
 * Description: This file defines the interface of voice_prompt_api.c.
 *
 */

#ifndef VOICE_PROMPT_AWS_H
#define VOICE_PROMPT_AWS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "voice_prompt_api.h"
#include "voice_prompt_main.h"

#define VOICE_PROMPT_SYNC_DELAY_MIN  200   /* Minimum delay time required by vp sync. */


/**
 *  @brief This enum defines the BT sniff mode status in the module.
 */
typedef enum {
    VOICE_PROMPT_AWS_STATE_SNIFF,              /**<  BT in sniff mode, or the BT sniff mode has not been changed by the module. */
    VOICE_PROMPT_AWS_STATE_SNIFF_EXITING,      /**<  The BT sniff mode is exiting. */
    VOICE_PROMPT_AWS_STATE_ACTIVE,             /**<  The BT controller is active and not in sniff mode. */
    VOICE_PROMPT_AWS_STATE_SWITCHING,          /**<  AWS switching. */
} voice_prompt_aws_state_t;


typedef enum {
    VOICE_PROMPT_SYNC_TYPE_PLAY,        /**<  param voice_prompt_aws_sync_param_t. */
    VOICE_PROMPT_SYNC_TYPE_STOP,        /**<  param voice_prompt_aws_sync_stop_t. */
    VOICE_PROMPT_SYNC_TYPE_LANG,        /**<  param voice_prompt_aws_sync_lang_t. */
    VOICE_PROMPT_SYNC_TYPE_PLAY_PEER,   /**<  param voice_prompt_param_t. */
    VOICE_PROMPT_SYNC_TYPE_STOP_PEER,   /**<  param voice_prompt_aws_stop_peer_t. */
} voice_prompt_aws_sync_type_t;

typedef struct {
    voice_prompt_aws_sync_type_t type;
    uint8_t param[0];
} voice_prompt_aws_sync_param_t;

typedef struct {
    bt_clock_t play_clock;
    voice_prompt_param_t vp;
    uint16_t delay_ms;
} voice_prompt_aws_sync_play_t;

typedef struct {
    bt_clock_t stop_clock;
    uint32_t vp_index;
    bool preempted;
} voice_prompt_aws_sync_stop_t;

typedef struct {
    uint32_t vp_index;
    bool sync;
} voice_prompt_aws_stop_peer_t;

typedef struct {
    bt_status_t status;                     /**<  BT status. */
    bt_gap_sniff_mode_changed_ind_t ind;    /**<  Extra data of bt_gap ind. */
} voice_prompt_aws_sniff_change_t;

typedef struct {
    uint8_t lang_idx;  /* 0xFF means set by codec. */
    voice_prompt_lang_codec_t codec; /* VOICE_PROMPT_LANG_CODEC_INVALID means set by index. */
} voice_prompt_aws_sync_lang_t;

void voice_prompt_aws_init();
voice_prompt_aws_state_t voice_prompt_aws_get_state();

/* VP_STATUS_SNIFF_EXITING means should wait.
 * VP_STATUS_SUCCESS means should just sync.
 * VP_STATUS_FAIL means role wrong or AWS disconnected. */
voice_prompt_status_t voice_prompt_aws_exit_sniff();
void voice_prompt_aws_exit_sniff_cnf(void *msg_data);
bt_status_t voice_prompt_aws_enable_sniff(bool enable);

voice_prompt_status_t voice_prompt_aws_sync_play(voice_prompt_param_t *vp, uint16_t delay, uint32_t *tar_gpt);
voice_prompt_status_t voice_prompt_aws_sync_stop(uint32_t vp_index, bool preempted, uint32_t *tar_gpt);
voice_prompt_status_t voice_prompt_aws_play_peer(voice_prompt_param_t *vp);
voice_prompt_status_t voice_prompt_aws_stop_peer(uint32_t vp_index, bool sync);
voice_prompt_status_t voice_prompt_aws_sync_language(uint8_t lang_idx, voice_prompt_lang_codec_t codec);
void voice_prompt_aws_hdl_if_unlock();
void voice_prompt_aws_hdl_remote_disconnect();

void voice_prompt_aws_lock_sleep();
void voice_prompt_aws_unlock_sleep();

#ifdef __cplusplus
}
#endif

#endif /* VOICE_PROMPT_API_H */

