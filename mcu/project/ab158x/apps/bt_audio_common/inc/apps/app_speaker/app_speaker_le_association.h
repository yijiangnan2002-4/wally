/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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


#ifndef __APP_SPEAKER_LE_ASSOCIATION__
#define __APP_SPEAKER_LE_ASSOCIATION__

#include "bt_sink_srv.h"
#include "bt_aws_mce_srv.h"

#define BT_AWS_MCE_LE_ASSOCIATION_APP_START_PAIR                0x0
#define BT_AWS_MCE_LE_ASSOCIATION_APP_START_SCAN                0x01
#define BT_AWS_MCE_LE_ASSOCIATION_APP_START_ADV                 0x02
#define BT_AWS_MCE_LE_ASSOCIATION_APP_UNGROUP                   0x03
typedef uint8_t app_speaker_le_ass_action_t;

#define BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_SCAN                (BT_MODULE_CUSTOM_LE_ASSOCIATION | 0x01)
#define BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_RESET               (BT_MODULE_CUSTOM_LE_ASSOCIATION | 0x02)
#define BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_UNGROUP             (BT_MODULE_CUSTOM_LE_ASSOCIATION | 0x03)
#define BT_AWS_MCE_LE_ASSOCIATION_APP_TIMER_ADV                 (BT_MODULE_CUSTOM_LE_ASSOCIATION | 0x04)

#define LE_ASSOCIATION_EVENT_ASSOCIATION_DONE                   0x01
#define LE_ASSOCIATION_EVENT_SCAN_TIMEOUT                       0x02
#define LE_ASSOCIATION_EVENT_ADV_TIMEOUT                        0x03
#define LE_ASSOCIATION_EVENT_AWS_MCE_UNGROUP                    0x04
#define LE_ASSOCIATION_EVENT_TIMER_AWS_CONNECT                  0x05
#define LE_ASSOCIATION_EVENT_TIMER_ASSOCIATION                  0x06
#define LE_ASSOCIATION_EVENT_SWITCH_SINGLE                      0x07
#define LE_ASSOCIATION_EVENT_SWITCH_SINGLE_DONE                 0x08



void app_speaker_le_ass_init(void);

void app_speaker_le_ass_action(app_speaker_le_ass_action_t action, void *buff);

void app_speaker_le_ass_set_param(bt_aws_mce_role_t role, bt_aws_mce_srv_mode_t mode);

bt_aws_mce_role_t app_speaker_le_ass_get_target_role(void);

bool app_speaker_le_ass_switch_to_single(void);

void app_speaker_le_ass_get_aws_key(bt_key_t *aws_key);
void app_speaker_le_ass_set_aws_key(bt_key_t aws_key);

#endif /* __APP_SPEAKER_LE_ASSOCIATION__ */
