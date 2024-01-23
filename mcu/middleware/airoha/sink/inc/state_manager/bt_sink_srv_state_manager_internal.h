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

#ifndef __BT_SINK_SRV_STATE_MANAGER_INTERNAL_H__
#define __BT_SINK_SRV_STATE_MANAGER_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bt_sink_srv.h"
#include "bt_sink_srv_common.h"
#ifdef AIR_BT_SINK_MUSIC_ENABLE
#include "bt_sink_srv_music.h"
#endif
#include "bt_sink_srv_state_manager.h"
#include "bt_sink_srv_utils.h"

#include "bt_connection_manager.h"
#include "bt_utils.h"
#include "project_config.h"

#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le.h"
#endif

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#endif

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
#include "bt_role_handover.h"
#endif

#define BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM            6
#define BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM     (BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM + 1)
#define BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE            {0}
#define BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE_STATE      {{0}}

#define BT_SINK_SRV_STATE_MANAGER_NONE_STATE \
    (BT_SINK_SRV_STATE_POWER_ON | BT_SINK_SRV_STATE_CONNECTED)

#define BT_SINK_SRV_STATE_MANAGER_CALL_STATE \
    (BT_SINK_SRV_STATE_INCOMING | BT_SINK_SRV_STATE_OUTGOING | BT_SINK_SRV_STATE_ACTIVE |               \
     BT_SINK_SRV_STATE_TWC_INCOMING | BT_SINK_SRV_STATE_TWC_OUTGOING | BT_SINK_SRV_STATE_HELD_ACTIVE |  \
     BT_SINK_SRV_STATE_HELD_REMAINING | BT_SINK_SRV_STATE_MULTIPARTY)

#define BT_SINK_SRV_STATE_MANAGER_MEDIA_STATE \
    (BT_SINK_SRV_STATE_STREAMING)

#define BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE(state) \
    ((state) == BT_SINK_SRV_STATE_NONE || ((state) & BT_SINK_SRV_STATE_MANAGER_NONE_STATE))

#define BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(state) \
    ((state) & BT_SINK_SRV_STATE_MANAGER_CALL_STATE)

#define BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE(state) \
    ((state) & BT_SINK_SRV_STATE_MANAGER_MEDIA_STATE)

#define BT_SINK_SRV_STATE_MANAGER_PROFILE_MASK \
    (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) | \
     BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HSP) | \
     BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK))

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
#define BT_SINK_SRV_STATE_MANAGER_IS_SINK_SRV_DEVICE(type) \
     ((type) == AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP ||      \
      (type) == AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP ||  \
      (type) == AUDIO_SRC_SRV_PSEUDO_DEVICE_HFP ||       \
      (type) == AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE ||       \
      (type) == AUDIO_SRC_SRV_PSEUDO_DEVICE_DUMMY)
#else
#define BT_SINK_SRV_STATE_MANAGER_IS_SINK_SRV_DEVICE(type) \
    ((type) == AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP ||      \
     (type) == AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP ||  \
     (type) == AUDIO_SRC_SRV_PSEUDO_DEVICE_HFP ||       \
     (type) == AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE)
#endif

#define BT_SINK_SRV_STATE_MANAGER_IS_CALL_DEVICE(device)                \
    ((NULL != (device)) &&                                              \
     (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE((device)->call_state) ||  \
      BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != (device)->call_audio_state))

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
#define BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_INVALID            0x00
#define BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_READY              0x01
#define BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_PLAYING            0x02
typedef uint8_t bt_sink_srv_state_manager_next_state_t;
#endif

#define BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_RING              0x01
#define BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_TWC_RING          0x02
#define BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_SEND_RING         0x04
typedef uint8_t bt_sink_srv_state_manager_device_flag_t;

#define BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_STATE               0x00
#define BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_FOCUS               0x01
#define BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_PLAYED              0x02
#define BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_ACTION              0x03
#define BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_PLAY_COUNT          0x04
#define BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_REQUEST_STATE       0x05
typedef uint8_t bt_sink_srv_state_manager_sync_type_t;

#define BT_SINK_SRV_STATE_MANAGER_SYNC_DIRECTION_AGENT          0x00
#define BT_SINK_SRV_STATE_MANAGER_SYNC_DIRECTION_PARTNER        0x01
typedef uint8_t bt_sink_srv_state_manager_sync_direction_t;

#define BT_SINK_SRV_STATE_MANAGER_FOCUS_DEVICE_CALL             0x01
#define BT_SINK_SRV_STATE_MANAGER_FOCUS_DEVICE_MEDIA            0x02
typedef uint8_t bt_sink_srv_state_manager_focus_device_t;

typedef struct {
    bt_sink_srv_state_manager_device_type_t     type;
    bt_bd_addr_t                                address;
    bt_sink_srv_state_t                         call_state;
    bt_sink_srv_sco_connection_state_t          call_audio_state;
    bt_sink_srv_state_t                         media_state;
    audio_src_srv_handle_t                      *call_device;
    audio_src_srv_handle_t                      *media_device;
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
    audio_src_srv_handle_t                      *dummy_device;
    bt_sink_srv_state_manager_next_state_t      next_state;
#endif
    bt_sink_srv_state_manager_device_flag_t     flag;
} bt_sink_srv_state_manager_device_t;

typedef struct {
    bt_sink_srv_state_t                         call_state;
    bt_sink_srv_state_t                         media_state;
    bt_sink_srv_state_t                         previous_state;
    bt_sink_srv_state_manager_device_t          *focus_device;
    bt_sink_srv_state_manager_device_t          devices[BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM];
    bt_sink_srv_state_manager_played_device_t   played_devices[BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM];
    bt_sink_srv_state_manager_play_count_t      play_count;
} bt_sink_srv_state_manager_context_t;

typedef struct {
    bt_sink_srv_state_t                         focus_state;
    bt_sink_srv_state_t                         other_state;
    bt_sink_srv_state_t                         result;
} bt_sink_srv_state_manager_remap_table_t;

typedef struct {
    bt_sink_srv_state_t                         focus_state;
    bt_sink_srv_state_t                         other_state;
    bt_sink_srv_action_t                        action;
    bt_sink_srv_action_t                        focus_action;
    bt_sink_srv_action_t                        other_action;
    bool                                        swap_focus;
    bool                                        find_last_other;
} bt_sink_srv_state_manager_dispatch_table_t;

typedef struct {
    bt_sink_srv_state_manager_sync_type_t       type;
    uint8_t                                     length;
    bt_sink_srv_state_manager_sync_direction_t  direction;
} bt_sink_srv_state_manager_sync_header_t;

typedef struct {
    bt_sink_srv_state_manager_sync_header_t     header;
    bt_sink_srv_state_t                         state;
} bt_sink_srv_state_manager_sync_state_t;

typedef struct {
    bt_sink_srv_state_manager_sync_header_t     header;
    bt_sink_srv_state_manager_focus_device_t    focus_type;
    bt_sink_srv_state_manager_device_type_t     device_type;
    bt_bd_addr_t                                address;
} bt_sink_srv_state_manager_sync_focus_t;

typedef struct {
    bt_sink_srv_state_manager_sync_header_t     header;
    bt_sink_srv_state_manager_played_device_t   played_devices[BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM];
} bt_sink_srv_state_manager_sync_played_t;

typedef struct {
    bt_sink_srv_state_manager_sync_header_t     header;
    bt_sink_srv_action_t                        action;
    uint32_t                                    parameter_length;
    uint8_t                                     parameter[1];
} bt_sink_srv_state_manager_sync_action_t;

typedef struct {
    bt_sink_srv_state_manager_sync_header_t     header;
    bt_sink_srv_state_manager_play_count_t      play_count;
} bt_sink_srv_state_manager_sync_play_count_t;

typedef struct {
    bt_sink_srv_state_manager_sync_header_t     header;
} bt_sink_srv_state_manager_sync_request_state_t;

typedef struct {
    bt_sink_srv_state_manager_focus_device_t    focus_type;
} bt_sink_srv_state_manager_rho_data_t;

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_add_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device);

void bt_sink_srv_state_manager_remove_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device);

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_type_t type,
    bt_bd_addr_t *address);

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_call_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_t call_state,
    bt_sink_srv_state_manager_device_t *skip_device);

#ifdef AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE
bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_call_state_ext(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_t call_state,
    bt_bd_addr_t *skip_address);
#endif

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_call_audio_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_sco_connection_state_t call_audio_state,
    bt_sink_srv_state_manager_device_t *skip_device);

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_media_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_t media_state,
    bt_sink_srv_state_manager_device_t *skip_device);

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_psedev(
    bt_sink_srv_state_manager_context_t *context,
    audio_src_srv_handle_t *psedev);

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_flag(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_flag_t flag);

void bt_sink_srv_state_manager_set_focus_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    bool is_add);

void bt_sink_srv_state_manager_update_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    audio_src_srv_handle_t *running_device,
    bool set_focus_device);

void bt_sink_srv_state_manager_notify_state_change_internal(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_t state,
    bool is_sync);

bt_status_t bt_sink_srv_state_manager_edr_action_handler(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_action_t action,
    void *parameter);

bt_status_t bt_sink_srv_state_manager_call_callback(
    bt_sink_srv_state_manager_event_t event,
    bt_bd_addr_t *address,
    void *parameter);

bt_status_t bt_sink_srv_state_manager_music_callback(
    bt_sink_srv_state_manager_event_t event,
    bt_bd_addr_t *address,
    void *parameter);

bt_status_t bt_sink_srv_state_manager_le_callback(
    bt_sink_srv_state_manager_event_t event,
    bt_bd_addr_t *address,
    void *parameter);

uint32_t bt_gap_le_srv_get_address_by_link_type(
    bt_addr_t *address_list,
    uint32_t list_num,
    bool is_local_addr);

void bt_sink_srv_state_manager_update_ring_ind(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    bt_sink_srv_state_t previous_state);

void bt_sink_srv_state_manager_get_device_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    bt_sink_srv_device_state_t *device_state);

bt_sink_srv_state_manager_context_t *bt_sink_srv_state_manager_get_context(void);
void bt_sink_srv_state_manager_update_edr_devices(bt_sink_srv_state_manager_context_t *context);
void bt_sink_srv_state_manager_sync_played_devices(bt_sink_srv_state_manager_context_t *context);

#if defined(MTK_AWS_MCE_ENABLE)
void bt_sink_srv_state_manager_sync_state_change(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_t state);
#endif

#ifdef MTK_AWS_MCE_ENABLE
void bt_sink_srv_state_manager_aws_mce_report_callback(bt_aws_mce_report_info_t *info);
#endif

#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
void bt_sink_srv_state_manager_rho_status(
    const bt_bd_addr_t *address,
    bt_aws_mce_role_t role,
    bt_role_handover_event_t event,
    bt_status_t status);

uint8_t bt_sink_srv_state_manager_rho_get_len(const bt_bd_addr_t *address);
bt_status_t bt_sink_srv_state_manager_rho_get_data(const bt_bd_addr_t *address, void *data);
bt_status_t bt_sink_srv_state_manager_rho_update(bt_role_handover_update_info_t *info);
bool bt_sink_srv_state_manager_is_rho_update(bt_sink_srv_state_manager_context_t *context);

#endif

#ifdef MTK_BT_CM_SUPPORT
bt_status_t bt_sink_srv_state_manager_cm_callback(
    bt_cm_event_t event_id,
    void *parameter,
    uint32_t parameter_length);
#endif

#ifdef AIR_LE_AUDIO_ENABLE
void bt_sink_srv_state_manager_gap_le_srv_callback(bt_gap_le_srv_event_t event, void *parameter);
#endif

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
void bt_sink_srv_state_manager_alloc_psedev(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device);

void bt_sink_srv_state_manager_free_psedev(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device);

void bt_sink_srv_state_manager_play_psedev(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device);

void bt_sink_srv_state_manager_stop_psedev(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device);
#endif

#ifdef __cplusplus
}
#endif

#endif

