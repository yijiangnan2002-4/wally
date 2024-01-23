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

#ifndef __BT_SINK_SRV_STATE_MANAGER_H__
#define __BT_SINK_SRV_STATE_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bt_sink_srv.h"
#include "audio_src_srv.h"

#define BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_INVALID       BT_SINK_SRV_DEVICE_INVALID
#define BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR           BT_SINK_SRV_DEVICE_EDR
#define BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE            BT_SINK_SRV_DEVICE_LE
typedef uint8_t bt_sink_srv_state_manager_device_type_t;

#define BT_SINK_SRV_STATE_MANAGER_EVENT_GET_STATE           0x00
#define BT_SINK_SRV_STATE_MANAGER_EVENT_GET_CALL_DEVICE     0x01
#define BT_SINK_SRV_STATE_MANAGER_EVENT_GET_MEDIA_DEVICE    0x02
#define BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT  0x03
#define BT_SINK_SRV_STATE_MANAGER_EVENT_GET_PLAY_COUNT      0x04
typedef uint8_t bt_sink_srv_state_manager_event_t;

#define BT_SINK_SRV_STATE_MANAGER_DEVICE_MASK_NONE          0x00
#define BT_SINK_SRV_STATE_MANAGER_DEVICE_MASK_CALL          0x01
#define BT_SINK_SRV_STATE_MANAGER_DEVICE_MASK_MEDIA         0x02
typedef uint8_t bt_sink_srv_state_manager_device_mask_t;

#define BT_SINK_SRV_STATE_MANAGER_PLAY_COUNT_INVALID        0x0000
#define BT_SINK_SRV_STATE_MANAGER_PLAY_COUNT_MIN            0x0001
#define BT_SINK_SRV_STATE_MANAGER_PLAY_COUNT_MAX            0xFFFF
typedef uint16_t bt_sink_srv_state_manager_play_count_t;

typedef struct {
    bt_sink_srv_state_manager_device_type_t         type;
    bt_bd_addr_t                                    address;
    bt_sink_srv_state_manager_device_mask_t         mask;
} bt_sink_srv_state_manager_played_device_t;

typedef struct {
    bt_sink_srv_device_t                            type;
    bt_bd_addr_t                                    address;
} bt_sink_srv_state_manager_get_reject_config_t;

typedef struct {
    bt_sink_srv_device_t                            type;
    bt_bd_addr_t                                    address;
    audio_src_srv_handle_t                          *suspend_handle;
} bt_sink_srv_state_manager_get_suspend_config_t;

typedef struct {
    bt_sink_srv_device_t                            type;
    bt_bd_addr_t                                    address;
} bt_sink_srv_state_manager_get_resume_config_t;

typedef union {
    bt_sink_srv_state_manager_get_reject_config_t   get_reject_config;
    bt_sink_srv_state_manager_get_suspend_config_t  get_suspend_config;
    bt_sink_srv_state_manager_get_resume_config_t   get_resume_config;
} bt_sink_srv_state_manager_get_config_t;

uint32_t bt_sink_srv_state_manager_get_played_device_list(
    bt_sink_srv_state_manager_played_device_t *list,
    uint32_t list_number);

void bt_sink_srv_state_manager_notify_ring_ind(
    bt_sink_srv_state_manager_device_type_t type,
    bt_bd_addr_t *address,
    bool play);

void bt_sink_srv_state_manager_notify_state_change(
    bt_sink_srv_state_manager_device_type_t type,
    bt_sink_srv_state_t state);

void bt_sink_srv_state_manager_notify_call_audio_state(
    bt_sink_srv_state_manager_device_type_t type,
    bt_bd_addr_t *address,
    bt_sink_srv_sco_connection_state_t call_audio_state);

bt_sink_srv_allow_result_t bt_sink_srv_state_manager_psedev_compare(
    audio_src_srv_handle_t *current,
    audio_src_srv_handle_t *coming);

bt_status_t bt_sink_srv_state_manager_get_config(
    bt_sink_srv_get_config_t type,
    bt_sink_srv_state_manager_get_config_t *param,
    bt_sink_srv_config_t *config);

void bt_sink_srv_state_manager_initialize(void);
bt_sink_srv_state_manager_play_count_t bt_sink_srv_state_manager_get_play_count(void);
void bt_sink_srv_state_manager_running_psedev_change(audio_src_srv_handle_t *running);
bt_status_t bt_sink_srv_state_manager_action_handler(bt_sink_srv_action_t action, void *parameter);

#ifdef __cplusplus
}
#endif

#endif

