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
#if defined(AIR_RECORD_ADVANCED_ENABLE)

#include "scenario_advanced_record.h"

// static uint8_t get_sample_size(hal_audio_format_t format)
// {
//     if(format <= HAL_AUDIO_PCM_FORMAT_U8){
//         return 1;
//     } else if((format >= HAL_AUDIO_PCM_FORMAT_S16_LE)&&(format <= HAL_AUDIO_PCM_FORMAT_U16_BE)){
//         return 2;
//     } else if((format >= HAL_AUDIO_PCM_FORMAT_S24_LE)&&(format <= HAL_AUDIO_PCM_FORMAT_U24_BE)){
//         return 3;
//     } else if((format >= HAL_AUDIO_PCM_FORMAT_S32_LE)&&(format <= HAL_AUDIO_PCM_FORMAT_U32_BE)){
//         return 4;
//     } else {
//         configASSERT(0);
//         return 0;
//     }
// }

void audio_transmitter_advanced_record_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    switch (config->scenario_sub_id) {
        case AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC:
        {
            if(ami_get_stream_in_channel_num(AUDIO_SCENARIO_TYPE_BLE_UL) > 2){
                configASSERT(0 && "[wireless_mic] HWIO config input channel > 2 ! error");
            }

            //hal_dvfs_lock_control(HAL_DVFS_HIGH_SPEED_208M, HAL_DVFS_LOCK);
#ifdef AIR_LD_NR_ENABLE
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            ami_mic_ld_nr_set_parameter(hal_audio_query_voice_mic_type());
#else
            ami_mic_ld_nr_set_parameter(VOICE_MIC_TYPE_FIXED);
#endif
#endif

            n_mic_config_t *n_mic_config;
            n_mic_config = &(config->scenario_config.advanced_record_config.n_mic_config);
            open_param->param.stream_in  = STREAM_IN_AFE;
            open_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;

            if(n_mic_config->input_codec_type == AUDIO_DSP_CODEC_TYPE_PCM) {
                hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &(open_param->stream_in_param));
                open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM_SUB;
                open_param->stream_in_param.afe.format = n_mic_config->input_codec_param.pcm.format;
                open_param->stream_in_param.afe.sampling_rate = n_mic_config->input_codec_param.pcm.sample_rate;
                open_param->stream_in_param.afe.irq_period = n_mic_config->input_codec_param.pcm.frame_interval/1000;
                open_param->stream_in_param.afe.frame_size = n_mic_config->input_codec_param.pcm.sample_rate * (n_mic_config->input_codec_param.pcm.frame_interval/1000)/1000;
                open_param->stream_in_param.afe.frame_number = 6;
                open_param->stream_in_param.afe.hw_gain = false; //only use sw gain
            } else {
                TRANSMITTER_LOG_E("AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC input can`t use other type %d than PCM", 1, n_mic_config->input_codec_type);
                AUDIO_ASSERT(0);
            }

            if(n_mic_config->input_codec_type == AUDIO_DSP_CODEC_TYPE_PCM) {
                open_param->stream_out_param.data_ul.scenario_type = config->scenario_type;
                open_param->stream_out_param.data_ul.scenario_sub_id = config->scenario_sub_id;
                open_param->stream_out_param.data_ul.p_share_info = hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_ADVANCED_RECORD_N_MIC);
                open_param->stream_out_param.data_ul.data_notification_frequency = 4;
                open_param->stream_out_param.data_ul.max_payload_size = RECORD_ADVANCED_FRAME_SIZE;//open_param->stream_in_param.afe.frame_size * n_mic_config->output_codec_param.pcm.channel_mode * get_sample_size(open_param->stream_in_param.afe.format); //MAX format
                open_param->stream_out_param.data_ul.scenario_param.advanced_record_param.codec_type = n_mic_config->output_codec_type;
                open_param->stream_out_param.data_ul.scenario_param.advanced_record_param.codec_param.pcm.channel_mode = n_mic_config->output_codec_param.pcm.channel_mode;
                open_param->stream_out_param.data_ul.scenario_param.advanced_record_param.codec_param.pcm.sample_rate = n_mic_config->output_codec_param.pcm.sample_rate;
                open_param->stream_out_param.data_ul.scenario_param.advanced_record_param.codec_param.pcm.frame_interval = n_mic_config->output_codec_param.pcm.frame_interval;
                open_param->stream_out_param.data_ul.scenario_param.advanced_record_param.codec_param.pcm.format = n_mic_config->output_codec_param.pcm.format;
                audio_transmitter_reset_share_info_by_block(open_param->stream_out_param.data_ul.p_share_info,
                                                            open_param->stream_out_param.data_ul.p_share_info->start_addr,
                                                            open_param->stream_out_param.data_ul.p_share_info->length,
                                                            open_param->stream_out_param.data_ul.max_payload_size);
            }
            else {
                TRANSMITTER_LOG_E("AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC output not support type %d", 1, n_mic_config->output_codec_type);
                AUDIO_ASSERT(0);
            }
            // TRANSMITTER_LOG_I("interface0 =%d, device0 =%d, interface1 =%d, device1 =%d, memory=%d, format=%d, sampling_rate=%d, frame_size=%d, max_payload_size=%d",9,
            // open_param->stream_in_param.afe.audio_interface,
            // open_param->stream_in_param.afe.audio_device,
            // open_param->stream_in_param.afe.audio_interface1,
            // open_param->stream_in_param.afe.audio_device1,
            // open_param->stream_in_param.afe.memory,
            // open_param->stream_in_param.afe.format,
            // open_param->stream_in_param.afe.sampling_rate,
            // open_param->stream_in_param.afe.frame_size,
            // open_param->stream_out_param.data_ul.max_payload_size);
#ifdef __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__
            {
                bt_sink_srv_audio_setting_vol_info_t vol_info;

                vol_info.type = VOL_HFP;
                vol_info.vol_info.hfp_vol_info.codec = BT_HFP_CODEC_TYPE_CVSD;
                vol_info.vol_info.hfp_vol_info.dev_in = open_param->stream_in_param.afe.audio_device;
                vol_info.vol_info.hfp_vol_info.dev_out = HAL_AUDIO_DEVICE_HEADSET;
                vol_info.vol_info.hfp_vol_info.lev_in = 0;
                vol_info.vol_info.hfp_vol_info.lev_out = 0;
                extern void bt_sink_srv_am_set_volume(bt_sink_srv_am_stream_type_t in_out, bt_sink_srv_audio_setting_vol_info_t *vol_info);
                bt_sink_srv_am_set_volume(STREAM_IN, &vol_info);
            }
#else
            aud_set_volume_level(STREAM_IN, AUD_VOL_AUDIO, stream_in->audio_device, (bt_sink_srv_am_volume_level_t)stream_in->audio_volume);
#endif /* __BT_SINK_SRV_AUDIO_SETTING_SUPPORT__ */
            break;
        }
        default:
            return;
    }
}
void audio_transmitter_advanced_record_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param)
{
    switch (config->scenario_sub_id) {
        case AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC:
            start_param->param.stream_in = STREAM_IN_AFE;
            start_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;
            start_param->stream_in_param.afe.mce_flag = true;

            break;
        default:
            return;
    }
}


#endif