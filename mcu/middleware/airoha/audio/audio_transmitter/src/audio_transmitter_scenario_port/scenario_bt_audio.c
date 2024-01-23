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
#include "audio_transmitter_playback_port.h"
#include "usbaudio_drv.h"
#include "bt_sink_srv_ami.h"
#include "hal_audio_message_struct_common.h"
#include "hal_dvfs_internal.h"
#include "scenario_dongle_common.h"
#include "nvkey.h"
#include "audio_nvdm_coef.h"

/* Private define ------------------------------------------------------------*/
#define AUDIO_BT_MSBC_SHARE_BUF_FRAME_SIZE             (60U)
#define AUDIO_BT_CVSD_SHARE_BUF_FRAME_SIZE             (120U)
#define AUDIO_BT_HFP_SHARE_BUF_PARTTERN_TOTAL_SIZE     (480U)

#if defined(AIR_BTA_IC_PREMIUM_G2)
    #if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        #define AUDIO_BT_DONGLE_DVFS_LEVEL                     HAL_DVFS_FULL_SPEED_104M
    #else
        #define AUDIO_BT_DONGLE_DVFS_LEVEL                     HAL_DVFS_HIGH_SPEED_208M
    #endif
#else
    #if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        #define AUDIO_BT_DONGLE_DVFS_LEVEL                     HAL_DVFS_OPP_MID
    #else
        #define AUDIO_BT_DONGLE_DVFS_LEVEL                     HAL_DVFS_OPP_HIGH
    #endif
#endif
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern audio_dongle_usb_handle_t audio_dongle_usb_rx_handle[AUDIO_DONGLE_USB_RX_PORT_TOTAL];
extern audio_dongle_usb_handle_t audio_dongle_usb_tx_handle[AUDIO_DONGLE_USB_TX_PORT_TOTAL];
#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
static const uint8_t NVKEY_E711[] = {
0x01, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xFF, 0xFF, 0x58, 0x1B, 0x00, 0x00, 0xB4, 0xE2, 0xFF, 0xFF,
0x0A, 0x00, 0x00, 0x00, 0x18, 0xFC, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
0x91, 0x87, 0x08, 0x00, 0xCB, 0xBF, 0x04, 0x00, 0x90, 0x87, 0x08, 0x00, 0xE8, 0x9F, 0xB8, 0xFF,
0x25, 0x29, 0x32, 0x00, 0x2E, 0xC5, 0x42, 0x00, 0x5F, 0xC7, 0xF3, 0xFF, 0x2D, 0xC5, 0x42, 0x00,
0x29, 0x00, 0xB8, 0xFF, 0x29, 0x36, 0x64, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x06, 0x00, 0xA0, 0x0A, 0x0C,
0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
};
#endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */

#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
typedef enum __LHDCV5_MTU_SIZE__
{
  LHDCV5_MTU_MIN       = 300,
  LHDCV5_MTU_2MBPS     = 660,
  LHDCV5_MTU_3MBPS     = 1023,
  LHDCV5_MTU_MHDT_4DH5 = 1392,
  LHDCV5_MTU_MHDT_6DH5 = 2089,
  LHDCV5_MTU_MHDT_8DH5 = 2820,
  LHDCV5_MTU_MAX       = 4096,
} LHDCV5_MTU_SIZE_T;
#endif
typedef struct {
    bt_audio_usb_detect_config_t config;
    uint32_t                     timer_handler;
    uint32_t                     buffer_wo;
    uint32_t                     buffer_ro;
    uint32_t                     detect_count;
    bool                         is_trigger_flag;
} bt_audio_usb_detect_param_t;
/* Public variables ----------------------------------------------------------*/
extern const int16_t g_gain_compensation_table[11];
/* Private functions ---------------------------------------------------------*/
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
static void bt_auio_dongle_check_lhdc_mtu_size(uint32_t mtu_size)
{
    switch (mtu_size) {
        case LHDCV5_MTU_MIN:
        case LHDCV5_MTU_2MBPS:
        case LHDCV5_MTU_3MBPS:
        case LHDCV5_MTU_MHDT_4DH5:
        case LHDCV5_MTU_MHDT_6DH5:
        case LHDCV5_MTU_MHDT_8DH5:
        case LHDCV5_MTU_MAX:
            /* pass through */
            break;
        default:
            AUDIO_ASSERT(0 && "[BT Audio][DL] lhdc mtu size is invalid");
            break;
    }
}
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */

#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
static bt_audio_usb_detect_param_t  usb_detect_dl[AUDIO_DONGLE_USB_RX_PORT_TOTAL] = {0};
static bt_audio_usb_detect_param_t  usb_detect_ul[AUDIO_DONGLE_USB_TX_PORT_TOTAL] = {0};
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static hal_gpt_callback_t bt_audio_usb_irq_detect_gpt_cb(void *user_data)
{
    audio_transmitter_scenario_sub_id_btaudiodongle_t sub_id = *(audio_transmitter_scenario_sub_id_btaudiodongle_t *)user_data;
    bt_audio_usb_detect_param_t *usb_detect_ptr = NULL;
    n9_dsp_share_info_t *p_dsp_info = NULL;
    bool is_ul = false;
    /* parameter check */
    switch (sub_id) {
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
            p_dsp_info = audio_dongle_usb_tx_handle[0].p_dsp_info;
            usb_detect_ptr = &usb_detect_ul[0];
            is_ul = true;
            break;
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            p_dsp_info = audio_dongle_usb_tx_handle[1].p_dsp_info;
            usb_detect_ptr = &usb_detect_ul[1];
            is_ul = true;
            break;
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
            p_dsp_info = audio_dongle_usb_rx_handle[0].p_dsp_info;
            usb_detect_ptr = &usb_detect_dl[0];
            break;
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
            p_dsp_info = audio_dongle_usb_rx_handle[1].p_dsp_info;
            usb_detect_ptr = &usb_detect_dl[1];
            break;
        default:
            AUDIO_ASSERT(0 && "[BT Audio][USB] detect sub id error, plz check the parameter");
            break;
    }
    if (p_dsp_info) {
        if (is_ul) {
            if (usb_detect_ptr->buffer_ro == p_dsp_info->read_offset) {
                usb_detect_ptr->detect_count ++;
            } else {
                usb_detect_ptr->detect_count = 0;
            }
        } else {
            if (usb_detect_ptr->buffer_wo == p_dsp_info->write_offset) {
                usb_detect_ptr->detect_count ++;
            } else {
                usb_detect_ptr->detect_count = 0;
            }
        }
        usb_detect_ptr->buffer_wo = p_dsp_info->write_offset;
        usb_detect_ptr->buffer_ro = p_dsp_info->read_offset;
        uint64_t detect_time = usb_detect_ptr->detect_count * usb_detect_ptr->config.timer_period_ms;
        if ((detect_time >= usb_detect_ptr->config.notify_threshold_ms) && (!usb_detect_ptr->is_trigger_flag)) {
            /* notify host usb no data */
            if (usb_detect_ptr->config.cb) {
                TRANSMITTER_LOG_I("[BT Audio][USB] id[%d] usb no data %u", 2, sub_id, detect_time);
                usb_detect_ptr->config.cb(AUDIO_BT_DONGLE_USB_DATA_SUSPEND);
            }
            usb_detect_ptr->is_trigger_flag = true;
        } else if ((detect_time < usb_detect_ptr->config.notify_threshold_ms) && (usb_detect_ptr->is_trigger_flag)) {
            /* notify host usb data resume */
            if (usb_detect_ptr->config.cb) {
                TRANSMITTER_LOG_I("[BT Audio][USB] id[%d] usb data resume %u", 2, sub_id, detect_time);
                usb_detect_ptr->config.cb(AUDIO_BT_DONGLE_USB_DATA_RESUME);
            }
            usb_detect_ptr->is_trigger_flag = false;
        }
        TRANSMITTER_LOG_I("[BT Audio][USB] id[%d] count %d threshold %d wo %d ro %d pre wo %d flag %d", 7,
            sub_id,
            usb_detect_ptr->detect_count,
            usb_detect_ptr->config.notify_threshold_ms,
            p_dsp_info->write_offset,
            p_dsp_info->read_offset,
            usb_detect_ptr->buffer_wo,
            usb_detect_ptr->is_trigger_flag
            );
    } else {
        TRANSMITTER_LOG_E("[BT Audio][USB] id[%d] abnormal status streaming is ended!", 1, sub_id);
    }
    hal_gpt_status_t st = hal_gpt_sw_start_timer_ms(usb_detect_ptr->timer_handler, usb_detect_ptr->config.timer_period_ms, (hal_gpt_callback_t)bt_audio_usb_irq_detect_gpt_cb, user_data);
    if (st != HAL_GPT_STATUS_OK) {
        TRANSMITTER_LOG_E("[BT Audio] Error: id[%d] start gpt timer fail %d", 2, sub_id, st);
    }
    return 0;
}

static void bt_audio_dongle_open_stream_in_usb(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    uint32_t payload_size = 0;
    uint8_t usb_rx_port = 0;
    audio_dongle_usb_info_t *source_param = &(config->scenario_config.bt_audio_dongle_config.dl_info.source.usb_in);
    audio_dongle_usb_info_t *usb_in  = &(open_param->stream_in_param.data_dl.scenario_param.bt_audio_dongle_param.dl_info.source.usb_in);
    /* configure stream source */
    /* get usb frame size */
    payload_size = audio_dongle_get_usb_audio_frame_size(&(source_param->codec_type), &(source_param->codec_param));
    if (payload_size == 0) {
        TRANSMITTER_LOG_E("[BT Audio][DL][ERROR]Error usb frame size %d\r\n", 1, payload_size);
        AUDIO_ASSERT(0);
    }
    if ((config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0) ||
        (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0)) {
        usb_rx_port = 0;
    } else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1) ||
               (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1)) {
        usb_rx_port = 1;
    } else {
        AUDIO_ASSERT(0 && "[BT Audio][DL] Error sub id");
    }
    /* prepare usb-in paramters to USB Audio callback */
    audio_dongle_usb_rx_handle[usb_rx_port].frame_size                   = payload_size;
    audio_dongle_usb_rx_handle[usb_rx_port].frame_interval               = source_param->codec_param.pcm.frame_interval;
    audio_dongle_usb_rx_handle[usb_rx_port].usb_type                     = source_param->codec_type;
    audio_dongle_usb_rx_handle[usb_rx_port].usb_param.pcm.sample_rate    = source_param->codec_param.pcm.sample_rate;
    audio_dongle_usb_rx_handle[usb_rx_port].usb_param.pcm.channel_mode   = source_param->codec_param.pcm.channel_mode;
    audio_dongle_usb_rx_handle[usb_rx_port].usb_param.pcm.format         = source_param->codec_param.pcm.format;
    audio_dongle_usb_rx_handle[usb_rx_port].stream_is_started            = 1;
    audio_dongle_usb_rx_handle[usb_rx_port].dongle_stream_status         |= (1 << config->scenario_sub_id);
    audio_dongle_usb_rx_handle[usb_rx_port].p_dsp_info                   = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_DSP_RECEIVE_FROM_MCU_0 + usb_rx_port);
    open_param->stream_in_param.data_dl.p_share_info                     = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_DSP_RECEIVE_FROM_MCU_0 + usb_rx_port);
    /* prepare usb-in parameters to dsp */
    open_param->param.stream_in                                          = STREAM_IN_AUDIO_TRANSMITTER;
    open_param->stream_in_param.data_dl.scenario_type                    = config->scenario_type;
    open_param->stream_in_param.data_dl.scenario_sub_id                  = config->scenario_sub_id;
    open_param->stream_in_param.data_dl.data_notification_frequency      = 0;
    open_param->stream_in_param.data_dl.max_payload_size                 = payload_size;
    open_param->stream_in_param.data_dl.p_share_info->read_offset        = 0;
    open_param->stream_in_param.data_dl.p_share_info->write_offset       = 0;
    open_param->stream_in_param.data_dl.p_share_info->bBufferIsFull      = false;

    // init source info from config parameter
    memcpy(&(open_param->stream_in_param.data_dl.scenario_param.bt_audio_dongle_param.dl_info.source),
           &(config->scenario_config.bt_audio_dongle_config.dl_info.source), sizeof(bt_audio_dongle_dl_source_info_t));

    audio_transmitter_modify_share_info_by_block(open_param->stream_in_param.data_dl.p_share_info, payload_size);
    /* prepare default gain settings */
    // scenario_param->bt_audio_dongle_param.gain_default_L = -12000; // -120.00dB -> mute
    // scenario_param->bt_audio_dongle_param.gain_default_R = -12000; // -120.00dB -> mute

    /* stream common setting init */
    usb_in->channel_num    = source_param->codec_param.pcm.channel_mode;
    usb_in->sample_rate    = source_param->codec_param.pcm.sample_rate;
    usb_in->frame_interval = source_param->codec_param.pcm.frame_interval; // us
    usb_in->frame_samples  = usb_in->sample_rate * usb_in->frame_interval / 1000 / 1000;
    usb_in->frame_size     = usb_in->frame_samples * usb_in->channel_num * audio_dongle_get_usb_format_bytes(source_param->codec_param.pcm.format);
    usb_in->sample_format  = source_param->codec_param.pcm.format;

#if AIR_AUDIO_DONGLE_DEBUG_LANTENCY
    audio_usb_rx_scenario_latency_debug_init(usb_rx_port,
                                                payload_size,
                                                source_param->codec_param.pcm.channel_mode,
                                                source_param->codec_param.pcm.format);
#endif

    TRANSMITTER_LOG_I("[BT Audio][DL] USB setting: sub_id[%d], codec %u, fs %u, ch %u, format %u, irq %u, pay_load %u, sharebuffer 0x%x, size %d, 0x%x, frame samples %d, frame size %d, format %d\r\n", 13,
                        config->scenario_sub_id,
                        source_param->codec_type,
                        source_param->codec_param.pcm.sample_rate,
                        source_param->codec_param.pcm.channel_mode,
                        source_param->codec_param.pcm.format,
                        source_param->codec_param.pcm.frame_interval,
                        payload_size,
                        open_param->stream_in_param.data_dl.p_share_info,
                        open_param->stream_in_param.data_dl.p_share_info->length,
                        open_param->stream_in_param.data_dl.p_share_info->start_addr,
                        usb_in->frame_samples,
                        usb_in->frame_size,
                        usb_in->sample_format
                        );
}

static void bt_audio_dongle_open_stream_out_usb(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    uint32_t payload_size = 0;
    uint8_t usb_tx_port = 0;
    audio_dongle_usb_info_t *sink_param = &(config->scenario_config.bt_audio_dongle_config.ul_info.sink.usb_out);
    audio_dongle_usb_info_t *usb_out    = &(open_param->stream_out_param.data_ul.scenario_param.bt_audio_dongle_param.ul_info.sink.usb_out);
    /* configure stream source */
    /* get usb frame size */
    payload_size = audio_dongle_get_usb_audio_frame_size(&(sink_param->codec_type), &(sink_param->codec_param));
    if (payload_size == 0) {
        TRANSMITTER_LOG_E("[BT Audio][UL][ERROR]Error usb frame size %d\r\n", 1, payload_size);
        AUDIO_ASSERT(0);
    }
    if (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) {
        usb_tx_port = 0;
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1) {
        usb_tx_port = 1;
    } else {
        AUDIO_ASSERT(0 && "[BT Audio][UL] Error sub id");
    }
    /* prepare usb-in paramters to USB Audio callback */
    audio_dongle_usb_tx_handle[usb_tx_port].frame_size                   = payload_size;
    audio_dongle_usb_tx_handle[usb_tx_port].frame_interval               = sink_param->codec_param.pcm.frame_interval;
    audio_dongle_usb_tx_handle[usb_tx_port].usb_type                     = sink_param->codec_type;
    audio_dongle_usb_tx_handle[usb_tx_port].usb_param.pcm.sample_rate    = sink_param->codec_param.pcm.sample_rate;
    audio_dongle_usb_tx_handle[usb_tx_port].usb_param.pcm.channel_mode   = sink_param->codec_param.pcm.channel_mode;
    audio_dongle_usb_tx_handle[usb_tx_port].usb_param.pcm.format         = sink_param->codec_param.pcm.format;
    // TODO:dual usb card support ?
    audio_dongle_usb_tx_handle[usb_tx_port].p_dsp_info                   = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_DSP_SEND_TO_MCU_0);
    open_param->stream_out_param.data_ul.p_share_info                    = audio_dongle_usb_tx_handle[usb_tx_port].p_dsp_info;
    /* prepare usb-in parameters to dsp */
    open_param->param.stream_out                                     = STREAM_OUT_AUDIO_TRANSMITTER;
    open_param->stream_out_param.data_ul.scenario_type               = config->scenario_type;
    open_param->stream_out_param.data_ul.scenario_sub_id             = config->scenario_sub_id;
    open_param->stream_out_param.data_ul.data_notification_frequency = 0;
    open_param->stream_out_param.data_ul.max_payload_size            = payload_size;
    open_param->stream_out_param.data_ul.p_share_info->read_offset   = 0;
    open_param->stream_out_param.data_ul.p_share_info->write_offset  = 0;
    open_param->stream_out_param.data_ul.p_share_info->bBufferIsFull = false;

    // init source info from config parameter
    memcpy(&(open_param->stream_out_param.data_ul.scenario_param.bt_audio_dongle_param.ul_info.sink),
           &(config->scenario_config.bt_audio_dongle_config.ul_info.sink), sizeof(bt_audio_dongle_ul_sink_info_t));

    audio_transmitter_modify_share_info_by_block(open_param->stream_out_param.data_ul.p_share_info, payload_size);
    /* stream common setting init */
    usb_out->channel_num    = sink_param->codec_param.pcm.channel_mode;
    usb_out->sample_rate    = sink_param->codec_param.pcm.sample_rate;
    usb_out->frame_interval = sink_param->codec_param.pcm.frame_interval; // us
    usb_out->frame_samples  = usb_out->sample_rate * usb_out->frame_interval / 1000 / 1000;
    usb_out->frame_size     = usb_out->frame_samples * usb_out->channel_num * audio_dongle_get_usb_format_bytes(sink_param->codec_param.pcm.format);
    usb_out->sample_format  = sink_param->codec_param.pcm.format;

#if AIR_AUDIO_DONGLE_DEBUG_LANTENCY
    audio_usb_tx_scenario_latency_debug_init(0, payload_size, sink_param->codec_param.pcm.channel_mode, sink_param->codec_param.pcm.format, 5000);
#endif

    TRANSMITTER_LOG_I("[BT Audio][UL] USB setting: sub_id[%d], codec %u, fs %u, ch %u, format %u, irq %u, pay_load %u, sharebuffer 0x%x, size %d, 0x%x\r\n", 10,
                        config->scenario_sub_id,
                        sink_param->codec_type,
                        sink_param->codec_param.pcm.sample_rate,
                        sink_param->codec_param.pcm.channel_mode,
                        sink_param->codec_param.pcm.format,
                        sink_param->codec_param.pcm.frame_interval,
                        payload_size,
                        open_param->stream_out_param.data_ul.p_share_info,
                        open_param->stream_out_param.data_ul.p_share_info->length,
                        open_param->stream_out_param.data_ul.p_share_info->start_addr
                        );
}
#endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */

static void bt_audio_dongle_open_stream_out_bt(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    uint32_t i = 0;
    uint32_t payload_size = 0;
    uint8_t  format_bytes = 0; // audio_dongle_get_format_bytes(sink_info->bt_out.sample_format)
    bt_audio_dongle_dl_sink_info_t *sink_info = &(open_param->stream_out_param.bt_common.scenario_param.bt_audio_dongle_param.dl_info.sink);
    /* configure stream sink */
    /* prepare bt-out parameters to dsp */
    if ((config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.link_num == 0) ||
        (config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.link_num > BT_AUDIO_DATA_CHANNEL_NUMBER)) {
        TRANSMITTER_LOG_E("[BT Audio][DL][ERROR]Error dl link num %d\r\n", 1, config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.link_num);
        AUDIO_ASSERT(0);
    }
    open_param->param.stream_out                            = STREAM_OUT_BT_COMMON;
    open_param->stream_out_param.bt_common.scenario_type    = config->scenario_type;
    open_param->stream_out_param.bt_common.scenario_sub_id  = config->scenario_sub_id;
    open_param->stream_out_param.bt_common.share_info_type  = SHARE_BUFFER_INFO_TYPE;
    open_param->stream_out_param.bt_common.p_share_info     = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_SEND_TO_AIR_0); // Not used
    open_param->stream_out_param.bt_common.data_notification_frequency = 0;
    // init sink info from config parameter
    memcpy(sink_info, &(config->scenario_config.bt_audio_dongle_config.dl_info.sink), sizeof(bt_audio_dongle_dl_sink_info_t));
    if (config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.without_bt_link_mode_enable) {
        TRANSMITTER_LOG_I("[BT Audio][DL] enter without_bt_link_mode mode\r\n", 0);
    }
    if (!sink_info->bt_out.ts_not_reset_flag) {
        TRANSMITTER_LOG_I("[BT Audio][DL] reset ts\r\n", 0);
    }
    // BT Classic dongle only support 16bit.
    sink_info->bt_out.sample_format = HAL_AUDIO_PCM_FORMAT_S16_LE;
    format_bytes = audio_dongle_get_format_bytes(sink_info->bt_out.sample_format);
    /* change frame interval to align bt clk:0.3125ms : gTriggerDspEncodeIntervalInClkTick = ((gTriggerDspEncodeIntervalInClkTick*10000)/3125) + 1 */
    // sink_info->bt_out.frame_interval =sink_info->bt_out.frame_interval;
    open_param->stream_out_param.bt_common.max_payload_size = 0;
    // check parameters
    for (i = 0; i < config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.link_num; i++) {
        if (config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].enable) {
            payload_size = audio_dongle_get_codec_frame_size(&(config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_type),
                                                            &(config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param));
            if (payload_size == 0) {
                TRANSMITTER_LOG_E("[BT Audio][DL][ERROR]Error codec frame size %d\r\n", 1, payload_size);
                AUDIO_ASSERT(0);
            }
            if (open_param->stream_out_param.bt_common.max_payload_size < (payload_size)) {
                open_param->stream_out_param.bt_common.max_payload_size = payload_size;
            }
            sink_info->bt_out.codec_type = config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_type;
            if (config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) {
                //sink_info->bt_out.sample_rate    = sink_info->bt_out.bt_info[i].codec_param.msbc.sample_rate; //TODO
                sink_info->bt_out.channel_num    = 1; //sink_info->bt_out.bt_info[i].codec_param.msbc.channel_mode == 0 ? 1 : 2;
                // sink_info->bt_out.frame_samples  = sink_info->bt_out.frame_interval * sink_info->bt_out.sample_rate / 1000 / 1000; // 15ms 16bit
                sink_info->bt_out.frame_samples  = 7500 * sink_info->bt_out.sample_rate / 1000 / 1000; // 7.5ms 16bit
                sink_info->bt_out.frame_size     = sink_info->bt_out.frame_samples * format_bytes;
                hal_audio_set_hfp_avm_info(sink_info->bt_out.bt_info[i].share_info, AUDIO_BT_HFP_SHARE_BUF_PARTTERN_TOTAL_SIZE, AUDIO_BT_MSBC_SHARE_BUF_FRAME_SIZE);
                TRANSMITTER_LOG_I("[BT Audio][DL] BT link msbc setting ch%u: %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, 0x%x, 0x%x\r\n", 13,
                                    i + 1,
                                    config->scenario_sub_id,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_type,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.msbc.min_bit_pool,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.msbc.max_bit_pool,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.msbc.block_length,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.msbc.subband_num,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.msbc.alloc_method,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.msbc.sample_rate,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.msbc.channel_mode,
                                    payload_size,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].share_info,
                                    ((n9_dsp_share_info_t *)(config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].share_info))->start_addr);
            } else if (config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
                uint32_t sub_band = sink_info->bt_out.bt_info[i].codec_param.sbc_enc.subband_num == 0 ? 4 : 8;
                uint32_t blocks   = (sink_info->bt_out.bt_info[i].codec_param.sbc_enc.block_length + 1) * 4;
                sink_info->bt_out.frame_interval = (sink_info->bt_out.frame_interval / 1000 + 4) / 5 * 5000;
                //sink_info->bt_out.sample_rate    = sink_info->bt_out.bt_info[i].codec_param.sbc_enc.sample_rate; //TODO
                sink_info->bt_out.channel_num    = 1; //sink_info->bt_out.bt_info[i].codec_param.sbc_enc.channel_mode == 0 ? 1 : 2;
                sink_info->bt_out.frame_samples  = sub_band * blocks; // 15ms 16bit
                sink_info->bt_out.frame_size     = sink_info->bt_out.frame_samples * format_bytes;
                // config forwarder buffer and share buffer for eSCO
                TRANSMITTER_LOG_I("[BT Audio][DL] BT link sbc setting ch%u: %u, %u, %u, %u, %u, %u, %u, %u, 0x%x, %u, 0x%x, 0x%x\r\n", 13,
                                    i + 1,
                                    config->scenario_sub_id,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_type,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.sbc_enc.max_bit_pool,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.sbc_enc.min_bit_pool,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.sbc_enc.block_length,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.sbc_enc.subband_num,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.sbc_enc.alloc_method,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.sbc_enc.sample_rate,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.sbc_enc.channel_mode,
                                    payload_size,
                                    config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].share_info,
                                    ((n9_dsp_share_info_t *)(config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].share_info))->start_addr);
            } else if (config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_CVSD) {
                // sink_info->bt_out.frame_samples  = sink_info->bt_out.sample_rate * sink_info->bt_out.frame_interval / 1000 / 1000; // 15ms 16bit
                sink_info->bt_out.frame_samples  = 7500 * sink_info->bt_out.sample_rate / 1000 / 1000; // 7.5ms 16bit
                sink_info->bt_out.frame_size     = sink_info->bt_out.frame_samples * format_bytes;
                sink_info->bt_out.channel_num    = 1;
                hal_audio_set_hfp_avm_info(sink_info->bt_out.bt_info[i].share_info, AUDIO_BT_HFP_SHARE_BUF_PARTTERN_TOTAL_SIZE, AUDIO_BT_CVSD_SHARE_BUF_FRAME_SIZE);
            }
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
            else if (config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_LHDC) {
                    sink_info->bt_out.sample_format = HAL_AUDIO_PCM_FORMAT_S24_LE;
                    bt_auio_dongle_check_lhdc_mtu_size(config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.mtu_size);
                    sink_info->bt_out.frame_interval = 5000; // lhdc 5ms
                    sink_info->bt_out.sample_rate    = config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.sample_rate;
                    sink_info->bt_out.frame_samples  = sink_info->bt_out.frame_interval * sink_info->bt_out.sample_rate / 1000 / 1000; // 7.5ms 16bit
                    format_bytes = audio_dongle_get_format_bytes(config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.sample_format);
                    sink_info->bt_out.frame_size     = sink_info->bt_out.frame_samples  * format_bytes;
                    sink_info->bt_out.channel_num    = 1;
                    TRANSMITTER_LOG_I("[BT Audio][DL] BT link lhdc setting ch%u: %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, 0x%x, %u, 0x%x, 0x%x", 17,
                                        i + 1,
                                        config->scenario_sub_id,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_type,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.mtu_size,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.sample_rate,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.min_bit_rate,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.max_bit_rate,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.sample_format,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.frame_interval,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.version,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.loss_type,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.meta_type,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.raw_mode,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_param.lhdc.cbr_bit_rate,
                                        payload_size,
                                        config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].share_info,
                                        ((n9_dsp_share_info_t *)(config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].share_info))->start_addr);
            }
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */
            else {
                TRANSMITTER_LOG_I("[BT Audio][DL] codec type error %d", 1, config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].codec_type);
            }
            if (((((n9_dsp_share_info_t *)(sink_info->bt_out.bt_info[i].share_info))->sub_info.block_info.block_size) != ((payload_size + sizeof(BT_AUDIO_HEADER) + 3)/4*4)) &&
                ((((n9_dsp_share_info_t *)(sink_info->bt_out.bt_info[i].share_info))->sub_info.block_info.block_size) != payload_size)) {
                /* eSCO data is handled by Forwarder HW, so there is no header in it */
                TRANSMITTER_LOG_E("[BT Audio][DL][ERROR]Error packet size %d, %d\r\n", 2, payload_size + sizeof(BT_AUDIO_HEADER), (((n9_dsp_share_info_t *)(config->scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.bt_info[i].share_info))->sub_info.block_info.block_size));
                AUDIO_ASSERT(0);
            }
            TRANSMITTER_LOG_I("[BT Audio][DL] BT sink share buffer info 0x%x size %d addr 0x%x wo %d ro %d block size %d block number %d", 7,
                (n9_dsp_share_info_t *)(sink_info->bt_out.bt_info[i].share_info),
                ((n9_dsp_share_info_t *)(sink_info->bt_out.bt_info[i].share_info))->length,
                ((n9_dsp_share_info_t *)(sink_info->bt_out.bt_info[i].share_info))->start_addr,
                ((n9_dsp_share_info_t *)(sink_info->bt_out.bt_info[i].share_info))->write_offset,
                ((n9_dsp_share_info_t *)(sink_info->bt_out.bt_info[i].share_info))->read_offset,
                ((n9_dsp_share_info_t *)(sink_info->bt_out.bt_info[i].share_info))->sub_info.block_info.block_size,
                ((n9_dsp_share_info_t *)(sink_info->bt_out.bt_info[i].share_info))->sub_info.block_info.block_num
                );
        } else {
            open_param->stream_out_param.bt_common.scenario_param.bt_audio_dongle_param.dl_info.sink.bt_out.bt_info[i].share_info = NULL;
        }
    }

    TRANSMITTER_LOG_I("[BT Audio][DL] BT out: sub id [%d] codec %d, fs %d, ch %d, format %d, frame samples %d, frame size %d, frame interval %d bit_rate %d link_num %d", 10,
        open_param->stream_out_param.bt_common.scenario_sub_id,
        sink_info->bt_out.codec_type,
        sink_info->bt_out.sample_rate,
        sink_info->bt_out.channel_num,
        sink_info->bt_out.sample_format,
        sink_info->bt_out.frame_samples,
        sink_info->bt_out.frame_size,
        sink_info->bt_out.frame_interval,
        sink_info->bt_out.bit_rate,
        sink_info->bt_out.link_num
        );
}

static void bt_audio_dongle_open_stream_in_bt(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    uint32_t i              = 0;
    uint32_t payload_size   = 0;
    uint32_t process_frames = 0;
    uint8_t  format_bytes   = 0; // audio_dongle_get_format_bytes(bt_in->sample_format);
    /* configure stream sink */
    /* prepare bt-out parameters to dsp */
    bt_audio_dongle_bt_in_info_t *bt_in = &(open_param->stream_in_param.bt_common.scenario_param.bt_audio_dongle_param.ul_info.source.bt_in);
    if ((config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.link_num == 0) ||
        (config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.link_num > BT_AUDIO_DATA_CHANNEL_NUMBER)) {
        TRANSMITTER_LOG_E("[BT Audio][UL][ERROR]Error dl link num %d\r\n", 1, config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.link_num);
        AUDIO_ASSERT(0);
    }
    open_param->param.stream_in                            = STREAM_IN_BT_COMMON;
    open_param->stream_in_param.bt_common.scenario_type    = config->scenario_type;
    open_param->stream_in_param.bt_common.scenario_sub_id  = config->scenario_sub_id;
    open_param->stream_in_param.bt_common.share_info_type  = SHARE_BUFFER_INFO_TYPE;
    open_param->stream_in_param.bt_common.p_share_info     = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ULL_AUDIO_V2_DONGLE_BT_RECEIVE_FROM_AIR_0); // not used
    open_param->stream_in_param.bt_common.data_notification_frequency = 0;

    // init source info from config parameter
    memcpy(&(open_param->stream_in_param.bt_common.scenario_param.bt_audio_dongle_param.ul_info.source),
           &(config->scenario_config.bt_audio_dongle_config.ul_info.source), sizeof(bt_audio_dongle_ul_source_info_t));

    /* bt in common setting init */
    // BT Classic dongle only support 16bit.
    bt_in->sample_format = HAL_AUDIO_PCM_FORMAT_S16_LE;
    format_bytes   = audio_dongle_get_format_bytes(bt_in->sample_format);
    process_frames = bt_in->frame_interval / 7500;
    open_param->stream_in_param.bt_common.max_payload_size = 0;
    for (i = 0; i < config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.link_num; i++) {
        if (config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].enable) {
            bt_in->codec_type = config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_type;
            payload_size = process_frames * audio_dongle_get_codec_frame_size(  &(config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_type),
                                                            &(config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_param));
            if (payload_size == 0) {
                TRANSMITTER_LOG_E("[BT Audio][UL][ERROR]Error codec frame size %d\r\n", 1, payload_size);
                AUDIO_ASSERT(0);
            }
            if (open_param->stream_in_param.bt_common.max_payload_size < (payload_size)) {
                open_param->stream_in_param.bt_common.max_payload_size = payload_size;
            }
            if (config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) {
                bt_in->channel_num  = 1;
                bt_in->sample_rate  = 16000;
                hal_audio_set_hfp_avm_info(bt_in->bt_info[i].share_info, AUDIO_BT_HFP_SHARE_BUF_PARTTERN_TOTAL_SIZE, AUDIO_BT_MSBC_SHARE_BUF_FRAME_SIZE);
                TRANSMITTER_LOG_I("[BT Audio][UL] BT link msbc setting ch%u: %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, 0x%x, 0x%x\r\n", 13,
                                    i+1,
                                    config->scenario_sub_id,
                                    config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_type,
                                    config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_param.msbc.min_bit_pool,
                                    config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_param.msbc.max_bit_pool,
                                    config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_param.msbc.block_length,
                                    config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_param.msbc.subband_num,
                                    config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_param.msbc.alloc_method,
                                    config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_param.msbc.sample_rate,
                                    config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_param.msbc.channel_mode,
                                    payload_size,
                                    config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].share_info,
                                    ((n9_dsp_share_info_t *)(config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].share_info))->start_addr);
            } else if (config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_type == AUDIO_DSP_CODEC_TYPE_CVSD) {
                bt_in->channel_num   = 1;
                bt_in->sample_rate   = 8000;
                hal_audio_set_hfp_avm_info(bt_in->bt_info[i].share_info, AUDIO_BT_HFP_SHARE_BUF_PARTTERN_TOTAL_SIZE, AUDIO_BT_CVSD_SHARE_BUF_FRAME_SIZE);
            } else {
                TRANSMITTER_LOG_I("[BT Audio][DL] codec type error %d", 1, config->scenario_config.bt_audio_dongle_config.ul_info.source.bt_in.bt_info[i].codec_type);
                AUDIO_ASSERT(0);
            }
            if ((((n9_dsp_share_info_t *)(bt_in->bt_info[i].share_info))->sub_info.block_info.block_size) != ((payload_size + 3)/4*4)) {
                TRANSMITTER_LOG_E("[BT Audio][UL][ERROR]Error packet size %d, %d\r\n", 2, payload_size, (((n9_dsp_share_info_t *)(bt_in->bt_info[i].share_info))->sub_info.block_info.block_size));
                // AUDIO_ASSERT(0);
            }
            TRANSMITTER_LOG_I("[BT Audio][UL] BT source share buffer info 0x%x size %d addr 0x%x wo %d ro %d block size %d block number %d", 7,
                (n9_dsp_share_info_t *)(bt_in->bt_info[i].share_info),
                ((n9_dsp_share_info_t *)(bt_in->bt_info[i].share_info))->length,
                ((n9_dsp_share_info_t *)(bt_in->bt_info[i].share_info))->start_addr,
                ((n9_dsp_share_info_t *)(bt_in->bt_info[i].share_info))->write_offset,
                ((n9_dsp_share_info_t *)(bt_in->bt_info[i].share_info))->read_offset,
                ((n9_dsp_share_info_t *)(bt_in->bt_info[i].share_info))->sub_info.block_info.block_size,
                ((n9_dsp_share_info_t *)(bt_in->bt_info[i].share_info))->sub_info.block_info.block_num
                );
        } else {
            open_param->stream_in_param.bt_common.scenario_param.bt_audio_dongle_param.ul_info.source.bt_in.bt_info[i].share_info = NULL;
        }
    }
    bt_in->frame_samples = bt_in->frame_interval * bt_in->sample_rate / 1000 / 1000;
    bt_in->frame_size    = bt_in->frame_samples * format_bytes;
    /* prepare default gain settings */
    // open_param->stream_in_param.bt_common.scenario_param.bt_audio_dongle_param.gain_default_L = -12000; // -120.00dB
    // open_param->stream_in_param.bt_common.scenario_param.bt_audio_dongle_param.gain_default_R = -12000; // -120.00dB
    /* config nvdm: plc / rx nr / drc */
    DSP_FEATURE_TYPE_LIST AudioFeatureList_bt_source_hfp_ul[2] = {FUNC_END, FUNC_END};
#ifndef AIR_AIRDUMP_ENABLE
    AudioFeatureList_bt_source_hfp_ul[0] = FUNC_RX_NR;
#else
    AudioFeatureList_bt_source_hfp_ul[0] = FUNC_PLC;
#endif
    sysram_status_t status = audio_nvdm_set_feature(2, AudioFeatureList_bt_source_hfp_ul);
    if (status != NVDM_STATUS_NAT_OK) {
        TRANSMITTER_LOG_E("[BT Audio][UL] failed to set parameters to share memory - err(%d)\r\n", 1, status);
        AUDIO_ASSERT(0);
    }
    TRANSMITTER_LOG_I("[BT Audio][UL] BT in: sub id [%d] codec %d, fs %d, ch %d, format %d, frame samples %d, frame size %d, frame interval %d bit_rate %d link_num %d", 10,
        open_param->stream_in_param.bt_common.scenario_sub_id,
        bt_in->codec_type,
        bt_in->sample_rate,
        bt_in->channel_num,
        bt_in->sample_format,
        bt_in->frame_samples,
        bt_in->frame_size,
        bt_in->frame_interval,
        bt_in->bit_rate,
        bt_in->link_num
        );
}
/****************************************************************************************************************************************************/
/*                                                      AFE IN COMMON                                                                               */
/****************************************************************************************************************************************************/
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
static void bt_audio_dongle_open_stream_in_afe(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    audio_dongle_set_stream_in_afe(config, open_param);
    TRANSMITTER_LOG_I("[BT Audio][DL] afe setting: sub_id[%d], device %u, interface %u, format %u, hwsrc input rate %d, hwsrc output rate %d, frame size %d, frame number %d\r\n", 8,
        config->scenario_sub_id,
        open_param->stream_in_param.afe.audio_device,
        open_param->stream_in_param.afe.audio_interface,
        open_param->stream_in_param.afe.format,
        open_param->stream_in_param.afe.sampling_rate,
        open_param->stream_in_param.afe.stream_out_sampling_rate,
        open_param->stream_in_param.afe.frame_size,
        open_param->stream_in_param.afe.frame_number
        );
}
#endif /* afe in */
/* Public functions ----------------------------------------------------------*/

void bt_audio_dongle_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    audio_nvdm_reset_sysram();

#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
    sysram_status_t status;
    DSP_FEATURE_TYPE_LIST AudioFeatureList_SilenceDetection[] = {
        FUNC_SILENCE_DETECTION,
        FUNC_END,
    };

    switch (config->scenario_sub_id) {
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            /* prepare silence detection NVKEY */
            status = NVDM_STATUS_ERROR;
            while (status != NVDM_STATUS_NAT_OK) {
                /* set NVKEYs that the usb chat stream uses into the share buffer */
                status = audio_nvdm_set_feature(sizeof(AudioFeatureList_SilenceDetection)/sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_SilenceDetection);
                if (status != NVDM_STATUS_NAT_OK) {
                    TRANSMITTER_LOG_E("[BT Audio][DL][silence_detection] failed to set parameters to share memory - err(%d)\r\n", 1, status);
                    // AUDIO_ASSERT(0);
                    nvkey_write_data(NVID_DSP_ALG_SIL_DET2, (const uint8_t *)&NVKEY_E711[0], sizeof(NVKEY_E711));
                }
            }
            break;
        default:
            break;
    }
#endif /*AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE  */

    switch (config->scenario_sub_id) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        /* DL HFP Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            /* config dl source: usb in */
            bt_audio_dongle_open_stream_in_usb(config, open_param);
            /* config dl sink  : bt out */
            bt_audio_dongle_open_stream_out_bt(config, open_param);
            break;
        /* HFP UL Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            /* config ul source: bt in */
            bt_audio_dongle_open_stream_in_bt(config, open_param);
            /* config ul sink  : usb out */
            bt_audio_dongle_open_stream_out_usb(config, open_param);
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            /* config dl sink  : usb out */
            bt_audio_dongle_open_stream_out_bt(config, open_param);
            /* config dl source: bt in */
            bt_audio_dongle_open_stream_in_afe(config, open_param);
            open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM_SUB;
            break;
#endif
        default:
            break;
    }
}

void bt_audio_dongle_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param)
{
    switch (config->scenario_sub_id) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        /* DL HFP Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            start_param->param.stream_in  = STREAM_IN_AUDIO_TRANSMITTER;
            start_param->param.stream_out = STREAM_OUT_BT_COMMON;
            break;
        /* HFP UL Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            start_param->param.stream_in  = STREAM_IN_BT_COMMON;
            start_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            audio_dongle_set_start_avm_config(config, start_param);
            start_param->stream_in_param.afe.mce_flag = true; // sw trigger enable
            break;
#endif
        default:
            break;
    }
}

audio_transmitter_status_t bt_audio_dongle_set_runtime_config_playback(audio_transmitter_config_t *config, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config, mcu2dsp_audio_transmitter_runtime_config_param_t *runtime_config_param)
{
    audio_transmitter_status_t ret = AUDIO_TRANSMITTER_STATUS_FAIL;
    uint32_t operation = runtime_config_type;
    audio_transmitter_bt_audio_runtime_config_t *temp_config = &(runtime_config->bt_audio_runtime_config);
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
    bt_audio_usb_detect_param_t *detect_param = NULL;
#endif
    hal_gpt_status_t st = HAL_GPT_STATUS_OK;
    TRANSMITTER_LOG_I("[BT Audio][DL][config] runtime scenario_sub_id [%d] operation [%d].", 2 ,
        config->scenario_sub_id,
        operation
        );

    switch (operation) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case BT_AUDIO_DONGLE_CONFIG_OP_USB_DETECT_ENABLE:
            {
                switch (config->scenario_sub_id) {
                        /* DL HFP Path */
                        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
                        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
                        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
                        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
                        /* HFP UL Path */
                        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
                        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
                            if ((config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0) ||
                                (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0)) {
                                detect_param = &usb_detect_dl[0];
                            } else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1) ||
                                (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1)) {
                                detect_param = &usb_detect_dl[1];
                            } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) {
                                detect_param = &usb_detect_ul[0];
                            } else {
                                detect_param = &usb_detect_ul[1];
                            }
                            memset(detect_param, 0, sizeof(bt_audio_usb_detect_param_t));
                            detect_param->config.timer_period_ms     = temp_config->config.usb_detect.timer_period_ms;
                            detect_param->config.notify_threshold_ms = temp_config->config.usb_detect.notify_threshold_ms;
                            detect_param->config.cb                  = temp_config->config.usb_detect.cb;
                            if (detect_param->timer_handler == 0) {
                                st = hal_gpt_sw_get_timer(&detect_param->timer_handler);
                                if (st != HAL_GPT_STATUS_OK) {
                                    TRANSMITTER_LOG_E("[BT Audio][USB] Error: get gpt handle fail %d", 1, st);
                                }
                            }
                            st = hal_gpt_sw_start_timer_ms(detect_param->timer_handler, detect_param->config.timer_period_ms, (hal_gpt_callback_t)bt_audio_usb_irq_detect_gpt_cb, &(config->scenario_sub_id));
                            if (st != HAL_GPT_STATUS_OK) {
                                TRANSMITTER_LOG_E("[BT Audio][USB] Error: start gpt timer fail %d", 1, st);
                            }
                            TRANSMITTER_LOG_I("[BT Audio][USB] start usb detect sub id %d irq %d threshold %d cb 0x%x", 4,
                                config->scenario_sub_id,
                                detect_param->config.timer_period_ms,
                                detect_param->config.notify_threshold_ms,
                                detect_param->config.cb
                                );
                            break;
                    default:
                        break;
                }
            }
            break;
        case BT_AUDIO_DONGLE_CONFIG_OP_USB_DETECT_DISABLE:
            {
                switch (config->scenario_sub_id) {
                    /* DL HFP Path */
                    case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
                    case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
                    case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
                    case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
                    /* HFP UL Path */
                    case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
                    case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
                        if ((config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0) ||
                            (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0)) {
                            detect_param = &usb_detect_dl[0];
                        } else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1) ||
                            (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1)) {
                            detect_param = &usb_detect_dl[1];
                        } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) {
                            detect_param = &usb_detect_ul[0];
                        } else {
                            detect_param = &usb_detect_ul[1];
                        }
                        if (detect_param->timer_handler != 0) {
                            st = hal_gpt_sw_stop_timer_ms(detect_param->timer_handler);
                            if (st != HAL_GPT_STATUS_OK) {
                                TRANSMITTER_LOG_E("[BT Audio][USB] Error: stop gpt timer fail %d", 1, st);
                            }
                            st = hal_gpt_sw_free_timer(detect_param->timer_handler);
                            if (st != HAL_GPT_STATUS_OK) {
                                TRANSMITTER_LOG_E("[BT Audio][USB] Error: free gpt timer fail %d", 1, st);
                            }
                            detect_param->timer_handler = 0;
                        }
                        TRANSMITTER_LOG_I("[BT Audio][USB] stop usb detect sub id %d", 1, config->scenario_sub_id);
                        break;
                    default:
                        break;
                }
            }
            break;
#endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */
        case BT_AUDIO_DONGLE_CONFIG_OP_SET_MUTE:
        case BT_AUDIO_DONGLE_CONFIG_OP_SET_UNMUTE:
#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
        case BT_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_ENABLE:
        case BT_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_DISABLE:
#endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */
            {
                switch (config->scenario_sub_id) {
                    /* DL HFP Path */
                    case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0 ... AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
                    case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0 ... AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
#endif /* afe in */
                        runtime_config_param->config_operation = operation;
                        ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                        break;
                    default:
                        break;
                }
            }
            break;
        case BT_AUDIO_DONGLE_CONFIG_OP_SET_VOLUME_LEVEL:
            {
                switch (config->scenario_sub_id) {
                    /* DL HFP Path */
                    case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0 ... AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
                    case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0 ... AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
#endif /* afe in */
                    {
                        uint32_t vol_max_level = 0;
                        uint8_t  vol_ratio     = temp_config->config.vol_config.vol_ratio;
                        vol_type_t vol_type[4] =  {
                            VOL_USB_AUDIO_SW_IN, /* USB IN   */
                            VOL_LINE_IN,         /* LINE IN  */
                            VOL_A2DP,            /* I2S IN 0 */
                            VOL_VP               /* I2S IN 1 */
                            };
                        uint32_t index = 0;
                        vol_max_level = bt_sink_srv_ami_get_usb_music_sw_max_volume_level();
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
                        if ((config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0) ||
                            (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0)) {
                            index = 1;
                            vol_max_level = bt_sink_srv_ami_get_lineIN_max_volume_level();
                        } else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1) ||
                            (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1)) {
                            index = 2;
                            vol_max_level = bt_sink_srv_ami_get_a2dp_max_volume_level();
                        } else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1) ||
                            (config->scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1)) {
                            index = 3;
                            vol_max_level = bt_sink_srv_ami_get_vp_max_volume_level();
                        }
#endif
                        bt_audio_dongle_gain_info_t gain_info[BT_AUDIO_VOLUME_CH_MAX] = {0};
                        for (uint32_t i = 0; i < BT_AUDIO_VOLUME_CH_MAX; i ++) {
                            if (temp_config->config.vol_config.vol_level[i] == BT_AUDIO_VOLUME_LEVEL_INVALID) {
                                gain_info[i].ignore_flag = 0x1;
                            } else if (temp_config->config.vol_config.vol_level[i] == BT_AUDIO_VOLUME_LEVEL_MUTE) {
                                gain_info[i].mute_flag   = 0x1;
                            } else {
                                if (temp_config->config.vol_config.vol_level[i] > vol_max_level) {
                                    temp_config->config.vol_config.vol_level[i] = vol_max_level;
                                    TRANSMITTER_LOG_E("[BT Audio][DL] ch %d vol level is %d larger than max level, set to max level %d\r\n", 3,
                                        i,
                                        temp_config->config.vol_config.vol_level[i],
                                        vol_max_level
                                        );
                                }
                                gain_info[i].gain_value = audio_get_gain_out_in_dB(temp_config->config.vol_config.vol_level[i], GAIN_DIGITAL, vol_type[index]);
                                gain_info[i].gain_value += g_gain_compensation_table[vol_ratio / AUDIO_DONGLE_GAIN_COMPENSATION_STEP_MAX];
                            }
                            TRANSMITTER_LOG_I("[BT Audio][DL] set volume ch %d vol ignore %d mute %d gain(0.01dB) %d level %d", 5,
                                i,
                                gain_info[i].ignore_flag,
                                gain_info[i].mute_flag,
                                gain_info[i].gain_value,
                                temp_config->config.vol_config.vol_level[i]
                                );
                        }
                        runtime_config_param->config_operation = operation;
                        memcpy(runtime_config_param->config_param, &gain_info, sizeof(bt_audio_dongle_gain_info_t) * BT_AUDIO_VOLUME_CH_MAX);
                        ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                    }
                }
            }
            break;
        default:
            break;
    }
    return ret;
}

audio_transmitter_status_t bt_audio_dongle_get_runtime_config(uint8_t scenario_type, uint8_t scenario_sub_id, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config)
{
    audio_transmitter_status_t ret = AUDIO_TRANSMITTER_STATUS_FAIL;
    switch (scenario_sub_id) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        /* DL HFP Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:

            break;
        /* HFP UL Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:

            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            break;
#endif
        default:
            break;
    }
    return ret;
}

void bt_audio_dongle_state_started_handler(uint8_t scenario_sub_id)
{
    TRANSMITTER_LOG_I("[BT Audio][sub id %d] state started", 1, scenario_sub_id);
    switch (scenario_sub_id) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        /* DL HFP Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
            #if defined(AIR_USB_AUDIO_1_SPK_ENABLE)
                USB_Audio_Register_Rx_Callback(0, audio_dongle_usb0_rx_cb);
                TRANSMITTER_LOG_I("[BT Audio] Register audio_dongle_usb0_rx_cb", 0);
            #else
                AUDIO_ASSERT(0 && "[BT Audio] please enable AIR_USB_AUDIO_1_SPK_ENABLE");
            #endif
            break;
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            #if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
                USB_Audio_Register_Rx_Callback(1, audio_dongle_usb1_rx_cb);
                TRANSMITTER_LOG_I("[BT Audio] Register audio_dongle_usb1_rx_cb", 0);
            #else
                AUDIO_ASSERT(0 && "[BT Audio] please enable AIR_USB_AUDIO_2_SPK_ENABLE");
            #endif
            break;
        /* HFP UL Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            #if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
                USB_Audio_Register_Tx_Callback(0, audio_dongle_usb0_tx_cb);
                TRANSMITTER_LOG_I("[BT Audio] Register audio_dongle_usb0_tx_cb", 0);
            #else
                AUDIO_ASSERT(0 && "[BT Audio] please enable AIR_USB_AUDIO_1_SPK_ENABLE");
            #endif
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            break;
#endif
        default:
            break;
    }
}

void bt_audio_dongle_state_stopping_handler(uint8_t scenario_sub_id)
{
    TRANSMITTER_LOG_I("[BT Audio][sub id %d] state stopping", 1, scenario_sub_id);
    switch (scenario_sub_id) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        /* DL HFP Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
            #if defined(AIR_USB_AUDIO_1_SPK_ENABLE)
                USB_Audio_Register_Rx_Callback(0, NULL);
                TRANSMITTER_LOG_I("[BT Audio] UN-Register audio_dongle_usb0_rx_cb", 0);
            #else
                AUDIO_ASSERT(0 && "[BT Audio] please enable AIR_USB_AUDIO_1_SPK_ENABLE");
            #endif
            break;
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            #if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
                USB_Audio_Register_Rx_Callback(1, NULL);
                TRANSMITTER_LOG_I("[BT Audio] UN-Register audio_dongle_usb1_rx_cb", 0);
            #else
                AUDIO_ASSERT(0 && "[BT Audio] please enable AIR_USB_AUDIO_2_SPK_ENABLE");
            #endif
            break;
        /* HFP UL Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            #if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
                USB_Audio_Register_Tx_Callback(0, NULL);
                TRANSMITTER_LOG_I("[BT Audio] UN-Register audio_dongle_usb0_tx_cb", 0);
            #else
                AUDIO_ASSERT(0 && "[BT Audio] please enable AIR_USB_AUDIO_1_SPK_ENABLE");
            #endif
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            break;
#endif
        default:
            break;
    }
}

void bt_audio_dongle_state_starting_handler(uint8_t scenario_sub_id)
{
    TRANSMITTER_LOG_I("[BT Audio][sub id %d] state start", 1, scenario_sub_id);
    hal_dvfs_lock_control(AUDIO_BT_DONGLE_DVFS_LEVEL, HAL_DVFS_LOCK);
    switch (scenario_sub_id) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        /* DL HFP Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
            audio_dongle_usb_rx_handle[0].stream_is_started = 1;
            audio_dongle_usb_rx_handle[0].dongle_stream_status |= (1 << scenario_sub_id);
            break;
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            audio_dongle_usb_rx_handle[1].stream_is_started = 1;
            audio_dongle_usb_rx_handle[1].dongle_stream_status |= (1 << scenario_sub_id);
            break;
        /* HFP UL Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            audio_dongle_usb_tx_handle[0].stream_is_started = 1;
            audio_dongle_usb_tx_handle[0].dongle_stream_status |= (1 << scenario_sub_id);
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            break;
#endif
        default:
            break;
    }
}

void bt_audio_dongle_state_idle_handler(uint8_t scenario_sub_id)
{
    TRANSMITTER_LOG_I("[BT Audio][sub id %d] state idle", 1, scenario_sub_id);
    hal_dvfs_lock_control(AUDIO_BT_DONGLE_DVFS_LEVEL, HAL_DVFS_UNLOCK);
    switch (scenario_sub_id) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        /* DL HFP Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
            audio_dongle_usb_rx_handle[0].stream_is_started = 0;
            audio_dongle_usb_rx_handle[0].dongle_stream_status &= ~(1 << scenario_sub_id);
            if (audio_dongle_usb_rx_handle[0].dongle_stream_status == 0) {
                memset(&audio_dongle_usb_rx_handle[0], 0, sizeof(audio_dongle_usb_handle_t));
            }
            break;
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            audio_dongle_usb_rx_handle[1].stream_is_started = 0;
            audio_dongle_usb_rx_handle[1].dongle_stream_status &= ~(1 << scenario_sub_id);
            if (audio_dongle_usb_rx_handle[1].dongle_stream_status == 0) {
                memset(&audio_dongle_usb_rx_handle[1], 0, sizeof(audio_dongle_usb_handle_t));
            }
            break;
        /* HFP UL Path */
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            audio_dongle_usb_tx_handle[0].stream_is_started = 0;
            audio_dongle_usb_tx_handle[0].dongle_stream_status &= ~(1 << scenario_sub_id);
            if (audio_dongle_usb_tx_handle[0].dongle_stream_status == 0) {
                memset(&audio_dongle_usb_tx_handle[0], 0, sizeof(audio_dongle_usb_handle_t));
            }
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            break;
#endif
        default:
            break;
    }
}

#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
audio_transmitter_status_t bt_audio_dongle_read_data_from_usb(uint32_t scenario_sub_id, uint8_t *data, uint32_t *length)
{
    uint8_t usb_rx_port = 0;
    if ((scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0) ||
        (scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0)) {
        usb_rx_port = 0;
    } else if ((scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1) ||
               (scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1)) {
        usb_rx_port = 1;
    } else {
        AUDIO_ASSERT(0 && "[BT Audio][DL] Error sub id");
    }
    if (audio_dongle_read_data_from_usb(usb_rx_port, data, length) == false) {
        return AUDIO_TRANSMITTER_STATUS_FAIL;
    }
    return AUDIO_TRANSMITTER_STATUS_SUCCESS;
}

audio_transmitter_status_t bt_audio_dongle_write_data_to_usb(uint32_t scenario_sub_id, uint8_t *data, uint32_t *length)
{
    uint8_t usb_tx_port = 0;
    if (scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0) {
        usb_tx_port = 0;
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1) {
        usb_tx_port = 1;
    } else {
        AUDIO_ASSERT(0 && "[BT Audio][UL] Error sub id");
    }
    if (audio_dongle_write_data_to_usb(usb_tx_port, data, length) == false) {
        return AUDIO_TRANSMITTER_STATUS_FAIL;
    }
    return AUDIO_TRANSMITTER_STATUS_SUCCESS;
}
#endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */

#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
void bt_audio_dongle_silence_detection_enable(audio_scenario_type_t scenario)
{
    audio_transmitter_runtime_config_t config;
    audio_transmitter_status_t ret;
    audio_transmitter_id_t id;
    audio_transmitter_scenario_sub_id_btaudiodongle_t sub_id;
    bool find_out_id_flag = false;
    uint32_t i;

    /* find out audio transmitter id by audio scenario type */
    sub_id = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0 + (scenario-AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0);
    for (i = 0; i < AUDIO_TRANSMITTER_MAX; i++) {
        if ((g_audio_transmitter_control[i].state == AUDIO_TRANSMITTER_STATE_STARTED) &&
            (g_audio_transmitter_control[i].config.scenario_type == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE) &&
            (g_audio_transmitter_control[i].config.scenario_sub_id == sub_id))
        {
            id = i;
            find_out_id_flag = true;
            break;
        }
    }

    /* runtime config silence detection */
    if (find_out_id_flag == true)
    {
        memset(&config, 0, sizeof(audio_transmitter_runtime_config_t)); /* This para is not used, avoid of sanity scan error */
        ret = audio_transmitter_set_runtime_config(id, BT_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_ENABLE, &config);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
            TRANSMITTER_LOG_E("[BT Audio][DL][silence_detection][ERROR] enable silence detection fail, %d\r\n", 1, scenario);
        }
    }
    else
    {
        TRANSMITTER_LOG_E("[BT Audio][DL][silence_detection][ERROR] audio transmitter id is not found, %d\r\n", 1, scenario);
    }
}

void bt_audio_dongle_silence_detection_disable(audio_scenario_type_t scenario)
{
    audio_transmitter_runtime_config_t config;
    audio_transmitter_status_t ret;
    audio_transmitter_id_t id;
    audio_transmitter_scenario_sub_id_btaudiodongle_t sub_id;
    bool find_out_id_flag = false;
    uint32_t i;

    /* find out audio transmitter id by audio scenario type */
    sub_id = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0 + (scenario-AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0);
    for (i = 0; i < AUDIO_TRANSMITTER_MAX; i++) {
        if ((g_audio_transmitter_control[i].state == AUDIO_TRANSMITTER_STATE_STARTED) &&
            (g_audio_transmitter_control[i].config.scenario_type == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE) &&
            (g_audio_transmitter_control[i].config.scenario_sub_id == sub_id))
        {
            id = i;
            find_out_id_flag = true;
            break;
        }
    }

    /* runtime config silence detection */
    if (find_out_id_flag == true)
    {
        memset(&config, 0, sizeof(audio_transmitter_runtime_config_t)); /* This para is not used, avoid of sanity scan error */
        ret = audio_transmitter_set_runtime_config(id, BT_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_DISABLE, &config);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
            TRANSMITTER_LOG_E("[BT Audio][DL][silence_detection][ERROR] disable silence detection fail, %d\r\n", 1, scenario);
        }
    }
    else
    {
        TRANSMITTER_LOG_E("[BT Audio][DL][silence_detection][ERROR] audio transmitter id is not found, %d\r\n", 1, scenario);
    }
}
#endif /* AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE */

#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */
