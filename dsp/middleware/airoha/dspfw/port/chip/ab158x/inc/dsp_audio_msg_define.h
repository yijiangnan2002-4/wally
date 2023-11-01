/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef _DSP_AUDIOMSGDEFINE_H_
#define _DSP_AUDIOMSGDEFINE_H_

#include "hal_audio_cm4_dsp_message.h"

//--------------------------------------------
// CM4 to DSP message base address
//--------------------------------------------
#define MSG_DSP_NULL_REPORT             0xffff

//--------------------------------------------
// DSP to CM4 message base address (ACK)
//--------------------------------------------
#define MSG_DSP2MCU_TEST0_BASE         0x0000
#define MSG_DSP2MCU_TEST1_BASE         0x0C00

//--------------------------------------------
// Detailed message from N9 to DSP
//--------------------------------------------
typedef enum {
    MSG_N92DSP_CLOCK_LEADS  = 0x3001,
    MSG_N92DSP_CLOCK_LAGS,
    MSG_N92DSP_DL_IRQ,
    MSG_N92DSP_UL_IRQ,
    MSG_N92DSP_MIC_IRQ,
} N92DSP_AUDIO_MSG;

typedef enum {
    MSG_DSP2LOCAL_AUDIO_PROCESS     = 0x0F00,
} DSP2LOCAL_AUDIO_MSG;

typedef enum {
    MSG2_DSP2CN4_REINIT_BUF_ABNORMAL =  1,
    MSG2_DSP2CN4_REINIT_AFE_ABNORMAL =  2,
} DSP_REINIT_CAUSE;


#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_mcu_dsp_common.h"

typedef enum {
    AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_WIND_NOISE = 0,               /**< The wind detect type to set anc extend ramp gain control.   */
    AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_HOWLING_CONTROL,              /**< No use for now.   */
    AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_USER_UNAWARE,                 /**< The user unaware type to set anc extend ramp gain control.   */
    AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_ENVIRONMENT_DETECTION,        /**< The noise gate type to set anc extend ramp gain control.   */
    AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_NUM,
    AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_MAX          = 0xFF,
} audio_anc_control_extend_ramp_gain_type_t;


typedef enum {
    AUDIO_ANC_CONTROL_EXTEND_GAIN_NORMAL = 0,   // FF + FB
    AUDIO_ANC_CONTROL_EXTEND_GAIN_FF,           // FF only
    AUDIO_ANC_CONTROL_EXTEND_GAIN_FB,           // FB only
} audio_anc_control_extend_ramp_gain_mode_t;

typedef enum {
    AUDIO_ADAPTIVE_ANC_TYPE_USER_UNAWARE = 0,    /**< The user unaware type to set status >**/
    AUDIO_ADAPTIVE_ANC_TYPE_NUM,
    AUDIO_ADAPTIVE_ANC_TYPE_MAX          = 0xFFFF,
} audio_adaptive_anc_type_t;

typedef struct audio_adaptive_anc_report_s {
    uint16_t adaptive_type;    //enum: audio_adaptive_anc_type_t
    int16_t data[2];
} audio_adaptive_anc_report_t, *audio_adaptive_anc_report_ptr_t;

#endif

typedef struct audio_extend_gain_control_s {
    uint8_t gain_type;      // enum: audio_anc_control_extend_ramp_gain_type_t
    uint8_t misc;           // misc parameters for different gain type
    int16_t gain[2];
} audio_extend_gain_control_t, *audio_extend_gain_control_ptr_t;

//--------------------------------------------
// Detailed message from DSP to N9
//--------------------------------------------
typedef enum {
    MSG_DSP2N9_UL_START = 0X3002,
} DSP2N9_AUDIO_MSG;

#endif //_DSP_AUDIOMSGDEFINE_H_
