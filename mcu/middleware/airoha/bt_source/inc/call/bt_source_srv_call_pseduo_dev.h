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

#ifndef __BT_SOURCE_SRV_CALL_PSEDUO_DEV_H__
#define __BT_SOURCE_SRV_CALL_PSEDUO_DEV_H__
#include "bt_type.h"
#include "bt_source_srv_call_audio.h"

#define BT_SOURCE_SRV_CALL_PSEDUO_DEV_NUM                     0x03

#define BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECT_REQ_IND     0x01
#define BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECTED           0x02
#define BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED        0x03
#define BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_CONNECTED            0x04
#define BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DISCONNECTED         0x05
#define BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_ACTIVATED            0x06
#define BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DEACTIVATED          0x07
#define BT_SOURCE_SRV_CALL_PSD_EVENT_CALL_START               0x08
#define BT_SOURCE_SRV_CALL_PSD_EVENT_CALL_END                 0x09

#define BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_PLAY_IND           0x50
#define BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_STOP_IND           0x51

#define BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECT_REQ         0x80
#define BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECT_REQ      0x81
#define BT_SOURCE_SRV_CALL_PSD_EVENT_SUSPEND_REQ              0x82
#define BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_REPLAY_REQ         0x83
#define BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_STOP_REQ           0x84
typedef uint8_t bt_source_srv_call_psd_event_t;

#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_DEINIT                             0x01
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_ACTIVATE_SCO                       0x02
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_DEACTIVATE_SCO                     0x03
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_SUSPEND                            0x04
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_MIC_LOCATION                   0x05
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_REMOTE_ADDRESS                 0x06
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_CALL_STATE                     0x07
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_ESCO_STATE                     0x08
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_SLIENCE_DETECTION_SUSPEND          0x09
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_SLIENCE_DETECTION_RESUME           0x0A
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_AUDIO_PLAY_COMPLETE                0x0B
#define BT_SOURCE_SRV_CALL_PSD_USER_EVENT_AUDIO_STOP_COMPLETE                0x0C
typedef uint8_t bt_source_srv_call_psd_user_event_t;

#define BT_SOURCE_SRV_CALL_PSD_LOCATION_REMOTE            0x00
#define BT_SOURCE_SRV_CALL_PSD_LOCATION_LOCAL             0x01
typedef uint8_t bt_source_srv_call_psd_location_t;

#define BT_SOURCE_SRV_CALL_PSD_ESCO_DISCONNECTED          0x00
#define BT_SOURCE_SRV_CALL_PSD_ESCO_CONNECTED             0x01
typedef uint8_t bt_source_srv_psd_esco_state_t;

#define BT_SOURCE_SRV_CALL_PSD_CALL_NON_EXISTENCE         0x00
#define BT_SOURCE_SRV_CALL_PSD_CALL_EXISTENCE             0x01
typedef uint8_t bt_source_srv_psd_call_state_t;

#define  BT_SOURCE_SRV_CALL_PSD_CALL_ACTION_CLOSE         0x00
#define  BT_SOURCE_SRV_CALL_PSD_CALL_ACTION_OPEN          0x01
#define  BT_SOURCE_SRV_CALL_PSD_CALL_ACTION_UPDATE        0x02
typedef uint8_t bt_source_srv_psd_call_port_action_t;

typedef struct {
    bt_source_srv_call_psd_location_t location;
} bt_source_srv_call_psd_mic_location_t;

typedef struct {
    bt_bd_addr_t   remote_address;
} bt_source_srv_call_psd_remote_address_t;

/* The structure define for BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_REPLAY_REQ event */
typedef struct {
    bt_source_srv_call_audio_play_t        type;
    bt_source_srv_psd_call_port_action_t   port_action;
} bt_source_srv_call_psd_audio_replay_req_t;

typedef struct {
    bool                      is_allow_audio_stop;
} bt_source_srv_call_psd_call_end_t;

typedef bt_source_srv_call_psd_audio_replay_req_t bt_source_srv_call_psd_audio_stop_req_t;

typedef struct {
    bt_source_srv_psd_esco_state_t    state;
} bt_source_srv_psd_get_esco_state_t;

typedef struct {
    bt_source_srv_psd_call_state_t    state;
} bt_source_srv_psd_get_call_state_t;

typedef bt_status_t (*bt_source_srv_call_psd_user_callback)(void *device, bt_source_srv_call_psd_user_event_t event_id, void *parameter);


bt_status_t bt_source_srv_call_psd_init(void);

bt_status_t bt_source_srv_call_psd_deinit(void);

void *bt_source_srv_call_psd_alloc_device(bt_bd_addr_t *bd_address, bt_source_srv_call_psd_user_callback callback);

bt_status_t bt_source_srv_call_psd_free_device(void *device);

void bt_source_srv_call_psd_set_codec_type(void *device, bt_source_srv_call_audio_codec_type_t codec_type);

bt_source_srv_call_audio_codec_type_t bt_source_srv_call_psd_get_codec_type(void *device);

void bt_source_srv_call_psd_event_notify(void *device, bt_source_srv_call_psd_event_t event, void *parameter);

bool bt_source_srv_call_psd_is_ready(void *device);

bool bt_source_srv_call_psd_is_connecting(void *device);

bool bt_source_srv_call_psd_is_playing(void *device);

void bt_source_srv_call_psd_set_speaker_volume(void *device, bt_source_srv_call_audio_volume_t volume);

bt_source_srv_call_audio_volume_t bt_source_srv_call_psd_get_speaker_volume(void *device);

bt_status_t bt_source_srv_call_psd_audio_mute(void *device, bt_source_srv_call_audio_play_t play_type);

bt_status_t bt_source_srv_call_psd_audio_unmute(void *device, bt_source_srv_call_audio_play_t play_type);

#endif
