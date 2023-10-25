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

#ifndef __BT_SOURCE_SRV_CALL_AUDIO_H__
#define __BT_SOURCE_SRV_CALL_AUDIO_H__
#include "bt_sink_srv_ami.h"
#include "bt_hfp.h"
#include "bt_source_srv.h"
#include "audio_transmitter_control.h"
#include "audio_transmitter_control_port.h"

typedef bt_sink_srv_am_volume_level_out_t          bt_source_srv_call_audio_volume_t;
typedef bt_sink_srv_am_audio_capability_t          bt_source_srv_call_audio_capability_t;
typedef bt_sink_srv_am_priority_t                  bt_source_srv_call_audio_priority_t;
typedef bt_sink_srv_am_notify_callback             bt_source_srv_call_audio_notify_cb;

typedef int8_t                                     bt_source_srv_call_audio_id_t;
typedef uint8_t                                    bt_source_srv_call_audio_sub_id_t;
typedef audio_transmitter_config_t                 bt_source_srv_call_audio_transmitter_config_t;
typedef audio_codec_msbc_t                         bt_source_srv_call_audio_codec_msbc_t;

typedef bt_hfp_audio_codec_type_t                  bt_source_srv_call_audio_codec_type_t;

#define BT_SOURCE_SRV_CALL_AUDIO_PLAY_NUMBER       2

#define BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID      AUD_ID_INVALID
#define BT_SOURCE_SRV_CALL_AUDIO_MAX_VOLUME      (AUD_VOL_OUT_MAX - 1)
#define BT_SOURCE_SRV_CALL_AUDIO_MIN_VOLUME      AUD_VOL_OUT_LEVEL0
#define BT_SOURCE_SRV_CALL_AUDIO_DEFAULT_VOLUME  AUD_VOL_OUT_LEVEL10
#define BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID      AUD_ID_INVALID

#define BT_SOURCE_SRV_CALL_AUDIO_SHARE_INFO_INDEX_DONGLE_BT_SEND_TO_AIR_0        AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_SEND_TO_AIR_0
#define BT_SOURCE_SRV_CALL_AUDIO_SHARE_INFO_INDEX_DONGLE_BT_RECEIVE_FROM_AIR_0   AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_RECEIVE_FROM_AIR_0

#define BT_SOURCE_SRV_CALL_AUDIO_VOLUME_MASK                      0xF0


typedef enum {
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_CODEC = 0,
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL,
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL,
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN,
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_I2S_IN,
    BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_MAX
} bt_source_srv_call_audio_play_t;

typedef struct {
    bt_source_srv_call_audio_play_t                   type;
    union {
        bt_source_srv_call_audio_transmitter_config_t transmitter_config;
        bt_source_srv_call_audio_capability_t         audio_capability;
    };
} bt_source_srv_call_audio_config_t;

bt_source_srv_call_audio_id_t bt_source_srv_call_audio_config_init(bt_source_srv_call_audio_config_t *config, bt_source_srv_call_audio_codec_type_t codec_type, void *user_data, void *user_callback);

bt_status_t bt_source_srv_call_audio_play(bt_source_srv_call_audio_id_t audio_id);

bt_status_t bt_source_srv_call_audio_stop(bt_source_srv_call_audio_id_t audio_id);

bt_status_t bt_source_srv_call_audio_config_deinit(bt_source_srv_call_audio_id_t audio_id);

bt_status_t bt_source_srv_call_audio_controller_config(bt_source_srv_call_audio_codec_type_t codec_type);

bt_status_t bt_source_srv_call_audio_trigger_play(uint32_t gap_handle);

bt_status_t bt_source_src_call_audio_mute(bt_source_srv_call_audio_id_t audio_id);

bt_status_t bt_source_src_call_audio_unmute(bt_source_srv_call_audio_id_t audio_id);

void bt_source_srv_call_audio_slience_detection_start(bt_source_srv_call_audio_id_t audio_id, void *callback);

void bt_source_srv_call_audio_slience_detection_stop(bt_source_srv_call_audio_id_t audio_id);

#endif
