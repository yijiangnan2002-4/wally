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
#include "scenario_tdm.h"

#ifdef MTK_TDM_ENABLE
#if defined(MTK_AVM_DIRECT)
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
#endif
#endif

/*------------------------------------------------PORT----AIR_AUDIO_I2S_SLAVE_TDM_ENABLE------------------------------------------------------------------*/
#if defined(AIR_AUDIO_I2S_SLAVE_TDM_ENABLE)
void audio_transmitter_tdm_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    open_param->param.stream_in  = STREAM_IN_AFE;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_VIRTUAL_OUT_ENABLE
    open_param->param.stream_out = STREAM_OUT_VIRTUAL;
#else
    open_param->param.stream_out = STREAM_OUT_AFE;
#endif
    hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &open_param->stream_in_param);
    hal_audio_get_stream_out_setting_config(AU_DSP_VOICE, &open_param->stream_out_param);
    if (config->scenario_config.tdm_config.frame_size != 32 && config->scenario_config.tdm_config.frame_size != 64) {
        configASSERT(0 && "[TDM] frame size is only support 32 and 64!");
    }
    if (config->scenario_config.tdm_config.in_channel <= HAL_AUDIO_I2S_TDM_DISABLE || config->scenario_config.tdm_config.in_channel > HAL_AUDIO_I2S_TDM_8CH) {
        configASSERT(0 && "[TDM] TDM in ch invalid");
    }
    if (config->scenario_config.tdm_config.out_channel < HAL_AUDIO_I2S_TDM_DISABLE || config->scenario_config.tdm_config.out_channel > HAL_AUDIO_I2S_TDM_8CH) {
        configASSERT(0 && "[TDM] TDM out ch invalid");
    }
    if (config->scenario_config.tdm_config.out_channel == HAL_AUDIO_I2S_TDM_DISABLE) {
        open_param->param.stream_out = STREAM_OUT_VIRTUAL;
    }
    TRANSMITTER_LOG_I("[TDM]rate=%d,frame size=%d,in ch=%d,out ch=%d", 4,
                        hal_audio_sampling_rate_enum_to_value(config->scenario_config.tdm_config.sampling_rate),
                        config->scenario_config.tdm_config.frame_size,
                        config->scenario_config.tdm_config.in_channel * 2,
                        config->scenario_config.tdm_config.out_channel * 2);
#if 0 //i2s2 for tdm
    hal_gpio_init(41);
    hal_pinmux_set_function(41, 2); //clk
    hal_gpio_init(43);
    hal_pinmux_set_function(43, 2); //ws
    hal_gpio_init(42);
    hal_pinmux_set_function(42, 2); //tx
    hal_gpio_init(34);
    hal_pinmux_set_function(34, 2); //rx
#endif
    open_param->stream_in_param.afe.audio_device = HAL_AUDIO_DEVICE_I2S_SLAVE;
    open_param->stream_in_param.afe.memory = config->scenario_config.tdm_config.in_memory;//HAL_AUDIO_MEM1
    open_param->stream_in_param.afe.audio_interface = config->scenario_config.tdm_config.in_interface;//HAL_AUDIO_INTERFACE_1
    open_param->stream_in_param.afe.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
    open_param->stream_in_param.afe.sampling_rate = hal_audio_sampling_rate_enum_to_value(config->scenario_config.tdm_config.sampling_rate);
    open_param->stream_in_param.afe.irq_period = 0;
    open_param->stream_in_param.afe.frame_size = config->scenario_config.tdm_config.frame_size;
    open_param->stream_in_param.afe.frame_number = 4;
    open_param->stream_in_param.afe.hw_gain = false;
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    //I2S Slave parameters
    open_param->stream_in_param.afe.i2s_format = HAL_AUDIO_I2S_I2S;
    open_param->stream_in_param.afe.i2s_word_length = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;
    open_param->stream_in_param.afe.i2S_Slave_TDM = config->scenario_config.tdm_config.in_channel;
#endif
    open_param->stream_out_param.afe.audio_device = HAL_AUDIO_DEVICE_I2S_SLAVE;
    open_param->stream_out_param.afe.memory = config->scenario_config.tdm_config.out_memory;//HAL_AUDIO_MEM1
    open_param->stream_out_param.afe.audio_interface = config->scenario_config.tdm_config.out_interface;//HAL_AUDIO_INTERFACE_1
    open_param->stream_out_param.afe.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
    open_param->stream_out_param.afe.stream_out_sampling_rate = hal_audio_sampling_rate_enum_to_value(config->scenario_config.tdm_config.sampling_rate);
    open_param->stream_out_param.afe.sampling_rate = hal_audio_sampling_rate_enum_to_value(config->scenario_config.tdm_config.sampling_rate);
    open_param->stream_out_param.afe.irq_period = 0;
    open_param->stream_out_param.afe.frame_size = config->scenario_config.tdm_config.frame_size;
    open_param->stream_out_param.afe.frame_number = 4;
    open_param->stream_out_param.afe.hw_gain = true;
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    //I2S Slave parameters
    open_param->stream_out_param.afe.i2s_format = HAL_AUDIO_I2S_I2S;
    open_param->stream_out_param.afe.i2s_word_length = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;
    open_param->stream_out_param.afe.i2S_Slave_TDM = config->scenario_config.tdm_config.out_channel;
#endif

    open_param->stream_in_param.afe.ul_adc_mode[0] = HAL_AUDIO_ANALOG_INPUT_ACC10K;
    open_param->stream_in_param.afe.performance = AFE_PEROFRMANCE_HIGH_MODE;
#ifdef ENABLE_HWSRC_CLKSKEW
    open_param->stream_out_param.afe.clkskew_mode = CLK_SKEW_V1;
#endif
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    open_param->stream_out_param.afe.performance = AFE_PEROFRMANCE_NORMAL_MODE;
#endif
}

void audio_transmitter_tdm_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param)
{
    start_param->param.stream_in = STREAM_IN_AFE;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_VIRTUAL_OUT_ENABLE
    start_param->param.stream_out = STREAM_OUT_VIRTUAL;
#else
    start_param->param.stream_out = STREAM_OUT_AFE;
#endif
    memset((void *)&start_param->stream_in_param, 0, sizeof(mcu2dsp_start_stream_in_param_t));
    memset((void *)&start_param->stream_out_param, 0, sizeof(mcu2dsp_start_stream_out_param_t));
}
#endif
