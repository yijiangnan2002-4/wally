/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

#include "scenario_gaming_mode.h"
#include "audio_transmitter_playback_port.h"
#include "hal_audio.h"
#ifdef AIR_AUDIO_DUMP_ENABLE
#include "audio_dump.h"
#endif
#include "nvkey.h"
#ifdef HAL_DVFS_MODULE_ENABLED
#include "hal_dvfs_internal.h"
#endif
#include "hal_gpt.h"
#include "hal_gpio.h"
#include "scenario_dongle_common.h"

/*------------------------------------------------PORT----AIR_BT_ULTRA_LOW_LATENCY_ENABLE------------------------------------------------------------------*/
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)

#define GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY      1
#ifdef HAL_DVFS_MODULE_ENABLED
#ifdef AIR_BTA_IC_PREMIUM_G2
#define ULL_V1_AUDIO_DVFS_FREQ HAL_DVFS_HIGH_SPEED_208M
#else
#define ULL_V1_AUDIO_DVFS_FREQ HAL_DVFS_OPP_HIGH
#endif
#endif

uint32_t usb_first_in_flag_0 = 0;
uint32_t usb_first_in_flag_1 = 0;
uint16_t gaming_mode_latency_debug_0 = 0;
uint16_t gaming_mode_latency_debug_1 = 0;
#ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
uint32_t g_ull_line_in_default_d_gain = 0;
extern void bt_sink_srv_am_set_volume(bt_sink_srv_am_stream_type_t in_out, bt_sink_srv_audio_setting_vol_info_t *vol_info);
#endif

#ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
#ifdef AIR_BTA_IC_PREMIUM_G2
extern ATTR_TEXT_IN_TCM hal_clock_status_t clock_mux_sel(clock_mux_sel_id mux_id, uint32_t mux_sel);
#endif
#endif

#include "usbaudio_drv.h"
static audio_transmitter_block_header_t usb_stream_header = {0, 0};
static uint32_t g_usb_0_stream_curr_payload_size = 0;
static uint32_t g_usb_1_stream_curr_payload_size = 0;
static uint32_t g_usb_0_stream_avm_buffer_length = 0;
static uint32_t g_usb_1_stream_avm_buffer_length = 0;

//#define DONGLE_PAYLOAD_SIZE_PCM 576//1ms for max 96K/24bit/Stereo
#define DONGLE_PAYLOAD_SIZE_ENCODED 84//2.5ms for 48K/16bit/Stereo/opus + uint32 crc32

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
#define NVKEYID_E450            "E450"
static const uint8_t NVKEY_E450[] = {
    0x01, 0x00, 0x00, 0x00, 0x67, 0xF3, 0xFF, 0xFF, 0xC8, 0x00, 0x00, 0x00, 0x67, 0xF3, 0xFF, 0xFF, 0xD0, 0x07, 0x00, 0x00, 0x18, 0xFC, 0xFF, 0xFF, 0x19, 0x00, 0x00, 0x00, 0xE7, 0xFF, 0xFF, 0xFF, 0x91, 0x87, 0x08, 0x00, 0xCB, 0xBF, 0x04, 0x00, 0x90, 0x87, 0x08, 0x00, 0xE8, 0x9F, 0xB8, 0xFF, 0x25, 0x29, 0x32, 0x00, 0x2E, 0xC5, 0x42, 0x00, 0x5F, 0xC7, 0xF3, 0xFF, 0x2D, 0xC5, 0x42, 0x00, 0x29, 0x00, 0xB8, 0xFF, 0x29, 0x36, 0x64, 0x00, 0x00, 0x00, 0x40, 0x00, 0xC9, 0x7E, 0x74, 0x00, 0xCF, 0x84, 0x8B, 0xFF, 0xC8, 0x7E, 0x74, 0x00, 0xA6, 0x23, 0x88, 0xFF, 0x71, 0xA4, 0x70, 0x00, 0xD0, 0xEA, 0x7E, 0x00, 0x16, 0x29, 0x81, 0xFF, 0xCF, 0xEA, 0x7E, 0x00, 0xDF, 0x73, 0x81, 0xFF, 0x85, 0x91, 0x7D, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x1E, 0x00, 0x06, 0x00, 0xB0, 0x0A, 0x0C, 0x00, 0x00, 0x00, 0x00,
};
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

#if defined(AIR_USB_AUDIO_1_SPK_ENABLE)
static void usb_audio_rx_cb_gaming_dongle_0(void)
{
    static uint32_t previous_count = 0;
    uint32_t gpt_count, duration_count;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    n9_dsp_share_info_t *p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_DSP_RECEIVE_FROM_MCU_0);

    uint32_t buf_size = 0;
    uint8_t *p_source_buf = NULL;

    uint32_t available_data_size = USB_Audio_Get_Len_Received_Data(0);

    if (usb_first_in_flag_0 == 0) {
        USB_Audio_Rx_Buffer_Drop_Bytes(0, available_data_size);
        usb_first_in_flag_0 = 1;
        previous_count = gpt_count;
        TRANSMITTER_LOG_I("[USB_RX_DEBUG 0] 1st USB IRQ is triggered!!! drop %d bytes", 1, available_data_size);
        return;
    }

    hal_gpt_get_duration_count(previous_count, gpt_count, &duration_count);
    if (duration_count > 2000) {
        TRANSMITTER_LOG_I("[USB_RX_DEBUG 0]usb_audio_rx_cb_gaming_dongle_0 duration = %d", 1, duration_count);
    }
    previous_count = gpt_count;

    if (available_data_size > g_usb_0_stream_curr_payload_size) {
        TRANSMITTER_LOG_I("[USB_RX_DEBUG 0]Too much data in USB buffer %d %d\r\n", 2, available_data_size, g_usb_0_stream_curr_payload_size);
    }

    while (available_data_size >= g_usb_0_stream_curr_payload_size) {
        hal_audio_buf_mgm_get_free_buffer(p_dsp_info, &p_source_buf, &buf_size);
        if (buf_size < (g_usb_0_stream_curr_payload_size + BLK_HEADER_SIZE)) {
            TRANSMITTER_LOG_I("[USB_RX_DEBUG 0]Not enough share buffer space", 0);
            //AUDIO_ASSERT(0);
            break;
        }

        usb_stream_header.sequence_number++;
        usb_stream_header.data_length = g_usb_0_stream_curr_payload_size;

        memcpy(p_source_buf, &usb_stream_header, BLK_HEADER_SIZE);
        USB_Audio_Read_Data(0, p_source_buf + BLK_HEADER_SIZE, g_usb_0_stream_curr_payload_size);
#ifdef AIR_AUDIO_DUMP_ENABLE
        //LOG_AUDIO_DUMP(p_source_buf + BLK_HEADER_SIZE, g_usb_0_stream_curr_payload_size, SOURCE_IN4);
#endif
#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY
        audio_usb_rx_scenario_latency_debug(0, p_source_buf + BLK_HEADER_SIZE);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY */
        hal_audio_buf_mgm_get_write_data_done(p_dsp_info, g_usb_0_stream_curr_payload_size + BLK_HEADER_SIZE); /* always keep same block size */

#if 0
        hal_ccni_message_t ccni_msg;
        ccni_msg.ccni_message[0] = usb_stream_header.sequence_number;
        ccni_msg.ccni_message[1] = 0;
        for (uint32_t i = 0 ; (hal_ccni_set_event(CCNI_CM4_TO_DSP0_EVENT2, &ccni_msg)) != HAL_CCNI_STATUS_OK ; i++) {
            if ((i % 1000) == 0) {
                log_hal_msgid_info("[USB_RX_DEBUG]Send message waiting %d\r\n", 1, (int)i);
            }
        }
#endif

        available_data_size = USB_Audio_Get_Len_Received_Data(0);
    }
}
#endif /* AIR_USB_AUDIO_1_SPK_ENABLE */

#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
static void usb_audio_rx_cb_gaming_dongle_1(void)
{
    static uint32_t previous_count = 0;
    uint32_t gpt_count, duration_count;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    n9_dsp_share_info_t *p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_DSP_RECEIVE_FROM_MCU_1);

    uint32_t buf_size = 0;
    uint8_t *p_source_buf = NULL;

    uint32_t available_data_size = USB_Audio_Get_Len_Received_Data(1);

    if (usb_first_in_flag_1 == 0) {
        USB_Audio_Rx_Buffer_Drop_Bytes(1, available_data_size);
        usb_first_in_flag_1 = 1;
        previous_count = gpt_count;
        return;
    }

    hal_gpt_get_duration_count(previous_count, gpt_count, &duration_count);
    if (duration_count > 2000) {
        TRANSMITTER_LOG_I("[USB_RX_DEBUG 1]usb_audio_rx_cb_gaming_dongle_1 duration = %d", 1, duration_count);
    }
    previous_count = gpt_count;

    if (available_data_size > g_usb_1_stream_curr_payload_size) {
        log_hal_msgid_info("[USB_RX_DEBUG 1]Too much data in USB buffer %d\r\n", 2, available_data_size, g_usb_1_stream_curr_payload_size);
    }

    while (available_data_size >= g_usb_1_stream_curr_payload_size) {
        hal_audio_buf_mgm_get_free_buffer(p_dsp_info, &p_source_buf, &buf_size);
        if (buf_size < (g_usb_1_stream_curr_payload_size + BLK_HEADER_SIZE)) {
            TRANSMITTER_LOG_I("[USB_RX_DEBUG 1]Not enough share buffer space", 0);
            break;
        }

        usb_stream_header.sequence_number++;
        usb_stream_header.data_length = g_usb_1_stream_curr_payload_size;

        memcpy(p_source_buf, &usb_stream_header, BLK_HEADER_SIZE);
        USB_Audio_Read_Data(1, p_source_buf + BLK_HEADER_SIZE, g_usb_1_stream_curr_payload_size);
#ifdef AIR_AUDIO_DUMP_ENABLE
        //LOG_AUDIO_DUMP(p_source_buf + BLK_HEADER_SIZE, g_usb_1_stream_curr_payload_size, SOURCE_IN5);
#endif
#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY
        audio_usb_rx_scenario_latency_debug(1, p_source_buf + BLK_HEADER_SIZE);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY */
        hal_audio_buf_mgm_get_write_data_done(p_dsp_info, g_usb_1_stream_curr_payload_size + BLK_HEADER_SIZE);

#if 0
        hal_ccni_message_t ccni_msg;
        ccni_msg.ccni_message[0] = usb_stream_header.sequence_number;
        ccni_msg.ccni_message[1] = 0;
        for (uint32_t i = 0 ; (hal_ccni_set_event(CCNI_CM4_TO_DSP0_EVENT2, &ccni_msg)) != HAL_CCNI_STATUS_OK ; i++) {
            if ((i % 1000) == 0) {
                log_hal_msgid_info("[USB_RX_DEBUG]Send message waiting %d\r\n", 1, (int)i);
            }
        }
#endif

        available_data_size = USB_Audio_Get_Len_Received_Data(1);
    }
}
#endif /* AIR_USB_AUDIO_2_SPK_ENABLE */

uint32_t gaming_mode_opus_codec_version = 0; // 0 - default codec

void gaming_mode_codec_configure(audio_dsp_codec_type_t codec_type, void *codec_param, uint32_t codec_param_size)
{
    if (codec_type == AUDIO_DSP_CODEC_TYPE_OPUS)
    {
        if (codec_param_size == 0)
        {
            gaming_mode_opus_codec_version = (uint32_t)codec_param;
            TRANSMITTER_LOG_E("[ULL][config] codec version is 0x%x\r\n", 1, gaming_mode_opus_codec_version);
        }
    }
    else
    {
        TRANSMITTER_LOG_E("[ULL][ERROR] codec type is not support, %d\r\n", 1, codec_type);
    }
}

void gaming_mode_dongle_set_stream_out_bt_common(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    open_param->param.stream_out = STREAM_OUT_BT_COMMON;
    open_param->stream_out_param.bt_common.scenario_type = config->scenario_type;
    open_param->stream_out_param.bt_common.scenario_sub_id = config->scenario_sub_id;
    open_param->stream_out_param.bt_common.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_BT_SEND_TO_AIR);
    open_param->stream_out_param.bt_common.share_info_type = SHARE_BUFFER_INFO_TYPE;
    open_param->stream_out_param.bt_common.data_notification_frequency = 1;
    open_param->stream_out_param.bt_common.max_payload_size = DONGLE_PAYLOAD_SIZE_ENCODED;
    open_param->stream_out_param.bt_common.scenario_param.gaming_mode_param.codec_type = AUDIO_DSP_CODEC_TYPE_OPUS;
    open_param->stream_out_param.bt_common.scenario_param.gaming_mode_param.codec_param.opus.version = gaming_mode_opus_codec_version;
    ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->read_offset = 0;
    ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->write_offset = 0;
    ((n9_dsp_share_info_t *)(open_param->stream_out_param.bt_common.p_share_info))->bBufferIsFull = false;
    audio_transmitter_modify_share_info_by_block(open_param->stream_out_param.bt_common.p_share_info, DONGLE_PAYLOAD_SIZE_ENCODED);
}

void gaming_mode_dongle_set_stream_in_bt_common(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    open_param->param.stream_in = STREAM_IN_BT_COMMON;
    open_param->stream_in_param.bt_common.scenario_type = config->scenario_type;
    open_param->stream_in_param.bt_common.scenario_sub_id = config->scenario_sub_id;
    open_param->stream_in_param.bt_common.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_BT_RECEIVE_FROM_AIR);
    open_param->stream_in_param.bt_common.share_info_type = AVM_SHARE_BUF_INFO_TYPE;
    open_param->stream_in_param.bt_common.data_notification_frequency = 0;
    open_param->stream_in_param.bt_common.max_payload_size = 240;
    open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.period = 7500;
    open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.codec_type = AUDIO_DSP_CODEC_TYPE_OPUS;
    //open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.codec_param.opus.sample_rate = 0x8;
    if ((config->scenario_config.gaming_mode_config.voice_dongle_config.codec_param.opus.bit_rate != 32000) && (config->scenario_config.gaming_mode_config.voice_dongle_config.codec_param.opus.bit_rate != 50133)) {
        TRANSMITTER_LOG_E("Error codec bitrate, %u\r\n", 1, config->scenario_config.gaming_mode_config.voice_dongle_config.codec_param.opus.bit_rate);
        AUDIO_ASSERT(0);
    }
    open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.codec_param.opus.bit_rate = config->scenario_config.gaming_mode_config.voice_dongle_config.codec_param.opus.bit_rate;
    open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.codec_param.opus.version = gaming_mode_opus_codec_version;
    //open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.codec_param.opus.bit_rate = 0x0;
    //open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.codec_param.opus.channel_mode = 0x8;
    /*share buffer modify by bt host, AVM buffer info*/
    //audio_transmitter_modify_share_info_by_block(open_param->stream_in_param.bt_common.p_share_info, 240);
    /*data write by bt controller*/
    //write_info = open_param->stream_in_param.bt_common.p_share_info;
}

void gaming_mode_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    memset(&usb_stream_header, 0, sizeof(audio_transmitter_block_header_t));
    if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET) {

        sysram_status_t     status;
        DSP_FEATURE_TYPE_LIST AudioFeatureList_GameVoiceHead[2] = {
            FUNC_GAMING_HEADSET,
            FUNC_END,
        };
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
        voice_mic_type_t mic_cur_type = hal_audio_query_voice_mic_type();
        TRANSMITTER_LOG_I("GamingVoiceHead mic_cur_type: 0x%x", 1, mic_cur_type);
        if (mic_cur_type == VOICE_MIC_TYPE_FIXED) {
            AudioFeatureList_GameVoiceHead[0] = FUNC_GAMING_HEADSET;
        } else if (mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
            AudioFeatureList_GameVoiceHead[0] = FUNC_GAMING_BOOM_MIC;
        } else {
            TRANSMITTER_LOG_E("GamingVoiceHead no this mic type - (%d)\r\n", 1, mic_cur_type);
            AUDIO_ASSERT(0);
        }
#endif

        /* reset share buffer before put parameters*/
        audio_nvdm_reset_sysram();
        status = audio_nvdm_set_feature(2, AudioFeatureList_GameVoiceHead);
        if (status != NVDM_STATUS_NAT_OK) {
            TRANSMITTER_LOG_E("GamingVoiceHead is failed to set parameters to share memory - err(%d)\r\n", 1, status);
            AUDIO_ASSERT(0);
        }

        open_param->param.stream_in = STREAM_IN_AFE;
        hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &open_param->stream_in_param);
        //open_param->stream_in_param.afe.audio_device    = HAL_AUDIO_DEVICE_MAIN_MIC_L;
        //open_param->stream_in_param.afe.stream_channel  = HAL_AUDIO_DIRECT;
        open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM1 | HAL_AUDIO_MEM3; //mic in & echo ref
        //open_param->stream_in_param.afe.audio_interface = HAL_AUDIO_INTERFACE_1;
        open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE;
#ifdef AIR_UL_FIX_SAMPLING_RATE_48K
        open_param->stream_in_param.afe.sampling_rate   = 48000;
#else
        open_param->stream_in_param.afe.sampling_rate   = 16000;
#endif
#if defined(AIR_ULL_VOICE_LOW_LATENCY_ENABLE)
        open_param->stream_in_param.afe.irq_period      = 7.5;
#ifdef AIR_UL_FIX_SAMPLING_RATE_48K
        open_param->stream_in_param.afe.frame_size      = 360;
#else
        open_param->stream_in_param.afe.frame_size      = 120;
#endif
#else
        open_param->stream_in_param.afe.irq_period      = 15;
#ifdef AIR_UL_FIX_SAMPLING_RATE_48K
        open_param->stream_in_param.afe.frame_size      = 720;
#else
        open_param->stream_in_param.afe.frame_size      = 240;
#endif
#endif
        open_param->stream_in_param.afe.frame_number    = 6;
        open_param->stream_in_param.afe.hw_gain         = false;
        open_param->stream_in_param.afe.misc_parms      = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_2p4v;

        open_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;
        open_param->stream_out_param.data_ul.scenario_type = config->scenario_type;
        open_param->stream_out_param.data_ul.scenario_sub_id = config->scenario_sub_id;
        open_param->stream_out_param.data_ul.data_notification_frequency = 0;

        if ((config->scenario_config.gaming_mode_config.voice_headset_config.codec_param.opus.bit_rate != 32000) && (config->scenario_config.gaming_mode_config.voice_headset_config.codec_param.opus.bit_rate != 50133)) {
            TRANSMITTER_LOG_E("Error codec bitrate, %u\r\n", 1, config->scenario_config.gaming_mode_config.voice_headset_config.codec_param.opus.bit_rate);
            AUDIO_ASSERT(0);
        }
        if (config->scenario_config.gaming_mode_config.voice_headset_config.codec_param.opus.bit_rate != 32000) {
            open_param->stream_out_param.data_ul.max_payload_size = 52; // header:4 + payload:30
        } else {
            open_param->stream_out_param.data_ul.max_payload_size = 36; // header:4 + payload:47
        }
        open_param->stream_out_param.data_ul.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_BT_SEND_TO_AIR);
        //hal_audio_reset_share_info(open_param->stream_out_param.data_ul.p_share_info);
        open_param->stream_out_param.data_ul.p_share_info->read_offset = 0;
        open_param->stream_out_param.data_ul.p_share_info->write_offset = 0;
        open_param->stream_out_param.data_ul.p_share_info->bBufferIsFull = false;

        if (config->scenario_config.gaming_mode_config.voice_headset_config.codec_param.opus.bit_rate != 32000) {
            open_param->stream_out_param.data_ul.p_share_info->sub_info.block_info.block_size = 52; // header:4 + payload:47
            open_param->stream_out_param.data_ul.p_share_info->sub_info.block_info.block_num = open_param->stream_out_param.data_ul.p_share_info->length / 52;
        } else {
            open_param->stream_out_param.data_ul.p_share_info->sub_info.block_info.block_size = 36; // header:4 + payload:47
            open_param->stream_out_param.data_ul.p_share_info->sub_info.block_info.block_num = open_param->stream_out_param.data_ul.p_share_info->length / 36;
        }
        open_param->stream_out_param.data_ul.p_share_info->length = open_param->stream_out_param.data_ul.p_share_info->sub_info.block_info.block_size * open_param->stream_out_param.data_ul.p_share_info->sub_info.block_info.block_num;
        //memset((void *)open_param->stream_out_param.data_ul.p_share_info->start_addr, 0, open_param->stream_out_param.data_ul.p_share_info->length);

    } else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) || (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1)) {
        audio_src_srv_report("[Game mode music dongle] USB_IN %d, config: %d, %d", 3,
                                config->scenario_sub_id,
                                config->scenario_config.gaming_mode_config.music_dongle_config.usb_param.pcm.sample_rate,
                                config->scenario_config.gaming_mode_config.music_dongle_config.usb_param.pcm.format);

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
        if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
            /* prepare game chat balance feature's NVKEY */
            sysram_status_t status;
            nvkey_status_t nvdm_status;
            DSP_FEATURE_TYPE_LIST AudioFeatureList_GameChatBalance[] = {
                FUNC_GAME_CHAT_VOLUME_SMART_BALANCE,
                FUNC_END,
            };

            /* reset share buffer before put parameters*/
            audio_nvdm_reset_sysram();
            status = NVDM_STATUS_ERROR;
            while (status != NVDM_STATUS_NAT_OK) {
                /* set NVKEYs that the usb chat stream uses into the share buffer */
                status = audio_nvdm_set_feature(sizeof(AudioFeatureList_GameChatBalance) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_GameChatBalance);
                if (status != NVDM_STATUS_NAT_OK) {
                    audio_src_srv_report("[Game/Chat Balance] failed to set parameters to share memory - err(%d)\r\n", 1, status);

                    /* workaround for that maybe this NVKEY_E450 is not exsited after the FOTA */
                    if ((nvdm_status_t)status == NVDM_STATUS_ITEM_NOT_FOUND) {
                        audio_src_srv_report("[Game/Chat Balance] set default parameters into the NVKEY\r\n", 0);
                        nvdm_status = nvkey_write_data(NVID_DSP_ALG_GC_VOL_SMART_BAL, (const uint8_t *)&NVKEY_E450[0], sizeof(NVKEY_E450));
                        if (nvdm_status != NVKEY_STATUS_OK) {
                            audio_src_srv_report("[Game/Chat Balance] failed to set default parameters into the NVKEY - err(%d)\r\n", 1, nvdm_status);
                            AUDIO_ASSERT(0);
                        }
                    } else {
                        AUDIO_ASSERT(0);
                    }
                }
            }
        }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

        open_param->param.stream_in = STREAM_IN_AUDIO_TRANSMITTER;
        open_param->stream_in_param.data_dl.scenario_type = config->scenario_type;
        open_param->stream_in_param.data_dl.scenario_sub_id = config->scenario_sub_id;
        if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
            open_param->stream_in_param.data_dl.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_DSP_RECEIVE_FROM_MCU_0);
            if (g_usb_0_stream_avm_buffer_length == 0) {
                g_usb_0_stream_avm_buffer_length = open_param->stream_in_param.data_dl.p_share_info->length; /* Save the length of AVM buffer */
            } else {
                open_param->stream_in_param.data_dl.p_share_info->length = g_usb_0_stream_avm_buffer_length; /* Reset the length of AVM buffer */
            }
        } else {
            open_param->stream_in_param.data_dl.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_DSP_RECEIVE_FROM_MCU_1);
            if (g_usb_1_stream_avm_buffer_length == 0) {
                g_usb_1_stream_avm_buffer_length = open_param->stream_in_param.data_dl.p_share_info->length; /* Save the length of AVM buffer */
            } else {
                open_param->stream_in_param.data_dl.p_share_info->length = g_usb_1_stream_avm_buffer_length; /* Reset the length of AVM buffer */
            }
        }
        open_param->stream_in_param.data_dl.data_notification_frequency = 0;
        open_param->stream_in_param.data_dl.scenario_param.gaming_mode_param.period = 2500;
        open_param->stream_in_param.data_dl.scenario_param.gaming_mode_param.codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
        open_param->stream_in_param.data_dl.scenario_param.gaming_mode_param.codec_param.pcm.sample_rate = config->scenario_config.gaming_mode_config.music_dongle_config.usb_param.pcm.sample_rate;
        open_param->stream_in_param.data_dl.scenario_param.gaming_mode_param.codec_param.pcm.format = config->scenario_config.gaming_mode_config.music_dongle_config.usb_param.pcm.format;
        if (open_param->stream_in_param.data_dl.scenario_param.gaming_mode_param.codec_param.pcm.format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
            open_param->stream_in_param.data_dl.max_payload_size = (open_param->stream_in_param.data_dl.scenario_param.gaming_mode_param.codec_param.pcm.sample_rate / 1000) * 3 * 2;
        } else {
            open_param->stream_in_param.data_dl.max_payload_size = (open_param->stream_in_param.data_dl.scenario_param.gaming_mode_param.codec_param.pcm.sample_rate / 1000) * 2 * 2;
        }
        open_param->stream_in_param.data_dl.p_share_info->read_offset = 0;
        open_param->stream_in_param.data_dl.p_share_info->write_offset = 0;
        open_param->stream_in_param.data_dl.p_share_info->bBufferIsFull = false;
        audio_transmitter_modify_share_info_by_block(open_param->stream_in_param.data_dl.p_share_info, open_param->stream_in_param.data_dl.max_payload_size);
        usb_first_in_flag_0 = 0;
        usb_first_in_flag_1 = 0;
        if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
            g_usb_0_stream_curr_payload_size = open_param->stream_in_param.data_dl.max_payload_size;
        } else {
            g_usb_1_stream_curr_payload_size = open_param->stream_in_param.data_dl.max_payload_size;
        }

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY
        audio_usb_rx_scenario_latency_debug_init(config->scenario_sub_id - AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0,
                                                    open_param->stream_in_param.data_dl.max_payload_size,
                                                    config->scenario_config.gaming_mode_config.music_dongle_config.usb_param.pcm.channel_mode,
                                                    config->scenario_config.gaming_mode_config.music_dongle_config.usb_param.pcm.format);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY */

        uint32_t gain = audio_get_gain_out_in_dB(0, GAIN_DIGITAL, VOL_USB_AUDIO_SW_IN);
        uint32_t gain_default = gain;
        uint8_t volume_level = 0;//config->scenario_config.gaming_mode_config.voice_dongle_config.volume_level_default_L;
        if (volume_level <= bt_sink_srv_ami_get_usb_music_sw_max_volume_level()) {
            gain = audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_AUDIO_SW_IN);
        }
        open_param->stream_in_param.data_dl.scenario_param.gaming_mode_param.gain_default_L = gain;
        gain = gain_default;
        volume_level = 0;//config->scenario_config.gaming_mode_config.voice_dongle_config.volume_level_default_R;
        if (volume_level <= bt_sink_srv_ami_get_usb_music_sw_max_volume_level()) {
            gain = audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_AUDIO_SW_IN);
        }
        open_param->stream_in_param.data_dl.scenario_param.gaming_mode_param.gain_default_R = gain;

        /* configure stream sink */
        gaming_mode_dongle_set_stream_out_bt_common(config, open_param);

        /*share buffer modify by bt host*/
        //audio_transmitter_modify_share_info_by_block(open_param->stream_out_param.data_ul.p_share_info, 240);
        /*data read by bt controller*/
        //read_info = open_param->stream_out_param.data_ul.p_share_info;
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) {
#ifdef AIR_ECNR_POST_PART_ENABLE
        sysram_status_t status;
        DSP_FEATURE_TYPE_LIST AudioFeatureList_GameModeUsbDongle[] = {
            FUNC_TX_POST_ECNR,
            FUNC_END,
        };
        audio_nvdm_reset_sysram();
        status = audio_nvdm_set_feature(sizeof(AudioFeatureList_GameModeUsbDongle) / sizeof(DSP_FEATURE_TYPE_LIST), AudioFeatureList_GameModeUsbDongle);
        if (status != NVDM_STATUS_NAT_OK) {
            audio_src_srv_report("[Game mode music dongle] failed to set parameters to share memory - err(%d)\r\n", 1, status);
            configASSERT(0);
        }
        audio_src_srv_report("[Game mode music dongle] load ecnr nvkey done", 0);
#endif

        /* configure stream source */
        gaming_mode_dongle_set_stream_in_bt_common(config, open_param);
        uint32_t gain = audio_get_gain_in_in_dB(0, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
        uint32_t gain_default = gain;
        uint8_t volume_level = 0;//config->scenario_config.gaming_mode_config.voice_dongle_config.volume_level_default_L;
        if (volume_level <= bt_sink_srv_ami_get_usb_voice_sw_max_volume_level()) {
            gain = audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
        }
        open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.gain_default_L = gain;
        gain = gain_default;
        volume_level = 0;//config->scenario_config.gaming_mode_config.voice_dongle_config.volume_level_default_R;
        if (volume_level <= bt_sink_srv_ami_get_usb_voice_sw_max_volume_level()) {
            gain = audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
        }
        open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.gain_default_R = gain;

        open_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;
        open_param->stream_out_param.data_ul.scenario_type = config->scenario_type;
        open_param->stream_out_param.data_ul.scenario_sub_id = config->scenario_sub_id;
        open_param->stream_out_param.data_ul.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_DSP_SEND_TO_MCU);
        open_param->stream_out_param.data_ul.data_notification_frequency = 0;
        open_param->stream_out_param.data_ul.max_payload_size = 1440;
        open_param->stream_out_param.data_ul.scenario_param.gaming_mode_param.codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
        if ((config->scenario_config.gaming_mode_config.voice_dongle_config.usb_param.pcm.sample_rate != 16000) && (config->scenario_config.gaming_mode_config.voice_dongle_config.usb_param.pcm.sample_rate != 48000)) {
            TRANSMITTER_LOG_E("Error usb sample rate, %u\r\n", 1, config->scenario_config.gaming_mode_config.voice_dongle_config.usb_param.pcm.sample_rate);
            AUDIO_ASSERT(0);
        }
        open_param->stream_out_param.data_ul.scenario_param.gaming_mode_param.codec_param.pcm.sample_rate = config->scenario_config.gaming_mode_config.voice_dongle_config.usb_param.pcm.sample_rate;
        //hal_audio_reset_share_info(open_param->stream_out_param.data_ul.p_share_info);
        //memset((void *)open_param->stream_out_param.data_ul.p_share_info->start_addr, 0, open_param->stream_out_param.data_ul.p_share_info->length);
        open_param->stream_out_param.data_ul.p_share_info->read_offset = 0;
        open_param->stream_out_param.data_ul.p_share_info->write_offset = 0;
        open_param->stream_out_param.data_ul.p_share_info->bBufferIsFull = false;
        /*share buffer modify by transmitter, n9_dsp buffer info*/
        audio_transmitter_modify_share_info_by_block(open_param->stream_out_param.data_ul.p_share_info, open_param->stream_out_param.data_ul.max_payload_size);
        /*data read by bt bt host, read by transmitter read API*/
        //*read_info = open_param->stream_out_param.data_ul.p_share_info;
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN) {
#ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
        /* Source setting */
        TRANSMITTER_LOG_I("[ULL][Line In] avm play", 0);
        open_param->param.stream_in = STREAM_IN_AFE;
        open_param->stream_in_param.afe.audio_device        = HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL;
        open_param->stream_in_param.afe.stream_channel      = HAL_AUDIO_DIRECT;
        open_param->stream_in_param.afe.memory              = HAL_AUDIO_MEM1;
        open_param->stream_in_param.afe.format              = HAL_AUDIO_PCM_FORMAT_S16_LE;
        hal_audio_get_stream_in_setting_config(AU_DSP_LINEIN, &open_param->stream_in_param);
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        open_param->stream_in_param.afe.audio_interface = hal_audio_convert_linein_interface(audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_Path,true);
        open_param->stream_in_param.afe.performance = audio_nvdm_HW_config.audio_scenario.Audio_Analog_LineIn_Performance_Sel;
        open_param->stream_in_param.afe.iir_filter[0] = audio_nvdm_HW_config.adc_dac_config.ADDA_Audio_IIR_Filter;
        #if defined(AIR_BTA_IC_PREMIUM_G2)
        open_param->stream_in_param.afe.ul_adc_mode[0] = audio_nvdm_HW_config.adc_dac_config.amic_config[0].ADDA_Analog_MIC_Mode;
        #else
        open_param->stream_in_param.afe.ul_adc_mode[0] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC0_Mode;
        #endif
#else
        open_param->stream_in_param.afe.audio_interface = HAL_AUDIO_INTERFACE_1;
#endif
        if (open_param->stream_in_param.afe.audio_device & HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL) {
            open_param->stream_in_param.afe.misc_parms      = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_2p4v;
        } else {
            open_param->stream_in_param.afe.misc_parms      = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_1p85v;
        }
        open_param->stream_in_param.afe.sampling_rate       = 48000;
        open_param->stream_in_param.afe.frame_size          = 120;
        open_param->stream_in_param.afe.frame_number        = 8;
        open_param->stream_in_param.afe.irq_period          = 0;
        open_param->stream_in_param.afe.hw_gain             = false;
        // open_param->stream_in_param.afe.iir_filter[0]    = HAL_AUDIO_UL_IIR_DISABLE;
        #ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        #ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        open_param->stream_in_param.afe.audio_device1       = HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL;
        open_param->stream_in_param.afe.audio_interface1    = hal_audio_convert_linein_interface(audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_Path,true);
        #endif
        bt_sink_srv_audio_setting_vol_info_t vol_info;
        memset(&vol_info, 0, sizeof(bt_sink_srv_audio_setting_vol_info_t));
        vol_info.type = VOL_LINE_IN;
        vol_info.vol_info.lineIN_vol_info.dev_out = HAL_AUDIO_DEVICE_HEADSET;
        vol_info.vol_info.lineIN_vol_info.dev_in  = HAL_AUDIO_DEVICE_MAIN_MIC;
        vol_info.vol_info.lineIN_vol_info.lev_in  = 15; // not care digital gain, only need analog gain
        bt_sink_srv_am_set_volume(STREAM_IN,  &vol_info);
        g_ull_line_in_default_d_gain = audio_get_gain_in_in_dB(15, GAIN_DIGITAL, VOL_LINE_IN);
        #endif
        /* Sink setting */
        gaming_mode_dongle_set_stream_out_bt_common(config, open_param);
#else
    TRANSMITTER_LOG_E("[ULL][Line In] Line in feature option is not enable", 0);
    configASSERT(0);
#endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE */
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
        TRANSMITTER_LOG_I("[ULL][Line Out] avm play", 0);
        // BT Parameter
        /* configure stream source */
        gaming_mode_dongle_set_stream_in_bt_common(config, open_param);
        uint32_t gain = audio_get_gain_in_in_dB(0, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
        //uint32_t gain = audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_LINE_IN);
        uint32_t gain_default=gain;
        uint8_t volume_level = 0;//config->scenario_config.gaming_mode_config.voice_dongle_config.volume_level_default_L;
        if(volume_level <= bt_sink_srv_ami_get_usb_voice_sw_max_volume_level()){
            gain = audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
        }
        open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.gain_default_L = gain;
        gain = gain_default;
        volume_level = 0;//config->scenario_config.gaming_mode_config.voice_dongle_config.volume_level_default_R;
        if(volume_level <= bt_sink_srv_ami_get_usb_voice_sw_max_volume_level()){
            gain = audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
        }
        open_param->stream_in_param.bt_common.scenario_param.gaming_mode_param.gain_default_R = gain;

        // AFE Parameter
        open_param->param.stream_out = STREAM_OUT_AFE;
        open_param->stream_out_param.afe.memory = HAL_AUDIO_MEM1;
        open_param->stream_out_param.afe.format = HAL_AUDIO_PCM_FORMAT_S16_LE;
        open_param->stream_out_param.afe.stream_out_sampling_rate = 16000;
        open_param->stream_out_param.afe.sampling_rate = 48000;
        open_param->stream_out_param.afe.irq_period      = 5;
        open_param->stream_out_param.afe.frame_size      = 480;
        open_param->stream_out_param.afe.frame_number    = 4;
        open_param->stream_out_param.afe.hw_gain         = false;
        open_param->stream_out_param.afe.audio_device    = HAL_AUDIO_DEVICE_DAC_DUAL;
#else
    TRANSMITTER_LOG_E("[ULL][Line Out] Line out feature option is not enable", 0);
    configASSERT(0);
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_I2S_IN) {
#ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
        /* Source setting */
        TRANSMITTER_LOG_I("[ULL][I2S In] avm play device 0x%x interface %d sample rate %d i2s format %d i2s word length %d", 5,
            config->scenario_config.gaming_mode_config.i2s_in_dongle_config.audio_device,
            config->scenario_config.gaming_mode_config.i2s_in_dongle_config.audio_interface,
            config->scenario_config.gaming_mode_config.i2s_in_dongle_config.codec_param.pcm.sample_rate,
            config->scenario_config.gaming_mode_config.i2s_in_dongle_config.i2s_fromat,
            config->scenario_config.gaming_mode_config.i2s_in_dongle_config.i2s_word_length);
        open_param->param.stream_in = STREAM_IN_AFE;
        hal_audio_get_stream_in_setting_config(AU_DSP_LINEIN, &open_param->stream_in_param);
        open_param->stream_in_param.afe.audio_device        = config->scenario_config.gaming_mode_config.i2s_in_dongle_config.audio_device;
        open_param->stream_in_param.afe.stream_channel      = HAL_AUDIO_DIRECT;
        #ifdef AIR_GAMING_MODE_DONGLE_AFE_ENABLE
        open_param->stream_in_param.afe.memory              = HAL_AUDIO_MEM1;
        #else
        open_param->stream_in_param.afe.memory              = HAL_AUDIO_MEM2;
        #endif

        open_param->stream_in_param.afe.format              = HAL_AUDIO_PCM_FORMAT_S32_LE; //config->scenario_config.gaming_mode_config.i2s_in_dongle_config.codec_param.pcm.format; // HAL_AUDIO_PCM_FORMAT_S16_LE;
        open_param->stream_in_param.afe.audio_interface     = config->scenario_config.gaming_mode_config.i2s_in_dongle_config.audio_interface;
        open_param->stream_out_param.afe.misc_parms         = I2S_CLK_SOURCE_DCXO;

        #ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        open_param->stream_in_param.afe.audio_device1       = config->scenario_config.gaming_mode_config.i2s_in_dongle_config.audio_device;
        open_param->stream_in_param.afe.audio_interface1    = config->scenario_config.gaming_mode_config.i2s_in_dongle_config.audio_interface;
        #endif
        /* I2S SLAVE IN DMA Mode */
        if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
            #ifdef AIR_BTA_IC_PREMIUM_G2
                clock_mux_sel(CLK_AUD_GPSRC_SEL, 2); // boost hwsrc convert speed
            #endif
            #if 0
            open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM6; // DMA
            #elif 1
            open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM1; // DMA
            #else
            open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM7; // DMA+TDM
            #endif
        }
        /* hwsrc output rate */
        open_param->stream_in_param.afe.stream_out_sampling_rate = 48000; // ULL Dongle only support 48K in stream.
        /* hwsrc input rate */
        open_param->stream_in_param.afe.sampling_rate       = config->scenario_config.gaming_mode_config.i2s_in_dongle_config.codec_param.pcm.sample_rate;
        open_param->stream_in_param.afe.frame_size          = open_param->stream_in_param.afe.stream_out_sampling_rate * 10 / 4 / 1000; // 2.5 ms 120(48KHz)
        open_param->stream_in_param.afe.frame_number        = 8;
        open_param->stream_in_param.afe.irq_period          = 0;
        open_param->stream_in_param.afe.hw_gain             = false;

        // I2S setting
        open_param->stream_in_param.afe.i2s_format          = config->scenario_config.gaming_mode_config.i2s_in_dongle_config.i2s_fromat;
        open_param->stream_in_param.afe.i2s_word_length     = config->scenario_config.gaming_mode_config.i2s_in_dongle_config.i2s_word_length;
        /* cover 96K */
        //hal_audio_dsp_hwsrc_clkmux_control(96000, true);

        #if AIR_INTERNAL_DEBUG_HW_LOOPBACK
            /* DAC SINK */
            hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param->stream_out_param);
            open_param->param.stream_out = STREAM_OUT_AFE;
            open_param->stream_out_param.afe.memory          = HAL_AUDIO_MEM1;
            open_param->stream_out_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S32_LE; //config->scenario_config.gaming_mode_config.i2s_in_dongle_config.codec_param.pcm.format;
            open_param->stream_out_param.afe.stream_out_sampling_rate   = config->scenario_config.gaming_mode_config.i2s_in_dongle_config.codec_param.pcm.sample_rate;
            open_param->stream_out_param.afe.sampling_rate   = config->scenario_config.gaming_mode_config.i2s_in_dongle_config.codec_param.pcm.sample_rate;
            open_param->stream_out_param.afe.irq_period      = 1;
            open_param->stream_out_param.afe.frame_size      = 240;
            open_param->stream_out_param.afe.frame_number    = 4;
            open_param->stream_out_param.afe.hw_gain         = true;
        #else
            /* BT SINK */
            gaming_mode_dongle_set_stream_out_bt_common(config, open_param);
        #endif
#else
    TRANSMITTER_LOG_E("[ULL][I2S In] I2S In feature option is not enable", 0);
    configASSERT(0);
#endif /* AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
    } else {
        TRANSMITTER_LOG_E("not in gaming mode scenario sub id list\r\n", 0);
        AUDIO_ASSERT(0);
    }
}
void gaming_mode_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param)
{
    if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET) {
        start_param->param.stream_in = STREAM_IN_AFE;
        start_param->stream_in_param.afe.aws_flag = true;
        start_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;
    } else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) || (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1)) {
        start_param->param.stream_in = STREAM_IN_AUDIO_TRANSMITTER;
        start_param->param.stream_out = STREAM_OUT_BT_COMMON;
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) {
        start_param->param.stream_in = STREAM_IN_BT_COMMON;
        start_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;
    }
#ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
    else if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN)
    {
        start_param->param.stream_in = STREAM_IN_AFE;
        start_param->param.stream_out = STREAM_OUT_BT_COMMON;
        start_param->stream_in_param.afe.aws_flag = true;
    }
#endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE */
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
    else if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_LINE_OUT)
    {
        start_param->param.stream_in = STREAM_IN_BT_COMMON;
        start_param->param.stream_out = STREAM_OUT_AFE;
        start_param->stream_in_param.afe.aws_flag = true;
    }
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
#ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
    else if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_I2S_IN)
    {
        start_param->param.stream_in = STREAM_IN_AFE;
        #if AIR_INTERNAL_DEBUG_HW_LOOPBACK
        start_param->param.stream_out = STREAM_OUT_AFE;
        start_param->stream_in_param.afe.aws_flag = false;
        #else
        start_param->param.stream_out = STREAM_OUT_BT_COMMON;
        start_param->stream_in_param.afe.aws_flag = true;
        #endif
    }
#endif /* AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
}

typedef struct {
    uint32_t vol_gain_1;
    uint32_t vol_gain_2;
} vol_gain_t;
typedef struct {
    uint8_t scenario_type;
    uint8_t scenario_sub_id;
} dl_mixer_param_t;


#define GAIN_COMPENSATION_STEP 10
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

audio_transmitter_status_t gaming_mode_set_runtime_config_playback(audio_transmitter_config_t *config, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config, mcu2dsp_audio_transmitter_runtime_config_param_t *runtime_config_param)
{
    audio_transmitter_status_t ret              = AUDIO_TRANSMITTER_STATUS_FAIL;
    uint32_t                   operation        = runtime_config_type;
    vol_gain_t                 gain             = {0, 0};
    uint8_t                    volume_level     = 0;
    uint8_t                    volume_level_max = 0;
    uint8_t                    volume_ratio     = 0;
    switch (config->scenario_sub_id) {
        case AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT:
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
        case AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_LINE_OUT:
#endif
            switch (operation) {
                case GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_L:
                case GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_R:
                case GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_DUL:
                    volume_level_max = (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) ?
                                        bt_sink_srv_ami_get_usb_voice_sw_max_volume_level() :
                                        bt_sink_srv_ami_get_lineIN_max_volume_level();
                    if ((operation == GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_L) || (operation == GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_DUL)) {
                        volume_level = runtime_config->gaming_mode_runtime_config.vol_level.vol_level_l;
                        if (volume_level > volume_level_max) {
                            volume_level = volume_level_max;
                            TRANSMITTER_LOG_E("set L volume %d level more than max level %d\r\n", 2, volume_level, volume_level_max);
                        }
                        gain.vol_gain_1 = (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) ?
                                            audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT) :
                                            audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_LINE_IN);
                    }
                    if ((operation == GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_R) || (operation == GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_DUL)) {
                        volume_level = runtime_config->gaming_mode_runtime_config.vol_level.vol_level_r;
                        if (volume_level > volume_level_max) {
                            volume_level = volume_level_max;
                            TRANSMITTER_LOG_E("set R volume %d level more than max level %d\r\n", 2, volume_level, volume_level_max);
                        }
                        gain.vol_gain_2 = (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) ?
                                            audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT) :
                                            audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_LINE_IN);
                    }
                    runtime_config_param->config_operation = operation;
                    memcpy(runtime_config_param->config_param, &gain, sizeof(vol_gain_t));
                    TRANSMITTER_LOG_I("scenario_sub_id =%d: operation %d L:volume level %d gain=%d R:volume level %d gain=%d.", 6,
                        config->scenario_sub_id,
                        operation,
                        runtime_config->gaming_mode_runtime_config.vol_level.vol_level_l,
                        gain.vol_gain_1,
                        runtime_config->gaming_mode_runtime_config.vol_level.vol_level_r,
                        gain.vol_gain_2
                        );
                    ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                    break;
                case GAMING_MODE_CONFIG_OP_LATENCY_SWITCH:
                    {
                        uint32_t latency_us = runtime_config->gaming_mode_runtime_config.latency_us;
                        TRANSMITTER_LOG_I("scenario_sub_id =%d: latency switch %d.", 2, config->scenario_sub_id, latency_us);
                        runtime_config_param->config_operation = operation;
                        memcpy(runtime_config_param->config_param, &latency_us, sizeof(uint32_t));
                                    ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                    }
                    break;
                default:
                    TRANSMITTER_LOG_E("operation %d can not do in ULL V1 scenario id %d. ", 2,
                        operation,
                        config->scenario_sub_id
                        );
                    AUDIO_ASSERT(0);
                    ret = AUDIO_TRANSMITTER_STATUS_FAIL;
                    break;
            }
            break;
        case AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0:
        case AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1:
#ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
            case AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN:
#endif
#ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
            case AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_I2S_IN:
#endif

            switch (operation) {
                case GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_L:
                case GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_R:
                case GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL:
                    volume_ratio = runtime_config->gaming_mode_runtime_config.vol_level.vol_ratio;
                    if (volume_ratio > 100) {
                        TRANSMITTER_LOG_E("Volume ratio should between 0 and 100, volume_ratio = \r\n", 1, volume_ratio);
                        volume_ratio = 100;
                    }
                    volume_level_max = ((config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) ||
                                        (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1)) ?
                                        bt_sink_srv_ami_get_usb_music_sw_max_volume_level() :
                                        bt_sink_srv_ami_get_lineIN_max_volume_level();
                    if ((operation == GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_L) || (operation == GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL)) {
                        volume_level = runtime_config->gaming_mode_runtime_config.vol_level.vol_level_l;
                        if (volume_level > volume_level_max) {
                            volume_level = volume_level_max;
                            TRANSMITTER_LOG_E("set L volume %d level more than max level %d\r\n", 2, volume_level, volume_level_max);
                        }
                        gain.vol_gain_1 = (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) ?
                                            audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT) :
                                            audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_LINE_IN);
                        gain.vol_gain_1 += gain_compensation_table[volume_ratio / GAIN_COMPENSATION_STEP];
                        #ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
                        if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN) {
                            gain.vol_gain_1 += (g_ull_line_in_default_d_gain);
                            TRANSMITTER_LOG_I("[ULL][Line in] L default d gain = %d", 1, g_ull_line_in_default_d_gain);
                        }
                        #endif
                    }
                    if ((operation == GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_R) || (operation == GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL)) {
                        volume_level = runtime_config->gaming_mode_runtime_config.vol_level.vol_level_r;
                        if (volume_level > volume_level_max) {
                            volume_level = volume_level_max;
                            TRANSMITTER_LOG_E("set R volume %d level more than max level %d\r\n", 2, volume_level, volume_level_max);
                        }
                        gain.vol_gain_2 = (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) ?
                                            audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT) :
                                            audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_LINE_IN);
                        gain.vol_gain_2 += gain_compensation_table[volume_ratio / GAIN_COMPENSATION_STEP];
                        #ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
                        if (config->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN) {
                            gain.vol_gain_2 += (g_ull_line_in_default_d_gain);
                            TRANSMITTER_LOG_I("[ULL][Line in] R default d gain = %d", 1, g_ull_line_in_default_d_gain);
                        }
                        #endif
                    }
                    runtime_config_param->config_operation = operation;
                    memcpy(runtime_config_param->config_param, &gain, sizeof(vol_gain_t));
                            TRANSMITTER_LOG_I("scenario_sub_id =%d: operation %d L:volume level %d gain=%d R:volume level %d gain=%d volume_ratio = %d.", 7,
                                config->scenario_sub_id,
                                operation,
                                runtime_config->gaming_mode_runtime_config.vol_level.vol_level_l,
                                gain.vol_gain_1,
                                runtime_config->gaming_mode_runtime_config.vol_level.vol_level_r,
                                gain.vol_gain_2,
                                volume_ratio
                                );
                    ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                    break;
                case GAMING_MODE_CONFIG_OP_MUSIC_MIX:
                    {
                        dl_mixer_param_t dl_mixer;
                        TRANSMITTER_LOG_I("scenario_sub_id =%d: MUSIC_MIX id %d.", 2, config->scenario_sub_id, runtime_config->gaming_mode_runtime_config.dl_mixer_id);
                        runtime_config_param->config_operation = operation;
                        dl_mixer.scenario_type = g_audio_transmitter_control[runtime_config->gaming_mode_runtime_config.dl_mixer_id].config.scenario_type;
                        dl_mixer.scenario_sub_id = g_audio_transmitter_control[runtime_config->gaming_mode_runtime_config.dl_mixer_id].config.scenario_sub_id;
                        memcpy(runtime_config_param->config_param, &dl_mixer, sizeof(dl_mixer_param_t));
                            ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                    }
                    break;
                case GAMING_MODE_CONFIG_OP_MUSIC_UNMIX:
                    ret = AUDIO_TRANSMITTER_STATUS_SUCCESS;
                    TRANSMITTER_LOG_I("scenario_sub_id =%d: MUSIC_UNMIX.", 1, config->scenario_sub_id);
                    runtime_config_param->config_operation = operation;
                    break;
                default:
                    TRANSMITTER_LOG_E("operation %d can not do in ULL V1 scenario id %d. ", 2,
                        operation,
                        config->scenario_sub_id
                        );
                    AUDIO_ASSERT(0);
                    ret = AUDIO_TRANSMITTER_STATUS_FAIL;
                    break;
            }
            break;
        default:
            ret = AUDIO_TRANSMITTER_STATUS_FAIL;
            break;
    }
    return ret;
}

audio_transmitter_status_t gaming_mode_get_runtime_config(uint8_t scenario_type, uint8_t scenario_sub_id, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config)
{
    if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) {
        return AUDIO_TRANSMITTER_STATUS_SUCCESS;
    }
    return AUDIO_TRANSMITTER_STATUS_FAIL;
}

void gaming_mode_state_started_handler(uint8_t scenario_sub_id)
{
    if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
        usb_first_in_flag_0 = 0;
#if defined(AIR_USB_AUDIO_1_SPK_ENABLE)
        USB_Audio_Register_Rx_Callback(0, usb_audio_rx_cb_gaming_dongle_0);
#endif  /* AIR_USB_AUDIO_1_SPK_ENABLE */
        TRANSMITTER_LOG_I("[USB_RX_DEBUG 0]register usb_audio_rx_cb 0", 0);
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1) {
        usb_first_in_flag_1 = 0;
#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
        USB_Audio_Register_Rx_Callback(1, usb_audio_rx_cb_gaming_dongle_1);
#endif /* AIR_USB_AUDIO_2_SPK_ENABLE */
        TRANSMITTER_LOG_I("[USB_RX_DEBUG 1]register usb_audio_rx_cb 1", 0);
    }
}

void gaming_mode_state_idle_handler(uint8_t scenario_sub_id)
{
    if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
#if defined(AIR_USB_AUDIO_1_SPK_ENABLE)
        USB_Audio_Register_Rx_Callback(0, NULL);
#endif  /* AIR_USB_AUDIO_1_SPK_ENABLE */
        usb_first_in_flag_0 = 0;
        TRANSMITTER_LOG_I("[USB_RX_DEBUG 0]Unregister usb_audio_rx_cb 0", 0);
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1) {
#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
        USB_Audio_Register_Rx_Callback(1, NULL);
#endif /* AIR_USB_AUDIO_2_SPK_ENABLE */
        usb_first_in_flag_1 = 0;
        TRANSMITTER_LOG_I("[USB_RX_DEBUG 1]Unregister usb_audio_rx_cb 1", 0);
    }
}

void gaming_mode_state_starting_handler(uint8_t scenario_sub_id)
{
    if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET) {
#ifndef AIR_ECNR_PREV_PART_ENABLE
#if defined(AIR_3RD_PARTY_NR_ENABLE)
        hal_dvfs_lock_control(ULL_V1_AUDIO_DVFS_FREQ, HAL_DVFS_LOCK);
#endif /*defined(AIR_3RD_PARTY_NR_ENABLE)*/
#endif /*defined(AIR_ECNR_PREV_PART_ENABLE)*/
    } else {
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_lock_control(ULL_V1_AUDIO_DVFS_FREQ, HAL_DVFS_LOCK);
#endif
    }
}

audio_transmitter_status_t gaming_mode_stop_handler(uint16_t scenario_sub_id)
{
    if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET) {
        return AUDIO_TRANSMITTER_STATUS_SUCCESS;
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN) {
        return AUDIO_TRANSMITTER_STATUS_SUCCESS;
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
        return AUDIO_TRANSMITTER_STATUS_SUCCESS;
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_I2S_IN) {
        /* cover 96K */
        // TODO TODO TODO
        // hal_audio_dsp_hwsrc_clkmux_control(96000, false);
        return AUDIO_TRANSMITTER_STATUS_SUCCESS;
    }
    return AUDIO_TRANSMITTER_STATUS_FAIL;
}

void gaming_mode_state_stoping_handler(uint8_t scenario_sub_id)
{
    if (scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET) {
#ifndef AIR_ECNR_PREV_PART_ENABLE
#if defined(AIR_3RD_PARTY_NR_ENABLE)
        hal_dvfs_lock_control(ULL_V1_AUDIO_DVFS_FREQ, HAL_DVFS_UNLOCK);
#endif /*defined(AIR_3RD_PARTY_NR_ENABLE)*/
#endif /*defined(AIR_ECNR_PREV_PART_ENABLE)*/
    } else {
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_lock_control(ULL_V1_AUDIO_DVFS_FREQ, HAL_DVFS_UNLOCK);
#endif
    }
}

#endif

