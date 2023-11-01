/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef __BT_SINK_SRV_AVRCP_H__
#define __BT_SINK_SRV_AVRCP_H__

#include "stdint.h"
#include "bt_connection_manager_internal.h"
#include "bt_avrcp.h"
#include "bt_sink_srv_music.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BT_SINK_SRV_AVRCP_DISCONNECTED,
    BT_SINK_SRV_AVRCP_CONNECTING,
    BT_SINK_SRV_AVRCP_CONNECTED,
    BT_SINK_SRV_AVRCP_PLAY,
    BT_SINK_SRV_AVRCP_STOP,

    BT_SINK_SRV_AVRCP_TOTAL
} bt_sink_srv_avrcp_state_t;

#ifdef AIR_FEATURE_SINK_AVRCP_BQB
#define BT_SINK_SRV_AVRCP_BQB_HEADER "AT+BTSINKBQB="
#endif

bt_status_t  bt_sink_srv_avrcp_cm_callback_handler(bt_cm_profile_service_handle_t type, void *data);
bt_status_t bt_sink_srv_avrcp_action_handler(bt_sink_srv_action_t action, void *param);

int32_t bt_sink_srv_avrcp_play_music(bt_sink_srv_music_device_t *dev);

int32_t bt_sink_srv_avrcp_stop_music(bt_sink_srv_music_device_t *dev);

bt_status_t bt_sink_srv_avrcp_force_pause_music(bt_sink_srv_music_device_t *dev);

int32_t bt_sink_srv_avrcp_change_track(uint32_t handle, bool is_ptrack);

bt_status_t bt_sink_srv_avrcp_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);

bt_status_t bt_sink_srv_music_avrcp_action_handler(bt_sink_srv_action_t action, void *param);

uint8_t bt_sink_srv_avrcp_get_volume_level(uint8_t avrcp_volume);
bt_status_t bt_sink_srv_avrcp_volume_notification(uint32_t handle, uint8_t vol_level, uint8_t type);
void bt_sink_srv_init_role(bt_avrcp_role_t role);
bt_status_t bt_sink_srv_avrcp_send_play_pause_command(bt_sink_srv_music_device_t *dev, bt_avrcp_operation_id_t action_id);

int32_t bt_sink_srv_avrcp_send_pass_through(uint32_t handle, bt_avrcp_operation_id_t op_id, bt_avrcp_operation_state_t op_state);
#ifdef AIR_FEATURE_SINK_AVRCP_BQB
bool bt_sink_srv_avrcp_bqb_in_progress(void);
bt_status_t bt_sink_srv_avrcp_bqb_common_cb(bt_msg_type_t msg, bt_status_t status, void *buff);
#endif

#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
bool bt_sink_srv_avrcp_is_push_release_command_timer_active(void);
#endif
bool bt_sink_srv_avrcp_is_playing(uint32_t avrcp_status);

#ifdef __cplusplus
}
#endif

#endif
