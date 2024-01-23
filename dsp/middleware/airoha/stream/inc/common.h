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

#ifndef __COMMON_H__
#define __COMMON_H__

/*!
 *@file   common.h
 *@brief  defines common definition  of streaming
 *
 @verbatim
 @endverbatim
 */

#include "types.h"
#include "dsp_task.h"
#include "transform_.h"
#include "source_.h"
#include "dsp_drv_dfe.h"
#include "hal_audio_afe_define.h"
#include "hal_audio_afe_control.h"
#include "audio_afe_common.h"
#include "dsp_temp.h"
#include "hal_audio_message_struct_common.h"

////////////////////////////////////////////////////////////////////////////////
// Constant Definitions ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#ifdef AIR_FULL_ADAPTIVE_ANC_STEREO_ENABLE
#define BUFFER_INFO_CH_NUM      10
#else
#ifdef AIR_HW_VIVID_PT_STEREO_ENABLE
#define BUFFER_INFO_CH_NUM      7
#else
#define BUFFER_INFO_CH_NUM      5
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function Declarations ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
typedef enum {
    BUFFER_TYPE_CIRCULAR_BUFFER     = (U8)(0UL),
    BUFFER_TYPE_QUEUE               = (U8)(1UL),
    BUFFER_TYPE_DIRECT_BUFFER       = (U8)(2UL),
    BUFFER_TYPE_INTERLEAVED_BUFFER  = (U8)(3UL),
    BUFFER_TYPE_OTHER               = (U8)(4UL),
} BUFFER_TYPE;


typedef struct {
    U8    *startaddr[BUFFER_INFO_CH_NUM];
    U32    ReadOffset;
    U32    WriteOffset;
    U16    channelSelected;
    U32    length;
    BOOL   bBufferIsFull;
} BUFFER_INFO, *BUFFER_INFO_PTR;

#define BT_AWS_MCE_ROLE_NONE                0x00          /**< No Role. */
#define BT_AWS_MCE_ROLE_CLINET              0x10          /**< Client Role. */
#define BT_AWS_MCE_ROLE_PARTNER             0x20          /**< Partner Role. */
#define BT_AWS_MCE_ROLE_AGENT               0x40          /**< Agent Role. */

#define INFO_STATUS_USED                    0x01
#define INFO_STATUS_FREE                    0x10

typedef struct {
    U8*  addr;
    U32  length;
} MEMBLK;

typedef union {
    BUFFER_INFO         BufferInfo;

    n9_dsp_share_info_t   ShareBufferInfo;

    avm_share_buf_info_t  AVMBufferInfo;

} STREAM_BUFFER;

typedef struct {
    U16 cid;
    U16 connhandle;
    U32 id_ext;
    SOURCE rfcomm;
    VOID *handler;
    U8 is_rfcomm;
} L2CAP_SOURCE_PARAMETER;

typedef struct {
    U8 session_idx;
    U8 dlci;
    U8 rx_credit;
    U8 isFC;
    U32 id_ext;
    VOID *handler;
    BOOL(*MoreSpaceCB)(SOURCE);
} RFCOMM_SOURCE_PARAMETER;

typedef struct {
    U16 dcid;
    U16 connhandle;
    VOID *handler;
    U32 id_ext;
    U16 mtu;
    U16 cid;
} L2CAP_SINK_PARAMETER;

typedef struct {
    U8 session_idx;
    U8 dlci;
    U8 tx_credit;
    U8 isFC;
    VOID *handler;
    U16 mfs;
    BOOL creditFail;
    BOOL L2Fail;
} RFCOMM_SINK_PARAMETER;

typedef struct {
    struct SCO_PARA_CTRL_s {
        U32 codec_type             : 2; // mSBC,CVSD,transparent
        U32 data_length            : 16;
        U32 RESERVE                : 14; // reserved
    } field;
    U8 *sbc_codec_param_ptr;
    S16 mSBC_frame_cnt;
} SCO_PARAMETER;

typedef struct {
    U16 process_data_length;
    U16 frame_length;
#ifdef MTK_BT_HFP_FORWARDER_ENABLE
    avm_share_buf_info_ptr share_info_base_addr;
#else
    n9_dsp_share_info_ptr share_info_base_addr;
#endif
    BOOL IsFirstIRQ;
    BOOL dl_enable_ul;
    U16 write_offset_advance;
    U32 ul_play_gpt;
#ifdef MTK_BT_HFP_FORWARDER_ENABLE
    bool rx_forwarder_en;
    bool tx_forwarder_en;
#endif
#ifdef DEBUG_ESCO_PLK_LOST
    U32 lost_pkt_num;
    U32 forwarder_pkt_num;
#endif
    BOOL dl_reinit;
    BOOL ul_reinit;
    U8 out_channel_num;
} N9SCO_PARAMETER;

#ifdef AIR_BT_CODEC_BLE_ENABLED

#define BT_BLE_CODEC_NONE      (0x00)               /**<  None. */
#define BT_BLE_CODEC_LC3       (0x01)               /**<  LC3 codec. */
#define BT_BLE_CODEC_LC3PLUS   (0x02)               /**<  LC3PLUS codec. */
#define BT_BLE_CODEC_VENDOR    (0x03)               /**<  Vendor codec. */
#define BT_BLE_CODEC_ULD       (0x04)               /**<  ULD codec. */


#define CODEC_AUDIO_MODE    0x0         /* For music play */
#define CODEC_VOICE_MODE    0x1         /* For phone call */
#define CODEC_LL_MODE       0x2

#define SINGLE_CIS_MODE 0x0
#define DUAL_CIS_MODE   0x1

#define SINGLE_CIS_MODE 0x0
#define DUAL_CIS_MODE   0x1

typedef enum {
    DUAL_CIS_DISABLED      = 0,
    DUAL_CIS_WAITING_MAIN  = 1,
    DUAL_CIS_WAITING_SUB   = 2,
    DUAL_CIS_BOTH_ENABLED  = 3,
} dual_cis_status;

typedef enum {
    SUB_CIS_CHANNEL_L      = 0,
    SUB_CIS_CHANNEL_R      = 1,
} dual_cis_channel_status;

typedef enum {
    BLE_CONTEXT_PROHIBITED       = 0x0000,
    BLE_CONTEXT_UNSPECIFIED      = 0x0001,
    BLE_CONTEXT_CONVERSATIONAL   = 0x0002,
    BLE_CONTEXT_MEDIA            = 0x0004,
    BLE_CONTEXT_GAME             = 0x0008,
    BLE_CONTEXT_INSTRUCTIONAL    = 0x0010,
    BLE_CONTEXT_VOICE_ASSISTANTS = 0x0020,
    BLE_CONTEXT_LIVE             = 0x0040,
    BLE_CONTEXT_SOUND_EFFECTS    = 0x0080,
    BLE_CONTEXT_NOTIFICATIONS    = 0x0100,
    BLE_CONTEXT_RINGTONE         = 0x0200,
    BLE_CONTEXT_ALERTS           = 0x0400,
    BLE_CONTEXT_EMERGENCY_ALARM  = 0x0800,
    BLE_CONTENT_TYPE_ULL_BLE     = 0x1234,  /**< AIROHA proprietary ULL BLE */
    BLE_CONTENT_TYPE_WIRELESS_MIC= 0x5678,  /**< AIROHA proprietary Wireless mic.*/
} bt_ble_context_type_t;

typedef struct {
    U16 process_number; /* the process package number */
    U16 frame_length; /* payload size */
    n9_dsp_share_info_ptr share_info_base_addr;
    BOOL IsFirstIRQ;
    BOOL skip_after_drop;
    BOOL IsPlayInfoReceived;
    BOOL IsSubPlayInfoReceived;
    U8  pkt_lost_cnt[2];
    U8  dl_afe_skip_time;
    U16 write_offset_advance;
    U32 ul_play_gpt; /* record the time to trigger UL start up */
    dual_cis_status dual_cis_status;
    dual_cis_channel_status  dual_cis_sub_channel_sel;
    U32  dual_cis_buffer_offset;
    n9_dsp_share_info_ptr sub_share_info_base_addr;
    U32 context_type;
    U32 sampling_rate;
    U32 seq_num; /* total sequence number of received frame */
    U32 predict_timestamp;
    U16 predict_packet_cnt[2];
    U32 frame_interval;   /* unit: us   (7500 = 7.5ms) */
    U32 ret_window_len; /* unit :us */
    U16 iso_interval; /* unit :us */
    U8  ft;
    U8  seq_miss_cnt;
    U8  frame_per_iso;
    U8  predict_frame_counter;
    U8  codec_type;
    U8  plc_state_len;
    U32 underrun_recovery_size;
    U32 crc_init;
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
    BOOL dl_reinit;
    BOOL ul_reinit;
#endif
    U8  out_channel;
    BOOL is_lightmode;
    U16 dummy_insert_number;
    U16 dummy_process_status;
    U16 prefill_us;
} N9BLE_PARAMETER;

#endif // #ifdef AIR_BT_CODEC_BLE_ENABLED


/**
 *  @brief This structure defines the sink CM4 record information.
 */
typedef struct{
    n9_dsp_share_info_ptr share_info_base_addr;
    U16 process_data_length;
    U16 frame_length;
    U32 frames_per_message;
    DSP_ENC_BITRATE_t bitrate;
    bool interleave;
} CM4_RECORD_PARAMETER;

typedef struct {
    U8 *addr;
    U32 length;
} REGION_PARAMETER;

typedef struct {
    U8  channel_sel;
    U16 frame_size;
    STREAM_SRC_VDM_PTR_t src;
    U8  channel_num;
    TRANSFORM transform;
} DSP_PARAMETER;

typedef struct {
    U8  channel_sel;
    U16 frame_size;
    STREAM_SRC_VDM_PTR_t src;
    U8  channel_num;
    U8  HW_type;
    VOID *handler;
    hal_audio_format_t format;
    uint32_t rate;                     /* rate in Hz       */
    uint32_t src_rate;
    uint32_t count;                    /* trigger periods  */
    uint32_t period;                     /* delay in ms      */
    uint32_t buffer_size;              /* buffer size      */
    uint32_t format_bytes;
    uint32_t sram_empty_fill_size;
    afe_block_t AfeBlkControl;         // for interleaved
    afe_stream_channel_t connect_channel_type;
    bool is_pcm_ul1_open;
    bool irq_compen_flag;
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    uint16_t                         Frame_Size;
    uint8_t                          Buffer_Frame_Num;
#endif
    hal_audio_device_t               audio_device;
    hal_audio_device_t               audio_device1;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    hal_audio_device_t               audio_device2;
    hal_audio_device_t               audio_device3;
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    hal_audio_device_t               audio_device4;
    hal_audio_device_t               audio_device5;
    hal_audio_device_t               audio_device6;
    hal_audio_device_t               audio_device7;
#endif
#endif
    hal_audio_channel_selection_t    stream_channel;
    hal_audio_memory_t               memory;
    hal_audio_interface_t            audio_interface;
    hal_audio_interface_t            audio_interface1;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    hal_audio_interface_t            audio_interface2;
    hal_audio_interface_t            audio_interface3;
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    hal_audio_interface_t            audio_interface4;
    hal_audio_interface_t            audio_interface5;
    hal_audio_interface_t            audio_interface6;
    hal_audio_interface_t            audio_interface7;
#endif
#endif
    uint32_t audio_device_rate[8];
    uint32_t audio_memory_rate[8];

    audio_pcm_ops_p                         ops;
    uint32_t                                misc_parms;
    bool     hw_gain;
    bool     echo_reference;
    bool     irq_exist;
    U8       afe_wait_play_en_cnt;
    bool     aws_sync_request;
    uint32_t aws_sync_time;
    uint16_t pop_noise_pkt_num;
    bool     mute_flag;

    bool with_sink_src;
    bool     linein_scenario_flag;
    bool     lineout_scenario_flag;
    bool     is_memory_start;
#ifdef HAL_AUDIO_ENABLE_PATH_MEM_DEVICE
    //modify for ab1568
    //for hal_audio_set_path
    hal_audio_path_parameter_t path_handle;
    //for hal_audio_set_memory
    hal_audio_memory_parameter_t mem_handle;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    hal_audio_memory_parameter_t mem_handle1;
    hal_audio_memory_parameter_t mem_handle2;
#endif
    //for hal_audio_set_device
    hal_audio_device_parameter_t device_handle;
    hal_audio_device_parameter_t device_handle1;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    hal_audio_device_parameter_t device_handle2;
    hal_audio_device_parameter_t device_handle3;
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    hal_audio_device_parameter_t device_handle4;
    hal_audio_device_parameter_t device_handle5;
    hal_audio_device_parameter_t device_handle6;
    hal_audio_device_parameter_t device_handle7;
#endif
#endif
#endif
    uint8_t sw_channels;
#ifdef AIR_HFP_DNN_PATH_ENABLE
    bool enable_ul_dnn;
#endif
    uint32_t scenario_id;
    uint32_t scenario_sub_id;
    bool     drop_redundant_data_at_first_time;
    clkskew_mode_t clk_skew_mode;
    hal_audio_afe_hwsrc_type_t hwsrc_type;
} AUDIO_PARAMETER;

typedef struct {
    U8 tone;
    U8 CycleNum;
    S16 volume_start;
    S16 volume_step;
    S16 volume_max;
} RT_INFO_CTL_STRU;

typedef union VPRT_PARA_CTRL_u {
    RT_INFO_CTL_STRU RTInfo;
    U32 Pattern_Length;
} VPRT_PARA_CTRL_t;

typedef struct {
    U8  mode;
    U16 PatternNum;
    VPRT_PARA_CTRL_t para;
} VPRT_PARAMETER;


typedef struct {
    U8  channel_sel;
    U8  sampling_rate;
    U16 frame_size;
    U8  resolution;
} USBCLASS_PARAMETER;

typedef struct {
    U32 offset;
    U8 *ptr;
} AUDIO_QUEUE_PARAMETER;

typedef struct {
    U32 readOffset;
    U32 totalFrameCnt;
    U32 dropFrameCnt;
    U8 samplingFreq;
    U16 inSize;
} A2DP_PARAMETER;

typedef struct {
    VOID (*entry)(U8 *ptr, U32 length);
    U32 mem_size;
    U32 data_size;
    U32 data_samplingrate;
    U32 default_data_size;
    U32 default_data_samplingrate;
    VOID *handler;
    int8_t user_count;
    U8 channel_num;
    bool is_processed;
    bool is_dummy_data;
    VIRTUAL_SOURCE_TYPE virtual_source_type;
} AUDIO_VIRTUAL_PARAMETER;

typedef struct {
    U32 max_output_length;
    U32 remain_len;
    U8 temp4copy;
    U8 memory_type;
    VOID *handler;
} MEMORY_PARAMETER;

/* N9 A2DP parameter structure */
/**
 *  @brief Define for A2DP codec type.
 */
#define BT_A2DP_CODEC_SBC      (0)           /**< SBC codec. */
#define BT_A2DP_CODEC_AAC      (2)           /**< AAC codec. */
#define BT_A2DP_CODEC_AIRO_CELT (0x65)        /**< Airoha CELT codec. */
#define BT_A2DP_CODEC_VENDOR   (0xFF)           /**< VENDOR codec. */
#define BT_A2DP_CODEC_VENDOR_2 (0xA5)           /**< VENDOR codec. */
#define BT_HFP_CODEC_CVSD  (1)           /**< SBC codec. */
#define BT_HFP_CODEC_mSBC  (2)           /**< AAC codec. */

#define BT_A2DP_CODEC_LHDC_CODEC_ID     (0x4C35)
#define BT_A2DP_CODEC_LC3PLUS_CODEC_ID  (0x0001)

typedef U8 bt_a2dp_codec_type_t;    /**< The type of A2DP codec. */

/**
 *  @brief Define for A2DP role type.
 */

#define BT_A2DP_SOURCE          (0)    /**< SRC role. */
#define BT_A2DP_SINK            (1)    /**< SNK role. */
#define BT_A2DP_SOURCE_AND_SINK (2)    /**< Both roles for a single device (series case). */
#define BT_A2DP_INVALID_ROLE    (0xFF) /**< Invalid role. */
typedef U8 bt_a2dp_role_t;        /**< The type of A2DP role. */

/**
 *  @brief This structure defines the SBC codec details.
 */
typedef struct {
    U8 min_bit_pool;       /**< The minimum bit pool. */
    U8 max_bit_pool;       /**< The maximum bit pool. */
    U8 block_length;       /**< b0: 16, b1: 12, b2: 8, b3: 4. */
    U8 subband_num;        /**< b0: 8, b1: 4. */
    U8 alloc_method;       /**< b0: loudness, b1: SNR. */
    U8 sample_rate;        /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    U8 channel_mode;       /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
} bt_codec_sbc_t;

/**
 *  @brief This structure defines the AAC codec details.
 */
typedef struct {
    BOOL vbr;              /**< Indicates if VBR is supported or not. */
    BOOL drc;              /**< Indicates if DRC is supported or not. */
    U8 object_type;        /**< b4: M4-scalable, b5: M4-LTP, b6: M4-LC, b7: M2-LC. */
    U8 channels;           /**< b0: 2, b1: 1. */
    U16 sample_rate;       /**< b0~b11: 96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000. */
    U32 bit_rate;          /**< Constant/peak bits per second in 23 bit UiMsbf. A value of 0 indicates that the bit rate is unknown. */
} bt_codec_aac_t;

typedef struct {
    uint16_t codec_id;
    uint8_t channels;           /**< b0: 2, b1: 1. */
    uint8_t sample_rate;       /**< b0~b11: 96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000. */
    uint32_t vendor_id;          /**< Constant/peak bits per second in 23 bit UiMsbf. A value of 0 indicates that the bit rate is unknown. */
    uint8_t duration_resolution; /**< b4~b6: 2.5ms, 5ms, 10ms, bits per sample b0: 32bit, b1: 24bit. b2: 16bit.*/
    BOOL is_raw_mode;           /**< Indicate raw mode. 1: enable, 0:disable. */
    BOOL is_low_latency;        /**< 1: low latency 0: normal latency. */
    BOOL is_lossless_mode;      /**< 1: lossless mode 0: lossy mode. */
} bt_codec_vendor_t;



/**
 *  @brief This union defines the A2DP codec.
 */
typedef union {
    bt_codec_sbc_t sbc;    /**< SBC codec. */
    bt_codec_aac_t aac;    /**< AAC codec. */
    bt_codec_vendor_t vend;  /**< vend codec. */
} bt_codec_t;

/**
 *  @brief This structure defines the A2DP codec capability.
 */
typedef struct {
    bt_a2dp_codec_type_t type;  /**< Codec type. */
    bt_codec_t codec;           /**< Codec information. */
} bt_codec_capability_t;

/** @brief This structure defines the A2DP codec. */
typedef struct {
    bt_codec_capability_t codec_cap;    /**< The capabilities of Bluetooth codec */
    bt_a2dp_role_t             role;    /**< The Bluetooth codec roles */
} bt_codec_a2dp_audio_t;

// typedef struct {
// bt_a2dp_codec_type_t type;  /**< Codec type. */
// bt_codec_t codec;           /**< Codec information. */
// } bt_codec_capability_t;

// /** @brief This structure defines the A2DP codec. */
// typedef struct {
// bt_codec_capability_t codec_cap;    /**< The capabilities of Bluetooth codec */
// bt_a2dp_role_t             role;    /**< The Bluetooth codec roles */
// } bt_codec_a2dp_audio_t;
typedef struct {
    U16  a2dp_report_cnt;    /**< The capabilities of Bluetooth codec */
    U16  a2dp_accumulate_cnt;    /**< The Bluetooth codec roles */
    U32 *p_a2dp_bitrate_report;
} bt_a2dp_bitrate_report;

typedef enum {
    reinit_state_normal_mode,
    reinit_state_observe_mode,
    reinit_state_trigger_mode,
} a2dp_reinit_state_machine;

#define MAX_IOS_SBC_LOST_PTK    (10)
typedef struct _bt_a2dp_lost_ptk_t {
    U32 u4SeqNo;
    U32 u4NumFrameToPad;
    U32 u4NumFramePadded;
    BOOL verified_flag;
} bt_a2dp_lost_ptk_t;

#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
typedef struct {
    U32 FirstAnchor;
    U32 MinAfeSinkSize;
    U32 AccumulatedSamples;
    U8  CurrentSeqNo;
    U8  CntIdx;
    U8  DebugSeqNo;
    BOOL Initialized;
    U32 AfeAnchor;
    U32 AfeAnchorIntra;
    U32 MuteCnt;
    U8 BypassDrop;
    U8 SeqNotMatchCnt;
} AIROHA_GAMING_CTRL, *AIROHA_GAMING_CTRL_PTR;

typedef struct {
    U8  SeqNo;
    U8  FrameDataBegin[1];
} AIROHA_GAMING_FRAME_TYPE, *AIROHA_GAMING_FRAME_PTR;

typedef struct {
    U8  FrameNo;
    U8  FrameSize;
    U16 CheckSum;
    U32 NewestAnchor;
    U8  FrameBegin[1];
} AIROHA_GAMING_BUFFER_TYPE, *AIROHA_GAMING_BUFFER_PTR;
#endif

typedef struct {
    bt_codec_a2dp_audio_t codec_info;
    BOOL cp_exist;
    BOOL mce_flag;/* 1: AWS MCE mode */
    BOOL fragment_flag;
    BOOL ios_aac_flag;
    U32  share_info_base_addr;
    U32  pkt_lost_report_state;
    U32  predict_timestamp;
    U32  timestamp_ratio;
    U32  predict_asi;
    U32  current_packet_size;
    U32  current_frame_size;
    U32  current_bitstream_size;
    U32  current_seq_num;
    U32  current_timestamp;
    U32  readOffset;/* frame offset, also store AFE buffer report in avm mode  */
    U32  totalFrameCnt;/* if fragmented, the number of remaining fragments, including current fragment.
                          if non-fragmented, the number of frames contained in this packet. */
    U32  *p_afe_buf_report;
    U32  dropFrameCnt;
    U32  current_asi;
    U32  *asi_buf;
    U32  *min_gap_buf;
    U32  prev_seq_num;
    U16  DspReportStartId;
    bt_a2dp_bitrate_report  a2dp_bitrate_report;
    U32  sink_latency;
    U16  buffer_empty_cnt;
    BOOL ios_sbc_flag;
    BOOL latency_monitor;
    BOOL alc_monitor;
    BOOL asi_sync_en;
    BOOL resend_reinit_req;
    U16 pkt_loss_seq_no;
    U16 padding_count;
    U16 u2OverPadFrameCnt;
    U32  *a2dp_lostnum_report; // U32 lostnum U32 current_ts
    volatile AUDIO_SYNC_INFO *sync_info;
    U32  *pcdc_info_buf;
    BOOL is_plc_frame;
    U32  plc_state_len;
#ifdef AIR_A2DP_REINIT_V2_ENABLE
    U32  reinit_request_cnt;
    bool reinit_request;
    bool Is_SpeDev;
    U32  reinit_observe_cnt;
    U32  rssi_bad_cnt;
    a2dp_reinit_state_machine reinit_state;
#endif
#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
    AIROHA_GAMING_BUFFER_PTR pkt_ptr;
    U8 FrameNo;
    U16 PktNewestSeqNo;
    U16 PktCnt;
#endif
} N9_A2DP_PARAMETER;

/* N9 A2DP parameter structure */

/** @brief Define audio sampling rate. */
typedef enum {
    AUDIO_SAMPLING_RATE_8KHZ      = 0, /**< 8000Hz  */
    AUDIO_SAMPLING_RATE_11_025KHZ = 1, /**< 11025Hz */
    AUDIO_SAMPLING_RATE_12KHZ     = 2, /**< 12000Hz */
    AUDIO_SAMPLING_RATE_16KHZ     = 3, /**< 16000Hz */
    AUDIO_SAMPLING_RATE_22_05KHZ  = 4, /**< 22050Hz */
    AUDIO_SAMPLING_RATE_24KHZ     = 5, /**< 24000Hz */
    AUDIO_SAMPLING_RATE_32KHZ     = 6, /**< 32000Hz */
    AUDIO_SAMPLING_RATE_44_1KHZ   = 7, /**< 44100Hz */
    AUDIO_SAMPLING_RATE_48KHZ     = 8, /**< 48000Hz */
    AUDIO_SAMPLING_RATE_96KHZ     = 9  /**< 96000Hz */
} audio_sampling_rate_t;

/** @brief Define the number of bits per second (bps) to stream audio data. */
typedef enum {
    AUDIO_BITS_PER_SAMPLING_16    = 0, /**< 16 bps */
    AUDIO_BITS_PER_SAMPLING_24    = 1  /**< 24 bps */
} audio_bits_per_sample_t;

/** @brief audio channel number define */
typedef enum {
    AUDIO_MONO                  = 0, /**< A single channel.  */
    AUDIO_STEREO                = 1, /**< Two channels. */
    AUDIO_STEREO_BOTH_L_CHANNEL = 2, /**< Two channels, but only output L channel. That is (L, R) -> (L, L). */
    AUDIO_STEREO_BOTH_R_CHANNEL = 3, /**< Two channels, but only output R channel. That is (L, R) -> (R, R). */
    AUDIO_STEREO_BOTH_L_R_SWAP  = 4  /**< Two channels, L and R channels are swapped. That is (L, R) -> (R, L). */
} audio_channel_number_t;

typedef struct {
    SINK attSINK;
    U16 attTxHandle;
    U8 linkIdx;
} AIRAPP_SINK_PARAMETER;

/** @brief audio channel number define */

typedef enum {
    AUDIO_INOUT_ENABLE           = 0,
    AUDIO_OUTPUT_ONLY            = 1,
    AUDIO_INPUT_ONLY             = 2,
    AUDIO_INOUT_MUTE             = 3,
} audio_inout_mute_ctrl_t;


/**
 *  @brief This structure defines the extracted CM4 playback share information.
 */
typedef struct {
    audio_bits_per_sample_t  bit_type;
    U32 sampling_rate;
    audio_channel_number_t channel_number;
    U8  codec_type;
    U8  source_channels;
    U32 share_info_base_addr;
} cm4_playback_info_t;


/**
 *  @brief This structure defines the source CM4 playback information.
 */
typedef struct {
    cm4_playback_info_t info;
    U32  current_frame_size;
    U32  remain_bs_size;
    U8   data_request;
    U8   data_refill_request;
} CM4_PLAYBACK_PARAMETER;

typedef struct {
    DSP_ENC_BITRATE_t bitrate;
    U8   first_packet;
    U8   payload_count; // frame count
} data_transl_param_t;

typedef struct {
    /* last timestemp */
    uint32_t last_timestamp;
    /* actual total time */
    uint32_t act_time;
    /* actual times */
    uint32_t act_count;
    /* actual total diff */
    int32_t act_diff;
    /* average diff */
    float ave_diff;
    /* compensatory samples */
    int32_t compen_samples;
} usb_in_clk_skew, bt_out_clk_skew;

typedef struct {
    /* timer handler */
    uint32_t handle;
    /* timer period */
    uint32_t period;
    /* timer callback */
    hal_gpt_callback_t callback;
    /* timer timestemp */
    uint32_t timestamp;
} timer_param;

typedef struct {
    /* codec type */
    audio_dsp_codec_type_t codec_type;
    /* codec parameters */
    audio_codec_param_t codec_param;
    /* receive data period, uint: us */
    uint32_t data_period;
    /* receive data timestemp */
    uint32_t data_timestamp;
    /* timer parameters */
    timer_param timer;
    /* clock skew */
    usb_in_clk_skew clk_skew;
    /* the frames has been processed */
    uint32_t process_frames;
    /* fetch flag */
    uint32_t fetch_flag;
    /* application handle */
    void *handle;
} usb_in_parameter;

typedef struct {
    /* codec type */
    audio_dsp_codec_type_t codec_type;
    /* codec parameters */
    audio_codec_param_t codec_param;
    /* send data period, uint: us */
    uint32_t data_period;
    /* send data timestemp */
    uint32_t data_timestamp;
    /* event status */
    uint32_t event_status;
    /* event timestemp */
    uint32_t event_timestamp;
    /* event fail count */
    uint32_t event_failcount;
    /* timer parameters */
    timer_param timer;
    /* clock skew */
    bt_out_clk_skew clk_skew;
} bt_out_parameter;

typedef union {
    usb_in_parameter usb_in_param;
    bt_out_parameter bt_out_param;
} usb_in_broadcast_param_t;

typedef struct {
    /* codec type */
    audio_dsp_codec_type_t codec_type;
    /* codec parameters */
    audio_codec_param_t codec_param;
    /* receive data period, uint: us */
    uint32_t data_period;
    /* receive data timestemp */
    uint32_t data_timestamp;
    /* event status */
    uint32_t event_status;
    /* event timestemp */
    uint32_t event_timestamp;
    /* event fail count */
    uint32_t event_failcount;
    /* timer parameters */
    timer_param timer;
    /* the frames has been processed */
    uint32_t process_frames;
    /* the frames needed to be dropped */
    uint32_t drop_frames;
    /* fetch flag */
    uint32_t fetch_flag;
    /* application handle */
    void *handle;
} bt_in_parameter;

typedef struct {
    /* codec type */
    audio_dsp_codec_type_t codec_type;
    /* codec parameters */
    audio_codec_param_t codec_param;
    /* send data period, uint: us */
    uint32_t data_period;
    /* send data timestemp */
    uint32_t data_timestamp;
    /* timer parameters */
    timer_param timer;
} usb_out_parameter;

typedef union {
    bt_in_parameter     bt_in_param;
    usb_out_parameter   usb_out_param;
} usb_out_broadcast_param_t;

typedef struct {
    U8   seq_num;
    U8   frame_size;
    U16  checksum;
    U8   process_frame_num;
    bool ul_process_done;
    U8   dl_irq_cnt;
} game_headset_voice_param_t;

typedef struct {
    usb_in_parameter usb_in_param;
    bool is_use_afe_dl3;
    bool is_dummy_data;
    bool is_afe_irq_comming;
} usb_in_local_param_t;

typedef struct {
    audio_dsp_codec_type_t codec_type;
    audio_codec_param_t codec_param;
    void *handle;
    bool is_with_ecnr;
    bool is_with_swb;
} usb_out_local_param_t;

typedef struct {
    audio_dsp_codec_type_t codec_type;
    audio_codec_param_t codec_param;
    void *handle;
} line_in_local_param_t;

typedef struct {
    audio_dsp_codec_type_t codec_type;
    audio_codec_param_t codec_param;
    void *handle;
} line_out_local_param_t;

typedef union {
    uint32_t reserved;
    /* For scenario specific struct define */
    data_transl_param_t data_transl_param;
    usb_in_broadcast_param_t usb_in_broadcast_param;
    usb_out_broadcast_param_t usb_out_broadcast_param;
    game_headset_voice_param_t voice_param;
    usb_in_local_param_t usb_in_local_param;
    usb_out_local_param_t usb_out_local_param;
    advanced_record_param_t advanced_record_param;
} audio_transmitter_scenario_param_t;

typedef struct {
    U8   scenario_type;
    U8   scenario_sub_id;
    U16  current_notification_index;
    U16  data_notification_frequency; /* frequency of notice CM4 */
    uint16_t max_payload_size; /* max allowed size in a block */
    U16  seq_num;
    U16  frame_size; /* current actually frame size */
    bool is_assembling; /* indicate whether the package has been generated done */
    bool is_customize; /* indicate whether user want to assemble the package itself */
    n9_dsp_share_info_ptr share_info_base_addr;
    audio_transmitter_scenario_param_t scenario_param;
} AUDIO_TRANSMITTER_PARAMETER;

typedef union {
    uint32_t reserved;
    /* For scenario specific struct define */
    usb_in_broadcast_param_t    usb_in_broadcast_param;
    usb_out_broadcast_param_t   usb_out_broadcast_param;
    void *                      dongle_handle;
} bt_common_scenario_param_t;

typedef struct {
    U8   scenario_type;
    U8   scenario_sub_id;
    U16  current_notification_index;
    U16  data_notification_frequency; /* frequency of notice BT controller */
    uint16_t max_payload_size; /* max allowed size in a block */
    U16  seq_num;
    U16  frame_size; /* current actually frame size */
    uint32_t status;
    STREAM_BUFFER_TYPE share_info_type;
    STREAM_BUFFER *share_info_base_addr;
    // n9_dsp_share_info_ptr share_info_base_addr;
    bt_common_scenario_param_t scenario_param;
} BT_COMMON_PARAMETER;

typedef union {
    L2CAP_SOURCE_PARAMETER  l2cap;
    RFCOMM_SOURCE_PARAMETER rfcomm;
    SCO_PARAMETER           sco;
    N9SCO_PARAMETER         n9sco;
#ifdef AIR_BT_CODEC_BLE_ENABLED
    N9BLE_PARAMETER         n9ble;
#endif
    REGION_PARAMETER        region;
    DSP_PARAMETER           dsp;
    AUDIO_PARAMETER         audio;
    VPRT_PARAMETER          VPRT;
    USBCLASS_PARAMETER      USB;
    AUDIO_QUEUE_PARAMETER   audioQ;
    A2DP_PARAMETER          a2dp;
    MEMORY_PARAMETER        memory;
    N9_A2DP_PARAMETER       n9_a2dp;
    CM4_PLAYBACK_PARAMETER  cm4_playback;
    AUDIO_TRANSMITTER_PARAMETER data_dl;
    BT_COMMON_PARAMETER     bt_common;
    AUDIO_VIRTUAL_PARAMETER virtual_para;
} SOURCE_PARAMETER;

typedef union {
    L2CAP_SINK_PARAMETER    l2cap;
    SCO_PARAMETER           sco;
    N9SCO_PARAMETER         n9sco;
#ifdef AIR_BT_CODEC_BLE_ENABLED
    N9BLE_PARAMETER         n9ble;
#endif
    RFCOMM_SINK_PARAMETER   rfcomm;
    REGION_PARAMETER        region;
    DSP_PARAMETER           dsp;
    AUDIO_PARAMETER         audio;
    USBCLASS_PARAMETER      USB;
    AUDIO_QUEUE_PARAMETER   audioQ;
    AUDIO_VIRTUAL_PARAMETER virtual_para;
    AIRAPP_SINK_PARAMETER   airapp;
    MEMORY_PARAMETER        memory;
    CM4_RECORD_PARAMETER    cm4_record;
    AUDIO_TRANSMITTER_PARAMETER data_ul;
    BT_COMMON_PARAMETER     bt_common;
} SINK_PARAMETER;


typedef struct {
    U8 type;
    U8 addr[6];
} BDADDR;
typedef struct {
    U32 lap;
    U8 uap;
    U16 nap;
} BD_ADDR;

/** @brief Mark highest bit for the valid of scenario type. */
#define HAL_AUDIO_STREAM_IN_SCENARIO_MARK 0x8000

/** @brief Define the audio stream in scenario type. */
typedef enum {
    HAL_AUDIO_STREAM_IN_SCENARIO_HFP = 0,  /**<  stream in scenario for HFP. */
    HAL_AUDIO_STREAM_IN_SCENARIO_LINE_IN,  /**<  stream in scenario for LINE IN. */
    HAL_AUDIO_STREAM_IN_SCENARIO_BLE_CALL, /**<  stream in scenario for BLE CALL. */
    HAL_AUDIO_STREAM_IN_SCENARIO_MAX,      /**<  stream in scenario max. */
    HAL_AUDIO_STREAM_IN_SCENARIO_INVALID,  /**<  stream in scenario invalid. */
} hal_audio_stream_in_scenario_t;

#ifdef AIR_BT_CODEC_BLE_ENABLED

typedef enum {
    CHANNEL_MODE_DL_ONLY,
    CHANNEL_MODE_DL_UL_BOTH,
    CHANNEL_MODE_UL_ONLY,
    CHANNEL_MODE_NUM
} channel_mode_t;
#endif

/* SideTone message parameter structure */
typedef afe_sidetone_param_t mcu2dsp_sidetone_param_t;
typedef afe_sidetone_param_p mcu2dsp_sidetone_param_p;

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
#define PEQ_DIRECT      (0)
#define PEQ_SYNC        (1)
typedef struct {
    uint8_t         *nvkey_addr;
    uint16_t        peq_nvkey_id;
    uint8_t         drc_enable;
    uint8_t         setting_mode;
    uint32_t        target_bt_clk;
    uint8_t         phase_id;
    uint8_t         drc_force_disable;
    uint8_t         gpt_time_sync;
} mcu2dsp_peq_param_t, *mcu2dsp_peq_param_p;
#endif

//--------------------------------------------
// Temp audio data
//--------------------------------------------
#ifdef HAL_AUDIO_ANC_ENABLE
typedef struct {
    au_afe_open_param_t    adc_setting;
    au_afe_open_param_t    dac_setting;
} mcu2dsp_open_adda_param_t, *mcu2dsp_open_adda_param_p;
#endif

typedef struct {
    uint32_t sequence_number;
    uint32_t bt_clock;
} audio_dsp_a2dp_dl_play_en_param_t, *audio_dsp_a2dp_dl_play_en_param_p;

typedef struct {
    audio_dsp_a2dp_dl_play_en_param_t play_en;
    bool is_play_en_ready;
    bool is_a2dp_started;
} audio_dsp_ull_start_ctrl_param_t;

#endif
