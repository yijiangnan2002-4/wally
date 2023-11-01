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

#ifndef __BT_SOURCE_SRV_CALL_H__
#define __BT_SOURCE_SRV_CALL_H__
#include "bt_source_srv.h"
#include "bt_source_srv_call_audio.h"
#include "bt_source_srv_common.h"


#define BT_SOURCE_SRV_CALL_VOLUME_SCALE_100           100U
#define BT_SOURCE_SRV_CALL_VOLUME_SCALE_50            50U
#define BT_SOURCE_SRV_CALL_VOLUME_SCALE_15            15U
typedef uint32_t bt_source_srv_call_volume_scale_t;


#define BT_SOURCE_SRV_CALL_PORT_ACTION_NONE      BT_SOURCE_SRV_COMMON_PORT_ACTION_NONE
#define BT_SOURCE_SRV_CALL_PORT_ACTION_OPEN      BT_SOURCE_SRV_COMMON_PORT_ACTION_OPEN
#define BT_SOURCE_SRV_CALL_PORT_ACTION_CLOSE     BT_SOURCE_SRV_COMMON_PORT_ACTION_CLOSE
#define BT_SOURCE_SRV_CALL_PORT_ACTION_UPDATE    BT_SOURCE_SRV_COMMON_PORT_ACTION_UPDATE
typedef bt_source_srv_common_port_action_t bt_source_srv_call_port_action_t;

typedef struct {
    bt_source_srv_hfp_init_parameter_t *hfp_init_param;
} bt_source_srv_call_init_parameter_t;

#ifdef MTK_BT_CM_SUPPORT
typedef struct {
    uint8_t                  local_speaker_volume;
    uint8_t                  speaker_volume;
    uint8_t                  mic_gain;
    uint8_t                  audio_source_speaker_volume;            /* PC default volume */
} bt_source_srv_call_stored_data_t;
#endif

typedef enum {
    BT_SOURCE_SRV_CALL_GAIN_LOCAL_SPEAKER = 0,
    BT_SOURCE_SRV_CALL_GAIN_SPEAKER,
    BT_SOURCE_SRV_CALL_GAIN_MIC,
    BT_SOURCE_SRV_CALL_GAIN_AUDIO_SOURCE_SPEAKER
} bt_source_srv_call_gain_t;

bt_status_t bt_source_srv_call_init(bt_source_srv_call_init_parameter_t *init_param);

bt_status_t bt_source_srv_call_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);

bt_status_t bt_source_srv_call_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length);

bt_status_t bt_source_srv_call_audio_gain_init(bt_bd_addr_t *bd_address, void *device);

bt_status_t bt_source_srv_call_audio_gain_update(bt_source_srv_call_gain_t gain_type, bt_bd_addr_t *bd_address, void *device, \
        bt_source_srv_call_audio_volume_t gain_level);

uint32_t bt_source_srv_call_get_audio_gain_level(bt_source_srv_call_gain_t gain_type, bt_bd_addr_t *bd_address);

bt_status_t bt_source_srv_call_audio_port_update(bt_source_srv_port_t audio_port, bt_source_srv_call_port_action_t action);

uint32_t bt_source_srv_call_convert_volume(bt_source_srv_call_volume_scale_t vcf_type, uint32_t volume_value, bool is_forward);

uint8_t bt_source_srv_call_get_playing_device_codec(void);

uint32_t bt_source_srv_call_convert_volume_step(uint32_t min_volume, uint32_t max_volume);

#endif
