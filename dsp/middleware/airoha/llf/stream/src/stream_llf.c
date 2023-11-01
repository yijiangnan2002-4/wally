/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "dsp_buffer.h"
#include "stream_audio_driver.h"
#include "hal_audio_afe_control.h"
#include "hal_audio_afe_define.h"
#include "audio_afe_common.h"
#include "stream_audio_setting.h"
#include "FreeRTOS.h"
#include "dsp_drv_afe.h"
#include "dsp_scenario.h"
#include "common.h"
#include "dsp_dump.h"
#include "dsp_audio_process.h"
#include "stream_llf.h"
#include "dsp_temp.h"
#include "hal_audio_volume.h"
#include "preloader_pisplit.h"
#include "FreeRTOS.h"
#include "hal_audio_message_struct_common.h"
#include "dsp_memory.h"

log_create_module(LLF, PRINT_LEVEL_INFO);


/* Private define ------------------------------------------------------------*/
//#define LLF_FRAME_WORK_DEBUG
#define RLEN(woff, roff, len) ((woff >= roff) ? (woff - roff) :(len - roff + woff))
#define DSP_CALLBACK_SKIP_ALL_PROCESS   (0xFF)


/* Private typedef -----------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
extern BOOL DSP_Callback_Undo(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr);
extern BOOL DSP_Callback_Malloc(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr);
extern BOOL DSP_Callback_Init(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr);
extern BOOL DSP_Callback_ZeroPadding(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr);
extern VOID DSP_Callback_CheckSkipProcess(VOID *ptr);
extern void DSP_CleanUpCallbackOutBuf(DSP_ENTRY_PARA_PTR entry_para);
extern VOID DSP_Callback_ResolutionConfig(DSP_STREAMING_PARA_PTR stream);
BOOL LLF_Callback_Handler(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr);
extern hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask);
extern hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);
extern void DSP_LIB_AUDIO_DUMP(U8 *audio, U32 audio_size, U32 dumpID);
extern U32 hal_llf_get_data_buf_idx(llf_data_type_t type);

extern DSP_STREAMING_PARA LLStreaming[NO_OF_LL_STREAM];
extern dsp_llf_feature_entry_t LLF_feature_entry_table[AUDIO_LLF_TYPE_ALL];
extern uint32_t g_anc_debug_mask;
bool LLF_dma_disable = false;
CONNECTION_IF LLF_if;
bool LLF_music_voice_state[LLF_DL_MIX_TYPE_NUM] = {0};
n9_dsp_share_info_t LLF_share_info = {0};
static SemaphoreHandle_t g_llf_sharebuf_semaphore = NULL;
U32 LLF_anc_bypass_mode;
U32 llf_dl_need_compensation = 0;
U32 llf_no_swap_dl_state = 0;
DSP_STREAMING_PARA LLFStreamingISR[NO_OF_LLF_STREAM];

CALLBACK_STATE_ENTRY LLF_CallbackEntryTable[] = {
    DSP_Callback_Undo,              /*CALLBACK_DISABLE*/
    DSP_Callback_Malloc,            /*CALLBACK_MALLOC*/
    DSP_Callback_Init,              /*CALLBACK_INIT*/
    DSP_Callback_Undo,              /*CALLBACK_SUSPEND*/
    LLF_Callback_Handler,           /*CALLBACK_HANDLER*/
    DSP_Callback_ZeroPadding,       /*CALLBACK_BYPASSHANDLER*/
    DSP_Callback_ZeroPadding,       /*CALLBACK_ZEROPADDING*/
    DSP_Callback_Undo,              /*CALLBACK_WAITEND*/
};

audio_source_pcm_ops_t afe_platform_LLF_ul_ops = {
    .probe      = NULL,//pcm_ul1_probe,
    .open       = NULL,//pcm_ul1_open,
    .close      = NULL,//pcm_ul1_close,
    .hw_params  = NULL,//pcm_ul1_hw_params,
    .trigger    = NULL,//pcm_ul1_trigger,
    .copy       = NULL,//pcm_ul1_copy,
};

audio_sink_pcm_ops_t afe_platform_LLF_dl_ops = {
    .probe      = NULL,//pcm_ul1_probe,
    .open       = NULL,//pcm_ul1_open,
    .close      = NULL,//pcm_ul1_close,
    .hw_params  = NULL,//pcm_ul1_hw_params,
    .trigger    = NULL,//pcm_ul1_trigger,
    .copy       = NULL,//pcm_ul1_copy,
};

#ifndef DSP_ALIGN4
#define     DSP_ALIGN4      ALIGN(4)
#endif



/* Private functions ---------------------------------------------------------*/

extern void hal_llf_init(void);
extern void hal_llf_set_source_address(SOURCE source, U8 channel_number, U32 data_order[LLF_DATA_TYPE_NUM], U8 earbuds_ch);
extern void hal_llf_set_sink_address(SINK sink);
extern void hal_llf_interrupt_ctrl(bool enable);
extern void hal_llf_set_mic_device(llf_data_type_t mic, hal_audio_control_t device, hal_audio_interface_t dev_interface, U8 enable);
extern void hal_llf_set_anc_ff_cal_gain(int16_t anc_ff_cal_gain);
extern void hal_llf_swap_dl(bool no_swap);
extern void hal_llf_sel_dl_channel(bool dl_need_compensation);
extern SOURCE new_source(SOURCE_TYPE SourceType);
extern SINK new_sink(SINK_TYPE SinkType);
extern SOURCE dsp_open_stream_in(mcu2dsp_open_param_p open_param);
extern SINK dsp_open_stream_out(mcu2dsp_open_param_p open_param);
extern void dsp_start_stream_in(mcu2dsp_start_param_p start_param, SOURCE source);
extern void dsp_start_stream_out(mcu2dsp_start_param_p start_param, SINK sink);


void llf_sharebuf_semaphore_create(void)
{
    if (g_llf_sharebuf_semaphore == NULL) {
        g_llf_sharebuf_semaphore = xSemaphoreCreateBinary();
        if (!g_llf_sharebuf_semaphore) {
            AUDIO_ASSERT(0 && "create get_info_semaphore FAIL \n");
        } else {
            xSemaphoreGive(g_llf_sharebuf_semaphore);
        }
    }
}

bool llf_sharebuf_semaphore_take(void)
{
    if (g_llf_sharebuf_semaphore != NULL) {
        return xSemaphoreTake(g_llf_sharebuf_semaphore, portMAX_DELAY);
    }
    return pdFALSE;
}

void llf_sharebuf_semaphore_give(void)
{
    if (g_llf_sharebuf_semaphore != NULL) {
        xSemaphoreGive(g_llf_sharebuf_semaphore);
    }
}

ATTR_TEXT_IN_IRAM U32 SourceSizeAudioLLF(SOURCE source)
{
    BUFFER_INFO_PTR buff_info_ptr = &(source->streamBuffer.BufferInfo);

    U32 writeOffset   = buff_info_ptr->WriteOffset;
    U32 readOffset    = buff_info_ptr->ReadOffset;
    S32 wr_diff       = writeOffset - readOffset;
    U32 length        = buff_info_ptr->length;
    U32 data_size;
    AUDIO_PARAMETER* audio_para_ptr = &(source->param.audio);
    U32 buffer_per_channel_shift = ((audio_para_ptr->channel_num >= 2) && (source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER))
                                   ? 1
                                   : 0;
    U32 audio_frame_size = audio_para_ptr->frame_size;

    data_size = (wr_diff >= 0) ? (wr_diff) >> buffer_per_channel_shift : ((S32)length + wr_diff) >> buffer_per_channel_shift;
    //data_size = (writeOffset >= readOffset) ? (writeOffset - readOffset) >> buffer_per_channel_shift : (length - readOffset + writeOffset) >> buffer_per_channel_shift;
#ifdef LLF_FRAME_WORK_DEBUG
    DSP_LLF_LOG_I("[LLF]DSP SourceSizeAudioLLF full(%d) data_size(0x%x) frame_size(0x%x)\r\n", 3, buff_info_ptr->bBufferIsFull, data_size, audio_frame_size);
#endif
    if ((buff_info_ptr->bBufferIsFull) || (data_size >= audio_frame_size)) { //[CHK this part]
#ifdef LLF_FRAME_WORK_DEBUG
        DSP_LLF_LOG_I("[LLF]SourceSizeAudioLLF size(%d)\r\n", 1, audio_frame_size);
#endif
        return audio_frame_size;
    } else {
        //DSP_LLF_LOG_I("[LLF]SourceSizeAudioLLF 2 size(%d)\r\n", 1, 0);
        return 0;
    }
    return 0;
}

ATTR_TEXT_IN_IRAM VOID SourceDropAudioLLF(SOURCE source, U32 amount)
{
    BUFFER_INFO_PTR buff_info_ptr = &(source->streamBuffer.BufferInfo);
    U32 ReadOffset = buff_info_ptr->ReadOffset;
    U32 BufLen     = buff_info_ptr->length;
    U32 buffer_per_channel_shift = ((source->param.audio.channel_num >= 2) && (source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER))
                                   ? 1
                                   : 0;
    if (amount == 0) {
        return;
    } else if (buff_info_ptr->bBufferIsFull == TRUE) {
        buff_info_ptr->bBufferIsFull = FALSE;
    }

    amount = (amount << buffer_per_channel_shift);
    //buff_info_ptr->ReadOffset = (buff_info_ptr->ReadOffset + amount) % (buff_info_ptr->length);
    ReadOffset += amount;
    if (ReadOffset >= BufLen) {
        ReadOffset -= BufLen;
    }
    buff_info_ptr->ReadOffset = ReadOffset;
#ifdef LLF_FRAME_WORK_DEBUG
    DSP_LLF_LOG_I("[LLF]SourceDropAudioLLF ReadOffset 0x%x\r\n", 1, buff_info_ptr->ReadOffset);
#endif
}

BOOL SourceConfigureAudioLLF(SOURCE source, stream_config_type type, U32 value)
{
    UNUSED(source);
    UNUSED(type);
    UNUSED(value);
    DSP_LLF_LOG_I("[LLF]source configure", 0);
    return TRUE;
}

BOOL SourceCloseAudioLLF(SOURCE source)
{
    UNUSED(source);
    DSP_LLF_LOG_I("[LLF]source close", 0);
    //hal_memory_free_sram(AUDIO_SCENARIO_TYPE_ANC, HAL_AUDIO_AGENT_DEVICE_ANC);
    void* buffer = (void*)source->param.audio.AfeBlkControl.phys_buffer_addr;
    if (buffer) {
        preloader_pisplit_free_memory(buffer);
        source->param.audio.AfeBlkControl.phys_buffer_addr = 0;
    }
    return TRUE;
}

ATTR_TEXT_IN_IRAM BOOL Source_Audio_ReadLLF(SOURCE source, U8 *dst_addr, U32 length)
{
    U32 i, channel_num, channel_sel, dl_ch;
    void** in_ptr;
    BUFFER_INFO_PTR buff_info_ptr = &(source->streamBuffer.BufferInfo);
    U8** buff_startaddr = &(buff_info_ptr->startaddr[0]);
    U32 ReadOffset = buff_info_ptr->ReadOffset;

    U32 *dst_addr_word, *SrcBuf, *SrcBufStart, *SrcCBufEnd;
    U32 k, src_word_size = (length >> 2);;

    U32 SrcCBufSize, UnwrapSize;
    S32 WrapSize;
    S32* data;
    S32 j;
    U32 frame_len =  length >> 2;
    DSP_CALLBACK_PTR callback_ptr = NULL;
    TRANSFORM transform = source->transform;


    if (transform) {
        callback_ptr = DLLT_Callback_Get(source, transform->sink);
        in_ptr = &(callback_ptr->EntryPara.in_ptr[0]);
        dst_addr = in_ptr[0];
        channel_num = callback_ptr->EntryPara.in_channel_num;
        channel_sel = 0;
    } else {
        return FALSE;
    }

    if ((source->buftype != BUFFER_TYPE_INTERLEAVED_BUFFER) || (channel_num == 1)) {
        for (i = 0 ; i < channel_num ; i++) {
#ifdef LLF_FRAME_WORK_DEBUG
            DSP_LLF_LOG_I("[LLF]Source_Audio_ReadLLF, BUFFER_type(%d) ch(%d) dst(0x%x) src(0x%x) length(%d) mute(%d)\r\n", 6, source->buftype, i, dst_addr, source->streamBuffer.BufferInfo.startaddr[channel_sel] + ReadOffset, length, source->param.audio.mute_flag);
#endif
            /* buffer copy*/

            SrcCBufSize     = buff_info_ptr->length >> 2;
            SrcBufStart     = (U32*)buff_startaddr[channel_sel];
            SrcBuf          = SrcBufStart + (ReadOffset >> 2);
            SrcCBufEnd      = SrcBufStart + SrcCBufSize;
            UnwrapSize      = SrcCBufEnd - SrcBuf;
            WrapSize        = src_word_size - UnwrapSize;
            dst_addr_word = (U32*)dst_addr;

            if (WrapSize > 0) {
                for (k = 0; k < UnwrapSize; k++) {
                    *(dst_addr_word + k) = *(SrcBuf + k);
                }

                while ((U32)WrapSize > SrcCBufSize) {
                    for (k = 0; k < SrcCBufSize; k++) {
                        *(dst_addr_word + UnwrapSize + k) = *(SrcBufStart + k);
                    }
                    WrapSize -= SrcCBufSize;
                }

                for (k = 0; k < (U32)WrapSize; k++) {
                    *(dst_addr_word + UnwrapSize + k) = *(SrcBufStart + k);
                }
            } else {
                for (k = 0; k < src_word_size; k++) {
                    *(dst_addr_word + k) = *(SrcBuf + k);
                }
            }

            #if 1 //gain compensation
            data = (S32*)dst_addr;
            for (j = frame_len - 1; j >= 0; j--) {
                *(data+j) = MAX(MIN((S64)(*(data+j) << 1), 2147483647LL), -2147483648LL);//2us
                /* slower - 3us*/
                //                sample_val = *(data + j);
                //                if ((sample_val <= 1073741823) && (sample_val >= -1073741824)) {
                //                    *(data + j) = sample_val << 1;
                //                } else if (sample_val > 1073741823) {
                //                    *(data + j) = 2147483647;
                //                } else if (*(data + j) < -1073741824) {
                //                    *(data + j) = -2147483648;
                //                }
            }
            #endif

            dl_ch = hal_llf_get_data_buf_idx(LLF_DATA_TYPE_MUSIC_VOICE) - 1;
            /* Fill zero packet to prevent UL pop noise */
            if (source->param.audio.mute_flag == TRUE) {
                if (((llf_dl_need_compensation == 0) && (i == dl_ch)) || (i != dl_ch)) {
                memset(dst_addr, 0, length);
            }
            }
            if (g_anc_debug_mask & AUDIO_SWPT_DUMP_MASK) {
                DSP_LIB_AUDIO_DUMP((U8*)SrcBuf, (U32)length, AUDIO_LLF_MIC_IN_0 + i);// 32bits
            }


            dst_addr = in_ptr[++channel_sel];
        }
    }
    return TRUE;
}

VOID Source_Audio_LLF_Interface_Init(SOURCE source)
{
    /* interface init */
    source->sif.SourceSize         = SourceSizeAudioLLF;
    source->sif.SourceDrop         = SourceDropAudioLLF;
    source->sif.SourceConfigure    = SourceConfigureAudioLLF;
    source->sif.SourceClose        = SourceCloseAudioLLF;
    source->sif.SourceReadBuf      = (source_read_buffer_entry)Source_Audio_ReadLLF;
}

void audio_llf_set_ops(void *param)
{
    if (param == NULL) {
        DSP_LLF_LOG_E("[LLF]DSP audio ops parametser invalid\r\n", 0);
        return;
    }
    if (audio_ops_distinguish_audio_sink(param)) {
        SINK sink = param;
        sink->param.audio.ops = (audio_pcm_ops_p)&afe_platform_LLF_dl_ops;
    } else if (audio_ops_distinguish_audio_source(param)) {
        SOURCE source = param;
        source->param.audio.ops = (audio_pcm_ops_p)&afe_platform_LLF_ul_ops;
    }
}

void Source_LLF_Buffer_Ctrl(SOURCE source, BOOL ctrl, U8 earbuds_ch, U32 data_order[])
{
    U16 i;
    U8 *mem_ptr;
    U32 mem_size;
    int32_t buf_ch;

    if (ctrl) {
//        DSP_LLF_LOG_I("[LLF]Source_LLF_Buffer_Ctrl, frame_size:%d, buff_len:%d, channel_num:%d", 3, source->param.audio.frame_size, source->param.audio.buffer_size, source->param.audio.channel_num);

        buf_ch = source->param.audio.channel_num;

        if (buf_ch > 0) {
            mem_size = buf_ch * source->param.audio.buffer_size;
            source->streamBuffer.BufferInfo.length = source->param.audio.buffer_size;

            //source->param.audio.AfeBlkControl.phys_buffer_addr = hal_memory_allocate_sram(AUDIO_SCENARIO_TYPE_ANC, HAL_AUDIO_AGENT_DEVICE_ANC, mem_size);
            source->param.audio.AfeBlkControl.phys_buffer_addr = (U32)preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, mem_size);

//            DSP_LLF_LOG_I("[LLF]audio source buffer addr(0x%x) size(%u)", 2, source->param.audio.AfeBlkControl.phys_buffer_addr, mem_size);
            mem_ptr = (U8*)source->param.audio.AfeBlkControl.phys_buffer_addr;
            memset(mem_ptr, 0, mem_size);

            for (i = 0; ((i<buf_ch) && (i<BUFFER_INFO_CH_NUM)); i++) {
                if (!source->streamBuffer.BufferInfo.startaddr[i]) {
                    source->streamBuffer.BufferInfo.startaddr[i] = mem_ptr;
                    mem_ptr += source->streamBuffer.BufferInfo.length;
                }
            }

            hal_llf_set_source_address(source, buf_ch, data_order, earbuds_ch);

        }
    } else {

    }
}

/* Public functions ----------------------------------------------------------*/
SOURCE StreamAudioLLFSource(void *param)
{
    SOURCE source = NULL;
    SOURCE_TYPE source_type = SOURCE_TYPE_LLF;
    //U32 input_data_order[LLF_DATA_TYPE_NUM];

    if (Source_blks[source_type] != NULL) {
        DSP_LLF_LOG_I("[LLF]source exit",0);
        return Source_blks[source_type];
    }
    source = new_source(source_type);
    if (source == NULL) {
        return NULL;
    }

    source->type = source_type;
    source->buftype                 = BUFFER_TYPE_CIRCULAR_BUFFER;
    source->taskId                  = DLL_TASK_ID;
    llf_open_param_p open_param = NULL;
    AUDIO_PARAMETER *pAudPara = &source->param.audio;
    if (param) {
        open_param = (llf_open_param_p)param;
        DSP_LLF_LOG_I("[LLF]source(type%d) open_param frame_size:%d, frame_num:%d, resolution:%d, device(%d %d %d), interface(%d %d %d)\r\n", 10, SOURCE_TYPE_LLF, open_param->frame_size, open_param->frame_number, open_param->format, open_param->audio_device[LLF_DATA_TYPE_REAR_L], open_param->audio_device[LLF_DATA_TYPE_INEAR_L], open_param->audio_device[LLF_DATA_TYPE_TALK], open_param->audio_interface[LLF_DATA_TYPE_REAR_L], open_param->audio_interface[LLF_DATA_TYPE_INEAR_L], open_param->audio_interface[LLF_DATA_TYPE_TALK]);
        pAudPara->format_bytes   = 4;//U24
        pAudPara->format         = HAL_AUDIO_PCM_FORMAT_S32_LE;
        pAudPara->frame_size     = open_param->frame_size * pAudPara->format_bytes;
        pAudPara->channel_num    = open_param->channel_num;
        pAudPara->buffer_size    = open_param->frame_size * open_param->frame_number * pAudPara->format_bytes;
        pAudPara->rate           = 50000;//fixed
        pAudPara->src_rate       = 50000; //fixed
        pAudPara->count          = open_param->frame_size;
        //pAudPara->period         = open_param->frame_size / (pAudPara->rate /1000);//16/50
        pAudPara->echo_reference = (open_param->echo_reference[0] | open_param->echo_reference[1]) ? true : false;

        if (open_param->format > HAL_AUDIO_PCM_FORMAT_U16_BE) {
            stream_feature_configure_resolution((stream_feature_list_ptr_t)LLF_if.pfeature_table, RESOLUTION_32BIT, 0);
        }

        for (U32 i = 0; i < LLF_DATA_TYPE_MIC_NUM; i++) {
            if (open_param->audio_device[i]) {
                hal_llf_set_mic_device(LLF_DATA_TYPE_REAR_L + i, (hal_audio_control_t)open_param->audio_device[i], (hal_audio_interface_t)open_param->audio_interface[i], open_param->audio_device_enable[i]);
            }
        }
        hal_llf_set_anc_ff_cal_gain(open_param->anc_ff_cal_gain);
        llf_dl_need_compensation = open_param->music_need_compensation;
        llf_no_swap_dl_state = !llf_dl_need_compensation;
        //hal_llf_sel_dl_channel(open_param->music_need_compensation);

    } else {
        DSP_LLF_LOG_E("[LLF]source open param NULL!", 0);
    }
    DSP_LLF_LOG_I("[LLF]source frame_size:%d, buff_len:%d, channel_num:%d\r\n", 3, pAudPara->frame_size, pAudPara->buffer_size, pAudPara->channel_num);

    audio_llf_set_ops(source);
    Source_LLF_Buffer_Ctrl(source, TRUE, open_param->earbuds_ch, open_param->in_data_order);
    Source_Audio_LLF_Interface_Init(source);

    LLF_share_info.start_addr = hal_memview_cm4_to_dsp0(open_param->share_info.start_addr);
    LLF_share_info.length = open_param->share_info.length;
    return source;
}

SOURCE dsp_open_stream_in_LLF(mcu2dsp_open_param_p open_param)
{
    SOURCE source;

    //DSP_LLF_LOG_I("[LLF]Stream in aduio LLF\r\n", 0);

    source = StreamAudioLLFSource(&(open_param->stream_in_param));
    if (source) {
        //DSP_LLF_LOG_I("[LLF]DSP source create successfully\r\n", 0);
    } else {
        DSP_LLF_LOG_E("[LLF]DSP source create fail\r\n", 0);
    }

    return source;
}

ATTR_TEXT_IN_IRAM U32 SinkSizeAudioLLF(SINK sink)
{
    BUFFER_INFO_PTR buff_info_ptr = &(sink->streamBuffer.BufferInfo);

    U32 writeOffset = buff_info_ptr->WriteOffset;
    U32 readOffset  = buff_info_ptr->ReadOffset;
    S32 wr_diff     = writeOffset - readOffset;
    U32 length      = buff_info_ptr->length;
    if ((buff_info_ptr->bBufferIsFull == FALSE) && (Audio_setting->Audio_sink.Pause_Flag == FALSE))
    {
#ifdef LLF_FRAME_WORK_DEBUG
        DSP_LLF_LOG_I("[LLF]SinkSizeAudioLLF, sink size(%d)", 1, writeOffset >= readOffset ? (length + readOffset - writeOffset) : (readOffset - writeOffset));
#endif
        return wr_diff >= 0 ? (length - (U32)wr_diff) : (readOffset - writeOffset);
    }
    else
    {
#ifdef LLF_FRAME_WORK_DEBUG
        DSP_LLF_LOG_I("[LLF]SinkSizeAudioLLF, sink size(0)", 0);
#endif
        return 0;

    }
}

ATTR_TEXT_IN_IRAM BOOL SinkFlushAudioLLF(SINK sink, U32 amount)
{
    BUFFER_INFO_PTR buff_info_ptr = &(sink->streamBuffer.BufferInfo);
    U32 WriteOffset = buff_info_ptr->WriteOffset;
    U32 BufLen = buff_info_ptr->length;
    U32 buffer_per_channel_shift = ((sink->param.audio.channel_num>=2) && (sink->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER ))
                                     ? 1
                                     : 0;
    if ((amount >= 0) && (SinkSizeAudioLLF(sink) >= amount)) {
        //    buff_info_ptr->WriteOffset
        //            = (buff_info_ptr->WriteOffset + (amount<<buffer_per_channel_shift))
        //                % (buff_info_ptr->length);
        WriteOffset += amount << buffer_per_channel_shift;
        if (WriteOffset > BufLen) {
            WriteOffset -= BufLen;
        }
        buff_info_ptr->WriteOffset = WriteOffset;

        if (WriteOffset == buff_info_ptr->ReadOffset){
            buff_info_ptr->bBufferIsFull = TRUE;
        }
#ifdef LLF_FRAME_WORK_DEBUG
        DSP_LLF_LOG_I("[LLF]SinkFlushAudioLLF, WriteOffset:0x%x, amount:0x%x, buffer_per_channel_shift:%d\r\n", 3, buff_info_ptr->WriteOffset, amount, buffer_per_channel_shift);
#endif
    } else {
        return FALSE;
    }
    return true;
}

U32 SinkClaimAudioLLF(SINK sink, U32 extra)
{
    UNUSED(sink);
    UNUSED(extra);
    return 0;
}


BOOL SinkConfigureAudioLLF(SINK sink, stream_config_type type, U32 value)
{
    UNUSED(sink);
    UNUSED(type);
    UNUSED(value);
    return true;
}

U8* SinkMapAudioLLF(SINK sink)
{
    UNUSED(sink);
    return NULL;
}

BOOL SinkCloseAudioLLF(SINK sink)
{
    UNUSED(sink);
    DSP_LLF_LOG_I("[LLF]Sink Close\r\n", 0);
    //hal_memory_free_sram(AUDIO_SCENARIO_TYPE_SIDETONE, HAL_AUDIO_AGENT_DEVICE_SIDETONE);
    void* buffer = (void*)sink->param.audio.AfeBlkControl.phys_buffer_addr;
    if (buffer) {
        preloader_pisplit_free_memory(buffer);
        sink->param.audio.AfeBlkControl.phys_buffer_addr = 0;
    }
    return true;
}

ATTR_TEXT_IN_IRAM BOOL Sink_Audio_WriteLLF(SINK sink, U8 *src_addr, U32 length)
{
    U32 i, channel_num, channel_sel;
    void** out_ptr;
    BUFFER_INFO_PTR buff_info_ptr = &(sink->streamBuffer.BufferInfo);
    U8** buff_startaddr = &(buff_info_ptr->startaddr[0]);
    U32 writeOffset = buff_info_ptr->WriteOffset;

    U32 *src_addr_word, *DestCBufStart, *DestCBufEnd, *DestBuf;
    U32 k, sink_word_size = length >> 2;

    U32 DestCBufSize, UnwrapSize;
    S32 WrapSize;
    TRANSFORM transform = sink->transform;
    DSP_CALLBACK_PTR callback_ptr = NULL;

    if (transform) {
        callback_ptr = DLLT_Callback_Get(transform->source, sink);
        out_ptr = &(callback_ptr->EntryPara.out_ptr[0]);
        src_addr = out_ptr[0];
        channel_num = sink->param.audio.channel_num;
        channel_sel = 0;
    } else {
        return false;
    }

    if (sink->param.audio.mute_flag) {
        memset(buff_startaddr[0], 0, buff_info_ptr->length * channel_num);
    } else {

        if ((sink->buftype != BUFFER_TYPE_INTERLEAVED_BUFFER) || (channel_num==1)) {
            for (i = 0; i < channel_num; i++) {
#ifdef LLF_FRAME_WORK_DEBUG
                DSP_LLF_LOG_I("[LLF]Sink_Audio_WriteLLF, BUFFER_type(%d) ch(%d) dst(0x%x) src(0x%x) length(%d) mute(%d)\r\n", 6, sink->buftype, i, sink->streamBuffer.BufferInfo.startaddr[channel_sel] + writeOffset, src_addr, length, Audio_setting->Audio_sink.Mute_Flag);
#endif
    //            DSP_D2C_BufferCopy(buff_startaddr[channel_sel] + writeOffset,
    //                               src_addr,
    //                               length,
    //                               buff_startaddr[channel_sel],
    //                               buff_info_ptr->length);

                /*buffer copy*/

                DestCBufStart = (U32*)buff_startaddr[channel_sel];
                DestCBufSize  = buff_info_ptr->length >> 2;
                DestCBufEnd   = DestCBufStart + DestCBufSize;
                DestBuf       = DestCBufStart + (writeOffset >> 2);
                UnwrapSize    = DestCBufEnd - DestBuf;
                WrapSize      = sink_word_size - UnwrapSize;
                src_addr_word = (U32*)src_addr;

                if (WrapSize > 0) {
                    for (k = 0; k < UnwrapSize; k++) {
                        *(DestBuf + k) = *(src_addr_word + k);
                    }

                    while (WrapSize > (S32)DestCBufSize) {
                        for (k = 0; k < DestCBufSize; k++) {
                            *(DestCBufStart + k) = *(src_addr_word + UnwrapSize + k);
                        }
                        WrapSize -= DestCBufSize;
                    }

                    for (k = 0; k < (U32)WrapSize; k++) {
                        *(DestCBufStart + k) = *(src_addr_word + UnwrapSize + k);
                    }
                } else {
                    for (k = 0; k < sink_word_size; k++) {
                        *(DestBuf + k) = *(src_addr_word + k);
                    }
                }

                if (g_anc_debug_mask & AUDIO_SWPT_DUMP_MASK) {
                    DSP_LIB_AUDIO_DUMP((U8*)src_addr, length, AUDIO_LLF_OUT_L);
                }

                src_addr = out_ptr[++channel_sel];
            }

        } else {

        }
    }
    return TRUE;
}

VOID Sink_Audio_LLF_Interface_Init(SINK sink)
{
    /* interface init */
    sink->sif.SinkSlack         = SinkSizeAudioLLF;
    sink->sif.SinkFlush         = SinkFlushAudioLLF;

    sink->sif.SinkConfigure     = SinkConfigureAudioLLF;
    sink->sif.SinkClaim         = SinkClaimAudioLLF;
    sink->sif.SinkMap           = SinkMapAudioLLF;
    sink->sif.SinkClose         = SinkCloseAudioLLF;
    sink->sif.SinkWriteBuf      = (sink_write_buffer_entry)Sink_Audio_WriteLLF;
}

void Sink_LLF_Buffer_Ctrl(SINK sink, BOOL ctrl)
{
    U8 *mem_ptr;
    U32 mem_size;
    int32_t buf_ch;
    U16 i;

    if (ctrl) {
//        DSP_LLF_LOG_I("[LLF]Sink_LLF_Buffer_Ctrl, frame_size:%d, buff_len:%d, channel_num:%d", 3, sink->param.audio.frame_size, sink->param.audio.buffer_size, sink->param.audio.channel_num);

        buf_ch = sink->param.audio.channel_num;

        if (buf_ch > 0) {
            mem_size = buf_ch * sink->param.audio.buffer_size;
            sink->streamBuffer.BufferInfo.length = sink->param.audio.buffer_size;

            //sink->param.audio.AfeBlkControl.phys_buffer_addr = hal_memory_allocate_sram(AUDIO_SCENARIO_TYPE_SIDETONE, HAL_AUDIO_AGENT_DEVICE_SIDETONE, mem_size);
            sink->param.audio.AfeBlkControl.phys_buffer_addr = (U32)preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, mem_size);

//            DSP_LLF_LOG_I("[LLF]audio sink buffer addr(0x%x) size(%u)", 2, sink->param.audio.AfeBlkControl.phys_buffer_addr, mem_size);
            mem_ptr = (U8*)sink->param.audio.AfeBlkControl.phys_buffer_addr;
            memset(mem_ptr, 0, mem_size);

            for (i = 0; ((i<buf_ch) && (i<BUFFER_INFO_CH_NUM)); i++) {
                if (!sink->streamBuffer.BufferInfo.startaddr[i]) {
                    sink->streamBuffer.BufferInfo.startaddr[i] = mem_ptr;
                    mem_ptr += sink->streamBuffer.BufferInfo.length;
                }
            }

            hal_llf_set_sink_address(sink);

        }
        /* At now the sink is not started, so we can add prefill data here */
        U32 output_prefill_sample = sink->param.audio.count << 1;
        U32 output_prefill = output_prefill_sample * sink->param.audio.format_bytes * sink->param.audio.channel_num;
        DSP_LLF_LOG_I("[LLF] sink afe buffer prefill size: %d (samples),ch:%d", 2, output_prefill_sample - sink->param.audio.count, sink->param.audio.channel_num);
        sink->streamBuffer.BufferInfo.ReadOffset = (0) % sink->streamBuffer.BufferInfo.length;
        sink->streamBuffer.BufferInfo.WriteOffset = (0 + output_prefill) % sink->streamBuffer.BufferInfo.length;
    } else {

    }
}

SINK StreamAudioLLFSink(void *param)
{
    SINK sink = NULL;
    SINK_TYPE sink_type = SINK_TYPE_LLF;

    if (Sink_blks[sink_type] != NULL) {
        DSP_LLF_LOG_I("[LLF]sink exit",0);
        return Sink_blks[sink_type];
    }

    sink = new_sink(SINK_TYPE_LLF);
    if (sink == NULL) {
        return sink;
    }
    sink->taskid                    = DHP_TASK_ID;
    sink->type                      = sink_type;
    sink->buftype                   = BUFFER_TYPE_INTERLEAVED_BUFFER;

    llf_open_param_p open_param = NULL;
    AUDIO_PARAMETER *pAudPara = &sink->param.audio;
    if (param) {
        open_param = (llf_open_param_p)param;
        DSP_LLF_LOG_I("[LLF]sink type(%d) open_param frame_size:%d, frame_num:%d\r\n", 3, SINK_TYPE_LLF, open_param->frame_size, open_param->frame_number);
        pAudPara->format_bytes   = 4;
        pAudPara->format         = HAL_AUDIO_PCM_FORMAT_S32_LE;
        pAudPara->channel_num    = 1;
        pAudPara->frame_size     = open_param->frame_size * pAudPara->format_bytes;
        pAudPara->buffer_size    = open_param->frame_size * open_param->frame_number * pAudPara->format_bytes;
        pAudPara->rate           = 50000;
        pAudPara->src_rate       = 50000;
        pAudPara->count          = open_param->frame_size;

    } else {
        DSP_LLF_LOG_E("[LLF]sink open param NULL!", 0);
    }

    audio_llf_set_ops(sink);
    Sink_LLF_Buffer_Ctrl(sink, TRUE);
    Sink_Audio_LLF_Interface_Init(sink);
    return sink;
}

SINK dsp_open_stream_out_LLF(mcu2dsp_open_param_p open_param)
{
    SINK sink;

//    DSP_LLF_LOG_I("[LLF]Stream out aduio LLF\r\n", 0);

    sink = StreamAudioLLFSink(&(open_param->stream_out_param));
    if (sink != NULL) {
//        DSP_LLF_LOG_I("[LLF]DSP sink create successfully\r\n", 0);
    } else {
        DSP_LLF_LOG_E("[LLF]DSP sink create fail\r\n", 0);
    }
    return sink;
}

void port_dsp_llf_feature_init(llf_type_t scenario_id, U32 sub_id, SOURCE source, SINK sink)
{
    AUDIO_PARAMETER *pAudPara = &source->param.audio;
    pAudPara->scenario_id = (U32)scenario_id;
    pAudPara->scenario_sub_id = sub_id;
    sink->param.audio.scenario_id = pAudPara->scenario_id;
    sink->param.audio.scenario_sub_id = pAudPara->scenario_sub_id;


    if (scenario_id < AUDIO_LLF_TYPE_ALL) {
        if (LLF_feature_entry_table[scenario_id].init_entry) {
            audio_llf_stream_ctrl_t ctrl;
            ctrl.type = scenario_id;
            ctrl.sub_mode = sub_id;
            ctrl.channel_num = source->param.audio.channel_num;
            ctrl.frame_size = source->param.audio.frame_size;
            ctrl.source = (void*)source;
            LLF_feature_entry_table[scenario_id].init_entry(&ctrl);
        }
    }

}

void port_dsp_llf_feature_deinit(SOURCE source, SINK sink)
{
    llf_type_t scenario_id = source->param.audio.scenario_id;
    UNUSED(sink);

    if (scenario_id < AUDIO_LLF_TYPE_ALL) {
        if (LLF_feature_entry_table[scenario_id].deinit_entry) {
            LLF_feature_entry_table[scenario_id].deinit_entry(source, sink);
        }
    }
}

stream_feature_list_t* port_dsp_llf_feature_get_list(llf_type_t scenario_id, U32 sub_id)
{
    if (scenario_id < AUDIO_LLF_TYPE_ALL) {
        if (LLF_feature_entry_table[scenario_id].feature_list_entry) {
            return LLF_feature_entry_table[scenario_id].feature_list_entry(scenario_id, sub_id);
        }
    }
    return NULL;
}


void port_dsp_llf_suspend(SOURCE source, SINK sink)
{
    llf_type_t scenario_id = source->param.audio.scenario_id;

    if (scenario_id < AUDIO_LLF_TYPE_ALL) {
        if (LLF_feature_entry_table[scenario_id].suspend_entry) {
            audio_llf_stream_ctrl_t ctrl;
            ctrl.type = scenario_id;
            ctrl.sub_mode = source->param.audio.scenario_sub_id;
            ctrl.channel_num = source->param.audio.channel_num;
            ctrl.source = (void*)source;
            ctrl.sink = (void*)sink;
            LLF_feature_entry_table[scenario_id].suspend_entry(&ctrl);
        }
    }
}

void port_dsp_llf_resume(SOURCE source, SINK sink)
{
    llf_type_t scenario_id = source->param.audio.scenario_id;

    if (scenario_id < AUDIO_LLF_TYPE_ALL) {
        if (LLF_feature_entry_table[scenario_id].resume_entry) {
            audio_llf_stream_ctrl_t ctrl;
            ctrl.type = scenario_id;
            ctrl.sub_mode = source->param.audio.scenario_sub_id;
            ctrl.channel_num = source->param.audio.channel_num;
            ctrl.source = (void*)source;
            ctrl.sink = (void*)sink;
            LLF_feature_entry_table[scenario_id].resume_entry(&ctrl);
        }
    }
}

void port_dsp_llf_runtime_config(audio_llf_runtime_config_t *param)
{
    if (param->type < AUDIO_LLF_TYPE_ALL) {
        if (LLF_feature_entry_table[param->type].runtime_config_entry) {
            LLF_feature_entry_table[param->type].runtime_config_entry(param);
        }
    }
}

ATTR_TEXT_IN_IRAM BOOL dsp_llf_stream_handler(SOURCE source, SINK sink)
{
    U32 length_src, length_snk, length;

    DSP_CALLBACK_PTR callback_ptr = DLLT_Callback_Get(source, sink);
    DSP_STREAMING_PARA_PTR pStream = NULL;

    if (callback_ptr != NULL) {
        pStream = DSP_CONTAINER_OF(callback_ptr, DSP_STREAMING_PARA, callback);
    }

    if ((source == NULL) || (sink == NULL) || (pStream == NULL)) {
        return FALSE;
    } else if ((callback_ptr != NULL) && ((callback_ptr->Status != CALLBACK_SUSPEND) && (callback_ptr->Status != CALLBACK_WAITEND))) {
        return FALSE;
    } else if (pStream->streamingStatus == STREAMING_END) {
        return FALSE;
    }

    length_src = SourceSizeAudioLLF(source);
    length_snk = SinkSizeAudioLLF(sink);
    length = MIN(length_src, length_snk);

    if (length == 0) {
        return FALSE;
    }

    if (callback_ptr->Status == CALLBACK_SUSPEND) {
        callback_ptr->Status = CALLBACK_HANDLER;
    } else if ((callback_ptr->Status == CALLBACK_WAITEND) && (length != 0)) {
        callback_ptr->Status = CALLBACK_HANDLER;
    }

    return TRUE;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 BOOL LLF_Callback_Handler(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr)
{
    BOOL result = FALSE;
    entry_para->number.field.process_sequence = 1;
    entry_para->codec_out_size = entry_para->pre_codec_out_size;
    entry_para->resolution.process_res = entry_para->resolution.source_in_res;

    if (feature_table_ptr != NULL) {
        while (feature_table_ptr->ProcessEntry != NULL) {
            entry_para->mem_ptr = feature_table_ptr->MemPtr;
            if (!(entry_para->skip_process == entry_para->number.field.process_sequence)) {

                if ((feature_table_ptr->ProcessEntry != stream_pcm_copy_process) || (entry_para->resolution.feature_res == RESOLUTION_16BIT)) {//skip pcm cpy to save mips

                    if ((feature_table_ptr->ProcessEntry(entry_para))) {
                        DSP_CleanUpCallbackOutBuf(entry_para);
                        DSP_MW_LOG_I("handler return true", 0);
                        result = TRUE;
                        break;
                    }
                }

            } else {
                DSP_CleanUpCallbackOutBuf(entry_para);
            }
            if (feature_table_ptr->ProcessEntry == stream_function_end_process) {
                break;
            }
            entry_para->number.field.process_sequence++;
            feature_table_ptr++;
        }
    } else {
        stream_pcm_copy_process(entry_para);
        result = TRUE;
    }
    return result;
}

ATTR_TEXT_IN_IRAM VOID dsp_llf_callback_processing(DSP_STREAMING_PARA_PTR stream)
{
    volatile DSP_CALLBACK_STATUS *cb_status_ptr = &stream->callback.Status;
    DSP_CALLBACK_STATUS handlingStatus = *cb_status_ptr, nextStatus;
    DSP_ENTRY_PARA_PTR cb_entry_para_ptr = &stream->callback.EntryPara;
    SOURCE source = stream->source;
    SINK sink = stream->sink;
    U32 in_size, in_malloc_size, remainSize, sink_size = 0;
    U16 *callbackOutSizePtr = &(stream->callback.EntryPara.codec_out_size);
    U32 callbackOutSize = *callbackOutSizePtr;
    DSP_STREAMING_STATUS_PTR streamingStatus_ptr = &stream->streamingStatus;
    DSP_STREAMING_STATUS streamingStatus = *streamingStatus_ptr;
    DSP_FEATURE_TABLE_PTR FeatureTablePtr = stream->callback.FeatureTablePtr;

    if (handlingStatus == CALLBACK_HANDLER) {
        cb_entry_para_ptr->in_size = SourceSizeAudioLLF(source);
        in_size = cb_entry_para_ptr->in_size;
        in_malloc_size = cb_entry_para_ptr->in_malloc_size;

        if (in_size > in_malloc_size) {
            cb_entry_para_ptr->in_size = in_malloc_size;
            in_size = in_malloc_size;
        }

        sink_size = SinkSizeAudioLLF(sink);
        if (in_size != 0) {
            if (sink_size >= callbackOutSize) {
                Source_Audio_ReadLLF(source, NULL, in_size);
            }
        } else {
            DSP_LLF_LOG_I("[LLF] Callback meet source size 0", 0);
        }

        if ((streamingStatus == STREAMING_START) || (streamingStatus == STREAMING_DEINIT)) {
            *streamingStatus_ptr = STREAMING_ACTIVE;
            //handlingStatus = CALLBACK_INIT;

        } else if (streamingStatus == STREAMING_END) {
            handlingStatus = CALLBACK_INIT;
        }

    }/* else if (handlingStatus == CALLBACK_INIT) {
        //cb_entry_para_ptr->in_size = 0;
    }*/


    cb_entry_para_ptr->skip_process = 0;
    if (handlingStatus == CALLBACK_HANDLER) {
        if (callbackOutSize > sink_size) {
            cb_entry_para_ptr->in_size = 0;
            *callbackOutSizePtr = 0;
            callbackOutSize = 0;
            cb_entry_para_ptr->skip_process = DSP_CALLBACK_SKIP_ALL_PROCESS;
        } else if (in_size == 0) {
            if (FeatureTablePtr->ProcessEntry != stream_pcm_copy_process) {
                cb_entry_para_ptr->skip_process = CODEC_ALLOW_SEQUENCE;
            }
        }
    }
    cb_entry_para_ptr->force_resume = FALSE;
    if (!(cb_entry_para_ptr->skip_process == DSP_CALLBACK_SKIP_ALL_PROCESS)) {
        LLF_CallbackEntryTable[handlingStatus](cb_entry_para_ptr, FeatureTablePtr);
    }

    if (handlingStatus == CALLBACK_HANDLER) {
        if (callbackOutSize != 0) {
            remainSize = callbackOutSize;
            while (remainSize > 0) {
                while (sink_size == 0) {
                    vTaskSuspend(cb_entry_para_ptr->DSPTask);
                }

                if (remainSize <= sink_size) {
                    sink_size = remainSize;
                }
                Sink_Audio_WriteLLF(sink, NULL, sink_size);
                SinkFlushAudioLLF(sink, sink_size);

                remainSize -= sink_size;
            }
        }

        //Source Drop
        if (in_size != 0) {
            SourceDropAudioLLF(source, in_size);
        }

        nextStatus = CALLBACK_SUSPEND;
    } else if (handlingStatus == CALLBACK_INIT) {
        DSP_Callback_ResolutionConfig(stream);
        nextStatus = CALLBACK_SUSPEND;

    } else if (handlingStatus == CALLBACK_MALLOC) {
        nextStatus = CALLBACK_INIT;
    } else {
        //CALLBACK_SUSPEND / CALLBACK_DISABLE / CALLBACK_WAITEND
        return;
    }


    U32 mask;
    hal_nvic_save_and_set_interrupt_mask_special(&mask);

    //DSP_Callback_ChangeStatus
    stream->callback.Status = nextStatus;

    if ((nextStatus == CALLBACK_SUSPEND) || (nextStatus == CALLBACK_INIT)) {
        source->transform->Handler(source, sink);
    }
    hal_nvic_restore_interrupt_mask_special(mask);
}

void dsp_llf_callback_feature_config(DSP_STREAMING_PARA_PTR stream, stream_feature_list_ptr_t feature_list_ptr, bool is_enable)
{
    U32  featureEntrySize;
    stream_feature_type_ptr_t  featureTypePtr;
    VOID*  mem_ptr;
    U32 i, featureEntryNum = 0;
    U32 codecOutSize, codecOutResolution;
    U32 mallocSize, chNum, frameSize = 0;

    if (is_enable) {
        stream->callback.EntryPara.DSPTask = DLL_TASK_ID;

        if ((featureTypePtr = stream->callback.EntryPara.feature_ptr = feature_list_ptr)!= NULL) {
            for(featureEntryNum = 1 ; *(featureTypePtr) != FUNC_END ; ++featureTypePtr) {
                featureEntryNum++;
            }
        }
        featureEntrySize = featureEntryNum * sizeof(DSP_FEATURE_TABLE);
        stream->callback.EntryPara.number.field.feature_number = featureEntryNum;

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        stream->callback.EntryPara.scenario_type = stream->source->scenario_type;
        GroupMemoryInfo_t *fw_memory_addr;
        SubComponentType_t subcomponent_type_of_fw;
        DspMemoryManagerReturnStatus_t memory_return_status;

        subcomponent_type_of_fw = dsp_memory_get_fw_subcomponent_by_scenario(stream->callback.EntryPara.scenario_type);
        fw_memory_addr = dsp_memory_get_fw_memory_info_by_scenario(stream->callback.EntryPara.scenario_type);
        memory_return_status = DspMemoryManager_AcquireGroupMemory(dsp_memory_get_component_by_scenario(stream->callback.EntryPara.scenario_type), subcomponent_type_of_fw, fw_memory_addr);
        if (memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK) {
            DSP_LLF_LOG_E("DSP callback FeatureConfig FW malloc FAIL (%d) !!!", 1, memory_return_status);

            assert(0);
        }
        mem_ptr = fw_memory_addr->DramAddr;
#else
        //DSP_LLF_LOG_I("[DSP_RESOURCE][DSP_Callback_FeatureConfig] featureEntrySize:%d", 1, featureEntrySize);
        DSP_FEATURE_TABLE_PTR featurePtr;
        mem_ptr = DSPMEM_tmalloc(stream->callback.EntryPara.DSPTask, featureEntrySize, stream);
#endif

        if (featureEntrySize > 0) {
            stream->callback.FeatureTablePtr  = mem_ptr;
            featureTypePtr = feature_list_ptr;
            for(i = 0 ; i < featureEntryNum ; i++) {
#ifndef AIR_DSP_MEMORY_REGION_ENABLE
                featurePtr = (DSP_FEATURE_TABLE_PTR)&stream_feature_table[((U32)(*(featureTypePtr)&0xFF))];
                *(stream->callback.FeatureTablePtr + i) = *featurePtr;
                if (i == 0 || i == featureEntryNum - 2) {
                    if ((*(featureTypePtr) & 0xFFFF0000) != 0) {
                        codecOutSize = (*(featureTypePtr)) >> 16;
                        ((stream->callback.FeatureTablePtr + i)->MemPtr) = (VOID *)codecOutSize;
                    } else {
                        codecOutSize = (U32)((stream->callback.FeatureTablePtr + i)->MemPtr);
                    }
                }
#else
                //Store codec output size at FUNC_END
                if (i==featureEntryNum-1) {
                    if ((*(featureTypePtr)&0xFFFF0000) != 0)
                    {
                        codecOutSize = (*(featureTypePtr))>>16;
                        ((stream->callback.FeatureTablePtr + i)->MemPtr) = (VOID*)codecOutSize;
                    }
                }
#endif
                if (i==0)
                {
                    codecOutResolution = (*(featureTypePtr)&0xFF00)>>8;
                    stream->callback.EntryPara.resolution.feature_res = ((codecOutResolution==RESOLUTION_16BIT) || (codecOutResolution==RESOLUTION_32BIT))
                                                                        ? codecOutResolution
                                                                        : RESOLUTION_16BIT;
                }

                featureTypePtr++;
            }

            //DSP_Callback_ParaSetup
            stream->callback.EntryPara.in_size        = stream->source->param.audio.frame_size;
            stream->callback.EntryPara.in_channel_num = stream->source->param.audio.channel_num;
            stream->callback.EntryPara.in_sampling_rate = 50; //50K

            stream->callback.EntryPara.out_channel_num         = stream->sink->param.audio.channel_num;
            stream->callback.EntryPara.codec_out_size          = stream->sink->param.audio.frame_size;//(U32)(stream->callback.FeatureTablePtr->MemPtr);
            stream->callback.EntryPara.codec_out_sampling_rate = 50;
            frameSize = stream->sink->param.audio.frame_size;

            stream->callback.Src.out_frame_size  = (stream->callback.EntryPara.with_src == 0)
                                                   ? 0
                                                   : stream->callback.Src.out_frame_size;

            //allocte in/out buffer
            stream->callback.EntryPara.out_malloc_size = MAX(MAX(MAX(stream->callback.EntryPara.codec_out_size,
                                                                 stream->callback.EntryPara.encoder_out_size),
                                                             stream->callback.Src.out_frame_size),
                                                         frameSize);
            stream->callback.EntryPara.in_malloc_size = stream->callback.EntryPara.in_size;

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
            GroupMemoryInfo_t *memory_info_ptr;
            SubComponentType_t sub_component_type;
            DspMemoryManagerReturnStatus_t memory_return_status;
#endif
            DSP_LLF_LOG_I("[DSP_RESOURCE] DSP stream in/out buf malloc", 0);
            if (stream->callback.FeatureTablePtr->FeatureType == CODEC_PCM_COPY) {
                DSP_LLF_LOG_I("[DSP] test out_channel_num%d, in_channel_num%d, ",3,
                stream->callback.EntryPara.out_channel_num,stream->callback.EntryPara.in_channel_num,
                MAX(MAX(stream->callback.EntryPara.out_channel_num, 1), stream->callback.EntryPara.in_channel_num));
                chNum = MAX(MAX(stream->callback.EntryPara.out_channel_num, 1), stream->callback.EntryPara.in_channel_num);
                AUDIO_ASSERT(chNum <= LLF_DATA_TYPE_NUM);
                frameSize = MAX(stream->callback.EntryPara.in_malloc_size, stream->callback.EntryPara.out_malloc_size);
                stream->callback.EntryPara.in_malloc_size = stream->callback.EntryPara.out_malloc_size = frameSize;
                stream->callback.EntryPara.out_channel_num = chNum;
                mallocSize = frameSize * chNum;
                if ((frameSize & 3) && (chNum > 1)) {
                    DSP_LLF_LOG_I("[DSP] Unaligned Callback Frame Size:%d!!", 1, frameSize);
                }
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
                sub_component_type = dsp_memory_get_stream_in_subcomponent_by_scenario(stream->callback.EntryPara.scenario_type);
                memory_info_ptr = dsp_memory_get_stream_in_memory_info_by_scenario(stream->callback.EntryPara.scenario_type);
                memory_return_status = DspMemoryManager_AcquireGroupMemory(dsp_memory_get_component_by_scenario(stream->callback.EntryPara.scenario_type), sub_component_type, memory_info_ptr);
                if (memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK) {
                    DSP_LLF_LOG_E("DSP callback ParaSetup In/Out malloc FAIL (%d) !!!", 1, memory_return_status);
                    assert(0);
                }
                if (memory_info_ptr->DramSizeInBytes < mallocSize) {
                    DSP_LLF_LOG_E("DSP callback ParaSetup In/Out memory insufficient Allocated:%d require:%d !!!", 2, memory_info_ptr->DramSizeInBytes , mallocSize);
                    assert(0);
                }
                mem_ptr = (uint8_t *)NARROW_UP_TO_8_BYTES_ALIGN((uint32_t)(memory_info_ptr->DramAddr));;
                memset(mem_ptr, 0, mallocSize);
#else
                mem_ptr = DSPMEM_tmalloc(stream->callback.EntryPara.DSPTask, mallocSize, stream);
                memset(mem_ptr, 0, mallocSize);
#endif
                for (i = 0 ; i < chNum ; i++) {
                    stream->callback.EntryPara.out_ptr[i] = mem_ptr;
                    stream->callback.EntryPara.in_ptr[i] = mem_ptr;
                    mem_ptr += frameSize;
                }
                DSP_LLF_LOG_I("[DSP] Callback stream Setup codec is CODEC_PCM_COPY, Frame Size:%d, channel_num:%d", 2, frameSize, chNum);

            } else {
                DSP_LLF_LOG_E("first feature is not CODEC_PCM_COPY", 0);
            }

            stream->callback.EntryPara.number.field.process_sequence  = 0;
            stream->callback.EntryPara.number.field.source_type       = (U8)stream->source->type;
            stream->callback.EntryPara.number.field.sink_type         = (U8)stream->sink->type;
            stream->callback.EntryPara.with_encoder                   = FALSE;

            //stream->callback.EntryPara.src_out_size = stream->callback.EntryPara.codec_out_size;
            stream->callback.EntryPara.src_out_sampling_rate =  stream->callback.EntryPara.codec_out_sampling_rate;

            //malloc working buffer for each feature
            //stream->callback.Status = CALLBACK_MALLOC;
            LLF_CallbackEntryTable[CALLBACK_MALLOC](&stream->callback.EntryPara, stream->callback.FeatureTablePtr);

            //stream->callback.Status = CALLBACK_INIT;
            LLF_CallbackEntryTable[CALLBACK_INIT](&stream->callback.EntryPara, stream->callback.FeatureTablePtr);

            DSP_Callback_ResolutionConfig(stream);
            stream->callback.Status = CALLBACK_SUSPEND;

        } else {
            stream->callback.FeatureTablePtr  = NULL;
            DSP_LLF_LOG_I("DSP - Warning:Feature Ptr Null. Source:%d, Sink:%d", 2, stream->source->type, stream->sink->type);
        }

    } else {
        DSP_LLF_LOG_I("Close Callback Stream, LLF type:%d", 1, stream->source->param.audio.scenario_id);
        DSPMEM_Free(stream->callback.EntryPara.DSPTask, stream);
        DSP_Callback_StreamingInit(stream, 1, stream->callback.EntryPara.DSPTask);
    }
}

TRANSFORM TrasformLLF_ISR(SOURCE source, SINK sink, VOID *feature_list_ptr)
{
    TRANSFORM transform = NULL;
    U32 transform_xLinkRegAddr = (U32)__builtin_return_address(0);

    if ((source->transform != NULL) || ((sink->transform != NULL) && (sink->type != SINK_TYPE_DSP_VIRTUAL))) {
        if (source && sink) {
            DSP_LLF_LOG_I("[transfrom]the prev_function lr= 0x%X, source type=0x%X, transform=0x%X, sink type=0x%X, transform=0x%X", 5,
                transform_xLinkRegAddr, source->type, source->transform, sink->type, sink->transform);
        }

        if (source && source->transform && source->transform->sink) {
            DSP_LLF_LOG_I("[transfrom]source linked to sink=0x%X type=0x%X", 2,
            source->transform->sink, source->transform->sink->type);
        }

        if (sink && sink->transform && sink->transform->source) {
            DSP_LLF_LOG_I("[transfrom]sink linked to source=0x%X type=0x%X", 2,
            sink->transform->source, sink->transform->source->type);
        }

        if (source->transform == sink->transform) {
            transform = source->transform;
        }
    } else {
        U32 transform_len = sizeof(TRANSFORM_T);
        transform = pvPortMalloc(transform_len);
        if (transform != NULL) {
            DSP_LLF_LOG_I("[transfrom]the prev_function lr= 0x%x , the malloc_address = 0x%x , the malloc_size = 0x%x", 3, transform_xLinkRegAddr, transform, transform_len);
            memset(transform, 0, transform_len);
            transform->source = source;
            transform->sink = sink;
            //transform->Handler = Stream_Audio_Handler;

            dlist_init(&transform->list);
            dlist_append(&transform->list, &gTransformList);

            source->transform = transform;
            sink->transform = transform;

            //DSP callback config
            if (LLFStreamingISR[0].streamingStatus == STREAMING_DISABLE) {
                LLFStreamingISR[0].source = source;
                LLFStreamingISR[0].sink = sink;
                LLFStreamingISR[0].streamingStatus = STREAMING_START;
                dsp_llf_callback_feature_config((DSP_STREAMING_PARA_PTR)&LLFStreamingISR[0], feature_list_ptr, true);
            }
        }
    }
    return transform;
}

void StreamLLFClose_ISR(U16 msgID)
{
    LLFStreamingISR[0].DspReportEndId = msgID;
    LLFStreamingISR[0].streamingStatus = STREAMING_END;

    dsp_llf_callback_feature_config(&LLFStreamingISR[0], NULL, false);
    PL_CRITICAL(StreamTransformClose, (VOID *)LLF_if.transform);
    if (msgID != MSG_DSP_NULL_REPORT) {
        aud_msg_ack(msgID, FALSE);
    }
}

void dsp_llf_open(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    llf_type_t scenario_id;
    U32 sub_id;

    UNUSED(ack);
    scenario_id = (msg.ccni_message[0] >> 8) & 0xFF;
    sub_id = (msg.ccni_message[0] & 0xFF);

    hal_llf_init();

    LLF_if.pfeature_table = (stream_feature_list_ptr_t)port_dsp_llf_feature_get_list(scenario_id, sub_id);

    /* remap to non-cacheable address */
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    LLF_if.source = dsp_open_stream_in(open_param);
    LLF_if.sink   = dsp_open_stream_out(open_param);
    //DSP_LLF_LOG_I("[LLF] OPEN, Source type(%d)=0x%x Sink type(%d)=0x%x", 4, SOURCE_TYPE_LLF, LLF_if.source, SINK_TYPE_LLF, LLF_if.sink);

    DSP_Callback_PreloaderConfig(LLF_if.pfeature_table, AUDIO_SCENARIO_TYPE_ADVANCED_PASSTHROUGH_HEARING_AID);
    port_dsp_llf_feature_init(scenario_id, sub_id, LLF_if.source, LLF_if.sink);

}

void dsp_llf_start(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);

    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    dsp_start_stream_in(start_param, LLF_if.source);
    dsp_start_stream_out(start_param, LLF_if.sink);

    if (LLF_if.pfeature_table) {
        if (LLF_if.sink->param.audio.scenario_id != AUDIO_LLF_TYPE_VIVID_PT) {
            LLF_if.transform = TrasformAudio2Audio(LLF_if.source, LLF_if.sink, LLF_if.pfeature_table);
            LLF_if.transform->Handler = dsp_llf_stream_handler;
        } else {
            LLF_if.transform = TrasformLLF_ISR(LLF_if.source, LLF_if.sink, LLF_if.pfeature_table);
        }
        if (LLF_if.transform == NULL) {
            DSP_LLF_LOG_E("[LLF] transform failed", 0);
        }

        //enable IRQ
        hal_llf_interrupt_ctrl(true);
    } else {
        DSP_LLF_LOG_E("[LLF] feature list is NULL", 0);
    }
}

void dsp_llf_stop(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_LLF_LOG_I("[LLF] STOP", 0);

    hal_llf_swap_dl(true);

    if (LLF_if.transform != NULL) {
        //disable IRQ
        hal_llf_interrupt_ctrl(false);

        if (LLF_if.sink->param.audio.scenario_id != AUDIO_LLF_TYPE_VIVID_PT) {
            StreamDSPClose(LLF_if.transform->source, LLF_if.transform->sink, msg.ccni_message[0]>>16|0x8000);
        } else {
            StreamLLFClose_ISR(msg.ccni_message[0]>>16|0x8000);
        }
    } else {
        DSP_LLF_LOG_E("[LLF] not exit, just ack.", 0);
        aud_msg_ack(msg.ccni_message[0]>>16|0x8000, FALSE);
    }
    LLF_if.transform = NULL;

}

void dsp_llf_close(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_LLF_LOG_I("[LLF] CLOSE", 0);

    DSP_Callback_UnloaderConfig(LLF_if.pfeature_table, AUDIO_SCENARIO_TYPE_ADVANCED_PASSTHROUGH_HEARING_AID);

    SourceClose(LLF_if.source);
    SinkClose(LLF_if.sink);

    port_dsp_llf_feature_deinit(LLF_if.source, LLF_if.sink);

    memset(&LLF_if, 0, sizeof(CONNECTION_IF));
}

void dsp_llf_suspend(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_LLF_LOG_I("[LLF] SUSPEND", 0);
    port_dsp_llf_suspend(LLF_if.source, LLF_if.sink);
}

void dsp_llf_resume(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_LLF_LOG_I("[LLF] RESUME", 0);
    port_dsp_llf_resume(LLF_if.source, LLF_if.sink);
}

void dsp_llf_runtime_config(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    //DSP_LLF_LOG_I("[LLF] CONFIG", 0);
    U32* share_ptr = (U32*)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    audio_llf_runtime_config_t* param = (audio_llf_runtime_config_t*)share_ptr;

    port_dsp_llf_runtime_config(param);
}

void dsp_llf_anc_bypass_mode(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    //UNUSED(msg);
    UNUSED(ack);
    LLF_anc_bypass_mode = (msg.ccni_message[0] & 0xFF);
    DSP_LLF_LOG_I("[LLF] get anc bypass mode:%d", 1, LLF_anc_bypass_mode);
}

n9_dsp_share_info_t* dsp_llf_query_share_info(void)
{
    return &LLF_share_info;
}

void dsp_llf_notify(U8 notify_event, U8 *data, U8 data_len)
{
    hal_ccni_message_t ccni_msg;
    memset((void *)&ccni_msg, 0, sizeof(hal_ccni_message_t));
    ccni_msg.ccni_message[0] = (MSG_DSP2MCU_LLF_NOTIFY << 16) |
                                    (data_len << 8) |
                                    (notify_event & 0xFF);
    ccni_msg.ccni_message[1] = (U32)data;
    DSP_LLF_LOG_I("dsp_llf_notify, ccni_message[0]=0x%x, ccni_message[1]=0x%x", 2, ccni_msg.ccni_message[0], ccni_msg.ccni_message[1]);
//    for (int i=0; i<data_len; i++) {
//        DSP_LLF_LOG_I("dsp_llf_notify, data(0x%x)=%d", 2, data+i, *(data+i));
//    }
    aud_msg_tx_handler(ccni_msg, 0, FALSE);
}

void dsp_llf_set_audio_dl_status(audio_llf_dl_mix_type_t type, bool is_running)
{
    if (type <LLF_DL_MIX_TYPE_NUM) {
        LLF_music_voice_state[type] = is_running;
        //DSP_LLF_LOG_I("set dl (%d) status(%d)", 2, type, is_running);
        if (LLF_if.transform != NULL) {
            if (llf_dl_need_compensation) {
                hal_llf_swap_dl(llf_no_swap_dl_state);
            }
            U32 scenario_id = LLF_if.source->param.audio.scenario_id;
            if (LLF_feature_entry_table[scenario_id].set_dl_state_entry) {
                LLF_feature_entry_table[scenario_id].set_dl_state_entry(LLF_music_voice_state);
            }
        }
    }
}

ATTR_TEXT_IN_IRAM void dsp_llf_get_audio_dl_status(bool dl_state[LLF_DL_MIX_TYPE_NUM])
{
    U8 i;
    for (i=0; i<LLF_DL_MIX_TYPE_NUM; i++) {
        dl_state[i] = LLF_music_voice_state[i];
    }
    //DSP_LLF_LOG_I("get dl status(%d %d %d)", 3, LLF_music_voice_state[LLF_DL_MIX_TYPE_A2DP], LLF_music_voice_state[LLF_DL_MIX_TYPE_ESCO], LLF_music_voice_state[LLF_DL_MIX_TYPE_VP]);
}

void dsp_llf_set_input_channel_num(U8 num)
{
    U8 i = 0;
    for (i = 0; i < NO_OF_LL_STREAM; i++) {
        if (LLStreaming[i].streamingStatus > STREAMING_DISABLE) {
            if (LLStreaming[i].source->scenario_type == AUDIO_SCENARIO_TYPE_ADVANCED_PASSTHROUGH_HEARING_AID) {
                if (LLStreaming[i].source->param.audio.channel_num >= num) {
                    LLStreaming[i].callback.EntryPara.out_channel_num = num;
                    LLStreaming[i].callback.EntryPara.in_channel_num = num;
                    DSP_LLF_LOG_I("[LLF] dsp_llf_set_input_channel_num:%d", 1, num);
                }
            }
        }
    }
}

U8 dsp_llf_get_input_channel_num(void)
{
    U8 i = 0;
    for (i = 0; i < NO_OF_LL_STREAM; i++) {
        if (LLStreaming[i].streamingStatus == STREAMING_ACTIVE) {
            if (LLStreaming[i].source->scenario_type == AUDIO_SCENARIO_TYPE_ADVANCED_PASSTHROUGH_HEARING_AID) {
                return LLStreaming[i].callback.EntryPara.in_channel_num;
            }
        }
    }
    return 0;
}

void dsp_llf_mute(bool is_mute)
{
    SOURCE source = LLF_if.source;
    SINK sink = LLF_if.sink;
    BUFFER_INFO_PTR buff_info_ptr = &(sink->streamBuffer.BufferInfo);

    if (sink) {
        sink->param.audio.mute_flag = is_mute;
        memset(buff_info_ptr->startaddr[0], 0, buff_info_ptr->length * sink->param.audio.channel_num);
    }
    if (source) {
        source->param.audio.mute_flag = is_mute;
    }

    DSP_LLF_LOG_I("[LLF] mute:%d", 1, is_mute);
}

U32 dsp_llf_get_data_buf_idx(llf_data_type_t type)
{
    return hal_llf_get_data_buf_idx(type);
}

