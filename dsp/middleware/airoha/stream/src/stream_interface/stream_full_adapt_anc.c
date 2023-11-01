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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "dsp_buffer.h"
#include "stream_audio_driver.h"
#include "hal_audio_afe_control.h"
#include "hal_audio_afe_define.h"
#include "audio_afe_common.h"
#include "stream_audio_setting.h"
#include "dsp_callback.h"
#include "FreeRTOS.h"
#include "dsp_drv_afe.h"
#include "dsp_scenario.h"
#include "dsp_audio_process.h"

#include "anc_api.h"
#include "full_adapt_anc_api.h"

/* Private define ------------------------------------------------------------*/
//#define ANC_FRAME_WORK_DEBUG

#ifndef FULL_ADAPT_ANC_USE_MSGID_LOG
#define FULL_ADAPT_ANC_USE_MSGID_LOG
#endif

#ifdef FULL_ADAPT_ANC_USE_MSGID_LOG
#define FA_ANC_LOG_E(message_id,fmt,arg...)    anc_port_log_error(FULL_ADAPT_ANC_LOG_CONTROL_BLOCK_INDEX,message_id,##arg)
#define FA_ANC_LOG_W(message_id,fmt,arg...)    anc_port_log_notice(FULL_ADAPT_ANC_LOG_CONTROL_BLOCK_INDEX,message_id,##arg)
#define FA_ANC_LOG_I(message_id,fmt,arg...)    anc_port_log_info(FULL_ADAPT_ANC_LOG_CONTROL_BLOCK_INDEX,message_id,##arg)
#else
#define FA_ANC_LOG_E(_message, arg_cnt, ...)  LOG_MSGID_E(full_adapt_anc,_message, arg_cnt, ##__VA_ARGS__)
#define FA_ANC_LOG_W(_message, arg_cnt, ...)  LOG_MSGID_W(full_adapt_anc,_message, arg_cnt, ##__VA_ARGS__)
#define FA_ANC_LOG_I(_message, arg_cnt, ...)  LOG_MSGID_I(full_adapt_anc,_message, arg_cnt, ##__VA_ARGS__)
#define FA_ANC_LOG_D(_message, arg_cnt, ...)  LOG_MSGID_D(full_adapt_anc,_message, arg_cnt, ##__VA_ARGS__)
#endif

#ifdef AIR_FULL_ADAPTIVE_ANC_STEREO_ENABLE
#define FAULL_ADPAT_ANC_DMA_CH_NUM 8 //in*4, out*4
#else
#ifdef FULL_ADAPT_ANC_HW_DECOUPLE_DEBUG_ENABLE
#define FAULL_ADPAT_ANC_DMA_CH_NUM 5 //in*3, out*2
#else
#define FAULL_ADPAT_ANC_DMA_CH_NUM 4 //in*2, out*2
#endif
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
CONNECTION_IF full_adapt_ANC_if;

stream_feature_list_t stream_feature_list_full_adapt_ANC[] = {
    CODEC_PCM_COPY,
    FUNC_FULL_ADAPT_ANC,
    FUNC_END,
};

/* Public variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
uint32_t dummy_write_offset = 0;
uint32_t buffer_legnth = 5120;
ATTR_TEXT_IN_IRAM U32 SourceSizeAudioAdapt_ANC(SOURCE source)
{
    U32 writeOffset   = source->streamBuffer.BufferInfo.WriteOffset;
    U32 readOffset    = source->streamBuffer.BufferInfo.ReadOffset;
    U32 length        = source->streamBuffer.BufferInfo.length;
    U16 bytePerSample = source->param.audio.format_bytes;
    U32 data_size;
    U32 buffer_per_channel_shift = ((source->param.audio.channel_num >= 2) && (source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER))
                                   ? 1
                                   : 0;
    U32 audio_frame_size = source->param.audio.frame_size;
    data_size = (writeOffset >= readOffset) ? (writeOffset - readOffset) >> buffer_per_channel_shift : (length - readOffset + writeOffset) >> buffer_per_channel_shift;
    UNUSED(bytePerSample);
#ifdef ANC_FRAME_WORK_DEBUG
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_19, "[Rdebug]DSP SourceSizeAudioAdapt_ANC full(%d) data_size(0x%x) frame_size(0x%x)\r\n", 3, source->streamBuffer.BufferInfo.bBufferIsFull, data_size, audio_frame_size);
#endif
    if ((source->streamBuffer.BufferInfo.bBufferIsFull) || (data_size >= audio_frame_size)) { //[CHK this part]
#ifdef ANC_FRAME_WORK_DEBUG
        FA_ANC_LOG_I(g_FA_ANC_msg_id_string_20, "[Rdebug]DSP SourceSizeAudioAdapt_ANC size(%d)\r\n", 1, audio_frame_size);
#endif
        return (U32)audio_frame_size;
    } else {
#ifdef ANC_FRAME_WORK_DEBUG
        FA_ANC_LOG_I(g_FA_ANC_msg_id_string_21, "[Rdebug]DSP SourceSizeAudioAdapt_ANC size(%d)\r\n", 1, 0);
#endif
        return 0;
    }
}

ATTR_TEXT_IN_IRAM_LEVEL_2 VOID SourceDropAudioAdapt_ANC(SOURCE source, U32 amount)
{
#ifdef ANC_FRAME_WORK_DEBUG
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_22, "[Rdebug]DSP adapt_anc SourceDrop %d\r\n", 1, amount);
#endif
    U32 buffer_per_channel_shift = ((source->param.audio.channel_num >= 2) && (source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER))
                                   ? 1
                                   : 0;
    if (amount == 0) {
        return;
    } else if (source->streamBuffer.BufferInfo.bBufferIsFull == TRUE) {
#if 1//ndef ANC_PATTERN_TEST
        source->streamBuffer.BufferInfo.bBufferIsFull = FALSE;
#endif
    }

    amount = (amount << buffer_per_channel_shift);
    source->streamBuffer.BufferInfo.ReadOffset
        = (source->streamBuffer.BufferInfo.ReadOffset + amount)
          % (source->streamBuffer.BufferInfo.length);
#ifdef ANC_FRAME_WORK_DEBUG
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_23, "[Rdebug]DSP adapt_anc ReadOffset 0x%x\r\n", 1, source->streamBuffer.BufferInfo.ReadOffset);
#endif
}

BOOL SourceConfigureAudioAdapt_ANC(SOURCE source, stream_config_type type, U32 value)
{
#if 0
    return Source_Audio_Configuration(source, type, value);
#else
    UNUSED(source);
    UNUSED(type);
    UNUSED(value);
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_24, "DSP adapt_anc SourceConfig\r\n", 0);
    return 0;
#endif
}

BOOL SourceCloseAudioAdapt_ANC(SOURCE source)
{
#if 0
    audio_ops_close(source);
    return Source_Audio_CloseProcedure(source);
#else
    UNUSED(source);
#if 0
    SOURCE local_source = Source_blks[SOURCE_TYPE_ADAPT_ANC];
    if (local_source != NULL) {
        vPortFree(local_source);
    }
    Source_blks[SOURCE_TYPE_ADAPT_ANC] = NULL;
#endif
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_25, "DSP adapt_anc SourceClose\r\n", 0);
    return 0;
#endif
}

//#define FULL_ADAPT_ANC_PROFILING_HIGH_ENABLE
extern uint32_t SW_DEBUG_SRAM_1_GPT_1M;
extern uint32_t SW_DEBUG_SRAM_2_GPT_1M;
extern uint32_t SW_DEBUG_SRAM_3_GPT_1M;
extern uint32_t SW_DEBUG_SRAM_4_GPT_1M;

ATTR_TEXT_IN_IRAM_LEVEL_2 BOOL Source_Audio_ReadAdaptAncBuffer(SOURCE source, U8 *dst_addr, U32 length)
{
//============================= Profiling process time.
#ifdef FULL_ADAPT_ANC_PROFILING_HIGH_ENABLE
    uint32_t mask_profiling;
    hal_nvic_save_and_set_interrupt_mask(&mask_profiling);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &SW_DEBUG_SRAM_1_GPT_1M);
#endif
//--------------------------------------

#ifdef ANC_FRAME_WORK_DEBUG
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_26, "[Rdebug]DSP adapt_anc SourceReadBUF\r\n", 0);
#endif
    TRANSFORM transform =  source->transform;
    DSP_CALLBACK_PTR callback_ptr = NULL;
    U32 ReadOffset = source->streamBuffer.BufferInfo.ReadOffset;
    U8 i;
    U8 channel_num = 0, channel_sel;
    U16 unwrap_size, copy_offset, copy_size;
    U8 *deChannel_ptr;
    if (transform != NULL && dst_addr == NULL) {
        callback_ptr = DSP_Callback_Get(source, transform->sink);
        dst_addr = callback_ptr->EntryPara.in_ptr[0];
        channel_num = callback_ptr->EntryPara.in_channel_num;
        channel_sel = 0;
    } else {
        channel_num = 1;
        channel_sel = source->streamBuffer.BufferInfo.channelSelected;
    }
    if ((source->buftype != BUFFER_TYPE_INTERLEAVED_BUFFER) || (source->param.audio.channel_num == 1) || (transform == NULL) || (callback_ptr == NULL)) {
        for (i = 0 ; i < channel_num ; i++) {
#ifdef ANC_FRAME_WORK_DEBUG
            if (i < 1) {
                FA_ANC_LOG_I(g_FA_ANC_msg_id_string_27, "[Rdebug]DSP BUFFER_type(%d) ch(%d) dst(0x%x) src(0x%x) length(%d) mute(%d)\r\n", 6, source->buftype, i, dst_addr, source->streamBuffer.BufferInfo.startaddr[channel_sel] + ReadOffset, length, source->param.audio.mute_flag);
            }
#endif
#ifdef FULL_ADAPT_ANC_PROFILING_HIGH_ENABLE
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &SW_DEBUG_SRAM_2_GPT_1M);
#endif
            DSP_C2D_BufferCopy(dst_addr,
                               source->streamBuffer.BufferInfo.startaddr[channel_sel] + ReadOffset,
                               length,
                               source->streamBuffer.BufferInfo.startaddr[channel_sel],
                               source->streamBuffer.BufferInfo.length);
#ifdef FULL_ADAPT_ANC_PROFILING_HIGH_ENABLE
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &SW_DEBUG_SRAM_3_GPT_1M);
#endif
            /* Fill zero packet to prevent UL pop noise */
            if (source->param.audio.mute_flag == TRUE) {
                memset(dst_addr, 0, length);
            }

            if (callback_ptr != NULL) {
                dst_addr = callback_ptr->EntryPara.in_ptr[++channel_sel];
            }
        }
    } else {
        for (i = 0; i < channel_num; i += 2) {
            copy_offset = 0;
            ReadOffset = source->streamBuffer.BufferInfo.ReadOffset;
            while (length > copy_offset) {
                unwrap_size = source->streamBuffer.BufferInfo.length - ReadOffset; /* Remove + 1 to sync more common usage */
                copy_size = MIN((length - copy_offset), unwrap_size >> 1);
                if (source->param.audio.format_bytes == 4) {
                    if (!(callback_ptr->EntryPara.in_ptr[i + 1] == NULL)) {
                        deChannel_ptr = callback_ptr->EntryPara.in_ptr[i + 1] + copy_offset; /* For 2-Mic Path: L R L R L R ... */
                    } else {
                        deChannel_ptr = callback_ptr->EntryPara.in_ptr[i + 1]; /* For EC Path: C X C X C X ... */
                    }
#if 1//modify for 32bit UL path with echo ref
#ifdef ANC_FRAME_WORK_DEBUG
                    if (i < 1) {
                        FA_ANC_LOG_I(g_FA_ANC_msg_id_string_28, "[Rdebug]DSP BUFFER_type(%d) ch(%d) dst_1(0x%x) dst_2(0x%x) src(0x%x) length(%d)\r\n", 5, source->buftype, i, (U32 *)((U8 *)callback_ptr->EntryPara.in_ptr[i] + copy_offset), (U32 *)(deChannel_ptr), (U32 *)(source->streamBuffer.BufferInfo.startaddr[channel_sel] + ReadOffset), copy_size >> 2);
                    }
#endif
#ifdef FULL_ADAPT_ANC_PROFILING_HIGH_ENABLE
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &SW_DEBUG_SRAM_2_GPT_1M);
#endif
                    DSP_I2D_BufferCopy_32bit_mute((U32 *)(source->streamBuffer.BufferInfo.startaddr[channel_sel] + ReadOffset),
                                                  (U32 *)((U8 *)callback_ptr->EntryPara.in_ptr[i] + copy_offset),
                                                  (U32 *)(deChannel_ptr),
                                                  copy_size >> 2,
                                                  source->param.audio.mute_flag);
#ifdef FULL_ADAPT_ANC_PROFILING_HIGH_ENABLE
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &SW_DEBUG_SRAM_3_GPT_1M);
#endif
#else
                    DSP_I2D_BufferCopy_32bit_mute((U32 *)(source->streamBuffer.BufferInfo.startaddr[channel_sel] + ReadOffset),
                                                  (U32 *)((U8 *)callback_ptr->EntryPara.in_ptr[i] + copy_offset),
                                                  (U32 *)((U8 *)callback_ptr->EntryPara.in_ptr[i + 1] + copy_offset),
                                                  copy_size >> 2,
                                                  source->param.audio.mute_flag);

#endif
                } else {
                    if (!(callback_ptr->EntryPara.in_ptr[i + 1] == NULL)) {
                        deChannel_ptr = callback_ptr->EntryPara.in_ptr[i + 1] + copy_offset; /* For 2-Mic Path: L R L R L R ... */
                    } else {
                        deChannel_ptr = callback_ptr->EntryPara.in_ptr[i + 1]; /* For EC Path: C X C X C X ... */
                    }
                    DSP_I2D_BufferCopy_16bit_mute((U16 *)(source->streamBuffer.BufferInfo.startaddr[channel_sel] + ReadOffset),
                                                  (U16 *)(callback_ptr->EntryPara.in_ptr[i] + copy_offset),
                                                  (U16 *)(deChannel_ptr),
                                                  copy_size >> 1,
                                                  source->param.audio.mute_flag);
                }
                ReadOffset = (ReadOffset + (copy_size << 1)) % source->streamBuffer.BufferInfo.length;
                copy_offset += copy_size;
            }
            channel_sel++;
        }
    }

//============================= Profiling process time.
#ifdef FULL_ADAPT_ANC_PROFILING_HIGH_ENABLE
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &SW_DEBUG_SRAM_4_GPT_1M);
    hal_nvic_restore_interrupt_mask(mask_profiling);
#endif
//--------------------------------------
    return TRUE;
}

audio_source_pcm_ops_t afe_platform_adapt_anc_ops = {
    .probe      = NULL,//pcm_ul1_probe,
    .open       = NULL,//pcm_ul1_open,
    .close      = NULL,//pcm_ul1_close,
    .hw_params  = NULL,//pcm_ul1_hw_params,
    .trigger    = NULL,//pcm_ul1_trigger,
    .copy       = NULL,//pcm_ul1_copy,
};

void audio_adapt_anc_set_ops(void *param)
{
    if (param == NULL) {
        FA_ANC_LOG_E(g_FA_ANC_msg_id_string_29, "DSP adapt_anc ops parametser invalid\r\n", 0);
        return;
    }
    if (audio_ops_distinguish_audio_sink(param)) {
        //SINK sink = param;
        //sink->param.audio.ops = (audio_pcm_ops_p)&afe_platform_dl1_ops;
        FA_ANC_LOG_E(g_FA_ANC_msg_id_string_30, "DSP adapt_anc ops parametser invalid sink\r\n", 0);
    } else if (audio_ops_distinguish_audio_source(param)) {
        SOURCE source = param;
        source->param.audio.ops = (audio_pcm_ops_p)&afe_platform_adapt_anc_ops;
    }
}

VOID Source_Audio_Adpat_ANC_Init(SOURCE source)
{
    /* interface init */
    source->sif.SourceSize         = SourceSizeAudioAdapt_ANC;
    source->sif.SourceDrop         = SourceDropAudioAdapt_ANC;
    source->sif.SourceConfigure    = SourceConfigureAudioAdapt_ANC;
    source->sif.SourceClose        = SourceCloseAudioAdapt_ANC;   //SourceCloseAudio;
    source->sif.SourceReadBuf      = (source_read_buffer_entry)Source_Audio_ReadAdaptAncBuffer; //SourceReadBufAudioAfe
}

#ifdef AIR_FULL_ADAPTIVE_ANC_STEREO_ENABLE
extern void StreamAudioSetAdaptAncDMA(uint32_t index0_address, uint32_t index1_address, uint32_t index2_address, uint32_t index3_address, uint32_t index4_address, uint32_t index5_address, uint32_t index6_address, uint32_t index7_address);
#else
#ifdef FULL_ADAPT_ANC_HW_DECOUPLE_DEBUG_ENABLE
extern void StreamAudioSetAdaptAncDMA(uint32_t index0_address, uint32_t index1_address, uint32_t index2_address, uint32_t index3_address, uint32_t index4_address);
#else
extern void StreamAudioSetAdaptAncDMA(uint32_t index0_address, uint32_t index1_address, uint32_t index2_address, uint32_t index3_address);
#endif
#endif
extern SOURCE new_source(SOURCE_TYPE SourceType);
SOURCE_T g_StreamAdaptANC_SOURCE;
SOURCE StreamAudioAdaptAncSource(void *param)
{
    SOURCE source = NULL;
    SOURCE_TYPE source_type = SOURCE_TYPE_ADAPT_ANC;
    audio_transmitter_open_param_p open_param;

    if (Source_blks[SOURCE_TYPE_ADAPT_ANC] != NULL) {
        //return Source_blks[SOURCE_TYPE_ADAPT_ANC];
        source = Source_blks[SOURCE_TYPE_ADAPT_ANC];
        memset(source, 0, sizeof(SOURCE_T));
        source->type = SOURCE_TYPE_ADAPT_ANC;
    } else {
#if 1
        Source_blks[SOURCE_TYPE_ADAPT_ANC] = &g_StreamAdaptANC_SOURCE;
        source = Source_blks[SOURCE_TYPE_ADAPT_ANC];
        memset(source, 0, sizeof(SOURCE_T));
        source->type = SOURCE_TYPE_ADAPT_ANC;
#else
        source = new_source(SOURCE_TYPE_ADAPT_ANC);
#endif
    }
    if (source == NULL) {
        return NULL;
    }

    open_param = (audio_transmitter_open_param_p)param;
    source->type = source_type;
    source->buftype                 = BUFFER_TYPE_CIRCULAR_BUFFER;
    source->taskId                  = DAV_TASK_ID;
    source->param.audio.frame_size  = 64 * 4; //U24
    source->param.audio.channel_num = FAULL_ADPAT_ANC_DMA_CH_NUM;

#if 0//def ANC_PATTERN_TEST
    source->param.audio.channel_num = 2; //in*2 only
#endif

#if 0
    Source_Audio_Buffer_Ctrl(source, TRUE);
#else
    U8 *mem_ptr;
    U32 mem_size;
#ifdef AIR_FULL_ADAPTIVE_ANC_STEREO_ENABLE
    source->streamBuffer.BufferInfo.length = source->param.audio.frame_size * 4; // 4 frames
#else
    source->streamBuffer.BufferInfo.length = source->param.audio.frame_size * 4; // 4 frames
#endif
    mem_size = source->param.audio.channel_num * (source->streamBuffer.BufferInfo.length);

    //mem_ptr = (void *)hal_memory_allocate_sram(HAL_AUDIO_AGENT_DEVICE_ANC, mem_size);
    mem_ptr = (U8 *)StreamAudioGetAdaptAncSource();

    memset(mem_ptr, 0, mem_size);
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_31, "[Rdebug][A_ANC]afe init, start addr(0x%x) size(%d)", 2, mem_ptr, mem_size);

    source->streamBuffer.BufferInfo.startaddr[0] = mem_ptr;
    source->param.audio.AfeBlkControl.phys_buffer_addr = (uint32_t)source->streamBuffer.BufferInfo.startaddr[0];

    mem_ptr = (U8 *)(source->param.audio.AfeBlkControl.phys_buffer_addr) + source->streamBuffer.BufferInfo.length;
    for (U8 i = 0; ((i < source->param.audio.channel_num) && (i < BUFFER_INFO_CH_NUM)) ; i++) {
        if (!source->streamBuffer.BufferInfo.startaddr[i]) {
            source->streamBuffer.BufferInfo.startaddr[i] = mem_ptr;
            mem_ptr += (source->streamBuffer.BufferInfo.length);
        }
        FA_ANC_LOG_I(g_FA_ANC_msg_id_string_32, "[Rdebug][A_ANC]afe_number=%d,startaddr[0]=0x%x,startaddr[%d]=0x%x", 4, BUFFER_INFO_CH_NUM, source->streamBuffer.BufferInfo.startaddr[0], i, source->streamBuffer.BufferInfo.startaddr[i]);
    }
#ifdef AIR_FULL_ADAPTIVE_ANC_STEREO_ENABLE
    StreamAudioSetAdaptAncDMA((uint32_t)source->streamBuffer.BufferInfo.startaddr[0],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[1],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[2],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[3],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[4],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[5],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[6],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[7]);
#else
#ifdef FULL_ADAPT_ANC_HW_DECOUPLE_DEBUG_ENABLE
    StreamAudioSetAdaptAncDMA((uint32_t)source->streamBuffer.BufferInfo.startaddr[0],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[1],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[2],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[3],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[4]);
#else
    StreamAudioSetAdaptAncDMA((uint32_t)source->streamBuffer.BufferInfo.startaddr[0],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[1],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[2],
                              (uint32_t)source->streamBuffer.BufferInfo.startaddr[3]);
#endif
#endif
#endif

    audio_adapt_anc_set_ops(source);
    Source_Audio_Adpat_ANC_Init(source);
    return source;
}

SOURCE dsp_open_stream_in_adapt_anc(mcu2dsp_open_param_p open_param)
{
    SOURCE source;

    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_33, "Stream in aduio adapt_anc\r\n", 0);

    source = StreamAudioAdaptAncSource(NULL);
    if (source) {
#ifdef AIR_FULL_ADAPTIVE_ANC_USE_PIC
        full_adapt_ANC_if.pfeature_table = stream_feature_list_full_adapt_ANC;
#endif
        FA_ANC_LOG_I(g_FA_ANC_msg_id_string_34, "DSP source create successfully\r\n", 0);
    } else {
        FA_ANC_LOG_E(g_FA_ANC_msg_id_string_35, "DSP source create fail\r\n", 0);
    }
    UNUSED(open_param);
    return source;
}

extern SOURCE dsp_open_stream_in(mcu2dsp_open_param_p open_param);
extern SINK dsp_open_stream_out(mcu2dsp_open_param_p open_param);
extern void dsp_start_stream_in(mcu2dsp_start_param_p start_param, SOURCE source);
extern void dsp_start_stream_out(mcu2dsp_start_param_p start_param, SINK sink);
void dsp_adpat_anc_open(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_36, "[ANC_API] ADAP_ANC Open", 0);

    /* remap to non-cacheable address */
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_37, "[ANC_API] Source(0x%x) Sink(0x%x)", 2, open_param->param.stream_in, open_param->param.stream_out);
    full_adapt_ANC_if.source = dsp_open_stream_in(open_param);
    full_adapt_ANC_if.sink   = dsp_open_stream_out(open_param);
#ifdef AIR_FULL_ADAPTIVE_ANC_USE_PIC
    DSP_Callback_PreloaderConfig(full_adapt_ANC_if.pfeature_table, AUDIO_SCENARIO_TYPE_FADP_ANC_STREAM);
#endif
    full_adapt_ANC_if.transform = NULL;
}

extern volatile uint8_t g_control_scenario;
extern void full_adapt_anc_change_control_state(anc_full_adapt_control_state_t control_state);
extern void dsp_adpat_anc_init_param(void *param_address);
extern void hal_anc_set_dma_enable(uint8_t enable);
extern dsp_anc_control_result_t full_adapt_anc_change_control_scenario(uint16_t scenario);
void dsp_adpat_anc_start(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint32_t savedmask;
    UNUSED(ack);
    UNUSED(savedmask);
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_38, "[ANC_API] ADAP_ANC Start", 0);
    mcu2dsp_start_param_p  start_param;
    dsp_anc_control_result_t ret = DSP_ANC_CONTROL_EXECUTION_FAIL;
    start_param    = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    dsp_start_stream_in(start_param, full_adapt_ANC_if.source);
    dsp_start_stream_out(start_param, full_adapt_ANC_if.sink);

    if (hal_memview_cm4_to_dsp0((uint32_t)start_param->param.Feature)) {
        dsp_adpat_anc_init_param((void *)hal_memview_cm4_to_dsp0((uint32_t)start_param->param.Feature));
    } else {
        FA_ANC_LOG_E(g_FA_ANC_msg_id_string_39, "[ANC_API] ADAP_ANC Start sharp_p NULL", 0);
    }
    full_adapt_ANC_if.transform = TrasformAudio2Audio(full_adapt_ANC_if.source, full_adapt_ANC_if.sink, stream_feature_list_full_adapt_ANC);
    if (full_adapt_ANC_if.transform == NULL) {
        FA_ANC_LOG_E(g_FA_ANC_msg_id_string_40, "[ANC_API] transform failed", 0);
    }

    ret = full_adapt_anc_change_control_scenario(g_control_scenario);

    hal_anc_set_dma_enable(1);

#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    // Send dummy CCNI message to trigger sleep condition check
    dsp_adapt_anc_send_unstable_status(AUDIO_ADAPTIVE_ANC_CLK_CTRL_TYPE_LIB, 2);
#endif

    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_41, "[ANC_API] Start END (%d),ramp(0x%x)", 2, g_control_scenario, *((volatile uint32_t *)(0xC9050600)));
}

extern volatile uint8_t LOW_TASK_PROCESS_STATUS;
extern volatile bool TRIGGER_FRAMEWORK_END;
extern void full_adapt_anc_reinitialize(void);
void dsp_adpat_anc_stop(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    uint32_t mask;
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_42, "[ANC_API] ADAP_ANC Stop", 0);
    if (full_adapt_ANC_if.transform != NULL) {
        /* Stop trigger low priority task process. */
        hal_nvic_save_and_set_interrupt_mask(&mask);
        TRIGGER_FRAMEWORK_END = true;
        hal_nvic_restore_interrupt_mask(mask);
        uint8_t time_out_count = 0;
        while (LOW_TASK_PROCESS_STATUS != 0) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            time_out_count++;
            if (time_out_count > 2) {
                FA_ANC_LOG_E(g_FA_ANC_msg_id_string_45, "[ANC_API] ADAP_ANC Stop process timeout", 0);
                break;
            }
        }
        StreamDSPClose(full_adapt_ANC_if.transform->source, full_adapt_ANC_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);

        /* Trigger stop ANC DMA. */
        hal_anc_set_dma_enable(0);
    } else {
        FA_ANC_LOG_E(g_FA_ANC_msg_id_string_43, "ADAP_ANC not exit, just ack.", 0);
        aud_msg_ack(msg.ccni_message[0] >> 16 | 0x8000, FALSE);
    }

    full_adapt_ANC_if.transform = NULL;
    full_adapt_anc_reinitialize();
}

void dsp_adpat_anc_close(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
#ifdef AIR_FULL_ADAPTIVE_ANC_USE_PIC
    DSP_Callback_UnloaderConfig(full_adapt_ANC_if.pfeature_table, AUDIO_SCENARIO_TYPE_FADP_ANC_STREAM);
#endif
    TRIGGER_FRAMEWORK_END = false;
    FA_ANC_LOG_I(g_FA_ANC_msg_id_string_44, "[ANC_API] ADAP_ANC Close", 0);
    SourceClose(full_adapt_ANC_if.source);
    SinkClose(full_adapt_ANC_if.sink);
    memset(&full_adapt_ANC_if, 0, sizeof(CONNECTION_IF));
}

/*
 * Get adapt source buffer
 * Units: sample
*/
ATTR_TEXT_IN_IRAM_LEVEL_2 uint32_t audio_adapt_anc_query_data_amount(void)
{
    volatile SOURCE local_source = Source_blks[SOURCE_TYPE_ADAPT_ANC];
    if (local_source != NULL) {
        BUFFER_INFO *buffer_info = &local_source->streamBuffer.BufferInfo;
        if ((buffer_info->WriteOffset - buffer_info->ReadOffset) > 0) {
            return buffer_info->WriteOffset - buffer_info->ReadOffset;
        } else {
            return 0;
        }
    }
    return 0;
}
