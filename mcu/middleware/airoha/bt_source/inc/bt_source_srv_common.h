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
#ifndef __BT_SOURCE_SRV_COMMON_H__
#define __BT_SOURCE_SRV_COMMON_H__
#include "bt_source_srv.h"

typedef struct {
    bt_source_srv_port_t             port;
    uint32_t                         sample_rate;
    uint32_t                         sample_size;
    uint32_t                         sample_channel;
} bt_source_srv_common_audio_port_context_t;


#define BT_SOURCE_SRV_COMMON_PORT_ACTION_NONE        0x00
#define BT_SOURCE_SRV_COMMON_PORT_ACTION_OPEN        0x01
#define BT_SOURCE_SRV_COMMON_PORT_ACTION_CLOSE       0x02
#define BT_SOURCE_SRV_COMMON_PORT_ACTION_UPDATE      0x03
typedef uint8_t bt_source_srv_common_port_action_t;

bt_status_t bt_source_srv_common_init(void);

bt_status_t bt_source_srv_common_audio_find_port_context(bt_source_srv_port_t port, bt_source_srv_common_audio_port_context_t *context);

bt_status_t bt_source_srv_common_audio_get_default_port_parameter(bt_source_srv_port_t port, bt_source_srv_common_audio_port_context_t *context);

bt_status_t bt_source_srv_common_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length);

bool bt_source_srv_common_audio_port_is_valid(bt_source_srv_port_t port);

bt_source_srv_t bt_source_srv_common_get_playing_device(void);

bt_status_t bt_source_srv_common_switch_sniff_mode(bt_bd_addr_t *dev_addr,bool is_allow);

#ifdef AIR_FEATURE_SOURCE_MHDT_SUPPORT
bt_status_t bt_source_srv_common_switch_mhdt(bt_bd_addr_t *remote_address, bool is_enable);
#endif

#endif
