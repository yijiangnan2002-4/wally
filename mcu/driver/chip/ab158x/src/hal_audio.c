/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#include <string.h>
#include "hal_audio.h"

/*
  In this file, we implement the APIs listed in audio.h.
  If we need to communicate with DSP, it will call APIs provided by hal_audio_dsp_controller.c.
*/


#if defined(HAL_AUDIO_MODULE_ENABLED)

//==== Include header files ====
#include <assert.h>
#include "hal_log.h"
#include "hal_ccni.h"
#include "hal_gpio.h"
#include "memory_attribute.h"
#include "hal_audio_cm4_dsp_message.h"
#include "hal_audio_message_struct.h"
#include "hal_audio_internal.h"
#include "hal_audio_internal_nvkey_struct.h"
#include "hal_clock_platform.h"
#include "hal_hw_semaphore.h"
#include "hal_resource_assignment.h"
#include "hal_audio_message_struct_common.h"
#ifdef HAL_DVFS_MODULE_ENABLED
#include "hal_dvfs.h"
#include "hal_dvfs_internal.h"
#endif
#include "hal_clock.h"

#ifdef AIR_DCHS_MODE_ENABLE
extern void dchs_dl_get_stream_out_volume(hal_audio_hw_stream_out_index_t hw_gain_index, uint32_t digital_volume_index);
#endif

#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
void hal_audio_status_change_dac_mode_config(uint32_t new_dac_mode);
uint32_t* targe_dac_mode = NULL;
#endif

#if defined(HAL_AUDIO_SUPPORT_APLL)
extern hal_clock_status_t clock_mux_sel(clock_mux_sel_id mux_id, uint32_t mux_sel);
#if 0//modify for ab1568
extern  uint8_t clock_set_pll_on(clock_pll_id pll_id);
extern  uint8_t clock_set_pll_off(clock_pll_id pll_id);
#endif
static int16_t aud_apll_1_cntr;
static int16_t aud_apll_2_cntr;
static hal_audio_mclk_status_t mclk_status[4]; // 4 is number of I2S interfaces.
extern void ami_hal_audio_status_set_running_flag(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool is_running);
#endif

#ifdef HAL_AUDIO_ANC_ENABLE
extern void hal_anc_get_input_device(hal_audio_device_t *in_device1, hal_audio_device_t *in_device2, hal_audio_device_t *in_device3, hal_audio_device_t *in_device4, hal_audio_device_t *in_device5, hal_audio_interface_t *in_interface1, hal_audio_interface_t *in_interface2, hal_audio_interface_t *in_interface3, hal_audio_interface_t *in_interface4, hal_audio_interface_t *in_interface5);
#endif

//==== Static variables ====
uint16_t g_stream_in_sample_rate = 16000;
uint16_t g_stream_in_code_type   = AUDIO_DSP_CODEC_TYPE_PCM;//modify for opus
uint16_t g_wwe_mode = 0;
encoder_bitrate_t g_bit_rate = ENCODER_BITRATE_32KBPS;

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
volatile voice_mic_type_t current_voice_mic_type = VOICE_MIC_TYPE_FIXED;
voice_mic_type_t hal_audio_query_voice_mic_type(void)
{
    return current_voice_mic_type;
}
#endif

audio_common_t audio_common;
HAL_AUDIO_DVFS_CLK_SELECT_t audio_nvdm_dvfs_config;
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
au_afe_multi_input_instance_param_t audio_multi_instance_ctrl = {
    false,  //is_modified
    0,      //audio_device
    0,      //audio_device1
    0,      //audio_device2
    0,      //audio_device3
    0,      //audio_interface
    0,      //audio_interface1
    0,      //audio_interface2
    0,      //audio_interface3
};
#endif
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
#define NVKEY_INDEX1 0xC0
#define NVKEY_INDEX2 0x30
#define NVKEY_INDEX3 0x0C
#define NVKEY_INDEX4 0x03
HAL_AUDIO_CHANNEL_SELECT_t audio_Channel_Select;
HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;

const audio_version_t SW_version = SDK_V1p4; /*Need Change by Owner.*/
volatile audio_version_t nvdm_version = SDK_NONE;
#endif
#define HAL_AUDIO_MAX_OUTPUT_SAMPLING_FREQUENCY 192000
#define HAL_AUDIO_SAMPLING_RATE_MAX HAL_AUDIO_SAMPLING_RATE_192KHZ
//static int default_audio_device_out     = HAL_AUDIO_DEVICE_DAC_DUAL;
//static int default_audio_device_in      = HAL_AUDIO_DEVICE_MAIN_MIC_L;
const char supported_SR_audio_adc_in[HAL_AUDIO_SAMPLING_RATE_MAX + 1] = {
    false,  //HAL_AUDIO_SAMPLING_RATE_8KHZ      = 0, /**< 8000Hz  */
    false,  //HAL_AUDIO_SAMPLING_RATE_11_025KHZ = 1, /**< 11025Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_12KHZ     = 2, /**< 12000Hz */
    true,   //HAL_AUDIO_SAMPLING_RATE_16KHZ     = 3, /**< 16000Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_22_05KHZ  = 4, /**< 22050Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_24KHZ     = 5, /**< 24000Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_32KHZ     = 6, /**< 32000Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_44_1KHZ   = 7, /**< 44100Hz */
    true,   //HAL_AUDIO_SAMPLING_RATE_48KHZ     = 8, /**< 48000Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_88_2KHZ   = 9, /**< 88200Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_96KHZ     = 10,/**< 96000Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_176_4KHZ  = 11,/**< 176400Hz */
    false   //HAL_AUDIO_SAMPLING_RATE_192KHZ    = 12,/**< 192000Hz */
};

const char supported_SR_audio_dac_out[HAL_AUDIO_SAMPLING_RATE_MAX + 1] = {
    false,  //HAL_AUDIO_SAMPLING_RATE_8KHZ      = 0, /**< 8000Hz  */
    false,  //HAL_AUDIO_SAMPLING_RATE_11_025KHZ = 1, /**< 11025Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_12KHZ     = 2, /**< 12000Hz */
    true,   //HAL_AUDIO_SAMPLING_RATE_16KHZ     = 3, /**< 16000Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_22_05KHZ  = 4, /**< 22050Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_24KHZ     = 5, /**< 24000Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_32KHZ     = 6, /**< 32000Hz */
    true,   //HAL_AUDIO_SAMPLING_RATE_44_1KHZ   = 7, /**< 44100Hz */
    true,   //HAL_AUDIO_SAMPLING_RATE_48KHZ     = 8, /**< 48000Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_88_2KHZ   = 9, /**< 88200Hz */
    true,   //HAL_AUDIO_SAMPLING_RATE_96KHZ     = 10,/**< 96000Hz */
    false,  //HAL_AUDIO_SAMPLING_RATE_176_4KHZ  = 11,/**< 176400Hz */
    true    //HAL_AUDIO_SAMPLING_RATE_192KHZ    = 12,/**< 192000Hz */
};

const char supported_SR_audio_i2s_inout[HAL_AUDIO_SAMPLING_RATE_MAX + 1] = {
    true,  //HAL_AUDIO_SAMPLING_RATE_8KHZ      = 0, /**< 8000Hz  */
    true,  //HAL_AUDIO_SAMPLING_RATE_11_025KHZ = 1, /**< 11025Hz */
    true,  //HAL_AUDIO_SAMPLING_RATE_12KHZ     = 2, /**< 12000Hz */
    true,  //HAL_AUDIO_SAMPLING_RATE_16KHZ     = 3, /**< 16000Hz */
    true,  //HAL_AUDIO_SAMPLING_RATE_22_05KHZ  = 4, /**< 22050Hz */
    true,  //HAL_AUDIO_SAMPLING_RATE_24KHZ     = 5, /**< 24000Hz */
    true,  //HAL_AUDIO_SAMPLING_RATE_32KHZ     = 6, /**< 32000Hz */
    true,  //HAL_AUDIO_SAMPLING_RATE_44_1KHZ   = 7, /**< 44100Hz */
    true,  //HAL_AUDIO_SAMPLING_RATE_48KHZ     = 8, /**< 48000Hz */
    true,  //HAL_AUDIO_SAMPLING_RATE_88_2KHZ   = 9, /**< 88200Hz */
    true,  //HAL_AUDIO_SAMPLING_RATE_96KHZ     = 10,/**< 96000Hz */
    true,  //HAL_AUDIO_SAMPLING_RATE_176_4KHZ  = 11,/**< 176400Hz */
    true   //HAL_AUDIO_SAMPLING_RATE_192KHZ    = 12,/**< 192000Hz */
};

//==== Public API ====
hal_audio_status_t hal_audio_init(void)
{
    if (audio_common.init) {
        return HAL_AUDIO_STATUS_OK;
    }

    hal_audio_dsp_controller_init();

#if defined(HAL_AUDIO_SUPPORT_APLL)
    aud_apll_1_cntr = 0;
    aud_apll_2_cntr = 0;
    memset((void *)&mclk_status, 0, 4 * sizeof(hal_audio_mclk_status_t));
#endif

    audio_common.init = true;

    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_deinit(void)
{
    if (audio_common.init) {
#ifdef HAL_AUDIO_DSP_SHUTDOWN_SPECIAL_CONTROL_ENABLE
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_DUMMY_DSP_SHUTDOWN, 0, 0, true);
#endif
        hal_audio_dsp_controller_deinit();
    }

    audio_common.init = false;

    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Register callback to copy the content of stream out
  * @ callback : callback function
  * @ user_data : user data (for exampple, handle)
  * @ Retval: HAL_AUDIO_STATUS_OK if operation is successful, others if operation is invalid
  */
hal_audio_status_t hal_audio_register_copied_stream_out_callback(hal_audio_stream_copy_callback_t callback, void *user_data)
{
    //KH: ToDo
    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Updates the audio output frequency
  * @ sample_rate : audio frequency used to play the audio stream
  * @ This API should be called before hal_audio_start_stream_out() to adjust the audio frequency
  * @ Retval: HAL_AUDIO_STATUS_OK if operation is successful, others if sample rate is invalid
  */
hal_audio_status_t hal_audio_set_stream_out_sampling_rate(hal_audio_sampling_rate_t sampling_rate)
{
    switch (sampling_rate) {
        case HAL_AUDIO_SAMPLING_RATE_8KHZ:
        case HAL_AUDIO_SAMPLING_RATE_11_025KHZ:
        case HAL_AUDIO_SAMPLING_RATE_12KHZ:
        case HAL_AUDIO_SAMPLING_RATE_16KHZ:
        case HAL_AUDIO_SAMPLING_RATE_22_05KHZ:
        case HAL_AUDIO_SAMPLING_RATE_24KHZ:
        case HAL_AUDIO_SAMPLING_RATE_32KHZ:
        case HAL_AUDIO_SAMPLING_RATE_44_1KHZ:
        case HAL_AUDIO_SAMPLING_RATE_48KHZ:
        case HAL_AUDIO_SAMPLING_RATE_88_2KHZ:
        case HAL_AUDIO_SAMPLING_RATE_96KHZ:
        case HAL_AUDIO_SAMPLING_RATE_176_4KHZ:
        case HAL_AUDIO_SAMPLING_RATE_192KHZ:
            audio_common.stream_out.stream_sampling_rate = sampling_rate;
            return HAL_AUDIO_STATUS_OK;
        default:
            return HAL_AUDIO_STATUS_INVALID_PARAMETER;
    }
}

/**
  * @ Updates the audio output channel number
  * @ channel_number : audio channel mode to play the audio stream
  * @ This API should be called before hal_audio_start_stream_out() to adjust the output channel number
  * @ Retval: HAL_AUDIO_STATUS_OK if operation success, others if channel number is invalid
  */
hal_audio_status_t hal_audio_set_stream_out_channel_number(hal_audio_channel_number_t channel_number)
{
    hal_audio_status_t result = HAL_AUDIO_STATUS_OK;
    switch (channel_number) {
        case HAL_AUDIO_MONO:
            audio_common.stream_out.stream_channel = HAL_AUDIO_MONO;
            break;
        case HAL_AUDIO_STEREO:
        case HAL_AUDIO_STEREO_BOTH_L_CHANNEL:
        case HAL_AUDIO_STEREO_BOTH_R_CHANNEL:
        case HAL_AUDIO_STEREO_BOTH_L_R_SWAP:
            audio_common.stream_out.stream_channel = HAL_AUDIO_STEREO;
            break;
        default:
            return HAL_AUDIO_STATUS_INVALID_PARAMETER;
    }
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_OUTPUT_DEVICE_CHANNEL, 0, channel_number, false);
    return result;
}

/**
  * @ Updates the audio output channel mode
  * @ channel_mode : audio channel mode to play the audio stream
  * @ This API should be called before hal_audio_start_stream_out() to adjust the output channel mode
  * @ Retval: HAL_AUDIO_STATUS_OK if operation is successful, others if channel mode is invalid
  */
hal_audio_status_t hal_audio_set_stream_out_channel_mode(hal_audio_channel_number_t channel_mode)
{
    hal_audio_status_t result = HAL_AUDIO_STATUS_OK;
    switch (channel_mode) {
        case HAL_AUDIO_MONO:
        case HAL_AUDIO_STEREO:
        case HAL_AUDIO_STEREO_BOTH_L_CHANNEL:
        case HAL_AUDIO_STEREO_BOTH_R_CHANNEL:
        case HAL_AUDIO_STEREO_BOTH_L_R_SWAP:
            audio_common.stream_out.stream_channel_mode = channel_mode;
            break;
        default:
            result = HAL_AUDIO_STATUS_INVALID_PARAMETER;
            break;
    }
    return result;
}

/**
  * @ Start the playback of audio stream
  */
hal_audio_status_t hal_audio_start_stream_out(hal_audio_active_type_t active_type)
{
    //ToDo: limit the scope -- treat it as local playback
    //audio_dsp_playback_info_t temp_param;
    void *p_param_share;
    bool is_running;

    audio_message_type_t msg_type = AUDIO_MESSAGE_TYPE_PLAYBACK;
    //n9_dsp_share_info_t *p_share_buf_info;

    is_running = hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_PLAYBACK);

    // Open playback
    mcu2dsp_open_param_t open_param;

    // Collect parameters
    open_param.param.stream_in  = STREAM_IN_PLAYBACK;
    open_param.param.stream_out = STREAM_OUT_AFE;
    open_param.audio_scenario_type = AUDIO_SCENARIO_TYPE_VP;

    open_param.stream_in_param.playback.bit_type = HAL_AUDIO_BITS_PER_SAMPLING_16;
    open_param.stream_in_param.playback.sampling_rate = audio_common.stream_out.stream_sampling_rate;
    open_param.stream_in_param.playback.channel_number = audio_common.stream_out.stream_channel;
    open_param.stream_in_param.playback.codec_type = 0;  //KH: should use AUDIO_DSP_CODEC_TYPE_PCM
    open_param.stream_in_param.playback.p_share_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(msg_type);

    hal_audio_reset_share_info(open_param.stream_in_param.playback.p_share_info);
#if 0
    open_param.stream_out_param.afe.audio_device    = HAL_AUDIO_DEVICE_DAC_DUAL;
    open_param.stream_out_param.afe.stream_channel  = HAL_AUDIO_DIRECT;
    if (open_param.stream_out_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
        open_param.stream_out_param.afe.misc_parms      = I2S_CLK_SOURCE_DCXO;
    } else {
        open_param.stream_out_param.afe.misc_parms      = DOWNLINK_PERFORMANCE_NORMAL;
    }
#else
    hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param.stream_out_param);
#endif
    open_param.stream_out_param.afe.memory          = HAL_AUDIO_MEM1;
    open_param.stream_out_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE;
    open_param.stream_out_param.afe.stream_out_sampling_rate   = 16000;
    open_param.stream_out_param.afe.sampling_rate   = 16000;
    open_param.stream_out_param.afe.irq_period      = 10;
    open_param.stream_out_param.afe.frame_size      = 256;
    open_param.stream_out_param.afe.frame_number    = 2;
    open_param.stream_out_param.afe.hw_gain         = false;
    p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), msg_type);
    if (is_running) {
        // Reentry: don't allow multiple playback
        log_hal_msgid_info("Re-entry\r\n", 0);
    } else {
        hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_PLAYBACK, &open_param, true);
    }
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PLAYBACK_OPEN, AUDIO_DSP_CODEC_TYPE_PCM, (uint32_t)p_param_share, true);

    // Start playback
    mcu2dsp_start_param_t start_param;

    // Collect parameters
    start_param.param.stream_in     = STREAM_IN_PLAYBACK;
    start_param.param.stream_out    = STREAM_OUT_AFE;

    start_param.stream_out_param.afe.aws_flag   =  false;

    p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_start_param_t), msg_type);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PLAYBACK_START, 0, (uint32_t)p_param_share, true);

    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Stop the playback of audio stream
  */
void hal_audio_stop_stream_out(void)
{
    //ToDo: limit the scope -- treat it as local playback
    audio_message_type_t msg_type = AUDIO_MESSAGE_TYPE_PLAYBACK;
    n9_dsp_share_info_t *p_share_buf_info;

    // Stop playback
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PLAYBACK_STOP, 0, 0, true);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PLAYBACK_CLOSE, 0, 0, true);

    hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_PLAYBACK, NULL, false);

    // Clear buffer
    p_share_buf_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(msg_type);
    hal_audio_reset_share_info(p_share_buf_info);
}

/**
  * @ Updates the audio output volume
  * @ hw_gain_index: HW_Gain_1/2/3, can't set HW_Gain_ALL.
  * @ digital_volume_index: digital gain index
  * @ analog_volume_index : analog gain index
  */
hal_audio_status_t hal_audio_set_stream_out_volume(hal_audio_hw_stream_out_index_t hw_gain_index, uint32_t digital_volume_index, uint32_t analog_volume_index)
{
    uint32_t data32;
    switch (hw_gain_index) {
        case HAL_AUDIO_STREAM_OUT1:
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            audio_common.stream_out.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_0] = digital_volume_index;
            audio_common.stream_out.analog_gain_index[INPUT_ANALOG_GAIN_FOR_MIC_L] = analog_volume_index;
#else
            audio_common.stream_out.digital_gain_index = digital_volume_index;
            audio_common.stream_out.analog_gain_index = analog_volume_index;
#endif
            break;
        case HAL_AUDIO_STREAM_OUT2:
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            audio_common.stream_out_DL2.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_0] = digital_volume_index;
            audio_common.stream_out_DL2.analog_gain_index[INPUT_ANALOG_GAIN_FOR_MIC_L] = analog_volume_index;
#else
            audio_common.stream_out_DL2.digital_gain_index = digital_volume_index;
            audio_common.stream_out_DL2.analog_gain_index = analog_volume_index;
#endif
            break;
        case HAL_AUDIO_STREAM_OUT3:
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            audio_common.stream_out_DL3.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_0] = digital_volume_index;
            audio_common.stream_out_DL3.analog_gain_index[INPUT_ANALOG_GAIN_FOR_MIC_L] = analog_volume_index;
#else
            audio_common.stream_out_DL3.digital_gain_index = digital_volume_index;
            audio_common.stream_out_DL3.analog_gain_index = analog_volume_index;
#endif
            break;
        case HAL_AUDIO_STREAM_OUT4:
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            audio_common.stream_out_DL12.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_0] = digital_volume_index;
            audio_common.stream_out_DL12.analog_gain_index[INPUT_ANALOG_GAIN_FOR_MIC_L] = analog_volume_index;
#else
            audio_common.stream_out_DL12.digital_gain_index = digital_volume_index;
            audio_common.stream_out_DL12.analog_gain_index = analog_volume_index;
#endif

            break;
        default:
            log_hal_msgid_error("hal_audio_set_stream_out_volume hw_gain index error %d", 1, hw_gain_index);
            return HAL_AUDIO_STATUS_INVALID_PARAMETER;
    }
    data32 = (analog_volume_index << 16) | (digital_volume_index & 0xFFFF);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_OUTPUT_DEVICE_VOLUME, hw_gain_index, data32, false);

    #ifdef AIR_DCHS_MODE_ENABLE
    dchs_dl_get_stream_out_volume(hw_gain_index, digital_volume_index);
    #endif //AIR_DCHS_MODE_ENABLE
    return HAL_AUDIO_STATUS_OK;
}

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT)
/**
  * @ Mute stream ouput path
  * @ mute: true -> set mute / false -> set unmute
  * @ hw_gain_index: HAL_AUDIO_STREAM_OUT1-> indicate hw gain1 / HAL_AUDIO_STREAM_OUT2-> indicate hw gain2 / HAL_AUDIO_STREAM_OUT_ALL-> indicate hw gain1 and hw gain2
  */
void hal_audio_mute_stream_out(bool mute, hal_audio_hw_stream_out_index_t hw_gain_index)
{
    uint32_t data32;
    if (hw_gain_index & HAL_AUDIO_STREAM_OUT1) {
        audio_common.stream_out.mute = mute;
    }
    if (hw_gain_index & HAL_AUDIO_STREAM_OUT2) {
        audio_common.stream_out_DL2.mute = mute;
    }
    if (hw_gain_index & HAL_AUDIO_STREAM_OUT3) {
        audio_common.stream_out_DL3.mute = mute;
    }
    if (hw_gain_index & HAL_AUDIO_STREAM_OUT4) {
        audio_common.stream_out_DL12.mute = mute;
    }
    data32 = (hw_gain_index << 16) | (mute & 0xFFFF);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_MUTE_OUTPUT_DEVICE, 0, data32, false);
}
#else
/**
  * @ Mute stream ouput path
  * @ mute: true -> set mute / false -> set unmute
  */
void hal_audio_mute_stream_out(bool mute)
{
    audio_common.stream_out.mute = mute;

    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_MUTE_OUTPUT_DEVICE, 0, mute, false);
}
#endif

/**
  * @ Control the audio output device
  * @ device: output device
  */
hal_audio_status_t hal_audio_set_stream_out_device(hal_audio_device_t device)
{
    audio_common.stream_out.audio_device = device;

    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_OUTPUT_DEVICE, 0, device, false);

    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Write data into audio output stream for playback.
  * @ buffer: Pointer to the buffer
  * @ size : number of audio data [in bytes]
  * @ Retval HAL_AUDIO_STATUS_OK if operation is successful, others if failed.
  */
hal_audio_status_t hal_audio_write_stream_out(const void *buffer, uint32_t size)
{
    //ToDo: limit the scope -- treat it as local playback
    hal_audio_status_t result;

    result = hal_audio_write_stream_out_by_type(AUDIO_MESSAGE_TYPE_PLAYBACK, buffer, size);

    return result;
}

/**
  * @ Query the free space of output stream.
  * @ sample_count : number of free space [in bytes]
  * @ Retval HAL_AUDIO_STATUS_OK if operation is successful, others if failed
  */
hal_audio_status_t hal_audio_get_stream_out_sample_count(uint32_t *sample_count)
{
    //ToDo: limit the scope -- treat it as local playback
    n9_dsp_share_info_t *p_info = hal_audio_query_playback_share_info();

    *sample_count = hal_audio_buf_mgm_get_free_byte_count(p_info);

    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Register the callback of stream out.
  * @ callback : callback function
  * @ user_data : pointer of user data
  * @ Retval HAL_AUDIO_STATUS_OK if operation is successful, others if failed
  */
hal_audio_status_t hal_audio_register_stream_out_callback(hal_audio_stream_out_callback_t callback, void *user_data)
{
    //ToDo: limit the scope -- treat it as local playback

    hal_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_PLAYBACK, callback, user_data);

    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Updates the audio input frequency
  * @ sample_rate : audio frequency used to record the audio stream
  * @ This API should be called before hal_audio_start_stream_in() to adjust the audio frequency
  * @ Retval: HAL_AUDIO_STATUS_OK if operation is successful, others if sample rate is invalid
  */
hal_audio_status_t hal_audio_set_stream_in_sampling_rate(hal_audio_sampling_rate_t sampling_rate)
{
    //ToDo: extend the sampling rate from 8k/16kHz to 8k~48kHz

    switch (sampling_rate) {
        case HAL_AUDIO_SAMPLING_RATE_8KHZ:
        case HAL_AUDIO_SAMPLING_RATE_11_025KHZ:
        case HAL_AUDIO_SAMPLING_RATE_12KHZ:
        case HAL_AUDIO_SAMPLING_RATE_16KHZ:
        case HAL_AUDIO_SAMPLING_RATE_22_05KHZ:
        case HAL_AUDIO_SAMPLING_RATE_24KHZ:
        case HAL_AUDIO_SAMPLING_RATE_32KHZ:
        case HAL_AUDIO_SAMPLING_RATE_44_1KHZ:
        case HAL_AUDIO_SAMPLING_RATE_48KHZ:
            audio_common.stream_in.stream_sampling_rate = sampling_rate;
            return HAL_AUDIO_STATUS_OK;
        default:
            return HAL_AUDIO_STATUS_INVALID_PARAMETER;
    }
}

/**
  * @ Updates the audio input channel number
  * @ channel_number : audio channel mode to record the audio stream
  * @ This API should be called before hal_audio_start_stream_in() to adjust the input channel number
  * @ Retval: HAL_AUDIO_STATUS_OK if operation is successful, others if channel number is invalid
  */
hal_audio_status_t hal_audio_set_stream_in_channel_number(hal_audio_channel_number_t channel_number)
{
    switch (channel_number) {
        case HAL_AUDIO_MONO:
        case HAL_AUDIO_STEREO:
            audio_common.stream_in.stream_channel = channel_number;
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_INPUT_DEVICE_CHANNEL, 0, channel_number, false);
            return HAL_AUDIO_STATUS_OK;
        default:
            return HAL_AUDIO_STATUS_INVALID_PARAMETER;
    }
}

#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
/**
  * @ Updates the audio input volume for multiple microphones.
  * @ volume_index0: input gain index 0
  * @ volume_index1: input gain index 1
  * @ gain_select  : select which pair of gain to be setting
  */
hal_audio_status_t hal_audio_set_stream_in_volume_for_multiple_microphone(uint32_t volume_index0, uint32_t volume_index1, hal_audio_input_gain_select_t gain_select)
{
    uint32_t data32;

    if (gain_select == HAL_AUDIO_INPUT_GAIN_SELECTION_D0_A0) {
        audio_common.stream_in.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_0] = volume_index0;
        audio_common.stream_in.analog_gain_index[INPUT_ANALOG_GAIN_FOR_MIC_L] = volume_index1;
    } else if (gain_select == HAL_AUDIO_INPUT_GAIN_SELECTION_D0_D1) {
        audio_common.stream_in.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_0] = volume_index0;
        audio_common.stream_in.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_1] = volume_index1;
    } else if (gain_select == HAL_AUDIO_INPUT_GAIN_SELECTION_D2_D3) {
        audio_common.stream_in.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_2] = volume_index0;
        audio_common.stream_in.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_3] = volume_index1;
    } else if (gain_select == HAL_AUDIO_INPUT_GAIN_SELECTION_A0_A1) {
        audio_common.stream_in.analog_gain_index[INPUT_ANALOG_GAIN_FOR_MIC_L] = volume_index0;
        audio_common.stream_in.analog_gain_index[INPUT_ANALOG_GAIN_FOR_MIC_R] = volume_index1;
    }

    data32 = (volume_index1 << 16) | (volume_index0 & 0xFFFF);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_INPUT_DEVICE_VOLUME, gain_select, data32, false);

    return HAL_AUDIO_STATUS_OK;
}

/**
  * @ Updates the audio input volume
  * @ digital_volume_index: digital gain index
  * @ analog_volume_index : analog gain index
  */
hal_audio_status_t hal_audio_set_stream_in_volume(uint32_t digital_volume_index, uint32_t analog_volume_index)

{
    uint32_t data32;

    audio_common.stream_in.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_0] = digital_volume_index;
    audio_common.stream_in.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_1] = digital_volume_index;
    audio_common.stream_in.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_2] = digital_volume_index;
    audio_common.stream_in.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_DEVICE_3] = digital_volume_index;
    audio_common.stream_in.digital_gain_index[INPUT_DIGITAL_GAIN_FOR_ECHO_PATH] = digital_volume_index;
    audio_common.stream_in.analog_gain_index[INPUT_ANALOG_GAIN_FOR_MIC_L] = analog_volume_index;
    audio_common.stream_in.analog_gain_index[INPUT_ANALOG_GAIN_FOR_MIC_R] = analog_volume_index;

    data32 = (analog_volume_index << 16) | (digital_volume_index & 0xFFFF);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_INPUT_DEVICE_VOLUME, 0, data32, false);

    return HAL_AUDIO_STATUS_OK;
}

#else
/**
  * @ Updates the audio input volume
  * @ digital_volume_index: digital gain index
  * @ analog_volume_index : analog gain index
  */
hal_audio_status_t hal_audio_set_stream_in_volume(uint32_t digital_volume_index, uint32_t analog_volume_index)

{
    uint32_t data32;

    audio_common.stream_in.digital_gain_index = digital_volume_index;
    audio_common.stream_in.analog_gain_index = analog_volume_index;

    data32 = (analog_volume_index << 16) | (digital_volume_index & 0xFFFF);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_INPUT_DEVICE_VOLUME, 0, data32, false);

    return HAL_AUDIO_STATUS_OK;
}

#endif

/**
  * @ Mute stream in path
  * @ mute: true -> set mute / false -> set unmute
  */
void hal_audio_mute_stream_in(bool mute)
{
    audio_common.stream_in.mute = mute;

    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_MUTE_INPUT_DEVICE, 0, mute, false);
}

/**
  * @ Mute stream in path by scenario type
  * @ mute: true -> set mute / false -> set unmute
  */
void hal_audio_mute_stream_in_by_scenario(hal_audio_stream_in_scenario_t type, bool mute)
{
    uint16_t data16;

    audio_common.stream_in.mute = mute;

    data16 = type << 8 | 0x8000; /* Mark highest bit for the valid of scenario type */
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_MUTE_INPUT_DEVICE, data16, mute, false);
}

/**
  * @ Control the audio input device
  * @ device: input device
  */
hal_audio_status_t hal_audio_set_stream_in_device(hal_audio_device_t device)
{
    audio_common.stream_in.audio_device = device;

    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_INPUT_DEVICE, 0, device, false);

    return HAL_AUDIO_STATUS_OK;
}

/**
 * @ Query the size of needed memory to be allocated for internal use in audio driver
 * @ memory_size : the amount of memory required by the audio driver for an internal use (in bytes).
 * @ Retval HAL_AUDIO_STATUS_OK if operation is successful, others if failed.
 */
hal_audio_status_t hal_audio_get_memory_size(uint32_t *memory_size)
{
    //ToDo: assume that we don't ennd extra memory
    *memory_size = 0;

    return HAL_AUDIO_STATUS_OK;
}

/**
 * @ Hand over allocated memory to audio driver
 * @ memory : the pointer to an allocated memory. It should be 4 bytes aligned.
 * @ Retval HAL_AUDIO_STATUS_OK if operation is successful, others if failed.
 */
hal_audio_status_t hal_audio_set_memory(void *memory)
{
    audio_common.allocated_memory = memory;

    return HAL_AUDIO_STATUS_OK;
}

/**
 * @ Get audio clock.
 * @ sample_count : a pointer to the accumulated audio sample count.
 * @ Retval HAL_AUDIO_STATUS_OK if operation is successful, others if failed.
 */
hal_audio_status_t hal_audio_get_audio_clock(uint32_t *sample_count)
{
    //ToDo: currently, use fake function.
    *sample_count = 0;

    return HAL_AUDIO_STATUS_OK;
}

#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
uint32_t hal_audio_sampling_rate_enum_to_value(hal_audio_sampling_rate_t hal_audio_sampling_rate_enum)
{
    switch (hal_audio_sampling_rate_enum) {
        case HAL_AUDIO_SAMPLING_RATE_8KHZ:
            return   8000;
        case HAL_AUDIO_SAMPLING_RATE_11_025KHZ:
            return  11025;
        case HAL_AUDIO_SAMPLING_RATE_12KHZ:
            return  12000;
        case HAL_AUDIO_SAMPLING_RATE_16KHZ:
            return  16000;
        case HAL_AUDIO_SAMPLING_RATE_22_05KHZ:
            return  22050;
        case HAL_AUDIO_SAMPLING_RATE_24KHZ:
            return  24000;
        case HAL_AUDIO_SAMPLING_RATE_32KHZ:
            return  32000;
        case HAL_AUDIO_SAMPLING_RATE_44_1KHZ:
            return  44100;
        case HAL_AUDIO_SAMPLING_RATE_48KHZ:
            return  48000;
        case HAL_AUDIO_SAMPLING_RATE_88_2KHZ:
            return  88200;
        case HAL_AUDIO_SAMPLING_RATE_96KHZ:
            return  96000;
        case HAL_AUDIO_SAMPLING_RATE_176_4KHZ:
            return 176400;
        case HAL_AUDIO_SAMPLING_RATE_192KHZ:
            return 192000;

        default:
            return 8000;
    }
}

hal_audio_sampling_rate_t hal_audio_sampling_rate_value_to_enum(uint32_t sample_rate)
{
    hal_audio_sampling_rate_t sample_rate_index;
    switch (sample_rate) {
        case 8000:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_8KHZ;
            break;
        case 11025:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_11_025KHZ;
            break;
        case 12000:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_12KHZ;
            break;
        case 16000:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_16KHZ;
            break;
        case 22050:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_22_05KHZ;
            break;
        case 24000:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_24KHZ;
            break;
        case 32000:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_32KHZ;
            break;
        case 44100:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_44_1KHZ;
            break;
        case 48000:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_48KHZ;
            break;
        case 88200:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_88_2KHZ;
            break;
        case 96000:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_96KHZ;
            break;
        case 176400:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_176_4KHZ;
            break;
        case 192000:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_192KHZ;
            break;
        default:
            sample_rate_index = HAL_AUDIO_SAMPLING_RATE_44_1KHZ; // acutally, most musics are 44.1kHz
            break;
    }
    return sample_rate_index;
}

int32_t hal_audio_get_device_out_supported_frequency(hal_audio_device_t audio_out_device, hal_audio_sampling_rate_t freq)
{
    int32_t device_supported_frequency = -1;
    int32_t i = freq;

    switch (audio_out_device) {
        case HAL_AUDIO_DEVICE_DAC_L:
        case HAL_AUDIO_DEVICE_DAC_R:
        case HAL_AUDIO_DEVICE_DAC_DUAL:
            while (1) {
                if (i > HAL_AUDIO_SAMPLING_RATE_MAX) {
                    device_supported_frequency = -1;
                    break;
                }
                if (supported_SR_audio_dac_out[i] == true) {
                    device_supported_frequency = i;
                    break;
                }
                i++;
            }
            break;
        case HAL_AUDIO_DEVICE_I2S_MASTER:
        case HAL_AUDIO_DEVICE_I2S_SLAVE:
            while (1) {
                if (i > HAL_AUDIO_SAMPLING_RATE_MAX) {
                    device_supported_frequency = -1;
                    break;
                }
                if (supported_SR_audio_i2s_inout[i] == true) {
                    device_supported_frequency = i;
                    break;
                }
                i++;
            }
            break;
        default:
            break;
    }

    if (device_supported_frequency == -1) {
        switch (i - 1) {
            case HAL_AUDIO_SAMPLING_RATE_88_2KHZ:
                device_supported_frequency = HAL_AUDIO_SAMPLING_RATE_96KHZ;
                break;
            case HAL_AUDIO_SAMPLING_RATE_176_4KHZ:
                device_supported_frequency = HAL_AUDIO_SAMPLING_RATE_192KHZ;
                break;
            default:
                log_hal_msgid_warning("Not found AFE supported rate", 0);
                break;
        }

    }
    return device_supported_frequency;
}


static hal_audio_device_t hal_audio_convert_linein_config(uint8_t Mic_NVkey)
{
    hal_audio_device_t device;

    switch (Mic_NVkey) {
        //----Line in
        case 0x00:
            device = HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL;
            break;
        //----I2S Master
        case 0x01:
            device = HAL_AUDIO_DEVICE_I2S_MASTER;
            break;
        //----I2S Slave
        case 0x02:
            device = HAL_AUDIO_DEVICE_I2S_SLAVE;
            break;
        default:
            device = HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL;
            break;
    }

    return device;
}

hal_audio_interface_t hal_audio_convert_linein_interface(uint8_t Mic_NVkey, bool is_input_device)
{
    hal_audio_interface_t audio_interface;
    uint8_t I2S_NVkey = 0;

    switch (Mic_NVkey) {
        //----Line in
        case 0x00:
            audio_interface = HAL_AUDIO_INTERFACE_2;
            break;
        //----I2S Master
        case 0x01:
        //----I2S Slave
        case 0x02:
            if (is_input_device == true) {
                I2S_NVkey = audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_I2S_Interface;
            } else {
                I2S_NVkey = audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_I2S_Interface;
            }
            switch (I2S_NVkey) {
                case 0x00: //I2S0
                    audio_interface = HAL_AUDIO_INTERFACE_1;
                    break;
                case 0x01: //I2S1
                    audio_interface = HAL_AUDIO_INTERFACE_2;
                    break;
                case 0x02: //I2S2
                    audio_interface = HAL_AUDIO_INTERFACE_3;
                    break;
                default:
                    audio_interface = HAL_AUDIO_INTERFACE_NONE;
                    break;
            }
            break;
        default:
            audio_interface = HAL_AUDIO_INTERFACE_1;
            break;
    }

    return audio_interface;
}

hal_audio_device_t hal_audio_convert_mic_config(uint8_t Mic_NVkey)
{
    hal_audio_device_t device;

    switch (Mic_NVkey) {
        //----Analog Mic L
        case 0x00:
        case 0x02:
        case 0x04:
            device = HAL_AUDIO_DEVICE_MAIN_MIC_L;
            break;
        //----Analog Mic R
        case 0x01:
        case 0x03:
        case 0x05:
            device = HAL_AUDIO_DEVICE_MAIN_MIC_R;
            break;
        //----Digital Mic L
        case 0x08:
        case 0x0A:
        case 0x0C:
            device = HAL_AUDIO_DEVICE_DIGITAL_MIC_L;
            break;
        //----Digital Mic R
        case 0x09:
        case 0x0B:
        case 0x0D:
            device = HAL_AUDIO_DEVICE_DIGITAL_MIC_R;
            break;
        //----I2S Master L
        case 0x10:
        case 0x30:
        case 0x50:
            device = HAL_AUDIO_DEVICE_I2S_MASTER_L;
            break;
        //----I2S Master R
        case 0x20:
        case 0x40:
        case 0x60:
            device = HAL_AUDIO_DEVICE_I2S_MASTER_R;
            break;
        //----I2S Slave
        case 0x80:
        case 0x90:
        case 0xA0:
        case 0xB0:
        case 0xC0:
        case 0xD0:
            //TODO need confirm msg
            device = HAL_AUDIO_DEVICE_I2S_SLAVE;
            break;
        default:
            device = HAL_AUDIO_DEVICE_NONE;
            break;
    }
    return device;
}

static hal_audio_interface_t hal_audio_convert_mic_interface(uint8_t Mic_NVkey)
{
    hal_audio_interface_t audio_interface;

    switch (Mic_NVkey) {
        /*AMIC0 connect interface 1*/
        case 0x00:
        case 0x01:
            audio_interface = HAL_AUDIO_INTERFACE_1;
            break;
        /*AMIC1 connect interface 2*/
        case 0x02:
        case 0x03:
            audio_interface = HAL_AUDIO_INTERFACE_2;
            break;
        /*AMIC2 connect interface 3*/
        case 0x04:
        case 0x05:
            audio_interface = HAL_AUDIO_INTERFACE_3;
            break;
        /*DMIC1 connect interface 1*/
        case 0x08:
        case 0x09:
            audio_interface = HAL_AUDIO_INTERFACE_1;
            break;
        /*DMIC2 connect interface 2*/
        case 0x0A:
        case 0x0B:
            audio_interface = HAL_AUDIO_INTERFACE_2;
            break;
        /*DMIC2 connect interface 3*/
        case 0x0C:
        case 0x0D:
            audio_interface = HAL_AUDIO_INTERFACE_3;
            break;
        /*I2S Master 0 interface 1*/
        case 0x10:
        case 0x20:
            audio_interface = HAL_AUDIO_INTERFACE_1;
            break;
        /*I2S Master 1 interface 2*/
        case 0x30:
        case 0x40:
            audio_interface = HAL_AUDIO_INTERFACE_2;
            break;
        /*I2S Master 2 interface 3*/
        case 0x50:
        case 0x60:
            audio_interface = HAL_AUDIO_INTERFACE_3;
            break;
        /*I2S Slave 0 interface 1*/
        case 0x80:
        case 0x90:
            audio_interface = HAL_AUDIO_INTERFACE_1;
            break;
        /*I2S Slave 1 interface 2*/
        case 0xA0:
        case 0xB0:
            audio_interface = HAL_AUDIO_INTERFACE_2;
            break;
        /*I2S Slave 2 interface 3*/
        case 0xC0:
        case 0xD0:
            audio_interface = HAL_AUDIO_INTERFACE_3;
            break;
        default:
            audio_interface = HAL_AUDIO_INTERFACE_NONE;
            break;
    }

    return audio_interface;
}

#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
void hal_audio_multi_instance_confg(mcu2dsp_open_stream_in_param_t *stream_in_open_param)
{
    if (audio_multi_instance_ctrl.is_modified) {
        stream_in_open_param->afe.audio_device      = audio_multi_instance_ctrl.audio_device;
        stream_in_open_param->afe.audio_device1     = audio_multi_instance_ctrl.audio_device1;
        stream_in_open_param->afe.audio_device2     = audio_multi_instance_ctrl.audio_device2;
        stream_in_open_param->afe.audio_device3     = audio_multi_instance_ctrl.audio_device3;
        stream_in_open_param->afe.audio_interface   = audio_multi_instance_ctrl.audio_interface;
        stream_in_open_param->afe.audio_interface1  = audio_multi_instance_ctrl.audio_interface1;
        stream_in_open_param->afe.audio_interface2  = audio_multi_instance_ctrl.audio_interface2;
        stream_in_open_param->afe.audio_interface3  = audio_multi_instance_ctrl.audio_interface3;

        log_hal_msgid_info("[HAL_AUDIO] Open Para is modified from ATC, Dev0:%d, Dev1:%d, Dev2:%d, Dev3:%d",
                           4,
                           stream_in_open_param->afe.audio_device,
                           stream_in_open_param->afe.audio_device1,
                           stream_in_open_param->afe.audio_device2,
                           stream_in_open_param->afe.audio_device3);

        log_hal_msgid_info("[HAL_AUDIO] Open Para is modified from ATC, Int0:%d, Int1:%d, Int2:%d, Int3:%d",
                           4,
                           stream_in_open_param->afe.audio_interface,
                           stream_in_open_param->afe.audio_interface1,
                           stream_in_open_param->afe.audio_interface2,
                           stream_in_open_param->afe.audio_interface3);

        if (audio_multi_instance_ctrl.echo_path_enabled) {
            stream_in_open_param->afe.memory |= HAL_AUDIO_MEM3;
            log_hal_msgid_info("[HAL_AUDIO] Echo path is enabled from ATC", 0);
        } else {
            stream_in_open_param->afe.memory &= (~HAL_AUDIO_MEM3);
            log_hal_msgid_info("[HAL_AUDIO] Echo path is disabled from ATC", 0);
        }
    }
}
#endif

hal_audio_status_t hal_audio_translate_mic_config(hal_audio_mic_config_t *mic_config, mcu2dsp_open_stream_in_param_t *stream_in_open_param)
{
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    uint32_t i;

    for (i = 0; i < 8; i++) {
        stream_in_open_param->afe.amic_type[i] = mic_config->amic_type[i];
        stream_in_open_param->afe.dmic_selection[i] = mic_config->dmic_selection[i];
        stream_in_open_param->afe.ul_adc_mode[i] = mic_config->ul_adc_mode[i];
    }
    for (i = 0; i < 5; i++) {
        stream_in_open_param->afe.bias_voltage[i] = mic_config->bias_voltage[i];
    }
    for (i = 0; i < 3; i++) {
        stream_in_open_param->afe.iir_filter[i] = mic_config->iir_filter[i];
    }
    stream_in_open_param->afe.bias_select = mic_config->bias_select;
    stream_in_open_param->afe.with_external_bias = mic_config->with_external_bias;
    stream_in_open_param->afe.with_bias_lowpower = mic_config->with_bias_lowpower;
    stream_in_open_param->afe.bias1_2_with_LDO0 = mic_config->bias1_2_with_LDO0;
#endif

    stream_in_open_param->afe.adc_mode = mic_config->adc_mode;
    stream_in_open_param->afe.performance = mic_config->performance;

#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    for (i = 0; i < 8; i++) {
        log_hal_msgid_info("stream_in_open_param->afe.amic_type[%d] = %d", 2, i, stream_in_open_param->afe.amic_type[i]);
        log_hal_msgid_info("stream_in_open_param->afe.dmic_selection[%d] = %d", 2, i, stream_in_open_param->afe.dmic_selection[i]);
        log_hal_msgid_info("stream_in_open_param->afe.ul_adc_mode[%d] = %d", 2, i, stream_in_open_param->afe.ul_adc_mode[i]);
    }
    for (i = 0; i < 5; i++) {
        log_hal_msgid_info("stream_in_open_param->afe.bias_voltage[%d] = %d", 2, i, stream_in_open_param->afe.bias_voltage[i]);
    }
    for (i = 0; i < 3; i++) {
        log_hal_msgid_info("stream_in_open_param->afe.iir_filter[%d] = %d", 2, i, stream_in_open_param->afe.iir_filter[i]);
    }
    log_hal_msgid_info("stream_in_open_param->afe.bias_select = %d", 1, stream_in_open_param->afe.bias_select);
    log_hal_msgid_info("stream_in_open_param->afe.with_external_bias = %d", 1, stream_in_open_param->afe.with_external_bias);
    log_hal_msgid_info("stream_in_open_param->afe.with_bias_lowpower = %d", 1, stream_in_open_param->afe.with_bias_lowpower);
    log_hal_msgid_info("stream_in_open_param->afe.bias1_2_with_LDO0 = %d", 1, stream_in_open_param->afe.bias1_2_with_LDO0);
#endif
    log_hal_msgid_info("stream_in_open_param->afe.adc_mode = %d", 1, stream_in_open_param->afe.adc_mode);
    log_hal_msgid_info("stream_in_open_param->afe.performance = %d", 1, stream_in_open_param->afe.performance);

    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_get_mic_config(audio_scenario_sel_t Audio_or_Voice, hal_audio_mic_config_t *mic_config)
{
    uint32_t i;
    mcu2dsp_open_stream_in_param_t stream_in_open_param;

    hal_audio_get_stream_in_setting_config(Audio_or_Voice, &stream_in_open_param);

#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    for (i = 0; i < 8; i++) {
        mic_config->amic_type[i] = stream_in_open_param.afe.amic_type[i];
        mic_config->dmic_selection[i] = stream_in_open_param.afe.dmic_selection[i];
        mic_config->ul_adc_mode[i] = stream_in_open_param.afe.ul_adc_mode[i];
    }
    for (i = 0; i < 5; i++) {
        mic_config->bias_voltage[i] = stream_in_open_param.afe.bias_voltage[i];
    }
    for (i = 0; i < 3; i++) {
        mic_config->iir_filter[i] = stream_in_open_param.afe.iir_filter[i];
    }
    mic_config->bias_select = stream_in_open_param.afe.bias_select;
    mic_config->with_external_bias = stream_in_open_param.afe.with_external_bias;
    mic_config->with_bias_lowpower = stream_in_open_param.afe.with_bias_lowpower;
    mic_config->bias1_2_with_LDO0 = stream_in_open_param.afe.bias1_2_with_LDO0;
#endif
    mic_config->adc_mode = stream_in_open_param.afe.adc_mode;
    mic_config->performance = stream_in_open_param.afe.performance;

#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    for (i = 0; i < 8; i++) {
        log_hal_msgid_info("mic_config->amic_type[%d] = %d", 2, i, mic_config->amic_type[i]);
        log_hal_msgid_info("mic_config->dmic_selection[%d] = %d", 2, i, mic_config->dmic_selection[i]);
        log_hal_msgid_info("mic_config->ul_adc_mode[%d] = %d", 2, i, mic_config->ul_adc_mode[i]);
    }
    for (i = 0; i < 5; i++) {
        log_hal_msgid_info("mic_config->bias_voltage[%d] = %d", 2, i, mic_config->bias_voltage[i]);
    }
    for (i = 0; i < 3; i++) {
        log_hal_msgid_info("mic_config->iir_filter[%d] = %d", 2, i, mic_config->iir_filter[i]);
    }
    log_hal_msgid_info("mic_config->bias_select = %d", 1, mic_config->bias_select);
    log_hal_msgid_info("mic_config->with_external_bias = %d", 1, mic_config->with_external_bias);
    log_hal_msgid_info("mic_config->with_bias_lowpower = %d", 1, mic_config->with_bias_lowpower);
    log_hal_msgid_info("mic_config->bias1_2_with_LDO0 = %d", 1, mic_config->bias1_2_with_LDO0);
#endif
    log_hal_msgid_info("mic_config->adc_mode = %d", 1, mic_config->adc_mode);
    log_hal_msgid_info("mic_config->performance = %d", 1, mic_config->performance);

    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_get_stream_in_setting_config(audio_scenario_sel_t Audio_or_Voice, mcu2dsp_open_stream_in_param_t *stream_in_open_param)
{
    bool Extended_Mic_Config_Flag = false;
    hal_audio_channel_selection_t *MemInterface = &stream_in_open_param->afe.stream_channel;
    hal_gpio_status_t status = HAL_GPIO_STATUS_OK;
    hal_gpio_data_t channel_gpio_data = HAL_GPIO_DATA_LOW;
    uint8_t channel_temp = 0;
    uint32_t i = 0;
    stream_in_open_param->afe.with_upwdown_sampler = false;
    stream_in_open_param->afe.audio_path_input_rate = 0;
    stream_in_open_param->afe.audio_path_output_rate = 0;
    hal_audio_device_t *audio_device = NULL;
    hal_audio_interface_t *audio_interface = NULL;

    /*Audio HW I/O Configure setting*/
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    if ((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path != 0x4) || (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref1_Input_Select == 0xFF)) {
        if (hal_audio_convert_mic_config(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select2) == HAL_AUDIO_DEVICE_NONE) {
            stream_in_open_param->afe.max_channel_num = 1;
        } else {
            stream_in_open_param->afe.max_channel_num = 2;
        }
    } else if (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref5_Input_Select != 0xFF) {
        stream_in_open_param->afe.max_channel_num = 6;
    } else if (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref4_Input_Select != 0xFF) {
        stream_in_open_param->afe.max_channel_num = 5;
    } else if (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref3_Input_Select != 0xFF) {
        stream_in_open_param->afe.max_channel_num = 4;
    } else if (audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref2_Input_Select != 0xFF) {
        stream_in_open_param->afe.max_channel_num = 3;
    } else {
        stream_in_open_param->afe.max_channel_num = 2;
    }
    log_hal_msgid_info("[DETACHABLE MIC] max_channel_num:%d", 1, stream_in_open_param->afe.max_channel_num);
    if ((hal_audio_query_voice_mic_type() == VOICE_MIC_TYPE_DETACHABLE) && (Audio_or_Voice != AU_DSP_AUDIO) && (Audio_or_Voice != AU_DSP_ANC) && (Audio_or_Voice != AU_DSP_LINEIN)) {
        stream_in_open_param->afe.audio_device  = hal_audio_convert_mic_config(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select);
        stream_in_open_param->afe.audio_interface = hal_audio_convert_mic_interface(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select);
        stream_in_open_param->afe.audio_device1  = hal_audio_convert_mic_config(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select2);
        stream_in_open_param->afe.audio_interface1 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Select2);

        //----AMIC MIC bias enable
        stream_in_open_param->afe.bias_select = audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Bias_Enable;
        stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Bias_Enable) << 20;
        //----performance mode
        switch (audio_nvdm_HW_config.detach_mic_scenario.Detach_MIC_Analog_ADC_Performance_Sel) {
            case 0x00:  //---Normal_Mode
                stream_in_open_param->afe.performance = 0x00;
                break;
            case 0x01:  //---High_Performance
                stream_in_open_param->afe.performance = 0x01;
                break;
            case 0x02:  //---Low_Power_mode
                stream_in_open_param->afe.performance = 0x02;
                break;
            case 0x03:  //---Ultra_Low_Power_mode
                stream_in_open_param->afe.performance = 0x03;
                break;
            case 0x04:  //---Super_Ultra_Low_Power_mode
                stream_in_open_param->afe.performance = 0x04;
                break;
            default:
                log_hal_msgid_error("[AUDIO][HAL] stream in ADC performance config error", 0);
                return HAL_AUDIO_STATUS_ERROR;
        }
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        stream_in_open_param->afe.audio_device2 = 0;
        stream_in_open_param->afe.audio_device3 = 0;
        stream_in_open_param->afe.audio_device4 = 0;
        stream_in_open_param->afe.audio_device5 = 0;
        stream_in_open_param->afe.audio_interface2 = 0;
        stream_in_open_param->afe.audio_interface3 = 0;
        stream_in_open_param->afe.audio_interface4 = 0;
        stream_in_open_param->afe.audio_interface5 = 0;
#endif
    } else
#endif
    {
        /*scenario AFE config part*/
        if (Audio_or_Voice == AU_DSP_VOICE) { //0:Audio, 1:Voice
            switch (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path) {
                case 0x04: //Multi-mic
                    Extended_Mic_Config_Flag = true;
                    break;
                case 0x02: //I2S_Master_In
                    stream_in_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_MASTER_L;
                    stream_in_open_param->afe.audio_interface = 1 << (audio_nvdm_HW_config.voice_scenario.Voice_Input_I2S_Interface);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                    stream_in_open_param->afe.audio_device1 = HAL_AUDIO_DEVICE_I2S_MASTER_R;
                    stream_in_open_param->afe.audio_interface1 = 1 << (audio_nvdm_HW_config.voice_scenario.Voice_Input_I2S_Interface);
#endif
                    break;
                case 0x03: //I2S_Slave_In
                    stream_in_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_SLAVE;
                    stream_in_open_param->afe.audio_interface = 1 << (audio_nvdm_HW_config.voice_scenario.Voice_Input_I2S_Interface);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                    stream_in_open_param->afe.audio_device1 = HAL_AUDIO_DEVICE_I2S_SLAVE;
                    stream_in_open_param->afe.audio_interface1 = 1 << (audio_nvdm_HW_config.voice_scenario.Voice_Input_I2S_Interface);
#endif
                    break;
                case 0x01: //Digital Mic
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_DMIC_channel == 0x02) {
                        stream_in_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DIGITAL_MIC_DUAL;
                    } else if (audio_nvdm_HW_config.voice_scenario.Voice_Input_DMIC_channel == 0x01) {
                        stream_in_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DIGITAL_MIC_R;
                    } else if (audio_nvdm_HW_config.voice_scenario.Voice_Input_DMIC_channel == 0x00) {
                        stream_in_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DIGITAL_MIC_L;
                    }
                    break;
                case 0x00: //Analog Mic
                    if (audio_nvdm_HW_config.voice_scenario.Voice_Input_AMIC_channel == 0x02) {
                        stream_in_open_param->afe.audio_device = HAL_AUDIO_DEVICE_MAIN_MIC_DUAL;
                    } else if (audio_nvdm_HW_config.voice_scenario.Voice_Input_AMIC_channel == 0x01) {
                        stream_in_open_param->afe.audio_device = HAL_AUDIO_DEVICE_MAIN_MIC_R;
                    } else if (audio_nvdm_HW_config.voice_scenario.Voice_Input_AMIC_channel == 0x00) {
                        stream_in_open_param->afe.audio_device = HAL_AUDIO_DEVICE_MAIN_MIC_L;
                    }
                    break;
                default:
                    stream_in_open_param->afe.audio_device = HAL_AUDIO_DEVICE_MAIN_MIC_L;
                    break;
            }
            if (Extended_Mic_Config_Flag) {
                stream_in_open_param->afe.audio_device  = hal_audio_convert_mic_config(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select);
                stream_in_open_param->afe.audio_device1 = hal_audio_convert_mic_config(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref1_Input_Select);
                stream_in_open_param->afe.audio_device2 = hal_audio_convert_mic_config(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref2_Input_Select);
                stream_in_open_param->afe.audio_device3 = hal_audio_convert_mic_config(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref3_Input_Select);
                stream_in_open_param->afe.audio_device4 = hal_audio_convert_mic_config(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref4_Input_Select);
                stream_in_open_param->afe.audio_device5 = hal_audio_convert_mic_config(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref5_Input_Select);
                stream_in_open_param->afe.audio_interface = hal_audio_convert_mic_interface(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Main_Input_Select);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                stream_in_open_param->afe.audio_interface1 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref1_Input_Select);
                stream_in_open_param->afe.audio_interface2 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref2_Input_Select);
                stream_in_open_param->afe.audio_interface3 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref3_Input_Select);
                stream_in_open_param->afe.audio_interface4 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref4_Input_Select);
                stream_in_open_param->afe.audio_interface5 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.voice_scenario.Voice_Multiple_Mic_Ref5_Input_Select);
#endif
            } else {
                /*config the interface via 1 AMIC or 1 DMIC setting*/
                if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x00) {
                    /*Amic*/
                    stream_in_open_param->afe.audio_interface = hal_audio_convert_mic_interface(audio_nvdm_HW_config.voice_scenario.Voice_Analog_MIC_Sel);
                } else if (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path == 0x01) {
                    /*Dmic*/
                    stream_in_open_param->afe.audio_interface = hal_audio_convert_mic_interface(audio_nvdm_HW_config.voice_scenario.Voice_Digital_MIC_Sel);
                }
                /*when multi mic feature option is open,should reset the open param to support 1 mic setting*/
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                if ((audio_nvdm_HW_config.voice_scenario.Voice_Input_Path != 0x02) && (audio_nvdm_HW_config.voice_scenario.Voice_Input_Path != 0x03)) {
                    stream_in_open_param->afe.audio_device1 = 0;
                    stream_in_open_param->afe.audio_interface1 = 0;
                }
                stream_in_open_param->afe.audio_device2 = 0;
                stream_in_open_param->afe.audio_device3 = 0;
                stream_in_open_param->afe.audio_device4 = 0;
                stream_in_open_param->afe.audio_device5 = 0;
                stream_in_open_param->afe.audio_interface2 = 0;
                stream_in_open_param->afe.audio_interface3 = 0;
                stream_in_open_param->afe.audio_interface4 = 0;
                stream_in_open_param->afe.audio_interface5 = 0;
#endif
            }
            //----AMIC MIC bias enable
            stream_in_open_param->afe.bias_select = audio_nvdm_HW_config.voice_scenario.Voice_MIC_Bias_Enable;
            stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.voice_scenario.Voice_MIC_Bias_Enable) << 20;
            //----performance mode
            switch (audio_nvdm_HW_config.voice_scenario.Voice_Analog_ADC_Performance_Sel) {
                case 0x00:  //---Normal_Mode
                    stream_in_open_param->afe.performance = 0x00;
                    break;
                case 0x01:  //---High_Performance
                    stream_in_open_param->afe.performance = 0x01;
                    break;
                case 0x02:  //---Low_Power_mode
                    stream_in_open_param->afe.performance = 0x02;
                    break;
                case 0x03:  //---Ultra_Low_Power_mode
                    stream_in_open_param->afe.performance = 0x03;
                    break;
                case 0x04:  //---Super_Ultra_Low_Power_mode
                    stream_in_open_param->afe.performance = 0x04;
                    break;
                default:
                    log_hal_msgid_error("[AUDIO][HAL] stream in ADC performance config error", 0);
                    return HAL_AUDIO_STATUS_ERROR;
            }
        } else if (Audio_or_Voice == AU_DSP_LINEIN) {
            //----config the audio device && interface
            stream_in_open_param->afe.audio_device  = hal_audio_convert_linein_config(audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_Path);
            stream_in_open_param->afe.audio_interface = hal_audio_convert_linein_interface(audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_Path,true);
            #ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                stream_in_open_param->afe.audio_device1 = stream_in_open_param->afe.audio_device;
                stream_in_open_param->afe.audio_interface1 = stream_in_open_param->afe.audio_interface;
            #endif
            //----LINE IN bias enable
            stream_in_open_param->afe.bias_select = audio_nvdm_HW_config.audio_scenario.Audio_LineIn_Bias_Enable;
            stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.audio_scenario.Audio_LineIn_Bias_Enable) << 20;
            //----performance mode
            switch (audio_nvdm_HW_config.audio_scenario.Audio_Analog_LineIn_Performance_Sel) {
                case 0x00:  //---Normal_Mode
                    stream_in_open_param->afe.performance = 0x00;
                    break;
                case 0x01:  //---High_Performance
                    stream_in_open_param->afe.performance = 0x01;
                    break;
                case 0x02:  //---Low_Power_mode
                    stream_in_open_param->afe.performance = 0x02;
                    break;
                case 0x03:  //---Ultra_Low_Power_mode
                    stream_in_open_param->afe.performance = 0x03;
                    break;
                case 0x04:  //---Super_Ultra_Low_Power_mode
                    stream_in_open_param->afe.performance = 0x04;
                    break;
                default:
                    log_hal_msgid_error("[AUDIO][HAL] stream in ADC performance config error", 0);
                    return HAL_AUDIO_STATUS_ERROR;
            }
        } else if (Audio_or_Voice == AU_DSP_ANC) {
#ifdef HAL_AUDIO_ANC_ENABLE
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
            hal_anc_get_input_device(&(stream_in_open_param->afe.audio_device), &(stream_in_open_param->afe.audio_device1), &(stream_in_open_param->afe.audio_device2), &(stream_in_open_param->afe.audio_device3), &(stream_in_open_param->afe.audio_device4),
                                     &(stream_in_open_param->afe.audio_interface), &(stream_in_open_param->afe.audio_interface1), &(stream_in_open_param->afe.audio_interface2), &(stream_in_open_param->afe.audio_interface3), &(stream_in_open_param->afe.audio_interface4));
#else
            hal_anc_get_input_device(&(stream_in_open_param->afe.audio_device), &(stream_in_open_param->afe.audio_device1), &(stream_in_open_param->afe.audio_device2), &(stream_in_open_param->afe.audio_device3), &(stream_in_open_param->afe.audio_device4),
                                     &(stream_in_open_param->afe.audio_interface), NULL, NULL, NULL, NULL);
#endif
#else
            stream_in_open_param->afe.audio_interface  = HAL_AUDIO_INTERFACE_1;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
            stream_in_open_param->afe.audio_interface1 = HAL_AUDIO_INTERFACE_2;
            stream_in_open_param->afe.audio_interface2 = HAL_AUDIO_INTERFACE_3;
            stream_in_open_param->afe.audio_interface3 = HAL_AUDIO_INTERFACE_4;
#endif
#endif
            //----AMIC MIC bias enable
            stream_in_open_param->afe.bias_select = audio_nvdm_HW_config.anc_scenario.ANC_MIC_Bias_Enable;
            stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.anc_scenario.ANC_MIC_Bias_Enable) << 20;
            //----performance mode
            switch (audio_nvdm_HW_config.anc_scenario.ANC_MIC_Analog_ADC_Performance_Sel) {
                case 0x00:  //---Normal_Mode
                    stream_in_open_param->afe.performance = 0x00;
                    break;
                case 0x01:  //---High_Performance
                    stream_in_open_param->afe.performance = 0x01;
                    break;
                case 0x02:  //---Low_Power_mode
                    stream_in_open_param->afe.performance = 0x02;
                    break;
                case 0x03:  //---Ultra_Low_Power_mode
                    stream_in_open_param->afe.performance = 0x03;
                    break;
                case 0x04:  //---Super_Ultra_Low_Power_mode
                    stream_in_open_param->afe.performance = 0x04;
                    break;
                default:
                    log_hal_msgid_error("[AUDIO][HAL] stream in ADC performance config error", 0);
                    return HAL_AUDIO_STATUS_ERROR;
            }
        } else if (Audio_or_Voice == AU_DSP_RECORD) {
            stream_in_open_param->afe.audio_device = hal_audio_convert_mic_config(audio_nvdm_HW_config.record_scenario.Record_Main_Input_Select);
            stream_in_open_param->afe.audio_device1 = hal_audio_convert_mic_config(audio_nvdm_HW_config.record_scenario.Record_Ref_Input_Select);
            //reserve record param for default val
            stream_in_open_param->afe.audio_device2 = hal_audio_convert_mic_config(audio_nvdm_HW_config.record_scenario.Record_Ref2_Input_Select);
            stream_in_open_param->afe.audio_device3 = hal_audio_convert_mic_config(audio_nvdm_HW_config.record_scenario.Record_Ref3_Input_Select);
            stream_in_open_param->afe.audio_device4 = hal_audio_convert_mic_config(audio_nvdm_HW_config.record_scenario.Record_Ref4_Input_Select);
            stream_in_open_param->afe.audio_device5 = hal_audio_convert_mic_config(audio_nvdm_HW_config.record_scenario.Record_Ref5_Input_Select);

            stream_in_open_param->afe.audio_interface = hal_audio_convert_mic_interface(audio_nvdm_HW_config.record_scenario.Record_Main_Input_Select);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
            stream_in_open_param->afe.audio_interface1 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.record_scenario.Record_Ref_Input_Select);
            //reserve record param for default val
            stream_in_open_param->afe.audio_interface2 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.record_scenario.Record_Ref2_Input_Select);
            stream_in_open_param->afe.audio_interface3 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.record_scenario.Record_Ref3_Input_Select);
            stream_in_open_param->afe.audio_interface4 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.record_scenario.Record_Ref4_Input_Select);
            stream_in_open_param->afe.audio_interface5 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.record_scenario.Record_Ref5_Input_Select);
#endif
            //----AMIC MIC bias enable
            stream_in_open_param->afe.bias_select = audio_nvdm_HW_config.record_scenario.Record_MIC_Bias_Enable;
            stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.record_scenario.Record_MIC_Bias_Enable) << 20;
            //----performance mode
            switch (audio_nvdm_HW_config.record_scenario.Record_Analog_ADC_Performance_Sel) {
                case 0x00:  //---Normal_Mode
                    stream_in_open_param->afe.performance = 0x00;
                    break;
                case 0x01:  //---High_Performance
                    stream_in_open_param->afe.performance = 0x01;
                    break;
                case 0x02:  //---Low_Power_mode
                    stream_in_open_param->afe.performance = 0x02;
                    break;
                case 0x03:  //---Ultra_Low_Power_mode
                    stream_in_open_param->afe.performance = 0x03;
                    break;
                case 0x04:  //---Super_Ultra_Low_Power_mode
                    stream_in_open_param->afe.performance = 0x04;
                    break;
                default:
                    log_hal_msgid_error("[AUDIO][HAL] stream in ADC performance config error", 0);
                    return HAL_AUDIO_STATUS_ERROR;
            }
        } else if ((Audio_or_Voice == AU_DSP_VAD_PHASE0) || (Audio_or_Voice == AU_DSP_VAD_PHASE1)) {
            if (Audio_or_Voice == AU_DSP_VAD_PHASE0) {
                stream_in_open_param->afe.audio_device  = hal_audio_convert_mic_config(audio_nvdm_HW_config.VAD_scenario.VAD_phase0_Main_Input_Select);
                stream_in_open_param->afe.audio_interface = hal_audio_convert_mic_interface(audio_nvdm_HW_config.VAD_scenario.VAD_phase0_Main_Input_Select);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                stream_in_open_param->afe.audio_device1 = hal_audio_convert_mic_config(audio_nvdm_HW_config.VAD_scenario.VAD_phase0_Ref_Input_Select);
                stream_in_open_param->afe.audio_interface1 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.VAD_scenario.VAD_phase0_Ref_Input_Select);
#endif
            } else {
                stream_in_open_param->afe.audio_device  = hal_audio_convert_mic_config(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Main_Input_Select);
                stream_in_open_param->afe.audio_interface = hal_audio_convert_mic_interface(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Main_Input_Select);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
                stream_in_open_param->afe.audio_device1 = hal_audio_convert_mic_config(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Ref_Input_Select);
                stream_in_open_param->afe.audio_interface1 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Ref_Input_Select);
                stream_in_open_param->afe.audio_device2 = hal_audio_convert_mic_config(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Ref2_Input_Select);
                stream_in_open_param->afe.audio_interface2 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Ref2_Input_Select);
                stream_in_open_param->afe.audio_device3 = hal_audio_convert_mic_config(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Ref3_Input_Select);
                stream_in_open_param->afe.audio_interface3 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Ref3_Input_Select);
                stream_in_open_param->afe.audio_device4 = hal_audio_convert_mic_config(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Ref4_Input_Select);
                stream_in_open_param->afe.audio_interface4 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Ref4_Input_Select);
                stream_in_open_param->afe.audio_device5 = hal_audio_convert_mic_config(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Ref5_Input_Select);
                stream_in_open_param->afe.audio_interface5 = hal_audio_convert_mic_interface(audio_nvdm_HW_config.VAD_scenario.VAD_phase1_Ref5_Input_Select);
#endif
            }
            //----AMIC MIC bias enable
            stream_in_open_param->afe.bias_select = audio_nvdm_HW_config.VAD_scenario.VAD_MIC_Bias_Enable;
            stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.VAD_scenario.VAD_MIC_Bias_Enable) << 20;
            //----performance mode
            switch (audio_nvdm_HW_config.VAD_scenario.VAD_Analog_ADC_Performance_Sel) {
                case 0x00:  //---Normal_Mode
                    stream_in_open_param->afe.performance = 0x00;
                    break;
                case 0x01:  //---High_Performance
                    stream_in_open_param->afe.performance = 0x01;
                    break;
                case 0x02:  //---Low_Power_mode
                    stream_in_open_param->afe.performance = 0x02;
                    break;
                case 0x03:  //---Ultra_Low_Power_mode
                    stream_in_open_param->afe.performance = 0x03;
                    break;
                case 0x04:  //---Super_Ultra_Low_Power_mode
                    stream_in_open_param->afe.performance = 0x04;
                    break;
                default:
                    log_hal_msgid_error("[AUDIO][HAL] stream in ADC performance config error", 0);
                    return HAL_AUDIO_STATUS_ERROR;
            }
        } else {
            log_hal_msgid_error("[AUDIO][HAL] hal_audio_get_stream_in_setting_config scenario_sel error.", 0);
            return HAL_AUDIO_STATUS_ERROR;
        }
    }
    /*common AFE config part*/
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    hal_audio_multi_instance_confg(stream_in_open_param);
#endif
    /*check if multi device have config I2S device*/
    audio_device = &(stream_in_open_param->afe.audio_device);
    audio_interface = &(stream_in_open_param->afe.audio_interface);
    /*max 8 device support on platform*/
    for (uint32_t i = 0; i < 8; i++) {
        if (*audio_device & (HAL_AUDIO_DEVICE_I2S_MASTER | HAL_AUDIO_DEVICE_I2S_MASTER_L | HAL_AUDIO_DEVICE_I2S_MASTER_R)) {
            stream_in_open_param->afe.audio_path_output_rate = 16000;
            stream_in_open_param->afe.with_upwdown_sampler = true;
            /*config I2S HW settings*/
            if (*audio_interface & HAL_AUDIO_INTERFACE_1) {
                switch (audio_nvdm_HW_config.I2SM_config.I2S0_Master_Low_jitter) {
                    case 0x1: //----0x1: APLL
                        stream_in_open_param->afe.is_low_jitter[0] = true;
                        break;
                    case 0x0: //----0x0: DCXO
                        stream_in_open_param->afe.is_low_jitter[0] = false;
                        break;
                }
                stream_in_open_param->afe.i2s_master_format[0] = audio_nvdm_HW_config.I2SM_config.I2S0_Master_Format;
                stream_in_open_param->afe.i2s_master_word_length[0] = audio_nvdm_HW_config.I2SM_config.I2S0_Master_Word_length;
                switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S0_Master_Sampling_Rate) {
                    case 0x1: //----0x1: fix 48000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[0] = 48000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[0];
                        break;
                    case 0x2: //----0x2: fix 96000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[0] = 96000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[0];
                        break;
                    case 0x3: //----0x3: fix 32000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[0] = 32000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[0];
                        break;
                    case 0x0: //----0x0: by scenario
                    default:
                        stream_in_open_param->afe.i2s_master_sampling_rate[0] = 0;
                        break;
                }
            } else if (*audio_interface & HAL_AUDIO_INTERFACE_2) {
                switch (audio_nvdm_HW_config.I2SM_config.I2S1_Master_Low_jitter) {
                    case 0x1: //----0x1: APLL
                        stream_in_open_param->afe.is_low_jitter[1] = true;
                        break;
                    case 0x0: //----0x0: DCXO
                        stream_in_open_param->afe.is_low_jitter[1] = false;
                        break;
                }
                stream_in_open_param->afe.i2s_master_format[1] = audio_nvdm_HW_config.I2SM_config.I2S1_Master_Format;
                stream_in_open_param->afe.i2s_master_word_length[1] = audio_nvdm_HW_config.I2SM_config.I2S1_Master_Word_length;
                switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S1_Master_Sampling_Rate) {
                    case 0x1: //----0x1: fix 48000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[1] = 48000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[1];
                        break;
                    case 0x2: //----0x2: fix 96000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[1] = 96000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[1];
                        break;
                    case 0x3: //----0x3: fix 32000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[1] = 32000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[1];
                        break;
                    case 0x0: //----0x0: by scenario
                    default:
                        stream_in_open_param->afe.i2s_master_sampling_rate[1] = 0;
                        break;
                }
            } else if (*audio_interface & HAL_AUDIO_INTERFACE_3) {
                switch (audio_nvdm_HW_config.I2SM_config.I2S2_Master_Low_jitter) {
                    case 0x1: //----0x1: APLL
                        stream_in_open_param->afe.is_low_jitter[2] = true;
                        break;
                    case 0x0: //----0x0: DCXO
                        stream_in_open_param->afe.is_low_jitter[2] = false;
                        break;
                }
                stream_in_open_param->afe.i2s_master_format[2] = audio_nvdm_HW_config.I2SM_config.I2S2_Master_Format;
                stream_in_open_param->afe.i2s_master_word_length[2] = audio_nvdm_HW_config.I2SM_config.I2S2_Master_Word_length;
                switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S2_Master_Sampling_Rate) {
                    case 0x1: //----0x1: fix 48000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[2] = 48000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[2];
                        break;
                    case 0x2: //----0x2: fix 96000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[2] = 96000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[2];
                        break;
                    case 0x3: //----0x3: fix 32000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[2] = 32000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[2];
                        break;
                    case 0x0: //----0x0: by scenario
                    default:
                        stream_in_open_param->afe.i2s_master_sampling_rate[2] = 0;
                        break;
                }
            } else if (*audio_interface & HAL_AUDIO_INTERFACE_4) {
               switch (audio_nvdm_HW_config.I2SM_config.I2S3_Master_Low_jitter) {
                    case 0x1: //----0x1: APLL
                        stream_in_open_param->afe.is_low_jitter[3] = true;
                        break;
                    case 0x0: //----0x0: DCXO
                        stream_in_open_param->afe.is_low_jitter[3] = false;
                        break;
                }
                stream_in_open_param->afe.i2s_master_format[3] = audio_nvdm_HW_config.I2SM_config.I2S3_Master_Format;
                stream_in_open_param->afe.i2s_master_word_length[3] = audio_nvdm_HW_config.I2SM_config.I2S3_Master_Word_length;
                switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S3_Master_Sampling_Rate) {
                    case 0x1: //----0x1: fix 48000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[3] = 48000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[3];
                        break;
                    case 0x2: //----0x2: fix 96000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[3] = 96000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[3];
                        break;
                    case 0x3: //----0x3: fix 32000Hz
                        stream_in_open_param->afe.i2s_master_sampling_rate[3] = 32000;
                        stream_in_open_param->afe.audio_path_input_rate = stream_in_open_param->afe.i2s_master_sampling_rate[3];
                        break;
                    case 0x0: //----0x0: by scenario
                    default:
                        stream_in_open_param->afe.i2s_master_sampling_rate[3] = 0;
                        break;
                }
            }
            //break;
        } else if (*audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
            /*config I2S HW settings*/
            stream_in_open_param->afe.i2s_format = audio_nvdm_HW_config.I2SS_config.I2S_Slave_Format;
            stream_in_open_param->afe.i2S_Slave_TDM = audio_nvdm_HW_config.I2SS_config.I2S_Slave_TDM;
            stream_in_open_param->afe.i2s_word_length = audio_nvdm_HW_config.I2SS_config.I2S_Slave_Word_length;
            log_hal_msgid_info("[stream in] have config I2S slave device in", 0);
            //break;
        }
        audio_device++;
        audio_interface++;
    }

    //----AMIC ACC DCC Setting
    stream_in_open_param->afe.ul_adc_mode[0] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC0_Mode;
    stream_in_open_param->afe.ul_adc_mode[1] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC1_Mode;
    stream_in_open_param->afe.ul_adc_mode[2] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC2_Mode;
    stream_in_open_param->afe.ul_adc_mode[3] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC3_Mode;
    stream_in_open_param->afe.ul_adc_mode[4] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC4_Mode;
    stream_in_open_param->afe.ul_adc_mode[5] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC5_Mode;
    stream_in_open_param->afe.amic_type[0] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC0_Type;
    stream_in_open_param->afe.amic_type[1] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC1_Type;
    stream_in_open_param->afe.amic_type[2] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC2_Type;
    stream_in_open_param->afe.amic_type[3] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC3_Type;
    stream_in_open_param->afe.amic_type[4] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC4_Type;
    stream_in_open_param->afe.amic_type[5] = audio_nvdm_HW_config.adc_dac_config.ADDA_Analog_MIC5_Type;
    //----AMIC MIC bias voltage
    stream_in_open_param->afe.bias_voltage[0] = audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias0_Level;
    stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias0_Level);
    stream_in_open_param->afe.bias_voltage[1] = audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias1_Level;
    stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias1_Level) << 4;
    stream_in_open_param->afe.bias_voltage[2] = audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias2_Level;
    stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias2_Level) << 8;
    stream_in_open_param->afe.bias_voltage[3] = audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias3_Level;
    stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias3_Level) << 12;
    stream_in_open_param->afe.bias_voltage[4] = audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias4_Level;
    stream_in_open_param->afe.misc_parms |= (uint32_t)(audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias4_Level) << 16;
    //---iir_filter
    if ((Audio_or_Voice != AU_DSP_AUDIO) && (Audio_or_Voice != AU_DSP_LINEIN)) {
        memset(stream_in_open_param->afe.iir_filter, audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_IIR_Filter, sizeof(stream_in_open_param->afe.iir_filter));
    } else {
        memset(stream_in_open_param->afe.iir_filter, audio_nvdm_HW_config.adc_dac_config.ADDA_Audio_IIR_Filter, sizeof(stream_in_open_param->afe.iir_filter));
    }
    /*use mic scenario common config*/
    if (Audio_or_Voice != AU_DSP_AUDIO) {
        //---with_external_bias
        stream_in_open_param->afe.with_external_bias = 0;
        //---bias lowpower enable
        stream_in_open_param->afe.with_bias_lowpower = (audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias_Mode == 0x2)? 1:0;
        //---bias1_2_with_LDO0
        if (audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias012_share_LDO) {
            stream_in_open_param->afe.bias1_2_with_LDO0 = true;
        } else {
            stream_in_open_param->afe.bias1_2_with_LDO0 = false;
        }
        //---dmic_selection
        stream_in_open_param->afe.dmic_selection[0] = audio_nvdm_HW_config.dmic_config.DMIC_First_Digital_MIC_Pin_Sel;
        stream_in_open_param->afe.dmic_selection[1] = audio_nvdm_HW_config.dmic_config.DMIC_First_Digital_MIC_Pin_Sel;
        stream_in_open_param->afe.dmic_selection[2] = audio_nvdm_HW_config.dmic_config.DMIC_Second_Digital_MIC_Pin_Sel;
        stream_in_open_param->afe.dmic_selection[3] = audio_nvdm_HW_config.dmic_config.DMIC_Second_Digital_MIC_Pin_Sel;
        stream_in_open_param->afe.dmic_selection[4] = audio_nvdm_HW_config.dmic_config.DMIC_Third_Digital_MIC_Pin_Sel;
        stream_in_open_param->afe.dmic_selection[5] = audio_nvdm_HW_config.dmic_config.DMIC_Third_Digital_MIC_Pin_Sel;
        /*enable mic bias 3 select if dmic select analog dmic 3*/
        for (uint8_t i = 0; i < 6; i++) {
            if (stream_in_open_param->afe.dmic_selection[i] == 0x05) {
                //stream_in_open_param->afe.bias_select |= HAL_AUDIO_BIAS_SELECT_BIAS3;
                break;
            }
        }
        //---DMIC clock
        memset(stream_in_open_param->afe.dmic_clock_rate, 0x00, sizeof(stream_in_open_param->afe.dmic_clock_rate));
    }
    /*Audio Channel selection setting*/
    if (audio_Channel_Select.modeForAudioChannel) {
        //----HW_mode
        status = hal_gpio_get_input((hal_gpio_pin_t)audio_Channel_Select.hwAudioChannel.gpioIndex, &channel_gpio_data);
        if (status == HAL_GPIO_STATUS_OK) {
            if (channel_gpio_data == HAL_GPIO_DATA_HIGH) {
                channel_temp = (audio_Channel_Select.hwAudioChannel.audioChannelGPIOH & (NVKEY_INDEX1 | NVKEY_INDEX2)) >> 4;
            } else {
                channel_temp = (audio_Channel_Select.hwAudioChannel.audioChannelGPIOL & (NVKEY_INDEX1 | NVKEY_INDEX2)) >> 4;
            }
        } else {
            channel_temp = AU_DSP_CH_LR; //default.
            log_hal_msgid_info("Get Stream in channel setting false with HW_mode.", 0);
        }
    } else {
        channel_temp = (audio_Channel_Select.audioChannel & (NVKEY_INDEX1 | NVKEY_INDEX2)) >> 4;
    }
    switch (channel_temp) {
        case AU_DSP_CH_LR: {
            *MemInterface = HAL_AUDIO_DIRECT;
            break;
        }
        case AU_DSP_CH_L: {
            *MemInterface = HAL_AUDIO_BOTH_L;
            break;
        }
        case AU_DSP_CH_R: {
            *MemInterface = HAL_AUDIO_BOTH_R;
            break;
        }
        case AU_DSP_CH_SWAP: {
            *MemInterface = HAL_AUDIO_SWAP_L_R;
            break;
        }
        case AU_DSP_CH_MIX: {
            *MemInterface = HAL_AUDIO_MIX_L_R;
            break;
        }
        case AU_DSP_CH_MIX_SHIFT: {
            *MemInterface = HAL_AUDIO_MIX_SHIFT_L_R;
            break;
        }
        default: {
            *MemInterface = HAL_AUDIO_DIRECT;
            break;
        }
    }

    //For debug
    log_hal_msgid_info("[stream in] Audio_or_Voice = %d", 1, Audio_or_Voice);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    log_hal_msgid_info("[stream in] audio_device = %d,audio_device1 = %d,audio_device2 = %d,audio_device3 = %d,audio_device4 = %d,audio_device5 = %d", 6,
                       stream_in_open_param->afe.audio_device,
                       stream_in_open_param->afe.audio_device1,
                       stream_in_open_param->afe.audio_device2,
                       stream_in_open_param->afe.audio_device3,
                       stream_in_open_param->afe.audio_device4,
                       stream_in_open_param->afe.audio_device5);
    log_hal_msgid_info("[stream in] audio_interface = %d,audio_interface1 = %d,audio_interface2 = %d,audio_interface3 = %d,audio_interface4 = %d,audio_interface5 = %d", 6,
                       stream_in_open_param->afe.audio_interface,
                       stream_in_open_param->afe.audio_interface1,
                       stream_in_open_param->afe.audio_interface2,
                       stream_in_open_param->afe.audio_interface3,
                       stream_in_open_param->afe.audio_interface4,
                       stream_in_open_param->afe.audio_interface5);
#else
    log_hal_msgid_info("[stream in] audio_device = %d, audio_interface = %d", 2, stream_in_open_param->afe.audio_device, stream_in_open_param->afe.audio_interface);
#endif

    log_hal_msgid_info("[stream in] bias0_1_2_with_LDO0 = %d", 1, stream_in_open_param->afe.bias1_2_with_LDO0);
    log_hal_msgid_info("[stream in] bias_select = %d", 1, stream_in_open_param->afe.bias_select);
    for (i = 0; i < 5; i++) {
        log_hal_msgid_info("[stream in] bias_voltage[%d] = %d", 2, i, stream_in_open_param->afe.bias_voltage[i]);
    }
    for (i = 0; i < 6; i++) {
        log_hal_msgid_info("[stream in] ul_adc_mode[%d] = %d, amic_type[%d] = %d", 4, i, stream_in_open_param->afe.ul_adc_mode[i], i, stream_in_open_param->afe.amic_type[i]);
    }
    return HAL_AUDIO_STATUS_OK;

    log_hal_msgid_info("[stream in] performance = %d", 1, stream_in_open_param->afe.performance);
}

uint8_t hal_audio_get_stream_in_channel_num(audio_scenario_sel_t Audio_or_Voice)
{
    mcu2dsp_open_stream_in_param_t stream_in_open_param;
    uint8_t num=0;
    hal_audio_get_stream_in_setting_config(Audio_or_Voice, &stream_in_open_param);
    if ((stream_in_open_param.afe.audio_device!=0)&&(stream_in_open_param.afe.audio_interface!=0))
    {
        num++;
    }
    if ((stream_in_open_param.afe.audio_device1!=0)&&(stream_in_open_param.afe.audio_interface1!=0))
    {
        num++;
    }
    if ((stream_in_open_param.afe.audio_device2!=0)&&(stream_in_open_param.afe.audio_interface2!=0))
    {
        num++;
    }
    if ((stream_in_open_param.afe.audio_device3!=0)&&(stream_in_open_param.afe.audio_interface3!=0))
    {
        num++;
    }
    if ((stream_in_open_param.afe.audio_device4!=0)&&(stream_in_open_param.afe.audio_interface4!=0))
    {
        num++;
    }
    if ((stream_in_open_param.afe.audio_device5!=0)&&(stream_in_open_param.afe.audio_interface5!=0))
    {
        num++;
    }
    log_hal_msgid_info("[stream in] get channel num = %d", 1, num);
    return num;
}

hal_audio_status_t hal_audio_get_stream_out_setting_config(audio_scenario_sel_t Audio_or_Voice, mcu2dsp_open_stream_out_param_t *stream_out_open_param)
{
    hal_audio_channel_selection_t *MemInterface = &stream_out_open_param->afe.stream_channel;
    hal_gpio_status_t status;
    hal_gpio_data_t channel_gpio_data = HAL_GPIO_DATA_LOW;
    uint8_t channel_temp;

    /*Audio HW I/O Configure setting */
    if (Audio_or_Voice == AU_DSP_VOICE) { //0:Audio, 1:Voice
        switch (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path) {
            case 0x00: //----Analog_SPK_Out_L
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_L;
                break;
            case 0x01: //----Analog_SPK_Out_R
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_R;
                break;
            case 0x02: //----Analog_SPK_Out_Dual
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
                break;
            case 0x03: //----I2S_Master_Out
            case 0x04: //----I2S_Slave_Out
                stream_out_open_param->afe.audio_interface = 1 << (audio_nvdm_HW_config.voice_scenario.Voice_Output_I2S_Interface);
                if (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03) {
                    stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_MASTER;
                } else if (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x04) {
                    stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_SLAVE;
                }
                if (stream_out_open_param->afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
                    if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_1) {
                        switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S0_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_2) {
                        switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S1_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_3) {
                        switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S2_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_4) {
                        switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S3_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 0;
                                break;
                        }
                    }
                }
                break;
            default:
                log_hal_msgid_info("Get Voice Stream out Device error. Defualt", 0);
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
                break;
        }

        //---with_external_bias
        stream_out_open_param->afe.with_external_bias = 0;
        //---bias lowpower enable
        stream_out_open_param->afe.with_bias_lowpower = 0;
        //---bias1_2_with_LDO0
        if (audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias012_share_LDO) {
            stream_out_open_param->afe.bias1_2_with_LDO0 = true;
        } else {
            stream_out_open_param->afe.bias1_2_with_LDO0 = false;
        }
        //---dmic_selection
        stream_out_open_param->afe.dmic_selection[0] = audio_nvdm_HW_config.dmic_config.DMIC_First_Digital_MIC_Pin_Sel;
        stream_out_open_param->afe.dmic_selection[1] = audio_nvdm_HW_config.dmic_config.DMIC_Second_Digital_MIC_Pin_Sel;
        stream_out_open_param->afe.dmic_selection[2] = audio_nvdm_HW_config.dmic_config.DMIC_Third_Digital_MIC_Pin_Sel;
        //---iir_filter
        memset(stream_out_open_param->afe.iir_filter, audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_IIR_Filter, sizeof(stream_out_open_param->afe.iir_filter));
    } else if (Audio_or_Voice == AU_DSP_AUDIO) {
        switch (audio_nvdm_HW_config.audio_scenario.Audio_A2DP_Output_Path) {
            case 0x00: //----Analog_SPK_Out_L
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_L;
                break;
            case 0x01: //----Analog_SPK_Out_R
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_R;
                break;
            case 0x02: //----Analog_SPK_Out_Dual
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
                break;
            case 0x03: //----I2S_Master_Out
            case 0x04: //----I2S_Slave_Out
                stream_out_open_param->afe.audio_interface = 1 << (audio_nvdm_HW_config.audio_scenario.Audio_A2DP_Output_I2S_Interface);
                if (audio_nvdm_HW_config.audio_scenario.Audio_A2DP_Output_Path == 0x03) {
                    stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_MASTER;
                } else if (audio_nvdm_HW_config.audio_scenario.Audio_A2DP_Output_Path == 0x04) {
                    stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_SLAVE;
                }
                if (stream_out_open_param->afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
                    if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_1) {
                        switch (audio_nvdm_HW_config.audio_scenario.Audio_I2S0_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_2) {
                        switch (audio_nvdm_HW_config.audio_scenario.Audio_I2S1_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_3) {
                        switch (audio_nvdm_HW_config.audio_scenario.Audio_I2S2_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_4) {
                        switch (audio_nvdm_HW_config.audio_scenario.Audio_I2S3_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 32000;
                            break;
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 0;
                                break;
                        }
                    }
                }
                break;
            default:
                log_hal_msgid_info("Get Audio Stream out Device error. Defualt", 0);
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
                break;
        }
    } else if (Audio_or_Voice == AU_DSP_LINEIN) {
        switch (audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_Path) {
            case 0x00: //----Analog_SPK_Out_L
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_L;
                break;
            case 0x01: //----Analog_SPK_Out_R
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_R;
                break;
            case 0x02: //----Analog_SPK_Out_Dual
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
                break;
            case 0x03: //----I2S_Master_Out
            case 0x04: //----I2S_Slave_Out
                stream_out_open_param->afe.audio_interface = hal_audio_convert_linein_interface(0x01, false);
                if (audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_Path == 0x03) {
                    stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_MASTER;
                } else if (audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_Path == 0x04) {
                    stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_SLAVE;
                }
                if (stream_out_open_param->afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
                    if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_1) {
                        switch (audio_nvdm_HW_config.audio_scenario.Audio_I2S0_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_2) {
                        switch (audio_nvdm_HW_config.audio_scenario.Audio_I2S1_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_3) {
                        switch (audio_nvdm_HW_config.audio_scenario.Audio_I2S2_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 48000;
                                break;
                            case 0x2: //----0x1: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_4) {
                        switch (audio_nvdm_HW_config.audio_scenario.Audio_I2S3_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 48000;
                                break;
                            case 0x2: //----0x1: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 0;
                                break;
                        }
                    }
                }
                break;
            default:
                log_hal_msgid_info("Get Audio Stream out Device error. Defualt", 0);
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
                break;
        }
    } else if (Audio_or_Voice == AU_DSP_ANC) {
        switch (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path) {
            case 0x00: //----Analog_SPK_Out_L
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_L;
                break;
            case 0x01: //----Analog_SPK_Out_R
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_R;
                break;
            case 0x02: //----Analog_SPK_Out_Dual
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
                break;
            case 0x03: //----I2S_Master_Out
                stream_out_open_param->afe.audio_interface = 1 << (audio_nvdm_HW_config.voice_scenario.Voice_Output_I2S_Interface);
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_MASTER;
            case 0x04: //----I2S_Slave_out
                break;
            default:
                log_hal_msgid_info("Get Audio Stream out Device error. Defualt", 0);
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
                break;
        }
    } else if (Audio_or_Voice == AU_DSP_SIDETONE) {
        switch (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path) {
            case 0x00: //----Analog_SPK_Out_L
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_L;
                break;
            case 0x01: //----Analog_SPK_Out_R
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_R;
                break;
            case 0x02: //----Analog_SPK_Out_Dual
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
                break;
            case 0x03: //----I2S_Master_Out
            case 0x04: //----I2S_Slave_Out
#ifdef HAL_CHANGE_SIDETONE_OUTPUT_I2S_INTERFACE_USE_A2DP
                /* The sidetone use A2DP path */
                stream_out_open_param->afe.audio_interface = 1 << (audio_nvdm_HW_config.audio_scenario.Audio_Output_I2S_Interface);
#else
                /* The sidetone use HFP path */
                stream_out_open_param->afe.audio_interface = 1 << (audio_nvdm_HW_config.voice_scenario.Voice_Output_I2S_Interface);
#endif
                if (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x03) {
                    stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_MASTER;
                } else if (audio_nvdm_HW_config.voice_scenario.Voice_Output_Path == 0x04) {
                    stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_I2S_SLAVE;
                }
                if (stream_out_open_param->afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
                    if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_1) {
                        switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S0_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[0] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_2) {
                        switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S1_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[1] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_3) {
                        switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S2_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[2] = 0;
                                break;
                        }
                    } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_4) {
                        switch (audio_nvdm_HW_config.voice_scenario.Voice_I2S3_Master_Sampling_Rate) {
                            case 0x1: //----0x1: fix 48000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 48000;
                                break;
                            case 0x2: //----0x2: fix 96000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 96000;
                                break;
                            case 0x3: //----0x3: fix 32000Hz
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 32000;
                            break;
                            case 0x0: //----0x0: by scenario
                            default:
                                stream_out_open_param->afe.i2s_master_sampling_rate[3] = 0;
                                break;
                        }
                    }
                }
                break;
            default:
                log_hal_msgid_info("Get Voice Stream out Device error. Defualt", 0);
                stream_out_open_param->afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
                break;
        }

        //---with_external_bias
        stream_out_open_param->afe.with_external_bias = 0;
        //---bias lowpower enable
        stream_out_open_param->afe.with_bias_lowpower = 0;
        //---bias1_2_with_LDO0
        if (audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias012_share_LDO) {
            stream_out_open_param->afe.bias1_2_with_LDO0 = true;
        } else {
            stream_out_open_param->afe.bias1_2_with_LDO0 = false;
        }
        //---dmic_selection
        stream_out_open_param->afe.dmic_selection[0] = audio_nvdm_HW_config.dmic_config.DMIC_First_Digital_MIC_Pin_Sel;
        stream_out_open_param->afe.dmic_selection[1] = audio_nvdm_HW_config.dmic_config.DMIC_Second_Digital_MIC_Pin_Sel;
        stream_out_open_param->afe.dmic_selection[2] = audio_nvdm_HW_config.dmic_config.DMIC_Third_Digital_MIC_Pin_Sel;
        //---iir_filter
        memset(stream_out_open_param->afe.iir_filter, audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_IIR_Filter, sizeof(stream_out_open_param->afe.iir_filter));
    }

    /*I2S COMMON PARA*/
    if (stream_out_open_param->afe.audio_device & (HAL_AUDIO_DEVICE_I2S_MASTER | HAL_AUDIO_DEVICE_I2S_MASTER_L | HAL_AUDIO_DEVICE_I2S_MASTER_R)) {
        /*config I2S HW settings*/
        if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_1) {
            switch (audio_nvdm_HW_config.I2SM_config.I2S0_Master_Low_jitter) {
                case 0x1: //----0x1: APLL
                    stream_out_open_param->afe.is_low_jitter[0] = true;
                    break;
                case 0x0: //----0x0: DCXO
                    stream_out_open_param->afe.is_low_jitter[0] = false;
                    break;
            }
            stream_out_open_param->afe.i2s_master_format[0] = audio_nvdm_HW_config.I2SM_config.I2S0_Master_Format;
            stream_out_open_param->afe.i2s_master_word_length[0] = audio_nvdm_HW_config.I2SM_config.I2S0_Master_Word_length;
        } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_2) {
            switch (audio_nvdm_HW_config.I2SM_config.I2S1_Master_Low_jitter) {
                case 0x1: //----0x1: APLL
                    stream_out_open_param->afe.is_low_jitter[1] = true;
                    break;
                case 0x0: //----0x0: DCXO
                    stream_out_open_param->afe.is_low_jitter[1] = false;
                    break;
                }
            stream_out_open_param->afe.i2s_master_format[1] = audio_nvdm_HW_config.I2SM_config.I2S1_Master_Format;
            stream_out_open_param->afe.i2s_master_word_length[1] = audio_nvdm_HW_config.I2SM_config.I2S1_Master_Word_length;
        } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_3) {
            switch (audio_nvdm_HW_config.I2SM_config.I2S2_Master_Low_jitter) {
                case 0x1: //----0x1: APLL
                    stream_out_open_param->afe.is_low_jitter[2] = true;
                    break;
                case 0x0: //----0x0: DCXO
                    stream_out_open_param->afe.is_low_jitter[2] = false;
                    break;
            }
            stream_out_open_param->afe.i2s_master_format[2] = audio_nvdm_HW_config.I2SM_config.I2S2_Master_Format;
            stream_out_open_param->afe.i2s_master_word_length[2] = audio_nvdm_HW_config.I2SM_config.I2S2_Master_Word_length;
        } else if (stream_out_open_param->afe.audio_interface == HAL_AUDIO_INTERFACE_4) {
            switch (audio_nvdm_HW_config.I2SM_config.I2S3_Master_Low_jitter) {
                case 0x1: //----0x1: APLL
                    stream_out_open_param->afe.is_low_jitter[3] = true;
                    break;
                case 0x0: //----0x0: DCXO
                    stream_out_open_param->afe.is_low_jitter[3] = false;
                    break;
            }
            stream_out_open_param->afe.i2s_master_format[3] = audio_nvdm_HW_config.I2SM_config.I2S3_Master_Format;
            stream_out_open_param->afe.i2s_master_word_length[3] = audio_nvdm_HW_config.I2SM_config.I2S3_Master_Word_length;
        }
    } else if (stream_out_open_param->afe.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
        /*config I2S HW settings*/
        stream_out_open_param->afe.i2s_format = audio_nvdm_HW_config.I2SS_config.I2S_Slave_Format;
        stream_out_open_param->afe.i2S_Slave_TDM = audio_nvdm_HW_config.I2SS_config.I2S_Slave_TDM;
        stream_out_open_param->afe.i2s_word_length = audio_nvdm_HW_config.I2SS_config.I2S_Slave_Word_length;
        //break;
    }

    /*DAC AFE PARA*/
    switch (audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Class_AB_G_Performance_Sel) {
        case 0x01: { //----DAC High Perfromance
            stream_out_open_param->afe.performance = 0x01;
            log_hal_msgid_info("DL DAC change to HP mode", 0);
            break;
        }
        case 0x00:  //----DAC Normal mode
        default:
            stream_out_open_param->afe.performance = 0x00;
            break;
    }

    switch (audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Mode_Sel) {
        case 0x00: //Class G2
            stream_out_open_param->afe.dl_dac_mode = 0;
            break;
        case 0x01:  //Class AB
            stream_out_open_param->afe.dl_dac_mode = 1;
            break;
        case 0x02: //Class D
            stream_out_open_param->afe.dl_dac_mode = 2;
            break;
        case 0x03: //Class G3
            stream_out_open_param->afe.dl_dac_mode = 3;
            break;
        case 0x04: //OL Class D
            stream_out_open_param->afe.dl_dac_mode = 4;
            break;
        default:
            stream_out_open_param->afe.dl_dac_mode = 0;
            break;
    }

    /*Audio Channel selection setting*/
    if (audio_Channel_Select.modeForAudioChannel) {
        //----HW_mode
        status = hal_gpio_get_input((hal_gpio_pin_t)audio_Channel_Select.hwAudioChannel.gpioIndex, &channel_gpio_data);
        if (status == HAL_GPIO_STATUS_OK) {
            if (channel_gpio_data == HAL_GPIO_DATA_HIGH) {
                channel_temp = audio_Channel_Select.hwAudioChannel.audioChannelGPIOH & (NVKEY_INDEX3 | NVKEY_INDEX4);
            } else {
                channel_temp = audio_Channel_Select.hwAudioChannel.audioChannelGPIOL & (NVKEY_INDEX3 | NVKEY_INDEX4);
            }
        } else {
            channel_temp = AU_DSP_CH_LR; //default.
            log_hal_msgid_info("Get Stream out channel setting false with HW_mode.", 0);
        }
    } else {
        channel_temp = audio_Channel_Select.audioChannel & (NVKEY_INDEX3 | NVKEY_INDEX4);
    }

//Change to DSP SW Channel select
    switch (channel_temp) {
        case AU_DSP_CH_LR: {
            *MemInterface = HAL_AUDIO_DIRECT;
            break;
        }
#if 0
        case AU_DSP_CH_L: {
            *MemInterface = HAL_AUDIO_BOTH_L;
            break;
        }
        case AU_DSP_CH_R: {
            *MemInterface = HAL_AUDIO_BOTH_R;
            break;
        }
        case AU_DSP_CH_SWAP: {
            *MemInterface = HAL_AUDIO_SWAP_L_R;
            break;
        }
        case AU_DSP_CH_MIX: {
            *MemInterface = HAL_AUDIO_MIX_L_R;
            break;
        }
        case AU_DSP_CH_MIX_SHIFT: {
            *MemInterface = HAL_AUDIO_MIX_SHIFT_L_R;
            break;
        }
#endif
        default: {
            *MemInterface = HAL_AUDIO_DIRECT;
            break;
        }
    }
    //For debug
    log_hal_msgid_info("[stream out] Audio_or_Voice = %d", 1, Audio_or_Voice);
    log_hal_msgid_info("[stream out] audio_device = %d", 1, stream_out_open_param->afe.audio_device);
    log_hal_msgid_info("[stream out] audio_interface = %d", 1, stream_out_open_param->afe.audio_interface);
    log_hal_msgid_info("[stream out] stream_channel = %d", 1, stream_out_open_param->afe.stream_channel);
    log_hal_msgid_info("[stream out] bias0_1_2_with_LDO0 = %d", 1, stream_out_open_param->afe.bias1_2_with_LDO0);

    return HAL_AUDIO_STATUS_OK;
}
#endif

#if defined(HAL_DVFS_MODULE_ENABLED)
void hal_audio_set_dvfs_clk(audio_scenario_sel_t Audio_or_Voice, dvfs_frequency_t *dvfs_clk)
{
    if (Audio_or_Voice == AU_DSP_VOICE) { //0:Audio, 1:Voice
        switch (audio_nvdm_dvfs_config.HFP_DVFS_CLK) {
            case 0x00:
                *dvfs_clk = HAL_DVFS_OPP_LOW;
                break;
            case 0x01:
                *dvfs_clk = HAL_DVFS_OPP_MID;
                break;
            case 0x02:
                *dvfs_clk = HAL_DVFS_OPP_HIGH;
                break;
            default:
                *dvfs_clk = HAL_DVFS_OPP_LOW;
                break;
        }
    }
}
#endif

void hal_audio_set_dvfs_control(hal_audio_dvfs_speed_t DVFS_SPEED, hal_audio_dvfs_lock_parameter_t DVFS_lock)
{
#ifdef HAL_DVFS_MODULE_ENABLED
    hal_dvfs_lock_control(DVFS_SPEED, DVFS_lock);
#else
    log_hal_msgid_info("[Hal]dvfs module not enable.", 0);
#endif
}

#if defined(HAL_AUDIO_SUPPORT_DEBUG_DUMP)
/**
  * @ Dump audio debug register
  */
void hal_audio_debug_dump(void)
{
    if (hal_audio_status_query_running_flag_value() == false) {
        log_hal_msgid_info("Not do audio debug dump, dsp_controller.running == 0", 0);
        return;
    }

    log_hal_msgid_info("[BASIC]", 0);
    log_hal_msgid_info("[Audio Top Control Register 0]0x70000000 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000000)));
    log_hal_msgid_info("[Audio Top Control Register 1]0x70000004 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000004)));
    log_hal_msgid_info("[AFE Control Register 0]0x70000010 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000010)));
    log_hal_msgid_info("[AFE Control Register 1]0x70000014 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000014)));
    log_hal_msgid_info("[AFE Control Register 2]0x700002E0 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x700002E0)));

    log_hal_msgid_info("[Connection]", 0);
    log_hal_msgid_info("[AFE_CONN0]0x70000020 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000020)));
    log_hal_msgid_info("[AFE_CONN1]0x70000024 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000024)));
    log_hal_msgid_info("[AFE_CONN2]0x70000028 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000028)));
    log_hal_msgid_info("[AFE_CONN3]0x7000002c = 0x%x\r\n", 1, *((volatile uint32_t *)(0x7000002c)));
    log_hal_msgid_info("[AFE_CONN4]0x70000030 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000030)));
    log_hal_msgid_info("[AFE_CONN5]0x7000005c = 0x%x\r\n", 1, *((volatile uint32_t *)(0x7000005c)));
    log_hal_msgid_info("[AFE_CONN6]0x700000bc = 0x%x\r\n", 1, *((volatile uint32_t *)(0x700000bc)));
    log_hal_msgid_info("[AFE_CONN7]0x70000420 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000420)));
    log_hal_msgid_info("[AFE_CONN8]0x70000438 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000438)));
    log_hal_msgid_info("[AFE_CONN9]0x70000440 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000440)));
    log_hal_msgid_info("[AFE_CONN10]0x70000444 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000444)));
    log_hal_msgid_info("[AFE_CONN11]0x70000448 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000448)));
    log_hal_msgid_info("[AFE_CONN12]0x7000044c = 0x%x\r\n", 1, *((volatile uint32_t *)(0x7000044c)));
    log_hal_msgid_info("[AFE_CONN13]0x70000450 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000450)));
    log_hal_msgid_info("[AFE_CONN14]0x70000454 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000454)));
    log_hal_msgid_info("[AFE_CONN15]0x70000458 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000458)));
    log_hal_msgid_info("[AFE_CONN16]0x7000045c = 0x%x\r\n", 1, *((volatile uint32_t *)(0x7000045c)));
    log_hal_msgid_info("[AFE_CONN17]0x70000460 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000460)));
    log_hal_msgid_info("[AFE_CONN18]0x70000464 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000464)));
    log_hal_msgid_info("[AFE_CONN19]0x70000468 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000468)));
    log_hal_msgid_info("[AFE_CONN20]0x7000046c = 0x%x\r\n", 1, *((volatile uint32_t *)(0x7000046c)));
    log_hal_msgid_info("[AFE_CONN21]0x70000470 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000470)));
    log_hal_msgid_info("[AFE_CONN22]0x70000474 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000474)));
    log_hal_msgid_info("[AFE_CONN23]0x70000478 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000478)));

    log_hal_msgid_info("[Output data format]0x7000006c = 0x%x\r\n", 1, *((volatile uint32_t *)(0x7000006c)));

    log_hal_msgid_info("[HWSRC]", 0);
    log_hal_msgid_info("0x70000004 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000004)));
    log_hal_msgid_info("0x70001150 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70001150)));
    log_hal_msgid_info("0x70001170 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70001170)));
    log_hal_msgid_info("[HWSRC DL1]0x70001100 = 0x%x, ASRC_BUSY=%x\r\n", 2, *((volatile uint32_t *)(0x70001100)), (*((volatile uint32_t *)(0x70001100))) & (1 << 9));
    log_hal_msgid_info("[HWSRC DL1]0x70001100 = 0x%x, ASRC_EN=%x\r\n", 2, *((volatile uint32_t *)(0x70001100)), (*((volatile uint32_t *)(0x70001100))) & (1 << 8));
    log_hal_msgid_info("[HWSRC DL2]0x70001200 = 0x%x, ASRC_BUSY=%x\r\n", 2, *((volatile uint32_t *)(0x70001200)), (*((volatile uint32_t *)(0x70001200))) & (1 << 9));
    log_hal_msgid_info("[HWSRC DL2]0x70001200 = 0x%x, ASRC_EN=%x\r\n", 2, *((volatile uint32_t *)(0x70001200)), (*((volatile uint32_t *)(0x70001200))) & (1 << 8));

    log_hal_msgid_info("[I2S Master]", 0);
    log_hal_msgid_info("[I2S Master 0]0x70000860 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000860)));
    log_hal_msgid_info("[I2S Master 1]0x70000864 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000864)));
    log_hal_msgid_info("[I2S Master 2]0x70000868 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000868)));
    log_hal_msgid_info("[I2S Master 3]0x7000086c = 0x%x\r\n", 1, *((volatile uint32_t *)(0x7000086c)));
    log_hal_msgid_info("[I2S Master clock gating]0x70000004 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000004)));
    log_hal_msgid_info("[DSP APLL clock gating]", 0);
    log_hal_msgid_info("0x70000000 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000000)));
    log_hal_msgid_info("0x70000dd0 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000dd0)));

    log_hal_msgid_info("[Mic]", 0);
    log_hal_msgid_info("0xA2070108 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070108)));
    log_hal_msgid_info("0xA207010C = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA207010C)));
    log_hal_msgid_info("0xA2070224 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070224)));
    log_hal_msgid_info("0xA2070124 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070124)));
    log_hal_msgid_info("0xA2070130 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070130)));
    log_hal_msgid_info("0xA2070134 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070134)));
    log_hal_msgid_info("0x70000f98 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000f98)));
    log_hal_msgid_info("0xA207011C = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA207011C)));
    log_hal_msgid_info("0xA2070128 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070128)));
    log_hal_msgid_info("0xA207012C = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA207012C)));
    log_hal_msgid_info("[AFE_ADDA_TOP_CON0]0x70000120 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000120)));
    log_hal_msgid_info("[AFE_ADDA_UL_SRC_CON0]0x70000114 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000114)));
    log_hal_msgid_info("[AFE_ADDA2_UL_SRC_CON0]0x70000604 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000604)));
    log_hal_msgid_info("[AFE_ADDA6_UL_SRC_CON0]0x70000a84 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000a84)));
    log_hal_msgid_info("[AFE_ADDA_UL_DL_CON0]0x70000124 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000124)));

    log_hal_msgid_info("[DAC]", 0);
    log_hal_msgid_info("0x70000108 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000108)));
    log_hal_msgid_info("0x70000c50 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000c50)));
    log_hal_msgid_info("[AFE_ADDA_UL_DL_CON0]0x70000124 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000124)));
    log_hal_msgid_info("0xA2070200 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070200)));
    log_hal_msgid_info("0xA2070204 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070204)));
    log_hal_msgid_info("0xA2070208 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070208)));
    log_hal_msgid_info("0xA207020C = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA207020C)));
    log_hal_msgid_info("0xA2070210 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070210)));
    log_hal_msgid_info("0xA2070214 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070214)));
    log_hal_msgid_info("0xA207021C = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA207021C)));
    log_hal_msgid_info("0xA2070220 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070220)));
    log_hal_msgid_info("0xA2070224 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070224)));
    log_hal_msgid_info("0xA2070228 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070228)));
    log_hal_msgid_info("0xA207022C = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA207022C)));
    log_hal_msgid_info("0xA2070230 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070230)));
    log_hal_msgid_info("0x70000f50 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000f50)));
    log_hal_msgid_info("0x70000ed0 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000ed0)));

    log_hal_msgid_info("[INPUT GAIN]", 0);
    log_hal_msgid_info("[Input analog gain][L PGA GAIN]0xA2070100 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070100)));
    log_hal_msgid_info("[Input analog gain][R PGA GAIN]0xA2070104 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070104)));
    log_hal_msgid_info("[Input digital gain][SW GAIN]", 0);
    log_hal_msgid_info("[OUTPUT GAIN]", 0);
    log_hal_msgid_info("[Output analog gain][AMP GAIN]0x70000f58 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000f58)));
    log_hal_msgid_info("[Output digital gain][HW GAIN1 TARGET]0x70000414 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000414)));
    log_hal_msgid_info("[Output digital gain][HW GAIN2 TARGET]0x7000042C = 0x%x\r\n", 1, *((volatile uint32_t *)(0x7000042C)));
    log_hal_msgid_info("[Output digital gain][HW GAIN1 CURRENT]0x70000424 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000424)));
    log_hal_msgid_info("[Output digital gain][HW GAIN2 CURRENT]0x7000043C = 0x%x\r\n", 1, *((volatile uint32_t *)(0x7000043C)));

    log_hal_msgid_info("[UL/DL and classg]", 0);
    log_hal_msgid_info("0x70000908 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000908)));
    log_hal_msgid_info("0x70000900 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000900)));
    log_hal_msgid_info("0x70000908 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000908)));
    log_hal_msgid_info("0x70000e6c = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000e6c)));
    log_hal_msgid_info("0x70000EE4 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000EE4)));
    log_hal_msgid_info("0x70000EE8 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000EE8)));
    log_hal_msgid_info("0x70000EEC = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000EEC)));
    log_hal_msgid_info("0x70000EE0 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000EE0)));
    log_hal_msgid_info("0x70000EDC = 0x%x\r\n", 1, *((volatile uint32_t *)(0x70000EDC)));
    log_hal_msgid_info("0xA2070224 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2070224)));

    log_hal_msgid_info("[I2S Slave]", 0);
    log_hal_msgid_info("[I2S Slave 0]0xA0070038 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA0070038)));
    log_hal_msgid_info("[I2S Slave 0]0xA0070008 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA0070008)));
    log_hal_msgid_info("[I2S Slave 0]0xA0070030 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA0070030)));
    log_hal_msgid_info("[I2S Slave 1]0xA0080038 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA0080038)));
    log_hal_msgid_info("[I2S Slave 1]0xA0080008 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA0080008)));
    log_hal_msgid_info("[I2S Slave 1]0xA0080030 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA0080030)));
    log_hal_msgid_info("[I2S Slave 2]0xA0090038 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA0090038)));
    log_hal_msgid_info("[I2S Slave 2]0xA0090008 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA0090008)));
    log_hal_msgid_info("[I2S Slave 2]0xA0090030 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA0090030)));
    log_hal_msgid_info("[I2S Slave 3]0xA00A0038 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA00A0038)));
    log_hal_msgid_info("[I2S Slave 3]0xA00A0008 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA00A0008)));
    log_hal_msgid_info("[I2S Slave 3]0xA00A0030 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA00A0030)));

    log_hal_msgid_info("[APLL]", 0);
    log_hal_msgid_info("[APLL 1]0xA2050003 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050003)));
    log_hal_msgid_info("[APLL 1]0xA2050000 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050000)));
    log_hal_msgid_info("[APLL 1]0xA2050001 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050001)));
    log_hal_msgid_info("[APLL 1]0xA2050004 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050004)));
    log_hal_msgid_info("[APLL 1]0xA205002C = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA205002C)));

    log_hal_msgid_info("[APLL 2]0xA2050103 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050103)));
    log_hal_msgid_info("[APLL 2]0xA2050100 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050100)));
    log_hal_msgid_info("[APLL 2]0xA2050101 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050101)));
    log_hal_msgid_info("[APLL 2]0xA2050104 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050104)));
    log_hal_msgid_info("[APLL 2]0xA205012C = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA205012C)));

    log_hal_msgid_info("[APLL TUNER]", 0);
    log_hal_msgid_info("[APLL TUNER 1]0xA2050038 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050038)));
    log_hal_msgid_info("[APLL TUNER 1]0xA2050034 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050034)));
    log_hal_msgid_info("[APLL TUNER 2]0xA2050138 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050138)));
    log_hal_msgid_info("[APLL TUNER 2]0xA2050134 = 0x%x\r\n", 1, *((volatile uint32_t *)(0xA2050134)));

    log_hal_msgid_info("[AFE MEMIF]", 0);
    log_hal_msgid_info("[HD_MODE]0x700003F8 = 0x%x\r\n", 1, *((volatile uint32_t *)(0x700003F8)));
    log_hal_msgid_info("[HDALIGN]0x700003FC = 0x%x\r\n", 1, *((volatile uint32_t *)(0x700003FC)));
    log_hal_msgid_info("[MSB]0x700000CC= 0x%x\r\n", 1, *((volatile uint32_t *)(0x700000CC)));
}
#endif

#if defined(HAL_AUDIO_SUPPORT_APLL)

#define AFE_WRITE8(addr, val)   *((volatile uint8_t *)(addr)) = val
#define AFE_READ(addr)          *((volatile uint32_t *)(addr))
#define AFE_WRITE(addr, val)    *((volatile uint32_t *)(addr)) = val
#define AFE_SET_REG(addr, val, msk)  AFE_WRITE((addr), ((AFE_READ(addr) & (~(msk))) | ((val) & (msk))))
#define ReadREG(_addr)          (*(volatile uint32_t *)(_addr))

afe_apll_source_t afe_get_apll_by_samplerate(uint32_t samplerate)
{
    if (samplerate == 176400 || samplerate == 88200 || samplerate == 44100 || samplerate == 22050 || samplerate == 11025) {
        return AFE_APLL1;
    } else {
        return AFE_APLL2;
    }
}
#if 0//modify for ab1568
void afe_set_aplltuner(bool enable, afe_apll_source_t apll)
{
    log_hal_msgid_info("DSP afe_set_apll_for_i2s_reg APLL:%d, enable:%d\r\n", 2, apll, enable);
    if (true == enable) {
        // Clear upper layer audio CG
        //AFE_SET_REG(0xA2020238, 0x01020000, 0xFFFFFFFF);//CKSYS_CLK_CFG_2 //Selects clk_aud_interface0_sel APLL2_CK, 49.152(MHz) and clk_aud_interface1_sel APLL1_CK, 45.1584(MHz)

        switch (apll) {
            case AFE_APLL1:
                //Open APLL1
                clock_mux_sel(CLK_AUD_INTERFACE1_SEL, 1);//Selects clk_aud_interface1_sel APLL1_CK, 45.1584(MHz)
                //Setting APLL1 Tuner
                AFE_SET_REG(APLL1_CTL14__F_RG_APLL1_LCDDS_TUNER_PCW_NCPO, 0x0DE517A9, 0xFFFFFFFF);//[31:24] is integer  number, [23:0] is fractional number
                //AFE_SET_REG(APLL1_CTL12__F_RG_APLL1_LCDDS_TUNER_PCW_NCPO, 0x00000100, 0xFFFFFFFF);//DDS SSC enable
                AFE_SET_REG(APLL1_CTL13__F_RG_APLL1_LCDDS_TUNER_EN, 0x00000001, 0xFFFFFFFF);//DDS PCW tuner enable
                break;
            case AFE_APLL2:
                //Open APLL2
                clock_mux_sel(CLK_AUD_INTERFACE0_SEL, 2);//Selects clk_aud_interface0_sel APLL2_CK, 49.152(MHz)
                //Setting APLL2 Tuner
                AFE_SET_REG(APLL2_CTL14__F_RG_APLL2_LCDDS_TUNER_PCW_NCPO, 0x0F1FAA4C, 0xFFFFFFFF);//[31:24] is integer  number, [23:0] is fractional number
                //AFE_SET_REG(APLL2_CTL12__F_RG_APLL2_LCDDS_TUNER_PCW_NCPO, 0x00000100, 0xFFFFFFFF);//DDS SSC enable
                AFE_SET_REG(APLL2_CTL13__F_RG_APLL2_LCDDS_TUNER_EN, 0x00000001, 0xFFFFFFFF);//DDS PCW tuner enable
                break;
            default:
                log_hal_msgid_warning("I2S Master not turn on APLL1 and APLL2", 0);
                break;
        }
    } else {
        uint32_t set_value = 0;
        switch (apll) {
            case AFE_APLL1:
                // Disable APLL1 Tuner
                set_value = ReadREG(APLL1_CTL15__F_RG_APLL1_LCDDS_PCW_NCPO);                //[31:24] is integer  number, [23:0] is fractional number
                AFE_SET_REG(APLL1_CTL10__F_RG_APLL1_LCDDS_PCW_NCPO, set_value, 0xFFFFFFFF); //[31:24] is integer  number, [23:0] is fractional number
                //AFE_WRITE8(APLL1_CTL12__F_RG_APLL1_LCDDS_TUNER_PCW_NCPO, 0);
                AFE_SET_REG(APLL1_CTL13__F_RG_APLL1_LCDDS_TUNER_EN, 0x00000000, 0xFFFFFFFF);//DDS PCW tuner disable
                break;
            case AFE_APLL2:
                // Disable APLL2 Tuner
                set_value = ReadREG(APLL2_CTL15__F_RG_APLL2_LCDDS_PCW_NCPO);                //[31:24] is integer  number, [23:0] is fractional number
                AFE_SET_REG(APLL2_CTL10__F_RG_APLL2_LCDDS_PCW_NCPO, set_value, 0xFFFFFFFF); //[31:24] is integer  number, [23:0] is fractional number
                //AFE_WRITE8(APLL2_CTL12__F_RG_APLL2_LCDDS_TUNER_PCW_NCPO, 0);
                AFE_SET_REG(APLL2_CTL13__F_RG_APLL2_LCDDS_TUNER_EN, 0x00000000, 0xFFFFFFFF);//DDS PCW tuner disable
                break;
            default:
                log_hal_msgid_warning("I2S Master not turn off APLL1 and APLL2", 0);
                break;
        }
    }
}

#define AUDIO_TOP_CON0                          0x70000000
#define AFE_APLL1_TUNER_CFG                     0x700003f0
#define AFE_APLL2_TUNER_CFG                     0x700003f4

#define AUDIO_TOP_CON0_PDN_APLL2_TUNER_POS      (18)
#define AUDIO_TOP_CON0_PDN_APLL2_TUNER_MASK     (1<<AUDIO_TOP_CON0_PDN_APLL2_TUNER_POS)
#define AUDIO_TOP_CON0_PDN_APLL_TUNER_POS       (19)
#define AUDIO_TOP_CON0_PDN_APLL_TUNER_MASK      (1<<AUDIO_TOP_CON0_PDN_APLL_TUNER_POS)

#define MAX_TIMES  10000

void afe_aplltuner_clock_on(bool enable, afe_apll_source_t apll)
{
    uint32_t take_times = 0;
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);

    while (++take_times) {
        if (HAL_HW_SEMAPHORE_STATUS_OK == hal_hw_semaphore_take(HW_SEMAPHORE_APLL)) {
            break;
        }
        if (take_times > MAX_TIMES) {
            hal_nvic_restore_interrupt_mask(mask);
            //error handling
            log_hal_msgid_info("[SEMAPHORE] CM4 take semaphore %d fail.", 1, HW_SEMAPHORE_APLL);
            log_hal_msgid_info("[SEMAPHORE] CM4 take semaphore %d fail.", 0);
            assert(0);
            return;
        }
    }

    if (true == enable) {
        switch (apll) {
            case AFE_APLL1:
                AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_APLL_TUNER_POS, AUDIO_TOP_CON0_PDN_APLL_TUNER_MASK); //PDN control for apll tuner
                break;
            case AFE_APLL2:
                AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_APLL2_TUNER_POS, AUDIO_TOP_CON0_PDN_APLL2_TUNER_MASK);//PDN control for apll2 tuner
                break;
            default:
                break;
        }
    } else {
        switch (apll) {
            case AFE_APLL1:
                AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_APLL_TUNER_POS, AUDIO_TOP_CON0_PDN_APLL_TUNER_MASK);
                break;
            case AFE_APLL2:
                AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_APLL2_TUNER_POS, AUDIO_TOP_CON0_PDN_APLL2_TUNER_MASK);
                break;
            default:
                break;
        }
    }

    if (HAL_HW_SEMAPHORE_STATUS_OK == hal_hw_semaphore_give(HW_SEMAPHORE_APLL)) {
        hal_nvic_restore_interrupt_mask(mask);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
        //error handling
        log_hal_msgid_info("[SEMAPHORE] CM4 give semaphore %d fail.", 1, HW_SEMAPHORE_APLL);
        log_hal_msgid_info("[SEMAPHORE] CM4 give semaphore %d fail.", 0);
        assert(0);
    }
}

void afe_enable_apll_tuner(bool enable, afe_apll_source_t apll)
{
    if (true == enable) {
        switch (apll) {
            //Enable tuner
            case AFE_APLL1:
                AFE_SET_REG(AFE_APLL1_TUNER_CFG, 0x00000432, 0x0000FFF7);//AFE TUNER Control Register
                AFE_SET_REG(AFE_APLL1_TUNER_CFG, 0x1, 0x1);
                break;
            case AFE_APLL2:
                AFE_SET_REG(AFE_APLL2_TUNER_CFG, 0x00000434, 0x0000FFF7);//AFE TUNER2 Control Register
                AFE_SET_REG(AFE_APLL2_TUNER_CFG, 0x1, 0x1);
                break;
            default:
                break;
        }
    } else {
        switch (apll) {
            //Disable tuner
            case AFE_APLL1:
                AFE_SET_REG(AFE_APLL1_TUNER_CFG, 0x0, 0x1);
                break;
            case AFE_APLL2:
                AFE_SET_REG(AFE_APLL2_TUNER_CFG, 0x0, 0x1);
                break;
            default:
                break;
        }
    }
}
#endif
hal_audio_status_t hal_audio_apll_enable(bool enable, uint32_t samplerate)
{
    //uint32_t mask;
    //hal_nvic_save_and_set_interrupt_mask(&mask);
    hal_audio_status_t result = HAL_AUDIO_STATUS_OK;

    if (true == enable) {
        switch (afe_get_apll_by_samplerate(samplerate)) {
            case AFE_APLL1:
                aud_apll_1_cntr++;
                if (aud_apll_1_cntr == 1) {
#ifdef HAL_PMU_MODULE_ENABLED
#if 0 //modify for ab1568
                    pmu_vcore_lock_control(PMU_NORMAL, PMIC_VCORE_1P1_V, PMU_LOCK);
                    log_hal_msgid_info("frequency is risen to 1.1V for open apll1", 0);
#endif
#endif
                    log_hal_msgid_info("[APLL] TurnOnAPLL1, FS:%d, APLL1_CNT:%d", 2, samplerate, aud_apll_1_cntr);
#if 0
                    clock_set_pll_on(CLK_APLL1);
                    afe_set_aplltuner(true, AFE_APLL1);
                    afe_aplltuner_clock_on(true, AFE_APLL1);
                    afe_enable_apll_tuner(true, AFE_APLL1);
#else
                    //clock_mux_sel(CLK_AUD_INTERFACE0_SEL,2);
                    //hal_clock_enable(HAL_CLOCK_CG_AUD_INTF0);//modify for ab1568
                    clock_mux_sel(CLK_AUD_INTERFACE1_SEL, 2);
                    hal_clock_enable(HAL_CLOCK_CG_AUD_INTF1);//modify for ab1568

                    // Step 2 Enable APLL - Setting APLL tuner - use APLL1 :
                    //AFE_SET_REG(0xA2050038, 0x0DE517A9);
                    //AFE_SET_REG(0xA2050030, 0x00000100);
                    //AFE_SET_REG(0xA2050034, 0x00000001);
                    //APLL1 44.1k base
                    //AFE_SET_REG(AFE_APLL1_TUNER_CFG, 0x433);
#endif
                } else {
                    log_hal_msgid_info("[APLL] TurnOnAPLL1 again, FS:%d, APLL1_CNT:%d", 2, samplerate, aud_apll_1_cntr);
                }
                break;
            case AFE_APLL2:
                aud_apll_2_cntr++;
                if (aud_apll_2_cntr == 1) {
#ifdef HAL_PMU_MODULE_ENABLED
#if 0 //modify for ab1568
                    pmu_vcore_lock_control(PMU_NORMAL, PMIC_VCORE_1P1_V, PMU_LOCK);
                    log_hal_msgid_info("frequency is risen to 1.1V for open apll2", 0);
#endif
#endif
                    log_hal_msgid_info("[APLL] TurnOnAPLL2, FS:%d, APLL2_CNT:%d", 2, samplerate, aud_apll_2_cntr);
#if 0
                    clock_set_pll_on(CLK_APLL2);
                    afe_set_aplltuner(true, AFE_APLL2);
                    afe_aplltuner_clock_on(true, AFE_APLL2);
                    afe_enable_apll_tuner(true, AFE_APLL2);
#else
                    //clock_mux_sel(CLK_AUD_INTERFACE0_SEL,3);// 3 : APLL2_CK,    24.576 / 22.579 MHz
                    //hal_clock_enable(HAL_CLOCK_CG_AUD_INTF0);//modify for ab1568
                    clock_mux_sel(CLK_AUD_INTERFACE0_SEL, 3); // 3 : APLL2_CK,    24.576 / 22.579 MHz
                    hal_clock_enable(HAL_CLOCK_CG_AUD_INTF0);//modify for ab1568

                    // Step 2 Enable APLL - Setting APLL tuner - use APLL2 :
                    //AFE_SET_REG(0xA2050138, 0x0F1FAA4C);
                    //AFE_SET_REG(0xA2050130, 0x00000100);
                    //AFE_SET_REG(0xA2050134, 0x00000001);
                    //APLL2 48k base
                    //AFE_SET_REG(AFE_APLL1_TUNER_CFG, 0x435);
#endif
                } else {
                    log_hal_msgid_info("[APLL] TurnOnAPLL2 again, FS:%d, APLL2_CNT:%d", 2, samplerate, aud_apll_2_cntr);
                }
                break;
            default:
                result = HAL_AUDIO_STATUS_INVALID_PARAMETER;
                break;
        }
    } else {
        switch (afe_get_apll_by_samplerate(samplerate)) {
            case AFE_APLL1:
                aud_apll_1_cntr--;
                if (aud_apll_1_cntr == 0) {
#if 0//modify for ab1568
                    afe_enable_apll_tuner(false, AFE_APLL1);
                    afe_aplltuner_clock_on(false, AFE_APLL1);
                    afe_set_aplltuner(false, AFE_APLL1);
                    clock_set_pll_off(CLK_APLL1);
                    log_hal_msgid_info("[APLL] TurnOffAPLL1, FS:%d, APLL1_CNT:%d", 2, samplerate, aud_apll_1_cntr);
#endif
                    hal_clock_disable(HAL_CLOCK_CG_AUD_INTF1);//modify for ab1568
#ifdef HAL_PMU_MODULE_ENABLED
#if 0//modify for ab1568
                    pmu_vcore_lock_control(PMU_NORMAL, PMIC_VCORE_1P1_V, PMU_UNLOCK);
                    log_hal_msgid_info("frequency is risen to 1.1V for close apll1", 0);
#endif
#endif
                } else if (aud_apll_1_cntr < 0) {
                    log_hal_msgid_info("[APLL] Error, Already TurnOffAPLL1, FS:%d, APLL1_CNT:0", 1, samplerate);
                    aud_apll_1_cntr = 0;
                    result = HAL_AUDIO_STATUS_ERROR;
                } else {
                    log_hal_msgid_info("[APLL] TurnOffAPLL1 again, FS:%d, APLL1_CNT:%d", 2, samplerate, aud_apll_1_cntr);
                }
                break;
            case AFE_APLL2:
                aud_apll_2_cntr--;
                if (aud_apll_2_cntr == 0) {
#if 0//modify for ab1568
                    afe_enable_apll_tuner(false, AFE_APLL2);
                    afe_aplltuner_clock_on(false, AFE_APLL2);
                    afe_set_aplltuner(false, AFE_APLL2);
                    clock_set_pll_off(CLK_APLL2);
                    log_hal_msgid_info("[APLL] TurnOffAPLL2, FS:%d, APLL2_CNT:%d", 2, samplerate, aud_apll_2_cntr);
#endif
                    hal_clock_disable(HAL_CLOCK_CG_AUD_INTF0);//modify for ab1568
#ifdef HAL_PMU_MODULE_ENABLED
#if 0//modify for ab1568
                    pmu_vcore_lock_control(PMU_NORMAL, PMIC_VCORE_1P1_V, PMU_UNLOCK);
                    log_hal_msgid_info("frequency is risen to 1.1V for close apll1", 0);
#endif
#endif
                } else if (aud_apll_2_cntr < 0) {
                    log_hal_msgid_info("[APLL] Error, Already TurnOffAPLL2, FS:%d, APLL2_CNT:0", 1, samplerate);
                    aud_apll_2_cntr = 0;
                    result = HAL_AUDIO_STATUS_ERROR;
                } else {
                    log_hal_msgid_info("[APLL] TurnOffAPLL2 again, FS:%d, APLL2_CNT:%d", 2, samplerate, aud_apll_2_cntr);
                }
                break;
            default:
                result = HAL_AUDIO_STATUS_INVALID_PARAMETER;
                break;
        }
    }
    //hal_nvic_restore_interrupt_mask(mask);
    return result;
}

hal_audio_status_t hal_audio_query_apll_status(void)
{
    log_hal_msgid_info("[APLL] aud_apll_1_cntr=%d", 1, aud_apll_1_cntr);
    log_hal_msgid_info("[APLL] aud_apll_2_cntr=%d", 1, aud_apll_2_cntr);
    return HAL_AUDIO_STATUS_OK;
}

hal_audio_status_t hal_audio_mclk_enable(bool enable, afe_mclk_out_pin_t mclkoutpin, afe_apll_source_t apll, uint8_t divider)
{
    if (mclkoutpin != AFE_MCLK_PIN_FROM_I2S0 && mclkoutpin != AFE_MCLK_PIN_FROM_I2S1 && mclkoutpin != AFE_MCLK_PIN_FROM_I2S2 && mclkoutpin != AFE_MCLK_PIN_FROM_I2S3) {
        log_hal_msgid_info("[MCLK] not support mclkoutpin=%d", 1, mclkoutpin);
        return HAL_AUDIO_STATUS_INVALID_PARAMETER;
    }
    if (apll != AFE_APLL1 && apll != AFE_APLL2) {
        log_hal_msgid_info("[MCLK] not support apll=%d", 1, apll);
        return HAL_AUDIO_STATUS_INVALID_PARAMETER;
    }
    if (divider > 127) {
        log_hal_msgid_info("[MCLK] not support divider=%d", 1, divider);
        return HAL_AUDIO_STATUS_INVALID_PARAMETER;
    }
    mcu2dsp_open_param_t open_param;
    memset(&open_param, 0, sizeof(mcu2dsp_open_param_t));
    if (apll == AFE_APLL2) {
        open_param.stream_out_param.afe.sampling_rate = 48000;
    } else {
        open_param.stream_out_param.afe.sampling_rate = 44100;
    }

    if (enable) {
        if (mclk_status[mclkoutpin].status == MCLK_DISABLE) {
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_MCLK, &open_param, true);
            uint32_t clock_divider_reg;
            uint32_t clock_divider_shift;

            if ((mclkoutpin == AFE_MCLK_PIN_FROM_I2S0) || (mclkoutpin == AFE_MCLK_PIN_FROM_I2S1)) {
                clock_divider_reg = (uint32_t)CKSYS_BASE + 0x0308;
            } else {
                clock_divider_reg = (uint32_t)CKSYS_BASE + 0x030C;
            }
            if ((mclkoutpin == AFE_MCLK_PIN_FROM_I2S0) || (mclkoutpin == AFE_MCLK_PIN_FROM_I2S2)) {
                clock_divider_shift = 0;
            } else {
                clock_divider_shift = 16;
            }
            switch (apll) {
                case AFE_APLL1:
                    //hal_audio_apll_enable(true, 44100);
                    clock_mux_sel(CLK_MCLK_SEL, 0);
                    AFE_SET_REG((uint32_t)CKSYS_BASE + 0x0304, 0, 0x3 << (8 * mclkoutpin));                // I2S0/1/2/3 clock_source from APLL1
                    mclk_status[mclkoutpin].apll = AFE_APLL1;
                    break;
                case AFE_APLL2:
                default:
                    //hal_audio_apll_enable(true, 48000);
                    clock_mux_sel(CLK_MCLK_SEL, 1);
                    AFE_SET_REG((uint32_t)CKSYS_BASE + 0x0304, 0x1 << (8 * mclkoutpin), 0x3 << (8 * mclkoutpin)); // I2S0/1/2/3 clock_source from APLL2
                    mclk_status[mclkoutpin].apll = AFE_APLL2;
                    break;
            }
            // Setting audio clock divider   //Toggled to apply apll_ck_div bit-8 or bit-24
            //MCLK = clock_source/(1+n), n = [6:0], clock_source : AFE_I2S_SETTING_MCLK_SOURCE, n : AFE_I2S_SETTING_MCLK_DIVIDER)
#if 0
            bool toggled_bit = 0;
            toggled_bit = (ReadREG(clock_divider_reg) & (0x00000100 << clock_divider_shift)) >> 8;
            if (toggled_bit == true) {
                AFE_SET_REG(clock_divider_reg, divider << clock_divider_shift, 0x17f << clock_divider_shift);
            } else {
                AFE_SET_REG(clock_divider_reg, (divider | 0x00000100) << clock_divider_shift, 0x17f << clock_divider_shift);
            }
#else

            AFE_SET_REG(clock_divider_reg, divider << clock_divider_shift, 0x17f << clock_divider_shift);
            AFE_SET_REG(clock_divider_reg, (divider | 0x00000100) << clock_divider_shift, 0x17f << clock_divider_shift);
            AFE_SET_REG(clock_divider_reg, divider << clock_divider_shift, 0x17f << clock_divider_shift);
#endif
            //Power on apll12_div0/1/2/3 divider
            AFE_SET_REG((uint32_t)CKSYS_BASE + 0x0300, 0 << (8 * mclkoutpin), 1 << (8 * mclkoutpin));
            mclk_status[mclkoutpin].mclk_cntr++;
            mclk_status[mclkoutpin].divider = divider;
            mclk_status[mclkoutpin].status = MCLK_ENABLE;
            log_hal_msgid_info("[MCLK] TurnOnMCLK[%d], apll%d with divider%d, MCLK_CNT=%d", 4, mclkoutpin, apll, divider, mclk_status[mclkoutpin].mclk_cntr);
            return HAL_AUDIO_STATUS_OK;
        } else {
            if ((mclk_status[mclkoutpin].apll == apll) && (mclk_status[mclkoutpin].divider == divider)) {
                mclk_status[mclkoutpin].mclk_cntr++;
                log_hal_msgid_info("[MCLK] TurnOnMCLK[%d], apll%d with divider%d again, MCLK_CNT=%d", 4, mclkoutpin, apll, divider, mclk_status[mclkoutpin].mclk_cntr);
                return HAL_AUDIO_STATUS_OK;
            } else {
                log_hal_msgid_info("[MCLK] Error, Already TurnOnMCLK[%d] apll%d with divider%d, Request apll%d with divider%d is invalid", 5, mclkoutpin, mclk_status[mclkoutpin].apll, mclk_status[mclkoutpin].divider, apll, divider);
                return HAL_AUDIO_STATUS_ERROR;
            }
        }
    } else {
        if (mclk_status[mclkoutpin].status == MCLK_ENABLE) {
            if ((mclk_status[mclkoutpin].apll == apll) && (mclk_status[mclkoutpin].divider == divider)) {
                mclk_status[mclkoutpin].mclk_cntr--;

                if (mclk_status[mclkoutpin].mclk_cntr == 0) {
                    AFE_SET_REG((uint32_t)CKSYS_BASE + 0x0300, 1 << (8 * mclkoutpin), 1 << (8 * mclkoutpin));
                    if (mclk_status[mclkoutpin].apll == AFE_APLL1) {
                        //hal_audio_apll_enable(false, 44100);
                    } else {
                        //hal_audio_apll_enable(false, 48000);
                    }
                    mclk_status[mclkoutpin].status = MCLK_DISABLE;
                    mclk_status[mclkoutpin].mclk_cntr = 0;
                    mclk_status[mclkoutpin].apll = AFE_APLL_NONE;
                    mclk_status[mclkoutpin].divider = 0;
                    log_hal_msgid_info("[MCLK] TurnOffMCLK[%d], apll%d with divider%d, MCLK_CNT=%d", 4, mclkoutpin, apll, divider, mclk_status[mclkoutpin].mclk_cntr);
                    if (mclk_status[AFE_MCLK_PIN_FROM_I2S0].mclk_cntr == 0 && mclk_status[AFE_MCLK_PIN_FROM_I2S1].mclk_cntr == 0 && mclk_status[AFE_MCLK_PIN_FROM_I2S2].mclk_cntr == 0 && mclk_status[AFE_MCLK_PIN_FROM_I2S3].mclk_cntr == 0) {
                        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_MCLK, &open_param, false);
                    }
                    return HAL_AUDIO_STATUS_OK;
                } else if (mclk_status[mclkoutpin].mclk_cntr < 0) {
                    log_hal_msgid_info("[MCLK] Error, Already TurnOffMCLK[%d], apll%d with divider%d, MCLK_CNT=%d", 4, mclkoutpin, apll, divider, mclk_status[mclkoutpin].mclk_cntr);
                    mclk_status[mclkoutpin].mclk_cntr = 0;
                    return HAL_AUDIO_STATUS_ERROR;
                } else {
                    log_hal_msgid_info("[MCLK] TurnOffMCLK[%d], apll%d with divider%d again, MCLK_CNT=%d", 4, mclkoutpin, apll, divider, mclk_status[mclkoutpin].mclk_cntr);
                    return HAL_AUDIO_STATUS_OK;
                }
            } else {
                log_hal_msgid_info("[MCLK] Error, Already TurnOnMCLK[%d] apll%d with divider%d, Request TurnOffMCLK apll%d with divider%d is invalid", 5, mclkoutpin, mclk_status[mclkoutpin].apll, mclk_status[mclkoutpin].divider, apll, divider);
                return HAL_AUDIO_STATUS_ERROR;
            }
        } else {
            log_hal_msgid_info("[MCLK] Already TurnOffMCLK[%d]", 1, mclkoutpin);
            return HAL_AUDIO_STATUS_ERROR;
        }
    }
}

hal_audio_status_t hal_audio_query_mclk_status(void)
{
    uint8_t i = 0;
    for (i = 0; i < 4; i++) {
        log_hal_msgid_info("[MCLK] mclk_status[%d].status=%d", 2, i, mclk_status[i].status);
        log_hal_msgid_info("[MCLK] mclk_status[%d].mclk_cntr=%d", 2, i, mclk_status[i].mclk_cntr);
        log_hal_msgid_info("[MCLK] mclk_status[%d].apll=%d", 2, i, mclk_status[i].apll);
        log_hal_msgid_info("[MCLK] mclk_status[%d].divider=%d", 2, i, mclk_status[i].divider);
    }
    return HAL_AUDIO_STATUS_OK;
}
#endif

#ifdef HAL_AUDIO_ANC_ENABLE
extern void ami_hal_audio_status_set_running_flag(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool is_running);
extern bool ami_hal_audio_status_query_running_flag(audio_scenario_type_t type);
uint32_t hal_audio_get_anc_talk_mic_settings(void)
{
    uint32_t talk_mic = 6; //None-type
    mcu2dsp_open_stream_in_param_t in_param;
    memset(&in_param, 0, sizeof(mcu2dsp_open_stream_in_param_t));
    hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &in_param);
    switch (in_param.afe.audio_device) {
        case HAL_AUDIO_DEVICE_MAIN_MIC_L: {
            if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_1) {
                talk_mic = 0;
            } else if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_2) {
                talk_mic = 2;
            } else if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_3) {
                talk_mic = 4;
            }
            break;
        }
        case HAL_AUDIO_DEVICE_MAIN_MIC_R: {
            if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_1) {
                talk_mic = 1;
            } else if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_2) {
                talk_mic = 3;
            } else if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_3) {
                talk_mic = 5;
            }
            break;
        }
        case HAL_AUDIO_DEVICE_DIGITAL_MIC_L: {
            if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_1) {
                talk_mic = 8;
            } else if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_2) {
                talk_mic = 10;
            } else if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_3) {
                talk_mic = 12;
            }
            break;
        }
        case HAL_AUDIO_DEVICE_DIGITAL_MIC_R: {
            if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_1) {
                talk_mic = 9;
            } else if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_2) {
                talk_mic = 11;
            } else if (in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_3) {
                talk_mic = 13;
            }
            break;
        }
        default:
            break;
    }
    log_hal_msgid_info("[ANC_GET] Call(0x%x)(0x%x), mic_type(%d)", 3, in_param.afe.audio_device, in_param.afe.audio_interface, talk_mic);
    return talk_mic;
}

hal_audio_status_t hal_audio_notify_anc_afe_settings(bool isEnable)
{
    if (isEnable) {
        void *p_param_share;
        mcu2dsp_open_param_p adda_param = pvPortMalloc(sizeof(mcu2dsp_open_param_t));
        if (adda_param != NULL) {
            memset(adda_param, 0, sizeof(mcu2dsp_open_param_t));
            hal_audio_get_stream_in_setting_config(AU_DSP_ANC, &(adda_param->stream_in_param));
            hal_audio_get_stream_out_setting_config(AU_DSP_ANC, &(adda_param->stream_out_param));
            adda_param->stream_in_param.afe.sampling_rate  = 16000;
#ifdef AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ
            adda_param->stream_out_param.afe.sampling_rate = HAL_AUDIO_FIXED_AFE_96K_SAMPLE_RATE;
#else
            adda_param->stream_out_param.afe.sampling_rate = HAL_AUDIO_FIXED_AFE_48K_SAMPLE_RATE;
#endif
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE) || defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)
            adda_param->stream_in_param.afe.with_bias_lowpower = AUDIO_MICBIAS_LOW_POWER_MODE;
#endif

            //Set running flag
#if 1
            if (!ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_ANC)) {
                ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_ANC, adda_param, true);
                hal_clock_enable(HAL_CLOCK_CG_AUD_ANC);
            }
#endif
            p_param_share = hal_audio_dsp_controller_put_paramter(adda_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_ANC);

            //hal_audio_dsp_dl_clkmux_control(AUDIO_MESSAGE_TYPE_ANC, adda_param->stream_out_param.afe.audio_device, adda_param->stream_out_param.afe.sampling_rate, TRUE);
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_ANC_SET_PARAM, (unsigned short)(4 << 12), (uint32_t)p_param_share, true);
            vPortFree(adda_param);
        } else {
            log_hal_msgid_error("[ANC_API] Error!!! malloc fail for afe_settings_notify", 0);
            return HAL_AUDIO_STATUS_ERROR;
        }
    }
#if 0
    else {
        //hal_audio_dsp_dl_clkmux_control(AUDIO_MESSAGE_TYPE_ANC, 0, 0, FALSE);
    }
#endif
    return HAL_AUDIO_STATUS_OK;
}
#endif

#ifdef LINE_IN_PURE_FOR_AMIC_CLASS_G_HQA
static linein_playback_state_hqa_t linein_status;
linein_result_hqa_t audio_pure_linein_playback_open_HQA(hal_audio_sampling_rate_t linein_sample_rate, hal_audio_device_t in_audio_device, hal_audio_interface_t device_in_interface_HQA, hal_audio_analog_mdoe_t adc_mode_HQA, hal_audio_performance_mode_t mic_performance_HQA, hal_audio_device_t out_audio_device, hal_audio_performance_mode_t dac_performance_HQA)
{
    if (audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Mode_Sel == 0x0) {
        if (dac_performance_HQA == AFE_PEROFRMANCE_LOW_POWER_MODE) {
            audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Mode_Sel = 0x3;
        }
    }

    log_hal_msgid_info("enter pure_linein_playback_open_HQA, sample_rate=%d, in_audio_device=0x%x, out_audio_device=0x%x\n", 3, hal_audio_sampling_rate_enum_to_value(linein_sample_rate), in_audio_device, out_audio_device);
    if (linein_status != LINEIN_STATE_HQA_IDLE) {
        log_hal_msgid_info("cannot open because state(%d)\n", 1, linein_status);
        return LINEIN_EXECUTION_HQA_FAIL;
    }

    if (linein_sample_rate != HAL_AUDIO_SAMPLING_RATE_16KHZ && linein_sample_rate != HAL_AUDIO_SAMPLING_RATE_48KHZ && linein_sample_rate != HAL_AUDIO_SAMPLING_RATE_96KHZ && linein_sample_rate != HAL_AUDIO_SAMPLING_RATE_192KHZ) {
        log_hal_msgid_info("not support sample rate=%d\n", 1, hal_audio_sampling_rate_enum_to_value(linein_sample_rate));
        return LINEIN_EXECUTION_HQA_FAIL;
    }
    if (in_audio_device != HAL_AUDIO_DEVICE_I2S_MASTER && in_audio_device != HAL_AUDIO_DEVICE_I2S_SLAVE && in_audio_device != HAL_AUDIO_DEVICE_SPDIF && ((in_audio_device & HAL_AUDIO_DEVICE_MAIN_MIC_DUAL) == false)
        && ((in_audio_device & HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL) == false) && ((in_audio_device & HAL_AUDIO_DEVICE_DIGITAL_MIC_DUAL) == false)) {
        log_hal_msgid_info("not support in device=%d\n", 1, in_audio_device);
        return LINEIN_EXECUTION_HQA_FAIL;
    }

    if (out_audio_device != HAL_AUDIO_DEVICE_I2S_MASTER && out_audio_device != HAL_AUDIO_DEVICE_I2S_SLAVE && out_audio_device != HAL_AUDIO_DEVICE_SPDIF && ((out_audio_device & HAL_AUDIO_DEVICE_DAC_DUAL) == false)) {
        log_hal_msgid_info("not support out device=%d\n", 1, out_audio_device);
        return LINEIN_EXECUTION_HQA_FAIL;
    }

    mcu2dsp_open_param_t open_param;
    memset(&open_param, 0, sizeof(mcu2dsp_open_param_t));
    void *p_param_share;

    open_param.param.stream_in = STREAM_IN_AFE;
    open_param.param.stream_out = STREAM_OUT_AFE;
    open_param.audio_scenario_type = AUDIO_SCENARIO_TYPE_LINE_IN;

    open_param.stream_in_param.afe.audio_interface = device_in_interface_HQA;

#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    open_param.stream_in_param.afe.performance = mic_performance_HQA;
    open_param.stream_in_param.afe.bias_voltage[0] = HAL_AUDIO_BIAS_VOLTAGE_2_40V;
    open_param.stream_in_param.afe.bias_voltage[1] = HAL_AUDIO_BIAS_VOLTAGE_2_40V;
    open_param.stream_in_param.afe.bias_voltage[2] = HAL_AUDIO_BIAS_VOLTAGE_2_40V;
    open_param.stream_in_param.afe.bias_voltage[3] = HAL_AUDIO_BIAS_VOLTAGE_2_40V;
    open_param.stream_in_param.afe.bias_voltage[4] = HAL_AUDIO_BIAS_VOLTAGE_2_40V;
    open_param.stream_in_param.afe.bias_select = audio_nvdm_HW_config.voice_scenario.Voice_MIC_Bias_Enable;
    open_param.stream_in_param.afe.iir_filter[0] = HAL_AUDIO_UL_IIR_DISABLE;
#ifdef AIR_BTA_IC_PREMIUM_G2
    if ((adc_mode_HQA == HAL_AUDIO_ANALOG_INPUT_ACC10K)) {
#else //AIR_BTA_IC_PREMIUM_G3
    if ((adc_mode_HQA == HAL_AUDIO_ANALOG_INPUT_ACC10K) || (adc_mode_HQA == HAL_AUDIO_ANALOG_INPUT_ACC10K_SINGLE)) {
#endif
        open_param.stream_in_param.afe.adc_mode = HAL_AUDIO_ANALOG_INPUT_ACC10K;
#ifdef AIR_BTA_IC_PREMIUM_G2
    } else if ((adc_mode_HQA == HAL_AUDIO_ANALOG_INPUT_ACC20K)) {
#else //AIR_BTA_IC_PREMIUM_G3
    } else if ((adc_mode_HQA == HAL_AUDIO_ANALOG_INPUT_ACC20K) || (adc_mode_HQA == HAL_AUDIO_ANALOG_INPUT_ACC20K_SINGLE)) {
#endif
        open_param.stream_in_param.afe.adc_mode = HAL_AUDIO_ANALOG_INPUT_ACC20K;
    }
#ifndef AIR_BTA_IC_PREMIUM_G2 //AIR_BTA_IC_PREMIUM_G3
    if ((adc_mode_HQA == HAL_AUDIO_ANALOG_INPUT_ACC10K) || (adc_mode_HQA == HAL_AUDIO_ANALOG_INPUT_ACC20K)) {
        open_param.stream_in_param.afe.adc_type = HAL_AUDIO_ANALOG_TYPE_DIFF;
    } else if ((adc_mode_HQA == HAL_AUDIO_ANALOG_INPUT_ACC10K_SINGLE) || (adc_mode_HQA == HAL_AUDIO_ANALOG_INPUT_ACC20K_SINGLE)) {
        open_param.stream_in_param.afe.adc_type = HAL_AUDIO_ANALOG_TYPE_SINGLE;
    }
#endif
    open_param.stream_in_param.afe.dmic_selection[0] = HAL_AUDIO_DMIC_GPIO_DMIC0;
    if ((in_audio_device == HAL_AUDIO_DEVICE_DIGITAL_MIC_DUAL)) {
        if ((int)adc_mode_HQA == (int)HAL_AUDIO_DMIC_GPIO_DMIC0) {
            open_param.stream_in_param.afe.dmic_selection[0] = HAL_AUDIO_DMIC_GPIO_DMIC0;
        } else if ((int)adc_mode_HQA == (int)HAL_AUDIO_DMIC_GPIO_DMIC1) {
            open_param.stream_in_param.afe.dmic_selection[0] = HAL_AUDIO_DMIC_GPIO_DMIC1;
        } else if ((int)adc_mode_HQA == (int)HAL_AUDIO_DMIC_ANA_DMIC0) {
            open_param.stream_in_param.afe.dmic_selection[0] = HAL_AUDIO_DMIC_ANA_DMIC0;
        } else if ((int)adc_mode_HQA == (int)HAL_AUDIO_DMIC_ANA_DMIC1) {
            open_param.stream_in_param.afe.dmic_selection[0] = HAL_AUDIO_DMIC_ANA_DMIC1;
        } else if ((int)adc_mode_HQA == (int)HAL_AUDIO_DMIC_ANA_DMIC2) {
            open_param.stream_in_param.afe.dmic_selection[0] = HAL_AUDIO_DMIC_ANA_DMIC2;
        } else if ((int)adc_mode_HQA == (int)HAL_AUDIO_DMIC_ANA_DMIC3) {
            open_param.stream_in_param.afe.dmic_selection[0] = HAL_AUDIO_DMIC_ANA_DMIC3;
        }
        log_hal_msgid_info("[Dmic_HQA] adc_mode/DMIC_pin %d, dmic_selection %d", 2, adc_mode_HQA, open_param.stream_in_param.afe.dmic_selection[0]);
    }
#endif

    open_param.stream_in_param.afe.audio_device = in_audio_device;
    open_param.stream_in_param.afe.sampling_rate = hal_audio_sampling_rate_enum_to_value(linein_sample_rate);
    if (open_param.stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
        open_param.stream_in_param.afe.misc_parms      = I2S_CLK_SOURCE_DCXO;
        if (linein_sample_rate > HAL_AUDIO_SAMPLING_RATE_48KHZ) {
            open_param.stream_in_param.afe.misc_parms  = I2S_CLK_SOURCE_APLL;
        }
    } else {
        if (in_audio_device & HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL) {
            open_param.stream_in_param.afe.misc_parms      = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_2p4v;
        } else {
            open_param.stream_in_param.afe.misc_parms      = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_1p85v;
        }
    }

    open_param.stream_out_param.afe.audio_device = out_audio_device;
    //open_param.stream_out_param.afe.audio_interface = HAL_AUDIO_INTERFACE_1;
    open_param.stream_out_param.afe.audio_interface = device_in_interface_HQA;
    open_param.stream_out_param.afe.stream_channel = HAL_AUDIO_DIRECT;
    //LINEIN_PLAYBACK_LOG_I("[Factory Test] Loopback headset setting ", 0);

    open_param.stream_out_param.afe.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
    open_param.stream_out_param.afe.stream_out_sampling_rate = hal_audio_sampling_rate_enum_to_value(linein_sample_rate);
#if defined (FIXED_SAMPLING_RATE_TO_48KHZ)
    open_param.stream_out_param.afe.sampling_rate   = HAL_AUDIO_FIXED_AFE_48K_SAMPLE_RATE;
#elif defined (AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
    open_param.stream_out_param.afe.sampling_rate   = HAL_AUDIO_FIXED_AFE_96K_SAMPLE_RATE;
#else
    open_param.stream_out_param.afe.sampling_rate   = hal_audio_sampling_rate_enum_to_value(linein_sample_rate);
#endif
    open_param.stream_in_param.afe.sampling_rate = open_param.stream_out_param.afe.sampling_rate;
    open_param.stream_out_param.afe.hw_gain = true;
    if (open_param.stream_in_param.afe.audio_device & HAL_AUDIO_DEVICE_I2S_MASTER) {
        if (open_param.stream_in_param.afe.audio_interface & HAL_AUDIO_INTERFACE_1) {
            open_param.stream_in_param.afe.i2s_master_sampling_rate[0] = open_param.stream_in_param.afe.sampling_rate;
        } else if (open_param.stream_in_param.afe.audio_interface & HAL_AUDIO_INTERFACE_2) {
            open_param.stream_in_param.afe.i2s_master_sampling_rate[1] = open_param.stream_in_param.afe.sampling_rate;
        } else if (open_param.stream_in_param.afe.audio_interface & HAL_AUDIO_INTERFACE_3) {
            open_param.stream_in_param.afe.i2s_master_sampling_rate[2] = open_param.stream_in_param.afe.sampling_rate;
        } else if (open_param.stream_in_param.afe.audio_interface & HAL_AUDIO_INTERFACE_4) {
            open_param.stream_in_param.afe.i2s_master_sampling_rate[3] = open_param.stream_in_param.afe.sampling_rate;
        }
    }
    if (open_param.stream_out_param.afe.audio_device & HAL_AUDIO_DEVICE_I2S_MASTER) {
        open_param.stream_out_param.afe.misc_parms      = I2S_CLK_SOURCE_DCXO;
        if (linein_sample_rate > HAL_AUDIO_SAMPLING_RATE_48KHZ) {
            open_param.stream_out_param.afe.misc_parms  = I2S_CLK_SOURCE_APLL;
        }
        if (open_param.stream_out_param.afe.audio_interface & HAL_AUDIO_INTERFACE_1) {
            open_param.stream_out_param.afe.i2s_master_sampling_rate[0] = open_param.stream_out_param.afe.sampling_rate;
        } else if (open_param.stream_out_param.afe.audio_interface & HAL_AUDIO_INTERFACE_2) {
            open_param.stream_out_param.afe.i2s_master_sampling_rate[1] = open_param.stream_out_param.afe.sampling_rate;
        } else if (open_param.stream_out_param.afe.audio_interface & HAL_AUDIO_INTERFACE_3) {
            open_param.stream_out_param.afe.i2s_master_sampling_rate[2] = open_param.stream_out_param.afe.sampling_rate;
        } else if (open_param.stream_out_param.afe.audio_interface & HAL_AUDIO_INTERFACE_4) {
            open_param.stream_out_param.afe.i2s_master_sampling_rate[3] = open_param.stream_out_param.afe.sampling_rate;
        }
    } else {
        open_param.stream_out_param.afe.misc_parms      = DOWNLINK_PERFORMANCE_NORMAL;
    }

    open_param.stream_out_param.afe.performance = dac_performance_HQA;

#if 0
    if ((in_audio_device & HAL_AUDIO_DEVICE_I2S_MASTER) || (out_audio_device & HAL_AUDIO_DEVICE_I2S_MASTER)) {
        if (open_param.stream_in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_1) {
            hal_gpio_init(2);
            hal_pinmux_set_function(2, 3);
            hal_gpio_init(3);
            hal_pinmux_set_function(3, 3);
            hal_gpio_init(4);
            hal_pinmux_set_function(4, 3);
            hal_gpio_init(6);
            hal_pinmux_set_function(6, 3);
            hal_gpio_init(5);
            hal_pinmux_set_function(5, 3); //O:I2S_MST0_MCLK
            //hal_audio_mclk_enable(true, AFE_MCLK_PIN_FROM_I2S0, AFE_APLL2, 0);//enable mclk
            LINEIN_PLAYBACK_LOG_I("I2S0 GPIO Set done\r\n", 0);
        } else if (open_param.stream_in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_2) {
            hal_gpio_init(16);
            hal_pinmux_set_function(16, 3);
            hal_gpio_init(26);
            hal_pinmux_set_function(26, 3);
            hal_gpio_init(17);
            hal_pinmux_set_function(17, 3);
            hal_gpio_init(18);
            hal_pinmux_set_function(18, 3);
            hal_gpio_init(15);
            hal_pinmux_set_function(15, 3); //O:I2S_MST1_MCLK
            //hal_audio_mclk_enable(true, AFE_MCLK_PIN_FROM_I2S1, AFE_APLL2, 0);//enable mclk
            LINEIN_PLAYBACK_LOG_I("I2S1 GPIO Set done\r\n", 0);
        } else if (open_param.stream_in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_3) {
            hal_gpio_init(29);
            hal_pinmux_set_function(29, 1);
            hal_gpio_init(32);
            hal_pinmux_set_function(32, 1);
            hal_gpio_init(33);
            hal_pinmux_set_function(33, 1);
            hal_gpio_init(37);
            hal_pinmux_set_function(37, 1);
            hal_gpio_init(31);
            hal_pinmux_set_function(31, 1); //O:I2S_MST2_MCLK
            //hal_audio_mclk_enable(true, AFE_MCLK_PIN_FROM_I2S2, AFE_APLL2, 0);//enable mclk
            LINEIN_PLAYBACK_LOG_I("I2S2 GPIO Set done\r\n", 0);
        } else if (open_param.stream_in_param.afe.audio_interface == HAL_AUDIO_INTERFACE_4) {
            hal_gpio_init(38);
            hal_pinmux_set_function(38, 1);
            hal_gpio_init(39);
            hal_pinmux_set_function(39, 1);
            hal_gpio_init(40);
            hal_pinmux_set_function(40, 1);
            hal_gpio_init(43);
            hal_pinmux_set_function(43, 1);
            hal_gpio_init(41);
            hal_pinmux_set_function(41, 1); //O:I2S_MST3_MCLK
            //hal_audio_mclk_enable(true, AFE_MCLK_PIN_FROM_I2S3, AFE_APLL2, 0);//enable mclk
            LINEIN_PLAYBACK_LOG_I("I2S3 GPIO Set done\r\n", 0);
        }
    }

    if (in_audio_device & HAL_AUDIO_DEVICE_DIGITAL_MIC_DUAL) {
        if ((uint32_t)adc_mode_HQA == (uint32_t)HAL_AUDIO_DMIC_GPIO_DMIC0) {
            //2822
            //GPIO2 AuxFunc.3 DMIC0_CLK
            //GPIO3 AuxFunc.3 DMIC0_DAT
            hal_gpio_init(2);
            hal_pinmux_set_function(2, 3);
            hal_gpio_init(3);
            hal_pinmux_set_function(3, 3);
            LINEIN_PLAYBACK_LOG_I("DMIC0 GPIO Set done dmic sel %d\r\n", 1, adc_mode_HQA);
        } else if ((uint32_t)adc_mode_HQA == (uint32_t)HAL_AUDIO_DMIC_GPIO_DMIC1) {
            //2822
            //GPIO4 AuxFunc.3 DMIC0_CLK
            //GPIO5 AuxFunc.3 DMIC0_DAT
            hal_gpio_init(4);
            hal_pinmux_set_function(4, 3);
            hal_gpio_init(5);
            hal_pinmux_set_function(5, 3);
            LINEIN_PLAYBACK_LOG_I("DMIC1 GPIO Set done dmic sel %d\r\n", 1, adc_mode_HQA);
        }
    }
#endif

#if defined(MTK_EXTERNAL_DSP_NEED_SUPPORT)
    ami_set_afe_param(STREAM_OUT, linein_sample_rate, true);
#endif
    p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_LINEIN);
    ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_LINE_IN, &open_param, true);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_TRULY_LINEIN_PLAYBACK_OPEN, 0, (uint32_t)p_param_share, true);

    linein_status = LINEIN_STATE_HQA_PLAY;
    return LINEIN_EXECUTION_HQA_SUCCESS;
}

linein_result_hqa_t audio_pure_linein_playback_close_HQA()
{
    log_hal_msgid_info("enter pure_linein_playback_close\n", 0);
    if (linein_status != LINEIN_STATE_HQA_PLAY) {
        log_hal_msgid_info("cannot close because state(%d)\n", 1, linein_status);
        return LINEIN_EXECUTION_HQA_FAIL;
    }
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_TRULY_LINEIN_PLAYBACK_CLOSE, 0, 0, true);
    linein_status = LINEIN_STATE_HQA_IDLE;
    ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_LINE_IN, NULL, false);
    return LINEIN_EXECUTION_HQA_SUCCESS;
}
#endif

#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
void hal_audio_status_change_dac_mode_config(uint32_t new_dac_mode)
{
    if (hal_audio_status_get_clock_gate_status(AUDIO_POWER_DAC)) { // Power on DAC

        /***************** Power Off DAC ******************/
        hal_audio_status_enable_dac(false);

        /*************** Change DAC Mode *****************/
        audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Mode_Sel = new_dac_mode;

        /***************** Power On DAC ******************/
        hal_audio_status_enable_dac(true);

    }else {
        audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Mode_Sel = new_dac_mode;
    }
}

void hal_audio_status_change_dac_mode_handler(void)
{
    if(targe_dac_mode) {
        hal_audio_status_change_dac_mode_config(*targe_dac_mode);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_DAC_EXIT_DEACTIVE_MODE, *(uint16_t*)targe_dac_mode, 0, true);
        vPortFreeNC(targe_dac_mode);
        targe_dac_mode = NULL;
    }
}

bool hal_audio_status_change_dac_mode(uint32_t dac_mode)
{
    bool prc_result = false;

    if(dac_mode != audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Mode_Sel && (targe_dac_mode == NULL)) {
        targe_dac_mode  = (uint32_t *)pvPortMallocNC(sizeof(uint32_t));
        *targe_dac_mode = dac_mode;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_DAC_ENTER_DEACTIVE_MODE, 0, 0, false);
        prc_result = true;
    }

    return prc_result;
}

#endif  /*AIR_DAC_MODE_RUNTIME_CHANGE*/

#endif /* defined(HAL_AUDIO_MODULE_ENABLED) */
