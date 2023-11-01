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
#include "msbc_enc_interface.h"
#include "dsp_feature_interface.h"
#include "dsp_audio_process.h"
#include "dsp_dump.h"

#ifdef MTK_BT_A2DP_MSBC_USE_PIC
#include "msbc_enc_portable.h"
#endif

#define MSBC_DEBUG_LOG_ENABLE       (0)

#define MSBC_ENCODE_FRAME_SIZE_7_5  (240) // 16K 7.5ms 16bit
/**
 *
 * Function Prototype
 *
 */
bool stream_codec_encoder_msbc_initialize(void *para);
bool stream_codec_encoder_msbc_process(void *para);


/**
 * stream_codec_encoder_msbc_initialize
 *
 * This function is used to init memory space for msbc encoder
 *
 *
 * @para : Default parameter of callback function
 * @codecMemPtr : Memory allocated by callback function
 *
 */
bool stream_codec_encoder_msbc_initialize(void *para)
{
    SBC_Encoder_Init(stream_codec_get_workingbuffer(para));
    mSBC_SW_SN_init((S16 *)(&(((SBC_ENC_PARAMS *)stream_codec_get_workingbuffer(para))->mSBC_frame_cnt)));
    DSP_MW_LOG_D("mSBC encode init\r\n", 0);
    return FALSE;
}


/**
 * stream_codec_encoder_msbc_process
 *
 * This function an input channel into a single mSBC frame
 *
 *
 * @para : Default parameter of callback function
 * @codecMemPtr : Memory allocated by callback function
 *
 */
bool stream_codec_encoder_msbc_process(void *para)
{
    S16 *InBuf = (S16 *)stream_codec_get_1st_input_buffer(para);
    S16 *OutBuf = (S16 *)stream_codec_get_1st_output_buffer(para);
    U8 i = 0;
    U8 encode_frames = 0;
    U32 in_size = stream_codec_get_input_size(para);

    if((in_size == 0) || ((in_size % 240) != 0)) {
        stream_codec_modify_output_size(para, 0);//word -> byte transfer
        DSP_MW_LOG_I("mSBC encode size mismatch %d % 240 != 0\r\n", 1, in_size);
        return FALSE;
    }
    encode_frames = in_size / MSBC_ENCODE_FRAME_SIZE_7_5;
    if (stream_codec_get_input_samplingrate(para) == FS_RATE_8K) {
        S16 *InCodecBuf = stream_codec_get_1st_input_buffer(para);
        U16 InSampleNumber = in_size / 2;
        for (i = InSampleNumber ; i > 0 ; i--) {
            InCodecBuf[2 * i - 1] = InCodecBuf[i - 1];
            InCodecBuf[2 * i - 2] = InCodecBuf[i - 1];
        }
        stream_codec_modify_input_size(para, in_size * 2);
        //Modify Codec in sampling rate
        //DSP_ModifyCodecInStreamSamplingRate(para, stream_codec_get_input_samplingrate(para)*2);
    }
#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)InBuf, encode_frames * MSBC_ENCODE_FRAME_SIZE_7_5, AUDIO_BT_SRC_DONGLE_DL_ENC_IN_ESCO);
#endif /* AIR_AUDIO_DUMP_ENABLE */
#if MSBC_DEBUG_LOG_ENABLE
    DSP_MW_LOG_I("mSBC encode data header : 0x%08x 0x%08x, encoder frames %d(7.5ms)", 3, *(InBuf ), *(InBuf +4), encode_frames);
#endif
    for (i = 0 ; i < encode_frames ; i++) {
        mSBC_Encoder_Process(InBuf + i * 120, OutBuf + 1 + i * 30, stream_codec_get_workingbuffer(para));
        //mSBC_SW_SN(OutBuf+i*30, &((SBC_ENC_PARAMS*)stream_codec_get_workingbuffer(para))->mSBC_frame_cnt);
        mSBC_SW_SN(OutBuf + i * 30, (S16 *)(&(((SBC_ENC_PARAMS *)stream_codec_get_workingbuffer(para))->mSBC_frame_cnt)));
#if MSBC_DEBUG_LOG_ENABLE
        DSP_MW_LOG_I("mSBC encode data : 0x%08x 0x%08x 0x%08x 0x%08x\r\n", 4,
            *(OutBuf +i*30),
            *(OutBuf +i*30+4),
            *(OutBuf +i*30+8),
            *(OutBuf +i*30+16)
            );
        DSP_MW_LOG_I("mSBC frame cnt: 0x%x, 0x%x", 2, (&(((SBC_ENC_PARAMS*)stream_codec_get_workingbuffer(para))->mSBC_frame_cnt)),
            (((SBC_ENC_PARAMS*)stream_codec_get_workingbuffer(para))->mSBC_frame_cnt));
#endif
    }
#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)OutBuf, 60 * encode_frames, VOICE_VC_IN2);
    LOG_AUDIO_DUMP((U8 *)OutBuf, 60 * encode_frames, AUDIO_BT_SRC_DONGLE_DL_ENC_OUT_ESCO);
#endif
    stream_codec_modify_output_size(para, 60 * encode_frames); //word -> byte transfer

    return FALSE;
}

#ifdef PRELOADER_ENABLE
BOOL mSBC_Encoder_Open(VOID *para)
{
    DSP_MW_LOG_I("[PIC] mSBC Encoder Open", 0);
    UNUSED(para);
    return TRUE;
}

BOOL mSBC_Encoder_Close(VOID *para)
{
    DSP_MW_LOG_I("[PIC] mSBC Encoder Close", 0);
    UNUSED(para);
    return TRUE;
}
#endif

