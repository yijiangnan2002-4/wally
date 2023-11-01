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

#ifndef _AUDIO_ANC_LLF_CONTROL_INTERNAL_H_
#define _AUDIO_ANC_LLF_CONTROL_INTERNAL_H_

/* Includes ------------------------------------------------------------------*/
//#include "FreeRTOS.h"
#include "types.h"
#include "llf_common.h"
#include "bt_aws_mce_report.h"

/* Public define -------------------------------------------------------------*/



/* Public typedef ------------------------------------------------------------*/




/** @brief This enum defines the audio LLF running status */
typedef enum {
    LLF_RUNNING_STATUS_START = 0,
    LLF_RUNNING_STATUS_RUNNING,
    LLF_RUNNING_STATUS_RESUME,
    LLF_RUNNING_STATUS_SUSPEND,
    LLF_RUNNING_STATUS_RESET,
    LLF_RUNNING_STATUS_SWITCH,
    LLF_RUNNING_STATUS_STOP,
    LLF_RUNNING_STATUS_CLOSE,
    LLF_RUNNING_STATUS_INCASE_SUSPEND,
} llf_running_status_t;




typedef struct {
    llf_running_status_t running_state;
    U8 earbuds_ch;
    U8 sub_mode;
    U8 dsp_share_buffer_write_done;

    llf_type_t type;
    U32 frame_length;
    U32 buffer_length;
    stream_handler_t stream_handler;
    U32 device_mask;

    U8 next_sub_mode;

    llf_type_t next_type;
    U32 next_frame_length;
    U32 next_buffer_length;
    stream_handler_t next_stream_handler;
    U32 next_device_mask;

} llf_control_t;

typedef struct {
    llf_type_t type;
    U8 sub_mode;
    U8 config_event;
    U32 setting;
} llf_control_runtime_config_t;

typedef struct {
    U32 event_id;
    U32 param_len;
    U32 param;
} aws_mce_report_llf_param_t;

typedef enum {
    /*UL */
    LLF_SCENARIO_CHANGE_MASK_UL_CALL    = 1 << LLF_SCENARIO_CHANGE_UL_CALL,
    LLF_SCENARIO_CHANGE_MASK_UL_VA      = 1 << LLF_SCENARIO_CHANGE_UL_VA,

    /*DL*/
    LLF_SCENARIO_CHANGE_MASK_DL_A2DP    = 1 << LLF_SCENARIO_CHANGE_DL_A2DP,

} llf_scenario_change_event_mask_t;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
bool llf_sharebuf_semaphore_take(void);
void llf_sharebuf_semaphore_give(void);
bt_status_t llf_control_send_aws_mce_param(U32 event_id, U32 data_len, U8* p_data);


void llf_control_set_parameter(U16 nvkey_id);
void llf_control_set_multi_parameters(U16 nvkey_id_list[], U8 key_num);
void llf_control_realtime_set_parameter(U16 nvkey_id, U32 nvkey_length, U8* data);

void llf_control_init(void);
llf_status_t llf_control_suspend(void);
llf_status_t llf_control_resume(void);
llf_status_t llf_control_command_handler(llf_control_user_type_t source, llf_control_event_t event, void* command);
llf_status_t llf_control_register_callback(llf_control_callback_t callback, llf_control_event_t event_mask, llf_control_callback_level_t level);

#endif /* _AUDIO_ANC_LLF_CONTROL_INTERNAL_H_ */

