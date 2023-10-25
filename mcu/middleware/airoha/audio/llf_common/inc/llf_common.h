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

#ifndef __LLF_COMMON_H__
#define __LLF_COMMON_H__

#include "types.h"
#include "stdbool.h"
#include "audio_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNUSED
#define UNUSED(x)  ((void)(x))
#endif
#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

/** @brief The atci cmd source of the llf_control_command_handler. */
#define AUDIO_LLF_CONTROL_SOURCE_FROM_ATCI         (1)
#define AUDIO_LLF_CONTROL_SOURCE_FROM_AM           (2)

/** @brief This enum defines the LLF return types */
typedef enum {
    LLF_STATUS_NONE = -2,
    LLF_STATUS_FAIL = -1,    /**< The psap func return fail.   */
    LLF_STATUS_SUCCESS = 0,   /**< The psap func return success.   */
    LLF_STATUS_CANCEL = 1,   /**< The psap func return cancel.   */
} llf_status_t, audio_psap_status_t, audio_vivid_pt_status_t;

/** @brief LLF control event values. */
typedef enum {
    LLF_CONTROL_EVENT_ON                 = 1 << 0,  /**< The event to enable llf.   */
    LLF_CONTROL_EVENT_OFF                = 1 << 1,  /**< The event to disable llf.   */
    LLF_CONTROL_EVENT_SUSPEND            = 1 << 2,  /**< The event to suspend llf.   */
    LLF_CONTROL_EVENT_RESUME             = 1 << 3,  /**< The event to resume llf.   */
    LLF_CONTROL_EVENT_RUNTIME_CONFIG     = 1 << 4,  /**< The event to config llf.   */
    LLF_CONTROL_EVENT_RESET              = 1 << 5,  /**< The event to reset llf.   */
    LLF_CONTROL_EVENT_UPDATE_PARA        = 1 << 6,  /**< The event to update llf.   */
} llf_control_event_t;

/** @brief This enum defines the audio LLF subset */
typedef enum {
    LLF_TYPE_HEARING_AID                 = 0,
    LLF_TYPE_VIVID_PT,
    LLF_TYPE_RESERVED_1,
    LLF_TYPE_RESERVED_2,
    LLF_TYPE_SAMPLE,
    LLF_TYPE_ALL,
    LLF_TYPE_DUMMY                       = 0xFFFFFFFF,
} llf_type_t;

/** @brief This enum defines the scenario event */
typedef enum {
    /*UL */
    LLF_SCENARIO_CHANGE_UL_CALL    = 0,
    LLF_SCENARIO_CHANGE_UL_VA      = 1,
    LLF_SCENARIO_CHANGE_UL_MAX     = LLF_SCENARIO_CHANGE_UL_VA,

    /*DL*/
    LLF_SCENARIO_CHANGE_DL_VP      = 8,
    LLF_SCENARIO_CHANGE_DL_A2DP    = 9,
    LLF_SCENARIO_CHANGE_DL_CALL    = 10,
    LLF_SCENARIO_CHANGE_DL_MAX     = LLF_SCENARIO_CHANGE_DL_CALL,

} llf_scenario_change_event_t;

/** @brief The source of the llf_control_command_handler. */
typedef uint8_t llf_control_user_type_t;

/** @brief This function send open/close stream request*/
typedef llf_status_t (*stream_handler_t)(bool enable);

typedef void (*llf_control_callback_t)(llf_type_t type, llf_control_event_t event, llf_status_t result);

typedef enum {
    LLF_MAX_CALLBACK_LEVEL_ALL = 0,
    LLF_MAX_CALLBACK_LEVEL_SUCCESS_ONLY,
} llf_control_callback_level_t;

/** @brief This structure support the audio LLF path setting */
typedef struct {
    llf_type_t type;
    U8 sub_mode;
    U32 frame_length;
    U32 buffer_length;
    stream_handler_t stream_handler;
    S32 delay_time; //ms
    U32 param;
    U32 param_len;
} llf_control_cap_t;


#ifdef __cplusplus
}
#endif
#endif  /*__LLF_COMMON_H__*/

