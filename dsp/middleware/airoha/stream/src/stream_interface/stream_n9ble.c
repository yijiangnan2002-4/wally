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

#ifdef AIR_BT_CODEC_BLE_ENABLED

#include "types.h"
#include "stream_audio.h"
#include "source_inter.h"
#include "sink_inter.h"
#include "transform.h"
#include "stream_n9ble.h"
#include "dsp_audio_msg.h"
#include "audio_config.h"
#include "dsp_audio_process.h"
#include "dsp_memory.h"
#include "dsp_share_memory.h"
#include "dsp_temp.h"
#include "dsp_dump.h"
#ifdef AIR_BT_CLK_SKEW_ENABLE
#include "clk_skew.h"
#endif
#include "source_inter.h"
#include "audio_nvdm_common.h"
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "stream_audio_transmitter.h"
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */
#ifdef AIR_BT_LE_LC3PLUS_ENABLE
#include "lc3plus_dec_interface.h"
#include "lc3plus_enc_interface.h"
#endif
#if defined(AIR_CELT_DEC_V2_ENABLE)
#include "celt_dec_interface_v2.h"
#endif /* AIR_CELT_DEC_V2_ENABLE */

#if defined(AIR_CELT_ENC_V2_ENABLE)
#include "celt_enc_interface_v2.h"
#endif /* AIR_CELT_ENC_V2_ENABLE */

#if defined(AIR_AUDIO_ULD_DECODE_ENABLE)
#include "uld_dec_interface.h"
#endif

#ifdef MTK_PEQ_ENABLE
#include "peq_interface.h"
#endif
#include "bt_interface.h"

Stream_n9ble_Config_t N9BLE_setting;
static U8 g_ble_pkt_lost[4][2];
static uint32_t g_pkt_lost_count = 0;

uint16_t g_ble_abr_length=0;
uint16_t g_ble_ul_abr_length=0;

#ifdef AIR_BLE_FEATURE_MODE_ENABLE
static bool g_ble_dl_ul_process_active;
#endif
extern bool ULL_NrOffloadFlag;
#ifdef AIR_ECNR_PREV_PART_ENABLE
extern uint8_t voice_ecnr_ec_get_postec_gain(void);
#endif

//#define SINE_PATTERN_TEST   1
#ifdef SINE_PATTERN_TEST
#define SINE_PATTERN_TEST_SKIP_CNT (100) // 1s for 10ms case, 0.75s for 7.5ms case
U8 sine_1k_10ms_64kbps_pattern[][80] = 
{
    {
    	0x99, 0x20, 0xF2, 0xBA, 0xCA, 0x9A, 0x8B, 0x66, 0x7B, 0x06, 0xA5, 0x46,
    	0x77, 0xF4, 0x3D, 0xF0, 0xB2, 0xD6, 0x24, 0xE9, 0x7C, 0xF5, 0xA5, 0xA5,
    	0xFA, 0x8D, 0x7E, 0xBA, 0xE6, 0xE3, 0xF5, 0xAC, 0x84, 0x0A, 0x5A, 0x79,
    	0x56, 0x15, 0x5F, 0xC5, 0x16, 0xD1, 0xA8, 0xD1, 0xC5, 0x56, 0x94, 0xAD,
    	0x4A, 0xD5, 0x5A, 0xD4, 0xAD, 0x5A, 0xA6, 0x83, 0xAA, 0x80, 0xFB, 0x0B,
    	0xD2, 0x96, 0x3A, 0xCE, 0xD4, 0x76, 0x0A, 0xAE, 0xAA, 0x88, 0xC9, 0x83,
    	0x49, 0x72, 0x53, 0xE8, 0x43, 0xFE, 0x41, 0x2B
    },
    {
    	0x6A, 0x88, 0xD7, 0x6B, 0x4C, 0x0C, 0x5B, 0xAA, 0xDA, 0xDC, 0x2A, 0x5E,
    	0x2F, 0xED, 0x7A, 0x99, 0x39, 0x52, 0x09, 0x0E, 0x37, 0xF0, 0xDC, 0xE5,
    	0x91, 0xA9, 0xA5, 0x82, 0x76, 0x3B, 0x90, 0x92, 0xE9, 0x1A, 0x1C, 0x4A,
    	0x59, 0x62, 0x70, 0x11, 0x0C, 0xFC, 0x20, 0x00, 0xB8, 0xE8, 0x65, 0x1F,
    	0x81, 0x8F, 0x9F, 0xCD, 0x1D, 0x8C, 0x78, 0x9A, 0xDA, 0xE8, 0x35, 0x1A,
    	0xB1, 0x84, 0xCE, 0x3B, 0xA1, 0x21, 0xA5, 0x80, 0x64, 0xA7, 0x8F, 0x83,
    	0x41, 0x02, 0xAA, 0xE6, 0x4B, 0xED, 0x12, 0x5B
    }
};

U8 sine_1k_7p5ms_64kbps_pattern[][60] = 
{
    {
    	0x57, 0x66, 0xF1, 0x0E, 0x0B, 0xC7, 0x90, 0x70, 0x14, 0x80, 0xAB, 0xF3,
    	0x83, 0x38, 0xE2, 0x0F, 0x18, 0x30, 0x14, 0xC8, 0xD8, 0x55, 0x6C, 0xDE,
    	0x09, 0x24, 0xBC, 0x5C, 0x24, 0x40, 0x07, 0xE3, 0xAA, 0x50, 0x0D, 0x27,
    	0x35, 0x65, 0x95, 0x55, 0x52, 0xB4, 0x01, 0xA0, 0xE7, 0x17, 0xD8, 0xE6,
    	0xC2, 0x60, 0x98, 0x93, 0xF9, 0x38, 0xEB, 0xF2, 0x23, 0xAF, 0x2C, 0xEF
    },
    {
    	0xD3, 0x6B, 0x97, 0xE2, 0x6B, 0x08, 0x10, 0x5C, 0x1D, 0x74, 0x11, 0x44,
    	0x20, 0x0C, 0x29, 0xC2, 0xA2, 0xB9, 0x2F, 0xA8, 0xF1, 0x5F, 0x3E, 0xB1,
    	0x30, 0xE9, 0x15, 0xCE, 0xE7, 0xB7, 0xC3, 0x44, 0x68, 0x29, 0xD0, 0xAC,
    	0x31, 0xDC, 0x06, 0xD3, 0x37, 0x99, 0x0D, 0x4D, 0x97, 0xCB, 0x15, 0x69,
    	0xCD, 0x08, 0x53, 0xC1, 0xA1, 0x04, 0x94, 0x01, 0x66, 0xFE, 0x88, 0xD3
    },
    {
    	0xD3, 0x6B, 0x99, 0x02, 0xF0, 0x97, 0x0D, 0x62, 0x5B, 0x05, 0x71, 0xF8,
    	0xF0, 0x98, 0x6E, 0x87, 0x39, 0x97, 0x00, 0xC1, 0x0A, 0xD2, 0x7B, 0x85,
    	0xEE, 0x04, 0x3E, 0x2D, 0x6A, 0x60, 0x60, 0xF1, 0xEE, 0xDD, 0x79, 0xA3,
    	0x8B, 0xB1, 0x14, 0x72, 0xF2, 0x04, 0x4D, 0xE7, 0xC6, 0x9B, 0x8E, 0x64,
    	0xA0, 0x1F, 0x68, 0xC1, 0xA1, 0x04, 0x94, 0x01, 0x66, 0xFE, 0x8C, 0xD3
    },
    {
    	0xD3, 0x6B, 0x97, 0xE2, 0x6A, 0xCA, 0x90, 0x7E, 0x8F, 0x81, 0xCE, 0x07,
    	0x92, 0x52, 0x96, 0x1E, 0x53, 0x26, 0x51, 0xE5, 0xF5, 0x77, 0xE6, 0xAA,
    	0xFC, 0xED, 0x82, 0x23, 0x29, 0x9F, 0x9F, 0x0D, 0x2D, 0xD1, 0x75, 0xA3,
    	0xBB, 0xB1, 0xD4, 0x72, 0x32, 0x08, 0x4D, 0xE4, 0xC6, 0x98, 0x8E, 0x54,
    	0xAC, 0x13, 0x5B, 0xC1, 0xA9, 0x04, 0x94, 0x01, 0x66, 0xFE, 0x8C, 0xD3
    },
    {
    	0xD3, 0x6B, 0x99, 0x02, 0xF0, 0x97, 0x0D, 0x62, 0x5B, 0x05, 0x71, 0xF8,
    	0xF0, 0x98, 0x6E, 0x87, 0x39, 0x97, 0x00, 0xC1, 0x0A, 0xD2, 0x7B, 0x85,
    	0xEE, 0x04, 0x3E, 0x2D, 0x6A, 0x60, 0x60, 0xF1, 0xEE, 0xDD, 0x79, 0xA3,
    	0x8B, 0xB1, 0x14, 0x72, 0xF2, 0x04, 0x4D, 0xE7, 0xC6, 0x9B, 0x8E, 0x64,
    	0xA0, 0x1F, 0x68, 0xC1, 0xA9, 0x04, 0x94, 0x01, 0x66, 0xFE, 0x8C, 0xD3
    }
};

#endif // #ifdef SINE_PATTERN_TEST


static U16 N9Ble_calculate_avm_frame_num(U16 avm_buffer_length, U16 frame_length){
    return (avm_buffer_length/ALIGN_4(frame_length + BLE_AVM_FRAME_HEADER_SIZE));
}

#if defined(AIR_CELT_DEC_V2_ENABLE) || defined(AIR_AUDIO_ULD_DECODE_ENABLE)
static void N9ble_add_status_to_frames_in_packet(SOURCE source, U8 *dst_addr, U8 *read_ptr, U32 packet_size, U32 frame_num, U32 status)
{
    //status: 0 -> Normal
    //status: 1 -> PLC
    U8 *wptr;
    U32 frame_size = packet_size/frame_num;
    U32 combinedSize = ALIGN_4((source->param.n9ble.plc_state_len) + frame_size);/*4B align is needed because of decoder interface*/
    for (U32 i = 0; i < frame_num; i++) {
        wptr = dst_addr + combinedSize*i;
        //Add status
        *((U32*)wptr) = status;
        //Update wptr
        wptr += source->param.n9ble.plc_state_len;
        //Write date to input buffer
        if (status == 0) {
            memcpy(wptr, read_ptr + BLE_AVM_FRAME_HEADER_SIZE + frame_size*i, frame_size);
        } else {
            memset(wptr, 0, frame_size);
        }
    }
    return;
}
#endif

static U32 N9Ble_check_avm_frame_valid(LE_AUDIO_HEADER *buf_header, U32 check_abr)
{
    /*
        _reserved_byte_0Dh = 0x0 ==> NULL
        _reserved_byte_0Dh = 0x1 ==> VALID
        _reserved_byte_0Dh = 0x3 ==> PADDING
    */
    U32 frame_valid = (buf_header->TimeStamp != BLE_AVM_INVALID_TIMESTAMP) && (buf_header->_reserved_byte_0Dh & 0x01);
    if(frame_valid && check_abr) {
        frame_valid = (buf_header->Pdu_LEN_Without_MIC) && (g_ble_abr_length == buf_header->Pdu_LEN_Without_MIC);
    }
    return frame_valid;
}

ATTR_TEXT_IN_IRAM static VOID N9BleRx_update_from_share_information(SOURCE source)
{
    memcpy(&(source->streamBuffer.ShareBufferInfo), source->param.n9ble.share_info_base_addr, sizeof(n9_dsp_share_info_t));

    #ifdef AIR_BTA_IC_PREMIUM_G3
    source->streamBuffer.ShareBufferInfo.start_addr |= 0x60000000;
    #else
    source->streamBuffer.ShareBufferInfo.start_addr = hal_memview_cm4_to_dsp0(source->streamBuffer.ShareBufferInfo.start_addr);
    #endif

    /* Refer to the hal_audio_dsp_controller.c for the limitaion of legnth */
    source->streamBuffer.ShareBufferInfo.length = ALIGN_4(source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE) * source->param.n9ble.process_number * N9Ble_calculate_avm_frame_num(source->param.n9ble.share_info_base_addr->length, source->param.n9ble.frame_length);
}

ATTR_TEXT_IN_IRAM static VOID N9BleTx_update_from_share_information(SINK sink)
{
    memcpy(&(sink->streamBuffer.ShareBufferInfo), sink->param.n9ble.share_info_base_addr, sizeof(n9_dsp_share_info_t));

    #ifdef AIR_BTA_IC_PREMIUM_G3
    sink->streamBuffer.ShareBufferInfo.start_addr |= 0x60000000;
    #else
    sink->streamBuffer.ShareBufferInfo.start_addr = hal_memview_cm4_to_dsp0(sink->streamBuffer.ShareBufferInfo.start_addr);
    #endif

    /* Refer to the hal_audio_dsp_controller.c for the limitaion of legnth */
    sink->streamBuffer.ShareBufferInfo.length = ALIGN_4(sink->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE) * N9Ble_calculate_avm_frame_num(sink->param.n9ble.share_info_base_addr->length, sink->param.n9ble.frame_length);
}

ATTR_TEXT_IN_IRAM_LEVEL_2 VOID N9Ble_update_readoffset_share_information(SOURCE source, U32 ReadOffset)
{
    source->param.n9ble.share_info_base_addr->read_offset = ReadOffset;
    source->param.n9ble.share_info_base_addr->bBufferIsFull = FALSE;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 VOID N9Ble_update_writeoffset_share_information(SINK sink, U32 WriteOffset)
{
    sink->param.n9ble.share_info_base_addr->write_offset = WriteOffset;
    if (WriteOffset == sink->param.n9ble.share_info_base_addr->read_offset) {
        sink->param.n9ble.share_info_base_addr->bBufferIsFull = TRUE;
    }
}

ATTR_TEXT_IN_IRAM_LEVEL_2 static VOID N9Ble_update_timestamp_share_information(U32 *timestamp_ptr, U32 timestamp)
{
    *timestamp_ptr = timestamp;
}

static VOID N9Ble_Reset_Sourceoffset_share_information(SOURCE source)
{
    source->param.n9ble.share_info_base_addr->write_offset = 0;
    source->param.n9ble.share_info_base_addr->read_offset = 0;
    source->param.n9ble.share_info_base_addr->length = ALIGN_4(source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE) * source->param.n9ble.process_number * N9Ble_calculate_avm_frame_num(source->param.n9ble.share_info_base_addr->length, source->param.n9ble.frame_length);
    source->param.n9ble.share_info_base_addr->bBufferIsFull = FALSE;
}

static VOID N9Ble_Reset_Sinkoffset_share_information(SINK sink)
{
    sink->param.n9ble.share_info_base_addr->write_offset = 0;
    sink->param.n9ble.share_info_base_addr->read_offset = 0;
    sink->param.n9ble.share_info_base_addr->length = ALIGN_4(sink->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE) * N9Ble_calculate_avm_frame_num(sink->param.n9ble.share_info_base_addr->length, sink->param.n9ble.frame_length);
    sink->param.n9ble.share_info_base_addr->bBufferIsFull = FALSE;
}

static VOID N9Ble_SinkUpdateLocalWriteOffset(SINK sink, U8 num)
{
    sink->streamBuffer.ShareBufferInfo.write_offset += ALIGN_4(sink->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE) * num;
    sink->streamBuffer.ShareBufferInfo.write_offset %= sink->streamBuffer.ShareBufferInfo.length;
}

VOID N9Ble_SourceUpdateLocalReadOffset(SOURCE source, U8 num)
{
    source->streamBuffer.ShareBufferInfo.read_offset += ALIGN_4(source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE) * num;
    source->streamBuffer.ShareBufferInfo.read_offset %= source->streamBuffer.ShareBufferInfo.length;
}

static VOID N9Ble_Rx_Buffer_Init(SOURCE source)
{
    N9BleRx_update_from_share_information(source);
    N9Ble_Reset_Sourceoffset_share_information(source);
}

static VOID N9Ble_Tx_Buffer_Init(SINK sink)
{
    N9BleTx_update_from_share_information(sink);
    N9Ble_Reset_Sinkoffset_share_information(sink);
}

static VOID N9Ble_Default_setting_init(SOURCE source, SINK sink)
{

    if (source != NULL) {
        //source->param.n9ble.share_info_base_addr->length        = ((28+156)*6);
        N9BLE_setting.N9Ble_source.Buffer_Frame_Num        = N9Ble_calculate_avm_frame_num(source->param.n9ble.share_info_base_addr->length, source->param.n9ble.frame_length);
        N9BLE_setting.N9Ble_source.Process_Frame_Num       = source->param.n9ble.process_number;
        N9BLE_setting.N9Ble_source.Frame_Size              = ALIGN_4(BLE_AVM_FRAME_HEADER_SIZE + source->param.n9ble.frame_length);
        DSP_MW_LOG_I("[BLE] source avm length %d source Frame_Size: %d  Buffer_Frame_Num: %d", 3, source->param.n9ble.share_info_base_addr->length, N9BLE_setting.N9Ble_source.Frame_Size, N9BLE_setting.N9Ble_source.Buffer_Frame_Num);
    }

    if (sink != NULL) {
        //sink->param.n9ble.share_info_base_addr->length        = ((28+156)*6);
        N9BLE_setting.N9Ble_sink.Buffer_Frame_Num        = N9Ble_calculate_avm_frame_num(sink->param.n9ble.share_info_base_addr->length, sink->param.n9ble.frame_length);
        N9BLE_setting.N9Ble_sink.Process_Frame_Num         = sink->param.n9ble.process_number;
        N9BLE_setting.N9Ble_sink.Frame_Size                = ALIGN_4(BLE_AVM_FRAME_HEADER_SIZE + sink->param.n9ble.frame_length);
        //N9BLE_setting.N9Ble_sink.N9_Ro_abnormal_cnt        = 0;
        DSP_MW_LOG_I("[BLE] sink avm length %d sink Frame_Size: %d  Buffer_Frame_Num: %d", 3, sink->param.n9ble.share_info_base_addr->length, N9BLE_setting.N9Ble_sink.Frame_Size, N9BLE_setting.N9Ble_sink.Buffer_Frame_Num);
    }
}

#if ((defined(AIR_UL_FIX_SAMPLING_RATE_48K)|| defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE)) && defined(AIR_FIXED_RATIO_SRC))
#include "src_fixed_ratio_interface.h"

static uint32_t internal_fs_converter(stream_samplerate_t fs)
{
    switch (fs) {
        case FS_RATE_44_1K:
            return 44100;

        case FS_RATE_8K:
        case FS_RATE_16K:
        case FS_RATE_24K:
        case FS_RATE_32K:
        case FS_RATE_48K:
        case FS_RATE_96K:
            return fs * 1000;

        default:
            DSP_MW_LOG_E("[BLE] sample rate is not supported!", 0);
            AUDIO_ASSERT(FALSE);
            return fs;
    }
}
#endif
#if (defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC))
#define n9ble_dl_fixed_ratio_port_ma

#if defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_48k) || defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_96k)
#define N9BLE_DL_FIXED_RATIO_PORT_NAX_NUM 5
#else
#define N9BLE_DL_FIXED_RATIO_PORT_NAX_NUM 3
#endif

static src_fixed_ratio_port_t *g_n9ble_dl_swb_fixed_ratio_port[N9BLE_DL_FIXED_RATIO_PORT_NAX_NUM];
static src_fixed_ratio_port_t *g_n9ble_ul_swb_fixed_ratio_port[3];

static const N9ble_Swsrc_Config_t N9Ble_Swsrc_maping_table[] = {
    /*fs_in, fs_out, fs_tmp1, fs_tmp2, swb_cnt*/
    { 8, 8, 0, 0, 0},
    { 8, 16, 0, 0, 1},
    { 8, 24, 0, 0, 1},
    { 8, 32, 16, 0, 2},
    { 8, 48, 16, 0, 2},
    { 8, 96, 16, 32, 3},
    {16, 16, 0, 0, 0},
    {16, 24, 48, 0, 2},
    {16, 32, 0, 0, 1},
    {16, 48, 0, 0, 1},
    {16, 96, 32, 0, 2},
    {24, 24, 0, 0, 0},
    {24, 32, 48, 96, 3},
    {24, 48, 0, 0, 1},
    {24, 96, 48, 0, 2},
    {32, 32, 0, 0, 0},
    {32, 48, 96, 0, 2},
    {32, 96, 0, 0, 1},
    {48, 48, 0, 0, 0},
    {48, 96, 0, 0, 1},
    {96, 96, 0, 0, 0},
};

#if defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_48k) || defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_96k)
static const N9ble_Swsrc_Config_t N9Ble_fix_rate_Swsrc_maping_table[] = {
    /*fs_in, fs_out, fs_tmp1, fs_tmp2, swb_cnt*/
    { 32, 48, 96, 0, 2},
    { 32, 96, 0, 0, 1},
    { 16, 48, 0, 0, 1},
    { 16, 96, 16, 0, 1},
};
#endif

void N9Ble_DL_Sample_Rate_Parameter_Maping(N9ble_Swsrc_Config_t *table, uint32_t table_size, stream_samplerate_t fs_in, stream_samplerate_t fs_out, stream_samplerate_t *fs_temp, stream_samplerate_t *fs_temp2, uint8_t *swb_cnt)
{
    uint32_t i;
    for (i = 0; i < table_size; i++) {
        if (fs_in <= fs_out) {
            if ((fs_in == table[i].fs_in) && (fs_out == table[i].fs_out)) {
                *fs_temp = table[i].fs_tmp1;
                *fs_temp2 = table[i].fs_tmp2;
                *swb_cnt = table[i].swb_cnt;
                break;
            }
        } else {
            if ((fs_out == table[i].fs_in) && (fs_in == table[i].fs_out)) {
                *fs_temp = table[i].fs_tmp2;
                *fs_temp2 = table[i].fs_tmp1;
                *swb_cnt = table[i].swb_cnt;
                break;
            }
        }
    }

    if ((!(*fs_temp)) && (!(*fs_temp2))) { /*swb_cnt=1*/
        (*fs_temp) = fs_out;
        (*fs_temp2) = fs_out;
    }
    if ((!(*fs_temp)) || (!(*fs_temp2))) { /*swb_cnt<3*/
        if (!(*fs_temp)) {
            (*fs_temp) = (*fs_temp2);
            (*fs_temp2) = fs_out;
        } else {
            (*fs_temp2) = fs_out;
        }
    }
}

void N9Ble_DL_SWB_Sample_Rate_Init(void)
{
    uint32_t channel_number;
    DSP_STREAMING_PARA_PTR dl_stream;
    src_fixed_ratio_config_t *smp_config;
    SOURCE source = Source_blks[SOURCE_TYPE_N9BLE];

    dl_stream = DSP_Streaming_Get(source, source->transform->sink);
    //channel_number = stream_function_get_channel_number(&(dl_stream->callback.EntryPara));
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    channel_number = 2;
#else
    channel_number = source->transform->sink->param.audio.channel_num;
#endif
    stream_samplerate_t fs_in = dl_stream->callback.EntryPara.in_sampling_rate;
    //stream_samplerate_t fs_out = dl_stream->sink->param.audio.src_rate / 1000; //dl_stream->callback.EntryPara.codec_out_sampling_rate;
    stream_samplerate_t fs_out = 48;
    stream_samplerate_t fs_temp = 0, fs_temp2 = 0;
    uint8_t swb_cnt = 0;
    uint32_t used_port_num = 1;
#if defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_48k) || defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_96k)
    uint8_t swb_cnt_fix_rate = 0;
    stream_samplerate_t fs_in_fix_rate, fs_out_fix_rate, fs_temp_fix_rate, fs_temp2_fix_rate;
#endif

    if (fs_in == 96){
        fs_out = 96;
    }
    if(source->param.n9ble.context_type == BLE_CONTEXT_CONVERSATIONAL){
#ifdef AIR_BT_BLE_SWB_ENABLE
        fs_out = 32;
#else
        fs_out = 16;
#endif
    } else if (source->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE && source->param.n9ble.codec_type == BT_BLE_CODEC_ULD){
#ifdef AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ
        fs_out = 96;
#endif
    }

    /*fs_in = FS_RATE_32K;
    fs_temp = FS_RATE_16K;//fs_out;
    fs_temp2 = FS_RATE_48K;//fs_out;
    fs_out = FS_RATE_16K;
    swb_cnt = 1;*/

    N9Ble_DL_Sample_Rate_Parameter_Maping((N9ble_Swsrc_Config_t *)N9Ble_Swsrc_maping_table, ARRAY_SIZE(N9Ble_Swsrc_maping_table), fs_in, fs_out, &fs_temp, &fs_temp2, &swb_cnt);
    DSP_MW_LOG_I("[BLE] N9Ble_DL_SWB_Sample_Rate_Init %d %d %d %d %d", 5, fs_in, fs_temp, fs_temp2, fs_out, swb_cnt);

#if defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_48k) || defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_96k)
    if(source->param.n9ble.context_type == BLE_CONTEXT_CONVERSATIONAL) {
        fs_in_fix_rate = fs_out;
#if defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_48k)
        fs_out_fix_rate = 48;
#else
        fs_out_fix_rate = 96;
#endif
        N9Ble_DL_Sample_Rate_Parameter_Maping((N9ble_Swsrc_Config_t *)N9Ble_fix_rate_Swsrc_maping_table, ARRAY_SIZE(N9Ble_fix_rate_Swsrc_maping_table), fs_in_fix_rate, fs_out_fix_rate, &fs_temp_fix_rate, &fs_temp2_fix_rate, &swb_cnt_fix_rate);
        DSP_MW_LOG_I("[BLE][fix_rate] %d %d %d %d %d", 5, fs_in_fix_rate, fs_temp_fix_rate, fs_temp2_fix_rate, fs_out_fix_rate, swb_cnt_fix_rate);
    }
#endif //AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_48k || AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_96k

    smp_config = (src_fixed_ratio_config_t *)pvPortMalloc(sizeof(src_fixed_ratio_config_t));
    configASSERT(smp_config);
    memset((U8*)smp_config, 0, sizeof(src_fixed_ratio_config_t));

    smp_config->cvt_num = swb_cnt;
    smp_config->channel_number = channel_number;
    smp_config->resolution = dl_stream->callback.EntryPara.resolution.feature_res;
#if defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_48k) || defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_96k)
    if(source->param.n9ble.context_type == BLE_CONTEXT_CONVERSATIONAL) {
        smp_config->multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE_AND_ALTERNATE;
        smp_config->cvt_processing_num_in_list = 1;
    } else
#endif
    {
        smp_config->multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;
    }
    smp_config->in_sampling_rate = internal_fs_converter(fs_in);
    smp_config->out_sampling_rate = internal_fs_converter(fs_temp);
    g_n9ble_dl_swb_fixed_ratio_port[0] = stream_function_src_fixed_ratio_get_port(source);
    stream_function_src_fixed_ratio_init(g_n9ble_dl_swb_fixed_ratio_port[0], smp_config);
    used_port_num++;

    if (swb_cnt > 1) {
        smp_config->in_sampling_rate = internal_fs_converter(fs_temp);
        smp_config->out_sampling_rate = internal_fs_converter(fs_temp2);
        g_n9ble_dl_swb_fixed_ratio_port[1] = stream_function_src_fixed_ratio_get_2nd_port(source);
        stream_function_src_fixed_ratio_init(g_n9ble_dl_swb_fixed_ratio_port[1], smp_config);
        used_port_num++;
    }

    if (swb_cnt > 2) {
        smp_config->in_sampling_rate = internal_fs_converter(fs_temp2);
        smp_config->out_sampling_rate = internal_fs_converter(fs_out);
        g_n9ble_dl_swb_fixed_ratio_port[2] = stream_function_src_fixed_ratio_get_3rd_port(source);
        stream_function_src_fixed_ratio_init(g_n9ble_dl_swb_fixed_ratio_port[2], smp_config);
        used_port_num++;
    }

#if defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_48k) || defined (AIR_LE_CALL_DL_STREAM_RATE_FIX_TO_96k)
    if(source->param.n9ble.context_type == BLE_CONTEXT_CONVERSATIONAL) {
        smp_config->cvt_processing_num_in_list = 2;
        smp_config->cvt_num = swb_cnt_fix_rate;

        smp_config->in_sampling_rate = internal_fs_converter(fs_in_fix_rate);
        smp_config->out_sampling_rate = internal_fs_converter(fs_temp_fix_rate);
        g_n9ble_dl_swb_fixed_ratio_port[used_port_num] = stream_function_src_fixed_ratio_get_number_port(source, used_port_num);
        stream_function_src_fixed_ratio_init(g_n9ble_dl_swb_fixed_ratio_port[used_port_num], smp_config);
        used_port_num++;


        if (swb_cnt_fix_rate > 1) {
            smp_config->in_sampling_rate = internal_fs_converter(fs_temp_fix_rate);
            smp_config->out_sampling_rate = internal_fs_converter(fs_temp2_fix_rate);
            g_n9ble_dl_swb_fixed_ratio_port[used_port_num] = stream_function_src_fixed_ratio_get_number_port(source, used_port_num);
            stream_function_src_fixed_ratio_init(g_n9ble_dl_swb_fixed_ratio_port[used_port_num], smp_config);
            used_port_num++;
        }

        if (swb_cnt_fix_rate > 2) {
            smp_config->in_sampling_rate = internal_fs_converter(fs_temp2_fix_rate);
            smp_config->out_sampling_rate = internal_fs_converter(fs_out_fix_rate);
            g_n9ble_dl_swb_fixed_ratio_port[used_port_num] = stream_function_src_fixed_ratio_get_number_port(source, used_port_num);
            stream_function_src_fixed_ratio_init(g_n9ble_dl_swb_fixed_ratio_port[used_port_num], smp_config);
            used_port_num++;
        }
    }
#endif
    DSP_MW_LOG_I("[BLE] N9Ble_DL_SWB_Sample_Rate_Init: channel_number %d, fs_in %d, fs_temp %d, fs_temp2 %d, fs_out %d", 5,
                 smp_config->channel_number, fs_in, fs_temp, fs_temp2, fs_out);

    if(smp_config) {
        vPortFree(smp_config);
    }
}

void N9Ble_DL_SWB_Sample_Rate_Deinit(void)
{
    uint32_t i;

    DSP_MW_LOG_I("[BLE]N9Ble_DL_SWB_Sample_Rate_Deinit", 0);

    for (i = 0 ; i < ARRAY_SIZE(g_n9ble_dl_swb_fixed_ratio_port) ; ++i) {
        if (g_n9ble_dl_swb_fixed_ratio_port[i]) {
            stream_function_src_fixed_ratio_deinit(g_n9ble_dl_swb_fixed_ratio_port[i]);
            g_n9ble_dl_swb_fixed_ratio_port[i] = NULL;
        }
    }
}

void N9Ble_UL_SWB_Sample_Rate_Init(void)
{
    uint32_t channel_number;
    DSP_STREAMING_PARA_PTR ul_stream;
    src_fixed_ratio_config_t *smp_config;
    SINK sink = Sink_blks[SINK_TYPE_N9BLE];

    ul_stream = DSP_Streaming_Get(sink->transform->source, sink);
    channel_number = stream_function_get_channel_number(&(ul_stream->callback.EntryPara));
    stream_samplerate_t fs_in = ul_stream->callback.EntryPara.in_sampling_rate;
    stream_samplerate_t fs_out = ul_stream->callback.EntryPara.codec_out_sampling_rate;
    stream_samplerate_t fs_temp = 0, fs_temp2 = 0;
    uint8_t swb_cnt = 0;
    /*fs_in = FS_RATE_16K;
    fs_temp = FS_RATE_32K;//fs_out;
    fs_temp2 = FS_RATE_96K;//fs_out;
    fs_out = FS_RATE_32K;
    swb_cnt = 1;*/

    uint32_t i;
    for (i = 0; i < ARRAY_SIZE(N9Ble_Swsrc_maping_table); i++) {
        if (fs_in <= fs_out) {
            if ((fs_in == N9Ble_Swsrc_maping_table[i].fs_in) && (fs_out == N9Ble_Swsrc_maping_table[i].fs_out)) {
                fs_temp = N9Ble_Swsrc_maping_table[i].fs_tmp1;
                fs_temp2 = N9Ble_Swsrc_maping_table[i].fs_tmp2;
                swb_cnt = N9Ble_Swsrc_maping_table[i].swb_cnt;
                break;
            }
        } else {
            if ((fs_out == N9Ble_Swsrc_maping_table[i].fs_in) && (fs_in == N9Ble_Swsrc_maping_table[i].fs_out)) {
                fs_temp = N9Ble_Swsrc_maping_table[i].fs_tmp2;
                fs_temp2 = N9Ble_Swsrc_maping_table[i].fs_tmp1;
                swb_cnt = N9Ble_Swsrc_maping_table[i].swb_cnt;
                break;
            }
        }
    }

    if ((!fs_temp) && (!fs_temp2)) {
        fs_temp = fs_out;
        fs_temp2 = fs_out;
    }
    if ((!fs_temp) || (!fs_temp2)) {
        if (!fs_temp) {
            fs_temp = fs_temp2;
            fs_temp2 = fs_out;
        } else {
            fs_temp2 = fs_out;
        }
    }

    DSP_MW_LOG_I("[BLE] N9Ble_UL_SWB_Sample_Rate_Init %d %d %d %d %d", 5, fs_in, fs_temp, fs_temp2, fs_out, swb_cnt);


    smp_config = pvPortMalloc(sizeof(src_fixed_ratio_config_t));
    configASSERT(smp_config);
    memset((U8*)smp_config, 0, sizeof(src_fixed_ratio_config_t));

    smp_config->cvt_num = swb_cnt;
    smp_config->channel_number = channel_number;
    smp_config->resolution = RESOLUTION_16BIT; // ul_stream->callback.EntryPara.resolution.feature_res;
    smp_config->multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;

    smp_config->in_sampling_rate = internal_fs_converter(fs_in);
    smp_config->out_sampling_rate = internal_fs_converter(fs_temp);
    g_n9ble_ul_swb_fixed_ratio_port[0] = stream_function_src_fixed_ratio_get_port(sink);
    stream_function_src_fixed_ratio_init(g_n9ble_ul_swb_fixed_ratio_port[0], smp_config);

    if (swb_cnt > 1) {
        smp_config->in_sampling_rate = internal_fs_converter(fs_temp);
        smp_config->out_sampling_rate = internal_fs_converter(fs_temp2);
        g_n9ble_ul_swb_fixed_ratio_port[1] = stream_function_src_fixed_ratio_get_2nd_port(sink);
        stream_function_src_fixed_ratio_init(g_n9ble_ul_swb_fixed_ratio_port[1], smp_config);
    }

    if (swb_cnt > 2) {
        smp_config->in_sampling_rate = internal_fs_converter(fs_temp2);
        smp_config->out_sampling_rate = internal_fs_converter(fs_out);
        g_n9ble_ul_swb_fixed_ratio_port[2] = stream_function_src_fixed_ratio_get_3rd_port(sink);
        stream_function_src_fixed_ratio_init(g_n9ble_ul_swb_fixed_ratio_port[2], smp_config);
    }

    DSP_MW_LOG_I("[BLE] N9Ble_UL_SWB_Sample_Rate_Init: channel_number %d, fs_in %d, fs_temp %d, fs_temp2 %d, fs_out %d", 5,
                 smp_config->channel_number, fs_in, fs_temp, fs_temp2, fs_out);

    if(smp_config) {
        vPortFree(smp_config);
    }
}

void N9Ble_UL_SWB_Sample_Rate_Deinit(void)
{
    uint32_t i;

    DSP_MW_LOG_I("[BLE]N9Ble_UL_SWB_Sample_Rate_Deinit", 0);

    for (i = 0 ; i < ARRAY_SIZE(g_n9ble_ul_swb_fixed_ratio_port) ; ++i) {
        if (g_n9ble_ul_swb_fixed_ratio_port[i]) {
            stream_function_src_fixed_ratio_deinit(g_n9ble_ul_swb_fixed_ratio_port[i]);
            g_n9ble_ul_swb_fixed_ratio_port[i] = NULL;
        }
    }
}

#endif
#if (defined(AIR_UL_FIX_SAMPLING_RATE_48K) && defined(AIR_FIXED_RATIO_SRC))
static src_fixed_ratio_port_t *g_n9ble_ul_src_fixed_ratio_port;

void N9Ble_UL_Fix_Sample_Rate_Init(void)
{
    uint32_t channel_number;
    DSP_STREAMING_PARA_PTR ul_stream;
    src_fixed_ratio_config_t smp_config = {0};
    volatile SINK sink = Sink_blks[SINK_TYPE_N9BLE];

    ul_stream = DSP_Streaming_Get(sink->transform->source, sink);
    channel_number = stream_function_get_channel_number(&(ul_stream->callback.EntryPara));

    smp_config.channel_number = channel_number;
    smp_config.in_sampling_rate = internal_fs_converter((stream_samplerate_t)(ul_stream->callback.EntryPara.in_sampling_rate));
    smp_config.out_sampling_rate = 16000;//internal_fs_converter((stream_samplerate_t)(ul_stream->callback.EntryPara.codec_out_sampling_rate));
    smp_config.resolution = ul_stream->callback.EntryPara.resolution.feature_res;
    smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_SINGLE;
    smp_config.cvt_num = 1;
    g_n9ble_ul_src_fixed_ratio_port = stream_function_src_fixed_ratio_get_port(sink);

    DSP_MW_LOG_I("[BLE] src_fixed_ratio_init: channel_number %d, in_sampling_rate %d, out_sampling_rate %d, resolution %d", 4,
                 smp_config.channel_number, smp_config.in_sampling_rate, smp_config.out_sampling_rate, smp_config.resolution);
    stream_function_src_fixed_ratio_init(g_n9ble_ul_src_fixed_ratio_port, &smp_config);
}

void N9Ble_UL_Fix_Sample_Rate_Deinit(void)
{
    if (g_n9ble_ul_src_fixed_ratio_port) {
        stream_function_src_fixed_ratio_deinit(g_n9ble_ul_src_fixed_ratio_port);
        g_n9ble_ul_src_fixed_ratio_port = NULL;
    }
}
#endif

#if defined(AIR_MUTE_MIC_DETECTION_ENABLE)
#include "volume_estimator_interface.h"
#include "stream_nvkey_struct.h"
#include "preloader_pisplit.h"
#include "audio_nvdm_common.h"
void *p_ble_ul_vol_estimator_nvkey_buf = NULL;
volume_estimator_port_t *p_ble_ul_meter_port;

void N9Ble_UL_Volume_Estimator_Init(SINK sink){
    /* init volume estimator port */
    p_ble_ul_vol_estimator_nvkey_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, sizeof(audio_spectrum_meter_nvkey_t));
    if (p_ble_ul_vol_estimator_nvkey_buf == NULL) {
        AUDIO_ASSERT(0);
    }
    if (nvkey_read_full_key(NVKEY_DSP_PARA_SILENCE_DETECTION2, p_ble_ul_vol_estimator_nvkey_buf, sizeof(audio_spectrum_meter_nvkey_t)) != NVDM_STATUS_NAT_OK) {
        AUDIO_ASSERT(0);
    }
    volume_estimator_config_t config;
    p_ble_ul_meter_port = volume_estimator_get_port(sink);
    if (p_ble_ul_meter_port == NULL) {
        AUDIO_ASSERT(0 && "[BLE] Audio Spectrum memter port is null.");
    }
    config.resolution = RESOLUTION_16BIT;
    config.frame_size = sink->transform->source->param.audio.rate/1000 * 15 * ((config.resolution == RESOLUTION_16BIT)? 2:4);
    //config.frame_size = sink->transform->source->param.audio.rate/1000 * sink->param.n9ble.frame_interval/1000 * ((config.resolution == RESOLUTION_16BIT)? 2:4);
    config.channel_num = 1;
    config.mode = VOLUME_ESTIMATOR_CHAT_INSTANT_MODE;
    config.sample_rate = sink->transform->source->param.audio.rate;
    config.nvkey_para = (void *)&(((audio_spectrum_meter_nvkey_t *)p_ble_ul_vol_estimator_nvkey_buf)->chat_vol_nvkey);
    config.internal_buffer = NULL;
    config.internal_buffer_size = 0;
    volume_estimator_init(p_ble_ul_meter_port, &config);
    DSP_MW_LOG_I("[BLE] volume estimator 0x%x info, %d, %d, %d,internal_buffer 0x%x, 0x%x\r\n", 6,
                p_ble_ul_meter_port,
                config.frame_size,
                config.channel_num,
                config.sample_rate,
                config.internal_buffer,
                config.internal_buffer_size);
}

void N9Ble_UL_Volume_Estimator_Deinit(void){
    volume_estimator_deinit(p_ble_ul_meter_port);
    if (p_ble_ul_vol_estimator_nvkey_buf) {
        preloader_pisplit_free_memory(p_ble_ul_vol_estimator_nvkey_buf);
        p_ble_ul_vol_estimator_nvkey_buf = NULL;
    }
    DSP_MW_LOG_I("[BLE] volume estimator deinit",0);
}
#endif

#ifdef AIR_BLE_UL_SW_GAIN_CONTROL_ENABLE
#ifdef AIR_SOFTWARE_GAIN_ENABLE
#include "sw_gain_interface.h"
#define BLE_SW_GAIN_MUTE_VALUE (-(96 * 100))
static bool g_ble_ul_sw_gain_is_ready;
static sw_gain_port_t *g_ble_ul_sw_gain_port;
static int32_t g_ble_ul_sw_gain_value;
extern bool g_call_mute_flag;

void N9Ble_UL_SW_Gain_Init(void)
{
    uint32_t i, channel_number;
    sw_gain_config_t sw_gain_config;
    DSP_STREAMING_PARA_PTR ul_stream;
    volatile SINK sink = Sink_blks[SINK_TYPE_N9BLE];

    ul_stream = DSP_Streaming_Get(sink->transform->source, sink);
    channel_number = stream_function_get_channel_number(&(ul_stream->callback.EntryPara));

    g_ble_ul_sw_gain_port = stream_function_sw_gain_get_port(sink);
    AUDIO_ASSERT(g_ble_ul_sw_gain_port != NULL);
    sw_gain_config.resolution = RESOLUTION_16BIT;
    sw_gain_config.target_gain = BLE_SW_GAIN_MUTE_VALUE;
    sw_gain_config.up_step = 1;
    sw_gain_config.up_samples_per_step = 2;
    sw_gain_config.down_step = -1;
    sw_gain_config.down_samples_per_step = 2;
    stream_function_sw_gain_init(g_ble_ul_sw_gain_port, channel_number, &sw_gain_config);
    for (i = 0; i < channel_number; i++) {
        stream_function_sw_gain_configure_gain_target(g_ble_ul_sw_gain_port, i + 1, BLE_SW_GAIN_MUTE_VALUE);
    }

    g_ble_ul_sw_gain_is_ready = true;
    g_call_mute_flag = true;
}

static void N9Ble_UL_SW_Gain_DeInit(void)
{
    if (g_ble_ul_sw_gain_port != NULL) {
        g_ble_ul_sw_gain_is_ready = false;
        stream_function_sw_gain_deinit(g_ble_ul_sw_gain_port);
        g_ble_ul_sw_gain_port = NULL;
    }
}

void N9Ble_UL_Set_SW_Gain(int32_t new_gain)
{
    sw_gain_config_t old_config;

    if (g_ble_ul_sw_gain_is_ready == true) {
        stream_function_sw_gain_get_config(g_ble_ul_sw_gain_port, 2, &old_config); /* Only channel 1 is usefull */
        DSP_MW_LOG_I("[DSP][BLE] UL set SW gain from %d*0.01dB to %d*0.01dB\r\n", 2, old_config.target_gain, new_gain);
        if(g_call_mute_flag){
            DSP_MW_LOG_I("[DSP][BLE] Call UL is mute, save new gain only.\r\n", 0);
        }else{
            stream_function_sw_gain_configure_gain_target(g_ble_ul_sw_gain_port, 1, new_gain); /* Only channel 1 is usefull */
        }
        g_ble_ul_sw_gain_value = new_gain;
    } else {
        DSP_MW_LOG_E("[DSP][BLE] UL set SW gain fail\r\n", 0);
    }
}

void N9Ble_UL_Set_SW_Gain_Mute(bool mute)
{
    sw_gain_config_t old_config;
    int32_t new_gain;

    if (g_ble_ul_sw_gain_is_ready == true) {
        if(mute){
            new_gain = -(96 * 100);
        }else{
            new_gain = g_ble_ul_sw_gain_value;
        }
        stream_function_sw_gain_get_config(g_ble_ul_sw_gain_port, 2, &old_config); /* Only channel 1 is usefull */
        DSP_MW_LOG_I("[DSP][BLE] UL set SW gain from %d*0.01dB to %d*0.01dB\r\n", 2, old_config.target_gain, new_gain);
        stream_function_sw_gain_configure_gain_target(g_ble_ul_sw_gain_port, 1, new_gain); /* Only channel 1 is usefull */
    } else {
        DSP_MW_LOG_E("[DSP][BLE] UL set SW gain fail\r\n", 0);
    }
}
#endif
#endif
/**
 * SinkSlackN9Ble
 *
 * Function to query the remain buffer free size of BLE sink.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_2 static U32 SinkSlackN9Ble(SINK sink)
{
//    DSP_MW_LOG_I("[BLE] SinkSlackN9Ble    sink frame_length: %d  process_number: %d", 2, sink->param.n9ble.frame_length, sink->param.n9ble.process_number);
    uint32_t write_num = sink->param.n9ble.process_number;
    uint32_t dummy_insert = sink->param.n9ble.dummy_insert_number;
    uint32_t dummy_status = sink->param.n9ble.dummy_process_status;
    if(dummy_insert || dummy_status) {
        DSP_MW_LOG_W("[BLE][sink] SinkSlackN9Ble dummy %d %d process: %d skip write", 3, dummy_insert, dummy_status, write_num);
        return sink->param.n9ble.frame_length * write_num;
    }
    if(ULL_NrOffloadFlag){
        return sink->param.n9ble.frame_length;
    }else{
        return sink->param.n9ble.frame_length * 3; /* always guarantee buffer for three frames*/
    }
}


/**
 * SinkClaimN9Ble
 *
 * Function to request the framework to write data into BLE sink.
 * Note: this function should NOT called by framework.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
static U32 SinkClaimN9Ble(SINK sink, U32 extra)
{
    UNUSED(sink);
    UNUSED(extra);

    DSP_MW_LOG_E("[BLE][sink] SinkClaimN9Ble called!!!", 0);
    AUDIO_ASSERT(0);

    return SINK_INVALID_CLAIM;
}

/**
 * SinkMapN9Ble
 *
 * Function to read the decoded data in BLE sink.
 * Note: this function should NOT called by framework.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
static U8 *SinkMapN9Ble(SINK sink)
{
    UNUSED(sink);

    DSP_MW_LOG_E("[BLE][sink] SinkMapN9Ble called!!!", 0);
    AUDIO_ASSERT(0);

    return 0;
}

/**
 * SinkFlushN9Ble
 *
 * Function to update the WPTR for BLE sink.
 *
 * param :amount - The amount of data written into sink.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
*/
ATTR_TEXT_IN_IRAM_LEVEL_2 static BOOL SinkFlushN9Ble(SINK sink, U32 amount)
{
    uint32_t ProcessFrameLen;
    DSP_STREAMING_PARA_PTR stream = NULL;
    stream = DSP_Streaming_Get(sink->transform->source, sink);
    stream->callback.EntryPara.out_channel_num = sink->param.n9ble.out_channel;

    if (ULL_NrOffloadFlag == true) {
        ProcessFrameLen = (g_ble_ul_abr_length - 1) * sink->param.n9ble.process_number;
    } else {
        ProcessFrameLen = g_ble_ul_abr_length * sink->param.n9ble.process_number;
    }

    if (amount % ProcessFrameLen) {
        DSP_MW_LOG_E("[BLE][sink] flush size mismatch %d, %d!!!", 2, amount, ProcessFrameLen);
        AUDIO_ASSERT(0);
        return FALSE;
    }

    uint32_t flush_num = sink->param.n9ble.process_number;

    if(sink->param.n9ble.dummy_insert_number && sink->param.n9ble.codec_type != BT_BLE_CODEC_LC3) {
        --sink->param.n9ble.dummy_insert_number;
#ifndef AIR_WIRELESS_MIC_TX_ENABLE
        flush_num <<= 1;
#endif
    }

    sink->param.n9ble.predict_timestamp += flush_num * ((sink->param.n9ble.frame_interval << 1) / LE_TIME_BT_CLOCK);
    sink->param.n9ble.predict_timestamp &= 0xFFFFFFF;
    if (flush_num) {
        N9BleTx_update_from_share_information(sink);
        N9Ble_SinkUpdateLocalWriteOffset(sink, flush_num);
        N9Ble_update_writeoffset_share_information(sink, sink->streamBuffer.ShareBufferInfo.write_offset);

        sink->param.n9ble.seq_num += flush_num;

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
#ifdef AIR_BT_LE_LC3PLUS_ENABLE
#ifdef AIR_AUDIO_DUMP_ENABLE
        /* dump channel 1 LC3+ codec in data */
        uint8_t *codec_in_data_address;
        uint32_t codec_in_data_frame_size;
        if (sink->param.n9ble.codec_type == BT_BLE_CODEC_LC3PLUS)
        {
            stream_codec_encoder_lc3plus_get_data_info(LC3PLUS_ENC_PORT_0, 1, &codec_in_data_address, &codec_in_data_frame_size);
            LOG_AUDIO_DUMP((uint8_t *)codec_in_data_address, codec_in_data_frame_size*2, AUDIO_SOURCE_IN_L);
        }
#endif
#endif
#endif
#ifdef AIR_BT_LE_LC3PLUS_ENABLE
        if ((sink->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE) && (sink->param.n9ble.codec_type == BT_BLE_CODEC_LC3PLUS) && ((sink->streamBuffer.ShareBufferInfo.asi_cur < sink->param.n9ble.predict_timestamp)&&(sink->streamBuffer.ShareBufferInfo.asi_base & 0x80000000)))
        {
            DSP_MW_LOG_I("[BLE][sink] flush change frame size %d, %d!!!", 2, amount, (sink->streamBuffer.ShareBufferInfo.asi_base & 0xFFFF) * 8 * 1000 / (sink->param.n9ble.frame_interval));
            stream_codec_encoder_lc3plus_set_bitrate(LC3PLUS_ENC_PORT_0, (sink->streamBuffer.ShareBufferInfo.asi_base & 0xFFFF) * 8 * 1000 / (sink->param.n9ble.frame_interval) * 1000 );
            sink->param.n9ble.share_info_base_addr->asi_base &= 0xFFFF;
            g_ble_ul_abr_length = (U16)sink->param.n9ble.share_info_base_addr->asi_base;
            DSP_MW_LOG_I("[BLE][sink] flush change frame size done %d!!!",1, g_ble_ul_abr_length);

        }
#endif
    }

    sink->param.n9ble.dummy_process_status = 0;

    /*
        DSP_MW_LOG_I("[BLE][sink] SinkFlushN9Ble flush_num: %d  sink local offset:%X", 2, flush_num, sink->streamBuffer.ShareBufferInfo.WriteOffset);
    */
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
    if (HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) {
        if (g_ble_dl_ul_process_active == false) {
            audio_nvdm_update_status(AUDIO_NVDM_USER_BLE, AUDIO_NVDM_STATUS_POST_CHANGE);
            g_ble_dl_ul_process_active = true;
        }
        if ((sink->param.n9ble.ul_reinit == true) && (Source_blks[SOURCE_TYPE_N9BLE] != NULL) && (Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.dl_reinit == true)) {
            Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.dl_reinit = false;
            sink->param.n9ble.ul_reinit = false;
            audio_nvdm_update_status(AUDIO_NVDM_USER_BLE, AUDIO_NVDM_STATUS_POST_CHANGE);
        }
    }
#endif
    return TRUE;
}

#ifdef AIR_ULL_BLE_HEADSET_ENABLE
extern const uint32_t crc32_tab[];

ATTR_TEXT_IN_IRAM_LEVEL_1 static uint32_t CRC32_Generate(uint8_t *ptr, uint32_t length, uint32_t crc_init)
{
    const uint8_t *p;

    p = ptr;
    crc_init = crc_init ^ ~0U;

    while (length--) {
        crc_init = crc32_tab[(crc_init ^ *p++) & 0xFF] ^ (crc_init >> 8);
    }

    return crc_init ^ ~0U;
}
#endif


/**
 * SinkBufferWriteN9Ble
 *
 * Function to write the framwork data to BLE sink.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_2 static BOOL SinkBufferWriteN9Ble(SINK sink, U8 *src_addr, U32 length)
{
    U16 i;
    U8 *write_ptr;
    uint32_t ProcessFrameLen;
    LE_AUDIO_HEADER *buf_header;
    U32 timestamp = sink->param.n9ble.predict_timestamp;
    uint32_t write_num = sink->param.n9ble.process_number;
    uint32_t dummy_insert = sink->param.n9ble.dummy_insert_number;
    uint32_t dummy_status = sink->param.n9ble.dummy_process_status;

    if(dummy_insert || dummy_status) {
        DSP_MW_LOG_W("[BLE][sink] dummy %d %d process: %d skip write", 3, dummy_insert, dummy_status, write_num);
        if(dummy_status) {
            return TRUE;
        }
    }

    if (ULL_NrOffloadFlag == true) {
        ProcessFrameLen = (g_ble_ul_abr_length - 1) * write_num;
    } else {
        ProcessFrameLen = g_ble_ul_abr_length * write_num;
    }
    if (length % ProcessFrameLen) {
        DSP_MW_LOG_E("[BLE][sink] write size mismatch %d, %d!!!", 2, length, ProcessFrameLen);
        AUDIO_ASSERT(0);
        return FALSE;
    }

    N9BleTx_update_from_share_information(sink);

    /*
        DSP_MW_LOG_I("SinkBufferWriteN9Ble src_addr:%X length:%d write_num:%d",3, src_addr, length, write_num);
    */

    for (i = 0 ; i < write_num; i++) {
        U8 *src_ptr = src_addr + (U32)(i * g_ble_ul_abr_length);
        write_ptr = (U8 *)(sink->streamBuffer.ShareBufferInfo.start_addr + sink->streamBuffer.ShareBufferInfo.write_offset);
        if(ULL_NrOffloadFlag){
            uint8_t PostEC_Gain = 0;
            memcpy(write_ptr + BLE_AVM_FRAME_HEADER_SIZE, src_ptr, g_ble_ul_abr_length-1);
#ifdef AIR_ECNR_PREV_PART_ENABLE
#ifndef AIR_ECNR_SEPARATE_MODE_ENABLE
            PostEC_Gain = voice_ecnr_ec_get_postec_gain();
#endif
#else
            PostEC_Gain = 0;
#endif
            memcpy(write_ptr + BLE_AVM_FRAME_HEADER_SIZE + g_ble_ul_abr_length-1, &PostEC_Gain, 1);
            DSP_MW_LOG_I("[DSP][VOICE_NR] send PostEC_Gain %d, sink->param.n9ble.frame_length %d", 2, PostEC_Gain, g_ble_ul_abr_length);
        }else{
            #ifdef SINE_PATTERN_TEST
            U32 pattern_idx = sink->param.n9ble.seq_num + i;

            if(pattern_idx < SINE_PATTERN_TEST_SKIP_CNT) {
                timestamp += ((sink->param.n9ble.frame_interval << 1) / LE_TIME_BT_CLOCK);
                N9Ble_SinkUpdateLocalWriteOffset(sink, 1);
                continue;
            } else {
                pattern_idx -= SINE_PATTERN_TEST_SKIP_CNT;
            }

            if(sink->param.n9ble.codec_type == BT_BLE_CODEC_LC3
                && sink->param.n9ble.sampling_rate == 32000
                && sink->param.n9ble.frame_interval == 10000
                && sink->param.n9ble.frame_length == 80)
            {
                if(pattern_idx == 0) {
                    src_ptr = sine_1k_10ms_64kbps_pattern[0];
                }
                else
                {
                    src_ptr = sine_1k_10ms_64kbps_pattern[1];
                }
            }
            else
            if(sink->param.n9ble.codec_type == BT_BLE_CODEC_LC3
                && sink->param.n9ble.sampling_rate == 32000
                && sink->param.n9ble.frame_interval == 7500
                && sink->param.n9ble.frame_length == 60)
            {
                if(pattern_idx < 5) {
                    src_ptr = sine_1k_7p5ms_64kbps_pattern[pattern_idx];
                }
                else
                {
                    src_ptr = sine_1k_7p5ms_64kbps_pattern[(pattern_idx - 5)%4 + 1];
                }
            }
            #endif

            memcpy(write_ptr + BLE_AVM_FRAME_HEADER_SIZE, src_ptr, g_ble_ul_abr_length);

            #if 0
            LOG_AUDIO_DUMP(src_ptr, sink->param.n9ble.frame_length, AUDIO_WOOFER_UPSAMPLE_16K);
            #endif
        }
        buf_header = (LE_AUDIO_HEADER *)write_ptr;
        buf_header->DataOffset = BLE_AVM_FRAME_HEADER_SIZE;
        buf_header->Pdu_LEN_Without_MIC = g_ble_ul_abr_length;
        buf_header->DataLen = g_ble_ul_abr_length;
        buf_header->TimeStamp = timestamp;
        #ifdef AIR_ULL_BLE_HEADSET_ENABLE
        buf_header->crc32_value = CRC32_Generate((uint8_t *)src_ptr, g_ble_ul_abr_length, sink->param.n9ble.crc_init);
        #endif
        timestamp += ((sink->param.n9ble.frame_interval << 1) / LE_TIME_BT_CLOCK);
        //DSP_MW_LOG_I("[BLE][sink]offset %d, ts %d!!!", 2, sink->streamBuffer.ShareBufferInfo.WriteOffset, buf_header->TimeStamp);
        N9Ble_SinkUpdateLocalWriteOffset(sink, 1);
    }
    /*
        DSP_MW_LOG_I("[BLE][sink] SinkBufferWriteN9Ble sink local offset:%X", 1, sink->streamBuffer.ShareBufferInfo.WriteOffset);
    */
    return TRUE;
}

/**
 * SinkCloseN9Ble
 *
 * Function to shutdown BLE sink.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
static BOOL SinkCloseN9Ble(SINK sink)
{
    sink->param.n9ble.frame_length = 0;
    sink->param.n9ble.process_number = 0;
    sink->param.n9ble.share_info_base_addr = NULL;
    sink->param.n9ble.seq_num = 0;

#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
    N9Ble_UL_SWB_Sample_Rate_Deinit();
#endif

#if defined(AIR_UL_FIX_SAMPLING_RATE_48K) && defined(AIR_FIXED_RATIO_SRC)
    N9Ble_UL_Fix_Sample_Rate_Deinit();
#endif

if ((sink->param.n9ble.context_type != BLE_CONTENT_TYPE_ULL_BLE) && (sink->param.n9ble.context_type != BLE_CONTENT_TYPE_WIRELESS_MIC)) {
#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
    N9Ble_UL_Volume_Estimator_Deinit();
#endif
#if defined(AIR_BLE_UL_SW_GAIN_CONTROL_ENABLE) && defined(AIR_SOFTWARE_GAIN_ENABLE)
    N9Ble_UL_SW_Gain_DeInit();
#endif
}

#ifdef AIR_BT_LE_LC3PLUS_ENABLE
    if(sink->param.n9ble.codec_type == BT_BLE_CODEC_LC3PLUS ) {
        DSP_MW_LOG_I("[BLE][LC3PLUS_ENC] deinit\r\n", 0);
        stream_codec_encoder_lc3plus_deinit(LC3PLUS_ENC_PORT_0,sink);
    }
#endif
#ifdef AIR_CELT_ENC_V2_ENABLE
    if(sink->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR ) {
        DSP_MW_LOG_I("[BLE][CELT] deinit\r\n", 0);
        stream_codec_encoder_celt_v2_deinit(CELT_ENC_PORT_0,sink);
    }
#endif

    return TRUE;
}

/**
 * SinkInitN9Ble
 *
 * Function to initialize BLE sink.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
VOID SinkInitN9Ble(SINK sink)
{
    N9Ble_Default_setting_init(NULL, sink);

    sink->buftype = BUFFER_TYPE_CIRCULAR_BUFFER;
    N9Ble_Tx_Buffer_Init(sink);

    sink->sif.SinkSlack       = SinkSlackN9Ble;
    sink->sif.SinkClaim       = SinkClaimN9Ble;
    sink->sif.SinkMap         = SinkMapN9Ble;
    sink->sif.SinkFlush       = SinkFlushN9Ble;
    sink->sif.SinkClose       = SinkCloseN9Ble;
    sink->sif.SinkWriteBuf    = SinkBufferWriteN9Ble;

    sink->param.n9ble.IsFirstIRQ = TRUE;
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
    sink->param.n9ble.ul_reinit = false;
#endif
    sink->param.n9ble.seq_num = 0;
}

/**
 * SourceSizeN9Ble
 *
 * Function to report remaining Source buffer avail size.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM static U32 SourceSizeN9Ble(SOURCE source)
{
#ifdef MTK_PEQ_ENABLE
    BTCLK bt_clk;
    BTPHASE bt_phase;
#endif
    U32 writeOffset, readOffset, length, ProcessFrameLen, RemainLen;
    LE_AUDIO_HEADER *buf_header;
    U8 *read_ptr;
    N9BleRx_update_from_share_information(source);

    writeOffset = source->streamBuffer.ShareBufferInfo.write_offset;
    readOffset  = source->streamBuffer.ShareBufferInfo.read_offset;
    length      = source->streamBuffer.ShareBufferInfo.length;
    read_ptr = (U8 *)(source->streamBuffer.ShareBufferInfo.start_addr + source->streamBuffer.ShareBufferInfo.read_offset);
    buf_header = (LE_AUDIO_HEADER *)read_ptr;

    ProcessFrameLen = source->param.n9ble.process_number * ALIGN_4(source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE);
    RemainLen = (readOffset > writeOffset) ? (length - readOffset + writeOffset) : (writeOffset - readOffset);
    if (source->streamBuffer.ShareBufferInfo.bBufferIsFull == true) {
        RemainLen = length;
    }

    //if ((source->param.n9ble.write_offset_advance == 0) && (RemainLen < ProcessFrameLen)) {
    //    DSP_MW_LOG_E("[BLE][source] AVM buffer detect underflow!!! RO: %d, WO: %d", 2, readOffset, writeOffset);
    //}

    //if(source->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE){
    //    g_ble_abr_length = source->param.n9ble.frame_length;
    //}else
    {
        if( N9Ble_check_avm_frame_valid(buf_header, FALSE)
            && (buf_header->Pdu_LEN_Without_MIC != g_ble_abr_length)
        )
        {
            if(buf_header->Pdu_LEN_Without_MIC <= source->param.n9ble.frame_length)
            {
                g_ble_abr_length = buf_header->Pdu_LEN_Without_MIC;
                DSP_MW_LOG_I("[BLE][source] ABR length: %d", 1, g_ble_abr_length);
            }
            else
            {
                DSP_MW_LOG_W("[BLE][source] ABR length: %d > %d, keep %d", 3, buf_header->Pdu_LEN_Without_MIC, source->param.n9ble.frame_length, g_ble_abr_length);
            }
        }
    }
    ProcessFrameLen = (source->param.n9ble.dual_cis_status != DUAL_CIS_DISABLED) ? g_ble_abr_length * source->param.n9ble.process_number * 2 : g_ble_abr_length * source->param.n9ble.process_number;
    ProcessFrameLen += source->param.n9ble.plc_state_len;

#ifdef MTK_PEQ_ENABLE
    MCE_GetBtClk(&bt_clk, &bt_phase, BT_CLK_Offset);
    PEQ_Update_Info(bt_clk, source->param.n9ble.predict_timestamp);
#endif
#ifdef AIR_CELT_DEC_V2_ENABLE
    if(source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR){
        ProcessFrameLen = g_ble_abr_length + 2*source->param.n9ble.plc_state_len;
    }
#endif

    return (source->param.n9ble.dl_afe_skip_time) ? 0 : ProcessFrameLen;

}

/**
 * SourceMapN9Ble
 *
 * Function to  read the received data in BLE source.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
static U8 *SourceMapN9Ble(SOURCE source)
{
    UNUSED(source);
    AUDIO_ASSERT(0);

    return NULL;
}

/**
 * SourceDropN9Ble
 *
 * Function to drop the data in BLE sink.
 *
 * param :amount - The amount of data to drop in sink.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_2 static VOID SourceDropN9Ble(SOURCE source, U32 amount)
{
    U16 i;
    U32 ProcessFrameLen;

    N9BleRx_update_from_share_information(source);

#ifdef AIR_CELT_DEC_V2_ENABLE
    if(source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR){
        ProcessFrameLen = g_ble_abr_length + 2*source->param.n9ble.plc_state_len;
    } else {
#endif
    ProcessFrameLen = (g_ble_abr_length * source->param.n9ble.process_number) << (source->param.n9ble.dual_cis_status != DUAL_CIS_DISABLED);
    ProcessFrameLen += source->param.n9ble.plc_state_len;
#ifdef AIR_CELT_DEC_V2_ENABLE
    }
#endif

    if (amount != ProcessFrameLen) {
        DSP_MW_LOG_E("[BLE][source] SourceDropN9Ble drop size mismatch %d, %d!!!", 2, amount, ProcessFrameLen);
        return;
    } else {
        for (i = 0 ; i < N9BLE_setting.N9Ble_source.Process_Frame_Num; i++) {
            LE_AUDIO_HEADER *buf_header;
            buf_header = (LE_AUDIO_HEADER *)(source->streamBuffer.ShareBufferInfo.start_addr + source->streamBuffer.ShareBufferInfo.read_offset);
            N9Ble_update_timestamp_share_information(&buf_header->TimeStamp, BLE_AVM_INVALID_TIMESTAMP); /* invalid the timestamp */
            if (source->param.n9ble.dual_cis_status != DUAL_CIS_DISABLED) {

                buf_header = (LE_AUDIO_HEADER *)((U32)source->param.n9ble.sub_share_info_base_addr + ((source->streamBuffer.ShareBufferInfo.read_offset + source->param.n9ble.dual_cis_buffer_offset)%source->streamBuffer.ShareBufferInfo.length));
                N9Ble_update_timestamp_share_information(&buf_header->TimeStamp, BLE_AVM_INVALID_TIMESTAMP); /* invalid the timestamp */
            }
            N9Ble_SourceUpdateLocalReadOffset(source, 1);
        }
        //DSP_MW_LOG_I("[BLE] source drop, update ro to:%d",1, source->streamBuffer.ShareBufferInfo.ReadOffset);
        N9Ble_update_readoffset_share_information(source, source->streamBuffer.ShareBufferInfo.read_offset);
    }
    source->param.n9ble.predict_frame_counter++;
    if (source->param.n9ble.predict_frame_counter == source->param.n9ble.frame_per_iso) {
        source->param.n9ble.predict_frame_counter = 0;
        source->param.n9ble.predict_timestamp += ((source->param.n9ble.iso_interval << 1) / LE_TIME_BT_CLOCK);
        source->param.n9ble.predict_timestamp = (source->param.n9ble.predict_timestamp & 0xFFFFFFF);
    }
    source->param.n9ble.write_offset_advance = 0;
    if (source->param.n9ble.seq_miss_cnt > LE_AUDIO_OFFSET_PROTECT) {
        LE_AUDIO_HEADER *buf_header;
        buf_header = (LE_AUDIO_HEADER *)(source->streamBuffer.ShareBufferInfo.start_addr + source->streamBuffer.ShareBufferInfo.read_offset);
        DSP_MW_LOG_E("[BLE][source] drop detect timestamp mismatch trigger offset re-sync,next TS : %d", 1, buf_header->TimeStamp);

        if (buf_header->TimeStamp <= source->param.n9ble.predict_timestamp) {
            N9Ble_SourceUpdateLocalReadOffset(source, (source->param.n9ble.predict_timestamp - buf_header->TimeStamp) / ((source->param.n9ble.frame_interval << 1) / LE_TIME_BT_CLOCK));
            DSP_MW_LOG_E("[BLE][source] offset adjust: %d", 1, (source->param.n9ble.predict_timestamp - buf_header->TimeStamp) / ((source->param.n9ble.frame_interval << 1) / LE_TIME_BT_CLOCK));
            source->param.n9ble.seq_miss_cnt = 0;
        } else if (buf_header->TimeStamp != BLE_AVM_INVALID_TIMESTAMP && (buf_header->_reserved_byte_0Dh & 0x01)) {
            N9Ble_SourceUpdateLocalReadOffset(source, 1);
        }
        N9Ble_update_readoffset_share_information(source, source->streamBuffer.ShareBufferInfo.read_offset);
    }
    source->param.n9ble.seq_num += source->param.n9ble.process_number;
    if (source->param.n9ble.skip_after_drop == TRUE) {
        DSP_MW_LOG_E("[BLE] SOURCE skip after drop", 0);
    }
    U32 mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (source->param.n9ble.skip_after_drop == TRUE) {
        source->param.n9ble.dl_afe_skip_time += 2;
        source->param.n9ble.skip_after_drop = FALSE;
    }
    hal_nvic_restore_interrupt_mask(mask);
}

/**
 * SourceConfigureN9Ble
 *
 * Function to configure BLE source.
 *
 * param :type - The configure type.
 *
 * param :value - The configure value.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
static BOOL SourceConfigureN9Ble(SOURCE source, stream_config_type type, U32 value)
{
    switch (type) {
        case SCO_SOURCE_WO_ADVANCE:
            source->param.n9ble.write_offset_advance = value;
            break;
        default:
            DSP_MW_LOG_E("[BLE][source] SourceConfigureN9Ble call with error type %d, value %d!!!", 2, type, value);
            AUDIO_ASSERT(0);
            return FALSE;
    }

    return TRUE;
}

/**
 * SourceReadBufN9Ble
 *
 * Function to read data from BLE source.
 *
 * param :dst_addr - The destination buffer to write data into.
 *
 * param :length -The leng of data to read.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_2 static BOOL SourceReadBufN9Ble(SOURCE source, U8 *dst_addr, U32 length)
{
    U16 i, j, cis_num;
    U8 *read_ptr;
    U8 *write_ptr;
    U32 ProcessFrameLen;
    LE_AUDIO_HEADER *buf_header;

    ProcessFrameLen = (g_ble_abr_length * source->param.n9ble.process_number) << (source->param.n9ble.dual_cis_status != DUAL_CIS_DISABLED);
    ProcessFrameLen += source->param.n9ble.plc_state_len;

#if (defined(AIR_BT_LE_LC3PLUS_ENABLE))
    U32* plc_report_addr = (U32*)dst_addr;
#endif

#if defined(AIR_CELT_DEC_V2_ENABLE) || defined(AIR_AUDIO_ULD_DECODE_ENABLE)
    TRANSFORM transform =  source->transform;
    DSP_CALLBACK_PTR callback_ptr = NULL;
    U32 frame_num = 1;
    if(source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR || source->param.n9ble.codec_type == BT_BLE_CODEC_ULD){
        if (transform != NULL) {
            callback_ptr = DSP_Callback_Get(source, transform->sink);
            dst_addr = callback_ptr->EntryPara.in_ptr[0];
        }
        if (source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR) {
            ProcessFrameLen = g_ble_abr_length + 2*source->param.n9ble.plc_state_len;
            frame_num = 2;
        }
    }
#endif

    if (ProcessFrameLen != length) {
        DSP_MW_LOG_E("[BLE][source] SourceReadBufN9Ble found size mismatch %d, %d!!!", 2, length, ProcessFrameLen);
        //return FALSE;
    }
    #ifdef AIR_BT_LE_LC3PLUS_ENABLE
    if (source->param.n9ble.plc_state_len != 0)
    {
        if(source->param.n9ble.codec_type == BT_BLE_CODEC_LC3PLUS) {
            *plc_report_addr = LC3PLUS_DEC_FRAME_STATUS_NORMAL;
            dst_addr += source->param.n9ble.plc_state_len;
        }
    }
    #endif

    N9BleRx_update_from_share_information(source);
    //Clock_Skew_Offset_Update_BLE(source);

    cis_num = (source->param.n9ble.dual_cis_status == DUAL_CIS_DISABLED) ? 1 : 2;/*SINGLE CIS or DUAL CIS*/
    for (i = 0; i < source->param.n9ble.process_number; i++) {
        for (j = 0; j < cis_num; j++) {
            n9_dsp_share_info_ptr base_addr;
            uint32_t RO, WO;
            if (j == 0) {
                base_addr = source->param.n9ble.share_info_base_addr;
                RO = source->streamBuffer.ShareBufferInfo.read_offset;
                read_ptr = (U8 *)(source->streamBuffer.ShareBufferInfo.start_addr + RO);
            } else {
                base_addr = source->param.n9ble.sub_share_info_base_addr;
                RO = (source->streamBuffer.ShareBufferInfo.read_offset + source->param.n9ble.dual_cis_buffer_offset)%source->streamBuffer.ShareBufferInfo.length;
                read_ptr = (U8 *)((U32)source->param.n9ble.sub_share_info_base_addr + RO);
            }
            WO = base_addr->write_offset;
            write_ptr = (source->param.n9ble.dual_cis_status == DUAL_CIS_DISABLED) ? dst_addr + i * g_ble_abr_length : dst_addr + (i * 2 + j) * g_ble_abr_length;

            buf_header = (LE_AUDIO_HEADER *)read_ptr;
           //DSP_MW_LOG_I("[BLE][source] Start_addr = 0x%x ,TimeStamp = 0x%08x, PduLen = %d", 3, source->streamBuffer.ShareBufferInfo.start_addr, buf_header->TimeStamp, buf_header->Pdu_LEN_Without_MIC);

#if 0
            DSP_MW_LOG_I("[BLE_DEBUG] BEFORE info_addr = 0x%08x, predict = 0x%08x, TimeStamp = 0x%08x, valid = %d cnt= %d, RO = %d, WO = %d", 7,
                source->param.n9ble.share_info_base_addr,
                source->param.n9ble.predict_timestamp,
                buf_header->TimeStamp,
                buf_header->_reserved_byte_0Dh,
                buf_header->EventCount,
                source->streamBuffer.ShareBufferInfo.read_offset,
                source->param.n9ble.share_info_base_addr->write_offset
            );
#endif
        U8 ble_pkt_lost_state_prev = g_ble_pkt_lost[i][j];
        if (N9Ble_check_avm_frame_valid(buf_header, TRUE)) {
            if (buf_header->_reserved_byte_0Dh == 0x03) {
                g_ble_pkt_lost[i][j] = BLE_PKT_PADDING;
#if defined(AIR_CELT_DEC_V2_ENABLE) || defined(AIR_AUDIO_ULD_DECODE_ENABLE)
                U32 combinedSize = ALIGN_4((source->param.n9ble.plc_state_len) + g_ble_abr_length/frame_num); /*4B align is needed because of decoder interface*/
                if(source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR || source->param.n9ble.codec_type == BT_BLE_CODEC_ULD) {
                    memset(callback_ptr->EntryPara.in_ptr[j], 0, combinedSize*frame_num);
                } else {
#endif
                memset(write_ptr, 0, g_ble_abr_length);
#if defined(AIR_CELT_DEC_V2_ENABLE) || defined(AIR_AUDIO_ULD_DECODE_ENABLE)
                }
#endif
            } else {
                g_ble_pkt_lost[i][j] = BLE_PKT_VALID;
#if defined(AIR_CELT_DEC_V2_ENABLE) || defined(AIR_AUDIO_ULD_DECODE_ENABLE)
                if(source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR || source->param.n9ble.codec_type == BT_BLE_CODEC_ULD) {
                    N9ble_add_status_to_frames_in_packet(source, callback_ptr->EntryPara.in_ptr[j], read_ptr,  g_ble_abr_length, frame_num, 0); //status:0 -> Normal
                } else {
#endif
                memcpy(write_ptr, read_ptr + BLE_AVM_FRAME_HEADER_SIZE, g_ble_abr_length);

#if defined(AIR_CELT_DEC_V2_ENABLE) || defined(AIR_AUDIO_ULD_DECODE_ENABLE)
                }
#endif
            }

            if(!source->param.n9ble.predict_packet_cnt[j]) {
                source->param.n9ble.predict_packet_cnt[j] = buf_header->EventCount;
            }

            if(source->param.n9ble.predict_packet_cnt[j] != buf_header->EventCount) {
                DSP_MW_LOG_W("[BLE][source][%d] Rx packet cnt mismatch %d != %d RO:%d WO:%d", 5
                    , j
                    , source->param.n9ble.predict_packet_cnt[j]
                    , buf_header->EventCount
                    , RO
                    , WO
                );
                source->param.n9ble.predict_packet_cnt[j] = 0;
            }


           if (ble_pkt_lost_state_prev != g_ble_pkt_lost[i][j]) {
                DSP_MW_LOG_W("[BLE][source][%d] Rx packet RECV%d with SEQ: %6d, PTS:%8d, TS: %8d, DIFF: %5d, PCNT: %8d, CNT: %4d, VALID: 0x%X, RO: %4d, WO: %4d", 11
                    , j
                    , g_ble_pkt_lost[i][j]
                    , source->param.n9ble.seq_num + i
                    , source->param.n9ble.predict_timestamp
                    , buf_header->TimeStamp
                    , source->param.n9ble.predict_timestamp - buf_header->TimeStamp
                    , source->param.n9ble.predict_packet_cnt[j]
                    , buf_header->EventCount
                    , buf_header->_reserved_byte_0Dh
                    , RO
                    , WO
                );
            }
            source->param.n9ble.pkt_lost_cnt[j] = 0;

            if (((abs32((S32)(buf_header->TimeStamp - source->param.n9ble.predict_timestamp)) * 625) >> 1) > source->param.n9ble.iso_interval) {
                DSP_MW_LOG_W("[BLE][source][%d] ts info %d %d index:%d mis_cnt:%d", 5, j, buf_header->TimeStamp, source->param.n9ble.predict_timestamp, RO, source->param.n9ble.seq_miss_cnt);
                if (j == 0) {
                    source->param.n9ble.seq_miss_cnt++;
                }
            } else {
                if (j == 0) {
                    source->param.n9ble.seq_miss_cnt = 0;
                }
            }
#if 0
            if(source->param.n9ble.predict_packet_cnt && source->param.n9ble.share_info_base_addr->write_offset == source->streamBuffer.ShareBufferInfo.read_offset) {
                DSP_MW_LOG_E("[BLE][source] AVM RW access conflict seq: %d, PTS: %d, RO: %d, WO %d", 4
                    , source->param.n9ble.seq_num + i
                    , source->param.n9ble.predict_timestamp
                    , source->streamBuffer.ShareBufferInfo.read_offset
                    , source->param.n9ble.share_info_base_addr->write_offset
                );
            }
#endif
        } else {
            g_ble_pkt_lost[i][j] = BLE_PKT_LOST;
#if defined(AIR_CELT_DEC_V2_ENABLE) || defined(AIR_AUDIO_ULD_DECODE_ENABLE)
            if(source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR || source->param.n9ble.codec_type == BT_BLE_CODEC_ULD) {
                N9ble_add_status_to_frames_in_packet(source, callback_ptr->EntryPara.in_ptr[j], read_ptr,g_ble_abr_length, frame_num, 1);//status:1 -> PLC
            } else {
#endif
            memset(write_ptr, 0, g_ble_abr_length);
#if defined(AIR_CELT_DEC_V2_ENABLE) || defined(AIR_AUDIO_ULD_DECODE_ENABLE)
            }
#endif
            if (ble_pkt_lost_state_prev != g_ble_pkt_lost[i][j]) {
                DSP_MW_LOG_W("[BLE][source][%d] Rx packet LOST  with SEQ: %6d, PTS:%8d, TS: %8d, DIFF: %5d, PCNT: %8d, CNT: %4d, VALID: 0x%X, RO: %4d, WO: %4d", 10
                    , j
                    , source->param.n9ble.seq_num + i
                    , source->param.n9ble.predict_timestamp
                    , buf_header->TimeStamp
                    , -1
                    , source->param.n9ble.predict_packet_cnt[j]
                    , buf_header->EventCount
                    , buf_header->_reserved_byte_0Dh
                    , RO
                    , WO
                );
            }
            source->param.n9ble.pkt_lost_cnt[j] = 1;

            #ifdef AIR_BT_LE_LC3PLUS_ENABLE
            if (source->param.n9ble.plc_state_len != 0)
            {
                if(source->param.n9ble.codec_type == BT_BLE_CODEC_LC3PLUS) {
                    *plc_report_addr = LC3PLUS_DEC_FRAME_STATUS_PLC;
                }
            }
            #endif
            g_pkt_lost_count++;
        }

        if(source->param.n9ble.predict_packet_cnt[j]) {
            ++source->param.n9ble.predict_packet_cnt[j];
        }

#if 0
        DSP_MW_LOG_I("[BLE_DEBUG] AFTER info_addr = 0x%08x, predict = 0x%08x, TimeStamp = 0x%08x, valid = %d cnt= %d, RO = %d, WO = %d", 7,
            source->param.n9ble.share_info_base_addr,
            source->param.n9ble.predict_timestamp,
            buf_header->TimeStamp,
            buf_header->_reserved_byte_0Dh,
            buf_header->EventCount,
            source->streamBuffer.ShareBufferInfo.read_offset,
            //source->streamBuffer.ShareBufferInfo.write_offset
            source->param.n9ble.share_info_base_addr->write_offset
        );

#endif
        }
        N9Ble_SourceUpdateLocalReadOffset(source, 1);
    }

    return TRUE;
}

bool ble_query_rx_packet_lost_status(uint32_t index)
{
    return g_ble_pkt_lost[index][0];
}
bool ble_query_rx_sub_cis_packet_lost_status(uint32_t index)
{
    return g_ble_pkt_lost[index][1];
}
/**
 * SourceCloseN9Ble
 *
 * Function to shutdown BLE source.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
static BOOL SourceCloseN9Ble(SOURCE source)
{
    source->param.n9ble.frame_length = 0;
    source->param.n9ble.process_number = 0;
    source->param.n9ble.share_info_base_addr = NULL;
    source->param.n9ble.seq_num = 0;

#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
    N9Ble_DL_SWB_Sample_Rate_Deinit();
#endif

#ifdef AIR_BT_LE_LC3PLUS_ENABLE
       if(source->param.n9ble.codec_type == BT_BLE_CODEC_LC3PLUS ) {
           DSP_MW_LOG_I("[BLE][LC3PLUS_DEC] deinit\r\n", 0);
           stream_codec_decoder_lc3plus_deinit(LC3PLUS_DEC_PORT_0,source);
        }
#endif

#ifdef AIR_CELT_DEC_V2_ENABLE
       if(source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR) {
           DSP_MW_LOG_I("[BLE][CELT] deinit\r\n", 0);
           stream_codec_decoder_celt_v2_deinit(CELT_DEC_PORT_0,source);
       }
#endif
#if AIR_AUDIO_ULD_DECODE_ENABLE
           if(source->param.n9ble.codec_type == BT_BLE_CODEC_ULD) {
               DSP_MW_LOG_I("[BLE][ULD] deinit\r\n", 0);
               stream_codec_decoder_uld_deinit(source);
           }
#endif

    DSP_MW_LOG_I("[BLE][source] total packet lost count %d", 1, g_pkt_lost_count);

    return TRUE;
}

/**
 * SourceInitBle
 *
 * Function to initialize BLE source.
 *
 * @Author : RyanHung <Ryan.Hung@airoha.com.tw>
 */
VOID SourceInitN9Ble(SOURCE source)
{
    N9Ble_Default_setting_init(source, NULL);

    source->buftype = BUFFER_TYPE_CIRCULAR_BUFFER;
    N9Ble_Rx_Buffer_Init(source);

    source->sif.SourceSize       = SourceSizeN9Ble;
    source->sif.SourceReadBuf    = SourceReadBufN9Ble;
    source->sif.SourceMap        = SourceMapN9Ble;
    source->sif.SourceConfigure  = SourceConfigureN9Ble;
    source->sif.SourceDrop       = SourceDropN9Ble;
    source->sif.SourceClose      = SourceCloseN9Ble;

    source->param.n9ble.IsFirstIRQ = TRUE;
    source->param.n9ble.write_offset_advance = 1; // force stream process to prevent first package lost
    source->param.n9ble.seq_num = 0;
    source->param.n9ble.predict_packet_cnt[0] = 0;
    source->param.n9ble.predict_packet_cnt[1] = 0;
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
    source->param.n9ble.dl_reinit = false;
    g_ble_dl_ul_process_active = false;
#endif
    g_pkt_lost_count = 0;
    memset(g_ble_pkt_lost, 0, sizeof(g_ble_pkt_lost));
}

VOID SourceInitBleAvm(SOURCE source,SINK sink)
{
    N9Ble_Default_setting_init(source, sink);
    if(source != NULL){
        N9BleRx_update_from_share_information(source);
    }
    if(sink != NULL){
        N9BleTx_update_from_share_information(sink);
    }
}


////////////////////////////////////////////////////////////////////////////////
//              BLE Audio playback source related
////////////////////////////////////////////////////////////////////////////////

#define BLE_UL_MAX_NUM                  2

typedef struct {
    uint32_t buffer_start[BLE_UL_MAX_NUM];
    uint32_t buffer_offset[BLE_UL_MAX_NUM];
    uint16_t buffer_index;
    uint16_t buffer_index_max;
    uint16_t buffer_size;
    uint16_t frame_head_len;
    uint16_t frame_data_len;
    uint8_t  is_source;
    uint8_t  is_playback_mode;
    uint8_t  is_1k_tone_mode;
    uint8_t  sample_rate;
    uint8_t  bitrate;
    uint8_t  num_of_sink;
    uint8_t  seq_num;
} LE_AUDIO_SOURCE_CTRL;

static LE_AUDIO_SOURCE_CTRL le_source;
static n9_dsp_share_info_ptr ul_info[BLE_UL_MAX_NUM];
static n9_dsp_share_info_ptr dl_info;

void CB_N9_BLE_UL_INFO(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);

    uint16_t index = (uint16_t)msg.ccni_message[0];

    le_source.is_source = TRUE; //this CCNI should only be used by SOURCE device
    le_source.is_playback_mode = TRUE;

    switch (index) {
        case 0x5A5A:
            le_source.is_playback_mode = FALSE;
            ul_info[0] = NULL;
            ul_info[1] = NULL;
            return;
        case 1:
            ul_info[0] = (msg.ccni_message[1] == 0) ? NULL : (n9_dsp_share_info_ptr)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
            ul_info[1] = NULL;

            le_source.buffer_start[0]  = hal_memview_cm4_to_dsp0(ul_info[0]->start_addr);
            le_source.buffer_offset[0] = 0;

            //DSP_MW_LOG_I("[le audio DSP] ul1 info = %X  ul1 buffer = %X", 2, ul_info[0], le_source.buffer_start[0]);
            break;
        case 2:
            ul_info[1] = (msg.ccni_message[1] == 0) ? NULL : (n9_dsp_share_info_ptr)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

            le_source.buffer_start[1]  = hal_memview_cm4_to_dsp0(ul_info[1]->start_addr);
            le_source.buffer_offset[1] = 0;

            //DSP_MW_LOG_I("[le audio DSP] ul2 info = %X  ul2 buffer = %X", 2, ul_info[1], le_source.buffer_start[1]);
            break;
        default:
            //DSP_MW_LOG_I("[le audio DSP] invalid UL info", 0);
            break;
    }
}

void CB_N9_BLE_DL_INFO(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);

    le_source.is_source = TRUE; //this CCNI should only be used by SOURCE device
    le_source.is_playback_mode = FALSE;

    dl_info = (msg.ccni_message[1] == 0) ? NULL : (n9_dsp_share_info_ptr)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    //DSP_MW_LOG_I("[le audio DSP] dl buffer = %X", 1, dl_info);

    if (le_source.is_playback_mode) {
        //DSP_MW_LOG_I("[le audio DSP] Music mode (playback)", 0);
    } else {
        //DSP_MW_LOG_I("[le audio DSP] Voice mode (loopback)", 0);
    }
}

void CB_N9_BLE_UL_UPDATE_TIMESTAMP(hal_ccni_event_t event, void *msg)
{
    uint32_t i, bt_count, buffer_offset;
    hal_ccni_message_t *ccni_msg = msg;
    LE_AUDIO_HEADER *buf_header = NULL;
    uint8_t *avm_buffer;
    uint32_t saved_mask;
    uint32_t buf_index;
    UNUSED(event);
    UNUSED(msg);
    UNUSED(avm_buffer);

    bt_count = ccni_msg->ccni_message[0];

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    buf_index = le_source.buffer_index;
    if ((le_source.is_source) && (le_source.is_playback_mode)) {
        buffer_offset = (ALIGN_4(le_source.frame_head_len + le_source.frame_data_len)) * le_source.buffer_index;

        for (i = 0; i < BLE_UL_MAX_NUM; i++) {
            if (ul_info[i]) {
                buf_header = (LE_AUDIO_HEADER *)(le_source.buffer_start[i] + buffer_offset);
                buf_header->TimeStamp = bt_count;
            }
        }

        le_source.buffer_index = (le_source.buffer_index + 1) % le_source.buffer_index_max;
        le_source.seq_num = (le_source.seq_num + 1) & 0xff;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    //DSP_MW_LOG_I("[le audio DSP] irq update timestamp, 0x%x, 0x%x, 0x%x", 3, buf_header, buf_index, bt_count);
}

void CB_N9_BLE_UL_PLAYBACK_DATA_INFO(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint32_t saved_mask;
    UNUSED(ack);

    if ((uint16_t)msg.ccni_message[0] == 0xFFFF) {
        le_source.is_1k_tone_mode = !le_source.is_1k_tone_mode;
        //DSP_MW_LOG_I("[le audio DSP] toggle playback source between line_in/1K tone", 0);
    } else if ((uint16_t)msg.ccni_message[0] == 0x9999) {
//        hal_audio_trigger_start_parameter_t sw_trigger_start;
//        sw_trigger_start.enable = true;
//        sw_trigger_start.memory_select = playback_vp_if.sink->param.audio.mem_handle.memory_select;
//        hal_audio_set_value((hal_audio_set_value_parameter_t *)&sw_trigger_start,HAL_AUDIO_SET_TRIGGER_MEMORY_START);
        //DSP_MW_LOG_I("[le audio DSP] trigger line-in irq start", 0);
    } else {
        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
        le_source.is_source = TRUE;
        le_source.is_playback_mode = TRUE;
        le_source.frame_head_len = (uint16_t)BLE_AVM_FRAME_HEADER_SIZE;
        le_source.frame_data_len = (uint16_t)msg.ccni_message[1];
        le_source.sample_rate    = (uint8_t)(msg.ccni_message[1] >> 16);
        le_source.bitrate        = (uint8_t)(msg.ccni_message[1] >> 24);
        le_source.num_of_sink    = (uint16_t)msg.ccni_message[0];
        le_source.buffer_size    = ALIGN_4(le_source.frame_data_len + le_source.frame_head_len) * N9Ble_calculate_avm_frame_num(((28+156)*6),le_source.frame_data_len);
        le_source.buffer_index   = 0;
        le_source.buffer_index_max = N9Ble_calculate_avm_frame_num(((28+156)*6),le_source.frame_data_len);
        le_source.seq_num = 0;

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
        /* register ccni callback */
        extern void audio_transmitter_register_isr_handler(uint32_t index, f_audio_transmitter_ccni_callback_t callback);
        audio_transmitter_register_isr_handler(0, CB_N9_BLE_UL_UPDATE_TIMESTAMP);
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

        hal_nvic_restore_interrupt_mask(saved_mask);

        //DSP_MW_LOG_I("[le audio DSP] sink num: %d   frame_data_len:%d buffer_size:%d  sample_rate:%d bitrate:%d blocknum:%d", 6,
        //             le_source.num_of_sink, le_source.frame_data_len, le_source.buffer_size, le_source.sample_rate, le_source.bitrate, le_source.buffer_index_max);
    }
}

bool STREAM_BLE_UL_CHECK_MODE(void)
{
    return le_source.is_1k_tone_mode;
}

bool STREAM_BLE_UL_GET_PARAM(uint32_t *sample_rate, uint32_t *bitrate)
{
    if (le_source.is_source && le_source.is_playback_mode) {
        if (sample_rate) {
            *sample_rate = 1000 * le_source.sample_rate;
        }
        if (bitrate) {
            *bitrate = 1000 * le_source.bitrate;
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

bool STREAM_BLE_UL_CHECK_BUF(uint8_t index)
{
    if (le_source.is_source && le_source.is_playback_mode) {
        if (index < BLE_UL_MAX_NUM && ul_info[index]) {
            return TRUE;
        }
    }
    return FALSE;
}

uint32_t STREAM_BLE_UL_GET_BUF_INDEX(void)
{
    return le_source.buffer_index;
}

void STREAM_BLE_UL_WRITE_BUF(uint8_t index, uint8_t *buf, uint32_t len, uint32_t buffer_index)
{
    uint32_t buffer_offset;

    if (len != le_source.frame_data_len) {
        //DSP_MW_LOG_I("[le audio DSP] STREAM_BLE_UL_WRITE_BUF len abnormal: %d ", 1, len);
    }

    if (le_source.is_source && le_source.is_playback_mode && index < BLE_UL_MAX_NUM) {
        if (ul_info[index]) {

            buffer_offset = (ALIGN_4(le_source.frame_head_len + le_source.frame_data_len)) * buffer_index;
            LE_AUDIO_HEADER *buf_header = (LE_AUDIO_HEADER *)(le_source.buffer_start[index] + buffer_offset);
            uint8_t *avm_buffer = (uint8_t *)buf_header + le_source.frame_head_len;

            if ((uint32_t)avm_buffer & 0x80000000) {

                memcpy(avm_buffer, buf, len);
                buf_header->DataOffset              = le_source.frame_head_len;
                buf_header->Pdu_LEN_Without_MIC     = le_source.frame_data_len;

                // static uint32_t countttt = 0;
                // countttt++;

                // le_source.buffer_offset[index] += ALIGN_4(le_source.frame_head_len + le_source.frame_data_len);
                // if(countttt<50 && index==0)
                //     DSP_MW_LOG_I("[le audio DSP] 1.le_source.buffer_offset[index] : %d ", 1, le_source.buffer_offset[index] );
                // le_source.buffer_offset[index] %=  le_source.buffer_size;
                // if(countttt<50 && index==0)
                //     DSP_MW_LOG_I("[le audio DSP] 2.le_source.buffer_offset[index] : %d ", 1, le_source.buffer_offset[index] );
            } else {
                //DSP_MW_LOG_I("[le audio DSP] UL_GET_BUF AVM abnormal    avm_buffer:%X ", 1, avm_buffer);
            }

        } else {
            //DSP_MW_LOG_I("[le audio DSP] STREAM_BLE_UL_WRITE_BUF index invalid: %d", 1, index);
        }
    }
}


bool N9_BLE_SOURCE_ROUTINE(void)
{
    if (le_source.is_source) {
//
//        static uint8_t count = 0;
//        if(count<20)
//        {
//            count++;
//            if(ul1_info)
//                DSP_MW_LOG_I("ul1_info: start %X read %X write %X next %X sample %X length%X", 6,ul1_info->startaddr,ul1_info->ReadOffset,ul1_info->WriteOffset,ul1_info->sub_info.next,ul1_info->sample_rate,ul1_info->length);
//            else
//                DSP_MW_LOG_I("ul1_info empty",0);
//            if(dl_info)
//                DSP_MW_LOG_I("dl_info: start %X read %X write %X next %X sample %X length%X", 6,dl_info->startaddr,dl_info->ReadOffset,dl_info->WriteOffset,dl_info->sub_info.next,dl_info->sample_rate,dl_info->length);
//            else
//                DSP_MW_LOG_I("dl_info empty",0);
//        }
//
//        if(le_source.is_playback_mode && le_source.is_stream_data_valid)
//        {
//            SHARE_BUFFER_INFO avm_info;
//
//
//            memcpy((U32)&avm_info, (U32)ul1_info, 24);/* use first 24 bytes in info */
//
//            avm_info.startaddr = hal_memview_cm4_to_dsp0(avm_info.startaddr);
//            avm_info.length = BLE_AVM_FRAME_NUM*(BLE_AVM_FRAME_HEADER_SIZE+le_source.frame_length);
//
//            if(le_source.num_of_sink >= 1)
//            {
//                memcpy((U32)(avm_info.startaddr + avm_info.WriteOffset + BLE_AVM_FRAME_HEADER_SIZE), (U32)(le_source.stream_L_addr+le_source.stream_offset),le_source.frame_length);
//                avm_info.WriteOffset = (avm_info.WriteOffset+le_source.frame_length+BLE_AVM_FRAME_HEADER_SIZE)%avm_info.length;
//            }
//            if(le_source.num_of_sink >= 2)
//            {
//                memcpy((U32)(avm_info.startaddr + avm_info.WriteOffset + BLE_AVM_FRAME_HEADER_SIZE), (U32)(le_source.stream_R_addr+le_source.stream_offset),le_source.frame_length);
//                avm_info.WriteOffset = (avm_info.WriteOffset+le_source.frame_length+BLE_AVM_FRAME_HEADER_SIZE)%avm_info.length;
//            }
//
//            le_source.stream_offset = (le_source.frame_length+le_source.stream_offset)%le_source.stream_size;
//
//            if(le_source.stream_offset == 0)
//            {
//                printf("stream_offset == 0. Repeat again");
//            }
//
//            memcpy((U32)ul1_info, (U32)&avm_info, 24);/* use first 24 bytes in info */
//        }
//
        return TRUE;
    } else { // not source device
        return FALSE;
    }
}

#endif

