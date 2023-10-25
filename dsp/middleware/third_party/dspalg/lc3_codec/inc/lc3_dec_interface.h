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

#ifndef __LC3_DEC_INTERFACE_H__
#define __LC3_DEC_INTERFACE_H__

#ifdef AIR_BT_CODEC_BLE_ENABLED

#include "dsp_feature_interface.h"
#include "lc3_codec_api.h"
#include "lc3_enc_interface.h"


/* Reserve temp buffer for L/R deinterleave, asseming 48KHz/32bit resolution */
#define DSP_LC3_DECODER_DEINTERLEAVE_BUF_SIZE 3840
//#define DSP_LC3_DECODER_MEMSIZE (16844 + DSP_LC3_DECODER_DEINTERLEAVE_BUF_SIZE)

/* Size of stream output buffer for each channel, max 480 sample (48K) * 4 (24 bit resolution), cache 2 frame max */
#define DSP_LC3_DECODER_OUT_BUF_SIZE 1920 * 2

bool stream_codec_decoder_lc3_initialize(void *para);
bool stream_codec_decoder_lc3_process(void *para);
VOID lc3_set_audio_plc_mode(LC3_PLC_MODE_T mode);
void lc3_dual_decode_mode_set(BOOL isDualMode);
void LC3_Dec_Deinit(void);

#endif

#define LC3_CACHE_BUFFER_SIZE           2560//640*2*2 (total_sample * channel_number * input_resolution)
#define LC3_DEC_VALID_MEMORY_CHECK_VALUE   ((U32)0x99190349)


typedef struct stru_lc3_dec_para_u {
    U32 MemoryCheck;
    U8  InitDone;
    DSP_ALIGN8 U8 g_lc3_cache_buffer[LC3_CACHE_BUFFER_SIZE];
    //DSP_ALIGN8 U8 ScratchMemory[1];
} LC3_DEC_INSTANCE, *LC3_DEC_INSTANCE_PTR;

typedef struct stru_lc3_para_u {
    LC3_DEC_INSTANCE lc3_dec_memory;
    LC3_ENC_INSTANCE lc3_enc_memory;
} LC3_INSTANCE, *LC3_INSTANCE_PTR;

#define DSP_LC3_DECODER_MEMSIZE  (sizeof(LC3_DEC_INSTANCE))

#endif /* __LC3_DEC_INTERFACE_H__ */

