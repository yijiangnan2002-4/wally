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
#ifndef __BT_ULL_AUDIO_MANAGER_H__
#define __BT_ULL_AUDIO_MANAGER_H__

#include "bt_type.h"
#include "bt_sink_srv_ami.h"
#include "bt_codec.h"
#include "bt_avm.h"

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_control.h"
#include "audio_transmitter_control_port.h"
#endif

#define BT_ULL_FLAG_WAIT_START 0x01
#define BT_ULL_FLAG_WAIT_AM_OPEN_CODEC          ((BT_ULL_FLAG_WAIT_START) << 0)
#define BT_ULL_FLAG_WAIT_AMI_STOP_CODEC          ((BT_ULL_FLAG_WAIT_START) << 1)
#define BT_ULL_FLAG_WAIT_AMI_RESTART_CODEC          ((BT_ULL_FLAG_WAIT_START) << 2)


#define BT_ULL_AM_IDLE                    (0x00)
#define BT_ULL_AM_READY                   (0x01)
#define BT_ULL_AM_PLAYING                 (0x02)
typedef uint32_t bt_ull_am_state_t;

#define BT_ULL_AM_PLAY_IND                (0x00)
#define BT_ULL_AM_STOP_IND                (0x01)
#define BT_ULL_AM_SUSPEND_IND             (0x02)
#define BT_ULL_AM_REJECT_IND              (0x03)
#define BT_ULL_AM_CODEC_OPEN_IND          (0x04)
typedef uint8_t bt_ull_am_ind_state_t;


typedef enum {
    BT_ULL_SUB_STATE_NONE = 0,
    BT_ULL_SUB_STATE_PREPARE_AUDIO_SRC,
    BT_ULL_SUB_STATE_PREPARE_CODEC,
    BT_ULL_SUB_STATE_CLEAR_CODEC,
} bt_ull_am_sub_state_t;

void bt_ull_am_callback(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param);
audio_src_srv_handle_t *bt_ull_am_init(void);
void bt_ull_am_deinit(void);
void bt_ull_am_play(void);
void bt_ull_am_stop(void);
void bt_ull_am_restart(void);

/**
 * @brief                     Request to mute the current player.
 * @param[in] is_mute             is the action of mute.
 * @return                    The result of set mute.
 */
int32_t bt_ull_am_set_mute(bool is_mute);
int32_t bt_ull_am_set_volume(uint8_t volume);

#endif
