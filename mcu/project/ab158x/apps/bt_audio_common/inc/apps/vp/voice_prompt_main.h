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
 * File: app_voice_prompt.h
 *
 * Description: This file defines the interface of app_voice_prompt.c.
 *
 */

#ifndef VOICE_PROMPT_MAIN_H
#define VOICE_PROMPT_MAIN_H

#include "bt_sink_srv.h"
#include "bt_sink_srv_common.h"
#include "bt_gap.h"
#include "voice_prompt_api.h"

#ifdef __cplusplus
extern "C" {
#endif


#define VOICE_PROMPT_RT_DEFAULT_DELAY 600     /* Default loop ringtone delay time for the first playment. */

typedef enum {
    VIOCE_PROMPT_STAT_NONE,
    VIOCE_PROMPT_STAT_INIT,
    VIOCE_PROMPT_STAT_PLAYING,
    VIOCE_PROMPT_STAT_STOPING,
    VIOCE_PROMPT_STAT_SNIFF_EXITING,
    VIOCE_PROMPT_STAT_WAIT_IF_UNLOCK,
    //VIOCE_PROMPT_STAT_RECVED_PLAY, //partner
    //VIOCE_PROMPT_STAT_RECVED_STOP, //partner
} voice_prompt_state_t;


/**
 *  @brief This enum defines the event type of msgs sent to ui_realtime task.
 */
typedef uint16_t voice_prompt_msg_id_t;
/* MSGs sent when upper layer call the API. */
#define UI_REALTIME_MSG_SET_VP               0x0001     /**<  Notify upper layer set a new VP, param voice_prompt_msg_set_vp_t. */
#define UI_REALTIME_MSG_UPDATE_CONTROL       0x0002     /**<  Notify upper layer update control flag, param voice_prompt_msg_set_vp_t. */
#define UI_REALTIME_MSG_PLAY_PEER            0x0003     /**<  Notify upper layer set VP to peer, param voice_prompt_param_t. */
#define UI_REALTIME_MSG_STOP_VP              0x0004     /**<  Notify upper layer stop a VP. param voice_prompt_stop_t. */
#define UI_REALTIME_MSG_CLEAR_ALL            0x0005     /**<  Clear all the VP, no param. */
#define UI_REALTIME_MSG_LANGUAGE_SET         0x0006     /**<  Notify upper layer set VP language, param voice_prompt_set_lang_t. */

/* MSG sent by prompt_control callback. */
#define UI_REALTIME_MSG_PLAY_START           0x0007     /**<  Notify a voice prompt playing started. param null. */
#define UI_REALTIME_MSG_PLAY_END             0x0008     /**<  Notify a voice prompt playing stopped. param null. */

/* MSG sent during AWS sync. */
#define UI_REALTIME_MSG_SNIFF_CHANGE         0x0009     /**<  Notify sniff mode changed and need sync VP, param voice_prompt_sniff_change_t. */
#define UI_REALTIME_MSG_SYNCED_PLAY          0x000A     /**<  Notify received VP sync from agent and need play local VP. param voice_prompt_param_t */
#define UI_REALTIME_MSG_SYNCED_STOP          0x000B     /**<  Notify received VP sync stop from agent and need stop local VP. voice_prompt_synced_stop_t*/

#define UI_REALTIME_MSG_PROC_NEXT            0x000C     /**<  Notify to handle next item in the queue. param NULL. */

#define UI_REALTIME_MSG_IF_UNLOCK            0x000D     /**<  Notify that AWS switch success, param null. */
#define UI_REALTIME_MSG_IF_TIMEOUT           0x000E     /**<  Notify that AWS switch fail after IF unlock timeout. */
#define UI_REALTIME_MSG_REMOTE_DISCONNECT    0x000F     /**<  Notify that remote device has disconnect, param null. */

typedef struct {
    voice_prompt_param_t param;
    uint16_t play_id;
} voice_prompt_msg_set_vp_t;

typedef struct {
    uint16_t play_id;
    uint32_t vp_index;
    bool on_peer;
    bool sync;
} voice_prompt_stop_t;

typedef struct {
    uint8_t lang_idx;  /* 0xFF means set by codec. */
    voice_prompt_lang_codec_t codec; /* VOICE_PROMPT_LANG_CODEC_INVALID means set by index. */
    bool sync;
} voice_prompt_set_lang_t;

typedef struct {
    bt_status_t status;                     /**<  BT status. */
    bt_gap_sniff_mode_changed_ind_t ind;    /**<  Extra data of bt_gap ind. */
} voice_prompt_sniff_change_t;

typedef struct {
    uint32_t tar_gpt;
    uint32_t vp_index;
    bool preempted;
} voice_prompt_synced_stop_t;



#ifdef __cplusplus
}
#endif

#endif /* BT_SINK_APP_VOICE_PROMPT_H */

