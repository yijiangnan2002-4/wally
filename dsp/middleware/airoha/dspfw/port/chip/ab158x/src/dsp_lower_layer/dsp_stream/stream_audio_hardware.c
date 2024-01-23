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

/*!
 *@file   stream_audio_hardware.c
 *@brief  Defines the hardware control for audio stream
 *
 @verbatim
         * Programmer : SYChiu@airoha.com.tw, Ext.3307
         * Programmer : BrianChen@airoha.com.tw, Ext.2641
         * Programmer : MachiWu@airoha.com.tw, Ext.2673
 @endverbatim
 */

//-
#include "types.h"
#include "audio_config.h"
#include "source_inter.h"
#include "sink_inter.h"
#include "stream_audio_setting.h"
#include "dtm.h"
#include "stream_audio_driver.h"
#include "stream_audio_hardware.h"
#include "hal_audio_afe_control.h"
#include "dsp_audio_ctrl.h"
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_mcu_dsp_common.h"
#endif
#ifdef AIR_HFP_DNN_PATH_ENABLE
#include "dsp_scenario.h"
#endif
#ifdef AIR_DCHS_MODE_ENABLE
#include "stream_dchs.h"
#endif
U16  ADC_SOFTSTART;
EXTERN afe_t afe;


#define AUDIO_AFE_DL_DEFAULT_FRAME_NUM 4
#define AUDIO_AFE_UL_DEFAULT_FRAME_NUM 4

// NOTE: 4096 for HWSRC i2s-tracking buffer's size at 48K side
// TODO: 8192 for HWSRC i2s-tracking buffer's size at 96K side
#ifdef AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ
#define AUDIO_AFE_SOURCE_ASRC_BUFFER_SIZE 8192
#else
#define AUDIO_AFE_SOURCE_ASRC_BUFFER_SIZE 4096
#endif
void afe_dl2_interrupt_handler(void);
void afe_dl1_interrupt_handler(void);
#if defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined (AIR_DCHS_MODE_ENABLE) || defined(AIR_WIRELESS_MIC_RX_ENABLE) || defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
void afe_dl3_interrupt_handler(void);
#endif
#if defined (AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_MIXER_STREAM_ENABLE)
void afe_dl12_interrupt_handler(void);
#endif
void afe_vul1_interrupt_handler(void);
void i2s_slave_ul_interrupt_handler(void);
void i2s_slave_dl_interrupt_handler(void);
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
void i2s_slave_ul_tdm_interrupt_handler(void);
void i2s_slave_dl_tdm_interrupt_handler(void);
#endif
void Source_device_set_para(hal_audio_device_parameter_t *device_handle);
hal_audio_bias_selection_t micbias_para_convert(uint32_t  in_misc_parms);


VOID Sink_Audio_Get_Default_Parameters(SINK sink)
{
    AUDIO_PARAMETER *pAudPara = &sink->param.audio;
    hal_audio_format_t format;
    uint32_t media_frame_samples;

    hal_audio_path_parameter_t *path_handle = &sink->param.audio.path_handle;
    hal_audio_memory_parameter_t *mem_handle = &sink->param.audio.mem_handle;
    hal_audio_device_parameter_t *device_handle = &sink->param.audio.device_handle;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    hal_audio_device_parameter_t *device_handle1 = &sink->param.audio.device_handle1;
    hal_audio_device_parameter_t *device_handle2 = &sink->param.audio.device_handle2;
#endif
    memset(&sink->param.audio.AfeBlkControl, 0, sizeof(afe_block_t));

    pAudPara->clk_skew_mode = Audio_setting->Audio_sink.clkskew_mode;

    format = gAudioCtrl.Afe.AfeDLSetting.format ;
    /* calculate memory size for delay */
    if (format == HAL_AUDIO_PCM_FORMAT_S32_LE ||
        format == HAL_AUDIO_PCM_FORMAT_U32_LE ||
        format == HAL_AUDIO_PCM_FORMAT_S24_LE ||
        format == HAL_AUDIO_PCM_FORMAT_U24_LE) {
        pAudPara->format_bytes = 4;
    } else {
        pAudPara->format_bytes = 2;
    }

    pAudPara->format        = format;

    pAudPara->channel_num = ((sink->param.audio.channel_sel == AUDIO_CHANNEL_A) ||
                             (sink->param.audio.channel_sel == AUDIO_CHANNEL_B) ||
                             (sink->param.audio.channel_sel == AUDIO_CHANNEL_VP))
                            ? 1 : 2;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    if (sink->param.audio.channel_sel == AUDIO_CHANNEL_4ch) {
        pAudPara->channel_num = 4;
    } else if (sink->param.audio.channel_sel == AUDIO_CHANNEL_6ch) {
        pAudPara->channel_num = 6;
    }
#endif
    pAudPara->rate          = gAudioCtrl.Afe.AfeDLSetting.rate;
    pAudPara->src_rate      = gAudioCtrl.Afe.AfeDLSetting.src_rate;
    pAudPara->period        = gAudioCtrl.Afe.AfeDLSetting.period;           /* ms, how many period to trigger */
#if defined(ENABLE_HWSRC_ON_MAIN_STREAM) || defined(MTK_HWSRC_IN_STREAM)
    pAudPara->hwsrc_type    = gAudioCtrl.Afe.AfeDLSetting.hwsrc_type; 
#endif
#if 1   // for FPGA early porting
    if (sink->type == SINK_TYPE_VP_AUDIO) {
        pAudPara->sw_channels = pAudPara->channel_num;
        media_frame_samples = Audio_setting->Audio_VP.Frame_Size;
    } else {
        pAudPara->sw_channels = Audio_setting->Audio_sink.Software_Channel_Num;
        media_frame_samples = Audio_setting->Audio_sink.Frame_Size;//Audio_setting->Audio_sink.Frame_Size;//AUDIO_AAC_FRAME_SAMPLES;
    }
#else
    switch (gAudioCtrl.Afe.OperationMode) {
        case AU_AFE_OP_ESCO_VOICE_MODE:
            media_frame_samples = AUDIO_SBC_FRAME_SAMPLES;  // use mSBC (worst case)
            break;
        case AU_AFE_OP_PLAYBACK_MODE:
            media_frame_samples = AUDIO_AAC_FRAME_SAMPLES; // TODO:
            break;
        default:
            media_frame_samples = AUDIO_SBC_FRAME_SAMPLES;
            break;
    }
#endif

#if defined (AIR_ADVANCED_PASSTHROUGH_ENABLE)
    if ((sink->type == SINK_TYPE_AUDIO_DL3) && (Audio_setting->Audio_sink.Buffer_Frame_Num != AUDIO_AFE_DL_DEFAULT_FRAME_NUM) && (Audio_setting->Audio_sink.Buffer_Frame_Num > 0))
    {
        pAudPara->buffer_size   = Audio_setting->Audio_sink.Buffer_Frame_Num * media_frame_samples *
                              pAudPara->channel_num * pAudPara->format_bytes;
    }
    else
#endif
#if defined (AIR_WIRED_AUDIO_ENABLE)
    if ((gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0) || (gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN))
    {
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
        if (pAudPara->channel_num >= 2) {
            DSP_MW_LOG_I("[MULTI_STREAM] channel_num acutual: %d, force to 2\r\n", 1, pAudPara->channel_num);
            pAudPara->buffer_size = Audio_setting->Audio_sink.Buffer_Frame_Num * media_frame_samples * 2 * pAudPara->format_bytes;
        } else {
            pAudPara->buffer_size = Audio_setting->Audio_sink.Buffer_Frame_Num * media_frame_samples * pAudPara->channel_num * pAudPara->format_bytes;
        }
#else
        pAudPara->buffer_size   = Audio_setting->Audio_sink.Buffer_Frame_Num * media_frame_samples *
                            pAudPara->channel_num * pAudPara->format_bytes;
#endif
    }
    else
#endif
#if defined (AIR_WIRELESS_MIC_RX_ENABLE)
    if ((gAudioCtrl.Afe.AfeDLSetting.scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_LINE_OUT) && (gAudioCtrl.Afe.AfeDLSetting.scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0))
    {
        pAudPara->buffer_size   = Audio_setting->Audio_sink.Buffer_Frame_Num * media_frame_samples * pAudPara->channel_num * pAudPara->format_bytes;
    }
    else
#endif
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    if ((gAudioCtrl.Afe.AfeDLSetting.scenario_type >= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0) && (gAudioCtrl.Afe.AfeDLSetting.scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0))
    {
        pAudPara->buffer_size   = Audio_setting->Audio_sink.Buffer_Frame_Num * media_frame_samples * pAudPara->channel_num * pAudPara->format_bytes;
    }
    else
#endif
    {
        if (sink->type == SINK_TYPE_VP_AUDIO) {
            pAudPara->buffer_size = Audio_setting->Audio_VP.Buffer_Frame_Num * media_frame_samples * pAudPara->channel_num * pAudPara->format_bytes;
            DSP_MW_LOG_I("audio sink default buffer size:%d = frame num:%d * frame sample:%d * channel num:%d * format_bytes: %d \r\n",5,pAudPara->buffer_size, Audio_setting->Audio_VP.Buffer_Frame_Num, media_frame_samples, pAudPara->channel_num, pAudPara->format_bytes);
        } else {
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
            if (pAudPara->channel_num >= 2) {
                DSP_MW_LOG_I("[MULTI_STREAM] channel_num acutual: %d, force to 2\r\n", 1, pAudPara->channel_num);
                pAudPara->buffer_size = Audio_setting->Audio_sink.Buffer_Frame_Num * media_frame_samples * 2 * pAudPara->format_bytes;
            } else {
                pAudPara->buffer_size = Audio_setting->Audio_sink.Buffer_Frame_Num * media_frame_samples * pAudPara->channel_num * pAudPara->format_bytes;
            }
#else
            pAudPara->buffer_size = Audio_setting->Audio_sink.Buffer_Frame_Num * media_frame_samples * pAudPara->channel_num * pAudPara->format_bytes;
#endif
        }
    }
    DSP_MW_LOG_I("audio sink default buffer size:%d = frame num:%d * frame sample:%d * channel num:%d * format_bytes: %d \r\n",5,pAudPara->buffer_size, Audio_setting->Audio_sink.Buffer_Frame_Num, media_frame_samples, pAudPara->channel_num, pAudPara->format_bytes);
    pAudPara->AfeBlkControl.u4asrc_buffer_size = pAudPara->buffer_size;

    if (pAudPara->period == 1) {
#if defined(AIR_WIRED_AUDIO_ENABLE) && defined(AIR_USB_IN_LATENCY_LOW)
        pAudPara->count         = (pAudPara->rate * pAudPara->period) / 1000;
#else
        /* Gaming Headset Customized Period */
        pAudPara->count         = 120;
#endif
    } else {
        pAudPara->count         = (pAudPara->rate * pAudPara->period) / 1000;
    }

    if (pAudPara->count >= pAudPara->buffer_size) {
        pAudPara->count = pAudPara->buffer_size >> 2;
    }
    if (pAudPara->period == 0) {
        pAudPara->count = (media_frame_samples * pAudPara->rate) / pAudPara->src_rate;
        pAudPara->period = pAudPara->count / (pAudPara->rate / 1000);
        if((media_frame_samples * pAudPara->rate) % pAudPara->src_rate) {
            pAudPara->irq_compen_flag = true;
        }
    }
    pAudPara->audio_device                   = gAudioCtrl.Afe.AfeDLSetting.audio_device;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    pAudPara->audio_device1                  = gAudioCtrl.Afe.AfeDLSetting.audio_device1;
    pAudPara->audio_device2                  = gAudioCtrl.Afe.AfeDLSetting.audio_device2;
#endif
// #ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
//     pAudPara->audio_device1                  = HAL_AUDIO_CONTROL_NONE;
//     pAudPara->audio_device2                  = HAL_AUDIO_CONTROL_NONE;
//     pAudPara->audio_device3                  = HAL_AUDIO_CONTROL_NONE;
//     pAudPara->audio_device4                  = HAL_AUDIO_CONTROL_NONE;
//     pAudPara->audio_device5                  = HAL_AUDIO_CONTROL_NONE;
//     pAudPara->audio_device6                  = HAL_AUDIO_CONTROL_NONE;
//     pAudPara->audio_device7                  = HAL_AUDIO_CONTROL_NONE;
// #endif
    pAudPara->stream_channel                 = gAudioCtrl.Afe.AfeDLSetting.stream_channel;
    pAudPara->memory                         = gAudioCtrl.Afe.AfeDLSetting.memory;
    pAudPara->audio_interface                = gAudioCtrl.Afe.AfeDLSetting.audio_interface;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    pAudPara->audio_interface1               = gAudioCtrl.Afe.AfeDLSetting.audio_interface1;
    pAudPara->audio_interface2               = gAudioCtrl.Afe.AfeDLSetting.audio_interface2;
#endif
// #ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
//     pAudPara->audio_interface1               = HAL_AUDIO_INTERFACE_NONE;
//     pAudPara->audio_interface2               = HAL_AUDIO_INTERFACE_NONE;
//     pAudPara->audio_interface3               = HAL_AUDIO_INTERFACE_NONE;
//     pAudPara->audio_interface4               = HAL_AUDIO_INTERFACE_NONE;
//     pAudPara->audio_interface5               = HAL_AUDIO_INTERFACE_NONE;
//     pAudPara->audio_interface6               = HAL_AUDIO_INTERFACE_NONE;
//     pAudPara->audio_interface7               = HAL_AUDIO_INTERFACE_NONE;
// #endif
    pAudPara->hw_gain                        = gAudioCtrl.Afe.AfeDLSetting.hw_gain;
    pAudPara->echo_reference                 = gAudioCtrl.Afe.AfeDLSetting.echo_reference;
#ifdef AUTO_ERROR_SUPPRESSION
    pAudPara->misc_parms.I2sClkSourceType    = gAudioCtrl.Afe.AfeDLSetting.misc_parms.I2sClkSourceType;
    pAudPara->misc_parms.MicbiasSourceType   = gAudioCtrl.Afe.AfeDLSetting.misc_parms.MicbiasSourceType;
#endif
#ifdef AIR_HFP_DNN_PATH_ENABLE
    pAudPara->enable_ul_dnn                  = gAudioCtrl.Afe.AfeDLSetting.enable_ul_dnn;
#endif
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    DSP_MW_LOG_I("[Sink Common] audio sink default device:0x%x 0x%x 0x%x interface:0x%x 0x%x 0x%x in rate:%d out rate:%d buffer_size:%d, count:%d memory:0x%x channel:%d hw_gain:%d", 13,
                pAudPara->audio_device,
                pAudPara->audio_device1,
                pAudPara->audio_device2,
                pAudPara->audio_interface,
                pAudPara->audio_interface1,
                pAudPara->audio_interface2,
                pAudPara->src_rate,
                pAudPara->rate,
                pAudPara->buffer_size,
                pAudPara->count,
                pAudPara->memory,
                pAudPara->stream_channel,
                pAudPara->hw_gain
                );
#else
    DSP_MW_LOG_I("[Sink Common] audio sink default device:0x%x interface:0x%x in rate:%d out rate:%d buffer_size:%d, count:%d memory:0x%x channel:%d hw_gain:%d", 9,
                pAudPara->audio_device,
                 pAudPara->audio_interface,
                pAudPara->src_rate,
                pAudPara->rate,
                pAudPara->buffer_size,
                pAudPara->count,
                pAudPara->memory,
                pAudPara->stream_channel,
                pAudPara->hw_gain
                );
#endif
    pAudPara->with_sink_src = false;//HWSRC Continuous mode
    //for hal_audio_set_memory
    mem_handle->scenario_type = gAudioCtrl.Afe.AfeDLSetting.scenario_type;
    mem_handle->buffer_length = pAudPara->buffer_size;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
        hal_audio_memory_parameter_t *mem_handle1 = &sink->param.audio.mem_handle1;
        hal_audio_memory_parameter_t *mem_handle2 = &sink->param.audio.mem_handle2;
        if (pAudPara->channel_num == 4) {
            if (mem_handle->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM) {
                mem_handle->memory_select = hal_memory_convert_dl(pAudPara->memory & HAL_AUDIO_MEM3);
                mem_handle1->memory_select = hal_memory_convert_dl(pAudPara->memory & HAL_AUDIO_MEM1);
            } else {
                mem_handle->memory_select = hal_memory_convert_dl(pAudPara->memory & HAL_AUDIO_MEM1);
                mem_handle1->memory_select = hal_memory_convert_dl(pAudPara->memory & HAL_AUDIO_MEM3);
            }
            DSP_MW_LOG_I("[MULTI_STREAM] pAudPara->memory 0x%x, 0x%x, 0x%x ", 3, pAudPara->memory, mem_handle->memory_select, mem_handle1->memory_select);
        } else if (pAudPara->channel_num == 6){
            if (mem_handle->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM) {
                mem_handle->memory_select = hal_memory_convert_dl(pAudPara->memory & HAL_AUDIO_MEM3);
                mem_handle1->memory_select = hal_memory_convert_dl(pAudPara->memory & HAL_AUDIO_MEM1);
            } else {
                mem_handle->memory_select = hal_memory_convert_dl(pAudPara->memory & HAL_AUDIO_MEM1);
                mem_handle1->memory_select = hal_memory_convert_dl(pAudPara->memory & HAL_AUDIO_MEM3);
            }
            mem_handle2->memory_select = hal_memory_convert_dl(pAudPara->memory & HAL_AUDIO_MEM4);
            DSP_MW_LOG_I("[MULTI_STREAM] pAudPara->memory 0x%x ", 1, pAudPara->memory);
        } else {
            mem_handle->memory_select = hal_memory_convert_dl(pAudPara->memory);
        }
#else
    mem_handle->memory_select = hal_memory_convert_dl(pAudPara->memory);
#endif
    mem_handle->irq_counter = pAudPara->count;
    mem_handle->pcm_format = (hal_audio_format_t)pAudPara->format;
#ifdef ENABLE_HWSRC_CLKSKEW
    if(Audio_setting->Audio_sink.clkskew_mode == CLK_SKEW_V2) {
        mem_handle->asrc_clkskew_mode = HAL_AUDIO_SRC_CLK_SKEW_V2;
    }
#endif
#if 1
    if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL1) {
        hal_audio_set_value_parameter_t handle;
            handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_AUDIOSYS;
            handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_DL_DL1;
            handle.register_irq_handler.entry = afe_dl1_interrupt_handler;
        hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
    } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL2) {
#ifdef MTK_PROMPT_SOUND_ENABLE
        hal_audio_set_value_parameter_t handle;
            handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_AUDIOSYS;
            handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_DL_DL2;
            handle.register_irq_handler.entry = afe_dl2_interrupt_handler;
        hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
#endif
    } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL3) {
#if defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined (AIR_DCHS_MODE_ENABLE) || defined(AIR_WIRELESS_MIC_RX_ENABLE) || defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        hal_audio_set_value_parameter_t handle;
        handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_AUDIOSYS;
        handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_DL_DL3;
        handle.register_irq_handler.entry = afe_dl3_interrupt_handler;
        hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
#endif
    } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL12) {
#if defined (AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_MIXER_STREAM_ENABLE)
        hal_audio_set_value_parameter_t handle;
        handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_AUDIOSYS;
        handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_DL_DL12;
        handle.register_irq_handler.entry = afe_dl12_interrupt_handler;
        hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
#endif
    } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_SLAVE_DMA) {
        hal_audio_set_value_parameter_t handle;
        handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_I2S_SLAVE;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_DL_SLAVE_DMA;
#endif
        handle.register_irq_handler.entry = i2s_slave_dl_interrupt_handler;
        hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
    } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_SLAVE_TDM) {
        hal_audio_set_value_parameter_t handle;
        handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_I2S_SLAVE;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_DL_SLAVE_TDM;
        handle.register_irq_handler.entry = i2s_slave_dl_tdm_interrupt_handler;
#endif
        hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
    }
    mem_handle->with_mono_channel = (pAudPara->channel_num == 1) ? true : false;

    //for hal_audio_set_path
    path_handle->scenario_type = gAudioCtrl.Afe.AfeDLSetting.scenario_type;
    path_handle->connection_number = pAudPara->channel_num;
    if ((sink->type == SINK_TYPE_VP_AUDIO) &&
        ((pAudPara->audio_device != HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL)) && (pAudPara->audio_device != HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER)  && (pAudPara->audio_device != HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE)) {
        path_handle->connection_number = 1;
    }

    uint32_t i;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
	hal_audio_path_port_parameter_t input_port_parameters[3], output_port_parameters[3];
#else
    hal_audio_path_port_parameter_t input_port_parameters[2], output_port_parameters[2];
#endif
    input_port_parameters[0].memory_select = mem_handle->memory_select;
    output_port_parameters[0].device_interface = (hal_audio_interface_t)pAudPara->audio_interface;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    if (pAudPara->channel_num >= 4) {
        input_port_parameters[1].memory_select = mem_handle1->memory_select;
        output_port_parameters[1].device_interface = pAudPara->audio_interface1;
        if (pAudPara->channel_num >= 6) {
            input_port_parameters[2].memory_select = mem_handle2->memory_select;
            output_port_parameters[2].device_interface = pAudPara->audio_interface2;
        }
    }
#endif
#ifdef AIR_HFP_DNN_PATH_ENABLE
    if ((pAudPara->enable_ul_dnn == true) && ((mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL3) || (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL3))) {
        uint32_t connection_sequence = mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL3 ? 1 : 0;
        path_handle->connection_number = 1;
        path_handle->input.interconn_sequence[0]  = stream_audio_convert_control_to_interconn(HAL_AUDIO_CONTROL_MEMORY_INTERFACE, input_port_parameters[0], 0, false);
        path_handle->output.interconn_sequence[0] = stream_audio_convert_control_to_interconn(pAudPara->audio_device, output_port_parameters[0], connection_sequence, false);
        DSP_MW_LOG_I("[Sink Common][DNN] audio sink default enable_ul_dnn: %d, mem_handle->memory_select:0x%x", 2, pAudPara->enable_ul_dnn, mem_handle->memory_select);
    } else
#endif
    {
        for (i = 0 ; i < path_handle->connection_number ; i++) {
            path_handle->input.interconn_sequence[i]  = stream_audio_convert_control_to_interconn(HAL_AUDIO_CONTROL_MEMORY_INTERFACE, input_port_parameters[i/2], (mem_handle->with_mono_channel) ? 0 : i, false);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
            if(i < 2) {
                path_handle->output.interconn_sequence[i] = stream_audio_convert_control_to_interconn(pAudPara->audio_device, output_port_parameters[i/2], i, false);
            } else if (i < 4) {
                path_handle->output.interconn_sequence[i] = stream_audio_convert_control_to_interconn(pAudPara->audio_device1, output_port_parameters[i/2], i, false);
            } else if (i < 6) {
                path_handle->output.interconn_sequence[i] = stream_audio_convert_control_to_interconn(pAudPara->audio_device2, output_port_parameters[i/2], i, false);
            }
#else
            path_handle->output.interconn_sequence[i] = stream_audio_convert_control_to_interconn(pAudPara->audio_device, output_port_parameters[i/2], i, false);
#endif
            path_handle->audio_input_rate[i] = pAudPara->rate;//afe_get_audio_device_samplerate(pAudPara->audio_device , pAudPara->audio_interface);
            path_handle->audio_output_rate[i] = pAudPara->rate;//afe_get_audio_device_samplerate(pAudPara->audio_device , pAudPara->audio_interface);
            path_handle->with_updown_sampler[i] = false;
        }
    }
    path_handle->with_hw_gain = pAudPara->hw_gain;
    path_handle->with_dl_deq_mixer = false;//for anc & deq

    switch (pAudPara->stream_channel) {
        case     HAL_AUDIO_DIRECT:
            path_handle->connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
            break;
        case     HAL_AUDIO_SWAP_L_R:
            path_handle->connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH02CH01;
            break;
        case     HAL_AUDIO_MIX_L_R:
            path_handle->connection_selection = HAL_AUDIO_INTERCONN_MIX;
            break;
        default:
            path_handle->connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
            break;
    }
    //for hal_audio_set_device
    device_handle->common.audio_device = pAudPara->audio_device;
    device_handle->common.device_interface = pAudPara->audio_interface;

    extern void set_device_handle_param(hal_audio_device_parameter_t *device_handle, AUDIO_PARAMETER *pAudPara, hal_audio_memory_parameter_t *mem_handle);
    if(pAudPara->audio_device != HAL_AUDIO_CONTROL_NONE) {
        set_device_handle_param(device_handle, pAudPara, mem_handle);
    }

#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    device_handle1->common.audio_device = pAudPara->audio_device1;
    device_handle2->common.audio_device = pAudPara->audio_device2;
    device_handle1->common.device_interface = pAudPara->audio_interface1;
    device_handle2->common.device_interface = pAudPara->audio_interface2;

    if(pAudPara->audio_device1 != HAL_AUDIO_CONTROL_NONE) {
        set_device_handle_param(device_handle1, pAudPara, mem_handle1);
    }
    if(pAudPara->audio_device2 != HAL_AUDIO_CONTROL_NONE) {
        set_device_handle_param(device_handle2, pAudPara, mem_handle2);
    }
#endif
}

void set_device_handle_param(hal_audio_device_parameter_t *device_handle, AUDIO_PARAMETER *pAudPara, hal_audio_memory_parameter_t *mem_handle)
{
    device_handle->common.scenario_type = gAudioCtrl.Afe.AfeDLSetting.scenario_type;
    device_handle->common.is_tx = true;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    gAudioCtrl.Afe.AfeDLSetting.i2s_master_format[0] = HAL_AUDIO_I2S_I2S;
    gAudioCtrl.Afe.AfeDLSetting.i2s_master_word_length[0] = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;

    gAudioCtrl.Afe.AfeDLSetting.i2s_master_format[1] = HAL_AUDIO_I2S_I2S;
    gAudioCtrl.Afe.AfeDLSetting.i2s_master_word_length[1] = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;

    gAudioCtrl.Afe.AfeDLSetting.i2s_master_format[2] = HAL_AUDIO_I2S_I2S;
    gAudioCtrl.Afe.AfeDLSetting.i2s_master_word_length[2] = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;

    gAudioCtrl.Afe.AfeDLSetting.i2s_master_format[3] = HAL_AUDIO_I2S_I2S;
    gAudioCtrl.Afe.AfeDLSetting.i2s_master_word_length[3] = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;

#endif
    if (device_handle->common.audio_device & HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL) {
        device_handle->dac.rate = pAudPara->rate;
        device_handle->dac.dac_mode = gAudioCtrl.Afe.AfeDLSetting.adc_mode;//HAL_AUDIO_ANALOG_OUTPUT_CLASSAB;
        device_handle->dac.dc_compensation_value = afe.stream_out.dc_compensation_value;
#if defined(AIR_BTA_IC_PREMIUM_G2)
        device_handle->dac.with_high_performance = gAudioCtrl.Afe.AfeDLSetting.performance;
#else
        device_handle->dac.performance = gAudioCtrl.Afe.AfeDLSetting.performance;
#endif
        device_handle->dac.with_phase_inverse = false;
        device_handle->dac.with_force_change_rate = true;
    } else if (device_handle->common.audio_device & (HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R)) {
        if (device_handle->i2s_master.i2s_interface & HAL_AUDIO_INTERFACE_1) {
            device_handle->i2s_master.rate = (gAudioCtrl.Afe.AfeDLSetting.i2s_master_sampling_rate[0]>0)?gAudioCtrl.Afe.AfeDLSetting.i2s_master_sampling_rate[0]:pAudPara->rate;
            device_handle->i2s_master.i2s_format = gAudioCtrl.Afe.AfeDLSetting.i2s_master_format[0];
            device_handle->i2s_master.word_length = gAudioCtrl.Afe.AfeDLSetting.i2s_master_word_length[0];
            device_handle->i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeDLSetting.is_low_jitter[0];
        } else if (device_handle->i2s_master.i2s_interface & HAL_AUDIO_INTERFACE_2) {
            device_handle->i2s_master.rate = (gAudioCtrl.Afe.AfeDLSetting.i2s_master_sampling_rate[1]>0)?gAudioCtrl.Afe.AfeDLSetting.i2s_master_sampling_rate[1]:pAudPara->rate;
            device_handle->i2s_master.i2s_format = gAudioCtrl.Afe.AfeDLSetting.i2s_master_format[1];
            device_handle->i2s_master.word_length = gAudioCtrl.Afe.AfeDLSetting.i2s_master_word_length[1];
            device_handle->i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeDLSetting.is_low_jitter[1];
        } else if (device_handle->i2s_master.i2s_interface & HAL_AUDIO_INTERFACE_3) {
            device_handle->i2s_master.rate = (gAudioCtrl.Afe.AfeDLSetting.i2s_master_sampling_rate[2]>0)?gAudioCtrl.Afe.AfeDLSetting.i2s_master_sampling_rate[2]:pAudPara->rate;
            device_handle->i2s_master.i2s_format = gAudioCtrl.Afe.AfeDLSetting.i2s_master_format[2];
            device_handle->i2s_master.word_length = gAudioCtrl.Afe.AfeDLSetting.i2s_master_word_length[2];
            device_handle->i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeDLSetting.is_low_jitter[2];
        } else if (device_handle->i2s_master.i2s_interface & HAL_AUDIO_INTERFACE_4) {
            device_handle->i2s_master.rate = (gAudioCtrl.Afe.AfeDLSetting.i2s_master_sampling_rate[3]>0)?gAudioCtrl.Afe.AfeDLSetting.i2s_master_sampling_rate[3]:pAudPara->rate;
            device_handle->i2s_master.i2s_format = gAudioCtrl.Afe.AfeDLSetting.i2s_master_format[3];
            device_handle->i2s_master.word_length = gAudioCtrl.Afe.AfeDLSetting.i2s_master_word_length[3];
            device_handle->i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeDLSetting.is_low_jitter[3];
        }
        if (device_handle->i2s_master.rate > 48000) {
            device_handle->i2s_master.is_low_jitter = true;
        }
        device_handle->i2s_master.mclk_divider = 2;
        device_handle->i2s_master.with_mclk = false;
        device_handle->i2s_master.is_recombinant = false;
        pAudPara->rate = device_handle->i2s_master.rate;
    } else if (device_handle->common.audio_device & HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        device_handle->i2s_slave.rate = pAudPara->rate;
        device_handle->i2s_slave.i2s_format = gAudioCtrl.Afe.AfeDLSetting.i2s_format;
        device_handle->i2s_slave.word_length = gAudioCtrl.Afe.AfeDLSetting.i2s_word_length;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        device_handle->i2s_slave.tdm_channel = gAudioCtrl.Afe.AfeULSetting.tdm_channel;
#endif
        device_handle->i2s_slave.memory_select = mem_handle->memory_select;
        if ((device_handle->i2s_slave.memory_select == HAL_AUDIO_MEMORY_DL_SLAVE_DMA) || (device_handle->i2s_slave.memory_select == HAL_AUDIO_MEMORY_DL_SLAVE_TDM)) {
            device_handle->i2s_slave.is_vdma_mode = true;
        } else {
            device_handle->i2s_slave.is_vdma_mode = false;
        }
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        if (device_handle->i2s_slave.memory_select == HAL_AUDIO_MEMORY_DL_SLAVE_TDM) {
            if (device_handle->i2s_slave.i2s_interface == HAL_AUDIO_INTERFACE_1) {
                AUDIO_ASSERT(0 && "[Sink Common] DL I2S Slave0 not support TDM mode");
            }
            if ((device_handle->i2s_slave.tdm_channel != HAL_AUDIO_I2S_TDM_4CH) && (device_handle->i2s_slave.tdm_channel != HAL_AUDIO_I2S_TDM_6CH) && (device_handle->i2s_slave.tdm_channel != HAL_AUDIO_I2S_TDM_8CH)) {
                DSP_MW_LOG_I("[Sink Common] DL I2S Slave tdm channel : %d invalid and assert", 1, device_handle->i2s_slave.tdm_channel);
                AUDIO_ASSERT(0 && "[Sink Common] DL I2S Slave tdm channel invalid");
            }
        }
#endif
#ifdef AIR_HWSRC_TX_TRACKING_ENABLE
        if(pAudPara->clk_skew_mode == CLK_SKEW_DISSABLE) {
            if (pAudPara->audio_interface == HAL_AUDIO_INTERFACE_1) {
                mem_handle->src_tracking_clock_source =  HAL_AUDIO_SRC_TRACKING_I2S1;
            } else if (pAudPara->audio_interface == HAL_AUDIO_INTERFACE_2) {
                mem_handle->src_tracking_clock_source =  HAL_AUDIO_SRC_TRACKING_I2S2;
            } else if (pAudPara->audio_interface == HAL_AUDIO_INTERFACE_3) {
                mem_handle->src_tracking_clock_source =  HAL_AUDIO_SRC_TRACKING_I2S3;
            } else if (pAudPara->audio_interface == HAL_AUDIO_INTERFACE_4) {
                mem_handle->src_tracking_clock_source =  HAL_AUDIO_SRC_TRACKING_I2S4;
            }
            DSP_MW_LOG_I("[Sink Common] audio sink default i2s slave tracking source = %d", 1, mem_handle->src_tracking_clock_source);
        }
#endif
    } else if (device_handle->common.audio_device & HAL_AUDIO_CONTROL_DEVICE_SPDIF) {
        device_handle->spdif.i2s_setting.rate = pAudPara->rate;
        device_handle->spdif.i2s_setting.i2s_format = HAL_AUDIO_I2S_I2S;
        device_handle->spdif.i2s_setting.word_length = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;
        device_handle->spdif.i2s_setting.mclk_divider = 2;
        device_handle->spdif.i2s_setting.with_mclk = false;
        device_handle->spdif.i2s_setting.is_low_jitter = false;
        device_handle->spdif.i2s_setting.is_recombinant = false;
    }
#endif
}

hal_audio_bias_selection_t micbias_para_convert(uint32_t  in_misc_parms)
{
    hal_audio_bias_selection_t bias_selection;
    bias_selection = in_misc_parms >> 20;
    return bias_selection;
}

VOID Source_Audio_Get_Default_Parameters(SOURCE source)
{
    AUDIO_PARAMETER *pAudPara = &source->param.audio;
    hal_audio_format_t format;
    uint32_t media_frame_samples;
    hal_audio_path_parameter_t *path_handle = &source->param.audio.path_handle;
    hal_audio_memory_parameter_t *mem_handle = &source->param.audio.mem_handle;
    hal_audio_device_parameter_t *device_handle = &source->param.audio.device_handle;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    hal_audio_device_parameter_t *device_handle1 = &source->param.audio.device_handle1;
    hal_audio_device_parameter_t *device_handle2 = &source->param.audio.device_handle2;
    hal_audio_device_parameter_t *device_handle3 = &source->param.audio.device_handle3;
    hal_audio_device_parameter_t *device_handle4 = &source->param.audio.device_handle4;
    hal_audio_device_parameter_t *device_handle5 = &source->param.audio.device_handle5;
    hal_audio_device_parameter_t *device_handle6 = &source->param.audio.device_handle6;
    hal_audio_device_parameter_t *device_handle7 = &source->param.audio.device_handle7;
#endif
    memset(&source->param.audio.AfeBlkControl, 0, sizeof(afe_block_t));

    pAudPara->clk_skew_mode = Audio_setting->Audio_source.clkskew_mode;

    format = gAudioCtrl.Afe.AfeULSetting.format;
    /*if (pAudPara->audio_device & (HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R)) {
        if (gAudioCtrl.Afe.AfeULSetting.i2s_master_word_length == HAL_AUDIO_I2S_WORD_LENGTH_16BIT)
        {
            format ==  HAL_AUDIO_PCM_FORMAT_S16_LE;
        } else if (gAudioCtrl.Afe.AfeULSetting.i2s_master_word_length == HAL_AUDIO_I2S_WORD_LENGTH_32BIT)
        {
            format ==  HAL_AUDIO_PCM_FORMAT_S32_LE;
        }
    }*/
    /* calculate memory size for delay */
    if (format == HAL_AUDIO_PCM_FORMAT_S32_LE ||
        format == HAL_AUDIO_PCM_FORMAT_U32_LE ||
        format == HAL_AUDIO_PCM_FORMAT_S24_LE ||
        format == HAL_AUDIO_PCM_FORMAT_U24_LE) {
        pAudPara->format_bytes = 4;
    } else {
        pAudPara->format_bytes = 2;
    }

    pAudPara->format        = format;

    if ((pAudPara->channel_sel == AUDIO_CHANNEL_A) || (pAudPara->channel_sel == AUDIO_CHANNEL_B)) {
        pAudPara->channel_num = 1;
    } else if (pAudPara->channel_sel == AUDIO_CHANNEL_A_AND_B) {
        pAudPara->channel_num = 2;
    } else if (pAudPara->channel_sel == AUDIO_CHANNEL_3ch) {
        pAudPara->channel_num = 3;
    } else if (pAudPara->channel_sel == AUDIO_CHANNEL_4ch) {
        pAudPara->channel_num = 4;
    } else if (pAudPara->channel_sel == AUDIO_CHANNEL_5ch) {
        pAudPara->channel_num = 5;
    } else if (pAudPara->channel_sel == AUDIO_CHANNEL_6ch) {
        pAudPara->channel_num = 6;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    } else if (pAudPara->channel_sel == AUDIO_CHANNEL_8ch) {
        pAudPara->channel_num = 8;
#endif
    } else {
        pAudPara->channel_num = 2;
    }
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    pAudPara->Frame_Size = Audio_setting->Audio_source.Frame_Size;
    pAudPara->Buffer_Frame_Num = Audio_setting->Audio_source.Buffer_Frame_Num;
#endif
    // pAudPara->channel_num   = 1;
    pAudPara->rate          = gAudioCtrl.Afe.AfeULSetting.rate;
    pAudPara->src_rate      = gAudioCtrl.Afe.AfeULSetting.src_rate;
    pAudPara->period        = gAudioCtrl.Afe.AfeULSetting.period;

    // for early porting
    media_frame_samples = Audio_setting->Audio_source.Frame_Size;//AUDIO_AAC_FRAME_SAMPLES;
    pAudPara->echo_reference                 = gAudioCtrl.Afe.AfeULSetting.echo_reference;

#ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
    if (pAudPara->echo_reference) {
        pAudPara->channel_num++;
    }
#endif

    uint32_t connection_number = pAudPara->channel_num;

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    if ((gAudioCtrl.Afe.AfeULSetting.max_channel_num) && (gAudioCtrl.Afe.AfeULSetting.scenario_type != AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT)) {
        pAudPara->channel_num = gAudioCtrl.Afe.AfeULSetting.max_channel_num;
#ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
        if (pAudPara->echo_reference) {
#ifdef AIR_ECHO_PATH_STEREO_ENABLE
            pAudPara->channel_num += 2;
#else
            pAudPara->channel_num++;
#endif
        }
#endif
    }
#endif
    if((pAudPara->src_rate != 0) && (pAudPara->rate != pAudPara->src_rate)) {
        pAudPara->AfeBlkControl.u4asrcflag = true;
        pAudPara->mem_handle.pure_agent_with_src = true;
    }

    uint8_t channel_num     = (pAudPara->channel_num >= 2) ? 2 : 1;
    pAudPara->buffer_size   = media_frame_samples * Audio_setting->Audio_source.Buffer_Frame_Num * channel_num * pAudPara->format_bytes;
    DSP_MW_LOG_I("audio source default buffer size:%d = frame num:%d * frame sample:%d * channel num:%d * format_bytes: %d \r\n", 5, pAudPara->buffer_size, Audio_setting->Audio_source.Buffer_Frame_Num, media_frame_samples, channel_num, pAudPara->format_bytes);

    pAudPara->AfeBlkControl.u4asrc_buffer_size = AUDIO_AFE_SOURCE_ASRC_BUFFER_SIZE;//AUDIO_AFE_BUFFER_SIZE;//pAudPara->buffer_size;
#ifdef AIR_I2S_SLAVE_ENABLE
    if (gAudioCtrl.Afe.AfeULSetting.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        pAudPara->AfeBlkControl.u4asrc_buffer_size = pAudPara->buffer_size / 2;
    }
#endif
    //pAudPara->buffer_size   = AUDIO_SOURCE_DEFAULT_FRAME_NUM * media_frame_samples *
    //                          pAudPara->channel_num * pAudPara->format_bytes;
// printf("===> %d %d %d %d = %d\r\n", AUDIO_SOURCE_DEFAULT_FRAME_NUM, media_frame_samples, pAudPara->channel_num,pAudPara->format_bytes,   pAudPara->buffer_size);
    pAudPara->count         = (pAudPara->rate * pAudPara->period) / 1000;
    if (pAudPara->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        #ifdef AIR_HWSRC_RX_TRACKING_ENABLE
        pAudPara->count         = (pAudPara->src_rate * pAudPara->period) / 1000;
        #endif
    }
    if (pAudPara->count >= pAudPara->buffer_size) {
        pAudPara->count = pAudPara->buffer_size >> 2;
    }
    if (pAudPara->period == 0) {
        pAudPara->count = media_frame_samples;
        pAudPara->period = media_frame_samples / (pAudPara->rate / 1000);
    }
    pAudPara->audio_device                   = gAudioCtrl.Afe.AfeULSetting.audio_device;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    pAudPara->audio_device1                  = gAudioCtrl.Afe.AfeULSetting.audio_device1;
    pAudPara->audio_device2                  = gAudioCtrl.Afe.AfeULSetting.audio_device2;
    pAudPara->audio_device3                  = gAudioCtrl.Afe.AfeULSetting.audio_device3;
    pAudPara->audio_device4                  = gAudioCtrl.Afe.AfeULSetting.audio_device4;
    pAudPara->audio_device5                  = gAudioCtrl.Afe.AfeULSetting.audio_device5;
    pAudPara->audio_device6                  = gAudioCtrl.Afe.AfeULSetting.audio_device6;
    pAudPara->audio_device7                  = gAudioCtrl.Afe.AfeULSetting.audio_device7;
#endif
    pAudPara->stream_channel                 = gAudioCtrl.Afe.AfeULSetting.stream_channel;
    pAudPara->memory                         = gAudioCtrl.Afe.AfeULSetting.memory;
    pAudPara->audio_interface                = gAudioCtrl.Afe.AfeULSetting.audio_interface;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    pAudPara->audio_interface1               = gAudioCtrl.Afe.AfeULSetting.audio_interface1;
    pAudPara->audio_interface2               = gAudioCtrl.Afe.AfeULSetting.audio_interface2;
    pAudPara->audio_interface3               = gAudioCtrl.Afe.AfeULSetting.audio_interface3;
    pAudPara->audio_interface4               = gAudioCtrl.Afe.AfeULSetting.audio_interface4;
    pAudPara->audio_interface5               = gAudioCtrl.Afe.AfeULSetting.audio_interface5;
    pAudPara->audio_interface6               = gAudioCtrl.Afe.AfeULSetting.audio_interface6;
    pAudPara->audio_interface7               = gAudioCtrl.Afe.AfeULSetting.audio_interface7;
#endif
    for (uint32_t i = 0; i < 8; i++) {
        pAudPara->audio_device_rate[i]       = (gAudioCtrl.Afe.AfeULSetting.audio_device_rate[i] == 0) ? pAudPara->rate : gAudioCtrl.Afe.AfeULSetting.audio_device_rate[i];
        pAudPara->audio_memory_rate[i]       = (gAudioCtrl.Afe.AfeULSetting.audio_memory_rate[i] == 0) ? pAudPara->rate : gAudioCtrl.Afe.AfeULSetting.audio_memory_rate[i];
        DSP_MW_LOG_I("audio_device_rate[%d] = %d\r\n", 2, i, pAudPara->audio_device_rate[i]);
    }

    pAudPara->hw_gain                        = gAudioCtrl.Afe.AfeULSetting.hw_gain;
#ifdef AUTO_ERROR_SUPPRESSION
    pAudPara->misc_parms.I2sClkSourceType    = gAudioCtrl.Afe.AfeULSetting.misc_parms.I2sClkSourceType;
    pAudPara->misc_parms.MicbiasSourceType   = gAudioCtrl.Afe.AfeULSetting.misc_parms.MicbiasSourceType;
#endif
#ifdef AIR_HFP_DNN_PATH_ENABLE
    pAudPara->enable_ul_dnn                  = gAudioCtrl.Afe.AfeULSetting.enable_ul_dnn;
#endif

    DSP_MW_LOG_I("audio source default buffer_size:%d, count:%d\r\n", 2, pAudPara->buffer_size, pAudPara->count);
    DSP_MW_LOG_I("audio source default device:%d, channel:%d, memory:%d, interface:%d rate %d\r\n", 5, pAudPara->audio_device,
                 pAudPara->stream_channel,
                 pAudPara->memory,
                 pAudPara->audio_interface,
                 pAudPara->rate);

    //for hal_audio_set_memory
    mem_handle->scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
    mem_handle->buffer_length = pAudPara->buffer_size;
    mem_handle->memory_select = hal_memory_convert_ul(pAudPara->memory);

    if (!(mem_handle->memory_select & (HAL_AUDIO_MEMORY_UL_SLAVE_TDM | HAL_AUDIO_MEMORY_UL_SLAVE_DMA))) {
        if ((pAudPara->channel_num) >= 3) {
            mem_handle->memory_select |= HAL_AUDIO_MEMORY_UL_VUL2;
        }
        if ((pAudPara->channel_num) >= 5) {
            mem_handle->memory_select |= HAL_AUDIO_MEMORY_UL_VUL3;
        }
        if ((pAudPara->channel_num) >= 7) {
            mem_handle->memory_select |= HAL_AUDIO_MEMORY_UL_AWB;
        }
        if ((pAudPara->channel_num) >= 9) {
#ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
            mem_handle->memory_select |= HAL_AUDIO_MEMORY_UL_AWB2;
        }
        if ((pAudPara->channel_num) >= 11) {
#endif
            DSP_MW_LOG_W("DSP STREAM: no memory agent for more channels.", 0);
        }
    }
    mem_handle->irq_counter = pAudPara->count;
    mem_handle->pcm_format = (hal_audio_format_t)pAudPara->format;

#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE)
    if ((source->type >= SOURCE_TYPE_SUBAUDIO_MIN) && (source->type <= SOURCE_TYPE_SUBAUDIO_MAX)) {
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
        if (pAudPara->memory == (HAL_AUDIO_MEM_SUB|HAL_AUDIO_MEM4)){
            hal_audio_set_value_parameter_t handle;
            handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_AUDIOSYS;
            handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_UL_AWB;
            handle.register_irq_handler.entry = afe_subsource2_interrupt_handler;
            hal_audio_set_value((hal_audio_set_value_parameter_t *)&handle, HAL_AUDIO_SET_IRQ_HANDLER);
        }
#endif
    } else
#endif
    {
        if (mem_handle->memory_select & HAL_AUDIO_MEMORY_UL_VUL1) {
#ifdef AIR_I2S_SLAVE_ENABLE
            if (pAudPara->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
#if 0
                hal_audio_set_value_parameter_t handle;
                handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_I2S_SLAVE;
                //handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_UL_VUL1;
                handle.register_irq_handler.entry = i2s_slave_1_ul_interrupt_handler;
                DSP_MW_LOG_I("[HAS][LINEIN]HAL_AUDIO_MEMORY_UL_VUL1 memory_select %d,audio_irq %d,entry %x\r\n", 3, handle.register_irq_handler.memory_select, handle.register_irq_handler.audio_irq, handle.register_irq_handler.entry);
                hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
#endif
            } else
#endif
            {
                hal_audio_set_value_parameter_t handle;
                handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_AUDIOSYS;
                handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_UL_VUL1;
                handle.register_irq_handler.entry = afe_vul1_interrupt_handler;
                hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
            }
        } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_UL_VUL2) {

        } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_UL_SLAVE_DMA) {
            hal_audio_set_value_parameter_t handle;
            handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_I2S_SLAVE;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
            handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_UL_SLAVE_DMA;
#endif
            handle.register_irq_handler.entry = i2s_slave_ul_interrupt_handler;
            hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
        } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_UL_SLAVE_TDM) {
            hal_audio_set_value_parameter_t handle;
            handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_I2S_SLAVE;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
            handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_UL_SLAVE_TDM;
            handle.register_irq_handler.entry = i2s_slave_ul_tdm_interrupt_handler;
#endif
            hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
        }
    }
    mem_handle->with_mono_channel = (pAudPara->channel_num == 1) ? true : false;

    //path
    path_handle->scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;

    switch (pAudPara->stream_channel) {
        case     HAL_AUDIO_DIRECT:
            path_handle->connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
            break;
        case     HAL_AUDIO_SWAP_L_R:
            path_handle->connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH02CH01;
            break;
        case     HAL_AUDIO_MIX_L_R:
            path_handle->connection_selection = HAL_AUDIO_INTERCONN_MIX;
            break;
        default:
            path_handle->connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
            break;
    }
    path_handle->connection_number = connection_number;

    uint32_t i;
    hal_audio_path_port_parameter_t input_port_parameters, output_port_parameters;
    input_port_parameters.device_interface =(hal_audio_interface_t)pAudPara->audio_interface;
    #ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
    output_port_parameters.memory_select = mem_handle->memory_select;
    #else
    output_port_parameters.memory_select = mem_handle->memory_select & (~HAL_AUDIO_MEMORY_UL_AWB2);
    #endif
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    hal_audio_device_t path_audio_device[HAL_AUDIO_PATH_SUPPORT_SEQUENCE] = {
        pAudPara->audio_device, pAudPara->audio_device1, pAudPara->audio_device2, pAudPara->audio_device3
        #ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        , pAudPara->audio_device4, pAudPara->audio_device5, pAudPara->audio_device6, pAudPara->audio_device7
        #endif /* MTK_AUDIO_HW_IO_CONFIG_ENHANCE */
    };
    hal_audio_interface_t device_interface[HAL_AUDIO_PATH_SUPPORT_SEQUENCE]= {
        (hal_audio_interface_t)pAudPara->audio_interface, (hal_audio_interface_t)pAudPara->audio_interface1, (hal_audio_interface_t)pAudPara->audio_interface2, (hal_audio_interface_t)pAudPara->audio_interface3
        #ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        , (hal_audio_interface_t)pAudPara->audio_interface4, (hal_audio_interface_t)pAudPara->audio_interface5, (hal_audio_interface_t)pAudPara->audio_interface6, (hal_audio_interface_t)pAudPara->audio_interface7
        #endif /* MTK_AUDIO_HW_IO_CONFIG_ENHANCE */
    };
    hal_audio_memory_selection_t memory_select[HAL_AUDIO_PATH_SUPPORT_SEQUENCE]= {
        HAL_AUDIO_MEMORY_UL_VUL1, HAL_AUDIO_MEMORY_UL_VUL1, HAL_AUDIO_MEMORY_UL_VUL2, HAL_AUDIO_MEMORY_UL_VUL2
        #ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        , HAL_AUDIO_MEMORY_UL_VUL3, HAL_AUDIO_MEMORY_UL_VUL3, HAL_AUDIO_MEMORY_UL_AWB, HAL_AUDIO_MEMORY_UL_AWB
        #endif /* MTK_AUDIO_HW_IO_CONFIG_ENHANCE */
    };
#else
    hal_audio_device_t path_audio_device[HAL_AUDIO_PATH_SUPPORT_SEQUENCE] = {
        pAudPara->audio_device, pAudPara->audio_device
    };
    hal_audio_interface_t device_interface[HAL_AUDIO_PATH_SUPPORT_SEQUENCE] = {
        (hal_audio_interface_t)pAudPara->audio_interface, (hal_audio_interface_t)pAudPara->audio_interface
    };
    hal_audio_memory_selection_t memory_select[HAL_AUDIO_PATH_SUPPORT_SEQUENCE] = {
        output_port_parameters.memory_select, output_port_parameters.memory_select
    };
#endif

#ifdef AIR_HFP_DNN_PATH_ENABLE
    if ((pAudPara->enable_ul_dnn == true) && ((pAudPara->memory & HAL_AUDIO_MEM_SUB) || (pAudPara->memory & HAL_AUDIO_MEM1))) {
        uint32_t connection_sequence = (pAudPara->memory & HAL_AUDIO_MEM_SUB) ? 1 : 0;
        path_handle->connection_number = 1;
        input_port_parameters.device_interface = device_interface[0];
        output_port_parameters.memory_select = memory_select[0];
        path_handle->input.interconn_sequence[0]  = stream_audio_convert_control_to_interconn(path_audio_device[0], input_port_parameters, connection_sequence, true);
        path_handle->output.interconn_sequence[0] = stream_audio_convert_control_to_interconn(HAL_AUDIO_CONTROL_MEMORY_INTERFACE, output_port_parameters, 0, true);
        DSP_MW_LOG_I("[Source Common] audio source default enable_ul_dnn: %d, pAudPara->memory:0x%x", 2, pAudPara->enable_ul_dnn, pAudPara->memory);
    } else
#endif

{
    for (i=0 ; i<path_handle->connection_number ; i++) {
        if (path_audio_device[i] & (HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R)) {
            if (device_interface[i] == HAL_AUDIO_INTERFACE_1) {
                pAudPara->audio_device_rate[i] = (gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[0]>0)?gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[0]:pAudPara->rate;;
            } else if (device_interface[i] == HAL_AUDIO_INTERFACE_2) {
                pAudPara->audio_device_rate[i] = (gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[1]>0)?gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[1]:pAudPara->rate;;
            } else if (device_interface[i] == HAL_AUDIO_INTERFACE_3) {
                pAudPara->audio_device_rate[i] = (gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[2]>0)?gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[2]:pAudPara->rate;;
            } else if (device_interface[i] == HAL_AUDIO_INTERFACE_4) {
                pAudPara->audio_device_rate[i] = (gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[3]>0)?gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[3]:pAudPara->rate;;
            }
        }
        input_port_parameters.device_interface = device_interface[i];
        output_port_parameters.memory_select = memory_select[i];
        #ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
#ifdef AIR_ECHO_PATH_STEREO_ENABLE
            if (pAudPara->echo_reference && (i == path_handle->connection_number - 2)) {
#else
            if (pAudPara->echo_reference && (i == path_handle->connection_number - 1)) {
#endif
#ifdef AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE
                if (path_handle->scenario_type == AUDIO_SCENARIO_TYPE_ANC_MONITOR_STREAM) {
                    path_handle->input.interconn_sequence[i]  =  HAL_AUDIO_INTERCONN_SELECT_INPUT_DOWN_SAMPLER01_CH1;
                } else {
                    path_handle->input.interconn_sequence[i]  =  HAL_AUDIO_INTERCONN_SELECT_INPUT_DOWN_SAMPLER23_CH1;
                }
#else
                path_handle->input.interconn_sequence[i]  =  HAL_AUDIO_INTERCONN_SELECT_INPUT_DOWN_SAMPLER23_CH1;
                path_handle->output.interconn_sequence[i] =  HAL_AUDIO_INTERCONN_SELECT_OUTPUT_MEMORY_VUL1_CH1 + i;
#ifdef AIR_ECHO_PATH_STEREO_ENABLE
            } else if (pAudPara->echo_reference && (i == path_handle->connection_number - 1)) {
                path_handle->input.interconn_sequence[i]  =  HAL_AUDIO_INTERCONN_SELECT_INPUT_DOWN_SAMPLER23_CH2;
                path_handle->output.interconn_sequence[i] =  HAL_AUDIO_INTERCONN_SELECT_OUTPUT_MEMORY_VUL1_CH1 + i;
#endif
            } else
#endif

#endif
        {
            path_handle->input.interconn_sequence[i]  = stream_audio_convert_control_to_interconn(path_audio_device[i], input_port_parameters, i, true);
            path_handle->output.interconn_sequence[i] = stream_audio_convert_control_to_interconn(HAL_AUDIO_CONTROL_MEMORY_INTERFACE, output_port_parameters, i, true);
        }
        path_handle->with_updown_sampler[i] = false;
        path_handle->audio_input_rate[i]  = pAudPara->audio_device_rate[i];
        path_handle->audio_output_rate[i] = pAudPara->audio_memory_rate[i];
        const bool isEchoReferenceInput = ((path_handle->input.interconn_sequence[i] == HAL_AUDIO_INTERCONN_SELECT_INPUT_DOWN_SAMPLER23_CH1)
                                               || (path_handle->input.interconn_sequence[i] == HAL_AUDIO_INTERCONN_SELECT_INPUT_DOWN_SAMPLER23_CH2));

        if (!isEchoReferenceInput
            && path_handle->audio_input_rate[i] != 0 && path_handle->audio_output_rate[i] != 0
            && path_handle->audio_input_rate[i] != path_handle->audio_output_rate[i]) {

            path_handle->with_updown_sampler[i]   = true;
        }
    }

}

    path_handle->with_hw_gain = pAudPara->hw_gain ;

#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE) || defined (AIR_BT_AUDIO_DONGLE_ENABLE)
    //Update memory and path selection for Sub-source
    if ((source->type >= SOURCE_TYPE_SUBAUDIO_MIN) && (source->type <= SOURCE_TYPE_SUBAUDIO_MAX)) {
        bool memory_fined = false;
        mem_handle->memory_select = 0;

        //whether if use same memory interface
        SOURCE_TYPE     search_source_type;
        hal_audio_memory_selection_t memory_assign, memory_occupied = 0;


        for (i = 0; i < path_handle->connection_number ; i += 2) {
            memory_fined = false;
            if (path_handle->input.interconn_sequence[i] == (uint8_t)(HAL_AUDIO_INTERCONN_SEQUENCE_DUMMY & 0xFF)) {
                if (i == 0) {
                    path_handle->connection_number = 0;
                }
                break;
            }

            for (search_source_type = SOURCE_TYPE_SUBAUDIO_MIN ; search_source_type <= SOURCE_TYPE_SUBAUDIO_MAX ; search_source_type++) {
                if ((!Source_blks[search_source_type]) || (source->type == search_source_type)) {
                    continue;
                }
#if defined(AIR_MULTI_MIC_STREAM_ENABLE) && !defined(AIR_BTA_IC_PREMIUM_G3) || defined (AIR_BT_AUDIO_DONGLE_ENABLE)
                uint32_t interconn_sequence;
                for (interconn_sequence = 0 ; interconn_sequence < HAL_AUDIO_PATH_SUPPORT_SEQUENCE ; interconn_sequence += 2) {
                    if ((path_handle->input.interconn_sequence[i] == Source_blks[search_source_type]->param.audio.path_handle.input.interconn_sequence[interconn_sequence]) &&
                        (path_handle->input.interconn_sequence[i + 1] == Source_blks[search_source_type]->param.audio.path_handle.input.interconn_sequence[interconn_sequence + 1])) {
                        path_handle->output.interconn_sequence[i] = Source_blks[search_source_type]->param.audio.path_handle.output.interconn_sequence[interconn_sequence];
                        path_handle->output.interconn_sequence[i + 1] = Source_blks[search_source_type]->param.audio.path_handle.output.interconn_sequence[interconn_sequence + 1];
                        mem_handle->memory_select |= stream_audio_convert_interconn_to_memory(source->param.audio.path_handle.output.interconn_sequence[i]);
                        pAudPara->buffer_size = Source_blks[search_source_type]->param.audio.buffer_size;
                        DSP_MW_LOG_I("[Source Common] audio source buffer find exist memory agent 0x%x  %d\n", 2, mem_handle->memory_select, source->param.audio.path_handle.output.interconn_sequence[i]);
                        search_source_type = SOURCE_TYPE_SUBAUDIO_MAX;
                        memory_fined = true;
                        break;
                    }
                }
#endif
                memory_occupied |= Source_blks[search_source_type]->param.audio.mem_handle.memory_select;
            }

            if (!memory_fined) {
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
                if (pAudPara->memory == (HAL_AUDIO_MEM_SUB|HAL_AUDIO_MEM4)){
                    if (!(memory_occupied&HAL_AUDIO_MEMORY_UL_AWB)) {
                        memory_assign = HAL_AUDIO_MEMORY_UL_AWB;
                    } else {
                        assert(0);
                    }

                } else
#endif
                {
                if (!(mem_handle->memory_select & HAL_AUDIO_MEMORY_UL_VUL3) && !(memory_occupied & HAL_AUDIO_MEMORY_UL_VUL3)) {
                    memory_assign = HAL_AUDIO_MEMORY_UL_VUL3;

                } else if (!(mem_handle->memory_select & HAL_AUDIO_MEMORY_UL_AWB) && !(memory_occupied & HAL_AUDIO_MEMORY_UL_AWB)) {
                    memory_assign = HAL_AUDIO_MEMORY_UL_AWB;
#ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
                } else if (!(mem_handle->memory_select & HAL_AUDIO_MEMORY_UL_AWB2) && !(memory_occupied & HAL_AUDIO_MEMORY_UL_AWB2)) {
                    memory_assign = HAL_AUDIO_MEMORY_UL_AWB2;
#endif
                }else{
                    DSP_MW_LOG_E("[Source Common] DSP audio sub-source memory_agent is not enough \n", 0);
                }
                }
                mem_handle->memory_select |= memory_assign;
                path_handle->output.interconn_sequence[i] = stream_audio_convert_control_to_interconn(HAL_AUDIO_CONTROL_MEMORY_INTERFACE, (hal_audio_path_port_parameter_t)memory_assign, i, true);
                path_handle->output.interconn_sequence[i + 1] = stream_audio_convert_control_to_interconn(HAL_AUDIO_CONTROL_MEMORY_INTERFACE, (hal_audio_path_port_parameter_t)memory_assign, i + 1, true);
            }
            #if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
                if (pAudPara->memory != (HAL_AUDIO_MEM_SUB|HAL_AUDIO_MEM4))
            #endif
                {
                    hal_audio_set_value_parameter_t handle;
                    handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_AUDIOSYS;
                    handle.register_irq_handler.memory_select = memory_assign;
                    handle.register_irq_handler.entry = afe_subsource_interrupt_handler;
                    hal_audio_set_value((hal_audio_set_value_parameter_t *)&handle, HAL_AUDIO_SET_IRQ_HANDLER);
                }
        }
        DSP_MW_LOG_I("[Source Common] DSP audio sub-source:%d, memory_agent:0x%x \n", 2, source->type, mem_handle->memory_select);
        if (path_audio_device[i] & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)) {
            for (i=0 ; i<connection_number ; i++) {
                path_handle->input.interconn_sequence[i]  = stream_audio_convert_control_to_interconn(path_audio_device[i], input_port_parameters, i, true);
                output_port_parameters.memory_select = memory_assign;
                path_handle->output.interconn_sequence[i] = stream_audio_convert_control_to_interconn(HAL_AUDIO_CONTROL_MEMORY_INTERFACE, output_port_parameters, i, true);
            }
        }
    }
#endif

#ifndef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
    if (pAudPara->echo_reference) {
        mem_handle->memory_select = mem_handle->memory_select | HAL_AUDIO_MEMORY_UL_AWB2;
    }

    if (mem_handle->memory_select == HAL_AUDIO_MEMORY_UL_AWB2) {
        //Echo path Only
        DSP_MW_LOG_I("DSP audio source echo path Only", 0);
    }
#endif

    //for hal_audio_set_device
    if ((pAudPara->audio_device) & (HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL | HAL_AUDIO_CONTROL_DEVICE_LINE_IN_DUAL | HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL| HAL_AUDIO_CONTROL_DEVICE_ANC | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R | HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE
#if defined(AIR_BTA_IC_PREMIUM_G3)
        | HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_DUAL
#endif
        )
    ) {
        device_handle->common.rate = pAudPara->audio_device_rate[0];
        device_handle->common.device_interface = (hal_audio_interface_t)pAudPara->audio_interface;
        device_handle->common.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
        device_handle->common.is_tx = false;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        device_handle1->common.rate = pAudPara->audio_device_rate[1];
        device_handle1->common.device_interface = (hal_audio_interface_t)pAudPara->audio_interface1;
        device_handle1->common.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
        device_handle1->common.is_tx = false;
        device_handle2->common.rate = pAudPara->audio_device_rate[2];
        device_handle2->common.device_interface = (hal_audio_interface_t)pAudPara->audio_interface2;
        device_handle2->common.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
        device_handle2->common.is_tx = false;
        device_handle3->common.rate = pAudPara->audio_device_rate[3];
        device_handle3->common.device_interface = (hal_audio_interface_t)pAudPara->audio_interface3;
        device_handle3->common.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
        device_handle3->common.is_tx = false;
        device_handle4->common.rate = pAudPara->audio_device_rate[4];
        device_handle4->common.device_interface = (hal_audio_interface_t)pAudPara->audio_interface4;
        device_handle4->common.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
        device_handle4->common.is_tx = false;
        device_handle5->common.rate = pAudPara->audio_device_rate[5];
        device_handle5->common.device_interface = (hal_audio_interface_t)pAudPara->audio_interface5;
        device_handle5->common.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
        device_handle5->common.is_tx = false;
        device_handle6->common.rate = pAudPara->audio_device_rate[6];
        device_handle6->common.device_interface = (hal_audio_interface_t)pAudPara->audio_interface6;
        device_handle6->common.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
        device_handle6->common.is_tx = false;
        device_handle7->common.rate = pAudPara->audio_device_rate[7];
        device_handle7->common.device_interface = (hal_audio_interface_t)pAudPara->audio_interface7;
        device_handle7->common.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
        device_handle7->common.is_tx = false;
#endif
        DSP_MW_LOG_I("[Source Common] set device common.rate %d,source rate %d", 2, device_handle->common.rate, pAudPara->rate);
    }

    if (pAudPara->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        device_handle->i2s_slave.memory_select = mem_handle->memory_select;
        #ifdef AIR_HWSRC_RX_TRACKING_ENABLE
        mem_handle->src_rate = gAudioCtrl.Afe.AfeULSetting.src_rate;
        mem_handle->src_buffer_length = pAudPara->buffer_size;
        mem_handle->buffer_length = AUDIO_AFE_SOURCE_ASRC_BUFFER_SIZE;
        if(pAudPara->clk_skew_mode == CLK_SKEW_DISSABLE) {
            if(pAudPara->audio_interface == HAL_AUDIO_INTERFACE_1)
                mem_handle->src_tracking_clock_source =  HAL_AUDIO_SRC_TRACKING_I2S1;
            else if(pAudPara->audio_interface == HAL_AUDIO_INTERFACE_2)
                mem_handle->src_tracking_clock_source =  HAL_AUDIO_SRC_TRACKING_I2S2;
            else if(pAudPara->audio_interface == HAL_AUDIO_INTERFACE_3)
                mem_handle->src_tracking_clock_source =  HAL_AUDIO_SRC_TRACKING_I2S3;
            else if(pAudPara->audio_interface == HAL_AUDIO_INTERFACE_4)
                mem_handle->src_tracking_clock_source =  HAL_AUDIO_SRC_TRACKING_I2S4;
            DSP_MW_LOG_I("[HWSRC]rx tracking clock_source =  %d\r\n",1,mem_handle->src_tracking_clock_source);
        }
        #endif
    }

    device_handle->common.audio_device = pAudPara->audio_device;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    device_handle1->common.audio_device = pAudPara->audio_device1;
    device_handle2->common.audio_device = pAudPara->audio_device2;
    device_handle3->common.audio_device = pAudPara->audio_device3;
    device_handle4->common.audio_device = pAudPara->audio_device4;
    device_handle5->common.audio_device = pAudPara->audio_device5;
    device_handle6->common.audio_device = pAudPara->audio_device6;
    device_handle7->common.audio_device = pAudPara->audio_device7;
#endif
    Source_device_set_para(device_handle);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    Source_device_set_para(device_handle1);
    Source_device_set_para(device_handle2);
    Source_device_set_para(device_handle3);
    Source_device_set_para(device_handle4);
    Source_device_set_para(device_handle5);
    Source_device_set_para(device_handle6);
    Source_device_set_para(device_handle7);
#endif


}

void Source_device_set_para(hal_audio_device_parameter_t *device_handle)
{
    uint32_t i = 0, index;
    index = hal_audio_convert_interfac_to_index(device_handle->common.device_interface);
    if ((device_handle->common.audio_device)&HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL) {
        device_handle->analog_mic.rate = device_handle->common.rate;//AUDIO_SOURCE_DEFAULT_ANALOG_VOICE_RATE;
        device_handle->analog_mic.mic_interface = device_handle->common.device_interface;//HAL_AUDIO_INTERFACE_1;
        device_handle->analog_mic.scenario_type = device_handle->common.scenario_type;
        device_handle->analog_mic.is_tx = device_handle->common.is_tx;
        device_handle->analog_mic.bias_select = gAudioCtrl.Afe.AfeULSetting.bias_select;
        for (i = 0; i < HAL_AUDIO_MIC_BIAS_BLOCK_NUMBER; i++) {
            device_handle->analog_mic.bias_voltage[i] = gAudioCtrl.Afe.AfeULSetting.bias_voltage[i];
                }

        if (index <= 3) {
            device_handle->analog_mic.iir_filter = gAudioCtrl.Afe.AfeULSetting.iir_filter[index];
            if (device_handle->common.audio_device == HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_R) {
                device_handle->analog_mic.adc_parameter.adc_mode = gAudioCtrl.Afe.AfeULSetting.ul_adc_mode[index*2 + 1];
                device_handle->analog_mic.adc_parameter.adc_type = gAudioCtrl.Afe.AfeULSetting.amic_type[index*2 + 1];
            } else {
                device_handle->analog_mic.adc_parameter.adc_mode = gAudioCtrl.Afe.AfeULSetting.ul_adc_mode[index*2];
                device_handle->analog_mic.adc_parameter.adc_type = gAudioCtrl.Afe.AfeULSetting.amic_type[index*2];
        }
        switch (device_handle->common.device_interface) {
            case HAL_AUDIO_INTERFACE_1:
                device_handle->analog_mic.iir_filter = gAudioCtrl.Afe.AfeULSetting.iir_filter[0];
                break;
            case HAL_AUDIO_INTERFACE_2:
                device_handle->analog_mic.iir_filter = gAudioCtrl.Afe.AfeULSetting.iir_filter[1];
                break;
            case HAL_AUDIO_INTERFACE_3:
                device_handle->analog_mic.iir_filter = gAudioCtrl.Afe.AfeULSetting.iir_filter[2];
                break;
            default:
                device_handle->analog_mic.iir_filter = HAL_AUDIO_UL_IIR_50HZ_AT_48KHZ;//HAL_AUDIO_UL_IIR_5HZ_AT_48KHZ;
        }

        device_handle->analog_mic.with_external_bias = gAudioCtrl.Afe.AfeULSetting.with_external_bias;
        device_handle->analog_mic.bias1_2_with_LDO0 = gAudioCtrl.Afe.AfeULSetting.bias1_2_with_LDO0;
        device_handle->analog_mic.with_bias_lowpower = gAudioCtrl.Afe.AfeULSetting.with_bias_lowpower;
        device_handle->analog_mic.adc_parameter.performance = gAudioCtrl.Afe.AfeULSetting.performance;
            // DSP_MW_LOG_I("[Source Common] Source external_bias:%d bias1_2_with_LDO0:%d bias_low_power:%d performance:%d adc_mode:%d", 6,
            //     device_handle->analog_mic.with_external_bias,
            //     device_handle->analog_mic.bias1_2_with_LDO0,
            //     device_handle->analog_mic.with_bias_lowpower,
            //     device_handle->analog_mic.adc_parameter.performance,
            //     device_handle->analog_mic.adc_parameter.adc_mode
            //     );
        }
    } else if ((device_handle->common.audio_device)&HAL_AUDIO_CONTROL_DEVICE_LINE_IN_DUAL) {
        device_handle->linein.rate =  device_handle->common.rate;//AUDIO_SOURCE_DEFAULT_ANALOG_AUDIO_RATE;
        device_handle->linein.scenario_type = device_handle->common.scenario_type;
        device_handle->linein.is_tx = device_handle->common.is_tx;
        device_handle->linein.bias_select = gAudioCtrl.Afe.AfeULSetting.bias_select;
        for (i = 0; i < HAL_AUDIO_MIC_BIAS_BLOCK_NUMBER; i++) {
            device_handle->linein.bias_voltage[i] = gAudioCtrl.Afe.AfeULSetting.bias_voltage[i];
                }
        if (index <= 3) {
            device_handle->linein.iir_filter = gAudioCtrl.Afe.AfeULSetting.iir_filter[index];
            if (device_handle->common.audio_device == HAL_AUDIO_CONTROL_DEVICE_LINE_IN_R) {
                device_handle->linein.adc_parameter.adc_mode = gAudioCtrl.Afe.AfeULSetting.ul_adc_mode[index*2+1];
            } else {
                device_handle->linein.adc_parameter.adc_mode = gAudioCtrl.Afe.AfeULSetting.ul_adc_mode[index*2];
            }
            device_handle->linein.adc_parameter.adc_type = 1;
        } else {
            device_handle->linein.iir_filter = HAL_AUDIO_UL_IIR_50HZ_AT_48KHZ;//HAL_AUDIO_UL_IIR_5HZ_AT_48KHZ;
                device_handle->linein.adc_parameter.adc_mode = HAL_AUDIO_ANALOG_INPUT_ACC10K;
        }
        device_handle->linein.adc_parameter.performance = gAudioCtrl.Afe.AfeULSetting.performance;

    } else if ((device_handle->common.audio_device)&HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL) {
        device_handle->digital_mic.rate = device_handle->common.rate;//AUDIO_SOURCE_DEFAULT_ANALOG_VOICE_RATE;
        device_handle->digital_mic.mic_interface = device_handle->common.device_interface;//HAL_AUDIO_INTERFACE_1;
        device_handle->digital_mic.scenario_type = device_handle->common.scenario_type;
        device_handle->digital_mic.is_tx = device_handle->common.is_tx;
        switch (device_handle->common.audio_device) {
            case HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL:
            case HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_L:
                if (device_handle->common.device_interface == HAL_AUDIO_INTERFACE_1) {
                    device_handle->digital_mic.dmic_selection = gAudioCtrl.Afe.AfeULSetting.dmic_selection[0];
                } else if (device_handle->common.device_interface == HAL_AUDIO_INTERFACE_2) {
                    device_handle->digital_mic.dmic_selection = gAudioCtrl.Afe.AfeULSetting.dmic_selection[2];
                } else if (device_handle->common.device_interface == HAL_AUDIO_INTERFACE_3) {
                    device_handle->digital_mic.dmic_selection = gAudioCtrl.Afe.AfeULSetting.dmic_selection[4];
                } else if (device_handle->common.device_interface == HAL_AUDIO_INTERFACE_4) {
                    device_handle->digital_mic.dmic_selection = gAudioCtrl.Afe.AfeULSetting.dmic_selection[6];
                }
                break;
            case HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_R:
                if (device_handle->common.device_interface == HAL_AUDIO_INTERFACE_1) {
                    device_handle->digital_mic.dmic_selection = gAudioCtrl.Afe.AfeULSetting.dmic_selection[1];
                } else if (device_handle->common.device_interface == HAL_AUDIO_INTERFACE_2) {
                    device_handle->digital_mic.dmic_selection = gAudioCtrl.Afe.AfeULSetting.dmic_selection[3];
                } else if (device_handle->common.device_interface == HAL_AUDIO_INTERFACE_3) {
                    device_handle->digital_mic.dmic_selection = gAudioCtrl.Afe.AfeULSetting.dmic_selection[5];
                } else if (device_handle->common.device_interface == HAL_AUDIO_INTERFACE_4) {
                    device_handle->digital_mic.dmic_selection = gAudioCtrl.Afe.AfeULSetting.dmic_selection[7];
                }
                break;
            default:
                device_handle->digital_mic.dmic_selection = HAL_AUDIO_DMIC_GPIO_DMIC0;
                break;
        }
        DSP_MW_LOG_I("dmic_selection %d", 1, device_handle->digital_mic.dmic_selection);
        device_handle->digital_mic.bias_select = gAudioCtrl.Afe.AfeULSetting.bias_select;
        for (i = 0; i < HAL_AUDIO_MIC_BIAS_BLOCK_NUMBER; i++) {
            device_handle->digital_mic.bias_voltage[i] = gAudioCtrl.Afe.AfeULSetting.bias_voltage[i];
            // DSP_MW_LOG_I("[Source Common] digital mic bias voltage[%d] = %d", 2, i, device_handle->digital_mic.bias_voltage[i]);
        }
        if (index <= 2){
            device_handle->digital_mic.iir_filter = gAudioCtrl.Afe.AfeULSetting.iir_filter[index];
            device_handle->digital_mic.dmic_clock_rate = gAudioCtrl.Afe.AfeULSetting.dmic_clock_rate[index];
            if (device_handle->common.audio_device == HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_R) {
                device_handle->digital_mic.dmic_selection = gAudioCtrl.Afe.AfeULSetting.dmic_selection[index*2+1];
            } else {
                device_handle->digital_mic.dmic_selection = gAudioCtrl.Afe.AfeULSetting.dmic_selection[index*2];
            }
        } else {
                device_handle->digital_mic.iir_filter = HAL_AUDIO_UL_IIR_DISABLE;
                device_handle->digital_mic.dmic_clock_rate = 0;/**AFE_DMIC_CLOCK_3_25M*/
            device_handle->digital_mic.dmic_selection = HAL_AUDIO_DMIC_GPIO_DMIC0;
        }

        DSP_MW_LOG_I("[Source Common] dmic_selection %d", 1, device_handle->digital_mic.dmic_selection);

        device_handle->digital_mic.with_external_bias = gAudioCtrl.Afe.AfeULSetting.with_external_bias;
        device_handle->digital_mic.with_bias_lowpower = gAudioCtrl.Afe.AfeULSetting.with_bias_lowpower;
        device_handle->digital_mic.bias1_2_with_LDO0 = gAudioCtrl.Afe.AfeULSetting.bias1_2_with_LDO0;
    } else if ((device_handle->common.audio_device)&HAL_AUDIO_CONTROL_DEVICE_VAD) {
        device_handle->vad.rate = AUDIO_SOURCE_DEFAULT_ANALOG_VOICE_RATE;
        device_handle->vad.scenario_type = device_handle->common.scenario_type;
        device_handle->vad.is_tx = device_handle->common.is_tx;
    } else if ((device_handle->common.audio_device) & (HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R)) {
        device_handle->i2s_master.i2s_interface = device_handle->common.device_interface;//HAL_AUDIO_INTERFACE_1;
        device_handle->i2s_master.scenario_type = device_handle->common.scenario_type;
        device_handle->i2s_master.is_tx = device_handle->common.is_tx;
        if (device_handle->i2s_master.i2s_interface & HAL_AUDIO_INTERFACE_1) {
            device_handle->i2s_master.rate = (gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[0]>0)?gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[0]:device_handle->common.rate;
            device_handle->i2s_master.i2s_format = gAudioCtrl.Afe.AfeULSetting.i2s_master_format[0];
            device_handle->i2s_master.word_length = gAudioCtrl.Afe.AfeULSetting.i2s_master_word_length[0];
            device_handle->i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeULSetting.is_low_jitter[0];
        } else if (device_handle->i2s_master.i2s_interface & HAL_AUDIO_INTERFACE_2) {
            device_handle->i2s_master.rate = (gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[1]>0)?gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[1]:device_handle->common.rate;
            device_handle->i2s_master.i2s_format = gAudioCtrl.Afe.AfeULSetting.i2s_master_format[1];
            device_handle->i2s_master.word_length = gAudioCtrl.Afe.AfeULSetting.i2s_master_word_length[1];
            device_handle->i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeULSetting.is_low_jitter[1];
        } else if (device_handle->i2s_master.i2s_interface & HAL_AUDIO_INTERFACE_3) {
            device_handle->i2s_master.rate = (gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[2]>0)?gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[2]:device_handle->common.rate;
            device_handle->i2s_master.i2s_format = gAudioCtrl.Afe.AfeULSetting.i2s_master_format[2];
            device_handle->i2s_master.word_length = gAudioCtrl.Afe.AfeULSetting.i2s_master_word_length[2];
            device_handle->i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeULSetting.is_low_jitter[2];
        } else if (device_handle->i2s_master.i2s_interface & HAL_AUDIO_INTERFACE_4) {
            device_handle->i2s_master.rate = (gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[3]>0)?gAudioCtrl.Afe.AfeULSetting.i2s_master_sampling_rate[3]:device_handle->common.rate;
            device_handle->i2s_master.i2s_format = gAudioCtrl.Afe.AfeULSetting.i2s_master_format[3];
            device_handle->i2s_master.word_length = gAudioCtrl.Afe.AfeULSetting.i2s_master_word_length[3];
            device_handle->i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeULSetting.is_low_jitter[3];
        }
        if (device_handle->i2s_master.rate > 48000) {
            device_handle->i2s_master.is_low_jitter = true;
        }
        // DSP_MW_LOG_I("i2s master configuration: interface=%d, sampling_rate=%d,format=%d,word_length=%d,is_low_jitter=%d", 5,
        //              device_handle->i2s_master.i2s_interface,
        //              device_handle->i2s_master.rate,
        //              device_handle->i2s_master.i2s_format,
        //              device_handle->i2s_master.word_length,
        //              device_handle->i2s_master.is_low_jitter
        // );
        device_handle->i2s_master.mclk_divider = 2;
        device_handle->i2s_master.with_mclk = false;
        device_handle->i2s_master.is_recombinant = false;
    } else if ((device_handle->common.audio_device)&HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        device_handle->i2s_slave.rate = device_handle->common.rate;//48000;
        device_handle->i2s_slave.scenario_type = device_handle->common.scenario_type;
        device_handle->i2s_slave.is_tx = device_handle->common.is_tx;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        device_handle->i2s_slave.i2s_interface = gAudioCtrl.Afe.AfeULSetting.audio_interface;//HAL_AUDIO_INTERFACE_1;
#else
        device_handle->i2s_slave.i2s_interface = device_handle->common.device_interface;//HAL_AUDIO_INTERFACE_1;
#endif
        device_handle->i2s_slave.i2s_format = gAudioCtrl.Afe.AfeULSetting.i2s_format;
        device_handle->i2s_slave.word_length = gAudioCtrl.Afe.AfeULSetting.i2s_word_length;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        device_handle->i2s_slave.tdm_channel = gAudioCtrl.Afe.AfeULSetting.tdm_channel;
#endif
        if ((device_handle->i2s_slave.memory_select == HAL_AUDIO_MEMORY_UL_SLAVE_DMA) || (device_handle->i2s_slave.memory_select == HAL_AUDIO_MEMORY_UL_SLAVE_TDM)) {
            device_handle->i2s_slave.is_vdma_mode = true;
        } else {
#ifdef AIR_I2S_SLAVE_ENABLE
            device_handle->i2s_slave.is_vdma_mode = true;
#else
            device_handle->i2s_slave.is_vdma_mode = false;
#endif
        }
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        if (device_handle->i2s_slave.memory_select == HAL_AUDIO_MEMORY_UL_SLAVE_TDM) {
            if (device_handle->i2s_slave.i2s_interface == HAL_AUDIO_INTERFACE_1) {
                AUDIO_ASSERT(0 && "[SLAVE TDM] UL I2S Slave0 not support TDM mode");
            }
            if ((device_handle->i2s_slave.tdm_channel != HAL_AUDIO_I2S_TDM_4CH) && (device_handle->i2s_slave.tdm_channel != HAL_AUDIO_I2S_TDM_6CH) && (device_handle->i2s_slave.tdm_channel != HAL_AUDIO_I2S_TDM_8CH)) {
                DSP_MW_LOG_I("[SLAVE TDM] UL I2S Slave tdm channel : %d invalid and assert", 1, device_handle->i2s_slave.tdm_channel);
                AUDIO_ASSERT(0 && "[SLAVE TDM] UL I2S Slave tdm channel invalid");
            }
        }
#endif
    }
}

VOID Sink_Audio_HW_Init_AFE(SINK sink)
{
    // do .hw_params()
    // according to sink to init the choosen AFE IO block
    // 1) hw_type
    // 2) channel
    // 3) mem allocate

    // TODO: AFE Clock init here <----
    int32_t ret = 0;
    ret = audio_ops_probe(sink);
    if (ret) {
        ret = 1;
        goto ERR_HANDLER;
    }
    ret = audio_ops_hw_params(sink);
    if (ret) {
        ret = 2;
        goto ERR_HANDLER;
    }
    switch (sink->param.audio.HW_type) {
        case  AUDIO_HARDWARE_PCM :
        case AUDIO_HARDWARE_I2S_M ://I2S master
        case AUDIO_HARDWARE_I2S_S ://I2S slave
            return;
            break;
        default:
            AUDIO_ASSERT(0 && "sink->param.audio.HW_type error");
            break;
    }
ERR_HANDLER:
    DSP_MW_LOG_E("audio sink type:%d scenario_type:%d error:%d", 3, sink->type, sink->scenario_type, ret);
}

VOID Sink_Audio_HW_Init(SINK sink)
{
    UNUSED(sink);
    return;
}


VOID Source_Audio_HW_Init(SOURCE source)
{
    // TODO: AFE Clock init here <----
    int32_t ret = 0;
    ret = audio_ops_probe(source);
    if (ret) {
        ret = 1;
        goto ERR_HANDLER;
    }
    ret = audio_ops_hw_params(source);
    if (ret) {
        ret = 2;
        goto ERR_HANDLER;
    }
ERR_HANDLER:
    DSP_MW_LOG_E("audio source type:%d scenario_type:%d error:%d", 3, source->type, source->scenario_type, ret);
}


VOID AudioSourceHW_Ctrl(SOURCE source, BOOL IsEnabled)
{
    UNUSED(source);
    UNUSED(IsEnabled);
}


VOID AudioSinkHW_Ctrl(SINK sink, BOOL IsEnabled)
{
    UNUSED(sink);
    UNUSED(IsEnabled);
}

