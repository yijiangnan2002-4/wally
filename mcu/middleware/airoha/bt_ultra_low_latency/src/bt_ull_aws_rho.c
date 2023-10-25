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
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
#include "bt_spp.h"
#include "bt_ull_utility.h"
#include "bt_ull_aws_mce.h"
#include "bt_role_handover.h"
#include "bt_device_manager.h"
#include "bt_utils.h"


typedef struct {
    uint8_t sample_rate;    /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000. */
    bool is_mute;               /**< audio streaming is mute/unmute */
    bool is_streaming;       /**< audio streaming is playing or not */
    bt_ull_original_duel_volume_t volume;              /** < audio volume level */
    uint8_t mic_frame_size;               /** < mic frame size */
} bt_ull_streaming_rho_info_t;

typedef struct {
    uint16_t max_packet_size;
    bt_ull_allow_play_t allow_play;        /**< headset/earbuds is allow dongle play. */
    uint16_t            dl_latency;        /**< downlink latency, default: 20, unit: ms */
    uint16_t            ul_latency;        /**< uplink latency, unit: ms */
    /* usb dongle relate */
    bt_ull_streaming_rho_info_t dl_speaker;
    bt_ull_streaming_rho_info_t ul_microphone;
    bt_ull_last_connect_device_t remode_device;  /**< last connected phone info */
} bt_ull_rho_context_t;

static bt_status_t  bt_ull_rho_is_allowed(const bt_bd_addr_t *addr)
{
    bt_aws_mce_role_t aws_role = bt_device_manager_aws_local_info_get_role();

    if (BT_AWS_MCE_ROLE_AGENT != aws_role) {
        ull_report_error("[ULL][RHO][W] Not allowed due to role is :0x%02x", 1, aws_role);
        return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

static uint8_t bt_ull_rho_get_length(const bt_bd_addr_t *addr)
{
    uint8_t data_length = 0;
    bt_ull_context_t *ull_context = bt_ull_get_context();

    if (ull_context->is_ull_connected && addr
        && (0 == memcmp(addr, &(ull_context->bt_addr), sizeof(bt_bd_addr_t)))) {
        data_length = sizeof(bt_ull_rho_context_t);
    }

    ull_report("[ULL][RHO][I] rho_get_length :0x%x", 1, data_length);
    return data_length;
}

static bt_status_t  bt_ull_rho_get_data(const bt_bd_addr_t *addr, void *data)
{
    bt_ull_context_t *ull_context = bt_ull_get_context();
    bt_ull_rho_context_t *rho_context = (bt_ull_rho_context_t *)data;

    if (false == ull_context->is_ull_connected) {
        return BT_STATUS_FAIL;
    }
    /* RHO data */
    rho_context->max_packet_size = ull_context->max_packet_size;
    rho_context->allow_play = ull_context->allow_play;
    rho_context->dl_latency = ull_context->dl_latency;
    rho_context->ul_latency = ull_context->ul_latency;

    rho_context->dl_speaker.sample_rate = ull_context->dl_speaker.sample_rate;
    rho_context->dl_speaker.is_mute = ull_context->dl_speaker.is_mute;
    rho_context->dl_speaker.is_streaming = ull_context->dl_speaker.is_streaming;
    memcpy(&(rho_context->dl_speaker.volume), &(ull_context->dl_speaker.original_volume), sizeof(bt_ull_original_duel_volume_t));

    rho_context->ul_microphone.mic_frame_size = bt_ull_get_mic_frame_size();
    rho_context->ul_microphone.sample_rate = ull_context->ul_microphone.sample_rate;
    rho_context->ul_microphone.is_mute = ull_context->ul_microphone.is_mute;
    rho_context->ul_microphone.is_streaming = ull_context->ul_microphone.is_streaming;
    memcpy(&(rho_context->ul_microphone.volume), &(ull_context->ul_microphone.original_volume), sizeof(bt_ull_original_duel_volume_t));
    /* rho remote smartphone address */
    memcpy(&(rho_context->remode_device), &(ull_context->remode_device), sizeof(bt_ull_last_connect_device_t));
    ull_report("[ULL][RHO][I] rho_get_data mic_frame_size:%d", 1, rho_context->ul_microphone.mic_frame_size);
    return BT_STATUS_SUCCESS;
}

static bt_status_t  bt_ull_rho_update(bt_role_handover_update_info_t *info)
{
    bt_ull_context_t *ull_context = bt_ull_get_context();

#ifdef BT_ROLE_HANDOVER_WITH_SPP_BLE
    ull_context->spp_handle = BT_SPP_INVALID_HANDLE;
    memset(&(ull_context->bt_addr), 0x00, sizeof(bt_bd_addr_t));
    ull_report("[ULL][RHO][I] ULL RHO update current role 0x%x", 1, info->role);
    if (BT_AWS_MCE_ROLE_PARTNER == info->role) {
        bt_utils_assert(0 != info->length);
        bt_ull_rho_context_t *rho_context = (bt_ull_rho_context_t *)(info->data);

        memcpy(&(ull_context->bt_addr), info->addr, sizeof(bt_bd_addr_t));
        ull_context->spp_handle = bt_spp_get_handle_by_local_server_id(info->addr, BT_ULL_ROLE_SERVER_ID);

        ull_context->max_packet_size = rho_context->max_packet_size;
        ull_context->allow_play = rho_context->allow_play;
        ull_context->dl_latency = rho_context->dl_latency;
        ull_context->ul_latency = rho_context->ul_latency;
        /* update downlink streaming info */
        ull_context->dl_speaker.sample_rate = rho_context->dl_speaker.sample_rate;
        ull_context->dl_speaker.is_mute = rho_context->dl_speaker.is_mute;
        ull_context->dl_speaker.is_streaming = rho_context->dl_speaker.is_streaming;
        memcpy(&(ull_context->dl_speaker.original_volume), &(rho_context->dl_speaker.volume), sizeof(bt_ull_original_duel_volume_t));
        /* update uplink streaming info */
        ull_context->ul_microphone.sample_rate = rho_context->ul_microphone.sample_rate;
        ull_context->ul_microphone.is_mute = rho_context->ul_microphone.is_mute;
        ull_context->ul_microphone.is_streaming = rho_context->ul_microphone.is_streaming;
        memcpy(&(ull_context->ul_microphone.original_volume), &(rho_context->ul_microphone.volume), sizeof(bt_ull_original_duel_volume_t));
        if(rho_context->ul_microphone.mic_frame_size != bt_ull_get_mic_frame_size()) {
            bt_ull_set_mic_frame_size(rho_context->ul_microphone.mic_frame_size);
        }
        /* update remote smartphone address */
        memcpy(&(ull_context->remode_device), &(rho_context->remode_device), sizeof(bt_ull_last_connect_device_t));

        ull_report("[ULL] rho complemte, new agent ull codec am_state:0x%x, current server mic state: 0x%x, spp_handle:0x%x", 3,
            ull_context->am_state, ull_context->ul_microphone.is_streaming, ull_context->spp_handle);
        if (BT_ULL_AM_PLAYING == ull_context->am_state && ull_context->ul_microphone.is_streaming) {
            /* start voice path */
            if(ull_context->ul_microphone.resource_handle != NULL) {
               audio_src_srv_resource_manager_take(ull_context->ul_microphone.resource_handle);
            }
        } else if (false == ull_context->is_ull_connected) {
            ull_report("[ULL][RHO][I] RHO Update not in ULL aws link! ", 0);
            ull_context->is_ull_connected = true;
            /* set codec info */
            ull_context->codec_type = BT_ULL_OPUS_CODEC;
            memset(&(ull_context->codec_cap), 0x00, sizeof(bt_a2dp_codec_capability_t));
            ull_context->codec_cap.type = BT_ULL_CODEC_AIRO_CELT;
            ull_context->codec_cap.sep_type = BT_A2DP_SINK;
        }
    } else if (BT_AWS_MCE_ROLE_AGENT == info->role) {
        ull_report("[ULL] rho complemte, new partner mic stats:0x%x", 1,
                   ull_context->ul_microphone.is_transmitter_start);
        bt_bd_addr_t *agent_addr = bt_connection_manager_device_local_info_get_local_address();
        memcpy(&(ull_context->bt_addr), agent_addr, sizeof(bt_bd_addr_t));
        ull_context->ul_microphone.is_streaming = false;
        /* stop voice audio transmitter */
        if (AUD_ID_INVALID != ull_context->ul_microphone.transmitter) {
            ull_context->ul_microphone.is_request_transmitter_start = false;
            if (true == ull_context->ul_microphone.is_transmitter_start) {
                /* stop transmitter & disable sidetone */
                am_audio_side_tone_disable();
                bt_ull_stop_transmitter(&ull_context->ul_microphone);
            }
        }
    }
#endif
    return BT_STATUS_SUCCESS;
}

static void bt_ull_rho_status_callback(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status)
{
    ull_report("[ULL][RHO][I] Status callback role:0x%02x, event:0x%02x, status:0x%x", 3, role, event, status);

    switch (event) {
        case BT_ROLE_HANDOVER_START_IND: {

        }
        break;

        case BT_ROLE_HANDOVER_PREPARE_REQ_IND: {

        }
        break;

        case BT_ROLE_HANDOVER_COMPLETE_IND:
        {
            bt_ull_context_t* ull_context = bt_ull_get_context();

            if (BT_AWS_MCE_ROLE_PARTNER == role) {
                if(BT_STATUS_SUCCESS == status) {
                     if (ull_context->dl_latency > BT_ULL_DOWNLINK_LIMIATTION_MULTILINK) {
                         bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
                     } else {
                         if (ull_context->is_ull_connected) {
                             /* should disable page scan if latency < 25ms */
                             bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_DISABLE);
                        }
                    }
                }
#ifdef MTK_AWS_MCE_ENABLE
                bt_ull_update_audio_buffer(ull_context->ull_role, BT_ULL_EARBUDS_CLIENT);
#else
                bt_ull_update_audio_buffer(ull_context->ull_role, BT_ULL_HEADSET_CLIENT);
#endif
                audio_transmitter_status_t status = audio_transmitter_deinit(ull_context->ul_microphone.transmitter);
                ull_report("[ULL] rho complemte, new agent transmitter deinit :0x%x", 1, status);
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                       ull_context->ul_microphone.transmitter = AUD_ID_INVALID;
                       bt_ull_init_transimter(ull_context->codec_type, BT_ULL_MIC_TRANSMITTER);
                }

                if (ull_context->is_ull_connected) {
                    if (BT_ULL_AM_PLAYING != ull_context->am_state) {
                        bt_ull_aws_mce_handle_connection(ull_context->is_ull_connected);
                        if (ull_context->dl_speaker.is_streaming || ull_context->ul_microphone.is_streaming) {
                            bt_ull_am_play();
                        }
                    }
                }
            }else if (BT_AWS_MCE_ROLE_AGENT == role) {
                 if(status != BT_STATUS_SUCCESS) {
                      if (ull_context->dl_latency > BT_ULL_DOWNLINK_LIMIATTION_MULTILINK) {
                          bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
                      } else {
                        if (ull_context->is_ull_connected) {
                              /* should disable page scan if latency < 25ms */
                             bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_DISABLE);
                        }
                    }
                }
            }
        }
        break;

        default:
            break;
    }
}

void bt_ull_rho_init(void)
{
    bt_role_handover_callbacks_t bt_ull_rho_callback_sets = {
        .allowed_cb =   &bt_ull_rho_is_allowed,
        .get_len_cb =   &bt_ull_rho_get_length,
        .get_data_cb =  &bt_ull_rho_get_data,
        .update_cb =    &bt_ull_rho_update,
        .status_cb =    &bt_ull_rho_status_callback
    };
    bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_ULL, &bt_ull_rho_callback_sets);
}

#endif
