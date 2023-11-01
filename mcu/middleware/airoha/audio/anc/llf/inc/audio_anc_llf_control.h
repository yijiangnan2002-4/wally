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

#ifndef __AUDIO_ANC_LLF_CONTROL_H__
#define __AUDIO_ANC_LLF_CONTROL_H__

#include "types.h"
#include "stdbool.h"
#include "audio_log.h"
#include "audio_anc_llf_control_internal.h"
#include "bt_aws_mce_report.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNUSED
#define UNUSED(x)  ((void)(x))
#endif
#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif




/** @brief The result notification of LLF control. */
#define LLF_MAX_CALLBACK_NUM                  (3)



typedef struct {
    llf_control_callback_t       callback_list[LLF_MAX_CALLBACK_NUM];
    llf_control_event_t          event_mask[LLF_MAX_CALLBACK_NUM];
    llf_control_callback_level_t cb_level[LLF_MAX_CALLBACK_NUM];
} llf_control_callback_service_t;

/** @brief This enum defines the audio LLF input type */
typedef enum {
    LLF_DATA_TYPE_REAR_L                 = 0,
    LLF_DATA_TYPE_RESERVED_1,    //currently unavailable
    LLF_DATA_TYPE_INEAR_L,
    LLF_DATA_TYPE_RESERVED_2,    //currently unavailable
    LLF_DATA_TYPE_TALK,
    LLF_DATA_TYPE_MIC_NUM,
    LLF_DATA_TYPE_MUSIC_VOICE = LLF_DATA_TYPE_MIC_NUM,
    LLF_DATA_TYPE_REF,
    LLF_DATA_TYPE_NUM,
} llf_data_type_t;








typedef llf_status_t (*llf_control_open_entry)(void);
typedef void (*llf_control_set_parameter_entry)(U32 is_runtime);
typedef llf_status_t (*llf_control_device)(U32 enable);
typedef void (*llf_control_dsp_callback)(S32 event, void *user_data);
typedef void (*bt_aws_mce_callback)(bt_aws_mce_report_info_t *para);
typedef llf_status_t (*llf_control_runtime_config_handler)(U8 event, S32 param, void* misc);
typedef void (*llf_control_switch_mode_callback)(void);


typedef struct {
    llf_control_open_entry open_entry;
    llf_control_open_entry close_entry;
    llf_control_set_parameter_entry set_para_entry;
    llf_control_dsp_callback dsp_callback_entry;
    bt_aws_mce_callback bt_aws_mce_report_callback_entry;
    llf_control_runtime_config_handler runtime_config_handler_entry;
    llf_control_switch_mode_callback switch_mode_callback_entry;
} llf_control_entry_t;





llf_status_t llf_callback_service(llf_type_t type, llf_control_event_t event, llf_status_t result);
llf_status_t llf_control(llf_control_event_t event, llf_control_cap_t *psap_cap);
void llf_control_set_status(llf_running_status_t running, llf_type_t type, U8 sub_mode, void *misc);
void llf_control_get_status(llf_running_status_t *running, llf_type_t *type, U8 *sub_mode, void *misc);
bool llf_control_query_share_buffer(void);
llf_status_t llf_control_register_entry(llf_type_t type, llf_control_entry_t *entry);
U32 llf_control_get_device_msk_by_data_list(U32 data_list[]);
void llf_control_query_data_wait(void);
llf_status_t llf_control_query_data_ready(void);


#ifdef __cplusplus
}
#endif
#endif  /*__AUDIO_ANC_LLF_CONTROL_H__*/

