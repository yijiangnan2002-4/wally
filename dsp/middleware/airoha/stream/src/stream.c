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
 *@file   Stream.c
 *@brief  define stream api provides efficiently processing streams
 *
 @verbatim
 @endverbatim
 */

//- system header
#include "dlist.h"
#include "stream.h"
#include "dtm.h"
//- module header
#include "source_inter.h"
#include "sink_inter.h"
#include "dsp_callback.h"

#include "stream.h"
#include <string.h>
#include "dsp_memory.h"
#include "stream_memory.h"
#include "stream_n9_a2dp.h"
#include "types.h"
#include "stream_n9sco.h"
#include "hal_hw_semaphore.h"
#include "hal_audio_afe_control.h"
#include "stream_audio_hardware.h"
#include "stream_audio_driver.h"
#include "stream_audio_afe.h"
#include "stream_audio.h"
#include "dsp_audio_ctrl.h"
#include "stream_cm4_record.h"
#include <string.h>
#include "stream_cm4_playback.h"
#include "stream_cm4_vp_playback.h"
#include "stream_audio_transmitter.h"
#include "dsp_share_memory.h"
#include "dsp_scenario.h"
#ifdef AIR_BT_CODEC_BLE_ENABLED
#include "stream_n9ble.h"
#ifdef AIR_BT_LE_LC3_ENABLE
#include "lc3_codec_api.h"
#endif
#endif
#include "stream_audio_transmitter.h"
#include "stream_bt_common.h"

#ifdef ENABLE_HWSRC_CLKSKEW
#include "clk_skew.h"
#endif
#ifdef AIR_CELT_ENC_ENABLE
#include "celt_enc_interface.h"
#endif

#ifdef MTK_SENSOR_SOURCE_ENABLE
#include "stream_gsensor_detect.h"
#include "stream_data_ul.h"
#endif
#ifdef AIR_AUDIO_ULD_DECODE_ENABLE
#include "uld_dec_interface.h"
#endif

#ifdef MTK_CELT_DEC_ENABLE
#include "celt_dec_interface.h"
#endif
#ifdef AIR_SOFTWARE_SRC_ENABLE
#include "sw_src_interface.h"
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
#include "clk_skew_sw.h"
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
#include "sw_gain_interface.h"
#endif
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
#include "sw_buffer_interface.h"
#endif
#ifdef AIR_SOFTWARE_MIXER_ENABLE
#include "sw_mixer_interface.h"
#endif /* AIR_SOFTWARE_MIXER_ENABLE */

#ifdef AIR_GAMING_MODE_DONGLE_ENABLE
#include "scenario_ull_audio.h"
#endif /* AIR_GAMING_MODE_DONGLE_ENABLE */

#ifdef AIR_WIRED_AUDIO_ENABLE
#include "scenario_wired_audio.h"
#endif /* AIR_WIRED_AUDIO_ENABLE */

#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
#include "scenario_ble_audio.h"
#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
#include "lc3_enc_branch_interface.h"
#include "lc3_codec_portable.h"
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */

#ifdef AIR_ULL_AUDIO_V2_DONGLE_ENABLE
#include "scenario_ull_audio_v2.h"
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */

#ifdef AIR_VP_NOT_USE_HWSRC_DEFAULT_ENABLE
#include "dsp_scenario.h"
extern CONNECTION_IF n9_sco_dl_if;
#endif

#ifdef AIR_BT_LE_LC3PLUS_ENABLE
#include "lc3plus_dec_interface.h"
#include "lc3plus_enc_interface.h"
#endif

#if defined(AIR_CELT_DEC_V2_ENABLE)
#include "celt_dec_interface_v2.h"
#endif /* AIR_CELT_DEC_V2_ENABLE */

#if defined(AIR_CELT_ENC_V2_ENABLE)
#include "celt_enc_interface_v2.h"
#endif /* AIR_CELT_ENC_V2_ENABLE */

#ifdef AIR_DCHS_MODE_ENABLE
#include "stream_dchs.h"
#endif

#ifdef AIR_WIRELESS_MIC_RX_ENABLE
#include "scenario_wireless_mic_rx.h"
#endif

#ifdef ENABLE_HWSRC_CLKSKEW
#include "audio_hwsrc_monitor.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Constant Definitions ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define HwSemRetryTimes 50000
#define ASRC_OUT_BUFFER_SIZE 4096;

extern uint16_t g_ble_abr_length;
extern uint16_t g_ble_ul_abr_length;

extern bool ULL_NrOffloadFlag;
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
extern stream_feature_list_t stream_feature_list_bt_audio_dongle_usb_in_a2dp_lhdc[];
#endif
////////////////////////////////////////////////////////////////////////////////
// Function Declarations ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
BOOL StreamBufferToBufferHandler(SOURCE source, SINK sink)
{
    U32 result = FALSE;
    U32 RxSize, TxSize, MoveLength;

    if (source && sink) {
        RxSize = SourceSize(source);
        TxSize = SinkSlack(sink);
        MoveLength = RxSize < TxSize ? RxSize : TxSize;

        /* use drop and flush to handle wrap case */
#if 1
        VOLATILE BUFFER_INFO_PTR sourceBuffer = &source->streamBuffer.BufferInfo;
        VOLATILE BUFFER_INFO_PTR sinkBuffer   = &sink->streamBuffer.BufferInfo;

        while (MoveLength) {
            U32 sourceResidue = sourceBuffer->length - sourceBuffer->ReadOffset;
            U32 sinkResidue   = sinkBuffer->length - sinkBuffer->WriteOffset;

            U32 length = MIN(MoveLength, MIN(sourceResidue, sinkResidue));
            memcpy(sinkBuffer->startaddr[0] + sinkBuffer->WriteOffset, sourceBuffer->startaddr[0] + sourceBuffer->ReadOffset, length);
            SinkFlush(sink, length);
            SourceDrop(source, length);
            MoveLength -= length;
        }
        /* normal case */
#else
        if (MoveLength) {
            result = TRUE;
            BUFFER_INFO_PTR sourceBuffer = &source->streamBuffer.BufferInfo;
            BUFFER_INFO_PTR sinkBuffer   = &sink->streamBuffer.BufferInfo;

            U32 sourceResidue = sourceBuffer->length - sourceBuffer->ReadOffset;
            U32 sinkResidue   = sinkBuffer->length - sinkBuffer->WriteOffset;

            /* source buffer wrap */
            if (MoveLength > sourceResidue) {
                /* sink buffer wrap */
                if (MoveLength > sinkResidue) {
                    /* sink more residue */
                    if (sinkResidue > sourceResidue) {
                        U32 len1 = sourceResidue;
                        U32 len2 = sinkResidue - len1;
                        U32 len3 = MoveLength - len1 - len2;
                        memcpy(sinkBuffer->startaddr[0] + sinkBuffer->WriteOffset, sourceBuffer->startaddr[0] + sourceBuffer->ReadOffset, len1);
                        memcpy(sinkBuffer->startaddr[0] + sinkBuffer->WriteOffset + len1, sourceBuffer->startaddr[0], len2);
                        memcpy(sinkBuffer->startaddr[0], sourceBuffer->startaddr[0] + len2, len3);
                    }
                    /* source more residue */
                    else {
                        U32 len1 = sinkResidue;
                        U32 len2 = sourceResidue - len1;
                        U32 len3 = MoveLength - len1 - len2;
                        memcpy(sinkBuffer->startaddr[0] + sinkBuffer->WriteOffset, sourceBuffer->startaddr[0] + sourceBuffer->ReadOffset, len1);
                        memcpy(sinkBuffer->startaddr[0], sourceBuffer->startaddr[0] + sourceBuffer->ReadOffset + len1, len2);
                        memcpy(sinkBuffer->startaddr[0] + len2, sourceBuffer->startaddr[0], len3);
                    }
                }
                /* sink buffer no wrap */
                else {
                    U32 len1 = sourceResidue;
                    U32 len2 = MoveLength - len1;
                    memcpy(sinkBuffer->startaddr[0] + sinkBuffer->WriteOffset, sourceBuffer->startaddr[0] + sourceBuffer->ReadOffset, len1);
                    memcpy(sinkBuffer->startaddr[0] + sinkBuffer->WriteOffset + len1, sourceBuffer->startaddr[0], len2);
                }
            }
            /* souce buffer no wrap */
            else {
                /* sink buffer wrap */
                if (MoveLength > sinkResidue) {
                    U32 len1 = sinkResidue;
                    U32 len2 = MoveLength - len1;
                    memcpy(sinkBuffer->startaddr[0] + sinkBuffer->WriteOffset, sourceBuffer->startaddr[0] + sourceBuffer->ReadOffset, len1);
                    memcpy(sinkBuffer->startaddr[0], sourceBuffer->startaddr[0] + sourceBuffer->ReadOffset + len1, len2);
                }
                /* sink buffer no wrap */
                else {
                    memcpy(sinkBuffer->startaddr[0] + sinkBuffer->WriteOffset, sourceBuffer->startaddr[0] + sourceBuffer->ReadOffset, MoveLength);
                }
            }
        }
        SinkFlush(sink, MoveLength);
        SourceDrop(source, MoveLength);
#endif
        result = TRUE;
    }

    return result;
}

BOOL StreamBufferToQueueHandler(SOURCE source, SINK sink)
{
    BOOL result = FALSE;
    U32 RxSize, TxSize, MoveLength;

    if (source && sink) {
        RxSize = SourceSize(source);
        TxSize = SinkSlack(sink);
        MoveLength = RxSize < TxSize ? RxSize : TxSize;

        if (MoveLength) {
            BUFFER_INFO_PTR sourceBuffer = &source->streamBuffer.BufferInfo;

            U32 SinkOffset = SinkClaim(sink, MoveLength);
            U8 *TxBuffer   = SinkMap(sink);
            U32 sourceResidue = sourceBuffer->length - sourceBuffer->ReadOffset;

            /* source buffer wrap */
            if (MoveLength > sourceResidue) {
                U32 len1 = sourceResidue;
                U32 len2 = MoveLength - len1;
                memcpy(TxBuffer + SinkOffset, sourceBuffer->startaddr[0] + sourceBuffer->ReadOffset, len1);
                memcpy(TxBuffer + SinkOffset + len1, sourceBuffer->startaddr[0], len2);
            }
            /* source buffer no wrap */
            else {
                memcpy(TxBuffer + SinkOffset, sourceBuffer->startaddr[0] + sourceBuffer->ReadOffset, MoveLength);
            }
            SinkFlush(sink, MoveLength);
            SourceDrop(source, MoveLength);
            result = TRUE;
        }
    }
    return result;
}


BOOL StreamDirectHandler(SOURCE source, SINK sink)
{
    U32 size;
    U8 *Source_ptr;
    U8 *Sink_ptr;

    size = SourceSize(source);
    if (!size) {
        return TRUE;
    }

    // Try to get space of the same size for writing.
    if (SinkClaim(sink, size) != 0xffffffff) {
        Sink_ptr = SinkMap(sink);
        Source_ptr = SourceMap(source);
        memcpy(Sink_ptr, Source_ptr, size);
        SinkFlush(sink, size);
        // Clear the input data.
        SourceDrop(source, size);
        return TRUE;
    }

    return FALSE;

}

/**
 * @brief malloc memory for source
 *
 * @param  SourceType   specific which source to malloc
 * @return   pointer of source
 */
SOURCE new_source(SOURCE_TYPE SourceType)
{
    SOURCE source;

    source = pvPortMalloc(sizeof(SOURCE_T));
    if (source) {
        memset(source, 0, sizeof(SOURCE_T));
        source->type = SourceType;
        Source_blks[SourceType] = source;
    }
    return source;
}

/**
 * @brief malloc memory for sink
 *
 * @param  SinkType   specific which sink to malloc
 * @return   pointer of sink
 */
SINK new_sink(SINK_TYPE SinkType)
{
    SINK sink;

    sink = pvPortMalloc(sizeof(SINK_T));
    if (sink) {
        memset(sink, 0, sizeof(SINK_T));
        sink->type = SinkType;
        Sink_blks[SinkType] = sink;
    }
    return sink;
}

/**
 * @brief Move the specified number of bytes from source to sink.
 *
 * @param sink   The Sink to move data to.
 * @param source The Source to move data from.
 * @param count  The number of bytes to move.
 * @return Zero on failure and the count on success.
 */
U32 StreamMove(SINK sink, SOURCE source, U32 count)
{
    //#warning not implement yet
    UNUSED(sink);
    UNUSED(source);
    UNUSED(count);
    return 0;
}

/**
 * @brief Make an automatic connection between a source and sink
 *
 * @param source The Source data will be taken from.
 * @param sink   The Sink data will be written to.
 *
 * @return a transform on success, or zero on failure.
 */
TRANSFORM StreamConnect(SOURCE source, SINK sink)
{
    TRANSFORM transform = NULL;

    if (source && sink) {
        /* check if source and sink have transform*/
        if ((source->transform == NULL) && (sink->transform == NULL)) {
            transform = pvPortMalloc(sizeof(TRANSFORM_T));
            if (transform) {
                memset(transform, 0, sizeof(TRANSFORM_T));
                transform->source = source;
                transform->sink = sink;
                source->transform = transform;
                sink->transform = transform;

                dlist_init(&transform->list);
                dlist_append(&transform->list, &gTransformList);

                /* assign handler */
                if ((source->buftype == BUFFER_TYPE_CIRCULAR_BUFFER) && (sink->buftype == BUFFER_TYPE_CIRCULAR_BUFFER)) {
                    transform->Handler = StreamBufferToBufferHandler;
                } else if ((source->buftype == BUFFER_TYPE_CIRCULAR_BUFFER) && (sink->buftype == BUFFER_TYPE_QUEUE)) {
                    transform->Handler = StreamBufferToQueueHandler;
                } else if ((source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER) && (sink->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER)) {
                    transform->Handler = StreamBufferToQueueHandler;
                } else if ((source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER) && (sink->buftype == BUFFER_TYPE_QUEUE)) {
                    transform->Handler = StreamBufferToQueueHandler;
                } else if ((source->buftype == BUFFER_TYPE_QUEUE) && (sink->buftype == BUFFER_TYPE_CIRCULAR_BUFFER)) {
                    /* do we have this case? */
                } else {
                    transform->Handler = StreamDirectHandler;
                }
            }
        } else if (source->transform == sink->transform) {
            transform = source->transform;
        }
    }
    return transform;
}

/**
 * @brief Break transform of source and sink
 *
 * @param transform the transform to break
 */
VOID StreamDisconnect(TRANSFORM transform)
{
    if (transform) {
        if ((transform->source) && (transform->sink)) {
            if (transform == transform->source->transform) {
                if (transform->TransformClose != NULL) {
                    PL_CRITICAL(TransformChangeHandlerClose, (VOID *)transform);
                } else {
                    StreamTransformClose(transform);
                }
            }
        }
    }
}

/**
 * @brief Request to create an audio source
 * @param hardware The audio hardware which would be reserved as a source
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Source ID associated with the audio hardware.
 */
SOURCE StreamAudioAfeSource(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    UNUSED(instance);

    // TODO: line in and eSCO input selection
    Source_Audio_SelectInstance(hardware, instance);
    SOURCE source = NULL;
    if ((Source_blks[SOURCE_TYPE_AUDIO] != NULL)
        && (Source_blks[SOURCE_TYPE_AUDIO]->param.audio.HW_type == hardware)
        && (Source_blks[SOURCE_TYPE_AUDIO]->param.audio.channel_sel == channel)) {
        DSP_MW_LOG_I("source exit", 0);
        return Source_blks[SOURCE_TYPE_AUDIO];
    } else if (Source_blks[SOURCE_TYPE_AUDIO] == NULL) {
        source = new_source(SOURCE_TYPE_AUDIO);
        if (source == NULL) {
            return NULL;
        }
    } else {
        source = Source_blks[SOURCE_TYPE_AUDIO];
    }

    source->param.audio.HW_type     = hardware;
    source->param.audio.channel_sel = channel;
    source->taskId     = DAV_TASK_ID;
    source->buftype                   = BUFFER_TYPE_INTERLEAVED_BUFFER;

    Audio_Default_setting_init(); // Temp for audio test
    Source_Audio_Get_Default_Parameters(source);

    audio_afe_set_ops(source);
    Source_Audio_HW_Init(source);
#ifdef AIR_I2S_SLAVE_ENABLE
    if (source->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        source->param.audio.AfeBlkControl.u4asrcflag = true;
    }
#endif
#ifdef AIR_HWSRC_RX_TRACKING_ENABLE
    if (source->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        if (source->param.audio.mem_handle.src_rate != 0) {
            DSP_MW_LOG_I("[HWSRC] enable source hwsrc", 0);
            source->param.audio.mem_handle.pure_agent_with_src = true;
        }
    }
#endif
    Source_Audio_Path_Init(source);
    Source_Audio_Afe_Path_Interface_Init(source);

    return source;
}

#ifdef AIR_I2S_SLAVE_ENABLE
/**
 * @brief Request to create an audio source
 * @param hardware The audio hardware which would be reserved as a source
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Source ID associated with the audio hardware.
 */

SOURCE StreamAudioAfe2Source(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    UNUSED(instance);

    // TODO: line in and eSCO input selection
    Source_Audio_SelectInstance(hardware, instance);

    SOURCE source;
    if ((Source_blks[SOURCE_TYPE_AUDIO2] != NULL)
        && (Source_blks[SOURCE_TYPE_AUDIO2]->param.audio.HW_type == hardware)
        && (Source_blks[SOURCE_TYPE_AUDIO2]->param.audio.channel_sel == channel)) {
        return Source_blks[SOURCE_TYPE_AUDIO2];
    } else if (Source_blks[SOURCE_TYPE_AUDIO2] == NULL) {
        source = new_source(SOURCE_TYPE_AUDIO2);
        if (source == NULL) {
            return NULL;
        }
    } else {
        source = Source_blks[SOURCE_TYPE_AUDIO2];
    }

    source->param.audio.HW_type     = hardware;
    source->param.audio.channel_sel = channel;
    source->taskId     = DAV_TASK_ID;
    source->buftype                   = BUFFER_TYPE_INTERLEAVED_BUFFER;

    Audio_Default_setting_init(); // Temp for audio test
    Source_Audio_Get_Default_Parameters(source);

    audio_afe_set_ops(source);
    Source_Audio_HW_Init(source);
    //#ifndef HAL_AUDIO_ENABLE_PATH_MEM_DEVICE
#ifdef AIR_I2S_SLAVE_ENABLE
    if (source->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        DSP_MW_LOG_I("[HAS] enter AfeBlkControl.u4asrcflag", 0);
        source->param.audio.AfeBlkControl.u4asrcflag = true;
    }
#endif
    Source_Audio_Path_Init(source);
    Source_Audio_Afe_Path_Interface_Init(source);

    return source;
}
#endif

/**
 * @brief Request to create an audio sub-source
 * @param hardware The audio hardware which would be reserved as a source
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Source ID associated with the audio hardware.
 */
SOURCE StreamAudioAfeSubSource(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    SOURCE source = NULL;
    UNUSED(instance);
    UNUSED(hardware);
    UNUSED(channel);
#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE) || defined (AIR_BT_AUDIO_DONGLE_ENABLE)
    // TODO: line in and eSCO input selection
    Source_Audio_SelectInstance(hardware, instance);

    SOURCE_TYPE      source_type;
    for (source_type = SOURCE_TYPE_SUBAUDIO_MIN ; source_type <= SOURCE_TYPE_SUBAUDIO_MAX ; source_type++) {
        if (Source_blks[source_type] == NULL) {
            source = new_source(source_type);
            break;
        }
    }

    if (source == NULL) {
        return NULL;
    }

    source->param.audio.HW_type     = hardware;
    source->param.audio.channel_sel = channel;
    source->taskId                  = DAV_TASK_ID;
    source->buftype                 = BUFFER_TYPE_INTERLEAVED_BUFFER;

    Audio_Default_setting_init(); // Temp for audio test
    Source_Audio_Get_Default_Parameters(source);

    audio_afe_set_ops(source);
    Source_Audio_HW_Init(source);
#ifndef HAL_AUDIO_ENABLE_PATH_MEM_DEVICE
    if (source->param.audio.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
        source->param.audio.AfeBlkControl.u4asrcflag = true;
    }
#endif
#ifdef AIR_HWSRC_RX_TRACKING_ENABLE
    if (source->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        if (source->param.audio.mem_handle.src_rate != 0) {
            DSP_MW_LOG_I("[HWSRC] enable source hwsrc", 0);
            source->param.audio.mem_handle.pure_agent_with_src = true;
        }
    }
#endif
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
#ifdef AIR_I2S_SLAVE_ENABLE
    if (source->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        source->param.audio.AfeBlkControl.u4asrcflag = true;
    }
#endif
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */
    Source_Audio_Path_Init(source);
    Source_Audio_Afe_Path_Interface_Init(source);
#endif
    return source;
}

#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
/**
 * @brief Request to create an audio tdm-source
 * @param hardware The audio hardware which would be reserved as a source
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Source ID associated with the audio hardware.
 */
SOURCE StreamAudioAfeTdmSource(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    UNUSED(instance);

    // TODO: line in and eSCO input selection
    Source_Audio_SelectInstance(hardware, instance);

    SOURCE source;
    if ((Source_blks[SOURCE_TYPE_TDMAUDIO] != NULL)
        && (Source_blks[SOURCE_TYPE_TDMAUDIO]->param.audio.HW_type == hardware)
        && (Source_blks[SOURCE_TYPE_TDMAUDIO]->param.audio.channel_sel == channel)) {
        return Source_blks[SOURCE_TYPE_TDMAUDIO];
    } else if (Source_blks[SOURCE_TYPE_TDMAUDIO] == NULL) {
        source = new_source(SOURCE_TYPE_TDMAUDIO);
        if (source == NULL) {
            return NULL;
        }
    } else {
        source = Source_blks[SOURCE_TYPE_TDMAUDIO];
    }

    source->param.audio.HW_type     = hardware;
    source->param.audio.channel_sel = channel;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_TASK_ENABLE
    source->taskId                  = DTDM_TASK_ID;
#else
    source->taskId                  = DHP_TASK_ID;
#endif
    source->buftype                 = BUFFER_TYPE_INTERLEAVED_BUFFER;

    Audio_Default_setting_init(); // Temp for audio test
    Source_Audio_Get_Default_Parameters(source);
    audio_afe_set_ops(source);
    Source_Audio_HW_Init(source);
#ifndef HAL_AUDIO_ENABLE_PATH_MEM_DEVICE
    if (source->param.audio.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
        source->param.audio.AfeBlkControl.u4asrcflag = true;
    }
#endif
    Source_Audio_Path_Init(source);
    Source_Audio_Afe_Path_Interface_Init(source);

    return source;
}
#endif


/**
 * @brief Request to create an audio source
 * @param hardware The audio hardware which would be reserved as a source
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Source ID associated with the audio hardware.
 */
SOURCE StreamAudioSource(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    UNUSED(instance);

    Source_Audio_SelectInstance(hardware, instance);

    SOURCE source;
    if ((Source_blks[SOURCE_TYPE_AUDIO] != NULL)
        && (Source_blks[SOURCE_TYPE_AUDIO]->param.audio.HW_type == hardware)
        && (Source_blks[SOURCE_TYPE_AUDIO]->param.audio.channel_sel == channel)) {
        return Source_blks[SOURCE_TYPE_AUDIO];
    } else if (Source_blks[SOURCE_TYPE_AUDIO] == NULL) {
        source = new_source(SOURCE_TYPE_AUDIO);
        if (source == NULL) {
            return NULL;
        }
    } else {
        source = Source_blks[SOURCE_TYPE_AUDIO];
    }

    source->param.audio.HW_type     = hardware;
    source->param.audio.channel_sel = channel;
    source->taskId     = DAV_TASK_ID;

    Audio_Default_setting_init(); // Temp for audio test

    Source_Audio_HW_Init(source);
    Source_Audio_Path_Init(source);
    Source_Audio_Path_Interface_Init(source);

    return source;
}
/**
 * @brief Request to create an audio sink afe
 * @param hardware The audio hardware which would be reserved as a sink
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Sink ID associated with the audio hardware.
 */
SINK StreamAudioAfeSink(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    UNUSED(instance);
    SINK sink;
    if ((Sink_blks[SINK_TYPE_AUDIO] != NULL)
        && (Sink_blks[SINK_TYPE_AUDIO]->param.audio.HW_type == hardware)
        && (Sink_blks[SINK_TYPE_AUDIO]->param.audio.channel_sel == channel)) {
        return Sink_blks[SINK_TYPE_AUDIO];
    } else if (Sink_blks[SINK_TYPE_AUDIO] == NULL) {
        sink = new_sink(SINK_TYPE_AUDIO);

        if (sink == NULL) {
            return NULL;
        }

    } else {
        return NULL;
    }

    sink->param.audio.HW_type       = hardware;
    sink->param.audio.channel_sel   = channel;
    sink->taskid                    = DAV_TASK_ID;
    sink->type                      = SINK_TYPE_AUDIO;
    sink->buftype                   = BUFFER_TYPE_INTERLEAVED_BUFFER;//BUFFER_TYPE_INTERLEAVED_BUFFER; BUFFER_TYPE_CIRCULAR_BUFFER

    Sink_Audio_Get_Default_Parameters(sink);
    Audio_Default_setting_init(); // Temp for audio test

    audio_afe_set_ops(sink);
    Sink_Audio_HW_Init_AFE(sink);

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
    if(sink->param.audio.hwsrc_type != HAL_AUDIO_HWSRC_IN_STREAM){
#ifndef ENABLE_HWSRC_CLKSKEW
    if ((sink->param.audio.rate != sink->param.audio.src_rate) && (sink->param.audio.with_sink_src == false)) //modify for HWSRC power measuring
#else

#ifndef AIR_HWSRC_TX_TRACKING_ENABLE
    if (((sink->param.audio.rate != sink->param.audio.src_rate) && (sink->param.audio.with_sink_src == false)) || (sink->param.audio.clk_skew_mode == CLK_SKEW_V2)) //modify for HWSRC power measuring
#else
    if ((sink->param.audio.with_sink_src == false) || (sink->param.audio.clk_skew_mode == CLK_SKEW_V2))//modify for HWSRC power measuring
#endif

#endif /*ENABLE_HWSRC_CLKSKEW*/
    {
        sink->param.audio.AfeBlkControl.u4asrcflag = true;//Enable DL1 asrc //modify for asrc
        sink->param.audio.AfeBlkControl.u4asrc_buffer_size = ASRC_OUT_BUFFER_SIZE; //modify for asrc
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_DUAL_CHIP_I2S_ENABLE)
        extern bool g_esco_dl_open_flag;

        #ifdef AIR_BT_CODEC_BLE_ENABLED
        extern bool g_n9_ble_dl_open_flag;
        #endif

        if(g_esco_dl_open_flag
            #ifdef AIR_BT_CODEC_BLE_ENABLED
            || g_n9_ble_dl_open_flag
            #endif
        )
        {
            sink->param.audio.AfeBlkControl.u4asrc_buffer_size = 12544;
        }
#endif
#ifdef HAL_AUDIO_ENABLE_PATH_MEM_DEVICE
        sink->param.audio.mem_handle.pure_agent_with_src = true;//modify for ab1568
        DSP_MW_LOG_I("[HWSRC]enable sink hwsrc\r\n", 0);
#endif
    }

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    extern bool anc_leakage_compensation_src_usage_flag;
    anc_leakage_compensation_src_usage_flag = true;
    DSP_MW_LOG_I("[RECORD_LC]anc_leakage_compensation_src_usage_flag = true\r\n", 0);
#endif
    }
#endif /*ENABLE_HWSRC_ON_MAIN_STREAM*/

    Sink_Audio_Path_Init_AFE(sink);
    Sink_Audio_Afe_Path_Interface_Init(sink);

    return sink;
}

#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
/**
 * @brief Request to create an audio tdm-sink
 * @param hardware The audio hardware which would be reserved as a sink
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Sink ID associated with the audio hardware.
 */
SINK StreamAudioAfeTdmSink(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    UNUSED(instance);
    SINK sink;
    if ((Sink_blks[SINK_TYPE_TDMAUDIO] != NULL)
        && (Sink_blks[SINK_TYPE_TDMAUDIO]->param.audio.HW_type == hardware)
        && (Sink_blks[SINK_TYPE_TDMAUDIO]->param.audio.channel_sel == channel)) {
        return Sink_blks[SINK_TYPE_TDMAUDIO];
    } else if (Sink_blks[SINK_TYPE_TDMAUDIO] == NULL) {
        sink = new_sink(SINK_TYPE_TDMAUDIO);

        if (sink == NULL) {
            return NULL;
        }

    } else {
        return NULL;
    }

    sink->param.audio.HW_type       = hardware;
    sink->param.audio.channel_sel   = channel;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_TASK_ENABLE
    sink->taskid                    = DTDM_TASK_ID;
#else
    sink->taskid                    = DHP_TASK_ID;
#endif
    sink->type                      = SINK_TYPE_TDMAUDIO;
    sink->buftype                   = BUFFER_TYPE_INTERLEAVED_BUFFER;//BUFFER_TYPE_INTERLEAVED_BUFFER; BUFFER_TYPE_CIRCULAR_BUFFER

    Sink_Audio_Get_Default_Parameters(sink);
    Audio_Default_setting_init(); // Temp for audio test

    audio_afe_set_ops(sink);
    Sink_Audio_HW_Init_AFE(sink);

    Sink_Audio_Path_Init_AFE(sink);
    Sink_Audio_Afe_Path_Interface_Init(sink);

    return sink;
}
#endif

#ifdef MTK_PROMPT_SOUND_ENABLE
/**
 * @brief Request to create an audio sink afe for VP
 * @param hardware The audio hardware which would be reserved as a sink
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Sink ID associated with the audio hardware.
 */
SINK StreamAudioAfe2Sink(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    UNUSED(instance);
    SINK sink;
    if ((Sink_blks[SINK_TYPE_VP_AUDIO] != NULL)
        && (Sink_blks[SINK_TYPE_VP_AUDIO]->param.audio.HW_type == hardware)
        && (Sink_blks[SINK_TYPE_VP_AUDIO]->param.audio.channel_sel == channel)) {
        return Sink_blks[SINK_TYPE_VP_AUDIO];
    } else if (Sink_blks[SINK_TYPE_VP_AUDIO] == NULL) {
        sink = new_sink(SINK_TYPE_VP_AUDIO);

        if (sink == NULL) {
            return NULL;
        }

    } else {
        return NULL;
    }

    sink->param.audio.HW_type       = hardware;
    sink->param.audio.channel_sel   = channel;
    sink->taskid                    = DAV_TASK_ID; ///TODO: need check task ID later
    sink->type                      = SINK_TYPE_VP_AUDIO;
    sink->buftype                   = BUFFER_TYPE_INTERLEAVED_BUFFER;

    Sink_Audio_Get_Default_Parameters(sink);
    Audio_Default_setting_init(); // Temp for audio test

    //audio_afe2_set_sink_ops(hal_audio_get_stream_out_device());
    audio_afe_set_ops(sink);

    Sink_Audio_HW_Init_AFE(sink);
#ifdef AIR_VP_NOT_USE_HWSRC_DEFAULT_ENABLE
    if ((n9_sco_dl_if.sink) && (sink->param.audio.with_sink_src == false)) { // If phone call exists, VP should use HWSRC.
#else
    if (sink->param.audio.with_sink_src == false) {
#endif
#if !defined AIR_I2S_SLAVE_ENABLE
        DSP_MW_LOG_I("StreamAudioAfe2Sink, VP uses HWSRC, %d != %d\r\n", 2, sink->param.audio.rate, sink->param.audio.src_rate);
        sink->param.audio.AfeBlkControl.u4asrcflag = true;//Enable DL2 asrc
#ifdef HAL_AUDIO_ENABLE_PATH_MEM_DEVICE
        sink->param.audio.mem_handle.pure_agent_with_src = true;//modify for ab1568
        if (sink->param.audio.device_handle.common.audio_device & HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL) {
            sink->param.audio.device_handle.dac.with_force_change_rate = false;
        }
#endif
#endif //#ifndef AIR_HWSRC_RX_TRACKING_ENABLE
    }

#if defined(ENABLE_HWSRC_ON_MAIN_STREAM) || defined(MTK_HWSRC_IN_STREAM)
    sink->param.audio.AfeBlkControl.u4asrc_buffer_size = ASRC_OUT_BUFFER_SIZE;
#endif

    Sink_Audio_Path_Init_AFE(sink);
    Sink_Audio_Afe_Path_Interface_Init(sink);

    DSP_MW_LOG_I("StreamAudioAfe2Sink done\r\n", 0);

    return sink;
}
#endif

#if defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined (AIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE) || defined(AIR_HFP_DNN_PATH_ENABLE) || defined(AIR_DCHS_MODE_ENABLE) || defined(AIR_WIRELESS_MIC_RX_ENABLE) || defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
/**
 * @brief Request to create an audio sink afe for advance passthru
 * @param hardware The audio hardware which would be reserved as a sink
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Sink ID associated with the audio hardware.
 */

SINK StreamAudioAfe3Sink(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    UNUSED(instance);
    SINK sink;
    if ((Sink_blks[SINK_TYPE_AUDIO_DL3] != NULL)
        && (Sink_blks[SINK_TYPE_AUDIO_DL3]->param.audio.HW_type == hardware)
        && (Sink_blks[SINK_TYPE_AUDIO_DL3]->param.audio.channel_sel == channel)) {
        return Sink_blks[SINK_TYPE_AUDIO_DL3];
    } else if (Sink_blks[SINK_TYPE_AUDIO_DL3] == NULL) {
        sink = new_sink(SINK_TYPE_AUDIO_DL3);

        if (sink == NULL) {
            return NULL;
        }

    } else {
        return NULL;
    }

    sink->param.audio.HW_type       = hardware;
    sink->param.audio.channel_sel   = channel;
    sink->taskid                    = DHP_TASK_ID;
    sink->type                      = SINK_TYPE_AUDIO_DL3;
    sink->buftype                   = BUFFER_TYPE_INTERLEAVED_BUFFER;

    Sink_Audio_Get_Default_Parameters(sink);
    Audio_Default_setting_init(); // Temp for audio test

    audio_afe_set_ops(sink);
    Sink_Audio_HW_Init_AFE(sink);


    Sink_Audio_Path_Init_AFE(sink);
    Sink_Audio_Afe_Path_Interface_Init(sink);

    return sink;
}
#endif

#if defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined (AIR_MIXER_STREAM_ENABLE)
/**
 * @brief Request to create an audio sink afe for DL12
 * @param hardware The audio hardware which would be reserved as a sink
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Sink ID associated with the audio hardware.
 */

SINK StreamAudioAfe12Sink(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    UNUSED(instance);
    SINK sink;
    if ((Sink_blks[SINK_TYPE_AUDIO_DL12] != NULL)
        && (Sink_blks[SINK_TYPE_AUDIO_DL12]->param.audio.HW_type == hardware)
        && (Sink_blks[SINK_TYPE_AUDIO_DL12]->param.audio.channel_sel == channel)) {
        return Sink_blks[SINK_TYPE_AUDIO_DL12];
    } else if (Sink_blks[SINK_TYPE_AUDIO_DL12] == NULL) {
        sink = new_sink(SINK_TYPE_AUDIO_DL12);

        if (sink == NULL) {
            return NULL;
        }

    } else {
        return NULL;
    }

    sink->param.audio.HW_type       = hardware;
    sink->param.audio.channel_sel   = channel;
    sink->taskid                    = DAV_TASK_ID;
    #ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
        sink->taskid                = DDCHS_TASK_ID;
    }
    #endif
    #ifdef AIR_MIXER_STREAM_ENABLE
    sink->taskid                    = DDCHS_TASK_ID;
    #endif
    sink->type                      = SINK_TYPE_AUDIO_DL12;
    sink->buftype                   = BUFFER_TYPE_INTERLEAVED_BUFFER;

    Sink_Audio_Get_Default_Parameters(sink);
    Audio_Default_setting_init(); // Temp for audio test

    audio_afe_set_ops(sink);
    Sink_Audio_HW_Init_AFE(sink);

    Sink_Audio_Path_Init_AFE(sink);
    Sink_Audio_Afe_Path_Interface_Init(sink);

    return sink;
}
#endif /* AIR_ADVANCED_PASSTHROUGH_ENABLE */


/**
 * @brief Request to create an audio sink
 * @param hardware The audio hardware which would be reserved as a sink
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Sink ID associated with the audio hardware.
 */
SINK StreamAudioSink(audio_hardware hardware, audio_instance instance, audio_channel channel)
{
    UNUSED(instance);
    SINK sink;
    if ((Sink_blks[SINK_TYPE_AUDIO] != NULL)
        && (Sink_blks[SINK_TYPE_AUDIO]->param.audio.HW_type == hardware)
        && (Sink_blks[SINK_TYPE_AUDIO]->param.audio.channel_sel == channel)) {
        return Sink_blks[SINK_TYPE_AUDIO];
    } else if (Sink_blks[SINK_TYPE_AUDIO] == NULL) {
        sink = new_sink(SINK_TYPE_AUDIO);

        if (sink == NULL) {
            return NULL;
        }

    } else {
        return NULL;
    }

    sink->param.audio.HW_type       = hardware;
    sink->param.audio.channel_sel   = channel;
    sink->taskid                    = DAV_TASK_ID;

    Audio_Default_setting_init(); // Temp for audio test

    Sink_Audio_HW_Init(sink);
    Sink_Audio_Path_Init(sink);
    Sink_Audio_Path_Interface_Init(sink);

    return sink;
}
/**
 * @brief Request to create an audio aux path sink
 * @this sink is an auxiliary path for StreamAudioSink which shared the same audio output
 * @return The Sink ID associated with the audio hardware.
 */
SINK StreamVPPathSink(audio_hardware hardware, audio_instance instance)
{
    UNUSED(instance);
    SINK sink;
    if ((Sink_blks[SINK_TYPE_VP_AUDIO] != NULL)
        && (Sink_blks[SINK_TYPE_VP_AUDIO]->param.audio.HW_type == hardware)) {
        return Sink_blks[SINK_TYPE_VP_AUDIO];
    } else if (Sink_blks[SINK_TYPE_VP_AUDIO] == NULL) {
        sink = new_sink(SINK_TYPE_VP_AUDIO);

        if (sink == NULL) {
            return NULL;
        }
    } else {
        sink = Sink_blks[SINK_TYPE_VP_AUDIO];
    }
    sink->param.audio.HW_type     = hardware;
    sink->param.audio.channel_sel = AUDIO_CHANNEL_VP;
    sink->taskid                  = DPR_TASK_ID;

    Audio_Default_setting_init(); // Temp for audio test

    if (Audio_Status.audio_in_use == 0) {
        Sink_Audio_HW_Init(sink);
    }
    Sink_Audio_SubPath_Init(sink);
    Sink_Audio_Path_Interface_Init(sink);

    return sink;
}


/**
 * @brief Request to create a sink which is joint to transform
 * @param transform The audio transform to connect
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Sink ID associated with the audio hardware.
 */
SINK StreamJointSink(TRANSFORM transform, audio_channel channel)
{
    SINK sink;
    if ((Sink_blks[SINK_TYPE_DSP_JOINT_0] != NULL)
        && (Sink_blks[SINK_TYPE_DSP_JOINT_0]->param.dsp.transform == transform)
        && (Sink_blks[SINK_TYPE_DSP_JOINT_0]->param.audio.channel_sel == channel)) {
        return Sink_blks[SINK_TYPE_DSP_JOINT_0];
    } else if ((Sink_blks[SINK_TYPE_DSP_JOINT_1] != NULL)
               && (Sink_blks[SINK_TYPE_DSP_JOINT_1]->param.dsp.transform == transform)
               && (Sink_blks[SINK_TYPE_DSP_JOINT_1]->param.audio.channel_sel == channel)) {
        return Sink_blks[SINK_TYPE_DSP_JOINT_1];
    } else if (Sink_blks[SINK_TYPE_DSP_JOINT_0] == NULL) {
        sink = new_sink(SINK_TYPE_DSP_JOINT_0);

        if (sink == NULL) {
            return NULL;
        }
    } else if (Sink_blks[SINK_TYPE_DSP_JOINT_1] == NULL) {
        sink = new_sink(SINK_TYPE_DSP_JOINT_1);

        if (sink == NULL) {
            return NULL;
        }
    } else {
        sink = Sink_blks[SINK_TYPE_DSP_JOINT_0];
    }


    sink->param.dsp.channel_sel = channel;
    sink->param.dsp.frame_size  = transform->sink->param.audio.frame_size;
    sink->param.dsp.channel_num = transform->sink->param.audio.channel_num;
    sink->taskid                = DAV_TASK_ID;
    sink->type                  = SINK_TYPE_DSP_JOINT;
    sink->buftype               = BUFFER_TYPE_CIRCULAR_BUFFER;
    sink->transform             = NULL;

    Sink_Audio_Buffer_Ctrl(sink, TRUE);
    Sink_Audio_Path_Interface_Init(sink);
    sink->param.dsp.transform     = transform;
    return sink;
}
#if ViturlStreamEnable
/**
 * @brief Request to create a virtual sink which can report result
 * @param entry The function entry which  will be call when Flush sink
 * @param report_length The length of report
 *
 * @return The Sink ID associated with the audio hardware.
 */
SOURCE StreamVirtualSource(VOID *entry, VIRTUAL_SOURCE_TYPE type, U32 report_length, U8 channel_num, U32 data_size, U32 samplingrate)
{
    SOURCE source = NULL;
    U8 *mem_ptr;
    SOURCE_TYPE source_type;
    for (source_type = SOURCE_TYPE_DSP_VIRTUAL_MIN; source_type <= SOURCE_TYPE_DSP_VIRTUAL_MAX ; source_type++) {
        if (Source_blks[source_type] == NULL) {
            source = new_source(source_type);
            break;
        }
    }
    if (source == NULL) {
        return NULL;
    }

    source->taskId = DAV_TASK_ID;
    source->buftype               = BUFFER_TYPE_CIRCULAR_BUFFER;
    source->type                  = source_type;
    source->transform             = NULL;
    source->param.virtual_para.entry     = entry;
    source->param.virtual_para.mem_size  = report_length;
    source->param.virtual_para.data_size = data_size;
    source->param.virtual_para.default_data_size = data_size;
    source->param.virtual_para.data_samplingrate = samplingrate;
    source->param.virtual_para.default_data_samplingrate = samplingrate;
    source->param.virtual_para.channel_num = channel_num;
    source->param.virtual_para.is_processed = true; //not process at first.
    source->param.virtual_para.is_dummy_data = true; //no data in comming at first.
    source->param.virtual_para.virtual_source_type = type;
    // source->param.virtual_para.user_count = 1;
    //TODO if use no-zero data need malloc buffer and set data.
    if (report_length && (type != VIRTUAL_SOURCE_ZERO_DATA)) {
        for (U8 i = 0; i < channel_num; i++)
        {
            mem_ptr = pvPortMalloc(report_length);
            if (mem_ptr != NULL) {
                memset(mem_ptr, 0, report_length);
                source->streamBuffer.BufferInfo.startaddr[i] = mem_ptr;
            }
        }
    }
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    Source_VirtualSource_Interface_Init(source);
#else
    DSP_MW_LOG_E("DSP stream VirtualSource err, not enabel AIR_AUDIO_HARDWARE_ENABLE",0);
    AUDIO_ASSERT(0);
#endif
    DSP_MW_LOG_I("DSP stream VirtualSource user_count:%d, channel_num:%d, mem_size:%d", 3, source->param.virtual_para.user_count,source->param.virtual_para.channel_num,source->param.virtual_para.mem_size);
    return source;
}

SINK StreamVirtualSink(VOID *entry, U32 report_length)
{
    SINK sink;
    U8 *mem_ptr;

    if (Sink_blks[SINK_TYPE_DSP_VIRTUAL] == NULL) {
        sink = new_sink(SINK_TYPE_DSP_VIRTUAL);

    } else {
        sink = Sink_blks[SINK_TYPE_DSP_VIRTUAL];
        sink->param.virtual_para.user_count++;
        DSP_MW_LOG_I("DSP stream VirtualSink :%d", 1, sink->param.virtual_para.user_count);
        return sink;
    }

    if (sink == NULL) {
        return NULL;
    }

    sink->taskid                  = DAV_TASK_ID;
    sink->buftype                 = BUFFER_TYPE_CIRCULAR_BUFFER;
    sink->transform               = NULL;
    sink->param.virtual_para.entry     = entry;
    sink->param.virtual_para.mem_size  = report_length;
    sink->param.virtual_para.user_count = 1;

    if (report_length) {
        mem_ptr = pvPortMalloc(report_length);
        if (mem_ptr != NULL) {
            memset(mem_ptr, 0, report_length);
            sink->streamBuffer.BufferInfo.startaddr[0] = mem_ptr;
        }
    }
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    Sink_VirtualSink_Interface_Init(sink);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
    return sink;
}
#endif
/**
 * @brief Request to create a source which is branched from transform
 * @param transform The audio transform to connect
 * @param instance The audio hardware instance (meaning depends on \e hardware)
 * @param channel The audio channel (meaning depends on \e hardware)
 *
 * @return The Source ID associated with the audio hardware.
 */
SOURCE StreamBranchSource(TRANSFORM transform, audio_channel channel)
{
    SOURCE source;
    if ((Source_blks[SOURCE_TYPE_DSP_BRANCH_0] != NULL)
        && (Source_blks[SOURCE_TYPE_DSP_BRANCH_0]->param.dsp.transform == transform)
        && (Source_blks[SOURCE_TYPE_DSP_BRANCH_0]->param.audio.channel_sel == channel)) {
        return Source_blks[SOURCE_TYPE_DSP_BRANCH_0];
    } else if ((Source_blks[SOURCE_TYPE_DSP_BRANCH_1] != NULL)
               && (Source_blks[SOURCE_TYPE_DSP_BRANCH_1]->param.dsp.transform == transform)
               && (Source_blks[SOURCE_TYPE_DSP_BRANCH_1]->param.audio.channel_sel == channel)) {
        return Source_blks[SOURCE_TYPE_DSP_BRANCH_1];
    } else if (Source_blks[SOURCE_TYPE_DSP_BRANCH_0] == NULL) {
        source = new_source(SOURCE_TYPE_DSP_BRANCH_0);
        if (source == NULL) {
            return NULL;
        }
    } else if (Source_blks[SOURCE_TYPE_DSP_BRANCH_1] == NULL) {
        source = new_source(SOURCE_TYPE_DSP_BRANCH_1);
        if (source == NULL) {
            return NULL;
        }
    } else {
        source = Source_blks[SOURCE_TYPE_DSP_BRANCH_0];
    }


    source->param.dsp.channel_sel = channel;
    source->param.dsp.frame_size  = transform->sink->param.audio.frame_size;
    source->param.dsp.channel_num = transform->sink->param.audio.channel_num;
    source->taskId                = DAV_TASK_ID;//DSP_CallbackTask_Get(transform->source, transform->sink);//DAV_TASK_ID;
    source->buftype               = BUFFER_TYPE_CIRCULAR_BUFFER;
    source->type                  = SOURCE_TYPE_DSP_BRANCH;
    source->transform             = NULL;

    Source_Audio_Buffer_Ctrl(source, TRUE);
    Source_Audio_Path_Interface_Init(source);
    source->param.dsp.transform   = transform;
    return source;
}




/**
 * @breif init stream system
 */
VOID Stream_Init(VOID)
{
    TransformList_Init();
}

VOID StreamTransformClose(VOID *pTransform)
{
    TRANSFORM transform = (TRANSFORM)pTransform;
    dlist_remove(&transform->list);
    transform->source->transform = NULL;
    transform->sink->transform = NULL;
    vPortFree(transform);
}

#ifdef AIR_BT_HFP_ENABLE
SINK StreamN9ScoSink(void *param)
{
    SINK sink;
    if (Sink_blks[SINK_TYPE_N9SCO] != NULL) {
        return Sink_blks[SINK_TYPE_N9SCO];
    }
    sink = new_sink(SINK_TYPE_N9SCO);
    if (sink == NULL) {
        return NULL;
    }
    sink->taskid = DAV_TASK_ID;
    sink->param.n9sco.share_info_base_addr = param;

    SinkInitN9Sco(sink);

    return sink;
}


SOURCE StreamN9ScoSource(void *param)
{
    SOURCE source;

    if (Source_blks[SOURCE_TYPE_N9SCO] != NULL) {
        return Source_blks[SOURCE_TYPE_N9SCO];
    }

    source = new_source(SOURCE_TYPE_N9SCO);
    if (source == NULL) {
        return NULL;
    }
    source->taskId = DAV_TASK_ID;
    source->param.n9sco.share_info_base_addr = param;

    SourceInitN9Sco(source);

    return source;
}
#endif

#ifdef AIR_BT_CODEC_BLE_ENABLED

SINK StreamN9BleSink(void *param)
{
    SINK sink;
    mcu2dsp_open_param_p open_param;

    if (Sink_blks[SINK_TYPE_N9BLE] != NULL) {
        return Sink_blks[SINK_TYPE_N9BLE];
    }

    sink = new_sink(SINK_TYPE_N9BLE);
    if (sink == NULL) {
        return NULL;
    }


    open_param = (mcu2dsp_open_param_p)param;
    sink->taskid = DAV_TASK_ID;
    sink->param.n9ble.share_info_base_addr = open_param->stream_out_param.ble.p_share_info;
    if (ULL_NrOffloadFlag == true) {
        sink->param.n9ble.frame_length = open_param->stream_out_param.ble.frame_payload_length + 1;
    } else {
    sink->param.n9ble.frame_length = open_param->stream_out_param.ble.frame_payload_length;
    }
    g_ble_ul_abr_length = sink->param.n9ble.frame_length;
    sink->param.n9ble.process_number = 0;
    sink->param.n9ble.dummy_insert_number = 0;
    sink->param.n9ble.dummy_process_status = 0;
    sink->param.n9ble.context_type = open_param->stream_out_param.ble.context_type;
    sink->param.n9ble.sampling_rate = open_param->stream_out_param.ble.sampling_frequency;
    sink->param.n9ble.share_info_base_addr->sampling_rate =  open_param->stream_out_param.ble.sampling_frequency;
    sink->param.n9ble.frame_interval = open_param->stream_out_param.ble.frame_ms;
    sink->param.n9ble.codec_type = open_param->stream_out_param.ble.codec_type;
    sink->param.n9ble.crc_init = sink->param.n9ble.share_info_base_addr->asi_cur;//CRC init value saved in asi_current
    sink->param.n9ble.is_lightmode = open_param->stream_out_param.ble.is_lightmode;
    SinkInitN9Ble(sink);

    DSP_MW_LOG_I("ULL_NrOffloadFlag %d, frame_payload_length %d, StreamN9BleSink   frame_length:%d   process_number:%d   frame_interval:%d crc_init:%d  type:%d", 7,
                 ULL_NrOffloadFlag, open_param->stream_out_param.ble.frame_payload_length, sink->param.n9ble.frame_length, sink->param.n9ble.process_number, sink->param.n9ble.frame_interval,sink->param.n9ble.crc_init,sink->param.n9ble.context_type);

    sink->param.n9ble.IsPlayInfoReceived = FALSE;
    /*config LC3 init param*/

    if (open_param->stream_out_param.ble.codec_type == BT_BLE_CODEC_LC3)
    {
        #ifdef AIR_BT_LE_LC3_ENABLE
        LC3I_Set_Enc_Param(1, sink->param.n9ble.frame_interval / 100, sink->param.n9ble.sampling_rate, (sink->param.n9ble.frame_length * 8) * (1000 * 10) / (sink->param.n9ble.frame_interval / 100));
        #endif

    }
    #ifdef AIR_BT_LE_LC3PLUS_ENABLE
    else if (open_param->stream_out_param.ble.codec_type == BT_BLE_CODEC_LC3PLUS)
    {
        /* codec init */
        lc3plus_enc_config_t lc3plus_enc_config;
        //ull2.0
        if (sink->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE) {
        lc3plus_enc_config.sample_bits      = 16; // May be different for wireless mic
#ifdef AIR_BT_ULL_FB_ENABLE
        if (open_param->stream_out_param.ble.sampling_frequency == 48000) {
            lc3plus_enc_config.sample_bits      = 24; // Fullband only support 48k 24bit
        }
#endif
        lc3plus_enc_config.channel_num      = 1;  // May be different for wireless mic
        lc3plus_enc_config.sample_rate      = open_param->stream_out_param.ble.sampling_frequency;
        lc3plus_enc_config.bit_rate         = 8*((U32)open_param->stream_out_param.ble.frame_payload_length)*(1000000/(open_param->stream_out_param.ble.frame_ms));
        lc3plus_enc_config.frame_interval   = open_param->stream_out_param.ble.frame_ms;
        lc3plus_enc_config.frame_samples    = open_param->stream_out_param.ble.sampling_frequency/1000 * open_param->stream_out_param.ble.frame_ms/1000;
        lc3plus_enc_config.out_frame_size   = open_param->stream_out_param.ble.frame_payload_length;
        lc3plus_enc_config.channel_mode     = LC3PLUS_ENC_CHANNEL_MODE_MONO;
        lc3plus_enc_config.codec_mode       = LC3PLUS_ARCH_FX;
        lc3plus_enc_config.in_frame_size    = sizeof(U16) * lc3plus_enc_config.frame_samples;   // May be different for wireless mic
#ifdef AIR_BT_ULL_FB_ENABLE
        if (open_param->stream_out_param.ble.sampling_frequency == 48000) {
            lc3plus_enc_config.in_frame_size    = sizeof(U32) * lc3plus_enc_config.frame_samples;   // May be different for wireless mic
        }
#endif
        lc3plus_enc_config.process_frame_num   = (ULL_NrOffloadFlag) ? (5 * 1000 / open_param->stream_out_param.ble.frame_ms) : (15 * 1000 / open_param->stream_out_param.ble.frame_ms);// May be different for wireless mic
#ifdef AIR_BT_ULL_FB_ENABLE
        if (open_param->stream_out_param.ble.sampling_frequency == 48000) {
            lc3plus_enc_config.process_frame_num = (open_param->stream_in_param.afe.irq_period == 5) ? 1 : 3; // Config to 5ms frame interval for FB case
        }
#endif
        }
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
        //wireless mic
        else if (sink->param.n9ble.context_type == BLE_CONTENT_TYPE_WIRELESS_MIC) {
            lc3plus_enc_config.sample_bits      = 24;
            lc3plus_enc_config.channel_num      = open_param->stream_out_param.ble.channel_num;
            lc3plus_enc_config.sample_rate      = open_param->stream_out_param.ble.sampling_frequency;
            lc3plus_enc_config.bit_rate         = 8*((U32)open_param->stream_out_param.ble.frame_payload_length)*(1000000/(open_param->stream_out_param.ble.frame_ms));
            lc3plus_enc_config.frame_interval   = open_param->stream_out_param.ble.frame_ms;
            lc3plus_enc_config.frame_samples    = open_param->stream_out_param.ble.sampling_frequency/1000 * open_param->stream_out_param.ble.frame_ms/1000;
            lc3plus_enc_config.out_frame_size   = open_param->stream_out_param.ble.frame_payload_length;
            if(lc3plus_enc_config.channel_num == 1)
                lc3plus_enc_config.channel_mode     = LC3PLUS_ENC_CHANNEL_MODE_MONO;
            else if(lc3plus_enc_config.channel_num == 2)
                lc3plus_enc_config.channel_mode     = LC3PLUS_ENC_CHANNEL_MODE_STEREO;
            lc3plus_enc_config.codec_mode       = LC3PLUS_ARCH_FX;
            lc3plus_enc_config.in_frame_size    = sizeof(U32) * lc3plus_enc_config.frame_samples;
            lc3plus_enc_config.process_frame_num   = 1;
        }
#endif
        else{
            DSP_MW_LOG_E("[BLE][LC3PLUS_ENC]  context_type = %d is invalid", 1, sink->param.n9ble.context_type);
            AUDIO_ASSERT(0);
        }
        stream_codec_encoder_lc3plus_init(LC3PLUS_ENC_PORT_0, sink, &lc3plus_enc_config);
        DSP_MW_LOG_I("[BLE][LC3PLUS_ENC] init %u, %u, %u, %u, %u, %u, %u, %u, %u  \r\n", 9,
            lc3plus_enc_config.sample_bits,
            lc3plus_enc_config.sample_rate,
            lc3plus_enc_config.bit_rate,
            lc3plus_enc_config.channel_num,
            lc3plus_enc_config.frame_interval,
            lc3plus_enc_config.out_frame_size,
            lc3plus_enc_config.frame_samples,
            lc3plus_enc_config.in_frame_size,
            lc3plus_enc_config.process_frame_num);
        sink->param.n9ble.process_number = lc3plus_enc_config.process_frame_num;
    }
    #endif
    #ifdef AIR_CELT_ENC_V2_ENABLE
    else if (open_param->stream_out_param.ble.codec_type == BT_BLE_CODEC_VENDOR)
    {
        /* codec init */
        celt_enc_port_config_t celt_enc_config;
        //ull2.0
        if (sink->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE) {
            celt_enc_config.sample_bits      = 16;
            celt_enc_config.channel_num      = 1;
            celt_enc_config.sample_rate      = open_param->stream_out_param.ble.sampling_frequency;
            celt_enc_config.bit_rate         = 8*((U32)open_param->stream_out_param.ble.frame_payload_length)*(1000000/(open_param->stream_out_param.ble.frame_ms));
            celt_enc_config.frame_interval   = open_param->stream_out_param.ble.frame_ms;
            celt_enc_config.frame_samples    = open_param->stream_out_param.ble.sampling_frequency/1000 * open_param->stream_out_param.ble.frame_ms/1000;
            celt_enc_config.out_frame_size   = open_param->stream_out_param.ble.frame_payload_length;
            celt_enc_config.channel_mode     = CELT_ENC_CHANNEL_MODE_MONO;
            celt_enc_config.in_frame_size    = sizeof(U16) * celt_enc_config.frame_samples;
            celt_enc_config.process_frame_num   = 15 * 1000 / open_param->stream_out_param.ble.frame_ms;
        }
        else{
            DSP_MW_LOG_E("[BLE][CELT_ENC]  context_type = %d is invalid", 1, sink->param.n9ble.context_type);
            AUDIO_ASSERT(0);
        }
        stream_codec_encoder_celt_v2_init(CELT_ENC_PORT_0, sink, &celt_enc_config);
        DSP_MW_LOG_I("[BLE][CELT_ENC] init %u, %u, %u, %u, %u, %u, %u, %u, %u  \r\n", 9,
            celt_enc_config.sample_bits,
            celt_enc_config.sample_rate,
            celt_enc_config.bit_rate,
            celt_enc_config.channel_num,
            celt_enc_config.frame_interval,
            celt_enc_config.out_frame_size,
            celt_enc_config.frame_samples,
            celt_enc_config.in_frame_size,
            celt_enc_config.process_frame_num);
        sink->param.n9ble.process_number = celt_enc_config.process_frame_num;

    }
    #endif /* AIR_CELT_ENC_V2_ENABLE */
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    else if (open_param->stream_out_param.ble.codec_type == BT_BLE_CODEC_ULD)
    {
        sink->param.n9ble.process_number = 1;
        DSP_MW_LOG_E("[BLE][ULD_ENC]  process_number =%d", 1, sink->param.n9ble.process_number);
    }
#endif
    return sink;
}


SOURCE StreamN9BleSource(void *param)
{
    SOURCE source;
    mcu2dsp_open_param_p open_param;

    if (Source_blks[SOURCE_TYPE_N9BLE] != NULL) {
        return Source_blks[SOURCE_TYPE_N9BLE];
    }

    source = new_source(SOURCE_TYPE_N9BLE);
    if (source == NULL) {
        return NULL;
    }
    open_param = (mcu2dsp_open_param_p)param;
    source->param.n9ble.frame_interval = open_param->stream_in_param.ble.frame_ms;

    source->taskId = DAV_TASK_ID;
    source->param.n9ble.share_info_base_addr = open_param->stream_in_param.ble.p_share_info;
    source->param.n9ble.frame_length = open_param->stream_in_param.ble.frame_payload_length;
    g_ble_abr_length = open_param->stream_in_param.ble.frame_payload_length;

    DSP_MW_LOG_I("[BLE] StreamN9BleSource g_ble_abr_length %d",1,g_ble_abr_length);
    source->param.n9ble.process_number = 1;//(open_param->stream_out_param.afe.irq_period * 1000) / open_param->stream_in_param.ble.frame_ms;
    //force process_number to be 1 since irq_period would be 0 because 7.5 ms can't be represented hence this parameter is abandoned
    source->param.n9ble.context_type = open_param->stream_in_param.ble.context_type;
    source->param.n9ble.sampling_rate = open_param->stream_in_param.ble.sampling_frequency;
    source->param.n9ble.share_info_base_addr->sampling_rate = open_param->stream_in_param.ble.sampling_frequency;
    source->param.n9ble.dl_afe_skip_time = 0;
    source->param.n9ble.skip_after_drop = 0;
    AudioAfeConfiguration(AUDIO_SINK_IRQ_RATE, source->param.n9ble.sampling_rate);
    source->param.n9ble.plc_state_len = 0;
    source->param.n9ble.codec_type = open_param->stream_in_param.ble.codec_type;
    source->param.n9ble.is_lightmode = open_param->stream_in_param.ble.is_lightmode;

    source->param.n9ble.IsPlayInfoReceived = FALSE;
    if (open_param->stream_in_param.ble.dual_cis_mode == DUAL_CIS_MODE) {
        DSP_MW_LOG_I("[BLE] sub share info add: 0x%x\r\n", 1, open_param->stream_in_param.ble.p_sub_share_info);
        source->param.n9ble.sub_share_info_base_addr = (n9_dsp_share_info_ptr)hal_memview_cm4_to_dsp0((U32)open_param->stream_in_param.ble.p_sub_share_info);
        DSP_MW_LOG_I("[BLE] share buffer startaddr: 0x%x\r\n", 1, source->param.n9ble.sub_share_info_base_addr);
        source->param.n9ble.dual_cis_status = DUAL_CIS_WAITING_MAIN;
        source->param.n9ble.dual_cis_sub_channel_sel = SUB_CIS_CHANNEL_R;
        source->param.n9ble.dual_cis_buffer_offset = 0;
        source->param.n9ble.IsSubPlayInfoReceived = FALSE;
    } else {
        source->param.n9ble.dual_cis_status = DUAL_CIS_DISABLED;
        source->param.n9ble.IsSubPlayInfoReceived = TRUE;
    }
    if (open_param->stream_in_param.ble.codec_type == BT_BLE_CODEC_LC3)
    {
        #ifdef AIR_BT_LE_LC3_ENABLE
        extern uint32_t g_plc_mode;
        #if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
        LC3I_PLC_MODE_T plc_mode = (g_plc_mode == 0)? LC3I_PLC_STANDARD : LC3I_PLC_ADVANCED;
        #else
        LC3I_PLC_MODE_T plc_mode = (g_plc_mode == 0)? LC3I_PLC_STANDARD : LC3I_TPLC;
        #endif
#ifdef AIR_BT_BLE_FIX_ADV_PLC
        plc_mode = 1;
#endif
        if (open_param->stream_in_param.ble.context_type == BLE_CONTEXT_CONVERSATIONAL)
        {
            LC3I_Set_Dec_Param(1, source->param.n9ble.frame_interval / 100, plc_mode, source->param.n9ble.sampling_rate, (source->param.n9ble.frame_length * 8) * (1000 * 10) / (source->param.n9ble.frame_interval / 100));
        } else {
            LC3I_Set_Dec_Param(open_param->stream_in_param.ble.channel_num, source->param.n9ble.frame_interval / 100, plc_mode, source->param.n9ble.sampling_rate, (source->param.n9ble.frame_length * 8) * (1000 * 10) / (source->param.n9ble.frame_interval / 100));
        }

        #endif
    }
    #ifdef AIR_BT_LE_LC3PLUS_ENABLE
    else if (open_param->stream_in_param.ble.codec_type == BT_BLE_CODEC_LC3PLUS)
    {
           extern uint32_t g_plc_mode;
        /* init lc3plus codec */
           lc3plus_dec_port_config_t lc3plus_dec_config;
           lc3plus_dec_config.sample_bits      = 32;
           lc3plus_dec_config.sample_rate      = open_param->stream_in_param.ble.sampling_frequency;
           lc3plus_dec_config.bit_rate         = open_param->stream_in_param.ble.channel_num*8*((U32)open_param->stream_in_param.ble.frame_payload_length)*(1000000/(open_param->stream_in_param.ble.frame_ms));//open_param->stream_in_param.ble.;
           lc3plus_dec_config.in_channel_num   = 1;
           lc3plus_dec_config.out_channel_num  = 2;
           lc3plus_dec_config.channel_mode     = (open_param->stream_in_param.ble.channel_num == 1) ? LC3PLUS_DEC_CHANNEL_MODE_MONO : LC3PLUS_DEC_CHANNEL_MODE_STEREO;
           lc3plus_dec_config.frame_interval   = open_param->stream_in_param.ble.frame_ms;
           lc3plus_dec_config.frame_size       = open_param->stream_in_param.ble.frame_payload_length*open_param->stream_in_param.ble.channel_num;
           lc3plus_dec_config.frame_samples    = open_param->stream_in_param.ble.sampling_frequency/1000 * open_param->stream_in_param.ble.frame_ms/1000;
           lc3plus_dec_config.plc_enable       = 1;
           lc3plus_dec_config.plc_method       = (g_plc_mode == 0)? LC3PLUS_PLCMETH_STD : LC3PLUS_PLCMETH_ADV_TDC_NS;
           stream_codec_decoder_lc3plus_init(LC3PLUS_DEC_PORT_0, source, &lc3plus_dec_config);
           DSP_MW_LOG_I("[BLE][LC3PLUS_DEC] init %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                       lc3plus_dec_config.sample_bits,
                       lc3plus_dec_config.sample_rate,
                       lc3plus_dec_config.bit_rate,
                       lc3plus_dec_config.channel_mode,
                       lc3plus_dec_config.in_channel_num,
                       lc3plus_dec_config.out_channel_num,
                       lc3plus_dec_config.frame_interval,
                       lc3plus_dec_config.frame_size,
                       lc3plus_dec_config.frame_samples,
                       lc3plus_dec_config.plc_enable);
    }
    #endif

    #ifdef AIR_CELT_DEC_V2_ENABLE
    else if (open_param->stream_in_param.ble.codec_type == BT_BLE_CODEC_VENDOR)
    {
        uint16_t process_frame_num = 2;

        /* init celt codec */
        celt_dec_port_config_t celt_dec_config;
        celt_dec_config.sample_bits            = 16;
        celt_dec_config.sample_rate            = open_param->stream_in_param.ble.sampling_frequency;
        celt_dec_config.bit_rate               = open_param->stream_in_param.ble.channel_num*8*((U32)open_param->stream_in_param.ble.frame_payload_length)*(1000000/(open_param->stream_in_param.ble.frame_ms));
        celt_dec_config.in_channel_num         = (open_param->stream_in_param.ble.channel_num == 1) ? 1 : 2;
        celt_dec_config.out_channel_num        = 2;
        celt_dec_config.channel_mode           = (open_param->stream_in_param.ble.channel_num == 1) ? CELT_DEC_CHANNEL_MODE_MONO : CELT_DEC_CHANNEL_MODE_MULTI_MONO;
        celt_dec_config.frame_interval         = open_param->stream_in_param.ble.frame_ms/process_frame_num;
        celt_dec_config.frame_size             = open_param->stream_in_param.ble.frame_payload_length/ process_frame_num;
        celt_dec_config.frame_samples          = (open_param->stream_in_param.ble.sampling_frequency/1000 * open_param->stream_in_param.ble.frame_ms/1000)/process_frame_num;
        celt_dec_config.plc_enable             = 1;

        stream_codec_decoder_celt_v2_init(CELT_DEC_PORT_0, source, &celt_dec_config);
        DSP_MW_LOG_I("[BLE][CELT_DEC] init %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                celt_dec_config.sample_bits,
                celt_dec_config.sample_rate,
                celt_dec_config.bit_rate,
                celt_dec_config.channel_mode,
                celt_dec_config.in_channel_num,
                celt_dec_config.out_channel_num,
                celt_dec_config.frame_interval,
                celt_dec_config.frame_size,
                celt_dec_config.frame_samples,
                celt_dec_config.plc_enable);
    }

    #endif /* AIR_CELT_DEC_V2_ENABLE */
    #ifdef AIR_AUDIO_ULD_DECODE_ENABLE
    else if (open_param->stream_in_param.ble.codec_type == BT_BLE_CODEC_ULD)
    {
        /* init celt codec */
        uld_dec_port_config_t uld_dec_config;
        uld_dec_config.bit_rate               = open_param->stream_in_param.ble.channel_num*8*((U32)open_param->stream_in_param.ble.frame_payload_length)*(1000000/(open_param->stream_in_param.ble.frame_ms));
        uld_dec_config.in_channel_num         = (open_param->stream_in_param.ble.channel_num == 1) ? 1 : 2;
        uld_dec_config.interval               = 2000; /*us*/

        stream_codec_decoder_uld_init(source, &uld_dec_config);
        DSP_MW_LOG_I("[BLE][ULD_DEC] init %u, %u %u\r\n", 3,
                uld_dec_config.bit_rate,
                uld_dec_config.in_channel_num,
                uld_dec_config.interval);
    }
    #endif

    SourceInitN9Ble(source);

    return source;
}
#endif /* AIR_BT_CODEC_BLE_ENABLED */

SOURCE StreamMemorySource(U8 *Memory_addr, U32 Memory_len)
{
    SOURCE source;

    if (Source_blks[SOURCE_TYPE_MEMORY] != NULL) {
        return Source_blks[SOURCE_TYPE_MEMORY];
    }

    source = new_source(SOURCE_TYPE_MEMORY);
    if (source) {
        source->streamBuffer.BufferInfo.startaddr[0]   = Memory_addr;
        source->streamBuffer.BufferInfo.length         = Memory_len;
        SourceInit_Memory(source);
    }

    return source;
}

SINK StreamMemorySink(U8 *Memory_addr, U32 Memory_len)
{
    SINK sink;

    if (Sink_blks[SINK_TYPE_MEMORY] != NULL) {
        return Sink_blks[SINK_TYPE_MEMORY];
    }

    sink = new_sink(SINK_TYPE_MEMORY);
    if (sink) {
        sink->streamBuffer.BufferInfo.startaddr[0]   = Memory_addr;
        sink->streamBuffer.BufferInfo.length         = Memory_len;
        sink->param.memory.memory_type = (((U32)Memory_addr >> 24) == 0x41) ? Flash_addr : Ram_addr;
        sink->param.memory.remain_len = 0;
        sink->taskid = DAV_TASK_ID;
        SinkInit_Memory(sink);
    }

    return sink;
}

#ifdef MTK_SENSOR_SOURCE_ENABLE
SOURCE StreamGsensorSource(void)
{
    SOURCE source = NULL;
    if (Source_blks[SOURCE_TYPE_GSENSOR] != NULL) {
        return Source_blks[SOURCE_TYPE_GSENSOR];
    }
    source = new_source(SOURCE_TYPE_GSENSOR);
    if (source) {
        SourceInit_GSENSOR(source);
    }
    return source;
}
#endif

#ifdef MTK_BT_A2DP_ENABLE
SOURCE StreamN9A2dpSource(void *param)
{
    SOURCE source;
    audio_dsp_a2dp_dl_open_param_p open_param = param;

    // printf("0x%08X\r\n", (uint32_t)param);

    if (Source_blks[SOURCE_TYPE_A2DP]) {
        return Source_blks[SOURCE_TYPE_A2DP];
    }
    source = new_source(SOURCE_TYPE_A2DP);

    if (source != NULL) {
        // printf("0x%08X\r\n", (uint32_t)&(source->param.n9_a2dp.codec_info));
        // printf("0x%08X\r\n", (uint32_t)&(source->param.n9_a2dp.share_info_base_addr));
        memcpy(&(source->param.n9_a2dp.codec_info), &(open_param->codec_info), sizeof(bt_codec_a2dp_audio_t));
        source->param.n9_a2dp.share_info_base_addr = (U32)open_param->p_share_info;
#ifdef MTK_BT_AVM_SHARE_BUF
        memcpy(&(source->streamBuffer.AVMBufferInfo), (U8*)source->param.n9_a2dp.share_info_base_addr, (sizeof(avm_share_buf_info_t) - 4 /*sizeof(reserved[1])*/));/* share info fix 36 byte */
        source->param.n9_a2dp.pcdc_info_buf = open_param->p_pcdc_anchor_info_buf;
#else
        memcpy(&(source->streamBuffer.ShareBufferInfo), (U8*)source->param.n9_a2dp.share_info_base_addr, (sizeof(n9_dsp_share_info_t) - 8 /*sizeof(asi_base)+sizeof(asi_current)*/));/* share info fix 32 byte */
        source->param.n9_a2dp.asi_buf = open_param->p_asi_buf;
        source->param.n9_a2dp.min_gap_buf = open_param->p_min_gap_buf;
#endif

        source->param.n9_a2dp.a2dp_bitrate_report.p_a2dp_bitrate_report = open_param->p_current_bit_rate;
        source->param.n9_a2dp.sink_latency = open_param->sink_latency;
        source->param.n9_a2dp.a2dp_lostnum_report = open_param->p_lostnum_report;

        source->param.n9_a2dp.sync_info = (AUDIO_SYNC_INFO*)hal_memview_cm4_to_dsp0((U32)((audio_dsp_a2dp_dl_open_param_p)param)->p_audio_sync_info);


#ifdef MTK_BT_AVM_SHARE_BUF
        source->streamBuffer.AVMBufferInfo.StartAddr = hal_memview_cm4_to_dsp0(source->streamBuffer.AVMBufferInfo.StartAddr);
#else

        source->streamBuffer.ShareBufferInfo.start_addr = hal_memview_cm4_to_dsp0(source->streamBuffer.ShareBufferInfo.start_addr);
        memset(source->param.n9_a2dp.a2dp_lostnum_report, 0, 8);
#endif
        DSP_MW_LOG_I("share_info_base_addr: 0x%08X 0x%08X 0x%08X\r\n", 3, source->param.n9_a2dp.share_info_base_addr, source->param.n9_a2dp.asi_buf, source->param.n9_a2dp.a2dp_bitrate_report.p_a2dp_bitrate_report);
        SourceInit_N9_a2dp(source);
    } else {
        return NULL;
    }

    return source;
}
#endif

SOURCE StreamCM4PlaybackSource(void *param)
{
    UNUSED(param);
    DSP_MW_LOG_I("[CM4_PB] Source is not supported ", 0);
    return NULL;
}

#ifdef MTK_PROMPT_SOUND_ENABLE
SOURCE StreamCM4VPPlaybackSource(void *param)
{
    audio_dsp_playback_info_t *info_from_cm4 = (audio_dsp_playback_info_t *)param;

    SOURCE source;
    U32 sample_rate;

    if (Source_blks[SOURCE_TYPE_CM4_VP_PLAYBACK]) {
        return Source_blks[SOURCE_TYPE_CM4_VP_PLAYBACK];
    }

    source = new_source(SOURCE_TYPE_CM4_VP_PLAYBACK);
    if (source != NULL) {

        switch (info_from_cm4->sampling_rate) {
            case AUDIO_SAMPLING_RATE_8KHZ:
                sample_rate = 8000;
                break;

            case AUDIO_SAMPLING_RATE_11_025KHZ:
                sample_rate = 11025;
                break;

            case AUDIO_SAMPLING_RATE_12KHZ:
                sample_rate = 12000;
                break;

            case AUDIO_SAMPLING_RATE_16KHZ:
                sample_rate = 16000;
                break;

            case AUDIO_SAMPLING_RATE_22_05KHZ:
                sample_rate = 22050;
                break;

            case AUDIO_SAMPLING_RATE_24KHZ:
                sample_rate = 24000;
                break;

            case AUDIO_SAMPLING_RATE_32KHZ:
                sample_rate = 32000;
                break;

            case AUDIO_SAMPLING_RATE_44_1KHZ:
                sample_rate = 44100;
                break;

            case AUDIO_SAMPLING_RATE_48KHZ:
                sample_rate = 48000;
                break;

            case AUDIO_SAMPLING_RATE_96KHZ:
                sample_rate = 96000;
                break;

            default:
                sample_rate = 16000;
                DSP_MW_LOG_I("[CM4_VP_PB] Unknown FS", 0);
        }

        source->param.cm4_playback.info.bit_type = (audio_bits_per_sample_t)info_from_cm4->bit_type;
        source->param.cm4_playback.info.sampling_rate = sample_rate;
        source->param.cm4_playback.info.channel_number = (audio_channel_number_t)info_from_cm4->channel_number;
        source->param.cm4_playback.info.codec_type = info_from_cm4->codec_type;
        source->param.cm4_playback.info.share_info_base_addr = hal_memview_cm4_to_dsp0((uint32_t)info_from_cm4->p_share_info);
        source->param.cm4_playback.info.source_channels
            = (AUDIO_MONO == source->param.cm4_playback.info.channel_number) ? 1 : 2;

        DSP_MW_LOG_I("[CM4_VP_PB] info_from_cm4 = 0x%x, bit_type = %d, sampling_rate = %d, channel_number = %d, codec_type = %d, share_info_base_addr = 0x%x", 6,
                     param, info_from_cm4->bit_type, info_from_cm4->sampling_rate, info_from_cm4->channel_number, info_from_cm4->codec_type, info_from_cm4->p_share_info);

        n9_dsp_share_info_t *share_buff_info  = (n9_dsp_share_info_t *)source->param.cm4_playback.info.share_info_base_addr;
        UNUSED(share_buff_info);
        DSP_MW_LOG_I("[CM4_VP_PB] wo = %x", 1, share_buff_info->write_offset);

        source->taskId = DAV_TASK_ID; // Yo: Should switch to DPR_TASK_ID after applying VP afe sink

        SourceInit_CM4_vp_playback(source);
    } else {
        return NULL;
    }

    return source;

}
#endif

#ifdef AIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
extern U16 cm4_vp_dummy_source_playback_return_share_information_size(uint8_t index);
extern U32 cm4_vp_dummy_source_playback_return_share_information_address(uint8_t index);
extern void cm4_vp_dummy_source_playback_set_param(uint8_t mode, uint8_t index);
SOURCE StreamCM4VPDummySourcePlaybackSource(void *param)
{
    cm4_playback_share_info_t *info_from_cm4 = (cm4_playback_share_info_t *)param;

    SOURCE source;
    U32 sample_rate;

    if (Source_blks[SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK]) {
        return Source_blks[SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK];
    }

    source = new_source(SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK);
    if (source != NULL) {

        switch (info_from_cm4->sampling_rate) {
            case AUDIO_SAMPLING_RATE_48KHZ:
                sample_rate = 48000;
                break;

            default:
                sample_rate = 48000;
                DSP_MW_LOG_I("[CM4_VP_DUMMY] Unknown FS", 0);
        }

        source->param.cm4_playback.info.bit_type = (audio_bits_per_sample_t)info_from_cm4->bit_type;
        source->param.cm4_playback.info.sampling_rate = sample_rate;
        source->param.cm4_playback.info.channel_number = (audio_channel_number_t)info_from_cm4->channel_number;
        source->param.cm4_playback.info.codec_type = info_from_cm4->codec_type;
        /*Test*/
        //source->param.cm4_playback.info.share_info_base_addr = hal_memview_cm4_to_dsp0(info_from_cm4->share_info_base_addr);
        source->param.cm4_playback.info.share_info_base_addr = info_from_cm4->share_info_base_addr;

        n9_dsp_share_info_t *share_buff_info  = source->param.cm4_playback.info.share_info_base_addr;

        share_buff_info->startaddr  = cm4_vp_dummy_source_playback_return_share_information_address(info_from_cm4->data_local_address_index);
        share_buff_info->length     = cm4_vp_dummy_source_playback_return_share_information_size(info_from_cm4->data_local_stream_mode);
        cm4_vp_dummy_source_playback_set_param(info_from_cm4->data_local_stream_mode, info_from_cm4->data_local_address_index);
        share_buff_info->sample_rate   =  48000;
        share_buff_info->bBufferIsFull = true;
        //DSP_MW_LOG_I("[Rdebug]StreamCM4DUMMY_SourcePlaybackSource share_info_base_addr(0x%x) info(0x%x) sh_length(%d) ReadOff(%d)", 4, source->param.cm4_playback.info.share_info_base_addr, info_from_cm4->share_info_base_addr, share_buff_info->length, share_buff_info->ReadOffset);
        /**/
        source->param.cm4_playback.info.source_channels
            = (AUDIO_MONO == source->param.cm4_playback.info.channel_number) ? 1 : 2;

        DSP_MW_LOG_I("[CM4_VP_DUMMY] info_from_cm4 = 0x%x", 1, param);
        DSP_MW_LOG_I("[CM4_VP_DUMMY] bit_type = %d", 1, info_from_cm4->bit_type);
        DSP_MW_LOG_I("[CM4_VP_DUMMY] sampling_rate = %d", 1, info_from_cm4->sampling_rate);
        DSP_MW_LOG_I("[CM4_VP_DUMMY] channel_number = %d", 1, info_from_cm4->channel_number);
        DSP_MW_LOG_I("[CM4_VP_DUMMY] codec_type = %d", 1, info_from_cm4->codec_type);
        DSP_MW_LOG_I("[CM4_VP_DUMMY] data_local_address = 0x%x", 1, info_from_cm4->data_local_address_index);
        DSP_MW_LOG_I("[CM4_VP_DUMMY] data_local_stream_mode = 0x%x", 1, info_from_cm4->data_local_stream_mode);

        //n9_dsp_share_info_t *share_buff_info  = source->param.cm4_playback.info.share_info_base_addr;

        //DSP_MW_LOG_I("[CM4_VP_DUMMY] wo = %x", 1, share_buff_info->WriteOffset);

        source->taskId = DHP_TASK_ID;

        SourceInit_CM4_vp_dummy_source_playback(source);
    } else {
        return NULL;
    }

    return source;

}
#endif

/**
 * StreamCm4RecordSink
 * @breif record data to share buffer
 */
SINK StreamCm4RecordSink(void *param)
{
    SINK sink = NULL;
#ifdef MTK_CM4_RECORD_ENABLE
    cm4_record_open_param_p record_open;
    if (Sink_blks[SINK_TYPE_CM4RECORD] != NULL) {
        return Sink_blks[SINK_TYPE_CM4RECORD];
    }
    sink = new_sink(SINK_TYPE_CM4RECORD);
    if (sink == NULL) {
        return NULL;
    }
    record_open = (cm4_record_open_param_p)param;

#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    sink->taskid = DHP_TASK_ID;
#else
    sink->taskid = DAV_TASK_ID;
#endif
    sink->param.cm4_record.share_info_base_addr = (n9_dsp_share_info_ptr)hal_memview_cm4_to_dsp0((U32)record_open->p_share_info);
    sink->param.cm4_record.frames_per_message = record_open->frames_per_message;
    sink->param.cm4_record.bitrate = record_open->bitrate;
    sink->param.cm4_record.interleave = record_open->interleave;
    SinkInitCm4Record(sink);
#else
    UNUSED(param);
#endif
    return sink;
}

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
/**
 * @brief This function is used to config the parameter of source based on the speical scenario type.
 *
 * @param open_param is pointer to the parameter from the cm4.
 * @param source_param is pointer to the parameter of the source.
 */
static void port_audio_transmitter_init_source(SOURCE source, audio_transmitter_open_param_p open_param, AUDIO_TRANSMITTER_PARAMETER *source_param)
{
    UNUSED(source);
    UNUSED(source_param);

    if (open_param->scenario_type == AUDIO_TRANSMITTER_A2DP_SOURCE) {
        //source_param->scenario_param.data_transl_param.first_packet = ;
    }
#if defined(AIR_GAMING_MODE_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) {
        /* Dongle side, music path, usb in case */
        if ((open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) ||
            (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1)) {
            /* task config */
            source->taskId = DHP_TASK_ID;
            /* codec config */
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.codec_type = open_param->scenario_param.gaming_mode_param.codec_type;
            if (open_param->scenario_param.gaming_mode_param.codec_type == AUDIO_DSP_CODEC_TYPE_PCM) {
                source_param->scenario_param.usb_in_broadcast_param.usb_in_param.codec_param.pcm.sample_rate  = open_param->scenario_param.gaming_mode_param.codec_param.pcm.sample_rate;
                source_param->scenario_param.usb_in_broadcast_param.usb_in_param.codec_param.pcm.channel_mode = open_param->scenario_param.gaming_mode_param.codec_param.pcm.channel_mode;
                source_param->scenario_param.usb_in_broadcast_param.usb_in_param.codec_param.pcm.format       = open_param->scenario_param.gaming_mode_param.codec_param.pcm.format;
            }
            /* data config */
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.data_period = open_param->scenario_param.gaming_mode_param.period;
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.data_timestamp = 0;
            /* timer config */
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.timer.period = open_param->scenario_param.gaming_mode_param.period / 3;
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.timer.handle = 0;
            // hal_gpt_sw_get_timer(&source_param->scenario_param.usb_in_broadcast_param.usb_in_param.timer.handle);
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.timer.timestamp = 0;
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.timer.callback = NULL;
        }
    }
#endif /* AIR_GAMING_MODE_DONGLE_ENABLE */
#if defined (AIR_WIRED_AUDIO_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_WIRED_AUDIO) {
        wired_audio_usb_in_source_init(source, open_param, source_param);
    }
#endif /*AIR_WIRED_AUDIO_ENABLE*/
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) {
        /* Dongle side, music path, usb in case */
        if ((open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) ||
            (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1)
            #ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
                || (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN)
            #endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */
            #ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
                || (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)
            #endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
            ) {
            /* task config */
            source->taskId = DHP_TASK_ID;
            /* codec config */
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.codec_type = open_param->scenario_param.ble_audio_dongle_param.codec_type;
            if (open_param->scenario_param.ble_audio_dongle_param.codec_type == AUDIO_DSP_CODEC_TYPE_PCM) {
                source_param->scenario_param.usb_in_broadcast_param.usb_in_param.codec_param.pcm.sample_rate  = open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate;
                source_param->scenario_param.usb_in_broadcast_param.usb_in_param.codec_param.pcm.channel_mode = open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.channel_mode;
                source_param->scenario_param.usb_in_broadcast_param.usb_in_param.codec_param.pcm.format       = open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.format;
            }
            /* data config */
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.data_period = open_param->scenario_param.ble_audio_dongle_param.period;
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.data_timestamp = 0;
            /* clock skew config */
            source_param->scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples = 0;
        }
    }
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE) {
        /* Dongle side, music path, usb in case */
        if ((open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) ||
            (open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)) {

        } else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) {

        } else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) {

        } else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0) {

        }

    }
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
// TODO Fix bug
#if defined(AIR_BT_AUDIO_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE) {
        /* Dongle side, music path, usb in case */
        switch (open_param->scenario_sub_id) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
                break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
                break;
#endif
            default:
                break;
        }
    }
#endif /*  AIR_BT_AUDIO_DONGLE_ENABLE */
    else {
        AUDIO_ASSERT(0);
    }
}

/**
 * @brief This function is used to init auido transmitter source.
 *
 * @param param is pointer to the parameter.
 * @return SOURCE is the instance which is initialized.
 */
SOURCE StreamAudioTransmitterSource(void *param)
{
    SOURCE source = NULL;
    SOURCE_TYPE source_type;
    audio_transmitter_open_param_p open_param;

    for (source_type = SOURCE_TYPE_AUDIO_TRANSMITTER_MIN; source_type <= SOURCE_TYPE_AUDIO_TRANSMITTER_MAX ; source_type++) {
        if (Source_blks[source_type] == NULL) {
            source = new_source(source_type);
            break;
        }
    }
    if (source == NULL) {
        return NULL;
    }

    open_param = (audio_transmitter_open_param_p)param;
    source->type = source_type;
    source->buftype = BUFFER_TYPE_CIRCULAR_BUFFER;
    source->taskId = DAV_TASK_ID;
    source->param.data_dl.share_info_base_addr = (n9_dsp_share_info_ptr)hal_memview_cm4_to_dsp0((U32)open_param->p_share_info);
    source->param.data_dl.share_info_base_addr->bBufferIsFull = false;
    source->param.data_dl.scenario_type = open_param->scenario_type;
    source->param.data_dl.scenario_sub_id = open_param->scenario_sub_id;
    source->param.data_dl.data_notification_frequency = open_param->data_notification_frequency;
    source->param.data_dl.max_payload_size = open_param->max_payload_size;
    source->param.data_dl.current_notification_index = 0;
    source->param.data_dl.seq_num = 0;
    source->param.data_dl.frame_size = 0;
    source->param.data_dl.is_assembling = false;
    source->param.data_dl.is_customize = false;

    /* Set the sink parameters from open parameters */
    port_audio_transmitter_init_source(source, open_param, &(source->param.data_dl));

    SourceInit_audio_transmitter(source);

    return source;
}

/*
 * This function is used to config the parameter of sink.
 * User should add porting code in this function for the specific scenario.
 */
static void port_audio_transmitter_init_sink(SINK sink, audio_transmitter_open_param_p open_param, AUDIO_TRANSMITTER_PARAMETER *sink_param)
{
    UNUSED(sink);
    UNUSED(sink_param);

    if (open_param->scenario_type == AUDIO_TRANSMITTER_A2DP_SOURCE) {
        //sink_param->scenario_param.data_transl_param.first_packet = ;
    }
#ifdef MTK_SENSOR_SOURCE_ENABLE
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_GSENSOR) {

    }
#endif
#ifdef AIR_MULTI_MIC_STREAM_ENABLE
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_MULTI_MIC_STREAM) {

    }
#endif
#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) {
        /* Dongle side, voice path, usb out case */
        if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) {
            /* codec config */
            sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.codec_type = open_param->scenario_param.gaming_mode_param.codec_type;
            if (open_param->scenario_param.gaming_mode_param.codec_type == AUDIO_DSP_CODEC_TYPE_PCM) {
                sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.codec_param.pcm.sample_rate  = open_param->scenario_param.gaming_mode_param.codec_param.pcm.sample_rate;
                sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.codec_param.pcm.channel_mode = open_param->scenario_param.gaming_mode_param.codec_param.pcm.channel_mode;
                sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.codec_param.pcm.format       = open_param->scenario_param.gaming_mode_param.codec_param.pcm.format;
            }
            /* data config */
            sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.data_period = open_param->scenario_param.gaming_mode_param.period;
            sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.data_timestamp = 0;
            /* timer config */
            sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.timer.period = 0;
            sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.timer.handle = 0;
            sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.timer.timestamp = 0;
            sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.timer.callback = NULL;
        } else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET) {
            DSP_ALG_UpdateEscoRxMode(VOICE_WB);
            sink_param->scenario_param.voice_param.seq_num = 0;
#if defined(AIR_ULL_VOICE_LOW_LATENCY_ENABLE)
            sink_param->scenario_param.voice_param.frame_size = 120;
            sink_param->scenario_param.voice_param.process_frame_num = 1;
#else
            sink_param->scenario_param.voice_param.frame_size = 240;
            sink_param->scenario_param.voice_param.process_frame_num = 2;
#endif
            sink_param->scenario_param.voice_param.ul_process_done = false;
            sink_param->scenario_param.voice_param.dl_irq_cnt = 0;
            sink_param->is_assembling = true;
            sink_param->is_customize = true;
        }
    }
#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE */
#ifdef AIR_WIRED_AUDIO_ENABLE
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_WIRED_AUDIO) {
        wired_audio_usb_out_sink_init(sink, open_param, sink_param);
    }
#endif
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) {
        /* Dongle side, voice path, usb out case */
        if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT) {
            /* codec config */
            sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.codec_type = open_param->scenario_param.ble_audio_dongle_param.codec_type;
            if (open_param->scenario_param.ble_audio_dongle_param.codec_type == AUDIO_DSP_CODEC_TYPE_PCM) {
                sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.codec_param.pcm.sample_rate  = open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate;
                sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.codec_param.pcm.channel_mode = open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.channel_mode;
                sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.codec_param.pcm.format       = open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.format;
            }
            /* data config */
            sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.data_period = open_param->scenario_param.ble_audio_dongle_param.period;
            sink_param->scenario_param.usb_out_broadcast_param.usb_out_param.data_timestamp = 0;
            /* audio transmitter behavior config */
            sink_param->is_assembling = true;
            sink_param->is_customize = true;
        }
    }
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE) {
        /* Dongle side, voice path, usb out case */
        if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0)
        {
            /* audio transmitter behavior config */
            sink_param->is_assembling = true;
            sink_param->is_customize = true;
        }
    }
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_WIRELESS_MIC_RX) {
        /* Dongle side, voice path, usb out case */
        if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0)
        {
            /* audio transmitter behavior config */
            sink_param->is_assembling = true;
            sink_param->is_customize = true;
        }
    }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
#if defined(AIR_BT_AUDIO_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE) {
        /* Dongle side, voice path, usb out case */
        #ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
            if ((open_param->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) ||
                (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1)) {
                /* audio transmitter behavior config */
                sink_param->is_assembling = true;
                sink_param->is_customize = true;
            }
        #endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */
    }
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */
#if defined(AIR_RECORD_ADVANCED_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_ADVANCED_RECORD) {
        /* Dongle side, voice path, usb out case */
        if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC)
        {
            sink_param->is_assembling = true;
            sink_param->is_customize = true;
        }
    }
#endif /* AIR_RECORD_ADVANCED_ENABLE */
    else {
        AUDIO_ASSERT(0);
    }
}

SINK StreamAudioTransmitterSink(void *param)
{
    SINK sink = NULL;
    SINK_TYPE sink_type;
    audio_transmitter_open_param_p open_param;

    for (sink_type = SINK_TYPE_AUDIO_TRANSMITTER_MIN ; sink_type <= SINK_TYPE_AUDIO_TRANSMITTER_MAX ; sink_type++) {
        if (Sink_blks[sink_type] == NULL) {
            sink = new_sink(sink_type);
            break;
        }
    }
    if (sink == NULL) {
        return NULL;
    }

    open_param = (audio_transmitter_open_param_p)param;
    sink->type = sink_type;
    sink->buftype = BUFFER_TYPE_CIRCULAR_BUFFER;
    sink->taskid = DAV_TASK_ID;
    sink->param.data_ul.share_info_base_addr = (n9_dsp_share_info_ptr)hal_memview_cm4_to_dsp0((U32)open_param->p_share_info);
    sink->param.data_ul.share_info_base_addr->bBufferIsFull = false;
    sink->param.data_ul.scenario_type = open_param->scenario_type;
    sink->param.data_ul.scenario_sub_id = open_param->scenario_sub_id;
    sink->param.data_ul.data_notification_frequency = open_param->data_notification_frequency;
    sink->param.data_ul.max_payload_size = open_param->max_payload_size;
    memcpy(&sink->param.data_ul.scenario_param, &open_param->scenario_param, sizeof(audio_transmitter_scenario_open_param_t));
    sink->param.data_ul.current_notification_index = 0;
    sink->param.data_ul.seq_num = 0;
    sink->param.data_ul.frame_size = 0;
    sink->param.data_ul.is_assembling = false;
    sink->param.data_ul.is_customize = false;

    /* Set the sink parameters from open parameters */
    port_audio_transmitter_init_sink(sink, open_param, &(sink->param.data_ul));

    SinkInit_audio_transmitter(sink);

    return sink;
}
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

#ifdef AIR_AUDIO_BT_COMMON_ENABLE
/**
 * @brief This function is used to config the parameter of source based on the speical scenario type.
 *
 * @param source is pointer to the source who needs to be inited.
 * @param open_param is pointer to the parameter from the cm4.
 * @param source_param is pointer to the parameter of the source.
 */
static void port_bt_common_init_source(SOURCE source, bt_common_open_param_p open_param, BT_COMMON_PARAMETER *source_param)
{
    UNUSED(source);
    UNUSED(source_param);

    if (open_param->scenario_type == AUDIO_TRANSMITTER_A2DP_SOURCE) {
        //source_param->scenario_param.data_transl_param.first_packet = ;
    }
#if defined(AIR_GAMING_MODE_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) {
        /* Dongle side, voice path, bt source in case */
        if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
            || (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT)
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
            ) {
            /* codec config */
            source_param->scenario_param.usb_out_broadcast_param.bt_in_param.codec_type = open_param->scenario_param.gaming_mode_param.codec_type;
            if (open_param->scenario_param.gaming_mode_param.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS) {
                if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) {
                extern stream_feature_list_t stream_feature_list_usb_out_broadcast[];
                stream_feature_configure_type(stream_feature_list_usb_out_broadcast, CODEC_DECODER_CELT_HD, CONFIG_DECODER);
    }
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
    else if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
            extern stream_feature_list_t stream_feature_list_game_line_out_broadcast[];
            stream_feature_configure_type(stream_feature_list_game_line_out_broadcast, CODEC_DECODER_CELT_HD, CONFIG_DECODER);
                }
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
            source_param->scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.opus.sample_rate  = open_param->scenario_param.gaming_mode_param.codec_param.opus.sample_rate;
            source_param->scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.opus.channel_mode = open_param->scenario_param.gaming_mode_param.codec_param.opus.channel_mode;
            source_param->scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.opus.bit_rate     = open_param->scenario_param.gaming_mode_param.codec_param.opus.bit_rate;
        }
        /* data config */
        source_param->scenario_param.usb_out_broadcast_param.bt_in_param.data_period = open_param->scenario_param.gaming_mode_param.period;
        source_param->scenario_param.usb_out_broadcast_param.bt_in_param.data_timestamp = 0;
        /* timer config */
        source_param->scenario_param.usb_out_broadcast_param.bt_in_param.timer.period = open_param->scenario_param.gaming_mode_param.period / 3;
        source_param->scenario_param.usb_out_broadcast_param.bt_in_param.timer.handle = 0;
        // hal_gpt_sw_get_timer(&source_param->scenario_param.usb_out_broadcast_param.bt_in_param.timer.handle);
        source_param->scenario_param.usb_out_broadcast_param.bt_in_param.timer.timestamp = 0;
        source_param->scenario_param.usb_out_broadcast_param.bt_in_param.timer.callback = NULL;
        /* fetch flag config */
        source_param->scenario_param.usb_out_broadcast_param.bt_in_param.fetch_flag = 0;
    }
    }
#endif /* AIR_GAMING_MODE_DONGLE_ENABLE */
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) {
        /* Dongle side, voice path, bt source in case */
        if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT) {
            /* codec config */
            source_param->scenario_param.usb_out_broadcast_param.bt_in_param.codec_type = open_param->scenario_param.ble_audio_dongle_param.codec_type;
            if (open_param->scenario_param.ble_audio_dongle_param.codec_type == AUDIO_DSP_CODEC_TYPE_LC3) {
                extern stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_out_broadcast[];
                stream_feature_configure_type(stream_feature_list_ble_audio_dongle_usb_out_broadcast, CODEC_DECODER_LC3, CONFIG_DECODER);
#ifdef PRELOADER_ENABLE
                if (open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval == 10000) {
                    lc3i_fft10ms_library_load(NULL, NULL, NULL);
                } else if (open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval == 7500) {
                    lc3i_fft7_5ms_library_load(NULL, NULL, NULL);
                } else {
                    DSP_MW_LOG_E("[BLE Audio Dongle] unsupported DL frame interval %d", 1, open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval);
                }
#endif
                source_param->scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.sample_rate    = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.sample_rate;
                source_param->scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.channel_mode   = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.channel_mode;
                source_param->scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.bit_rate       = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.bit_rate;
                source_param->scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.frame_interval = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval;
                source_param->scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.frame_size     = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_size;
            }
            /* data config */
            source_param->scenario_param.usb_out_broadcast_param.bt_in_param.data_period = open_param->scenario_param.ble_audio_dongle_param.period;
            source_param->scenario_param.usb_out_broadcast_param.bt_in_param.data_timestamp = 0;
        }
    }
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE) {
        /* Dongle side, voice path, bt source in case */
        if ((open_param->scenario_sub_id >= AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0) && (open_param->scenario_sub_id < AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0))
        {
        }
    }
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_WIRELESS_MIC_RX) {
        /* Dongle side, voice path, bt source in case */
        if ((open_param->scenario_sub_id >= AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0) && (open_param->scenario_sub_id < AUDIO_TRANSMITTER_WIRELESS_MIC_RX_SUB_ID_MAX))
        {
            wireless_mic_rx_ul_source_init(source, open_param, source_param);
        }
    }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
#if defined(AIR_BT_AUDIO_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE) {
        /* Dongle side, voice path, usb out case */
        #ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
            if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) {
                CONNECTION_IF *con_if = port_audio_transmitter_get_connection_if(open_param->scenario_type, (audio_transmitter_scenario_sub_id_t)(uint32_t)open_param->scenario_sub_id);
                if (open_param->scenario_param.bt_audio_dongle_param.ul_info.source.bt_in.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) {
                    con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_out_hfp_msbc;
                } else if (open_param->scenario_param.bt_audio_dongle_param.ul_info.source.bt_in.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_CVSD) {
                    con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_out_hfp_cvsd;
                }
            } else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1) {
                CONNECTION_IF *con_if = port_audio_transmitter_get_connection_if(open_param->scenario_type, (audio_transmitter_scenario_sub_id_t)(uint32_t)open_param->scenario_sub_id);
                if (open_param->scenario_param.bt_audio_dongle_param.ul_info.source.bt_in.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) {
                    con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_out_hfp_msbc;
                } else if (open_param->scenario_param.bt_audio_dongle_param.ul_info.source.bt_in.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_CVSD) {
                    con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_out_hfp_cvsd;
                }
            }
        #endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */
    }
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */
    else {
        AUDIO_ASSERT(0);
    }
}

/**
 * @brief This function is used to init bt common source.
 *
 * @param param is pointer to the parameter.
 * @return SOURCE is the instance which is initialized.
 */
SOURCE StreamBTCommonSource(void *param)
{
    SOURCE source = NULL;
    SOURCE_TYPE source_type;
    bt_common_open_param_p open_param;

    for (source_type = SOURCE_TYPE_BT_COMMON_MIN; source_type <= SOURCE_TYPE_BT_COMMON_MAX ; source_type++) {
        if (Source_blks[source_type] == NULL) {
            source = new_source(source_type);
            break;
        }
    }
    if (source == NULL) {
        return NULL;
    }

    open_param = (bt_common_open_param_p)param;
    source->type = source_type;
    source->buftype = BUFFER_TYPE_CIRCULAR_BUFFER;
    source->taskId = DAV_TASK_ID;
    source->param.bt_common.share_info_type = open_param->share_info_type;
    source->param.bt_common.share_info_base_addr = (STREAM_BUFFER *)hal_memview_cm4_to_dsp0((U32)open_param->p_share_info);
    source->param.bt_common.scenario_type = open_param->scenario_type;
    source->param.bt_common.scenario_sub_id = open_param->scenario_sub_id;
    source->param.bt_common.data_notification_frequency = open_param->data_notification_frequency;
    source->param.bt_common.max_payload_size = open_param->max_payload_size;
    source->param.bt_common.current_notification_index = 0;
    source->param.bt_common.seq_num = 0;
    source->param.bt_common.frame_size = 0;
    source->param.bt_common.status = BT_COMMON_STATUS_DEINIT;

    /* Set the sink parameters from open parameters */
    port_bt_common_init_source(source, open_param, &(source->param.bt_common));

    SourceInit_bt_common(source);

    return source;
}

/**
 * @brief This function is used to config the parameter of sink based on the speical scenario type.
 *
 * @param sink is pointer to the sink who needs to be inited.
 * @param open_param is pointer to the parameter from the cm4.
 * @param sink_param is pointer to the parameter of the sink.
 */
extern stream_feature_list_t stream_feature_list_bt_audio_dongle_usb_in_broadcast_0[];
extern stream_feature_list_t stream_feature_list_bt_audio_dongle_usb_in_broadcast_1[];
static void port_bt_common_init_sink(SINK sink, bt_common_open_param_p open_param, BT_COMMON_PARAMETER *sink_param)
{
    UNUSED(sink);
    UNUSED(sink_param);

    if (open_param->scenario_type == AUDIO_TRANSMITTER_A2DP_SOURCE) {

    }
#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) {
        /* Dongle side, music path, bt sink out case */
        if ((open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) ||
            (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1)
#ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
            || (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN)
#endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE */
#ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
            || (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_I2S_IN)
#endif /* AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
            ) {
            /* task config */
            sink->taskid = DHP_TASK_ID;
            /* codec config */
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_type = open_param->scenario_param.gaming_mode_param.codec_type;
            if (open_param->scenario_param.gaming_mode_param.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS) {
                if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
                    extern stream_feature_list_t stream_feature_list_usb_in_broadcast_0[];
                    stream_feature_configure_type(stream_feature_list_usb_in_broadcast_0, CODEC_ENCODER_OPUS, CONFIG_ENCODER);
                } else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1) {
                    extern stream_feature_list_t stream_feature_list_usb_in_broadcast_1[];
                    stream_feature_configure_type(stream_feature_list_usb_in_broadcast_1, CODEC_ENCODER_OPUS, CONFIG_ENCODER);
                }
#ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
        else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN) {
                    extern stream_feature_list_t stream_feature_list_game_line_in_broadcast[];
                    stream_feature_configure_type(stream_feature_list_game_line_in_broadcast, CODEC_ENCODER_OPUS, CONFIG_ENCODER);
        }
#endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE */
#ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
        else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_I2S_IN) {
                    extern stream_feature_list_t stream_feature_list_game_i2s_in_broadcast[];
                    stream_feature_configure_type(stream_feature_list_game_i2s_in_broadcast, CODEC_ENCODER_OPUS, CONFIG_ENCODER);
                }
#endif /* AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.opus.sample_rate  = open_param->scenario_param.gaming_mode_param.codec_param.opus.sample_rate;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.opus.channel_mode = open_param->scenario_param.gaming_mode_param.codec_param.opus.channel_mode;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.opus.bit_rate     = open_param->scenario_param.gaming_mode_param.codec_param.opus.bit_rate;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.opus.version      = open_param->scenario_param.gaming_mode_param.codec_param.opus.version;
#ifdef AIR_CELT_ENC_ENABLE
                extern uint32_t stream_codec_celt_set_version(uint32_t version);
                stream_codec_celt_set_version(open_param->scenario_param.gaming_mode_param.codec_param.opus.version);
                ULL_LOG_I("[ULL][DL][config] codec version, 0x%x\r\n", 1, open_param->scenario_param.gaming_mode_param.codec_param.opus.version);
                stream_codec_encoder_celt_init(480 * 3);
#endif /* AIR_CELT_ENC_ENABLE */
            }
            /* data config */
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.data_period = open_param->scenario_param.gaming_mode_param.period;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp = 0;
            /* event config */
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.event_status = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.event_failcount = 0;
            /* timer config */
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.timer.period = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.timer.handle = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.timer.timestamp = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.timer.callback = NULL;
            /* clock skew config */
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.clk_skew.last_timestamp = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.clk_skew.act_time = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.clk_skew.act_count = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.clk_skew.act_diff = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.clk_skew.ave_diff = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.clk_skew.compen_samples = 0;
        }
    }
#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) {
        CONNECTION_IF *con_if = port_audio_transmitter_get_connection_if(open_param->scenario_type,
                                                                    (audio_transmitter_scenario_sub_id_t)(uint32_t)open_param->scenario_sub_id);
        /* Dongle side, music path, bt sink out case */
        if ((open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) ||
            (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1)
            #ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
                || (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN)
            #endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */
            #ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
                || (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)
            #endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
            ) {
            /* task config */
            sink->taskid = DHP_TASK_ID;
            /* codec config */
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_type = open_param->scenario_param.ble_audio_dongle_param.codec_type;
            if (open_param->scenario_param.ble_audio_dongle_param.codec_type == AUDIO_DSP_CODEC_TYPE_LC3) {
                extern stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_in_broadcast_0[];
                extern stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_in_broadcast_1[];
                if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) {
                    con_if->pfeature_table = stream_feature_list_ble_audio_dongle_usb_in_broadcast_0;
                } else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1) {
                    con_if->pfeature_table = stream_feature_list_ble_audio_dongle_usb_in_broadcast_1;
                } else {
                    stream_feature_configure_type(con_if->pfeature_table, CODEC_ENCODER_LC3_BRANCH, CONFIG_ENCODER_REPLACE);
                }
#ifdef PRELOADER_ENABLE
                if (open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval == 10000) {
                    lc3i_fft10ms_library_load(NULL, NULL, NULL);
                } else if (open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval == 7500) {
                    lc3i_fft7_5ms_library_load(NULL, NULL, NULL);
                } else {
                    DSP_MW_LOG_E("[BLE Audio Dongle] unsupported UL frame interval %d", 1, open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval);
                }
#endif
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3.sample_rate    = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.sample_rate;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3.channel_mode   = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.channel_mode;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3.bit_rate       = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.bit_rate;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3.frame_interval = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3.frame_size     = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_size;
            }
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
            else if (open_param->scenario_param.ble_audio_dongle_param.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS) {
                DSP_MW_LOG_I("[ble audio dongle][dl] sink init: lc3plus %d, %d, %d, %d, %d, %d", 6,
                    open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.sample_rate,
                    open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.channel_mode,
                    open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.bit_rate,
                    open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.frame_interval,
                    open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.frame_size,
                    open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.sample_format);
                extern stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_in_broadcast_0_LC3_P[];
                if ((open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) ||
                   (open_param->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1)) {
                    con_if->pfeature_table = stream_feature_list_ble_audio_dongle_usb_in_broadcast_0_LC3_P;
                } else {
                    stream_feature_configure_type(con_if->pfeature_table, CODEC_ENCODER_LC3PLUS, CONFIG_ENCODER_REPLACE);
                }
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3plus.sample_rate    = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.sample_rate;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3plus.channel_mode   = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.channel_mode;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3plus.bit_rate       = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.bit_rate;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3plus.frame_interval = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.frame_interval;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3plus.frame_size     = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.frame_size;
                sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3plus.sample_format  = open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.sample_format;
            }
#endif
            /* data config */
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.data_period = open_param->scenario_param.ble_audio_dongle_param.period;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp = 0;
            /* event config */
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.event_status = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp = 0;
            sink_param->scenario_param.usb_in_broadcast_param.bt_out_param.event_failcount = 0;
        }
    }
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE) {
        /* Dongle side, music path, bt sink out case */
        if ((open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) ||
            (open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)) {

        } else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) {

        } else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) {

        } else if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0) {

        }
    }
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
// TODO Fix bug
#if defined(AIR_BT_AUDIO_DONGLE_ENABLE)
    else if (open_param->scenario_type == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE) {
        CONNECTION_IF *con_if = port_audio_transmitter_get_connection_if(open_param->scenario_type, (audio_transmitter_scenario_sub_id_t)(uint32_t)open_param->scenario_sub_id);
        /* Dongle side, music path, usb in case */
        switch (open_param->scenario_sub_id) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
                if (open_param->scenario_param.bt_audio_dongle_param.dl_info.sink.bt_out.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
                    con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_in_a2dp;
                    stream_feature_configure_type(stream_feature_list_bt_audio_dongle_usb_in_a2dp, CODEC_ENCODER_SBC, CONFIG_ENCODER_REPLACE);
                } else if (open_param->scenario_param.bt_audio_dongle_param.dl_info.sink.bt_out.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) {
                    con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_in_hfp_msbc;
                } else if (open_param->scenario_param.bt_audio_dongle_param.dl_info.sink.bt_out.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_CVSD) {
                    con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_in_hfp_cvsd;
                }
                #ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
                    else if (open_param->scenario_param.bt_audio_dongle_param.dl_info.sink.bt_out.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_LHDC) {
                        con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_in_a2dp_lhdc;
                        // stream_feature_configure_type(stream_feature_list_bt_audio_dongle_usb_in_a2dp, CODEC_ENCODER_VENDOR, CONFIG_ENCODER_REPLACE);
                    }
                #endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */
                break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
                // TODO:
                if (open_param->scenario_param.bt_audio_dongle_param.dl_info.sink.bt_out.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
                    // con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_in_a2dp;
                } else if (open_param->scenario_param.bt_audio_dongle_param.dl_info.sink.bt_out.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) {
                    // con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_in_hfp_msbc;
                } else if (open_param->scenario_param.bt_audio_dongle_param.dl_info.sink.bt_out.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_CVSD) {
                    // con_if->pfeature_table = stream_feature_list_bt_audio_dongle_usb_in_hfp_cvsd;
                }
                break;
#endif
            default:
                UNUSED(con_if);
                break;
        }
    }
#endif /*  AIR_BT_AUDIO_DONGLE_ENABLE */
    else {
        DSP_MW_LOG_E("init fail %d-%d", 2, open_param->scenario_type, open_param->scenario_sub_id);
        AUDIO_ASSERT(0);
    }
}

/**
 * @brief This function is used to init bt common ssink.
 *
 * @param param is pointer to the parameter.
 * @return SINK is the instance which is initialized.
 */
SINK StreamBTCommonSink(void *param)
{
    SINK sink = NULL;
    SINK_TYPE sink_type;
    bt_common_open_param_p open_param;

    for (sink_type = SINK_TYPE_BT_COMMON_MIN ; sink_type <= SINK_TYPE_BT_COMMON_MAX ; sink_type++) {
        if (Sink_blks[sink_type] == NULL) {
            sink = new_sink(sink_type);
            break;
        }
    }
    if (sink == NULL) {
        return NULL;
    }

    open_param = (bt_common_open_param_p)param;
    sink->type = sink_type;
    sink->buftype = BUFFER_TYPE_CIRCULAR_BUFFER;
    sink->taskid = DAV_TASK_ID;
    sink->param.bt_common.share_info_type = open_param->share_info_type;
    sink->param.bt_common.share_info_base_addr = (STREAM_BUFFER *)hal_memview_cm4_to_dsp0((U32)open_param->p_share_info);
    sink->param.bt_common.scenario_type = open_param->scenario_type;
    sink->param.bt_common.scenario_sub_id = open_param->scenario_sub_id;
    sink->param.bt_common.data_notification_frequency = open_param->data_notification_frequency;
    sink->param.bt_common.max_payload_size = open_param->max_payload_size;
    sink->param.bt_common.current_notification_index = 0;
    sink->param.bt_common.seq_num = 0;
    sink->param.bt_common.frame_size = 0;
    sink->param.bt_common.status = BT_COMMON_STATUS_DEINIT;

    /* Set the sink parameters from open parameters */
    port_bt_common_init_sink(sink, open_param, &(sink->param.bt_common));

    SinkInit_bt_common(sink);

    return sink;
}
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */

VOID StreamCloseAll(TRANSFORM transform, CLOSE_MODE_enum_s mode)
{
    DSP_STREAMING_PARA_PTR pStream;
    pStream = DSP_Streaming_Get(transform->source, transform->sink);
    U16 AckEndId = MSG_DSP_NULL_REPORT;
    DSP_MW_LOG_I("close all", 0);
    if (InstantCloseMode == mode) {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
        audio_ops_trigger(transform->sink, AFE_PCM_TRIGGER_STOP);
        audio_ops_trigger(transform->source, AFE_PCM_TRIGGER_STOP);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
        if (NULL != pStream) {
            AckEndId = pStream->DspReportEndId;
            DSP_Callback_Config(transform->source, transform->sink, NULL, FALSE);
        }
        PL_CRITICAL(StreamTransformClose, (VOID *)transform);
        if (AckEndId != MSG_DSP_NULL_REPORT) {
            aud_msg_ack(AckEndId, FALSE);
        }
    } else if (DspSoftCloeMode == mode) {
        if (NULL != pStream) {
            PL_CRITICAL(TransformChangeHandlerClose, (VOID *)transform);
        } else {
            AUDIO_ASSERT(0);
        }
    } else {
        AUDIO_ASSERT(0);
    }
}
VOID StreamDSPClose(SOURCE source, SINK sink, U16 msgID)
{
    DSP_STREAMING_PARA_PTR  pStream = DSP_Streaming_Get(source, sink);
#ifdef ENABLE_HWSRC_CLKSKEW
        if (Clock_Skew_HWSRC_Is_Enable(source, sink)) {
            hal_audio_src_underrun_monitor_stop();
        }
#endif /* ENABLE_HWSRC_CLKSKEW */
    pStream->DspReportEndId = msgID;
    pStream->streamingStatus = STREAMING_END;
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    Sink_Audio_ClosureSmooth(sink);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
    vTaskResume(pStream->callback.EntryPara.DSPTask);
    //source->transform->Handler(source,sink);
}

ATTR_TEXT_IN_IRAM_LEVEL_1 VOID StreamDSP_HWSemaphoreTake(VOID)
{
    uint32_t take_times = 0;

    /* Add hw semaphore to avoid multi-core access */
    while (++take_times) {
        if (HAL_HW_SEMAPHORE_STATUS_OK == hal_hw_semaphore_take(HW_SEMAPHORE_DSP_CIRCULAR_BUFFER)) {
            break;
        }

        if (take_times > HwSemRetryTimes) {
            //error handling
            // printf("%s : Can not take HW Semaphore\r\n", __func__);
            AUDIO_ASSERT(0);
        }
        // vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

ATTR_TEXT_IN_IRAM_LEVEL_1 VOID StreamDSP_HWSemaphoreGive(VOID)
{
    if (HAL_HW_SEMAPHORE_STATUS_OK != hal_hw_semaphore_give(HW_SEMAPHORE_DSP_CIRCULAR_BUFFER)) {
        //error handling
        // printf("%s : Can not give HW Semaphore\r\n", __func__);
        AUDIO_ASSERT(0);
    }
}

