/* Copyright Statement:
*
* (C) 2022 Airoha Technology Corp. All rights reserved.
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

//#if defined(AIR_QUEUE_BUFFER_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "dsp_feature_interface.h"
#include "dsp_audio_process.h"
#include "queue_buffer_interface.h"
#include "dsp_dump.h"
#include "dsp_memory.h"
#include "memory_attribute.h"

/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
static QUE_DATA_INFO_t que_data_info[QUE_DATA_INFO_MAX] = 
{
    //  codec_type,                     que_size
    {CODEC_DECODER_SBC,     1024*4},
    {CODEC_DECODER_VENDOR,  1024*4},
    {BYPASS_QUE_TYPE,            0},
    {BYPASS_QUE_TYPE,            0},
    {BYPASS_QUE_TYPE,            0},
};
/* Private functions ---------------------------------------------------------*/

bool stream_function_queue_buffer_initialize(void *para)
{
    QUE_BUFFER_PARAM_t * que_buf_para_ptr = (QUE_BUFFER_PARAM_t *)stream_function_get_working_buffer(para);
    U8 codec_type = stream_function_get_decoder_type(para);
    U8 channels = stream_function_get_channel_number(para);
    U16 i = 0;

    for(i = 0; i < QUE_DATA_INFO_MAX;i++){
        if(que_data_info[i].codec_type == codec_type){
            que_buf_para_ptr->que_info.codec_type = que_data_info[i].codec_type;
            que_buf_para_ptr->que_info.que_size = que_data_info[i].que_size;
            que_buf_para_ptr->enable = true;
            break;
        }
    }

    DSP_MW_LOG_I("[QB]Init, codec_type:0x%x, que_size:%d, ch:%d",3,que_data_info[i].codec_type,que_data_info[i].que_size,channels);

    for(i = 0; i < channels; i++){
        que_buf_para_ptr->buff_ptr[i] = stream_function_get_inout_buffer(para,i+1);
        DSP_MW_LOG_I("[QB]init, channel:%d, buf_addr:0x%x",2,i,stream_function_get_inout_buffer(para,i+1));
    }

    return FALSE;
}

bool stream_function_queue_buffer_process(void *para)
{
    QUE_BUFFER_PARAM_t * que_buf_para_ptr = (QUE_BUFFER_PARAM_t *)stream_function_get_working_buffer(para);
    U16 frame_size = stream_function_get_output_size(para);
    U16 output_size = 0;

    if(!que_buf_para_ptr->enable || !frame_size){
        return FALSE;
    }

    if(que_buf_para_ptr->que_info.que_size % frame_size != 0){
        DSP_MW_LOG_E("[QB]This feature only supports integer multiple",0);
        AUDIO_ASSERT(0);
    }

    que_buf_para_ptr->data_size += frame_size;

    if(que_buf_para_ptr->data_size == que_buf_para_ptr->que_info.que_size){
        output_size = que_buf_para_ptr->que_info.que_size;
        que_buf_para_ptr->data_size = 0;
    }

    stream_function_modify_buffer_offset(para, que_buf_para_ptr->data_size);

    stream_function_modify_output_size(para,output_size);

    return FALSE;
}
//#endif /* AIR_QUEUE_BUFFER_ENABLE */
