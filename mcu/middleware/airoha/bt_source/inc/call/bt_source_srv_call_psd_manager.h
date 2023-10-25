/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#ifndef __BT_SOURCE_SRV_CALL_PSD_MANAGER_H__
#define __BT_SOURCE_SRV_CALL_PSD_MANAGER_H__
#include "bt_type.h"
#include "audio_src_srv.h"
#include "bt_source_srv_call_audio.h"
#include "bt_source_srv_call_pseduo_dev.h"

typedef enum {
    BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER = 0x00,
    BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_MAX,
} bt_source_srv_call_psd_audio_source_t;

#define BT_SOURCE_SRV_CALL_PSD_STATE_NONE                       0x00
#define BT_SOURCE_SRV_CALL_PSD_STATE_READY                      0x01
#define BT_SOURCE_SRV_CALL_PSD_STATE_TAKE_AUDIO_SRC             0x02
#define BT_SOURCE_SRV_CALL_PSD_STATE_PLAY                       0x03
#define BT_SOURCE_SRV_CALL_PSD_STATE_GIVE_AUDIO_SRC             0x04
typedef uint8_t bt_source_srv_call_psd_state_t;

#define BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE                   0x00
#define BT_SOURCE_SRV_CALL_PSD_SUB_STATE_CONNECTING             0x01
#define BT_SOURCE_SRV_CALL_PSD_SUB_STATE_DISCONNECTING          0x02
#define BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_IDLE              0x03
#define BT_SOURCE_SRV_CALL_PSD_SUB_STATE_SCO_ACTIVATING         0x04
#define BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STARTING          0x05
#define BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAYING                0x06
#define BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STOPPING          0x07
#define BT_SOURCE_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING       0x08
typedef uint8_t bt_source_srv_call_psd_sub_state_t;

#define BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_INIT                  0x00
#define BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE                  0x01
#define BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY                 0x02
#define BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_PLAY                  0x03
typedef uint8_t bt_source_srv_call_psd_next_state_t;

#define BT_SOURCE_SRV_CALL_PSD_FLAG_NONE                       0x0000
#define BT_SOURCE_SRV_CALL_PSD_FLAG_TRANSMITTER_TAKED          0x0001

#define BT_SOURCE_SRV_CALL_PSD_FLAG_TRANSMITTER_ADD_WL         0x0010
#define BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING            0x0020
#define BT_SOURCE_SRV_CALL_PSD_FLAG_UL_NEXT_REPLAY             0x0040
#define BT_SOURCE_SRV_CALL_PSD_FLAG_DL_NEXT_REPLAY             0x0080
#define BT_SOURCE_SRV_CALL_PSD_FLAG_LINE_IN_NEXT_REPLAY        0x0100
#define BT_SOURCE_SRV_CALL_PSD_FLAG_I2S_IN_NEXT_REPLAY         0x0200

#define BT_SOURCE_SRV_CALL_PSD_FLAG_UL_PLAY_TRIGGER            0x1000
#define BT_SOURCE_SRV_CALL_PSD_FLAG_DL_PLAY_TRIGGER            0x2000
#define BT_SOURCE_SRV_CALL_PSD_FLAG_LINE_IN_PLAY_TRIGGER       0x3000
#define BT_SOURCE_SRV_CALL_PSD_FLAG_I2S_IN_PLAY_TRIGGER        0x4000
typedef uint16_t bt_source_srv_call_psd_flag_t;

#define BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, flag)       (device->flags & flag)
#define BT_SOURCE_SRV_CALL_PSD_SET_FLAG(device, flag)          (device->flags |= flag)
#define BT_SOURCE_SRV_CALL_PSD_REMOVE_FLAG(device, flag)       (device->flags &= ~flag)

typedef struct {
    audio_src_srv_resource_manager_handle_t     *transmitter_audio_src;
    bt_source_srv_call_audio_id_t               audio_id[BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_MAX];
    bt_source_srv_call_psd_state_t              state;
    bt_source_srv_call_psd_sub_state_t          sub_state;
    bt_source_srv_call_psd_next_state_t         next_state;
    bt_source_srv_call_audio_codec_type_t       codec_type;
    bt_source_srv_call_audio_volume_t           speaker_volume;
    bt_source_srv_call_audio_volume_t           mic_volume;
    bt_source_srv_call_psd_user_callback        user_callback;
    bt_source_srv_call_psd_flag_t               flags;
    uint8_t                                     audio_play_number;
} bt_source_srv_call_pseduo_dev_t;

bt_status_t bt_source_srv_call_psd_alloc_audio_src(bt_source_srv_call_pseduo_dev_t *device);

bt_status_t bt_source_srv_call_psd_free_audio_src(bt_source_srv_call_pseduo_dev_t *device);

bt_status_t bt_source_srv_call_psd_state_machine(bt_source_srv_call_pseduo_dev_t *device,
        bt_source_srv_call_psd_event_t event, void *parameter);

void bt_source_srv_call_psd_audio_transmitter_callback(audio_transmitter_event_t event, void *data, void *user_data);

#endif
