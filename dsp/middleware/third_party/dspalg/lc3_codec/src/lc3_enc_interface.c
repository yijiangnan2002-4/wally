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

#include "assert.h"
#include "dsp_feature_interface.h"
#include "dsp_audio_process.h"
#include "dsp_dump.h"
#include "sink_inter.h"
#include "source_inter.h"
#include "src_fixed_ratio_interface.h"
#include "preloader_pisplit.h"
#include "dsp_rom_table.h"

#include "bt_types.h"
#include "preloader_pisplit.h"

#ifdef AIR_BT_CODEC_BLE_ENABLED

#include "lc3_enc_interface.h"
#include "lc3_dec_interface.h"

#ifdef MTK_BT_A2DP_LC3_USE_PIC
#include "lc3_codec_portable.h"
extern SemaphoreHandle_t g_lc3i_codec_process_xSemaphore; // for lc3i process only
#endif

//#define LC3_ENCODE_DUMP_ENABLE

extern Multi_FFT lc3_FFTx;
extern LC3I_PTR lc3_MEM_PTR[C_LC3I_MAX_NUM];
extern LC3_INSTANCE_PTR lc3_memory;
extern void *p_lc3i_tab;
extern void *p_lc3i_tab_common;
static void *p_lc3i_tab_enc = NULL;
static void *p_lc3i_enc = NULL;
extern U8 g_lc3_user_cnt;
extern ENCDEC lc3_param_encdec[2];

//static const uint32_t g_lc3_pcm_frame_samples[] = {80, 160, 240, 320, 480, 480};

//static uint32_t sample_rate_translate(stream_samplerate_t sample_rate)
//{
//    if (sample_rate == FS_RATE_8K) {
//        return 0;
//    } else if (sample_rate == FS_RATE_16K) {
//        return 1;
//    } else if (sample_rate == FS_RATE_24K) {
//        return 2;
//    } else if (sample_rate == FS_RATE_32K) {
//        return 3;
//    } else if (sample_rate == FS_RATE_44_1K) {
//        return 4;
//    } else if (sample_rate == FS_RATE_48K) {
//        return 5;
//    } else {
//        AUDIO_ASSERT(0);
//    }
//}

void LC3I_Set_Enc_Param(U16 Channel, U16 frame_interval,U32 sample_rate, U32 bit_rate)
{
    g_lc3_user_cnt += 1;
    DSP_MW_LOG_I("[BLE][UL] g_lc3_user_cnt++ %d", 1, g_lc3_user_cnt);

    lc3_param_encdec[1].alg = C_Alg_LC3;
    lc3_param_encdec[1].mode = C_LC3I_Enc;
    lc3_param_encdec[1].plcmeth = 0;
    lc3_param_encdec[1].frame_ms = frame_interval;
    lc3_param_encdec[1].ch = Channel;
    lc3_param_encdec[1].sr = sample_rate;
    lc3_param_encdec[1].bps = 16;
    lc3_param_encdec[1].bitrate = bit_rate;
    lc3_param_encdec[1].delay = LC3_NO_DELAY_COMPENSATION;
    lc3_param_encdec[1].lfe= 0;

    if(frame_interval == LC3_INTERVAL_10_MS) {
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
        lc3_FFTx.FFT12N = FFT12N_7P5_MS;// FFT6N and FFT12N
    }

    DSP_MW_LOG_I("[lc3][enc][init_config] plcmeth %d, frame_ms %d, ch %d, sample_rate %d",4,lc3_param_encdec[1].plcmeth,lc3_param_encdec[1].frame_ms,lc3_param_encdec[1].ch,lc3_param_encdec[1].sr);
    DSP_MW_LOG_I("[lc3][enc][init_config] resolution %d, bitrate %d, delay %d, lfe %d",4,lc3_param_encdec[1].bps,lc3_param_encdec[1].bitrate,lc3_param_encdec[1].delay,lc3_param_encdec[1].lfe);

}


static uint16_t lc3_enc_get_frame_interval_from_source(void *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    uint16_t interval_in_100us;

    /* unit of stream_ptr->source->param.audio.period is 1ms */
    /* unit of frame interval needed by LC3 API is 100us */

    interval_in_100us = stream_ptr->source->param.audio.period * 10;

    if (interval_in_100us != 100 && interval_in_100us != 150) {
        interval_in_100us = 75;    /* 7.5ms would be truncated in audio.period representation format*/
    }

    return interval_in_100us;       /* return value should be either 75/100/150*/
}

static uint16_t lc3_enc_get_frame_interval_from_sink(void *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);

    /* unit of stream_ptr->sink->param.n9ble.frame_interval is 1us */
    /* unit of frame interval needed by LC3 API is 100us */
    return stream_ptr->sink->param.n9ble.frame_interval / 100;
}

static uint16_t lc3_enc_get_sample_rate(void *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);

    return stream_ptr->sink->param.n9ble.sampling_rate;
}

extern bool ULL_NrOffloadFlag;

static uint16_t lc3_enc_get_frame_length(void *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);

    if (ULL_NrOffloadFlag == true) {
        return stream_ptr->sink->param.n9ble.frame_length - 1;
    } else {
        return stream_ptr->sink->param.n9ble.frame_length;
    }
}

static void lc3_enc_set_sink_process_num(void *para, uint16_t num)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);

    stream_ptr->sink->param.n9ble.process_number = num;
}

static uint16_t lc3_enc_get_sink_dummy_insert_num(void *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);

    return stream_ptr->sink->param.n9ble.dummy_insert_number;
}

static void lc3_enc_set_sink_dummy_insert_num(void *para, uint16_t num)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);

    stream_ptr->sink->param.n9ble.dummy_insert_number = num;
}

static void lc3_enc_set_sink_dummy_process_status(void *para, uint16_t num)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);

    stream_ptr->sink->param.n9ble.dummy_process_status = num;
}

bool STREAM_BLE_UL_CHECK_MODE(void);
bool STREAM_BLE_UL_GET_PARAM(uint32_t *sample_rate, uint32_t *bitrate);
bool STREAM_BLE_UL_CHECK_BUF(uint8_t index);
uint32_t STREAM_BLE_UL_GET_BUF_INDEX(void);
void STREAM_BLE_UL_WRITE_BUF(uint8_t index, uint8_t *buf, uint32_t dst_buf_len, uint32_t buffer_index);

static bool lc3_enc_src_enabled;
static bool lc3_enc_sw_buf_enabled;

static uint8_t *lc3_enc_sw_buf;
static uint8_t  lc3_enc_sw_buf_index;
static uint16_t lc3_enc_sw_buf_block_len;
static uint16_t lc3_enc_sw_buf_remain_len;
static uint16_t lc3_enc_sw_buf_consume_len;


/*static ALIGN(8) dsp_updn_sample_interface_t lc3_enc_upsample_para = {
    .NvKey = {
        0x1FCF,0x2752,0x4306,0xCD63,0x2752,0x2752,0x08F8,0x2752,
        0x2839,0xA49F,0x00CE,0x2752,0x19C3,0x893E,0x2752,0x0000,
    },
};*/
extern int16_t dsp_sampler_coef[32];
void* lc3_enc_smp_instance_ptr;

BOOL LC3_Enc_MemCheck (VOID)
{
    BOOL ret = FALSE;
    if ((NULL != lc3_memory) && (LC3_DEC_VALID_MEMORY_CHECK_VALUE == lc3_memory->lc3_enc_memory.MemoryCheck))
    {
        ret = TRUE;
    }
    return ret;
}

VOID LC3_Tab_Enc_MemInit (VOID* para,LC3I_Param *param){
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    uint32_t lc3i_tab_enc_size = 0;
    uint32_t lc3i_tab_common_size = 0;
    uint32_t lc3i_enc_size = 0;

    LC3_ERR_T ret;
    DSP_MW_LOG_I("[lc3][enc] LC3_Tab_Enc_MemInit", 0);

    if (!LC3_Enc_MemCheck())
    {
#ifdef ROM_TABLE_ADDR_LC3
        LC3I_Set_ROM_Start(ROM_TABLE_ADDR_LC3);
#endif
        if (p_lc3i_tab_common == NULL && lc3_memory == NULL) {
            lc3_memory = (LC3_INSTANCE_PTR) preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE , sizeof(LC3_INSTANCE));
            /*Table Common*/
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
            lc3i_tab_common_size = LC3PLUSN_Tab_Common_Get_MemSize();
#else
            lc3i_tab_common_size = LC3I_Tab_Common_Get_MemSize();
#endif
            lc3i_tab_common_size = (lc3i_tab_common_size + 7) / 8 * 8;
            p_lc3i_tab_common = (void*) preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE , lc3i_tab_common_size);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
            ret = LC3PLUSN_Tab_Common_Init(p_lc3i_tab_common);
#else
            ret = LC3I_Tab_Common_Init(p_lc3i_tab_common);
#endif
            if (ret != LC3_OK) {
                DSP_MW_LOG_E("[lc3][fail] LC3I_Tab_Common_Init() %d", 1, ret);
                return;
            }
            DSP_MW_LOG_I("[lc3][enc][dec] LC3I_Tab_Common_Init() Done 0x%08x", 1, p_lc3i_tab_common);
        }

        /*Table Dec*/
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        lc3i_tab_enc_size = LC3PLUSN_Tab_Enc_Get_MemSize(param);
#else
        lc3i_tab_enc_size = LC3I_Tab_Enc_Get_MemSize(param);
#endif
        lc3i_tab_enc_size = (lc3i_tab_enc_size + 7) / 8 * 8;
        p_lc3i_tab_enc = (void*) preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE , lc3i_tab_enc_size);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        ret = LC3PLUSN_Tab_Enc_Init(p_lc3i_tab_common, p_lc3i_tab_enc, param);
#else
        ret = LC3I_Tab_Enc_Init(p_lc3i_tab_common, p_lc3i_tab_enc, param);
#endif
        if (ret != LC3_OK) {
            DSP_MW_LOG_E("[lc3][fail] LC3I_Tab_Enc_Init() %d", 1, ret);
            return;
        }
        DSP_MW_LOG_I("[lc3][dec] LC3I_Tab_Enc_Init() Done",1, p_lc3i_tab_enc);

        /*Decoder Working Buffer*/
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        lc3i_enc_size = LC3PLUSN_Enc_Get_MemSize(lc3_param_encdec[1].ch, lc3_param_encdec[1].sr, lc3_param_encdec[1].frame_ms);
#else
        lc3i_enc_size = LC3I_Enc_Get_MemSize(lc3_param_encdec[1].ch, lc3_param_encdec[1].sr, lc3_param_encdec[1].frame_ms);
#endif
        lc3i_enc_size = (lc3i_enc_size + 7) / 8 * 8;
        p_lc3i_enc = (void*) preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE , lc3i_enc_size);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        ret = LC3PLUSN_Enc_Init(p_lc3i_enc, lc3_param_encdec[1].bps, lc3_param_encdec[1].sr, lc3_param_encdec[1].ch, lc3_param_encdec[1].bitrate, lc3_param_encdec[1].frame_ms, lc3_param_encdec[1].delay, lc3_param_encdec[1].lfe);
#else
        ret = LC3I_Enc_Init(p_lc3i_enc, lc3_param_encdec[1].bps, lc3_param_encdec[1].sr, lc3_param_encdec[1].ch, lc3_param_encdec[1].bitrate, lc3_param_encdec[1].frame_ms, lc3_param_encdec[1].delay, lc3_param_encdec[1].lfe);
#endif
        if (ret != LC3_OK) {
            DSP_MW_LOG_E("[lc3][fail] LC3I_Enc_Init() %d", 1, ret);
            return;
        }
        DSP_MW_LOG_I("[lc3][dec] LC3I_Enc_Init() Done",0);

        lc3_memory->lc3_enc_memory.MemoryCheck = LC3_DEC_VALID_MEMORY_CHECK_VALUE;
        lc3_memory->lc3_enc_memory.InitDone = true;
    }
}

void LC3_Enc_Deinit(void){
    U16 frame_interval = lc3_param_encdec[1].frame_ms;
    lc3_param_encdec[1].alg        = 0;
    lc3_param_encdec[1].mode       = 0;
    lc3_param_encdec[1].plcmeth    = 0;
    lc3_param_encdec[1].frame_ms   = 0;
    lc3_param_encdec[1].ch         = 0;
    lc3_param_encdec[1].sr         = 0;
    lc3_param_encdec[1].bps        = 0;
    lc3_param_encdec[1].bitrate    = 0;
    lc3_param_encdec[1].delay      = 0;
    lc3_param_encdec[1].lfe        = 0;
#ifdef PRELOADER_ENABLE
    if (frame_interval == LC3_INTERVAL_10_MS) {
        lc3i_fft10ms_library_unload();
    } else if (frame_interval == LC3_INTERVAL_7P5_MS){
        lc3i_fft7_5ms_library_unload();
    } else {
        DSP_MW_LOG_E("[BLE] unsupported UL frame interval %d", 1, frame_interval);
    }
#endif
    UNUSED(frame_interval);
    /*Working memory*/
    preloader_pisplit_free_memory(p_lc3i_enc);
    p_lc3i_enc = NULL;
    /*Table enc*/
    preloader_pisplit_free_memory(p_lc3i_tab_enc);
    p_lc3i_tab_enc = NULL;

    lc3_memory->lc3_enc_memory.InitDone = FALSE;
    lc3_memory->lc3_enc_memory.MemoryCheck = 0;

    g_lc3_user_cnt -= 1;
    if (g_lc3_user_cnt == 0) {
        /*Table common memory*/
        preloader_pisplit_free_memory(p_lc3i_tab_common);
        p_lc3i_tab_common = NULL;
        preloader_pisplit_free_memory(lc3_memory);
        lc3_memory = NULL;
    }
    DSP_MW_LOG_I("[BLE][UL] g_lc3_user_cnt-- %d", 1, g_lc3_user_cnt);
}

bool stream_codec_encoder_lc3_initialize (void *para)
{
    uint32_t bit_rate, sample_rate;
    #ifndef AIR_BLE_FIXED_RATIO_SRC_ENABLE
    int32_t src_quality_mode = 0;
    uint32_t instance_size = get_updn_samp_memsize(src_quality_mode);
    #endif
    uint16_t pcm_resolution;
    uint16_t frame_interval, frame_length;

    if (stream_codec_get_input_resolution(para) == RESOLUTION_16BIT) {
        pcm_resolution = 16;
    } else {
        DSP_MW_LOG_I("[lc3][enc] current LC3 encoder only support 16 bit resolution: (%d)", 1, stream_codec_get_input_resolution(para));
        return false;
    }

    if (STREAM_BLE_UL_GET_PARAM(&sample_rate, &bit_rate)) {
        /*  is le audio source application, get frame interval from stream source (afe)*/
        frame_interval = lc3_enc_get_frame_interval_from_source(para);
        DSP_MW_LOG_I("[lc3][enc] get encode parameter from stream", 0);
    } else {
        stream_samplerate_t sample_rate_in  = stream_codec_get_input_samplingrate(para);
        UNUSED(sample_rate_in);

        frame_interval = lc3_enc_get_frame_interval_from_sink(para);
        sample_rate = lc3_enc_get_sample_rate(para);
        frame_length = lc3_enc_get_frame_length(para);

        DSP_MW_LOG_I("[lc3][enc] sample rate in: %d out: %d", 2, sample_rate_in, sample_rate);

#ifndef AIR_BLE_FIXED_RATIO_SRC_ENABLE
#ifdef AIR_BT_BLE_SWB_ENABLE
        DSP_MW_LOG_I("AIR_BLE_FIXED_RATIO_SRC_ENABLE", 0);
        if (sample_rate_in == FS_RATE_32K && sample_rate == 16000) {
#else
        if (sample_rate_in == FS_RATE_16K && sample_rate == 32000) {
#endif
            lc3_enc_src_enabled = true;
            if (lc3_enc_smp_instance_ptr) {
                preloader_pisplit_free_memory(lc3_enc_smp_instance_ptr);
            }
            lc3_enc_smp_instance_ptr = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, instance_size);
            if (lc3_enc_smp_instance_ptr == NULL) {
                DSP_MW_LOG_E("[lc3][enc][init] malloc fail!", 0);
                AUDIO_ASSERT(0);
            }
            updn_samp_init(lc3_enc_smp_instance_ptr, (void*)&dsp_sampler_coef, src_quality_mode);
        } else {
            lc3_enc_src_enabled = false;
        }
#else
        lc3_enc_src_enabled = false;
#endif

        if (lc3_enc_get_frame_interval_from_source(para) == 150) {
            lc3_enc_sw_buf_enabled = true;
            lc3_enc_sw_buf_index = 1;
            lc3_enc_sw_buf_block_len = (sample_rate/100)*3;                  /* either 32K or 16K sample rate*/
            lc3_enc_sw_buf_remain_len  = (frame_interval == LC3_INTERVAL_10_MS) ?  (sample_rate/100) : 0;      /* dummy 5ms silence for 10ms interval case*/
            lc3_enc_sw_buf_consume_len = (frame_interval == LC3_INTERVAL_10_MS) ?  ((sample_rate/100)*2) : (lc3_enc_sw_buf_block_len/2);
            lc3_enc_sw_buf = &lc3_memory->lc3_enc_memory.g_lc3_cache_buffer[0];
            memset(lc3_enc_sw_buf, 0, 2000);
        } else {
            lc3_enc_sw_buf_enabled = false;
        }

        if (lc3_enc_src_enabled && !lc3_enc_sw_buf_enabled) {
            DSP_MW_LOG_E("[lc3][enc][fail] something wrong. should not enable src only\r\n", 0);
            AUDIO_ASSERT(0);
        }
        bit_rate = (frame_length * 8) * (1000 * 10) / (frame_interval);
    }

    LC3I_Param lc3_param;
    lc3_param.frame_ms = frame_interval;
    lc3_param.sr       = sample_rate;
    LC3_Tab_Enc_MemInit(para, &lc3_param);
    lc3_memory->lc3_enc_memory.InitDone = TRUE;

    DSP_MW_LOG_I("[lc3][enc] stream_codec_encoder_lc3_initialize() exit", 0);

    return false;
}

bool stream_codec_encoder_lc3_process(void *para)
{
    LC3_ERR_T ret = LC3_OK;
    uint8_t *input_buffer = stream_codec_get_1st_input_buffer(para);
    uint8_t *out_buffer = stream_codec_get_1st_output_buffer(para);
    uint32_t out_size = 0;

    if (lc3_memory->lc3_enc_memory.InitDone == FALSE) {
        DSP_MW_LOG_E("[lc3][enc][fail] stream_codec_encoder_lc3_initialize is not called\r\n", 0);
        AUDIO_ASSERT(0);
    }

    lc3_enc_sw_buf = &lc3_memory->lc3_enc_memory.g_lc3_cache_buffer[0];//workaround

    /* check if software buffer or up sampling mechanism is needed*/
    if (lc3_enc_sw_buf_enabled) {
        uint8_t *temp_buf = lc3_enc_sw_buf + lc3_enc_sw_buf_index * lc3_enc_sw_buf_block_len;
        uint32_t this_out_size = 0;
        uint16_t process_num = 0;
        uint16_t dummy_insert_num = 0;
        lc3_enc_sw_buf_index ^= 1;
        //LOG_AUDIO_DUMP(input_buffer, in_frame_size, AUDIO_INS_OUT_L);

        if (lc3_enc_src_enabled) {
            stream_samplerate_t sample_rate_in  = stream_codec_get_input_samplingrate(para);
            uint32_t sample_rate = lc3_enc_get_sample_rate(para);

            if (sample_rate_in == FS_RATE_16K && sample_rate == 32000) { /*32000 -> 16000*/
                //updn_sampling_by2_Proc(&lc3_enc_upsample_para, (int16_t*)input_buffer, (int16_t*)temp_buf, 240, 1, true);       /*up by 2*/
                updn_samp_prcs_16b(lc3_enc_smp_instance_ptr, 1, 2, (int16_t *)input_buffer, (int16_t *)temp_buf, 240);
            } else if (sample_rate_in == FS_RATE_32K && sample_rate == 16000) { /*16000 -> 32000*/
                updn_samp_prcs_16b(lc3_enc_smp_instance_ptr, 0, 2, (int16_t *)input_buffer, (int16_t *)temp_buf, 480);
            }
        } else {
            memcpy(temp_buf, input_buffer, lc3_enc_sw_buf_block_len);
        }

        input_buffer = temp_buf - lc3_enc_sw_buf_remain_len;
        lc3_enc_sw_buf_remain_len += lc3_enc_sw_buf_block_len;

        dummy_insert_num = lc3_enc_get_sink_dummy_insert_num(para);
        if(dummy_insert_num) {
            lc3_enc_set_sink_dummy_insert_num(para, dummy_insert_num - 1);
            lc3_enc_set_sink_dummy_process_status(para, 1);
            DSP_MW_LOG_I("[lc3][enc] insert dummy %d %d %d %d", 4, dummy_insert_num, lc3_enc_sw_buf_remain_len, lc3_enc_sw_buf_block_len, lc3_enc_sw_buf_remain_len + lc3_enc_sw_buf_block_len);
            lc3_enc_sw_buf_index ^= 1;
            lc3_enc_sw_buf_remain_len += lc3_enc_sw_buf_block_len;
        }

        do {
#ifdef AIR_AUDIO_DUMP_ENABLE
            LOG_AUDIO_DUMP(input_buffer, lc3_enc_sw_buf_consume_len, AUDIO_WOOFER_PLC_OUT);
#endif
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
            ret = LC3PLUSN_Enc_Prcs(p_lc3i_enc, p_lc3i_tab_common, out_buffer, input_buffer, &this_out_size, &lc3_FFTx);
#else
            ret = LC3I_Enc_Prcs(p_lc3i_enc, p_lc3i_tab_common, out_buffer, input_buffer, &this_out_size, &lc3_FFTx);
#endif
#ifdef AIR_AUDIO_DUMP_ENABLE
            //LOG_AUDIO_DUMP((U8 *)&this_out_size, 2, AUDIO_WOOFER_UPSAMPLE_16K);
            LOG_AUDIO_DUMP(out_buffer, this_out_size, AUDIO_WOOFER_UPSAMPLE_16K);
#endif
            out_size += this_out_size;
            if(!dummy_insert_num) {
            out_buffer += this_out_size;
            input_buffer += lc3_enc_sw_buf_consume_len;
            }
            lc3_enc_sw_buf_remain_len -= lc3_enc_sw_buf_consume_len;
            process_num++;

            //DSP_MW_LOG_E("[lc3][enc] lc3_process  this_out_size:%d out_size:%d process_num:%d", 3,this_out_size, out_size ,process_num);
        } while (lc3_enc_sw_buf_remain_len >= lc3_enc_sw_buf_consume_len);

        lc3_enc_set_sink_process_num(para, process_num);

    } else {
        /* always encode an frame */
        //LOG_AUDIO_DUMP(input_buffer, in_frame_size, AUDIO_INS_OUT_L);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        ret = LC3PLUSN_Enc_Prcs(p_lc3i_enc, p_lc3i_tab_common, out_buffer, input_buffer, &out_size, &lc3_FFTx);
#else
        ret = LC3I_Enc_Prcs(p_lc3i_enc, p_lc3i_tab_common, out_buffer, input_buffer, &out_size, &lc3_FFTx);
#endif
        lc3_enc_set_sink_process_num(para, 1);
    }

    if (ret != LC3_OK) {
        DSP_MW_LOG_E("[lc3][enc][fail] LC3_Enc_Prcs() %d", 1, ret);
        goto error;
    }
    stream_codec_modify_output_size(para, out_size);

#ifdef LC3_ENCODE_DUMP_ENABLE
    {
        //After discussing with algorithm team(HsinAn), this header is no need
        /*uint8_t header_dump_buf[6];
        static bool first_call = true;
        if (first_call == true) {
            header_dump_buf[0] = 320 & 0xFF;
            header_dump_buf[1] = (320 >> 8) & 0xFF;
            header_dump_buf[2] = 640 & 0xFF;
            header_dump_buf[3] = (640 >> 8) & 0xFF;
            header_dump_buf[4] = 0x01;
            header_dump_buf[5] = 0x00;
            LOG_AUDIO_DUMP(header_dump_buf, sizeof(header_dump_buf), AUDIO_INS_OUT_R);
            first_call = false;
        }*/

        uint16_t frame_length;
        frame_length = out_size;
        LOG_AUDIO_DUMP((uint8_t *)&frame_length, 2, AUDIO_INS_OUT_R);
    }
    LOG_AUDIO_DUMP(out_buffer, out_size, AUDIO_INS_OUT_R);
#endif

    //DSP_MW_LOG_I("[lc3][enc] stream_codec_encoder_lc3_process out_size %d\r\n", 1, out_size);

    return false;

error:
    stream_codec_modify_output_size(para, 0);
    return true;
}

const int16_t tone_1K_fs_48K[48] = { 0, 1074, 2130, 3150, 4115, 5011, 5820, 6530, 7128, 7604, 7950, 8160, 8230, 8160, 7950, 7603, 7128, 6530, 5819, 5011, 4115, 3149, 2131, 1074, 0, -1075, -2130, -3150, -4115, -5011, -5820, -6529, -7128, -7605, -7950, -8160, -8231, -8160, -7950, -7604, -7128, -6530, -5820, -5010, -4115, -3150, -2130, -1074};
const int16_t tone_1K_fs_32K[32] = { 0, 1605, 3150, 4573, 5820, 6844, 7604, 8073, 8230, 8072, 7604, 6844, 5819, 4573, 3150, 1607, 0, -1605, -3150, -4573, -5820, -6843, -7605, -8072, -8230, -8073, -7604, -6844, -5820, -4574, -3150, -1607};
const int16_t tone_1K_fs_16K[16] = { 0, 3150, 5820, 7604, 8230, 7604, 5819, 3150, 0, -3150, -5820, -7605, -8231, -7604, -5820, -3150};

static void internal_fill_1k_tone(uint8_t *dst, uint8_t *src, uint16_t dst_buf_len, uint16_t src_buf_len, uint16_t offset)
{
    if (dst) {
        uint16_t src_offset = offset;
        uint16_t dst_offset = 0;
        uint16_t remainder  = dst_buf_len;
        uint16_t copy_len   = src_buf_len - src_offset;


        while (remainder) {
            memcpy(dst + dst_offset, src + src_offset, copy_len);

            src_offset = 0;
            dst_offset += copy_len;
            remainder  -= copy_len;
            copy_len    = MIN(src_buf_len, remainder);
        }
    }
}

bool stream_codec_encoder_lc3_process_branch(void *para)
{

    LC3_ERR_T ret = LC3_OK;
    uint16_t pcm_resolution;
    uint8_t *input_buffer, *out_buffer, *working_buffer;
    uint32_t out_size, in_frame_size, sample_rate;
    uint32_t saved_mask, buffer_index;

    if (lc3_memory->lc3_enc_memory.InitDone == FALSE) {
        DSP_MW_LOG_E("[lc3][enc][fail] stream_codec_encoder_lc3_initialize is not called\r\n", 0);
        AUDIO_ASSERT(0);
    }

    /* check the size of the input buffer */
    in_frame_size = stream_codec_get_input_size(para);

    if (stream_codec_get_input_resolution(para) == RESOLUTION_16BIT) {
        pcm_resolution = 16;
    } else {
        DSP_MW_LOG_I("[lc3][enc] current LC3 encoder only support 16 bit resolution: (%d)", 1, stream_codec_get_input_resolution(para));
        goto error;
    }

    STREAM_BLE_UL_GET_PARAM(&sample_rate, NULL);


//    if ((sample_rate*2)/100 != in_frame_size) {
//        DSP_MW_LOG_E("[lc3][enc][fail] input PCM size mismatch. get: %d, expect %d\r\n", 2, in_frame_size,(sample_rate*2)/100 );
//        goto error;
//    }


    static uint16_t lc3_enc_count = 1000;

    if (lc3_enc_count == 1000) {
        lc3_enc_count = 1;

        uint8_t ch_num = stream_codec_get_output_channel_number(para);
        UNUSED(ch_num);
        //input_buffer = stream_codec_get_1st_input_buffer(para);
        //working_buffer = &lc3_memory->ScratchMemory[0];

        DSP_MW_LOG_E("[lc3][enc][para] ch_num: %d in_frame_size %d sample_rate %d pcm_resolution %d \r\n",
                     4, ch_num, in_frame_size, sample_rate, pcm_resolution);
    } else {
        lc3_enc_count ++;
    }


    if (STREAM_BLE_UL_CHECK_MODE()) { /* check if it is now 1K tone mode and overwrite input buffer*/
        static uint16_t offset = 0;
        uint16_t pattern_len = sizeof(tone_1K_fs_48K);
        uint8_t *pattern = (uint8_t *)tone_1K_fs_48K;
        uint8_t *input1_buffer = stream_codec_get_1st_input_buffer(para);
        uint8_t *input2_buffer = stream_codec_get_2nd_input_buffer(para);


        switch (sample_rate) {
            case 48000:
                pattern_len = sizeof(tone_1K_fs_48K);
                pattern = (uint8_t *)tone_1K_fs_48K;
                break;
            case 32000:
                pattern_len = sizeof(tone_1K_fs_32K);
                pattern = (uint8_t *)tone_1K_fs_32K;
                break;
            case 16000:
                pattern_len = sizeof(tone_1K_fs_16K);
                pattern = (uint8_t *)tone_1K_fs_16K;
                break;
            default:
                break;
        }

        if (input1_buffer) {
            internal_fill_1k_tone(input1_buffer, pattern, in_frame_size, pattern_len, offset);

            if (input2_buffer) {
                memcpy(input2_buffer, input1_buffer, in_frame_size);
            }
        }

        offset = (in_frame_size + offset) % pattern_len;    // next round would start from this offset
    }

//#define LC3_UL_DATA_PATH_DEBUG
#ifdef LC3_UL_DATA_PATH_DEBUG  /*debug usage, overwrite encoded data to increasing index */
    static uint8_t test_index = 0;
    test_index++;
#endif

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    buffer_index = STREAM_BLE_UL_GET_BUF_INDEX();
    hal_nvic_restore_interrupt_mask(saved_mask);

    /* encode an frame */
    if (STREAM_BLE_UL_CHECK_BUF(0)) {
        input_buffer = stream_codec_get_1st_input_buffer(para);
        //working_buffer = &lc3_memory->ScratchMemory[0];
        out_buffer = &lc3_memory->lc3_enc_memory.g_lc3_cache_buffer[0];
//        LOG_AUDIO_DUMP(input_buffer, in_frame_size, AUDIO_INS_OUT_L);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        ret = LC3PLUSN_Enc_Prcs(p_lc3i_enc, p_lc3i_tab_common, out_buffer, input_buffer, &out_size, &lc3_FFTx);;
#else
        ret = LC3I_Enc_Prcs(p_lc3i_enc, p_lc3i_tab_common, out_buffer, input_buffer, &out_size, &lc3_FFTx);
#endif
#ifdef LC3_UL_DATA_PATH_DEBUG
        memset(out_buffer, test_index, out_size);
#endif
        STREAM_BLE_UL_WRITE_BUF(0, out_buffer, out_size, buffer_index);
//        ret = LC3_Dec_Prcs(working_buffer, out_buffer, input_buffer, out_size, 0);  // test self decode
//        LOG_AUDIO_DUMP(out_buffer, out_size, AUDIO_INS_OUT_R);
        if (ret != LC3_OK) {
            DSP_MW_LOG_E("[lc3][enc][fail] LC3_Enc_Prcs(L ch) %d out_size:%d", 2, ret, out_size);
            goto error;
        }
    }

    if (STREAM_BLE_UL_CHECK_BUF(1)) {
        input_buffer = stream_codec_get_2nd_input_buffer(para);
        working_buffer = stream_codec_get_workingbuffer(para);
        out_buffer = working_buffer + (DSP_LC3_ENCODER_MEM_EACH_CH - 1000);
//        LOG_AUDIO_DUMP(input_buffer, in_frame_size, AUDIO_INS_OUT_R);
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        ret = LC3PLUSN_Enc_Prcs(p_lc3i_enc, p_lc3i_tab_common, out_buffer, input_buffer, &out_size, &lc3_FFTx);;
#else
        ret = LC3I_Enc_Prcs(p_lc3i_enc, p_lc3i_tab_common, out_buffer, input_buffer, &out_size, &lc3_FFTx);
#endif
#ifdef LC3_UL_DATA_PATH_DEBUG
        memset(out_buffer, test_index, out_size);
#endif
        STREAM_BLE_UL_WRITE_BUF(1, out_buffer, out_size, buffer_index);
        if (ret != LC3_OK) {
            DSP_MW_LOG_E("[lc3][enc][fail] LC3_Enc_Prcs(R ch) %d out_size:%d", 2, ret, out_size);
            goto error;
        }
    }

    BTCLK BtClk;
    BTPHASE BtPhase;
    extern VOID MCE_GetBtClk(BTCLK *pCurrCLK, BTPHASE *pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);
    MCE_GetBtClk(&BtClk, &BtPhase, BT_CLK_Offset);
    DSP_MW_LOG_I("[le audio DSP] stream update data, 0x%x, %d", 2, BtClk, buffer_index);

#ifdef AIR_AUDIO_DUMP_ENABLE
    input_buffer = stream_codec_get_1st_input_buffer(para);
    LOG_AUDIO_DUMP(input_buffer, in_frame_size, VOICE_TX_MIC_3);
#endif /* AIR_AUDIO_DUMP_ENABLE */

    stream_codec_modify_output_size(para, stream_codec_get_input_size(para));
    return false;

error:
    stream_codec_modify_output_size(para, stream_codec_get_input_size(para));
    return true;
}
#endif

