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

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "scenario_ull_audio_v2.h"
#include "types.h"
#include "source_inter.h"
#include "sink_inter.h"
#include "dsp_buffer.h"
#include "dsp_memory.h"
#include "dsp_callback.h"
#include "dsp_temp.h"
#include "dsp_dump.h"
#include "dsp_scenario.h"
#include "dsp_audio_msg_define.h"
#include "dsp_audio_process.h"
#include "stream_audio_transmitter.h"
#include "stream_bt_common.h"
#include "bt_types.h"
#include "hal_gpt.h"
#include "hal_sleep_manager.h"
#include "memory_attribute.h"
#include "clk_skew_sw.h"
#include "sw_gain_interface.h"
#include "sw_buffer_interface.h"
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
#include "lc3plus_enc_interface.h"
#include "lc3plus_dec_interface.h"
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
#include "hal_audio_driver.h"
#include "hal_audio_register.h"
#include "dsp_share_memory.h"
#if defined(AIR_CELT_DEC_V2_ENABLE)
#include "celt_dec_interface_v2.h"
#endif /* AIR_CELT_DEC_V2_ENABLE */
#if defined(AIR_CELT_ENC_V2_ENABLE)
#include "celt_enc_interface_v2.h"
#endif /* AIR_CELT_ENC_V2_ENABLE */
#include "bt_interface.h"
#ifdef AIR_SILENCE_DETECTION_ENABLE
#include "volume_estimator_interface.h"
#include "dtm.h"
#include "audio_nvdm_common.h"
#include "preloader_pisplit.h"
#include "stream_nvkey_struct.h"
#endif /* AIR_SILENCE_DETECTION_ENABLE */
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
#include "volume_estimator_interface.h"
#include "dtm.h"
#include "preloader_pisplit.h"
#include "audio_nvdm_common.h"
#include "stream_nvkey_struct.h"
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
#include "scenario_audio_common.h"
#include "hal_nvic_internal.h"
#include "task.h"

/* Private define ------------------------------------------------------------*/
#define ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_DUMP                      1
#define ULL_AUDIO_V2_DONGLE_UL_PATH_DEBUG_DUMP                      1
#define ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG                       0
#define ULL_AUDIO_V2_DONGLE_UL_PATH_DEBUG_LOG                       0

#define DL_MAX_USB_CARD_NUMBER                                      2
#define DL_CLOCK_SKEW_CHECK_COUNT                                   (4)
#define DL_PROCESS_TIME_MAX                                         (1250-10)
#define DL_ULD_PROCESS_TIME_MAX                                     (1000)
#define DL_PRELOADER_RESERVED_USB_FRAMES                            (5) /* Keep enough time to avoid of conflict between preloader process and final process  */
#define DL_STOP_PADDING_PKG_NUMBER                                  5
#define DL_PLAY_TIME_CHECK_THRESHOLD                                1000000 /* Unit of us */

#define UL_CLOCK_SKEW_CHECK_COUNT                                   (4)
#define UL_BT_TIMESTAMP_DIFF                                        (2)  /* 0.625ms/0.3125ms */
#define UL_PLAYEN_DELAY_FRAME_5000US                                (10-4) /* procsess time - 1ms */
#define UL_FIRST_PACKET_SAFE_DELAY_FRAME_5000US                     (2)
#define UL_BT_RETRY_WINDOW_FRAME_5000US                             (18)
#define UL_BT_INTERVAL_FRAME_5000US                                 (16)
#define UL_PLAYEN_DELAY_FRAME_5000US_LINE_OUT                       (0) /* procsess time 3ms */
#define UL_FIRST_PACKET_SAFE_DELAY_FRAME_5000US_LINE_OUT            (-2)
#define UL_PLAYEN_DELAY_FRAME_5000US_I2S_SLV_OUT                    (0) /* procsess time 3ms */
#define UL_FIRST_PACKET_SAFE_DELAY_FRAME_5000US_I2S_SLV_OUT         (-2)
#define UL_PROCESS_TIME_FRAME                                       (3)
#define UL_USB_PREFILL_SAMPLES_FRAME                                (1)
#define UL_LINE_PREFILL_SAMPLES_FRAME                               (0)
#define UL_I2S_SLV_PREFILL_SAMPLES_FRAME                            (1)
#define UL_PACKET_ANCHOR_IN_MIXER_INVAILD_VALUE                     (0xFFFFFFFF)

#ifndef min
#define min(_a, _b)   (((_a)<(_b))?(_a):(_b))
#endif

#ifndef max
#define max(_a, _b)   (((_a)<(_b))?(_b):(_a))
#endif

log_create_module(ull_v2_log, PRINT_LEVEL_INFO);
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
void audio_dongle_set_dl12_for_i2s_slave_out(ull_audio_v2_dongle_ul_handle_t *dongle_handle, bool control);
#endif
/* Private typedef -----------------------------------------------------------*/
typedef struct  {
    U16 DataOffset; /* offset of payload */
    U16 _reserved_word_02h;
    U32 TimeStamp; /* this CIS/BIS link's CLK, Unit:312.5us */
    U16 ConnEvtCnt; /* event count seem on airlog, for debug propose */
    U8 SampleSeq;  /* Sameple sequence of this SDU interval Ex:0,1,2... */
    U8 _reserved_byte_0Bh;
    U8 PduHdrLo;
    // U8 _reserved_byte_0Dh;
    U8 valid_packet; /* padding packet: 0x03, valid packet: 0x01, invalid packet 0x00 */
    U16 PduLen ; /* payload size */
    U16 DataLen;
    U16 _reserved_word_12h;
    // U32 _reserved_long_0;
    U32 crc32_value;
    U32 _reserved_long_1;
} ULL_AUDIO_V2_HEADER;

typedef struct {
    uint8_t scenario_type;
    uint8_t scenario_sub_id;
} dl_mixer_param_t;

typedef struct {
    uint32_t vol_ch;
    int32_t  vol_gain;
} vol_gain_t;

typedef struct {
    uint8_t ch_choose;
    uint8_t ch_connection[8];
} ull_audio_v2_connection_info_t;

typedef union {
    vol_gain_t                vol_gain;
    dl_mixer_param_t          dl_mixer_param;
    ull_audio_v2_connection_info_t connection_info;
} ull_audio_v2_dongle_runtime_config_operation_param_t, *ull_audio_v2_dongle_runtime_config_operation_param_p;

typedef struct {
    ull_audio_v2_dongle_runtime_config_operation_t          config_operation;
    ull_audio_v2_dongle_runtime_config_operation_param_t    config_param;
} ull_audio_v2_runtime_config_param_t, *ull_audio_v2_dongle_runtime_config_param_p;

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
} ull_audio_v2_init_play_info_t;

#ifdef AIR_SILENCE_DETECTION_ENABLE
typedef enum {
    ULL_AUDIO_V2_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW = 0,
    ULL_AUDIO_V2_DONGLE_SILENCE_DETECTION_STATUS_SILENCE,
    ULL_AUDIO_V2_DONGLE_SILENCE_DETECTION_STATUS_NOT_SILENCE,
} ull_audio_v2_dongle_silence_detection_status_t;

typedef struct {
    volume_estimator_port_t *port;
    int16_t enable;
    int16_t nvkey_enable;
    void *nvkey_buf;
    uint32_t frame_size;
    uint32_t data_size;
    uint32_t data_buf_size;
    void *data_buf;
    int32_t current_db;
    int32_t failure_threshold_db;
    int32_t effective_threshold_db;
    uint32_t effective_delay_ms;
    uint32_t effective_delay_us_count[8]; /* By channel */
    uint32_t failure_delay_ms;
    uint32_t failure_delay_us_count[8]; /* By channel */
    ull_audio_v2_dongle_silence_detection_status_t status;
    uint32_t total_channel;
    uint32_t data_size_channel;
    uint32_t sample_rate;
} ull_audio_v2_dongle_silence_detection_handle_t;
#endif /* AIR_SILENCE_DETECTION_ENABLE */

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
typedef struct {
    int16_t enable;
    uint16_t gain_setting_update_done;
    volume_estimator_port_t *port;
    ull_audio_v2_dongle_dl_handle_t *chat_dongle_handle;
    sw_gain_port_t *game_path_gain_port;
    void *nvkey_buf;
    uint32_t frame_size;
    uint32_t data_size;
    uint32_t data_buf_size;
    void *data_buf;
    int32_t current_db;
    int32_t failure_threshold_db;
    int32_t effective_threshold_db;
    int32_t adjustment_amount_db;
    int32_t up_step_db;
    int32_t down_step_db;
    uint32_t effective_delay_ms;
    uint32_t effective_delay_us_count;
    uint32_t failure_delay_ms;
    uint32_t failure_delay_us_count;
    bool active_flag;
    int32_t chat_path_default_vol_gain[2];
    int32_t game_path_default_vol_gain[2];
    int32_t game_path_balance_vol_gain[2];
} ull_audio_v2_dongle_vol_balance_handle_t;
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
static bool g_i2s_slv_tracking_start_flag = true;
static uint32_t gpt_i2s_slv_trigger_handle = 0;
hal_gpt_callback_t ull_audio_v2_dongle_dl_i2s_slv_gpt_cb(void *user_data);
#endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE */
static ull_audio_v2_dongle_bt_info_t ull_audio_v2_dl_bt_info[ULL_AUDIO_V2_DATA_CHANNEL_NUMBER];
static ull_audio_v2_dongle_bt_info_t ull_audio_v2_ul_bt_info[ULL_AUDIO_V2_DATA_CHANNEL_NUMBER];
extern const uint32_t crc32_tab[];
static ull_audio_v2_init_play_info_t ull_audio_v2_bt_init_play_info =
{
    .dl_retransmission_window_clk = UL_BT_RETRY_WINDOW_FRAME_5000US,
    .dl_retransmission_window_phase = 0,
};
#if defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
static uint32_t ull_audio_v2_ul_sw_timer = 0;
static uint32_t ull_audio_v2_ul_sw_timer_count = 0;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */
static uint32_t ull_audio_v2_ul_packet_anchor_in_mixer = 0;
static uint32_t ull_audio_v2_ul_packet_size_in_mixer = 0;

//#if defined(AIR_ECNR_POST_PART_ENABLE)
static bool g_nr_offlad_status = false;
static uint8_t g_post_ec_gain;

extern CONNECTION_IF g_ull_audio_v2_dongle_usb_out_broadcast_streams;
extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_out_broadcast_nr_offload[];
//#endif /* AIR_ECNR_POST_PART_ENABLE */

#ifdef AIR_SILENCE_DETECTION_ENABLE
static uint32_t ull_audio_v2_dl_sw_timer = 0;
static uint32_t ull_audio_v2_dl_sw_timer_count = 0;
static bool ull_audio_v2_dl_without_bt_link_mode_enable = false;
#endif /* AIR_SILENCE_DETECTION_ENABLE */
static uint32_t ull_audio_v2_dl_stream_sw_timer;
static bool g_ull_audio_v2_dl_stream_irq_is_trigger;
static bool g_ull_audio_v2_dl_stream_sw_timer_is_trigger;
static bool g_ull_audio_v2_dl_play_info_need_trigger;
#if defined(AIR_AUDIO_ULD_ENCODE_ENABLE)
static uint32_t g_ull_audio_v2_dl_uld_enc_cnt = 0;
static uld_enc_port_t *g_ull_audio_v2_dl_uld_enc_port = NULL;
#endif /* AIR_AUDIO_ULD_ENCODE_ENABLE */
static uint8_t g_ull_audio_v2_dongle_dl_dummy[512];
static bool g_ull_audio_v2_dongle_dl_dummy_process_flag[DL_MAX_USB_CARD_NUMBER];
static uint32_t g_ull_audio_v2_dongle_dl_dummy_process_cnt[DL_MAX_USB_CARD_NUMBER];

/* Public variables ----------------------------------------------------------*/
ull_audio_v2_dongle_dl_handle_t *ull_audio_v2_dongle_first_dl_handle = NULL;
ull_audio_v2_dongle_ul_handle_t *ull_audio_v2_dongle_first_ul_handle = NULL;
#ifdef AIR_SILENCE_DETECTION_ENABLE
ull_audio_v2_dongle_silence_detection_handle_t ull_audio_v2_dongle_silence_detection_handle[AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_SUB_ID_MAX-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0];
#endif /* AIR_SILENCE_DETECTION_ENABLE */
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
ull_audio_v2_dongle_vol_balance_handle_t ull_audio_v2_dongle_vol_balance_handle;
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

/* Private functions ---------------------------------------------------------*/
EXTERN VOID StreamDSP_HWSemaphoreTake(VOID);
EXTERN VOID StreamDSP_HWSemaphoreGive(VOID);
extern VOID audio_transmitter_share_information_update_read_offset(SOURCE source, U32 ReadOffset);
extern VOID audio_transmitter_share_information_fetch(SOURCE source, SINK sink);
extern VOID audio_transmitter_share_information_update_write_offset(SINK sink, U32 WriteOffset);
extern void bt_common_share_information_fetch(SOURCE source, SINK sink);
extern void bt_common_share_information_update_read_offset(SOURCE source, U32 ReadOffset);
extern VOID audio_transmitter_share_information_fetch(SOURCE source, SINK sink);
extern VOID MCE_GetBtClk(BTCLK* pCurrCLK, BTPHASE* pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);
extern CONNECTION_IF *port_audio_transmitter_get_connection_if(audio_transmitter_scenario_type_t scenario_id, audio_transmitter_scenario_sub_id_t sub_id);
extern hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);
extern hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask);

#if (defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
void ull_audio_v2_dongle_dl_init_afe_in(ull_audio_v2_dongle_dl_handle_t *dongle_handle, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param);
void ull_audio_v2_dongle_dl_deinit_afe_in(ull_audio_v2_dongle_dl_handle_t *dongle_handle);
void ull_audio_v2_dongle_dl_start_afe_in(ull_audio_v2_dongle_dl_handle_t *dongle_handle);
void ull_audio_v2_dongle_dl_stop_afe_in(ull_audio_v2_dongle_dl_handle_t *dongle_handle);
static uint32_t ull_audio_v2_dongle_dl_calculate_afe_in_jitter_bt_audio(ull_audio_v2_dongle_dl_handle_t *dongle_handle, uint32_t gpt_count);
#endif

static uint32_t ull_audio_v2_codec_get_frame_size(audio_dsp_codec_type_t *codec_type, audio_codec_param_t *codec_param)
{
    uint32_t frame_size = 0;

    switch (*codec_type)
    {
        case AUDIO_DSP_CODEC_TYPE_LC3:
            frame_size = codec_param->lc3.bit_rate * codec_param->lc3.frame_interval / 8 / 1000 / 1000;
            break;

        case AUDIO_DSP_CODEC_TYPE_LC3PLUS:
            frame_size = codec_param->lc3plus.bit_rate * codec_param->lc3plus.frame_interval / 8 / 1000 / 1000;
            break;

        case AUDIO_DSP_CODEC_TYPE_OPUS:
            frame_size = codec_param->opus.bit_rate * codec_param->opus.frame_interval / 8 / 1000 / 1000;
            break;

        case AUDIO_DSP_CODEC_TYPE_ULD:
            frame_size = codec_param->uld.bit_rate * codec_param->uld.frame_interval / 8 / 1000 / 1000;
            break;

        default:
            ULL_V2_LOG_E("[ULL Audio V2][ERROR]This codec is not supported at now, %u\r\n", 1, *codec_type);
            AUDIO_ASSERT(0);
            break;
    }

    return frame_size;
}

static uint32_t usb_audio_get_frame_size(audio_dsp_codec_type_t *usb_type, audio_codec_param_t *usb_param)
{
    uint32_t frame_size = 0;
    uint32_t samples = 0;
    uint32_t channel_num = 0;
    uint32_t resolution_size = 0;

    if (*usb_type == AUDIO_DSP_CODEC_TYPE_PCM)
    {
        frame_size = 1;

        switch (usb_param->pcm.frame_interval)
        {
            case 1000:
                break;

            default:
                AUDIO_ASSERT(0);
                break;
        }

        switch (usb_param->pcm.sample_rate)
        {
            case 44100:
                samples = 44;
                break;

            case 16000:
            case 32000:
            case 48000:
            case 96000:
                samples = usb_param->pcm.sample_rate/1000;
                break;

            default:
                AUDIO_ASSERT(0);
                break;
        }

        switch (usb_param->pcm.channel_mode)
        {
            case 1:
                channel_num = 1;
                break;

            case 2:
                channel_num = 2;
                break;

            case 8:
                channel_num = 8;
                break;

            default:
                AUDIO_ASSERT(0);
                break;
        }

        switch (usb_param->pcm.format)
        {
            case HAL_AUDIO_PCM_FORMAT_S16_LE:
                resolution_size = 2;
                break;

            case HAL_AUDIO_PCM_FORMAT_S24_LE:
                resolution_size = 3;
                break;

            default:
                AUDIO_ASSERT(0);
                break;
        }
    }
    else
    {
        frame_size = 0;
    }

    frame_size = frame_size * samples * channel_num * resolution_size;

    return frame_size;
}

ATTR_TEXT_IN_IRAM static uint32_t ull_audio_v2_dongle_crc32_generate(uint8_t *ptr, uint32_t length, uint32_t crc_init)
{
    const uint8_t *p;

    p = ptr;
    crc_init = crc_init ^ ~0U;

    while (length--) {
        crc_init = crc32_tab[(crc_init ^ *p++) & 0xFF] ^ (crc_init >> 8);
    }

    return crc_init ^ ~0U;
}

ATTR_TEXT_IN_IRAM static uint32_t ull_audio_v2_dongle_crc32_generate_dummy(uint32_t length, uint32_t crc_init)
{
    crc_init = crc_init ^ ~0U;

    while (length--) {
        crc_init = crc32_tab[(crc_init ^ 0) & 0xFF] ^ (crc_init >> 8);
    }

    return crc_init ^ ~0U;
}


void ull_audio_v2_dongle_resolution_config(DSP_STREAMING_PARA_PTR stream)
{
    SINK   sink   = stream->sink;
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    if ((sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) ||
        (sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
        (sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) {
            stream->callback.EntryPara.resolution.source_in_res = (stream->source->param.audio.format <= HAL_AUDIO_PCM_FORMAT_U16_BE)
                                                        ? RESOLUTION_16BIT
                                                        : RESOLUTION_32BIT;
            if (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                stream->callback.EntryPara.resolution.feature_res  = RESOLUTION_32BIT;
                stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_32BIT;
            } else {
                stream->callback.EntryPara.resolution.feature_res  = RESOLUTION_16BIT;
                stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_16BIT;
            }
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_dl_sw_mixer_connect(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    uint32_t saved_mask = 0;
    if (dongle_handle->index == 1) {
        /* it is the first dl stream */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, true);
        /* update mixer status */
        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
        dongle_handle->mixer_status = ULL_AUDIO_V2_DONGLE_MIXER_UNMIX;
        hal_nvic_restore_interrupt_mask(saved_mask);
    } else {
        /* it is not the first dl stream, needs to do mixer connection based on BT link info */
        ull_audio_v2_dongle_dl_handle_t *c_handle = NULL;
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, true);
        c_handle = ull_audio_v2_dongle_first_dl_handle;
        for (uint32_t i=0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++)
        {
            if (c_handle != dongle_handle)
            {
                for (uint32_t j = 0; j < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; j++)
                {
                    if (dongle_handle->sink_info.bt_out.bt_info[j] != NULL)
                    {
                        for (uint32_t k = 0; k < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; k++)
                        {
                            if (c_handle->sink_info.bt_out.bt_info[k] != NULL)
                            {
                                if (dongle_handle->sink_info.bt_out.bt_info[j] == c_handle->sink_info.bt_out.bt_info[k])
                                {
                                    /* in here, it means the currnet dongle_handle ch_k should be mixed with the new dongle_handle ch_j */
                                    stream_function_sw_mixer_channel_connect(c_handle->mixer_member, k+1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle->mixer_member, j+1);
                                    stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, j+1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, c_handle->mixer_member, k+1);
                                    /* update mixer status */
                                    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                                    c_handle->mixer_status      = ULL_AUDIO_V2_DONGLE_MIXER_MIX;
                                    dongle_handle->mixer_status = ULL_AUDIO_V2_DONGLE_MIXER_MIX;
                                    hal_nvic_restore_interrupt_mask(saved_mask);
                                    DSP_MW_LOG_I("[ULL Audio V2][DL][scenario type %d-%d][handle 0x%x]sw mixer member 0x%x ch%d connect to [handle 0x%x]sw mixer member 0x%x ch%d\r\n", 8,
                                                dongle_handle->source->scenario_type,
                                                dongle_handle->sink->scenario_type,
                                                dongle_handle,
                                                dongle_handle->mixer_member,
                                                j+1,
                                                c_handle,
                                                c_handle->mixer_member,
                                                k+1);
                                }
                            }
                        }
                    }
                }
            }
            /* switch to the next dl stream */
            c_handle = c_handle->next_dl_handle;
        }
    }
}

void ull_audio_v2_dongle_dl_sw_mixer_disconnect(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    uint32_t mixer_count = 0;
    ull_audio_v2_dongle_dl_handle_t *c_handle = ull_audio_v2_dongle_first_dl_handle;
    for (uint32_t i=0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++) {
        if (c_handle->mixer_status == ULL_AUDIO_V2_DONGLE_MIXER_MIX) {
            mixer_count += 1;
        }
        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }
    if (mixer_count <= 2) {
        c_handle = ull_audio_v2_dongle_first_dl_handle;
        for (uint32_t i=0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++) {
            if (c_handle->mixer_status == ULL_AUDIO_V2_DONGLE_MIXER_MIX) {
                /* reset mixer status */
                c_handle->mixer_status = ULL_AUDIO_V2_DONGLE_MIXER_UNMIX;
            }
            /* switch to the next dl stream */
            c_handle = c_handle->next_dl_handle;
        }
    }
    dongle_handle->mixer_status = ULL_AUDIO_V2_DONGLE_MIXER_UNMIX;
}

/******************************************************************************/
/*               ULL audio 2.0 dongle DL path Private Functions               */
/******************************************************************************/
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static ull_audio_v2_dongle_dl_handle_t *ull_audio_v2_dongle_dl_get_handle(void *owner)
{
    uint32_t saved_mask;
    int16_t count;
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = NULL;
    ull_audio_v2_dongle_dl_handle_t *c_handle = NULL;

    dongle_handle = malloc(sizeof(ull_audio_v2_dongle_dl_handle_t));
    if (dongle_handle == NULL)
    {
        AUDIO_ASSERT(0);
    }
    memset(dongle_handle, 0, sizeof(ull_audio_v2_dongle_dl_handle_t));

    dongle_handle->total_number = -1;
    dongle_handle->index = -1;
    dongle_handle->owner = owner;
    dongle_handle->next_dl_handle = NULL;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (ull_audio_v2_dongle_first_dl_handle == NULL)
    {
        /* there is no item on the handle list */
        dongle_handle->total_number = 1;
        dongle_handle->index = 1;

        /* set this handle as the head of the handle list */
        ull_audio_v2_dongle_first_dl_handle = dongle_handle;
    }
    else
    {
        /* there are other items on the handle list */
        count = 1;
        c_handle = ull_audio_v2_dongle_first_dl_handle;
        while (c_handle != NULL)
        {
            count += 1;

            c_handle->total_number += 1;
            if (c_handle->total_number <= 0)
            {
                /* error status */
                AUDIO_ASSERT(0);
            }

            if (c_handle->next_dl_handle == NULL)
            {
                /* c_handle is the last item on the list now */
                dongle_handle->total_number = c_handle->total_number;
                dongle_handle->index = c_handle->total_number;

                /* dongle_handle is the last item on the list now */
                c_handle->next_dl_handle = dongle_handle;

                break;
            }

            c_handle = c_handle->next_dl_handle;
        }
        if ((c_handle == NULL) || (dongle_handle->total_number != count))
        {
            AUDIO_ASSERT(0);
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    return dongle_handle;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_dl_release_handle(ull_audio_v2_dongle_dl_handle_t *handle)
{
    uint32_t saved_mask, i, count;
    ull_audio_v2_dongle_dl_handle_t *l_handle = NULL;
    ull_audio_v2_dongle_dl_handle_t *c_handle = NULL;
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = NULL;

    if ((handle == NULL) || (ull_audio_v2_dongle_first_dl_handle == NULL))
    {
        AUDIO_ASSERT(0);
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (ull_audio_v2_dongle_first_dl_handle->total_number <= 0)
    {
        /* error status */
        AUDIO_ASSERT(0);
    }
    else if ((ull_audio_v2_dongle_first_dl_handle->total_number == 1) &&
            (ull_audio_v2_dongle_first_dl_handle == handle))
    {
        /* this handle is only one item on handle list */
        if (ull_audio_v2_dongle_first_dl_handle->next_dl_handle != NULL)
        {
            AUDIO_ASSERT(0);
        }

        /* there is no item on the handle list */
        ull_audio_v2_dongle_first_dl_handle = NULL;
    }
    else if ((ull_audio_v2_dongle_first_dl_handle->total_number > 1) &&
            (ull_audio_v2_dongle_first_dl_handle == handle))
    {
        /* this handle is the first item on handle list, but there are other handles on the list */
        c_handle = ull_audio_v2_dongle_first_dl_handle;
        count = ull_audio_v2_dongle_first_dl_handle->total_number;
        for (i=0; i < count; i++)
        {
            c_handle->total_number -= 1;
            c_handle->index -= 1;

            c_handle = c_handle->next_dl_handle;
        }
        if (c_handle != NULL)
        {
            /* error status */
            AUDIO_ASSERT(0);
        }

        /* change the next iten to the head */
        ull_audio_v2_dongle_first_dl_handle = handle->next_dl_handle;
    }
    else
    {
        /* this handle is the middle item on handle list */
        c_handle = ull_audio_v2_dongle_first_dl_handle;
        count = ull_audio_v2_dongle_first_dl_handle->total_number;
        if (count == 1)
        {
            /* error status */
            AUDIO_ASSERT(0);
        }
        for (i=0; i < count; i++)
        {
            if (c_handle == handle)
            {
                if (dongle_handle != NULL)
                {
                    /* error status */
                    AUDIO_ASSERT(0);
                }

                /* find out the handle on the list */
                dongle_handle = handle;
                l_handle->next_dl_handle = c_handle->next_dl_handle;
            }

            if (dongle_handle == NULL)
            {
                /* only the total_number of the handle in front of the released handle needs to be decreased */
                c_handle->total_number -= 1;
            }
            else
            {
                /* Both the total_number and the index of the handle in back of the released handle need to be decreased */
                c_handle->total_number -= 1;
                c_handle->index -= 1;
            }

            l_handle = c_handle;
            c_handle = c_handle->next_dl_handle;
        }
        if ((c_handle != NULL) || (dongle_handle == NULL))
        {
            /* error status */
            AUDIO_ASSERT(0);
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    free(handle);
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_mixer_precallback(sw_mixer_member_t *member, void *para)
{
    uint32_t in_frame_size;

    in_frame_size = stream_function_get_output_size(para);
    if (in_frame_size == 0)
    {
        /* it means the stream is in preload mode and this precallback will force the sw mixer to return */
        stream_function_sw_mixer_member_force_to_exit(member);
    }
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_mixer_postcallback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size)
{
    uint32_t i;
    ull_audio_v2_dongle_dl_handle_t *c_handle;
    SOURCE source;
    bool all_streams_in_mixer = true;
    UNUSED(para);

    /* change this handle's data status */
    source = (SOURCE)(member->owner);
    c_handle = (ull_audio_v2_dongle_dl_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    c_handle->data_status = ULL_AUDIO_V2_DONGLE_DL_DATA_IN_MIXER;

    /* check if this stream is unmixed or the first packet is not ready */
    if (c_handle->mixer_status == ULL_AUDIO_V2_DONGLE_MIXER_UNMIX)
    {
        /* clear this stream's input buffer */
        stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);

        /* change this handle data status */
        c_handle->data_status = ULL_AUDIO_V2_DONGLE_DL_DATA_EMPTY;
        /* this stream is unmixed now, so we can directly return here */
        return;
    }
    else if (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY)
    {
        /* clear this stream's input buffer */
        stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);

        /* change this handle data status */
        c_handle->data_status = ULL_AUDIO_V2_DONGLE_DL_DATA_EMPTY;
        /* in here, it means it is the first packet, we need to drop the later sink flow */
        *out_frame_size = 0;

        return;
    }

    /* check if all mixed stream data is in this mixer */
    c_handle = ull_audio_v2_dongle_first_dl_handle;
    for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++)
    {
        if ((c_handle->data_status == ULL_AUDIO_V2_DONGLE_DL_DATA_IN_STREAM) &&
            (c_handle->mixer_status != ULL_AUDIO_V2_DONGLE_MIXER_UNMIX) &&
            (c_handle->first_packet_status != ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY))
        {
            /* this stream should be mixed, but its data is not in this mixer now.
               so we need to bypass this mix result in this time */
            all_streams_in_mixer = false;
            break;
        }
        // ULL_V2_LOG_W("[ULL Audio V2][DL][handle 0x%x]all_streams_in_mixer %d, %d, %d, %d\r\n", 5, c_handle, all_streams_in_mixer, c_handle->data_status, c_handle->mixer_status, c_handle->first_packet_status);
        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }
    /* check if the output data in this time includes all stream data */
    if ((*out_frame_size != 0) && (all_streams_in_mixer == true))
    {
        /* all_streams_in_mixer is true, so all stream data is in this mixer.
           So we can clean all mixed streams' input buffers now and the mix result are sent to the sink */
        c_handle = ull_audio_v2_dongle_first_dl_handle;
        for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++)
        {
            if (c_handle->mixer_status != ULL_AUDIO_V2_DONGLE_MIXER_UNMIX)
            {
                /* clear this stream's input buffer */
                stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);

                /* change this handle data status */
                c_handle->data_status = ULL_AUDIO_V2_DONGLE_DL_DATA_EMPTY;
            }
            /* switch to the next dl stream */
            c_handle = c_handle->next_dl_handle;
        }
    }
    else
    {
        /* all_streams_in_mixer is false, so some stream data is not in this mixer.
           So we need to bypass this mix result */
        *out_frame_size = 0;
    }
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_prepare_packet_check(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    uint32_t avail_size;
    SOURCE source = dongle_handle->source;
    uint32_t unprocess_frames;
    uint32_t remain_samples_in_sw_buffer;
    uint32_t unprocess_samples;
    uint32_t sample_size, check_threshold;
    uint32_t total_buffer_size;

    /* get actual data size include header size in the share buffer */
    audio_transmitter_share_information_fetch(source, NULL);
    if (source->streamBuffer.ShareBufferInfo.read_offset < source->streamBuffer.ShareBufferInfo.write_offset)
    {
        /* normal case */
        avail_size = source->streamBuffer.ShareBufferInfo.write_offset - source->streamBuffer.ShareBufferInfo.read_offset;
    }
    else if (source->streamBuffer.ShareBufferInfo.read_offset == source->streamBuffer.ShareBufferInfo.write_offset)
    {
        if(source->streamBuffer.ShareBufferInfo.bBufferIsFull == true)
        {
            /* buffer is full, so read_offset == write_offset */
            avail_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;
        }
        else
        {
            /* buffer is empty, so read_offset == write_offset */
            avail_size = 0;
            // ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING][handle 0x%x]data is 0\r\n", 1, dongle_handle);
        }
    }
    else
    {
        /* buffer wrapper case */
        avail_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num
                    - source->streamBuffer.ShareBufferInfo.read_offset
                    + source->streamBuffer.ShareBufferInfo.write_offset;
    }

    /* get unprocess frame number */
    unprocess_frames = avail_size/source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
    /* if this is the first time, check if the data in usb buffer is larger than starting standard */
    if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY)
    {
        if (unprocess_frames < dongle_handle->source_info.usb_in.frame_max_num)
        {
            /* update data state machine */
            dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_DL_DATA_EMPTY;
            /* there is a fetch requirement */
            dongle_handle->fetch_count = 0;
            dongle_handle->source_preload_count = 0;
        }
        else
        {
            /* pre-release some buffer space when the avail size of source avm buffer is almost full, to prevent of MCU write fail during the 1st stream process */
            if (unprocess_frames > source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num - dongle_handle->sink_info.bt_out.frame_interval / 1000) {
                total_buffer_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;
                source->streamBuffer.ShareBufferInfo.read_offset += (dongle_handle->sink_info.bt_out.frame_interval / 1000) * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
                source->streamBuffer.ShareBufferInfo.read_offset %= total_buffer_size;
                audio_transmitter_share_information_update_read_offset(source, source->streamBuffer.ShareBufferInfo.read_offset);
                ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING][handle 0x%x] pre-release some AVM buffer space, %u, %u, %u\r\n", 5, dongle_handle, unprocess_frames, source->streamBuffer.ShareBufferInfo.read_offset, source->streamBuffer.ShareBufferInfo.write_offset);
            }
            /* update data state machine */
            dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_DL_DATA_IN_STREAM;
            /* there is a fetch requirement */
            dongle_handle->fetch_count = 1;
            dongle_handle->source_preload_count = 0;
        }
    }
    else
    {
        /* get unprocess samples */
        unprocess_samples = 0;
        sample_size = (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) ? sizeof(int16_t) : sizeof(int32_t);
        if (dongle_handle->stream_mode == ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_STANDBY)
        {
            remain_samples_in_sw_buffer = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port_0, 1) / sample_size;
            unprocess_samples = unprocess_frames*dongle_handle->src_out_frame_samples+remain_samples_in_sw_buffer;
        }
        else if (dongle_handle->process_frames == 0)
        {
            remain_samples_in_sw_buffer = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port_0, 1) / sample_size;
            unprocess_samples = unprocess_frames*dongle_handle->src_out_frame_samples+remain_samples_in_sw_buffer;
        }
        else
        {
            unprocess_samples = unprocess_frames*dongle_handle->src_out_frame_samples+dongle_handle->buffer0_remain_samples;
        }

        /* check if usb data is enough for one DL packet */
        if (g_ull_audio_v2_dongle_dl_dummy_process_cnt[dongle_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0] > 0) {
            check_threshold = dongle_handle->sink_info.bt_out.frame_samples + 1;
        } else {
            check_threshold = dongle_handle->sink_info.bt_out.frame_samples;
        }
        if (unprocess_samples >= check_threshold)
        {
            /* update data state machine */
            dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_DL_DATA_IN_STREAM;
            /* there is a fetch requirement */
            dongle_handle->fetch_count = 1;
        }
        else
        {
            /* update data state machine */
            dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_DL_DATA_EMPTY;
            /* decrease fetch count */
            dongle_handle->fetch_count = 0;
            // ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING][handle 0x%x]data is not enough, %d, %d, %d\r\n", 4, dongle_handle, unprocess_frames*dongle_handle->source_info.usb_in.frame_samples, remain_samples_0, dongle_handle->sink_info.bt_out.frame_samples);
        }
    }

#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (dongle_handle->data_status == ULL_AUDIO_V2_DONGLE_DL_DATA_EMPTY)
    {
        if ((hal_nvic_query_exception_number() > 0) && (dongle_handle->total_number == 1))
        {
            audio_scenario_type_t audio_scenario_type;
            ull_audio_v2_dongle_silence_detection_handle_t *silence_detection_handle;

            /* get scenario and handle */
            audio_scenario_type = AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0+dongle_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0;
            silence_detection_handle = &ull_audio_v2_dongle_silence_detection_handle[dongle_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0];
            if (silence_detection_handle->enable)
            {
                /* copy all zere data into the buffer only when there is only one streams is opened and there is no usb data in this stream */
                silence_detection_handle->data_size = dongle_handle->sink_info.bt_out.frame_samples * sizeof(int32_t);
                memset(silence_detection_handle->data_buf, 0, silence_detection_handle->data_size);

                /* Set check channel to only 1 as the data empty is detected */
                silence_detection_handle->total_channel = 1;
                silence_detection_handle->data_size_channel = silence_detection_handle->data_size;

                /* send msg to DTM task to do silence detection */
                if (DTM_query_queue_avail_number() >= 2) {
                    DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_PROCESS, (uint32_t)audio_scenario_type, true);
                } else {
                    ULL_V2_LOG_I("[ULL Audio V2][DL]scenario %d-%d abort silence detection, DTM queue full\r\n", 2,
                                            source->param.data_dl.scenario_type,
                                            source->param.data_dl.scenario_sub_id);
                }
            }
        }
    }
#endif /* AIR_SILENCE_DETECTION_ENABLE */
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_ccni_handler(hal_ccni_event_t event, void * msg)
{
    SINK sink;
    uint32_t saved_mask;
    uint32_t i, no_usb_data_1, no_usb_data_2, exist_other_stream;
    uint32_t gpt_count;
    uint32_t bt_count;
    uint32_t blk_index, pdu_size, new_bit_rate;
    hal_ccni_message_t *ccni_msg = msg;
    ull_audio_v2_dongle_dl_handle_t *c_handle;
    static uint32_t error_count = 0;
    DSP_STREAMING_PARA_PTR usb_saved_stream;

    UNUSED(event);

#if ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_V2_LOG_I("[ULL Audio V2][DL][DEBUG][ull_audio_v2_dongle_dl_ccni_handler]entry: %d, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG */

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    /* get timestamp for debug */
    bt_count = ccni_msg->ccni_message[0];
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    /* BT controller will send write index info on ccni msg */
    blk_index = (uint16_t)(ccni_msg->ccni_message[1]);
    pdu_size = (uint16_t)(ccni_msg->ccni_message[1] >> 16);
    hal_nvic_restore_interrupt_mask(saved_mask);

    /* check if there is any DL stream */
    if (ull_audio_v2_dongle_first_dl_handle == NULL)
    {
        if ((error_count%1000) == 0)
        {
            ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING]dl dongle_handle is NULL, %d\r\n", 1, error_count);
        }
        error_count += 1;
        goto _ccni_return;
    }
    else
    {
        error_count = 0;
    }

#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (ull_audio_v2_dl_without_bt_link_mode_enable == true)
    {
        /* bypss preload mode in without_bt_link_mode */
        ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING] without_bt_link_mode_enable is enable, unexpected BTccni msg is comming\r\n", 0);
        goto _ccni_return;
    }
#endif /* AIR_SILENCE_DETECTION_ENABLE */

    /* trigger all started DL stream one by one */
    exist_other_stream = false;
    no_usb_data_1 = true;
    no_usb_data_2 = true;
    usb_saved_stream = NULL;
    c_handle = ull_audio_v2_dongle_first_dl_handle;
    for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++)
    {
        if ((c_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_START) || (c_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_RUNNING))
        {
            /* Get the current bitrate */
            if (c_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS) {
                new_bit_rate = (pdu_size * 8 * 1000 * 1000) / c_handle->sink_info.bt_out.frame_interval;
                if (new_bit_rate != c_handle->sink_info.bt_out.bit_rate) {
                    ULL_V2_LOG_I("[ULL Audio V2][DL] bit-rate from %d to %d bps\r\n", 2, c_handle->sink_info.bt_out.bit_rate, new_bit_rate);
                    c_handle->sink_info.bt_out.bit_rate = new_bit_rate;
                    c_handle->sink_info.bt_out.frame_size = (c_handle->sink_info.bt_out.bit_rate * c_handle->sink_info.bt_out.frame_interval) / (8 * 1000 * 1000);
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
                    uint32_t j;
                    stream_codec_encoder_lc3plus_set_bitrate(LC3PLUS_ENC_PORT_0, c_handle->sink_info.bt_out.bit_rate);
                    for (j = 0; j < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; j++) {
                        if (c_handle->sink_info.bt_out.bt_info[j] != NULL) {
                            c_handle->sink_info.bt_out.bt_info[j]->bt_link_info.codec_param.lc3plus.bit_rate = new_bit_rate;
                            c_handle->sink_info.bt_out.bt_info[j]->bt_link_info.codec_param.lc3plus.frame_size = pdu_size;
                        }
                    }
#endif
                }
            }

            switch (c_handle->sub_id)
            {
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
                    /* get sink */
                    sink = (SINK)c_handle->owner;
                    if ((sink == NULL) || (sink->transform == NULL))
                    {
                        break;
                    }
                    /* set timestamp for debug */
                    c_handle->ccni_in_bt_count  = bt_count;
                    c_handle->ccni_in_gpt_count = gpt_count;
                    /* set blk index */
                    c_handle->ccni_blk_index    = blk_index;
                    //hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                    /* check if the stream needs to prepare a new DL packet */
                    ull_audio_v2_dongle_dl_prepare_packet_check(c_handle);
                    //hal_nvic_restore_interrupt_mask(saved_mask);
                    /* Handler the stream */
                    AudioCheckTransformHandle(sink->transform);

                    DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(sink->transform->source, sink);
                    if (usb_saved_stream == NULL) {
                        usb_saved_stream = stream;
                    }
                    if (stream->callback.Status == CALLBACK_HANDLER) {
                        if (c_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) {
                            no_usb_data_1 = false;
                        } else if (c_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1) {
                            no_usb_data_2 = false;
                        }
                    }
                    break;

                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0:
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE)
                {
                    BUFFER_INFO *buffer_info = NULL;
                    AUDIO_PARAMETER *runtime = NULL;
                    uint32_t data_size;
                    uint32_t buffer_per_channel_shift;
                    SOURCE source = c_handle->source;

                    exist_other_stream = true;

                    /* stream is started */
                    /* set timestamp for debug */
                    c_handle->ccni_in_bt_count  = bt_count;
                    c_handle->ccni_in_gpt_count = gpt_count;
                    /* set blk index */
                    c_handle->ccni_blk_index    = blk_index;
                    /* increase fetch count */
                    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                    c_handle->fetch_count += 1;
                    hal_nvic_restore_interrupt_mask(saved_mask);
                    /* afe in config */
                    buffer_info = &source->streamBuffer.BufferInfo;
                    runtime = &source->param.audio;
                    /* update read & write pointer */
                    c_handle->pre_write_offset  = buffer_info->WriteOffset;
                    c_handle->pre_read_offset   = buffer_info->ReadOffset;
                    c_handle->afe_vul_cur       = AFE_GET_REG(c_handle->afe_vul_cur_addr);
                    c_handle->afe_vul_base      = AFE_GET_REG(c_handle->afe_vul_base_addr);
                    c_handle->cur_write_offset  = c_handle->afe_vul_cur - c_handle->afe_vul_base;
                    buffer_info->WriteOffset = c_handle->cur_write_offset;
                    /* check if the samples are enough */
                    buffer_per_channel_shift = ((source->param.audio.channel_num>=2) && (source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER )) ? 1 : 0;
                    data_size = (c_handle->cur_write_offset >= c_handle->pre_read_offset)
                                ? (c_handle->cur_write_offset - c_handle->pre_read_offset)>>buffer_per_channel_shift
                                : (source->streamBuffer.BufferInfo.length - c_handle->pre_read_offset + c_handle->cur_write_offset)>>buffer_per_channel_shift;
                    /* update afe buffer current size before process */
                    c_handle->afe_buffer_latency_size = data_size;
                    if (runtime->irq_exist == false) {
                        /* get the consuming time */
                        /* Jitter between BT CCNI IRQ and Audio HW, to avoid that data is less than a frame: 5ms * sample_rate * format */
                        uint32_t duration_cnt = 0;
                        uint32_t gpt_count_1 = 0;
                        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count_1);
                        hal_gpt_get_duration_count(gpt_count, gpt_count_1, &duration_cnt);
                        uint32_t jitter_size = ull_audio_v2_dongle_dl_calculate_afe_in_jitter_bt_audio(c_handle, duration_cnt);

                        /* stream is played till the samples in AFE buffer is enough */
                        if (data_size >= (source->param.audio.frame_size + jitter_size)) {
                            if ((source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
                                (source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) {
                                /* tracking mode should buffer some data to avoid irq period jitter */
                                #ifndef AIR_I2S_SLAVE_ENABLE
                                buffer_info->ReadOffset += ((data_size - (source->param.audio.frame_size + jitter_size)) << buffer_per_channel_shift);
                                buffer_info->ReadOffset = (source->streamBuffer.BufferInfo.ReadOffset % (source->streamBuffer.BufferInfo.length));
                                if (source->param.audio.format_bytes == 4) {
                                    buffer_info->ReadOffset = EIGHT_BYTE_ALIGNED(buffer_info->ReadOffset);
                                } else {
                                    buffer_info->ReadOffset = FOUR_BYTE_ALIGNED(buffer_info->ReadOffset);
                                }
                                c_handle->pre_read_offset = buffer_info->ReadOffset;
                                #endif
                            } else {
                                /* LINE IN only need the latest 5ms data */
                                buffer_info->ReadOffset += ((data_size - (source->param.audio.frame_size + jitter_size)) << buffer_per_channel_shift);
                                buffer_info->ReadOffset = (source->streamBuffer.BufferInfo.ReadOffset % (source->streamBuffer.BufferInfo.length));
                                if (source->param.audio.format_bytes == 4) {
                                    buffer_info->ReadOffset = EIGHT_BYTE_ALIGNED(buffer_info->ReadOffset);
                                } else {
                                    buffer_info->ReadOffset = FOUR_BYTE_ALIGNED(buffer_info->ReadOffset);
                                }
                                c_handle->pre_read_offset = buffer_info->ReadOffset;
                            }

                            /* samples are enough, so update data status */
                            c_handle->data_status = ULL_AUDIO_V2_DONGLE_DL_DATA_IN_STREAM;

                            /* update flag */
                            runtime->irq_exist = true;

                            ULL_V2_LOG_I("[ULL Audio V2][DL][AFE IN] scenario type %d, first handle afe buffer ro[%d] wo[%d] %d", 4,
                                source->scenario_type,
                                buffer_info->ReadOffset,
                                buffer_info->WriteOffset,
                                jitter_size
                                );
                            /* update this status to trigger sw mixer handle */
                            c_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_READY;

                            /* Handler the stream */
                            AudioCheckTransformHandle(source->transform);
                        }
                        if ((source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
                            (source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) {
                            if ((buffer_info->WriteOffset & (~15)) == (buffer_info->ReadOffset & (~15))) {
                                ULL_V2_LOG_E("[ULL Audio V2][DL][AFE IN] hwsrc start full owo[%d] oro[%d]", 2, buffer_info->WriteOffset, buffer_info->ReadOffset);
                            }
                        }
                        c_handle->afe_buffer_latency_size = (buffer_info->WriteOffset >= buffer_info->ReadOffset)
                                    ? (buffer_info->WriteOffset - buffer_info->ReadOffset)>>buffer_per_channel_shift
                                    : (source->streamBuffer.BufferInfo.length - buffer_info->ReadOffset + buffer_info->WriteOffset)>>buffer_per_channel_shift;
                    } else {
                        if (data_size >= source->param.audio.frame_size) {
                            /* samples are enough, so update data status */
                            c_handle->data_status = ULL_AUDIO_V2_DONGLE_DL_DATA_IN_STREAM;
                        } else {
                            ULL_V2_LOG_E("[ULL Audio V2][DL][AFE IN] scenario type %d, size %d < frame size %d ccni_wo[%d] cur_wo[%d]", 5,
                                source->scenario_type,
                                data_size,
                                source->param.audio.frame_size,
                                c_handle->cur_write_offset,
                                AFE_GET_REG(c_handle->afe_vul_cur_addr) - AFE_GET_REG(c_handle->afe_vul_base_addr)
                                );
                        }
                        /* Handler the stream */
                        AudioCheckTransformHandle(source->transform);
                    }
                    #ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
                    /* vdma tracking mode */
                    if ((source->param.audio.AfeBlkControl.u4asrcflag) && (!g_i2s_slv_tracking_start_flag)) {
                        g_i2s_slv_tracking_start_flag = true;
                        vdma_channel_t dma_channel;
                        vdma_status_t i2s_vdma_status;
                        uint32_t port_tmp = 0;
                        extern vdma_channel_t g_i2s_slave_vdma_channel_infra[];
                        if (source->param.audio.audio_interface == HAL_AUDIO_INTERFACE_1) {
                            port_tmp = 0;
                        } else if (source->param.audio.audio_interface == HAL_AUDIO_INTERFACE_2) {
                            port_tmp = 1;
                        } else if (source->param.audio.audio_interface == HAL_AUDIO_INTERFACE_3) {
                            port_tmp = 2;
                        } else {
                            ULL_V2_LOG_E("[ULL Audio V2][DL] start i2s slave audio_interface = %d fail", 1, source->param.audio.audio_interface);
                            AUDIO_ASSERT(0);
                        }
                        dma_channel = g_i2s_slave_vdma_channel_infra[port_tmp * 2 + 1];
                        buffer_info->ReadOffset = buffer_info->WriteOffset; // clear the buffer
                        AFE_WRITE(ASM_CH01_OBUF_RDPNT, AFE_GET_REG(ASM_CH01_OBUF_WRPNT));
                        i2s_vdma_status = vdma_start(dma_channel);
                        if (i2s_vdma_status != VDMA_OK) {
                            ULL_V2_LOG_E("[ULL Audio V2][DL] DSP UL start i2s slave set vdma_start fail %d", 1, i2s_vdma_status);
                            AUDIO_ASSERT(0);
                        }
                        ULL_V2_LOG_I("[ULL Audio V2][DL] VDMA Start", 0);
                    }
                    /* interconn tracking mode */
                    if ((source->param.audio.mem_handle.pure_agent_with_src) && (!g_i2s_slv_tracking_start_flag)) {
                        g_i2s_slv_tracking_start_flag = true;
                        /* start gpt timer to trigger vul1 irq */
                        hal_gpt_status_t gpt_status = HAL_GPT_STATUS_OK;
                        if (gpt_i2s_slv_trigger_handle == 0) {
                            gpt_status = hal_gpt_sw_get_timer(&gpt_i2s_slv_trigger_handle);
                            if (gpt_status != HAL_GPT_STATUS_OK) {
                                ULL_V2_LOG_E("[ULL Audio V2][DL][AFE IN] get gpt handle fail %d", 1, gpt_status);
                                AUDIO_ASSERT(0);
                            }
                        }
                        gpt_status = hal_gpt_sw_start_timer_us(gpt_i2s_slv_trigger_handle, 2500, (hal_gpt_callback_t)ull_audio_v2_dongle_dl_i2s_slv_gpt_cb, c_handle);
                    }
                    #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE */
                }
#endif
                    break;
                default:
                    AUDIO_ASSERT(0);
                    break;
            }
        }

        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }
    if (c_handle != NULL)
    {
        AUDIO_ASSERT(0);
    }

    if ((exist_other_stream == false) && (no_usb_data_1 == true) && (no_usb_data_2 == true) && (usb_saved_stream != NULL)) {
        c_handle = (ull_audio_v2_dongle_dl_handle_t *)usb_saved_stream->source->transform->sink->param.bt_common.scenario_param.dongle_handle;
        if (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_READY) {
            g_ull_audio_v2_dongle_dl_dummy_process_flag[c_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0] = true;
            /* Force to wake up the stream process task for dummy prcoess */
            usb_saved_stream->callback.Status = CALLBACK_HANDLER;
            xTaskResumeFromISR(usb_saved_stream->source->taskId);
            portYIELD_FROM_ISR(pdTRUE);
            g_ull_audio_v2_dongle_dl_dummy_process_cnt[c_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0]++;
            ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING]dl silence padding to sink buffer, accu %d", 1, g_ull_audio_v2_dongle_dl_dummy_process_cnt[c_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0]);
        }
    }

_ccni_return:
    return;
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_handler_usb_preload(uint32_t usb_port)
{
    SINK sink;
    //uint32_t saved_mask;
    uint32_t i, preloader_usb_frames;
    ull_audio_v2_dongle_dl_handle_t *c_handle;
    static uint32_t error_count = 0;

    /* check if there is any DL stream */
    if (ull_audio_v2_dongle_first_dl_handle == NULL)
    {
        if ((error_count%1000) == 0)
        {
            ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING]dl dongle_handle in handler1 is NULL, %d\r\n", 1, error_count);
        }
        error_count += 1;
        goto _handler_return;
    }
    else
    {
        error_count = 0;
    }

#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (ull_audio_v2_dl_without_bt_link_mode_enable == true)
    {
        /* bypss preload mode in without_bt_link_mode */
        goto _handler_return;
    }
#endif /* AIR_SILENCE_DETECTION_ENABLE */

    /* trigger all started DL stream one by one */
    c_handle = ull_audio_v2_dongle_first_dl_handle;
    for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++)
    {
        if ((c_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_START) || (c_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_RUNNING))
        {
            switch (c_handle->sub_id)
            {
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
                    /* get sink */
                    sink = (SINK)c_handle->owner;
                    if ((sink == NULL) || (sink->transform == NULL))
                    {
                        break;
                    }
                    if (usb_port != (c_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0))
                    {
                        /* Not the current USB Port trigger preload flow */
                        break;
                    }
                    //hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                    if (c_handle->source_info.usb_in.frame_max_num >= DL_PRELOADER_RESERVED_USB_FRAMES) {
                        preloader_usb_frames = c_handle->source_info.usb_in.frame_max_num - DL_PRELOADER_RESERVED_USB_FRAMES;
                    } else {
                        preloader_usb_frames = 0;
                    }
                    if ((c_handle->fetch_count == 0)
                        && (c_handle->stream_mode == ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_STANDBY)
                        && (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_READY)
                        && (c_handle->process_frames_total < preloader_usb_frames)
                        && (g_ull_audio_v2_dongle_dl_dummy_process_cnt[c_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0] == 0))
                    {
                        /* set preload flag */
                        c_handle->source_preload_count = 1;

                        /* Handler the stream */
                        AudioCheckTransformHandle(sink->transform);
                    }
                    //hal_nvic_restore_interrupt_mask(saved_mask);
                    break;

                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0:
                    break;

                default:
                    AUDIO_ASSERT(0);
                    break;
            }
        }

        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }
    if (c_handle != NULL)
    {
        AUDIO_ASSERT(0);
    }

_handler_return:
    return;
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_preload_handler1(hal_ccni_event_t event, void * msg)
{
    UNUSED(event);
    UNUSED(msg);

    ull_audio_v2_dongle_dl_handler_usb_preload(0);
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_preload_handler2(hal_ccni_event_t event, void * msg)
{
    UNUSED(event);
    UNUSED(msg);

    ull_audio_v2_dongle_dl_handler_usb_preload(1);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_dl_bt_info_init(ull_audio_v2_dongle_dl_handle_t *dongle_handle, uint32_t i, bt_common_open_param_p bt_common_open_param)
{
    uint32_t j, saved_mask;

    /* update bt link settings to global state machine */
    for (j = 0; j < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; j++)
    {
        if (hal_memview_cm4_to_dsp0((uint32_t)(bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.bt_link_info[i].share_info)) == (uint32_t)(ull_audio_v2_dl_bt_info[j].bt_link_info.share_info))
        {
            /* check bt link settings */
            if (memcmp(&(bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.bt_link_info[i].codec_type), &(ull_audio_v2_dl_bt_info[j].bt_link_info.codec_type), sizeof(ull_audio_v2_dongle_bt_link_info_t)-sizeof(void *)) != 0)
            {
                /* same share buffer, same codec settings */
                ULL_V2_LOG_W("[ULL Audio V2][DL][ERROR]codec setting is different\r\n", 0);
                //AUDIO_ASSERT(0);
            }
            /* in here, it means this bt link's setting has been used */
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            ull_audio_v2_dl_bt_info[j].user_count += 1;
            hal_nvic_restore_interrupt_mask(saved_mask);
            dongle_handle->sink_info.bt_out.bt_info[i] = &ull_audio_v2_dl_bt_info[j];
        }
    }
    if (dongle_handle->sink_info.bt_out.bt_info[i] == NULL)
    {
        /* in here, it means this bt link's setting has not been used */
        for (j = 0; j < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; j++)
        {
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            if (ull_audio_v2_dl_bt_info[j].bt_link_info.share_info == NULL)
            {
                if (ull_audio_v2_dl_bt_info[j].user_count != 0)
                {
                    ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]user count is error, %u\r\n", 1, ull_audio_v2_dl_bt_info[j].user_count);
                    AUDIO_ASSERT(0);
                }
                ull_audio_v2_dl_bt_info[j].user_count += 1;
                ull_audio_v2_dl_bt_info[j].seq_num = 0;
                ull_audio_v2_dl_bt_info[j].blk_index = 0;
                ull_audio_v2_dl_bt_info[j].blk_index_previous = 0;
                ull_audio_v2_dl_bt_info[j].crc32_init = (0xfffffff0 | i);
                memcpy(&(ull_audio_v2_dl_bt_info[j].bt_link_info), &(bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.bt_link_info[i]), sizeof(ull_audio_v2_dongle_bt_link_info_t));
                ull_audio_v2_dl_bt_info[j].bt_link_info.share_info = (void *)hal_memview_cm4_to_dsp0((uint32_t)(ull_audio_v2_dl_bt_info[j].bt_link_info.share_info));
                dongle_handle->sink_info.bt_out.bt_info[i] = &ull_audio_v2_dl_bt_info[j];
                hal_nvic_restore_interrupt_mask(saved_mask);
                break;
            }
            hal_nvic_restore_interrupt_mask(saved_mask);
        }
        if (dongle_handle->sink_info.bt_out.bt_info[i] == NULL)
        {
            /* not found suitable bt info channel */
            ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]all bt info are used\r\n", 0);
            AUDIO_ASSERT(0);
        }
    }
}

static void ull_audio_v2_dongle_dl_bt_out_init(ull_audio_v2_dongle_dl_handle_t *dongle_handle, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    uint32_t i;
    uint32_t frame_interval;
    uint32_t sample_rate;
    hal_audio_format_t sample_format;
    uint32_t frame_samples;
    uint32_t frame_size;
    uint32_t bit_rate;
    audio_dsp_codec_type_t codec_type;
    uint32_t channel_num = 0;
#if defined(AIR_CELT_ENC_V2_ENABLE)
    uint32_t version = 0;
#endif /* AIR_CELT_ENC_V2_ENABLE */
    UNUSED(audio_transmitter_open_param);

    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.bt_link_info[i].share_info == NULL)
        {
            continue;
        }
        else
        {
            /* update channel num */
            channel_num += 1;
            ull_audio_v2_dongle_dl_bt_info_init(dongle_handle, i, bt_common_open_param);
            if ((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
            {
                codec_type     = AUDIO_DSP_CODEC_TYPE_LC3PLUS;
                frame_interval = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.lc3plus.frame_interval;
                sample_rate    = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.lc3plus.sample_rate;
                sample_format  = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.lc3plus.sample_format;
                bit_rate       = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.lc3plus.bit_rate;
                frame_samples  = sample_rate/1000*frame_interval/1000;
                frame_size     = ull_audio_v2_codec_get_frame_size(&((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_type), &((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param));
            }
            else if ((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
            {
#if defined(AIR_CELT_ENC_V2_ENABLE)
                codec_type     = AUDIO_DSP_CODEC_TYPE_OPUS;
                frame_interval = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.opus.frame_interval;
                sample_rate    = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.opus.sample_rate;
                sample_format  = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.opus.sample_format;
                bit_rate       = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.opus.bit_rate;
                frame_samples  = sample_rate/1000*frame_interval/1000;
                frame_size     = ull_audio_v2_codec_get_frame_size(&((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_type), &((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param));
                if (version == 0)
                {
                    version = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.opus.version;
                    extern uint32_t stream_codec_celt_set_version(uint32_t version);
                    stream_codec_celt_set_version(version);
                    ULL_V2_LOG_E("[ULL Audio V2][DL]opus codec version = 0x%x\r\n", 1, version);
                }
                else if (version != ((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.opus.version))
                {
                    ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]codec version is not right, 0x%x, 0x%x\r\n", 2, version, (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.opus.version);
                    AUDIO_ASSERT(0);
                }
#endif /* AIR_CELT_ENC_V2_ENABLE */
            }
            else if ((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_type == AUDIO_DSP_CODEC_TYPE_ULD)
            {
                codec_type     = AUDIO_DSP_CODEC_TYPE_ULD;
                frame_interval = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.uld.frame_interval;
                sample_rate    = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.uld.sample_rate;
                sample_format  = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.uld.sample_format;
                bit_rate       = (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param.uld.bit_rate;
                frame_samples  = sample_rate/1000*frame_interval/1000;
                frame_size     = ull_audio_v2_codec_get_frame_size(&((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_type), &((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_param));
            }
            else
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]codec is not support, %u\r\n", 1, (dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.codec_type);
                AUDIO_ASSERT(0);
            }
            /* update codec type */
            if (dongle_handle->sink_info.bt_out.codec_type == 0)
            {
                dongle_handle->sink_info.bt_out.codec_type = codec_type;
            }
            else if (dongle_handle->sink_info.bt_out.codec_type != codec_type)
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]codec type is not right, %u, %u\r\n", 2, dongle_handle->sink_info.bt_out.codec_type, codec_type);
                AUDIO_ASSERT(0);
            }
            /* update frame interval */
            if (dongle_handle->sink_info.bt_out.frame_interval == 0)
            {
                dongle_handle->sink_info.bt_out.frame_interval = frame_interval;
            }
            else if (dongle_handle->sink_info.bt_out.frame_interval != frame_interval)
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]frame interval is not right, %u, %u\r\n", 2, dongle_handle->sink_info.bt_out.frame_interval, frame_interval);
                AUDIO_ASSERT(0);
            }
            /* update frame sample rate */
            if (dongle_handle->sink_info.bt_out.sample_rate == 0)
            {
                dongle_handle->sink_info.bt_out.sample_rate = sample_rate;
            }
            else if (dongle_handle->sink_info.bt_out.sample_rate != sample_rate)
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]sample rate is not right, %u, %u\r\n", 2, dongle_handle->sink_info.bt_out.sample_rate, sample_rate);
                AUDIO_ASSERT(0);
            }
            /* update frame sample format */
            if (dongle_handle->sink_info.bt_out.sample_format == 0)
            {
                dongle_handle->sink_info.bt_out.sample_format = sample_format;
            }
            else if (dongle_handle->sink_info.bt_out.sample_format != sample_format)
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]sample format is not right, %u, %u\r\n", 2, dongle_handle->sink_info.bt_out.sample_format, sample_format);
                AUDIO_ASSERT(0);
            }
            /* update frame samples */
            if (dongle_handle->sink_info.bt_out.frame_samples == 0)
            {
                dongle_handle->sink_info.bt_out.frame_samples = frame_samples;
            }
            else if (dongle_handle->sink_info.bt_out.frame_samples != frame_samples)
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]sample is not right, %u, %u\r\n", 2, dongle_handle->sink_info.bt_out.frame_samples, frame_samples);
                AUDIO_ASSERT(0);
            }
            /* update frame size */
            if (dongle_handle->sink_info.bt_out.frame_size == 0)
            {
                dongle_handle->sink_info.bt_out.frame_size = frame_size;
            }
            else if (dongle_handle->sink_info.bt_out.frame_size != frame_size)
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]frame size is not right, %u, %u\r\n", 2, dongle_handle->sink_info.bt_out.frame_size, frame_size);
                AUDIO_ASSERT(0);
            }
            /* update codec bit rate */
            if (dongle_handle->sink_info.bt_out.bit_rate == 0)
            {
                dongle_handle->sink_info.bt_out.bit_rate = bit_rate;
            }
            else if (dongle_handle->sink_info.bt_out.bit_rate != bit_rate)
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]bit rate is not right, %u, %u\r\n", 2, dongle_handle->sink_info.bt_out.bit_rate, bit_rate);
                AUDIO_ASSERT(0);
            }
        }
    }
    dongle_handle->sink_info.bt_out.channel_num = channel_num;
    ULL_V2_LOG_I("[ULL Audio V2][DL][scenario type %d-%d][handle 0x%x]bt out info, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                dongle_handle->source->scenario_type,
                dongle_handle->sink->scenario_type,
                dongle_handle,
                dongle_handle->sink_info.bt_out.channel_num,
                dongle_handle->sink_info.bt_out.sample_rate,
                dongle_handle->sink_info.bt_out.sample_format,
                dongle_handle->sink_info.bt_out.frame_size,
                dongle_handle->sink_info.bt_out.frame_samples,
                dongle_handle->sink_info.bt_out.frame_interval,
                dongle_handle->sink_info.bt_out.bit_rate);
    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->sink_info.bt_out.bt_info[i] != NULL)
        {
            n9_dsp_share_info_ptr p_share_info;
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.share_info);
            ULL_V2_LOG_I("[ULL Audio V2][DL][scenario type %d-%d][handle 0x%x]bt out channel %u info, %u, %u, %u, %u, 0x%x, 0x%x, 0x%x\r\n", 11,
                        dongle_handle->source->scenario_type,
                        dongle_handle->sink->scenario_type,
                        dongle_handle,
                        i+1,
                        (dongle_handle->sink_info.bt_out.bt_info[i])->seq_num,
                        (dongle_handle->sink_info.bt_out.bt_info[i])->user_count,
                        (dongle_handle->sink_info.bt_out.bt_info[i])->blk_index,
                        (dongle_handle->sink_info.bt_out.bt_info[i])->blk_index_previous,
                        (dongle_handle->sink_info.bt_out.bt_info[i])->crc32_init,
                        p_share_info,
                        hal_memview_infrasys_to_dsp0(p_share_info->start_addr));
        }
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_dl_bt_out_deinit(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    uint32_t i;
    uint32_t saved_mask;

    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->sink_info.bt_out.bt_info[i] == NULL)
        {
            continue;
        }
        else
        {
            if ((dongle_handle->sink_info.bt_out.bt_info[i])->user_count == 0)
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]user count is error, %u\r\n", 1, (dongle_handle->sink_info.bt_out.bt_info[i])->user_count);
                AUDIO_ASSERT(0);
            }
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            (dongle_handle->sink_info.bt_out.bt_info[i])->user_count -= 1;
            if ((dongle_handle->sink_info.bt_out.bt_info[i])->user_count == 0)
            {
                memset(dongle_handle->sink_info.bt_out.bt_info[i], 0, sizeof(ull_audio_v2_dongle_bt_info_t));
            }
            dongle_handle->sink_info.bt_out.bt_info[i] = NULL;
            hal_nvic_restore_interrupt_mask(saved_mask);
        }
    }
}

static void ull_audio_v2_dongle_dl_common_init(ull_audio_v2_dongle_dl_handle_t *dongle_handle, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    UNUSED(bt_common_open_param);
    UNUSED(audio_transmitter_open_param);
    /* codec init */
    if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0)
        {
            extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_0[];
            stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_0, CODEC_ENCODER_LC3PLUS, CONFIG_ENCODER);
        }
        else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)
        {
            extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_1[];
            stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_1, CODEC_ENCODER_LC3PLUS, CONFIG_ENCODER);
        }
        #ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
            else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) {
                extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_in_broadcast[];
                stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_line_in_broadcast, CODEC_ENCODER_LC3PLUS, CONFIG_ENCODER);
            }
        #endif
        #if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
            else if ((dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
             (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) {
                extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_i2s_in_broadcast[];
                stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_i2s_in_broadcast, CODEC_ENCODER_LC3PLUS, CONFIG_ENCODER);
            }
        #endif

        lc3plus_enc_config_t lc3plus_enc_config;
        lc3plus_enc_config.sample_bits       = 24;
        lc3plus_enc_config.channel_num       = dongle_handle->sink_info.bt_out.channel_num;
        lc3plus_enc_config.sample_rate       = dongle_handle->sink_info.bt_out.sample_rate;
        lc3plus_enc_config.bit_rate          = dongle_handle->sink_info.bt_out.bit_rate;
        lc3plus_enc_config.frame_interval    = dongle_handle->sink_info.bt_out.frame_interval;
        lc3plus_enc_config.frame_samples     = dongle_handle->sink_info.bt_out.frame_samples;
        lc3plus_enc_config.in_frame_size     = dongle_handle->sink_info.bt_out.frame_samples*sizeof(int32_t);
        lc3plus_enc_config.out_frame_size    = dongle_handle->sink_info.bt_out.frame_size;
        lc3plus_enc_config.process_frame_num = 1;
        lc3plus_enc_config.codec_mode        = LC3PLUS_ARCH_FX;
        lc3plus_enc_config.channel_mode      = LC3PLUS_ENC_CHANNEL_MODE_MULTI;
        stream_codec_encoder_lc3plus_init(LC3PLUS_ENC_PORT_0, dongle_handle->sink, &lc3plus_enc_config);
        ULL_V2_LOG_I("[ULL Audio V2][DL][scenario type %d-%d][handle 0x%x]lc3plus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                    dongle_handle->source->scenario_type,
                    dongle_handle->sink->scenario_type,
                    dongle_handle,
                    lc3plus_enc_config.sample_bits,
                    lc3plus_enc_config.channel_num,
                    lc3plus_enc_config.sample_rate,
                    lc3plus_enc_config.bit_rate,
                    lc3plus_enc_config.frame_interval,
                    lc3plus_enc_config.frame_samples,
                    lc3plus_enc_config.in_frame_size,
                    lc3plus_enc_config.out_frame_size,
                    lc3plus_enc_config.codec_mode);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_ENC_V2_ENABLE)
        if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0)
        {
            extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_0[];
            stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_0, CODEC_ENCODER_OPUS_V2, CONFIG_ENCODER);
        }
        else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)
        {
            extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_1[];
            stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_1, CODEC_ENCODER_OPUS_V2, CONFIG_ENCODER);
        }
        #ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
            else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) {
                extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_in_broadcast[];
                stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_line_in_broadcast, CODEC_ENCODER_OPUS_V2, CONFIG_ENCODER);
            }
        #endif
        #if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
            else if ((dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
             (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) {
                extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_i2s_in_broadcast[];
                stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_i2s_in_broadcast, CODEC_ENCODER_OPUS_V2, CONFIG_ENCODER);
            }
        #endif

        celt_enc_port_config_t celt_enc_config;
        celt_enc_config.sample_bits       = 16;
        celt_enc_config.channel_num       = dongle_handle->sink_info.bt_out.channel_num;
        celt_enc_config.sample_rate       = dongle_handle->sink_info.bt_out.sample_rate;
        celt_enc_config.bit_rate          = dongle_handle->sink_info.bt_out.bit_rate;
        celt_enc_config.process_frame_num = 2;
        celt_enc_config.frame_interval    = dongle_handle->sink_info.bt_out.frame_interval/celt_enc_config.process_frame_num;
        celt_enc_config.frame_samples     = dongle_handle->sink_info.bt_out.frame_samples/celt_enc_config.process_frame_num;
        celt_enc_config.in_frame_size     = dongle_handle->sink_info.bt_out.frame_samples/celt_enc_config.process_frame_num*sizeof(int16_t);
        celt_enc_config.out_frame_size    = dongle_handle->sink_info.bt_out.frame_size/celt_enc_config.process_frame_num;
        celt_enc_config.complexity        = 0;
        celt_enc_config.channel_mode      = CELT_ENC_CHANNEL_MODE_MULTI_MONO;
        stream_codec_encoder_celt_v2_init(CELT_ENC_PORT_0, dongle_handle->sink, &celt_enc_config);
        ULL_V2_LOG_I("[ULL Audio V2][DL][scenario %d-%d][handle 0x%x]opus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                    dongle_handle->source->scenario_type,
                    dongle_handle->sink->scenario_type,
                    dongle_handle,
                    celt_enc_config.sample_bits,
                    celt_enc_config.channel_num,
                    celt_enc_config.sample_rate,
                    celt_enc_config.bit_rate,
                    celt_enc_config.frame_interval,
                    celt_enc_config.frame_samples,
                    celt_enc_config.in_frame_size,
                    celt_enc_config.out_frame_size,
                    celt_enc_config.complexity);
#endif /* AIR_CELT_ENC_V2_ENABLE */
    }
#if defined(AIR_AUDIO_ULD_ENCODE_ENABLE)
    else if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_ULD) {
        uld_enc_port_config_t uld_enc_config;

        if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) {
            extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_0[];
            stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_0, CODEC_ENCODER_ULD, CONFIG_ENCODER);
        }
        else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1) {
            extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_1[];
            stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_1, CODEC_ENCODER_ULD, CONFIG_ENCODER);
        }
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) {
            extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_in_broadcast[];
            stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_line_in_broadcast, CODEC_ENCODER_ULD, CONFIG_ENCODER);
        }
#endif
#if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
        else if ((dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
         (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) {
            extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_i2s_in_broadcast[];
            stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_i2s_in_broadcast, CODEC_ENCODER_ULD, CONFIG_ENCODER);
        }
#endif

        if (g_ull_audio_v2_dl_uld_enc_cnt++ == 0) {
            g_ull_audio_v2_dl_uld_enc_port = stream_codec_encoder_uld_get_port(dongle_handle->sink);
            AUDIO_ASSERT(g_ull_audio_v2_dl_uld_enc_port != NULL);
        uld_enc_config.bit_rate = dongle_handle->sink_info.bt_out.bit_rate;
        uld_enc_config.interval = dongle_handle->sink_info.bt_out.frame_interval;
        uld_enc_config.in_channel_num = dongle_handle->sink_info.bt_out.channel_num;
        if (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
            uld_enc_config.resolution = RESOLUTION_16BIT;
        } else {
            uld_enc_config.resolution = RESOLUTION_32BIT;
        }
        uld_enc_config.sample_rate = dongle_handle->sink_info.bt_out.sample_rate;
            stream_codec_encoder_uld_init(g_ull_audio_v2_dl_uld_enc_port, &uld_enc_config);
        ULL_V2_LOG_I("[ULL Audio V2][DL][scenario type %d-%d][handle 0x%x]uld info, %d, %d, %d, %d, %d\r\n", 8,
                    dongle_handle->source->scenario_type,
                    dongle_handle->sink->scenario_type,
                    dongle_handle,
                    uld_enc_config.bit_rate,
                    uld_enc_config.interval,
                    uld_enc_config.in_channel_num,
                    uld_enc_config.resolution,
                    uld_enc_config.sample_rate);
    }
        dongle_handle->uld_port = g_ull_audio_v2_dl_uld_enc_port;
    }
#endif /* AIR_AUDIO_ULD_ENCODE_ENABLE */
}

static void ull_audio_v2_dongle_dl_common_deinit(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    /* codec deinit */
    if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        stream_codec_encoder_lc3plus_deinit(LC3PLUS_ENC_PORT_0, dongle_handle->sink);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_ENC_V2_ENABLE)
        stream_codec_encoder_celt_v2_deinit(CELT_ENC_PORT_0, dongle_handle->sink);
#endif /* AIR_CELT_ENC_V2_ENABLE */
    }
#if defined(AIR_AUDIO_ULD_ENCODE_ENABLE)
    else if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_ULD) {
        if (--g_ull_audio_v2_dl_uld_enc_cnt == 0) {
            stream_codec_encoder_uld_deinit(g_ull_audio_v2_dl_uld_enc_port);
            g_ull_audio_v2_dl_uld_enc_port = NULL;
        }
        dongle_handle->uld_port = NULL;
    }
#endif /* AIR_AUDIO_ULD_ENCODE_ENABLE */
}

static void ull_audio_v2_dongle_dl_usb_in_init(ull_audio_v2_dongle_dl_handle_t *dongle_handle, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    uint32_t i = 0;
    n9_dsp_share_info_ptr p_share_info;
    uint32_t sample_size = 0;
    stream_resolution_t stream_resolution = RESOLUTION_16BIT;

    /* bt out info init */
    ull_audio_v2_dongle_dl_bt_out_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
    ULL_V2_LOG_I("[ULL Audio V2][DL][scenario %d-%d][handle 0x%x]bt out info, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                audio_transmitter_open_param->scenario_type,
                audio_transmitter_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->sink_info.bt_out.channel_num,
                dongle_handle->sink_info.bt_out.sample_rate,
                dongle_handle->sink_info.bt_out.sample_format,
                dongle_handle->sink_info.bt_out.frame_size,
                dongle_handle->sink_info.bt_out.frame_samples,
                dongle_handle->sink_info.bt_out.frame_interval,
                dongle_handle->sink_info.bt_out.bit_rate);
    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->sink_info.bt_out.bt_info[i] != NULL)
        {
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.share_info);
            ULL_V2_LOG_I("[ULL Audio V2][DL][scenario %d-%d][handle 0x%x]bt out channel %u info, %u, %u, %u, %u, 0x%x, 0x%x, 0x%x\r\n", 11,
                        audio_transmitter_open_param->scenario_type,
                        audio_transmitter_open_param->scenario_sub_id,
                        dongle_handle,
                        i+1,
                        (dongle_handle->sink_info.bt_out.bt_info[i])->seq_num,
                        (dongle_handle->sink_info.bt_out.bt_info[i])->user_count,
                        (dongle_handle->sink_info.bt_out.bt_info[i])->blk_index,
                        (dongle_handle->sink_info.bt_out.bt_info[i])->blk_index_previous,
                        (dongle_handle->sink_info.bt_out.bt_info[i])->crc32_init,
                        p_share_info,
                        hal_memview_infrasys_to_dsp0(p_share_info->start_addr));
        }
    }

    if (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)
    {
        sample_size = sizeof(int32_t);
        stream_resolution = RESOLUTION_32BIT;
    }
    else if (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
    {
        sample_size = sizeof(int16_t);
        stream_resolution = RESOLUTION_16BIT;
    }
    else
    {
        ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]sample_format is not supported, %u\r\n", 1, dongle_handle->sink_info.bt_out.sample_format);
        AUDIO_ASSERT(0);
    }
    dongle_handle->process_sample_rate_max = MAX(dongle_handle->sink_info.bt_out.sample_rate, dongle_handle->source_info.usb_in.sample_rate);
    /* usb in info init */
    dongle_handle->source_info.usb_in.channel_num       = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.channel_mode;
    dongle_handle->source_info.usb_in.sample_rate       = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.sample_rate;
    dongle_handle->source_info.usb_in.sample_format     = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.format;
    dongle_handle->source_info.usb_in.frame_size        = usb_audio_get_frame_size(&audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_type, &audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param);
    dongle_handle->source_info.usb_in.frame_samples     = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.sample_rate/audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.frame_interval;
    dongle_handle->source_info.usb_in.frame_interval    = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.frame_interval;
    dongle_handle->source_info.usb_in.frame_max_num     = (dongle_handle->sink_info.bt_out.frame_interval+500) / 1000 + 1;
    ULL_V2_LOG_I("[ULL Audio V2][DL][scenario %d-%d][handle 0x%x]usb in info, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                audio_transmitter_open_param->scenario_type,
                audio_transmitter_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->source_info.usb_in.channel_num,
                dongle_handle->source_info.usb_in.sample_rate,
                dongle_handle->source_info.usb_in.sample_format,
                dongle_handle->source_info.usb_in.frame_size,
                dongle_handle->source_info.usb_in.frame_samples,
                dongle_handle->source_info.usb_in.frame_interval,
                dongle_handle->source_info.usb_in.frame_max_num);

    // /* sw src init */
    // src_fixed_ratio_config_t sw_src_config;
    dongle_handle->src_in_frame_samples     = dongle_handle->source_info.usb_in.frame_samples;
    dongle_handle->src_in_frame_size        = dongle_handle->src_in_frame_samples * sample_size;
    dongle_handle->src_out_frame_samples    = dongle_handle->sink_info.bt_out.sample_rate/1000*dongle_handle->source_info.usb_in.frame_interval/1000;
    dongle_handle->src_out_frame_size       = dongle_handle->src_out_frame_samples * sample_size;
    /* unified fs convertor */
    audio_dl_unified_fs_convertor_param_t fs_param = {0};
    // get param from dongle
    fs_param.in_rate    = dongle_handle->source_info.usb_in.sample_rate;
    fs_param.out_rate   = dongle_handle->sink_info.bt_out.sample_rate;
    fs_param.period_ms  = dongle_handle->sink_info.bt_out.frame_interval;
    fs_param.in_ch_num  = dongle_handle->source_info.usb_in.channel_num;
    fs_param.pcm_format = dongle_handle->sink_info.bt_out.sample_format;
    fs_param.source     = dongle_handle->source;
    fs_param.codec_type = dongle_handle->sink_info.bt_out.codec_type;
    fs_param.process_max_size = (dongle_handle->source_info.usb_in.frame_max_num + 1) * fs_param.in_rate * sample_size / 1000;
    audio_dl_unified_fs_convertor_init(&fs_param);
    // set param to dongle
    dongle_handle->process_sample_rate_max = fs_param.process_sample_rate_max;
    dongle_handle->src0_port = (src_fixed_ratio_port_t *) fs_param.src0_port;
    dongle_handle->src1_port = (src_fixed_ratio_port_t *) fs_param.src1_port;
    dongle_handle->src_port  = fs_param.src2_port;

    /* sw gain init */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution               = stream_resolution;
    default_config.target_gain              = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    default_config.up_step                  = 1;
    default_config.up_samples_per_step      = 2;
    default_config.down_step                = -1;
    default_config.down_samples_per_step    = 2;
    dongle_handle->gain_port                = stream_function_sw_gain_get_port(dongle_handle->source);
    stream_function_sw_gain_init(dongle_handle->gain_port, dongle_handle->source_info.usb_in.channel_num, &default_config);
    for (i = 0; i < dongle_handle->source_info.usb_in.channel_num/2; i++)
    {
        default_gain = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
        stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2*i+1, default_gain);
        default_gain = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R;
        stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2*i+2, default_gain);
    }
    ULL_V2_LOG_I("[ULL Audio V2][DL][scenario %d-%d][handle 0x%x]sw gain 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                audio_transmitter_open_param->scenario_type,
                audio_transmitter_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->gain_port,
                default_config.resolution,
                default_config.target_gain,
                default_config.up_step,
                default_config.up_samples_per_step,
                default_config.down_step,
                default_config.down_samples_per_step,
                audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L,
                audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R);

    /* sw buffer init */
    sw_buffer_config_t buffer_config;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    buffer_config.mode                  = (bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.without_bt_link_mode_enable == 0) ? SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH : SW_BUFFER_MODE_DROP_DATA_WHEN_BUFFER_FULL;
#else
    buffer_config.mode                  = SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH;
#endif /* AIR_SILENCE_DETECTION_ENABLE */
    buffer_config.total_channels        = dongle_handle->source_info.usb_in.channel_num;
    buffer_config.watermark_max_size    = 8*dongle_handle->src_out_frame_samples*dongle_handle->sink_info.bt_out.frame_interval/dongle_handle->source_info.usb_in.frame_interval*sample_size;
    buffer_config.watermark_min_size    = 0;
    buffer_config.output_size           = dongle_handle->src_out_frame_samples*dongle_handle->sink_info.bt_out.frame_interval/dongle_handle->source_info.usb_in.frame_interval*sample_size;
    dongle_handle->buffer_port_0        = stream_function_sw_buffer_get_unused_port(dongle_handle->source);
    stream_function_sw_buffer_init(dongle_handle->buffer_port_0, &buffer_config);
    dongle_handle->buffer_default_output_size = dongle_handle->src_out_frame_samples*dongle_handle->sink_info.bt_out.frame_interval/dongle_handle->source_info.usb_in.frame_interval*sample_size;
    dongle_handle->buffer0_output_size = dongle_handle->buffer_default_output_size;
    ULL_V2_LOG_I("[ULL Audio V2][DL][scenario %d-%d][handle 0x%x]sw buffer 0x%x info, %d, %d, %d, %d, %d, %d, %d\r\n", 11,
                audio_transmitter_open_param->scenario_type,
                audio_transmitter_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->buffer_port_0,
                buffer_config.mode,
                buffer_config.total_channels,
                buffer_config.watermark_max_size,
                buffer_config.watermark_min_size,
                buffer_config.output_size,
                dongle_handle->buffer_default_output_size,
                dongle_handle->buffer0_output_size);

    /* sw clk skew init */
    sw_clk_skew_config_t sw_clk_skew_config;
    dongle_handle->clk_skew_port                = stream_function_sw_clk_skew_get_port(dongle_handle->source);
    sw_clk_skew_config.channel                  = dongle_handle->source_info.usb_in.channel_num;
    if (stream_resolution == RESOLUTION_16BIT)
    {
        sw_clk_skew_config.bits = 16;
    }
    else
    {
        sw_clk_skew_config.bits = 32;
    }
    sw_clk_skew_config.order                    = C_Flp_Ord_1;
    sw_clk_skew_config.skew_io_mode             = C_Skew_Inp;
    sw_clk_skew_config.skew_compensation_mode   = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    sw_clk_skew_config.skew_work_mode           = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size          = (dongle_handle->src_out_frame_samples*dongle_handle->sink_info.bt_out.frame_interval/dongle_handle->source_info.usb_in.frame_interval+1)*sample_size;
    sw_clk_skew_config.continuous_frame_size    = dongle_handle->src_out_frame_samples*dongle_handle->sink_info.bt_out.frame_interval/dongle_handle->source_info.usb_in.frame_interval*sample_size;
    stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
    dongle_handle->compen_method                = ULL_AUDIO_V2_COMPENSATORY_METHOD_SW_CLK_SKEW;
    dongle_handle->clk_skew_watermark_samples   = dongle_handle->src_out_frame_samples*1000/dongle_handle->source_info.usb_in.frame_interval;
    dongle_handle->clk_skew_compensation_mode   = sw_clk_skew_config.skew_compensation_mode;
    ULL_V2_LOG_I("[ULL Audio V2][DL][scenario %d-%d][handle 0x%x]sw clk skew 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 14,
                audio_transmitter_open_param->scenario_type,
                audio_transmitter_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->clk_skew_port,
                sw_clk_skew_config.channel,
                sw_clk_skew_config.bits,
                sw_clk_skew_config.order,
                sw_clk_skew_config.skew_io_mode,
                sw_clk_skew_config.skew_compensation_mode,
                sw_clk_skew_config.skew_work_mode,
                sw_clk_skew_config.max_output_size,
                sw_clk_skew_config.continuous_frame_size,
                dongle_handle->compen_method,
                dongle_handle->clk_skew_watermark_samples);

    /* sw mixer init */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback     = ull_audio_v2_dongle_dl_mixer_precallback;
    callback_config.postprocess_callback    = ull_audio_v2_dongle_dl_mixer_postcallback;
    in_ch_config.total_channel_number       = 2;
    in_ch_config.resolution                 = stream_resolution;
    in_ch_config.input_mode                 = SW_MIXER_CHANNEL_MODE_FORCED_ENOUGH;
    in_ch_config.buffer_size                = dongle_handle->sink_info.bt_out.sample_rate/1000*dongle_handle->sink_info.bt_out.frame_interval/1000*sample_size;
    out_ch_config.total_channel_number      = 2;
    out_ch_config.resolution                = stream_resolution;
    dongle_handle->mixer_member             = stream_function_sw_mixer_member_create((void *)dongle_handle->source, SW_MIXER_MEMBER_MODE_NO_BYPASS, &callback_config, &in_ch_config, &out_ch_config);
    ull_audio_v2_dongle_dl_sw_mixer_connect(dongle_handle);
    ULL_V2_LOG_I("[ULL Audio V2][DL][scenario type %d-%d][handle 0x%x]sw mixer 0x%x info, %d, %d, %d, %d, %d, %d, %d\r\n", 11,
                dongle_handle->source->scenario_type,
                dongle_handle->sink->scenario_type,
                dongle_handle,
                dongle_handle->mixer_member,
                in_ch_config.total_channel_number,
                in_ch_config.resolution,
                in_ch_config.input_mode,
                in_ch_config.buffer_size,
                out_ch_config.total_channel_number,
                out_ch_config.resolution,
                dongle_handle->mixer_status);

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
    if (audio_transmitter_open_param->scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) {
        ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[0] = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
        ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[1] = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R;
        ull_audio_v2_dongle_vol_balance_handle.chat_dongle_handle = dongle_handle;
    }
    else
    {
        ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0] = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
        ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1] = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R;
        ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
        ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R;
        ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port = dongle_handle->gain_port;
    }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

    g_ull_audio_v2_dongle_dl_dummy_process_flag[dongle_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0] = false;
    g_ull_audio_v2_dongle_dl_dummy_process_cnt[dongle_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0] = 0;
}

static void ull_audio_v2_dongle_dl_usb_in_deinit(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    /* bt out info deinit */
    ull_audio_v2_dongle_dl_bt_out_deinit(dongle_handle);

    /* unified fs convertor */
    audio_dl_unified_fs_convertor_param_t fs_param = {0};
    fs_param.src0_port  = dongle_handle->src0_port;
    fs_param.src1_port  = dongle_handle->src1_port;
    fs_param.src2_port  = dongle_handle->src_port;
    fs_param.pcm_format = dongle_handle->sink_info.bt_out.sample_format;
    audio_dl_unified_fs_convertor_deinit(&fs_param);

    /* sw gain deinit */
    stream_function_sw_gain_deinit(dongle_handle->gain_port);

    /* sw buffer deinit */
    stream_function_sw_buffer_deinit(dongle_handle->buffer_port_0);

    /* sw clk skew deinit */
    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);

    /* sw mixer deinit */
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
    ull_audio_v2_dongle_dl_sw_mixer_disconnect(dongle_handle);

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
    if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) {
        DTM_enqueue(DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_DISABLE, AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0, false);
        ull_audio_v2_dongle_vol_balance_handle.chat_dongle_handle = NULL;
    }
    else {
        ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port = NULL;
    }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
}

ATTR_TEXT_IN_IRAM static bool ull_audio_v2_dongle_dl_usb_get_actual_avail_size(SOURCE source, uint32_t *avail_size)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t unprocess_frames;

    /* get actual data size include header size in the share buffer */
    audio_transmitter_share_information_fetch(source, NULL);
    if (source->streamBuffer.ShareBufferInfo.read_offset < source->streamBuffer.ShareBufferInfo.write_offset)
    {
        /* normal case */
        *avail_size = source->streamBuffer.ShareBufferInfo.write_offset - source->streamBuffer.ShareBufferInfo.read_offset;
    }
    else if (source->streamBuffer.ShareBufferInfo.read_offset == source->streamBuffer.ShareBufferInfo.write_offset)
    {
        if(source->streamBuffer.ShareBufferInfo.bBufferIsFull == true)
        {
            /* buffer is full, so read_offset == write_offset */
            *avail_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;
        }
        else
        {
            /* buffer is empty, so read_offset == write_offset */
            *avail_size = 0;
            // ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING][handle 0x%x]data is 0\r\n", 1, dongle_handle);
        }
    }
    else
    {
        /* buffer wrapper case */
        *avail_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num
                    - source->streamBuffer.ShareBufferInfo.read_offset
                    + source->streamBuffer.ShareBufferInfo.write_offset;
    }

    /* get actual data size exclude header size in the share buffer */
    if (*avail_size != 0)
    {
        /* get unprocess frame number */
        unprocess_frames = *avail_size/source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
        /* if this is the first time, check if the data in usb buffer is larger than starting standard */
        if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY)
        {
            if (unprocess_frames < dongle_handle->source_info.usb_in.frame_max_num)
            {
                /* the data is not enough, so change avail_size to zero */
                *avail_size = 0;
            }
            else
            {
                /* change avail_size to actual data size in the buffer */
                *avail_size = *avail_size - unprocess_frames*sizeof(audio_transmitter_frame_header_t);
                *avail_size = *avail_size / dongle_handle->source_info.usb_in.channel_num;
            }
        }
        else
        {
            /* change avail_size to actual data size in the buffer */
            *avail_size = *avail_size - unprocess_frames*sizeof(audio_transmitter_frame_header_t);
            *avail_size = *avail_size / dongle_handle->source_info.usb_in.channel_num;
        }
    }

#if ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_V2_LOG_I("[ULL Audio V2][DL][DEBUG][handle 0x%x][get_actual_avail_size]:%u, %u, %d, %u, %d, %d", 7, dongle_handle, dongle_handle->data_status, source->streamBuffer.ShareBufferInfo.read_offset, source->streamBuffer.ShareBufferInfo.write_offset, *avail_size, current_timestamp, hal_nvic_query_exception_number());
#endif /* ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_IRAM static int32_t ull_audio_v2_dongle_dl_usb_clock_skew_check(ull_audio_v2_dongle_dl_handle_t *dongle_handle, uint32_t input_samples, uint32_t *buffer0_output_size)
{
    int32_t compensatory_samples = 0;
    int32_t remain_samples;
    int32_t remain_samples_0;
    int32_t output_samples = dongle_handle->sink_info.bt_out.frame_samples;
    uint32_t buffer_output_size = dongle_handle->buffer_default_output_size;
    int32_t frac_rpt;
    uint32_t sample_size = 0;

    if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_READY)
    {
        /* get remain samples */
        sample_size = (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) ? sizeof(int16_t) : sizeof(int32_t);
        remain_samples_0 = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port_0, 1) / sample_size;

        if ((dongle_handle->clk_skew_count > -DL_CLOCK_SKEW_CHECK_COUNT) && (dongle_handle->clk_skew_count < DL_CLOCK_SKEW_CHECK_COUNT))
        {
            /* in here, clock skew is not started */
            remain_samples = remain_samples_0+input_samples-output_samples;
            if (remain_samples < 0)
            {
                /* reset state machine */
                compensatory_samples = 0;
                dongle_handle->clk_skew_count = 0;
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x] samples are not enough, %d, %d, %d\r\n", 4, dongle_handle, remain_samples_0, input_samples, output_samples);
            }
            else if (remain_samples < dongle_handle->clk_skew_watermark_samples)
            {
                /* usb clock is slower than bt clock */
                dongle_handle->clk_skew_count += 1;
                if (dongle_handle->clk_skew_count == DL_CLOCK_SKEW_CHECK_COUNT)
                {
                    /* do nothing */
                }
            }
            else if (remain_samples > dongle_handle->clk_skew_watermark_samples)
            {
                /* usb clock is faster than bt clock */
                dongle_handle->clk_skew_count -= 1;
                if (dongle_handle->clk_skew_count == -DL_CLOCK_SKEW_CHECK_COUNT)
                {
                    /* do nothing */
                }
            }
            else
            {
                /* usb clock is as the same as bt clock */
                if (dongle_handle->clk_skew_count > 0)
                {
                    dongle_handle->clk_skew_count -= 1;
                }
                else if (dongle_handle->clk_skew_count < 0)
                {
                    dongle_handle->clk_skew_count += 1;
                }
            }
            /* do not compensatory */
            compensatory_samples = 0;
        }
        else
        {
            /* in here, clock skew is running */
            remain_samples = remain_samples_0+input_samples-output_samples;
            if (remain_samples < 0)
            {
                /* reset state machine */
                compensatory_samples = 0;
                dongle_handle->clk_skew_count = 0;
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x] samples are not enough, %d, %d, %d\r\n", 4, dongle_handle, remain_samples_0, input_samples, output_samples);
            }
            else if (dongle_handle->clk_skew_count == DL_CLOCK_SKEW_CHECK_COUNT)
            {
                if (remain_samples >= dongle_handle->clk_skew_watermark_samples)
                {
                    /* watermark is ok and stop compensatory */
                    compensatory_samples = 0;
                    dongle_handle->clk_skew_count = 0;
                }
                else
                {
                    /* do +1 sample compensatory */
                    compensatory_samples = 1;
                    stream_function_sw_clk_skew_get_frac_rpt(dongle_handle->clk_skew_port, 1, &frac_rpt);
                    if (frac_rpt == (dongle_handle->clk_skew_compensation_mode - 1))
                    {
                        buffer_output_size = dongle_handle->buffer_default_output_size - sample_size;
                    }
                }
            }
            else if (dongle_handle->clk_skew_count == -DL_CLOCK_SKEW_CHECK_COUNT)
            {
                if (remain_samples <= dongle_handle->clk_skew_watermark_samples)
                {
                    /* watermark is ok and stop compensatory */
                    compensatory_samples = 0;
                    dongle_handle->clk_skew_count = 0;
                }
                else
                {
                    /* do -1 sample compensatory */
                    compensatory_samples = -1;
                    stream_function_sw_clk_skew_get_frac_rpt(dongle_handle->clk_skew_port, 1, &frac_rpt);
                    if (frac_rpt == -(dongle_handle->clk_skew_compensation_mode - 1))
                    {
                        buffer_output_size = dongle_handle->buffer_default_output_size + sample_size;
                    }
                }
            }
            else
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x] error clk skew count, %d\r\n", 2, dongle_handle, dongle_handle->clk_skew_count);
                AUDIO_ASSERT(0);
            }
        }
    }

    *buffer0_output_size = buffer_output_size;

    return compensatory_samples;
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_usb_data_copy(ull_audio_v2_dongle_dl_handle_t *dongle_handle, DSP_STREAMING_PARA_PTR stream, uint8_t *src_buf, uint32_t samples, uint32_t samples_offset)
{
    switch (dongle_handle->source_info.usb_in.channel_num)
    {
        case 2:
            if ((dongle_handle->source_info.usb_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) && (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE))
            {
                ShareBufferCopy_I_16bit_to_D_32bit_2ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        samples);
            }
            else if ((dongle_handle->source_info.usb_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) && (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE))
            {
                ShareBufferCopy_I_16bit_to_D_16bit_2ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        samples);
            }
            else if ((dongle_handle->source_info.usb_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) && (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE))
            {
                ShareBufferCopy_I_24bit_to_D_32bit_2ch((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        samples);
            }
            else if ((dongle_handle->source_info.usb_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) && (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE))
            {
                ShareBufferCopy_I_24bit_to_D_16bit_2ch((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        samples);
            }
            else
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x]sample format is not supported, %u, %u\r\n", 2, dongle_handle, dongle_handle->source_info.usb_in.sample_format);
                AUDIO_ASSERT(0);
            }
            break;

        case 8:
            if ((dongle_handle->source_info.usb_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) && (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE))
            {
                ShareBufferCopy_I_16bit_to_D_32bit_8ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[2])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[3])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[4])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[5])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[6])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[7])+samples_offset,
                                                        samples);
            }
            else if ((dongle_handle->source_info.usb_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) && (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE))
            {
                ShareBufferCopy_I_16bit_to_D_16bit_8ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[2])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[3])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[4])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[5])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[6])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[7])+samples_offset,
                                                        samples);
            }
            else if ((dongle_handle->source_info.usb_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) && (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE))
            {
                ShareBufferCopy_I_24bit_to_D_32bit_8ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[2])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[3])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[4])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[5])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[6])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[7])+samples_offset,
                                                        samples);
            }
            else if ((dongle_handle->source_info.usb_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) && (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE))
            {
                ShareBufferCopy_I_24bit_to_D_16bit_8ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[2])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[3])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[4])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[5])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[6])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[7])+samples_offset,
                                                        samples);
            }
            else
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x]sample format is not supported, %u, %u\r\n", 2, dongle_handle, dongle_handle->source_info.usb_in.sample_format);
                AUDIO_ASSERT(0);
            }
            break;

        default:
            ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x]channel num is not supported, %u, %u\r\n", 2, dongle_handle, dongle_handle->source_info.usb_in.channel_num);
            AUDIO_ASSERT(0);
    }
}

static ATTR_TEXT_IN_IRAM void ull_audio_v2_dongle_dl_sink_internal_copy_payload(SINK sink, uint8_t *src_buf, bool is_dummy_copy)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(sink->transform->source, sink);
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t i, j;
    uint32_t *src_addr;
    uint32_t *des_addr;
    uint32_t blk_size;
    uint32_t blk_num;
    uint32_t blk_index;
    n9_dsp_share_info_ptr p_share_info;
    ULL_AUDIO_V2_HEADER *p_ull_audio_header;
    bool crc32_flag = false;
    uint32_t crc32_init[ULL_AUDIO_V2_DATA_CHANNEL_NUMBER];

    /* get current bt clock */
    MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);

    /* write DL packet to different share buffer one by one */
    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->sink_info.bt_out.bt_info[i] == NULL)
        {
            continue;
        }
        else
        {
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.share_info);
            blk_size     = p_share_info->sub_info.block_info.block_size;
            blk_num      = p_share_info->sub_info.block_info.block_num;
            blk_index    = dongle_handle->ccni_blk_index;
            crc32_init[i]= p_share_info->asi_cur;
            if (blk_index < blk_num)
            {
                /* get header address and data address */
                des_addr = (uint32_t *)(hal_memview_infrasys_to_dsp0(p_share_info->start_addr) + blk_size * blk_index);
                p_ull_audio_header = (ULL_AUDIO_V2_HEADER *)des_addr;
                des_addr = (uint32_t *)((uint32_t)des_addr + sizeof(ULL_AUDIO_V2_HEADER));

                /* check if blk size is enough */
                if ((dongle_handle->sink_info.bt_out.frame_size+sizeof(ULL_AUDIO_V2_HEADER)) > blk_size)
                {
                    ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x]blk size is not right, %d, %d\r\n", 3, dongle_handle, blk_size, dongle_handle->sink_info.bt_out.frame_size+sizeof(ULL_AUDIO_V2_HEADER));
                    AUDIO_ASSERT(0);
                }

                /* write DL packet data into share buffer block */
                if (is_dummy_copy == true) {
                    memset(des_addr, 0, dongle_handle->sink_info.bt_out.frame_size);
                } else {
                    src_addr     = stream_function_get_inout_buffer((void *)(&(stream->callback.EntryPara)), (i+1));
                    src_buf      = (uint8_t *)src_addr;
                    for (j = 0; j < (((uint32_t)(dongle_handle->sink_info.bt_out.frame_size+3))/4); j++) {
                        /* share buffer block must be 4B aligned, so we can use word access to get better performance */
                        *des_addr++ = *src_addr++;
                    }
                }

                /* write DL packet header into share buffer block */
                p_ull_audio_header->DataOffset = sizeof(ULL_AUDIO_V2_HEADER);
                p_ull_audio_header->PduLen     = dongle_handle->sink_info.bt_out.frame_size;
                p_ull_audio_header->DataLen    = p_ull_audio_header->PduLen;
                p_ull_audio_header->TimeStamp  = bt_clk;
                p_ull_audio_header->SampleSeq  = (dongle_handle->sink_info.bt_out.bt_info[i])->seq_num;
                /* check if crc32_value is changed before do CRC generate */
                if ((dongle_handle->sink_info.bt_out.bt_info[i])->crc32_init != crc32_init[i])
                {
                    crc32_flag = true;
                }
                if (is_dummy_copy == true) {
                    p_ull_audio_header->crc32_value = ull_audio_v2_dongle_crc32_generate_dummy(dongle_handle->sink_info.bt_out.frame_size, crc32_init[i]);
                } else {
                    p_ull_audio_header->crc32_value = ull_audio_v2_dongle_crc32_generate((uint8_t *)src_buf, dongle_handle->sink_info.bt_out.frame_size, crc32_init[i]);
                }

                /* update state machine */
                (dongle_handle->sink_info.bt_out.bt_info[i])->seq_num += 1;
                (dongle_handle->sink_info.bt_out.bt_info[i])->blk_index_previous = (dongle_handle->sink_info.bt_out.bt_info[i])->blk_index;
                (dongle_handle->sink_info.bt_out.bt_info[i])->blk_index = blk_index;
            }
            else
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x]channel%d blk index is not right, %d, %d\r\n", 4, dongle_handle, i+1, blk_index, blk_num);
            }
        }
    }

    /* get current bt clock */
    MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);

    /* Output crc32_init value changed log for debug */
    if (crc32_flag)
    {
        for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
        {
            if (dongle_handle->sink_info.bt_out.bt_info[i] != NULL)
            {
                ULL_V2_LOG_W("[ULL Audio V2][DL][handle 0x%x]bt out channel %u crc32_init value is changed from 0x%x to 0x%x\r\n", 11,
                            dongle_handle,
                            i+1,
                            (dongle_handle->sink_info.bt_out.bt_info[i])->crc32_init,
                            crc32_init[i]);
                (dongle_handle->sink_info.bt_out.bt_info[i])->crc32_init = crc32_init[i];
            }
        }
    }

    /* update time stamp */
    dongle_handle->data_out_bt_count = bt_clk;
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_sink_internal_update_write_offset(SINK sink, U32 amount, uint32_t *new_write_offset)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t i, write_offset, saved_mask;
    n9_dsp_share_info_ptr p_share_info;
    uint32_t blk_size;
    uint32_t blk_num;
    uint32_t blk_index;
    UNUSED(amount);
    UNUSED(new_write_offset);

    /* update write index */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    StreamDSP_HWSemaphoreTake();
    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->sink_info.bt_out.bt_info[i] == NULL)
        {
            continue;
        }
        else
        {
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.share_info);
            blk_size     = p_share_info->sub_info.block_info.block_size;
            blk_num      = p_share_info->sub_info.block_info.block_num;
            blk_index    = dongle_handle->ccni_blk_index;
            if (blk_index < blk_num)
            {
                write_offset = (uint32_t)(((blk_index+1) % blk_num) * blk_size);
                p_share_info->write_offset = write_offset;
            }
        }
    }
    StreamDSP_HWSemaphoreGive();
    hal_nvic_restore_interrupt_mask(saved_mask);
}


ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_dummy_status_reset(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    uint32_t i;

    /* Reset clkskew status */
    dongle_handle->clk_skew_count = 0;

    /* Reset SW buffer */
    for (i = 0; i < dongle_handle->source_info.usb_in.channel_num; i++) {
        stream_function_sw_buffer_reset_channel_buffer(dongle_handle->buffer_port_0, 1 + i, false);
    }

    /* Reset SW mixer */

    /* Reset stream status */
    audio_transmitter_share_information_fetch(dongle_handle->source, NULL);
    dongle_handle->source->streamBuffer.ShareBufferInfo.read_offset = dongle_handle->source->streamBuffer.ShareBufferInfo.write_offset;
    audio_transmitter_share_information_update_read_offset(dongle_handle->source, dongle_handle->source->streamBuffer.ShareBufferInfo.read_offset);

    ULL_V2_LOG_W("[ULL Audio V2][DL][handle 0x%x] reset dummy process status, %d", 2, dongle_handle, dongle_handle->source->streamBuffer.ShareBufferInfo.read_offset);
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_dummy_process(SINK sink)
{
    uint32_t i, dump_size, consume_size, left_size, sample_byte;
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;

    /* Copy payload to sink buffer */
    ull_audio_v2_dongle_dl_sink_internal_copy_payload(sink, g_ull_audio_v2_dongle_dl_dummy, true);

    /* Update the WPTR of sink buffer */
    ull_audio_v2_dongle_dl_sink_internal_update_write_offset(sink, 0, NULL);

    /* Do audio dump */
    if (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
        sample_byte = 2;
    } else {
        sample_byte = 4;
    }
    dump_size = (dongle_handle->sink_info.bt_out.sample_rate / 1000) * (dongle_handle->sink_info.bt_out.frame_interval / 1000) * sample_byte;
    left_size = dump_size;
    for (i = 0; i < dump_size; i += consume_size) {
        if (left_size >= sizeof(g_ull_audio_v2_dongle_dl_dummy)) {
            consume_size = sizeof(g_ull_audio_v2_dongle_dl_dummy);
        } else {
            consume_size = left_size;
        }
        left_size -= consume_size;
        LOG_AUDIO_DUMP(g_ull_audio_v2_dongle_dl_dummy, consume_size, AUDIO_SOURCE_IN_L);
        LOG_AUDIO_DUMP(g_ull_audio_v2_dongle_dl_dummy, consume_size, AUDIO_SOURCE_IN_R);
    }

    ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING]dl silence padding to sink buffer done", 0);
}

#ifdef AIR_SILENCE_DETECTION_ENABLE
static void ull_audio_v2_dongle_dl_software_timer_handler(void *user_data)
{
    SINK sink;
    uint32_t i;
    uint32_t gpt_count;
    uint32_t bt_count;
    uint32_t blk_index;
    ull_audio_v2_dongle_dl_handle_t *c_handle;
    static uint32_t error_count = 0;
    uint32_t gpt_time_us;
    UNUSED(user_data);

    /* repeat this timer */
    gpt_time_us = ull_audio_v2_dongle_first_dl_handle->sink_info.bt_out.frame_interval;
    hal_gpt_sw_start_timer_us(ull_audio_v2_dl_sw_timer, gpt_time_us, ull_audio_v2_dongle_dl_software_timer_handler, NULL);

    /* get timestamp for debug */
    bt_count = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    /* dummy write index */
    blk_index = 0;

    /* check if there is any DL stream */
    if (ull_audio_v2_dongle_first_dl_handle == NULL)
    {
        if ((error_count%1000) == 0)
        {
            ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING]dl dongle_handle is NULL, %d\r\n", 1, error_count);
        }
        error_count += 1;
        return;
    }
    else
    {
        error_count = 0;
    }

    /* trigger all started DL stream one by one */
    c_handle = ull_audio_v2_dongle_first_dl_handle;
    for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++)
    {
        if ((c_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_START) || (c_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_RUNNING))
        {
            switch (c_handle->sub_id)
            {
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
                    /* get sink */
                    sink = (SINK)c_handle->owner;
                    if ((sink == NULL) || (sink->transform == NULL))
                    {
                        break;
                    }
                    /* set timestamp for debug */
                    c_handle->ccni_in_bt_count  = bt_count;
                    c_handle->ccni_in_gpt_count = gpt_count;
                    /* set blk index */
                    c_handle->ccni_blk_index    = blk_index;
                    /* check if the stream needs to prepare a new DL packet */
                    ull_audio_v2_dongle_dl_prepare_packet_check(c_handle);
                    /* Handler the stream */
                    AudioCheckTransformHandle(sink->transform);
                    break;

                default:
                    AUDIO_ASSERT(0);
                    break;
            }
        }

        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }
    if (c_handle != NULL)
    {
        AUDIO_ASSERT(0);
    }

    return;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_dl_software_timer_start(ull_audio_v2_dongle_dl_handle_t *handle)
{
    uint32_t saved_mask;
    uint32_t gpt_time_us;
    bool do_timer_operation = false;
    UNUSED(handle);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (ull_audio_v2_dl_sw_timer_count == 0)
    {
        do_timer_operation = true;
    }
    ull_audio_v2_dl_sw_timer_count += 1;
    if (ull_audio_v2_dl_sw_timer_count == 0)
    {
        AUDIO_ASSERT(0);
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (do_timer_operation)
    {
        hal_gpt_sw_get_timer(&ull_audio_v2_dl_sw_timer);
        gpt_time_us = ull_audio_v2_dongle_first_dl_handle->sink_info.bt_out.frame_interval;
        hal_gpt_sw_start_timer_us(ull_audio_v2_dl_sw_timer, gpt_time_us, ull_audio_v2_dongle_dl_software_timer_handler, NULL);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_dl_software_timer_stop(ull_audio_v2_dongle_dl_handle_t *handle)
{
    uint32_t saved_mask;
    bool do_timer_operation = false;
    UNUSED(handle);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (ull_audio_v2_dl_sw_timer_count == 0)
    {
        AUDIO_ASSERT(0);
    }
    ull_audio_v2_dl_sw_timer_count -= 1;
    if (ull_audio_v2_dl_sw_timer_count == 0)
    {
        do_timer_operation = true;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (do_timer_operation)
    {
        hal_gpt_sw_stop_timer_ms(ull_audio_v2_dl_sw_timer);
        hal_gpt_sw_free_timer(ull_audio_v2_dl_sw_timer);
        ull_audio_v2_dl_sw_timer = 0;
    }
}
#endif /* AIR_SILENCE_DETECTION_ENABLE */

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_dl_stream_software_timer_handler(void *user_data)
{
    int32_t interval_delta;
    uint32_t i, interval_delta_value, next_blk_index, gpt_time_us, current_gpt;
    hal_ccni_message_t ccni_msg;
    ull_audio_v2_dongle_dl_handle_t *handle;
    n9_dsp_share_info_ptr p_share_info;
    BTTIME_STRU curr_bt_time, trigger_bt_time;

    UNUSED(user_data);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_gpt);
    MCE_GetBtClk(&(curr_bt_time.period), &(curr_bt_time.phase), BT_CLK_Offset);
    UNUSED(curr_bt_time.phase);

    handle = ull_audio_v2_dongle_first_dl_handle;
    for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++) {
        if (handle->stream_status != ULL_AUDIO_V2_DONGLE_STREAM_DEINIT) {
            break;
        }
        handle = handle->next_dl_handle;
    }
    if (i >= (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number) {
        return;
    }

    curr_bt_time.phase += (curr_bt_time.period & 0x00000003) * 625;
    curr_bt_time.period &= 0xFFFFFFFC;
    interval_delta = MCE_Compare_Val_FromAB(&curr_bt_time, &(handle->stream_trigger_bt_time));
    if (interval_delta >= 0) {
        interval_delta_value = (int32_t)MCE_Get_Offset_FromAB(&curr_bt_time, &(handle->stream_trigger_bt_time));
    } else {
        interval_delta_value = (int32_t)MCE_Get_Offset_FromAB(&(handle->stream_trigger_bt_time), &curr_bt_time);
    }

    /* repeat this timer */
    gpt_time_us = handle->sink_info.bt_out.frame_interval;
    if (interval_delta >= 0) {
        gpt_time_us += interval_delta_value;
    } else {
        if (g_ull_audio_v2_dl_stream_irq_is_trigger == true) {
            gpt_time_us = handle->sink_info.bt_out.frame_interval - interval_delta_value % handle->sink_info.bt_out.frame_interval;
        } else {
            gpt_time_us = 2 * handle->sink_info.bt_out.frame_interval - interval_delta_value % handle->sink_info.bt_out.frame_interval;
        }
    }
    hal_gpt_sw_start_timer_us(ull_audio_v2_dl_stream_sw_timer, gpt_time_us, ull_audio_v2_dongle_dl_stream_software_timer_handler, NULL);

    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++) {
        if (handle->sink_info.bt_out.bt_info[i] != NULL) {
            break;
        }
    }
    p_share_info = (n9_dsp_share_info_ptr)((handle->sink_info.bt_out.bt_info[i])->bt_link_info.share_info);
    if (g_ull_audio_v2_dl_stream_irq_is_trigger == true) {
        next_blk_index = (handle->ccni_blk_index + 1) % p_share_info->sub_info.block_info.block_num;
    } else {
        next_blk_index = handle->ccni_blk_index;
    }

#if ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG
    ULL_V2_LOG_I("[ULL Audio V2][DL] dl_stream_software_timer_handler: trigger: 0x%x, 0x%x, expect: 0x%x, 0x%x, drift %d, %d, gpt: %d, curr GPT: %d, AVM idx: %d", 9,
                    handle->stream_trigger_bt_time.period, handle->stream_trigger_bt_time.phase,
                    curr_bt_time.period, curr_bt_time.phase, interval_delta, interval_delta_value, gpt_time_us, current_gpt, next_blk_index);
#endif

    if (g_ull_audio_v2_dl_stream_irq_is_trigger == true) {
        ccni_msg.ccni_message[0] = curr_bt_time.period;
        ccni_msg.ccni_message[1] = next_blk_index;
        ull_audio_v2_dongle_dl_ccni_handler(0, &ccni_msg);
    }

    MCE_Add_us_FromA(handle->sink_info.bt_out.frame_interval, &(handle->stream_trigger_bt_time), &trigger_bt_time);

    /* Update trigger time for all streams */
    handle = ull_audio_v2_dongle_first_dl_handle;
    for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++) {
        handle->stream_trigger_bt_time = trigger_bt_time;
        handle = handle->next_dl_handle;
    }

    g_ull_audio_v2_dl_stream_irq_is_trigger = true;
}

static void ull_audio_v2_dongle_dl_stream_software_timer_start(void)
{
    uint32_t saved_mask;
    uint32_t gpt_time_us;
    BTTIME_STRU curr_bt_time;
    ull_audio_v2_dongle_dl_handle_t *handle;

    ULL_V2_LOG_I("[ULL Audio V2][DL] dl_stream_software_timer_start begin %d, 0x%x, %d", 3,
                    g_ull_audio_v2_dl_play_info_need_trigger,
                    ull_audio_v2_dongle_first_dl_handle,
                    g_ull_audio_v2_dl_stream_sw_timer_is_trigger);

    /* Check whether play info is ready */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if ((g_ull_audio_v2_dl_play_info_need_trigger == false) ||
        (ull_audio_v2_dongle_first_dl_handle == NULL) ||
        (g_ull_audio_v2_dl_stream_sw_timer_is_trigger == true)) {
        hal_nvic_restore_interrupt_mask(saved_mask);
        return;
    }
    g_ull_audio_v2_dl_stream_sw_timer_is_trigger = true;
    g_ull_audio_v2_dl_play_info_need_trigger = false;
    hal_nvic_restore_interrupt_mask(saved_mask);

    handle = ull_audio_v2_dongle_first_dl_handle;
    handle->ccni_blk_index = ull_audio_v2_bt_init_play_info.dl_packet_counter;
    MCE_GetBtClk(&(curr_bt_time.period), &(curr_bt_time.phase), BT_CLK_Offset);
    handle->stream_trigger_bt_time.period = ull_audio_v2_bt_init_play_info.dl_timestamp_clk & 0xFFFFFFFC;
    handle->stream_trigger_bt_time.phase = ull_audio_v2_bt_init_play_info.dl_timestamp_phase;
    MCE_Subtract_us_Fromb(DL_ULD_PROCESS_TIME_MAX, &handle->stream_trigger_bt_time, &handle->stream_trigger_bt_time);
    gpt_time_us = MCE_Get_Offset_FromAB(&curr_bt_time, &handle->stream_trigger_bt_time);
    if (gpt_time_us > DL_PLAY_TIME_CHECK_THRESHOLD) {
        gpt_time_us = MCE_Get_Offset_FromAB(&handle->stream_trigger_bt_time, &curr_bt_time);
        gpt_time_us = (gpt_time_us / handle->sink_info.bt_out.frame_interval + 2) * handle->sink_info.bt_out.frame_interval;
        MCE_Add_us_FromA(gpt_time_us, &handle->stream_trigger_bt_time, &handle->stream_trigger_bt_time);
        gpt_time_us = MCE_Get_Offset_FromAB(&curr_bt_time, &handle->stream_trigger_bt_time);
        ULL_V2_LOG_W("[ULL Audio V2][DL] dl_stream_software_timer_start re-adjust the play time", 0);
    }
    hal_gpt_sw_get_timer(&ull_audio_v2_dl_stream_sw_timer);
    hal_gpt_sw_start_timer_us(ull_audio_v2_dl_stream_sw_timer, gpt_time_us, ull_audio_v2_dongle_dl_stream_software_timer_handler, NULL);
    g_ull_audio_v2_dl_stream_irq_is_trigger = false;

    ULL_V2_LOG_I("[ULL Audio V2][DL] dl_stream_software_timer_start end: GPT interval: %d, curr BT: 0x%x, 0x%x, DSP trigger BT: 0x%x, 0x%x, expect BT: 0x%x, 0x%x", 7,
                    gpt_time_us,
                    curr_bt_time.period,
                    curr_bt_time.phase,
                    handle->stream_trigger_bt_time.period,
                    handle->stream_trigger_bt_time.phase,
                    ull_audio_v2_bt_init_play_info.dl_timestamp_clk & 0xFFFFFFFC,
                    ull_audio_v2_bt_init_play_info.dl_timestamp_phase);
}

static void ull_audio_v2_dongle_dl_stream_software_timer_stop(void)
{
    hal_gpt_sw_stop_timer_us(ull_audio_v2_dl_stream_sw_timer);
    hal_gpt_sw_free_timer(ull_audio_v2_dl_stream_sw_timer);
    ull_audio_v2_dl_stream_sw_timer = 0;
    g_ull_audio_v2_dl_play_info_need_trigger = false;
    g_ull_audio_v2_dl_stream_irq_is_trigger = false;
    g_ull_audio_v2_dl_stream_sw_timer_is_trigger = false;
}


/******************************************************************************/
/*               ULL audio 2.0 dongle UL path Private Functions               */
/******************************************************************************/
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static ull_audio_v2_dongle_ul_handle_t *ull_audio_v2_dongle_ul_get_handle(void *owner)
{
    uint32_t saved_mask;
    int16_t count;
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = NULL;
    ull_audio_v2_dongle_ul_handle_t *c_handle = NULL;

    dongle_handle = malloc(sizeof(ull_audio_v2_dongle_ul_handle_t));
    if (dongle_handle == NULL)
    {
        AUDIO_ASSERT(0);
    }
    memset(dongle_handle, 0, sizeof(ull_audio_v2_dongle_ul_handle_t));

    dongle_handle->total_number = -1;
    dongle_handle->index = -1;
    dongle_handle->owner = owner;
    dongle_handle->next_ul_handle = NULL;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (ull_audio_v2_dongle_first_ul_handle == NULL)
    {
        /* there is no item on the handle list */
        dongle_handle->total_number = 1;
        dongle_handle->index = 1;

        /* set this handle as the head of the handle list */
        ull_audio_v2_dongle_first_ul_handle = dongle_handle;
    }
    else
    {
        /* there are other items on the handle list */
        count = 1;
        c_handle = ull_audio_v2_dongle_first_ul_handle;
        while (c_handle != NULL)
        {
            count += 1;

            c_handle->total_number += 1;
            if (c_handle->total_number <= 0)
            {
                /* error status */
                AUDIO_ASSERT(0);
            }

            if (c_handle->next_ul_handle == NULL)
            {
                /* c_handle is the last item on the list now */
                dongle_handle->total_number = c_handle->total_number;
                dongle_handle->index = c_handle->total_number;

                /* dongle_handle is the last item on the list now */
                c_handle->next_ul_handle = dongle_handle;

                break;
            }

            c_handle = c_handle->next_ul_handle;
        }
        if ((c_handle == NULL) || (dongle_handle->total_number != count))
        {
            AUDIO_ASSERT(0);
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    return dongle_handle;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_ul_release_handle(ull_audio_v2_dongle_ul_handle_t *handle)
{
    uint32_t saved_mask, i, count;
    ull_audio_v2_dongle_ul_handle_t *l_handle = NULL;
    ull_audio_v2_dongle_ul_handle_t *c_handle = NULL;
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = NULL;

    if ((handle == NULL) || (ull_audio_v2_dongle_first_ul_handle == NULL))
    {
        AUDIO_ASSERT(0);
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (ull_audio_v2_dongle_first_ul_handle->total_number <= 0)
    {
        /* error status */
        AUDIO_ASSERT(0);
    }
    else if ((ull_audio_v2_dongle_first_ul_handle->total_number == 1) &&
            (ull_audio_v2_dongle_first_ul_handle == handle))
    {
        /* this handle is only one item on handle list */
        if (ull_audio_v2_dongle_first_ul_handle->next_ul_handle != NULL)
        {
            AUDIO_ASSERT(0);
        }

        /* there is no item on the handle list */
        ull_audio_v2_dongle_first_ul_handle = NULL;
    }
    else if ((ull_audio_v2_dongle_first_ul_handle->total_number > 1) &&
            (ull_audio_v2_dongle_first_ul_handle == handle))
    {
        /* this handle is the first item on handle list, but there are other handles on the list */
        c_handle = ull_audio_v2_dongle_first_ul_handle;
        count = ull_audio_v2_dongle_first_ul_handle->total_number;
        for (i=0; i < count; i++)
        {
            c_handle->total_number -= 1;
            c_handle->index -= 1;

            c_handle = c_handle->next_ul_handle;
        }
        if (c_handle != NULL)
        {
            /* error status */
            AUDIO_ASSERT(0);
        }

        /* change the next iten to the head */
        ull_audio_v2_dongle_first_ul_handle = handle->next_ul_handle;
    }
    else
    {
        /* this handle is the middle item on handle list */
        c_handle = ull_audio_v2_dongle_first_ul_handle;
        count = ull_audio_v2_dongle_first_ul_handle->total_number;
        if (count == 1)
        {
            /* error status */
            AUDIO_ASSERT(0);
        }
        for (i=0; i < count; i++)
        {
            if (c_handle == handle)
            {
                /* find out the handle on the list */
                dongle_handle = handle;
                l_handle->next_ul_handle = c_handle->next_ul_handle;
            }

            if (dongle_handle == NULL)
            {
                c_handle->total_number -= 1;
            }
            else
            {
                c_handle->total_number -= 1;
                c_handle->index -= 1;
            }

            l_handle = c_handle;
            c_handle = c_handle->next_ul_handle;
        }
        if ((c_handle != NULL) || (dongle_handle == NULL))
        {
            /* error status */
            AUDIO_ASSERT(0);
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    free(handle);
}

ATTR_TEXT_IN_IRAM static bool ull_audio_v2_dongle_ul_play_time_is_arrived(ull_audio_v2_dongle_ul_handle_t *dongle_handle, uint32_t bt_clk)
{
    bool ret = false;

    if ((dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED) || (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY))
    {
        if (dongle_handle->source_info.bt_in.first_anchor_bt_clk < dongle_handle->source_info.bt_in.play_en_bt_clk)
        {
            if (bt_clk >= dongle_handle->source_info.bt_in.play_en_bt_clk)
            {
                /* --------- ........ --------- anchor ---------- bt_clk ---------- playen --------- ........ -------- */
                ret = true;
            }
            else if ((bt_clk < dongle_handle->source_info.bt_in.play_en_bt_clk) && (bt_clk < dongle_handle->source_info.bt_in.first_anchor_bt_clk))
            {
                /* ---------- bt_clk --------- ........ --------- anchor  --------- ........ ---------- playen -------- */
                ret = true;
            }
        }
        else
        {
            if ((bt_clk >= dongle_handle->source_info.bt_in.play_en_bt_clk) && (bt_clk < dongle_handle->source_info.bt_in.first_anchor_bt_clk))
            {
                /* ---------- playen ---------- bt_clk --------- ........ --------- anchor  --------- ........  -------- */
                ret = true;
            }
        }
    }

    return ret;
}

ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_ul_ccni_handler(hal_ccni_event_t event, void * msg)
{
    SOURCE source;
    hal_ccni_message_t *ccni_msg = msg;
    ull_audio_v2_dongle_ul_handle_t *c_handle;
    uint32_t gpt_count;
    uint32_t mcu_gpt_count;
    uint32_t mcu_frame_count;
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t write_offset;
    uint32_t i;
    UNUSED(event);

    /* check if there is any UL stream */
    if (ull_audio_v2_dongle_first_ul_handle == NULL)
    {
        ULL_V2_LOG_W("[ULL Audio V2][UL][WARNNING]ul dongle_handle is NULL\r\n", 0);
        goto _ccni_return;
    }
    /* get timestamp for debug */
    mcu_gpt_count = ccni_msg->ccni_message[0];
    mcu_frame_count = ccni_msg->ccni_message[1];
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    /* trigger all started UL stream one by one */
    c_handle = ull_audio_v2_dongle_first_ul_handle;
    for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_ul_handle->total_number; i++)
    {
        if ((c_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_START) || (c_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_RUNNING))
        {
            switch (c_handle->sub_id)
            {
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
                    /* get source */
                    source = (SOURCE)c_handle->owner;
                    if ((source == NULL) || (source->transform == NULL))
                    {
                        break;
                    }
                    /* set timestamp for debug */
                    c_handle->ccni_in_bt_count  = mcu_gpt_count;
                    c_handle->ccni_in_gpt_count = gpt_count;
                    /* save the unprocessed frame count in the share buffer from the mcu side */
                    c_handle->sink_info.usb_out.mcu_frame_count = mcu_frame_count;
                    /* increase fetch count */
                    if (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY)
                    {
                        /* trigger stream to find out the play time */
                        c_handle->fetch_count = 1;
                    }
                    else if (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY)
                    {
                        /* check if the current bt clock is play time */
                        MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);
                        if (ull_audio_v2_dongle_ul_play_time_is_arrived(c_handle, bt_clk))
                        {
                            c_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED;
                            /* update share buffer write offset here for more accurate latency */
                            audio_transmitter_share_information_fetch(NULL, source->transform->sink);
                            write_offset = (source->transform->sink->streamBuffer.ShareBufferInfo.write_offset + source->transform->sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * c_handle->process_frames) % (source->transform->sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->transform->sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num);
                            if (write_offset != 0)
                            {
                                audio_transmitter_share_information_update_write_offset(source->transform->sink, write_offset);
                                /* update sink->streamBuffer.ShareBufferInfo.write_offset in here for ull_audio_v2_dongle_ul_sink_query_write_offset() */
                                audio_transmitter_share_information_fetch(NULL, source->transform->sink);
                                ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x]stream is played, %d, 0x%x, 0x%x\r\n", 4, c_handle, c_handle->process_frames, bt_clk, write_offset);
                                c_handle->process_frames = 0;
                                c_handle->fetch_count = 0;
                            }
                            else
                            {
                                ULL_V2_LOG_E("[ULL Audio V2][UL][handle 0x%x]stream is played but there is no data, %d, 0x%x, 0x%x\r\n", 4, c_handle, c_handle->process_frames, bt_clk, write_offset);
                                c_handle->fetch_count = 0;
                            }
                        }
                        else
                        {
                            c_handle->fetch_count = 0;
                        }
                    }
                    else if (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED)
                    {
                        /* trigger stream do something */
                        c_handle->fetch_count = 0;
                    }
                    else
                    {
                        c_handle->fetch_count = 0;
                    }
                    /* Handler the stream */
                    AudioCheckTransformHandle(source->transform);
                    break;

#if defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
                    break;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

                default:
                    AUDIO_ASSERT(0);
                    break;
            }
        }

        /* switch to the next dl stream */
        c_handle = c_handle->next_ul_handle;
    }
    if (c_handle != NULL)
    {
        AUDIO_ASSERT(0);
    }

_ccni_return:
    return;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_ul_mixer_precallback(sw_mixer_member_t *member, void *para)
{
    uint32_t saved_mask;
    SOURCE source;
    ull_audio_v2_dongle_ul_handle_t *dongle_handle;
    ull_audio_v2_dongle_ul_handle_t *c_handle;
    uint32_t i;
    uint8_t *stream_ch_buffer;
    uint32_t channel_number;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (ull_audio_v2_dongle_first_ul_handle->total_number > 1)
    {
        source = (SOURCE)(member->owner);
        dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
        if (dongle_handle->data_status == ULL_AUDIO_V2_DONGLE_UL_DATA_IN_MIXER)
        {
            /*  because the required ul packet data are in other stream's mixer and the decoder output of the currnet stream size is 0,
                we need to set the frame size of the current stream to the effective size for the following features are running correctly */
            stream_function_modify_output_size(para, ull_audio_v2_ul_packet_size_in_mixer);
            /*  because the required ul packet data are in other stream's mixer,
                we need to set the channel data of the current stream to 0, then the effective data in other stream's mixer will mixed with these 0 data */
            switch (dongle_handle->sub_id)
            {
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
                    channel_number = dongle_handle->sink_info.usb_out.channel_num;
                    break;

#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
                    channel_number = dongle_handle->sink_info.line_out.channel_num;
                    break;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
                    channel_number = dongle_handle->sink_info.i2s_mst_out.channel_num;
                    break;
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
                    channel_number = dongle_handle->sink_info.i2s_slv_out.channel_num;
                    break;
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

                default:
                    channel_number = 0;
                    AUDIO_ASSERT(0);
                    break;
            }
            for (i = 0; i < channel_number; i++)
            {
                stream_ch_buffer = stream_function_get_inout_buffer(para, i + 1);
                memset(stream_ch_buffer, 0, ull_audio_v2_ul_packet_size_in_mixer);
            }
        }
        else if (dongle_handle->data_status == ULL_AUDIO_V2_DONGLE_UL_DATA_NORMAL)
        {
            /*  because the required ul packet data are in the current stream's mixer,
                we need to record the newest packet size for the other stream */
            ull_audio_v2_ul_packet_size_in_mixer = stream_function_get_output_size(para);
            /*  because the required ul packet data are in the current stream's mixer,
                we need to clean other streams input channel and the oldest data in the other streams will not be mixed with the current stream */
            c_handle = ull_audio_v2_dongle_first_ul_handle;
            for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_ul_handle->total_number; i++)
            {
                if (c_handle != dongle_handle)
                {
                    stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);
                }
                c_handle = c_handle->next_ul_handle;
            }
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_ul_mixer_all_stream_connections_update(ull_audio_v2_dongle_ul_handle_t *dongle_handle, bool new_stream, uint32_t mixer_connection_status)
{
    int16_t i,j;
    ull_audio_v2_dongle_ul_handle_t *c_handle;
    //uint32_t saved_mask;
    uint32_t ch_connection = 0;
    uint32_t last_ch_connection = 0;

    if (new_stream)
    {
        /* not only update this new stream's connection status */
        //hal_nvic_save_and_set_interrupt_mask_special(&saved_mask);
        for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
        {
            ch_connection = (mixer_connection_status >> (8*i))&0xff;
            if (ch_connection != 0)
            {
                c_handle = ull_audio_v2_dongle_first_ul_handle;
                for (j = 0; j < ull_audio_v2_dongle_first_ul_handle->total_number; j++)
                {
                    /* connect new connection */
                    if (c_handle == dongle_handle)
                    {
                        stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, ch_connection, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle->mixer_member, i+1);
                    }
                    else
                    {
                        stream_function_sw_mixer_channel_connect(c_handle->mixer_member, ch_connection, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle->mixer_member, i+1);
                    }
                    /* switch the next stream */
                    c_handle = c_handle->next_ul_handle;
                }
            }
        }
        //hal_nvic_restore_interrupt_mask_special(saved_mask);

        /* but also connect this stream into other stream */
        //hal_nvic_save_and_set_interrupt_mask_special(&saved_mask);
        c_handle = ull_audio_v2_dongle_first_ul_handle;
        for (j = 0; j < ull_audio_v2_dongle_first_ul_handle->total_number; j++)
        {
            if (c_handle != dongle_handle)
            {
                for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
                {
                    ch_connection = (c_handle->mixer_connection_status >> (8*i))&0xff;
                    if (ch_connection != 0)
                    {
                        stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, ch_connection, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, c_handle->mixer_member, i+1);
                    }
                }
            }
            /* switch the next stream */
            c_handle = c_handle->next_ul_handle;
        }
        //hal_nvic_restore_interrupt_mask_special(saved_mask);
    }
    else
    {
        /* only update this stream's connection status */
        if (mixer_connection_status != dongle_handle->mixer_connection_status)
        {
            //hal_nvic_save_and_set_interrupt_mask_special(&saved_mask);
            for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
            {
                ch_connection = (mixer_connection_status >> (8*i))&0xff;
                last_ch_connection = (dongle_handle->mixer_connection_status >> (8*i))&0xff;
                if (ch_connection != last_ch_connection)
                {
                    c_handle = ull_audio_v2_dongle_first_ul_handle;
                    for (j = 0; j < ull_audio_v2_dongle_first_ul_handle->total_number; j++)
                    {
                        /* disconnect old connection */
                        stream_function_sw_mixer_channel_disconnect(c_handle->mixer_member, last_ch_connection, dongle_handle->mixer_member, i+1);
                        /* connect new connection */
                        if (c_handle == dongle_handle)
                        {
                            stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, ch_connection, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle->mixer_member, i+1);
                        }
                        else
                        {
                            stream_function_sw_mixer_channel_connect(c_handle->mixer_member, ch_connection, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle->mixer_member, i+1);
                        }
                        /* switch the next stream */
                        c_handle = c_handle->next_ul_handle;
                    }
                }
            }
            //hal_nvic_restore_interrupt_mask_special(saved_mask);
        }
    }

    ULL_V2_LOG_I("[ULL Audio V2][UL][stream_connections_update][handle 0x%x] update stream connection status: %u, 0x%x.",
                3,
                dongle_handle,
                new_stream,
                mixer_connection_status);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_ul_bt_info_init(ull_audio_v2_dongle_ul_handle_t *dongle_handle, uint32_t i, bt_common_open_param_p bt_common_open_param)
{
    uint32_t j, saved_mask;

    /* update bt link settings to global state machine */
    for (j = 0; j < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; j++)
    {
        if (hal_memview_cm4_to_dsp0((uint32_t)(bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.bt_link_info[i].share_info)) == (uint32_t)(ull_audio_v2_ul_bt_info[j].bt_link_info.share_info))
        {
            /* check bt link settings */
            if (memcmp(&(bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.bt_link_info[i].codec_type), &(ull_audio_v2_ul_bt_info[j].bt_link_info.codec_type), sizeof(ull_audio_v2_dongle_bt_link_info_t)-sizeof(void *)) != 0)
            {
                /* same share buffer, same codec settings */
                ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]codec setting is different\r\n", 0);
                AUDIO_ASSERT(0);
            }
            /* in here, it means this bt link's setting has been used */
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            ull_audio_v2_ul_bt_info[j].user_count += 1;
            hal_nvic_restore_interrupt_mask(saved_mask);
            dongle_handle->source_info.bt_in.bt_info[i] = &ull_audio_v2_ul_bt_info[j];
        }
    }
    if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
    {
        /* in here, it means this bt link's setting has not been used */
        for (j = 0; j < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; j++)
        {
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            if (ull_audio_v2_ul_bt_info[j].bt_link_info.share_info == NULL)
            {
                if (ull_audio_v2_ul_bt_info[j].user_count != 0)
                {
                    ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]user count is not right, %u\r\n", 1, ull_audio_v2_ul_bt_info[j].user_count);
                    AUDIO_ASSERT(0);
                }
                ull_audio_v2_ul_bt_info[j].user_count += 1;
                ull_audio_v2_ul_bt_info[j].seq_num = 0;
                ull_audio_v2_ul_bt_info[j].blk_index = 0;
                ull_audio_v2_ul_bt_info[j].blk_index_previous = 0;
                memcpy(&(ull_audio_v2_ul_bt_info[j].bt_link_info), &(bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.bt_link_info[i]), sizeof(ull_audio_v2_dongle_bt_link_info_t));
                ull_audio_v2_ul_bt_info[j].bt_link_info.share_info = (void *)hal_memview_cm4_to_dsp0((uint32_t)(ull_audio_v2_ul_bt_info[j].bt_link_info.share_info));
                dongle_handle->source_info.bt_in.bt_info[i] = &ull_audio_v2_ul_bt_info[j];
                hal_nvic_restore_interrupt_mask(saved_mask);
                break;
            }
            hal_nvic_restore_interrupt_mask(saved_mask);
        }
        if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
        {
            /* not found suitable bt info channel */
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]all bt info are used\r\n", 0);
            AUDIO_ASSERT(0);
        }
    }
}

static void ull_audio_v2_dongle_ul_bt_in_init(ull_audio_v2_dongle_ul_handle_t *dongle_handle, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    uint32_t i;
    uint32_t frame_interval;
    uint32_t sample_rate;
    hal_audio_format_t sample_format;
    uint32_t frame_samples;
    uint32_t frame_size;
    uint32_t bit_rate;
    uint32_t channel_num = 0;
    audio_dsp_codec_type_t codec_type;
#if defined(AIR_CELT_DEC_V2_ENABLE)
    uint32_t version = 0;
#endif /* AIR_CELT_DEC_V2_ENABLE */
    UNUSED(audio_transmitter_open_param);

    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.bt_link_info[i].share_info == NULL)
        {
            continue;
        }
        else
        {
            /* update channel num */
            channel_num += 1;
            ull_audio_v2_dongle_ul_bt_info_init(dongle_handle, i, bt_common_open_param);
            if ((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
            {
                codec_type     = AUDIO_DSP_CODEC_TYPE_LC3PLUS;
                bit_rate       = (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.lc3plus.bit_rate;
                frame_interval = (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.lc3plus.frame_interval;
                sample_rate    = (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.lc3plus.sample_rate;
                sample_format  = (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.lc3plus.sample_format;
                frame_samples  = sample_rate/1000*frame_interval/1000;
                frame_size     = ull_audio_v2_codec_get_frame_size(&((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_type), &((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param));
            }
            else if ((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
            {
#if defined(AIR_CELT_DEC_V2_ENABLE)
                codec_type     = AUDIO_DSP_CODEC_TYPE_OPUS;
                bit_rate       = (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.opus.bit_rate;
                frame_interval = (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.opus.frame_interval;
                sample_rate    = (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.opus.sample_rate;
                sample_format  = (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.opus.sample_format;
                frame_samples  = sample_rate/1000*frame_interval/1000;
                frame_size     = ull_audio_v2_codec_get_frame_size(&((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_type), &((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param));
                if (version == 0)
                {
                    version = (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.opus.version;
                    extern uint32_t stream_codec_celt_set_version(uint32_t version);
                    stream_codec_celt_set_version(version);
                    ULL_V2_LOG_E("[ULL Audio V2][UL]opus codec version = 0x%x\r\n", 1, version);
                }
                else if (version != ((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.opus.version))
                {
                    ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]codec version is not right, 0x%x, 0x%x\r\n", 2, version, (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_param.opus.version);
                    AUDIO_ASSERT(0);
                }
#endif /* AIR_CELT_DEC_V2_ENABLE */
            }
            else
            {
                ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]codec is not supported, %d\r\n", 1, (dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.codec_type);
                AUDIO_ASSERT(0);
            }
            /* update codec type */
            if (dongle_handle->source_info.bt_in.codec_type == 0)
            {
                dongle_handle->source_info.bt_in.codec_type = codec_type;
            }
            else if (dongle_handle->source_info.bt_in.codec_type != codec_type)
            {
                ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]codec_type is not right, %u, %u\r\n", 2, dongle_handle->source_info.bt_in.codec_type, codec_type);
                AUDIO_ASSERT(0);
            }
            /* update frame interval */
            if (dongle_handle->source_info.bt_in.frame_interval == 0)
            {
                dongle_handle->source_info.bt_in.frame_interval = frame_interval;
            }
            else if (dongle_handle->source_info.bt_in.frame_interval != frame_interval)
            {
                ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]frame_interval is not right, %u, %u\r\n", 2, dongle_handle->source_info.bt_in.frame_interval, frame_interval);
                AUDIO_ASSERT(0);
            }
            /* update frame sample rate */
            if (dongle_handle->source_info.bt_in.sample_rate == 0)
            {
                dongle_handle->source_info.bt_in.sample_rate = sample_rate;
            }
            else if (dongle_handle->source_info.bt_in.sample_rate != sample_rate)
            {
                ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_rate is not right, %u, %u\r\n", 2, dongle_handle->source_info.bt_in.sample_rate, sample_rate);
                AUDIO_ASSERT(0);
            }
            /* update frame sample format */
            if (dongle_handle->source_info.bt_in.sample_format == 0)
            {
                dongle_handle->source_info.bt_in.sample_format = sample_format;
            }
            else if (dongle_handle->source_info.bt_in.sample_format != sample_format)
            {
                ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_format is not right, %u, %u\r\n", 2, dongle_handle->source_info.bt_in.sample_format, sample_format);
                AUDIO_ASSERT(0);
            }
            /* update frame samples */
            if (dongle_handle->source_info.bt_in.frame_samples == 0)
            {
                dongle_handle->source_info.bt_in.frame_samples = frame_samples;
            }
            else if (dongle_handle->source_info.bt_in.frame_samples != frame_samples)
            {
                ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]frame_samples is not right, %u, %u\r\n", 2, dongle_handle->source_info.bt_in.frame_samples, frame_samples);
                AUDIO_ASSERT(0);
            }
            /* update frame samples */
            if (dongle_handle->source_info.bt_in.frame_size == 0)
            {
                dongle_handle->source_info.bt_in.frame_size = frame_size;
            }
            else if (dongle_handle->source_info.bt_in.frame_size != frame_size)
            {
                ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]frame_size is not right, %u, %u\r\n", 2, dongle_handle->source_info.bt_in.frame_size, frame_size);
                AUDIO_ASSERT(0);
            }
            /* update bit_rate */
            if (dongle_handle->source_info.bt_in.bit_rate == 0)
            {
                dongle_handle->source_info.bt_in.bit_rate = bit_rate;
            }
            else if (dongle_handle->source_info.bt_in.bit_rate != bit_rate)
            {
                ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]bit_rate is not right, %u, %u\r\n", 2, dongle_handle->source_info.bt_in.bit_rate, bit_rate);
                AUDIO_ASSERT(0);
            }
        }
    }
    dongle_handle->source_info.bt_in.channel_num = channel_num;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_ul_bt_in_deinit(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t i;
    uint32_t saved_mask;

    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
        {
            continue;
        }
        else
        {
            if ((dongle_handle->source_info.bt_in.bt_info[i])->user_count == 0)
            {
                ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]user_count is not right, %u\r\n", 1, (dongle_handle->source_info.bt_in.bt_info[i])->user_count);
                AUDIO_ASSERT(0);
            }
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            (dongle_handle->source_info.bt_in.bt_info[i])->user_count -= 1;
            if ((dongle_handle->source_info.bt_in.bt_info[i])->user_count == 0)
            {
                memset(dongle_handle->source_info.bt_in.bt_info[i], 0, sizeof(ull_audio_v2_dongle_bt_info_t));
            }
            dongle_handle->source_info.bt_in.bt_info[i] = NULL;
            hal_nvic_restore_interrupt_mask(saved_mask);
        }
    }
}

#if defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
ATTR_TEXT_IN_IRAM static void ull_audio_v2_dongle_ul_software_timer_handler(void *user_data)
{
    uint32_t saved_mask;
    uint32_t gpt_count;
    ull_audio_v2_dongle_ul_handle_t *c_handle;
    SOURCE source;
    uint32_t i;
    UNUSED(user_data);

    /* repeat this timer */
    hal_gpt_sw_start_timer_us(ull_audio_v2_ul_sw_timer, 1000, ull_audio_v2_dongle_ul_software_timer_handler, NULL);

    /* check if there is any UL stream */
    if (ull_audio_v2_dongle_first_ul_handle == NULL)
    {
        ULL_V2_LOG_W("[ULL Audio V2][UL][WARNNING]ul dongle_handle is NULL\r\n", 0);
        return;
    }

    /* get timestamp for debug */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    /* trigger all started UL stream one by one */
    c_handle = ull_audio_v2_dongle_first_ul_handle;
    for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_ul_handle->total_number; i++)
    {
        if ((c_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_START) || (c_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_RUNNING))
        {
            switch (c_handle->sub_id)
            {
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
                    /* get source */
                    source = (SOURCE)c_handle->owner;
                    if ((source == NULL) || (source->transform == NULL))
                    {
                        break;
                    }
                    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                    /* increase fetch count */
                    if (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY)
                    {
                        /* set timestamp for debug */
                        c_handle->ccni_in_bt_count  = gpt_count;
                        c_handle->ccni_in_gpt_count = gpt_count;
                        /* trigger stream to find out the play time */
                        c_handle->fetch_count = 1;
                        /* Handler the stream */
                        AudioCheckTransformHandle(source->transform);
                    }
                    hal_nvic_restore_interrupt_mask(saved_mask);
                    break;

                case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
                    break;

                default:
                    AUDIO_ASSERT(0);
                    break;
            }
        }
        /* switch to the next dl stream */
        c_handle = c_handle->next_ul_handle;
    }
    if (c_handle != NULL)
    {
        AUDIO_ASSERT(0);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_ul_software_timer_start(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t saved_mask;
    UNUSED(dongle_handle);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (ull_audio_v2_ul_sw_timer_count == 0)
    {
        hal_gpt_sw_get_timer(&ull_audio_v2_ul_sw_timer);
        hal_gpt_sw_start_timer_us(ull_audio_v2_ul_sw_timer, 1000, ull_audio_v2_dongle_ul_software_timer_handler, NULL);
    }
    ull_audio_v2_ul_sw_timer_count += 1;
    if (ull_audio_v2_ul_sw_timer_count == 0)
    {
        AUDIO_ASSERT(0);
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    ULL_V2_LOG_I("[ULL Audio V2][UL]software timer start, %d\r\n", 1, ull_audio_v2_ul_sw_timer_count);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_ul_software_timer_stop(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t saved_mask;
    UNUSED(dongle_handle);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (ull_audio_v2_ul_sw_timer_count == 0)
    {
        AUDIO_ASSERT(0);
    }
    ull_audio_v2_ul_sw_timer_count -= 1;
    if (ull_audio_v2_ul_sw_timer_count == 0)
    {
        hal_gpt_sw_stop_timer_ms(ull_audio_v2_ul_sw_timer);
        hal_gpt_sw_free_timer(ull_audio_v2_ul_sw_timer);
        ull_audio_v2_ul_sw_timer = 0;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    //ULL_V2_LOG_I("[ULL Audio V2][UL]software timer stop, %d\r\n", 1, ull_audio_v2_ul_sw_timer_count);
}
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

/******************************************************************************/
/*          ULL audio 2.0 dongle UL USB-out path Private Functions            */
/******************************************************************************/
static void ull_audio_v2_dongle_ul_usb_out_init(ull_audio_v2_dongle_ul_handle_t *dongle_handle, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    uint32_t i;
    uint32_t sample_size = 0;
    n9_dsp_share_info_ptr p_share_info;
    stream_resolution_t stream_resolution = RESOLUTION_16BIT;

//#ifdef AIR_ECNR_POST_PART_ENABLE
    /* Record the NR offload status */
    g_nr_offlad_status = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.nr_offload_flag;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x][nr_offload] enable status %d", 4,
                audio_transmitter_open_param->scenario_type,
                audio_transmitter_open_param->scenario_sub_id,
                dongle_handle,
                g_nr_offlad_status);
//#endif /* AIR_ECNR_POST_PART_ENABLE */

    /* bt in info init */
    ull_audio_v2_dongle_ul_bt_in_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
    if (dongle_handle->source_info.bt_in.frame_interval == 5000)
    {
        dongle_handle->source_info.bt_in.play_en_delay                    = UL_PLAYEN_DELAY_FRAME_5000US;
        dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay  = UL_FIRST_PACKET_SAFE_DELAY_FRAME_5000US;
        dongle_handle->source_info.bt_in.bt_retry_window                  = ull_audio_v2_bt_init_play_info.dl_retransmission_window_clk;
        dongle_handle->source_info.bt_in.bt_interval                      = UL_BT_INTERVAL_FRAME_5000US;
        dongle_handle->source_info.bt_in.bt_channel_anchor_diff           = UL_BT_INTERVAL_FRAME_5000US/2;
    }
    else
    {
        ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]frame_interval is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.frame_interval);
        AUDIO_ASSERT(0);
    }
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]bt in info, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", 15,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->source_info.bt_in.channel_num,
                dongle_handle->source_info.bt_in.sample_rate,
                dongle_handle->source_info.bt_in.sample_format,
                dongle_handle->source_info.bt_in.frame_size,
                dongle_handle->source_info.bt_in.frame_samples,
                dongle_handle->source_info.bt_in.frame_interval,
                dongle_handle->source_info.bt_in.bit_rate,
                dongle_handle->source_info.bt_in.play_en_delay,
                dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay,
                dongle_handle->source_info.bt_in.bt_retry_window,
                dongle_handle->source_info.bt_in.bt_interval,
                dongle_handle->source_info.bt_in.bt_channel_anchor_diff);
    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->source_info.bt_in.bt_info[i] != NULL)
        {
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.share_info);
            ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]bt in channel %u info, %u, %u, %u, %u, %u, 0x%x, 0x%x\r\n", 11,
                        bt_common_open_param->scenario_type,
                        bt_common_open_param->scenario_sub_id,
                        dongle_handle,
                        i+1,
                        (dongle_handle->source_info.bt_in.bt_info[i])->seq_num,
                        (dongle_handle->source_info.bt_in.bt_info[i])->user_count,
                        (dongle_handle->source_info.bt_in.bt_info[i])->blk_index,
                        (dongle_handle->source_info.bt_in.bt_info[i])->blk_index_previous,
                        (dongle_handle->source_info.bt_in.bt_info[i])->crc32_init,
                        p_share_info,
                        hal_memview_infrasys_to_dsp0(p_share_info->start_addr));
        }
    }

    /* usb out info init */
    dongle_handle->sink_info.usb_out.channel_num       = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.channel_mode;
    dongle_handle->sink_info.usb_out.sample_rate       = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.sample_rate;
    dongle_handle->sink_info.usb_out.sample_format     = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.format;
    dongle_handle->sink_info.usb_out.frame_size        = usb_audio_get_frame_size(&audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_type, &audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param);
    dongle_handle->sink_info.usb_out.frame_samples     = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.sample_rate/audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.frame_interval;
    dongle_handle->sink_info.usb_out.frame_interval    = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.frame_interval;
    dongle_handle->sink_info.usb_out.frame_max_num     = (dongle_handle->source_info.bt_in.frame_interval*2) / audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.codec_param.pcm.frame_interval;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]usb out info, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->sink_info.usb_out.channel_num,
                dongle_handle->sink_info.usb_out.sample_rate,
                dongle_handle->sink_info.usb_out.sample_format,
                dongle_handle->sink_info.usb_out.frame_size,
                dongle_handle->sink_info.usb_out.frame_samples,
                dongle_handle->sink_info.usb_out.frame_interval,
                dongle_handle->sink_info.usb_out.frame_max_num);

    /* codec init */
    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_out_broadcast[];
        stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_usb_out_broadcast, CODEC_DECODER_LC3PLUS, CONFIG_DECODER);
        extern uint32_t g_plc_mode;
        lc3plus_dec_port_config_t lc3plus_dec_config;
        if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            lc3plus_dec_config.sample_bits  = 16;
            sample_size = sizeof(uint16_t);
            stream_resolution = RESOLUTION_16BIT;
        }
        else if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)
        {
            lc3plus_dec_config.sample_bits  = 24;
            sample_size = sizeof(uint32_t);
            stream_resolution = RESOLUTION_32BIT;
        }
        else
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_format is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.sample_format);
            AUDIO_ASSERT(0);
        }
        lc3plus_dec_config.sample_rate      = dongle_handle->source_info.bt_in.sample_rate;
        lc3plus_dec_config.bit_rate         = dongle_handle->source_info.bt_in.bit_rate;
        lc3plus_dec_config.channel_mode     = LC3PLUS_DEC_CHANNEL_MODE_MULTI_MONO;
        lc3plus_dec_config.in_channel_num   = dongle_handle->source_info.bt_in.channel_num;
        lc3plus_dec_config.out_channel_num  = dongle_handle->source_info.bt_in.channel_num;
        lc3plus_dec_config.frame_interval   = dongle_handle->source_info.bt_in.frame_interval;
        lc3plus_dec_config.frame_size       = dongle_handle->source_info.bt_in.frame_size;
        lc3plus_dec_config.frame_samples    = dongle_handle->source_info.bt_in.frame_samples;
        lc3plus_dec_config.plc_enable       = 1;
        lc3plus_dec_config.plc_method       = (g_plc_mode == 0)? LC3PLUS_PLCMETH_STD : LC3PLUS_PLCMETH_ADV_TDC_NS;
        stream_codec_decoder_lc3plus_init(LC3PLUS_DEC_PORT_0, dongle_handle->source, &lc3plus_dec_config);
        ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]lc3plus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                    bt_common_open_param->scenario_type,
                    bt_common_open_param->scenario_sub_id,
                    dongle_handle,
                    lc3plus_dec_config.sample_bits,
                    lc3plus_dec_config.sample_rate,
                    lc3plus_dec_config.bit_rate,
                    lc3plus_dec_config.channel_mode,
                    lc3plus_dec_config.in_channel_num,
                    lc3plus_dec_config.out_channel_num,
                    lc3plus_dec_config.frame_interval,
                    lc3plus_dec_config.frame_samples,
                    lc3plus_dec_config.frame_size);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
        extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_out_broadcast[];
        stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_usb_out_broadcast, CODEC_DECODER_OPUS_V2, CONFIG_DECODER);

        celt_dec_port_config_t celt_dec_config;
        if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            celt_dec_config.sample_bits  = 16;
            sample_size = sizeof(uint16_t);
            stream_resolution = RESOLUTION_16BIT;
        }
        else
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_format is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.sample_format);
            AUDIO_ASSERT(0);
        }
        celt_dec_config.sample_rate      = dongle_handle->source_info.bt_in.sample_rate;
        celt_dec_config.bit_rate         = dongle_handle->source_info.bt_in.bit_rate;
        celt_dec_config.channel_mode     = CELT_DEC_CHANNEL_MODE_MULTI_MONO;
        celt_dec_config.in_channel_num   = dongle_handle->source_info.bt_in.channel_num;
        celt_dec_config.out_channel_num  = dongle_handle->source_info.bt_in.channel_num;
        celt_dec_config.frame_interval   = dongle_handle->source_info.bt_in.frame_interval;
        celt_dec_config.frame_size       = dongle_handle->source_info.bt_in.frame_size;
        celt_dec_config.frame_samples    = dongle_handle->source_info.bt_in.frame_samples;
        celt_dec_config.plc_enable       = 1;
        stream_codec_decoder_celt_v2_init(CELT_DEC_PORT_0, dongle_handle->source, &celt_dec_config);
        ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]opus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                    bt_common_open_param->scenario_type,
                    bt_common_open_param->scenario_sub_id,
                    dongle_handle,
                    celt_dec_config.sample_bits,
                    celt_dec_config.sample_rate,
                    celt_dec_config.bit_rate,
                    celt_dec_config.channel_mode,
                    celt_dec_config.in_channel_num,
                    celt_dec_config.out_channel_num,
                    celt_dec_config.frame_interval,
                    celt_dec_config.frame_samples,
                    celt_dec_config.frame_size);
#endif /* AIR_CELT_DEC_V2_ENABLE */
    }

    /* sw mxier init */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback = ull_audio_v2_dongle_ul_mixer_precallback;
    callback_config.postprocess_callback = NULL;
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = stream_resolution;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_OVERWRITE;
    in_ch_config.buffer_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    out_ch_config.total_channel_number = 2;
    out_ch_config.resolution = stream_resolution;
    dongle_handle->mixer_member= stream_function_sw_mixer_member_create((void *)dongle_handle->source, SW_MIXER_MEMBER_MODE_NO_BYPASS, &callback_config, &in_ch_config, &out_ch_config);
    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, false);
    ull_audio_v2_dongle_ul_mixer_all_stream_connections_update(dongle_handle, true, 0x0101);
    dongle_handle->mixer_connection_status = 0x0101;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw mixer 0x%x info, %d, %d, %d, %d, %d, %d, 0x%x\r\n", 11,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->mixer_member,
                in_ch_config.total_channel_number,
                in_ch_config.resolution,
                in_ch_config.input_mode,
                in_ch_config.buffer_size,
                out_ch_config.total_channel_number,
                out_ch_config.resolution,
                dongle_handle->mixer_connection_status);

    /* sw clk skew init */
    sw_clk_skew_config_t sw_clk_skew_config;
    dongle_handle->clk_skew_port = stream_function_sw_clk_skew_get_port(dongle_handle->source);
    sw_clk_skew_config.channel = 2;
    if (stream_resolution == RESOLUTION_16BIT)
    {
        sw_clk_skew_config.bits = 16;
    }
    else
    {
        sw_clk_skew_config.bits = 32;
    }
    sw_clk_skew_config.order = C_Flp_Ord_1;
    sw_clk_skew_config.skew_io_mode = C_Skew_Oup;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    sw_clk_skew_config.skew_work_mode = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size = 2*dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    sw_clk_skew_config.continuous_frame_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
    dongle_handle->compen_samples = 0;
    dongle_handle->clk_skew_count = 0;
    dongle_handle->clk_skew_watermark_samples = (UL_USB_PREFILL_SAMPLES_FRAME+3)*dongle_handle->source_info.bt_in.sample_rate/1000; /* process time 3ms + clock skew 1ms */
    dongle_handle->clk_skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw clk skew 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 15,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->clk_skew_port,
                sw_clk_skew_config.channel,
                sw_clk_skew_config.bits,
                sw_clk_skew_config.order,
                sw_clk_skew_config.skew_io_mode,
                sw_clk_skew_config.skew_compensation_mode,
                sw_clk_skew_config.skew_work_mode,
                sw_clk_skew_config.max_output_size,
                sw_clk_skew_config.continuous_frame_size,
                dongle_handle->compen_samples,
                dongle_handle->clk_skew_count,
                dongle_handle->clk_skew_watermark_samples);

    /* sw buffer init */
    sw_buffer_config_t buffer_config;

//#ifdef AIR_ECNR_POST_PART_ENABLE
    if (g_nr_offlad_status == true) {
        /* Config the baud mode for ECNR */
        if (dongle_handle->source_info.bt_in.frame_samples == 32000) {
            DSP_ALG_UpdateEscoTxMode(VOICE_SWB);
        } else {
            DSP_ALG_UpdateEscoTxMode(VOICE_WB);
        }

        /* Replace the NR offload feature */
        g_ull_audio_v2_dongle_usb_out_broadcast_streams.pfeature_table = stream_feature_list_ull_audio_v2_dongle_usb_out_broadcast_nr_offload;

        /* init sw buffer list */
        dongle_handle->ecnr_buffer_list = stream_function_sw_buffer_get_list(dongle_handle->source);
        stream_function_sw_buffer_list_init(dongle_handle->ecnr_buffer_list, 3);

        /* config the first buffer */
        buffer_config.mode = SW_BUFFER_MODE_MULTI_BUFFERS;
        buffer_config.total_channels = 2;
        if (dongle_handle->source_info.bt_in.sample_rate == 32000) {
            buffer_config.watermark_max_size = 480 * sizeof(int16_t);
        } else {
            buffer_config.watermark_max_size = 240 * sizeof(int16_t);
        }
        buffer_config.watermark_min_size = 0;
        buffer_config.output_size = buffer_config.watermark_max_size; /* Output with 15ms */
        dongle_handle->ecnr_in_buffer_port = stream_function_sw_buffer_get_unused_port(dongle_handle->source);
        stream_function_sw_buffer_init(dongle_handle->ecnr_in_buffer_port, &buffer_config);
        /* Prefill 10ms for ECNR process in the 1st time */
        if (dongle_handle->source_info.bt_in.sample_rate == 32000) {
            stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->ecnr_in_buffer_port, 1, 320 * sizeof(int16_t), true);
            stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->ecnr_in_buffer_port, 2, 320 * sizeof(int16_t), true);
        } else {
            stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->ecnr_in_buffer_port, 1, 160 * sizeof(int16_t), true);
            stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->ecnr_in_buffer_port, 2, 160 * sizeof(int16_t), true);
        }
        stream_function_sw_buffer_list_insert_buffer(dongle_handle->ecnr_buffer_list, dongle_handle->ecnr_in_buffer_port, 0);

        /* config the second buffer */
        buffer_config.mode = SW_BUFFER_MODE_MULTI_BUFFERS;
        buffer_config.total_channels = 2;
        buffer_config.output_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
        buffer_config.watermark_max_size = buffer_config.output_size * 3; /* must be 5ms*N */
        buffer_config.watermark_min_size = 0;
        dongle_handle->ecnr_out_buffer_port = stream_function_sw_buffer_get_unused_port(dongle_handle->source);
        stream_function_sw_buffer_init(dongle_handle->ecnr_out_buffer_port, &buffer_config);
        stream_function_sw_buffer_list_insert_buffer(dongle_handle->ecnr_buffer_list, dongle_handle->ecnr_out_buffer_port, 1);

        /* config the third buffer */
        buffer_config.mode = SW_BUFFER_MODE_MULTI_BUFFERS;
        buffer_config.total_channels = 2;
        buffer_config.watermark_max_size = 4*dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
        buffer_config.watermark_min_size = 0;
        buffer_config.output_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
        dongle_handle->buffer_port = stream_function_sw_buffer_get_unused_port(dongle_handle->source);
        stream_function_sw_buffer_init(dongle_handle->buffer_port, &buffer_config);
        /* prefill 2ms */
        stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->buffer_port, 0, dongle_handle->source_info.bt_in.sample_rate/1000*UL_USB_PREFILL_SAMPLES_FRAME*sample_size, true);
        dongle_handle->buffer_default_output_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
        stream_function_sw_buffer_list_insert_buffer(dongle_handle->ecnr_buffer_list, dongle_handle->buffer_port, 2);
    } else {
        buffer_config.mode = SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH;
        buffer_config.total_channels = 2;
        buffer_config.watermark_max_size = 4*dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
        buffer_config.watermark_min_size = 0;
        buffer_config.output_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
        dongle_handle->buffer_port = stream_function_sw_buffer_get_port(dongle_handle->source);
        stream_function_sw_buffer_init(dongle_handle->buffer_port, &buffer_config);
        /* prefill 2ms */
        stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->buffer_port, 0, dongle_handle->source_info.bt_in.sample_rate/1000*UL_USB_PREFILL_SAMPLES_FRAME*sample_size, true);
        dongle_handle->buffer_default_output_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    }
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw buffer 0x%x info, %d, %d, %d, %d, %d, %d\r\n", 11,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->buffer_port,
                buffer_config.mode,
                buffer_config.total_channels,
                buffer_config.watermark_max_size,
                buffer_config.watermark_min_size,
                buffer_config.output_size,
                dongle_handle->buffer_default_output_size);
//#endif /* AIR_ECNR_POST_PART_ENABLE */

    /* sw gain init */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution = stream_resolution;
    default_config.target_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    default_config.up_step = 25;
    default_config.up_samples_per_step = dongle_handle->sink_info.usb_out.sample_rate/1000;
    default_config.down_step = -25;
    default_config.down_samples_per_step = dongle_handle->sink_info.usb_out.sample_rate/1000;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(dongle_handle->source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    default_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw gain 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->gain_port,
                default_config.resolution,
                default_config.target_gain,
                default_config.up_step,
                default_config.up_samples_per_step,
                default_config.down_step,
                default_config.down_samples_per_step,
                bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L,
                bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R);

    /* sw src init */
    sw_src_config_t sw_src_config;
    dongle_handle->src_in_frame_samples     = dongle_handle->source_info.bt_in.sample_rate/1000;
    dongle_handle->src_in_frame_size        = dongle_handle->src_in_frame_samples * sample_size;
    dongle_handle->src_out_frame_samples    = dongle_handle->sink_info.usb_out.sample_rate/1000;
    dongle_handle->src_out_frame_size       = dongle_handle->src_out_frame_samples * sample_size;
    sw_src_config.mode = SW_SRC_MODE_NORMAL;
    sw_src_config.channel_num = 2;
    sw_src_config.in_res = stream_resolution;
    sw_src_config.in_sampling_rate  = dongle_handle->source_info.bt_in.sample_rate;
    sw_src_config.in_frame_size_max = dongle_handle->source_info.bt_in.sample_rate/1000*2*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    sw_src_config.out_res           = stream_resolution;
    sw_src_config.out_sampling_rate = dongle_handle->sink_info.usb_out.sample_rate;
    sw_src_config.out_frame_size_max= dongle_handle->sink_info.usb_out.sample_rate/1000*2*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    dongle_handle->src_port = stream_function_sw_src_get_port(dongle_handle->source);
    stream_function_sw_src_init(dongle_handle->src_port, &sw_src_config);
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw src 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 16,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->src_port,
                sw_src_config.mode,
                sw_src_config.channel_num,
                sw_src_config.in_res,
                sw_src_config.in_sampling_rate,
                sw_src_config.in_frame_size_max,
                sw_src_config.out_res,
                sw_src_config.out_sampling_rate,
                sw_src_config.out_frame_size_max,
                dongle_handle->src_in_frame_samples,
                dongle_handle->src_in_frame_size,
                dongle_handle->src_out_frame_samples,
                dongle_handle->src_out_frame_size);
}

static void ull_audio_v2_dongle_ul_usb_out_deinit(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    /* bt in info deinit */
    ull_audio_v2_dongle_ul_bt_in_deinit(dongle_handle);

    /* codec deinit */
    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        stream_codec_decoder_lc3plus_deinit(LC3PLUS_DEC_PORT_0, dongle_handle->source);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
        stream_codec_decoder_celt_v2_deinit(CELT_DEC_PORT_0, dongle_handle->source);
#endif /* AIR_CELT_DEC_V2_ENABLE */
    }

    /* sw mixer deinit */
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);

    /* sw clk skew deinit */
    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);

    /* sw buffer deinit */
    stream_function_sw_buffer_deinit(dongle_handle->buffer_port);
//#ifdef AIR_ECNR_POST_PART_ENABLE
    if (g_nr_offlad_status == true) {
        stream_function_sw_buffer_deinit(dongle_handle->ecnr_in_buffer_port);
        stream_function_sw_buffer_deinit(dongle_handle->ecnr_out_buffer_port);
        stream_function_sw_buffer_list_deinit(dongle_handle->ecnr_buffer_list);
    }
//#endif /* AIR_ECNR_POST_PART_ENABLE */

    /* sw gain deinit */
    stream_function_sw_gain_deinit(dongle_handle->gain_port);

    /* sw src deinit */
    stream_function_sw_src_deinit(dongle_handle->src_port);
}

static int32_t ull_audio_v2_dongle_ul_usb_clock_skew_check(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    int32_t compensatory_samples = 0;
    uint32_t remain_samples_in_share_buffer;
    uint32_t remain_samples_in_sw_buffer;
    uint32_t frame_samples;
    int32_t frac_rpt;
    uint32_t sample_size;

    sample_size = (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) ? sizeof(int16_t) : sizeof(int32_t);

    if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED)
    {
        /* get remain samples */
        frame_samples = dongle_handle->source_info.bt_in.sample_rate/1000;
        remain_samples_in_share_buffer = dongle_handle->sink_info.usb_out.mcu_frame_count_latch*frame_samples;
        remain_samples_in_sw_buffer = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sample_size;

        if ((int32_t)(remain_samples_in_share_buffer+remain_samples_in_sw_buffer) > dongle_handle->clk_skew_watermark_samples)
        {
            if (dongle_handle->clk_skew_count > -UL_CLOCK_SKEW_CHECK_COUNT)
            {
                dongle_handle->clk_skew_count -= 1;
            }
        }
        else if ((int32_t)(remain_samples_in_share_buffer+remain_samples_in_sw_buffer) < dongle_handle->clk_skew_watermark_samples)
        {
            if (dongle_handle->clk_skew_count < UL_CLOCK_SKEW_CHECK_COUNT)
            {
                dongle_handle->clk_skew_count += 1;
            }
        }
        else
        {
            // if ((dongle_handle->clk_skew_count < UL_CLOCK_SKEW_CHECK_COUNT) && (dongle_handle->clk_skew_count > 0))
            // {
            //     dongle_handle->clk_skew_count -= 1;
            // }
            // else if ((dongle_handle->clk_skew_count > UL_CLOCK_SKEW_CHECK_COUNT) && (dongle_handle->clk_skew_count < 0))
            // {
            //     dongle_handle->clk_skew_count += 1;
            // }
            dongle_handle->clk_skew_count = 0;
        }

        if (dongle_handle->clk_skew_count == UL_CLOCK_SKEW_CHECK_COUNT)
        {
            compensatory_samples = 1;
        }
        else if (dongle_handle->clk_skew_count == -UL_CLOCK_SKEW_CHECK_COUNT)
        {
            compensatory_samples = -1;
            stream_function_sw_clk_skew_get_frac_rpt(dongle_handle->clk_skew_port, 1, &frac_rpt);
            if (frac_rpt == -(dongle_handle->clk_skew_compensation_mode - 1))
            {
                /* in here clock skew will cut 1 sample data, so we need to make sure the buffer output size is N * 1ms */
                dongle_handle->buffer_output_size = (dongle_handle->buffer_default_output_size/sample_size + remain_samples_in_sw_buffer - 1)/frame_samples*frame_samples*sample_size;
            }
        }
        else
        {
            compensatory_samples = 0;
        }
    }

    return compensatory_samples;
}

static uint32_t ull_audio_v2_dongle_ul_sink_get_unprocess_frame_num(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t avail_size;
    uint32_t frame_num;
    SINK sink = dongle_handle->sink;

    audio_transmitter_share_information_fetch(NULL, sink);
    if (sink->streamBuffer.ShareBufferInfo.read_offset < sink->streamBuffer.ShareBufferInfo.write_offset)
    {
        /* normal case */
        avail_size = sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num
                    - sink->streamBuffer.ShareBufferInfo.write_offset
                    + sink->streamBuffer.ShareBufferInfo.read_offset;
    }
    else if (sink->streamBuffer.ShareBufferInfo.read_offset == sink->streamBuffer.ShareBufferInfo.write_offset)
    {
        if(sink->streamBuffer.ShareBufferInfo.bBufferIsFull == true)
        {
            /* buffer is full, so read_offset == write_offset */
            avail_size = 0;
        }
        else
        {
            /* buffer is empty, so read_offset == write_offset */
            avail_size = sink->streamBuffer.ShareBufferInfo.length;
        }
    }
    else
    {
        /* buffer wrapper case */
        avail_size = sink->streamBuffer.ShareBufferInfo.read_offset - sink->streamBuffer.ShareBufferInfo.write_offset;
    }

    avail_size = sink->streamBuffer.ShareBufferInfo.length - avail_size;
    frame_num = avail_size / sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;

    return frame_num;
}

/******************************************************************************/
/*          ULL audio 2.0 dongle UL line-out path Private Functions           */
/******************************************************************************/
#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
static void ull_audio_v2_dongle_ul_line_out_init(ull_audio_v2_dongle_ul_handle_t *dongle_handle, au_afe_open_param_p afe_open_param, bt_common_open_param_p bt_common_open_param)
{
    uint32_t i;
    uint32_t sample_size = 0;
    n9_dsp_share_info_ptr p_share_info;
    stream_resolution_t stream_resolution = RESOLUTION_16BIT;

    /* bt in info init */
    ull_audio_v2_dongle_ul_bt_in_init(dongle_handle, (audio_transmitter_open_param_p)afe_open_param, bt_common_open_param);
    if (dongle_handle->source_info.bt_in.frame_interval == 5000)
    {
        dongle_handle->source_info.bt_in.play_en_delay                    = UL_PLAYEN_DELAY_FRAME_5000US_LINE_OUT;
        dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay  = UL_FIRST_PACKET_SAFE_DELAY_FRAME_5000US_LINE_OUT;
        dongle_handle->source_info.bt_in.bt_retry_window                  = ull_audio_v2_bt_init_play_info.dl_retransmission_window_clk;
        dongle_handle->source_info.bt_in.bt_interval                      = UL_BT_INTERVAL_FRAME_5000US;
        dongle_handle->source_info.bt_in.bt_channel_anchor_diff           = UL_BT_INTERVAL_FRAME_5000US/2;
    }
    else
    {
        ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]frame_interval is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.frame_interval);
        AUDIO_ASSERT(0);
    }
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]bt in info, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", 15,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->source_info.bt_in.channel_num,
                dongle_handle->source_info.bt_in.sample_rate,
                dongle_handle->source_info.bt_in.sample_format,
                dongle_handle->source_info.bt_in.frame_size,
                dongle_handle->source_info.bt_in.frame_samples,
                dongle_handle->source_info.bt_in.frame_interval,
                dongle_handle->source_info.bt_in.bit_rate,
                dongle_handle->source_info.bt_in.play_en_delay,
                dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay,
                dongle_handle->source_info.bt_in.bt_retry_window,
                dongle_handle->source_info.bt_in.bt_interval,
                dongle_handle->source_info.bt_in.bt_channel_anchor_diff);
    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->source_info.bt_in.bt_info[i] != NULL)
        {
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.share_info);
            ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]bt in channel %u info, %u, %u, %u, %u, %u, 0x%x, 0x%x\r\n", 11,
                        bt_common_open_param->scenario_type,
                        bt_common_open_param->scenario_sub_id,
                        dongle_handle,
                        i+1,
                        (dongle_handle->source_info.bt_in.bt_info[i])->seq_num,
                        (dongle_handle->source_info.bt_in.bt_info[i])->user_count,
                        (dongle_handle->source_info.bt_in.bt_info[i])->blk_index,
                        (dongle_handle->source_info.bt_in.bt_info[i])->blk_index_previous,
                        (dongle_handle->source_info.bt_in.bt_info[i])->crc32_init,
                        p_share_info,
                        hal_memview_infrasys_to_dsp0(p_share_info->start_addr));
        }
    }

    /* line out info init */
    dongle_handle->sink_info.line_out.channel_num       = 2;
    dongle_handle->sink_info.line_out.sample_rate       = afe_open_param->stream_out_sampling_rate;
    dongle_handle->sink_info.line_out.sample_format     = afe_open_param->format;
    dongle_handle->sink_info.line_out.frame_samples     = afe_open_param->frame_size;
    if (dongle_handle->sink_info.line_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
    {
        sample_size = sizeof(uint16_t);
    }
    else
    {
        sample_size = sizeof(uint32_t);
    }
    dongle_handle->sink_info.line_out.frame_size        = dongle_handle->sink_info.line_out.frame_samples*sample_size;
    dongle_handle->sink_info.line_out.frame_interval    = dongle_handle->source_info.bt_in.frame_interval;
    dongle_handle->sink_info.line_out.frame_max_num     = (dongle_handle->source_info.bt_in.frame_interval) / 1000;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]line out info, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->sink_info.line_out.channel_num,
                dongle_handle->sink_info.line_out.sample_rate,
                dongle_handle->sink_info.line_out.sample_format,
                dongle_handle->sink_info.line_out.frame_size,
                dongle_handle->sink_info.line_out.frame_samples,
                dongle_handle->sink_info.line_out.frame_interval,
                dongle_handle->sink_info.line_out.frame_max_num);

    /* dummy state machine for source_readbuf() */
    dongle_handle->buffer_default_output_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;

    /* codec init */
    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_out[];
        stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_line_out, CODEC_DECODER_LC3PLUS, CONFIG_DECODER);

        lc3plus_dec_port_config_t lc3plus_dec_config;
        if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            lc3plus_dec_config.sample_bits  = 16;
            sample_size = sizeof(uint16_t);
            stream_resolution = RESOLUTION_16BIT;
        }
        else if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)
        {
            lc3plus_dec_config.sample_bits  = 24;
            sample_size = sizeof(uint32_t);
            stream_resolution = RESOLUTION_32BIT;
        }
        else
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_format is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.sample_format);
            AUDIO_ASSERT(0);
        }
        lc3plus_dec_config.sample_rate      = dongle_handle->source_info.bt_in.sample_rate;
        lc3plus_dec_config.bit_rate         = dongle_handle->source_info.bt_in.bit_rate;
        lc3plus_dec_config.channel_mode     = LC3PLUS_DEC_CHANNEL_MODE_MULTI_MONO;
        lc3plus_dec_config.in_channel_num   = dongle_handle->source_info.bt_in.channel_num;
        lc3plus_dec_config.out_channel_num  = dongle_handle->source_info.bt_in.channel_num;
        lc3plus_dec_config.frame_interval   = dongle_handle->source_info.bt_in.frame_interval;
        lc3plus_dec_config.frame_size       = dongle_handle->source_info.bt_in.frame_size;
        lc3plus_dec_config.frame_samples    = dongle_handle->source_info.bt_in.frame_samples;
        lc3plus_dec_config.plc_enable       = 1;
        lc3plus_dec_config.plc_method       = (g_plc_mode == 0)? LC3PLUS_PLCMETH_STD : LC3PLUS_PLCMETH_ADV_TDC_NS;
        stream_codec_decoder_lc3plus_init(LC3PLUS_DEC_PORT_0, dongle_handle->source, &lc3plus_dec_config);
        ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]lc3plus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                    bt_common_open_param->scenario_type,
                    bt_common_open_param->scenario_sub_id,
                    dongle_handle,
                    lc3plus_dec_config.sample_bits,
                    lc3plus_dec_config.sample_rate,
                    lc3plus_dec_config.bit_rate,
                    lc3plus_dec_config.channel_mode,
                    lc3plus_dec_config.in_channel_num,
                    lc3plus_dec_config.out_channel_num,
                    lc3plus_dec_config.frame_interval,
                    lc3plus_dec_config.frame_samples,
                    lc3plus_dec_config.frame_size);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
        extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_out[];
        stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_line_out, CODEC_DECODER_OPUS_V2, CONFIG_DECODER);

        celt_dec_port_config_t celt_dec_config;
        if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            celt_dec_config.sample_bits  = 16;
            sample_size = sizeof(uint16_t);
            stream_resolution = RESOLUTION_16BIT;
        }
        else
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_format is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.sample_format);
            AUDIO_ASSERT(0);
        }
        celt_dec_config.sample_rate      = dongle_handle->source_info.bt_in.sample_rate;
        celt_dec_config.bit_rate         = dongle_handle->source_info.bt_in.bit_rate;
        celt_dec_config.channel_mode     = CELT_DEC_CHANNEL_MODE_MULTI_MONO;
        celt_dec_config.in_channel_num   = dongle_handle->source_info.bt_in.channel_num;
        celt_dec_config.out_channel_num  = dongle_handle->source_info.bt_in.channel_num;
        celt_dec_config.frame_interval   = dongle_handle->source_info.bt_in.frame_interval;
        celt_dec_config.frame_size       = dongle_handle->source_info.bt_in.frame_size;
        celt_dec_config.frame_samples    = dongle_handle->source_info.bt_in.frame_samples;
        celt_dec_config.plc_enable       = 1;
        stream_codec_decoder_celt_v2_init(CELT_DEC_PORT_0, dongle_handle->source, &celt_dec_config);
        ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]opus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                    bt_common_open_param->scenario_type,
                    bt_common_open_param->scenario_sub_id,
                    dongle_handle,
                    celt_dec_config.sample_bits,
                    celt_dec_config.sample_rate,
                    celt_dec_config.bit_rate,
                    celt_dec_config.channel_mode,
                    celt_dec_config.in_channel_num,
                    celt_dec_config.out_channel_num,
                    celt_dec_config.frame_interval,
                    celt_dec_config.frame_samples,
                    celt_dec_config.frame_size);
#endif /* AIR_CELT_DEC_V2_ENABLE */
    }

    /* sw mxier init */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback = ull_audio_v2_dongle_ul_mixer_precallback;
    callback_config.postprocess_callback = NULL;
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = stream_resolution;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_OVERWRITE;
    in_ch_config.buffer_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    out_ch_config.total_channel_number = 2;
    out_ch_config.resolution = stream_resolution;
    dongle_handle->mixer_member= stream_function_sw_mixer_member_create((void *)dongle_handle->source, SW_MIXER_MEMBER_MODE_NO_BYPASS, &callback_config, &in_ch_config, &out_ch_config);
    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, false);
    ull_audio_v2_dongle_ul_mixer_all_stream_connections_update(dongle_handle, true, 0x0101);
    dongle_handle->mixer_connection_status = 0x0101;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw mixer 0x%x info, %d, %d, %d, %d, %d, %d, 0x%x\r\n", 11,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->mixer_member,
                in_ch_config.total_channel_number,
                in_ch_config.resolution,
                in_ch_config.input_mode,
                in_ch_config.buffer_size,
                out_ch_config.total_channel_number,
                out_ch_config.resolution,
                dongle_handle->mixer_connection_status);

    /* sw gain init */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution = stream_resolution;
    default_config.target_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    default_config.up_step = 25;
    default_config.up_samples_per_step = dongle_handle->sink_info.line_out.sample_rate/1000;
    default_config.down_step = -25;
    default_config.down_samples_per_step = dongle_handle->sink_info.line_out.sample_rate/1000;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(dongle_handle->source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    default_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw gain 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->gain_port,
                default_config.resolution,
                default_config.target_gain,
                default_config.up_step,
                default_config.up_samples_per_step,
                default_config.down_step,
                default_config.down_samples_per_step,
                bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L,
                bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R);
}

static void ull_audio_v2_dongle_ul_line_out_deinit(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    /* bt in info deinit */
    ull_audio_v2_dongle_ul_bt_in_deinit(dongle_handle);

    /* codec deinit */
    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        stream_codec_decoder_lc3plus_deinit(LC3PLUS_DEC_PORT_0, dongle_handle->source);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
        stream_codec_decoder_celt_v2_deinit(CELT_DEC_PORT_0, dongle_handle->source);
#endif /* AIR_CELT_DEC_V2_ENABLE */
    }

    /* sw mixer deinit */
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);

    /* sw gain deinit */
    stream_function_sw_gain_deinit(dongle_handle->gain_port);
}

static void ull_audio_v2_dongle_ul_line_out_start(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t gpt_count;
    uint32_t sample_size;
    hal_audio_memory_parameter_t *mem_handle = &dongle_handle->sink->param.audio.mem_handle;
    hal_audio_agent_t agent;

    /* get agent */
    agent = hal_memory_convert_agent(mem_handle->memory_select);
    /* disable AFE irq here */
    hal_memory_set_irq_enable(agent, HAL_AUDIO_CONTROL_OFF);

    /* set agent regsiters' address */
    switch (agent) {
        case HAL_AUDIO_AGENT_MEMORY_DL1:
            dongle_handle->sink_info.line_out.afe_cur_addr  = AFE_DL1_CUR;
            dongle_handle->sink_info.line_out.afe_base_addr = AFE_DL1_BASE;
            break;

        case HAL_AUDIO_AGENT_MEMORY_DL2:
            dongle_handle->sink_info.line_out.afe_cur_addr  = AFE_DL2_CUR;
            dongle_handle->sink_info.line_out.afe_base_addr = AFE_DL2_BASE;
            break;

        case HAL_AUDIO_AGENT_MEMORY_DL3:
            dongle_handle->sink_info.line_out.afe_cur_addr  = AFE_DL3_CUR;
            dongle_handle->sink_info.line_out.afe_base_addr = AFE_DL3_BASE;
            break;

        case HAL_AUDIO_AGENT_MEMORY_DL12:
            dongle_handle->sink_info.line_out.afe_cur_addr  = AFE_DL12_CUR;
            dongle_handle->sink_info.line_out.afe_base_addr = AFE_DL12_BASE;
            break;

        default:
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR][handle 0x%x][scenario %d-%d]line-out stream start, unknow agent = 0x%x", 4,
                dongle_handle,
                dongle_handle->source->param.bt_common.scenario_type,
                dongle_handle->source->param.bt_common.scenario_sub_id,
                agent);
            AUDIO_ASSERT(0);
    }

    /* set 3ms prefill size for process time 3ms */
    if (dongle_handle->sink_info.line_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
    {
        sample_size = sizeof(uint16_t);
    }
    else
    {
        sample_size = sizeof(uint32_t);
    }
    dongle_handle->sink->streamBuffer.BufferInfo.WriteOffset = sample_size*dongle_handle->sink_info.line_out.channel_num*(UL_PROCESS_TIME_FRAME*dongle_handle->sink_info.line_out.sample_rate/1000);
    dongle_handle->sink->streamBuffer.BufferInfo.ReadOffset  = 0;

    /* start 1ms timer for trigger stream */
    ull_audio_v2_dongle_ul_software_timer_start(dongle_handle);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][scenario %d-%d]line-out stream start, gpt count = 0x%x, start_addr = 0x%x, write_offset = 0x%x, read_offset = 0x%x, length = 0x%x, cur_addr = 0x%x, base_addr = 0x%x, cur_ro = 0x%x, cur_base = 0x%x", 12,
                dongle_handle,
                dongle_handle->source->param.bt_common.scenario_type,
                dongle_handle->source->param.bt_common.scenario_sub_id,
                gpt_count,
                dongle_handle->sink->streamBuffer.BufferInfo.startaddr[0],
                dongle_handle->sink->streamBuffer.BufferInfo.WriteOffset,
                dongle_handle->sink->streamBuffer.BufferInfo.ReadOffset,
                dongle_handle->sink->streamBuffer.BufferInfo.length,
                dongle_handle->sink_info.line_out.afe_cur_addr,
                dongle_handle->sink_info.line_out.afe_base_addr,
                AFE_GET_REG(dongle_handle->sink_info.line_out.afe_cur_addr),
                AFE_GET_REG(dongle_handle->sink_info.line_out.afe_base_addr));
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_ul_line_out_stop(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t gpt_count;
    uint32_t saved_mask;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_START)
    {
        /* stop timer if the stream is not running */
        ull_audio_v2_dongle_ul_software_timer_stop(dongle_handle);
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][scenario %d-%d]line-out stream start, gpt count = 0x%x", 4,
                dongle_handle,
                dongle_handle->source->param.bt_common.scenario_type,
                dongle_handle->source->param.bt_common.scenario_sub_id,
                gpt_count);
}

static int32_t ull_audio_v2_dongle_ul_line_out_clock_skew_check(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    int32_t compensatory_samples = 0;
    UNUSED(dongle_handle);

    return compensatory_samples;
}
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
static void ull_audio_v2_dongle_ul_i2s_mst_out_init(ull_audio_v2_dongle_ul_handle_t *dongle_handle, au_afe_open_param_p afe_open_param, bt_common_open_param_p bt_common_open_param)
{
    uint32_t i;
    uint32_t sample_size = 0;
    n9_dsp_share_info_ptr p_share_info;
    stream_resolution_t stream_resolution = RESOLUTION_16BIT;

    /* bt in info init */
    ull_audio_v2_dongle_ul_bt_in_init(dongle_handle, (audio_transmitter_open_param_p)afe_open_param, bt_common_open_param);
    if (dongle_handle->source_info.bt_in.frame_interval == 5000)
    {
        dongle_handle->source_info.bt_in.play_en_delay                    = UL_PLAYEN_DELAY_FRAME_5000US_LINE_OUT;
        dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay  = UL_FIRST_PACKET_SAFE_DELAY_FRAME_5000US_LINE_OUT;
        dongle_handle->source_info.bt_in.bt_retry_window                  = ull_audio_v2_bt_init_play_info.dl_retransmission_window_clk;
        dongle_handle->source_info.bt_in.bt_interval                      = UL_BT_INTERVAL_FRAME_5000US;
        dongle_handle->source_info.bt_in.bt_channel_anchor_diff           = UL_BT_INTERVAL_FRAME_5000US/2;
    }
    else
    {
        ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]frame_interval is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.frame_interval);
        AUDIO_ASSERT(0);
    }
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]bt in info, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", 15,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->source_info.bt_in.channel_num,
                dongle_handle->source_info.bt_in.sample_rate,
                dongle_handle->source_info.bt_in.sample_format,
                dongle_handle->source_info.bt_in.frame_size,
                dongle_handle->source_info.bt_in.frame_samples,
                dongle_handle->source_info.bt_in.frame_interval,
                dongle_handle->source_info.bt_in.bit_rate,
                dongle_handle->source_info.bt_in.play_en_delay,
                dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay,
                dongle_handle->source_info.bt_in.bt_retry_window,
                dongle_handle->source_info.bt_in.bt_interval,
                dongle_handle->source_info.bt_in.bt_channel_anchor_diff);
    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->source_info.bt_in.bt_info[i] != NULL)
        {
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.share_info);
            ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]bt in channel %u info, %u, %u, %u, %u, %u, 0x%x, 0x%x\r\n", 11,
                        bt_common_open_param->scenario_type,
                        bt_common_open_param->scenario_sub_id,
                        dongle_handle,
                        i+1,
                        (dongle_handle->source_info.bt_in.bt_info[i])->seq_num,
                        (dongle_handle->source_info.bt_in.bt_info[i])->user_count,
                        (dongle_handle->source_info.bt_in.bt_info[i])->blk_index,
                        (dongle_handle->source_info.bt_in.bt_info[i])->blk_index_previous,
                        (dongle_handle->source_info.bt_in.bt_info[i])->crc32_init,
                        p_share_info,
                        hal_memview_infrasys_to_dsp0(p_share_info->start_addr));
        }
    }

    /* i2s mst out info init */
    dongle_handle->sink_info.i2s_mst_out.channel_num       = 2;
    dongle_handle->sink_info.i2s_mst_out.sample_rate       = afe_open_param->stream_out_sampling_rate;
    dongle_handle->sink_info.i2s_mst_out.sample_format     = afe_open_param->format;
    dongle_handle->sink_info.i2s_mst_out.frame_samples     = afe_open_param->frame_size;
    if (dongle_handle->sink_info.i2s_mst_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
    {
        sample_size = sizeof(uint16_t);
    }
    else
    {
        sample_size = sizeof(uint32_t);
    }
    dongle_handle->sink_info.i2s_mst_out.frame_size        = dongle_handle->sink_info.i2s_mst_out.frame_samples*sample_size;
    dongle_handle->sink_info.i2s_mst_out.frame_interval    = dongle_handle->source_info.bt_in.frame_interval;
    dongle_handle->sink_info.i2s_mst_out.frame_max_num     = (dongle_handle->source_info.bt_in.frame_interval) / 1000;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]i2s mst out info, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->sink_info.i2s_mst_out.channel_num,
                dongle_handle->sink_info.i2s_mst_out.sample_rate,
                dongle_handle->sink_info.i2s_mst_out.sample_format,
                dongle_handle->sink_info.i2s_mst_out.frame_size,
                dongle_handle->sink_info.i2s_mst_out.frame_samples,
                dongle_handle->sink_info.i2s_mst_out.frame_interval,
                dongle_handle->sink_info.i2s_mst_out.frame_max_num);

    /* dummy state machine for source_readbuf() */
    dongle_handle->buffer_default_output_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;

    /* codec init */
    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_out[];
        stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_line_out, CODEC_DECODER_LC3PLUS, CONFIG_DECODER);

        lc3plus_dec_port_config_t lc3plus_dec_config;
        if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            lc3plus_dec_config.sample_bits  = 16;
            sample_size = sizeof(uint16_t);
            stream_resolution = RESOLUTION_16BIT;
        }
        else if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)
        {
            lc3plus_dec_config.sample_bits  = 24;
            sample_size = sizeof(uint32_t);
            stream_resolution = RESOLUTION_32BIT;
        }
        else
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_format is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.sample_format);
            AUDIO_ASSERT(0);
        }
        lc3plus_dec_config.sample_rate      = dongle_handle->source_info.bt_in.sample_rate;
        lc3plus_dec_config.bit_rate         = dongle_handle->source_info.bt_in.bit_rate;
        lc3plus_dec_config.channel_mode     = LC3PLUS_DEC_CHANNEL_MODE_MULTI_MONO;
        lc3plus_dec_config.in_channel_num   = dongle_handle->source_info.bt_in.channel_num;
        lc3plus_dec_config.out_channel_num  = dongle_handle->source_info.bt_in.channel_num;
        lc3plus_dec_config.frame_interval   = dongle_handle->source_info.bt_in.frame_interval;
        lc3plus_dec_config.frame_size       = dongle_handle->source_info.bt_in.frame_size;
        lc3plus_dec_config.frame_samples    = dongle_handle->source_info.bt_in.frame_samples;
        lc3plus_dec_config.plc_enable       = 1;
        lc3plus_dec_config.plc_method       = (g_plc_mode == 0)? LC3PLUS_PLCMETH_STD : LC3PLUS_PLCMETH_ADV_TDC_NS;
        stream_codec_decoder_lc3plus_init(LC3PLUS_DEC_PORT_0, dongle_handle->source, &lc3plus_dec_config);
        ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]lc3plus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                    bt_common_open_param->scenario_type,
                    bt_common_open_param->scenario_sub_id,
                    dongle_handle,
                    lc3plus_dec_config.sample_bits,
                    lc3plus_dec_config.sample_rate,
                    lc3plus_dec_config.bit_rate,
                    lc3plus_dec_config.channel_mode,
                    lc3plus_dec_config.in_channel_num,
                    lc3plus_dec_config.out_channel_num,
                    lc3plus_dec_config.frame_interval,
                    lc3plus_dec_config.frame_samples,
                    lc3plus_dec_config.frame_size);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
        extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_out[];
        stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_line_out, CODEC_DECODER_OPUS_V2, CONFIG_DECODER);

        celt_dec_port_config_t celt_dec_config;
        if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            celt_dec_config.sample_bits  = 16;
            sample_size = sizeof(uint16_t);
            stream_resolution = RESOLUTION_16BIT;
        }
        else
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_format is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.sample_format);
            AUDIO_ASSERT(0);
        }
        celt_dec_config.sample_rate      = dongle_handle->source_info.bt_in.sample_rate;
        celt_dec_config.bit_rate         = dongle_handle->source_info.bt_in.bit_rate;
        celt_dec_config.channel_mode     = CELT_DEC_CHANNEL_MODE_MULTI_MONO;
        celt_dec_config.in_channel_num   = dongle_handle->source_info.bt_in.channel_num;
        celt_dec_config.out_channel_num  = dongle_handle->source_info.bt_in.channel_num;
        celt_dec_config.frame_interval   = dongle_handle->source_info.bt_in.frame_interval;
        celt_dec_config.frame_size       = dongle_handle->source_info.bt_in.frame_size;
        celt_dec_config.frame_samples    = dongle_handle->source_info.bt_in.frame_samples;
        celt_dec_config.plc_enable       = 1;
        stream_codec_decoder_celt_v2_init(CELT_DEC_PORT_0, dongle_handle->source, &celt_dec_config);
        ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]opus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                    bt_common_open_param->scenario_type,
                    bt_common_open_param->scenario_sub_id,
                    dongle_handle,
                    celt_dec_config.sample_bits,
                    celt_dec_config.sample_rate,
                    celt_dec_config.bit_rate,
                    celt_dec_config.channel_mode,
                    celt_dec_config.in_channel_num,
                    celt_dec_config.out_channel_num,
                    celt_dec_config.frame_interval,
                    celt_dec_config.frame_samples,
                    celt_dec_config.frame_size);
#endif /* AIR_CELT_DEC_V2_ENABLE */
    }

    /* sw mxier init */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback = ull_audio_v2_dongle_ul_mixer_precallback;
    callback_config.postprocess_callback = NULL;
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = stream_resolution;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_OVERWRITE;
    in_ch_config.buffer_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    out_ch_config.total_channel_number = 2;
    out_ch_config.resolution = stream_resolution;
    dongle_handle->mixer_member= stream_function_sw_mixer_member_create((void *)dongle_handle->source, SW_MIXER_MEMBER_MODE_NO_BYPASS, &callback_config, &in_ch_config, &out_ch_config);
    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, false);
    ull_audio_v2_dongle_ul_mixer_all_stream_connections_update(dongle_handle, true, 0x0101);
    dongle_handle->mixer_connection_status = 0x0101;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw mixer 0x%x info, %d, %d, %d, %d, %d, %d, 0x%x\r\n", 11,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->mixer_member,
                in_ch_config.total_channel_number,
                in_ch_config.resolution,
                in_ch_config.input_mode,
                in_ch_config.buffer_size,
                out_ch_config.total_channel_number,
                out_ch_config.resolution,
                dongle_handle->mixer_connection_status);

    /* sw gain init */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution = stream_resolution;
    default_config.target_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    default_config.up_step = 25;
    default_config.up_samples_per_step = dongle_handle->sink_info.i2s_mst_out.sample_rate/1000;
    default_config.down_step = -25;
    default_config.down_samples_per_step = dongle_handle->sink_info.i2s_mst_out.sample_rate/1000;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(dongle_handle->source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    default_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw gain 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->gain_port,
                default_config.resolution,
                default_config.target_gain,
                default_config.up_step,
                default_config.up_samples_per_step,
                default_config.down_step,
                default_config.down_samples_per_step,
                bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L,
                bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R);

    /* sw src init */
    sw_src_config_t sw_src_config;
    dongle_handle->src_in_frame_samples     = dongle_handle->source_info.bt_in.sample_rate/1000;
    dongle_handle->src_in_frame_size        = dongle_handle->src_in_frame_samples * sample_size;
    dongle_handle->src_out_frame_samples    = dongle_handle->sink_info.usb_out.sample_rate/1000;
    dongle_handle->src_out_frame_size       = dongle_handle->src_out_frame_samples * sample_size;
    sw_src_config.mode = SW_SRC_MODE_NORMAL;
    sw_src_config.channel_num = 2;
    sw_src_config.in_res = stream_resolution;
    sw_src_config.in_sampling_rate  = dongle_handle->source_info.bt_in.sample_rate;
    sw_src_config.in_frame_size_max = dongle_handle->source_info.bt_in.sample_rate/1000*2*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    sw_src_config.out_res           = stream_resolution;
    sw_src_config.out_sampling_rate = dongle_handle->sink_info.usb_out.sample_rate;
    sw_src_config.out_frame_size_max= dongle_handle->sink_info.usb_out.sample_rate/1000*2*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    dongle_handle->src_port = stream_function_sw_src_get_port(dongle_handle->source);
    stream_function_sw_src_init(dongle_handle->src_port, &sw_src_config);
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw src 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 16,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->src_port,
                sw_src_config.mode,
                sw_src_config.channel_num,
                sw_src_config.in_res,
                sw_src_config.in_sampling_rate,
                sw_src_config.in_frame_size_max,
                sw_src_config.out_res,
                sw_src_config.out_sampling_rate,
                sw_src_config.out_frame_size_max,
                dongle_handle->src_in_frame_samples,
                dongle_handle->src_in_frame_size,
                dongle_handle->src_out_frame_samples,
                dongle_handle->src_out_frame_size);
}

static void ull_audio_v2_dongle_ul_i2s_mst_out_deinit(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    /* bt in info deinit */
    ull_audio_v2_dongle_ul_bt_in_deinit(dongle_handle);

    /* codec deinit */
    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        stream_codec_decoder_lc3plus_deinit(LC3PLUS_DEC_PORT_0, dongle_handle->source);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
        stream_codec_decoder_celt_v2_deinit(CELT_DEC_PORT_0, dongle_handle->source);
#endif /* AIR_CELT_DEC_V2_ENABLE */
    }

    /* sw mixer deinit */
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);

    /* sw gain deinit */
    stream_function_sw_gain_deinit(dongle_handle->gain_port);

    /* sw src deinit */
    stream_function_sw_src_deinit(dongle_handle->src_port);
}

static void ull_audio_v2_dongle_ul_i2s_mst_out_start(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t gpt_count;
    uint32_t sample_size;
    hal_audio_memory_parameter_t *mem_handle = &dongle_handle->sink->param.audio.mem_handle;
    hal_audio_agent_t agent;

    /* get agent */
    agent = hal_memory_convert_agent(mem_handle->memory_select);
    /* disable AFE irq here */
    hal_memory_set_irq_enable(agent, HAL_AUDIO_CONTROL_OFF);

    /* set agent regsiters' address */
    switch (agent) {
        case HAL_AUDIO_AGENT_MEMORY_DL1:
            dongle_handle->sink_info.i2s_mst_out.afe_cur_addr  = AFE_DL1_CUR;
            dongle_handle->sink_info.i2s_mst_out.afe_base_addr = AFE_DL1_BASE;
            break;

        case HAL_AUDIO_AGENT_MEMORY_DL2:
            dongle_handle->sink_info.i2s_mst_out.afe_cur_addr  = AFE_DL2_CUR;
            dongle_handle->sink_info.i2s_mst_out.afe_base_addr = AFE_DL2_BASE;
            break;

        case HAL_AUDIO_AGENT_MEMORY_DL3:
            dongle_handle->sink_info.i2s_mst_out.afe_cur_addr  = AFE_DL3_CUR;
            dongle_handle->sink_info.i2s_mst_out.afe_base_addr = AFE_DL3_BASE;
            break;

        case HAL_AUDIO_AGENT_MEMORY_DL12:
            dongle_handle->sink_info.i2s_mst_out.afe_cur_addr  = AFE_DL12_CUR;
            dongle_handle->sink_info.i2s_mst_out.afe_base_addr = AFE_DL12_BASE;
            break;

        default:
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR][handle 0x%x][scenario %d-%d]i2s_mst-out stream start, unknow agent = 0x%x", 4,
                dongle_handle,
                dongle_handle->source->param.bt_common.scenario_type,
                dongle_handle->source->param.bt_common.scenario_sub_id,
                agent);
            AUDIO_ASSERT(0);
    }

    /* set 3ms prefill size for process time 3ms */
    if (dongle_handle->sink_info.i2s_mst_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
    {
        sample_size = sizeof(uint16_t);
    }
    else
    {
        sample_size = sizeof(uint32_t);
    }
    dongle_handle->sink->streamBuffer.BufferInfo.WriteOffset = sample_size*dongle_handle->sink_info.i2s_mst_out.channel_num*(UL_PROCESS_TIME_FRAME*dongle_handle->sink_info.i2s_mst_out.sample_rate/1000);
    dongle_handle->sink->streamBuffer.BufferInfo.ReadOffset  = 0;

    /* start 1ms timer for trigger stream */
    ull_audio_v2_dongle_ul_software_timer_start(dongle_handle);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][scenario %d-%d]i2s_mst-out stream start, gpt count = 0x%x, start_addr = 0x%x, write_offset = 0x%x, read_offset = 0x%x, length = 0x%x, cur_addr = 0x%x, base_addr = 0x%x, cur_ro = 0x%x, cur_base = 0x%x", 12,
                dongle_handle,
                dongle_handle->source->param.bt_common.scenario_type,
                dongle_handle->source->param.bt_common.scenario_sub_id,
                gpt_count,
                dongle_handle->sink->streamBuffer.BufferInfo.startaddr[0],
                dongle_handle->sink->streamBuffer.BufferInfo.WriteOffset,
                dongle_handle->sink->streamBuffer.BufferInfo.ReadOffset,
                dongle_handle->sink->streamBuffer.BufferInfo.length,
                dongle_handle->sink_info.i2s_mst_out.afe_cur_addr,
                dongle_handle->sink_info.i2s_mst_out.afe_base_addr,
                AFE_GET_REG(dongle_handle->sink_info.i2s_mst_out.afe_cur_addr),
                AFE_GET_REG(dongle_handle->sink_info.i2s_mst_out.afe_base_addr));
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_ul_i2s_mst_out_stop(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t gpt_count;
    uint32_t saved_mask;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_START)
    {
        /* stop timer if the stream is not running */
        ull_audio_v2_dongle_ul_software_timer_stop(dongle_handle);
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][scenario %d-%d]i2s_mst-out stream start, gpt count = 0x%x", 4,
                dongle_handle,
                dongle_handle->source->param.bt_common.scenario_type,
                dongle_handle->source->param.bt_common.scenario_sub_id,
                gpt_count);
}

static int32_t ull_audio_v2_dongle_ul_i2s_mst_out_clock_skew_check(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    int32_t compensatory_samples = 0;
    UNUSED(dongle_handle);

    return compensatory_samples;
}
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

/******************************************************************************/
/*         ULL audio 2.0 dongle UL i2s-slv-out path Private Functions         */
/******************************************************************************/
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
EXTERN Stream_audio_Config_Ptr Audio_setting;
extern ATTR_TEXT_IN_IRAM void afe_dl12_interrupt_handler(void);
static int32_t i2s_dl12_counter = 0;
void audio_dongle_set_dl12_for_i2s_slave_out(ull_audio_v2_dongle_ul_handle_t *dongle_handle, bool control)
{
    /* set dummy agent dl12 for i2s slv tracking mode */
    if (((i2s_dl12_counter == 0) && (control)) || ((i2s_dl12_counter == 1) && (!control)))
    {
        hal_audio_memory_parameter_t mem_handle = {0};
        hal_audio_memory_parameter_t *dongle_mem_handle = &(dongle_handle->sink->param.audio.mem_handle);
        mem_handle.memory_select = HAL_AUDIO_MEMORY_DL_DL12;
        mem_handle.audio_path_rate = dongle_mem_handle->audio_path_rate;
        mem_handle.buffer_length = 0;
        mem_handle.irq_counter = dongle_mem_handle->irq_counter;
        mem_handle.pcm_format  = dongle_mem_handle->pcm_format;
        mem_handle.sync_status = HAL_AUDIO_MEMORY_SYNC_PLAY_EN;
        hal_audio_set_memory(&mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, control);
        /* register handler for dl12 */
        hal_audio_set_value_parameter_t handle;
        handle.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_AUDIOSYS;
        handle.register_irq_handler.memory_select = HAL_AUDIO_MEMORY_DL_DL12;
        handle.register_irq_handler.entry = afe_dl12_interrupt_handler;
        hal_audio_set_value(&handle, HAL_AUDIO_SET_IRQ_HANDLER);
        Audio_setting->Audio_sink.Zero_Padding_Cnt = 0; // avoid stream can't close
    }
    if (control) {
        i2s_dl12_counter ++;
    } else {
        i2s_dl12_counter --;
    }
    ULL_V2_LOG_I("[ULL Audio V2][UL] i2s_dl12_counter %d", 1, i2s_dl12_counter);
    AUDIO_ASSERT((i2s_dl12_counter >= 0) && "i2s dl12 counter error");
}

static void ull_audio_v2_dongle_ul_i2s_slv_out_init(ull_audio_v2_dongle_ul_handle_t *dongle_handle, au_afe_open_param_p afe_open_param, bt_common_open_param_p bt_common_open_param)
{
    uint32_t i;
    uint32_t sample_size = 0;
    n9_dsp_share_info_ptr p_share_info;
    stream_resolution_t stream_resolution = RESOLUTION_16BIT;

    /* bt in info init */
    ull_audio_v2_dongle_ul_bt_in_init(dongle_handle, (audio_transmitter_open_param_p)afe_open_param, bt_common_open_param);
    if (dongle_handle->source_info.bt_in.frame_interval == 5000)
    {
        dongle_handle->source_info.bt_in.play_en_delay                    = UL_PLAYEN_DELAY_FRAME_5000US_I2S_SLV_OUT;
        dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay  = UL_FIRST_PACKET_SAFE_DELAY_FRAME_5000US_I2S_SLV_OUT;
        dongle_handle->source_info.bt_in.bt_retry_window                  = ull_audio_v2_bt_init_play_info.dl_retransmission_window_clk;
        dongle_handle->source_info.bt_in.bt_interval                      = UL_BT_INTERVAL_FRAME_5000US;
        dongle_handle->source_info.bt_in.bt_channel_anchor_diff           = UL_BT_INTERVAL_FRAME_5000US/2;
    }
    else
    {
        ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]frame_interval is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.frame_interval);
        AUDIO_ASSERT(0);
    }
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]bt in info, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", 15,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->source_info.bt_in.channel_num,
                dongle_handle->source_info.bt_in.sample_rate,
                dongle_handle->source_info.bt_in.sample_format,
                dongle_handle->source_info.bt_in.frame_size,
                dongle_handle->source_info.bt_in.frame_samples,
                dongle_handle->source_info.bt_in.frame_interval,
                dongle_handle->source_info.bt_in.bit_rate,
                dongle_handle->source_info.bt_in.play_en_delay,
                dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay,
                dongle_handle->source_info.bt_in.bt_retry_window,
                dongle_handle->source_info.bt_in.bt_interval,
                dongle_handle->source_info.bt_in.bt_channel_anchor_diff);
    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->source_info.bt_in.bt_info[i] != NULL)
        {
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.share_info);
            ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]bt in channel %u info, %u, %u, %u, %u, %u, 0x%x, 0x%x\r\n", 11,
                        bt_common_open_param->scenario_type,
                        bt_common_open_param->scenario_sub_id,
                        dongle_handle,
                        i+1,
                        (dongle_handle->source_info.bt_in.bt_info[i])->seq_num,
                        (dongle_handle->source_info.bt_in.bt_info[i])->user_count,
                        (dongle_handle->source_info.bt_in.bt_info[i])->blk_index,
                        (dongle_handle->source_info.bt_in.bt_info[i])->blk_index_previous,
                        (dongle_handle->source_info.bt_in.bt_info[i])->crc32_init,
                        p_share_info,
                        hal_memview_infrasys_to_dsp0(p_share_info->start_addr));
        }
    }

    /* i2s_slv out info init */
    dongle_handle->sink_info.i2s_slv_out.channel_num       = 2;
    dongle_handle->sink_info.i2s_slv_out.sample_rate       = afe_open_param->stream_out_sampling_rate;
    dongle_handle->sink_info.i2s_slv_out.sample_format     = afe_open_param->format;
    dongle_handle->sink_info.i2s_slv_out.frame_samples     = afe_open_param->frame_size;
    if (dongle_handle->sink_info.i2s_slv_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
    {
        sample_size = sizeof(uint16_t);
    }
    else
    {
        sample_size = sizeof(uint32_t);
    }
    dongle_handle->sink_info.i2s_slv_out.frame_size        = dongle_handle->sink_info.i2s_slv_out.frame_samples*sample_size;
    dongle_handle->sink_info.i2s_slv_out.frame_interval    = dongle_handle->source_info.bt_in.frame_interval;
    dongle_handle->sink_info.i2s_slv_out.frame_max_num     = (dongle_handle->source_info.bt_in.frame_interval) / 1000;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]i2s_slv out info, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->sink_info.i2s_slv_out.channel_num,
                dongle_handle->sink_info.i2s_slv_out.sample_rate,
                dongle_handle->sink_info.i2s_slv_out.sample_format,
                dongle_handle->sink_info.i2s_slv_out.frame_size,
                dongle_handle->sink_info.i2s_slv_out.frame_samples,
                dongle_handle->sink_info.i2s_slv_out.frame_interval,
                dongle_handle->sink_info.i2s_slv_out.frame_max_num);

    /* dummy state machine for source_readbuf() */
    dongle_handle->buffer_default_output_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;

    /* codec init */
    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_i2s_slv_out[];
        stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_i2s_slv_out, CODEC_DECODER_LC3PLUS, CONFIG_DECODER);

        lc3plus_dec_port_config_t lc3plus_dec_config;
        if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            lc3plus_dec_config.sample_bits  = 16;
            sample_size = sizeof(uint16_t);
            stream_resolution = RESOLUTION_16BIT;
        }
        else if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)
        {
            lc3plus_dec_config.sample_bits  = 24;
            sample_size = sizeof(uint32_t);
            stream_resolution = RESOLUTION_32BIT;
        }
        else
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_format is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.sample_format);
            AUDIO_ASSERT(0);
        }
        lc3plus_dec_config.sample_rate      = dongle_handle->source_info.bt_in.sample_rate;
        lc3plus_dec_config.bit_rate         = dongle_handle->source_info.bt_in.bit_rate;
        lc3plus_dec_config.channel_mode     = LC3PLUS_DEC_CHANNEL_MODE_MULTI_MONO;
        lc3plus_dec_config.in_channel_num   = dongle_handle->source_info.bt_in.channel_num;
        lc3plus_dec_config.out_channel_num  = dongle_handle->source_info.bt_in.channel_num;
        lc3plus_dec_config.frame_interval   = dongle_handle->source_info.bt_in.frame_interval;
        lc3plus_dec_config.frame_size       = dongle_handle->source_info.bt_in.frame_size;
        lc3plus_dec_config.frame_samples    = dongle_handle->source_info.bt_in.frame_samples;
        lc3plus_dec_config.plc_enable       = 1;
        lc3plus_dec_config.plc_method       = (g_plc_mode == 0)? LC3PLUS_PLCMETH_STD : LC3PLUS_PLCMETH_ADV_TDC_NS;
        stream_codec_decoder_lc3plus_init(LC3PLUS_DEC_PORT_0, dongle_handle->source, &lc3plus_dec_config);
        ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]lc3plus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                    bt_common_open_param->scenario_type,
                    bt_common_open_param->scenario_sub_id,
                    dongle_handle,
                    lc3plus_dec_config.sample_bits,
                    lc3plus_dec_config.sample_rate,
                    lc3plus_dec_config.bit_rate,
                    lc3plus_dec_config.channel_mode,
                    lc3plus_dec_config.in_channel_num,
                    lc3plus_dec_config.out_channel_num,
                    lc3plus_dec_config.frame_interval,
                    lc3plus_dec_config.frame_samples,
                    lc3plus_dec_config.frame_size);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
        extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_i2s_slv_out[];
        stream_feature_configure_type(stream_feature_list_ull_audio_v2_dongle_i2s_slv_out, CODEC_DECODER_OPUS_V2, CONFIG_DECODER);

        celt_dec_port_config_t celt_dec_config;
        if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            celt_dec_config.sample_bits  = 16;
            sample_size = sizeof(uint16_t);
            stream_resolution = RESOLUTION_16BIT;
        }
        else
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_format is not supported, %u\r\n", 1, dongle_handle->source_info.bt_in.sample_format);
            AUDIO_ASSERT(0);
        }
        celt_dec_config.sample_rate      = dongle_handle->source_info.bt_in.sample_rate;
        celt_dec_config.bit_rate         = dongle_handle->source_info.bt_in.bit_rate;
        celt_dec_config.channel_mode     = CELT_DEC_CHANNEL_MODE_MULTI_MONO;
        celt_dec_config.in_channel_num   = dongle_handle->source_info.bt_in.channel_num;
        celt_dec_config.out_channel_num  = dongle_handle->source_info.bt_in.channel_num;
        celt_dec_config.frame_interval   = dongle_handle->source_info.bt_in.frame_interval;
        celt_dec_config.frame_size       = dongle_handle->source_info.bt_in.frame_size;
        celt_dec_config.frame_samples    = dongle_handle->source_info.bt_in.frame_samples;
        celt_dec_config.plc_enable       = 1;
        stream_codec_decoder_celt_v2_init(CELT_DEC_PORT_0, dongle_handle->source, &celt_dec_config);
        ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]opus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                    bt_common_open_param->scenario_type,
                    bt_common_open_param->scenario_sub_id,
                    dongle_handle,
                    celt_dec_config.sample_bits,
                    celt_dec_config.sample_rate,
                    celt_dec_config.bit_rate,
                    celt_dec_config.channel_mode,
                    celt_dec_config.in_channel_num,
                    celt_dec_config.out_channel_num,
                    celt_dec_config.frame_interval,
                    celt_dec_config.frame_samples,
                    celt_dec_config.frame_size);
#endif /* AIR_CELT_DEC_V2_ENABLE */
    }

    /* sw mxier init */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback = ull_audio_v2_dongle_ul_mixer_precallback;
    callback_config.postprocess_callback = NULL;
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = stream_resolution;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_OVERWRITE;
    in_ch_config.buffer_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    out_ch_config.total_channel_number = 2;
    out_ch_config.resolution = stream_resolution;
    dongle_handle->mixer_member= stream_function_sw_mixer_member_create((void *)dongle_handle->source, SW_MIXER_MEMBER_MODE_NO_BYPASS, &callback_config, &in_ch_config, &out_ch_config);
    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, false);
    ull_audio_v2_dongle_ul_mixer_all_stream_connections_update(dongle_handle, true, 0x0101);
    dongle_handle->mixer_connection_status = 0x0101;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw mixer 0x%x info, %d, %d, %d, %d, %d, %d, 0x%x\r\n", 11,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->mixer_member,
                in_ch_config.total_channel_number,
                in_ch_config.resolution,
                in_ch_config.input_mode,
                in_ch_config.buffer_size,
                out_ch_config.total_channel_number,
                out_ch_config.resolution,
                dongle_handle->mixer_connection_status);

    /* sw clk skew init */
    sw_clk_skew_config_t sw_clk_skew_config;
    dongle_handle->clk_skew_port = stream_function_sw_clk_skew_get_port(dongle_handle->source);
    sw_clk_skew_config.channel = 2;
    if (stream_resolution == RESOLUTION_16BIT)
    {
        sw_clk_skew_config.bits = 16;
    }
    else
    {
        sw_clk_skew_config.bits = 32;
    }
    sw_clk_skew_config.order = C_Flp_Ord_1;
    sw_clk_skew_config.skew_io_mode = C_Skew_Oup;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    sw_clk_skew_config.skew_work_mode = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size = 2*dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    sw_clk_skew_config.continuous_frame_size = dongle_handle->source_info.bt_in.sample_rate/1000*dongle_handle->source_info.bt_in.frame_interval/1000*sample_size;
    stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
    dongle_handle->compen_samples = 0;
    dongle_handle->clk_skew_count = 0;
    dongle_handle->clk_skew_watermark_samples = (UL_I2S_SLV_PREFILL_SAMPLES_FRAME+UL_PROCESS_TIME_FRAME)*dongle_handle->source_info.bt_in.sample_rate/1000; /* process time 3ms + clock skew 1ms */
    dongle_handle->clk_skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw clk skew 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 15,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->clk_skew_port,
                sw_clk_skew_config.channel,
                sw_clk_skew_config.bits,
                sw_clk_skew_config.order,
                sw_clk_skew_config.skew_io_mode,
                sw_clk_skew_config.skew_compensation_mode,
                sw_clk_skew_config.skew_work_mode,
                sw_clk_skew_config.max_output_size,
                sw_clk_skew_config.continuous_frame_size,
                dongle_handle->compen_samples,
                dongle_handle->clk_skew_count,
                dongle_handle->clk_skew_watermark_samples);

    /* sw gain init */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution = stream_resolution;
    default_config.target_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    default_config.up_step = 25;
    default_config.up_samples_per_step = dongle_handle->sink_info.i2s_slv_out.sample_rate/1000;
    default_config.down_step = -25;
    default_config.down_samples_per_step = dongle_handle->sink_info.i2s_slv_out.sample_rate/1000;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(dongle_handle->source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    default_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    ULL_V2_LOG_I("[ULL Audio V2][UL][scenario %d-%d][handle 0x%x]sw gain 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                bt_common_open_param->scenario_type,
                bt_common_open_param->scenario_sub_id,
                dongle_handle,
                dongle_handle->gain_port,
                default_config.resolution,
                default_config.target_gain,
                default_config.up_step,
                default_config.up_samples_per_step,
                default_config.down_step,
                default_config.down_samples_per_step,
                bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L,
                bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R);
}

static void ull_audio_v2_dongle_ul_i2s_slv_out_deinit(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    /* bt in info deinit */
    ull_audio_v2_dongle_ul_bt_in_deinit(dongle_handle);

    /* codec deinit */
    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        stream_codec_decoder_lc3plus_deinit(LC3PLUS_DEC_PORT_0, dongle_handle->source);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
        stream_codec_decoder_celt_v2_deinit(CELT_DEC_PORT_0, dongle_handle->source);
#endif /* AIR_CELT_DEC_V2_ENABLE */
    }

    /* sw mixer deinit */
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);

    /* sw clk skew deinit */
    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);

    /* sw gain deinit */
    stream_function_sw_gain_deinit(dongle_handle->gain_port);
}

static void ull_audio_v2_dongle_ul_i2s_slv_out_start(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t gpt_count;
    uint32_t sample_size;
    hal_audio_memory_parameter_t *mem_handle = &dongle_handle->sink->param.audio.mem_handle;
    hal_audio_agent_t agent;

    /* get agent */
    agent = hal_memory_convert_agent(mem_handle->memory_select);
    /* disable AFE irq here */
    hal_memory_set_irq_enable(agent, HAL_AUDIO_CONTROL_OFF);

    /* set agent regsiters' address */
    switch (agent) {
        case HAL_AUDIO_AGENT_MEMORY_DL1:
            // dl1 -> hwsrc1
            if (dongle_handle->sink->param.audio.AfeBlkControl.u4asrcflag) {
                dongle_handle->sink_info.i2s_slv_out.afe_cur_addr  = ASM_CH01_IBUF_RDPNT;
                dongle_handle->sink_info.i2s_slv_out.afe_base_addr = ASM_IBUF_SADR;
                dongle_handle->sink->param.audio.AfeBlkControl.u4asrcid = AFE_MEM_ASRC_1;
            } else {
                dongle_handle->sink_info.i2s_slv_out.afe_cur_addr  = AFE_DL1_CUR;
                dongle_handle->sink_info.i2s_slv_out.afe_base_addr = AFE_DL1_BASE;
            }
            break;

        case HAL_AUDIO_AGENT_MEMORY_DL2:
            dongle_handle->sink_info.i2s_slv_out.afe_cur_addr  = AFE_DL2_CUR;
            dongle_handle->sink_info.i2s_slv_out.afe_base_addr = AFE_DL2_BASE;
            break;

        case HAL_AUDIO_AGENT_MEMORY_DL3:
            dongle_handle->sink_info.i2s_slv_out.afe_cur_addr  = AFE_DL3_CUR;
            dongle_handle->sink_info.i2s_slv_out.afe_base_addr = AFE_DL3_BASE;
            break;

        case HAL_AUDIO_AGENT_MEMORY_DL12:
            dongle_handle->sink_info.i2s_slv_out.afe_cur_addr  = AFE_DL12_CUR;
            dongle_handle->sink_info.i2s_slv_out.afe_base_addr = AFE_DL12_BASE;
            break;

        default:
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR][handle 0x%x][scenario %d-%d]i2s_slv-out stream start, unknow agent = 0x%x", 4,
                dongle_handle,
                dongle_handle->source->param.bt_common.scenario_type,
                dongle_handle->source->param.bt_common.scenario_sub_id,
                agent);
            AUDIO_ASSERT(0);
    }

    /* set 4ms prefill size for process time 3ms + 1ms clock skew */
    if (dongle_handle->sink_info.i2s_slv_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
    {
        sample_size = sizeof(uint16_t);
    }
    else
    {
        sample_size = sizeof(uint32_t);
    }
    // 2 irq period (10ms)
    dongle_handle->sink->streamBuffer.BufferInfo.WriteOffset = sample_size * dongle_handle->sink_info.i2s_slv_out.channel_num *
                                                                ((10 * UL_I2S_SLV_PREFILL_SAMPLES_FRAME) * dongle_handle->sink_info.i2s_slv_out.sample_rate/1000) + 64;
    if (dongle_handle->sink->streamBuffer.BufferInfo.WriteOffset > dongle_handle->sink->streamBuffer.BufferInfo.length) {
        ULL_V2_LOG_W("[ULL Audio V2][UL][handle 0x%x][scenario %d-%d] prefill size is bigger than buffer length, %d > %d", 2,
            dongle_handle->sink->streamBuffer.BufferInfo.WriteOffset,
            dongle_handle->sink->streamBuffer.BufferInfo.length
        );
    }
    dongle_handle->sink->streamBuffer.BufferInfo.ReadOffset  = 0;

    /* start 1ms timer for trigger stream */
    ull_audio_v2_dongle_ul_software_timer_start(dongle_handle);

    /* enable dl12(audio clk) to replace dl3(mst clk) */
    audio_dongle_set_dl12_for_i2s_slave_out(dongle_handle, 1);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][scenario %d-%d]i2s_slv-out stream start, gpt count = 0x%x, start_addr = 0x%x, write_offset = 0x%x, read_offset = 0x%x, length = 0x%x, cur_addr = 0x%x, base_addr = 0x%x, cur_ro = 0x%x, cur_base = 0x%x", 12,
                dongle_handle,
                dongle_handle->source->param.bt_common.scenario_type,
                dongle_handle->source->param.bt_common.scenario_sub_id,
                gpt_count,
                dongle_handle->sink->streamBuffer.BufferInfo.startaddr[0],
                dongle_handle->sink->streamBuffer.BufferInfo.WriteOffset,
                dongle_handle->sink->streamBuffer.BufferInfo.ReadOffset,
                dongle_handle->sink->streamBuffer.BufferInfo.length,
                dongle_handle->sink_info.i2s_slv_out.afe_cur_addr,
                dongle_handle->sink_info.i2s_slv_out.afe_base_addr,
                AFE_GET_REG(dongle_handle->sink_info.i2s_slv_out.afe_cur_addr),
                AFE_GET_REG(dongle_handle->sink_info.i2s_slv_out.afe_base_addr));
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_ul_i2s_slv_out_stop(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t gpt_count;
    uint32_t saved_mask;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_START)
    {
        /* stop timer if the stream is not running */
        ull_audio_v2_dongle_ul_software_timer_stop(dongle_handle);
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
    /* enable dl12(audio clk) to replace dl3(mst clk) */
    audio_dongle_set_dl12_for_i2s_slave_out(dongle_handle, 0);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][scenario %d-%d]i2s_slv-out stream stop, gpt count = 0x%x", 4,
                dongle_handle,
                dongle_handle->source->param.bt_common.scenario_type,
                dongle_handle->source->param.bt_common.scenario_sub_id,
                gpt_count);
}

static int32_t ull_audio_v2_dongle_ul_i2s_slv_out_clock_skew_check(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    int32_t compensatory_samples = 0;
    int32_t remain_samples;
    int32_t sample_size = 0;
    uint32_t write_offset;
    uint32_t read_offset;

    if (dongle_handle->sink_info.i2s_slv_out.sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE)
    {
        sample_size = sizeof(int32_t);
    }
    else
    {
        sample_size = sizeof(int16_t);
    }

    if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED)
    {
        /* get remain samples in AFE SRAM */
        write_offset = dongle_handle->sink->streamBuffer.BufferInfo.WriteOffset;
        read_offset  = dongle_handle->sink->streamBuffer.BufferInfo.ReadOffset;
        if (write_offset >= read_offset)
        {
            remain_samples = (write_offset - read_offset) / sample_size / dongle_handle->sink_info.i2s_slv_out.channel_num;
        }
        else
        {
            remain_samples = (dongle_handle->sink->streamBuffer.BufferInfo.length + write_offset - read_offset) / sample_size / dongle_handle->sink_info.i2s_slv_out.channel_num;
        }

        if (remain_samples > dongle_handle->clk_skew_watermark_samples)
        {
            if (dongle_handle->clk_skew_count > -UL_CLOCK_SKEW_CHECK_COUNT)
            {
                dongle_handle->clk_skew_count -= 1;
            }
        }
        else if (remain_samples < dongle_handle->clk_skew_watermark_samples)
        {
            if (dongle_handle->clk_skew_count < UL_CLOCK_SKEW_CHECK_COUNT)
            {
                dongle_handle->clk_skew_count += 1;
            }
        }
        else
        {
            // if ((dongle_handle->clk_skew_count < UL_CLOCK_SKEW_CHECK_COUNT) && (dongle_handle->clk_skew_count > 0))
            // {
            //     dongle_handle->clk_skew_count -= 1;
            // }
            // else if ((dongle_handle->clk_skew_count > UL_CLOCK_SKEW_CHECK_COUNT) && (dongle_handle->clk_skew_count < 0))
            // {
            //     dongle_handle->clk_skew_count += 1;
            // }
            dongle_handle->clk_skew_count = 0;
        }

        if (dongle_handle->clk_skew_count == UL_CLOCK_SKEW_CHECK_COUNT)
        {
            compensatory_samples = 1;
        }
        else if (dongle_handle->clk_skew_count == -UL_CLOCK_SKEW_CHECK_COUNT)
        {
            compensatory_samples = -1;
        }
        else
        {
            compensatory_samples = 0;
        }
    }

    return compensatory_samples;
}
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

/******************************************************************************/
/*             ULL audio 2.0 dongle UL stream Private Functions               */
/******************************************************************************/
static ull_audio_v2_dongle_first_packet_status_t ull_audio_v2_dongle_ul_first_packet_check(ull_audio_v2_dongle_ul_handle_t *dongle_handle, uint32_t bt_clk)
{
    uint32_t i, saved_mask;
    n9_dsp_share_info_ptr p_share_info;
    uint16_t blk_index = 0;
    uint16_t blk_num;
    uint16_t read_offset;
    uint32_t total_buffer_size;
    ULL_AUDIO_V2_HEADER *p_ull_audio_header;
    ull_audio_v2_dongle_first_packet_status_t first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY;
    uint32_t anchor = 0;
    uint32_t anchor_timeout = 0;
    uint32_t anchor_playen = 0;
    uint32_t anchor_playen_safe =0;

    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
        {
            continue;
        }
        else
        {
            /* set read blk to the write blk - 1 and get this read blk' header */
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.share_info);
            if (p_share_info->write_offset == 0)
            {
                continue;
            }
            total_buffer_size = p_share_info->sub_info.block_info.block_size * p_share_info->sub_info.block_info.block_num;
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            StreamDSP_HWSemaphoreTake();
            read_offset = (p_share_info->write_offset+total_buffer_size-p_share_info->sub_info.block_info.block_size)%total_buffer_size;
            p_ull_audio_header = (ULL_AUDIO_V2_HEADER *)(hal_memview_infrasys_to_dsp0(p_share_info->start_addr) + read_offset);
            StreamDSP_HWSemaphoreGive();
            hal_nvic_restore_interrupt_mask(saved_mask);
            blk_index = read_offset/p_share_info->sub_info.block_info.block_size;
            blk_num = p_share_info->sub_info.block_info.block_num;

            /* check packet's timestamp */
            if (p_ull_audio_header->valid_packet != 1)
            {
                continue;
            }
            else
            {
                anchor_timeout = p_ull_audio_header->TimeStamp;
                anchor = (anchor_timeout+0x10000000-dongle_handle->source_info.bt_in.bt_retry_window) & 0x0fffffff;
                anchor_playen = (anchor_timeout+dongle_handle->source_info.bt_in.play_en_delay) & 0x0fffffff;
                if (dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay >= 0) {
                    anchor_playen_safe = (anchor_timeout + dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay) & 0x0fffffff;
                } else {
                    anchor_playen_safe = (anchor_timeout + 0x10000000 - (uint32_t)(-dongle_handle->source_info.bt_in.play_en_first_packet_safe_delay)) & 0x0fffffff;
                }

                /* check if this packet is suitable for play */
                if (anchor < anchor_playen_safe)
                {
                    if ((bt_clk >= anchor) && (bt_clk <= anchor_playen_safe))
                    {
                        /* --------- ........ --------- anchor ---------- bt_clk ---------- anchor_playen_safe --------- ........ -------- */
                        first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY;
                        goto _find_first_packet;
                    }
                }
                else
                {
                    if ((bt_clk < anchor) && (bt_clk <= anchor_playen_safe))
                    {
                        /* --------- ........ ---------- bt_clk ---------- anchor_playen_safe --------- ........ --------- anchor -------- */
                        first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY;
                        goto _find_first_packet;
                    }
                    else if ((bt_clk >= anchor) && (bt_clk > anchor_playen_safe))
                    {
                        /* --------- ........ ---------- anchor_playen_safe --------- ........ --------- anchor -------- bt_clk ---------- */
                        first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY;
                        goto _find_first_packet;
                    }
                }
            }
        }
    }

_find_first_packet:
    if (first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY)
    {
        /* update play info */
        dongle_handle->source_info.bt_in.play_en_bt_clk = anchor_playen;
        dongle_handle->source_info.bt_in.first_anchor_bt_clk = anchor;
        dongle_handle->source_info.bt_in.first_packet_bt_clk = anchor_timeout;
        dongle_handle->source_info.bt_in.fetch_anchor_previous = (anchor_timeout+0x10000000-dongle_handle->source_info.bt_in.bt_interval) & 0x0fffffff;
        dongle_handle->source_info.bt_in.fetch_anchor = anchor_timeout;
        for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
        {
            if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
            {
                continue;
            }
            else
            {
                if (((dongle_handle->source_info.bt_in.bt_info[i])->blk_index_previous == 0) && ((dongle_handle->source_info.bt_in.bt_info[i])->blk_index == 0))
                {
                    (dongle_handle->source_info.bt_in.bt_info[i])->blk_index_previous = (blk_index+blk_num-1)%blk_num;
                    (dongle_handle->source_info.bt_in.bt_info[i])->blk_index = blk_index;
                }
            }
        }
        ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x] first packet is ready %d, 0x%x, 0x%x, 0x%x, 0x%x", 6,
                        dongle_handle,
                        blk_index,
                        dongle_handle->source_info.bt_in.first_anchor_bt_clk,
                        dongle_handle->source_info.bt_in.first_packet_bt_clk,
                        bt_clk,
                        dongle_handle->source_info.bt_in.play_en_bt_clk);
    }
    else
    {
        ULL_V2_LOG_W("[ULL Audio V2][UL][WARNNING][handle 0x%x] first packet is not ready %d, 0x%x, 0x%x, 0x%x, 0x%x", 6,
                        dongle_handle,
                        blk_index,
                        anchor,
                        anchor_timeout,
                        bt_clk,
                        anchor_playen);
    }

    return first_packet_status;
}

static bool ull_audio_v2_dongle_ul_fetch_time_is_timeout(ull_audio_v2_dongle_ul_handle_t *dongle_handle, uint32_t bt_clk)
{
    bool ret = false;
    uint32_t fetch_anchor_next_2;

    if ((dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED) || (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY))
    {
        fetch_anchor_next_2 = (dongle_handle->source_info.bt_in.fetch_anchor+dongle_handle->source_info.bt_in.bt_interval*2) & 0x0fffffff;
        if (dongle_handle->source_info.bt_in.fetch_anchor < fetch_anchor_next_2)
        {
            if (bt_clk >= fetch_anchor_next_2)
            {
                /* --------- ........ --------- fetch_anchor ---------- fetch_anchor_next ---------- fetch_anchor_next_2 ---------- bt_clk --------- ........ -------- */
                ret = true;
            }
            else if ((bt_clk < dongle_handle->source_info.bt_in.fetch_anchor) && (bt_clk < dongle_handle->source_info.bt_in.fetch_anchor_previous) && (bt_clk < fetch_anchor_next_2))
            {
                /* --------- ........ --------  bt_clk --------- ........ --------- fetch_anchor_previous --------- fetch_anchor ---------- fetch_anchor_next ---------- fetch_anchor_next_2 ---------- */
                ret = true;
            }
        }
        else
        {
            if ((bt_clk >= fetch_anchor_next_2) && (bt_clk < dongle_handle->source_info.bt_in.fetch_anchor_previous))
            {
                /* ---------- fetch_anchor_next_2 --------- ........ ---------- bt_clk --------- ........ --------- fetch_anchor_previous --------- fetch_anchor --------- fetch_anchor_next -------- */
                /* --------- fetch_anchor_next ---------- fetch_anchor_next_2 --------- ........ ---------- bt_clk --------- ........ --------- fetch_anchor_previous --------- fetch_anchor -------- */
                ret = true;
            }
        }
    }

    return ret;
}

static void ull_audio_v2_dongle_ul_process_timeout_packets(ull_audio_v2_dongle_ul_handle_t *dongle_handle, uint32_t bt_clk, uint16_t bt_phase)
{
    uint32_t i;
    n9_dsp_share_info_ptr p_share_info;
    uint32_t blk_size;
    uint32_t blk_num;
    uint32_t blk_index;
    uint32_t blk_index_previous;
    bool do_drop = false;
    uint32_t drop_frames = 0;

    while (ull_audio_v2_dongle_ul_fetch_time_is_timeout(dongle_handle, bt_clk) == true)
    {
        /* drop this packet */
        drop_frames += 1;
        dongle_handle->source_info.bt_in.fetch_anchor_previous = dongle_handle->source_info.bt_in.fetch_anchor;
        dongle_handle->source_info.bt_in.fetch_anchor = (dongle_handle->source_info.bt_in.fetch_anchor + dongle_handle->source_info.bt_in.bt_interval) & 0x0fffffff;
        MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);
    }

    if (drop_frames != 0)
    {
        do_drop = true;
    }

    if (do_drop)
    {
        /* drop all timeout packets */
        for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
        {
            if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
            {
                continue;
            }
            else
            {
                /* get blk info */
                p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.share_info);
                blk_size     = p_share_info->sub_info.block_info.block_size;
                blk_num      = p_share_info->sub_info.block_info.block_num;
                /* update blk index */
                // (dongle_handle->source_info.bt_in.bt_info[i])->blk_index_previous = (dongle_handle->source_info.bt_in.bt_info[i])->blk_index;
                blk_index_previous = (dongle_handle->source_info.bt_in.bt_info[i])->blk_index;
                blk_index = ((dongle_handle->source_info.bt_in.bt_info[i])->blk_index + drop_frames) % blk_num;
                (dongle_handle->source_info.bt_in.bt_info[i])->blk_index = blk_index;
            }
        }
        ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR][handle 0x%x] packet is timeout %d, %d, %d, 0x%x, 0x%x, 0x%x", 7,
                        dongle_handle,
                        drop_frames,
                        blk_index_previous,
                        blk_index,
                        dongle_handle->source_info.bt_in.fetch_anchor_previous,
                        dongle_handle->source_info.bt_in.fetch_anchor,
                        bt_clk);
    }
}

static bool ull_audio_v2_dongle_ul_fetch_timestamp_is_correct(ull_audio_v2_dongle_ul_handle_t *dongle_handle, uint32_t timestamp)
{
    bool ret = false;
    uint32_t fetch_anchor_max;
    uint32_t fetch_anchor_min;

    if ((dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED) || (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY))
    {
        if (timestamp == dongle_handle->source_info.bt_in.fetch_anchor)
        {
            ret = true;
        }
        else
        {
            fetch_anchor_max = (dongle_handle->source_info.bt_in.fetch_anchor+dongle_handle->source_info.bt_in.bt_channel_anchor_diff) & 0x0fffffff;
            fetch_anchor_min = (dongle_handle->source_info.bt_in.fetch_anchor+0x10000000-dongle_handle->source_info.bt_in.bt_channel_anchor_diff) & 0x0fffffff;
            if (fetch_anchor_max > fetch_anchor_min)
            {
                if ((timestamp < fetch_anchor_max) && (timestamp > fetch_anchor_min))
                {
                    ret = true;
                }
            }
            else
            {
                if ((timestamp < fetch_anchor_max) || (timestamp > fetch_anchor_min))
                {
                    ret = true;
                }
            }
        }
    }

    return ret;
}

static bool ull_audio_v2_dongle_ul_packet_is_processed_by_other_stream(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    bool ret = false;

    if (ull_audio_v2_dongle_first_ul_handle->total_number > 1)
    {
        if (ull_audio_v2_ul_packet_anchor_in_mixer == dongle_handle->source_info.bt_in.fetch_anchor)
        {
            ret = true;
        }
    }

    return ret;
}

/* Public functions ----------------------------------------------------------*/
/******************************************************************************/
/*               ULL audio 2.0 dongle common Public Functions                 */
/******************************************************************************/
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_init_play_info(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    ull_audio_v2_init_play_info_t *play_info;
    ull_audio_v2_dongle_ul_handle_t *c_handle;
    uint32_t i;
    uint32_t saved_mask;
    UNUSED(ack);

    /* save play info to the global variables */
    play_info = (ull_audio_v2_init_play_info_t *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    memcpy(&ull_audio_v2_bt_init_play_info, play_info, sizeof(ull_audio_v2_init_play_info_t));

    /* update uplink BT transmission window clk */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (ull_audio_v2_dongle_first_ul_handle != NULL)
    {
        c_handle = ull_audio_v2_dongle_first_ul_handle;
        for (i=0; i < (uint32_t)ull_audio_v2_dongle_first_ul_handle->total_number; i++)
        {
            if ((ull_audio_v2_bt_init_play_info.dl_retransmission_window_clk != 0) && (c_handle->source_info.bt_in.bt_retry_window != ull_audio_v2_bt_init_play_info.dl_retransmission_window_clk))
            {
                c_handle->source_info.bt_in.bt_retry_window = ull_audio_v2_bt_init_play_info.dl_retransmission_window_clk;
            }
            /* switch the next handle */
            c_handle = c_handle->next_ul_handle;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if ((ull_audio_v2_dongle_first_ul_handle != NULL) || (ull_audio_v2_dongle_first_dl_handle != NULL)) {
        ull_audio_v2_dongle_dl_stream_software_timer_start();
    }

    ULL_V2_LOG_I("[ULL Audio V2][UL][config] play_info->dl_timestamp_clk %d",   1, ull_audio_v2_bt_init_play_info.dl_retransmission_window_clk);
    ULL_V2_LOG_I("[ULL Audio V2][UL][config] play_info->dl_timestamp_phase %d", 1, ull_audio_v2_bt_init_play_info.dl_retransmission_window_phase);
}

/******************************************************************************/
/*               ULL audio 2.0 dongle DL path Public Functions                */
/******************************************************************************/
uint32_t ull_audio_v2_dongle_dl_get_stream_in_max_size_each_channel(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t stream_in_size = 0;
    uint32_t frame_samples;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            frame_samples = max(dongle_handle->source_info.usb_in.frame_samples, dongle_handle->sink_info.bt_out.frame_samples*1000/dongle_handle->sink_info.bt_out.frame_interval);
            stream_in_size = 2*dongle_handle->source_info.usb_in.frame_max_num*frame_samples*sizeof(uint32_t);
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return stream_in_size;
}

uint32_t ull_audio_v2_dongle_dl_get_stream_in_channel_number(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t channel_num = 0;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            channel_num = dongle_handle->source_info.usb_in.channel_num;
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return channel_num;
}

stream_samplerate_t ull_audio_v2_dongle_dl_get_stream_in_sampling_rate_each_channel(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    stream_samplerate_t samplerate = 0;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            samplerate = dongle_handle->source_info.usb_in.sample_rate/1000;
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return samplerate;
}

uint32_t ull_audio_v2_dongle_dl_get_stream_out_max_size_each_channel(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t stream_out_size = 0;
    uint32_t                        in_format_bytes  = audio_dongle_get_format_bytes(dongle_handle->source_info.usb_in.sample_format);
    uint32_t                        out_format_bytes = audio_dongle_get_format_bytes(dongle_handle->sink_info.bt_out.sample_format);
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            stream_out_size = 2*dongle_handle->sink_info.bt_out.frame_size;
            stream_out_size = MAX(stream_out_size, dongle_handle->process_sample_rate_max * dongle_handle->sink_info.bt_out.frame_interval / 1000 / 1000 * MAX(in_format_bytes, out_format_bytes));
            break;
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN:
#endif
#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0:
#endif
#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0:
#endif
            stream_out_size = 2*dongle_handle->sink_info.bt_out.frame_size;
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return stream_out_size;
}

uint32_t ull_audio_v2_dongle_dl_get_stream_out_channel_number(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t channel_num = 0;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN:
#endif
#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0:
#endif
#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0:
#endif
            channel_num = dongle_handle->sink_info.bt_out.channel_num;
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return channel_num;
}

stream_samplerate_t ull_audio_v2_dongle_dl_get_stream_out_sampling_rate_each_channel(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    stream_samplerate_t samplerate = 0;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN:
#endif
#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0:
#endif
#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0:
#endif
            samplerate = dongle_handle->sink_info.bt_out.sample_rate/1000;
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return samplerate;
}

void ull_audio_v2_dongle_dl_init(SOURCE source, SINK sink, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    /* get handle for application config */
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = ull_audio_v2_dongle_dl_get_handle(sink);
    dongle_handle->source = source;
    dongle_handle->sink   = sink;
    sink->param.bt_common.scenario_param.dongle_handle = (void *)dongle_handle;
    ULL_V2_LOG_I("[ULL Audio V2][DL] init scenario type %d-%d", 2, source->scenario_type, sink->scenario_type);
    /* task config */
    source->taskId = DHP_TASK_ID;
    sink->taskid = DHP_TASK_ID;

    /* init audio info */
    switch (source->scenario_type)
    {
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            dongle_handle->sub_id = audio_transmitter_open_param->scenario_sub_id;
            ull_audio_v2_dongle_dl_usb_in_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            /* register preload ccni handler */
            if (dongle_handle->sink_info.bt_out.codec_type != AUDIO_DSP_CODEC_TYPE_ULD) {
                if (source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0)
                {
                    audio_transmitter_register_isr_handler(1, ull_audio_v2_dongle_dl_preload_handler1);
                }
                else
                {
                    audio_transmitter_register_isr_handler(2, ull_audio_v2_dongle_dl_preload_handler2);
                }
            }
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable line in!", 0);
                AUDIO_ASSERT(0);
            #else
                UNUSED(audio_transmitter_open_param);
                dongle_handle->sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN;
                ull_audio_v2_dongle_dl_init_afe_in(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE */
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable i2s mst in!", 0);
                AUDIO_ASSERT(0);
            #else
                UNUSED(audio_transmitter_open_param);
                dongle_handle->sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0;
                ull_audio_v2_dongle_dl_init_afe_in(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE */
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable i2s slv in!", 0);
                AUDIO_ASSERT(0);
            #else
                UNUSED(audio_transmitter_open_param);
                dongle_handle->sub_id = AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0;
                ull_audio_v2_dongle_dl_init_afe_in(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE */
            break;
        default:
            ULL_V2_LOG_E("[ULL Audio V2][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    ull_audio_v2_dongle_dl_common_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* lock sleep because sleep wake-up time will consume the stream process time */
    hal_sleep_manager_lock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif

    /* register ccni handler */
    if (dongle_handle->sink_info.bt_out.frame_interval % 1250 == 0) {
        audio_transmitter_register_isr_handler(0, ull_audio_v2_dongle_dl_ccni_handler);
    } else {
        g_ull_audio_v2_dl_play_info_need_trigger = true;
    }

#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.without_bt_link_mode_enable == 0)
    {
        if (ull_audio_v2_dl_without_bt_link_mode_enable == true)
        {
            AUDIO_ASSERT(0);
        }
    }
    else
    {
        /* start gpt timer that act as bt link to trigger dsp stream */
        ull_audio_v2_dongle_dl_software_timer_start(dongle_handle);
        if (dongle_handle->total_number == 1)
        {
            ull_audio_v2_dl_without_bt_link_mode_enable = true;
            ULL_V2_LOG_I("[ULL Audio V2][DL] scenario type %d enter without_bt_link_mode mode", 1, source->scenario_type);
        }
    }

    /* silence detection init */
    DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_INIT, AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0+dongle_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0, false);
    //ull_audio_v2_dongle_silence_detection_init(dongle_handle, AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0+dongle_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0);
#endif /* AIR_SILENCE_DETECTION_ENABLE */

    /* stream status update */
    dongle_handle->stream_status = ULL_AUDIO_V2_DONGLE_STREAM_INIT;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_dl_unregister_isr_handler(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    uint32_t saved_mask;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->total_number == 1)
    {
        if (dongle_handle->sink_info.bt_out.frame_interval % 1250) {
            ull_audio_v2_dongle_dl_stream_software_timer_stop();
        } else {
            audio_transmitter_unregister_isr_handler(0, ull_audio_v2_dongle_dl_ccni_handler);
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
}

void ull_audio_v2_dongle_dl_deinit(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    UNUSED(source);
    UNUSED(sink);
    ULL_V2_LOG_I("[ULL Audio V2][DL] deinit scenario type %d-%d", 2, source->scenario_type, sink->scenario_type);

    /* unregister ccni handler */
    ull_audio_v2_dongle_dl_unregister_isr_handler(dongle_handle);

    /* deinit audio info */
    switch (source->scenario_type)
    {
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            ull_audio_v2_dongle_dl_usb_in_deinit(dongle_handle);
            /* unregister preload ccni handler */
            if (dongle_handle->sink_info.bt_out.codec_type != AUDIO_DSP_CODEC_TYPE_ULD) {
                if (source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0)
                {
                    audio_transmitter_unregister_isr_handler(1, ull_audio_v2_dongle_dl_preload_handler1);
                }
                else
                {
                    audio_transmitter_unregister_isr_handler(2, ull_audio_v2_dongle_dl_preload_handler2);
                }
            }
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable line in!", 0);
                AUDIO_ASSERT(0);
            #else
                ull_audio_v2_dongle_dl_deinit_afe_in(dongle_handle);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE */
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable i2s mst in!", 0);
                AUDIO_ASSERT(0);
            #else
                ull_audio_v2_dongle_dl_deinit_afe_in(dongle_handle);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE */
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable i2s slv in!", 0);
                AUDIO_ASSERT(0);
            #else
                ull_audio_v2_dongle_dl_deinit_afe_in(dongle_handle);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE */
            break;
        default:
            ULL_V2_LOG_E("[ULL Audio V2][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    ull_audio_v2_dongle_dl_common_deinit(dongle_handle);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* unlock sleep */
    hal_sleep_manager_unlock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif

#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (ull_audio_v2_dl_without_bt_link_mode_enable == true)
    {
        /* stop gpt timer that act as bt link to trigger dsp stream */
        ull_audio_v2_dongle_dl_software_timer_stop(dongle_handle);
        if (dongle_handle->total_number == 1)
        {
            ull_audio_v2_dl_without_bt_link_mode_enable = false;
            ULL_V2_LOG_I("[ULL Audio V2][DL] scenario type %d exit without_bt_link_mode mode", 1, source->scenario_type);
        }
    }

    /* silence detection deinit */
    DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_DEINIT, AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0+dongle_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0, false);
    //ull_audio_v2_dongle_silence_detection_deinit(dongle_handle, AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0+dongle_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0);
#endif /* AIR_SILENCE_DETECTION_ENABLE */

    /* release handle */
    ull_audio_v2_dongle_dl_release_handle(dongle_handle);
    sink->param.bt_common.scenario_param.dongle_handle = NULL;
}

void ull_audio_v2_dongle_dl_start(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    UNUSED(source);
    UNUSED(sink);

    switch (source->scenario_type)
    {
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            dongle_handle->fetch_count = 0;
            dongle_handle->source_preload_count = 0;
            dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY;
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable line in!", 0);
                AUDIO_ASSERT(0);
            #else
                dongle_handle->fetch_count = 0;
                dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY;
                ull_audio_v2_dongle_dl_start_afe_in(dongle_handle);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE */
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable i2s mst in!", 0);
                AUDIO_ASSERT(0);
            #else
                dongle_handle->fetch_count = 0;
                dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY;
                ull_audio_v2_dongle_dl_start_afe_in(dongle_handle);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE */
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable i2s slv in!", 0);
                AUDIO_ASSERT(0);
            #else
                dongle_handle->fetch_count = 0;
                dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY;
                ull_audio_v2_dongle_dl_start_afe_in(dongle_handle);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE */
            break;
        default:
            ULL_V2_LOG_E("[ULL Audio V2][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }

    /* stream status update */
    dongle_handle->stream_status = ULL_AUDIO_V2_DONGLE_STREAM_START;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_dl_stop(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t i, saved_mask;
    n9_dsp_share_info_ptr p_share_info;

    UNUSED(source);
    UNUSED(sink);

    /* stream status update */
    dongle_handle->stream_status = ULL_AUDIO_V2_DONGLE_STREAM_DEINIT;

    switch (source->scenario_type)
    {
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY;
            hal_nvic_restore_interrupt_mask(saved_mask);
            for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++) {
                ull_audio_v2_dongle_bt_info_t *bt_info;
                bt_info = dongle_handle->sink_info.bt_out.bt_info[i];
                if (bt_info != NULL) {
                    p_share_info = (n9_dsp_share_info_ptr)(bt_info->bt_link_info.share_info);
                    dongle_handle->ccni_blk_index = (bt_info->blk_index + 1) % p_share_info->sub_info.block_info.block_num;
                    ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING]dl silence padding to sink buffer done, index %d\r\n", 1, dongle_handle->ccni_blk_index);
                    break;
                }
            }
            for (i = 0; i < DL_STOP_PADDING_PKG_NUMBER; i++) {
                ull_audio_v2_dongle_dl_dummy_process(sink);
                dongle_handle->ccni_blk_index = (dongle_handle->ccni_blk_index + 1) % p_share_info->sub_info.block_info.block_num;
            }
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable line in!", 0);
                AUDIO_ASSERT(0);
            #else
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY;
                hal_nvic_restore_interrupt_mask(saved_mask);
                ull_audio_v2_dongle_dl_stop_afe_in(dongle_handle);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE */
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable i2s mst in!", 0);
                AUDIO_ASSERT(0);
            #else
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY;
                hal_nvic_restore_interrupt_mask(saved_mask);
                ull_audio_v2_dongle_dl_stop_afe_in(dongle_handle);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE */
            break;
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0:
            #ifndef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
                ULL_V2_LOG_E("[ULL Audio V2][DL] plz enable i2s slv in!", 0);
                AUDIO_ASSERT(0);
            #else
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY;
                hal_nvic_restore_interrupt_mask(saved_mask);
                ull_audio_v2_dongle_dl_stop_afe_in(dongle_handle);
            #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE */
            break;
        default:
            AUDIO_ASSERT(0);
            break;
    }
}

bool ull_audio_v2_dongle_dl_config(SOURCE source, stream_config_type type, U32 value)
{
    bool ret = true;
    ull_audio_v2_dongle_runtime_config_param_p config_param = (ull_audio_v2_dongle_runtime_config_param_p)value;
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t vol_ch   = 0;
    int32_t  vol_gain = 0;
    sw_gain_config_t old_config;
    uint32_t channel  = 0;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    audio_scenario_type_t audio_scenario_type;
#endif /* AIR_SILENCE_DETECTION_ENABLE */
    uint32_t i;
    UNUSED(source);
    UNUSED(type);
    UNUSED(value);

    switch (config_param->config_operation)
    {
        case ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_VOL_INFO:
            vol_ch = config_param->config_param.vol_gain.vol_ch;
            vol_gain = config_param->config_param.vol_gain.vol_gain;
            if (vol_ch != 0)
            {
                stream_function_sw_gain_get_config(dongle_handle->gain_port, vol_ch, &old_config);
                ULL_V2_LOG_I("[ULL Audio V2][DL][config][handle 0x%x]scenario %d-%d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 6,
                            dongle_handle,
                            source->param.data_dl.scenario_type,
                            source->param.data_dl.scenario_sub_id,
                            vol_ch,
                            old_config.target_gain,
                            vol_gain);
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
                if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) {
                    ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[0] = vol_gain;
                    ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[1] = vol_gain;
                } else {
                    ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0] = vol_gain;
                    ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1] = vol_gain;
                    if ((ull_audio_v2_dongle_vol_balance_handle.enable) && (ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] < vol_gain)) {
                        /* Note: at now the channel volume should be the same */
                        vol_gain = ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0];
                    } else {
                        ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] = vol_gain;
                        ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] = vol_gain;
                    }
                }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
                stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, vol_ch, vol_gain);
            }
            else
            {
#if defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
                if ((source->scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0) &&
                    (source->scenario_type >= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN)) {
                    DSP_CALLBACK_PTR callback_ptr = DSP_Callback_Get(source, source->transform->sink);
                    channel = callback_ptr->EntryPara.in_channel_num;
                } else
#endif
                {
                    channel = dongle_handle->source_info.usb_in.channel_num;
                }
                for (i = 0; i < channel; i++)
                {
                    vol_ch = i+1;
                    stream_function_sw_gain_get_config(dongle_handle->gain_port, vol_ch, &old_config);
                    ULL_V2_LOG_I("[ULL Audio V2][DL][config][handle 0x%x]scenario %d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 5,
                                dongle_handle,
                                source->scenario_type,
                                vol_ch,
                                old_config.target_gain,
                                vol_gain);
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
                    if (vol_ch > 2)
                    {
                        ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x] Smart balanece only support 2 channel, %u\r\n", 2, dongle_handle, vol_ch);
                        AUDIO_ASSERT(0);
                    }
                    if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) {
                        ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[i] = vol_gain;
                    } else {
                        ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[i] = vol_gain;
                        if ((ull_audio_v2_dongle_vol_balance_handle.enable) && (ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[i] < vol_gain)) {
                            vol_gain = ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[i];
                        } else {
                            ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[i] = vol_gain;
                        }
                    }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
                    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, vol_ch, vol_gain);
                }
            }
            break;

        case ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_MIX:
            ULL_V2_LOG_I("[ULL Audio V2][DL][config] mix done. do nothing. scenario type [%d]", 1, dongle_handle->source->scenario_type);
            break;

        case ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_UNMIX:
            ULL_V2_LOG_I("[ULL Audio V2][DL][config] unmix done. do nothing. scenario type [%d]", 1, dongle_handle->source->scenario_type);
            break;

        case ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_DL_BITRATE:
            break;

#ifdef AIR_SILENCE_DETECTION_ENABLE
        case ULL_AUDIO_V2_DONGLE_CONFIG_OP_SILENCE_DETECTION_ENABLE:
            audio_scenario_type = AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0+dongle_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0;
            DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_ENABLE, (uint32_t)audio_scenario_type, false);
            ULL_V2_LOG_I("[ULL Audio V2][DL][config]scenario %d-%d enable silence detection\r\n", 2,
                        source->param.data_dl.scenario_type,
                        source->param.data_dl.scenario_sub_id);
            break;

        case ULL_AUDIO_V2_DONGLE_CONFIG_OP_SILENCE_DETECTION_DISABLE:
            audio_scenario_type = AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0+dongle_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0;
            DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_DISABLE, (uint32_t)audio_scenario_type, false);
            ULL_V2_LOG_I("[ULL Audio V2][DL][config]scenario %d-%d disable silence detection\r\n", 2,
                        source->param.data_dl.scenario_type,
                        source->param.data_dl.scenario_sub_id);
            break;
#endif /* AIR_SILENCE_DETECTION_ENABLE */

        default:
            ULL_V2_LOG_E("[ULL Audio V2][DL][config] unknow cmd:%d\r\n", 1, config_param->config_operation);
            break;
    }

    return ret;
}

ATTR_TEXT_IN_IRAM bool ull_audio_v2_dongle_dl_source_get_avail_size(SOURCE source, uint32_t *avail_size)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t saved_mask;

    if ((g_ull_audio_v2_dongle_dl_dummy_process_flag[dongle_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0] == true) &&
        (hal_nvic_query_exception_number() == -1)) {
        /* Do dummy process when no data for USB in */
        ull_audio_v2_dongle_dl_dummy_process(source->transform->sink);
        g_ull_audio_v2_dongle_dl_dummy_process_flag[dongle_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0] = false;
        if (g_ull_audio_v2_dongle_dl_dummy_process_cnt[dongle_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0] == 1) {
            ull_audio_v2_dongle_dl_dummy_status_reset(dongle_handle);
        }
        /* Bypass real stream process */
        *avail_size = 0;
        return true;
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->fetch_count != 0)
    {
        /* there is a fetch requirement, so set avail size as dummy 1ms data and the actual avail size will be quired in the later stream process */
        *avail_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size-sizeof(audio_transmitter_frame_header_t);
        *avail_size = *avail_size / dongle_handle->source_info.usb_in.channel_num;
    }
    else if (dongle_handle->source_preload_count != 0)
    {
        if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_READY)
        {
            /* there is a fetch requirement, so set avail size as dummy 1ms data and the actual avail size will be quired in the later stream process */
            *avail_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size-sizeof(audio_transmitter_frame_header_t);
            *avail_size = *avail_size / dongle_handle->source_info.usb_in.channel_num;
        }
        else
        {
            /* there is no fetch requirement, because the stream is not started */
            *avail_size = 0;
            /* reset fetch count */
            dongle_handle->source_preload_count = 0;
        }
    }
    else
    {
        /* there is no fetch requirement, so set avail size as 0 */
        *avail_size = 0;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

#if ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_V2_LOG_I("[ULL Audio V2][DL][DEBUG][handle 0x%x][get_avail_size]:%u, %u, %d, %u, %d, %d", 7, dongle_handle, dongle_handle->data_status, source->streamBuffer.ShareBufferInfo.read_offset, source->streamBuffer.ShareBufferInfo.write_offset, *avail_size, current_timestamp, hal_nvic_query_exception_number());
#endif /* ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_IRAM uint32_t ull_audio_v2_dongle_dl_source_copy_payload(SOURCE source, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(source, source->transform->sink);
    uint32_t unprocess_frames, check_threshold;
    uint32_t total_samples;
    uint32_t avail_size;
    uint32_t read_offset;
    uint32_t current_frame;
    uint32_t current_frame_samples;
    uint32_t total_buffer_size;
    audio_transmitter_frame_header_t *frame_header;
    uint32_t saved_mask;
    uint32_t sample_size;
    UNUSED(dst_buf);

    /* get the total buffer size in share buffer */
    total_buffer_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;

    /* choose stream mode */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    dongle_handle->process_frames = 0;
    if (dongle_handle->fetch_count != 0)
    {
        dongle_handle->stream_mode = ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_PREPARE_PACKET;
    }
    else if (dongle_handle->source_preload_count != 0)
    {
        dongle_handle->stream_mode = ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_PRELOAD;
    }
    else
    {
        ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x] mode is not right, %u, %u\r\n", 3, dongle_handle, dongle_handle->fetch_count, dongle_handle->source_preload_count);
        AUDIO_ASSERT(0);
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    /* get actual data size */
    ull_audio_v2_dongle_dl_usb_get_actual_avail_size(source, &avail_size);
    /* in here avail_size is the data size of the each channel, we need to get the total data size of all channels */
    length = avail_size * dongle_handle->source_info.usb_in.channel_num;

    /* copy the pcm data in share buffer into the stream buffers */
    if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY)
    {
        unprocess_frames = length / (source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size-sizeof(audio_transmitter_frame_header_t));
        /* check if the unprocessed frame is enough */
        if (unprocess_frames < dongle_handle->source_info.usb_in.frame_max_num)
        {
            /* in here, it means the unprocessed frames is not enough */
            /* update stream samples */
            unprocess_frames = 0;
            total_samples = 0;
            length = 0;
        }
        else
        {
            /* Only copy the latest x.xms data from the share buffer and the remian data in the head are dropped */
            total_samples = 0;
            read_offset = (source->streamBuffer.ShareBufferInfo.read_offset + source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size*(unprocess_frames-dongle_handle->source_info.usb_in.frame_max_num)) % total_buffer_size;
            src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + read_offset);
            current_frame = 0;
            while(current_frame < dongle_handle->source_info.usb_in.frame_max_num)
            {
                /* get current frame info and current frame samples */
                frame_header = (audio_transmitter_frame_header_t *)src_buf;
                if (frame_header->payload_len != dongle_handle->source_info.usb_in.frame_size)
                {
                    ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x]frame size is not matched, %u, %u\r\n", 3, dongle_handle, frame_header->payload_len, dongle_handle->source_info.usb_in.frame_size);
                    AUDIO_ASSERT(0);
                }
                current_frame_samples = dongle_handle->source_info.usb_in.frame_samples;
                /* copy usb audio data from share buffer */
                /* 0.5ms case */
                if ((current_frame == 0) && ((dongle_handle->sink_info.bt_out.frame_interval%500)%2 != 0))
                {
                    /* only copy 0.5ms(for example 48K*0.5ms=24samples) data at the first time */
                    current_frame_samples = dongle_handle->source_info.usb_in.frame_samples/2;
                    /* offset 0.5ms data in src_buf */
                    src_buf = src_buf + dongle_handle->source_info.usb_in.frame_size/2;
                }
                ull_audio_v2_dongle_dl_usb_data_copy(dongle_handle, stream, src_buf, current_frame_samples, total_samples);
                /* 0.5ms case */
                if ((current_frame == 0) && ((dongle_handle->sink_info.bt_out.frame_interval%500)%2 != 0))
                {
                    /* offset back src_buf to the original position */
                    src_buf = src_buf - dongle_handle->source_info.usb_in.frame_size/2;
                }
                /* update total samples */
                total_samples += current_frame_samples;
                /* update copied frame number */
                current_frame += 1;
                /* change to the next frame */
                read_offset = (read_offset + source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) % total_buffer_size;
                src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + read_offset);
            }

            /* update stream samples */
            length = total_samples;
        }
    }
    else
    {
        /* get the unprocessed frame num and check if the uprocess frame is too much */
        unprocess_frames = length / (source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size-sizeof(audio_transmitter_frame_header_t));
        if ((g_ull_audio_v2_dongle_dl_dummy_process_cnt[dongle_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0] > 0) && (dongle_handle->source_preload_count == 0)) {
            /* Check with other threshold when NOT in preloader mode */
            check_threshold = dongle_handle->source_info.usb_in.frame_max_num - 1;
            ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING][handle 0x%x] 1st restore from dummy process, unprocess_frames %u", 2,
                            dongle_handle, unprocess_frames);
            AUDIO_ASSERT(unprocess_frames >= dongle_handle->source_info.usb_in.frame_max_num);
            AUDIO_ASSERT(stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port_0, 1) == 0);
#if 0
            /* Do codec reset */
            if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS) {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
                stream_codec_encoder_lc3plus_deinit(LC3PLUS_ENC_PORT_0, dongle_handle->sink);
                lc3plus_enc_config_t lc3plus_enc_config;
                lc3plus_enc_config.sample_bits       = 24;
                lc3plus_enc_config.channel_num       = dongle_handle->sink_info.bt_out.channel_num;
                lc3plus_enc_config.sample_rate       = dongle_handle->sink_info.bt_out.sample_rate;
                lc3plus_enc_config.bit_rate          = dongle_handle->sink_info.bt_out.bit_rate;
                lc3plus_enc_config.frame_interval    = dongle_handle->sink_info.bt_out.frame_interval;
                lc3plus_enc_config.frame_samples     = dongle_handle->sink_info.bt_out.frame_samples;
                lc3plus_enc_config.in_frame_size     = dongle_handle->sink_info.bt_out.frame_samples*sizeof(int32_t);
                lc3plus_enc_config.out_frame_size    = dongle_handle->sink_info.bt_out.frame_size;
                lc3plus_enc_config.process_frame_num = 1;
                lc3plus_enc_config.codec_mode        = LC3PLUS_ARCH_FX;
                lc3plus_enc_config.channel_mode      = LC3PLUS_ENC_CHANNEL_MODE_MULTI;
                stream_codec_encoder_lc3plus_init(LC3PLUS_ENC_PORT_0, dongle_handle->sink, &lc3plus_enc_config);
                ULL_V2_LOG_I("[ULL Audio V2][DL][scenario type %d-%d][handle 0x%x]reset lc3plus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                            dongle_handle->source->scenario_type,
                            dongle_handle->sink->scenario_type,
                            dongle_handle,
                            lc3plus_enc_config.sample_bits,
                            lc3plus_enc_config.channel_num,
                            lc3plus_enc_config.sample_rate,
                            lc3plus_enc_config.bit_rate,
                            lc3plus_enc_config.frame_interval,
                            lc3plus_enc_config.frame_samples,
                            lc3plus_enc_config.in_frame_size,
                            lc3plus_enc_config.out_frame_size,
                            lc3plus_enc_config.codec_mode);
                stream_codec_encoder_lc3plus_initialize(&(stream->callback.EntryPara));
#endif
            } else if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS) {
#if defined(AIR_CELT_ENC_V2_ENABLE)
                stream_codec_encoder_celt_v2_deinit(CELT_ENC_PORT_0, dongle_handle->sink);
                celt_enc_port_config_t celt_enc_config;
                celt_enc_config.sample_bits       = 16;
                celt_enc_config.channel_num       = dongle_handle->sink_info.bt_out.channel_num;
                celt_enc_config.sample_rate       = dongle_handle->sink_info.bt_out.sample_rate;
                celt_enc_config.bit_rate          = dongle_handle->sink_info.bt_out.bit_rate;
                celt_enc_config.process_frame_num = 2;
                celt_enc_config.frame_interval    = dongle_handle->sink_info.bt_out.frame_interval/celt_enc_config.process_frame_num;
                celt_enc_config.frame_samples     = dongle_handle->sink_info.bt_out.frame_samples/celt_enc_config.process_frame_num;
                celt_enc_config.in_frame_size     = dongle_handle->sink_info.bt_out.frame_samples/celt_enc_config.process_frame_num*sizeof(int16_t);
                celt_enc_config.out_frame_size    = dongle_handle->sink_info.bt_out.frame_size/celt_enc_config.process_frame_num;
                celt_enc_config.complexity        = 0;
                celt_enc_config.channel_mode      = CELT_ENC_CHANNEL_MODE_MULTI_MONO;
                stream_codec_encoder_celt_v2_init(CELT_ENC_PORT_0, dongle_handle->sink, &celt_enc_config);
                ULL_V2_LOG_I("[ULL Audio V2][DL][scenario %d-%d][handle 0x%x]reset opus info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                            dongle_handle->source->scenario_type,
                            dongle_handle->sink->scenario_type,
                            dongle_handle,
                            celt_enc_config.sample_bits,
                            celt_enc_config.channel_num,
                            celt_enc_config.sample_rate,
                            celt_enc_config.bit_rate,
                            celt_enc_config.frame_interval,
                            celt_enc_config.frame_samples,
                            celt_enc_config.in_frame_size,
                            celt_enc_config.out_frame_size,
                            celt_enc_config.complexity);
                stream_codec_encoder_celt_v2_initialize(&(stream->callback.EntryPara));
#endif /* AIR_CELT_ENC_V2_ENABLE */
            }
#endif
            sw_clk_skew_config_t sw_clk_skew_config;
            sw_clk_skew_config.channel                  = dongle_handle->source_info.usb_in.channel_num;
            if (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                sample_size = sizeof(int32_t);
                sw_clk_skew_config.bits = 32;
            } else {
                sample_size = sizeof(int16_t);
                sw_clk_skew_config.bits = 16;
            }
            sw_clk_skew_config.order                    = C_Flp_Ord_1;
            sw_clk_skew_config.skew_io_mode             = C_Skew_Inp;
            sw_clk_skew_config.skew_compensation_mode   = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
            sw_clk_skew_config.skew_work_mode           = SW_CLK_SKEW_CONTINUOUS;
            sw_clk_skew_config.max_output_size          = (dongle_handle->src_out_frame_samples*dongle_handle->sink_info.bt_out.frame_interval/dongle_handle->source_info.usb_in.frame_interval+1)*sample_size;
            sw_clk_skew_config.continuous_frame_size    = dongle_handle->src_out_frame_samples*dongle_handle->sink_info.bt_out.frame_interval/dongle_handle->source_info.usb_in.frame_interval*sample_size;
            stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);
            dongle_handle->clk_skew_port                = stream_function_sw_clk_skew_get_port(dongle_handle->source);
            stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
            stream_function_sw_clk_skew_initialize(&(stream->callback.EntryPara));
        } else {
            check_threshold = dongle_handle->source_info.usb_in.frame_max_num + 1;
            /* Add current process count control to prevent of last CCNI process with 0 frame error */
            if (dongle_handle->source_preload_count != 0) {
                if (unprocess_frames > 1) {
                    unprocess_frames -= 1;
                }
            }
        }
        if (unprocess_frames > check_threshold)
        {
            /* drop oldest data in the share buffer */
            source->streamBuffer.ShareBufferInfo.read_offset = (source->streamBuffer.ShareBufferInfo.write_offset+total_buffer_size-source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size*dongle_handle->source_info.usb_in.frame_max_num) % total_buffer_size;
            audio_transmitter_share_information_update_read_offset(source, source->streamBuffer.ShareBufferInfo.read_offset);
            audio_transmitter_share_information_fetch(source, NULL);
            ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING][handle 0x%x]unprocess frame is too much, %u, %u, %u, %u\r\n", 5, dongle_handle, unprocess_frames, dongle_handle->source_info.usb_in.frame_max_num, source->streamBuffer.ShareBufferInfo.read_offset, source->streamBuffer.ShareBufferInfo.write_offset);
            unprocess_frames = dongle_handle->source_info.usb_in.frame_max_num;
        }

        /* Copy all data from the share buffer */
        total_samples = 0;
        read_offset = source->streamBuffer.ShareBufferInfo.read_offset;
        src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + read_offset);
        current_frame = 0;
        while(current_frame < unprocess_frames)
        {
            /* get current frame info and current frame samples */
            frame_header = (audio_transmitter_frame_header_t *)src_buf;
            if (frame_header->payload_len != dongle_handle->source_info.usb_in.frame_size)
            {
                ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x]frame size is not matched, %u, %u\r\n", 3, dongle_handle, frame_header->payload_len, dongle_handle->source_info.usb_in.frame_size);
                AUDIO_ASSERT(0);
            }
            current_frame_samples = dongle_handle->source_info.usb_in.frame_samples;
            /* copy usb audio data from share buffer one by one frame */
            ull_audio_v2_dongle_dl_usb_data_copy(dongle_handle, stream, src_buf, current_frame_samples, total_samples);
            /* update total samples */
            total_samples += current_frame_samples;
            /* update copied frame number */
            current_frame += 1;
            /* change to the next frame */
            read_offset = (read_offset + source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) % total_buffer_size;
            src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + read_offset);
        }

        /* update stream samples */
        length = total_samples;
    }

    /* update stream status */
    sample_size = (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) ? sizeof(int16_t) : sizeof(int32_t);
    stream->callback.EntryPara.in_channel_num           = dongle_handle->source_info.usb_in.channel_num;
    stream->callback.EntryPara.out_channel_num          = dongle_handle->source_info.usb_in.channel_num;
    stream->callback.EntryPara.in_sampling_rate         = dongle_handle->source_info.usb_in.sample_rate/1000;
    stream->callback.EntryPara.in_size                  = length * sample_size;
    if (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
    {
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.feature_res   = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.sink_out_res  = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.process_res   = RESOLUTION_16BIT;
    }
    else
    {
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.feature_res   = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.sink_out_res  = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.process_res   = RESOLUTION_32BIT;
    }

    //hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    /* update state machine */
    dongle_handle->process_frames = unprocess_frames;
    dongle_handle->process_frames_total += unprocess_frames;
    dongle_handle->buffer0_remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port_0, 1) / sample_size;

    if (dongle_handle->stream_mode == ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_PREPARE_PACKET)
    {
        if (dongle_handle->data_status == ULL_AUDIO_V2_DONGLE_DL_DATA_IN_STREAM)
        {
            /* configure clock skew settings */
            dongle_handle->compen_samples = ull_audio_v2_dongle_dl_usb_clock_skew_check(dongle_handle, dongle_handle->process_frames*dongle_handle->src_out_frame_samples, &dongle_handle->buffer0_output_size);
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, dongle_handle->compen_samples);

            /* configure buffer output size, this setting will be changed by clock skew status */
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port_0, 0, dongle_handle->buffer0_output_size);
        }
        else
        {
            /* check process_frames */
            if (dongle_handle->process_frames == 0)
            {
                /* decrease fetch count */
                dongle_handle->fetch_count = 0;
                /* decrease preload count */
                dongle_handle->source_preload_count = 0;
                /* reset stream mode */
                dongle_handle->stream_mode = ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_STANDBY;
            }
        }
    }
    else
    {
        /* configure buffer output size, this setting should be always 0 because it is preload mode */
        stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port_0, 0, 0);

        /* check process_frames */
        if (dongle_handle->process_frames == 0)
        {
            /* there are must new usb data in this mode */
            ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x] stream preload mode frame is not right, %d\r\n", 2, dongle_handle, length);
            // AUDIO_ASSERT(0);
            /* reset preload count */
            dongle_handle->source_preload_count = 0;
            /* reset stream mode */
            dongle_handle->stream_mode = ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_STANDBY;
        }
    }
    //hal_nvic_restore_interrupt_mask(saved_mask);

#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (ull_audio_v2_dl_without_bt_link_mode_enable == true) {
        if ((dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) ||
            (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)) {
            uint32_t i, avail_query_size;
            stream_function_sw_mixer_channel_input_get_data_info(dongle_handle->mixer_member, 1, NULL, NULL, &avail_query_size);
            if (avail_query_size != 0) {
                stream_function_sw_mixer_member_input_buffers_clean(dongle_handle->mixer_member, false);
                ULL_V2_LOG_E("[ULL Audio V2][DL][copy_payload][handle 0x%x] detect stream mix fail %d", 2, dongle_handle, avail_query_size);
            }
            avail_query_size = stream_function_sw_buffer_get_channel_free_size(dongle_handle->buffer_port_0, 1);
            if (avail_query_size < stream->callback.EntryPara.in_size) {
                for (i = 0; i < dongle_handle->source_info.usb_in.channel_num; i++) {
                    stream_function_sw_buffer_reset_channel_buffer(dongle_handle->buffer_port_0, 1 + i, false);
                }
                ULL_V2_LOG_E("[ULL Audio V2][DL][copy_payload][handle 0x%x] detect sw buffer almost full %d", 2, dongle_handle, avail_query_size);
            }
        }
    }
#endif

    /* avoid payload length check error in here */
    length = (length > source->param.data_dl.max_payload_size) ? source->param.data_dl.max_payload_size : length;

    /* update timestamp for debug */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&dongle_handle->data_in_gpt_count);

#if ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_V2_LOG_I("[ULL Audio V2][DL][DEBUG][copy_payload]: %d, %d, %u, %d, %d", 5, unprocess_frames, total_samples, length, current_timestamp, hal_nvic_query_exception_number());
#endif /* ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG */

    return length;
}

ATTR_TEXT_IN_IRAM uint32_t ull_audio_v2_dongle_dl_source_get_new_read_offset(SOURCE source, uint32_t amount)
{
    UNUSED(amount);
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t total_buffer_size;
    n9_dsp_share_info_ptr ShareBufferInfo;
    uint32_t read_offset;
    uint32_t saved_mask;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    /* get new read offset */
    ShareBufferInfo = &source->streamBuffer.ShareBufferInfo;
    total_buffer_size = ShareBufferInfo->sub_info.block_info.block_size * ShareBufferInfo->sub_info.block_info.block_num;
    read_offset = (ShareBufferInfo->read_offset+ShareBufferInfo->sub_info.block_info.block_size*dongle_handle->process_frames) % total_buffer_size;
    hal_nvic_restore_interrupt_mask(saved_mask);

    return read_offset;
}

ATTR_TEXT_IN_IRAM void ull_audio_v2_dongle_dl_source_drop_postprocess(SOURCE source, uint32_t amount)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t saved_mask;
    UNUSED(amount);

#if ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    uint32_t remain_samples;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    uint32_t sample_size = (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) ? sizeof(int16_t) : sizeof(int32_t);
    remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port_0, 1) / sample_size;
    ULL_V2_LOG_I("[ULL Audio V2][DL][DEBUG][source_drop_postprocess][handle 0x%x]: %d, %d, %d, %d, %d, %d, %d, %d", 9, dongle_handle, dongle_handle->stream_mode, dongle_handle->fetch_count, dongle_handle->source_preload_count, dongle_handle->process_frames, dongle_handle->process_frames_total, remain_samples, current_timestamp, hal_nvic_query_exception_number());
#endif /* ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_LOG */

    /* update first packet state machine */
    if ((dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY) && (dongle_handle->process_frames != 0))
    {
        dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_READY;
    }

    /* update stream status */
    if (dongle_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_START)
    {
        dongle_handle->stream_status = ULL_AUDIO_V2_DONGLE_STREAM_RUNNING;
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->stream_mode == ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_PREPARE_PACKET)
    {
        /* decrease fetch count */
        dongle_handle->fetch_count = 0;
    }
    else if (dongle_handle->stream_mode == ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_PRELOAD)
    {
        /* decrease preload count */
        dongle_handle->source_preload_count = 0;
        /* reset stream mode */
        dongle_handle->stream_mode = ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_STANDBY;
    }
    /* reset process_frames */
    dongle_handle->process_frames = 0;
    hal_nvic_restore_interrupt_mask(saved_mask);
}

bool ull_audio_v2_dongle_dl_source_close(SOURCE source)
{
    UNUSED(source);

    return true;
}

ATTR_TEXT_IN_IRAM bool ull_audio_v2_dongle_dl_sink_get_avail_size(SINK sink, uint32_t *avail_size)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;

    /* change avail_size to actual frame size */
    *avail_size = dongle_handle->sink_info.bt_out.frame_size;

    return true;
}

ATTR_TEXT_IN_IRAM uint32_t ull_audio_v2_dongle_dl_sink_copy_payload(SINK sink, uint8_t *src_buf, uint32_t length)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;

    /* check parameter */
    if (length != dongle_handle->sink_info.bt_out.frame_size)
    {
        ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x]length is not right, %d, %d\r\n", 3, dongle_handle, length, dongle_handle->sink_info.bt_out.frame_size);
        AUDIO_ASSERT(0);
    }

    ull_audio_v2_dongle_dl_sink_internal_copy_payload(sink, src_buf, false);

    return length;
}

ATTR_TEXT_IN_IRAM bool ull_audio_v2_dongle_dl_sink_get_new_write_offset(SINK sink, U32 amount, uint32_t *new_write_offset)
{
    ull_audio_v2_dongle_dl_sink_internal_update_write_offset(sink, amount, new_write_offset);

    /* we will update the write offsets of the different share buffers in here directly, so return false to aviod the upper layer update write offset */
    return false;
}

ATTR_TEXT_IN_IRAM bool ull_audio_v2_dongle_dl_sink_query_notification(SINK sink, bool *notification_flag)
{
    UNUSED(sink);

    *notification_flag = false;

    return true;
}

ATTR_TEXT_IN_IRAM bool ull_audio_v2_dongle_dl_sink_send_data_ready_notification(SINK sink)
{
    UNUSED(sink);

    return true;
}

ATTR_TEXT_IN_IRAM void ull_audio_v2_dongle_dl_sink_flush_postprocess(SINK sink, uint32_t amount)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    ull_audio_v2_dongle_dl_handle_t *c_handle          = NULL;
    uint32_t                        i                  = 0;
    uint32_t                        remain_samples_0   = 0;
    int32_t                         frac_rpt           = 0;
    SINK                            c_sink             = NULL;
    bool                            sink_flag          = false;
    uint32_t                        duration           = 0;
    uint32_t                        codec_finish_count = 0;
    uint32_t saved_mask;
    uint32_t sample_size;
    UNUSED(sink);
    UNUSED(amount);

    /* Reset dummy status after an actual frame process is done */
    for (i = 0; i < DL_MAX_USB_CARD_NUMBER; i++) {
        g_ull_audio_v2_dongle_dl_dummy_process_cnt[i] = 0;
    }

    /* update time stamp */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&dongle_handle->data_out_gpt_count);

    /* audio dump */
    #ifdef AIR_AUDIO_DUMP_ENABLE
    #if ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_DUMP
    uint8_t *codec_in_data_address;
    uint32_t codec_in_data_frame_size;
    uint32_t blk_size;
    uint32_t blk_num;
    uint32_t blk_index;
    n9_dsp_share_info_ptr p_share_info;
    uint8_t *codec_out_data_address;
    uint32_t codec_out_data_frame_size;
    /* codec input dump */
    if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        /* dump channel 1 LC3+ codec in data */
        stream_codec_encoder_lc3plus_get_data_info(LC3PLUS_ENC_PORT_0, 1, &codec_in_data_address, &codec_in_data_frame_size);
        LOG_AUDIO_DUMP((uint8_t *)codec_in_data_address, codec_in_data_frame_size, AUDIO_SOURCE_IN_L);
        /* dump channel 2 LC3+ codec in data */
        stream_codec_encoder_lc3plus_get_data_info(LC3PLUS_ENC_PORT_0, 2, &codec_in_data_address, &codec_in_data_frame_size);
        LOG_AUDIO_DUMP((uint8_t *)codec_in_data_address, codec_in_data_frame_size, AUDIO_SOURCE_IN_R);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
        /* dump channel 1 Opus codec in data */
        stream_codec_encoder_celt_v2_get_data_info(CELT_ENC_PORT_0, 1, &codec_in_data_address, &codec_in_data_frame_size);
        LOG_AUDIO_DUMP((uint8_t *)codec_in_data_address, codec_in_data_frame_size, AUDIO_SOURCE_IN_L);
        /* dump channel 2 Opus codec in data */
        stream_codec_encoder_celt_v2_get_data_info(CELT_ENC_PORT_0, 2, &codec_in_data_address, &codec_in_data_frame_size);
        LOG_AUDIO_DUMP((uint8_t *)codec_in_data_address, codec_in_data_frame_size, AUDIO_SOURCE_IN_R);
#endif /* AIR_CELT_DEC_V2_ENABLE */
    }
#if defined(AIR_AUDIO_ULD_ENCODE_ENABLE)
    else if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_ULD) {
        /* dump channel 1 Opus codec in data */
        stream_codec_encoder_uld_get_data_info(dongle_handle->uld_port, 1, &codec_in_data_address, &codec_in_data_frame_size);
        LOG_AUDIO_DUMP((uint8_t *)codec_in_data_address, codec_in_data_frame_size, AUDIO_SOURCE_IN_L);
        /* dump channel 2 Opus codec in data */
        stream_codec_encoder_uld_get_data_info(dongle_handle->uld_port, 2, &codec_in_data_address, &codec_in_data_frame_size);
        LOG_AUDIO_DUMP((uint8_t *)codec_in_data_address, codec_in_data_frame_size, AUDIO_SOURCE_IN_R);
    }
#endif /* AIR_AUDIO_ULD_ENCODE_ENABLE */
    /* codec out dump */
    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->sink_info.bt_out.bt_info[i] == NULL)
        {
            continue;
        }
        else
        {
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->sink_info.bt_out.bt_info[i])->bt_link_info.share_info);
            blk_size     = p_share_info->sub_info.block_info.block_size;
            blk_num      = p_share_info->sub_info.block_info.block_num;
            blk_index    = dongle_handle->ccni_blk_index;
            if (blk_index < blk_num)
            {
                codec_out_data_address = (uint8_t *)(hal_memview_infrasys_to_dsp0(p_share_info->start_addr) + blk_size * blk_index);
                codec_out_data_frame_size = dongle_handle->sink_info.bt_out.frame_size + sizeof(ULL_AUDIO_V2_HEADER);
                LOG_AUDIO_DUMP((uint8_t *)codec_out_data_address, codec_out_data_frame_size, SINK_OUT1+i);
            }
        }
    }
    #endif /* ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_DUMP */
    #endif /* AIR_AUDIO_DUMP_ENABLE */

    /* output debug log */
    c_handle = ull_audio_v2_dongle_first_dl_handle;
    for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++)
    {
        c_sink = (SINK)(c_handle->owner);
        if (c_sink->transform != NULL)
        {
            if (sink == c_sink)
            {
                /* this stream is the last stream that all data are mixed */
                sink_flag = true;
                hal_gpt_get_duration_count(c_handle->ccni_in_gpt_count, c_handle->data_out_gpt_count, &duration);
            }
            else
            {
                sink_flag = false;
            }

#if (defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
            if ((c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) ||
                (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
                (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) {
                /* afe in case */
                //c_handle->cur_read_offset = c_sink->transform->source->streamBuffer.BufferInfo.ReadOffset;
                uint32_t src_1_tracking_rate = AFE_READ(ASM_FREQUENCY_2) * 100 / 4096;
                uint32_t src_2_tracking_rate = AFE_READ(ASM2_FREQUENCY_2) * 100 / 4096;
                ULL_V2_LOG_I("[ULL Audio V2][DL][scenario type %d][AFE IN] latency size %d wo[%d] ro[%d] afe wo[0x%x] base[0x%x] src1 rate %d src2 rate %d", 8,
                    c_sink->scenario_type,
                    c_handle->afe_buffer_latency_size,
                    c_handle->source->streamBuffer.BufferInfo.WriteOffset,
                    c_handle->source->streamBuffer.BufferInfo.ReadOffset,
                    c_handle->afe_vul_cur,
                    c_handle->afe_vul_base,
                    src_1_tracking_rate,
                    src_2_tracking_rate
                );
            }
#endif
            if ((c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) ||
                (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)) {
                sample_size = (c_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) ? sizeof(int16_t) : sizeof(int32_t);
                remain_samples_0 = stream_function_sw_buffer_get_channel_used_size(c_handle->buffer_port_0, 1) / sample_size;
                stream_function_sw_clk_skew_get_frac_rpt(c_handle->clk_skew_port, 1, &frac_rpt);
            }

            ULL_V2_LOG_I("[ULL Audio V2][DL][handle 0x%x][scenario type %d] %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x", 18,
                            c_handle,
                            c_sink->scenario_type,
                            sink_flag,
                            c_handle->mixer_status,
                            c_handle->first_packet_status,
                            c_handle->ccni_blk_index,
                            c_handle->fetch_count,
                            c_handle->process_frames_total,
                            c_handle->process_frames,
                            c_handle->buffer0_output_size,
                            remain_samples_0,
                            c_handle->clk_skew_count,
                            frac_rpt,
                            c_handle->compen_samples,
                            c_handle->ccni_in_bt_count,
                            c_handle->data_out_bt_count,
                            c_handle->ccni_in_gpt_count,
                            c_handle->data_out_gpt_count);

            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            if ((c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) ||
                (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1))
            {
                if (c_handle->stream_mode == ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_PREPARE_PACKET)
                {
                    /* decrease fetch count */
                    c_handle->fetch_count = 0;
                    /* decrease preload count */
                    c_handle->source_preload_count = 0;
                    /* reset stream mode */
                    c_handle->stream_mode = ULL_AUDIO_V2_DONGLE_STREAM_MODE_DL_STANDBY;
                    /* reset state machine */
                    c_handle->process_frames_total = 0;
                    hal_nvic_restore_interrupt_mask(saved_mask);
                }
                else
                {
                    hal_nvic_restore_interrupt_mask(saved_mask);
                    if (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_READY)
                    {
                        ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR][handle 0x%x] stream mode is not right, %d, %d, %d\r\n", 4, c_handle, c_handle->stream_mode, c_handle->fetch_count, c_handle->source_preload_count);
                    }
                }
            } else {
                hal_nvic_restore_interrupt_mask(saved_mask);
            }

            /* do usb data audio dump */
            #ifdef AIR_AUDIO_DUMP_ENABLE
            #if ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_DUMP
            uint32_t j, k, address, read_offset, total_buffer_size, channel_size;
            if ((c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) ||
                (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)) {
                total_buffer_size = c_sink->transform->source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * c_sink->transform->source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;
                channel_size = (c_sink->transform->source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t)) / c_handle->source_info.usb_in.channel_num;
                for (j = 0; j < c_handle->process_frames_total; j++) {
                    read_offset = (c_sink->transform->source->streamBuffer.ShareBufferInfo.read_offset + c_sink->transform->source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * j) % total_buffer_size;
                    address = c_sink->transform->source->streamBuffer.ShareBufferInfo.start_addr + read_offset + sizeof(audio_transmitter_frame_header_t);
                    for (k = 0; k < c_handle->source_info.usb_in.channel_num; k++) {
                        LOG_AUDIO_DUMP((uint8_t *)(address + k * channel_size), channel_size, DONGLE_AUDIO_USB_IN_I_1 + k + 8 * (c_handle->sub_id - AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0));
                    }
                }
            }
            #endif /* ULL_AUDIO_V2_DONGLE_DL_PATH_DEBUG_DUMP */
            #endif /* AIR_AUDIO_DUMP_ENABLE */

#if defined(AIR_SILENCE_DETECTION_ENABLE) || defined(AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE)
            uint8_t *data_buf_1_head;
            uint8_t *data_buf_1_tail;
            uint32_t data_buf_1_size;
#endif /* defined(AIR_SILENCE_DETECTION_ENABLE) || defined(AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE) */

#ifdef AIR_SILENCE_DETECTION_ENABLE
            DSP_CALLBACK_PTR callback_ptr;
            uint8_t *p_curr_buf;
            uint32_t curr_idx, data_size_by_channel;
            audio_scenario_type_t audio_scenario_type;
            ull_audio_v2_dongle_silence_detection_handle_t *silence_detection_handle;
            /* get scenario and handle */
            audio_scenario_type = AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0+c_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0;
            silence_detection_handle = &ull_audio_v2_dongle_silence_detection_handle[c_handle->sub_id-AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0];
            if (silence_detection_handle->enable)
            {
                /* copy audio data into the silence detection buffer */
                data_size_by_channel = c_handle->sink_info.bt_out.frame_samples * sizeof(int32_t);
                callback_ptr = DSP_Callback_Get(sink->transform->source, sink);
                for (curr_idx = 0; curr_idx < callback_ptr->EntryPara.out_channel_num; curr_idx++) {
                    p_curr_buf = (uint8_t *)(silence_detection_handle->data_buf) + curr_idx * data_size_by_channel;
                    if (c_handle->first_packet_status != ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY)
                    {
                        /* data_buf_1_head & data_buf_2_head is pointer the frame start address */
                        stream_function_sw_mixer_channel_input_get_data_info(c_handle->mixer_member, curr_idx + 1, &data_buf_1_head, &data_buf_1_tail, &data_buf_1_size);
                        if (c_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
                        {
                            ShareBufferCopy_D_16bit_to_D_32bit_1ch((uint16_t* )data_buf_1_head, (uint32_t* )p_curr_buf, c_handle->sink_info.bt_out.frame_samples);
                        }
                        else
                        {
                            memcpy(p_curr_buf, data_buf_1_head, data_size_by_channel);
                        }
                    }
                    else
                    {
                        memset(p_curr_buf, 0, data_size_by_channel);
                    }
                }
                silence_detection_handle->data_size = data_size_by_channel * callback_ptr->EntryPara.out_channel_num;

                /* Set check channel to only 1 as the data empty is detected */
                if (c_handle->first_packet_status != ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY) {
                    silence_detection_handle->total_channel = callback_ptr->EntryPara.out_channel_num;
                } else {
                    silence_detection_handle->total_channel = 1;
                }
                silence_detection_handle->data_size_channel = data_size_by_channel;

                /* send msg to DTM task to do silence detection */
                if (DTM_query_queue_avail_number() >= 2) {
                    DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_PROCESS, (uint32_t)audio_scenario_type, false);
                } else {
                    ULL_V2_LOG_I("[ULL Audio V2][DL]scenario %d-%d abort silence detection, DTM queue full\r\n", 2,
                                            c_sink->transform->source->param.data_dl.scenario_type,
                                            c_sink->transform->source->param.data_dl.scenario_sub_id);
                }
            }
#endif /* AIR_SILENCE_DETECTION_ENABLE */

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
            if ((c_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) && (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_NOT_READY) && (c_handle->process_frames != 0))
            {
                DTM_enqueue(DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_ENABLE, AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0, false);
            }
            else if ((c_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) && (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_READY) && (c_handle->process_frames != 0))
            {
                /* copy audio data into the silence detection buffer */
                ull_audio_v2_dongle_vol_balance_handle.data_size = c_handle->sink_info.bt_out.frame_samples * sizeof(int32_t);
                /* data_buf_1_head & data_buf_2_head is pointer the frame start address */
                stream_function_sw_mixer_channel_input_get_data_info(c_handle->mixer_member, 1, &data_buf_1_head, &data_buf_1_tail, &data_buf_1_size);
                if (c_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
                {
                    ShareBufferCopy_D_16bit_to_D_32bit_1ch((uint16_t* )data_buf_1_head, (uint32_t* )(ull_audio_v2_dongle_vol_balance_handle.data_buf), c_handle->sink_info.bt_out.frame_samples);
                }
                else
                {
                    memcpy(ull_audio_v2_dongle_vol_balance_handle.data_buf, data_buf_1_head, ull_audio_v2_dongle_vol_balance_handle.data_size);
                }
                DTM_enqueue(DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_PROCESS, AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0, false);
            }
            else if ((c_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) && (c_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_READY) && (c_handle->process_frames == 0))
            {
                /* copy silence data into the silence detection buffer */
                ull_audio_v2_dongle_vol_balance_handle.data_size = c_handle->sink_info.bt_out.frame_samples * sizeof(int32_t);
                memset(ull_audio_v2_dongle_vol_balance_handle.data_buf, 0, ull_audio_v2_dongle_vol_balance_handle.data_size);
                DTM_enqueue(DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_PROCESS, AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0, false);
            }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
        }

        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }

    /* output warning log if the process time is too long */
    if (duration >= DL_PROCESS_TIME_MAX)
    {
        c_handle = ull_audio_v2_dongle_first_dl_handle;
        for (i = 0; i < (uint32_t)ull_audio_v2_dongle_first_dl_handle->total_number; i++)
        {
            c_sink = (SINK)(c_handle->owner);
            if (c_sink->transform != NULL)
            {
                codec_finish_count = 0;
                if (c_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
                {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
                    stream_codec_encoder_lc3plus_get_finish_gpt_count(LC3PLUS_ENC_PORT_0, &codec_finish_count);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
                }
                else if (c_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
                {
#if defined(AIR_CELT_DEC_V2_ENABLE)
                    stream_codec_encoder_celt_v2_get_finish_gpt_count(CELT_ENC_PORT_0, &codec_finish_count);
#endif /* AIR_CELT_DEC_V2_ENABLE */
                }
#if defined(AIR_AUDIO_ULD_ENCODE_ENABLE)
                else if (c_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_ULD) {

                    stream_codec_encoder_uld_get_finish_gpt_count(c_handle->uld_port, &codec_finish_count);
                }
#endif /* AIR_AUDIO_ULD_ENCODE_ENABLE */
                if ((c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) ||
                    (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)) {
                    ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING][handle 0x%x] process time is over, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", 11,
                                    c_handle,
                                    duration,
                                    c_handle->ccni_in_bt_count,
                                    c_handle->data_out_bt_count,
                                    c_handle->ccni_in_gpt_count,
                                    c_handle->buffer_port_0->finish_gpt_count,
                                    c_handle->clk_skew_port->finish_gpt_count,
                                    c_handle->gain_port->finish_gpt_count,
                                    c_handle->mixer_member->finish_gpt_count,
                                    codec_finish_count,
                                    c_handle->data_out_gpt_count);
                }
#if (defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
                else if ((c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) ||
                            (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
                            (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) {
                    ULL_V2_LOG_W("[ULL Audio V2][DL][WARNNING][handle 0x%x] process time is over, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", 11,
                                    c_handle,
                                    duration,
                                    c_handle->ccni_in_bt_count,
                                    c_handle->data_out_bt_count,
                                    c_handle->ccni_in_gpt_count,
                                    0, // afe in has no sw buffer
                                    0, // afe in has no clk skew
                                    c_handle->gain_port->finish_gpt_count,
                                    c_handle->mixer_member->finish_gpt_count,
                                    codec_finish_count,
                                    c_handle->data_out_gpt_count);
                }
#endif /* afe in type */
            }
            /* switch to the next dl stream */
            c_handle = c_handle->next_dl_handle;
        }
    }
}

bool ull_audio_v2_dongle_dl_sink_close(SINK sink)
{
    UNUSED(sink);

    return true;
}

/******************************************************************************/
/*               ULL audio 2.0 dongle UL path Public Functions                */
/******************************************************************************/
ATTR_TEXT_IN_IRAM bool ull_audio_v2_dongle_ul_fetch_time_is_arrived(ull_audio_v2_dongle_ul_handle_t *dongle_handle, uint32_t bt_clk)
{
    bool ret = false;
    uint32_t fetch_anchor_previous_10;

    if ((dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED) || (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY))
    {
        fetch_anchor_previous_10 = (dongle_handle->source_info.bt_in.fetch_anchor + 0x10000000 - (dongle_handle->source_info.bt_in.bt_retry_window / dongle_handle->source_info.bt_in.bt_interval + 1) * dongle_handle->source_info.bt_in.bt_interval) & 0x0fffffff;
        if (fetch_anchor_previous_10 < dongle_handle->source_info.bt_in.fetch_anchor)
        {
            if (bt_clk >= dongle_handle->source_info.bt_in.fetch_anchor)
            {
                /* --------- ........ --------- fetch_anchor_previous ---------- fetch_anchor ---------- bt_clk --------- ........ -------- */
                ret = true;
            }
            else if ((bt_clk < dongle_handle->source_info.bt_in.fetch_anchor) && (bt_clk < fetch_anchor_previous_10))
            {
                /* --------- ........ ---------- bt_clk --------- ........ --------- fetch_anchor_previous_10  --------- fetch_anchor_previous ---------- fetch_anchor -------- */
                ret = true;
            }
        }
        else
        {
            if ((bt_clk >= dongle_handle->source_info.bt_in.fetch_anchor) && (bt_clk < fetch_anchor_previous_10))
            {
                /* ---------- fetch_anchor ---------- bt_clk --------- ........ --------- fetch_anchor_previous_10--------- fetch_anchor_previous -------- */
                /* --------- fetch_anchor_previous ---------- fetch_anchor ---------- bt_clk --------- ........ --------- fetch_anchor_previous_10 --------- */
                ret = true;
            }
        }
    }

    return ret;
}

uint32_t ull_audio_v2_dongle_ul_get_stream_in_max_size_each_channel(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    uint32_t stream_in_size = 0;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */
#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */
            if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
            {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
                stream_in_size = 2*((dongle_handle->source_info.bt_in.frame_size+sizeof(lc3plus_dec_frame_status_t)+3)/4*4);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
            }
            else
            {
#if defined(AIR_CELT_DEC_V2_ENABLE)
                stream_in_size = 2*((dongle_handle->source_info.bt_in.frame_size+sizeof(celt_dec_frame_status_t)+3)/4*4);
#endif /* AIR_CELT_DEC_V2_ENABLE */
            }
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return stream_in_size;
}

uint32_t ull_audio_v2_dongle_ul_get_stream_in_channel_number(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    uint32_t channel_number = 0;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */
#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */
            channel_number = dongle_handle->source_info.bt_in.channel_num;
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return channel_number;
}

stream_samplerate_t ull_audio_v2_dongle_ul_get_stream_in_sampling_rate_each_channel(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    stream_samplerate_t samplerate = 0;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */
#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */
            samplerate = dongle_handle->source_info.bt_in.sample_rate/1000;
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return samplerate;
}

uint32_t ull_audio_v2_dongle_ul_get_stream_out_max_size_each_channel(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    uint32_t stream_out_size = 0;
    uint32_t frame_samples;
    uint32_t frame_sample_size;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
            frame_samples = max(dongle_handle->sink_info.usb_out.frame_samples, dongle_handle->source_info.bt_in.frame_samples*1000/dongle_handle->source_info.bt_in.frame_interval);
            if ((dongle_handle->sink_info.usb_out.sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE) || (dongle_handle->source_info.bt_in.sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE))
            {
                frame_sample_size = sizeof(int32_t);
            }
            else
            {
                frame_sample_size = sizeof(int16_t);
            }
            stream_out_size = 2*(dongle_handle->sink_info.usb_out.frame_max_num*frame_samples*frame_sample_size);
            break;

#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
            frame_samples = max(dongle_handle->sink_info.line_out.frame_samples, dongle_handle->source_info.bt_in.frame_samples*1000/dongle_handle->source_info.bt_in.frame_interval);
            if ((dongle_handle->sink_info.line_out.sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE) || (dongle_handle->source_info.bt_in.sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE))
            {
                frame_sample_size = sizeof(int32_t);
            }
            else
            {
                frame_sample_size = sizeof(int16_t);
            }
            stream_out_size = 2*(dongle_handle->sink_info.line_out.frame_max_num*frame_samples*frame_sample_size);
            break;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
            frame_samples = max(dongle_handle->sink_info.i2s_mst_out.frame_samples, dongle_handle->source_info.bt_in.frame_samples*1000/dongle_handle->source_info.bt_in.frame_interval);
            if ((dongle_handle->sink_info.i2s_mst_out.sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE) || (dongle_handle->source_info.bt_in.sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE))
            {
                frame_sample_size = sizeof(int32_t);
            }
            else
            {
                frame_sample_size = sizeof(int16_t);
            }
            stream_out_size = 2*(dongle_handle->sink_info.i2s_mst_out.frame_max_num*frame_samples*frame_sample_size);
            break;
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
            frame_samples = max(dongle_handle->sink_info.i2s_slv_out.frame_samples, dongle_handle->source_info.bt_in.frame_samples*1000/dongle_handle->source_info.bt_in.frame_interval);
            if ((dongle_handle->sink_info.i2s_slv_out.sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE) || (dongle_handle->source_info.bt_in.sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE))
            {
                frame_sample_size = sizeof(int32_t);
            }
            else
            {
                frame_sample_size = sizeof(int16_t);
            }
            stream_out_size = 2*(dongle_handle->sink_info.i2s_slv_out.frame_max_num*frame_samples*frame_sample_size);
            break;
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return stream_out_size;
}

uint32_t ull_audio_v2_dongle_ul_get_stream_out_channel_number(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    uint32_t channel_number = 0;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
            channel_number = max(dongle_handle->sink_info.usb_out.channel_num, dongle_handle->source_info.bt_in.channel_num);
            break;

#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
            channel_number = max(dongle_handle->sink_info.line_out.channel_num, dongle_handle->source_info.bt_in.channel_num);
            break;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
            channel_number = max(dongle_handle->sink_info.i2s_mst_out.channel_num, dongle_handle->source_info.bt_in.channel_num);
            break;
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
            channel_number = max(dongle_handle->sink_info.i2s_slv_out.channel_num, dongle_handle->source_info.bt_in.channel_num);
            break;
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return channel_number;
}

stream_samplerate_t ull_audio_v2_dongle_ul_get_stream_out_sampling_rate_each_channel(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    stream_samplerate_t samplerate = 0;
    UNUSED(source);
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
            samplerate = dongle_handle->sink_info.usb_out.sample_rate/1000;
            break;

#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
            samplerate = dongle_handle->sink_info.line_out.sample_rate/1000;
            break;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
            samplerate = dongle_handle->sink_info.i2s_mst_out.sample_rate/1000;
            break;
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
            samplerate = dongle_handle->sink_info.i2s_slv_out.sample_rate/1000;
            break;
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return samplerate;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_ul_init(SOURCE source, SINK sink, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    /* get handle for application config */
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = ull_audio_v2_dongle_ul_get_handle(source);
    uint32_t saved_mask;

    source->param.bt_common.scenario_param.dongle_handle = (void *)dongle_handle;
    dongle_handle->source = source;
    dongle_handle->sink   = sink;
    dongle_handle->sub_id = bt_common_open_param->scenario_sub_id;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->total_number == 1)
    {
        /* reset state machine */
        ull_audio_v2_ul_packet_anchor_in_mixer = UL_PACKET_ANCHOR_IN_MIXER_INVAILD_VALUE;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    /* task config */
    source->taskId = DAV_TASK_ID;
    sink->taskid = DAV_TASK_ID;

    /* init audio info */
    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
            ull_audio_v2_dongle_ul_usb_out_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            break;

#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
            ull_audio_v2_dongle_ul_line_out_init(dongle_handle, (au_afe_open_param_p)audio_transmitter_open_param, bt_common_open_param);
            break;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
            ull_audio_v2_dongle_ul_i2s_mst_out_init(dongle_handle, (au_afe_open_param_p)audio_transmitter_open_param, bt_common_open_param);
            break;
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
            ull_audio_v2_dongle_ul_i2s_slv_out_init(dongle_handle, (au_afe_open_param_p)audio_transmitter_open_param, bt_common_open_param);
            break;
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

        default:
            AUDIO_ASSERT(0);
            break;
    }

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* lock sleep because sleep wake-up time will consume the stream process time */
    hal_sleep_manager_lock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif

    /* register ccni handler */
    bt_common_register_isr_handler(ull_audio_v2_dongle_ul_ccni_handler);

    /* stream status update */
    dongle_handle->stream_status = ULL_AUDIO_V2_DONGLE_STREAM_INIT;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_ul_deinit(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    uint32_t saved_mask;
    UNUSED(sink);

    /* unregister ccni handler */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->total_number == 1)
    {
        /* unregister ccni handler */
        bt_common_unregister_isr_handler(ull_audio_v2_dongle_ul_ccni_handler);
        /* reset state machine */
        ull_audio_v2_ul_packet_anchor_in_mixer = UL_PACKET_ANCHOR_IN_MIXER_INVAILD_VALUE;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    /* deinit audio info */
    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
            ull_audio_v2_dongle_ul_usb_out_deinit(dongle_handle);
            break;

#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
            ull_audio_v2_dongle_ul_line_out_deinit(dongle_handle);
            break;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
            ull_audio_v2_dongle_ul_i2s_mst_out_deinit(dongle_handle);
            break;
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
            ull_audio_v2_dongle_ul_i2s_slv_out_deinit(dongle_handle);
            break;
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

        default:
            AUDIO_ASSERT(0);
            break;
    }

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* unlock sleep */
    hal_sleep_manager_unlock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif

    /* stream status update */
    dongle_handle->stream_status = ULL_AUDIO_V2_DONGLE_STREAM_DEINIT;

    /* release handle */
    ull_audio_v2_dongle_ul_release_handle(dongle_handle);
    source->param.bt_common.scenario_param.dongle_handle = NULL;
}

void ull_audio_v2_dongle_ul_start(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY;
            break;

#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY;
            ull_audio_v2_dongle_ul_line_out_start(dongle_handle);
            break;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY;
            ull_audio_v2_dongle_ul_i2s_mst_out_start(dongle_handle);
            break;
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY;
            ull_audio_v2_dongle_ul_i2s_slv_out_start(dongle_handle);
            break;
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

        default:
            AUDIO_ASSERT(0);
            break;
    }

    /* stream status update */
    dongle_handle->stream_status = ULL_AUDIO_V2_DONGLE_STREAM_START;
}

void ull_audio_v2_dongle_ul_stop(SOURCE source, SINK sink)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    UNUSED(sink);

    switch (dongle_handle->sub_id)
    {
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0:
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY;
            break;

#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT:
            ull_audio_v2_dongle_ul_line_out_stop(dongle_handle);
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY;
            break;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0:
            ull_audio_v2_dongle_ul_i2s_mst_out_stop(dongle_handle);
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY;
            break;
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        case AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0:
            ull_audio_v2_dongle_ul_i2s_slv_out_stop(dongle_handle);
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY;
            break;
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

        default:
            AUDIO_ASSERT(0);
            break;
    }

    /* stream status update */
    dongle_handle->stream_status = ULL_AUDIO_V2_DONGLE_STREAM_START;
}

bool ull_audio_v2_dongle_ul_config(SOURCE source, stream_config_type type, U32 value)
{
    bool ret = true;
    ull_audio_v2_dongle_runtime_config_param_p config_param = (ull_audio_v2_dongle_runtime_config_param_p)value;
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    uint32_t vol_ch;
    int32_t vol_gain;
    sw_gain_config_t old_config;
    uint32_t ch_number = 0;
    uint32_t ch_connection = 0;
    uint32_t mixer_connection_status;
    uint32_t last_mixer_connection_status;
    uint32_t i;
    UNUSED(source);
    UNUSED(type);
    UNUSED(value);

    switch (config_param->config_operation)
    {
        case ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_UL_VOL_INFO:
            vol_ch = config_param->config_param.vol_gain.vol_ch;
            vol_gain = config_param->config_param.vol_gain.vol_gain;
            if (vol_ch != 0)
            {
                stream_function_sw_gain_get_config(dongle_handle->gain_port, vol_ch, &old_config);
                ULL_V2_LOG_I("[ULL Audio V2][UL][config][handle 0x%x]scenario %d-%d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 6,
                            dongle_handle,
                            source->param.bt_common.scenario_type,
                            source->param.bt_common.scenario_sub_id,
                            vol_ch,
                            old_config.target_gain,
                            vol_gain);
                stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, vol_ch, vol_gain);
            }
            else
            {
                vol_ch = 1;
                stream_function_sw_gain_get_config(dongle_handle->gain_port, vol_ch, &old_config);
                ULL_V2_LOG_I("[ULL Audio V2][UL][config][handle 0x%x]scenario %d-%d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 6,
                            dongle_handle,
                            source->param.bt_common.scenario_type,
                            source->param.bt_common.scenario_sub_id,
                            vol_ch,
                            old_config.target_gain,
                            vol_gain);
                stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, vol_ch, vol_gain);
                vol_ch = 2;
                stream_function_sw_gain_get_config(dongle_handle->gain_port, vol_ch, &old_config);
                ULL_V2_LOG_I("[ULL Audio V2][UL][config][handle 0x%x]scenario %d-%d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 6,
                            dongle_handle,
                            source->param.bt_common.scenario_type,
                            source->param.bt_common.scenario_sub_id,
                            vol_ch,
                            old_config.target_gain,
                            vol_gain);
                stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, vol_ch, vol_gain);
            }
            break;

        case ULL_AUDIO_V2_DONGLE_CONFIG_OP_SET_UL_CH_INPUT_SOURCE:
            last_mixer_connection_status = dongle_handle->mixer_connection_status;
            ch_number = config_param->config_param.connection_info.ch_choose;
            if (ch_number != 0)
            {
                ch_connection = config_param->config_param.connection_info.ch_connection[ch_number-1];
                mixer_connection_status = last_mixer_connection_status;
                mixer_connection_status &= ~(0xff<<(8*(ch_number-1)));
                mixer_connection_status |= ch_connection<<(8*(ch_number-1));
                ull_audio_v2_dongle_ul_mixer_all_stream_connections_update(dongle_handle, false, mixer_connection_status);
                dongle_handle->mixer_connection_status = mixer_connection_status;
            }
            else
            {
                ch_connection = config_param->config_param.connection_info.ch_connection[0];
                mixer_connection_status = last_mixer_connection_status;
                for (i = 0; i < 2; i++)
                {
                    mixer_connection_status &= ~(0xff<<(8*(i)));
                    mixer_connection_status |= ch_connection<<(8*(i));
                }
                ull_audio_v2_dongle_ul_mixer_all_stream_connections_update(dongle_handle, false, mixer_connection_status);
                dongle_handle->mixer_connection_status = mixer_connection_status;
            }
            ULL_V2_LOG_I("[ULL Audio V2][UL][config][handle 0x%x]scenario %d-%d change channel %d status to 0x%x, connection status = from 0x%x to 0x%x\r\n", 7,
                        dongle_handle,
                        source->param.bt_common.scenario_type,
                        source->param.bt_common.scenario_sub_id,
                        ch_number,
                        ch_connection,
                        last_mixer_connection_status,
                        dongle_handle->mixer_connection_status);
            break;

        default:
            ULL_V2_LOG_E("[ULL Audio V2][UL][config] unknow cmd:%d\r\n", 1, config_param->config_operation);
            break;
    }

    return ret;
}

ATTR_TEXT_IN_IRAM bool ull_audio_v2_dongle_ul_source_get_avail_size(SOURCE source, uint32_t *avail_size)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t saved_mask;

    /* get current bt clk */
    MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);

    if (dongle_handle->fetch_count != 0)
    {
        /* If there is fetch requirement, we must wake up the stream even there is no packet on the share buffer */
        *avail_size = dongle_handle->source_info.bt_in.frame_size;
    }
    else if (ull_audio_v2_dongle_ul_fetch_time_is_arrived(dongle_handle, bt_clk))
    {
        /* If there is data in the share buffer, we must wake up the stream to process it */
        *avail_size = dongle_handle->source_info.bt_in.frame_size;
        /* USB-out case: latch unprocess frame count in the share buffer in irq level */
        if ((hal_nvic_query_exception_number() > 0) && (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0))
        {
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->sink_info.usb_out.mcu_frame_count_latch = dongle_handle->sink_info.usb_out.mcu_frame_count;
            hal_nvic_restore_interrupt_mask(saved_mask);
        }
    }
    else
    {
        *avail_size = 0;
    }

    return true;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ uint32_t ull_audio_v2_dongle_ul_source_copy_payload(SOURCE source, uint8_t *dst_buf, uint32_t length)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(source, source->transform->sink);
    uint32_t saved_mask;
    ull_audio_v2_dongle_first_packet_status_t first_packet_status;
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t remain_samples;
    uint32_t frame_samples;
    uint32_t i;
    n9_dsp_share_info_ptr p_share_info;
    uint16_t read_offset;
    ULL_AUDIO_V2_HEADER *p_ull_audio_header;
    uint8_t *src_buf;
    int32_t sample_size = 0;
#if defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
    uint32_t play_en_nclk;
    uint16_t play_en_intra_clk;
    uint32_t play_en_clk;
    uint16_t play_en_phase;
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

    UNUSED(length);

    /* dongle status check */
    switch (dongle_handle->stream_status)
    {
        /* In this status stage, it means the stream is not ready */
        case ULL_AUDIO_V2_DONGLE_STREAM_DEINIT:
        case ULL_AUDIO_V2_DONGLE_STREAM_INIT:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->fetch_count = 0;
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        /* In this status stage, we will check if the newest packet is suitable for playing */
        case ULL_AUDIO_V2_DONGLE_STREAM_START:
            /* get current bt clock */
            MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);
            first_packet_status = ull_audio_v2_dongle_ul_first_packet_check(dongle_handle, bt_clk);
            switch (first_packet_status)
            {
                case ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_NOT_READY:
                case ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_TIMEOUT:
                    /* reset fetch count */
                    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                    dongle_handle->fetch_count = 0;
                    hal_nvic_restore_interrupt_mask(saved_mask);
                    break;

                case ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY:
                    //hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                    /* update stream status */
                    dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY;
                    dongle_handle->stream_status = ULL_AUDIO_V2_DONGLE_STREAM_RUNNING;
#if defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
                    if ((dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT) || (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0) || (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0))
                    {
                        if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT) {
                            hal_audio_memory_parameter_t *mem_handle = &dongle_handle->sink->param.audio.mem_handle;
                            /* get agent */
                            hal_audio_agent_t agent;
                            agent = hal_memory_convert_agent(mem_handle->memory_select);
                            /* enable AFE irq here */
                            hal_memory_set_irq_enable(agent, HAL_AUDIO_CONTROL_ON);
                        } else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0) {
                            hal_audio_memory_parameter_t *mem_handle = &dongle_handle->sink->param.audio.mem_handle;
                            /* get agent */
                            hal_audio_agent_t agent;
                            agent = hal_memory_convert_agent(mem_handle->memory_select);
                            /* enable AFE irq here */
                            hal_memory_set_irq_enable(agent, HAL_AUDIO_CONTROL_ON);
                        } else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0) {
                            // prefill data into hwsrc input buffer
                            AFE_WRITE(ASM_CH01_IBUF_WRPNT + 0x100 * dongle_handle->sink->param.audio.AfeBlkControl.u4asrcid, AFE_GET_REG(ASM_IBUF_SADR + 0x100 * dongle_handle->sink->param.audio.AfeBlkControl.u4asrcid) +
                                dongle_handle->sink->streamBuffer.BufferInfo.WriteOffset);
                        }
                        /* set play en to enable agent */
                        play_en_clk = dongle_handle->source_info.bt_in.play_en_bt_clk;
                        play_en_phase = 0;
                        if (play_en_phase != 0)
                        {
                            /* clk-(625-(dl_retransmission_window_phase%625)) == clk-1+625-(625-(dl_retransmission_window_phase%625)) */
                            play_en_clk = (play_en_clk+0x10000000-1) & 0x0fffffff;
                            // play_en_phase = 625 - (625-play_en_phase);
                        }
                        MCE_TransBT2NativeClk(play_en_clk, play_en_phase, &play_en_nclk, &play_en_intra_clk, BT_CLK_Offset);
                        /* Note: prefill buffer length is 4 samples, so read offset need to be add 32(4sample*4byte*2ch) for debugging the latency */
                        AFE_SET_REG(AFE_MEMIF_PBUF_SIZE, 0, 3 << 0); //reduce pbuffer size
                        AFE_SET_REG(AFE_MEMIF_MINLEN, 1, 0XF); //reduce pbuffer size
                        hal_audio_afe_set_play_en(play_en_nclk, (uint32_t)play_en_intra_clk);
                        /* stop timer */
                        ull_audio_v2_dongle_ul_software_timer_stop(dongle_handle);
                        /* update state machine */
                        dongle_handle->fetch_count = 0;
                        ULL_V2_LOG_I("[ULL Audio V2][UL] Play_En config with bt_clk:0x%08x, bt_phase:0x%08x, play_en_nclk:0x%08x, play_en_intra_clk:0x%08x", 4, play_en_clk, play_en_phase, play_en_nclk, play_en_intra_clk);
                    }
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */
                    //hal_nvic_restore_interrupt_mask(saved_mask);
                    break;

                /* Error status */
                default:
                    AUDIO_ASSERT(0);
                    break;
            }
            break;

        /* In this status stage, the stream is started and we will set flag to fetch a new packet */
        case ULL_AUDIO_V2_DONGLE_STREAM_RUNNING:
            break;

        /* Error status */
        default:
            AUDIO_ASSERT(0);
            break;
    }

    /* process the frame when the stream is running */
    dongle_handle->drop_frames = 0;
    MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);
    if (dongle_handle->stream_status == ULL_AUDIO_V2_DONGLE_STREAM_RUNNING)
    {
        if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_READY)
        {
            if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0)
            {
                /* usb-out case */
                sample_size = (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) ? sizeof(int16_t) : sizeof(int32_t);
                if (dongle_handle->buffer_first_output_done == false)
                {
                    frame_samples = dongle_handle->source_info.bt_in.sample_rate/1000*UL_USB_PREFILL_SAMPLES_FRAME;
                    dongle_handle->buffer_first_output_done = true;
                }
                else
                {
                    frame_samples = 0;
                }
                /* first packet must not be timeout, so do not need to if the new packet is timeout here */
                /* fetch first packet data into sw buffer but not play */
                if (ull_audio_v2_dongle_ul_fetch_time_is_arrived(dongle_handle, bt_clk) == true)
                {
                    /* do decode */
                    dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_NORMAL;
                    dongle_handle->buffer_output_size = dongle_handle->buffer_default_output_size + frame_samples*sample_size;
                }
                else
                {
                    /* bypass decoder */
                    dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER;
                    dongle_handle->buffer_output_size = frame_samples*sample_size;
                }
            }
#if defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
            else
            {
                /* line out and i2s mst/i2s slve out case */
                dongle_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED;
                dongle_handle->buffer_output_size = 0;
                /* check if the fetch time of the first packet is arrived */
                if (ull_audio_v2_dongle_ul_fetch_time_is_arrived(dongle_handle, bt_clk) == true)
                {
                    /* do decode */
                    dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_NORMAL;
                }
                else
                {
                    /* bypass decoder */
                    dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER;
                }
            }
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */
        }
        else if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED)
        {
            sample_size = (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) ? sizeof(int16_t) : sizeof(int32_t);
            if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0)
            {
                frame_samples = dongle_handle->source_info.bt_in.sample_rate/1000;
                remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sample_size;
                /* process timeout packets */
                ull_audio_v2_dongle_ul_process_timeout_packets(dongle_handle, bt_clk, bt_phase);
                /* check if the fetch time is arrived */
                if (ull_audio_v2_dongle_ul_fetch_time_is_arrived(dongle_handle, bt_clk) == true)
                {
                    /* do decode */
                    dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_NORMAL;
                    dongle_handle->buffer_output_size = dongle_handle->buffer_default_output_size + remain_samples/frame_samples*frame_samples*sample_size;
                }
                else
                {
                    /* bypass decoder */
                    dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER;
                    dongle_handle->buffer_output_size = remain_samples/frame_samples*frame_samples*sample_size;
                }
            }
#if defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
            else
            {
                /* do not check remain samples on the line-out and the i2s-mst/i2s-slv out case */
                remain_samples = 0;
                /* process timeout packets */
                ull_audio_v2_dongle_ul_process_timeout_packets(dongle_handle, bt_clk, bt_phase);
                /* check if the fetch time is arrived */
                if (ull_audio_v2_dongle_ul_fetch_time_is_arrived(dongle_handle, bt_clk) == true)
                {
                    /* do decode */
                    dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_NORMAL;
                    dongle_handle->buffer_output_size = 0;
                }
                else
                {
                    /* bypass decoder */
                    dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER;
                    dongle_handle->buffer_output_size = 0;
                }
            }
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) || defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) || defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */
        }
        else
        {
            /* bypass decoder */
            dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER;
            dongle_handle->buffer_output_size = 0;
        }
    }
    else
    {
        /* bypass decoder */
        dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER;
        dongle_handle->buffer_output_size = 0;
    }

    /* check if other stream has process this packet */
    if (dongle_handle->data_status == ULL_AUDIO_V2_DONGLE_UL_DATA_NORMAL)
    {
        if (ull_audio_v2_dongle_ul_packet_is_processed_by_other_stream(dongle_handle))
        {
            /* update status for bypass decoder */
            dongle_handle->data_status = ULL_AUDIO_V2_DONGLE_UL_DATA_IN_MIXER;
        }
        else
        {
            /* update mixer status */
            ull_audio_v2_ul_packet_anchor_in_mixer = dongle_handle->source_info.bt_in.fetch_anchor;
        }
    }

    /* process the new packet */
    switch (dongle_handle->data_status)
    {
        case ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER:
            dongle_handle->source_info.bt_in.channel_data_status = 0;
            for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
            {
                if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
                {
                    continue;
                }
                else
                {
                    dst_buf = (uint8_t *)stream->callback.EntryPara.in_ptr[i];
                    /* set decoder do bypass flag */
                    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
                    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
                        *((lc3plus_dec_frame_status_t *)dst_buf) = LC3PLUS_DEC_FRAME_STATUS_BYPASS_DECODER;
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
                    }
                    else
                    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
                        *((celt_dec_frame_status_t *)dst_buf) = CELT_DEC_FRAME_STATUS_BYPASS_DECODER;
#endif /* AIR_CELT_DEC_V2_ENABLE */
                    }
                }
            }
            break;

        case ULL_AUDIO_V2_DONGLE_UL_DATA_PLC:
            dongle_handle->source_info.bt_in.channel_data_status = 0;
            for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
            {
                if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
                {
                    continue;
                }
                else
                {
                    dst_buf = (uint8_t *)stream->callback.EntryPara.in_ptr[i];
                    /* set decoder do PLC flag */
                    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
                    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
                        *((lc3plus_dec_frame_status_t *)dst_buf) = LC3PLUS_DEC_FRAME_STATUS_PLC;
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
                    }
                    else
                    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
                        *((celt_dec_frame_status_t *)dst_buf) = CELT_DEC_FRAME_STATUS_PLC;
#endif /* AIR_CELT_DEC_V2_ENABLE */
                    }
                }
            }
            break;

        case ULL_AUDIO_V2_DONGLE_UL_DATA_NORMAL:
            dongle_handle->drop_frames += 1;
            dongle_handle->source_info.bt_in.channel_data_status = 0;
            for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
            {
                if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
                {
                    continue;
                }
                else
                {
                    dst_buf = (uint8_t *)stream->callback.EntryPara.in_ptr[i];
                    p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.share_info);
                    read_offset = (dongle_handle->source_info.bt_in.bt_info[i])->blk_index*p_share_info->sub_info.block_info.block_size;
                    p_ull_audio_header = (ULL_AUDIO_V2_HEADER *)(hal_memview_infrasys_to_dsp0(p_share_info->start_addr) + read_offset);
                    src_buf = (uint8_t *)(hal_memview_infrasys_to_dsp0(p_share_info->start_addr) + read_offset + sizeof(ULL_AUDIO_V2_HEADER));
                    if (p_ull_audio_header->valid_packet != 1)
                    {
                        /* set decoder do PLC flag */
                        if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
                        {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
                            *((lc3plus_dec_frame_status_t *)dst_buf) = LC3PLUS_DEC_FRAME_STATUS_PLC;
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
                        }
                        else
                        {
#if defined(AIR_CELT_DEC_V2_ENABLE)
                            *((celt_dec_frame_status_t *)dst_buf) = CELT_DEC_FRAME_STATUS_PLC;
#endif /* AIR_CELT_DEC_V2_ENABLE */
                        }
                        // ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR][handle 0x%x] valid flag is not right %d, 0x%x, 0x%x, 0x%x, 0x%x", 6,
                        //                 dongle_handle,
                        //                 (dongle_handle->source_info.bt_in.bt_info[i])->blk_index,
                        //                 dongle_handle->source_info.bt_in.fetch_anchor_previous,
                        //                 dongle_handle->source_info.bt_in.fetch_anchor,
                        //                 p_ull_audio_header->TimeStamp,
                        //                 bt_clk);
                    }
                    else
                    {
                        /* check frame size */
                        /* [nr_offload] */
//#ifdef AIR_ECNR_POST_PART_ENABLE
                        uint32_t actual_PduLen;

                        if (g_nr_offlad_status == true) {
                            actual_PduLen = p_ull_audio_header->PduLen - 1;
                        } else {
                            actual_PduLen = p_ull_audio_header->PduLen;
                        }
                        if (actual_PduLen != dongle_handle->source_info.bt_in.frame_size) {
                            ULL_V2_LOG_W("[ULL Audio V2][UL][ERROR][handle 0x%x] Bit rate is changed, frame_size from %d to %d", 3,
                                            dongle_handle,
                                            actual_PduLen,
                                            dongle_handle->source_info.bt_in.frame_size);
                            dongle_handle->source_info.bt_in.frame_size = actual_PduLen;
                        }
//#endif /* AIR_ECNR_POST_PART_ENABLE */

                        /* check frame timestamp */
                        if (ull_audio_v2_dongle_ul_fetch_timestamp_is_correct(dongle_handle, p_ull_audio_header->TimeStamp))
                        {
                            /* set decoder do decode flag */
                            if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
                            {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
                                *((lc3plus_dec_frame_status_t *)dst_buf) = LC3PLUS_DEC_FRAME_STATUS_NORMAL;
//#ifdef AIR_ECNR_POST_PART_ENABLE
                                if (g_nr_offlad_status == true) {
                                    g_post_ec_gain = src_buf[dongle_handle->source_info.bt_in.frame_size];
                                    ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][nr_offload] Get the PostEC gain %d", 2, dongle_handle, g_post_ec_gain);
                                }
//#endif /* AIR_ECNR_POST_PART_ENABLE */
                                /* copy frame data into the stream buffer */
                                memcpy(dst_buf+sizeof(lc3plus_dec_frame_status_t), src_buf, dongle_handle->source_info.bt_in.frame_size);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
                            }
                            else if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS) {
#if defined(AIR_CELT_DEC_V2_ENABLE)
                                *((celt_dec_frame_status_t *)dst_buf) = CELT_DEC_FRAME_STATUS_NORMAL;
//#ifdef AIR_ECNR_POST_PART_ENABLE
                                if (g_nr_offlad_status == true) {
                                    g_post_ec_gain = src_buf[dongle_handle->source_info.bt_in.frame_size];
                                    ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][nr_offload] Get the PostEC gain %d", 2, dongle_handle, g_post_ec_gain);
                                }
//#endif /* AIR_ECNR_POST_PART_ENABLE */
                                /* copy frame data into the stream buffer */
                                memcpy(dst_buf+sizeof(celt_dec_frame_status_t), src_buf, dongle_handle->source_info.bt_in.frame_size);
#endif /* AIR_CELT_DEC_V2_ENABLE */
                            } else {
                                UNUSED(g_post_ec_gain);
                            }
                            dongle_handle->source_info.bt_in.channel_data_status |= 0x1<<i;
                        }
                        else
                        {
                            /* set decoder do PLC flag */
                            if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
                            {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
                                *((lc3plus_dec_frame_status_t *)dst_buf) = LC3PLUS_DEC_FRAME_STATUS_PLC;
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
                            }
                            else
                            {
#if defined(AIR_CELT_DEC_V2_ENABLE)
                                *((celt_dec_frame_status_t *)dst_buf) = CELT_DEC_FRAME_STATUS_PLC;
#endif /* AIR_CELT_DEC_V2_ENABLE */
                            }
                            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR][handle 0x%x] timestamp is not right %d, 0x%x, 0x%x, 0x%x, 0x%x", 6,
                                            dongle_handle,
                                            (dongle_handle->source_info.bt_in.bt_info[i])->blk_index,
                                            dongle_handle->source_info.bt_in.fetch_anchor_previous,
                                            dongle_handle->source_info.bt_in.fetch_anchor,
                                            p_ull_audio_header->TimeStamp,
                                            bt_clk);
                        }
                    }
                }
            }
            /* update fetch timestamp */
            dongle_handle->source_info.bt_in.fetch_anchor_previous = dongle_handle->source_info.bt_in.fetch_anchor;
            dongle_handle->source_info.bt_in.fetch_anchor = (dongle_handle->source_info.bt_in.fetch_anchor + dongle_handle->source_info.bt_in.bt_interval) & 0x0fffffff;
            break;

        case ULL_AUDIO_V2_DONGLE_UL_DATA_IN_MIXER:
            dongle_handle->source_info.bt_in.channel_data_status = 0;
            for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
            {
                if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
                {
                    continue;
                }
                else
                {
                    dst_buf = (uint8_t *)stream->callback.EntryPara.in_ptr[i];
                    /* set decoder do bypass flag */
                    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
                    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
                        *((lc3plus_dec_frame_status_t *)dst_buf) = LC3PLUS_DEC_FRAME_STATUS_BYPASS_DECODER;
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
                    }
                    else
                    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
                        *((celt_dec_frame_status_t *)dst_buf) = CELT_DEC_FRAME_STATUS_BYPASS_DECODER;
#endif /* AIR_CELT_DEC_V2_ENABLE */
                    }
                }
            }
            /* set drop frame to 0 because other stream has process this packet */
            dongle_handle->drop_frames = 0;
            /* update fetch timestamp */
            dongle_handle->source_info.bt_in.fetch_anchor_previous = dongle_handle->source_info.bt_in.fetch_anchor;
            dongle_handle->source_info.bt_in.fetch_anchor = (dongle_handle->source_info.bt_in.fetch_anchor + dongle_handle->source_info.bt_in.bt_interval) & 0x0fffffff;
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    /* update stream status */
    if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
    {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
        stream->callback.EntryPara.in_size = dongle_handle->source_info.bt_in.frame_size+sizeof(lc3plus_dec_frame_status_t);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
    }
    else
    {
#if defined(AIR_CELT_DEC_V2_ENABLE)
        stream->callback.EntryPara.in_size = dongle_handle->source_info.bt_in.frame_size+sizeof(celt_dec_frame_status_t);
#endif /* AIR_CELT_DEC_V2_ENABLE */
    }
    if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
    {
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.feature_res   = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.sink_out_res  = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.process_res   = RESOLUTION_16BIT;
    }
    else
    {
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.feature_res   = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.sink_out_res  = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.process_res   = RESOLUTION_32BIT;
    }
#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
    if (source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0) {
        if (stream->sink->param.audio.format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
            stream->callback.EntryPara.resolution.sink_out_res  = RESOLUTION_16BIT;
        } else {
            stream->callback.EntryPara.resolution.sink_out_res  = RESOLUTION_32BIT;
        }
    }
#endif /* AIR_DONGLE_I2S_MST_OUT_ENABLE */

    /* do clock skew */
    if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0)
    {
        /* configure clock skew settings */
        if (dongle_handle->data_status != ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER)
        {
            dongle_handle->compen_samples = ull_audio_v2_dongle_ul_usb_clock_skew_check(dongle_handle);
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, dongle_handle->compen_samples);
        }
        else
        {
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, 0);
        }

        /* configure buffer output size */
        stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 0, dongle_handle->buffer_output_size);
    }
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
    else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0)
    {
        /* configure clock skew settings */
        if (dongle_handle->data_status != ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER)
        {
            dongle_handle->compen_samples = ull_audio_v2_dongle_ul_i2s_slv_out_clock_skew_check(dongle_handle);
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, dongle_handle->compen_samples);
        }
        else
        {
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, 0);
        }
    }
#endif /* AIR_DONGLE_I2S_SLV_OUT_ENABLE */
#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
    else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT)
    {
        /* configure clock skew settings */
        if (dongle_handle->data_status != ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER)
        {
            dongle_handle->compen_samples = ull_audio_v2_dongle_ul_line_out_clock_skew_check(dongle_handle);
        }
    }
#endif /* AIR_DONGLE_LINE_OUT_ENABLE */
#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
    else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0)
    {
        /* configure clock skew settings */
        if (dongle_handle->data_status != ULL_AUDIO_V2_DONGLE_UL_DATA_BYPASS_DECODER)
        {
            dongle_handle->compen_samples = ull_audio_v2_dongle_ul_i2s_mst_out_clock_skew_check(dongle_handle);
        }
    }
#endif /* AIR_DONGLE_I2S_MST_OUT_ENABLE */

#if ULL_AUDIO_V2_DONGLE_UL_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_V2_LOG_I("[ULL Audio V2][UL][DEBUG][source_copy_payload]: 0x%0x, 0x%0x, 0x%0x, 0x%0x, 0x%0x, 0x%0x, %d", 7, dongle_handle->data_status, dongle_handle->drop_frames, dongle_handle->source_info.bt_in.fetch_anchor_previous, dongle_handle->source_info.bt_in.fetch_anchor, dongle_handle->source_info.bt_in.channel_data_status, current_timestamp, hal_nvic_query_exception_number());
#endif /* ULL_AUDIO_V2_DONGLE_UL_PATH_DEBUG_LOG */

    return length;
}

bool ull_audio_v2_dongle_ul_source_get_new_read_offset(SOURCE source, U32 amount, U32 *new_read_offset)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    uint32_t i, saved_mask;
    n9_dsp_share_info_ptr p_share_info;
    uint32_t blk_size;
    uint32_t blk_num;
    uint32_t blk_index;
    UNUSED(amount);
    UNUSED(new_read_offset);

    if (dongle_handle->drop_frames != 0)
    {
        for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
        {
            if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
            {
                continue;
            }
            else
            {
                /* get blk info */
                p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.share_info);
                blk_size     = p_share_info->sub_info.block_info.block_size;
                blk_num      = p_share_info->sub_info.block_info.block_num;
                /* get new blk index */
                // (dongle_handle->source_info.bt_in.bt_info[i])->blk_index_previous = (dongle_handle->source_info.bt_in.bt_info[i])->blk_index;
                blk_index = ((dongle_handle->source_info.bt_in.bt_info[i])->blk_index + dongle_handle->drop_frames) % blk_num;
                // (dongle_handle->source_info.bt_in.bt_info[i])->blk_index = blk_index;
                /* update read index */
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                StreamDSP_HWSemaphoreTake();
                p_share_info->read_offset = blk_index * blk_size;
                StreamDSP_HWSemaphoreGive();
                hal_nvic_restore_interrupt_mask(saved_mask);
            }
        }
    }

    /* we will update the read offsets of the different share buffers in here directly, so return false to aviod the upper layer update read offset */
    return false;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ull_audio_v2_dongle_ul_fetch_count_decrease(ull_audio_v2_dongle_ul_handle_t *dongle_handle)
{
    uint32_t saved_mask;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->fetch_count != 0)
    {
        dongle_handle->fetch_count -= 1;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
}

void ull_audio_v2_dongle_ul_source_drop_postprocess(SOURCE source, uint32_t amount)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(source, source->transform->sink);
    uint32_t i;
    uint32_t read_offset;
    n9_dsp_share_info_ptr p_share_info;
    uint32_t blk_size;
    uint32_t blk_num;
    uint32_t blk_index = 0;
    uint32_t blk_index_previous = 0;
    ULL_AUDIO_V2_HEADER *p_ull_audio_header;
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t remain_samples_in_sw_buffer;
    uint32_t sample_size = 0;
    int32_t frac_rpt;
    uint32_t frame_num;
    UNUSED(amount);
    UNUSED(stream);

    /* update time stamp */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&dongle_handle->data_out_gpt_count);

    /* get sample size */
    sample_size = (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) ? sizeof(int16_t) : sizeof(int32_t);

    /* audio dump */
    #ifdef AIR_AUDIO_DUMP_ENABLE
    #if ULL_AUDIO_V2_DONGLE_UL_PATH_DEBUG_DUMP
    uint8_t *read_pointer;
    uint32_t data_size;
    if (dongle_handle->data_status == ULL_AUDIO_V2_DONGLE_UL_DATA_NORMAL)
    {
        if (dongle_handle->source_info.bt_in.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
        {
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
            LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.in_ptr[0])+sizeof(lc3plus_dec_frame_status_t), dongle_handle->source_info.bt_in.frame_size, VOICE_TX_MIC_0);
            LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.in_ptr[1])+sizeof(lc3plus_dec_frame_status_t), dongle_handle->source_info.bt_in.frame_size, VOICE_TX_MIC_1);
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
        }
        else
        {
#if defined(AIR_CELT_DEC_V2_ENABLE)
            LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.in_ptr[0])+sizeof(celt_dec_frame_status_t), dongle_handle->source_info.bt_in.frame_size, VOICE_TX_MIC_0);
            LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.in_ptr[1])+sizeof(celt_dec_frame_status_t), dongle_handle->source_info.bt_in.frame_size, VOICE_TX_MIC_1);
#endif /* AIR_CELT_DEC_V2_ENABLE */
        }
        /* dump channel 1's decoder output data */
        stream_function_sw_mixer_channel_input_get_data_info(dongle_handle->mixer_member, 1, &read_pointer, NULL, &data_size);
        LOG_AUDIO_DUMP(read_pointer, data_size, VOICE_RX_NR_IN);
        /* dump channel 2's decoder output data */
        stream_function_sw_mixer_channel_input_get_data_info(dongle_handle->mixer_member, 2, &read_pointer, NULL, &data_size);
        LOG_AUDIO_DUMP(read_pointer, data_size, VOICE_RX_NR_OUT);
    }
    if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0)
    {
        if (dongle_handle->buffer_output_size != 0)
        {
            if (dongle_handle->sink_info.usb_out.channel_num == 2)
            {
                LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[0]), dongle_handle->process_frames*dongle_handle->sink_info.usb_out.frame_samples*sample_size, VOICE_TX_MIC_2);
                LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[1]), dongle_handle->process_frames*dongle_handle->sink_info.usb_out.frame_samples*sample_size, VOICE_TX_MIC_3);
            }
            else
            {
                LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[0]), dongle_handle->process_frames*dongle_handle->sink_info.usb_out.frame_samples*sample_size, VOICE_TX_MIC_2);
            }
        }
    }
#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
    else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT)
    {
        LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[0]), dongle_handle->sink_info.line_out.frame_samples*dongle_handle->sink_info.line_out.frame_interval/1000*sample_size, VOICE_TX_REF);
        LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[1]), dongle_handle->sink_info.line_out.frame_samples*dongle_handle->sink_info.line_out.frame_interval/1000*sample_size, VOICE_TX_NR_OUT);
    }
#endif /* AIR_DONGLE_LINE_OUT_ENABLE */
#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
    else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0)
    {
        LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[0]), dongle_handle->sink_info.i2s_mst_out.frame_samples*dongle_handle->sink_info.i2s_mst_out.frame_interval/1000*sample_size, PROMPT_VP_PATTERN);
        LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[1]), dongle_handle->sink_info.i2s_mst_out.frame_samples*dongle_handle->sink_info.i2s_mst_out.frame_interval/1000*sample_size, PROMPT_VP_OUT);
    }
#endif /* AIR_DONGLE_I2S_MST_OUT_ENABLE */
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
    else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0)
    {
        stream_function_sw_clk_skew_get_output_size(dongle_handle->clk_skew_port, &data_size);
        LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[0]), data_size, VOICE_TX_CPD_IN);
        LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[1]), data_size, VOICE_TX_CPD_OUT);
    }
#endif /* AIR_DONGLE_I2S_SLV_OUT_ENABLE */
    #endif /* ULL_AUDIO_V2_DONGLE_UL_PATH_DEBUG_DUMP */
    #endif /* AIR_AUDIO_DUMP_ENABLE */

    /* add debug log */
    MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);
    for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
    {
        if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
        {
            continue;
        }
        else
        {
            blk_index_previous = (dongle_handle->source_info.bt_in.bt_info[i])->blk_index_previous;
            blk_index = (dongle_handle->source_info.bt_in.bt_info[i])->blk_index;
            break;
        }
    }
    if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0)
    {
        remain_samples_in_sw_buffer = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sample_size;
        stream_function_sw_clk_skew_get_frac_rpt(dongle_handle->clk_skew_port, 1, &frac_rpt);
        frame_num = ull_audio_v2_dongle_ul_sink_get_unprocess_frame_num(dongle_handle);
        ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][source_drop_postprocess] usb0-out %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x", 20,
                        dongle_handle,
                        dongle_handle->stream_status,
                        dongle_handle->first_packet_status,
                        dongle_handle->data_status,
                        dongle_handle->drop_frames,
                        blk_index,
                        dongle_handle->source_info.bt_in.fetch_anchor_previous,
                        bt_clk,
                        dongle_handle->source_info.bt_in.channel_data_status,
                        dongle_handle->buffer_output_size/sample_size,
                        dongle_handle->sink_info.usb_out.mcu_frame_count_latch,
                        remain_samples_in_sw_buffer,
                        frame_num,
                        dongle_handle->clk_skew_count,
                        frac_rpt,
                        dongle_handle->compen_samples,
                        dongle_handle->ccni_in_bt_count,
                        dongle_handle->data_out_bt_count,
                        dongle_handle->ccni_in_gpt_count,
                        dongle_handle->data_out_gpt_count);
    }
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
    else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0)
    {
        dongle_handle->sink_info.i2s_slv_out.pre_write_offset = dongle_handle->sink_info.i2s_slv_out.cur_write_offset;
        dongle_handle->sink_info.i2s_slv_out.pre_read_offset  = dongle_handle->sink_info.i2s_slv_out.cur_read_offset;
        dongle_handle->sink_info.i2s_slv_out.cur_write_offset = source->transform->sink->streamBuffer.BufferInfo.WriteOffset;
        dongle_handle->sink_info.i2s_slv_out.cur_read_offset  = source->transform->sink->streamBuffer.BufferInfo.ReadOffset;
        dongle_handle->sink_info.i2s_slv_out.afe_cur          = AFE_GET_REG(dongle_handle->sink_info.i2s_slv_out.afe_cur_addr);
        dongle_handle->sink_info.i2s_slv_out.afe_base         = AFE_GET_REG(dongle_handle->sink_info.i2s_slv_out.afe_base_addr);
        uint32_t src_1_tracking_rate = (AFE_READ(ASM_FREQUENCY_2) != 0) ? 0x94C5F000 / AFE_READ(ASM_FREQUENCY_2) : 0;
        uint32_t src_2_tracking_rate = (AFE_READ(ASM2_FREQUENCY_2) != 0) ? 0x94C5F000 / AFE_READ(ASM2_FREQUENCY_2) : 0;
        ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][source_drop_postprocess] i2ss-out %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, rate1 %d rate2 %d", 19,
                        dongle_handle,
                        dongle_handle->stream_status,
                        dongle_handle->first_packet_status,
                        dongle_handle->data_status,
                        dongle_handle->drop_frames,
                        blk_index,
                        dongle_handle->source_info.bt_in.fetch_anchor_previous,
                        bt_clk,
                        dongle_handle->source_info.bt_in.channel_data_status,
                        dongle_handle->clk_skew_count,
                        dongle_handle->compen_samples,
                        source->transform->sink->streamBuffer.BufferInfo.WriteOffset,
                        source->transform->sink->streamBuffer.BufferInfo.ReadOffset,
                        dongle_handle->sink_info.i2s_slv_out.afe_cur,
                        dongle_handle->ccni_in_bt_count,
                        dongle_handle->ccni_in_gpt_count,
                        dongle_handle->data_out_gpt_count,
                        src_1_tracking_rate,
                        src_2_tracking_rate);
    }
#endif /* AIR_DONGLE_I2S_SLV_OUT_ENABLE */
#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
    else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT)
    {
        dongle_handle->sink_info.line_out.pre_write_offset = dongle_handle->sink_info.line_out.cur_write_offset;
        dongle_handle->sink_info.line_out.pre_read_offset  = dongle_handle->sink_info.line_out.cur_read_offset;
        dongle_handle->sink_info.line_out.cur_write_offset = source->transform->sink->streamBuffer.BufferInfo.WriteOffset;
        dongle_handle->sink_info.line_out.cur_read_offset  = source->transform->sink->streamBuffer.BufferInfo.ReadOffset;
        dongle_handle->sink_info.line_out.afe_cur          = AFE_GET_REG(dongle_handle->sink_info.line_out.afe_cur_addr);
        dongle_handle->sink_info.line_out.afe_base         = AFE_GET_REG(dongle_handle->sink_info.line_out.afe_base_addr);
        ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][source_drop_postprocess] line-out %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", 15,
                        dongle_handle,
                        dongle_handle->stream_status,
                        dongle_handle->first_packet_status,
                        dongle_handle->data_status,
                        dongle_handle->drop_frames,
                        blk_index,
                        dongle_handle->source_info.bt_in.fetch_anchor_previous,
                        bt_clk,
                        dongle_handle->source_info.bt_in.channel_data_status,
                        dongle_handle->sink_info.line_out.cur_write_offset,
                        dongle_handle->sink_info.line_out.cur_read_offset,
                        dongle_handle->sink_info.line_out.afe_cur,
                        dongle_handle->ccni_in_bt_count,
                        dongle_handle->ccni_in_gpt_count,
                        dongle_handle->data_out_gpt_count);
    }
#endif /* AIR_DONGLE_LINE_OUT_ENABLE */
#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
    else if (dongle_handle->sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0)
    {
        dongle_handle->sink_info.i2s_mst_out.pre_write_offset = dongle_handle->sink_info.i2s_mst_out.cur_write_offset;
        dongle_handle->sink_info.i2s_mst_out.pre_read_offset  = dongle_handle->sink_info.i2s_mst_out.cur_read_offset;
        dongle_handle->sink_info.i2s_mst_out.cur_write_offset = source->transform->sink->streamBuffer.BufferInfo.WriteOffset;
        dongle_handle->sink_info.i2s_mst_out.cur_read_offset  = source->transform->sink->streamBuffer.BufferInfo.ReadOffset;
        dongle_handle->sink_info.i2s_mst_out.afe_cur          = AFE_GET_REG(dongle_handle->sink_info.i2s_mst_out.afe_cur_addr);
        dongle_handle->sink_info.i2s_mst_out.afe_base         = AFE_GET_REG(dongle_handle->sink_info.i2s_mst_out.afe_base_addr);
        ULL_V2_LOG_I("[ULL Audio V2][UL][handle 0x%x][source_drop_postprocess] i2s_mst-out %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", 15,
                        dongle_handle,
                        dongle_handle->stream_status,
                        dongle_handle->first_packet_status,
                        dongle_handle->data_status,
                        dongle_handle->drop_frames,
                        blk_index,
                        dongle_handle->source_info.bt_in.fetch_anchor_previous,
                        bt_clk,
                        dongle_handle->source_info.bt_in.channel_data_status,
                        dongle_handle->sink_info.i2s_mst_out.cur_write_offset,
                        dongle_handle->sink_info.i2s_mst_out.cur_read_offset,
                        dongle_handle->sink_info.i2s_mst_out.afe_cur,
                        dongle_handle->ccni_in_bt_count,
                        dongle_handle->ccni_in_gpt_count,
                        dongle_handle->data_out_gpt_count);
    }
#endif /* AIR_DONGLE_I2S_MST_OUT_ENABLE */

    /* drop packets */
    if (dongle_handle->drop_frames != 0)
    {
        for (i = 0; i < ULL_AUDIO_V2_DATA_CHANNEL_NUMBER; i++)
        {
            if (dongle_handle->source_info.bt_in.bt_info[i] == NULL)
            {
                continue;
            }
            else
            {
                /* get blk info */
                p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->source_info.bt_in.bt_info[i])->bt_link_info.share_info);
                blk_size     = p_share_info->sub_info.block_info.block_size;
                blk_num      = p_share_info->sub_info.block_info.block_num;
                /* update blk index */
                (dongle_handle->source_info.bt_in.bt_info[i])->blk_index_previous = (dongle_handle->source_info.bt_in.bt_info[i])->blk_index;
                blk_index_previous = (dongle_handle->source_info.bt_in.bt_info[i])->blk_index;
                blk_index = ((dongle_handle->source_info.bt_in.bt_info[i])->blk_index + dongle_handle->drop_frames) % blk_num;
                (dongle_handle->source_info.bt_in.bt_info[i])->blk_index = blk_index;
                /* clear valid flag */
                while (blk_index != blk_index_previous)
                {
                    /* get packet header */
                    read_offset = blk_index_previous*blk_size;
                    p_ull_audio_header = (ULL_AUDIO_V2_HEADER *)(hal_memview_infrasys_to_dsp0(p_share_info->start_addr) + read_offset);
                    /* clear valid flag */
                    p_ull_audio_header->valid_packet = 0;
                    /* switch the next block */
                    blk_index_previous = (blk_index_previous+1)% blk_num;
                }
            }
        }
        dongle_handle->drop_frames = 0;
    }

    /* decrease fetch count */
    ull_audio_v2_dongle_ul_fetch_count_decrease(dongle_handle);
}

bool ull_audio_v2_dongle_ul_source_close(SOURCE source)
{
    UNUSED(source);

    return true;
}

ATTR_TEXT_IN_IRAM bool ull_audio_v2_dongle_ul_sink_get_avail_size(SINK sink, uint32_t *avail_size)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)(sink->transform->source->param.bt_common.scenario_param.dongle_handle);
    uint32_t frame_num;

    if (sink->streamBuffer.ShareBufferInfo.read_offset < sink->streamBuffer.ShareBufferInfo.write_offset)
    {
        /* normal case */
        *avail_size = sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num
                    - sink->streamBuffer.ShareBufferInfo.write_offset
                    + sink->streamBuffer.ShareBufferInfo.read_offset;
    }
    else if (sink->streamBuffer.ShareBufferInfo.read_offset == sink->streamBuffer.ShareBufferInfo.write_offset)
    {
        if(sink->streamBuffer.ShareBufferInfo.bBufferIsFull == true)
        {
            /* buffer is full, so read_offset == write_offset */
            *avail_size = 0;
        }
        else
        {
            /* buffer is empty, so read_offset == write_offset */
            *avail_size = sink->streamBuffer.ShareBufferInfo.length;
        }
    }
    else
    {
        /* buffer wrapper case */
        *avail_size = sink->streamBuffer.ShareBufferInfo.read_offset - sink->streamBuffer.ShareBufferInfo.write_offset;
    }

    frame_num = *avail_size / sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
    if ((frame_num < dongle_handle->sink_info.usb_out.frame_max_num) && (dongle_handle->fetch_count != 0))
    {
        /* in each time, there must be at least 10ms data free space in the share buffer */
        ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]frame num is not right, %u, %u, %u\r\n", 3, frame_num, dongle_handle->sink_info.usb_out.frame_max_num, dongle_handle->fetch_count);
        AUDIO_ASSERT(0);
    }

    return true;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ uint32_t ull_audio_v2_dongle_ul_sink_copy_payload(SINK sink, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)(sink->transform->source->param.bt_common.scenario_param.dongle_handle);
    DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(sink->transform->source, sink);
    uint32_t process_frames;
    uint32_t saved_mask;
    uint32_t i;
    uint32_t payload_size;
    uint32_t write_offset;
    uint32_t start_addr;
    uint32_t block_size;
    uint32_t block_length;
    UNUSED(src_buf);

    /* copy pcm data into the share buffer one by one 1ms */
    if (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)
    {
        if (length%(sizeof(int32_t)*dongle_handle->sink_info.usb_out.frame_samples) != 0)
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]length is not right, %u\r\n", 1, length);
            AUDIO_ASSERT(0);
        }
        process_frames = ((length/sizeof(int32_t))/dongle_handle->sink_info.usb_out.frame_samples);
    }
    else
    {
        if (length%(sizeof(int16_t)*dongle_handle->sink_info.usb_out.frame_samples) != 0)
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]length is not right, %u\r\n", 1, length);
            AUDIO_ASSERT(0);
        }
        process_frames = ((length/sizeof(int16_t))/dongle_handle->sink_info.usb_out.frame_samples);
    }
    start_addr = sink->streamBuffer.ShareBufferInfo.start_addr;
    block_size = sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
    block_length = sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED)
    {
        write_offset = sink->streamBuffer.ShareBufferInfo.write_offset;
    }
    else
    {
        write_offset = (sink->streamBuffer.ShareBufferInfo.write_offset + dongle_handle->process_frames * block_size) % block_length;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
    for (i = 0; i < process_frames; i++)
    {
        /* get dst buffer */
        dst_buf = (uint8_t *)(start_addr + ((write_offset + block_size*i) % block_length));

        if ((dongle_handle->sink_info.usb_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) && (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE))
        {
            /* copy pcm samples into the share buffer */
            if (dongle_handle->sink_info.usb_out.channel_num == 2)
            {
                // DSP_D2I_BufferCopy_16bit((U16 *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
                //                         (U16 *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->sink_info.usb_out.frame_samples,
                //                         (U16 *)(stream->callback.EntryPara.out_ptr[1])+i*dongle_handle->sink_info.usb_out.frame_samples,
                //                         dongle_handle->sink_info.usb_out.frame_samples);
                ShareBufferCopy_D_16bit_to_I_16bit_2ch( (uint16_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint16_t *)(stream->callback.EntryPara.out_ptr[1])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint16_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
                                                        dongle_handle->sink_info.usb_out.frame_samples);
                payload_size = dongle_handle->sink_info.usb_out.frame_samples*2*sizeof(int16_t);
            }
            else
            {
                // memcpy(dst_buf+sizeof(audio_transmitter_frame_header_t), (U16 *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->sink_info.usb_out.frame_samples, dongle_handle->sink_info.usb_out.frame_samples*sizeof(int16_t));
                ShareBufferCopy_D_16bit_to_D_16bit_1ch( (uint16_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint16_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
                                                        dongle_handle->sink_info.usb_out.frame_samples);
                payload_size = dongle_handle->sink_info.usb_out.frame_samples*sizeof(int16_t);
            }
        }
        else if ((dongle_handle->sink_info.usb_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) && (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE))
        {
            /* copy pcm samples into the share buffer */
            if (dongle_handle->sink_info.usb_out.channel_num == 2)
            {
                ShareBufferCopy_D_32bit_to_I_16bit_2ch( (uint32_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint32_t *)(stream->callback.EntryPara.out_ptr[1])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint16_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
                                                        dongle_handle->sink_info.usb_out.frame_samples);
                payload_size = dongle_handle->sink_info.usb_out.frame_samples*2*sizeof(int16_t);
            }
            else
            {
                ShareBufferCopy_D_32bit_to_D_16bit_1ch( (uint32_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint16_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
                                                        dongle_handle->sink_info.usb_out.frame_samples);
                payload_size = dongle_handle->sink_info.usb_out.frame_samples*sizeof(int16_t);
            }
        }
        else if ((dongle_handle->sink_info.usb_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) && (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE))
        {
            /* copy pcm samples into the share buffer */
            if (dongle_handle->sink_info.usb_out.channel_num == 2)
            {
                ShareBufferCopy_D_16bit_to_I_24bit_2ch( (uint16_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint16_t *)(stream->callback.EntryPara.out_ptr[1])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint8_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
                                                        dongle_handle->sink_info.usb_out.frame_samples);
                payload_size = dongle_handle->sink_info.usb_out.frame_samples*2*3;
            }
            else
            {
                ShareBufferCopy_D_16bit_to_D_24bit_1ch( (uint16_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint8_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
                                                        dongle_handle->sink_info.usb_out.frame_samples);
                payload_size = dongle_handle->sink_info.usb_out.frame_samples*3;
            }
        }
        else if ((dongle_handle->sink_info.usb_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) && (dongle_handle->source_info.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE))
        {
            /* copy pcm samples into the share buffer */
            if (dongle_handle->sink_info.usb_out.channel_num == 2)
            {
                ShareBufferCopy_D_32bit_to_I_24bit_2ch( (uint32_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint32_t *)(stream->callback.EntryPara.out_ptr[1])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint8_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
                                                        dongle_handle->sink_info.usb_out.frame_samples);
                payload_size = dongle_handle->sink_info.usb_out.frame_samples*2*3;
            }
            else
            {
                ShareBufferCopy_D_32bit_to_D_24bit_1ch( (uint32_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->sink_info.usb_out.frame_samples,
                                                        (uint8_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
                                                        dongle_handle->sink_info.usb_out.frame_samples);
                payload_size = dongle_handle->sink_info.usb_out.frame_samples*3;
            }
        }
        else
        {
            ULL_V2_LOG_E("[ULL Audio V2][UL][ERROR]sample_format is not supported, %u, %u\r\n", 2, dongle_handle->sink_info.usb_out.sample_format, dongle_handle->source_info.bt_in.sample_format);
            AUDIO_ASSERT(0);
        }

        /* write seq number and payload_len into the share buffer */
        ((audio_transmitter_frame_header_t *)dst_buf)->seq_num      = dongle_handle->sink_info.usb_out.seq_num;
        ((audio_transmitter_frame_header_t *)dst_buf)->payload_len  = payload_size;

        /* update seq number */
        dongle_handle->sink_info.usb_out.seq_num = (dongle_handle->sink_info.usb_out.seq_num+1)&0xffff;
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED)
    {
        dongle_handle->process_frames = process_frames;
    }
    else
    {
        /* If stream is not played, only copy data into share buffer but not update write offset */
        dongle_handle->process_frames += process_frames;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    return payload_size;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_ul_sink_query_write_offset(SINK sink, uint32_t *write_offset)
{
    ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)(sink->transform->source->param.bt_common.scenario_param.dongle_handle);
    uint32_t saved_mask;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->first_packet_status == ULL_AUDIO_V2_DONGLE_UL_FIRST_PACKET_PLAYED)
    {
        /* If stream is played, update write offset */
        *write_offset = (sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * dongle_handle->process_frames) % (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num);
        audio_transmitter_share_information_update_write_offset(sink, *write_offset);
    }
    else
    {
        /* If stream is not played, only copy data into share buffer but not update write offset */
        *write_offset = sink->streamBuffer.ShareBufferInfo.write_offset;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    /* update time stamp */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&dongle_handle->data_out_bt_count);

    #if ULL_AUDIO_V2_DONGLE_UL_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_V2_LOG_I("[ULL Audio V2][UL][DEBUG][sink_query_write_offset]: %u, %u, %d, %d", 4, dongle_handle->first_packet_status, *write_offset, current_timestamp, hal_nvic_query_exception_number());
    #endif /* ULL_AUDIO_V2_DONGLE_UL_PATH_DEBUG_LOG */
}

bool ull_audio_v2_dongle_ul_sink_close(SINK sink)
{
    UNUSED(sink);

    return true;
}

#if (defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
/****************************************************************************************************************************************************/
/*                                                       ULL Dongle AFE IN(I2S IN/LINE IN)                                                          */
/****************************************************************************************************************************************************/
void ull_audio_v2_dongle_dl_init_afe_in(ull_audio_v2_dongle_dl_handle_t *dongle_handle, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    /* bt out info init */
    ull_audio_v2_dongle_dl_bt_out_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
    uint32_t sample_size = 0;
    stream_resolution_t stream_resolution = RESOLUTION_16BIT;
    if (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
        sample_size = sizeof(int32_t);
        stream_resolution = RESOLUTION_32BIT;
    } else if (dongle_handle->sink_info.bt_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
        sample_size = sizeof(int16_t);
        stream_resolution = RESOLUTION_16BIT;
    } else {
        ULL_V2_LOG_E("[ULL Audio V2][DL][ERROR]sample_format is not supported, %u\r\n", 1, dongle_handle->sink_info.bt_out.sample_format);
        AUDIO_ASSERT(0);
    }
    /* sw gain config */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution               = stream_resolution;
    default_config.target_gain              = 0; //audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    default_config.up_step                  = 1;
    default_config.up_samples_per_step      = 2;
    default_config.down_step                = -1;
    default_config.down_samples_per_step    = 2;
    dongle_handle->gain_port                = stream_function_sw_gain_get_port(dongle_handle->source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    default_gain = audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    ULL_V2_LOG_I("[ULL Audio V2][DL][scenario type %d-%d][handle 0x%x]sw gain 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                dongle_handle->source->scenario_type,
                dongle_handle->sink->scenario_type,
                dongle_handle,
                dongle_handle->gain_port,
                default_config.resolution,
                default_config.target_gain,
                default_config.up_step,
                default_config.up_samples_per_step,
                default_config.down_step,
                default_config.down_samples_per_step,
                audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L,
                audio_transmitter_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_R);

    /* sw src init */
    src_fixed_ratio_config_t sw_src_config = {0};
    dongle_handle->src_in_frame_samples     = dongle_handle->source->param.audio.frame_size; // * source->param.audio.channel_num;
    dongle_handle->src_in_frame_size        = dongle_handle->src_in_frame_samples * sample_size;
    dongle_handle->src_out_frame_samples    = dongle_handle->sink_info.bt_out.sample_rate/1000*dongle_handle->sink_info.bt_out.frame_interval/1000;
    dongle_handle->src_out_frame_size       = dongle_handle->src_out_frame_samples * sample_size;
    sw_src_config.channel_number            = 2;
    sw_src_config.in_sampling_rate          = dongle_handle->source->param.audio.src_rate; // for uplink, use src_out rate.
    sw_src_config.out_sampling_rate         = dongle_handle->sink_info.bt_out.sample_rate;
    sw_src_config.resolution                = stream_resolution;
    sw_src_config.multi_cvt_mode            = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_SINGLE;
    sw_src_config.cvt_num                   = 1;
    sw_src_config.with_codec                = false;
    dongle_handle->src_port                 = stream_function_src_fixed_ratio_get_port(dongle_handle->source);
    stream_function_src_fixed_ratio_init(dongle_handle->src_port, &sw_src_config);
    ULL_V2_LOG_I("[ULL Audio V2][DL][scenario type %d-%d][handle 0x%x]sw src 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 15,
                dongle_handle->source->scenario_type,
                dongle_handle->sink->scenario_type,
                dongle_handle,
                dongle_handle->src_port,
                sw_src_config.multi_cvt_mode,
                sw_src_config.cvt_num,
                sw_src_config.with_codec,
                sw_src_config.channel_number,
                sw_src_config.resolution,
                sw_src_config.in_sampling_rate,
                sw_src_config.out_sampling_rate,
                dongle_handle->src_in_frame_samples,
                dongle_handle->src_in_frame_size,
                dongle_handle->src_out_frame_samples,
                dongle_handle->src_out_frame_size);
    /* sw mixer init */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback     = NULL;
    callback_config.postprocess_callback    = ull_audio_v2_dongle_dl_mixer_postcallback;
    in_ch_config.total_channel_number       = 2;
    in_ch_config.resolution                 = stream_resolution;
    in_ch_config.input_mode                 = SW_MIXER_CHANNEL_MODE_FORCED_ENOUGH;
    in_ch_config.buffer_size                = dongle_handle->sink_info.bt_out.sample_rate/1000*dongle_handle->sink_info.bt_out.frame_interval/1000*sample_size;
    out_ch_config.total_channel_number      = 2;
    out_ch_config.resolution                = stream_resolution;
    dongle_handle->mixer_member             = stream_function_sw_mixer_member_create((void *)dongle_handle->source, SW_MIXER_MEMBER_MODE_NO_BYPASS, &callback_config, &in_ch_config, &out_ch_config);
    ull_audio_v2_dongle_dl_sw_mixer_connect(dongle_handle);
    ULL_V2_LOG_I("[ULL Audio V2][DL][scenario type %d-%d][handle 0x%x]sw mixer 0x%x info, %d, %d, %d, %d, %d, %d, %d\r\n", 11,
                dongle_handle->source->scenario_type,
                dongle_handle->sink->scenario_type,
                dongle_handle,
                dongle_handle->mixer_member,
                in_ch_config.total_channel_number,
                in_ch_config.resolution,
                in_ch_config.input_mode,
                in_ch_config.buffer_size,
                out_ch_config.total_channel_number,
                out_ch_config.resolution,
                dongle_handle->mixer_status);
#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
    g_i2s_slv_tracking_start_flag = true;
#endif
}

void ull_audio_v2_dongle_dl_deinit_afe_in(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    /* bt out info deinit */
    ull_audio_v2_dongle_dl_bt_out_deinit(dongle_handle);

    /* sw gain deinit */
    stream_function_sw_gain_deinit(dongle_handle->gain_port);

    /* sw mixer deinit */
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
    ull_audio_v2_dongle_dl_sw_mixer_disconnect(dongle_handle);
}

void ull_audio_v2_dongle_dl_stop_afe_in(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    uint32_t gpt_count;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    /* set sw gain -120dB to mute:dual channel */
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, -120000); // -120dB
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, -120000); // -120dB
    ULL_V2_LOG_I("[ULL Audio V2][DL][AFE IN] scenario type [%d]-[%d] stop stream, gpt count = 0x%x", 3,
                dongle_handle->source->scenario_type,
                dongle_handle->sink->scenario_type,
                gpt_count
                );
}

void ull_audio_v2_dongle_dl_start_afe_in(ull_audio_v2_dongle_dl_handle_t *dongle_handle)
{
    uint32_t gpt_count;
    hal_audio_memory_parameter_t *mem_handle = &dongle_handle->source->param.audio.mem_handle;
    SOURCE source = dongle_handle->source;
    hal_audio_agent_t agent;

    if ((mem_handle->memory_select != HAL_AUDIO_MEMORY_UL_SLAVE_DMA) &&
        (mem_handle->memory_select != HAL_AUDIO_MEMORY_UL_SLAVE_TDM) &&
        (mem_handle->memory_select != 0)) {
        // interconn mode
        agent = hal_memory_convert_agent(mem_handle->memory_select);
        /* disable AFE irq here */
        hal_memory_set_irq_enable(agent, HAL_AUDIO_CONTROL_OFF);
    }
    hal_audio_trigger_start_parameter_t start_parameter;

    /* set flag */
    dongle_handle->source->param.audio.drop_redundant_data_at_first_time = true; // not used ?

    /* enable afe agent here */
    if ((source->param.audio.mem_handle.pure_agent_with_src != true) && // i2s slv interconn tracking mode
        (source->param.audio.AfeBlkControl.u4asrcflag != true)) {       // i2s slv vdma tracking mode
        /* interconn non-tracking mode */
        start_parameter.memory_select = mem_handle->memory_select;
        start_parameter.enable = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
        /* set agent regsiters' address */
        switch (agent) {
            case HAL_AUDIO_AGENT_MEMORY_VUL1:
                dongle_handle->afe_vul_cur_addr  = AFE_VUL_CUR;
                dongle_handle->afe_vul_base_addr = AFE_VUL_BASE;
                break;
            case HAL_AUDIO_AGENT_MEMORY_VUL2:
                dongle_handle->afe_vul_cur_addr  = AFE_VUL2_CUR;
                dongle_handle->afe_vul_base_addr = AFE_VUL2_BASE;
                break;
            case HAL_AUDIO_AGENT_MEMORY_VUL3:
                dongle_handle->afe_vul_cur_addr  = AFE_VUL3_CUR;
                dongle_handle->afe_vul_base_addr = AFE_VUL3_BASE;
                break;
            default:
                ULL_V2_LOG_E("[ULL Audio V2][DL][AFE IN] ERROR: unknow agent = 0x%x", 1, agent);
                AUDIO_ASSERT(0);
        }
    } else {
        /* Tracking mode */
        if (source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0) {
            if (source->param.audio.AfeBlkControl.u4asrcflag) { // vdma mode use hwsrc1
                if (mem_handle->memory_select == HAL_AUDIO_MEMORY_UL_VUL2) {
                    ULL_V2_LOG_I("[ULL Audio V2][DL][AFE IN] ERROR: start with HWSRC2, this data path is not support, please check memory select", 0);
                    dongle_handle->afe_vul_cur_addr  = ASM2_CH01_OBUF_WRPNT;
                    dongle_handle->afe_vul_base_addr = ASM2_OBUF_SADR;
                    AUDIO_ASSERT(0);
                } else {
                    dongle_handle->afe_vul_cur_addr  = ASM_CH01_OBUF_WRPNT;
                    dongle_handle->afe_vul_base_addr = ASM_OBUF_SADR;
                }
            } else { // interconn mode use hwsrc2
                dongle_handle->afe_vul_cur_addr  = ASM2_CH01_OBUF_WRPNT;
                dongle_handle->afe_vul_base_addr = ASM2_OBUF_SADR;
            }
#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
            g_i2s_slv_tracking_start_flag = false;
#endif
        } else {
            ULL_V2_LOG_E("[ULL Audio V2][DL][AFE IN] ERROR: tracking mode, unknow agent = 0x%x", 1, agent);
            AUDIO_ASSERT(0);
        }
    }
    /* clear stream info */
    dongle_handle->source->streamBuffer.BufferInfo.ReadOffset  = 0;
    dongle_handle->source->streamBuffer.BufferInfo.WriteOffset = 0;
    dongle_handle->source->streamBuffer.BufferInfo.bBufferIsFull = false;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    ULL_V2_LOG_I("[ULL Audio V2][DL][AFE IN] scenario type %d-%d, start stream gpt count = 0x%x, cur_addr = 0x%x, base_addr = 0x%x, cur_ro = 0x%x, cur_base = 0x%x", 7,
                dongle_handle->source->scenario_type,
                dongle_handle->sink->scenario_type,
                gpt_count,
                dongle_handle->afe_vul_cur_addr,
                dongle_handle->afe_vul_base_addr,
                AFE_GET_REG(dongle_handle->afe_vul_cur_addr),
                AFE_GET_REG(dongle_handle->afe_vul_base_addr)
                );
}

static uint32_t ull_audio_v2_dongle_dl_calculate_afe_in_jitter_bt_audio(ull_audio_v2_dongle_dl_handle_t *dongle_handle, uint32_t gpt_count)
{
    uint32_t jitter_size, ratio_ms;

    ratio_ms = gpt_count / 100 + 1; /* Unit of 0.1ms */
    jitter_size = (dongle_handle->source->param.audio.frame_size * ratio_ms * 100) / dongle_handle->sink_info.bt_out.frame_interval;

    return jitter_size;
}

#ifdef AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
hal_gpt_callback_t ull_audio_v2_dongle_dl_i2s_slv_gpt_cb(void *user_data)
{
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = (ull_audio_v2_dongle_dl_handle_t *)user_data;
    hal_audio_memory_parameter_t *mem_handle = &dongle_handle->source->param.audio.mem_handle;
    hal_audio_agent_t agent = hal_memory_convert_agent(mem_handle->memory_select);
    hal_audio_trigger_start_parameter_t start_parameter;
    start_parameter.memory_select = mem_handle->memory_select;
    start_parameter.enable = true;
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
    /* enable AFE irq here */
    hal_memory_set_irq_enable(agent, HAL_AUDIO_CONTROL_ON);
    return 0;
}
#endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE */
#endif /* AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE || AIR_GAMING_MODE_DONGLE_V2_I2S_IN_ENABLE */

/******************************************************************************/
/*          BLE audio source dongle silence detection Public Functions        */
/******************************************************************************/
#ifdef AIR_SILENCE_DETECTION_ENABLE
void ull_audio_v2_dongle_silence_detection_init(audio_scenario_type_t scenario)
{
    volume_estimator_port_t *port = NULL;
    volume_estimator_config_t config;
    uint32_t data_size, sub_id;
    void *data_buf;
    void *nvkey_buf;
    ull_audio_v2_dongle_silence_detection_handle_t *silence_detection_handle = NULL;
    CONNECTION_IF *application_ptr;
    ull_audio_v2_dongle_dl_handle_t *dongle_handle;

    UNUSED(scenario);

    /* get NVKEY settings */
    nvkey_buf = preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE , sizeof(silence_detection_nvkey_t));
    if (nvkey_buf == NULL)
    {
        AUDIO_ASSERT(0);
    }
    if (nvkey_read_full_key(NVKEY_DSP_PARA_SILENCE_DETECTION2, nvkey_buf, sizeof(silence_detection_nvkey_t)) != NVDM_STATUS_NAT_OK)
    {
        AUDIO_ASSERT(0);
    }

    /* get handle */
    if ((scenario < AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) || (scenario > AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0))
    {
        AUDIO_ASSERT(0);
    }
    silence_detection_handle = &ull_audio_v2_dongle_silence_detection_handle[scenario-AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0];

    /* get volume estimator port */
    sub_id = scenario - AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0;
    application_ptr = port_audio_transmitter_get_connection_if(AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE, (audio_transmitter_scenario_sub_id_t)(AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0 + sub_id));
    dongle_handle = application_ptr->sink->param.bt_common.scenario_param.dongle_handle;
    port = volume_estimator_get_port(dongle_handle->owner);
    if (port == NULL)
    {
        AUDIO_ASSERT(0);
    }

    /* init volume estimator port */
    config.resolution = RESOLUTION_32BIT;
    config.frame_size = VOLUME_ESTIMATOR_CALCULATE_FRAME_SAMPLE_SIZE * sizeof(int32_t); /* 40samples*mono*32bit */
    config.channel_num = 1;
    config.mode = VOLUME_ESTIMATOR_CHAT_INSTANT_MODE;
    config.sample_rate = dongle_handle->sink_info.bt_out.sample_rate;
    config.nvkey_para = (void *)&(((silence_detection_nvkey_t *)nvkey_buf)->chat_vol_nvkey);
    volume_estimator_init(port, &config);

    /* malloc 5ms memory for queue mono pcm data */
    data_size = dongle_handle->sink_info.bt_out.frame_samples * sizeof(int32_t) * dongle_handle->source_info.usb_in.channel_num;
    data_buf = preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE, data_size);
    if (data_buf == NULL)
    {
        AUDIO_ASSERT(0);
    }
    memset(data_buf, 0, data_size);

    /* update state machine */
    if (silence_detection_handle->port != NULL)
    {
        AUDIO_ASSERT(0);
    }
    silence_detection_handle->sample_rate = config.sample_rate;
    silence_detection_handle->port = port;
    silence_detection_handle->enable = 0;
    silence_detection_handle->nvkey_buf = nvkey_buf;
    silence_detection_handle->nvkey_enable = (int16_t)(((silence_detection_nvkey_t *)nvkey_buf)->enable);
    silence_detection_handle->current_db = -144*100;
    silence_detection_handle->effective_threshold_db = ((silence_detection_nvkey_t *)nvkey_buf)->effective_threshold_db;
    silence_detection_handle->effective_delay_ms = ((silence_detection_nvkey_t *)nvkey_buf)->effective_delay_ms;
    memset(silence_detection_handle->effective_delay_us_count, 0, sizeof(silence_detection_handle->effective_delay_us_count));
    silence_detection_handle->failure_threshold_db = ((silence_detection_nvkey_t *)nvkey_buf)->failure_threshold_db;
    silence_detection_handle->failure_delay_ms = ((silence_detection_nvkey_t *)nvkey_buf)->failure_delay_ms;
    memset(silence_detection_handle->failure_delay_us_count, 0, sizeof(silence_detection_handle->failure_delay_us_count));
    silence_detection_handle->frame_size = config.frame_size;
    silence_detection_handle->data_size = 0;
    silence_detection_handle->data_buf_size = data_size;
    silence_detection_handle->data_buf = data_buf;
    silence_detection_handle->status = ULL_AUDIO_V2_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW;
    silence_detection_handle->total_channel = 0;
    silence_detection_handle->data_size_channel = 0;

    DSP_MW_LOG_I("[ULL Audio V2][DL][silence_detection][init]:%d, %d, %d, %d, %d, %d\r\n", 6,
                silence_detection_handle->nvkey_enable,
                silence_detection_handle->current_db,
                silence_detection_handle->effective_threshold_db,
                silence_detection_handle->effective_delay_ms,
                silence_detection_handle->failure_threshold_db,
                silence_detection_handle->failure_delay_ms);
}

void ull_audio_v2_dongle_silence_detection_deinit(audio_scenario_type_t scenario)
{
    volume_estimator_port_t *port;
    ull_audio_v2_dongle_silence_detection_handle_t *silence_detection_handle = NULL;
    UNUSED(scenario);

    /* get handle */
    if ((scenario < AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) || (scenario > AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0))
    {
        AUDIO_ASSERT(0);
    }
    silence_detection_handle = &ull_audio_v2_dongle_silence_detection_handle[scenario-AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0];

    /* check state machine */
    if ((silence_detection_handle->port == NULL) || (silence_detection_handle->data_buf == NULL) || (silence_detection_handle->nvkey_buf == NULL))
    {
        AUDIO_ASSERT(0);
    }
    port = silence_detection_handle->port;

    /* deinit chat volume estimator port */
    volume_estimator_deinit(port);

    /* free memory */
    preloader_pisplit_free_memory(silence_detection_handle->data_buf);
    preloader_pisplit_free_memory(silence_detection_handle->nvkey_buf);

    /* reset state machine */
    memset(silence_detection_handle, 0, sizeof(ull_audio_v2_dongle_silence_detection_handle_t));

    DSP_MW_LOG_I("[ULL Audio V2][DL][silence_detection][deinit]:%d, %d, %d, %d, %d, %d\r\n", 6,
                silence_detection_handle->nvkey_enable,
                silence_detection_handle->current_db,
                silence_detection_handle->effective_threshold_db,
                silence_detection_handle->effective_delay_ms,
                silence_detection_handle->failure_threshold_db,
                silence_detection_handle->failure_delay_ms);
}

void ull_audio_v2_dongle_silence_detection_enable(audio_scenario_type_t scenario)
{
    ull_audio_v2_dongle_silence_detection_handle_t *silence_detection_handle = NULL;

    /* get handle */
    if ((scenario < AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) || (scenario > AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0))
    {
        AUDIO_ASSERT(0);
    }
    silence_detection_handle = &ull_audio_v2_dongle_silence_detection_handle[scenario-AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0];

    silence_detection_handle->enable = 1;
    silence_detection_handle->status = ULL_AUDIO_V2_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW;
    memset(silence_detection_handle->failure_delay_us_count, 0, sizeof(silence_detection_handle->failure_delay_us_count));
    memset(silence_detection_handle->effective_delay_us_count, 0, sizeof(silence_detection_handle->effective_delay_us_count));
    DSP_MW_LOG_I("[ULL Audio V2][DL][silence_detection][enable]:%d\r\n", 1, scenario);
}

void ull_audio_v2_dongle_silence_detection_disable(audio_scenario_type_t scenario)
{
    ull_audio_v2_dongle_silence_detection_handle_t *silence_detection_handle = NULL;

    /* get handle */
    if ((scenario < AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) || (scenario > AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0))
    {
        AUDIO_ASSERT(0);
    }
    silence_detection_handle = &ull_audio_v2_dongle_silence_detection_handle[scenario-AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0];

    silence_detection_handle->enable = 0;
    silence_detection_handle->status = ULL_AUDIO_V2_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW;
    memset(silence_detection_handle->failure_delay_us_count, 0, sizeof(silence_detection_handle->failure_delay_us_count));
    memset(silence_detection_handle->effective_delay_us_count, 0, sizeof(silence_detection_handle->effective_delay_us_count));
    DSP_MW_LOG_I("[ULL Audio V2][DL][silence_detection][disable]:%d\r\n", 1, scenario);
}

void ull_audio_v2_dongle_silence_detection_process(audio_scenario_type_t scenario)
{
    ull_audio_v2_dongle_silence_detection_handle_t *silence_detection_handle = NULL;
    volume_estimator_port_t *port;
    uint32_t i, frame_num;
    aud_msg_status_t status;
    hal_ccni_message_t msg;
    ull_audio_v2_dongle_silence_detection_status_t silence_detection_status_backup;
    bool is_silence_status, is_not_silence_status;
    uint32_t volume_estimator_calculate_frame_time_us = 0;
    /* get handle */
    if ((scenario < AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) || (scenario > AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0))
    {
        AUDIO_ASSERT(0);
    }
    silence_detection_handle = &ull_audio_v2_dongle_silence_detection_handle[scenario-AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0];

    if (silence_detection_handle->enable)
    {
        volume_estimator_calculate_frame_time_us = VOLUME_ESTIMATOR_CALCULATE_FRAME_SAMPLE_SIZE * 1000000 / silence_detection_handle->sample_rate;
        port = silence_detection_handle->port;
        LOG_AUDIO_DUMP(silence_detection_handle->data_buf, silence_detection_handle->data_size, AUDIO_WOOFER_CPD_OUT);
        is_silence_status = false;
        is_not_silence_status = false;
        for (i = 0; i < silence_detection_handle->total_channel; i++) {
            for (frame_num = 0; frame_num < (silence_detection_handle->data_size_channel/silence_detection_handle->frame_size); frame_num++)
            {
                /* get the latest volume of each 2.5ms frame and the current_db will be updated */
                if (volume_estimator_process(port, (void *)((uint8_t *)(silence_detection_handle->data_buf) + silence_detection_handle->data_size_channel*i + silence_detection_handle->frame_size*frame_num), silence_detection_handle->frame_size, &silence_detection_handle->current_db) != VOLUME_ESTIMATOR_STATUS_OK)
                {
                    AUDIO_ASSERT(0);
                }
                //DSP_MW_LOG_I("[ULL Audio V2][DL][silence_detection][volume]:scenario %d, ch %d, current_db %d\r\n", 3, scenario, i, silence_detection_handle->current_db);
                /* check this frame's volume */
                if (silence_detection_handle->current_db <= silence_detection_handle->effective_threshold_db)
                {
                    silence_detection_handle->failure_delay_us_count[i] = 0;
                    silence_detection_handle->effective_delay_us_count[i] += volume_estimator_calculate_frame_time_us;
                    if (silence_detection_handle->effective_delay_us_count[i] >= silence_detection_handle->effective_delay_ms*1000)
                    {
                        if (silence_detection_handle->status != ULL_AUDIO_V2_DONGLE_SILENCE_DETECTION_STATUS_SILENCE)
                        {
                            is_silence_status = true;
                        }
                        else
                        {
                            /* do nothing because the silence status had been sent */
                            is_silence_status = false;
                        }
                        /* prevent overflow */
                        silence_detection_handle->effective_delay_us_count[i] = silence_detection_handle->effective_delay_ms*1000;
                    } else {
                        is_silence_status = false;
                    }
                }
                else if (silence_detection_handle->current_db >= silence_detection_handle->failure_threshold_db)
                {
                    is_silence_status = false;
                    silence_detection_handle->effective_delay_us_count[i] = 0;
                    silence_detection_handle->failure_delay_us_count[i] += volume_estimator_calculate_frame_time_us;
                    if (silence_detection_handle->failure_delay_us_count[i] >= silence_detection_handle->failure_delay_ms*1000)
                    {
                        if (silence_detection_handle->status != ULL_AUDIO_V2_DONGLE_SILENCE_DETECTION_STATUS_NOT_SILENCE)
                        {
                            is_not_silence_status = true;
                        }
                        else
                        {
                            /* do nothing because the not silence status had been sent */
                        }
                        /* prevent overflow */
                        silence_detection_handle->failure_delay_us_count[i] = silence_detection_handle->failure_delay_ms*1000;
                    }
                }
                else
                {
                    /* keep status so do nothing */
                    is_silence_status = false;
                }
            }
        }

        /* Send the silence detection event to MCU side */
        if (is_not_silence_status == true) {
            /* update status */
            silence_detection_status_backup = silence_detection_handle->status;
            silence_detection_handle->status = ULL_AUDIO_V2_DONGLE_SILENCE_DETECTION_STATUS_NOT_SILENCE;
            /* send msg to mcu side to alert that this is not a silence period */
            msg.ccni_message[0] = ((MSG_DSP2MCU_BT_AUDIO_DL_SILENCE_DETECTION_FEEDBACK << 16) | 0);
            msg.ccni_message[1] = scenario;
            status = aud_msg_tx_handler(msg, 0, FALSE);
            if (status != AUDIO_MSG_STATUS_OK)
            {
                /* rollback state machine that the next silence period will send msg again */
                silence_detection_handle->status = silence_detection_status_backup;
            }
            DSP_MW_LOG_I("[ULL Audio V2][DL][silence_detection][send msg]: scenario %d, %d, %d, %d, %d, %d, %d\r\n", 7,
                        scenario,
                        silence_detection_handle->status,
                        silence_detection_handle->current_db,
                        silence_detection_handle->failure_threshold_db,
                        silence_detection_handle->failure_delay_ms,
                        silence_detection_handle->failure_delay_us_count[i],
                        status);
        } else {
            if (is_silence_status == true) {
                /* update status */
                silence_detection_status_backup = silence_detection_handle->status;
                silence_detection_handle->status = ULL_AUDIO_V2_DONGLE_SILENCE_DETECTION_STATUS_SILENCE;
                /* send msg to mcu side to alert that this is a silence period */
                msg.ccni_message[0] = ((MSG_DSP2MCU_BT_AUDIO_DL_SILENCE_DETECTION_FEEDBACK << 16) | 1);
                msg.ccni_message[1] = scenario;
                status = aud_msg_tx_handler(msg, 0, FALSE);
                if (status != AUDIO_MSG_STATUS_OK)
                {
                    /* rollback state machine that the next silence period will send msg again */
                    silence_detection_handle->status = silence_detection_status_backup;
                }
                DSP_MW_LOG_I("[ULL Audio V2][DL][silence_detection][send msg]: scenario %d, %d, %d, %d, %d, %d, %d\r\n", 7,
                            scenario,
                            silence_detection_handle->status,
                            silence_detection_handle->current_db,
                            silence_detection_handle->effective_threshold_db,
                            silence_detection_handle->effective_delay_ms,
                            silence_detection_handle->effective_delay_us_count[i],
                            status);
            }
        }
    }
}
#endif /* AIR_SILENCE_DETECTION_ENABLE */

/******************************************************************************/
/*           Gaming mode dongle game/chat volume balance functions            */
/******************************************************************************/
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_game_chat_volume_smart_balance_enable(void)
{
    uint32_t saved_mask;
    void *owner;
    volume_estimator_port_t *port = NULL;
    volume_estimator_config_t config;
    uint32_t data_size;
    void *data_buf;
    void *nvkey_buf;
    ull_audio_v2_dongle_dl_handle_t *dongle_handle = NULL;

    nvkey_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, sizeof(gaming_mode_vol_balance_nvkey_t));
    if (nvkey_buf == NULL) {
        AUDIO_ASSERT(0);
    }
    if (nvkey_read_full_key(NVKEY_DSP_PARA_GAME_CHAT_VOLUME_SMART_BALANCE, nvkey_buf, sizeof(gaming_mode_vol_balance_nvkey_t)) != NVDM_STATUS_NAT_OK) {
        AUDIO_ASSERT(0);
    }

    /* get handle */
    dongle_handle = ull_audio_v2_dongle_vol_balance_handle.chat_dongle_handle;

    /* get chat volume estimator port */
    owner = dongle_handle->source;
    port = volume_estimator_get_port(owner);
    if (port == NULL) {
        AUDIO_ASSERT(0);
    }

    /* init chat volume estimator port */
    config.channel_num = 1;
    config.frame_size = dongle_handle->sink_info.bt_out.sample_rate/1000*sizeof(int32_t)*5/2; /* 2.5ms*48K*mono*32bit */
    config.resolution = RESOLUTION_32BIT;
    config.mode = VOLUME_ESTIMATOR_CHAT_INSTANT_MODE;
    config.sample_rate = dongle_handle->sink_info.bt_out.sample_rate;
    config.nvkey_para = (void *) & (((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->chat_vol_nvkey);
    volume_estimator_init(port, &config);

    /* malloc memory for queue stereo pcm data */
    data_size = dongle_handle->sink_info.bt_out.frame_samples * sizeof(int32_t);
    data_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, data_size);
    if (data_buf == NULL) {
        AUDIO_ASSERT(0);
    }
    memset(data_buf, 0, data_size);

    /* update state machine */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (ull_audio_v2_dongle_vol_balance_handle.port != NULL) {
        AUDIO_ASSERT(0);
    }
    ull_audio_v2_dongle_vol_balance_handle.port = port;
    ull_audio_v2_dongle_vol_balance_handle.nvkey_buf = nvkey_buf;
    ull_audio_v2_dongle_vol_balance_handle.enable = (int16_t)(((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->enable);
    ull_audio_v2_dongle_vol_balance_handle.gain_setting_update_done = 0;
    ull_audio_v2_dongle_vol_balance_handle.current_db = -144 * 100;
    ull_audio_v2_dongle_vol_balance_handle.effective_threshold_db = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->effective_threshold_db;
    ull_audio_v2_dongle_vol_balance_handle.effective_delay_ms = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->effective_delay_ms;
    ull_audio_v2_dongle_vol_balance_handle.effective_delay_us_count = 0;
    ull_audio_v2_dongle_vol_balance_handle.failure_threshold_db = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->failure_threshold_db;
    ull_audio_v2_dongle_vol_balance_handle.failure_delay_ms = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->failure_delay_ms;
    ull_audio_v2_dongle_vol_balance_handle.failure_delay_us_count = 0;
    ull_audio_v2_dongle_vol_balance_handle.adjustment_amount_db = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->adjustment_amount_db;
    ull_audio_v2_dongle_vol_balance_handle.up_step_db = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->up_step_db;
    ull_audio_v2_dongle_vol_balance_handle.down_step_db = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->down_step_db;
    ull_audio_v2_dongle_vol_balance_handle.active_flag = false;
    ull_audio_v2_dongle_vol_balance_handle.frame_size = config.frame_size;
    ull_audio_v2_dongle_vol_balance_handle.data_size = 0;
    ull_audio_v2_dongle_vol_balance_handle.data_buf_size = data_size;
    ull_audio_v2_dongle_vol_balance_handle.data_buf = data_buf;
    hal_nvic_restore_interrupt_mask(saved_mask);

    ULL_V2_LOG_I("[Game/Chat balance][enable]:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 16,
                 ull_audio_v2_dongle_vol_balance_handle.enable,
                 ull_audio_v2_dongle_vol_balance_handle.active_flag,
                 ull_audio_v2_dongle_vol_balance_handle.current_db,
                 ull_audio_v2_dongle_vol_balance_handle.effective_threshold_db,
                 ull_audio_v2_dongle_vol_balance_handle.effective_delay_ms,
                 ull_audio_v2_dongle_vol_balance_handle.failure_threshold_db,
                 ull_audio_v2_dongle_vol_balance_handle.failure_delay_ms,
                 ull_audio_v2_dongle_vol_balance_handle.adjustment_amount_db,
                 ull_audio_v2_dongle_vol_balance_handle.up_step_db,
                 ull_audio_v2_dongle_vol_balance_handle.down_step_db,
                 ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[0],
                 ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[1],
                 ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0],
                 ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1],
                 ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0],
                 ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1]);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_game_chat_volume_smart_balance_disable(void)
{
    uint32_t saved_mask;
    volume_estimator_port_t *port;

    /* check state machine */
    if ((ull_audio_v2_dongle_vol_balance_handle.port == NULL) || (ull_audio_v2_dongle_vol_balance_handle.data_buf == NULL) || (ull_audio_v2_dongle_vol_balance_handle.nvkey_buf == NULL) || (ull_audio_v2_dongle_vol_balance_handle.data_size != 480)) {
        AUDIO_ASSERT(0);
    }
    port = ull_audio_v2_dongle_vol_balance_handle.port;

    /* deinit chat volume estimator port */
    volume_estimator_deinit(port);

    /* free memory */
    preloader_pisplit_free_memory(ull_audio_v2_dongle_vol_balance_handle.data_buf);
    preloader_pisplit_free_memory(ull_audio_v2_dongle_vol_balance_handle.nvkey_buf);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if ((ull_audio_v2_dongle_vol_balance_handle.enable) && (ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port != NULL)) {
        /* process L-channel */
        if (ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0] != ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0]) {
            /* recover gain to the default settings */
            stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 1, ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0]);
            ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] = ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0];
        }

        /* process R-channel */
        if (ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1] != ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1]) {
            /* recover gain to the default settings */
            stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 2, ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1]);
            ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] = ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1];
        }

        /* restore ramp-up and ramp-down setting */
        stream_function_sw_gain_configure_gain_up(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port,   1,  1, 2);
        stream_function_sw_gain_configure_gain_up(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port,   2,  1, 2);
        stream_function_sw_gain_configure_gain_down(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 1, -1, 2);
        stream_function_sw_gain_configure_gain_down(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 2, -1, 2);
    }

    /* reset state machine */
    ull_audio_v2_dongle_vol_balance_handle.port = NULL;
    ull_audio_v2_dongle_vol_balance_handle.nvkey_buf = NULL;
    ull_audio_v2_dongle_vol_balance_handle.enable = 0;
    ull_audio_v2_dongle_vol_balance_handle.gain_setting_update_done = 0;
    ull_audio_v2_dongle_vol_balance_handle.current_db = -144 * 100;
    ull_audio_v2_dongle_vol_balance_handle.effective_threshold_db = 0;
    ull_audio_v2_dongle_vol_balance_handle.effective_delay_ms = 0;
    ull_audio_v2_dongle_vol_balance_handle.effective_delay_us_count = 0;
    ull_audio_v2_dongle_vol_balance_handle.failure_threshold_db = 0;
    ull_audio_v2_dongle_vol_balance_handle.failure_delay_ms = 0;
    ull_audio_v2_dongle_vol_balance_handle.failure_delay_us_count = 0;
    ull_audio_v2_dongle_vol_balance_handle.adjustment_amount_db = 0;
    ull_audio_v2_dongle_vol_balance_handle.up_step_db = 0;
    ull_audio_v2_dongle_vol_balance_handle.down_step_db = 0;
    ull_audio_v2_dongle_vol_balance_handle.active_flag = false;
    ull_audio_v2_dongle_vol_balance_handle.data_size = 0;
    ull_audio_v2_dongle_vol_balance_handle.data_buf_size = 0;
    ull_audio_v2_dongle_vol_balance_handle.data_buf = NULL;
    hal_nvic_restore_interrupt_mask(saved_mask);

    ULL_V2_LOG_I("[Game/Chat balance][disable]:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 16,
                 ull_audio_v2_dongle_vol_balance_handle.enable,
                 ull_audio_v2_dongle_vol_balance_handle.active_flag,
                 ull_audio_v2_dongle_vol_balance_handle.current_db,
                 ull_audio_v2_dongle_vol_balance_handle.effective_threshold_db,
                 ull_audio_v2_dongle_vol_balance_handle.effective_delay_ms,
                 ull_audio_v2_dongle_vol_balance_handle.failure_threshold_db,
                 ull_audio_v2_dongle_vol_balance_handle.failure_delay_ms,
                 ull_audio_v2_dongle_vol_balance_handle.adjustment_amount_db,
                 ull_audio_v2_dongle_vol_balance_handle.up_step_db,
                 ull_audio_v2_dongle_vol_balance_handle.down_step_db,
                 ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[0],
                 ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[1],
                 ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0],
                 ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1],
                 ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0],
                 ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1]);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ull_audio_v2_dongle_game_chat_volume_smart_balance_do_process(void)
{
    uint32_t saved_mask;
    volume_estimator_port_t *port;
    int32_t new_gain_1;
    int32_t new_gain_2;
    bool gain_change = false;
    uint32_t frame_num;

    /* check state machine */
    if ((ull_audio_v2_dongle_vol_balance_handle.port == NULL) || (ull_audio_v2_dongle_vol_balance_handle.data_buf == NULL) || (ull_audio_v2_dongle_vol_balance_handle.nvkey_buf == NULL) || (ull_audio_v2_dongle_vol_balance_handle.data_size != 480)) {
        AUDIO_ASSERT(0);
    }
    port = ull_audio_v2_dongle_vol_balance_handle.port;

    if (ull_audio_v2_dongle_vol_balance_handle.enable)
    {
        /* update ramp-up/ramp-down settings at the first time */
        if ((ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port != NULL) && (ull_audio_v2_dongle_vol_balance_handle.gain_setting_update_done == 0))
        {
            /* do ramp-up and ramp-down setting */
            stream_function_sw_gain_configure_gain_up(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port,   1, ull_audio_v2_dongle_vol_balance_handle.up_step_db,   2);
            stream_function_sw_gain_configure_gain_up(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port,   2, ull_audio_v2_dongle_vol_balance_handle.up_step_db,   2);
            stream_function_sw_gain_configure_gain_down(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 1, ull_audio_v2_dongle_vol_balance_handle.down_step_db, 2);
            stream_function_sw_gain_configure_gain_down(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 2, ull_audio_v2_dongle_vol_balance_handle.down_step_db, 2);
            /* set flag */
            ull_audio_v2_dongle_vol_balance_handle.gain_setting_update_done = 1;
        }

        for (frame_num = 0; frame_num < (ull_audio_v2_dongle_vol_balance_handle.data_size/ull_audio_v2_dongle_vol_balance_handle.frame_size); frame_num++)
        {
            /* get the latest volume on chat path */
            if (volume_estimator_process(port, (void *)((uint8_t *)(ull_audio_v2_dongle_vol_balance_handle.data_buf)+ull_audio_v2_dongle_vol_balance_handle.frame_size*frame_num), ull_audio_v2_dongle_vol_balance_handle.frame_size, &ull_audio_v2_dongle_vol_balance_handle.current_db) != VOLUME_ESTIMATOR_STATUS_OK) {
                AUDIO_ASSERT(0);
            }
            // ULL_V2_LOG_I("[Game/Chat balance][get chat gain]:%d\r\n", 1, ull_audio_v2_dongle_vol_balance_handle.current_db);

            /* check if needs to do the balance */
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            if (ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port != NULL)
            {
                if ((ull_audio_v2_dongle_vol_balance_handle.current_db >= ull_audio_v2_dongle_vol_balance_handle.effective_threshold_db) &&
                    (ull_audio_v2_dongle_vol_balance_handle.effective_delay_us_count >= ull_audio_v2_dongle_vol_balance_handle.effective_delay_ms * 1000))
                {
                    /* there is chat voice on the chat path */
                    /* process L-channel */
                    new_gain_1 = (ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[0] + ull_audio_v2_dongle_vol_balance_handle.adjustment_amount_db);
                    new_gain_1 = min(new_gain_1, ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0]);
                    if (ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] != new_gain_1)
                    {
                        /* the default gain is larger than the target and the current gain is not equal to the target, so we need to ramp down the volume on the game path */
                        stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 1, new_gain_1);
                        ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] = new_gain_1;
                        gain_change = true;
                    }

                    /* process R-channel */
                    new_gain_2 = (ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[1] + ull_audio_v2_dongle_vol_balance_handle.adjustment_amount_db);
                    new_gain_2 = min(new_gain_2, ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1]);
                    if (ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] != new_gain_2) {
                        /* the default gain is larger than the target and the current gain is not equal to the target, so we need to ramp down the volume on the game path */
                        stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 2, new_gain_2);
                        ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] = new_gain_2;
                        gain_change = true;
                    }

                    /* set active flag */
                    ull_audio_v2_dongle_vol_balance_handle.active_flag = true;

                    /* reset failure_delay_us_count */
                    ull_audio_v2_dongle_vol_balance_handle.failure_delay_us_count = 0;
                }
                else if ((ull_audio_v2_dongle_vol_balance_handle.current_db >= ull_audio_v2_dongle_vol_balance_handle.effective_threshold_db) &&
                        (ull_audio_v2_dongle_vol_balance_handle.effective_delay_us_count < ull_audio_v2_dongle_vol_balance_handle.effective_delay_ms * 1000))
                {
                    /* there is chat voice on the chat path but time is not enough */
                    ull_audio_v2_dongle_vol_balance_handle.effective_delay_us_count += 2500;

                    /* reset failure_delay_us_count */
                    ull_audio_v2_dongle_vol_balance_handle.failure_delay_us_count = 0;
                }
                else if ((ull_audio_v2_dongle_vol_balance_handle.current_db <= ull_audio_v2_dongle_vol_balance_handle.failure_threshold_db) &&
                        (ull_audio_v2_dongle_vol_balance_handle.failure_delay_us_count >= ull_audio_v2_dongle_vol_balance_handle.failure_delay_ms * 1000))
                {
                    /* there is no chat voice on the chat path */
                    /* process L-channel */
                    if (ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0] != ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0]) {
                        /* recover gain to the default settings */
                        stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 1, ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0]);
                        ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] = ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0];
                        gain_change = true;
                    }

                    /* process R-channel */
                    if (ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1] != ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1]) {
                        /* recover gain to the default settings */
                        stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 2, ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1]);
                        ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] = ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1];
                        gain_change = true;
                    }

                    /* set active flag */
                    ull_audio_v2_dongle_vol_balance_handle.active_flag = false;

                    /* reset effective_delay_us_count */
                    ull_audio_v2_dongle_vol_balance_handle.effective_delay_us_count = 0;
                }
                else if ((ull_audio_v2_dongle_vol_balance_handle.current_db <= ull_audio_v2_dongle_vol_balance_handle.failure_threshold_db) &&
                        (ull_audio_v2_dongle_vol_balance_handle.failure_delay_us_count < ull_audio_v2_dongle_vol_balance_handle.failure_delay_ms * 1000))
                {
                    /* there is no chat voice on the chat path but time is not enough */
                    ull_audio_v2_dongle_vol_balance_handle.failure_delay_us_count += 2500;

                    /* reset effective_delay_us_count */
                    ull_audio_v2_dongle_vol_balance_handle.effective_delay_us_count = 0;

                    /* process L-channel */
                    if ((ull_audio_v2_dongle_vol_balance_handle.active_flag) && (ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[0] < ull_audio_v2_dongle_vol_balance_handle.effective_threshold_db)) {
                        /* In this stage(chat/game balance is actived, but chat spk volume is adjusted by end-user), we should recover gain to the default settings because the chat gain is too less */
                        new_gain_1 = ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0];
                        if (ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] != new_gain_1) {
                            stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 1, new_gain_1);
                            ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] = new_gain_1;
                            gain_change = true;
                        }
                    } else if (ull_audio_v2_dongle_vol_balance_handle.active_flag) {
                        /* In this stage(chat/game balance is actived, but game spk volume is adjusted by end-user), we should set gain to the minimum in default gain and smart-target gain */
                        new_gain_1 = (ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[0] + ull_audio_v2_dongle_vol_balance_handle.adjustment_amount_db);
                        new_gain_1 = min(new_gain_1, ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0]);
                        if (ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] != new_gain_1) {
                            /* set gain to the minimun settings */
                            stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 1, new_gain_1);
                            ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] = new_gain_1;
                            gain_change = true;
                        }
                    } else {
                        /* In normal default vol and balance vol should be the same, this flow is designed for preventing errors */
                        if (ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0] != ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0]) {
                            /* recover gain to the default settings */
                            stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 1, ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0]);
                            ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0] = ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0];
                            gain_change = true;
                        }
                    }

                    /* process R-channel */
                    if ((ull_audio_v2_dongle_vol_balance_handle.active_flag) && (ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[1] < ull_audio_v2_dongle_vol_balance_handle.effective_threshold_db)) {
                        /* In this stage, we should recover gain to the default settings because the chat gain is too less */
                        new_gain_2 = ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1];
                        if (ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] != new_gain_2) {
                            stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 2, new_gain_2);
                            ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] = new_gain_2;
                            gain_change = true;
                        }
                    } else if (ull_audio_v2_dongle_vol_balance_handle.active_flag) {
                        /* In this stage(chat/game balance is actived, but game spk volume is adjusted by end-user), we should set gain to the minimum in default gain and smart-target gain */
                        new_gain_2 = (ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[1] + ull_audio_v2_dongle_vol_balance_handle.adjustment_amount_db);
                        new_gain_2 = min(new_gain_2, ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1]);
                        if (ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] != new_gain_2) {
                            /* recover gain to the minimun settings */
                            stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 2, new_gain_2);
                            ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] = new_gain_2;
                            gain_change = true;
                        }
                    } else {
                        /* In normal default vol and balance vol should be the same, this flow is designed for preventing errors */
                        if (ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1] != ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1]) {
                            /* recover gain to the default settings */
                            stream_function_sw_gain_configure_gain_target(ull_audio_v2_dongle_vol_balance_handle.game_path_gain_port, 2, ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1]);
                            ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1] = ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1];
                            gain_change = true;
                        }
                    }
                }
            }
            hal_nvic_restore_interrupt_mask(saved_mask);
        }
    }

    if (gain_change) {
        ULL_V2_LOG_I("[Game/Chat balance][change gain]:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 13,
                     ull_audio_v2_dongle_vol_balance_handle.active_flag,
                     ull_audio_v2_dongle_vol_balance_handle.current_db,
                     ull_audio_v2_dongle_vol_balance_handle.effective_threshold_db,
                     ull_audio_v2_dongle_vol_balance_handle.effective_delay_us_count,
                     ull_audio_v2_dongle_vol_balance_handle.failure_threshold_db,
                     ull_audio_v2_dongle_vol_balance_handle.failure_delay_us_count,
                     ull_audio_v2_dongle_vol_balance_handle.adjustment_amount_db,
                     ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[0],
                     ull_audio_v2_dongle_vol_balance_handle.chat_path_default_vol_gain[1],
                     ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[0],
                     ull_audio_v2_dongle_vol_balance_handle.game_path_default_vol_gain[1],
                     ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[0],
                     ull_audio_v2_dongle_vol_balance_handle.game_path_balance_vol_gain[1]);
    }
}
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
