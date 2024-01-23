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

#include "config.h"
#include "dsp_audio_process.h"
#include "dsp_feature_interface.h"
#include "dsp_callback.h"
#include "dsp_drv_dfe.h"
#include "sfr_au_src.h"
#include "dsp_buffer.h"
#include "dsp_memory.h"
#include "dsp_dump.h"
#include "stream_audio_driver.h"
#include <string.h>
#ifdef AIR_SOFTWARE_GAIN_ENABLE
#include "sw_gain_interface.h"
#endif
#include <hal_audio_message_struct_common.h>
#ifdef MTK_HWSRC_IN_STREAM
//modify for asrc
#include "hal_audio_driver.h"
#include "hal_clock_internal.h"
#include "semphr.h"
#include "hal_resource_assignment.h"

typedef BOOL (*SRC_HANDLE_ENTRY)(VOID *, VOID *, VOID *, U32);
STATIC SRC_HANDLE_ENTRY SRC_Handle_Entry;
#define HWSRC_DEFAULT_CHANNEL 2
#endif
/******************************************************************************
 * CONSTANT DEFINITIONS
 ******************************************************************************/
#define CALLBACK_SRC_FLOAT_PROTECTION 5
#define ProcessSize 512
#define MAX_CHANNELS 2

#ifdef AIR_AUDIO_HARDWARE_ENABLE
#ifdef AIR_SOFTWARE_GAIN_ENABLE
#define SW_GAIN_MUTE_VALUE (-(96 * 100))

bool g_call_mute_flag = false;
static sw_gain_port_t *g_call_sw_gain_port = NULL;
#endif
#endif

/******************************************************************************
 * Function Declaration
 ******************************************************************************/
#ifdef AIR_HFP_DNN_PATH_ENABLE
void gain_change_detect(U8 mic_map_table);
#endif


/******************************************************************************
 * Variables
 ******************************************************************************/



/**
 * @brief Copies the values of num sample from 16-bit resulation data
 * to 24-bit resulation data.
 *
 * @param des     Pointer to the destination array where the content is to be copied.
 * @param src     Pointer to the source of data to be copied.
 * @param sample  Number of sample to copy.
 */
VOID DSP_Converter_16Bit_to_24bit(
    S32 *des,
    S16 *src,
    U32  sample)
{
    U32 i = 0;

    if (des == NULL || src == NULL) {
        return;
    }
#if 0
    des += (sample - 1);
    src += (sample - 1);

    for (i = 0; i < sample; i++) {
        *des = *src;
        *des <<= 8;
        des--;
        src--;
    }
#else

    ae_p16x2s  *ae_src = (ae_p16x2s *)(src + sample);
    ae_p24x2s  *ae_des = (ae_p24x2s *)(des + sample);
    ae_p24x2s p_tmp = AE_CVTP24A16(0);

    for (i = 0; i < sample; i += 2) {
        AE_LP16X2F_IU(p_tmp, ae_src, -4);
        AE_SP24X2S_IU(p_tmp, ae_des, -8);

    }
#endif
}

/**
 * @brief Copies the values of num sample from 16-bit resulation data
 * to 32-bit resulation data.
 *
 * @param des     Pointer to the destination array where the content is to be copied.
 * @param src     Pointer to the source of data to be copied.
 * @param sample  Number of sample to copy.
 */
VOID dsp_converter_16bit_to_32bit(
    S32 *des,
    S16 *src,
    U32  sample)
{
    U32 i;

    if (des == NULL || src == NULL) {
        return;
    }

    ae_p16x2s  *ae_src = (ae_p16x2s *)(src + sample);
    ae_p24x2s  *ae_des = (ae_p24x2s *)(des + sample);
    ae_p24x2s   p_tmp = AE_CVTP24A16(0);

    for (i = 0; i < sample; i += 2) {
        AE_LP16X2F_IU(p_tmp, ae_src, -4);
        AE_SP24X2F_IU(p_tmp, ae_des, -8);

    }
}

/**
 * @brief Copies the values of num sample from 32-bit resulation data
 * to 16-bit resulation data.
 *
 * @param des     Pointer to the destination array where the content is to be copied.
 * @param src     Pointer to the source of data to be copied.
 * @param sample  Number of sample to copy.
 */
VOID dsp_converter_32bit_to_16bit(
    S16 *des,
    S32 *src,
    U32  sample)
{
    U32 i;

    if (des == NULL || src == NULL) {
        return;
    }

    ae_p24x2s  *ae_src = (ae_p24x2s *)src;
    ae_p16x2s  *ae_des = (ae_p16x2s *)des;
    ae_p24x2s   p_tmp = AE_CVTP24A16(0);

    ae_src--;
    ae_des--;
    for (i = 0; i < sample; i += 2) {
        AE_LP24X2F_IU(p_tmp, ae_src, 8);
        AE_SP16X2F_IU(p_tmp, ae_des, 4);
    }
}

/**
 * @brief Gain Adjust 16-bit data.
 *
 * @param src     Pointer to the source of data to be adjusted.
 * @param sample  Number of sample to copy.
 * @param gain    Adjust Gain .
 */
VOID DSP_GainAdjust_16bit(
    S16 *src,
    U32  sample,
    S16  gain)
{
    U32 i;
    S32 temp;
    //TEMP!!
    if (gain != VAL_1_IN_Q15FORMAT) {
        for (i = 0 ; i < sample ; i++) {
            temp = (S32) * (src + i) * gain / VAL_1_IN_Q15FORMAT;
            *(src + i) = (S16)temp;
        }
    }
}

VOID DSP_GainAdjust_32bit(
    S32 *src,
    U32  sample,
    S16  gain)
{
    S64 temp;
    U32 i;
    //TEMP!!
    if (gain != VAL_1_IN_Q15FORMAT) {
        for (i = 0 ; i < sample ; i++) {
            temp = (S64) * (src + i) * gain / VAL_1_IN_Q15FORMAT;
            *(src + i) = (S32)temp;
        }
    }
}

VOID DSP_D2I_BufferCopy_16bit(U16 *DestBuf,
                              U16 *SrcBuf1,
                              U16 *SrcBuf2,
                              U16  SampleSize)
{
    int i;

    for (i = 0; i < SampleSize; i++) {
        DestBuf[i * 2    ] = SrcBuf1[i];
        DestBuf[i * 2 + 1] = SrcBuf2[i];
    }
}

ATTR_TEXT_IN_IRAM_LEVEL_2 VOID DSP_I2D_BufferCopy_16bit_mute(U16 *SrcBuf,
                                                             U16 *DestBuf1,
                                                             U16 *DestBuf2,
                                                             U16  SampleSize,
                                                             BOOL muteflag)
{
    int i;
    if (muteflag == TRUE) {
        if (!(DestBuf2 == NULL)) {
            for (i = 0; i < SampleSize; i++) {
                DestBuf1[i] = 0;
                DestBuf2[i] = 0;
            }
        } else {
            for (i = 0; i < SampleSize; i++) {
                DestBuf1[i] = 0;
            }
        }
    } else {
        if (!(DestBuf2 == NULL)) {
            for (i = 0; i < SampleSize; i++) {
                DestBuf1[i] = SrcBuf[i * 2];
                DestBuf2[i] = SrcBuf[i * 2 + 1];
            }
        } else {
            for (i = 0; i < SampleSize; i++) {
                DestBuf1[i] = SrcBuf[i * 2];
            }
        }
    }
}

ATTR_TEXT_IN_IRAM VOID DSP_D2I_BufferCopy_32bit(U32 *DestBuf,
                                                U32 *SrcBuf1,
                                                U32 *SrcBuf2,
                                                U32  SampleSize)
{
    U32 i;
    for (i = 0; i < SampleSize; i++) {
        DestBuf[i * 2    ] = SrcBuf1[i];
        DestBuf[i * 2 + 1] = SrcBuf2[i];
    }
}

VOID DSP_I2D_BufferCopy_32bit_mute(U32 *SrcBuf,
                                   U32 *DestBuf1,
                                   U32 *DestBuf2,
                                   U16  SampleSize,
                                   BOOL muteflag)
{
    int i;
    if (muteflag == TRUE) {
        if (!(DestBuf2 == NULL)) {
            for (i = 0; i < SampleSize; i++) {
                DestBuf1[i] = 0;
                DestBuf2[i] = 0;
            }
        } else {
            for (i = 0; i < SampleSize; i++) {
                DestBuf1[i] = 0;
            }
        }
    } else {
        if (!(DestBuf2 == NULL)) {
            for (i = 0; i < SampleSize; i++) {
                DestBuf1[i] = SrcBuf[i * 2];
                DestBuf2[i] = SrcBuf[i * 2 + 1];
            }
        } else {
            for (i = 0; i < SampleSize; i++) {
                DestBuf1[i] = SrcBuf[i * 2];
            }
        }
    }
}

VOID DSP_Fade_Process(Audio_Fade_Ctrl_Ptr fade_ctrl_ptr, U8 *src_addr, U32 length)
{
    U32 remainLength = length;
    U32 processLength;
    while (remainLength != 0) {
        if (fade_ctrl_ptr->Target_Level == fade_ctrl_ptr->Current_Level) {
            processLength = remainLength;
        } else {
            if (fade_ctrl_ptr->Resolution == RESOLUTION_16BIT) {
                processLength = sizeof(S16);
            } else {
                processLength = sizeof(S32);
            }
        }

        if (fade_ctrl_ptr->Resolution == RESOLUTION_16BIT) {
            DSP_GainAdjust_16bit((S16 *)src_addr, processLength / sizeof(S16), fade_ctrl_ptr->Current_Level);
        } else {
            DSP_GainAdjust_32bit((S32 *)src_addr, processLength / sizeof(S32), fade_ctrl_ptr->Current_Level);
        }

        if (fade_ctrl_ptr->Step > 0) {
            if ((fade_ctrl_ptr->Current_Level + fade_ctrl_ptr->Step) < fade_ctrl_ptr->Target_Level) {
                fade_ctrl_ptr->Current_Level += fade_ctrl_ptr->Step;
            } else {
                fade_ctrl_ptr->Current_Level = fade_ctrl_ptr->Target_Level;
            }
        } else {
            if ((fade_ctrl_ptr->Current_Level + fade_ctrl_ptr->Step) > fade_ctrl_ptr->Target_Level) {
                fade_ctrl_ptr->Current_Level += fade_ctrl_ptr->Step;
            } else {
                fade_ctrl_ptr->Current_Level = fade_ctrl_ptr->Target_Level;
            }
        }
        src_addr += processLength;
        remainLength -= processLength;
    }
}

/**
 * stream_pcm_copy_process
 *
 * Copy callback in_ptr to out_ptr
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
bool stream_pcm_copy_process(void *para)
{
    U32 i, in_ChNum;
    in_ChNum = 0;

    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    if((stream_ptr ->source ->type >= SOURCE_TYPE_DSP_VIRTUAL_MIN)&&(stream_ptr ->source ->type <= SOURCE_TYPE_DSP_VIRTUAL_MAX)) {
        if(stream_ptr->source->param.virtual_para.is_dummy_data == false){
            stream_codec_modify_output_size(para, stream_ptr->source->param.virtual_para.data_size);
            stream_codec_modify_output_samplingrate(para, stream_ptr->source->param.virtual_para.data_samplingrate);
        } else {
            stream_codec_modify_output_size(para, stream_ptr->source->param.virtual_para.default_data_size);
            stream_codec_modify_output_samplingrate(para, stream_ptr->source->param.virtual_para.default_data_samplingrate);
        }
    } else {
        stream_codec_modify_output_size(para, stream_codec_get_input_size(para));
        stream_codec_modify_output_samplingrate(para, stream_codec_get_input_samplingrate(para));
    }

    for (i = 0 ; i < stream_codec_get_output_channel_number(para) ; i++) {
        if (i != in_ChNum) {
            memcpy(stream_codec_get_output_buffer(para, i + 1),
                   stream_codec_get_input_buffer(para, in_ChNum + 1),
                   stream_codec_get_output_size(para));
        }
        in_ChNum++;
        in_ChNum %= stream_codec_get_input_channel_number(para);
    }

    if (stream_codec_get_input_resolution(para) != stream_codec_get_output_resolution(para)) {
        if (stream_codec_get_input_resolution(para) == RESOLUTION_16BIT) {
            for (i = 0 ; i < stream_codec_get_output_channel_number(para) ; i++) {
                dsp_converter_16bit_to_32bit((S32 *)stream_codec_get_output_buffer(para, i + 1),
                                             (S16 *)stream_codec_get_output_buffer(para, i + 1),
                                             stream_codec_get_output_size(para) / sizeof(S16));
            }
            stream_codec_modify_output_size(para, stream_codec_get_output_size(para) * 2);
        } else {
            for (i = 0 ; i < stream_codec_get_output_channel_number(para) ; i++) {
                dsp_converter_32bit_to_16bit((S16 *)stream_codec_get_output_buffer(para, i + 1),
                                             (S32 *)stream_codec_get_output_buffer(para, i + 1),
                                             stream_codec_get_output_size(para) / sizeof(S32));
            }
            stream_codec_modify_output_size(para, stream_codec_get_output_size(para) / 2);
        }
    }
    stream_codec_modify_resolution(para, stream_codec_get_output_resolution(para));
    return 0;
}
bool stream_function_size_converter_initialize(void *para)
{
    //Target IP init process
    UNUSED(para);
    return 0;
}



bool stream_function_size_converter_process(void *para)
{
    //S16* BufL = (S16*)stream_function_get_1st_inout_buffer(para);
    //S16* BufR = (S16*)stream_function_get_2nd_inout_buffer(para);
    U16 FrameSize = stream_function_get_output_size(para);

    U16 ProcessTimes, i;
    bool status = FALSE;
    if (FrameSize == 0) {
        return FALSE;
    } else if ((FrameSize % ProcessSize) != 0) {
        //printf("Warning:Process size not a factor of codec out frame size");
    }
    ProcessTimes = (FrameSize / ProcessSize);
    for (i = 0 ; i < ProcessTimes ; i++) {
        //  ((status =  Target_IP_process(stream_codec_get_workingbuffer(para), (U8*)InBuf + i*Target IP init, (U8*)OutBuf + i*240, 0)))
        /*bool Target_IP_process (InstancePtr ptr, S16* buffer_L_Channel,  S16* buffer_R_Channel, S16 frame_size ) */
        //if((status =  Target_IP_process(stream_codec_get_workingbuffer(para), (U8*)BufL + i*ProcessSize, (U8*)BufL + i*ProcessSize, ProcessSize)))
        //{break;}
    }
    return status;
}



BOOL UART2AudioInit(VOID *para)
{
    memset(stream_codec_get_1st_input_buffer(para), 0, stream_codec_get_input_size(para));
    memset(stream_codec_get_1st_output_buffer(para), 0, stream_codec_get_output_size(para));
    if (stream_codec_get_output_channel_number(para) > 1) {
        memset(stream_codec_get_2nd_output_buffer(para), 0, stream_codec_get_output_size(para));
    }
    stream_codec_modify_output_samplingrate(para, FS_RATE_48K);
    return 0;
}

BOOL UART2AudioCodec(VOID *para)
{
    U16 i = 0;
    U16 y = 0;
    for (i = 0; i < stream_codec_get_input_size(para) ;) {
        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 0] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 0];
        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 2] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 1];
        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 3] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 2];

        ((U8 *)stream_codec_get_2nd_output_buffer(para))[y + 0] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 3];
        ((U8 *)stream_codec_get_2nd_output_buffer(para))[y + 2] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 4];
        ((U8 *)stream_codec_get_2nd_output_buffer(para))[y + 3] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 5];

        y = y + 4;
        i = i + 6;
    }

    stream_codec_modify_output_size(para, stream_codec_get_input_size(para) * 2 / 3);
    return 0;
}
BOOL UART2AudioCodec_16bit(VOID *para)
{
    U16 i = 0;
    U16 y = 0;
    for (i = 0; i < stream_codec_get_input_size(para) ;) {
        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 0] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 0];
        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 1] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 1];

        ((U8 *)stream_codec_get_2nd_output_buffer(para))[y + 0] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 2];
        ((U8 *)stream_codec_get_2nd_output_buffer(para))[y + 1] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 3];

        y = y + 2;
        i = i + 4;
    }
    stream_codec_modify_output_size(para, stream_codec_get_input_size(para) / 2);
    return 0;
}
BOOL Audio2DataStreamCodec(VOID *para)
{

    U16 i = 0;
    U16 y = 0;
    for (i = 0; i < stream_codec_get_input_size(para) ;) {
        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 0] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 0];
        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 1] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 2];
        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 2] = ((U8 *)stream_codec_get_1st_input_buffer(para))[i + 3];

        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 3] = ((U8 *)stream_codec_get_2nd_input_buffer(para))[i + 0];
        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 4] = ((U8 *)stream_codec_get_2nd_input_buffer(para))[i + 2];
        ((U8 *)stream_codec_get_1st_output_buffer(para))[y + 5] = ((U8 *)stream_codec_get_2nd_input_buffer(para))[i + 3];

        y = y + 6;
        i = i + 4;
    }

    return 0;
}

#ifdef MTK_HWSRC_IN_STREAM

#define HWSRC_SEMAPHORE_MAX 5
#define HWSRC_SEMAPHORE_INIT 0
SemaphoreHandle_t gHwsrc_port_semphr = NULL;

void hwsrc_get_offset(DSP_SRC_FEATURE_PTR  src_feature_ptr)
{
    src_feature_ptr->inSRC_ro = AFE_READ(ASM_CH01_IBUF_RDPNT)- AFE_READ(ASM_IBUF_SADR);
    src_feature_ptr->inSRC_wo = AFE_READ(ASM_CH01_IBUF_WRPNT)- AFE_READ(ASM_IBUF_SADR);
    src_feature_ptr->inSRC_count = ((src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_size  )
                            - src_feature_ptr->inSRC_ro)
                            % src_feature_ptr->inSRC_mem_size;

    src_feature_ptr->outSRC_ro = AFE_READ(ASM_CH01_OBUF_RDPNT) - AFE_READ(ASM_OBUF_SADR);
    src_feature_ptr->outSRC_wo = AFE_READ(ASM_CH01_OBUF_WRPNT) - AFE_GET_REG(ASM_OBUF_SADR);
    src_feature_ptr->outSRC_count = (src_feature_ptr->outSRC_wo > src_feature_ptr->outSRC_ro) ?
                                    src_feature_ptr->outSRC_wo - src_feature_ptr->outSRC_ro :
                                    src_feature_ptr->outSRC_wo + src_feature_ptr->outSRC_mem_size - src_feature_ptr->outSRC_ro;
}

BOOL hwsrc_port_take_semphr(void)
{
    // DSP_MW_LOG_I("hwsrc_port_take_semphr start", 0);
	if(gHwsrc_port_semphr && (pdTRUE == xSemaphoreTake(gHwsrc_port_semphr, portMAX_DELAY)))
	{
		return TRUE;
	}
    DSP_MW_LOG_I("asrc_port_take_semphr fail", 0);
    return FALSE;
}

uint32_t hwsrc_port_get_semphr_count(void)
{
	return uxSemaphoreGetCount(gHwsrc_port_semphr);
}

#define EEROR_RANGE 0.01

extern bool hal_audio_src_set_start(afe_src_configuration_t *configuration, hal_audio_memory_sync_selection_t sync_select, hal_audio_control_status_t control);
extern bool hal_audio_src_configuration(afe_src_configuration_t *configuration, hal_audio_control_status_t control);

void DSP_Set_ASRC_Configuration_Parameters(VOID *para)
{
    DSP_MW_LOG_I("Set_ASRC_Configuration_Parameters\n", 0);
    DSP_SRC_FEATURE_PTR  src_feature_ptr;
    DSP_STREAMING_PARA_PTR stream_ptr;
    S32 nch = HWSRC_DEFAULT_CHANNEL;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    src_feature_ptr = stream_function_get_working_buffer(para);
    afe_src_configuration_t src_configuration;



    memset(&src_configuration, 0, sizeof(src_configuration));
    src_configuration.hwsrc_type = stream_ptr->sink->param.audio.hwsrc_type;
    src_configuration.id = AFE_MEM_ASRC_1;
    src_configuration.input_buffer.addr = hal_memview_dsp0_to_infrasys((uint32_t)src_feature_ptr->inSRC_mem_ptr);
    src_configuration.input_buffer.offset = 0;
    src_configuration.input_buffer.rate = DSP_FsChange2SRCInRate((stream_samplerate_t)(U8)stream_function_get_samplingrate(para));
    src_configuration.input_buffer.size = src_feature_ptr->inSRC_mem_size;
    src_configuration.is_mono = 0;
    src_configuration.mode = AFE_SRC_NO_TRACKING;
    src_configuration.output_buffer.addr = hal_memview_dsp0_to_infrasys((uint32_t)src_feature_ptr->outSRC_mem_ptr);
    src_configuration.output_buffer.offset = stream_ptr->callback.Src.out_frame_size * nch + 64;
    src_configuration.output_buffer.rate = DSP_FsChange2SRCOutRate((stream_samplerate_t)(U8)stream_ptr->callback.Src.out_sampling_rate);
    src_configuration.output_buffer.size = src_feature_ptr->outSRC_mem_size;
    src_configuration.ul_mode = false;
    src_configuration.out_frame_size = stream_ptr->callback.Src.out_frame_size * nch;
    if (DSP_RsChange2SRCOutRs(stream_ptr->callback.Src.in_resolution) == 4) {
        src_configuration.input_buffer.format = (hal_audio_format_t)HAL_AUDIO_PCM_FORMAT_S32_LE;
        src_configuration.output_buffer.format  = (hal_audio_format_t)HAL_AUDIO_PCM_FORMAT_S32_LE;
    } else {
        src_configuration.input_buffer.format =  (hal_audio_format_t)HAL_AUDIO_PCM_FORMAT_S16_LE;
        src_configuration.output_buffer.format =  (hal_audio_format_t)HAL_AUDIO_PCM_FORMAT_S16_LE;
    }

    hal_audio_src_configuration(&src_configuration, HAL_AUDIO_CONTROL_ON);
    hal_audio_src_set_start(&src_configuration, HAL_AUDIO_MEMORY_SYNC_NONE, HAL_AUDIO_CONTROL_ON);

    DSP_MW_LOG_I("transform asrc in rate:%d, out rate:%d\r\n", 2, src_configuration.input_buffer.rate, src_configuration.output_buffer.rate);

    src_feature_ptr->inSRC_wo = 0;//init
    AFE_WRITE(ASM_CH01_IBUF_WRPNT, src_feature_ptr->inSRC_wo + AFE_READ(ASM_IBUF_SADR));
    src_feature_ptr->outSRC_ro = 0;//init
    AFE_WRITE(ASM_CH01_OBUF_RDPNT, src_feature_ptr->outSRC_ro + AFE_GET_REG(ASM_OBUF_SADR));
    DSP_MW_LOG_I("ASRC inwptr %d outrptr %d\n", 2, AFE_READ(ASM_CH01_IBUF_WRPNT) - AFE_GET_REG(ASM_IBUF_SADR),
                 AFE_READ(ASM_CH01_OBUF_RDPNT) - AFE_GET_REG(ASM_OBUF_SADR));
}

BOOL DSP_Callback_ASRC_Handle(VOID *para, VOID *buf_ptr1, VOID *buf_ptr2, U32 in_size)
{
    U32 inWriteOffset;
    DSP_SRC_FEATURE_PTR  src_feature_ptr;
    DSP_STREAMING_PARA_PTR stream_ptr;
    U32 irq_en_mask;
    U32 volatile irq_status;
    S32 input_expect_data;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    src_feature_ptr = stream_function_get_working_buffer(para);
    S32 bytes_per_sample = DSP_RsChange2SRCOutRs(stream_ptr->callback.Src.in_resolution);
    S32 nch = HWSRC_DEFAULT_CHANNEL;
    U32 input_rate = DSP_FsChange2SRCInRate((stream_samplerate_t)(U8)stream_function_get_samplingrate(para));
    U32 output_rate = DSP_FsChange2SRCOutRate((stream_samplerate_t)(U8)stream_ptr->callback.Src.out_sampling_rate);
    U32 irq_count = hwsrc_port_get_semphr_count();
    afe_src_configuration_t src_configuration;
    memset(&src_configuration, 0, sizeof(afe_src_configuration_t));
    src_configuration.id = AFE_MEM_ASRC_1;
    src_configuration.hwsrc_type = stream_ptr->sink->param.audio.hwsrc_type;
    if(input_rate == output_rate){
        input_expect_data = stream_ptr->callback.Src.out_frame_size * nch;
    }else{
        input_expect_data = (((stream_ptr->callback.Src.out_frame_size * nch) * input_rate) / output_rate) + (EEROR_RANGE * stream_ptr->callback.Src.out_frame_size * nch);
    }

    if(src_feature_ptr->InitDone != 1){
            DSP_Set_ASRC_Configuration_Parameters(para);
        src_feature_ptr->InitDone = 1;
    }

    src_feature_ptr->inSRC_ro = AFE_READ(ASM_CH01_IBUF_RDPNT) - AFE_READ(ASM_IBUF_SADR);
    src_feature_ptr->inSRC_wo = AFE_READ(ASM_CH01_IBUF_WRPNT) - AFE_READ(ASM_IBUF_SADR);
    src_feature_ptr->inSRC_count = ((src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_size)
                                    - src_feature_ptr->inSRC_ro)
                                    % src_feature_ptr->inSRC_mem_size;

    // DSP_MW_LOG_I("1 inSRC_wo %d inSRC_ro %d inSRC_count %d outSRC_wo %d,outSRC_ro %d outSRC_count %d \n",6,src_feature_ptr->inSRC_wo,src_feature_ptr->inSRC_ro,src_feature_ptr->inSRC_count,src_feature_ptr->outSRC_wo,src_feature_ptr->outSRC_ro,src_feature_ptr->outSRC_count);

    // DSP_MW_LOG_I("2 in_size %d,nch:%d,IRD:%x,IWR:%x,ORD:%x,OWR:%x\n ",6,in_size,nch,AFE_READ(ASM_CH01_IBUF_RDPNT),AFE_READ(ASM_CH01_IBUF_WRPNT),AFE_READ(ASM_CH01_OBUF_RDPNT),AFE_READ(ASM_CH01_OBUF_WRPNT));
    if (src_feature_ptr->inSRC_count+(in_size * nch) <= src_feature_ptr->inSRC_mem_size)
    {
        if(nch < MAX_CHANNELS){
            //memcpy(src_feature_ptr->SRC_INPUT_BUF, buf_ptr1, in_size);
            DSP_D2C_BufferCopy(src_feature_ptr->inSRC_wo+src_feature_ptr->inSRC_mem_ptr,
            buf_ptr1,
            in_size,
            src_feature_ptr->inSRC_mem_ptr,
            src_feature_ptr->inSRC_mem_size);
        }
        else if(nch == MAX_CHANNELS){
                U8 *DestCBufEnd     = (U8 *)((U8 *)src_feature_ptr->inSRC_mem_ptr + src_feature_ptr->inSRC_mem_size);
                U16 UnwrapSize      = (U8 *)DestCBufEnd - (U8 *)(src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_ptr); /* Remove + 1 to sync more common usage */
            S32 WrapSize        = (in_size * nch) - UnwrapSize;
            configASSERT((DestCBufEnd >= (U8 *)(src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_ptr)) && ((src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_ptr) >= src_feature_ptr->inSRC_mem_ptr));
            if (bytes_per_sample == 4){
                if (WrapSize > 0)
                {
                    DSP_D2I_BufferCopy_32bit((U32 *)(src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_ptr),
                                        (U32 *)buf_ptr1,
                                        (U32 *)buf_ptr2,
                                        (UnwrapSize / nch) >> 2);

                        DSP_D2I_BufferCopy_32bit((U32 *)src_feature_ptr->inSRC_mem_ptr,
                        (U32 *)((U8 *)buf_ptr1 + (UnwrapSize >> 1)),
                        (U32 *)((U8 *)buf_ptr2 + (UnwrapSize >> 1)),
                        (WrapSize / nch) >> 2);
                }
                else
                {
                    //memcpy(DestBuf, SrcBuf, CopySize);
                    DSP_D2I_BufferCopy_32bit((U32 *)(src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_ptr),
                    (U32 *)buf_ptr1,
                    (U32 *)buf_ptr2,
                    in_size >> 2);
                }

            }else{
                if (WrapSize > 0)
                {
                    DSP_D2I_BufferCopy_16bit((U16 *)(src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_ptr),
                                        (U16 *)buf_ptr1,
                                        (U16 *)buf_ptr2,
                                        (UnwrapSize / nch) >> 1);

                    //memcpy(DestCBufStart, (U8*)SrcBuf + UnwrapSize, WrapSize);
                        DSP_D2I_BufferCopy_16bit((U16 *)src_feature_ptr->inSRC_mem_ptr,
                        (U16 *)((U8 *)buf_ptr1 + (UnwrapSize >> 1)),
                        (U16 *)((U8 *)buf_ptr2 + (UnwrapSize >> 1)),
                        (WrapSize / nch) >> 1);
                }
                else
                {
                    DSP_D2I_BufferCopy_16bit((U16 *)(src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_ptr),
                    (U16 *)buf_ptr1,
                    (U16 *)buf_ptr2,
                    in_size >> 1);
                }
            }
        }


        src_feature_ptr->inSRC_wo = AFE_READ(ASM_CH01_IBUF_WRPNT) - AFE_READ(ASM_IBUF_SADR);
        inWriteOffset = src_feature_ptr->inSRC_wo;
        // printf("src_feature_ptr->inSRC_wo %d (in_size*nch) %d bytes_per_sample %d\n",src_feature_ptr->inSRC_wo,(in_size*nch),bytes_per_sample);
        src_feature_ptr->inSRC_wo += (in_size * nch);
        src_feature_ptr->inSRC_wo %= src_feature_ptr->inSRC_mem_size;

        AFE_WRITE(ASM_CH01_IBUF_WRPNT, src_feature_ptr->inSRC_wo + AFE_READ(ASM_IBUF_SADR));
        // DSP_MW_LOG_I("3 inSRC_wo %d inSRC_ro %d in_size %d\n",5,src_feature_ptr->inSRC_wo,src_feature_ptr->inSRC_ro,(in_size * nch));
        // DSP_MW_LOG_I("4 IRD:%x,IWR:%x,ORD:%x,OWR:%x\n ",4,AFE_READ(ASM_CH01_IBUF_RDPNT),AFE_READ(ASM_CH01_IBUF_WRPNT),AFE_READ(ASM_CH01_OBUF_RDPNT),AFE_READ(ASM_CH01_OBUF_WRPNT));

    }

    hwsrc_get_offset(src_feature_ptr);
    // DSP_MW_LOG_I("outSRC_wo %d,outSRC_ro %d inSRC_count %d outSRC_count %d out_frame_size %d\n",5,src_feature_ptr->outSRC_wo,src_feature_ptr->outSRC_ro,
    // src_feature_ptr->inSRC_count,src_feature_ptr->outSRC_count,stream_ptr->callback.Src.out_frame_size);
    if(src_feature_ptr->outSRC_count < stream_ptr->callback.Src.out_frame_size * nch){
        if(src_feature_ptr->inSRC_count >= input_expect_data){
            if(hwsrc_port_take_semphr()){
                // DSP_MW_LOG_I("asrc_port_take_semphr success", 0);
            }else{
                // DSP_MW_LOG_I("asrc_port_take_semphr fail", 0);
                return FALSE;
            }
        }else{
            return FALSE;
        }
    }else{
        if(irq_count > 0){
                if(hwsrc_port_take_semphr()){
                    // DSP_MW_LOG_I("irq_count asrc_port_take_semphr success", 0);
                }else{
                    // DSP_MW_LOG_I("asrc_port_take_semphr fail", 0);
                return FALSE;
            }
        }
    }
    hwsrc_get_offset(src_feature_ptr);
    // DSP_MW_LOG_I("while inSRC_wo %d inSRC_ro %d inSRC_count %d,IRD:%x,IWR:%x\n",5,src_feature_ptr->inSRC_wo,src_feature_ptr->inSRC_ro,src_feature_ptr->inSRC_count,AFE_READ(ASM_CH01_IBUF_RDPNT),AFE_READ(ASM_CH01_IBUF_WRPNT));
    // DSP_MW_LOG_I("while outSRC_wo %d,outSRC_ro %d outSRC_count %d out_frame_size %d extract_length:%d ORD :%x OWR:%x\n",7,src_feature_ptr->outSRC_wo,src_feature_ptr->outSRC_ro,
    // src_feature_ptr->outSRC_count,stream_ptr->callback.Src.out_frame_size,src_feature_ptr->inSRC_extract_length,AFE_READ(ASM_CH01_OBUF_RDPNT),AFE_READ(ASM_CH01_OBUF_WRPNT));

    if (src_feature_ptr->outSRC_count > stream_ptr->callback.Src.out_frame_size * nch)//(stream_ptr->callback.Src.out_frame_size * nch)
    {

        if(nch < MAX_CHANNELS){

            DSP_C2D_BufferCopy(buf_ptr1,
                                src_feature_ptr->outSRC_ro + src_feature_ptr->outSRC_mem_ptr,
                                stream_ptr->callback.Src.out_frame_size * nch,
                                src_feature_ptr->outSRC_mem_ptr,
                                src_feature_ptr->outSRC_mem_size);

        }
        else if(nch==MAX_CHANNELS){
                    U8 *SrcCBufEnd      =  (U8 *)((U8 *)src_feature_ptr->outSRC_mem_ptr +  src_feature_ptr->outSRC_mem_size);
            U16 UnwrapSize      = (U8 *)SrcCBufEnd - (U8 *)(src_feature_ptr->outSRC_ro + src_feature_ptr->outSRC_mem_ptr); /* Remove + 1 to sync more common usage */
                    S32 WrapSize        = (stream_ptr->callback.Src.out_frame_size * nch) - UnwrapSize;
                    configASSERT((SrcCBufEnd >= (U8 *)(src_feature_ptr->outSRC_ro + src_feature_ptr->outSRC_mem_ptr)) && ((src_feature_ptr->outSRC_ro + src_feature_ptr->outSRC_mem_ptr) >= src_feature_ptr->outSRC_mem_ptr));
            if (bytes_per_sample == 4){
                    if (WrapSize > 0)
                    {
                    DSP_I2D_BufferCopy_32bit_mute((U32 *)(src_feature_ptr->outSRC_ro + src_feature_ptr->outSRC_mem_ptr),
                                            (U32 *)buf_ptr1,
                                            (U32 *)buf_ptr2,
                                        (UnwrapSize / nch) >> 2,
                                        false);

                    DSP_I2D_BufferCopy_32bit_mute((U32 *)src_feature_ptr->outSRC_mem_ptr,
                                            (U32 *)((U8 *)buf_ptr1 + (UnwrapSize >> 1)),
                                            (U32 *)((U8 *)buf_ptr2 + (UnwrapSize >> 1)),
                                        (WrapSize / nch) >> 2,
                                        false);

                    }
                    else
                    {

                    DSP_I2D_BufferCopy_32bit_mute((U32 *)(src_feature_ptr->outSRC_ro + src_feature_ptr->outSRC_mem_ptr),
                                            (U32 *)buf_ptr1,
                                            (U32 *)buf_ptr2,
                                        (stream_ptr->callback.Src.out_frame_size) >> 2,
                                        false);
                    }

                }else{
                    if (WrapSize > 0)
                    {

                    DSP_I2D_BufferCopy_16bit_mute((U16 *)(src_feature_ptr->outSRC_ro + src_feature_ptr->outSRC_mem_ptr),
                                            (U16 *)buf_ptr1,
                                            (U16 *)buf_ptr2,
                                        (UnwrapSize / nch) >> 1,
                                        false);

                    DSP_I2D_BufferCopy_16bit_mute((U16 *)src_feature_ptr->outSRC_mem_ptr,
                                            (U16 *)((U8 *)buf_ptr1 + (UnwrapSize >> 1)),
                                            (U16 *)((U8 *)buf_ptr2 + (UnwrapSize >> 1)),
                                        (WrapSize / nch) >> 1,
                                        false);

                    }
                    else
                    {
                    DSP_I2D_BufferCopy_16bit_mute((U16 *)(src_feature_ptr->outSRC_ro + src_feature_ptr->outSRC_mem_ptr),
                                            (U16 *)buf_ptr1,
                                            (U16 *)buf_ptr2,
                                        (stream_ptr->callback.Src.out_frame_size) >> 1,
                                        false);
                    }
                }
        }

        src_feature_ptr->outSRC_ro = AFE_READ(ASM_CH01_OBUF_RDPNT) - AFE_READ(ASM_OBUF_SADR);
        src_feature_ptr->outSRC_ro += (stream_ptr->callback.Src.out_frame_size * nch);
        src_feature_ptr->outSRC_ro %= src_feature_ptr->outSRC_mem_size;
        AFE_WRITE(ASM_CH01_OBUF_RDPNT, src_feature_ptr->outSRC_ro + AFE_GET_REG(ASM_OBUF_SADR));
        // DSP_MW_LOG_I("src write after outSRC_wo %d,outSRC_ro %d outSRC_count %d out_frame_size %d\n ",4,src_feature_ptr->outSRC_wo,src_feature_ptr->outSRC_ro,src_feature_ptr->outSRC_count,stream_ptr->callback.Src.out_frame_size);

        irq_status = AFE_GET_REG(ASM_IFR)&ASM_IFR_MASK;
        irq_en_mask = (AFE_GET_REG(ASM_IER))&ASM_IFR_MASK;
        AFE_SET_REG(ASM_IFR, irq_status, irq_en_mask);
        hal_src_set_irq_enable(&src_configuration, true);

    }
    else
    {
        return FALSE;
        // need to handle for callbac
    }

    return TRUE;
}
#endif

BOOL DSP_Callback_SRC_Handle(VOID *para, VOID *buf_ptr1, VOID *buf_ptr2, U32 in_size)
{
    U32 inWriteOffset;
    U32 outReadOffset;
    DSP_SRC_FEATURE_PTR  src_feature_ptr;
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    src_feature_ptr = stream_function_get_working_buffer(para);
    inWriteOffset = src_feature_ptr->inSRC_wo;

    while (DSP_GetSRCStatus(src_feature_ptr->src_ptr) && (stream_ptr->streamingStatus == STREAMING_ACTIVE)){
       // DSP_LOG_WarningPrint(DSP_WARNING_WAIT_SRC_DONE, 0);
       // osTaskSignalWait(src_feature_ptr->task_id);
       // osTaskTaskingRequest();
    }
    if (src_feature_ptr->inSRC_count + in_size <= src_feature_ptr->inSRC_mem_size){
        DSP_D2C_BufferCopy((VOID *)(DSP_GetSRCIn1BufPtr(src_feature_ptr->src_ptr) + src_feature_ptr->inSRC_wo),
                           buf_ptr1,
                           in_size,
                           DSP_GetSRCIn1BufPtr(src_feature_ptr->src_ptr),
                           DSP_GetSRCInBufSize(src_feature_ptr->src_ptr));

        if (src_feature_ptr->channel_num >= 2)
        {
            DSP_D2C_BufferCopy((VOID *)(DSP_GetSRCIn2BufPtr(src_feature_ptr->src_ptr) + src_feature_ptr->inSRC_wo),
                               buf_ptr2,
                               in_size,
                               DSP_GetSRCIn2BufPtr(src_feature_ptr->src_ptr),
                               DSP_GetSRCInBufSize(src_feature_ptr->src_ptr));
        }

        inWriteOffset = src_feature_ptr->inSRC_wo;
        src_feature_ptr->inSRC_wo += in_size;
        src_feature_ptr->inSRC_wo %= src_feature_ptr->inSRC_mem_size;
    }
    else{
        // need to handle for callback
    }


    src_feature_ptr->inSRC_ro = DSP_GetSRCInReadOffset(src_feature_ptr->src_ptr);
    src_feature_ptr->inSRC_count = ((src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_size)
                                    - src_feature_ptr->inSRC_ro)
                                   % src_feature_ptr->inSRC_mem_size;

    if(OFFSET_OVERFLOW_CHK(inWriteOffset, src_feature_ptr->inSRC_wo, src_feature_ptr->inSRC_ro)||
      (src_feature_ptr->inSRC_count <= src_feature_ptr->inSRC_extract_length+CALLBACK_SRC_FLOAT_PROTECTION) ){
        return FALSE;
    }

    outReadOffset = (DSP_GetSRCOutWriteOffset(src_feature_ptr->src_ptr) +
                     DSP_GetSRCOutBufSize(src_feature_ptr->src_ptr) - DSP_GetSRCOutFrameSize(src_feature_ptr->src_ptr))
                     % DSP_GetSRCOutBufSize(src_feature_ptr->src_ptr);
    DSP_SetSRCTrigger(src_feature_ptr->src_ptr);

    DSP_C2D_BufferCopy(buf_ptr1,
                       (VOID *)(DSP_GetSRCOut1BufPtr(src_feature_ptr->src_ptr) + outReadOffset),
                       DSP_GetSRCOutFrameSize(src_feature_ptr->src_ptr),
                       DSP_GetSRCOut1BufPtr(src_feature_ptr->src_ptr),
                       DSP_GetSRCOutBufSize(src_feature_ptr->src_ptr));
    if (src_feature_ptr->channel_num >= 2){
        DSP_C2D_BufferCopy(buf_ptr2,
                           (VOID *)(DSP_GetSRCOut2BufPtr(src_feature_ptr->src_ptr) + outReadOffset),
                           DSP_GetSRCOutFrameSize(src_feature_ptr->src_ptr),
                           DSP_GetSRCOut2BufPtr(src_feature_ptr->src_ptr),
                           DSP_GetSRCOutBufSize(src_feature_ptr->src_ptr));
    }

    src_feature_ptr->outSRC_ro = outReadOffset;
    src_feature_ptr->outSRC_wo = DSP_GetSRCOutWriteOffset(src_feature_ptr->src_ptr) ;
    return TRUE;
}

VOID DSP_SRC_CBufWrite(DSP_SRC_FEATURE_PTR src_feature_ptr, VOID *buf_ptr1, VOID *buf_ptr2, U32 in_size)
{
    DSP_D2C_BufferCopy((VOID *)(src_feature_ptr->buf_mem_ptr + src_feature_ptr->buf_wo),
                       buf_ptr1,
                       in_size,
                       (VOID *)src_feature_ptr->buf_mem_ptr,
                       src_feature_ptr->buf_mem_size);

    if (src_feature_ptr->channel_num > 1){
        DSP_D2C_BufferCopy((VOID *)(src_feature_ptr->buf_mem_ptr+src_feature_ptr->buf_mem_size + src_feature_ptr->buf_wo),
                           buf_ptr2,
                           in_size,
                           (VOID *)(src_feature_ptr->buf_mem_ptr+src_feature_ptr->buf_mem_size),
                           src_feature_ptr->buf_mem_size);
    }

    src_feature_ptr->buf_wo += in_size;
    src_feature_ptr->buf_wo %= src_feature_ptr->buf_mem_size;
    src_feature_ptr->buf_count += in_size;
}

VOID DSP_SRC_CBufRead(DSP_SRC_FEATURE_PTR src_feature_ptr, VOID *buf_ptr1, VOID *buf_ptr2, U32 out_size)
{

    DSP_C2D_BufferCopy(buf_ptr1,
                       (VOID *)(src_feature_ptr->buf_mem_ptr + src_feature_ptr->buf_ro),
                       out_size,
                       (VOID *)src_feature_ptr->buf_mem_ptr,
                       src_feature_ptr->buf_mem_size);

    if (src_feature_ptr->channel_num > 1){
        DSP_C2D_BufferCopy(buf_ptr2,
                       (VOID *)(src_feature_ptr->buf_mem_ptr+src_feature_ptr->buf_mem_size + src_feature_ptr->buf_ro),
                       out_size,
                       (VOID *)(src_feature_ptr->buf_mem_ptr+src_feature_ptr->buf_mem_size),
                       src_feature_ptr->buf_mem_size);
    }

    src_feature_ptr->buf_ro += out_size;
    src_feature_ptr->buf_ro %= src_feature_ptr->buf_mem_size;
    src_feature_ptr->buf_count -= out_size;
}

#ifdef MTK_HWSRC_IN_STREAM
uint8_t hwsrc_sleep_manager_handle = 0;

bool stream_function_src_initialize(void *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    DSP_SRC_FEATURE_PTR src_feature_ptr;
    DSP_DRV_SRC_VDM_INIT_STRU src_setting;
    U32 ch_num;
    U8 *mem_ptr;
    U16 srcInFrameSize;
    src_feature_ptr = stream_function_get_working_buffer(para);
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    ch_num = HWSRC_DEFAULT_CHANNEL;
    srcInFrameSize = stream_ptr->callback.EntryPara.out_malloc_size;

    src_feature_ptr->inSRC_mem_size     = srcInFrameSize * DSP_CALLBACK_SRC_IN_FRAME;
    src_feature_ptr->outSRC_mem_size    = stream_ptr->callback.Src.out_frame_size * DSP_CALLBACK_SRC_OUT_FRAME;
    src_feature_ptr->buf_mem_size       = srcInFrameSize * DSP_CALLBACK_SRC_BUF_FRAME;
    DSP_MW_LOG_I("stream_function_src_initialize \n", 0);


#ifdef MTK_HWSRC_IN_STREAM
    if (gHwsrc_port_semphr == NULL)
    {
        gHwsrc_port_semphr = xSemaphoreCreateCounting(HWSRC_SEMAPHORE_MAX, HWSRC_SEMAPHORE_INIT);

        if( gHwsrc_port_semphr == NULL )
        {
            configASSERT(0);
        }
    }

    mem_ptr = (U8 *)((U32)src_feature_ptr + DSP_OFFSET_OF(DSP_SRC_FEATURE, mem_begin));
    mem_ptr = (U8 *)((U32)(mem_ptr + 15) & ~15);  //modify for asrc 8-byte align
    src_feature_ptr->inSRC_mem_ptr = mem_ptr;
    mem_ptr += src_feature_ptr->inSRC_mem_size * ch_num;
    mem_ptr = (U8 *)((U32)(mem_ptr + 15) & ~15);  //modify for asrc 8-byte align
    src_feature_ptr->outSRC_mem_ptr = mem_ptr;
    mem_ptr += src_feature_ptr->outSRC_mem_size * ch_num;
    mem_ptr = (U8 *)((U32)(mem_ptr + 15) & ~15);  //modify for asrc 8-byte align
    src_feature_ptr->buf_mem_ptr = mem_ptr;
#else
    mem_ptr = (U8 *)((U32)src_feature_ptr + DSP_OFFSET_OF(DSP_SRC_FEATURE, mem_begin));
    src_feature_ptr->inSRC_mem_ptr = mem_ptr;
    mem_ptr += src_feature_ptr->inSRC_mem_size * ch_num;
    src_feature_ptr->outSRC_mem_ptr = mem_ptr;
    mem_ptr += src_feature_ptr->outSRC_mem_size * ch_num;
    src_feature_ptr->buf_mem_ptr = mem_ptr;
#endif

    memset(src_feature_ptr->inSRC_mem_ptr,    0, (src_feature_ptr->inSRC_mem_size +
                                                  src_feature_ptr->outSRC_mem_size +
                                                  src_feature_ptr->buf_mem_size) * ch_num);
#ifdef MTK_HWSRC_IN_STREAM
    src_feature_ptr->outSRC_mem_size = src_feature_ptr->outSRC_mem_size * 2;
    src_feature_ptr->inSRC_mem_size = src_feature_ptr->inSRC_mem_size * 2;

    if (AFE_GET_REG(ASM_GEN_CONF + AFE_MEM_ASRC_1)&ASM_GEN_CONF_ASRC_BUSY_MASK) {
        DSP_MW_LOG_E("DSP_SRC_Init() error: asrc[%d] is running\r\n", 1, AFE_MEM_ASRC_1);
    }
    if ((AFE_GET_REG(ASM_GEN_CONF + AFE_MEM_ASRC_1)&ASM_GEN_CONF_ASRC_EN_MASK)) {

        afe_src_configuration_t src_configuration;
        memset(&src_configuration, 0, sizeof(afe_src_configuration_t));
        src_configuration.id = AFE_MEM_ASRC_1;
        hal_audio_src_set_start(&src_configuration, HAL_AUDIO_MEMORY_SYNC_NONE, HAL_AUDIO_CONTROL_OFF);
        hal_audio_src_configuration(&src_configuration, HAL_AUDIO_CONTROL_OFF);
    }
    if(!hwsrc_sleep_manager_handle){
        hwsrc_sleep_manager_handle = hal_sleep_manager_set_sleep_handle("hwsrc");
        hal_sleep_manager_lock_sleep(hwsrc_sleep_manager_handle);
        DSP_MW_LOG_I("hal_sleep_manager_lock_sleep\n", 0);
    }
#endif

    stream_ptr->callback.Src.inSRC_Full = FALSE;
    stream_ptr->callback.Src.outSRC_Full = FALSE;

    src_feature_ptr->in_sampling_rate =  stream_function_get_samplingrate(para);
    src_feature_ptr->channel_num      =  HWSRC_DEFAULT_CHANNEL;
    src_feature_ptr->task_id          =  (TaskHandle_t)stream_function_get_task(para);
    /*Configure SRC*/
    src_setting.src_ptr        = stream_ptr->callback.Src.src_ptr;
    src_setting.mode           = SRC_OVDM;
    src_setting.task_id        = src_feature_ptr->task_id ;
    src_setting.radma_buf_addr = (U8 *)(src_feature_ptr->inSRC_mem_ptr);
    src_setting.radma_buf_size = src_feature_ptr->inSRC_mem_size;
    src_setting.radma_THD      = srcInFrameSize;
    src_setting.wadma_buf_addr = (U8 *)(src_feature_ptr->outSRC_mem_ptr);
    src_setting.wadma_buf_size = src_feature_ptr->outSRC_mem_size;
    src_setting.wadma_THD      = stream_ptr->callback.Src.out_frame_size;
    src_setting.fs_in          = DSP_FsChange2SRCInRate(src_feature_ptr->in_sampling_rate);
    src_setting.fs_out         = DSP_FsChange2SRCOutRate(stream_ptr->callback.Src.out_sampling_rate);
#ifdef MTK_HWSRC_IN_STREAM
    src_setting.Res_In         = (stream_resolution_t)DSP_RsChange2SRCOutRs(stream_ptr->callback.Src.in_resolution);
    src_setting.Res_Out        = (stream_resolution_t)DSP_RsChange2SRCOutRs(stream_ptr->callback.Src.out_resolution);
#else
    src_setting.Res_In         = stream_ptr->callback.Src.in_resolution;
    src_setting.Res_Out        = stream_ptr->callback.Src.out_resolution;
#endif

    src_setting.channel_num    = ch_num;


    //stream_ptr->callback.Src.src_ptr = DSP_DRV_SRC_VDM_INIT(&src_setting);
#ifdef MTK_HWSRC_IN_STREAM
    stream_ptr->callback.Src.src_ptr = (SRC_PTR_s)ASRC_1;
#endif

    stream_ptr->callback.EntryPara.src_out_sampling_rate = stream_ptr->callback.Src.out_sampling_rate;

    src_feature_ptr->src_ptr = stream_ptr->callback.Src.src_ptr;
    src_feature_ptr->inSRC_extract_length = (stream_ptr->callback.Src.out_frame_size * src_feature_ptr->in_sampling_rate)
                                            / stream_ptr->callback.Src.out_sampling_rate;
    src_feature_ptr->in_max_size = 0;

    src_feature_ptr->inSRC_ro = 0;// DSP_GetSRCInReadOffset(src_feature_ptr->src_ptr);
    src_feature_ptr->inSRC_wo = 0;//(src_feature_ptr->inSRC_ro + src_feature_ptr->inSRC_mem_size/2)%src_feature_ptr->inSRC_mem_size;
    src_feature_ptr->inSRC_count = ((src_feature_ptr->inSRC_wo + src_feature_ptr->inSRC_mem_size)
                                    - src_feature_ptr->inSRC_ro)
                                   % src_feature_ptr->inSRC_mem_size;
    DSP_MW_LOG_I("src_feature_ptr->inSRC_mem_size %d \n", 1, src_feature_ptr->inSRC_mem_size);

    src_feature_ptr->outSRC_ro = 0;
    src_feature_ptr->outSRC_wo = 0;

    src_feature_ptr->buf_ro = 0;
    src_feature_ptr->buf_wo = 0;//stream_ptr->callback.Src.out_frame_size;
#ifndef MTK_HWSRC_IN_STREAM
    src_feature_ptr->buf_count = ((src_feature_ptr->buf_wo + src_feature_ptr->buf_mem_size)
                                  - src_feature_ptr->buf_ro)
                                 % src_feature_ptr->buf_mem_size;
#endif
#ifdef MTK_HWSRC_IN_STREAM
    DSP_MW_LOG_I("MTK_HWSRC_IN_STREAM\n", 0);
    src_feature_ptr->InitDone = 0;
    SRC_Handle_Entry = DSP_Callback_ASRC_Handle;//DSP_Callback_SRC_Handle;
#endif

    return FALSE;

}



bool stream_function_src_process(void *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    DSP_SRC_FEATURE_PTR src_feature_ptr;
    U32 in_size, out_size = 0;
    U32 ch_num;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    src_feature_ptr = stream_function_get_working_buffer(para);
    ch_num = stream_function_get_channel_number(para);
    UNUSED(in_size);
    if (src_feature_ptr->src_ptr != NULL) {
        if ((src_feature_ptr->in_sampling_rate != stream_function_get_samplingrate(para)) ||
            (stream_ptr->callback.Src.in_resolution != stream_function_get_output_resolution(para))) {
            stream_ptr->callback.Src.in_resolution = stream_function_get_output_resolution(para);
            stream_feature_reinitialize(para);
            DSP_MW_LOG_I("rate %d != %d\r\n", 2, src_feature_ptr->in_sampling_rate, stream_function_get_samplingrate(para));
            DSP_MW_LOG_I("in_resolution %d != %d\r\n", 2, stream_ptr->callback.Src.in_resolution, stream_function_get_output_resolution(para));
        } else {
            //In Ring-Buffer
            if (src_feature_ptr->in_max_size < stream_function_get_output_size(para)) {
                src_feature_ptr->in_max_size = stream_function_get_output_size(para);
            }
#ifdef MTK_HWSRC_IN_STREAM
            if (SRC_Handle_Entry(para,
                                 stream_function_get_1st_inout_buffer(para),
                                 stream_function_get_2nd_inout_buffer(para),
                                 stream_function_get_output_size(para))) {
                out_size = stream_ptr->callback.Src.out_frame_size;

            }

            stream_ptr->callback.Src.inSRC_Full = (src_feature_ptr->inSRC_count > (8 * src_feature_ptr->inSRC_extract_length));
            stream_ptr->callback.Src.outSRC_Full = (src_feature_ptr->outSRC_count > (2 * stream_ptr->callback.Src.out_frame_size * HWSRC_DEFAULT_CHANNEL));
            stream_ptr->callback.EntryPara.src_out_sampling_rate = stream_ptr->callback.Src.out_sampling_rate;
#else

            DSP_SRC_CBufWrite(src_feature_ptr,
                              stream_function_get_1st_inout_buffer(para),
                              stream_function_get_2nd_inout_buffer(para),
                              stream_function_get_output_size(para));

            do {
                in_size = MIN(MIN(src_feature_ptr->buf_count,
                                  src_feature_ptr->inSRC_mem_size - src_feature_ptr->inSRC_count - CALLBACK_SRC_FLOAT_PROTECTION),
                              stream_ptr->callback.EntryPara.out_malloc_size);
                DSP_SRC_CBufRead(src_feature_ptr,
                                 stream_function_get_1st_inout_buffer(para),
                                 stream_function_get_2nd_inout_buffer(para),
                                 in_size);
                /*SRC triger and prco*/
                if (DSP_Callback_SRC_Handle(para,
                                            stream_function_get_1st_inout_buffer(para),
                                            stream_function_get_2nd_inout_buffer(para),
                                            in_size)) {
                    out_size = stream_ptr->callback.Src.out_frame_size;
                    in_size = 0;
                    break;
                }
            } while (in_size > 0);

            stream_ptr->callback.Src.inSRC_Full = ((src_feature_ptr->buf_mem_size - src_feature_ptr->buf_count) < src_feature_ptr->in_max_size);
            stream_ptr->callback.Src.outSRC_Full = (src_feature_ptr->inSRC_count > 2 * src_feature_ptr->inSRC_extract_length);

            stream_ptr->callback.EntryPara.src_out_sampling_rate = stream_ptr->callback.Src.out_sampling_rate;
#endif
            stream_function_modify_output_resolution(para, stream_ptr->callback.Src.out_resolution);
        }
    } else {
        out_size = stream_ptr->callback.EntryPara.codec_out_size;
        stream_ptr->callback.EntryPara.src_out_sampling_rate = stream_ptr->callback.EntryPara.codec_out_sampling_rate;
    }

    stream_ptr->callback.EntryPara.src_out_size = out_size;
#ifdef MTK_HWSRC_IN_STREAM
    return FALSE;
#else
    return (stream_ptr->callback.EntryPara.src_out_size == 0)
           ? TRUE
           : FALSE;
#endif

}
#endif /* MTK_HWSRC_IN_STREAM */

VOID DSP_Callback_SRC_Config(DSP_STREAMING_PARA_PTR stream, stream_feature_type_ptr_t feature_type_ptr, U32 feature_entry_num)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    stream->callback.EntryPara.with_src    = feature_entry_num;
    stream->callback.Src.out_sampling_rate = (*(feature_type_ptr) & 0xFF00) >> 8;
    stream->callback.Src.out_resolution    = (*(feature_type_ptr) & 0x00010000) >> 16;
    stream->callback.Src.in_resolution     = (*(feature_type_ptr) & 0x00020000) >> 17;
    stream->callback.Src.out_frame_size    = (*(feature_type_ptr) & 0xFFFC0000) >> 18;

    if (stream->callback.Src.out_frame_size == 0 && stream->callback.Src.out_sampling_rate == 0) {
        stream->callback.Src.out_resolution = (stream->sink->param.audio.AfeBlkControl.u4asrcflag)
                                              ? Audio_setting->resolution.SRCInRes
                                              : Audio_setting->resolution.AudioOutRes;
        stream->callback.Src.in_resolution = Audio_setting->resolution.AudioInRes;

        stream->callback.Src.out_frame_size = Audio_setting->Audio_sink.Frame_Size;
        stream->callback.Src.out_sampling_rate = AudioSinkSamplingRate_Get();
    }

    *(feature_type_ptr) &= ~0xFFFFFF00;
#else
    UNUSED(stream);
    UNUSED(feature_type_ptr);
    UNUSED(feature_entry_num);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}


ATTR_TEXT_IN_IRAM_LEVEL_1 BOOL DSP_Callback_SRC_Triger_Chk(DSP_CALLBACK_PTR callback_ptr)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    if ((callback_ptr->EntryPara.with_src != FALSE) &&
        (callback_ptr->Src.src_ptr != NULL) &&
        (((callback_ptr->Src.outSRC_Full == TRUE) || (callback_ptr->Src.inSRC_Full == TRUE)) &&
         (callback_ptr->EntryPara.number.field.process_sequence < callback_ptr->EntryPara.with_src))) {      /*SRCin buf full*/
        callback_ptr->EntryPara.in_size = 0;
        callback_ptr->EntryPara.codec_out_size = 0;
        callback_ptr->EntryPara.resolution.process_res = callback_ptr->EntryPara.resolution.feature_res;
        return FALSE;
    } else {
        return TRUE;
    }
#else
    UNUSED(callback_ptr);
    return TRUE;
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}

U32 DSP_calculate_shift_bit(U32 value)
{
    U32 i;
    for (i = 0 ; i < 32 ; i++) {
        if (value == 0) {
            break;
        }
        value = value >> 1;
    }
    return (i > 0) ? i - 1 : 0;
}

U32 dsp_count_bit(U32 value)
{
    U32 count = 0;
    while (value) {
        count++;
        value &= (value - 1) ;
    }
    return count;
}

#define DSP_APPLY_SW_GAIN_CONST_TABLE
const uint32_t dsp_apply_sw_gain_table[] = {
    0,      // 0db 0
    29204,  //-1db 0.8912509381337455299531086810783
    26028,  //-2db 0.79432823472428150206591828283639
    23197,  //-3db 0.70794578438413791080221494218931
    20675,  //-4db 0.63095734448019324943436013662234
    18426,  //-5db 0.56234132519034908039495103977648
    0,      // 0db
};

VOID dsp_apply_sw_gain_16bit(void *ptr, U32 sample, S32 gain_times_of_db)
{
    S16 *buf16_ptr;
    S32 buf;
    U32 i;
    U32 absolute_gain;
    S32 multiplier = 0, times_6db, remain_multiply;

    absolute_gain = (gain_times_of_db >= 0) ? gain_times_of_db : -gain_times_of_db;

    buf16_ptr = ptr;

    times_6db = (absolute_gain / 6);
    if (gain_times_of_db == 0) {
        return;
    } else if (gain_times_of_db > 0) {
        if (absolute_gain % 6) {
            times_6db++;
        }
        remain_multiply = (times_6db) * 6 - absolute_gain;
    } else {
        remain_multiply = absolute_gain - times_6db * 6;
    }
#ifdef DSP_APPLY_SW_GAIN_CONST_TABLE
    multiplier = dsp_apply_sw_gain_table[remain_multiply];
#else
    if (remain_multiply == 1) {
        multiplier = 29204;//-1db 0.8912509381337455299531086810783
    } else if (remain_multiply == 2) {
        multiplier = 26028;//-2db 0.79432823472428150206591828283639
    } else if (remain_multiply == 3) {
        multiplier = 23197;//-3db 0.70794578438413791080221494218931
    } else if (remain_multiply == 4) {
        multiplier = 20675;//-4db 0.63095734448019324943436013662234
    } else if (remain_multiply == 5) {
        multiplier = 18426;//-5db 0.56234132519034908039495103977648
    }
#endif

    for (i = 0 ; i < sample ; i++) {

        buf = (S32)buf16_ptr[i];

        if (gain_times_of_db >= 1) {
            buf = buf << times_6db;
        } else {
            buf = buf >> times_6db;
        }

        if (multiplier) {
            buf = (((S32)buf * multiplier) >> 15); // *(multiplier/32768)
        }

        //overflow check
        if (buf > 0x7FFF) {
            buf = 0x7FFF;
        } else if (buf < (S16)0x8000) {
            buf = 0x8000;
        }
        buf16_ptr[i] = (S16)buf;
    }
}


VOID dsp_apply_sw_gain_32bit(void *ptr, U32 sample, S32 gain_times_of_db)
{
    S32 *buf32_ptr;
    S64 buf;
    U32 i;
    U32 absolute_gain;
    S32 multiplier = 0, times_6db, remain_multiply;

    absolute_gain = (gain_times_of_db >= 0) ? gain_times_of_db : -gain_times_of_db;

    times_6db = (absolute_gain / 6);
    if (gain_times_of_db == 0) {
        return;
    } else if (gain_times_of_db > 0) {
        if (absolute_gain % 6) {
            times_6db++;
        }
        remain_multiply = (times_6db) * 6 - absolute_gain;
    } else {
        remain_multiply = absolute_gain - times_6db * 6;
    }

#ifdef DSP_APPLY_SW_GAIN_CONST_TABLE
    multiplier = dsp_apply_sw_gain_table[remain_multiply];
#else
    if (remain_multiply == 1) {
        multiplier = 1913946815;//-1db 0.8912509381337455299531086810783
    } else if (remain_multiply == 2) {
        multiplier = 1705806895;//-2db 0.79432823472428150206591828283639
    } else if (remain_multiply == 3) {
        multiplier = 1520301995;//-3db 0.70794578438413791080221494218931
    } else if (remain_multiply == 4) {
        multiplier = 1354970579;//-4db 0.63095734448019324943436013662234
    } else if (remain_multiply == 5) {
        multiplier = 1207618800;//-5db 0.56234132519034908039495103977648
    }
#endif

    buf32_ptr = ptr;
    for (i = 0 ; i < sample ; i++) {

        buf = (S64)buf32_ptr[i];

        if (gain_times_of_db >= 1) {
            buf = buf << times_6db;
        } else {
            buf = buf >> times_6db;
        }

        if (multiplier) {
#ifdef DSP_APPLY_SW_GAIN_CONST_TABLE
            buf = (((S64)buf * multiplier) >> 15); // *(multiplier/32768)
#else
            buf = (((S64)buf * multiplier) >> 31); // *(multiplier/2147483648)
#endif
        }

        //overflow check
        if (buf > 0x7FFFFFFF) {
            buf = 0x7FFFFFFF;
        } else if (buf < (S32)0x80000000) {
            buf = 0x80000000;
        }
        buf32_ptr[i] = (S32)buf;
    }
}

#define AFE_MAP_TABLE_MAX_USER 3
#define AFE_MAP_TABLE_MAX_CHANNEL_NUMBER 16

static void *g_mic_map_table_owner[AFE_MAP_TABLE_MAX_USER];
static afe_input_digital_gain_t g_mic_map_table[AFE_MAP_TABLE_MAX_USER][AFE_MAP_TABLE_MAX_CHANNEL_NUMBER];

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void afe_audio_init_input_channel(void *owner, uint32_t channel_number, DSP_STREAMING_PARA_PTR stream_ptr)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    uint32_t audio_device[8];
    uint32_t audio_interface[8];
    AUDIO_PARAMETER *audio_para;
    uint32_t i, index, irq_mask;
#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
    uint8_t mic_mapping_table_base;
#endif

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    for (index = 0; index < AFE_MAP_TABLE_MAX_USER; index++) {
        if (g_mic_map_table_owner[index] == NULL) {
            break;
        }
    }
    if (index >= AFE_MAP_TABLE_MAX_USER) {
        hal_nvic_restore_interrupt_mask(irq_mask);
        DSP_MW_LOG_E("[AFE MAP TABLE] afe_audio_init_input_channel: table is full", 0);
        return;
    }
    g_mic_map_table_owner[index] = owner;
    hal_nvic_restore_interrupt_mask(irq_mask);

    audio_para = &stream_ptr->source->param.audio;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    audio_device[0] = audio_para->audio_device;
    audio_device[1] = audio_para->audio_device1;
    audio_device[2] = audio_para->audio_device2;
    audio_device[3] = audio_para->audio_device3;
    audio_device[4] = audio_para->audio_device4;
    audio_device[5] = audio_para->audio_device5;
    audio_device[6] = audio_para->audio_device6;
    audio_device[7] = audio_para->audio_device7;
    audio_interface[0] = audio_para->audio_interface;
    audio_interface[1] = audio_para->audio_interface1;
    audio_interface[2] = audio_para->audio_interface2;
    audio_interface[3] = audio_para->audio_interface3;
    audio_interface[4] = audio_para->audio_interface4;
    audio_interface[5] = audio_para->audio_interface5;
    audio_interface[6] = audio_para->audio_interface6;
    audio_interface[7] = audio_para->audio_interface7;
#else
    memset(audio_device, 0, sizeof(audio_device));
    audio_device[0] = audio_para->audio_device;
    memset(audio_interface, 0, sizeof(audio_interface));
    audio_interface[0] = audio_para->audio_interface;
#endif

#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
    DSP_FEATURE_TABLE_PTR feature_table_ptr = stream_ptr->callback.FeatureTablePtr;
    stream_feature_type_t mic_fature_type = FUNC_END;
    if (feature_table_ptr) {
        while (feature_table_ptr->FeatureType != FUNC_END) {
            if ((feature_table_ptr->FeatureType >= FUNC_FUNCTION_A) && (feature_table_ptr->FeatureType <= FUNC_FUNCTION_H)) {
                mic_fature_type = (stream_feature_type_t)feature_table_ptr->FeatureType;
                break;
            }
            feature_table_ptr++;
        }
    }
    if (mic_fature_type != FUNC_END) {
        mic_mapping_table_base = INPUT_DIGITAL_GAIN_FOR_SPECIAL_FUNCTION_BASE;
        DSP_MW_LOG_W("[AFE MAP TABLE] DSP Mic Functions type:0x%x", 1, mic_fature_type);
    }
#endif

    if ((stream_ptr->source->type == SOURCE_TYPE_AUDIO) || (stream_ptr->source->type == SOURCE_TYPE_AUDIO2) ||
#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE)
        ((stream_ptr->source->type >= SOURCE_TYPE_SUBAUDIO_MIN) && (stream_ptr->source->type <= SOURCE_TYPE_SUBAUDIO_MAX)))
#else
        (0))
#endif
    {
        if (stream_ptr->source->param.audio.echo_reference == true) {
            channel_number--;
            g_mic_map_table[index][channel_number] = INPUT_DIGITAL_GAIN_FOR_ECHO_PATH;
#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
            if (mic_mapping_table_base) {
                g_mic_map_table[index][channel_number] = INPUT_DIGITAL_GAIN_FOR_SPECIAL_FUNCTION_ECHO;
            }
#endif
            DSP_MW_LOG_I("[AFE MAP TABLE] mic_map_table echo[%d]=%d", 2, channel_number, g_mic_map_table[index][channel_number]);
        }
    }

    for (i = 1; i <= channel_number; i++) {
        if (audio_device[i - 1] != (uint32_t)NULL && audio_interface[i - 1] != (uint32_t)NULL) {
            if ((audio_device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_L) || (audio_device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_L)) {
                if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC0_L;
                } else if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_2) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC1_L;
                } else if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_3) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC2_L;
                } else {
                    DSP_MW_LOG_W("[AFE MAP TABLE] mic L error ch=%d,if=%d", 2, i - 1, audio_interface[i - 1]);
                }
            } else if ((audio_device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_R) || (audio_device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_R)) {
                if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC0_R;
                } else if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_2) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC1_R;
                } else if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_3) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC2_R;
                } else {
                    DSP_MW_LOG_W("[AFE MAP TABLE] mic R error ch=%d,if=%d", 2, i - 1, audio_interface[i - 1]);
                }
            } else if ((audio_device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER) ||
                       (audio_device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE)) {
                if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S0_L;
                } else if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_2) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S1_L;
                } else if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_3) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S2_L;
                } else {
                    DSP_MW_LOG_W("[AFE MAP TABLE] I2S error ch=%d,if=%d", 2, i - 1, audio_interface[i - 1]);
                }
            } else if (audio_device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L) {
                if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S0_L;
                } else if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_2) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S1_L;
                } else if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_3) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S2_L;
                } else {
                    DSP_MW_LOG_W("[AFE MAP TABLE] I2S error ch=%d,if=%d", 2, i - 1, audio_interface[i - 1]);
                }
            } else if (audio_device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R) {
                if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S0_R;
                } else if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_2) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S1_R;
                } else if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_3) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S2_R;
                } else {
                    DSP_MW_LOG_W("[AFE MAP TABLE] I2S error ch=%d,if=%d", 2, i - 1, audio_interface[i - 1]);
                }
            } else if (audio_device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_LINE_IN_L) {
                if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_LINEIN_L;
                } else {
                    DSP_MW_LOG_W("[AFE MAP TABLE] LINE_IN_L error ch=%d,if=%d", 2, i - 1, audio_interface[i - 1]);
                }
            } else if (audio_device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_LINE_IN_R) {
                if (audio_interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    g_mic_map_table[index][i - 1] = INPUT_DIGITAL_GAIN_FOR_LINEIN_R;
                } else {
                    DSP_MW_LOG_W("[AFE MAP TABLE] LINE_IN_R error ch=%d,if=%d", 2, i - 1, audio_interface[i - 1]);
                }
            }
#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
            g_mic_map_table[index][i - 1] += mic_mapping_table_base; //To separate mic scenario and special function gain
#endif
        } else {
            DSP_MW_LOG_I("[AFE MAP TABLE] get null device interface %d", 1, i - 1);
        }
        DSP_MW_LOG_I("[AFE MAP TABLE] ch_num:%d, mic_map_table[%d]=%d, device:0x%x ", 4, channel_number, i - 1, g_mic_map_table[index][i - 1], audio_device[i - 1]);
    }
#else
    UNUSED(owner);
    UNUSED(channel_number);
    UNUSED(stream_ptr);
#endif
}

afe_input_digital_gain_t afe_audio_get_input_channel(void *owner, uint32_t index)
{
    uint32_t i;

    if ((index == 0) || (index > AFE_MAP_TABLE_MAX_CHANNEL_NUMBER)) {
        DSP_MW_LOG_E("[AFE MAP TABLE] afe_audio_get_input_channel: index error %d", 1, index);
        assert(0);
    }

    for (i = 0; i < AFE_MAP_TABLE_MAX_USER; i++) {
        if (g_mic_map_table_owner[i] == owner) {
            break;
        }
    }
    if (i >= AFE_MAP_TABLE_MAX_USER) {
        DSP_MW_LOG_E("[AFE MAP TABLE] afe_audio_get_input_channel: don't find the owner", 0);
        return INPUT_DIGITAL_GAIN_NUM;
    }

    return g_mic_map_table[i][index - 1];
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void afe_audio_deinit_input_channel(void *owner)
{
    uint32_t i, irq_mask;

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    for (i = 0; i < AFE_MAP_TABLE_MAX_USER; i++) {
        if (g_mic_map_table_owner[i] == owner) {
            break;
        }
    }
    if (i >= AFE_MAP_TABLE_MAX_USER) {
        hal_nvic_restore_interrupt_mask(irq_mask);
        DSP_MW_LOG_E("[AFE MAP TABLE] afe_audio_deinit_input_channel: don't find the owner", 0);
        return;
    }

    g_mic_map_table_owner[i] = NULL;
    hal_nvic_restore_interrupt_mask(irq_mask);
}

#ifdef AIR_AUDIO_HARDWARE_ENABLE
#ifdef AIR_SOFTWARE_GAIN_ENABLE
void call_sw_gain_deinit(void)
{
    if (g_call_sw_gain_port != NULL) {
        stream_function_sw_gain_deinit(g_call_sw_gain_port);
        g_call_sw_gain_port = NULL;
        DSP_MW_LOG_I("[SW GAIN] Port deinit for call scenario done", 0);
    }
}
#endif
#endif

typedef void (*sw_gain_entry)(void *ptr, U32 sample, S32 gain_times_of_db);

bool stream_function_gain_initialize(void *para)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    DSP_STREAMING_PARA_PTR stream_ptr;
    U8 *mic_map_table;
    uint32_t i, channel_number;
    U8 mic_mapping_table_base;
    mic_mapping_table_base = 0;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    mic_map_table = stream_function_get_working_buffer(para);
    channel_number = stream_function_get_channel_number(para);

#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
    DSP_FEATURE_TABLE_PTR feature_table_ptr = stream_ptr->callback.FeatureTablePtr;
    stream_feature_type_t mic_fature_type = FUNC_END;
    if (feature_table_ptr) {
        while (feature_table_ptr->FeatureType != FUNC_END) {
            if ((feature_table_ptr->FeatureType >= FUNC_FUNCTION_A) && (feature_table_ptr->FeatureType <= FUNC_FUNCTION_H)) {
                mic_fature_type = (stream_feature_type_t)feature_table_ptr->FeatureType;
                break;
            }
            feature_table_ptr++;
        }
    }
    if (mic_fature_type != FUNC_END) {
        mic_mapping_table_base = INPUT_DIGITAL_GAIN_FOR_SPECIAL_FUNCTION_BASE;
        DSP_MW_LOG_W("[SW GAIN]DSP Mic Functions type:0x%x", 1, mic_fature_type);
    }
#endif

#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    uint32_t device[] = {stream_ptr->source->param.audio.audio_device, stream_ptr->source->param.audio.audio_device1, stream_ptr->source->param.audio.audio_device2, stream_ptr->source->param.audio.audio_device3,
                         stream_ptr->source->param.audio.audio_device4, stream_ptr->source->param.audio.audio_device5, stream_ptr->source->param.audio.audio_device6, stream_ptr->source->param.audio.audio_device7
                        };
    uint32_t interface[] = {stream_ptr->source->param.audio.audio_interface, stream_ptr->source->param.audio.audio_interface1, stream_ptr->source->param.audio.audio_interface2, stream_ptr->source->param.audio.audio_interface3,
                 stream_ptr->source->param.audio.audio_interface4, stream_ptr->source->param.audio.audio_interface5, stream_ptr->source->param.audio.audio_interface6, stream_ptr->source->param.audio.audio_interface7
    };
#else
    uint32_t device[] = {stream_ptr->source->param.audio.audio_device, 0, 0, 0, 0, 0, 0, 0};
    uint32_t interface[] = {stream_ptr->source->param.audio.audio_interface, 0, 0, 0, 0, 0, 0, 0};
#endif

    if ((stream_ptr->source->type == SOURCE_TYPE_AUDIO) || (stream_ptr->source->type == SOURCE_TYPE_AUDIO2) ||
#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE)
        ((stream_ptr->source->type >= SOURCE_TYPE_SUBAUDIO_MIN) && (stream_ptr->source->type <= SOURCE_TYPE_SUBAUDIO_MAX)))
#else
        (0))
#endif
    {
        if (stream_ptr->source->param.audio.echo_reference) {
            channel_number--;
            mic_map_table[channel_number] = INPUT_DIGITAL_GAIN_FOR_ECHO_PATH;
#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
            if (mic_mapping_table_base) {
                mic_map_table[channel_number] = INPUT_DIGITAL_GAIN_FOR_SPECIAL_FUNCTION_ECHO;
            }
#endif
            DSP_MW_LOG_I("[SW GAIN] mic_map_table echo[%d]=%d", 2, channel_number, mic_map_table[channel_number]);
        }
    }

    for (i = 1 ; i <= channel_number ; i++) {
        if (device[i - 1] != (uint32_t)NULL && interface[i - 1] != (uint32_t)NULL) {
            if ((device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_L) || (device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_L)) {
                if (interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC0_L;
                } else if (interface[i - 1] == HAL_AUDIO_INTERFACE_2) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC1_L;
                } else if (interface[i - 1] == HAL_AUDIO_INTERFACE_3) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC2_L;
                } else {
                    DSP_MW_LOG_W("[SW GAIN] mic L error ch=%d,if=%d", 2, i - 1, interface[i - 1]);
                }

            } else if ((device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_R) || (device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_R)) {
                if (interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC0_R;
                } else if (interface[i - 1] == HAL_AUDIO_INTERFACE_2) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC1_R;
                } else if (interface[i - 1] == HAL_AUDIO_INTERFACE_3) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_MIC2_R;
                } else {
                    DSP_MW_LOG_W("[SW GAIN] mic R error ch=%d,if=%d", 2, i - 1, interface[i - 1]);
                }
            } else if ((device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER) ||
                       (device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) ||
                       (device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L)) {

                if (interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S0_L;
                } else if (interface[i - 1] == HAL_AUDIO_INTERFACE_2) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S1_L;
                } else if (interface[i - 1] == HAL_AUDIO_INTERFACE_3) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S2_L;
                } else {
                    DSP_MW_LOG_W("[SW GAIN] I2S error ch=%d,if=%d", 2, i - 1, interface[i - 1]);
                }
            } else if (device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R) {
                if (interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S0_R;
                } else if (interface[i - 1] == HAL_AUDIO_INTERFACE_2) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S1_R;
                } else if (interface[i - 1] == HAL_AUDIO_INTERFACE_3) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_I2S2_R;
                } else {
                    DSP_MW_LOG_W("[SW GAIN] I2S error ch=%d,if=%d", 2, i - 1, interface[i - 1]);
                }
            } else if (device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_LINE_IN_L) {
                if (interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_LINEIN_L;
                } else {
                    DSP_MW_LOG_W("[SW GAIN] LINE_IN_L error ch=%d,if=%d", 2, i - 1, interface[i - 1]);
                }

            } else if (device[i - 1] == HAL_AUDIO_CONTROL_DEVICE_LINE_IN_R) {
                if (interface[i - 1] == HAL_AUDIO_INTERFACE_1) {
                    mic_map_table[i - 1] = INPUT_DIGITAL_GAIN_FOR_LINEIN_R;
                } else {
                    DSP_MW_LOG_W("[SW GAIN] LINE_IN_R error ch=%d,if=%d", 2, i - 1, interface[i - 1]);
                }
            } else {

            }
#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
            mic_map_table[i - 1] += mic_mapping_table_base; //To separate mic scenario and special function gain
#endif
        } else {
            DSP_MW_LOG_I("[SW GAIN] get null device interface %d", 1, i - 1);
        }
        DSP_MW_LOG_I("[SW GAIN] ch_num:%d, mic_map_table[%d]=%d, device:0x%x ", 4, channel_number, i - 1, mic_map_table[i - 1], device[i - 1]);
    }

#ifdef AIR_SOFTWARE_GAIN_ENABLE
    int32_t init_digital_gain;
    sw_gain_config_t default_config;

#ifdef AIR_BT_CODEC_BLE_ENABLED
#ifdef AIR_BLE_UL_SW_GAIN_CONTROL_ENABLE
    if((stream_ptr->sink->type == SINK_TYPE_N9SCO) || ((stream_ptr->sink->type == SINK_TYPE_N9BLE) && (stream_ptr->sink->param.n9ble.codec_type != BT_BLE_CODEC_LC3)))
#else
    if((stream_ptr->sink->type == SINK_TYPE_N9SCO) || (stream_ptr->sink->type == SINK_TYPE_N9BLE))
#endif
#else
    if (stream_ptr->sink->type == SINK_TYPE_N9SCO)
#endif
    {
        if (g_call_sw_gain_port != NULL) {
            DSP_MW_LOG_I("[SW GAIN] Port is occupied, deinit first", 0);
            stream_function_sw_gain_deinit(g_call_sw_gain_port);
        }
        init_digital_gain = (int32_t)afe_audio_get_input_digital_gain(mic_map_table[0]);
        default_config.resolution = stream_function_get_output_resolution(para);
        default_config.target_gain = init_digital_gain;
        default_config.up_step = 25;
        default_config.up_samples_per_step = 48;
        default_config.down_step = -25;
        default_config.down_samples_per_step = 48;
        g_call_sw_gain_port = stream_function_sw_gain_get_port(stream_ptr->sink);
        channel_number = stream_function_get_channel_number(para);
        stream_function_sw_gain_init(g_call_sw_gain_port, channel_number, &default_config);
        stream_function_sw_gain_initialize(para);
        DSP_MW_LOG_I("[SW GAIN] Port init done for call", 0);
    }
#endif

#else
    UNUSED(para);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

    return 0;
}

#ifdef AIR_HFP_DNN_PATH_ENABLE
int32_t digital_gain_times_of_db_last_8;
int32_t digital_gain_times_of_db_last_9;
void gain_change_detect(U8 mic_map_table)
{
    if (mic_map_table == 8) {
        if (digital_gain_times_of_db_last_8 != afe_audio_get_input_digital_gain(mic_map_table)) {
            DSP_MW_LOG_I("[stream_function_gain_process] mic_map_table[%d] gain update to 0x%x", 2, mic_map_table, afe_audio_get_input_digital_gain(mic_map_table));
        }
        digital_gain_times_of_db_last_8 = afe_audio_get_input_digital_gain(mic_map_table);
    } else if (mic_map_table == 9) {
        if (digital_gain_times_of_db_last_9 != afe_audio_get_input_digital_gain(mic_map_table)) {
            DSP_MW_LOG_I("[stream_function_gain_process] mic_map_table[%d] gain update to 0x%x", 2, mic_map_table, afe_audio_get_input_digital_gain(mic_map_table));
        }
        digital_gain_times_of_db_last_9 = afe_audio_get_input_digital_gain(mic_map_table);
    }
}
#endif
bool stream_function_gain_process(void *para)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    uint32_t i, channel_number;
    int32_t digital_gain_times_of_db;
    DSP_STREAMING_PARA_PTR stream_ptr;
    sw_gain_entry gain_entry;
    uint32_t sample;
    U8 *mic_map_table;
    bool is_truncate_to_16_bit = false;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    mic_map_table = stream_function_get_working_buffer(para);
    channel_number = stream_function_get_channel_number(para);

    if (stream_function_get_output_resolution(para) == RESOLUTION_16BIT) {
        gain_entry = dsp_apply_sw_gain_16bit;
        sample = stream_function_get_output_size(para) / sizeof(S16);
    } else {
        gain_entry = dsp_apply_sw_gain_32bit;
        sample = stream_function_get_output_size(para) / sizeof(S32);
    }

#ifdef AIR_SOFTWARE_GAIN_ENABLE
    bool is_mute = 0;

#ifdef AIR_BT_CODEC_BLE_ENABLED
#ifdef AIR_BLE_UL_SW_GAIN_CONTROL_ENABLE
    if((stream_ptr->sink->type == SINK_TYPE_N9SCO) || ((stream_ptr->sink->type == SINK_TYPE_N9BLE) && (stream_ptr->sink->param.n9ble.codec_type != BT_BLE_CODEC_LC3)))
#else
    if((stream_ptr->sink->type == SINK_TYPE_N9SCO) || (stream_ptr->sink->type == SINK_TYPE_N9BLE))
#endif
#else
    if (stream_ptr->sink->type == SINK_TYPE_N9SCO)
#endif
        {
        is_mute = g_call_mute_flag;
        for (i = 1; i <= channel_number; i++) {
            if (is_mute == true) {
                digital_gain_times_of_db = SW_GAIN_MUTE_VALUE;
            } else {
                digital_gain_times_of_db = (int32_t)afe_audio_get_input_digital_gain(mic_map_table[i - 1]);
            }
            stream_function_sw_gain_configure_gain_target(g_call_sw_gain_port, i, digital_gain_times_of_db);
        }
        stream_function_sw_gain_process(para);

#ifdef AIR_UL_FIX_RESOLUTION_32BIT
        //truncate for speech lib
        if (stream_function_get_output_resolution(para) == RESOLUTION_32BIT) {
            for (i = 1; i <= channel_number; i++) {
                dsp_converter_32bit_to_16bit((S16 *)stream_function_get_inout_buffer(para, i),
                                                     (S32 *)stream_function_get_inout_buffer(para, i),
                                                     stream_function_get_output_size(para) / sizeof(S32));
            }
            stream_function_modify_output_size(para, stream_function_get_output_size(para) / 2);
            stream_function_modify_output_resolution(para, RESOLUTION_16BIT);
        }
#endif

#ifdef AIR_AUDIO_DUMP_ENABLE
        LOG_AUDIO_DUMP((U8*)(stream_function_get_inout_buffer(para, 1)), (U32)(stream_function_get_output_size(para)), SOURCE_IN3);
        if(channel_number > 1) {
            LOG_AUDIO_DUMP((U8*)(stream_function_get_inout_buffer(para, 2)), (U32)(stream_function_get_output_size(para)), SOURCE_IN4);
        }
#endif

        return 0;
    }
#endif

#ifdef AIR_DCHS_MODE_ENABLE
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    if (stream_ptr->sink->type == SINK_TYPE_DSP_VIRTUAL){
        is_mute = g_call_mute_flag;
    }
#endif
#endif
    for (i = 1 ; i <= channel_number ; i++) {
#ifdef AIR_DCHS_MODE_ENABLE
#ifdef AIR_SOFTWARE_GAIN_ENABLE
        if (is_mute == true) {
            digital_gain_times_of_db = (SW_GAIN_MUTE_VALUE / 100);
        }else
#endif
#endif
        {
            digital_gain_times_of_db = (int32_t)((S16)afe_audio_get_input_digital_gain(mic_map_table[i - 1]) / 100);
        }
#ifdef AIR_HFP_DNN_PATH_ENABLE
        if (i == 1) {
            gain_change_detect(mic_map_table[i - 1]);
        }
#endif
        gain_entry(stream_function_get_inout_buffer(para, i),
                   sample,
                   digital_gain_times_of_db);


#ifdef AIR_UL_FIX_RESOLUTION_32BIT
        //truncate for speech lib
        if ((stream_ptr->source->scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL) ||
            (stream_ptr->source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL) ||
            (stream_ptr->source->scenario_type == AUDIO_SCENARIO_TYPE_WWE) ||
            (stream_ptr->source->scenario_type == AUDIO_SCENARIO_TYPE_RECORD)) {

            if (stream_function_get_output_resolution(para) == RESOLUTION_32BIT) {
                dsp_converter_32bit_to_16bit((S16 *)stream_function_get_inout_buffer(para, i),
                                             (S32 *)stream_function_get_inout_buffer(para, i),
                                             stream_function_get_output_size(para) / sizeof(S32));
                is_truncate_to_16_bit = true;

            }
        }

#endif
    }


    if (is_truncate_to_16_bit) {
        stream_function_modify_output_size(para, stream_function_get_output_size(para) / 2);
        stream_function_modify_output_resolution(para, RESOLUTION_16BIT);
    }


#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)(stream_function_get_inout_buffer(para, 1)), (U32)(stream_function_get_output_size(para)), SOURCE_IN3);
    if (channel_number > 1) {
        LOG_AUDIO_DUMP((U8 *)(stream_function_get_inout_buffer(para, 2)), (U32)(stream_function_get_output_size(para)), SOURCE_IN4);
        if(channel_number > 2) {
            LOG_AUDIO_DUMP((U8*)(stream_function_get_inout_buffer(para, 3)), (U32)(stream_function_get_output_size(para)), SOURCE_IN2);
        }
    }
#endif
    //DSP_MW_LOG_I("SOURCE_IN3 0x%x SOURCE_IN4 0x%x",2,stream_function_get_inout_buffer(para, 1),stream_function_get_inout_buffer(para, 2));

#else
    UNUSED(para);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

    return 0;
}


#ifdef MTK_SPECIAL_FUNCTIONS_ENABLE
/**
 * Fixed gain table for mic functions
 * Gain setting depend on input sequence
 * unit:db
 */
const int32_t dsp_fixed_sw_gain_table[] = {
    0,//1st input channel
    0,//2nd input channel
    0,//3rd input channel
    0,//4th input channel
};

bool stream_function_fixed_gain_initialize(void *para)
{
    UNUSED(para);
    return false;
}

bool stream_function_fixed_gain_process(void *para)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    uint32_t i, channel_number;
    DSP_STREAMING_PARA_PTR stream_ptr;
    sw_gain_entry gain_entry;
    uint32_t sample;
    U8 *mic_map_table;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    mic_map_table = stream_function_get_working_buffer(para);
    channel_number = stream_function_get_channel_number(para);

    if (stream_function_get_output_resolution(para) == RESOLUTION_16BIT) {
        gain_entry = dsp_apply_sw_gain_16bit;
        sample = stream_function_get_output_size(para) / sizeof(S16);
    } else {
        gain_entry = dsp_apply_sw_gain_32bit;
        sample = stream_function_get_output_size(para) / sizeof(S32);
    }

    for (i = 1 ; i <= channel_number ; i++) {
        gain_entry(stream_function_get_inout_buffer(para, i),
                   sample,
                   dsp_fixed_sw_gain_table[i - 1]);
    }
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

    return false;
}

#endif

#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
/**
 * Fixed gain table for tdm functions
 * Gain setting depend on input sequence
 * unit:db
 */
const int32_t dsp_fixed_sw_gain_tdm_table[] = {
    0,//1st input channel
    0,//2nd input channel
    0,//3rd input channel
    0,//4th input channel
    0,//5st input channel
    0,//6nd input channel
    0,//7rd input channel
    0,//8th input channel
};

bool stream_function_fixed_gain_tdm_initialize(void *para)
{
    UNUSED(para);
    return false;
}

bool stream_function_fixed_gain_tdm_process(void *para)
{
    uint32_t i, channel_number;
    DSP_STREAMING_PARA_PTR stream_ptr;
    sw_gain_entry gain_entry;
    uint32_t sample;
    U8 *mic_map_table;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    mic_map_table = stream_function_get_working_buffer(para);
    channel_number = stream_function_get_channel_number(para);

    if (stream_function_get_output_resolution(para) == RESOLUTION_16BIT) {
        gain_entry = dsp_apply_sw_gain_16bit;
        sample = stream_function_get_output_size(para) / sizeof(S16);
    } else {
        gain_entry = dsp_apply_sw_gain_32bit;
        sample = stream_function_get_output_size(para) / sizeof(S32);
    }

    for (i = 1 ; i <= channel_number ; i++) {
        if (i == 1) {
            LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, i), (U32)stream_function_get_output_size(para), SOURCE_IN1);
        }
        if (i == 2) {
            LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, i), (U32)stream_function_get_output_size(para), SOURCE_IN2);
        }
        if (i == 3) {
            LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, i), (U32)stream_function_get_output_size(para), SOURCE_IN3);
        }
        if (i == 4) {
            LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, i), (U32)stream_function_get_output_size(para), SOURCE_IN4);
        }
        if (i == 5) {
            LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, i), (U32)stream_function_get_output_size(para), VOICE_TX_MIC_0);
        }
        if (i == 6) {
            LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, i), (U32)stream_function_get_output_size(para), VOICE_TX_MIC_1);
        }
        if (i == 7) {
            LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, i), (U32)stream_function_get_output_size(para), VOICE_TX_MIC_2);
        }
        if (i == 8) {
            LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, i), (U32)stream_function_get_output_size(para), VOICE_TX_MIC_3);
        }
        gain_entry(stream_function_get_inout_buffer(para, i),
                   sample,
                   dsp_fixed_sw_gain_tdm_table[i - 1]);
    }

    return false;
}

#endif

static copy_to_virtual_source_port_t copy_to_virtual_source_port[COPY_TO_VIRTUAL_SOURCE_PORT_MAX];;
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ copy_to_virtual_source_port_t *stream_function_copy_to_virtual_source_get_port(void *owner)
{
    int32_t i;
    copy_to_virtual_source_port_t *port = NULL;
    uint32_t saved_mask;

    /* Find out a port for this owner */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    for (i = COPY_TO_VIRTUAL_SOURCE_PORT_MAX - 1; i >= 0; i--) {
        /* Check if there is unused port */
        if (copy_to_virtual_source_port[i].owner == NULL) {
            port = &copy_to_virtual_source_port[i];
            continue;
        }

        /* Check if this owner has already owned a sw gain */
        if (copy_to_virtual_source_port[i].owner == owner) {
            port = &copy_to_virtual_source_port[i];
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (port == NULL) {
        DSP_MW_LOG_E("[COPY_TO_VIRTUAL_SOURCE] Port not enough!", 0);
        return port;
    }

    port->owner = owner;

    return port;
}

ATTR_TEXT_IN_IRAM static copy_to_virtual_source_port_t *stream_function_copy_to_virtual_source_find_out_port(DSP_STREAMING_PARA_PTR stream_ptr)
{
    int32_t i;
    copy_to_virtual_source_port_t *port = NULL;

    for (i = COPY_TO_VIRTUAL_SOURCE_PORT_MAX - 1; i >= 0; i--) {
        /* Check if this source or sink has already owned a sw gain */
        if ((copy_to_virtual_source_port[i].owner == stream_ptr->source) ||
            (copy_to_virtual_source_port[i].owner == stream_ptr->sink)) {
            port = &copy_to_virtual_source_port[i];
            break;
        }
    }
    if (port == NULL) {
        DSP_MW_LOG_E("[COPY_TO_VIRTUAL_SOURCE] Port is not found!", 0);
        AUDIO_ASSERT(FALSE);
    }
    return port;
}

void stream_function_copy_to_virtual_source_init(copy_to_virtual_source_port_t *port, VIRTUAL_SOURCE_TYPE virtual_source_type)
{
    /* check port */
    if (port == NULL) {
        DSP_MW_LOG_E("[COPY_TO_VIRTUAL_SOURCE] Port is NULL!", 0);
        AUDIO_ASSERT(FALSE);
    }
    port->virtual_source_type = virtual_source_type;
}

void stream_function_copy_to_virtual_source_deinit(copy_to_virtual_source_port_t *port)
{
    /* check port */
    if (port == NULL) {
        DSP_MW_LOG_E("[COPY_TO_VIRTUAL_SOURCE] Port is NULL!", 0);
        AUDIO_ASSERT(FALSE);
    }
    port->owner = NULL;
}

bool stream_copy_to_virtual_source_initialize(void *para)
{
    UNUSED(para);
    return 0;
}

bool stream_copy_to_virtual_sourc_process(void *para)
{
    UNUSED(para);
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    copy_to_virtual_source_port_t *port;
    port = stream_function_copy_to_virtual_source_find_out_port(stream_ptr);

    if (port->virtual_source_type == VIRTUAL_SOURCE_ZERO_DATA) {
        /* bypass */
    } else {
        U32 source_type;
        for(source_type = SOURCE_TYPE_DSP_VIRTUAL_MIN; source_type<=SOURCE_TYPE_DSP_VIRTUAL_MAX; source_type++){
            U32 i, in_ChNum;
            in_ChNum = 0;
            if((Source_blks[source_type] == NULL)||Source_blks[source_type]->transform == NULL){
                continue;
            }

            if(port->virtual_source_type == Source_blks[source_type]->param.virtual_para.virtual_source_type){
                if(Source_blks[source_type]->param.virtual_para.is_dummy_data != true){
                    DSP_MW_LOG_E("stream_copy_to_virtual_sourc_process last data still not used",0);
                    continue;
                }
                void *virtual_para = (void *)&((DSP_Callback_Get(Source_blks[source_type], Source_blks[source_type]->transform->sink))->EntryPara);

                U8 channel_num = stream_codec_get_output_channel_number(para) < stream_codec_get_output_channel_number(virtual_para) ?
                                    stream_codec_get_output_channel_number(para) : stream_codec_get_output_channel_number(virtual_para);
                U32 output_size = stream_function_get_output_size(para) < Source_blks[source_type]->param.virtual_para.mem_size ?
                                    stream_function_get_output_size(para) : Source_blks[source_type]->param.virtual_para.mem_size;
                for (i = 0 ; i < channel_num ; i++) {
                    if(Source_blks[source_type]->streamBuffer.BufferInfo.startaddr[i] != NULL){
                        memcpy(Source_blks[source_type]->streamBuffer.BufferInfo.startaddr[i],
                                stream_function_get_inout_buffer(para, i+1),
                                output_size);
                        if(i==0){
                            #include "dsp_dump.h"
                            //LOG_AUDIO_DUMP(stream_function_get_inout_buffer(para, i+1), output_size, AUDIO_DUMP_TEST_ID_3);
                        }
                    }
                }
                #ifdef AIR_WIRED_AUDIO_ENABLE
                if(stream_ptr->source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT){
                    if ((stream_codec_get_output_channel_number(para) == 1) && (stream_codec_get_output_channel_number(virtual_para) > 1))
                    {
                        for (i = 1 ; i < stream_codec_get_output_channel_number(virtual_para) ; i++) {
                            if(Source_blks[source_type]->streamBuffer.BufferInfo.startaddr[i] != NULL){
                                memcpy(Source_blks[source_type]->streamBuffer.BufferInfo.startaddr[i],
                                        stream_function_get_inout_buffer(para, 1),
                                        output_size);
                            }
                        }
                    }
                }
                #endif/* AIR_WIRED_AUDIO_ENABLE */
                //Source_blks[source_type]->param.virtual_para.channel_num = ((DSP_ENTRY_PARA_PTR)para)->in_channel_num;
                Source_blks[source_type]->param.virtual_para.data_size = output_size;
                Source_blks[source_type]->param.virtual_para.data_samplingrate = stream_function_get_samplingrate(para);
                Source_blks[source_type]->param.virtual_para.is_dummy_data = false;
            }
        }
    }

    // DSP_MW_LOG_I("stream_copy_to_virtual_sourc_process data_size %d, data_samplingrate %d, is_dummy_data %d",3,
    //     output_size, stream_function_get_samplingrate(para), false);
    return 0;
}

#ifdef AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE

sw_gain_port_t *g_DL_SW_gain_port = NULL;
sw_gain_port_t *g_DL_SW_gain_port_2 = NULL;
sw_gain_port_t *g_DL_SW_gain_port_3 = NULL;
static uint32_t g_DL_SW_main_gain = 0;
static uint32_t g_DL_SW_main_gain_2 = 0;
static uint32_t g_DL_SW_main_gain_3 = 0;
uint32_t g_DL_SW_main_gain_temp = 0;
uint32_t g_DL_SW_main_gain_temp_2 = 0;
uint32_t g_DL_SW_main_gain_temp_3 = 0;
static uint32_t g_LR_balance_gain_l = 0;
static uint32_t g_LR_balance_gain_r = 0;
uint32_t g_DL_SW_gain_l_temp = 0;
uint32_t g_DL_SW_gain_r_temp = 0;

dl_sw_gain_default_para_t g_DL_SW_gain_default_para;

static void DL_SW_update_gain(void)
{
    if(g_LR_balance_gain_l != g_DL_SW_gain_l_temp){
        if ((g_DL_SW_gain_port != NULL) && (Sink_blks[SINK_TYPE_AUDIO]!= NULL)) {
            DSP_MW_LOG_I("[DL_SW_GAIN] L channel, change from %d to %d, gain1 L %ddB", 3, g_LR_balance_gain_l, g_DL_SW_gain_l_temp,(int32_t)g_DL_SW_main_gain + (int32_t)g_DL_SW_gain_l_temp);
            stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port, 1, (int32_t)g_DL_SW_main_gain + (int32_t)g_DL_SW_gain_l_temp);
        }
        if ((g_DL_SW_gain_port_2 != NULL) && (Sink_blks[SINK_TYPE_VP_AUDIO]!= NULL)) {
            DSP_MW_LOG_I("[DL_SW_GAIN] L channel, change from %d to %d, gain2 L %ddB", 3, g_LR_balance_gain_l, g_DL_SW_gain_l_temp,(int32_t)g_DL_SW_main_gain_2 + (int32_t)g_DL_SW_gain_l_temp);
            stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port_2, 1, (int32_t)g_DL_SW_main_gain_2 + (int32_t)g_DL_SW_gain_l_temp);
        }
        if ((g_DL_SW_gain_port_3 != NULL) && (Sink_blks[SINK_TYPE_AUDIO_DL3]!= NULL)) {
            DSP_MW_LOG_I("[DL_SW_GAIN] L channel, change from %d to %d, gain3 L %ddB", 3, g_LR_balance_gain_l, g_DL_SW_gain_l_temp,(int32_t)g_DL_SW_main_gain_3 + (int32_t)g_DL_SW_gain_l_temp);
            stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port_3, 1, (int32_t)g_DL_SW_main_gain_3 + (int32_t)g_DL_SW_gain_l_temp);
        }
        g_LR_balance_gain_l = g_DL_SW_gain_l_temp;
    }
    if(g_LR_balance_gain_r != g_DL_SW_gain_r_temp){
        if ((g_DL_SW_gain_port != NULL) && (Sink_blks[SINK_TYPE_AUDIO]!= NULL)) {
            DSP_MW_LOG_I("[DL_SW_GAIN] R channel, change from %d to %d, gain1 R %ddB", 3, g_LR_balance_gain_r, g_DL_SW_gain_r_temp,(int32_t)g_DL_SW_main_gain + (int32_t)g_DL_SW_gain_r_temp);
            stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port, 2, (int32_t)g_DL_SW_main_gain + (int32_t)g_DL_SW_gain_r_temp);
        }
        if ((g_DL_SW_gain_port_2 != NULL) && (Sink_blks[SINK_TYPE_VP_AUDIO]!= NULL)) {
            DSP_MW_LOG_I("[DL_SW_GAIN] R channel, change from %d to %d, gain2 R %ddB", 3, g_LR_balance_gain_r, g_DL_SW_gain_r_temp,(int32_t)g_DL_SW_main_gain_2 + (int32_t)g_DL_SW_gain_r_temp);
            stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port_2, 2, (int32_t)g_DL_SW_main_gain_2 + (int32_t)g_DL_SW_gain_r_temp);
        }
        if ((g_DL_SW_gain_port_3 != NULL) && (Sink_blks[SINK_TYPE_AUDIO_DL3]!= NULL)) {
            DSP_MW_LOG_I("[DL_SW_GAIN] R channel, change from %d to %d, gain3 R %ddB", 3, g_LR_balance_gain_r, g_DL_SW_gain_r_temp,(int32_t)g_DL_SW_main_gain_3 + (int32_t)g_DL_SW_gain_r_temp);
            stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port_3, 2, (int32_t)g_DL_SW_main_gain_3 + (int32_t)g_DL_SW_gain_r_temp);
        }
        g_LR_balance_gain_r = g_DL_SW_gain_r_temp;
    }

    if((g_DL_SW_main_gain != g_DL_SW_main_gain_temp) && (g_DL_SW_gain_port != NULL)){
        DSP_MW_LOG_I("[DL_SW_GAIN] main gain, change from %d to %d, L %ddB, R %ddB", 4, g_DL_SW_main_gain, g_DL_SW_main_gain_temp, (int32_t)g_DL_SW_main_gain_temp + (int32_t)g_LR_balance_gain_l, (int32_t)g_DL_SW_main_gain_temp + (int32_t)g_LR_balance_gain_r);
        g_DL_SW_main_gain = g_DL_SW_main_gain_temp;
        stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port, 1, (int32_t)g_DL_SW_main_gain + (int32_t)g_LR_balance_gain_l);
        stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port, 2, (int32_t)g_DL_SW_main_gain + (int32_t)g_LR_balance_gain_r);
    }
    if((g_DL_SW_main_gain_2 != g_DL_SW_main_gain_temp_2) && (g_DL_SW_gain_port_2 != NULL)){
        DSP_MW_LOG_I("[DL_SW_GAIN] main gain 2, change from %d to %d, L %ddB, R %ddB", 4, g_DL_SW_main_gain_2, g_DL_SW_main_gain_temp_2, (int32_t)g_DL_SW_main_gain_temp_2 + (int32_t)g_LR_balance_gain_l, (int32_t)g_DL_SW_main_gain_temp_2 + (int32_t)g_LR_balance_gain_r);
        g_DL_SW_main_gain_2 = g_DL_SW_main_gain_temp_2;
        stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port_2, 1, (int32_t)g_DL_SW_main_gain_2 + (int32_t)g_LR_balance_gain_l);
        if(g_DL_SW_gain_port_2->total_channels > 1){
            stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port_2, 2, (int32_t)g_DL_SW_main_gain_2 + (int32_t)g_LR_balance_gain_r);
        }
    }
    if((g_DL_SW_main_gain_3 != g_DL_SW_main_gain_temp_3) && (g_DL_SW_gain_port_3 != NULL)){
        DSP_MW_LOG_I("[DL_SW_GAIN] main gain 3, change from %d to %d, L %ddB, R %ddB", 4, g_DL_SW_main_gain, g_DL_SW_main_gain_temp_3, (int32_t)g_DL_SW_main_gain_temp_3 + (int32_t)g_LR_balance_gain_l, (int32_t)g_DL_SW_main_gain_temp_3 + (int32_t)g_LR_balance_gain_r);
        g_DL_SW_main_gain_3 = g_DL_SW_main_gain_temp_3;
        stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port_3, 1, (int32_t)g_DL_SW_main_gain_3 + (int32_t)g_LR_balance_gain_l);
        stream_function_sw_gain_configure_gain_target(g_DL_SW_gain_port_3, 2, (int32_t)g_DL_SW_main_gain_3 + (int32_t)g_LR_balance_gain_r);
    }
}

bool stream_function_DL_SW_initialize(void *para)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    DSP_STREAMING_PARA_PTR stream_ptr;
    U8* mic_map_table;
    uint32_t channel_number;
    U8 mic_mapping_table_base;
    mic_mapping_table_base = 0;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    mic_map_table = stream_function_get_working_buffer(para);
    channel_number = stream_function_get_channel_number(para);

    if(((g_DL_SW_gain_default_para.enable_vp == false) && ((stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_VP) || (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_VP_PRE) || (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_VP_DUMMY_PRE) || (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_VP_DUMMY)))
        || ((g_DL_SW_gain_default_para.enable_a2dp == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_A2DP))
        || ((g_DL_SW_gain_default_para.enable_hfp == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL))
        || ((g_DL_SW_gain_default_para.enable_ble_music == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL))
        || ((g_DL_SW_gain_default_para.enable_ble_call == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL))
        || ((g_DL_SW_gain_default_para.enable_ull_music == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL))
        || ((g_DL_SW_gain_default_para.enable_ull_call == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL))
        || ((g_DL_SW_gain_default_para.enable_line_in == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN))
        || ((g_DL_SW_gain_default_para.enable_usb_in == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0))
        || ((g_DL_SW_gain_default_para.enable_usb_in == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1))
    ){
        return 0;
    }
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    // for HW gain 1
    int32_t init_digital_gain;
    sw_gain_config_t default_config;

    sw_gain_port_t **sw_gain_port = NULL;
    uint32_t *main_gain = 0;

    if(stream_ptr->sink->type == SINK_TYPE_VP_AUDIO){
        // for HW gain 2, vp, no need reinit port
        sw_gain_port = &g_DL_SW_gain_port_2;
        main_gain = &g_DL_SW_main_gain_2;
    } else if(stream_ptr->sink->type >= SINK_TYPE_AUDIO_DL3){
        // for HW gain 3, line in
        sw_gain_port = &g_DL_SW_gain_port_3;
        main_gain = &g_DL_SW_main_gain_3;
        channel_number = 2;//stream_function_get_channel_number(para);
    } else if(stream_ptr->sink->type >= SINK_TYPE_AUDIO) {
        // for HW gain 1
        sw_gain_port = &g_DL_SW_gain_port;
        main_gain = &g_DL_SW_main_gain;
        channel_number = 2;//stream_function_get_channel_number(para);
    }

    // if ((*sw_gain_port != NULL) && ((*sw_gain_port)->owner != stream_ptr->sink)) {
    //     DSP_MW_LOG_I("[DL_SW_GAIN] Port 0x%x is occupied, deinit first", 1, *sw_gain_port);
    //     stream_function_sw_gain_deinit(*sw_gain_port);
    // }

    init_digital_gain = -9600;
    default_config.resolution = stream_function_get_output_resolution(para);//RESOLUTION_16BIT;
    default_config.current_gain = init_digital_gain;
    default_config.target_gain = init_digital_gain;
    default_config.up_step = 25;
    default_config.up_samples_per_step = 4;
    default_config.down_step = -25;
    default_config.down_samples_per_step = 4;
    if((*sw_gain_port == NULL)||((*sw_gain_port != NULL) && ((SINK)((*sw_gain_port)->owner))->type != stream_ptr->sink->type)){
        *sw_gain_port = stream_function_sw_gain_get_port(stream_ptr->sink);
    }
    stream_function_sw_gain_init(*sw_gain_port, channel_number, &default_config);
    stream_function_sw_gain_initialize(para);
    DL_SW_update_gain();
    stream_function_sw_gain_configure_gain_target(*sw_gain_port, 1, (int32_t)*main_gain + (int32_t)g_LR_balance_gain_l);
    stream_function_sw_gain_configure_gain_target(*sw_gain_port, 2, (int32_t)*main_gain + (int32_t)g_LR_balance_gain_r);
    DSP_MW_LOG_I("[DL_SW_GAIN] g_port, 0x%x, 0x%x, 0x%x, ", 3, (uint32_t)g_DL_SW_gain_port,  (uint32_t)g_DL_SW_gain_port_2,  (uint32_t)g_DL_SW_gain_port_3);
    DSP_MW_LOG_I("[DL_SW_GAIN] Port 0x%x init done for type %d, gain_l = %d, gain_r = %d", 4, *sw_gain_port, stream_ptr->sink->type, (int32_t)*main_gain + (int32_t)g_LR_balance_gain_l, (int32_t)*main_gain + (int32_t)g_LR_balance_gain_r);

#endif

#else
    UNUSED(para);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
    return 0;
}


bool stream_function_DL_SW_process(void *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    if(((g_DL_SW_gain_default_para.enable_vp == false) && ((stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_VP) || (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_VP_PRE) || (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_VP_DUMMY_PRE) || (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_VP_DUMMY)))
        || ((g_DL_SW_gain_default_para.enable_a2dp == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_A2DP))
        || ((g_DL_SW_gain_default_para.enable_hfp == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL))
        || ((g_DL_SW_gain_default_para.enable_ble_music == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL))
        || ((g_DL_SW_gain_default_para.enable_ble_call == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL))
        || ((g_DL_SW_gain_default_para.enable_ull_music == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL))
        || ((g_DL_SW_gain_default_para.enable_ull_call == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL))
        || ((g_DL_SW_gain_default_para.enable_line_in == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN))
        || ((g_DL_SW_gain_default_para.enable_usb_in == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0))
        || ((g_DL_SW_gain_default_para.enable_usb_in == false) && (stream_ptr->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1))
    ){
        return 0;
    }
    DL_SW_update_gain();
    sw_gain_port_t *sw_gain_port = stream_function_sw_gain_get_port(stream_ptr->sink);
    if(sw_gain_port->config->resolution != stream_function_get_output_resolution(para)) {
        stream_function_sw_gain_configure_resolution(sw_gain_port, 1, stream_function_get_output_resolution(para));
        stream_function_sw_gain_configure_resolution(sw_gain_port, 2, stream_function_get_output_resolution(para));
    }

    return stream_function_sw_gain_process(para);
}

void DL_SW_gain_setting(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);

    if((int16_t)msg.ccni_message[0] == DL_SW_GAIN_SET_LR_BALANCE_OFFSET){
        g_DL_SW_gain_r_temp = (int16_t)msg.ccni_message[1];
        g_DL_SW_gain_l_temp = (int16_t)(msg.ccni_message[1] >> 16);
        DSP_MW_LOG_I("[DL_SW_GAIN] new setting L = %d R = %d", 2, g_DL_SW_gain_l_temp, g_DL_SW_gain_r_temp);
    } else if((int16_t)msg.ccni_message[0] == DL_SW_GAIN_SET_MAINGAIN_1){
        g_DL_SW_main_gain_temp = (int32_t)msg.ccni_message[1];
        DSP_MW_LOG_I("[DL_SW_GAIN] new setting main gain = %d", 1, g_DL_SW_main_gain_temp);
    } else if((int16_t)msg.ccni_message[0] == DL_SW_GAIN_SET_MAINGAIN_2){
        g_DL_SW_main_gain_temp_2 = (int32_t)msg.ccni_message[1];
        DSP_MW_LOG_I("[DL_SW_GAIN] new setting main 2 gain = %d", 1, g_DL_SW_main_gain_temp_2);
    } else if((int16_t)msg.ccni_message[0] == DL_SW_GAIN_SET_MAINGAIN_3){
        g_DL_SW_main_gain_temp_3 = (int32_t)msg.ccni_message[1];
        DSP_MW_LOG_I("[DL_SW_GAIN] new setting main 3 gain = %d", 1, g_DL_SW_main_gain_temp_3);
    } else if(((int16_t)msg.ccni_message[0] >= DL_SW_GAIN_SET_MAINGAIN_1_FADE_IN_PARAM) && ((int16_t)msg.ccni_message[0] <= DL_SW_GAIN_SET_MAINGAIN_3_FADE_OUT_PARAM)) {
        int32_t new_step, new_samples_per_step;
        new_step = (int16_t)(msg.ccni_message[1] >> 16);
        new_samples_per_step = (int16_t)msg.ccni_message[1];
        sw_gain_port_t *port;
        uint8_t in_out = 0;
        if((int16_t)msg.ccni_message[0] == DL_SW_GAIN_SET_MAINGAIN_1_FADE_IN_PARAM){
            in_out = 0;
            port = g_DL_SW_gain_port;
        } else if((int16_t)msg.ccni_message[0] == DL_SW_GAIN_SET_MAINGAIN_1_FADE_OUT_PARAM){
            in_out = 1;
            port = g_DL_SW_gain_port;
        } else if((int16_t)msg.ccni_message[0] == DL_SW_GAIN_SET_MAINGAIN_2_FADE_IN_PARAM){
            in_out = 0;
            port = g_DL_SW_gain_port_2;
        } else if((int16_t)msg.ccni_message[0] == DL_SW_GAIN_SET_MAINGAIN_2_FADE_OUT_PARAM){
            in_out = 1;
            port = g_DL_SW_gain_port_2;
        } else if((int16_t)msg.ccni_message[0] == DL_SW_GAIN_SET_MAINGAIN_3_FADE_IN_PARAM){
            in_out = 0;
            port = g_DL_SW_gain_port_3;
        } else if((int16_t)msg.ccni_message[0] == DL_SW_GAIN_SET_MAINGAIN_3_FADE_OUT_PARAM){
            in_out = 1;
            port = g_DL_SW_gain_port_3;
        } else {
            AUDIO_ASSERT(0);
        }
        if(in_out == 0){
            for(uint8_t i=0; i<port->total_channels; i++){
                stream_function_sw_gain_configure_gain_up(port, i, new_step, new_samples_per_step);
            }
        } else {
            new_step = 0 - new_step;
            for(uint8_t i=0; i<port->total_channels; i++){
                stream_function_sw_gain_configure_gain_down(port, i, new_step, new_samples_per_step);
            }
        }
    }
}

void DL_SW_get_default_para(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    if (msg.ccni_message[1] != 0 ){
        memcpy(&g_DL_SW_gain_default_para, (void *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]),sizeof(dl_sw_gain_default_para_t));
    }
}
#endif /*AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE*/