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

#include "bt_type.h"
#include "bt_callback_manager.h"
#include "bt_sink_srv_ami.h"
#include "hal_audio_message_struct.h"
#include "hal_audio_message_struct_common.h"

#include "bt_ull_utility.h"
#include "bt_utils.h"

extern bt_ull_context_t g_bt_ull_context;

static bt_ull_req_t g_ull_tx_cache[BT_ULL_TX_MAX_CACHE];
static bool g_bt_ull_mode = false;

#if defined(AIR_ECNR_POST_PART_ENABLE) || defined(AIR_ECNR_PREV_PART_ENABLE)
#define A_MIC_FRAME_SIZE                       31 /*mic frame size is 31B */
#define B_MIC_FRAME_SIZE                       48 /*mic frame size is 48B */
static uint8_t g_bt_frame_size = A_MIC_FRAME_SIZE; /*NR default is A_MIC_FRAME_SIZE */
#else
#define A_MIC_FRAME_SIZE                       30 /*mic frame size is 30B */
#define B_MIC_FRAME_SIZE                       47 /*mic frame size is 47B */
static uint8_t g_bt_frame_size = A_MIC_FRAME_SIZE; /*default is A_MIC_FRAME_SIZE */
#endif

bt_ull_streaming_if_info_t *streaming_interface[] = {
   &g_bt_ull_context.dl_speaker,
   &g_bt_ull_context.dl_chat,
   &g_bt_ull_context.ul_microphone,
};

//log_create_module(ULL, PRINT_LEVEL_INFO);
static void bt_ull_init_transmitter_volume(bt_ull_streaming_t *streaming, bt_sink_srv_am_volume_level_t default_vol);
//static bt_sink_srv_am_volume_level_out_t bt_ull_volume_exchange_via_ratio(bt_sink_srv_am_volume_level_out_t vol, uint8_t ratio);
static void bt_ull_mic_resource_callback(struct _audio_src_srv_resource_manager_handle_t *current_handle, audio_src_srv_resource_manager_event_t event);
void bt_ull_mix_streaming_handle(bt_ull_streaming_if_info_t *master_in, bt_ull_streaming_if_info_t *sub_in);

void bt_ull_set_music_enable(uint32_t handle, bt_avm_role_t role, bool enable, uint16_t dl_latency)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    uint16_t dl_retry = ((dl_latency - 15) * 10) / 25; /* base latency 15ms, 1 retry = 2.5ms */
    /* uplink range: 25ms/40ms/60ms */
    uint16_t uplink_retry;
    if (BT_ULL_UPLINK_MAX == ctx->ul_latency) {
        uplink_retry = 18;
    } else if (BT_ULL_DOWNLINK_LIMIATTION == ctx->ul_latency) {
        uplink_retry = 2;   // air latency: 7.5ms
    } else {
        uplink_retry = 4;   // air latency: 15ms
    }
    ull_report("[ULL][COMMON] set_music_enable, handle:0x%x, roler:0x%x, enable:0x%x, dl_retry:%d, cur_avm:0x%x, ul_retry:%d", 6,
               handle, role, enable, dl_retry, ctx->avm_enable, uplink_retry);
    if (ctx->avm_enable != enable) {
        if (handle) {
            bt_status_t status = bt_avm_set_ull_gaming_mode_enable(handle, role, enable, dl_retry, uplink_retry);
            while (status == BT_STATUS_OUT_OF_MEMORY) {
                vTaskDelay(5);
                status = bt_avm_set_ull_gaming_mode_enable(handle, role, enable, dl_retry, uplink_retry);
            }
        }
        ctx->avm_enable = enable;
    }
}

void bt_ull_update_audio_buffer(bt_ull_role_t role, bt_ull_client_t client_type)
{
    bt_avm_ext_share_buffer_info_t audio_ext_buf = {0};

    avm_share_buf_info_t *info;
    info = (avm_share_buf_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_BT_RECEIVE_FROM_AIR);
    n9_dsp_share_info_t *p_share_info;
    p_share_info  = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_BT_SEND_TO_AIR);
    uint8_t mic_block_size = bt_ull_get_mic_frame_size();

    if (BT_ULL_ROLE_CLIENT == role) {
        hal_audio_set_gaming_mode_avm_info(info, SHARE_BUFFER_GAMING_MODE_SIZE, (((80 + 1) * 3 + 8 + 3) / 4) * 4);
        audio_ext_buf.music_dl_address = (uint32_t)info;
        hal_audio_set_audio_transmitter_share_info(p_share_info, p_share_info->length, mic_block_size);
        audio_ext_buf.voice_up_address = (uint32_t)p_share_info;
    } else {
        hal_audio_set_gaming_mode_avm_info(info, SHARE_BUFFER_GAMING_MODE_SIZE, ((mic_block_size + 1) * 1 + 8));
        audio_ext_buf.music_dl_address = (uint32_t)info;
        hal_audio_set_audio_transmitter_share_info(p_share_info, p_share_info->length, 80 + 4); /* 80B data + 4B CRC header */
        audio_ext_buf.voice_up_address = (uint32_t)p_share_info;
        audio_ext_buf.reserve |= 0x01;  /* bit0: notify contollter dongle role */
    }
    if (BT_ULL_HEADSET_CLIENT == client_type) {
        audio_ext_buf.reserve |= 0x00;  /* bit1~bit2: 0 means headset */
    } else if (BT_ULL_EARBUDS_CLIENT == client_type) {
        audio_ext_buf.reserve |= 0x02;  /* bit1~bit2: 1 means earbuds  */
    } else {
        bt_utils_assert(0 && "dongle side recieve known client type!");
    }
    audio_ext_buf.reserve |= (uint32_t)(((uint32_t)mic_block_size << 24) | ((uint32_t)80 << 16)); /* bit24~bit31:voice up-link audio frame size.  bit16~bit23:music down-link audio frame size  */
    audio_ext_buf.sco_dl_address = (uint32_t)hal_audio_query_bt_voice_dl_share_info();
    audio_ext_buf.sco_up_address = (uint32_t)hal_audio_query_bt_voice_ul_share_info();
    audio_ext_buf.clock_mapping_address = (uint32_t)hal_audio_query_ull_rcdc_share_info();

    extern uint32_t sub_chip_version_get();
    uint32_t sub_version = sub_chip_version_get();
    ull_report("[ULL][COMMON] update audio buffer, sub_chip_version:0x%x", 1, sub_version);

    bt_status_t ret = bt_avm_set_ext_share_buffer(&audio_ext_buf);
    ull_report("[ULL][COMMON] update audio buffer, reserve:0x%x, ret:0x%x, client_type:0x%x", 3, audio_ext_buf.reserve, ret, client_type);
}


void bt_ull_save_last_connected_device_info(uint8_t *address, bt_cm_profile_service_mask_t profiles)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL][COMMON] save last connected device info, profiles: 0x%x", 1, profiles);
    memcpy(ctx->remode_device.bt_addr, address, BT_BD_ADDR_LEN);
    ctx->remode_device.profile = profiles;
}

void bt_ull_clear_last_connected_device_info(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL][COMMON] clear last connected device info", 0);
    memset(ctx->remode_device.bt_addr, 0x00, BT_BD_ADDR_LEN);
    ctx->remode_device.profile = BT_CM_PROFILE_SERVICE_MASK_NONE;
}

bt_sink_srv_am_volume_level_out_t bt_ull_get_volume_level(uint8_t volume)
{
    float local_level_f = AUD_VOL_OUT_LEVEL0;
    float orignal_f = volume;
    bt_sink_srv_am_volume_level_t max_lev = AUD_VOL_OUT_LEVEL15;

    local_level_f = (orignal_f * max_lev) / BT_ULL_USB_VOLUME_MAX + 0.5f;
    ull_report("[ULL][COMMON] get_volume_level[s]-orignal: %d, vol_level: %d", 2, volume, (uint8_t)local_level_f);
    return (uint8_t)local_level_f;
}

bt_ull_streaming_if_info_t *bt_ull_get_streaming_interface(bt_ull_streaming_t *streaming)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_streaming_if_info_t *ptr= NULL;

    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface && 0x00 == streaming->port) {
        ptr = &(ctx->dl_speaker);
    } else if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface && 0x01 == streaming->port) {
        ptr = &(ctx->dl_chat);
    } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface) {
        ptr = &(ctx->ul_microphone);
    }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
    else if (BT_ULL_STREAMING_INTERFACE_LINE_IN == streaming->streaming_interface) {
        ptr = &(ctx->dl_linein);
     }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
     else if (BT_ULL_STREAMING_INTERFACE_LINE_OUT == streaming->streaming_interface) {
       ptr = &(ctx->ul_lineout);
     }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
     else if (BT_ULL_STREAMING_INTERFACE_I2S_IN == streaming->streaming_interface) {
      ptr = &(ctx->dl_linei2s);
     }
#endif
     else {
      bt_utils_assert(0 && "unknown usb end pointer type");
      return NULL;
    }
  return ptr;
}


uint32_t bt_ull_get_usb_sample_rate(bt_ull_transmitter_t transmitter_type)
{
    bt_ull_streaming_if_info_t *p_streaming = NULL;
    bt_ull_context_t *ctx = bt_ull_get_context();
    p_streaming = streaming_interface[transmitter_type];

    if(BT_ULL_ROLE_SERVER == ctx->ull_role) {
       if (transmitter_type <= BT_ULL_MIC_TRANSMITTER) {
            return p_streaming->sample_rate;
       }
    }
    ull_report("[ULL] bt_ull_srv_get_usb_sample_rate, transmitter %d not suppotrt", 1, transmitter_type);
    return 0;
}

uint32_t bt_ull_get_usb_sample_size(bt_ull_transmitter_t transmitter_type)
{
   bt_ull_streaming_if_info_t *p_streaming = NULL;
   bt_ull_context_t *ctx = bt_ull_get_context();
   p_streaming = streaming_interface[transmitter_type];

    if(BT_ULL_ROLE_SERVER == ctx->ull_role) {
       if (BT_ULL_MIC_TRANSMITTER >= transmitter_type) {
         return p_streaming->sample_size;
       }
    }
    ull_report("[ULL] bt_ull_srv_get_usb_sample_size, transmitter %d not suppotrt", 1, transmitter_type);
    return 0;
}
uint32_t bt_ull_get_usb_sample_channel(bt_ull_transmitter_t transmitter_type)
{
   bt_ull_streaming_if_info_t *p_streaming = NULL;
   bt_ull_context_t *ctx = bt_ull_get_context();
   p_streaming = streaming_interface[transmitter_type];

   if(BT_ULL_ROLE_SERVER == ctx->ull_role) {
       if (BT_ULL_MIC_TRANSMITTER >= transmitter_type) {
            return p_streaming->sample_channel;
       }
   }
   ull_report("[ULL] bt_ull_srv_get_usb_sample_channel, transmitter %d not suppotrt", 1, transmitter_type);
   return 0;
}



/* right now, we always only support OPUS codec */
void bt_ull_init_transimter(bt_ull_codec_t codec, bt_ull_transmitter_t transmitter)
{
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    bt_ull_context_t *ctx = bt_ull_get_context();
    audio_transmitter_config_t config;

    memset(&config, 0x00, sizeof(config));
    config.scenario_type = AUDIO_TRANSMITTER_GAMING_MODE;
    uint8_t mic_block_size = bt_ull_get_mic_frame_size();

    if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
        if (BT_ULL_OPUS_CODEC == codec) {
            if (BT_ULL_MIC_TRANSMITTER == transmitter) {
            /* Constuct handle with resource type and handle name */
            if (ctx->ul_microphone.resource_handle == NULL) {
                ctx->ul_microphone.resource_handle = audio_src_srv_resource_manager_construct_handle(AUDIO_SRC_SRV_RESOURCE_TYPE_MIC,AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_ULL_UL);
                if(ctx->ul_microphone.resource_handle != NULL) {
                    ctx->ul_microphone.resource_handle->state = AUDIO_SRC_SRV_EVENT_NONE;
                    ctx->ul_microphone.resource_handle->callback_func = bt_ull_mic_resource_callback;
                    ctx->ul_microphone.resource_handle->priority = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_ULL_UL_PRIORITY;
                    ull_report("[ULL] mic_resource_handle client resource_handle: 0x%04x, priority:0x%d ", 2, ctx->ul_microphone.resource_handle,ctx->ul_microphone.resource_handle->priority);
                } else {
                    bt_utils_assert(0 && "[ULL]fail to get mic_resource handle!!!");
                }
            }
            voice_headset_config_t *voice_param = &config.scenario_config.gaming_mode_config.voice_headset_config;
            config.scenario_sub_id = AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET;
            voice_param->codec_type = AUDIO_DSP_CODEC_TYPE_OPUS;
            voice_param->codec_param.opus.sample_rate = 0x08;   /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
            if (A_MIC_FRAME_SIZE == mic_block_size) {
                /* default is 30B  or default is 31B*/
                voice_param->codec_param.opus.bit_rate = 32000;
            } else if(B_MIC_FRAME_SIZE == mic_block_size){
                /* default is 47B  or default is 48B*/
                voice_param->codec_param.opus.bit_rate = 50133;
            }
            voice_param->codec_param.opus.channel_mode = 0x08;  /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
            config.msg_handler = bt_ull_audio_transmitter_voice_callback;
            config.user_data = NULL;
            if (AUD_ID_INVALID == ctx->ul_microphone.transmitter) {
                ctx->ul_microphone.transmitter = audio_transmitter_init(&config);
            }
            }
        }
    } else if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
        if (BT_ULL_OPUS_CODEC == codec) {
            if (BT_ULL_GAMING_TRANSMITTER == transmitter) {
                /* gaming streaming path */
                music_dongle_config_t *gaming_param = &config.scenario_config.gaming_mode_config.music_dongle_config;
                config.scenario_sub_id = AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1;    /* gaming */

                gaming_param->codec_param.opus.sample_rate = 0x01;   /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
                gaming_param->codec_param.opus.bit_rate = 0x02;      /**< b0: 16kbps, b1:32kbps, b2:64kbps. */
                gaming_param->codec_param.opus.channel_mode = 0x01;  /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */

                ull_report("[ULL] ull server init gaming transmitter: 0x%x, sample_rate:0x%x, volume left: 0x%x, right: 0x%x", 4, ctx->dl_speaker.transmitter, ctx->dl_speaker.sample_rate,
                           ctx->dl_speaker.volume.vol_left, ctx->dl_speaker.volume.vol_right);

                gaming_param->usb_param.pcm.sample_rate = bt_ull_sample_rate_exchange(bt_ull_get_usb_sample_rate(transmitter));
                gaming_param->usb_param.pcm.format = bt_ull_sample_size_exchange(bt_ull_get_usb_sample_size(transmitter));
                gaming_param->usb_param.pcm.channel_mode = bt_ull_get_usb_sample_channel(transmitter);
                ull_report("[ULL] gaming usb_param: sample_rate 0x%x, format:0x%x, channel_mode: 0x%x", 3, gaming_param->usb_param.pcm.sample_rate, gaming_param->usb_param.pcm.format,
                           gaming_param->usb_param.pcm.channel_mode);

                config.msg_handler = bt_ull_audio_transmitter_music_callback;
                config.user_data = NULL;
                if (AUD_ID_INVALID == ctx->dl_speaker.transmitter) {
                    ctx->dl_speaker.transmitter = audio_transmitter_init(&config);
                }
            } else if (BT_ULL_CHAT_TRANSMITTER == transmitter) {
                /* chat streaming path */
                music_dongle_config_t *chat_param = &config.scenario_config.gaming_mode_config.music_dongle_config;
                config.scenario_sub_id = AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0;    /* chat */
                chat_param->codec_param.opus.sample_rate = 0x01;  /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
                chat_param->codec_param.opus.bit_rate = 0x02;  /**< b0: 16kbps, b1:32kbps, b2:64kbps. */
                chat_param->codec_param.opus.channel_mode = 0x01;  /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
                ull_report("[ULL] ull server init chat transmitter: 0x%x, sample_rate:0x%x, volume left: 0x%x, right: 0x%x", 4, ctx->dl_chat.transmitter, ctx->dl_chat.sample_rate,
                           ctx->dl_chat.volume.vol_left, ctx->dl_chat.volume.vol_right);
                chat_param->usb_param.pcm.sample_rate = bt_ull_sample_rate_exchange(bt_ull_get_usb_sample_rate(transmitter));
                chat_param->usb_param.pcm.format = bt_ull_sample_size_exchange(bt_ull_get_usb_sample_size(transmitter));
                chat_param->usb_param.pcm.channel_mode = bt_ull_get_usb_sample_channel(transmitter);
                ull_report("[ULL] chat usb_param: sample_rate 0x%x, format:0x%x, channel_mode: 0x%x", 3, chat_param->usb_param.pcm.sample_rate, chat_param->usb_param.pcm.format,
                           chat_param->usb_param.pcm.channel_mode);
                config.msg_handler = bt_ull_audio_transmitter_chat_callback;
                config.user_data = NULL;
                if (AUD_ID_INVALID == ctx->dl_chat.transmitter) {
                    ctx->dl_chat.transmitter = audio_transmitter_init(&config);
                }
            } else if (BT_ULL_MIC_TRANSMITTER == transmitter) {
                /* Constuct handle with resource type and handle name */
                if (ctx->ul_microphone.resource_handle == NULL) {
                    ctx->ul_microphone.resource_handle = audio_src_srv_resource_manager_construct_handle(AUDIO_SRC_SRV_RESOURCE_TYPE_MIC, AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_ULL_UL);
                    if (ctx->ul_microphone.resource_handle != NULL) {
                        ctx->ul_microphone.resource_handle->state = AUDIO_SRC_SRV_EVENT_NONE;
                        ctx->ul_microphone.resource_handle->callback_func = bt_ull_mic_resource_callback;
                        ctx->ul_microphone.resource_handle->priority = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_ULL_UL_PRIORITY;
                        ull_report("[ULL] mic_resource_handle server resource_handle: 0x%04x, priority:0x%d ", 2, ctx->ul_microphone.resource_handle,ctx->ul_microphone.resource_handle->priority);
                    } else {
                        bt_utils_assert(0 && "[ULL]fail to get mic_resource handle!!!");
                    }
                }
                voice_dongle_config_t *voice_param = &config.scenario_config.gaming_mode_config.voice_dongle_config;
                /* voice path */
                config.scenario_sub_id = AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT;    /* mic */
                voice_param->codec_type = AUDIO_DSP_CODEC_TYPE_OPUS;
                voice_param->codec_param.opus.sample_rate = 0x08;    /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
                if (A_MIC_FRAME_SIZE == mic_block_size) {
                    /* default is 30B  or  default is 31B */
                    voice_param->codec_param.opus.bit_rate = 32000;
                } else if(B_MIC_FRAME_SIZE == mic_block_size){
                    /* default is 47B  or  default is 48B */
                    voice_param->codec_param.opus.bit_rate = 50133;
                }
                voice_param->codec_param.opus.channel_mode = 0x08;   /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
                /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
                if (ctx->ul_microphone.sample_rate & 0x01) {
                    voice_param->usb_param.pcm.sample_rate = 48000;
                } else if (ctx->ul_microphone.sample_rate & 0x08) {
                    voice_param->usb_param.pcm.sample_rate = 16000;
                } else {
                    bt_utils_assert(0 && "unknown sample rate!");
                }
            config.msg_handler = bt_ull_audio_transmitter_voice_callback;
            config.user_data = NULL;
            if (AUD_ID_INVALID == ctx->ul_microphone.transmitter) {
                ctx->ul_microphone.transmitter = audio_transmitter_init(&config);
             }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
            } else if (BT_ULL_LINE_IN_TRANSMITTER == transmitter) {
            /* line_in streaming path */
            linein_dongle_config_t *linein_param = &config.scenario_config.gaming_mode_config.linein_dongle_config;

            config.scenario_sub_id = AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN;    /* Line-In */
            linein_param->codec_param.opus.sample_rate = 0x01;   /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
            linein_param->codec_param.opus.bit_rate = 0x02;      /**< b0: 16kbps, b1:32kbps, b2:64kbps. */
            linein_param->codec_param.opus.channel_mode = 0x01;  /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
            config.msg_handler = bt_ull_audio_transmitter_line_in_callback;
            config.user_data = NULL;
            if (AUD_ID_INVALID == ctx->dl_linein.transmitter) {
                ctx->dl_linein.transmitter = audio_transmitter_init(&config);
                ull_report("line_in transmitter id: %d",1,ctx->dl_linein.transmitter);
              }
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
            } else if (BT_ULL_LINE_OUT_TRANSMITTER == transmitter) {
            /* line_out streaming path */
            lineout_dongle_config_t *lineout_param = &config.scenario_config.gaming_mode_config.lineout_dongle_config;

            config.scenario_sub_id = AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_LINE_OUT;    /* Line-Out */
            lineout_param->codec_type = AUDIO_DSP_CODEC_TYPE_OPUS;
            lineout_param->codec_param.opus.sample_rate = 0x08;    /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
            if (30 == mic_block_size) {
                lineout_param->codec_param.opus.bit_rate = 32000;
            } else {
                /* default is 47B */
                lineout_param->codec_param.opus.bit_rate = 50133;
            }
            lineout_param->codec_param.opus.channel_mode = 0x08;   /**< b0: joint stereo, b1: stereo, b2: dual channel, b3: mono. */
            /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
            if (ctx->ul_lineout.sample_rate & 0x01) {
                lineout_param->usb_param.pcm.sample_rate = 48000;
            } else if (ctx->ul_lineout.sample_rate & 0x08) {
                lineout_param->usb_param.pcm.sample_rate = 16000;
            } else {
                bt_utils_assert(0 && "unknown sample rate!");
            }
            config.msg_handler = bt_ull_audio_transmitter_line_out_callback;
            config.user_data = NULL;
            if (AUD_ID_INVALID == ctx->ul_lineout.transmitter) {
                ctx->ul_lineout.transmitter = audio_transmitter_init(&config);
             }
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
             } else if (BT_ULL_I2S_IN_TRANSMITTER == transmitter) {
             /* line_i2s streaming path */
             i2s_in_dongle_config_t *i2s_param = &config.scenario_config.gaming_mode_config.i2s_in_dongle_config;
             i2s_in_dongle_config_t *config_param = &ctx->linei2s_param;
             config.scenario_sub_id = AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_I2S_IN;    /* Line-i2s */
             memcpy(i2s_param,config_param,sizeof(i2s_in_dongle_config_t));
             config.msg_handler = bt_ull_audio_transmitter_i2s_in_callback;
             config.user_data = NULL;
             if (AUD_ID_INVALID == ctx->dl_linei2s.transmitter) {
                ctx->dl_linei2s.transmitter = audio_transmitter_init(&config);
             }
             ull_report("[ULL] ull server init line_i2s transmitter: %d, audio_device: %d, audio_interface: %d, sample_rate: %d, i2s_fromat: %d, i2s_word_length: %d", 6,
                ctx->dl_linei2s.transmitter, i2s_param->audio_device, i2s_param->audio_interface, i2s_param->codec_param.pcm.sample_rate, i2s_param->i2s_fromat, i2s_param->i2s_word_length);
#endif
           }
        }
    }
#endif
}


void bt_ull_dvfs_lock(dvfs_frequency_t feq)
{
    ull_report("[ULL][COMMON] bt_ull_dvfs_lock: 0x%x", 1, feq);
#ifdef HAL_DVFS_416M_SOURCE
    hal_dvfs_lock_control(feq, HAL_DVFS_LOCK);
#elif defined(HAL_DVFS_312M_SOURCE)
    hal_dvfs_lock_control(DVFS_156M_SPEED, HAL_DVFS_LOCK);
#elif defined(AIR_BTA_IC_PREMIUM_G3)
    hal_dvfs_lock_control(feq, HAL_DVFS_LOCK);
#endif
}

void bt_ull_dvfs_unlock(dvfs_frequency_t feq)
{
    ull_report("[ULL][COMMON] bt_ull_dvfs_unlock: 0x%x", 1, feq);
#ifdef HAL_DVFS_416M_SOURCE
    hal_dvfs_lock_control(feq, HAL_DVFS_UNLOCK);
#elif defined(HAL_DVFS_312M_SOURCE)
    hal_dvfs_lock_control(DVFS_156M_SPEED, HAL_DVFS_UNLOCK);
#elif defined(AIR_BTA_IC_PREMIUM_G3)
    hal_dvfs_lock_control(feq, HAL_DVFS_UNLOCK);
#endif
}

void *bt_ull_memory_alloc(uint16_t size)
{
#ifdef WIN32
    void *memory = (void *)malloc(size);
#else
    void *memory = (void *)pvPortMalloc(size);
#endif /* WIN32 */
    if (NULL != memory) {
        memset(memory, 0, size);
    }
    return memory;
}

void bt_ull_memory_free(void *point)
{
    if (point) {
#ifdef WIN32
        free(point);
#else
        vPortFree(point);
#endif /* WIN32 */
    }
}


#ifdef AIR_AUDIO_TRANSMITTER_ENABLE

void bt_ull_mix_streaming_handle(bt_ull_streaming_if_info_t *master_streaming, bt_ull_streaming_if_info_t *sub_streaming)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_streaming_t streaming = {0};
    if (sub_streaming->is_streaming
        && AUD_ID_INVALID != sub_streaming->transmitter
        && sub_streaming->is_transmitter_start
        && !(sub_streaming->streaming_flag & BT_ULL_STREAMING_STOPPING)) {
        /*line_in && i2s_in is active, because streaming start default ratio is 0, we default mix both to simplify flow */
        audio_transmitter_runtime_config_type_t vol_op = GAMING_MODE_CONFIG_OP_MUSIC_MIX;
        audio_transmitter_runtime_config_t config;
        config.gaming_mode_runtime_config.dl_mixer_id = sub_streaming->transmitter;
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(master_streaming->transmitter, vol_op, &config)) {
            ull_report_error("[ULL][API] 2-rx mixing fail, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                master_streaming->transmitter, config.gaming_mode_runtime_config.dl_mixer_id);
        } else {
            BT_ULL_SET_FLAG(master_streaming->streaming_flag, BT_ULL_STREAMING_MIXED);
            BT_ULL_SET_FLAG(sub_streaming->streaming_flag, BT_ULL_STREAMING_MIXED);
            /* transmitter mix ok, according to ratio set streaming volume */
            if(sub_streaming == &ctx->dl_speaker) {
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                streaming.port = 0; /* gaming streaming port */
            } else if(sub_streaming == &ctx->dl_chat) {
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                streaming.port = 1; /* chat streaming port */
            }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
            else if (sub_streaming == &ctx->dl_linein) {
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
                streaming.port = 1; /* line-in streaming port */
            }
#endif 

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
            else if (sub_streaming == &ctx->dl_linei2s) {
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
                streaming.port = 1; /* i2s-in streaming port */
            }
#endif 
            else {
                bt_utils_assert(0 && "unknown streaming param!");
            }
            bt_ull_set_transmitter_volume(&streaming);
        }
    }
}

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE 
void bt_ull_audio_transmitter_i2s_in_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION != event) {
        ull_report("[ULL] audio_transmitter_linei2s_callback event = 0x%x", 1, event);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            ull_report("[ULL] audio_transmitter_linei2s_callback start,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                ctx->dl_linei2s.is_request_transmitter_start, ctx->dl_linei2s.streaming_flag);
            ctx->dl_linei2s.is_transmitter_start = true;
            BT_ULL_REMOVE_FLAG(ctx->dl_linei2s.streaming_flag, BT_ULL_STREAMING_STOPPING);
            BT_ULL_REMOVE_FLAG(ctx->dl_linei2s.streaming_flag, BT_ULL_STREAMING_STARTING);

            /* usb sample rate, should reinit transmitter */
            if (ctx->dl_linei2s.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                /* 1. stop transmitter -> deinit transmitter -> reinit transmitter */
                bt_ull_stop_transmitter(&ctx->dl_linei2s);
            } else {
                if (false == ctx->dl_linei2s.is_request_transmitter_start) {
                    bt_ull_stop_transmitter(&ctx->dl_linei2s);
            } else {
                if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                    bt_ull_mix_streaming_handle(&ctx->dl_linei2s,&ctx->dl_linein);
#endif
                    bt_ull_mix_streaming_handle(&ctx->dl_linei2s,&ctx->dl_chat);
                    bt_ull_mix_streaming_handle(&ctx->dl_linei2s,&ctx->dl_speaker);

                    /*line_i2s mix itself*/
                    audio_transmitter_runtime_config_type_t vol_op = GAMING_MODE_CONFIG_OP_MUSIC_MIX;
                    audio_transmitter_runtime_config_t config;
                    config.gaming_mode_runtime_config.dl_mixer_id = ctx->dl_linei2s.transmitter;
                    if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ctx->dl_linei2s.transmitter, vol_op, &config)) {
                            ull_report("[ULL][API] line_in mixing itself fail, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                                ctx->dl_linei2s.transmitter, config.gaming_mode_runtime_config.dl_mixer_id);
                        } else {
                           ull_report("[ULL][API] line_in mixing itself success, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                                ctx->dl_linei2s.transmitter, config.gaming_mode_runtime_config.dl_mixer_id);
                    }
                    /* set line_in transmitter volume */
                    bt_ull_streaming_t streaming;
                    streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
                    streaming.port = 0; /* line_in streaming port */
                    bt_ull_init_transmitter_volume(&streaming, AUD_VOL_OUT_LEVEL15);
                    bt_ull_set_transmitter_volume(&streaming);
                }
            }
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL:
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            ull_report("[ULL] audio_transmitter_linei2s_callback stop,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                ctx->dl_linei2s.is_request_transmitter_start, ctx->dl_linei2s.streaming_flag);
            ctx->dl_linei2s.is_transmitter_start = false;
            BT_ULL_REMOVE_FLAG(ctx->dl_linei2s.streaming_flag, BT_ULL_STREAMING_STOPPING);
            BT_ULL_REMOVE_FLAG(ctx->dl_linei2s.streaming_flag, BT_ULL_STREAMING_STARTING);
            /* transmitter stop, remove mix flag */
            BT_ULL_REMOVE_FLAG(ctx->dl_linei2s.streaming_flag, BT_ULL_STREAMING_MIXED);

            /* usb sample rate, should reinit transmitter */
            if (ctx->dl_linei2s.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                BT_ULL_REMOVE_FLAG(ctx->dl_linei2s.streaming_flag, BT_ULL_STREAMING_RECONFIG);
                /* 2. deinit transmitter */
                audio_transmitter_status_t status = audio_transmitter_deinit(ctx->dl_linei2s.transmitter);
                ull_report("[ULL] dl_linein transmitter deinit :0x%x", 1, status);
                /* 3. reinit transmitter with new sample rate */
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                    ctx->dl_linei2s.transmitter = AUD_ID_INVALID;
                    bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_I2S_IN_TRANSMITTER);
                }
            }
            if (ctx->dl_linei2s.is_request_transmitter_start) {
                /* 2-rx, we should wait for all transmitter stop, then start again */
                ull_report("[ULL] chat streaming flag = 0x%x, is_start = 0x%x", 2, ctx->dl_chat.streaming_flag, ctx->dl_chat.is_transmitter_start);
                if (AUD_ID_INVALID != ctx->dl_chat.transmitter
                    && ctx->dl_chat.is_transmitter_start
                    && (ctx->dl_chat.streaming_flag & BT_ULL_STREAMING_STOPPING)) {
                    ull_report("[ULL] chat streaming is stopping, we need wait it stop.", 0);
                    break;
                }
                ull_report("[ULL] gaming streaming flag = 0x%x, is_start = 0x%x", 2, ctx->dl_speaker.streaming_flag, ctx->dl_speaker.is_transmitter_start);
                if (AUD_ID_INVALID != ctx->dl_speaker.transmitter
                    && ctx->dl_speaker.is_transmitter_start
                    && (ctx->dl_speaker.streaming_flag & BT_ULL_STREAMING_STOPPING)) {
                    ull_report("[ULL] gaming streaming is stopping, we need wait it stop.", 0);
                    break;
                }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                ull_report("[ULL] line_in streaming flag = 0x%x, is_start = 0x%x", 2, ctx->dl_linein.streaming_flag, ctx->dl_linein.is_transmitter_start);
                if (AUD_ID_INVALID != ctx->dl_linein.transmitter
                    && ctx->dl_linein.is_transmitter_start
                    && (ctx->dl_linein.streaming_flag & BT_ULL_STREAMING_STOPPING)) {
                    ull_report("[ULL] line_in streaming is stopping, we need wait it stop.", 0);
                    break;
                }
#endif
                if (ctx->dl_speaker.is_request_transmitter_start) {
                    bt_ull_start_transmitter(&ctx->dl_speaker);
                }
                if (ctx->dl_chat.is_request_transmitter_start) {
                    bt_ull_start_transmitter(&ctx->dl_chat);
                }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                if (ctx->dl_linein.is_request_transmitter_start) {
                    bt_ull_start_transmitter(&ctx->dl_linein);
                }
#endif
                bt_ull_start_transmitter(&ctx->dl_linei2s);
                /* fdcb notify controller enable, controller will be clear music downlink avm buffer */
                uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
                bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, true, ctx->dl_latency);
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION: {
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
}

#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
void bt_ull_audio_transmitter_line_out_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION != event) {
        ull_report("[ULL] audio_transmitter_lineout_callback event = 0x%x", 1, event);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            ull_report("[ULL] audio_transmitter_lineout_callback start,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                ctx->ul_lineout.is_request_transmitter_start, ctx->ul_lineout.streaming_flag);
            ctx->ul_lineout.is_transmitter_start = true;
            BT_ULL_REMOVE_FLAG(ctx->ul_lineout.streaming_flag, BT_ULL_STREAMING_STARTING);
            BT_ULL_REMOVE_FLAG(ctx->ul_lineout.streaming_flag, BT_ULL_STREAMING_STOPPING);

            /* usb sample rate, should reinit transmitter */
            if (ctx->ul_lineout.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                /* 1. stop transmitter -> deinit transmitter -> reinit transmitter */
                bt_ull_stop_transmitter(&ctx->ul_lineout);
            } else {
                if (false == ctx->ul_lineout.is_request_transmitter_start) {
                    bt_ull_stop_transmitter(&ctx->ul_lineout);
            } else {
                if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
                    /* set server microphone uplink latency when voice transmitter start */
                    audio_transmitter_runtime_config_t config;
                    config.gaming_mode_runtime_config.latency_us = ctx->ul_latency * 1000;
                    ull_report("[ULL] voice_transmitter started,  set uplink latency:%d", 1, ctx->ul_latency);
                    if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ctx->ul_lineout.transmitter, GAMING_MODE_CONFIG_OP_LATENCY_SWITCH, &config)) {
                        ull_report_error("[ULL][API] audio_transmitter_set_runtime_config fail, trans_id:0x%x, latency: %d", 2,
                            ctx->ul_lineout.transmitter, config.gaming_mode_runtime_config.latency_us);
                    }
                    bt_ull_streaming_t streaming;
                    streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
                    streaming.port = 0;
                    bt_ull_init_transmitter_volume(&streaming, bt_sink_srv_ami_get_usb_voice_sw_default_volume_level());
                    bt_ull_set_transmitter_volume(&streaming);
                } else {
                    am_audio_side_tone_enable();
                }
            }
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL:
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            ull_report("[ULL] audio_transmitter_lineout_callback stop,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                ctx->ul_lineout.is_request_transmitter_start, ctx->ul_lineout.streaming_flag);
            ctx->ul_lineout.is_transmitter_start = false;
            BT_ULL_REMOVE_FLAG(ctx->ul_lineout.streaming_flag, BT_ULL_STREAMING_STARTING);
            BT_ULL_REMOVE_FLAG(ctx->ul_lineout.streaming_flag, BT_ULL_STREAMING_STOPPING);

            /* usb sample rate, should reinit transmitter */
            if (ctx->ul_lineout.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                BT_ULL_REMOVE_FLAG(ctx->ul_lineout.streaming_flag, BT_ULL_STREAMING_RECONFIG);
                /* 2. deinit transmitter */
                audio_transmitter_status_t status = audio_transmitter_deinit(ctx->ul_lineout.transmitter);
                ull_report("[ULL] lineout transmitter deinit :0x%x", 1, status);
                /* 3. reinit transmitter with new sample rate */
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                    ctx->ul_lineout.transmitter = AUD_ID_INVALID;
                    bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_LINE_OUT_TRANSMITTER);
                }
            }
            /* check last user request restart transmitter or not */
            if (ctx->ul_lineout.is_request_transmitter_start) {
                bt_ull_start_transmitter(&ctx->ul_lineout);
            } else {
               bt_ull_stop_transmitter(&ctx->ul_lineout);
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION: {
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
}
#endif


#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
void bt_ull_audio_transmitter_line_in_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION != event) {
        ull_report("[ULL] audio_transmitter_linein_callback event = 0x%x", 1, event);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            ull_report("[ULL] audio_transmitter_linein_callback start,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                ctx->dl_linein.is_request_transmitter_start, ctx->dl_linein.streaming_flag);
            ctx->dl_linein.is_transmitter_start = true;
            BT_ULL_REMOVE_FLAG(ctx->dl_linein.streaming_flag, BT_ULL_STREAMING_STOPPING);
            BT_ULL_REMOVE_FLAG(ctx->dl_linein.streaming_flag, BT_ULL_STREAMING_STARTING);

            /* usb sample rate, should reinit transmitter */
            if (ctx->dl_linein.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                /* 1. stop transmitter -> deinit transmitter -> reinit transmitter */
                bt_ull_stop_transmitter(&ctx->dl_linein);
            } else {
                if (false == ctx->dl_linein.is_request_transmitter_start) {
                    bt_ull_stop_transmitter(&ctx->dl_linein);
            } else {
                if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
                    bt_ull_mix_streaming_handle(&ctx->dl_linein,&ctx->dl_chat);
                    bt_ull_mix_streaming_handle(&ctx->dl_linein,&ctx->dl_speaker);
                    /*line_in mix itself*/
                    audio_transmitter_runtime_config_type_t vol_op = GAMING_MODE_CONFIG_OP_MUSIC_MIX;
                    audio_transmitter_runtime_config_t config;
                    config.gaming_mode_runtime_config.dl_mixer_id = ctx->dl_linein.transmitter;
                    if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ctx->dl_linein.transmitter, vol_op, &config)) {
                            ull_report("[ULL][API] line_in mixing itself fail, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                                ctx->ul_microphone.transmitter, config.gaming_mode_runtime_config.dl_mixer_id);
                        } else {
                           ull_report("[ULL][API] line_in mixing itself success, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                                ctx->ul_microphone.transmitter, config.gaming_mode_runtime_config.dl_mixer_id);
                    }
                    /* set line_in transmitter volume */
                    bt_ull_streaming_t streaming;
                    streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
                    streaming.port = 0; /* line_in streaming port */
                    bt_ull_init_transmitter_volume(&streaming, AUD_VOL_OUT_LEVEL15);
                    bt_ull_set_transmitter_volume(&streaming);
                }
            }
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL:
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            ull_report("[ULL] audio_transmitter_linein_callback stop,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                ctx->dl_linein.is_request_transmitter_start, ctx->dl_linein.streaming_flag);
            ctx->dl_linein.is_transmitter_start = false;
            BT_ULL_REMOVE_FLAG(ctx->dl_linein.streaming_flag, BT_ULL_STREAMING_STOPPING);
            BT_ULL_REMOVE_FLAG(ctx->dl_linein.streaming_flag, BT_ULL_STREAMING_STARTING);
            /* transmitter stop, remove mix flag */
            BT_ULL_REMOVE_FLAG(ctx->dl_linein.streaming_flag, BT_ULL_STREAMING_MIXED);

            /* usb sample rate, should reinit transmitter */
            if (ctx->dl_linein.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                BT_ULL_REMOVE_FLAG(ctx->dl_linein.streaming_flag, BT_ULL_STREAMING_RECONFIG);
                /* 2. deinit transmitter */
                audio_transmitter_status_t status = audio_transmitter_deinit(ctx->dl_linein.transmitter);
                ull_report("[ULL] dl_linein transmitter deinit :0x%x", 1, status);
                /* 3. reinit transmitter with new sample rate */
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                    ctx->dl_linein.transmitter = AUD_ID_INVALID;
                    bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_LINE_IN_TRANSMITTER);
                }
            }
            if (ctx->dl_linein.is_request_transmitter_start) {
                /* 2-rx, we should wait for all transmitter stop, then start again */
                ull_report("[ULL] chat streaming flag = 0x%x, is_start = 0x%x", 2, ctx->dl_chat.streaming_flag, ctx->dl_chat.is_transmitter_start);
                if (AUD_ID_INVALID != ctx->dl_chat.transmitter
                    && ctx->dl_chat.is_transmitter_start
                    && (ctx->dl_chat.streaming_flag & BT_ULL_STREAMING_STOPPING)) {
                    ull_report("[ULL] chat streaming is stopping, we need wait it stop.", 0);
                    break;
                }
                ull_report("[ULL] gaming streaming flag = 0x%x, is_start = 0x%x", 2, ctx->dl_speaker.streaming_flag, ctx->dl_speaker.is_transmitter_start);
                if (AUD_ID_INVALID != ctx->dl_speaker.transmitter
                    && ctx->dl_speaker.is_transmitter_start
                    && (ctx->dl_speaker.streaming_flag & BT_ULL_STREAMING_STOPPING)) {
                    ull_report("[ULL] gaming streaming is stopping, we need wait it stop.", 0);
                    break;
                }
                if (ctx->dl_speaker.is_request_transmitter_start) {
                    bt_ull_start_transmitter(&ctx->dl_speaker);
                }
                if (ctx->dl_chat.is_request_transmitter_start) {
                    bt_ull_start_transmitter(&ctx->dl_chat);
                }
                bt_ull_start_transmitter(&ctx->dl_linein);
                /* fdcb notify controller enable, controller will be clear music downlink avm buffer */
                uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
                bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, true, ctx->dl_latency);
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION: {
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
}
#endif

void bt_ull_audio_transmitter_voice_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION != event) {
        ull_report("[ULL] audio_transmitter_voice_callback event = 0x%x", 1, event);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            ull_report("[ULL] audio_transmitter_voice_callback start,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                       ctx->ul_microphone.is_request_transmitter_start, ctx->ul_microphone.streaming_flag);
            ctx->ul_microphone.is_transmitter_start = true;
            BT_ULL_REMOVE_FLAG(ctx->ul_microphone.streaming_flag, BT_ULL_STREAMING_STARTING);
            BT_ULL_REMOVE_FLAG(ctx->ul_microphone.streaming_flag, BT_ULL_STREAMING_STOPPING);
            bt_ull_event_callback(BT_ULL_EVENT_UPLINK_START_SUCCESS, NULL, 0);

            /* usb sample rate, should reinit transmitter */
            if (ctx->ul_microphone.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                /* 1. stop transmitter -> deinit transmitter -> reinit transmitter */
                bt_ull_stop_transmitter(&ctx->ul_microphone);
            } else {
                if (false == ctx->ul_microphone.is_request_transmitter_start) {
                    bt_ull_stop_transmitter(&ctx->ul_microphone);
                } else {
                    if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
                        /* set server microphone uplink latency when voice transmitter start */
                        audio_transmitter_runtime_config_t config;
                        config.gaming_mode_runtime_config.latency_us = ctx->ul_latency * 1000;
                        ull_report("[ULL] voice_transmitter started,  set uplink latency:%d", 1, ctx->ul_latency);
                        if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ctx->ul_microphone.transmitter, GAMING_MODE_CONFIG_OP_LATENCY_SWITCH, &config)) {
                            ull_report_error("[ULL][API] audio_transmitter_set_runtime_config fail, trans_id:0x%x, latency: %d", 2,
                                             ctx->ul_microphone.transmitter, config.gaming_mode_runtime_config.latency_us);
                        }
                        bt_ull_streaming_t streaming;
                        streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                        streaming.port = 0;
                        bt_ull_init_transmitter_volume(&streaming, bt_sink_srv_ami_get_usb_voice_sw_default_volume_level());
                        bt_ull_set_transmitter_volume(&streaming);
                    } else {
                        am_audio_side_tone_enable();
                    }
                }
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL:
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            ull_report("[ULL] audio_transmitter_voice_callback stop,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                       ctx->ul_microphone.is_request_transmitter_start, ctx->ul_microphone.streaming_flag);
            ctx->ul_microphone.is_transmitter_start = false;
            BT_ULL_REMOVE_FLAG(ctx->ul_microphone.streaming_flag, BT_ULL_STREAMING_STARTING);
            BT_ULL_REMOVE_FLAG(ctx->ul_microphone.streaming_flag, BT_ULL_STREAMING_STOPPING);
            bt_ull_event_callback(BT_ULL_EVENT_UPLINK_STOP_SUCCESS, NULL, 0);

            /* usb sample rate, should reinit transmitter */
            if (ctx->ul_microphone.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                BT_ULL_REMOVE_FLAG(ctx->ul_microphone.streaming_flag, BT_ULL_STREAMING_RECONFIG);
                /* 2. deinit transmitter */
                audio_transmitter_status_t status = audio_transmitter_deinit(ctx->ul_microphone.transmitter);
                ull_report("[ULL] voice transmitter deinit :0x%x", 1, status);
                /* 3. reinit transmitter with new sample rate */
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                    ctx->ul_microphone.transmitter = AUD_ID_INVALID;
                    bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_MIC_TRANSMITTER);
                    ctx->ul_microphone.is_request_transmitter_start = true;
                }
            }
            /* check last user request restart transmitter or not */
            if (ctx->ul_microphone.is_request_transmitter_start) {
                bt_ull_start_transmitter(&ctx->ul_microphone);
            } else {
                if (ctx->ul_microphone.resource_handle != NULL) {
                    if ((AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS == ctx->ul_microphone.resource_handle->state)
                        || (AUDIO_SRC_SRV_EVENT_SUSPEND == ctx->ul_microphone.resource_handle->state)) {
                        audio_src_srv_resource_manager_give(ctx->ul_microphone.resource_handle);
                    }
                    if (ctx->ul_microphone.is_suspend) {
                        ctx->ul_microphone.is_suspend = false;
                        audio_src_srv_resource_manager_add_waiting_list(ctx->ul_microphone.resource_handle);
                    }
                }
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION: {
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
}

void bt_ull_audio_transmitter_music_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION != event) {
        ull_report("[ULL] audio_transmitter_music_callback event = 0x%x", 1, event);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            ull_report("[ULL] audio_transmitter_music_callback start,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                       ctx->dl_speaker.is_request_transmitter_start, ctx->dl_speaker.streaming_flag);
            ctx->dl_speaker.is_transmitter_start = true;
            BT_ULL_REMOVE_FLAG(ctx->dl_speaker.streaming_flag, BT_ULL_STREAMING_STOPPING);
            BT_ULL_REMOVE_FLAG(ctx->dl_speaker.streaming_flag, BT_ULL_STREAMING_STARTING);

            /* usb sample rate, should reinit transmitter */
            if (ctx->dl_speaker.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                /* 1. stop transmitter -> deinit transmitter -> reinit transmitter */
                bt_ull_stop_transmitter(&ctx->dl_speaker);
            } else {
                if (false == ctx->dl_speaker.is_request_transmitter_start) {
                    bt_ull_stop_transmitter(&ctx->dl_speaker);
            } else {
                if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
                    bt_ull_mix_streaming_handle(&ctx->dl_speaker,&ctx->dl_chat);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                    bt_ull_mix_streaming_handle(&ctx->dl_speaker,&ctx->dl_linein);
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                    bt_ull_mix_streaming_handle(&ctx->dl_speaker,&ctx->dl_linei2s);
#endif
                    /*music mix itself*/
                    audio_transmitter_runtime_config_type_t vol_op = GAMING_MODE_CONFIG_OP_MUSIC_MIX;
                    audio_transmitter_runtime_config_t config;
                    config.gaming_mode_runtime_config.dl_mixer_id = ctx->dl_speaker.transmitter;
                    if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ctx->dl_speaker.transmitter, vol_op, &config)) {
                        ull_report("[ULL][API] music mixing self fail, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                        ctx->dl_speaker.transmitter, config.gaming_mode_runtime_config.dl_mixer_id);
                     } else {
                        ull_report("[ULL][API] music mixing self success, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                        ctx->dl_speaker.transmitter, config.gaming_mode_runtime_config.dl_mixer_id);
                     }
                    /* set gaming transmitter volume */
                    bt_ull_streaming_t streaming;
                    streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    streaming.port = 0; /* gaming streaming port */
                    bt_ull_init_transmitter_volume(&streaming, bt_sink_srv_ami_get_usb_music_sw_default_volume_level());
                    bt_ull_set_transmitter_volume(&streaming);
                }
            }
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL:
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            ull_report("[ULL] audio_transmitter_music_callback stop,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                       ctx->dl_speaker.is_request_transmitter_start, ctx->dl_speaker.streaming_flag);
            ctx->dl_speaker.is_transmitter_start = false;
            BT_ULL_REMOVE_FLAG(ctx->dl_speaker.streaming_flag, BT_ULL_STREAMING_STOPPING);
            BT_ULL_REMOVE_FLAG(ctx->dl_speaker.streaming_flag, BT_ULL_STREAMING_STARTING);
            /* transmitter stop, remove mix flag */
            BT_ULL_REMOVE_FLAG(ctx->dl_speaker.streaming_flag, BT_ULL_STREAMING_MIXED);

            /* usb sample rate, should reinit transmitter */
            if (ctx->dl_speaker.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                BT_ULL_REMOVE_FLAG(ctx->dl_speaker.streaming_flag, BT_ULL_STREAMING_RECONFIG);
                /* 2. deinit transmitter */
                audio_transmitter_status_t status = audio_transmitter_deinit(ctx->dl_speaker.transmitter);
                ull_report("[ULL] gaming transmitter deinit :0x%x", 1, status);
                /* 3. reinit transmitter with new sample rate */
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                    ctx->dl_speaker.transmitter = AUD_ID_INVALID;
                    bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_GAMING_TRANSMITTER);
                    ctx->dl_speaker.is_request_transmitter_start = true;
                }
            }
            if (ctx->dl_speaker.is_request_transmitter_start) {
                /* 2-rx, we should wait for all transmitter stop, then start again */
                ull_report("[ULL] chat streaming flag = 0x%x, is_start = 0x%x", 2, ctx->dl_chat.streaming_flag, ctx->dl_chat.is_transmitter_start);
                if (AUD_ID_INVALID != ctx->dl_chat.transmitter
                    && ctx->dl_chat.is_transmitter_start
                    && (ctx->dl_chat.streaming_flag & BT_ULL_STREAMING_STOPPING)) {
                    ull_report("[ULL] chat streaming is stopping, we need wait it stop.", 0);
                    break;
                }
                if (ctx->dl_chat.is_request_transmitter_start) {
                    bt_ull_start_transmitter(&ctx->dl_chat);
                }
                bt_ull_start_transmitter(&ctx->dl_speaker);
                /* fdcb notify controller enable, controller will be clear music downlink avm buffer */
                uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
                bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, true, ctx->dl_latency);
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION: {
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
}

void bt_ull_audio_transmitter_chat_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION != event) {
        ull_report("[ULL] audio_transmitter_chat_callback event = 0x%x", 1, event);
    }
    BT_ULL_MUTEX_LOCK();
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            ull_report("[ULL] audio_transmitter_chat_callback start,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                       ctx->dl_chat.is_request_transmitter_start, ctx->dl_chat.streaming_flag);
            ctx->dl_chat.is_transmitter_start = true;
            BT_ULL_REMOVE_FLAG(ctx->dl_chat.streaming_flag, BT_ULL_STREAMING_STOPPING);
            BT_ULL_REMOVE_FLAG(ctx->dl_chat.streaming_flag, BT_ULL_STREAMING_STARTING);

            /* usb sample rate, should reinit transmitter */
            if (ctx->dl_chat.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                /* 1. stop transmitter -> deinit transmitter -> reinit transmitter */
                bt_ull_stop_transmitter(&ctx->dl_chat);
            } else {
                if (false == ctx->dl_chat.is_request_transmitter_start) {
                    bt_ull_stop_transmitter(&ctx->dl_chat);
            } else {
                if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
                    bt_ull_mix_streaming_handle(&ctx->dl_chat,&ctx->dl_speaker);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                    bt_ull_mix_streaming_handle(&ctx->dl_chat,&ctx->dl_linein);
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                    bt_ull_mix_streaming_handle(&ctx->dl_chat,&ctx->dl_linei2s);
#endif
                    audio_transmitter_runtime_config_type_t vol_op = GAMING_MODE_CONFIG_OP_MUSIC_MIX;
                    audio_transmitter_runtime_config_t config;
                    config.gaming_mode_runtime_config.dl_mixer_id = ctx->dl_chat.transmitter;
                    if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ctx->dl_chat.transmitter, vol_op, &config)) {
                            ull_report("[ULL][API] chat mixing self fail, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                                ctx->dl_chat.transmitter, config.gaming_mode_runtime_config.dl_mixer_id);
                        } else {
                           ull_report("[ULL][API] chat mixing self success, trans_id:0x%x, mix_trans_id: 0x%x", 2,
                                ctx->dl_chat.transmitter, config.gaming_mode_runtime_config.dl_mixer_id);
                    }
                    /* set chat transmitter volume */
                    bt_ull_streaming_t streaming;
                    streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    streaming.port = 1; /* chat streaming port */
                    bt_ull_init_transmitter_volume(&streaming, bt_sink_srv_ami_get_usb_music_sw_default_volume_level());
                    bt_ull_set_transmitter_volume(&streaming);
                }
            }
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL:
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            ull_report("[ULL] audio_transmitter_chat_callback stop,  is_request_transmitter_start = 0x%x, flag:0x%x", 2,
                ctx->dl_chat.is_request_transmitter_start, ctx->dl_chat.streaming_flag);
            ctx->dl_chat.is_transmitter_start = false;
            BT_ULL_REMOVE_FLAG(ctx->dl_chat.streaming_flag, BT_ULL_STREAMING_STOPPING);
            BT_ULL_REMOVE_FLAG(ctx->dl_chat.streaming_flag, BT_ULL_STREAMING_STARTING);
            /* transmitter stop, remove mix flag */
            BT_ULL_REMOVE_FLAG(ctx->dl_chat.streaming_flag, BT_ULL_STREAMING_MIXED);

            /* usb sample rate, should reinit transmitter */
            if (ctx->dl_chat.streaming_flag & BT_ULL_STREAMING_RECONFIG) {
                BT_ULL_REMOVE_FLAG(ctx->dl_chat.streaming_flag, BT_ULL_STREAMING_RECONFIG);
                /* 2. deinit transmitter */
                audio_transmitter_status_t status = audio_transmitter_deinit(ctx->dl_chat.transmitter);
                ull_report("[ULL] chat transmitter deinit :0x%x", 1, status);
                /* 3. reinit transmitter with new sample rate */
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                    ctx->dl_chat.transmitter = AUD_ID_INVALID;
                    bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_CHAT_TRANSMITTER);
                    ctx->dl_chat.is_request_transmitter_start = true;
                }
            }

            if (ctx->dl_chat.is_request_transmitter_start) {
                /* 2-rx, we should wait for all transmitter stop, then start again */
                ull_report("[ULL] gaming streaming flag = 0x%x, is_start = 0x%x", 2, ctx->dl_speaker.streaming_flag, ctx->dl_speaker.is_transmitter_start);
                if (AUD_ID_INVALID != ctx->dl_speaker.transmitter
                    && ctx->dl_speaker.is_transmitter_start
                    && (ctx->dl_speaker.streaming_flag & BT_ULL_STREAMING_STOPPING)) {
                    ull_report("[ULL] gaming streaming is stopping, we need wait it stop.", 0);
                    break;
                }

                if (ctx->dl_speaker.is_request_transmitter_start) {
                    bt_ull_start_transmitter(&ctx->dl_speaker);
                }
                bt_ull_start_transmitter(&ctx->dl_chat);
                /* fdcb notify controller enable, controller will be clear music downlink avm buffer */
                uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
                bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, true, ctx->dl_latency);
            }
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION: {
            break;
        }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
}
#endif

static void bt_ull_init_transmitter_volume(bt_ull_streaming_t *streaming, bt_sink_srv_am_volume_level_t default_vol)
{
    bt_ull_streaming_if_info_t *ep = NULL;
    ep = bt_ull_get_streaming_interface(streaming);

    ull_report("[ULL] init_transmitter_volume, type =0x%x, init before volume left = %d, right = %d, default = %d", 4,
               streaming->streaming_interface, ep->volume.vol_left, ep->volume.vol_right, default_vol);
    if (AUD_VOL_OUT_MAX != ep->volume.vol_right && AUD_VOL_OUT_MAX == ep->volume.vol_left) {
        /* only right channle volume valid, we set both channle same volume */
        ep->volume.vol_left = ep->volume.vol_right;
    } else if (AUD_VOL_OUT_MAX == ep->volume.vol_right && AUD_VOL_OUT_MAX != ep->volume.vol_left) {
        /* only left channle volume valid, we set both channle same volume */
        ep->volume.vol_right = ep->volume.vol_left;
    } else if (AUD_VOL_OUT_MAX == ep->volume.vol_right && AUD_VOL_OUT_MAX == ep->volume.vol_left) {
        /* both L && R channel volume invalid, we use defaul volume */
        ep->volume.vol_left = default_vol;
        ep->volume.vol_right = default_vol;
    } else {
        /* both L && R channel volume valid */
    }
    ull_report("[ULL] init_transmitter_volume, type =0x%x, init after volume left = %d, right = %d, default = %d", 4,
               streaming->streaming_interface, ep->volume.vol_left, ep->volume.vol_right, default_vol);
}

void bt_ull_set_transmitter_volume(bt_ull_streaming_t *streaming)
{
    bt_ull_streaming_if_info_t *ep = NULL;
    audio_transmitter_runtime_config_t config;
    audio_transmitter_runtime_config_type_t vol_op = GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL;
    bt_ull_context_t *ctx = bt_ull_get_context();

    ull_report("[ULL] set transmitter volume, streaming_type = 0x%x, port:0x%x", 2,
        streaming->streaming_interface, streaming->port);
    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface
        && 0x00 == streaming->port) {
        ep = &(ctx->dl_speaker);
    } else if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface
        && 0x01 == streaming->port) {
        ep = &(ctx->dl_chat);
    } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface) {
        ep = &(ctx->ul_microphone);
        vol_op = GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_DUL;
    }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
    else if (BT_ULL_STREAMING_INTERFACE_LINE_IN == streaming->streaming_interface) {
        ep = &(ctx->dl_linein);
    }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
    else if (BT_ULL_STREAMING_INTERFACE_LINE_OUT == streaming->streaming_interface) {
        ep = &(ctx->ul_lineout);
        vol_op = GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_DUL;
     }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
    else if (BT_ULL_STREAMING_INTERFACE_I2S_IN == streaming->streaming_interface) {
        ep = &(ctx->dl_linei2s);
    }
#endif
    else {
        bt_utils_assert(0 && "unknown if_type");
        return;
    }

    uint8_t idx = 0;
    ull_report("[ULL] base volume_left = %d, volume_right = %d", 2, ep->volume.vol_left, ep->volume.vol_right);
    for (idx = 0; idx < BT_ULL_MAX_STREAMING_NUM; idx++) {
        ull_report("[ULL] dl_mix_ratio, streaming[%d] type:0x%x, port:0x%x, ratio: %d", 4,
            idx, ctx->dl_mix_ratio.streamings[idx].streaming.streaming_interface, ctx->dl_mix_ratio.streamings[idx].streaming.port, ctx->dl_mix_ratio.streamings[idx].ratio);
    }

    if (AUD_ID_INVALID != ep->transmitter && ep->is_transmitter_start) {
        config.gaming_mode_runtime_config.vol_level.vol_level_r = ep->volume.vol_right;
        config.gaming_mode_runtime_config.vol_level.vol_level_l = ep->volume.vol_left;
        if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
        || BT_ULL_STREAMING_INTERFACE_LINE_IN == streaming->streaming_interface
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
        || BT_ULL_STREAMING_INTERFACE_I2S_IN == streaming->streaming_interface
#endif
        ){
            uint8_t idx = 0;
            for (idx = 0; idx < BT_ULL_MAX_STREAMING_NUM; idx++) {
                if (!memcmp(streaming, &(ctx->dl_mix_ratio.streamings[idx].streaming), sizeof(bt_ull_streaming_t))) {
                    config.gaming_mode_runtime_config.vol_level.vol_ratio = ctx->dl_mix_ratio.streamings[idx].ratio;
                    ull_report("[ULL] streaming mix ratio: %d", 1, config.gaming_mode_runtime_config.vol_level.vol_ratio);
                    break;
                }
            }
        }
        /* transmitter is mute */
        if (ep->is_mute) {
            ull_report("[ULL] dongle transmitter is mute!", 0);
            config.gaming_mode_runtime_config.vol_level.vol_level_r = 0;
            config.gaming_mode_runtime_config.vol_level.vol_level_l = 0;
        }
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ep->transmitter, vol_op, &config)) {
            ull_report_error("[ULL][API] audio_transmitter_set_runtime_config fail, trans_id:0x%x, vol_left: 0x%x, vol_right: 0x%x", 3,
                             ep->transmitter, config.gaming_mode_runtime_config.vol_level.vol_level_l, config.gaming_mode_runtime_config.vol_level.vol_level_r);
        }
    } else {
        ull_report("[ULL] skip set volume due to streaming is not start!", 0);
    }
}

void bt_ull_clear_spp_cache(void)
{
    memset(&g_ull_tx_cache[0], 0x00, sizeof(bt_ull_req_t)*BT_ULL_TX_MAX_CACHE);
}

void bt_ull_cache_spp_tx(bt_ull_req_t *tx_req)
{
    uint8_t idx = 0;
    for (idx = 0; idx < BT_ULL_TX_MAX_CACHE; idx++) {
        if (BT_ULL_EVENT_UNKNOWN == g_ull_tx_cache[idx].event) {
            memcpy(&g_ull_tx_cache[idx], tx_req, sizeof(bt_ull_req_t));
            ull_report("[ULL] bt_ull_cache_spp_tx, event = 0x%x", 1, tx_req->event);
            break;
        }
    }
}

void bt_ull_send_cache_spp_data(uint32_t handle)
{
    uint8_t idx = 0;
    bt_status_t result;

    for (idx = 0; idx < BT_ULL_TX_MAX_CACHE; idx++) {
        if (BT_ULL_EVENT_UNKNOWN != g_ull_tx_cache[idx].event) {
            ull_report("[ULL] bt_ull_send_cache_spp_data, event = 0x%x", 1, g_ull_tx_cache[idx].event);
            result = bt_ull_send_data(handle, (uint8_t *)&g_ull_tx_cache[idx], sizeof(bt_ull_req_t));
            if (BT_STATUS_SUCCESS == result) {
                g_ull_tx_cache[idx].event = BT_ULL_EVENT_UNKNOWN;
            } else {
                ull_report("[ULL] bt_ull_send_cache_spp_data fail, result = 0x%x", 1, result);
                break;
            }
        }
    }
}


uint32_t bt_ull_sample_rate_exchange(uint8_t sample_bit)
{
    uint32_t sample_rate = 48000;
    /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    if (sample_bit & 0x01) {
        sample_rate = 48000;
    } else if (sample_bit & 0x02) {
        sample_rate = 44100;
    } else if (sample_bit & 0x04) {
        sample_rate = 32000;
    } else if (sample_bit & 0x08) {
        sample_rate = 16000;
    } else if (sample_bit & 0x10) {
        sample_rate = 96000;
    } else {
        ull_report_error("[ULL] bt_ull_sample_rate_exchange, unknown sample rate bit: 0x%x", 1, sample_bit);
    }
    return sample_rate;
}


uint32_t bt_ull_sample_size_exchange(uint8_t sample_size)
{
    uint32_t ret = 0;
    ull_report("[ULL] bt_ull_sample_size_exchange, sample size: 0x%x", 1, sample_size);
    if (0x01 == sample_size) {
        ret = HAL_AUDIO_PCM_FORMAT_S8;
    } else if (0x02 == sample_size) {
        ret = HAL_AUDIO_PCM_FORMAT_S16_LE;
    } else if (0x03 == sample_size) {
        ret = HAL_AUDIO_PCM_FORMAT_S24_LE;
    } else if (0x04 == sample_size) {
        ret = HAL_AUDIO_PCM_FORMAT_S32_LE;
    } else {
        ull_assert(0 && "invalid sample size");
    }
    return ret;
}

#if 0
static bt_sink_srv_am_volume_level_out_t bt_ull_volume_exchange_via_ratio(bt_sink_srv_am_volume_level_out_t vol, uint8_t ratio)
{
    float local_level_f = AUD_VOL_OUT_LEVEL0;
    float orignal_f = vol;
    /* ratio 0 ~ 100 */
    local_level_f = (orignal_f * ratio) / 100 + 0.5f;
    return (uint8_t)local_level_f;
}
#endif

void bt_ull_set_ULL_mode(bool enable)
{
    ull_report("[ULL] bt_ull_set_ULL_mode: 0x%x", 1, enable);
    g_bt_ull_mode = enable;
}

bool bt_ull_get_ULL_mode(void)
{
    ull_report("[ULL] bt_ull_get_ULL_mode: 0x%x", 1, g_bt_ull_mode);
    return g_bt_ull_mode;
}

void bt_ull_notify_server_play_is_allow(bt_ull_allow_play_t is_allow)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    /* sync usb play state to client */
    bt_ull_req_t request;
    memset(&request, 0x00, sizeof(request));
    request.event = BT_ULL_EVENT_ALLOW_PLAY_REPORT;

    ull_report("[ULL] bt_ull_notify_server_play_is_allow, is_allow: 0x%x, hanlde: 0x%x, ctx->is_ull_connected: 0x%x", 3,
               is_allow, ctx->spp_handle, ctx->is_ull_connected);

    ctx->allow_play = is_allow;
    if (ctx->is_ull_connected) {
        bt_ull_allow_play_report_t *report = (bt_ull_allow_play_report_t *) & (request.allow_play_report);
        report->allow_play = is_allow;
        bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
    }
}

void bt_ull_start_transmitter(bt_ull_streaming_if_info_t *ep)
{
    ull_report("[ULL] bt_ull_start_transmitter, ep->transmitter: 0x%x, ep->streaming_flag:0x%x, ep->is_transmitter_start:0x%x", 3,
               ep->transmitter, ep->streaming_flag, ep->is_transmitter_start);
    ep->is_request_transmitter_start = true;
    if (false == ep->is_transmitter_start) {
        if (ep->streaming_flag & BT_ULL_STREAMING_STOPPING
            || ep->streaming_flag & BT_ULL_STREAMING_STARTING) {
            /* transmitter is starting or stop, just need wait */
        } else {
            BT_ULL_SET_FLAG(ep->streaming_flag, BT_ULL_STREAMING_STARTING);
            if (AUD_ID_INVALID != ep->transmitter
                && AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_start(ep->transmitter)) {
                ull_report_error("[ULL][Error] audio_transmitter_start fail, 0x%x", 1, ep->transmitter);
                BT_ULL_REMOVE_FLAG(ep->streaming_flag, BT_ULL_STREAMING_STARTING);
            }
        }
    }
}

void bt_ull_stop_transmitter(bt_ull_streaming_if_info_t *ep)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL] bt_ull_stop_transmitter, ep->transmitter: 0x%x, ep->streaming_flag:0x%x, ep->is_transmitter_start:0x%x", 3,
               ep->transmitter, ep->streaming_flag, ep->is_transmitter_start);
    ep->is_request_transmitter_start = false;
    if (true == ep->is_transmitter_start) {
        if (ep->streaming_flag & BT_ULL_STREAMING_STOPPING
            || ep->streaming_flag & BT_ULL_STREAMING_STARTING) {
            /* transmitter is starting or stop, just need wait */
        } else {
            /* gaming & chat transmitter should unmix before stop */
            if ((&(ctx->ul_microphone) != ep)
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
             && (&(ctx->ul_lineout) != ep)
#endif
            ){
                /* unmix 2-rx before transmitter stop */
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ep->transmitter, GAMING_MODE_CONFIG_OP_MUSIC_UNMIX, NULL)) {
                    ull_report_error("[ULL] unmix fail, trans_id:0x%x", 1, ep->transmitter);
                }
            }
            BT_ULL_SET_FLAG(ep->streaming_flag, BT_ULL_STREAMING_STOPPING);
            if (AUD_ID_INVALID != ep->transmitter
                && AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_stop(ep->transmitter)) {
                ull_report_error("[ULL][Error] audio_transmitter_stop fail, 0x%x", 1, ep->transmitter);
                BT_ULL_REMOVE_FLAG(ep->streaming_flag, BT_ULL_STREAMING_STOPPING);
            }
        }
    }
}

void bt_ull_set_mic_frame_size(uint8_t size)
{
    ull_report("[ULL] bt_ull_set_mic_frame_size: %d", 1, size);
    g_bt_frame_size = size;
}

uint8_t bt_ull_get_mic_frame_size(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    uint8_t  bt_frame_size;

    if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
        if(ctx->sdk_version < AIROHA_SDK_VERSION) {
            /*SDK 2.7.1, Mic Frame Size should set 47B*/
            bt_frame_size = B_MIC_FRAME_SIZE;
        } else {
          uint32_t bt_bitrate = audio_ull_get_uplink_bitrate();
          if (32000 == bt_bitrate) {
            bt_frame_size = A_MIC_FRAME_SIZE;
          } else if(50133 == bt_bitrate) {
            bt_frame_size = B_MIC_FRAME_SIZE;
          } else {
            bt_utils_assert(0 && "the bt_bitrate config error");
            return 0;
          }
      }
    } else {
      bt_frame_size = g_bt_frame_size;
    }
    ull_report("[ULL] bt_ull_get_mic_frame_size: %d", 1, bt_frame_size);
    return bt_frame_size;
}

bool bt_ull_srv_is_transmitter_start(bt_ull_streaming_t *streaming)
{
    bt_ull_streaming_if_info_t *ep = NULL;
    bt_ull_context_t *ctx = bt_ull_get_context();

    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface
        && 0x00 == streaming->port) {
        ep = &(ctx->dl_speaker);
    } else if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface
               && 0x01 == streaming->port) {
        ep = &(ctx->dl_chat);
    } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface) {
        ep = &(ctx->ul_microphone);
    } else {
        bt_utils_assert(0 && "is transmitter start unknown usb end pointer type");
        return false;
    }
    return ep->is_transmitter_start;
}

void bt_ull_mic_resource_callback(struct _audio_src_srv_resource_manager_handle_t *current_handle, audio_src_srv_resource_manager_event_t event)
{
    bt_ull_context_t *ctx = bt_ull_get_context();

    /* Please check the current_handle is your self, then check the event for the current_handle */
    ull_report("[ULL][MIC_RESOURCE_CALLBACK]current resource_handle:0x%04x, resource_handle:0x%04x, ull event:0x%04x", 3, current_handle, ctx->ul_microphone.resource_handle, event);
    if (current_handle  == ctx->ul_microphone.resource_handle) {
        switch (event) {
            case  AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS: {
                bt_ull_start_transmitter(&ctx->ul_microphone);
                break;
            }
            case AUDIO_SRC_SRV_EVENT_TAKE_REJECT: {
                audio_src_srv_resource_manager_add_waiting_list(ctx->ul_microphone.resource_handle);
                break;
            }
            case AUDIO_SRC_SRV_EVENT_GIVE_SUCCESS: {
                break;
            }
            case AUDIO_SRC_SRV_EVENT_SUSPEND: {
                bt_ull_stop_transmitter(&ctx->ul_microphone);
                ctx->ul_microphone.is_suspend = true;
                break;
            }
            default: {
                break;
            }
        }
    }
}


