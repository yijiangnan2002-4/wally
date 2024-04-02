/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#ifndef __APP_HEAR_THROUGH_ACTIVITY_H__

#include "ui_shell_activity.h"
#include "stdint.h"

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define APP_HEAR_THROUGH_EVENT_ID_BASE                          0x0000
#define APP_HEAR_THROUGH_EVENT_ID_RACE_CMD                      0x0001
#define APP_HEAR_THROUGH_EVENT_ID_POWER_ON_TO_OPERATE_HT        0x0002
#define APP_HEAR_THROUGH_EVENT_ID_BLE_ADV_TIMEOUT               0x0003
#define APP_HEAR_THROUGH_EVENT_ID_RACE_CONNECTED                0x0004
#define APP_HEAR_THROUGH_EVENT_ID_RACE_DISCONNECTED             0x0005
#define APP_HEAR_THROUGH_EVENT_ID_VP_STREAMING_BEGIN            0x0006
#define APP_HEAR_THROUGH_EVENT_ID_VP_STREAMING_END              0x0007
#define APP_HEAR_THROUGH_EVENT_ID_MIDDLEWARE_CONTROL_CALLBACK   0x0008

bool app_hear_through_activity_proc(ui_shell_activity_t *self,
                                    uint32_t event_group,
                                    uint32_t event_id,
                                    void *extra_data,
                                    size_t data_len);

void app_hear_through_activity_switch_to_hear_through();

void app_hear_through_activity_switch_ambient_control();

void app_hear_through_activity_handle_anc_switched(bool need_sync, bool anc_enable);

void app_hear_through_activity_power_on_vp_start_to_play();
extern  bool app_hear_through_switch_on_off(bool need_store, bool enable);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#endif /* __APP_HEAR_THROUGH_ACTIVITY_H__ */

