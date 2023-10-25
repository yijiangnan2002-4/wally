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

#ifndef _DSP_CONTROL_H_
#define _DSP_CONTROL_H_

#include "types.h"
typedef enum AU_AUDIO_GAIN_COMPONENT_e {
    AUDIO_GAIN_A2DP,
    AUDIO_GAIN_LINE,
    AUDIO_GAIN_SCO,
    AUDIO_GAIN_VC,
    AUDIO_GAIN_VP,
    AUDIO_GAIN_RT,
    AUDIO_GAIN_SCO_NB,
    AUDIO_GAIN_AT,
    AUDIO_GAIN_MAX_COMPONENT,
} AUDIO_GAIN_COMPONENT_t;


typedef enum AU_AFE_OUT_GAIN_COMPONENT_e {
    AU_AFE_OUT_COMPONENT_EAR_GAIN       = 0,
    AU_AFE_OUT_COMPONENT_DEPOP_GAIN     = 1,
    AU_AFE_OUT_COMPONENT_GAIN_INDEX     = 2,
    AU_AFE_OUT_COMPONENT_NO             = 3,
} AU_AFE_OUT_GAIN_COMPONENT_t;


typedef enum AU_AFE_IN_GAIN_COMPONENT_e {
    AU_AFE_IN_COMPONENT_LINE_L          = 0,
    AU_AFE_IN_COMPONENT_LINE_R          = 1,
    AU_AFE_IN_COMPONENT_MIC_L           = 2,
    AU_AFE_IN_COMPONENT_MIC_R           = 3,
    AU_AFE_IN_COMPONENT_ANC_MIC_L       = 4,
    AU_AFE_IN_COMPONENT_ANC_MIC_R       = 5,
    AU_AFE_IN_COMPONENT_VAD_MIC         = 6,
    AU_AFE_IN_COMPONENT_NO              = 7,
} AU_AFE_IN_GAIN_COMPONENT_t;


typedef enum AU_DFE_IN_GAIN_COMPONENT_e {
    AU_DFE_IN_COMPONENT_DIGITAL_GAIN    = 0,
    AU_DFE_IN_COMPONENT_NO              = 1,
} AU_DFE_IN_GAIN_COMPONENT_t;

typedef enum DSP_AU_COMMAND_ANC_e {
    AU_ANC_DISABLE,
    AU_ANC_FF,
    AU_ANC_FB,
    AU_ANC_HYBRID,
} DSP_AU_COMMAND_ANC_t;

typedef enum DSP_AU_COMMAND_AT_e {
    AU_AT_DISABLE,
    AU_AT_SIDETONE,
    AU_AT_mDSP,
    AU_AT_DSP,
} DSP_AU_COMMAND_AT_t;

typedef enum AU_AFE_SPK_VOLTAGE_e {
    AU_AFE_SPK_VOL_3V               = 0,
    AU_AFE_SPK_VOL_1_8V             = 1,
    AU_AFE_SPK_VOL_1_6V             = 2,
} AU_AFE_SPK_VOLTAGE_t;


EXTERN S16 DSP_GC_GetDigitalOutLevel(AUDIO_GAIN_COMPONENT_t Component);
EXTERN VOID DSP_GC_SetDigitalOutLevel(AUDIO_GAIN_COMPONENT_t Component, S16 Gain);
//EXTERN VOID DSP_GC_SetAnalogInLevelImmediate (AU_ANALOG_IN_GAIN_COMPONENT_t Component, S16 Gain);


#endif /* _DSP_CONTROL_H_ */

