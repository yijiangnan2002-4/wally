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

#ifndef _SCENARIO_DONGLE_COMMON_H_
#define _SCENARIO_DONGLE_COMMON_H_

/* Includes ------------------------------------------------------------------*/
#include "hal_audio.h"
#include "sink_inter.h"
#include "common.h"

#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_GAMING_MODE_DONGLE_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_BLE_HEADSET_ENABLE)

#ifdef __cplusplus
extern "C"
{
#endif
/* Private define ------------------------------------------------------------*/
#define EIGHT_BYTE_ALIGNED(size)   (((size) + 7) & ~0x7UL)
#define FOUR_BYTE_ALIGNED(size)    (((size) + 3) & ~0x3UL)

#define AUDIO_DONGLE_SAMPLE_RATE_8K                                 (8000)  // 8K

#define DONGLE_COMMON_USE_MSGID_SEND_LOG
#ifdef DONGLE_COMMON_USE_MSGID_SEND_LOG
#define COMMON_DONGLE_LOG_E(_message, arg_cnt, ...)  LOG_MSGID_E(common_dongle_log,_message, arg_cnt, ##__VA_ARGS__)
#define COMMON_DONGLE_LOG_W(_message, arg_cnt, ...)  LOG_MSGID_W(common_dongle_log,_message, arg_cnt, ##__VA_ARGS__)
#define COMMON_DONGLE_LOG_I(_message, arg_cnt, ...)  LOG_MSGID_I(common_dongle_log,_message, arg_cnt, ##__VA_ARGS__)
#define COMMON_DONGLE_LOG_D(_message, arg_cnt, ...)  LOG_MSGID_D(common_dongle_log,_message, arg_cnt, ##__VA_ARGS__)
#else
#define COMMON_DONGLE_LOG_E(_message, arg_cnt, ...)  LOG_E(common_dongle_log,_message, ##__VA_ARGS__)
#define COMMON_DONGLE_LOG_W(_message, arg_cnt, ...)  LOG_W(common_dongle_log,_message, ##__VA_ARGS__)
#define COMMON_DONGLE_LOG_I(_message, arg_cnt, ...)  LOG_I(common_dongle_log,_message, ##__VA_ARGS__)
#define COMMON_DONGLE_LOG_D(_message, arg_cnt, ...)  LOG_D(common_dongle_log,_message, ##__VA_ARGS__)
#endif
/* Private typedef -----------------------------------------------------------*/
typedef enum {
    AUDIO_DONGLE_TYPE_ULL_V1 = 0,
    AUDIO_DONGLE_TYPE_ULL_V2,
    AUDIO_DONGLE_TYPE_BLE,
    AUDIO_DONGLE_TYPE_BT,
    AUDIO_DONGLE_TYPE_MAX
} audio_dongle_type_t;

typedef enum {
    AUDIO_DONGLE_STREAM_TYPE_DL = 0,
    AUDIO_DONGLE_STREAM_TYPE_UL,
} audio_dongle_stream_type_t;

typedef enum {
    AUDIO_DONGLE_STREAM_DEINIT = 0,
    AUDIO_DONGLE_STREAM_INIT,
    AUDIO_DONGLE_STREAM_START,
    AUDIO_DONGLE_STREAM_RUNNING,
    AUDIO_DONGLE_STREAM_STOP
} audio_dongle_stream_status_t;

typedef enum {
    AUDIO_DONGLE_DL_DATA_EMPTY = 0,
    AUDIO_DONGLE_DL_DATA_IN_STREAM,
    AUDIO_DONGLE_DL_DATA_IN_MIXER,
    AUDIO_DONGLE_UL_DATA_EMPTY,
    AUDIO_DONGLE_UL_DATA_NORMAL,
    AUDIO_DONGLE_UL_DATA_PLC,
    AUDIO_DONGLE_UL_DATA_BYPASS_DECODER,
    AUDIO_DONGLE_UL_DATA_SINK_HALF_PERIOD,
} audio_dongle_data_status_t;

typedef enum {
    AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY = 0,
    AUDIO_DONGLE_DL_FIRST_PACKET_READY,
    AUDIO_DONGLE_DL_FIRST_PACKET_TIMEOUT,
    AUDIO_DONGLE_UL_FIRST_PACKET_NOT_READY,
    AUDIO_DONGLE_UL_FIRST_PACKET_READY,
    AUDIO_DONGLE_UL_FIRST_PACKET_TIMEOUT,
    AUDIO_DONGLE_UL_FIRST_PACKET_PLAYED,
} audio_dongle_first_packet_status_t;

typedef enum {
    AUDIO_DONGLE_DEC_FRAME_STATUS_NORMAL = 0,
    AUDIO_DONGLE_DEC_FRAME_STATUS_PLC = 1,
    AUDIO_DONGLE_DEC_FRAME_STATUS_BYPASS_DECODER = 2
} audio_dongle_ul_dec_frame_status_t;

typedef enum {
    AUDIO_DONGLE_MIXER_UNMIX = 0,
    AUDIO_DONGLE_MIXER_MIX,
} audio_dongle_mixer_status_t;

typedef struct {
    uint32_t afe_vul_cur_addr;
    uint32_t afe_vul_base_addr;
    uint32_t afe_vul_cur;
    uint32_t afe_vul_base;
    uint32_t pre_write_offset;
    uint32_t cur_write_offset;
    uint32_t pre_read_offset;
    uint32_t cur_read_offset;
    int32_t  bt_afe_deviation;
    uint32_t afe_buffer_latency_size;
    bool     i2s_slv_flag;
} audio_dongle_afe_in_info_t;

typedef struct {
    uint32_t process_max_size; // the max process size of input 1ch
    uint32_t in_rate;
    uint32_t out_rate;
    uint32_t period_ms;
    uint32_t in_ch_num;
    hal_audio_format_t pcm_format;
    audio_dsp_codec_type_t codec_type;
    SOURCE source;
    uint32_t process_sample_rate_max;
    void *src0_port;
    void *src1_port;
    void *src2_port;
} audio_dl_unified_fs_convertor_param_t;

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
    uint32_t iso_anchor_clock;
    uint16_t iso_anchor_phase;
    uint32_t ul_avm_info_addr;
    uint32_t dl_avm_info_addr;
} audio_dongle_init_le_play_info_t;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE) || defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE || defined AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
void audio_dongle_dl_afe_start(void *dongle_handle, audio_dongle_type_t dongle_type);
void audio_dongle_afe_in_ccni_handler(void *dongle_handle, audio_dongle_type_t dongle_type);
#endif

#if defined(AIR_BT_AUDIO_DONGLE_USB_ENABLE)
int32_t audio_dongle_dl_usb_clock_skew_check(void *dongle_handle, uint32_t input_samples, uint32_t *buffer0_output_size, audio_dongle_type_t dongle_type);
void audio_dongle_dl_usb_data_copy(
    void *dongle_handle,
    SINK sink,
    uint8_t *src_buf,
    uint32_t samples,
    uint32_t samples_offset,
    audio_dongle_type_t dongle_type);
int32_t audio_dongle_ul_usb_clock_skew_check(void *dongle_handle, audio_dongle_type_t dongle_type);
#endif

/* Misc function for common flow */
uint8_t audio_dongle_get_usb_format_bytes(hal_audio_format_t pcm_format);
uint8_t audio_dongle_get_format_bytes(hal_audio_format_t pcm_format);
void audio_dongle_sink_get_share_buffer_avail_size(SINK sink, uint32_t *avail_size);
uint32_t audio_dongle_get_n9_share_buffer_data_size(n9_dsp_share_info_t *share_info);
bool audio_dl_unified_fs_convertor_init(audio_dl_unified_fs_convertor_param_t *param);
bool audio_dl_unified_fs_convertor_deinit(audio_dl_unified_fs_convertor_param_t *param);
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
uint32_t audio_dongle_get_n9_share_buffer_data_size_without_header(n9_dsp_share_info_t *share_info);
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
void audio_dongle_init_le_play_info(hal_ccni_message_t msg, hal_ccni_message_t *ack);
#endif

#endif /* dongle type:ble/v1.0/v2.0/bt */

void ShareBufferCopy_I_24bit_to_D_32bit_1ch(uint8_t* src_buf, uint32_t* dest_buf, uint32_t samples);
void ShareBufferCopy_I_16bit_to_D_16bit_2ch(uint32_t *src_buf, uint16_t *dest_buf1, uint16_t *dest_buf2, uint32_t samples);
void ShareBufferCopy_I_16bit_to_D_32bit_2ch(uint32_t* src_buf, uint32_t* dest_buf1, uint32_t* dest_buf2, uint32_t samples);
void ShareBufferCopy_I_24bit_to_D_16bit_2ch(uint8_t* src_buf, uint16_t* dest_buf1, uint16_t* dest_buf2, uint32_t samples);
void ShareBufferCopy_I_24bit_to_D_32bit_2ch(uint8_t* src_buf, uint32_t* dest_buf1, uint32_t* dest_buf2, uint32_t samples);
void ShareBufferCopy_I_16bit_to_D_32bit_8ch(uint32_t* src_buf, uint32_t* dest_buf1, uint32_t* dest_buf2, uint32_t* dest_buf3, uint32_t* dest_buf4, uint32_t* dest_buf5, uint32_t* dest_buf6, uint32_t* dest_buf7, uint32_t* dest_buf8, uint32_t samples);
void ShareBufferCopy_I_16bit_to_D_16bit_8ch(uint32_t* src_buf, uint16_t* dest_buf1, uint16_t* dest_buf2, uint16_t* dest_buf3, uint16_t* dest_buf4, uint16_t* dest_buf5, uint16_t* dest_buf6, uint16_t* dest_buf7, uint16_t* dest_buf8, uint32_t samples);
void ShareBufferCopy_I_24bit_to_D_32bit_8ch(uint32_t* src_buf, uint32_t* dest_buf1, uint32_t* dest_buf2, uint32_t* dest_buf3, uint32_t* dest_buf4, uint32_t* dest_buf5, uint32_t* dest_buf6, uint32_t* dest_buf7, uint32_t* dest_buf8, uint32_t samples);
void ShareBufferCopy_I_24bit_to_D_16bit_8ch(uint32_t* src_buf, uint16_t* dest_buf1, uint16_t* dest_buf2, uint16_t* dest_buf3, uint16_t* dest_buf4, uint16_t* dest_buf5, uint16_t* dest_buf6, uint16_t* dest_buf7, uint16_t* dest_buf8, uint32_t samples);

void ShareBufferCopy_D_32bit_to_D_24bit_1ch(uint32_t* src_buf, uint8_t* dest_buf1, uint32_t samples);
void ShareBufferCopy_D_32bit_to_I_24bit_2ch(uint32_t* src_buf1, uint32_t* src_buf2, uint8_t* dest_buf1, uint32_t samples);
void ShareBufferCopy_D_16bit_to_D_16bit_1ch(uint16_t* src_buf, uint16_t* dest_buf1, uint32_t samples);
void ShareBufferCopy_D_16bit_to_I_16bit_2ch(uint16_t* src_buf1, uint16_t* src_buf2, uint16_t* dest_buf1, uint32_t samples);
void ShareBufferCopy_D_32bit_to_D_16bit_1ch(uint32_t* src_buf, uint16_t* dest_buf1, uint32_t samples);
void ShareBufferCopy_D_32bit_to_I_16bit_2ch(uint32_t* src_buf1, uint32_t* src_buf2, uint16_t* dest_buf1, uint32_t samples);
void ShareBufferCopy_D_16bit_to_I_24bit_2ch(uint16_t* src_buf1, uint16_t* src_buf2, uint8_t* dest_buf1, uint32_t samples);
void ShareBufferCopy_D_16bit_to_D_24bit_1ch(uint16_t* src_buf1, uint8_t* dest_buf1, uint32_t samples);
void ShareBufferCopy_D_16bit_to_D_32bit_1ch(uint16_t* src_buf, uint32_t* dest_buf, uint32_t samples);
void ShareBufferCopy_D_16bit_to_I_24bit_1ch(uint16_t* src_buf1, uint8_t* dest_buf1, uint32_t samples);

#ifdef __cplusplus
}
#endif
#endif /* _SCENARIO_DONGLE_COMMON_H_ */