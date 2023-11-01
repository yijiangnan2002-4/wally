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
#include "source_inter.h"
#include "dsp_buffer.h"
#include "dsp_memory.h"
#include "stream_cm4_playback.h"
#include "dsp_callback.h"
#include "dsp_temp.h"
#include "dsp_dump.h"
#include "dsp_audio_process.h"

#define CM4_PLAYBACK_PCM        0 // Yo: should use AUDIO_DSP_CODEC_TYPE_PCM to sync with MCU
#define GET_HW_SEM_RETRY_TIMES  10000
#ifndef UNUSED
#define UNUSED(p) ((void)(p))
#endif

static volatile cm4_playback_pcm_ctrl_blk_t CM4_PlaybackCtrl = {
    .data_request_threshold = 2048,
    .data_request_signal = 0,
    .frame_size = 128,
};

static volatile uint32_t int_mask;

VOID CB_CM4_PLAYBACK_DATA_REQ_ACK(VOID)
{
    CM4_PlaybackCtrl.data_request_signal = 0;
}


static VOID cm4_playback_send_data_request(VOID)
{
    hal_ccni_message_t msg;
    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    msg.ccni_message[0] = MSG_DSP2MCU_PLAYBACK_DATA_REQUEST << 16;
    aud_msg_tx_handler(msg, 0, FALSE);
}


static VOID cm4_playback_parameter_initialization(VOID)
{
    CM4_PlaybackCtrl.data_request_signal = 0;
}


static VOID cm4_playback_hardware_semaphore_take(VOID) // Similar to StreamDSP_HWSemaphoreTake, may consider to combine
{
    uint32_t take_times = 0;

    while (++take_times) {
        hal_nvic_save_and_set_interrupt_mask((uint32_t *)&int_mask);

        if (HAL_HW_SEMAPHORE_STATUS_OK == hal_hw_semaphore_take(HW_SEMAPHORE_AUDIO_CM4_DSP0_PLAYBACK)) {
            break;
        }

        if (take_times > GET_HW_SEM_RETRY_TIMES) {
            hal_nvic_restore_interrupt_mask(int_mask);

            //error handling
            DSP_MW_LOG_I("[CM4_PB] Can not take HW Semaphore", 0);
            AUDIO_ASSERT(0);
        }

        //vTaskDelay(2/portTICK_PERIOD_MS);

        hal_nvic_restore_interrupt_mask(int_mask);
    }
}


static VOID cm4_playback_hardware_semaphore_give(VOID) // Similar to StreamDSP_HWSemaphoreGive, may consider to combine
{
    if (HAL_HW_SEMAPHORE_STATUS_OK == hal_hw_semaphore_give(HW_SEMAPHORE_AUDIO_CM4_DSP0_PLAYBACK)) {
        hal_nvic_restore_interrupt_mask(int_mask);
    } else {
        hal_nvic_restore_interrupt_mask(int_mask);

        //error handling
        DSP_MW_LOG_I("[CM4_PB] Can not give HW Semaphore", 0);
        AUDIO_ASSERT(0);
    }
}


static VOID cm4_playback_update_from_share_information(SOURCE source)
{

    n9_dsp_share_info_t *ptr = (n9_dsp_share_info_t *)source->param.cm4_playback.info.share_info_base_addr;

    cm4_playback_hardware_semaphore_take();

    /* Put your code here, if the semaphore is taken, do something for the critical resource...... */
    source->streamBuffer.ShareBufferInfo.start_addr = hal_memview_cm4_to_dsp0(ptr->start_addr);
    source->streamBuffer.ShareBufferInfo.write_offset = ptr->write_offset;
    source->streamBuffer.ShareBufferInfo.length = ptr->length;
    source->streamBuffer.ShareBufferInfo.bBufferIsFull = ptr->bBufferIsFull;

    cm4_playback_hardware_semaphore_give();
}


static VOID cm4_playback_update_to_share_information(SOURCE source)
{
    n9_dsp_share_info_t *ptr = (n9_dsp_share_info_t *)source->param.cm4_playback.info.share_info_base_addr;

    cm4_playback_hardware_semaphore_take();

    /* Put your code here, if the semaphore is taken, do something for the critical resource...... */
    ptr->read_offset = source->streamBuffer.ShareBufferInfo.read_offset;
    ptr->bBufferIsFull = source->streamBuffer.ShareBufferInfo.bBufferIsFull;

    cm4_playback_hardware_semaphore_give();
}

BOOL SourceReadBuf_CM4_playback(SOURCE source, U8 * dst_addr, U32 length)
{
    TRANSFORM transform =  source->transform;
    n9_dsp_share_info_t *share_buff_info  = &(source->streamBuffer.ShareBufferInfo);

    DSP_CALLBACK_PTR callback_ptr = NULL;
    audio_channel_number_t channel = source->param.cm4_playback.info.channel_number;
    audio_bits_per_sample_t bit_type = source->param.cm4_playback.info.bit_type;
    U8 source_channels = source->param.cm4_playback.info.source_channels;

    if (transform != NULL) {
        callback_ptr = DSP_Callback_Get(source, transform->sink);
        if (callback_ptr == NULL) {
            //error handling
            DSP_MW_LOG_I("[CM4_PB] SourceReadBuf_CM4_vp_playback : Can not get DSP_Callback", 0);
            return FALSE;
        } else {
            if (source->type == SOURCE_TYPE_CM4_PLAYBACK){
                dst_addr = callback_ptr->EntryPara.in_ptr[0];
            } else {
                dst_addr = (2 == source_channels) ? callback_ptr->EntryPara.out_ptr[0] : callback_ptr->EntryPara.in_ptr[0];
            }
        }
    } else {
        DSP_MW_LOG_E("[CM4_PB]transform NULL.", 0);
        return FALSE;
    }

    DSP_C2D_BufferCopy((VOID *)  dst_addr,
                       (VOID *)(share_buff_info->start_addr + share_buff_info->read_offset),
                       (U16)    length * source_channels,
                       (VOID *)  share_buff_info->start_addr,
                       (U16)    share_buff_info->length);

    if (AUDIO_BITS_PER_SAMPLING_24 == bit_type) {
        U32 *dst_addr1 = NULL;
        U32 *dst_addr0 = NULL;
        U32 *src_temp_addr = NULL;

        /* Use channel 0 as temp buffer */
        src_temp_addr = (U32 *)dst_addr;

        if (2 == source_channels) {

            dst_addr0 = callback_ptr->EntryPara.in_ptr[0];
            dst_addr1 = callback_ptr->EntryPara.in_ptr[1];

            switch (channel) {
                case AUDIO_STEREO:
                    DSP_I2D_BufferCopy_32bit_mute(src_temp_addr, dst_addr0, dst_addr1, (U16)(length >> 2), false);
                    break;

                case AUDIO_STEREO_BOTH_L_CHANNEL:
                    DSP_I2D_BufferCopy_32bit_mute(src_temp_addr, dst_addr0, (U32 *)NULL, (U16)length, false);
                    DSP_I2D_BufferCopy_32bit_mute(src_temp_addr, dst_addr1, (U32 *)NULL, (U16)length, false);
                    break;

                case AUDIO_STEREO_BOTH_R_CHANNEL:
                    DSP_I2D_BufferCopy_32bit_mute(src_temp_addr, (U32 *)NULL, dst_addr0, (U16)length, false);
                    DSP_I2D_BufferCopy_32bit_mute(src_temp_addr, (U32 *)NULL, dst_addr1, (U16)length, false);
                    break;

                case AUDIO_STEREO_BOTH_L_R_SWAP:
                    DSP_I2D_BufferCopy_32bit_mute(src_temp_addr, dst_addr1, dst_addr0, (U16)length, false);
                    break;

                case AUDIO_MONO:
                default:
                    AUDIO_ASSERT(FALSE);
                    break;
            }
        }
#ifdef AIR_AUDIO_DUMP_ENABLE
        if (length != 0) {
            if (source->type == SOURCE_TYPE_CM4_VP_PLAYBACK){
                LOG_AUDIO_DUMP((U8 *)dst_addr0, (U32)length, PROMPT_VP_PATTERN);
#ifdef AIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
            } else if (source->type == SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK) {
                LOG_AUDIO_DUMP((U8 *)dst_addr0, (U32)length, PROMPT_RT_PATTERN);
#endif
            }
        }
#endif
    } else {
        U16 *dst_addr1 = NULL;
        U16 *dst_addr0 = NULL;
        U16 *src_temp_addr = NULL;

        /* Use channel 0 as temp buffer */
        src_temp_addr = (U16 *)dst_addr;

        if (2 == source_channels) {

            dst_addr0 = callback_ptr->EntryPara.in_ptr[0];
            dst_addr1 = callback_ptr->EntryPara.in_ptr[1];

            switch (channel) {
                case AUDIO_STEREO:
                    DSP_I2D_BufferCopy_16bit_mute(src_temp_addr, dst_addr0, dst_addr1, (U16)(length >> 1), false);
                    break;

                case AUDIO_STEREO_BOTH_L_CHANNEL:
                    DSP_I2D_BufferCopy_16bit_mute(src_temp_addr, dst_addr0, (U16 *)NULL, (U16)length, false);
                    DSP_I2D_BufferCopy_16bit_mute(src_temp_addr, dst_addr1, (U16 *)NULL, (U16)length, false);
                    break;

                case AUDIO_STEREO_BOTH_R_CHANNEL:
                    DSP_I2D_BufferCopy_16bit_mute(src_temp_addr, (U16 *)NULL, dst_addr0, (U16)length, false);
                    DSP_I2D_BufferCopy_16bit_mute(src_temp_addr, (U16 *)NULL, dst_addr1, (U16)length, false);
                    break;

                case AUDIO_STEREO_BOTH_L_R_SWAP:
                    DSP_I2D_BufferCopy_16bit_mute(src_temp_addr, dst_addr1, dst_addr0, (U16)length, false);
                    break;

                case AUDIO_MONO:
                default:
                    AUDIO_ASSERT(FALSE);
                    break;
            }
        }
#ifdef AIR_AUDIO_DUMP_ENABLE
        if (length != 0) {
            if (source->type == SOURCE_TYPE_CM4_VP_PLAYBACK){
                LOG_AUDIO_DUMP((U8 *)dst_addr0, (U32)length, PROMPT_VP_PATTERN);
#ifdef AIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
            } else if (source->type == SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK) {
                LOG_AUDIO_DUMP((U8 *)dst_addr0, (U32)length, PROMPT_RT_PATTERN);
#endif
            }
        }
#endif
    }
    return TRUE;
}


U32 SourceSize_CM4_playback(SOURCE source)
{
    n9_dsp_share_info_t *share_buff_info  = &(source->streamBuffer.ShareBufferInfo);
    CM4_PLAYBACK_PARAMETER *cm4_playback_param = &(source->param.cm4_playback);
    U8 source_channels = cm4_playback_param->info.source_channels;

    /* update share information data */
    cm4_playback_update_from_share_information(source);

    /* Check there is data in share buffer or not */
    if (share_buff_info->bBufferIsFull ||
        share_buff_info->read_offset != share_buff_info->write_offset) {

        if (share_buff_info->bBufferIsFull && share_buff_info->read_offset == share_buff_info->write_offset) {
            cm4_playback_param->remain_bs_size = share_buff_info->length;
        } else if (share_buff_info->write_offset > share_buff_info->read_offset) {
            cm4_playback_param->remain_bs_size = (share_buff_info->write_offset - share_buff_info->read_offset);
        } else if (share_buff_info->read_offset > share_buff_info->write_offset) {
            cm4_playback_param->remain_bs_size = share_buff_info->write_offset + (share_buff_info->length - share_buff_info->read_offset);
        }
        if (source->param.cm4_playback.data_refill_request == TRUE) {
            if (share_buff_info->bBufferIsFull == TRUE) {
                source->param.cm4_playback.data_refill_request = FALSE;
                DSP_MW_LOG_I("[CM4_PB] data_refill_request done wo: %d ro: %d", 2, share_buff_info->read_offset, share_buff_info->write_offset);
            } else {
                return 0;
            }
        }
        /* PCM part */
        if (cm4_playback_param->info.codec_type == CM4_PLAYBACK_PCM) {
            if (cm4_playback_param->remain_bs_size >= source_channels * CM4_PlaybackCtrl.frame_size) {
                return CM4_PlaybackCtrl.frame_size;
            } else {
                DSP_MW_LOG_I("[CM4_PB] Not enough bitstream", 0);
                return 0;
            }
        } else {
            /* Not support codec type */
            DSP_MW_LOG_I("[CM4_PB] Not support codec type", 0);
            return 0;
        }
    } else {
        /* No data in the buffer */
        //printf("[CM4_PB] No data in buffer");
        return 0;
    }
}


VOID SourceDrop_CM4_playback(SOURCE source, U32 amount)
{
    n9_dsp_share_info_t *share_buff_info = &(source->streamBuffer.ShareBufferInfo);
    CM4_PLAYBACK_PARAMETER *cm4_playback_param = &(source->param.cm4_playback);
    U8 source_channels = cm4_playback_param->info.source_channels;

    /* PCM part */
    if (cm4_playback_param->info.codec_type == CM4_PLAYBACK_PCM) {
        if (amount == CM4_PlaybackCtrl.frame_size) {

            amount = amount * source_channels;

            if (share_buff_info->bBufferIsFull == 1) {
                share_buff_info->bBufferIsFull = 0;
            }

            share_buff_info->read_offset += amount;

            if (share_buff_info->read_offset >= share_buff_info->length) {
                share_buff_info->read_offset -= share_buff_info->length;
            }

            cm4_playback_param->remain_bs_size -= amount;

            cm4_playback_update_to_share_information(source);

            if (cm4_playback_param->remain_bs_size <= CM4_PlaybackCtrl.data_request_threshold) {

                if (CM4_PlaybackCtrl.data_request_signal == 0) {

                    CM4_PlaybackCtrl.data_request_signal = 1;

                    cm4_playback_send_data_request();
                }
            }
        } else {
            /* TBD */
            DSP_MW_LOG_I("[CM4_PB] unexpeceted drop size :%d", 1, amount);
        }
    } else {
        //Not support codec type
        DSP_MW_LOG_I("[CM4_PB] Not support codec type", 0);
    }
}


U8 *SourceMap_CM4_playback(SOURCE source)
{
    UNUSED(source);
    return MapAddr;
}


BOOL SourceConfigure_CM4_playback(SOURCE source, stream_config_type type, U32 value)
{
    UNUSED(source);
    UNUSED(type);
    UNUSED(value);
    return TRUE;
}


BOOL SourceClose_CM4_playback(SOURCE source)
{
    UNUSED(source);
    return TRUE;
}


VOID SourceInit_CM4_playback(SOURCE source)
{
    /* buffer init */
    source->type = SOURCE_TYPE_CM4_PLAYBACK;
    source->buftype = BUFFER_TYPE_CIRCULAR_BUFFER;

    cm4_playback_parameter_initialization();

    /* interface init */
    source->sif.SourceSize        = SourceSize_CM4_playback;
    source->sif.SourceMap         = SourceMap_CM4_playback;
    source->sif.SourceConfigure   = SourceConfigure_CM4_playback;
    source->sif.SourceDrop        = SourceDrop_CM4_playback;
    source->sif.SourceClose       = SourceClose_CM4_playback;
    source->sif.SourceReadBuf     = SourceReadBuf_CM4_playback;
}

