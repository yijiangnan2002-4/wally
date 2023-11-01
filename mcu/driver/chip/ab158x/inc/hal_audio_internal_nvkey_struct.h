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

#ifndef __HAL_AUDIO_INTERNAL_NVKEY_STRUCT_H__
#define __HAL_AUDIO_INTERNAL_NVKEY_STRUCT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "hal_audio_internal.h"

#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
#define PACKED __attribute__((packed))
#endif

/** @brief audio structure */
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
/* NvkeyDefine NVID_DSP_FW_AUDIO_CH_CFG */
//0xE0F1 structure
typedef struct HAL_AUDIO_CHANNEL_SELECT_s
{
    uint8_t                     modeForAudioChannel;    /**< NVkey_0     Channel select mode. 0:SW_mode, 1: HW_mode */
    uint8_t                     audioChannel;           /**< NVkey_1     Channel select. 0:None, 1:L CH, 2:R CH, 3:Two CH */
    HAL_AUDIO_CH_SEL_HW_MODE_t  hwAudioChannel;         /**< NVkey_2-4  Channel select param in HW_Mode.*/
} PACKED HAL_AUDIO_CHANNEL_SELECT_t;

/* NvkeyDefine NVID_DSP_FW_AUDIO_HW_IO_CFG */
//0xE00 structure
typedef struct HAL_DSP_PARA_AU_AFE_CTRL_NEW_s {
    /*commmon hw module config*/
    /*############################################ADC DAC config part####################################*/
     HAL_DSP_PARA_AU_AFE_ADC_DAC_CONFIG_t adc_dac_config;
    /*#########################################Digital Mic config part####################################*/
     HAL_DSP_PARA_AU_AFE_DMIC_CONFIG_t dmic_config;
    /*#########################################Sidetone config part#######################################*/
     HAL_DSP_PARA_AU_AFE_SIDETONE_CONFIG_t sidetone_config;
    /*#######################################I2S master config part#######################################*/
     HAL_DSP_PARA_AU_AFE_I2SM_CONFIG_t I2SM_config;
    /*#######################################I2S Slave config part#######################################*/
     HAL_DSP_PARA_AU_AFE_I2SS_CONFIG_t I2SS_config;
     uint8_t test_mode_config[4];
    /*scenario config*/
    /*############################################ANC part#############################################*/
    HAL_DSP_PARA_AU_AFE_ANC_SCENARIO_t anc_scenario;
    /*############################################PassThrough part#############################################*/
    HAL_DSP_PARA_AU_AFE_PASSTHROUGH_SCENARIO_t passthrough_scenario;
    /*############################################Audio part#############################################*/
     HAL_DSP_PARA_AU_AFE_AUDIO_SCENARIO_t audio_scenario;
    /*############################################Voice part#############################################*/
     HAL_DSP_PARA_AU_AFE_VOICE_SCENARIO_t voice_scenario;
    /*############################################Record part############################################*/
     HAL_DSP_PARA_AU_AFE_RECORD_SCENARIO_t record_scenario;
    /*############################################VAD part############################################*/
     HAL_DSP_PARA_AU_AFE_VAD_SCENARIO_t VAD_scenario;
     /*############################################Detach Mic part############################################*/
    HAL_DSP_PARA_AU_AFE_DETACH_MIC_SCENARIO_t detach_mic_scenario;
    uint8_t reserve_scenario[4];
}PACKED HAL_DSP_PARA_AU_AFE_CTRL_t;

#endif
#endif /*__HAL_AUDIO_INTERNAL_NVKEY_STRUCT_H__ */
