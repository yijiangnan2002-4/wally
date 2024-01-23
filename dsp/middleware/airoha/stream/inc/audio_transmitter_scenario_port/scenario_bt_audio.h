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

#ifndef _SCENARIO_BT_AUDIO_H_
#define _SCENARIO_BT_AUDIO_H_

#if defined(AIR_BT_AUDIO_DONGLE_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "syslog.h"
#include "scenario_audio_common.h"
#include "stream_audio_transmitter.h"
#include "sw_mixer_interface.h"
#include "hal_audio_message_struct_common.h"
#include "common.h"
#include "hal_resource_assignment.h"
#include "clk_skew_sw.h"
#include "sw_gain_interface.h"
#include "sw_buffer_interface.h"
#include "sw_mixer_interface.h"
#include "sw_src_interface.h"
#include "src_fixed_ratio_interface.h"
#include "sw_src_interface.h"
#include "sink_inter.h"
#include "source_inter.h"
#include "stream_n9sco.h"
#ifdef __cplusplus
extern "C"
{
#endif
/* Public define -------------------------------------------------------------*/
#define BT_DONGLE_USE_MSGID_SEND_LOG
#ifdef BT_DONGLE_USE_MSGID_SEND_LOG
#define BT_DONGLE_LOG_E(_message, arg_cnt, ...)  LOG_MSGID_E(bt_dongle_log,_message, arg_cnt, ##__VA_ARGS__)
#define BT_DONGLE_LOG_W(_message, arg_cnt, ...)  LOG_MSGID_W(bt_dongle_log,_message, arg_cnt, ##__VA_ARGS__)
#define BT_DONGLE_LOG_I(_message, arg_cnt, ...)  LOG_MSGID_I(bt_dongle_log,_message, arg_cnt, ##__VA_ARGS__)
#define BT_DONGLE_LOG_D(_message, arg_cnt, ...)  LOG_MSGID_D(bt_dongle_log,_message, arg_cnt, ##__VA_ARGS__)
#else
#define BT_DONGLE_LOG_E(_message, arg_cnt, ...)  LOG_E(bt_dongle_log,_message, ##__VA_ARGS__)
#define BT_DONGLE_LOG_W(_message, arg_cnt, ...)  LOG_W(bt_dongle_log,_message, ##__VA_ARGS__)
#define BT_DONGLE_LOG_I(_message, arg_cnt, ...)  LOG_I(bt_dongle_log,_message, ##__VA_ARGS__)
#define BT_DONGLE_LOG_D(_message, arg_cnt, ...)  LOG_D(bt_dongle_log,_message, ##__VA_ARGS__)
#endif

/* Public typedef ------------------------------------------------------------*/
typedef struct  {
    U16 payload_offset;       /* offset of payload */
    U16 _reserved_word_02h;   /* not used */
    U32 time_stamp;           /* frames * (framenumber - 1) */
    U16 airlog_event_count;   /* event count seem on airlog, for debug propose */ /* not used */
    U8 sample_sequence;       /* Sample sequence of this SDU interval Ex:0,1,2... */ /* not used */
    U8 _reserved_byte_0bh;    /* not used */
    U8 pdu_hdr_lo;            /* not used */
    U8 valid_packet;          /* valid packet: 0x01, invalid packet 0x00 */ /* not used */
    U8 pdu_len;               /* not used */
    U8 _reserved_byte_0fh;    /* not used */
    U16 data_len;             /* payload size */
    U16 _reserved_word_12h;   /* not used */
    U32 _sync_frame_0;        /* 0xABCD */
    U32 _sync_frame_1;        /* 0x5A5A */
} BT_AUDIO_HEADER;

typedef enum {
    BT_AUDIO_TYPE_NONE = 0,
    BT_AUDIO_TYPE_HFP  = 1,
    BT_AUDIO_TYPE_A2DP = 2
} bt_audio_type;

typedef struct bt_audio_dongle_handle_t {
    void *owner;
    //int16_t total_number;
    //int16_t index;
    struct bt_audio_dongle_handle_t *next_handle;
    SOURCE source;
    SINK   sink;
    ALIGN(1) bt_audio_type                      link_type;
    ALIGN(1) audio_dongle_stream_status_t       stream_status;
    ALIGN(1) audio_dongle_data_status_t         data_status;
    ALIGN(1) audio_dongle_first_packet_status_t first_packet_status;
    bt_audio_dongle_config_t stream_info;
    uint32_t drop_frames;
    uint32_t ccni_in_bt_count;
    uint32_t ccni_in_gpt_count;
    uint32_t data_in_gpt_count;
    uint32_t data_out_gpt_count;
    uint32_t data_out_bt_count;
    uint16_t fetch_count;     // how many bt out frame size need to encode in this irq period (DL), 0x5A5A is a symbol to indicate ccni request
    uint16_t ccni_blk_index;
    uint32_t process_frames;  // how many usb in frame size need to process this time (DL)
    sw_gain_port_t *gain_port;
    uint32_t buffer_default_output_size;
    uint32_t buffer0_output_size;
    sw_buffer_port_t *buffer_port_0;
    sw_clk_skew_port_t *clk_skew_port;
    //ull_audio_v2_dongle_compensatory_method_t compen_method;
    int16_t compen_samples;
    int16_t clk_skew_count;
    int32_t clk_skew_watermark_samples;
    int32_t clk_skew_compensation_mode;
    src_fixed_ratio_port_t *src0_port; // for 96k -> 48k
    src_fixed_ratio_port_t *src1_port; // for 96k -> 48k
    void     *src_port;
    uint16_t usb_frame_size;
    uint16_t src_in_frame_size;
    uint16_t src_in_frame_samples;
    uint16_t src_out_frame_size;
    uint16_t src_out_frame_samples;
    audio_dongle_mixer_status_t mixer_status;
    sw_mixer_member_t *mixer_member;
    uint16_t latency_size;  // source buffer current size
    uint16_t pre_ro[BT_AUDIO_DATA_CHANNEL_NUMBER];        // for dump bt read data
    uint16_t cur_ro[BT_AUDIO_DATA_CHANNEL_NUMBER];        // for dump bt read data
    uint32_t process_sample_rate_max;
    int32_t gain_compensation;
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
    audio_dongle_afe_in_info_t afe_in;
#endif
} bt_audio_dongle_handle_t;

typedef struct {
    uint32_t pattern_frame_size;         // frame size of data
    uint16_t rx_ul_fwd_buf_index_pre;    // fwd hw buffer wo index
    uint16_t tx_dl_fwd_buf_index_pre;    // fwd hw buffer ro index
    uint32_t pkt_num_per_frame;          // no used
    uint32_t rx_ul_fwd_irq_gpt_cnt;      // rx forwarder irq gpt count
    uint32_t tx_dl_fwd_irq_gpt_cnt;      // tx forwarder irq gpt count
    uint64_t rx_ul_fwd_irq_process_cnt;  // rx forwarder irq process count
    uint64_t tx_dl_fwd_irq_process_cnt;  // tx forwarder irq process count
    uint32_t rx_ul_fwd_frame_id;
    sco_pkt_type pkt_type;
    bool     is_ul_stream_ready;
    bool     is_dl_stream_ready;
    avm_share_buf_info_t *rx_ul_info;
    avm_share_buf_info_t *tx_dl_info;
    // debug pkt lost for rx fwd
    uint16_t forwarder_pkt_num;
    uint16_t lost_pkt_num;
} sco_fwd_info;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/****************************************************************************************************************************************************/
/*                                                       BT Source Dongle DL(USB IN)                                                                */
/****************************************************************************************************************************************************/
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
bool     bt_audio_dongle_dl_source_get_avail_size(SOURCE source, uint32_t *avail_size);
uint32_t bt_audio_dongle_dl_source_get_new_read_offset(SOURCE source, uint32_t amount);
uint32_t bt_audio_dongle_dl_source_copy_payload(SOURCE source, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length);
void     bt_audio_dongle_dl_source_drop_postprocess(SOURCE source, uint32_t amount);
#endif

bt_audio_dongle_handle_t *bt_audio_dongle_query_handle_by_scenario_type(audio_scenario_type_t type, audio_dongle_stream_type_t stream_type);

void bt_audio_dongle_dl_init(SOURCE source, SINK sink, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param);
void bt_audio_dongle_dl_deinit(SOURCE source, SINK sink);
void bt_audio_dongle_dl_start(SOURCE source, SINK sink);
void bt_audio_dongle_dl_stop(SOURCE source, SINK sink);
bool bt_audio_dongle_dl_config(SOURCE source, stream_config_type type, U32 value);
bool bt_audio_dongle_dl_source_close(SOURCE source);
bool     bt_audio_dongle_dl_sink_get_avail_size(SINK sink, uint32_t *avail_size);
uint32_t bt_audio_dongle_dl_sink_copy_payload(SINK sink, uint8_t *src_buf, uint32_t length);
bool     bt_audio_dongle_dl_sink_query_notification(SINK sink, bool *notification_flag);
bool     bt_audio_dongle_dl_sink_get_new_write_offset(SINK sink, U32 amount, uint32_t *new_write_offset);
bool     bt_audio_dongle_dl_sink_send_data_ready_notification(SINK sink);
void     bt_audio_dongle_dl_sink_flush_postprocess(SINK sink, uint32_t amount);
bool bt_audio_dongle_dl_sink_close(SINK sink);

uint32_t bt_audio_dongle_dl_get_stream_in_max_size_each_channel(SOURCE source, SINK sink);
uint32_t bt_audio_dongle_dl_get_stream_in_channel_number(SOURCE source, SINK sink);
stream_samplerate_t bt_audio_dongle_dl_get_stream_in_sampling_rate_each_channel(SOURCE source, SINK sink);
stream_samplerate_t bt_audio_dongle_dl_get_stream_out_sampling_rate_each_channel(SOURCE source, SINK sink);
uint32_t bt_audio_dongle_dl_get_stream_out_max_size_each_channel(SOURCE source, SINK sink);
uint32_t bt_audio_dongle_dl_get_stream_out_channel_number(SOURCE source, SINK sink);

/****************************************************************************************************************************************************/
/*                                                       BT Source Dongle UL(USB OUT)                                                               */
/****************************************************************************************************************************************************/
void bt_audio_dongle_ul_init(SOURCE source, SINK sink, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param);
void bt_audio_dongle_ul_deinit(SOURCE source, SINK sink);
void bt_audio_dongle_ul_start(SOURCE source, SINK sink);
void bt_audio_dongle_ul_stop(SOURCE source, SINK sink);
bool bt_audio_dongle_ul_config(SOURCE source, stream_config_type type, U32 value);
bool bt_audio_dongle_ul_source_close(SOURCE source);

// ul config parameter of stream
uint32_t bt_audio_dongle_ul_get_stream_in_max_size_each_channel(SOURCE source, SINK sink);
uint32_t bt_audio_dongle_ul_get_stream_in_channel_number(SOURCE source, SINK sink);
stream_samplerate_t bt_audio_dongle_ul_get_stream_in_sampling_rate_each_channel(SOURCE source, SINK sink);
uint32_t bt_audio_dongle_ul_get_stream_out_max_size_each_channel(SOURCE source, SINK sink);
uint32_t bt_audio_dongle_ul_get_stream_out_channel_number(SOURCE source, SINK sink);
stream_samplerate_t bt_audio_dongle_ul_get_stream_out_sampling_rate_each_channel(SOURCE source, SINK sink);

// ul source
bool bt_audio_dongle_ul_source_get_avail_size(SOURCE source, uint32_t *avail_size);
uint32_t bt_audio_dongle_ul_source_copy_payload(SOURCE source, uint8_t *dst_buf, uint32_t length);
bool bt_audio_dongle_ul_source_get_new_read_offset(SOURCE source, U32 amount, U32 *new_read_offset);
void bt_audio_dongle_ul_source_drop_postprocess(SOURCE source, uint32_t amount);

// ul sink
bool bt_audio_dongle_ul_sink_get_avail_size(SINK sink, uint32_t *avail_size);
uint32_t bt_audio_dongle_ul_sink_copy_payload(SINK sink, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length);
void bt_audio_dongle_ul_sink_query_write_offset(SINK sink, uint32_t *write_offset);
bool bt_audio_dongle_ul_sink_close(SINK sink);

#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
extern void bt_audio_dongle_silence_detection_init(audio_scenario_type_t scenario);
extern void bt_audio_dongle_silence_detection_deinit(audio_scenario_type_t scenario);
extern void bt_audio_dongle_silence_detection_enable(audio_scenario_type_t scenario);
extern void bt_audio_dongle_silence_detection_disable(audio_scenario_type_t scenario);
extern void bt_audio_dongle_silence_detection_process(audio_scenario_type_t scenario);
#endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */
void bt_audio_dongle_resolution_config(DSP_STREAMING_PARA_PTR stream);
#ifdef __cplusplus
}
#endif
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */
#endif /* _SCENARIO_BT_AUDIO_H_ */
