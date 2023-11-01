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

#ifndef VOICE_PROMPT_QUEUE_H
#define VOICE_PROMPT_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "voice_prompt_api.h"

/**
 *  @brief This enum defines the list status.
 */
typedef enum {
    VP_QUEUE_SUCCESS,      /**<  0, success. */
    VP_QUEUE_FAIL,         /**<  1, fail. */
    VP_QUEUE_NOT_FOUND,    /**<  2, not found. */
    VP_QUEUE_FULL,         /**<  3, queue full. */
    VP_QUEUE_PARAM_WRONG,  /**<  3, param wrong. */
} voice_prompt_queue_status_t;

typedef enum {
    VP_ITEM_STATE_QUEUE,
    VP_ITEM_STATE_WAIT_SYNC_PLAY,
    VP_ITEM_STATE_WAIT_SYNC_LOOP,
    VP_ITEM_STATE_PLAYING,
    VP_ITEM_STATE_WAIT_SYNC_STOP,
    VP_ITEM_STATE_WAIT_SYNC_PREEMPT,
    VP_ITEM_STATE_STOPING,
} voice_prompt_item_state_t;

typedef struct {
    uint32_t                     vp_index;
    uint16_t                     id;             /**<  0 ~ VOICE_PROMPT_ID_MAX. */
    voice_prompt_control_mask_t  control;
    uint16_t                     delay_time;
    uint32_t                     target_gpt;
    bool                         sync_by_peer;
    voice_prompt_play_callback_t callback;
    voice_prompt_item_state_t    state;
} voice_prompt_list_item_t;


void voice_prompt_queue_init();
uint16_t voice_prompt_queue_generate_id();
uint8_t voice_prompt_queue_get_current_length();
voice_prompt_queue_status_t voice_prompt_queue_insert_before(voice_prompt_param_t *param, uint16_t id, bool sync_by_peer, uint8_t before_index);
voice_prompt_queue_status_t voice_prompt_queue_insert_after(voice_prompt_param_t *param, uint16_t id, bool sync_by_peer, uint8_t after_index);
voice_prompt_queue_status_t voice_prompt_queue_replace(voice_prompt_param_t *param, uint16_t id, bool sync_by_peer, uint8_t replace_idx);
voice_prompt_queue_status_t voice_prompt_queue_push(voice_prompt_param_t *param, uint16_t id, bool sync_by_peer);
voice_prompt_queue_status_t voice_prompt_queue_delete(uint8_t list_idx);
voice_prompt_queue_status_t voice_prompt_queue_delete_by_vp_index(uint32_t vp_index);
voice_prompt_queue_status_t voice_prompt_queue_delete_by_vp_index_expcur(uint32_t vp_index);
voice_prompt_queue_status_t voice_prompt_queue_delete_all();
voice_prompt_queue_status_t voice_prompt_queue_delete_excp(uint8_t list_idx);
voice_prompt_queue_status_t voice_prompt_queue_delete_by_id(uint16_t id);
voice_prompt_queue_status_t voice_prompt_queue_get_idx_by_id(uint16_t id, uint8_t *idx);
voice_prompt_list_item_t *voice_prompt_queue_get_item_by_id(uint16_t id);
voice_prompt_list_item_t *voice_prompt_queue_get_current_item();
voice_prompt_list_item_t *voice_prompt_queue_get_tail_item();

#endif /* VOICE_PROMPT_QUEUE_H */
