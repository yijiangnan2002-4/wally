/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef __BT_SINK_SRV_AUDIO_SETTING_H__
#define __BT_SINK_SRV_AUDIO_SETTING_H__


//#if !defined(MTK_NO_PSRAM_ENABLE)
#define __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
//#endif

#include "stdint.h"
#include "audio_coefficient.h"
#include "bt_sink_srv_audio_setting_nvkey_struct.h"
#include "hal_audio_message_struct_common.h"

#define __GAIN_TABLE_NVDM_DIRECT__

#ifdef __GAIN_TABLE_NVDM_DIRECT__
#define PACKED __attribute__((packed))
#endif


#define BT_SINK_SRV_AUDIO_SETTING_FALG_GEN_VOL_READY                    (0x0001)


/* EAPS_GAIN_PARAMETER_STREAM_OUT_GAIN_SIZE */
#if PRODUCT_VERSION == 2533
#define BT_SINK_SRV_AUDIO_SETTING_STREAM_OUT_GAIN_LEV                   (16)
#else
#define BT_SINK_SRV_AUDIO_SETTING_STREAM_OUT_GAIN_LEV                   (16)
#endif

/* EAPS_GAIN_PARAMETER_STREAM_IN_GAIN_SIZE */
#define BT_SINK_SRV_AUDIO_SETTING_STREAM_IN_GAIN_LEV                    (1)

#ifdef __GAIN_TABLE_NVDM_DIRECT__
/*NVDM ID string*/
/*OUT*/
#define NVDM_ID_STRING_SIZE                     5
#define NVDM_ID_A2DP_PERCENTAGE_TABLE_STRING    "F23B"
#define NVDM_ID_HFP_PERCENTAGE_TABLE_STRING     "F23A"
#define NVDM_ID_MP3_PERCENTAGE_TABLE_STRING     "F23C"
#define NVDM_ID_VPRT_PERCENTAGE_TABLE_STRING    "F23E"
#define NVDM_ID_LINEIN_PERCENTAGE_TABLE_STRING  "F23F"
#define NVDM_ID_USB_AUDIO_PERCENTAGE_TABLE_STRING  "F240"
/*IN*/
#define NVDM_ID_SCO_MIC_PERCENTAGE_TABLE_STRING "F23D"
#endif

#define NVDM_ID_AUDIO_DUMP_TABLE_STRING         "E001"

typedef uint16_t device_t;

typedef enum {
    VOL_A2DP = 0,
    VOL_HFP,
    VOL_PCM,
    VOL_MP3,
    VOL_HFP_NB,
    VOL_VP,
    VOL_VC,
    VOL_LINE_IN,
    VOL_USB_AUDIO_IN,
    VOL_ANC,
    VOL_USB_AUDIO_SW_IN,
    VOL_USB_VOICE_SW_OUT,
    VOL_LINE_IN_DL3,
    VOL_USB_AUDIO_IN_DL3,

    VOL_DEF, /* stream out only */
    VOL_TOTAL
} vol_type_t;

typedef enum {
    GAIN_ANALOG,
    GAIN_DIGITAL,

    GAIN_TOTAL
} gain_type_t;

typedef struct {
    uint32_t digital;
    uint32_t analog_L;
    uint32_t analog_R;
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
    uint32_t analog_MIC2;
    uint32_t analog_MIC3;
    uint32_t analog_MIC4;
    uint32_t analog_MIC5;
#endif
} vol_t;

typedef struct {
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
    //multi mic gain setting
    uint32_t digital_MIC0_L_Digital_Vol;
    uint32_t digital_MIC0_R_Digital_Vol;
    uint32_t digital_MIC1_L_Digital_Vol;
    uint32_t digital_MIC1_R_Digital_Vol;
    uint32_t digital_MIC2_L_Digital_Vol;
    uint32_t digital_MIC2_R_Digital_Vol;
    uint32_t digital_I2S0_L_Digital_Vol;
    uint32_t digital_I2S0_R_Digital_Vol;
    uint32_t digital_I2S1_L_Digital_Vol;
    uint32_t digital_I2S1_R_Digital_Vol;
    uint32_t digital_I2S2_L_Digital_Vol;
    uint32_t digital_I2S2_R_Digital_Vol;
    uint32_t digital_LINEIN_L_Digital_Vol;
    uint32_t digital_LINEIN_R_Digital_Vol;
    uint32_t digital_Echo_Reference_Vol;
    //mic function gain setting
    uint32_t digital_MIC0_L_Func_Digital_Vol;
    uint32_t digital_MIC0_R_Func_Digital_Vol;
    uint32_t digital_MIC1_L_Func_Digital_Vol;
    uint32_t digital_MIC1_R_Func_Digital_Vol;
    uint32_t digital_MIC2_L_Func_Digital_Vol;
    uint32_t digital_MIC2_R_Func_Digital_Vol;
    uint32_t digital_Echo_Func_Reference_Vol;
    uint32_t reserved;
#else
    uint32_t digital_Ref1;
    uint32_t digital_Ref2;
    uint32_t digital_RESERVE;
    uint32_t digital_Echo;
#endif
} vol_multiMIC_t;

typedef struct {
    device_t dev;
    uint8_t lev;
} a2dp_vol_info_t;

typedef struct {
    device_t dev;
    uint8_t lev;
} usb_audio_vol_info_t;

typedef struct {
    vol_t vol;
} a2dp_vol_t;

typedef struct {
    vol_t vol;
} usb_audio_vol_t;

typedef struct {
    uint8_t codec;
    device_t dev_in;
    uint8_t lev_in;
    device_t dev_out;
    uint8_t lev_out;
} hfp_vol_info_t;

typedef struct {
    vol_t          vol_in;
    vol_multiMIC_t vol_multiMIC_in;
    vol_t vol_out;
} hfp_vol_t;

typedef struct {
    device_t dev;
    uint8_t lev;
} pcm_vol_info_t;

typedef struct {
    vol_t vol;
} pcm_vol_t;

typedef struct {
    device_t dev;
    uint8_t lev;
} mp3_vol_info_t;

typedef struct {
    vol_t vol;
} mp3_vol_t;

typedef struct {
    device_t dev;
    uint8_t lev;
} def_vol_info_t;

typedef struct {
    vol_t vol;
} def_vol_t;

typedef struct {
    device_t dev;
    uint8_t lev;
} vp_vol_info_t;

typedef struct {
    vol_t vol;
} vp_vol_t;

typedef struct {
    device_t dev_in;
    uint8_t lev_in;
} vc_vol_info_t;

typedef struct {
    vol_t vol;
    vol_multiMIC_t vol_multiMIC;
} vc_vol_t;

typedef struct {
    device_t  dev_in;
    uint8_t   lev_in;
    device_t  dev_out;
    uint8_t   lev_out;
} lineIN_vol_info_t;

typedef struct {
    vol_t vol_in;
    vol_t vol_out;
} lineIN_vol_t;

typedef struct {
    vol_type_t type;
    union {
        a2dp_vol_info_t   a2dp_vol_info;
        hfp_vol_info_t    hfp_vol_info;
        pcm_vol_info_t    pcm_vol_info;
        mp3_vol_info_t    mp3_vol_info;
        def_vol_info_t    def_vol_info;
        vp_vol_info_t     vp_vol_info;
        vc_vol_info_t     vc_vol_info;
        lineIN_vol_info_t lineIN_vol_info;
        usb_audio_vol_info_t usb_audio_vol_info;
    } vol_info;
} bt_sink_srv_audio_setting_vol_info_t;

typedef struct {
    vol_type_t type;
    union {
        a2dp_vol_t   a2dp_vol;
        hfp_vol_t    hfp_vol;
        pcm_vol_t    pcm_vol;
        mp3_vol_t    mp3_vol;
        def_vol_t    def_vol;
        vp_vol_t     vp_vol;
        vc_vol_t     vc_vol;
        lineIN_vol_t lineIN_vol;
        usb_audio_vol_t usb_audio_vol;
    } vol;
} bt_sink_srv_audio_setting_vol_t;

typedef struct {
    uint32_t flag;
} bt_sink_srv_audio_setting_context_t;

void bt_sink_srv_audio_setting_init(void);

int32_t bt_sink_srv_audio_setting_get_vol(bt_sink_srv_audio_setting_vol_info_t *vol_info,
                                          bt_sink_srv_audio_setting_vol_t *vol);

void bt_sink_srv_audio_setting_update_voice_fillter_setting(bt_sink_srv_audio_setting_vol_info_t *vol_info,
                                                            const audio_eaps_t *am_speech_eaps);

#ifdef __GAIN_TABLE_NVDM_DIRECT__
uint8_t audio_get_max_sound_level_out(vol_type_t volType);
uint8_t audio_get_max_sound_level_in(vol_type_t volType);
#endif

uint32_t audio_get_gain_in_in_dB(uint8_t level, gain_type_t gainType, vol_type_t volType);
uint32_t audio_get_gain_out_in_dB(uint8_t level, gain_type_t gainType, vol_type_t volType);
bool audio_get_analog_gain_out_offset_in_db(uint16_t *L_offset_gain, uint16_t *R_offset_gain);

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
void bt_sink_srv_audio_setting_detach_mic_gain_config(uint32_t *analog_gain, uint32_t *digital_gain, bt_sink_audio_setting_multi_vol_config_t *mult_vol_cfg);
#endif

#ifdef AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE
void bt_sink_srv_audio_setting_init_audio_platform_info(void);
mcu_dsp_audio_platform_share_buffer_info_t *bt_sink_srv_audio_setting_get_audio_platform_share_info(void);
void bt_sink_srv_audio_setting_set_audio_platform_output_volume(vol_type_t type, int32_t digital_gain);

#endif
#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
void mute_mic_detection(int32_t *out_data, uint32_t volume_len, void *user_data);
#endif
#endif /* __BT_SINK_SRV_AUDIO_SETTING_H__ */

