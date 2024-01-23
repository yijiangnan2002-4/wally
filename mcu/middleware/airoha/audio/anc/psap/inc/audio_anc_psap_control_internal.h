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

#ifndef _AUDIO_ANC_PSAP_CONTROL_INTERNAL_H_
#define _AUDIO_ANC_PSAP_CONTROL_INTERNAL_H_

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)

/* Includes ------------------------------------------------------------------*/
//#include "FreeRTOS.h"
#include "types.h"
#include "audio_anc_psap_control.h"
//#include "hal_audio.h"
#include "bt_aws_mce_report.h"
#include "audio_anc_llf_control.h"
#include "hal_audio_message_struct_common.h"

/* Public define -------------------------------------------------------------*/
#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

/* Public typedef ------------------------------------------------------------*/
typedef enum {
    HA_SUB_MODE_HEARING_AID = 0,
    HA_SUB_MODE_PSAP,
    HA_SUB_MODE_NUM,
} ha_scenario_sub_mode_t;


typedef struct {
    U8 drc_MPO_adjust_switch : 1;                          /**< @Value 0 @Desc 1 */
    S8 drc_MPO_adjust_value  : 7;                          /**< @Value 0 @Desc 1 */
} PACKED ha_mpo_adjust_t;

typedef enum {
    HA_TRIAL_RUN_EVENT_CHECK_CALIBRATION = 1,
    HA_TRIAL_RUN_EVENT_HLC_TABLE,
    HA_TRIAL_RUN_EVENT_USER_EQ_TABLE,
    HA_TRIAL_RUN_EVENT_VENDOR_EQ_TABLE,
} ha_trial_run_event_t;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void bt_aws_mce_report_ha_callback(bt_aws_mce_report_info_t *para);
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
void audio_anc_psap_control_check_audio_running_flag(audio_scenario_type_t type, bool is_running);
#endif
void audio_ha_switch_mode_callback(void);
void audio_anc_psap_control_set_parameter(U32 is_runtime);
audio_psap_status_t audio_anc_psap_control_runtime_config_handler(U8 event, S32 param, void* misc);
void audio_anc_psap_control_dsp_callback(S32 event, void *user_data);
audio_psap_status_t audio_anc_psap_control_disable_internal(void);
llf_status_t audio_anc_psap_control_set_mpo_adjustment(ha_mpo_adjust_t *mpo_adj);
llf_status_t audio_anc_psap_control_get_mpo_adjustment(ha_mpo_adjust_t *mpo_adj);
void audio_anc_psap_control_senario_notification(llf_scenario_change_event_t scenario, bool is_on);

#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
audio_psap_status_t audio_anc_psap_control_get_switch_status(bool *enable);
#endif

#endif /* AIR_HEARING_AID_ENABLE */

#endif /* _AUDIO_ANC_PSAP_CONTROL_INTERNAL_H_ */

