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

/* Includes ------------------------------------------------------------------*/
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_ENABLE)

#include "audio_transmitter_playback_port.h"
#include "bt_sink_srv_ami.h"
#include "hal_audio_message_struct_common.h"
#include "hal_dvfs_internal.h"
#include "scenario_dongle_common.h"
#ifdef AIR_AUDIO_DUMP_ENABLE
#include "audio_dump.h"
#endif
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
#include "scenario_ble_audio.h"
#endif
#ifdef AIR_USB_AUDIO_ENABLE
#include "usbaudio_drv.h"
#endif

/* Private define ------------------------------------------------------------*/
#define DONGLE_AUDIO_COMMON_DL_PATH_DEBUG_LOG   (1)
#define DONGLE_AUDIO_COMMON_UL_PATH_DEBUG_LOG   (1)
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
uint32_t dl_stream_status = 0;
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
/* Public variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/****************************************************************************************************************************************************/
/*                                                         USB COMMON                                                                               */
/****************************************************************************************************************************************************/
audio_dongle_usb_handle_t audio_dongle_usb_rx_handle[AUDIO_DONGLE_USB_RX_PORT_TOTAL] = {0};
audio_dongle_usb_handle_t audio_dongle_usb_tx_handle[AUDIO_DONGLE_USB_TX_PORT_TOTAL] = {0};
#if AIR_AUDIO_DONGLE_DEBUG_LANTENCY
bool is_frist_dl_flag = true;
bool is_frist_ul_flag = true;

void usb_audio_dongle_rx_latency_debug_control(uint32_t port, bool enable, uint32_t gpio_num, uint32_t threshold);
void usb_audio_dongle_tx_latency_debug_control(uint32_t port, bool enable, uint32_t gpio_num, uint32_t threshold);

static void usb_audio_tx_cb_latency_debug(audio_dongle_usb_handle_t *handle, uint8_t *source_buf)
{
    int16_t *start_address = (int16_t *)source_buf;
    uint32_t current_level = 0;
    uint32_t i;
    int16_t sample_value = 0;
    uint16_t frame_samples;
    uint16_t channel_num;
    uint16_t resolution_size;

    if (is_frist_dl_flag == true) {
        is_frist_dl_flag = false;
        usb_audio_dongle_tx_latency_debug_control(0, 1, 4, 20000);
    }

    if (handle->latency_debug_enable)
    {
        if (handle->usb_param.pcm.channel_mode == 1)
        {
            channel_num = 1;
        }
        else
        {
            channel_num = 2;
        }
        resolution_size = 2;
        frame_samples = handle->frame_size / resolution_size / channel_num;
        for (i = 0; i < frame_samples; i++) {
            sample_value += (*(start_address + i*channel_num) / frame_samples);
        }

        if ((sample_value >= 0) && (sample_value >= handle->detect_threshold))
        {
            current_level = 1;
        }
        else if ((sample_value < 0) && (sample_value <= (-(int16_t)(handle->detect_threshold))))
        {
            current_level = 0;
        }

        TRANSMITTER_LOG_I("[Dongle Common][tx_Cb] latency debug monitor %d %d %d %d %d", 5, frame_samples, *start_address, sample_value, handle->detect_threshold, current_level);

        if (current_level != handle->latency_debug_last_level) {
            hal_gpio_set_output(handle->latency_debug_gpio_pin, current_level);
            handle->latency_debug_last_level = current_level;
        }
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void usb_audio_dongle_tx_latency_debug_control(uint32_t port, bool enable, uint32_t gpio_num, uint32_t threshold)
{
    uint32_t saved_mask;
    TRANSMITTER_LOG_I("[Dongle Common][tx_Cb] latency debug %d %d %d %d", 4, port, enable, gpio_num, threshold);
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (enable) {
        audio_dongle_usb_tx_handle[port].latency_debug_enable = 1;
        audio_dongle_usb_tx_handle[port].latency_debug_last_level = 0;
        audio_dongle_usb_tx_handle[port].latency_debug_last_sample = 0;
        audio_dongle_usb_tx_handle[port].latency_debug_gpio_pin = gpio_num;
        audio_dongle_usb_tx_handle[port].detect_threshold = threshold;
    } else {
        audio_dongle_usb_tx_handle[port].latency_debug_enable = 0;
        audio_dongle_usb_tx_handle[port].latency_debug_last_level = 0;
        audio_dongle_usb_tx_handle[port].latency_debug_last_sample = 0;
        audio_dongle_usb_tx_handle[port].latency_debug_gpio_pin = gpio_num;
        audio_dongle_usb_tx_handle[port].detect_threshold = 0;
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    hal_gpio_init(audio_dongle_usb_tx_handle[port].latency_debug_gpio_pin);
    hal_pinmux_set_function(audio_dongle_usb_tx_handle[port].latency_debug_gpio_pin, 0);
    hal_gpio_set_direction(audio_dongle_usb_tx_handle[port].latency_debug_gpio_pin, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(audio_dongle_usb_tx_handle[port].latency_debug_gpio_pin, HAL_GPIO_DATA_LOW);
}

static void usb_audio_rx_cb_latency_debug(audio_dongle_usb_handle_t *handle, uint8_t *source_buf)
{
    int16_t *start_address = NULL;
    uint32_t current_level = 0;
    uint32_t i;
    int16_t current_sample;
    int16_t next_sample;

    if (is_frist_ul_flag == true) {
        is_frist_ul_flag = false;
        usb_audio_dongle_rx_latency_debug_control(0, 1, 5, 10000);
    }

    if (handle->latency_debug_enable)
    {
        if (handle->usb_param.pcm.format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            current_level = handle->latency_debug_last_level;
            start_address = (int16_t *)source_buf;
            if ((*start_address > handle->latency_debug_last_sample) &&
                (*start_address - handle->latency_debug_last_sample > handle->detect_threshold))
            {
                current_level = 1;
            }
            else if ((*start_address < handle->latency_debug_last_sample) &&
                    (handle->latency_debug_last_sample - *start_address > handle->detect_threshold))
            {
                current_level = 0;
            }
            for (i = 0; i < handle->frame_size/(2*handle->usb_param.pcm.channel_mode)-1; i++)
            {
                current_sample  = *((int16_t *)(source_buf+i*(2*handle->usb_param.pcm.channel_mode)));
                next_sample     = *((int16_t *)(source_buf+(i+1)*(2*handle->usb_param.pcm.channel_mode)));
                if ((current_sample > next_sample) &&
                    (current_sample - next_sample > handle->detect_threshold))
                {
                    current_level = 0;
                    break;
                }
                else if ((current_sample < next_sample) &&
                        (next_sample - current_sample > handle->detect_threshold))
                {
                    current_level = 1;
                    break;
                }
                else
                {
                }
            }
            handle->latency_debug_last_sample = *((int16_t *)(source_buf + 2 * handle->usb_param.pcm.channel_mode * (handle->frame_size/(2*handle->usb_param.pcm.channel_mode)-1)));
            if (current_level != handle->latency_debug_last_level)
            {
                hal_gpio_set_output(handle->latency_debug_gpio_pin, current_level);
                handle->latency_debug_last_level = current_level;
            }
        }
        else if (handle->usb_param.pcm.format == HAL_AUDIO_PCM_FORMAT_S24_LE)
        {
            current_level = handle->latency_debug_last_level;
            start_address    = (int16_t *)(source_buf+1); // drop the low 8bit
            if ((*start_address > handle->latency_debug_last_sample) &&
                (*start_address - handle->latency_debug_last_sample > handle->detect_threshold))
            {
                current_level = 1;
            }
            else if ((*start_address < handle->latency_debug_last_sample) &&
                    (handle->latency_debug_last_sample - *start_address > handle->detect_threshold))
            {
                current_level = 0;
            }
            for (i = 0; i < handle->frame_size/(3*handle->usb_param.pcm.channel_mode)-1; i++)
            {
                current_sample  = *((int16_t *)(source_buf+i*(3*handle->usb_param.pcm.channel_mode)+1)); // drop the low 8bit
                next_sample     = *((int16_t *)(source_buf+(i+1)*(3*handle->usb_param.pcm.channel_mode)+1)); // drop the low 8bit
                if ((current_sample > next_sample) &&
                    (current_sample - next_sample) > handle->detect_threshold)
                {
                    current_level = 0;
                    break;
                }
                else if ((current_sample < next_sample) &&
                        (next_sample - current_sample > handle->detect_threshold))
                {
                    current_level = 1;
                    break;
                }
                else
                {
                }
            }
            handle->latency_debug_last_sample = *((int16_t *)(source_buf + 3 * handle->usb_param.pcm.channel_mode * (handle->frame_size/(3*handle->usb_param.pcm.channel_mode)-1) + 1));
            if (current_level != handle->latency_debug_last_level)
            {
                hal_gpio_set_output(handle->latency_debug_gpio_pin, current_level);
                handle->latency_debug_last_level = current_level;
            }
        }
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void usb_audio_dongle_rx_latency_debug_control(uint32_t port, bool enable, uint32_t gpio_num, uint32_t threshold)
{
    uint32_t saved_mask;
    TRANSMITTER_LOG_I("[Dongle Common][Rx_Cb] latency debug %d %d %d %d", 4, port, enable, gpio_num, threshold);
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (enable) {
        audio_dongle_usb_rx_handle[port].latency_debug_enable = 1;
        audio_dongle_usb_rx_handle[port].latency_debug_last_level = 0;
        audio_dongle_usb_rx_handle[port].latency_debug_last_sample = 0;
        audio_dongle_usb_rx_handle[port].latency_debug_gpio_pin = gpio_num;
        audio_dongle_usb_rx_handle[port].detect_threshold = threshold;
    } else {
        audio_dongle_usb_rx_handle[port].latency_debug_enable = 0;
        audio_dongle_usb_rx_handle[port].latency_debug_last_level = 0;
        audio_dongle_usb_rx_handle[port].latency_debug_last_sample = 0;
        audio_dongle_usb_rx_handle[port].latency_debug_gpio_pin = gpio_num;
        audio_dongle_usb_rx_handle[port].detect_threshold = 0;
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    hal_gpio_init(audio_dongle_usb_rx_handle[port].latency_debug_gpio_pin);
    hal_pinmux_set_function(audio_dongle_usb_rx_handle[port].latency_debug_gpio_pin, 0);
    hal_gpio_set_direction(audio_dongle_usb_rx_handle[port].latency_debug_gpio_pin, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(audio_dongle_usb_rx_handle[port].latency_debug_gpio_pin, HAL_GPIO_DATA_LOW);
}
#endif /* AIR_AUDIO_DONGLE_DEBUG_LANTENCY */
#if defined(AIR_USB_AUDIO_1_SPK_ENABLE) || defined(AIR_USB_AUDIO_2_SPK_ENABLE)
static const uint8_t all_zero_buffer[192] = {0}; // 48K 1ms 2CH
#endif
uint32_t ble_audio_codec_get_frame_size(audio_dsp_codec_type_t *codec_type, audio_codec_param_t *codec_param)
{
    uint32_t frame_size = 0;

    if (*codec_type != AUDIO_DSP_CODEC_TYPE_LC3) {
        AUDIO_ASSERT(0 && "Dongle codec_type is not LC3");
    }

    frame_size = codec_param->lc3.bit_rate * codec_param->lc3.frame_interval / 8 / 1000 / 1000;

    return frame_size;
}

uint8_t audio_dongle_get_usb_format_bytes(hal_audio_format_t pcm_format)
{
    uint8_t pcm_format_bytes = 0;
    if ((pcm_format < HAL_AUDIO_PCM_FORMAT_S16_LE) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S8)) {
        pcm_format_bytes = 1;
    } else if ((pcm_format < HAL_AUDIO_PCM_FORMAT_S24_LE) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S16_LE)) {
        pcm_format_bytes = 2;
    } else if ((pcm_format < HAL_AUDIO_PCM_FORMAT_S32_LE) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S24_LE)) {
        pcm_format_bytes = 3;
    } else if ((pcm_format < HAL_AUDIO_PCM_FORMAT_LAST) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S32_LE)) {
        pcm_format_bytes = 4;
    } else {
        AUDIO_ASSERT(0 && "hal error format");
    }
    return pcm_format_bytes;
}

uint8_t audio_dongle_get_format_bytes(hal_audio_format_t pcm_format)
{
    uint8_t pcm_format_bytes = 0;
    if ((pcm_format < HAL_AUDIO_PCM_FORMAT_S16_LE) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S8)) {
        pcm_format_bytes = 1;
    } else if ((pcm_format < HAL_AUDIO_PCM_FORMAT_S24_LE) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S16_LE)) {
        pcm_format_bytes = 2;
    } else if ((pcm_format < HAL_AUDIO_PCM_FORMAT_LAST) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S24_LE)) {
        pcm_format_bytes = 4;
    } else {
        AUDIO_ASSERT(0 && "hal error format");
    }
    return pcm_format_bytes;
}

uint32_t audio_dongle_get_codec_frame_size(audio_dsp_codec_type_t *codec_type, audio_codec_param_t *codec_param)
{
    uint32_t frame_size = 0;
    audio_dsp_codec_type_t type = *codec_type;
    switch (type) {
        case AUDIO_DSP_CODEC_TYPE_PCM:
            frame_size = codec_param->pcm.sample_rate * codec_param->pcm.frame_interval * audio_dongle_get_format_bytes(codec_param->pcm.format);
            break;
        case AUDIO_DSP_CODEC_TYPE_CVSD:
            frame_size = 120; // 120bytes pcm data/7.5ms
            break;
        case AUDIO_DSP_CODEC_TYPE_MSBC:
            frame_size = 60; //  60bytes codec output data/7.5ms
            break;
        case AUDIO_DSP_CODEC_TYPE_SBC:
            #ifdef AIR_BT_AUDIO_DONGLE_ENABLE
            {
                uint32_t channel = codec_param->sbc_enc.channel_mode == SBC_ENCODER_MONO ? 1 : 2;
                uint32_t sub_band = codec_param->sbc_enc.subband_num == SBC_ENCODER_SUBBAND_NUMBER_4 ? 4 : 8;
                uint32_t blocks  = (codec_param->sbc_enc.block_length + 1) * 4;
                frame_size += 4 + channel * sub_band / 2;
                // 0: mono 1: dual channel
                if ((codec_param->sbc_enc.channel_mode == SBC_ENCODER_MONO) || (codec_param->sbc_enc.channel_mode == SBC_ENCODER_DUAL_CHANNEL)) {
                    frame_size += blocks * channel * codec_param->sbc_enc.max_bit_pool / 8;
                    if (blocks * channel * codec_param->sbc_enc.max_bit_pool % 8) {
                        frame_size += 1;
                    }
                } else if (codec_param->sbc_enc.channel_mode == SBC_ENCODER_JOINT_STEREO) {
                    frame_size += (sub_band + blocks * codec_param->sbc_enc.max_bit_pool) / 8;
                    if ((sub_band + blocks * codec_param->sbc_enc.max_bit_pool) % 8) {
                        frame_size += 1;
                    }
                } else if (codec_param->sbc_enc.channel_mode == SBC_ENCODER_STEREO) {
                    frame_size += (blocks * codec_param->sbc_enc.max_bit_pool) / 8;
                    if ((blocks * codec_param->sbc_enc.max_bit_pool) % 8) {
                        frame_size += 1;
                    }
                } else {
                    AUDIO_ASSERT(0 && "error sbc enc channel mode");
                }
            }
            #endif /* AIR_BT_AUDIO_DONGLE_ENABLE */
            break;
        case AUDIO_DSP_CODEC_TYPE_LC3:
            frame_size = codec_param->lc3.bit_rate * codec_param->lc3.frame_interval / 8 / 1000 / 1000;
            break;
        #ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
            case AUDIO_DSP_CODEC_TYPE_LHDC:
                /* LHDC */
                frame_size = codec_param->lhdc.mtu_size; // 1 mtu size = 1 packet
                break;
        #endif
        default:
            TRANSMITTER_LOG_I("[Dongle Common] codec type is not support %d", 1, codec_type);
            AUDIO_ASSERT(0);
            break;

    }
    return frame_size;
}

uint32_t audio_dongle_get_usb_audio_frame_size(audio_dsp_codec_type_t *usb_type, audio_codec_param_t *usb_param)
{
    uint32_t frame_size = 0;
    uint32_t samples = 0;
    uint32_t channel_num = 0;
    uint32_t resolution_size = 0;

    if (*usb_type == AUDIO_DSP_CODEC_TYPE_PCM) {
        frame_size = 1;
        switch (usb_param->pcm.frame_interval) {
            case 1000:
                break;
            default:
                AUDIO_ASSERT(0);
                break;
        }
        switch (usb_param->pcm.sample_rate) {
            case 16000:
            case 32000:
            case 44100:
            case 48000:
            case 96000:
                samples = usb_param->pcm.sample_rate/1000;
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
            case 8:
                channel_num = 8;
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
#ifdef AIR_USB_AUDIO_ENABLE
#if defined(AIR_USB_AUDIO_1_SPK_ENABLE) || defined(AIR_USB_AUDIO_2_SPK_ENABLE)
static void audio_dongle_usb0_rx_cb_handle(uint8_t port)
{
    uint32_t gpt_count, duration_count;
    uint32_t available_data_size;
    n9_dsp_share_info_t *p_dsp_info;
    uint32_t buf_size;
    uint8_t *p_source_buf;

    /* get current gpt count */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    #if DONGLE_AUDIO_COMMON_DL_PATH_DEBUG_LOG
    //TRANSMITTER_LOG_I("[Dongle Common][Rx_Cb] port %d cur gpt = %u", 2, port, gpt_count);
    #endif /* DONGLE_AUDIO_COMMON_DL_PATH_DEBUG_LOG */

    if (audio_dongle_usb_rx_handle[port].dongle_stream_status == 0) {
        // We should do callback until the steam started.
        available_data_size = USB_Audio_Get_Len_Received_Data(port);
        USB_Audio_Rx_Buffer_Drop_Bytes(port, available_data_size);
        TRANSMITTER_LOG_I("[Dongle Common][Rx_Cb] stream is not ready", 0);
        return;
    }

    /* get share buffer info */
    p_dsp_info = audio_dongle_usb_rx_handle[port].p_dsp_info;
    /* check if is the first time */
    if (audio_dongle_usb_rx_handle[port].first_time == 0) {
        /* this is the first irq, we need to drop all relict USB data */
        available_data_size = USB_Audio_Get_Len_Received_Data(port);
        USB_Audio_Rx_Buffer_Drop_Bytes(port, available_data_size);
        audio_dongle_usb_rx_handle[port].first_time = 1;
        audio_dongle_usb_rx_handle[port].previous_gpt_count = gpt_count;
        TRANSMITTER_LOG_I("[Dongle Common][Rx_Cb] port %d first time = %u", 2, port, gpt_count);
        #if DONGLE_AUDIO_COMMON_DL_PATH_DEBUG_LOG
        hal_audio_buf_mgm_get_free_buffer(p_dsp_info, &p_source_buf, &buf_size);
        TRANSMITTER_LOG_I("[Dongle Common][Rx_Cb] port %d first time buffer free size %d", 2, port, buf_size);
        #endif
    } else {
        /* this is not the first irq, we need to copy usb into share buffer */
        hal_gpt_get_duration_count(audio_dongle_usb_rx_handle[port].previous_gpt_count, gpt_count, &duration_count);
        if ((duration_count > 1500) || (duration_count < 500)) {
            TRANSMITTER_LOG_E("[Dongle Common][ERROR][Rx_Cb] port %d duration = %d", 2, port, duration_count);
        }
        audio_dongle_usb_rx_handle[port].previous_gpt_count = gpt_count;

        /* get usb data size */
        available_data_size = USB_Audio_Get_Len_Received_Data(port);
        if ((available_data_size > audio_dongle_usb_rx_handle[port].frame_size) ||
            (available_data_size % audio_dongle_usb_rx_handle[port].frame_size)) {
            TRANSMITTER_LOG_E("[Dongle Common][ERROR][Rx_Cb] data in USB %d buffer is abnormal %d %d\r\n", 3,
                port,
                available_data_size,
                audio_dongle_usb_rx_handle[port].frame_size
                );
        }

        /* copy usb data into share buffer block one by one */
        while (available_data_size > 0) {
            /* get share buffer block info */
            hal_audio_buf_mgm_get_free_buffer(p_dsp_info, &p_source_buf, &buf_size);
            if (buf_size < (audio_dongle_usb_rx_handle[port].frame_size + BLK_HEADER_SIZE)) {
                TRANSMITTER_LOG_I("[Dongle Common][Rx_Cb]Not enough share buffer space in USB %d, %u", 2, port, buf_size);
                USB_Audio_Rx_Buffer_Drop_Bytes(port, audio_dongle_usb_rx_handle[port].frame_size);
                available_data_size = USB_Audio_Get_Len_Received_Data(port);
                continue;
            }

            /* update usb data header */
            audio_dongle_usb_rx_handle[port].usb_stream_header.sequence_number++;
            audio_dongle_usb_rx_handle[port].usb_stream_header.data_length = audio_dongle_usb_rx_handle[port].frame_size;

            /* write usb data into share buffer block */
            // memcpy(p_source_buf, &audio_dongle_usb_rx_handle[0].usb_stream_header, BLK_HEADER_SIZE);
            ((audio_transmitter_block_header_t *)p_source_buf)->sequence_number = audio_dongle_usb_rx_handle[port].usb_stream_header.sequence_number;
            ((audio_transmitter_block_header_t *)p_source_buf)->data_length     = audio_dongle_usb_rx_handle[port].usb_stream_header.data_length;
            USB_Audio_Read_Data(port, p_source_buf + BLK_HEADER_SIZE, audio_dongle_usb_rx_handle[port].frame_size);
            if (available_data_size % audio_dongle_usb_rx_handle[port].frame_size) {
                memset(p_source_buf + BLK_HEADER_SIZE, 0, audio_dongle_usb_rx_handle[port].frame_size);
            }
            hal_audio_buf_mgm_get_write_data_done(p_dsp_info, audio_dongle_usb_rx_handle[port].frame_size + BLK_HEADER_SIZE);

            #if DONGLE_AUDIO_COMMON_DL_PATH_DEBUG_LOG
            uint32_t current_timestamp;
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
            hal_gpt_get_duration_count(gpt_count, current_timestamp, &duration_count);
            TRANSMITTER_LOG_I("[Dongle Common][Rx_Cb] port %d r_offset = %u, w_offset = %u, process_time = %u, gpt = %u", 5,
                port,
                p_dsp_info->read_offset,
                p_dsp_info->write_offset,
                duration_count,
                gpt_count
                );
            #endif /* DONGLE_AUDIO_COMMON_DL_PATH_DEBUG_LOG */

            /* audio dump */
            // LOG_AUDIO_DUMP(p_source_buf + BLK_HEADER_SIZE, audio_dongle_usb_rx_handle[0].frame_size, SOURCE_IN4);

            /* get residual usb data size */
            available_data_size = USB_Audio_Get_Len_Received_Data(port);

            #if AIR_AUDIO_DONGLE_DEBUG_LANTENCY
            usb_audio_rx_cb_latency_debug(&audio_dongle_usb_rx_handle[port], (p_source_buf + BLK_HEADER_SIZE));
            #endif /* AIR_AUDIO_DONGLE_DEBUG_LANTENCY */
        }
    }
}
#endif /* defined(AIR_USB_AUDIO_1_SPK_ENABLE) || defined(AIR_USB_AUDIO_2_SPK_ENABLE) */

#if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
static void audio_dongle_usb0_tx_cb_handle(uint8_t port)
{
    uint32_t gpt_count, duration_count;
    uint32_t data_size;
    n9_dsp_share_info_t *p_dsp_info;
    uint32_t blk_size;
    uint32_t data_size_total;
    uint8_t *p_source_buf;

    /* get current gpt count */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    #if DONGLE_AUDIO_COMMON_UL_PATH_DEBUG_LOG
    //TRANSMITTER_LOG_I("[Dongle Common][Tx_Cb] usb_audio_tx_cb_ble_audio_dongle_0 callback = %u", 1, gpt_count);
    #endif /* DONGLE_AUDIO_COMMON_UL_PATH_DEBUG_LOG */

    if (audio_dongle_usb_tx_handle[port].dongle_stream_status == 0) {
        /* workaround: If the stream is not started, need to send zero data to usb to avoid usb host error */
        USB_Audio_TX_SendData(0, audio_dongle_usb_tx_handle[port].frame_size, (uint8_t *)all_zero_buffer);
        return;
    }

    /* get share buffer info */
    p_dsp_info = audio_dongle_usb_tx_handle[port].p_dsp_info;

    /* check usb irq duration */
    if (audio_dongle_usb_tx_handle[port].first_time == 0) {
        audio_dongle_usb_tx_handle[port].previous_gpt_count = gpt_count;
        audio_dongle_usb_tx_handle[port].first_time  = 1;
        TRANSMITTER_LOG_I("[Dongle Common][Tx_Cb]usb_audio_tx_cb_ble_audio_dongle_%d callback first time = %u", 2, port, gpt_count);
    } else {
        hal_gpt_get_duration_count(audio_dongle_usb_tx_handle[port].previous_gpt_count, gpt_count, &duration_count);
        if ((duration_count > 1500) || (duration_count < 500)) {
            TRANSMITTER_LOG_E("[Dongle Common][ERROR][Tx_Cb] usb_audio_tx_cb_ble_audio_dongle_%d duration = %d", 2, port, duration_count);
        }
        audio_dongle_usb_tx_handle[port].previous_gpt_count = gpt_count;
    }

    /* get data info */
    hal_audio_buf_mgm_get_data_buffer(p_dsp_info, &p_source_buf, &data_size_total);
    if (data_size_total == 0) {
        data_size = 0;
        p_source_buf = NULL;
    } else {
        data_size = ((audio_transmitter_block_header_t *)p_source_buf)->data_length;
        /* check data size */
        if (data_size != audio_dongle_usb_tx_handle[port].frame_size) {
            TRANSMITTER_LOG_E("[Dongle Common][ERROR][Tx_Cb] usb_audio_tx_cb_ble_audio_dongle_%d data_size is not right %u, %u", 3,
                port,
                data_size,
                audio_dongle_usb_tx_handle[port].frame_size);
            AUDIO_ASSERT(0);
        }
        p_source_buf += sizeof(audio_transmitter_block_header_t);
    }

    // /* check if needs to send ccni to trigger dsp flow */
    // if (data_size_total == (audio_dongle_usb_tx_handle[port].frame_size+sizeof(audio_transmitter_block_header_t))*DONGLE_USB_TX_SEND_CCNI_FRAMES) {
    //     /* send ccni to trigger dsp flow if there are only 3ms data in share buffer, so dsp has 2ms to process new data */
    //     usb_audio_tx_trigger_dsp_flow(gpt_count, DONGLE_USB_TX_SEND_CCNI_FRAMES);
    // } else if (data_size_total == 0) {
    //     /* send ccni to trigger dsp flow if there is no data in share buffer */
    //     usb_audio_tx_trigger_dsp_flow(gpt_count, 0);
    // }

    /* send usb data */
    if (data_size == 0) {
        if (audio_dongle_usb_tx_handle[port].stream_is_started == 0) {
            /* the stream is not started, so send slience data */
            USB_Audio_TX_SendData(0, audio_dongle_usb_tx_handle[port].frame_size, (uint8_t *)all_zero_buffer);
        } else {
            USB_Audio_TX_SendData(0, audio_dongle_usb_tx_handle[port].frame_size, (uint8_t *)all_zero_buffer);
            TRANSMITTER_LOG_E("[Dongle Common][ERROR][Tx_Cb] usb_audio_tx_cb_ble_audio_dongle_%d data is not enough", 1, port);
            // AUDIO_ASSERT(0);
        }
    } else {
        /* set data from share buffer into USB FIFO */
        USB_Audio_TX_SendData(0, audio_dongle_usb_tx_handle[port].frame_size, p_source_buf);
        /* drop this packet */
        blk_size = p_dsp_info->sub_info.block_info.block_size;
        hal_audio_buf_mgm_get_read_data_done(p_dsp_info, blk_size);

        if (audio_dongle_usb_tx_handle[port].stream_is_started == 0) {
            /* set that the stream is started */
            audio_dongle_usb_tx_handle[port].stream_is_started = 1;
        }
    }
    #if DONGLE_AUDIO_COMMON_UL_PATH_DEBUG_LOG
    uint32_t current_timestamp;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    hal_gpt_get_duration_count(gpt_count, current_timestamp, &duration_count);
    TRANSMITTER_LOG_I("[Dongle Common][Tx_Cb] port %d r_offset = %u, w_offset = %u, process_time = %u gpt = %u", 5,
        port,
        p_dsp_info->read_offset,
        p_dsp_info->write_offset,
        duration_count,
        gpt_count
        );
    #endif /* DONGLE_AUDIO_COMMON_UL_PATH_DEBUG_LOG */

    #if AIR_AUDIO_DONGLE_DEBUG_LANTENCY
    if (data_size == 0) {
        usb_audio_tx_cb_latency_debug(&audio_dongle_usb_tx_handle[port], (uint8_t *)all_zero_buffer);
    } else {
        usb_audio_tx_cb_latency_debug(&audio_dongle_usb_tx_handle[port], p_source_buf);
    }
    #endif /* AIR_AUDIO_DONGLE_DEBUG_LANTENCY */
}

#endif /* AIR_USB_AUDIO_1_MIC_ENABLE */

#if defined(AIR_USB_AUDIO_1_SPK_ENABLE)
void audio_dongle_usb0_rx_cb(void)
{
    audio_dongle_usb0_rx_cb_handle(0); // USB_0
}
#endif /* AIR_USB_AUDIO_1_SPK_ENABLE */

#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
void audio_dongle_usb1_rx_cb(void)
{
    audio_dongle_usb0_rx_cb_handle(1); // USB_1
}
#endif /* AIR_USB_AUDIO_1_SPK_ENABLE */

#if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
void audio_dongle_usb0_tx_cb(void)
{
    audio_dongle_usb0_tx_cb_handle(0); // USB_0
}
#endif /* AIR_USB_AUDIO_1_MIC_ENABLE */

#endif /* AIR_USB_AUDIO_ENABLE */

bool audio_dongle_write_data_to_usb(uint8_t usb_port, uint8_t *data, uint32_t *length)
{
    uint32_t gpt_count, duration_count;
    uint32_t data_size;
    n9_dsp_share_info_t *p_dsp_info;
    uint32_t blk_size;
    uint32_t data_size_total;
    uint8_t *p_source_buf;
    if (usb_port >= AUDIO_DONGLE_USB_TX_PORT_TOTAL) {
        AUDIO_ASSERT(0 && "[Dongle Common]read data from error usb port number");
        return false;
    }
    /* get current gpt count */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    #if ULL_AUDIO_V2_DONGLE_UL_PATH_DEBUG_LOG
    TRANSMITTER_LOG_I("[Dongle Common][UL]read data to usb callback = %u", 1, gpt_count);
    #endif /* ULL_AUDIO_V2_DONGLE_UL_PATH_DEBUG_LOG */

    if (audio_dongle_usb_tx_handle[usb_port].dongle_stream_status == 0) {
        /* workaround: If the stream is not started, need to send zero data to usb to avoid usb host error */
        memset(data, 0, *length);
        return true;;
    }

    /* check if length is right */
    if (audio_dongle_usb_tx_handle[usb_port].frame_size != *length) {
        TRANSMITTER_LOG_E("[Dongle Common][UL][ERROR] read data to usb length is not right %u, %u", 2, *length, audio_dongle_usb_tx_handle[usb_port].frame_size);
        AUDIO_ASSERT(0);
    }

    /* get share buffer info */
    p_dsp_info = audio_dongle_usb_tx_handle[usb_port].p_dsp_info;

    /* check usb irq duration */
    if (audio_dongle_usb_tx_handle[usb_port].first_time == 0) {
        audio_dongle_usb_tx_handle[usb_port].previous_gpt_count = gpt_count;
        audio_dongle_usb_tx_handle[usb_port].first_time  = 1;

        TRANSMITTER_LOG_I("[Dongle Common][UL][USB_TX_DEBUG 0]read data to usb callback first time = %u", 1, gpt_count);
    } else {
        hal_gpt_get_duration_count(audio_dongle_usb_tx_handle[usb_port].previous_gpt_count, gpt_count, &duration_count);
        if ((duration_count > 1500) || (duration_count < 500)) {
            TRANSMITTER_LOG_E("[Dongle Common][UL][ERROR][USB_TX_DEBUG 0]read data to usb duration = %d", 1, duration_count);
        }
        audio_dongle_usb_tx_handle[usb_port].previous_gpt_count = gpt_count;
    }

    /* get data info */
    hal_audio_buf_mgm_get_data_buffer(p_dsp_info, &p_source_buf, &data_size_total);
    if (data_size_total == 0) {
        data_size = 0;
        p_source_buf = NULL;
    } else {
        data_size = ((audio_transmitter_block_header_t *)p_source_buf)->data_length;
        /* check data size */
        if (data_size != audio_dongle_usb_tx_handle[usb_port].frame_size) {
            TRANSMITTER_LOG_E("[Dongle Common][UL][ERROR]read data to usb data_size is not right %u, %u", 2, data_size, audio_dongle_usb_tx_handle[usb_port].frame_size);
            AUDIO_ASSERT(0);
        }
        p_source_buf += sizeof(audio_transmitter_block_header_t);
    }

    /* check if needs to send ccni to trigger dsp flow */
    // if (data_size_total == (audio_dongle_usb_tx_handle[usb_port].frame_size+sizeof(audio_transmitter_block_header_t))*DONGLE_USB_TX_SEND_CCNI_FRAMES)
    // {
    //     /* send ccni to trigger dsp flow if there are only 3ms data in share buffer, so dsp has 2ms to process new data */
    //     usb_audio_tx_trigger_dsp_flow(gpt_count, DONGLE_USB_TX_SEND_CCNI_FRAMES);
    // }
    // else if (data_size_total == 0)
    // {
    //     /* send ccni to trigger dsp flow if there is no data in share buffer */
    //     usb_audio_tx_trigger_dsp_flow(gpt_count, 0);
    // }

    /* send usb data */
    if (data_size == 0) {
        if (audio_dongle_usb_tx_handle[usb_port].stream_is_started == 0) {
            /* the stream is not started, so send slience data */
            memset(data, 0, *length);
        } else {
            memset(data, 0, *length);
            TRANSMITTER_LOG_E("[Dongle Common][UL][ERROR]read data to usb data is not enough", 0);
            // AUDIO_ASSERT(0);
        }
    } else {
        /* set data from share buffer into USB FIFO */
        memcpy(data, p_source_buf, audio_dongle_usb_tx_handle[usb_port].frame_size);

        /* drop this packet */
        blk_size = p_dsp_info->sub_info.block_info.block_size;
        hal_audio_buf_mgm_get_read_data_done(p_dsp_info, blk_size);

        if (audio_dongle_usb_tx_handle[usb_port].stream_is_started == 0) {
            /* set that the stream is started */
            audio_dongle_usb_tx_handle[usb_port].stream_is_started = 1;
        }
    }

    #if AIR_AUDIO_DONGLE_DEBUG_LANTENCY
    if (data_size == 0) {
        usb_audio_tx_cb_latency_debug(&audio_dongle_usb_tx_handle[usb_port], (uint8_t *)all_zero_buffer);
    } else {
        usb_audio_tx_cb_latency_debug(&audio_dongle_usb_tx_handle[usb_port], p_source_buf);
    }
    #endif /* AIR_AUDIO_DONGLE_DEBUG_LANTENCY */

    return true;
}

bool audio_dongle_read_data_from_usb(uint8_t usb_port, uint8_t *data, uint32_t *length)
{
    uint32_t gpt_count, duration_count;
    uint32_t available_data_size;
    n9_dsp_share_info_t *p_dsp_info;
    uint32_t buf_size;
    uint8_t *p_source_buf;

    /* get current gpt count */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    #if DONGLE_AUDIO_COMMON_DL_PATH_DEBUG_LOG
    TRANSMITTER_LOG_I("[Dongle Common][DL][DEBUG]read data from usb cur gpt = %u", 1, gpt_count);
    #endif /* DONGLE_AUDIO_COMMON_DL_PATH_DEBUG_LOG */

    /* get share buffer info */
    p_dsp_info = audio_dongle_usb_rx_handle[usb_port].p_dsp_info;

    /* check if is the first time */
    if (audio_dongle_usb_rx_handle[usb_port].first_time == 0)
    {
        /* this is the first irq, we need to drop all relict USB data */
        audio_dongle_usb_rx_handle[usb_port].first_time = 1;
        audio_dongle_usb_rx_handle[usb_port].previous_gpt_count = gpt_count;

        TRANSMITTER_LOG_I("[Dongle Common][DL]read data from usb first time = %u", 1, gpt_count);
    }
    else
    {
        /* this is not the first irq, we need to copy usb into share buffer */
        hal_gpt_get_duration_count(audio_dongle_usb_rx_handle[usb_port].previous_gpt_count, gpt_count, &duration_count);
        if ((duration_count > 1500) || (duration_count < 500))
        {
            TRANSMITTER_LOG_E("[Dongle Common][DL][ERROR]read data from usb duration = %d", 1, duration_count);
        }
        audio_dongle_usb_rx_handle[usb_port].previous_gpt_count = gpt_count;

        /* get usb data size */
        available_data_size = *length;
        if (available_data_size != audio_dongle_usb_rx_handle[usb_port].frame_size)
        {
            TRANSMITTER_LOG_E("[Dongle Common][DL][ERROR]read data from usb data size is not right %d, %d\r\n", 2, available_data_size, audio_dongle_usb_rx_handle[usb_port].frame_size);
            AUDIO_ASSERT(0);
        }

        /* copy usb data into share buffer block one by one */
        while (available_data_size >= audio_dongle_usb_rx_handle[usb_port].frame_size)
        {
            /* get share buffer block info */
            hal_audio_buf_mgm_get_free_buffer(p_dsp_info, &p_source_buf, &buf_size);
            if (buf_size < (audio_dongle_usb_rx_handle[usb_port].frame_size + BLK_HEADER_SIZE))
            {
                TRANSMITTER_LOG_I("[Dongle Common][DL][ERROR]read data from usbNot enough share buffer space, %u", 1, buf_size);
                available_data_size = available_data_size-audio_dongle_usb_rx_handle[usb_port].frame_size;
                data = data + audio_dongle_usb_rx_handle[usb_port].frame_size;
                continue;
            }

            /* update usb data header */
            audio_dongle_usb_rx_handle[usb_port].usb_stream_header.sequence_number++;
            audio_dongle_usb_rx_handle[usb_port].usb_stream_header.data_length = audio_dongle_usb_rx_handle[usb_port].frame_size;

            /* write usb data into share buffer block */
            ((audio_transmitter_block_header_t *)p_source_buf)->sequence_number = audio_dongle_usb_rx_handle[usb_port].usb_stream_header.sequence_number;
            ((audio_transmitter_block_header_t *)p_source_buf)->data_length     = audio_dongle_usb_rx_handle[usb_port].usb_stream_header.data_length;
            memcpy(p_source_buf + BLK_HEADER_SIZE, data, audio_dongle_usb_rx_handle[usb_port].frame_size);
            hal_audio_buf_mgm_get_write_data_done(p_dsp_info, audio_dongle_usb_rx_handle[usb_port].frame_size + BLK_HEADER_SIZE);

            #if DONGLE_AUDIO_COMMON_DL_PATH_DEBUG_LOG
            TRANSMITTER_LOG_I("[Dongle Common][DL][DEBUG]read data from usb r_offset = %u, w_offset = %u", 2, p_dsp_info->read_offset, p_dsp_info->write_offset);
            #endif /* DONGLE_AUDIO_COMMON_DL_PATH_DEBUG_LOG */

            /* audio dump */
            // LOG_AUDIO_DUMP(p_source_buf + BLK_HEADER_SIZE, audio_dongle_usb_rx_handle[usb_port].frame_size, SOURCE_IN4);

            /* get residual usb data size */
            available_data_size = available_data_size-audio_dongle_usb_rx_handle[usb_port].frame_size;
            data = data + audio_dongle_usb_rx_handle[usb_port].frame_size;

            #if AIR_AUDIO_DONGLE_DEBUG_LANTENCY
            usb_audio_rx_cb_latency_debug(&audio_dongle_usb_rx_handle[usb_port], (p_source_buf + BLK_HEADER_SIZE));
            #endif /* AIR_AUDIO_DONGLE_DEBUG_LANTENCY */
        }
    }

    return true;
}

/****************************************************************************************************************************************************/
/*                                                      AFE IN COMMON                                                                               */
/****************************************************************************************************************************************************/
#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE || defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
uint32_t g_dongle_i2s_in_default_d_gain = 0;
#endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE)
extern void bt_sink_srv_am_set_volume(bt_sink_srv_am_stream_type_t in_out, bt_sink_srv_audio_setting_vol_info_t *vol_info);
uint32_t g_dongle_line_in_default_d_gain = 0;
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
static void bt_audio_dongle_config_i2s_in_parameter(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param);
static void bt_audio_dongle_config_adc_in_parameter(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param);
#endif
#if defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
static void ull_audio_v2_config_adc_in_parameter(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    /* ULL 2.0 Line in parameters */
    open_param->stream_in_param.afe.format = HAL_AUDIO_PCM_FORMAT_U32_LE;
    #ifdef AIR_BTA_IC_PREMIUM_G2
        open_param->stream_in_param.afe.sampling_rate = config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param.link_param[0].codec_param.lc3plus.sample_rate;  // G2 -> 48K
        open_param->stream_in_param.afe.frame_size    = open_param->stream_in_param.afe.stream_out_sampling_rate * 5 / 1000; // period 5ms
    #else
        if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL) {
            open_param->stream_in_param.afe.sampling_rate = 48000;  // G3 -> 96K (line in no support 96k)
            open_param->stream_in_param.afe.frame_size    = 240; // period 5ms
        } else {
            open_param->stream_in_param.afe.sampling_rate = config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param.link_param[0].codec_param.lc3plus.sample_rate;  // G2 -> 48K
            open_param->stream_in_param.afe.frame_size    = open_param->stream_in_param.afe.stream_out_sampling_rate * 5 / 1000; // period 5ms
        }
    #endif
    open_param->stream_in_param.afe.frame_number = 4;
    open_param->stream_in_param.afe.irq_period   = 0;
    open_param->stream_in_param.afe.hw_gain      = false;
    #ifdef AIR_AUDIO_WIRED_MIXING_ENABLE
    #ifdef AIR_AUDIO_DONGLE_MIXING_WIRED_ENABLE
    wired_audio_config_sync_mixing_parameter_to_dongle(config, open_param);
    #endif /* AIR_AUDIO_DONGLE_MIXING_WIRED_ENABLE */
    #endif /* AIR_AUDIO_WIRED_MIXING_ENABLE */
}
#endif
#if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
static void ull_audio_v2_config_i2s_in_parameter(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    (void)(config);
    open_param->stream_in_param.afe.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
    /* hwsrc input rate */
    open_param->stream_in_param.afe.sampling_rate = config->scenario_config.ull_audio_v2_dongle_config.dl_config.source_param.i2s_mst_in_param.codec_param.pcm.sample_rate;
    /* hwsrc output rate */
    #if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_TRACKING_VDMA_MODE_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_TRACKING_MEMIF_MODE_ENABLE)
        /* tracking mode */
        if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
            if (open_param->stream_in_param.afe.sampling_rate == 0) {
                open_param->stream_in_param.afe.sampling_rate = 48000;
            }
            open_param->stream_in_param.afe.stream_out_sampling_rate = config->scenario_config.ull_audio_v2_dongle_config.dl_config.sink_param.bt_out_param.link_param[0].codec_param.lc3plus.sample_rate;
            open_param->stream_in_param.afe.frame_size = open_param->stream_in_param.afe.stream_out_sampling_rate * 5 / 1000; // period 5ms
        } else {
            /* there is no need to support tracking mode in i2s master in */
            open_param->stream_in_param.afe.frame_size = open_param->stream_in_param.afe.sampling_rate * 5 / 1000; // period 5ms
        }
    #else
        /* In non-tracking mode, ull2.0 only support 48K/96K input media data */
        open_param->stream_in_param.afe.frame_size = open_param->stream_in_param.afe.sampling_rate * 5 / 1000; // period 5ms
    #endif
    open_param->stream_in_param.afe.frame_number        = 4;
    open_param->stream_in_param.afe.irq_period          = 0;
    open_param->stream_in_param.afe.hw_gain             = false;
    if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
        open_param->stream_in_param.afe.audio_path_input_rate    = open_param->stream_in_param.afe.sampling_rate;
        //open_param->stream_in_param.afe.audio_path_output_rate   = open_param->stream_in_param.afe.sampling_rate;
    }
    // // I2S setting
    // open_param->stream_in_param.afe.i2s_format          = config->scenario_config.ull_audio_v2_dongle_config.dl_config.source_param.i2s_mst_in_param.i2s_fromat;
    // open_param->stream_in_param.afe.i2s_word_length     = config->scenario_config.ull_audio_v2_dongle_config.dl_config.source_param.i2s_mst_in_param.i2s_word_length;
    #ifdef AIR_AUDIO_WIRED_MIXING_ENABLE
    #ifdef AIR_AUDIO_DONGLE_MIXING_WIRED_ENABLE
    wired_audio_config_sync_mixing_parameter_to_dongle(config, open_param);
    #endif /* AIR_AUDIO_DONGLE_MIXING_WIRED_ENABLE */
    #endif /* AIR_AUDIO_WIRED_MIXING_ENABLE */
}
#endif /* ullv2 afe in */
void audio_dongle_config_line_in_parameter(uint32_t id, audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    hal_audio_get_stream_in_setting_config(AU_DSP_LINEIN, &open_param->stream_in_param);
    open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM1;
    switch (id) {
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        case (AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE << 16 | AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN):
            /* ULL 2.0 Line in parameters */
            ull_audio_v2_config_adc_in_parameter(config, open_param);
            break;
#endif /* AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE */
#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
        case (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN):
            /* LE Dongle Line in parameters */
            open_param->stream_in_param.afe.format        = HAL_AUDIO_PCM_FORMAT_S16_LE;
            open_param->stream_in_param.afe.sampling_rate = 48000; // le music mode: 48k-10ms-16bit
            open_param->stream_in_param.afe.frame_size    = open_param->stream_in_param.afe.sampling_rate *
                                                         config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.frame_interval / 1000 / 1000;;
            open_param->stream_in_param.afe.frame_number  = 4;
            open_param->stream_in_param.afe.irq_period    = 0;
            open_param->stream_in_param.afe.hw_gain       = false;
            break;
#endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0) :
        case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0) :
            bt_audio_dongle_config_adc_in_parameter(config, open_param);
            break;
#endif
        default:
            TRANSMITTER_LOG_E("[Dongle Common] ERROR: line in is not support in id %d, please check the func !", 1, id);
            assert(0);
            break;
    }
    /* Just use line in volume to align ADC Gain with config tool */
    bt_sink_srv_audio_setting_vol_info_t vol_info;
    memset(&vol_info, 0, sizeof(bt_sink_srv_audio_setting_vol_info_t));
    vol_info.type = VOL_LINE_IN;
    vol_info.vol_info.lineIN_vol_info.dev_out = HAL_AUDIO_DEVICE_HEADSET;
    vol_info.vol_info.lineIN_vol_info.dev_in  = HAL_AUDIO_DEVICE_MAIN_MIC;
    vol_info.vol_info.lineIN_vol_info.lev_in  = 15; // not care digital gain, only need analog gain
    bt_sink_srv_am_set_volume(STREAM_IN,  &vol_info);
    g_dongle_line_in_default_d_gain = audio_get_gain_in_in_dB(15, GAIN_DIGITAL, VOL_LINE_IN);
}
#endif /* le + ull2.0 line in */

#if defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
void audio_dongle_config_i2s_in_parameter(uint32_t id, audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &open_param->stream_in_param);
    open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM1;
    if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
        #if defined AIR_BTA_IC_PREMIUM_G2 || defined AIR_BTA_IC_STEREO_HIGH_G3
            clock_mux_sel(CLK_AUD_GPSRC_SEL, 2); // boost hwsrc convert speed
        #endif
    }
    switch (id) {
#if defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
        case (AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE << 16 | AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0):
        case (AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE << 16 | AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0):
            ull_audio_v2_config_i2s_in_parameter(config, open_param);
            break;
#endif /* ULLV2 I2S IN */

#if defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
        case (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) :
            open_param->stream_in_param.afe.format = HAL_AUDIO_PCM_FORMAT_S16_LE;
            #ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_TRACKING_VDMA_MODE_ENABLE
                if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
                    // i2s slave in vdma driver is default configured with 32-bit.
                    open_param->stream_in_param.afe.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
                }
            #endif
            if ((open_param->stream_in_param.afe.audio_device & HAL_AUDIO_DEVICE_MAIN_MIC_DUAL) ||
                (open_param->stream_in_param.afe.audio_device & HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL) ||
                (open_param->stream_in_param.afe.audio_device & HAL_AUDIO_DEVICE_DIGITAL_MIC_DUAL)) {
                // ADC d gain
                g_dongle_i2s_in_default_d_gain = 4800; // 48dB
            }
            open_param->stream_in_param.afe.frame_number        = 4;
            open_param->stream_in_param.afe.irq_period          = 0;
            open_param->stream_in_param.afe.hw_gain             = false;
            /* hwsrc input rate */
            open_param->stream_in_param.afe.sampling_rate       = 48000;
            /* hwsrc output rate */
            open_param->stream_in_param.afe.stream_out_sampling_rate = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.sample_rate;
            open_param->stream_in_param.afe.frame_size = open_param->stream_in_param.afe.sampling_rate *
                                                         config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.frame_interval / 1000 / 1000;
            break;
#endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1) :
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1) :
                bt_audio_dongle_config_i2s_in_parameter(config, open_param);
#endif
        default:
            break;
    }
}
#endif /* le + ull2.0 i2s in */

#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
// parameter setting
static void bt_audio_dongle_config_i2s_in_parameter(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    (void)(config);
    // tracking mode only support 32bit (sw limitation)
    bt_audio_dongle_dl_sink_info_t *sink_info = &(open_param->stream_out_param.bt_common.scenario_param.bt_audio_dongle_param.dl_info.sink);
    open_param->stream_in_param.afe.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
    /* hwsrc input rate for i2s slave tracking mode */
    open_param->stream_in_param.afe.sampling_rate = 48000;
    /* hwsrc output rate for i2s slave tracking mode */
    open_param->stream_in_param.afe.stream_out_sampling_rate = sink_info->bt_out.sample_rate;
    if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
        open_param->stream_in_param.afe.audio_path_input_rate = open_param->stream_in_param.afe.sampling_rate;
    }
    open_param->stream_in_param.afe.frame_size = sink_info->bt_out.frame_interval * open_param->stream_in_param.afe.stream_out_sampling_rate / 1000 / 1000;
    if (sink_info->bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
        open_param->stream_in_param.afe.frame_number = 3;
    } else if ((sink_info->bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) || (sink_info->bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_CVSD)) {
        open_param->stream_in_param.afe.frame_number = 3;
    }
}

static void bt_audio_dongle_config_adc_in_parameter(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    (void)(config);
    bt_audio_dongle_dl_sink_info_t *sink_info = &(open_param->stream_out_param.bt_common.scenario_param.bt_audio_dongle_param.dl_info.sink);
    open_param->stream_in_param.afe.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
    open_param->stream_in_param.afe.sampling_rate = 48000;
    open_param->stream_in_param.afe.stream_out_sampling_rate = sink_info->bt_out.sample_rate;
    open_param->stream_in_param.afe.frame_size = sink_info->bt_out.frame_interval * open_param->stream_in_param.afe.stream_out_sampling_rate / 1000 / 1000;
    if (sink_info->bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_SBC) {
        open_param->stream_in_param.afe.frame_number = (sink_info->bt_out.frame_interval / (sink_info->bt_out.frame_samples * 1000 * 10 / sink_info->bt_out.sample_rate) + 1) / 100 * 2;
    } else if ((sink_info->bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) || (sink_info->bt_out.codec_type == AUDIO_DSP_CODEC_TYPE_CVSD)) {
        open_param->stream_in_param.afe.frame_number = 4 * (sink_info->bt_out.frame_interval / 7500);
    }
    /* Just use line in volume to align ADC Gain with config tool */
    // bt_sink_srv_audio_setting_vol_info_t vol_info;
    // memset(&vol_info, 0, sizeof(bt_sink_srv_audio_setting_vol_info_t));
    // vol_info.type = VOL_LINE_IN;
    // vol_info.vol_info.lineIN_vol_info.dev_out = HAL_AUDIO_DEVICE_HEADSET;
    // vol_info.vol_info.lineIN_vol_info.dev_in  = HAL_AUDIO_DEVICE_MAIN_MIC;
    // vol_info.vol_info.lineIN_vol_info.lev_in  = 15; // not care digital gain, only need analog gain
    // bt_sink_srv_am_set_volume(STREAM_IN,  &vol_info);
    // g_dongle_line_in_default_d_gain = audio_get_gain_in_in_dB(15, GAIN_DIGITAL, VOL_LINE_IN);
}
#endif /* AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE || AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE */

#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
static void audio_dongle_config_analog_gain(void)
{
    bt_sink_srv_audio_setting_vol_info_t vol_info;
    /* Just use line in volume to align ADC Gain with config tool */
    memset(&vol_info, 0, sizeof(bt_sink_srv_audio_setting_vol_info_t));
    vol_info.type = VOL_LINE_IN;
    vol_info.vol_info.lineIN_vol_info.dev_out = HAL_AUDIO_DEVICE_HEADSET;
    vol_info.vol_info.lineIN_vol_info.dev_in  = HAL_AUDIO_DEVICE_MAIN_MIC;
    vol_info.vol_info.lineIN_vol_info.lev_in  = 15; // not care digital gain, only need analog gain
    bt_sink_srv_am_set_volume(STREAM_IN,  &vol_info);
    g_dongle_line_in_default_d_gain = audio_get_gain_in_in_dB(15, GAIN_DIGITAL, VOL_LINE_IN);
}

static void audio_dongle_config_afe_in_parameter(uint32_t id, audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    hal_audio_get_stream_in_setting_config(AU_DSP_RECORD, &open_param->stream_in_param);
    open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM_SUB;
    open_param->stream_in_param.afe.irq_period = 0;
    open_param->stream_in_param.afe.hw_gain    = false;
    switch (id) {
#if defined AIR_GAMING_MODE_DONGLE_V2_AFE_IN_ENABLE
        case (AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE << 16 | AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_AFE_IN_0):
            if ((open_param->stream_in_param.afe.audio_device & (HAL_AUDIO_DEVICE_I2S_MASTER |
                                                                 HAL_AUDIO_DEVICE_I2S_MASTER_L |
                                                                 HAL_AUDIO_DEVICE_I2S_MASTER_R |
                                                                 HAL_AUDIO_DEVICE_I2S_SLAVE))) {
                ull_audio_v2_config_i2s_in_parameter(config, open_param);
            } else {
                ull_audio_v2_config_adc_in_parameter(config, open_param);
                audio_dongle_config_analog_gain();
            }
            break;
#endif /* AIR_GAMING_MODE_DONGLE_V2_AFE_IN_ENABLE */
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2) :
        case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2) :
            if ((open_param->stream_in_param.afe.audio_device & (HAL_AUDIO_DEVICE_I2S_MASTER |
                                                                 HAL_AUDIO_DEVICE_I2S_MASTER_L |
                                                                 HAL_AUDIO_DEVICE_I2S_MASTER_R |
                                                                 HAL_AUDIO_DEVICE_I2S_SLAVE))) {
                bt_audio_dongle_config_i2s_in_parameter(config, open_param);
            } else {
                bt_audio_dongle_config_adc_in_parameter(config, open_param);
                audio_dongle_config_analog_gain();
            }
            break;
#endif
        default:
            break;
    }
    return;
}
#endif /* BT Audio AFE IN */
void audio_dongle_set_stream_in_afe(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    open_param->param.stream_in = STREAM_IN_AFE;
    open_param->stream_in_param.afe.stream_channel = HAL_AUDIO_DIRECT;
    uint32_t id = (config->scenario_type << 16) | config->scenario_sub_id;
    switch (id) {
        /* line in case */
        #if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
            case (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) :
                audio_dongle_config_line_in_parameter(id, config, open_param);
                break;
        #endif
        #if defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
            case (AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE << 16 | AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) :
                audio_dongle_config_line_in_parameter(id, config, open_param);
                break;
        #endif
        /* i2s in case */
        #if defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
            case (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) :
                audio_dongle_config_i2s_in_parameter(id, config, open_param);
                break;
        #endif
        #if defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
            case (AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE << 16 | AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) :
                audio_dongle_config_i2s_in_parameter(id, config, open_param);
                break;
        #endif
        #if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
            case (AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE << 16 | AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0) :
                audio_dongle_config_i2s_in_parameter(id, config, open_param);
                break;
        #endif
        #if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0) :
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0) :
                audio_dongle_config_line_in_parameter(id, config, open_param);
                break;
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1) :
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1) :
                audio_dongle_config_i2s_in_parameter(id, config, open_param);
                break;
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2) :
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2) :
                audio_dongle_config_afe_in_parameter(id, config, open_param);
                break;
        #endif
        default:
            TRANSMITTER_LOG_E("[Dongle Common] ERROR: id %d is not support, please check the func !", 1, id);
            assert(0);
            break;
    }
}

void audio_dongle_set_start_avm_config(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param)
{
    uint32_t id = (config->scenario_type << 16) | config->scenario_sub_id;
    switch (id) {
        #if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
            case (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) :
                goto DL_CASE;
                break;
        #endif
        #if defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
            case (AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE << 16 | AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) :
                goto DL_CASE;
                break;
        #endif
        #if defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
            case (AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) :
                goto DL_CASE;
                break;
        #endif
        #if defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
            case (AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE << 16 | AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) :
                goto DL_CASE;
                break;
        #endif
        #if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
            case (AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE << 16 | AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0) :
                goto DL_CASE;
                break;
        #endif
        #if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0) :
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1) :
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2) :
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0) :
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1) :
            case (AUDIO_TRANSMITTER_BT_AUDIO_DONGLE << 16 | AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2) :
                goto DL_CASE;
            break;
        #endif
        default:
            TRANSMITTER_LOG_E("[Dongle Common] ERROR: id %d is not support, please check the func !", 1, id);
            assert(0);
            break;
    }
DL_CASE:
    /* sync start */
    start_param->param.stream_in              = STREAM_IN_AFE;
    start_param->param.stream_out             = STREAM_OUT_BT_COMMON;
    start_param->stream_in_param.afe.aws_flag = true;
    return;
// UL_CASE:
//     /* TBD: UL case */
//     return;
}
#endif /* All Afe in Type */
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
void audio_dongle_set_stream_out_bt_common(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    /* get codec frame size */
    uint32_t payload_size = ble_audio_codec_get_frame_size(&(config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_type),
                                                    &(config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param));
    if ((payload_size == 0) || (payload_size != config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.frame_size)) {
        TRANSMITTER_LOG_E("[BLE Audio Dongle] ERROR: id [%d]-[%d] codec frame size %d, %d\r\n", 4,
            config->scenario_type,
            config->scenario_sub_id,
            payload_size,
            config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.frame_size);
        AUDIO_ASSERT(0);
    }
    open_param->param.stream_out = STREAM_OUT_BT_COMMON;
    open_param->stream_out_param.bt_common.scenario_type = config->scenario_type;
    open_param->stream_out_param.bt_common.scenario_sub_id = config->scenario_sub_id;
    open_param->stream_out_param.bt_common.share_info_type = SHARE_BUFFER_INFO_TYPE;
    open_param->stream_out_param.bt_common.data_notification_frequency = 1;
    open_param->stream_out_param.bt_common.max_payload_size = payload_size + sizeof(LE_AUDIO_HEADER);
    open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.period                         = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.period;
    open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.channel_enable                 = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.channel_enable;
    open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.codec_type                     = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_type;//AUDIO_DSP_CODEC_TYPE_LC3
    memcpy(&(open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3), &(config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3), sizeof(audio_codec_lc3_t));
    /* upper layer will prepare channel 2's share memory */
    open_param->stream_out_param.bt_common.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_SEND_TO_AIR_1);
    // if (dl_stream_status == 0)
    // {
    //     memset((((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->start_addr), 0, BT_AVM_SHARE_BUFFER_SIZE);
    // }
    // ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->read_offset         = 0;
    // ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->write_offset        = 0;
    // ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->sub_info.block_size = (payload_size+sizeof(LE_AUDIO_HEADER)+3)/4*4; //4B align
    // ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->sub_info.block_num  = BT_AVM_SHARE_BUFFER_SIZE / (((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->sub_info.block_size);
    open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_2 = (uint8_t *)open_param->stream_out_param.bt_common.p_share_info;
    /* upper layer will prepare channel 1's share memory */
    open_param->stream_out_param.bt_common.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_SEND_TO_AIR_0);
    // if (dl_stream_status == 0)
    // {
    //     memset((((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->start_addr), 0, BT_AVM_SHARE_BUFFER_SIZE);
    // }
    // ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->read_offset         = 0;
    // ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->write_offset        = 0;
    // ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->sub_info.block_size = (payload_size+sizeof(LE_AUDIO_HEADER)+3)/4*4; //4B align
    // ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->sub_info.block_num  = BT_AVM_SHARE_BUFFER_SIZE / (((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->sub_info.block_size);
    open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_1 = (uint8_t *)open_param->stream_out_param.bt_common.p_share_info;
    if (config->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) {
        dl_stream_status |= 0x1;
    } else {
        dl_stream_status |= 0x2;
    }
    TRANSMITTER_LOG_I("[BLE Audio Dongle][dl] codec setting: %u, %u, 0x%x, %u, %u, %u, %u, %u, %u, 0x%x, 0x%x, 0x%x, 0x%x\r\n", 13,
                        config->scenario_sub_id,
                        config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.period,
                        config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.channel_enable,
                        payload_size,
                        config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_type,
                        config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.sample_rate,
                        config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.channel_mode,
                        config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.frame_interval,
                        config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.bit_rate,
                        open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_1,
                        ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_1))->start_addr,
                        open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_2,
                        ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_2))->start_addr);
}
#endif

/* Vendor codec */
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
static uint32_t bt_audio_dongle_vendor_lhdc_v5_get_sample_rate(uint8_t vendor_value)
{
    /* ————————————————————————————————————————————————————————————————— */
    /* | - 7 - | - 6 - | - 5 - | - 4 - | - 3 - | - 2 - | - 1 - | - 0 - | */
    /* |  res  |  res  |  44.1 |  48   |  res  |  96   |  res  |  192  | */
    /* ————————————————————————————————————————————————————————————————— */
    uint8_t id = vendor_value;
    uint32_t fs = 0xFFFFFFFF;
    if (vendor_value & (1 << 0)) { // 192K
        fs = 192 * 1000;
        vendor_value &= ~(1 << 0);
    } else if (vendor_value & (1 << 2)) { // 96K
        fs = 96 * 1000;
        vendor_value &= ~(1 << 2);
    } else if (vendor_value & (1 << 4)) { // 48K
        fs = 48 * 1000;
        vendor_value &= ~(1 << 4);
    } else if (vendor_value & (1 << 5)) { // 44.1K
        fs = 44100;
        vendor_value &= ~(1 << 5);
    }
    if (vendor_value != 0) {
        TRANSMITTER_LOG_W("[Dongle Common] lhdc fs id is abnormal 0x%x", 1, id);
    }
    return fs;
}

static uint32_t bt_audio_dongle_vendor_lhdc_v5_get_min_bit_rate(uint8_t vendor_value)
{
    /* ————————————————————————————————————————————————————————————————— */
    /* | - 7 - | - 6 - | - 5 - | - 4 - | - 3 - | - 2 - | - 1 - | - 0 - | */
    /* |  min bitrate  |  max bitrate  |  res  | 16bit | 24bit | 32bit | */
    /* ————————————————————————————————————————————————————————————————— */
    uint8_t  id = (vendor_value >> 6) & (0xFF);
    uint32_t fs = 0xFFFFFFFF;
    switch (id) {
        case  0x0:
            /* no limit */
            fs = 0xFF;
            break;
        case 0x1:
            /* 128 kbps */
            fs = 128;
            break;
        case 0x2:
            /* 256 kbps */
            fs = 256;
            break;
        case 0x3:
            /* 400 kbps */
            fs = 400;
            break;
        default:
            break;
    }
    return fs;
}

static uint32_t bt_audio_dongle_vendor_lhdc_v5_get_max_bit_rate(uint8_t vendor_value)
{
    /* ————————————————————————————————————————————————————————————————— */
    /* | - 7 - | - 6 - | - 5 - | - 4 - | - 3 - | - 2 - | - 1 - | - 0 - | */
    /* |  min bitrate  |  max bitrate  |  res  | 16bit | 24bit | 32bit | */
    /* ————————————————————————————————————————————————————————————————— */
    uint8_t  id = (vendor_value >> 4) & (0xFF);
    uint32_t fs = 0xFFFFFFFF;
    switch (id) {
        case  0x0:
            /* no limit */
            fs = 0xFF;
            break;
        case 0x1:
            /* 400 kbps */
            fs = 400;
            break;
        case 0x2:
            /* 600 kbps */
            fs = 600;
            break;
        case 0x3:
            /* 900 kbps */
            fs = 900;
            break;
        default:
            break;
    }
    return fs;
}

static hal_audio_format_t bt_audio_dongle_vendor_lhdc_v5_get_resolution(uint8_t vendor_value)
{
    /* ————————————————————————————————————————————————————————————————— */
    /* | - 7 - | - 6 - | - 5 - | - 4 - | - 3 - | - 2 - | - 1 - | - 0 - | */
    /* |  min bitrate  |  max bitrate  |  res  | 16bit | 24bit | 32bit | */
    /* ————————————————————————————————————————————————————————————————— */
    uint8_t  id = (vendor_value) & (0xFF);
    uint32_t res = HAL_AUDIO_PCM_FORMAT_DUMMY;
    switch (id) {
        case (1 << 0):
            /* 32bit */
            res = HAL_AUDIO_PCM_FORMAT_S32_LE;
            break;
        case (1 << 1):
            /* 24bit */
            res = HAL_AUDIO_PCM_FORMAT_S24_LE;
            break;
        case (1 << 2):
            /* 16bit */
            res = HAL_AUDIO_PCM_FORMAT_S16_LE;
            break;
        default:
            AUDIO_ASSERT(0 && "Vendor resolution error");
            break;
    }
    return res;
}

static uint32_t bt_audio_dongle_vendor_lhdc_v5_get_frame_interval(uint8_t vendor_value)
{
    /* ————————————————————————————————————————————————————————————————— */
    /* | - 7 - | - 6 - | - 5 - | - 4 - | - 3 - | - 2 - | - 1 - | - 0 - | */
    /* |  res  |  res  |  res  | period|        version number         | */
    /* ————————————————————————————————————————————————————————————————— */
    uint8_t  id = (vendor_value >> 4) & (0xFF);
    uint32_t period = 0xFFFFFFFF;
    switch (id) {
        case  0b1:
            /* 5ms */
            period = 5000;
            break;
        default:
            AUDIO_ASSERT(0 && "Vendor id error");
            break;
    }
    return period;
}

static uint32_t bt_audio_dongle_vendor_lhdc_v5_get_version(uint8_t vendor_value)
{
    /* ————————————————————————————————————————————————————————————————— */
    /* | - 7 - | - 6 - | - 5 - | - 4 - | - 3 - | - 2 - | - 1 - | - 0 - | */
    /* |  res  |  res  |  res  | period|        version number         | */
    /* ————————————————————————————————————————————————————————————————— */
    uint8_t  id = (vendor_value) & (0xFF);
    return id;
}

static uint8_t bt_audio_dongle_vendor_lhdc_v5_get_loss_type(uint8_t vendor_value)
{
    /* ————————————————————————————————————————————————————————————————— */
    /* | - 7 - | - 6 - | - 5 - | - 4 - | - 3 - | - 2 - | - 1 - | - 0 - | */
    /* |lossless|  ll  |  res  |  res  |  res  | meta  |  jas  |  ar   | */
    /* ————————————————————————————————————————————————————————————————— */
    uint8_t  id = (vendor_value >> 4) & (0xFF);
    if (id & (1 << 3)) {
        id = 1;
    } else {
        id = 0;
    }
    return id;
}

static uint8_t bt_audio_dongle_vendor_lhdc_v5_get_meta_type(uint8_t vendor_value)
{
    /* ————————————————————————————————————————————————————————————————— */
    /* | - 7 - | - 6 - | - 5 - | - 4 - | - 3 - | - 2 - | - 1 - | - 0 - | */
    /* |lossless|  ll  |  res  |  res  |  res  | meta  |  jas  |  ar   | */
    /* ————————————————————————————————————————————————————————————————— */
    uint8_t  id = (vendor_value) & (0xFF);
    return id;
}

static bool bt_audio_dongle_vendor_lhdc_v5_get_raw_mode(uint8_t vendor_value)
{
    /* ————————————————————————————————————————————————————————————————— */
    /* | - 7 - | - 6 - | - 5 - | - 4 - | - 3 - | - 2 - | - 1 - | - 0 - | */
    /* | raw   |  res  |  res  |  res  |  res  |  res  |  res  |  res  | */
    /* ————————————————————————————————————————————————————————————————— */
    bool  id = (vendor_value) & (0b10000000);
    return id;
}

bool audio_dongle_vendor_parameter_parse(audio_codec_vendor_config_t *vendor, void *param)
{
    bool ret = false;
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
    if (vendor->vendor_id == AUDIO_DONGLE_VENDOR_CODEC_LHDC_V5_VENDOR_ID) {
        if (vendor->codec_id == AUDIO_DONGLE_VENDOR_CODEC_LHDC_V5_CODEC_ID) {
            TRANSMITTER_LOG_I("[Dongle Common] vendor id 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", 10,
                vendor->vendor_id,
                vendor->codec_id,
                vendor->value[0],
                vendor->value[1],
                vendor->value[2],
                vendor->value[3],
                vendor->value[4], // not used
                vendor->value[5]  // not used
                );
            audio_codec_lhdc_t *lhdc = (audio_codec_lhdc_t *)param;
            /* fs */
            lhdc->sample_rate    = bt_audio_dongle_vendor_lhdc_v5_get_sample_rate(vendor->value[0]);
            /* bitrate */
            lhdc->max_bit_rate   = bt_audio_dongle_vendor_lhdc_v5_get_max_bit_rate(vendor->value[1]);
            lhdc->min_bit_rate   = bt_audio_dongle_vendor_lhdc_v5_get_min_bit_rate(vendor->value[1]);
            /* resolution */
            lhdc->sample_format  = bt_audio_dongle_vendor_lhdc_v5_get_resolution(vendor->value[1]);
            /* frame period */
            lhdc->frame_interval = bt_audio_dongle_vendor_lhdc_v5_get_frame_interval(vendor->value[2]);
            /* version */
            lhdc->version        = bt_audio_dongle_vendor_lhdc_v5_get_version(vendor->value[2]);
            /* loss type */
            lhdc->loss_type      = bt_audio_dongle_vendor_lhdc_v5_get_loss_type(vendor->value[3]);
            /* meta type */
            lhdc->meta_type      = bt_audio_dongle_vendor_lhdc_v5_get_meta_type(vendor->value[3]);
            /* raw mode */
            lhdc->raw_mode       = bt_audio_dongle_vendor_lhdc_v5_get_raw_mode(vendor->value[4]);
            ret = true;
        }
    }
#endif
    if (!ret) {
        TRANSMITTER_LOG_E("[Dongle Common] vendor error: 0x%x 0x%x", 2, vendor->vendor_id, vendor->codec_id);
    }
    return ret;
}
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */
#endif /* dongle side */