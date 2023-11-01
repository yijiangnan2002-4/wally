/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "bt_type.h"
#include "bt_os_layer_api.h"

#include "bt_ull_service.h"
#include "bt_ull_utility.h"
#include "bt_ull_le_service.h"
#include "bt_ull_le_utility.h"
//#include "bt_ull_le_conn_service.h"
#include "bt_avm.h"
#include "avm_external.h"
#include "nvkey.h"
#include "nvkey_id_list.h"

#include "bt_ull_le_hid_utility.h"

/**************************************************************************************************
* Define
**************************************************************************************************/

/*Channel for headset or earbuds*/
#define BT_ULL_LE_SHARE_INFO_DL_CH_1 0x00
#define BT_ULL_LE_SHARE_INFO_UL_CH_1 0x01
#define BT_ULL_LE_SHARE_INFO_DL_CH_2 0x02
#define BT_ULL_LE_SHARE_INFO_UL_CH_2 0x03
#define BT_ULL_LE_SHARE_INFO_MAX_NUM 0x04
#ifdef AIR_WIRELESS_MIC_ENABLE
#define BT_ULL_LE_SHARE_INFO_MIC_1   0x00
#define BT_ULL_LE_SHARE_INFO_MIC_2   0x01
#define BT_ULL_LE_SHARE_INFO_MIC_3   0x02
#define BT_ULL_LE_SHARE_INFO_MIC_4   0x03
#define BT_ULL_LE_SHARE_INFO_MIC_MAX_NUM 0x04

#endif
/*Channel for wireless mic*/
#ifndef AIR_WIRELESS_MIC_ENABLE
#define BT_ULL_LE_TRANSMITTER_SHARE_INFO_MAX     (BT_ULL_LE_SHARE_INFO_UL_CH_2 + 1)
#else
#define BT_ULL_LE_TRANSMITTER_SHARE_INFO_MAX     (BT_ULL_LE_SHARE_INFO_MIC_4 + 1)
#endif
typedef uint8_t bt_ull_le_share_info_t;
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
/*Share buffer size for headset or earbuds*/
#define BT_ULL_LE_SHARE_BUFFER_DL_CH1_SIZE     ((294 + sizeof(bt_ull_le_share_buffer_header) + 3) / 4 * 4) * 7 //2184 ->( sdu_size + dsp header ) * (FT + 1) 4 byte align
#define BT_ULL_LE_SHARE_BUFFER_DL_CH2_SIZE     ((294 + sizeof(bt_ull_le_share_buffer_header) + 3) / 4 * 4) * 7 //2184 ->( sdu_size + dsp header ) * (FT + 1)
#define BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE     ((65 + sizeof(bt_ull_le_share_buffer_header) + 3) / 4 * 4) * 7 //672 ->( sdu_size + dsp header ) * (FT + 1)
#define BT_ULL_LE_SHARE_BUFFER_UL_CH2_SIZE     ((65 + sizeof(bt_ull_le_share_buffer_header) + 3) / 4 * 4) * 7 //672 ->( sdu_size + dsp header ) * (FT + 1)
#else
/*Share buffer size for headset or earbuds*/
#define BT_ULL_LE_SHARE_BUFFER_DL_CH1_SIZE     (1*512+408) // 920
#define BT_ULL_LE_SHARE_BUFFER_DL_CH2_SIZE     (1*512+408) // 920
#define BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE     (1*512+408) // 920
#define BT_ULL_LE_SHARE_BUFFER_UL_CH2_SIZE     (1*512+408) // 920
#endif
#ifdef AIR_WIRELESS_MIC_ENABLE
/*Share buffer size for wireless mic*/
#define BT_ULL_LE_SHARE_BUFFER_MIC_CH1_SIZE     (1*512+408) // 920
#define BT_ULL_LE_SHARE_BUFFER_MIC_CH2_SIZE     (1*512+408) // 920
#define BT_ULL_LE_SHARE_BUFFER_MIC_CH3_SIZE     (1*512+408) // 920
#define BT_ULL_LE_SHARE_BUFFER_MIC_CH4_SIZE     (1*512+408) // 920
#endif


#define BT_ULL_LE_DEFAULT_KEY_LENGTH          (32)
#define BT_ULL_LE_RSI_PRAND_SIZE              (3)

/**************************************************************************************************
* Structure
**************************************************************************************************/

/**
 * @brief Define the ULL Dongle share buffer header.
 */
typedef struct  {
    uint16_t DataOffset;
    uint16_t _reserved_word_02h;
    uint32_t TimeStamp;
    uint16_t ConnEvtCnt;
    uint8_t SampleSeq;
    uint8_t _reserved_byte_0Bh;
    uint8_t PduHdrLo;
    uint8_t _reserved_byte_0Dh;
    uint8_t PduLen ;
    uint8_t _reserved_byte_0Fh;
    uint16_t DataLen;
    uint16_t _reserved_word_12h;
    uint32_t CrcValue;
    uint32_t _reserved_long_1;
} bt_ull_le_share_buffer_header;

/**************************************************************************************************
* Variable
**************************************************************************************************/
/*Share buffer start address for headset or earbuds*/
uint32_t g_ull_le_dl_ch1_addr;
uint32_t g_ull_le_dl_ch2_addr;
uint32_t g_ull_le_ul_ch1_addr;
uint32_t g_ull_le_ul_ch2_addr;
#ifdef AIR_WIRELESS_MIC_ENABLE
/*Share buffer point for wireless mic*/
uint32_t g_ull_le_mic_ch1_addr;
uint32_t g_ull_le_mic_ch2_addr;
uint32_t g_ull_le_mic_ch3_addr;
uint32_t g_ull_le_mic_ch4_addr;
#endif

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
extern bt_ull_le_srv_context_t g_ull_le_ctx;
#endif

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
extern bt_ull_le_hid_srv_context_t g_bt_ull_hid_le_ctx;
#endif

static n9_dsp_share_info_t *g_ull_le_share_info[BT_ULL_LE_TRANSMITTER_SHARE_INFO_MAX];
static uint8_t const g_ull_le_uuid128_default[BT_ULL_LE_MAX_UUID_LENGTH] = {
    0x45, 0x4C, 0x42, 0x61, 0x68, 0x6F, 0x72, 0x69,               \
    0x41, 0x07, 0xAB, 0x2D, 0x4D, 0x49, 0x52, 0x50
};
static const uint32_t g_ull_le_sdu_interval_tbl[] = {
    /*  sdu_interval (us)   bt_ull_le_sdu_interval_t */
    0,
    1250,
    2500,
    5000,
    7500,
    10000,
    1000,
    2000,
};
/**
 * @brief This global vriable define the 48K/96K codec sample rate switch
 */
#ifdef AIR_DCHS_MODE_ENABLE
uint32_t g_ull_le_dchs_dl_codec_samplerate = 48000;
#endif

/******************************Codec table**********************************************/
/*    Scenario                      |  AB156x DL,        AB156x UL   |  AB158x DL,        AB158x UL              */
/*    ULL2.0_Lc3Plus            | 96K/304kbps,   16K/64kbps    | 96K/304kbps,   32K/64kbps              */
/*    ULL2.0_Lc3Plus)           | 96K/172kbps,   16K/64kbps    | 96K/172kbps,   32K/64kbps              */
/*    ULL2.0_Dual chip/LP     | 48K/200kbps,   16K/64kbps    | 48K/200kbps,   32K/64kbps              */
/*    ULL2.0(Opus)               | 48K/320kbps,   16K/64kbps    |       N/A,              N/A                    */
/*___________________________________________________________________________________*/
/*    WMic_Lc3Plus_Stereo    |      N/A,           48K/200kbps  |       N/A,              N/A                     */
/*    WMic_Lc3Plus_Mono      |      N/A,           48K/100kbps  |       N/A,              N/A                    */
/******************************Codec table**********************************************/

/***********************************************************Down Link***********************************************************************************/
bt_ull_le_codec_param_t g_ull_le_spk_stream_param_samp_48k_br_200k_stero = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_STEREO,              /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_126,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_201_6_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_48K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_spk_stream_param_samp_48k_br_160k_stero = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_1,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_STEREO,              /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_100,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_160_0_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_48K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

//for 1565 opus codec
bt_ull_le_codec_param_t g_ull_le_spk_stream_param_samp_48k_br_320k_stero = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_DUAL_MONO,           /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_200,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_320_0_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_48K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_spk_stream_param_samp_96k_br_172k_stero = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_4,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_STEREO,              /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_108,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_172_8_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_96K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_spk_stream_param_samp_96k_br_272k_stero = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_STEREO,              /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_170,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_272_0_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_96K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_spk_stream_param_samp_96k_br_304k_stero = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_3,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_STEREO,              /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_190,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_304_0_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_96K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_spk_stream_param_samp_96k_br_560k_stero = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_3,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_STEREO,              /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_350,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_560_0_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_96K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

//for 1577 mHDT high-res audio
bt_ull_le_codec_param_t g_ull_le_spk_stream_param_samp_96k_br_940k_stero = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_DUAL_MONO,           /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_588,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_940_8_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_96K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_spk_stream_codec_uld_samp_48k_br_400k_stero = {
    BT_ULL_LE_SDU_INTERVAL_2000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_STEREO,              /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_100,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_400_0_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_48K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

/***********************************************************Up Link***********************************************************************************/
bt_ull_le_codec_param_t g_ull_le_mic_stream_param_samp_16k_br_32k_mono = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_4,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_MONO,                /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_20,       /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_32_0_KBPS,        /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_16K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_mic_stream_param_samp_16k_br_64k_mono = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_3,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_MONO,                /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_40,     /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_64_0_KBPS,     /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_16K,             /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_mic_stream_codec_uld_samp_32k_br_64k_mono = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_MONO,                /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_40,       /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_64_0_KBPS,        /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_32K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};
bt_ull_le_codec_param_t g_ull_le_mic_stream_param_samp_32k_br_64k_mono = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_4,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_MONO,                /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_40,       /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_64_0_KBPS,        /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_32K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_mic_stream_param_samp_16k_br_104k_mono = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_DUAL_MONO,           /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_65,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_104_0_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_16K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_mic_stream_param_samp_32k_br_104k_mono = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_DUAL_MONO,           /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_65,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_104_0_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_32K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

bt_ull_le_codec_param_t g_ull_le_mic_stream_param_samp_48k_br_104k_mono = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_DUAL_MONO,           /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_65,    /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_104_0_KBPS,       /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_48K,            /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};

//wireless mic ul stero
bt_ull_le_codec_param_t g_ull_le_mic_stream_param_samp_48k_br_200k_stero = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_1,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_STEREO,              /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_126,     /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_201_6_KBPS,        /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_48K,             /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};
//wireless mic ul mono
bt_ull_le_codec_param_t g_ull_le_mic_stream_param_samp_48k_br_100k_mono = {
    BT_ULL_LE_SDU_INTERVAL_5000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_1,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_MONO,                /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_63,     /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_100_8_KBPS,        /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_48K,             /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};
//wireless mic ul mono (ULD codec)
bt_ull_le_codec_param_t g_ull_le_mic_stream_uld_codec_param_samp_48k_br_200k_mono = {
    BT_ULL_LE_SDU_INTERVAL_1000_US,             /* SDU interval */
    BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,          /* Retransmission number, Max_Share_num*/
    BT_ULL_LE_DEFAULT_MAX_TRANSPORT_LATENCY,    /* Max transport latency (ms) */
    BT_ULL_LE_CHANNEL_MODE_MONO,                /**< b0: Mono, b1: Stereo: b2:dual channel. */
    BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_25,     /* Maximum SDU size (unit: octets) */
    BT_ULL_LE_DEFAULT_BITRATE_200_0_KBPS,        /* bitrate. uinit:bps */
    BT_ULL_LE_DEFAULT_SAMPLERTE_48K,             /* Streaming data sample rate, unit:HZ (16000,44100,48000,96000...) */
};



typedef struct {
    bt_ull_streaming_interface_t interface;
    uint8_t port_num;
    bt_ull_transmitter_t transmitter;
    bt_ull_le_stream_port_mask_t streaming_port;
    bt_ull_le_srv_audio_out_t audio_out;
    bt_ull_le_stream_mode_t stream_mode;
} bt_ull_le_srv_port_transmitter_mapping_t;

bt_ull_le_srv_stream_context_t g_ull_le_stream_ctx;

static const bt_ull_le_srv_port_transmitter_mapping_t g_ull_le_port_trans_mapping_table[BT_ULL_TRANSMITTER_MAX_NUM] =
{
    {BT_ULL_STREAMING_INTERFACE_SPEAKER,    0, BT_ULL_GAMING_TRANSMITTER,   BT_ULL_LE_STREAM_PORT_MASK_GAMING,   BT_ULL_LE_SRV_AUDIO_OUT_USB, BT_ULL_LE_STREAM_MODE_DOWNLINK},
    {BT_ULL_STREAMING_INTERFACE_SPEAKER,    1, BT_ULL_CHAT_TRANSMITTER,     BT_ULL_LE_STREAM_PORT_MASK_CHAT,     BT_ULL_LE_SRV_AUDIO_OUT_USB, BT_ULL_LE_STREAM_MODE_DOWNLINK},
    {BT_ULL_STREAMING_INTERFACE_MICROPHONE, 0, BT_ULL_MIC_TRANSMITTER,      BT_ULL_LE_STREAM_PORT_MASK_MIC,      BT_ULL_LE_SRV_AUDIO_OUT_USB, BT_ULL_LE_STREAM_MODE_UPLINK},
    {BT_ULL_STREAMING_INTERFACE_LINE_IN,    0, BT_ULL_LINE_IN_TRANSMITTER,  BT_ULL_LE_STREAM_PORT_MASK_LINE_IN,  BT_ULL_LE_SRV_AUDIO_OUT_AUX, BT_ULL_LE_STREAM_MODE_DOWNLINK},
    {BT_ULL_STREAMING_INTERFACE_LINE_OUT,   0, BT_ULL_LINE_OUT_TRANSMITTER, BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT, BT_ULL_LE_SRV_AUDIO_OUT_AUX, BT_ULL_LE_STREAM_MODE_UPLINK},
    {BT_ULL_STREAMING_INTERFACE_I2S_IN,     0, BT_ULL_I2S_IN_TRANSMITTER,   BT_ULL_LE_STREAM_PORT_MASK_I2S_IN,   BT_ULL_LE_SRV_AUDIO_OUT_I2S, BT_ULL_LE_STREAM_MODE_DOWNLINK},
    {BT_ULL_STREAMING_INTERFACE_I2S_OUT,    0, BT_ULL_I2S_OUT_TRANSMITTER,  BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT,  BT_ULL_LE_SRV_AUDIO_OUT_I2S, BT_ULL_LE_STREAM_MODE_UPLINK}
};

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    extern bt_ull_le_srv_context_t g_ull_le_ctx;
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    extern bt_ull_le_hid_srv_context_t g_bt_ull_hid_le_ctx;
#endif

/**************************************************************************************************
* Prototype
**************************************************************************************************/
//extern unsigned char* bt_pka_get_leaudio_AVM_addr(uint16_t size);
//extern bt_ull_le_srv_stream_context_t *bt_ull_le_srv_get_stream_context(void);
extern void hal_audio_set_le_audio_avm_buf_size(audio_message_type_t type, uint32_t buf_size);

static void bt_ull_le_srv_allocate_avm_block_addr(bt_ull_client_t client_type);
static n9_dsp_share_info_t *bt_ull_le_srv_get_avm_buffer_info(bt_ull_le_share_info_t type, bt_ull_client_t client_type, uint32_t bit_rate,
                                                              uint32_t frame_interval, uint32_t frame_size);
static void bt_ull_le_srv_set_codec_param(bt_ull_role_t role, bt_ull_le_codec_param_t *dl_codec_param, bt_ull_le_codec_param_t *ul_codec_param) ;

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void *bt_ull_le_srv_memcpy(void *dest, const void *src, uint32_t size)
{
    if ((NULL == dest) || (NULL == src)) {
        return NULL;
    }
    return memcpy(dest, src, size);
}

void *bt_ull_le_srv_memory_alloc(uint16_t size)
{
    void *memory = (void *)pvPortMalloc(size);
    if (NULL != memory) {
        memset(memory, 0, size);
    }
    return memory;
}

void bt_ull_le_srv_memory_free(void *point)
{
    if (point) {
        vPortFree(point);
    }
}

void *bt_ull_le_srv_memset(void *buf, uint8_t value, uint32_t size)
{
    return memset(buf, value, size);
}

int32_t bt_ull_le_srv_memcmp(const void *buf1, const void *buf2, uint32_t size)
{
    return memcmp(buf1, buf2, size);
}

void bt_ull_le_srv_reverse_copy(uint8_t *dst, const uint8_t *src, uint32_t length)
{
    uint32_t i;

    for (i = 0; i < length; i++) {
        dst[i] = src[length - i - 1];
    }
}

void bt_ull_le_srv_random(bt_key_t output)
{
    uint8_t i;
    for (i = 0; i < sizeof(bt_key_t); i += 2) {
        uint16_t r =  bt_os_layer_generate_random();
        bt_ull_le_srv_memcpy(output + i, &r, 2);
    }
}

void bt_ull_le_srv_encrypt(bt_key_t output, const bt_key_t key, const bt_key_t data)
{
    bt_os_layer_aes_buffer_t encrypted_data, plain_text, key_struct;
    uint8_t buffer[BT_ULL_LE_DEFAULT_KEY_LENGTH] = {0};

    key_struct.buffer = (void *)key;
    key_struct.length = sizeof(bt_key_t);

    plain_text.buffer = (void *)data;
    plain_text.length = sizeof(bt_key_t);

    encrypted_data.buffer = buffer;
    encrypted_data.length = BT_ULL_LE_DEFAULT_KEY_LENGTH;//check size

    bt_os_layer_aes_encrypt(&encrypted_data, &plain_text, &key_struct);
    bt_ull_le_srv_memcpy(output, buffer, sizeof(bt_key_t));
}

bool bt_ull_le_srv_endian_order_swap(uint8_t *dest, const uint8_t *src, uint8_t len)
{
    uint8_t temp[16] = {0}; /*Add temp variable to support dest and src are same point*/

    if (len == 2 || len == 4 || len == 8 || len == 16) {
        uint8_t i;
        for (i = 0; i < len; i++) {
            *(temp + i) = *(src + (len - 1) - i);
        }
        bt_ull_le_srv_memcpy(dest, temp, len);
        return true;
    }
    return false;
}

bt_status_t bt_ull_le_srv_verify_rsi(bt_ull_le_rsi_t rsi)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)

    uint8_t h[sizeof(bt_key_t)], p[sizeof(bt_key_t)], b_sirk[sizeof(bt_key_t)], result[sizeof(bt_key_t)];

    if (bt_ull_le_srv_sirk_is_null(g_ull_le_ctx.sirk)) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_verify_rsi, Fail! Sirk is NULL!", 0);
        return BT_STATUS_FAIL;
    }

    memset(p, 0, sizeof(bt_key_t));
    p[13] = rsi[5];
    p[14] = rsi[4];
    p[15] = rsi[3];

    bt_ull_le_srv_reverse_copy(b_sirk, g_ull_le_ctx.sirk, sizeof(bt_key_t));
    bt_ull_le_srv_encrypt(h, b_sirk, p);  //h:output, p:plain text in big endian
    bt_ull_le_srv_endian_order_swap(result, h, sizeof(bt_key_t));

    if (0 == memcmp(result, rsi, BT_ULL_LE_RSI_PRAND_SIZE)) {
        ull_report("[ULL][LE] bt_ull_le_srv_verify_rsi, PASS!! rsi[0~2]:%x-%x-%x, Local value[0~2]:%x-%x-%x", 6,
            rsi[0], rsi[1], rsi[2], result[0], result[1], result[2]);
        return BT_STATUS_SUCCESS;
    }
    /*
    ull_report_error("[ULL][LE] bt_ull_le_srv_verify_rsi, Fail!, rsi[0~2]:%x-%x-%x, Local value[0~2]:%x-%x-%x", 6,
                     rsi[0], rsi[1], rsi[2], result[0], result[1], result[2]);
                     */
#endif
    return BT_STATUS_FAIL;
}

const bt_ull_le_uuid_t *bt_ull_le_srv_get_uuid(void)
{
    return &g_ull_le_uuid128_default;
}

bt_ull_client_t bt_ull_le_srv_get_client_type(void)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    return g_ull_le_ctx.client_type;
#elif defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    return g_bt_ull_hid_le_ctx.audio_sink.client_type;
#endif
    return BT_ULL_INVALID_CLIENT;
}

bt_status_t bt_ull_le_srv_get_rsi(bt_ull_le_rsi_t rsi)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    uint8_t h[sizeof(bt_key_t)], p[sizeof(bt_key_t)], b_sirk[sizeof(bt_key_t)];
    uint8_t i;

    if (bt_ull_le_srv_sirk_is_null(g_ull_le_ctx.sirk)) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_get_rsi, Fail", 0);
        return BT_STATUS_FAIL;
    }

    bt_ull_le_srv_random(h);
    memset(p, 0, sizeof(bt_key_t));
    memcpy(&p[13], h, BT_ULL_LE_RSI_PRAND_SIZE); //big endian
    p[13] = (0x40 | (p[13] >> 4));

    bt_ull_le_srv_reverse_copy(b_sirk, g_ull_le_ctx.sirk, sizeof(bt_key_t));
    bt_ull_le_srv_encrypt(h, b_sirk, p);  //h:output, p:plain text in big endian

    for (i = 0; i < 3; i++) {
        rsi[i] = h[15 - i];  //h: hash
        rsi[i + 3] = p[15 - i]; //p: prand
    }
    ull_report("[ULL][LE] Get RSI: %2x-%2x-%2x-%2x-%2x-%2x", 6, rsi[0], rsi[1], rsi[2], rsi[3], rsi[4], rsi[5]);
#endif
    return BT_STATUS_SUCCESS;
}

bt_key_t *bt_ull_le_srv_get_sirk(void)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)

    if (bt_ull_le_srv_sirk_is_null(g_ull_le_ctx.sirk)) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_get_sirk, Fail", 0);
        return NULL;
    }
    return &(g_ull_le_ctx.sirk);
#else
    return NULL;
#endif
}

bt_ull_role_t bt_ull_le_srv_get_role(void)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    return g_ull_le_ctx.role;
#elif defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    return g_bt_ull_hid_le_ctx.role;
#endif
    return BT_ULL_ROLE_UNKNOWN;
}

bt_status_t bt_ull_le_srv_set_sirk(bt_key_t sirk)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    if (!bt_ull_le_srv_sirk_is_null(sirk)) {
        memcpy(&(g_ull_le_ctx.sirk), sirk, sizeof(bt_key_t));

        ull_report("[ULL][LE] Set SIRK: %2x-%2x-%2x-%2x-%2x-%2x-%2x-%2x", 8, sirk[0], sirk[1], sirk[2], sirk[3],
                   sirk[4], sirk[5], sirk[6], sirk[7]);
        ull_report("[ULL][LE] Set SIRK: %2x-%2x-%2x-%2x-%2x-%2x-%2x-%2x", 8, sirk[8], sirk[9], sirk[10], sirk[11],
                   sirk[12], sirk[13], sirk[14], sirk[15]);

        return BT_STATUS_SUCCESS;
    }
#endif
    ull_report_error("[ULL][LE] bt_ull_le_srv_set_sirk, Fail", 0);
    return BT_STATUS_FAIL;
}

bool bt_ull_le_srv_sirk_is_null(bt_key_t sirk)
{
    bt_key_t invalid_sirk;
    memset(&invalid_sirk, 0x00, sizeof(bt_key_t));

    if (0 == memcmp(&sirk, &invalid_sirk, sizeof(bt_key_t))) {
        return true;
    }
    return false;
}

uint8_t bt_ull_le_srv_get_coordinated_set_size(void)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    return g_ull_le_ctx.cs_size;
#else
    return 0;
#endif
}

bt_status_t bt_ull_le_srv_set_coordinated_set_size(uint8_t size)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    if (0 == size) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_set_coordinated_set_size, invalid size", 0);
        return BT_STATUS_FAIL;
    }
    g_ull_le_ctx.cs_size = size;
    return BT_STATUS_SUCCESS;
#else
    return 0;
#endif
}

bt_ull_le_codec_t bt_ull_le_srv_get_codec_type(void)
{
    return bt_ull_le_srv_get_stream_context()->codec_type;
}

uint32_t bt_ull_le_srv_get_ul_sample_rate(void)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    return g_ull_le_ctx.client_preferred_codec_param.ul_samplerate;
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    return g_bt_ull_hid_le_ctx.audio_sink.client_preferred_codec_param.ul_samplerate;
#endif

    return 0;
}

uint32_t bt_ull_le_srv_get_dl_sample_rate(void)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    return g_ull_le_ctx.client_preferred_codec_param.dl_samplerate;
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    return g_bt_ull_hid_le_ctx.audio_sink.client_preferred_codec_param.dl_samplerate;
#endif
    return 0;
}

uint32_t bt_ull_le_srv_get_codec_sample_rate(bt_ull_transmitter_t transmitter, bool is_uplink, bt_ull_role_t role)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_SERVER == role) {
        if ((BT_ULL_GAMING_TRANSMITTER <= transmitter) && (BT_ULL_TRANSMITTER_MAX_NUM > transmitter)) {
            return stream_ctx->server.stream[transmitter].codec_param.sample_rate;
        }
    } else if (BT_ULL_ROLE_CLIENT == role) {
        if (is_uplink) {
            return stream_ctx->client.ul.codec_param.sample_rate;
        } else {
            return stream_ctx->client.dl.codec_param.sample_rate;
        }
    }

    ull_report_error("[ULL][LE] bt_ull_le_srv_get_codec_sample_rate, transmitter %d not suppotrt", 1, transmitter);
    return 0;
}

uint32_t bt_ull_le_srv_get_usb_sample_rate(bt_ull_transmitter_t transmitter, bt_ull_role_t role)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_SERVER == role) {
        if ((BT_ULL_GAMING_TRANSMITTER <= transmitter) && (BT_ULL_TRANSMITTER_MAX_NUM > transmitter)) {
            return stream_ctx->server.stream[transmitter].usb_sample_rate;
        }
    }

    ull_report_error("[ULL][LE] bt_ull_le_srv_get_usb_sample_rate, transmitter %d not suppotrt", 1, transmitter);
    return 0;
}

uint32_t bt_ull_le_srv_get_usb_sample_size(bt_ull_transmitter_t transmitter, bt_ull_role_t role)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_SERVER == role) {
        if ((BT_ULL_GAMING_TRANSMITTER <= transmitter) && (BT_ULL_TRANSMITTER_MAX_NUM > transmitter)) {
            return stream_ctx->server.stream[transmitter].usb_sample_size;
        }
    }

    ull_report_error("[ULL][LE] bt_ull_le_srv_get_usb_sample_size, transmitter %d not suppotrt", 1, transmitter);
    return 0;
}
uint32_t bt_ull_le_srv_get_usb_sample_channel(bt_ull_transmitter_t transmitter, bt_ull_role_t role)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_SERVER == role) {
        if ((BT_ULL_GAMING_TRANSMITTER <= transmitter) && (BT_ULL_TRANSMITTER_MAX_NUM > transmitter)) {
            return stream_ctx->server.stream[transmitter].usb_sample_channel;
        }
    }

    ull_report_error("[ULL][LE] bt_ull_le_srv_get_usb_sample_size, transmitter %d not suppotrt", 1, transmitter);
    return 0;
}

uint32_t bt_ull_le_srv_get_bitrate(bool is_uplink, bt_ull_role_t role)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_SERVER == role) {
        if (is_uplink) {
            return stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].codec_param.bit_rate;
        } else if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
            return stream_ctx->server.stream[BT_ULL_CHAT_TRANSMITTER].codec_param.bit_rate;
        } else if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
            return stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].codec_param.bit_rate;
        } else {//DL default value
            return stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].codec_param.bit_rate;
        }
    } else if (BT_ULL_ROLE_CLIENT == role) {
        if (is_uplink) {
            return stream_ctx->client.ul.codec_param.bit_rate;
        } else {
            return stream_ctx->client.dl.codec_param.bit_rate;
        }
    }

    ull_report_error("[ULL][LE] bt_ull_le_srv_get_bitrate, error, is_uplink %d is_uplink", 1, is_uplink);
    return 0;
}

uint32_t bt_ull_le_srv_get_sdu_interval(bool is_uplink, bt_ull_role_t role)
{
    bt_ull_le_sdu_interval_t sdu_interval = BT_ULL_LE_SDU_INTERVAL_INVALID;
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_SERVER == role) {
        if (is_uplink) {
            sdu_interval = stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].codec_param.sdu_interval;
        } else if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
            sdu_interval = stream_ctx->server.stream[BT_ULL_CHAT_TRANSMITTER].codec_param.sdu_interval;
        } else if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
            sdu_interval = stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].codec_param.sdu_interval;
        } else {//DL default value
            sdu_interval = stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].codec_param.sdu_interval;
        }
    } else if (BT_ULL_ROLE_CLIENT == role) {
        if (is_uplink) {
            sdu_interval = stream_ctx->client.ul.codec_param.sdu_interval;
        } else {
            sdu_interval = stream_ctx->client.dl.codec_param.sdu_interval;
        }
    }
    return g_ull_le_sdu_interval_tbl[sdu_interval];
}

uint16_t bt_ull_le_srv_get_sdu_size(bool is_uplink, bt_ull_role_t role)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_SERVER == role) {
        if (is_uplink) {
            return stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].codec_param.sdu_size;
        } else if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
            return stream_ctx->server.stream[BT_ULL_CHAT_TRANSMITTER].codec_param.sdu_size;
        } else if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
            return stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].codec_param.sdu_size;
        } else {//DL default value
            return stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].codec_param.sdu_size;
        }
    } else if (BT_ULL_ROLE_CLIENT == role) {
        if (is_uplink) {
            return stream_ctx->client.ul.codec_param.sdu_size;
        } else {
            return stream_ctx->client.dl.codec_param.sdu_size;
        }
    }

    ull_report_error("[ULL][LE] bt_ull_le_srv_get_sdu_size, error, is_uplink %d is_uplink", 1, is_uplink);
    return 0;
}

#ifdef AIR_WIRELESS_MIC_RX_ENABLE
uint8_t bt_ull_le_srv_get_air_cis_count(void)
{
    return g_ull_le_ctx.cis_num;
}
#endif
bt_ull_le_channel_mode_t bt_ull_le_srv_get_channel_mode(bt_ull_transmitter_t transmitter, bool is_uplink, bt_ull_role_t role)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_SERVER == role) {
        if ((BT_ULL_GAMING_TRANSMITTER <= transmitter) && (BT_ULL_TRANSMITTER_MAX_NUM > transmitter)) {
            return stream_ctx->server.stream[transmitter].codec_param.channel_mode;
        }
    } else if (BT_ULL_ROLE_CLIENT == role) {
        if (is_uplink) {
            return stream_ctx->client.ul.codec_param.channel_mode;
        }
        return stream_ctx->client.dl.codec_param.channel_mode;
    }

    ull_report_error("[ULL][LE] bt_ull_le_srv_get_channel_mode, error, is_uplink %d is_uplink", 1, is_uplink);
    return BT_ULL_LE_CHANNEL_MODE_MONO;
}

void bt_ull_le_srv_reset_stream_ctx(void)
{
    uint8_t i;
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    bt_ull_le_srv_memset(stream_ctx, 0x00, sizeof(bt_ull_le_srv_stream_context_t));
    stream_ctx->dl_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
    stream_ctx->ul_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;

    /**< Default Codec is LC3plus. */
    stream_ctx->codec_type = BT_ULL_LE_CODEC_LC3PLUS;
    stream_ctx->audio_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT;
    bt_ull_role_t role = bt_ull_le_srv_get_role();
    if (BT_ULL_ROLE_SERVER == role) {
        /**< Default Codec Parameters. */
#ifdef AIR_BTA_IC_PREMIUM_G3
        bt_ull_le_srv_set_codec_param(BT_ULL_ROLE_SERVER, &(g_ull_le_spk_stream_param_samp_96k_br_304k_stero), &(g_ull_le_mic_stream_param_samp_32k_br_64k_mono));
        ull_report("[ULL][LE] chip(ULL 2.0): ab158x, ull v2 server, DL default codec param(96KHZ/304Kbps), UL default codec param(32KHZ/64Kbps)!", 0);
#elif defined(AIR_BTA_IC_PREMIUM_G2)
    #ifdef AIR_WIRELESS_MIC_ENABLE
        bt_ull_le_srv_set_codec_param(BT_ULL_ROLE_SERVER, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_48k_br_200k_stero));
        ull_report("[ULL][LE] chip(WIRELESS MIC): ab156x, wireless mic RX, DL default codec param(48KHZ/201.6Kbps), UL default codec param(48KHZ/201.6Kbps)!", 0);
#else
        bt_ull_le_srv_set_codec_param(BT_ULL_ROLE_SERVER, &(g_ull_le_spk_stream_param_samp_96k_br_304k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_64k_mono));
        ull_report("[ULL][LE] chip: ab156x(ULL 2.0), ull v2 Server, DL default codec param(96KHZ/304Kbps), UL default codec param(16KHZ/64Kbps)!", 0);
#endif
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        bt_ull_le_srv_set_codec_param(BT_ULL_ROLE_SERVER, &(g_ull_le_spk_stream_param_samp_48k_br_160k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_32k_mono));
        ull_report("[ULL][LE] chip(ULL 2.0 HID): ab157x, ull v2 server, DL default codec param(48KHZ/200 Kbps), UL default codec param(16KHZ/32Kbps)!", 0);
#else
        bt_ull_le_srv_set_codec_param(BT_ULL_ROLE_SERVER, &(g_ull_le_spk_stream_param_samp_96k_br_304k_stero), &(g_ull_le_mic_stream_param_samp_32k_br_64k_mono));
        ull_report("[ULL][LE] chip(ULL 2.0): ab157x, ull v2 server, DL default codec param(96KHZ/304Kbps), UL default codec param(32KHZ/64Kbps)!", 0);
#endif
#else
        ull_report_error("[ULL][LE] bt_ull_le_srv_reset_stream_ctx, invalid chip version", 0);
#endif
        stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].usb_sample_rate = 0x00;
        stream_ctx->server.stream[BT_ULL_CHAT_TRANSMITTER].usb_sample_rate = 0x00;
        stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].usb_sample_rate = 0x00;
    } else if (BT_ULL_ROLE_CLIENT == role) {
        bt_ull_le_srv_client_preferred_codec_param *codec_param = bt_ull_le_srv_get_client_preferred_codec_param();
        if (!codec_param) {
            ull_report_error("[ULL][LE] bt_ull_le_srv_reset_stream_ctx, invalid params-1", 0);
            return;
        }
        #ifdef AIR_ULL_AUDIO_V3_ENABLE
        codec_param->codec_type = bt_ull_le_srv_get_client_preffered_codec(BT_ULL_LE_SCENARIO_ULLV3_0);
        codec_param->dl_samplerate = bt_ull_le_srv_get_client_preffered_dl_codec_samplerate(BT_ULL_LE_SCENARIO_ULLV3_0);
        codec_param->ul_samplerate = bt_ull_le_srv_get_client_preffered_ul_codec_samplerate(BT_ULL_LE_SCENARIO_ULLV3_0);
        #else
        codec_param->codec_type = bt_ull_le_srv_get_client_preffered_codec(BT_ULL_LE_SCENARIO_ULLV2_0);
        codec_param->dl_samplerate = bt_ull_le_srv_get_client_preffered_dl_codec_samplerate(BT_ULL_LE_SCENARIO_ULLV2_0);
        codec_param->ul_samplerate = bt_ull_le_srv_get_client_preffered_ul_codec_samplerate(BT_ULL_LE_SCENARIO_ULLV2_0);
        #endif
        stream_ctx->codec_type = codec_param->codec_type;
        bt_ull_le_srv_set_codec_param_by_sample_rate(BT_ULL_ROLE_CLIENT, codec_param->dl_samplerate, codec_param->ul_samplerate);
    } else {
        ull_report_error("[ULL][LE] bt_ull_le_srv_reset_stream_ctx, invalid role", 0);
        return;
    }

    /**< Default gaming && chat streaming 100% mix. */
    stream_ctx->dl_mix_ratio.num_streaming = BT_ULL_MAX_STREAMING_NUM;
    for (i = 0; i < BT_ULL_MAX_STREAMING_NUM; i ++) {
        stream_ctx->dl_mix_ratio.streamings[i].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        stream_ctx->dl_mix_ratio.streamings[i].streaming.port = i; /* gaming streaming port */
        stream_ctx->dl_mix_ratio.streamings[i].ratio = BT_ULL_MIX_RATIO_MAX;
    }
    ull_report("[ULL][LE] reset stream ctx, YES!", 0);
}

bt_ull_le_srv_audio_quality_t bt_ull_le_srv_read_aud_quality_from_nvkey(void)
{
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    bt_ull_le_srv_audio_quality_t quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID;
    uint32_t size = sizeof(bt_ull_le_srv_audio_quality_t);
    status = nvkey_read_data(NVID_BT_HOST_ULL_AUDIO_QUALITY, (uint8_t *)&quality, &size);
    if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_read_aud_quality_from_nvkey, error status:%d, size: %d", 2, status, size);
    } else {
        ull_report("[ULL][LE] bt_ull_le_srv_read_aud_quality_from_nvkey, status:%d, quality: %d", 2, status, quality);
    }
    return quality;
}

bt_status_t bt_ull_le_srv_write_aud_quality_to_nvkey(bt_ull_le_srv_audio_quality_t quality)
{
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    uint32_t size = sizeof(bt_ull_le_srv_audio_quality_t);
    bt_ull_le_srv_audio_quality_t temp_quality = quality;
    status = nvkey_write_data(NVID_BT_HOST_ULL_AUDIO_QUALITY, (const uint8_t *)&temp_quality, size);
    return NVKEY_STATUS_OK == status ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

void bt_ull_le_srv_change_audio_quality(bt_ull_role_t role, bt_ull_le_srv_audio_quality_t quality)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_srv_client_preferred_codec_param *codec_param = &(g_ull_le_ctx.client_preferred_codec_param);
    ull_report("[ULL][LE] bt_ull_le_srv_change_audio_quality, %d, codec:%d, ul: %d, dl: %d!", 4, \
        quality, codec_param->codec_type, codec_param->ul_samplerate, codec_param->dl_samplerate);

    switch (quality) {
         case BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT: {
             stream_ctx->audio_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT;
             if (BT_ULL_ROLE_CLIENT == role) {
                 codec_param->dl_samplerate = bt_ull_le_srv_get_client_preffered_dl_codec_samplerate(g_ull_le_ctx.client_preferred_scenario_type);
                 codec_param->ul_samplerate = bt_ull_le_srv_get_client_preffered_ul_codec_samplerate(g_ull_le_ctx.client_preferred_scenario_type);
                 codec_param->codec_type = bt_ull_le_srv_get_client_preffered_codec(g_ull_le_ctx.client_preferred_scenario_type);
                 stream_ctx->codec_type = codec_param->codec_type;
                 //bt_ull_le_srv_write_aud_quality_to_nvkey(stream_ctx->audio_quality);
             }
             bt_ull_le_srv_set_codec_param_by_sample_rate(role, codec_param->dl_samplerate, codec_param->ul_samplerate);
             break;
         }
         case BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_HFP: {
             if (BT_ULL_LE_CODEC_LC3PLUS == stream_ctx->codec_type) {
                 stream_ctx->audio_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_HFP;
             }
             break;
         }
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
         case BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY: {
             if (BT_ULL_LE_CODEC_LC3PLUS == stream_ctx->codec_type) {
                 stream_ctx->audio_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY;
                 //bt_ull_le_srv_write_aud_quality_to_nvkey(stream_ctx->audio_quality);
                 if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == codec_param->dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == codec_param->ul_samplerate) {
                     bt_ull_le_srv_set_codec_param(role, &g_ull_le_spk_stream_param_samp_96k_br_560k_stero, &g_ull_le_mic_stream_param_samp_16k_br_104k_mono);
                 } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == codec_param->dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_32K == codec_param->ul_samplerate) {
                     bt_ull_le_srv_set_codec_param(role, &g_ull_le_spk_stream_param_samp_96k_br_560k_stero, &g_ull_le_mic_stream_param_samp_32k_br_104k_mono);
                 } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == codec_param->dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_48K == codec_param->ul_samplerate) {
                     bt_ull_le_srv_set_codec_param(role, &g_ull_le_spk_stream_param_samp_96k_br_560k_stero, &g_ull_le_mic_stream_param_samp_48k_br_104k_mono);
                 } else {
                     ull_report_error("[ULL][LE] bt_ull_le_srv_change_audio_quality, Not support EDRLE 4M", 0);
                 }
             } else {
                 ull_report_error("[ULL][LE] bt_ull_le_srv_change_audio_quality, Codec not support High-quality audio!", 0);
             }
             break;
         }
         case BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION: {
             if (BT_ULL_LE_CODEC_LC3PLUS == stream_ctx->codec_type) {
                 stream_ctx->audio_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION;
                 //bt_ull_le_srv_write_aud_quality_to_nvkey(stream_ctx->audio_quality);
                 if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == codec_param->dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == codec_param->ul_samplerate) {
                     bt_ull_le_srv_set_codec_param(role, &g_ull_le_spk_stream_param_samp_96k_br_940k_stero, &g_ull_le_mic_stream_param_samp_16k_br_104k_mono);
                 } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == codec_param->dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_32K == codec_param->ul_samplerate) {
                     bt_ull_le_srv_set_codec_param(role, &g_ull_le_spk_stream_param_samp_96k_br_940k_stero, &g_ull_le_mic_stream_param_samp_32k_br_104k_mono);
                 } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == codec_param->dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_48K == codec_param->ul_samplerate) {
                     bt_ull_le_srv_set_codec_param(role, &g_ull_le_spk_stream_param_samp_96k_br_940k_stero, &g_ull_le_mic_stream_param_samp_48k_br_104k_mono);
                 } else {
                     ull_report_error("[ULL][LE] bt_ull_le_srv_change_audio_quality, Not support EDRLE 4M", 0);
                 }
             } else {
                 ull_report_error("[ULL][LE] bt_ull_le_srv_change_audio_quality, Codec not support High-Res audio!", 0);
             }
             break;
         }
#endif
         case BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER: {
             if (BT_ULL_LE_CODEC_LC3PLUS == stream_ctx->codec_type) {
                 stream_ctx->audio_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER;
                 if (BT_ULL_ROLE_CLIENT == role) {
                     //bt_ull_le_srv_write_aud_quality_to_nvkey(stream_ctx->audio_quality);
                 }
                 //codec_param->dl_samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_48K;
                 if (BT_ULL_LE_DEFAULT_SAMPLERTE_16K == codec_param->ul_samplerate) {
                     bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_64k_mono));
                 } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_32K == codec_param->ul_samplerate) {
                     bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_32k_br_64k_mono));
                 }
             } else {
                 ull_report_error("[ULL][LE] bt_ull_le_srv_change_audio_quality, Codec not support low power!", 0);
             }
             break;
         }

         default :
             break;

    }
#endif
}

void bt_ull_le_srv_switch_codec_params(bt_ull_role_t role, uint8_t index)
{
    bt_ull_le_codec_param_t *ul_codec_param = NULL;
#ifdef AIR_BTA_IC_PREMIUM_G3
    ul_codec_param = &g_ull_le_mic_stream_param_samp_32k_br_64k_mono;
    ull_report("[ULL][LE] AB158X, UL codec: 32Khz/64Kbps!", 0);
#endif

#ifdef AIR_BTA_IC_PREMIUM_G2
#ifndef AIR_WIRELESS_MIC_ENABLE
    ul_codec_param = &g_ull_le_mic_stream_param_samp_16k_br_64k_mono;
    ull_report("[ULL][LE] AB156X, UL codec: 16Khz/64Kbps!", 0);
#endif
#endif

    switch (index) {
#ifndef AIR_WIRELESS_MIC_ENABLE
        case 0: {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_172k_stero), ul_codec_param);
            ull_report("[ULL][LE] DL codec: 96Khz/172.8Kbps!", 0);
        }
        break;

        case 1: {
            /**< Default Codec Parameters. */
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_304k_stero), ul_codec_param);
            ull_report("[ULL][LE] DL codec: 96Khz/304Kbps!", 0);
        }
        break;
#endif
        case 2: {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), ul_codec_param);
            ull_report("[ULL][LE] DL codec: 48Khz/200Kbps!", 0);
        }
        break;

        default:
            ull_report("[ULL][LE] [ULL][LE] switch codec params index is %d, not support now", 1, index);
            break;
    }
}

static void bt_ull_le_srv_allocate_avm_block_addr(bt_ull_client_t client_type)
{
    /* ul: to controller, dl: from controller */
    if ((BT_ULL_HEADSET_CLIENT == client_type) \
        || (BT_ULL_EARBUDS_CLIENT == client_type) \
        || (BT_ULL_SPEAKER_CLIENT == client_type)) {
        g_ull_le_dl_ch1_addr = (uint32_t)bt_pka_get_leaudio_AVM_addr(BT_ULL_LE_SHARE_BUFFER_DL_CH1_SIZE + BT_ULL_LE_SHARE_BUFFER_DL_CH2_SIZE \
                                                                     + BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE + BT_ULL_LE_SHARE_BUFFER_UL_CH2_SIZE);
        g_ull_le_dl_ch2_addr = g_ull_le_dl_ch1_addr + BT_ULL_LE_SHARE_BUFFER_DL_CH1_SIZE;
        g_ull_le_ul_ch1_addr = g_ull_le_dl_ch2_addr + BT_ULL_LE_SHARE_BUFFER_DL_CH2_SIZE;
        g_ull_le_ul_ch2_addr = g_ull_le_ul_ch1_addr + BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE;

    }
#ifdef AIR_WIRELESS_MIC_ENABLE
    else if (BT_ULL_MIC_CLIENT == client_type) {
        g_ull_le_mic_ch1_addr = (uint32_t)bt_pka_get_leaudio_AVM_addr(BT_ULL_LE_SHARE_BUFFER_MIC_CH1_SIZE + BT_ULL_LE_SHARE_BUFFER_MIC_CH2_SIZE \
                                                            + BT_ULL_LE_SHARE_BUFFER_MIC_CH3_SIZE + BT_ULL_LE_SHARE_BUFFER_MIC_CH4_SIZE);
        g_ull_le_mic_ch2_addr = g_ull_le_mic_ch1_addr + BT_ULL_LE_SHARE_BUFFER_MIC_CH1_SIZE;
        g_ull_le_mic_ch3_addr = g_ull_le_mic_ch2_addr + BT_ULL_LE_SHARE_BUFFER_MIC_CH2_SIZE;
        g_ull_le_mic_ch4_addr = g_ull_le_mic_ch3_addr + BT_ULL_LE_SHARE_BUFFER_MIC_CH3_SIZE;
    }
#endif
    ull_report("[ULL][LE] bt_ull_le_srv_allocate_avm_block_addr, client_type is %d", 1, client_type);

}
static bt_status_t bt_ull_le_srv_init_n9_dsp_share_info(n9_dsp_share_info_t *share_info,
            uint32_t start_addr,
            uint16_t buf_size,
            uint32_t frame_size)
{
    if (NULL == share_info) {
        ull_report_error("[ULL][LE] N9 dsp share info is null!", 0);
        return BT_STATUS_FAIL;
    }
    share_info->start_addr = start_addr;
    share_info->length = buf_size;
    memset((void *)share_info->start_addr, 0, buf_size);
    share_info->read_offset = 0;
    share_info->write_offset = 0;
    share_info->sub_info.block_info.block_size = (frame_size + sizeof(bt_ull_le_share_buffer_header) + 3) / 4 * 4; //4B align
    share_info->sub_info.block_info.block_num = buf_size / (share_info->sub_info.block_info.block_size);
    share_info->anchor_clk = frame_size;
    ull_report("[ULL][LE] bt_ull_le_srv_init_n9_dsp_share_info, start addr: %x, buf size: %d, block size: %d, block num: %d, anchor clk: %d", 5, \
        start_addr, buf_size, share_info->sub_info.block_info.block_size, share_info->sub_info.block_info.block_num, share_info->anchor_clk);
    return BT_STATUS_SUCCESS;
}
static n9_dsp_share_info_t *bt_ull_le_srv_get_avm_buffer_info(bt_ull_le_share_info_t type, bt_ull_client_t client_type, uint32_t bit_rate, uint32_t frame_interval, uint32_t frame_size)
{
    n9_dsp_share_info_t *p_dsp_info = NULL;
    uint32_t payload_size = 0;

    bt_ull_le_codec_t codec = bt_ull_le_srv_get_client_preferred_codec_type();

    /* get codec frame size */
    payload_size = (bit_rate / 100) * (frame_interval / 10) / 8 / 1000;
    ull_report("[ULL][LE] bt_ull_le_srv_get_avm_buffer_info, type:%x bit_rate:%d frame_interval:%d frame_size:%d payload_size:%x, codec: %d", 6,
               type,
               bit_rate,
               frame_interval,
               frame_size,
               payload_size,
               codec);

    if ((0 == payload_size) || (payload_size != frame_size)) {
        ull_report_error("[ULL][LE] get_avm_buffer_info, error codec frame size:%d %d", 2, payload_size, frame_size);
        //configASSERT(0);
    }

    /* get share buffer info */
    if (BT_ULL_MIC_CLIENT == client_type) {
    #ifdef AIR_WIRELESS_MIC_ENABLE
    switch (type) {
            case BT_ULL_LE_SHARE_INFO_MIC_1: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRELESS_MIC_RX_BT_RECEIVE_FROM_AIR_0);
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_mic_ch1_addr, BT_ULL_LE_SHARE_BUFFER_MIC_CH1_SIZE, frame_size);
            break;
        }

        case BT_ULL_LE_SHARE_INFO_MIC_2: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRELESS_MIC_RX_BT_RECEIVE_FROM_AIR_1);
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_mic_ch2_addr, BT_ULL_LE_SHARE_BUFFER_MIC_CH2_SIZE, frame_size);
            break;
        }

        case BT_ULL_LE_SHARE_INFO_MIC_3: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRELESS_MIC_RX_BT_RECEIVE_FROM_AIR_2);
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_mic_ch3_addr, BT_ULL_LE_SHARE_BUFFER_MIC_CH3_SIZE, frame_size);
            break;
        }

        case BT_ULL_LE_SHARE_INFO_MIC_4: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRELESS_MIC_RX_BT_RECEIVE_FROM_AIR_3);
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_mic_ch4_addr, BT_ULL_LE_SHARE_BUFFER_MIC_CH4_SIZE, frame_size);
            break;
        }

        default:
            break;
        }
    #else
        ull_report_error("[ULL][LE] the option - AIR_WIRELESS_MIC_ENABLE is not open.", 0);
    #endif
    } else if (BT_ULL_HEADSET_CLIENT == client_type \
        || BT_ULL_EARBUDS_CLIENT == client_type \
        || BT_ULL_SPEAKER_CLIENT == client_type) {
        switch (type) {
        case BT_ULL_LE_SHARE_INFO_DL_CH_1: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_SEND_TO_AIR_0);
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
            if (BT_ULL_LE_CODEC_LC3PLUS == codec) {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_dl_ch1_addr, BT_ULL_LE_SHARE_BUFFER_DL_CH1_SIZE, BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_588 >> 1); /*max bitrate is 940.8k, max sdu size is 588, set max value for ABR feature*/
            } else {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_dl_ch1_addr, BT_ULL_LE_SHARE_BUFFER_DL_CH1_SIZE, (frame_size >> 1));
            }
#else
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_dl_ch1_addr, BT_ULL_LE_SHARE_BUFFER_DL_CH1_SIZE, (frame_size >> 1));
#endif
            break;
        }

        case BT_ULL_LE_SHARE_INFO_DL_CH_2: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_SEND_TO_AIR_1);
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
            if (BT_ULL_LE_CODEC_LC3PLUS == codec) {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_dl_ch2_addr, BT_ULL_LE_SHARE_BUFFER_DL_CH2_SIZE, BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_588 >> 1); /*max bitrate is 940.8k, max sdu size is 588, set max value for ABR feature*/
            } else {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_dl_ch2_addr, BT_ULL_LE_SHARE_BUFFER_DL_CH2_SIZE, (frame_size >> 1));
            }

#else
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_dl_ch2_addr, BT_ULL_LE_SHARE_BUFFER_DL_CH2_SIZE, (frame_size >> 1));
#endif
            break;
        }

        case BT_ULL_LE_SHARE_INFO_UL_CH_1: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_RECEIVE_FROM_AIR_0);
#ifdef AIR_ULL_ECNR_POST_PART_ENABLE
            /*for AB156x ULL 2.0  uplink AINR Feature.*/
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
            if (BT_ULL_LE_CODEC_LC3PLUS == codec) {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch1_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE, BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_65 + 1); /*max bitrate is 940.8k, max sdu size is 588, set max value for ABR feature*/
            } else {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch1_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE, frame_size + 1);
            }
#else
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch1_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE, frame_size + 1);
#endif
#else
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
            if (BT_ULL_LE_CODEC_LC3PLUS == codec) {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch1_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE, BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_65); /*max bitrate is 940.8k, max sdu size is 588, set max value for ABR feature*/
            } else {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch1_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE, frame_size);
            }
#else
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch1_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE, frame_size);
#endif
#endif
            break;
        }

        case BT_ULL_LE_SHARE_INFO_UL_CH_2: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_RECEIVE_FROM_AIR_1);
#ifdef AIR_ULL_ECNR_POST_PART_ENABLE
            /*for AB156x ULL 2.0  uplink AINR Feature.*/
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
            if (BT_ULL_LE_CODEC_LC3PLUS == codec) {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch2_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH2_SIZE, BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_65 + 1); /*max bitrate is 940.8k, max sdu size is 588, set max value for ABR feature*/
            } else {
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch2_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH2_SIZE, frame_size + 1);
            }
#else
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch2_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH2_SIZE, frame_size + 1);
#endif
#else
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
            if (BT_ULL_LE_CODEC_LC3PLUS == codec) {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch2_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH2_SIZE, BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_65); /*max bitrate is 940.8k, max sdu size is 588, set max value for ABR feature*/
            } else {
                bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch2_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH2_SIZE, frame_size);
            }
#else
            bt_ull_le_srv_init_n9_dsp_share_info(p_dsp_info, g_ull_le_ul_ch2_addr, BT_ULL_LE_SHARE_BUFFER_UL_CH2_SIZE, frame_size);
#endif
#endif
            break;
        }

        default:
            break;
    }
    }
    ull_report("[ULL][LE] bt_ull_le_srv_get_avm_buffer_info, type:%x info:%x", 2, type, p_dsp_info);
    return p_dsp_info;
}

void bt_ull_le_srv_deinit_share_info(void)
{
    uint8_t i;

    ull_report("[ULL][LE] bt_ull_le_srv_deinit_share_info", 0);
    for (i = 0; i < BT_ULL_LE_TRANSMITTER_SHARE_INFO_MAX; i ++) {
        g_ull_le_share_info[i] = NULL;
    }
    g_ull_le_dl_ch1_addr = 0;
    g_ull_le_dl_ch2_addr = 0;
    g_ull_le_ul_ch1_addr = 0;
    g_ull_le_ul_ch2_addr = 0;

#ifdef AIR_WIRELESS_MIC_ENABLE
    g_ull_le_mic_ch1_addr = 0;
    g_ull_le_mic_ch2_addr = 0;
    g_ull_le_mic_ch3_addr = 0;
    g_ull_le_mic_ch4_addr = 0;

#endif
}

bt_status_t bt_ull_le_srv_init_share_info(bt_ull_client_t client_type)
{
    uint8_t i = 0;
    bt_ull_le_srv_allocate_avm_block_addr(client_type);
    bt_ull_role_t role = bt_ull_le_srv_get_role();
    /* get share buffer info */
    if (BT_ULL_HEADSET_CLIENT == client_type \
        || BT_ULL_EARBUDS_CLIENT == client_type \
        || BT_ULL_SPEAKER_CLIENT == client_type) {
        for (i = 0; i < BT_ULL_LE_SHARE_INFO_MAX_NUM; i ++) {
            if ((i == BT_ULL_LE_SHARE_INFO_DL_CH_1) || (i == BT_ULL_LE_SHARE_INFO_DL_CH_2)) {
                /*slim code , Downlink buffer*/
                g_ull_le_share_info[i] = bt_ull_le_srv_get_avm_buffer_info(i,
                                                                  client_type,
                                                                  bt_ull_le_srv_get_bitrate(false, role),
                                                                  bt_ull_le_srv_get_sdu_interval(false, role),
                                                                  bt_ull_le_srv_get_sdu_size(false, role));
            } else {
                /*slim code , Uplink buffer*/
                g_ull_le_share_info[i] = bt_ull_le_srv_get_avm_buffer_info(i,
                                                                  client_type,
                                                                  bt_ull_le_srv_get_bitrate(true, role),
                                                                  bt_ull_le_srv_get_sdu_interval(true, role),
                                                                  bt_ull_le_srv_get_sdu_size(true, role));
            }
        }
        for (i = 0; i < BT_ULL_LE_TRANSMITTER_SHARE_INFO_MAX; i ++) {
            if (g_ull_le_share_info[i] == NULL) {
                ull_report_error("[ULL][LE] bt_ull_le_srv_init_share_info, share info is null! idx: %d", 1, i);
                return BT_STATUS_FAIL;
            }
        }
    }
#ifdef AIR_WIRELESS_MIC_ENABLE
    else if (BT_ULL_MIC_CLIENT == client_type) {
        for (i = 0; i < BT_ULL_LE_SHARE_INFO_MIC_MAX_NUM; i ++) {
            g_ull_le_share_info[i] = bt_ull_le_srv_get_avm_buffer_info(i,
                                                              client_type,
                                                              bt_ull_le_srv_get_bitrate(true, role),
                                                              bt_ull_le_srv_get_sdu_interval(true, role),
                                                              bt_ull_le_srv_get_sdu_size(true, role));

        }
        for (i = 0; i < BT_ULL_LE_TRANSMITTER_SHARE_INFO_MAX; i ++) {
            if (g_ull_le_share_info[i] == NULL) {
                ull_report_error("[ULL][LE] bt_ull_le_srv_init_share_info, Mic share info is null! idx: %d", 1, i);
                return BT_STATUS_FAIL;
            }
        }
    }
#endif
    else {
        return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_ull_le_srv_set_avm_share_buffer(bt_ull_role_t role, bt_ull_client_t client_type, uint8_t link_count)
{
    bt_avm_leaudio_buffer_info_t *share_buffer;
    uint8_t i, cis_count;
    bt_status_t status;
    if (BT_ULL_ROLE_SERVER == role) {
        cis_count = link_count;
    } else {
        cis_count = 0x02;
    }

    share_buffer = bt_ull_le_srv_memory_alloc(sizeof(bt_leaudio_buffer_set_t) * cis_count + sizeof(uint16_t));
    if (share_buffer == NULL) {
        ull_report_error("[ULL][LE] bt_ull_srv_set_avm_share_buffer, out of memory", 0);
        ull_assert(0);
        return BT_STATUS_OUT_OF_MEMORY;
    }

    if (BT_ULL_ROLE_SERVER == role) {
    share_buffer->count = cis_count;
    if (BT_ULL_HEADSET_CLIENT == client_type \
        || BT_ULL_EARBUDS_CLIENT == client_type \
        || BT_ULL_SPEAKER_CLIENT == client_type) {
        for (i = 0; i < cis_count; i ++) {//because of controller using UL as DL address
            share_buffer->buffer[i].ul_address = (uint32_t)g_ull_le_share_info[i * 2];
            share_buffer->buffer[i].dl_address = (uint32_t)g_ull_le_share_info[i * 2 + 1];
        }
    }
#ifdef AIR_WIRELESS_MIC_ENABLE
    else if (BT_ULL_MIC_CLIENT == client_type) {
        for (i = 0; i < cis_count; i ++) {
            share_buffer->buffer[i].dl_address = (uint32_t)g_ull_le_share_info[i];
            //share_buffer->buffer[i].ul_address = (uint32_t)g_ull_le_share_info[i];
        }
    }
#endif
    } else {
        /*reset avm buffer to zero*/
        hal_audio_set_le_audio_avm_buf_size(AUDIO_MESSAGE_TYPE_BLE_AUDIO_UL, 0);
        hal_audio_set_le_audio_avm_buf_size(AUDIO_MESSAGE_TYPE_BLE_AUDIO_DL, 0);
        hal_audio_set_le_audio_avm_buf_size(AUDIO_MESSAGE_TYPE_BLE_AUDIO_SUB_UL, 0);
        hal_audio_set_le_audio_avm_buf_size(AUDIO_MESSAGE_TYPE_BLE_AUDIO_SUB_DL, 0);

        uint32_t dl_bitrate = bt_ull_le_srv_get_bitrate(false, role);
        uint32_t ul_bitrate = bt_ull_le_srv_get_bitrate(true, role);
        uint32_t sdu_interval = bt_ull_le_srv_get_sdu_interval(false, role);
        uint32_t dl_buffer_size = 0x0;
        uint32_t ul_buffer_size = 0x0;
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
        dl_buffer_size = BT_ULL_LE_SHARE_BUFFER_DL_CH1_SIZE;
        ul_buffer_size = BT_ULL_LE_SHARE_BUFFER_UL_CH1_SIZE;
#else
        dl_buffer_size = ((((dl_bitrate / 100) * (sdu_interval / 10) / 8 / 1000) >> 1) + sizeof(bt_ull_le_share_buffer_header) + 3) / 4 * 4 * 7;
        ul_buffer_size = ((((ul_bitrate / 100) * (sdu_interval / 10) / 8 / 1000)) + sizeof(bt_ull_le_share_buffer_header) + 3) / 4 * 4 * 7;
#endif

        hal_audio_set_le_audio_avm_buf_size(AUDIO_MESSAGE_TYPE_BLE_AUDIO_DL,     dl_buffer_size);
        hal_audio_set_le_audio_avm_buf_size(AUDIO_MESSAGE_TYPE_BLE_AUDIO_SUB_DL, dl_buffer_size);
        hal_audio_set_le_audio_avm_buf_size(AUDIO_MESSAGE_TYPE_BLE_AUDIO_UL,     ul_buffer_size);
        //hal_audio_set_le_audio_avm_buf_size(AUDIO_MESSAGE_TYPE_BLE_AUDIO_SUB_UL, ul_buffer_size);
        share_buffer->count = cis_count;
        share_buffer->buffer[0].dl_address = (uint32_t)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_BLE_AUDIO_DL);
        share_buffer->buffer[0].ul_address = (uint32_t)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_BLE_AUDIO_UL);
        share_buffer->buffer[1].dl_address = (uint32_t)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_BLE_AUDIO_SUB_DL);
        share_buffer->buffer[1].ul_address = (uint32_t)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_BLE_AUDIO_SUB_UL);
        ull_report("[ULL][LE] bt_ull_srv_set_avm_share_buffer, Client! dl_br: %d, ul_br: %d, dl_buf_size: %d, ul_buf_size: %d, sdu_interval: %d", 5, \
            dl_bitrate, ul_bitrate, dl_buffer_size, ul_buffer_size, sdu_interval);
    }
    status = bt_avm_set_leaudio_buffer(share_buffer);
    bt_ull_le_srv_memory_free(share_buffer);
    ull_report("[ULL][LE] bt_ull_srv_set_avm_share_buffer, cis_count:%d, status: %x", 2, cis_count, status);
    return status;

}


uint32_t bt_ull_le_srv_get_avm_share_buffer_address(bt_ull_client_t client_type, bt_ull_le_srv_audio_out_t out_type, bt_ull_transmitter_t transmitter, uint8_t link_index)
{
    uint32_t share_avm_address = 0;

    if (BT_ULL_HEADSET_CLIENT == client_type \
        || BT_ULL_EARBUDS_CLIENT == client_type \
        || BT_ULL_SPEAKER_CLIENT == client_type) {
        switch (transmitter) {
            case BT_ULL_GAMING_TRANSMITTER:
            case BT_ULL_CHAT_TRANSMITTER:
            case BT_ULL_LINE_IN_TRANSMITTER:
            case BT_ULL_I2S_IN_TRANSMITTER: {
                share_avm_address = (uint32_t)g_ull_le_share_info[link_index * 2];
            }
            break;
            case BT_ULL_MIC_TRANSMITTER:
            case BT_ULL_I2S_OUT_TRANSMITTER:
            case BT_ULL_LINE_OUT_TRANSMITTER: {
                share_avm_address = (uint32_t)g_ull_le_share_info[link_index * 2 + 1];
            }
            break;
            default:
                share_avm_address = 0;
                break;
        }
    }
#ifdef AIR_WIRELESS_MIC_ENABLE
    else if (BT_ULL_MIC_CLIENT == client_type) {
        if (BT_ULL_MIC_TRANSMITTER == transmitter || BT_ULL_LINE_OUT_TRANSMITTER == transmitter || BT_ULL_I2S_OUT_TRANSMITTER == transmitter) {
            share_avm_address = (uint32_t)g_ull_le_share_info[link_index];
        } else {
            share_avm_address = 0;
        }
    }
#endif
    ull_report("[ULL][LE] bt_ull_le_srv_get_avm_share_buffer, share_avm_address: 0x%x", 1, share_avm_address);
    return share_avm_address;
}

static void bt_ull_le_srv_set_codec_param(bt_ull_role_t role, bt_ull_le_codec_param_t *dl_codec_param, bt_ull_le_codec_param_t *ul_codec_param)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_SERVER == role) {
        if (dl_codec_param) {
            memcpy(&(stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].codec_param), dl_codec_param, sizeof(bt_ull_le_codec_param_t));
            memcpy(&(stream_ctx->server.stream[BT_ULL_CHAT_TRANSMITTER].codec_param), dl_codec_param, sizeof(bt_ull_le_codec_param_t));
            memcpy(&(stream_ctx->server.stream[BT_ULL_LINE_IN_TRANSMITTER].codec_param), dl_codec_param, sizeof(bt_ull_le_codec_param_t));
            memcpy(&(stream_ctx->server.stream[BT_ULL_I2S_IN_TRANSMITTER].codec_param), dl_codec_param, sizeof(bt_ull_le_codec_param_t));
        }
        if (ul_codec_param) {
            memcpy(&(stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].codec_param), ul_codec_param, sizeof(bt_ull_le_codec_param_t));
            memcpy(&(stream_ctx->server.stream[BT_ULL_LINE_OUT_TRANSMITTER].codec_param), ul_codec_param, sizeof(bt_ull_le_codec_param_t));
            memcpy(&(stream_ctx->server.stream[BT_ULL_I2S_OUT_TRANSMITTER].codec_param), ul_codec_param, sizeof(bt_ull_le_codec_param_t));
        }
    } else if (BT_ULL_ROLE_CLIENT == role) {
        if (dl_codec_param) {
            memcpy(&(stream_ctx->client.dl.codec_param), dl_codec_param, sizeof(bt_ull_le_codec_param_t));
        }
        if (ul_codec_param) {
            memcpy(&(stream_ctx->client.ul.codec_param), ul_codec_param, sizeof(bt_ull_le_codec_param_t));
        }
    } else {
        ull_assert(0 && "role is invalid");
    }
}

#ifdef AIR_WIRELESS_MIC_ENABLE
void bt_ull_le_srv_set_wireless_mic_codec_param(bt_ull_role_t role)
{
    ull_report("[ULL][LE] bt_ull_le_srv_set_wireless_mic_codec_param, role: 0x%x, cs_size: %d, client_preferred_channel_mode: 0x%x", 3,
        role, g_ull_le_ctx.cs_size, g_ull_le_ctx.client_preferred_channel_mode);

        switch (g_ull_le_ctx.client_preferred_channel_mode) {
        case BT_ULL_LE_CHANNEL_MODE_MONO: {
            if (BT_ULL_LE_CODEC_ULD == bt_ull_le_srv_get_codec_type()) {
                bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_uld_codec_param_samp_48k_br_200k_mono));
                ull_report("[ULL][LE] chip: ab156x, wireless mic TX, UL default codec param(48KHZ/200.0Kbps)!", 0);
            } else {
                bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_48k_br_100k_mono));
                ull_report("[ULL][LE] chip: ab156x, wireless mic TX, UL default codec param(48KHZ/100.8Kbps/Mono)!", 0);
            }
        }
        break;

        default: {
            if (2 >= g_ull_le_ctx.cs_size) {
                bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_48k_br_200k_stero));
                    ull_report("[ULL][LE] chip: ab156x, wireless mic TX, UL default codec param(48KHZ/201.6Kbps/Stero)!", 0);
            } else {/* 3TX or 4TX, only support mono channel now. */
                bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_48k_br_100k_mono));
                ull_report("[ULL][LE] chip: ab156x, wireless mic TX, TX num is %d, UL default codec param(48KHZ/100.8Kbps/Mono)!", 1, g_ull_le_ctx.cs_size);
            }
        }
        break;
    }
}

#endif

void bt_ull_le_srv_set_client_preferred_codec_type(bt_ull_le_codec_t codec_type)
{
    ull_report("[ULL][LE] bt_ull_le_srv_set_client_preferred_codec_type, codec_type: %d", 1, codec_type);
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    g_ull_le_ctx.client_preferred_codec_type = codec_type;
#elif defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    g_bt_ull_hid_le_ctx.audio_sink.client_preferred_codec_type = codec_type;
#endif
}


bt_ull_le_codec_t bt_ull_le_srv_get_client_preferred_codec_type(void)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    return g_ull_le_ctx.client_preferred_codec_param.codec_type;
#elif defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    return g_bt_ull_hid_le_ctx.audio_sink.client_preferred_codec_type;
#endif
    return 0xFF;
}


void bt_ull_le_srv_set_opus_codec_param(void)
{
    /**< Default Codec is LC3plus. */
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    stream_ctx->codec_type = BT_ULL_LE_CODEC_OPUS;
#ifdef AIR_BTA_IC_PREMIUM_G2
    bt_ull_role_t role = bt_ull_le_srv_get_role();
    bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_320k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_64k_mono));
        ull_report("[ULL][LE] chip: ab156x(ULL 2.0), ull v2 Server, DL support opus codec, default codec param(48KHZ/320Kbps), UL default codec param(16KHZ/64Kbps)!", 0);
#else
    ull_report_error("[ULL][LE] invalid chip version, not support Opus codec", 0);
#endif
}

bt_ull_le_codec_t bt_ull_le_srv_get_client_preffered_codec(bt_ull_le_scenario_t mode_type)
{
   bt_ull_le_codec_t codec_type = BT_ULL_LE_CODEC_LC3PLUS;

   if (BT_ULL_LE_SCENARIO_ULLV3_0 == mode_type) {
        #if defined AIR_ULL_AUDIO_V3_ENABLE
           ull_report("[ULL][LE] client preffered codec type DL is ULD And UL is LC3PLUS! ", 0);
           codec_type =  BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS;
        #endif
   } else if (BT_ULL_LE_SCENARIO_ULLV2_0 == mode_type) {
        #if (defined AIR_WIRELESS_MIC_ENABLE) && (defined AIR_AUDIO_ULD_CODEC_ENABLE)
           ull_report("[ULL][LE] client preffered codec type is ULD!", 0);
           codec_type = BT_ULL_LE_CODEC_ULD;
        #elif defined AIR_AUDIO_LC3PLUS_CODEC_ENABLE
           ull_report("[ULL][LE] client preffered codec type is LC3PLUS!", 0);
           codec_type = BT_ULL_LE_CODEC_LC3PLUS;
        #elif defined AIR_AUDIO_VEND_CODEC_ENABLE
           ull_report("[ULL][LE] client preffered codec type is OPUS!", 0);
           codec_type = BT_ULL_LE_CODEC_OPUS;
        #else
           ull_report("[ULL][LE] client default preffered codec type is OPUS!", 0);
           codec_type = BT_ULL_LE_CODEC_OPUS;
        #endif
   }
   ull_report("[ULL][LE] client preffered codec type is %d!", 1, codec_type);
   return codec_type;
}

uint32_t bt_ull_le_srv_get_client_preffered_dl_codec_samplerate(bt_ull_le_scenario_t mode_type)
{
    uint32_t samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_96K; //default dl 96K

    if (BT_ULL_LE_SCENARIO_ULLV3_0 == mode_type) {
        #if defined AIR_ULL_AUDIO_V3_ENABLE
            samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_48K;
            ull_report("[ULL][LE] ULLV3 Codec support DL samplerate: %d!", 1, samplerate);
        #endif
    } else if (BT_ULL_LE_SCENARIO_ULLV2_0 == mode_type) {
        #ifdef AIR_DCHS_MODE_ENABLE
            samplerate = g_ull_le_dchs_dl_codec_samplerate;
            ull_report("[ULL][LE] DCHS support DL samplerate: %d!", 1, samplerate);
        #elif (defined AIR_WIRELESS_MIC_ENABLE) || ((defined AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) && (defined FIXED_SAMPLING_RATE_TO_48KHZ))
            samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_48K; //dual mode dl 48K
            ull_report("[ULL][LE] Dual Chip support DL samplerate: %d!", 1, samplerate);
        #elif defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
            samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_48K;
            ull_report("[ULL][LE] ULL HID DL saplerate: %d!", 1, samplerate);
        #elif defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) && defined (AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
            samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_96K;
            ull_report("[ULL][LE] Dual Chip support DL samplerate: %d!", 1, samplerate);
        #else
            #if defined(AIR_AUDIO_VEND_CODEC_ENABLE)
                samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_48K;
                ull_report("[ULL][LE] Opus Codec support DL samplerate: %d!", 1, samplerate);
            #elif defined (AIR_AUDIO_LC3PLUS_CODEC_ENABLE)
                samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_96K;
                ull_report("[ULL][LE] LC3+ Codec support DL samplerate: %d!", 1, samplerate);
            #endif
        #endif
    }
    ull_report("[ULL][LE] client preffered dl samplerate: %d!", 1, samplerate);
    return samplerate;
}

uint32_t bt_ull_le_srv_get_client_preffered_ul_codec_samplerate(bt_ull_le_scenario_t mode_type)
{
    uint32_t samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_16K;

    if(BT_ULL_LE_SCENARIO_ULLV3_0 == mode_type) {
        #if defined AIR_ULL_AUDIO_V3_ENABLE
            samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_32K;
            ull_report("[ULL][LE] ULLV3 Codec support ul samplerate: %d!", 1, samplerate);
        #endif
    } else if (BT_ULL_LE_SCENARIO_ULLV2_0 == mode_type) {
        #ifdef AIR_WIRELESS_MIC_ENABLE
            samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_48K;
            ull_report("[ULL][LE] Wireless mic ul samplerate: %d!", 1, samplerate);
        #elif defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
            samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_16K;
            ull_report("[ULL][LE] ULL2.0 HID ul samplerate: %d!", 1, samplerate);
        #elif defined (AIR_AUDIO_VEND_CODEC_ENABLE)
            samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_16K;
            ull_report("[ULL][LE] ULL2.0 Opus ul samplerate: %d!", 1, samplerate);
        #elif defined (AIR_AUDIO_LC3PLUS_CODEC_ENABLE)
            #if defined(AIR_BT_ULL_FB_ENABLE)
                samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_48K;
                ull_report("[ULL][LE] ULL2.0(FB) LC3+ ul samplerate: %d!", 1, samplerate);
            #elif defined (AIR_BT_ULL_SWB_ENABLE)
                samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_32K;
                ull_report("[ULL][LE] ULL2.0(SWB) LC3+ ul samplerate: %d!", 1, samplerate);
            #elif defined (AIR_BT_ULL_WB_ENABLE)
                samplerate = BT_ULL_LE_DEFAULT_SAMPLERTE_16K;
                ull_report("[ULL][LE] ULL2.0(WB) LC3+ ul samplerate: %d!", 1, samplerate);
            #endif
        #endif
    }
    ull_report("[ULL][LE] client preffered ul samplerate: %d!", 1, samplerate);
    return samplerate;
}

void bt_ull_le_srv_set_codec_param_by_sample_rate(bt_ull_role_t role, uint32_t dl_samplerate, uint32_t ul_samplerate)
{
    ull_report("[ULL][LE]set codec param, role: 0x%x, dl_samplerate: %d, ul_samplerate: %d", 3, role, dl_samplerate, ul_samplerate);
#ifdef AIR_BTA_IC_PREMIUM_G3
    if (BT_ULL_LE_CODEC_LC3PLUS == bt_ull_le_srv_get_codec_type()) {
    if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_32K == ul_samplerate) {
        bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_32k_br_64k_mono));
        ull_report("[ULL][LE] chip(DCHS): ab158x, DL default codec param(48KHZ/200Kbps), UL default codec param(32KHZ/64Kbps)!", 0);
    } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_32K == ul_samplerate) {
        bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_304k_stero), &(g_ull_le_mic_stream_param_samp_32k_br_64k_mono));
        ull_report("[ULL][LE] chip(ULL 2.0): ab158x, DL default codec param(96KHZ/304Kbps), UL default codec param(32KHZ/64Kbps)!", 0);
        } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_48K == ul_samplerate) {
                bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_272k_stero), &(g_ull_le_mic_stream_param_samp_48k_br_104k_mono));
                ull_report("[ULL][LE] chip: ab158x(ULL2.0), DL support lc3plus codec, default codec param(96KHZ/272Kbps), UL default codec param(48KHZ/104Kbps)!", 0);
        }
    }
#elif defined(AIR_BTA_IC_PREMIUM_G2)
#ifdef AIR_WIRELESS_MIC_ENABLE
    if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_48K == ul_samplerate) {
        if (BT_ULL_LE_CODEC_ULD == bt_ull_le_srv_get_codec_type()) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_uld_codec_param_samp_48k_br_200k_mono));
            ull_report("[ULL][LE] chip(WIRELESS MIC): ab156x, wireless mic, DL default ULD codec param(48KHZ/201.6Kbps), UL default codec param(48KHZ/200.0Kbps)!", 0);
        } else if (BT_ULL_LE_CODEC_LC3PLUS== bt_ull_le_srv_get_codec_type()) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_48k_br_200k_stero));
            ull_report("[ULL][LE] chip(WIRELESS MIC): ab156x, wireless mic, DL default lc3plus codec param(48KHZ/201.6Kbps), UL default codec param(48KHZ/201.6Kbps)!", 0);
        }
    }
#else
    if (BT_ULL_LE_CODEC_LC3PLUS == bt_ull_le_srv_get_codec_type()) {
        if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_304k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab156x(ULL 2.0), DL default lc3plus codec param(96KHZ/304Kbps), UL default codec param(16KHZ/64Kbps)!", 0);
        }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        else if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_32k_mono));
            ull_report("[ULL][LE] chip: ab156x (ULL2.0 HID), DL support lc3plus codec, default codec param(48KHZ/200Kbps), UL default codec param(16KHZ/32Kbps)!", 0);
        }
#endif
        else if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab156x Dual chip(ULL2.0), DL support lc3plus codec, default codec param(48KHZ/200Kbps), UL default codec param(16KHZ/64Kbps)!", 0);
        } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_32K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_32k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab156x Dual chip(ULL2.0), DL support lc3plus codec, default codec param(48KHZ/200Kbps), UL default codec param(32KHZ/64Kbps)!", 0);
        } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_32K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_304k_stero), &(g_ull_le_mic_stream_param_samp_32k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab156x Dongle + ab158x Headset, DL support lc3plus codec, default codec param(96KHZ/304Kbps), UL default codec param(32KHZ/64Kbps)!", 0);
        } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_48K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_272k_stero), &(g_ull_le_mic_stream_param_samp_48k_br_104k_mono));
            ull_report("[ULL][LE] chip: ab156x(ULL2.0), DL support lc3plus codec, default codec param(96KHZ/272Kbps), UL default codec param(48KHZ/104Kbps)!", 0);
        }
    } else if (BT_ULL_LE_CODEC_OPUS == bt_ull_le_srv_get_codec_type()) {
        if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_320k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab156x(ULL 2.0), DL support opus codec, default codec param(48KHZ/320Kbps), UL default codec param(16KHZ/64Kbps)!", 0);
        }
    }
#endif
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
    if (BT_ULL_LE_CODEC_LC3PLUS == bt_ull_le_srv_get_codec_type()) {
        if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_304k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab157x(ULL 2.0), DL default lc3plus codec param(96KHZ/304Kbps), UL default codec param(16KHZ/64Kbps)!", 0);
        }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        else if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == ul_samplerate) {
            //bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_32k_mono));
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_160k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_32k_mono));
            ull_report("[ULL][LE] chip: ab157x (ULL2.0 HID), DL support lc3plus codec, default codec param(48KHZ/160Kbps), UL default codec param(16KHZ/32Kbps)!", 0);
        } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_304k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab157x (ULL2.0 HID), DL support lc3plus codec, default codec param(96KHZ/304Kbps), UL default codec param(16KHZ/64Kbps)!", 0);
        }
#endif
        else if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab157x Dual chip(ULL2.0), DL support lc3plus codec, default codec param(48KHZ/200Kbps), UL default codec param(16KHZ/64Kbps)!", 0);
        } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_32K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_200k_stero), &(g_ull_le_mic_stream_param_samp_32k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab157x Dual chip(ULL2.0), DL support lc3plus codec, default codec param(48KHZ/200Kbps), UL default codec param(32KHZ/64Kbps)!", 0);
        } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_32K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_304k_stero), &(g_ull_le_mic_stream_param_samp_32k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab157x(ULL2.0), DL support lc3plus codec, default codec param(96KHZ/304Kbps), UL default codec param(32KHZ/64Kbps)!", 0);
        } else if (BT_ULL_LE_DEFAULT_SAMPLERTE_96K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_48K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_96k_br_272k_stero), &(g_ull_le_mic_stream_param_samp_48k_br_104k_mono));
            ull_report("[ULL][LE] chip: ab157x(ULL2.0), DL support lc3plus codec, default codec param(96KHZ/272Kbps), UL default codec param(48KHZ/104Kbps)!", 0);
        }
    } else if (BT_ULL_LE_CODEC_OPUS == bt_ull_le_srv_get_codec_type()) {
        if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_16K == ul_samplerate) {
            bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_param_samp_48k_br_320k_stero), &(g_ull_le_mic_stream_param_samp_16k_br_64k_mono));
            ull_report("[ULL][LE] chip: ab157x(ULL 2.0), DL support opus codec, default codec param(48KHZ/320Kbps), UL default codec param(16KHZ/64Kbps)!", 0);
        }
    } else if (BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS == bt_ull_le_srv_get_codec_type()) {
      if (BT_ULL_LE_DEFAULT_SAMPLERTE_48K == dl_samplerate && BT_ULL_LE_DEFAULT_SAMPLERTE_32K == ul_samplerate) {
         bt_ull_le_srv_set_codec_param(role, &(g_ull_le_spk_stream_codec_uld_samp_48k_br_400k_stero), &(g_ull_le_mic_stream_codec_uld_samp_32k_br_64k_mono));
         ull_report("[ULL][LE] chip: ab157x(ULL3.0), DL support uld codec,UL support lc3plus codec, default codec param(48KHZ/400Kbps), UL default codec param(32KHZ/64kbps)!", 0);
      }
    } else {
         ull_report("[ULL][LE] chip: ab157x(ULL), Set unsupport codec param!", 0);
    }
#else
    ull_report_error("[ULL][LE] bt_ull_le_srv_reset_stream_ctx, invalid chip version", 0);
#endif
}

bt_ull_le_srv_client_preferred_codec_param* bt_ull_le_srv_get_client_preferred_codec_param(void)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    return &(g_ull_le_ctx.client_preferred_codec_param);
#elif defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    return &g_bt_ull_hid_le_ctx.audio_sink.client_preferred_codec_param;
#else
    return NULL;
#endif
}

#ifdef AIR_DCHS_MODE_ENABLE
void bt_ull_le_srv_set_client_preffered_dl_codec_samplerate(uint32_t samplerate)
{
    ull_report("[ULL][LE] bt_ull_le_srv_set_client_preffered_dl_codec_samplerate: %d!", 1, samplerate);
    g_ull_le_dchs_dl_codec_samplerate = samplerate;
}
#endif

bt_status_t bt_ull_le_srv_set_access_address(bt_ull_le_set_adv_scan_access_addr_t *access_addr)
{
    return bt_ull_le_set_access_address(access_addr);
}


bt_ull_transmitter_t bt_ull_le_srv_get_transmitter_by_stream_interface(bt_ull_streaming_t *streaming)
{
    uint8_t i;
    for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
        if ((g_ull_le_port_trans_mapping_table[i].interface == streaming->streaming_interface) &&
            (g_ull_le_port_trans_mapping_table[i].port_num == streaming->port)) {
            return g_ull_le_port_trans_mapping_table[i].transmitter;
        }
    }
    ull_report_error("[ULL][LE] bt_ull_le_srv_get_transmitter_by_stream_interface error, type: 0x%x, port:0x%x", 2,
        streaming->streaming_interface, streaming->port);
    return 0xFF;
}

bt_status_t bt_ull_le_srv_get_stream_by_transmitter(bt_ull_transmitter_t transmitter, bt_ull_streaming_t *streaming)
{
    bt_ull_streaming_t *stream = streaming;

    if (transmitter < BT_ULL_TRANSMITTER_MAX_NUM) {
        stream->port = g_ull_le_port_trans_mapping_table[transmitter].port_num;
        stream->streaming_interface = g_ull_le_port_trans_mapping_table[transmitter].interface;
        return BT_STATUS_SUCCESS;
    }
    ull_report_error("[ULL][LE] bt_ull_le_srv_get_stream_by_transmitter error, transmitter: 0x%x", 1, transmitter);
    return BT_STATUS_FAIL;
}

bt_ull_le_stream_port_mask_t bt_ull_le_srv_get_stream_port_by_transmitter(bt_ull_transmitter_t transmitter)
{
    if (transmitter < BT_ULL_TRANSMITTER_MAX_NUM) {
        return g_ull_le_port_trans_mapping_table[transmitter].streaming_port;
    }
    ull_report_error("[ULL][LE] bt_ull_le_srv_get_stream_port_by_transmitter error, transmitter: 0x%x", 1, transmitter);
    return BT_ULL_LE_STREAM_PORT_MASK_NONE;
}

bt_ull_transmitter_t bt_ull_le_srv_get_transmitter_by_stream_port(bt_ull_le_stream_port_mask_t stream_port)
{
    uint8_t i;
    for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
        if (g_ull_le_port_trans_mapping_table[i].streaming_port == stream_port) {
            return g_ull_le_port_trans_mapping_table[i].transmitter;
        }
    }
    ull_report_error("[ULL][LE] bt_ull_le_srv_get_stream_port_by_transmitter error, stream_port: 0x%x", 1, stream_port);
    return 0xFF;
}

bt_ull_le_srv_audio_out_t bt_ull_le_srv_get_audio_out_by_stream_port(bt_ull_le_stream_port_mask_t stream_port)
{
    uint8_t i;
    for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
        if (g_ull_le_port_trans_mapping_table[i].streaming_port == stream_port) {
            return g_ull_le_port_trans_mapping_table[i].audio_out;
        }
    }
    ull_report_error("[ULL][LE] bt_ull_le_srv_get_audio_out_type_by_stream_port error, stream_port: 0x%x", 1, stream_port);
    return BT_ULL_LE_SRV_AUDIO_OUT_USB;
}

bt_ull_le_stream_mode_t bt_ull_le_srv_get_stream_mode(bt_ull_transmitter_t transmitter)
{
    if (transmitter < BT_ULL_TRANSMITTER_MAX_NUM) {
        return g_ull_le_port_trans_mapping_table[transmitter].stream_mode;
    }
    ull_report_error("[ULL][LE] bt_ull_le_srv_get_stream_mode fail, transmitter: 0x%x", 1, transmitter);
    return BT_ULL_LE_STREAM_MODE_DOWNLINK;
}

bt_ull_le_srv_stream_context_t *bt_ull_le_srv_get_stream_context(void)
{
    return &g_ull_le_stream_ctx;
}

bt_ull_le_codec_param_t *bt_ull_le_srv_get_codec_param(bt_ull_role_t role, bt_ull_transmitter_t transmitter)
{
    bt_ull_le_codec_param_t *codec_info = NULL;
    bt_ull_le_srv_server_stream_t *stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(role, transmitter);

    if (stream_info) {
        codec_info = &(stream_info->codec_param);
    } else {
        ull_report_error("[ULL][LE] bt_ull_le_srv_get_codec_param, Role %d, trans type %d, Not suppport this transmitter", 2, role, transmitter);
    }
    return codec_info;
}

void *bt_ull_le_srv_get_stream_info(bt_ull_role_t role, bt_ull_transmitter_t transmitter)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_CLIENT == role) {
        switch (transmitter) {
            case BT_ULL_GAMING_TRANSMITTER:
            case BT_ULL_CHAT_TRANSMITTER:
            case BT_ULL_LINE_IN_TRANSMITTER:
            case BT_ULL_I2S_IN_TRANSMITTER: {
                return (void *)&(stream_ctx->client.dl);
            } break;

            case BT_ULL_MIC_TRANSMITTER:
            case BT_ULL_LINE_OUT_TRANSMITTER:
            case BT_ULL_I2S_OUT_TRANSMITTER: {
                return (void *)&(stream_ctx->client.ul);
            } break;

            default:
                ull_report_error("[ULL][LE] bt_ull_le_srv_get_stream_info, Role %d, trans type %d, Not suppport this transmitter", 2, role, transmitter);
                return NULL;
        }
    } else if (BT_ULL_ROLE_SERVER == role){
        if ((BT_ULL_GAMING_TRANSMITTER <= transmitter) && (BT_ULL_TRANSMITTER_MAX_NUM > transmitter)) {
            return (void *)&(stream_ctx->server.stream[transmitter]);
        } else {
            ull_report_error("[ULL][LE] bt_ull_le_srv_get_stream_info, Role %d, trans type %d, Not suppport this transmitter", 2, role, transmitter);
                return NULL;
        }
    } else {
        ull_report_error("[ULL][LE] bt_ull_le_srv_get_stream_info, Role is not support", 0);
        return NULL;
        }
}

bool bt_ull_le_srv_is_streaming(bt_ull_le_stream_mode_t stream_mode)
{
    bool is_streaming = false;
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

        switch (stream_mode) {
            case BT_ULL_LE_STREAM_MODE_DOWNLINK: {
                if (0 == (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) &&
                   (0 == (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT)) &&
                   (0 == (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_IN)) &&
                   (0 == (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_IN))) {
                    is_streaming = false;
                } else {
                    is_streaming = true;
        }
            } break;

            case BT_ULL_LE_STREAM_MODE_UPLINK: {
                if (0 == (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) &&
                   (0 == (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT)) &&
                   (0 == (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT))) {
                    is_streaming = false;
    } else {
                    is_streaming = true;
    }
            } break;

            default:
                break;
    }
    ull_report("[ULL][LE] bt_ull_le_srv_is_streaming, stream_mode(%d) is streaming(%d)", 2, stream_mode, is_streaming);
    return is_streaming;
}

void bt_ull_le_srv_set_transmitter_is_start(bt_ull_transmitter_t transmitter_type, bool is_start)
{
    bt_ull_le_srv_server_stream_t *stream_info;
    stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, transmitter_type);
    if (NULL == stream_info) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_set_transmitter_is_start stream info is NULL", 0);
        return;
    }
    stream_info->is_transmitter_start = is_start;
    ull_report("[ULL][LE] Set AT Start: %d", 1, is_start);
}

bool bt_ull_le_srv_is_any_streaming_started(bt_ull_role_t role)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    if (BT_ULL_ROLE_SERVER == role) {
        uint8_t i;
        for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
            if (stream_ctx->server.stream[i].is_transmitter_start) {
                ull_report("[ULL][LE] bt_ull_le_srv_is_any_streaming_started, Yes! Server role, trans_type is %d.", 1, i);
                return true;
            }
        }
    } else if ((BT_ULL_ROLE_CLIENT == role) && ((stream_ctx->client.dl.is_am_open) ||
        (stream_ctx->client.ul.is_am_open))) {
        ull_report("[ULL][LE] bt_ull_le_srv_is_any_streaming_started, Yes! Client role.", 0);
        return true;
    }

    ull_report_error("[ULL][LE] bt_ull_le_srv_is_any_streaming_started, No!", 0);
    return false;
}




void bt_ull_le_srv_notify_server_play_is_allow(bt_ull_allow_play_t is_allow, uint8_t reason)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    extern void bt_ull_le_srv_indicate_server_play_is_allow(bt_ull_allow_play_t is_allow, uint8_t reason);
    bt_ull_le_srv_indicate_server_play_is_allow(is_allow, reason);
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    extern void bt_ull_le_hid_srv_indicate_server_play_is_allow(bt_ull_allow_play_t is_allow, uint8_t reason);
    bt_ull_le_hid_srv_indicate_server_play_is_allow(is_allow, reason);
#endif
}




bt_status_t bt_ull_le_srv_deactive_streaming(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    extern bt_status_t bt_ull_le_srv_disable_streaming(void);
    status = bt_ull_le_srv_disable_streaming();
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    extern bt_status_t bt_ull_le_hid_srv_disable_streaming(void);
    status = bt_ull_le_hid_srv_disable_streaming();
#endif
    return status;
}




bt_status_t bt_ull_le_srv_active_streaming(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    extern bt_status_t bt_ull_le_srv_enable_streaming(void);
    status = bt_ull_le_srv_enable_streaming();
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    extern bt_status_t bt_ull_le_hid_srv_enable_streaming(void);
    status = bt_ull_le_hid_srv_enable_streaming();
#endif
    return status;
}


bool bt_ull_le_service_is_connected(void)
{
    bool is_connected = false;
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    extern bool bt_ull_le_srv_is_connected(void);

    is_connected = bt_ull_le_srv_is_connected();
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    extern bool bt_ull_le_hid_srv_is_connected(void);

    is_connected = bt_ull_le_hid_srv_is_connected();
#endif
    return is_connected;
}

void bt_ull_le_srv_silence_detection_notify_client_status(bool is_silence, bt_ull_transmitter_t transmitter_type)
{
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    extern void bt_ull_le_srv_silence_detection_notify_client(bool is_silence, bt_ull_transmitter_t transmitter_type);

    bt_ull_le_srv_silence_detection_notify_client(is_silence, transmitter_type);
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    extern void bt_ull_le_hid_srv_silence_detection_notify_client(bool is_silence, bt_ull_transmitter_t transmitter_type);

    bt_ull_le_hid_srv_silence_detection_notify_client(is_silence, transmitter_type);
#endif

}

#endif
