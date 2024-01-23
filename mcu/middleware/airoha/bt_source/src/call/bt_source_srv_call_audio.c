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

#include "bt_sink_srv_ami.h"
#include "bt_source_srv_call_audio.h"
#include "bt_source_srv_hfp.h"
#if defined(MTK_AVM_DIRECT)
#include "bt_avm.h"
#endif
#include "bt_source_srv_internal.h"
#include "bt_source_srv_common.h"
#include "bt_source_srv_utils.h"
#include "hal_audio_message_struct_common.h"
#include "avm_external.h"
#include "scenario_bt_audio.h"


#define BT_SOURCE_SRV_CALL_AUDIO_PORT_MAX                          BT_SOURCE_SRV_PORT_MIC
#define BT_SOURCE_SRV_CALL_AUDIO_DSP_FORMAT_MAX                    5U
#define BT_SOURCE_SRV_CALL_AUDIO_LINK_MAX                          1U
#define BT_SOURCE_SRV_CALL_AUDIO_FRAME_INTERVAL                    1000U

#define BT_SOURCE_SRV_CALL_AUDIO_FRAME_MAX_NUMBER                  1U

#define BT_SOURCE_SRV_CALL_AUDIO_CHANNEL_NUMBER                    1U

#define BT_SOURCE_SRV_CALL_AUDIO_INVALID_SUB_ID                    0xFF

/* 7.5ms frame size */
#define BT_SOURCE_SRV_CALL_AUDIO_CVSD_DL_FRAME_SIZE                120U
#define BT_SOURCE_SRV_CALL_AUDIO_CVSD_UL_FRAME_SIZE                120U

#define BT_SOURCE_SRV_CALL_AUDIO_MSBC_DL_FRAME_SIZE                60U
#define BT_SOURCE_SRV_CALL_AUDIO_MSBC_UL_FRAME_SIZE                60U

#define BT_SOURCE_SRV_CALL_AUDIO_SAMPLE_RATE_8K                    8000U
#define BT_SOURCE_SRV_CALL_AUDIO_SAMPLE_RATE_16K                   16000U

#define BT_SOURCE_SRV_CALL_AUDIO_BT_FRAME_INTERVAL                 7500U   /* UL 7.5ms + DL 7.5ms */

typedef enum {
    BT_SOURCE_SRV_AVM_SHARE_INFO_DL_CH_1 = 0,
    BT_SOURCE_SRV_AVM_SHARE_INFO_UL_CH_1,
    BT_SOURCE_SRV_AVM_SHARE_INFO_TYPE_MAX
} bt_source_srv_avm_share_info_t;

const static uint32_t g_audio_dsp_format_mapping[BT_SOURCE_SRV_CALL_AUDIO_DSP_FORMAT_MAX] = {
    HAL_AUDIO_PCM_FORMAT_DUMMY,    /* AFE_PCM_FORMAT_DUMMY */
    HAL_AUDIO_PCM_FORMAT_U8,       /* AFE_PCM_FORMAT_U8 */
    HAL_AUDIO_PCM_FORMAT_S16_LE,   /* AFE_PCM_FORMAT_S16_LE */
    HAL_AUDIO_PCM_FORMAT_S24_LE,   /* AFE_PCM_FORMAT_S24_LE */
    HAL_AUDIO_PCM_FORMAT_S32_LE    /* AFE_PCM_FORMAT_S32_LE */
};

static const bt_source_srv_call_audio_codec_msbc_t g_audio_msbc_capacity = {
    .min_bit_pool = 26,          /**< The minimum bit pool. */
    .max_bit_pool = 26,          /**< The maximum bit pool. */
    .block_length = 0x01,        /**< b0: 16, b1: 12, b2: 8, b3: 4. */
    .subband_num  = 0x01,        /**< b0: 8, b1: 4. */
    .alloc_method = 0x01,        /**< b0: loudness, b1: SNR. */
    .sample_rate  = 0x08,        /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    .channel_mode = 0x08         /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
};

static audio_dsp_codec_type_t bt_source_srv_call_audio_dsp_codec_mapping(bt_source_srv_call_audio_codec_type_t codec_type)
{
    audio_dsp_codec_type_t dsp_codec_type = AUDIO_DSP_CODEC_TYPE_DUMMY;
    switch (codec_type) {
        case BT_HFP_CODEC_TYPE_CVSD: {
            dsp_codec_type = AUDIO_DSP_CODEC_TYPE_CVSD;
        }
        break;
        case BT_HFP_CODEC_TYPE_MSBC: {
            dsp_codec_type = AUDIO_DSP_CODEC_TYPE_MSBC;
        }
        break;
        default:
            break;
    }
    LOG_MSGID_I(source_srv, "[AG][AUDIO] codec = %02x mapping dsp codec %02x", 2, codec_type, dsp_codec_type);
    return dsp_codec_type;
}

static void *bt_source_srv_call_audio_get_share_info(bt_source_srv_avm_share_info_t type)
{
    n9_dsp_share_info_t *forwarder_share_info = NULL;
    switch (type) {
        case BT_SOURCE_SRV_AVM_SHARE_INFO_DL_CH_1: {
            forwarder_share_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_BT_VOICE_UL);
            if (forwarder_share_info == NULL) {
                LOG_MSGID_E(source_srv, "[AG][AUDIO] query transmitter share info fail", 0);
                return NULL;
            }
            forwarder_share_info->read_offset = 0;
            forwarder_share_info->write_offset = 0;
        }
        break;
        case BT_SOURCE_SRV_AVM_SHARE_INFO_UL_CH_1: {
            /* hal audio UL/DL for sink, so the source should be flipped */
            forwarder_share_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_BT_VOICE_DL);
            if (forwarder_share_info == NULL) {
                LOG_MSGID_E(source_srv, "[AG][AUDIO] query transmitter share info fail", 0);
                return NULL;
            }
            forwarder_share_info->read_offset = 0;
            forwarder_share_info->write_offset = 0;
        }
        break;
        default:
            break;
    }
    return (void *)forwarder_share_info;
}

bt_status_t bt_source_srv_call_audio_controller_config(bt_source_srv_call_audio_codec_type_t codec_type)
{
    bt_avm_ext_share_buffer_info_t buffer_info = {0};
    uint32_t frame_size = (codec_type == BT_HFP_CODEC_TYPE_MSBC) ? BT_SOURCE_SRV_CALL_AUDIO_MSBC_DL_FRAME_SIZE : BT_SOURCE_SRV_CALL_AUDIO_CVSD_DL_FRAME_SIZE;
    buffer_info.sco_up_address = (uint32_t)bt_source_srv_call_audio_get_share_info(BT_SOURCE_SRV_AVM_SHARE_INFO_UL_CH_1);
    buffer_info.sco_dl_address = (uint32_t)bt_source_srv_call_audio_get_share_info(BT_SOURCE_SRV_AVM_SHARE_INFO_DL_CH_1);
    buffer_info.reserve |= (((uint8_t)(((frame_size + 3) >> 2) * 4)) << 16);    /* DL packet length:bit16~bit23 */
    buffer_info.reserve |= (((uint8_t)frame_size) << 24);                       /* UL packet length:bit24~bit31 */
    buffer_info.reserve |= 0x01;                                                /* bit0: notify contollter dongle role */
    LOG_MSGID_I(source_srv, "[AG][AUDIO] audio controller config up address = %02x, dl_address = %02x, reserve = %02x", 3,
                buffer_info.sco_up_address, buffer_info.sco_dl_address, buffer_info.reserve);
    return bt_avm_set_ext_share_buffer(&buffer_info);
}

bt_status_t bt_source_srv_call_audio_trigger_play(uint32_t gap_handle)
{
    bt_status_t status = BT_STATUS_FAIL;
#if defined(MTK_AVM_DIRECT)
    bt_clock_t play_clk = {0};
    LOG_MSGID_I(source_srv, "[AG][AUDIO] initial sync, handle:0x%x", 1, gap_handle);
    if (gap_handle) {
        status = bt_avm_set_audio_tracking_time(gap_handle, BT_AVM_TYPE_CALL, &play_clk);
    }
#endif
    return status;
}

bt_source_srv_call_audio_id_t bt_source_srv_call_audio_config_init(bt_source_srv_call_audio_config_t *config, bt_source_srv_call_audio_codec_type_t codec_type, void *user_data, void *user_callback)
{
    uint32_t i = 0;
    bt_source_srv_common_audio_port_context_t port_context = {0};
    LOG_MSGID_I(source_srv, "[AG][AUDIO] audio config init type = %02x codec type = %02x", 2, config->type, codec_type);
    switch (config->type) {
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL: {
            bool is_audio_port_chat = false;
            bool is_force_open = false;
            bt_audio_dongle_dl_info_t *dl_config = &config->transmitter_config.scenario_config.bt_audio_dongle_config.dl_info;
            if (bt_source_srv_common_audio_find_port_context(BT_SOURCE_SRV_PORT_CHAT_SPEAKER, &port_context) != BT_STATUS_SUCCESS) {
                if ((bt_source_srv_common_audio_find_port_context(BT_SOURCE_SRV_PORT_GAMING_SPEAKER, &port_context) != BT_STATUS_SUCCESS) &&
                        (bt_source_srv_common_audio_port_is_valid(BT_SOURCE_SRV_PORT_MIC))) {
                    /*
                     * type:PC workaround
                     * root cause:call audio no sound when only MIC open.
                     * solution:call need to force open DL for use deafult DL port parameter when only MIC port open.
                     */
                    bt_source_srv_common_audio_get_default_port_parameter(BT_SOURCE_SRV_PORT_CHAT_SPEAKER, &port_context);
                    is_audio_port_chat = true;
                    is_force_open = true;
                }
            } else {
                is_audio_port_chat = true;
            }

            if ((!is_force_open) && (!bt_source_srv_common_audio_port_is_valid(port_context.port))) {
                return BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID;
            }
            /* PC->audio:source config */
            dl_config->source.usb_in.codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
            dl_config->source.usb_in.codec_param.pcm.sample_rate = port_context.sample_rate;
            dl_config->source.usb_in.codec_param.pcm.channel_mode = port_context.sample_channel;
            dl_config->source.usb_in.codec_param.pcm.format = g_audio_dsp_format_mapping[port_context.sample_size];
            dl_config->source.usb_in.codec_param.pcm.frame_interval = BT_SOURCE_SRV_CALL_AUDIO_FRAME_INTERVAL;
            dl_config->source.usb_in.frame_max_num = BT_SOURCE_SRV_CALL_AUDIO_FRAME_MAX_NUMBER;
            LOG_MSGID_I(source_srv, "[AG][AUDIO] audio DL config pcm sample_rate = %02x channel_mode = %02x, format = %02x", 3,
                        port_context.sample_rate,  port_context.sample_channel, g_audio_dsp_format_mapping[port_context.sample_size]);
            /* audio->BT:sink config */
            dl_config->sink.bt_out.link_num = BT_SOURCE_SRV_CALL_AUDIO_LINK_MAX;

            for (i = 0; i < BT_SOURCE_SRV_CALL_AUDIO_LINK_MAX; i++) {
                dl_config->sink.bt_out.bt_info[i].enable = true;
                dl_config->sink.bt_out.bt_info[i].share_info = bt_source_srv_call_audio_get_share_info(BT_SOURCE_SRV_AVM_SHARE_INFO_DL_CH_1);
                dl_config->sink.bt_out.bt_info[i].codec_type = bt_source_srv_call_audio_dsp_codec_mapping(codec_type);
                dl_config->sink.bt_out.sample_rate = (codec_type == BT_HFP_CODEC_TYPE_MSBC) ? BT_SOURCE_SRV_CALL_AUDIO_SAMPLE_RATE_16K : BT_SOURCE_SRV_CALL_AUDIO_SAMPLE_RATE_8K;
                dl_config->sink.bt_out.sample_format = HAL_AUDIO_PCM_FORMAT_S16_LE;
                dl_config->sink.bt_out.frame_interval = BT_SOURCE_SRV_CALL_AUDIO_BT_FRAME_INTERVAL;
                dl_config->sink.bt_out.channel_num = BT_SOURCE_SRV_CALL_AUDIO_CHANNEL_NUMBER;
                if (codec_type == BT_HFP_CODEC_TYPE_MSBC) {
                    bt_source_srv_memcpy(&dl_config->sink.bt_out.bt_info[i].codec_param.msbc, &g_audio_msbc_capacity, sizeof(bt_source_srv_call_audio_codec_msbc_t));
                }
            }

            config->transmitter_config.scenario_type = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE;
            config->transmitter_config.user_data = user_data;
            config->transmitter_config.msg_handler = user_callback;
            config->transmitter_config.scenario_sub_id = is_audio_port_chat ? AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0 : AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1;
        }
        break;
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL: {
            bt_audio_dongle_ul_info_t *ul_config = &config->transmitter_config.scenario_config.bt_audio_dongle_config.ul_info;
            if ((bt_source_srv_common_audio_find_port_context(BT_SOURCE_SRV_PORT_MIC, &port_context) != BT_STATUS_SUCCESS) ||
                    (!bt_source_srv_common_audio_port_is_valid(port_context.port))) {
                return BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID;
            }

            /* BT->audio:source config */
            ul_config->source.bt_in.link_num = BT_SOURCE_SRV_CALL_AUDIO_LINK_MAX;
            for (i = 0; i < BT_SOURCE_SRV_CALL_AUDIO_LINK_MAX; i++) {
                ul_config->source.bt_in.bt_info[i].enable = true;
                ul_config->source.bt_in.bt_info[i].share_info = bt_source_srv_call_audio_get_share_info(BT_SOURCE_SRV_AVM_SHARE_INFO_UL_CH_1);
                ul_config->source.bt_in.bt_info[i].codec_type = bt_source_srv_call_audio_dsp_codec_mapping(codec_type);
                ul_config->source.bt_in.sample_rate = (codec_type == BT_HFP_CODEC_TYPE_MSBC) ? BT_SOURCE_SRV_CALL_AUDIO_SAMPLE_RATE_16K : BT_SOURCE_SRV_CALL_AUDIO_SAMPLE_RATE_8K;
                ul_config->source.bt_in.sample_format = HAL_AUDIO_PCM_FORMAT_S16_LE;
                ul_config->source.bt_in.frame_interval = BT_SOURCE_SRV_CALL_AUDIO_BT_FRAME_INTERVAL;
                ul_config->source.bt_in.channel_num = BT_SOURCE_SRV_CALL_AUDIO_CHANNEL_NUMBER;
                if (codec_type == BT_HFP_CODEC_TYPE_MSBC) {
                    bt_source_srv_memcpy(&ul_config->source.bt_in.bt_info[i].codec_param.msbc, &g_audio_msbc_capacity, sizeof(bt_source_srv_call_audio_codec_msbc_t));
                }
            }

            /* audio->PC:sink config */
            ul_config->sink.usb_out.codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
            ul_config->sink.usb_out.codec_param.pcm.sample_rate = port_context.sample_rate;
            ul_config->sink.usb_out.codec_param.pcm.channel_mode = port_context.sample_channel;
            ul_config->sink.usb_out.codec_param.pcm.format = g_audio_dsp_format_mapping[port_context.sample_size];
            ul_config->sink.usb_out.codec_param.pcm.frame_interval = BT_SOURCE_SRV_CALL_AUDIO_FRAME_INTERVAL;
            ul_config->sink.usb_out.frame_max_num = BT_SOURCE_SRV_CALL_AUDIO_FRAME_MAX_NUMBER;
            LOG_MSGID_I(source_srv, "[AG][AUDIO] audio UL config pcm sample_rate = %02x channel_mode = %02x, format = %02x", 3,
                        port_context.sample_rate,  port_context.sample_channel, g_audio_dsp_format_mapping[port_context.sample_size]);

            config->transmitter_config.scenario_type = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE;
            config->transmitter_config.user_data = user_data;
            config->transmitter_config.msg_handler = user_callback;
            config->transmitter_config.scenario_sub_id = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0;
        }
        break;
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN:
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_I2S_IN: {

            bt_source_srv_port_t port = (BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN == config->type) ?
                                        BT_SOURCE_SRV_PORT_LINE_IN : BT_SOURCE_SRV_PORT_I2S_IN;

            if (bt_source_srv_common_audio_find_port_context(port, &port_context) != BT_STATUS_SUCCESS) {
                return BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID;
            }
            bt_audio_dongle_dl_info_t *dl_config = &config->transmitter_config.scenario_config.bt_audio_dongle_config.dl_info;

            /* audio->BT:sink config */
            dl_config->sink.bt_out.link_num = BT_SOURCE_SRV_CALL_AUDIO_LINK_MAX;
            for (i = 0; i < BT_SOURCE_SRV_CALL_AUDIO_LINK_MAX; i++) {
                dl_config->sink.bt_out.bt_info[i].enable = true;
                dl_config->sink.bt_out.bt_info[i].share_info = bt_source_srv_call_audio_get_share_info(BT_SOURCE_SRV_AVM_SHARE_INFO_DL_CH_1);
                dl_config->sink.bt_out.bt_info[i].codec_type = bt_source_srv_call_audio_dsp_codec_mapping(codec_type);
                dl_config->sink.bt_out.sample_rate = (codec_type == BT_HFP_CODEC_TYPE_MSBC) ? BT_SOURCE_SRV_CALL_AUDIO_SAMPLE_RATE_16K : BT_SOURCE_SRV_CALL_AUDIO_SAMPLE_RATE_8K;
                dl_config->sink.bt_out.sample_format = HAL_AUDIO_PCM_FORMAT_S16_LE;
                dl_config->sink.bt_out.frame_interval = BT_SOURCE_SRV_CALL_AUDIO_BT_FRAME_INTERVAL;
                dl_config->sink.bt_out.channel_num = BT_SOURCE_SRV_CALL_AUDIO_CHANNEL_NUMBER;
                if (codec_type == BT_HFP_CODEC_TYPE_MSBC) {
                    bt_source_srv_memcpy(&dl_config->sink.bt_out.bt_info[i].codec_param.msbc, &g_audio_msbc_capacity, sizeof(bt_source_srv_call_audio_codec_msbc_t));
                }
            }

            config->transmitter_config.scenario_type = AUDIO_TRANSMITTER_BT_AUDIO_DONGLE;
            config->transmitter_config.user_data = user_data;
            config->transmitter_config.msg_handler = user_callback;
            config->transmitter_config.scenario_sub_id = (BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN == config->type) ? \
                    AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0 : AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1;
        }
        break;
#endif
        default:
            return BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID;
    }

    bt_source_srv_call_audio_id_t audio_id = audio_transmitter_init(&config->transmitter_config);
    if (audio_id == BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID) {
        LOG_MSGID_E(source_srv, "[AG][AUDIO] audio config init fail for invalid id", 0);
        return BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID;
    }
    LOG_MSGID_I(source_srv, "[AG][AUDIO] audio config init id = %02x success", 1, audio_id);
    return audio_id;
}

bt_status_t bt_source_srv_call_audio_play(bt_source_srv_call_audio_id_t audio_id)
{
    audio_transmitter_status_t audio_status = audio_transmitter_start(audio_id);
    if (audio_status != AUDIO_TRANSMITTER_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[AG][AUDIO] audio play fail status = %02x", 1, audio_status);
        return BT_STATUS_FAIL;
    }

    LOG_MSGID_I(source_srv, "[AG][AUDIO] audio play success id = %02x", 1, audio_id);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_srv_call_audio_stop(bt_source_srv_call_audio_id_t audio_id)
{
    audio_transmitter_status_t audio_status = audio_transmitter_stop(audio_id);
    if (audio_status != AUDIO_TRANSMITTER_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[AG][AUDIO] audio stop fail id = %02x, status = %02x", 2, audio_id, audio_status);
        return BT_STATUS_FAIL;
    }
    LOG_MSGID_I(source_srv, "[AG][AUDIO] audio stop success id = %02x", 1, audio_id);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_srv_call_audio_config_deinit(bt_source_srv_call_audio_id_t audio_id)
{
    if (audio_id == BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID) {
        LOG_MSGID_W(source_srv, "[AG][AUDIO] audio config deinit id is invalid", 0);
        return BT_STATUS_FAIL;
    }

    audio_transmitter_status_t audio_status = audio_transmitter_deinit(audio_id);
    if (audio_status != AUDIO_TRANSMITTER_STATUS_SUCCESS) {
        LOG_MSGID_E(source_srv, "[AG][AUDIO] audio config deinit fail id = %02x, status = %02x", 2, audio_id, audio_status);
        return BT_STATUS_FAIL;
    }
    LOG_MSGID_I(source_srv, "[AG][AUDIO] audio config deinit id = %02x success", 1, audio_id);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_src_call_audio_mute(bt_source_srv_call_audio_id_t audio_id)
{
    audio_transmitter_status_t audio_status = audio_transmitter_set_runtime_config(audio_id, BT_AUDIO_DONGLE_CONFIG_OP_SET_MUTE, NULL);
    LOG_MSGID_I(source_srv, "[AG][AUDIO] mute audio id = %02x status = %02x", 2, audio_id, audio_status);
    return (audio_status == AUDIO_TRANSMITTER_STATUS_SUCCESS) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

bt_status_t bt_source_src_call_audio_unmute(bt_source_srv_call_audio_id_t audio_id)
{
    audio_transmitter_status_t audio_status = audio_transmitter_set_runtime_config(audio_id, BT_AUDIO_DONGLE_CONFIG_OP_SET_UNMUTE, NULL);
    LOG_MSGID_I(source_srv, "[AG][AUDIO] unmute audio id = %02x status = %02x", 2, audio_id, audio_status);
    return (audio_status == AUDIO_TRANSMITTER_STATUS_SUCCESS) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

void bt_source_srv_call_audio_slience_detection_start(bt_source_srv_call_audio_id_t audio_id, void *callback)
{
    audio_transmitter_runtime_config_t audio_config = {
        .bt_audio_runtime_config.config.usb_detect.timer_period_ms = 500,
        .bt_audio_runtime_config.config.usb_detect.notify_threshold_ms = 1000,
        .bt_audio_runtime_config.config.usb_detect.cb = callback,
    };

    audio_transmitter_status_t audio_status = audio_transmitter_set_runtime_config(audio_id, BT_AUDIO_DONGLE_CONFIG_OP_USB_DETECT_ENABLE, &audio_config);

    LOG_MSGID_I(source_srv, "[AG][AUDIO] audio id = %02x slience detection start status = %02x", 2, audio_id, audio_status);
}

void bt_source_srv_call_audio_slience_detection_stop(bt_source_srv_call_audio_id_t audio_id)
{
    audio_transmitter_status_t audio_status = audio_transmitter_set_runtime_config(audio_id, BT_AUDIO_DONGLE_CONFIG_OP_USB_DETECT_DISABLE, NULL);
    LOG_MSGID_I(source_srv, "[AG][AUDIO] audio id = %02x slience detection stop status = %02x", 2, audio_id, audio_status);
}
