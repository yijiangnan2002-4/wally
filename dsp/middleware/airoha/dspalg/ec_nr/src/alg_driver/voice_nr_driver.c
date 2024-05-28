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

#include "config.h"
#include "dsp_feature_interface.h"
#include "dsp_utilities.h"
#include "dsp_buffer.h"
#include "dsp_audio_ctrl.h"
#include "dsp_share_memory.h"
#include "audio_nvdm_common.h"
#include "dsp_dump.h"
#include "string.h"
#include "dsp_memory.h"
#include "dsp_audio_msg_define.h"
#include "dsp_audio_msg.h"
#include "dsp_callback.h"
#include "voice_nr_interface.h"
#include "voice_nr_driver.h"
#include "voice_nr_portable.h"
#include "dsp_rom_table.h"
#include "dsp_stream_task.h"

#define VOICE_FB_5_MS_FRAME_SIZE      (960)
#define VOICE_FB_15_MS_FRAME_SIZE      (2880)
#define VOICE_SWB_15_MS_FRAME_SIZE      (960)
#define VOICE_SWB_7P5_MS_FRAME_SIZE     (480)
#define VOICE_SWB_5_MS_FRAME_SIZE       (320)
#define VOICE_WB_15_MS_FRAME_SIZE       (480)
#define VOICE_WB_7P5_MS_FRAME_SIZE      (240)
#define VOICE_WB_5_MS_FRAME_SIZE        (160)
#define DSP_AEC_NR_SCRATCH_MEMSIZE      (1)

#define DSP_ALG_ECNR_EC_BUFFER_MAGIC_NUMBER         0x6128800
#define DSP_ALG_ECNR_NR_BUFFER_MAGIC_NUMBER         0x6128801
#define DSP_ALG_ECNR_POSTEC_BUFFER_MAGIC_NUMBER     0x6128802
#define DSP_ALG_ECNR_TXEQ_BUFFER_MAGIC_NUMBER       0x6128803
#define DSP_ALG_ECNR_EC_POSTEC_BUFFER_MAGIC_NUMBER  0x6128804
#define DSP_ALG_ECNR_ALL_BUFFER_MAGIC_NUMBER        0x6128805
typedef struct {
    uint32_t MemoryCheck;
    uint32_t deinit_process;
    uint32_t init_flag;
} voice_ecnr_common_para_t;

DSP_PARA_AEC_STRU *g_voice_ecnr_nvkey_aec;
#if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
static U32 g_igo_nr_curr_mode = 0;
#endif

static voice_ecnr_status_t voice_ecnr_common_init(void *para, voice_ecnr_common_para_t **p_common, bool alloc_flag, uint32_t alloc_size, uint32_t magic_number)
{
    bool re_init;
    DSP_STREAMING_PARA_PTR stream_ptr;
    voice_ecnr_common_para_t *p_common_curr;

    p_common_curr = *p_common;

    /* Check and alloc working buffer */
    re_init = false;
    if ((p_common_curr == NULL) || (p_common_curr->MemoryCheck != magic_number)) {
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        uint32_t work_buf_length;
        UNUSED(stream_ptr);
        if (alloc_flag == true) {
            work_buf_length = stream_function_get_working_buffer_length(para);
            AUDIO_ASSERT(work_buf_length >= alloc_size);
            p_common_curr = (voice_ecnr_common_para_t *)stream_function_get_working_buffer(para);
        }
#else
        UNUSED(alloc_flag);
        stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
#ifdef AIR_BT_CODEC_BLE_ENABLED
        if ((stream_ptr->source->param.n9ble.codec_type == BT_BLE_CODEC_LC3) || (stream_ptr->sink->param.n9ble.codec_type == BT_BLE_CODEC_LC3)) {
            #ifndef LE_AUDIO_MAGIC_STREAM_PTR
            #define LE_AUDIO_MAGIC_STREAM_PTR       0x12345678 /* to protect ECNR memory and support DL/UL on different tasks */
            #endif
            p_common_curr = (voice_ecnr_common_para_t *)DSPMEM_tmalloc(DAV_TASK_ID, alloc_size, (VOID*)LE_AUDIO_MAGIC_STREAM_PTR);// for UL/DL on different task
            DSP_MW_LOG_E("[DSP][VOICE_NR] set magic stream_ptr 0x%x", 1, LE_AUDIO_MAGIC_STREAM_PTR);
        } else {
#endif
        p_common_curr = (voice_ecnr_common_para_t *)DSPMEM_tmalloc(DAV_TASK_ID, alloc_size, stream_ptr);
#ifdef AIR_BT_CODEC_BLE_ENABLED
        }
#endif

#endif
        if (p_common_curr != NULL) {
            p_common_curr->MemoryCheck = magic_number;
            p_common_curr->deinit_process = false;
            p_common_curr->init_flag = false;
        }
        re_init = true;
    }
    *p_common = p_common_curr;

    /* Check whether later init is needed */
    if ((re_init == false) && (p_common_curr->deinit_process == false)) {
        return VOICE_ECNR_STATUS_BYPASS;
    }

    if (p_common_curr->deinit_process == true) {
        p_common_curr->deinit_process = false;
    }

    return VOICE_ECNR_STATUS_OK;
}

static void voice_ecnr_common_deinit(voice_ecnr_common_para_t **p_common, uint32_t magic_number)
{
    voice_ecnr_common_para_t *p_common_curr;

    p_common_curr = *p_common;
    if ((p_common_curr == NULL) || (p_common_curr->MemoryCheck != magic_number)) {
        DSP_MW_LOG_E("[DSP][VOICE_NR] deinit check, working memory check fail", 0);
        return;
    }

    p_common_curr->deinit_process = true;
}

static voice_ecnr_mic_mode_t g_voice_ecnr_mic_mode;

void voice_ecnr_set_mic_mode(voice_ecnr_mic_mode_t mode)
{
    g_voice_ecnr_mic_mode = mode;
    DSP_MW_LOG_I("[DSP][VOICE_NR] ecnr_mic_mode %d ", 1, mode);
}

static bool voice_ecnr_check_running_status_internal(voice_ecnr_common_para_t *common, uint32_t magic_number)
{
    if ((common != NULL) &&
        (common->init_flag == true) &&
        (common->MemoryCheck == magic_number)) {
        return true;
    }

    return false;
}

/************************************
 * For Rx NR part library porting
 ************************************/
typedef struct {
    DSP_ALIGN4 DSP_PARA_SWB_RX_EQ_STRU rx_eq;
} nvkey_rx_nr_config_t;

static voice_ecnr_status_t voice_rxnr_init(void *p_working_buf, DSP_PARA_NR_STRU *nvkey_nr, DSP_PARA_AEC_NR_SWB_STRU *nvkey_swb_nr, nvkey_rx_nr_config_t *nvkey_rx_config)
{
    if (gDspAlgParameter.EscoMode.Rx == VOICE_FB) {
        //nvkey_read_full_key(NVKEY_DSP_PARA_SWB_RX_EQ, &nvkey_rx_config->rx_eq, sizeof(DSP_PARA_SWB_RX_EQ_STRU));
        DSP_MW_LOG_W("[DSP][VOICE_NR] voice_rxnr_init, FB_RX_NR_init alg not available yet", 0);
    } else if (gDspAlgParameter.EscoMode.Rx == VOICE_SWB) {
        nvkey_read_full_key(NVKEY_DSP_PARA_SWB_RX_EQ, &nvkey_rx_config->rx_eq, sizeof(DSP_PARA_SWB_RX_EQ_STRU));
        SWB_RX_NR_init(p_working_buf, nvkey_nr, nvkey_swb_nr, &nvkey_rx_config->rx_eq);
    } else if (gDspAlgParameter.EscoMode.Rx == VOICE_WB) {
        nvkey_read_full_key(NVKEY_DSP_PARA_WB_RX_EQ, &nvkey_rx_config->rx_eq, sizeof(DSP_PARA_WB_RX_EQ_STRU));
        WB_RX_NR_init(p_working_buf, nvkey_nr, &nvkey_rx_config->rx_eq);
    } else {
        nvkey_read_full_key(NVKEY_DSP_PARA_NB_RX_EQ, &nvkey_rx_config->rx_eq, sizeof(DSP_PARA_NB_RX_EQ_STRU));
        WB_RX_NR_init(p_working_buf, nvkey_nr, &nvkey_rx_config->rx_eq);
    }
    return VOICE_ECNR_STATUS_OK;
}

#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_3RD_PARTY_NR_ENABLE)
extern uint32_t *p_rx_eq;
extern bool g_rx_eq_update;

voice_ecnr_status_t voice_RX_EQ_update(uint32_t *p_rx_eq){
    DSP_MW_LOG_I("[DSP][VOICE_NR] mode:%d, 0x%x , %x, %x", 4, gDspAlgParameter.EscoMode.Rx, p_rx_eq,p_rx_eq[0],p_rx_eq[1]);
    if (gDspAlgParameter.EscoMode.Rx == VOICE_SWB) {
        RX_EQ_update(p_rx_eq, VOICE_SWB);
    } else if (gDspAlgParameter.EscoMode.Rx == VOICE_WB) {
        RX_EQ_update(p_rx_eq, VOICE_WB);
    } else {
        RX_EQ_update(p_rx_eq, VOICE_NB);
    }

    return VOICE_ECNR_STATUS_OK;
}

#endif

voice_ecnr_status_t voice_rxnr_process(void *para, uint32_t length, int16_t *in_out)
{
    UNUSED(para);

    if (length == 0) {
        return VOICE_ECNR_STATUS_BYPASS;
    }

    ///TODO:: check the flow here
#ifndef AIR_3RD_PARTY_NR_ENABLE
#if 0
    if (gNeedChangeRxEQ) {
        EQ_update(p_ecnr_bss, p_rxeq_NvKey);
        gNeedChangeRxEQ = false;
        DSP_MW_LOG_I("aec_nr change RX EQ param", 0);
    }
#endif
#endif

    LOG_AUDIO_DUMP((uint8_t *)in_out, length, VOICE_RX_NR_IN);

#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_3RD_PARTY_NR_ENABLE)
    if(g_rx_eq_update){
        voice_RX_EQ_update(p_rx_eq);
        g_rx_eq_update = false;
    }
#endif

    switch (gDspAlgParameter.EscoMode.Rx) {
        /* Rx side always convert to 16K when NB is configured. */
        case VOICE_NB:
        case VOICE_WB:
            if (length != VOICE_WB_15_MS_FRAME_SIZE) {
                return VOICE_ECNR_STATUS_ERROR;
            }
            Voice_WB_RX_Prcs(in_out);
            break;
        case VOICE_SWB:
            if (length != VOICE_SWB_15_MS_FRAME_SIZE) {
                return VOICE_ECNR_STATUS_ERROR;
            }
            Voice_SWB_RX_Prcs(in_out);
            break;
        default:
            return VOICE_ECNR_STATUS_ERROR;
    }
#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((uint8_t *)in_out, length, VOICE_RX_NR_OUT);
#endif

    return VOICE_ECNR_STATUS_OK;
}

/************************************
 * For EC library porting
 ************************************/
#if defined(AIR_3RD_PARTY_NR_ENABLE)
#if defined(AIR_ECNR_PREV_PART_ENABLE)

#ifndef AIR_ECNR_SEPARATE_MODE_ENABLE

typedef struct {
    voice_ecnr_common_para_t common;
    DSP_ALIGN4 DSP_PARA_AEC_STRU nvkey_aec;
    DSP_ALIGN4 DSP_PARA_NR_STRU nvkey_nr;
    DSP_ALIGN4 DSP_PARA_INEAR_STRU nvkey_inear;
#ifdef AIR_BTA_IC_PREMIUM_G2
    DSP_ALIGN4 DSP_PARA_AEC_NR_STRU nvkey_aec_nr;
#endif
#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
    DSP_ALIGN4 DSP_PARA_AEC_NR_SWB_STRU nvkey_nr_swb;
#endif
    DSP_ALIGN8 uint8_t AecOut[VOICE_SWB_15_MS_FRAME_SIZE];
    DSP_ALIGN16 uint8_t ScratchMemory[DSP_AEC_NR_SCRATCH_MEMSIZE]; //Set at the end of structure for dynamic size
} voice_ecnr_ec_para_t;

static voice_ecnr_ec_para_t *g_voice_ecnr_ec_para = NULL;
static uint32_t g_voice_ecnr_ec_period = 0;

bool voice_ecnr_ec_config_period(uint32_t period)
{
    if ((period != 7500) && (period != 5000)) {
        DSP_MW_LOG_E("[DSP][VOICE_NR] inhouse EC period not support %d", 1, period);
        return false;
    }

    g_voice_ecnr_ec_period = period;

    DSP_MW_LOG_E("[DSP][VOICE_NR] inhouse EC period set to %d", 1, period);

    return true;
}

bool voice_ecnr_ec_check_running_status(void)
{
    return voice_ecnr_check_running_status_internal((voice_ecnr_common_para_t *)g_voice_ecnr_ec_para, DSP_ALG_ECNR_EC_BUFFER_MAGIC_NUMBER);
}

voice_ecnr_status_t voice_ecnr_ec_init(void *para)
{
    voice_ecnr_status_t ret;

    /* Check whether init is needed */
    ret = voice_ecnr_common_init(para, (voice_ecnr_common_para_t **)&g_voice_ecnr_ec_para, true, sizeof(voice_ecnr_ec_para_t) + prev_ec_get_ec120_memsize(), DSP_ALG_ECNR_EC_BUFFER_MAGIC_NUMBER);
    if (ret == VOICE_ECNR_STATUS_BYPASS) {
        return VOICE_ECNR_STATUS_BYPASS;
    }

    /* load nvkey */
    nvkey_read_full_key(NVKEY_DSP_PARA_AEC, &g_voice_ecnr_ec_para->nvkey_aec, sizeof(DSP_PARA_AEC_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_NR, &g_voice_ecnr_ec_para->nvkey_nr, sizeof(DSP_PARA_NR_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_INEAR, &g_voice_ecnr_ec_para->nvkey_inear, sizeof(DSP_PARA_INEAR_STRU));
#ifdef AIR_BTA_IC_PREMIUM_G2
    voice_nvkey_aec_nr_porting_layer(&g_voice_ecnr_ec_para->nvkey_aec, &g_voice_ecnr_ec_para->nvkey_nr, &g_voice_ecnr_ec_para->nvkey_inear, &g_voice_ecnr_ec_para->nvkey_aec_nr);
#endif
#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
    if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
        nvkey_read_full_key(NVKEY_DSP_PARA_AEC_NR_SWB, &g_voice_ecnr_ec_para->nvkey_nr_swb, sizeof(DSP_PARA_AEC_NR_SWB_STRU));
    }
#endif

    /* Init the alg */
    if (g_voice_ecnr_ec_period == 7500) {
#ifdef AIR_BTA_IC_PREMIUM_G2
        prev_ec_Voice_EC120_Init(false, g_voice_ecnr_ec_para->ScratchMemory, &g_voice_ecnr_ec_para->nvkey_aec_nr, (int16_t *)&g_voice_ecnr_ec_para->nvkey_aec_nr.WB_NR_TX_NOISE_GAIN_LIMITER);
#elif defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
        if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            prev_ec_Voice_SWB_EC120_Init(g_voice_ecnr_ec_para->ScratchMemory, &g_voice_ecnr_ec_para->nvkey_aec, &g_voice_ecnr_ec_para->nvkey_nr_swb);
        } else {
            prev_ec_Voice_EC120_Init(false, g_voice_ecnr_ec_para->ScratchMemory, &g_voice_ecnr_ec_para->nvkey_aec, (int16_t *)&g_voice_ecnr_ec_para->nvkey_nr.WB_NR_TX_NOISE_GAIN_LIMITER);
        }
#endif
    } else if (g_voice_ecnr_ec_period == 5000) {
#ifdef AIR_BTA_IC_PREMIUM_G2
        prev_ec_Voice_EC80_Init(false, g_voice_ecnr_ec_para->ScratchMemory, &g_voice_ecnr_ec_para->nvkey_aec_nr, (int16_t *)&g_voice_ecnr_ec_para->nvkey_aec_nr.WB_NR_TX_NOISE_GAIN_LIMITER);
#elif defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
        if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            prev_ec_Voice_SWB_EC80_Init(g_voice_ecnr_ec_para->ScratchMemory, &g_voice_ecnr_ec_para->nvkey_aec, &g_voice_ecnr_ec_para->nvkey_nr, &g_voice_ecnr_ec_para->nvkey_inear, &g_voice_ecnr_ec_para->nvkey_nr_swb);
        } else {
            prev_ec_Voice_EC80_Init(false, g_voice_ecnr_ec_para->ScratchMemory, &g_voice_ecnr_ec_para->nvkey_aec, (int16_t *)&g_voice_ecnr_ec_para->nvkey_nr.WB_NR_TX_NOISE_GAIN_LIMITER);
        }
#endif
    } else {
        AUDIO_ASSERT(0);
    }

    g_voice_ecnr_nvkey_aec = &g_voice_ecnr_ec_para->nvkey_aec;

    g_voice_ecnr_ec_para->common.init_flag = true;

    /* Query the alg version log */
    DSP_MW_LOG_I("[DSP][VOICE_NR] inhouse EC version %d", 1, prev_ec_get_ECNR_SVN());

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_ec_deinit(void)
{
    voice_ecnr_common_deinit((voice_ecnr_common_para_t **)&g_voice_ecnr_ec_para, DSP_ALG_ECNR_EC_BUFFER_MAGIC_NUMBER);

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_ec_process(void *para, uint32_t length, int16_t *main_mic, int16_t *ff_mic, int16_t *fb_mic, int16_t *ec_in, int16_t *out)
{
#ifdef AIR_BTA_IC_PREMIUM_G2
    uint8_t extra_OutBuf[VOICE_WB_7P5_MS_FRAME_SIZE];
#else
    uint8_t extra_OutBuf[VOICE_SWB_7P5_MS_FRAME_SIZE];
#endif

    UNUSED(para);
    UNUSED(fb_mic);

    if (((gDspAlgParameter.EscoMode.Tx == VOICE_SWB) &&
        (length != VOICE_SWB_5_MS_FRAME_SIZE) && (length != VOICE_SWB_7P5_MS_FRAME_SIZE) && (length != VOICE_SWB_15_MS_FRAME_SIZE)) ||
        ((gDspAlgParameter.EscoMode.Tx == VOICE_WB) &&
        (length != VOICE_WB_5_MS_FRAME_SIZE) && (length != VOICE_WB_7P5_MS_FRAME_SIZE) && (length != VOICE_WB_15_MS_FRAME_SIZE))) {
        DSP_MW_LOG_E("[DSP][VOICE_NR] inhouse EC frame size error, baud mode %d, expect 5ms or 7.5ms or 15ms, actual length %d", 2, gDspAlgParameter.EscoMode.Tx, length);
        return VOICE_ECNR_STATUS_ERROR;
    }

#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
    if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
        prev_ec_Voice_SWB_EC120_Prcs(main_mic, ff_mic, ec_in, (int16_t *)g_voice_ecnr_ec_para->AecOut, (int16_t *)extra_OutBuf);
    }
#endif
    if (gDspAlgParameter.EscoMode.Tx == VOICE_WB) {
        prev_ec_Voice_EC120_Prcs(main_mic, ff_mic, ec_in, (int16_t *)g_voice_ecnr_ec_para->AecOut, (int16_t *)extra_OutBuf);
    }
    memcpy(out, g_voice_ecnr_ec_para->AecOut, length);

#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((uint8_t *)out, length, AUDIO_ENHANCEMENT_IN_L);
#endif

    /* Get the EC gain for post EC reference */
    gDspAlgParameter.PostEC_Gain = prev_ec_PostEC120_Info(&gDspAlgParameter.AecNr);

    return VOICE_ECNR_STATUS_OK;
}

uint32_t voice_ecnr_ec_get_ref_gain(int16_t *gain_buf)
{
    uint32_t i, length;

    length = 0;
    if (voice_ecnr_ec_check_running_status() == true) {
        prev_ec_EC_REF_GAIN_READBACK(gain_buf);
        length += 8;
        for (i = 0; i < 8; i++) {
            DSP_MW_LOG_I("[DSP][VOICE_NR] DSP get ref gain, GainAddr:0x%x", 1, gain_buf[i]);
        }
    }

    return length;
}

uint8_t voice_ecnr_ec_get_postec_gain(void)
{
    return gDspAlgParameter.PostEC_Gain;
}

#endif
#endif
#endif



/************************************
 * For NR library porting
 ************************************/
#if defined(AIR_3RD_PARTY_NR_ENABLE)

typedef struct {
    voice_ecnr_common_para_t common;
#if defined( AIR_AI_NR_PREMIUM_INEAR_270K_ENABLE)
    DSP_ALIGN4 DSP_PARA_RESERVED_STRU ResKey;
#else
    DSP_ALIGN4 DSP_PARA_RESERVED_STRU ResKey[2];
#endif
#ifdef AIR_BTA_IC_PREMIUM_G2
    DSP_ALIGN4 DSP_PARA_WB_TX_EQ_STRU ff_eq;
#elif defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
    DSP_ALIGN4 DSP_PARA_SWB_TX_EQ_STRU ff_eq;
#endif
#if defined(AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE) || defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE)
    DSP_ALIGN4 DSP_PARA_WB_TX_EQ_STRU ff_eq_1;
#endif
#if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
    DSP_ALIGN4 DSP_PARA_AST_EQ_STRU fb_eq[6];
#endif
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
    DSP_ALIGN16 int32_t NR_out_buf[720];//worst case: 48k/ 24bit/ 15ms
#else
    DSP_ALIGN16 int16_t NR_out_buf[480];//worst case: 32k/ 16bit/ 15ms
#endif
    DSP_ALIGN16 uint8_t ScratchMemory[DSP_AEC_NR_SCRATCH_MEMSIZE]; //Set at the end of structure for dynamic size
} voice_ecnr_nr_para_t;

static uint32_t g_voice_ecnr_nr_memsize;
static voice_ecnr_nr_para_t *g_voice_ecnr_nr_para = NULL;

#if (defined(AIR_BTA_IC_PREMIUM_G2) && (defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE) || defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE))) || (!defined(AIR_BTA_IC_PREMIUM_G2) && (defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)))
#ifdef MTK_ANC_ENABLE
#include "anc_api.h"
#endif
static _SWITCH_MODE igo_nr_get_curr_mode(void)
{
    _SWITCH_MODE anc_mode;

#ifdef MTK_ANC_ENABLE
    uint8_t anc_enable;
    dsp_anc_control_type_t anc_type;
    dsp_anc_control_get_status(&anc_enable, &anc_type, NULL);
    if (anc_enable == true) {
        if (anc_type == DSP_ANC_CONTROL_TYPE_PASSTHRU_FF) {
            anc_mode = _PASSTHROUGH;
        } else {
            anc_mode = _ANC_ON;
        }
    } else {
        anc_mode = _ANC_OFF;
    }
#else
    anc_mode = _ANC_OFF;
#endif

    return anc_mode;
}
#endif

#if (defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_BTA_IC_PREMIUM_G2)) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
static uint16_t g_voice_ecnr_ec_postec_fb_gain;
#endif

#if defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE) || defined(AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE)
static uint16_t g_voice_ecnr_ec_postec_wind_info;
#endif

#if defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE) || defined(AIR_AI_NR_PREMIUM_ENABLE)
#if !defined(AIR_AI_NR_PREMIUM_INEAR_270K_ENABLE)
static _NR_LEVEL g_igo_nr_level = _NR_LEVEL1;
#endif
#endif

bool voice_ecnr_nr_get_framework_workingbuffer(int16_t** NR_out_buf)
{
    *NR_out_buf = (int16_t*)g_voice_ecnr_nr_para->NR_out_buf;
    return true;
}

bool voice_ecnr_nr_check_running_status(void)
{
    return voice_ecnr_check_running_status_internal((voice_ecnr_common_para_t *)g_voice_ecnr_nr_para, DSP_ALG_ECNR_NR_BUFFER_MAGIC_NUMBER);
}

voice_ecnr_status_t voice_ecnr_nr_init(void *para)
{
    uint32_t work_buf_size;
    voice_ecnr_status_t ret;

#if defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE) || defined(AIR_AI_NR_PREMIUM_ENABLE)
    U32 igo_mode = 0;
    work_buf_size = IGO_NR_MEMSIZE;
#else
    work_buf_size = 0;
#endif
#if defined (AIR_VOICE_BAND_CONFIG_TYPE_FB)
    if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
        DSP_MW_LOG_I("[DSP][VOICE_NR] FB alg currently only supports EC and does not yet support NR", 0);
        g_voice_ecnr_nr_memsize = sizeof(voice_ecnr_nr_para_t);
        ret = voice_ecnr_common_init(para, (voice_ecnr_common_para_t **)&g_voice_ecnr_nr_para, true, g_voice_ecnr_nr_memsize, DSP_ALG_ECNR_NR_BUFFER_MAGIC_NUMBER);
        return VOICE_ECNR_STATUS_BYPASS;
    }
#endif

    /* Check whether init is needed */
    g_voice_ecnr_nr_memsize = sizeof(voice_ecnr_nr_para_t) + work_buf_size;
    ret = voice_ecnr_common_init(para, (voice_ecnr_common_para_t **)&g_voice_ecnr_nr_para, true, g_voice_ecnr_nr_memsize, DSP_ALG_ECNR_NR_BUFFER_MAGIC_NUMBER);
    if (ret == VOICE_ECNR_STATUS_BYPASS) {
        #if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
        voice_ecnr_check_txnr_lib_match(para);//check igo lib match stream
        #endif
        #if (defined(AIR_BTA_IC_PREMIUM_G2) && (defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE) || defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE))) || (!defined(AIR_BTA_IC_PREMIUM_G2) && (defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)))
        if (igo_nr_get_curr_mode() == g_igo_nr_curr_mode) {
            return VOICE_ECNR_STATUS_BYPASS;
        }
        #else
        return VOICE_ECNR_STATUS_BYPASS;
        #endif
    }

    /* load nvkey */
#if defined(AIR_AI_NR_PREMIUM_INEAR_270K_ENABLE)
    nvkey_read_full_key(NVKEY_DSP_PARA_RESERVED, &g_voice_ecnr_nr_para->ResKey, sizeof(DSP_PARA_RESERVED_STRU));
#else
    nvkey_read_full_key(NVKEY_DSP_PARA_RESERVED, &g_voice_ecnr_nr_para->ResKey[0], sizeof(DSP_PARA_RESERVED_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_RESERVED_1, &g_voice_ecnr_nr_para->ResKey[1], sizeof(DSP_PARA_RESERVED_STRU));
#endif
    if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
        nvkey_read_full_key(NVKEY_DSP_PARA_SWB_TX_EQ, &g_voice_ecnr_nr_para->ff_eq, sizeof(DSP_PARA_SWB_TX_EQ_STRU));
#endif
    } else if (gDspAlgParameter.EscoMode.Tx == VOICE_WB) {
        nvkey_read_full_key(NVKEY_DSP_PARA_WB_TX_EQ, &g_voice_ecnr_nr_para->ff_eq, sizeof(DSP_PARA_WB_TX_EQ_STRU));
    } else {
        nvkey_read_full_key(NVKEY_DSP_PARA_NB_TX_EQ, &g_voice_ecnr_nr_para->ff_eq, sizeof(DSP_PARA_NB_TX_EQ_STRU));
    }
#if defined(AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE) || defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)  || defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE)
    nvkey_read_full_key(NVKEY_DSP_PARA_WB_TX_PRE_EQ, &g_voice_ecnr_nr_para->ff_eq_1, sizeof(DSP_PARA_WB_TX_EQ_STRU));
#endif
#if defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE) || defined(AIR_AI_NR_PREMIUM_ENABLE)
    nvkey_read_full_key(NVKEY_DSP_PARA_AST_EQ,   &g_voice_ecnr_nr_para->fb_eq[0], sizeof(DSP_PARA_AST_EQ_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_AST_EQ_1, &g_voice_ecnr_nr_para->fb_eq[1], sizeof(DSP_PARA_AST_EQ_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_AST_EQ_2, &g_voice_ecnr_nr_para->fb_eq[2], sizeof(DSP_PARA_AST_EQ_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_AST_EQ_3, &g_voice_ecnr_nr_para->fb_eq[3], sizeof(DSP_PARA_AST_EQ_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_AST_EQ_4, &g_voice_ecnr_nr_para->fb_eq[4], sizeof(DSP_PARA_AST_EQ_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_AST_EQ_5, &g_voice_ecnr_nr_para->fb_eq[5], sizeof(DSP_PARA_AST_EQ_STRU));
#endif

    /* Init the alg */
#if defined(AIR_AI_NR_PREMIUM_ENABLE) && defined(AIR_BTA_IC_PREMIUM_G2)

#if defined(AIR_AI_NR_PREMIUM_200K_ENABLE)
    if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
        IGO_NR_Init(g_voice_ecnr_nr_para->ScratchMemory, (uint32_t *)(g_voice_ecnr_nr_para->ResKey), &g_voice_ecnr_nr_para->ff_eq, g_igo_nr_level, _NB);
    } else {
        IGO_NR_Init(g_voice_ecnr_nr_para->ScratchMemory, (uint32_t *)(g_voice_ecnr_nr_para->ResKey), &g_voice_ecnr_nr_para->ff_eq, g_igo_nr_level, _WB);
    }
    DSP_MW_LOG_I("[DSP][VOICE_NR] NR init nr level %d", 1, g_igo_nr_level);
#elif defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)
    igo_mode = igo_nr_get_curr_mode();
    if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
        IGO_NR_Init(g_voice_ecnr_nr_para->ScratchMemory, (uint32_t *)(g_voice_ecnr_nr_para->ResKey), &g_voice_ecnr_nr_para->ff_eq, &g_voice_ecnr_nr_para->ff_eq_1, igo_mode, g_igo_nr_level, _NB);
    } else {
        IGO_NR_Init(g_voice_ecnr_nr_para->ScratchMemory, (uint32_t *)(g_voice_ecnr_nr_para->ResKey), &g_voice_ecnr_nr_para->ff_eq, &g_voice_ecnr_nr_para->ff_eq_1, igo_mode, g_igo_nr_level, _WB);
    }
    DSP_MW_LOG_I("[DSP][VOICE_NR] short boom mic NR (for open office) init nr level %d", 1, g_igo_nr_level);
#endif

#elif (defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_BTA_IC_PREMIUM_G2)) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)

    _BAND_MODE baud_mode;

    igo_mode = igo_nr_get_curr_mode();
    if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
        baud_mode = _SWB;
    } else {
        baud_mode = _WB;
    }
#if defined( AIR_AI_NR_PREMIUM_INEAR_270K_ENABLE)
    IGO_1MIC_NR_PARAMS *p_params;

    p_params = (IGO_1MIC_NR_PARAMS *)&g_voice_ecnr_nr_para->ResKey;
    DSP_MW_LOG_I("[DSP][VOICE_NR] NR init nr_strength %d, baud_mode %d", 2, p_params->nr_strength, baud_mode);
    UNUSED(igo_mode);
    IGO_NR_Init(g_voice_ecnr_nr_para->ScratchMemory, work_buf_size, p_params, &g_voice_ecnr_nr_para->ff_eq, &g_voice_ecnr_nr_para->fb_eq, baud_mode);
#else
#if defined(AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE)
    IGO_NR_Init(g_voice_ecnr_nr_para->ScratchMemory, (uint32_t *)g_voice_ecnr_nr_para->ResKey, &g_voice_ecnr_nr_para->ff_eq, &g_voice_ecnr_nr_para->ff_eq_1, &g_voice_ecnr_nr_para->fb_eq, (uint8_t)igo_mode, g_igo_nr_level, baud_mode);
#else

#if defined(AIR_AI_NR_PRIMARY_80K_ENABLE)
    IGO_NR_Init(g_voice_ecnr_nr_para->ScratchMemory, ROM_TABLE_ADDR_IGO_AINR_80K, (uint32_t *)g_voice_ecnr_nr_para->ResKey, &g_voice_ecnr_nr_para->ff_eq, &g_voice_ecnr_nr_para->fb_eq, (uint8_t)igo_mode, baud_mode);
#else
#if defined(ROM_TABLE_ADDR_IGO_AINR_270K)
    IGO_NR_Init(g_voice_ecnr_nr_para->ScratchMemory, ROM_TABLE_ADDR_IGO_AINR_270K, (uint32_t *)g_voice_ecnr_nr_para->ResKey, &g_voice_ecnr_nr_para->ff_eq, &g_voice_ecnr_nr_para->fb_eq, (uint8_t)igo_mode, g_igo_nr_level, baud_mode);
#else
    IGO_NR_Init(g_voice_ecnr_nr_para->ScratchMemory, (uint32_t *)g_voice_ecnr_nr_para->ResKey, &g_voice_ecnr_nr_para->ff_eq, &g_voice_ecnr_nr_para->fb_eq, (uint8_t)igo_mode, g_igo_nr_level, baud_mode);
#endif
#endif
#endif
    DSP_MW_LOG_I("[DSP][VOICE_NR] NR init igo_mode %d, g_igo_nr_level %d, baud_mode %d", 3, igo_mode, g_igo_nr_level, baud_mode);
#endif

#endif
    g_voice_ecnr_nr_para->common.init_flag = true;

    #if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
    g_igo_nr_curr_mode = igo_mode;
    voice_ecnr_check_txnr_lib_match(para);//check igo lib match stream
    #endif

    /* Query the alg version log */
#if defined(AIR_AI_NR_PREMIUM_ENABLE)
    version_info_t version_info;
    IGO_NR_Get_Lib_Version(&version_info);
    DSP_MW_LOG_I("[DSP][VOICE_NR] igo NR init done, version: %d.%d.%d.%d", 4, version_info.v1, version_info.v2, version_info.v3, version_info.v4);
#elif defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
    version_info_t version_info;
    IGO_NR_Get_Lib_Version(&version_info);
    DSP_MW_LOG_I("[DSP][VOICE_NR] igo NR (+1) init done, version: %d.%d.%d.%d", 4, version_info.v1, version_info.v2, version_info.v3, version_info.v4);
#else
    DSP_MW_LOG_I("[DSP][VOICE_NR] NR init done", 0);
#endif

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_nr_deinit(void)
{
    voice_ecnr_common_deinit((voice_ecnr_common_para_t **)&g_voice_ecnr_nr_para, DSP_ALG_ECNR_NR_BUFFER_MAGIC_NUMBER);

    return VOICE_ECNR_STATUS_OK;
}

bool voice_ecnr_need_check_match(void)
{
    #ifdef AIR_BTA_IC_PREMIUM_G2 //ab156x
        #ifdef AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE
        return false;
        #endif
    #elif defined(AIR_BTA_IC_PREMIUM_G3) //ab158x
        #if defined(AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE)
        return false;
        #endif
    #endif
    return true;
}

void voice_ecnr_check_lib_version(void)
{
    #if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
    version_info_t version_info;
    IGO_NR_Get_Lib_Version(&version_info);
    bool v1_check_result = false;
    bool v3_check_result = (version_info.v3 > 0);
    switch (version_info.v1) {
        case 2://156x official release -> 3RD_PARTY_AI_NR | 3RD_PARTY_AI_NR_INEAR
            #if defined(AIR_BTA_IC_PREMIUM_G2) && (defined(AIR_AI_NR_PREMIUM_200K_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_200K_ENABLE))
            v1_check_result = true;
            #endif
            break;
        case 4://157x official release -> 3RD_PARTY_AI_NR | 3RD_PARTY_AI_NR_INEAR
            #if defined(AIR_BTA_IC_STEREO_HIGH_G3) && (defined(AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE))
            v1_check_result = true;
            #endif
            break;
        case 5://158x official release -> 3RD_PARTY_AI_NR | 3RD_PARTY_AI_NR_INEAR
            #if defined(AIR_BTA_IC_PREMIUM_G3) && (defined(AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE))
            v1_check_result = true;
            #endif
            break;
        case 6://158x special release:pro distractor -> 3RD_PARTY_AI_NR_PRO_DISTRACTOR
            #if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE)
            v1_check_result = true;
            #endif
            break;
        case 7://156x special release:open office -> 3RD_PARTY_AI_NR_SHORT_BOOM_OO
            #if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)
            v1_check_result = true;
            #endif
            break;
        case 8://158x special release:open office -> 3RD_PARTY_AI_NR_PRO_TWS_OO
            #if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE)
            v1_check_result = true;
            #endif
            break;
        case 9://158x special release:pro broadside -> 3RD_PARTY_AI_NR_PRO_BROADSIDE_SEPARATE_MODE
            #if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE)
            v1_check_result = true;
            #endif
            break;
        default:
            break;
    }
    if(!(v1_check_result && v3_check_result)){
        DSP_MW_LOG_E("[DSP][VOICE_NR]not support nr lib version:%d.%d.%d.%d", 4, version_info.v1, version_info.v2, version_info.v3, version_info.v4);
        AUDIO_ASSERT(0);
    }
    #endif
}

bool voice_ecnr_check_entry_func_exist(void *para, stream_feature_entry_t entry_func)
{
    DSP_STREAMING_PARA_PTR pStream = DSP_STREAMING_GET_FROM_PRAR(para);
    DSP_FEATURE_TABLE_PTR feature_table_ptr = pStream->callback.FeatureTablePtr;
    bool check_exist = false;
    if (feature_table_ptr != NULL){
        while (feature_table_ptr->ProcessEntry != NULL){
            if(feature_table_ptr->ProcessEntry == entry_func){
                check_exist = true;
                break;
            }
            if(feature_table_ptr->ProcessEntry == stream_function_end_process){
                break;
            }
            feature_table_ptr++;
        }
    }
    return check_exist;
}

void voice_ecnr_check_txnr_lib_match(void *para)
{
    if(voice_ecnr_check_entry_func_exist(para, stream_function_aec_process) == false){
        DSP_MW_LOG_I("[DSP][VOICE_NR] stream has no tx nr, bypass lib match check", 0);
        return;
    }
    else if(voice_nr_check_mp() == false)	// richard for patch from Airoha
    {
	DSP_MW_LOG_I("[DSP][VOICE_NR] tx nr disabled, bypass lib match check", 0);
	return;
    }

    voice_ecnr_check_lib_version();//check version

    if(voice_ecnr_need_check_match() == false){ //special IGO nr lib no need check match for next check
        return;
    }
    DSP_STREAMING_PARA_PTR pStream = DSP_STREAMING_GET_FROM_PRAR(para);
    U32 channel_num = (pStream->source->param.audio.echo_reference ? stream_function_get_channel_number(para) - 1 : stream_function_get_channel_number(para));//- echo ch
    #ifdef AIR_BTA_IC_PREMIUM_G2 //ab156x
        #ifdef AIR_AI_NR_PREMIUM_ENABLE //1mic
            if(channel_num > 2){
                DSP_MW_LOG_E("[DSP][VOICE_NR] 1/2mic lib don't match stream channel num:%d", 1, channel_num);
                AUDIO_ASSERT(0);
            }
            #ifndef AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE
                //1mic nr need check if have FUNC_TX_EQ
                if(voice_ecnr_check_entry_func_exist(para, stream_function_tx_eq_process) == false){
                    DSP_MW_LOG_E("[DSP][VOICE_NR] 1mic lib need FUNC_TX_EQ feature", 0);
                }
            #endif
        #elif defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE) //1+1mic
            if(channel_num < 2 || channel_num > 3){
                DSP_MW_LOG_E("[DSP][VOICE_NR] 1 + 1mic lib don't match stream channel num:%d", 1, channel_num);
                AUDIO_ASSERT(0);
            }
        #endif
    #else //ab158x/157x
        U32 * nr_config = (U32 *)&g_voice_ecnr_nr_para->ResKey[0];
        bool is_1mic = nr_config[3] & 0x100000;
        if(is_1mic && channel_num > 2){ //1mic
            DSP_MW_LOG_E("[DSP][VOICE_NR] 1/2mic lib don't match stream channel num:%d", 1, channel_num);
            AUDIO_ASSERT(0);
        }
        if(is_1mic == false && (channel_num < 2 || channel_num > 3)){ //1+1mic
            DSP_MW_LOG_E("[DSP][VOICE_NR] 1 + 1mic lib don't match stream channel num:%d", 1, channel_num);
            AUDIO_ASSERT(0);
        }
    #endif
    DSP_MW_LOG_I("[DSP][VOICE_NR] nr lib check pass", 0);
}

voice_ecnr_status_t voice_ecnr_nr_process(void *para, uint32_t length, int16_t *main_mic, int16_t *ff_mic, int16_t *fb_mic, int16_t *ref, int16_t *out)
{
    UNUSED(para);

    if (length == 0) {
        return VOICE_ECNR_STATUS_BYPASS;
    }
    if (((gDspAlgParameter.EscoMode.Tx == VOICE_FB) && ((length != VOICE_FB_15_MS_FRAME_SIZE) && (length != VOICE_FB_5_MS_FRAME_SIZE))) ||
        ((gDspAlgParameter.EscoMode.Tx == VOICE_SWB) && (length != VOICE_SWB_15_MS_FRAME_SIZE)) ||
        ((gDspAlgParameter.EscoMode.Tx == VOICE_NB || gDspAlgParameter.EscoMode.Tx == VOICE_WB) && (length != VOICE_WB_15_MS_FRAME_SIZE))) {
        return VOICE_ECNR_STATUS_ERROR;
    }

#if (defined(AIR_BTA_IC_PREMIUM_G2) && (defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE) || defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE))) || (!defined(AIR_BTA_IC_PREMIUM_G2) && (defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)))
    if (igo_nr_get_curr_mode() != g_igo_nr_curr_mode) {
        voice_ecnr_nr_init(para);
    }
#endif

#if defined(AIR_AI_NR_PREMIUM_ENABLE) && defined(AIR_BTA_IC_PREMIUM_G2)

    if (gDspAlgParameter.EscoMode.Tx != VOICE_SWB && gDspAlgParameter.EscoMode.Tx != VOICE_FB) {
        #if defined(AIR_AI_NR_PREMIUM_200K_ENABLE)
            UNUSED(ff_mic);
            UNUSED(fb_mic);
            UNUSED(ref);
            IGO_NR_Prcs(main_mic, out, (uint32_t *)g_voice_ecnr_nr_para->ResKey, g_igo_nr_level, g_voice_ecnr_nr_para->ScratchMemory);
        #elif defined(AIR_AI_NR_PREMIUM_200K_SHORT_BOOM_OO_ENABLE)
            _SWITCH_MODE igo_mode;
            UNUSED(fb_mic);
            igo_mode = igo_nr_get_curr_mode();
            IGO_NR_Prcs(main_mic, ff_mic, ref, out, (uint32_t *)g_voice_ecnr_nr_para->ResKey, igo_mode, g_igo_nr_level, g_voice_ecnr_nr_para->ScratchMemory);
        #else
            UNUSED(main_mic);
            UNUSED(ff_mic);
            UNUSED(fb_mic);
            UNUSED(ref);
        #endif
    } else {
            /*Do nothing since there is no iGO SWB Lib*/
            memcpy(out, main_mic, length);
            /*DSP_MW_LOG_I("[DSP][VOICE_NR] G2 SWB Bypass iGO NR", 0);*/
    }

#elif (defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_BTA_IC_PREMIUM_G2)) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)

    if (gDspAlgParameter.EscoMode.Tx != VOICE_FB) {
        _SWITCH_MODE igo_mode;
        igo_mode = igo_nr_get_curr_mode();
        #if defined( AIR_AI_NR_PREMIUM_INEAR_270K_ENABLE)
            UNUSED(ff_mic);
            IGO_NR_Prcs(main_mic, fb_mic, ref, out, g_voice_ecnr_nr_para->ScratchMemory, igo_mode);
        #else
            #if defined(AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE)
                IGO_NR_Prcs(main_mic, ff_mic, fb_mic, ref, out, (uint32_t *)g_voice_ecnr_nr_para->ResKey, (uint8_t)igo_mode, g_igo_nr_level, g_voice_ecnr_ec_postec_fb_gain, g_voice_ecnr_nr_para->ScratchMemory);
            #elif defined( AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE)
                IGO_NR_Prcs(main_mic, ff_mic, fb_mic, ref, out, (uint32_t *)g_voice_ecnr_nr_para->ResKey, (uint8_t)igo_mode, g_igo_nr_level, g_voice_ecnr_ec_postec_fb_gain, g_voice_ecnr_ec_postec_wind_info, g_voice_ecnr_nr_para->ScratchMemory);
            #elif defined(AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE)
                UNUSED(ff_mic);
                #if defined(BASE_STEREO_HIGH_G3_TYPE_72)
                IGO_NR_Prcs(main_mic, fb_mic, ref, out, g_voice_ecnr_nr_para->ScratchMemory);
                #else
                IGO_NR_Prcs(main_mic, fb_mic, ref, out, (uint32_t *)g_voice_ecnr_nr_para->ResKey, (uint8_t)igo_mode, g_igo_nr_level, g_voice_ecnr_ec_postec_fb_gain, g_voice_ecnr_ec_postec_wind_info, g_voice_ecnr_nr_para->ScratchMemory);
                #endif
            #else
                UNUSED(ff_mic);
                #if defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_AI_NR_PRIMARY_80K_ENABLE)
                    IGO_NR_Prcs(main_mic, fb_mic, ref, out, g_voice_ecnr_nr_para->ScratchMemory);
                #else
                    IGO_NR_Prcs(main_mic, fb_mic, ref, out, (uint32_t *)g_voice_ecnr_nr_para->ResKey, (uint8_t)igo_mode, g_igo_nr_level, g_voice_ecnr_ec_postec_fb_gain, g_voice_ecnr_nr_para->ScratchMemory);
                #endif
            #endif
        #endif
    } else {
        /*Do nothing since there is no iGO FB Lib*/
        memcpy(out, main_mic, length);
    }
#else

    UNUSED(main_mic);
    UNUSED(ff_mic);
    UNUSED(fb_mic);
    UNUSED(ref);

#endif


#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((uint8_t *)out, length, AUDIO_ENHANCEMENT_OUT_L);
#endif

    return VOICE_ECNR_STATUS_OK;
}

uint32_t voice_ecnr_nr_query_version(uint8_t *version)
{
    uint32_t length;

    if (voice_ecnr_nr_check_running_status() == true) {
#if defined(AIR_AI_NR_PREMIUM_INEAR_200K_ENABLE)
        IGO_NR_Get_Lib_Version((version_info_t *)version);
        length = sizeof(version_info_t);
#else
        UNUSED(version);
        length = 0;
#endif
    } else {
        length = 0;
    }

    return length;
}

void voice_ecnr_nr_set_nr_level(uint32_t nr_level)
{
#if (defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE) || defined(AIR_AI_NR_PREMIUM_ENABLE))
#if !defined( AIR_AI_NR_PREMIUM_INEAR_270K_ENABLE)
    if (nr_level < _NR_LEVEL_MAX) {
        g_igo_nr_level = nr_level;
        DSP_MW_LOG_I("[DSP][AEC/NR] IGO NR level set to %d", 1, nr_level);
    } else {
        DSP_MW_LOG_E("[DSP][AEC/NR] IGO NR level invalid %d, max %d", 2, nr_level, _NR_LEVEL_MAX);
    }
#else
    UNUSED(nr_level);
#endif
#else
    UNUSED(nr_level);
#endif
}

#endif


/************************************
 * For PostEC library porting
 ************************************/
#if defined(AIR_3RD_PARTY_NR_ENABLE)
#if defined(AIR_ECNR_POST_PART_ENABLE)

#ifndef AIR_ECNR_SEPARATE_MODE_ENABLE

typedef struct {
    voice_ecnr_common_para_t common;
    DSP_ALIGN4 DSP_PARA_AEC_STRU nvkey_aec;
#ifdef AIR_BTA_IC_PREMIUM_G2
    DSP_ALIGN4 DSP_PARA_NR_STRU nvkey_nr;
    DSP_ALIGN4 DSP_PARA_INEAR_STRU nvkey_inear;
    DSP_ALIGN4 DSP_PARA_AEC_NR_STRU nvkey_aec_nr;
#endif
    DSP_ALIGN16 uint8_t ScratchMemory[DSP_AEC_NR_SCRATCH_MEMSIZE]; //Set at the end of structure for dynamic size
} voice_ecnr_postec_para_t;

static voice_ecnr_postec_para_t *g_voice_ecnr_postec_para = NULL;

__attribute__(( weak )) uint8_t aec_nr_port_get_postec_gain(void)
{
    return 0;
}

bool voice_ecnr_postec_check_running_status(void)
{
    return voice_ecnr_check_running_status_internal((voice_ecnr_common_para_t *)g_voice_ecnr_postec_para, DSP_ALG_ECNR_POSTEC_BUFFER_MAGIC_NUMBER);
}

voice_ecnr_status_t voice_ecnr_postec_init(void *para)
{
    voice_ecnr_status_t ret;
    uint32_t alloc_size;

    alloc_size = sizeof(voice_ecnr_postec_para_t) + get_post_ec_memsize();
    //AUDIO_ASSERT((g_voice_ecnr_nr_memsize + alloc_size) <= stream_function_get_working_buffer_length(para));

    /* Check whether init is needed */
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    uint8_t *p_voice_nr_buf;
    p_voice_nr_buf = (uint8_t *)g_voice_ecnr_nr_para;
    g_voice_ecnr_postec_para = (voice_ecnr_postec_para_t *)(p_voice_nr_buf + ((g_voice_ecnr_nr_memsize + 7) / 8) * 8);
    ret = voice_ecnr_common_init(para, (voice_ecnr_common_para_t **)&g_voice_ecnr_postec_para, false, alloc_size, DSP_ALG_ECNR_POSTEC_BUFFER_MAGIC_NUMBER);
#else
    ret = voice_ecnr_common_init(para, (voice_ecnr_common_para_t **)&g_voice_ecnr_postec_para, true, alloc_size, DSP_ALG_ECNR_POSTEC_BUFFER_MAGIC_NUMBER);
#endif
    if (ret == VOICE_ECNR_STATUS_BYPASS) {
        return VOICE_ECNR_STATUS_BYPASS;
    }

    /* load nvkey */
    nvkey_read_full_key(NVKEY_DSP_PARA_AEC, &g_voice_ecnr_postec_para->nvkey_aec, sizeof(DSP_PARA_AEC_STRU));
#ifdef AIR_BTA_IC_PREMIUM_G2
    nvkey_read_full_key(NVKEY_DSP_PARA_NR, &g_voice_ecnr_postec_para->nvkey_nr, sizeof(DSP_PARA_NR_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_INEAR, &g_voice_ecnr_postec_para->nvkey_inear, sizeof(DSP_PARA_INEAR_STRU));
    voice_nvkey_aec_nr_porting_layer(&g_voice_ecnr_postec_para->nvkey_aec, &g_voice_ecnr_postec_para->nvkey_nr, &g_voice_ecnr_postec_para->nvkey_inear, &g_voice_ecnr_postec_para->nvkey_aec_nr);
#endif

    /* Init the alg */
#ifdef AIR_BTA_IC_PREMIUM_G2
    EXT_POST_EC_Init(g_voice_ecnr_postec_para->ScratchMemory, &g_voice_ecnr_postec_para->nvkey_aec_nr);
#elif defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
    EXT_POST_EC_Init(g_voice_ecnr_postec_para->ScratchMemory, &g_voice_ecnr_postec_para->nvkey_aec);
#endif

    g_voice_ecnr_nvkey_aec = &g_voice_ecnr_postec_para->nvkey_aec;

    g_voice_ecnr_postec_para->common.init_flag = true;

    /* Query the alg version log */
    DSP_MW_LOG_I("[DSP][VOICE_NR] Post EC init done", 0);

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_postec_deinit(void)
{
    voice_ecnr_common_deinit((voice_ecnr_common_para_t **)&g_voice_ecnr_postec_para, DSP_ALG_ECNR_POSTEC_BUFFER_MAGIC_NUMBER);

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_postec_process(void *para, uint32_t length, int16_t *in, int16_t *out)
{
    uint8_t post_ec_gain;

    UNUSED(para);

    if (length == 0) {
        return VOICE_ECNR_STATUS_BYPASS;
    }
    if (((gDspAlgParameter.EscoMode.Tx == VOICE_SWB) && (length != VOICE_SWB_15_MS_FRAME_SIZE)) ||
        ((gDspAlgParameter.EscoMode.Tx == VOICE_WB) && (length != VOICE_WB_15_MS_FRAME_SIZE))) {
        DSP_MW_LOG_E("[DSP][VOICE_NR] frame size invalid, mode %d, actual: %d", 2, gDspAlgParameter.EscoMode.Tx, length);
        return VOICE_ECNR_STATUS_ERROR;
    }

    /* Using the end of stream buffer for extra data transmit */
    post_ec_gain = aec_nr_port_get_postec_gain();
    EXT_POST_EC_PRCS(g_voice_ecnr_postec_para->ScratchMemory, in, out, post_ec_gain, &(gDspAlgParameter.AecNr));

#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((uint8_t *)out, length, VOICE_TX_NR_OUT);
#endif

    return VOICE_ECNR_STATUS_OK;
}

#endif
#endif
#endif


/************************************
 * For TX EQ library porting
 ************************************/
#if defined(AIR_3RD_PARTY_NR_ENABLE) && !defined(AIR_BTA_IC_STEREO_HIGH_G3)

typedef struct {
    voice_ecnr_common_para_t common;
    DSP_ALIGN4 DSP_PARA_WB_TX_FIR_EQ_STRU TxFirEq;
    DSP_ALIGN16 uint8_t ScratchMemory[DSP_AEC_NR_SCRATCH_MEMSIZE]; //Set at the end of structure for dynamic size
} voice_ecnr_tx_eq_para_t;

static voice_ecnr_tx_eq_para_t *g_voice_ecnr_tx_eq_para = NULL;

bool voice_ecnr_tx_eq_check_running_status(void)
{
    return voice_ecnr_check_running_status_internal((voice_ecnr_common_para_t *)g_voice_ecnr_tx_eq_para, DSP_ALG_ECNR_TXEQ_BUFFER_MAGIC_NUMBER);
}

voice_ecnr_status_t voice_ecnr_tx_eq_init(void *para)
{
    uint16_t key_id;
    voice_ecnr_status_t ret;

    /* Check whether init is needed */
    ret = voice_ecnr_common_init(para, (voice_ecnr_common_para_t **)&g_voice_ecnr_tx_eq_para, true, sizeof(voice_ecnr_tx_eq_para_t) + get_fir_wbeq_memsize(), DSP_ALG_ECNR_TXEQ_BUFFER_MAGIC_NUMBER);
    if (ret == VOICE_ECNR_STATUS_BYPASS) {
        return VOICE_ECNR_STATUS_BYPASS;
    }

    /* load nvkey */
    if (gDspAlgParameter.EscoMode.Tx == VOICE_WB) {
        key_id = NVKEY_DSP_PARA_WB_TX_FIR_EQ;
        nvkey_read_full_key(key_id, &g_voice_ecnr_tx_eq_para->TxFirEq, sizeof(DSP_PARA_WB_TX_FIR_EQ_STRU));
    } else if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
        key_id = NVKEY_DSP_PARA_NB_TX_FIR_EQ;
        nvkey_read_full_key(key_id, &g_voice_ecnr_tx_eq_para->TxFirEq, sizeof(DSP_PARA_NB_TX_FIR_EQ_STRU));
    }

    /* Init the alg */
    Voice_TxEQ_init(&g_voice_ecnr_tx_eq_para->ScratchMemory, &g_voice_ecnr_tx_eq_para->TxFirEq);

    g_voice_ecnr_tx_eq_para->common.init_flag = true;

    /* Query the alg version log */
    DSP_MW_LOG_I("[DSP][VOICE_NR] inhouse TX EQ version", 0);

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_tx_eq_deinit(void)
{
    voice_ecnr_common_deinit((voice_ecnr_common_para_t **)&g_voice_ecnr_tx_eq_para, DSP_ALG_ECNR_TXEQ_BUFFER_MAGIC_NUMBER);

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_tx_eq_process(void *para, uint32_t length, int16_t *in_out)
{
    UNUSED(para);

    Voice_TxEQ_Prcs_Length(in_out, &g_voice_ecnr_tx_eq_para->ScratchMemory, length >> 1);

    return VOICE_ECNR_STATUS_OK;
}

#endif



/************************************
 * For EC/PostEC library porting
 ************************************/
#if defined(AIR_3RD_PARTY_NR_ENABLE)

typedef struct {
    voice_ecnr_common_para_t common;
    DSP_ALIGN4 nvkey_rx_nr_config_t nvkey_rx;
    DSP_ALIGN4 DSP_PARA_AEC_STRU nvkey_aec;
    DSP_ALIGN4 DSP_PARA_NR_STRU nvkey_nr;
    DSP_ALIGN4 DSP_PARA_INEAR_STRU nvkey_inear;
    DSP_ALIGN4 DSP_PARA_AEC_NR_SWB_STRU nvkey_nr_swb;
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
    DSP_ALIGN16 int32_t EC_out1_buf[720];//worst case: 48k/ 24bit/ 15ms
    DSP_ALIGN16 int32_t EC_out2_buf[720];//worst case: 48k/ 24bit/ 15ms
    DSP_ALIGN16 int32_t EC_fb_buf[720];  //worst case: 48k/ 24bit/ 15ms
#else
    DSP_ALIGN16 int16_t EC_out1_buf[480];//worst case: 32k/ 16bit/ 15ms
    DSP_ALIGN16 int16_t EC_out2_buf[480];//worst case: 32k/ 16bit/ 15ms
    DSP_ALIGN16 int16_t EC_fb_buf[480];  //worst case: 32k/ 16bit/ 15ms
#endif
    DSP_ALIGN16 uint8_t ScratchMemory[DSP_AEC_NR_SCRATCH_MEMSIZE]; //Set at the end of structure for dynamic size
} voice_ecnr_ec_postec_para_t;

static voice_ecnr_ec_postec_para_t *g_voice_ecnr_ec_postec_para = NULL;

bool voice_ecnr_ec_postec_get_framework_workingbuffer(int16_t** EC_out1_buf, int16_t** EC_out2_buf, int16_t** EC_fb_buf)
{
    *EC_out1_buf = (int16_t*)g_voice_ecnr_ec_postec_para->EC_out1_buf;
    *EC_out2_buf = (int16_t*)g_voice_ecnr_ec_postec_para->EC_out2_buf;
    *EC_fb_buf   = (int16_t*)g_voice_ecnr_ec_postec_para->EC_fb_buf;
    return true;
}

bool voice_ecnr_ec_postec_check_running_status(void)
{
    return voice_ecnr_check_running_status_internal((voice_ecnr_common_para_t *)g_voice_ecnr_ec_postec_para, DSP_ALG_ECNR_EC_POSTEC_BUFFER_MAGIC_NUMBER);
}

voice_ecnr_status_t voice_ecnr_ec_postec_init(void *para)
{
    uint32_t alloc_size = 0;
    voice_ecnr_status_t ret;
    bool is_nb_mode;

    if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
        is_nb_mode = true;
    } else {
        is_nb_mode = false;
    }

    /* Check whether init is needed */
    if (g_voice_ecnr_mic_mode <= VOICE_ECNR_MIC_MODE_2) {
        if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
#if defined (AIR_VOICE_BAND_CONFIG_TYPE_FB)
            alloc_size = get_aec_nr_memsize_fb() + sizeof(voice_ecnr_ec_postec_para_t);
#endif
        } else {
            alloc_size = get_aec_nr_memsize() + sizeof(voice_ecnr_ec_postec_para_t);
        }
    } else {
        if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
            alloc_size = 0;
            DSP_MW_LOG_E("[DSP][VOICE_NR] FullBand Inear Algorithm not available yet", 0);
            assert(0);
        } else {
            alloc_size = get_aec_nr_inear_memsize() + sizeof(voice_ecnr_ec_postec_para_t);
        }
    }

    //AUDIO_ASSERT((g_voice_ecnr_nr_memsize + alloc_size) <= stream_function_get_working_buffer_length(para));

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    uint8_t *p_voice_nr_buf;
    p_voice_nr_buf = (uint8_t *)g_voice_ecnr_nr_para;
    g_voice_ecnr_ec_postec_para = (voice_ecnr_ec_postec_para_t *)(p_voice_nr_buf + ((g_voice_ecnr_nr_memsize + 7) / 8) * 8);
    ret = voice_ecnr_common_init(para, (voice_ecnr_common_para_t **)&g_voice_ecnr_ec_postec_para, false, alloc_size, DSP_ALG_ECNR_EC_POSTEC_BUFFER_MAGIC_NUMBER);
#else
    ret = voice_ecnr_common_init(para, (voice_ecnr_common_para_t **)&g_voice_ecnr_ec_postec_para, true, alloc_size, DSP_ALG_ECNR_EC_POSTEC_BUFFER_MAGIC_NUMBER);
#endif
    if (ret == VOICE_ECNR_STATUS_BYPASS) {
        return VOICE_ECNR_STATUS_BYPASS;
    }

    /* load nvkey */
    nvkey_read_full_key(NVKEY_DSP_PARA_AEC, &g_voice_ecnr_ec_postec_para->nvkey_aec, sizeof(DSP_PARA_AEC_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_NR, &g_voice_ecnr_ec_postec_para->nvkey_nr, sizeof(DSP_PARA_NR_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_INEAR, &g_voice_ecnr_ec_postec_para->nvkey_inear, sizeof(DSP_PARA_INEAR_STRU));
    if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
        nvkey_read_full_key(NVKEY_DSP_PARA_AEC_NR_SWB, &g_voice_ecnr_ec_postec_para->nvkey_nr_swb, sizeof(DSP_PARA_AEC_NR_SWB_STRU));
    } else if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
        nvkey_read_full_key(NVKEY_DSP_PARA_AEC_NR_SWB, &g_voice_ecnr_ec_postec_para->nvkey_nr_swb, sizeof(DSP_PARA_AEC_NR_SWB_STRU));
    }

    /* Init the alg */
    if (g_voice_ecnr_mic_mode <= VOICE_ECNR_MIC_MODE_2) {
        if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
            Voice_FB_EC_Init(&g_voice_ecnr_ec_postec_para->ScratchMemory,
                              &g_voice_ecnr_ec_postec_para->nvkey_aec, &g_voice_ecnr_ec_postec_para->nvkey_nr,
                              &g_voice_ecnr_ec_postec_para->nvkey_inear, &g_voice_ecnr_ec_postec_para->nvkey_nr_swb);
#endif
        } else if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            Voice_SWB_EC_Init(&g_voice_ecnr_ec_postec_para->ScratchMemory,
                              &g_voice_ecnr_ec_postec_para->nvkey_aec, &g_voice_ecnr_ec_postec_para->nvkey_nr,
                              &g_voice_ecnr_ec_postec_para->nvkey_inear, &g_voice_ecnr_ec_postec_para->nvkey_nr_swb);
        } else {
            Voice_EC_Init(is_nb_mode, &g_voice_ecnr_ec_postec_para->ScratchMemory,
                          &g_voice_ecnr_ec_postec_para->nvkey_aec, &g_voice_ecnr_ec_postec_para->nvkey_nr,
                          &g_voice_ecnr_ec_postec_para->nvkey_inear, (int16_t *)&g_voice_ecnr_ec_postec_para->nvkey_nr.WB_NR_TX_NOISE_GAIN_LIMITER);
        }
    } else {
        if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
            DSP_MW_LOG_E("[DSP][VOICE_NR] FullBand Inear Algorithm not available yet", 0);
            assert(0);
        } else if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            Voice_SWB_EC_Inear_Init(&g_voice_ecnr_ec_postec_para->ScratchMemory,
                                    &g_voice_ecnr_ec_postec_para->nvkey_aec, &g_voice_ecnr_ec_postec_para->nvkey_nr,
                                    &g_voice_ecnr_ec_postec_para->nvkey_inear, &g_voice_ecnr_ec_postec_para->nvkey_nr_swb);
        } else {
            Voice_EC_Inear_Init(is_nb_mode, &g_voice_ecnr_ec_postec_para->ScratchMemory,
                                &g_voice_ecnr_ec_postec_para->nvkey_aec, &g_voice_ecnr_ec_postec_para->nvkey_nr,
                                &g_voice_ecnr_ec_postec_para->nvkey_inear, (int16_t *)&g_voice_ecnr_ec_postec_para->nvkey_nr.WB_NR_TX_NOISE_GAIN_LIMITER);
        }
    }

    g_voice_ecnr_nvkey_aec = &g_voice_ecnr_ec_postec_para->nvkey_aec;

    /* Init RX NR part */
    voice_rxnr_init(&g_voice_ecnr_ec_postec_para->ScratchMemory, &g_voice_ecnr_ec_postec_para->nvkey_nr,
                    &g_voice_ecnr_ec_postec_para->nvkey_nr_swb, &g_voice_ecnr_ec_postec_para->nvkey_rx);
    g_voice_ecnr_ec_postec_para->common.init_flag = true;

#if defined(AIR_VOICE_NR_USE_PIC_ENABLE)
    if (gDspAlgParameter.EscoMode.Tx != VOICE_FB) {
        Assign_EC_RXNR_scratch_memory(&g_voice_ecnr_ec_postec_para->ScratchMemory);/*EC IP with static lib for initial function*/
    }
#endif


    /* Query the alg version log */
    if (gDspAlgParameter.EscoMode.Tx != VOICE_FB) {
        DSP_MW_LOG_I("[DSP][VOICE_NR] inhouse EC / POST_EC version %d", 1, get_ECNR_SVN()); //should compare with get_ECNR_Init_SVN();
    } else {
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
        DSP_MW_LOG_I("[DSP][VOICE_NR] inhouse EC / POST_EC FB version %d", 1, get_ECNR_SVN_FB());//FB
#endif
    }

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_ec_postec_deinit(void)
{
    voice_ecnr_common_deinit((voice_ecnr_common_para_t **)&g_voice_ecnr_ec_postec_para, DSP_ALG_ECNR_EC_POSTEC_BUFFER_MAGIC_NUMBER);

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_ec_postec_ec_process(void *para, uint32_t length, int16_t *main_mic, int16_t *ff_mic, int16_t *fb_mic, int16_t *ec_in, int16_t *main_out, int16_t *ff_out, int16_t *fb_out)
{
    UNUSED(para);
    UNUSED(length);
    UNUSED(ff_out);

    if (g_voice_ecnr_mic_mode <= VOICE_ECNR_MIC_MODE_2) {
        if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
            #ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
            Voice_FB_EC_1M_Prcs((S32*)main_mic, (S32*)ec_in, (S32*)main_out);
            #endif
        } else
        if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            Voice_SWB_EC_Prcs(main_mic, ff_mic, ec_in, main_out, ff_out);
        } else {
            Voice_EC_Prcs(main_mic, ff_mic, ec_in, main_out, ff_out);
        }
#if (defined(AIR_AI_NR_PREMIUM_ENABLE) && !defined(AIR_BTA_IC_PREMIUM_G2))
        g_voice_ecnr_ec_postec_fb_gain = 0x7FFF;
#endif
    } else {
        if (fb_mic == NULL) {
            fb_mic = ff_mic;
            ff_mic = NULL;
        }
        if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
            DSP_MW_LOG_E("[DSP][VOICE_NR] FB Inear Algorithm not available yet", 0);
            assert(0);
        } else if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            Voice_SWB_EC_Inear_Prcs(main_mic, ff_mic, fb_mic, ec_in, main_out, ff_out, fb_out);
        } else {
            Voice_EC_Inear_Prcs(main_mic, ff_mic, fb_mic, ec_in, main_out, ff_out, fb_out);
        }
#if defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
#if defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE) || defined(AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE)
        g_voice_ecnr_ec_postec_wind_info = WIND_INFO_READBACK();
#endif
#if !defined( AIR_AI_NR_PREMIUM_INEAR_270K_ENABLE)
        g_voice_ecnr_ec_postec_fb_gain = PF_GAIN_FB_READBACK();
        if ((g_voice_ecnr_nvkey_aec->AEC_NR_EN & 0x0003) == 0x0003) {
            g_voice_ecnr_ec_postec_fb_gain = 0x7FFF;
        }
#ifdef AIR_AUDIO_DUMP_ENABLE
        uint16_t fb_gain_dump[10];
        memset(fb_gain_dump, 0, sizeof(fb_gain_dump));
        fb_gain_dump[0] = 0xACE0;
        fb_gain_dump[1] = length / 2;
        fb_gain_dump[2] = g_voice_ecnr_ec_postec_fb_gain;
#if defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE) || defined(AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE)
        fb_gain_dump[3] = g_voice_ecnr_ec_postec_wind_info;
#endif
        LOG_AUDIO_DUMP((U8*)fb_gain_dump, sizeof(fb_gain_dump), VOICE_FB_ECOUT_INFO);
        if (fb_out != NULL) {
            LOG_AUDIO_DUMP((U8*)fb_out, length, VOICE_FB_ECOUT_INFO);
        }
#endif
#endif
#endif
    }

#ifdef AIR_AUDIO_DUMP_ENABLE
    if (gDspAlgParameter.EscoMode.Tx != VOICE_FB) {
        LOG_AUDIO_DUMP((uint8_t *)main_out, length, AUDIO_ENHANCEMENT_IN_L);
        if (ff_mic != NULL) {
            LOG_AUDIO_DUMP((uint8_t *)ff_out, length, AUDIO_ENHANCEMENT_OUT_R);
        }
        if (fb_mic != NULL) {
            LOG_AUDIO_DUMP((uint8_t *)fb_out, length, AUDIO_ENHANCEMENT_IN_R);
        }
    } else {
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
        extern void ShareBufferCopy_D_32bit_to_D_24bit_1ch(uint32_t* src_buf, uint8_t* dest_buf1, uint32_t samples);
        uint8_t *dest = (uint8_t*)main_mic; //use input buffer as a temp buffer for audio dump
        uint8_t samples = length>>2;
        uint32_t length2=length*3/4;
        uint32_t* src_buf[3]             = {(uint32_t*)main_out, (uint32_t*)ff_out, (uint32_t*)fb_out};
        DSP_DATADUMP_MASK_BIT dump_id[3] = {AUDIO_ENHANCEMENT_IN_L, AUDIO_ENHANCEMENT_OUT_R, AUDIO_ENHANCEMENT_IN_R};
        for (uint32_t i = 0; i < sizeof(dump_id)/sizeof(DSP_DATADUMP_MASK_BIT); i++) {
            if (check_cfg_dump_id(dump_id[i])) {
                ShareBufferCopy_D_32bit_to_D_24bit_1ch((uint32_t*)src_buf[i], dest, samples);
                LOG_AUDIO_DUMP((uint8_t *)dest, length2, dump_id[i]);
            }
        }
#endif
    }
#endif

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_ec_postec_process(void *para, uint32_t length, int16_t *in, int16_t *out)
{
    UNUSED(para);
    UNUSED(length);

    if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
        Voice_FB_PostEC_Prcs((S32 *)in, (S32 *)out, &gDspAlgParameter.AecNr);
#endif
    } else if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
        Voice_SWB_PostEC_Prcs(in, out, &gDspAlgParameter.AecNr);
    } else {
        Voice_PostEC_Prcs(in, out, &gDspAlgParameter.AecNr);
    }

    return VOICE_ECNR_STATUS_OK;
}

uint32_t voice_ecnr_ec_postec_get_ref_gain(int16_t *gain_buf)
{
    uint32_t i, length;

    length = 0;
    if (voice_ecnr_ec_postec_check_running_status() == true) {
        if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
        #ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
            EC_REF_GAIN_READBACK_FB(gain_buf);
        #endif
        } else {
            EC_REF_GAIN_READBACK(gain_buf);
        }

        length += 8;
        for (i = 0; i < 8; i++) {
            DSP_MW_LOG_I("[DSP][VOICE_NR] DSP get ref gain, GainAddr:0x%x", 1, gain_buf[i]);
        }
#if defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
        if (gDspAlgParameter.EscoMode.Tx != VOICE_FB) {
            EC_Inear_REF_GAIN_READBACK(gain_buf + length);
            length += 4;
            for (i = 0; i < 4; i++) {
                DSP_MW_LOG_I("[DSP][VOICE_NR] DSP get inear ref gain, GainAddr:0x%x", 1, gain_buf[length + i]);
            }
        } else {
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
            DSP_MW_LOG_I("[DSP][VOICE_NR] FB isn't yet support inear ref gain", 0);
#endif
        }
#endif
        if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            BAND5_EC_REF_GAIN_READBACK(gain_buf + length);
            for (i = 0; i < 3; i++) {
                DSP_MW_LOG_I("[DSP][VOICE_NR] DSP get band5 ref gain, GainAddr:0x%x", 1, gain_buf[length + i]);
            }
        }
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
         else if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
             BAND5_EC_REF_GAIN_READBACK_FB(gain_buf + length);
             for (i = 0; i < 3; i++) {
                 DSP_MW_LOG_I("[DSP][VOICE_NR] DSP get fb band5 ref gain, GainAddr:0x%x", 1, gain_buf[length + i]);
             }
         }
#endif
    }

    return length;
}

#endif






/*****************************************
 * For ECNR(EC/NR/PostEC) library porting
 ****************************************/
#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_ECNR_1_OR_2_MIC_ENABLE)

typedef struct {
    voice_ecnr_common_para_t common;
    DSP_ALIGN4 nvkey_rx_nr_config_t nvkey_rx;
    DSP_ALIGN4 DSP_PARA_AEC_STRU nvkey_aec;
    DSP_ALIGN4 DSP_PARA_NR_STRU nvkey_nr;
    DSP_ALIGN4 DSP_PARA_INEAR_STRU nvkey_inear;
    DSP_ALIGN4 DSP_PARA_SWB_TX_EQ_STRU nvkey_tx_eq;
    DSP_ALIGN4 DSP_PARA_AEC_NR_SWB_STRU nvkey_aec_nr_swb;
    DSP_ALIGN4 DSP_PARA_InEarEQ_STRU nvkey_inear_eq;
    DSP_ALIGN4 DSP_PARA_AST_EQ_STRU nvkey_ast_eq;
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
    DSP_ALIGN16 int32_t out_buf[720];//worst case: 48k/ 24bit/ 15ms
#else
    DSP_ALIGN16 int16_t out_buf[480];//worst case: 32k/ 16bit/ 15ms
#endif
    DSP_ALIGN16 uint8_t ScratchMemory[DSP_AEC_NR_SCRATCH_MEMSIZE]; //Set at the end of structure for dynamic size
} voice_ecnr_all_para_t;

static voice_ecnr_all_para_t *g_voice_ecnr_all_para = NULL;

bool voice_ecnr_all_get_framework_workingbuffer(int16_t** out_buf)
{
    *out_buf = (int16_t*)g_voice_ecnr_all_para->out_buf;
    return true;
}

bool voice_ecnr_all_check_running_status(void)
{
    return voice_ecnr_check_running_status_internal((voice_ecnr_common_para_t *)g_voice_ecnr_all_para, DSP_ALG_ECNR_ALL_BUFFER_MAGIC_NUMBER);
}

voice_ecnr_status_t voice_ecnr_all_init(void *para)
{
    bool is_nb_mode;
    uint32_t alloc_size;
    voice_ecnr_status_t ret;

    /* Check whether init is needed */
    alloc_size = get_aec_nr_memsize() + sizeof(voice_ecnr_all_para_t);
    ret = voice_ecnr_common_init(para, (voice_ecnr_common_para_t **)&g_voice_ecnr_all_para, true, alloc_size, DSP_ALG_ECNR_ALL_BUFFER_MAGIC_NUMBER);
    if (ret == VOICE_ECNR_STATUS_BYPASS) {
        return VOICE_ECNR_STATUS_BYPASS;
    }

    /* load nvkey */
    nvkey_read_full_key(NVKEY_DSP_PARA_AEC, &g_voice_ecnr_all_para->nvkey_aec, sizeof(DSP_PARA_AEC_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_NR, &g_voice_ecnr_all_para->nvkey_nr, sizeof(DSP_PARA_NR_STRU));
    nvkey_read_full_key(NVKEY_DSP_PARA_INEAR, &g_voice_ecnr_all_para->nvkey_inear, sizeof(DSP_PARA_INEAR_STRU));

    if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
        nvkey_read_full_key(NVKEY_DSP_PARA_NB_TX_EQ, &g_voice_ecnr_all_para->nvkey_tx_eq, sizeof(DSP_PARA_NB_TX_EQ_STRU));
    } else if (gDspAlgParameter.EscoMode.Tx == VOICE_WB) {
        nvkey_read_full_key(NVKEY_DSP_PARA_WB_TX_EQ, &g_voice_ecnr_all_para->nvkey_tx_eq, sizeof(DSP_PARA_WB_TX_EQ_STRU));
    } else {
        nvkey_read_full_key(NVKEY_DSP_PARA_SWB_TX_EQ, &g_voice_ecnr_all_para->nvkey_tx_eq, sizeof(DSP_PARA_SWB_TX_EQ_STRU));
        nvkey_read_full_key(NVKEY_DSP_PARA_AEC_NR_SWB, &g_voice_ecnr_all_para->nvkey_aec_nr_swb, sizeof(DSP_PARA_AEC_NR_SWB_STRU));
    }
    if (g_voice_ecnr_mic_mode > VOICE_ECNR_MIC_MODE_2) {
        nvkey_read_full_key(NVKEY_DSP_PARA_INEAR_EQ, &g_voice_ecnr_all_para->nvkey_inear_eq, sizeof(DSP_PARA_InEarEQ_STRU));
        nvkey_read_full_key(NVKEY_DSP_PARA_AST_EQ, &g_voice_ecnr_all_para->nvkey_ast_eq, sizeof(DSP_PARA_AST_EQ_STRU));
    }

    /* Init the alg */
    if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
        is_nb_mode = true;
    } else {
        is_nb_mode = false;
    }
    if (g_voice_ecnr_mic_mode <= VOICE_ECNR_MIC_MODE_2) {
        if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            Voice_SWB_TX_Init(NULL, &g_voice_ecnr_all_para->ScratchMemory,
                              &g_voice_ecnr_all_para->nvkey_aec, &g_voice_ecnr_all_para->nvkey_nr,
                              &g_voice_ecnr_all_para->nvkey_inear, &g_voice_ecnr_all_para->nvkey_aec_nr_swb,
                              &g_voice_ecnr_all_para->nvkey_tx_eq);
        } else {
            Voice_WB_TX_Init(is_nb_mode, NULL, &g_voice_ecnr_all_para->ScratchMemory,
                             &g_voice_ecnr_all_para->nvkey_aec, &g_voice_ecnr_all_para->nvkey_nr,
                             &g_voice_ecnr_all_para->nvkey_inear, &g_voice_ecnr_all_para->nvkey_tx_eq);
        }
    } else {
        if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            Voice_SWB_TX_Inear_Init_V2(NULL, &g_voice_ecnr_all_para->ScratchMemory,
                                       &g_voice_ecnr_all_para->nvkey_aec, &g_voice_ecnr_all_para->nvkey_nr,
                                       &g_voice_ecnr_all_para->nvkey_inear, &g_voice_ecnr_all_para->nvkey_aec_nr_swb,
                                       &g_voice_ecnr_all_para->nvkey_tx_eq, &g_voice_ecnr_all_para->nvkey_inear_eq,
                                       &g_voice_ecnr_all_para->nvkey_ast_eq);
        } else {
            Voice_WB_TX_Inear_Init_V2(is_nb_mode, NULL, &g_voice_ecnr_all_para->ScratchMemory,
                                      &g_voice_ecnr_all_para->nvkey_aec, &g_voice_ecnr_all_para->nvkey_nr,
                                      &g_voice_ecnr_all_para->nvkey_inear, &g_voice_ecnr_all_para->nvkey_tx_eq,
                                      &g_voice_ecnr_all_para->nvkey_inear_eq, &g_voice_ecnr_all_para->nvkey_ast_eq);
        }
    }

    g_voice_ecnr_nvkey_aec = &g_voice_ecnr_all_para->nvkey_aec;

    /* Init RX NR part */
    voice_rxnr_init(&g_voice_ecnr_all_para->ScratchMemory, &g_voice_ecnr_all_para->nvkey_nr,
                    &g_voice_ecnr_all_para->nvkey_aec_nr_swb, &g_voice_ecnr_all_para->nvkey_rx);

    g_voice_ecnr_all_para->common.init_flag = true;

#if defined(AIR_VOICE_NR_USE_PIC_ENABLE)
    Assign_EC_RXNR_scratch_memory(&g_voice_ecnr_all_para->ScratchMemory);/*EC IP with static lib for initial function*/
#endif

    /* Query the alg version log */
    #if defined(AIR_ECNR_1MIC_INEAR_ENABLE)
    DSP_MW_LOG_I("[DSP][VOICE_NR] Inhouse 1+1 version %d", 1, get_ECNR_SVN());//should compare with get_ECNR_Init_SVN()
    #elif defined(AIR_ECNR_2MIC_INEAR_ENABLE)
    DSP_MW_LOG_I("[DSP][VOICE_NR] Inhouse 2+1 version %d", 1, get_ECNR_SVN());
    #elif defined(AIR_ECNR_1_OR_2_MIC_ENABLE)
    DSP_MW_LOG_I("[DSP][VOICE_NR] Inhouse 1/2 version %d", 1, get_ECNR_SVN());
    #endif

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_all_deinit(void)
{
    voice_ecnr_common_deinit((voice_ecnr_common_para_t **)&g_voice_ecnr_all_para, DSP_ALG_ECNR_ALL_BUFFER_MAGIC_NUMBER);

    return VOICE_ECNR_STATUS_OK;
}

voice_ecnr_status_t voice_ecnr_all_process(void *para, uint32_t length, int16_t *main_mic, int16_t *ff_mic, int16_t *fb_mic, int16_t *ec_in, int16_t *out)
{
    UNUSED(para);
    UNUSED(length);

    if (g_voice_ecnr_mic_mode <= VOICE_ECNR_MIC_MODE_2) {
        if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            Voice_SWB_TX_Prcs(main_mic, ff_mic, ec_in, out, &gDspAlgParameter.AecNr);
        } else {
            Voice_WB_TX_Prcs(main_mic, ff_mic, ec_in, out, &gDspAlgParameter.AecNr);
        }
    } else {
        if (fb_mic == NULL) {
            fb_mic = ff_mic;
            ff_mic = NULL;
        }
        if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            Voice_SWB_TX_Inear_Prcs_V2(main_mic, ff_mic, fb_mic, ec_in, out, &gDspAlgParameter.AecNr);
        } else {
            Voice_WB_TX_Inear_Prcs_V2(main_mic, ff_mic, fb_mic, ec_in, out, &gDspAlgParameter.AecNr);
        }
    }
    return VOICE_ECNR_STATUS_OK;
}

uint32_t voice_ecnr_all_get_ref_gain(int16_t *gain_buf)
{
    uint32_t i, length;

    length = 0;
    if (voice_ecnr_all_check_running_status() == true) {
        /*
         * airoha 1/2 MIC: EC_REF_GAIN_READBACK (16 bytes, for 1MIC case, 0 for gain[4]~gain[7])
         * airoha 1+1 MIC: EC_REF_GAIN_READBACK (16 bytes, 0 for gain[4]~gain[7]), IE_EC_REF_GAIN_READBACK(8 bytes for +1)
         * airoha 2+1 MIC: EC_REF_GAIN_READBACK (16 bytes), IE_EC_REF_GAIN_READBACK(8 bytes for +1)
         *
         */
        EC_REF_GAIN_READBACK(gain_buf);
        length += 8;
        for (i = 0; i < 8; i++) {
            DSP_MW_LOG_I("[DSP][VOICE_NR] DSP get ref gain, GainAddr:0x%x", 1, gain_buf[i]);
        }
        if (g_voice_ecnr_mic_mode > VOICE_ECNR_MIC_MODE_2) {
            IE_EC_REF_GAIN_READBACK(gain_buf + length);
            for (i = 0; i < 4; i++) {
                DSP_MW_LOG_I("[DSP][VOICE_NR] DSP get inear ref gain, GainAddr:0x%x", 1, gain_buf[length + i]);
            }
            length += 4;
        }
        if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
            BAND5_EC_REF_GAIN_READBACK(gain_buf + length);
            length += 3;
        }
    }

    return length;
}

#endif

void voice_avc_set_enable(bool enable)
{
    UNUSED(enable);
    #ifdef AIR_3RD_PARTY_NR_ENABLE
        if(g_voice_ecnr_ec_postec_para){
            g_voice_ecnr_ec_postec_para->nvkey_nr.SW_BYPASS_AVC = enable;
        }
    #endif

    #if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_ECNR_1_OR_2_MIC_ENABLE)
        if(g_voice_ecnr_all_para){
            g_voice_ecnr_all_para->nvkey_nr.SW_BYPASS_AVC = enable;
        }
    #endif
    DSP_MW_LOG_I("[DSP][VOICE_NR] [AVC] enable: %d", 1, enable);
}

bool voice_avc_check_enable(void)
{
    #ifdef AIR_3RD_PARTY_NR_ENABLE
        return g_voice_ecnr_ec_postec_para->nvkey_nr.SW_BYPASS_AVC;
    #endif

    #if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_ECNR_1_OR_2_MIC_ENABLE)
        return g_voice_ecnr_all_para->nvkey_nr.SW_BYPASS_AVC;
    #endif

    return false;
}
