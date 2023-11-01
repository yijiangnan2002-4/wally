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

#ifndef _SCENARIO_WIRELESS_MIC_RX_H_
#define _SCENARIO_WIRELESS_MIC_RX_H_

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "config.h"
#include "types.h"
#include "source_inter.h"
#include "sink_inter.h"
#include "source.h"
#include "sink.h"
#include "transform_inter.h"
#include "stream_audio_hardware.h"
#include "stream_audio_setting.h"
#include "stream_audio_driver.h"
#include "dsp_audio_msg_define.h"
#include "dsp_audio_msg.h"
#include "hal_hw_semaphore.h"
#include "hal_resource_assignment.h"
#include "clk_skew_sw.h"
#include "sw_gain_interface.h"
#include "sw_buffer_interface.h"
#include "sw_mixer_interface.h"
#include "sw_src_interface.h"
#include "src_fixed_ratio_interface.h"
#include "common.h"
#include "compander_interface_sw.h"

/* Public define -------------------------------------------------------------*/
#define WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER             4
#define WIRELESS_MIC_RX_ULD_DATA_CHANNEL_NUMBER         2
#define WIRELESS_MIC_RX_LC3PLUS_RESERVED_PROCESS_TIME   2000 /* 2ms for 208MHz; 3ms for 104MHz */
#define WIRELESS_MIC_RX_ULD_RESERVED_PROCESS_TIME       700 /* 0.5ms + 0.2ms(PLC) */

/* Public typedef ------------------------------------------------------------*/
typedef enum {
    WIRELESS_MIC_RX_STREAM_DEINIT = 0,
    WIRELESS_MIC_RX_STREAM_INIT,
    WIRELESS_MIC_RX_STREAM_START,
    WIRELESS_MIC_RX_STREAM_RUNNING,
} wireless_mic_rx_stream_status_t;

typedef enum {
    WIRELESS_MIC_RX_UL_DATA_EMPTY,
    WIRELESS_MIC_RX_UL_DATA_NORMAL,
    WIRELESS_MIC_RX_UL_DATA_PLC,
    WIRELESS_MIC_RX_UL_DATA_BYPASS_DECODER,
    WIRELESS_MIC_RX_UL_DATA_IN_MIXER,
    WIRELESS_MIC_RX_UL_DATA_BYPASS_DECODER_BUT_UPDATE_FETCH,
} wireless_mic_rx_data_status_t;

typedef enum {
    WIRELESS_MIC_RX_UL_FIRST_PACKET_NOT_READY = 0,
    WIRELESS_MIC_RX_UL_FIRST_PACKET_READY,
    WIRELESS_MIC_RX_UL_FIRST_PACKET_TIMEOUT,
    WIRELESS_MIC_RX_UL_FIRST_PACKET_PLAYED,
} wireless_mic_rx_first_packet_status_t;

typedef enum {
    WIRELESS_MIC_RX_MIXER_UNMIX = 0,
    WIRELESS_MIC_RX_MIXER_MIX,
} wireless_mic_rx_mixer_status_t;

typedef enum {
    WIRELESS_MIC_RX_COMPENSATORY_METHOD_DISABLE = 0,
    WIRELESS_MIC_RX_COMPENSATORY_METHOD_SW_CLK_SKEW,
} wireless_mic_rx_compensatory_method_t;

typedef struct  {
    uint8_t  seq_num;
    uint8_t  user_count;
    uint16_t blk_index;
    uint16_t blk_index_previous;
    uint32_t crc32_init;
    wireless_mic_rx_bt_link_info_t bt_link_info;
} wireless_mic_rx_bt_info_t;

typedef struct {
    audio_dsp_codec_type_t codec_type;
    uint32_t channel_num;
    uint32_t channel_mode;
    uint32_t sample_rate;
    hal_audio_format_t sample_format;
    uint32_t frame_size;
    uint32_t frame_samples;
    uint32_t frame_interval;
    uint32_t bit_rate;
    uint32_t bt_retry_window;
    uint32_t bt_retry_window_phase;
    uint32_t bt_interval;
    uint32_t bt_interval_phase;
    uint32_t bt_channel_anchor_diff;
    uint32_t bt_channel_anchor_phase_diff;
    uint16_t play_en_delay;
    int16_t  play_en_first_packet_safe_delay;
    uint32_t play_en_bt_clk;
    uint32_t play_en_bt_clk_phase;
    uint32_t first_packet_bt_clk;
    uint32_t first_packet_bt_clk_phase;
    uint32_t first_anchor_bt_clk;
    uint32_t first_anchor_bt_clk_phase;
    uint32_t fetch_anchor_previous;
    uint32_t fetch_anchor_phase_previous;
    uint32_t fetch_anchor;
    uint32_t fetch_anchor_phase;
    uint32_t channel_data_status;
    wireless_mic_rx_bt_info_t *bt_info[WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER];
} wireless_mic_rx_bt_in_info_t;

typedef union {
    uint32_t reserved;
    wireless_mic_rx_bt_in_info_t bt_in;
} wireless_mic_rx_source_info_t;

typedef struct {
    uint32_t channel_num;
    uint32_t sample_rate;
    hal_audio_format_t sample_format;
    uint32_t frame_size;
    uint32_t frame_samples;
    uint32_t frame_interval;
    uint16_t frame_max_num;
    uint16_t mcu_frame_count;
    uint16_t mcu_frame_count_latch;
    uint16_t seq_num;
    uint32_t re_trigger_ongoing;
} wireless_mic_rx_usb_out_info_t;

typedef struct {
    uint32_t channel_num;
    uint32_t sample_rate;
    hal_audio_format_t sample_format;
    uint32_t frame_size;
    uint32_t frame_samples;
    uint32_t frame_interval;
    uint32_t frame_max_num;
    uint32_t afe_cur_addr;
    uint32_t afe_base_addr;
    uint32_t afe_cur;
    uint32_t afe_base;
    uint32_t pre_write_offset;
    uint32_t cur_write_offset;
    uint32_t pre_read_offset;
    uint32_t cur_read_offset;
} wireless_mic_rx_line_out_info_t;

typedef struct {
    uint32_t channel_num;
    uint32_t sample_rate;
    hal_audio_format_t sample_format;
    uint32_t frame_size;
    uint32_t frame_samples;
    uint32_t frame_interval;
    uint32_t frame_max_num;
    uint32_t afe_cur_addr;
    uint32_t afe_base_addr;
    uint32_t afe_cur;
    uint32_t afe_base;
    uint32_t afe_hwsrc_base_addr;
    uint32_t afe_hwsrc_len_addr;
    uint32_t afe_hwsrc_rptr_addr;
    uint32_t afe_hwsrc_wptr_addr;
    uint32_t afe_hwsrc_base;
    uint32_t afe_hwsrc_rptr;
    uint32_t afe_hwsrc_wptr;
    uint32_t pre_write_offset;
    uint32_t cur_write_offset;
    uint32_t pre_read_offset;
    uint32_t cur_read_offset;
} wireless_mic_rx_i2s_slv_out_info_t;

typedef union {
    uint32_t reserved;
    wireless_mic_rx_usb_out_info_t usb_out;
    wireless_mic_rx_line_out_info_t line_out;
    wireless_mic_rx_i2s_slv_out_info_t i2s_slv_out;
} wireless_mic_rx_sink_info_t;

typedef struct wireless_mic_rx_ul_handle_t wireless_mic_rx_ul_handle_t;
struct wireless_mic_rx_ul_handle_t {
    void *owner;
    int16_t total_number;
    int16_t index;
    wireless_mic_rx_ul_handle_t *next_ul_handle;
    audio_transmitter_scenario_sub_id_wirelessmicrx_t sub_id;
    SOURCE source;
    SINK   sink;
    wireless_mic_rx_stream_status_t stream_status;
    wireless_mic_rx_data_status_t data_status;
    wireless_mic_rx_first_packet_status_t first_packet_status;
    wireless_mic_rx_source_info_t source_info;
    wireless_mic_rx_sink_info_t sink_info;
    uint32_t ccni_in_bt_count;
    uint32_t ccni_in_gpt_count;
    uint32_t data_in_gpt_count;
    uint32_t data_out_gpt_count;
    uint32_t data_out_bt_count;
    uint32_t stream_process_count;
    uint16_t fetch_count;
    uint16_t process_frames;
    uint32_t drop_frames;
    sw_mixer_member_t *mixer_member;
    sw_clk_skew_port_t *clk_skew_port;
    int16_t compen_samples;
    int16_t clk_skew_count;
    int32_t clk_skew_watermark_samples;
    int32_t clk_skew_compensation_mode;
    sw_buffer_port_t *buffer_port;
    uint32_t buffer_output_size;
    uint32_t buffer_default_output_size;
    sw_gain_port_t *gain_port;
    sw_compander_port_t *drc_port;
    src_fixed_ratio_port_t *src_port;
    uint16_t src_in_frame_size;
    uint16_t src_in_frame_samples;
    uint16_t src_out_frame_size;
    uint16_t src_out_frame_samples;
    uint32_t src_reserved_size;
    bool src_lock_write_flag;
    uint32_t is_play_en_trigger;
};

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/******************************************************************************/
/*                  wireless mic rx common Public Functions                   */
/******************************************************************************/
extern bool wireless_mic_rx_ul_fetch_time_is_arrived(wireless_mic_rx_ul_handle_t *rx_handle, uint32_t bt_clk, uint16_t bt_phase);
extern void wireless_mic_rx_init_play_info(hal_ccni_message_t msg, hal_ccni_message_t *ack);
extern void wireless_mic_rx_playen_disable(SINK sink);
/******************************************************************************/
/*                  wireless mic rx UL path Public Functions                  */
/******************************************************************************/
extern uint32_t wireless_mic_rx_ul_get_stream_in_max_size_each_channel(SOURCE source, SINK sink);
extern uint32_t wireless_mic_rx_ul_get_stream_in_channel_number(SOURCE source, SINK sink);
extern stream_samplerate_t wireless_mic_rx_ul_get_stream_in_sampling_rate_each_channel(SOURCE source, SINK sink);
extern uint32_t wireless_mic_rx_ul_get_stream_out_max_size_each_channel(SOURCE source, SINK sink);
extern uint32_t wireless_mic_rx_ul_get_stream_out_channel_number(SOURCE source, SINK sink);
extern stream_samplerate_t wireless_mic_rx_ul_get_stream_out_sampling_rate_each_channel(SOURCE source, SINK sink);
extern void wireless_mic_rx_ul_init(SOURCE source, SINK sink, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param);
extern void wireless_mic_rx_ul_deinit(SOURCE source, SINK sink);
extern void wireless_mic_rx_ul_start(SOURCE source, SINK sink);
extern void wireless_mic_rx_ul_stop(SOURCE source, SINK sink);
extern bool wireless_mic_rx_ul_config(SOURCE source, stream_config_type type, U32 value);
extern void wireless_mic_rx_ul_source_init(SOURCE source, bt_common_open_param_p open_param, BT_COMMON_PARAMETER *source_param);
extern bool wireless_mic_rx_ul_source_get_avail_size(SOURCE source, uint32_t *avail_size);
extern uint32_t wireless_mic_rx_ul_source_copy_payload(SOURCE source, uint8_t *dst_buf, uint32_t length);
extern bool wireless_mic_rx_ul_source_get_new_read_offset(SOURCE source, U32 amount, U32 *new_read_offset);
extern void wireless_mic_rx_ul_source_drop_postprocess(SOURCE source, uint32_t amount);
extern bool wireless_mic_rx_ul_source_close(SOURCE source);
extern bool wireless_mic_rx_ul_sink_get_avail_size(SINK sink, uint32_t *avail_size);
extern uint32_t wireless_mic_rx_ul_sink_copy_payload(SINK sink, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length);
extern void wireless_mic_rx_ul_sink_query_write_offset(SINK sink, uint32_t *write_offset);
extern bool wireless_mic_rx_ul_sink_close(SINK sink);

#endif /* AIR_WIRELESS_MIC_RX_ENABLE */

#endif /* _SCENARIO_WIRELESS_MIC_RX_H_ */
