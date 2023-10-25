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

#ifndef __AUDIO_NVKEY_STRUCT_H__
#define __AUDIO_NVKEY_STRUCT_H__

#include <stdint.h>

#ifndef AUDIO_DUMP_CONFIG_MAX_NUM
#define AUDIO_DUMP_CONFIG_MAX_NUM     (20)
#endif

/**
 *  @brief This structure defines the dsp audio dump NVKey.
 */
typedef struct stru_dsp_datadump_para_s {
    uint32_t Dump_Scenario_Sel;                     /**< @Value 0x00000002 @Desc 0:Diaable, 1:I2S_MASTER, 2:I2S_SLAVE, 3:SPDIF, 4:USB, 5:AIR, */
    uint32_t Dump_Id_Version;                       /**< @Value Dump id Version*/
    uint16_t Dump_IDs[AUDIO_DUMP_CONFIG_MAX_NUM];   /**< @Value each byte map a dump id */
} PACKED DSP_PARA_DATADUMP_STRU;

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
typedef struct {
    uint8_t a2dp_pre_peq_enable;
    uint8_t a2dp_pre_peq_sound_mode;
    uint8_t a2dp_post_peq_enable;
    uint8_t a2dp_post_peq_sound_mode;
    uint8_t linein_pre_peq_enable;
    uint8_t linein_pre_peq_sound_mode;
    uint8_t linein_post_peq_enable;
    uint8_t linein_post_peq_sound_mode;
    uint8_t advanced_passthrough_pre_peq_enable;
    uint8_t advanced_passthrough_pre_peq_sound_mode;
    uint8_t advanced_passthrough_post_peq_enable;
    uint8_t advanced_passthrough_post_peq_sound_mode;
    uint8_t adaptive_peq_enable;
    uint8_t adaptive_peq_sound_mode;
    uint8_t mic_peq_enable;
    uint8_t mic_peq_sound_mode;
    uint8_t advanced_record_peq_enable;
    uint8_t advanced_record_peq_sound_mode;
    uint8_t vivid_pt_peq_enable;
    uint8_t vivid_pt_peq_sound_mode;
    uint16_t nvkey_change_mask;
} peq_nvdm_misc_param_t;

typedef enum {
    PEQ_AUDIO_PATH_A2DP = 0x0,
    PEQ_AUDIO_PATH_LINEIN,
    PEQ_AUDIO_PATH_ADVANCED_PASSTHROUGH,
    PEQ_AUDIO_PATH_ADAPTIVE_EQ,
    PEQ_AUDIO_PATH_VP,
    PEQ_AUDIO_PATH_VP_AEQ,
    PEQ_AUDIO_PATH_MIC,
    PEQ_AUDIO_PATH_ADVANCED_RECORD,
    PEQ_AUDIO_PATH_USB,
    PEQ_AUDIO_PATH_VIVID_PT,
} peq_audio_path_id_t;

typedef struct {
    uint16_t audioPathID;
    uint16_t nvkeyID;
} peq_audio_path_info_t;

typedef struct {
    uint16_t numOfAudioPath;
    peq_audio_path_info_t audioPathList[1];
} peq_audio_path_table_t;

typedef enum {
    PEQ_PHASE_ID_0 = 0,
    PEQ_PHASE_ID_1,
} peq_phase_id_t;

typedef struct {
    uint16_t phaseID;
    uint16_t nvkeyID;
} peq_single_phase_t;

typedef struct {
    uint16_t numOfPhase;
    peq_single_phase_t phaseList[1];
} peq_single_set_t;

typedef struct {
    uint16_t numOfSet;
    peq_single_set_t setList[1];
} peq_full_set_t;

typedef enum {
    PEQ_ELEMENT_ID_32K = 0x0,
    PEQ_ELEMENT_ID_441K,
    PEQ_ELEMENT_ID_48K,
} peq_element_id_t;

typedef struct {
    uint16_t elementID;
    uint16_t numOfParameter;
    uint16_t parameter[1];
} peq_element_t;

typedef struct {
    uint16_t numOfElement;
    uint16_t peqAlgorithmVer;
    peq_element_t elementList[1];
} peq_parameter_t;

#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
typedef struct {
    uint32_t    uplink_bitrate; /* unit: bps, only support 32000 and 50133 */
} ull_misc_nvkey_t;
#endif /* AIR_BT_ULTRA_LOW_LATENCY_ENABLE */

#endif /* __AUDIO_NVKEY_STRUCT_H__ */
