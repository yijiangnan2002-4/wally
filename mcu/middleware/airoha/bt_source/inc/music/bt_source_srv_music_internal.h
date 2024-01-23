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

#ifndef __BT_SOURCE_SRV_MUSIC_INTERNAL__
#define __BT_SOURCE_SRV_MUSIC_INTERNAL__
#include "bt_avrcp.h"
#include "bt_a2dp.h"
#include "bt_source_srv.h"
#include "bt_connection_manager_internal.h"


#define BT_SOURCE_SRV_MAX_DEVICE_NUMBER BT_SINK_SRV_CM_MAX_DEVICE_NUMBER
#define BT_SOURCE_SRV_INVAILD_HANDLE 0xFFFF
#define BT_SOURCE_SRV_A2DP_CODEC_MAX_NUM 1
#define BT_SOURCE_SRV_A2DP_INVAILD_LAST_CMD 0

#define BT_SRV_MUSIC_DEVICE_ADDR_A2DP 2
#define BT_SRV_MUSIC_DEVICE_UNUSED 5
#define BT_SRV_MUSIC_DEVICE_A2DP_HD 0
#define BT_SRV_MUSIC_DEVICE_AVRCP_HD 1
#define BT_SRV_MUSIC_DEVICE_ADDR_AVRCP 3
#define BT_SRV_MUSIC_DEVICE_AUDIO_RESOURCE_DEVICE 4
#define BT_SRV_MUSIC_DEVICE_HIGHLIGHT 6
#define BT_SRV_MUSIC_DEVICE_USED    7
#define BT_SRV_MUSIC_DEVICE_VAILD_DEVICE 8

typedef uint8_t bt_srv_music_device_type_t;



/*bt source internal state*/
#define BT_SOURCE_SRV_STATE_IDLE 0x00
#define BT_SOURCE_SRV_STATE_CONNECTING 0x01
#define BT_SOURCE_SRV_STATE_READY 0x02
#define BT_SOURCE_SRV_STATE_WAIT_STREAMING_RESPONSE 0x03
#define BT_SOURCE_SRV_STATE_PREPAR_STREAMING 0x04
#define BT_SOURCE_SRV_STATE_STREAMING 0x05
#define BT_SOURCE_SRV_STATE_PREPARE_SUSPEND 0x06
#define BT_SOURCE_SRV_STATE_PREPARE_TAKE_AUDIO_RESOURCE 0x007
#define BT_SOURCE_SRV_STATE_TAKE_AUDIO_RESOURCE 0x08
#define BT_SOURCE_SRV_STATE_DISCONNECTING 0x09
typedef uint8_t bt_source_srv_state_t;

#define BT_SOURCE_SRV_A2DP_REGISTER_FLAG 0x01
#define BT_SOURCE_SRV_A2DP_MUTE_FLAG 0x02

#define BT_SOURCE_SRV_AVRCP_CONNECTION_TIMER_DUR  (800)

#define BT_SOURCE_SRV_NEXT_ACTION_IS_NO_ACTION (0x00)
#define BT_SOURCE_SRV_NEXT_ACTION_SWITCH_CODEC_TO_SBC (0x01)
#define BT_SOURCE_SRV_NEXT_ACTION_SWITCH_CODEC_TO_VENDOR_CODEC (0x02)
typedef uint32_t bt_source_srv_next_action_t;


typedef struct {
    bt_bd_addr_t dev_addr;
    /*a2dp*/
    bt_source_srv_state_t a2dp_state;/*a2dp state*/
    uint8_t sub_state;/*optional*/
    uint32_t a2dp_hd;
    uint32_t max_mtu;
    uint8_t detect_flag;
    uint8_t flag;/*bit0:register_flag, bit1:mute_flag*/
    uint32_t next_action;
    uint32_t codec_type;
    bt_a2dp_codec_capability_t capabilty;/*codec*/

    /*avrcp*/
    uint32_t avrcp_hd;
    uint32_t last_cmd;
    uint8_t bt_volume;
    bool absolute_support;
    bt_source_srv_state_t avrcp_state;/*avrcp state*/
    bt_avrcp_role_t avrcp_role; /*avrcp role in connetion*/
    bt_avrcp_media_play_status_event_t avrcp_play_status;/*avrcp state */

    void *audio_dev;
} bt_source_srv_music_device_t;

typedef struct {
    bool bqb_flag;
    bt_source_srv_music_device_t source_dev[BT_SOURCE_SRV_MAX_DEVICE_NUMBER];
    //bt_sink_srv_device_t sink_dev[BT_SOURCE_SRV_MAX_DEVICE_NUMBER];
} bt_audio_srv_context_t;

uint32_t bt_source_srv_get_max_mtu_size(uint32_t handle);

#endif

