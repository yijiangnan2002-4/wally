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


#ifndef __BT_SOURCE_SRV_A2DP_H__
#define __BT_SOURCE_SRV_A2DP_H__


#include "bt_a2dp.h"
#include "bt_connection_manager.h"
#include "bt_source_srv_avrcp.h"
#include "FreeRTOSConfig.h"
#include "bt_source_srv.h"
#include "bt_sink_srv_ami.h"
#include "bt_utils.h"
#include "bt_source_srv_music_internal.h"
#include "bt_source_srv_music_psd_manager.h"
#include "bt_source_srv_music_pseduo_dev.h"
#include "bt_callback_manager.h"
#include "bt_avm.h"
#include "bt_timer_external.h"
#include "bt_source_srv_utils.h"

void bt_source_srv_a2dp_init(void);
bt_status_t bt_source_srv_a2dp_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);
bt_status_t bt_source_srv_a2dp_connect_handler(bt_cm_profile_service_handle_t type, void *data);
bt_status_t bt_source_srv_a2dp_action_handler(uint32_t action, void* param, uint32_t length);
bt_status_t bt_source_srv_a2dp_get_init_params(bt_a2dp_init_params_t *param);
bt_status_t bt_source_srv_a2dp_audio_port_update(bt_source_srv_port_t audio_port, bt_source_srv_music_port_action_t action);
bt_status_t bt_source_srv_a2dp_common_action_handler(uint32_t action, void* param, uint32_t length);

#endif

