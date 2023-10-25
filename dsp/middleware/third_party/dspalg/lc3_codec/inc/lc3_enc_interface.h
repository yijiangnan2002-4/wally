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

#ifndef __LC3_ENC_INTERFACE_H__
#define __LC3_ENC_INTERFACE_H__

#ifdef AIR_BT_CODEC_BLE_ENABLED

#include "dsp_feature_interface.h"
#include "lc3_codec_api.h"

#define DSP_LC3_ENCODER_MAX_CH_NUM      (1)
#define DSP_LC3_ENCODER_MEM_EACH_CH     (15000+1000)        /* 15000: needed by encoder,   1000: needed by other processing*/
#define DSP_LC3_ENCODER_MEMSIZE         (DSP_LC3_ENCODER_MEM_EACH_CH * DSP_LC3_ENCODER_MAX_CH_NUM)
/* Size of stream output buffer for each channel, max 480 sample (48K) * 2 (16 bit resolution) */
#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
#define DSP_LC3_ENCODER_OUT_BUF_SIZE    (960*2*3)
#else
#define DSP_LC3_ENCODER_OUT_BUF_SIZE    (960*2)
#endif

bool stream_codec_encoder_lc3_initialize(void *para);
bool stream_codec_encoder_lc3_process(void *para);
bool stream_codec_encoder_lc3_process_branch(void *para);
void LC3_Enc_Deinit(void);

#define LC3_ENC_VALID_MEMORY_CHECK_VALUE   ((U32)0x99190349)
#define LC3_UL_CACHE_BUFFER_SIZE           2880 /* for the worst case: UL 48kHz, (48000/100*3)*2 */


typedef struct stru_lc3_enc_para_u {
    U32 MemoryCheck;
    U8  InitDone;
    DSP_ALIGN8 U8 g_lc3_cache_buffer[LC3_UL_CACHE_BUFFER_SIZE];
    //DSP_ALIGN8 U8 ScratchMemory[1];
} LC3_ENC_INSTANCE, *LC3_ENC_INSTANCE_PTR;

#endif

#endif /* __LC3_ENC_INTERFACE_H__ */

