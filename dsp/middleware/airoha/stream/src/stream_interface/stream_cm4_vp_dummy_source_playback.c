/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

#ifdef AIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE

#include "types.h"
#include "source_inter.h"
#include "dsp_buffer.h"
#include "stream_cm4_vp_dummy_source_playback.h"
#include "dsp_callback.h"
#include "dsp_temp.h"
#include "dsp_dump.h"

#define CM4_VP_PLAYBACK_PCM        0 // Yo: should use AUDIO_DSP_CODEC_TYPE_PCM to sync with MCU
#define GET_HW_SEM_RETRY_TIMES  10000
#ifndef UNUSED
#define UNUSED(p) ((void)(p))
#endif

EXTERN BOOL SourceReadBuf_CM4_playback(SOURCE source, U8 * dst_addr, U32 length);

static volatile cm4_vp_dummy_source_playback_pcm_ctrl_blk_t CM4_Vp_Dummy_Source_PlaybackCtrl = {
    .data_index  = 0x0,
    .stream_mode = DUMMY_SOURCE_MODE_LOOP,
    .frame_size  = 2048,
};

static volatile uint32_t int_mask;

VOID CB_CM4_VP_DUMMY_SOURCE_PLAYBACK_DATA_REQ_ACK(VOID)
{
    //CM4_Vp_Dummy_Source_PlaybackCtrl.data_request_signal = 0;
}


static VOID cm4_vp_dummy_source_playback_send_data_request(VOID)
{
    hal_ccni_message_t msg;
    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    msg.ccni_message[0] = MSG_DSP2MCU_PROMPT_DATA_REQUEST << 16;
    aud_msg_tx_handler(msg, 0, TRUE);
}


static VOID cm4_vp_dummy_source_playback_parameter_initialization(VOID)
{
    //CM4_Vp_Dummy_Source_PlaybackCtrl.stream_mode = 0;
}


ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static VOID cm4_vp_dummy_source_playback_hardware_semaphore_take(VOID) // Similar to StreamDSP_HWSemaphoreTake, may consider to combine
{
    uint32_t take_times = 0;

    while (++take_times) {
        hal_nvic_save_and_set_interrupt_mask(&int_mask);

        if (HAL_HW_SEMAPHORE_STATUS_OK == hal_hw_semaphore_take(HW_SEMAPHORE_AUDIO_CM4_DSP0_PLAYBACK)) {
            break;
        }

        if (take_times > GET_HW_SEM_RETRY_TIMES) {
            hal_nvic_restore_interrupt_mask(int_mask);

            //error handling
            DSP_MW_LOG_I("[CM4_VP_DUMMY] Can not take HW Semaphore", 0);
            AUDIO_ASSERT(0);
        }

        //vTaskDelay(2/portTICK_PERIOD_MS);

        hal_nvic_restore_interrupt_mask(int_mask);
    }
}


static VOID cm4_vp_dummy_source_playback_hardware_semaphore_give(VOID) // Similar to StreamDSP_HWSemaphoreGive, may consider to combine
{
    if (HAL_HW_SEMAPHORE_STATUS_OK == hal_hw_semaphore_give(HW_SEMAPHORE_AUDIO_CM4_DSP0_PLAYBACK)) {
        hal_nvic_restore_interrupt_mask(int_mask);
    } else {
        hal_nvic_restore_interrupt_mask(int_mask);

        //error handling
        DSP_MW_LOG_I("[CM4_VP_DUMMY] Can not give HW Semaphore", 0);
        AUDIO_ASSERT(0);
    }
}


static VOID cm4_vp_dummy_source_playback_update_from_share_information(SOURCE source)
{

    n9_dsp_share_info_t *ptr = (n9_dsp_share_info_t *)source->param.cm4_playback.info.share_info_base_addr;

    //cm4_vp_dummy_source_playback_hardware_semaphore_take();

    /* Put your code here, if the semaphore is taken, do something for the critical resource...... */
    source->streamBuffer.ShareBufferInfo.startaddr = ptr->startaddr;
    source->streamBuffer.ShareBufferInfo.WriteOffset = ptr->WriteOffset;
    source->streamBuffer.ShareBufferInfo.length = ptr->length;
    source->streamBuffer.ShareBufferInfo.bBufferIsFull = ptr->bBufferIsFull;

    //cm4_vp_dummy_source_playback_hardware_semaphore_give();
}


static VOID cm4_vp_dummy_source_playback_update_to_share_information(SOURCE source)
{
    n9_dsp_share_info_t *ptr = (n9_dsp_share_info_t *)source->param.cm4_playback.info.share_info_base_addr;

    //cm4_vp_dummy_source_playback_hardware_semaphore_take();

    /* Put your code here, if the semaphore is taken, do something for the critical resource...... */
    ptr->ReadOffset = source->streamBuffer.ShareBufferInfo.ReadOffset;
    ptr->bBufferIsFull = source->streamBuffer.ShareBufferInfo.bBufferIsFull;

    //cm4_vp_dummy_source_playback_hardware_semaphore_give();
}
#if 1 /*32 bit mono data table for demo.*/
const uint32_t DUMMY_tone_test[] = {  //500Hz mono, 1000Hz Stereo
#if 1 //*8096
0x00000000, 0x00000000, 0xFEF743A0, 0xFEF743A0, 0xFDF2F9C0, 0xFDF2F9C0, 0xFCF794E0, 0xFCF794E0, 0xFC098780, 0xFC098780, 0xFB2CE540, 0xFB2CE540, 0xFA6562E0, 0xFA6562E0, 0xF9B65640, 0xF9B65640, 0xF9231540, 0xF9231540, 0xF8ADB980, 0xF8ADB980, 0xF8585CA0, 0xF8585CA0, 0xF82499C0, 0xF82499C0, 0xF8134E40, 0xF8134E40, 0xF82499C0, 0xF82499C0, 0xF8585CA0, 0xF8585CA0, 0xF8ADB980, 0xF8ADB980, 0xF9231540, 0xF9231540, 0xF9B65640, 0xF9B65640, 0xFA6562E0, 0xFA6562E0, 0xFB2CE540, 0xFB2CE540, 0xFC098780, 0xFC098780, 0xFCF794E0, 0xFCF794E0, 0xFDF2F9C0, 0xFDF2F9C0, 0xFEF743A0, 0xFEF743A0,
0x00000000, 0x00000000, 0x0108DC00, 0x0108DC00, 0x020D25E0, 0x020D25E0, 0x03084B80, 0x03084B80, 0x03F67880, 0x03F67880, 0x04D2FB20, 0x04D2FB20, 0x059A7D80, 0x059A7D80, 0x06496A80, 0x06496A80, 0x06DCEAC0, 0x06DCEAC0, 0x07526620, 0x07526620, 0x07A7A360, 0x07A7A360, 0x07DB6640, 0x07DB6640, 0x07ECD160, 0x07ECD160, 0x07DB6640, 0x07DB6640, 0x07A7A360, 0x07A7A360, 0x07526620, 0x07526620, 0x06DCEAC0, 0x06DCEAC0, 0x06496A80, 0x06496A80, 0x059A7D80, 0x059A7D80, 0x04D2FB20, 0x04D2FB20, 0x03F67880, 0x03F67880, 0x03084B80, 0x03084B80, 0x020D25E0, 0x020D25E0, 0x0108DC00, 0x0108DC00,
#else
0x00000000, 0x00000000, 0xFFFC0000, 0xFFFC0000, 0xFFF80000, 0xFFF80000, 0xFFF40000, 0xFFF40000, 0xFFF00000, 0xFFF00000, 0xFFED0000, 0xFFED0000, 0xFFEA0000, 0xFFEA0000, 0xFFE70000, 0xFFE70000, 0xFFE50000, 0xFFE50000, 0xFFE30000, 0xFFE30000, 0xFFE20000, 0xFFE20000, 0xFFE10000, 0xFFE10000, 0xFFE00000, 0xFFE00000, 0xFFE10000, 0xFFE10000, 0xFFE20000, 0xFFE20000, 0xFFE30000, 0xFFE30000, 0xFFE50000, 0xFFE50000, 0xFFE70000, 0xFFE70000, 0xFFEA0000, 0xFFEA0000, 0xFFED0000, 0xFFED0000, 0xFFF00000, 0xFFF00000, 0xFFF40000, 0xFFF40000, 0xFFF80000, 0xFFF80000, 0xFFFC0000, 0xFFFC0000,
0x00000000, 0x00000000, 0x00040000, 0x00040000, 0x00080000, 0x00080000, 0x000C0000, 0x000C0000, 0x00100000, 0x00100000, 0x00130000, 0x00130000, 0x00160000, 0x00160000, 0x00190000, 0x00190000, 0x001B0000, 0x001B0000, 0x001D0000, 0x001D0000, 0x001E0000, 0x001E0000, 0x001F0000, 0x001F0000, 0x00200000, 0x00200000, 0x001F0000, 0x001F0000, 0x001E0000, 0x001E0000, 0x001D0000, 0x001D0000, 0x001B0000, 0x001B0000, 0x00190000, 0x00190000, 0x00160000, 0x00160000, 0x00130000, 0x00130000, 0x00100000, 0x00100000, 0x000C0000, 0x000C0000, 0x00080000, 0x00080000, 0x00040000, 0x00040000,
#endif
#else /*16 bit mono data table for demo.*/
const uint16_t DUMMY_tone_test[] = {
    0x0000, 0x0000, 0xF7A1, 0xF7A1, 0xEF66, 0xEF66, 0xE773, 0xE773, 0xDFEC, 0xDFEC, 0xD8F2, 0xD8F2, 0xD2A3, 0xD2A3, 0xCD1A, 0xCD1A, 0xC872, 0xC872, 0xC4BC, 0xC4BC, 0xC209, 0xC209, 0xC066, 0xC066, 0xBFDA, 0xBFDA, 0xC067, 0xC067, 0xC209, 0xC209, 0xC4BC, 0xC4BC, 0xC872, 0xC872, 0xCD1B, 0xCD1B, 0xD2A4, 0xD2A4, 0xD8F3, 0xD8F3, 0xDFEC, 0xDFEC, 0xE773, 0xE773, 0xEF65, 0xEF65, 0xF7A0, 0xF7A0,
    0x0001, 0x0001, 0x0860, 0x0860, 0x109B, 0x109B, 0x188C, 0x188C, 0x2014, 0x2014, 0x270D, 0x270D, 0x2D5C, 0x2D5C, 0x32E4, 0x32E4, 0x378E, 0x378E, 0x3B45, 0x3B45, 0x3DF7, 0x3DF7, 0x3F9A, 0x3F9A, 0x4027, 0x4027, 0x3F9A, 0x3F9A, 0x3DF7, 0x3DF7, 0x3B45, 0x3B45, 0x378E, 0x378E, 0x32E5, 0x32E5, 0x2D5D, 0x2D5D, 0x270E, 0x270E, 0x2013, 0x2013, 0x188C, 0x188C, 0x109B, 0x109B, 0x085F, 0x085F,
#endif
};

void cm4_vp_dummy_source_playback_set_param(uint8_t mode, uint8_t index)
{
    CM4_Vp_Dummy_Source_PlaybackCtrl.stream_mode = mode;
    CM4_Vp_Dummy_Source_PlaybackCtrl.data_index = index;
    DSP_MW_LOG_I("[CM4_VP_DUMMY] Stream mode :%d data_index:%d", 2, CM4_Vp_Dummy_Source_PlaybackCtrl.stream_mode, CM4_Vp_Dummy_Source_PlaybackCtrl.data_index);
}

U16 cm4_vp_dummy_source_playback_return_share_information_size(uint8_t index)
{
    return sizeof(DUMMY_tone_test);
}

U32 cm4_vp_dummy_source_playback_return_share_information_address(uint8_t index)
{
    return DUMMY_tone_test;
}

n9_dsp_share_info_t DUMMY_buff_info;

U32 SourceSize_CM4_vp_dummy_source_playback(SOURCE source)
{
    n9_dsp_share_info_t *share_buff_info  = &(source->streamBuffer.ShareBufferInfo);
    CM4_PLAYBACK_PARAMETER *cm4_playback_param = &(source->param.cm4_playback);
    U8 source_channels = cm4_playback_param->info.source_channels;

    /* update share information data */
    cm4_vp_dummy_source_playback_update_from_share_information(source);

    // DSP_MW_LOG_I("[CM4_VP_DUMMY] Return Fixed size.", 0);
    share_buff_info->bBufferIsFull = true;

    xTaskResumeFromISR(source->taskId);
    portYIELD_FROM_ISR(pdTRUE); // force to do context switch
    return CM4_Vp_Dummy_Source_PlaybackCtrl.frame_size;

#if 0
    if (cm4_playback_param->remain_bs_size <= CM4_Vp_Dummy_Source_PlaybackCtrl.data_request_threshold) {

        if (CM4_Vp_Dummy_Source_PlaybackCtrl.data_request_signal == 0) {

            CM4_Vp_Dummy_Source_PlaybackCtrl.data_request_signal = 1;

            vp_data_request_flag = 1;

            xTaskResumeFromISR(pDHP_TaskHandler);
            portYIELD_FROM_ISR(); // force to do context switch
        }
    }
    /* PCM part */
    if (cm4_playback_param->info.codec_type == CM4_VP_PLAYBACK_PCM) {
        if (cm4_playback_param->remain_bs_size >= source_channels * CM4_Vp_Dummy_Source_PlaybackCtrl.frame_size) {
            return CM4_Vp_Dummy_Source_PlaybackCtrl.frame_size;
        } else {
            DSP_MW_LOG_I("[CM4_VP_DUMMY] Not enough bitstream", 0);
            return 0;
        }
    } else {
        /* Not support codec type */
        DSP_MW_LOG_I("[CM4_VP_DUMMY] Not support codec type", 0);
        return 0;
    }
#endif
}


VOID SourceDrop_CM4_vp_dummy_source_playback(SOURCE source, U32 amount)
{
    n9_dsp_share_info_t *share_buff_info = &(source->streamBuffer.ShareBufferInfo);
    CM4_PLAYBACK_PARAMETER *cm4_playback_param = &(source->param.cm4_playback);
    U8 source_channels = cm4_playback_param->info.source_channels;

    if ((amount == CM4_Vp_Dummy_Source_PlaybackCtrl.frame_size)) {

        amount = amount * source_channels;

        if (share_buff_info->bBufferIsFull == 1) {
            share_buff_info->bBufferIsFull = 0;
        }

        share_buff_info->ReadOffset += amount;

        if (share_buff_info->ReadOffset >= share_buff_info->length) {
            share_buff_info->ReadOffset -= share_buff_info->length;
        }

        //cm4_playback_param->remain_bs_size -= amount;
        //DSP_MW_LOG_I("[Rdebug] SourceDrop amount == CM4_Vp_Dummy_Source_PlaybackCtrl.frame_size.", 0);

        cm4_vp_dummy_source_playback_update_to_share_information(source);
    } else {
        DSP_MW_LOG_I("[Rdebug] SourceDrop amount != CM4_Vp_Dummy_Source_PlaybackCtrl.frame_size. (%d)(%d)", 2, amount, CM4_Vp_Dummy_Source_PlaybackCtrl.frame_size);
        /* TBD */
    }
}


U8 * SourceMap_CM4_vp_dummy_source_playback(SOURCE source)
{
    UNUSED(source);
    return MapAddr;
}


BOOL SourceConfigure_CM4_vp_dummy_source_playback(SOURCE source, stream_config_type type, U32 value)
{
    UNUSED(source);
    UNUSED(type);
    UNUSED(value);
    return TRUE;
}


BOOL SourceClose_CM4_vp_dummy_source_playback(SOURCE source)
{
    UNUSED(source);
    return TRUE;
}


VOID SourceInit_CM4_vp_dummy_source_playback(SOURCE source)
{
    /* buffer init */
    source->type = SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK;
    source->buftype = BUFFER_TYPE_CIRCULAR_BUFFER;

    cm4_vp_dummy_source_playback_parameter_initialization();

    /* interface init */
    source->sif.SourceSize        = SourceSize_CM4_vp_dummy_source_playback;
    source->sif.SourceMap         = SourceMap_CM4_vp_dummy_source_playback;
    source->sif.SourceConfigure   = SourceConfigure_CM4_vp_dummy_source_playback;
    source->sif.SourceDrop        = SourceDrop_CM4_vp_dummy_source_playback;
    source->sif.SourceClose       = SourceClose_CM4_vp_dummy_source_playback;
    source->sif.SourceReadBuf     = SourceReadBuf_CM4_playback;
}
#endif /* AIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE */

