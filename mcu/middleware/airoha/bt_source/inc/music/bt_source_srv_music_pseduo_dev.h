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

#ifndef __BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_H__
#define __BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_H__
#include "bt_type.h"
#include "bt_a2dp.h"
#include "bt_source_srv.h"
#include "bt_source_srv_common.h"

#define BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_NUM                     0x03

#define BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_CONNECT_REQ_IND     0x01
#define BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_CONNECTED           0x02
#define BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED        0x03
#define BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_REPLAY             0x04
#define BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_START              0x08
#define BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_PLAY_IND           0x50
#define BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_STOP_IND           0x51
#define BT_SOURCE_SRV_MUSIC_PSD_EVENT_SUSPEND_REQ              0x82
typedef uint8_t bt_source_srv_music_psd_event_t;

#define BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_DEINIT              0x01
#define BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_REJECT              0x02
#define BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_SUSPEND             0x03
#define BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_RESUME              0x04
#define BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_MEDIA_DETECT         0x05
#define BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_SUSPEND_BY_PRIORITY 0x06
#define BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_AUDIO_START_SUCCESS 0x07
#define BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_AUDIO_START_FAIL 0x08
#define BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_SUSPEND_BY_PORT_INVAILD 0x09



#define BT_SOURCE_SRV_MUSIC_PORT_ACTION_NONE      BT_SOURCE_SRV_COMMON_PORT_ACTION_NONE
#define BT_SOURCE_SRV_MUSIC_PORT_ACTION_OPEN      BT_SOURCE_SRV_COMMON_PORT_ACTION_OPEN
#define BT_SOURCE_SRV_MUSIC_PORT_ACTION_CLOSE     BT_SOURCE_SRV_COMMON_PORT_ACTION_CLOSE
#define BT_SOURCE_SRV_MUSIC_PORT_ACTION_UPDATE    BT_SOURCE_SRV_COMMON_PORT_ACTION_UPDATE
typedef bt_source_srv_common_port_action_t bt_source_srv_music_port_action_t;


typedef struct {
    bt_source_srv_port_t  port;
    bt_source_srv_music_port_action_t action;
} bt_source_srv_music_psd_audio_replay_req_t;

typedef uint8_t bt_source_srv_music_psd_user_event_t;
typedef bt_status_t (*bt_source_srv_music_psd_user_musicback)(bt_source_srv_music_psd_user_event_t event_id, void *device, void *parameter);

typedef bt_a2dp_codec_type_t bt_source_srv_music_audio_codec_type_t;
typedef uint8_t bt_source_srv_music_audio_volume_t;


bt_status_t bt_source_srv_music_psd_init(void);

bt_status_t bt_source_srv_music_psd_deinit(void);

void *bt_source_srv_music_psd_alloc_device(bt_bd_addr_t *bd_address, bt_source_srv_music_psd_user_musicback musicback);

bt_status_t bt_source_srv_music_psd_free_device(void *device);

void bt_source_srv_music_psd_set_codec_type(void *device, bt_source_srv_music_audio_codec_type_t codec_type);

void bt_source_srv_music_psd_set_speaker_volume(void *device, bt_source_srv_music_audio_volume_t volume);

bt_source_srv_music_audio_volume_t bt_source_srv_music_psd_get_speaker_volume(void *device);

bool bt_source_srv_music_psd_is_playing(void *device);

void bt_source_srv_music_psd_update_port(void *device, bt_source_srv_port_t port);

bt_status_t bt_source_srv_music_audio_port_update(bt_source_srv_port_t audio_port, bt_source_srv_music_port_action_t action);

uint32_t bt_source_srv_music_get_audio_number(void *device);

bool bt_source_srv_music_psd_is_idle(void *device);

#endif
