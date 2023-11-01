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


#ifndef __APP_HEARING_AID_CONFIG_H__
#define __APP_HEARING_AID_CONFIG_H__

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "app_hearing_aid_utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define APP_HEARING_AID_EXECUTE_NONE                        0x00
#define APP_HEARING_AID_EXECUTE_ON_AGENT                    0x01
#define APP_HEARING_AID_EXECUTE_ON_PARTNER                  0x02
#define APP_HEARING_AID_EXECUTE_ON_BOTH                     0x03

typedef struct {
    uint8_t                 execute_get_where;
    uint8_t                 execute_set_where;
    bool                    notify_need_sync;
    bool                    need_execute_set_sync;
    bool (*ha_cmd_get_handler)(app_hear_through_request_t *request, uint8_t *response, uint16_t *response_len);
    bool (*ha_cmd_set_handler)(uint8_t *parameter);
    uint32_t (*ha_cmd_get_combine_response_len)();
    bool (*ha_cmd_get_combine_handler)(bool is_local_response, uint8_t *response, uint16_t response_len, uint8_t *combine_response, uint16_t *combine_response_len);
    bool (*ha_notify)(uint8_t role, uint8_t *data, uint16_t data_len, uint8_t *notify_data, uint16_t *notify_data_len);
} app_hearing_aid_execute_handler_t;

extern const app_hearing_aid_execute_handler_t app_ha_exe_handler_list[];
extern const uint8_t app_ha_exe_handler_count;

uint8_t app_hearing_aid_config_get_where_to_execute(uint8_t code, uint16_t type);
bool app_hearing_aid_config_get_notify_sync(uint16_t type);

bool app_hearing_aid_config_get_need_execute_set_cmd_sync(uint16_t type);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
#endif /* __APP_HEARING_AID_CONFIG_H__ */


