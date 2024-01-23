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

#ifdef AIR_BT_CODEC_BLE_V2_ENABLED

/* Includes ------------------------------------------------------------------*/
#include "assert.h"
#include "dsp_feature_interface.h"
#include "dsp_audio_process.h"
#include "dsp_dump.h"
#include "sink_inter.h"
#include "source_inter.h"
#include "memory_attribute.h"
#include "preloader_pisplit.h"
#include "dsp_rom_table.h"
#include "lc3_enc_branch_interface.h"
#include "lc3_dec_interface_v2.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
lc3_enc_branch_para_t lc3_enc_branch_para[LC3_ENC_BRANCH_INDEX_MAX];
extern U8 g_lc3_v2_user_cnt;
extern void *p_lc3i_tab_common;
static void *p_lc3i_tab_enc = NULL;
extern Multi_FFT lc3_FFTx;

/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void stream_codec_encoder_lc3_branch_init(lc3_enc_branch_index_t index, void *user, lc3_enc_branch_config_t *config)
{
    uint32_t saved_mask;
    int32_t i;
    void **p_user = NULL;

    if (index >= LC3_ENC_BRANCH_INDEX_MAX) {
        AUDIO_ASSERT(0);
    }

    if ((config->frame_interval/100) == LC3_INTERVAL_10_MS)
    {
        lc3_FFTx.fix_fft_init = fix_fft_Init_10MS;
        lc3_FFTx.fix_fft10 = fix_fft10_10_MS;
        lc3_FFTx.fix_fft15 = fix_fft15_10_MS;
        lc3_FFTx.fix_fft20 = fix_fft20_10_MS;
        lc3_FFTx.fix_fft30 = fix_fft30_10_MS;
        lc3_FFTx.fix_fft40 = fix_fft40_10_MS;
        lc3_FFTx.FFT8N = FFT8N_10_MS;
    } else {
        lc3_FFTx.fix_fft_init = fix_fft_Init_7P5_MS;
        lc3_FFTx.fix_fft15 = fix_fft15_7P5_MS;
        lc3_FFTx.fix_fft30 = fix_fft30_7P5_MS;
        lc3_FFTx.fix_fft40 = fix_fft40_7P5_MS;
        lc3_FFTx.FFT4N = FFT4N_7P5_MS;
        lc3_FFTx.FFT8N = FFT8N_7P5_MS;
        lc3_FFTx.FFT12N = FFT12N_7P5_MS; // FFT6N and FFT12N
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (lc3_enc_branch_para[index].enc_count == 0) {
        /* update status */
        lc3_enc_branch_para[index].enc_status = LC3_ENC_BRANCH_STATUS_INIT;

        /* config lc3 codec */
        lc3_enc_branch_para[index].sample_bits = config->sample_bits;
        lc3_enc_branch_para[index].channel_no = config->channel_no;
        lc3_enc_branch_para[index].sample_rate = config->sample_rate;
        lc3_enc_branch_para[index].bit_rate = config->bit_rate;
        lc3_enc_branch_para[index].frame_interval = config->frame_interval;
        lc3_enc_branch_para[index].delay = config->delay;

        /* get remain samples memory */
        lc3_enc_branch_para[index].remain_buffer_size = config->buffer_size;
    }

    /* increase status count */
    lc3_enc_branch_para[index].enc_count += 1;
    if (lc3_enc_branch_para[index].enc_count == 0) {
        AUDIO_ASSERT(0);
    }

    /* register this user into this index */
    for (i = LC3_ENC_BRANCH_USER_COUNT - 1; i >= 0; i--) {
        if (lc3_enc_branch_para[index].user[i] == user) {
            p_user = &(lc3_enc_branch_para[index].user[i]);
            break;
        } else if (lc3_enc_branch_para[index].user[i] == NULL) {
            p_user = &(lc3_enc_branch_para[index].user[i]);
        }
    }
    if (p_user != NULL) {
        *p_user = user;
    } else {
        AUDIO_ASSERT(0);
    }
    g_lc3_v2_user_cnt += 1;

    hal_nvic_restore_interrupt_mask(saved_mask);

    return;
}

void stream_codec_encoder_lc3_branch_deinit(lc3_enc_branch_index_t index, void *user)
{
    uint32_t saved_mask;
    int32_t i;
    bool mem_flag = false;
    void *WorkMemPtr = NULL;
    void *RemainMemPtr = NULL;
    void **p_user = NULL;

    if (index >= LC3_ENC_BRANCH_INDEX_MAX) {
        AUDIO_ASSERT(0);
    }

#ifdef PRELOADER_ENABLE
    if ((lc3_enc_branch_para[index].frame_interval/100) == LC3_INTERVAL_10_MS) {
        lc3i_fft10ms_library_unload();
    } else if ((lc3_enc_branch_para[index].frame_interval/100) == LC3_INTERVAL_7P5_MS){
        lc3i_fft7_5ms_library_unload();
    } else {
        DSP_MW_LOG_E("[LC3_BRANCH_ENC] unsupported DL frame interval %d", 1, lc3_enc_branch_para[index].frame_interval);
    }
#endif

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    /* decrease status count */
    if (lc3_enc_branch_para[index].enc_count == 0) {
        AUDIO_ASSERT(0);
    }
    lc3_enc_branch_para[index].enc_count -= 1;

    if (lc3_enc_branch_para[index].enc_count == 0) {
        /* update status */
        lc3_enc_branch_para[index].enc_status = LC3_ENC_BRANCH_STATUS_DEINIT;

        /* reset lc3 codec */
        lc3_enc_branch_para[index].sample_bits = 0;
        lc3_enc_branch_para[index].channel_no = 0;
        lc3_enc_branch_para[index].sample_rate = 0;
        lc3_enc_branch_para[index].bit_rate = 0;
        lc3_enc_branch_para[index].frame_interval = 0;
        lc3_enc_branch_para[index].delay = 0;
        lc3_enc_branch_para[index].remain_buffer_size = 0;
        WorkMemPtr = lc3_enc_branch_para[index].work_mem_ptr;
        lc3_enc_branch_para[index].work_mem_ptr = NULL;
        RemainMemPtr = lc3_enc_branch_para[index].remain_mem_ptr;
        lc3_enc_branch_para[index].remain_mem_ptr = NULL;

        /* set memory operation flag */
        mem_flag = true;
    }

    /* unregister this user into this index */
    for (i = LC3_ENC_BRANCH_USER_COUNT - 1; i >= 0; i--) {
        if (lc3_enc_branch_para[index].user[i] == user) {
            p_user = &(lc3_enc_branch_para[index].user[i]);
            break;
        }
    }
    if (p_user != NULL) {
        *p_user = NULL;
    } else {
        AUDIO_ASSERT(0);
    }
    g_lc3_v2_user_cnt -= 1;

    hal_nvic_restore_interrupt_mask(saved_mask);

    if (mem_flag) {
        /* free working memory */
        preloader_pisplit_free_memory(WorkMemPtr);

        /* free remain samples memory */
        preloader_pisplit_free_memory(RemainMemPtr);

        if (p_lc3i_tab_enc){
            /*table dec*/
            preloader_pisplit_free_memory(p_lc3i_tab_enc);
            p_lc3i_tab_enc = NULL;
        }
    }

    if (g_lc3_v2_user_cnt == 0) {
        /*Table common memory*/
        preloader_pisplit_free_memory(p_lc3i_tab_common);
        p_lc3i_tab_common = NULL;
    }

    return;
}

void stream_codec_encoder_lc3_branch_get_data_info(lc3_enc_branch_index_t index, uint32_t channel, uint8_t **in_buffer_address, uint32_t *in_frame_size)
{
    lc3_enc_branch_para_t *lc3_para = NULL;

    if (index >= LC3_ENC_BRANCH_INDEX_MAX) {
        assert(0);
    }

    lc3_para = &lc3_enc_branch_para[index];

    if ((channel > lc3_para->channel_no) || (channel == 0)) {
        assert(0);
    }

    *in_buffer_address = (uint8_t *)((uint32_t)lc3_para->remain_mem_ptr + (channel - 1) * lc3_para->remain_buffer_size);
    *in_frame_size    = lc3_para->in_frame_size;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ bool stream_codec_encoder_lc3_branch_initialize(void *para)
{
    uint32_t i, j;
    uint32_t saved_mask;
    lc3_enc_branch_para_t *lc3_para = NULL;
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    LC3_ERR_T ret;
    void *p_lc3i_enc = NULL;
    LC3I_Param param;
    uint32_t lc3i_tab_enc_size = 0;
    uint32_t lc3i_tab_common_size = 0;
    uint32_t lc3i_enc_size = 0;

    /* find out which index is belong to this stream */
    for (i = 0; i < LC3_ENC_BRANCH_INDEX_MAX; i++) {
        for (j = 0; j < LC3_ENC_BRANCH_USER_COUNT; j++) {
            /* Check if this source or sink has already be regitsered into this index */
            if ((lc3_enc_branch_para[i].user[j] == stream_ptr->source) ||
                (lc3_enc_branch_para[i].user[j] == stream_ptr->sink)) {
                lc3_para = &lc3_enc_branch_para[i];
                goto FIND_ENCODER_INDEX;
            }
        }
    }
    if (lc3_para == NULL) {
        AUDIO_ASSERT(0);
    }
FIND_ENCODER_INDEX:

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    /* status check */
    if (lc3_para->enc_status == LC3_ENC_BRANCH_STATUS_INIT) {
        lc3_para->enc_status = LC3_ENC_BRANCH_STATUS_RUNNING;
    } else if (lc3_para->enc_status == LC3_ENC_BRANCH_STATUS_RUNNING) {
        hal_nvic_restore_interrupt_mask(saved_mask);
        return false;
    } else {
        DSP_MW_LOG_I("[LC3_BRANCH_ENC] error status:%d", 1, lc3_para->enc_status);
        AUDIO_ASSERT(FALSE);
        return true;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

#ifdef ROM_TABLE_ADDR_LC3
    LC3I_Set_ROM_Start(ROM_TABLE_ADDR_LC3);
#endif

    /* Because celt API maybe is PIC, so we need to get working memory here for safety */
    lc3_para->remain_mem_ptr = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, lc3_para->remain_buffer_size * lc3_para->channel_no);
    if (lc3_para->remain_mem_ptr == NULL) {
        AUDIO_ASSERT(0);
    }

    if (p_lc3i_tab_common == NULL) {
        /*Table Common*/
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        lc3i_tab_common_size = LC3PLUSN_Tab_Common_Get_MemSize();
#else
        lc3i_tab_common_size = LC3I_Tab_Common_Get_MemSize();
#endif
        lc3i_tab_common_size = (lc3i_tab_common_size + 7) / 8 * 8;
        p_lc3i_tab_common = (void*) preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE , lc3i_tab_common_size);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        ret = LC3PLUSN_Tab_Common_Init(p_lc3i_tab_common);
#else
        ret = LC3I_Tab_Common_Init(p_lc3i_tab_common);
#endif
        if (ret != LC3_OK) {
            DSP_MW_LOG_E("[lc3][fail] LC3I_Tab_Common_Init() %d", 1, ret);
            return true;
        }
        DSP_MW_LOG_I("[lc3][enc] LC3I_Tab_Common_Init() Done 0x%08x", 1, p_lc3i_tab_common);
    }

    param.frame_ms = lc3_para->frame_interval/100;
    param.sr = lc3_para->sample_rate;
    /*Table Enc*/
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
    lc3i_tab_enc_size = LC3PLUSN_Tab_Enc_Get_MemSize(&param);
#else
    lc3i_tab_enc_size = LC3I_Tab_Enc_Get_MemSize(&param);
#endif
    lc3i_tab_enc_size = (lc3i_tab_enc_size + 7) / 8 * 8;
    p_lc3i_tab_enc = (void*) preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE , lc3i_tab_enc_size);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
    ret = LC3PLUSN_Tab_Enc_Init(p_lc3i_tab_common, p_lc3i_tab_enc, &param);
#else
    ret = LC3I_Tab_Enc_Init(p_lc3i_tab_common, p_lc3i_tab_enc, &param);
#endif
    if (ret != LC3_OK) {
        DSP_MW_LOG_E("[lc3][fail] LC3I_Tab_Enc_Init() %d", 1, ret);
        return true;
    }
    DSP_MW_LOG_I("[lc3][enc] LC3I_Tab_Enc_Init() Done",1, p_lc3i_tab_enc);

    /*Decoder Working Buffer*/
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
    lc3i_enc_size = LC3PLUSN_Enc_Get_MemSize(1, lc3_para->sample_rate, lc3_para->frame_interval/100);
#else
    lc3i_enc_size = LC3I_Enc_Get_MemSize(1, lc3_para->sample_rate, lc3_para->frame_interval/100);
#endif
    lc3_para->work_buffer_size = (lc3i_enc_size + 7) / 8 * 8;
    lc3_para->work_mem_ptr = (void*) preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE , lc3_para->work_buffer_size * lc3_para->channel_no);
    if (lc3_para->work_mem_ptr == NULL) {
        AUDIO_ASSERT(0);
        return true;
    }

    if (stream_codec_get_input_resolution(para) != RESOLUTION_16BIT) {
        DSP_MW_LOG_I("[lc3][enc][branch] current LC3 encoder only support 16 bit resolution: (%d)", 1, stream_codec_get_input_resolution(para));
        AUDIO_ASSERT(0);
        return false;
    }

    /* do encoder init one by one channel */
    for (i = 0; i < lc3_para->channel_no; i++) {
        p_lc3i_enc = (void *)((uint8_t *)lc3_para->work_mem_ptr + i * lc3_para->work_buffer_size);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        ret = LC3PLUSN_Enc_Init(p_lc3i_enc, lc3_para->sample_bits, lc3_para->sample_rate, 1, lc3_para->bit_rate, lc3_para->frame_interval / 100, lc3_para->delay, 0);
#else
        ret = LC3I_Enc_Init(p_lc3i_enc, lc3_para->sample_bits, lc3_para->sample_rate, 1, lc3_para->bit_rate, lc3_para->frame_interval / 100, lc3_para->delay, 0);
#endif
        if (ret != LC3_OK) {
            DSP_MW_LOG_E("[lc3][enc][branch] init fail %d", 1, ret);
            AUDIO_ASSERT(0);
            return true;
        }
    }

#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
    DSP_MW_LOG_I("[lc3][enc][branch] init done, version = 0x%x", 1, LC3I_Get_Version());
#endif

    return false;
}

ATTR_TEXT_IN_IRAM bool stream_codec_encoder_lc3_branch_process(void *para)
{
    uint32_t i, j;
    lc3_enc_branch_para_t *lc3_para = NULL;
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    int16_t *in_buf;
    int16_t *out_buf;
    uint32_t in_frame_size;
    int16_t *remain_mem;
    void *p_lc3i_enc;
    uint32_t out_size;
    LC3_ERR_T ret;

    /* find out which index is belong to this stream */
    for (i = 0; i < LC3_ENC_BRANCH_INDEX_MAX; i++) {
        for (j = 0; j < LC3_ENC_BRANCH_USER_COUNT; j++) {
            /* Check if this source or sink has already be regitsered into this index */
            if ((lc3_enc_branch_para[i].user[j] == stream_ptr->source) ||
                (lc3_enc_branch_para[i].user[j] == stream_ptr->sink)) {
                lc3_para = &lc3_enc_branch_para[i];
                goto FIND_ENCODER_INDEX;
            }
        }
    }
    if (lc3_para == NULL) {
        AUDIO_ASSERT(0);
    }
FIND_ENCODER_INDEX:

    in_frame_size = stream_codec_get_input_size(para);
    lc3_para->in_frame_size = in_frame_size;
    if (in_frame_size == 0) {
        /* config output size */
        stream_codec_modify_output_size(para, in_frame_size);
        lc3_para->out_frame_size = in_frame_size;

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&lc3_para->finish_gpt_count);

        return false;
    }

    /* do encoder one by one channel */
    for (i = 0; i < lc3_para->channel_no; i++) {
        in_buf = (int16_t *)stream_codec_get_input_buffer(para, i + 1);
        out_buf = (int16_t *)stream_codec_get_output_buffer(para, i + 1);
        remain_mem = (int16_t *)((uint32_t)lc3_para->remain_mem_ptr + i * lc3_para->remain_buffer_size);
        p_lc3i_enc = (void *)((uint8_t *)lc3_para->work_mem_ptr + i * lc3_para->work_buffer_size);

        /* copy data from stream buffer to remain buffer */
        if (in_frame_size > lc3_para->remain_buffer_size) {
            AUDIO_ASSERT(0);
        }
        for (j = 0; j < in_frame_size / sizeof(int16_t); j++) {
            *(remain_mem + j) = *(in_buf + j);
        }

        /* do encode and output to the stream buffer */
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        ret = LC3PLUSN_Enc_Prcs(p_lc3i_enc, p_lc3i_tab_common, (uint8_t *)out_buf, (uint8_t *)remain_mem, &out_size, &lc3_FFTx);
#else
        ret = LC3I_Enc_Prcs(p_lc3i_enc, p_lc3i_tab_common, (uint8_t *)out_buf, (uint8_t *)remain_mem, &out_size, &lc3_FFTx);
#endif
        if (ret != LC3_OK) {
            DSP_MW_LOG_E("[lc3][enc][branch] process fail %d", 1, ret);
            AUDIO_ASSERT(0);
            return true;
        }
    }

    /* config output size */
    stream_codec_modify_output_size(para, out_size);
    lc3_para->out_frame_size = out_size;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&lc3_para->finish_gpt_count);

    return false;
}

#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */
