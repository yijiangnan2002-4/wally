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

/**
 * File: voice_prompt_local.h
 *
 * Description: This file defines the interface of voice_prompt_local.c.
 *
 */

#ifndef VOICE_PROMPT_LOCAL_H
#define VOICE_PROMPT_LOCAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "voice_prompt_api.h"
#include "syslog.h"

//log_create_module(VOICE_PROMPT_APP, PRINT_LEVEL_INFO);

#define VP_LOG_I(_message,...)       LOG_I(VOICE_PROMPT, (_message), ##__VA_ARGS__)

#define VP_LOG_MSGID_E(msg, ...)              LOG_MSGID_E(VOICE_PROMPT, msg, ##__VA_ARGS__)
#define VP_LOG_MSGID_W(msg, ...)              LOG_MSGID_W(VOICE_PROMPT, msg, ##__VA_ARGS__)
#define VP_LOG_MSGID_I(msg, ...)              LOG_MSGID_I(VOICE_PROMPT, msg, ##__VA_ARGS__)
#define VP_LOG_MSGID_D(msg, ...)              LOG_MSGID_D(VOICE_PROMPT, msg, ##__VA_ARGS__)

voice_prompt_status_t voice_prompt_local_play(uint32_t vp_index, uint32_t target_gpt);
voice_prompt_status_t voice_prompt_local_stop(uint32_t tar_gpt);

#ifdef __cplusplus
}
#endif

#endif /* VOICE_PROMPT_LOCAL_H */

