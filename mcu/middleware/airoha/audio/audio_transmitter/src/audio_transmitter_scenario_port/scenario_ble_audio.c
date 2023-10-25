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
#include "audio_transmitter_playback_port.h"
#include "usbaudio_drv.h"
#include "bt_sink_srv_ami.h"
#include "hal_audio_message_struct_common.h"
#include "hal_dvfs_internal.h"
#include "scenario_dongle_common.h"
#include "nvkey.h"
#include "hal.h"

/* Private define ------------------------------------------------------------*/
#define USB_RX_PORT_TOTAL 2

#define DONGLE_PAYLOAD_SIZE_USB_RX_PCM 192    //1ms for 48K/16bit/Stereo
#define DONGLE_PAYLOAD_SIZE_ENCODED 155 //10ms/124Kbps for 48K/16bit/Mono/LC3

#define USB_TX_PORT_TOTAL 1

#define DONGLE_PAYLOAD_SIZE_USB_TX_PCM_MAX 192    //1ms for 48K/16bit/Stereo
#define DONGLE_PAYLOAD_SIZE_USB_TX_PCM 128    //1ms for 32K/16bit/Stereo
#define DONGLE_PAYLOAD_SIZE_DECODED 80 //10ms/64Kbps for 32K/16bit/Mono/LC3

#define DONGLE_USB_TX_SEND_CCNI_FRAMES 3

#define BLE_AUDIO_DONGLE_DEBUG_LANTENCY             1
#define BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG       1
#define BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_LOG       0

#define BT_AVM_SHARE_BUFFER_SIZE            (5*1024)

#define GAIN_COMPENSATION_STEP 10

/* Private typedef -----------------------------------------------------------*/

typedef struct {
#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
    uint16_t latency_debug_enable;
    int32_t latency_debug_detect_threshold;
    int16_t latency_debug_last_sample;
    uint16_t latency_debug_last_level;
    hal_gpio_pin_t gpio_pin;
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
    uint8_t usb_first_in_flag;
    uint8_t stream_is_started;
    uint32_t previous_gpt_count;
    audio_transmitter_block_header_t usb_stream_header;
    uint32_t period;
    uint32_t frame_size;
    audio_dsp_codec_type_t usb_type;
    audio_codec_param_t usb_param;
} ble_audio_dongle_usb_handle_t;

typedef struct {
    uint32_t vol_gain_1;
    uint32_t vol_gain_2;
} vol_gain_t;

typedef struct {
    uint8_t scenario_type;
    uint8_t scenario_sub_id;
} dl_mixer_param_t;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
extern uint32_t g_dongle_line_in_default_d_gain;
#endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */
#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
extern uint32_t g_dongle_i2s_in_default_d_gain;
#endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
static ble_audio_dongle_usb_handle_t usb_stream_rx_handle[USB_RX_PORT_TOTAL];
static ble_audio_dongle_usb_handle_t usb_stream_tx_handle[USB_TX_PORT_TOTAL];
#if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
static uint8_t all_zero_buffer[DONGLE_PAYLOAD_SIZE_USB_TX_PCM_MAX];
#endif
extern uint32_t dl_stream_status;
static uint32_t ul_stream_status = 0;
const static int16_t gain_compensation_table[GAIN_COMPENSATION_STEP + 1] = {
    /*
    Ratio |    db    | Compensation
    0%    |  -60db   | 0xE890
    10%   |  -20db   | 0xF830
    20%   | -13.98db | 0xFA8B
    30%   | -10.46db | 0xFBEB
    40%   |  -7.96db | 0xFCE5
    50%   |  -6.02db | 0xFDA6
    60%   |  -4.44db | 0xFE45
    70%   |  -3.1db  | 0xFECB
    80%   |  -1.94db | 0xFF3F
    90%   |  -0.92db | 0xFFA5
    100%  |     0db  | 0
    */
    0xE890,
    0xF830,
    0xFA8B,
    0xFBEB,
    0xFCE5,
    0xFDA6,
    0xFE45,
    0xFECB,
    0xFF3F,
    0xFFA5,
    0x0
};
#ifdef AIR_SILENCE_DETECTION_ENABLE
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
#endif /* AIR_SILENCE_DETECTION_ENABLE */

/* Public variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern void audio_dongle_set_stream_in_afe(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param);
extern void audio_dongle_set_stream_out_bt_common(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param);
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

#if defined(AIR_USB_AUDIO_1_SPK_ENABLE) || defined(AIR_USB_AUDIO_2_SPK_ENABLE)
#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
static void usb_audio_rx_cb_latency_debug(uint32_t port, uint8_t *p_source_buf)
{
    int16_t *start_address = NULL;
    uint32_t current_level = 0;
    uint32_t i;
    int16_t current_sample;
    int16_t next_sample;

    if (usb_stream_rx_handle[port].latency_debug_enable) {
        if (usb_stream_rx_handle[port].usb_param.pcm.format == HAL_AUDIO_PCM_FORMAT_S16_LE)
        {
            current_level = usb_stream_rx_handle[port].latency_debug_last_level;
            start_address = (int16_t *)p_source_buf;

            TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 0] latency_debug monitor 1st sample value %d, frame_size %d, channel num %d, format %d", 4,
                                *start_address,
                                usb_stream_rx_handle[port].frame_size,
                                usb_stream_rx_handle[port].usb_param.pcm.channel_mode,
                                usb_stream_rx_handle[port].usb_param.pcm.format);

            if ((*start_address > usb_stream_rx_handle[port].latency_debug_last_sample) &&
                ((*start_address - usb_stream_rx_handle[port].latency_debug_last_sample) > usb_stream_rx_handle[port].latency_debug_detect_threshold)) {
                current_level = 1;
            }
            else if ((*start_address < usb_stream_rx_handle[port].latency_debug_last_sample) &&
                    (usb_stream_rx_handle[port].latency_debug_last_sample - *start_address > usb_stream_rx_handle[port].latency_debug_detect_threshold)) {
                current_level = 0;
            }
            for (i = 0; i < (usb_stream_rx_handle[port].frame_size / (2 * usb_stream_rx_handle[port].usb_param.pcm.channel_mode) - 1); i++) {
                current_sample  = *((int16_t *)(p_source_buf+i*(2*usb_stream_rx_handle[port].usb_param.pcm.channel_mode)));
                next_sample     = *((int16_t *)(p_source_buf+(i+1)*(2*usb_stream_rx_handle[port].usb_param.pcm.channel_mode)));
                if ((current_sample > next_sample) &&
                    (current_sample - next_sample > usb_stream_rx_handle[port].latency_debug_detect_threshold))
                {
                    current_level = 0;
                    break;
                }
                else if ((current_sample < next_sample) &&
                        (next_sample - current_sample > usb_stream_rx_handle[port].latency_debug_detect_threshold))
                {
                    current_level = 1;
                    break;
                }
            }

            usb_stream_rx_handle[port].latency_debug_last_sample = *((int16_t *)(p_source_buf + 2 * usb_stream_rx_handle[port].usb_param.pcm.channel_mode * (usb_stream_rx_handle[port].frame_size/(2*usb_stream_rx_handle[port].usb_param.pcm.channel_mode)-1)));
            if (current_level != usb_stream_rx_handle[port].latency_debug_last_level)
            {
                TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 0] latency_debug gpio level change to %d", 1, current_level);
                hal_gpio_set_output(usb_stream_rx_handle[port].gpio_pin, current_level);
                usb_stream_rx_handle[port].latency_debug_last_level = current_level;
            }
        }
        else if (usb_stream_rx_handle[port].usb_param.pcm.format == HAL_AUDIO_PCM_FORMAT_S24_LE)
        {
            current_level = usb_stream_rx_handle[port].latency_debug_last_level;
            start_address    = (int16_t *)(p_source_buf+1); // drop the low 8bit

            TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 0] latency_debug monitor 1st sample value %d, frame_size %d, channel num %d, format %d", 4,
                                *start_address,
                                usb_stream_rx_handle[port].frame_size,
                                usb_stream_rx_handle[port].usb_param.pcm.channel_mode,
                                usb_stream_rx_handle[port].usb_param.pcm.format);

            if ((*start_address > usb_stream_rx_handle[port].latency_debug_last_sample) &&
                (*start_address - usb_stream_rx_handle[port].latency_debug_last_sample > usb_stream_rx_handle[port].latency_debug_detect_threshold)) {
                current_level = 1;
            }
            else if ((*start_address < usb_stream_rx_handle[port].latency_debug_last_sample) &&
                    (usb_stream_rx_handle[port].latency_debug_last_sample - *start_address > usb_stream_rx_handle[port].latency_debug_detect_threshold)) {
                current_level = 0;
            }
            for (i = 0; i < usb_stream_rx_handle[port].frame_size/(3*usb_stream_rx_handle[port].usb_param.pcm.channel_mode)-1; i++) {
                current_sample  = *((int16_t *)(p_source_buf+i*(3*usb_stream_rx_handle[port].usb_param.pcm.channel_mode)+1)); // drop the low 8bit
                next_sample     = *((int16_t *)(p_source_buf+(i+1)*(3*usb_stream_rx_handle[port].usb_param.pcm.channel_mode)+1)); // drop the low 8bit
                if ((current_sample > next_sample) &&
                    (current_sample - next_sample) > usb_stream_rx_handle[port].latency_debug_detect_threshold) {
                    current_level = 0;
                    break;
                } else if ((current_sample < next_sample) &&
                        (next_sample - current_sample > usb_stream_rx_handle[port].latency_debug_detect_threshold)) {
                    current_level = 1;
                    break;
                }
            }
            usb_stream_rx_handle[port].latency_debug_last_sample = *((int16_t *)(p_source_buf + 3 * usb_stream_rx_handle[port].usb_param.pcm.channel_mode * (usb_stream_rx_handle[port].frame_size/(3*usb_stream_rx_handle[port].usb_param.pcm.channel_mode)-1) + 1));
            if (current_level != usb_stream_rx_handle[port].latency_debug_last_level) {
                TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 0] latency_debug gpio level change to %d", 1, current_level);
                hal_gpio_set_output(usb_stream_rx_handle[port].gpio_pin, current_level);
                usb_stream_rx_handle[port].latency_debug_last_level = current_level;
            }
        }
    }
}
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
#endif /* AIR_USB_AUDIO_1_SPK_ENABLE || AIR_USB_AUDIO_2_SPK_ENABLE */

#if defined(AIR_USB_AUDIO_1_SPK_ENABLE)
static void usb_audio_rx_cb_ble_audio_dongle_0(void)
{
    uint32_t gpt_count, duration_count;
    uint32_t available_data_size;
    n9_dsp_share_info_t *p_dsp_info;
    uint32_t buf_size;
    uint8_t *p_source_buf;

    /* get current gpt count */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 0]usb_audio_rx_cb_ble_audio_dongle_0 callback = %u", 1, gpt_count);
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    /* get share buffer info */
    p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_DSP_RECEIVE_FROM_MCU_0);

    if (usb_stream_rx_handle[0].usb_first_in_flag == 0) {
        /* this is the first irq, we need to drop all relict USB data */
        available_data_size = USB_Audio_Get_Len_Received_Data(0);
        USB_Audio_Rx_Buffer_Drop_Bytes(0, available_data_size);
        usb_stream_rx_handle[0].usb_first_in_flag = 1;
        usb_stream_rx_handle[0].previous_gpt_count = gpt_count;

        TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 0]usb_audio_rx_cb_ble_audio_dongle_0 callback first time = %u", 1, gpt_count);

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
        TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 1]share buffer addr = 0x%x", 1, p_dsp_info);
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */
    } else {
        /* this is not the first irq, we need to copy usb into share buffer */
        hal_gpt_get_duration_count(usb_stream_rx_handle[0].previous_gpt_count, gpt_count, &duration_count);
        if ((duration_count > 1500) || (duration_count < 500)) {
            TRANSMITTER_LOG_E("[BLE Audio Dongle][USB_RX_DEBUG 0]usb_audio_rx_cb_ble_audio_dongle_0 duration = %d", 1, duration_count);
        }
        usb_stream_rx_handle[0].previous_gpt_count = gpt_count;

        /* get usb data size */
        available_data_size = USB_Audio_Get_Len_Received_Data(0);
        if ((available_data_size > usb_stream_rx_handle[0].frame_size) || (available_data_size % usb_stream_rx_handle[0].frame_size)) {
            TRANSMITTER_LOG_E("[BLE Audio Dongle][USB_RX_DEBUG 0] data in USB buffer is abnormal %d %d\r\n", 2,
                available_data_size,
                usb_stream_rx_handle[0].frame_size
                );
        }
        /* copy usb data into share buffer block one by one */
        while (available_data_size > 0) {
            /* get share buffer block info */
            hal_audio_buf_mgm_get_free_buffer(p_dsp_info, &p_source_buf, &buf_size);
            if (buf_size < (usb_stream_rx_handle[0].frame_size + BLK_HEADER_SIZE)) {
                TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 0]Not enough share buffer space", 0);
                // AUDIO_ASSERT(0);
                break;
            }

            /* update usb data header */
            usb_stream_rx_handle[0].usb_stream_header.sequence_number++;
            usb_stream_rx_handle[0].usb_stream_header.data_length = usb_stream_rx_handle[0].frame_size;

            /* write usb data into share buffer block */
            // memcpy(p_source_buf, &usb_stream_rx_handle[0].usb_stream_header, BLK_HEADER_SIZE);
            ((audio_transmitter_block_header_t *)p_source_buf)->sequence_number = usb_stream_rx_handle[0].usb_stream_header.sequence_number;
            ((audio_transmitter_block_header_t *)p_source_buf)->data_length     = usb_stream_rx_handle[0].usb_stream_header.data_length;
            USB_Audio_Read_Data(0, p_source_buf + BLK_HEADER_SIZE, usb_stream_rx_handle[0].frame_size);
            if (available_data_size % usb_stream_rx_handle[0].frame_size) {
                memset(p_source_buf + BLK_HEADER_SIZE, 0, usb_stream_rx_handle[0].frame_size);
            }
            hal_audio_buf_mgm_get_write_data_done(p_dsp_info, usb_stream_rx_handle[0].frame_size + BLK_HEADER_SIZE);
#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
            TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 0]r_offset = %u, w_offset = %u", 2, p_dsp_info->read_offset, p_dsp_info->write_offset);
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

            //LOG_AUDIO_DUMP(p_source_buf + BLK_HEADER_SIZE, usb_stream_rx_handle[0].frame_size, 13);

            /* get residual usb data size */
            available_data_size = USB_Audio_Get_Len_Received_Data(0);

#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
            usb_audio_rx_cb_latency_debug(0, p_source_buf + BLK_HEADER_SIZE);
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
        }
    }
}
#endif /* AIR_USB_AUDIO_1_SPK_ENABLE */

#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
static void usb_audio_rx_cb_ble_audio_dongle_1(void)
{
    uint32_t gpt_count, duration_count;
    uint32_t available_data_size;
    n9_dsp_share_info_t *p_dsp_info;
    uint32_t buf_size;
    uint8_t *p_source_buf;

    /* get current gpt count */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
    TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 1]usb_audio_rx_cb_ble_audio_dongle_1 callback = %u", 1, gpt_count);
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

    /* get share buffer info */
    p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_DSP_RECEIVE_FROM_MCU_1);

    if (usb_stream_rx_handle[1].usb_first_in_flag == 0) {
        /* this is the first irq, we need to drop all relict USB data */
        available_data_size = USB_Audio_Get_Len_Received_Data(1);
        USB_Audio_Rx_Buffer_Drop_Bytes(1, available_data_size);
        usb_stream_rx_handle[1].usb_first_in_flag = 1;
        usb_stream_rx_handle[1].previous_gpt_count = gpt_count;

        TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 1]usb_audio_rx_cb_ble_audio_dongle_1 callback first time = %u", 1, gpt_count);

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
        TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 1]share buffer addr = 0x%x", 1, p_dsp_info);
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */
    } else {
        /* this is not the first irq, we need to copy usb into share buffer */
        hal_gpt_get_duration_count(usb_stream_rx_handle[1].previous_gpt_count, gpt_count, &duration_count);
        if ((duration_count > 1500) || (duration_count < 500)) {
            TRANSMITTER_LOG_E("[BLE Audio Dongle][USB_RX_DEBUG 1]usb_audio_rx_cb_ble_audio_dongle_1 duration = %d", 1, duration_count);
        }
        usb_stream_rx_handle[1].previous_gpt_count = gpt_count;

        /* get usb data size */
        available_data_size = USB_Audio_Get_Len_Received_Data(1);
        if ((available_data_size > usb_stream_rx_handle[1].frame_size) || (available_data_size % usb_stream_rx_handle[1].frame_size)) {
            TRANSMITTER_LOG_E("[BLE Audio Dongle][USB_RX_DEBUG 1] data in USB buffer is abnormal %d %d\r\n", 2,
                available_data_size,
                usb_stream_rx_handle[1].frame_size
                );
        }

        /* copy usb data into share buffer block one by one */
        while (available_data_size > 0) {
            /* get share buffer block info */
            hal_audio_buf_mgm_get_free_buffer(p_dsp_info, &p_source_buf, &buf_size);
            if (buf_size < (usb_stream_rx_handle[1].frame_size + BLK_HEADER_SIZE)) {
                TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 1]Not enough share buffer space", 0);
                // AUDIO_ASSERT(0);
                break;
            }

            /* update usb data header */
            usb_stream_rx_handle[1].usb_stream_header.sequence_number++;
            usb_stream_rx_handle[1].usb_stream_header.data_length = usb_stream_rx_handle[1].frame_size;

            /* write usb data into share buffer block */
            // memcpy(p_source_buf, &usb_stream_rx_handle[1].usb_stream_header, BLK_HEADER_SIZE);
            ((audio_transmitter_block_header_t *)p_source_buf)->sequence_number = usb_stream_rx_handle[1].usb_stream_header.sequence_number;
            ((audio_transmitter_block_header_t *)p_source_buf)->data_length     = usb_stream_rx_handle[1].usb_stream_header.data_length;
            USB_Audio_Read_Data(1, p_source_buf + BLK_HEADER_SIZE, usb_stream_rx_handle[1].frame_size);
            if (available_data_size % usb_stream_rx_handle[1].frame_size) {
                memset(p_source_buf + BLK_HEADER_SIZE, 0, usb_stream_rx_handle[1].frame_size);
            }
            hal_audio_buf_mgm_get_write_data_done(p_dsp_info, usb_stream_rx_handle[1].frame_size + BLK_HEADER_SIZE);

#if BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG
            TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 1]r_offset = %u, w_offset = %u", 2, p_dsp_info->read_offset, p_dsp_info->write_offset);
#endif /* BLE_AUDIO_DONGLE_MUSIC_PATH_DEBUG_LOG */

            //LOG_AUDIO_DUMP(p_source_buf + BLK_HEADER_SIZE, usb_stream_rx_handle[1].frame_size, 13);

            /* get residual usb data size */
            available_data_size = USB_Audio_Get_Len_Received_Data(1);

#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
            usb_audio_rx_cb_latency_debug(1, p_source_buf + BLK_HEADER_SIZE);
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
        }
    }
}
#endif /* AIR_USB_AUDIO_2_SPK_ENABLE */

#if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
static void usb_audio_tx_trigger_dsp_flow(void)
{
    hal_ccni_message_t ccni_msg = {{0}};
    hal_ccni_status_t ccni_status;

    ccni_status = hal_ccni_set_event(CCNI_CM4_TO_DSP0_BT_COMMON, &ccni_msg);
    if (ccni_status != HAL_CCNI_STATUS_OK) {
        TRANSMITTER_LOG_E("[usb_audio_tx_trigger_dsp_flow]send ccni fail, %d\r\n", 1, ccni_status);
        // AUDIO_ASSERT(0);
    }

#if BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_LOG
    TRANSMITTER_LOG_I("[usb_audio_tx_trigger_dsp_flow]send ccni", 0);
#endif /* BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_LOG */
}

static void usb_audio_tx_cb_ble_audio_dongle_0(void)
{
    uint32_t gpt_count, duration_count;
    uint32_t data_size;
    n9_dsp_share_info_t *p_dsp_info;
    uint32_t blk_size;
    uint32_t data_size_total;
    uint8_t *p_source_buf;

    /* get current gpt count */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
#if BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_LOG
    TRANSMITTER_LOG_I("[BLE Audio Dongle]usb_audio_tx_cb_ble_audio_dongle_0 callback = %u", 1, gpt_count);
#endif /* BLE_AUDIO_DONGLE_VOICE_PATH_DEBUG_LOG */

    if ((ul_stream_status & 0x1) == 0) {
        /* workaround: If the stream is not started, need to send zero data to usb to avoid usb host error */
        USB_Audio_TX_SendData(0, usb_stream_tx_handle[0].frame_size, (uint8_t *)all_zero_buffer);
        return;
    }

    /* get share buffer info */
    p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_DSP_SEND_TO_MCU);

    if (usb_stream_tx_handle[0].usb_first_in_flag == 0) {
        usb_stream_tx_handle[0].previous_gpt_count = gpt_count;
        usb_stream_tx_handle[0].usb_first_in_flag  = 1;

        TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_TX_DEBUG 0]usb_audio_tx_cb_ble_audio_dongle_0 callback first time = %u", 1, gpt_count);
    } else {
        hal_gpt_get_duration_count(usb_stream_tx_handle[0].previous_gpt_count, gpt_count, &duration_count);
        if ((duration_count > 1500) || (duration_count < 500)) {
            TRANSMITTER_LOG_E("[BLE Audio Dongle][USB_TX_DEBUG 0]usb_audio_tx_cb_ble_audio_dongle_0 duration = %d", 1, duration_count);
        }
        usb_stream_tx_handle[0].previous_gpt_count = gpt_count;
    }

    /* get data info */
    hal_audio_buf_mgm_get_data_buffer(p_dsp_info, &p_source_buf, &data_size_total);
    if (data_size_total == 0) {
        data_size = 0;
        p_source_buf = NULL;
    } else {
        data_size = ((audio_transmitter_block_header_t *)p_source_buf)->data_length;
        /* check data size */
        if (data_size != usb_stream_tx_handle[0].frame_size) {
            TRANSMITTER_LOG_E("[BLE Audio Dongle]usb_audio_tx_cb_ble_audio_dongle_0 data_size is not right %u, %u", 2, data_size, usb_stream_tx_handle[0].frame_size);
            AUDIO_ASSERT(0);
        }
        p_source_buf += sizeof(audio_transmitter_block_header_t);
    }

    /* check if needs to send ccni to trigger dsp flow */
    if (data_size_total == (usb_stream_tx_handle[0].frame_size + sizeof(audio_transmitter_block_header_t))*DONGLE_USB_TX_SEND_CCNI_FRAMES) {
        /* send ccni to trigger dsp flow if there are only 3ms data in share buffer, so dsp has 3ms to process new data */
        usb_audio_tx_trigger_dsp_flow();
    } else if (data_size_total == 0) {
        /* send ccni to trigger dsp flow if there is no data in share buffer */
        usb_audio_tx_trigger_dsp_flow();
    }

    /* send usb data */
    if (data_size == 0) {
        if (usb_stream_tx_handle[0].stream_is_started == 0) {
            /* the stream is not started, so send slience data */
            USB_Audio_TX_SendData(0, usb_stream_tx_handle[0].frame_size, (uint8_t *)all_zero_buffer);
        } else {
            USB_Audio_TX_SendData(0, usb_stream_tx_handle[0].frame_size, (uint8_t *)all_zero_buffer);
            TRANSMITTER_LOG_E("[BLE Audio Dongle]usb_audio_tx_cb_ble_audio_dongle_0 data is not enough", 0);
            // AUDIO_ASSERT(0);
        }
    } else {
        /* set data from share buffer into USB FIFO */
        USB_Audio_TX_SendData(0, usb_stream_tx_handle[0].frame_size, p_source_buf);

        /* drop this packet */
        blk_size = p_dsp_info->sub_info.block_info.block_size;
        hal_audio_buf_mgm_get_read_data_done(p_dsp_info, blk_size);

        if (usb_stream_tx_handle[0].stream_is_started == 0) {
            /* set that the stream is started */
            usb_stream_tx_handle[0].stream_is_started = 1;
        }
    }

#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
    int16_t *start_address = NULL;
    uint32_t current_level = 0;
    uint32_t i;
    int16_t sample_value = 0;
    uint16_t frame_samples;
    uint16_t channel_num;
    uint16_t resolution_size;

    if (usb_stream_tx_handle[0].latency_debug_enable) {
        if (data_size == 0) {
            start_address = (int16_t *)all_zero_buffer;
        } else {
            start_address = (int16_t *)p_source_buf;
        }
        if (usb_stream_tx_handle[0].usb_param.pcm.channel_mode == 1) {
            channel_num = 1;
        } else {
            channel_num = 2;
        }
        resolution_size = 2;
        frame_samples = usb_stream_tx_handle[0].frame_size / resolution_size / channel_num;
        for (i = 0; i < frame_samples; i++) {
            sample_value += (*(start_address + i * channel_num) / frame_samples);
        }
        if (sample_value >= 5000) {
            current_level = 1;
        } else {
            current_level = 0;
        }
        if (current_level != usb_stream_tx_handle[0].latency_debug_last_level) {
            hal_gpio_set_output(usb_stream_tx_handle[0].gpio_pin, current_level);
            usb_stream_tx_handle[0].latency_debug_last_level = current_level;
        }
    }
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
}
#endif /* AIR_USB_AUDIO_1_MIC_ENABLE */

/* Public functions ----------------------------------------------------------*/
#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ble_audio_dongle_rx_latency_debug_control(uint32_t port, bool enable, hal_gpio_pin_t gpio_pin, int32_t detect_threshold)
{
    uint32_t saved_mask;

    hal_gpio_init(gpio_pin);
    hal_pinmux_set_function(gpio_pin, 0);
    hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(gpio_pin, HAL_GPIO_DATA_LOW);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (enable) {
        usb_stream_rx_handle[port].latency_debug_enable = 1;
        usb_stream_rx_handle[port].latency_debug_last_level = 0;
        usb_stream_rx_handle[port].latency_debug_last_sample = 0;
    } else {
        usb_stream_rx_handle[port].latency_debug_enable = 0;
        usb_stream_rx_handle[port].latency_debug_last_level = 0;
        usb_stream_rx_handle[port].latency_debug_last_sample = 0;
    }
    usb_stream_rx_handle[port].gpio_pin = gpio_pin;
    usb_stream_rx_handle[port].latency_debug_detect_threshold = detect_threshold;

    hal_nvic_restore_interrupt_mask(saved_mask);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ble_audio_dongle_tx_latency_debug_control(uint32_t port, bool enable, hal_gpio_pin_t gpio_pin)
{
    uint32_t saved_mask;

    hal_gpio_init(gpio_pin);
    hal_pinmux_set_function(gpio_pin, 0);
    hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_OUTPUT);
    hal_gpio_set_output(gpio_pin, HAL_GPIO_DATA_LOW);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (enable) {
        usb_stream_tx_handle[port].latency_debug_enable = 1;
        usb_stream_tx_handle[port].latency_debug_last_level = 0;
        usb_stream_tx_handle[port].latency_debug_last_sample = 0;
    } else {
        usb_stream_tx_handle[port].latency_debug_enable = 0;
        usb_stream_tx_handle[port].latency_debug_last_level = 0;
        usb_stream_tx_handle[port].latency_debug_last_sample = 0;
    }
    usb_stream_tx_handle[port].gpio_pin = gpio_pin;

    hal_nvic_restore_interrupt_mask(saved_mask);
}
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */

void ble_audio_dongle_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    uint32_t payload_size = 0;
    uint32_t gain;
    uint8_t volume_level;
    uint8_t volume_ratio;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    sysram_status_t status;
    DSP_FEATURE_TYPE_LIST AudioFeatureList_SilenceDetection[] = {
        FUNC_SILENCE_DETECTION,
        FUNC_END,
    };
#endif /*AIR_SILENCE_DETECTION_ENABLE  */

    switch (config->scenario_sub_id) {
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT:
            /* stream source */
            /* get codec frame size */
            payload_size = ble_audio_codec_get_frame_size(&(config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_type),
                                                          &(config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param));
            if ((payload_size == 0) || (payload_size != config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param.lc3.frame_size)) {
                TRANSMITTER_LOG_E("[BLE Audio Dongle] error codec frame size %d, %d\r\n", 2, payload_size, config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param.lc3.frame_size);
                AUDIO_ASSERT(0);
            }
            open_param->param.stream_in = STREAM_IN_BT_COMMON;
            open_param->stream_in_param.bt_common.scenario_type = config->scenario_type;
            open_param->stream_in_param.bt_common.scenario_sub_id = config->scenario_sub_id;
            open_param->stream_in_param.bt_common.share_info_type = SHARE_BUFFER_INFO_TYPE;
            open_param->stream_in_param.bt_common.data_notification_frequency = 1;
            open_param->stream_in_param.bt_common.max_payload_size = payload_size + sizeof(LE_AUDIO_HEADER);
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.period                          = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.period;
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.channel_enable                  = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.channel_enable;
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.codec_type                      = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_type;
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3.sample_rate     = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param.lc3.sample_rate;
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3.channel_mode    = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param.lc3.channel_mode;
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval  = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param.lc3.frame_interval;
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_size      = payload_size;
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3.bit_rate        = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param.lc3.bit_rate;
            /* upper layer will prepare channel 2's share memory */
            open_param->stream_in_param.bt_common.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_RECEIVE_FROM_AIR_1);
            // memset((((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->start_addr), 0, BT_AVM_SHARE_BUFFER_SIZE);
            // ((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->read_offset          = 0;
            // ((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->write_offset         = 0;
            // ((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->sub_info.block_size  = (payload_size+sizeof(LE_AUDIO_HEADER)+3)/4*4; //4B align
            // ((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->sub_info.block_num   = BT_AVM_SHARE_BUFFER_SIZE / (((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->sub_info.block_size);
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_2 = (uint8_t *)open_param->stream_in_param.bt_common.p_share_info;
            /* upper layer will prepare channel 1's share memory */
            open_param->stream_in_param.bt_common.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_RECEIVE_FROM_AIR_0);
            // memset((((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->start_addr), 0, BT_AVM_SHARE_BUFFER_SIZE);
            // ((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->read_offset          = 0;
            // ((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->write_offset         = 0;
            // ((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->sub_info.block_size  = (payload_size+sizeof(LE_AUDIO_HEADER)+3)/4*4; //4B align
            // ((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->sub_info.block_num   = BT_AVM_SHARE_BUFFER_SIZE / (((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.p_share_info))->sub_info.block_size);
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_1 = (uint8_t *)open_param->stream_in_param.bt_common.p_share_info;
            /* gain setting */
            gain = audio_get_gain_in_in_dB(0, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
            volume_level = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.vol_level.vol_level_l;
            if(volume_level > bt_sink_srv_ami_get_usb_voice_sw_max_volume_level()){
                volume_level = bt_sink_srv_ami_get_usb_voice_sw_max_volume_level();
            }
            gain = audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.gain_default_L = gain;
            volume_level = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.vol_level.vol_level_r;
            if(volume_level > bt_sink_srv_ami_get_usb_voice_sw_max_volume_level()){
                volume_level = bt_sink_srv_ami_get_usb_voice_sw_max_volume_level();
            }
            gain = audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
            open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.gain_default_R = gain;
            if (config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.test_mode_enable) {
                open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.gain_default_L = 0;
                open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.gain_default_R = 0;
                TRANSMITTER_LOG_I("[BLE Audio Dongle][ul] enter test mode\r\n", 0);
            }
            TRANSMITTER_LOG_I("[BLE Audio Dongle][ul] codec setting: %u, %u, 0x%x, %u, %u, %u, %u, %u, %u, 0x%x, 0x%x, 0x%x, 0x%x\r\n", 13,
                              config->scenario_sub_id,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.period,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.channel_enable,
                              payload_size,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_type,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param.lc3.sample_rate,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param.lc3.channel_mode,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param.lc3.frame_interval,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.codec_param.lc3.bit_rate,
                              open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_1,
                              ((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_1))->start_addr,
                              open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_2,
                              ((n9_dsp_share_info_t *)(open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.share_buffer_channel_2))->start_addr);

            /* stream sink */
            /* get usb frame size */
            payload_size = usb_audio_get_frame_size(&(config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_type),
                                                    &(config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_param));
            if (payload_size == 0) {
                TRANSMITTER_LOG_E("[BLE Audio Dongle] error usb frame size %d\r\n", 1, payload_size);
                AUDIO_ASSERT(0);
            }
            open_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;
            open_param->stream_out_param.data_ul.scenario_type = config->scenario_type;
            open_param->stream_out_param.data_ul.scenario_sub_id = config->scenario_sub_id;
            open_param->stream_out_param.data_ul.data_notification_frequency = 0;
            open_param->stream_out_param.data_ul.max_payload_size = payload_size;
            open_param->stream_out_param.data_ul.scenario_param.ble_audio_dongle_param.period                       = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.period;
            open_param->stream_out_param.data_ul.scenario_param.ble_audio_dongle_param.channel_enable               = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.channel_enable;
            open_param->stream_out_param.data_ul.scenario_param.ble_audio_dongle_param.codec_type                   = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_type;
            open_param->stream_out_param.data_ul.scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate  = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_param.pcm.sample_rate;
            open_param->stream_out_param.data_ul.scenario_param.ble_audio_dongle_param.codec_param.pcm.channel_mode = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_param.pcm.channel_mode;
            open_param->stream_out_param.data_ul.scenario_param.ble_audio_dongle_param.codec_param.pcm.format       = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_param.pcm.format;
            usb_stream_tx_handle[0].period                      = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.period;
            usb_stream_tx_handle[0].frame_size                  = payload_size;
            usb_stream_tx_handle[0].usb_type                    = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_type;
            usb_stream_tx_handle[0].usb_param.pcm.sample_rate   = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_param.pcm.sample_rate;
            usb_stream_tx_handle[0].usb_param.pcm.channel_mode  = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_param.pcm.channel_mode;
            usb_stream_tx_handle[0].usb_param.pcm.format        = config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_param.pcm.format;
            open_param->stream_out_param.data_ul.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_DSP_SEND_TO_MCU);
            open_param->stream_out_param.data_ul.p_share_info->read_offset = 0;
            open_param->stream_out_param.data_ul.p_share_info->write_offset = 0;
            open_param->stream_out_param.data_ul.p_share_info->bBufferIsFull = false;
            audio_transmitter_modify_share_info_by_block(open_param->stream_out_param.data_ul.p_share_info, payload_size);
            TRANSMITTER_LOG_I("[BLE Audio Dongle][ul] usb setting: %u, %u, 0x%x, %u, %u, %u, %u, %u, %d, %d, 0x%x\r\n", 11,
                              config->scenario_sub_id,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.period,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.channel_enable,
                              payload_size,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_type,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_param.pcm.sample_rate,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_param.pcm.channel_mode,
                              config->scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.usb_param.pcm.format,
                              open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.gain_default_L,
                              open_param->stream_in_param.bt_common.scenario_param.ble_audio_dongle_param.gain_default_R,
                              open_param->stream_out_param.data_ul.p_share_info);
            break;

        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
#ifdef AIR_SILENCE_DETECTION_ENABLE
            /* prepare silence detection NVKEY */
            /* reset share buffer before put parameters */
            audio_nvdm_reset_sysram();
            status = NVDM_STATUS_ERROR;
            while (status != NVDM_STATUS_NAT_OK)
            {
                /* set NVKEYs that the usb chat stream uses into the share buffer */
                status = audio_nvdm_set_feature(sizeof(AudioFeatureList_SilenceDetection)/sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_SilenceDetection);
                if (status != NVDM_STATUS_NAT_OK)
                {
                    TRANSMITTER_LOG_E("[SilenceDetection] failed to set parameters to share memory - err(%d)\r\n", 1, status);
                    // AUDIO_ASSERT(0);
                    nvkey_write_data(NVID_DSP_ALG_SIL_DET2, (const uint8_t *)&NVKEY_E711[0], sizeof(NVKEY_E711));
                }
            }
#endif /* AIR_SILENCE_DETECTION_ENABLE */

            /* stream source */
            /* get usb frame size */
            payload_size = usb_audio_get_frame_size(&(config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_type),
                                                    &(config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param));
            if (payload_size == 0) {
                TRANSMITTER_LOG_E("[BLE Audio Dongle] error usb frame size %d\r\n", 1, payload_size);
                AUDIO_ASSERT(0);
            }
            open_param->param.stream_in = STREAM_IN_AUDIO_TRANSMITTER;
            open_param->stream_in_param.data_dl.scenario_type = config->scenario_type;
            open_param->stream_in_param.data_dl.scenario_sub_id = config->scenario_sub_id;
            open_param->stream_in_param.data_dl.data_notification_frequency = 0;
            open_param->stream_in_param.data_dl.max_payload_size = payload_size;
            open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.period                        = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.period;
            open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.channel_enable                = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.channel_enable;
            open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.codec_type                    = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_type;
            open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.codec_param.pcm.sample_rate   = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.sample_rate;
            open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.codec_param.pcm.channel_mode  = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.channel_mode;
            open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.codec_param.pcm.format        = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.format;
            if (config->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) {
                usb_stream_rx_handle[0].period                      = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.period;
                usb_stream_rx_handle[0].frame_size                  = payload_size;
                usb_stream_rx_handle[0].usb_type                    = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_type;
                usb_stream_rx_handle[0].usb_param.pcm.sample_rate   = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.sample_rate;
                usb_stream_rx_handle[0].usb_param.pcm.channel_mode  = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.channel_mode;
                usb_stream_rx_handle[0].usb_param.pcm.format        = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.format;
                open_param->stream_in_param.data_dl.p_share_info    = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_DSP_RECEIVE_FROM_MCU_0);
            } else {
                usb_stream_rx_handle[1].period                      = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.period;
                usb_stream_rx_handle[1].frame_size                  = payload_size;
                usb_stream_rx_handle[1].usb_type                    = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_type;
                usb_stream_rx_handle[1].usb_param.pcm.sample_rate   = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.sample_rate;
                usb_stream_rx_handle[1].usb_param.pcm.channel_mode  = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.channel_mode;
                usb_stream_rx_handle[1].usb_param.pcm.format        = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.format;
                open_param->stream_in_param.data_dl.p_share_info    = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_DSP_RECEIVE_FROM_MCU_1);
            }
            open_param->stream_in_param.data_dl.p_share_info->read_offset = 0;
            open_param->stream_in_param.data_dl.p_share_info->write_offset = 0;
            open_param->stream_in_param.data_dl.p_share_info->bBufferIsFull = false;
            audio_transmitter_modify_share_info_by_block(open_param->stream_in_param.data_dl.p_share_info, payload_size);
            /* gain setting */
            gain = audio_get_gain_out_in_dB(0, GAIN_DIGITAL, VOL_USB_AUDIO_SW_IN);
            volume_ratio = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.vol_level.vol_ratio;
            volume_level = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.vol_level.vol_level_l;
            if(volume_level > bt_sink_srv_ami_get_usb_music_sw_max_volume_level()){
                volume_level = bt_sink_srv_ami_get_usb_music_sw_max_volume_level();
            }
            gain = audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_AUDIO_SW_IN);
            gain = gain + gain_compensation_table[volume_ratio/GAIN_COMPENSATION_STEP];
            open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.gain_default_L = gain;
            volume_level = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.vol_level.vol_level_r;
            if(volume_level > bt_sink_srv_ami_get_usb_music_sw_max_volume_level()){
                volume_level = bt_sink_srv_ami_get_usb_music_sw_max_volume_level();
            }
            gain = audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_AUDIO_SW_IN);
            gain = gain + gain_compensation_table[volume_ratio/GAIN_COMPENSATION_STEP];
            open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.gain_default_R = gain;
            if (config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.test_mode_enable) {
                open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.gain_default_L = 0;
                open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.gain_default_R = 0;
                TRANSMITTER_LOG_I("[BLE Audio Dongle][dl] enter test mode\r\n", 0);
            }
            TRANSMITTER_LOG_I("[BLE Audio Dongle][dl] usb setting: %u, %u, 0x%x, %u, %u, %u, %u, %u, %d, %d, 0x%x\r\n", 11,
                              config->scenario_sub_id,
                              config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.period,
                              config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.channel_enable,
                              payload_size,
                              config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_type,
                              config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.sample_rate,
                              config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.channel_mode,
                              config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.usb_param.pcm.format,
                              open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.gain_default_L,
                              open_param->stream_in_param.data_dl.scenario_param.ble_audio_dongle_param.gain_default_R,
                              open_param->stream_in_param.data_dl.p_share_info);

            /* stream sink */
            /* get codec frame size */
            payload_size = ble_audio_codec_get_frame_size(&(config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_type),
                                                          &(config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param));
            if ((payload_size == 0) || (payload_size != config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.frame_size)) {
                TRANSMITTER_LOG_E("[BLE Audio Dongle] error codec frame size %d, %d\r\n", 2, payload_size, config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.frame_size);
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
            open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3.sample_rate    = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.sample_rate;
            open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3.channel_mode   = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.channel_mode;
            open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_interval = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.frame_interval;
            open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3.frame_size     = payload_size;
            open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.codec_param.lc3.bit_rate       = config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.codec_param.lc3.bit_rate;
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
#ifdef AIR_SILENCE_DETECTION_ENABLE
            /* check if enter without bt link mode */
            if (config->scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config.without_bt_link_mode_enable)
            {
                open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.without_bt_link_mode_enable = 1;
                TRANSMITTER_LOG_I("[BLE Audio Dongle][dl] enter without_bt_link_mode mode\r\n", 0);
            }
            else
            {
                open_param->stream_out_param.bt_common.scenario_param.ble_audio_dongle_param.without_bt_link_mode_enable = 0;
            }
#endif /*AIR_SILENCE_DETECTION_ENABLE  */
            break;

#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
            /* config stream in */
            audio_dongle_set_stream_in_afe(config, open_param);
            /* config stream out */
            audio_dongle_set_stream_out_bt_common(config, open_param);
            break;
#endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */
#if defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
            /* config stream in */
            audio_dongle_set_stream_in_afe(config, open_param);
            /* config stream out */
            audio_dongle_set_stream_out_bt_common(config, open_param);
            break;
#endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
        default:
            TRANSMITTER_LOG_E("not in ble source dongle scenario sub id list\r\n", 0);
            AUDIO_ASSERT(0);
            break;
    }
}

void ble_audio_dongle_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param)
{
    switch (config->scenario_sub_id) {
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT:
            start_param->param.stream_in = STREAM_IN_BT_COMMON;
            start_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;
            break;

        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
            start_param->param.stream_in = STREAM_IN_AUDIO_TRANSMITTER;
            start_param->param.stream_out = STREAM_OUT_BT_COMMON;
            break;
#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
            audio_dongle_set_start_avm_config(config, start_param);
            break;
#endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */
#if defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
            audio_dongle_set_start_avm_config(config, start_param);
            break;
#endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
        default:
            TRANSMITTER_LOG_E("not in ble source dongle scenario sub id list\r\n", 0);
            AUDIO_ASSERT(0);
            break;
    }
}

audio_transmitter_status_t ble_audio_dongle_set_runtime_config_playback(audio_transmitter_config_t *config, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config, mcu2dsp_audio_transmitter_runtime_config_param_t *runtime_config_param)
{
    audio_transmitter_status_t ret              = AUDIO_TRANSMITTER_STATUS_FAIL;
    uint32_t                   operation        = runtime_config_type;
    vol_gain_t                 gain             = {0, 0};
    uint8_t                    volume_level     = 0;
    uint8_t                    volume_ratio     = 0;
    uint8_t                    volume_level_max = 0;
    switch (config->scenario_sub_id) {
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
#endif
            {
                switch (operation) {
                    case BLE_AUDIO_DONGLE_CONFIG_OP_MUSIC_MIX:
                        TRANSMITTER_LOG_I("scenario_sub_id =%d: MUSIC_MIX id %d.", 2, config->scenario_sub_id, runtime_config->ble_audio_dongle_runtime_config.dl_mixer_id);
                        runtime_config_param->config_operation = operation;
                        dl_mixer_param_t dl_mixer;
                        dl_mixer.scenario_type = g_audio_transmitter_control[runtime_config->ble_audio_dongle_runtime_config.dl_mixer_id].config.scenario_type;
                        dl_mixer.scenario_sub_id = g_audio_transmitter_control[runtime_config->ble_audio_dongle_runtime_config.dl_mixer_id].config.scenario_sub_id;
                        memcpy(runtime_config_param->config_param, &dl_mixer, sizeof(dl_mixer_param_t));
                        ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                        break;

                    case BLE_AUDIO_DONGLE_CONFIG_OP_MUSIC_UNMIX:
                        runtime_config_param->config_operation = operation;
                        ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                        break;

                    case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_L:
                    case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_R:
                    case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL:
                if ((operation == BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_L) || (operation == BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL))
                {
                        volume_ratio = runtime_config->ble_audio_dongle_runtime_config.vol_level.vol_ratio;
                        if (volume_ratio > 100) {
                            TRANSMITTER_LOG_E("Volume ratio should between 0 and 100, volume_ratio = \r\n", 1, volume_ratio);
                            volume_ratio = 100;
                        }
                        volume_level_max = (config->scenario_sub_id != AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) ?
                                            bt_sink_srv_ami_get_usb_music_sw_max_volume_level() :
                                            bt_sink_srv_ami_get_lineIN_max_volume_level();
                        if ((operation == BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_L) || (operation == BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL)) {
                            volume_level = runtime_config->ble_audio_dongle_runtime_config.vol_level.vol_level_l;
                            if (volume_level > volume_level_max) {
                                volume_level = volume_level_max;
                                TRANSMITTER_LOG_E("set L volume %d level more than max level %d\r\n", 2, volume_level, volume_level_max);
                            }
                            gain.vol_gain_1 = (config->scenario_sub_id != AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) ?
                                                audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT) :
                                                audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_LINE_IN);
                            gain.vol_gain_1 += gain_compensation_table[volume_ratio / GAIN_COMPENSATION_STEP];
#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
                            if (config->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) {
                                gain.vol_gain_1 += (g_dongle_line_in_default_d_gain);
                                TRANSMITTER_LOG_I("[BLE][Line in] L default d gain = %d", 1, g_dongle_line_in_default_d_gain);
                            }
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
                            if (config->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) {
                                gain.vol_gain_1 += g_dongle_i2s_in_default_d_gain;
                                TRANSMITTER_LOG_I("[BLE][i2s in] L default d gain = %d", 1, g_dongle_i2s_in_default_d_gain);
                            }
#endif
                        }
                        if ((operation == BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_R) || (operation == BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL)) {
                            volume_level = runtime_config->ble_audio_dongle_runtime_config.vol_level.vol_level_r;
                            if (volume_level > volume_level_max) {
                                volume_level = volume_level_max;
                                TRANSMITTER_LOG_E("set L volume %d level more than max level %d\r\n", 2, volume_level, volume_level_max);
                            }
                            gain.vol_gain_2 = (config->scenario_sub_id != AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) ?
                                                audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT) :
                                                audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_LINE_IN);
                            gain.vol_gain_2 += gain_compensation_table[volume_ratio / GAIN_COMPENSATION_STEP];
#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
                            if (config->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) {
                                gain.vol_gain_2 += (g_dongle_line_in_default_d_gain);
                                TRANSMITTER_LOG_I("[BLE][Line in] R default d gain = %d", 1, g_dongle_line_in_default_d_gain);
                            }
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
                            if (config->scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) {
                                gain.vol_gain_2 += g_dongle_i2s_in_default_d_gain;
                                TRANSMITTER_LOG_I("[BLE][i2s in] R default d gain = %d", 1, g_dongle_i2s_in_default_d_gain);
                            }
#endif
                        }
                        runtime_config_param->config_operation = operation;
                        memcpy(runtime_config_param->config_param, &gain, sizeof(vol_gain_t));
                        TRANSMITTER_LOG_I("scenario_sub_id =%d: operation %d L:volume level %d gain=%d R:volume level %d gain=%d volume_ratio = %d.", 7,
                                            config->scenario_sub_id,
                                            operation,
                                            runtime_config->ble_audio_dongle_runtime_config.vol_level.vol_level_l,
                                            gain.vol_gain_1,
                                            runtime_config->ble_audio_dongle_runtime_config.vol_level.vol_level_r,
                                            gain.vol_gain_2,
                                            volume_ratio
                                            );
                        ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                        break;

#ifdef AIR_SILENCE_DETECTION_ENABLE
                    case BLE_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_ENABLE:
                        runtime_config_param->config_operation = operation;
                        TRANSMITTER_LOG_I("scenario_sub_id =%d: operation %d.",
                                        2 ,config->scenario_sub_id, operation);
                        ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                        break;

                    case BLE_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_DISABLE:
                        runtime_config_param->config_operation = operation;
                        TRANSMITTER_LOG_I("scenario_sub_id =%d: operation %d.",
                                        2 ,config->scenario_sub_id, operation);
                        ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                        break;
#endif /*AIR_SILENCE_DETECTION_ENABLE  */

                    default:
                        TRANSMITTER_LOG_E("subid %d can not do in BLE", 1,
                            config->scenario_sub_id
                            );
                        break;
                }
            }
            break;
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT:
            {
                switch (operation) {
                    case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_L:
                    case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_R:
                    case BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_DUL:
                if ((operation == BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_L) || (operation == BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_DUL))
                {
                        volume_level = runtime_config->ble_audio_dongle_runtime_config.vol_level.vol_level_l;
                        if (volume_level > bt_sink_srv_ami_get_usb_voice_sw_max_volume_level()) {
                            volume_level = bt_sink_srv_ami_get_usb_voice_sw_max_volume_level();
                            TRANSMITTER_LOG_E("set L volume %d level more than max level %d\r\n", 2, volume_level, bt_sink_srv_ami_get_usb_voice_sw_max_volume_level());
                        }
                        gain.vol_gain_1 = audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
                }
                if ((operation == BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_R) || (operation == BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_DUL))
                {
                        volume_level = runtime_config->ble_audio_dongle_runtime_config.vol_level.vol_level_r;
                        if (volume_level > bt_sink_srv_ami_get_usb_voice_sw_max_volume_level()) {
                            volume_level = bt_sink_srv_ami_get_usb_voice_sw_max_volume_level();
                            TRANSMITTER_LOG_E("set R volume %d level more than max level %d\r\n", 2, volume_level, bt_sink_srv_ami_get_usb_voice_sw_max_volume_level());
                        }
                        gain.vol_gain_2 = audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
                }
                        runtime_config_param->config_operation = operation;
                        memcpy(runtime_config_param->config_param, &gain, sizeof(vol_gain_t));
                        TRANSMITTER_LOG_I("scenario_sub_id =%d: operation %d L:volume level %d gain=%d R:volume level %d gain=%d volume_ratio = %d.",
                                        7, config->scenario_sub_id, operation, runtime_config->ble_audio_dongle_runtime_config.vol_level.vol_level_l, gain.vol_gain_1, runtime_config->ble_audio_dongle_runtime_config.vol_level.vol_level_r, gain.vol_gain_2, volume_ratio);
                        ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                        break;

                    case BLE_AUDIO_DONGLE_CONFIG_OP_SET_UL_CH1_INPUT_SOURCE:
                    case BLE_AUDIO_DONGLE_CONFIG_OP_SET_UL_CH2_INPUT_SOURCE:
                        runtime_config_param->config_operation = operation;
                        memcpy(runtime_config_param->config_param, &runtime_config->ble_audio_dongle_runtime_config.channel_enable, sizeof(uint32_t));
                        TRANSMITTER_LOG_I("scenario_sub_id =%d: operation %d channel_enable 0x%x.",
                                        3, config->scenario_sub_id, operation, runtime_config->ble_audio_dongle_runtime_config.channel_enable);
                        ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                        break;

                    default:
                        break;
                }
            }
            break;
        default:
            TRANSMITTER_LOG_E("operation %d can not do in BLE scenario id %d. ", 2,
                        operation,
                        config->scenario_sub_id
                        );
            break;
    }
    }
    return ret;
}

audio_transmitter_status_t ble_audio_dongle_get_runtime_config(uint8_t scenario_type, uint8_t scenario_sub_id, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config)
{
    audio_transmitter_status_t ret = AUDIO_TRANSMITTER_STATUS_FAIL;
    uint32_t operation = runtime_config_type;

    if (scenario_type != AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) {
        TRANSMITTER_LOG_I("[BLE Audio Dongle] error scenario_type %d.", 1, scenario_type);
        return ret;
    }

    if ((scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) || (scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1)
#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
    || (scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN)
#endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */
#if defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
    || (scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)
#endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
        ) {
        switch (operation) {
            case BLE_AUDIO_DONGLE_CONFIG_OP_GET_DL_CH1_SHARE_INFO:
                runtime_config->ble_audio_dongle_runtime_config.share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_SEND_TO_AIR_0);
                ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                break;

            case BLE_AUDIO_DONGLE_CONFIG_OP_GET_DL_CH2_SHARE_INFO:
                runtime_config->ble_audio_dongle_runtime_config.share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_SEND_TO_AIR_1);
                ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                break;

            default:
                break;
        }
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT) {
        switch (operation) {
            case BLE_AUDIO_DONGLE_CONFIG_OP_GET_UL_CH1_SHARE_INFO:
                runtime_config->ble_audio_dongle_runtime_config.share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_RECEIVE_FROM_AIR_0);
                ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                break;

            case BLE_AUDIO_DONGLE_CONFIG_OP_GET_UL_CH2_SHARE_INFO:
                runtime_config->ble_audio_dongle_runtime_config.share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_RECEIVE_FROM_AIR_1);
                ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                break;

            default:
                break;
        }
    }

    return ret;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ble_audio_dongle_state_started_handler(uint8_t scenario_sub_id)
{
    uint32_t saved_mask;

    switch (scenario_sub_id) {
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            usb_stream_tx_handle[0].usb_first_in_flag = 0;
            usb_stream_tx_handle[0].stream_is_started = 0;
#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
            usb_stream_tx_handle[0].latency_debug_enable = 0;
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
            ul_stream_status = ul_stream_status | 0x1;
            hal_nvic_restore_interrupt_mask(saved_mask);
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
            USB_Audio_Register_Tx_Callback(0, usb_audio_tx_cb_ble_audio_dongle_0);
            TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_TX_DEBUG 0]Register usb_audio_tx_cb 0", 0);
#endif /* AIR_USB_AUDIO_1_MIC_ENABLE */
            break;

        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
            usb_stream_rx_handle[0].usb_first_in_flag = 0;
            usb_stream_rx_handle[0].stream_is_started = 0;
#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
            usb_stream_rx_handle[0].latency_debug_enable = 0;
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
#if defined(AIR_USB_AUDIO_1_SPK_ENABLE)
            USB_Audio_Register_Rx_Callback(0, usb_audio_rx_cb_ble_audio_dongle_0);
            TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 0]Register usb_audio_rx_cb 0", 0);
#endif /* AIR_USB_AUDIO_1_SPK_ENABLE */
            break;

        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
            usb_stream_rx_handle[1].usb_first_in_flag = 0;
            usb_stream_rx_handle[1].stream_is_started = 0;
#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
            usb_stream_rx_handle[1].latency_debug_enable = 0;
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
            USB_Audio_Register_Rx_Callback(1, usb_audio_rx_cb_ble_audio_dongle_1);
            TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 1]Register usb_audio_rx_cb 1", 0);
#endif /* AIR_USB_AUDIO_2_SPK_ENABLE */
            break;
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
            break;
        default:
            TRANSMITTER_LOG_E("not in ble source dongle scenario sub id list\r\n", 0);
            AUDIO_ASSERT(0);
            break;
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ble_audio_dongle_state_idle_handler(uint8_t scenario_sub_id)
{
    uint32_t saved_mask;

    switch (scenario_sub_id) {
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT:
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
            USB_Audio_Register_Tx_Callback(0, NULL);
            TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_TX_DEBUG 0]Unregister usb_audio_tx_cb 0", 0);
#endif /* AIR_USB_AUDIO_1_MIC_ENABLE */
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            usb_stream_tx_handle[0].usb_first_in_flag = 0;
            usb_stream_tx_handle[0].stream_is_started = 0;
#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
            usb_stream_tx_handle[0].latency_debug_enable = 0;
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
            ul_stream_status = ul_stream_status & 0xfffffffe;
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
#if defined(AIR_USB_AUDIO_1_SPK_ENABLE)
            USB_Audio_Register_Rx_Callback(0, NULL);
            TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 0]Unregister usb_audio_rx_cb 0", 0);
#endif /* AIR_USB_AUDIO_1_SPK_ENABLE */
            usb_stream_rx_handle[0].usb_first_in_flag = 0;
            usb_stream_rx_handle[0].stream_is_started = 0;
#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
            usb_stream_rx_handle[0].latency_debug_enable = 0;
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
            dl_stream_status = dl_stream_status & 0xfffffffe;
            break;

        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
            USB_Audio_Register_Rx_Callback(1, NULL);
            TRANSMITTER_LOG_I("[BLE Audio Dongle][USB_RX_DEBUG 1]Unregister usb_audio_rx_cb 1", 0);
#endif /* AIR_USB_AUDIO_2_SPK_ENABLE */
            usb_stream_rx_handle[1].usb_first_in_flag = 0;
            usb_stream_rx_handle[1].stream_is_started = 0;
#if BLE_AUDIO_DONGLE_DEBUG_LANTENCY
            usb_stream_rx_handle[1].latency_debug_enable = 0;
#endif /* BLE_AUDIO_DONGLE_DEBUG_LANTENCY */
            dl_stream_status = dl_stream_status & 0xfffffffd;
            break;
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
            break;
        default:
            TRANSMITTER_LOG_E("not in ble source dongle scenario sub id list\r\n", 0);
            AUDIO_ASSERT(0);
            break;
    }
}

void ble_audio_dongle_state_starting_handler(uint8_t scenario_sub_id)
{
    switch (scenario_sub_id) {
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
#ifdef HAL_DVFS_MODULE_ENABLED
            /* there is counter in DVFS API, so do not need add counter here */
            #if defined(AIR_BTA_IC_PREMIUM_G2)
            hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_LOCK);
            #else
            hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
            #endif
#endif
            TRANSMITTER_LOG_I("[BLE Audio Dongle] lock cpu to high", 0);
            break;

        default:
            TRANSMITTER_LOG_E("not in ble source dongle scenario sub id list\r\n", 0);
            AUDIO_ASSERT(0);
            break;
    }
}

void ble_audio_dongle_state_stoping_handler(uint8_t scenario_sub_id)
{
    switch (scenario_sub_id) {
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN:
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN:
#ifdef HAL_DVFS_MODULE_ENABLED
            /* there is counter in DVFS API, so do not need add counter here */
            #if defined(AIR_BTA_IC_PREMIUM_G2)
            hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_UNLOCK);
            #else
            hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_UNLOCK);
            #endif
#endif
            TRANSMITTER_LOG_I("[BLE Audio Dongle] unlock cpu to high", 0);
            break;

        default:
            TRANSMITTER_LOG_E("not in ble source dongle scenario sub id list\r\n", 0);
            AUDIO_ASSERT(0);
            break;
    }
}

/******************************************************************************/
/*          BLE audio source dongle silence detection Public Functions        */
/******************************************************************************/
#ifdef AIR_SILENCE_DETECTION_ENABLE
void ble_audio_dongle_silence_detection_enable(audio_scenario_type_t scenario)
{
    audio_transmitter_runtime_config_t config;
    audio_transmitter_status_t ret;
    audio_transmitter_id_t id;
    audio_transmitter_scenario_sub_id_bleaudiodongle_t sub_id;
    bool find_out_id_flag = false;
    uint32_t i;

    /* find out audio transmitter id by audio scenario type */
    sub_id = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0 + (scenario-AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0);
    for (i = 0; i < AUDIO_TRANSMITTER_MAX; i++) {
        if ((g_audio_transmitter_control[i].state == AUDIO_TRANSMITTER_STATE_STARTED) &&
            (g_audio_transmitter_control[i].config.scenario_type == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) &&
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
        ret = audio_transmitter_set_runtime_config(id, BLE_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_ENABLE, &config);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
            TRANSMITTER_LOG_E("[SD][BLE Audio Dongle][ERROR] enable silence detection fail, %d\r\n", 1, scenario);
        }
    }
    else
    {
        TRANSMITTER_LOG_E("[SD][BLE Audio Dongle][ERROR] audio transmitter id is not found, %d\r\n", 1, scenario);
    }
}

void ble_audio_dongle_silence_detection_disable(audio_scenario_type_t scenario)
{
    audio_transmitter_runtime_config_t config;
    audio_transmitter_status_t ret;
    audio_transmitter_id_t id;
    audio_transmitter_scenario_sub_id_bleaudiodongle_t sub_id;
    bool find_out_id_flag = false;
    uint32_t i;

    /* find out audio transmitter id by audio scenario type */
    sub_id = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0 + (scenario-AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0);
    for (i = 0; i < AUDIO_TRANSMITTER_MAX; i++) {
        if ((g_audio_transmitter_control[i].state == AUDIO_TRANSMITTER_STATE_STARTED) &&
            (g_audio_transmitter_control[i].config.scenario_type == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) &&
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
        ret = audio_transmitter_set_runtime_config(id, BLE_AUDIO_DONGLE_CONFIG_OP_SILENCE_DETECTION_DISABLE, &config);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
            TRANSMITTER_LOG_E("[SD][BLE Audio Dongle][ERROR] disable silence detection fail, %d\r\n", 1, scenario);
        }
    }
    else
    {
        TRANSMITTER_LOG_E("[SD][BLE Audio Dongle][ERROR] audio transmitter id is not found, %d\r\n", 1, scenario);
    }
}
#endif /* AIR_SILENCE_DETECTION_ENABLE */

#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
