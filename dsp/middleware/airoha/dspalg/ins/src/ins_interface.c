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

#include "dsp_audio_process.h"
#include "dsp_ins_nvkey_struct.h"
#include "ins_interface.h"
#include "dsp_callback.h"
#include "dsp_sdk.h"
#include "dsp_memory.h"
#include "audio_nvdm_common.h"
#include "dsp_dump.h"
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_mcu_dsp_common.h"
#endif
#ifdef MTK_BT_A2DP_CPD_USE_PIC
#include "cpd_portable.h"
#endif

/*
 * Buffer Control
*/
INS_INSTANCE_ptr InsMemoryPtr = NULL;
INS_INSTANCE_ptr UsbinsMemoryPtr = NULL;

#define INS_AU_FRAME_SIZE      (32)
#define INS_VALID_MEMORY_CHECK_VALUE              ((U32)0x6128800)
#define USB_INS_VALID_MEMORY_CHECK_VALUE              ((U32)0x6124900)
#define INS_RUNTIME_INVALID_MEMORY_CHECK_VALUE    ((U32)0x5A5A5A5A)
#define INS_MEMSIZE          (sizeof(INS_INSTANCE_t))

typedef enum {
    MODE_AU1_INS = 0,
    MODE_AU2_INS = 1,
} eINS_AU_MODE;

/**
 *
 * Function Prototype
 *
 */
bool INS_MemInit(void *para,eINS_AU_MODE au_path);
bool INS_MemCheck(eINS_AU_MODE au_path);



/**
 * INS_MemInit
 *
 * This function is used to init memory space for INS process
 *
 *
 * @para : Default parameter of callback function
 * @return : Initialize result
 */
bool INS_MemInit(void *para,eINS_AU_MODE au_path)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    U32 InstanceSize = 0;
    InstanceSize = get_INS_memsize();
    
    if (!INS_MemCheck(au_path)) {
        if(au_path == MODE_AU1_INS){
            if (!(InsMemoryPtr && (InsMemoryPtr->MemoryCheck == INS_RUNTIME_INVALID_MEMORY_CHECK_VALUE))) {
                if ((INS_MEMSIZE+InstanceSize) != stream_function_get_working_buffer_length(para)) {
                    DSP_MW_LOG_E("INS MEM SIZE is insufficient require:%d, allocated:%d", 2,INS_MEMSIZE+InstanceSize, stream_function_get_working_buffer_length(para));
                    assert(false);
                }
            InsMemoryPtr = (INS_INSTANCE_ptr)stream_function_get_working_buffer(para);
            }
            InsMemoryPtr->MemoryCheck = INS_VALID_MEMORY_CHECK_VALUE;
            return FALSE;
        }else if(au_path == MODE_AU2_INS){
            if (!(UsbinsMemoryPtr && (UsbinsMemoryPtr->MemoryCheck == INS_RUNTIME_INVALID_MEMORY_CHECK_VALUE))) {
                if ((INS_MEMSIZE+InstanceSize) != stream_function_get_working_buffer_length(para)) {
                    DSP_MW_LOG_E("INS MEM SIZE is insufficient require:%d, allocated:%d", 2,INS_MEMSIZE+InstanceSize, stream_function_get_working_buffer_length(para));
                    assert(false);
                }
            UsbinsMemoryPtr = (INS_INSTANCE_ptr)stream_function_get_working_buffer(para);
            }
            UsbinsMemoryPtr->MemoryCheck = USB_INS_VALID_MEMORY_CHECK_VALUE;
            return FALSE;
        }

    }
    return TRUE;
}

/**
 * INS_MemCheck
 *
 * This function is used to check init memory space for INS process
 *
 *
 * @para : Default parameter of callback function
 * @return : Check result
 */
bool INS_MemCheck(eINS_AU_MODE au_path)
{
    if(au_path == MODE_AU1_INS){
        if (NULL != InsMemoryPtr) {
            if (INS_VALID_MEMORY_CHECK_VALUE == InsMemoryPtr->MemoryCheck) {
                return TRUE;
            }
        }
    }else if(au_path == MODE_AU2_INS){
        if (NULL != UsbinsMemoryPtr) {
            if (USB_INS_VALID_MEMORY_CHECK_VALUE == UsbinsMemoryPtr->MemoryCheck) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

void stream_function_ins_deinitialize(void)
{
    if (InsMemoryPtr) {
        InsMemoryPtr->MemoryCheck = INS_RUNTIME_INVALID_MEMORY_CHECK_VALUE;
    }else if(UsbinsMemoryPtr){
        UsbinsMemoryPtr->MemoryCheck = INS_RUNTIME_INVALID_MEMORY_CHECK_VALUE;
    }
}

/**
 * Audio_INS_Init
 *
 * This function is used to init memory space for audio INS
 *
 * @para : Default parameter of callback function
 *
 */
bool ins_audio_initialize(VOID *para, INS_INSTANCE_ptr au_ins_instance)
{
    void *p_scratch = &au_ins_instance->ScratchMemory[0];
    void *p_nvkey = &au_ins_instance->NvKey;

    UNUSED(para);

    nvkey_read_full_key(NVKEY_DSP_PARA_INS, p_nvkey, sizeof(INS_NVKEY_STATE));

#if defined(AIR_WIRED_AUDIO_ENABLE) && !defined(AIR_DCHS_MODE_ENABLE)
    audio_scenario_type_t wired_audio_type;
    wired_audio_type = DSP_STREAMING_GET_FROM_PRAR(para)->source->scenario_type;
    if ((wired_audio_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0) ||
        (wired_audio_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1) ||
        (wired_audio_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN)) {
        INS_SetFrame8_Init(p_scratch);
    }
#endif

    INS_Init(p_scratch, p_nvkey);
    return FALSE;
}

bool stream_function_ins_audio_initialize(VOID *para)
{
    if (INS_MemInit(para,MODE_AU1_INS)) {
        return TRUE;
    }
    return ins_audio_initialize(para, InsMemoryPtr);
}

bool stream_function_usb_ins_audio_initialize(VOID *para)
{
    if (INS_MemInit(para,MODE_AU2_INS)) {
        return TRUE;
    }
    return ins_audio_initialize(para, UsbinsMemoryPtr);
}

#if defined(AIR_LINE_IN_LATENCY_LOW) || defined(AIR_LINE_IN_LATENCY_MEDIUM) || defined(AIR_USB_IN_LATENCY_LOW)
ATTR_TEXT_IN_IRAM bool ins_audio_process(VOID *para,INS_INSTANCE_ptr au_ins_instance)
#else
bool ins_audio_process(VOID *para,INS_INSTANCE_ptr au_ins_instance)
#endif
{
    S32 *BufL = (S32 *)stream_function_get_1st_inout_buffer(para);
    S32 *BufR = (S32 *)stream_function_get_2nd_inout_buffer(para);
    U16 FrameSize = stream_function_get_output_size(para);
    S16 dev_channels = (S16)stream_function_get_device_channel_number(para);
    U16 InsSampleSize;
    S16 channel_mode = 2; // 2:stereo 1:mono
    void *p_scratch = &au_ins_instance->ScratchMemory[0];
    void *p_nvkey = &au_ins_instance->NvKey;
    UNUSED(p_nvkey);
	
    if (au_ins_instance->NvKey.ENABLE == 0) {
        return FALSE;
    }

    if ((0 == FrameSize) || (FrameSize % INS_AU_FRAME_SIZE)) {
        return FALSE;
    }

    if (dev_channels == 1) {
        BufR = NULL;
        channel_mode = 1;
    }

    if (stream_function_get_output_resolution(para) == RESOLUTION_32BIT) {
        InsSampleSize = FrameSize >> 2;
    } else {
        InsSampleSize = FrameSize >> 1;
        dsp_converter_16bit_to_32bit(BufL, (S16 *)BufL, InsSampleSize);
        dsp_converter_16bit_to_32bit(BufR, (S16 *)BufR, InsSampleSize);
        stream_function_modify_output_size(para, InsSampleSize << 2);
        stream_function_modify_output_resolution(para, RESOLUTION_32BIT);
    }

#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)BufL, (U32)(InsSampleSize << 2), AUDIO_INS_IN_L);
    if (BufR) {
        LOG_AUDIO_DUMP((U8 *)BufR, (U32)(InsSampleSize << 2), AUDIO_INS_IN_R);
    }
#endif

    channel_mode = (BufR == NULL) ? 1 : 2;
    INS_Prcs(p_scratch, BufL, BufR, InsSampleSize, channel_mode);

#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)BufL, (U32)(InsSampleSize << 2), AUDIO_INS_OUT_L);
    if (BufR) {
        LOG_AUDIO_DUMP((U8 *)BufR, (U32)(InsSampleSize << 2), AUDIO_INS_OUT_R);
    }
#endif
    return FALSE;
}

/**
 * stream_function_ins_audio_process
 *
 * audio INS main process
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_ins_audio_process(void *para)
{
    return ins_audio_process(para,InsMemoryPtr);
}

bool stream_function_usb_ins_audio_process(void *para)
{
    return ins_audio_process(para,UsbinsMemoryPtr);
}
