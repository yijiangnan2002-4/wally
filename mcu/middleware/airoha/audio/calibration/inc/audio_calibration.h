/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#ifndef __AUDIO_CALIBRATION_H__
#define __AUDIO_CALIBRATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "syslog.h"
#include "race_cmd.h"
#include "hal_audio.h"

#if defined(AIR_COMPONENT_CALIBRATION_SYNC_WITH_ANC_ENABLE)
#include "anc_control_api.h"
#endif
#define PACKED  __attribute__((packed))

/******************************************************************************
 * Constant Definitions
 ******************************************************************************/

/******************************************************************************
 * Type Definitions
 ******************************************************************************/
typedef enum {
    AUDIO_CALIBRATION_EXECUTION_FAIL                        = -1,
    AUDIO_CALIBRATION_EXECUTION_SUCCESS                     =  0,
    AUDIO_CALIBRATION_EXECUTION_CANCELLED                   =  1,
} audio_calibration_result_t;

typedef enum {
    AUDIO_CALIBRATION_COMPONENT_OUTPUT_MIN,
    AUDIO_CALIBRATION_COMPONENT_DAC_L = AUDIO_CALIBRATION_COMPONENT_OUTPUT_MIN,
    AUDIO_CALIBRATION_COMPONENT_DAC_R,
    AUDIO_CALIBRATION_COMPONENT_OUTPUT_MAX = AUDIO_CALIBRATION_COMPONENT_DAC_R,

    AUDIO_CALIBRATION_COMPONENT_INPUT_MIN,
    AUDIO_CALIBRATION_COMPONENT_MIC0 = AUDIO_CALIBRATION_COMPONENT_INPUT_MIN,
    AUDIO_CALIBRATION_COMPONENT_MIC1,
    AUDIO_CALIBRATION_COMPONENT_MIC2,
    AUDIO_CALIBRATION_COMPONENT_MIC3,
    AUDIO_CALIBRATION_COMPONENT_MIC4,
    AUDIO_CALIBRATION_COMPONENT_MIC5,
    AUDIO_CALIBRATION_COMPONENT_INPUT_MAX = AUDIO_CALIBRATION_COMPONENT_MIC5,

    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_OUTPUT_MIN,
    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_DAC_L = AUDIO_CALIBRATION_FREQUENCY_RESPONSE_OUTPUT_MIN,
    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_DAC_R,
    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_OUTPUT_MAX = AUDIO_CALIBRATION_FREQUENCY_RESPONSE_DAC_R,

    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_INPUT_MIN,
    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_MIC0 = AUDIO_CALIBRATION_FREQUENCY_RESPONSE_INPUT_MIN,
    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_MIC1,
    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_MIC2,
    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_MIC3,
    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_MIC4,
    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_MIC5,
    AUDIO_CALIBRATION_FREQUENCY_RESPONSE_INPUT_MAX = AUDIO_CALIBRATION_FREQUENCY_RESPONSE_MIC5,

    AUDIO_CALIBRATION_COMPONENT_NUMBER,
    AUDIO_CALIBRATION_COMPONENT_MASK                = 0x3F,
    AUDIO_CALIBRATION_COMPONENT_DCHS_RIGHT_CUP_MASK = 0x40,
    AUDIO_CALIBRATION_COMPONENT_DCHS_LEFT_CUP_MASK  = 0x80,

} audio_calibration_component_t;


typedef int8_t audio_calibration_id_t;

typedef struct {
    int16_t gain_offset[AUDIO_CALIBRATION_COMPONENT_NUMBER];
} audio_calibration_component_gain_offset_t;

typedef struct {
    hal_audio_device_t              device;
    hal_audio_interface_t           device_interface;
    audio_calibration_component_t   component;
} audio_calibration_device_matching_t;


typedef struct {
    void *callback_handler;
    void *callback_user_data;

    audio_calibration_component_gain_offset_t component;
#if defined(AIR_COMPONENT_CALIBRATION_SYNC_WITH_ANC_ENABLE)
    audio_anc_control_calibrate_gain_t anc_gain;
    audio_calibration_component_t anc_mic_matching_table[AUDIO_ANC_TOTAL_MIC_NUMBER];
#endif

    bool is_init;
    bool is_used;
} audio_calibration_aud_id_type_t;



/******************************************************************************
 * Global Varables
 ******************************************************************************/

/******************************************************************************
 * Functions Declaration
 ******************************************************************************/
#ifdef AIR_COMPONENT_CALIBRATION_ENABLE
audio_calibration_id_t audio_calibration_init(void *ccni_callback, void *user_data, void *cb_handler);
audio_calibration_result_t audio_calibration_read_nvkey(void);
audio_calibration_result_t audio_calibration_write_nvkey(void);
audio_calibration_result_t audio_calibration_update_gain_offset_to_dsp(void);
audio_calibration_result_t audio_calibration_set_component_gain_offset(audio_calibration_component_t component, int16_t gain_offset);
int16_t audio_calibration_get_component_gain_offset(audio_calibration_component_t component);

#endif

#ifdef __cplusplus
}
#endif

/**
* @}
* @}
*/
#endif  /*__AUDIO_CALIBRATION_H__*/
