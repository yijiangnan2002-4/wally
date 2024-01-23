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
#include "dsp_feature_interface.h"
#include "dsp_utilities.h"
#include "dsp_buffer.h"
#include "audio_plc.h"
#include "config.h"
#include "audio_plc_interface.h"
#include "dsp_dump.h"
#include "dsp_audio_process.h"
#include "dsp_memory.h"
#include "dsp_callback.h"
#include "hal_dwt.h"

#include "common.h"
#include "dsp_para_plc_nvstruc.h"

static DSP_AUDIO_PLC_CTRL_t g_audio_plc_ctrl;

#if (AIR_AUDIO_PLC_DEBUG_CONVERT_TO_16BIT_ENABLE)
S16 temp_buffer[2048];
#endif

DSP_PARA_AUDIO_PLC_STRU plc_nvkey = {
	0x1,					// U8  ENABLE
	0x1,					// U8  REVISION
	0x0003,					// U16 option: b[1] = 1(2-channel     ), 0(1-channel      )
							//             b[0] = 1(Enable PRE_EMP), 0(Disable PRE_EMP)
	128,    				// S16 framesize
	0x0000,					// U16 reserve_1
	965,					// S16 pitch_lag_max, 720, 965
	500,					// S16 pitch_lag_min, 100, 500
	PLC_DECAY_LEVEL,        // S16 decay_level   compensate sample = 32768 / decay_level    Ex: decay_level = 16 => compensate sample = 32768 / 16 = 2048 samples
	1024,					// S16 max_period must be 2000 or 1024, max_period must be larger than pitch_lag_max or pitch_lag_min.
	0,					    // 0(disable debounce_frame), max number is up to 16
	0x0000,					// U16 reserve_3
	0x0000,					// U16 reserve_4
	0x0000,					// U16 reserve_5
	0x0000,					// U16 reserve_6
	0x0000 					// U16 reserve_7
};

extern BOOL check_source_buffer_a2dp(U32 boundary_samples);
                                                                          
static DSP_AUDIO_PLC_SUPPORT_PARA_t g_plc_support_para[] = 
    //PLC API      frame size
{
    {PLC_Proc_256,     256},
    {PLC_Proc_512,     512},
    {PLC_Proc_1024,    1024},
#if defined (MTK_BT_A2DP_VENDOR_3_ENABLE) 
    {PLC_Proc_480,     480},
#endif
};

bool Audio_PLC_Init (VOID* para);

bool Audio_PLC_Process (VOID* para);

bool Audio_PLC_Init (VOID* para)
{
    AUDIO_PLC_SCRATCH_PTR_t audio_plc_feature_ptr = stream_function_get_working_buffer(para);
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);

    if(stream_ptr->source->type == SOURCE_TYPE_A2DP) {
        audio_plc_feature_ptr->audio_plc_get_bfi = stream_codec_get_mute_flag; //the function which used to return BFI information   0:good frame   1:bad frane
#if (AIR_AUDIO_PLC_REMOVE_SHORT_SOUND_ENABLE)
        audio_plc_feature_ptr->audio_plc_check_buffer_level = check_source_buffer_a2dp; //the function the function which used to reply whether the buffer level reaches the target 
#endif
    }
#ifdef AIR_BT_CODEC_BLE_ENABLED
    else if(stream_ptr->source->type == SOURCE_TYPE_N9BLE) {
        audio_plc_feature_ptr->audio_plc_get_bfi = NULL; //the function which used to return BFI information   0:good frame   1:bad frane
    }
#endif
    else {
        AUDIO_ASSERT(FALSE && "[Audio_PLC] unsupport scenario");
    }

    DSP_MW_LOG_I("[Audio_PLC] Init\r\n",0);

    return FALSE;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 U16 DSP_GetStreamCodecOutSize(VOID* para)
{
    return ((DSP_ENTRY_PARA_PTR)para)->codec_out_size;
}

U32 audio_plc_fs_converter(stream_samplerate_t fs_in)
{
    U32 fs_out;

    switch (fs_in) {
        case FS_RATE_8K:
            fs_out = 8000;
            break;
        case FS_RATE_16K:
            fs_out = 16000;
            break;
        case FS_RATE_24K:
            fs_out = 24000;
            break;
        case FS_RATE_32K:
            fs_out = 32000;
            break;
        case FS_RATE_44_1K:
            fs_out = 44100;
            break;
        case FS_RATE_48K:
            fs_out = 48000;
            break;
        case FS_RATE_88_2K:
            fs_out = 88200;
            break;
        case FS_RATE_96K:
            fs_out = 96000;
            break;
        case FS_RATE_192K:
            fs_out = 192000;
            break;
        default:
            assert(0);
            fs_out = 0;
            break;
    }
    return fs_out;
}

U32 Audio_PLC_Support_Api(U16 streamlength_sample)
{
    U32 re_val = INVALID_INDEX;
    U32 support_amount = ARRAY_SIZE(g_plc_support_para);
    U32 i = 0;

    for(i = 0; i < support_amount; i++){
        if(g_plc_support_para[i].frame_size == streamlength_sample){
            re_val = i;
            break;
        }
    }

    return re_val;
}

U32 Audio_PLC_Check_Support_Api(U16 streamlength_sample)
{
    U32 support_ndex = Audio_PLC_Support_Api(streamlength_sample);

    if(support_ndex == INVALID_INDEX) {//need to do buffering or batch processing
        if((streamlength_sample < 256 && 256 % streamlength_sample == 0) || (streamlength_sample > 256 && streamlength_sample % 256 == 0)) {
            streamlength_sample = 256;
        } else if((streamlength_sample < 480 && 480 % streamlength_sample == 0) || (streamlength_sample > 480 && streamlength_sample % 480 == 0)) {
            streamlength_sample = 480;
        }
        support_ndex = Audio_PLC_Support_Api(streamlength_sample);
    }
    return support_ndex;
}

void Audio_PLC_Init_Parameter(VOID* para, AUDIO_PLC_SCRATCH_PTR_t audio_plc_feature_ptr, U16 streamlength_sample, U32 ch_num, stream_samplerate_t samepling_rate)
{
    U32 support_ndex = Audio_PLC_Check_Support_Api(streamlength_sample);
    U32 rate = audio_plc_fs_converter(samepling_rate);
    U32 buffer_size = 0;
    DSP_ENTRY_PARA_PTR entry_para = (DSP_ENTRY_PARA_PTR)para;
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);

    plc_nvkey.option  = (ch_num > 1) ? 3:1;	// 3(stereo), 1(mono)

    if(support_ndex != INVALID_INDEX) {

        audio_plc_feature_ptr->audio_plc_proc = g_plc_support_para[support_ndex].audio_plc_proc;
        audio_plc_feature_ptr->processing_samples = g_plc_support_para[support_ndex].frame_size;
        plc_nvkey.framesize = g_plc_support_para[support_ndex].frame_size;

#if (AIR_AUDIO_PLC_REMOVE_SHORT_SOUND_ENABLE)
        audio_plc_feature_ptr->burst_mode_lower_th = PLC_COMPENSATION_SAMPLES / (audio_plc_feature_ptr->processing_samples);
        audio_plc_feature_ptr->burst_mode_upper_th = PLC_OBSERVE_BURST_MODE_UPPER_TH_MS / ((audio_plc_feature_ptr->processing_samples * 1000) / rate);
#endif
        if(plc_nvkey.framesize > streamlength_sample) {
            buffer_size = plc_nvkey.framesize * ch_num * sizeof(U32);
            audio_plc_feature_ptr->temp_buffer_p  = DSPMEM_tmalloc(entry_para->DSPTask, buffer_size, stream_ptr);
            DSP_MW_LOG_I("[Audio_PLC] need to do buffering, input samples:%d, processing samples:%d, ch_num:%d, malloc buffer size:%d", 4, streamlength_sample, plc_nvkey.framesize, ch_num, buffer_size);
        }

        if(rate > 48000) {
            plc_nvkey.pitch_lag_max = 965*2;
            plc_nvkey.pitch_lag_min = 500*2;
            plc_nvkey.max_period  = 2000;
        }

        if ( (plc_nvkey.pitch_lag_max > plc_nvkey.max_period) || ((plc_nvkey.pitch_lag_min > plc_nvkey.max_period)) ) {
            AUDIO_ASSERT(0 && "[Audio_PLC] max_period must be larger than pitch_lag_max or pitch_lag_min.");
        }

        if(MAX_PLC_MEM_SIZE >= get_plc_memsize(ch_num,(int) plc_nvkey.max_period)&& ch_num <= 2) {
            PLC_Init(audio_plc_feature_ptr->p_plc_mem_ext, &plc_nvkey);
            DSP_MW_LOG_I("[Audio_PLC] init virsion:0x%x, get memsize %d, framesize %d, using PLC_Proc_%d", 4, get_audio_plc_version(), get_plc_memsize(ch_num, plc_nvkey.max_period),plc_nvkey.framesize, g_plc_support_para[support_ndex].frame_size);
        }else{
            DSP_MW_LOG_E("[Audio_PLC] error:memory not enough %d < %d\r\n", 2, MAX_PLC_MEM_SIZE,get_plc_memsize(ch_num, plc_nvkey.max_period));
            AUDIO_ASSERT(0);
        }

    }else{
        DSP_MW_LOG_E("[Audio_PLC] meet unsupported size : %d and it doesn't work",1,streamlength_sample);
        plc_nvkey.framesize = 0xFFFF;
    }

    audio_plc_feature_ptr->init_done = true;
}

ATTR_TEXT_IN_IRAM bool Audio_PLC_Buffering(AUDIO_PLC_SCRATCH_PTR_t audio_plc_feature_ptr, S32 *BFI, void *Buf_L, void *Buf_R, U32 ch_num, U16 streamlength_bytes)
{
    U16 processing_bytes = audio_plc_feature_ptr->processing_samples * sizeof(U32);
    void *start_address = (audio_plc_feature_ptr->temp_buffer_p);

    audio_plc_feature_ptr->temp_bfi |= (*BFI);

    start_address = start_address + audio_plc_feature_ptr->plc_data_in_buffer;

    memcpy(start_address, Buf_L, streamlength_bytes);

    if(ch_num > 1) {
        start_address += processing_bytes;
        memcpy(start_address, Buf_R, streamlength_bytes);
    }

    audio_plc_feature_ptr->plc_data_in_buffer += streamlength_bytes;

    if(audio_plc_feature_ptr->plc_data_in_buffer >= processing_bytes) {

        *BFI = audio_plc_feature_ptr->temp_bfi;
        audio_plc_feature_ptr->temp_bfi = audio_plc_good_frame;

        audio_plc_feature_ptr->plc_data_in_buffer = 0;

        memcpy(Buf_L, (audio_plc_feature_ptr->temp_buffer_p), processing_bytes);

        if(ch_num > 1) {
            memcpy(Buf_R, ((audio_plc_feature_ptr->temp_buffer_p) + processing_bytes), processing_bytes);
        }
        return true;
    }

    return false;
}

#if (AIR_AUDIO_PLC_REMOVE_SHORT_SOUND_ENABLE)
ATTR_TEXT_IN_IRAM BOOL Audio_PLC_Observe_Short_stream(AUDIO_PLC_SCRATCH_PTR_t audio_plc_feature_ptr, S32 *BFI)
{
    if(audio_plc_feature_ptr->mode != AUDIO_PLC_OBSERVE_BURST_MODE || *BFI != audio_plc_good_frame) {
        return true;
    }

    if(!(audio_plc_feature_ptr->audio_plc_check_buffer_level(SHORT_SOUND_BOUNDARY_SAMPLES))) {
        if(audio_plc_feature_ptr->continuous_bf <= audio_plc_feature_ptr->burst_mode_upper_th || audio_plc_feature_ptr->good_frame_is_droped) {
            audio_plc_feature_ptr->good_frame_is_droped = true;
            *BFI = audio_plc_remove_frame;
            DSP_MW_LOG_I("[Audio_PLC] find short stream && continuous_bf = %d. remove it!", 1, audio_plc_feature_ptr->continuous_bf);
            return false;
        } else {
            DSP_MW_LOG_I("[Audio_PLC] find short stream but continuous_bf = %d. disable observe burst mode.", 1, audio_plc_feature_ptr->continuous_bf);
            return true;
        }
    } else {
        audio_plc_feature_ptr->mode = AUDIO_PLC_NORMAL_MODE;
        audio_plc_feature_ptr->good_frame_is_droped = false;
        DSP_MW_LOG_I("[Audio_PLC] disable observe burst mode", 0);
        return true;
    }
}

ATTR_TEXT_IN_IRAM void Audio_PLC_State_Machine(AUDIO_PLC_SCRATCH_PTR_t audio_plc_feature_ptr, S32 *BFI)
{
    switch(audio_plc_feature_ptr->mode) {
        case AUDIO_PLC_NORMAL_MODE:
            if((audio_plc_feature_ptr->continuous_bf >= audio_plc_feature_ptr->burst_mode_lower_th) && (*BFI != audio_plc_good_frame)) {//check and change mode
                audio_plc_feature_ptr->mode = AUDIO_PLC_OBSERVE_BURST_MODE;
                //DSP_MW_LOG_I("[Audio_PLC] enable observe burst mode", 0);
            }
            break;
        case AUDIO_PLC_OBSERVE_BURST_MODE:
            Audio_PLC_Observe_Short_stream(audio_plc_feature_ptr, BFI);
            break;
        default:
            assert(0);
    }

    audio_plc_feature_ptr->continuous_bf = (*BFI != audio_plc_good_frame) ? audio_plc_feature_ptr->continuous_bf + 1 : 0;
}
#endif

ATTR_TEXT_IN_IRAM bool Audio_PLC_Process (VOID* para)
{
    AUDIO_PLC_SCRATCH_PTR_t audio_plc_feature_ptr = stream_function_get_working_buffer(para);
    stream_samplerate_t samepling_rate = stream_function_get_samplingrate(para);
    S32 BFI = (S32)(audio_plc_feature_ptr->audio_plc_get_bfi(para));
    void *Buf_L = stream_function_get_1st_inout_buffer(para);
    void *Buf_R = stream_function_get_2nd_inout_buffer(para);
    U16 streamlength_bytes = DSP_GetStreamCodecOutSize(para);
    U16 streamlength_sample = streamlength_bytes/sizeof(U32);
    U32 processing_num = 0, i = 0, processing_bytes;
    U32 plcProcOutLen = 0;
    U32 ch_num = 2;

    if(!g_audio_plc_ctrl.enable) {
        return FALSE;
    }

    if (stream_function_get_output_resolution(para) == RESOLUTION_16BIT) {
        dsp_converter_16bit_to_32bit(Buf_L, (S16 *)Buf_L, streamlength_bytes >> 1);
        if (ch_num == 2) {
            dsp_converter_16bit_to_32bit(Buf_R, (S16 *)Buf_R, streamlength_bytes >> 1);
        }
        streamlength_bytes = streamlength_bytes << 1;
        streamlength_sample = streamlength_bytes/sizeof(U32);
        stream_function_modify_output_resolution(para, RESOLUTION_32BIT);
    }

    if(audio_plc_feature_ptr->init_done == false && g_audio_plc_ctrl.enable) {
        Audio_PLC_Init_Parameter(para, audio_plc_feature_ptr, streamlength_sample, ch_num, samepling_rate);
        hal_dwt_control_watchpoint(HAL_DWT_0, HAL_DWT_ENABLE);
        U32 ret = hal_dwt_request_watchpoint(HAL_DWT_0, (U32)(&(g_audio_plc_ctrl.enable)), 0x0, WDE_DATA_WO);
        if(ret != 0) {
            assert(0);
        }
    }

    if(audio_plc_feature_ptr->init_done && g_audio_plc_ctrl.enable && audio_plc_feature_ptr->processing_samples) {

#ifdef AIR_AUDIO_DUMP_ENABLE
#if(AIR_AUDIO_PLC_DEBUG_CONVERT_TO_16BIT_ENABLE)
        dsp_converter_32bit_to_16bit(temp_buffer, (S32 *)Buf_L, streamlength_bytes);
        LOG_AUDIO_DUMP((U8*)temp_buffer, streamlength_bytes >> 1, AUDIO_PLC_IN_L);
#else
        LOG_AUDIO_DUMP((U8*)Buf_L, streamlength_bytes, AUDIO_PLC_IN_L);
        LOG_AUDIO_DUMP((U8*)Buf_R, streamlength_bytes, AUDIO_PLC_IN_R);
#endif
#endif

        if(audio_plc_feature_ptr->temp_buffer_p && !Audio_PLC_Buffering(audio_plc_feature_ptr, &BFI, Buf_L, Buf_R, ch_num, streamlength_bytes)) {
            stream_function_modify_output_size(para, 0);
            return FALSE;
        }

        processing_num = (streamlength_sample > audio_plc_feature_ptr->processing_samples) ? (streamlength_sample / audio_plc_feature_ptr->processing_samples) : 1;
        processing_bytes = audio_plc_feature_ptr->processing_samples * sizeof(U32);

        for(i = 0; i < processing_num; i++) {
#if (AIR_AUDIO_PLC_REMOVE_SHORT_SOUND_ENABLE)
            if(audio_plc_feature_ptr->audio_plc_check_buffer_level) {
                Audio_PLC_State_Machine(audio_plc_feature_ptr, &BFI);
            }
#endif
            plcProcOutLen += (audio_plc_feature_ptr->audio_plc_proc)(audio_plc_feature_ptr->p_plc_mem_ext, Buf_L + (processing_bytes * i), Buf_R + (processing_bytes * i), BFI);
        }

#ifdef AIR_AUDIO_DUMP_ENABLE
#if(AIR_AUDIO_PLC_DEBUG_CONVERT_TO_16BIT_ENABLE)
        dsp_converter_32bit_to_16bit(temp_buffer,(S32 *)Buf_L,plcProcOutLen);
        LOG_AUDIO_DUMP((U8*)temp_buffer, (plcProcOutLen*sizeof(S16)), AUDIO_PLC_OUT_L);
#else
        LOG_AUDIO_DUMP((U8*)Buf_L, (plcProcOutLen*sizeof(S32)), AUDIO_PLC_OUT_L);
        LOG_AUDIO_DUMP((U8*)Buf_R, (plcProcOutLen*sizeof(S32)), AUDIO_PLC_OUT_R);
#endif
#endif

        stream_function_modify_output_size(para,(plcProcOutLen * sizeof(U32)));
    }else{
        //do nothing
    }

    return FALSE;
}


void Audio_PLC_ctrl(dsp_audio_plc_ctrl_t audio_plc_ctrl)
{
    g_audio_plc_ctrl.enable = audio_plc_ctrl.enable;
    DSP_MW_LOG_I("[Audio_PLC] enable %d\r\n", 1,audio_plc_ctrl.enable);
}

U8 Audio_PLC_get_ctrl_state(void)
{
    return g_audio_plc_ctrl.enable;
}


