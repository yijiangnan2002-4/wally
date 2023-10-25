/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#ifndef _STREAM_N9BLE_H_
#define _STREAM_N9BLE_H_

/*!
 *@file   stream.h
 *@brief  defines the heap management of system
 *
 @verbatim
         Author : CharlesSu   <CharlesSu@airoha.com.tw>
                  ChiaHoHu    <ChiaHoHu@airoha.com.tw>
                  HungChiaLin <HungChiaLin@airoha.com.tw>
 @endverbatim
 */

#include "config.h"
#include "types.h"
#include "source_inter.h"
#include "sink_inter.h"

#include "source.h"
#include "sink.h"
//#include "transform.h"

#include "config.h"
#include "types.h"
#include "source_inter.h"
#include "sink_inter.h"

#include "transform_inter.h"

#include "common.h"



////////////////////////////////////////////////////////////////////////////////
// Type Defintions /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define BLE_AVM_INVALID_TIMESTAMP       0xFFFFFFFF
#define BLE_AVM_FRAME_HEADER_SIZE       28
#define BLE_UL_ERROR_DETECT_THD         8
#define LE_AUDIO_IRQ_INTERVAL_MS        10
#define LE_AUDIO_DL_PROCESS_TIME_MS     2
#define LE_AUDIO_THD_EXTEND_TIME_MS     50
#define LE_AUDIO_MAGIC_STREAM_PTR       0x12345678 /* to protect ECNR memory and support DL/UL on different tasks */

#define LE_AUDIO_OFFSET_PROTECT         10
#define LE_TIME_BT_CLOCK                625 /* unit with 312.5us */

#define abs32(x) ( (x >= 0) ? x : (-x) )

#define N9BLE_DEBUG

#ifndef ALIGN_4
#define ALIGN_4(_value)             (((_value) + 3) & ~3u)
#endif

typedef struct N9Ble_Sink_Config_s {
//    U16  N9_Ro_abnormal_cnt;
    U16  Buffer_Frame_Num;
    U16  Process_Frame_Num;
    U16  Frame_Size;
//    U16  Output_sample_rate;
//    BOOL isEnable;
} N9Ble_Sink_config_t;

typedef struct N9Ble_Source_Config_s {
    U16  Buffer_Frame_Num;
    U16  Process_Frame_Num;
    U16  Frame_Size;
//    U32  Input_sample_rate;
//    BOOL isEnable;
} N9Ble_Source_config_t;

typedef enum {
    BLE_PKT_FREE,
    BLE_PKT_USED,
    BLE_PKT_LOST,
} ble_packet_state;

typedef struct Stream_n9ble_Config_s {
    N9Ble_Source_config_t N9Ble_source;
    N9Ble_Sink_config_t   N9Ble_sink;
} Stream_n9ble_Config_t, *Stream_n9ble_Config_Ptr;

typedef struct  {
    stream_samplerate_t fs_in;
    stream_samplerate_t fs_out;
    stream_samplerate_t fs_tmp1;
    stream_samplerate_t fs_tmp2;
    uint8_t swb_cnt;
} N9ble_Swsrc_Config_t, *N9ble_Swsrc_Config_Ptr;

typedef struct  {
    U16 DataOffset; /* offset of payload */
    U16 _reserved_word_02h;
    U32 TimeStamp; /* this CIS/BIS link's CLK, Unit:312.5us */
    U16 hw_rssi_agc_log2;
    U16 hw_rssi_log2;
    U8 PduHdrLo;
    U8 _reserved_byte_0Dh;//indicate it¡¦s valid packet(0x1) or invalid packet(0x0) *
    U16 Pdu_LEN_Without_MIC; //For Uplink pls fill the Data size
    U16 DataLen;
    U16 EventCount; //Debug purpose *
    U32 crc32_value;
    U32 _reserved_Long_1;
} LE_AUDIO_HEADER;


////////////////////////////////////////////////////////////////////////////////
// Function Declarations ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
EXTERN VOID SinkInitN9Ble(SINK sink);
EXTERN VOID SourceInitN9Ble(SOURCE source);
EXTERN VOID SourceInitBleAvm(SOURCE source,SINK sink);
EXTERN Stream_n9ble_Config_t N9BLE_setting;
EXTERN bool ble_query_rx_packet_lost_status(uint32_t index);
EXTERN bool ble_query_rx_sub_cis_packet_lost_status(uint32_t index);
EXTERN VOID N9Ble_SourceUpdateLocalReadOffset(SOURCE source, U8 num);
#if defined(AIR_UL_FIX_SAMPLING_RATE_48K) && defined(AIR_FIXED_RATIO_SRC)
EXTERN void N9Ble_UL_Fix_Sample_Rate_Init(void);
EXTERN void N9Ble_UL_Fix_Sample_Rate_Deinit(void);
#endif

#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
EXTERN void N9Ble_DL_SWB_Sample_Rate_Init(void);
EXTERN void N9Ble_DL_SWB_Sample_Rate_Deinit(void);
EXTERN void N9Ble_UL_SWB_Sample_Rate_Init(void);
EXTERN void N9Ble_UL_SWB_Sample_Rate_Deinit(void);
#endif

#if defined(AIR_MUTE_MIC_DETECTION_ENABLE)
EXTERN void N9Ble_UL_Volume_Estimator_Init(SINK sink);
EXTERN void N9Ble_UL_Volume_Estimator_Deinit(void);
#endif

#endif /* _STREAM_N9SCO_H_ */

