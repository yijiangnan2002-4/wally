/* Copyright Statement:
 *
 * (C) 2014  Airoha Technology Corp. All rights reserved.
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

#include "types.h"
#include "dsp_memory.h"
#include "stream_audio.h"
#include "source_inter.h"
#include "sink_inter.h"
#include "transform.h"
#include "stream_n9sco.h"
#include "dsp_audio_msg.h"
#include "voice_plc_interface.h"
#include "audio_config.h"
#include "dsp_audio_process.h"
#include "dsp_memory.h"
#include "dsp_share_memory.h"
#include "dsp_temp.h"
#include "dsp_buffer.h"
#include "timers.h"
#include "dsp_dump.h"
#include "audio_nvdm_common.h"
#include "bt_interface.h"
#include "hal_resource_assignment.h"
#include "hal_audio_driver.h"
#ifdef AIR_DCHS_MODE_ENABLE
#include "stream_dchs.h"
#include "hal_gpt.h"
#include "stream_dchs.h"
#ifdef AIR_CPU_IN_SECURITY_MODE
#define CONN_BT_TIMCON_BASE 0xA0000000
#else
#define CONN_BT_TIMCON_BASE 0xB0000000
#endif
#endif//AIR_DCHS_MODE_ENABLE
#define DSP_FORWARD_BUFFER_SIZE          120
#define DSP_SCO_INBAND_INFORMATION       20
#ifdef AIR_HFP_SYNC_STOP_ENABLE
#define ESCO_SYNC_STOP_DELAY_ACK_TIME    2   /* Unit of 15ms */
#endif
#define AIR_ESCO_DL_FULL_DEBUG_ENABLE    0   /* for debug esco dl */
#define ESCO_INVALID_VALUE               0xFFFFFFFF

Stream_n9sco_Config_Ptr N9SCO_setting;
uint16_t escoseqn;
bool g_ignore_next_drop_flag = false; // avoid double drop in dav task

static int32_t g_sco_pkt_type;
static int32_t g_sco_frame_per_pkt_num;
static int32_t g_sco_frame_id;
static int32_t g_rx_fwd_irq_time;
static int32_t g_tx_fwd_irq_time;
static int32_t g_pattern_framesize;
static uint32_t g_prev_tx_fwd_buf_idx;
static uint32_t g_prev_rx_fwd_buf_idx;
static uint32_t g_esco_dl_afe_previous_writeoffset;
static uint32_t g_esco_ul_afe_previous_readoffset;
static uint32_t g_esco_ul_avm_previous_writeoffset;
static bool g_esco_stream_ready_check;
static bool g_esco_dl_ul_process_active;
#ifdef AIR_HFP_SYNC_STOP_ENABLE
static int32_t g_esco_dl_sync_stop_detect;
#endif
static volatile bool g_debug_forwarder_assert_flag;
static volatile uint32_t g_rx_fwd_running_count;
static volatile uint32_t g_tx_fwd_running_count;

extern bool ClkSkewMode_g;

//#define DEBUG_ESCO_TX_PKG_CONTENT_VERIFY

#ifdef DEBUG_ESCO_TX_PKG_CONTENT_VERIFY
#define DEBUG_TEMP_FWD_BUFFER_NUMBER    6
#define DEBUG_TEMP_FWD_FRAME_SIZE       120
static uint32_t g_debug_tx_temp_forwarder_index;
static uint32_t g_debug_tx_temp_count;
static uint8_t g_debug_tx_temp_forwarder_buffer[DEBUG_TEMP_FWD_FRAME_SIZE * DEBUG_TEMP_FWD_BUFFER_NUMBER];
#endif

#ifdef AIR_HFP_SYNC_STOP_ENABLE
extern void dsp_sync_callback_hfp(cm4_dsp_audio_sync_action_type_t request_action_id, void *user_data);
#endif
extern bool afe_audio_device_ready(SOURCE_TYPE source_type, SINK_TYPE sink_type);
extern void StreamDSP_HWSemaphoreTake(void);
extern void StreamDSP_HWSemaphoreGive(void);
extern hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);
extern hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask);

#if defined(AIR_FIXED_RATIO_SRC)
#include "src_fixed_ratio_interface.h"

#if defined(AIR_UL_FIX_SAMPLING_RATE_48K) || defined(AIR_UL_FIX_SAMPLING_RATE_32K)
static src_fixed_ratio_port_t *g_esco_ul_src_fixed_ratio_port;
#endif
static src_fixed_ratio_port_t *g_esco_ul_src_fixed_ratio_2nd_port;
static src_fixed_ratio_port_t *g_esco_dl_src_fixed_ratio_port;

static uint32_t internal_fs_converter(stream_samplerate_t fs)
{
    switch (fs) {
        case FS_RATE_44_1K:
            return 44100;

        case FS_RATE_8K:
        case FS_RATE_16K:
        case FS_RATE_24K:
        case FS_RATE_32K:
        case FS_RATE_48K:
            return fs * 1000;

        default:
            AUDIO_ASSERT(false && "[DSP][eSCO] sample rate is not supported!");
            return fs;
    }
}

void Sco_UL_Fix_Sample_Rate_Init(void)
{
    uint32_t channel_number;
    DSP_STREAMING_PARA_PTR ul_stream;
    src_fixed_ratio_config_t smp_config = {0};
    volatile SINK sink = Sink_blks[SINK_TYPE_N9SCO];

    ul_stream = DSP_Streaming_Get(sink->transform->source, sink);
    channel_number = stream_function_get_channel_number(&(ul_stream->callback.EntryPara));

#if defined(AIR_UL_FIX_SAMPLING_RATE_48K) || defined(AIR_UL_FIX_SAMPLING_RATE_32K)
    /* Used for 48K fix input */
    smp_config.channel_number = channel_number;
    smp_config.in_sampling_rate = internal_fs_converter((stream_samplerate_t)(ul_stream->callback.EntryPara.in_sampling_rate));
    smp_config.out_sampling_rate = internal_fs_converter((stream_samplerate_t)(ul_stream->callback.EntryPara.codec_out_sampling_rate));
    smp_config.resolution = ul_stream->callback.EntryPara.resolution.feature_res;
    smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_ALTERNATE;
    smp_config.cvt_num = (gDspAlgParameter.EscoMode.Tx == VOICE_NB) ? 2 : 1;
    smp_config.with_codec = true;
    g_esco_ul_src_fixed_ratio_port = stream_function_src_fixed_ratio_get_port(sink);
    stream_function_src_fixed_ratio_init(g_esco_ul_src_fixed_ratio_port, &smp_config);

    /* Used in the last stage for 16K -> 8K */
    if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
        smp_config.channel_number = 1;
        smp_config.in_sampling_rate = internal_fs_converter(FS_RATE_16K);
        smp_config.out_sampling_rate = internal_fs_converter(FS_RATE_8K);
        smp_config.resolution = ul_stream->callback.EntryPara.resolution.feature_res;
        smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_ALTERNATE;
        smp_config.cvt_num = 2;
        g_esco_ul_src_fixed_ratio_2nd_port = stream_function_src_fixed_ratio_get_2nd_port(sink);
        stream_function_src_fixed_ratio_init(g_esco_ul_src_fixed_ratio_2nd_port, &smp_config);
    }
#else
    /* Used in the last stage for 16K -> 8K */
    if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
        smp_config.channel_number = 1;
        smp_config.in_sampling_rate = internal_fs_converter(FS_RATE_16K);
        smp_config.out_sampling_rate = internal_fs_converter(FS_RATE_8K);
        smp_config.resolution = ul_stream->callback.EntryPara.resolution.feature_res;
        smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_SINGLE;
        smp_config.cvt_num = 1;
        smp_config.with_codec = true;
        g_esco_ul_src_fixed_ratio_2nd_port = stream_function_src_fixed_ratio_get_port(sink);
        stream_function_src_fixed_ratio_init(g_esco_ul_src_fixed_ratio_2nd_port, &smp_config);
    }
#endif
}

void Sco_UL_Fix_Sample_Rate_Deinit(void)
{
#if defined(AIR_UL_FIX_SAMPLING_RATE_48K) || defined(AIR_UL_FIX_SAMPLING_RATE_32K)
    if (g_esco_ul_src_fixed_ratio_port) {
        stream_function_src_fixed_ratio_deinit(g_esco_ul_src_fixed_ratio_port);
        g_esco_ul_src_fixed_ratio_port = NULL;
    }
#endif
    if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
        if (g_esco_ul_src_fixed_ratio_2nd_port) {
            stream_function_src_fixed_ratio_deinit(g_esco_ul_src_fixed_ratio_2nd_port);
            g_esco_ul_src_fixed_ratio_2nd_port = NULL;
        }
    }
}

void Sco_DL_Fix_Sample_Rate_Init(void)
{
    DSP_STREAMING_PARA_PTR dl_stream;
    src_fixed_ratio_config_t smp_config = {0};
    volatile SOURCE source = Source_blks[SOURCE_TYPE_N9SCO];

    if (gDspAlgParameter.EscoMode.Rx == VOICE_NB) {
        /* Used in the 1st stage for 8K -> 16K */
        dl_stream = DSP_Streaming_Get(source, source->transform->sink);
        dl_stream->callback.EntryPara.in_sampling_rate = FS_RATE_8K;
        smp_config.channel_number = 1;
        smp_config.in_sampling_rate = internal_fs_converter((stream_samplerate_t)(dl_stream->callback.EntryPara.in_sampling_rate));
        smp_config.out_sampling_rate = internal_fs_converter(FS_RATE_16K);
        smp_config.resolution = dl_stream->callback.EntryPara.resolution.feature_res;
        smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_SINGLE;
        smp_config.cvt_num = 1;
        smp_config.with_codec = true;
        g_esco_dl_src_fixed_ratio_port = stream_function_src_fixed_ratio_get_port(source);
        stream_function_src_fixed_ratio_init(g_esco_dl_src_fixed_ratio_port, &smp_config);
    }
}

void Sco_DL_Fix_Sample_Rate_Deinit(void)
{
    if (gDspAlgParameter.EscoMode.Rx == VOICE_NB) {
        if (g_esco_dl_src_fixed_ratio_port) {
            stream_function_src_fixed_ratio_deinit(g_esco_dl_src_fixed_ratio_port);
            g_esco_dl_src_fixed_ratio_port = NULL;
        }
    }
}
#endif

static uint32_t Sco_Get_RX_FWD_Pkt_Type(sco_pkt_type *type)
{
    uint32_t pkt_num;
    uint16_t RxDataLen = Forwarder_Get_RX_FWD_Pattern_Size(gDspAlgParameter.EscoMode.Rx);

    if (gDspAlgParameter.EscoMode.Rx == VOICE_NB) {
        RxDataLen = RxDataLen / 2;
    }

    switch (RxDataLen) {
        case SCO_PKT_2EV3_LEN:
            *type = SCO_PKT_2EV3;
            pkt_num = 1;
            break;
        case SCO_PKT_EV3_LEN:
            *type = SCO_PKT_EV3;
            pkt_num = 2;
            break;
        case SCO_PKT_HV2_LEN:
            *type = SCO_PKT_HV2;
            pkt_num = 3;
            break;
        case SCO_PKT_HV1_LEN:
            *type = SCO_PKT_HV1;
            pkt_num = 6;
            break;
        default:
            *type = SCO_PKT_2EV3;
            pkt_num = 1;
    }

    return pkt_num;
}

ATTR_TEXT_IN_IRAM static void Sco_Rx_update_from_share_information(SOURCE source)
{
    StreamDSP_HWSemaphoreTake();
    memcpy(&(source->streamBuffer.AVMBufferInfo), source->param.n9sco.share_info_base_addr, 40);/* share info fix 40 byte */
    source->streamBuffer.AVMBufferInfo.StartAddr = hal_memview_cm4_to_dsp0(source->streamBuffer.AVMBufferInfo.StartAddr);
    source->streamBuffer.AVMBufferInfo.ForwarderAddr = hal_memview_cm4_to_dsp0(source->streamBuffer.AVMBufferInfo.ForwarderAddr);
    StreamDSP_HWSemaphoreGive();
}

ATTR_TEXT_IN_IRAM_LEVEL_2 static void Sco_Rx_update_writeoffset_share_information(SOURCE source, uint32_t WriteOffset)
{
    StreamDSP_HWSemaphoreTake();
    source->param.n9sco.share_info_base_addr->WriteIndex = (uint16_t)WriteOffset;
    StreamDSP_HWSemaphoreGive();
}

ATTR_TEXT_IN_IRAM_LEVEL_2 static void Sco_Rx_update_readoffset_share_information(SOURCE source, uint32_t ReadOffset)
{
    StreamDSP_HWSemaphoreTake();
    source->param.n9sco.share_info_base_addr->ReadIndex = (uint16_t)ReadOffset;
#ifdef PT_bBufferIsFull_ready
    source->param.n9sco.share_info_base_addr->bBufferIsFull = false;
#endif
    StreamDSP_HWSemaphoreGive();
}

ATTR_TEXT_IN_IRAM static void Sco_Tx_update_from_share_information(SINK sink)
{
    StreamDSP_HWSemaphoreTake();
    memcpy(&(sink->streamBuffer.AVMBufferInfo), sink->param.n9sco.share_info_base_addr, 40);/* share info fix 40 byte */
    sink->streamBuffer.AVMBufferInfo.StartAddr = hal_memview_cm4_to_dsp0(sink->streamBuffer.AVMBufferInfo.StartAddr);
    //sink->streamBuffer.AVMBufferInfo.ForwarderAddr = hal_memview_cm4_to_dsp0(sink->streamBuffer.AVMBufferInfo.ForwarderAddr);
    StreamDSP_HWSemaphoreGive();
}

ATTR_TEXT_IN_IRAM static void Sco_Tx_update_from_share_information_forwarder(SINK sink)
{
    StreamDSP_HWSemaphoreTake();
    memcpy(&(sink->streamBuffer.AVMBufferInfo), sink->param.n9sco.share_info_base_addr, 40);/* share info fix 40 byte */
    sink->streamBuffer.AVMBufferInfo.StartAddr = hal_memview_cm4_to_dsp0(sink->streamBuffer.AVMBufferInfo.StartAddr);
    sink->streamBuffer.AVMBufferInfo.ForwarderAddr = hal_memview_cm4_to_dsp0(sink->streamBuffer.AVMBufferInfo.ForwarderAddr);
    StreamDSP_HWSemaphoreGive();
}

ATTR_TEXT_IN_IRAM_LEVEL_2 static void Sco_Tx_update_readoffset_share_information(SINK sink, uint32_t ReadOffset)
{
    StreamDSP_HWSemaphoreTake();
    sink->param.n9sco.share_info_base_addr->ReadIndex = (uint16_t)ReadOffset;
    StreamDSP_HWSemaphoreGive();
}

ATTR_TEXT_IN_IRAM_LEVEL_2 static void Sco_Tx_update_writeoffset_share_information(SINK sink, uint32_t WriteOffset)
{
    StreamDSP_HWSemaphoreTake();
    sink->param.n9sco.share_info_base_addr->WriteIndex = (uint16_t)WriteOffset;
    StreamDSP_HWSemaphoreGive();
}

static void Sco_Reset_Sinkoffset_share_information(SINK sink)
{
    StreamDSP_HWSemaphoreTake();
    sink->param.n9sco.share_info_base_addr->WriteIndex = 0;
    sink->param.n9sco.share_info_base_addr->ReadIndex  = 0;

    StreamDSP_HWSemaphoreGive();
}

static void Sco_Default_setting_init(void)
{
    if (N9SCO_setting != NULL) {
        return;
    }

    N9SCO_setting = pvPortMalloc(sizeof(Stream_n9sco_Config_t));

    memset(N9SCO_setting, 0, sizeof(Stream_n9sco_Config_t));
    N9SCO_setting->N9Sco_source.Process_Frame_Num = 2;
    N9SCO_setting->N9Sco_sink.Process_Frame_Num = 2;
    N9SCO_setting->N9Sco_sink.N9_Ro_abnormal_cnt = 0;
}

static void Sco_Source_N9Sco_Buffer_Init(SOURCE source)
{
    Sco_Rx_update_from_share_information(source);
    Sco_Rx_update_readoffset_share_information(source, 0);
    Sco_Rx_update_writeoffset_share_information(source, 0);
}

static void Sco_Tx_Forwarder_Buf_Init(SINK sink)
{
    memset((void *)hal_memview_cm4_to_dsp0(sink->streamBuffer.AVMBufferInfo.ForwarderAddr), 0, DSP_FORWARD_BUFFER_SIZE * 2);
}

static void Sco_Sink_N9Sco_Buffer_Init(SINK sink)
{
    Sco_Tx_update_from_share_information(sink);
    Sco_Reset_Sinkoffset_share_information(sink);

    /* Prefill 0 frame in Sink AVM Buffer */
    Sco_Tx_update_writeoffset_share_information(sink, 0);
}

#ifndef AIR_DCHS_MODE_ENABLE
static void Sco_Rx_Self_Recover_Process(SOURCE source, SINK sink)
{
    AUDIO_PARAMETER *audio;
    BUFFER_INFO *afe_buf_info;
    uint32_t differ_avm, esco_pcm_length, sample_rate, pattern_framesize;
    uint32_t prev_afe_ptr, curr_afe_ptr, differ_afe, prev_avm_ptr, curr_avm_ptr, avm_buf_len;

    prev_afe_ptr = g_esco_dl_afe_previous_writeoffset;
    curr_afe_ptr = sink->streamBuffer.BufferInfo.WriteOffset;
    afe_buf_info = &sink->streamBuffer.BufferInfo;
    audio = &sink->param.audio;
    if (++g_rx_fwd_running_count % 2 == 0) {
        /* Check and fix the WPTR of sink AFE buffer */
        if (prev_afe_ptr != ESCO_INVALID_VALUE) {
            if (curr_afe_ptr >= prev_afe_ptr) {
                differ_afe = curr_afe_ptr - prev_afe_ptr;
            } else {
                differ_afe = curr_afe_ptr + afe_buf_info->length - prev_afe_ptr;
            }
            /* Need to consider the impact of clock skew process, which may increase or decrease the esco_pcm_length */
            if (audio->AfeBlkControl.u4asrcflag) {
                sample_rate = audio->src_rate;
            } else {
                sample_rate = audio->rate;
            }
            esco_pcm_length = audio->channel_num * sample_rate * audio->period * audio->format_bytes / 1000;
            if ((differ_afe >= (esco_pcm_length + 100)) || (differ_afe <= (esco_pcm_length - 100))) {
                DSP_MW_LOG_I("[RX FWD] Do AFE buffer pointer fix, diff %d, prev pointer %d, curr pointer %d, compare value %d, %d, %d, %d, %d, %d", 9,
                             differ_afe, prev_afe_ptr, curr_afe_ptr, esco_pcm_length,
                             audio->channel_num, sample_rate, audio->period, audio->format_bytes, audio->AfeBlkControl.u4asrcflag);
                curr_afe_ptr = (prev_afe_ptr + esco_pcm_length) % afe_buf_info->length;
                /* Fix the AFE buffer pointer */
                afe_buf_info->WriteOffset = curr_afe_ptr;
                /* Fix the ASRC pointer */
                if (audio->AfeBlkControl.u4asrcflag) {
#ifdef ENABLE_HWSRC_CLKSKEW
                    if (ClkSkewMode_g == CLK_SKEW_V2) {
                        // enable src1 to avoid hwsrc underflow
                        if (afe_get_asrc_irq_is_enabled(AFE_MEM_ASRC_1, ASM_IER_IBUF_EMPTY_INTEN_MASK) == false) {
                            // afe_set_asrc_irq_enable(AFE_MEM_ASRC_1, false);
                            hal_src_set_irq_enable(AFE_MEM_ASRC_1, true);
                        }
                    } else {
                        hal_audio_set_value_parameter_t set_value_parameter;
                        set_value_parameter.set_current_offset.pure_agent_with_src = audio->mem_handle.pure_agent_with_src;
                        set_value_parameter.set_current_offset.memory_select = audio->mem_handle.memory_select;
                        set_value_parameter.set_current_offset.offset = curr_afe_ptr + (uint32_t)afe_buf_info->startaddr[0];
                        hal_audio_set_value(&set_value_parameter, HAL_AUDIO_SET_SRC_INPUT_CURRENT_OFFSET);
                    }
#else
                    hal_audio_set_value_parameter_t set_value_parameter;
                    set_value_parameter.set_current_offset.pure_agent_with_src = audio->mem_handle.pure_agent_with_src;
                    set_value_parameter.set_current_offset.memory_select = audio->mem_handle.memory_select;
                    set_value_parameter.set_current_offset.offset = curr_afe_ptr + (uint32_t)afe_buf_info->startaddr[0];
                    hal_audio_set_value(&set_value_parameter, HAL_AUDIO_SET_SRC_INPUT_CURRENT_OFFSET);
#endif
                    g_ignore_next_drop_flag = true;
                }
            }
        }
        /* Record the current ptr offset for next compare */
        g_esco_dl_afe_previous_writeoffset = curr_afe_ptr;

        /* Check and fix the RPTR of source AVM buffer */
        prev_avm_ptr = (uint32_t)source->streamBuffer.AVMBufferInfo.ReadIndex;
        curr_avm_ptr = (uint32_t)source->streamBuffer.AVMBufferInfo.WriteIndex;
        avm_buf_len = source->streamBuffer.AVMBufferInfo.MemBlkSize * source->streamBuffer.AVMBufferInfo.MemBlkNum;
        pattern_framesize = g_pattern_framesize * g_sco_frame_per_pkt_num;
        if (curr_avm_ptr <= prev_avm_ptr) {
            differ_avm = (avm_buf_len - prev_avm_ptr + curr_avm_ptr) / pattern_framesize;
        } else {
            differ_avm = (curr_avm_ptr - prev_avm_ptr) / pattern_framesize;
        }
        if (differ_avm != N9SCO_setting->N9Sco_source.Process_Frame_Num) {
            DSP_MW_LOG_I("[RX FWD] Do AVM buffer pointer fix, diff %d, prev pointer %d, curr pointer %d", 3, differ_avm, prev_avm_ptr, curr_avm_ptr);
            curr_avm_ptr = ((curr_avm_ptr + avm_buf_len) - N9SCO_setting->N9Sco_source.Process_Frame_Num * pattern_framesize) % avm_buf_len;
            Sco_Rx_update_readoffset_share_information(source, curr_avm_ptr);
        }
    }
}
#endif

static void Sco_RX_IntrHandler(void)
{
    uint16_t pattern_framesize, play_en_intra_clk = 0;
    uint32_t rx_forwarder_gpt_time, interval_gpt_time, spec_interval;
    uint32_t play_en_bt_clk = 0, play_en_nclk = 0;
    uint32_t avm_buf_len, avm_buf_next_wo, curr_rx_fwd_buf_idx;
    uint8_t *fd_packet_ptr, *fd_pattern_ptr, *avm_buf_ptr, *avm_info_ptr;
    VOICE_RX_SINGLE_PKT_STRU_PTR_t inbaud_info_1, inbaud_info_2;
    volatile SOURCE source = Source_blks[SOURCE_TYPE_N9SCO];
    volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    volatile SOURCE ul_source = Source_blks[SOURCE_TYPE_AUDIO];
    volatile SINK ul_sink = Sink_blks[SINK_TYPE_N9SCO];
    DSP_CALLBACK_PTR callback_ptr = NULL;
    DSP_STREAMING_PARA_PTR ul_stream = NULL;
    AUDIO_PARAMETER *sink_param = &sink->param.audio;
    N9SCO_PARAMETER *src_param = &source->param.n9sco;

    /* Check whether DL/UL streaming parameter is set up */
    if ((source == NULL) || (sink == NULL) || (source->transform == NULL) || (sink->transform == NULL)) {
        Forwarder_Rx_Intr_HW_Handler();
        return;
    }

    /* If disconnect is detected, need to disable Rx forwader IRQ and mute volume in order to avoid pop noise */
    AUDIO_ASSERT(g_debug_forwarder_assert_flag != true);
    if (Forwarder_Rx_Check_Disconnect_Status() == true) {
        g_debug_forwarder_assert_flag = true;
        Forwarder_Rx_Reset_Disconnect_Status();
        Forwarder_Rx_Intr_HW_Handler();
        hal_nvic_disable_irq(BT_AURX_IRQn);
        Forwarder_Rx_Intr_Ctrl(false);
        Forwarder_Tx_Intr_HW_Handler();
        hal_nvic_disable_irq(BT_AUTX_IRQn);
        Forwarder_Tx_Intr_Ctrl(false);
        // note: mute volume in order to avoid pop noise
        hal_ccni_message_t msg;
        msg.ccni_message[0] = 0;//For DL1
        msg.ccni_message[1] = (0xD120) | (HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX << 16); //-120dB
        DSP_GC_SetOutputVolume(msg, NULL);
        DSP_MW_LOG_W("[RX FWD] disconnect, set the volume(MIN) to avoid pop noise in DL1", 0);
        return;
    }

    /* Wait Rx stream init done */
    callback_ptr = DSP_Callback_Get(source, sink);
    if (callback_ptr == NULL) {
        Forwarder_Rx_Intr_HW_Handler();
        return;
    }
    if ((src_param->rx_forwarder_en == true) && (callback_ptr->Status != CALLBACK_SUSPEND)) {
        DSP_MW_LOG_W("[RX FWD] Warning: eSCO DL Start has been delayed by dl init for a little time, Status = %d", 1, callback_ptr->Status);
        Forwarder_Rx_Intr_HW_Handler();
        return;
    }

    /* Wait Tx stream init done */
    if ((ul_source == NULL) || (ul_sink == NULL)) {
        Forwarder_Rx_Intr_HW_Handler();
        return;
    }
    callback_ptr = DSP_Callback_Get(ul_source, ul_sink);
    if (callback_ptr == NULL) {
        Forwarder_Rx_Intr_HW_Handler();
        return;
    }
    if (g_esco_stream_ready_check == false) {
        ul_stream = DSP_Streaming_Get(ul_source, ul_sink);
        if (!((ul_stream->streamingStatus == STREAMING_START) && (callback_ptr->Status == CALLBACK_SUSPEND))) {
            DSP_MW_LOG_W("[RX FWD] Warning: eSCO UL Start has been delayed by ul init for a little time, Status = %d", 1, callback_ptr->Status);
            Forwarder_Rx_Intr_HW_Handler();
            return;
        }
    }

    /* Wait the audio hardware become ready */
    if ((g_esco_stream_ready_check == false) && !afe_audio_device_ready(SOURCE_TYPE_N9SCO, SINK_TYPE_AUDIO)) {
        DSP_MW_LOG_W("[RX FWD] warning afe device not ready", 0);
        Forwarder_Rx_Intr_HW_Handler();
        return;
    }

    /* Now the condition for stream process is ready, mark the global flag */
    g_esco_stream_ready_check = true;

#ifdef AIR_HFP_SYNC_START_ENABLE
    /* Notice Controller the DSP eSCO flow is ready now */
    Forwarder_Set_DSP_Ready_Status();

    /* Wait for the sync start event from the controller side */
    if (Forwarder_Check_Sync_Start_Status() == false) {
        DSP_MW_LOG_W("[RX FWD] wait sync start flag", 0);
        Forwarder_Rx_Intr_HW_Handler();
        return;
    }
    DSP_MW_LOG_W("[RX FWD] wait sync start flag done", 0);
#endif
    /* Check afe dl irq status*/
    if ((sink_param->irq_exist == false) && (src_param->rx_forwarder_en == false)) {
        DSP_MW_LOG_I("[RX FWD] DL AFE irq is not exist", 0);
    }

    /* Get frame size, frame type, frame number */
    pattern_framesize = Forwarder_Get_RX_FWD_Pattern_Size(gDspAlgParameter.EscoMode.Rx);
    if (pattern_framesize == 0) {
        DSP_MW_LOG_W("[RX FWD] pattern_framesize == 0", 0);
        Forwarder_Rx_Intr_HW_Handler();
        return;
    }
    if (g_sco_pkt_type == SCO_PKT_NULL) {
        g_sco_frame_per_pkt_num = Sco_Get_RX_FWD_Pkt_Type(&g_sco_pkt_type);
        g_pattern_framesize = pattern_framesize;
    }

    /* Check Rx fwd irq interval */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &rx_forwarder_gpt_time);
    if (g_rx_fwd_running_count != 0) {
        hal_gpt_get_duration_count(g_rx_fwd_irq_time, rx_forwarder_gpt_time, &interval_gpt_time);
        spec_interval = (g_pattern_framesize / 10) * 1250; /* 10 -> 1.25ms, 20 -> 2.5ms, 30 -> 3.75ms, 60 -> 7.5ms */
        if (gDspAlgParameter.EscoMode.Rx == VOICE_NB) {
            spec_interval /= 2;
        }
        if (interval_gpt_time < 0x10) {
            DSP_MW_LOG_W("[RX FWD] interval too close: %d us", 1, interval_gpt_time);
            g_rx_fwd_irq_time = rx_forwarder_gpt_time;
            Forwarder_Rx_Intr_HW_Handler();
            return;
        } else if ((interval_gpt_time > (spec_interval + 500)) || (interval_gpt_time < (spec_interval - 500))) {
            DSP_MW_LOG_W("[RX FWD] interval is not stable, expect %d, actual us: %d", 2, spec_interval, interval_gpt_time);
        }
    }
    g_rx_fwd_irq_time = rx_forwarder_gpt_time;

    /* Check Rx fwd buffer index */
    curr_rx_fwd_buf_idx = Forwarder_Rx_Status();
    if (((curr_rx_fwd_buf_idx != 0) && (curr_rx_fwd_buf_idx != 1)) || (g_prev_rx_fwd_buf_idx == curr_rx_fwd_buf_idx)) {
        DSP_MW_LOG_W("[RX FWD] get forwarder buffer index error, prev status:%d, current status:%d", 2, g_prev_rx_fwd_buf_idx, curr_rx_fwd_buf_idx);
        g_prev_rx_fwd_buf_idx = curr_rx_fwd_buf_idx;
        Forwarder_Rx_Intr_HW_Handler();
        return;
    }
    g_prev_rx_fwd_buf_idx = curr_rx_fwd_buf_idx;

    /* Fetch the AVM buffer info and forwarder buffer info
     * AVM buffer layout: frame 1#, frame 2#, ..., frame n#, inbaud for frame 1#, ..., inbaud for frame n#
     * forwarder buffer layout: inbaud for frame 1#, frame 1#, frame 2#, frame 2# (keep fixed 360 byte for each frame including inbaud)
     */
    Sco_Rx_update_from_share_information(source);
    avm_buf_len = source->streamBuffer.AVMBufferInfo.MemBlkSize * source->streamBuffer.AVMBufferInfo.MemBlkNum;
    fd_packet_ptr = (uint8_t *)(source->streamBuffer.AVMBufferInfo.ForwarderAddr + curr_rx_fwd_buf_idx * (DSP_FORWARD_BUFFER_SIZE + DSP_SCO_INBAND_INFORMATION));
    fd_pattern_ptr = (uint8_t *)(fd_packet_ptr + DSP_SCO_INBAND_INFORMATION);
    avm_buf_ptr = (uint8_t *)(source->streamBuffer.AVMBufferInfo.StartAddr + (uint32_t)source->streamBuffer.AVMBufferInfo.WriteIndex);
    avm_info_ptr = (uint8_t *)(source->streamBuffer.AVMBufferInfo.StartAddr + avm_buf_len + DSP_SCO_INBAND_INFORMATION * (((uint32_t)source->streamBuffer.AVMBufferInfo.WriteIndex) / (pattern_framesize * g_sco_pkt_type)));

    /* Copy Inband Info from forwarder buffer to AVM buffer */
    if (g_sco_pkt_type != SCO_PKT_2EV3) {
        if (g_sco_frame_id == 1) {
            DSP_D2C_BufferCopy(avm_info_ptr, fd_packet_ptr, DSP_SCO_INBAND_INFORMATION, (void *)(source->streamBuffer.AVMBufferInfo.StartAddr + avm_buf_len), DSP_SCO_INBAND_INFORMATION * source->streamBuffer.AVMBufferInfo.MemBlkNum);
        } else {
            inbaud_info_1 = (VOICE_RX_SINGLE_PKT_STRU_PTR_t)avm_info_ptr;
            inbaud_info_2 = (VOICE_RX_SINGLE_PKT_STRU_PTR_t)fd_packet_ptr;
            inbaud_info_1->InbandInfo.CrcErr |= inbaud_info_2->InbandInfo.CrcErr;
            inbaud_info_1->InbandInfo.HecErr |= inbaud_info_2->InbandInfo.HecErr;
            inbaud_info_1->InbandInfo.RxEd &= inbaud_info_2->InbandInfo.RxEd;
        }
        g_sco_frame_id++;
        if (g_sco_frame_id > g_sco_frame_per_pkt_num) {
            g_sco_frame_id = 1;
        }
    } else {
        DSP_D2C_BufferCopy(avm_info_ptr, fd_packet_ptr, DSP_SCO_INBAND_INFORMATION, (void *)(source->streamBuffer.AVMBufferInfo.StartAddr + avm_buf_len), DSP_SCO_INBAND_INFORMATION * source->streamBuffer.AVMBufferInfo.MemBlkNum);
    }

    /* Copy Inband Info from forwarder buffer to AVM buffer */
    DSP_D2C_BufferCopy(avm_buf_ptr, fd_pattern_ptr, pattern_framesize, (void *)source->streamBuffer.AVMBufferInfo.StartAddr, (uint16_t)avm_buf_len);

    /* For pkt lost debug: Accumulate Lost Pkt Num */
#ifdef DEBUG_ESCO_PLK_LOST
    src_param->forwarder_pkt_num ++;
    if (Voice_PLC_CheckInfoValid((VOICE_RX_SINGLE_PKT_STRU_PTR_t)fd_packet_ptr) == false) {
        src_param->lost_pkt_num++;
    }
    if (src_param->forwarder_pkt_num == 200) {
        DSP_MW_LOG_I("[RX FWD] lost packet: %d per %d pkt", 2, src_param->lost_pkt_num, src_param->forwarder_pkt_num);
        src_param->lost_pkt_num = 0;
        src_param->forwarder_pkt_num = 0;
    }
#endif

    /* Clean Fwd Pattern and inbaud Info */
    memset(fd_pattern_ptr, 0, pattern_framesize);
    Voice_PLC_CleanInfo((VOICE_RX_SINGLE_PKT_STRU_PTR_t)fd_packet_ptr);

    /* Update AVM buffer writeoffset */
    avm_buf_next_wo = ((uint32_t)source->streamBuffer.AVMBufferInfo.WriteIndex + (uint32_t)pattern_framesize) % avm_buf_len;
    Sco_Rx_update_writeoffset_share_information(source, avm_buf_next_wo);
    if (avm_buf_next_wo == source->streamBuffer.AVMBufferInfo.ReadIndex) {
        DSP_MW_LOG_I("[RX FWD] AVM buffer WPTR == RPTR %d %d", 2, avm_buf_next_wo, source->streamBuffer.AVMBufferInfo.ReadIndex);
    }

    /* Check the position offset of WPTR/RPTR for both AVM buffer and AFE buffer */
    source->streamBuffer.AVMBufferInfo.WriteIndex = (uint16_t)avm_buf_next_wo;
    if ((avm_buf_next_wo % (uint32_t)source->streamBuffer.AVMBufferInfo.MemBlkSize) == 0) {
        #ifndef AIR_DCHS_MODE_ENABLE
        Sco_Rx_Self_Recover_Process(source, sink);
        #endif
    }

    /* Trigger 1st DL AFE IRQ by below condition
     * 1. The AFE irq has not been triggered before.
     * 2. The trigger flow has not run before.
     * 3. The initial frame size meet the request of DL stream.
     */
    if ((sink_param->irq_exist == false) && (src_param->rx_forwarder_en == true) &&
        (avm_buf_next_wo == (uint32_t)source->streamBuffer.AVMBufferInfo.MemBlkSize * (uint32_t)N9SCO_setting->N9Sco_source.Process_Frame_Num)) {
#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_GPIO_DEBUG
        hal_gpio_toggle_pin(HAL_GPIO_23);
#endif

        /* Let the AFE play_en to be triggered in the next Rx anchor time */
        #ifdef AIR_DCHS_MODE_ENABLE
        if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
        #endif
            play_en_bt_clk = Forwarder_Rx_AncClk();
            MCE_TransBT2NativeClk(play_en_bt_clk, 1250, &play_en_nclk, &play_en_intra_clk, BT_CLK_Offset);
            hal_audio_afe_set_play_en(play_en_nclk, (uint32_t)play_en_intra_clk);
            DSP_MW_LOG_I("[RX FWD] Play_En config with bt_clk:0x%08x, play_en_nclk:0x%08x, play_en_intra_clk:0x%08x", 3, play_en_bt_clk, play_en_nclk, play_en_intra_clk);
        #ifdef AIR_DCHS_MODE_ENABLE
        }else{
            if(!dchs_dl_check_scenario_play_en_exist(HAL_AUDIO_AGENT_MEMORY_DL1)){
                dchs_dl_set_hfp_play_en();
            }
        }
        #endif

        /* Force to do context switch to resume DAVT task to process. */
        source->transform->Handler(source, sink);
        xTaskResumeFromISR(source->taskId);
        portYIELD_FROM_ISR(pdTRUE);

        /* For mSBC (AirMode: 3)
         *   packet type: 2EV3, packet size: 60, packet number: 1, interval: 7.5ms
         *   packet type: EV3,  packet size: 30, packet number: 2, interval: 3.75ms
         * For CVSD (AirMode: 2)
         *   packet type: 2EV3, packet size: 120, packet number: 1, interval: 7.5ms
         *   packet type: EV3,  packet size: 60, packet number: 2, interval: 3.75ms
         *   packet type: HV2,  packet size: 30, packet number: 3, interval: 2.5ms
         *   packet type: HV1,  packet size: 20, packet number: 4, interval: 1.25ms
         */
        DSP_MW_LOG_I("[RX FWD] First resume DAVT, packet type %d, packet size %d, packet number %d, Play_En config 0x%08x, 0x%08x, 0x%08x", 6,
                        g_sco_pkt_type, g_pattern_framesize, g_sco_frame_per_pkt_num,
                        play_en_bt_clk, play_en_nclk, play_en_intra_clk);

        /* Mark the trigger flow has already run */
        src_param->rx_forwarder_en = false;

        /* As the NR will change the out_channel_num, so need to save it here and restore in the beginning of every process */
        ul_stream = DSP_Streaming_Get(ul_source, ul_sink);
        ul_sink->param.n9sco.out_channel_num = ul_stream->callback.EntryPara.out_channel_num;
    }

    Forwarder_Rx_Intr_HW_Handler();
}

static void Sco_TX_IntrHandler(void)
{
    uint16_t pattern_framesize;
    uint8_t *fd_packet_ptr, *avm_buf_ptr;
    uint32_t tx_forwarder_gpt_time, interval_gpt_time, spec_interval;
    uint32_t avm_buf_len, avm_buf_next_ro, tx_first_forwarder_time, tx_buf_idx;
    volatile SINK sink = Sink_blks[SINK_TYPE_N9SCO];

    if (sink == NULL) {
        Forwarder_Tx_Intr_HW_Handler();
        return;
    }

    if (sink->param.n9sco.tx_forwarder_en == true) {
        sink->param.n9sco.tx_forwarder_en = false;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &tx_first_forwarder_time);
        DSP_MW_LOG_I("[TX FWD] First Tx forwarder IRQ, gpt_time: %d", 1, tx_first_forwarder_time);
    }

    /* Check Tx fwd irq interval */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &tx_forwarder_gpt_time);
    if (g_tx_fwd_running_count++ != 0) {
        hal_gpt_get_duration_count(g_tx_fwd_irq_time, tx_forwarder_gpt_time, &interval_gpt_time);
        spec_interval = (g_pattern_framesize / 10) * 1250; /* 10 -> 1.25ms, 20 -> 2.5ms, 30 -> 3.75ms, 60 -> 7.5ms */
        if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
            spec_interval /= 2;
        }
        if (interval_gpt_time < (spec_interval / 5)) {
            DSP_MW_LOG_W("[TX FWD] interval too close: %d us", 1, interval_gpt_time);
            g_tx_fwd_irq_time = tx_forwarder_gpt_time;
            Forwarder_Tx_Intr_HW_Handler();
            return;
        } else if ((interval_gpt_time > (spec_interval + 500)) || (interval_gpt_time < (spec_interval - 500))) {
            DSP_MW_LOG_W("[TX FWD] interval is not stable, expect %d, actual us: %d", 2, spec_interval, interval_gpt_time);
        }
    }
    g_tx_fwd_irq_time = tx_forwarder_gpt_time;

    /* Fetch the AVM buffer info */
    Sco_Tx_update_from_share_information_forwarder(sink);

    /* Check whether the AVM buffer is underflow */
    if (sink->streamBuffer.AVMBufferInfo.ReadIndex == sink->streamBuffer.AVMBufferInfo.WriteIndex) {
        DSP_MW_LOG_W("[TX FWD] RPTR == WPTR, %d %d", 2, sink->streamBuffer.AVMBufferInfo.ReadIndex, sink->streamBuffer.AVMBufferInfo.WriteIndex);
        Forwarder_Tx_Intr_HW_Handler();
        return;
    }

    /* Check Tx fwd buffer index */
    tx_buf_idx = Forwarder_Tx_Status();
    if (((tx_buf_idx != 0) && (tx_buf_idx != 1)) || (tx_buf_idx == g_prev_tx_fwd_buf_idx)) {
        DSP_MW_LOG_W("[TX FWD] tx forwarder buffer index error: prev %d, current %d", 2, g_prev_tx_fwd_buf_idx, tx_buf_idx);
    }
    g_prev_tx_fwd_buf_idx = tx_buf_idx;

    /* Copy the packet from the AVM buffer to Tx forwarder buffer */
    pattern_framesize = g_pattern_framesize;
    fd_packet_ptr = (uint8_t *)(sink->streamBuffer.AVMBufferInfo.ForwarderAddr + tx_buf_idx * DSP_FORWARD_BUFFER_SIZE);
    avm_buf_ptr = (uint8_t *)(sink->streamBuffer.AVMBufferInfo.StartAddr + (uint32_t)sink->streamBuffer.AVMBufferInfo.ReadIndex);
    memcpy(fd_packet_ptr, avm_buf_ptr, pattern_framesize);

#ifdef DEBUG_ESCO_TX_PKG_CONTENT_VERIFY
    /* Copy the data to temp buffer for later audio dump */
    memcpy(&g_debug_tx_temp_forwarder_buffer[g_debug_tx_temp_forwarder_index * pattern_framesize], fd_packet_ptr, pattern_framesize);
    g_debug_tx_temp_forwarder_index++;
    g_debug_tx_temp_forwarder_index %= DEBUG_TEMP_FWD_BUFFER_NUMBER;
    g_debug_tx_temp_count++;
#endif

    /* Update the RPTR of AVM buffer */
    avm_buf_len = (sink->streamBuffer.AVMBufferInfo.MemBlkSize) * (sink->streamBuffer.AVMBufferInfo.MemBlkNum);
    avm_buf_next_ro = ((uint32_t)sink->streamBuffer.AVMBufferInfo.ReadIndex + (uint32_t)pattern_framesize) % avm_buf_len;
    Sco_Tx_update_readoffset_share_information(sink, avm_buf_next_ro);

    Forwarder_Tx_Intr_HW_Handler();
}

static void Sco_Audio_Fwd_Ctrl(fowarder_ctrl forwarder_en, fowarder_type forwarder_type)
{
    if (forwarder_type == RX_FORWARDER) {
        if (forwarder_en == ENABLE_FORWARDER) {
            hal_nvic_register_isr_handler(BT_AURX_IRQn, (hal_nvic_isr_t)Sco_RX_IntrHandler);
            hal_nvic_enable_irq(BT_AURX_IRQn);
            Forwarder_Rx_Intr_Ctrl(false);
            Forwarder_Rx_Reset_Disconnect_Status();
        } else {
            hal_nvic_disable_irq(BT_AURX_IRQn);
            Forwarder_Rx_Intr_Ctrl(false);
        }
    } else if (forwarder_type == TX_FORWARDER) {
        if (forwarder_en == ENABLE_FORWARDER) {
            hal_nvic_register_isr_handler(BT_AUTX_IRQn, (hal_nvic_isr_t)Sco_TX_IntrHandler);
            hal_nvic_enable_irq(BT_AUTX_IRQn);
            Forwarder_Tx_Intr_Ctrl(false);
        } else {
            hal_nvic_disable_irq(BT_AUTX_IRQn);
            Forwarder_Tx_Intr_Ctrl(false);
            Forwarder_Tx_Buf_Ctrl(false);
        }
    }
}

ATTR_TEXT_IN_IRAM_LEVEL_2 static uint32_t SinkSlackSco(SINK sink)
{
    /* Always return the fix buffer size, and check the AVM buffer status in Forwarder buffer */
    return sink->param.n9sco.process_data_length;
}

static uint32_t SinkClaimSco(SINK sink, uint32_t extra)
{
    uint32_t sharebuflen, writeOffset, readOffset, RemainBuf;

    Sco_Tx_update_from_share_information(sink);

    sharebuflen = sink->streamBuffer.AVMBufferInfo.MemBlkSize * sink->streamBuffer.AVMBufferInfo.MemBlkNum;
    writeOffset = sink->streamBuffer.AVMBufferInfo.WriteIndex;
    readOffset  = sink->streamBuffer.AVMBufferInfo.ReadIndex;
    RemainBuf = (readOffset >= writeOffset) ? (sharebuflen - writeOffset + readOffset) : (writeOffset - readOffset);

    if ((extra != 0) && (RemainBuf > extra)) {
        return 0;
    } else {
        return SINK_INVALID_CLAIM;
    }
}

static uint8_t *SinkMapSco(SINK sink)
{
    Sco_Tx_update_from_share_information(sink);

    if (sink->streamBuffer.AVMBufferInfo.ReadIndex != 0) {
        memcpy(MapAddr + sink->streamBuffer.AVMBufferInfo.ReadIndex, &(sink->streamBuffer.AVMBufferInfo.StartAddr), sink->streamBuffer.AVMBufferInfo.ReadIndex);
    }

    return MapAddr;

}

ATTR_TEXT_IN_IRAM_LEVEL_2 static BOOL SinkFlushSco(SINK sink, uint32_t amount)
{
    uint16_t sharebuflen, framesize;
    DSP_STREAMING_PARA_PTR ul_stream = NULL;

    /* Restore the out_channel_num, as the NR will change it after each process. */
    ul_stream = DSP_Streaming_Get(sink->transform->source, sink);
    ul_stream->callback.EntryPara.out_channel_num = sink->param.n9sco.out_channel_num;

    if ((SinkSlackSco(sink) == 0) || (amount != sink->param.n9sco.process_data_length)) {
        return false;
    }

    Sco_Tx_update_from_share_information(sink);
    sharebuflen = sink->streamBuffer.AVMBufferInfo.MemBlkSize * sink->streamBuffer.AVMBufferInfo.MemBlkNum;
    framesize = sink->streamBuffer.AVMBufferInfo.MemBlkSize;
    sink->streamBuffer.AVMBufferInfo.WriteIndex = (sink->streamBuffer.AVMBufferInfo.WriteIndex + N9SCO_setting->N9Sco_sink.Process_Frame_Num * framesize) % (sharebuflen);
    Sco_Tx_update_writeoffset_share_information(sink, sink->streamBuffer.AVMBufferInfo.WriteIndex);

    if (sink->param.n9sco.tx_forwarder_en == true) {
        /*DSP_MW_LOG_I("[DSP][eSCO] sinkflush eSCO UL is first IRQ, wo:%d, ro:%d, sink->param.n9sco.tx_forwarder_en:%d", 3, sink->streamBuffer.AVMBufferInfo.WriteIndex, sink->streamBuffer.AVMBufferInfo.ReadIndex, sink->param.n9sco.tx_forwarder_en);*/
        Forwarder_Tx_Intr_HW_Handler();
        Forwarder_Tx_Intr_Ctrl(true);
        Forwarder_Tx_Buf_Ctrl(true);
    }

    if (sink->param.n9sco.tx_forwarder_en == true) {
        DSP_MW_LOG_I("[DSP][eSCO] sinkflush eSCO UL is first IRQ, wo:%d, ro:%d, sink->param.n9sco.tx_forwarder_en:%d", 3, sink->streamBuffer.AVMBufferInfo.WriteIndex, sink->streamBuffer.AVMBufferInfo.ReadIndex, sink->param.n9sco.tx_forwarder_en);
    }

    if (HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) {
        if (g_esco_dl_ul_process_active == false) {
            audio_nvdm_update_status(AUDIO_NVDM_USER_HFP, AUDIO_NVDM_STATUS_POST_CHANGE);
            g_esco_dl_ul_process_active = true;
        }
        if ((sink->param.n9sco.ul_reinit == true) && (Source_blks[SOURCE_TYPE_N9SCO] != NULL) && (Source_blks[SOURCE_TYPE_N9SCO]->param.n9sco.dl_reinit == true)) {
            Source_blks[SOURCE_TYPE_N9SCO]->param.n9sco.dl_reinit = false;
            sink->param.n9sco.ul_reinit = false;
            audio_nvdm_update_status(AUDIO_NVDM_USER_HFP, AUDIO_NVDM_STATUS_POST_CHANGE);
        }
    }

#ifdef DEBUG_ESCO_TX_PKG_CONTENT_VERIFY
    uint32_t irq_status, current_temp_forwarder_index;

    if (g_debug_tx_temp_count >= 6) {
        hal_nvic_save_and_set_interrupt_mask(&irq_status);
        current_temp_forwarder_index = g_debug_tx_temp_forwarder_index;
        hal_nvic_restore_interrupt_mask(irq_status);
        current_temp_forwarder_index = (current_temp_forwarder_index + DEBUG_TEMP_FWD_BUFFER_NUMBER - 2) % DEBUG_TEMP_FWD_BUFFER_NUMBER;
        LOG_AUDIO_DUMP((U8 *)&g_debug_tx_temp_forwarder_buffer[current_temp_forwarder_index * framesize], (U32)framesize, AUDIO_WOOFER_CPD_OUT);
        current_temp_forwarder_index = (current_temp_forwarder_index + DEBUG_TEMP_FWD_BUFFER_NUMBER + 1) % DEBUG_TEMP_FWD_BUFFER_NUMBER;
        LOG_AUDIO_DUMP((U8 *)&g_debug_tx_temp_forwarder_buffer[current_temp_forwarder_index * framesize], (U32)framesize, AUDIO_WOOFER_CPD_OUT);
    }
#endif

    return true;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 static BOOL SinkBufferWriteSco(SINK sink, uint8_t *src_addr, uint32_t length)
{
    uint8_t *write_ptr;
    uint16_t i, sharebuflen, framesize;

    if (sink->param.n9sco.process_data_length != length) {
        return false;
    }

    Sco_Tx_update_from_share_information(sink);

    sharebuflen = sink->streamBuffer.AVMBufferInfo.MemBlkSize * sink->streamBuffer.AVMBufferInfo.MemBlkNum;
    framesize = sink->streamBuffer.AVMBufferInfo.MemBlkSize;
    for (i = 0 ; i < N9SCO_setting->N9Sco_sink.Process_Frame_Num ; i++) {
        write_ptr = (uint8_t *)(sink->streamBuffer.AVMBufferInfo.StartAddr + sink->streamBuffer.AVMBufferInfo.WriteIndex);
        memcpy(write_ptr, src_addr + (uint32_t)(i * framesize), framesize);
        sink->streamBuffer.AVMBufferInfo.WriteIndex = (sink->streamBuffer.AVMBufferInfo.WriteIndex + framesize) % (sharebuflen);
    }

    return true;
}

static BOOL SinkCloseSco(SINK sink)
{
    sink->param.n9sco.process_data_length = 0;

    Sco_Audio_Fwd_Ctrl(DISABLE_FORWARDER, TX_FORWARDER);

#if defined(AIR_FIXED_RATIO_SRC)
    Sco_UL_Fix_Sample_Rate_Deinit();
#endif

    return true;
}

void SinkInitN9Sco(SINK sink)
{
    /* buffer init */
    Sco_Default_setting_init();
    sink->buftype = BUFFER_TYPE_CIRCULAR_BUFFER;
    Sco_Sink_N9Sco_Buffer_Init(sink);
    Sco_Tx_Forwarder_Buf_Init(sink);
    Sco_Audio_Fwd_Ctrl(ENABLE_FORWARDER, TX_FORWARDER);

    sink->param.n9sco.process_data_length = N9SCO_setting->N9Sco_sink.Process_Frame_Num * (sink->streamBuffer.AVMBufferInfo.MemBlkSize);
    sink->param.n9sco.tx_forwarder_en = false;

    /* interface init */
    sink->sif.SinkSlack       = SinkSlackSco;
    sink->sif.SinkClaim       = SinkClaimSco;
    sink->sif.SinkMap         = SinkMapSco;
    sink->sif.SinkFlush       = SinkFlushSco;
    sink->sif.SinkClose       = SinkCloseSco;
    sink->sif.SinkWriteBuf    = SinkBufferWriteSco;

    sink->param.n9sco.IsFirstIRQ = true;
    escoseqn = 0;
    sink->param.n9sco.ul_reinit = false;
}

ATTR_TEXT_IN_IRAM static uint32_t SourceSizeSco(SOURCE source)
{
    uint32_t writeOffset, readOffset, avmbufLen, processFrameLen, dataRemainLen;

    Sco_Rx_update_from_share_information(source);

    writeOffset = source->streamBuffer.AVMBufferInfo.WriteIndex;
    readOffset  = source->streamBuffer.AVMBufferInfo.ReadIndex;
    avmbufLen = source->streamBuffer.AVMBufferInfo.MemBlkSize * source->streamBuffer.AVMBufferInfo.MemBlkNum;
    processFrameLen = (N9SCO_setting->N9Sco_source.Process_Frame_Num) * source->streamBuffer.AVMBufferInfo.MemBlkSize;
    dataRemainLen = (readOffset > writeOffset) ? (avmbufLen - readOffset + writeOffset) : (writeOffset - readOffset);

    if ((dataRemainLen >= processFrameLen) || (source->param.n9sco.write_offset_advance != 0)) {
        return source->param.n9sco.process_data_length;
    } else {
        return 0;
    }
}

static uint8_t *SourceMapSco(SOURCE source)
{
    Sco_Rx_update_from_share_information(source);

    if (source->streamBuffer.AVMBufferInfo.ReadIndex != 0) {
        memcpy(MapAddr + source->streamBuffer.AVMBufferInfo.ReadIndex, &(source->streamBuffer.AVMBufferInfo.StartAddr), source->streamBuffer.AVMBufferInfo.ReadIndex);
    }

    return MapAddr;
}

#if defined (AIR_DCHS_MODE_ENABLE)
uint32_t dchs_hfp_handle;
uint32_t gpt_count_end;
static void hal_audio_gpt_trigger_mem(void)
{
    uint32_t savedmask = 0;
    uint32_t curr_cnt  = 0;
    U32 cur_native_bt_clk = 0, cur_native_bt_phase = 0;

    hal_nvic_save_and_set_interrupt_mask_special(&savedmask); // enter cirtical code region

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
    if (gpt_count_end > curr_cnt) { // gpt register does not overflow
        // DSP_MW_LOG_I("[DCHS UL][hfp set value] trigger %d curr_cnt %u  tar %d", 3, __LINE__, curr_cnt, gpt_count_end);
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            if (curr_cnt >= gpt_count_end) { // expire at time
                break;
            }
        }
    } else if (curr_cnt - gpt_count_end > 0x7fffffff) { // gpt register overflow
        // DSP_MW_LOG_I("[DCHS UL][hfp set value] trigger %d curr_cnt %u  tar %d", 3, __LINE__, curr_cnt, gpt_count_end);
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            if (curr_cnt >= gpt_count_end) { // expire at time
                if ((curr_cnt & 0x80000000) == 0x0) {
                    break;
                }
            }
        }
    } else {
        DSP_MW_LOG_E("[DCHS UL][hfp set value]Warning: already expire\r\n", 0);
        // AUDIO_ASSERT(0);
    }

    volatile SINK eSCO_sink = Sink_blks[SINK_TYPE_N9SCO];
    hal_audio_trigger_start_parameter_t start_parameter;
    start_parameter.memory_select = eSCO_sink->transform->source->param.audio.mem_handle.memory_select;
    start_parameter.enable = true;
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
    MCE_GetBtClk((BTCLK *)&cur_native_bt_clk, (BTPHASE *)&cur_native_bt_phase, DCHS_CLK_Offset);
    DSP_MW_LOG_I("[DCHS UL] eSCO UL SET Trigger Mem success cur_native_bt_clk:%u,cur_native_bt_phase:%u",2,cur_native_bt_clk,cur_native_bt_phase);
    hal_gpt_sw_free_timer(dchs_hfp_handle);
    hal_nvic_restore_interrupt_mask_special(savedmask);
}
#endif
ATTR_TEXT_IN_IRAM_LEVEL_2 static void SourceDropSco(SOURCE source, uint32_t amount)
{
    uint16_t framesize;
    uint32_t sharebuflen, relative_delay, delay_thd;
    volatile SINK eSCO_sink = Sink_blks[SINK_TYPE_N9SCO];
    hal_audio_trigger_start_parameter_t start_parameter;

    if (amount != source->param.n9sco.process_data_length) {
        return;
    }

    Sco_Rx_update_from_share_information(source);

    sharebuflen = source->streamBuffer.AVMBufferInfo.MemBlkSize * source->streamBuffer.AVMBufferInfo.MemBlkNum;
    framesize = source->streamBuffer.AVMBufferInfo.MemBlkSize;
    source->streamBuffer.AVMBufferInfo.ReadIndex = (source->streamBuffer.AVMBufferInfo.ReadIndex + N9SCO_setting->N9Sco_source.Process_Frame_Num * framesize) % (sharebuflen);
    Sco_Rx_update_readoffset_share_information(source, source->streamBuffer.AVMBufferInfo.ReadIndex);

    source->param.n9sco.write_offset_advance = 0;

    ///TODO: optimize this condition check
    if (((Sink_blks[SINK_TYPE_N9SCO] != NULL) && (Sink_blks[SINK_TYPE_N9SCO]->param.n9sco.IsFirstIRQ == true)) &&
        ((source->transform != NULL) && (source->param.n9sco.dl_enable_ul == false))) {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &(eSCO_sink->param.n9sco.ul_play_gpt));
        relative_delay = ((eSCO_sink->param.n9sco.ul_play_gpt - source->param.n9sco.ul_play_gpt) * 1000) >> 5;
        delay_thd = ((6 + ((relative_delay >> 10) / (15 * 4))) % 15); //extend THD every 15*4 ms
        if ((((relative_delay >> 10) % 15) < delay_thd) && (eSCO_sink->transform != NULL)) {
            eSCO_sink->param.n9sco.IsFirstIRQ = false;
            eSCO_sink->param.n9sco.tx_forwarder_en = true;
            hal_gpt_delay_us(delay_thd * 1000 - ((relative_delay * 1000 >> 10) % 15000));

            start_parameter.memory_select = eSCO_sink->transform->source->param.audio.mem_handle.memory_select;
            start_parameter.enable = true;
#if defined (AIR_DCHS_MODE_ENABLE)
            if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
                uint32_t count_1, hfp_delay_count,delay_time;
                S32 cur_native_bt_clk = 0,cur_native_bt_phase = 0;
                delay_time = 15;
                hal_sw_gpt_absolute_parameter_t  dchs_hfp_absolute_parameter;

                hal_gpt_sw_get_timer(&dchs_hfp_handle);
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count_1);
                MCE_GetBtClk((BTCLK *)&cur_native_bt_clk, (BTPHASE *)&cur_native_bt_phase, DCHS_CLK_Offset);

                hfp_delay_count = ((uint32_t)(delay_time * 1000));
                gpt_count_end = count_1 + hfp_delay_count;
                dchs_hfp_absolute_parameter.absolute_time_count = count_1 + hfp_delay_count - 600;
                dchs_hfp_absolute_parameter.callback = (void*)hal_audio_gpt_trigger_mem;
                dchs_hfp_absolute_parameter.maxdelay_time_count = hfp_delay_count;
                dps_uart_relay_ul_mem_sync_info(delay_time, cur_native_bt_clk, cur_native_bt_phase);
                hal_gpt_sw_start_timer_for_absolute_tick_1M(dchs_hfp_handle,&dchs_hfp_absolute_parameter);
            }else{
                hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
            }
#else
            hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
// #endif

            DSP_MW_LOG_I("[DSP][eSCO] UL SET TRIGGER MEM audio.rate %d audio.count %d\r\n", 2, eSCO_sink->transform->source->param.audio.rate, eSCO_sink->transform->source->param.audio.count);
// #endif
#endif
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &(eSCO_sink->param.n9sco.ul_play_gpt));
            DSP_MW_LOG_I("[DSP][eSCO] UL start from DL drop, delay :%d GTP_N :%d", 2, (delay_thd - relative_delay), eSCO_sink->param.n9sco.ul_play_gpt);
        } else {
            N9SCO_setting->N9Sco_sink.N9_Ro_abnormal_cnt = 0;
            SinkFlushSco(eSCO_sink, eSCO_sink->param.n9sco.process_data_length);
            DSP_MW_LOG_I("[DSP][eSCO] UL start from DL drop too late, delay :%d GTP_N :%d, GPT_P :%d\r\n", 3, relative_delay, (eSCO_sink->param.n9sco.ul_play_gpt), source->param.n9sco.ul_play_gpt);
        }
    }

    escoseqn += 2;
}

static BOOL SourceConfigureSco(SOURCE source, stream_config_type type, uint32_t value)
{
    switch (type) {
        case SCO_SOURCE_WO_ADVANCE:
            source->param.n9sco.write_offset_advance = value;
            break;
        default:
            return false;
            break;
    }

    return true;
}

bool Check_Forwarder_Disable_Status(void)
{
    if ((rBb->_reserved_dword_904h[1] & (1 << 3)) != 0) {
        return true;
    }
    return false;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 static BOOL SourceReadBufSco(SOURCE source, uint8_t *dst_addr, uint32_t length)
{
    uint16_t i, sharebuflen, framesize;
    uint8_t *write_ptr;
    uint8_t *info_ptr;
    VOICE_RX_INBAND_INFO_t RxPacketInfo;

    if (source->param.n9sco.process_data_length != length) {
        return false;
    }

#ifdef AIR_HFP_SYNC_STOP_ENABLE
    if (g_esco_dl_sync_stop_detect < 0) {
        if (Forwarder_Check_Sync_Stop_Status() == true) {
            /* If the Rx fowarder has been stopped, we need to mute the output */
            g_esco_dl_sync_stop_detect = 0;
            dsp_sync_callback_hfp(SUBMSG_MCU2DSP_SYNC_STOP, NULL);
#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_GPIO_DEBUG
            hal_gpio_toggle_pin(HAL_GPIO_23);
#endif
            DSP_MW_LOG_I("[DSP][eSCO] [Source Read] detect sync stop event", 0);
        }
    } else {
        if (++g_esco_dl_sync_stop_detect == ESCO_SYNC_STOP_DELAY_ACK_TIME) {
            /* Delay some time for the voice to ramp down to mute */
            Forwarder_Reset_Sync_Stop_Status();
            DSP_MW_LOG_I("[DSP][eSCO] [Source Read] ack sync stop event", 0);
        }
    }
#endif

    Sco_Rx_update_from_share_information(source);
    sharebuflen = source->streamBuffer.AVMBufferInfo.MemBlkSize * source->streamBuffer.AVMBufferInfo.MemBlkNum;
    framesize = source->streamBuffer.AVMBufferInfo.MemBlkSize;
    for (i = 0; i < N9SCO_setting->N9Sco_source.Process_Frame_Num; i++) {
        write_ptr = (uint8_t *)(source->streamBuffer.AVMBufferInfo.StartAddr + source->streamBuffer.AVMBufferInfo.ReadIndex);
        info_ptr = (uint8_t *)(source->streamBuffer.AVMBufferInfo.StartAddr + (uint32_t)sharebuflen + DSP_SCO_INBAND_INFORMATION * (((uint32_t)source->streamBuffer.AVMBufferInfo.ReadIndex) / (uint32_t)framesize));
        RxPacketInfo = (((VOICE_RX_SINGLE_PKT_STRU_PTR_t)info_ptr)->InbandInfo);
#ifdef AIR_HFP_SYNC_STOP_ENABLE
        /* Mark frame as packet expire status when sync stop is detect */
        if (g_esco_dl_sync_stop_detect >= 0) {
            ((VOICE_RX_SINGLE_PKT_STRU_PTR_t)info_ptr)->InbandInfo.CrcErr = 1;
            ((VOICE_RX_SINGLE_PKT_STRU_PTR_t)info_ptr)->InbandInfo.HecErr = 1;
            ((VOICE_RX_SINGLE_PKT_STRU_PTR_t)info_ptr)->InbandInfo.RxEd = 0;
        }
#endif
        if (Voice_PLC_CheckInfoValid((VOICE_RX_SINGLE_PKT_STRU_PTR_t)info_ptr) == false) {
            DSP_MW_LOG_I("[DSP][eSCO] [Source Read] meet packet expired %d", 1, escoseqn + i);
            Voice_PLC_CheckAndFillZeroResponse((S16 *)(dst_addr + i * framesize), gDspAlgParameter.EscoMode.Rx);
        } else {
            if (Check_Forwarder_Disable_Status()) {
                DSP_MW_LOG_I("[DSP][eSCO] [Source Read] prefill silence for forwarder disable %d", 1, escoseqn + i);
                Voice_PLC_CheckAndFillZeroResponse((S16 *)(dst_addr + i * framesize), gDspAlgParameter.EscoMode.Rx);
            } else {
                memcpy(dst_addr + i * framesize, write_ptr, framesize);
            }
        }
#ifdef AIR_HFP_SYNC_STOP_ENABLE
        /* Mark PLC status as good frame status when sync stop is detect */
        if (g_esco_dl_sync_stop_detect >= 0) {
            RxPacketInfo.CrcErr = 0;
            RxPacketInfo.HecErr = 0;
            RxPacketInfo.RxEd = 1;
        }
#endif
        Voice_PLC_UpdateInbandInfo(&RxPacketInfo, sizeof(VOICE_RX_INBAND_INFO_t), i);
        source->streamBuffer.AVMBufferInfo.ReadIndex = (source->streamBuffer.AVMBufferInfo.ReadIndex + framesize) % (sharebuflen);
    }

    return true;
}

static BOOL SourceCloseSco(SOURCE source)
{
    source->param.n9sco.process_data_length = 0;
    g_ignore_next_drop_flag = false;
    Sco_Audio_Fwd_Ctrl(DISABLE_FORWARDER, RX_FORWARDER);

#if defined(AIR_FIXED_RATIO_SRC)
    Sco_DL_Fix_Sample_Rate_Deinit();
#endif

    return true;
}

void SourceInitN9Sco(SOURCE source)
{
    /* buffer init */
    Sco_Default_setting_init();
    source->buftype = BUFFER_TYPE_CIRCULAR_BUFFER;
    Sco_Source_N9Sco_Buffer_Init(source);
    Sco_Audio_Fwd_Ctrl(ENABLE_FORWARDER, RX_FORWARDER); // register rx forwarder irq handler

    source->param.n9sco.process_data_length = (N9SCO_setting->N9Sco_source.Process_Frame_Num) * (source->streamBuffer.AVMBufferInfo.MemBlkSize);

    /* interface init */
    source->sif.SourceSize        = SourceSizeSco;
    source->sif.SourceReadBuf     = SourceReadBufSco;
    source->sif.SourceMap         = SourceMapSco;
    source->sif.SourceConfigure   = SourceConfigureSco;
    source->sif.SourceDrop        = SourceDropSco;
    source->sif.SourceClose       = SourceCloseSco;

    /* Enable Interrupt */
    source->param.n9sco.IsFirstIRQ = true;
    source->param.n9sco.dl_enable_ul = true;
    source->param.n9sco.write_offset_advance = 0;
#ifdef DEBUG_ESCO_PLK_LOST
    source->param.n9sco.lost_pkt_num = 0;
    source->param.n9sco.forwarder_pkt_num = 0;
#endif
    source->param.n9sco.dl_reinit = false;

    g_sco_frame_id = 1;
    g_debug_forwarder_assert_flag = false;
    g_sco_pkt_type = SCO_PKT_NULL;
    g_prev_rx_fwd_buf_idx = ESCO_INVALID_VALUE;
    g_prev_tx_fwd_buf_idx = ESCO_INVALID_VALUE;
    g_esco_dl_afe_previous_writeoffset = ESCO_INVALID_VALUE;
    g_esco_ul_afe_previous_readoffset = ESCO_INVALID_VALUE;
    g_esco_ul_avm_previous_writeoffset = ESCO_INVALID_VALUE;
    g_esco_stream_ready_check = false;
#ifdef AIR_HFP_SYNC_STOP_ENABLE
    g_esco_dl_sync_stop_detect = -1;
#endif
    g_rx_fwd_running_count = 0;
    g_tx_fwd_running_count = 0;
    g_esco_dl_ul_process_active = false;
    g_ignore_next_drop_flag = false;
#ifdef DEBUG_ESCO_TX_PKG_CONTENT_VERIFY
    g_debug_tx_temp_forwarder_index = 0;
    g_debug_tx_temp_count = 0;
    memset(g_debug_tx_temp_forwarder_buffer, 0, sizeof(g_debug_tx_temp_forwarder_buffer));
#endif

#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_GPIO_DEBUG
    hal_gpio_init(HAL_GPIO_23);
    hal_pinmux_set_function(HAL_GPIO_23, 0);
    hal_gpio_set_direction(HAL_GPIO_23, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(HAL_GPIO_23, HAL_GPIO_DATA_LOW);
#endif
}

