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

/******************************************************************************
 * Constant define
 ******************************************************************************/
#define DUMPDATA_MASK_WORD_NO       (2)
#define NO_OF_BITS_IN_A_WORD        (32)
#define AUDIO_DUMP_CONFIG_MAX_NUM   (20)

/******************************************************************************
 * NVKey Structure
 ******************************************************************************/
//typedef struct stru_dsp_datadump_para_s
//{
//    uint32_t Dump_Enable;                /**< @Value 0 @Desc 0:Diaable 1:Enable */
//    uint32_t Dump_Scenario_Sel;          /**< @Value 0x00000002 @Desc 0:Diaable, 1:I2S_MASTER, 2:I2S_SLAVE, 3:SPDIF, 4:USB, 5:AIR, */
//    uint32_t Dump_Mask_0;                /**< @Value 0x00000000 @Desc Mask ID  0-31 */
//    uint32_t Dump_Mask_1;                /**< @Value 0x00000000 @Desc Mask ID 32-63*/
//    uint32_t Buf_size;                   /**< @Value 0x00001388 @Desc internal buffer size for dump data in */
//} PACKED, DSP_PARA_DATADUMP_STRU;

typedef enum {
    DATADUMP_DISABLE = 0,
    DATADUMP_ENABLE,
} DSP_DATADUMP_STATUS, *DSP_DATADUMP_STATUS_PTR;

typedef enum {
    DUMP_DISABLE = 0,
    DUMP_BY_USB,
} DSP_DATADUMP_SEL, *DSP_DATADUMP_SEL_PTR;

typedef struct {
    DSP_DATADUMP_STATUS DumpStatus;
    DSP_DATADUMP_SEL    DumpScenario;
    uint32_t Mask[DUMPDATA_MASK_WORD_NO];
    uint32_t BufSize;
} DSP_DATADUMP_CTRL_STRU, *DSP_DATADUMP_CTRL_STRU_ptr;



/******************************************************************************
 * Enumerations
 ******************************************************************************/
typedef enum {
    //_RESERVED                   = 0,
    SOURCE_IN1                  = 1,
    SOURCE_IN2                  = 2,
    SOURCE_IN3                  = 3,
    SOURCE_IN4                  = 4,
    SOURCE_IN5                  = 5,
    SINK_OUT1                   = 6,
    SINK_OUT2                   = 7,
    SINK_SUBPATH_OUT            = 8,
    AUDIO_CODEC_IN_LENGTH       = 9,

    VOICE_TX_MIC_0              = 10,
    VOICE_TX_MIC_1              = 11,
    VOICE_TX_MIC_2              = 12,
    VOICE_TX_MIC_3              = 13,
    VOICE_TX_REF                = 14,
    VOICE_TX_NR_OUT             = 15,
    VOICE_TX_CPD_IN             = 16,
    VOICE_TX_CPD_OUT            = 17,
    VOICE_TX_OUT                = 18,

    VOICE_RX_PLC_IN             = 19,
    VOICE_RX_PLC_INFO           = 20,
    VOICE_RX_PLC_OUT            = 21,
    VOICE_RX_NR_IN              = 22,
    VOICE_RX_NR_OUT             = 23,
    VOICE_RX_CPD_IN             = 24,
    VOICE_RX_CPD_OUT            = 25,
    VOICE_RX_OUT                = 26,

    VOICE_VC_IN1                = 27,
    VOICE_VC_IN2                = 28,
    VOICE_VC_RESULT             = 29,

    PROMPT_VP_PATTERN           = 30,
    PROMPT_VP_OUT               = 31,
    PROMPT_RT_PATTERN           = 32,
    PROMPT_RT_OUT               = 33,

    AUDIO_CODEC_IN              = 34,
    AUDIO_SOURCE_IN_L           = 35,
    AUDIO_SOURCE_IN_R           = 36,
    AUDIO_INS_IN_L              = 37,
    AUDIO_INS_IN_R              = 38,
    AUDIO_INS_OUT_L             = 39,
    AUDIO_INS_OUT_R             = 40,
    AUDIO_ENHANCEMENT_IN_L      = 41,
    AUDIO_ENHANCEMENT_IN_R      = 42,
    AUDIO_ENHANCEMENT_OUT_L     = 43,
    AUDIO_ENHANCEMENT_OUT_R     = 44,
    AUDIO_PEQ_IN_L              = 45,
    AUDIO_PEQ_IN_R              = 46,
    AUDIO_PEQ_OUT_L             = 47,
    AUDIO_PEQ_OUT_R             = 48,
    AUDIO_PEQ2_IN_L             = 49,
    AUDIO_PEQ2_IN_R             = 50,
    AUDIO_PEQ2_OUT_L            = 51,
    AUDIO_PEQ2_OUT_R            = 52,
    AUDIO_CPD_IN_L              = 53,
    AUDIO_CPD_IN_R              = 54,
    AUDIO_CPD_OUT_L             = 55,
    AUDIO_CPD_OUT_R             = 56,

    AUDIO_SOUNDBAR_INPUT        = 57,
    AUDIO_SOUNDBAR_TX           = 58,

    AUDIO_WOOFER_RX             = 59,
    AUDIO_WOOFER_UPSAMPLE_8K    = 60,
    AUDIO_WOOFER_PLC_OUT        = 61,
    AUDIO_WOOFER_UPSAMPLE_16K   = 62,
    AUDIO_WOOFER_CPD_OUT        = 63,

    /* USB Audio */
    USB_AUDIO_RX1                = 400,
    USB_AUDIO_RX2                = 401,
    USB_AUDIO_TX1                = 402,
    USB_AUDIO_TX2                = 403,
    /* USB Audio - End */
    AUDIO_CODEC_TEST_DATA       = 720,

    DSP_DATADUMP_MAX_BIT        = 65535,
} DSP_DATADUMP_MASK_BIT;
/*
    Add new dump id here, **Value must be unique, must align with DSP dsp_dump.h enum**
    Name Rule : AUDIO + Scenoria + Feature + Other,  like: AUDIO_A2DP_PEQ_IN/OUT
*/

typedef struct{
    uint16_t dump_id;
    uint32_t dump_accum_bytes;
}DumpIDs_AccumBytes;

/******************************************************************************
 * External Function Prototypes
 ******************************************************************************/
void LOG_AUDIO_DUMP(uint8_t *audio, uint32_t audio_size, DSP_DATADUMP_MASK_BIT dumpID);
