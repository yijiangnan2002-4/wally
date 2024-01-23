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

#ifndef __HAL_AUDIO_MESSAGE_STRUCT_COMMON_H__
#define __HAL_AUDIO_MESSAGE_STRUCT_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

/* Power off dac after delay <AUDIO_AMP_DELAY_OFF_TIMER_DURATION_MS> */
#define LLF_DATA_TYPE_NUMBER                    (7)
#define LLF_DATA_TYPE_MIC_NUMBER                (5)
#define LLF_ECHO_REF_NUMBER                     (2)


#ifdef MTK_AMP_DC_COMPENSATION_ENABLE
#define AUDIO_AMP_DELAY_OFF_TIMER_DURATION_MS    (2000) // delay 2s
#else
#define AUDIO_AMP_DELAY_OFF_TIMER_DURATION_MS    (0)    // no delay
#endif

#define AVM_SHAEE_BUF_INFO

typedef enum {
    AUDIO_DSP_CODEC_TYPE_CVSD = 0,
    AUDIO_DSP_CODEC_TYPE_MSBC,
    AUDIO_DSP_CODEC_TYPE_DNN, // for DNN loopback
    AUDIO_DSP_CODEC_TYPE_LC3,
    AUDIO_DSP_CODEC_TYPE_LC3PLUS,
    AUDIO_DSP_CODEC_TYPE_ULD,

    AUDIO_DSP_CODEC_TYPE_PCM = 0x100,
    AUDIO_DSP_CODEC_TYPE_SBC,
    AUDIO_DSP_CODEC_TYPE_MP3,
    AUDIO_DSP_CODEC_TYPE_AAC,
    AUDIO_DSP_CODEC_TYPE_VENDOR,
    AUDIO_DSP_CODEC_TYPE_OPUS,
    AUDIO_DSP_CODEC_TYPE_ANC_LC, //for leakage compensation
#ifndef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
    AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF,
#else
    AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_SZ,
    AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_PZ,
    AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_PZ_FIR,
#endif
    AUDIO_DSP_CODEC_TYPE_PCM_WWE,
    AUDIO_DSP_CODEC_TYPE_FADP_ANC_COMP,
    AUDIO_DSP_CODEC_TYPE_MAX,
    AUDIO_DSP_CODEC_TYPE_DUMMY = 0x7FFFFFFF,  /**<  Dummy for DSP structure alignment */
} audio_dsp_codec_type_t;

typedef enum {
    HAL_AUDIO_PCM_FORMAT_S8 = 0,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_U8,                           /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_S16_LE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_S16_BE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_U16_LE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_U16_BE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_S24_LE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_S24_BE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_U24_LE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_U24_BE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_S32_LE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_S32_BE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_U32_LE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_U32_BE,                       /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_LAST,                         /**< PCM format setting  */
    HAL_AUDIO_PCM_FORMAT_DUMMY  = 0xFFFFFFFF,          /**< For dsp structure alignment */
} hal_audio_format_t;
typedef hal_audio_format_t afe_pcm_format_t;

//-------------------------------------------------------------------------------------------------
// BOTH AUDIO & CONTROLLER WILL USE THIS STRUCT, BE CAREFUL !!! Should be align with 6x/7x/8x
typedef struct {
    uint32_t StartAddr;       // start address of share buffer
    uint16_t ReadIndex;  // read pointer of share buffer  : DSP monitor
    uint16_t WriteIndex; // write pointer of share buffer : Controller monitor
    uint32_t SampleRate; // sample rate for clock skew counting
    uint16_t MemBlkSize; // block length for each frame
    uint16_t MemBlkNum;  // number of block for frame usage
    uint32_t DbgInfoAddr; // start address of controller packet address table
    uint16_t FrameSampleNum;  // DSP notify audio
    uint16_t codec_type;      // Codec information
    uint16_t codec_config;    // Codec config information
    uint16_t NotifyCount;  // notify count of DSP notify controller not to sleep
    uint32_t ForwarderAddr; // forwarder buffer address
    uint32_t SinkLatency; // a2dp sink latency
    uint8_t role;//partner or agent
    char    rssi;//Received Signal Strength Indication
    uint8_t local_asi_en;//enable local asi or not
    uint8_t reserved; // reserved
} avm_share_buf_info_t, *avm_share_buf_info_ptr;
//-------------------------------------------------------------------------------------------------

// N9-DSP share information buffer (32 bytes)
typedef union {
    uint32_t next;
    struct {
        uint16_t block_size;
        uint16_t block_num;
    } block_info;
} sub_info_t;

//-------------------------------------------------------------------------------------------------
// BOTH AUDIO & CONTROLLER WILL USE THIS STRUCT, BE CAREFUL !!! Should be align with 6x/7x/8x
typedef struct {
    uint32_t start_addr;       // start address of N9-DSP share buffer
    uint32_t read_offset;      // read pointer of N9-DSP share buffer
    uint32_t write_offset;     // write pointer of N9-DSP share buffer
    sub_info_t sub_info;       // next read position in buf for DSP
    uint32_t sampling_rate;    // for AWS clock skew
    uint32_t length;           // total length of N9-DSP share buffer
    uint8_t  bBufferIsFull;    // buffer full flag, when N9 find there is no free buffer for putting a packet,
                               // set this flag = 1, DSP will reset this flag when data be taken by DSP
    uint8_t  notify_count;     // notify count
    int32_t  drift_comp_val;   // long term drift compensation value
    uint32_t anchor_clk;       // long term drift anchor clk
    uint32_t asi_base;         // As coming packet size in BLE/ULL UL
    uint32_t asi_cur;          // As CRC init in BLE DL , and apply coming packet size time in BLE/ULL UL
} n9_dsp_share_info_t, *n9_dsp_share_info_ptr;
//-------------------------------------------------------------------------------------------------

typedef enum {
    CPD_HSE_MODE_NORMAL = 0,
    CPD_HSE_MODE_1,                               /*G616*/
    CPD_HSE_MODE_2,                               /*N@W*/
    CPD_HSE_MODE_DUMMY            = 0x7FFFFFFF,   /**<  Dummy for DSP structure alignment */
} voice_cpd_hse_mode_t;

typedef struct {
    uint16_t                         cpd_nv_length;
    voice_cpd_hse_mode_t             cpd_hse_mode;
} audio_cpd_param_t, *audio_cpd_param_p;

/* For HFP open parameter */
typedef uint8_t audio_dsp_hfp_codec_param_t;

typedef struct {
    audio_dsp_hfp_codec_param_t     codec_type;
    avm_share_buf_info_t            *p_share_info;
    uint32_t                        bt_inf_address;
    uint32_t                        *clk_info_address;
    uint32_t                        *p_air_dump_buf;
    uint32_t                        *p_rx_audio_forwarder_buf;
    uint32_t                        *p_tx_audio_forwarder_buf;
    audio_cpd_param_t               cpd_param;
} audio_dsp_hfp_open_param_t, *audio_dsp_hfp_open_param_p;

#if 0
typedef U8 bt_hfp_codec_type_t;    /**< The type of HFP codec  */ /**< 1: CVSD, 2: mSBC */

typedef struct {
    bt_hfp_codec_type_t   codec_type;         /**< Codec type. */
    avm_share_buf_info_ptr p_share_info;                       /**< Codec information. */
    uint32_t              bt_inf_address;
    uint32_t              clk_info_address;
    uint32_t              p_air_dump_buf;
    uint32_t               p_rx_audio_forwarder_buf;
    uint32_t               p_tx_audio_forwarder_buf;
} bt_hfp_open_param_t, *bt_hfp_open_param_p;
#endif

/* For BLE open parameter */
typedef uint8_t audio_dsp_ble_codec_param_t;

typedef struct {
    audio_dsp_ble_codec_param_t     codec_type;
    uint8_t  channel_num;
    uint8_t  channel_mode;
    uint16_t frame_payload_length;
    uint32_t  frame_ms;
    uint32_t sampling_frequency;
    uint32_t context_type;
    uint8_t  dual_cis_mode;
    uint8_t  decode_mode;
    n9_dsp_share_info_t             *p_share_info;
    n9_dsp_share_info_t             *p_sub_share_info;
    uint32_t                        *p_air_dump_buf;
    audio_cpd_param_t               cpd_param;
    bool     nr_offload_flag;
    bool     is_lightmode;
} audio_dsp_ble_open_param_t, *audio_dsp_ble_open_param_p;

//-------------------------------------------------------------------------------------------------
// BOTH AUDIO & CONTROLLER WILL USE THIS STRUCT, BE CAREFUL !!! Should be align with 6x/7x/8x
typedef struct {
    uint32_t iso_interval; /* Unit with BT clock (312.5us) */
    uint32_t dl_timestamp_clk; /* Unit with BT clock (312.5us), indicate the first anchor of DL */
    uint32_t dl_retransmission_window_clk; /* Unit with BT clock (312.5us), valid bit[27:2] */
    uint16_t dl_timestamp_phase; /* Unit with 0.5us, valid value: 0~2499 */
    uint16_t dl_retransmission_window_phase; /* Unit with 0.5us, valid value: 0~2499 */
    uint8_t  dl_ft;
    uint8_t  dl_packet_counter; /* ISO DL packet counter & 0xFF */
    uint8_t  ul_ft;
    uint8_t  ul_packet_counter; /* ISO UL packet counter & 0xFF */
    uint32_t ul_timestamp; /* Unit with BT clock (312.5us), indicate the first anchor of UL */
    uint32_t ISOAnchorClk;
    uint16_t ISOAnchorPhase;
    uint32_t ul_avm_info_addr;
    uint32_t dl_avm_info_addr;
} ble_init_play_info_t;
//-------------------------------------------------------------------------------------------------


/*************************a2dp param************************/
typedef struct {
    uint8_t  aws_role;
    uint8_t  info_status;
    uint16_t cur_seqn;
    uint32_t cur_asi;
} AUDIO_SYNC_INFO;

typedef struct {
    uint8_t param[20];//TEMP!! align bt_codec_a2dp_audio_t
} audio_dsp_a2DP_codec_param_t;

typedef struct
{
    uint8_t enable;
} dsp_audio_plc_ctrl_t, *dsp_audio_plc_ctrl_p;

/* For A2DP open parameter */
typedef struct {
    audio_dsp_a2DP_codec_param_t    codec_info;
    #ifdef AVM_SHAEE_BUF_INFO
    avm_share_buf_info_t            *p_share_info;
    #else
    n9_dsp_share_info_t             *p_share_info;
    #endif
    uint32_t                        *p_asi_buf;
    uint32_t                        *p_min_gap_buf;
    uint32_t                        *p_current_bit_rate;
    uint32_t                        sink_latency;
    uint32_t                        bt_inf_address;
    uint32_t                        *p_afe_buf_report;
    uint32_t                        *p_lostnum_report;
    AUDIO_SYNC_INFO                 *p_audio_sync_info;
    uint32_t                        *p_pcdc_anchor_info_buf;
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    uint32_t                        *ull_clk_info_address;
#endif
#ifdef MTK_AUDIO_PLC_ENABLE
    dsp_audio_plc_ctrl_t            audio_plc;
#endif
} audio_dsp_a2dp_dl_open_param_t, *audio_dsp_a2dp_dl_open_param_p;

/* For A2DP start parameter */
typedef struct {
    uint32_t                    start_time_stamp;
    uint32_t                    time_stamp_ratio;
    uint32_t                    start_asi;
    uint32_t                    start_bt_clk;
    uint32_t                    start_bt_intra_clk;
    bool                        content_protection_exist;
    bool                        alc_monitor;
    bool                        latency_monitor_enable;
} audio_dsp_a2dp_dl_start_param_t, *audio_dsp_a2dp_dl_start_param_p;

/*************************record param************************/
typedef enum
{
    ENCODER_BITRATE_16KBPS  = 16,
    ENCODER_BITRATE_32KBPS  = 32,
    ENCODER_BITRATE_64KBPS  = 64,
    ENCODER_BITRATE_MAX     = 0xFFFFFFFF,
} encoder_bitrate_t;
typedef encoder_bitrate_t DSP_ENC_BITRATE_t;

typedef struct {
    n9_dsp_share_info_t *p_share_info;
    uint32_t frames_per_message;
    encoder_bitrate_t bitrate;
    bool interleave;
} cm4_record_open_param_t, *cm4_record_open_param_p;

/*************************transmitter param************************/
typedef struct {
    uint32_t sample_rate;
    uint32_t channel_mode;
    hal_audio_format_t format;
    uint32_t frame_interval;    /**< uint:us. */
} audio_codec_pcm_t;

typedef struct {
    uint8_t min_bit_pool;       /**< The minimum bit pool. */
    uint8_t max_bit_pool;       /**< The maximum bit pool. */
    uint8_t block_length;       /**< b0: 4, b1: 8, b2: 12, b3: 16. */
    uint8_t subband_num;        /**< b0: 4, b1: 8. */
    uint8_t alloc_method;       /**< b0: loudness, b1: SNR. */
    uint8_t sample_rate;        /**< b0: 16000, b1: 32000, b2: 44100, b3: 48000. */
    uint8_t channel_mode;       /**< b0: mono, b1: dual channel, b2: stereo, b3: joint stereo. */
} audio_codec_sbc_enc_t;

typedef struct {
    uint8_t min_bit_pool;       /**< The minimum bit pool. */
    uint8_t max_bit_pool;       /**< The maximum bit pool. */
    uint8_t block_length;       /**< b0: 16, b1: 12, b2: 8, b3: 4. */
    uint8_t subband_num;        /**< b0: 8, b1: 4. */
    uint8_t alloc_method;       /**< b0: loudness, b1: SNR. */
    uint8_t sample_rate;        /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    uint8_t channel_mode;       /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
} audio_codec_sbc_t;

typedef struct {
    uint32_t sample_rate;        /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    hal_audio_format_t sample_format;
    uint32_t bit_rate;           /**< b0: 16kbps, b1:32kbps, b2:64kbps. */
    uint32_t channel_mode;       /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
    uint32_t frame_interval;     /**< uint:us. */
    uint32_t frame_size;         /**< uint:Byte. */
    uint32_t version;            /**< codec version info */
} audio_codec_opus_t;

typedef struct {
    uint32_t sample_rate;        /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    uint32_t bit_rate;           /**< uint:bps. */
    uint32_t channel_mode;       /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
    uint32_t frame_interval;     /**< uint:us. */
    uint32_t frame_size;         /**< uint:Byte. */
} audio_codec_lc3_t;

typedef struct {
    uint32_t sample_rate;        /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    hal_audio_format_t sample_format;
    uint32_t bit_rate;           /**< uint:bps. */
    uint32_t channel_mode;
    uint32_t frame_interval;     /**< uint:us. */
    uint32_t frame_size;         /**< uint:Byte. */
} audio_codec_lc3plus_t;

typedef struct {
    uint32_t sample_rate;        /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    hal_audio_format_t sample_format;
    uint32_t bit_rate;           /**< uint:bps. */
    uint32_t channel_mode;
    uint32_t frame_interval;     /**< uint:us. */
    uint32_t frame_size;         /**< uint:Byte. */
} audio_codec_uld_t;

typedef struct {
    uint8_t min_bit_pool;       /**< The minimum bit pool. */
    uint8_t max_bit_pool;       /**< The maximum bit pool. */
    uint8_t block_length;       /**< b0: 16, b1: 12, b2: 8, b3: 4. */
    uint8_t subband_num;        /**< b0: 8, b1: 4. */
    uint8_t alloc_method;       /**< b0: loudness, b1: SNR. */
    uint8_t sample_rate;        /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    uint8_t channel_mode;       /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
} audio_codec_msbc_t;
typedef union {
    audio_codec_pcm_t pcm;
    audio_codec_sbc_enc_t sbc_enc;
    audio_codec_sbc_t sbc;
    audio_codec_opus_t opus;
    audio_codec_lc3_t lc3;
    audio_codec_lc3plus_t lc3plus;
    audio_codec_uld_t uld;
    audio_codec_msbc_t msbc;
} audio_codec_param_t;

typedef struct {
    uint32_t codec_type;
    audio_codec_param_t codec_param;
    uint32_t period;
    uint32_t gain_default_L;
    uint32_t gain_default_R;
} gaming_mode_param_t;


typedef struct {
    uint32_t codec_type;
    audio_codec_param_t codec_param;
    uint32_t period;
    int32_t gain[8];
    bool is_with_ecnr;
    bool is_with_swb;
} wired_audio_param_t;

typedef struct {
    uint32_t codec_type;
    audio_codec_param_t codec_param;
    uint32_t period;
    uint32_t channel_enable;
    int32_t gain_default_L;
    int32_t gain_default_R;
    uint8_t *share_buffer_channel_1;
    uint8_t *share_buffer_channel_2;
    uint8_t without_bt_link_mode_enable;
} ble_audio_dongle_param_t;

typedef struct {
    void *share_info;
    uint32_t codec_type;
    audio_codec_param_t codec_param;
} ull_audio_v2_dongle_bt_link_info_t;

typedef struct {
    uint32_t codec_type;
    audio_codec_param_t codec_param;
    int32_t gain_default_L;
    int32_t gain_default_R;
    ull_audio_v2_dongle_bt_link_info_t bt_link_info[4];
    bool nr_offload_flag;
    uint8_t without_bt_link_mode_enable;
} ull_audio_v2_dongle_param_t;

#if defined (AIR_BT_AUDIO_DONGLE_ENABLE)

#define BT_AUDIO_DATA_CHANNEL_NUMBER  (1)

typedef struct {
    bool enable;
    void *share_info;
    audio_dsp_codec_type_t codec_type;
    audio_codec_param_t codec_param;
    uint16_t blk_index;
    uint16_t blk_index_previous;
} bt_audio_dongle_bt_link_info_t;

typedef struct {
    audio_dsp_codec_type_t codec_type;
    audio_codec_param_t codec_param;
    uint32_t channel_num;
    uint32_t sample_rate;
    hal_audio_format_t sample_format;
    uint32_t frame_size;
    uint32_t frame_samples;
    uint32_t frame_interval;
    uint32_t frame_max_num;
    uint32_t seq_num;
} audio_dongle_usb_info_t;

typedef struct {
    audio_dsp_codec_type_t codec_type;
    uint32_t channel_num;
    uint32_t sample_rate;
    hal_audio_format_t sample_format;
    uint32_t frame_size;
    uint32_t frame_samples;
    uint32_t frame_interval;
    uint32_t bit_rate;
    uint32_t bt_interval;
    uint32_t link_num;
    bt_audio_dongle_bt_link_info_t bt_info[BT_AUDIO_DATA_CHANNEL_NUMBER];
} bt_audio_dongle_bt_in_info_t;

typedef union {
    uint32_t _reserved;
    audio_dongle_usb_info_t usb_out;
} bt_audio_dongle_ul_sink_info_t;

typedef union {
    uint32_t _reserved;
    bt_audio_dongle_bt_in_info_t bt_in;
} bt_audio_dongle_ul_source_info_t;

/* For ul: bt in(afe in) -> usb out */
typedef struct {
    bt_audio_dongle_ul_source_info_t  source;
    bt_audio_dongle_ul_sink_info_t    sink;
} bt_audio_dongle_ul_info_t;

typedef struct {
    audio_dsp_codec_type_t codec_type;
    uint32_t channel_num;
    uint32_t sample_rate;
    hal_audio_format_t sample_format;
    uint32_t frame_size;
    uint32_t frame_samples;
    uint32_t frame_interval;
    uint32_t bit_rate;
    uint32_t link_num;
    bt_audio_dongle_bt_link_info_t bt_info[BT_AUDIO_DATA_CHANNEL_NUMBER];
    bool     without_bt_link_mode_enable;
    bool     ts_not_reset_flag; // timestamp not reset
} bt_audio_dongle_bt_out_info_t;

typedef union {
    uint32_t _reserved;
    bt_audio_dongle_bt_out_info_t bt_out;
} bt_audio_dongle_dl_sink_info_t;

typedef union {
    uint32_t _reserved;
    audio_dongle_usb_info_t usb_in;
} bt_audio_dongle_dl_source_info_t;

/* For dl: usb in(afe in) -> bt out */
typedef struct {
    bt_audio_dongle_dl_source_info_t  source;
    bt_audio_dongle_dl_sink_info_t    sink;
} bt_audio_dongle_dl_info_t;

typedef struct {
    union {
        bt_audio_dongle_dl_info_t dl_info;
        bt_audio_dongle_ul_info_t ul_info;
    };
} bt_audio_dongle_config_t;

#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */

typedef struct {
    uint32_t codec_type;
    audio_codec_param_t codec_param;
}advanced_record_param_t;

typedef union {
    uint32_t reserved;
    gaming_mode_param_t gaming_mode_param;
    wired_audio_param_t wired_audio_param;
    ble_audio_dongle_param_t ble_audio_dongle_param;
    ull_audio_v2_dongle_param_t ull_audio_v2_dongle_param;
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
    bt_audio_dongle_config_t bt_audio_dongle_param;
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */
    advanced_record_param_t advanced_record_param;
} audio_transmitter_scenario_open_param_t;

typedef struct{
    uint8_t scenario_type;
    uint8_t scenario_sub_id;
    uint16_t data_notification_frequency;
    uint16_t max_payload_size;
    n9_dsp_share_info_t *p_share_info;
    audio_transmitter_scenario_open_param_t scenario_param;
}audio_transmitter_open_param_t, *audio_transmitter_open_param_p;

/*************************bt common param************************/
typedef enum {
    BUFFER_INFO_TYPE = 0,
    SHARE_BUFFER_INFO_TYPE,
    AVM_SHARE_BUF_INFO_TYPE,
    STREAM_BUFFER_TYPE_MAX = 0xffffffff
} STREAM_BUFFER_TYPE;

typedef union {
    uint32_t reserved;
    /* For scenario specific struct define */
    gaming_mode_param_t gaming_mode_param;
    ble_audio_dongle_param_t ble_audio_dongle_param;
    ull_audio_v2_dongle_param_t ull_audio_v2_dongle_param;
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
    bt_audio_dongle_config_t bt_audio_dongle_param;
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */
} bt_common_scenario_open_param_t;

typedef struct {
    uint8_t scenario_type;
    uint8_t scenario_sub_id;
    uint16_t data_notification_frequency;
    uint16_t max_payload_size;
    STREAM_BUFFER_TYPE share_info_type;
    void *p_share_info;
    bt_common_scenario_open_param_t scenario_param;
} bt_common_open_param_t, *bt_common_open_param_p;

/** @brief audio channel number define */
typedef enum {
    HAL_AUDIO_MONO                  = 0, /**< A single channel.  */
    HAL_AUDIO_STEREO                = 1, /**< Two channels. */
    HAL_AUDIO_STEREO_BOTH_L_CHANNEL = 2, /**< Two channels, but only output L channel. That is (L, R) -> (L, L). */
    HAL_AUDIO_STEREO_BOTH_R_CHANNEL = 3, /**< Two channels, but only output R channel. That is (L, R) -> (R, R). */
    HAL_AUDIO_STEREO_BOTH_L_R_SWAP  = 4, /**< Two channels, L and R channels are swapped. That is (L, R) -> (R, L). */
    HAL_AUDIO_CHANNEL_NUM_MAX       = 0xFFFFFFFF
} hal_audio_channel_number_t;

/**
 *  @brief This structure defines the CM4 playback share information.
 */
typedef struct {
    n9_dsp_share_info_t         *p_share_info;
    hal_audio_channel_number_t  channel_number;
    uint8_t  bit_type;
    uint8_t  sampling_rate;
    uint8_t  codec_type;
    uint8_t  data_local_address_index;
    uint8_t  data_local_stream_mode;
} audio_dsp_playback_info_t;

/*************************afe param************************/
/** @brief Audio device. */
typedef enum {
    //MCU side
    HAL_AUDIO_DEVICE_NONE               = 0x0000,  /**<  No audio device is on. */
    HAL_AUDIO_DEVICE_MAIN_MIC_L         = 0x0001,  /**<  Stream in: main mic L. */
    HAL_AUDIO_DEVICE_MAIN_MIC_R         = 0x0002,  /**<  Stream in: main mic R. */
    HAL_AUDIO_DEVICE_MAIN_MIC_DUAL      = 0x0003,  /**<  Stream in: main mic L+R. */
    HAL_AUDIO_DEVICE_LINEINPLAYBACK_L   = 0x0004,  /**<  Stream in: line in playback L. */
    HAL_AUDIO_DEVICE_LINEINPLAYBACK_R   = 0x0008,  /**<  Stream in: line in playback R. */
    HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL = 0x000c, /**<  Stream in: line in playback L+R. */
    HAL_AUDIO_DEVICE_USBAUDIOPLAYBACK_L   = 0x0014,  /**<  Stream in: usb audio playback L. */
    HAL_AUDIO_DEVICE_USBAUDIOPLAYBACK_R   = 0x0018,  /**<  Stream in: usb audio playback R. */
    HAL_AUDIO_DEVICE_USBAUDIOPLAYBACK_DUAL = 0x001c, /**<  Stream in: usb audio playback L+R. */
    HAL_AUDIO_DEVICE_DIGITAL_MIC_L      = 0x0010,  /**<  Stream in: digital mic L. */
    HAL_AUDIO_DEVICE_DIGITAL_MIC_R      = 0x0020,  /**<  Stream in: digital mic R. */
    HAL_AUDIO_DEVICE_DIGITAL_MIC_DUAL   = 0x0030,  /**<  Stream in: digital mic L+R. */

    HAL_AUDIO_DEVICE_DAC_L              = 0x0100,  /**<  Stream out:speaker L. */
    HAL_AUDIO_DEVICE_DAC_R              = 0x0200,  /**<  Stream out:speaker R. */
    HAL_AUDIO_DEVICE_DAC_DUAL           = 0x0300,  /**<  Stream out:speaker L+R. */

    HAL_AUDIO_DEVICE_SPDIF_IN           = 0x1000, /**<  Stream in: SPDIF. */
    HAL_AUDIO_DEVICE_I2S_SLAVE          = 0x2000,  /**<  Stream in/out: I2S slave role */
    HAL_AUDIO_DEVICE_EXT_CODEC          = 0x3000,   /**<  Stream out: external amp.&codec, stereo/mono */
    HAL_AUDIO_DEVICE_SPDIF              = 0x4000,   /**<  Stream out: SPDIF. */

    HAL_AUDIO_DEVICE_I2S_MASTER_L       = 0x10000, /**<  Stream in/out: I2S master L */
    HAL_AUDIO_DEVICE_I2S_MASTER_R       = 0x20000, /**<  Stream in/out: I2S master R */
    HAL_AUDIO_DEVICE_I2S_MASTER         = 0x30000, /**<  Stream in/out: I2S master role */

    HAL_AUDIO_DEVICE_MAIN_MIC           = 0x0001,       /**<  OLD: Stream in: main mic. */
    HAL_AUDIO_DEVICE_HEADSET_MIC        = 0x0002,       /**<  OLD: Stream in: earphone mic. */
    HAL_AUDIO_DEVICE_HANDSET            = 0x0004,       /**<  OLD: Stream out:receiver. */
    HAL_AUDIO_DEVICE_HANDS_FREE_MONO    = 0x0008,       /**<  OLD: Stream out:loudspeaker, mono. */
    HAL_AUDIO_DEVICE_HANDS_FREE_STEREO  = 0x0010,       /**<  OLD: Stream out:loudspeaker, stereo to mono L=R=(R+L)/2. */
    HAL_AUDIO_DEVICE_HEADSET            = 0x0020,       /**<  OLD: Stream out:earphone, stereo */
    HAL_AUDIO_DEVICE_HEADSET_MONO       = 0x0040,       /**<  OLD: Stream out:earphone, mono to stereo. L=R. */
    HAL_AUDIO_DEVICE_LINE_IN            = 0x0080,       /**<  OLD: Stream in/out: line in. */
    HAL_AUDIO_DEVICE_DUAL_DIGITAL_MIC   = 0x0100,       /**<  OLD: Stream in: dual digital mic. */
    HAL_AUDIO_DEVICE_SINGLE_DIGITAL_MIC = 0x0200,       /**<  OLD: Stream in: single digital mic. */
    HAL_AUDIO_DEVICE_LOOPBACK_L         = 0x0400,   /**<  Stream in:     HW AD loopback. */
    HAL_AUDIO_DEVICE_LOOPBACK_R         = 0x0800,   /**<  Stream in:     HW AD loopback. */
    HAL_AUDIO_DEVICE_LOOPBACK_DUAL      = 0x0C00,   /**<  Stream in:     HW AD loopback. */
    HAL_AUDIO_DEVICE_LOOPBACK_HW_GAIN_L         = 0x100000,   /**<  Stream in: HW Gain L loopback. */
    HAL_AUDIO_DEVICE_LOOPBACK_HW_GAIN_R         = 0x200000,   /**<  Stream in: HW Gain R loopback. */
    HAL_AUDIO_DEVICE_LOOPBACK_I2S_MASTER_L      = 0x400000,  /**<  Stream in: I2S master L loopback. */
    HAL_AUDIO_DEVICE_LOOPBACK_I2S_MASTER_R      = 0x800000,  /**<  Stream in: I2S master R loopback. */

    HAL_AUDIO_DEVICE_DUMMY              = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
#if 0
    //DSP side
    HAL_AUDIO_CONTROL_NONE                              = 0x0000,   /**<  No audio device is on. */

    HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_L               = 0x0001,
    HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_R               = 0x0002,
    HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL            = 0x0003,   /**<  Stream in:     main mic. */

    HAL_AUDIO_CONTROL_DEVICE_LINE_IN_L                  = 0x0004,
    HAL_AUDIO_CONTROL_DEVICE_LINE_IN_R                  = 0x0008,
    HAL_AUDIO_CONTROL_DEVICE_LINE_IN_DUAL               = 0x000C,   /**<  Stream in:     LineIn. */

    HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_L              = 0x0010,
    HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_R              = 0x0020,
    HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL           = 0x0030,   /**<  Stream in:     digital mic. */

    HAL_AUDIO_CONTROL_DEVICE_ANC                        = 0x0040,   /**<  Stream in:     HW ANC. */
    HAL_AUDIO_CONTROL_DEVICE_VAD                        = 0x0080,   /**<  Stream in:     HW VAD. */
    HAL_AUDIO_CONTROL_DEVICE_VOW                        = 0x00C0,   /**<  Stream in:     VOW. */

    HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_L             = 0x0100,
    HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_R             = 0x0200,
    HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL          = 0x0300,   /**<  Stream out:    AMP. */

    HAL_AUDIO_CONTROL_DEVICE_SIDETONE                   = 0x0400,   /**<  Stream out:    HW Sidetone. */
    HAL_AUDIO_CONTROL_DEVICE_LOOPBACK                   = 0x0800,   /**<  Stream in:     HW AD loopback. */

    HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE                  = 0x2000,   /**<  Stream in/out: I2S slave. */
    HAL_AUDIO_CONTROL_DEVICE_SPDIF                      = 0x4000,   /**<  Stream out: SPDIF. */

    HAL_AUDIO_CONTROL_MEMORY_INTERFACE                  = 0x8000,   /**<  Stream internal: Memory interface. */
    HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L               = 0x10000,
    HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R               = 0x20000,
    HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER                 = 0x30000,  /**<  Stream in/out: I2S master. */
    HAL_AUDIO_CONTROL_DUMMY                             = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
#endif
} hal_audio_device_t;

/** @brief audio channel selection define */
typedef enum {
    HAL_AUDIO_DIRECT                     = 0, /**< A single interconnection, output equal to input. */
    HAL_AUDIO_SWAP_L_R                   = 2, /**< L and R channels are swapped. That is (L, R) -> (R, L). */
    HAL_AUDIO_BOTH_L                     = 3, /**< only output L channel. That is (L, R) -> (L, L). */
    HAL_AUDIO_BOTH_R                     = 4, /**< only output R channel. That is (L, R) -> (R, R). */
    HAL_AUDIO_MIX_L_R                    = 5, /**< L and R channels are mixed. That is (L, R) -> (L+R, L+R). */
    HAL_AUDIO_MIX_SHIFT_L_R              = 6, /**< L and R channels are mixed and shift. That is (L, R) -> (L/2+R/2, L/2+R/2). */
    HAL_AUDIO_CHANNEL_DUMMY              = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} hal_audio_channel_selection_t;

/** @brief AUDIO port */
typedef enum {
    HAL_AUDIO_STREAM_OUT1 = 1 << 0,    /**<  stream out HWGAIN1 only. */
    HAL_AUDIO_STREAM_OUT2 = 1 << 1,    /**<  stream out HWGAIN2 only. */
    HAL_AUDIO_STREAM_OUT3 = 1 << 2,    /**<  stream out HWGAIN3 only. */
    HAL_AUDIO_STREAM_OUT4 = 1 << 3,    /**<  stream out HWGAIN4 only. */
    HAL_AUDIO_STREAM_OUT_ALL = 0xFFFFFFFF, /**<  stream out HWGAIN1 & HWGAIN2 & HWGAIN3. */
} hal_audio_hw_stream_out_index_t;

typedef enum {
    HAL_AUDIO_MEM1                              = 0x0001,       /**< Memory path 1. UL:DL1_data, VUL:VUL1_data   */
    HAL_AUDIO_MEM2                              = 0x0002,       /**< Memory path 2. UL:DL2_data, VUL:VUL2_data   */
    HAL_AUDIO_MEM3                              = 0x0004,       /**< Memory path 3. UL:DL3_data, VUL:AWB_data   */
    HAL_AUDIO_MEM4                              = 0x0008,       /**< Memory path 4. UL:DL12_data, VUL:AWB2_data   */
    HAL_AUDIO_MEM5                              = 0x0010,       /**< Memory path 5. VUL:VUL3_data   */
    HAL_AUDIO_MEM6                              = 0x0020,       /**< Memory path 6. SLAVE DMA data   */
    HAL_AUDIO_MEM7                              = 0x0040,       /**< Memory path 7. SLAVE TDM data   */
    HAL_AUDIO_MEM_SUB                           = 0x0100,       /**< Memory path Sub. memory interfaceusage depend on mian source   */
    HAL_AUDIO_MEM_DUMMY                         = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} hal_audio_memory_t;

/** @brief audio interface define */
typedef enum {
    HAL_AUDIO_INTERFACE_NONE                    = 0x0000,       /**< No audio interface should be selected, */
    HAL_AUDIO_INTERFACE_1                       = 0x0001,       /**< Audio interface path 1. UL:UL SRC ch1 ch2, I2S Master:I2S0, */
    HAL_AUDIO_INTERFACE_2                       = 0x0002,       /**< Audio interface path 2. UL:UL SRC ch3 ch4, I2S Master:I2S1, */
    HAL_AUDIO_INTERFACE_3                       = 0x0004,       /**< Audio interface path 3. UL:UL SRC ch5 ch6, I2S Master:I2S2, */
    HAL_AUDIO_INTERFACE_4                       = 0x0008,       /**< Audio interface path 4. I2S Master:I2S3, */
    HAL_AUDIO_INTERFACE_DUMMY                   = 0xFFFFFFFF,   /**<  for DSP structrue alignment */
} hal_audio_interface_t;

/** @brief Hal audio analog type setting*/
typedef enum {
    HAL_AUDIO_ANALOG_TYPE_DIFF     = 0,            /**<   for amic type*/
    HAL_AUDIO_ANALOG_TYPE_SINGLE   = 1,            /**<   for amic type*/
    HAL_AUDIO_ANALOG_TYPE_DUMMY    = 0x7FFFFFFF,   /**<  Dummy for DSP structure alignment */
} hal_audio_adc_type_t;

typedef enum {
    HAL_AUDIO_NO_USE_HWSRC    = 0,        /**< no use hwsrc*/
    HAL_AUDIO_HWSRC_IN_STREAM = 1,        /**< use hwsrc in stream */
    HAL_AUDIO_HWSRC_ON_STREAM = 2,        /**< use hwsrc on stream */
    HAL_AUDIO_HWSRC_TYPE_DUMMY =  0x7FFFFFFF,   /**<  Dummy for DSP structure alignment */
} hal_audio_afe_hwsrc_type_t;

/** @brief Hal audio analog mode setting. */
#if defined(AIR_BTA_IC_PREMIUM_G3)
typedef enum {
    HAL_AUDIO_ANALOG_INPUT_ACC10K   = 0,            /**<   for amic mode*/
    HAL_AUDIO_ANALOG_INPUT_ACC20K   = 1,            /**<   for amic mode*/
    HAL_AUDIO_ANALOG_INPUT_ACC10K_SINGLE = 2,            /**<   for amic mode*/
    HAL_AUDIO_ANALOG_INPUT_ACC20K_SINGLE = 3,            /**<   for amic mode*/

    HAL_AUDIO_ANALOG_OUTPUT_CLASSG2 = 0,            /**<   for dac mode*/
    HAL_AUDIO_ANALOG_OUTPUT_CLASSAB = 1,            /**<   for dac mode*/
    HAL_AUDIO_ANALOG_OUTPUT_CLASSD  = 2,            /**<   for dac mode*/
    HAL_AUDIO_ANALOG_OUTPUT_CLASSG3 = 3,            /**<   for dac mode*/
    HAL_AUDIO_ANALOG_OUTPUT_OLCLASSD = 4,           /**<   for dac mode*/
    HAL_AUDIO_ANALOG_MODE_DUMMY     = 0x7FFFFFFF,   /**<  Dummy for DSP structure alignment */
} hal_audio_analog_mdoe_t;
#endif

#if defined(AIR_BTA_IC_PREMIUM_G2)
typedef enum {
    HAL_AUDIO_ANALOG_INPUT_ACC10K   = 0,            /**<   for amic mode*/
    HAL_AUDIO_ANALOG_INPUT_ACC20K   = 1,            /**<   for amic mode*/
    HAL_AUDIO_ANALOG_INPUT_DCC      = 2,            /**<   for amic mode*/

    HAL_AUDIO_ANALOG_OUTPUT_CLASSG  = 0,            /**<   for dac mode*/
    HAL_AUDIO_ANALOG_OUTPUT_CLASSAB = 1,            /**<   for dac mode*/
    HAL_AUDIO_ANALOG_OUTPUT_CLASSD  = 2,            /**<   for dac mode*/
    HAL_AUDIO_ANALOG_MODE_DUMMY     = 0x7FFFFFFF,   /**<  Dummy for DSP structure alignment */
} hal_audio_analog_mdoe_t;
#endif

/** @brief Hal audio micbias performance mode setting. */
typedef enum {
    AUDIO_MICBIAS_NONE_MODE       = -1,
    AUDIO_MICBIAS_NORMAL_MODE     = 0,           /**<  for micbias performance normal mode */
    AUDIO_MICBIAS_HIGH_MODE       = 1,           /**<  for micbias performance high performance mode */
    AUDIO_MICBIAS_LOW_POWER_MODE  = 2,           /**<  for micbias performance low power mode */
    AUDIO_MICBIAS_DUMMY           = 0x7FFFFFFF,  /**<  Dummy for DSP structure alignment */
} hal_audio_micbias_mode_t;

/** @brief Hal audio performance mode setting. */
typedef enum {
    AFE_PEROFRMANCE_NORMAL_MODE     = 0,           /**<  for mic/dac performance mode */
    AFE_PEROFRMANCE_HIGH_MODE       = 1,           /**<  for mic/dac performance mode */
    AFE_PEROFRMANCE_LOW_POWER_MODE  = 2,          /**<  for mic performance mode */
    //The followings are only for UL
    AFE_PEROFRMANCE_ULTRA_LOW_POWER_MODE  = 3,    /**<  for mic performance mode */
    AFE_PEROFRMANCE_SUPER_ULTRA_LOW_POWER_MODE  = 4, /**<  for mic performance mode */
    AFE_PEROFRMANCE_MAX,
    AFE_PEROFRMANCE_DUMMY     = 0x7FFFFFFF,   /**<  Dummy for DSP structure alignment */
} hal_audio_performance_mode_t;

/** @brief Hal audio performance mode setting. */
typedef enum {
    AFE_COMMON_PARA_DAC_DEACTIVE     = 0,           /**<  for dac deactive mode */
    AFE_COMMON_PARA_MAX,
} hal_audio_common_param_t;

typedef enum {
    CLK_SKEW_DISSABLE = 0,
    CLK_SKEW_V1 = 1, /* sw clk skew */
    CLK_SKEW_V2 = 2, /* hwsrc clk skew */
    CLK_SKEW_DUMMY = 0xFFFFFFFF,
} clkskew_mode_t;

/** @brief Hal audio anc debug path sel. */
typedef enum {
    HAL_AUDIO_ANC_DEBUG_SEL_CH23     = 0,
    HAL_AUDIO_ANC_DEBUG_SEL_CH01     = 1,
    HAL_AUDIO_ANC_DEBUG_SEL_ADP_CH01 = 2,
    HAL_AUDIO_ANC_DEBUG_SEL_DUMMY    = 0x7FFFFFFF,
} hal_audio_anc_debug_sel_t;

/** @brief Hal audio dmic selection. */
typedef enum {
    HAL_AUDIO_DMIC_GPIO_DMIC0   = 0x0,              /**<  for dmic selection */
    HAL_AUDIO_DMIC_GPIO_DMIC1,                      /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC0,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC1,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC2,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC3,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC4,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_ANA_DMIC5,                       /**<  for dmic selection */
    HAL_AUDIO_DMIC_DUMMY        = 0xFFFFFFFF,           /**<  for DSP structrue alignment */
} hal_audio_dmic_selection_t;

typedef enum {
    AFE_DMIC_CLOCK_3_25M    = 0,
    AFE_DMIC_CLOCK_1_625M   = 1,
    AFE_DMIC_CLOCK_812_5K   = 2,
    AFE_DMIC_CLOCK_406_25K  = 3,
    AFE_DMIC_CLOCK_MAX,
    AFE_DMIC_CLOCK_DUMMY    = 0x7fffffff,
} afe_dmic_clock_rate_t;

typedef struct {
    hal_audio_device_t               audio_device;
    hal_audio_device_t               audio_device1;
    hal_audio_device_t               audio_device2;
    hal_audio_device_t               audio_device3;
    hal_audio_device_t               audio_device4;
    hal_audio_device_t               audio_device5;
    hal_audio_device_t               audio_device6;
    hal_audio_device_t               audio_device7;
    uint32_t                                audio_device_input_rate[8]; /**< for audio_device_input_rate */
    /*I2S master sampling_rate*/
    uint32_t                                i2s_master_sampling_rate[4];
    uint32_t                                misc_parms;
    uint32_t                                sampling_rate;
    uint32_t                                stream_out_sampling_rate;
    uint32_t                                stream_process_sampling_rate; /* stream process sample rate for both AFE source/sink scenario */
    uint16_t                                frame_size;
    afe_pcm_format_t                        format;
    hal_audio_analog_mdoe_t                 adc_mode;
    hal_audio_memory_t                      memory;
    uint8_t                                 ul_adc_mode[8];
    /*amic type
        0x0: MEMS,
        0x1: ECM Differential,
        0x2: ECM Single,
    */
    uint8_t                                 amic_type[8];
    /*downlink dac mode
        0x0: Class_G,
        0x1: Class_AB,
        0x2: Class_D,
    */
    uint8_t                                 dl_dac_mode;
    /*bias voltage,support 5 bia voltage
        0x0: 1.8V,
        0x1: 1.85V,
        0x2: 1.9V,
        0x3: 2.0V,
        0x4: 2.1V,
        0x5: 2.2V,
        0x6: 2.4V,
        0x7: 2.55V */
    uint8_t                                 bias_voltage[5];/**hal_audio_bias_voltage_t*/
    /*bias enable
        bit mask to enable Bias
        8'b 00000001: Bias 0,
        8'b 00000010: Bias 1,
        8'b 00000100: Bias 2,
        8'b 00001000: Bias 3,
        8'b 00010000: Bias 4,*/
    uint8_t                                 bias_select;/**hal_audio_bias_selection_t*/
    /*external bias enable (reserve)*/
    uint8_t                                 with_external_bias;/**< for with_external_bias */
    /*DMIC select (reserve???)*/
    uint8_t                                 dmic_selection[8];/**< hal_audio_dmic_selection_t */
    /*iir filter (reserve)*/
    uint8_t                                 iir_filter[3];/**hal_audio_ul_iir_t*/
    /*I2S format (reserve)*/
    uint8_t                                 i2s_format;/**< hal_audio_i2s_format_t */
    /*I2S slave TDM (reserve)*/
    uint8_t                                 i2S_Slave_TDM;
    /*I2S word length(reserve)*/
    uint8_t                                 i2s_word_length;/**< hal_audio_i2s_word_length_t */
    /*I2S master format*/
    uint8_t                                 i2s_master_format[4];/**< hal_audio_i2s_format_t */
    /*I2S master word length*/
    uint8_t                                 i2s_master_word_length[4];/**< hal_audio_i2s_word_length_t */
    uint8_t                                 frame_number;
    uint8_t                                 irq_period;
    uint8_t                                 sw_channels;
    /*DMIC clock(reserve)*/
    uint8_t                                 dmic_clock_rate[3];/**afe_dmic_clock_rate_t*/
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    uint8_t  max_channel_num;
#endif
    hal_audio_channel_selection_t           stream_channel;
    hal_audio_interface_t                   audio_interface;
    hal_audio_interface_t                   audio_interface1;
    hal_audio_interface_t                   audio_interface2;
    hal_audio_interface_t                   audio_interface3;
    hal_audio_interface_t                   audio_interface4;
    hal_audio_interface_t                   audio_interface5;
    hal_audio_interface_t                   audio_interface6;
    hal_audio_interface_t                   audio_interface7;
    /*uplink adc mode:
        0x0: ACC_10k,
        0x1: ACC_20k,
        0x2: DCC,
    */
    hal_audio_anc_debug_sel_t               anc_ch_select;
    /*bias lowpower enable (reserve)*/
    hal_audio_micbias_mode_t                with_bias_lowpower;/**< for with_bias_lowpower */
    /*define if bias1_2 config with LDO0 (reserve)*/
    hal_audio_performance_mode_t            performance;
    clkskew_mode_t                          clkskew_mode;
#ifdef AIR_HFP_DNN_PATH_ENABLE
    bool                                    enable_ul_dnn;
#endif
    hal_audio_afe_hwsrc_type_t              hwsrc_type;
    hal_audio_adc_type_t                    adc_type;
    bool                                    bias1_2_with_LDO0;/**< for bias1_2_with_LDO0 */
    /*I2S low jitter config
        false, DCXO
        true,  APLL*/
    bool                                    is_low_jitter[4];
    bool                                    hw_gain;
#ifdef AIR_DCHS_MODE_ENABLE
    uint8_t                                 dchs_ul_scenario_type;
    uint32_t                                codec_type;
#endif
} au_afe_open_param_t,*au_afe_open_param_p;

typedef enum {
    AUDIO_DRIVER_SET_GAIN_OFFSET    = 0,
    AUDIO_DRIVER_SET_HOLD_AMP_GPIO  = 1,
    AUDIO_DRIVER_SET_3RD_PARTY_PLATFORM_SHARE_INFO  = 2,
#ifdef AIR_KEEP_I2S_ENABLE
    AUDIO_DRIVER_SET_DEVICE         = 3,
#endif
    AUDIO_DRIVER_SET_NUMBER,
} audio_driver_set_info_t;

typedef struct {
    uint8_t                                 scenario_type;
    uint8_t                                 scenario_sub_id;
    uint16_t                                frame_size;

    hal_audio_device_t                      audio_device[LLF_DATA_TYPE_MIC_NUMBER];

    hal_audio_interface_t                   audio_interface[LLF_DATA_TYPE_MIC_NUMBER];

    uint32_t                                audio_device_enable[LLF_DATA_TYPE_MIC_NUMBER];

    afe_pcm_format_t                        format;
    uint8_t                                 frame_number;
    uint8_t                                 channel_num;
    uint8_t                                 earbuds_ch;
    uint8_t                                 music_need_compensation;

    bool                                    echo_reference[LLF_ECHO_REF_NUMBER];
    int16_t                                 anc_ff_cal_gain;

    uint32_t                                in_data_order[LLF_DATA_TYPE_NUMBER];

    n9_dsp_share_info_t                     share_info;
} llf_open_param_t, *llf_open_param_p;

#ifdef AIR_DCHS_MODE_ENABLE
typedef struct {
    uint8_t codec;
    uint16_t dev_in;
    uint8_t lev_in;
    uint16_t dev_out;
    uint8_t lev_out;
} dchs_vol_info_t, *dchs_vol_info_p;
#endif

typedef enum {
    VIRTUAL_SOURCE_ZERO_DATA,
    VIRTUAL_SOURCE_WIRED_AUDIO_USB_OUT,
}VIRTUAL_SOURCE_TYPE;

typedef struct {
    uint32_t virtual_source_type;
    uint32_t virtual_mem_size;
} audio_virtual_open_param_t;

/* Open message member parameter structure */
typedef enum {
    STREAM_IN_AFE  = 0,
    STREAM_IN_HFP,
    STREAM_IN_BLE,
    STREAM_IN_A2DP,
    STREAM_IN_PLAYBACK,
    STREAM_IN_VP,
    STREAM_IN_VP_DUMMY_SOURCE,
    STREAM_IN_GSENSOR,
    STREAM_IN_AUDIO_TRANSMITTER,
    STREAM_IN_BT_COMMON,
    STREAM_IN_ADAPT_ANC,
    STREAM_IN_UART,
    STREAM_IN_LLF,
    STREAM_IN_VIRTUAL,
    STREAM_IN_MIXER = 14,
    STREAM_IN_DUMMY = 0xFFFFFFFF,
} mcu2dsp_stream_in_selection;

typedef enum {
    STREAM_OUT_AFE  = 0,
    STREAM_OUT_HFP,
    STREAM_OUT_BLE,
    STREAM_OUT_RECORD,
    STREAM_OUT_VIRTUAL,
    STREAM_OUT_AUDIO_TRANSMITTER,
    STREAM_OUT_BT_COMMON,
    STREAM_OUT_LLF,
    STREAM_OUT_DUMMY = 0xFFFFFFFF,
} mcu2dsp_stream_out_selection;

typedef struct {
    mcu2dsp_stream_in_selection     stream_in;
    mcu2dsp_stream_out_selection    stream_out;
    uint32_t                      *Feature;
}  mcu2dsp_param_t, *mcu2dsp_param_p;

typedef union {
    au_afe_open_param_t         afe;
    audio_dsp_hfp_open_param_t  hfp;
#ifdef AIR_BT_CODEC_BLE_ENABLED
    audio_dsp_ble_open_param_t  ble;
#endif
    audio_dsp_a2dp_dl_open_param_t a2dp;
    audio_dsp_playback_info_t   playback;
    audio_transmitter_open_param_t data_dl;
    bt_common_open_param_t      bt_common;
    llf_open_param_t     LLF;
#ifdef AIR_DCHS_MODE_ENABLE
    dchs_vol_info_t             vol_info;
#endif
    audio_virtual_open_param_t  virtual_param;
} mcu2dsp_open_stream_in_param_t, *mcu2dsp_open_stream_in_param_p;

typedef union {
    au_afe_open_param_t         afe;
    audio_dsp_hfp_open_param_t  hfp;
#ifdef AIR_BT_CODEC_BLE_ENABLED
    audio_dsp_ble_open_param_t  ble;
#endif
    cm4_record_open_param_t     record;
    audio_transmitter_open_param_t data_ul;
    bt_common_open_param_t      bt_common;
    llf_open_param_t     LLF;
#ifdef AIR_DCHS_MODE_ENABLE
    dchs_vol_info_t             vol_info;
#endif
} mcu2dsp_open_stream_out_param_t, *mcu2dsp_open_stream_out_param_p;

/** @brief Audio scenario type for clock setting */
typedef enum {
    AUDIO_SCENARIO_TYPE_COMMON   = 0,                 /**< Audio scenario type common. NOT USE */
    AUDIO_SCENARIO_TYPE_AMP         ,                 /**< Audio scenario type AMP. Only Used By DSP */
    AUDIO_SCENARIO_TYPE_VP_PRE      ,                 /**< Audio scenario type voice prompt pre enable. */
    AUDIO_SCENARIO_TYPE_VP          ,                 /**< Audio scenario type voice prompt. */
    AUDIO_SCENARIO_TYPE_VP_DUMMY_PRE,                 /**< Audio scenario type DJ mode pre enable. */
    AUDIO_SCENARIO_TYPE_VP_DUMMY    ,                 /**< Audio scenario type DJ mode. */
    AUDIO_SCENARIO_TYPE_HFP_UL      ,                 /**< Audio scenario type eSCO UL. */
    AUDIO_SCENARIO_TYPE_HFP_DL      ,                 /**< Audio scenario type eSCO DL. */
    AUDIO_SCENARIO_TYPE_RECORD      ,                 /**< Audio scenario type Record. */
    AUDIO_SCENARIO_TYPE_WWE         ,                 /**< Audio scenario type WWE. */
    AUDIO_SCENARIO_TYPE_A2DP    = 10,                 /**< Audio scenario type A2DP. */
    AUDIO_SCENARIO_TYPE_BLE_UL      ,                 /**< Audio scenario type BLE UL. */
    AUDIO_SCENARIO_TYPE_BLE_DL      ,                 /**< Audio scenario type BLE DL. */
    AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL,                 /**< Audio scenario type BLE DL. */
    AUDIO_SCENARIO_TYPE_ANC         ,                 /**< Audio scenario type ANC. */
    AUDIO_SCENARIO_TYPE_SIDETONE    ,                 /**< Audio scenario type Sidetone. */
    AUDIO_SCENARIO_TYPE_VOW         ,                 /**< Audio scenario type VOW. */
    AUDIO_SCENARIO_TYPE_LINE_IN     ,                 /**< Audio scenario type Line in. */
    AUDIO_SCENARIO_TYPE_LINE_OUT    ,                 /**< Audio scenario type Line out. */
    AUDIO_SCENARIO_TYPE_SPDIF       ,                 /**< Audio scenario type Spdif. */
    AUDIO_SCENARIO_TYPE_MCLK    = 20,                 /**< Audio scenario type MCLK, for AUDIO_MESSAGE_TYPE_MCLK. */
    AUDIO_SCENARIO_TYPE_USB_AUDIO_PLAYBACK ,          /**< Audio scenario type USB Audio, for AUDIO_MESSAGE_TYPE_PLAYBACK. */
    AUDIO_SCENARIO_TYPE_PLAYBACK    ,                 /**< Audio scenario type PLAYBACK, for old code. */
    AUDIO_SCENARIO_TYPE_DC_COMPENSATION,              /**< Audio scenario type DC Compensation. */


    /* audio transmitter type*/
    AUDIO_SCENARIO_TYPE_GSENSOR_FUNCTION_D = 30,                  /**< Audio scenario type audio transmitter gsensor function D. */
    AUDIO_SCENARIO_TYPE_GSENSOR_FUNCTION_F,                       /**< Audio scenario type audio transmitter gsensor function F. */
    AUDIO_SCENARIO_TYPE_GSENSOR_FUNCTION_G,                       /**< Audio scenario type audio transmitter gsensor function G. */
    AUDIO_SCENARIO_TYPE_MULTI_MIC_STREAM_FUNCTION_A,              /**< Audio scenario type audio transmitter multi-mic function A. */
    AUDIO_SCENARIO_TYPE_MULTI_MIC_STREAM_FUNCTION_B,              /**< Audio scenario type audio transmitter multi-mic function B. */
    AUDIO_SCENARIO_TYPE_MULTI_MIC_STREAM_FUNCTION_C,              /**< Audio scenario type audio transmitter multi-mic function C. */
    AUDIO_SCENARIO_TYPE_MULTI_MIC_STREAM_FUNCTION_F,              /**< Audio scenario type audio transmitter multi-mic function F. */
    AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_HEADSET,                /**< Audio scenario type audio transmitter gaming mode voice headset. */
    AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_USB_OUT ,        /**< Audio scenario type audio transmitter gaming mode voice dongle usb out. */
    AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_0,        /**< Audio scenario type audio transmitter gaming mode music dongle usb in 0. */
    AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_1 = 40,   /**< Audio scenario type audio transmitter gaming mode music dongle usb in 0. */
    AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT,        /**< Audio scenario type audio transmitter gaming mode voice dongle line out. */
    AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN,         /**< Audio scenario type audio transmitter gaming mode music dongle line in. */
    AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_I2S_OUT,         /**< Audio scenario type audio transmitter gaming mode voice dongle i2s out. */
    AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN,          /**< Audio scenario type audio transmitter gaming mode music dongle i2s in. */
    AUDIO_SCENARIO_TYPE_ANC_MONITOR_STREAM,                       /**< Audio scenario type audio transmitter anc monitor stream. */
    AUDIO_SCENARIO_TYPE_TEST_AUDIO_LOOPBACK,                      /**< Audio scenario type audio transmitter audio loopback, record mic data send to user. */
    AUDIO_SCENARIO_TYPE_TDM,                                      /**< Audio scenario type audio transmitter TDM. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT,                      /**< Audio scenario type audio transmitter wire audio usb out. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0,                     /**< Audio scenario type audio transmitter wire audio usb in 0. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1 = 50,                /**< Audio scenario type audio transmitter wire audio usb in 1. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT,                     /**< Audio scenario type audio transmitter wire audio Line out. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN,                      /**< Audio scenario type audio transmitter wire audio Line in. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER,     /**< Audio scenario type audio transmitter wire audio dual chip Line in master. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE,      /**< Audio scenario type audio transmitter wire audio dual chip Line in slave. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER,    /**< Audio scenario type audio transmitter wire audio dual chip Line out master. */
    AUDIO_SCENARIO_TYPE_ADVANCED_PASSTHROUGH_HEARING_AID,         /**< Audio scenario type audio transmitter advance passthrough hearing aid. */
    AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_VOICE_USB_OUT,           /**< Audio scenario type audio transmitter ble audio dongle usb out. */
    AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0,          /**< Audio scenario type audio transmitter ble audio dongle usb in 0. */
    AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1,          /**< Audio scenario type audio transmitter ble audio dongle usb in 1. */
    AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_VOICE_LINE_OUT = 60,     /**< Audio scenario type audio transmitter ble audio dongle line out. */
    AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN,           /**< Audio scenario type audio transmitter ble audio dongle line in. */
    AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN,            /**< Audio scenario type audio transmitter ble audio dongle i2s in. */
    AUDIO_SCENARIO_TYPE_AUDIO_HW_LOOPBACK_I2S0_TO_DAC,            /**< Audio scenario type audio transmitter audio hardware loopback I2S to DAC. */
    AUDIO_SCENARIO_TYPE_AUDIO_HW_LOOPBACK_ADC_TO_I2S0,            /**< Audio scenario type audio transmitter audio hardware loopback ADC to I2S. */
    AUDIO_SCENARIO_TYPE_AUDIO_HW_LOOPBACK_LINEIN_TO_I2S2,         /**< Audio scenario type audio transmitter audio hardware loopback line in to I2S. */
    AUDIO_SCENARIO_TYPE_ADAPTIVE_EQ_MONITOR_STREAM,               /**< Audio scenario type audio transmitter adaptive eq monitor stream. */
    AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0,         /**< Audio scenario type audio transmitter ull audio v2 dongle usb out. */
    AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT,          /**< Audio scenario type audio transmitter ull audio v2 dongle line out. */
    AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0,     /**< Audio scenario type audio transmitter ull audio v2 dongle I2S master out. */
    AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0 = 70,/**< Audio scenario type audio transmitter ull audio v2 dongle I2S slave out. */
    AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0,          /**< Audio scenario type audio transmitter ull audio v2 dongle usb in 0. */
    AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1,          /**< Audio scenario type audio transmitter ull audio v2 dongle usb in 1. */
    AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN,           /**< Audio scenario type audio transmitter ull audio v2 dongle line in. */
    AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0,      /**< Audio scenario type audio transmitter ull audio v2 dongle I2S master in. */
    AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0,      /**< Audio scenario type audio transmitter ull audio v2 dongle I2S slave in. */
    AUDIO_SCENARIO_TYPE_MIXER_STREAM,                             /**< Audio scenario type audio transmitter dchs uart dl. */
    AUDIO_SCENARIO_TYPE_DCHS_UART_UL,                             /**< Audio scenario type audio transmitter dchs uart ul. */
    AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_USB_OUT_0,             /**< Audio scenario type audio transmitter wireless microphone receiver usb out. */
    AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_LINE_OUT,              /**< Audio scenario type audio transmitter wireless microphone receiver line out. */
    AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_MST_OUT_0 = 80,    /**< Audio scenario type audio transmitter wireless microphone receiver I2S master out. */
    AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0,         /**< Audio scenario type audio transmitter wireless microphone receiver I2S slave out. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0,         /**< Audio scenario type audio transmitter bt source dongle usb 0 out for call. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0,          /**< Audio scenario type audio transmitter bt source dongle usb 0 in for call. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0,         /**< Audio scenario type audio transmitter bt source dongle usb 0 in for music. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1,         /**< Audio scenario type audio transmitter bt source dongle usb 1 out for call. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1,          /**< Audio scenario type audio transmitter bt source dongle usb 1 in for call. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1,         /**< Audio scenario type audio transmitter bt source dongle usb 1 in for music. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0,         /**< Audio scenario type audio transmitter bt source dongle line in for music. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1,         /**< Audio scenario type audio transmitter bt source dongle i2s in for music. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2,         /**< Audio scenario type audio transmitter bt source dongle dummy in for music. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0,          /**< Audio scenario type audio transmitter bt source dongle line in for call. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1,          /**< Audio scenario type audio transmitter bt source dongle i2s in for call. */
    AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2,          /**< Audio scenario type audio transmitter bt source dongle dummy in for call. */

    AUDIO_SCENARIO_TYPE_ADVANCED_RECORD_N_MIC,                    /**< Audio scenario type audio transmitter advanced record n mic. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_OUT_IEM,               /**< Audio scenario type audio transmitter wire audio usb in mix with usb out. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM,                   /**< Audio scenario type audio transmitter wire audio main stream. */
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_SUBSTREAM,                    /**< Audio scenario type audio transmitter wire audio sub stream. */
    /* audio transmitter type end */
    AUDIO_SCENARIO_TYPE_ANC_SUB_MIC_USR_0,                        /**< Audio scenario type ANC sub mic user 0. */
    AUDIO_SCENARIO_TYPE_ANC_SUB_MIC_USR_1,                        /**< Audio scenario type ANC sub mic user 1. */
    AUDIO_SCENARIO_TYPE_ANC_SUB_MIC_USR_2,                        /**< Audio scenario type ANC sub mic user 2. */
    AUDIO_SCENARIO_TYPE_ANC_SUB_MIC_USR_3,                        /**< Audio scenario type ANC sub mic user 3. */
    AUDIO_SCENARIO_TYPE_ANC_SUB_MIC_USR_4,                        /**< Audio scenario type ANC sub mic user 4. */
    AUDIO_SCENARIO_TYPE_ANC_SUB_MIC_USR_5,                        /**< Audio scenario type ANC sub mic user 5. */

    AUDIO_SCENARIO_TYPE_FADP_ANC_STREAM,              /**< Audio scenario type FADP ANC STREAM. */
    AUDIO_SCENARIO_TYPE_LLF_ANC_STREAM,               /**< Audio scenario type LLF ANC STREAM. */

    AUDIO_SCENARIO_TYPE_DEVICE,                       /**< Audio scenario type DEVICE. for turn on device only */
    AUDIO_SCENARIO_TYPE_BLE_ULL_DL,
    /* END */
    AUDIO_SCENARIO_TYPE_END          ,                /**< Audio scenario type End. */
    AUDIO_SCEANRIO_TYPE_MAX      = 127,               /**< Audio scenario type max 128. NOT USE */
    AUDIO_SCEANRIO_TYPE_NO_USE    = 0x7FFFFFFF,        /**<  Dummy for DSP structure alignment */
} audio_scenario_type_t;

typedef struct {
    uint32_t mcu_clock_enable;
    uint32_t dsp_clock_used;
} audio_clock_share_buffer_t, *audio_clock_share_buffer_p;

/** @brief Audio clock setting */
typedef enum {
    /* clock */
    AUDIO_CLOCK_INT        = 0,                                /**< Audio Clock for Audio internal bus */
    AUDIO_CLOCK_ENGINE     = 1,                                /**< Audio Clock for Memory agent engine */
    AUDIO_CLOCK_GPSRC      = 2,                                /**< Audio Clock for HWSRC */
    AUDIO_CLOCK_UPLINK     = 3,                                /**< Audio Clock for DL hi-res */
    AUDIO_CLOCK_DWLINK     = 4,                                /**< Audio Clock for UL hi-res */
    AUDIO_CLOCK_SPDIF      = 5,                                /**< Audio Clock for SPDIF */
    AUDIO_CLOCK_INTF0_IN   = 6,                                /**< Audio Clock for I2S IN APLL2 48KHz, APLL2:49.152M */
    AUDIO_CLOCK_INTF1_IN   = 7,                                /**< Audio Clock for I2S IN APLL1 44.1KHz, APLL1:45.1584MHZ */
    AUDIO_CLOCK_INTF0_OUT  = 8,                                /**< Audio Clock for I2S OUT APLL2 48KHz, APLL2:49.152M */
    AUDIO_CLOCK_INTF1_OUT  = 9,                                /**< Audio Clock for I2S OUT APLL1 44.1KHz, APLL1:45.1584MHZ */
    AUDIO_CLOCK_TEST       = 10,                               /**< Audio Clock for TEST */
    AUDIO_CLOCK_ANC        = 11,                               /**< Audio Clock for ANC */
    AUDIO_CLOCK_CLD        = 12,                               /**< Audio Clock for CLD */
    AUDIO_CLOCK_VOW        = 13,                               /**< Audio Clock for VOW */
    /* power */
    AUDIO_POWER_MICBIAS_0_HP  = 14,                            /**< Audio Power for Micbias 1 high performance mode*/
    AUDIO_POWER_MICBIAS_1_HP  = 15,                            /**< Audio Power for Micbias 2 high performance mode*/
    AUDIO_POWER_MICBIAS_2_HP  = 16,                            /**< Audio Power for Micbias 3 high performance mode*/
    AUDIO_POWER_MICBIAS_0_NM  = 17,                            /**< Audio Power for Micbias 1 normal mode*/
    AUDIO_POWER_MICBIAS_1_NM  = 18,                            /**< Audio Power for Micbias 2 normal mode*/
    AUDIO_POWER_MICBIAS_2_NM  = 19,                            /**< Audio Power for Micbias 3 normal mode*/
    AUDIO_POWER_MICBIAS_0_LP  = 20,                            /**< Audio Power for Micbias 1 low power mode*/
    AUDIO_POWER_MICBIAS_1_LP  = 21,                            /**< Audio Power for Micbias 2 low power mode*/
    AUDIO_POWER_MICBIAS_2_LP  = 22,                            /**< Audio Power for Micbias 3 low power mode*/
    AUDIO_POWER_MICBIAS_END = AUDIO_POWER_MICBIAS_2_LP,        /**< Audio Power for Micbias End */
    AUDIO_POWER_MICBIAS_SHARE = 23,                            /**< Audio Power for Micbias Share LDO0 */
    AUDIO_POWER_DAC            ,                               /**< Audio Power for DAC Out(vitual), for amp lock */
    AUDIO_POWER_I2S            ,                               /**< Audio Power for I2S MST Out(vitual), for amp lock */
    /* SPM STATE */
    AUDIO_DSP_SPM_STATE1       ,                               /**< Audio DSP Low power state1, dsp can't sleep */
    AUDIO_DSP_SPM_STATE3       ,                               /**< Audio DSP Low power state3, audio won't use, reserved */
    AUDIO_DSP_SPM_STATE4       ,                               /**< Audio DSP Low power state4, dsp can sleep */
    AUDIO_POWER_END            ,                               /**< Audio Power End Number */
    AUDIO_CLOCK_MAX = 0xFFFFFFFF,                              /**< Audio Clock Max Number */
} audio_clock_setting_type_t;

typedef enum {
    /* sub agent */
    HAL_AUDIO_SUB_AGENT_MIN                      = 0,
    HAL_AUDIO_AFE_CLOCK_AFE                      = HAL_AUDIO_SUB_AGENT_MIN,
    HAL_AUDIO_AFE_CLOCK_I2S0                     = 1,
    HAL_AUDIO_AFE_CLOCK_I2S1                     = 2,
    HAL_AUDIO_AFE_CLOCK_I2S2                     = 3,
    HAL_AUDIO_AFE_CLOCK_I2S3                     = 4,
    HAL_AUDIO_AFE_CLOCK_22M                      = 5,
    HAL_AUDIO_AFE_CLOCK_24M                      = 6,
    HAL_AUDIO_AFE_CLOCK_APLL                     = 7,
    HAL_AUDIO_AFE_CLOCK_APLL2                    = 8,
    HAL_AUDIO_AFE_CLOCK_ADC_COMMON               = 9,
    HAL_AUDIO_AFE_CLOCK_ADC23                    = 10,
    HAL_AUDIO_AFE_CLOCK_ADC45                    = 11,
    HAL_AUDIO_AFE_CLOCK_ANC                      = 12,
    HAL_AUDIO_AFE_CLOCK_ADC_HIRES                = 13,
    HAL_AUDIO_AFE_CLOCK_DAC                      = 14,
    HAL_AUDIO_AFE_CLOCK_DAC_HIRES                = 15,
    HAL_AUDIO_AFE_CLOCK_I2S_SLV_HCLK             = 16,
    HAL_AUDIO_AFE_CONTROL_ADDA                   = 17,
    HAL_AUDIO_AFE_CLOCK_SRC_COMMON               = 18,
    HAL_AUDIO_AFE_CLOCK_SRC1                     = 19,
    HAL_AUDIO_AFE_CLOCK_SRC2                     = 20,

    HAL_AUDIO_SUB_AGENT_NUMBERS                  = 21,
    HAL_AUDIO_SUB_AGENT_ERROR                    = 0xFFFFFFFF,
} hal_audio_sub_agent_t;

/* Open message parameter structure */
typedef struct {
    mcu2dsp_param_t                 param;
    audio_scenario_type_t           audio_scenario_type;
    mcu2dsp_open_stream_in_param_t  stream_in_param;
    mcu2dsp_open_stream_out_param_t stream_out_param;
} mcu2dsp_open_param_t, *mcu2dsp_open_param_p;

typedef struct {
    bool                        mce_flag;
    bool                        aws_sync_request;
    uint32_t                    aws_sync_time;
    bool                        aws_flag;
} audio_dsp_afe_start_param_t, *audio_dsp_afe_start_param_p;

typedef union {
    audio_dsp_a2dp_dl_start_param_t a2dp;
    audio_dsp_afe_start_param_t     afe;
} mcu2dsp_start_stream_in_param_t, *mcu2dsp_start_stream_in_param_p;

typedef union {
    audio_dsp_afe_start_param_t     afe;
} mcu2dsp_start_stream_out_param_t, *mcu2dsp_start_stream_out_param_p;

/* Start message parameter structure */
typedef struct {
    mcu2dsp_param_t                     param;
    mcu2dsp_start_stream_in_param_t     stream_in_param;
    mcu2dsp_start_stream_out_param_t    stream_out_param;
} mcu2dsp_start_param_t, *mcu2dsp_start_param_p;

typedef enum {
    AUDIO_STRAM_DEINIT_ALL = 0,
    AUDIO_STRAM_DEINIT_VOICE_AEC,
    AUDIO_STRAM_DEINIT_VOICE_CPD,
    AUDIO_STRAM_DEINIT_ANC_MONITOR,
    AUDIO_STRAM_DEINIT_ULL_DL,
} audio_stream_deinit_id_t;

/**
 *  @brief This struct defines Audio dsp sync parameter info.
 */
typedef enum {
    AUDIO_SYNC_GAIN_SELECT_CHANNEL_L = 0,
    AUDIO_SYNC_GAIN_SELECT_CHANNEL_R,
    AUDIO_SYNC_GAIN_SELECT_CHANNEL_DUAL,
    AUDIO_SYNC_MAX = 0xFFFFFFFF
} gain_select_t;

typedef struct {
    uint32_t gain;
    gain_select_t gain_select;
} vol_gain_info_t;

typedef struct {
    uint32_t gpt_count;             // the target gpt timer count of sync
    uint32_t *nvkey_addr;            // share buffer addr of updated nvkey
    vol_gain_info_t vol_gain_info;  // downlink volume level
} cm4_dsp_audio_sync_request_param_t;

/**
 *  @brief This struct defines action sync mechanism between L and R detail info.
 */
typedef enum {
    MCU2DSP_SYNC_REQUEST_START      = 0,
    MCU2DSP_SYNC_REQUEST_STOP       = 1,
    MCU2DSP_SYNC_REQUEST_SET_VOLUME = 2,
    MCU2DSP_SYNC_REQUEST_SET_MUTE   = 3,
    MCU2DSP_SYNC_REQUEST_FADE_OUT   = 4,
    MCU2DSP_SYNC_REQUEST_SET_RX_EQ  = 5,
    MCU2DSP_SYNC_REQUEST_ACTION_MAX = 0xFFFFFFFF
} cm4_dsp_audio_sync_action_type_t;

typedef enum {
    MCU2DSP_SYNC_REQUEST_HFP  = 0,
    MCU2DSP_SYNC_REQUEST_A2DP = 1,
    MCU2DSP_SYNC_REQUEST_ANC  = 2,
    MCU2DSP_SYNC_REQUEST_VP   = 3,
    MCU2DSP_SYNC_REQUEST_ADAPT_ANC = 4,
    MCU2DSP_SYNC_REQUEST_BLE  = 5,
    MCU2DSP_SYNC_REQUEST_LLF        = 6,
    MCU2DSP_SYNC_REQUEST_SCENARIO_MAX = 0xFFFFFFFF
} cm4_dsp_audio_sync_scenario_type_t;




/**
 *  @brief This struct defines the share buffer inforation for 3rd party audio platform integration.
 */
 typedef enum {
    AUDIO_PLATFORM_VOICE_MIC0 = 0,
    AUDIO_PLATFORM_VOICE_MIC1,
    AUDIO_PLATFORM_FF_MIC0,
    AUDIO_PLATFORM_FF_MIC1,
    AUDIO_PLATFORM_FB_MIC,
    AUDIO_PLATFORM_INPUT_NUMBER,
} mcu_dsp_audio_platform_input_selection;

 typedef enum {
    AUDIO_PLATFORM_OUTPUT_VP = 0,
    AUDIO_PLATFORM_OUTPUT_BTA,  //A2DP or HFP or Le Audio
    AUDIO_PLATFORM_OUTPUT_ULL,
    AUDIO_PLATFORM_OUTPUT_LINEIN,
    AUDIO_PLATFORM_OUTPUT_USB,

    AUDIO_PLATFORM_OUTPUT_UNUSED,
    AUDIO_PLATFORM_OUTPUT_NUMBER,
} mcu_dsp_audio_platform_output_selection;

typedef struct {
    //Write to DSP
    uint16_t use_case;
    int16_t mic_gain[AUDIO_PLATFORM_INPUT_NUMBER];
    int16_t downlink_gain[AUDIO_PLATFORM_OUTPUT_NUMBER];
    uint16_t quaternions[4];
    uint16_t clear_howling_flag;


    //Read from DSP
    uint16_t layout_ersion;
    uint16_t howling_flag;
    uint16_t rms_value[AUDIO_PLATFORM_INPUT_NUMBER];
} mcu_dsp_audio_platform_share_buffer_info_t;

#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
typedef void (*audio_volume_monitor_callback_t)(int32_t *volume_data, uint32_t volume_len, void *user_data);

typedef struct audio_volume_monitor_node_t {
    /* header + payload */
    struct audio_volume_monitor_node_t *next;
    audio_scenario_type_t type;         /* owner */
    audio_volume_monitor_callback_t cb; /* owner's callback */
    void                  *user_data;   /* TBD */
    uint32_t              update_count; /* total update times */
    bool                  update_flag;  /* 1: already updated 0: not updated */
    uint32_t              ch;
    uint32_t              volume_len;   /* the size of volume_data, 20ms * len */
    int32_t               *volume_data; /* non-cacheable system ram */
} audio_volume_monitor_node_t;
#endif

#ifdef AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE
typedef struct
{
    hal_audio_hw_stream_out_index_t index;
    uint16_t new_step;
    uint16_t new_samples_per_step;
}dl_sw_gain_fade_para_t;

typedef struct
{
    uint8_t LR_balance_status;
    uint8_t LR_balance_ID;
}LR_balance_para_t;

typedef struct
{
    uint8_t enable_vp : 1;
    uint8_t enable_a2dp : 1;
    uint8_t enable_hfp : 1;
    uint8_t enable_ble_music : 1;
    uint8_t enable_ble_call : 1;
    uint8_t enable_ull_music : 1;
    uint8_t enable_ull_call : 1;
    uint8_t enable_line_in : 1;
    uint8_t enable_usb_in : 1;
    uint8_t enable_revert[2];
    LR_balance_para_t LR_balance_para_default;
    uint8_t max_gain_limiter;
    uint8_t revert[9];
    uint8_t ch_num;
    uint8_t level_num;
    int16_t offset_value[1];
}dl_sw_gain_default_para_t;

typedef enum {
    DL_SW_GAIN_SET_LR_BALANCE_OFFSET = 0,              /**< LR balance offset setting  */
    DL_SW_GAIN_SET_MAINGAIN_1,
    DL_SW_GAIN_SET_MAINGAIN_2,
    DL_SW_GAIN_SET_MAINGAIN_3,
    DL_SW_GAIN_SET_MAINGAIN_1_FADE_IN_PARAM,
    DL_SW_GAIN_SET_MAINGAIN_1_FADE_OUT_PARAM,
    DL_SW_GAIN_SET_MAINGAIN_2_FADE_IN_PARAM,
    DL_SW_GAIN_SET_MAINGAIN_2_FADE_OUT_PARAM,
    DL_SW_GAIN_SET_MAINGAIN_3_FADE_IN_PARAM,
    DL_SW_GAIN_SET_MAINGAIN_3_FADE_OUT_PARAM,
    DL_SW_GAIN_SET_ID_MAX            = 0xFFFFFFFF,     /**< For dsp structure alignment */
} dl_sw_gain_setting_command_id_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /*__HAL_AUDIO_MESSAGE_STRUCT_COMMON_H__ */
