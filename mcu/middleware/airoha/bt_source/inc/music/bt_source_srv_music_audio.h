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
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOMUSICY ACKNOWLEDGES AND AGREES
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

#ifndef __BT_SOURCE_SRV_MUSIC_AUDIO_H__
#define __BT_SOURCE_SRV_MUSIC_AUDIO_H__
#include "bt_type.h"
#include "bt_a2dp.h"
#include "audio_transmitter_control.h"
#include "audio_transmitter_control_port.h"
#include "bt_source_srv.h"
#include "bt_sink_srv_ami.h"
#include "bt_source_srv_common.h"
#include "bt_source_srv_music_pseduo_dev.h"
#include "bt_source_srv_music_internal.h"
#include "bt_codec.h"

#define BT_A2DP_SOURCE_TRANSMITTER_SHARE_BUFFER_SIZE (40*1024)
#define BT_A2DP_SOURCE_FRAME_INTERVAL 5*1000

#define BT_SOURCE_SRV_MUSIC_AUDIO_DEFAULT_VOLUME  AUD_VOL_OUT_LEVEL10

typedef enum{
    BT_SOURCE_SRV_MUSIC_AUDIO_PLAY_TYPE_NONE = 0x00,
    BT_SOURCE_SRV_MUSIC_AUDIO_PLAY_TYPE_CODEC,
    BT_SOURCE_SRV_MUSIC_AUDIO_PLAY_TYPE_TRANSMITTER_UL,
    BT_SOURCE_SRV_MUSIC_AUDIO_PLAY_TYPE_TRANSMITTER_DL,
    BT_SOURCE_SRV_MUSIC_AUDIO_PLAY_TYPE_LINE_IN,
    BT_SOURCE_SRV_MUSIC_AUDIO_PLAY_TYPE_I2S_IN,
    BT_SOURCE_SRV_MUSIC_AUDIO_PLAY_TYPE_I2S_IN_1,

    BT_SOURCE_SRV_MUSIC_AUDIO_PLAY_TYPE_MAX
} bt_source_srv_music_audio_play_t;

#define BT_SOURCE_SRV_MUSIC_AUDIO_PORT_UPDATE_PORT            0x0001
#define BT_SOURCE_SRV_MUSIC_AUDIO_PORT_UPDATE_SAMPLE_RATE     0x0002
#define BT_SOURCE_SRV_MUSIC_AUDIO_PORT_UPDATE_SAMPLE_SIZE     0x0004
#define BT_SOURCE_SRV_MUSIC_AUDIO_PORT_UPDATE_SAMPLE_CHANNEL  0x0008
typedef uint16_t bt_source_srv_music_audio_port_update_t;

#define BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID      AUD_ID_INVALID

#define BT_SOURCE_SRV_CODEC_SBC  (1 << 0)
#define BT_SOURCE_SRV_CODEC_AAC  (1 << 1)
#define BT_SOURCE_SRV_CODEC_LDAC (1 << 2)
#define BT_SOURCE_SRV_CODEC_LHDC (1 << 3)

typedef bt_sink_srv_am_audio_capability_t          bt_source_srv_music_audio_capability_t;

typedef audio_transmitter_id_t                     bt_source_srv_music_audio_id_t;
typedef uint8_t                                    bt_source_srv_music_audio_sub_id_t;
typedef audio_transmitter_config_t                 bt_source_srv_music_audio_transmitter_config_t;
typedef bt_a2dp_codec_type_t                  bt_source_srv_music_audio_codec_type_t;


typedef struct {
    bt_source_srv_port_t             port;
    uint32_t                         sample_rate;
    uint32_t                         sample_size;
    uint32_t                         sample_channel;
} bt_source_srv_music_audio_port_context_t;

typedef struct {
    bt_source_srv_music_audio_play_t                   type;
    union {
        bt_source_srv_music_audio_transmitter_config_t transmitter_config;
        bt_source_srv_music_audio_capability_t         audio_capability;
    };
} bt_source_srv_music_audio_config_t;


bt_status_t bt_source_srv_music_audio_config_init(bt_source_srv_music_audio_config_t *config, bt_source_srv_music_device_t *context,  void *user_data, void *user_callback);

bt_source_srv_music_audio_id_t bt_source_srv_music_audio_start(bt_source_srv_music_audio_config_t *config);

bt_status_t bt_source_srv_music_audio_stop(audio_transmitter_id_t id);

bt_status_t bt_source_srv_music_audio_deinit(audio_transmitter_id_t id);

bt_status_t bt_source_srv_set_music_enable(bt_bd_addr_t *dev_addr, bool enable);

void bt_source_srv_music_psd_start_detect_media_data(int8_t audio_id);

bt_status_t bt_source_src_music_audio_mute(audio_transmitter_id_t sub_id);

bt_status_t bt_source_src_music_audio_unmute(audio_transmitter_id_t sub_id);

#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
bt_status_t bt_source_srv_music_set_audio_volume_by_port(bt_source_srv_music_device_t *device, bt_source_srv_port_t port, uint32_t volume);
#endif

#endif

