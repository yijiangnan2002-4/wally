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

#ifndef __APP_SWIFT_CUST_PAIR_H__
#define __APP_SWIFT_CUST_PAIR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) && defined(AIR_SPEAKER_ENABLE)
#error "For Speaker, not enable APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE"
#endif

enum {
    APP_SWIFT_PAIR_EVENT_RESTART_ADV             = 0,
    APP_SWIFT_PAIR_EVENT_SWIFT_ADV_TIMEOUT,
    APP_SWIFT_PAIR_EVENT_SWIFT_QUICK_TIMEOUT,

    APP_SWIFT_PAIR_EVENT_CUST_SWITCH_TYPE        = 0x10,
    APP_SWIFT_PAIR_EVENT_CUST_ADV_TIMEOUT,
    APP_SWIFT_PAIR_EVENT_CUST_ADV_STOP,
    APP_SWIFT_PAIR_EVENT_CUST_GFP_ADV_RESTORE,
    APP_SWIFT_PAIR_EVENT_CUST_LEA_ADV_ADJUST,
    APP_SWIFT_PAIR_EVENT_CUST_TRY_EDR_CONN,
    APP_SWIFT_PAIR_EVENT_CUST_CONN_TIMER_BASE    = 0x1000,
    APP_SWIFT_PAIR_EVENT_CUST_DISCONNECT_BASE    = 0x2000,
};

void       app_swift_pair_restart_adv(void);

uint16_t   app_swift_pair_get_conn_handle(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_SWIFT_CUST_PAIR_H__ */

