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

#ifndef _CPD_PARA_H_
#define _CPD_PARA_H_

#if 0
#ifdef WIN32
#define ALIGN(x) __declspec(align(x))
#else
#define ALIGN(x) __attribute__((aligned (x)))
#endif
#else
#include "dsp_utilities.h"
#endif
#include "dsp_gain_control.h"
#include "dsp_para_cpd_nvkey_struct.h"

#define MAX_AU_CPD_DELAY_SIZE 96*2*2 // *2(stereo)*2(96K sample rate)
#define MAX_BUF_SIZE    8
#define PACKET_BUF_SIZE 8
#define DSP_CPD_SCRATCH_MEMSIZE 1

//AU:
typedef struct AU_CPD_INSTANCE_s {
    U32 MemoryCheck;
    S32 positive_gain;
    DSP_ALIGN8 CPD_AU_NVKEY_STATE NvKey;
    DSP_ALIGN8 U8 ScratchMemory[DSP_CPD_SCRATCH_MEMSIZE]; //Set at the end of structure for dynamic size
} AU_CPD_INSTANCE_t, *AU_CPD_INSTANCE_ptr;


//VO:
typedef struct VO_CPD_INSTANCE_s {
    U32 MemoryCheck;
    U32 positive_gain; //only used for RX CPD
#ifdef AIR_HEARING_PROTECTION_ENABLE
    DSP_ALIGN8 S16 voice_peq[212];
#endif
    DSP_ALIGN8 CPD_VO_NVKEY_STATE NvKey;
    DSP_ALIGN8 U8 ScratchMemory[DSP_CPD_SCRATCH_MEMSIZE]; //Set at the end of structure for dynamic size
} VO_CPD_INSTANCE_t, *VO_CPD_INSTANCE_ptr;

typedef struct {
    S16 TX_GAIN;
    S16 CPD_VAD;
    S16 avc_vol;
} ECNR_OUT;

#endif /* _DSP_PARA_H_ */

