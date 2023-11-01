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

#ifndef __APP_HEARING_AID_AWS_H__
#define __APP_HEARING_AID_AWS_H__

#if (defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)) && defined(AIR_TWS_ENABLE)

#include "stdbool.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define APP_HEARING_AID_OP_COMMAND_NONE                         0x00
#define APP_HEARING_AID_OP_COMMAND_CONTROL_HA                   0x01
#define APP_HEARING_AID_OP_COMMAND_SWITCH_BF                    0x02
#define APP_HEARING_AID_OP_COMMAND_SWITCH_AEA_CONFIGURE         0x03
#define APP_HEARING_AID_OP_COMMAND_SWITCH_MASTER_MIC_CHANNEL    0x04
#define APP_HEARING_AID_OP_COMMAND_SWITCH_TUNING_MODE           0x05
#define APP_HEARING_AID_OP_COMMAND_CHANGE_LEVEL                 0x06
#define APP_HEARING_AID_OP_COMMAND_CHANGE_VOLUME                0x07
#define APP_HEARING_AID_OP_COMMAND_CHANGE_MODE                  0x08
#define APP_HEARING_AID_OP_COMMAND_MODIFY_MODE_INDEX_REQ        0x09
#define APP_HEARING_AID_OP_COMMAND_SET_USER_SWITCH              0x0A
#define APP_HEARING_AID_OP_COMMAND_PLAY_SYNC_VP                 0x0B
#define APP_HEARING_AID_OP_COMMAND_REQUEST_POWER_OFF            0x0C
#define APP_HEARING_AID_OP_COMMAND_RACE_CMD_REQUEST             0x0D
#define APP_HEARING_AID_OP_COMMAND_RACE_CMD_RESPONSE            0x0E
#define APP_HEARING_AID_OP_COMMAND_RSSI_OPERATION               0x0F

typedef struct {
    uint8_t     l_index;
    uint8_t     r_index;
}  __attribute__((packed)) app_hearing_aid_aws_lr_index_change_t;

typedef struct {
    uint8_t     l_index;
    uint8_t     r_index;
    bool        up;
}  __attribute__((packed)) app_hearing_aid_aws_lr_index_with_direction_change_t;

typedef struct {
    uint8_t     index;
}  __attribute__((packed)) app_hearing_aid_aws_index_change_t;

typedef struct {
    uint8_t     vp_index;
}  __attribute__((packed)) app_hearing_aid_aws_sync_vp_play_t;

typedef struct {
    uint8_t             which;
    bool                from_key;
    bool                mix_table_need_execute;
    bool                is_origin_on;
    bool                mix_table_to_enable;
    bool                drc_to_enable;
}  __attribute__((packed)) app_hearing_aid_aws_sync_operate_ha_t;

typedef struct {
    int8_t              new_rssi;
} __attribute__((packed)) app_hearing_aid_aws_rssi_operate_t;

typedef struct {
    uint8_t             cmd_op_code;
    uint16_t            cmd_op_type;
    uint8_t             cmd_op_data_len;
    uint8_t             cmd_op_data[0];
} __attribute__((packed)) app_hearing_aid_aws_race_cmd_op_response_t;

void app_hearing_aid_aws_init();

void app_hearing_aid_aws_deinit();

void app_hearing_aid_aws_process_data(uint32_t aws_id, uint8_t *aws_data, uint32_t aws_data_len);

void app_hearing_aid_aws_send_middleware_configuration_sync_request();

void app_hearing_aid_aws_sync_agent_middleware_configuration_to_partner();

void app_hearing_aid_aws_sync_agent_user_configuration_to_partner();

void app_hearing_aid_aws_reset_middleware_configuration();

bool app_hearing_aid_aws_send_operate_command(uint8_t code, uint8_t *buf, uint16_t buf_len, bool need_execute_locally, uint32_t delay_ms);

bool app_hearing_aid_aws_send_notification(uint8_t role, uint32_t code, uint8_t *notify_data, uint16_t notify_data_len);

bool app_hearing_aid_aws_is_connected();

bool app_hearing_aid_aws_set_vp_streaming_state(bool streaming);

void app_hearing_aid_aws_sync_agent_app_info_to_partner(uint8_t *data, uint32_t data_len);

void app_hearing_aid_aws_handle_sync_execute_event(uint32_t event_id,
                                                    void *extra_data,
                                                    uint32_t data_len);

void app_hearing_aid_aws_handle_connected(bool need_middleware_sync);

void app_hearing_aid_aws_handle_disconnected();

void app_hearing_aid_aws_send_rssi_reading_event();

void app_hearing_aid_aws_remove_rssi_reading_event();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* (AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE) && AIR_TWS_ENABLE */

#endif /* __APP_HEARING_AID_AWS_H__ */

