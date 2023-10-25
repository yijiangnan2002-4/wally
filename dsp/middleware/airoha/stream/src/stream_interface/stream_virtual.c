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
 *@file   stream_audio.c
 *@brief  Defines the audio stream
 *
 @verbatim
 @endverbatim
 */

//-
#include "types.h"
#include "dsp_drv_dfe.h"
#include "stream_audio.h"
#include "audio_config.h"
//#include "os_intr_lv3.h"
#include "dsp_buffer.h"
#include "source.h"
#include "sink.h"
#include "source_inter.h"
#include "sink_inter.h"
#include "dsp_memory.h"
#include <string.h>
#ifdef AIR_DCHS_MODE_ENABLE
#include "dsp_mux_uart.h"
#endif

#include "stream_virtual.h"

BOOL SinkFlushVirtualSink(SINK sink, U32 amount)
{

    if (sink->param.virtual_para.entry != NULL) {
        sink->param.virtual_para.entry((VOID *)sink->streamBuffer.BufferInfo.startaddr[0], amount);
    }
    if (sink->param.virtual_para.handler != NULL) {
        //MSG_MessageSend(sink->param.virtual_para.handler, MESSAGE_SINK_NEW_DATA_IN, 0);
    }
    return TRUE;

}

BOOL SinkCloseVirtualSink(SINK sink)
{
    sink->param.virtual_para.user_count--;
    if(!sink->param.virtual_para.user_count) {
        if (sink->streamBuffer.BufferInfo.startaddr[0]) {
            vPortFree(sink->streamBuffer.BufferInfo.startaddr[0]);
            sink->streamBuffer.BufferInfo.startaddr[0] = NULL;
        }
    }
    return (sink->param.virtual_para.user_count) ? false : true;
}

U32 SinkSlackVirtualSink(SINK sink)
{

    return (sink->param.virtual_para.mem_size) ? sink->param.virtual_para.mem_size : 0xffffffff ;
}

U32 SinkClaimVirtualSink(SINK sink, U32 len)
{
    return (len > sink->param.virtual_para.mem_size)
           ? 0xffffffff
           : 0;
}

U8 *SinkMapVirtualSink(SINK sink)
{
    return (U8 *)sink->streamBuffer.BufferInfo.startaddr[0];
}

BOOL SinkWriteBufVirtualSink(SINK sink, U8 *src_addr, U32 length)
{
    TRANSFORM transform = sink->transform;
    DSP_CALLBACK_PTR callback_ptr;

    if (!sink->param.virtual_para.mem_size) {
        return FALSE;
    }

    if (transform != NULL && src_addr == NULL) {
        callback_ptr = DSP_Callback_Get(transform->source, sink);
        src_addr = callback_ptr->EntryPara.out_ptr[0];
    }

    if (length <= sink->param.virtual_para.mem_size) {
        memcpy(sink->streamBuffer.BufferInfo.startaddr[0],
               src_addr,
               length);
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL SinkConfigureVirtualSink(SINK sink, stream_config_type type, U32 value)
{
    return Sink_VirtualSink_Configuration(sink, type, value);
}


VOID Sink_VirtualSink_Interface_Init(SINK sink)
{
    /* interface init */
    sink->sif.SinkSlack         = SinkSlackVirtualSink;
    sink->sif.SinkFlush         = SinkFlushVirtualSink;
    sink->sif.SinkConfigure     = SinkConfigureVirtualSink;
    sink->sif.SinkClose         = SinkCloseVirtualSink;
    sink->sif.SinkClaim         = SinkClaimVirtualSink;
    sink->sif.SinkMap           = SinkMapVirtualSink;
    sink->sif.SinkWriteBuf      = SinkWriteBufVirtualSink;
}

U32 SourceSizeVirtualSource(SOURCE source)
{
    if(source->param.virtual_para.is_processed == false){
        DSP_MW_LOG_D("SourceSizeVirtualSource :%d",1,source->param.virtual_para.mem_size);
        return (source->param.virtual_para.mem_size);
    } else {
        DSP_MW_LOG_D("SourceSizeVirtualSource :%d is_processed",1,0);
        return 0;
    }
}

#include "dsp_dump.h"
BOOL SourceReadBufVirtualSource(SOURCE source, U8 *dst_addr, U32 length)
{
    TRANSFORM transform = source->transform;
    DSP_CALLBACK_PTR callback_ptr;
    DSP_MW_LOG_D("SourceReadBufVirtualSource mem_size:%d, data_size %d, data_samplingrate %d is_processed %d, is_dummy_data %d", 5,
                    source->param.virtual_para.mem_size,
                    source->param.virtual_para.data_size,
                    source->param.virtual_para.data_samplingrate,
                    source->param.virtual_para.is_processed,
                    source->param.virtual_para.is_dummy_data);
    if (!source->param.virtual_para.mem_size) {
        return FALSE;
    }

    if (transform != NULL) {
        callback_ptr = DSP_Callback_Get(source, transform->sink);
        if (length <= source->param.virtual_para.mem_size) {
            for(uint8_t i=0; i<callback_ptr->EntryPara.in_channel_num; i++){
                dst_addr = callback_ptr->EntryPara.out_ptr[i];
                //DSP_MW_LOG_I("SourceReadBufVirtualSource :0x%x, 0x%x, %d", 3, dst_addr, source->streamBuffer.BufferInfo.startaddr[i],length);
                if (dst_addr != NULL) {
                    if ((source->streamBuffer.BufferInfo.startaddr[i] == NULL) || (source->param.virtual_para.is_dummy_data == true)) {
                        memset(dst_addr, 0, length);
                    } else {
                        memcpy(dst_addr,
                        source->streamBuffer.BufferInfo.startaddr[i],
                        source->param.virtual_para.data_size);
                    }
                }
                if(i == 0){
                    //LOG_AUDIO_DUMP(source->streamBuffer.BufferInfo.startaddr[i], source->param.virtual_para.data_size, AUDIO_DUMP_TEST_ID_3);
                }
            }
            source->param.virtual_para.is_processed = true;  //data used
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
}

U8 *SourceMapVirtualSource(SOURCE source)
{
    return (U8 *)source->streamBuffer.BufferInfo.startaddr[0];
}

BOOL SourceConfigureVirtualSource(SOURCE source, stream_config_type type, U32 value)
{
    UNUSED(source);
    UNUSED(type);
    UNUSED(value);
    return TRUE;
}

VOID SourceDropVirtualSource(SOURCE source, U32 amount)
{
    UNUSED(source);
    UNUSED(amount);
    source->param.virtual_para.is_dummy_data = true; //data used, if no data coming, next data will be dummy 0.
}

BOOL SourceCloseVirtualSource(SOURCE source)
{
    for(uint8_t i=0; i<BUFFER_INFO_CH_NUM; i++){
        if (source->streamBuffer.BufferInfo.startaddr[i]) {
            vPortFree(source->streamBuffer.BufferInfo.startaddr[i]);
            source->streamBuffer.BufferInfo.startaddr[i] = NULL;
        }
    }
#if defined(AIR_WIRED_AUDIO_ENABLE)
    if (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_OUT_IEM){
        extern void wired_audio_usb_in_out_source_close(SOURCE source);
        wired_audio_usb_in_out_source_close(source);
    } else if(source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM) {
        extern void wired_audio_main_stream_source_close(SOURCE source);
        wired_audio_main_stream_source_close(source);
    }
#endif
    return true;
}
VOID Source_VirtualSource_Interface_Init(SOURCE source)
{
    /* interface init */
    source->sif.SourceSize        = SourceSizeVirtualSource;
    source->sif.SourceReadBuf     = SourceReadBufVirtualSource;
    source->sif.SourceMap         = SourceMapVirtualSource;
    source->sif.SourceConfigure   = SourceConfigureVirtualSource;
    source->sif.SourceDrop        = SourceDropVirtualSource;
    source->sif.SourceClose       = SourceCloseVirtualSource;
}

