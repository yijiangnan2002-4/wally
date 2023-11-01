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

#ifndef __BT_SINK_SRV_CALL_H__
#define __BT_SINK_SRV_CALL_H__

#include "bt_sink_srv.h"
#include "bt_sink_srv_ami.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef MTK_BT_CM_SUPPORT
void bt_sink_srv_call_init(void);
#endif

#ifdef AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
#define BT_SINK_SRV_CALL_AUDIO_SWITCH       0x00000001
#endif

typedef uint32_t bt_sink_srv_call_flag_t;

typedef struct {
    bt_sink_srv_call_flag_t      flags;
} bt_sink_srv_call_context_t;

#define BT_SINK_SRV_CALL_SET_FLAG(context, flag) do { \
    (context->flags) |= (flag); \
} while(0);

#define BT_SINK_SRV_CALL_REMOVE_FLAG(context, flag) do { \
    (context->flags) &= ~(flag); \
} while(0);

#define BT_SINK_SRV_CALL_IS_FLAG_EXIST(context, flag)    (context->flags & flag)

bt_status_t bt_sink_srv_call_action_handler(bt_sink_srv_action_t action, void *parameters);
bt_status_t bt_sink_srv_call_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);
bt_status_t bt_sink_srv_call_get_device_state(bt_sink_srv_device_state_t *device_state);

bt_sink_srv_call_context_t *bt_sink_srv_call_get_context(void);
bt_sink_srv_am_id_t bt_sink_srv_call_get_playing_audio_id(void);

#ifdef AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
bt_status_t bt_sink_srv_call_audio_switch_handle(bool value, void *playing_dev, void *paly_idle_dev);
#endif
#ifdef __cplusplus
}
#endif

#endif /* __BT_SINK_SRV_CALL_H__ */
