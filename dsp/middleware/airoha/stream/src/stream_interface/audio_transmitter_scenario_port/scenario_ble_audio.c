/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "scenario_ble_audio.h"
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
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
#include "clk_skew_sw.h"
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */
#ifdef AIR_SOFTWARE_GAIN_ENABLE
#include "sw_gain_interface.h"
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
#include "sw_buffer_interface.h"
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */
#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
#include "lc3plus_enc_interface.h"
#endif
#include "lc3_enc_branch_interface.h"
#include "lc3_dec_interface_v2.h"
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */
#include "audio_transmitter_mcu_dsp_common.h"
#include "hal_audio_message_struct_common.h"
#ifdef AIR_SILENCE_DETECTION_ENABLE
#include "volume_estimator_interface.h"
#include "dtm.h"
#include "audio_nvdm_common.h"
#include "preloader_pisplit.h"
#include "stream_nvkey_struct.h"
#include "scenario_audio_common.h"
#endif /* AIR_SILENCE_DETECTION_ENABLE */
#include "hal_nvic_internal.h"
#include "task.h"

/* Private define ------------------------------------------------------------*/
#define BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG                       0
#define BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_DUMP                      1
#define BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LANTENCY                  0
#define BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG                       0
#define BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP                      1

#define DL_MIN_USB_FRAMES_IN_START                                  (11)
#define DL_SAMPLES_IN_EACH_USB_FRAME                                (48)
#define DL_SAMPLES_IN_EACH_CODEC_FRAME                              (48*10)
#define DL_DATA_SIZE_IN_EACH_LC3_FRAME                              (155)

#define UL_MIN_USB_FRAMES_IN_START                                  (10)
#define UL_SAMPLES_IN_EACH_USB_FRAME                                (32)
#define UL_SAMPLES_IN_EACH_CODEC_FRAME                              (32*10)

#define UL_BT_TIMESTAMP_DIFF                                        (20)
#define UL_PLAYEN_DELAY_FRAME_10000US                               (4) // process time = 1.25ms
#define UL_FIRST_PACKET_SAFE_DELAY_FRAME_10000US                    (0)
#define UL_BT_RETRY_WINDOW_FRAME_10000US                            (32)
#define UL_BT_INTERVAL_FRAME_10000US                                (32)
#define UL_PLAYEN_DELAY_FRAME_7500US                                (4) // process time = 1.25ms
#define UL_FIRST_PACKET_SAFE_DELAY_FRAME_7500US                     (0)
#define UL_BT_RETRY_WINDOW_FRAME_7500US                             (24)
#define UL_BT_INTERVAL_FRAME_7500US                                 (24)
#define UL_OUTPUT_MIN_FRAMES                                        (3+2)
#define UL_USB_OUT_PREFILL_MS                                       (3) // 3ms

log_create_module(ble_dongle_log, PRINT_LEVEL_INFO);

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
    U8 valid_packet; /* valid packet: 0x01, invalid packet 0x00 or 0x03 */
    U8 PduLen ; /* payload size */
    U8 _reserved_byte_0Fh;
    U16 DataLen;
    U16 _reserved_word_12h;
    U32 _reserved_long_0;
    U32 _reserved_long_1;
} LE_AUDIO_HEADER;

typedef struct {
    int32_t gain_1;
    int32_t gain_2;
} op_vol_level_param_t;

typedef struct {
    uint8_t scenario_id;
    uint8_t sub_id;
} dl_mixer_param_t;

typedef struct {
    uint32_t latency_us;
} latency_param_t;

typedef union {
    op_vol_level_param_t      vol_level;
    dl_mixer_param_t          dl_mixer_param;
    latency_param_t           latency_param;
    void                     *share_info;
    uint32_t                  channel_enable;
} ble_audio_dongle_runtime_config_operation_param_t, *ble_audio_dongle_runtime_config_operation_param_p;

typedef struct {
    ble_audio_dongle_runtime_config_operation_t          config_operation;
    ble_audio_dongle_runtime_config_operation_param_t    config_param;
} ble_audio_dongle_runtime_config_param_t, *ble_audio_dongle_runtime_config_param_p;

#ifdef AIR_SILENCE_DETECTION_ENABLE
typedef enum {
    BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW = 0,
    BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_SILENCE,
    BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_NOT_SILENCE,
} ble_audio_dongle_silence_detection_status_t;

typedef struct {
    volume_estimator_port_t *port;
    int16_t enable;
    int16_t nvkey_enable;
    void *nvkey_buf;
    uint32_t process_size;
    uint32_t data_size;
    uint32_t data_buf_size;
    void *data_buf;
    int32_t current_db;
    int32_t failure_threshold_db;
    int32_t effective_threshold_db;
    uint32_t effective_delay_ms;
    uint32_t effective_delay_us_count;
    uint32_t failure_delay_ms;
    uint32_t failure_delay_us_count;
    ble_audio_dongle_silence_detection_status_t status;
} ble_audio_dongle_silence_detection_handle_t;
#endif /* AIR_SILENCE_DETECTION_ENABLE */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static LE_AUDIO_INFO ble_audio_dl_info;
static LE_AUDIO_INFO ble_audio_ul_info;
#ifdef AIR_SILENCE_DETECTION_ENABLE
static uint32_t ble_audio_dongle_dl_sw_timer = 0;
static uint32_t ble_audio_dongle_dl_sw_timer_count = 0;
static bool ble_audio_dongle_dl_without_bt_link_mode_enable = false;
#endif /* AIR_SILENCE_DETECTION_ENABLE */
static SemaphoreHandle_t g_ble_audio_dongle_dl_xSemaphore = NULL;

extern stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_in_broadcast_0[];
extern stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_in_broadcast_1[];

/* Public variables ----------------------------------------------------------*/
ble_audio_dongle_dl_handle_t *ble_audio_dongle_first_dl_handle = NULL;
ble_audio_dongle_ul_handle_t *ble_audio_dongle_first_ul_handle = NULL;
#ifdef AIR_SILENCE_DETECTION_ENABLE
ble_audio_dongle_silence_detection_handle_t ble_audio_dongle_silence_detection_handle[AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN + 1 - AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0] = {0};
#endif /* AIR_SILENCE_DETECTION_ENABLE */

extern audio_dongle_init_le_play_info_t audio_dongle_bt_init_play_info;

/* Private functions ---------------------------------------------------------*/
#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
static void ble_audio_dongle_dl_afe_in_init(
    ble_audio_dongle_dl_handle_t *dongle_handle,
    audio_transmitter_open_param_p audio_transmitter_open_param,
    bt_common_open_param_p bt_common_open_param
    );
static void ble_audio_dongle_dl_afe_in_deinit(
    ble_audio_dongle_dl_handle_t *dongle_handle
    );
static void ble_audio_dongle_dl_afe_in_start(
    ble_audio_dongle_dl_handle_t *dongle_handle
    );
static uint32_t ble_audio_dongle_dl_calculate_afe_in_jitter_bt_audio(
    ble_audio_dongle_dl_handle_t *dongle_handle,
    uint32_t gpt_count);
extern bool g_i2s_slv_common_tracking_start_flag;
#endif
EXTERN VOID StreamDSP_HWSemaphoreTake(VOID);
EXTERN VOID StreamDSP_HWSemaphoreGive(VOID);
extern VOID audio_transmitter_share_information_update_read_offset(SOURCE source, U32 ReadOffset);
extern VOID audio_transmitter_share_information_fetch(SOURCE source, SINK sink);
extern void bt_common_share_information_fetch(SOURCE source, SINK sink);
extern void bt_common_share_information_update_read_offset(SOURCE source, U32 ReadOffset);
extern VOID MCE_GetBtClk(BTCLK *pCurrCLK, BTPHASE *pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);
extern VOID audio_transmitter_share_information_update_write_offset(SINK sink, U32 WriteOffset);

extern hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask);
extern hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);

static uint32_t usb_audio_get_frame_size(audio_dsp_codec_type_t *usb_type, audio_codec_param_t *usb_param)
{
    uint32_t frame_size = 0;
    uint32_t samples = 0;
    uint32_t channel_num = 0;
    uint32_t resolution_size = 0;

    if (*usb_type == AUDIO_DSP_CODEC_TYPE_PCM) {
        frame_size = 1;

        switch (usb_param->pcm.sample_rate) {
            case 44100:
                samples = 44;
                break;

            case 16000:
            case 32000:
            case 48000:
            case 96000:
                samples = usb_param->pcm.sample_rate / 1000;
                break;

            default:
                AUDIO_ASSERT(0);
                break;
        }

        switch (usb_param->pcm.channel_mode) {
            case 1:
                channel_num = 1;
                break;

            case 2:
                channel_num = 2;
                break;

            default:
                AUDIO_ASSERT(0);
                break;
        }

        switch (usb_param->pcm.format) {
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
    } else {
        frame_size = 0;
    }

    frame_size = frame_size * samples * channel_num * resolution_size;

    return frame_size;
}

#if 0
static uint32_t ble_audio_codec_get_frame_size(audio_dsp_codec_type_t *codec_type, audio_codec_param_t *codec_param)
{
    uint32_t frame_size = 0;

    if (*codec_type != AUDIO_DSP_CODEC_TYPE_LC3) {
        AUDIO_ASSERT(0);
    }

    frame_size = codec_param->lc3.bit_rate * codec_param->lc3.frame_interval / 8 / 1000 / 1000;

    return frame_size;
}
#endif

/******************************************************************************/
/*         BLE audio source dongle music path Private Functions               */
/******************************************************************************/
static void ble_audio_dongle_dl_audio_info_init(bt_common_open_param_p bt_common_open_param, SINK sink, uint8_t **share_buffer_info, uint32_t channel_enable)
{
    uint32_t i;

    // hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    /* init ble audio info */
    if (ble_audio_dl_info.status == 0) {
        bt_common_share_information_fetch(NULL, sink);

        if (bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_type == AUDIO_DSP_CODEC_TYPE_LC3) {
            ble_audio_dl_info.lc3_packet_frame_interval = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval;
            ble_audio_dl_info.lc3_packet_frame_size     = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_size;
            ble_audio_dl_info.lc3_packet_frame_samples  = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.sample_rate / 1000 * bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval / 1000;
            ble_audio_dl_info.lc3_packet_frame_sample_byte = sizeof(int16_t);
        } else if (bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS) {
            ble_audio_dl_info.lc3_packet_frame_interval = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.frame_interval;
            ble_audio_dl_info.lc3_packet_frame_size     = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.frame_size;
            ble_audio_dl_info.lc3_packet_frame_samples  = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.sample_rate / 1000 * bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.frame_interval / 1000;
            if (bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
                ble_audio_dl_info.lc3_packet_frame_sample_byte = sizeof(int16_t);
            } else {
                ble_audio_dl_info.lc3_packet_frame_sample_byte = sizeof(int32_t);
            }
        }
        ble_audio_dl_info.share_buffer_blk_size     = sink->streamBuffer.AVMBufferInfo.MemBlkSize;
        ble_audio_dl_info.share_buffer_blk_num      = sink->streamBuffer.AVMBufferInfo.MemBlkNum;
        ble_audio_dl_info.share_buffer_blk_index    = 0;
        ble_audio_dl_info.share_buffer_blk_index_previous = 0;
        ble_audio_dl_info.channel_enable            = channel_enable;
        for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
            ble_audio_dl_info.p_share_buffer_info[i] = (n9_dsp_share_info_t *)hal_memview_cm4_to_dsp0((uint32_t)(*(share_buffer_info+i)));
        }
        ble_audio_dl_info.status = 1;
    }

    // hal_nvic_restore_interrupt_mask(saved_mask);

    BLE_LOG_I("[ble audio dongle][dl][audio_info_init]: %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x\r\n", 9,
                 ble_audio_dl_info.lc3_packet_frame_interval,
                 ble_audio_dl_info.lc3_packet_frame_size,
                 ble_audio_dl_info.lc3_packet_frame_samples,
                 ble_audio_dl_info.lc3_packet_frame_sample_byte,
                 ble_audio_dl_info.share_buffer_blk_size,
                 ble_audio_dl_info.share_buffer_blk_num,
                 ble_audio_dl_info.p_share_buffer_info[0],
                 ble_audio_dl_info.p_share_buffer_info[1],
                 channel_enable);
}

static ble_audio_dongle_dl_handle_t *ble_audio_dongle_dl_get_handle(void *owner)
{
    // uint32_t saved_mask;
    int16_t count;
    ble_audio_dongle_dl_handle_t *dongle_handle = NULL;
    ble_audio_dongle_dl_handle_t *c_handle = NULL;

    dongle_handle = malloc(sizeof(ble_audio_dongle_dl_handle_t));
    if (dongle_handle == NULL) {
        AUDIO_ASSERT(0);
    }
    memset(dongle_handle, 0, sizeof(ble_audio_dongle_dl_handle_t));

    dongle_handle->total_number = -1;
    dongle_handle->index = -1;
    dongle_handle->owner = owner;
    dongle_handle->next_dl_handle = NULL;

    // hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (ble_audio_dongle_first_dl_handle == NULL) {
        /* there is no item on the handle list */
        dongle_handle->total_number = 1;
        dongle_handle->index = 1;

        /* set this handle as the head of the handle list */
        ble_audio_dongle_first_dl_handle = dongle_handle;
    } else {
        /* there are other items on the handle list */
        count = 1;
        c_handle = ble_audio_dongle_first_dl_handle;
        while (c_handle != NULL) {
            count += 1;

            c_handle->total_number += 1;
            if (c_handle->total_number <= 0) {
                /* error status */
                AUDIO_ASSERT(0);
            }

            if (c_handle->next_dl_handle == NULL) {
                /* c_handle is the last item on the list now */
                dongle_handle->total_number = c_handle->total_number;
                dongle_handle->index = c_handle->total_number;

                /* dongle_handle is the last item on the list now */
                c_handle->next_dl_handle = dongle_handle;

                break;
            }

            c_handle = c_handle->next_dl_handle;
        }
        if ((c_handle == NULL) || (dongle_handle->total_number != count)) {
            AUDIO_ASSERT(0);
        }
    }

    // hal_nvic_restore_interrupt_mask(saved_mask);

    return dongle_handle;
}

static void ble_audio_dongle_dl_release_handle(ble_audio_dongle_dl_handle_t *handle)
{
    uint32_t i, count;
    ble_audio_dongle_dl_handle_t *l_handle = NULL;
    ble_audio_dongle_dl_handle_t *c_handle = NULL;
    ble_audio_dongle_dl_handle_t *dongle_handle = NULL;

    if ((handle == NULL) || (ble_audio_dongle_first_dl_handle == NULL)) {
        AUDIO_ASSERT(0);
    }

    // hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (ble_audio_dongle_first_dl_handle->total_number <= 0) {
        /* error status */
        AUDIO_ASSERT(0);
    } else if ((ble_audio_dongle_first_dl_handle->total_number == 1) &&
               (ble_audio_dongle_first_dl_handle == handle)) {
        /* this handle is only one item on handle list */
        if (ble_audio_dongle_first_dl_handle->next_dl_handle != NULL) {
            AUDIO_ASSERT(0);
        }

        /* there is no item on the handle list */
        ble_audio_dongle_first_dl_handle = NULL;
    } else if ((ble_audio_dongle_first_dl_handle->total_number > 1) &&
               (ble_audio_dongle_first_dl_handle == handle)) {
        /* this handle is the first item on handle list, but there are other handles on the list */
        c_handle = ble_audio_dongle_first_dl_handle;
        count = ble_audio_dongle_first_dl_handle->total_number;
        for (i = 0; i < count; i++) {
            c_handle->total_number -= 1;
            c_handle->index -= 1;

            c_handle = c_handle->next_dl_handle;
        }
        if (c_handle != NULL) {
            /* error status */
            AUDIO_ASSERT(0);
        }

        /* change the next iten to the head */
        ble_audio_dongle_first_dl_handle = handle->next_dl_handle;
    } else {
        /* this handle is the middle item on handle list */
        c_handle = ble_audio_dongle_first_dl_handle;
        count = ble_audio_dongle_first_dl_handle->total_number;
        if (count == 1) {
            /* error status */
            AUDIO_ASSERT(0);
        }
        for (i = 0; i < count; i++) {
            if (c_handle == handle) {
                if (dongle_handle != NULL) {
                    /* error status */
                    AUDIO_ASSERT(0);
                }

                /* find out the handle on the list */
                dongle_handle = handle;
                l_handle->next_dl_handle = c_handle->next_dl_handle;
            }

            if (dongle_handle == NULL) {
                /* only the total_number of the handle in front of the released handle needs to be decreased */
                c_handle->total_number -= 1;
            } else {
                /* Both the total_number and the index of the handle in back of the released handle need to be decreased */
                c_handle->total_number -= 1;
                c_handle->index -= 1;
            }

            l_handle = c_handle;
            c_handle = c_handle->next_dl_handle;
        }
        if ((c_handle != NULL) || (dongle_handle == NULL)) {
            /* error status */
            AUDIO_ASSERT(0);
        }
    }

    // hal_nvic_restore_interrupt_mask(saved_mask);

    free(handle);
}

#ifdef AIR_SILENCE_DETECTION_ENABLE
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ble_audio_dongle_dl_software_timer_handler(void *user_data)
{
    SOURCE source = NULL;
    uint32_t saved_mask = 0;
    uint32_t i, count, gpt_count, bt_count;
    ble_audio_dongle_dl_handle_t *c_handle = NULL;
    UNUSED(user_data);

    /* repeat this timer */
    hal_gpt_sw_start_timer_us(ble_audio_dongle_dl_sw_timer, ble_audio_dl_info.lc3_packet_frame_interval, ble_audio_dongle_dl_software_timer_handler, NULL);

    if (ble_audio_dongle_first_dl_handle == NULL)
    {
        DSP_MW_LOG_E("[ble audio dongle][dl]dl handle is NULL\r\n", 0);
        return;
    }

    // hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    /* get timestamp for debug */
    bt_count = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    /* BT controller will send write index info on ccni msg */
    ble_audio_dl_info.share_buffer_blk_index = 0;
    if (ble_audio_dl_info.share_buffer_blk_index >= ble_audio_dl_info.share_buffer_blk_num)
    {
        DSP_MW_LOG_E("[ble audio dongle][dl] blk index is error, %u\r\n", 1, ble_audio_dl_info.share_buffer_blk_index);
        ble_audio_dl_info.share_buffer_blk_index = 0;
    }

    c_handle = ble_audio_dongle_first_dl_handle;
    count = ble_audio_dongle_first_dl_handle->total_number;
    for (i=0; i < count; i++)
    {
        /* get source */
        source = (SOURCE)c_handle->owner;
        if ((source == NULL) || (source->transform == NULL))
        {
            DSP_MW_LOG_E("[ble audio dongle][dl]source or transform is NULL\r\n", 0);
            /* switch to the next dl stream */
            c_handle = c_handle->next_dl_handle;
            continue;
        }

        hal_nvic_save_and_set_interrupt_mask(&saved_mask);

        /* set timestamp for debug */
        c_handle->ccni_bt_count = bt_count;
        c_handle->ccni_gpt_count = gpt_count;

        /* set fetch flag to indicate that the next flow can fetch packets form buffer */
        if(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.fetch_flag == 0)
        {
            source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.fetch_flag = 1;
        }

        hal_nvic_restore_interrupt_mask(saved_mask);

        /* Handler the stream */
        AudioCheckTransformHandle(source->transform);

        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }
    if (c_handle != NULL)
    {
        AUDIO_ASSERT(0);
    }

    // hal_nvic_restore_interrupt_mask(saved_mask);

    return;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ble_audio_dongle_dl_software_timer_start(ble_audio_dongle_dl_handle_t *handle)
{
    uint32_t saved_mask;
    UNUSED(handle);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (ble_audio_dongle_dl_sw_timer_count == 0)
    {
        hal_gpt_sw_get_timer(&ble_audio_dongle_dl_sw_timer);
        hal_gpt_sw_start_timer_us(ble_audio_dongle_dl_sw_timer, ble_audio_dl_info.lc3_packet_frame_interval, ble_audio_dongle_dl_software_timer_handler, NULL);
    }
    ble_audio_dongle_dl_sw_timer_count += 1;
    if (ble_audio_dongle_dl_sw_timer_count == 0)
    {
        AUDIO_ASSERT(0);
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ble_audio_dongle_dl_software_timer_stop(ble_audio_dongle_dl_handle_t *handle)
{
    uint32_t saved_mask;
    UNUSED(handle);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (ble_audio_dongle_dl_sw_timer_count == 0)
    {
        AUDIO_ASSERT(0);
    }
    ble_audio_dongle_dl_sw_timer_count -= 1;
    if (ble_audio_dongle_dl_sw_timer_count == 0)
    {
        hal_gpt_sw_stop_timer_ms(ble_audio_dongle_dl_sw_timer);
        hal_gpt_sw_free_timer(ble_audio_dongle_dl_sw_timer);
        ble_audio_dongle_dl_sw_timer = 0;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
}
#endif /* AIR_SILENCE_DETECTION_ENABLE */

/******************************************************************************/
/*         BLE audio source dongle voice path Private Functions               */
/******************************************************************************/
static void ble_audio_dongle_ul_audio_info_init(ble_audio_dongle_ul_handle_t *dongle_handle, SOURCE source, uint8_t **share_buffer_info, uint32_t channel_enable)
{
    uint32_t i;
    ble_audio_dongle_ul_channel_info_t *current_channel_info;

    // hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    /* init ble audio info */
    if (ble_audio_ul_info.status == 0) {
        bt_common_share_information_fetch(source, NULL);

        if (source->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.codec_type == AUDIO_DSP_CODEC_TYPE_LC3) {
            ble_audio_ul_info.lc3_packet_frame_interval = source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.frame_interval;
            ble_audio_ul_info.lc3_packet_frame_size     = source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.frame_size;
            ble_audio_ul_info.lc3_packet_frame_samples  = source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.sample_rate / 1000 * source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.frame_interval / 1000;
        } else if (source->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS) {
            ble_audio_ul_info.lc3_packet_frame_interval = source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3plus.frame_interval;
            ble_audio_ul_info.lc3_packet_frame_size     = source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3plus.frame_size;
            ble_audio_ul_info.lc3_packet_frame_samples  = source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3plus.sample_rate / 1000 * source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3plus.frame_interval / 1000;
        }
        ble_audio_ul_info.share_buffer_blk_size     = source->streamBuffer.AVMBufferInfo.MemBlkSize;
        ble_audio_ul_info.share_buffer_blk_num      = source->streamBuffer.AVMBufferInfo.MemBlkNum;
        ble_audio_ul_info.share_buffer_blk_index    = 0;
        ble_audio_ul_info.share_buffer_blk_index_previous = 0;
        ble_audio_ul_info.channel_enable            = channel_enable;
        for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
            ble_audio_ul_info.p_share_buffer_info[i] = (n9_dsp_share_info_t *)hal_memview_cm4_to_dsp0((uint32_t)(*(share_buffer_info+i)));
            current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
            if (channel_enable & (0x1 << i)) {
                current_channel_info->enable = true;
            } else {
                current_channel_info->enable = false;
            }
            current_channel_info->channel_anchor = 0;
            current_channel_info->status = BLE_AUDIO_DONGLE_UL_DATA_EMPTY;
        }
        ble_audio_ul_info.status = 1;
    }

    // hal_nvic_restore_interrupt_mask(saved_mask);

    BLE_LOG_I("[ble audio dongle][ul][audio_info_init]: %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x\r\n", 8,
                 ble_audio_ul_info.lc3_packet_frame_interval,
                 ble_audio_ul_info.lc3_packet_frame_size,
                 ble_audio_ul_info.lc3_packet_frame_samples,
                 ble_audio_ul_info.share_buffer_blk_size,
                 ble_audio_ul_info.share_buffer_blk_num,
                 ble_audio_ul_info.p_share_buffer_info[0],
                 ble_audio_ul_info.p_share_buffer_info[1],
                 channel_enable);
}

static ble_audio_dongle_ul_handle_t *ble_audio_dongle_get_ul_handle(void *owner)
{
    uint32_t saved_mask;
    int16_t count;
    ble_audio_dongle_ul_handle_t *dongle_handle = NULL;
    ble_audio_dongle_ul_handle_t *c_handle = NULL;

    dongle_handle = malloc(sizeof(ble_audio_dongle_ul_handle_t));
    if (dongle_handle == NULL) {
        AUDIO_ASSERT(0);
    }
    memset(dongle_handle, 0, sizeof(ble_audio_dongle_ul_handle_t));

    dongle_handle->total_number = -1;
    dongle_handle->index = -1;
    dongle_handle->owner = owner;
    dongle_handle->next_ul_handle = NULL;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (ble_audio_dongle_first_ul_handle == NULL) {
        /* there is no item on the handle list */
        dongle_handle->total_number = 1;
        dongle_handle->index = 1;

        /* set this handle as the head of the handle list */
        ble_audio_dongle_first_ul_handle = dongle_handle;
    } else {
        /* there are other items on the handle list */
        count = 1;
        c_handle = ble_audio_dongle_first_ul_handle;
        while (c_handle != NULL) {
            count += 1;

            c_handle->total_number += 1;
            if (c_handle->total_number <= 0) {
                /* error status */
                AUDIO_ASSERT(0);
            }

            if (c_handle->next_ul_handle == NULL) {
                /* c_handle is the last item on the list now */
                dongle_handle->total_number = c_handle->total_number;
                dongle_handle->index = c_handle->total_number;

                /* dongle_handle is the last item on the list now */
                c_handle->next_ul_handle = dongle_handle;

                break;
            }

            c_handle = c_handle->next_ul_handle;
        }
        if ((c_handle == NULL) || (dongle_handle->total_number != count)) {
            AUDIO_ASSERT(0);
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    return dongle_handle;
}

ATTR_TEXT_IN_IRAM static void ble_audio_dongle_release_ul_handle(ble_audio_dongle_ul_handle_t *handle)
{
    uint32_t saved_mask, i, count;
    ble_audio_dongle_ul_handle_t *l_handle = NULL;
    ble_audio_dongle_ul_handle_t *c_handle = NULL;
    ble_audio_dongle_ul_handle_t *dongle_handle = NULL;

    if ((handle == NULL) || (ble_audio_dongle_first_ul_handle == NULL)) {
        AUDIO_ASSERT(0);
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (ble_audio_dongle_first_ul_handle->total_number <= 0) {
        /* error status */
        AUDIO_ASSERT(0);
    } else if ((ble_audio_dongle_first_ul_handle->total_number == 1) &&
               (ble_audio_dongle_first_ul_handle == handle)) {
        /* this handle is only one item on handle list */
        if (ble_audio_dongle_first_ul_handle->next_ul_handle != NULL) {
            AUDIO_ASSERT(0);
        }

        /* there is no item on the handle list */
        ble_audio_dongle_first_ul_handle = NULL;
    } else if ((ble_audio_dongle_first_ul_handle->total_number > 1) &&
               (ble_audio_dongle_first_ul_handle == handle)) {
        /* this handle is the first item on handle list, but there are other handles on the list */
        c_handle = ble_audio_dongle_first_ul_handle;
        count = ble_audio_dongle_first_ul_handle->total_number;
        for (i = 0; i < count; i++) {
            c_handle->total_number -= 1;
            c_handle->index -= 1;

            c_handle = c_handle->next_ul_handle;
        }
        if (c_handle != NULL) {
            /* error status */
            AUDIO_ASSERT(0);
        }

        /* change the next iten to the head */
        ble_audio_dongle_first_ul_handle = handle->next_ul_handle;
    } else {
        /* this handle is the middle item on handle list */
        c_handle = ble_audio_dongle_first_ul_handle;
        count = ble_audio_dongle_first_ul_handle->total_number;
        if (count == 1) {
            /* error status */
            AUDIO_ASSERT(0);
        }
        for (i = 0; i < count; i++) {
            if (c_handle == handle) {
                /* find out the handle on the list */
                dongle_handle = handle;
                l_handle->next_ul_handle = c_handle->next_ul_handle;
            }

            if (dongle_handle == NULL) {
                c_handle->total_number -= 1;
            } else {
                c_handle->total_number -= 1;
                c_handle->index -= 1;
            }

            l_handle = c_handle;
            c_handle = c_handle->next_ul_handle;
        }
        if ((c_handle != NULL) || (dongle_handle == NULL)) {
            /* error status */
            AUDIO_ASSERT(0);
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    free(handle);
}

static ble_audio_dongle_ul_first_packet_status_t ble_audio_dongle_ul_first_packet_check(ble_audio_dongle_ul_handle_t *dongle_handle, uint32_t anchor)
{
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t anchor_safe;
    uint32_t anchor_last;
    UNUSED(dongle_handle);

    /* get current bt clock */
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

    anchor_safe = (anchor + dongle_handle->play_en_first_packet_safe_delay) & 0x0fffffff;
    anchor_last = (anchor + 0x10000000 - dongle_handle->bt_interval) & 0x0fffffff;

    if ((anchor_safe >= anchor) && (anchor > anchor_last)) {
        if ((bt_clk >= anchor_last) && (bt_clk < anchor_safe)) {
            /* --------- ........ --------- anchor_last --------- anchor ---------- bt_clk ---------- anchor_safe --------- ........ -------- */
            return BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY;
        } else if (bt_clk < anchor_last) {
            /* --------- bt_clk --------- anchor_last --------- anchor ---------- anchor_safe ---------- ........ --------- ........ -------- */
        } else {
            /* --------- ........ --------- anchor_last --------- anchor ---------- anchor_safe ---------- bt_clk --------- ........ -------- */
            return BLE_AUDIO_DONGLE_UL_FIRST_PACKET_TIMEOUT;
        }
    } else if ((anchor_safe >= anchor) && (anchor < anchor_last)) {
        if ((bt_clk >= anchor) && (bt_clk < anchor_safe)) {
            /* --------- ........ --------- anchor ---------- bt_clk ---------- anchor_safe --------- ........ --------- anchor_last -------- */
            return BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY;
        } else if (bt_clk >= anchor_last) {
            /* --------- ........ --------- anchor  ---------- anchor_safe --------- ........ --------- anchor_last ---------- bt_clk -------- */
            return BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY;
        } else if (bt_clk < anchor) {
            /* ---------- bt_clk --------- ........ --------- anchor  ---------- anchor_safe --------- ........ --------- anchor_last  -------- */
            return BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY;
        } else {
            /* --------- ........ --------- anchor  ---------- anchor_safe ---------- bt_clk --------- ........ --------- anchor_last -------- */
            return BLE_AUDIO_DONGLE_UL_FIRST_PACKET_TIMEOUT;
        }
    } else {
        if ((bt_clk >= anchor) || (bt_clk < anchor_safe)) {
            /* --------- anchor_safe --------- ........ ---------- ........ --------- anchor -------- bt_clk ---------- */
            return BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY;
        } else if ((bt_clk < anchor) && (bt_clk >= anchor_last)) {
            /* --------- anchor_safe --------- ........ ---------- ........ --------- anchor_last -------- bt_clk --------- anchor  ---------- */
            return BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY;
        } else {
            /* --------- anchor_safe --------- ........ -------- bt_clk ---------- ........ --------- anchor_last --------- anchor  ---------- */
            return BLE_AUDIO_DONGLE_UL_FIRST_PACKET_TIMEOUT;
        }
    }

    return BLE_AUDIO_DONGLE_UL_FIRST_PACKET_NOT_READY;
}

static void ble_audio_dongle_ul_get_stream_anchor(ble_audio_dongle_ul_handle_t *dongle_handle, uint32_t *anchor)
{
    bool first_flag = false;
    uint32_t i;
    ble_audio_dongle_ul_channel_info_t *current_channel_info;

    for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
        current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
        if (current_channel_info->enable) {
            if (first_flag == false) {
                *anchor = current_channel_info->channel_anchor;
                first_flag = true;
            } else {
                if ((*anchor < (0x10000000 - dongle_handle->bt_channel_anchor_diff)) && (*anchor >= dongle_handle->bt_channel_anchor_diff)) {
                    if ((current_channel_info->channel_anchor < *anchor) &&
                        (current_channel_info->channel_anchor >= (*anchor - dongle_handle->bt_channel_anchor_diff))) {
                        /* --------- first anchor-anchor_diff --------- current channel's anchor ---------- first anchor -------- */
                        *anchor = current_channel_info->channel_anchor;
                    } else if ((current_channel_info->channel_anchor < *anchor) &&
                               (current_channel_info->channel_anchor < (*anchor - dongle_handle->bt_channel_anchor_diff))) {
                        /* --------- current channel's anchor --------- first anchor-anchor_diff ---------- first anchor -------- */
                        // AUDIO_ASSERT(0);
                        BLE_LOG_E("[ble audio dongle][ul][get_stream_anchor]: diff error ch%d anchor 0x%x, stream anchor 0x%x, anchor diff 0x%x", 4, i, current_channel_info->channel_anchor, *anchor, dongle_handle->bt_channel_anchor_diff);
                        if (ble_audio_dongle_ul_first_packet_check(dongle_handle, *anchor) == BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY) {
                            /* do nothing */
                        } else if (ble_audio_dongle_ul_first_packet_check(dongle_handle, current_channel_info->channel_anchor) == BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY) {
                            *anchor = current_channel_info->channel_anchor;
                        }
                    } else if ((current_channel_info->channel_anchor >= *anchor) &&
                               (current_channel_info->channel_anchor < (*anchor + dongle_handle->bt_channel_anchor_diff))) {
                        /* ---------  first anchor ---------- current channel's anchor -------- first anchor+anchor_diff -------- */
                        /* do thing */
                    } else {
                        /* ---------  first anchor ---------- first anchor+anchor_diff -------- current channel's anchor -------- */
                        // AUDIO_ASSERT(0);
                        BLE_LOG_E("[ble audio dongle][ul][get_stream_anchor]: diff error ch%d anchor 0x%x, stream anchor 0x%x, anchor diff 0x%x", 4, i, current_channel_info->channel_anchor, *anchor, dongle_handle->bt_channel_anchor_diff);
                        if (ble_audio_dongle_ul_first_packet_check(dongle_handle, *anchor) == BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY) {
                            /* do nothing */
                        } else if (ble_audio_dongle_ul_first_packet_check(dongle_handle, current_channel_info->channel_anchor) == BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY) {
                            *anchor = current_channel_info->channel_anchor;
                        }
                    }
                } else if (*anchor >= (0x10000000 - dongle_handle->bt_channel_anchor_diff)) {
                    if ((current_channel_info->channel_anchor < *anchor) &&
                        (current_channel_info->channel_anchor >= (*anchor - dongle_handle->bt_channel_anchor_diff))) {
                        /* --------- first anchor-anchor_diff --------- current channel's anchor ---------- first anchor -------- */
                        *anchor = current_channel_info->channel_anchor;
                    } else if ((current_channel_info->channel_anchor < *anchor) &&
                               (current_channel_info->channel_anchor < (*anchor - dongle_handle->bt_channel_anchor_diff)) &&
                               (current_channel_info->channel_anchor > ((*anchor + dongle_handle->bt_channel_anchor_diff) & 0x0fffffff))) {
                        /* --------- first anchor+anchor_diff ---------- ... -------- current channel's anchor --------- first anchor-anchor_diff ---------- first anchor -------- */
                        // AUDIO_ASSERT(0);
                        BLE_LOG_E("[ble audio dongle][ul][get_stream_anchor]: diff error ch%d anchor 0x%x, stream anchor 0x%x, anchor diff 0x%x", 4, i, current_channel_info->channel_anchor, *anchor, dongle_handle->bt_channel_anchor_diff);
                        if (ble_audio_dongle_ul_first_packet_check(dongle_handle, *anchor) == BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY) {
                            /* do nothing */
                        } else if (ble_audio_dongle_ul_first_packet_check(dongle_handle, current_channel_info->channel_anchor) == BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY) {
                            *anchor = current_channel_info->channel_anchor;
                        }
                    } else if ((current_channel_info->channel_anchor < *anchor) &&
                               (current_channel_info->channel_anchor < (*anchor - dongle_handle->bt_channel_anchor_diff)) &&
                               (current_channel_info->channel_anchor <= ((*anchor + dongle_handle->bt_channel_anchor_diff) & 0x0fffffff))) {
                        /* -------- current channel's anchor --------- first anchor+anchor_diff ---------- ... --------- first anchor-anchor_diff ---------- first anchor -------- */
                        /* do thing */
                    } else if ((current_channel_info->channel_anchor >= *anchor)) {
                        /* -------- first anchor+anchor_diff ---------- ... ---------  first anchor ---------- current channel's anchor  -------- */
                        /* do thing */
                    }
                } else if (*anchor < dongle_handle->bt_channel_anchor_diff) {
                    if ((current_channel_info->channel_anchor < *anchor)) {
                        /* ---------  current channel's anchor ---------- first anchor ---------- ... -------- first anchor-anchor_diff --------- */
                        *anchor = current_channel_info->channel_anchor;
                    } else if ((current_channel_info->channel_anchor >= *anchor) &&
                               (current_channel_info->channel_anchor < (*anchor + dongle_handle->bt_channel_anchor_diff))) {
                        /* ---------  first anchor ---------- current channel's anchor -------- first anchor+anchor_diff -------- */
                        /* do thing */
                    } else if ((current_channel_info->channel_anchor >= *anchor) &&
                               (current_channel_info->channel_anchor >= ((*anchor + 0x10000000 - dongle_handle->bt_channel_anchor_diff) & 0x0fffffff))) {
                        /* ---------  first anchor ---------- ... -------- first anchor-anchor_diff ---------- current channel's anchor -------- */
                        *anchor = current_channel_info->channel_anchor;
                    } else {
                        /* ---------  first anchor ---------- ... -------- current channel's anchor ---------- first anchor-anchor_diff -------- */
                        // AUDIO_ASSERT(0);
                        BLE_LOG_E("[ble audio dongle][ul][get_stream_anchor]: diff error ch%d anchor 0x%x, stream anchor 0x%x, anchor diff 0x%x", 4, i, current_channel_info->channel_anchor, *anchor, dongle_handle->bt_channel_anchor_diff);
                        if (ble_audio_dongle_ul_first_packet_check(dongle_handle, *anchor) == BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY) {
                            /* do nothing */
                        } else if (ble_audio_dongle_ul_first_packet_check(dongle_handle, current_channel_info->channel_anchor) == BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY) {
                            *anchor = current_channel_info->channel_anchor;
                        }
                    }
                }
            }

            BLE_LOG_I("[ble audio dongle][ul][get_stream_anchor]: ch%d anchor 0x%x, stream anchor 0x%x", 3, i, current_channel_info->channel_anchor, *anchor);
        }
    }

    if (first_flag == false) {
        AUDIO_ASSERT(0);
    }
}

static bool ble_audio_dongle_ul_play_time_is_arrived(ble_audio_dongle_ul_handle_t *dongle_handle)
{
    uint32_t bt_clk;
    uint16_t bt_phase;

    /* get current bt clock */
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

    if (dongle_handle->play_en_overflow) {
        if ((dongle_handle->play_en_bt_clk <= bt_clk) && (dongle_handle->first_anchor_bt_clk <= dongle_handle->first_packet_bt_clk) && (dongle_handle->first_anchor_bt_clk > bt_clk)) {
            /* --------- play_en_bt_clk --------- ........ ---------- bt_clk ---------- ........ --------- first_anchor_bt_clk -------- first_packet_bt_clk -------- */
            return true;
        } else if ((dongle_handle->play_en_bt_clk <= bt_clk) && (dongle_handle->first_anchor_bt_clk > dongle_handle->first_packet_bt_clk) && (dongle_handle->first_packet_bt_clk > bt_clk)) {
            /* --------- play_en_bt_clk --------- ........ ---------- bt_clk ---------- ........ -------- first_packet_bt_clk --------- first_anchor_bt_clk  -------- */
            return true;
        }
    } else {
        if (dongle_handle->play_en_bt_clk <= bt_clk) {
            /* --------- ........ --------- first_anchor_bt_clk -------- first_packet_bt_clk ---------- play_en_bt_clk ---------- bt_clk --------- ........ -------- */
            /* --------- ........ --------- first_packet_bt_clk -------- first_anchor_bt_clk ---------- play_en_bt_clk ---------- bt_clk --------- ........ -------- */
            return true;
        } else if ((dongle_handle->play_en_bt_clk > bt_clk) && (dongle_handle->first_anchor_bt_clk > bt_clk) && (dongle_handle->first_packet_bt_clk > bt_clk)) {
            /* --------- bt_clk --------- ........ -------- first_packet_bt_clk ---------- first_anchor_bt_clk ---------- play_en_bt_clk --------- ........ -------- */
            return true;
        }
    }

    return false;
}

static bool ble_audio_dongle_ul_fetch_time_is_arrived(ble_audio_dongle_ul_handle_t *dongle_handle)
{
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t anchor_time, fetch_time, fetch_time_last;
    bool ret = false;

    /* get current bt clock */
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

    fetch_time_last = (dongle_handle->stream_anchor_previous) & 0x0fffffff;
    anchor_time = (dongle_handle->stream_anchor) & 0x0fffffff;
    fetch_time = (dongle_handle->stream_anchor + 0) & 0x0fffffff;

    if ((fetch_time_last < anchor_time) && (anchor_time <= fetch_time)) {
        if (bt_clk >= fetch_time) {
            /* --------- fetch_time_last --------- anchor_time --------- fetch_time ---------- bt_clk -------- */
            ret = true;
        } else if ((bt_clk < fetch_time) && (bt_clk >= fetch_time_last)) {
            /* --------- fetch_time_last --------- anchor_time ---------- bt_clk ---------- fetch_time -------- */
            ret = false;
        } else {
            /* ---------- bt_clk ---------- ... --------- fetch_time_last --------- anchor_time --------- fetch_time  -------- */
            ret = true;
        }
    } else if ((fetch_time_last < anchor_time) && (anchor_time > fetch_time)) {
        if ((bt_clk >= fetch_time) && (bt_clk < fetch_time_last)) {
            /* --------- fetch_time ---------- bt_clk ---------- ... --------- fetch_time_last --------- anchor_time -------- */
            ret = true;
        } else {
            /* --------- fetch_time ---------- ... --------- fetch_time_last --------- anchor_time ---------- bt_clk -------- */
            ret = false;
        }
    } else if ((fetch_time_last > anchor_time) && (anchor_time <= fetch_time)) {
        if ((bt_clk >= fetch_time) && (bt_clk < fetch_time_last)) {
            /* --------- anchor_time --------- fetch_time ---------- bt_clk ---------- ... --------- fetch_time_last -------- */
            ret = true;
        } else {
            /* -------- bt_clk --------- anchor_time --------- fetch_time ---------- ... --------- fetch_time_last  ---------- */
            ret = false;
        }
    } else {
        BLE_LOG_E("[ble audio dongle][ul]:error anchor 0x%x, 0x%x, 0x%x, 0x%x", 4, fetch_time_last, anchor_time, fetch_time, bt_clk);
        AUDIO_ASSERT(0);
    }

#if BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_LOG
    BLE_LOG_I("[ble audio dongle][ul]:fetch %d, 0x%x, 0x%x, 0x%x, 0x%x, %d", 6, ret, fetch_time_last, anchor_time, fetch_time, bt_clk, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_LOG */

    return ret;
}

static bool ble_audio_dongle_ul_fetch_time_is_timeout(ble_audio_dongle_ul_handle_t *dongle_handle)
{
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t anchor_time, anchor_time_last, fetch_time_safe;
    bool ret = false;

    /* get current bt clock */
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

    anchor_time_last = (dongle_handle->stream_anchor + 0x10000000 - dongle_handle->bt_interval) & 0x0fffffff;
    anchor_time = (dongle_handle->stream_anchor) & 0x0fffffff;
    fetch_time_safe = (dongle_handle->stream_anchor + dongle_handle->play_en_delay + dongle_handle->bt_retry_window) & 0x0fffffff;

    if ((fetch_time_safe > anchor_time) && (anchor_time > anchor_time_last)) {
        if ((bt_clk >= anchor_time_last) && (bt_clk < fetch_time_safe)) {
            /* --------- ........ --------- anchor_last --------- anchor ---------- bt_clk ---------- fetch_safe --------- ........ -------- */
            ret = false;
        } else {
            /* --------- ........ --------- anchor_last --------- anchor ---------- fetch_safe ---------- bt_clk --------- ........ -------- */
            ret = true;
        }
    } else if ((fetch_time_safe > anchor_time) && (anchor_time < anchor_time_last)) {
        if ((bt_clk >= anchor_time) && (bt_clk < fetch_time_safe)) {
            /* --------- ........ --------- anchor ---------- bt_clk ---------- anchor_safe --------- ........ --------- anchor_last -------- */
            ret = false;
        } else if (bt_clk >= anchor_time_last) {
            /* --------- ........ --------- anchor  ---------- anchor_safe --------- ........ --------- anchor_last ---------- bt_clk -------- */
            ret = false;
        } else if (bt_clk < anchor_time) {
            /* ---------- bt_clk --------- ........ --------- anchor  ---------- anchor_safe --------- ........ --------- anchor_last  -------- */
            ret = false;
        } else {
            /* --------- ........ --------- anchor  ---------- anchor_safe ---------- bt_clk --------- ........ --------- anchor_last -------- */
            ret = true;
        }
    } else {
        if ((bt_clk >= anchor_time) || (bt_clk < fetch_time_safe)) {
            /* --------- anchor_safe --------- ........ ---------- ........ --------- anchor -------- bt_clk ---------- */
            ret = false;
        } else if ((bt_clk < anchor_time) && (bt_clk >= anchor_time_last)) {
            /* --------- anchor_safe --------- ........ ---------- ........ --------- anchor_last -------- bt_clk --------- anchor  ---------- */
            ret = false;
        } else {
            /* --------- anchor_safe --------- ........ -------- bt_clk ---------- ........ --------- anchor_last --------- anchor  ---------- */
            ret = true;
        }
    }
#if BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_LOG
    BLE_LOG_I("[ble audio dongle][ul]:timeout %d, 0x%x, 0x%x, 0x%x, 0x%x, %d", 6, ret, anchor_time_last, anchor_time, fetch_time_safe, bt_clk, hal_nvic_query_exception_number());
#endif
    return ret;
}

static bool ble_audio_dongle_ul_fetch_timestamp_is_correct(ble_audio_dongle_ul_handle_t *dongle_handle, uint32_t channel_anchor, uint32_t packet_timestamp)
{
    bool ret = true;
    UNUSED(dongle_handle);

    if (((channel_anchor >= UL_BT_TIMESTAMP_DIFF) && (channel_anchor < (0x10000000 - UL_BT_TIMESTAMP_DIFF))) &&
        ((packet_timestamp < (channel_anchor - UL_BT_TIMESTAMP_DIFF)) || (packet_timestamp > (channel_anchor + UL_BT_TIMESTAMP_DIFF)))
       ) {
        /* --------- packet_timestamp --------- ........ channel_anchor-2 -------- channel_anchor --------- channel_anchor+2 ........ --------- packet_timestamp ---------- */
        ret = false;
    } else if (((channel_anchor < UL_BT_TIMESTAMP_DIFF)) &&
               ((packet_timestamp < (channel_anchor + 0x10000000 - UL_BT_TIMESTAMP_DIFF)) || (packet_timestamp > (channel_anchor + UL_BT_TIMESTAMP_DIFF)))
              ) {
        /* -------- channel_anchor --------- channel_anchor+2 ........ packet_timestamp --------- ........ --------- packet_timestamp ........ --------- channel_anchor-2 ---------- */
        ret = false;
    } else if (((channel_anchor >= (0x10000000 - UL_BT_TIMESTAMP_DIFF))) &&
               ((packet_timestamp < (channel_anchor - UL_BT_TIMESTAMP_DIFF)) || (packet_timestamp > (channel_anchor + UL_BT_TIMESTAMP_DIFF - 0x10000000)))
              ) {
        /*  --------- channel_anchor+2 ........ packet_timestamp --------- ........ --------- packet_timestamp ........ --------- channel_anchor-2 -------- channel_anchor ---------- */
        ret = false;
    }

    return ret;
}

/* Public functions ----------------------------------------------------------*/
/******************************************************************************/
/*            BLE audio source dongle music path Public Functions             */
/******************************************************************************/
ATTR_TEXT_IN_IRAM_LEVEL_1 void ble_audio_dongle_mixer_precallback(sw_mixer_member_t *member, void *para)
{
    UNUSED(member);
    UNUSED(para);

    xSemaphoreTake(g_ble_audio_dongle_dl_xSemaphore, portMAX_DELAY);
}

ATTR_TEXT_IN_IRAM_LEVEL_1 void ble_audio_dongle_mixer_postcallback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size)
{
    int32_t i;
    ble_audio_dongle_dl_handle_t *c_handle = NULL;
    SOURCE source = NULL;
    bool all_streams_in_mixer = true;

    UNUSED(member);
    UNUSED(para);

    xSemaphoreGive(g_ble_audio_dongle_dl_xSemaphore);

    /* change this handle data status */
    source = (SOURCE)(member->owner);
    SINK sink = source->transform->sink;
    c_handle = (ble_audio_dongle_dl_handle_t *)(sink->param.bt_common.scenario_param.dongle_handle);
    c_handle->data_status = BLE_AUDIO_SOURCE_STREAM_DATA_IN_MIXER;
    if (c_handle->mixer_status == BLE_AUDIO_SOURCE_STREAM_MIXER_UNMIX) {
        /* clear this stream's input buffer */
        stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);

        /* change this handle data status */
        c_handle->data_status = BLE_AUDIO_SOURCE_STREAM_DATA_EMPTY;

        /* this stream is unmixed now, so we can directly return here */
        return;
    } else if (source->param.data_dl.current_notification_index == 0) {
        /* clear this stream's input buffer */
        stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);

        /* change this handle data status */
        c_handle->data_status = BLE_AUDIO_SOURCE_STREAM_DATA_EMPTY;

        /* in here, it means it is the first packet, we need to drop the later sink flow */
        *out_frame_size = 0;

        return;
    }

    /* check if all mixed stream data is in this mixer */
    c_handle = ble_audio_dongle_first_dl_handle;
    for (i = 0; i < ble_audio_dongle_first_dl_handle->total_number; i++) {
        source = (SOURCE)(c_handle->owner);
        if ((c_handle->data_status == BLE_AUDIO_SOURCE_STREAM_DATA_IN_STREAM) &&
            (c_handle->mixer_status != BLE_AUDIO_SOURCE_STREAM_MIXER_UNMIX) &&
            (source->param.data_dl.current_notification_index != 0)) {
            /* this stream should be mixed, but its data is not in this mixer now.
               so we need to bypass this mix result in this time */
            all_streams_in_mixer = false;
            break;
        }

        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }

    /* check if the output data in this time includes all stream data */
    if ((*out_frame_size != 0) && (all_streams_in_mixer == true)) {
        /* all_streams_in_mixer is true, so all stream data is in this mixer.
           So we can clean all mixed streams' input buffers now and the mix result are sent to the sink */
        c_handle = ble_audio_dongle_first_dl_handle;
        for (i = 0; i < ble_audio_dongle_first_dl_handle->total_number; i++) {
            if (c_handle->mixer_status != BLE_AUDIO_SOURCE_STREAM_MIXER_UNMIX) {
                /* clear this stream's input buffer */
                stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);

                /* change this handle data status */
                c_handle->data_status = BLE_AUDIO_SOURCE_STREAM_DATA_EMPTY;
            }

            /* switch to the next dl stream */
            c_handle = c_handle->next_dl_handle;
        }
    } else {
        /* all_streams_in_mixer is false, so some stream data is not in this mixer.
           So we need to bypass this mix result */
        *out_frame_size = 0;
    }
}

ATTR_TEXT_IN_IRAM void ble_audio_dongle_dl_ccni_handler(hal_ccni_event_t event, void *msg)
{
    SOURCE source = NULL;
    uint32_t saved_mask = 0;
    uint32_t i, count, gpt_count, bt_count;
    hal_ccni_message_t *ccni_msg = msg;
    ble_audio_dongle_dl_handle_t *c_handle = NULL;
    UNUSED(event);

    if (ble_audio_dongle_first_dl_handle == NULL) {
        BLE_LOG_E("[ble audio dongle][dl]dl handle is NULL\r\n", 0);
        goto _ccni_return;
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    /* get timestamp for debug */
    bt_count = ccni_msg->ccni_message[0];
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    /* BT controller will send write index info on ccni msg */
    ble_audio_dl_info.share_buffer_blk_index = (uint16_t)(ccni_msg->ccni_message[1]);
    hal_nvic_restore_interrupt_mask(saved_mask);
    if (ble_audio_dl_info.share_buffer_blk_index >= ble_audio_dl_info.share_buffer_blk_num) {
        BLE_LOG_E("[ble audio dongle][dl] blk index is error, %u\r\n", 1, ble_audio_dl_info.share_buffer_blk_index);
        ble_audio_dl_info.share_buffer_blk_index = 0;
    }


#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][ccni callback entry]: %d, %u, %d", 3, ble_audio_dl_info.share_buffer_blk_index, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    c_handle = ble_audio_dongle_first_dl_handle;
    count = ble_audio_dongle_first_dl_handle->total_number;
    for (i = 0; i < count; i++) {
        /* get source */
        source = (SOURCE)c_handle->owner;
        if ((source == NULL) || (source->transform == NULL)) {
            BLE_LOG_E("[ble audio dongle][dl]source or transform is NULL\r\n", 0);
            /* switch to the next dl stream */
            c_handle = c_handle->next_dl_handle;
            continue;
        }
        switch (source->scenario_type) {
            case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
            case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                /* set timestamp for debug */
                c_handle->ccni_bt_count = bt_count;
                c_handle->ccni_gpt_count = gpt_count;
                /* set fetch flag to indicate that the next flow can fetch packets form buffer */
                if (source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.fetch_flag == 0) {
                    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.fetch_flag = 1;
                }
                hal_nvic_restore_interrupt_mask(saved_mask);
                /* Handler the stream */
                AudioCheckTransformHandle(source->transform);
                break;
#if defined(AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE)
            case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
            case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
                {
                    if ((c_handle->stream_status == BLE_AUDIO_DONGLE_DL_STREAM_START) || (c_handle->stream_status == BLE_AUDIO_DONGLE_DL_STREAM_RUNNING)) {
                        BUFFER_INFO *buffer_info = NULL;
                        AUDIO_PARAMETER *runtime = NULL;
                        uint32_t data_size;
                        uint32_t buffer_per_channel_shift;
                        /* stream is started */
                        /* set timestamp for debug */
                        /* increase fetch count */
                        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                        c_handle->ccni_bt_count = bt_count;
                        c_handle->ccni_gpt_count = gpt_count;
                        // c_handle->fetch_count += 1;
                        hal_nvic_restore_interrupt_mask(saved_mask);
                        /* afe in config */
                        buffer_info = &source->streamBuffer.BufferInfo;
                        runtime = &source->param.audio;
                        audio_dongle_afe_in_info_t   *afe_in = &((ble_audio_dongle_dl_handle_t *)c_handle)->source_info.afe_in;
                        /* update read & write pointer */
                        afe_in->pre_write_offset  = buffer_info->WriteOffset;
                        afe_in->pre_read_offset   = buffer_info->ReadOffset;
                        afe_in->afe_vul_cur       = AFE_GET_REG(afe_in->afe_vul_cur_addr);
                        afe_in->afe_vul_base      = AFE_GET_REG(afe_in->afe_vul_base_addr);
                        afe_in->cur_write_offset  = afe_in->afe_vul_cur - afe_in->afe_vul_base;
                        buffer_info->WriteOffset = afe_in->cur_write_offset;
                        /* check if the samples are enough */
                        buffer_per_channel_shift = ((source->param.audio.channel_num>=2) && (source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER )) ? 1 : 0;
                        data_size = (afe_in->cur_write_offset >= afe_in->pre_read_offset)
                                    ? (afe_in->cur_write_offset - afe_in->pre_read_offset)>>buffer_per_channel_shift
                                    : (source->streamBuffer.BufferInfo.length - afe_in->pre_read_offset + afe_in->cur_write_offset)>>buffer_per_channel_shift;
                        /* update afe buffer current size before process */
                        afe_in->afe_buffer_latency_size = data_size;
                        if (runtime->irq_exist == false) {
                            /* get the consuming time */
                            /* Jitter between BT CCNI IRQ and Audio HW, to avoid that data is less than a frame: 5ms * sample_rate * format */
                            uint32_t duration_cnt = 0;
                            uint32_t gpt_count_1 = 0;
                            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count_1);
                            hal_gpt_get_duration_count(gpt_count, gpt_count_1, &duration_cnt);
                            uint32_t jitter_size = ble_audio_dongle_dl_calculate_afe_in_jitter_bt_audio(c_handle, duration_cnt);

                            /* stream is played till the samples in AFE buffer is enough */
                            if (data_size >= (source->param.audio.frame_size + jitter_size)) {
                                if (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) {
                                    /* tracking mode should buffer some data to avoid irq period jitter */
                                    #ifndef AIR_I2S_SLAVE_ENABLE
                                    buffer_info->ReadOffset += ((data_size - (source->param.audio.frame_size + jitter_size)) << buffer_per_channel_shift);
                                    buffer_info->ReadOffset = (source->streamBuffer.BufferInfo.ReadOffset % (source->streamBuffer.BufferInfo.length));
                                    if (source->param.audio.format_bytes == 4) {
                                        buffer_info->ReadOffset = EIGHT_BYTE_ALIGNED(buffer_info->ReadOffset);
                                    } else {
                                        buffer_info->ReadOffset = FOUR_BYTE_ALIGNED(buffer_info->ReadOffset);
                                    }
                                    afe_in->pre_read_offset = buffer_info->ReadOffset;
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
                                    afe_in->pre_read_offset = buffer_info->ReadOffset;
                                }

                                /* samples are enough, so update data status */
                                c_handle->data_status = BLE_AUDIO_SOURCE_STREAM_DATA_IN_STREAM;

                                /* update flag */
                                runtime->irq_exist = true;

                                BLE_LOG_I("[ble audio dongle][dl][afe in] scenario type %d, first handle afe buffer ro[%d] wo[%d] %d", 4,
                                    source->scenario_type,
                                    buffer_info->ReadOffset,
                                    buffer_info->WriteOffset,
                                    jitter_size
                                    );
                                /* update this status to trigger sw mixer handle */
                                //c_handle->first_packet_status = ULL_AUDIO_V2_DONGLE_DL_FIRST_PACKET_READY;

                                /* Handler the stream */
                                AudioCheckTransformHandle(source->transform);
                            }
                            if (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) {
                                uint8_t  src_id = source->param.audio.AfeBlkControl.u4asrcid;
                                uint32_t src_tracking_rate = AFE_READ(ASM_FREQUENCY_2 + 0x100 * src_id) * 100 / 4096;
                                if ((buffer_info->WriteOffset & (~15)) == (buffer_info->ReadOffset & (~15))) {
                                    BLE_LOG_E("[ble audio dongle][dl][afe in] hwsrc start fail owo[%d] oro[%d] src_%d rate %d", 4,
                                        buffer_info->WriteOffset,
                                        buffer_info->ReadOffset,
                                        src_id,
                                        src_tracking_rate
                                        );
                                }
                            }
                        } else {
                            if (data_size >= source->param.audio.frame_size) {
                                /* samples are enough, so update data status */
                                c_handle->data_status = BLE_AUDIO_SOURCE_STREAM_DATA_IN_STREAM;
                            } else {
                                BLE_LOG_E("[ble audio dongle][dl][afe in] scenario type %d, size %d < frame size %d ccni_wo[%d] cur_wo[%d]", 5,
                                    source->scenario_type,
                                    data_size,
                                    source->param.audio.frame_size,
                                    afe_in->cur_write_offset,
                                    AFE_GET_REG(afe_in->afe_vul_cur_addr) - AFE_GET_REG(afe_in->afe_vul_base_addr)
                                    );
                            }
                            /* Handler the stream */
                            AudioCheckTransformHandle(source->transform);
                        }
                        #ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
                        /* vdma tracking mode */
                        if ((source->param.audio.AfeBlkControl.u4asrcflag) && (!g_i2s_slv_common_tracking_start_flag)) {
                            g_i2s_slv_common_tracking_start_flag = true;
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
                                BLE_LOG_E("[ble audio dongle][dl][afe in] start i2s slave audio_interface = %d fail", 1, source->param.audio.audio_interface);
                                AUDIO_ASSERT(0);
                            }
                            dma_channel = g_i2s_slave_vdma_channel_infra[port_tmp * 2 + 1];
                            buffer_info->ReadOffset = buffer_info->WriteOffset; // clear the buffer
                            AFE_WRITE(ASM_CH01_OBUF_RDPNT, AFE_GET_REG(ASM_CH01_OBUF_WRPNT));
                            i2s_vdma_status = vdma_start(dma_channel);
                            if (i2s_vdma_status != VDMA_OK) {
                                BLE_LOG_E("[ble audio dongle][dl][afe in] DSP UL start i2s slave set vdma_start fail %d", 1, i2s_vdma_status);
                                AUDIO_ASSERT(0);
                            }
                            BLE_LOG_I("[ble audio dongle][dl][afe in] VDMA Start", 0);
                        }
                        /* interconn tracking mode */
                        if ((source->param.audio.mem_handle.pure_agent_with_src) && (!g_i2s_slv_common_tracking_start_flag)) {
                            // g_i2s_slv_common_tracking_start_flag = true;
                            // /* start gpt timer to trigger vul1 irq */
                            // hal_gpt_status_t gpt_status = HAL_GPT_STATUS_OK;
                            // if (gpt_i2s_slv_trigger_handle == 0) {
                            //     gpt_status = hal_gpt_sw_get_timer(&gpt_i2s_slv_trigger_handle);
                            //     if (gpt_status != HAL_GPT_STATUS_OK) {
                            //         BLE_LOG_E("[ble audio dongle][dl][afe in] get gpt handle fail %d", 1, gpt_status);
                            //         AUDIO_ASSERT(0);
                            //     }
                            // }
                            //gpt_status = hal_gpt_sw_start_timer_us(gpt_i2s_slv_trigger_handle, 2500, (hal_gpt_callback_t)ull_audio_v2_dongle_dl_i2s_slv_gpt_cb, c_handle);
                        }
                        #endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
                    }
                }
                break;
#endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE*/
            default:
                break;
        }

        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }
    if (c_handle != NULL) {
        AUDIO_ASSERT(0);
    }

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][ccni callback exit]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

_ccni_return:
#if BLE_AUDIO_MUSIC_DONGLE_DEBUG_LANTENCY
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&ble_audio_music_ccni_gpt_count1);
#endif /* BLE_AUDIO_MUSIC_DONGLE_DEBUG_LANTENCY */

    return;
}

static void ble_audio_dongle_dl_usb_in_init(ble_audio_dongle_dl_handle_t *dongle_handle, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    SOURCE              source             = dongle_handle->source;
    uint32_t            bt_sample_byte     = audio_dongle_get_format_bytes(dongle_handle->sink_info.bt_out.sample_format);
    audio_codec_pcm_t   *pcm               = NULL;
    UNUSED(bt_common_open_param);
    pcm = &(audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm);
    dongle_handle->usb_frame_size       = usb_audio_get_frame_size(&audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_type, &audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param);
    dongle_handle->usb_channel_num      = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.channel_mode;
    dongle_handle->usb_frame_samples    = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000;
    dongle_handle->usb_min_start_frames = ble_audio_dl_info.lc3_packet_frame_interval / 1000 + 1;
    dongle_handle->usb_sample_format    = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.format;
    BLE_LOG_I("[ble audio dongle][dl][config][USB][scenario %d-%d][handle 0x%x] %u, %u, %u, %u, %u\r\n", 8,
                 audio_transmitter_open_param->scenario_type,
                 audio_transmitter_open_param->scenario_sub_id,
                 dongle_handle,
                 dongle_handle->usb_frame_size,
                 dongle_handle->usb_channel_num,
                 dongle_handle->usb_frame_samples,
                 dongle_handle->usb_min_start_frames,
                 dongle_handle->usb_sample_format);

    dongle_handle->src_out_frame_samples = dongle_handle->sink_info.bt_out.sample_rate / 1000;
    dongle_handle->src_out_frame_size = dongle_handle->src_out_frame_samples * bt_sample_byte;

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    sw_clk_skew_config_t sw_clk_skew_config = {0};
    /* clock skew config */
    dongle_handle->clk_skew_port              = stream_function_sw_clk_skew_get_port(source);
    sw_clk_skew_config.channel                = 2;
    sw_clk_skew_config.bits                   = bt_sample_byte * 8;
    sw_clk_skew_config.order                  = C_Flp_Ord_5;
    sw_clk_skew_config.skew_io_mode           = C_Skew_Inp;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_1_FRAME;
    sw_clk_skew_config.skew_work_mode         = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size        = (dongle_handle->sink_info.bt_out.sample_rate / 1000 * ble_audio_dl_info.lc3_packet_frame_interval / 1000 + 1) * bt_sample_byte;  //48K * (10ms + 1sample)
    sw_clk_skew_config.continuous_frame_size  = dongle_handle->sink_info.bt_out.sample_rate / 1000 * ble_audio_dl_info.lc3_packet_frame_interval / 1000 * bt_sample_byte;
    stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
    BLE_LOG_I("[ble audio dongle][dl][config][SW_CLK_SKEW][scenario %d-%d][handle 0x%x] %u, %u, %u, %u, %u, %u, %u, %u\r\n", 11,
                 audio_transmitter_open_param->scenario_type,
                 audio_transmitter_open_param->scenario_sub_id,
                 dongle_handle,
                 sw_clk_skew_config.channel,
                 sw_clk_skew_config.bits,
                 sw_clk_skew_config.order,
                 sw_clk_skew_config.skew_io_mode,
                 sw_clk_skew_config.skew_compensation_mode,
                 sw_clk_skew_config.skew_work_mode,
                 sw_clk_skew_config.max_output_size,
                 sw_clk_skew_config.continuous_frame_size);
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    sw_buffer_config_t buffer_config = {0};
    buffer_config.mode = SW_BUFFER_MODE_DROP_DATA_WHEN_BUFFER_FULL;
    buffer_config.total_channels     = 2;
    buffer_config.watermark_max_size = 4 * dongle_handle->sink_info.bt_out.sample_rate / 1000 * ble_audio_dl_info.lc3_packet_frame_interval / 1000 * bt_sample_byte;  //48K * 40ms
    buffer_config.output_size        = dongle_handle->sink_info.bt_out.sample_rate / 1000 * ble_audio_dl_info.lc3_packet_frame_interval / 1000 * bt_sample_byte;      //48K * 10ms
    buffer_config.watermark_min_size = 0;
    dongle_handle->buffer_port       = stream_function_sw_buffer_get_port(source);
    stream_function_sw_buffer_init(dongle_handle->buffer_port, &buffer_config);
    BLE_LOG_I("[ble audio dongle][dl][config][SW_BUFFER][scenario %d-%d][handle 0x%x] %u, %u, %u, %u, %u\r\n", 8,
                 audio_transmitter_open_param->scenario_type,
                 audio_transmitter_open_param->scenario_sub_id,
                 dongle_handle,
                 buffer_config.mode,
                 buffer_config.total_channels,
                 buffer_config.watermark_max_size,
                 buffer_config.watermark_min_size,
                 buffer_config.output_size);
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */
}

void ble_audio_dongle_dl_usb_in_deinit(ble_audio_dongle_dl_handle_t *dongle_handle)
{
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    stream_function_sw_buffer_deinit(dongle_handle->buffer_port);
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */
}

void ble_audio_dongle_dl_init(SOURCE source, SINK sink, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    uint32_t            bt_sample_real_byte = 0;
    uint32_t            sample_size         = 0;
    stream_resolution_t bt_sample_res       = RESOLUTION_16BIT;
    /* get handle for application config */
    ble_audio_dongle_dl_handle_t *dongle_handle                                     = ble_audio_dongle_dl_get_handle(source);
                                 sink->param.bt_common.scenario_param.dongle_handle = (void *)dongle_handle;
                                 dongle_handle->source                              = source;
                                 dongle_handle->sink                                = sink;
                                 source->taskId                                     = DHP_TASK_ID;
                                 sink->taskid                                       = DHP_TASK_ID;
    hal_audio_format_t           process_res                                        = HAL_AUDIO_PCM_FORMAT_S16_LE;
    /* init audio info */
    if (dongle_handle->total_number == 1) { // first dl stream init
#ifdef HAL_SLEEP_MANAGER_ENABLED
        /* lock sleep because sleep wake-up time will consume the stream process time */
        hal_sleep_manager_lock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif
        /* register ccni handler */
        audio_transmitter_register_isr_handler(0, ble_audio_dongle_dl_ccni_handler);
        memset(&ble_audio_dl_info, 0, sizeof(LE_AUDIO_INFO));
        ble_audio_dongle_dl_audio_info_init(bt_common_open_param, sink, &bt_common_open_param->scenario_param.ble_audio_dongle_param.share_buffer_channel_1, bt_common_open_param->scenario_param.ble_audio_dongle_param.channel_enable);
    }

    dongle_handle->sink_info.bt_out.codec_type = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_type;
    if (bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_type == AUDIO_DSP_CODEC_TYPE_LC3) {
        dongle_handle->sink_info.bt_out.sample_format  = HAL_AUDIO_PCM_FORMAT_S16_LE;
        dongle_handle->sink_info.bt_out.sample_rate    = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.sample_rate;
        dongle_handle->sink_info.bt_out.frame_size     = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_size;
        dongle_handle->sink_info.bt_out.frame_interval = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval;
        dongle_handle->sink_info.bt_out.bit_rate       = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.bit_rate;
    } else {
        dongle_handle->sink_info.bt_out.sample_rate    = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.sample_rate;
        dongle_handle->sink_info.bt_out.sample_format  = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.sample_format;
        dongle_handle->sink_info.bt_out.frame_size     = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.frame_size;
        dongle_handle->sink_info.bt_out.frame_interval = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.frame_interval;
        dongle_handle->sink_info.bt_out.bit_rate       = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus.bit_rate;
    }
    process_res   = dongle_handle->sink_info.bt_out.sample_format;
    sample_size   = audio_dongle_get_format_bytes(process_res);
    bt_sample_res = dongle_handle->sink_info.bt_out.sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE ? RESOLUTION_16BIT : RESOLUTION_32BIT;
    if (bt_sample_res == RESOLUTION_32BIT) {
        bt_sample_real_byte = 3;
    } else {
        bt_sample_real_byte = 2;
    }
    /* common flow */
    switch (source->scenario_type) {
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
            ble_audio_dongle_dl_usb_in_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            break;

        #ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
            case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
                ble_audio_dongle_dl_afe_in_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
                break;
        #endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */

        #ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
            case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
                ble_audio_dongle_dl_afe_in_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
                break;
        #endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */

        default:
            BLE_LOG_E("[ble audio dongle][dl] ERROR: init scenario type is not suitable %d", 1, source->scenario_type);
            break;
    }
    /* common flow */
    /* unified fs convertor */
    audio_dl_unified_fs_convertor_param_t fs_param = {0};
    uint32_t in_fs  = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate;
    uint32_t out_fs = dongle_handle->sink_info.bt_out.sample_rate;
    if ((source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) ||
        (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)) {
        in_fs = 48000; // Use src out_rate
    }
    // get param from dongle
    fs_param.in_rate    = in_fs;
    fs_param.out_rate   = out_fs;
    fs_param.period_ms  = dongle_handle->sink_info.bt_out.frame_interval;
    fs_param.in_ch_num  = 2;
    fs_param.pcm_format = process_res;
    fs_param.source     = dongle_handle->source;
    fs_param.codec_type = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_type;
    if ((source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) ||
        (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1)) {
        fs_param.process_max_size = (dongle_handle->usb_min_start_frames + 1) * fs_param.in_rate * sample_size / 1000;
    } else {
        fs_param.process_max_size = fs_param.in_rate * fs_param.period_ms * sample_size / 1000 / 1000;
    }
    audio_dl_unified_fs_convertor_init(&fs_param);
    // set param to dongle
    dongle_handle->process_sample_rate_max = fs_param.process_sample_rate_max;
    dongle_handle->src0_port = (src_fixed_ratio_port_t *) fs_param.src0_port;
    dongle_handle->src1_port = (src_fixed_ratio_port_t *) fs_param.src1_port;
    dongle_handle->src_port  = fs_param.src2_port;
    /* sw gain */
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    /* sw gain config */
    int32_t default_gain;
    sw_gain_config_t default_config = {0};
    default_config.resolution            = bt_sample_res;
    default_config.target_gain           = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.gain_default_L;
    default_config.up_step               = 1;
    default_config.up_samples_per_step   = 2;
    default_config.down_step             = -1;
    default_config.down_samples_per_step = 2;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(source);
    if ((source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) ||
        (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)) {
        default_config.target_gain = -12000; // -120dB
    }
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.gain_default_L;
    if ((source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) ||
        (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)) {
        default_gain = -12000; // -120dB
    }
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    default_gain = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.gain_default_R;
    if ((source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) ||
        (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)) {
        default_gain = -12000; // -120dB
    }
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    BLE_LOG_I("[ble audio dongle][dl][config][SW_GAIN][scenario %d-%d][handle 0x%x] %d, %d\r\n", 5,
                 audio_transmitter_open_param->scenario_type,
                 audio_transmitter_open_param->scenario_sub_id,
                 dongle_handle,
                 audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.gain_default_L,
                 audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.gain_default_R);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
    /* sw mixer */
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    /* sw mixer config */
    sw_mixer_callback_config_t       callback_config = {0};
    sw_mixer_input_channel_config_t  in_ch_config    = {0};
    sw_mixer_output_channel_config_t out_ch_config   = {0};
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    extern void ble_audio_dongle_mixer_precallback(sw_mixer_member_t *member, void *para);
    callback_config.preprocess_callback = ble_audio_dongle_mixer_precallback;
    extern void ble_audio_dongle_mixer_postcallback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size);
    callback_config.postprocess_callback = ble_audio_dongle_mixer_postcallback;
    in_ch_config.total_channel_number    = 2;
    in_ch_config.input_mode              = SW_MIXER_CHANNEL_MODE_FORCED_ENOUGH;
    in_ch_config.resolution              = bt_sample_res;
    in_ch_config.buffer_size             = dongle_handle->sink_info.bt_out.sample_rate / 1000 * dongle_handle->sink_info.bt_out.frame_interval / 1000 * sample_size;
    out_ch_config.total_channel_number   = 2;
    out_ch_config.resolution             = bt_sample_res;
    dongle_handle->mixer_member = stream_function_sw_mixer_member_create((void *)source,
                                                                         SW_MIXER_MEMBER_MODE_NO_BYPASS,
                                                                         &callback_config,
                                                                         &in_ch_config,
                                                                         &out_ch_config);

    if (dongle_handle->index == 1) {
        /* it is the first dl stream */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, true);
    } else {
        /* it is not the first dl stream, so we need to disconnect default connection */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, false);
    }
    BLE_LOG_I("[ble audio dongle][dl][config][SW_MIXER][scenario %d-%d][handle 0x%x] %u, %u, %u, %u, %u, %u\r\n", 9,
                 audio_transmitter_open_param->scenario_type,
                 audio_transmitter_open_param->scenario_sub_id,
                 dongle_handle,
                 in_ch_config.total_channel_number,
                 in_ch_config.resolution,
                 in_ch_config.input_mode,
                 in_ch_config.buffer_size,
                 out_ch_config.total_channel_number,
                 out_ch_config.resolution);
#endif /* AIR_SOFTWARE_MIXER_ENABLE */
#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
    if (bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_type == AUDIO_DSP_CODEC_TYPE_LC3) {
        /* init lc3 codec */
        lc3_enc_branch_config_t lc3_enc_config = {0};
        lc3_enc_config.sample_bits      = sample_size * 8;
        lc3_enc_config.channel_no       = 2;
        lc3_enc_config.sample_rate      = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.sample_rate;
        lc3_enc_config.bit_rate         = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.bit_rate;
        lc3_enc_config.frame_interval   = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval;
        lc3_enc_config.delay            = LC3_DELAY_COMPENSATION_IN_DECODER;
        lc3_enc_config.buffer_size      = 48000 / 1000 * ble_audio_dl_info.lc3_packet_frame_interval / 1000 * sample_size; //10ms*48K*16bit
        stream_codec_encoder_lc3_branch_init(LC3_ENC_BRANCH_INDEX_0, sink, &lc3_enc_config);
        BLE_LOG_I("[ble audio dongle][dl][config][LC3 ENC][scenario %d-%d][handle 0x%x] %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                     audio_transmitter_open_param->scenario_type,
                     audio_transmitter_open_param->scenario_sub_id,
                     dongle_handle,
                     lc3_enc_config.sample_bits,
                     lc3_enc_config.channel_no,
                     lc3_enc_config.sample_rate,
                     lc3_enc_config.bit_rate,
                     lc3_enc_config.frame_interval,
                     lc3_enc_config.delay,
                     lc3_enc_config.buffer_size);
    }
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
    else if (bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS) {
        /* init lc3plus codec */
        lc3plus_enc_config_t lc3plus_enc_config = {0};
        audio_codec_lc3plus_t *p_lc3plus;
        p_lc3plus = &(bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3plus);
        lc3plus_enc_config.sample_bits       = bt_sample_real_byte * 8;
        lc3plus_enc_config.channel_num       = 2;
        lc3plus_enc_config.sample_rate       = p_lc3plus->sample_rate;
        lc3plus_enc_config.bit_rate          = p_lc3plus->bit_rate;
        lc3plus_enc_config.frame_interval    = p_lc3plus->frame_interval;
        lc3plus_enc_config.frame_samples     = (p_lc3plus->sample_rate / 1000) * (p_lc3plus->frame_interval / 1000);
        lc3plus_enc_config.in_frame_size     = lc3plus_enc_config.frame_samples * sample_size;
        lc3plus_enc_config.out_frame_size    = (p_lc3plus->bit_rate * p_lc3plus->frame_interval) / (8 * 1000 * 1000);
        lc3plus_enc_config.process_frame_num = 1;
        lc3plus_enc_config.codec_mode        = LC3PLUS_ARCH_FX;
        lc3plus_enc_config.channel_mode      = LC3PLUS_ENC_CHANNEL_MODE_MULTI;
        stream_codec_encoder_lc3plus_init(LC3PLUS_ENC_PORT_0, sink, &lc3plus_enc_config);
        BLE_LOG_I("[ble audio dongle][dl][config][LC3PLUS ENC][scenario %d-%d][handle 0x%x] %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", 12,
                    audio_transmitter_open_param->scenario_type,
                    audio_transmitter_open_param->scenario_sub_id,
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
    }
#endif /* AIR_BT_LE_LC3PLUS_ENABLE */
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */

#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (bt_common_open_param->scenario_param.ble_audio_dongle_param.without_bt_link_mode_enable == 0)
    {
        if (ble_audio_dongle_dl_without_bt_link_mode_enable == true)
        {
            AUDIO_ASSERT(0);
        }
    }
    else
    {
        /* start gpt timer that act as bt link to trigger dsp stream */
        ble_audio_dongle_dl_software_timer_start(dongle_handle);
        ble_audio_dongle_dl_without_bt_link_mode_enable = true;
    }

    /* silence detection init */
    ble_dongle_silence_detection_init(dongle_handle, AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0+source->param.data_dl.scenario_sub_id-AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0);
#endif /* AIR_SILENCE_DETECTION_ENABLE */

    if (g_ble_audio_dongle_dl_xSemaphore == NULL) {
        g_ble_audio_dongle_dl_xSemaphore = xSemaphoreCreateMutex();
        AUDIO_ASSERT(g_ble_audio_dongle_dl_xSemaphore != NULL);
        xSemaphoreGive(g_ble_audio_dongle_dl_xSemaphore);
    }

    dongle_handle->stream_status = BLE_AUDIO_DONGLE_DL_STREAM_INIT;
}

void ble_audio_dongle_dl_deinit(SOURCE source, SINK sink)
{
    audio_dsp_codec_type_t codec_type;
    // uint32_t saved_mask = 0;
    ble_audio_dongle_dl_handle_t *dongle_handle = (ble_audio_dongle_dl_handle_t *)(sink->param.bt_common.scenario_param.dongle_handle);

    codec_type = dongle_handle->sink_info.bt_out.codec_type;

    /* application deinit */
#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (ble_audio_dongle_dl_without_bt_link_mode_enable == true)
    {
        /* stop gpt timer that act as bt link to trigger dsp stream */
        ble_audio_dongle_dl_software_timer_stop(dongle_handle);
        if (dongle_handle->total_number == 1)
        {
            ble_audio_dongle_dl_without_bt_link_mode_enable = false;
        }
    }

    /* silence detection deinit */
    ble_dongle_silence_detection_deinit(dongle_handle, AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0+source->param.data_dl.scenario_sub_id-AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0);
#endif /* AIR_SILENCE_DETECTION_ENABLE */

    // hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    /* unregister ccni handler */
    if (dongle_handle->total_number == 1) {
        audio_transmitter_unregister_isr_handler(0, ble_audio_dongle_dl_ccni_handler);
#ifdef HAL_SLEEP_MANAGER_ENABLED
        /* unlock sleep */
        hal_sleep_manager_unlock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif
    }
    // hal_nvic_restore_interrupt_mask(saved_mask);
    /* common flow */
    /* unified fs convertor */
    audio_dl_unified_fs_convertor_param_t fs_param = {0};
    fs_param.src0_port  = dongle_handle->src0_port;
    fs_param.src1_port  = dongle_handle->src1_port;
    fs_param.src2_port  = dongle_handle->src_port;
    fs_param.pcm_format = dongle_handle->sink_info.bt_out.sample_format;
    audio_dl_unified_fs_convertor_deinit(&fs_param);
    /* sw gain */
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    stream_function_sw_gain_deinit(dongle_handle->gain_port);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */

    /* sw mixer */
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    if (dongle_handle->mixer_status == BLE_AUDIO_SOURCE_STREAM_MIXER_DISCON) {
        /* do disconnections */
        stream_function_sw_mixer_channel_disconnect_all(dongle_handle->mixer_member);
        dongle_handle->mixer_status = BLE_AUDIO_SOURCE_STREAM_MIXER_UNMIX;
        BLE_LOG_I("[ble audio dongle][dl] unmix type %d", 1, dongle_handle->source->scenario_type);
    }
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
#endif /* AIR_SOFTWARE_MIXER_ENABLE */

    switch (source->scenario_type) {
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
            ble_audio_dongle_dl_usb_in_deinit(dongle_handle);
            break;

        #ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
            case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
                ble_audio_dongle_dl_afe_in_deinit(dongle_handle);
                break;
        #endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */

        #ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
            case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
                ble_audio_dongle_dl_afe_in_deinit(dongle_handle);
                break;
        #endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */

        default:
            BLE_LOG_E("[ble audio dongle][dl] ERROR: deinit scenario type is not suitable %d", 1, source->scenario_type);
            break;
    }
    /* release handle */
    ble_audio_dongle_dl_release_handle(dongle_handle);
    /* deinit codec */
#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
    if (codec_type == AUDIO_DSP_CODEC_TYPE_LC3) {
        stream_codec_encoder_lc3_branch_deinit(LC3_ENC_BRANCH_INDEX_0, sink);
    }
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
    else if (codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS) {
        stream_codec_encoder_lc3plus_deinit(LC3PLUS_ENC_PORT_0, sink);
    }
#endif
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */

    dongle_handle->stream_status = BLE_AUDIO_DONGLE_DL_STREAM_DEINIT;
}


ATTR_TEXT_IN_IRAM_LEVEL_1 ble_audio_dongle_dl_handle_t *ble_audio_dongle_query_dl_handle(void *owner)
{
    uint32_t saved_mask, i, count;
    ble_audio_dongle_dl_handle_t *dongle_handle = NULL;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    dongle_handle = ble_audio_dongle_first_dl_handle;
    if (dongle_handle != NULL)
    {
        count = dongle_handle->total_number;
        for (i=0; i < count; i++)
        {
            /* check owner */
            if (dongle_handle->owner == owner)
            {
                break;
            }
            else
            {
                dongle_handle = dongle_handle->next_dl_handle;
            }
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    return dongle_handle;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ bool ble_audio_dongle_dl_config(SOURCE source, stream_config_type type, U32 value)
{
    // uint32_t saved_mask = 0;
    extern CONNECTION_IF * port_audio_transmitter_get_connection_if(audio_transmitter_scenario_type_t scenario_id, audio_transmitter_scenario_sub_id_t sub_id);
    ble_audio_dongle_runtime_config_param_p config_param = (ble_audio_dongle_runtime_config_param_p)value;
    SINK sink = source->transform->sink;
    ble_audio_dongle_dl_handle_t *dongle_handle = (ble_audio_dongle_dl_handle_t *)(sink->param.bt_common.scenario_param.dongle_handle);
    int32_t new_gain;
    sw_gain_config_t old_config;
    audio_transmitter_scenario_type_t scenario_id_0;
    audio_transmitter_scenario_sub_id_t sub_id_0;
    audio_transmitter_scenario_type_t scenario_id_1;
    audio_transmitter_scenario_sub_id_t sub_id_1;
    CONNECTION_IF *application_ptr_0 = NULL;
    CONNECTION_IF *application_ptr_1 = NULL;
    ble_audio_dongle_dl_handle_t *dongle_handle_0;
    ble_audio_dongle_dl_handle_t *dongle_handle_1;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    audio_scenario_type_t audio_scenario_type;
#endif /* AIR_SILENCE_DETECTION_ENABLE */
    UNUSED(dongle_handle);
    // UNUSED(saved_mask);
    UNUSED(type);
    UNUSED(value);

    switch (config_param->config_operation) {
        case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_L:
            /* left channel */
            new_gain = config_param->config_param.vol_level.gain_1;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 1, &old_config);
            BLE_LOG_I("[ble audio dongle][dl][config]scenario %d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 4,
                         source->scenario_type,
                         1,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, new_gain);
            break;

        case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_R:
            new_gain = config_param->config_param.vol_level.gain_2;
            /* right channel */
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 2, &old_config);
            BLE_LOG_I("[ble audio dongle][dl][config]scenario %d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 4,
                         source->scenario_type,
                         2,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, new_gain);
            break;

        case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL:
            /* left channel */
            new_gain = config_param->config_param.vol_level.gain_1;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 1, &old_config);
            BLE_LOG_I("[ble audio dongle][dl][config]scenario %d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 4,
                         source->scenario_type,
                         1,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, new_gain);

            /* right channel */
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 2, &old_config);
            new_gain = config_param->config_param.vol_level.gain_2;
            BLE_LOG_I("[ble audio dongle][dl][config]scenario %d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 4,
                         source->scenario_type,
                         2,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, new_gain);
            break;

        case BLE_AUDIO_DONGLE_CONFIG_OP_MUSIC_MIX:
            /* mix stream */
            scenario_id_0 = source->param.data_dl.scenario_type;
            sub_id_0.ble_audio_dongle_id = source->param.data_dl.scenario_sub_id;
            scenario_id_1 = config_param->config_param.dl_mixer_param.scenario_id;
            sub_id_1.ble_audio_dongle_id = config_param->config_param.dl_mixer_param.sub_id;
            if (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) {
                sub_id_0.ble_audio_dongle_id = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN;
                scenario_id_0 = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE;
            } else if (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) {
                sub_id_0.ble_audio_dongle_id = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN;
                scenario_id_0 = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE;
            }
            application_ptr_0 = port_audio_transmitter_get_connection_if(scenario_id_0, sub_id_0);
            application_ptr_1 = port_audio_transmitter_get_connection_if(scenario_id_1, sub_id_1);
            if ((application_ptr_0 == NULL) || (application_ptr_1 == NULL)) {
                BLE_LOG_E("[audio transmitter][config] fail %d %d, id 0 %d_%d id 1 %d_%d", 6,
                                application_ptr_0,
                                application_ptr_1,
                                scenario_id_0,
                                sub_id_0,
                                scenario_id_1,
                                sub_id_1
                                );
                AUDIO_ASSERT(0);
            }
            /* get application handle */
            if ((application_ptr_0->source->scenario_type != AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) &&
                (application_ptr_0->source->scenario_type != AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)) {
                dongle_handle_0 = (ble_audio_dongle_dl_handle_t *)(application_ptr_0->sink->param.bt_common.scenario_param.dongle_handle);
            } else {
                dongle_handle_0 = ble_audio_dongle_query_dl_handle(application_ptr_0->source);
            }
            if ((application_ptr_1->source->scenario_type != AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) &&
                (application_ptr_1->source->scenario_type != AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)) {
                dongle_handle_1 = (ble_audio_dongle_dl_handle_t *)(application_ptr_1->sink->param.bt_common.scenario_param.dongle_handle);
            } else {
                dongle_handle_1 = ble_audio_dongle_query_dl_handle(application_ptr_1->source);
            }

            xSemaphoreTake(g_ble_audio_dongle_dl_xSemaphore, portMAX_DELAY);

            /* do mix connections */
            if (dongle_handle_0 == dongle_handle_1) {
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_0->mixer_member, 1);
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_0->mixer_member, 2);
            } else {
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_0->mixer_member, 1);
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_0->mixer_member, 2);
                stream_function_sw_mixer_channel_connect(dongle_handle_1->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle_0->mixer_member, 1);
                stream_function_sw_mixer_channel_connect(dongle_handle_1->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle_0->mixer_member, 2);

                stream_function_sw_mixer_channel_connect(dongle_handle_1->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_1->mixer_member, 1);
                stream_function_sw_mixer_channel_connect(dongle_handle_1->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_1->mixer_member, 2);
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle_1->mixer_member, 1);
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle_1->mixer_member, 2);
            }

            /* update mixer status */
            dongle_handle_0->mixer_status = BLE_AUDIO_SOURCE_STREAM_MIXER_MIX;
            dongle_handle_1->mixer_status = BLE_AUDIO_SOURCE_STREAM_MIXER_MIX;

            xSemaphoreGive(g_ble_audio_dongle_dl_xSemaphore);

            BLE_LOG_I("[ble audio dongle][dl][config] mix done. %d, %d, %d, %d\r\n", 4, scenario_id_0, sub_id_0.ble_audio_dongle_id, scenario_id_1, sub_id_1.ble_audio_dongle_id);
            break;

        case BLE_AUDIO_DONGLE_CONFIG_OP_MUSIC_UNMIX:
            /* unmix stream */
            scenario_id_0 = source->param.data_dl.scenario_type;
            sub_id_0.ble_audio_dongle_id = source->param.data_dl.scenario_sub_id;
            if (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) {
                sub_id_0.ble_audio_dongle_id = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN;
                scenario_id_0 = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE;
            } else if (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) {
                sub_id_0.ble_audio_dongle_id = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN;
                scenario_id_0 = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE;
            }
            application_ptr_0 = port_audio_transmitter_get_connection_if(scenario_id_0, sub_id_0);
            if (application_ptr_0 == NULL) {
                AUDIO_ASSERT(0);
            }
            /* get application handle */
            if ((application_ptr_0->source->scenario_type != AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) &&
                (application_ptr_0->source->scenario_type != AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)) {
                dongle_handle_0 = (ble_audio_dongle_dl_handle_t *)(application_ptr_0->sink->param.bt_common.scenario_param.dongle_handle);
            } else {
                dongle_handle_0 = ble_audio_dongle_query_dl_handle(application_ptr_0->source);
            }
            /* update mixer status */
            dongle_handle_0->mixer_status = BLE_AUDIO_SOURCE_STREAM_MIXER_DISCON;
            // /* do disconnections */
            // stream_function_sw_mixer_channel_disconnect_all(dongle_handle_0->mixer_member);
            BLE_LOG_I("[ble audio dongle][dl][config] unmix done. %d, %d\r\n", 2, scenario_id_0, sub_id_0.ble_audio_dongle_id);
            break;

#ifdef AIR_SILENCE_DETECTION_ENABLE
        case BLE_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_ENABLE:
            audio_scenario_type = AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0+source->param.data_dl.scenario_sub_id-AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0;
            DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_ENABLE, (uint32_t)audio_scenario_type, false);
            DSP_MW_LOG_I("[ble audio dongle][dl][config]scenario %d-%d enable silence detection\r\n", 2,
                        source->param.data_dl.scenario_type,
                        source->param.data_dl.scenario_sub_id);
            break;

        case BLE_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_DISABLE:
            audio_scenario_type = AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0+source->param.data_dl.scenario_sub_id-AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0;
            DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_DISABLE, (uint32_t)audio_scenario_type, false);
            DSP_MW_LOG_I("[ble audio dongle][dl][config]scenario %d-%d disable silence detection\r\n", 2,
                        source->param.data_dl.scenario_type,
                        source->param.data_dl.scenario_sub_id);
            break;
#endif /* AIR_SILENCE_DETECTION_ENABLE */

        default:
            BLE_LOG_E("[ble audio dongle][dl][config] unknow cmd:%d\r\n", 1, config_param->config_operation);
            break;
    }

    return true;
}

ATTR_TEXT_IN_IRAM bool ble_audio_dongle_dl_source_get_avail_size(SOURCE source, uint32_t *avail_size)
{
    uint32_t frame_count = 0;
    SINK sink = source->transform->sink;
    ble_audio_dongle_dl_handle_t *dongle_handle = (ble_audio_dongle_dl_handle_t *)(sink->param.bt_common.scenario_param.dongle_handle);
    uint32_t remain_samples = 0;

    if (source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.fetch_flag == 0) {
        /* there is no fetch requirement, so set avail size as 0 */
        *avail_size = 0;
    } else {
        if (source->streamBuffer.ShareBufferInfo.read_offset < source->streamBuffer.ShareBufferInfo.write_offset) {
            /* normal case */
            *avail_size = source->streamBuffer.ShareBufferInfo.write_offset - source->streamBuffer.ShareBufferInfo.read_offset;
        } else if (source->streamBuffer.ShareBufferInfo.read_offset == source->streamBuffer.ShareBufferInfo.write_offset) {
            if (source->streamBuffer.ShareBufferInfo.bBufferIsFull == true) {
                /* buffer is full, so ReadOffset == WriteOffset */
                *avail_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;
                BLE_LOG_E("[ble audio dongle][dl]: buffer is full, index = %u", 1, source->streamBuffer.ShareBufferInfo.read_offset);
            } else {
                /* buffer is empty, so ReadOffset == WriteOffset */
                *avail_size = 0;
                dongle_handle->process_frames = 0;

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
                uint32_t current_timestamp = 0;
                uint32_t duration = 0;
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
                hal_gpt_get_duration_count(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.data_timestamp,
                                           current_timestamp,
                                           &duration);
                BLE_LOG_E("[ble audio dongle][dl]: buffer is empty in a long time, index = %u", 1, source->streamBuffer.ShareBufferInfo.read_offset);
                BLE_LOG_E("[ble audio dongle][dl]: duration = %u, last time = %u, current time = %u", 3,
                             duration,
                             source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.data_timestamp,
                             current_timestamp);
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */
            }
        } else {
            /* buffer wrapper case */
            *avail_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num
                        - source->streamBuffer.ShareBufferInfo.read_offset
                        + source->streamBuffer.ShareBufferInfo.write_offset;
        }

        if (*avail_size != 0) {
            /* if this is the first time, check if the data in usb buffer is larger than 11ms */
            if (((uint32_t)source->param.data_dl.current_notification_index == 0) &&
                (*avail_size < (dongle_handle->usb_min_start_frames * (uint32_t)source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size))) {
                /* the data is not enough, so change avail_size to zero */
                *avail_size = 0;
                if (hal_nvic_query_exception_number() > 0) {
                    dongle_handle->data_status = BLE_AUDIO_SOURCE_STREAM_DATA_EMPTY;
                }
            } else {
                /* change avail_size to actual data size in the buffer */
                frame_count = *avail_size / source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
                *avail_size = *avail_size - frame_count * sizeof(audio_transmitter_frame_header_t);

                if (hal_nvic_query_exception_number() > 0) {
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
                    /* check if there are at least 10ms data */
                    remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / ble_audio_dl_info.lc3_packet_frame_sample_byte;
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */

                    if ((frame_count * dongle_handle->src_out_frame_samples + remain_samples) >= ble_audio_dl_info.lc3_packet_frame_samples) {
                        /* update data status to indicate there are data in stream and the mixer needs to wait this stream */
                        dongle_handle->data_status = BLE_AUDIO_SOURCE_STREAM_DATA_IN_STREAM;
                    } else {
                        /* When in irq level, there should be at least 9ms data in share buffer because the period of ccni is 10ms */
                        BLE_LOG_E("[ble audio dongle][dl][get_avail_size]: handle 0x%x: data is not enough, %u, %u", 3, dongle_handle, *avail_size, remain_samples);
                        /* update data status to indicate there are not enough data in stream and the mixer do not need to wait this stream */
                        dongle_handle->data_status = BLE_AUDIO_SOURCE_STREAM_DATA_EMPTY;
                        *avail_size = 0;
                        dongle_handle->process_frames = 0;
                    }
                }
            }
        }

#ifdef AIR_SILENCE_DETECTION_ENABLE
        if (*avail_size == 0)
        {
            if ((hal_nvic_query_exception_number() > 0) && (dongle_handle->total_number == 1))
            {
                audio_scenario_type_t audio_scenario_type;
                ble_audio_dongle_silence_detection_handle_t *silence_detection_handle;

                /* get scenario and handle */
                audio_scenario_type = source->scenario_type;
                silence_detection_handle = &ble_audio_dongle_silence_detection_handle[source->scenario_type - AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0];
                if (silence_detection_handle->enable)
                {
                    /* copy all zere data into the buffer only when there is only one streams is opened and there is no usb data in this stream */
                    memset(silence_detection_handle->data_buf, 0, sizeof(int32_t)*ble_audio_dl_info.lc3_packet_frame_samples);
                    silence_detection_handle->data_size = ble_audio_dl_info.lc3_packet_frame_samples*sizeof(int32_t);

                    /* send msg to DTM task to do silence detection */
                    DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_PROCESS, (uint32_t)audio_scenario_type, true);
                }
            }
        }
#endif /* AIR_SILENCE_DETECTION_ENABLE */
    }

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][get_avail_size]:%u, %u, %d, %u, %d", 5,
                 source->streamBuffer.ShareBufferInfo.read_offset,
                 source->streamBuffer.ShareBufferInfo.write_offset,
                 *avail_size,
                 current_timestamp,
                 hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_IRAM uint32_t ble_audio_dongle_dl_source_copy_payload(SOURCE source, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    DSP_STREAMING_PARA_PTR stream = NULL;
    audio_transmitter_frame_header_t *frame_header = NULL;
    uint32_t total_frames = 0;
    uint32_t total_samples = 0;
    uint32_t current_frame_samples = 0;
    uint32_t current_frame = 0;
    uint32_t avail_size = 0;
    uint32_t total_buffer_size = 0;
    uint32_t ReadOffset = 0;
    SINK sink = source->transform->sink;
    ble_audio_dongle_dl_handle_t *dongle_handle = (ble_audio_dongle_dl_handle_t *)(sink->param.bt_common.scenario_param.dongle_handle);
    UNUSED(dst_buf);

    /* update data receive timestamp */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.data_timestamp);

    /* get the stream */
    stream = DSP_Streaming_Get(source, source->transform->sink);

    /* get the total frame number and buffer size in share buffer */
    total_frames = length / (source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t));
    total_buffer_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size *
                        source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;

    /* copy the pcm data in share buffer into the stream buffers */
    if (source->param.data_dl.current_notification_index == 0) {
        ble_audio_dongle_dl_source_get_avail_size(source, &avail_size);
        if (avail_size > length) {
            /* at the first time, maybe the actual data size is larger than stream->callback.EntryPara.in_malloc_size */
            /* in dsp_callback.c, DSP_Callback_Processing will cut length to the stream->callback.EntryPara.in_malloc_size */
            /* in here, we need to change length back to the actual data size */
            length = avail_size;
            total_frames = length / (source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t));
        }

        /* parameters check */
        if (total_frames < dongle_handle->usb_min_start_frames) {
            BLE_LOG_E("[ble audio dongle][dl]: error parameters %d, %d", 2, total_frames, length);
            AUDIO_ASSERT(0);
        }

        /* Only copy the last 11ms data from the share buffer and the remian data in the head are dropped */
        total_samples = 0;
        current_frame = 0;
        ReadOffset = (source->streamBuffer.ShareBufferInfo.read_offset +
                      source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * (total_frames - dongle_handle->usb_min_start_frames)
                     ) % total_buffer_size;
        src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + ReadOffset);
        while (current_frame < dongle_handle->usb_min_start_frames) {
            /* get current frame info and current frame samples */
            frame_header = (audio_transmitter_frame_header_t *)src_buf;
            if (dongle_handle->usb_sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
            {
                /* get samples */
                current_frame_samples = frame_header->payload_len / (sizeof(U16) * 2);
                // if (current_frame == 0)
                // {
                //     /* only copy 0.5ms(48K*0.5ms=24samples) data at the first time */
                //     current_frame_samples = 24;
                //     /* offset 0.5ms data in src_buf */
                //     src_buf = src_buf + 24 * (sizeof(U16) * 2);
                // }

                /* copy usb audio data from share buffer */
                if (ble_audio_dl_info.lc3_packet_frame_sample_byte == 2) {
                    DSP_I2D_BufferCopy_16bit_mute((U16 *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                             (U16 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                             (U16 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                             current_frame_samples,
                                             false);
                } else {
                    ShareBufferCopy_I_16bit_to_D_32bit_2ch((U32 *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                         (U32 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                                         (U32 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                                         current_frame_samples);
                }

                // if (current_frame == 0)
                // {
                //     /* offset back src_buf to the original position */
                //     src_buf = src_buf - 24 * (sizeof(U16) * 2);
                // }
            }
            else if (dongle_handle->usb_sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)
            {
                /* get samples */
                current_frame_samples = frame_header->payload_len / (3 * 2);

                /* copy usb audio data from share buffer */
                if (ble_audio_dl_info.lc3_packet_frame_sample_byte == 2) {
                    ShareBufferCopy_I_24bit_to_D_16bit_2ch((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                            (U16 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                                            (U16 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                                            current_frame_samples);
                } else {
                    ShareBufferCopy_I_24bit_to_D_32bit_2ch((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                            (U32 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                                            (U32 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                                            current_frame_samples);
                }
            }
            else
            {
                BLE_LOG_E("[ble audio dongle][dl]: error pcm format %d", 1, dongle_handle->usb_sample_format);
                AUDIO_ASSERT(0);
            }

            /* update total samples */
            total_samples += current_frame_samples;

            /* update copied frame number */
            current_frame += 1;

            /* change to the next frame */
            ReadOffset = (source->streamBuffer.ShareBufferInfo.read_offset +
                          source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * (total_frames - dongle_handle->usb_min_start_frames + current_frame)
                         ) % total_buffer_size;
            src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + ReadOffset);
        }

        /* update processed frames */
        source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames = total_frames;

        /* always output 480 samples */
        stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, ble_audio_dl_info.lc3_packet_frame_samples * ble_audio_dl_info.lc3_packet_frame_sample_byte);
        stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, ble_audio_dl_info.lc3_packet_frame_samples * ble_audio_dl_info.lc3_packet_frame_sample_byte);

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
        uint32_t current_timestamp = 0;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
        BLE_LOG_I("[ble audio dongle][dl][copy_payload]: get first data packet time = %u.", 1, current_timestamp);
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */
    } else {
        /* parameters check */
        if (total_frames > dongle_handle->usb_min_start_frames) {
            /* in here, it means there are at least 11 packet data in share buffer. */
            BLE_LOG_E("[ble audio dongle][dl]: too much data in share buffer %d, %d, %u, %u, 0x%x, 0x%x", 6,
                         total_frames,
                         length,
                         source->streamBuffer.ShareBufferInfo.read_offset,
                         source->streamBuffer.ShareBufferInfo.write_offset,
                         dongle_handle->data_in_gpt_count,
                         dongle_handle->ccni_gpt_count);

            // AUDIO_ASSERT(0);

            /* get the new read index */
            source->streamBuffer.ShareBufferInfo.read_offset = (source->streamBuffer.ShareBufferInfo.write_offset + total_buffer_size -
                                                               source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * dongle_handle->usb_min_start_frames
                                                              ) % total_buffer_size;

            /* set WriteOffset - 11 as the new ReadOffset, it will drop the other data in the share buffer */
            audio_transmitter_share_information_update_read_offset(source, source->streamBuffer.ShareBufferInfo.read_offset);

            /* get new share buffer info*/
            audio_transmitter_share_information_fetch(source, NULL);

            /* only process the last 3 packet data */
            total_frames = dongle_handle->usb_min_start_frames;
            length = dongle_handle->usb_min_start_frames * (source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t));

            BLE_LOG_E("[ble audio dongle][dl]: change status, %d, %d, %u, %u", 4,
                         total_frames,
                         length,
                         source->streamBuffer.ShareBufferInfo.read_offset,
                         source->streamBuffer.ShareBufferInfo.write_offset);
        }

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
        int32_t compensatory_samples = 0;
        uint32_t remain_samples = 0;
        uint32_t clk_skew_watermark;

        /* get remain samples */
        remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / ble_audio_dl_info.lc3_packet_frame_sample_byte;

        /* get clk skew water mark */
        if (ble_audio_dl_info.lc3_packet_frame_interval == 7500) {
            clk_skew_watermark = dongle_handle->src_out_frame_samples * 3 / 2;
        } else {
            clk_skew_watermark = dongle_handle->src_out_frame_samples * 2;
        }

        /* choose which way to do compensatory */
        if (dongle_handle->compen_method == BLE_AUDIO_SOURCE_COMPENSATORY_METHOD_SW_CLK_SKEW) {
            /* do nothing because sw clk skew is under running */
        } else if ((dongle_handle->total_compen_samples != 0) &&
                   (dongle_handle->compen_method == BLE_AUDIO_SOURCE_COMPENSATORY_METHOD_DISABLE)) {
            /* use sw clk skew on this stream */
            dongle_handle->compen_method = BLE_AUDIO_SOURCE_COMPENSATORY_METHOD_SW_CLK_SKEW;
        }

        /* do compensatory */
        if (dongle_handle->compen_method == BLE_AUDIO_SOURCE_COMPENSATORY_METHOD_SW_CLK_SKEW) {
            /* double check if the compen_samples is right */
            if ((dongle_handle->total_compen_samples > 0) && ((remain_samples + total_frames * dongle_handle->src_out_frame_samples - ble_audio_dl_info.lc3_packet_frame_samples) >= clk_skew_watermark)) {
                dongle_handle->total_compen_samples = clk_skew_watermark - (remain_samples + total_frames * dongle_handle->src_out_frame_samples - ble_audio_dl_info.lc3_packet_frame_samples);
            } else if ((dongle_handle->total_compen_samples < 0) && ((remain_samples + total_frames * dongle_handle->src_out_frame_samples - ble_audio_dl_info.lc3_packet_frame_samples) <= clk_skew_watermark)) {
                dongle_handle->total_compen_samples = clk_skew_watermark - (remain_samples + total_frames * dongle_handle->src_out_frame_samples - ble_audio_dl_info.lc3_packet_frame_samples);
            }

            if (dongle_handle->total_compen_samples > 0) {
                /* buffer output 479 samples, clk skew will change them to 480 samples */
                compensatory_samples = 1;
                dongle_handle->total_compen_samples -= 1;
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, (ble_audio_dl_info.lc3_packet_frame_samples - 1) * ble_audio_dl_info.lc3_packet_frame_sample_byte);
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, (ble_audio_dl_info.lc3_packet_frame_samples - 1) * ble_audio_dl_info.lc3_packet_frame_sample_byte);
            } else if (dongle_handle->total_compen_samples < 0) {
                /* buffer output 481 samples, clk skew will change them to 480 samples */
                compensatory_samples = -1;
                dongle_handle->total_compen_samples += 1;
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, (ble_audio_dl_info.lc3_packet_frame_samples + 1) * ble_audio_dl_info.lc3_packet_frame_sample_byte);
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, (ble_audio_dl_info.lc3_packet_frame_samples + 1) * ble_audio_dl_info.lc3_packet_frame_sample_byte);
            } else {
                /* buffer output 480 samples */
                compensatory_samples = 0;
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, ble_audio_dl_info.lc3_packet_frame_samples * ble_audio_dl_info.lc3_packet_frame_sample_byte);
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, ble_audio_dl_info.lc3_packet_frame_samples * ble_audio_dl_info.lc3_packet_frame_sample_byte);
            }
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, compensatory_samples);
            source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples = compensatory_samples;

            if (dongle_handle->total_compen_samples == 0) {
                dongle_handle->compen_method = BLE_AUDIO_SOURCE_COMPENSATORY_METHOD_DISABLE;
                dongle_handle->compen_count = 0;
                dongle_handle->compen_flag = 0;
            }
        } else {
            /* calculator compensatory samples */
            if ((dongle_handle->total_compen_samples == 0) && ((remain_samples + total_frames * dongle_handle->src_out_frame_samples - ble_audio_dl_info.lc3_packet_frame_samples) < clk_skew_watermark)) {
                if (dongle_handle->compen_count < 10) {
                    /* in here, it means there may be +1ms compensatory but its persistent period is less than 50ms */
                    dongle_handle->compen_count += 1;
                    dongle_handle->compen_flag = 1;
                } else {
                    /* in here, it means there is +1ms compensatory and its persistent period is more than 50ms */
                    dongle_handle->total_compen_samples = clk_skew_watermark - (remain_samples + total_frames * dongle_handle->src_out_frame_samples - ble_audio_dl_info.lc3_packet_frame_samples);
                    dongle_handle->compen_count = 0;
                    dongle_handle->compen_flag = 0;
                }
            } else if ((dongle_handle->total_compen_samples == 0) && (dongle_handle->compen_flag == 1) && ((remain_samples + total_frames * dongle_handle->src_out_frame_samples - ble_audio_dl_info.lc3_packet_frame_samples) >= clk_skew_watermark)) {
                /* in here, it means maybe the +1ms compensatory is false alarm and the usb irq is just not stable in this time */
                if (dongle_handle->compen_count != 0) {
                    dongle_handle->compen_count -= 1;
                }
                if (dongle_handle->compen_count == 0) {
                    dongle_handle->compen_flag = 0;
                }
            } else if ((dongle_handle->total_compen_samples == 0) && ((remain_samples + total_frames * dongle_handle->src_out_frame_samples - ble_audio_dl_info.lc3_packet_frame_samples) > clk_skew_watermark)) {
                if (dongle_handle->compen_count < 10) {
                    /* in here, it means there may be -1ms compensatory but its persistent period is less than 50ms */
                    dongle_handle->compen_count += 1;
                    dongle_handle->compen_flag = -1;
                } else {
                    /* in here, it means there is -1ms compensatory and its persistent period is more than 50ms*/
                    dongle_handle->total_compen_samples = clk_skew_watermark - (remain_samples + total_frames * dongle_handle->src_out_frame_samples - ble_audio_dl_info.lc3_packet_frame_samples);
                    dongle_handle->compen_count = 0;
                }
            } else if ((dongle_handle->total_compen_samples == 0) && (dongle_handle->compen_flag == -1) && ((remain_samples + total_frames * dongle_handle->src_out_frame_samples - ble_audio_dl_info.lc3_packet_frame_samples) <= clk_skew_watermark)) {
                /* in here, it means maybe the -1ms compensatory is false alarm and the usb irq is just not stable in this time */
                if (dongle_handle->compen_count != 0) {
                    dongle_handle->compen_count -= 1;
                }
                if (dongle_handle->compen_count == 0) {
                    dongle_handle->compen_flag = 0;
                }
            }

            /* always output 480 samples */
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, ble_audio_dl_info.lc3_packet_frame_samples * ble_audio_dl_info.lc3_packet_frame_sample_byte);
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, ble_audio_dl_info.lc3_packet_frame_samples * ble_audio_dl_info.lc3_packet_frame_sample_byte);

            /* bypass sw clk skew */
            compensatory_samples = 0;
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, compensatory_samples);
            source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples = compensatory_samples;
        }
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

        /* Copy all data from the share buffer */
        total_samples = 0;
        current_frame = 0;
        ReadOffset = source->streamBuffer.ShareBufferInfo.read_offset;
        src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + ReadOffset);
        while (current_frame < total_frames) {
            /* get current frame info and current frame samples */
            frame_header = (audio_transmitter_frame_header_t *)src_buf;
            if (dongle_handle->usb_sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
            {
                /* get samples */
                current_frame_samples = frame_header->payload_len / (sizeof(U16) * 2);

                /* copy usb audio data from share buffer */
                if (ble_audio_dl_info.lc3_packet_frame_sample_byte == 2) {
                    DSP_I2D_BufferCopy_16bit_mute((U16 *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                             (U16 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                             (U16 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                             current_frame_samples,
                                             false);
                } else {
                    ShareBufferCopy_I_16bit_to_D_32bit_2ch((U32 *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                     (U32 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                                     (U32 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                                     current_frame_samples);
                }
            }
            else if (dongle_handle->usb_sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)
            {
                /* get samples */
                current_frame_samples = frame_header->payload_len / (3 * 2);

                /* copy usb audio data from share buffer */
                if (ble_audio_dl_info.lc3_packet_frame_sample_byte == 2) {
                    ShareBufferCopy_I_24bit_to_D_16bit_2ch((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                            (U16 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                                            (U16 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                                            current_frame_samples);
                } else {
                    ShareBufferCopy_I_24bit_to_D_32bit_2ch((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                            (U32 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                                            (U32 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                                            current_frame_samples);
                }
            }
            else
            {
                BLE_LOG_E("[ble audio dongle][dl]: error pcm format %d", 1, dongle_handle->usb_sample_format);
                AUDIO_ASSERT(0);
            }

            /* update total samples */
            total_samples += current_frame_samples;

            /* update copied frame number */
            current_frame += 1;

            /* change to the next frame */
            ReadOffset = (source->streamBuffer.ShareBufferInfo.read_offset +
                          source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * current_frame
                         ) % total_buffer_size;
            src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + ReadOffset);
        }
        if (dongle_handle->usb_sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            if (total_samples != (length / (sizeof(S16) * 2))) {
                BLE_LOG_E("[ble audio dongle][dl]: error copy samples %d, %d", 2, length, total_samples);
                AUDIO_ASSERT(0);
            }
        }
        else
        {
            if (total_samples != (length / (3 * 2))) {
                BLE_LOG_E("[ble audio dongle][dl]: error copy samples %d, %d", 2, length, total_samples);
                AUDIO_ASSERT(0);
            }
        }

        /* update processed frames */
        source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames = total_frames;
    }

    /* update stream status */
    stream->callback.EntryPara.in_size = total_samples * ble_audio_dl_info.lc3_packet_frame_sample_byte;
    stream->callback.EntryPara.in_channel_num = 2;
    stream->callback.EntryPara.in_sampling_rate = source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.codec_param.pcm.sample_rate / 1000;
    if (ble_audio_dl_info.lc3_packet_frame_sample_byte == 2) {
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.feature_res = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.process_res = RESOLUTION_16BIT;
    } else {
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.feature_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.process_res = RESOLUTION_32BIT;
    }

    /* update state machine */
    dongle_handle->compen_samples = source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples;
    dongle_handle->process_frames = source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames;

    /* avoid payload length check error in here */
    length = (length > source->param.data_dl.max_payload_size) ? source->param.data_dl.max_payload_size : length;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&dongle_handle->data_in_gpt_count);

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][copy_payload]: %d, %d, %u, %d", 4, total_samples, length, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    return length;
}

ATTR_TEXT_IN_IRAM uint32_t ble_audio_dongle_dl_source_get_new_read_offset(SOURCE source, uint32_t amount)
{
    UNUSED(amount);
    uint32_t total_buffer_size;
    n9_dsp_share_info_ptr ShareBufferInfo;
    uint32_t ReadOffset;

    /* get new read offset */
    ShareBufferInfo = &source->streamBuffer.ShareBufferInfo;
    total_buffer_size = ShareBufferInfo->sub_info.block_info.block_size * ShareBufferInfo->sub_info.block_info.block_num;
    ReadOffset = (ShareBufferInfo->read_offset +
                  ShareBufferInfo->sub_info.block_info.block_size * source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames
                 ) % total_buffer_size;

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][get_new_read_offset]: %u, %u, %d", 3, ReadOffset, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    return ReadOffset;
}

ATTR_TEXT_IN_IRAM void ble_audio_dongle_dl_source_drop_postprocess(SOURCE source, uint32_t amount)
{
    uint32_t saved_mask = 0;
    SINK sink = source->transform->sink;
    ble_audio_dongle_dl_handle_t *dongle_handle = (ble_audio_dongle_dl_handle_t *)(sink->param.bt_common.scenario_param.dongle_handle);
    UNUSED(amount);
    UNUSED(dongle_handle);

    /* update index */
    if (source->param.data_dl.current_notification_index == 0) {
        source->param.data_dl.current_notification_index = 1;
    }

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t remain_samples = 0;
    remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / ble_audio_dl_info.lc3_packet_frame_sample_byte;
    BLE_LOG_I("[ble audio dongle][dl][drop_postprocess]: %d, %d, %d, %d", 4,
                 source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples,
                 remain_samples,
                 source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames,
                 amount);
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

#ifdef AIR_AUDIO_DUMP_ENABLE
#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_DUMP
    uint32_t i, address, read_offset, total_buffer_size;
    total_buffer_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size *
                        source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;
    for (i = 0; i < source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames; i++) {
        read_offset = (source->streamBuffer.ShareBufferInfo.read_offset +
                       source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * i
                      ) % total_buffer_size;
        address = source->streamBuffer.ShareBufferInfo.start_addr + read_offset + sizeof(audio_transmitter_frame_header_t);
        LOG_AUDIO_DUMP((uint8_t *)address,
                       source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t),
                       SOURCE_IN2 + (source->param.data_dl.scenario_sub_id - AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0));
    }
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */

    /* update state machine */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.fetch_flag = 0;
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames = 0;
    hal_nvic_restore_interrupt_mask(saved_mask);

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][drop_postprocess]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */
}

bool ble_audio_dongle_dl_source_close(SOURCE source)
{
    UNUSED(source);

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][close]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_IRAM bool ble_audio_dongle_dl_sink_get_avail_size(SINK sink, uint32_t *avail_size)
{
    UNUSED(sink);

    /* change avail_size to actual frame size */
    *avail_size = ble_audio_dl_info.share_buffer_blk_size - sizeof(LE_AUDIO_HEADER);

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][get_avail_size]: %d, %u, %d", 3, *avail_size, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_IRAM uint32_t ble_audio_dongle_dl_sink_copy_payload(SINK sink, uint8_t *src_buf, uint32_t length)
{
    uint32_t i, j;
    DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(sink->transform->source, sink);
    ble_audio_dongle_dl_handle_t *dongle_handle = (ble_audio_dongle_dl_handle_t *)(sink->transform->sink->param.bt_common.scenario_param.dongle_handle);
    UNUSED(src_buf);
    UNUSED(dongle_handle);
    uint32_t *src_addr = NULL;
    uint32_t *des_addr = NULL;
    LE_AUDIO_HEADER *p_ble_audio_header = NULL;
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t diff_clk;
    UNUSED(diff_clk);

    if (length != ble_audio_dl_info.lc3_packet_frame_size) {
        AUDIO_ASSERT(0);
    }

    /* get current bt clock */
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

    /* write LC3 packet to different share buffer one by one */
    for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
        src_addr = stream_function_get_inout_buffer((void *)(&(stream->callback.EntryPara)), (i + 1));

        /* TODO: add check block index and bt clock is matched? */

        /* share_buffer_block_index will be updated in every ccni callback */
        des_addr = (uint32_t *)(hal_memview_cm4_to_dsp0(ble_audio_dl_info.p_share_buffer_info[i]->start_addr) + ble_audio_dl_info.share_buffer_blk_size * ble_audio_dl_info.share_buffer_blk_index);
        p_ble_audio_header = (LE_AUDIO_HEADER *)des_addr;

        /* write lc3 packet into share buffer block */
        des_addr = (uint32_t *)((uint32_t)des_addr + sizeof(LE_AUDIO_HEADER));
        for (j = 0; j < (((uint32_t)(ble_audio_dl_info.lc3_packet_frame_size + 3)) / 4); j++) {
            /* share buffer block must be 4B aligned, so we can use word access to get better performance */
            *des_addr++ = *src_addr++;
        }

        /* write ble audio header into share buffer block */
        p_ble_audio_header->DataOffset = sizeof(LE_AUDIO_HEADER);
        p_ble_audio_header->PduLen     = ble_audio_dl_info.lc3_packet_frame_size;
        p_ble_audio_header->DataLen    = ble_audio_dl_info.lc3_packet_frame_size;
        p_ble_audio_header->TimeStamp  = bt_clk;
        p_ble_audio_header->SampleSeq  = ble_audio_dl_info.seq_num;
    }
    ble_audio_dl_info.seq_num += 1;

    /* update time stamp */
    sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp = bt_clk;

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][copy_payload]:0x%x, %d, %u, %d", 4, bt_clk, length, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    return length;
}

ATTR_TEXT_IN_IRAM bool ble_audio_dongle_dl_sink_get_new_write_offset(SINK sink, U32 amount, uint32_t *new_write_offset)
{
    uint32_t i, write_offset;
    UNUSED(amount);
    UNUSED(new_write_offset);
    UNUSED(sink);

    /* update write index */
    vTaskSuspendAll();
    StreamDSP_HWSemaphoreTake();
    for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
        write_offset = (uint32_t)(((ble_audio_dl_info.share_buffer_blk_index + 1) % ble_audio_dl_info.share_buffer_blk_num) * ble_audio_dl_info.share_buffer_blk_size);
        ble_audio_dl_info.p_share_buffer_info[i]->write_offset = write_offset;
    }
    StreamDSP_HWSemaphoreGive();
    xTaskResumeAll();

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][get_new_write_offset]:0x%x, %u, %d", 3, write_offset, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    /* we will update the write offsets of the different share buffers in here directly, so return false to aviod the upper layer update write offset */
    return false;
}

ATTR_TEXT_IN_IRAM bool ble_audio_dongle_dl_sink_query_notification(SINK sink, bool *notification_flag)
{
    UNUSED(sink);

    *notification_flag = false;

    return true;
}

ATTR_TEXT_IN_IRAM bool ble_audio_dongle_dl_sink_send_data_ready_notification(SINK sink)
{
    UNUSED(sink);

    return true;
}

ATTR_TEXT_IN_IRAM void ble_audio_dongle_dl_sink_flush_postprocess(SINK sink, uint32_t amount)
{
    int32_t i;
    uint32_t remain_samples = 0;
    SOURCE source;
    LE_AUDIO_HEADER *p_ble_audio_header = NULL;
    ble_audio_dongle_dl_handle_t *dongle_handle = (ble_audio_dongle_dl_handle_t *)(sink->transform->sink->param.bt_common.scenario_param.dongle_handle);
    UNUSED(amount);
    UNUSED(dongle_handle);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&dongle_handle->bt_out_gpt_count);

    /* clear PduLen on other share buffer blocks (not current share buffer block) for indicating there is no LC3 packet on this share buffer block */
    for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
        while (ble_audio_dl_info.share_buffer_blk_index_previous != ble_audio_dl_info.share_buffer_blk_index) {
            p_ble_audio_header = (LE_AUDIO_HEADER *)(hal_memview_cm4_to_dsp0(ble_audio_dl_info.p_share_buffer_info[i]->start_addr) + ble_audio_dl_info.share_buffer_blk_size * ble_audio_dl_info.share_buffer_blk_index_previous);
            // if (p_ble_audio_header->PduLen != 0)
            // {
            //     p_ble_audio_header->PduLen = 0;
            // }
            ble_audio_dl_info.share_buffer_blk_index_previous = (ble_audio_dl_info.share_buffer_blk_index_previous + 1) % (ble_audio_dl_info.share_buffer_blk_num);
        }
    }

    if (sink->param.bt_common.current_notification_index == 0) {
        /* increase the index */
        sink->param.bt_common.current_notification_index += 1;
        BLE_LOG_I("[ble audio dongle][dl]: ble audio first packet, 0x%x.", 1, sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp);
    }

#ifdef AIR_AUDIO_DUMP_ENABLE
#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_DUMP
#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
    uint8_t *codec_in_data_address;
    uint32_t codec_in_data_frame_size;
    /* dump channel 1 LC3 codec in data */
    if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_LC3) {
        stream_codec_encoder_lc3_branch_get_data_info(LC3_ENC_BRANCH_INDEX_0, 1, &codec_in_data_address, &codec_in_data_frame_size);
    }
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
    else if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS) {
        stream_codec_encoder_lc3plus_get_data_info(LC3PLUS_ENC_PORT_0, 1, &codec_in_data_address, &codec_in_data_frame_size);
    }
#endif
    else {
        codec_in_data_address = NULL;
        codec_in_data_frame_size = 0;
    }
    LOG_AUDIO_DUMP((uint8_t *)codec_in_data_address, codec_in_data_frame_size, AUDIO_SOURCE_IN_L);
    /* dump channel 2 LC3 codec in data */
    if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_LC3) {
        stream_codec_encoder_lc3_branch_get_data_info(LC3_ENC_BRANCH_INDEX_0, 2, &codec_in_data_address, &codec_in_data_frame_size);
    }
#if defined(AIR_BT_LE_LC3PLUS_ENABLE)
    else if (dongle_handle->sink_info.bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_LC3) {
        stream_codec_encoder_lc3plus_get_data_info(LC3PLUS_ENC_PORT_0, 2, &codec_in_data_address, &codec_in_data_frame_size);
    }
#endif
    else {
        codec_in_data_address = NULL;
        codec_in_data_frame_size = 0;
    }
    LOG_AUDIO_DUMP((uint8_t *)codec_in_data_address, codec_in_data_frame_size, AUDIO_SOURCE_IN_R);
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */
    uint32_t write_offset;
    /* dump channel 1 LC3 packet data */
    write_offset = (uint32_t)(hal_memview_cm4_to_dsp0(ble_audio_dl_info.p_share_buffer_info[0]->start_addr) + ble_audio_dl_info.share_buffer_blk_size * ble_audio_dl_info.share_buffer_blk_index) + sizeof(LE_AUDIO_HEADER);
    LOG_AUDIO_DUMP((uint8_t *)write_offset, ble_audio_dl_info.lc3_packet_frame_size, SINK_OUT1);
    /* dump channel 2 LC3 packet data */
    write_offset = (uint32_t)(hal_memview_cm4_to_dsp0(ble_audio_dl_info.p_share_buffer_info[1]->start_addr) + ble_audio_dl_info.share_buffer_blk_size * ble_audio_dl_info.share_buffer_blk_index) + sizeof(LE_AUDIO_HEADER);
    LOG_AUDIO_DUMP((uint8_t *)write_offset, ble_audio_dl_info.lc3_packet_frame_size, SINK_OUT2);
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */

    /* output debug log */
    dongle_handle = ble_audio_dongle_first_dl_handle;
    for (i = 0; i < ble_audio_dongle_first_dl_handle->total_number; i++) {
        source = (SOURCE)(dongle_handle->owner);

        if (source->transform != NULL) {
#if (defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE) || (defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE)
            if ((source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) ||
                (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)) {
                /* afe in case */
                audio_dongle_afe_in_info_t *afe_in = &((ble_audio_dongle_dl_handle_t *)dongle_handle)->source_info.afe_in;
                afe_in->cur_read_offset = sink->transform->source->streamBuffer.BufferInfo.ReadOffset;
                uint32_t src_1_tracking_rate = AFE_READ(ASM_FREQUENCY_2) * 100 / 4096;
                uint32_t src_2_tracking_rate = AFE_READ(ASM2_FREQUENCY_2) * 100 / 4096;
                BLE_LOG_I("[ble audio dongle][dl][scenario type %d][AFE IN] latency size %d wo[%d] ro[%d] afe wo[0x%x] base[0x%x] src1 rate %d src2 rate %d", 8,
                    sink->scenario_type,
                    afe_in->afe_buffer_latency_size,
                    dongle_handle->source->streamBuffer.BufferInfo.WriteOffset,
                    dongle_handle->source->streamBuffer.BufferInfo.ReadOffset,
                    afe_in->afe_vul_cur,
                    afe_in->afe_vul_base,
                    src_1_tracking_rate,
                    src_2_tracking_rate
                );
            }
#endif
            if ((source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) ||
                (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1)) {
                remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / ble_audio_dl_info.lc3_packet_frame_sample_byte;
            }

            BLE_LOG_I("[ble audio dongle][dl][flush_postprocess]: handle 0x%x: %d, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x", 13,
                         dongle_handle,
                         ble_audio_dl_info.seq_num,
                         ble_audio_dl_info.share_buffer_blk_index,
                         dongle_handle->process_frames,
                         remain_samples,
                         dongle_handle->total_compen_samples,
                         dongle_handle->compen_samples,
                         dongle_handle->compen_count,
                         dongle_handle->compen_method,
                         dongle_handle->ccni_bt_count,
                         sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp,
                         dongle_handle->ccni_gpt_count,
                         dongle_handle->bt_out_gpt_count);
            /* update mixer status */
            if (dongle_handle->mixer_status == BLE_AUDIO_SOURCE_STREAM_MIXER_DISCON) {
                /* do disconnections */
                stream_function_sw_mixer_channel_disconnect_all(dongle_handle->mixer_member);
                dongle_handle->mixer_status = BLE_AUDIO_SOURCE_STREAM_MIXER_UNMIX;
                BLE_LOG_I("[ble audio dongle][dl] unmix type %d", 1, dongle_handle->source->scenario_type);
            }
#ifdef AIR_SILENCE_DETECTION_ENABLE
            audio_scenario_type_t audio_scenario_type;
            ble_audio_dongle_silence_detection_handle_t *silence_detection_handle;
            uint8_t *data_buf_1_head;
            uint8_t *data_buf_1_tail;
            uint32_t data_buf_1_size;

            /* get scenario and handle */
            audio_scenario_type = AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0+source->param.data_dl.scenario_sub_id-AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0;
            silence_detection_handle = &ble_audio_dongle_silence_detection_handle[source->param.data_dl.scenario_sub_id-AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0];
            if (silence_detection_handle->enable)
            {
                /* copy audio data into the silence detection buffer */
                silence_detection_handle->data_size = ble_audio_dl_info.lc3_packet_frame_samples*sizeof(int32_t);
                if (dongle_handle->process_frames != 0)
                {
                    /* data_buf_1_head & data_buf_2_head is pointer the frame start address */
                    stream_function_sw_mixer_channel_input_get_data_info(dongle_handle->mixer_member, 1, &data_buf_1_head, &data_buf_1_tail, &data_buf_1_size);
                    if (ble_audio_dl_info.lc3_packet_frame_sample_byte == 2) {
                        ShareBufferCopy_D_16bit_to_D_32bit_1ch((uint16_t* )data_buf_1_head, (uint32_t* )(silence_detection_handle->data_buf), ble_audio_dl_info.lc3_packet_frame_samples);
                    } else {
                        memcpy(silence_detection_handle->data_buf, data_buf_1_head, ble_audio_dl_info.lc3_packet_frame_samples * ble_audio_dl_info.lc3_packet_frame_sample_byte);
                    }
                }
                else
                {
                    memset(silence_detection_handle->data_buf, 0, silence_detection_handle->data_size);
                }

                /* send msg to DTM task to do silence detection */
                DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_PROCESS, (uint32_t)audio_scenario_type, false);
            }
#endif /* AIR_SILENCE_DETECTION_ENABLE */
        }

        /* switch to the next dl stream */
        dongle_handle = dongle_handle->next_dl_handle;
    }

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][flush_postprocess]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */
}

bool ble_audio_dongle_dl_sink_close(SINK sink)
{
    UNUSED(sink);

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][dl][sink_close]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    return true;
}

/******************************************************************************/
/*            BLE audio source dongle voice path Public Functions             */
/******************************************************************************/
void ble_audio_dongle_ul_init(SOURCE source, SINK sink, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    ble_audio_dongle_ul_handle_t *dongle_handle;
    UNUSED(sink);

    /* register ccni handler */
    bt_common_register_isr_handler(ble_audio_dongle_ul_ccni_handler);

    /* get handle for application config */
    dongle_handle = ble_audio_dongle_get_ul_handle(source);
    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle = (void *)dongle_handle;

    /* init audio info */
    memset(&ble_audio_ul_info, 0, sizeof(LE_AUDIO_INFO));
    ble_audio_dongle_ul_audio_info_init(dongle_handle, source, &bt_common_open_param->scenario_param.ble_audio_dongle_param.share_buffer_channel_1, bt_common_open_param->scenario_param.ble_audio_dongle_param.channel_enable);
    dongle_handle->usb_frame_size       = usb_audio_get_frame_size(&audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_type, &audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param);
    dongle_handle->usb_channel_num      = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.channel_mode;
    dongle_handle->usb_frame_samples    = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000;
    dongle_handle->usb_min_start_frames = ble_audio_ul_info.lc3_packet_frame_interval / 1000;
    BLE_LOG_I("[ble audio dongle][ul][config][USB][scenario %d][handle 0x%x] %u, %u, %u, %u\r\n", 6,
                 source->scenario_type,
                 dongle_handle,
                 dongle_handle->usb_frame_size,
                 dongle_handle->usb_channel_num,
                 dongle_handle->usb_frame_samples,
                 dongle_handle->usb_min_start_frames);
    if (ble_audio_ul_info.lc3_packet_frame_interval == 10000) {
        dongle_handle->play_en_delay                    = UL_PLAYEN_DELAY_FRAME_10000US;
        dongle_handle->play_en_first_packet_safe_delay  = UL_FIRST_PACKET_SAFE_DELAY_FRAME_10000US;
        dongle_handle->bt_retry_window                  = audio_dongle_bt_init_play_info.dl_retransmission_window_clk > UL_BT_RETRY_WINDOW_FRAME_10000US ?
                                                            audio_dongle_bt_init_play_info.dl_retransmission_window_clk : UL_BT_RETRY_WINDOW_FRAME_10000US;
        dongle_handle->bt_interval                      = UL_BT_RETRY_WINDOW_FRAME_10000US;
        dongle_handle->bt_channel_anchor_diff           = UL_BT_RETRY_WINDOW_FRAME_10000US - 1;
    } else if (ble_audio_ul_info.lc3_packet_frame_interval == 7500) {
        dongle_handle->play_en_delay                    = UL_PLAYEN_DELAY_FRAME_7500US;
        dongle_handle->play_en_first_packet_safe_delay  = UL_FIRST_PACKET_SAFE_DELAY_FRAME_7500US;
        dongle_handle->bt_retry_window                  = audio_dongle_bt_init_play_info.dl_retransmission_window_clk > UL_BT_RETRY_WINDOW_FRAME_7500US ?
                                                            audio_dongle_bt_init_play_info.dl_retransmission_window_clk : UL_BT_RETRY_WINDOW_FRAME_7500US;
        dongle_handle->bt_interval                      = UL_BT_RETRY_WINDOW_FRAME_7500US;
        dongle_handle->bt_channel_anchor_diff           = UL_BT_RETRY_WINDOW_FRAME_7500US - 1;
    }
    BLE_LOG_I("[ble audio dongle][ul][config][Play_infor][scenario %d][handle 0x%x] playen delay %u, safty delay %u, retry window %u(bt clk), interval %u, diff %u\r\n", 7,
                 source->scenario_type,
                 dongle_handle,
                 dongle_handle->play_en_delay,
                 dongle_handle->play_en_first_packet_safe_delay,
                 dongle_handle->bt_retry_window,
                 dongle_handle->bt_interval,
                 dongle_handle->bt_channel_anchor_diff
                 );
#ifdef AIR_SOFTWARE_SRC_ENABLE
    sw_src_config_t sw_src_config;
    dongle_handle->src_out_frame_samples = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000 * ble_audio_ul_info.lc3_packet_frame_interval / 1000;
    dongle_handle->src_out_frame_size = dongle_handle->src_out_frame_samples * sizeof(int16_t);
    sw_src_config.mode = SW_SRC_MODE_NORMAL;
    sw_src_config.channel_num = 2;
    sw_src_config.in_res = RESOLUTION_16BIT;
    sw_src_config.in_sampling_rate  = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.sample_rate;
    sw_src_config.in_frame_size_max = ble_audio_ul_info.lc3_packet_frame_samples * sizeof(int16_t) * 2;
    sw_src_config.out_res           = RESOLUTION_16BIT;
    sw_src_config.out_sampling_rate = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate;
    sw_src_config.out_frame_size_max = dongle_handle->src_out_frame_size;
    dongle_handle->src_port = stream_function_sw_src_get_port(source);
    stream_function_sw_src_init(dongle_handle->src_port, &sw_src_config);
    BLE_LOG_I("[ble audio dongle][ul][config][SW_SRC][scenario %d][handle 0x%x] %u, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                 source->scenario_type,
                 dongle_handle,
                 sw_src_config.in_res,
                 sw_src_config.in_sampling_rate,
                 sw_src_config.in_frame_size_max,
                 sw_src_config.out_res,
                 sw_src_config.out_sampling_rate,
                 sw_src_config.out_frame_size_max,
                 dongle_handle->src_out_frame_size,
                 dongle_handle->src_out_frame_samples);
#endif /* AIR_SOFTWARE_SRC_ENABLE */

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    sw_clk_skew_config_t sw_clk_skew_config;
    /* clock skew config */
    dongle_handle->compen_samples = 0;
    dongle_handle->clk_skew_count = 0;
    dongle_handle->clk_skew_port = stream_function_sw_clk_skew_get_port(source);
    sw_clk_skew_config.channel = 2;
    sw_clk_skew_config.bits = 16;
    sw_clk_skew_config.order = C_Flp_Ord_5;
    sw_clk_skew_config.skew_io_mode = C_Skew_Oup;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_1_FRAME;
    sw_clk_skew_config.skew_work_mode = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size = 2 * audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000 * ble_audio_ul_info.lc3_packet_frame_interval / 1000 * sizeof(int16_t); /* 20ms/32K buffer */
    sw_clk_skew_config.continuous_frame_size = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000 * ble_audio_ul_info.lc3_packet_frame_interval / 1000 * sizeof(int16_t); /* fixed 10ms/32K input */
    stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
    dongle_handle->clk_skew_watermark_samples = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000; // 1ms for clk skew
    BLE_LOG_I("[ble audio dongle][ul][config][SW_CLK_SKEW][scenario %d][handle 0x%x] %u, %u, %u, %u, %u, %u, %u, %u\r\n", 10,
                 source->scenario_type,
                 dongle_handle,
                 sw_clk_skew_config.channel,
                 sw_clk_skew_config.bits,
                 sw_clk_skew_config.order,
                 sw_clk_skew_config.skew_io_mode,
                 sw_clk_skew_config.skew_compensation_mode,
                 sw_clk_skew_config.skew_work_mode,
                 sw_clk_skew_config.max_output_size,
                 sw_clk_skew_config.continuous_frame_size);
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    sw_buffer_config_t buffer_config;
    buffer_config.mode = SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH;
    buffer_config.total_channels = 2;
    buffer_config.watermark_max_size = 32 * audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000 * ble_audio_ul_info.lc3_packet_frame_interval / 1000 * sizeof(int16_t); /* 120ms * 48K */
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000 * ble_audio_ul_info.lc3_packet_frame_interval / 1000 * sizeof(int16_t); /* 10ms * 32K */
    dongle_handle->buffer_port = stream_function_sw_buffer_get_port(source);
    stream_function_sw_buffer_init(dongle_handle->buffer_port, &buffer_config);
    /* prefill 1ms for clk_skew */
    stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->buffer_port, 0, 1 * audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate/1000*sizeof(int16_t), true);
    BLE_LOG_I("[ble audio dongle][ul][config][SW_BUFFER][scenario %d][handle 0x%x] %u, %u, %u, %u, %u\r\n", 7,
                 source->scenario_type,
                 dongle_handle,
                 buffer_config.mode,
                 buffer_config.total_channels,
                 buffer_config.watermark_max_size,
                 buffer_config.watermark_min_size,
                 buffer_config.output_size);
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */

#ifdef AIR_SOFTWARE_GAIN_ENABLE
    /* configuare sw gain */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution = RESOLUTION_16BIT;
    default_config.target_gain = bt_common_open_param->scenario_param.ble_audio_dongle_param.gain_default_L;
    default_config.up_step = 25;
    default_config.up_samples_per_step = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000;
    default_config.down_step = -25;
    default_config.down_samples_per_step = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = bt_common_open_param->scenario_param.ble_audio_dongle_param.gain_default_L;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    default_gain = bt_common_open_param->scenario_param.ble_audio_dongle_param.gain_default_R;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    BLE_LOG_I("[ble audio dongle][ul][config][SW_GAIN][scenario %d][handle 0x%x] %d, %d\r\n", 4,
                 source->scenario_type,
                 dongle_handle,
                 bt_common_open_param->scenario_param.ble_audio_dongle_param.gain_default_L,
                 bt_common_open_param->scenario_param.ble_audio_dongle_param.gain_default_R);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */

#ifdef AIR_SOFTWARE_MIXER_ENABLE
    /* sw mixer config */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback = NULL;
    callback_config.postprocess_callback = NULL;
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = RESOLUTION_16BIT;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_OVERWRITE;
    in_ch_config.buffer_size = audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000 * bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval / 1000 * sizeof(int16_t); //48K * 10ms
    if (ble_audio_ul_info.lc3_packet_frame_interval == 7500) { //48K * 8ms, when use 7.5ms, output will be 7/8 ms
        in_ch_config.buffer_size += audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate / 1000 * sizeof(int16_t) / 2;
    }
    // in_ch_config.buffer_size += 3*audio_transmitter_open_param->scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate/1000*sizeof(int16_t);
    out_ch_config.total_channel_number = 2;
    out_ch_config.resolution = RESOLUTION_16BIT;
    dongle_handle->mixer_member = stream_function_sw_mixer_member_create((void *)source,
                                                                         SW_MIXER_MEMBER_MODE_NO_BYPASS,
                                                                         &callback_config,
                                                                         &in_ch_config,
                                                                         &out_ch_config);
    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, true);
    /* do connections */
    stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle->mixer_member, 1);
    dongle_handle->mixer_channel_enable[0] = 0x1;
    stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle->mixer_member, 2);
    dongle_handle->mixer_channel_enable[1] = 0x2;
    // stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle->mixer_member, 2);
    // stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle->mixer_member, 1);
    BLE_LOG_I("[ble audio dongle][ul][config][SW_MIXER][scenario %d][handle 0x%x] %u, %u, %u, %u, %u, %u\r\n", 8,
                 source->scenario_type,
                 dongle_handle,
                 in_ch_config.total_channel_number,
                 in_ch_config.resolution,
                 in_ch_config.input_mode,
                 in_ch_config.buffer_size,
                 out_ch_config.total_channel_number,
                 out_ch_config.resolution);
#endif /* AIR_SOFTWARE_MIXER_ENABLE */

#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
    /* init lc3 codec */
    extern uint32_t g_plc_mode;
    lc3_dec_port_config_t lc3_dec_config;
    lc3_dec_config.sample_bits      = 16;
    lc3_dec_config.sample_rate      = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.sample_rate;
    lc3_dec_config.bit_rate         = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.bit_rate;
    lc3_dec_config.dec_mode         = C_LC3_Audio_Dec;
    lc3_dec_config.channel_mode     = LC3_DEC_CHANNEL_MODE_DUAL_MONO;
    lc3_dec_config.delay            = LC3_NO_DELAY_COMPENSATION;
    lc3_dec_config.in_channel_num   = 2;
    lc3_dec_config.out_channel_num  = 2;
    lc3_dec_config.frame_interval   = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval;
    lc3_dec_config.frame_size       = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_size;
    lc3_dec_config.frame_samples    = bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.sample_rate / 1000 * bt_common_open_param->scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval / 1000;
    lc3_dec_config.plc_enable       = 1;
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
    lc3_dec_config.plc_mode         = (g_plc_mode == 0)? LC3_PLC_DISABLE : LC3_PLC_ADVANCED;
#else
    lc3_dec_config.plc_mode         = (g_plc_mode == 0)? LC3_PLC_DISABLE : LC3_TPLC;
#endif
    stream_codec_decoder_lc3_v2_init(LC3_DEC_PORT_0, source, &lc3_dec_config);
    BLE_LOG_I("[ble audio dongle][ul][config][LC3 DEC][scenario %d][handle 0x%x] %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", 16,
                 source->scenario_type,
                 dongle_handle,
                 lc3_dec_config.sample_bits,
                 lc3_dec_config.sample_rate,
                 lc3_dec_config.bit_rate,
                 lc3_dec_config.dec_mode,
                 lc3_dec_config.channel_mode,
                 lc3_dec_config.delay,
                 lc3_dec_config.in_channel_num,
                 lc3_dec_config.out_channel_num,
                 lc3_dec_config.frame_interval,
                 lc3_dec_config.frame_size,
                 lc3_dec_config.frame_samples,
                 lc3_dec_config.plc_enable,
                 lc3_dec_config.plc_mode);
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* lock sleep because sleep wake-up time will consume the stream process time */
    hal_sleep_manager_lock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif
}

void ble_audio_dongle_ul_deinit(SOURCE source, SINK sink)
{
    ble_audio_dongle_ul_handle_t *dongle_handle = (ble_audio_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    UNUSED(sink);

    /* unregister ccni handler */
    bt_common_unregister_isr_handler(ble_audio_dongle_ul_ccni_handler);

#ifdef AIR_SOFTWARE_SRC_ENABLE
    stream_function_sw_src_deinit(dongle_handle->src_port);
#endif /* AIR_SOFTWARE_SRC_ENABLE */

    /* application deinit */
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    stream_function_sw_buffer_deinit(dongle_handle->buffer_port);
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */

#ifdef AIR_SOFTWARE_GAIN_ENABLE
    stream_function_sw_gain_deinit(dongle_handle->gain_port);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */

#ifdef AIR_SOFTWARE_MIXER_ENABLE
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
#endif /* AIR_SOFTWARE_MIXER_ENABLE */

    /* deinit codec */
#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
    stream_codec_decoder_lc3_v2_deinit(LC3_DEC_PORT_0, source);
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* unlock sleep */
    hal_sleep_manager_unlock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif

    /* release handle */
    ble_audio_dongle_release_ul_handle(dongle_handle);
    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle = NULL;
}

ATTR_TEXT_IN_IRAM void ble_audio_dongle_ul_start(SOURCE source)
{
    uint32_t saved_mask = 0;
    ble_audio_dongle_ul_handle_t *dongle_handle = (ble_audio_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    dongle_handle->fetch_flag = 0;
    dongle_handle->play_en_status = 0;
    dongle_handle->stream_status = BLE_AUDIO_DONGLE_UL_STREAM_INIT;
    dongle_handle->frame_count = 0;
    dongle_handle->frame_index = 0;
    dongle_handle->compen_samples = 0;
    dongle_handle->process_frames = 0;
    dongle_handle->drop_frames = 0;
    dongle_handle->seq_num = 0;

    hal_nvic_restore_interrupt_mask(saved_mask);

    BLE_LOG_I("[ble audio dongle][ul][start]", 0);
}

ATTR_TEXT_IN_IRAM_LEVEL_1 void ble_audio_dongle_ul_ccni_handler(hal_ccni_event_t event, void *msg)
{
    ble_audio_dongle_ul_handle_t *dongle_handle = ble_audio_dongle_first_ul_handle;
    SOURCE source;
    uint32_t saved_mask;
    UNUSED(event);
    UNUSED(msg);

    if (dongle_handle == NULL) {
        BLE_LOG_E("[ble audio dongle][ccni handler]first_ul_handle is NULL\r\n", 0);
        goto _ccni_return;
    }

    source = (SOURCE)(dongle_handle->owner);
    if ((source == NULL) || (source->transform == NULL)) {
        BLE_LOG_E("[ble audio dongle][ccni handler]source or transform is NULL\r\n", 0);
        goto _ccni_return;
    }

    if (dongle_handle->stream_status != BLE_AUDIO_DONGLE_UL_STREAM_DEINIT) {
        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
        /* set fetch flag to trigger stream flow */
        dongle_handle->fetch_flag = 1;
        hal_nvic_restore_interrupt_mask(saved_mask);

        /* Handler the stream */
        AudioCheckTransformHandle(source->transform);
    }

#if BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][ul][ccni_handler]: %d, %u, %d", 3, dongle_handle->stream_status, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG */

_ccni_return:
    return;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ bool ble_audio_dongle_ul_config(SOURCE source, stream_config_type type, U32 value)
{
    // uint32_t saved_mask = 0;
    uint32_t i;
    ble_audio_dongle_runtime_config_param_p config_param = (ble_audio_dongle_runtime_config_param_p)value;
    ble_audio_dongle_ul_handle_t *dongle_handle = (ble_audio_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    UNUSED(type);
    UNUSED(value);
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    int32_t new_gain;
    sw_gain_config_t old_config;
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    uint32_t channel_num;
    uint32_t main_channel_num;
#endif /* AIR_SOFTWARE_MIXER_ENABLE */
    UNUSED(dongle_handle);
    UNUSED(i);

    switch (config_param->config_operation) {
        case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_L:
#ifdef AIR_SOFTWARE_GAIN_ENABLE
            /* left channel */
            new_gain = config_param->config_param.vol_level.gain_1;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 1, &old_config);
            BLE_LOG_I("[ble audio dongle][ul][config]change channel%d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                         1,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, new_gain);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
            break;

        case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_R:
#ifdef AIR_SOFTWARE_GAIN_ENABLE
            /* right channel */
            new_gain = config_param->config_param.vol_level.gain_2;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 2, &old_config);
            BLE_LOG_I("[ble audio dongle][ul][config]change channel%d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                         2,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, new_gain);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
            break;

        case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_DUL:
#ifdef AIR_SOFTWARE_GAIN_ENABLE
            /* left channel */
            new_gain = config_param->config_param.vol_level.gain_1;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 1, &old_config);
            BLE_LOG_I("[ble audio dongle][ul][config]change channel%d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                         1,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, new_gain);

            /* right channel */
            new_gain = config_param->config_param.vol_level.gain_2;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 2, &old_config);
            BLE_LOG_I("[ble audio dongle][ul][config]change channel%d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                         2,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, new_gain);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
            break;

        case BLE_AUDIO_DONGLE_CONFIG_OP_SET_UL_CH1_INPUT_SOURCE:
        case BLE_AUDIO_DONGLE_CONFIG_OP_SET_UL_CH2_INPUT_SOURCE:
#ifdef AIR_SOFTWARE_MIXER_ENABLE
            // hal_nvic_save_and_set_interrupt_mask_special(&saved_mask);
            channel_num = config_param->config_operation - BLE_AUDIO_DONGLE_CONFIG_OP_SET_UL_CH1_INPUT_SOURCE + 1;
            /* disconnect all input channels of the output channel */
            for (i = 0; i < sizeof(uint32_t); i++) {
                if (dongle_handle->mixer_channel_enable[channel_num - 1] & (0x1 << i)) {
                    stream_function_sw_mixer_channel_disconnect(dongle_handle->mixer_member, i + 1, dongle_handle->mixer_member, channel_num);
                }
            }
            /* find out main channel */
            main_channel_num = 0;
            for (i = 0; i < sizeof(uint32_t); i++) {
                if (config_param->config_param.channel_enable & (0x1 << i)) {
                    if (main_channel_num == 0) {
                        main_channel_num = i + 1;
                    }
                    if (channel_num == (i + 1)) {
                        main_channel_num = i + 1;
                    }
                }
            }
            /* do new connections */
            if (main_channel_num != 0) {
                /* in here, we will connect all enabled BT channel into the choosed audio channel */
                for (i = 0; i < sizeof(uint32_t); i++) {
                    if (config_param->config_param.channel_enable & (0x1 << i)) {
                        if (main_channel_num == (i + 1)) {
                            stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, i + 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle->mixer_member, channel_num);
                        } else {
                            stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, i + 1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle->mixer_member, channel_num);
                        }
                    }
                }
            } else {
                /* in here, we need to connect the corresponding BT channel into the choosed audio channel because sw mixer requries that all output channels' output frame size must be the same */
                stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, channel_num, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle->mixer_member, channel_num);
            }
            // hal_nvic_restore_interrupt_mask_special(saved_mask);
            /* update state machine */
            BLE_LOG_I("[ble audio dongle][ul][config]operation %d: change channel%d status from 0x%x to 0x%x\r\n", 4,
                         config_param->config_operation,
                         channel_num,
                         dongle_handle->mixer_channel_enable[channel_num - 1],
                         config_param->config_param.channel_enable);
            if (main_channel_num == 0) {
                config_param->config_param.channel_enable = 0x1 << (channel_num - 1);
            }
            dongle_handle->mixer_channel_enable[channel_num - 1] = config_param->config_param.channel_enable;
#endif /* AIR_SOFTWARE_MIXER_ENABLE */
            break;

        default:
            BLE_LOG_E("[ble audio dongle][ul][config] unknow cmd:%d\r\n", 1, config_param->config_operation);
            break;
    }

    return true;
}

bool ble_audio_dongle_ul_source_get_avail_size(SOURCE source, uint32_t *avail_size)
{
    ble_audio_dongle_ul_handle_t *dongle_handle = (ble_audio_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);

    /* If there is fetch requirement, we must wake up the stream even there is no packet on the share buffer */
    if (dongle_handle->fetch_flag != 0) {
        *avail_size = ble_audio_ul_info.lc3_packet_frame_size;
    } else if ((dongle_handle->stream_status == BLE_AUDIO_DONGLE_UL_STREAM_RUNNING) &&
               (dongle_handle->play_en_status != 0) &&
               ble_audio_dongle_ul_fetch_time_is_arrived(dongle_handle)) {
        /* If there is data in the share buffer, we must wake up the stream to process it */
        *avail_size = ble_audio_ul_info.lc3_packet_frame_size;
    } else {
        *avail_size = 0;
    }

#if BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][ul][source][get_avail_size]: %u, %u, %d", 3, *avail_size, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ uint32_t ble_audio_dongle_ul_source_copy_payload(SOURCE source, uint8_t *dst_buf, uint32_t length)
{
    uint8_t *src_buf = NULL;
    uint32_t saved_mask;
    uint32_t remain_samples = 0;
    DSP_STREAMING_PARA_PTR stream = NULL;
    uint32_t total_frames = 0;
    uint32_t channel_has_packet = 0;
    uint32_t channel_packet_is_vaild = 0;
    uint32_t first_vaild_timestamp;
    ble_audio_dongle_ul_first_packet_status_t first_packet_status;
    uint32_t bt_clk;
    uint16_t bt_phase;
    ble_audio_dongle_ul_handle_t *dongle_handle = (ble_audio_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    // int32_t compensatory_samples = 0;
    uint16_t write_index;
    uint16_t read_index;
    uint32_t i;
    bool first_flag;
    uint32_t packet_anchor;
    ble_audio_dongle_ul_channel_info_t *current_channel_info;
    LE_AUDIO_HEADER *current_channel_header;
    uint32_t frame_num;
    uint32_t avail_size;
    bool find_out_suitable;
    bool packet_is_vaild;
    int32_t data_wave = 0;
    UNUSED(src_buf);
    UNUSED(dst_buf);

    /* update data receive timestamp */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.data_timestamp);

    /* get the stream */
    stream = DSP_Streaming_Get(source, source->transform->sink);

    /* dongle status check */
    switch (dongle_handle->stream_status) {
        /* In this status stage, it means the stream is not ready */
        case BLE_AUDIO_DONGLE_UL_STREAM_DEINIT:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->fetch_flag = 0;
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        /* In this status stage, we will update ReadIndex to the lastest WriteIndex and the old packets are dropped */
        case BLE_AUDIO_DONGLE_UL_STREAM_INIT:
            /* find out the suitable ReadIndex */
            first_flag = false;
            read_index = 0;
            for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
                if (current_channel_info->enable) {
                    /* get the total frames in this channel's share buffer */
                    vTaskSuspendAll();
                    StreamDSP_HWSemaphoreTake();
                    write_index = ble_audio_ul_info.p_share_buffer_info[i]->write_offset / ble_audio_ul_info.share_buffer_blk_size;
                    StreamDSP_HWSemaphoreGive();
                    xTaskResumeAll();
                    total_frames = (write_index + ble_audio_ul_info.share_buffer_blk_num - ble_audio_ul_info.share_buffer_blk_index) % (ble_audio_ul_info.share_buffer_blk_num);

                    if (total_frames == 0) {
                        /* there is no packet in this channel's share buffer */
                        channel_has_packet &= ~(0x1 << i);
                    } else {
                        channel_has_packet |= 0x1 << i;
                        if (first_flag == false) {
                            read_index = write_index;
                            first_flag = true;
                        } else {
                            if ((write_index < read_index) && (write_index != 0)) {
                                /* this channel's write_index is oldest */
                                read_index = write_index;
                            } else if ((write_index >= read_index) && (write_index != 0)) {
                                /* do nothing */
                            } else if (write_index == 0) {
                                AUDIO_ASSERT(0);
                            }
                        }
                    }
                }
            }
            total_frames = (channel_has_packet != 0) ? 1 : 0;
            /* check if there is any packet in all channel's share buffer */
            if (total_frames != 0) {
                /* update read_index info */
                ble_audio_ul_info.share_buffer_blk_index = read_index;
                ble_audio_ul_info.share_buffer_blk_index_previous = (read_index + ble_audio_ul_info.share_buffer_blk_num - 1) % ble_audio_ul_info.share_buffer_blk_num;

                /* update read index */
                vTaskSuspendAll();
                StreamDSP_HWSemaphoreTake();
                for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                    ble_audio_ul_info.p_share_buffer_info[i]->read_offset = ble_audio_ul_info.share_buffer_blk_index * ble_audio_ul_info.share_buffer_blk_size;
                }
                StreamDSP_HWSemaphoreGive();
                xTaskResumeAll();

                BLE_LOG_I("[ble audio dongle][ul]: there is new packet, r_index = %u, status = %u", 2,
                             ble_audio_ul_info.share_buffer_blk_index,
                             dongle_handle->stream_status);

                /* update stream status */
                dongle_handle->stream_status = BLE_AUDIO_DONGLE_UL_STREAM_START;
            }

            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->fetch_flag = 0;
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        /* In this status stage, we will check if the newest packet is suitable for playing */
        case BLE_AUDIO_DONGLE_UL_STREAM_START:
            read_index = ble_audio_ul_info.share_buffer_blk_index;
            for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
                if (current_channel_info->enable) {
                    /* get the total frames in this channel's share buffer */
                    vTaskSuspendAll();
                    StreamDSP_HWSemaphoreTake();
                    write_index = ble_audio_ul_info.p_share_buffer_info[i]->write_offset / ble_audio_ul_info.share_buffer_blk_size;
                    current_channel_header = (LE_AUDIO_HEADER *)(hal_memview_cm4_to_dsp0(ble_audio_ul_info.p_share_buffer_info[i]->start_addr) + read_index * ble_audio_ul_info.share_buffer_blk_size);
                    StreamDSP_HWSemaphoreGive();
                    xTaskResumeAll();
                    total_frames = (write_index + ble_audio_ul_info.share_buffer_blk_num - read_index) % (ble_audio_ul_info.share_buffer_blk_num);

                    /* get the total frames and anchor time in this channel's share buffer */
                    if (total_frames == 0) {
                        /* there is no packet in this channel's share buffer */
                        channel_has_packet &= ~(0x1 << i);
                    } else {
                        channel_has_packet |= 0x1 << i;
                        /* check if this packet is valid */
                        if ((current_channel_header->valid_packet == 0) || (current_channel_header->valid_packet == 0x3)) {
                            channel_packet_is_vaild &= ~(0x1 << i);
                            current_channel_info->channel_anchor = 0xf0000000;
                        } else if (current_channel_header->valid_packet == 0x1) {
                            channel_packet_is_vaild |= 0x1 << i;
                            // current_channel_header->valid_packet = 0;
                            current_channel_info->channel_anchor = current_channel_header->TimeStamp;
                        } else {
                            BLE_LOG_E("[ble audio dongle][ul][source][copy_payload]: ERROR ch_%d, base 0x%x header 0x%x error valid flag %d r_index %d time_stamp 0x%x anchor 0x%x", 7,
                                i,
                                hal_memview_cm4_to_dsp0(ble_audio_ul_info.p_share_buffer_info[i]->start_addr),
                                (uint32_t)current_channel_header,
                                current_channel_header->valid_packet,
                                ble_audio_ul_info.share_buffer_blk_index,
                                current_channel_header->TimeStamp,
                                current_channel_info->channel_anchor
                                );
                            #if defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_PREMIUM_G3)
                                AUDIO_ASSERT(0);
                            #else
                                /* treat as invalid packet for STEREO_G3 Chip series */
                                channel_packet_is_vaild &= ~(0x1 << i);
                                current_channel_info->channel_anchor = 0xf0000000;
                            #endif
                        }
#if BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG
                        BLE_LOG_E("[ble audio dongle][ul][source][copy_payload]: ch_%d, base 0x%x header 0x%x valid %d r_index %d time_stamp 0x%x anchor 0x%x", 7,
                            i,
                            hal_memview_cm4_to_dsp0(ble_audio_ul_info.p_share_buffer_info[i]->start_addr),
                            (uint32_t)current_channel_header,
                            current_channel_header->valid_packet,
                            ble_audio_ul_info.share_buffer_blk_index,
                            current_channel_header->TimeStamp,
                            current_channel_info->channel_anchor
                            );
#endif /* BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG */
                    }
                }
            }
            total_frames = (channel_has_packet != 0) ? 1 : 0;
            /* init invalid channel's anchor */
            for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
                if ((current_channel_info->enable) && ((channel_packet_is_vaild & (0x1 << i)) != 0)) {
                    first_vaild_timestamp = current_channel_info->channel_anchor;
                }
            }
            for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
                if ((current_channel_info->enable) && ((channel_packet_is_vaild & (0x1 << i)) == 0)) {
                    /* because this channle's timestamp is invalid, so we use other channel timestamp as the init anchor */
                    current_channel_info->channel_anchor = first_vaild_timestamp;
                }
            }
            if ((total_frames != 0) && (channel_packet_is_vaild == 0)) {
                /* this packet in all channel is invalid, so we need to drop this packet */
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                dongle_handle->fetch_flag = 0;
                hal_nvic_restore_interrupt_mask(saved_mask);

                /* get current bt clock */
                MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

                /* drop this packet */
                ble_audio_ul_info.share_buffer_blk_index = (ble_audio_ul_info.share_buffer_blk_index + 1) % ble_audio_ul_info.share_buffer_blk_num;
                vTaskSuspendAll();
                StreamDSP_HWSemaphoreTake();
                for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                    ble_audio_ul_info.p_share_buffer_info[i]->read_offset = ble_audio_ul_info.share_buffer_blk_index * ble_audio_ul_info.share_buffer_blk_size;
                }
                StreamDSP_HWSemaphoreGive();
                xTaskResumeAll();

                BLE_LOG_I("[ble audio dongle][ul]: this first packet is invalid, current bt clk = 0x%x, r_index = %u, status = %u", 3,
                             bt_clk,
                             ble_audio_ul_info.share_buffer_blk_index,
                             dongle_handle->stream_status);
            } else if (total_frames != 0) {
                /* find out the stream anchor */
                ble_audio_dongle_ul_get_stream_anchor(dongle_handle, &packet_anchor);
                dongle_handle->stream_anchor = packet_anchor;
                dongle_handle->stream_anchor_previous = (packet_anchor + 0x10000000 - dongle_handle->bt_interval) & 0x0fffffff;
                /* check if if the newest packet is suitable for playing */
                first_packet_status = ble_audio_dongle_ul_first_packet_check(dongle_handle, packet_anchor);
                switch (first_packet_status) {
                    case BLE_AUDIO_DONGLE_UL_FIRST_PACKET_NOT_READY:
                        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                        dongle_handle->fetch_flag = 0;
                        hal_nvic_restore_interrupt_mask(saved_mask);
                        break;

                    case BLE_AUDIO_DONGLE_UL_FIRST_PACKET_TIMEOUT:
                        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                        dongle_handle->fetch_flag = 0;
                        hal_nvic_restore_interrupt_mask(saved_mask);

                        /* get current bt clock */
                        MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

                        /* drop this packet */
                        ble_audio_ul_info.share_buffer_blk_index = (ble_audio_ul_info.share_buffer_blk_index + 1) % ble_audio_ul_info.share_buffer_blk_num;
                        vTaskSuspendAll();
                        StreamDSP_HWSemaphoreTake();
                        for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                            ble_audio_ul_info.p_share_buffer_info[i]->read_offset = ble_audio_ul_info.share_buffer_blk_index * ble_audio_ul_info.share_buffer_blk_size;
                        }
                        StreamDSP_HWSemaphoreGive();
                        xTaskResumeAll();

                        BLE_LOG_I("[ble audio dongle][ul]: this first packet is dropped, anchor = 0x%x, current bt clk = 0x%x, r_index = %u, status = %u", 4,
                                     packet_anchor,
                                     bt_clk,
                                     ble_audio_ul_info.share_buffer_blk_index,
                                     dongle_handle->stream_status);
                        break;

                    case BLE_AUDIO_DONGLE_UL_FIRST_PACKET_READY:
                        // hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                        // dongle_handle->fetch_flag = 0;
                        // hal_nvic_restore_interrupt_mask(saved_mask);

                        /* update stream status */
                        dongle_handle->stream_status = BLE_AUDIO_DONGLE_UL_STREAM_RUNNING;

                        /* get current bt clock */
                        MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

                        /* first_packet_bt_clk should be later than first_anchor_bt_clk */
                        dongle_handle->first_packet_bt_clk = bt_clk;
                        /* first_anchor_bt_clk is the packet sent time on headset/earbuds side */
                        dongle_handle->first_anchor_bt_clk = packet_anchor;
                        if ((dongle_handle->first_anchor_bt_clk + dongle_handle->play_en_delay) > 0x0fffffff) {
                            dongle_handle->play_en_overflow = 1;
                        } else {
                            dongle_handle->play_en_overflow = 0;
                        }
                        /* play_en_bt_clk is the play time: 1 anchor + 1.25ms jitter */
                        dongle_handle->play_en_bt_clk = (dongle_handle->first_anchor_bt_clk + dongle_handle->play_en_delay) & 0x0fffffff;

                        BLE_LOG_I("[ble audio dongle][ul]: play will at 0x%x, first anchor = 0x%x, cur bt clk = 0x%x, r_index = %u, w_index = %u, status = %u", 6,
                                     dongle_handle->play_en_bt_clk,
                                     dongle_handle->first_anchor_bt_clk,
                                     bt_clk,
                                     ble_audio_ul_info.share_buffer_blk_index,
                                     write_index,
                                     dongle_handle->stream_status);
                        break;
                }
            }
            break;

        /* In this status stage, the stream is started and we will set flag to fetch a new packet */
        case BLE_AUDIO_DONGLE_UL_STREAM_RUNNING:
            break;

        /* Error status */
        default:
            AUDIO_ASSERT(0);
            break;
    }

    /* check if the play is started */
    if ((dongle_handle->stream_status == BLE_AUDIO_DONGLE_UL_STREAM_RUNNING) && (dongle_handle->play_en_status == 0)) {
        if (ble_audio_dongle_ul_play_time_is_arrived(dongle_handle)) {
            dongle_handle->play_en_status = 1;
            /* prefill 3ms to sink buffer for usb */
            audio_transmitter_share_information_fetch(NULL, source->transform->sink);
            SINK sink = source->transform->sink;
            for (uint32_t prefill_cnt = 0; prefill_cnt < UL_USB_OUT_PREFILL_MS; prefill_cnt++) {
                uint8_t *dst_buf = (uint8_t *)(sink->streamBuffer.ShareBufferInfo.start_addr + sink->streamBuffer.ShareBufferInfo.write_offset);
                /* write seq number and payload_len into the share buffer */
                ((audio_transmitter_frame_header_t *)dst_buf)->seq_num      = dongle_handle->seq_num;
                if (sink->param.data_ul.scenario_param.usb_out_broadcast_param.usb_out_param.codec_param.pcm.channel_mode == 2) {
                    ((audio_transmitter_frame_header_t *)dst_buf)->payload_len = dongle_handle->usb_frame_samples * 2 * sizeof(int16_t);
                } else {
                    ((audio_transmitter_frame_header_t *)dst_buf)->payload_len = dongle_handle->usb_frame_samples * 1 * sizeof(int16_t);
                }
                /* update seq number */
                dongle_handle->seq_num = (dongle_handle->seq_num + 1) & 0xffff;
                sink->streamBuffer.ShareBufferInfo.write_offset = (sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size)
                    % (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num);
            }
            audio_transmitter_share_information_update_write_offset(sink, sink->streamBuffer.ShareBufferInfo.write_offset);
            /* get current bt clock */
            MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);
            BLE_LOG_I("[ble audio dongle][ul]: play at now 0x%x, r_index = %u, w_index = %u, status = %u", 4,
                         bt_clk,
                         ble_audio_ul_info.share_buffer_blk_index,
                         source->streamBuffer.AVMBufferInfo.WriteIndex,
                         dongle_handle->stream_status);
        } else if (dongle_handle->fetch_flag) {
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->fetch_flag = 0;
            hal_nvic_restore_interrupt_mask(saved_mask);
        }
    }

    /* process the frame after the play is started */
    dongle_handle->drop_frames = 0;
    if ((dongle_handle->stream_status == BLE_AUDIO_DONGLE_UL_STREAM_RUNNING) && (dongle_handle->play_en_status != 0)) {
        /* check if the new packet is timeout */
        if ((ble_audio_dongle_ul_fetch_time_is_arrived(dongle_handle) == true) && (ble_audio_dongle_ul_fetch_time_is_timeout(dongle_handle) == true)) {
            /* the new packet has be timeout at now, we need to find out a suitbale packet */
            find_out_suitable = false;
            while (find_out_suitable == false) {
                /* drop this packet */
                dongle_handle->drop_frames += 1;
                dongle_handle->stream_anchor_previous = dongle_handle->stream_anchor;
                dongle_handle->stream_anchor = (dongle_handle->stream_anchor + dongle_handle->bt_interval) & 0x0fffffff;
                for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                    current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
                    if (current_channel_info->enable) {
                        current_channel_info->channel_anchor = (current_channel_info->channel_anchor + dongle_handle->bt_interval) & 0x0fffffff;
                    }
                }

                /* check the next packet if it is ok */
                if (ble_audio_dongle_ul_fetch_time_is_arrived(dongle_handle) == false) {
                    find_out_suitable = true;
                } else if (ble_audio_dongle_ul_fetch_time_is_timeout(dongle_handle) == false) {
                    find_out_suitable = true;
                }
            }
            ble_audio_ul_info.share_buffer_blk_index_previous = ble_audio_ul_info.share_buffer_blk_index;
            ble_audio_ul_info.share_buffer_blk_index = (ble_audio_ul_info.share_buffer_blk_index + dongle_handle->drop_frames) % ble_audio_ul_info.share_buffer_blk_num;
            BLE_LOG_E("[ble audio dongle][ul][source][copy_payload]: packet timeout, %u, %u, %u, %u", 4,
                dongle_handle->drop_frames,
                ble_audio_ul_info.share_buffer_blk_index_previous,
                ble_audio_ul_info.share_buffer_blk_index,
                dongle_handle->stream_anchor
                );
            dongle_handle->drop_frames = 0;
            /* clear the drop packets' valid flag */
            read_index = ble_audio_ul_info.share_buffer_blk_index_previous;
            while (read_index != ble_audio_ul_info.share_buffer_blk_index) {
                for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                    current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
                    if (current_channel_info->enable) {
                        /* get this channel's LE AUDIO header */
                        current_channel_header = (LE_AUDIO_HEADER *)(hal_memview_cm4_to_dsp0(ble_audio_ul_info.p_share_buffer_info[i]->start_addr) + read_index * ble_audio_ul_info.share_buffer_blk_size);
                        current_channel_header->valid_packet = 0;
                    }
                }
                read_index = (read_index + 1) % ble_audio_ul_info.share_buffer_blk_num;
            }
        }

        /* update each channel's data status */
        if ((ble_audio_dongle_ul_fetch_time_is_arrived(dongle_handle) == true) && (ble_audio_dongle_ul_fetch_time_is_timeout(dongle_handle) == false)) {
            /* the new packet shuold be arrived at now */
            for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
                if (current_channel_info->enable) {
                    /* get this channel's LE AUDIO header */
                    current_channel_header = (LE_AUDIO_HEADER *)(hal_memview_cm4_to_dsp0(ble_audio_ul_info.p_share_buffer_info[i]->start_addr) + ble_audio_ul_info.share_buffer_blk_index * ble_audio_ul_info.share_buffer_blk_size);
                    /* check if this packet is valid */
                    if ((current_channel_header->valid_packet == 0) || (current_channel_header->valid_packet == 0x3)) {
                        packet_is_vaild = false;
                    } else if (current_channel_header->valid_packet == 0x1) {
                        packet_is_vaild = true;
                        current_channel_header->valid_packet = 0;
                    } else {
                        BLE_LOG_E("[ble audio dongle][ul][source][copy_payload]: ch %d error vaild flag %d addr 0x%x", 3,
                            i,
                            current_channel_header->valid_packet,
                            (uint32_t)current_channel_header
                            );
                        AUDIO_ASSERT(0);
                    }
                    /* check if the packet on the share memory is right */
                    if (packet_is_vaild == false) {
                        /* this packet is invalid, do PLC */
                        current_channel_info->status = BLE_AUDIO_DONGLE_UL_DATA_PLC;
                        // BLE_LOG_E("[ble audio dongle][ul][source][copy_payload]: packet is invalid, %u", 1, current_channel_info->channel_anchor);
                    } else if (ble_audio_dongle_ul_fetch_timestamp_is_correct(dongle_handle, current_channel_info->channel_anchor, current_channel_header->TimeStamp) == false) {
                        /* timestamp is not right, do PLC */
                        current_channel_info->status = BLE_AUDIO_DONGLE_UL_DATA_PLC;
                        BLE_LOG_E("[ble audio dongle][ul][source][copy_payload]: error timestamp, 0x%x, 0x%x", 2, current_channel_header->TimeStamp, current_channel_info->channel_anchor);
                    } else {
                        /* anchor is right, use this packet */
                        current_channel_info->status = BLE_AUDIO_DONGLE_UL_DATA_NORMAL;
                        current_channel_info->channel_anchor = current_channel_header->TimeStamp;
                        if (current_channel_header->PduLen != ble_audio_ul_info.lc3_packet_frame_size) {
                            BLE_LOG_E("[ble audio dongle][ul][source][copy_payload]: error size, %u, %u", 2, current_channel_header->PduLen, ble_audio_ul_info.lc3_packet_frame_size);
                            AUDIO_ASSERT(0);
                        }
                    }
                    /* update channel anchor */
                    current_channel_info->channel_anchor = (current_channel_info->channel_anchor + dongle_handle->bt_interval) & 0x0fffffff;
                } else {
                    current_channel_info->status = BLE_AUDIO_DONGLE_UL_DATA_PLC;
                }
            }
            /* increase 1 to drop_frames, it will cause the read index update in later */
            dongle_handle->drop_frames += 1;
            /* update the next anchor */
            dongle_handle->stream_anchor_previous = dongle_handle->stream_anchor;
            dongle_handle->stream_anchor = (dongle_handle->stream_anchor + dongle_handle->bt_interval) & 0x0fffffff;
        } else if (dongle_handle->fetch_flag != 0) {
            /* the new packet is not arrived at now, but there is fetch requirment */
            remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);
            if ((int32_t)remain_samples >= dongle_handle->usb_min_start_frames * dongle_handle->usb_frame_samples) {
                /* the remain data is enough, so we just need to bypass decoder */
                for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                    current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
                    current_channel_info->status = BLE_AUDIO_DONGLE_UL_DATA_BYPASS_DECODER;
                }
            } else {
                /* the remain data is not enough, so we need to do PLC */
                for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                    current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
                    current_channel_info->status = BLE_AUDIO_DONGLE_UL_DATA_PLC;
                }
            }
        } else {
            /* there is no fetch requirment, so we just need to bypass decoder */
            for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
                current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
                current_channel_info->status = BLE_AUDIO_DONGLE_UL_DATA_BYPASS_DECODER;
            }
        }

#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
        /* copy different data into the different stream buffers based on the channel's data status */
        for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
            current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
            dst_buf = (uint8_t *)stream->callback.EntryPara.in_ptr[i];
            if (current_channel_info->status == BLE_AUDIO_DONGLE_UL_DATA_NORMAL) {
                /* set decoder do process flag */
                *((lc3_dec_frame_status_t *)dst_buf) = LC3_DEC_FRAME_STATUS_NORMAL;

                /* copy data into the stream buffers for decoder */
                src_buf = (uint8_t *)(hal_memview_cm4_to_dsp0(ble_audio_ul_info.p_share_buffer_info[i]->start_addr) + ble_audio_ul_info.share_buffer_blk_index * ble_audio_ul_info.share_buffer_blk_size + sizeof(LE_AUDIO_HEADER));
                memcpy(dst_buf + sizeof(lc3_dec_frame_status_t), src_buf, ble_audio_ul_info.lc3_packet_frame_size);

#ifdef AIR_AUDIO_DUMP_ENABLE
#if BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP
                // LOG_AUDIO_DUMP(dst_buf, ble_audio_ul_info.lc3_packet_frame_size + sizeof(lc3_dec_frame_status_t), SOURCE_IN4 + i);
                LOG_AUDIO_DUMP(dst_buf + sizeof(lc3_dec_frame_status_t), ble_audio_ul_info.lc3_packet_frame_size, SOURCE_IN4 + i);
#endif /* BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */
            } else if (current_channel_info->status == BLE_AUDIO_DONGLE_UL_DATA_PLC) {
                /* set decoder do PLC flag */
                *((lc3_dec_frame_status_t *)dst_buf) = LC3_DEC_FRAME_STATUS_PLC;

#ifdef AIR_AUDIO_DUMP_ENABLE
#if BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP
                // LOG_AUDIO_DUMP(dst_buf, ble_audio_ul_info.lc3_packet_frame_size + sizeof(lc3_dec_frame_status_t), SOURCE_IN4 + i);
                uint8_t zero_data_buf[256] = {0};
                LOG_AUDIO_DUMP(zero_data_buf, ble_audio_ul_info.lc3_packet_frame_size, SOURCE_IN4 + i);
#endif /* BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */
            } else {
                /* set bypass decoder flag */
                *((lc3_dec_frame_status_t *)dst_buf) = LC3_DEC_FRAME_STATUS_BYPASS_DECODER;
            }
        }
        /* update stream input size */
        stream->callback.EntryPara.in_size = ble_audio_ul_info.lc3_packet_frame_size + sizeof(lc3_dec_frame_status_t);
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
        /* do clock skew */
        if (ble_audio_ul_info.channel_enable & 0x1) {
            current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[0]);
        } else {
            current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[1]);
        }
        if (current_channel_info->status != BLE_AUDIO_DONGLE_UL_DATA_BYPASS_DECODER) {
            /* get remain samples */
            remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);
            if (ble_audio_ul_info.lc3_packet_frame_interval == 7500) {
                if (dongle_handle->fetch_flag != 0) {
                    if (dongle_handle->usb_min_start_frames == 8) {
                        data_wave = (dongle_handle->usb_min_start_frames * dongle_handle->usb_frame_samples);
                        if ((remain_samples + dongle_handle->src_out_frame_samples - data_wave) > dongle_handle->clk_skew_watermark_samples) {
                            dongle_handle->compen_samples = -1;
                        } else if ((remain_samples + dongle_handle->src_out_frame_samples - data_wave) < dongle_handle->clk_skew_watermark_samples) {
                            dongle_handle->compen_samples = 1;
                        } else {
                            dongle_handle->compen_samples = 0;
                        }
                    } else {
                        data_wave = (dongle_handle->usb_min_start_frames * dongle_handle->usb_frame_samples);
                        if ((remain_samples + dongle_handle->src_out_frame_samples - data_wave) > (dongle_handle->clk_skew_watermark_samples + dongle_handle->usb_frame_samples / 2)) {
                            dongle_handle->compen_samples = -1;
                        } else if ((remain_samples + dongle_handle->src_out_frame_samples - data_wave) < (dongle_handle->clk_skew_watermark_samples + dongle_handle->usb_frame_samples / 2)) {
                            dongle_handle->compen_samples = 1;
                        } else {
                            dongle_handle->compen_samples = 0;
                        }
                    }
                } else {
                    dongle_handle->compen_samples = 0;
                }
            } else {
                if (dongle_handle->fetch_flag != 0) {
                    total_frames = 0;
                } else {
                    total_frames = 1;
                }
                if ((remain_samples + total_frames * dongle_handle->src_out_frame_samples) > dongle_handle->clk_skew_watermark_samples) {
                    dongle_handle->compen_samples = -1;
                } else if ((remain_samples + total_frames * dongle_handle->src_out_frame_samples) < dongle_handle->clk_skew_watermark_samples) {
                    dongle_handle->compen_samples = 1;
                } else {
                    dongle_handle->compen_samples = 0;
                }
            }
            /* do compensatory */
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, dongle_handle->compen_samples);
        } else {
            dongle_handle->compen_samples = 0;
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, dongle_handle->compen_samples);
        }
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */
    } else {
#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
        /* copy different data into the different stream buffers based on the channel's data status */
        for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
            dst_buf = (uint8_t *)stream->callback.EntryPara.in_ptr[i];
            /* set bypass decoder flag */
            *((lc3_dec_frame_status_t *)dst_buf) = LC3_DEC_FRAME_STATUS_BYPASS_DECODER;
        }
        /* update stream input size */
        stream->callback.EntryPara.in_size = ble_audio_ul_info.lc3_packet_frame_size + sizeof(lc3_dec_frame_status_t);
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
        dongle_handle->compen_samples = 0;
        stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, dongle_handle->compen_samples);
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */
    }

    /* config output */
    audio_transmitter_share_information_fetch(NULL, source->transform->sink);
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->fetch_flag != 0) {
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
        if (dongle_handle->play_en_status != 0) {
            /* get frame numbers in the sink share buffer */
            if (source->transform->sink->streamBuffer.ShareBufferInfo.read_offset < source->transform->sink->streamBuffer.ShareBufferInfo.write_offset) {
                /* normal case */
                avail_size = source->transform->sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->transform->sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num
                             - source->transform->sink->streamBuffer.ShareBufferInfo.write_offset
                             + source->transform->sink->streamBuffer.ShareBufferInfo.read_offset;
            } else if (source->transform->sink->streamBuffer.ShareBufferInfo.read_offset == source->transform->sink->streamBuffer.ShareBufferInfo.write_offset) {
                if (source->transform->sink->streamBuffer.ShareBufferInfo.bBufferIsFull == true) {
                    /* buffer is full, so ReadOffset == WriteOffset */
                    avail_size = 0;
                } else {
                    /* buffer is empty, so ReadOffset == WriteOffset */
                    avail_size = source->transform->sink->streamBuffer.ShareBufferInfo.length;
                }
            } else {
                /* buffer wrapper case */
                avail_size = source->transform->sink->streamBuffer.ShareBufferInfo.read_offset - source->transform->sink->streamBuffer.ShareBufferInfo.write_offset;
            }
            frame_num = (source->transform->sink->streamBuffer.ShareBufferInfo.length - avail_size) / source->transform->sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
            dongle_handle->frame_num = frame_num;
            if (frame_num <= UL_OUTPUT_MIN_FRAMES) {
                /* only output 10ms*32K data to usb at every ccni interrupt trigger after the play is started*/
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, (dongle_handle->usb_min_start_frames * dongle_handle->usb_frame_samples)*sizeof(int16_t));
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, (dongle_handle->usb_min_start_frames * dongle_handle->usb_frame_samples)*sizeof(int16_t));
                dongle_handle->buffer_output_size = (dongle_handle->usb_min_start_frames * dongle_handle->usb_frame_samples);
            } else {
                /* do not output data to usb because there are enough data in the sink buffer */
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, 0);
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, 0);
                dongle_handle->buffer_output_size = 0;
            }
        } else {
            /* do not output data to usb at every ccni interrupt trigger because the play is not started*/
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, 0);
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, 0);
            dongle_handle->buffer_output_size = 0;
        }
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */

        /* decrease fetch flag */
        dongle_handle->fetch_flag = 0;
    } else {
        dongle_handle->frame_num = 0;
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
        /* keep these data in buffer and not output them to usb */
        stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, 0);
        stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, 0);
        dongle_handle->buffer_output_size = 0;
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

#if BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][ul][source][copy_payload]: %u, %u, %d", 3, length, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG */

    return length;
}

bool ble_audio_dongle_ul_source_get_new_read_offset(SOURCE source, U32 amount, U32 *new_read_offset)
{
    ble_audio_dongle_ul_handle_t *dongle_handle = (ble_audio_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    uint32_t i;
    UNUSED(amount);
    UNUSED(new_read_offset);

    if (dongle_handle->stream_status == BLE_AUDIO_DONGLE_UL_STREAM_RUNNING) {
        /* there is no the packet lost, so needs to update the read offset */
        ble_audio_ul_info.share_buffer_blk_index_previous = ble_audio_ul_info.share_buffer_blk_index;
        ble_audio_ul_info.share_buffer_blk_index = (ble_audio_ul_info.share_buffer_blk_index + dongle_handle->drop_frames) % ble_audio_ul_info.share_buffer_blk_num;
    } else {
        /* do not update read index */
        ble_audio_ul_info.share_buffer_blk_index = ble_audio_ul_info.share_buffer_blk_index;
    }

    /* update read index */
    vTaskSuspendAll();
    StreamDSP_HWSemaphoreTake();
    for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
        ble_audio_ul_info.p_share_buffer_info[i]->read_offset = ble_audio_ul_info.share_buffer_blk_index * ble_audio_ul_info.share_buffer_blk_size;
    }
    StreamDSP_HWSemaphoreGive();
    xTaskResumeAll();

    /* we will update the read offsets of the different share buffers in here directly, so return false to aviod the upper layer update read offset */
    return false;
}

void ble_audio_dongle_ul_source_drop_postprocess(SOURCE source, uint32_t amount)
{
    ble_audio_dongle_ul_handle_t *dongle_handle = (ble_audio_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    ble_audio_dongle_ul_channel_info_t *current_channel_info;
    ble_audio_dongle_ul_data_status_t status[BLE_AUDIO_DATA_CHANNEL_NUMBER];
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t i;
    uint32_t remain_samples;
    #ifdef AIR_AUDIO_DUMP_ENABLE
    #if BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP
    DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(source, source->transform->sink);
    #endif
    #endif
    UNUSED(amount);

    /* get current bt clock */
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

    /* update the next channel anchor */
    for (i = 0; i < BLE_AUDIO_DATA_CHANNEL_NUMBER; i++) {
        current_channel_info = (ble_audio_dongle_ul_channel_info_t *) & (dongle_handle->channel_info[i]);
        status[i] = current_channel_info->status;
    }

    remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);

    BLE_LOG_I("[ble audio dongle][ul][source_drop_postprocess] handle 0x%x: %d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x\r\n", 12,
                 dongle_handle,
                 ble_audio_ul_info.share_buffer_blk_index_previous,
                 dongle_handle->buffer_output_size,
                 dongle_handle->process_frames,
                 remain_samples,
                 dongle_handle->compen_samples,
                 dongle_handle->frame_num,
                 dongle_handle->stream_status,
                 status[0],
                 status[1],
                 dongle_handle->stream_anchor_previous,
                 bt_clk);
    if (dongle_handle->buffer_output_size != 0) {
        if (dongle_handle->buffer_output_first_time == 0) {
            // dongle_handle->usb_min_start_frames -= 5;
            dongle_handle->buffer_output_first_time = 1;
        }
        if (dongle_handle->usb_min_start_frames == 7) {
            dongle_handle->usb_min_start_frames = 8;
        } else if (dongle_handle->usb_min_start_frames == 8) {
            dongle_handle->usb_min_start_frames = 7;
        }

        if (source->transform->sink->param.data_ul.scenario_param.usb_out_broadcast_param.usb_out_param.codec_param.pcm.channel_mode == 2) {
#ifdef AIR_AUDIO_DUMP_ENABLE
#if BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP
            LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[0]), dongle_handle->process_frames * dongle_handle->usb_frame_samples * sizeof(int16_t), VOICE_TX_MIC_1);
#endif /* BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */
        } else {
            if (ble_audio_ul_info.channel_enable & 0x1) {
#ifdef AIR_AUDIO_DUMP_ENABLE
#if BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP
                LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[0]), dongle_handle->process_frames * dongle_handle->usb_frame_samples * sizeof(int16_t), VOICE_TX_MIC_1);
#endif /* BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */
            } else {
#ifdef AIR_AUDIO_DUMP_ENABLE
#if BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP
                LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.out_ptr[1]), dongle_handle->process_frames * dongle_handle->usb_frame_samples * sizeof(int16_t), VOICE_TX_MIC_1);
#endif /* BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */
            }
        }
    }
}

bool ble_audio_dongle_ul_source_close(SOURCE source)
{
    UNUSED(source);

#if BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][ul][source_close]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG */

    return true;
}

bool ble_audio_dongle_ul_sink_get_avail_size(SINK sink, uint32_t *avail_size)
{
    uint32_t frame_num;
    ble_audio_dongle_ul_handle_t *dongle_handle = (ble_audio_dongle_ul_handle_t *)(sink->transform->source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);

    if (sink->streamBuffer.ShareBufferInfo.read_offset < sink->streamBuffer.ShareBufferInfo.write_offset) {
        /* normal case */
        *avail_size = sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num
                      - sink->streamBuffer.ShareBufferInfo.write_offset
                      + sink->streamBuffer.ShareBufferInfo.read_offset;
    } else if (sink->streamBuffer.ShareBufferInfo.read_offset == sink->streamBuffer.ShareBufferInfo.write_offset) {
        if (sink->streamBuffer.ShareBufferInfo.bBufferIsFull == true) {
            /* buffer is full, so ReadOffset == WriteOffset */
            *avail_size = 0;
        } else {
            /* buffer is empty, so ReadOffset == WriteOffset */
            *avail_size = sink->streamBuffer.ShareBufferInfo.length;
        }
    } else {
        /* buffer wrapper case */
        *avail_size = sink->streamBuffer.ShareBufferInfo.read_offset - sink->streamBuffer.ShareBufferInfo.write_offset;
    }

    frame_num = *avail_size / sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
    if (frame_num < (dongle_handle->usb_min_start_frames / dongle_handle->usb_frame_samples) &&
        (dongle_handle->fetch_flag != 0)) {
        /* in each time, there must be at least 10ms data free space in the share buffer */
        AUDIO_ASSERT(0);
    }

#if BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][ul][sink][get_avail_size]: %u, %u, %d", 3, *avail_size, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG */

    return true;
}

uint32_t ble_audio_dongle_ul_sink_copy_payload(SINK sink, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    TRANSFORM transform = sink->transform;
    DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(transform->source, sink);
    UNUSED(src_buf);
    uint32_t i;
    ble_audio_dongle_ul_handle_t *dongle_handle = (ble_audio_dongle_ul_handle_t *)(transform->source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    uint32_t payload_size = 0;

    // for 7.5ms can`t use this rule.
    // if (length != dongle_handle->src_out_frame_samples*sizeof(int16_t))
    // {
    //     BLE_LOG_I("[ble audio dongle][ul][sink][copy_payload]: error size %u, %u", 2, length, dongle_handle->src_out_frame_samples*sizeof(int16_t));
    //     AUDIO_ASSERT(0);
    // }

    /* copy pcm data into the share buffer one by one 1ms */
    dongle_handle->process_frames = ((length / sizeof(int16_t)) / dongle_handle->usb_frame_samples);
    for (i = 0; i < dongle_handle->process_frames; i++) {
        /* get dst buffer */
        dst_buf = (uint8_t *)(sink->streamBuffer.ShareBufferInfo.start_addr +
                              ((sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * i)
                               % (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num)));

        /* copy pcm samples into the share buffer */
        if (sink->param.data_ul.scenario_param.usb_out_broadcast_param.usb_out_param.codec_param.pcm.channel_mode == 2) {
            DSP_D2I_BufferCopy_16bit((U16 *)(dst_buf + sizeof(audio_transmitter_frame_header_t)),
                                     (U16 *)(stream->callback.EntryPara.out_ptr[0]) + i * dongle_handle->usb_frame_samples,
                                     (U16 *)(stream->callback.EntryPara.out_ptr[1]) + i * dongle_handle->usb_frame_samples,
                                     dongle_handle->usb_frame_samples);
            payload_size = dongle_handle->usb_frame_samples * 2 * sizeof(int16_t);
        } else {
            if (ble_audio_ul_info.channel_enable & 0x1) {
                memcpy(dst_buf + sizeof(audio_transmitter_frame_header_t), (U16 *)(stream->callback.EntryPara.out_ptr[0]) + i * dongle_handle->usb_frame_samples, dongle_handle->usb_frame_samples * sizeof(int16_t));
            } else {
                memcpy(dst_buf + sizeof(audio_transmitter_frame_header_t), (U16 *)(stream->callback.EntryPara.out_ptr[1]) + i * dongle_handle->usb_frame_samples, dongle_handle->usb_frame_samples * sizeof(int16_t));
            }
            payload_size = dongle_handle->usb_frame_samples * sizeof(int16_t);
        }

        /* write seq number and payload_len into the share buffer */
        ((audio_transmitter_frame_header_t *)dst_buf)->seq_num      = dongle_handle->seq_num;
        ((audio_transmitter_frame_header_t *)dst_buf)->payload_len  = payload_size;

        /* update seq number */
        dongle_handle->seq_num = (dongle_handle->seq_num + 1) & 0xffff;
    }

#if BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][ul][sink][copy_payload]: %u, %u, %u, %d", 4, length, sink->streamBuffer.ShareBufferInfo.write_offset, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG */

    /* we will update the seq number and payload_len in here directly, so return 0 to aviod the upper layer update them */
    return payload_size;
}

void ble_audio_dongle_ul_sink_query_write_offset(SINK sink, uint32_t *write_offset)
{
    ble_audio_dongle_ul_handle_t *dongle_handle = (ble_audio_dongle_ul_handle_t *)(sink->transform->source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);

    *write_offset = (sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * (dongle_handle->usb_min_start_frames))
                    % (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num);

#if BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][ul][sink][query_write_offset]: %u, %u, %u, %d", 4, *write_offset, sink->streamBuffer.ShareBufferInfo.write_offset, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG */
}

bool ble_audio_dongle_ul_sink_close(SINK sink)
{
    UNUSED(sink);

#if BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BLE_LOG_I("[ble audio dongle][ul][sink_close]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* BLE_AUDIO_DONGLE_VOCIE_PATH_DEBUG_LOG */

    return true;
}

void ble_audio_dongle_dl_start(SOURCE source, SINK sink)
{
    ble_audio_dongle_dl_handle_t *dongle_handle = (ble_audio_dongle_dl_handle_t *)(sink->param.bt_common.scenario_param.dongle_handle);
    switch (source->scenario_type) {
#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
            ble_audio_dongle_dl_afe_in_start(dongle_handle);
            break;
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
            ble_audio_dongle_dl_afe_in_start(dongle_handle);
            break;
#endif
        default:
            break;
    }
    dongle_handle->stream_status = BLE_AUDIO_DONGLE_DL_STREAM_START;
}

/****************************************************************************************************************************************************/
/*                                                       ULL Dongle AFE IN(I2S IN/LINE IN)                                                          */
/****************************************************************************************************************************************************/
#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
static void ble_audio_dongle_dl_afe_in_init(
    ble_audio_dongle_dl_handle_t *dongle_handle,
    audio_transmitter_open_param_p audio_transmitter_open_param,
    bt_common_open_param_p bt_common_open_param
    )
{
    // no special handle
    UNUSED(dongle_handle);
    UNUSED(audio_transmitter_open_param);
    UNUSED(bt_common_open_param);
    return;
}

static void ble_audio_dongle_dl_afe_in_deinit(
    ble_audio_dongle_dl_handle_t *dongle_handle
    )
{
    // no special handle
    UNUSED(dongle_handle);
    return;
}

static void ble_audio_dongle_dl_afe_in_start(
    ble_audio_dongle_dl_handle_t *dongle_handle
    )
{
    // no special handle
    audio_dongle_dl_afe_start(dongle_handle, AUDIO_DONGLE_TYPE_BLE);
    return;
}

void ble_audio_dongle_dl_afe_in_stop(
    ble_audio_dongle_dl_handle_t *dongle_handle
    )
{
    // no special handle
    UNUSED(dongle_handle);
    return;
}

static uint32_t ble_audio_dongle_dl_calculate_afe_in_jitter_bt_audio(ble_audio_dongle_dl_handle_t *dongle_handle, uint32_t gpt_count)
{
    SOURCE source = dongle_handle->source;
    /* gpt_count should be μs */
    uint32_t ratio_ms = gpt_count/100 + 1; // add 0.1ms to avoid some abnormal case
    return ((uint64_t)source->param.audio.frame_size * ratio_ms)/50; // one channel
}

#endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */

/******************************************************************************/
/*          BLE audio source dongle silence detection Public Functions        */
/******************************************************************************/
#ifdef AIR_SILENCE_DETECTION_ENABLE
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ble_dongle_silence_detection_init(ble_audio_dongle_dl_handle_t *dongle_handle, audio_scenario_type_t scenario)
{
    uint32_t saved_mask;
    volume_estimator_port_t *port = NULL;
    volume_estimator_config_t config;
    void *data_buf;
    void *nvkey_buf;
    ble_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;

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

    /* get volume estimator port */
    port = volume_estimator_get_port(dongle_handle->owner);
    if (port == NULL)
    {
        AUDIO_ASSERT(0);
    }

    /* init volume estimator port */
    config.channel_num = 1;
    config.frame_size =  VOLUME_ESTIMATOR_CALCULATE_FRAME_SAMPLE_SIZE * config.channel_num * sizeof(int32_t); /* 40 samples * mono * 32bit */
    config.resolution = RESOLUTION_32BIT;
    config.mode = VOLUME_ESTIMATOR_CHAT_INSTANT_MODE;
    config.sample_rate = ble_audio_dl_info.lc3_packet_frame_samples * 1000000 / ble_audio_dl_info.lc3_packet_frame_interval;
    config.nvkey_para = (void *)&(((silence_detection_nvkey_t *)nvkey_buf)->chat_vol_nvkey);
    volume_estimator_init(port, &config);
    uint32_t data_size = ble_audio_dl_info.lc3_packet_frame_samples * sizeof(int32_t) * config.channel_num;
    /* malloc 10ms memory for queue mono pcm data */
    data_buf = preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE, data_size);
    if (data_buf == NULL)
    {
        AUDIO_ASSERT(0);
    }
    memset(data_buf, 0, data_size);

    /* get handle */
    uint32_t index = scenario - AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0;
    if (index > (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_SUB_ID_MAX - AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0)) {
        AUDIO_ASSERT(0 && "[ble audio] sceanrio id is not right");
    }
    silence_detection_handle = &ble_audio_dongle_silence_detection_handle[index];

    /* update state machine */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (silence_detection_handle->port != NULL)
    {
        AUDIO_ASSERT(0);
    }
    silence_detection_handle->port = port;
    silence_detection_handle->enable = 0;
    silence_detection_handle->nvkey_buf = nvkey_buf;
    silence_detection_handle->nvkey_enable = (int16_t)(((silence_detection_nvkey_t *)nvkey_buf)->enable);
    silence_detection_handle->current_db = -144*100;
    silence_detection_handle->effective_threshold_db = ((silence_detection_nvkey_t *)nvkey_buf)->effective_threshold_db;
    silence_detection_handle->effective_delay_ms = ((silence_detection_nvkey_t *)nvkey_buf)->effective_delay_ms;
    silence_detection_handle->effective_delay_us_count = 0;
    silence_detection_handle->failure_threshold_db = ((silence_detection_nvkey_t *)nvkey_buf)->failure_threshold_db;
    silence_detection_handle->failure_delay_ms = ((silence_detection_nvkey_t *)nvkey_buf)->failure_delay_ms;
    silence_detection_handle->failure_delay_us_count = 0;
    silence_detection_handle->process_size = config.frame_size;
    silence_detection_handle->data_size = 0;
    silence_detection_handle->data_buf_size = data_size;
    silence_detection_handle->data_buf = data_buf;
    silence_detection_handle->status = BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW;
    hal_nvic_restore_interrupt_mask(saved_mask);

    DSP_MW_LOG_I("[ble audio dongle][dl][silence_detection][init %d]:%d, %d, %d, %d, %d, %d\r\n", 7,
                scenario,
                silence_detection_handle->nvkey_enable,
                silence_detection_handle->current_db,
                silence_detection_handle->effective_threshold_db,
                silence_detection_handle->effective_delay_ms,
                silence_detection_handle->failure_threshold_db,
                silence_detection_handle->failure_delay_ms);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ble_dongle_silence_detection_deinit(ble_audio_dongle_dl_handle_t *dongle_handle, audio_scenario_type_t scenario)
{
    uint32_t saved_mask;
    volume_estimator_port_t *port;
    ble_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;
    UNUSED(dongle_handle);

    /* get handle */
    uint32_t index = scenario - AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0;
    if (index > (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_SUB_ID_MAX - AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0)) {
        AUDIO_ASSERT(0 && "[ble audio] sceanrio id is not right");
    }
    silence_detection_handle = &ble_audio_dongle_silence_detection_handle[index];

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
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    silence_detection_handle->port = NULL;
    silence_detection_handle->enable = 0;
    silence_detection_handle->nvkey_buf = NULL;
    silence_detection_handle->nvkey_enable = 0;
    silence_detection_handle->current_db = -144*100;
    silence_detection_handle->effective_threshold_db = 0;
    silence_detection_handle->effective_delay_ms = 0;
    silence_detection_handle->effective_delay_us_count = 0;
    silence_detection_handle->failure_threshold_db = 0;
    silence_detection_handle->failure_delay_ms = 0;
    silence_detection_handle->failure_delay_us_count = 0;
    silence_detection_handle->process_size = 0;
    silence_detection_handle->data_size = 0;
    silence_detection_handle->data_buf_size = 0;
    silence_detection_handle->data_buf = NULL;
    silence_detection_handle->status = BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW;
    hal_nvic_restore_interrupt_mask(saved_mask);

    DSP_MW_LOG_I("[ble audio dongle][dl][silence_detection][deinit %d]:%d, %d, %d, %d, %d, %d\r\n", 7,
                scenario,
                silence_detection_handle->nvkey_enable,
                silence_detection_handle->current_db,
                silence_detection_handle->effective_threshold_db,
                silence_detection_handle->effective_delay_ms,
                silence_detection_handle->failure_threshold_db,
                silence_detection_handle->failure_delay_ms);
}

void ble_dongle_silence_detection_enable(audio_scenario_type_t scenario)
{
    ble_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;

    /* get handle */
    uint32_t index = scenario - AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0;
    if (index > (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_SUB_ID_MAX - AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0)) {
        AUDIO_ASSERT(0 && "[ble audio] sceanrio id is not right");
    }
    silence_detection_handle = &ble_audio_dongle_silence_detection_handle[index];

    silence_detection_handle->enable = 1;
    silence_detection_handle->status = BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW;
    silence_detection_handle->failure_delay_us_count = 0;
    silence_detection_handle->effective_delay_us_count = 0;
    DSP_MW_LOG_I("[ble audio dongle][dl][silence_detection][enable]:%d\r\n", 1, scenario);
}

void ble_dongle_silence_detection_disable(audio_scenario_type_t scenario)
{
    ble_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;

    /* get handle */
    uint32_t index = scenario - AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0;
    if (index > (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_SUB_ID_MAX - AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0)) {
        AUDIO_ASSERT(0 && "[ble audio] sceanrio id is not right");
    }
    silence_detection_handle = &ble_audio_dongle_silence_detection_handle[index];

    silence_detection_handle->enable = 0;
    silence_detection_handle->status = BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW;
    silence_detection_handle->failure_delay_us_count = 0;
    silence_detection_handle->effective_delay_us_count = 0;
    DSP_MW_LOG_I("[ble audio dongle][dl][silence_detection][disable]:%d\r\n", 1, scenario);
}

void ble_dongle_silence_detection_process(audio_scenario_type_t scenario)
{
    ble_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;
    volume_estimator_port_t *port;
    uint32_t frame_num;
    aud_msg_status_t status;
    hal_ccni_message_t msg;
    ble_audio_dongle_silence_detection_status_t silence_detection_status_backup;
    uint32_t volume_estimator_calculate_frame_time_us = 0;
    /* get handle */
    uint32_t index = scenario - AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0;
    if (index > (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_SUB_ID_MAX - AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0)) {
        AUDIO_ASSERT(0 && "[ble audio] sceanrio id is not right");
    }
    silence_detection_handle = &ble_audio_dongle_silence_detection_handle[index];
    if (silence_detection_handle->enable)
    {
        port = silence_detection_handle->port;
        for (frame_num = 0; frame_num < (silence_detection_handle->data_size / silence_detection_handle->process_size); frame_num++)
        {
            /* get the latest volume of each 2.5ms frame and the current_db will be updated */
			volume_estimator_calculate_frame_time_us = VOLUME_ESTIMATOR_CALCULATE_FRAME_SAMPLE_SIZE * 1000000 / (ble_audio_dl_info.lc3_packet_frame_samples * 1000000 / ble_audio_dl_info.lc3_packet_frame_interval);
            if (volume_estimator_process(port, (void *)((uint8_t *)(silence_detection_handle->data_buf) + silence_detection_handle->process_size * frame_num), silence_detection_handle->process_size, &silence_detection_handle->current_db) != VOLUME_ESTIMATOR_STATUS_OK)
            {
                AUDIO_ASSERT(0);
            }
            // DSP_MW_LOG_I("[ble audio dongle][dl][silence_detection]:scenario %d, frame %d, current_db %d silence time %d non-silence time %d\r\n", 6,
            //     scenario,
            //     frame_num,
            //     silence_detection_handle->current_db,
            //     silence_detection_handle->effective_delay_us_count,
            //     silence_detection_handle->failure_delay_us_count
            //     );
            if (silence_detection_handle->current_db <= silence_detection_handle->effective_threshold_db)
            {
                silence_detection_handle->failure_delay_us_count = 0;
                silence_detection_handle->effective_delay_us_count += volume_estimator_calculate_frame_time_us;
                if (silence_detection_handle->effective_delay_us_count >= silence_detection_handle->effective_delay_ms*1000)
                {
                    if (silence_detection_handle->status != BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_SILENCE)
                    {
                        /* update status */
                        silence_detection_status_backup = silence_detection_handle->status;
                        silence_detection_handle->status = BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_SILENCE;
                        /* send msg to mcu side to alert that this is a silence period */
                        msg.ccni_message[0] = ((MSG_DSP2MCU_BT_AUDIO_DL_SILENCE_DETECTION_FEEDBACK << 16) | 1);
                        msg.ccni_message[1] = scenario;
                        status = aud_msg_tx_handler(msg, 0, FALSE);
                        if (status != AUDIO_MSG_STATUS_OK)
                        {
                            /* rollback state machine that the next silence period will send msg again */
                            silence_detection_handle->status = silence_detection_status_backup;
                        }
                        DSP_MW_LOG_I("[ble audio dongle][dl][silence_detection][send msg]:%d, %d, %d, %d, %d, %d\r\n", 6,
                                    silence_detection_handle->status,
                                    silence_detection_handle->current_db,
                                    silence_detection_handle->effective_threshold_db,
                                    silence_detection_handle->effective_delay_ms,
                                    silence_detection_handle->effective_delay_us_count,
                                    status);
                    }
                    else
                    {
                        /* do nothing because the silence status had been sent */
                    }
                    /* prevent overflow */
                    silence_detection_handle->effective_delay_us_count = silence_detection_handle->effective_delay_ms*1000;
                }
            }
            else if (silence_detection_handle->current_db >= silence_detection_handle->failure_threshold_db)
            {
                silence_detection_handle->effective_delay_us_count = 0;
                silence_detection_handle->failure_delay_us_count += volume_estimator_calculate_frame_time_us;
                if (silence_detection_handle->failure_delay_us_count >= silence_detection_handle->failure_delay_ms*1000)
                {
                    if (silence_detection_handle->status != BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_NOT_SILENCE)
                    {
                        /* update status */
                        silence_detection_status_backup = silence_detection_handle->status;
                        silence_detection_handle->status = BLE_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_NOT_SILENCE;
                        /* send msg to mcu side to alert that this is not a silence period */
                        msg.ccni_message[0] = ((MSG_DSP2MCU_BT_AUDIO_DL_SILENCE_DETECTION_FEEDBACK << 16) | 0);
                        msg.ccni_message[1] = scenario;
                        status = aud_msg_tx_handler(msg, 0, FALSE);
                        if (status != AUDIO_MSG_STATUS_OK)
                        {
                            /* rollback state machine that the next silence period will send msg again */
                            silence_detection_handle->status = silence_detection_status_backup;
                        }
                        DSP_MW_LOG_I("[ble audio dongle][dl][silence_detection][send msg]:%d, %d, %d, %d, %d, %d\r\n", 6,
                                    silence_detection_handle->status,
                                    silence_detection_handle->current_db,
                                    silence_detection_handle->failure_threshold_db,
                                    silence_detection_handle->failure_delay_ms,
                                    silence_detection_handle->failure_delay_us_count,
                                    status);
                    }
                    else
                    {
                        /* do nothing because the not silence status had been sent */
                    }
                    /* prevent overflow */
                    silence_detection_handle->failure_delay_us_count = silence_detection_handle->failure_delay_ms*1000;
                }
            }
            else
            {
                /* keep status so do nothing */
            }
        }
    }
}
#endif /* AIR_SILENCE_DETECTION_ENABLE */

#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
