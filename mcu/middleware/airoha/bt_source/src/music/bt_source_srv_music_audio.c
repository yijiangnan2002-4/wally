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
#include "bt_source_srv_utils.h"
#include "bt_source_srv_music_psd_manager.h"
#include "bt_source_srv_utils.h"
#include "bt_source_srv_music_audio.h"
#include "bt_avm.h"
#include "scenario_dongle_common.h"
#include "scenario_bt_audio.h"


#define BT_SOURCE_SRV_MUSIC_AUDIO_LINK_MAX  1
#define BT_SOURCE_SRV_MUSIC_AUDIO_DSP_FORMAT_MAX                    5
#define BT_SOURCE_SRV_MUSIC_AUDIO_FRAME_INTERVAL                    1000

static void bt_source_srv_music_set_audio_notify(n9_dsp_share_info_t *p_dsp_info, uint32_t inteval, uint32_t frame_size, uint8_t codec_type);

const static uint32_t g_audio_dsp_format_mapping[BT_SOURCE_SRV_MUSIC_AUDIO_DSP_FORMAT_MAX] = {
    HAL_AUDIO_PCM_FORMAT_DUMMY,//AFE_PCM_FORMAT_DUMMY,
    HAL_AUDIO_PCM_FORMAT_U8,//AFE_PCM_FORMAT_U8,
    HAL_AUDIO_PCM_FORMAT_S16_LE,//AFE_PCM_FORMAT_S16_LE,
    HAL_AUDIO_PCM_FORMAT_S24_LE,//AFE_PCM_FORMAT_S24_LE,
    HAL_AUDIO_PCM_FORMAT_S32_LE//AFE_PCM_FORMAT_S32_LE
};

const static uint32_t g_sbc_sample[] = {
    16000,
    32000,
    44100,
    48000
};

static audio_dsp_codec_type_t bt_source_srv_music_audio_dsp_codec_mapping(bt_a2dp_codec_capability_t *codec)
{
    audio_dsp_codec_type_t dsp_codec_type = AUDIO_DSP_CODEC_TYPE_DUMMY;
    switch (codec->type) {
        case BT_A2DP_CODEC_SBC: {
            dsp_codec_type = AUDIO_DSP_CODEC_TYPE_SBC;
            break;
        }

        case BT_A2DP_CODEC_VENDOR: {
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
            if (codec->codec.vendor.codec_id == BT_A2DP_CODEC_VENDOR_2_CODEC_ID) {
                dsp_codec_type = AUDIO_DSP_CODEC_TYPE_LHDC;
            }
#endif
        break;
        }
        default:
            break;
    }
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]audio_dsp_codec_mapping, codec_type: 0x%x, dsp_codec_type:0x%x", 2, codec->type, dsp_codec_type);
    return dsp_codec_type;
}

static void bt_source_srv_music_config_init_pcm(audio_codec_pcm_t *pcm, bt_source_srv_common_audio_port_context_t *port_context)
{
    pcm->sample_rate= port_context->sample_rate;
    pcm->channel_mode = port_context->sample_channel;
    pcm->format = g_audio_dsp_format_mapping[port_context->sample_size];
    pcm->frame_interval = BT_SOURCE_SRV_MUSIC_AUDIO_FRAME_INTERVAL;
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]config_init, rate:%x,channel:%x, format:%x,", 3, pcm->sample_rate,pcm->channel_mode, pcm->format);
}

static void bt_source_srv_music_set_audio_notify(n9_dsp_share_info_t *p_dsp_info, uint32_t inteval, uint32_t frame_size, uint8_t codec_type)
{
    bt_avm_ext_share_buffer_info_t buffer_info = {0};
    buffer_info.music_dl_address = (uint32_t)p_dsp_info;
    buffer_info.reserve = (((uint8_t)inteval) << 24);/*bit23~bit31*/
    buffer_info.reserve |= (((uint8_t)frame_size) << 16); /*bit15~bit23*/
    buffer_info.reserve |= 0x01;/* bit0: notify contollter dongle role */
    buffer_info.reserve |= ((codec_type) << 4);/* bit3~Bbit7 */
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]set_audio_notify, interval:0x%x,reserve:0x%x",2, inteval,buffer_info.reserve);
    bt_avm_set_ext_share_buffer(&buffer_info);
}

bt_status_t bt_source_srv_set_music_enable(bt_bd_addr_t *dev_addr, bool enable)
{
    bt_status_t ret = BT_STATUS_FAIL;

    bt_gap_connection_handle_t gap_handle = 0;


    gap_handle = bt_cm_get_gap_handle(*dev_addr);
    if (gap_handle) {
        ret = bt_avm_set_ull_gaming_mode_enable(gap_handle, BT_AVM_ROLE_NORMAL, enable, 0 , 0);
    }
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]set_music_enable: enable,gap_handle: 0x%x,ret:0x%x, enable:0x%x", 3,  gap_handle, ret, enable);
    return ret;
}

static audio_sbc_encoder_block_number_t bt_source_srv_music_get_sbc_block_length(uint8_t block_len)
{
    audio_sbc_encoder_block_number_t block_num =  SBC_ENCODER_BLOCK_NUMBER_4;

    if (block_len & 0x1) { /**< The block length (b4~b7), b4: 16, b5: 12, b6: 8, b7: 4. */
        block_num = SBC_ENCODER_BLOCK_NUMBER_16;
    } else if (block_len & 0x2) {
        block_num =  SBC_ENCODER_BLOCK_NUMBER_12;
    } else if (block_len & 0x4) {
        block_num =  SBC_ENCODER_BLOCK_NUMBER_8;
    }
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]get_sbc_block_length: block_len:0x%x, block_num: 0x%x", 2,  block_len, block_num);
    return block_num;
}

static audio_sbc_encoder_subband_number_t bt_source_srv_music_get_sbc_sub_num(uint8_t sub_mum)
{
    /**< The subbands (b2, b3), b2: 8 subbands, b3: 4 subbands. */
    audio_sbc_encoder_subband_number_t subbunds_num = SBC_ENCODER_SUBBAND_NUMBER_4;

    if (sub_mum & 0x01) {
        subbunds_num = SBC_ENCODER_SUBBAND_NUMBER_8;//8subunds
    }
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]get_sbc_sub_num: subbunds_num:0x%x, sub_mum: 0x%x", 2,subbunds_num, sub_mum);
    return subbunds_num;
}


static audio_sbc_encoder_allocation_method_t bt_source_srv_music_get_sbc_alloc_method(uint8_t alloc_method)
{
    /**< The allocation method (b0, b1), b0: Loudness, b1:SNR. */

    audio_sbc_encoder_allocation_method_t method = SBC_ENCODER_SNR;

    if (alloc_method & 0x01) {
        method = SBC_ENCODER_LOUDNESS;//SNR
    }
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]get_sbc_alloc_method: method:0x%x, alloc_method: 0x%x", 2, method, alloc_method);
    return method;
}

static audio_sbc_encoder_sampling_rate_t bt_source_srv_music_get_sbc_sample_rate(uint8_t sample_rate)
{
    audio_sbc_encoder_sampling_rate_t sbc_rate = SBC_ENCODER_SAMPLING_RATE_16000HZ;/**< The sample frequency (b4~b7), b4: 48kHz, b5: 44.1kHz, b6: 32kHz, b7: 16kHz. */

    if (sample_rate & 0x1){
        sbc_rate =  SBC_ENCODER_SAMPLING_RATE_48000HZ;//48k
    } else if (sample_rate & 0x2) {
        sbc_rate =  SBC_ENCODER_SAMPLING_RATE_44100HZ; //44.1kHz
    } else if (sample_rate & 0x4) {
        sbc_rate = SBC_ENCODER_SAMPLING_RATE_32000HZ;
    }
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]get_sbc_sample_rate: sbc_rate:0x%x, sample_rate: 0x%x", 2, sbc_rate, sample_rate);
    return sbc_rate;
}

static audio_sbc_encoder_channel_mode_t bt_source_srv_music_get_sbc_channel_mode(uint8_t channel_mode)
{
    /**< The channel mode (b0~b3), b0: Joint stereo, b1: Stereo, b2: Dual channel,b3: Mono. */
    audio_sbc_encoder_channel_mode_t chann_mode = SBC_ENCODER_MONO;

    if (channel_mode & 0x01) {
        chann_mode = SBC_ENCODER_JOINT_STEREO;//join stereo
    } else if (channel_mode & 0x2) {
        chann_mode = SBC_ENCODER_STEREO;//Stereo
    } else if (channel_mode & 0x04) {
        chann_mode = SBC_ENCODER_DUAL_CHANNEL;//dual channel
    }
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]get_sbc_channel_mode: chann_mode:0x%x, channel_mode: 0x%x", 2, chann_mode, channel_mode);
    return chann_mode;
}
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
static void bt_source_srv_music_fix_lhdc_codec(audio_codec_lhdc_t *audio_lhdc, bt_a2dp_vendor_codec_t *a2dp_vendor)
{
    audio_codec_lhdc_t lhdc_codec = {0};
    audio_codec_vendor_config_t audio_vend_codec = {0};
    audio_vend_codec.vendor_id = a2dp_vendor->vendor_id;
    audio_vend_codec.codec_id = a2dp_vendor->codec_id;
    memcpy(&(audio_vend_codec.value), &(a2dp_vendor->value), 5);
    audio_dongle_vendor_parameter_parse(&audio_vend_codec, &lhdc_codec);
    memcpy(audio_lhdc, &lhdc_codec, sizeof(audio_codec_lhdc_t));
}
#endif

static void bt_source_srv_music_fix_sbc_codec(audio_codec_sbc_enc_t *audio_sbc, bt_a2dp_sbc_codec_t *a2dp_sbc)
{
    audio_sbc->max_bit_pool = a2dp_sbc->max_bitpool;
    audio_sbc->min_bit_pool = a2dp_sbc->min_bitpool;

    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]config_init, block_len:%x,sub_num:0x%x,alloc_method:0x%x,sample_rate:0x%x", 4, a2dp_sbc->block_len, a2dp_sbc->subbands, a2dp_sbc->alloc_method, a2dp_sbc->sample_freq);

    audio_sbc->block_length = bt_source_srv_music_get_sbc_block_length(a2dp_sbc->block_len);
    audio_sbc->subband_num = bt_source_srv_music_get_sbc_sub_num(a2dp_sbc->subbands);
    audio_sbc->alloc_method = bt_source_srv_music_get_sbc_alloc_method(a2dp_sbc->alloc_method);
    audio_sbc->sample_rate = bt_source_srv_music_get_sbc_sample_rate(a2dp_sbc->sample_freq);
    audio_sbc->channel_mode = bt_source_srv_music_get_sbc_channel_mode(a2dp_sbc->channel_mode);
}

bt_status_t bt_source_srv_music_audio_config_init(bt_source_srv_music_audio_config_t *config, bt_source_srv_music_device_t *context,  void *user_data, void *user_callback)
{
    uint32_t frame_num = 0, interval = 0, frame_interval = 0, frame_size = 0;
    uint32_t mtu = context->max_mtu;
    uint32_t sub_band = 0, blocks = 0, sample_rate = 0;
    audio_dsp_codec_type_t type = AUDIO_DSP_CODEC_TYPE_DUMMY;
    uint8_t notify_type = BT_SOURCE_SRV_CODEC_SBC;
    n9_dsp_share_info_t *p_dsp_info = NULL;

    bt_audio_dongle_dl_info_t *dl_config = &config->transmitter_config.scenario_config.bt_audio_dongle_config.dl_info;

    bt_a2dp_codec_capability_t *codec = &context->capabilty;
    bt_source_srv_common_audio_port_context_t port_context = {0};

    bt_status_t ret = bt_source_srv_common_audio_find_port_context(config->type, &port_context);
    if ((ret != BT_STATUS_SUCCESS) || (!bt_source_srv_common_audio_port_is_valid(port_context.port))) {
        ret = BT_STATUS_FAIL;
        return ret;
    }
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]config_init, audio_port:0x%x,rate:0x%x,size:0x%x,channel:0x%x", 4, port_context.port,port_context.sample_rate, port_context.sample_size, port_context.sample_channel);
    /* PC->audio:source config */
    dl_config->source.usb_in.codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
    if (context->detect_flag) {
        dl_config->sink.bt_out.without_bt_link_mode_enable = true;
    } else {
        dl_config->sink.bt_out.without_bt_link_mode_enable = false;
    }

    bt_source_srv_music_config_init_pcm(&dl_config->source.usb_in.codec_param.pcm, &port_context);

    /* audio->BT:sink config */
    dl_config->sink.bt_out.link_num = BT_SOURCE_SRV_MUSIC_AUDIO_LINK_MAX;

    uint32_t i = 0, pay_load_size = 0;
    for (i = 0; i < BT_SOURCE_SRV_MUSIC_AUDIO_LINK_MAX; i++) {
        dl_config->sink.bt_out.bt_info[i].enable = true;
        type= bt_source_srv_music_audio_dsp_codec_mapping(codec);
        dl_config->sink.bt_out.bt_info[i].codec_type = type;

        switch (type) {
            case AUDIO_DSP_CODEC_TYPE_SBC: {
                bt_source_srv_music_fix_sbc_codec(&(dl_config->sink.bt_out.bt_info[i].codec_param.sbc_enc), &(codec->codec.sbc));
                sub_band = (dl_config->sink.bt_out.bt_info[i].codec_param.sbc_enc.subband_num == 0 )? 4 : 8;
                blocks = (dl_config->sink.bt_out.bt_info[i].codec_param.sbc_enc.block_length + 1) * 4;
                sample_rate = g_sbc_sample[dl_config->sink.bt_out.bt_info[i].codec_param.sbc_enc.sample_rate];
                frame_size = audio_dongle_get_codec_frame_size(&type, &dl_config->sink.bt_out.bt_info[i].codec_param); //4B align
                frame_num = mtu/frame_size;
                frame_interval = BT_A2DP_SOURCE_FRAME_INTERVAL;
                break;
            }
#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
            case AUDIO_DSP_CODEC_TYPE_LHDC: {
                bt_source_srv_music_fix_lhdc_codec(&(dl_config->sink.bt_out.bt_info[i].codec_param.lhdc), &(codec->codec.vendor));
                sample_rate = dl_config->sink.bt_out.bt_info[i].codec_param.lhdc.sample_rate;
                frame_interval = dl_config->sink.bt_out.bt_info[i].codec_param.lhdc.frame_interval;
                dl_config->sink.bt_out.bt_info[i].codec_param.lhdc.mtu_size = mtu;
                frame_size = audio_dongle_get_codec_frame_size(&type, &dl_config->sink.bt_out.bt_info[i].codec_param); //4B align
                notify_type = BT_SOURCE_SRV_CODEC_LHDC;
                break;
            }
#endif
        default:
            break;
        }

        p_dsp_info = hal_audio_query_bt_audio_dl_share_info();
        if (NULL != p_dsp_info) {
            memset((void *)p_dsp_info->start_addr, 0, BT_A2DP_SOURCE_TRANSMITTER_SHARE_BUFFER_SIZE);
            p_dsp_info->read_offset = 0;
            p_dsp_info->write_offset = 0;
            pay_load_size = ((frame_size + sizeof(BT_AUDIO_HEADER) + 3) >> 2) * 4 ;
            LOG_MSGID_I(source_srv,"[A2DP_SOURCE]config_init, pay_load_size:0x%x",1, pay_load_size);
            p_dsp_info->sub_info.block_info.block_size = pay_load_size;
            p_dsp_info->sub_info.block_info.block_num  = BT_A2DP_SOURCE_TRANSMITTER_SHARE_BUFFER_SIZE / (p_dsp_info->sub_info.block_info.block_size);
            dl_config->sink.bt_out.bt_info[i].share_info = p_dsp_info;
         }
        dl_config->sink.bt_out.channel_num = 1;
    }
    dl_config->source.usb_in.frame_max_num = frame_num;
    dl_config->sink.bt_out.frame_interval = frame_interval;
    interval = frame_interval/1000;
    dl_config->sink.bt_out.sample_rate    = sample_rate;
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]config_init usb in, interval:0x%x", 1, frame_interval);
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]config_init usb in, mtu:0x%x,max_num:0x%x, blocks:0x%x, sub_band:0x%x", 4, mtu, dl_config->source.usb_in.frame_max_num,blocks, sub_band);
    config->transmitter_config.scenario_type = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE;
    config->transmitter_config.user_data = user_data;
    config->transmitter_config.msg_handler = user_callback;
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
    if  (port_context.port == BT_SOURCE_SRV_PORT_LINE_IN) {
            config->transmitter_config.scenario_sub_id = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0;
    }
    else  if (port_context.port == BT_SOURCE_SRV_PORT_I2S_IN) {
        config->transmitter_config.scenario_sub_id = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1;
    }
    else if (port_context.port == BT_SOURCE_SRV_PORT_I2S_IN_1) {
        config->transmitter_config.scenario_sub_id = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2;
    } else
#endif
    {
        config->transmitter_config.scenario_sub_id = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0;
    }
    bt_source_srv_music_pseduo_dev_t *device = (bt_source_srv_music_pseduo_dev_t*)user_data;
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]config_init, detect_flag:0x%x", 1, context->detect_flag);

    if (!device->audio_play_num && !context->detect_flag) {
        bt_source_srv_music_set_audio_notify(p_dsp_info, interval, frame_size, notify_type);
    }

    return BT_STATUS_SUCCESS;
}


bt_source_srv_music_audio_id_t bt_source_srv_music_audio_start(bt_source_srv_music_audio_config_t *config)
{
    bt_source_srv_music_audio_id_t audio_id = BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID;
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]bt_source_srv_music_audio_init: enable: 0x%x", 1,config->transmitter_config.scenario_config.bt_audio_dongle_config.dl_info.sink.bt_out.without_bt_link_mode_enable);
    audio_id = audio_transmitter_init(&config->transmitter_config);
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]bt_source_srv_music_audio_start, audio_id:0x%x", 1, audio_id);
    if (audio_id == BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID) {
        return audio_id;
    }

    audio_transmitter_status_t audio_status = audio_transmitter_start(audio_id);
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]bt_source_srv_music_audio_start, audio_status:0x%x", 1, audio_status);
    if (audio_status != AUDIO_TRANSMITTER_STATUS_SUCCESS) {
        return audio_id;
    }

    return audio_id;
}

bt_status_t bt_source_srv_music_audio_stop(audio_transmitter_id_t id)
{
    audio_transmitter_status_t audio_status = audio_transmitter_set_runtime_config(id, BT_AUDIO_DONGLE_CONFIG_OP_USB_DETECT_DISABLE, NULL);
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]bt_source_srv_music_audio_stop, start:0x%x", 1, audio_status);

#if defined(AIR_BT_AUDIO_DONGLE_ENABLE) && defined(AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE)
    audio_silence_detection_scenario_stop(AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0);
#endif

    audio_status = audio_transmitter_stop(id);
    if (audio_status != AUDIO_TRANSMITTER_STATUS_SUCCESS) {
        return BT_STATUS_FAIL;
    }
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]bt_source_srv_music_audio_stop, stop:0x%x", 1, audio_status);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_srv_music_audio_deinit(audio_transmitter_id_t id)
{
    audio_transmitter_status_t status = audio_transmitter_deinit(id);
    LOG_MSGID_I(source_srv,"[A2DP_SOURCE]bt_source_srv_music_audio_deinit, id:0x%x, status:0x%x", 2, id, status);
    if (status != AUDIO_TRANSMITTER_STATUS_SUCCESS) {
        return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

#if (defined AIR_BT_AUDIO_DONGLE_ENABLE) || (defined AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE)

static void bt_source_srv_music_psd_audio_dectection_media_data_callback(bt_audio_notify_event_t event)
{
    uint8_t bt_event = (uint8_t)event;
    bt_source_srv_music_device_t *context = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_HIGHLIGHT, NULL);
    if (context == NULL) {
        context = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_VAILD_DEVICE, NULL);

    }
    if (context != NULL) {
        bt_source_srv_music_pseduo_dev_t *device = (bt_source_srv_music_pseduo_dev_t *)context->audio_dev;
        LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR]detect_media_data_timer:device = 0x%x, event= 0x%x", 2, device, bt_event);

        if (device->user_musicback) {
            device->user_musicback(BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_MEDIA_DETECT, (void *)device, &event);
        }
    }

}

#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
static void bt_source_srv_silence_detection_callback(audio_scenario_type_t scenario_type, bool silence_flag)
{
    bt_audio_notify_event_t event = AUDIO_BT_DONGLE_USB_DATA_SUSPEND;
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] bt_source_srv_silence_detection_callback:scenario_type,0x%0x, id = 0x%x", 2, scenario_type, silence_flag);

    if (silence_flag) {
        //detect silence

    } else {
        //detect data (not silence)
        event = AUDIO_BT_DONGLE_USB_DATA_RESUME;
    }
    bt_source_srv_music_psd_audio_dectection_media_data_callback(event);

}
#endif

void bt_source_srv_music_psd_start_detect_media_data(int8_t audio_id)
{
    audio_transmitter_status_t audio_status = AUDIO_TRANSMITTER_STATUS_FAIL;
    audio_transmitter_runtime_config_t audio_config = {0};
    bt_audio_usb_detect_config_t detect_config = {0};
    detect_config.timer_period_ms = 500;//2S check1��
    detect_config.notify_threshold_ms = 2000;// check 3s
    detect_config.cb = bt_source_srv_music_psd_audio_dectection_media_data_callback;
    memcpy(&(audio_config.bt_audio_runtime_config.config.usb_detect), &detect_config, sizeof(bt_audio_usb_detect_config_t));
    audio_status = audio_transmitter_set_runtime_config(audio_id, BT_AUDIO_DONGLE_CONFIG_OP_USB_DETECT_ENABLE, &audio_config);
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] start_detect_media_data:audio_status,0x%0x, id = 0x%x", 2, audio_status, audio_id);

#ifdef AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
    audio_scenario_type_t audio_scenario = AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0;
    audio_silence_detection_scenario_start(audio_scenario, bt_source_srv_silence_detection_callback);
#endif

}
#endif

bt_status_t bt_source_src_music_audio_mute(audio_transmitter_id_t sub_id)
{
    if (BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID == sub_id) {
        return BT_STATUS_FAIL;
    }
    audio_transmitter_status_t audio_status = audio_transmitter_set_runtime_config(sub_id, BT_AUDIO_DONGLE_CONFIG_OP_SET_MUTE, NULL);
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] audio play type = %02x mute status = %02x", 2, sub_id, audio_status);
    return (audio_status == AUDIO_TRANSMITTER_STATUS_SUCCESS) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

bt_status_t bt_source_src_music_audio_unmute(audio_transmitter_id_t sub_id)
{
    if (BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID == sub_id) {
        return BT_STATUS_FAIL;
    }
    audio_transmitter_status_t audio_status = audio_transmitter_set_runtime_config(sub_id, BT_AUDIO_DONGLE_CONFIG_OP_SET_UNMUTE, NULL);
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] audio play type = %02x unmute status = %02x", 2, sub_id, audio_status);
    return (audio_status == AUDIO_TRANSMITTER_STATUS_SUCCESS) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}



