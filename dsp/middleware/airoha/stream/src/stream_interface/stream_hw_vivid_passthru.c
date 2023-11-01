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
#include "dsp_callback.h"
#include "FreeRTOS.h"
#include "dsp_drv_afe.h"
#include "dsp_scenario.h"
#include "dsp_audio_process.h"

#include "anc_api.h"
#include "hw_vivid_passthru_api.h"

/* Private define ------------------------------------------------------------*/
#ifndef HW_VIVID_PT_USE_MSGID_LOG
#define HW_VIVID_PT_USE_MSGID_LOG
#endif

#ifdef HW_VIVID_PT_USE_MSGID_LOG
#define HW_VIVID_PT_LOG_E(message_id,fmt,arg...)   anc_port_log_error(HW_VIVID_PT_LOG_CONTROL_BLOCK_INDEX,message_id,##arg)
#define HW_VIVID_PT_LOG_W(message_id,fmt,arg...)   anc_port_log_notice(HW_VIVID_PT_LOG_CONTROL_BLOCK_INDEX,message_id,##arg)
#define HW_VIVID_PT_LOG_I(message_id,fmt,arg...)   anc_port_log_info(HW_VIVID_PT_LOG_CONTROL_BLOCK_INDEX,message_id,##arg)
#else
#define HW_VIVID_PT_LOG_E(_message, arg_cnt, ...)  LOG_MSGID_E(hw_vivid_passthru,_message, arg_cnt, ##__VA_ARGS__)
#define HW_VIVID_PT_LOG_W(_message, arg_cnt, ...)  LOG_MSGID_W(hw_vivid_passthru,_message, arg_cnt, ##__VA_ARGS__)
#define HW_VIVID_PT_LOG_I(_message, arg_cnt, ...)  LOG_MSGID_I(hw_vivid_passthru,_message, arg_cnt, ##__VA_ARGS__)
#define HW_VIVID_PT_LOG_D(_message, arg_cnt, ...)  LOG_MSGID_D(hw_vivid_passthru,_message, arg_cnt, ##__VA_ARGS__)
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
CONNECTION_IF hw_vivid_passthru_if;

stream_feature_list_t stream_feature_list_hw_vivid_PT[] = {
    CODEC_PCM_COPY,
    FUNC_HW_VIVID_PT,
    FUNC_END,
};

/* Public variables ----------------------------------------------------------*/
volatile bool g_HW_Vivid_PT_control_state = false;

/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
ATTR_TEXT_IN_IRAM U32 SourceSizeAudioHWVivid_PT(SOURCE source)
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

    if ((source->streamBuffer.BufferInfo.bBufferIsFull) || (data_size >= audio_frame_size)) {
        return (U32)audio_frame_size;
    } else {
        return 0;
    }
}

ATTR_TEXT_IN_IRAM_LEVEL_2 VOID SourceDropAudioHWVivid_PT(SOURCE source, U32 amount)
{
    U32 buffer_per_channel_shift = ((source->param.audio.channel_num >= 2) && (source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER))
                                   ? 1
                                   : 0;
    if (amount == 0) {
        return;
    } else if (source->streamBuffer.BufferInfo.bBufferIsFull == TRUE) {
        source->streamBuffer.BufferInfo.bBufferIsFull = FALSE;
    }

    amount = (amount << buffer_per_channel_shift);
    source->streamBuffer.BufferInfo.ReadOffset
        = (source->streamBuffer.BufferInfo.ReadOffset + amount)
          % (source->streamBuffer.BufferInfo.length);
}

BOOL SourceConfigureAudioHWVivid_PT(SOURCE source, stream_config_type type, U32 value)
{
    UNUSED(source);
    UNUSED(type);
    UNUSED(value);

    return 0;
}

BOOL SourceCloseAudioHWVivid_PT(SOURCE source)
{
    UNUSED(source);
    HW_VIVID_PT_LOG_I(g_HW_VIVID_PT_msg_id_string_3, "[HW Vivid Passthru]DSP HWVivid_PT SourceClose", 0);
    return 0;
}

//#define HW_VIVID_PT_MIPS_HIGH_PROFILING
extern uint32_t SW_DEBUG_SOURCE_READ_START;
extern uint32_t SW_DEBUG_SOURCE_READ_END;

ATTR_TEXT_IN_IRAM_LEVEL_2 BOOL Source_Audio_ReadHWVividPTBuffer(SOURCE source, U8 *dst_addr, U32 length)
{
#ifdef HW_VIVID_PT_MIPS_HIGH_PROFILING
    uint32_t mask_profiling;
    hal_nvic_save_and_set_interrupt_mask(&mask_profiling);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &SW_DEBUG_SOURCE_READ_START);
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
            DSP_C2D_BufferCopy(dst_addr,
                               source->streamBuffer.BufferInfo.startaddr[channel_sel] + ReadOffset,
                               length,
                               source->streamBuffer.BufferInfo.startaddr[channel_sel],
                               source->streamBuffer.BufferInfo.length);
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
#ifdef ANC_FRAME_WORK_DEBUG
                    if (i < 1) {
                        FA_ANC_LOG_I(g_FA_ANC_msg_id_string_28, "[Rdebug]DSP BUFFER_type(%d) ch(%d) dst_1(0x%x) dst_2(0x%x) src(0x%x) length(%d)\r\n", 5, source->buftype, i, (U32 *)((U8 *)callback_ptr->EntryPara.in_ptr[i] + copy_offset), (U32 *)(deChannel_ptr), (U32 *)(source->streamBuffer.BufferInfo.startaddr[channel_sel] + ReadOffset), copy_size >> 2);
                    }
#endif
                    DSP_I2D_BufferCopy_32bit_mute((U32 *)(source->streamBuffer.BufferInfo.startaddr[channel_sel] + ReadOffset),
                                                  (U32 *)((U8 *)callback_ptr->EntryPara.in_ptr[i] + copy_offset),
                                                  (U32 *)(deChannel_ptr),
                                                  copy_size >> 2,
                                                  source->param.audio.mute_flag);
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

#ifdef HW_VIVID_PT_MIPS_HIGH_PROFILING
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &SW_DEBUG_SOURCE_READ_END);
    hal_nvic_restore_interrupt_mask(mask_profiling);
#endif

    return TRUE;
}

audio_source_pcm_ops_t afe_platform_hw_vivid_pt_ops = {
    .probe      = NULL,//pcm_ul1_probe,
    .open       = NULL,//pcm_ul1_open,
    .close      = NULL,//pcm_ul1_close,
    .hw_params  = NULL,//pcm_ul1_hw_params,
    .trigger    = NULL,//pcm_ul1_trigger,
    .copy       = NULL,//pcm_ul1_copy,
};

void audio_hw_vivid_pt_set_ops(void *param)
{
    if (param == NULL) {
        HW_VIVID_PT_LOG_E(g_HW_VIVID_PT_msg_id_string_4, "[HW Vivid Passthru]DSP adapt_anc ops parametser invalid", 0);
        return;
    }
    if (audio_ops_distinguish_audio_sink(param)) {
        //SINK sink = param;
        //sink->param.audio.ops = (audio_pcm_ops_p)&afe_platform_dl1_ops;
        HW_VIVID_PT_LOG_E(g_HW_VIVID_PT_msg_id_string_5, "[HW Vivid Passthru]DSP adapt_anc ops parametser invalid sink", 0);
    } else if (audio_ops_distinguish_audio_source(param)) {
        SOURCE source = param;
        source->param.audio.ops = (audio_pcm_ops_p)&afe_platform_hw_vivid_pt_ops;
    }
}

VOID Source_Audio_HW_Vivid_PT_Init(SOURCE source)
{
    /* interface init */
    source->sif.SourceSize         = SourceSizeAudioHWVivid_PT;
    source->sif.SourceDrop         = SourceDropAudioHWVivid_PT;
    source->sif.SourceConfigure    = SourceConfigureAudioHWVivid_PT;
    source->sif.SourceClose        = SourceCloseAudioHWVivid_PT;
    source->sif.SourceReadBuf      = (source_read_buffer_entry)Source_Audio_ReadHWVividPTBuffer; //SourceReadBufAudioAfe
}

#ifdef AIR_HW_VIVID_PT_STEREO_ENABLE
extern void StreamAudioSetHWVividPTDMA(uint32_t index0_address, uint32_t index1_address, uint32_t index2_address,
                                       uint32_t index3_address, uint32_t index4_address, uint32_t index5_address);
#else
extern void StreamAudioSetHWVividPTDMA(uint32_t index0_address, uint32_t index1_address, uint32_t index2_address, uint32_t index3_address);
#endif

extern SOURCE new_source(SOURCE_TYPE SourceType);
SOURCE_T g_StreamHWVividPT_SOURCE;
SOURCE StreamAudioHWVividPTSource(void *param)
{
    SOURCE source = NULL;
    SOURCE_TYPE source_type = SOURCE_TYPE_HW_VIVID_PT;
    audio_transmitter_open_param_p open_param;

    if (Source_blks[SOURCE_TYPE_HW_VIVID_PT] != NULL) {
        source = Source_blks[SOURCE_TYPE_HW_VIVID_PT];
        memset(source, 0, sizeof(SOURCE_T));
        source->type = SOURCE_TYPE_HW_VIVID_PT;
    } else {
        Source_blks[SOURCE_TYPE_HW_VIVID_PT] = &g_StreamHWVividPT_SOURCE;
        source = Source_blks[SOURCE_TYPE_HW_VIVID_PT];
        memset(source, 0, sizeof(SOURCE_T));
        source->type = SOURCE_TYPE_HW_VIVID_PT;
    }
    if (source == NULL) {
        return NULL;
    }

    open_param = (audio_transmitter_open_param_p)param;
    source->type = source_type;
    source->buftype                 = BUFFER_TYPE_CIRCULAR_BUFFER;
    source->taskId                  = DAV_TASK_ID;
    source->param.audio.frame_size  = 64 * 4; //U24
#ifdef AIR_HW_VIVID_PT_STEREO_ENABLE
    source->param.audio.channel_num = 6; //in*4, out*2
#else
    source->param.audio.channel_num = 4; //in*2, out*1
#endif

    U8 *mem_ptr;
    U32 mem_size;

    source->streamBuffer.BufferInfo.length = source->param.audio.frame_size * 4; // 4 frames
    mem_size = source->param.audio.channel_num * (source->streamBuffer.BufferInfo.length);

    mem_ptr = (U8 *)StreamAudioGetHWVividPTSource();

    memset(mem_ptr, 0, mem_size);
    HW_VIVID_PT_LOG_I(g_HW_VIVID_PT_msg_id_string_6, "[HW Vivid Passthru]afe init, start addr(0x%x) size(%d)", 2, mem_ptr, mem_size);

    source->streamBuffer.BufferInfo.startaddr[0] = mem_ptr;
    source->param.audio.AfeBlkControl.phys_buffer_addr = (uint32_t)source->streamBuffer.BufferInfo.startaddr[0];

    mem_ptr = (U8 *)(source->param.audio.AfeBlkControl.phys_buffer_addr) + source->streamBuffer.BufferInfo.length;
    for (U8 i = 0; ((i < source->param.audio.channel_num) && (i < BUFFER_INFO_CH_NUM)) ; i++) {
        if (!source->streamBuffer.BufferInfo.startaddr[i]) {
            source->streamBuffer.BufferInfo.startaddr[i] = mem_ptr;
            mem_ptr += (source->streamBuffer.BufferInfo.length);
        }
        HW_VIVID_PT_LOG_I(g_HW_VIVID_PT_msg_id_string_7, "[HW Vivid Passthru]afe_number=%d,startaddr[0]=0x%x,startaddr[%d]=0x%x", 4, BUFFER_INFO_CH_NUM, source->streamBuffer.BufferInfo.startaddr[0], i, source->streamBuffer.BufferInfo.startaddr[i]);
    }
#ifdef AIR_HW_VIVID_PT_STEREO_ENABLE
    StreamAudioSetHWVividPTDMA((uint32_t)source->streamBuffer.BufferInfo.startaddr[0],
                               (uint32_t)source->streamBuffer.BufferInfo.startaddr[1],
                               (uint32_t)source->streamBuffer.BufferInfo.startaddr[2],
                               (uint32_t)source->streamBuffer.BufferInfo.startaddr[3],
                               (uint32_t)source->streamBuffer.BufferInfo.startaddr[4],
                               (uint32_t)source->streamBuffer.BufferInfo.startaddr[5]);
#else
    StreamAudioSetHWVividPTDMA((uint32_t)source->streamBuffer.BufferInfo.startaddr[0],
                               (uint32_t)source->streamBuffer.BufferInfo.startaddr[1],
                               (uint32_t)source->streamBuffer.BufferInfo.startaddr[2],
                               (uint32_t)source->streamBuffer.BufferInfo.startaddr[3]);
#endif

    audio_hw_vivid_pt_set_ops(source);
    Source_Audio_HW_Vivid_PT_Init(source);
    return source;
}

SOURCE dsp_open_stream_in_hw_vivid_pt(mcu2dsp_open_param_p open_param)
{
    SOURCE source;

    HW_VIVID_PT_LOG_I(g_HW_VIVID_PT_msg_id_string_8, "[HW Vivid Passthru]Stream in aduio hw_vivid_pt", 0);

    source = StreamAudioHWVividPTSource(NULL);
    if (!source) {
        HW_VIVID_PT_LOG_E(g_HW_VIVID_PT_msg_id_string_9, "[HW Vivid Passthru]DSP source create fail", 0);
    }
    UNUSED(open_param);
    return source;
}

extern SOURCE dsp_open_stream_in(mcu2dsp_open_param_p open_param);
extern SINK dsp_open_stream_out(mcu2dsp_open_param_p open_param);
extern void dsp_start_stream_in(mcu2dsp_start_param_p start_param, SOURCE source);
extern void dsp_start_stream_out(mcu2dsp_start_param_p start_param, SINK sink);
void dsp_hw_vivid_pt_open(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    HW_VIVID_PT_LOG_I(g_HW_VIVID_PT_msg_id_string_10, "[HW Vivid Passthru]HW VIVID PT OPEN", 0);

    /* remap to non-cacheable address */
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    HW_VIVID_PT_LOG_I(g_HW_VIVID_PT_msg_id_string_11, "[HW Vivid Passthru]Source(0x%x) Sink(0x%x)", 2, open_param->param.stream_in, open_param->param.stream_out);

    hw_vivid_passthru_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_hw_vivid_PT;

    hw_vivid_passthru_if.source = dsp_open_stream_in(open_param);
    hw_vivid_passthru_if.sink   = dsp_open_stream_out(open_param);
    hw_vivid_passthru_if.transform = NULL;

    DSP_Callback_PreloaderConfig(hw_vivid_passthru_if.pfeature_table, open_param->audio_scenario_type);
}

extern void dsp_hw_vivid_passthru_init_param(void *param_address);
extern void hal_anc_set_dma_enable(uint8_t enable);
void dsp_hw_vivid_pt_start(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint32_t savedmask;
    UNUSED(ack);
    UNUSED(savedmask);
    HW_VIVID_PT_LOG_I(g_HW_VIVID_PT_msg_id_string_12, "[HW Vivid Passthru]HW VIVID PT Start", 0);
    mcu2dsp_start_param_p  start_param;

    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    dsp_start_stream_in(start_param, hw_vivid_passthru_if.source);
    dsp_start_stream_out(start_param, hw_vivid_passthru_if.sink);

    if (hal_memview_cm4_to_dsp0((uint32_t)start_param->param.Feature)) {
        dsp_hw_vivid_passthru_init_param((void *)hal_memview_cm4_to_dsp0((uint32_t)start_param->param.Feature));
    } else {
        HW_VIVID_PT_LOG_E(g_HW_VIVID_PT_msg_id_string_2, "[HW Vivid Passthru]Start sharp_p NULL", 0);
    }
    hw_vivid_passthru_if.transform = TrasformAudio2Audio(hw_vivid_passthru_if.source, hw_vivid_passthru_if.sink, stream_feature_list_hw_vivid_PT);
    if (hw_vivid_passthru_if.transform == NULL) {
        HW_VIVID_PT_LOG_E(g_HW_VIVID_PT_msg_id_string_13, "[HW Vivid Passthru]transform failed", 0);
    }

    hal_anc_set_dma_enable(1);

    HW_VIVID_PT_LOG_I(g_HW_VIVID_PT_msg_id_string_14, "[HW Vivid Passthru]Start END ramp(0x%x)", 1, *((volatile uint32_t *)(0xC9050600)));
}

extern volatile uint8_t HWVIVID_LOW_TASK_PROCESS_STATUS;
extern volatile bool HWVIVID_TRIGGER_FRAMEWORK_END;
extern void hw_vivid_passthru_reinitialize(void);
void dsp_hw_vivid_pt_stop(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    uint32_t mask;
    HW_VIVID_PT_LOG_I(g_HW_VIVID_PT_msg_id_string_15, "[HW Vivid Passthru]HW_VIVID_PT Stop", 0);
    if (hw_vivid_passthru_if.transform != NULL) {
        /* Stop trigger low priority task process. */
        hal_nvic_save_and_set_interrupt_mask(&mask);
        HWVIVID_TRIGGER_FRAMEWORK_END = true;
        hal_nvic_restore_interrupt_mask(mask);
        uint8_t time_out_count = 0;
        while (HWVIVID_LOW_TASK_PROCESS_STATUS != 0) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            time_out_count++;
            if (time_out_count > 2) {
                HW_VIVID_PT_LOG_E(g_HW_VIVID_PT_msg_id_string_16, "[HW Vivid Passthru]HW_VIVID_PT Stop process timeout", 0);
                break;
            }
        }
        StreamDSPClose(hw_vivid_passthru_if.transform->source, hw_vivid_passthru_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);

        /* Trigger stop ANC DMA. */
        hal_anc_set_dma_enable(0);
    } else {
        HW_VIVID_PT_LOG_E(g_HW_VIVID_PT_msg_id_string_17, "[HW Vivid Passthru]HW_VIVID_PT not exit, just ack.", 0);
        aud_msg_ack(msg.ccni_message[0] >> 16 | 0x8000, FALSE);
    }

    hw_vivid_passthru_if.transform = NULL;
    hw_vivid_passthru_reinitialize();
}

void dsp_hw_vivid_pt_close(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
#ifdef PRELOADER_ENABLE
    DSP_PIC_FeatureDeinit(hw_vivid_passthru_if.pfeature_table);
#endif
    HWVIVID_TRIGGER_FRAMEWORK_END = false;
    HW_VIVID_PT_LOG_I(g_HW_VIVID_PT_msg_id_string_18, "[HW Vivid Passthru]HW_VIVID_PT Close", 0);
    SourceClose(hw_vivid_passthru_if.source);
    SinkClose(hw_vivid_passthru_if.sink);
    memset(&hw_vivid_passthru_if, 0, sizeof(CONNECTION_IF));
}

/*
 * Get hw vivid pt source buffer
 * Units: sample
*/
ATTR_TEXT_IN_IRAM_LEVEL_2 uint32_t audio_hw_vivid_pt_query_data_amount(void)
{
    volatile SOURCE local_source = Source_blks[SOURCE_TYPE_HW_VIVID_PT];
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

