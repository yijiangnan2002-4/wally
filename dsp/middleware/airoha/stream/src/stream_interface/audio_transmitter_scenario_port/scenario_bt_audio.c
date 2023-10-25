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

#if defined(AIR_BT_AUDIO_DONGLE_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "scenario_bt_audio.h"
#include "bt_interface.h"
#include "stream_bt_common.h"
#include "dsp_dump.h"
#include "sbc_encoder_interface.h"
#include "clk_skew_sw.h"
#include "sw_gain_interface.h"
#include "sw_buffer_interface.h"
#include "dsp_share_memory.h"
#include "voice_plc_interface.h"
#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
#include "volume_estimator_interface.h"
#include "dtm.h"
#include "audio_nvdm_common.h"
#include "preloader_pisplit.h"
#include "stream_nvkey_struct.h"
#endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */

#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
#include "lhdc_enc_interface.h"
#endif

/* Private define ------------------------------------------------------------*/
log_create_module(bt_dongle_log, PRINT_LEVEL_INFO);
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define BT_AUDIO_DONGLE_DL_DEBUG_ENABLE                (0)
#define BT_AUDIO_DONGLE_UL_DEBUG_ENABLE                (0)
#define BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE          (0)
#define BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE       (1)
#define AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE       (0)  // GPIO22 -> first data copy     // GPIO23 -> first data sink
#define GPIO_PIN_FIRST_RECEIVE                         (HAL_GPIO_0)
#define GPIO_PIN_FIRST_SEND                            (HAL_GPIO_1)

/* eSCO */
#define AUDIO_DSP_FORWARD_BUFFER_SIZE                  (120)
#define AUDIO_DSP_SCO_INBAND_INFORMATION               (20)
#define AUDIO_BT_MSBC_SHARE_BUF_FRAME_SIZE             (60U)
#define AUDIO_BT_CVSD_SHARE_BUF_FRAME_SIZE             (120U)
#define AUDIO_BT_HFP_BUF_PATTERN_TOTAL_SIZE            (480U)  // 8 * 60 frame
#define AUDIO_BT_HFP_BUF_INBAND_TOTAL_SIZE             (160U)  // 20 * 8
#define BT_AUDIO_UL_PREFILL_FRAME_NUM                  (1)     // prefill 1ms for usb audio
#define BT_AUDIO_UL_ANCHOR_INTERVAL                    (7500)  // 7.5ms

/* SRC Quality Mode */
#define AUDIO_SRC_QUALITY_MODE_HIGH_ENABLE             (0)
#if AUDIO_SRC_QUALITY_MODE_HIGH_ENABLE
    #define AUDIO_SRC_QUALITY_MODE                     (SRC_FIXED_RATIO_PORT_NORMAL_QUALITY)
#else
    #define AUDIO_SRC_QUALITY_MODE                     (SRC_FIXED_RATIO_PORT_HIGH_QUALITY)
#endif

/* ABR Control */
#if AUDIO_A2DP_SBC_VBR_ENABLE
#define AUDIO_A2DP_VBR_ENABLE
#endif

#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
#if AIR_AUDIO_LHDC_ABR_ENABLE
#define AUDIO_A2DP_VBR_ENABLE
#endif
#endif

/* Private variables ---------------------------------------------------------*/
#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE

#define SILENCE_DETECTION_IN_SOURCE   (1)
#define SILENCE_DETECTION_IN_SINK     (0)

#if SILENCE_DETECTION_IN_SOURCE && SILENCE_DETECTION_IN_SINK
// can't be set to 1 at the same time
#error
#endif

typedef enum {
    BT_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW = 0,
    BT_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_SILENCE,
    BT_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_NOT_SILENCE,
} bt_audio_dongle_silence_detection_status_t;

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
    bt_audio_dongle_silence_detection_status_t status;
    uint32_t total_channel;
    uint32_t data_size_channel;
} bt_audio_dongle_silence_detection_handle_t;
static bt_audio_dongle_silence_detection_handle_t g_bt_audio_dongle_silence_detection_handle;
static uint32_t g_bt_audio_dongle_silence_detection_init_count;
static uint32_t g_bt_audio_dongle_silence_detection_sample_rate;
static uint32_t g_bt_audio_dongle_silence_detection_interval; /* uint of us */
static uint32_t g_bt_audio_dongle_silence_detection_channels;
static bool     g_bt_audio_dongle_with_out_bt_link_mode = false;
static uint32_t g_bt_audio_dongle_timer_handle = 0;
#endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */
static bt_audio_dongle_handle_t *bt_audio_dongle_dl_handle_list = NULL;
static bt_audio_dongle_handle_t *bt_audio_dongle_ul_handle_list = NULL;
#if BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE
uint32_t gpt_test_handler = 0;
uint32_t gpt_test_irq_period = 0;
#endif /* BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE */
static sco_fwd_info g_sco_fwd_info = {0};
#if AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE
static bool first_dl_copy_data = false;
#endif

#if BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE
static uint32_t clk_skew_pre_usb_wo = 0;
static uint32_t clk_skew_bt_out_wo = 0;
static uint32_t clk_skew_usb_size_ms = 0;
static uint32_t clk_skew_bt_out_frame_size = 0;
static uint32_t clk_skew_pre_gpt_count = 0;
typedef struct scenario_bt_audio
{
    uint32_t duration;
    uint32_t usb_frame_ms;
    uint32_t sbc_frame_size;
    char     str[2];
} clk_skew_dump_t;

#endif /* BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE */

typedef union {
    uint32_t reserved;
} bt_audio_dongle_runtime_config_operation_param_t, *bt_audio_dongle_runtime_config_operation_param_p;

typedef struct {
    bt_audio_dongle_runtime_config_operation_t          config_operation;
    bt_audio_dongle_runtime_config_operation_param_t    config_param;
} bt_audio_dongle_runtime_config_param_t, *bt_audio_dongle_runtime_config_param_p;

#define BT_AUDIO_DONGLE_USB_IN_FS_MAX_NUM  (5)
const static uint32_t bt_audio_usb_in_sample_rate[BT_AUDIO_DONGLE_USB_IN_FS_MAX_NUM] = {
    16000, 32000, 48000, 96000, 192000
};

const static uint32_t bt_audio_usb_unified_sample_rate_setting_common[BT_AUDIO_DONGLE_USB_IN_FS_MAX_NUM][3] = {
    { 16000,  48000, 48000 },
    { 32000,  96000, 48000 },
    { 48000,  48000, 48000 },
    { 96000,  48000, 48000 },
    { 192000, 48000, 48000 }
};

const static uint32_t bt_audio_usb_unified_sample_rate_setting_nb[BT_AUDIO_DONGLE_USB_IN_FS_MAX_NUM][3] = {
    { 16000,  16000, 16000 },
    { 32000,  16000, 16000 },
    { 48000,  16000, 16000 },
    { 96000,  48000, 16000 },
    { 192000, 64000, 16000 }
};
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
const static uint32_t bt_audio_usb_unified_sample_rate_setting_lhdc[BT_AUDIO_DONGLE_USB_IN_FS_MAX_NUM][3] = {
    { 16000,  48000, 96000 },
    { 32000,  96000, 96000 },
    { 48000,  96000, 96000 },
    { 96000,  96000, 96000 },
    { 192000, 96000, 96000 }
};
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */
/* Public variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
extern VOID audio_transmitter_share_information_update_read_offset(SOURCE source, U32 ReadOffset);
extern VOID audio_transmitter_share_information_update_write_offset(SINK sink, U32 WriteOffset);
extern VOID audio_transmitter_share_information_fetch(SOURCE source, SINK sink);
extern VOID StreamDSP_HWSemaphoreTake(VOID);
extern VOID StreamDSP_HWSemaphoreGive(VOID);
// ATTR_TEXT_IN_IRAM static bool bt_audio_dongle_ul_fetch_time_is_arrived(bt_audio_dongle_handle_t *dongle_handle, uint32_t bt_clk);
static void bt_audio_dongle_dl_common_init(bt_audio_dongle_handle_t *dongle_handle);
static void bt_audio_dongle_dl_common_deinit(bt_audio_dongle_handle_t *dongle_handle);
static void sco_fwd_rx_ul_irq_handler(void);
static void sco_fwd_tx_dl_irq_handler(void);
static hal_nvic_status_t sco_fwd_irq_set_enable(fowarder_ctrl forwarder_en, fowarder_type forwarder_type);

#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
static void bt_audio_dongle_dl_afe_in_init(
    bt_audio_dongle_handle_t *dongle_handle,
    audio_transmitter_open_param_p audio_transmitter_open_param,
    bt_common_open_param_p bt_common_open_param);
static void bt_audio_dongle_dl_afe_in_deinit(bt_audio_dongle_handle_t *dongle_handle);
#endif /* Dongle afe in */

#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
static void bt_audio_dongle_dl_usb_in_init(
    bt_audio_dongle_handle_t *dongle_handle,
    audio_transmitter_open_param_p audio_transmitter_open_param,
    bt_common_open_param_p bt_common_open_param);
static void bt_audio_dongle_dl_usb_in_deinit(bt_audio_dongle_handle_t *dongle_handle);
#endif
static void bt_audio_dongle_dl_bt_out_init(
    bt_audio_dongle_handle_t *dongle_handle,
    audio_transmitter_open_param_p audio_transmitter_open_param,
    bt_common_open_param_p bt_common_open_param);
static void bt_audio_dongle_dl_bt_out_deinit(bt_audio_dongle_handle_t *dongle_handle);
#ifdef AUDIO_A2DP_VBR_ENABLE
void bt_audio_dongle_dl_sink_buffer_monitor(SINK sink);
#endif
/* Public functions ----------------------------------------------------------*/
extern BOOL msbc_subseqn_check(U8 *inbuf);

bt_audio_dongle_handle_t *bt_audio_dongle_query_handle_by_scenario_type(audio_scenario_type_t type, audio_dongle_stream_type_t stream_type)
{
    bt_audio_dongle_handle_t *handle = (stream_type == AUDIO_DONGLE_STREAM_TYPE_DL) ? bt_audio_dongle_dl_handle_list : bt_audio_dongle_ul_handle_list;
    bool find_flag = false;
    while (handle) {
        if (handle->source->scenario_type == type) {
            find_flag = true;
            break;
        }
        handle = handle->next_handle;
    }
    if (!find_flag) {
        BT_DONGLE_LOG_E("[BT Audio] not find dongle handle, type %d", 1, type);
    }
    return handle;
}

static uint32_t bt_audio_dongle_get_handle_length(audio_dongle_stream_type_t stream_type)
{
    uint32_t cnt = 0;
    bt_audio_dongle_handle_t *handle = (stream_type == AUDIO_DONGLE_STREAM_TYPE_DL) ? bt_audio_dongle_dl_handle_list : bt_audio_dongle_ul_handle_list;
    while (handle) {
        cnt++;
        handle = handle->next_handle;
    }
    return cnt;
}

static uint32_t bt_audio_dongle_get_handle_length_by_type(audio_dongle_stream_type_t stream_type, bt_audio_type type)
{
    uint32_t cnt = 0;
    AUDIO_ASSERT(type <= BT_AUDIO_TYPE_A2DP);
    bt_audio_dongle_handle_t *handle = (stream_type == AUDIO_DONGLE_STREAM_TYPE_DL) ? bt_audio_dongle_dl_handle_list : bt_audio_dongle_ul_handle_list;
    while (handle) {
        if (type == handle->link_type) cnt++;
        handle = handle->next_handle;
    }
    return cnt;
}

bt_audio_dongle_handle_t *bt_audio_dongle_get_handle(void *owner, audio_dongle_stream_type_t stream_type)
{
    bt_audio_dongle_handle_t *handle = (stream_type == AUDIO_DONGLE_STREAM_TYPE_DL) ? bt_audio_dongle_dl_handle_list : bt_audio_dongle_ul_handle_list;
    bt_audio_dongle_handle_t *new_handle = (bt_audio_dongle_handle_t *)pvPortMalloc(sizeof(bt_audio_dongle_handle_t));
    if (new_handle == NULL) {
        AUDIO_ASSERT(0 && "malloc fail");
    }
    memset(new_handle, 0, sizeof(bt_audio_dongle_handle_t));
    new_handle->owner = owner;
    new_handle->next_handle = NULL;
    if (handle == NULL) {
        if (stream_type == AUDIO_DONGLE_STREAM_TYPE_DL) {
            bt_audio_dongle_dl_handle_list = new_handle;
            handle = bt_audio_dongle_dl_handle_list;
        } else {
            bt_audio_dongle_ul_handle_list = new_handle;
            handle = bt_audio_dongle_ul_handle_list;
        }
    } else {
        while (handle->next_handle) {  /* Search the the last node */
            handle = handle->next_handle;
        }
        handle->next_handle = new_handle; // append this new node in the final position.
    }
    return new_handle;
}

void bt_audio_dongle_release_handle(bt_audio_dongle_handle_t *dongle_handle, audio_dongle_stream_type_t stream_type)
{
    bt_audio_dongle_handle_t *handle = (stream_type == AUDIO_DONGLE_STREAM_TYPE_DL) ? bt_audio_dongle_dl_handle_list : bt_audio_dongle_ul_handle_list;
    bt_audio_dongle_handle_t *pre_handle = NULL;
    bt_audio_dongle_handle_t *find_handle = NULL;
    if (handle == NULL) {
        BT_DONGLE_LOG_E("[BT Audio] delete handle fail 0x%x %d", 2, (uint32_t)dongle_handle, stream_type);
        AUDIO_ASSERT(0);
    }

    /* search handle */
    while (handle) {
        if (handle == dongle_handle) {
            find_handle = handle;
            break;
        }
        pre_handle = handle;
        handle = handle->next_handle;
    }

    // not found this node
    if (!find_handle) {
        BT_DONGLE_LOG_E("[BT Audio] delete handle fail 0x%x %d, not found", 2, (uint32_t)dongle_handle, stream_type);
        AUDIO_ASSERT(0);
    }

    /* delete this node */
    if (!pre_handle) {
        // delete the first handle
        if (stream_type == AUDIO_DONGLE_STREAM_TYPE_DL) {
            bt_audio_dongle_dl_handle_list = find_handle->next_handle;
        } else {
            bt_audio_dongle_ul_handle_list = find_handle->next_handle;
        }
    } else {
        pre_handle->next_handle = find_handle->next_handle;
    }

    vPortFree(find_handle);
    find_handle = NULL;
}

/****************************************************************************************************************************************************/
/*                                                       BT Source Dongle DL(USB IN)                                                                */
/****************************************************************************************************************************************************/
ATTR_TEXT_IN_IRAM static void bt_audio_dongle_dl_mixer_post_callback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size)
{
    uint32_t                 i                    = 0;
    bt_audio_dongle_handle_t *c_handle            = NULL;
    SOURCE                   source               = NULL;
    bool                     all_streams_in_mixer = true;
    uint32_t                 dl_stream_count      = bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_DL);
    UNUSED(para);

    /* change this handle's data status */
    source = (SOURCE)(member->owner);
    c_handle = (bt_audio_dongle_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    c_handle->data_status = AUDIO_DONGLE_DL_DATA_IN_MIXER;

    /* check if this stream is unmixed or the first packet is not ready */
    if (c_handle->mixer_status == AUDIO_DONGLE_MIXER_UNMIX) {
        /* clear this stream's input buffer */
        stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);
        /* change this handle data status */
        c_handle->data_status = AUDIO_DONGLE_DL_DATA_EMPTY;
        /* this stream is unmixed now, so we can directly return here */
        goto _end;
    } else if (c_handle->first_packet_status == AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY) {
        /* clear this stream's input buffer */
        stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);
        /* change this handle data status */
        c_handle->data_status = AUDIO_DONGLE_DL_DATA_EMPTY;
        /* in here, it means it is the first packet, we need to drop the later sink flow */
        *out_frame_size = 0;
        goto _end;
    }

    /* check if all mixed stream data is in this mixer */
    c_handle = bt_audio_dongle_dl_handle_list;
    for (i = 0; i < dl_stream_count; i++) {
        if ((c_handle->data_status == AUDIO_DONGLE_DL_DATA_IN_STREAM) &&
            (c_handle->mixer_status != AUDIO_DONGLE_MIXER_UNMIX) &&
            (c_handle->first_packet_status != AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY)) {
            /* this stream should be mixed, but its data is not in this mixer now.
               so we need to bypass this mix result in this time */
            all_streams_in_mixer = false;
            break;
        }
        // BT_DONGLE_LOG_E("[BT Audio][DL][type %d] all_streams_in_mixer %d, %d, %d, %d\r\n", 5,
        //     c_handle->source->scenario_type,
        //     all_streams_in_mixer,
        //     c_handle->data_status,
        //     c_handle->mixer_status,
        //     c_handle->first_packet_status
        //     );
        /* switch to the next dl stream */
        c_handle = c_handle->next_handle;
    }
    /* check if the output data in this time includes all stream data */
    if ((*out_frame_size != 0) && (all_streams_in_mixer == true)) {
        /* all_streams_in_mixer is true, so all stream data is in this mixer.
           So we can clean all mixed streams' input buffers now and the mix result are sent to the sink */
        c_handle = bt_audio_dongle_dl_handle_list;
        for (i = 0; i < dl_stream_count; i++) {
            if (c_handle->mixer_status != AUDIO_DONGLE_MIXER_UNMIX) {
                /* clear this stream's input buffer */
                stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);
                /* change this handle data status */
                c_handle->data_status = AUDIO_DONGLE_DL_DATA_EMPTY;
            }
            /* switch to the next dl stream */
            c_handle = c_handle->next_handle;
        }
    } else {
        /* all_streams_in_mixer is false, so some stream data is not in this mixer.
           So we need to bypass this mix result */
        *out_frame_size = 0;
    }

_end:
#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
    #if SILENCE_DETECTION_IN_SINK
        if (*out_frame_size != 0) {
            uint32_t curr_idx;
            uint8_t *p_curr_buf;
            bt_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;

            silence_detection_handle = &g_bt_audio_dongle_silence_detection_handle;
            if (silence_detection_handle->enable) {
                for (curr_idx = 0; curr_idx < stream_function_get_channel_number(para); curr_idx++) {
                    if (stream_function_get_output_resolution(para) == RESOLUTION_16BIT) {
                        p_curr_buf = (uint8_t *)(silence_detection_handle->data_buf) + curr_idx * stream_function_get_output_size(para) * 2;
                        ShareBufferCopy_D_16bit_to_D_32bit_1ch((uint16_t* )stream_function_get_inout_buffer(para, curr_idx + 1), (uint32_t* )p_curr_buf, stream_function_get_output_size(para) / 2);
                        silence_detection_handle->data_size_channel = stream_function_get_output_size(para) * 2;
                        silence_detection_handle->data_size = stream_function_get_channel_number(para) * stream_function_get_output_size(para) * 2;
                    } else {
                        p_curr_buf = (uint8_t *)(silence_detection_handle->data_buf) + curr_idx * stream_function_get_output_size(para);
                        memcpy(p_curr_buf, stream_function_get_inout_buffer(para, curr_idx + 1), stream_function_get_output_size(para));
                        silence_detection_handle->data_size_channel = stream_function_get_output_size(para);
                        silence_detection_handle->data_size = stream_function_get_channel_number(para) * stream_function_get_output_size(para);
                    }
                }
                silence_detection_handle->total_channel = stream_function_get_channel_number(para);
                /* send msg to DTM task to do silence detection */
                if (DTM_query_queue_avail_number() >= 2) {
                    DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_PROCESS, (uint32_t)source->scenario_type, false);
                } else {
                    BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection] scenario %d-%d abort silence detection, DTM queue full\r\n", 2,
                                            source->param.data_dl.scenario_type,
                                            source->param.data_dl.scenario_sub_id);
                }
            }
        }
    #else
        ;
    #endif /* SILENCE_DETECTION_IN_SINK */
#else
    ;
#endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */
}

ATTR_TEXT_IN_IRAM static void bt_audio_dongle_dl_ccni_handler(hal_ccni_event_t event, void *msg)
{
    SINK sink;
    uint32_t saved_mask;
    uint32_t i;
    uint32_t gpt_count;
#if BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE
#else
    uint32_t bt_count;
    // uint32_t blk_index; // BT Audio Dongle won't use it.
    hal_ccni_message_t *ccni_msg = msg;
#endif /* BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE */
    bt_audio_dongle_handle_t *c_handle = NULL;
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
    n9_dsp_share_info_t *share_info = NULL;
#endif
    n9_dsp_share_info_ptr p_share_info = NULL;
    static uint32_t error_count = 0;
    uint32_t dl_stream_count = 0;

    UNUSED(event);
    UNUSED(msg);
    /* get timestamp for debug */
#if BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE
#else
    if (!msg) {
        bt_count = ccni_msg->ccni_message[0];
    }
    // blk_index = (uint16_t)(ccni_msg->ccni_message[1]);
#endif /* BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE */

    /* -------------------------------------------------------------------------------------------------------------- */
    /* ---------------------------------------- DL CLK SKEW DEBUG  -------------------------------------------------- */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
#if BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE
    uint32_t dur = 0;
    bool debug_flag = false;
    if (clk_skew_pre_gpt_count == 0) {
        clk_skew_pre_gpt_count = gpt_count;
        clk_skew_usb_size_ms = 0;
        clk_skew_bt_out_frame_size = 0;
        clk_skew_bt_out_wo = 0;
        clk_skew_pre_usb_wo = 0;
    }
    hal_gpt_get_duration_count(clk_skew_pre_gpt_count, gpt_count, &dur);
    if (dur >= 1500000) {
        clk_skew_pre_gpt_count = gpt_count;
        debug_flag = true;
    }
#endif /* BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE */

    /* BT controller will send write index info on ccni msg */
#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
    BT_DONGLE_LOG_I("[BT Audio][DL] ccni irq %d without bt link mode %d", 2, gpt_count, g_bt_audio_dongle_with_out_bt_link_mode);
#else
    BT_DONGLE_LOG_I("[BT Audio][DL] ccni irq %d", 1, gpt_count);
#endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */
    dl_stream_count = bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_DL);
    /* check if there is any DL stream */
    if (dl_stream_count == 0) {
        if ((error_count % 1000) == 0) {
            BT_DONGLE_LOG_W("[BT Audio][DL][WARNNING] dl dongle_handle is NULL, %d\r\n", 1, error_count);
        }
        error_count += 1;
        goto _ccni_return;
    } else {
        error_count = 0;
#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
        if (g_bt_audio_dongle_with_out_bt_link_mode) {
            uint32_t frame_interval = bt_audio_dongle_dl_handle_list->stream_info.dl_info.sink.bt_out.frame_interval;
            hal_gpt_status_t st = hal_gpt_sw_start_timer_us(g_bt_audio_dongle_timer_handle, frame_interval, (hal_gpt_callback_t)bt_audio_dongle_dl_ccni_handler, NULL);
            if (st != HAL_GPT_STATUS_OK) {
                BT_DONGLE_LOG_W("[BT Audio][DL][WARNNING] start sw timer fail, %d\r\n", 1, st);
            }
        }
#endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */
    }

    /* trigger all started DL stream one by one */
    c_handle = bt_audio_dongle_dl_handle_list;
    for (i = 0; i < dl_stream_count; i++) {
        sink = c_handle->sink;
        if ((c_handle->stream_status == AUDIO_DONGLE_STREAM_START) || (c_handle->stream_status == AUDIO_DONGLE_STREAM_RUNNING)) {
            if ((sink == NULL) || (sink->transform == NULL) || (c_handle->source == NULL)) {
                break;
            }
            switch (sink->scenario_type) {
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
                    /* set timestamp for debug */
#if BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE
#else
                    c_handle->ccni_in_bt_count  = bt_count;
                    /* set blk index */
                    // c_handle->ccni_blk_index    = blk_index;
#endif /* BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE */
                    c_handle->ccni_in_gpt_count = gpt_count;
                    /* increase fetch count */
                    if (c_handle->fetch_count != 0) {
                        BT_DONGLE_LOG_E("[BT Audio][DL][ERROR] fetch count is abnormal %d, process time is not enough!!!", 1, c_handle->fetch_count);
                        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                        c_handle->fetch_count ++;
                    } else {
                        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                        c_handle->fetch_count = 0x5A5A; // 0x5A5A is a symbol to indicate ccni request
                    }
                    hal_nvic_restore_interrupt_mask(saved_mask);
                    if (c_handle->first_packet_status == AUDIO_DONGLE_DL_FIRST_PACKET_READY) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
                        // streamBuffer.ShareBufferInfo should be update
                        audio_transmitter_share_information_fetch(c_handle->source, NULL);
                        share_info = &(c_handle->source->streamBuffer.ShareBufferInfo);
                        c_handle->latency_size = audio_dongle_get_n9_share_buffer_data_size_without_header(share_info);
                        /* -------------------------------------------------------------------------------------------------------------- */
                        /* ---------------------------------------- DL CLK SKEW DEBUG  -------------------------------------------------- */
                        #if BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE
                            if (share_info->write_offset >= clk_skew_pre_usb_wo) {
                                clk_skew_usb_size_ms += (share_info->write_offset - clk_skew_pre_usb_wo) / share_info->sub_info.block_info.block_size;
                            } else {
                                clk_skew_usb_size_ms += share_info->sub_info.block_info.block_num - (clk_skew_pre_usb_wo / share_info->sub_info.block_info.block_size);
                                clk_skew_usb_size_ms += share_info->write_offset / share_info->sub_info.block_info.block_size;
                            }
                            clk_skew_pre_usb_wo = share_info->write_offset;
                            p_share_info = (n9_dsp_share_info_ptr)((c_handle->stream_info.dl_info.sink.bt_out.bt_info[0]).share_info);
                            if (p_share_info->write_offset >= clk_skew_bt_out_wo) {
                                clk_skew_bt_out_frame_size += (p_share_info->write_offset - clk_skew_bt_out_wo) / p_share_info->sub_info.block_info.block_size;
                            } else {
                                clk_skew_bt_out_frame_size += p_share_info->sub_info.block_info.block_num - clk_skew_bt_out_wo / p_share_info->sub_info.block_info.block_size;
                                clk_skew_bt_out_frame_size += p_share_info->write_offset / p_share_info->sub_info.block_info.block_size;
                            }
                            clk_skew_bt_out_wo = p_share_info->write_offset;
                            if (debug_flag) {
                                clk_skew_dump_t result = {0};
                                result.duration = dur;
                                result.usb_frame_ms = clk_skew_usb_size_ms;
                                result.sbc_frame_size = clk_skew_bt_out_frame_size;
                                result.str[0] = '\r';
                                result.str[1] = '\n';
                                LOG_AUDIO_DUMP((U8 *)&result, 14, AUDIO_BT_SRC_DONGLE_DL_CLK_SKEW_DEBUG);
                                BT_DONGLE_LOG_I("[BT Audio][DL][DEBUG] duration %u usb size %u clk_skew_bt_out_frame_size %u", 3, dur, clk_skew_usb_size_ms, clk_skew_bt_out_frame_size);
                                clk_skew_bt_out_frame_size = 0;
                                clk_skew_usb_size_ms = 0;
                            }
                        #endif /* BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE */
#endif
                    } else if (c_handle->first_packet_status == AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY) {
                        for (uint32_t j = 0; j < BT_AUDIO_DATA_CHANNEL_NUMBER; j++) {
                            if (!c_handle->stream_info.dl_info.sink.bt_out.bt_info[j].enable) {
                                continue;
                            } else {
                                p_share_info = (n9_dsp_share_info_ptr)((c_handle->stream_info.dl_info.sink.bt_out.bt_info[j]).share_info);
                                c_handle->pre_ro[j] = p_share_info->read_offset;
                            }
                        }
                    }
                    /* Handler the stream */
                    AudioCheckTransformHandle(sink->transform);
                    break;
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
                    c_handle->ccni_in_bt_count  = bt_count;
                    c_handle->ccni_in_gpt_count = gpt_count;
                    audio_dongle_afe_in_ccni_handler(c_handle, AUDIO_DONGLE_TYPE_BT);
                    AudioCheckTransformHandle(sink->transform);
                    break;
#endif
                default:
                    AUDIO_ASSERT(0);
                    break;
            }
            /* the first stream check the abr control */
            if ((i == 0) && (c_handle->first_packet_status == AUDIO_DONGLE_DL_FIRST_PACKET_READY)) {
                #ifdef AUDIO_A2DP_VBR_ENABLE
                    bt_audio_dongle_dl_sink_buffer_monitor(c_handle->sink);
                #endif /* AUDIO_A2DP_VBR_ENABLE */
            }
        }
        /* switch to the next dl stream */
        c_handle = c_handle->next_handle;
    }

    if (c_handle != NULL) {
        AUDIO_ASSERT(0);
    }

_ccni_return:
    return;
}

#if BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE
hal_gpt_callback_t gpt_test_dl_handle(void *user_data)
{
    UNUSED(user_data);
    bt_audio_dongle_dl_ccni_handler(0, NULL);
    hal_gpt_sw_start_timer_us(gpt_test_handler, gpt_test_irq_period, (hal_gpt_callback_t)gpt_test_dl_handle, NULL);
    return 0;
}
#endif /* BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE */

#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
static void bt_audio_dongle_dl_start_sw_timer(bt_audio_dongle_handle_t *dongle_handle)
{
    hal_gpt_status_t              gpt_status = HAL_GPT_STATUS_OK;
    bt_audio_dongle_bt_out_info_t *bt_out    = &(dongle_handle->stream_info.dl_info.sink.bt_out);
    uint32_t                      err_flag   = 0;
    if (g_bt_audio_dongle_timer_handle == 0) {
        gpt_status = hal_gpt_sw_get_timer(&g_bt_audio_dongle_timer_handle);
        if (gpt_status != HAL_GPT_STATUS_OK) {
            err_flag |= 1 << 0;
            goto __ERROR__;
        }
    } else {
        gpt_status = hal_gpt_sw_stop_timer_us(g_bt_audio_dongle_timer_handle);
        BT_DONGLE_LOG_W("[BT Audio][DL] sw timer is still on !!", 0);
        if (gpt_status != HAL_GPT_STATUS_OK) {
            err_flag |= 1 << 1;
            goto __ERROR__;
        }
    }
    gpt_status = hal_gpt_sw_start_timer_us(g_bt_audio_dongle_timer_handle, bt_out->frame_interval, (hal_gpt_callback_t)bt_audio_dongle_dl_ccni_handler, NULL);
    if (gpt_status != HAL_GPT_STATUS_OK) {
        err_flag |= 1 << 2;
        goto __ERROR__;
    }
    BT_DONGLE_LOG_I("[BT Audio][DL] start sw timer %d", 1, dongle_handle->source->scenario_type);
    return;
__ERROR__:
    BT_DONGLE_LOG_W("[BT Audio][DL] start sw timer error %d", 1, err_flag);
}

static void bt_audio_dongle_dl_stop_sw_timer(void)
{
    hal_gpt_status_t gpt_status = HAL_GPT_STATUS_OK;
    if (g_bt_audio_dongle_timer_handle != 0) {
        gpt_status = hal_gpt_sw_stop_timer_us(g_bt_audio_dongle_timer_handle);
        if (gpt_status != HAL_GPT_STATUS_OK) {
            BT_DONGLE_LOG_W("[BT Audio][DL] stop sw timer error %d", 1, gpt_status);
        }
        hal_gpt_sw_free_timer(g_bt_audio_dongle_timer_handle);
        g_bt_audio_dongle_timer_handle = 0;
        BT_DONGLE_LOG_I("[BT Audio][DL] stop sw timer", 0);
    }
}

#endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */

static bool bt_audio_dongle_dl_usb_in_check_sample_rate(stream_resolution_t resolution, uint32_t sample_rate)
{
    bool ret = false;
    if ((resolution == RESOLUTION_16BIT) || (resolution == RESOLUTION_32BIT)) {
        for (uint32_t i = 0; i < BT_AUDIO_DONGLE_USB_IN_FS_MAX_NUM; i ++) {
            if (sample_rate == bt_audio_usb_in_sample_rate[i]) {
                ret = true;
            }
        }
    } else {
        ret = true; // avoid false alarm
    }
    return ret;
}

static uint32_t bt_audio_dongle_dl_fs_convert_sample_rate_get(uint32_t in_rate, audio_dsp_codec_type_t codec_type, uint8_t sequence)
{
    uint32_t raw_index = 0;
    uint32_t sample_rate = 0;
    if (sequence > 2) {
        AUDIO_ASSERT(0 && "BT Audio input param error");
        return 0;
    }
    UNUSED(codec_type);
    switch (in_rate) {
        case 16000:
        case 32000:
        case 48000:
            raw_index = in_rate / 16000 - 1;
            break;
        case 96000:
            raw_index = 3;
            break;
        case 192000:
            raw_index = 4;
            break;
        default:
            AUDIO_ASSERT(0 && "BT Audio dl sample rate error");
            break;
    }
    switch (codec_type) {
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
        case AUDIO_DSP_CODEC_TYPE_LHDC:
            /* code */
            sample_rate = bt_audio_usb_unified_sample_rate_setting_lhdc[raw_index][sequence];
            break;
#endif
        case AUDIO_DSP_CODEC_TYPE_MSBC:
        case AUDIO_DSP_CODEC_TYPE_CVSD:
            sample_rate = bt_audio_usb_unified_sample_rate_setting_nb[raw_index][sequence];
            break;
        default:
            sample_rate = bt_audio_usb_unified_sample_rate_setting_common[raw_index][sequence];
            break;
    }
    return sample_rate;
}

void bt_audio_dongle_dl_init(SOURCE source, SINK sink, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    /* get handle for application config */
    bt_audio_dongle_handle_t       *dongle_handle      = (bt_audio_dongle_handle_t *)bt_audio_dongle_get_handle(sink, AUDIO_DONGLE_STREAM_TYPE_DL);
    bt_audio_dongle_dl_sink_info_t *sink_info          = &(dongle_handle->stream_info.dl_info.sink);
    dongle_handle->source                              = source;
    dongle_handle->sink                                = sink;
    sink->param.bt_common.scenario_param.dongle_handle = (void *)dongle_handle;
    /* task config */
    source->taskId = DHP_TASK_ID;
    sink->taskid   = DHP_TASK_ID;
    BT_DONGLE_LOG_I("[BT Audio][DL] init scenario type %d", 1, source->scenario_type);
#if AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE
    BT_DONGLE_LOG_I("[BT Audio][DL] init gpio %d %d", 2, GPIO_PIN_FIRST_SEND, GPIO_PIN_FIRST_RECEIVE);
    first_dl_copy_data = false;
    hal_pinmux_set_function(GPIO_PIN_FIRST_SEND, 0);
    hal_gpio_set_direction(GPIO_PIN_FIRST_SEND, 1);
    hal_gpio_disable_pull(GPIO_PIN_FIRST_SEND);
    hal_gpio_set_output(GPIO_PIN_FIRST_SEND, 0);

    hal_pinmux_set_function(GPIO_PIN_FIRST_RECEIVE, 0);
    hal_gpio_set_direction(GPIO_PIN_FIRST_RECEIVE, 1);
    hal_gpio_disable_pull(GPIO_PIN_FIRST_RECEIVE);
    hal_gpio_set_output(GPIO_PIN_FIRST_RECEIVE, 0);
#endif /* AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE */
    /* init audio info */
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
            dongle_handle->link_type = BT_AUDIO_TYPE_HFP;
            bt_audio_dongle_dl_usb_in_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            bt_audio_dongle_dl_bt_out_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            source->taskId = DAV_TASK_ID;
            sink->taskid   = DAV_TASK_ID;
            #if BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE
                clk_skew_pre_gpt_count = 0;
            #endif /* BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE */
            break;
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            dongle_handle->link_type = BT_AUDIO_TYPE_A2DP;
            bt_audio_dongle_dl_usb_in_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            bt_audio_dongle_dl_bt_out_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            #if BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE
                clk_skew_pre_gpt_count = 0;
            #endif /* BT_AUDIO_DONGLE_DL_CLK_SKEW_DEBUG_ENABLE */
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
            dongle_handle->link_type = BT_AUDIO_TYPE_A2DP;
            bt_audio_dongle_dl_afe_in_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            bt_audio_dongle_dl_bt_out_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            break;
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            dongle_handle->link_type = BT_AUDIO_TYPE_HFP;
            bt_audio_dongle_dl_afe_in_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            bt_audio_dongle_dl_bt_out_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            break;
#endif
        default:
            break;
    }
    /* common init: sw mixer */
    bt_audio_dongle_dl_common_init(dongle_handle);

    /* register ccni handler */
    if (bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_DL) == 1) {
        /* lock sleep because sleep wake-up time will consume the stream process time */
        hal_sleep_manager_lock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
    }
    if (bt_audio_dongle_get_handle_length_by_type(AUDIO_DONGLE_STREAM_TYPE_DL, dongle_handle->link_type) == 1) {
        if (dongle_handle->link_type == BT_AUDIO_TYPE_A2DP) {
            audio_transmitter_register_isr_handler(0, bt_audio_dongle_dl_ccni_handler);
        } else if (dongle_handle->link_type == BT_AUDIO_TYPE_HFP) {
            /* only the first hfp init the global variable */
            memset(&g_sco_fwd_info, 0, sizeof(sco_fwd_info));
            g_sco_fwd_info.pkt_type = SCO_PKT_NULL;
            g_sco_fwd_info.rx_ul_fwd_buf_index_pre = -1; // avoid enter wrong flow
            g_sco_fwd_info.tx_dl_fwd_buf_index_pre = -1; // avoid enter wrong flow
            /* enable tx forwarder */
            sco_fwd_irq_set_enable(ENABLE_FORWARDER, TX_FORWARDER);
            /* enable forwarder irq / hw buffer */
            Forwarder_Tx_Intr_Ctrl(true);
            Forwarder_Tx_Buf_Ctrl(true);
            g_sco_fwd_info.tx_dl_info = (avm_share_buf_info_ptr)(sink_info->bt_out.bt_info[0].share_info);
        }
    }
    /* stream status update */
    dongle_handle->stream_status = AUDIO_DONGLE_STREAM_INIT;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void bt_audio_dongle_dl_deinit(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t saved_mask;
    UNUSED(source);
    UNUSED(sink);
    BT_DONGLE_LOG_I("[BT Audio][DL] deinit scenario type %d", 1, source->scenario_type);
    /* unregister ccni handler */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_DL) == 1) {
        hal_sleep_manager_unlock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
    if (bt_audio_dongle_get_handle_length_by_type(AUDIO_DONGLE_STREAM_TYPE_DL, dongle_handle->link_type) == 1) {
        if (dongle_handle->link_type == BT_AUDIO_TYPE_A2DP) {
            audio_transmitter_unregister_isr_handler(0, bt_audio_dongle_dl_ccni_handler);
        } else if (dongle_handle->link_type == BT_AUDIO_TYPE_HFP) {
            /* disable tx forwarder */
            sco_fwd_irq_set_enable(DISABLE_FORWARDER, TX_FORWARDER);
            // disable forwarder irq / hw buffer
            Forwarder_Tx_Intr_Ctrl(false);
            Forwarder_Tx_Buf_Ctrl(false);
            g_sco_fwd_info.is_dl_stream_ready = false;
        }
    }
    /* deinit audio info */
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            bt_audio_dongle_dl_usb_in_deinit(dongle_handle);
            bt_audio_dongle_dl_bt_out_deinit(dongle_handle);
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            bt_audio_dongle_dl_afe_in_deinit(dongle_handle);
            bt_audio_dongle_dl_bt_out_deinit(dongle_handle);
            dongle_handle->link_type = BT_AUDIO_TYPE_HFP;
            break;
#endif
        default:
            BT_DONGLE_LOG_I("[BT Audio][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    bt_audio_dongle_dl_common_deinit(dongle_handle);
    /* stream status update */
    dongle_handle->stream_status = AUDIO_DONGLE_STREAM_DEINIT;
    /* release handle */
    bt_audio_dongle_release_handle(dongle_handle, AUDIO_DONGLE_STREAM_TYPE_DL);
    sink->param.bt_common.scenario_param.dongle_handle = NULL;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void bt_audio_dongle_dl_start(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t saved_mask;
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY;
            hal_nvic_restore_interrupt_mask(saved_mask);
#if BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE
            hal_gpt_sw_get_timer(&gpt_test_handler);
            gpt_test_irq_period = 7500; // 7.5ms
            hal_gpt_sw_start_timer_us(gpt_test_handler, gpt_test_irq_period, (hal_gpt_callback_t)gpt_test_dl_handle, NULL);
#endif /* BT_AUDIO_DONGLE_DEBUG_GPT_TEST_ENABLE */
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY;
            hal_nvic_restore_interrupt_mask(saved_mask);
            audio_dongle_dl_afe_start(dongle_handle, AUDIO_DONGLE_TYPE_BT);
            break;
#endif
        default:
            BT_DONGLE_LOG_I("[BT Audio][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    /* stream status update */
    dongle_handle->stream_status = AUDIO_DONGLE_STREAM_START;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void bt_audio_dongle_dl_stop(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t saved_mask;
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->first_packet_status = AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY;
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->first_packet_status = AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY;
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;
#endif
        default:
            BT_DONGLE_LOG_I("[BT Audio][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    dongle_handle->stream_status = AUDIO_DONGLE_STREAM_STOP;
}

bool bt_audio_dongle_dl_config(SOURCE source, stream_config_type type, U32 value)
{
    bt_audio_dongle_handle_t               *dongle_handle = bt_audio_dongle_query_handle_by_scenario_type(source->scenario_type, AUDIO_DONGLE_STREAM_TYPE_DL);
    bt_audio_dongle_dl_source_info_t       *source_info   = &(dongle_handle->stream_info.dl_info.source);
    bt_audio_dongle_runtime_config_param_p config_param   = (bt_audio_dongle_runtime_config_param_p)value;
    uint32_t                               i              = 0;
    //uint32_t saved_mask;
    UNUSED(value);
    UNUSED(type);
#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
    if (config_param->config_operation == BT_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_ENABLE) {
        DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_ENABLE, (uint32_t)source->scenario_type, false);
        BT_DONGLE_LOG_I("[BT Audio][DL][config][silence_detection]scenario %d-%d enable silence detection\r\n", 2,
                    source->param.data_dl.scenario_type,
                    source->param.data_dl.scenario_sub_id);
        return true;
    } else if (config_param->config_operation == BT_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_DISABLE) {
        DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_DISABLE, (uint32_t)source->scenario_type, false);
        BT_DONGLE_LOG_I("[BT Audio][DL][config][silence_detection]scenario %d-%d disable silence detection\r\n", 2,
                    source->param.data_dl.scenario_type,
                    source->param.data_dl.scenario_sub_id);
        return true;
    }
#endif

    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            switch (config_param->config_operation) {
                case BT_AUDIO_DONGLE_CONFIG_OP_SET_MUTE:
                    for (i = 0; i < source_info->usb_in.channel_num; i ++) {
                        stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, i + 1, -144 * 100);
                    }
                    BT_DONGLE_LOG_I("[BT Audio][DL][config] set mute, type %d", 1, source->scenario_type);
                    break;
                case BT_AUDIO_DONGLE_CONFIG_OP_SET_UNMUTE:
                    for (i = 0; i < source_info->usb_in.channel_num; i ++) {
                        stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, i + 1, 0);
                    }
                    BT_DONGLE_LOG_I("[BT Audio][DL][config] set un-mute, type %d", 1, source->scenario_type);
                    break;
                default:
                    break;
            }
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            break;
#endif
        default:
            BT_DONGLE_LOG_I("[BT Audio][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    return true;
}

/****************************************************************************************************************************************************/
/*                                                         USB COMMON                                                                               */
/****************************************************************************************************************************************************/
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void bt_audio_dongle_dl_common_init(bt_audio_dongle_handle_t *dongle_handle)
{
    bt_audio_dongle_dl_source_info_t *source_info    = &(dongle_handle->stream_info.dl_info.source);
    bt_audio_dongle_bt_out_info_t    *bt_out         = &(dongle_handle->stream_info.dl_info.sink.bt_out);
    // BT Classic dongle only support 16bit.
    stream_resolution_t              resolution      = (bt_out->sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE) ? RESOLUTION_16BIT : RESOLUTION_32BIT;
    uint32_t                         dl_stream_count = bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_DL);
    bt_audio_dongle_handle_t         *c_handle       = NULL;
    uint32_t                         saved_mask      = 0;
    /* sw mixer init */
    sw_mixer_callback_config_t       callback_config = {0};
    sw_mixer_input_channel_config_t  in_ch_config    = {0};
    sw_mixer_output_channel_config_t out_ch_config   = {0};
    bt_audio_dongle_dl_sink_info_t   *sink_info      = &(dongle_handle->stream_info.dl_info.sink);
    audio_dsp_codec_type_t           codec_type      = sink_info->bt_out.codec_type;
    uint32_t                         source_in_ch    = 0;
    uint32_t                         source_in_rate  = 0;
    /* channel config */
    switch (dongle_handle->source->scenario_type) {
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0...AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            source_in_ch = dongle_handle->source->param.audio.channel_num;
            if (source_in_ch > 2) {
                AUDIO_ASSERT(0 && "[BT Audio][DL] afe in channel number is not right");
            }
            source_in_rate = dongle_handle->source->param.audio.src_rate; // for ul, rate is input, src_rate is output.
            break;
#endif
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            source_in_ch   = source_info->usb_in.channel_num;
            source_in_rate = source_info->usb_in.sample_rate;
            if (source_in_ch > 2) {
                AUDIO_ASSERT(0 && "[BT Audio][DL] usb in channel number is not right");
            }
            break;
        default:
            break;
    }

    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback     = NULL;
    callback_config.postprocess_callback    = bt_audio_dongle_dl_mixer_post_callback;
    in_ch_config.total_channel_number       = source_in_ch;
    in_ch_config.resolution                 = resolution;
    in_ch_config.input_mode                 = SW_MIXER_CHANNEL_MODE_FORCED_ENOUGH;
    in_ch_config.buffer_size                = dongle_handle->src_out_frame_size;
    out_ch_config.total_channel_number      = source_in_ch;
    out_ch_config.resolution                = resolution;
    dongle_handle->mixer_member             = stream_function_sw_mixer_member_create((void *)dongle_handle->source, SW_MIXER_MEMBER_MODE_NO_BYPASS, &callback_config, &in_ch_config, &out_ch_config);
    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, true);
    if (dl_stream_count == 1) {
        /* update mixer status */
        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
        dongle_handle->mixer_status = AUDIO_DONGLE_MIXER_UNMIX;
        hal_nvic_restore_interrupt_mask(saved_mask);
    } else {
        /* it is not the first dl stream, needs to do mixer connection based on BT link info */
        c_handle = bt_audio_dongle_dl_handle_list;
        uint32_t i, j, k;
        for (i=0; i < dl_stream_count; i++) {
            if (c_handle != dongle_handle) {
                for (j = 0; j < BT_AUDIO_DATA_CHANNEL_NUMBER; j++) {
                    if (dongle_handle->stream_info.dl_info.sink.bt_out.bt_info[j].enable) {
                        for (k = 0; k < BT_AUDIO_DATA_CHANNEL_NUMBER; k++) {
                            if (c_handle->stream_info.dl_info.sink.bt_out.bt_info[k].enable) {
                                if (hal_memview_cm4_to_dsp0((uint32_t)dongle_handle->stream_info.dl_info.sink.bt_out.bt_info[j].share_info) ==
                                    hal_memview_cm4_to_dsp0((uint32_t)c_handle->stream_info.dl_info.sink.bt_out.bt_info[k].share_info)) {
                                    /* in here, it means the currnet dongle_handle ch_k should be mixed with the new dongle_handle ch_j */
                                    stream_function_sw_mixer_channel_connect(c_handle->mixer_member, k+1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle->mixer_member, j+1);
                                    stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, j+1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, c_handle->mixer_member, k+1);
                                    stream_function_sw_mixer_channel_connect(c_handle->mixer_member, k+2, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle->mixer_member, j+2);
                                    stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, j+2, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, c_handle->mixer_member, k+2);
                                    /* update mixer status */
                                    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                                    c_handle->mixer_status      = AUDIO_DONGLE_MIXER_MIX;
                                    dongle_handle->mixer_status = AUDIO_DONGLE_MIXER_MIX;
                                    hal_nvic_restore_interrupt_mask(saved_mask);
                                    BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw mixer member 0x%x ch%d connect to [scenario type %d][handle 0x%x] sw mixer member 0x%x ch%d\r\n", 8,
                                        dongle_handle->source->scenario_type,
                                        dongle_handle,
                                        dongle_handle->mixer_member,
                                        j+1,
                                        c_handle->source->scenario_type,
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
            c_handle = c_handle->next_handle;
        }
    }
    BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw mixer 0x%x info, %d, %d, %d, %d, %d, %d, %d\r\n", 10,
        dongle_handle->source->scenario_type,
        dongle_handle,
        dongle_handle->mixer_member,
        in_ch_config.total_channel_number,
        in_ch_config.resolution,
        in_ch_config.input_mode,
        in_ch_config.buffer_size,
        out_ch_config.total_channel_number,
        out_ch_config.resolution,
        dongle_handle->mixer_status
        );

#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
    /* Save the setting of SW mixer */
    if (sink_info->bt_out.without_bt_link_mode_enable) {
        if (bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_DL) == 1) {
            g_bt_audio_dongle_with_out_bt_link_mode = true;
            /* start sw timer */
            bt_audio_dongle_dl_start_sw_timer(dongle_handle);
            BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][scenario type %d] enter without bt link mode", 1,
                dongle_handle->source->scenario_type
                );
        }
    } else {
        if (g_bt_audio_dongle_with_out_bt_link_mode) {
            AUDIO_ASSERT(0 && "[BT Audio][DL] plz check the control flow of with out bt link mode");
        }
        g_bt_audio_dongle_with_out_bt_link_mode = false;
    }
#if SILENCE_DETECTION_IN_SOURCE
    g_bt_audio_dongle_silence_detection_sample_rate = source_info->usb_in.sample_rate;
    g_bt_audio_dongle_silence_detection_channels    = source_info->usb_in.channel_num;
#endif
#if SILENCE_DETECTION_IN_SINK
    g_bt_audio_dongle_silence_detection_sample_rate = bt_out->sample_rate;
    g_bt_audio_dongle_silence_detection_channels    = out_ch_config.total_channel_number;
#endif
    g_bt_audio_dongle_silence_detection_interval = (bt_out->frame_interval + 500) / 1000 * 1000;
    BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][scenario type %d] init parameter %d, %d, %d", 4,
                dongle_handle->source->scenario_type,
                g_bt_audio_dongle_silence_detection_sample_rate,
                g_bt_audio_dongle_silence_detection_interval,
                g_bt_audio_dongle_silence_detection_channels);
    DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_INIT, AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0, false);
#endif

    /* codec init */
    for (uint32_t i = 0; i < sink_info->bt_out.link_num; i++) {
        if (sink_info->bt_out.bt_info[i].enable) {
            if (sink_info->bt_out.bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
                sbc_enc_param_config_t sbc_enc_dl_config = {0};
                sbc_enc_dl_config.param.sampling_rate          = sink_info->bt_out.bt_info[i].codec_param.sbc_enc.sample_rate;
                sbc_enc_dl_config.param.block_number           = sink_info->bt_out.bt_info[i].codec_param.sbc_enc.block_length;
                sbc_enc_dl_config.param.subband_number         = sink_info->bt_out.bt_info[i].codec_param.sbc_enc.subband_num;
                sbc_enc_dl_config.param.channel_mode           = sink_info->bt_out.bt_info[i].codec_param.sbc_enc.channel_mode;
                sbc_enc_dl_config.param.allocation_method      = sink_info->bt_out.bt_info[i].codec_param.sbc_enc.alloc_method;
                sbc_enc_dl_config.param.max_bit_pool           = sink_info->bt_out.bt_info[i].codec_param.sbc_enc.max_bit_pool;
                sbc_enc_dl_config.param.min_bit_pool           = sink_info->bt_out.bt_info[i].codec_param.sbc_enc.min_bit_pool;
                sbc_enc_dl_config.init                         = true;
                sbc_enc_dl_config.input_working_buffer_size    = sink_info->bt_out.frame_size;
                sbc_enc_dl_config.mode                         = SBC_ENCODER_BUFFER_MODE;
                sbc_enc_dl_config.output_working_buffer_size   = bt_audio_dongle_dl_get_stream_out_max_size_each_channel(dongle_handle->source, dongle_handle->sink);
                stream_codec_encoder_sbc_init(&sbc_enc_dl_config);
            }
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
            else if (sink_info->bt_out.bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_LHDC) {
                /* LHDC */
                stream_codec_encoder_lhdc_init(&(sink_info->bt_out.bt_info[i].codec_param.lhdc), dongle_handle->source->scenario_type);
            }
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */
            /* remap buffer */
            /* NOTE: We can only remap share_info, but the internal of share_info can't be remapped, because BT will use it in mcu side!! */
            sink_info->bt_out.bt_info[i].share_info = (void *)hal_memview_cm4_to_dsp0((uint32_t)(sink_info->bt_out.bt_info[i].share_info));
        }
    }
    // update global esco mode
    if (bt_audio_dongle_get_handle_length_by_type(AUDIO_DONGLE_STREAM_TYPE_DL, dongle_handle->link_type) == 1) {
        if (codec_type == AUDIO_DSP_CODEC_TYPE_CVSD) {
            DSP_ALG_UpdateEscoTxMode(VOICE_NB);
            DSP_ALG_UpdateEscoRxMode(VOICE_NB);
        } else if (codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) {
            DSP_ALG_UpdateEscoTxMode(VOICE_WB);
            DSP_ALG_UpdateEscoRxMode(VOICE_WB);
        }
    }
    /* fs convert */
    // for msbc/cvsd/sbc, 2 fixed ratio + sw src
    // for lhdc, 3 fixed ratio
    bool is_bypass_src = (source_in_rate == sink_info->bt_out.sample_rate) ? true : false;
    uint32_t src_fixed_ratio_depth = 2;
    #ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
        if (codec_type == AUDIO_DSP_CODEC_TYPE_LHDC) {
            src_fixed_ratio_depth = 3;
        }
    #endif
    if ((codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) || (codec_type == AUDIO_DSP_CODEC_TYPE_CVSD)) {
        src_fixed_ratio_depth = 3;
    }
    /* 1st src fixed ratio, msbc/cvsd/sbc: 16k/48k/96k -> 48k, 32k -> 96k, 192k -> 96k */
    /* 1st src fixed ratio, lhdc: 16k/48k/96k -> 48k, 32k -> 96k, 192k -> 96k */
    src_fixed_ratio_config_t sw_src0_config  = {0};
    sw_src0_config.channel_number            = source_in_ch;
    sw_src0_config.in_sampling_rate          = source_in_rate;
    sw_src0_config.out_sampling_rate         = is_bypass_src ? source_in_rate :
                                                               bt_audio_dongle_dl_fs_convert_sample_rate_get(source_in_rate, codec_type, 1);
    sw_src0_config.resolution                = resolution;
    sw_src0_config.multi_cvt_mode            = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;
    sw_src0_config.cvt_num                   = src_fixed_ratio_depth;
    sw_src0_config.with_codec                = false;
    sw_src0_config.quality_mode              = AUDIO_SRC_QUALITY_MODE;
    dongle_handle->src0_port                 = stream_function_src_fixed_ratio_get_port(dongle_handle->source);
    dongle_handle->process_sample_rate_max   = MAX(dongle_handle->process_sample_rate_max, sw_src0_config.out_sampling_rate);
    BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw fixed src0 0x%x bypass %d, info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", 15,
                dongle_handle->source->scenario_type,
                dongle_handle,
                dongle_handle->src0_port,
                is_bypass_src,
                sw_src0_config.multi_cvt_mode,
                sw_src0_config.cvt_num,
                sw_src0_config.with_codec,
                sw_src0_config.channel_number,
                sw_src0_config.resolution,
                sw_src0_config.in_sampling_rate,
                sw_src0_config.out_sampling_rate,
                dongle_handle->src_in_frame_samples,
                dongle_handle->src_in_frame_size,
                dongle_handle->src_out_frame_samples,
                dongle_handle->src_out_frame_size
                );
    stream_function_src_fixed_ratio_init((src_fixed_ratio_port_t *)dongle_handle->src0_port, &sw_src0_config);

    /* 2nd src fixed ratio, 48k -> 48k, 96k -> 48k */
    src_fixed_ratio_config_t sw_src1_config = {0};
    sw_src1_config.channel_number            = source_in_ch;
    sw_src1_config.in_sampling_rate          = sw_src0_config.out_sampling_rate;
    sw_src1_config.out_sampling_rate         = is_bypass_src ? sw_src0_config.out_sampling_rate :
                                                               bt_audio_dongle_dl_fs_convert_sample_rate_get(source_in_rate, codec_type, 2);
    sw_src1_config.resolution                = resolution;
    sw_src1_config.multi_cvt_mode            = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;
    sw_src1_config.cvt_num                   = src_fixed_ratio_depth;
    sw_src1_config.with_codec                = false;
    sw_src1_config.quality_mode              = AUDIO_SRC_QUALITY_MODE;
    dongle_handle->src1_port                 = stream_function_src_fixed_ratio_get_2nd_port(dongle_handle->source);
    dongle_handle->process_sample_rate_max   = MAX(dongle_handle->process_sample_rate_max, sw_src1_config.out_sampling_rate);
    BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw fixed src1 0x%x info, %d, %d, %d, %d, %d, %d", 10,
                dongle_handle->source->scenario_type,
                dongle_handle,
                dongle_handle->src1_port,
                sw_src1_config.multi_cvt_mode,
                sw_src1_config.cvt_num,
                sw_src1_config.with_codec,
                sw_src1_config.channel_number,
                sw_src1_config.resolution,
                sw_src1_config.in_sampling_rate,
                sw_src1_config.out_sampling_rate
                );
    stream_function_src_fixed_ratio_init((src_fixed_ratio_port_t *)dongle_handle->src1_port, &sw_src1_config);
    /* 3st sbc: sw src, 48k -> 48k/44.1k/32k/16k/8k */
    /* 3st lhdc: fixed ratio, 48k -> 48k/44.1k/32k/16k/8k */
    if ((codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) || (codec_type == AUDIO_DSP_CODEC_TYPE_CVSD)
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
        || (codec_type == AUDIO_DSP_CODEC_TYPE_LHDC)
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */
    ) {
        /* 3nd src fixed ratio, 48k -> 48k, 96k -> 48k */
        src_fixed_ratio_config_t sw_src2_config = {0};
        sw_src2_config.channel_number            = source_in_ch;
        sw_src2_config.in_sampling_rate          = sw_src1_config.out_sampling_rate;
        sw_src2_config.out_sampling_rate         = is_bypass_src ? sw_src1_config.out_sampling_rate :
                                                                   sink_info->bt_out.sample_rate;
        sw_src2_config.resolution                = resolution;
        sw_src2_config.multi_cvt_mode            = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;
        sw_src2_config.cvt_num                   = src_fixed_ratio_depth;
        sw_src2_config.with_codec                = false;
        sw_src2_config.quality_mode              = AUDIO_SRC_QUALITY_MODE;
        dongle_handle->src_port                  = stream_function_src_fixed_ratio_get_3rd_port(dongle_handle->source);
        dongle_handle->process_sample_rate_max   = MAX(dongle_handle->process_sample_rate_max, sw_src2_config.out_sampling_rate);
        BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw fixed src2 0x%x info, %d, %d, %d, %d, %d, %d", 10,
                    dongle_handle->source->scenario_type,
                    dongle_handle,
                    dongle_handle->src_port,
                    sw_src2_config.multi_cvt_mode,
                    sw_src2_config.cvt_num,
                    sw_src2_config.with_codec,
                    sw_src2_config.channel_number,
                    sw_src2_config.resolution,
                    sw_src2_config.in_sampling_rate,
                    sw_src2_config.out_sampling_rate
                    );
        stream_function_src_fixed_ratio_init((src_fixed_ratio_port_t *)dongle_handle->src_port, &sw_src2_config);
    } else {
        uint32_t format_bytes = audio_dongle_get_format_bytes(sink_info->bt_out.sample_format);
        sw_src_config_t sw_src_config    = {0};
        uint32_t frame_samples_base = 0;
        AUDIO_ASSERT((sink_info->bt_out.sample_rate <= 48000) && "classic audio not support sample rate over 48000");
        sw_src_config.mode               = SW_SRC_MODE_NORMAL;
        sw_src_config.channel_num        = source_in_ch;
        sw_src_config.in_res             = resolution;
        sw_src_config.in_sampling_rate   = sw_src1_config.out_sampling_rate;
        frame_samples_base = dongle_handle->src_out_frame_samples / (sink_info->bt_out.sample_rate / AUDIO_DONGLE_SAMPLE_RATE_8K);
        sw_src_config.in_frame_size_max  = sw_src1_config.out_sampling_rate / AUDIO_DONGLE_SAMPLE_RATE_8K * frame_samples_base;
        sw_src_config.in_frame_size_max *= format_bytes;
        sw_src_config.out_res            = resolution;
        sw_src_config.out_sampling_rate  = sink_info->bt_out.sample_rate;
        sw_src_config.out_frame_size_max = dongle_handle->src_out_frame_size;
        dongle_handle->src_port          = stream_function_sw_src_get_port(dongle_handle->source);
        dongle_handle->process_sample_rate_max   = MAX(dongle_handle->process_sample_rate_max, sw_src_config.out_sampling_rate);
        BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw src 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 11,
                    dongle_handle->source->scenario_type,
                    dongle_handle,
                    dongle_handle->src_port,
                    sw_src_config.mode,
                    sw_src_config.channel_num,
                    sw_src_config.in_res,
                    sw_src_config.in_sampling_rate,
                    sw_src_config.in_frame_size_max,
                    sw_src_config.out_res,
                    sw_src_config.out_sampling_rate,
                    sw_src_config.out_frame_size_max
                    );
        stream_function_sw_src_init((sw_src_port_t *)dongle_handle->src_port, &sw_src_config);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void bt_audio_dongle_dl_common_deinit(bt_audio_dongle_handle_t *dongle_handle)
{
    bt_audio_dongle_handle_t      *c_handle       = NULL;
    uint32_t                      dl_stream_count = bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_DL);
    uint32_t                      saved_mask      = 0;
    uint32_t                      mixer_count     = 0;
    uint32_t                      i               = 0;
    bt_audio_dongle_bt_out_info_t *bt_out         = &(dongle_handle->stream_info.dl_info.sink.bt_out);
    audio_dsp_codec_type_t        codec_type      = bt_out->codec_type;
    /* sw mixer deinit */
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
    c_handle = bt_audio_dongle_dl_handle_list;
    for (i = 0; i < dl_stream_count; i++) {
        if (c_handle->mixer_status == AUDIO_DONGLE_MIXER_MIX) {
            mixer_count += 1;
        }
        /* switch to the next dl stream */
        c_handle = c_handle->next_handle;
    }
    if (mixer_count <= 2) {
        c_handle = bt_audio_dongle_dl_handle_list;
        for (i = 0; i < dl_stream_count; i++) {
            if (c_handle->mixer_status == AUDIO_DONGLE_MIXER_MIX) {
                /* reset mixer status */
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                c_handle->mixer_status = AUDIO_DONGLE_MIXER_UNMIX;
                hal_nvic_restore_interrupt_mask(saved_mask);
            }
            /* switch to the next dl stream */
            BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] disconnect to [scenario type %d][handle 0x%x]", 4,
                dongle_handle->source->scenario_type,
                dongle_handle,
                c_handle->source->scenario_type,
                c_handle
                );
            c_handle = c_handle->next_handle;
        }
    }
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    dongle_handle->mixer_status = AUDIO_DONGLE_MIXER_UNMIX;
    hal_nvic_restore_interrupt_mask(saved_mask);
    /* 1st src fixed ratio */
    stream_function_src_fixed_ratio_deinit((src_fixed_ratio_port_t *)dongle_handle->src0_port);
    /* 2nd src fixed ratio */
    stream_function_src_fixed_ratio_deinit((src_fixed_ratio_port_t *)dongle_handle->src1_port);

    /* codec deinit */
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
    if (codec_type == AUDIO_DSP_CODEC_TYPE_LHDC) {
        /* LHDC */
        stream_codec_encoder_lhdc_deinit(dongle_handle->source->scenario_type);
        /* 3rd src fixed ratio */
        stream_function_src_fixed_ratio_deinit((src_fixed_ratio_port_t *)dongle_handle->src_port);
    } else
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */
    if (codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
        stream_codec_encoder_sbc_deinit();
        stream_function_sw_src_deinit((sw_src_port_t *)dongle_handle->src_port);
    } else if ((codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) || (codec_type == AUDIO_DSP_CODEC_TYPE_CVSD)) {
        /* 3rd src fixed ratio */
        stream_function_src_fixed_ratio_deinit((src_fixed_ratio_port_t *)dongle_handle->src_port);
    } else {
        stream_function_sw_src_deinit((sw_src_port_t *)dongle_handle->src_port);
    }
#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
    if ((g_bt_audio_dongle_with_out_bt_link_mode) && (dl_stream_count == 1)) {
        /* stop timer */
        bt_audio_dongle_dl_stop_sw_timer();
        g_bt_audio_dongle_with_out_bt_link_mode = false;
        BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][scenario type %d] exit without bt link mode", 1,
            dongle_handle->source->scenario_type
            );
    }
    DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_DEINIT, AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0, false);
#endif
}

#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
static void bt_audio_dongle_dl_usb_in_init(
    bt_audio_dongle_handle_t *dongle_handle,
    audio_transmitter_open_param_p audio_transmitter_open_param,
    bt_common_open_param_p bt_common_open_param)
{
    bt_audio_dongle_dl_source_info_t *source_info       = &(dongle_handle->stream_info.dl_info.source);
    bt_audio_dongle_dl_sink_info_t   *sink_info         = &(dongle_handle->stream_info.dl_info.sink);
    stream_resolution_t              resolution         = RESOLUTION_16BIT;
    stream_resolution_t              out_resolution     = RESOLUTION_16BIT;
    uint32_t                         format_bytes       = 0;
    uint32_t                         frame_samples_base = 0;
    /* For clk skew, avoid audio transmitter common flow abandon! */
    dongle_handle->sink->param.bt_common.frame_size = 1;

    /* source info init */
    memcpy(source_info, &(audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.dl_info.source), sizeof(bt_audio_dongle_dl_source_info_t));
    /* sink info init */
    memcpy(sink_info, &(bt_common_open_param->scenario_param.bt_audio_dongle_param.dl_info.sink), sizeof(bt_audio_dongle_dl_sink_info_t));

    format_bytes   = audio_dongle_get_format_bytes(sink_info->bt_out.sample_format);
    resolution     = (source_info->usb_in.sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE) ? RESOLUTION_16BIT : RESOLUTION_32BIT;
    out_resolution = (sink_info->bt_out.sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE) ? RESOLUTION_16BIT : RESOLUTION_32BIT;
    bool ret = bt_audio_dongle_dl_usb_in_check_sample_rate(out_resolution, source_info->usb_in.sample_rate);
    AUDIO_ASSERT(ret && "[BT Audio][DL] fs is not right check usb in sample rate and bt out sample rate");
    /* init src in/out frame size, base on 8k */
    dongle_handle->process_sample_rate_max = MAX(sink_info->bt_out.sample_rate, source_info->usb_in.sample_rate);
    frame_samples_base                   = sink_info->bt_out.frame_interval * AUDIO_DONGLE_SAMPLE_RATE_8K / 1000 / 1000; // 8K base frame
    dongle_handle->src_out_frame_samples = sink_info->bt_out.sample_rate / AUDIO_DONGLE_SAMPLE_RATE_8K * frame_samples_base;
    dongle_handle->src_out_frame_size    = dongle_handle->src_out_frame_samples * format_bytes;
    dongle_handle->src_in_frame_samples  = source_info->usb_in.sample_rate / AUDIO_DONGLE_SAMPLE_RATE_8K * frame_samples_base;
    dongle_handle->src_in_frame_size     = dongle_handle->src_in_frame_samples * format_bytes;
    dongle_handle->usb_frame_size        = dongle_handle->src_in_frame_samples * audio_dongle_get_usb_format_bytes(source_info->usb_in.sample_format);
    source_info->usb_in.frame_max_num    = (sink_info->bt_out.frame_interval + 500) / 1000;
    /* feature list init */
    /* sw buffer init */
    sw_buffer_config_t buffer_config = {0};
    buffer_config.mode                  = SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH;
    buffer_config.total_channels        = source_info->usb_in.channel_num;
    buffer_config.watermark_max_size    = 8 * dongle_handle->src_in_frame_size;
    buffer_config.watermark_min_size    = 0;
    buffer_config.output_size           = dongle_handle->src_in_frame_size;
    dongle_handle->buffer_port_0        = stream_function_sw_buffer_get_unused_port(dongle_handle->source);
    dongle_handle->buffer_default_output_size = dongle_handle->src_in_frame_size; // per irq handle frame size(bytes)
    dongle_handle->buffer0_output_size = dongle_handle->buffer_default_output_size;
    BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw buffer 0x%x info, %d, %d, %d, %d, %d, %d, %d\r\n", 10,
        dongle_handle->source->scenario_type,
        dongle_handle,
        dongle_handle->buffer_port_0,
        buffer_config.mode,
        buffer_config.total_channels,
        buffer_config.watermark_max_size,
        buffer_config.watermark_min_size,
        buffer_config.output_size,
        dongle_handle->buffer_default_output_size,
        dongle_handle->buffer0_output_size
        );
    stream_function_sw_buffer_init(dongle_handle->buffer_port_0, &buffer_config);
    // if (dongle_handle->link_type == BT_AUDIO_TYPE_HFP) {
        // prefill 1ms to clkskew
        for (uint16_t i = 0; i < source_info->usb_in.channel_num; i ++) {
            stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->buffer_port_0, i, 1 * source_info->usb_in.sample_rate / 1000 * format_bytes, true);
        }
    // }

    /* sw clk skew init */
    sw_clk_skew_config_t sw_clk_skew_config = {0};
    dongle_handle->clk_skew_port                = stream_function_sw_clk_skew_get_port(dongle_handle->source);
    sw_clk_skew_config.channel                  = source_info->usb_in.channel_num;
    sw_clk_skew_config.bits = format_bytes * 8; // 16bits or 32bits
    sw_clk_skew_config.order                    = C_Flp_Ord_1;
    sw_clk_skew_config.skew_io_mode             = C_Skew_Inp;
    sw_clk_skew_config.skew_compensation_mode   = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    sw_clk_skew_config.skew_work_mode           = SW_CLK_SKEW_CONTINUOUS; //SW_CLK_SKEW_CONTINUOUS_MULTI_FRAME;
    sw_clk_skew_config.max_output_size          = dongle_handle->src_in_frame_size + format_bytes;
    sw_clk_skew_config.continuous_frame_size    = dongle_handle->src_in_frame_size;
    stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
    /* 1ms threshold value */
    // if (dongle_handle->link_type == BT_AUDIO_TYPE_HFP) {
        dongle_handle->clk_skew_watermark_samples   = source_info->usb_in.sample_rate * 1000 / 1000 / 1000; //dongle_handle->src_in_frame_samples;
    // } else {
    //     dongle_handle->clk_skew_watermark_samples   = 0; // A2DP no need prefill data
    // }
    dongle_handle->clk_skew_compensation_mode   = sw_clk_skew_config.skew_compensation_mode;
    BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw clk skew 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 12,
                dongle_handle->source->scenario_type,
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
                dongle_handle->clk_skew_watermark_samples);
    /* sw gian */
    int32_t default_gain = 0; // -120 dB to mute avoid pop noise
    sw_gain_config_t default_config;
    default_config.resolution               = out_resolution;
    default_config.target_gain              = default_gain; //audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.gain_default_L;
    default_config.up_step                  = 1;
    default_config.up_samples_per_step      = 2;
    default_config.down_step                = -1;
    default_config.down_samples_per_step    = 2;
    dongle_handle->gain_port                = stream_function_sw_gain_get_port(dongle_handle->source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    // default_gain = audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.gain_default_L;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    // default_gain = audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.gain_default_R;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw gain 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 11,
                dongle_handle->source->scenario_type,
                dongle_handle,
                dongle_handle->gain_port,
                default_config.resolution,
                default_config.target_gain,
                default_config.up_step,
                default_config.up_samples_per_step,
                default_config.down_step,
                default_config.down_samples_per_step,
                default_gain, // audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.gain_default_L
                default_gain  // audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.gain_default_R
                );

    UNUSED(audio_transmitter_open_param);
    UNUSED(bt_common_open_param);
}

static void bt_audio_dongle_dl_usb_in_deinit(bt_audio_dongle_handle_t *dongle_handle)
{
    /* sw gain deinit */
    stream_function_sw_gain_deinit(dongle_handle->gain_port);

    /* sw buffer deinit */
    stream_function_sw_buffer_deinit(dongle_handle->buffer_port_0);

    /* sw clk skew deinit */
    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);
}

static void bt_audio_dongle_dl_bt_out_init(
    bt_audio_dongle_handle_t *dongle_handle,
    audio_transmitter_open_param_p audio_transmitter_open_param,
    bt_common_open_param_p bt_common_open_param)
{
    UNUSED(audio_transmitter_open_param);
    UNUSED(bt_common_open_param);
    UNUSED(dongle_handle);
}

static void bt_audio_dongle_dl_bt_out_deinit(bt_audio_dongle_handle_t *dongle_handle)
{
    UNUSED(dongle_handle);
}

ATTR_TEXT_IN_IRAM bool bt_audio_dongle_dl_source_get_avail_size(SOURCE source, uint32_t *avail_size)
{
    bt_audio_dongle_handle_t      *dongle_handle        = (bt_audio_dongle_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    audio_dongle_usb_info_t       *usb_in               = &(dongle_handle->stream_info.dl_info.source.usb_in);
    bt_audio_dongle_bt_out_info_t *bt_out               = &(dongle_handle->stream_info.dl_info.sink.bt_out);
    n9_dsp_share_info_t           *share_info           = &(source->streamBuffer.ShareBufferInfo);
    uint32_t                      unprocess_frames      = 0;
    uint32_t                      saved_mask            = 0;
    // int32_t                       remain_samples_0      = 0;
    uint32_t                      required_size         = 0;
    uint32_t                      required_usb_in_frame = 0;
    /* get actual data size include header size in the share buffer */
    if (dongle_handle->fetch_count == 0) {
        /* there is no fetch requirement, so set avail size as 0 */
        *avail_size = 0;
    } else if (dongle_handle->fetch_count == 0x5A5A) { // ccni request
        *avail_size = audio_dongle_get_n9_share_buffer_data_size_without_header(share_info);
        /* get actual data size exclude header size in the share buffer */
        if (*avail_size != 0) {
            // one channel
            required_size = bt_out->frame_interval * usb_in->sample_rate / 1000 / 1000 * audio_dongle_get_usb_format_bytes(usb_in->sample_format);
            uint32_t ccni_process_frames   = required_size / dongle_handle->usb_frame_size; // how many codec frames should be process this bt ccni irq.
            /* convert to one channel */
            unprocess_frames = *avail_size / usb_in->frame_size;
            required_size = ccni_process_frames * dongle_handle->usb_frame_size;
            // dual channel
            required_size *= usb_in->channel_num;
            required_usb_in_frame = (required_size % usb_in->frame_size) ? (required_size / usb_in->frame_size + 1) : (required_size / usb_in->frame_size);
            if (dongle_handle->first_packet_status == AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY) {
                /* special handle for the first time */
                if (*avail_size < required_size) {
                    BT_DONGLE_LOG_E("[BT Audio][DL][WARNNING][scenario type %d] first time usb in data is not enough, %d < %d", 3,
                        source->scenario_type,
                        *avail_size,
                        required_size
                        );
                    *avail_size = 0;
                    if (hal_nvic_query_exception_number() > 0) {
                        /* update data state machine */
                        dongle_handle->data_status = AUDIO_DONGLE_DL_DATA_EMPTY;
                    }
                    /* decrease fetch count */
                    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                    dongle_handle->fetch_count = 0;
                    hal_nvic_restore_interrupt_mask(saved_mask);
                } else {
                    /* drop un-used data */
                    #if AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE
                        hal_gpio_set_output(GPIO_PIN_FIRST_RECEIVE, 1);
                    #endif /* AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE */
                    uint32_t total_buffer_size = share_info->sub_info.block_info.block_size * share_info->sub_info.block_info.block_num;
                    uint32_t read_offset = (share_info->write_offset + total_buffer_size - required_usb_in_frame * share_info->sub_info.block_info.block_size) %
                                        total_buffer_size;
                    audio_transmitter_share_information_update_read_offset(source, read_offset);
                    if (hal_nvic_query_exception_number() > 0) {
                        /* update data state machine */
                        // dongle_handle->data_status = AUDIO_DONGLE_DL_DATA_IN_STREAM;
                        /* reset fetch count to right value */
                        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                        dongle_handle->fetch_count    = 1;
                        dongle_handle->process_frames = 0;
                        dongle_handle->first_packet_status = AUDIO_DONGLE_DL_FIRST_PACKET_READY;
                        hal_nvic_restore_interrupt_mask(saved_mask);
                    }
                    //*avail_size = dongle_handle->usb_frame_size;
                }
            } else {
                dongle_handle->fetch_count = 1;
                dongle_handle->process_frames = 0;
            }
        } else {
            BT_DONGLE_LOG_E("[BT Audio][DL] there is no enough data from usb", 0);
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->fetch_count = 0;
            hal_nvic_restore_interrupt_mask(saved_mask);
        }
    } else {
        *avail_size = 1;
    }
    if (dongle_handle->fetch_count == 0) {
        *avail_size = 0;
    }
    // if (*avail_size == 0) {
    //     /* send silence to silence detection */
    //     #ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
    //     if (hal_nvic_query_exception_number() > 0) {
    //         uint32_t curr_idx;
    //         uint8_t *p_curr_buf;
    //         bt_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;
    //         silence_detection_handle = &g_bt_audio_dongle_silence_detection_handle;
    //         DSP_STREAMING_PARA_PTR   stream = DSP_Streaming_Get(source, source->transform->sink);
    //         void *para = (void *)&(stream->callback.EntryPara);
    //         if (silence_detection_handle->enable) {
    //             for (curr_idx = 0; curr_idx < stream_function_get_channel_number(para); curr_idx++) {
    //                 if (stream_function_get_output_resolution(para) == RESOLUTION_16BIT) {
    //                     p_curr_buf = (uint8_t *)(silence_detection_handle->data_buf) + curr_idx * stream_function_get_output_size(para) * 2;
    //                     memset(p_curr_buf, 0, stream_function_get_output_size(para) * 2);
    //                     silence_detection_handle->data_size_channel = stream_function_get_output_size(para) * 2;
    //                     silence_detection_handle->data_size = stream_function_get_channel_number(para) * stream_function_get_output_size(para) * 2;
    //                 } else {
    //                     p_curr_buf = (uint8_t *)(silence_detection_handle->data_buf) + curr_idx * stream_function_get_output_size(para);
    //                     memset(p_curr_buf, 0, stream_function_get_output_size(para));
    //                     silence_detection_handle->data_size_channel = stream_function_get_output_size(para);
    //                     silence_detection_handle->data_size = stream_function_get_channel_number(para) * stream_function_get_output_size(para);
    //                 }
    //             }
    //             silence_detection_handle->total_channel = stream_function_get_channel_number(para);
    //             /* send msg to DTM task to do silence detection */
    //             if (DTM_query_queue_avail_number() >= 2) {
    //                 DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_PROCESS, (uint32_t)source->scenario_type, false);
    //             } else {
    //                 BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection] scenario %d-%d abort silence detection, DTM queue full\r\n", 2,
    //                                         source->param.data_dl.scenario_type,
    //                                         source->param.data_dl.scenario_sub_id);
    //             }
    //         }
    //     }
    //     #endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */
    // }
    #if AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE
        hal_gpio_set_output(GPIO_PIN_FIRST_RECEIVE, 0);
    #endif /* AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE */
#if BT_AUDIO_DONGLE_DL_DEBUG_ENABLE
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BT_DONGLE_LOG_I("[BT Audio][DL][DEBUG][scenario type %d][get_avail_size]: %u, %u, %d, %u, %d, %d, %d, %d, %d", 10,
        source->scenario_type,
        dongle_handle->data_status,
        source->streamBuffer.ShareBufferInfo.read_offset,
        source->streamBuffer.ShareBufferInfo.write_offset,
        *avail_size,
        dongle_handle->fetch_count,
        current_timestamp,
        required_size,
        required_usb_in_frame,
        hal_nvic_query_exception_number()
        );
#endif /* BT_AUDIO_DONGLE_DL_DEBUG_ENABLE */
    return true;
}

ATTR_TEXT_IN_IRAM uint32_t bt_audio_dongle_dl_source_copy_payload(SOURCE source, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    DSP_STREAMING_PARA_PTR   stream         = DSP_Streaming_Get(source, source->transform->sink);
    // uint32_t unprocess_frames      = 0;
    uint32_t                         total_samples         = 0;
    uint32_t                         avail_size            = 0;
    uint32_t                         read_offset           = 0;
    uint32_t                         current_frame         = 0;
    uint32_t                         current_frame_samples = 0;
    uint32_t                         total_buffer_size     = 0;
    audio_transmitter_frame_header_t *frame_header         = NULL;
    n9_dsp_share_info_t              *share_info           = &(source->streamBuffer.ShareBufferInfo);
    // uint32_t sw_buffer_remain_size = 0;
    // uint32_t read_size             = length;
    uint32_t read_frames           = 0;
    audio_dongle_usb_info_t *usb_in = &(dongle_handle->stream_info.dl_info.source.usb_in);
    bt_audio_dongle_bt_out_info_t *bt_out = &(dongle_handle->stream_info.dl_info.sink.bt_out);
    uint8_t bytes_per_sample = audio_dongle_get_format_bytes(bt_out->sample_format);
    // BT Classic dongle only support 16bit;
    UNUSED(dst_buf);
    /* copy the pcm data in share buffer into the stream buffers */
    total_buffer_size = share_info->sub_info.block_info.block_size * share_info->sub_info.block_info.block_num;
    avail_size = audio_dongle_get_n9_share_buffer_data_size_without_header(&source->streamBuffer.ShareBufferInfo);
    read_frames = avail_size / usb_in->frame_size;
    read_frames = read_frames > usb_in->frame_max_num ? usb_in->frame_max_num : read_frames;
    total_samples = 0;
    read_offset = source->streamBuffer.ShareBufferInfo.read_offset;
    src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + read_offset);
    current_frame = 0;
    while(current_frame < read_frames) {
        /* get current frame info and current frame samples */
        frame_header = (audio_transmitter_frame_header_t *)src_buf;
        if (frame_header->payload_len != usb_in->frame_size) {
            BT_DONGLE_LOG_E("[BT Audio][DL][ERROR][scenario type %d] frame size is not matched, %u != %u, ro[%d] addr 0x%x", 5,
                source->scenario_type,
                frame_header->payload_len,
                usb_in->frame_size,
                read_offset,
                (uint32_t)src_buf
                );
            AUDIO_ASSERT(0);
        }
        current_frame_samples = usb_in->frame_samples;
        audio_dongle_dl_usb_data_copy(dongle_handle, source->transform->sink, src_buf, current_frame_samples, total_samples, AUDIO_DONGLE_TYPE_BT);
        total_samples += current_frame_samples;
        /* update copied frame number */
        current_frame += 1;
        /* change to the next frame */
        read_offset = (read_offset + source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) % total_buffer_size;
        src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + read_offset);
    }
    /* update stream data size */
    length = total_samples * bytes_per_sample; // only support 16bit
    /* update stream status */
    stream->callback.EntryPara.in_size                  = length;
    stream->callback.EntryPara.in_channel_num           = usb_in->channel_num;
    stream->callback.EntryPara.out_channel_num          = usb_in->channel_num;
    stream->callback.EntryPara.in_sampling_rate         = usb_in->sample_rate/1000;
    if (dongle_handle->stream_info.dl_info.sink.bt_out.sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE) {
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.feature_res   = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.sink_out_res  = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.process_res   = RESOLUTION_16BIT;
    } else {
        // AUDIO_ASSERT(0 && "BT Classic Dongle doesn't support over 16bit.")
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.feature_res   = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.sink_out_res  = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.process_res   = RESOLUTION_32BIT;
    }
    /* update state machine */
    dongle_handle->process_frames = read_frames;
    /* configure clock skew settings */
    dongle_handle->compen_samples = audio_dongle_dl_usb_clock_skew_check(dongle_handle, dongle_handle->process_frames * usb_in->frame_samples,
        &dongle_handle->buffer0_output_size, AUDIO_DONGLE_TYPE_BT);
    stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, dongle_handle->compen_samples);

    /* configure buffer output size, this setting should be fixed */
    stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port_0, 0, dongle_handle->buffer0_output_size);

    /* avoid payload length check error in here */
    length = (length > source->param.data_dl.max_payload_size) ? source->param.data_dl.max_payload_size : length;
    /* update timestamp for debug */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&dongle_handle->data_in_gpt_count);

    // DUMP CH1 BUFFER
    #ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.in_ptr[0]), stream->callback.EntryPara.in_size, AUDIO_BT_SRC_DONGLE_DL_USB_INL);
    LOG_AUDIO_DUMP((uint8_t *)(stream->callback.EntryPara.in_ptr[1]), stream->callback.EntryPara.in_size, AUDIO_BT_SRC_DONGLE_DL_USB_INR);
    #endif
    /* silence detection */
    #ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
    #if SILENCE_DETECTION_IN_SOURCE
        uint32_t curr_idx;
        uint8_t *p_curr_buf;
        bt_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;
        silence_detection_handle = &g_bt_audio_dongle_silence_detection_handle;
        void *para = (void *)&(stream->callback.EntryPara);
        if (silence_detection_handle->enable) {
            for (curr_idx = 0; curr_idx < stream_function_get_channel_number(para); curr_idx++) {
                if (stream_function_get_output_resolution(para) == RESOLUTION_16BIT) {
                    silence_detection_handle->data_size_channel = stream->callback.EntryPara.in_size * 2;
                    p_curr_buf = (uint8_t *)(silence_detection_handle->data_buf) + curr_idx * silence_detection_handle->data_size_channel;
                    ShareBufferCopy_D_16bit_to_D_32bit_1ch((uint16_t* )stream_function_get_inout_buffer(para, curr_idx + 1), (uint32_t* )p_curr_buf, stream->callback.EntryPara.in_size / 2);
                    silence_detection_handle->data_size = stream_function_get_channel_number(para) * silence_detection_handle->data_size_channel;
                } else {
                    silence_detection_handle->data_size_channel = stream->callback.EntryPara.in_size;
                    p_curr_buf = (uint8_t *)(silence_detection_handle->data_buf) + curr_idx * silence_detection_handle->data_size_channel;
                    memcpy(p_curr_buf, stream_function_get_inout_buffer(para, curr_idx + 1), stream->callback.EntryPara.in_size);
                    silence_detection_handle->data_size = stream_function_get_channel_number(para) * silence_detection_handle->data_size_channel;
                }
            }
            silence_detection_handle->total_channel = stream_function_get_channel_number(para);
            /* send msg to DTM task to do silence detection */
            if (DTM_query_queue_avail_number() >= 2) {
                DTM_enqueue(DTM_EVENT_ID_SILENCE_DETECTION_SCENARIO_PROCESS, (uint32_t)source->scenario_type, false);
            } else {
                BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection] scenario %d-%d abort silence detection, DTM queue full\r\n", 2,
                                        source->param.data_dl.scenario_type,
                                        source->param.data_dl.scenario_sub_id);
            }
        }
    #endif /* SILENCE_DETECTION_IN_SOURCE */
    #endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */
#if BT_AUDIO_DONGLE_DL_DEBUG_ENABLE
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BT_DONGLE_LOG_I("[BT Audio][DL][DEBUG][scenario type %d][copy_payload]: %d, %d, %u, %d, %d, %d", 7,
        source->scenario_type,
        dongle_handle->process_frames,
        total_samples,
        stream->callback.EntryPara.in_size,
        bt_audio_dongle_dl_get_stream_in_max_size_each_channel(source, source->transform->sink),
        current_timestamp,
        hal_nvic_query_exception_number()
        );
#endif /* BT_AUDIO_DONGLE_DL_DEBUG_ENABLE */

    return length;
}

ATTR_TEXT_IN_IRAM uint32_t bt_audio_dongle_dl_source_get_new_read_offset(SOURCE source, uint32_t amount)
{
    UNUSED(amount);
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t total_buffer_size;
    n9_dsp_share_info_ptr ShareBufferInfo;
    uint32_t read_offset;

    /* get new read offset */
    ShareBufferInfo = &source->streamBuffer.ShareBufferInfo;
    total_buffer_size = ShareBufferInfo->sub_info.block_info.block_size * ShareBufferInfo->sub_info.block_info.block_num;
    read_offset = (ShareBufferInfo->read_offset+ShareBufferInfo->sub_info.block_info.block_size * dongle_handle->process_frames) % total_buffer_size;

#if BT_AUDIO_DONGLE_DL_DEBUG_ENABLE
    BT_DONGLE_LOG_I("[BT Audio][DL][DEBUG][scenario type %d][get_new_source_ro]:amout %d, ro %d, frames %d, %u", 5,
        source->scenario_type,
        amount,
        read_offset,
        dongle_handle->process_frames,
        hal_nvic_query_exception_number()
        );
#endif /* BT_AUDIO_DONGLE_DL_DEBUG_ENABLE */
    return read_offset;
}

ATTR_TEXT_IN_IRAM void bt_audio_dongle_dl_source_drop_postprocess(SOURCE source, uint32_t amount)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->transform->sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t saved_mask;
    UNUSED(amount);

    /* update first packet state machine */
    if ((dongle_handle->first_packet_status == AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY) && (dongle_handle->process_frames != 0)) {
        dongle_handle->first_packet_status = AUDIO_DONGLE_DL_FIRST_PACKET_READY;
    }

    /* update stream status */
    if (dongle_handle->stream_status == AUDIO_DONGLE_STREAM_START) {
        dongle_handle->stream_status = AUDIO_DONGLE_STREAM_RUNNING;
    }

    /* decrease fetch count */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    dongle_handle->fetch_count --;
    /* check process */
    hal_nvic_restore_interrupt_mask(saved_mask);
}

bool bt_audio_dongle_dl_source_close(SOURCE source)
{
    UNUSED(source);

    return true;
}

ATTR_TEXT_IN_IRAM bool bt_audio_dongle_dl_sink_get_avail_size(SINK sink, uint32_t *avail_size)
{
    // bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;

    UNUSED(sink);
    /* change avail_size to actual frame size */
    /* sink avail size > 0 is ok */
    *avail_size = 0xFFFF;

    return true;
}

ATTR_TEXT_IN_IRAM uint32_t bt_audio_dongle_dl_sink_copy_payload(SINK sink, uint8_t *src_buf, uint32_t length)
{
    bt_audio_dongle_handle_t      *dongle_handle     = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t                      bt_clk             = 0;
    uint16_t                      bt_phase           = 0;
    bt_audio_dongle_bt_out_info_t *bt_out            = &(dongle_handle->stream_info.dl_info.sink.bt_out);
    DSP_STREAMING_PARA_PTR        stream             = DSP_Streaming_Get(sink->transform->source, sink);
    uint32_t                      *src_addr          = NULL;
    uint32_t                      *des_addr          = NULL;
    uint32_t                      blk_size           = 0;
    uint32_t                      blk_num            = 0;
    uint32_t                      blk_index          = 0;
    n9_dsp_share_info_ptr         p_share_info       = 0;
    uint32_t                      payload_size       = sink->param.bt_common.max_payload_size;
    uint32_t                      payload_size_4B    = (payload_size + 3) / 4 * 4;
    BT_AUDIO_HEADER               *p_bt_audio_header = NULL;
    bool                          with_header        = true;
    uint32_t                      process_times      = length / payload_size_4B;
    /* check eSCO or A2DP */
    if (dongle_handle->link_type != BT_AUDIO_TYPE_A2DP) {
        with_header = false;
    }
    /* write DL packet to different share buffer one by one */
    for (uint32_t i = 0; i < BT_AUDIO_DATA_CHANNEL_NUMBER; i++) {
        if (!bt_out->bt_info[i].enable) {
            continue;
        } else {
            /* check parameter */
            if (length % payload_size_4B) {
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
                if (bt_out->bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_LHDC) {
                    payload_size = length;
                    process_times = 1; // LHDC ouput one packet
                } else
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */
                if (bt_out->bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
                    /* the payload size is not fixed */
                    payload_size    = stream_codec_encoder_sbc_get_bit_stream_size();
                    payload_size_4B = (payload_size + 3) / 4 * 4;
                    process_times   = length / payload_size_4B;
                } else {
                    BT_DONGLE_LOG_E("[BT Audio][DL][ERROR][type 0x%x]length is not right, %d, %d", 3, sink->scenario_type, length, payload_size);
                    AUDIO_ASSERT(0);
                }
            }
            src_addr     = stream_function_get_1st_inout_buffer((void *)(&(stream->callback.EntryPara)));
            src_buf      = (uint8_t *)src_addr;
            p_share_info = (n9_dsp_share_info_ptr)((bt_out->bt_info[i]).share_info);
            blk_size     = p_share_info->sub_info.block_info.block_size;
            blk_num      = p_share_info->sub_info.block_info.block_num;
            // check ro valid
            if (p_share_info->write_offset % blk_size) {
                AUDIO_ASSERT(0 && "[BT Audio][DL][ERROR] p_share_info ro is invalid");
            }
            // update dongle blk index wo
            dongle_handle->ccni_blk_index = p_share_info->write_offset / blk_size;
            blk_index    = dongle_handle->ccni_blk_index;
            if (blk_index < blk_num ) {
                /* get header address and data address */
                for (uint32_t j = 0; j < process_times; j ++) {
                    if (with_header) {
                        des_addr = (uint32_t *)(hal_memview_cm4_to_dsp0(p_share_info->start_addr) + blk_size * ((blk_index + j) % blk_num));
                        p_bt_audio_header = (BT_AUDIO_HEADER *)des_addr;
                        des_addr = (uint32_t *)((uint32_t)des_addr + sizeof(BT_AUDIO_HEADER));
                        /* check if blk size is enough */
                        if ((payload_size + sizeof(BT_AUDIO_HEADER)) > blk_size) {
                            BT_DONGLE_LOG_E("[BT Audio][DL][ERROR][Scenario type %d]blk size is not right, %d, %d\r\n", 3, sink->scenario_type, blk_size, payload_size);
                            AUDIO_ASSERT(0);
                        }
                        // BT_DONGLE_LOG_I("[BT Audio][DL] TEST j %d src 0x%x des 0x%x header 0x%x", 4, j,
                        //     src_addr, des_addr, p_bt_audio_header);
                        /* write DL packet data into share buffer block */
                        for (uint32_t k = 0; k < (((uint32_t)(payload_size+3))/4); k ++) {
                            /* share buffer block must be 4B aligned, so we can use word access to get better performance */
                            *des_addr++ = *src_addr++;
                        }
                        // memcpy(des_addr, src_buf, payload_size);
                        /* update state machine */
                        // LOG_AUDIO_DUMP((uint8_t *)src_buf, payload_size, AUDIO_BT_SRC_DONGLE_DL_SINK_OUT_HEADER);
                        p_bt_audio_header->payload_offset = sizeof(BT_AUDIO_HEADER);
                        p_bt_audio_header->data_len       = payload_size;
                        p_bt_audio_header->time_stamp     = dongle_handle->drop_frames * dongle_handle->stream_info.dl_info.sink.bt_out.frame_samples;
                        p_bt_audio_header->_sync_frame_0  = 0xABCD1234;
                        p_bt_audio_header->_sync_frame_1  = 0x5A5AA5A5;
                        (bt_out->bt_info[i]).blk_index_previous = (bt_out->bt_info[i]).blk_index;
                        (bt_out->bt_info[i]).blk_index          = blk_index;
                        #if AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE
                            if (!first_dl_copy_data) {
                                hal_gpio_set_output(GPIO_PIN_FIRST_SEND, 1);
                            }
                        #endif
                        dongle_handle->drop_frames ++;
                    } else {
                        if (payload_size > blk_size) {
                            BT_DONGLE_LOG_E("[BT Audio][DL][ERROR][Scenario type %d]blk size is not right, %d, %d\r\n", 3, sink->scenario_type, blk_size, payload_size);
                            AUDIO_ASSERT(0);
                        }
                        if (payload_size % 4) {
                            AUDIO_ASSERT(0 && "[BT Audio][DL][ERROR]Error not 4B align");
                        }
                        des_addr = (uint32_t *)(hal_memview_cm4_to_dsp0(p_share_info->start_addr) + blk_size * ((blk_index + j) % blk_num));
                        /* write DL packet data into share buffer block */
                        for (uint32_t k = 0; k < (payload_size / 4); k ++) {
                            /* share buffer block must be 4B aligned, so we can use word access to get better performance */
                            *des_addr++ = *src_addr++;
                        }
                    }
                }

            } else {
                BT_DONGLE_LOG_E("[BT Audio][DL][ERROR][Scenario type %d]channel%d blk index is not right, %d, %d\r\n", 4, sink->scenario_type, i+1, blk_index, blk_num);
            }
        }
    }

    MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);

    /* update time stamp */
    dongle_handle->data_out_bt_count = bt_clk;
    /* avoid payload length check error in here */
    length = (length > sink->param.bt_common.max_payload_size) ? sink->param.bt_common.max_payload_size : length;
    return length;
}

ATTR_TEXT_IN_IRAM bool bt_audio_dongle_dl_sink_get_new_write_offset(SINK sink, U32 amount, uint32_t *new_write_offset)
{
    bt_audio_dongle_handle_t      *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    bt_audio_dongle_bt_out_info_t *bt_out        = &(dongle_handle->stream_info.dl_info.sink.bt_out);
    uint32_t                      i,              write_offset = 0;
    n9_dsp_share_info_ptr         p_share_info   = NULL;
    uint32_t                      blk_size       = 0;
    uint32_t                      blk_num        = 0;
    uint32_t                      blk_index      = 0;
    UNUSED(amount);
    UNUSED(new_write_offset);
    uint32_t payload_size    = dongle_handle->sink->param.bt_common.max_payload_size;
    uint32_t payload_size_4B = (payload_size + 3) / 4 * 4;
    uint32_t process_times   = amount / payload_size_4B;
    /* update write index */
    // StreamDSP_HWSemaphoreTake();
    for (i = 0; i < BT_AUDIO_DATA_CHANNEL_NUMBER; i++) {
        if (bt_out->bt_info[i].enable == false) {
            continue;
        } else {
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
            if (bt_out->bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_LHDC) {
                if (amount > 0) {
                    process_times = 1;
                }
            } else
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */
            if (bt_out->bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
                payload_size    = stream_codec_encoder_sbc_get_bit_stream_size();
                payload_size_4B = (payload_size + 3) / 4 * 4;
                process_times   = amount / payload_size_4B;
            }
            p_share_info = (n9_dsp_share_info_ptr)((bt_out->bt_info[i]).share_info);
            blk_size     = p_share_info->sub_info.block_info.block_size;
            blk_num      = p_share_info->sub_info.block_info.block_num;
            blk_index    = dongle_handle->ccni_blk_index;
            if (blk_index < blk_num ) {
                write_offset = (uint32_t)(((blk_index + process_times) % blk_num ) * blk_size);
                p_share_info->write_offset = write_offset;
            }
        }
    }
    // StreamDSP_HWSemaphoreGive();
#if BT_AUDIO_DONGLE_DL_DEBUG_ENABLE
    BT_DONGLE_LOG_I("[BT Audio][DL][DEBUG][scenario type %d][get_new_sink_wo]: amout %d, sink wo %d, ro %d, %u", 5,
        sink->scenario_type,
        amount,
        p_share_info->write_offset,
        p_share_info->read_offset,
        hal_nvic_query_exception_number()
        );
#endif /* BT_AUDIO_DONGLE_DL_DEBUG_ENABLE */
    /* we will update the write offsets of the different share buffers in here directly, so return false to aviod the upper layer update write offset */
    return false;
}

ATTR_TEXT_IN_IRAM bool bt_audio_dongle_dl_sink_query_notification(SINK sink, bool *notification_flag)
{
    UNUSED(sink);

    *notification_flag = false;

    return true;
}

ATTR_TEXT_IN_IRAM bool bt_audio_dongle_dl_sink_send_data_ready_notification(SINK sink)
{
    UNUSED(sink);

    return true;
}

bool bt_audio_dongle_dl_sink_close(SINK sink)
{
    UNUSED(sink);

    return true;
}

#ifdef AUDIO_A2DP_VBR_ENABLE
ATTR_TEXT_IN_IRAM void bt_audio_dongle_dl_sink_buffer_monitor(SINK sink)
{
    bt_audio_dongle_handle_t       *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    n9_dsp_share_info_ptr          p_share_info   = (n9_dsp_share_info_ptr)((dongle_handle->stream_info.dl_info.sink.bt_out.bt_info[0]).share_info);
    bt_audio_dongle_dl_sink_info_t *sink_info     = &(dongle_handle->stream_info.dl_info.sink);
    uint32_t                       block_num      = 0;
    /* monitor sink buffer of a2dp */
    if (dongle_handle->link_type == BT_AUDIO_TYPE_A2DP) {
        block_num = audio_dongle_get_n9_share_buffer_data_size(p_share_info) / p_share_info->sub_info.block_info.block_size;
        if (sink_info->bt_out.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
            /* sbc vbr */
            stream_codec_encoder_sbc_abr_control(block_num, p_share_info->sub_info.block_info.block_num);
        }
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
        else if (sink_info->bt_out.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_LHDC) {
            /* lhdc abr */
            #if AIR_AUDIO_LHDC_ABR_ENABLE
            stream_codec_encoder_lhdc_abr_control(sink->scenario_type, block_num);
            #endif /* AIR_AUDIO_LHDC_ABR_ENABLE */
        }
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */
    }
}
#endif /* AUDIO_A2DP_VBR_ENABLE */

ATTR_TEXT_IN_IRAM void bt_audio_dongle_dl_sink_flush_postprocess(SINK sink, uint32_t amount)
{
    UNUSED(sink);
    UNUSED(amount);
    bt_audio_dongle_handle_t *c_handle        = bt_audio_dongle_dl_handle_list;
    SINK                     c_sink           = NULL;
    bool                     sink_flag        = false;
    uint32_t                 duration         = 0;
    int32_t                  frac_rpt         = 0;
    uint32_t                 remain_samples_0 = 0;
    /* output debug log */
    uint32_t              dl_stream_count   = bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_DL);
    n9_dsp_share_info_ptr p_share_info      = NULL;
    uint32_t              blk_size          = 0;
    uint32_t              blk_num           = 0;
    uint32_t              blk_index         = 0;
    uint32_t              process_frames    = 0;
    uint8_t               src_id            = 0;
    uint32_t              src_tracking_rate = 0;
    uint32_t              source_ro         = 0;
    uint32_t              source_wo         = 0;
    uint32_t              p_share_ro        = 0;
    uint32_t              p_share_wo        = 0;
    for (uint32_t i = 0; i < dl_stream_count; i++) {
        bt_audio_dongle_bt_out_info_t *bt_out = &(c_handle->stream_info.dl_info.sink.bt_out);
        c_sink = (SINK)(c_handle->owner);
        if (c_sink->transform != NULL) {
            if (sink == c_sink) {
                /* this stream is the last stream that all data are mixed */
                sink_flag = true;
                hal_gpt_get_duration_count(c_handle->ccni_in_gpt_count, c_handle->data_out_gpt_count, &duration);
            } else {
                sink_flag = false;
            }
            if ((c_sink->scenario_type == AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0) ||
                (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1) ||
                (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0) ||
                (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1)) {
                // /* Afe in won't use clk skew or sw_buffer */
                remain_samples_0 = stream_function_sw_buffer_get_channel_used_size(c_handle->buffer_port_0, 1);
                stream_function_sw_clk_skew_get_frac_rpt(c_handle->clk_skew_port, 1, &frac_rpt);
                // update
                audio_transmitter_share_information_fetch(c_handle->source, NULL);
                source_ro = c_handle->source->streamBuffer.ShareBufferInfo.read_offset;
                source_wo = c_handle->source->streamBuffer.ShareBufferInfo.write_offset;
            } else if ((c_sink->scenario_type >= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0) &&
                       (c_sink->scenario_type <= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2)) {
                if (c_handle->source->param.audio.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
                    src_id = c_handle->source->param.audio.AfeBlkControl.u4asrcid;
                    src_tracking_rate = AFE_READ(ASM_FREQUENCY_2 + 0x100 * src_id) * 100 / 4096;
                }
                source_ro = c_handle->source->streamBuffer.BufferInfo.ReadOffset;
                source_wo = c_handle->source->streamBuffer.BufferInfo.WriteOffset;
            }
            /* codec out dump */
            if (sink_flag) {
                for (uint32_t j = 0; j < BT_AUDIO_DATA_CHANNEL_NUMBER; j ++) {
                    if (!bt_out->bt_info[j].enable) {
                        continue;
                    } else {
                        p_share_info = (n9_dsp_share_info_ptr)((bt_out->bt_info[j]).share_info);
                        blk_size     = p_share_info->sub_info.block_info.block_size;
                        blk_num      = p_share_info->sub_info.block_info.block_num;
                        blk_index    = p_share_info->write_offset / p_share_info->sub_info.block_info.block_size;
                        process_frames = (blk_index + blk_num - c_handle->ccni_blk_index) % blk_num;
                        for (uint32_t frame = 0; frame < process_frames; frame++) {
                            #ifdef AIR_AUDIO_DUMP_ENABLE
                                uint32_t base_addr = hal_memview_cm4_to_dsp0(p_share_info->start_addr);
                                uint8_t *codec_out_data_address = (uint8_t *)(base_addr + blk_size * ((c_handle->ccni_blk_index + frame) % blk_num));
                                LOG_AUDIO_DUMP((uint8_t *)codec_out_data_address, blk_size, AUDIO_BT_SRC_DONGLE_DL_SINK_OUT_HEADER);
                                LOG_AUDIO_DUMP((uint8_t *)(codec_out_data_address + sizeof(BT_AUDIO_HEADER)), c_sink->param.bt_common.max_payload_size, AUDIO_BT_SRC_DONGLE_DL_SINK_OUT_NO_HEADER);
                            #endif
                        }
                    }
                }
                p_share_ro = p_share_info->read_offset;
                p_share_wo = p_share_info->write_offset;
            }
            BT_DONGLE_LOG_I("[BT Audio][DL][handle 0x%x][scenario type %d] %d, %d, %d, %d, %d, [%d], %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x", 19,
                c_handle,
                c_sink->scenario_type,
                c_handle->fetch_count,
                sink_flag,
                c_handle->mixer_status,
                c_handle->first_packet_status,
                c_handle->data_status,
                process_frames,
                c_handle->ccni_blk_index,
                c_handle->process_frames,
                c_handle->buffer0_output_size,
                c_handle->clk_skew_count,
                frac_rpt,
                c_handle->compen_samples,
                c_handle->drop_frames,
                c_handle->ccni_in_bt_count,
                c_handle->data_out_bt_count,
                c_handle->ccni_in_gpt_count,
                c_handle->data_out_gpt_count
                );
            BT_DONGLE_LOG_I("[BT Audio][DL][handle 0x%x][scenario type %d] %d latency size %d %d source ro [%d] wo [%d] sink ro [%d] wo [%d] src_[%d] rate_[%d]", 11,
                c_handle,
                c_sink->scenario_type,
                c_handle->fetch_count,
                c_handle->latency_size,
                remain_samples_0,
                c_handle->source->streamBuffer.ShareBufferInfo.read_offset,
                c_handle->source->streamBuffer.ShareBufferInfo.write_offset,
                p_share_ro,
                p_share_wo,
                src_id,
                src_tracking_rate
                );
        }
        // update fetch time
        // c_handle->fetch_count --;
        /* switch to the next dl stream */
        c_handle = c_handle->next_handle;
    }
    #if AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE
        if (!first_dl_copy_data) {
            if ((c_sink->scenario_type == AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0) ||
                (c_sink->scenario_type == AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1)) {
                hal_gpio_set_output(GPIO_PIN_FIRST_SEND, 0);
                first_dl_copy_data = true;
            }
        }
    #endif
}

#endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */

uint32_t bt_audio_dongle_dl_get_stream_in_max_size_each_channel(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t       *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    bt_audio_dongle_dl_sink_info_t *sink_info     = &(dongle_handle->stream_info.dl_info.sink);
    audio_dongle_usb_info_t        *usb_in        = &(dongle_handle->stream_info.dl_info.source.usb_in);
    uint32_t                       stream_in_size = 0;
    uint32_t                       in_format_bytes  = audio_dongle_get_format_bytes(usb_in->sample_format);
    uint32_t                       out_format_bytes = audio_dongle_get_format_bytes(sink_info->bt_out.sample_format);
    UNUSED(source);
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            stream_in_size = MAX(dongle_handle->stream_info.dl_info.sink.bt_out.frame_interval * dongle_handle->process_sample_rate_max / 1000 / 1000 * MAX(in_format_bytes, out_format_bytes),
                                 dongle_handle->stream_info.dl_info.sink.bt_out.frame_interval / usb_in->frame_interval * usb_in->frame_size);
            if (sink_info->bt_out.bt_info[0].codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
                stream_in_size += sink_info->bt_out.frame_size;
            }
            break;
#endif
        default:
            UNUSED(stream_in_size);
            UNUSED(usb_in);
            BT_DONGLE_LOG_I("[BT Audio][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    return stream_in_size;
}

uint32_t bt_audio_dongle_dl_get_stream_in_channel_number(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t channel_num = 0;
    UNUSED(source);
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            channel_num = dongle_handle->stream_info.dl_info.source.usb_in.channel_num;
            break;
#endif
        default:
            BT_DONGLE_LOG_I("[BT Audio][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    return channel_num;
}

stream_samplerate_t bt_audio_dongle_dl_get_stream_in_sampling_rate_each_channel(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    stream_samplerate_t sample_rate = 0;
    UNUSED(source);
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            sample_rate = dongle_handle->stream_info.dl_info.source.usb_in.sample_rate / 1000;
            break;
#endif
        default:
            BT_DONGLE_LOG_I("[BT Audio][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    return sample_rate;
}

uint32_t bt_audio_dongle_dl_get_stream_out_channel_number(SOURCE source, SINK sink)
{
    //bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    uint32_t channel_num = 0;
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            channel_num = 1; //dongle_handle->stream_info.dl_info.sink.bt_out.channel_num;
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            channel_num = 1;
            break;
#endif
        default:
            BT_DONGLE_LOG_I("[BT Audio][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    return channel_num;
}

uint32_t bt_audio_dongle_dl_get_stream_out_max_size_each_channel(SOURCE source, SINK sink)
{
    uint32_t stream_out_size = 0;
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            stream_out_size = sink->param.bt_common.max_payload_size;
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            stream_out_size = sink->param.bt_common.max_payload_size;
            break;
#endif
        default:
            BT_DONGLE_LOG_I("[BT Audio][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }
    return stream_out_size;
}

stream_samplerate_t bt_audio_dongle_dl_get_stream_out_sampling_rate_each_channel(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    stream_samplerate_t samplerate = 0;
    UNUSED(sink);
    switch (dongle_handle->source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            samplerate = dongle_handle->stream_info.dl_info.sink.bt_out.sample_rate/1000;
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            samplerate = dongle_handle->stream_info.dl_info.sink.bt_out.sample_rate/1000;
            break;
#endif
        default:
            BT_DONGLE_LOG_I("[BT Audio][DL] scenario type %d is not support!", 1, source->scenario_type);
            AUDIO_ASSERT(0);
            break;
    }

    return samplerate;
}

/****************************************************************************************************************************************************/
/*                                                       BT Source Dongle UL(USB OUT)                                                               */
/****************************************************************************************************************************************************/
ATTR_TEXT_IN_IRAM static void bt_audio_dongle_ul_ccni_handler(hal_ccni_event_t event, void *msg)
{
    SOURCE                   source          = NULL;
    hal_ccni_message_t       *ccni_msg       = msg;
    bt_audio_dongle_handle_t *c_handle       = NULL;
    uint32_t                 saved_mask      = 0;
    uint32_t                 gpt_count       = 0;
    uint32_t                 mcu_gpt_count   = 0;
    uint32_t                 i               = 0;
    uint32_t                 ul_stream_count = 0;
    UNUSED(event);

    /* check if there is any UL stream */
    ul_stream_count = bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_UL);
    if (ul_stream_count == 0) {
        BT_DONGLE_LOG_W("[BT Audio][UL][WARNNING] there is no ul stream", 0);
        goto _ccni_return;
    }
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    /* get timestamp for debug */
    if (!ccni_msg) {
        mcu_gpt_count = ccni_msg->ccni_message[0];
    }
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    hal_nvic_restore_interrupt_mask(saved_mask);
    BT_DONGLE_LOG_I("[BT Audio][UL] ccni irq %d", 1, gpt_count);
    /* trigger all started UL stream one by one */
    c_handle = bt_audio_dongle_ul_handle_list;
    for (i = 0; i < ul_stream_count; i++) {
        source = c_handle->source;
        if ((c_handle->stream_status == AUDIO_DONGLE_STREAM_START) || (c_handle->stream_status == AUDIO_DONGLE_STREAM_RUNNING)) {
            if ((source == NULL) || (source->transform == NULL)) {
                break;
            }
            switch (source->scenario_type) {
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
                case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
                    /* set timestamp for debug */
                    c_handle->ccni_in_bt_count  = mcu_gpt_count;
                    c_handle->ccni_in_gpt_count = gpt_count;
                    // /* increase fetch count */
                    // if (c_handle->first_packet_status == AUDIO_DONGLE_UL_FIRST_PACKET_NOT_READY) {
                    //     /* trigger stream to find out the play time */
                    //     c_handle->fetch_count = 1;
                    // } else if (c_handle->first_packet_status == AUDIO_DONGLE_UL_FIRST_PACKET_READY) {
                    //     /* check if the current bt clock is play time */
                    //     MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);
                    //     if (bt_audio_dongle_ul_fetch_time_is_arrived(c_handle, bt_clk)) {
                    //         c_handle->first_packet_status = AUDIO_DONGLE_UL_FIRST_PACKET_PLAYED;
                    //         /* update share buffer write offset here for more accurate latency */
                    //         audio_transmitter_share_information_fetch(NULL, source->transform->sink);
                    //         write_offset = (source->transform->sink->streamBuffer.ShareBufferInfo.write_offset +
                    //                         source->transform->sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size *
                    //                         c_handle->process_frames) %
                    //                         (source->transform->sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size *
                    //                         source->transform->sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num);
                    //         audio_transmitter_share_information_update_write_offset(source->transform->sink, write_offset);
                    //         c_handle->process_frames = 0;
                    //         c_handle->fetch_count = 0;
                    //         BT_DONGLE_LOG_I("[BT Audio][UL][handle 0x%x] scenario type %d stream is played, %d, 0x%x, 0x%x", 5,
                    //             c_handle,
                    //             source->scenario_type,
                    //             c_handle->process_frames,
                    //             bt_clk,
                    //             write_offset
                    //             );
                    //     }
                    // } else if (c_handle->first_packet_status == AUDIO_DONGLE_UL_FIRST_PACKET_PLAYED) {
                    //     /* trigger stream do something */
                    //     c_handle->fetch_count = 1;
                    // } else {
                    //     c_handle->fetch_count = 0;
                    // }
                    if (c_handle->fetch_count != 0) {
                        BT_DONGLE_LOG_E("[BT Audio][UL][ERROR] fetch count is abnormal %d, process time is not enough!!!", 1, c_handle->fetch_count);
                    }
                    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                    c_handle->fetch_count = 0x5A5A; // 0x5A5A is a symbol to indicate ccni request
                    hal_nvic_restore_interrupt_mask(saved_mask);
                    /* Handler the stream */
                    AudioCheckTransformHandle(source->transform);
                    break;

                default:
                    AUDIO_ASSERT(0);
                    break;
            }
        }

        /* switch to the next dl stream */
        c_handle = c_handle->next_handle;
    }
    if (c_handle != NULL) {
        AUDIO_ASSERT(0);
    }

_ccni_return:
    return;
}

uint32_t bt_audio_dongle_ul_get_stream_in_max_size_each_channel(SOURCE source, SINK sink)
{
    uint32_t stream_in_size = 0;
    UNUSED(source);
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            stream_in_size = source->param.bt_common.max_payload_size;
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }

    return stream_in_size;
}

uint32_t bt_audio_dongle_ul_get_stream_in_channel_number(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    uint32_t channel_number = 0;
    UNUSED(source);
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            // HFP UL is always one channel
            channel_number = dongle_handle->stream_info.ul_info.source.bt_in.channel_num;
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }
    return channel_number;
}

stream_samplerate_t bt_audio_dongle_ul_get_stream_in_sampling_rate_each_channel(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    stream_samplerate_t samplerate = 0;
    UNUSED(source);
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            samplerate = dongle_handle->stream_info.ul_info.source.bt_in.sample_rate / 1000; // attention:44.1/22.05/11.025
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }
    return samplerate;
}

uint32_t bt_audio_dongle_ul_get_stream_out_max_size_each_channel(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    audio_dongle_usb_info_t *usb_out        = &(dongle_handle->stream_info.ul_info.sink.usb_out);
    uint32_t stream_out_size                = 0;
    UNUSED(source);
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            stream_out_size = (dongle_handle->src_out_frame_size % usb_out->frame_size) ? ((dongle_handle->src_out_frame_size / usb_out->frame_size + 1) * usb_out->frame_size) :
                                ((dongle_handle->src_out_frame_size / usb_out->frame_size) * usb_out->frame_size);
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }
    return stream_out_size;
}

uint32_t bt_audio_dongle_ul_get_stream_out_channel_number(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    uint32_t channel_number = 0;
    UNUSED(source);
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            channel_number = MAX(dongle_handle->stream_info.ul_info.sink.usb_out.channel_num, dongle_handle->stream_info.ul_info.source.bt_in.channel_num);
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }
    return channel_number;
}

stream_samplerate_t bt_audio_dongle_ul_get_stream_out_sampling_rate_each_channel(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    stream_samplerate_t samplerate = 0;
    UNUSED(source);
    UNUSED(sink);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            samplerate = dongle_handle->stream_info.ul_info.sink.usb_out.sample_rate / 1000;
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }
    return samplerate;
}

static void bt_audio_dongle_ul_bt_in_init(bt_audio_dongle_handle_t *dongle_handle, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    UNUSED(dongle_handle);
    UNUSED(audio_transmitter_open_param);
    UNUSED(bt_common_open_param);
    /* sink info init */
    memcpy(&(dongle_handle->stream_info.ul_info.sink), &(audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.ul_info.sink), sizeof(bt_audio_dongle_ul_sink_info_t));
}

static void bt_audio_dongle_ul_bt_in_deinit(bt_audio_dongle_handle_t *dongle_handle)
{
    UNUSED(dongle_handle);
}

static void bt_audio_dongle_ul_usb_out_init(bt_audio_dongle_handle_t *dongle_handle, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    UNUSED(dongle_handle);
    UNUSED(audio_transmitter_open_param);
    UNUSED(bt_common_open_param);
    /* sink info init */
    bt_audio_dongle_bt_in_info_t *bt_in   = &(dongle_handle->stream_info.ul_info.source.bt_in);
    audio_dongle_usb_info_t      *usb_out = &(dongle_handle->stream_info.ul_info.sink.usb_out);
    uint32_t bt_frame_size = 0;
    memcpy(&(dongle_handle->stream_info.ul_info.source), &(bt_common_open_param->scenario_param.bt_audio_dongle_param.ul_info.source), sizeof(bt_audio_dongle_ul_source_info_t));
    bt_frame_size = bt_in->frame_interval * bt_in->sample_rate;
    dongle_handle->stream_info.ul_info.sink.usb_out.frame_max_num = (bt_frame_size % dongle_handle->stream_info.ul_info.sink.usb_out.frame_size == 0) ?
                                                                     bt_frame_size / dongle_handle->stream_info.ul_info.sink.usb_out.frame_size :
                                                                     bt_frame_size / dongle_handle->stream_info.ul_info.sink.usb_out.frame_size + 1;

    stream_resolution_t resolution = (bt_in->sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE) ? RESOLUTION_16BIT : RESOLUTION_32BIT;
    uint32_t sample_size = audio_dongle_get_format_bytes(bt_in->sample_format);
    dongle_handle->src_in_frame_samples     = bt_in->frame_samples;
    dongle_handle->src_in_frame_size        = dongle_handle->src_in_frame_samples * sample_size;
    dongle_handle->src_out_frame_samples    = usb_out->sample_rate / 1000 * bt_in->frame_interval / 1000;
    dongle_handle->src_out_frame_size       = dongle_handle->src_out_frame_samples * sample_size;
    dongle_handle->usb_frame_size           = 0;
    /* sw clk skew init */
    sw_clk_skew_config_t sw_clk_skew_config;
    dongle_handle->clk_skew_port = stream_function_sw_clk_skew_get_port(dongle_handle->source);
    sw_clk_skew_config.channel = bt_in->channel_num;
    if (resolution == RESOLUTION_16BIT) {
        sw_clk_skew_config.bits = 16;
    } else {
        sw_clk_skew_config.bits = 32;
    }
    sw_clk_skew_config.order = C_Flp_Ord_5;
    sw_clk_skew_config.skew_io_mode = C_Skew_Oup;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    sw_clk_skew_config.skew_work_mode = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size = 2 * dongle_handle->src_in_frame_size;
    sw_clk_skew_config.continuous_frame_size = dongle_handle->src_in_frame_size;
    stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
    dongle_handle->compen_samples = 0;
    dongle_handle->clk_skew_count = 0;
    /* 1ms threshold value */
    dongle_handle->clk_skew_watermark_samples = bt_in->sample_rate * 1000 / 1000 / 1000;
    BT_DONGLE_LOG_I("[BT Audio][UL][scenario type %d][handle 0x%x]sw clk skew 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 14,
                dongle_handle->source->scenario_type,
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
    buffer_config.mode = SW_BUFFER_MODE_DROP_DATA_WHEN_BUFFER_FULL;
    buffer_config.total_channels = bt_in->channel_num;
    buffer_config.watermark_max_size = 4 * dongle_handle->src_in_frame_size;
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = dongle_handle->src_in_frame_size;
    dongle_handle->buffer_port_0 = stream_function_sw_buffer_get_port(dongle_handle->source);
    stream_function_sw_buffer_init(dongle_handle->buffer_port_0, &buffer_config);
    // prefill 1ms to clkskew
    for (uint16_t i = 0; i < bt_in->channel_num; i ++) {
        stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->buffer_port_0, i, 1 * bt_in->sample_rate / 1000 * sample_size, true);
    }
    dongle_handle->buffer_default_output_size = dongle_handle->src_in_frame_size;
    dongle_handle->buffer0_output_size        = dongle_handle->buffer_default_output_size;
    BT_DONGLE_LOG_I("[BT Audio][UL][scenario type %d][handle 0x%x]sw buffer 0x%x info, %d, %d, %d, %d, %d, %d\r\n", 10,
                dongle_handle->source->scenario_type,
                dongle_handle,
                dongle_handle->buffer_port_0,
                buffer_config.mode,
                buffer_config.total_channels,
                buffer_config.watermark_max_size,
                buffer_config.watermark_min_size,
                buffer_config.output_size,
                dongle_handle->buffer_default_output_size);

    /* sw gain init */
    int32_t default_gain = 0;
    sw_gain_config_t default_config;
    default_config.resolution = resolution;
    default_config.target_gain = default_gain;
    default_config.up_step = 1;
    default_config.up_samples_per_step = 2;
    default_config.down_step = -1;
    default_config.down_samples_per_step = 2;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(dongle_handle->source);
    stream_function_sw_gain_init(dongle_handle->gain_port, bt_in->channel_num, &default_config);
    //default_gain = bt_common_open_param->scenario_param.ull_audio_v2_dongle_param.gain_default_L;
    for (uint8_t i = 0; i < bt_in->channel_num; i ++) {
        stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, i + 1, default_gain);
    }
    BT_DONGLE_LOG_I("[BT Audio][UL][scenario type %d][handle 0x%x]sw gain 0x%x info, %d, %d, %d, %d, %d, %d, %d\r\n", 9,
                dongle_handle->source->scenario_type,
                dongle_handle,
                dongle_handle->gain_port,
                bt_in->channel_num,
                default_config.resolution,
                default_config.target_gain,
                default_config.up_step,
                default_config.up_samples_per_step,
                default_config.down_step,
                default_config.down_samples_per_step
                );
}

static void bt_audio_dongle_ul_usb_out_deinit(bt_audio_dongle_handle_t *dongle_handle)
{
    UNUSED(dongle_handle);

    /* sw gain deinit */
    stream_function_sw_gain_deinit(dongle_handle->gain_port);

    /* sw buffer deinit */
    stream_function_sw_buffer_deinit(dongle_handle->buffer_port_0);

    /* sw clk skew deinit */
    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);
}

static void bt_audio_dongle_ul_common_init(bt_audio_dongle_handle_t *dongle_handle)
{
    bt_audio_dongle_ul_source_info_t *source_info    = &(dongle_handle->stream_info.ul_info.source);
    stream_resolution_t              resolution      = (source_info->bt_in.sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE) ? RESOLUTION_16BIT : RESOLUTION_32BIT;
    // uint32_t                         ul_stream_count = bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_UL);
    // bt_audio_dongle_handle_t         *c_handle       = NULL;
    // uint32_t                         saved_mask      = 0;
    /* sw mixer init */
    sw_mixer_callback_config_t       callback_config = {0};
    sw_mixer_input_channel_config_t  in_ch_config    = {0};
    sw_mixer_output_channel_config_t out_ch_config   = {0};
    bt_audio_dongle_ul_sink_info_t   *sink_info      = &(dongle_handle->stream_info.ul_info.sink);
    audio_dsp_codec_type_t           codec_type      = source_info->bt_in.codec_type;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback     = NULL;
    callback_config.postprocess_callback    = NULL;
    in_ch_config.total_channel_number       = source_info->bt_in.channel_num;
    in_ch_config.resolution                 = resolution;
    in_ch_config.input_mode                 = SW_MIXER_CHANNEL_MODE_FORCED_ENOUGH;
    in_ch_config.buffer_size                = dongle_handle->src_out_frame_size;
    out_ch_config.total_channel_number      = source_info->bt_in.channel_num;
    out_ch_config.resolution                = resolution;
    dongle_handle->mixer_member             = stream_function_sw_mixer_member_create((void *)dongle_handle->source, SW_MIXER_MEMBER_MODE_NO_BYPASS, &callback_config, &in_ch_config, &out_ch_config);
    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, true);
    // if (ul_stream_count == 1) {
    //     /* update mixer status */
    //     hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    //     dongle_handle->mixer_status = AUDIO_DONGLE_MIXER_UNMIX;
    //     hal_nvic_restore_interrupt_mask(saved_mask);
    // } else {
    //     /* it is not the first dl stream, needs to do mixer connection based on BT link info */
    //     c_handle = bt_audio_dongle_ul_handle_list;
    //     uint32_t i, j, k;
    //     for (i=0; i < ul_stream_count; i++) {
    //         if (c_handle != dongle_handle) {
    //             for (j = 0; j < BT_AUDIO_DATA_CHANNEL_NUMBER; j++) {
    //                 if (source_info->bt_in.bt_info[j].enable) {
    //                     for (k = 0; k < BT_AUDIO_DATA_CHANNEL_NUMBER; k++) {
    //                         if (c_handle->stream_info.ul_info.source.bt_in.bt_info[k].enable) {
    //                             if (&dongle_handle->stream_info.ul_info.source.bt_in.bt_info[j] == &c_handle->stream_info.ul_info.source.bt_in.bt_info[k]) {
    //                                 /* in here, it means the currnet dongle_handle ch_k should be mixed with the new dongle_handle ch_j */
    //                                 stream_function_sw_mixer_channel_connect(c_handle->mixer_member, k+1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle->mixer_member, j+1);
    //                                 stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, j+1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, c_handle->mixer_member, k+1);
    //                                 /* update mixer status */
    //                                 hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    //                                 c_handle->mixer_status      = AUDIO_DONGLE_MIXER_MIX;
    //                                 dongle_handle->mixer_status = AUDIO_DONGLE_MIXER_MIX;
    //                                 hal_nvic_restore_interrupt_mask(saved_mask);
    //                                 BT_DONGLE_LOG_I("[BT Audio][UL][scenario type %d][handle 0x%x] sw mixer member 0x%x ch%d connect to [scenario type %d][handle 0x%x] sw mixer member 0x%x ch%d\r\n", 8,
    //                                     dongle_handle->source->scenario_type,
    //                                     dongle_handle,
    //                                     dongle_handle->mixer_member,
    //                                     j+1,
    //                                     c_handle->source->scenario_type,
    //                                     c_handle,
    //                                     c_handle->mixer_member,
    //                                     k+1);
    //                             }
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //         /* switch to the next dl stream */
    //         c_handle = c_handle->next_handle;
    //     }
    // }
    BT_DONGLE_LOG_I("[BT Audio][UL][scenario type %d][handle 0x%x] sw mixer 0x%x info, %d, %d, %d, %d, %d, %d, %d\r\n", 10,
        dongle_handle->source->scenario_type,
        dongle_handle,
        dongle_handle->mixer_member,
        in_ch_config.total_channel_number,
        in_ch_config.resolution,
        in_ch_config.input_mode,
        in_ch_config.buffer_size,
        out_ch_config.total_channel_number,
        out_ch_config.resolution,
        dongle_handle->mixer_status
        );
    /* codec init */
    for (uint32_t i = 0; i < source_info->bt_in.link_num; i++) {
        if (source_info->bt_in.bt_info[i].enable) {
            /* remap buffer */
            /* NOTE: We can only remap share_info, but the internal of share_info can't be remapped, because BT will use it in mcu side!! */
            source_info->bt_in.bt_info[i].share_info = (void *)hal_memview_cm4_to_dsp0((uint32_t)(source_info->bt_in.bt_info[i].share_info));
        }
    }
    /* fs convert */
    if (codec_type == AUDIO_DSP_CODEC_TYPE_CVSD) {
        /* sw src init */
        /* support 8k/16k/32k/48k  16bit */
        sw_src_config_t sw_src_config    = {0};
        sw_src_config.mode               = SW_SRC_MODE_NORMAL;
        sw_src_config.channel_num        = source_info->bt_in.channel_num;
        sw_src_config.in_res             = resolution;
        sw_src_config.in_sampling_rate   = source_info->bt_in.sample_rate;
        sw_src_config.in_frame_size_max  = dongle_handle->src_in_frame_size;
        sw_src_config.out_res            = resolution;
        sw_src_config.out_sampling_rate  = sink_info->usb_out.sample_rate;
        sw_src_config.out_frame_size_max = dongle_handle->src_out_frame_size;
        dongle_handle->src_port          = stream_function_sw_src_get_port(dongle_handle->source);
        BT_DONGLE_LOG_I("[BT Audio][UL][scenario type %d][handle 0x%x]sw src 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 15,
                    dongle_handle->source->scenario_type,
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
        stream_function_sw_src_init((sw_src_port_t *)dongle_handle->src_port, &sw_src_config);
    } else if (codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) {
        /* src fixed ratio init */
        /* support 1:2/1:3/2:1/3:1  16bit/32bit */
        src_fixed_ratio_config_t sw_src_config = {0};
        sw_src_config.channel_number            = 2;
        sw_src_config.in_sampling_rate          = source_info->bt_in.sample_rate;
        sw_src_config.out_sampling_rate         = sink_info->usb_out.sample_rate;
        sw_src_config.resolution                = resolution;
        sw_src_config.multi_cvt_mode            = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_SINGLE;
        sw_src_config.cvt_num                   = 1;
        sw_src_config.with_codec                = false;
        dongle_handle->src_port                 = stream_function_src_fixed_ratio_get_port(dongle_handle->source);
        BT_DONGLE_LOG_I("[BT Audio][UL][scenario type %d][handle 0x%x]sw src 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 14,
                    dongle_handle->source->scenario_type,
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
        stream_function_src_fixed_ratio_init((src_fixed_ratio_port_t *)dongle_handle->src_port, &sw_src_config);
    } else {
        AUDIO_ASSERT(0 && "[BT Audio][UL] error codec type");
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void bt_audio_dongle_ul_common_deinit(bt_audio_dongle_handle_t *dongle_handle)
{
    bt_audio_dongle_handle_t     *c_handle       = NULL;
    uint32_t                     ul_stream_count = bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_UL);
    uint32_t                     saved_mask      = 0;
    uint32_t                     mixer_count     = 0;
    uint32_t                     i               = 0;
    bt_audio_dongle_bt_in_info_t *bt_in          = &(dongle_handle->stream_info.ul_info.source.bt_in);
    audio_dsp_codec_type_t       codec_type      = bt_in->codec_type;
    /* sw mixer deinit */
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
    c_handle = bt_audio_dongle_dl_handle_list;
    for (i = 0; i < ul_stream_count; i++) {
        if (c_handle->mixer_status == AUDIO_DONGLE_MIXER_MIX) {
            mixer_count += 1;
        }
        /* switch to the next dl stream */
        c_handle = c_handle->next_handle;
    }
    if (mixer_count <= 2) {
        c_handle = bt_audio_dongle_ul_handle_list;
        for (i = 0; i < ul_stream_count; i++) {
            if (c_handle->mixer_status == AUDIO_DONGLE_MIXER_MIX) {
                /* reset mixer status */
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                c_handle->mixer_status = AUDIO_DONGLE_MIXER_UNMIX;
                hal_nvic_restore_interrupt_mask(saved_mask);
            }
            /* switch to the next dl stream */
            BT_DONGLE_LOG_I("[BT Audio][UL][scenario type %d][handle 0x%x] disconnect to [scenario type %d][handle 0x%x]", 4,
                dongle_handle->source->scenario_type,
                dongle_handle,
                c_handle->source->scenario_type,
                c_handle
                );
            c_handle = c_handle->next_handle;
        }
    }
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    dongle_handle->mixer_status = AUDIO_DONGLE_MIXER_UNMIX;
    hal_nvic_restore_interrupt_mask(saved_mask);
    /* sw src deinit */
    if (codec_type == AUDIO_DSP_CODEC_TYPE_CVSD) {
        stream_function_sw_src_deinit((sw_src_port_t *)dongle_handle->src_port);
    } else if (codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) {
        stream_function_src_fixed_ratio_deinit((src_fixed_ratio_port_t *)dongle_handle->src_port);
    } else {
        AUDIO_ASSERT(0 && "[BT Audio][UL] error codec type");
    }
}

void bt_audio_dongle_ul_init(SOURCE source, SINK sink, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    /* get handle for application config */
    bt_audio_dongle_handle_t         *dongle_handle      = (bt_audio_dongle_handle_t *)bt_audio_dongle_get_handle(sink, AUDIO_DONGLE_STREAM_TYPE_UL);
    bt_audio_dongle_ul_source_info_t *source_info        = &(dongle_handle->stream_info.ul_info.source);
    source->param.bt_common.scenario_param.dongle_handle = (void *)dongle_handle;
    dongle_handle->source = source;
    dongle_handle->sink   = sink;
    /* task config */
    source->taskId = DHP_TASK_ID;
    sink->taskid   = DHP_TASK_ID;
    BT_DONGLE_LOG_I("[BT Audio][UL] init scenario type %d", 1, source->scenario_type);
    /* init audio info */
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            bt_audio_dongle_ul_bt_in_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            bt_audio_dongle_ul_usb_out_init(dongle_handle, audio_transmitter_open_param, bt_common_open_param);
            dongle_handle->link_type = BT_AUDIO_TYPE_HFP;
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }
    bt_audio_dongle_ul_common_init(dongle_handle);
    if (bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_UL) == 1) {
        /* lock sleep because sleep wake-up time will consume the stream process time */
        hal_sleep_manager_lock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
        /* enable rx forwarder */
        sco_fwd_irq_set_enable(ENABLE_FORWARDER, RX_FORWARDER);
        Forwarder_Rx_Intr_Ctrl(true);
        Forwarder_Rx_Buf_Ctrl(true);
        g_sco_fwd_info.rx_ul_info = (avm_share_buf_info_ptr)(source_info->bt_in.bt_info[0].share_info);
    }
    /* stream status update */
    dongle_handle->stream_status = AUDIO_DONGLE_STREAM_INIT;
}

void bt_audio_dongle_ul_deinit(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    UNUSED(source);
    UNUSED(sink);
    BT_DONGLE_LOG_I("[BT Audio][UL] deinit scenario type %d", 1, source->scenario_type);
    if (bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_UL) == 1) {
        /* lock sleep because sleep wake-up time will consume the stream process time */
        hal_sleep_manager_unlock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
        /* disable rx forwarder */
        sco_fwd_irq_set_enable(DISABLE_FORWARDER, RX_FORWARDER);
        Forwarder_Rx_Intr_Ctrl(false);
        Forwarder_Rx_Buf_Ctrl(false);
        g_sco_fwd_info.is_ul_stream_ready = false;
    }
    /* deinit audio info */
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            bt_audio_dongle_ul_bt_in_deinit(dongle_handle);
            bt_audio_dongle_ul_usb_out_deinit(dongle_handle);
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }
    bt_audio_dongle_ul_common_deinit(dongle_handle);
    /* stream status update */
    dongle_handle->stream_status = AUDIO_DONGLE_STREAM_DEINIT;
    /* release handle */
    bt_audio_dongle_release_handle(dongle_handle, AUDIO_DONGLE_STREAM_TYPE_UL);
    source->param.bt_common.scenario_param.dongle_handle = NULL;
}

void bt_audio_dongle_ul_start(SOURCE source, SINK sink)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    UNUSED(sink);

    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            dongle_handle->fetch_count = 0;
            dongle_handle->first_packet_status = AUDIO_DONGLE_UL_FIRST_PACKET_NOT_READY;
            dongle_handle->data_status = AUDIO_DONGLE_UL_DATA_EMPTY;
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }

    /* stream status update */
    dongle_handle->stream_status = AUDIO_DONGLE_STREAM_START;
}

void bt_audio_dongle_ul_stop(SOURCE source, SINK sink)
{
    UNUSED(sink);
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            dongle_handle->first_packet_status = AUDIO_DONGLE_UL_FIRST_PACKET_NOT_READY;
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }
    dongle_handle->stream_status = AUDIO_DONGLE_STREAM_STOP;
}

bool bt_audio_dongle_ul_config(SOURCE source, stream_config_type type, U32 value)
{
    bt_audio_dongle_handle_t               *dongle_handle = bt_audio_dongle_query_handle_by_scenario_type(source->scenario_type, AUDIO_DONGLE_STREAM_TYPE_DL);
    audio_dongle_usb_info_t                *usb_out       = &(dongle_handle->stream_info.ul_info.sink.usb_out);
    bt_audio_dongle_runtime_config_param_p config_param   = (bt_audio_dongle_runtime_config_param_p)value;
    uint32_t                               i              = 0;
    UNUSED(type);
    UNUSED(value);
    //uint32_t saved_mask;
    UNUSED(value);
    UNUSED(type);

    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            switch (config_param->config_operation) {
                case BT_AUDIO_DONGLE_CONFIG_OP_SET_MUTE:
                    for (i = 0; i < usb_out->channel_num; i ++) {
                        stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, i + 1, -144 * 100);
                    }
                    BT_DONGLE_LOG_I("[BT Audio][UL][config] set mute, type %d", 1, source->scenario_type);
                    break;
                case BT_AUDIO_DONGLE_CONFIG_OP_SET_UNMUTE:
                    for (i = 0; i < usb_out->channel_num; i ++) {
                        stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, i + 1, 0);
                    }
                    BT_DONGLE_LOG_I("[BT Audio][UL][config] set un-mute, type %d", 1, source->scenario_type);
                    break;
                default:
                    break;
            }
            break;
#endif
        default:
            break;
    }
    return true;
}

bool bt_audio_dongle_ul_source_close(SOURCE source)
{
    UNUSED(source);
    return true;
}

// ATTR_TEXT_IN_IRAM static bool bt_audio_dongle_ul_fetch_time_is_arrived(bt_audio_dongle_handle_t *dongle_handle, uint32_t bt_clk)
// {
//     bool ret = false;
//     UNUSED(bt_clk);

//     if ((dongle_handle->first_packet_status == AUDIO_DONGLE_UL_FIRST_PACKET_PLAYED) || (dongle_handle->first_packet_status == AUDIO_DONGLE_UL_FIRST_PACKET_READY)) {
//         ret = true;
//     }

//     return ret;
// }

ATTR_TEXT_IN_IRAM bool bt_audio_dongle_ul_source_get_avail_size(SOURCE source, uint32_t *avail_size)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    n9_dsp_share_info_ptr    p_share_info   = (n9_dsp_share_info_ptr)(g_sco_fwd_info.rx_ul_info);
    uint32_t                 bt_clk         = 0;
    uint16_t                 bt_phase       = 0;
    uint32_t                 buffer_size    = 0;
    uint32_t                 one_frame_size = bt_audio_dongle_ul_get_stream_in_max_size_each_channel(source, NULL);

    /* get current bt clk */
    MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);
    /* get the total buffer size in share buffer */
    *avail_size = audio_dongle_get_n9_share_buffer_data_size(p_share_info);
    if (dongle_handle->fetch_count == 0x5A5A) {
        buffer_size = audio_dongle_get_n9_share_buffer_data_size(p_share_info);
        if (buffer_size < one_frame_size) {
            BT_DONGLE_LOG_E("[BT Audio][UL][WARNNING][scenario type %d] abnormal case size %d < %d", 3,
                source->scenario_type,
                buffer_size,
                one_frame_size
                );
            dongle_handle->fetch_count = 0;
            *avail_size = 0;
        } else {
            if (buffer_size % one_frame_size) {
                BT_DONGLE_LOG_E("[BT Audio][UL][WARNNING][scenario type %d] abnormal case size not align %d %d", 3,
                    source->scenario_type,
                    buffer_size,
                    one_frame_size
                    );
                *avail_size = 0;
                //AUDIO_ASSERT(0 && "[BT Audio][UL] size is not align")
            } else {
                dongle_handle->fetch_count = buffer_size / one_frame_size;
                *avail_size = bt_audio_dongle_ul_get_stream_in_max_size_each_channel(source, NULL);
            }
        }
    } else if (dongle_handle->fetch_count != 0) {
        /* If there is fetch requirement, we must wake up the stream even there is no packet on the share buffer */
        *avail_size = bt_audio_dongle_ul_get_stream_in_max_size_each_channel(source, NULL);
    } else {
        *avail_size = 0;
    }
#if BT_AUDIO_DONGLE_UL_DEBUG_ENABLE
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BT_DONGLE_LOG_I("[BT Audio][UL][DEBUG][scenario type %d][get_avail_size]: %u, %u, %d, %d, %d, %d, %d", 8,
        source->scenario_type,
        dongle_handle->data_status,
        ((n9_dsp_share_info_ptr)(g_sco_fwd_info.rx_ul_info))->read_offset,
        ((n9_dsp_share_info_ptr)(g_sco_fwd_info.rx_ul_info))->write_offset,
        *avail_size,
        dongle_handle->fetch_count,
        current_timestamp,
        hal_nvic_query_exception_number()
        );
#endif /* BT_AUDIO_DONGLE_UL_DEBUG_ENABLE */
    return true;
}

ATTR_TEXT_IN_IRAM uint32_t bt_audio_dongle_ul_source_copy_payload(SOURCE source, uint8_t *dst_buf, uint32_t length)
{
    bt_audio_dongle_handle_t *dongle_handle   = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    DSP_STREAMING_PARA_PTR   stream           = DSP_Streaming_Get(source, source->transform->sink);
    uint32_t                 saved_mask       = 0;
    uint32_t                 i                = 0;
    uint8_t                  *inband_info_ptr = NULL;
    uint8_t                  *src_buf         = NULL;
    n9_dsp_share_info_ptr    p_share_info     = NULL;
    uint32_t                 one_frame_size   = bt_audio_dongle_ul_get_stream_in_max_size_each_channel(source, NULL);

    /* dongle status check */
    switch (dongle_handle->stream_status) {
        /* In this status stage, it means the stream is not ready */
        case AUDIO_DONGLE_STREAM_DEINIT:
        case AUDIO_DONGLE_STREAM_INIT:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->fetch_count = 0;
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        /* In this status stage, we will check if the newest packet is suitable for playing */
        case AUDIO_DONGLE_STREAM_START:
            /* update stream status */
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->first_packet_status = AUDIO_DONGLE_UL_FIRST_PACKET_READY;
            dongle_handle->stream_status = AUDIO_DONGLE_STREAM_RUNNING;
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        /* In this status stage, the stream is started and we will set flag to fetch a new packet */
        case AUDIO_DONGLE_STREAM_RUNNING:
            break;

        /* Error status */
        default:
            AUDIO_ASSERT(0);
            break;
    }

    /* copy one frame to stream in buffer */
    for (i = 0; i < BT_AUDIO_DATA_CHANNEL_NUMBER; i++) {
        if (dongle_handle->stream_info.ul_info.source.bt_in.bt_info[i].enable == false) {
            continue;
        } else {
            uint32_t read_offset = 0;
            uint32_t index       = 0;
            p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->stream_info.ul_info.source.bt_in.bt_info[i]).share_info);
            uint32_t source_buf_len = p_share_info->sub_info.block_info.block_size * p_share_info->sub_info.block_info.block_num;
            uint8_t process_num = one_frame_size / p_share_info->sub_info.block_info.block_size;
            VOICE_RX_INBAND_INFO_t rx_packet_info;
            for (uint8_t j = 0; j < process_num; j ++) {
                read_offset = (p_share_info->read_offset + j * p_share_info->sub_info.block_info.block_size) % source_buf_len;
                index       = read_offset / p_share_info->sub_info.block_info.block_size;
                dst_buf = (uint8_t *)stream->callback.EntryPara.in_ptr[i] + p_share_info->sub_info.block_info.block_size * j;
                src_buf = (uint8_t *)(hal_memview_cm4_to_dsp0(p_share_info->start_addr) + read_offset);
                inband_info_ptr = (uint8_t *)(hal_memview_cm4_to_dsp0(p_share_info->start_addr) + source_buf_len + AUDIO_DSP_SCO_INBAND_INFORMATION * index);
                rx_packet_info = (((VOICE_RX_SINGLE_PKT_STRU_PTR_t)inband_info_ptr)->InbandInfo);
                /* copy frame data into the stream buffer */
                if (Voice_PLC_CheckInfoValid((VOICE_RX_SINGLE_PKT_STRU_PTR_t)inband_info_ptr) == false) {
                    BT_DONGLE_LOG_W("[BT Audio][UL][WARNING] source meet packet expired", 0);
                    Voice_PLC_CheckAndFillZeroResponse((S16 *)(dst_buf), gDspAlgParameter.EscoMode.Rx);
                } else {
                    memcpy(dst_buf, src_buf, p_share_info->sub_info.block_info.block_size);
                }
                Voice_PLC_UpdateInbandInfo(&rx_packet_info, sizeof(VOICE_RX_INBAND_INFO_t), j);
            }
        }
    }
    if (dongle_handle->data_status == AUDIO_DONGLE_UL_DATA_EMPTY) {
        dongle_handle->data_status = AUDIO_DONGLE_UL_DATA_NORMAL;
    }
    stream->callback.EntryPara.in_size = one_frame_size;
    /* configure clock skew settings */
    if (dongle_handle->data_status != AUDIO_DONGLE_UL_DATA_BYPASS_DECODER) {
        dongle_handle->compen_samples = audio_dongle_ul_usb_clock_skew_check(dongle_handle, AUDIO_DONGLE_TYPE_BT);
        stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, dongle_handle->compen_samples);
    } else {
        stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, 0);
    }

    /* configure buffer output size */
    stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port_0, 0, dongle_handle->buffer0_output_size);
#if BT_AUDIO_DONGLE_UL_DEBUG_ENABLE
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BT_DONGLE_LOG_I("[BT Audio][UL][DEBUG][scenario type %d][copy_payload]: %u, %u, %d, %u, %d, %d", 7,
        source->scenario_type,
        dongle_handle->data_status,
        ((n9_dsp_share_info_ptr)(g_sco_fwd_info.rx_ul_info))->read_offset,
        ((n9_dsp_share_info_ptr)(g_sco_fwd_info.rx_ul_info))->write_offset,
        dongle_handle->fetch_count,
        current_timestamp,
        hal_nvic_query_exception_number()
        );
#endif /* BT_AUDIO_DONGLE_DL_DEBUG_ENABLE */
    return length;
}

bool bt_audio_dongle_ul_source_get_new_read_offset(SOURCE source, U32 amount, U32 *new_read_offset)
{
    bt_audio_dongle_handle_t     *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    bt_audio_dongle_bt_in_info_t *bt_in         = &(dongle_handle->stream_info.ul_info.source.bt_in);
    uint32_t i;
    n9_dsp_share_info_ptr p_share_info;
    uint32_t blk_size;
    uint32_t blk_num;
    // uint32_t blk_index;
    UNUSED(amount);
    UNUSED(new_read_offset);

    if (dongle_handle->drop_frames != 0) {
        for (i = 0; i < BT_AUDIO_DATA_CHANNEL_NUMBER; i++)  {
            if (dongle_handle->stream_info.ul_info.source.bt_in.bt_info[i].enable == false) {
                continue;
            } else {
                /* get blk info */
                uint8_t block_num_one_process = bt_in->frame_interval / BT_AUDIO_UL_ANCHOR_INTERVAL;
                p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->stream_info.ul_info.source.bt_in.bt_info[i]).share_info);
                blk_size     = p_share_info->sub_info.block_info.block_size;
                blk_num      = p_share_info->sub_info.block_info.block_num;
                /* get new blk index */
                // (dongle_handle->stream_info.ul_info.source.bt_in.bt_info[i]).blk_index_previous = (dongle_handle->stream_info.ul_info.source.bt_in.bt_info[i]).blk_index;
                // blk_index = ((dongle_handle->stream_info.ul_info.source.bt_in.bt_info[i]).blk_index_previous + 1) % blk_num;
                // (dongle_handle->stream_info.ul_info.source.bt_in.bt_info[i]).blk_index = blk_index;
                /* update read index */
                // StreamDSP_HWSemaphoreTake();
                *new_read_offset = (p_share_info->read_offset + blk_size * block_num_one_process) % (blk_size * blk_num);
                // StreamDSP_HWSemaphoreGive();
            }
        }
    }
#if BT_AUDIO_DONGLE_UL_DEBUG_ENABLE
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BT_DONGLE_LOG_I("[BT Audio][UL][DEBUG][scenario type %d][get_new_read_offset]: %u, %u, %d, %d, %u, %d, %d", 8,
        source->scenario_type,
        dongle_handle->data_status,
        ((n9_dsp_share_info_ptr)(g_sco_fwd_info.rx_ul_info))->read_offset,
        ((n9_dsp_share_info_ptr)(g_sco_fwd_info.rx_ul_info))->write_offset,
        *new_read_offset,
        dongle_handle->fetch_count,
        current_timestamp,
        hal_nvic_query_exception_number()
        );
#endif /* BT_AUDIO_DONGLE_DL_DEBUG_ENABLE */
    /* we will update the read offsets of the different share buffers in here directly, so return false to aviod the upper layer update read offset */
    return false;
}

ATTR_TEXT_IN_IRAM void bt_audio_dongle_ul_source_drop_postprocess(SOURCE source, uint32_t amount)
{
    bt_audio_dongle_handle_t     *dongle_handle = (bt_audio_dongle_handle_t *)source->param.bt_common.scenario_param.dongle_handle;
    DSP_STREAMING_PARA_PTR       stream         = DSP_Streaming_Get(source, source->transform->sink);
    bt_audio_dongle_bt_in_info_t *bt_in         = &(dongle_handle->stream_info.ul_info.source.bt_in);
    uint32_t                     i              = 0;
    // uint32_t                 read_offset        = 0;
    n9_dsp_share_info_ptr p_share_info       = NULL;
    uint32_t              blk_size           = 0;
    uint32_t              blk_num            = 0;
    uint32_t              saved_mask         = 0;
    uint32_t              bt_clk             = 0;
    uint16_t              bt_phase           = 0;
    uint32_t              remain_samples     = 0;
    uint32_t              sample_size        = 0;
    uint32_t              out_free_size      = 0;
    UNUSED(amount);
    UNUSED(stream);

    /* update time stamp */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&dongle_handle->data_out_gpt_count);

    /* get sample size */
    sample_size = audio_dongle_get_format_bytes(dongle_handle->stream_info.ul_info.source.bt_in.sample_format);
    if ((dongle_handle->first_packet_status == AUDIO_DONGLE_UL_FIRST_PACKET_READY) && (dongle_handle->stream_status = AUDIO_DONGLE_STREAM_RUNNING)) {
        dongle_handle->first_packet_status = AUDIO_DONGLE_UL_FIRST_PACKET_PLAYED;
    }
    /* add debug log */
    MCE_GetBtClk((BTCLK*)&bt_clk, (BTPHASE*)&bt_phase, BT_CLK_Offset);
    switch (source->scenario_type) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            audio_dongle_sink_get_share_buffer_avail_size(source->transform->sink, &out_free_size);
            break;
#endif
        default:
            AUDIO_ASSERT(0 && "[BT Audio][UL] scenario type not support");
            break;
    }
    if (dongle_handle->buffer_port_0 != NULL) {
        remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port_0, 1) / sample_size;
    }
    BT_DONGLE_LOG_I("[BT Audio][UL][scenario type %d][handle 0x%x][source_drop_postprocess] %d, %d, %d, %d, %d, [%d], %d, %d, 0x%x, 0x%x, 0x%x, 0x%x, %d, %d, %d, %d", 17,
                    source->scenario_type,
                    dongle_handle,
                    dongle_handle->fetch_count,
                    dongle_handle->stream_status,
                    dongle_handle->first_packet_status,
                    dongle_handle->data_status,
                    dongle_handle->drop_frames,
                    out_free_size,
                    bt_clk,
                    dongle_handle->buffer0_output_size/sample_size,
                    remain_samples,
                    dongle_handle->clk_skew_count,
                    dongle_handle->compen_samples,
                    dongle_handle->ccni_in_bt_count,
                    dongle_handle->data_out_bt_count,
                    dongle_handle->ccni_in_gpt_count,
                    dongle_handle->data_out_gpt_count);
    /* drop packets */
    for (i = 0; i < BT_AUDIO_DATA_CHANNEL_NUMBER; i++) {
        if (dongle_handle->stream_info.ul_info.source.bt_in.bt_info[i].enable == false) {
            continue;
        } else {
                uint8_t blk_num_one_process = bt_in->frame_interval / BT_AUDIO_UL_ANCHOR_INTERVAL;
                p_share_info = (n9_dsp_share_info_ptr)((dongle_handle->stream_info.ul_info.source.bt_in.bt_info[i]).share_info);
                blk_size     = p_share_info->sub_info.block_info.block_size;
                blk_num      = p_share_info->sub_info.block_info.block_num;
                /* update read index */
                StreamDSP_HWSemaphoreTake();
                p_share_info->read_offset = (p_share_info->read_offset + blk_size * blk_num_one_process) % (blk_size * blk_num);
                StreamDSP_HWSemaphoreGive();
        }
        // dongle_handle->drop_frames ++;
    }

    /* decrease fetch count */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->fetch_count != 0) {
        dongle_handle->fetch_count -= 1;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
}

ATTR_TEXT_IN_IRAM bool bt_audio_dongle_ul_sink_get_avail_size(SINK sink, uint32_t *avail_size)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)(sink->transform->source->param.bt_common.scenario_param.dongle_handle);
    uint32_t frame_num;
    uint32_t one_bt_frame_size = dongle_handle->src_out_frame_size;
    uint32_t one_usb_frame_size = (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t));
    audio_dongle_sink_get_share_buffer_avail_size(sink, avail_size);
    frame_num = one_bt_frame_size / one_usb_frame_size + 1;
    if (*avail_size < (frame_num * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size)) {
         BT_DONGLE_LOG_E("[BT Audio][UL][scenario type %d][ERROR] usb out free space is not enough, size %d frame %d", 3,
            sink->scenario_type,
            *avail_size,
            frame_num
            );
        // AUDIO_ASSERT(0);
    }
#if BT_AUDIO_DONGLE_UL_DEBUG_ENABLE
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BT_DONGLE_LOG_I("[BT Audio][UL][DEBUG][scenario type %d][sink_size]: %d, %d, %d, %d, %d, %d, %d", 7,
        sink->scenario_type,
        dongle_handle->data_status,
        sink->streamBuffer.ShareBufferInfo.write_offset,
        sink->streamBuffer.ShareBufferInfo.read_offset,
        *avail_size,
        current_timestamp,
        hal_nvic_query_exception_number()
        );
#endif /* BT_AUDIO_DONGLE_UL_DEBUG_ENABLE */
    return true;
}

ATTR_TEXT_IN_IRAM uint32_t bt_audio_dongle_ul_sink_copy_payload(SINK sink, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)(sink->transform->source->param.bt_common.scenario_param.dongle_handle);
    DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(sink->transform->source, sink);
    uint32_t process_frames = 0;
    uint32_t saved_mask = 0;
    uint32_t i = 0;
    uint32_t payload_size = 0;
    bool half_period_flag = false;
    uint32_t total_size = 0;
    uint8_t  *cur_ptr = NULL;
    uint16_t copy_size = 0;
    uint32_t copy_offset   = 0;
    // sample_size = audio_dongle_get_format_bytes(dongle_handle->stream_info.ul_info.source.bt_in.sample_format);
    UNUSED(src_buf);
    total_size = sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;
    uint32_t one_usb_frame_size = (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t));
    if ((length % one_usb_frame_size) == (one_usb_frame_size / 2)) {
        // 0.5ms case
        half_period_flag = true;
        /* prefill */
        if (dongle_handle->first_packet_status == AUDIO_DONGLE_UL_FIRST_PACKET_READY) {
            BT_DONGLE_LOG_I("[BT Audio][UL][scenario type %d] prefill %d frames", 2,
                    sink->scenario_type,
                    BT_AUDIO_UL_PREFILL_FRAME_NUM
                    );
            for (uint32_t j = 0; j < BT_AUDIO_UL_PREFILL_FRAME_NUM; j ++) {
                uint8_t *ptr = (uint8_t *)(sink->streamBuffer.ShareBufferInfo.start_addr +
                    ((sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * j) % total_size));
                ((audio_transmitter_frame_header_t *)ptr)->seq_num      = dongle_handle->stream_info.ul_info.sink.usb_out.seq_num;
                ((audio_transmitter_frame_header_t *)ptr)->payload_len  = dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples * sizeof(int16_t);
                dongle_handle->stream_info.ul_info.sink.usb_out.seq_num = (dongle_handle->stream_info.ul_info.sink.usb_out.seq_num + 1) & 0xffff;
            }
            StreamDSP_HWSemaphoreTake();
            uint32_t write_offset = (sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size *
                BT_AUDIO_UL_PREFILL_FRAME_NUM) % (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size *
                sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num);
            sink->streamBuffer.ShareBufferInfo.write_offset = write_offset;
            StreamDSP_HWSemaphoreGive();
        }
    }

    process_frames = length / dongle_handle->stream_info.ul_info.sink.usb_out.frame_size;
    if (half_period_flag) {
        process_frames += 1;
    }

    for (i = 0; i < process_frames; i++) {
        dst_buf = (uint8_t *)(sink->streamBuffer.ShareBufferInfo.start_addr +
                    ((sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * i) % total_size));
        if ((half_period_flag) && (dongle_handle->data_status == AUDIO_DONGLE_UL_DATA_SINK_HALF_PERIOD) && (i == 0)) {
            /* get dst buffer */
            // 0.5ms
            cur_ptr     = dst_buf + sizeof(audio_transmitter_frame_header_t) + one_usb_frame_size / 2;
            copy_size   = dongle_handle->stream_info.ul_info.sink.usb_out.frame_size / 2;
        } else if ((half_period_flag) && (dongle_handle->data_status == AUDIO_DONGLE_UL_DATA_NORMAL) && (i == (process_frames - 1))) {
            cur_ptr     = dst_buf + sizeof(audio_transmitter_frame_header_t);
            copy_size   = dongle_handle->stream_info.ul_info.sink.usb_out.frame_size / 2;
        } else {
            cur_ptr     = dst_buf + sizeof(audio_transmitter_frame_header_t);
            copy_size   = dongle_handle->stream_info.ul_info.sink.usb_out.frame_size;
        }
        if ((dongle_handle->stream_info.ul_info.sink.usb_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) &&
            (dongle_handle->stream_info.ul_info.source.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)) {
            if ((dongle_handle->stream_info.ul_info.sink.usb_out.channel_num == 1) &&
                (dongle_handle->stream_info.ul_info.source.bt_in.channel_num == 1)) {
                // ShareBufferCopy_D_16bit_to_I_16bit_2ch( (uint16_t *)(stream->callback.EntryPara.out_ptr[0]) + i * cur_samples,
                //                                         (uint16_t *)(stream->callback.EntryPara.out_ptr[0]) + i * cur_samples,
                //                                         (uint16_t *)(cur_ptr),
                //                                         cur_samples);
                memcpy(cur_ptr, stream->callback.EntryPara.out_ptr[0] + copy_offset, copy_size);
                payload_size = dongle_handle->stream_info.ul_info.sink.usb_out.frame_size;
                // LOG_AUDIO_DUMP(cur_ptr, (dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples - cur_samples) * 2, AUDIO_BT_SRC_DONGLE_UL_USB_OUT);
                copy_offset += copy_size;
            }
        } else {
            BT_DONGLE_LOG_E("[BT Audio][UL][scenario type %d][ERROR]sample_format is not supported, %u, %u", 3,
                sink->scenario_type,
                dongle_handle->stream_info.ul_info.sink.usb_out.sample_format,
                dongle_handle->stream_info.ul_info.source.bt_in.sample_format);
            AUDIO_ASSERT(0);
        }
        /* write seq number and payload_len into the share buffer */
        if ((half_period_flag) && (dongle_handle->data_status == AUDIO_DONGLE_UL_DATA_NORMAL) && (i == (process_frames - 1))) {

        } else {
            ((audio_transmitter_frame_header_t *)dst_buf)->seq_num      = dongle_handle->stream_info.ul_info.sink.usb_out.seq_num;
            ((audio_transmitter_frame_header_t *)dst_buf)->payload_len  = payload_size;
            /* update seq number */
            dongle_handle->stream_info.ul_info.sink.usb_out.seq_num = (dongle_handle->stream_info.ul_info.sink.usb_out.seq_num + 1) & 0xffff;
        }

    }
    // for (i = 0; i < process_frames; i++) {
    //     /* get dst buffer */
    //     dst_buf = (uint8_t *)(sink->streamBuffer.ShareBufferInfo.start_addr +
    //                         ((sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * i) %
    //                         (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num)));

    //     if ((dongle_handle->stream_info.ul_info.sink.usb_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) &&
    //         (dongle_handle->stream_info.ul_info.source.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)) {
    //         /* copy pcm samples into the share buffer */
    //         if (dongle_handle->stream_info.ul_info.sink.usb_out.channel_num == 2) {
    //             ShareBufferCopy_D_16bit_to_I_16bit_2ch( (uint16_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint16_t *)(stream->callback.EntryPara.out_ptr[1])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint16_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
    //                                                     dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples);
    //             payload_size = dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples*2*sizeof(int16_t);
    //         } else {
    //             ShareBufferCopy_D_16bit_to_D_16bit_1ch( (uint16_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint16_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
    //                                                     dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples);
    //             payload_size = dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples*sizeof(int16_t);
    //         }
    //     } else if ((dongle_handle->stream_info.ul_info.sink.usb_out.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) &&
    //                (dongle_handle->stream_info.ul_info.source.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)) {
    //         /* copy pcm samples into the share buffer */
    //         if (dongle_handle->stream_info.ul_info.sink.usb_out.channel_num == 2) {
    //             ShareBufferCopy_D_32bit_to_I_16bit_2ch( (uint32_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint32_t *)(stream->callback.EntryPara.out_ptr[1])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint16_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
    //                                                     dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples);
    //             payload_size = dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples*2*sizeof(int16_t);
    //         } else {
    //             ShareBufferCopy_D_32bit_to_D_16bit_1ch( (uint32_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint16_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
    //                                                     dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples);
    //             payload_size = dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples*sizeof(int16_t);
    //         }
    //     } else if ((dongle_handle->stream_info.ul_info.sink.usb_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) &&
    //                (dongle_handle->stream_info.ul_info.source.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)) {
    //         /* copy pcm samples into the share buffer */
    //         if (dongle_handle->stream_info.ul_info.sink.usb_out.channel_num == 2) {
    //             ShareBufferCopy_D_16bit_to_I_24bit_2ch( (uint16_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint16_t *)(stream->callback.EntryPara.out_ptr[1])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint8_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
    //                                                     dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples);
    //             payload_size = dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples*2*3;
    //         } else {
    //             ShareBufferCopy_D_16bit_to_D_24bit_1ch( (uint16_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint8_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
    //                                                     dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples);
    //             payload_size = dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples*3;
    //         }
    //     } else if ((dongle_handle->stream_info.ul_info.sink.usb_out.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) &&
    //                (dongle_handle->stream_info.ul_info.source.bt_in.sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)) {
    //         /* copy pcm samples into the share buffer */
    //         if (dongle_handle->stream_info.ul_info.sink.usb_out.channel_num == 2) {
    //             ShareBufferCopy_D_32bit_to_I_24bit_2ch( (uint32_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint32_t *)(stream->callback.EntryPara.out_ptr[1])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint8_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
    //                                                     dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples);
    //             payload_size = dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples*2*3;
    //         } else {
    //             ShareBufferCopy_D_32bit_to_D_24bit_1ch( (uint32_t *)(stream->callback.EntryPara.out_ptr[0])+i*dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples,
    //                                                     (uint8_t *)(dst_buf+sizeof(audio_transmitter_frame_header_t)),
    //                                                     dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples);
    //             payload_size = dongle_handle->stream_info.ul_info.sink.usb_out.frame_samples*3;
    //         }
    //     } else {
    //         BT_DONGLE_LOG_E("[BT Audio][UL][ERROR]sample_format is not supported, %u, %u", 2, dongle_handle->stream_info.ul_info.sink.usb_out.sample_format, dongle_handle->stream_info.ul_info.source.bt_in.sample_format);
    //         AUDIO_ASSERT(0);
    //     }


    // }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    /* If stream is not played, only copy data into share buffer but not update write offset */
    if (dongle_handle->data_status != AUDIO_DONGLE_UL_DATA_SINK_HALF_PERIOD) {
        if (half_period_flag == true) {
            process_frames -= 1;
        }
    }
    if (dongle_handle->data_status == AUDIO_DONGLE_UL_DATA_SINK_HALF_PERIOD) {
        dongle_handle->data_status = AUDIO_DONGLE_UL_DATA_NORMAL;
    } else if (dongle_handle->data_status == AUDIO_DONGLE_UL_DATA_NORMAL) {
        if (half_period_flag) {
            dongle_handle->data_status = AUDIO_DONGLE_UL_DATA_SINK_HALF_PERIOD;
        }
    }
    dongle_handle->process_frames = process_frames;
    hal_nvic_restore_interrupt_mask(saved_mask);
#if BT_AUDIO_DONGLE_UL_DEBUG_ENABLE
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BT_DONGLE_LOG_I("[BT Audio][UL][DEBUG][scenario type %d][sink_copy]: %u, 0x%x, %u, %d, %u, %d, %d, %d, %d, %d", 11,
        sink->scenario_type,
        dongle_handle->data_status,
        sink->streamBuffer.ShareBufferInfo.start_addr,
        sink->streamBuffer.ShareBufferInfo.write_offset,
        sink->streamBuffer.ShareBufferInfo.read_offset,
        dongle_handle->fetch_count,
        process_frames,
        dongle_handle->process_frames,
        payload_size,
        current_timestamp,
        hal_nvic_query_exception_number()
        );
#endif /* BT_AUDIO_DONGLE_DL_DEBUG_ENABLE */
    /* we should return > 0 to trigger audio transmitter streaming */
    return payload_size;
}

ATTR_TEXT_IN_IRAM void bt_audio_dongle_ul_sink_query_write_offset(SINK sink, uint32_t *write_offset)
{
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)(sink->transform->source->param.bt_common.scenario_param.dongle_handle);
    uint32_t saved_mask;
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (dongle_handle->first_packet_status != AUDIO_DONGLE_UL_FIRST_PACKET_NOT_READY) {

        /* If stream is not played, update write offset */
        *write_offset = (sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size *
            dongle_handle->process_frames) % (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size *
            sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num);
        audio_transmitter_share_information_update_write_offset(sink, *write_offset);
    }
    //else {
    //     /* If stream is not played, only copy data into share buffer but not update write offset */
    //     *write_offset = sink->streamBuffer.ShareBufferInfo.write_offset;
    // }
    hal_nvic_restore_interrupt_mask(saved_mask);

    /* update time stamp */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&dongle_handle->data_out_bt_count);

#if BT_AUDIO_DONGLE_DL_DEBUG_ENABLE
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BT_DONGLE_LOG_I("[BT Audio][UL][DEBUG][sink_query_write_offset]: %u, %u, %d, %d", 4, dongle_handle->first_packet_status, *write_offset, current_timestamp, hal_nvic_query_exception_number());
#endif
#if BT_AUDIO_DONGLE_UL_DEBUG_ENABLE
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    BT_DONGLE_LOG_I("[BT Audio][UL][DEBUG][scenario type %d][sink_get_new_wo]: %d, %d, %d, %d, %d, %d", 6,
        sink->scenario_type,
        dongle_handle->data_status,
        *write_offset,
        dongle_handle->process_frames,
        current_timestamp,
        hal_nvic_query_exception_number()
        );
#endif /* BT_AUDIO_DONGLE_UL_DEBUG_ENABLE */
}

bool bt_audio_dongle_ul_sink_close(SINK sink)
{
    UNUSED(sink);

    return true;
}
/****************************************************************************************************************************************************/
/*                                                       eSCO Flow: RX/TX Forwarder                                                                 */
/****************************************************************************************************************************************************/
static uint32_t sco_get_tx_fwd_pkt_type(sco_pkt_type *type)
{
    uint32_t pkt_num;
    uint16_t RxDataLen = Forwarder_Get_TX_FWD_Pattern_Size(gDspAlgParameter.EscoMode.Tx);

    if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
        RxDataLen = RxDataLen / 2;
    }

    switch (RxDataLen) {
        case SCO_PKT_2EV3_LEN:
            *type = SCO_PKT_2EV3;
            pkt_num = 1;
            break;
        case SCO_PKT_EV3_LEN:
            *type = SCO_PKT_EV3;
            pkt_num = 2;
            break;
        case SCO_PKT_HV2_LEN:
            *type = SCO_PKT_HV2;
            pkt_num = 3;
            break;
        case SCO_PKT_HV1_LEN:
            *type = SCO_PKT_HV1;
            pkt_num = 6;
            break;
        default:
            *type = SCO_PKT_2EV3;
            pkt_num = 1;
    }

    return pkt_num;
}

static hal_nvic_status_t sco_fwd_irq_set_enable(fowarder_ctrl forwarder_en, fowarder_type forwarder_type)
{
    hal_nvic_status_t ret = HAL_NVIC_STATUS_OK;

    if (forwarder_type == RX_FORWARDER) {
        if (forwarder_en == ENABLE_FORWARDER) {
            ret = hal_nvic_disable_irq(BT_AURX_IRQn);
            if (ret != HAL_NVIC_STATUS_OK) {
                AUDIO_ASSERT(0 && "[BT Audio] disable rx fwd irq error");
                return ret;
            }
            ret = hal_nvic_register_isr_handler(BT_AURX_IRQn, (hal_nvic_isr_t)sco_fwd_rx_ul_irq_handler);
            if (ret != HAL_NVIC_STATUS_OK) {
                AUDIO_ASSERT(0 && "[BT Audio] register rx fwd irq error");
                return ret;
            }
            ret = hal_nvic_enable_irq(BT_AURX_IRQn);
            if (ret != HAL_NVIC_STATUS_OK) {
                AUDIO_ASSERT(0 && "[BT Audio] enable rx fwd irq error");
                return ret;
            }
            Forwarder_Rx_Intr_Ctrl(false);
            Forwarder_Rx_Reset_Disconnect_Status();
            BT_DONGLE_LOG_I("[BT Audio][Rx FWD] registerd callback handler done!", 0);
        } else {
            ret = hal_nvic_disable_irq(BT_AURX_IRQn);
            if (ret != HAL_NVIC_STATUS_OK) {
                AUDIO_ASSERT(0 && "[BT Audio] enable rx fwd irq error");
                return ret;
            }
            Forwarder_Rx_Intr_Ctrl(false);
            BT_DONGLE_LOG_I("[BT Audio][Rx FWD] un-registerd callback handler done!", 0);
        }
    } else if (forwarder_type == TX_FORWARDER) {
        if (forwarder_en == ENABLE_FORWARDER) {
            ret = hal_nvic_disable_irq(BT_AUTX_IRQn);
            if (ret != HAL_NVIC_STATUS_OK) {
                AUDIO_ASSERT(0 && "[BT Audio] disable tx fwd irq error");
                return ret;
            }
            ret = hal_nvic_register_isr_handler(BT_AUTX_IRQn, (hal_nvic_isr_t)sco_fwd_tx_dl_irq_handler);
            if (ret != HAL_NVIC_STATUS_OK) {
                AUDIO_ASSERT(0 && "[BT Audio] register tx fwd irq error");
                return ret;
            }
            ret = hal_nvic_enable_irq(BT_AUTX_IRQn);
            if (ret != HAL_NVIC_STATUS_OK) {
                AUDIO_ASSERT(0 && "[BT Audio] enable tx fwd irq error");
                return ret;
            }
            Forwarder_Tx_Intr_Ctrl(false);
            BT_DONGLE_LOG_I("[BT Audio][Tx FWD] registerd callback handler done!", 0);

        } else {
            ret = hal_nvic_disable_irq(BT_AUTX_IRQn);
            if (ret != HAL_NVIC_STATUS_OK) {
                AUDIO_ASSERT(0 && "[BT Audio] disable tx fwd irq error");
                return ret;
            }
            Forwarder_Tx_Intr_Ctrl(false);
            Forwarder_Tx_Buf_Ctrl(false);
            BT_DONGLE_LOG_I("[BT Audio][Tx FWD] un-registerd callback handler done!", 0);
        }
    } else {
        AUDIO_ASSERT(0 && "[BT Audio] forwarder_type error");
    }
    return ret;
}

/* For mSBC (AirMode: 3)
*   packet type: 2EV3, packet size: 60, packet number: 1, interval: 7.5ms
*   packet type: EV3,  packet size: 30, packet number: 2, interval: 3.75ms
* For CVSD (AirMode: 2)
*   packet type: 2EV3, packet size: 120, packet number: 1, interval: 7.5ms
*   packet type: EV3,  packet size: 60, packet number: 2, interval: 3.75ms
*   packet type: HV2,  packet size: 30, packet number: 3, interval: 2.5ms
*   packet type: HV1,  packet size: 20, packet number: 4, interval: 1.25ms
*/
// for dongle side, rx is uplink
ATTR_TEXT_IN_IRAM static void sco_fwd_rx_ul_irq_handler(void)
{
    uint32_t rx_forwarder_gpt_time = 0;
    uint32_t interval_gpt_time     = 0;
    uint32_t spec_interval         = 0;
    uint32_t pattern_frame_size    = 0;
    uint32_t curr_rx_fwd_buf_idx   = 0;
    VOICE_RX_SINGLE_PKT_STRU_PTR_t inbaud_info_1, inbaud_info_2;
    uint32_t                 ul_stream_count = 0;
    SOURCE                   source          = NULL;
    bool                     is_ul_ready     = false;
    bt_audio_dongle_handle_t *c_handle       = NULL;
    // mask RX fwd irq flag
    Forwarder_Rx_Intr_HW_Handler();
    ul_stream_count = bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_UL);
    if (ul_stream_count == 0) {
        BT_DONGLE_LOG_W("[BT Audio][UL][WARNNING] there is no ul stream", 0);
    }
    /* check rx fwd status */
    if ((ul_stream_count == 0) || (Forwarder_Rx_Check_Disconnect_Status() == true)) { // Rx/Tx is the same
        BT_DONGLE_LOG_W("[BT Audio][UL][RX FWD] detect controller disconnect", 0);
        Forwarder_Rx_Reset_Disconnect_Status();
        hal_nvic_disable_irq(BT_AURX_IRQn);
        Forwarder_Rx_Intr_Ctrl(false);
        Forwarder_Tx_Intr_HW_Handler();
        hal_nvic_disable_irq(BT_AUTX_IRQn);
        Forwarder_Tx_Intr_Ctrl(false);
        return;
    }
    /* check ul stream status */
    if (!g_sco_fwd_info.is_ul_stream_ready) {
        c_handle = bt_audio_dongle_ul_handle_list;
        for (uint32_t i = 0; i < ul_stream_count; i ++) {
            source = c_handle->source;
            if ((c_handle->stream_status == AUDIO_DONGLE_STREAM_START) || (c_handle->stream_status == AUDIO_DONGLE_STREAM_RUNNING)) {
                if ((c_handle->sink == NULL) || (source->transform == NULL) || (c_handle->source == NULL)) {
                    break;
                }
                is_ul_ready = true;
            }
            c_handle = c_handle->next_handle;
        }
        g_sco_fwd_info.is_ul_stream_ready = is_ul_ready;
    }
    if (!g_sco_fwd_info.is_ul_stream_ready) {
        BT_DONGLE_LOG_W("[BT Audio][UL][RX FWD] ul stream is not ready", 0);
        return;
    }

    /* Get frame size, frame type, frame number */
    pattern_frame_size = Forwarder_Get_RX_FWD_Pattern_Size(gDspAlgParameter.EscoMode.Rx);
    if (pattern_frame_size == 0) {
        BT_DONGLE_LOG_W("[BT Audio][UL][RX FWD] pattern_frame_size == 0", 0);
        return;
    }
    /* Check Rx fwd irq interval */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &rx_forwarder_gpt_time);
    if (g_sco_fwd_info.rx_ul_fwd_irq_process_cnt != 0) {
        hal_gpt_get_duration_count(g_sco_fwd_info.rx_ul_fwd_irq_gpt_cnt, rx_forwarder_gpt_time, &interval_gpt_time);
        spec_interval = (pattern_frame_size / 10) * 1250; /* 10 -> 1.25ms, 20 -> 2.5ms, 30 -> 3.75ms, 60 -> 7.5ms */
        if (gDspAlgParameter.EscoMode.Rx == VOICE_NB) {
            spec_interval /= 2;
        }
        if (interval_gpt_time < (spec_interval / 5)) {
            BT_DONGLE_LOG_W("[BT Audio][UL][RX FWD] interval too close: %d us", 1, interval_gpt_time);
            g_sco_fwd_info.rx_ul_fwd_irq_gpt_cnt = rx_forwarder_gpt_time;
            return;
        } else if ((interval_gpt_time > (spec_interval + 500)) || (interval_gpt_time < (spec_interval - 500))) {
            BT_DONGLE_LOG_W("[BT Audio][UL][RX FWD] interval is not stable, expect %d, actual us: %d", 2, spec_interval, interval_gpt_time);
        }
    }
    g_sco_fwd_info.rx_ul_fwd_irq_gpt_cnt = rx_forwarder_gpt_time;
    /* check stream status */
    /* Fetch the AVM buffer info and forwarder buffer info
     * AVM buffer layout: frame 1#, frame 2#, ..., frame n#, inbaud for frame 1#, ..., inbaud for frame n#
     * forwarder buffer layout: inbaud for frame 1#, frame 1#, frame 2#, frame 2# (keep fixed 140 byte for each frame including inbaud(20B))
     */
    /* Check Rx fwd buffer index */
    curr_rx_fwd_buf_idx = Forwarder_Rx_Status();
    if (((curr_rx_fwd_buf_idx != 0) && (curr_rx_fwd_buf_idx != 1)) || (g_sco_fwd_info.rx_ul_fwd_buf_index_pre == curr_rx_fwd_buf_idx)) {
        BT_DONGLE_LOG_W("[BT Audio][UL][RX FWD] get forwarder buffer index error, prev status:%d, current status:%d", 2, g_sco_fwd_info.rx_ul_fwd_buf_index_pre, curr_rx_fwd_buf_idx);
        // g_sco_fwd_info.rx_ul_fwd_buf_index_pre = curr_rx_fwd_buf_idx;
        // return;
    }
    g_sco_fwd_info.rx_ul_fwd_buf_index_pre = curr_rx_fwd_buf_idx;
    /* Copy the packet from the Rx forwarder hardware buffer to source buffer */
    n9_dsp_share_info_ptr  p_share_info           = (n9_dsp_share_info_ptr)(g_sco_fwd_info.rx_ul_info);
    avm_share_buf_info_ptr p_avm_info             = (avm_share_buf_info_ptr)(g_sco_fwd_info.rx_ul_info);
    uint32_t               source_buf_len         = p_share_info->sub_info.block_info.block_size * p_share_info->sub_info.block_info.block_num;
    uint8_t                *forwarder_packet_buf  = (uint8_t *)(hal_memview_cm4_to_dsp0(p_avm_info->ForwarderAddr) + curr_rx_fwd_buf_idx * (AUDIO_DSP_FORWARD_BUFFER_SIZE + AUDIO_DSP_SCO_INBAND_INFORMATION));
    uint8_t                *forwarder_pattern_buf = (uint8_t *)(forwarder_packet_buf + AUDIO_DSP_SCO_INBAND_INFORMATION);
    uint32_t               p_share_info_base_addr = hal_memview_cm4_to_dsp0(p_share_info->start_addr);
    uint8_t                *source_buf            = (uint8_t *)(p_share_info_base_addr + p_share_info->write_offset);
    uint8_t                *source_inband_buf     = (uint8_t *)(p_share_info_base_addr + source_buf_len +
                                                                AUDIO_DSP_SCO_INBAND_INFORMATION * (((uint32_t)p_share_info->write_offset) / (g_sco_fwd_info.pattern_frame_size * g_sco_fwd_info.pkt_type)));
    #ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP(forwarder_pattern_buf, pattern_frame_size, AUDIO_BT_SRC_DONGLE_UL_BT_IN);
    #endif
    /* Copy Inband Info from forwarder buffer to AVM buffer */
    if (g_sco_fwd_info.pkt_type != SCO_PKT_2EV3) {
        if (g_sco_fwd_info.rx_ul_fwd_frame_id == 1) {
            DSP_D2C_BufferCopy(source_inband_buf, forwarder_packet_buf, AUDIO_DSP_SCO_INBAND_INFORMATION, (void *)(p_share_info_base_addr + source_buf_len),
                AUDIO_DSP_SCO_INBAND_INFORMATION * p_share_info->sub_info.block_info.block_num);
        } else {
            inbaud_info_1 = (VOICE_RX_SINGLE_PKT_STRU_PTR_t)source_inband_buf;
            inbaud_info_2 = (VOICE_RX_SINGLE_PKT_STRU_PTR_t)forwarder_packet_buf;
            inbaud_info_1->InbandInfo.CrcErr |= inbaud_info_2->InbandInfo.CrcErr;
            inbaud_info_1->InbandInfo.HecErr |= inbaud_info_2->InbandInfo.HecErr;
            inbaud_info_1->InbandInfo.RxEd &= inbaud_info_2->InbandInfo.RxEd;
        }
        g_sco_fwd_info.rx_ul_fwd_frame_id ++;
        if (g_sco_fwd_info.rx_ul_fwd_frame_id > g_sco_fwd_info.pkt_num_per_frame) {
            g_sco_fwd_info.rx_ul_fwd_frame_id = 1;
        }
    } else {
        DSP_D2C_BufferCopy(source_inband_buf, forwarder_packet_buf, AUDIO_DSP_SCO_INBAND_INFORMATION,
            (void *)(p_share_info_base_addr + source_buf_len), AUDIO_DSP_SCO_INBAND_INFORMATION * p_share_info->sub_info.block_info.block_num);
    }
    /* Copy pattern from forwarder buffer to AVM buffer */
    DSP_D2C_BufferCopy(source_buf, forwarder_pattern_buf, pattern_frame_size,
        (void *)p_share_info_base_addr, (uint16_t)p_share_info->sub_info.block_info.block_size * p_share_info->sub_info.block_info.block_num);
    if (gDspAlgParameter.EscoMode.Rx == VOICE_WB) {
        if ((*(uint8_t *)forwarder_pattern_buf != 0x01) || (!msbc_subseqn_check(forwarder_pattern_buf))) { // 0x01 is msbc sync word
            BT_DONGLE_LOG_W("[BT Audio][UL][RX FWD][WARNNING] msbc header fail 0x%x 0x%x", 2,
                *(uint8_t *)forwarder_pattern_buf,
                *(uint8_t *)(forwarder_pattern_buf + 1)
            );
        }
    }
    /* Clean Fwd Pattern and inbaud Info */
    memset(forwarder_pattern_buf, 0, pattern_frame_size);
    /* Update the WPTR of source buffer */
    uint32_t source_buf_next_wo = (p_share_info->write_offset + (uint32_t)pattern_frame_size) % source_buf_len;
    // StreamDSP_HWSemaphoreTake();
    p_share_info->write_offset = source_buf_next_wo;
    // StreamDSP_HWSemaphoreGive();
#if BT_AUDIO_DONGLE_UL_DEBUG_ENABLE
    BT_DONGLE_LOG_I("[BT Audio][UL][RX FWD][DEBUG] p_share_info addr 0x%x wo %d ro %d", 3,
        source_buf,
        p_share_info->write_offset,
        p_share_info->read_offset
        );
#endif /* BT_AUDIO_DONGLE_UL_DEBUG_ENABLE */

    g_sco_fwd_info.forwarder_pkt_num ++;
    if (Voice_PLC_CheckInfoValid((VOICE_RX_SINGLE_PKT_STRU_PTR_t)forwarder_packet_buf) == false) {
        g_sco_fwd_info.lost_pkt_num ++;
    }
    if (g_sco_fwd_info.forwarder_pkt_num == 200) {
        DSP_MW_LOG_I("[RX FWD] lost packet: %d per %d pkt", 2, g_sco_fwd_info.lost_pkt_num, g_sco_fwd_info.forwarder_pkt_num);
        g_sco_fwd_info.lost_pkt_num = 0;
        g_sco_fwd_info.forwarder_pkt_num = 0;
    }

    // trigger stream dl flow
    Voice_PLC_CleanInfo((VOICE_RX_SINGLE_PKT_STRU_PTR_t)forwarder_packet_buf);
    // if (g_sco_fwd_info.rx_ul_fwd_irq_process_cnt % 2 != 0) { // 1 to do
        bt_audio_dongle_ul_ccni_handler(0, NULL);
    // }
    g_sco_fwd_info.rx_ul_fwd_irq_process_cnt ++;
}

// for dongle side, tx is downlink
ATTR_TEXT_IN_IRAM static void sco_fwd_tx_dl_irq_handler(void)
{
    uint32_t                       pattern_frame_size    = 0;
    uint32_t                       tx_forwarder_gpt_time = 0;
    uint32_t                       curr_tx_fwd_buf_idx   = 0;
    uint32_t                       spec_interval         = 0;
    uint32_t                       interval_gpt_time     = 0;
    uint32_t                       dl_stream_count       = 0;
    bt_audio_dongle_handle_t       *c_handle             = NULL;
    bool                           is_dl_ready           = false;
    // mask TX fwd irq flag
    Forwarder_Tx_Intr_HW_Handler();
    dl_stream_count = bt_audio_dongle_get_handle_length(AUDIO_DONGLE_STREAM_TYPE_DL);
    if (dl_stream_count == 0) {
        BT_DONGLE_LOG_W("[BT Audio][UL][WARNNING] there is no ul stream", 0);
    }
    if ((dl_stream_count == 0) || Forwarder_Rx_Check_Disconnect_Status()) { // Rx/Tx is the same
        BT_DONGLE_LOG_W("[BT Audio][DL][TX FWD] detect controller disconnect", 0);
        Forwarder_Rx_Reset_Disconnect_Status();
        Forwarder_Rx_Intr_HW_Handler();
        hal_nvic_disable_irq(BT_AURX_IRQn);
        Forwarder_Rx_Intr_Ctrl(false);
        hal_nvic_disable_irq(BT_AUTX_IRQn);
        Forwarder_Tx_Intr_Ctrl(false);
        return;
    }
    /* check dl stream status */
    if (!g_sco_fwd_info.is_dl_stream_ready) {
        c_handle = bt_audio_dongle_dl_handle_list;
        for (uint32_t i = 0; i < dl_stream_count; i ++) {
            if ((c_handle->stream_status == AUDIO_DONGLE_STREAM_START) || (c_handle->stream_status == AUDIO_DONGLE_STREAM_RUNNING)) {
                if ((c_handle->sink == NULL) || (c_handle->sink->transform == NULL) || (c_handle->source == NULL)) {
                    break;
                }
                is_dl_ready = true;
            }
            c_handle = c_handle->next_handle;
        }
        g_sco_fwd_info.is_dl_stream_ready = is_dl_ready;
    }
    if (!g_sco_fwd_info.is_dl_stream_ready) {
        BT_DONGLE_LOG_W("[BT Audio][DL][TX FWD] Dl stream is not ready", 0);
        return;
    }
    pattern_frame_size = Forwarder_Get_TX_FWD_Pattern_Size(gDspAlgParameter.EscoMode.Tx);
    if (pattern_frame_size == 0) {
        BT_DONGLE_LOG_W("[BT Audio][DL][TX FWD] pattern_frame_size == 0", 0);
        return;
    }
    // update global information
    if (g_sco_fwd_info.pkt_type == SCO_PKT_NULL) {
        g_sco_fwd_info.pkt_num_per_frame = sco_get_tx_fwd_pkt_type(&g_sco_fwd_info.pkt_type);
        g_sco_fwd_info.pattern_frame_size = pattern_frame_size;
    }
    if (pattern_frame_size != g_sco_fwd_info.pattern_frame_size) {
        BT_DONGLE_LOG_W("[BT Audio][DL][TX FWD] tx pattern size error %d != %d", 2, pattern_frame_size, g_sco_fwd_info.pattern_frame_size);
    }
    /* Check Tx fwd irq interval */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &tx_forwarder_gpt_time);
    if (g_sco_fwd_info.tx_dl_fwd_irq_process_cnt != 0) {
        hal_gpt_get_duration_count(g_sco_fwd_info.tx_dl_fwd_irq_gpt_cnt, tx_forwarder_gpt_time, &interval_gpt_time);
        spec_interval = (g_sco_fwd_info.pattern_frame_size / 10) * 1250; /* 10 -> 1.25ms, 20 -> 2.5ms, 30 -> 3.75ms, 60 -> 7.5ms */
        if (gDspAlgParameter.EscoMode.Tx == VOICE_NB) {
            spec_interval /= 2;
        }
        if (interval_gpt_time < 0x10) {
            BT_DONGLE_LOG_W("[BT Audio][DL][TX FWD] interval too close: %d us", 1, interval_gpt_time);
            g_sco_fwd_info.tx_dl_fwd_irq_gpt_cnt = tx_forwarder_gpt_time;
            return;
        } else if ((interval_gpt_time > (spec_interval + 500)) || (interval_gpt_time < (spec_interval - 500))) {
            BT_DONGLE_LOG_W("[BT Audio][DL][TX FWD] interval is not stable, expect %d, actual us: %d", 2, spec_interval, interval_gpt_time);
        }
    }
    g_sco_fwd_info.tx_dl_fwd_irq_gpt_cnt = tx_forwarder_gpt_time;

    /* Check Tx fwd buffer index */
    curr_tx_fwd_buf_idx = Forwarder_Tx_Status();
    if (((curr_tx_fwd_buf_idx != 0) && (curr_tx_fwd_buf_idx != 1)) || (g_sco_fwd_info.tx_dl_fwd_buf_index_pre == curr_tx_fwd_buf_idx)) {
        BT_DONGLE_LOG_W("[BT Audio][DL][TX FWD] get forwarder buffer index error, prev status:%d, current status:%d", 2,
            g_sco_fwd_info.tx_dl_fwd_buf_index_pre,
            curr_tx_fwd_buf_idx
            );
        g_sco_fwd_info.tx_dl_fwd_buf_index_pre = curr_tx_fwd_buf_idx;
        return;
    }
    g_sco_fwd_info.tx_dl_fwd_buf_index_pre = curr_tx_fwd_buf_idx;

    /* Copy the packet from the AVM buffer to Tx forwarder buffer */
    n9_dsp_share_info_ptr  p_share_info      = (n9_dsp_share_info_ptr)(g_sco_fwd_info.tx_dl_info);
    avm_share_buf_info_ptr p_avm_info        = (avm_share_buf_info_ptr)(g_sco_fwd_info.tx_dl_info);
    uint8_t                *for_warder_buf   = (uint8_t *)(hal_memview_cm4_to_dsp0(p_avm_info->ForwarderAddr) + curr_tx_fwd_buf_idx * AUDIO_DSP_FORWARD_BUFFER_SIZE);
    uint8_t                *sink_buf         = (uint8_t *)(hal_memview_cm4_to_dsp0(p_share_info->start_addr) + p_share_info->read_offset);
    // check dl sink buffer data
    uint32_t               avail_size        = audio_dongle_get_n9_share_buffer_data_size(p_share_info);
    if (avail_size >= pattern_frame_size) {
        if (avail_size > (pattern_frame_size * 2)) {
            BT_DONGLE_LOG_W("[BT Audio][DL][TX FWD] sink buffer data is so much %d > %d", 2, avail_size, 2 * pattern_frame_size);
        }
        memcpy(for_warder_buf, sink_buf, pattern_frame_size);
        #if AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE
            if (!first_dl_copy_data) {
                hal_gpio_set_output(GPIO_PIN_FIRST_SEND, 1);
            }
        #endif
        #ifdef AIR_AUDIO_DUMP_ENABLE
            LOG_AUDIO_DUMP(sink_buf, pattern_frame_size, AUDIO_BT_SRC_DONGLE_DL_SINK_OUT_ESCO);
        #endif
        /* Update the RPTR of AVM buffer */
        uint32_t sink_buf_len     = p_share_info->sub_info.block_info.block_size * p_share_info->sub_info.block_info.block_num;
        uint32_t sink_buf_next_ro = (p_share_info->read_offset + (uint32_t)pattern_frame_size) % sink_buf_len;
        // StreamDSP_HWSemaphoreTake();
        p_share_info->read_offset = sink_buf_next_ro;
        #if AIR_BT_AUDIO_DONGLE_LATENCY_DEBUG_ENABLE
            if (!first_dl_copy_data) {
                hal_gpio_set_output(GPIO_PIN_FIRST_SEND, 0);
                first_dl_copy_data = false;
            }
        #endif
        // StreamDSP_HWSemaphoreGive();
    } else {
        BT_DONGLE_LOG_W("[BT Audio][DL][TX FWD] sink buffer data is not enough %d < %d", 2, avail_size, pattern_frame_size);
        /* reset buffer */
        memset(for_warder_buf, 0, pattern_frame_size);
    }
    // trigger stream dl flow
    // if (g_sco_fwd_info.tx_dl_fwd_irq_process_cnt % 2) { // 0 to do
        bt_audio_dongle_dl_ccni_handler(0, NULL);
    // }
    g_sco_fwd_info.tx_dl_fwd_irq_process_cnt ++;
}

/****************************************************************************************************************************************************/
/*                                                       BT Dongle AFE IN(I2S IN/LINE IN)                                                          */
/****************************************************************************************************************************************************/
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
static void bt_audio_dongle_dl_afe_in_init(
    bt_audio_dongle_handle_t *dongle_handle,
    audio_transmitter_open_param_p audio_transmitter_open_param,
    bt_common_open_param_p bt_common_open_param)
{
    SOURCE                         source         = dongle_handle->source;
    bt_audio_dongle_dl_sink_info_t *sink_info     = &(dongle_handle->stream_info.dl_info.sink);
    stream_resolution_t            resolution     = RESOLUTION_16BIT;
    uint32_t                       format_bytes   = 0;
    uint32_t                       ratio          = 0;
    uint32_t                       stream_in_rate = 0;
    /* sink info init */
    memcpy(sink_info, &(bt_common_open_param->scenario_param.bt_audio_dongle_param.dl_info.sink), sizeof(bt_audio_dongle_dl_sink_info_t));
    format_bytes = source->param.audio.format_bytes;
    resolution   = (format_bytes > 2) ? RESOLUTION_32BIT : RESOLUTION_16BIT;
    stream_in_rate = source->param.audio.src_rate; // ul input rate for stream
    /* init src in/out frame size */
    ratio = (sink_info->bt_out.sample_rate > stream_in_rate) ?
                        (sink_info->bt_out.sample_rate / stream_in_rate) :
                        (stream_in_rate / sink_info->bt_out.sample_rate);
    AUDIO_ASSERT(ratio && "[BT Audio][DL] fs is not right check usb in sample rate and bt out sample rate");
    dongle_handle->src_out_frame_samples = sink_info->bt_out.frame_interval * sink_info->bt_out.sample_rate / 1000 / 1000;
    dongle_handle->src_out_frame_size    = dongle_handle->src_out_frame_samples * format_bytes;
    dongle_handle->src_in_frame_samples  = sink_info->bt_out.frame_interval * dongle_handle->source->param.audio.src_rate / 1000 / 1000;
    dongle_handle->src_in_frame_size     = dongle_handle->src_in_frame_samples * format_bytes;

    /* feature list init */
    // /* sw buffer init */
    // sw_buffer_config_t buffer_config = {0};
    // buffer_config.mode                  = SW_BUFFER_MODE_DROP_DATA_WHEN_BUFFER_FULL;
    // buffer_config.total_channels        = dongle_handle->source->param.audio.channel_num;
    // buffer_config.watermark_max_size    = 8 * dongle_handle->src_in_frame_size;
    // buffer_config.watermark_min_size    = 0;
    // buffer_config.output_size           = dongle_handle->src_in_frame_size;
    // dongle_handle->buffer_port_0        = stream_function_sw_buffer_get_unused_port(dongle_handle->source);
    // dongle_handle->buffer_default_output_size = dongle_handle->src_in_frame_size; // per irq handle frame size(bytes)
    // dongle_handle->buffer0_output_size = dongle_handle->buffer_default_output_size;
    // BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw buffer 0x%x info, %d, %d, %d, %d, %d, %d, %d\r\n", 10,
    //     dongle_handle->source->scenario_type,
    //     dongle_handle,
    //     dongle_handle->buffer_port_0,
    //     buffer_config.mode,
    //     buffer_config.total_channels,
    //     buffer_config.watermark_max_size,
    //     buffer_config.watermark_min_size,
    //     buffer_config.output_size,
    //     dongle_handle->buffer_default_output_size,
    //     dongle_handle->buffer0_output_size
    //     );
    // stream_function_sw_buffer_init(dongle_handle->buffer_port_0, &buffer_config);

    // /* sw gian */
    // int32_t default_gain = 0; // -120 dB to mute avoid pop noise
    // sw_gain_config_t default_config;
    // default_config.resolution               = RESOLUTION_16BIT;
    // default_config.target_gain              = default_gain; //audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.gain_default_L;
    // default_config.up_step                  = 1;
    // default_config.up_samples_per_step      = 2;
    // default_config.down_step                = -1;
    // default_config.down_samples_per_step    = 2;
    // dongle_handle->gain_port                = stream_function_sw_gain_get_port(dongle_handle->source);
    // stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    // // default_gain = audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.gain_default_L;
    // stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    // // default_gain = audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.gain_default_R;
    // stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    // BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d][handle 0x%x] sw gain 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 11,
    //             dongle_handle->source->scenario_type,
    //             dongle_handle,
    //             dongle_handle->gain_port,
    //             default_config.resolution,
    //             default_config.target_gain,
    //             default_config.up_step,
    //             default_config.up_samples_per_step,
    //             default_config.down_step,
    //             default_config.down_samples_per_step,
    //             default_gain, // audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.gain_default_L
    //             default_gain  // audio_transmitter_open_param->scenario_param.bt_audio_dongle_param.gain_default_R
    //             );
#ifdef AIR_SOFTWARE_DRC_ENABLE
    /* sw drc init */
    sw_compander_config_t drc_config;
    memset(&drc_config, 0, sizeof(sw_compander_config_t));
    drc_config.common.gain_control_mode = SW_COMPANDER_GAIN_CONTROL_NORMAL_MODE;
    drc_config.common.mode              = SW_COMPANDER_AUDIO_MODE;
    drc_config.common.channel_num       = dongle_handle->source->param.audio.channel_num;
    drc_config.common.sample_rate       = dongle_handle->source->param.audio.src_rate;
    drc_config.common.frame_base        = 8;                                      /* latency: 3*framebase samples   */
    drc_config.common.recovery_gain     = 4;                                      /* drc request: 4-bit right shift */
    drc_config.common.recovery_flag     = true;                                   /* drc do the 4-bit right shift   */
    drc_config.common.default_nvkey_id  = NVKEY_DSP_PARA_A2DP_AU_CPD;
    for (uint32_t index = 0; index < drc_config.common.channel_num; index ++) {
        drc_config.common.vol_gain[index] = -160 * 100;                           /* -160dB */
    }
    drc_config.default_nvkey_mem = NULL;
    sw_compander_port_t *drc_port = stream_function_sw_compander_get_port(dongle_handle->source);
    stream_function_sw_compander_init(drc_port, &drc_config);
    BT_DONGLE_LOG_I("[BT Audio][DL][scenario type %d-%d][handle 0x%x] sw drc 0x%x info, %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x\r\n", 15,
                dongle_handle->source->scenario_type,
                dongle_handle->sink->scenario_type,
                dongle_handle,
                drc_port,
                drc_config.common.gain_control_mode,
                drc_config.common.mode,
                drc_config.common.channel_num,
                drc_config.common.sample_rate,
                drc_config.common.frame_base,
                drc_config.common.recovery_gain,
                drc_config.common.vol_gain[0],
                drc_config.common.default_nvkey_id
                );
#endif
    UNUSED(audio_transmitter_open_param);
    UNUSED(bt_common_open_param);
}

static void bt_audio_dongle_dl_afe_in_deinit(bt_audio_dongle_handle_t *dongle_handle)
{
    /* sw gain deinit */
    // stream_function_sw_gain_deinit(dongle_handle->gain_port);
    UNUSED(dongle_handle);
#ifdef AIR_SOFTWARE_DRC_ENABLE
    sw_compander_port_t *drc_port;
    drc_port = stream_function_sw_compander_get_port(dongle_handle->source);
    stream_function_sw_compander_deinit(drc_port);
#endif
    // /* sw buffer deinit */
    // stream_function_sw_buffer_deinit(dongle_handle->buffer_port_0);
}
#endif /* Dongle Afe in */

void bt_audio_dongle_resolution_config(DSP_STREAMING_PARA_PTR stream)
{
    SINK   sink = stream->sink;
    bt_audio_dongle_handle_t *dongle_handle = (bt_audio_dongle_handle_t *)sink->param.bt_common.scenario_param.dongle_handle;
    if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0) &&
        (sink->scenario_type <= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2)) {
            stream->callback.EntryPara.resolution.source_in_res = (stream->source->param.audio.format <= HAL_AUDIO_PCM_FORMAT_U16_BE)
                                                        ? RESOLUTION_16BIT
                                                        : RESOLUTION_32BIT;
            if (dongle_handle->stream_info.dl_info.sink.bt_out.sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE) {
                // stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
                stream->callback.EntryPara.resolution.feature_res   = RESOLUTION_32BIT;
                stream->callback.EntryPara.resolution.sink_out_res  = RESOLUTION_32BIT;
            } else {
                // stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_16BIT;
                stream->callback.EntryPara.resolution.feature_res   = RESOLUTION_16BIT;
                stream->callback.EntryPara.resolution.sink_out_res  = RESOLUTION_16BIT;
            }
    }
}

#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
void bt_audio_dongle_silence_detection_init(audio_scenario_type_t scenario)
{
    uint32_t irq_mask;
    volume_estimator_port_t *port = NULL;
    volume_estimator_config_t config;
    uint32_t data_size;
    void *data_buf;
    void *nvkey_buf;
    bt_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;

    UNUSED(scenario);

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    if (g_bt_audio_dongle_silence_detection_init_count++ > 0) {
        hal_nvic_restore_interrupt_mask(irq_mask);
        BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][init]:[scenario type %d] bypass", 1, scenario);
        return;
    }
    hal_nvic_restore_interrupt_mask(irq_mask);

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
    if ((scenario < AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) || (scenario > AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2))
    {
        AUDIO_ASSERT(0);
    }

    /* get volume estimator port */
    port = volume_estimator_get_port(&g_bt_audio_dongle_silence_detection_handle);
    if (port == NULL)
    {
        AUDIO_ASSERT(0);
    }

    /* init volume estimator port */
    config.resolution = RESOLUTION_32BIT;
    config.frame_size = g_bt_audio_dongle_silence_detection_sample_rate / 1000 * sizeof(int32_t) * 5 / 2;
    config.channel_num = 1;
    config.mode = VOLUME_ESTIMATOR_CHAT_INSTANT_MODE;
    config.sample_rate = g_bt_audio_dongle_silence_detection_sample_rate;
    config.nvkey_para = (void *)&(((silence_detection_nvkey_t *)nvkey_buf)->chat_vol_nvkey);
    volume_estimator_init(port, &config);

    /* malloc 5ms memory for queue mono pcm data */
    data_size = g_bt_audio_dongle_silence_detection_sample_rate / 1000 * sizeof(int32_t) * g_bt_audio_dongle_silence_detection_interval / 1000 * g_bt_audio_dongle_silence_detection_channels;
    data_buf = preloader_pisplit_malloc_memory( PRELOADER_D_HIGH_PERFORMANCE, data_size);
    if (data_buf == NULL)
    {
        AUDIO_ASSERT(0);
    }
    memset(data_buf, 0, data_size);

    /* update state machine */
    silence_detection_handle = &g_bt_audio_dongle_silence_detection_handle;
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
    memset(silence_detection_handle->effective_delay_us_count, 0, sizeof(silence_detection_handle->effective_delay_us_count));
    silence_detection_handle->failure_threshold_db = ((silence_detection_nvkey_t *)nvkey_buf)->failure_threshold_db;
    silence_detection_handle->failure_delay_ms = ((silence_detection_nvkey_t *)nvkey_buf)->failure_delay_ms;
    memset(silence_detection_handle->failure_delay_us_count, 0, sizeof(silence_detection_handle->failure_delay_us_count));
    silence_detection_handle->frame_size = config.frame_size;
    silence_detection_handle->data_size = 0;
    silence_detection_handle->data_buf_size = data_size;
    silence_detection_handle->data_buf = data_buf;
    silence_detection_handle->status = BT_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW;
    silence_detection_handle->total_channel = 0;
    silence_detection_handle->data_size_channel = 0;

    BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][init]:%d, %d, %d, %d, %d, %d, %d, %d\r\n", 8,
                silence_detection_handle->nvkey_enable,
                silence_detection_handle->current_db,
                silence_detection_handle->effective_threshold_db,
                silence_detection_handle->effective_delay_ms,
                silence_detection_handle->failure_threshold_db,
                silence_detection_handle->failure_delay_ms,
                silence_detection_handle->data_buf_size,
                silence_detection_handle->frame_size);
}

void bt_audio_dongle_silence_detection_deinit(audio_scenario_type_t scenario)
{
    uint32_t irq_mask;
    volume_estimator_port_t *port;
    bt_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;
    UNUSED(scenario);

    /* get handle */
    if ((scenario < AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) || (scenario > AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2))
    {
        AUDIO_ASSERT(0);
    }

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    if (g_bt_audio_dongle_silence_detection_init_count-- > 1) {
        hal_nvic_restore_interrupt_mask(irq_mask);
        BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][deinit]:[scenario type %d] bypass", 1, scenario);
        return;
    }
    hal_nvic_restore_interrupt_mask(irq_mask);

    /* check state machine */
    silence_detection_handle = &g_bt_audio_dongle_silence_detection_handle;
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
    memset(silence_detection_handle, 0, sizeof(bt_audio_dongle_silence_detection_handle_t));

    g_bt_audio_dongle_silence_detection_init_count = false;

    BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][deinit]:%d, %d, %d, %d, %d, %d\r\n", 6,
                silence_detection_handle->nvkey_enable,
                silence_detection_handle->current_db,
                silence_detection_handle->effective_threshold_db,
                silence_detection_handle->effective_delay_ms,
                silence_detection_handle->failure_threshold_db,
                silence_detection_handle->failure_delay_ms);
}

void bt_audio_dongle_silence_detection_enable(audio_scenario_type_t scenario)
{
    bt_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;

    /* get handle */
    if ((scenario < AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) || (scenario > AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2))
    {
        AUDIO_ASSERT(0);
    }

    silence_detection_handle = &g_bt_audio_dongle_silence_detection_handle;
    silence_detection_handle->enable = 1;
    silence_detection_handle->status = BT_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW;
    memset(silence_detection_handle->failure_delay_us_count, 0, sizeof(silence_detection_handle->failure_delay_us_count));
    memset(silence_detection_handle->effective_delay_us_count, 0, sizeof(silence_detection_handle->effective_delay_us_count));
    BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][enable]:%d\r\n", 1, scenario);
}

void bt_audio_dongle_silence_detection_disable(audio_scenario_type_t scenario)
{
    bt_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;

    /* get handle */
    if ((scenario < AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) || (scenario > AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2))
    {
        AUDIO_ASSERT(0);
    }

    silence_detection_handle = &g_bt_audio_dongle_silence_detection_handle;
    silence_detection_handle->enable = 0;
    silence_detection_handle->status = BT_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_UNKNOW;
    memset(silence_detection_handle->failure_delay_us_count, 0, sizeof(silence_detection_handle->failure_delay_us_count));
    memset(silence_detection_handle->effective_delay_us_count, 0, sizeof(silence_detection_handle->effective_delay_us_count));
    BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][disable]:%d\r\n", 1, scenario);
}

void bt_audio_dongle_silence_detection_process(audio_scenario_type_t scenario)
{
    bt_audio_dongle_silence_detection_handle_t *silence_detection_handle = NULL;
    volume_estimator_port_t *port;
    uint32_t i, frame_num;
    aud_msg_status_t status;
    hal_ccni_message_t msg;
    bt_audio_dongle_silence_detection_status_t silence_detection_status_backup;
    bool is_silence_status, is_not_silence_status;

    /* get handle */
    if ((scenario < AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) || (scenario > AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2))
    {
        AUDIO_ASSERT(0);
    }

    silence_detection_handle = &g_bt_audio_dongle_silence_detection_handle;
    if (silence_detection_handle->enable)
    {
        port = silence_detection_handle->port;
        #ifdef AIR_AUDIO_DUMP_ENABLE
        LOG_AUDIO_DUMP(silence_detection_handle->data_buf, silence_detection_handle->data_size, AUDIO_WOOFER_CPD_OUT);
        #endif
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
                    silence_detection_handle->effective_delay_us_count[i] += 2500;
                    if (silence_detection_handle->effective_delay_us_count[i] >= silence_detection_handle->effective_delay_ms*1000)
                    {
                        if (silence_detection_handle->status != BT_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_SILENCE)
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
                    silence_detection_handle->failure_delay_us_count[i] += 2500;
                    if (silence_detection_handle->failure_delay_us_count[i] >= silence_detection_handle->failure_delay_ms*1000)
                    {
                        if (silence_detection_handle->status != BT_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_NOT_SILENCE)
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
            silence_detection_handle->status = BT_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_NOT_SILENCE;
            /* send msg to mcu side to alert that this is not a silence period */
            msg.ccni_message[0] = ((MSG_DSP2MCU_BT_AUDIO_DL_SILENCE_DETECTION_FEEDBACK << 16) | 0);
            msg.ccni_message[1] = scenario;
            status = aud_msg_tx_handler(msg, 0, FALSE);
            if (status != AUDIO_MSG_STATUS_OK)
            {
                /* rollback state machine that the next silence period will send msg again */
                silence_detection_handle->status = silence_detection_status_backup;
            }
            BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][send msg]: scenario %d, %d, %d, %d, %d, %d, %d\r\n", 7,
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
                silence_detection_handle->status = BT_AUDIO_DONGLE_SILENCE_DETECTION_STATUS_SILENCE;
                /* send msg to mcu side to alert that this is a silence period */
                msg.ccni_message[0] = ((MSG_DSP2MCU_BT_AUDIO_DL_SILENCE_DETECTION_FEEDBACK << 16) | 1);
                msg.ccni_message[1] = scenario;
                status = aud_msg_tx_handler(msg, 0, FALSE);
                if (status != AUDIO_MSG_STATUS_OK)
                {
                    /* rollback state machine that the next silence period will send msg again */
                    silence_detection_handle->status = silence_detection_status_backup;
                }
                BT_DONGLE_LOG_I("[BT Audio][DL][silence_detection][send msg]: scenario %d, %d, %d, %d, %d, %d, %d\r\n", 7,
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

#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */
