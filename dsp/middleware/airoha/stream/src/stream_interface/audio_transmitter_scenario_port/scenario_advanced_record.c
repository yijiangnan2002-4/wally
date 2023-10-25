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
#include "scenario_advanced_record.h"

#define ADVANCED_RECORD_DEBUG_LOG 0

#ifdef AIR_SOFTWARE_DRC_ENABLE
#include "compander_interface_sw.h"
#endif

#ifdef AIR_LD_NR_ENABLE
void *p_advanced_record_ld_nr_key;
#endif

#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
#include "volume_estimator_interface.h"
#include "stream_nvkey_struct.h"
#include "preloader_pisplit.h"
#include "audio_nvdm_common.h"
void *p_advanced_record_nvkey_buf = NULL;
volume_estimator_port_t *p_advanced_record_meter_port;
#endif

ATTR_TEXT_IN_IRAM static void ShareBufferCopy_D_32bit_to_I_24bit_2ch(uint32_t* src_buf1, uint32_t* src_buf2, uint8_t* dest_buf1, uint32_t samples)
{
    uint32_t data1;
    uint32_t data2;
    uint32_t i;

    for (i = 0; i < samples; i++) {
        data1 = src_buf1[i];
        data2 = src_buf2[i];
        *(dest_buf1+i*6+0) = (uint8_t)((data1>> 8)&0xff);
        *(dest_buf1+i*6+1) = (uint8_t)((data1>>16)&0xff);
        *(dest_buf1+i*6+2) = (uint8_t)((data1>>24)&0xff);
        *(dest_buf1+i*6+3) = (uint8_t)((data2>> 8)&0xff);
        *(dest_buf1+i*6+4) = (uint8_t)((data2>>16)&0xff);
        *(dest_buf1+i*6+5) = (uint8_t)((data2>>24)&0xff);
    }
}
ATTR_TEXT_IN_IRAM static void ShareBufferCopy_D_16bit_to_I_24bit_1ch(uint16_t* src_buf1, uint8_t* dest_buf1, uint32_t samples)
{
    uint32_t data32;
    uint32_t i, j;

    j = 0;
    for (i = 0; i < samples; i++) {
        if ((i % 4) == 0) {
            data32 = src_buf1[i] << 8; // 0x00XXXX00
            *(uint32_t *)(dest_buf1 + j * 12) = data32;
        } else if ((i % 4) == 1) {
            data32 = src_buf1[i]; //0x0000XXXX
            *(uint32_t *)(dest_buf1 + j * 12 + 4) = data32;
        } else if ((i % 4) == 2) {
            data32 = (src_buf1[i] & 0x00ff) << 24; // 0xXX000000
            *(uint32_t *)(dest_buf1 + j * 12 + 4) |= data32;
            data32 = (src_buf1[i] & 0xff00) >> 8;
            *(uint32_t *)(dest_buf1 + j * 12 + 8) = data32;
        } else {
            data32 = src_buf1[i] << 16; // 0xXXXX0000
            *(uint32_t *)(dest_buf1 + j * 12 + 8) |= data32;
            j++;
        }
    }
}

ATTR_TEXT_IN_IRAM static void ShareBufferCopy_D_32bit_to_D_24bit_1ch(uint32_t* src_buf, uint8_t* dest_buf1, uint32_t samples)
{
    uint32_t data;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data = src_buf[i];
        *(dest_buf1+i*3+0) = (uint8_t)((data>> 8)&0xff);
        *(dest_buf1+i*3+1) = (uint8_t)((data>>16)&0xff);
        *(dest_buf1+i*3+2) = (uint8_t)((data>>24)&0xff);
    }
}

ATTR_TEXT_IN_IRAM static void ShareBufferCopy_D_16bit_to_I_24bit_2ch(uint16_t* src_buf1, uint16_t* src_buf2, uint8_t* dest_buf1, uint32_t samples)
{
    uint32_t data32;
    uint16_t data16;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        if ((i%2) == 0)
        {
            data32 = (src_buf1[i]<<8); // 0x00XXXX00
            data16 = src_buf2[i]; //0xXXXX
            *(uint32_t *)(dest_buf1 + i*6) = data32;
            *(uint16_t *)(dest_buf1 + i*6 + 4) = data16;
        }
        else
        {
            data16 = (src_buf1[i]&0x00ff)<<8; //0xXX00
            data32 = (src_buf2[i]<<16) | ((src_buf1[i]&0xff00)>>8); // 0xXXXX00XX
            *(uint16_t *)(dest_buf1 + i*6) = data16;
            *(uint32_t *)(dest_buf1 + i*6 + 2) = data32;
        }
    }
}

void advanced_record_init(SOURCE source, SINK sink, mcu2dsp_open_param_p open_param)
{
    UNUSED(source);
    if (sink->param.data_ul.scenario_sub_id == AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC) {
        stream_resolution_t resolution;
        extern CONNECTION_IF advanced_record_n_mic_if;
        if(open_param->stream_in_param.afe.format <= HAL_AUDIO_PCM_FORMAT_U16_BE) {
            stream_feature_configure_resolution((stream_feature_list_ptr_t)advanced_record_n_mic_if.pfeature_table, RESOLUTION_16BIT, CONFIG_DECODER);
            resolution = RESOLUTION_16BIT;
        }
        else{
            stream_feature_configure_resolution((stream_feature_list_ptr_t)advanced_record_n_mic_if.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
            resolution = RESOLUTION_32BIT;
        }
        uint8_t channel_num = MIN(sink->param.data_ul.scenario_param.advanced_record_param.codec_param.pcm.channel_mode,source->param.audio.channel_num);
#ifdef AIR_LD_NR_ENABLE
        ld_nr_port_t *ld_nr_port;
        ld_nr_config_t ld_nr_config;
        ld_nr_port = stream_function_ld_nr_get_port(source);
        ld_nr_config.channel_num = channel_num;
        ld_nr_config.frame_size  = open_param->stream_in_param.afe.irq_period * open_param->stream_in_param.afe.sampling_rate / 1000 * ((resolution == RESOLUTION_16BIT)? 2:4);
        ld_nr_config.resolution  = resolution;
        ld_nr_config.sample_rate = source->param.audio.rate;

        if (p_advanced_record_ld_nr_key == NULL) {
            //PSAP_LOG_E(g_PSAP_msg_id_string_12, "hearing-aid LD_NR NVKEY NULL", 0);
            configASSERT(0);
        }

        ld_nr_config.nvkey_para = p_advanced_record_ld_nr_key;
        ld_nr_config.background_process_enable = true;
        ld_nr_config.background_process_fr_num = 2;
        stream_function_ld_nr_init(ld_nr_port, &ld_nr_config);
        DSP_MW_LOG_I("[advanced_record_n_mic][LD_NR]p_wireless_mic_ld_nr_key 0x%x channel_num=%d, resolution:%d", 3, p_advanced_record_ld_nr_key, channel_num, resolution);
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_SOFTWARE_DRC_ENABLE
        /* sw drc init */
        sw_compander_config_t drc_config;
        drc_config.mode = SW_COMPANDER_AUDIO_MODE;
        drc_config.channel_num = channel_num;
        drc_config.sample_rate = source->param.audio.rate;
        drc_config.frame_base = 8;
        drc_config.recovery_gain = 4; /* 0dB */
        drc_config.vol_default_gain = 0x08000000; /* 0dB */
        drc_config.default_nvkey_mem = NULL;
        drc_config.default_nvkey_id = NVKEY_DSP_PARA_ADVANCED_RECORD_AU_CPD;
        sw_compander_port_t *drc_port = stream_function_sw_compander_get_port(source);
        stream_function_sw_compander_init(drc_port, &drc_config);
        DSP_MW_LOG_I("[advanced_record_n_mic]sw drc 0x%x info, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x\r\n", 11,
                    drc_port,
                    drc_config.mode,
                    drc_config.channel_num,
                    drc_config.sample_rate,
                    drc_config.frame_base,
                    drc_config.recovery_gain,
                    drc_config.vol_default_gain,
                    drc_config.default_nvkey_id);
#endif

#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
        /* init volume estimator port */
        p_advanced_record_nvkey_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, sizeof(audio_spectrum_meter_nvkey_t));
        if (p_advanced_record_nvkey_buf == NULL) {
            AUDIO_ASSERT(0);
        }
        if (nvkey_read_full_key(NVKEY_DSP_PARA_SILENCE_DETECTION2, p_advanced_record_nvkey_buf, sizeof(audio_spectrum_meter_nvkey_t)) != NVDM_STATUS_NAT_OK) {
            AUDIO_ASSERT(0);
        }
        volume_estimator_config_t config;
        p_advanced_record_meter_port = volume_estimator_get_port(source);
        if (p_advanced_record_meter_port == NULL) {
            AUDIO_ASSERT(0 && "[ULL Audio V2][DL] Audio Spectrum memter port is null.");
        }
        config.resolution = RESOLUTION_32BIT;
        config.frame_size = /*open_param->stream_in_param.afe.irq_period*/2.5 * open_param->stream_in_param.afe.sampling_rate / 1000 * ((resolution == RESOLUTION_16BIT)? 2:4); /* 5ms*48K*mono*32bit */
        config.channel_num = 1;
        config.mode = VOLUME_ESTIMATOR_CHAT_INSTANT_MODE;
        config.sample_rate = source->param.audio.rate;
        config.nvkey_para = (void *)&(((audio_spectrum_meter_nvkey_t *)p_advanced_record_nvkey_buf)->chat_vol_nvkey);
        config.internal_buffer = NULL;
        config.internal_buffer_size = 0;
        volume_estimator_init(p_advanced_record_meter_port, &config);
        DSP_MW_LOG_I("[advanced_record_n_mic]volume estimator 0x%x info, %d, %d, %d,internal_buffer 0x%x, 0x%x\r\n", 6,
                    p_advanced_record_meter_port,
                    config.frame_size,
                    config.channel_num,
                    config.sample_rate,
                    config.internal_buffer,
                    config.internal_buffer_size);
#endif
    }
}

void advanced_record_deinit(SOURCE source, SINK sink)
{
    UNUSED(source);
    if (sink->param.data_ul.scenario_sub_id == AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC) {
#ifdef AIR_LD_NR_ENABLE
        ld_nr_port_t *ld_nr_port;
        ld_nr_port = stream_function_ld_nr_get_port(source);
        stream_function_ld_nr_deinit(ld_nr_port);
        preloader_pisplit_free_memory(p_advanced_record_ld_nr_key);
        p_advanced_record_ld_nr_key = NULL;
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_SOFTWARE_DRC_ENABLE
        sw_compander_port_t *drc_port;
        drc_port = stream_function_sw_compander_get_port(source);
        stream_function_sw_compander_deinit(drc_port);
#endif
#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
        volume_estimator_deinit(p_advanced_record_meter_port);
        if (p_advanced_record_nvkey_buf) {
            preloader_pisplit_free_memory(p_advanced_record_nvkey_buf);
            p_advanced_record_nvkey_buf = NULL;
        }
#endif
    }
}

uint8_t temp_buf[1500];
uint32_t advanced_record_n_mic_sink_copy_payload(SINK sink, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    audio_codec_pcm_t *codec_pcm;
    uint32_t channel_mode, actual_Sample, *write_data_size, *write_data_block;
    hal_audio_format_t source_format, sink_format;
    uint8_t *dst_buf_s;

    UNUSED(length);

    sink_format = sink->param.data_ul.scenario_param.advanced_record_param.codec_param.pcm.format;
    source_format = sink->transform->source->param.audio.format;
    channel_mode = sink->param.data_ul.scenario_param.advanced_record_param.codec_param.pcm.channel_mode;
    write_data_size = &(sink->param.data_ul.scenario_param.advanced_record_param.write_data_size);
    write_data_block = &(sink->param.data_ul.scenario_param.advanced_record_param.write_data_block);

    dst_buf_s = dst_buf;
    dst_buf = temp_buf;

    codec_pcm = &(sink->param.data_ul.scenario_param.advanced_record_param.codec_param.pcm);
    actual_Sample = (codec_pcm->sample_rate * codec_pcm->frame_interval) / (1000 * 1000);
    if (channel_mode == 1) {
        if(source_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
            if (sink_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                ShareBufferCopy_D_16bit_to_I_24bit_1ch((uint16_t *)src_buf, (uint8_t *)dst_buf, actual_Sample);
                length = actual_Sample * 3;
            } else if (sink_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
                memcpy(dst_buf, src_buf, actual_Sample * sizeof(uint16_t));
            }
        } else if(source_format >= HAL_AUDIO_PCM_FORMAT_S24_LE) {
            if (sink_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                ShareBufferCopy_D_32bit_to_D_24bit_1ch((uint32_t *)src_buf, (uint8_t *)dst_buf, actual_Sample);
                length = actual_Sample * 3;
            } else if (sink_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
                AUDIO_ASSERT(0); //not support yet
            }
        }
    } else if (channel_mode == 2) {
        DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(sink->transform->source, sink);
        VOID *src_buf1, *src_buf2;
        src_buf1 = stream->callback.EntryPara.out_ptr[0];
        if (sink->transform->source->param.audio.channel_num == 1) {
            src_buf2 = stream->callback.EntryPara.out_ptr[0];
        }
        else if(sink->transform->source->param.audio.channel_num == 2) {
            if(stream->callback.EntryPara.out_ptr[1] == NULL)
                src_buf2 = stream->callback.EntryPara.out_ptr[0];
            else
                src_buf2 = stream->callback.EntryPara.out_ptr[1];
        }
        if(source_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
            if (sink_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                ShareBufferCopy_D_16bit_to_I_24bit_2ch((uint16_t *)src_buf1, (uint16_t *)src_buf2, (uint8_t *)dst_buf, actual_Sample);
                length = actual_Sample * 3 * 2;
            } else if (sink_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
                DSP_D2I_BufferCopy_16bit((uint16_t *)dst_buf, (uint16_t *)src_buf1, (uint16_t *)src_buf2, actual_Sample);
                length = length * 2;
            }
        } else if(source_format >= HAL_AUDIO_PCM_FORMAT_S24_LE) {
            if (sink_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                ShareBufferCopy_D_32bit_to_I_24bit_2ch((uint32_t *)src_buf1, (uint32_t *)src_buf2, (uint8_t *)dst_buf, length / sizeof(uint32_t));
                length = actual_Sample * 3 * 2;
            } else if (sink_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
                AUDIO_ASSERT(0); //not support yet
            }
        }
    } else {
        AUDIO_ASSERT(0); //not support yet
    }

#ifdef AIR_AUDIO_DUMP_ENABLE
    if(source_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
        LOG_AUDIO_DUMP(src_buf, actual_Sample * sizeof(uint16_t), AUDIO_ADVANCED_RECORD_OUTPUT_CH_0);
    } else if(source_format >= HAL_AUDIO_PCM_FORMAT_S24_LE) {
        LOG_AUDIO_DUMP(src_buf, actual_Sample * sizeof(uint32_t), AUDIO_ADVANCED_RECORD_OUTPUT_CH_0);
    }
    //LOG_AUDIO_DUMP(dst_buf, length, WIRED_AUDIO_USB_OUT_O_2);
#endif

    if((*write_data_size + length) >= RECORD_ADVANCED_FRAME_SIZE) {
        (*write_data_block)++;
        memcpy(dst_buf_s + sizeof(audio_transmitter_frame_header_t) + *write_data_size, dst_buf, RECORD_ADVANCED_FRAME_SIZE - *write_data_size);
        #if ADVANCED_RECORD_DEBUG_LOG
        DSP_MW_LOG_E("[audio transmitter]: ADVANCED RECORD dst_buf_s=0x%x, src=0x%x size = %d", 3,
        dst_buf_s + sizeof(audio_transmitter_frame_header_t) + *write_data_size,
        dst_buf,
        RECORD_ADVANCED_FRAME_SIZE - *write_data_size);
        #endif

        if (*write_data_block == (sink->param.data_ul.share_info_base_addr->length / RECORD_ADVANCED_FRAME_SIZE))
        {
            memcpy((uint32_t*)(hal_memview_cm4_to_dsp0(sink->param.data_ul.share_info_base_addr->start_addr) + sizeof(audio_transmitter_frame_header_t)), dst_buf + RECORD_ADVANCED_FRAME_SIZE - *write_data_size, ((*write_data_size) + length - RECORD_ADVANCED_FRAME_SIZE));
            #if ADVANCED_RECORD_DEBUG_LOG
            DSP_MW_LOG_E("[audio transmitter]: ADVANCED RECORD dst_buf_s=0x%x, src=0x%x size = %d", 3,
            hal_memview_cm4_to_dsp0(sink->param.data_ul.share_info_base_addr->start_addr + sizeof(audio_transmitter_frame_header_t)),
            dst_buf + RECORD_ADVANCED_FRAME_SIZE - *write_data_size,
            (*write_data_size) + length - RECORD_ADVANCED_FRAME_SIZE);
            #endif
            *write_data_block = 0;
        } else {
            memcpy(dst_buf_s + RECORD_ADVANCED_FRAME_SIZE + sizeof(audio_transmitter_frame_header_t)*2, dst_buf + RECORD_ADVANCED_FRAME_SIZE - *write_data_size, ((*write_data_size) + length - RECORD_ADVANCED_FRAME_SIZE));
            #if ADVANCED_RECORD_DEBUG_LOG
            DSP_MW_LOG_E("[audio transmitter]: ADVANCED RECORD dst_buf_s=0x%x, src=0x%x size = %d", 3,
            dst_buf_s + RECORD_ADVANCED_FRAME_SIZE + sizeof(audio_transmitter_frame_header_t)*2,
            dst_buf + RECORD_ADVANCED_FRAME_SIZE - *write_data_size,
            (*write_data_size) + length - RECORD_ADVANCED_FRAME_SIZE);
            #endif
        }

        *write_data_size = *write_data_size + length- RECORD_ADVANCED_FRAME_SIZE;

        audio_transmitter_frame_header_t *data_ul_header = (audio_transmitter_frame_header_t *)(dst_buf_s);
        data_ul_header->seq_num = sink->param.data_ul.seq_num++;
        data_ul_header->payload_len = RECORD_ADVANCED_FRAME_SIZE;//sink->param.data_ul.frame_size;
        sink->param.data_ul.is_assembling = false;
        sink->param.data_ul.scenario_param.advanced_record_param.write_data_flag = true;

        return RECORD_ADVANCED_FRAME_SIZE;
    } else {
        memcpy(dst_buf_s + sizeof(audio_transmitter_frame_header_t) + *write_data_size, dst_buf, length);
        #if ADVANCED_RECORD_DEBUG_LOG
        DSP_MW_LOG_E("[audio transmitter]: ADVANCED RECORD dst_buf_s=0x%x, src=0x%x size = %d", 3,
        dst_buf_s + sizeof(audio_transmitter_frame_header_t) + *write_data_size,
        dst_buf,
        length);
        #endif

        *write_data_size = *write_data_size + length;
        return 0;
    }
}

ATTR_TEXT_IN_IRAM bool advanced_record_n_mic_sink_get_avail_size(SINK sink, uint32_t *avail_size)
{
    UNUSED(avail_size);
    if (sink->streamBuffer.ShareBufferInfo.bBufferIsFull == true) {
        DSP_MW_LOG_E("[audio transmitter]: ADVANCED RECORD query share buffer full", 0);
        //AUDIO_ASSERT(0);
    }
    return false;
}
bool advanced_record_n_mic_sink_query_notification(SINK sink, bool *notification_flag)
{
    UNUSED(sink);
    UNUSED(notification_flag);
    if(sink->param.data_ul.scenario_param.advanced_record_param.write_data_flag){
        *notification_flag = true;
        sink->param.data_ul.scenario_param.advanced_record_param.write_data_flag = false;
    } else {
        *notification_flag = false;
    }
    return true;
}
ATTR_TEXT_IN_IRAM void advanced_record_n_mic_sink_query_write_offset(SINK sink, uint32_t *write_offset)
{
    UNUSED(write_offset);
    n9_dsp_share_info_ptr ShareBufferInfo = &sink->streamBuffer.ShareBufferInfo;
    uint32_t total_buffer_size = ShareBufferInfo->sub_info.block_info.block_size * ShareBufferInfo->sub_info.block_info.block_num;
    if(sink->param.data_ul.scenario_param.advanced_record_param.write_data_flag){
        ShareBufferInfo->write_offset = (ShareBufferInfo->write_offset + ShareBufferInfo->sub_info.block_info.block_size) % total_buffer_size;
    }
}