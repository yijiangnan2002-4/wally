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
#ifndef __DSP_DUMP_H__
#define __DSP_DUMP_H__

/******************************************************************************
 * Private Constant define
 ******************************************************************************/
#define DUMP_ID_VERSION             (1)
#define AUDIO_DUMP_CONFIG_MAX_NUM   (20)
#define AUDIO_DUMP_DEVICE_LOGGING   (1)

typedef enum {
    DSP_DATADUMP_TEST_ID_MIN = 60001,
    AUDIO_DUMP_TEST_ID_1,
    AUDIO_DUMP_TEST_ID_2,
    AUDIO_DUMP_TEST_ID_3,
    AUDIO_DUMP_TEST_ID_4,
    AUDIO_DUMP_TEST_ID_5,
    DSP_DATADUMP_TEST_ID_MAX,
} DSP_DATADUMP_ID_TEST; //For Local Test Bypass Config Tool


/*********************************************************************************
    Add new dump id here, Value must be unique
    Name Rule : AUDIO + Scenoria + Feature + Other,  like: AUDIO_A2DP_PEQ_IN/OUT
**********************************************************************************/
typedef enum {
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

    AUDIO_AEQ_FF_MIC_IN         = 64,
    AUDIO_AEQ_FB_MIC_IN         = 65,
    AUDIO_AEQ_ECHO_IN           = 66,
    AUDIO_AEQ_PRO_OUT           = 67,

    VOICE_FB_ECOUT_INFO         = 72,

    SOURCE_COMMON_IN_L          = 73,
    SOURCE_COMMON_IN_R          = 74,
    SINK_COMMON_OUT_L           = 75,
    SINK_COMMON_OUT_R           = 76,

    /* Wired aduio dump begin */
    WIRED_AUDIO_AUDIO_DUMP_MIN  = 80,
    WIRED_AUDIO_USB_IN_I_1      = 81,
    WIRED_AUDIO_USB_IN_I_2      = 82,
    WIRED_AUDIO_USB_IN_I_3      = 83,
    WIRED_AUDIO_USB_IN_I_4      = 84,
    WIRED_AUDIO_USB_IN_I_5      = 85,
    WIRED_AUDIO_USB_IN_I_6      = 86,
    WIRED_AUDIO_USB_IN_I_7      = 87,
    WIRED_AUDIO_USB_IN_I_8      = 88,
    WIRED_AUDIO_USB_IN_2_I_1    = 89,
    WIRED_AUDIO_USB_IN_2_I_2    = 90,
    WIRED_AUDIO_USB_IN_O_1      = 91,
    WIRED_AUDIO_USB_IN_O_2      = 92,
    WIRED_AUDIO_USB_IN_O_3      = 93,
    WIRED_AUDIO_USB_IN_O_4      = 94,
    WIRED_AUDIO_USB_IN_O_5      = 95,
    WIRED_AUDIO_USB_IN_O_6      = 96,
    WIRED_AUDIO_USB_IN_O_7      = 97,
    WIRED_AUDIO_USB_IN_O_8      = 98,
    WIRED_AUDIO_USB_OUT_I_1     = 99,
    WIRED_AUDIO_USB_OUT_I_2     = 100,
    WIRED_AUDIO_USB_OUT_O_1     = 101,
    WIRED_AUDIO_USB_OUT_O_2     = 102,
    WIRED_AUDIO_LINE_IN_I_1     = 103,
    WIRED_AUDIO_LINE_IN_I_2     = 104,
    WIRED_AUDIO_LINE_IN_O_1     = 105,
    WIRED_AUDIO_LINE_IN_O_2     = 106,
    WIRED_AUDIO_LINE_OUT_I_1    = 107,
    WIRED_AUDIO_LINE_OUT_I_2    = 108,
    WIRED_AUDIO_LINE_OUT_O_1    = 109,
    WIRED_AUDIO_LINE_OUT_O_2    = 110,
    WIRED_AUDIO_AUDIO_DUMP_MAX  = 150,
    /* Wired aduio dump end */

    /* Full adaptive ANC audio dump begin */
    AUDIO_FADP_ANC_IN_1                   = 151,
    AUDIO_FADP_ANC_IN_2                   = 152,
    AUDIO_FADP_ANC_IN_3                   = 153,
    AUDIO_FADP_ANC_IN_4                   = 154,
    AUDIO_FADP_ANC_OUT_1                  = 155,
    AUDIO_FADP_ANC_OUT_2                  = 156,
    AUDIO_FADP_ANC_OUT_3                  = 157,
    AUDIO_FADP_ANC_OUT_4                  = 158,
    AUDIO_FADP_ANC_OUT_2CH_16BIT_IN_1     = 159,
    AUDIO_FADP_ANC_OUT_2CH_16BIT_IN_2     = 160,
    AUDIO_FADP_ANC_OUT_2CH_16BIT_OUT_1    = 161,
    AUDIO_FADP_ANC_OUT_2CH_16BIT_OUT_2    = 162,
    AUDIO_FADP_ANC_DEQ_IN_1               = 163,
    AUDIO_FADP_ANC_DEQ_IN_2               = 164,
    AUDIO_FADP_ANC_AUDIO_DUMP_MAX         = 170,
    /* Full adaptive ANC aduio dump end */

    /* Full adaptive ANC audio dump begin */
    AUDIO_HW_VIVID_PT_IN_1                = 171,
    AUDIO_HW_VIVID_PT_IN_2                = 172,
    AUDIO_HW_VIVID_PT_OUT                 = 173,
    AUDIO_HW_VIVID_PT_IN_3                = 174,
    AUDIO_HW_VIVID_PT_IN_4                = 175,
    AUDIO_HW_VIVID_PT_OUT_2               = 176,
    AUDIO_HW_VIVID_PT_AUDIO_DUMP_MAX      = 180,
    /* Full adaptive ANC aduio dump end */

    /*++++++++++++ DCHS DL ++++++++++++++++*/
    AUDIO_DCHS_DL_DATA_SOURCE             = 201,
    AUDIO_DCHS_DL_UART_SCENARIO_HWSRC_IN  = 202,
    AUDIO_DCHS_DL_OTHER_SCENARIO_DATA     = 203,
    AUDIO_DCHS_DL_LOCAL_CH_HWSRC_IN       = 204,
    AUDIO_DCHS_DL_UART_SCENARIO_L         = 205,
    AUDIO_DCHS_DL_UART_SCENARIO_R         = 206,
    AUDIO_DCHS_DL_LOCAL_SCENARIO_1_L      = 207,
    AUDIO_DCHS_DL_LOCAL_SCENARIO_1_R      = 208,
    AUDIO_DCHS_DL_LOCAL_SCENARIO_2_L      = 209,
    AUDIO_DCHS_DL_LOCAL_SCENARIO_2_R      = 210,
    AUDIO_DCHS_DL_RIGHT_SINK_L            = 211,
    AUDIO_DCHS_DL_RIGHT_SINK_R            = 212,
    AUDIO_DCHS_DL_LEFT_SINK_L             = 213,
    AUDIO_DCHS_DL_UART_RX                 = 214,
    AUDIO_DCHS_DL_UART_SCENARIO_SOURCE    = 215,
    AUDIO_DCHS_DL_LOCAL_SCENARIO_1_SOURCE = 216,
    AUDIO_DCHS_DL_LOCAL_SCENARIO_2_SOURCE = 217,
    AUDIO_DCHS_DL_SOURCE_IN_CH_1          = 218,
    AUDIO_DCHS_DL_SOURCE_IN_CH_2          = 219,
    AUDIO_DCHS_DL_SOURCE_IN_CH_3          = 220,
    AUDIO_DCHS_DL_SOURCE_IN_CH_4          = 221,
    AUDIO_DCHS_DL_SOURCE_IN_CH_5          = 222,
    AUDIO_DCHS_DL_SOURCE_IN_CH_6          = 223,
    AUDIO_DCHS_DL_SOURCE_IN_CH_7          = 224,
    AUDIO_DCHS_DL_SOURCE_IN_CH_8          = 225,
    AUDIO_DCHS_DL_MAX_RESERVE             = 250,
    /*------------ DCHS DL ---------------*/
    AUDIO_DCHS_UL_UART_RX_L       = 300,
    AUDIO_DCHS_UL_UART_RX_R       = 301,
    AUDIO_DCHS_UL_LOCAL_TX        = 302,
    AUDIO_DCHS_UL_UART_RX_R_TX    = 303,
    AUDIO_DCHS_UL_POST_EC_MIC1        = 304,
    AUDIO_DCHS_UL_POST_EC_MIC2        = 305,
    AUDIO_DCHS_UL_POST_EC_MIC3        = 306,
    AUDIO_DCHS_UL_MAX_RESERVE             = 320,
    /*------------ DCHS UL ---------------*/

    /* USB Audio, Unused */
    USB_AUDIO_RX1                = 400,
    USB_AUDIO_RX2                = 401,
    USB_AUDIO_TX1                = 402,
    USB_AUDIO_TX2                = 403,
    /* USB Audio - End */

    /* LEA Dongle DUMP ID BEGIN */
    AUDIO_LEA_DONGLE_DL_BEG         = 420,
    AUDIO_LEA_DONGLE_UL_DEC_OUT_0   = 440,
    AUDIO_LEA_DONGLE_UL_DEC_OUT_1   = 441,
    AUDIO_LEA_DONGLE_UL_END         = 499,
    /* LEA Dongle DUMP ID END */

    /* ULD dump */
    ULD_ENCODE_IN_1              = 500,
    ULD_ENCODE_IN_2              = 501,
    ULD_ENCODE_IN_3              = 502,
    ULD_ENCODE_IN_4              = 503,
    ULD_ENCODE_OUT_1             = 504,
    ULD_ENCODE_OUT_2             = 505,
    ULD_ENCODE_OUT_3             = 506,
    ULD_ENCODE_OUT_4             = 507,
    ULD_DECODE_IN                = 508,
    ULD_DECODE_OUT               = 509,

    /*------------ FrameSzie Adjustor ---------------*/
    AUDIO_FRAMESIZE_ADJUSTOR_IN_CH1 = 550,
    AUDIO_FRAMESIZE_ADJUSTOR_IN_CH2 = 551,
    AUDIO_FRAMESIZE_ADJUSTOR_IN_CH3 = 552,
    AUDIO_FRAMESIZE_ADJUSTOR_IN_CH4 = 553,
    AUDIO_FRAMESIZE_ADJUSTOR_OUT_CH1 = 554,
    AUDIO_FRAMESIZE_ADJUSTOR_OUT_CH2 = 555,
    AUDIO_FRAMESIZE_ADJUSTOR_OUT_CH3 = 556,
    AUDIO_FRAMESIZE_ADJUSTOR_OUT_CH4 = 557,

    AUDIO_FRAMESIZE_ADJUSTOR_CUS_IN_CH1 = 558,
    AUDIO_FRAMESIZE_ADJUSTOR_CUS_IN_CH2 = 559,
    AUDIO_FRAMESIZE_ADJUSTOR_CUS_IN_CH3 = 550,
    AUDIO_FRAMESIZE_ADJUSTOR_CUS_IN_CH4 = 551,
    AUDIO_FRAMESIZE_ADJUSTOR_CUS_OUT_CH1 = 552,
    AUDIO_FRAMESIZE_ADJUSTOR_CUS_OUT_CH2 = 553,
    AUDIO_FRAMESIZE_ADJUSTOR_CUS_OUT_CH3 = 554,
    AUDIO_FRAMESIZE_ADJUSTOR_CUS_OUT_CH4 = 555,


    /* Dongle dump ID */
    DONGLE_AUDIO_USB_IN_I_1      = 600,
    DONGLE_AUDIO_USB_IN_I_2      = 601,
    DONGLE_AUDIO_USB_IN_I_3      = 602,
    DONGLE_AUDIO_USB_IN_I_4      = 603,
    DONGLE_AUDIO_USB_IN_I_5      = 604,
    DONGLE_AUDIO_USB_IN_I_6      = 605,
    DONGLE_AUDIO_USB_IN_I_7      = 606,
    DONGLE_AUDIO_USB_IN_I_8      = 607,
    DONGLE_AUDIO_USB_IN_2_I_1    = 608,
    DONGLE_AUDIO_USB_IN_2_I_2    = 609,
    DONGLE_AUDIO_USB_IN_2_I_3    = 610,
    DONGLE_AUDIO_USB_IN_2_I_4    = 611,
    DONGLE_AUDIO_USB_IN_2_I_5    = 612,
    DONGLE_AUDIO_USB_IN_2_I_6    = 613,
    DONGLE_AUDIO_USB_IN_2_I_7    = 614,
    DONGLE_AUDIO_USB_IN_2_I_8    = 615,
    DONGLE_AUDIO_MAX_ID          = 699,

    /* BT Source Dongle DUMP ID BEGIN */
    AUDIO_BT_SRC_DONGLE_DL_ENC_IN                = 700,
    AUDIO_BT_SRC_DONGLE_DL_ENC_OUT               = 701,
    AUDIO_BT_SRC_DONGLE_DL_MCU_USB_IN_DUAL       = 702,
    AUDIO_BT_SRC_DONGLE_DL_USB_INL               = 703,
    AUDIO_BT_SRC_DONGLE_DL_USB_INR               = 704,
    AUDIO_BT_SRC_DONGLE_DL_SINK_OUT_HEADER       = 705,
    AUDIO_BT_SRC_DONGLE_DL_SINK_OUT_NO_HEADER    = 706,

    AUDIO_BT_SRC_DONGLE_DL_ENC_IN_ESCO           = 707,
    AUDIO_BT_SRC_DONGLE_DL_ENC_OUT_ESCO          = 708,
    AUDIO_BT_SRC_DONGLE_DL_SINK_OUT_ESCO         = 709,

    AUDIO_BT_SRC_DONGLE_UL_BT_IN                 = 710,
    AUDIO_BT_SRC_DONGLE_UL_BT_DEC_IN             = 711,
    AUDIO_BT_SRC_DONGLE_UL_BT_DEC_OUT            = 712,
    AUDIO_BT_SRC_DONGLE_UL_USB_OUT               = 714,
    AUDIO_BT_SRC_DONGLE_DL_CLK_SKEW_DEBUG        = 719,
    /* BT Source Dongle DUMP ID END */
    AUDIO_CODEC_TEST_DATA       = 720,

    /* volume monitor */
    AUDIO_VOLUME_MONITOR_INPUT_CH_0     = 730,
    AUDIO_VOLUME_MONITOR_INPUT_CH_1     = 731,

    /* advanced record*/
    AUDIO_ADVANCED_RECORD_OUTPUT_CH_0   = 735,

    /* Low Latency Framework DUMP ID BEGIN */
    AUDIO_LLF_MIC_IN_0          = 800,
    AUDIO_LLF_MIC_IN_1          = 801,
    AUDIO_LLF_MIC_IN_2          = 802,
    AUDIO_LLF_MIC_IN_3          = 803,
    AUDIO_LLF_MIC_IN_4          = 804,
    AUDIO_LLF_MIC_IN_5          = 805,
    AUDIO_LLF_OUT_L             = 806,
    AUDIO_LLF_OUT_R             = 807,
    AUDIO_PSAP_IN_0             = 808,
    AUDIO_PSAP_IN_1             = 809,
    AUDIO_PSAP_IN_2             = 810,
    AUDIO_PSAP_MUSIC_VOICE      = 811,
    AUDIO_PSAP_ECHO_REF         = 812,
    AUDIO_PSAP_AFC_OUT_0        = 813,
    AUDIO_PSAP_AFC_OUT_1        = 814,
    AUDIO_PSAP_PLC_OUT          = 815,
    AUDIO_PSAP_OUT_L            = 816,
    AUDIO_VIVID_IN_0            = 817,
    AUDIO_VIVID_LDNR_IN         = 818,
    AUDIO_VIVID_LDNR_OUT        = 819,
    AUDIO_VIVID_OUT             = 820,
    AUDIO_PSAP_MUSIC_VOICE_PLC_OUT  = 821,
    AUDIO_LLF_MAX_ID            = 850,

    /* SRC fixed ratio */
    SRC_FIXED_RATIO_P1_IN1       = 851,
    SRC_FIXED_RATIO_P1_IN2       = 852,
    SRC_FIXED_RATIO_P1_IN3       = 853,
    SRC_FIXED_RATIO_P1_IN4       = 854,
    SRC_FIXED_RATIO_P1_IN5       = 855,
    SRC_FIXED_RATIO_P1_IN6       = 856,
    SRC_FIXED_RATIO_P1_IN7       = 857,
    SRC_FIXED_RATIO_P1_IN8       = 858,
    SRC_FIXED_RATIO_P1_OUT1      = 859,
    SRC_FIXED_RATIO_P1_OUT2      = 860,
    SRC_FIXED_RATIO_P1_OUT3      = 861,
    SRC_FIXED_RATIO_P1_OUT4      = 862,
    SRC_FIXED_RATIO_P1_OUT5      = 863,
    SRC_FIXED_RATIO_P1_OUT6      = 864,
    SRC_FIXED_RATIO_P1_OUT7      = 865,
    SRC_FIXED_RATIO_P1_OUT8      = 866,
    SRC_FIXED_RATIO_P2_IN1       = 867,
    SRC_FIXED_RATIO_P2_IN2       = 868,
    SRC_FIXED_RATIO_P2_IN3       = 869,
    SRC_FIXED_RATIO_P2_IN4       = 870,
    SRC_FIXED_RATIO_P2_IN5       = 871,
    SRC_FIXED_RATIO_P2_IN6       = 872,
    SRC_FIXED_RATIO_P2_IN7       = 873,
    SRC_FIXED_RATIO_P2_IN8       = 874,
    SRC_FIXED_RATIO_P2_OUT1      = 875,
    SRC_FIXED_RATIO_P2_OUT2      = 876,
    SRC_FIXED_RATIO_P2_OUT3      = 877,
    SRC_FIXED_RATIO_P2_OUT4      = 878,
    SRC_FIXED_RATIO_P2_OUT5      = 879,
    SRC_FIXED_RATIO_P2_OUT6      = 880,
    SRC_FIXED_RATIO_P2_OUT7      = 881,
    SRC_FIXED_RATIO_P2_OUT8      = 882,
    SRC_FIXED_RATIO_P3_IN1       = 883,
    SRC_FIXED_RATIO_P3_IN2       = 884,
    SRC_FIXED_RATIO_P3_IN3       = 885,
    SRC_FIXED_RATIO_P3_IN4       = 886,
    SRC_FIXED_RATIO_P3_IN5       = 887,
    SRC_FIXED_RATIO_P3_IN6       = 888,
    SRC_FIXED_RATIO_P3_IN7       = 889,
    SRC_FIXED_RATIO_P3_IN8       = 890,
    SRC_FIXED_RATIO_P3_OUT1      = 891,
    SRC_FIXED_RATIO_P3_OUT2      = 892,
    SRC_FIXED_RATIO_P3_OUT3      = 893,
    SRC_FIXED_RATIO_P3_OUT4      = 894,
    SRC_FIXED_RATIO_P3_OUT5      = 895,
    SRC_FIXED_RATIO_P3_OUT6      = 896,
    SRC_FIXED_RATIO_P3_OUT7      = 897,
    SRC_FIXED_RATIO_P3_OUT8      = 898,

    AUDIO_HFP_AFE_IN1            = 900,
    AUDIO_HFP_AFE_IN2            = 901,
    AUDIO_HFP_AFE_IN3            = 902,
    AUDIO_HFP_AFE_IN4            = 903,
    AUDIO_HFP_AFE_IN5            = 904,
    /*++++++++++++ Mixer Stream +++++++++++++*/
    AUDIO_MIXER_STREAM_SINK_OUT         = 905,
    AUDIO_MIXER_STREAM_SOURCE_IN_CH1    = 906,
    AUDIO_MIXER_STREAM_SOURCE_IN_CH2    = 907,
    AUDIO_MIXER_STREAM_SOURCE_IN_CH3    = 908,
    AUDIO_MIXER_STREAM_SOURCE_IN_CH4    = 909,
    AUDIO_MIXER_STREAM_SOURCE_IN_CH5    = 910,
    AUDIO_MIXER_STREAM_SOURCE_IN_CH6    = 911,
    /*-------------Mixer Stream -------------*/

    /* Multiple stream out DUMP ID BEGIN */
    AUDIO_MULTIPLE_STREAM_OUT_0  = 910,
    AUDIO_MULTIPLE_STREAM_OUT_1  = 911,
    AUDIO_MULTIPLE_STREAM_OUT_2  = 912,
    AUDIO_MULTIPLE_STREAM_OUT_3  = 913,
    AUDIO_MULTIPLE_STREAM_OUT_4  = 914,
    AUDIO_MULTIPLE_STREAM_OUT_5  = 915,

    /*aidop plc*/
    AUDIO_PLC_IN_L               = 916,
    AUDIO_PLC_IN_R               = 917,
    AUDIO_PLC_OUT_L              = 918,
    AUDIO_PLC_OUT_R              = 919,
    AUDIO_PLC_BFI                = 920,

    DSP_DATADUMP_MAX_BIT = 60000,
} DSP_DATADUMP_MASK_BIT;
/*********************************************************************************
    Add new dump id here, Value must be unique
    Name Rule : AUDIO + Scenoria + Feature + Other,  like: AUDIO_A2DP_PEQ_IN/OUT
**********************************************************************************/

typedef struct{
    U16 dump_id;
    U32 dump_accum_bytes;
}DumpIDs_AccumBytes;

/******************************************************************************
 * External Function Prototypes
 ******************************************************************************/
EXTERN void LOG_AUDIO_DUMP(U8 *audio, U32 audio_size, U32 dump_id);
EXTERN U16 AudioDumpCfgIDs[AUDIO_DUMP_CONFIG_MAX_NUM];
EXTERN U32 AudioDumpDevice;
EXTERN U32 DumpIdCfgVersion;
EXTERN DumpIDs_AccumBytes DumpIDsAccumBytes[AUDIO_DUMP_CONFIG_MAX_NUM];

EXTERN void audio_dump_set_block(bool is_block_dump);
EXTERN bool check_cfg_dump_id(DSP_DATADUMP_MASK_BIT dumpID);
EXTERN void add_cfg_dump_id(DSP_DATADUMP_MASK_BIT dumpID);
EXTERN void remove_cfg_dump_id(DSP_DATADUMP_MASK_BIT dumpID);

#ifdef AIR_AUDIO_DUMP_BY_SPDIF_ENABLE
EXTERN void audio_dump_task_handler(uint32_t arg);
EXTERN void audio_dump_init(void);
#endif

#endif //__DSP_DUMP_H__
