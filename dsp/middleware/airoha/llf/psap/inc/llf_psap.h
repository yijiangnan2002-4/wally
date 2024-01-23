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

#ifndef _LLF_PSAP_H_
#define _LLF_PSAP_H_

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "source_inter.h"
#include "sink_inter.h"
#include "stream_llf.h"

/* Public define -------------------------------------------------------------*/


/* Public typedef ------------------------------------------------------------*/
typedef enum {
    HA_SUB_MODE_HEARING_AID = 0,
    HA_SUB_MODE_PSAP,
    HA_SUB_MODE_NUM,
} ha_scenario_sub_mode_t;



/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
stream_feature_list_t* psap_features_get_list(llf_type_t scenario_id, U32 sub_id);
void psap_features_init(audio_llf_stream_ctrl_t *ctrl);
void psap_features_deinit(SOURCE source, SINK sink);
void psap_suspend(audio_llf_stream_ctrl_t *ctrl);
void psap_resume(audio_llf_stream_ctrl_t *ctrl);
void psap_runtime_config(audio_llf_runtime_config_t *param);
void psap_set_dl_state(bool *dl_state);

#endif /* AIR_HEARING_AID_ENABLE */

#endif /* _LLF_PSAP_H_ */


