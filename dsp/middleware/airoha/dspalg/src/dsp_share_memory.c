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

//#include "os.h"
#include "dsp_utilities.h"
#include "dsp_share_memory.h"
#include "audio_config.h"

DSP_ALIGN8 DSP_SHARE_MEM_t gDspAlgParameter;
#ifdef MTK_BT_A2DP_ENABLE
DSP_ALIGN8 U8 gDspA2DPCodec;
#endif

/**
 * DSP_ALG_UpdateEscoTxMode
 *
 * This function is used to inform DSP eSCO TX algorithms the application mode set from Air
 *
 *
 * @mode : the mode to be set: 0 - VOICE_NB, 1 - VOICE_WB, 2 - VOICE_SWB
 *
 */
VOID DSP_ALG_UpdateEscoTxMode(SCO_CODEC mode)
{
    gDspAlgParameter.EscoMode.Tx = mode;
}


/**
 * DSP_ALG_UpdateEscoRxMode
 *
 * This function is used to inform DSP eSCO RX algorithms the application mode set from Air
 *
 *
 * @mode : the mode to be set: 0 - VOICE_NB, 1 - VOICE_WB, 2 - VOICE_SWB
 *
 */
VOID DSP_ALG_UpdateEscoRxMode(SCO_CODEC mode)
{
    gDspAlgParameter.EscoMode.Rx = mode;
}


/**
 * DSP_UpdateA2DPCodec
 *
 * This function is used to inform DSP A2DP codec type
 *
 *
 * @codec_type:
 *    (0)           < SBC codec. >
 *    (2)           < AAC codec. >
 *    (0xFF)        < VENDOR codec. >
 *
 */
#ifdef MTK_BT_A2DP_ENABLE
VOID DSP_UpdateA2DPCodec(U8 codec_type)
{
    gDspA2DPCodec = codec_type;
}
#endif

