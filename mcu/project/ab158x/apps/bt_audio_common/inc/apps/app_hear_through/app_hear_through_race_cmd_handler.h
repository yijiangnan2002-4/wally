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


#ifndef __APP_HEAR_THROUGH_RACE_CMD_HANDLER_H__
#define __APP_HEAR_THROUGH_RACE_CMD_HANDLER_H__

#include "race_cmd.h"
#include "stdint.h"

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE

#define RACE_ID_APP_HEAR_THROUGH_CONFIG      0x2c87

#define APP_HEAR_THROUGH_CMD_OP_CODE_NONE                0x00
#define APP_HEAR_THROUGH_CMD_OP_CODE_SET                 0x01
#define APP_HEAR_THROUGH_CMD_OP_CODE_GET                 0x02
typedef uint8_t app_hear_through_race_cmd_op_code;

typedef struct {
    app_hear_through_race_cmd_op_code     op_code;
    uint16_t                    op_type;
    uint8_t                     op_parameter[0];
} __attribute__((packed)) app_hear_through_request_t;

void *app_hear_through_race_cmd_handler(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id);

void app_hear_through_race_cmd_send_get_response(uint16_t config_type, uint8_t result, uint8_t *data, uint16_t data_len);

void app_hear_through_race_cmd_send_set_response(uint16_t config_type, uint8_t result);

void app_hear_through_race_cmd_send_notification(uint16_t config_type, uint8_t *data, uint16_t data_len);

#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#endif /* __APP_HEAR_THROUGH_RACE_CMD_HANDLER_H__ */

