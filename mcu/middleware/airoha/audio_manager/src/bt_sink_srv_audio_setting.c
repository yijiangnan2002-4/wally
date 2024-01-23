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

#include "bt_sink_srv_ami.h"
#include "bt_sink_srv_audio_setting.h"
#include "bt_sink_srv_audio_setting_nvkey_struct.h"

#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__

#include "stdint.h"
#include "nvkey.h"
#include "audio_coefficient.h"
#include "hal_audio.h"
#include "bt_hfp.h"
#include "bt_sink_srv_utils.h"
#include "audio_dsp_fd216_db_to_gain_value_mapping_table.h"
#include "FreeRTOS.h"

#ifdef __GAIN_TABLE_NVDM_DIRECT__
#include "nvkey_dspfw.h"
#include "nvdm.h"
#endif

#include "audio_log.h"


#if PRODUCT_VERSION == 2533
/* MT2533 */
#define SPEECH_MODE_HEADSET_HEADSET_MIC             (0)
/* external dsp*/
#define SPEECH_MODE_HEADSET_DUAL_DIGITAL_MIC        (0)
#define SPEECH_MODE_HEADSET_SINGLE_DIGITAL_MIC      (0)
#else
/* MT2523 */
#define SPEECH_MODE_HEADSET_HEADSET_MIC             (0)
#define SPEECH_MODE_HANDSFREE_MAIN_MIC              (1)
#define SPEECH_MODE_HEADSET_DIGITAL_MIC             (2)
#endif
#ifndef UNUSED
#define UNUSED(x)  ((void)(x))
#endif
#define AUDIO_SETTING_ERR_SUCCESS_7TH            (7)
#define AUDIO_SETTING_ERR_SUCCESS_6TH            (6)
#define AUDIO_SETTING_ERR_SUCCESS_5TH            (5)
#define AUDIO_SETTING_ERR_SUCCESS_4TH            (4)
#define AUDIO_SETTING_ERR_SUCCESS_3RD            (3)
#define AUDIO_SETTING_ERR_SUCCESS_2ND            (2)
#define AUDIO_SETTING_ERR_SUCCESS_1ST            (1)
#define AUDIO_SETTING_ERR_SUCCESS_OK             (0)
#define AUDIO_SETTING_ERR_FAIL_1ST               (-1)
#define AUDIO_SETTING_ERR_FAIL_2ND               (-2)
#define AUDIO_SETTING_ERR_FAIL_3RD               (-3)
#define AUDIO_SETTING_ERR_FAIL_4TH               (-4)
#define AUDIO_SETTING_ERR_FAIL_5TH               (-5)
#define AUDIO_SETTING_ERR_FAIL_6TH               (-6)
#define AUDIO_SETTING_ERR_FAIL_7TH               (-7)

/*Global VOL_LEVEL_MAX*/
#ifdef __GAIN_TABLE_NVDM_DIRECT__
uint8_t AUD_A2DP_VOL_OUT_MAX;
uint8_t AUD_A2DP_VOL_OUT_DEFAULT;
uint8_t AUD_HFP_VOL_OUT_MAX;
uint8_t AUD_HFP_VOL_OUT_DEFAULT;
uint8_t AUD_VPRT_VOL_OUT_MAX;
uint8_t AUD_VPRT_VOL_OUT_DEFAULT;
uint8_t AUD_LINEIN_VOL_OUT_MAX;
uint8_t AUD_LINEIN_VOL_OUT_DEFAULT;
uint8_t AUD_USB_AUDIO_VOL_OUT_MAX;
uint8_t AUD_USB_AUDIO_VOL_OUT_DEFAULT;

#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined (AIR_BT_AUDIO_DONGLE_ENABLE)
uint8_t AUD_USB_AUDIO_SW_VOL_OUT_MAX;
uint8_t AUD_USB_AUDIO_SW_VOL_OUT_DEFAULT;
uint8_t AUD_USB_VOICE_SW_VOL_OUT_MAX;
uint8_t AUD_USB_VOICE_SW_VOL_OUT_DEFAULT;
#endif

#endif


extern void speech_update_common(const uint16_t *common);
extern void speech_update_nb_param(const uint16_t *param);
extern void speech_update_wb_param(const uint16_t *param);
extern void speech_update_nb_fir(const int16_t *in_coeff, const int16_t *out_coeff);
extern void speech_update_wb_fir(const int16_t *in_coeff, const int16_t *out_coeff);
#ifdef __GAIN_TABLE_NVDM_DIRECT__
#define STRNCPY_GAIN(dest, source) strncpy(dest, source, strlen(source)+1)
#endif

/* static function */
static bt_sink_srv_audio_setting_context_t g_bt_sink_srv_audio_setting_ctx;

static void bt_sink_srv_audio_setting_gen_vol_table(void);

static bt_sink_srv_audio_setting_context_t *bt_sink_srv_audio_setting_get_context(void);

static void bt_sink_srv_audio_setting_get_a2dp_vol(a2dp_vol_info_t *info,
                                                   a2dp_vol_t *vol);

static void bt_sink_srv_audio_setting_get_hfp_vol(hfp_vol_info_t *info,
                                                  hfp_vol_t *vol);

static void bt_sink_srv_audio_setting_get_pcm_vol(pcm_vol_info_t *info,
                                                  pcm_vol_t *vol);

static void bt_sink_srv_audio_setting_get_mp3_vol(mp3_vol_info_t *info,
                                                  mp3_vol_t *vol);

static void bt_sink_srv_audio_setting_get_vp_vol(vp_vol_info_t *info,
                                                 vp_vol_t *vol);

extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;


static bt_sink_srv_audio_setting_context_t *bt_sink_srv_audio_setting_get_context(void)
{
    return &g_bt_sink_srv_audio_setting_ctx;
}

static void bt_sink_srv_audio_setting_gen_vol_table(void)
{
    bt_sink_srv_audio_setting_context_t *setting_ctx = NULL;

    setting_ctx = bt_sink_srv_audio_setting_get_context();

    setting_ctx->flag &= ~(BT_SINK_SRV_AUDIO_SETTING_FALG_GEN_VOL_READY);


    setting_ctx->flag |= BT_SINK_SRV_AUDIO_SETTING_FALG_GEN_VOL_READY;
}


void bt_sink_srv_audio_setting_init(void)
{
    audio_src_srv_report("[Sink][Setting]init--ver: %d", 1, PRODUCT_VERSION);

    memset(&g_bt_sink_srv_audio_setting_ctx, 0x00, sizeof(bt_sink_srv_audio_setting_context_t));

    bt_sink_srv_audio_setting_gen_vol_table();

    audio_src_srv_report("[Sink][Setting]eaps_reg--cid: 0x%08x", 1, &g_bt_sink_srv_audio_setting_ctx);
}

#ifdef __GAIN_TABLE_NVDM_DIRECT__
static uint16_t audio_get_Index(uint8_t percentage, uint16_t totalIndex)
{
    uint16_t index;

    index = (totalIndex * percentage) / 100;

    if (index == totalIndex) {
        index = (totalIndex - 1);
    }

    return index;
}

static uint8_t audio_get_max_sound_level_out_a2dp()
{
    bt_sink_srv_audio_setting_a2dp_vol_para_t *pNvdmA2dpVol = NULL;
    uint8_t totalLevel, status, defaultLevel;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_A2DP, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_a2dp Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmA2dpVol = (bt_sink_srv_audio_setting_a2dp_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_A2DP, (uint8_t *)pNvdmA2dpVol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_A2DP

    if (status || !pNvdmA2dpVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_a2dp Fail 2. Status:%d pNvdmA2dpVol:0x%x\n", 2, status, pNvdmA2dpVol);
        if (pNvdmA2dpVol) {
            vPortFree(pNvdmA2dpVol);
        }
        return 0;
    }

    totalLevel = pNvdmA2dpVol->a2dpVolPara.totalSoundLevel;
    defaultLevel = pNvdmA2dpVol->a2dpVolPara.defaultSoundLevel;

    AUD_A2DP_VOL_OUT_DEFAULT = defaultLevel;
    if (AUD_A2DP_VOL_OUT_DEFAULT > (totalLevel - 1)) { //Gain table max level was totalLevel-1
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_max_sound_level_out_a2dp error 2. Default level > totallevel-1");
    }

    vPortFree(pNvdmA2dpVol);
    return totalLevel;
}

static uint8_t audio_get_max_sound_level_out_usb_audio()
{
    bt_sink_srv_audio_setting_usb_audio_vol_para_t *pNvdmUsbAudioVol = NULL;
    uint8_t totalLevel, status, defaultLevel;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_AUDIO, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_usb_audio Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmUsbAudioVol = (bt_sink_srv_audio_setting_usb_audio_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_AUDIO, (uint8_t *)pNvdmUsbAudioVol, &tableSize);

    if (status || !pNvdmUsbAudioVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_usb_audio Fail 2. Status:%d pNvdmUsbAudioVol:0x%x\n", 2, status, pNvdmUsbAudioVol);
        if (pNvdmUsbAudioVol) {
            vPortFree(pNvdmUsbAudioVol);
        }
        return 0;
    }

    totalLevel = pNvdmUsbAudioVol->usbAudioVolPara.totalSoundLevel;
    defaultLevel = pNvdmUsbAudioVol->usbAudioVolPara.defaultSoundLevel;

    AUD_USB_AUDIO_VOL_OUT_DEFAULT = defaultLevel;
    if (AUD_USB_AUDIO_VOL_OUT_DEFAULT > (totalLevel - 1)) { //Gain table max level was totalLevel-1
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_max_sound_level_out_usb_audio error 2. Default level > totallevel-1");
    }

    vPortFree(pNvdmUsbAudioVol);
    return totalLevel;
}

#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
static uint8_t audio_get_max_sound_level_out_usb_audio_sw()
{
    bt_sink_srv_audio_setting_usb_sw_vol_para_t *pNvdmUsbAudioSwVol = NULL;
    uint8_t  totalLevel   = 0;
    uint8_t  status;
    uint8_t  defaultLevel = 0;
    uint32_t tableSize    = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_AUDIO_SW, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_usb_music_sw Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmUsbAudioSwVol = (bt_sink_srv_audio_setting_usb_sw_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_AUDIO_SW, (uint8_t *)pNvdmUsbAudioSwVol, &tableSize);

    if (status || !pNvdmUsbAudioSwVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_usb_music_sw Fail 2. Status:%d pNvdmUsbAudioSwVol:0x%x\n", 2, status, pNvdmUsbAudioSwVol);
        if (pNvdmUsbAudioSwVol) {
            vPortFree(pNvdmUsbAudioSwVol);
        }
        return 0;
    }

    totalLevel = pNvdmUsbAudioSwVol->usbVolPara.totalSoundLevel;
    defaultLevel = pNvdmUsbAudioSwVol->usbVolPara.defaultSoundLevel;

    AUD_USB_AUDIO_SW_VOL_OUT_MAX = defaultLevel;
    if (AUD_USB_AUDIO_SW_VOL_OUT_DEFAULT > (totalLevel - 1)) { //Gain table max level was totalLevel-1
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_max_sound_level_out_usb_music_sw error 2. Default level > totallevel-1");
    }

    vPortFree(pNvdmUsbAudioSwVol);
    return totalLevel;
}
#endif

static uint8_t audio_get_max_sound_level_out_hfp()
{
    bt_sink_srv_audio_setting_sco_vol_para_t *pNvdmHfpVol = NULL;
    uint8_t totalLevel, status;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_SCO, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_hfp Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmHfpVol = (bt_sink_srv_audio_setting_sco_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_SCO, (uint8_t *)pNvdmHfpVol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_SCO

    if (status || !pNvdmHfpVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_hfp Fail 2. Status:%d pNvdmHfpVol:0x%x\n", 2, status, pNvdmHfpVol);
        if (pNvdmHfpVol) {
            vPortFree(pNvdmHfpVol);
        }
        return 0;
    }

    totalLevel = pNvdmHfpVol->scoVolPara.totalSoundLevel;

    vPortFree(pNvdmHfpVol);
    return totalLevel;
}

static uint8_t audio_get_max_sound_level_out_mp3()
{
    bt_sink_srv_audio_setting_mp3_vol_para_t *pNvdmMp3Vol = NULL;
    uint8_t totalLevel, status;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_MP3, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_mp3 Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmMp3Vol = (bt_sink_srv_audio_setting_mp3_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_MP3, (uint8_t *)pNvdmMp3Vol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_MP3

    if (status || !pNvdmMp3Vol) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_mp3 Fail 2. Status:%d pNvdmMp3Vol:0x%x\n", 2, status, pNvdmMp3Vol);
        if (pNvdmMp3Vol) {
            vPortFree(pNvdmMp3Vol);
        }
        return 0;
    }

    totalLevel = pNvdmMp3Vol->mp3VolPara.totalSoundLevel;

    vPortFree(pNvdmMp3Vol);
    return totalLevel;
}

static uint8_t audio_get_max_sound_level_out_pcm()
{
    bt_sink_srv_audio_setting_vprt_vol_para_t *pNvdmVprtVol = NULL;
    uint8_t totalLevel, defaultLevel, status;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_VPRT, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_pcm Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmVprtVol = (bt_sink_srv_audio_setting_vprt_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_VPRT, (uint8_t *)pNvdmVprtVol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_VPRT

    if (status || !pNvdmVprtVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_pcm Fail 2. Status:%d pNvdmVprtVol:0x%x\n", 2, status, pNvdmVprtVol);
        if (pNvdmVprtVol) {
            vPortFree(pNvdmVprtVol);
        }
        return 0;
    }

    totalLevel = pNvdmVprtVol->vprtVolPara.totalSoundLevel;
    defaultLevel = pNvdmVprtVol->vprtVolPara.defaultSoundLevel;
    AUD_VPRT_VOL_OUT_DEFAULT = defaultLevel;
    if (AUD_VPRT_VOL_OUT_DEFAULT > (totalLevel - 1)) { //Gain table max level was totalLevel - 1
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_max_sound_level_out_vp error 2. Default level > totallevel-1");
    }

    vPortFree(pNvdmVprtVol);
    return totalLevel;
}

static uint8_t audio_get_max_sound_level_out_lineIN()
{
    bt_sink_srv_audio_setting_lineIn_vol_para_t *pNvdmLineIN_Vol = NULL;
    uint8_t totalLevel, defaultLevel, status;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_LINEIN, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_LineIN Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmLineIN_Vol = (bt_sink_srv_audio_setting_lineIn_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_LINEIN, (uint8_t *)pNvdmLineIN_Vol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_LINEIN

    if (status || !pNvdmLineIN_Vol) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out_LineIN Fail 2. Status:%d pNvdmHfpVol:0x%x\n", 2, status, pNvdmLineIN_Vol);
        if (pNvdmLineIN_Vol) {
            vPortFree(pNvdmLineIN_Vol);
        }
        return 0;
    }

    totalLevel = pNvdmLineIN_Vol->lineInVolPara.totalSoundLevel;
    defaultLevel = pNvdmLineIN_Vol->lineInVolPara.defaultSoundLevel;
    AUD_LINEIN_VOL_OUT_DEFAULT = defaultLevel;
    if (AUD_LINEIN_VOL_OUT_DEFAULT > (totalLevel - 1)) { //Gain table max level was totalLevel - 1
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_max_sound_level_out_lineIN error 2. Default level > totallevel-1");
    }

    vPortFree(pNvdmLineIN_Vol);
    return totalLevel;
}

uint8_t audio_get_max_sound_level_out(vol_type_t volType)
{
    uint8_t totalSoundLevel = 0;

    switch (volType) {
        case VOL_A2DP:
            totalSoundLevel = audio_get_max_sound_level_out_a2dp();
            break;

        case VOL_HFP:
            totalSoundLevel = audio_get_max_sound_level_out_hfp();
            break;

        case VOL_PCM:
            totalSoundLevel = audio_get_max_sound_level_out_pcm();
            break;

        case VOL_MP3:
            totalSoundLevel = audio_get_max_sound_level_out_mp3();
            break;

        case VOL_HFP_NB:
            totalSoundLevel = audio_get_max_sound_level_out_hfp();
            break;

        case VOL_VP:
            totalSoundLevel = audio_get_max_sound_level_out_pcm();
            break;

        case VOL_LINE_IN:
            totalSoundLevel = audio_get_max_sound_level_out_lineIN();
            break;

        case VOL_USB_AUDIO_IN:
            totalSoundLevel = audio_get_max_sound_level_out_usb_audio();
            break;
#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        case VOL_USB_AUDIO_SW_IN:
            totalSoundLevel = audio_get_max_sound_level_out_usb_audio_sw();
            break;
#endif
    }

    audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_out volType:%d totalLevel:%d\n", 2, volType, totalSoundLevel);
    return totalSoundLevel;
}

/*Get Stream Out Gain*/
static uint8_t audio_get_percentage_out_a2dp(uint8_t level)
{
    bt_sink_srv_audio_setting_a2dp_vol_para_t *pNvdmA2dpVol = NULL;
    uint8_t percentage, totalLevel, status, defaultLevel;
    uint8_t *pA2dpPercentageTable;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_A2DP, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_a2dp Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmA2dpVol = (bt_sink_srv_audio_setting_a2dp_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_A2DP, (uint8_t *)pNvdmA2dpVol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_A2DP

    if (status || !pNvdmA2dpVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_a2dp Fail 2. Status:%d pNvdmA2dpVol:0x%x\n", 2, status, pNvdmA2dpVol);
        if (pNvdmA2dpVol) {
            vPortFree(pNvdmA2dpVol);
        }
        return 0;
    }

    totalLevel   = pNvdmA2dpVol->a2dpVolPara.totalSoundLevel;
    defaultLevel = pNvdmA2dpVol->a2dpVolPara.defaultSoundLevel;

    /*For dynamic volume level changes by tool*/
    AUD_A2DP_VOL_OUT_MAX = totalLevel - 1;
    if ((AUD_A2DP_VOL_OUT_MAX == 0) || (AUD_A2DP_VOL_OUT_MAX > 100)) { //Gain table max level was 100
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_a2dp error 2. Max level=0 or more than 100");
    }
    AUD_A2DP_VOL_OUT_DEFAULT = defaultLevel;
    if (AUD_A2DP_VOL_OUT_DEFAULT > AUD_A2DP_VOL_OUT_MAX) { //Gain table max level was AUD_A2DP_VOL_OUT_MAX
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_a2dp error 2. default level > Max level ");
    }

    if (level >= totalLevel) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_a2dp Invalid Level=%d, total level\n", 2, level, totalLevel);
        level = totalLevel - 1;
    }

    pA2dpPercentageTable = ((uint8_t *)pNvdmA2dpVol) + sizeof(bt_sink_srv_audio_setting_vol_para) + totalLevel/*AVRCP*/ + totalLevel/*BeepTone*/;
    percentage = pA2dpPercentageTable[level];
    //audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_a2dp level[%d] percentage:%d\n", 2, level, percentage);

    vPortFree(pNvdmA2dpVol);
    return percentage;
}


/*Get Stream Out Gain*/
static uint8_t audio_get_percentage_out_usb_audio(uint8_t level)
{
    bt_sink_srv_audio_setting_usb_audio_vol_para_t *pNvdmUsbAudioVol = NULL;
    uint8_t percentage, totalLevel, status, defaultLevel;
    uint8_t *pUsbAudioPercentageTable;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_AUDIO, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_audio Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmUsbAudioVol = (bt_sink_srv_audio_setting_usb_audio_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_AUDIO, (uint8_t *)pNvdmUsbAudioVol, &tableSize);

    if (status || !pNvdmUsbAudioVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_audio Fail 2. Status:%d pNvdmUsbAudioVol:0x%x\n", 2, status, pNvdmUsbAudioVol);
        if (pNvdmUsbAudioVol) {
            vPortFree(pNvdmUsbAudioVol);
        }
        return 0;
    }

    totalLevel   = pNvdmUsbAudioVol->usbAudioVolPara.totalSoundLevel;
    defaultLevel = pNvdmUsbAudioVol->usbAudioVolPara.defaultSoundLevel;

    /*For dynamic volume level changes by tool*/
    AUD_USB_AUDIO_VOL_OUT_MAX = totalLevel - 1;
    if ((AUD_USB_AUDIO_VOL_OUT_MAX == 0) || (AUD_USB_AUDIO_VOL_OUT_MAX > 100)) { //Gain table max level was 100
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_usb_audio error 2. Max level=0 or more than 100");
    }
    AUD_USB_AUDIO_VOL_OUT_DEFAULT = defaultLevel;
    if (AUD_USB_AUDIO_VOL_OUT_DEFAULT > AUD_USB_AUDIO_VOL_OUT_MAX) { //Gain table max level was AUD_USB_AUDIO_VOL_OUT_MAX
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_usb_audio error 2. default level > Max level ");
    }

    if (level >= totalLevel) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_audio Invalid Level=%d, total level\n", 2, level, totalLevel);
        level = totalLevel - 1;
    }

    pUsbAudioPercentageTable = ((uint8_t *)pNvdmUsbAudioVol) + sizeof(bt_sink_srv_audio_setting_vol_para) + totalLevel/*AVRCP*/ + totalLevel/*BeepTone*/;
    percentage = pUsbAudioPercentageTable[level];
    //audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb level[%d] percentage:%d\n", 2, level, percentage);

    vPortFree(pNvdmUsbAudioVol);
    return percentage;
}

#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
static uint8_t audio_get_percentage_in_usb_voice_sw(uint8_t level)
{
    bt_sink_srv_audio_setting_usb_sw_vol_para_t *pNvdmUsbVoiceSwVol = NULL;
    uint8_t percentage   = 0;
    uint8_t totalLevel   = 0;
    uint8_t status       = 0;
    uint8_t defaultLevel = 0;
    uint8_t *pUsbVoivePercentageTable = NULL;
    uint32_t tableSize   = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_VOICE_SW, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_voice_sw Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmUsbVoiceSwVol = (bt_sink_srv_audio_setting_usb_sw_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_VOICE_SW, (uint8_t *)pNvdmUsbVoiceSwVol, &tableSize);

    if (status || !pNvdmUsbVoiceSwVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_voice_sw Fail 2. Status:%d pNvdmUsbVoiceSwVol:0x%x\n", 2, status, pNvdmUsbVoiceSwVol);
        if (pNvdmUsbVoiceSwVol) {
            vPortFree(pNvdmUsbVoiceSwVol);
        }
        return 0;
    }

    totalLevel   = pNvdmUsbVoiceSwVol->usbVolPara.totalSoundLevel;
    defaultLevel = pNvdmUsbVoiceSwVol->usbVolPara.defaultSoundLevel;

    /*For dynamic volume level changes by tool*/
    AUD_USB_VOICE_SW_VOL_OUT_MAX = totalLevel - 1;
    if ((AUD_USB_VOICE_SW_VOL_OUT_MAX == 0) || (AUD_USB_VOICE_SW_VOL_OUT_MAX > 100)) { //Gain table max level was 100
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_usb_voice_sw error 2. Max level=0 or more than 100");
    }
    AUD_USB_VOICE_SW_VOL_OUT_DEFAULT = defaultLevel;
    if (AUD_USB_VOICE_SW_VOL_OUT_DEFAULT > AUD_USB_VOICE_SW_VOL_OUT_MAX) { //Gain table max level was AUD_USB_AUDIO_VOL_OUT_MAX
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_usb_voice_sw error 2. default level > Max level ");
    }

    if (level >= totalLevel) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_voice_sw Invalid Level\n", 0);
        level = totalLevel - 1;
    }

    pUsbVoivePercentageTable = ((uint8_t *)pNvdmUsbVoiceSwVol) + sizeof(bt_sink_srv_audio_setting_vol_para);
    percentage = pUsbVoivePercentageTable[level];
    //audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_voice_sw level[%d] percentage:%d\n", 2, level, percentage);

    vPortFree(pNvdmUsbVoiceSwVol);
    return percentage;
}

static uint8_t audio_get_percentage_out_usb_audio_sw(uint8_t level)
{
    bt_sink_srv_audio_setting_usb_sw_vol_para_t *pNvdmUsbAudioVol = NULL;
    uint8_t percentage   = 0;
    uint8_t totalLevel   = 0;
    uint8_t status       = 0;
    uint8_t defaultLevel = 0;
    uint8_t *pUsbVoivePercentageTable = NULL;
    uint32_t tableSize   = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_AUDIO_SW, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_audio_sw Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmUsbAudioVol = (bt_sink_srv_audio_setting_usb_sw_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_AUDIO_SW, (uint8_t *)pNvdmUsbAudioVol, &tableSize);

    if (status || !pNvdmUsbAudioVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_audio_sw Fail 2. Status:%d pNvdmUsbAudioVol:0x%x\n", 2, status, pNvdmUsbAudioVol);
        if (pNvdmUsbAudioVol) {
            vPortFree(pNvdmUsbAudioVol);
        }
        return 0;
    }

    totalLevel   = pNvdmUsbAudioVol->usbVolPara.totalSoundLevel;
    defaultLevel = pNvdmUsbAudioVol->usbVolPara.defaultSoundLevel;

    /*For dynamic volume level changes by tool*/
    AUD_USB_AUDIO_SW_VOL_OUT_MAX = totalLevel - 1;
    if ((AUD_USB_AUDIO_SW_VOL_OUT_MAX == 0) || (AUD_USB_AUDIO_SW_VOL_OUT_MAX > 100)) { //Gain table max level was 100
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_usb_audio_sw error 2. Max level=0 or more than 100");
    }
    AUD_USB_AUDIO_SW_VOL_OUT_DEFAULT = defaultLevel;
    if (AUD_USB_AUDIO_SW_VOL_OUT_DEFAULT > AUD_USB_AUDIO_SW_VOL_OUT_MAX) { //Gain table max level was AUD_USB_AUDIO_VOL_OUT_MAX
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_a2dp error 2. default level > Max level ");
    }

    if (level >= totalLevel) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_audio_sw Invalid Level\n", 0);
        level = totalLevel - 1;
    }

    pUsbVoivePercentageTable = ((uint8_t *)pNvdmUsbAudioVol) + sizeof(bt_sink_srv_audio_setting_vol_para);
    percentage = pUsbVoivePercentageTable[level];
    //audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_usb_audio_sw level[%d] percentage:%d\n", 2, level, percentage);

    vPortFree(pNvdmUsbAudioVol);
    return percentage;
}
#endif

static uint8_t audio_get_percentage_out_hfp(uint8_t level)
{
    bt_sink_srv_audio_setting_sco_vol_para_t *pNvdmHfpVol = NULL;
    uint8_t percentage, totalLevel, status;
    uint8_t *pHfpPercentageTable;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_SCO, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_hfp Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmHfpVol = (bt_sink_srv_audio_setting_sco_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_SCO, (uint8_t *)pNvdmHfpVol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_SCO

    if (status || !pNvdmHfpVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_hfp Fail 2. Status:%d pNvdmHfpVol:0x%x\n", 2, status, pNvdmHfpVol);
        if (pNvdmHfpVol) {
            vPortFree(pNvdmHfpVol);
        }
        return 0;
    }

    totalLevel = pNvdmHfpVol->scoVolPara.totalSoundLevel;
    if (level >= totalLevel) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_hfp Invalid Level\n", 0);
        level = totalLevel - 1;
    }

    pHfpPercentageTable = ((uint8_t *)pNvdmHfpVol) + sizeof(bt_sink_srv_audio_setting_vol_para) + totalLevel/*VGS*/ + totalLevel/*BeepTone*/;
    percentage = pHfpPercentageTable[level];
    //audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_hfp level[%d] percentage:%d\n", 2, level, percentage);

    vPortFree(pNvdmHfpVol);
    return percentage;
}

static uint8_t audio_get_percentage_out_mp3(uint8_t level)
{
    bt_sink_srv_audio_setting_mp3_vol_para_t *pNvdmMp3Vol = NULL;
    uint8_t percentage, totalLevel, status;
    uint8_t *pMp3PercentageTable;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_MP3, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_mp3 Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmMp3Vol = (bt_sink_srv_audio_setting_mp3_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_MP3, (uint8_t *)pNvdmMp3Vol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_MP3

    if (status || !pNvdmMp3Vol) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_mp3 Fail 2. Status:%d pNvdmMp3Vol:0x%x\n", 2, status, pNvdmMp3Vol);
        if (pNvdmMp3Vol) {
            vPortFree(pNvdmMp3Vol);
        }
        return 0;
    }

    totalLevel = pNvdmMp3Vol->mp3VolPara.totalSoundLevel;
    if (level >= totalLevel) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_mp3 Invalid Level\n", 0);
        level = totalLevel - 1;
    }

    pMp3PercentageTable = ((uint8_t *)pNvdmMp3Vol) + sizeof(bt_sink_srv_audio_setting_vol_para);
    percentage = pMp3PercentageTable[level];
    //audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_mp3 level[%d] percentage:%d\n", 2, level, percentage);

    vPortFree(pNvdmMp3Vol);
    return percentage;
}

static uint8_t audio_get_percentage_out_pcm(uint8_t level)
{
    bt_sink_srv_audio_setting_vprt_vol_para_t *pNvdmVprtVol = NULL;
    uint8_t percentage, totalLevel, defaultLevel, status;
    uint8_t *pVprtPercentageTable;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_VPRT, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_pcm Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmVprtVol = (bt_sink_srv_audio_setting_vprt_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_VPRT, (uint8_t *)pNvdmVprtVol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_VPRT

    if (status || !pNvdmVprtVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_pcm Fail 2. Status:%d pNvdmVprtVol:0x%x\n", 2, status, pNvdmVprtVol);
        if (pNvdmVprtVol) {
            vPortFree(pNvdmVprtVol);
        }
        return 0;
    }

    totalLevel = pNvdmVprtVol->vprtVolPara.totalSoundLevel;
    defaultLevel = pNvdmVprtVol->vprtVolPara.defaultSoundLevel;
    /*For dynamic volume level changes by tool*/
    AUD_VPRT_VOL_OUT_MAX = totalLevel - 1;
    if ((AUD_VPRT_VOL_OUT_MAX == 0) || (AUD_VPRT_VOL_OUT_MAX > 100)) { //Gain table max level was 15
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_PCM error 2. Max level=0 or more than 100");
    }
    AUD_VPRT_VOL_OUT_DEFAULT = defaultLevel;
    if (AUD_VPRT_VOL_OUT_DEFAULT > AUD_VPRT_VOL_OUT_MAX) { //Gain table max level was AUD_VPRT_VOL_OUT_MAX
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_PCM error 2. default level > Max level ");
    }

    if (level >= totalLevel) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_pcm Invalid Level\n", 0);
        level = totalLevel - 1;
    }

    pVprtPercentageTable = ((uint8_t *)pNvdmVprtVol) + sizeof(bt_sink_srv_audio_setting_vol_para);
    percentage = pVprtPercentageTable[level];
    //audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_pcm level[%d] percentage:%d\n", 2, level, percentage);

    vPortFree(pNvdmVprtVol);
    return percentage;
}

static uint8_t audio_get_percentage_out_lineIN(uint8_t level)
{
    bt_sink_srv_audio_setting_lineIn_vol_para_t *pNvdmLineIN_Vol = NULL;
    uint8_t percentage, totalLevel, defaultLevel, status;
    uint8_t *pLineIN_PercentageTable;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_LINEIN, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_LineIN Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmLineIN_Vol = (bt_sink_srv_audio_setting_lineIn_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_LINEIN, (uint8_t *)pNvdmLineIN_Vol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_LINEIN

    if (status || !pNvdmLineIN_Vol) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_LineIN Fail 2. Status:%d pNvdmHfpVol:0x%x\n", 2, status, pNvdmLineIN_Vol);
        if (pNvdmLineIN_Vol) {
            vPortFree(pNvdmLineIN_Vol);
        }
        return 0;
    }

    totalLevel = pNvdmLineIN_Vol->lineInVolPara.totalSoundLevel;
    defaultLevel = pNvdmLineIN_Vol->lineInVolPara.defaultSoundLevel;
    /*For dynamic volume level changes by tool*/
    AUD_LINEIN_VOL_OUT_MAX = totalLevel - 1;
    if ((AUD_LINEIN_VOL_OUT_MAX == 0) || (AUD_LINEIN_VOL_OUT_MAX > 100)) { //Gain table max level was 100
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_lineIN error 2. Max level=0 or more than 100");
    }
    AUD_LINEIN_VOL_OUT_DEFAULT = defaultLevel;
    if (AUD_LINEIN_VOL_OUT_DEFAULT > AUD_LINEIN_VOL_OUT_MAX) { //Gain table max level was AUD_LINEIN_VOL_OUT_MAX
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_percentage_out_lineIN error 2. default level > Max level ");
    }

    if (level >= totalLevel) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_LineIN Invalid Level\n", 0);
        level = totalLevel - 1;
    }

    pLineIN_PercentageTable = ((uint8_t *)pNvdmLineIN_Vol) + sizeof(bt_sink_srv_audio_setting_vol_para) + totalLevel/*BeepTone*/;
    percentage = pLineIN_PercentageTable[level];
    //audio_src_srv_report("[Sink][Setting]audio_get_percentage_out_LineIN level[%d] percentage:%d total_level: %d\n", 3, level, percentage, totalLevel);

    vPortFree(pNvdmLineIN_Vol);
    return percentage;
}

static uint16_t audio_get_uint16_value(uint8_t *addr)
{
    //prevent alignment problem
    uint16_t value = 0;

    value |= (addr[0] << 0);
    value |= (addr[1] << 8);

    return value;
}

static uint32_t audio_get_uint32_value(uint8_t *addr)
{
    //prevent alignment problem
    uint32_t value = 0;

    value |= (addr[0] << 0);
    value |= (addr[1] << 8);
    value |= (addr[2] << 16);
    value |= (addr[3] << 24);

    return value;
}

bool audio_get_analog_gain_out_offset_in_db(uint16_t *L_offset_gain, uint16_t *R_offset_gain)
{
    uint16_t nvkeyId = NVID_DSP_FW_AG_TB_COMMON;
    uint32_t tableSize;
    uint8_t status;
    uint8_t *pGainTable = NULL;

    status = nvkey_data_item_length(nvkeyId, &tableSize);
    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_analog_gain_out_offset_in_db Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return false;
    }
    pGainTable = (uint8_t *)pvPortMalloc(tableSize);
    status = nvkey_read_data(nvkeyId, pGainTable, &tableSize);
    if (status || !pGainTable) {
        audio_src_srv_report("[Sink][Setting]audio_get_gain_out_in_dB Fail 2. Status:%d pGainTable:0x%x\n", 2, status, pGainTable);
        if (pGainTable) {
            vPortFree(pGainTable);
        }
        return false;
    }

    /*save the offset gain*/
    *L_offset_gain = audio_get_uint16_value(pGainTable + 2);
    *R_offset_gain = audio_get_uint16_value(pGainTable + 6);

    vPortFree(pGainTable);

    return true;
}

uint32_t audio_get_gain_out_in_dB(uint8_t level, gain_type_t gainType, vol_type_t volType)
{
    uint32_t gian_dB, tableSize;
    uint16_t totalIndex, index;
    uint8_t percentage, status;
    uint8_t *pGainTable;
    uint16_t nvkeyId;

    switch (volType) {
        case VOL_A2DP:
            percentage = audio_get_percentage_out_a2dp(level);
            if (gainType == GAIN_DIGITAL) {
                nvkeyId = NVID_DSP_FW_DG_TB_A2DP;
            } else {
                nvkeyId = NVID_DSP_FW_AG_TB_A2DP;
            }
            break;

        case VOL_HFP:
            percentage = audio_get_percentage_out_hfp(level);
            if (gainType == GAIN_DIGITAL) {
                nvkeyId = NVID_DSP_FW_DG_TB_SCO;
            } else {
                nvkeyId = NVID_DSP_FW_AG_TB_SCO;
            }
            break;

        case VOL_HFP_NB:
            percentage = audio_get_percentage_out_hfp(level);
            if (gainType == GAIN_DIGITAL) {
                nvkeyId = NVID_DSP_FW_DG_TB_SCONB;
            } else {
                nvkeyId = NVID_DSP_FW_AG_TB_SCONB;
            }
            break;

        case VOL_MP3:
            percentage = audio_get_percentage_out_mp3(level);
            if (gainType == GAIN_DIGITAL) {
                nvkeyId = NVID_DSP_FW_DG_TB_A2DP;
            } else {
                nvkeyId = NVID_DSP_FW_AG_TB_A2DP;
            }
            break;

        case VOL_PCM:
            percentage = audio_get_percentage_out_pcm(level);
            if (gainType == GAIN_DIGITAL) {
                nvkeyId = NVID_DSP_FW_DG_TB_RT;
            } else {
                nvkeyId = NVID_DSP_FW_AG_TB_RT;
            }
            break;

        case VOL_VP:
            percentage = audio_get_percentage_out_pcm(level);
            if (gainType == GAIN_DIGITAL) {
                nvkeyId = NVID_DSP_FW_DG_TB_VP;
            } else {
                nvkeyId = NVID_DSP_FW_AG_TB_VP;
            }
            break;

        case VOL_LINE_IN:
            percentage = audio_get_percentage_out_lineIN(level);
            if (gainType == GAIN_DIGITAL) {
                nvkeyId = NVID_DSP_FW_DG_TB_LINE;
            } else {
                nvkeyId = NVID_DSP_FW_AG_TB_LINE;
            }
            break;

        case VOL_USB_AUDIO_IN:
            percentage = audio_get_percentage_out_usb_audio(level);
            if (gainType == GAIN_DIGITAL) {
                nvkeyId = NVID_DSP_FW_DG_TB_USB_AUDIO;
            } else {
                nvkeyId = NVID_DSP_FW_AG_TB_USB_AUDIO;
            }
            break;
#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        case VOL_USB_AUDIO_SW_IN:
            percentage = audio_get_percentage_out_usb_audio_sw(level);
            nvkeyId = NVID_DSP_FW_IDGP_TB_SYS_SW;
            break;
#endif
        default:
            audio_src_srv_report("[Sink][Setting]audio_get_gain_out_in_dB Wrong VolType:%d\n", 1, volType);
            return 0;
            break;
    }

    status = nvkey_data_item_length(nvkeyId, &tableSize);
    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_gain_out_in_dB Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pGainTable = (uint8_t *)pvPortMalloc(tableSize);
    status = nvkey_read_data(nvkeyId, pGainTable, &tableSize);
    if (status || !pGainTable) {
        audio_src_srv_report("[Sink][Setting]audio_get_gain_out_in_dB Fail 2. Status:%d pGainTable:0x%x\n", 2, status, pGainTable);
        if (pGainTable) {
            vPortFree(pGainTable);
        }
        return 0;
    }

    totalIndex = audio_get_uint16_value(pGainTable); //the first value of 16 bits is total index.
    index = audio_get_Index(percentage, totalIndex);
    if (gainType == GAIN_DIGITAL) {
        gian_dB = (uint32_t)(int32_t)(int16_t)audio_get_uint16_value(pGainTable + 2 + (index * 2)); //digital out gain value is 16 bits.
    } else {
        gian_dB = audio_get_uint32_value(pGainTable + 2 + (index * 4)); //analog out gain value is 32 bits.
    }
    audio_src_srv_report("[Sink][Setting]audio_get_gain_out_in_dB Type:%d Level[%d] Percentage:%d index:%d (%c) Gain:0x%x\n", 6, volType, level, percentage, index, (gainType == GAIN_DIGITAL) ? 'D' : 'A', gian_dB);

    vPortFree(pGainTable);
    return gian_dB;
}

/*Get Stream In Gain*/
static uint8_t audio_get_max_sound_level_in_hfp()
{
    bt_sink_srv_audio_setting_sco_mic_vol_para_t *pNvdmHfpMicVol = NULL;
    uint8_t totalLevel, status;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_SCO_MIC, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_in_hfp Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmHfpMicVol = (bt_sink_srv_audio_setting_sco_mic_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_SCOMIC, (uint8_t *)pNvdmHfpMicVol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_SCO_MIC

    if (status || !pNvdmHfpMicVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_in_hfp Fail 2. Status:%d pNvdmHfpMicVol:0x%x\n", 2, status, pNvdmHfpMicVol);
        if (pNvdmHfpMicVol) {
            vPortFree(pNvdmHfpMicVol);
        }
        return 0;
    }

    totalLevel = pNvdmHfpMicVol->scoMicVolPara.totalSoundLevel;

    vPortFree(pNvdmHfpMicVol);
    return totalLevel;
}

#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
static uint8_t audio_get_max_sound_level_in_usb_voice_sw()
{
    bt_sink_srv_audio_setting_usb_sw_vol_para_t *pNvdmUsbVoiceSwVol = NULL;
    uint8_t  totalLevel   = 0;
    uint8_t  status;
    uint8_t  defaultLevel = 0;
    uint32_t tableSize    = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_VOICE_SW, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_in_usb_voice_sw Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmUsbVoiceSwVol = (bt_sink_srv_audio_setting_usb_sw_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_USB_VOICE_SW, (uint8_t *)pNvdmUsbVoiceSwVol, &tableSize);

    if (status || !pNvdmUsbVoiceSwVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_in_usb_voice_sw Fail 2. Status:%d pNvdmUsbVoiceSwVol:0x%x\n", 2, status, pNvdmUsbVoiceSwVol);
        if (pNvdmUsbVoiceSwVol) {
            vPortFree(pNvdmUsbVoiceSwVol);
        }
        return 0;
    }

    totalLevel = pNvdmUsbVoiceSwVol->usbVolPara.totalSoundLevel;
    defaultLevel = pNvdmUsbVoiceSwVol->usbVolPara.defaultSoundLevel;

    AUD_USB_VOICE_SW_VOL_OUT_MAX = defaultLevel;
    if (AUD_USB_VOICE_SW_VOL_OUT_DEFAULT > (totalLevel - 1)) { //Gain table max level was totalLevel-1
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_get_max_sound_level_in_usb_voice_sw error 2. default level > Max level ");
    }

    vPortFree(pNvdmUsbVoiceSwVol);
    return totalLevel;
}
#endif

uint8_t audio_get_max_sound_level_in(vol_type_t volType)
{
    uint8_t totalSoundLevel = 0;

    switch (volType) {
        case VOL_HFP:
        case VOL_HFP_NB:
            totalSoundLevel = audio_get_max_sound_level_in_hfp();
            break;
#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        case VOL_USB_VOICE_SW_OUT:
            totalSoundLevel = audio_get_max_sound_level_in_usb_voice_sw();
            break;
#endif
    }

    audio_src_srv_report("[Sink][Setting]audio_get_max_sound_level_in volType:%d totalLevel:%d\n", 2, volType, totalSoundLevel);
    return totalSoundLevel;
}

static uint8_t audio_get_percentage_in_hfp(uint8_t level)
{
    bt_sink_srv_audio_setting_sco_mic_vol_para_t *pNvdmHfpMicVol = NULL;
    uint8_t percentage, totalLevel, status;
    uint8_t *pHfpMicPercentageTable;
    uint32_t tableSize = 0;

    status = nvkey_data_item_length(NVKEY_DSP_PARA_LEVEL_PERCENTAGE_SCO_MIC, &tableSize);

    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_in_hfp Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pNvdmHfpMicVol = (bt_sink_srv_audio_setting_sco_mic_vol_para_t *)pvPortMalloc(tableSize);

    status = nvkey_read_data(NVID_DSP_FW_VOL_CFG_SCOMIC, (uint8_t *)pNvdmHfpMicVol, &tableSize); //NVKEY_DSP_PARA_LEVEL_PERCENTAGE_SCO_MIC

    if (status || !pNvdmHfpMicVol) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_in_hfp Fail 2. Status:%d pNvdmHfpMicVol:0x%x\n", 2, status, pNvdmHfpMicVol);
        if (pNvdmHfpMicVol) {
            vPortFree(pNvdmHfpMicVol);
        }
        return 0;
    }

    totalLevel = pNvdmHfpMicVol->scoMicVolPara.totalSoundLevel;
    if (level >= totalLevel) {
        audio_src_srv_report("[Sink][Setting]audio_get_percentage_in_hfp Invalid Level\n", 0);
        level = totalLevel - 1;
    }

    pHfpMicPercentageTable = ((uint8_t *)pNvdmHfpMicVol) + sizeof(bt_sink_srv_audio_setting_vol_para) + totalLevel/*VGM*/;
    percentage = pHfpMicPercentageTable[level];
    //audio_src_srv_report("[Sink][Setting]audio_get_percentage_in_hfp level[%d] percentage:%d\n", 2, level, percentage);

    vPortFree(pNvdmHfpMicVol);
    return percentage;
}

void audio_get_multi_gain_in_in_dB(gain_type_t gainType, vol_type_t volType, void *vol_entry)
{
    uint32_t tableSize;
    uint8_t  status;
    switch (volType) {
        case VOL_HFP:
        case VOL_HFP_NB: {
            hfp_vol_t *vol = vol_entry;
            if (gainType == GAIN_DIGITAL) {
                /*GAIN_DIGITAL*/
                bt_sink_srv_audio_setting_sco_extend_digital_mic_vol_para_t *pNvdmSCOMicVol_Digital = NULL;
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                bt_sink_srv_audio_setting_mic_func_vol_para_t *pNvdmMicFuncVol_Digtital = NULL;
#endif
                status = nvkey_data_item_length(NVKEY_DSP_PARA_DIN_GP_TABLE_SCO_MULTI_MIC, &tableSize);
                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_HFP) Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    return;
                }
                pNvdmSCOMicVol_Digital = (bt_sink_srv_audio_setting_sco_extend_digital_mic_vol_para_t *)pvPortMalloc(tableSize);
                status = nvkey_read_data(NVID_DSP_FW_IDGP_TB_SCO_MULMIC, (uint8_t *)pNvdmSCOMicVol_Digital, &tableSize);  //NVKEY_DSP_PARA_DIN_GP_TABLE_SCO_MULTI_MIC
                if (status || !pNvdmSCOMicVol_Digital) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_HFP) Fail 2. Status:%d pNvdmVCMicVol:0x%x\n", 2, status, pNvdmSCOMicVol_Digital);
                    if (pNvdmSCOMicVol_Digital) {
                        vPortFree(pNvdmSCOMicVol_Digital);
                    }
                    return;
                }
                /*Assaign Value*/
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                /*read the mic funtion digital gain setting from nvkey*/
                status = nvkey_data_item_length(NVID_DSP_FW_IDG_TB_MIC_FUNC, &tableSize);
                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_HFP) read mic func Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    if (pNvdmSCOMicVol_Digital) {
                        vPortFree(pNvdmSCOMicVol_Digital);
                    }
                    return;
                }
                pNvdmMicFuncVol_Digtital = (bt_sink_srv_audio_setting_mic_func_vol_para_t *)pvPortMalloc(tableSize);
                status = nvkey_read_data(NVID_DSP_FW_IDG_TB_MIC_FUNC, (uint8_t *)pNvdmMicFuncVol_Digtital, &tableSize);  //NVKEY_DSP_PARA_DIN_GP_TABLE_SCO_MULTI_MIC
                if (status || !pNvdmMicFuncVol_Digtital) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_HFP) read mic func Fail 2. Status:%d pNvdmVCMicVol:0x%x\n", 2, status, pNvdmMicFuncVol_Digtital);
                    if (pNvdmMicFuncVol_Digtital) {
                        vPortFree(pNvdmMicFuncVol_Digtital);
                    }
                    if (pNvdmSCOMicVol_Digital) {
                        vPortFree(pNvdmSCOMicVol_Digital);
                    }
                    return;
                }
                //get the multi mic d gain
                vol->vol_multiMIC_in.digital_MIC0_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_MIC0_L_Digital_Vol;
                vol->vol_multiMIC_in.digital_MIC0_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_MIC0_R_Digital_Vol;
                vol->vol_multiMIC_in.digital_MIC1_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_MIC1_L_Digital_Vol;
                vol->vol_multiMIC_in.digital_MIC1_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_MIC1_R_Digital_Vol;
                vol->vol_multiMIC_in.digital_MIC2_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_MIC2_L_Digital_Vol;
                vol->vol_multiMIC_in.digital_MIC2_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_MIC2_R_Digital_Vol;
                vol->vol_multiMIC_in.digital_I2S0_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_I2S0_L_Digital_Vol;
                vol->vol_multiMIC_in.digital_I2S0_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_I2S0_R_Digital_Vol;
                vol->vol_multiMIC_in.digital_I2S1_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_I2S1_L_Digital_Vol;
                vol->vol_multiMIC_in.digital_I2S1_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_I2S1_R_Digital_Vol;
                vol->vol_multiMIC_in.digital_I2S2_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_I2S2_L_Digital_Vol;
                vol->vol_multiMIC_in.digital_I2S2_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_I2S2_R_Digital_Vol;
                vol->vol_multiMIC_in.digital_LINEIN_L_Digital_Vol = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_LINEIN_L_Digital_Vol;
                vol->vol_multiMIC_in.digital_LINEIN_R_Digital_Vol = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_LINEIN_R_Digital_Vol;
                vol->vol_multiMIC_in.digital_Echo_Reference_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_Echo_Reference_Vol;
                //get the mic functions d gain
                vol->vol_multiMIC_in.digital_MIC0_L_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC0_L_Digital_Vol;
                vol->vol_multiMIC_in.digital_MIC0_R_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC0_R_Digital_Vol;
                vol->vol_multiMIC_in.digital_MIC1_L_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC1_L_Digital_Vol;
                vol->vol_multiMIC_in.digital_MIC1_R_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC1_R_Digital_Vol;
                vol->vol_multiMIC_in.digital_MIC2_L_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC2_L_Digital_Vol;
                vol->vol_multiMIC_in.digital_MIC2_R_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC2_R_Digital_Vol;
                vol->vol_multiMIC_in.digital_Echo_Func_Reference_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_Echo_Reference_Vol;
                vPortFree(pNvdmMicFuncVol_Digtital);
#else
                vol->vol_in.digital                  = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_MainMic_Digital_Vol;
                vol->vol_multiMIC_in.digital_Ref1    = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_RefMic_Digital_Vol;
                vol->vol_multiMIC_in.digital_Ref2    = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_Ref2Mic_Digital_Vol;
                vol->vol_multiMIC_in.digital_RESERVE = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_Reserve_Digital_Vol;
                vol->vol_multiMIC_in.digital_Echo    = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Digital->sco_Extended_Echo_Reference_Vol;
#endif
                vPortFree(pNvdmSCOMicVol_Digital);
            } else {
                /*GAIN_Analog*/
                bt_sink_srv_audio_setting_sco_extend_analog_mic_vol_para_t *pNvdmSCOMicVol_Analog = NULL;
                status = nvkey_data_item_length(NVKEY_DSP_PARA_AIN_GP_TABLE_SCO_MULTI_MIC, &tableSize);
                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_HFP) Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    return;
                }
                pNvdmSCOMicVol_Analog = (bt_sink_srv_audio_setting_sco_extend_analog_mic_vol_para_t *)pvPortMalloc(tableSize);
                status = nvkey_read_data(NVID_DSP_FW_IAGP_TB_SCO_ML_MIC, (uint8_t *)pNvdmSCOMicVol_Analog, &tableSize); //NVKEY_DSP_PARA_AIN_GP_TABLE_SCO_MULTI_MIC
                if (status || !pNvdmSCOMicVol_Analog) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_HFP) Fail 2. Status:%d pNvdmVCMicVol:0x%x\n", 2, status, pNvdmSCOMicVol_Analog);
                    if (pNvdmSCOMicVol_Analog) {
                        vPortFree(pNvdmSCOMicVol_Analog);
                    }
                    return;
                }
                /*Assaign Value*/
                vol->vol_in.analog_L = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Analog->sco_Extended_MicAnalogVol_L;
                vol->vol_in.analog_R = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Analog->sco_Extended_MicAnalogVol_R;
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                vol->vol_in.analog_MIC2 = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Analog->sco_Extended_MicAnalogVol_MIC2;
                vol->vol_in.analog_MIC3 = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Analog->sco_Extended_MicAnalogVol_MIC3;
                vol->vol_in.analog_MIC4 = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Analog->sco_Extended_MicAnalogVol_MIC4;
                vol->vol_in.analog_MIC5 = (uint32_t)(int32_t)(int16_t)pNvdmSCOMicVol_Analog->sco_Extended_MicAnalogVol_MIC5;
#endif
                vPortFree(pNvdmSCOMicVol_Analog);
            }
            break;
        }
        case VOL_VC: {
            vc_vol_t *vol = vol_entry;
            if (gainType == GAIN_DIGITAL) {
                /*GAIN_DIGITAL*/
                bt_sink_srv_audio_setting_vc_digital_mic_vol_para_t *pNvdmVCMicVol_Digital = NULL;
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                bt_sink_srv_audio_setting_mic_func_vol_para_t *pNvdmMicFuncVol_Digtital = NULL;
#endif

                status = nvkey_data_item_length(NVKEY_DSP_PARA_DIN_GAINTABLE_VC, &tableSize);
                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_VC) Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    return;
                }
                pNvdmVCMicVol_Digital = (bt_sink_srv_audio_setting_vc_digital_mic_vol_para_t *)pvPortMalloc(tableSize);
                status = nvkey_read_data(NVID_DSP_FW_IDG_TB_VC, (uint8_t *)pNvdmVCMicVol_Digital, &tableSize); //NVKEY_DSP_PARA_DIN_GAINTABLE_VC
                if (status || !pNvdmVCMicVol_Digital) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_VC) Fail 2. Status:%d pNvdmVCMicVol:0x%x\n", 2, status, pNvdmVCMicVol_Digital);
                    if (pNvdmVCMicVol_Digital) {
                        vPortFree(pNvdmVCMicVol_Digital);
                    }
                    return;
                }
                /*Assaign Value*/
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                /*read the mic funtion digital gain setting from nvkey*/
                status = nvkey_data_item_length(NVID_DSP_FW_IDG_TB_MIC_FUNC, &tableSize);
                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_VC) read mic func Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    if (pNvdmVCMicVol_Digital) {
                        vPortFree(pNvdmVCMicVol_Digital);
                    }
                    return;
                }
                pNvdmMicFuncVol_Digtital = (bt_sink_srv_audio_setting_mic_func_vol_para_t *)pvPortMalloc(tableSize);
                status = nvkey_read_data(NVID_DSP_FW_IDG_TB_MIC_FUNC, (uint8_t *)pNvdmMicFuncVol_Digtital, &tableSize);  //NVKEY_DSP_PARA_DIN_GP_TABLE_SCO_MULTI_MIC
                if (status || !pNvdmMicFuncVol_Digtital) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_VC) read mic func Fail 2. Status:%d pNvdmVCMicVol:0x%x\n", 2, status, pNvdmMicFuncVol_Digtital);
                    if (pNvdmMicFuncVol_Digtital) {
                        vPortFree(pNvdmMicFuncVol_Digtital);
                    }
                    vPortFree(pNvdmVCMicVol_Digital);
                    return;
                }
                //get the multi mic d gain
                vol->vol_multiMIC.digital_MIC0_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_MIC0_L_Digital_Vol;
                vol->vol_multiMIC.digital_MIC0_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_MIC0_R_Digital_Vol;
                vol->vol_multiMIC.digital_MIC1_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_MIC1_L_Digital_Vol;
                vol->vol_multiMIC.digital_MIC1_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_MIC1_R_Digital_Vol;
                vol->vol_multiMIC.digital_MIC2_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_MIC2_L_Digital_Vol;
                vol->vol_multiMIC.digital_MIC2_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_MIC2_R_Digital_Vol;
                vol->vol_multiMIC.digital_I2S0_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_I2S0_L_Digital_Vol;
                vol->vol_multiMIC.digital_I2S0_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_I2S0_R_Digital_Vol;
                vol->vol_multiMIC.digital_I2S1_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_I2S1_L_Digital_Vol;
                vol->vol_multiMIC.digital_I2S1_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_I2S1_R_Digital_Vol;
                vol->vol_multiMIC.digital_I2S2_L_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_I2S2_L_Digital_Vol;
                vol->vol_multiMIC.digital_I2S2_R_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_I2S2_R_Digital_Vol;
                vol->vol_multiMIC.digital_LINEIN_L_Digital_Vol = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_LINEIN_L_Digital_Vol;
                vol->vol_multiMIC.digital_LINEIN_R_Digital_Vol = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_LINEIN_R_Digital_Vol;
                vol->vol_multiMIC.digital_Echo_Reference_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_Echo_Reference_Vol;
                //get the mic functions d gain
                vol->vol_multiMIC.digital_MIC0_L_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC0_L_Digital_Vol;
                vol->vol_multiMIC.digital_MIC0_R_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC0_R_Digital_Vol;
                vol->vol_multiMIC.digital_MIC1_L_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC1_L_Digital_Vol;
                vol->vol_multiMIC.digital_MIC1_R_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC1_R_Digital_Vol;
                vol->vol_multiMIC.digital_MIC2_L_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC2_L_Digital_Vol;
                vol->vol_multiMIC.digital_MIC2_R_Func_Digital_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_MIC2_R_Digital_Vol;
                vol->vol_multiMIC.digital_Echo_Func_Reference_Vol   = (uint32_t)(int32_t)(int16_t)pNvdmMicFuncVol_Digtital->MIC_FUNC_Echo_Reference_Vol;
                vPortFree(pNvdmMicFuncVol_Digtital);
#else
                vol->vol.digital               = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_MainMic_Digital_Vol;
                vol->vol_multiMIC.digital_Ref1 = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Digital->VC_RefMic_Digital_Vol;
#endif
                vPortFree(pNvdmVCMicVol_Digital);
            } else {
                /*GAIN_Analog*/
                bt_sink_srv_audio_setting_vc_mic_vol_para_t *pNvdmVCMicVol_Analog = NULL;
                status = nvkey_data_item_length(NVKEY_DSP_PARA_AIN_GAINTABLE_VC, &tableSize);
                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_VC) Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    return;
                }
                pNvdmVCMicVol_Analog = (bt_sink_srv_audio_setting_vc_mic_vol_para_t *)pvPortMalloc(tableSize);
                status = nvkey_read_data(NVID_DSP_FW_IAG_TB_VC, (uint8_t *)pNvdmVCMicVol_Analog, &tableSize); //NVKEY_DSP_PARA_AIN_GAINTABLE_VC
                if (status || !pNvdmVCMicVol_Analog) {
                    audio_src_srv_report("[Sink][Setting]audio_get_multi_gain_in_in_dB(VOL_VC) Fail 2. Status:%d pNvdmVCMicVol_Analog:0x%x\n", 2, status, pNvdmVCMicVol_Analog);
                    if (pNvdmVCMicVol_Analog) {
                        vPortFree(pNvdmVCMicVol_Analog);
                    }
                    return;
                }
                /*Assaign Value*/
                vol->vol.analog_L = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Analog->VC_MicAnalogVol_L;
                vol->vol.analog_R = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Analog->VC_MicAnalogVol_R;
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                vol->vol.analog_MIC2 = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Analog->VC_MicAnalogVol_MIC2;
                vol->vol.analog_MIC3 = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Analog->VC_MicAnalogVol_MIC3;
                vol->vol.analog_MIC4 = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Analog->VC_MicAnalogVol_MIC4;
                vol->vol.analog_MIC5 = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Analog->VC_MicAnalogVol_MIC5;
#endif
                vPortFree(pNvdmVCMicVol_Analog);
            }
            break;
        }
        case VOL_LINE_IN: {
            lineIN_vol_t *vol = vol_entry;
            if (gainType == GAIN_DIGITAL) {
                /*digital gain*/
                uint16_t *pNvdmLineINMicVol = NULL;

                status = nvkey_data_item_length(NVKEY_DSP_PARA_DIN_GAINTABLE_LINE, &tableSize);

                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_DIN_in_lineIN Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    return;
                }
                pNvdmLineINMicVol = (uint16_t *)pvPortMalloc(tableSize);

                status = nvkey_read_data(NVID_DSP_FW_IDG_TB_LINE, (uint8_t *)pNvdmLineINMicVol, &tableSize); //NVKEY_DSP_PARA_DIN_GAINTABLE_LINE

                if (status || !pNvdmLineINMicVol) {
                    audio_src_srv_report("[Sink][Setting]audio_get_DIN_in_lineIN Fail 2. Status:%d pNvdmLineINMicVol:0x%x\n", 2, status, pNvdmLineINMicVol);
                    if (pNvdmLineINMicVol) {
                        vPortFree(pNvdmLineINMicVol);
                    }
                    return;
                }
                vol->vol_in.digital = (uint32_t)(int32_t)(int16_t) * pNvdmLineINMicVol;
                vPortFree(pNvdmLineINMicVol);
            } else {
                /*analog gain*/
                bt_sink_srv_audio_setting_lineIn_mic_vol_para_t *pNvdmLineINMicVol_Analog = NULL;

                status = nvkey_data_item_length(NVKEY_DSP_PARA_AIN_GAINTABLE_LINE, &tableSize);

                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_AIN_in_lineIN Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    return;
                }
                pNvdmLineINMicVol_Analog = (bt_sink_srv_audio_setting_lineIn_mic_vol_para_t *)pvPortMalloc(tableSize);

                status = nvkey_read_data(NVID_DSP_FW_IAG_TB_LINE, (uint8_t *)pNvdmLineINMicVol_Analog, &tableSize); //NVKEY_DSP_PARA_AIN_GAINTABLE_LINE

                if (status || !pNvdmLineINMicVol_Analog) {
                    audio_src_srv_report("[Sink][Setting]audio_get_AIN_in_lineIN Fail 2. Status:%d pNvdmLineINMicVol:0x%x\n", 2, status, pNvdmLineINMicVol_Analog);
                    if (pNvdmLineINMicVol_Analog) {
                        vPortFree(pNvdmLineINMicVol_Analog);
                    }
                    return;
                }
                vol->vol_in.analog_L = (uint32_t)(int32_t)(int16_t)pNvdmLineINMicVol_Analog->lineIn_MicAnalogVol_L;
                vol->vol_in.analog_R = (uint32_t)(int32_t)(int16_t)pNvdmLineINMicVol_Analog->lineIn_MicAnalogVol_R;
                vPortFree(pNvdmLineINMicVol_Analog);
            }
            break;
        }
        default:
            break;
    }
}

uint32_t audio_get_gain_in_in_dB(uint8_t level, gain_type_t gainType, vol_type_t volType)
{
    uint32_t gian_dB, tableSize;
    uint16_t totalIndex, index;
    uint8_t percentage, status;
    uint8_t *pGainTable;
    uint16_t nvkeyId;

    switch (volType) {
        case VOL_HFP:
            percentage = audio_get_percentage_in_hfp(level);
            if (gainType == GAIN_DIGITAL) {
                nvkeyId = NVID_DSP_FW_IDGP_TB_SCO;
            } else {
                nvkeyId = NVID_DSP_FW_IAGP_TB_SCO_L;
            }
            break;

        case VOL_HFP_NB:
            percentage = audio_get_percentage_in_hfp(level);
            if (gainType == GAIN_DIGITAL) {
                nvkeyId = NVID_DSP_FW_IDGP_TB_SCONB;
            } else {
                nvkeyId = NVID_DSP_FW_IAGP_TB_SCONB_L;
            }
            break;

        case VOL_VC:
            if (gainType == GAIN_DIGITAL) {
                bt_sink_srv_audio_setting_vc_digital_mic_vol_para_t *pNvdmVCMicVol = NULL;

                status = nvkey_data_item_length(NVKEY_DSP_PARA_DIN_GAINTABLE_VC, &tableSize);

                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_DIN_in_VC Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    return 0;
                }
                pNvdmVCMicVol = (bt_sink_srv_audio_setting_vc_digital_mic_vol_para_t *)pvPortMalloc(tableSize);

                status = nvkey_read_data(NVID_DSP_FW_IDG_TB_VC, (uint8_t *)pNvdmVCMicVol, &tableSize); //NVKEY_DSP_PARA_DIN_GAINTABLE_VC

                if (status || !pNvdmVCMicVol) {
                    audio_src_srv_report("[Sink][Setting]audio_get_DIN_in_VC Fail 2. Status:%d pNvdmVCMicVol:0x%x\n", 2, status, pNvdmVCMicVol);
                    if (pNvdmVCMicVol) {
                        vPortFree(pNvdmVCMicVol);
                    }
                    return 0;
                }
#ifdef MTK_AUDIO_GAIN_SETTING_ENHANCE
                if (audio_nvdm_HW_config.voice_scenario.Voice_Analog_MIC_Sel != 0xFF) {
                    switch (audio_nvdm_HW_config.voice_scenario.Voice_Analog_MIC_Sel) {
                        case 0x00:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC0_L_Digital_Vol;
                            break;
                        case 0x01:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC0_R_Digital_Vol;
                            break;
                        case 0x02:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC1_L_Digital_Vol;
                            break;
                        case 0x03:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC1_R_Digital_Vol;
                            break;
                        case 0x04:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC2_L_Digital_Vol;
                            break;
                        case 0x05:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC2_R_Digital_Vol;
                            break;
                        default:
                            audio_src_srv_report("[AM] 1 Amic setting wrong", 0);
                            if (pNvdmVCMicVol) {
                                vPortFree(pNvdmVCMicVol);
                            }
                            return 0;
                    }
                } else if (audio_nvdm_HW_config.voice_scenario.Voice_Digital_MIC_Sel != 0xFF) {
                    switch (audio_nvdm_HW_config.voice_scenario.Voice_Digital_MIC_Sel) {
                        case 0x08:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC0_L_Digital_Vol;
                            break;
                        case 0x09:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC0_R_Digital_Vol;
                            break;
                        case 0x0A:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC1_L_Digital_Vol;
                            break;
                        case 0x0B:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC1_R_Digital_Vol;
                            break;
                        case 0x0C:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC2_L_Digital_Vol;
                            break;
                        case 0x0D:
                            gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MIC2_R_Digital_Vol;
                            break;
                        default:
                            audio_src_srv_report("[AM] 1 Dmic setting wrong", 0);
                            if (pNvdmVCMicVol) {
                                vPortFree(pNvdmVCMicVol);
                            }
                            return 0;
                    }
                } else {
                    audio_src_srv_report("[AM] 1 mic setting wrong", 0);
                    if (pNvdmVCMicVol) {
                        vPortFree(pNvdmVCMicVol);
                    }
                    return 0;
                }
#else
                gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol->VC_MainMic_Digital_Vol;
#endif
                if (pNvdmVCMicVol) {
                    vPortFree(pNvdmVCMicVol);
                }
            } else {
                bt_sink_srv_audio_setting_vc_mic_vol_para_t *pNvdmVCMicVol_Analog = NULL;

                status = nvkey_data_item_length(NVKEY_DSP_PARA_AIN_GAINTABLE_VC, &tableSize);

                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_AIN_in_VC Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    return 0;
                }
                pNvdmVCMicVol_Analog = (bt_sink_srv_audio_setting_vc_mic_vol_para_t *)pvPortMalloc(tableSize);

                status = nvkey_read_data(NVID_DSP_FW_IAG_TB_VC, (uint8_t *)pNvdmVCMicVol_Analog, &tableSize); //NVKEY_DSP_PARA_AIN_GAINTABLE_VC

                if (status || !pNvdmVCMicVol_Analog) {
                    audio_src_srv_report("[Sink][Setting]audio_get_AIN_in_VC Fail 2. Status:%d pNvdmVCMicVol_Analog:0x%x\n", 2, status, pNvdmVCMicVol_Analog);
                    if (pNvdmVCMicVol_Analog) {
                        vPortFree(pNvdmVCMicVol_Analog);
                    }
                    return 0;
                }
                gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmVCMicVol_Analog->VC_MicAnalogVol_L;
                vPortFree(pNvdmVCMicVol_Analog);
            }
            return gian_dB;
            break;

        case VOL_LINE_IN:
            if (gainType == GAIN_DIGITAL) {
                uint16_t *pNvdmLineINMicVol = NULL;

                status = nvkey_data_item_length(NVKEY_DSP_PARA_DIN_GAINTABLE_LINE, &tableSize);

                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_DIN_in_lineIN Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    return 0;
                }
                pNvdmLineINMicVol = (uint16_t *)pvPortMalloc(tableSize);

                status = nvkey_read_data(NVID_DSP_FW_IDG_TB_LINE, (uint8_t *)pNvdmLineINMicVol, &tableSize); //NVKEY_DSP_PARA_DIN_GAINTABLE_LINE

                if (status || !pNvdmLineINMicVol) {
                    audio_src_srv_report("[Sink][Setting]audio_get_DIN_in_lineIN Fail 2. Status:%d pNvdmLineINMicVol:0x%x\n", 2, status, pNvdmLineINMicVol);
                    if (pNvdmLineINMicVol) {
                        vPortFree(pNvdmLineINMicVol);
                    }
                    return 0;
                }
                gian_dB = (uint32_t)(int32_t)(int16_t) * pNvdmLineINMicVol;
                vPortFree(pNvdmLineINMicVol);
            } else {
                bt_sink_srv_audio_setting_lineIn_mic_vol_para_t *pNvdmLineINMicVol_Analog = NULL;

                status = nvkey_data_item_length(NVKEY_DSP_PARA_AIN_GAINTABLE_LINE, &tableSize);

                if (status || !tableSize) {
                    audio_src_srv_report("[Sink][Setting]audio_get_AIN_in_lineIN Fail. Status:%d Len:%d\n", 2, status, tableSize);
                    return 0;
                }
                pNvdmLineINMicVol_Analog = (bt_sink_srv_audio_setting_lineIn_mic_vol_para_t *)pvPortMalloc(tableSize);

                status = nvkey_read_data(NVID_DSP_FW_IAG_TB_LINE, (uint8_t *)pNvdmLineINMicVol_Analog, &tableSize); //NVKEY_DSP_PARA_AIN_GAINTABLE_LINE

                if (status || !pNvdmLineINMicVol_Analog) {
                    audio_src_srv_report("[Sink][Setting]audio_get_AIN_in_lineIN Fail 2. Status:%d pNvdmLineINMicVol:0x%x\n", 2, status, pNvdmLineINMicVol_Analog);
                    if (pNvdmLineINMicVol_Analog) {
                        vPortFree(pNvdmLineINMicVol_Analog);
                    }
                    return 0;
                }
                gian_dB = (uint32_t)(int32_t)(int16_t)pNvdmLineINMicVol_Analog->lineIn_MicAnalogVol_L;
                vPortFree(pNvdmLineINMicVol_Analog);
            }
            return gian_dB;
            break;
#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        case VOL_USB_VOICE_SW_OUT:
            percentage = audio_get_percentage_in_usb_voice_sw(level);
            nvkeyId = NVID_DSP_FW_IDGP_TB_SYS_SW;
            break;
#endif
        default:
            audio_src_srv_report("[Sink][Setting]audio_get_gain_in_in_dB Wrong VolType:%d\n", 1, volType);
            return 0;
            break;
    }

    status = nvkey_data_item_length(nvkeyId, &tableSize);
    if (status || !tableSize) {
        audio_src_srv_report("[Sink][Setting]audio_get_gain_in_in_dB Fail. Status:%d Len:%d\n", 2, status, tableSize);
        return 0;
    }

    pGainTable = (uint8_t *)pvPortMalloc(tableSize);
    status = nvkey_read_data(nvkeyId, pGainTable, &tableSize);
    if (status || !pGainTable) {
        audio_src_srv_report("[Sink][Setting]audio_get_gain_in_in_dB Fail 2. Status:%d pGainTable:0x%x\n", 2, status, pGainTable);
        if (pGainTable) {
            vPortFree(pGainTable);
        }
        return 0;
    }

    totalIndex = audio_get_uint16_value(pGainTable); //the first value of 16 bits is total index.
    index = audio_get_Index(percentage, totalIndex);
    gian_dB = (uint32_t)(int32_t)(int16_t)audio_get_uint16_value(pGainTable + 2 + (index * 2)); //digital in and analog in gain value are 16 bits.
    audio_src_srv_report("[Sink][Setting]audio_get_gain_in_in_dB Type:%d Level[%d] Percentage:%d index:%d (%c) Gain:0x%x\n", 6, volType, level, percentage, index, (gainType == GAIN_DIGITAL) ? 'D' : 'A', gian_dB);

    vPortFree(pGainTable);
    return gian_dB;
}
#endif //__GAIN_TABLE_NVDM_DIRECT__

static void bt_sink_srv_audio_setting_get_a2dp_vol(a2dp_vol_info_t *info,
                                                   a2dp_vol_t *vol)
{
    uint8_t lev = info->lev;
    uint32_t digital = 0;
    uint32_t analog = 0;

    digital = audio_get_gain_out_in_dB(lev, GAIN_DIGITAL, VOL_A2DP);
    analog = audio_get_gain_out_in_dB(lev, GAIN_ANALOG, VOL_A2DP);

    vol->vol.digital = digital;
    vol->vol.analog_L = analog;
    audio_src_srv_report("[Sink][Setting]get_a2dp_vol--dev: 0x%x, lev: %d, d: 0x%x, a: 0x%x", 4,
                         info->dev, lev, digital, analog);
}


static void bt_sink_srv_audio_setting_get_usb_audio_vol(usb_audio_vol_info_t *info,
                                                        usb_audio_vol_t *vol)
{
    uint8_t lev = info->lev;
    uint32_t digital = 0;
    uint32_t analog = 0;

    digital = audio_get_gain_out_in_dB(lev, GAIN_DIGITAL, VOL_USB_AUDIO_IN);
    analog = audio_get_gain_out_in_dB(lev, GAIN_ANALOG, VOL_USB_AUDIO_IN);

    vol->vol.digital = digital;
    vol->vol.analog_L = analog;
    audio_src_srv_report("[Sink][Setting]get_usb_audio_vol--dev: 0x%x, lev: %d, d: 0x%x, a: 0x%x", 4,
                         info->dev, lev, digital, analog);
}

static void bt_sink_srv_audio_setting_get_hfp_vol(hfp_vol_info_t *info,
                                                  hfp_vol_t *vol)
{
    device_t dev_out = info->dev_out;
    device_t dev_in = info->dev_in;
    uint8_t lev_in = info->lev_in;
    uint8_t lev_out = info->lev_out;
    uint8_t codec = info->codec;
    int32_t ret = AUDIO_SETTING_ERR_SUCCESS_OK;
    uint32_t digital = 0;
    uint32_t analog = 0;
    uint8_t mode = 0;
    hfp_vol_t temp_vol = {{0}};

    if (BT_HFP_CODEC_TYPE_CVSD == codec) {
        if (HAL_AUDIO_DEVICE_HEADSET == dev_out &&
            ((HAL_AUDIO_DEVICE_HEADSET_MIC == dev_in) || (HAL_AUDIO_DEVICE_MAIN_MIC == dev_in))) {
            mode = SPEECH_MODE_HEADSET_HEADSET_MIC;
            /* vol in */
#if !defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)

            digital = audio_get_gain_in_in_dB(lev_in, GAIN_DIGITAL, VOL_HFP_NB);
            analog = audio_get_gain_in_in_dB(lev_in, GAIN_ANALOG, VOL_HFP_NB);

            temp_vol.vol_in.digital = digital;
            temp_vol.vol_in.analog_L = analog;
#else
            audio_get_multi_gain_in_in_dB(GAIN_DIGITAL, VOL_HFP_NB, &temp_vol);
            audio_get_multi_gain_in_in_dB(GAIN_ANALOG, VOL_HFP_NB, &temp_vol);
#endif
            /* vol out */
            digital = audio_get_gain_out_in_dB(lev_out, GAIN_DIGITAL, VOL_HFP_NB);
            analog = audio_get_gain_out_in_dB(lev_out, GAIN_ANALOG, VOL_HFP_NB);

            temp_vol.vol_out.digital = digital;
            temp_vol.vol_out.analog_L = analog;
            ret = AUDIO_SETTING_ERR_SUCCESS_4TH;
        } else if (HAL_AUDIO_DEVICE_HANDS_FREE_STEREO == dev_out &&
                   HAL_AUDIO_DEVICE_MAIN_MIC == dev_in) {
            mode = SPEECH_MODE_HANDSFREE_MAIN_MIC;
            /* vol in */
#if !defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            digital = audio_get_gain_in_in_dB(lev_in, GAIN_DIGITAL, VOL_HFP_NB);
            analog = audio_get_gain_in_in_dB(lev_in, GAIN_ANALOG, VOL_HFP_NB);
            temp_vol.vol_in.digital = digital;
            temp_vol.vol_in.analog_L = analog;
#else
            audio_get_multi_gain_in_in_dB(GAIN_DIGITAL, VOL_HFP_NB, &temp_vol);
            audio_get_multi_gain_in_in_dB(GAIN_ANALOG, VOL_HFP_NB, &temp_vol);
#endif
            /* vol out */
            digital = audio_get_gain_out_in_dB(lev_out, GAIN_DIGITAL, VOL_HFP_NB);
            analog = audio_get_gain_out_in_dB(lev_out, GAIN_ANALOG, VOL_HFP_NB);
            temp_vol.vol_out.digital = digital;
            temp_vol.vol_out.analog_L = analog;
            ret = AUDIO_SETTING_ERR_SUCCESS_5TH;
        } else if (HAL_AUDIO_DEVICE_HEADSET == dev_out &&
                   HAL_AUDIO_DEVICE_SINGLE_DIGITAL_MIC == dev_in) {
            mode = SPEECH_MODE_HEADSET_DIGITAL_MIC;
            /* vol in */
#if !defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            digital = audio_get_gain_in_in_dB(lev_in, GAIN_DIGITAL, VOL_HFP_NB);
            analog = audio_get_gain_in_in_dB(lev_in, GAIN_ANALOG, VOL_HFP_NB);
            temp_vol.vol_in.digital = digital;
            temp_vol.vol_in.analog_L = analog;
#else
            audio_get_multi_gain_in_in_dB(GAIN_DIGITAL, VOL_HFP_NB, &temp_vol);
            audio_get_multi_gain_in_in_dB(GAIN_ANALOG, VOL_HFP_NB, &temp_vol);
#endif
            /* vol out */
            digital = audio_get_gain_out_in_dB(lev_out, GAIN_DIGITAL, VOL_HFP_NB);
            analog = audio_get_gain_out_in_dB(lev_out, GAIN_ANALOG, VOL_HFP_NB);
            temp_vol.vol_out.digital = digital;
            temp_vol.vol_out.analog_L = analog;
            ret = AUDIO_SETTING_ERR_SUCCESS_6TH;
        } else {
            ret = AUDIO_SETTING_ERR_FAIL_2ND;
        }
    } else if (BT_HFP_CODEC_TYPE_MSBC == codec) {
        if (HAL_AUDIO_DEVICE_HEADSET == dev_out &&
            ((HAL_AUDIO_DEVICE_HEADSET_MIC == dev_in) || (HAL_AUDIO_DEVICE_MAIN_MIC == dev_in))) {
            mode = SPEECH_MODE_HEADSET_HEADSET_MIC;
            /* vol in */
#if !defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            digital = audio_get_gain_in_in_dB(lev_in, GAIN_DIGITAL, VOL_HFP);
            analog = audio_get_gain_in_in_dB(lev_in, GAIN_ANALOG, VOL_HFP);
            temp_vol.vol_in.digital = digital;
            temp_vol.vol_in.analog_L = analog;
#else
            audio_get_multi_gain_in_in_dB(GAIN_DIGITAL, VOL_HFP, &temp_vol);
            audio_get_multi_gain_in_in_dB(GAIN_ANALOG, VOL_HFP, &temp_vol);
#endif
            /* vol out */
            digital = audio_get_gain_out_in_dB(lev_out, GAIN_DIGITAL, VOL_HFP);
            analog = audio_get_gain_out_in_dB(lev_out, GAIN_ANALOG, VOL_HFP);
            temp_vol.vol_out.digital = digital;
            temp_vol.vol_out.analog_L = analog;
            ret = AUDIO_SETTING_ERR_SUCCESS_4TH;
        } else if (HAL_AUDIO_DEVICE_HANDS_FREE_STEREO == dev_out &&
                   HAL_AUDIO_DEVICE_MAIN_MIC == dev_in) {
            mode = SPEECH_MODE_HANDSFREE_MAIN_MIC;
            /* vol in */
#if !defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            digital = audio_get_gain_in_in_dB(lev_in, GAIN_DIGITAL, VOL_HFP);
            analog = audio_get_gain_in_in_dB(lev_in, GAIN_ANALOG, VOL_HFP);
            temp_vol.vol_in.digital = digital;
            temp_vol.vol_in.analog_L = analog;
#else
            audio_get_multi_gain_in_in_dB(GAIN_DIGITAL, VOL_HFP, &temp_vol);
            audio_get_multi_gain_in_in_dB(GAIN_ANALOG, VOL_HFP, &temp_vol);
#endif
            /* vol out */
            digital = audio_get_gain_out_in_dB(lev_out, GAIN_DIGITAL, VOL_HFP);
            analog = audio_get_gain_out_in_dB(lev_out, GAIN_ANALOG, VOL_HFP);
            temp_vol.vol_out.digital = digital;
            temp_vol.vol_out.analog_L = analog;
            ret = AUDIO_SETTING_ERR_SUCCESS_5TH;
        } else if (HAL_AUDIO_DEVICE_HEADSET == dev_out &&
                   HAL_AUDIO_DEVICE_SINGLE_DIGITAL_MIC == dev_in) {
            mode = SPEECH_MODE_HEADSET_DIGITAL_MIC;
            /* vol in */
#if !defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            digital = audio_get_gain_in_in_dB(lev_in, GAIN_DIGITAL, VOL_HFP);
            analog = audio_get_gain_in_in_dB(lev_in, GAIN_ANALOG, VOL_HFP);
            temp_vol.vol_in.digital = digital;
            temp_vol.vol_in.analog_L = analog;
#else
            audio_get_multi_gain_in_in_dB(GAIN_DIGITAL, VOL_HFP, &temp_vol);
            audio_get_multi_gain_in_in_dB(GAIN_ANALOG, VOL_HFP, &temp_vol);
#endif
            /* vol out */
            digital = audio_get_gain_out_in_dB(lev_out, GAIN_DIGITAL, VOL_HFP);
            analog = audio_get_gain_out_in_dB(lev_out, GAIN_ANALOG, VOL_HFP);
            temp_vol.vol_out.digital = digital;
            temp_vol.vol_out.analog_L = analog;
            ret = AUDIO_SETTING_ERR_SUCCESS_6TH;
        } else {
            ret = AUDIO_SETTING_ERR_FAIL_2ND;
        }
    } else {
        ret = AUDIO_SETTING_ERR_FAIL_7TH;
    }

    memcpy(vol, &temp_vol, sizeof(hfp_vol_t));

    audio_src_srv_report("[Sink][Setting]get_hfp_vol--dev_in: 0x%x, dev_out: 0x%x, lev_in: %d, lev_out: %d, codec: %d", 5,
                         info->dev_in, info->dev_out, info->lev_in, info->lev_out, codec);
    audio_src_srv_report("[Sink][Setting]get_hfp_vol--ret: %d, mode: %d, d-in: 0x%x, a-in: 0x%x, d-out: 0x%x, a-out: 0x%x", 6,
                         ret, mode, vol->vol_in.digital, vol->vol_in.analog_L, vol->vol_out.digital, vol->vol_out.analog_L);
    UNUSED(codec);
    UNUSED(ret);
    UNUSED(mode);
    UNUSED(lev_in);
}


static void bt_sink_srv_audio_setting_get_pcm_vol(pcm_vol_info_t *info,
                                                  pcm_vol_t *vol)
{
    uint8_t lev = info->lev;
    uint32_t digital = 0;
    uint32_t analog = 0;

    digital = audio_get_gain_out_in_dB(lev, GAIN_DIGITAL, VOL_PCM);
    analog = audio_get_gain_out_in_dB(lev, GAIN_ANALOG, VOL_PCM);

    vol->vol.digital = digital;
    vol->vol.analog_L = analog;
    audio_src_srv_report("[Sink][Setting]get_pcm_vol--dev: 0x%x, lev: %d, ret: %d, d: 0x%x, a: 0x%x", 4,
                         info->dev, lev, digital, analog);
}


static void bt_sink_srv_audio_setting_get_mp3_vol(mp3_vol_info_t *info,
                                                  mp3_vol_t *vol)
{
    uint8_t lev = info->lev;
    uint32_t digital = 0;
    uint32_t analog = 0;

    digital = audio_get_gain_out_in_dB(lev, GAIN_DIGITAL, VOL_MP3);
    analog = audio_get_gain_out_in_dB(lev, GAIN_ANALOG, VOL_MP3);

    vol->vol.digital = digital;
    vol->vol.analog_L = analog;
    audio_src_srv_report("[Sink][Setting]get_mp3_vol--dev: 0x%x, lev: %d, ret: %d, d: 0x%x, a: 0x%x", 4,
                         info->dev, lev, digital, analog);
}


static void bt_sink_srv_audio_setting_get_vp_vol(vp_vol_info_t *info,
                                                 vp_vol_t *vol)
{
    uint8_t lev = info->lev;
    uint32_t digital = 0;
    uint32_t analog = 0;

    digital = audio_get_gain_out_in_dB(lev, GAIN_DIGITAL, VOL_VP);
    analog = audio_get_gain_out_in_dB(lev, GAIN_ANALOG, VOL_VP);

    vol->vol.digital = digital;
    vol->vol.analog_L = analog;
    audio_src_srv_report("[Sink][Setting]get_vp_vol--lev: %d, ret: %d, d: 0x%x, a: 0x%x", 3,
                         lev,  digital, analog);
}

static void bt_sink_srv_audio_setting_get_vc_vol(vc_vol_info_t *info,
                                                 vc_vol_t *vol)
{
#if !defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
    uint8_t lev = info->lev_in;
    uint32_t digital = 0;
    uint32_t analog = 0;

    digital = audio_get_gain_in_in_dB(lev, GAIN_DIGITAL, VOL_VC);
    analog = audio_get_gain_in_in_dB(lev, GAIN_ANALOG, VOL_VC);

    vol->vol.digital = digital;
    vol->vol.analog_L = analog;
    audio_src_srv_report("[Sink][Setting]get_vc_vol--lev: %d, ret: %d, d: 0x%x, a: 0x%x", 3,
                         lev, digital, analog);
#else
    audio_get_multi_gain_in_in_dB(GAIN_DIGITAL, VOL_VC, vol);
    audio_get_multi_gain_in_in_dB(GAIN_ANALOG, VOL_VC, vol);
#endif
}

static void bt_sink_srv_audio_setting_get_lineIN_vol(lineIN_vol_info_t *info,
                                                     lineIN_vol_t *vol)
{
    device_t dev_out = info->dev_out;
    device_t dev_in = info->dev_in;
    uint8_t lev_in = info->lev_in;
    uint8_t lev_out = info->lev_out;
    int32_t ret = AUDIO_SETTING_ERR_SUCCESS_OK;
    uint32_t digital = 0;
    uint32_t analog = 0;
    uint8_t mode = 0;
    lineIN_vol_t temp_vol = {{0}};

    lev_in = lev_in;

    if (HAL_AUDIO_DEVICE_HEADSET == dev_out &&
        ((HAL_AUDIO_DEVICE_HEADSET_MIC == dev_in) || (HAL_AUDIO_DEVICE_MAIN_MIC == dev_in))) {
        mode = SPEECH_MODE_HEADSET_HEADSET_MIC;
        /* vol in */

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE) && defined(MTK_AUDIO_GAIN_SETTING_ENHANCE)
        audio_get_multi_gain_in_in_dB(GAIN_DIGITAL, VOL_LINE_IN, &temp_vol);
        audio_get_multi_gain_in_in_dB(GAIN_ANALOG, VOL_LINE_IN, &temp_vol);
#else
        digital = audio_get_gain_in_in_dB(lev_in, GAIN_DIGITAL, VOL_LINE_IN);
        temp_vol.vol_in.digital = digital;
        analog = audio_get_gain_in_in_dB(lev_in, GAIN_ANALOG, VOL_LINE_IN);
        temp_vol.vol_in.analog_L = analog;
#endif
        /* vol out */

        digital = audio_get_gain_out_in_dB(lev_out, GAIN_DIGITAL, VOL_LINE_IN);
        analog = audio_get_gain_out_in_dB(lev_out, GAIN_ANALOG, VOL_LINE_IN);
        temp_vol.vol_out.digital = digital;
        temp_vol.vol_out.analog_L = analog;
        ret = AUDIO_SETTING_ERR_SUCCESS_4TH;
    } else if (HAL_AUDIO_DEVICE_HANDS_FREE_STEREO == dev_out &&
               HAL_AUDIO_DEVICE_MAIN_MIC == dev_in) {
        mode = SPEECH_MODE_HANDSFREE_MAIN_MIC;
        /* vol in */

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE) && defined(MTK_AUDIO_GAIN_SETTING_ENHANCE)
        audio_get_multi_gain_in_in_dB(GAIN_DIGITAL, VOL_LINE_IN, &temp_vol);
        audio_get_multi_gain_in_in_dB(GAIN_ANALOG, VOL_LINE_IN, &temp_vol);
#else
        digital = audio_get_gain_in_in_dB(lev_in, GAIN_DIGITAL, VOL_LINE_IN);
        temp_vol.vol_in.digital = digital;
        analog = audio_get_gain_in_in_dB(lev_in, GAIN_ANALOG, VOL_LINE_IN);
        temp_vol.vol_in.analog_L = analog;
#endif
        /* vol out */

        digital = audio_get_gain_out_in_dB(lev_out, GAIN_DIGITAL, VOL_LINE_IN);
        analog = audio_get_gain_out_in_dB(lev_out, GAIN_ANALOG, VOL_LINE_IN);
        temp_vol.vol_out.digital = digital;
        temp_vol.vol_out.analog_L = analog;
        ret = AUDIO_SETTING_ERR_SUCCESS_5TH;
    } else if (HAL_AUDIO_DEVICE_HEADSET == dev_out &&
               HAL_AUDIO_DEVICE_SINGLE_DIGITAL_MIC == dev_in) {
        mode = SPEECH_MODE_HEADSET_DIGITAL_MIC;
        /* vol in */

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE) && defined(MTK_AUDIO_GAIN_SETTING_ENHANCE)
        audio_get_multi_gain_in_in_dB(GAIN_DIGITAL, VOL_LINE_IN, &temp_vol);
        audio_get_multi_gain_in_in_dB(GAIN_ANALOG, VOL_LINE_IN, &temp_vol);
#else
        digital = audio_get_gain_in_in_dB(lev_in, GAIN_DIGITAL, VOL_LINE_IN);
        temp_vol.vol_in.digital = digital;
        analog = audio_get_gain_in_in_dB(lev_in, GAIN_ANALOG, VOL_LINE_IN);
        temp_vol.vol_in.analog_L = analog;
#endif
        /* vol out */

        digital = audio_get_gain_out_in_dB(lev_out, GAIN_DIGITAL, VOL_LINE_IN);
        analog = audio_get_gain_out_in_dB(lev_out, GAIN_ANALOG, VOL_LINE_IN);
        temp_vol.vol_out.digital = digital;
        temp_vol.vol_out.analog_L = analog;
        ret = AUDIO_SETTING_ERR_SUCCESS_6TH;
    } else {
        ret = AUDIO_SETTING_ERR_FAIL_2ND;
    }

    memcpy(vol, &temp_vol, sizeof(lineIN_vol_t));

    audio_src_srv_report("[Sink][Setting]get_lineIN_vol--dev_in: 0x%x, dev_out: 0x%x, lev_in: %d, lev_out: %d", 5,
                         info->dev_in, info->dev_out, info->lev_in, info->lev_out);
    audio_src_srv_report("[Sink][Setting]get_lineIN_vol--ret: %d, mode: %d, d-in: 0x%x, a-in: 0x%x, d-out: 0x%x, a-out: 0x%x", 6,
                         ret, mode, vol->vol_in.digital, vol->vol_in.analog_L, vol->vol_out.digital, vol->vol_out.analog_L);
    UNUSED(ret);
    UNUSED(mode);
}

void bt_sink_srv_audio_setting_init_vol_level(void)
{
    /*A2DP*/
    AUD_A2DP_VOL_OUT_MAX = audio_get_max_sound_level_out(VOL_A2DP) - 1;
    if ((AUD_A2DP_VOL_OUT_MAX == 0) || (AUD_A2DP_VOL_OUT_MAX > 100)) { //Gain table max level was 100
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_init_vol_a2dp error 2. Max level=0 or more than 100");
    }
    //audio_src_srv_report("[Sink][Setting]A2DP MAX(%d), Default(%d)", 2, AUD_A2DP_VOL_OUT_MAX, AUD_A2DP_VOL_OUT_DEFAULT);
    /*HFP*/
//ToDo if need

    /*VP*/
    AUD_VPRT_VOL_OUT_MAX = audio_get_max_sound_level_out(VOL_VP) - 1;
    if ((AUD_VPRT_VOL_OUT_MAX == 0) || (AUD_VPRT_VOL_OUT_MAX > 100)) { //Gain table max level was 15
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_init_vol_vp error 2. Max level=0 or more than 100");
    }
    //audio_src_srv_report("[Sink][Setting]VP MAX(%d), Default(%d)", 2, AUD_VPRT_VOL_OUT_MAX, AUD_VPRT_VOL_OUT_DEFAULT);
    /*LineIN*/
    AUD_LINEIN_VOL_OUT_MAX = audio_get_max_sound_level_out(VOL_LINE_IN) - 1;
    if ((AUD_LINEIN_VOL_OUT_MAX == 0) || (AUD_LINEIN_VOL_OUT_MAX > 100)) { //Gain table max level was 100
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_init_vol_lineIN error 2. Max level=0 or more than 100");
    }

    /*USB AUDIO*/
#ifdef MTK_USB_AUDIO_PLAYBACK_ENABLE
    AUD_USB_AUDIO_VOL_OUT_MAX = audio_get_max_sound_level_out(VOL_USB_AUDIO_IN) - 1;
    if ((AUD_USB_AUDIO_VOL_OUT_MAX == 0) || (AUD_USB_AUDIO_VOL_OUT_MAX > 100)) { //Gain table max level was 100
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_init_vol_USB_AUDIO error 2. Max level=0 or more than 100");
    }
#endif

#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    AUD_USB_VOICE_SW_VOL_OUT_MAX = audio_get_max_sound_level_in(VOL_USB_VOICE_SW_OUT) - 1;
    if ((AUD_USB_VOICE_SW_VOL_OUT_MAX == 0) || (AUD_USB_VOICE_SW_VOL_OUT_MAX > 100)) { //Gain table max level was 100
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_init_vol_USB_VOICE_SW_OUT error 2. Max level=0 or more than 100");
    }
    AUD_USB_AUDIO_SW_VOL_OUT_MAX = audio_get_max_sound_level_out(VOL_USB_AUDIO_SW_IN) - 1;
    if ((AUD_USB_AUDIO_SW_VOL_OUT_MAX == 0) || (AUD_USB_AUDIO_SW_VOL_OUT_MAX > 100)) { //Gain table max level was 100
        AUDIO_ASSERT(0 && "[Sink][Setting]audio_init_vol_USB_AUDIO_SW_IN error 2. Max level=0 or more than 100");
    }
#endif
}

int32_t bt_sink_srv_audio_setting_get_vol(bt_sink_srv_audio_setting_vol_info_t *vol_info,
                                          bt_sink_srv_audio_setting_vol_t *vol)
{
    vol_type_t type;

    type = vol_info->type;

        switch (type) {
            case VOL_A2DP: {
                vol->type = VOL_A2DP;
                bt_sink_srv_audio_setting_get_a2dp_vol(&(vol_info->vol_info.a2dp_vol_info), &(vol->vol.a2dp_vol));
                break;
            }

            case VOL_HFP: {
                vol->type = VOL_HFP;
                bt_sink_srv_audio_setting_get_hfp_vol(&(vol_info->vol_info.hfp_vol_info), &(vol->vol.hfp_vol));
                break;
            }

            case VOL_PCM: {
                vol->type = VOL_PCM;
                bt_sink_srv_audio_setting_get_pcm_vol(&(vol_info->vol_info.pcm_vol_info), &(vol->vol.pcm_vol));
                break;
            }

            case VOL_MP3: {
                vol->type = VOL_MP3;
                bt_sink_srv_audio_setting_get_mp3_vol(&(vol_info->vol_info.mp3_vol_info), &(vol->vol.mp3_vol));
                break;
            }

            case VOL_DEF: {
                vol->type = VOL_DEF;
                bt_sink_srv_audio_setting_get_mp3_vol(&(vol_info->vol_info.mp3_vol_info), &(vol->vol.mp3_vol));
                break;
            }

            case VOL_VP: {
                vol->type = VOL_VP;
                bt_sink_srv_audio_setting_get_vp_vol(&(vol_info->vol_info.vp_vol_info), &(vol->vol.vp_vol));
                break;
            }

            case VOL_VC: {
                vol->type = VOL_VC;
                bt_sink_srv_audio_setting_get_vc_vol(&(vol_info->vol_info.vc_vol_info), &(vol->vol.vc_vol));
                break;
            }

            case VOL_LINE_IN: {
                vol->type = VOL_LINE_IN;
                bt_sink_srv_audio_setting_get_lineIN_vol(&(vol_info->vol_info.lineIN_vol_info), &(vol->vol.lineIN_vol));
                break;
            }

            case VOL_USB_AUDIO_IN: {
                vol->type = VOL_USB_AUDIO_IN;
                bt_sink_srv_audio_setting_get_usb_audio_vol(&(vol_info->vol_info.usb_audio_vol_info), &(vol->vol.usb_audio_vol));
                break;
            }

            case VOL_ANC: {
                vol->type = VOL_HFP;
                bt_sink_srv_audio_setting_get_hfp_vol(&(vol_info->vol_info.hfp_vol_info), &(vol->vol.hfp_vol));
                vol->type = VOL_ANC;
                break;
            }

            default:
                break;
        }
    return 0;
}

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
typedef enum{
    DETEC_MIC_L_CH,
    DETEC_MIC_R_CH
} detec_mic_channel_t;

static void detach_mic_get_gain(detec_mic_channel_t ch, uint8_t Detach_MIC_Select,uint32_t *analog_gain, uint32_t *digital_gain, bt_sink_audio_setting_multi_vol_config_t *mult_vol_cfg,
        bt_sink_srv_audio_setting_detach_mic_digital_mic_vol_para_t *pNvdmDetachMicVol, bt_sink_srv_audio_setting_detach_mic_analog_mic_vol_para_t *pNvdmDetachMicVol_Analog)
{
    uint16_t Analog_Vol_ACC10K = 0, Analog_Vol_ACC20K = 0, Analog_Vol_DCC = 0, Digital_Vol = 0;
    if(ch == DETEC_MIC_L_CH) {
        Analog_Vol_ACC10K = pNvdmDetachMicVol_Analog->Detach_MIC_Analog_Vol_ACC10K;
        Analog_Vol_ACC20K = pNvdmDetachMicVol_Analog->Detach_MIC_Analog_Vol_ACC20K;
        Analog_Vol_DCC = pNvdmDetachMicVol_Analog->Detach_MIC_Analog_Vol_DCC;
        Digital_Vol = pNvdmDetachMicVol->Detach_MIC_Digital_Vol;
    } else if(ch == DETEC_MIC_R_CH) {
        Analog_Vol_ACC10K = pNvdmDetachMicVol_Analog->Detach_MIC1_Analog_Vol_ACC10K;
        Analog_Vol_ACC20K = pNvdmDetachMicVol_Analog->Detach_MIC1_Analog_Vol_ACC20K;
        Analog_Vol_DCC = pNvdmDetachMicVol_Analog->Detach_MIC1_Analog_Vol_DCC;
        Digital_Vol = pNvdmDetachMicVol->Detach_MIC1_Digital_Vol;
    } else {
        AUDIO_ASSERT(0 && "[Sink][Setting]detach_mic_get_gain wrong channel");
    }
    switch (Detach_MIC_Select) {
        case 0x00:
            mult_vol_cfg->digital_MIC0_L = Digital_Vol;
                #if defined(AIR_BTA_IC_PREMIUM_G2)
                switch (audio_nvdm_HW_config.adc_dac_config.amic_config[0].ADDA_Analog_MIC_Mode)
                #else
                switch (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC0_Mode)
                #endif
                {
                case 0x00: //ACC_10K
                    *analog_gain = Analog_Vol_ACC10K;
                    break;
                case 0x01: //ACC_20K
                    *analog_gain = Analog_Vol_ACC20K;
                    break;
                case 0x02: //DCC
                    *analog_gain = Analog_Vol_DCC;
                    break;
                default:
                    audio_src_srv_err("[Sink][Setting] analog mic type error", 0);
                    break;
                }
            break;
        case 0x01:
            /*multi mic option*/
            if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                mult_vol_cfg->digital_MIC0_R = Digital_Vol;
            } else {
                /*1 mic use first dmic gain*/
                mult_vol_cfg->digital_MIC0_L = Digital_Vol;
            }
                #if defined(AIR_BTA_IC_PREMIUM_G2)
                switch (audio_nvdm_HW_config.adc_dac_config.amic_config[1].ADDA_Analog_MIC_Mode)
                #else
                switch (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC1_Mode) 
                #endif
                {
                case 0x00: //ACC_10K
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_R = Analog_Vol_ACC10K;
                    } else {
                        *analog_gain = Analog_Vol_ACC10K;
                    }
                    break;
                case 0x01: //ACC_20K
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_R = Analog_Vol_ACC20K;
                    } else {
                        *analog_gain = Analog_Vol_ACC20K;
                    }
                    break;
                case 0x02: //DCC
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_R = Analog_Vol_DCC;
                    } else {
                        *analog_gain = Analog_Vol_DCC;
                    }
                    break;
                default:
                    audio_src_srv_err("[Sink][Setting] analog mic type error", 0);
                    break;
                }
            break;
        case 0x02:
            if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                mult_vol_cfg->digital_MIC1_L = Digital_Vol;
            } else {
                mult_vol_cfg->digital_MIC0_L = Digital_Vol;
            }
                #if defined(AIR_BTA_IC_PREMIUM_G2)
                switch (audio_nvdm_HW_config.adc_dac_config.amic_config[2].ADDA_Analog_MIC_Mode)
                #else
                switch (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC2_Mode)
                #endif
                {
                case 0x00: //ACC_10K
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC2 = Analog_Vol_ACC10K;
                    } else {
                        *analog_gain = Analog_Vol_ACC10K;
                    }
                    break;
                case 0x01: //ACC_20K
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC2 = Analog_Vol_ACC20K;
                    } else {
                        *analog_gain = Analog_Vol_ACC20K;
                    }
                    break;
                case 0x02: //DCC
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC2 = Analog_Vol_DCC;
                    } else {
                        *analog_gain = Analog_Vol_DCC;
                    }
                    break;
                default:
                    audio_src_srv_err("[Sink][Setting] analog mic type error", 0);
                    break;
                }
            break;
        case 0x03:
            if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                mult_vol_cfg->digital_MIC1_R = Digital_Vol;
            } else {
                mult_vol_cfg->digital_MIC0_L = Digital_Vol;
            }
                #if defined(AIR_BTA_IC_PREMIUM_G2)
                switch (audio_nvdm_HW_config.adc_dac_config.amic_config[3].ADDA_Analog_MIC_Mode)
                #else
                switch (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC3_Mode) 
                #endif
                {
                case 0x00: //ACC_10K
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC3 = Analog_Vol_ACC10K;
                    } else {
                        *analog_gain = Analog_Vol_ACC10K;
                    }
                    break;
                case 0x01: //ACC_20K
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC3 = Analog_Vol_ACC20K;
                    } else {
                        *analog_gain = Analog_Vol_ACC20K;
                    }
                    break;
                case 0x02: //DCC
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC3 = Analog_Vol_DCC;
                    } else {
                        *analog_gain = Analog_Vol_DCC;
                    }
                    break;
                default:
                    audio_src_srv_err("[Sink][Setting] analog mic type error", 0);
                    break;
                }
            break;
        case 0x04:
            if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                mult_vol_cfg->digital_MIC2_L = Digital_Vol;
            } else {
                mult_vol_cfg->digital_MIC0_L = Digital_Vol;
            }
                #if defined(AIR_BTA_IC_PREMIUM_G2)
                switch (audio_nvdm_HW_config.adc_dac_config.amic_config[4].ADDA_Analog_MIC_Mode)
                #else
                switch (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC4_Mode)
                #endif
                {
                case 0x00: //ACC_10K
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC4 = Analog_Vol_ACC10K;
                    } else {
                        *analog_gain = Analog_Vol_ACC10K;
                    }
                    break;
                case 0x01: //ACC_20K
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC4 = Analog_Vol_ACC20K;
                    } else {
                        *analog_gain = Analog_Vol_ACC20K;
                    }
                    break;
                case 0x02: //DCC
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC4 = Analog_Vol_DCC;
                    } else {
                        *analog_gain = Analog_Vol_DCC;
                    }
                    break;
                default:
                    audio_src_srv_err("[Sink][Setting] analog mic type error", 0);
                    break;
                }
            break;
        case 0x05:
            if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                mult_vol_cfg->digital_MIC2_R = Digital_Vol;
            } else {
                mult_vol_cfg->digital_MIC0_L = Digital_Vol;
            }
                #if defined(AIR_BTA_IC_PREMIUM_G2)
                switch (audio_nvdm_HW_config.adc_dac_config.amic_config[5].ADDA_Analog_MIC_Mode)
                #else
                switch (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC5_Mode)
                #endif
                {
                case 0x00: //ACC_10K
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC5 = Analog_Vol_ACC10K;
                    } else {
                        *analog_gain = Analog_Vol_ACC10K;
                    }
                    break;
                case 0x01: //ACC_20K
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC5 = Analog_Vol_ACC20K;
                    } else {
                        *analog_gain = Analog_Vol_ACC20K;
                    }
                    break;
                case 0x02: //DCC
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                        mult_vol_cfg->analog_MIC5 = Analog_Vol_DCC;
                    } else {
                        *analog_gain = Analog_Vol_DCC;
                    }
                    break;
                default:
                    audio_src_srv_err("[Sink][Setting] analog mic type error", 0);
                    break;
                }
            break;
        case 0x10:
        case 0x20:
        case 0x30:
        case 0x40:
        case 0x50:
        case 0x60:
            /*multi mic option*/
            if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x04) {
                if(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select == 0x10){
                    mult_vol_cfg->digital_I2S0_L = Digital_Vol;
                } else if(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select == 0x20){
                    mult_vol_cfg->digital_I2S0_R = Digital_Vol;
                } else if(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select == 0x30){
                    mult_vol_cfg->digital_I2S1_L = Digital_Vol;
                } else if(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select == 0x40){
                    mult_vol_cfg->digital_I2S1_R = Digital_Vol;
                } else if(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select == 0x50){
                    mult_vol_cfg->digital_I2S2_L = Digital_Vol;
                } else if(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select == 0x60){
                    mult_vol_cfg->digital_I2S2_R = Digital_Vol;
                }
            } else {
                /*1 mic use first dmic gain*/
                mult_vol_cfg->digital_MIC0_L = Digital_Vol;
            }
            break;
        default:
            audio_src_srv_err("[Sink][Setting]audio_get_DIN_in_Detach Mic Fail 2. MIC device type error ,type = %d", 1, audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select);
            return;
    }
}
#endif
void bt_sink_srv_audio_setting_detach_mic_gain_config(uint32_t *analog_gain, uint32_t *digital_gain, bt_sink_audio_setting_multi_vol_config_t *mult_vol_cfg)
{
    bt_sink_srv_audio_setting_detach_mic_digital_mic_vol_para_t *pNvdmDetachMicVol = NULL;
    bt_sink_srv_audio_setting_detach_mic_analog_mic_vol_para_t *pNvdmDetachMicVol_Analog = NULL;
    uint32_t nvdm_tableSize = 0;
    uint8_t nvdm_status = 0;

    if (hal_audio_query_voice_mic_type() == VOICE_MIC_TYPE_DETACHABLE) {
        /*When detach mic is enabled,replace the gain that detach mic configed*/
        /*read the detach mic a-gain && d-gain nvkey*/
        /*read digtal gain*/
        nvdm_status = nvkey_data_item_length(NVID_DSP_FW_IDG_TB_DETACH_MIC, &nvdm_tableSize);

        if (nvdm_status || !nvdm_tableSize) {
            audio_src_srv_err("[Sink][Setting]audio_get_DIN_in_Detach Mic Fail. Status:%d Len:%d\n", 2, nvdm_status, nvdm_tableSize);
            return;
        }
        pNvdmDetachMicVol = (bt_sink_srv_audio_setting_detach_mic_digital_mic_vol_para_t *)pvPortMalloc(nvdm_tableSize);

        nvdm_status = nvkey_read_data(NVID_DSP_FW_IDG_TB_DETACH_MIC, (uint8_t *)pNvdmDetachMicVol, &nvdm_tableSize);

        if (nvdm_status || !pNvdmDetachMicVol) {
            audio_src_srv_err("[Sink][Setting]audio_get_DIN_in_Detach Mic Fail 2. Status:%d pNvdmDetachMicVol:0x%x\n", 2, nvdm_status, pNvdmDetachMicVol);
            if (pNvdmDetachMicVol) {
                vPortFree(pNvdmDetachMicVol);
            }
            return;
        }

        /*read analog gain*/
        nvdm_status = nvkey_data_item_length(NVID_DSP_FW_IAG_TB_DETACH_MIC, &nvdm_tableSize);

        if (nvdm_status || !nvdm_tableSize) {
            audio_src_srv_err("[Sink][Setting]audio_get_DIN_in_Detach Mic Fail. Status:%d Len:%d\n", 2, nvdm_status, nvdm_tableSize);
            if (pNvdmDetachMicVol) {
                vPortFree(pNvdmDetachMicVol);
            }
            return;
        }
        pNvdmDetachMicVol_Analog = (bt_sink_srv_audio_setting_detach_mic_analog_mic_vol_para_t *)pvPortMalloc(nvdm_tableSize);

        nvdm_status = nvkey_read_data(NVID_DSP_FW_IAG_TB_DETACH_MIC, (uint8_t *)pNvdmDetachMicVol_Analog, &nvdm_tableSize);

        if (nvdm_status || !pNvdmDetachMicVol_Analog) {
            audio_src_srv_err("[Sink][Setting]audio_get_DIN_in_Detach Mic Fail 2. Status:%d pNvdmDetachMicVol_Analog:0x%x\n", 2, nvdm_status, pNvdmDetachMicVol_Analog);
            if (pNvdmDetachMicVol_Analog) {
                vPortFree(pNvdmDetachMicVol_Analog);
            }
            if (pNvdmDetachMicVol) {
                vPortFree(pNvdmDetachMicVol);
            }
            return;
        }

#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        detach_mic_get_gain(DETEC_MIC_L_CH, audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select, analog_gain, digital_gain, mult_vol_cfg,pNvdmDetachMicVol,pNvdmDetachMicVol_Analog);
        detach_mic_get_gain(DETEC_MIC_R_CH, audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select2, analog_gain, digital_gain, mult_vol_cfg,pNvdmDetachMicVol,pNvdmDetachMicVol_Analog);

        if(nvdm_tableSize < sizeof(bt_sink_srv_audio_setting_detach_mic_analog_mic_vol_para_t)){
            audio_src_srv_err("[Sink][Setting] old detach mic gain len: %d, %d",2, nvdm_tableSize, sizeof(bt_sink_srv_audio_setting_detach_mic_analog_mic_vol_para_t));
        }

        /*free memory*/
        if (pNvdmDetachMicVol) {
            vPortFree(pNvdmDetachMicVol);
        }
        if (pNvdmDetachMicVol_Analog) {
            vPortFree(pNvdmDetachMicVol_Analog);
        }
        return;
#else
        *digital_gain = pNvdmDetachMicVol->Detach_MIC_Digital_Vol;
        #if defined(AIR_BTA_IC_PREMIUM_G2)
        switch (audio_nvdm_HW_config.adc_dac_config.amic_config[0].ADDA_Analog_MIC_Mode)
        #else
        switch (audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC0_Mode)
        #endif
        {
            case 0x00: //ACC_10K
                *analog_gain = pNvdmDetachMicVol_Analog->Detach_MIC_Analog_Vol_ACC10K;
                break;
            case 0x01: //ACC_20K
                *analog_gain = pNvdmDetachMicVol_Analog->Detach_MIC_Analog_Vol_ACC20K;
                break;
            case 0x02: //DCC
                *analog_gain = pNvdmDetachMicVol_Analog->Detach_MIC_Analog_Vol_DCC;
                break;
            default:
                audio_src_srv_err("[Sink][Setting] analog mic type error", 0);
                break;
        }
        audio_src_srv_report("[Sink][Setting] Detach Mic device = %d,dgain = %d,again = %d", 3, audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select, *digital_gain, *analog_gain);
#endif
        /*free memory*/
        if (pNvdmDetachMicVol) {
            vPortFree(pNvdmDetachMicVol);
        }
        if (pNvdmDetachMicVol_Analog) {
            vPortFree(pNvdmDetachMicVol_Analog);
        }
    }
}
#endif

void bt_sink_srv_audio_setting_update_voice_fillter_setting(bt_sink_srv_audio_setting_vol_info_t *vol_info,
                                                            const audio_eaps_t *am_speech_eaps)
{
    hfp_vol_info_t *vol_hfp = &(vol_info->vol_info.hfp_vol_info);
    device_t dev_out = vol_hfp->dev_out;
    device_t dev_in = vol_hfp->dev_in;
    //uint8_t lev_in = vol_hfp->lev_in;
    //uint8_t lev_out = vol_hfp->lev_out;
    int32_t ret = AUDIO_SETTING_ERR_SUCCESS_OK;
    uint8_t mode = 0;

    if (HAL_AUDIO_DEVICE_HEADSET == dev_out &&
        ((HAL_AUDIO_DEVICE_HEADSET_MIC == dev_in) || (HAL_AUDIO_DEVICE_MAIN_MIC == dev_in))) {
        mode = SPEECH_MODE_HEADSET_HEADSET_MIC;
        ret = AUDIO_SETTING_ERR_SUCCESS_4TH;
    } else if (HAL_AUDIO_DEVICE_HANDS_FREE_STEREO == dev_out &&
               HAL_AUDIO_DEVICE_MAIN_MIC == dev_in) {
        mode = SPEECH_MODE_HANDSFREE_MAIN_MIC;
        ret = AUDIO_SETTING_ERR_SUCCESS_5TH;
    } else if (HAL_AUDIO_DEVICE_HEADSET == dev_out &&
               HAL_AUDIO_DEVICE_SINGLE_DIGITAL_MIC == dev_in) {
        mode = SPEECH_MODE_HEADSET_DIGITAL_MIC;
        ret = AUDIO_SETTING_ERR_SUCCESS_6TH;
    } else {
        ret = AUDIO_SETTING_ERR_FAIL_2ND;
    }

    if (ret > AUDIO_SETTING_ERR_SUCCESS_OK) {
        speech_update_common(am_speech_eaps->speech_common_parameter.speech_common_parameter);
        speech_update_nb_fir((int16_t *)am_speech_eaps->voice_parameter.voice_nb_parameter[mode].voice_nb_enhancement_parameter.nb_stream_in_fir_coefficient,
                             (int16_t *)am_speech_eaps->voice_parameter.voice_nb_parameter[mode].voice_nb_enhancement_parameter.nb_stream_out_fir_coefficient);
        speech_update_nb_param(am_speech_eaps->voice_parameter.voice_nb_parameter[mode].voice_nb_enhancement_parameter.nb_mode_parameter);
        speech_update_wb_fir((int16_t *)am_speech_eaps->voice_parameter.voice_wb_parameter[mode].voice_wb_enhancement_parameter.wb_stream_in_fir_coefficient,
                             (int16_t *)am_speech_eaps->voice_parameter.voice_wb_parameter[mode].voice_wb_enhancement_parameter.wb_stream_out_fir_coefficient);
        speech_update_wb_param(am_speech_eaps->voice_parameter.voice_wb_parameter[mode].voice_wb_enhancement_parameter.wb_mode_parameter);
    }

    audio_src_srv_report("[Sink][Setting]update_voice_fillter--dev_in: 0x%x, dev_out: 0x%x, lev_in: %d, lev_out: %d, codec: %d,   ret: %d, mode: %d", 7,
                         vol_hfp->dev_in, vol_hfp->dev_out, vol_hfp->lev_in, vol_hfp->lev_out, vol_hfp->codec,  ret, mode);
    //audio_src_srv_report("[Sink][Setting]update_voice_fillter--ret: %d, mode: %d", 2, ret, mode);
    UNUSED(ret);
    UNUSED(mode);
}

#ifdef AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE
void bt_sink_srv_audio_setting_init_audio_platform_info(void)
{
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DRIVER_PARAM, 2, (uint32_t)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_3RD_PARTY_AUDIO_PLATFORM), true);
}

mcu_dsp_audio_platform_share_buffer_info_t * bt_sink_srv_audio_setting_get_audio_platform_share_info(void)
{
    return (mcu_dsp_audio_platform_share_buffer_info_t *)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_3RD_PARTY_AUDIO_PLATFORM);
}

const uint32_t bt_sink_srv_audio_volume_type_matching_table[VOL_TOTAL] = {
    AUDIO_PLATFORM_OUTPUT_BTA,      //VOL_A2DP = 0,
    AUDIO_PLATFORM_OUTPUT_BTA,      //VOL_HFP,
    AUDIO_PLATFORM_OUTPUT_UNUSED,   //VOL_PCM,
    AUDIO_PLATFORM_OUTPUT_VP,       //VOL_MP3,
    AUDIO_PLATFORM_OUTPUT_BTA,      //VOL_HFP_NB,
    AUDIO_PLATFORM_OUTPUT_VP,       //VOL_VP,
    AUDIO_PLATFORM_OUTPUT_UNUSED,   //VOL_VC,
    AUDIO_PLATFORM_OUTPUT_LINEIN,   //VOL_LINE_IN,
    AUDIO_PLATFORM_OUTPUT_USB,      //VOL_USB_AUDIO_IN,
    AUDIO_PLATFORM_OUTPUT_UNUSED,   //VOL_ANC,
    AUDIO_PLATFORM_OUTPUT_UNUSED,   //VOL_USB_AUDIO_SW_IN,
    AUDIO_PLATFORM_OUTPUT_UNUSED,   //VOL_USB_VOICE_SW_OUT,
    AUDIO_PLATFORM_OUTPUT_LINEIN,   //VOL_LINE_IN_DL3,
    AUDIO_PLATFORM_OUTPUT_USB,      //VOL_USB_AUDIO_IN_DL3,
    AUDIO_PLATFORM_OUTPUT_UNUSED,   //VOL_DEF, /* stream out only */
};
void bt_sink_srv_audio_setting_set_audio_platform_output_volume(vol_type_t type, int32_t digital_gain)
{
    mcu_dsp_audio_platform_share_buffer_info_t *share_info_ptr;
    mcu_dsp_audio_platform_output_selection output_select;
    if (type < VOL_TOTAL) {
        output_select = bt_sink_srv_audio_volume_type_matching_table[type];
        if (output_select < AUDIO_PLATFORM_OUTPUT_UNUSED) {
            share_info_ptr = (mcu_dsp_audio_platform_share_buffer_info_t *)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_3RD_PARTY_AUDIO_PLATFORM);
            share_info_ptr->downlink_gain[output_select] = (int16_t)digital_gain;
        }
    }
}

#endif

#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
#define AUDIO_VOLUME_MONITOR_DEBUG_ENABLE 0
#define MUTE_MIC_DETECTION_THD -8500
#define MUTE_MIC_DETECTION_INTERVAL_MS 600
audio_dsp_mute_speaking_detection_param_t  mute_speaking_detection_info;
extern bt_sink_srv_am_background_t *g_prCurrent_player;
#if defined(AIR_WIRED_AUDIO_ENABLE) && defined(AIR_USB_AUDIO_ENABLE)
extern bool g_usb_out_vad_enable;
#endif
uint8_t mute_mic_detection_cnt = 0;
bool last_det_sta = false;

void mute_mic_detection(int32_t *out_data, uint32_t volume_len, void *user_data){
    //audio_src_srv_report("[VAD] %d",1,*out_data);
    if(((g_prCurrent_player && g_prCurrent_player->audio_stream_in.audio_mute)
#if defined(AIR_WIRED_AUDIO_ENABLE) && defined(AIR_USB_AUDIO_ENABLE)
        ||(g_usb_out_vad_enable)
#endif
        ) && (*out_data > MUTE_MIC_DETECTION_THD)){
        mute_mic_detection_cnt++;
    }else{
        if(last_det_sta){
            last_det_sta = false;
            /* send notification to APP */
            if(mute_speaking_detection_info.callback != NULL) {
                audio_src_srv_report("[VAD][TEAMS] UL mute speaking detected end",0);
                mute_speaking_detection_info.callback(false);
            } else {
                audio_src_srv_report("[VAD][TEAMS] UL mute speaking detected end without callback",0);
            }
#if AUDIO_VOLUME_MONITOR_DEBUG_ENABLE
            audio_src_srv_report("[VAD][Audio Spectrum Meter] mute_mic_detection_end", 0);
#endif
        }
        mute_mic_detection_cnt = 0;
    }

    if(mute_mic_detection_cnt > (MUTE_MIC_DETECTION_INTERVAL_MS /15) && !last_det_sta){
        /* send notification to APP */
        if(mute_speaking_detection_info.callback != NULL) {
            audio_src_srv_report("[VAD][TEAMS] UL mute speaking detected start",0);
            mute_speaking_detection_info.callback(true);
        } else {
            audio_src_srv_report("[VAD][TEAMS] UL mute speaking detected start without callback",0);
        }
        last_det_sta = true;
        mute_mic_detection_cnt = 0;
#if AUDIO_VOLUME_MONITOR_DEBUG_ENABLE
        audio_src_srv_report("[VAD][Audio Spectrum Meter] mute_mic_detection_start", 0);
#endif
    }
#if AUDIO_VOLUME_MONITOR_DEBUG_ENABLE
    audio_src_srv_report("[VAD][Audio Spectrum Meter] mute_mic_detection_cnt %d", 1,mute_mic_detection_cnt);
#endif
}
#endif

#endif /* __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__ */

