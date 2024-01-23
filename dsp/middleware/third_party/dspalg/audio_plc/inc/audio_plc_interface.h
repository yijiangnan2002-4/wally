/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#ifndef AUDIO_PLC_INTERFACE_H
#define AUDIO_PLC_INTERFACE_H

#include "types.h"
//#include "DSP_Utilities.h"
#include "dsp_utilities.h"
#include "common.h"

//=====================================user parameters======================================
#define AIR_AUDIO_PLC_DEBUG_CONVERT_TO_16BIT_ENABLE (1)     //convert the resolution of dump stream from 32bits to 16bits
#define AIR_AUDIO_PLC_REMOVE_SHORT_SOUND_ENABLE     (1)     //if enable this option, short sound would be removed
#define PLC_COMPENSATION_SAMPLES                    (1024)  //compensation amount of audio plc
#define SHORT_SOUND_BOUNDARY_SAMPLES                (2048)  //if buffer level is lower than this value after audio drop, sound would be mute
#define PLC_OBSERVE_BURST_MODE_UPPER_TH_MS          (10000) //if the time of continuously audio drop is higher than this value and buffer level is lower than SHORT_SOUND_BOUNDARY_SAMPLES, sound would not be mute
//==========================================================================================

#define MAX_PLC_MEM_SIZE                (41740)//(29472)//(29464)//get from get_plc_memsize()
#define PLC_DECAY_LEVEL                 (32768 / PLC_COMPENSATION_SAMPLES)
#define AUDIO_PLC_MEMSIZE               (sizeof(AUDIO_PLC_SCRATCH_t))
#define SUPPOR_AMOUNT                   (4)
#define INVALID_INDEX                   0xFFFFFFFF


typedef int (*audio_plc_proc)(void *p_plc_mem_ext, int *Buf_L, int *Buf_R, int BFI);
typedef int (*audio_plc_get_bfi)(void* para);
typedef BOOL (*audio_plc_check_buffer_level)(U32 boundary_samples);

typedef enum  {
	audio_plc_good_frame = 0,
	audio_plc_bad_frame = 1,
	audio_plc_remove_frame = 2,
} audio_plc_bfi__stat_t;

typedef enum  {
	AUDIO_PLC_NORMAL_MODE = 0,
	AUDIO_PLC_OBSERVE_BURST_MODE = 1,
	AUDIO_PLC_REPEAT_PAST_STREAM_MODE = 2,
} audio_plc_mode_t;

typedef struct AUDIO_PLC_PARA_s
{
    S32                 (*audio_plc_proc)(void *p_plc_mem_ext, int *Buf_L, int *Buf_R, int BFI);
    U32                 (*audio_plc_get_bfi)(void* para);
    DSP_ALIGN8 S8       p_plc_mem_ext[MAX_PLC_MEM_SIZE];
    void                *temp_buffer_p;
    U32                 plc_data_in_queue;
    U32                 temp_bfi;
    U32                 plc_data_in_buffer;
    U16                 processing_samples;
    BOOL                init_done;
#if (AIR_AUDIO_PLC_REMOVE_SHORT_SOUND_ENABLE)
    BOOL                (*audio_plc_check_buffer_level)(U32 boundary_samples);
    audio_plc_mode_t    mode;
    U32                 continuous_bf;
    U32                 burst_mode_upper_th;
    U32                 burst_mode_lower_th;
    BOOL                good_frame_is_droped;
#endif
} AUDIO_PLC_SCRATCH_t, *AUDIO_PLC_SCRATCH_PTR_t;


typedef struct dsp_audio_plc_support_para_s
{
    S32                 (*audio_plc_proc)(void *p_plc_mem_ext, int *Buf_L, int *Buf_R, int BFI);
    U16                 frame_size;
}DSP_AUDIO_PLC_SUPPORT_PARA_t;

typedef struct dsp_audio_plc_ctrl_s
{
    U8              enable;
    U16             outputsize;
} DSP_AUDIO_PLC_CTRL_t;


EXTERN bool Audio_PLC_Init (VOID* para);
EXTERN bool Audio_PLC_Process (VOID* para);
EXTERN void Audio_PLC_ctrl (dsp_audio_plc_ctrl_t audio_plc_ctrl);
EXTERN U16 Audio_PLC_Get_OutputSize (void);
EXTERN void Audio_PLC_Modify_OutputSize (U16 outputsize);
EXTERN U8 Audio_PLC_get_ctrl_state(void);


#endif /* AUDIO_PLC_INTERFACE_H */
