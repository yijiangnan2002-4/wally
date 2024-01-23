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

#ifndef __BT_SOURCE_SRV_HFP_H__
#define __BT_SOURCE_SRV_HFP_H__
#include "bt_source_srv.h"
#include "bt_hfp.h"
#include "bt_source_srv_call.h"


#define BT_SOURCE_SRV_HFP_LINK_NUM                         0x02

#define BT_SOURCE_SRV_HFP_ACTION_MASK                      0x000000FF

#define BT_SOURCE_SRV_HFP_CMD_LENGTH                       255

#define BT_SOURCE_SRV_HFP_RING_ALERTING_TIMEOUT            3000

#define BT_SOURCE_SRV_HFP_AUDIO_STOP_TIMEOUT               20

#define BT_SOURCE_SRV_HFP_PHONE_NUMBER_NATIONAL            129
#define BT_SOURCE_SRV_HFP_PHONE_NUMBER_INTERNATIONAL       145

#define BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTED                  0x0001
#define BT_SOURCE_SRV_HFP_FLAG_PROFILE_DISCONNECT             0x0002
#define BT_SOURCE_SRV_HFP_FLAG_CLI_ENABLED                    0x0004
#define BT_SOURCE_SRV_HFP_FLAG_CALL_WAITING_ENABLED           0x0008
#define BT_SOURCE_SRV_HFP_FLAG_CALL_AG_INDICATOR_ENABLED      0x0010
#define BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECTING                 0x0020
#define BT_SOURCE_SRV_HFP_FLAG_SCO_DISCONNECT                 0x0040
#define BT_SOURCE_SRV_HFP_FLAG_SCO_RECONNECT                  0x0080
#define BT_SOURCE_SRV_HFP_FLAG_RING_START                     0x0100
#define BT_SOURCE_SRV_HFP_FLAG_MIC_MUTED                      0x0200
#define BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECT                    0x0800
#ifdef AIR_FEATURE_SOURCE_MHDT_SUPPORT
#define BT_SOURCE_SRV_HFP_FLAG_MHDT_EXITED                    0x1000
#endif
typedef uint16_t bt_source_srv_hfp_flag_t;

/* BQB test */
#define BT_SOURCE_SRV_HFP_BQB_FLAG_NONE                                 0x00
#define BT_SOURCE_SRV_HFP_BQB_FLAG_VIRTUAL_CALL                         0x01
#define BT_SOURCE_SRV_HFP_BQB_FLAG_DIALING_CALL_ERROR                   0x02
#define BT_SOURCE_SRV_HFP_BQB_FLAG_TWC                                  0x04
#define BT_SOURCE_SRV_HFP_BQB_FLAG_CODEC_CONNECTION                     0x08
typedef uint8_t bt_source_srv_hfp_bqb_flag_t;

#define BT_SOURCE_SRV_HFP_FLAG_SCO_CONNECT_WITH_SLC                     0x0001
#define BT_SOURCE_SRV_HFP_FLAG_SLC_GET_CALL_INFO_COMPLETE               0x0002
#define BT_SOURCE_SRV_HFP_FLAG_SPEAKER_MUTED                            0x0004
typedef uint16_t bt_source_srv_hfp_common_flag_t;

#define BT_SOURCE_SRV_HFP_ESCO_STATE_DISCONNECTED                        0x00
#define BT_SOURCE_SRV_HFP_ESCO_STATE_CONNECTING                          0x01
#define BT_SOURCE_SRV_HFP_ESCO_STATE_CONNECTED                           0x02
#define BT_SOURCE_SRV_HFP_ESCO_STATE_DISCONNECTING                       0x03
typedef uint8_t bt_source_srv_hfp_esco_state_t;

#define BT_SOURCE_SRV_HFP_FLAG_IS_SET(context, flag)       (context->flags & (flag))
#define BT_SOURCE_SRV_HFP_SET_FLAG(context, flag)          (context->flags |= (flag))
#define BT_SOURCE_SRV_HFP_REMOVE_FLAG(context, flag)       (context->flags &= ~(flag))


#define BT_SOURCE_SRV_HFP_CALL_TRANSFER_NONE                        0x00
#define BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALL                        0x01
#define BT_SOURCE_SRV_HFP_CALL_TRANSFER_CALLSETUP                   0x02
#define BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD                        0x04
#define BT_SOURCE_SRV_HFP_CALL_TRANSFER_HELD_ACTIVE_SWAPPED         0x08
typedef uint8_t bt_source_srv_hfp_call_transfer_t;

typedef enum {
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_ACTIVE = 0,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_LOCAL_HELD,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_DIALING,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_ALERTING,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_INCOMMING,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_WAITING,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_INVAILD,
    BT_SOURCE_SRV_HFP_CLCC_CALL_STATE_MAX,
} bt_source_srv_hfp_clcc_call_state_t;

typedef struct {
    bt_source_srv_hfp_call_transfer_t call_transfer;
    uint8_t                           call_status;
    uint8_t                           callsetup_status;
    uint8_t                           held_status;
} bt_source_srv_hfp_call_transfer_info_t;


bt_status_t bt_source_srv_hfp_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);

bt_status_t bt_source_srv_hfp_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length);

void bt_source_srv_hfp_event_notify(bt_source_srv_event_t event_id, void *parameter, uint32_t length);

bt_status_t bt_source_srv_hfp_transfer_call_state(bt_source_srv_call_state_t previous_state, bt_source_srv_call_state_t new_state);

bt_status_t bt_source_srv_hfp_init(bt_source_srv_hfp_init_parameter_t *hfp_init_param);

bt_status_t bt_source_srv_hfp_audio_port_update(bt_source_srv_port_t audio_port, bt_source_srv_call_port_action_t action);

bt_source_srv_hfp_codec_t bt_source_srv_hfp_get_playing_device_codec(void);

/* for AG BQB test use */
bt_status_t bt_source_srv_hfp_set_bqb_flag(bt_source_srv_hfp_bqb_flag_t flag);
bt_status_t bt_source_srv_hfp_remove_bqb_flag(bt_source_srv_hfp_bqb_flag_t flag);
bt_source_srv_hfp_bqb_flag_t bt_source_srv_hfp_get_bqb_flag(void);
#endif
