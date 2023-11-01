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

#ifndef __BT_SINK_SRV_AUDIO_SETTING_NVKEY_STRUCT_H__
#define __BT_SINK_SRV_AUDIO_SETTING_NVKEY_STRUCT_H__


#define __GAIN_TABLE_NVDM_DIRECT__

#ifdef __GAIN_TABLE_NVDM_DIRECT__
#define PACKED __attribute__((packed))
#endif

#ifdef __GAIN_TABLE_NVDM_DIRECT__
/* sound level to percentage from 1530 */
typedef struct {
    uint8_t baseSoundLevel;
    uint8_t decreaseSoundLevel;
} PACKED bt_sink_srv_audio_setting_decrease_vol;

typedef struct {
    uint8_t totalSoundLevel;
    uint8_t defaultSoundLevel;
    bt_sink_srv_audio_setting_decrease_vol DecreaseSoundLevelCtl;
} PACKED bt_sink_srv_audio_setting_vol_para;

/* NvkeyDefine NVID_DSP_FW_VOL_CFG_SCO */
//0xE080 structure
typedef struct {
    bt_sink_srv_audio_setting_vol_para scoVolPara;
    uint8_t scoSoundLevelToVgs[1]; //number of totalSoundLevel
    uint8_t scoSoundLevelToBeepTone[1]; //number of totalSoundLevel
    uint8_t scoSoundlevelToPercentage[1]; //number of totalSoundLevel
} PACKED bt_sink_srv_audio_setting_sco_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_VOL_CFG_A2DP */
//0xE081 structure
typedef struct {
    bt_sink_srv_audio_setting_vol_para a2dpVolPara;
    uint8_t a2dpSoundlevelToAvrcpVolume[1]; //number of totalSoundLevel
    uint8_t musicSoundLevelToBeepTone[1]; //number of totalSoundLevel
    uint8_t a2dpSoundlevelToPercentage[1]; //number of totalSoundLevel
} PACKED bt_sink_srv_audio_setting_a2dp_vol_para_t;

typedef struct {
    bt_sink_srv_audio_setting_vol_para usbAudioVolPara;
    uint8_t usbAudioSoundlevelToAvrcpVolume[1]; //number of totalSoundLevel
    uint8_t usbAudioSoundLevelToBeepTone[1]; //number of totalSoundLevel
    uint8_t usbAudioSoundlevelToPercentage[1]; //number of totalSoundLevel
} PACKED bt_sink_srv_audio_setting_usb_audio_vol_para_t;

typedef struct {
    bt_sink_srv_audio_setting_vol_para usbVolPara;
    uint8_t usbSoundlevelToAvrcpVolume[1]; //number of totalSoundLevel
    uint8_t usbSoundLevelToBeepTone[1]; //number of totalSoundLevel
    uint8_t usbSoundlevelToPercentage[1]; //number of totalSoundLevel
} PACKED bt_sink_srv_audio_setting_usb_sw_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_VOL_CFG_MP3 */
//0xE082 structure
typedef struct {
    bt_sink_srv_audio_setting_vol_para mp3VolPara;
    uint8_t mp3SoundlevelToPercentage[1];
} PACKED bt_sink_srv_audio_setting_mp3_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_VOL_CFG_SCOMIC */
//0xE083 structure
typedef struct {
    bt_sink_srv_audio_setting_vol_para scoMicVolPara;
    uint8_t scoMicSoundLevelToVgm[1];
    uint8_t scoMicSoundlevelToPercentage[1];
} PACKED bt_sink_srv_audio_setting_sco_mic_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_VOL_CFG_VPRT */
//0xE084 structure
typedef struct {
    bt_sink_srv_audio_setting_vol_para vprtVolPara;
    uint8_t vprtSoundlevelToPercentage[1];
} PACKED bt_sink_srv_audio_setting_vprt_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_VOL_CFG_LINEIN */
//0xE085 structure
typedef struct {
    bt_sink_srv_audio_setting_vol_para lineInVolPara;
    uint8_t lineInSoundLevelToBeepTone[1];
    uint8_t lineinSoundlevelToPercentage[1];
} PACKED bt_sink_srv_audio_setting_lineIn_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_IAG_TB_LINE */
//0xE041 structure
typedef struct {
    uint16_t lineIn_MicAnalogVol_L;
    uint16_t lineIn_MicAnalogVol_R;
} PACKED bt_sink_srv_audio_setting_lineIn_mic_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_IAG_TB_VC */
//0xE043 structure
typedef struct {
    uint16_t lineIn_MicAnalogVol_L;
    uint16_t lineIn_MicAnalogVol_R;
    uint16_t Mic_AnalogVol_L;
    uint16_t Mic_AnalogVol_R;
    uint16_t Anc_MicAnalogVol_L;
    uint16_t Anc_MicAnalogVol_R;
    uint16_t VC_MicAnalogVol_L;
    uint16_t VC_MicAnalogVol_R;
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
    uint16_t VC_MicAnalogVol_MIC2;
    uint16_t VC_MicAnalogVol_MIC3;
    uint16_t VC_MicAnalogVol_MIC4;
    uint16_t VC_MicAnalogVol_MIC5;
#endif
} PACKED bt_sink_srv_audio_setting_vc_mic_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_IDG_TB_VC */
//0xE033 structure
typedef struct {
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
    uint16_t VC_MIC0_L_Digital_Vol;
    uint16_t VC_MIC0_R_Digital_Vol;
    uint16_t VC_MIC1_L_Digital_Vol;
    uint16_t VC_MIC1_R_Digital_Vol;
    uint16_t VC_MIC2_L_Digital_Vol;
    uint16_t VC_MIC2_R_Digital_Vol;
    uint16_t VC_I2S0_L_Digital_Vol;
    uint16_t VC_I2S0_R_Digital_Vol;
    uint16_t VC_I2S1_L_Digital_Vol;
    uint16_t VC_I2S1_R_Digital_Vol;
    uint16_t VC_I2S2_L_Digital_Vol;
    uint16_t VC_I2S2_R_Digital_Vol;
    uint16_t VC_LINEIN_L_Digital_Vol;
    uint16_t VC_LINEIN_R_Digital_Vol;
    uint16_t VC_Echo_Reference_Vol;
    uint16_t reserved[9];
#else
    uint16_t VC_MainMic_Digital_Vol;
    uint16_t VC_RefMic_Digital_Vol;
#endif
} PACKED bt_sink_srv_audio_setting_vc_digital_mic_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_IAGP_TB_SCO_ML_MIC */
//0xE056 structure
typedef struct {
    uint16_t sco_Extended_MicAnalogVol_L;
    uint16_t sco_Extended_MicAnalogVol_R;
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
    uint16_t sco_Extended_MicAnalogVol_MIC2;
    uint16_t sco_Extended_MicAnalogVol_MIC3;
    uint16_t sco_Extended_MicAnalogVol_MIC4;
    uint16_t sco_Extended_MicAnalogVol_MIC5;
#endif
} PACKED bt_sink_srv_audio_setting_sco_extend_analog_mic_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_IDGP_TB_SCO_MULMIC */
//0xE058 structure
typedef struct {
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
    uint16_t sco_Extended_MIC0_L_Digital_Vol;
    uint16_t sco_Extended_MIC0_R_Digital_Vol;
    uint16_t sco_Extended_MIC1_L_Digital_Vol;
    uint16_t sco_Extended_MIC1_R_Digital_Vol;
    uint16_t sco_Extended_MIC2_L_Digital_Vol;
    uint16_t sco_Extended_MIC2_R_Digital_Vol;
    uint16_t sco_Extended_I2S0_L_Digital_Vol;
    uint16_t sco_Extended_I2S0_R_Digital_Vol;
    uint16_t sco_Extended_I2S1_L_Digital_Vol;
    uint16_t sco_Extended_I2S1_R_Digital_Vol;
    uint16_t sco_Extended_I2S2_L_Digital_Vol;
    uint16_t sco_Extended_I2S2_R_Digital_Vol;
    uint16_t sco_Extended_LINEIN_L_Digital_Vol;
    uint16_t sco_Extended_LINEIN_R_Digital_Vol;
    uint16_t sco_Extended_Echo_Reference_Vol;
    uint16_t reserved[9];
#else
    uint16_t sco_Extended_MainMic_Digital_Vol;
    uint16_t sco_Extended_RefMic_Digital_Vol;
    uint16_t sco_Extended_Ref2Mic_Digital_Vol;
    uint16_t sco_Extended_Reserve_Digital_Vol;
    uint16_t sco_Extended_Echo_Reference_Vol;
#endif
} PACKED bt_sink_srv_audio_setting_sco_extend_digital_mic_vol_para_t;

#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
typedef struct {
    uint16_t WWE_MIC0_L_Digital_Vol;
    uint16_t WWE_MIC0_R_Digital_Vol;
    uint16_t WWE_MIC1_L_Digital_Vol;
    uint16_t WWE_MIC1_R_Digital_Vol;
    uint16_t WWE_MIC2_L_Digital_Vol;
    uint16_t WWE_MIC2_R_Digital_Vol;
    uint16_t WWE_I2S0_L_Digital_Vol;
    uint16_t WWE_I2S0_R_Digital_Vol;
    uint16_t WWE_I2S1_L_Digital_Vol;
    uint16_t WWE_I2S1_R_Digital_Vol;
    uint16_t WWE_I2S2_L_Digital_Vol;
    uint16_t WWE_I2S2_R_Digital_Vol;
    uint16_t WWE_LINEIN_L_Digital_Vol;
    uint16_t WWE_LINEIN_R_Digital_Vol;
    uint16_t WWE_Echo_Reference_Vol;
    uint16_t reserved[9];
} PACKED bt_sink_srv_audio_setting_wwe_digital_mic_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_IDG_TB_MIC_FUNC */
//0xE068 structure
typedef struct {
    uint16_t MIC_FUNC_MIC0_L_Digital_Vol;
    uint16_t MIC_FUNC_MIC0_R_Digital_Vol;
    uint16_t MIC_FUNC_MIC1_L_Digital_Vol;
    uint16_t MIC_FUNC_MIC1_R_Digital_Vol;
    uint16_t MIC_FUNC_MIC2_L_Digital_Vol;
    uint16_t MIC_FUNC_MIC2_R_Digital_Vol;
    uint16_t MIC_FUNC_Echo_Reference_Vol;
    uint16_t reserved[9];
} PACKED bt_sink_srv_audio_setting_mic_func_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_IDG_TB_DETACH_MIC */
//0xE037 structure
typedef struct {
    uint16_t Detach_MIC_Digital_Vol;
    uint16_t Detach_MIC1_Digital_Vol;
    uint16_t reserved[2];
} PACKED bt_sink_srv_audio_setting_detach_mic_digital_mic_vol_para_t;

/* NvkeyDefine NVID_DSP_FW_IAG_TB_DETACH_MIC */
//0xE048 structure
typedef struct {
    uint16_t Detach_MIC_Analog_Vol_ACC10K;
    uint16_t Detach_MIC_Analog_Vol_ACC20K;
    uint16_t Detach_MIC_Analog_Vol_DCC;
    uint16_t Detach_MIC1_Analog_Vol_ACC10K;
    uint16_t Detach_MIC1_Analog_Vol_ACC20K;
    uint16_t Detach_MIC1_Analog_Vol_DCC;
} PACKED bt_sink_srv_audio_setting_detach_mic_analog_mic_vol_para_t;

typedef struct {
    uint32_t digital_MIC0_L;
    uint32_t digital_MIC0_R;
    uint32_t digital_MIC1_L;
    uint32_t digital_MIC1_R;
    uint32_t digital_MIC2_L;
    uint32_t digital_MIC2_R;
    uint32_t digital_I2S0_L;
    uint32_t digital_I2S0_R;
    uint32_t digital_I2S1_L;
    uint32_t digital_I2S1_R;
    uint32_t digital_I2S2_L;
    uint32_t digital_I2S2_R;
    uint32_t digital_LINEIN_L;
    uint32_t digital_LINEIN_R;
    uint32_t digital_Echo;
    uint32_t digital_MIC0_L_func;
    uint32_t digital_MIC0_R_func;
    uint32_t digital_MIC1_L_func;
    uint32_t digital_MIC1_R_func;
    uint32_t digital_MIC2_L_func;
    uint32_t digital_MIC2_R_func;
    uint32_t digital_Echo_func;
    uint32_t analog_R;
    uint32_t analog_MIC2;
    uint32_t analog_MIC3;
    uint32_t analog_MIC4;
    uint32_t analog_MIC5;
} PACKED bt_sink_audio_setting_multi_vol_config_t;

#endif

#endif

#endif /* __BT_SINK_SRV_AUDIO_SETTING_NVKEY_STRUCT_H__ */

