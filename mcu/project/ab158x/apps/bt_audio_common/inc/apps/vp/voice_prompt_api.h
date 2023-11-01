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

#ifndef VOICE_PROMPT_API_H
#define VOICE_PROMPT_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bt_aws_mce_srv.h"

#define VOICE_PROMPT_QUEUE_MAX            (10)
#define VOICE_PROMPT_ID_INVALID           (0x0000)
#define VOICE_PROMPT_ID_SYNCED            (0xFFFE)
#define VOICE_PROMPT_ID_MAX               (0xFFFD)

#define VOICE_PROMPT_VP_INDEX_INVALID                    0xFFFFFFFF
#define VOICE_PROMPT_VP_LEANGUAGE_INVALID                0xFF

typedef uint16_t voice_prompt_lang_codec_t;  /* Type of language codec. */
#define VOICE_PROMPT_LANG_CODEC_INVALID 0xFFFF
#define VOICE_PROMPT_LANG_CODEC_EN_US 0x409      /* Codec ID of EN_US. */
#define VOICE_PROMPT_LANG_CODEC_ZH_TW 0x404      /* Codec ID of ZH_TW. */
#define VOICE_PROMPT_LANG_CODEC_FR_FR 0x40C      /* Codec ID of FR_FR. */


typedef uint16_t voice_prompt_control_mask_t;
#define VOICE_PROMPT_CONTROL_MASK_NONE                   0x0000
/* Preempt the playing VP. If the playing VP not allow preempt, then it cannot preempt. */
#define VOICE_PROMPT_CONTROL_MASK_PREEMPT                0x0001
/* Not allow other VP preempt it. */
#define VOICE_PROMPT_CONTROL_MASK_NO_PREEMPTED           0x0002
/* This VP need to be sync play. */
#define VOICE_PROMPT_CONTROL_MASK_SYNC                   0x0004
/* If the VP is synced fail due to AWS disconnect, then don't play it.
 * Only works when VOICE_PROMPT_CONTROL_MASK_SYNC set.
 * Only for Agent. */
#define VOICE_PROMPT_CONTROL_MASK_SYNC_FAIL_NOT_PLAY     0x0008
/* If there's other VP playing when it is recved on Partner, then don't play it.
 * Only works when VOICE_PROMPT_CONTROL_MASK_SYNC set.
 * Only for Partner. */
#define VOICE_PROMPT_CONTROL_MASK_SYNCED_FAIL_NOT_PLAY   0x0010
/* Loop play. */
#define VOICE_PROMPT_CONTROL_MASK_LOOP                   0x0020
/* Clean up all the other VP and play this one. The previous and later coming VPs will not be played.
 * Should only use when power off or reboot. */
#define VOICE_PROMPT_CONTROL_MASK_CLEANUP                0x0040
/* When this VP is playing, other VPs will all be skipped. */
#define VOICE_PROMPT_CONTROL_MASK_SKIP_OTHER             0x0080
/* This VP need to be sync play which allow to be triggered on Partner.
 * This is for very special case when role switch happens suddenly */
#define VOICE_PROMPT_CONTROL_MASK_SYNC_MUST              0x0100


/* For sync incoming call VP. */
#define VOICE_PROMPT_CONTROL_LOOPRT         ( VOICE_PROMPT_CONTROL_MASK_PREEMPT | VOICE_PROMPT_CONTROL_MASK_NO_PREEMPTED  \
                                            | VOICE_PROMPT_CONTROL_MASK_SYNC | VOICE_PROMPT_CONTROL_MASK_SKIP_OTHER  \
                                            | VOICE_PROMPT_CONTROL_MASK_LOOP | VOICE_PROMPT_CONTROL_MASK_SYNCED_FAIL_NOT_PLAY)

/* For local incoming call VP. */
#define VOICE_PROMPT_CONTROL_LOCALRT         ( VOICE_PROMPT_CONTROL_MASK_PREEMPT | VOICE_PROMPT_CONTROL_MASK_NO_PREEMPTED  \
                                             | VOICE_PROMPT_CONTROL_MASK_SKIP_OTHER | VOICE_PROMPT_CONTROL_MASK_LOOP)

/* For sync but not loop ringtone. Ex seal check, user trigger. */
#define VOICE_PROMPT_CONTROL_SINGLERT       ( VOICE_PROMPT_CONTROL_MASK_PREEMPT | VOICE_PROMPT_CONTROL_MASK_NO_PREEMPTED  \
                                            | VOICE_PROMPT_CONTROL_MASK_SYNC | VOICE_PROMPT_CONTROL_MASK_SKIP_OTHER)

/* For power off VP. */
#define VOICE_PROMPT_CONTROL_POWEROFF       ( VOICE_PROMPT_CONTROL_MASK_PREEMPT | VOICE_PROMPT_CONTROL_MASK_CLEANUP )

/**
 *  @brief This enum defines the error code of VP module.
 */
typedef enum {
    VP_EVENT_PLAY_END = 0,    /**<  The playback is ended. */
    VP_EVENT_READY,          /**<  The VP item is in the head of the queue and ready to play. */
    VP_EVENT_START_PLAY,     /**<  The playback is started. */
    VP_EVENT_FAIL,           /**<  The playback is failed. */
    VP_EVENT_FILE_NO_FOUND,  /**<  The VP file is not found. */
    VP_EVENT_PREEMPTED,      /**<  The playback is preempted by higher priority VP. */
    VP_EVENT_SYNC_FAIL,      /**<  A sync VP sync fail due to AWS disconnect. */
    VP_EVENT_SYNC_STOP_FAIL, /**<  Stop VP sync fail due to AWS disconnect. */
} voice_prompt_event_t;

typedef enum {
    VP_STATUS_SUCCESS = 0,
    VP_STATUS_FAIL,
    VP_STATUS_INDEX_INVALID,
    VP_STATUS_CLEANED,
    VP_STATUS_PARAM_WRONG,
    VP_STATUS_ROLE_WRONG,
    VP_STATUS_FILE_NOT_FOUND,
    VP_STATUS_SNIFF_EXITING,
    VP_STATUS_AWS_DISCONNECT,
    VP_STATUS_AWS_IF_LOCEKD,
} voice_prompt_status_t;

/* User callback. */
typedef void (*voice_prompt_play_callback_t)(uint32_t index, voice_prompt_event_t err);

typedef struct {
    uint32_t                     vp_index;   /* VP index. refer to apps_config_vp_index_list.h. */
    voice_prompt_control_mask_t  control;    /* Control flags. */
    uint16_t                     delay_time; /* Delay time of sync VP. If a sync VP set with delay_ms 0, it will be delay for VOICE_PROMPT_SYNC_DELAY_MIN*/
    uint32_t                     target_gpt; /* Target play gpt. Should be 0 when VOICE_PROMPT_CONTROL_MASK_SYNC set. */
    voice_prompt_play_callback_t callback;   /* User callback. */
} voice_prompt_param_t;


/**
* @brief      This function is for user to play a VP.
* @param[in]  vp, VP param.
* @param[out] play_id.
* @return     VP_STATUS_SUCCESS when success.
*/
voice_prompt_status_t voice_prompt_play(voice_prompt_param_t *vp, uint16_t *play_id);

/**
* @brief      This function is for user to update control flags of the playing VP.
* @param[in]  play_id, the play_id of the target VP. It is returned by voice_prompt_play.
* @param[in]  vp, VP param.
* @return     VP_STATUS_SUCCESS when success.
*/
voice_prompt_status_t voice_prompt_update_control(uint16_t play_id, voice_prompt_param_t *vp);

/**
* @brief      This function is for Partner to send play cmd to Agent.
* @param[in]  target_role, usually is Agent.
* @param[in]  vp, VP param.
* @return     VP_STATUS_SUCCESS when success.
*/
voice_prompt_status_t voice_prompt_play_on_peer(bt_aws_mce_role_t target_role, voice_prompt_param_t *vp);

/**
* @brief      This function is for user to stop a VP.
* @param[in]  vp_index, the VP index want to stop.
* @param[in]  id, play_id returned by voice_prompt_play. Set to VOICE_PROMPT_ID_INVALID to stop all the vp_index VPs.
* @param[in]  sync, whether need to sync stop.
* @return     VP_STATUS_SUCCESS when success.
*/
voice_prompt_status_t voice_prompt_stop(uint32_t vp_index, uint16_t id, bool sync);

/**
* @brief      This function is for user to stop current playing VP and clear all the VPs in queue.
* @return     VP_STATUS_SUCCESS when success.
*/
voice_prompt_status_t voice_prompt_clear_all(void);

/**
* @brief      This function is for Partner to stop play on Agent.
* @param[in]  vp_index, VP index want to stop.
* @param[in]  sync, whether need to sync stop.
* @return     VP_STATUS_SUCCESS when success.
*/
voice_prompt_status_t voice_prompt_stop_on_peer(uint32_t vp_index, bool sync);

/**
* @brief      This function is for user to get the index of current playing VP.
* @return     VP index.
*/
uint32_t voice_prompt_get_current_index();

/**
* @brief      This function is for user to get the language setting.
* @return     Language index.
*/
uint8_t voice_prompt_get_current_language();

/**
* @brief      This function is for user to set the language setting.
* @param[in]  sync, whether need to sync the setting.
* @param[in]  lang_idx, language index.
* @return     VP_STATUS_SUCCESS when success.
*/
voice_prompt_status_t voice_prompt_set_language(uint8_t lang_idx, bool sync);

/**
* @brief      This function require set language for VP.
* @param[in]  langCodec, a language codec which want to set (voice_prompt_lang_codec_t), ref https://docs.microsoft.com/en-us/previous-versions/ms776294(v=vs.85).
* @param[in]  sync, it mean that VP need sync to partner at the same time.
* @return     If the operation completed successfully, return true, otherwise return false.
*/
voice_prompt_status_t voice_prompt_set_language_by_LCID(voice_prompt_lang_codec_t langCodec, bool sync);

/**
* @brief      This function is for user to get the supported language count.
* @return     count.
*/
uint16_t voice_prompt_get_support_language_count();

/**
* @brief      This function require get count which support.
* @param[in]  buffer_num, the number of codec it can store.
* @param[out] Buffer to storage language LICD.
* @return     If the operation completed successfully, return VP_STATUS_SUCCESS.
*/
voice_prompt_status_t voice_prompt_get_support_language(uint16_t *buffer, uint16_t buffer_num);

/* The follow apis is based on voice_prompt_play and it's for the vp which frequently used. */
voice_prompt_status_t voice_prompt_play_vp_press();
voice_prompt_status_t voice_prompt_play_sync_vp_press();
voice_prompt_status_t voice_prompt_play_vp_successed();
voice_prompt_status_t voice_prompt_play_sync_vp_successed();
voice_prompt_status_t voice_prompt_play_vp_failed();
voice_prompt_status_t voice_prompt_play_sync_vp_failed();

// richard for customer UI spec.
voice_prompt_status_t voice_prompt_play_vp_hearing_through();
voice_prompt_status_t voice_prompt_play_sync_vp_hearing_through();
voice_prompt_status_t voice_prompt_play_vp_anc_on();
voice_prompt_status_t voice_prompt_play_sync_vp_anc_on();
voice_prompt_status_t voice_prompt_play_vp_battery_fail();
voice_prompt_status_t voice_prompt_play_sync_vp_battery_fail();
voice_prompt_status_t voice_prompt_play_vp_volume_up();
voice_prompt_status_t voice_prompt_play_sync_vp_volume_up();
voice_prompt_status_t voice_prompt_play_vp_volume_down();
voice_prompt_status_t voice_prompt_play_sync_vp_volume_down();
voice_prompt_status_t voice_prompt_play_vp_speech_focus();
voice_prompt_status_t voice_prompt_play_sync_vp_speech_focus();

voice_prompt_status_t voice_prompt_play_vp_power_off(voice_prompt_control_mask_t mask);
voice_prompt_status_t voice_prompt_play_vp_power_on();

#ifdef __cplusplus
}
#endif

#endif /* VOICE_PROMPT_API_H */

