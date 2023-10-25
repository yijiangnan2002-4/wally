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

#ifdef AIR_BT_CODEC_BLE_ENABLED
#include "stream_n9ble.h"
#include "common.h"
#include "preloader_pisplit.h"

#include "lc3_dec_interface.h"
#ifdef MTK_BT_A2DP_LC3_USE_PIC
#include "lc3_codec_portable.h"
extern SemaphoreHandle_t g_lc3i_codec_process_xSemaphore; // for lc3i process only
#endif

#define LC3_MUSIC_SCENARIO_SAMPLE_SIZE  20
/*
#define LC3_CALL_SCENARIO_SAMPLE_SIZE   480    --> 32K out fs
#define LC3_CALL_SCENARIO_SAMPLE_SIZE   240    --> 16K out fs
 */
#define LC3_PLC_MUTE_OUT_THD          10

typedef enum {
    RESOLUTION_16_BIT,
    RESOLUTION_24_BIT,
    RESOLUTION_32_BIT,
} resolution_t;


static uint16_t g_prev_left_sample = 0;
static uint16_t g_lc3_frame_sample = 0;
static uint8_t lc3_plc_cnt = 0;
static bool lc3_dec_src_enabled;
static bool g_lc3_dec_stereo_mode = false;
//static LC3_DEC_INSTANCE_PTR lc3_dec_memory = NULL;
void *p_lc3i_tab_common = NULL;
static void *p_lc3i_tab_dec = NULL;
static void *p_lc3i_dec = NULL;
U8 g_lc3_user_cnt;
ENCDEC lc3_param_encdec[2];
LC3_INSTANCE_PTR lc3_memory = NULL;
LC3I_PTR lc3_MEM_PTR[C_LC3I_MAX_NUM];
Multi_FFT lc3_FFTx;
extern uint16_t g_ble_abr_length;
LC3_DEC_MODE_T lc3_dec_mode;

void LC3I_Set_Dec_Param(U16 Channel, U16 frame_interval,U16 plc_mode,U32 sample_rate, U32 bit_rate)
{
    g_lc3_user_cnt += 1;
    DSP_MW_LOG_I("[BLE][DL] g_lc3_user_cnt++ %d", 1, g_lc3_user_cnt);

    /*config LC3 init param*/
    lc3_param_encdec[0].alg = C_Alg_LC3;
    lc3_param_encdec[0].mode = C_LC3I_Dec;
    lc3_param_encdec[0].plcmeth = plc_mode;
    lc3_param_encdec[0].frame_ms = frame_interval;
    lc3_param_encdec[0].ch = Channel;
    lc3_param_encdec[0].sr = sample_rate;
    lc3_param_encdec[0].bps = 16;
    lc3_param_encdec[0].bitrate = bit_rate;
    lc3_param_encdec[0].delay = LC3_NO_DELAY_COMPENSATION;
    lc3_param_encdec[0].lfe= 0;

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

    DSP_MW_LOG_I("[lc3][dec][init_config] plcmeth %d, frame_ms %d, ch %d, sample_rate %d",4,lc3_param_encdec[0].plcmeth,lc3_param_encdec[0].frame_ms,lc3_param_encdec[0].ch,lc3_param_encdec[0].sr);
    DSP_MW_LOG_I("[lc3][dec][init_config] resolution %d, bitrate %d, delay %d, lfe %d",4,lc3_param_encdec[0].bps,lc3_param_encdec[0].bitrate,lc3_param_encdec[0].delay,lc3_param_encdec[0].lfe);
}
/*static ALIGN(8) dsp_updn_sample_interface_t lc3_dec_downsample_para = {
    .NvKey = {
        0x1FCF,0x2752,0x4306,0xCD63,0x2752,0x2752,0x08F8,0x2752,
        0x2839,0xA49F,0x00CE,0x2752,0x19C3,0x893E,0x2752,0x0000,
    },
};*/
extern int16_t dsp_sampler_coef[32];

void *lc3_dec_smp_instance_ptr;

static const uint32_t g_lc3_pcm_frame_samples[][6] = {
    {80, 160, 240, 320, 480, 480}, /* 10 ms */
    {60, 120, 180, 240, 360, 360}, /* 7.5 ms */
};
void lc3_dual_decode_mode_set(BOOL isStereo)
{
    g_lc3_dec_stereo_mode = isStereo;
}

uint32_t lc3_sample_rate_translate(stream_samplerate_t sample_rate)
{
    if (sample_rate == FS_RATE_8K) {
        return 0;
    } else if (sample_rate == FS_RATE_16K) {
        return 1;
    } else if (sample_rate == FS_RATE_24K) {
        return 2;
    } else if (sample_rate == FS_RATE_32K) {
        return 3;
    } else if (sample_rate == FS_RATE_44_1K) {
        return 4;
    } else if (sample_rate == FS_RATE_48K) {
        return 5;
    } else {
        AUDIO_ASSERT(0);
    }
    return 0;
}

ATTR_TEXT_IN_IRAM static void deinterleave_pcm_data(resolution_t in_res, resolution_t out_res, uint8_t *left_buf, uint8_t *right_buf, uint32_t total_len, uint16_t channel)
{
    uint32_t i, temp_1, temp_2, loop_cnt;
    uint32_t *p_buf, *p_left_buf, *p_right_buf;

    if (out_res == RESOLUTION_24_BIT) {
        AUDIO_ASSERT(0);
    }

    if (channel == 2) {
        if (out_res == RESOLUTION_32_BIT) {
            /* out with 32 resolution */
            if (in_res == RESOLUTION_24_BIT) {
                /* 24 bits -> 32 bits, L ->out_l, R -> out_r */
                p_buf = (uint32_t *)left_buf;
                p_left_buf = (uint32_t *)left_buf;
                p_right_buf = (uint32_t *)right_buf;
                loop_cnt = total_len / (3 * 2 * 2);
                for (i = loop_cnt; i > 0; i--) {
                    temp_1 = *p_buf++;
                    *p_left_buf++ = (temp_1 << 8);
                    temp_2 = *(p_buf++);
                    *p_right_buf++ = (temp_2 << 16) | (((temp_1) >> 16) & 0x00ff00L);
                    temp_1 = *p_buf++;
                    *p_left_buf++ = ((temp_2 >> 8) & 0x00ffff00) | (temp_1 << 24);
                    *p_right_buf++ = temp_1 & 0xFFFFFF00;
                }
            } else if (in_res == RESOLUTION_32_BIT) {
                /* 32 bits -> 32 bits, L ->out_l, R -> out_r */
                for (i = 0 ; i < total_len / (2 * sizeof(uint32_t)) ; i++) {
                    *(uint32_t *)&(left_buf[4 * i]) = *(uint32_t *) & (left_buf[8 * i]);
                    /* Should always have enough buffer size in splitBuf to do so */
                    *(uint32_t *)&(right_buf[4 * i]) = *(uint32_t *) & (left_buf[8 * i + 4]);

                }
            } else {
                AUDIO_ASSERT(0);
            }
        } else {
            /* out with 16 resolution */
            for (i = 0 ; i < total_len / (2 * sizeof(uint16_t)) ; i++) {
                /* 16 bits -> 16 bits, L ->out_l, R -> out_r */
                *(uint16_t *)&(left_buf[2 * i]) = *(uint16_t *) & (left_buf[4 * i]);
                /* Should always have enough buffer size in splitBuf to do so */
                *(uint16_t *)&(right_buf[2 * i]) = *(uint16_t *) & (left_buf[4 * i + 2]);
            }
        }
    }
}

BOOL LC3_Dec_MemCheck (VOID)
{
    BOOL ret = FALSE;
    if ((NULL != lc3_memory) && (LC3_DEC_VALID_MEMORY_CHECK_VALUE == lc3_memory->lc3_dec_memory.MemoryCheck))
    {
        ret = TRUE;
    }
    return ret;
}

VOID LC3_Tab_Dec_MemInit (VOID* para,LC3I_Param *param){
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    uint32_t lc3i_tab_dec_size = 0;
    uint32_t lc3i_tab_common_size = 0;
    uint32_t lc3i_dec_size = 0;

    LC3_ERR_T ret;
    DSP_MW_LOG_I("[lc3][dec] LC3_Decoder_TabInit", 0);
    if (!LC3_Dec_MemCheck())
    {
#ifdef ROM_TABLE_ADDR_LC3
        LC3I_Set_ROM_Start(ROM_TABLE_ADDR_LC3);
#endif
        if (p_lc3i_tab_common == NULL && lc3_memory == NULL) {
            lc3_memory = (LC3_INSTANCE_PTR) preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE , sizeof(LC3_INSTANCE));
            DSP_MW_LOG_I("[lc3][dec] LC3I_Tab_Common_Init()",0);
            /*Table Common*/
            lc3i_tab_common_size = LC3I_Tab_Common_Get_MemSize();
            DSP_MW_LOG_I("[lc3][dec] LC3I_Tab_Common_Init() lc3i_tab_common_size %d",1, lc3i_tab_common_size);

            lc3i_tab_common_size = (lc3i_tab_common_size + 7) / 8 * 8;
            p_lc3i_tab_common = (void*) preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE , lc3i_tab_common_size);
            ret = LC3I_Tab_Common_Init(p_lc3i_tab_common);
            if (ret != LC3_OK) {
                DSP_MW_LOG_E("[lc3][fail] LC3I_Tab_Common_Init() %d", 1, ret);
                return;
            }
            DSP_MW_LOG_I("[lc3][dec][enc] LC3I_Tab_Common_Init() Done 0x%08x",1,p_lc3i_tab_common);
        }

        DSP_MW_LOG_I("[lc3][dec] LC3I_Tab_Dec_Init()",0);
        /*Table Dec*/
        lc3i_tab_dec_size = LC3I_Tab_Dec_Get_MemSize(param);
        lc3i_tab_dec_size = (lc3i_tab_dec_size + 7) / 8 * 8;
        DSP_MW_LOG_I("[lc3][dec] LC3I_Tab_Dec_Init()lc3i_tab_dec_size %d",1, lc3i_tab_dec_size);

        p_lc3i_tab_dec = (void*) preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE , lc3i_tab_dec_size);
        ret = LC3I_Tab_Dec_Init(p_lc3i_tab_common, p_lc3i_tab_dec, param);
        if (ret != LC3_OK) {
            DSP_MW_LOG_E("[lc3][fail] LC3I_Tab_Dec_Init() %d", 1, ret);
            return;
        }
        DSP_MW_LOG_I("[lc3][dec] LC3I_Tab_Dec_Init() Done 0x%08x",1,p_lc3i_tab_dec);

        /*Decoder Working Buffer*/
        DSP_MW_LOG_I("[lc3][dec] LC3I_Dec_Init() ch %d sr %d frame_ms %d plcmeth %d ",4, lc3_param_encdec[0].ch, lc3_param_encdec[0].sr, lc3_param_encdec[0].frame_ms, lc3_param_encdec[0].plcmeth);
        lc3i_dec_size = LC3I_Dec_Get_MemSize(lc3_param_encdec[0].ch, lc3_param_encdec[0].sr, lc3_param_encdec[0].frame_ms, lc3_param_encdec[0].plcmeth);
        lc3i_dec_size = (lc3i_dec_size + 7) / 8 * 8;
        DSP_MW_LOG_I("[lc3][dec] LC3I_Dec_Init() lc3i_dec_size %d",1, lc3i_dec_size);
        p_lc3i_dec = (void*) preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE , lc3i_dec_size);
        DSP_MW_LOG_I("[lc3][dec] LC3I_Dec_Init() bps %d sr %d ch %d frame_ms %d delay %d plcmeth %d",6, lc3_param_encdec[0].bps, lc3_param_encdec[0].sr, lc3_param_encdec[0].ch, lc3_param_encdec[0].frame_ms, lc3_param_encdec[0].delay, lc3_param_encdec[0].plcmeth);
        ret = LC3I_Dec_Init(p_lc3i_dec, lc3_param_encdec[0].bps, lc3_param_encdec[0].sr, lc3_param_encdec[0].ch, lc3_param_encdec[0].frame_ms, lc3_param_encdec[0].delay, lc3_param_encdec[0].plcmeth);
        if (ret != LC3_OK) {
            DSP_MW_LOG_E("[lc3][fail] LC3I_Dec_Init() %d", 1, ret);
            return;
        }
        DSP_MW_LOG_I("[lc3] LC3I_Get_Version : %x",1,LC3I_Get_Version());
        DSP_MW_LOG_I("[lc3][dec] LC3I_Dec_Init() Done 0x%08x",1, p_lc3i_dec);

        lc3_memory->lc3_dec_memory.MemoryCheck = LC3_DEC_VALID_MEMORY_CHECK_VALUE;
        lc3_memory->lc3_dec_memory.InitDone = true;

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        if(lc3i_tab_size>stream_codec_get_working_buffer_length(para)){
            DSP_MW_LOG_E("[lc3] mem size is insufficient, required:%d, allocated %d",2,lc3i_tab_size,stream_codec_get_working_buffer_length(para));
            assert(0);
        }
        p_lc3i_tab = (void*)stream_codec_get_workingbuffer(para);
        lc3_memory = p_lc3i_tab + lc3i_tab_size;
#endif
    }
}

void LC3_Dec_Deinit(void){
    U16 frame_interval = lc3_param_encdec[0].frame_ms;
    lc3_param_encdec[0].alg        = 0;
    lc3_param_encdec[0].mode       = 0;
    lc3_param_encdec[0].plcmeth    = 0;
    lc3_param_encdec[0].frame_ms   = 0;
    lc3_param_encdec[0].ch         = 0;
    lc3_param_encdec[0].sr         = 0;
    lc3_param_encdec[0].bps        = 0;
    lc3_param_encdec[0].bitrate    = 0;
    lc3_param_encdec[0].delay      = 0;
    lc3_param_encdec[0].lfe        = 0;
#ifdef PRELOADER_ENABLE
    if (frame_interval == LC3_INTERVAL_10_MS) {
        lc3i_fft10ms_library_unload();
    } else if (frame_interval == LC3_INTERVAL_7P5_MS){
        lc3i_fft7_5ms_library_unload();
    } else {
        DSP_MW_LOG_E("[BLE] unsupported DL frame interval %d", 1, frame_interval);
    }
#endif
    UNUSED(frame_interval);
    /*Working memory*/
    preloader_pisplit_free_memory(p_lc3i_dec);
    p_lc3i_dec = NULL;

    /*table dec*/
    preloader_pisplit_free_memory(p_lc3i_tab_dec);
    p_lc3i_tab_dec = NULL;

    lc3_memory->lc3_dec_memory.InitDone = FALSE;
    lc3_memory->lc3_dec_memory.MemoryCheck = 0;

    g_lc3_user_cnt -= 1;
    if (g_lc3_user_cnt == 0) {
        /*Table common memory*/
        preloader_pisplit_free_memory(p_lc3i_tab_common);
        p_lc3i_tab_common = NULL;
        preloader_pisplit_free_memory(lc3_memory);
        lc3_memory = NULL;
    }
    DSP_MW_LOG_I("[BLE][DL] g_lc3_user_cnt-- %d", 1, g_lc3_user_cnt);
}

bool stream_codec_decoder_lc3_initialize(void *para)
{
    uint32_t sample_rate, channel, sample_rate_index, sample_limitation;
    #ifndef AIR_BLE_FIXED_RATIO_SRC_ENABLE
    int32_t src_quality_mode = 0;
    uint32_t instance_size = get_updn_samp_memsize(src_quality_mode);
    #endif
    uint16_t pcm_resolution;
    uint16_t frame_interval;
    DSP_STREAMING_PARA_PTR stream_ptr;

    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    stream_codec_modify_output_size(para, 0);

#if 0
    if (DSP_GetCodecOutResolution(para) == RESOLUTION_16BIT) {
        pcm_resolution = 16;
    } else {
        pcm_resolution = 32;
    }
#else
    pcm_resolution = 16;
#endif
    if (stream_codec_get_input_samplingrate(para) == FS_RATE_44_1K) {
        sample_rate = 44100;
    } else if (stream_codec_get_input_samplingrate(para) == FS_RATE_88_2K) {
        sample_rate = 88200;
    } else {
        sample_rate = stream_codec_get_input_samplingrate(para) * 1000;
    }

    lc3_dec_mode = (stream_ptr->source->param.n9ble.context_type == BLE_CONTEXT_CONVERSATIONAL)? C_LC3_Voice_Dec : C_LC3_Audio_Dec ;
    //lc3_dec_mode = (stream_codec_get_input_samplingrate(para) <= FS_RATE_32K) ? C_LC3_Voice_Dec : C_LC3_Audio_Dec ;
    channel = stream_codec_get_input_channel_number(para);
    if ((g_lc3_dec_stereo_mode) && (lc3_dec_mode == C_LC3_Audio_Dec)) { /*voice decode support mono only*/
        channel += 1;
    }


    frame_interval = stream_ptr->source->param.n9ble.frame_interval / 100;
    /* unit of stream_ptr->source->param.n9ble.frame_interval is 1us */
    /* unit of frame interval needed by LC3 API is 100us */

    stream_samplerate_t sample_rate_out = stream_ptr->sink->param.audio.src_rate / 1000;

#ifndef AIR_BLE_FIXED_RATIO_SRC_ENABLE
#ifdef AIR_BT_BLE_SWB_ENABLE
    DSP_MW_LOG_I("AIR_BT_BLE_SWB_ENABLE", 0);
    lc3_dec_src_enabled = false;
    if (sample_rate == 16000)
#else
    if (sample_rate == 32000)
#endif
    {
        lc3_dec_src_enabled = true;
        //updn_sampling_by2_Init(&lc3_dec_downsample_para);
        if (lc3_dec_smp_instance_ptr) {
            preloader_pisplit_free_memory(lc3_dec_smp_instance_ptr);
        }
        lc3_dec_smp_instance_ptr = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, instance_size);
        if (lc3_dec_smp_instance_ptr == NULL) {
            DSP_MW_LOG_E("[lc3][dec][init] malloc fail!", 0);
            AUDIO_ASSERT(0);
        }
        updn_samp_init(lc3_dec_smp_instance_ptr, (void*)&dsp_sampler_coef, src_quality_mode);

    } else {
        lc3_dec_src_enabled = false;
    }
#else
    DSP_MW_LOG_I("AIR_BLE_FIXED_RATIO_SRC_ENABLE",0);
    sample_rate_out = sample_rate / 1000;
    lc3_dec_src_enabled = false;
#endif
    DSP_MW_LOG_I("[lc3][dec] sample rate in: %d out: %d", 2, sample_rate, sample_rate_out);

    LC3I_Param lc3_param;
    lc3_param.frame_ms = frame_interval;
    lc3_param.sr = sample_rate;
    LC3_Tab_Dec_MemInit (para, &lc3_param);
    //DSP_MW_LOG_I("[lc3][XT] alg %X, p_mem %X, p_tmp %X",3,lc3_MEM_PTR[0].alg,lc3_MEM_PTR[0].p_mem,lc3_MEM_PTR[0].p_tmp);

    /* Decide the sample count of each frame */
    if (lc3_dec_mode == C_LC3_Audio_Dec) {
        sample_limitation = LC3_MUSIC_SCENARIO_SAMPLE_SIZE;
        sample_rate_index = lc3_sample_rate_translate(stream_codec_get_input_samplingrate(para));
        if (frame_interval == LC3_INTERVAL_10_MS) {
            g_lc3_frame_sample = g_lc3_pcm_frame_samples[0][sample_rate_index];
        } else {
            g_lc3_frame_sample = g_lc3_pcm_frame_samples[1][sample_rate_index];
        }
        g_lc3_frame_sample -= g_lc3_frame_sample % sample_limitation;
        g_prev_left_sample = 0;
    } else {
        g_lc3_frame_sample = sample_rate_out * 15;   /*LC3_CALL_SCENARIO_SAMPLE_SIZE; */
        g_prev_left_sample = (frame_interval == LC3_INTERVAL_10_MS) ? (sample_rate_out * frame_interval / 10) :  0;/*AFE prefill data already 15ms*/
        DSP_MW_LOG_I("[lc3][dec] prefill %d samples for call mode align 15 ms", 1, g_prev_left_sample);
        memset(&lc3_memory->lc3_dec_memory.g_lc3_cache_buffer[0],0,LC3_CACHE_BUFFER_SIZE);
    }

    lc3_memory->lc3_dec_memory.InitDone = true;
    DSP_MW_LOG_I("[lc3] LC3I_Get_Version : %x",1,LC3I_Get_Version());
    DSP_MW_LOG_I("[lc3][dec] stream_codec_decoder_lc3_initialize() exit", 0);
    return false;
}

ATTR_TEXT_IN_IRAM bool stream_codec_decoder_lc3_process(void *para)
{
    LC3_ERR_T ret;
    stream_resolution_t out_resolution;
    DSP_STREAMING_PARA_PTR stream_ptr;
    int16_t packet_lost_st;
    uint32_t out_frame_size, bit_stream_size, frame_sample_count, channel_number, bad_frame_indicator;
    int32_t look_ahead_delay;
    uint8_t *input_buffer, *left_buffer, *right_buffer;
    uint32_t i, curr_frame_sample_count, sample_limitation, total_sample, input_resolution;
    uint32_t sample_rate;

    /* check the status of the input buffer */
    if (stream_codec_get_input_size(para) == 0) {
        DSP_MW_LOG_E("[lc3][dec][fail] LC3 Decode size mismatch %d\r\n", 1, stream_codec_get_input_size(para));
        goto error;
    }

    /* Get the parameters */
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    bit_stream_size = g_ble_abr_length;//stream_codec_get_input_size(para);
    input_buffer = stream_codec_get_1st_input_buffer(para);
    left_buffer = stream_codec_get_1st_output_buffer(para);
    right_buffer = stream_codec_get_2nd_output_buffer(para);
    channel_number = stream_codec_get_input_channel_number(para);
    if (stream_codec_get_input_samplingrate(para) == FS_RATE_44_1K) {
        sample_rate = 44100;
    } else if (stream_codec_get_input_samplingrate(para) == FS_RATE_88_2K) {
        sample_rate = 88200;
    } else {
        sample_rate = stream_codec_get_input_samplingrate(para) * 1000;
    }

    /* Voice decode support mono only */
    if (g_lc3_dec_stereo_mode) {
        //if(stream_codec_get_input_samplingrate(para)>FS_RATE_32K){
        if (lc3_dec_mode == C_LC3_Audio_Dec) {
            channel_number = 2; /* music: L+R */
            if(stream_ptr->source->param.n9ble.dual_cis_status != DUAL_CIS_DISABLED) {
                bit_stream_size *= 2;
            }
        } else {
            if(stream_ptr->source->param.n9ble.dual_cis_status == DUAL_CIS_DISABLED)
            {
                channel_number = 1;
                bit_stream_size /= 2; /* voice: L only, and need to modify stream size to half */
            }
        }
    }

#if 0
    DSP_MW_LOG_I("[lc3][dec] bit_stream_size %d, input_buffer 0x%08x, working_buffer 0x%08x, left_buffer 0x%08x, right_buffer 0x%08x, channel_number %d\r\n", 6,
                 bit_stream_size, input_buffer, working_buffer, left_buffer, right_buffer, channel_number);
#endif

    //LOG_AUDIO_DUMP((uint8_t *)&bit_stream_size, 2, AUDIO_CODEC_IN);
    //LOG_AUDIO_DUMP(input_buffer, bit_stream_size, AUDIO_CODEC_IN);


    frame_sample_count = 0;
    /* Do cache buffer check if following feature need frame with different samples  */
    if (stream_codec_get_input_resolution(para) == RESOLUTION_16BIT) {
        input_resolution = 2;
    } else {
        input_resolution = 4;
    }

    for (i = 0; i < stream_ptr->source->param.n9ble.process_number; i++) {
        /* decode frame */
        packet_lost_st = ble_query_rx_packet_lost_status(i);
        if(g_lc3_dec_stereo_mode)
            packet_lost_st |= ble_query_rx_sub_cis_packet_lost_status(i);

        bad_frame_indicator = 0;
#ifdef AIR_AUDIO_DUMP_ENABLE
        LOG_AUDIO_DUMP(input_buffer + i * bit_stream_size, bit_stream_size, AUDIO_CODEC_IN);
#endif

        //DSP_MW_LOG_I("[lc3] LC3_Dec_Prcs  p_lc3i_ptr %x, lc3_dec_MEM_PTR:%X, p_lc3i_dec_tab %X,  input_buffer:%X left_buffer:%X  g_lc3_frame_sample:%d, bit_stream_size %d", 7, p_lc3_dec_ptr,&lc3_MEM_PTR, p_lc3i_tab,input_buffer,left_buffer,g_lc3_frame_sample,bit_stream_size);
        lc3_plc_cnt = (packet_lost_st)? lc3_plc_cnt + 1 : 0;
#if defined(MTK_BT_A2DP_LC3_USE_PIC) && !defined(DSP_MIPS_FEATURE_PROFILE)
        if (xSemaphoreTake(g_lc3i_codec_process_xSemaphore, portMAX_DELAY) != pdTRUE) {
            DSP_MW_LOG_E("[lc3][dec][fail] LC3_Dec_Prcs() semaphore take fail ",0);
            configASSERT(0);
        }
#endif
        ret = LC3I_Dec_Prcs(p_lc3i_dec, p_lc3i_tab_common,
                            input_buffer + i * bit_stream_size,
                            left_buffer + i * g_lc3_frame_sample * channel_number,
                            bit_stream_size, packet_lost_st, bad_frame_indicator, &lc3_FFTx);
        if ((ret != LC3_OK) && (packet_lost_st == 0)) {
            DSP_MW_LOG_E("[lc3][dec][fail] LC3_Dec_Prcs() %d PLC : %d SEQ: %d", 3, ret, packet_lost_st, stream_ptr->source->param.n9ble.seq_num + i);
            //configASSERT(0);
            memset(left_buffer, 0, g_lc3_frame_sample * input_resolution * channel_number);
        }
#if defined(MTK_BT_A2DP_LC3_USE_PIC) && !defined(DSP_MIPS_FEATURE_PROFILE)
        if (xSemaphoreGive(g_lc3i_codec_process_xSemaphore) == pdFALSE) {
            DSP_MW_LOG_E("[lc3][dec][fail] LC3_Dec_Prcs() semaphore give fail",0);
        }
#endif

        /* L/R and resolution adjustment if needed */
        LC3I_Dec_Get_Param(p_lc3i_dec, &curr_frame_sample_count, &look_ahead_delay);
        frame_sample_count += curr_frame_sample_count;
        //DSP_MW_LOG_I("[lc3] ret %d, frame_length %d, packet_lost_st %d, curr_frame_sample_count %d", 4, ret, stream_ptr->source->param.n9ble.frame_length, packet_lost_st, curr_frame_sample_count);
    }
    if (lc3_plc_cnt > LC3_PLC_MUTE_OUT_THD) {
        lc3_plc_cnt = LC3_PLC_MUTE_OUT_THD;
        memset(left_buffer, 0, g_lc3_frame_sample * input_resolution * channel_number);
    }

    if (lc3_dec_src_enabled) {
        if (sample_rate == 32000) {/*32->16*/
            //updn_sampling_by2_Proc(&lc3_dec_downsample_para, (int16_t*)left_buffer, (int16_t*)left_buffer, curr_frame_sample_count, 0, true);       /*down by 2*/
            updn_samp_prcs_16b(lc3_dec_smp_instance_ptr, 0, 2, (int16_t *)left_buffer, (int16_t *)left_buffer, curr_frame_sample_count);
            frame_sample_count /= 2;
            stream_codec_modify_output_samplingrate(para, FS_RATE_16K);
        } else if (sample_rate == 16000) {/*16->32*/
            //uint8_t *temp_buf = lc3_enc_sw_buf + lc3_enc_sw_buf_index * lc3_enc_sw_buf_block_len;
            updn_samp_prcs_16b(lc3_dec_smp_instance_ptr, 1, 2, (int16_t *)left_buffer, (int16_t *)right_buffer, curr_frame_sample_count);
            frame_sample_count *= 2;
            stream_codec_modify_output_samplingrate(para, FS_RATE_32K);
            memcpy(left_buffer, right_buffer, frame_sample_count * 2); //RESOLUTION_16BIT
        }
    }

    if (!((g_prev_left_sample == 0) && ((frame_sample_count % g_lc3_frame_sample) == 0))) {
        /* Copy to cache buffer */
        total_sample = frame_sample_count + g_prev_left_sample;
        assert((total_sample * channel_number * input_resolution) <= LC3_CACHE_BUFFER_SIZE);
        memcpy(&lc3_memory->lc3_dec_memory.g_lc3_cache_buffer[g_prev_left_sample * input_resolution * channel_number], left_buffer, frame_sample_count * input_resolution * channel_number);
        if (total_sample >= g_lc3_frame_sample) {
            /* decide current process size */
            sample_limitation = (total_sample / g_lc3_frame_sample) * g_lc3_frame_sample;
            /* Copy front data back to out buffer */
            memcpy(left_buffer, &lc3_memory->lc3_dec_memory.g_lc3_cache_buffer[0], sample_limitation * input_resolution * channel_number);
            /* Move left data to front of cache buffer */
            memmove(&lc3_memory->lc3_dec_memory.g_lc3_cache_buffer[0], &lc3_memory->lc3_dec_memory.g_lc3_cache_buffer[sample_limitation * input_resolution * channel_number], (total_sample - sample_limitation) * input_resolution * channel_number);
            g_prev_left_sample = total_sample - sample_limitation;
            frame_sample_count = sample_limitation;
        } else {
            /* Don't output data as the frame size is not enough */
            g_prev_left_sample += frame_sample_count;
            frame_sample_count = 0;
        }
    }
    //DSP_MW_LOG_I("[lc3] lc3 decode out size %d, left size %d", 2, frame_sample_count, g_prev_left_sample);

    //out_resolution = DSP_GetCodecOutResolution(para);
    out_resolution = RESOLUTION_16BIT;
    out_frame_size = 0;
    if (out_resolution == RESOLUTION_16BIT) {
        out_frame_size = frame_sample_count * channel_number * 2;
        deinterleave_pcm_data(RESOLUTION_16_BIT, RESOLUTION_16_BIT, left_buffer, right_buffer, out_frame_size, channel_number);
    } /*else {
        out_frame_size = frame_sample_count * channel_number * 4;
        deinterleave_pcm_data(RESOLUTION_32_BIT, RESOLUTION_32_BIT, left_buffer, right_buffer, out_frame_size, channel_number);
    }*/

    /* temp code for copy L to R */
    if (channel_number == 1) {
        memcpy(right_buffer, left_buffer, out_frame_size / channel_number);
    }

//#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP(left_buffer, out_frame_size / channel_number, AUDIO_SOURCE_IN_L);
    LOG_AUDIO_DUMP(right_buffer, out_frame_size / channel_number, AUDIO_SOURCE_IN_R);
//#endif

    /* config setting of out buffer after decoder done */
    //DSP_ModifyCodecProcessResolution(para, DSP_GetCodecOutResolution(para));
    stream_codec_modify_output_size(para, out_frame_size / channel_number);
    //DSP_ModifyCodecOutStreamSamplingRate(para, stream_codec_get_input_samplingrate(para));

    return false;

error:
    stream_codec_modify_output_size(para, 0);
    return true;
}

#endif

