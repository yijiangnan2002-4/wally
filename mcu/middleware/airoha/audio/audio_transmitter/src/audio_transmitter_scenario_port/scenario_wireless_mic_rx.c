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

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "scenario_wireless_mic_rx.h"
#include "audio_transmitter_playback_port.h"
#include "usbaudio_drv.h"
#include "bt_sink_srv_ami.h"
#include "audio_dump.h"
#include "hal_audio.h"
#include "nvkey.h"

/* Private define ------------------------------------------------------------*/
#define WIRELESS_MIC_RX_DEBUG_UT                    1
#define WIRELESS_MIC_RX_DEBUG_LANTENCY              1
#define WIRELESS_MIC_RX_UL_PATH_DEBUG_LOG           0

#define USB_TX_PORT_TOTAL                               1

#define WIRELESS_MIC_RX_PAYLOAD_SIZE_USB_TX_PCM_MAX              288    //1ms for 48K/24bit/Stereo
#define WIRELESS_MIC_RX_USB_TX_SEND_CCNI_FRAMES                  3

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
#if WIRELESS_MIC_RX_DEBUG_LANTENCY
    hal_gpio_pin_t latency_debug_gpio_pin;
    uint16_t latency_debug_enable;
    int16_t latency_debug_last_sample;
    uint16_t latency_debug_last_level;
#endif /* WIRELESS_MIC_RX_DEBUG_LANTENCY */
    uint8_t first_time;
    uint8_t stream_is_started;
    uint32_t previous_gpt_count;
    audio_transmitter_block_header_t usb_stream_header;
    uint32_t frame_interval; // uint: us
    uint32_t frame_size;
    audio_dsp_codec_type_t usb_type;
    audio_codec_param_t usb_param;
    n9_dsp_share_info_t *p_dsp_info;
} wireless_mic_rx_usb_handle_t;

typedef struct {
    uint32_t vol_ch;
    int32_t  vol_gain[WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER];
} vol_gain_t;

typedef struct {
    uint32_t bt_address_h;
    uint32_t bt_address_l;
    uint32_t channel_num;
} wireless_mic_rx_bt_link_setting_t;

typedef struct {
    int32_t vol_gain[WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER];
    int32_t vol_gain_offset[WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER];
} wireless_mic_rx_vol_local_info_t;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static wireless_mic_rx_usb_handle_t usb_stream_tx_handle[USB_TX_PORT_TOTAL];
static uint32_t wireless_mic_rx_ul_stream_status = 0;
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
static uint8_t all_zero_buffer[WIRELESS_MIC_RX_PAYLOAD_SIZE_USB_TX_PCM_MAX];
#endif /* AIR_USB_AUDIO_1_MIC_ENABLE */
static uint8_t wireless_mic_rx_channel_connection_status_temp[AUDIO_TRANSMITTER_WIRELESS_MIC_RX_SUB_ID_MAX][WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER];
static uint8_t wireless_mic_rx_channel_connection_status[AUDIO_TRANSMITTER_WIRELESS_MIC_RX_SUB_ID_MAX][WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER] =
{
    {0b0101, 0b1010, 0b0000, 0b0000},
    {0b0101, 0b1010, 0b0000, 0b0000},
    {0b0101, 0b1010, 0b0000, 0b0000},
    {0b0101, 0b1010, 0b0000, 0b0000}
};
static wireless_mic_rx_bt_link_setting_t wireless_mic_rx_bt_link_setting[WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER];
static uint32_t wireless_mic_rx_bt_link_codec_sample_rate = 0;
static uint32_t wireless_mic_rx_bt_link_codec_frame_interval = 0;
static uint8_t *wireless_mic_rx_audio_connection_info_all = NULL;
static uint32_t wireless_mic_rx_audio_connection_info_all_size = 0;
static uint8_t *wireless_mic_rx_audio_connection_info_cur = NULL;
static uint32_t wireless_mic_rx_audio_connection_info_size = 0;
static wireless_mic_rx_vol_local_info_t wireless_mic_rx_vol_local_info[AUDIO_TRANSMITTER_WIRELESS_MIC_RX_SUB_ID_MAX];
static bool g_wireless_mic_rx_dvfs_flag = false;

/* Public variables ----------------------------------------------------------*/
#if defined(MTK_AVM_DIRECT)
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
#endif

/* Private functions ---------------------------------------------------------*/
extern void bt_sink_srv_am_set_volume(bt_sink_srv_am_stream_type_t in_out, bt_sink_srv_audio_setting_vol_info_t *vol_info);
static uint32_t wireless_mic_rx_codec_get_frame_size(audio_dsp_codec_type_t *codec_type, audio_codec_param_t *codec_param)
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

        case AUDIO_DSP_CODEC_TYPE_ULD:
            frame_size = codec_param->uld.bit_rate * codec_param->uld.frame_interval / 8 / 1000 / 1000;
            break;

        default:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This codec is not supported at now, %u\r\n", 1, *codec_type);
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

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE

static uint32_t g_wireless_mic_rx_usb_onoff_flag = false;

#if WIRELESS_MIC_RX_DEBUG_LANTENCY
static void usb_audio_tx_cb_latency_debug(wireless_mic_rx_usb_handle_t *handle, uint8_t *source_buf)
{
    int16_t *start_address = (int16_t *)source_buf;
    uint32_t current_level = 0;
    uint32_t i;
    int16_t sample_value = 0;
    uint16_t frame_samples;
    uint16_t channel_num;
    uint16_t resolution_size;

    if (handle->latency_debug_enable)
    {
        channel_num = handle->usb_param.pcm.channel_mode;
        resolution_size = (handle->usb_param.pcm.format == HAL_AUDIO_PCM_FORMAT_S16_LE) ? 2 : 3;
        frame_samples = handle->frame_size / resolution_size / channel_num;
        for (i = 0; i < frame_samples; i++) {
            if (handle->usb_param.pcm.format == HAL_AUDIO_PCM_FORMAT_S16_LE)
            {
                sample_value += (*(start_address + i*channel_num) / frame_samples);
            }
            else
            {
                sample_value += (*(int16_t *)((uint32_t)start_address + i*channel_num*3 + 1) / frame_samples);
            }
        }
        if (sample_value >= 5000)
        {
            current_level = 1;
        }
        else
        {
            current_level = 0;
        }
        if (current_level != handle->latency_debug_last_level) {
            hal_gpio_set_output(handle->latency_debug_gpio_pin, current_level);
            handle->latency_debug_last_level = current_level;
        }
    }
}
#endif /* WIRELESS_MIC_RX_DEBUG_LANTENCY */

void wireless_mic_rx_tx_usb_irq_debug_onoff_control(uint32_t irq_onoff_flag)
{
    g_wireless_mic_rx_usb_onoff_flag = irq_onoff_flag;
}

static void usb_audio_tx_trigger_dsp_flow(uint32_t gpt_count, uint32_t frame_num)
{
    hal_ccni_message_t ccni_msg;
    hal_ccni_status_t ccni_status;

    if (g_wireless_mic_rx_usb_onoff_flag == true) {
        return;
    }

    ccni_msg.ccni_message[0] = gpt_count;
    ccni_msg.ccni_message[1] = frame_num;
    ccni_status = hal_ccni_set_event(CCNI_CM4_TO_DSP0_BT_COMMON, &ccni_msg);
    if (ccni_status != HAL_CCNI_STATUS_OK)
    {
        TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR][usb_audio_tx_trigger_dsp_flow]send ccni fail, %d\r\n", 1, ccni_status);
        // AUDIO_ASSERT(0);
    }

    #if WIRELESS_MIC_RX_UL_PATH_DEBUG_LOG
    TRANSMITTER_LOG_I("[Wireless MIC RX][UL][DEBUG][usb_audio_tx_trigger_dsp_flow]send ccni", 0);
    #endif /* WIRELESS_MIC_RX_UL_PATH_DEBUG_LOG */
}

static void usb_audio_tx_cb_wireless_mic_rx_0(void)
{
    uint32_t gpt_count, duration_count;
    uint32_t data_size;
    n9_dsp_share_info_t *p_dsp_info;
    uint32_t blk_size;
    uint32_t data_size_total;
    uint8_t *p_source_buf;
    uint32_t frame_count;

    /* get current gpt count */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    #if WIRELESS_MIC_RX_VOICE_PATH_DEBUG_LOG
    TRANSMITTER_LOG_I("[Wireless MIC RX][UL]usb_audio_tx_cb_wireless_mic_rx_0 callback = %u", 1, gpt_count);
    #endif /* WIRELESS_MIC_RX_VOICE_PATH_DEBUG_LOG */

    if ((wireless_mic_rx_ul_stream_status & 0x1) == 0)
    {
        /* workaround: If the stream is not started, need to send zero data to usb to avoid usb host error */
        USB_Audio_TX_SendData(0, usb_stream_tx_handle[0].frame_size, (uint8_t *)all_zero_buffer);
        return;
    }

    /* get share buffer info */
    p_dsp_info = usb_stream_tx_handle[0].p_dsp_info;

    /* check usb irq duration */
    if (usb_stream_tx_handle[0].first_time == 0)
    {
        usb_stream_tx_handle[0].previous_gpt_count = gpt_count;
        usb_stream_tx_handle[0].first_time  = 1;

        TRANSMITTER_LOG_I("[Wireless MIC RX][UL][USB_TX_DEBUG 0]usb_audio_tx_cb_wireless_mic_rx_0 callback first time = %u", 1, gpt_count);
    }
    else
    {
        hal_gpt_get_duration_count(usb_stream_tx_handle[0].previous_gpt_count, gpt_count, &duration_count);
        if ((duration_count > 1500) || (duration_count < 500))
        {
            TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR][USB_TX_DEBUG 0]usb_audio_tx_cb_wireless_mic_rx_0 duration = %d", 1, duration_count);
        }
        usb_stream_tx_handle[0].previous_gpt_count = gpt_count;
    }

    /* get data info */
    hal_audio_buf_mgm_get_data_buffer(p_dsp_info, &p_source_buf, &data_size_total);
    if (data_size_total == 0)
    {
        data_size = 0;
        p_source_buf = NULL;
    }
    else
    {
        data_size = ((audio_transmitter_block_header_t *)p_source_buf)->data_length;
        /* check data size */
        if (data_size != usb_stream_tx_handle[0].frame_size)
        {
            TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR]usb_audio_tx_cb_wireless_mic_rx_0 data_size is not right %u, %u, rptr 0x%08x, wptr 0x%08x, len %d", 5, data_size, usb_stream_tx_handle[0].frame_size, p_dsp_info->read_offset, p_dsp_info->write_offset, p_dsp_info->length);
            AUDIO_ASSERT(0);
        }
        p_source_buf += sizeof(audio_transmitter_block_header_t);
    }

    /* get unprocessed frame numbers in the share buffer */
    frame_count = data_size_total / (usb_stream_tx_handle[0].frame_size+sizeof(audio_transmitter_block_header_t));
    /* send ccni to trigger dsp flow in every 1ms */
    usb_audio_tx_trigger_dsp_flow(gpt_count, frame_count);

    /* send usb data */
    if (data_size == 0)
    {
        if (usb_stream_tx_handle[0].stream_is_started == 0)
        {
            /* the stream is not started, so send slience data */
            USB_Audio_TX_SendData(0, usb_stream_tx_handle[0].frame_size, (uint8_t *)all_zero_buffer);
        }
        else
        {
            USB_Audio_TX_SendData(0, usb_stream_tx_handle[0].frame_size, (uint8_t *)all_zero_buffer);
            TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR]usb_audio_tx_cb_wireless_mic_rx_0 data is not enough", 0);
            // AUDIO_ASSERT(0);
        }
    }
    else
    {
        /* set data from share buffer into USB FIFO */
        USB_Audio_TX_SendData(0, usb_stream_tx_handle[0].frame_size, p_source_buf);

        /* drop this packet */
        blk_size = p_dsp_info->sub_info.block_info.block_size;
        hal_audio_buf_mgm_get_read_data_done(p_dsp_info, blk_size);

        if (usb_stream_tx_handle[0].stream_is_started == 0)
        {
            /* set that the stream is started */
            usb_stream_tx_handle[0].stream_is_started = 1;
        }
    }

    #if WIRELESS_MIC_RX_DEBUG_LANTENCY
    if (data_size == 0)
    {
        usb_audio_tx_cb_latency_debug(&usb_stream_tx_handle[0], all_zero_buffer);
    }
    else
    {
        usb_audio_tx_cb_latency_debug(&usb_stream_tx_handle[0], p_source_buf);
    }
    #endif /* WIRELESS_MIC_RX_DEBUG_LANTENCY */
}
#endif /* AIR_USB_AUDIO_1_MIC_ENABLE */

static void wireless_mic_rx_bt_common_source_prepare(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    uint32_t i;
    uint32_t codec_sample_rate = 0;
    uint32_t codec_frame_interval = 0;
    uint32_t channel_num = 0;
    uint32_t payload_size = 0;
    audio_dsp_codec_type_t codec_type = AUDIO_DSP_CODEC_TYPE_MAX;

    if ((config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_num == 0) ||
        (config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_num > WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER))
    {
        TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR]Error ul link num %d\r\n", 1, config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_num);
        AUDIO_ASSERT(0);
    }
    open_param->param.stream_in                                         = STREAM_IN_BT_COMMON;
    open_param->stream_in_param.bt_common.scenario_type                 = config->scenario_type;
    open_param->stream_in_param.bt_common.scenario_sub_id               = config->scenario_sub_id;
    open_param->stream_in_param.bt_common.share_info_type               = SHARE_BUFFER_INFO_TYPE;
    open_param->stream_in_param.bt_common.p_share_info                  = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRELESS_MIC_RX_BT_RECEIVE_FROM_AIR_0);
    open_param->stream_in_param.bt_common.data_notification_frequency   = 0;
    for (i = 0; i < config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_num; i++)
    {
        if (config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].enable)
        {
            if (codec_type == AUDIO_DSP_CODEC_TYPE_MAX) {
                codec_type = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_type;
            } else {
                if (codec_type != config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_type) {
                    TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR]Error codec type %d, %d\r\n", 2, codec_type, config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_type);
                    AUDIO_ASSERT(0);
                }
            }
            payload_size = wireless_mic_rx_codec_get_frame_size(  &(config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_type),
                                                            &(config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param));
            if ((payload_size == 0) || (payload_size != config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.frame_size))
            {
                TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR]Error codec frame size %d, %d\r\n", 2, payload_size, config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.frame_size);
                AUDIO_ASSERT(0);
            }
            if (open_param->stream_in_param.bt_common.max_payload_size < (payload_size+sizeof(WIRELESS_MIC_RX_HEADER)))
            {
                open_param->stream_in_param.bt_common.max_payload_size = payload_size+sizeof(WIRELESS_MIC_RX_HEADER);
            }
            if ((((n9_dsp_share_info_t *)(config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].share_info))->sub_info.block_info.block_size) != ((payload_size+sizeof(WIRELESS_MIC_RX_HEADER)+3)/4*4))
            {
                TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR]Error packet size %d, %d\r\n", 2, payload_size, (((n9_dsp_share_info_t *)(config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].share_info))->sub_info.block_info.block_size));
                AUDIO_ASSERT(0);
            }
            open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].share_info                          = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].share_info;
            open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_type                          = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_type;
            if (config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_type == AUDIO_DSP_CODEC_TYPE_LC3PLUS)
            {
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.lc3plus.sample_rate     = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.sample_rate;
                if (codec_sample_rate == 0)
                {
                    codec_sample_rate = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.sample_rate;
                }
                else
                {
                    if (codec_sample_rate != config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.sample_rate)
                    {
                        TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR] lc3plus Error sample rate %d, %d\r\n", 2, codec_sample_rate, config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.sample_rate);
                        AUDIO_ASSERT(0);
                    }
                }
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.lc3plus.sample_format   = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.sample_format;
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.lc3plus.channel_mode    = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.channel_mode;
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.lc3plus.frame_interval  = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.frame_interval;
                if (codec_frame_interval == 0)
                {
                    codec_frame_interval = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.frame_interval;
                }
                else
                {
                    if (codec_frame_interval != config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.frame_interval)
                    {
                        TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR] lc3plus Error frame interval %d, %d\r\n", 2, codec_frame_interval, config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.frame_interval);
                        AUDIO_ASSERT(0);
                    }
                }
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.lc3plus.frame_size      = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.frame_size;
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.lc3plus.bit_rate        = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.bit_rate;
                channel_num = (config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.channel_mode==0x1)?1:2;
                TRANSMITTER_LOG_I("[Wireless MIC RX][UL] lc3plus, BT link codec setting ch%u: %u, %u, %u, %u, %u, %u, %u, %u, 0x%x, 0x%x, 0x%x, 0x%x\r\n", 13,
                                    i+1,
                                    config->scenario_sub_id,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_type,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.sample_rate,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.sample_format,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.channel_mode,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.frame_interval,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.frame_size,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.lc3plus.bit_rate,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].share_info,
                                    ((n9_dsp_share_info_t *)(config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].share_info))->start_addr,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].bt_address_h,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].bt_address_l);
            } else if (config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_type == AUDIO_DSP_CODEC_TYPE_ULD) {
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.uld.sample_rate     = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.sample_rate;
                if (codec_sample_rate == 0)
                {
                    codec_sample_rate = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.sample_rate;
                }
                else
                {
                    if (codec_sample_rate != config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.sample_rate)
                    {
                        TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR] uld Error sample rate %d, %d\r\n", 2, codec_sample_rate, config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.sample_rate);
                        AUDIO_ASSERT(0);
                    }
                }
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.uld.sample_format   = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.sample_format;
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.uld.channel_mode    = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.channel_mode;
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.uld.frame_interval  = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.frame_interval;
                if (codec_frame_interval == 0)
                {
                    codec_frame_interval = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.frame_interval;
                }
                else
                {
                    if (codec_frame_interval != config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.frame_interval)
                    {
                        TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR] uld Error frame interval %d, %d\r\n", 2, codec_frame_interval, config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.frame_interval);
                        AUDIO_ASSERT(0);
                    }
                }
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.uld.frame_size      = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.frame_size;
                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].codec_param.uld.bit_rate        = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.bit_rate;
                channel_num = (config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.channel_mode==0x1) ? 1 : 2;
                TRANSMITTER_LOG_I("[Wireless MIC RX][UL] uld, BT link codec setting ch%u: %u, %u, %u, %u, %u, %u, %u, %u, 0x%x, 0x%x, 0x%x, 0x%x\r\n", 13,
                                    i+1,
                                    config->scenario_sub_id,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_type,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.sample_rate,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.sample_format,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.channel_mode,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.frame_interval,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.frame_size,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].codec_param.uld.bit_rate,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].share_info,
                                    ((n9_dsp_share_info_t *)(config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].share_info))->start_addr,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].bt_address_h,
                                    config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].bt_address_l);
            }
            /* record bt address */
            wireless_mic_rx_bt_link_setting[i].bt_address_h = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].bt_address_h;
            wireless_mic_rx_bt_link_setting[i].bt_address_l = config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[i].bt_address_l;
            wireless_mic_rx_bt_link_setting[i].channel_num  = channel_num;
        }
        else
        {
            open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.bt_link_info[i].share_info = NULL;
            /* record bt address */
            wireless_mic_rx_bt_link_setting[i].bt_address_h = 0;
            wireless_mic_rx_bt_link_setting[i].bt_address_l = 0;
            wireless_mic_rx_bt_link_setting[i].channel_num  = 0;
        }
    }

    /* save global status */
    wireless_mic_rx_bt_link_codec_sample_rate = codec_sample_rate;
    wireless_mic_rx_bt_link_codec_frame_interval = codec_frame_interval;
}

/* Public functions ----------------------------------------------------------*/
#if WIRELESS_MIC_RX_DEBUG_UT
void wireless_mic_rx_ut_music_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    switch (event)
    {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS:
            TRANSMITTER_LOG_I("[wireless_mic_rx_ut_music_callback]: start successfully.", 0);
            break;

        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS:
            TRANSMITTER_LOG_I("[wireless_mic_rx_ut_music_callback]: stop successfully.", 0);
            break;

        default:
            TRANSMITTER_LOG_I("[wireless_mic_rx_ut_music_callback]: error event = %d.", 1, event);
            break;
    }
}
#endif /* WIRELESS_MIC_RX_DEBUG_UT */

#if WIRELESS_MIC_RX_DEBUG_LANTENCY
void wireless_mic_rx_tx_latency_debug_control(uint32_t port, bool enable, uint32_t gpio_num)
{
    uint32_t saved_mask;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (enable) {
        usb_stream_tx_handle[port].latency_debug_enable = 1;
        usb_stream_tx_handle[port].latency_debug_last_level = 0;
        usb_stream_tx_handle[port].latency_debug_last_sample = 0;
        usb_stream_tx_handle[port].latency_debug_gpio_pin = gpio_num;
    } else {
        usb_stream_tx_handle[port].latency_debug_enable = 0;
        usb_stream_tx_handle[port].latency_debug_last_level = 0;
        usb_stream_tx_handle[port].latency_debug_last_sample = 0;
        usb_stream_tx_handle[port].latency_debug_gpio_pin = gpio_num;
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    hal_gpio_set_output(usb_stream_tx_handle[port].latency_debug_gpio_pin, HAL_GPIO_DATA_LOW);
}
#endif /* WIRELESS_MIC_RX_DEBUG_LANTENCY */

void wireless_mic_rx_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    uint32_t payload_size = 0;
    sysram_status_t status;
    DSP_FEATURE_TYPE_LIST wireless_mic_rx_nvkey_list[] = {
        FUNC_WIRELESS_MIC,
        FUNC_END,
    };

    switch (config->scenario_sub_id)
    {
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0:
            /* configure stream source */
            /* prepare bt-out parameters to dsp */
            wireless_mic_rx_bt_common_source_prepare(config, open_param);
            /* prepare default gain settings */
            open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_L = -14400; // -144.00dB
            open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_R = -14400; // -144.00dB
            // open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_L = 0; // 0dB
            // open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_R = 0; // 0dB
            /* reset share buffer before put parameters*/
            audio_nvdm_reset_sysram();
            status = audio_nvdm_set_feature(sizeof(wireless_mic_rx_nvkey_list) / sizeof(DSP_FEATURE_TYPE_LIST), wireless_mic_rx_nvkey_list);
            if (status != NVDM_STATUS_NAT_OK) {
                TRANSMITTER_LOG_E("[Wireless MIC RX][UL] failed to set parameters to share memory - err(%d)\r\n", 1, status);
                AUDIO_ASSERT(0);
            }

            /* configure stream sink */
            /* get usb frame size */
            payload_size = usb_audio_get_frame_size(&(config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_type),
                                                    &(config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param));
            if (payload_size == 0)
            {
                TRANSMITTER_LOG_E("[Wireless MIC RX][UL][ERROR]Error usb frame size %d\r\n", 1, payload_size);
                AUDIO_ASSERT(0);
            }
            /* prepare usb-out paramters to USB Audio callback */
            usb_stream_tx_handle[config->scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0].frame_size                     = payload_size;
            usb_stream_tx_handle[config->scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0].frame_interval                 = config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.frame_interval;
            usb_stream_tx_handle[config->scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0].usb_type                       = config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_type;
            usb_stream_tx_handle[config->scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0].usb_param.pcm.sample_rate      = config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.sample_rate;
            usb_stream_tx_handle[config->scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0].usb_param.pcm.channel_mode     = config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.channel_mode;
            usb_stream_tx_handle[config->scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0].usb_param.pcm.format           = config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.format;
            usb_stream_tx_handle[config->scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0].p_dsp_info                     = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRELESS_MIC_RX_DSP_SEND_TO_MCU_0);
            /* prepare usb-out parameters to dsp */
            open_param->param.stream_out                                                                                = STREAM_OUT_AUDIO_TRANSMITTER;
            open_param->stream_out_param.data_ul.scenario_type                                                          = config->scenario_type;
            open_param->stream_out_param.data_ul.scenario_sub_id                                                        = config->scenario_sub_id;
            open_param->stream_out_param.data_ul.max_payload_size                                                       = payload_size;
            open_param->stream_out_param.data_ul.data_notification_frequency                                            = 0;
            open_param->stream_out_param.data_ul.scenario_param.wireless_mic_rx_param.codec_type                        = config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_type;
            open_param->stream_out_param.data_ul.scenario_param.wireless_mic_rx_param.codec_param.pcm.sample_rate       = config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.sample_rate;
            open_param->stream_out_param.data_ul.scenario_param.wireless_mic_rx_param.codec_param.pcm.channel_mode      = config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.channel_mode;
            open_param->stream_out_param.data_ul.scenario_param.wireless_mic_rx_param.codec_param.pcm.format            = config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.format;
            open_param->stream_out_param.data_ul.scenario_param.wireless_mic_rx_param.codec_param.pcm.frame_interval    = config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.frame_interval;
            open_param->stream_out_param.data_ul.p_share_info                                                           = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRELESS_MIC_RX_DSP_SEND_TO_MCU_0);
            open_param->stream_out_param.data_ul.p_share_info->read_offset                                              = 0;
            open_param->stream_out_param.data_ul.p_share_info->write_offset                                             = 0;
            open_param->stream_out_param.data_ul.p_share_info->bBufferIsFull                                            = false;
            audio_transmitter_modify_share_info_by_block(open_param->stream_out_param.data_ul.p_share_info, payload_size);
            TRANSMITTER_LOG_I("[Wireless MIC RX][UL]USB setting: %u, %u, %u, %u, %u, %u, %u, %d, %d, 0x%x, 0x%x\r\n", 11,
                                config->scenario_sub_id,
                                config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_type,
                                config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.sample_rate,
                                config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.channel_mode,
                                config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.format,
                                config->scenario_config.wireless_mic_rx_config.ul_config.sink_param.usb_out_param.codec_param.pcm.frame_interval,
                                payload_size,
                                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_L,
                                open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_R,
                                open_param->stream_out_param.data_ul.p_share_info,
                                open_param->stream_out_param.data_ul.p_share_info->start_addr);

            /* update state machine */
            wireless_mic_rx_ul_stream_status |= 0x1<<(config->scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT:

            //config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_num = 1; /* Temp workaround for debug */

            TRANSMITTER_LOG_I("[Wireless MIC RX][UL]Line Out setting: %d, %d, %d, %d, %d, %d, %d, %d, %d, %x, %x", 11,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_num,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[0].codec_type,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[0].codec_param.uld.bit_rate,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[0].codec_param.uld.channel_mode,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[0].codec_param.uld.frame_interval,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[0].codec_param.uld.frame_size,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[0].codec_param.uld.sample_format,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[0].codec_param.uld.sample_rate,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[0].enable,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[0].bt_address_l,
                                config->scenario_config.wireless_mic_rx_config.ul_config.source_param.bt_in_param.link_param[0].bt_address_h);

            /* configure stream source */
            /* prepare bt-out parameters to dsp */
            wireless_mic_rx_bt_common_source_prepare(config, open_param);
            /* prepare default gain settings */
            open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_L = -14400; // -144.00dB
            open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_R = -14400; // -144.00dB
            // open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_L = 0; // 0dB
            // open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_R = 0; // 0dB
            /* prepare drc feature's NVKEY */
            /* reset share buffer before put parameters*/
            audio_nvdm_reset_sysram();
            status = audio_nvdm_set_feature(sizeof(wireless_mic_rx_nvkey_list) / sizeof(DSP_FEATURE_TYPE_LIST), wireless_mic_rx_nvkey_list);
            if (status != NVDM_STATUS_NAT_OK) {
                TRANSMITTER_LOG_E("[Wireless MIC RX][UL] failed to set parameters to share memory - err(%d)\r\n", 1, status);
                AUDIO_ASSERT(0);
            }

            /* configure stream sink */
            /* config HW analog gain */
            bt_sink_srv_audio_setting_vol_info_t vol_info;
            memset(&vol_info, 0, sizeof(bt_sink_srv_audio_setting_vol_info_t));
            vol_info.type = VOL_A2DP;
            vol_info.vol_info.a2dp_vol_info.dev = HAL_AUDIO_DEVICE_HEADSET;
            vol_info.vol_info.a2dp_vol_info.lev = 0;
            bt_sink_srv_am_set_volume(STREAM_OUT,  &vol_info);
            /* config AFE */
            open_param->param.stream_out = STREAM_OUT_AFE;
            hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param->stream_out_param);
            open_param->stream_out_param.afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
            open_param->stream_out_param.afe.stream_channel = HAL_AUDIO_DIRECT;
            open_param->stream_out_param.afe.memory = HAL_AUDIO_MEM3;
            open_param->stream_out_param.afe.audio_interface = HAL_AUDIO_INTERFACE_1;
            open_param->stream_out_param.afe.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
            open_param->stream_out_param.afe.sampling_rate = wireless_mic_rx_bt_link_codec_sample_rate;
            open_param->stream_out_param.afe.stream_out_sampling_rate = wireless_mic_rx_bt_link_codec_sample_rate;
            open_param->stream_out_param.afe.irq_period = 0;
            open_param->stream_out_param.afe.frame_size = wireless_mic_rx_bt_link_codec_sample_rate/1000; /* frame samples */
            open_param->stream_out_param.afe.frame_number = wireless_mic_rx_bt_link_codec_frame_interval*4/1000+1;
            open_param->stream_out_param.afe.hw_gain = false;
            open_param->stream_out_param.afe.misc_parms = DOWNLINK_PERFORMANCE_NORMAL;
            TRANSMITTER_LOG_I("[Wireless MIC RX][UL]Line Out setting: %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", 14,
                                config->scenario_sub_id,
                                open_param->stream_out_param.afe.audio_device,
                                open_param->stream_out_param.afe.stream_channel,
                                open_param->stream_out_param.afe.memory,
                                open_param->stream_out_param.afe.audio_interface,
                                open_param->stream_out_param.afe.format,
                                open_param->stream_out_param.afe.sampling_rate,
                                open_param->stream_out_param.afe.stream_out_sampling_rate,
                                open_param->stream_out_param.afe.frame_size,
                                open_param->stream_out_param.afe.frame_number,
                                open_param->stream_out_param.afe.hw_gain,
                                open_param->stream_out_param.afe.misc_parms,
                                open_param->stream_out_param.afe.performance,
                                open_param->stream_out_param.afe.dl_dac_mode);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_MST_OUT_0:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, config->scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0:
            /* configure stream source */
            /* prepare bt-out parameters to dsp */
            wireless_mic_rx_bt_common_source_prepare(config, open_param);
            /* prepare default gain settings */
            open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_L = -14400; // -144.00dB
            open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_R = -14400; // -144.00dB
            // open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_L = 0; // 0dB
            // open_param->stream_in_param.bt_common.scenario_param.wireless_mic_rx_param.gain_default_R = 0; // 0dB
            /* reset share buffer before put parameters*/
            audio_nvdm_reset_sysram();
            status = audio_nvdm_set_feature(sizeof(wireless_mic_rx_nvkey_list) / sizeof(DSP_FEATURE_TYPE_LIST), wireless_mic_rx_nvkey_list);
            if (status != NVDM_STATUS_NAT_OK) {
                TRANSMITTER_LOG_E("[Wireless MIC RX][UL] failed to set parameters to share memory - err(%d)\r\n", 1, status);
                AUDIO_ASSERT(0);
            }

            /* configure stream sink */
            open_param->param.stream_out = STREAM_OUT_AFE;
            hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param->stream_out_param);
            open_param->stream_out_param.afe.audio_device = HAL_AUDIO_DEVICE_I2S_SLAVE;
            open_param->stream_out_param.afe.audio_device1 = HAL_AUDIO_DEVICE_I2S_SLAVE;
            open_param->stream_out_param.afe.stream_channel = HAL_AUDIO_DIRECT;
            open_param->stream_out_param.afe.memory = HAL_AUDIO_MEM1;
            open_param->stream_out_param.afe.audio_interface     = HAL_AUDIO_INTERFACE_3;
            open_param->stream_out_param.afe.audio_interface1    = HAL_AUDIO_INTERFACE_3;
            open_param->stream_out_param.afe.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
            open_param->stream_out_param.afe.sampling_rate = wireless_mic_rx_bt_link_codec_sample_rate;
            open_param->stream_out_param.afe.stream_out_sampling_rate = wireless_mic_rx_bt_link_codec_sample_rate;
            open_param->stream_out_param.afe.irq_period = 0;
            open_param->stream_out_param.afe.frame_size = wireless_mic_rx_bt_link_codec_sample_rate/1000; /* frame samples */
            open_param->stream_out_param.afe.frame_number = wireless_mic_rx_bt_link_codec_frame_interval*4/1000+1;;
            open_param->stream_out_param.afe.hw_gain = false;
            /* set i2s slave format */
            open_param->stream_out_param.afe.i2s_format          = HAL_AUDIO_I2S_I2S;
            open_param->stream_out_param.afe.i2s_word_length     = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;
            TRANSMITTER_LOG_I("[Wireless MIC RX][UL]I2S_SLV Out setting: %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", 15,
                                config->scenario_sub_id,
                                open_param->stream_out_param.afe.audio_device,
                                open_param->stream_out_param.afe.audio_device1,
                                open_param->stream_out_param.afe.stream_channel,
                                open_param->stream_out_param.afe.memory,
                                open_param->stream_out_param.afe.audio_interface,
                                open_param->stream_out_param.afe.audio_interface1,
                                open_param->stream_out_param.afe.format,
                                open_param->stream_out_param.afe.stream_out_sampling_rate,
                                (uint32_t)open_param->stream_out_param.afe.irq_period,
                                open_param->stream_out_param.afe.frame_size,
                                open_param->stream_out_param.afe.frame_number,
                                open_param->stream_out_param.afe.hw_gain,
                                open_param->stream_out_param.afe.i2s_format,
                                open_param->stream_out_param.afe.i2s_word_length);
#ifdef AIR_BTA_IC_PREMIUM_G2
            clock_mux_sel(CLK_AUD_GPSRC_SEL, 2); // boost hwsrc convert speed
#endif /* AIR_BTA_IC_PREMIUM_G2 */
            break;

        default:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not in the list, %u\r\n", 1, config->scenario_sub_id);
            AUDIO_ASSERT(0);
            break;
    }
}

void wireless_mic_rx_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param)
{
    switch (config->scenario_sub_id)
    {
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0:
            start_param->param.stream_in  = STREAM_IN_BT_COMMON;
            start_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT:
            start_param->param.stream_in  = STREAM_IN_BT_COMMON;
            start_param->param.stream_out = STREAM_OUT_AFE;
            start_param->stream_out_param.afe.aws_flag =  true;
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_MST_OUT_0:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, config->scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0:
            start_param->param.stream_in  = STREAM_IN_BT_COMMON;
            start_param->param.stream_out = STREAM_OUT_AFE;
            start_param->stream_out_param.afe.aws_flag =  true;
            break;

        default:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not in the list, %u\r\n", 1, config->scenario_sub_id);
            AUDIO_ASSERT(0);
            break;
    }
}

audio_transmitter_status_t wireless_mic_rx_set_runtime_config_playback(audio_transmitter_config_t *config, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config, mcu2dsp_audio_transmitter_runtime_config_param_t *runtime_config_param)
{
    audio_transmitter_status_t ret = AUDIO_TRANSMITTER_STATUS_FAIL;
    uint32_t operation = runtime_config_type;
    vol_gain_t gain;
    int32_t vol_gain;
    int32_t vol_level;
    uint32_t i;

    switch (config->scenario_sub_id)
    {
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0:
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT:
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0:
            switch (operation)
            {
                case WIRELESS_MIC_RX_CONFIG_OP_SET_UL_VOL_INFO:
                    vol_gain        = runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_gain/100;
                    gain.vol_ch     = runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_ch;
                    if (gain.vol_ch == 0)
                    {
                        for (i = 0; i < WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER; i++)
                        {
                            wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain[i] = vol_gain;
                            gain.vol_gain[i] = vol_gain + wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain_offset[i];
                        }
                    }
                    else
                    {
                        wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain[gain.vol_ch-1] = vol_gain;
                        gain.vol_gain[gain.vol_ch-1] = vol_gain + wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain_offset[gain.vol_ch-1];
                    }
                    runtime_config_param->config_operation = operation;
                    memcpy(runtime_config_param->config_param, &gain, sizeof(vol_gain_t));
                    TRANSMITTER_LOG_I("[Wireless MIC RX][UL]scenario_sub_id =%d: operation %d :volume gain=%d, volume_ratio = %d, volume channel=%d, finial gain=%d.",
                                        6 ,
                                        config->scenario_sub_id,
                                        operation,
                                        runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_gain,
                                        runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_ratio,
                                        runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_ch,
                                        gain.vol_gain);
                    ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                    break;

                case WIRELESS_MIC_RX_CONFIG_OP_SET_UL_VOL_LEVEL:
                    vol_level       = runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_level;
                    if (vol_level > bt_sink_srv_ami_get_usb_voice_sw_max_volume_level()) {
                        vol_level = bt_sink_srv_ami_get_usb_voice_sw_max_volume_level();
                        TRANSMITTER_LOG_E("[Wireless MIC RX]][UL][ERROR]set volume %d level more than max level %d\r\n", 2, vol_level, bt_sink_srv_ami_get_usb_voice_sw_max_volume_level());
                    }
                    vol_gain        = audio_get_gain_in_in_dB(vol_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
                    gain.vol_ch     = runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_ch;
                    if (gain.vol_ch == 0)
                    {
                        for (i = 0; i < WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER; i++)
                        {
                            wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain[i] = vol_gain;
                            gain.vol_gain[i] = vol_gain + wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain_offset[i];
                        }
                    }
                    else
                    {
                        wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain[gain.vol_ch-1] = vol_gain;
                        gain.vol_gain[gain.vol_ch-1] = vol_gain + wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain_offset[gain.vol_ch-1];
                    }
                    runtime_config_param->config_operation = WIRELESS_MIC_RX_CONFIG_OP_SET_UL_VOL_INFO;
                    memcpy(runtime_config_param->config_param, &gain, sizeof(vol_gain_t));
                    TRANSMITTER_LOG_I("[Wireless MIC RX]][UL]scenario_sub_id =%d: operation %d :volume level=%d, volume_ratio = %d, volume channel=%d, finial gain=%d.",
                                        6 ,
                                        config->scenario_sub_id,
                                        operation,
                                        runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_level,
                                        runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_ratio,
                                        runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_ch,
                                        gain.vol_gain);
                    ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                    break;

                case WIRELESS_MIC_RX_CONFIG_OP_SET_UL_VOL_OFFSET:
                    vol_gain        = runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_gain_offset;
                    gain.vol_ch     = runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_ch;
                    if (gain.vol_ch == 0)
                    {
                        for (i = 0; i < WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER; i++)
                        {
                            wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain_offset[i] = vol_gain;
                            gain.vol_gain[i] = vol_gain + wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain[i];
                        }
                    }
                    else
                    {
                        wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain_offset[gain.vol_ch-1] = vol_gain;
                        gain.vol_gain[gain.vol_ch-1] = vol_gain + wireless_mic_rx_vol_local_info[config->scenario_sub_id].vol_gain[gain.vol_ch-1];
                    }
                    runtime_config_param->config_operation = WIRELESS_MIC_RX_CONFIG_OP_SET_UL_VOL_INFO;
                    memcpy(runtime_config_param->config_param, &gain, sizeof(vol_gain_t));
                    TRANSMITTER_LOG_I("[Wireless MIC RX][UL]scenario_sub_id =%d: operation %d :volume gain offset=%d, volume_ratio = %d, volume channel=%d, finial gain=%d.",
                                        6 ,
                                        config->scenario_sub_id,
                                        operation,
                                        runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_gain_offset,
                                        runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_ratio,
                                        runtime_config->wireless_mic_rx_runtime_config.vol_info.vol_ch,
                                        gain.vol_gain);
                    ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                    break;

                case WIRELESS_MIC_RX_CONFIG_OP_SET_UL_CONNECTION_INFO:
                    if (wireless_mic_rx_audio_connection_info_cur != NULL)
                    {
                        free(wireless_mic_rx_audio_connection_info_cur);
                        TRANSMITTER_LOG_I("[Wireless MIC RX] free the last connection info, address = 0x%x, size = %u", 2, wireless_mic_rx_audio_connection_info_cur, wireless_mic_rx_audio_connection_info_size);
                    }
                    wireless_mic_rx_audio_connection_info_size = runtime_config->wireless_mic_rx_runtime_config.connection_info.size;
                    wireless_mic_rx_audio_connection_info_cur = malloc(wireless_mic_rx_audio_connection_info_size);
                    if (wireless_mic_rx_audio_connection_info_cur == NULL)
                    {
                        AUDIO_ASSERT(0);
                    }
                    TRANSMITTER_LOG_I("[Wireless MIC RX] the currnet connection info, address = 0x%x, size = %u", 2, wireless_mic_rx_audio_connection_info_cur, wireless_mic_rx_audio_connection_info_size);
                    memcpy(wireless_mic_rx_audio_connection_info_cur, (uint8_t *)(runtime_config->wireless_mic_rx_runtime_config.connection_info.info), wireless_mic_rx_audio_connection_info_size);
                    wireless_mic_rx_audio_connection_info_free((uint8_t *)(runtime_config->wireless_mic_rx_runtime_config.connection_info.info));
                    wireless_mic_rx_audio_connection_info_parse(wireless_mic_rx_audio_connection_info_cur, wireless_mic_rx_audio_connection_info_size);
                    runtime_config_param->config_operation = operation;
                    memcpy(runtime_config_param->config_param, &(wireless_mic_rx_channel_connection_status[0][0]), sizeof(wireless_mic_rx_channel_connection_status));
                    for (i = 0; i < AUDIO_TRANSMITTER_WIRELESS_MIC_RX_SUB_ID_MAX; i++)
                    {
                        TRANSMITTER_LOG_I("[Wireless MIC RX]][UL]scenario_sub_id =%d: operation %d : 0x%x, 0x%x, 0x%x, 0x%x.",
                                            6,
                                            i,
                                            operation,
                                            wireless_mic_rx_channel_connection_status[i][0],
                                            wireless_mic_rx_channel_connection_status[i][1],
                                            wireless_mic_rx_channel_connection_status[i][2],
                                            wireless_mic_rx_channel_connection_status[i][3]);
                    }
                    ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                    break;

                case WIRELESS_MIC_RX_CONFIG_OP_SET_UL_BT_ADDRESS:
                    if (runtime_config->wireless_mic_rx_runtime_config.bt_address_info.bt_ch >= WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER)
                    {
                        TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR] bt address channel num is not right, %u\r\n", 1, runtime_config->wireless_mic_rx_runtime_config.bt_address_info.bt_ch);
                        AUDIO_ASSERT(0);
                    }
                    wireless_mic_rx_bt_link_setting[runtime_config->wireless_mic_rx_runtime_config.bt_address_info.bt_ch].bt_address_h = runtime_config->wireless_mic_rx_runtime_config.bt_address_info.bt_address_h;
                    wireless_mic_rx_bt_link_setting[runtime_config->wireless_mic_rx_runtime_config.bt_address_info.bt_ch].bt_address_l = runtime_config->wireless_mic_rx_runtime_config.bt_address_info.bt_address_l;
                    TRANSMITTER_LOG_I("[Wireless MIC RX]][UL]scenario_sub_id =%d: operation %d :bt ch =%d, bt addr h = 0x%x, bt addr l = 0x%x.",
                                        5 ,
                                        config->scenario_sub_id,
                                        operation,
                                        runtime_config->wireless_mic_rx_runtime_config.bt_address_info.bt_ch,
                                        runtime_config->wireless_mic_rx_runtime_config.bt_address_info.bt_address_h,
                                        runtime_config->wireless_mic_rx_runtime_config.bt_address_info.bt_address_l);
                    ret = AUDIO_TRANSMITTER_STATUS_FAIL;
                    break;

                default:
                    break;
            }
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_MST_OUT_0:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, config->scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        default:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not in the list, %u\r\n", 1, config->scenario_sub_id);
            AUDIO_ASSERT(0);
            break;
    }

    return ret;
}

audio_transmitter_status_t wireless_mic_rx_get_runtime_config(uint8_t scenario_type, uint8_t scenario_sub_id, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config)
{
    audio_transmitter_status_t ret = AUDIO_TRANSMITTER_STATUS_FAIL;

    switch (scenario_sub_id)
    {
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_MST_OUT_0:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        default:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not in the list, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;
    }

    return ret;
}

void wireless_mic_rx_state_started_handler(uint8_t scenario_sub_id)
{
    uint32_t saved_mask;

    switch (scenario_sub_id)
    {
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            usb_stream_tx_handle[0].first_time = 0;
            usb_stream_tx_handle[0].stream_is_started = 0;
            #if WIRELESS_MIC_RX_DEBUG_LANTENCY
            usb_stream_tx_handle[0].latency_debug_enable = 0;
            usb_stream_tx_handle[0].latency_debug_gpio_pin = HAL_GPIO_13;
            #endif /* WIRELESS_MIC_RX_DEBUG_LANTENCY */
            wireless_mic_rx_ul_stream_status |= 0x1<<(scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0);
            hal_nvic_restore_interrupt_mask(saved_mask);
            #ifdef AIR_USB_AUDIO_1_MIC_ENABLE
            USB_Audio_Register_Tx_Callback(0, usb_audio_tx_cb_wireless_mic_rx_0);
            TRANSMITTER_LOG_I("[Wireless MIC RX][USB_TX_DEBUG 0]Register usb_audio_tx_cb 0", 0);
            #endif /* AIR_USB_AUDIO_1_MIC_ENABLE */
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            wireless_mic_rx_ul_stream_status |= 0x1<<(scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0);
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_MST_OUT_0:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            wireless_mic_rx_ul_stream_status |= 0x1<<(scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0);
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        default:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not in the list, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;
    }
}

void wireless_mic_rx_state_idle_handler(uint8_t scenario_sub_id)
{
    uint32_t saved_mask;

    switch (scenario_sub_id)
    {
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            usb_stream_tx_handle[0].first_time = 0;
            usb_stream_tx_handle[0].stream_is_started = 0;
            #if WIRELESS_MIC_RX_DEBUG_LANTENCY
            usb_stream_tx_handle[0].latency_debug_enable = 0;
            #endif /* WIRELESS_MIC_RX_DEBUG_LANTENCY */
            wireless_mic_rx_ul_stream_status &= ~(0x1<<(scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0));
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            wireless_mic_rx_ul_stream_status &= ~(0x1<<(scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0));
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_MST_OUT_0:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            wireless_mic_rx_ul_stream_status &= ~(0x1<<(scenario_sub_id-AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0));
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        default:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not in the list, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;
    }

    if (g_wireless_mic_rx_dvfs_flag == true) {
        switch (scenario_sub_id) {
            case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0:
            case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT:
            case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0:
                /* there is counter in DVFS API, so do not need add counter here */
#if defined(AIR_BTA_IC_PREMIUM_G2)
                hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_UNLOCK);
#else
                hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_UNLOCK);
#endif
                TRANSMITTER_LOG_I("[Wireless MIC RX] unlock cpu to low", 0);
                break;
        }
        g_wireless_mic_rx_dvfs_flag = false;
    }
}

void wireless_mic_rx_state_starting_handler(uint8_t scenario_sub_id)
{
    switch (scenario_sub_id)
    {
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0:
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT:
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0:
            /* there is counter in DVFS API, so do not need add counter here */
            #if defined(AIR_BTA_IC_PREMIUM_G2)
            hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_LOCK);
            #else
            hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
            #endif
            TRANSMITTER_LOG_I("[Wireless MIC RX] lock cpu to high", 0);
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_MST_OUT_0:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        default:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not in the list, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;
    }
}

void wireless_mic_rx_state_stoping_handler(uint8_t scenario_sub_id)
{
    switch (scenario_sub_id)
    {
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0:
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT:
        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0:
            g_wireless_mic_rx_dvfs_flag = true;
            break;

        case AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_MST_OUT_0:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not supported at now, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;

        default:
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This scenario is not in the list, %u\r\n", 1, scenario_sub_id);
            AUDIO_ASSERT(0);
            break;
    }
}

uint8_t *wireless_mic_rx_audio_connection_info_malloc(uint32_t info_size)
{
    uint8_t *info_address;

    info_address = malloc(info_size);

    return info_address;
}

void wireless_mic_rx_audio_connection_info_free(uint8_t *info)
{
    free(info);
}

void wireless_mic_rx_audio_connection_info_get(uint32_t mode, uint8_t **info, uint32_t *info_size)
{
    uint16_t nvkey_id;
    uint32_t nvkey_size;
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    uint8_t *info_start_address = NULL;
    uint32_t info_total_size = 0;
    uint8_t *currnet_info_address;
    uint32_t process_size;
    uint32_t total_process_size;
    uint32_t channel_num;

    /* check if the all connection info in NVKEY have been load */
    if (wireless_mic_rx_audio_connection_info_all == NULL)
    {
        /* get the total connection info in the NVKEY */
        nvkey_id = NVID_DSP_ALG_CONNECTION_INFO_TABLE1;
        status = nvkey_data_item_length(nvkey_id, &nvkey_size);
        if (status != NVKEY_STATUS_OK)
        {
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]get connection info size error, %u\r\n", 1, status);
            AUDIO_ASSERT(0);
        }
        wireless_mic_rx_audio_connection_info_all_size = nvkey_size;
        wireless_mic_rx_audio_connection_info_all = malloc(nvkey_size);
        if (wireless_mic_rx_audio_connection_info_all == NULL)
        {
            AUDIO_ASSERT(0);
        }
        TRANSMITTER_LOG_I("[Wireless MIC RX] malloc connection info, address = 0x%x, size = %u", 2, wireless_mic_rx_audio_connection_info_all, wireless_mic_rx_audio_connection_info_all_size);
        status = nvkey_read_data(nvkey_id, wireless_mic_rx_audio_connection_info_all, &nvkey_size);
        if (status != NVKEY_STATUS_OK)
        {
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]get connection info error, %u\r\n", 1, status);
            AUDIO_ASSERT(0);
        }
    }

    /* get the required connection info based on mode */
    total_process_size = 0;
    while (total_process_size < wireless_mic_rx_audio_connection_info_all_size)
    {
        process_size = 0;
        currnet_info_address = wireless_mic_rx_audio_connection_info_all + total_process_size;
        if (*((uint16_t *)currnet_info_address) == mode)
        {
            /* find out the required connection info */
            info_start_address = currnet_info_address + sizeof(uint16_t);
            /* calculate the total info size */
            channel_num = *(uint16_t *)(currnet_info_address + sizeof(uint16_t));
            process_size = channel_num * sizeof(uint32_t) + sizeof(uint16_t);
            info_total_size = process_size;
            /* let break while loop */
            process_size = wireless_mic_rx_audio_connection_info_all_size - total_process_size;
        }
        else
        {
            /* switch the next index */
            channel_num = *(uint16_t *)(currnet_info_address + sizeof(uint16_t));
            process_size = channel_num * sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t);
        }
        total_process_size += process_size;
    }

    /* return info details */
    if (info_start_address == NULL)
    {
        TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR] can not find out info\r\n", 0);
        AUDIO_ASSERT(0);
    }
    *info = info_start_address;
    *info_size = info_total_size;
    TRANSMITTER_LOG_I("[Wireless MIC RX] connection info, address = 0x%x, size = %u, mode = %u", 3, info_start_address, info_total_size, mode);
}

void wireless_mic_rx_audio_connection_info_parse(uint8_t *info, uint32_t info_size)
{
    uint32_t process_size;
    uint32_t total_process_size;
    uint32_t bt_address_h;
    uint32_t bt_address_l;
    uint32_t channel_number;
    uint32_t i, j;
    uint32_t bt_link_index;
    uint32_t channel_status;

    /* reset temp conection status table */
    memset(&(wireless_mic_rx_channel_connection_status_temp[0][0]), 0, sizeof(wireless_mic_rx_channel_connection_status_temp));

    /* save connection status into temp table */
    total_process_size = 0;
    while(total_process_size < info_size)
    {
        /* reset state machine */
        process_size = 0;

        /* get bt link index based on BT address and channel number */
        bt_link_index = WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER;
        bt_address_l = *((uint32_t *)info);
        bt_address_h = *((uint16_t *)(info+sizeof(uint32_t)));
        channel_number = *((uint16_t *)(info+sizeof(uint32_t)+sizeof(uint16_t)));
        for (i = 0; i < WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER; i++)
        {
            if ((wireless_mic_rx_bt_link_setting[i].bt_address_h == bt_address_h) && (wireless_mic_rx_bt_link_setting[i].bt_address_l == bt_address_l))
            {
                bt_link_index = i;
                break;
            }
        }
        process_size += 8; // BT address is 6B length + channel number is 2B length

        if (bt_link_index < WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER)
        {
            /* the bt link is actived */
            if (channel_number > wireless_mic_rx_bt_link_setting[bt_link_index].channel_num)
            {
                TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]channel number is not right, %u, %u\r\n", 2, channel_number, wireless_mic_rx_bt_link_setting[bt_link_index].channel_num);
                AUDIO_ASSERT(0);
            }
            else
            {
                for (i = 0; i < channel_number; i++)
                {
                    channel_status = *((uint32_t *)(info+sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t)*i));
                    /* DAC output connections */
                    if ((channel_status & 0x1) != 0)
                    {
                        /* DAC output L */
                        wireless_mic_rx_channel_connection_status_temp[AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT][0] |= 0x1<<(bt_link_index*wireless_mic_rx_bt_link_setting[bt_link_index].channel_num+i);
                    }
                    if ((channel_status & 0x2) != 0)
                    {
                        /* DAC output R */
                        wireless_mic_rx_channel_connection_status_temp[AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT][1] |= 0x1<<(bt_link_index*wireless_mic_rx_bt_link_setting[bt_link_index].channel_num+i);
                    }
                    /* I2S_SLV 2 output connections */
                    if ((channel_status & 0x1000) != 0)
                    {
                        /* I2S_SLV 2 output L */
                        wireless_mic_rx_channel_connection_status_temp[AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0][0] |= 0x1<<(bt_link_index*wireless_mic_rx_bt_link_setting[bt_link_index].channel_num+i);
                    }
                    if ((channel_status & 0x2000) != 0)
                    {
                        /* I2S_SLV 2 output R */
                        wireless_mic_rx_channel_connection_status_temp[AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0][1] |= 0x1<<(bt_link_index*wireless_mic_rx_bt_link_setting[bt_link_index].channel_num+i);
                    }
                    /* USB 0 output connections */
                    if ((channel_status & 0x4000) != 0)
                    {
                        /* USB 0 output L */
                        wireless_mic_rx_channel_connection_status_temp[AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0][0] |= 0x1<<(bt_link_index*wireless_mic_rx_bt_link_setting[bt_link_index].channel_num+i);
                    }
                    if ((channel_status & 0x8000) != 0)
                    {
                        /* USB 0 output R */
                        wireless_mic_rx_channel_connection_status_temp[AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0][1] |= 0x1<<(bt_link_index*wireless_mic_rx_bt_link_setting[bt_link_index].channel_num+i);
                    }
                }
            }
        }
        else
        {
            /* the bt link is not actived */
            TRANSMITTER_LOG_E("[Wireless MIC RX][ERROR]This BT link is not actived, 0x%x, 0x%x\r\n", 2, bt_address_h, bt_address_l);
        }
        process_size += channel_number * sizeof(uint32_t);

        /* switch the next setting */
        total_process_size += process_size;
        info += process_size;
    }

    /* update connection status into the actual table */
    for (i = 0; i < AUDIO_TRANSMITTER_WIRELESS_MIC_RX_SUB_ID_MAX; i++)
    {
        for (j = 0; j < WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER; j++)
        {
            if (wireless_mic_rx_channel_connection_status_temp[i][j] != wireless_mic_rx_channel_connection_status[i][j])
            {
                wireless_mic_rx_channel_connection_status[i][j] = wireless_mic_rx_channel_connection_status_temp[i][j];
            }
        }
    }
}

#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
