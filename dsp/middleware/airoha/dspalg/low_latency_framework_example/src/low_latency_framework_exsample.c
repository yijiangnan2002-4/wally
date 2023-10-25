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
#include "low_latency_framework_exsample.h"
#include "dsp_temp.h"
#include "hal_audio_message_struct_common.h"
#include "dllt.h"
#include "dsp_dump.h"


/* Public define -------------------------------------------------------------*/
#define LLF_EXAMPLE_DUMP_ENABLE

/* Public typedef ------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
stream_feature_list_t stream_feature_list_psap_sample[] = {
    CODEC_PCM_COPY,
    FUNC_LLF_SAMPLE,
    FUNC_END,
};

void *psap_sample_instance_p;

/* Private functions ----------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
stream_feature_list_t* llf_example_features_get_list(llf_type_t scenario_id, U32 sub_id)
{
    UNUSED(scenario_id);
    UNUSED(sub_id);
    return stream_feature_list_psap_sample;
}

bool stream_function_llf_example_initialize(void *para)
{
    psap_sample_instance_p = stream_function_get_working_buffer(para); /*size = PSAP_SAMPLE_MEM_SIZE*/
    if (psap_sample_instance_p == NULL) {
        AUDIO_ASSERT(0 && "[PSAP][SAMPLE]working_buffer NULL");
        return true;
    }
    DSP_LLF_LOG_I("[SAMPLE] initialize done", 0);
    return false;
}

ATTR_TEXT_IN_IRAM bool stream_function_llf_example_process(void *para)
{
    stream_resolution_t resolution = stream_function_get_output_resolution(para);/*0:RESOLUTION_16BIT; 1:RESOLUTION_32BIT*/
    uint32_t in_frame_size = stream_function_get_output_size(para);
    uint32_t channel_number = stream_function_get_channel_number(para);

    S32 *InBuf_rear        = (S32 *)stream_function_get_inout_buffer(para, dsp_llf_get_data_buf_idx(LLF_DATA_TYPE_REAR_L));
    S32 *InBuf_in_ear      = (S32 *)stream_function_get_inout_buffer(para, dsp_llf_get_data_buf_idx(LLF_DATA_TYPE_INEAR_L));
    S32 *InBuf_front       = (S32 *)stream_function_get_inout_buffer(para, dsp_llf_get_data_buf_idx(LLF_DATA_TYPE_TALK));
    S32 *InBuf_music_voice = (S32 *)stream_function_get_inout_buffer(para, dsp_llf_get_data_buf_idx(LLF_DATA_TYPE_MUSIC_VOICE));
    S32 *InBuf_echo_ref    = (S32 *)stream_function_get_inout_buffer(para, dsp_llf_get_data_buf_idx(LLF_DATA_TYPE_REF));
    S32 *OutBuf            = (S32 *)stream_function_get_inout_buffer(para, 1);/*Please put the output data in channel 1*/

#if defined(AIR_AUDIO_DUMP_ENABLE) && defined(LLF_EXAMPLE_DUMP_ENABLE)/*Disable dump could reduce duration*/
    if (InBuf_rear)
        LOG_AUDIO_DUMP((U8*)InBuf_rear, in_frame_size, AUDIO_PSAP_IN_0);//You could change DUMP ID for you
    if (InBuf_in_ear)
        LOG_AUDIO_DUMP((U8*)InBuf_in_ear, in_frame_size, AUDIO_PSAP_IN_1);
    if (InBuf_front)
        LOG_AUDIO_DUMP((U8*)InBuf_front, in_frame_size, AUDIO_PSAP_IN_2);
    if (InBuf_music_voice)
        LOG_AUDIO_DUMP((U8*)InBuf_music_voice, in_frame_size, AUDIO_PSAP_MUSIC_VOICE);
    if (InBuf_echo_ref)
        LOG_AUDIO_DUMP((U8*)InBuf_echo_ref, in_frame_size, AUDIO_PSAP_ECHO_REF);
#else
    UNUSED(in_frame_size);
#endif

    /*========== TODO: LLF ALGORITHM ==========

               put the output data in OutBuf

            ===========================================*/

#if defined(AIR_AUDIO_DUMP_ENABLE) && defined(LLF_EXAMPLE_DUMP_ENABLE)
    LOG_AUDIO_DUMP((U8*)OutBuf, in_frame_size, AUDIO_PSAP_OUT_L);
#endif
    UNUSED(resolution);
    UNUSED(channel_number);

//    DSP_LLF_LOG_I("[SAMPLE] res:%d, frame_size:%d(bytes), ch_num:%d, buff(0x%x 0x%x 0x%x 0x%x 0x%x)", 8, resolution, in_frame_size, channel_number, InBuf_rear, InBuf_in_ear, InBuf_front, InBuf_music_voice, InBuf_echo_ref);
    return false;
}

