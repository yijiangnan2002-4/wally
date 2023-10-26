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
#include "bt_spp.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager_internal.h"
#include "bt_callback_manager.h"
#include "bt_ull_service.h"
#include "bt_ull_utility.h"
#include "bt_ull_audio_manager.h"
#include "bt_utils.h"


#ifdef MTK_AWS_MCE_ENABLE
#include "bt_ull_aws_mce.h"
#endif
#include "bt_timer_external.h"

#include "usbaudio_drv.h"
#include "avm_direct.h"
#include "bt_a2dp.h"
#include "bt_avrcp.h"
#include "bt_gap.h"

/* Hal includes. */
#include "hal_ccni.h"
#include "hal_ccni_config.h"
#include "hal_gpt.h"

#ifdef BT_SINK_DUAL_ANT_ENABLE
#include "bt_sink_srv_dual_ant.h"
#endif

bt_ull_context_t g_bt_ull_context = {{0}};

#define ULL_CONFLICT_RECONNECT_TIMER        1000

#ifdef AIR_USB_ENABLE
#define BT_ULL_MIC_PCM_DATA_RATE 192
#include "usb_main.h"
/* for ull dongle voice uplink */
static uint16_t g_ull_voice_data_offset = 0;
static uint8_t *voice_data = NULL;
static uint32_t voice_data_length = 0;
static uint8_t joint_buffer[BT_ULL_MIC_PCM_DATA_RATE] = {0};     /* 48KHZ/16Bit/Stereo */
static uint8_t all_zero_buffer[BT_ULL_MIC_PCM_DATA_RATE] = {0};  /* 48KHZ/16Bit/Stereo */
static uint8_t joint_buffer_offset = 0;
static bool is_debug_mode_enable = false;
static uint32_t g_ull_usb_tx_gpt = 0;

#ifdef AIR_ECNR_POST_PART_ENABLE
static uint32_t g_ull_wait_dsp_decode_len = 0;
static bool is_dsp_pre_decode_done = false;
#endif

#endif
static bt_status_t bt_ull_connect(uint32_t *handle, const bt_bd_addr_t *address);
static bt_status_t bt_ull_disconnect(uint32_t handle);
static bt_status_t  bt_ull_cm_callback_handler(bt_cm_profile_service_handle_t type, void *data);
static bt_status_t bt_ull_spp_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff);
static uint32_t bt_ull_get_handle_by_address(const bt_bd_addr_t *address);
static bt_status_t bt_ull_connection_handle(uint32_t handle, bt_ull_connection_state_t state);
static void bt_ull_rx_data_handle(uint8_t *data, uint16_t length);
static bt_status_t bt_ull_cm_event_callback(bt_cm_event_t event_id, void *params, uint32_t params_len);
#ifdef AIR_USB_ENABLE
static void bt_ull_ccni_dsp(void);
static void bt_ull_usb_tx_cb(void);
#endif
static bt_status_t bt_ull_handle_start_streaming(bt_ull_streaming_t *streaming);
static bt_status_t bt_ull_handle_stop_streaming(bt_ull_streaming_t *streaming);
static bt_status_t bt_ull_handle_set_volume(bt_ull_volume_t *vol);
static bt_status_t bt_ull_handle_set_streaming_mute(bt_ull_streaming_t *streaming, bool is_mute);
static bt_status_t bt_ull_handle_set_streaming_sample_rate(bt_ull_sample_rate_t *sample_rate);
static bt_status_t bt_ull_handle_set_streaming_sample_size(bt_ull_streaming_sample_size_t *sample_size);
static bt_status_t bt_ull_handle_set_streaming_sample_channel(bt_ull_streaming_sample_channel_t *sample_channel);

static bt_status_t bt_ull_handle_set_streaming_latency(bt_ull_latency_t *req_latency);
static bt_status_t bt_ull_handle_set_streaming_mix_ratio(bt_ull_mix_ratio_t *ratio);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
static bt_status_t bt_ull_handle_set_streaming_param(bt_ull_interface_config_t *param);
#endif
static bt_status_t bt_ull_handle_usb_hid_control(bt_ull_usb_hid_control_t action);
static bt_status_t bt_ull_handle_gap_event_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);
static bt_status_t bt_ull_send_user_raw_data(uint32_t handle, uint8_t *packet, uint16_t packet_size);
static bt_status_t bt_ull_critial_data_init(uint8_t max_len);
static bt_status_t bt_ull_critial_data_send(uint32_t gap_handle, uint16_t flush_timeout, bt_avm_critial_data_t *data);

static void bt_ull_conflict_reconnect_timeout_callback(uint32_t timer_id, uint32_t data);
static bt_status_t bt_ull_unmix_handle(bt_ull_streaming_if_info_t *ep);
static void bt_ull_reconfig_transmitter(bt_ull_transmitter_t transmitter_type);
static uint32_t bt_ull_handle_sample_rate_switch(uint32_t sample_rate);



static bt_status_t bt_ull_set_version(uint32_t version);
extern void BT_Set_Peer_Sdk_Version(U32 value);

#ifdef AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE
static void bt_ull_uplink_latency_debug(uint16_t data_len, int16_t* data);
#endif

#ifdef MTK_AWS_MCE_ENABLE
static void bt_ull_agent_sync_connection_info(void);
static void bt_ull_sync_agent_info_to_partner(void);
#endif

bt_status_t bt_ull_srv_init(bt_ull_role_t role, bt_ull_callback callback)
{
    bt_status_t result = BT_STATUS_SUCCESS;

    memset(&g_bt_ull_context, 0, sizeof(bt_ull_context_t));

    g_bt_ull_context.ull_role = role;
    g_bt_ull_context.callback = callback;
    g_bt_ull_context.audio_id = AUD_ID_INVALID;
    g_bt_ull_context.ul_microphone.transmitter = AUD_ID_INVALID;
    g_bt_ull_context.ul_microphone.volume.vol_left = AUD_VOL_OUT_MAX;
    g_bt_ull_context.ul_microphone.volume.vol_right = AUD_VOL_OUT_MAX;
    g_bt_ull_context.ul_microphone.sample_rate = 0x01;
    g_bt_ull_context.dl_speaker.transmitter = AUD_ID_INVALID;
    g_bt_ull_context.dl_speaker.volume.vol_left = AUD_VOL_OUT_MAX;
    g_bt_ull_context.dl_speaker.volume.vol_right = AUD_VOL_OUT_MAX;

    g_bt_ull_context.dl_chat.transmitter = AUD_ID_INVALID;
    g_bt_ull_context.dl_chat.volume.vol_left = AUD_VOL_OUT_MAX;
    g_bt_ull_context.dl_chat.volume.vol_right = AUD_VOL_OUT_MAX;

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
    g_bt_ull_context.dl_linein.transmitter = AUD_ID_INVALID;
    g_bt_ull_context.dl_linein.volume.vol_left = AUD_VOL_OUT_MAX;
    g_bt_ull_context.dl_linein.volume.vol_right = AUD_VOL_OUT_MAX;
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
    g_bt_ull_context.ul_lineout.transmitter = AUD_ID_INVALID;
    g_bt_ull_context.ul_lineout.volume.vol_left = AUD_VOL_OUT_MAX;
    g_bt_ull_context.ul_lineout.volume.vol_right = AUD_VOL_OUT_MAX;
    g_bt_ull_context.ul_lineout.sample_rate = 0x01;
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
    g_bt_ull_context.dl_linei2s.transmitter = AUD_ID_INVALID;
    g_bt_ull_context.dl_linei2s.volume.vol_left = AUD_VOL_OUT_MAX;
    g_bt_ull_context.dl_linei2s.volume.vol_right = AUD_VOL_OUT_MAX;
#endif

    /* default gaming && chat streaming 100% mix */
    g_bt_ull_context.dl_mix_ratio.num_streaming = BT_ULL_MAX_STREAMING_NUM;
    g_bt_ull_context.dl_mix_ratio.streamings[0].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
    g_bt_ull_context.dl_mix_ratio.streamings[0].streaming.port = 0; /* gaming streaming port */
    g_bt_ull_context.dl_mix_ratio.streamings[0].ratio = BT_ULL_MIX_RATIO_MAX;
    g_bt_ull_context.dl_mix_ratio.streamings[1].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
    g_bt_ull_context.dl_mix_ratio.streamings[1].streaming.port = 1; /* chat streaming port */
    g_bt_ull_context.dl_mix_ratio.streamings[1].ratio = BT_ULL_MIX_RATIO_MAX;
    g_bt_ull_context.dl_latency = BT_ULL_DOWNLINK_LIMIATTION;
    g_bt_ull_context.ul_latency = BT_ULL_UPLINK_DEFAULT;
    g_bt_ull_context.sdk_version = AIROHA_SDK_VERSION;
    bt_ull_clear_spp_cache();
    result = bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_SPP, (void *)bt_ull_spp_event_callback);
    bt_cm_profile_service_register(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL, (bt_cm_profile_service_handle_callback_t)bt_ull_cm_callback_handler);
    /* init atci cmd module */
    //bt_ull_atci_init();

    /* register gap callback to ull sniff mode change notification */
    bt_callback_manager_register_callback(bt_callback_type_app_event,(uint32_t)(MODULE_MASK_GAP), (void *)bt_ull_handle_gap_event_callback);

    if (BT_ULL_ROLE_SERVER == role) {
#if defined(AIR_USB_ENABLE) && defined(AIR_USB_AUDIO_ENABLE) && defined(AIR_USB_AUDIO_1_MIC_ENABLE)
        USB_Audio_Register_Tx_Callback(0, bt_ull_usb_tx_cb);
#endif
    } else {
        if (BT_STATUS_SUCCESS != bt_cm_register_event_callback(&bt_ull_cm_event_callback)) {
            bt_utils_assert(0 && "register cm event fail !!!");
        }
        /* if support earbuds, register aws mce callback */
#ifdef MTK_AWS_MCE_ENABLE
        if (BT_STATUS_SUCCESS != bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_ULL, &bt_ull_aws_mce_packet_callback)) {
            bt_utils_assert(0 && "register aws mce report fail !!!");
        }
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
        bt_ull_rho_init();
#endif
#endif
    }
    return result;
}


bt_status_t bt_ull_srv_action(bt_ull_action_t action, const void *param, uint32_t param_len)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_context_t *ull_context = bt_ull_get_context();

    ull_report("[ULL][API] bt_ull_action action: 0x%x, role: 0x%x, spp_handle:0x%x, is_ull_connected:0x%x, is_ull_version_ready:0x%x", 5,
               action, ull_context->ull_role, ull_context->spp_handle, ull_context->is_ull_connected,ull_context->is_ull_version_ready);
    
    BT_ULL_MUTEX_LOCK();
    switch (action) {
        case BT_ULL_ACTION_START_STREAMING: {
            bt_utils_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)param;
            status = bt_ull_handle_start_streaming(streaming);
            break;
        }
        case BT_ULL_ACTION_STOP_STREAMING: {
            bt_utils_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)param;
            status = bt_ull_handle_stop_streaming(streaming);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_VOLUME: {
            bt_utils_assert(param);
            bt_ull_volume_t *volume = (bt_ull_volume_t *)param;
            status = bt_ull_handle_set_volume(volume);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_MUTE: {
            bt_utils_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)param;
            status = bt_ull_handle_set_streaming_mute(streaming, true);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_UNMUTE: {
            bt_utils_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)param;
            status = bt_ull_handle_set_streaming_mute(streaming, false);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE: {
            bt_utils_assert(param);
            bt_ull_sample_rate_t *sample_rate = (bt_ull_sample_rate_t *)param;
            status = bt_ull_handle_set_streaming_sample_rate(sample_rate);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_SAMPLE_SIZE: {
            bt_utils_assert(param);
            bt_ull_streaming_sample_size_t *sample_size = (bt_ull_streaming_sample_size_t *)param;
            status = bt_ull_handle_set_streaming_sample_size(sample_size);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_SAMPLE_CHANNEL: {
            bt_utils_assert(param);
            bt_ull_streaming_sample_channel_t *sample_channel= (bt_ull_streaming_sample_channel_t *)param;
            status = bt_ull_handle_set_streaming_sample_channel(sample_channel);
            break;
        }
        case BT_ULL_ACTION_START_PAIRING: {
            bt_utils_assert(param);
            bt_ull_pairing_info_t *pairing = (bt_ull_pairing_info_t *)param;
            status = bt_ull_air_pairing_start(pairing);
            break;
        }
        case BT_ULL_ACTION_STOP_PAIRING: {
            status = bt_ull_air_pairing_stop();
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_LATENCY: {
            bt_utils_assert(param);
            bt_ull_latency_t *latency = (bt_ull_latency_t *)param;
            status = bt_ull_handle_set_streaming_latency(latency);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_MIX_RATIO: {
            bt_utils_assert(param);
            bt_ull_mix_ratio_t *ratio = (bt_ull_mix_ratio_t *)param;
            status = bt_ull_handle_set_streaming_mix_ratio(ratio);
            break;
        }
        case BT_ULL_ACTION_TX_USER_DATA: {
            bt_utils_assert(param);
            bt_ull_user_data_t *tx = (bt_ull_user_data_t *)param;
#ifdef MTK_AWS_MCE_ENABLE
            bt_aws_mce_role_t cur_role = bt_device_manager_aws_local_info_get_role();
            if (BT_AWS_MCE_ROLE_PARTNER == cur_role
                || BT_AWS_MCE_ROLE_CLINET == cur_role) {
                ull_report("[ULL] BT_ULL_ACTION_TX_USER_DATA fail due to role is partner/client", 0);
                BT_ULL_MUTEX_UNLOCK();
                return BT_STATUS_FAIL;
            }
#endif
            if (!memcmp(ull_context->bt_addr, tx->remote_address, sizeof(bt_bd_addr_t))) {
                bt_ull_req_event_t req_action = BT_ULL_EVENT_USER_DATA;
                uint16_t total_len = tx->user_data_length + sizeof(tx->user_data_length) + sizeof(bt_ull_req_event_t);
                uint8_t *tx_buf = (uint8_t *)bt_ull_memory_alloc(total_len);
                tx_buf[0] = req_action;
                bt_utils_assert(tx->user_data_length && tx->user_data);
                memcpy(&tx_buf[1], &(tx->user_data_length), sizeof(tx->user_data_length));
                memcpy(&tx_buf[3], tx->user_data, tx->user_data_length);
                status = bt_ull_send_user_raw_data(ull_context->spp_handle, tx_buf, total_len);
                bt_ull_memory_free(tx_buf);
            } else {
                status = BT_STATUS_FAIL;
            }
            break;
        }
        case BT_ULL_ACTION_INIT_CRITICAL_USER_DATA: {
            bt_utils_assert(param);
            bt_ull_init_critical_user_data_t *max_len = (bt_ull_init_critical_user_data_t *)param;
            status = bt_ull_critial_data_init(max_len->max_user_data_length);
            break;
        }
        case BT_ULL_ACTION_TX_CRITICAL_USER_DATA: {
            bt_utils_assert(param);
#ifdef MTK_AWS_MCE_ENABLE
            bt_aws_mce_role_t cur_role = bt_device_manager_aws_local_info_get_role();
            if (BT_AWS_MCE_ROLE_PARTNER == cur_role
                || BT_AWS_MCE_ROLE_CLINET == cur_role) {
                ull_report("[ULL] BT_ULL_ACTION_TX_CRITICAL_USER_DATA fail due to role is partner/client", 0);
                BT_ULL_MUTEX_UNLOCK();
                return BT_STATUS_FAIL;
            }
#endif
            bt_ull_tx_critical_user_data_t *tx_data = (bt_ull_tx_critical_user_data_t *)param;
            if (ull_context->is_ull_connected) {
                bt_gap_connection_handle_t gap_handle = bt_cm_get_gap_handle(ull_context->bt_addr);
                bt_avm_critial_data_t critical_data = {0};
                critical_data.seq = ull_context->critical_data_tx_seq;
                critical_data.length = tx_data->user_data_length;
                critical_data.data = tx_data->user_data;
                critical_data.is_le_data = false;
                status = bt_ull_critial_data_send(gap_handle, tx_data->flush_timeout, &critical_data);
                if (BT_STATUS_SUCCESS == status) {
                    if (0xFF == ull_context->critical_data_tx_seq) {
                        ull_context->critical_data_tx_seq = 0x01;
                    } else {
                        ull_context->critical_data_tx_seq++;
                    }
                }
            }
            break;
        }
        case BT_ULL_ACTION_USB_HID_CONTROL: {
            bt_utils_assert(param);
            bt_ull_usb_hid_control_t control_id = *((bt_ull_usb_hid_control_t *)param);
            status = bt_ull_handle_usb_hid_control(control_id);
            break;
        }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
        case BT_ULL_ACTION_SET_INTERFACE_PARAM: {
            bt_utils_assert(param);
            bt_ull_interface_config_t *info = (bt_ull_interface_config_t*)param;
            bt_ull_handle_set_streaming_param(info);
            break;
        }
#endif
        case BT_ULL_ACTION_SET_VERSION: {
            bt_utils_assert(param);
            uint32_t p_version = *((uint32_t*)param);
            bt_ull_set_version(p_version);
            break;
        }
        default:
            status = BT_STATUS_FAIL;
            break;
    }
    BT_ULL_MUTEX_UNLOCK();

    return status;
}

bt_status_t bt_ull_srv_get_streaming_info(bt_ull_streaming_t streaming, bt_ull_streaming_info_t *info)
{
    bt_status_t status = BT_STATUS_SUCCESS;

    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL][API] bt_ull_get_streaming_info role: 0x%x, type: 0x%x, port:0x%x", 3,
               ctx->ull_role, streaming.streaming_interface, streaming.port);
    if (info) {
        if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming.streaming_interface) {
            if (0 == streaming.port) {
                if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
                    if (BT_ULL_AM_PLAYING == ctx->am_state) {
                        info->is_playing = true;
                    } else {
                        info->is_playing = false;
                    }
                } else {
                    info->is_playing = ctx->dl_speaker.is_streaming;
                }
                info->latency = ctx->dl_latency;
                info->sample_rate = bt_ull_sample_rate_exchange(ctx->dl_speaker.sample_rate);
                info->volume_left = ctx->dl_speaker.original_volume.vol_left;
                info->volume_right = ctx->dl_speaker.original_volume.vol_right;
            } else if (1 == streaming.port) {
                info->is_playing = ctx->dl_chat.is_streaming;
                info->latency = ctx->dl_latency;
                info->sample_rate = bt_ull_sample_rate_exchange(ctx->dl_chat.sample_rate);
                info->volume_left = ctx->dl_chat.original_volume.vol_left;
                info->volume_right = ctx->dl_chat.original_volume.vol_right;
            } else {
                status = BT_STATUS_FAIL;
            }
        } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming.streaming_interface) {
            info->is_playing = ctx->ul_microphone.is_transmitter_start;
            if (ctx->dl_latency > BT_ULL_DOWNLINK_SINGLE_LIMITATION) {
                info->latency = BT_ULL_UPLINK_MAX;
            } else {
                if(BT_ULL_ROLE_SERVER == ctx->ull_role) {
                  if(ctx->sdk_version < AIROHA_SDK_VERSION) {
                    info->latency = BT_ULL_UPLINK_LEGENCY_DEFAULT;
                  } else {
                    info->latency = BT_ULL_UPLINK_DEFAULT;
                  }
                } else {
                    info->latency = BT_ULL_UPLINK_DEFAULT;
                }
            }
            info->sample_rate = bt_ull_sample_rate_exchange(ctx->ul_microphone.sample_rate);
            info->volume_left = ctx->ul_microphone.original_volume.vol_left;
            info->volume_right = ctx->ul_microphone.original_volume.vol_right;
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
        } else if (BT_ULL_STREAMING_INTERFACE_LINE_IN == streaming.streaming_interface) {
             info->is_playing = ctx->dl_linein.is_streaming;
             info->latency = ctx->dl_latency;
             info->sample_rate = bt_ull_sample_rate_exchange(ctx->dl_linein.sample_rate);
             info->volume_left = ctx->dl_linein.original_volume.vol_left;
             info->volume_right = ctx->dl_linein.original_volume.vol_right;
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                } else if (BT_ULL_STREAMING_INTERFACE_I2S_IN == streaming.streaming_interface) {
             info->is_playing = ctx->dl_linei2s.is_streaming;
             info->latency = ctx->dl_latency;
             info->sample_rate = bt_ull_sample_rate_exchange(ctx->dl_linei2s.sample_rate);
             info->volume_left = ctx->dl_linei2s.original_volume.vol_left;
             info->volume_right = ctx->dl_linei2s.original_volume.vol_right;
#endif
        } else {
            status = BT_STATUS_FAIL;
        }
    } else {
        status = BT_STATUS_FAIL;
    }
    if (BT_STATUS_SUCCESS == status) {
        ull_report("[ULL][API] bt_ull_get_streaming_info, volume_left: %d, vol_right: %d, latency: %d, is_playing:0x%x, sample_rate: %d", 5,
                   info->volume_left, info->volume_right, info->latency, info->is_playing, info->sample_rate);
    }
    
    return status;
}

bt_status_t bt_ull_connect(uint32_t *handle, const bt_bd_addr_t *address)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_context_t *ull_context = bt_ull_get_context();

    if (address && handle) {
        status = bt_spp_connect(&(ull_context->spp_handle), address, bt_ull_get_uuid());
        if (BT_STATUS_SUCCESS == status) {
            *handle = ull_context->spp_handle;
            memcpy(ull_context->bt_addr, address, sizeof(bt_bd_addr_t));
        } else {
            ull_context->spp_handle = BT_SPP_INVALID_HANDLE;
            *handle = 0;
        }
    } else {
        status = BT_STATUS_FAIL;
        ull_report_error("[ULL]bt_ull_connect fail due to invalid parameters", 0);
    }

    ull_report("[ULL]bt_ull_connect, status: 0x%x, spp_handle:0x%x", 2, status, ull_context->spp_handle);
    return status;
}

bt_status_t bt_ull_disconnect(uint32_t handle)
{
    return bt_spp_disconnect(handle);
}


static void bt_ull_conflict_reconnect_timeout_callback(uint32_t timer_id, uint32_t data)
{
    bt_ull_context_t* ctx = bt_ull_get_context();
    ull_report("[ULL]reconnect_timeout_callback, reconnect handle: 0x%x, ctx->bt_addr:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", 7,
                      ctx->spp_handle, ctx->bt_addr[0], ctx->bt_addr[1], ctx->bt_addr[2], ctx->bt_addr[3], ctx->bt_addr[4], ctx->bt_addr[5]);
    bt_ull_connect(&(ctx->spp_handle), (const bt_bd_addr_t *)ctx->bt_addr);
}

uint32_t bt_ull_get_handle_by_address(const bt_bd_addr_t* address)
{
    uint32_t handle = 0;
    if (address && !memcmp(address, g_bt_ull_context.bt_addr, BT_BD_ADDR_LEN)) {
        handle = g_bt_ull_context.spp_handle;
    } else {
        handle = 0;
    }
    return handle;
}

bt_status_t bt_ull_spp_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    //ull_report("[ULL]bt_ull_spp_event_callback, msg: 0x%x, status: 0x%x", 2, msg, status);
    switch (msg) {
        case BT_SPP_CONNECT_IND: {
            if (NULL == buff) {
                break;
            }
            bt_spp_connect_ind_t *conn_ind_p = (bt_spp_connect_ind_t *)buff;
            ull_report("[ULL]BT_SPP_CONNECT_IND, handle: 0x%x, channel_id: 0x%x, local_handle: 0x%x", 3,
                       conn_ind_p->handle, conn_ind_p->local_server_id, ctx->spp_handle);

            if (BT_ULL_ROLE_SERVER_ID == conn_ind_p->local_server_id) {
                /* client always accept server connect_ind to avoid connection conflict */
                if (BT_SPP_INVALID_HANDLE != ctx->spp_handle
                    && BT_ULL_ROLE_SERVER == ctx->ull_role) {
                    bt_spp_connect_response(conn_ind_p->handle, false);
                } else {
                    ctx->spp_handle = conn_ind_p->handle;
                    ctx->server_channel_id = conn_ind_p->local_server_id;
                    memcpy(ctx->bt_addr, conn_ind_p->address, BT_BD_ADDR_LEN);
                    bt_spp_connect_response(ctx->spp_handle, true);
                }
            }
        }
        break;

    case BT_SPP_CONNECT_CNF: {/* transport connection is established. */
        if (NULL == buff) {
            break;
        }
        bt_spp_connect_cnf_t *conn_cnf_p = (bt_spp_connect_cnf_t *)buff;
        ull_report("[ULL]BT_SPP_CONNECT_CNF, ctx->spp_handle: 0x%x, conn_cnf_p handle: 0x%x, status: 0x%x", 3, ctx->spp_handle, conn_cnf_p->handle, status);
        if (ctx->spp_handle == conn_cnf_p->handle) {
            if (BT_STATUS_SUCCESS != status) {
                ctx->is_ull_connected = false;
                ctx->spp_handle = BT_SPP_INVALID_HANDLE;
                if (BT_ULL_ROLE_SERVER == ctx->ull_role && BT_STATUS_FAIL == status) {
                    bt_timer_ext_start(BT_ULL_CONFLICT_RECONNECT_TIMER_ID, 0 , ULL_CONFLICT_RECONNECT_TIMER, bt_ull_conflict_reconnect_timeout_callback);
                 } else {
                    /* notify app connected fail*/
                    bt_cm_profile_service_status_notify(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL, ctx->bt_addr, BT_CM_PROFILE_SERVICE_STATE_DISCONNECTED, status);
                 }
            } else {
                /* clear old cached spp data after spp connected */
                bt_ull_clear_spp_cache();
                ctx->server_channel_id = conn_cnf_p->server_id;
                ull_report("[ULL]BT_ULL_SPP_Connected, spp handle:0x%x, ull_role:0x%x, server_id:0x%x", 3, ctx->spp_handle, ctx->ull_role, conn_cnf_p->server_id);
                bt_cm_profile_service_status_notify(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL, ctx->bt_addr,
                    BT_CM_PROFILE_SERVICE_STATE_CONNECTED, status);
                ctx->max_packet_size = conn_cnf_p->max_packet_length;
                bt_ull_connection_handle(ctx->spp_handle, BT_ULL_CONNECTED);
            }
        }
    }
    break;

        case BT_SPP_DISCONNECT_IND: {
            if (NULL == buff) {
                break;
            }
            bt_spp_disconnect_ind_t *disc_ind_p = (bt_spp_disconnect_ind_t *)buff;
            if (ctx->spp_handle == disc_ind_p->handle) {
                /* clear old cached spp data after spp disconnected */
                bt_ull_clear_spp_cache();
                ctx->is_ull_connected = false;
                ctx->is_ull_version_ready = false;
                ull_report("[ULL]BT_ULL_SPP_Disconnected, spp handle:0x%x, ull_role:0x%x", 2, ctx->spp_handle, ctx->ull_role);
                bt_cm_profile_service_status_notify(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL, ctx->bt_addr,
                                                    BT_CM_PROFILE_SERVICE_STATE_DISCONNECTED, status);
                /* notify app disconencted*/
                bt_ull_connection_handle(ctx->spp_handle, BT_ULL_DISCONNECTED);
                ctx->spp_handle = BT_SPP_INVALID_HANDLE;
                ctx->is_disable_sniff = false;
            }
        }
        break;

        case BT_SPP_DATA_RECEIVED_IND: {
            if (NULL == buff) {
                break;
            }
            bt_spp_data_received_ind_t *data_ind_p = (bt_spp_data_received_ind_t *)buff;
            if (ctx->spp_handle == data_ind_p->handle) {
                bt_ull_rx_data_handle(data_ind_p->packet, data_ind_p->packet_length);
            }
        }
        break;

        case BT_SPP_READY_TO_SEND_IND: {
            if (NULL == buff) {
                break;
            }
            bt_spp_ready_to_send_ind_t *send_ind = (bt_spp_ready_to_send_ind_t *)buff;
            ull_report("[ULL]BT_SPP_READY_TO_SEND_IND, spp handle:0x%x, current handle:0x%x", 2, send_ind->handle, ctx->spp_handle);
            if (ctx->spp_handle == send_ind->handle) {
                bt_ull_send_cache_spp_data(ctx->spp_handle);
            }
        }
        break;

        default:
            break;
    }

    return BT_STATUS_SUCCESS;
}

bt_status_t bt_ull_send_user_raw_data(uint32_t handle, uint8_t *packet, uint16_t packet_size)
{
    uint16_t send_length = 0;
    bool need_resend = false;
    bt_status_t result = BT_STATUS_SUCCESS;

    if (g_bt_ull_context.max_packet_size < packet_size) {
        send_length = g_bt_ull_context.max_packet_size;
        need_resend = true;
        result = bt_spp_send(handle, packet, send_length);
        if ((BT_STATUS_SUCCESS == result) && (need_resend)) {
            need_resend = false;
            result = bt_ull_send_user_raw_data(handle, (uint8_t *)(packet + send_length), (packet_size - send_length));
        }
    } else {
        result = bt_spp_send(handle, packet, packet_size);
    }
    ull_report("[ULL] bt_ull_send_user_raw_data, len: 0x%x, channel_max_size:0x%x, result:0x%x", 3,
               packet_size, g_bt_ull_context.max_packet_size, result);

    return result;
}


bt_status_t bt_ull_send_data(uint32_t handle, uint8_t *packet, uint16_t packet_size)
{
    uint16_t send_length = 0;
    bool need_resend = false;
    bt_status_t result = 0;

    //ull_report("[ULL] send data len: 0x%x, channel_max_size:0x%x", 2, packet_size, g_bt_ull_context.max_packet_size);
    if (g_bt_ull_context.max_packet_size < packet_size) {
        send_length = g_bt_ull_context.max_packet_size;
        need_resend = true;
        result = bt_spp_send(handle, packet, send_length);
        if ((BT_STATUS_SUCCESS == result) && (need_resend)) {
            need_resend = false;
            result = bt_ull_send_data(handle, (uint8_t *)(packet + send_length), (packet_size - send_length));
        }
    } else {
        result = bt_spp_send(handle, packet, packet_size);
    }

    if (BT_STATUS_SPP_TX_NOT_AVAILABLE == result) {
        bt_ull_cache_spp_tx((bt_ull_req_t *)packet);
    }
    return result;
}

void bt_ull_hold_data(uint8_t *packet)
{
    ull_report("[ULL]bt_ull_hold_data, hold data point: 0x%4x", 1, packet);

    bt_spp_hold_data(packet);
}

void bt_ull_release_data(uint8_t *packet)
{
    ull_report("[ULL]bt_ull_release_data, hold data point: 0x%4x", 1, packet);

    bt_spp_release_data(packet);
}

bt_status_t  bt_ull_cm_callback_handler(bt_cm_profile_service_handle_t type, void *data)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t *address = NULL;
    bt_ull_context_t *ctx = bt_ull_get_context();

    ull_report("[ULL]bt_ull_cm_callback_handler, msg: 0x%x, ull_role: 0x%x", 2, type, ctx->ull_role);
    switch (type) {
        case BT_CM_PROFILE_SERVICE_HANDLE_POWER_ON: {
            /* power on clear spp handle */
            ctx->spp_handle = BT_SPP_INVALID_HANDLE;
            ctx->is_ull_connected = false;
            /** register serviceRecord. */
            status = bt_ull_sdp_register(ctx->ull_role);
            if (BT_STATUS_SUCCESS != status) {
                ull_report_error("[ULL]bt ultra low latency init sdp fail: 0x%x", 1, status);
                return BT_STATUS_SUCCESS;
            }

            if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
                /* dongle side do nothing, reconnect will be done by app/cm */
                ctx->dl_latency = BT_ULL_DOWNLINK_LIMIATTION;
            } else if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
                if (AUD_ID_INVALID != ctx->audio_id) {
                    bt_sink_srv_ami_audio_close(ctx->audio_id);
                    ctx->audio_id = AUD_ID_INVALID;
                }
                bt_ull_set_ULL_mode(false);
                ctx->audio_id = bt_sink_srv_ami_audio_open(AUD_MIDDLE, bt_ull_am_callback);
                ull_report("[ULL]bt_ull_init audio_id: 0x%x", 1, ctx->audio_id);
                bt_utils_assert(AUD_ID_INVALID != ctx->audio_id);
                ctx->dl_speaker.volume.vol_left = AUD_VOL_OUT_LEVEL10;  //TODO: need store in NVDM?
            } else {
                bt_utils_assert(0 && "unknown ull role");
            }
            break;
        }
        case BT_CM_PROFILE_SERVICE_HANDLE_POWER_OFF: {
            ctx->is_ull_connected = false;
            bt_ull_clear_last_connected_device_info();
            break;
        }
        case BT_CM_PROFILE_SERVICE_HANDLE_CONNECT: {
             address = (uint8_t *)data;
             bt_utils_assert(address);
             bt_timer_ext_stop(BT_ULL_CONFLICT_RECONNECT_TIMER_ID);
             if (false == ctx->is_ull_connected) {
                 status = bt_ull_connect(&(ctx->spp_handle), (const bt_bd_addr_t *)address);
             } else {
                 ull_report("[ULL] ull profile was connected, spp handle: 0x%x, skip connect request", 1, ctx->spp_handle);
                 status = BT_STATUS_FAIL;
             }
             break;
         }
         case BT_CM_PROFILE_SERVICE_HANDLE_DISCONNECT: {
             address = (uint8_t *)data;
             bt_utils_assert(address);
             bt_timer_ext_stop(BT_ULL_CONFLICT_RECONNECT_TIMER_ID);
             uint32_t ull_handle = bt_ull_get_handle_by_address((const bt_bd_addr_t *)address);
             ull_report("[ULL] disconnect ultra low latency, handle: 0x%x", 1, ull_handle); 
             if (ull_handle) {
                 status = bt_ull_disconnect(ull_handle);
             } else {
                status = BT_STATUS_FAIL;
             }
             break;
         }
         default:
             break;
     }
     return status;
}

static bt_status_t bt_ull_unmix_handle(bt_ull_streaming_if_info_t *ep)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    ull_report("[ULL] server unmix handle id: 0x%x, dl_speaker.is_transmitter_start: 0x%x", 2, ep->transmitter, ep->is_transmitter_start);
    /* close ep transmitter */
   ep->is_request_transmitter_start = false;
   if (AUD_ID_INVALID != ep->transmitter) {
      if (ep->is_transmitter_start) {
            /* unmix before transmitter stop */
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ep->transmitter, GAMING_MODE_CONFIG_OP_MUSIC_UNMIX, NULL)) {
                ull_report_error("[ULL][API] unmix fail, trans_id:0x%x", 1, ep->transmitter);
                status = BT_STATUS_FAIL;
            }
            bt_ull_stop_transmitter(ep);
      }
   }
   return status;
}

bt_status_t bt_ull_sync_connection_handle(uint32_t handle, bt_ull_role_t ull_role)
{
  bt_ull_req_t request;
  bt_status_t status = BT_STATUS_SUCCESS;
  bt_ull_context_t *ctx = bt_ull_get_context();

  /* exchange downlink codec info */
  memset(&request, 0x00, sizeof(request));
  request.event = BT_ULL_EVENT_CONNECTION_INFO;
  bt_ull_connection_info_t *connection_info = &(request.connection_info);

  if (BT_ULL_ROLE_SERVER == ull_role) {
     /* set uplink frame size */
     connection_info->uplink_info.ul_frame_size = bt_ull_get_mic_frame_size();
     ull_report("[ULL] server sync uplink_frame_size in connection handle %d", 1,connection_info->uplink_info.ul_frame_size);
     /* set server speaker info */
     connection_info->server_speaker_dl.codec_type = BT_ULL_OPUS_CODEC;
     connection_info->server_speaker_dl.sample_rate = ctx->dl_speaker.sample_rate;
     connection_info->server_speaker_dl.is_mute = ctx->dl_speaker.is_mute;
     memcpy(&(connection_info->server_speaker_dl.vol), &(ctx->dl_speaker.original_volume), sizeof(bt_ull_original_duel_volume_t));
     memcpy(&(connection_info->server_chat_volume), &(ctx->dl_chat.original_volume), sizeof(bt_ull_original_duel_volume_t));

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
     memcpy(&(connection_info->server_linein_volume), &(ctx->dl_linein.original_volume), sizeof(bt_ull_original_duel_volume_t));
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
     memcpy(&(connection_info->server_linei2s_volume), &(ctx->dl_linei2s.original_volume), sizeof(bt_ull_original_duel_volume_t));
#endif

     if (ctx->dl_speaker.is_streaming || ctx->dl_chat.is_streaming
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
         || ctx->dl_linein.is_streaming
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
         || ctx->dl_linei2s.is_streaming
#endif
        ) {
         connection_info->server_speaker_dl.is_streaming = true;
     } else {
         connection_info->server_speaker_dl.is_streaming = false;
     }
     /* set server microphone info */
     connection_info->server_mic_ul.codec_type = BT_ULL_OPUS_CODEC;
     connection_info->server_mic_ul.sample_rate = ctx->ul_microphone.sample_rate;;
     connection_info->server_mic_ul.is_mute = ctx->ul_microphone.is_mute;
     memcpy(&(connection_info->server_mic_ul.vol), &(ctx->ul_microphone.original_volume), sizeof(bt_ull_original_duel_volume_t));
     connection_info->server_mic_ul.is_streaming = ctx->ul_microphone.is_streaming;
     ull_report("[ULL] server speaker is_streaming: 0x%x, is_mute:0x%x, microphone is_streaming: 0x%x, is_mute:0x%x", 4,
                connection_info->server_speaker_dl.is_streaming, connection_info->server_speaker_dl.is_mute, connection_info->server_mic_ul.is_streaming, connection_info->server_mic_ul.is_mute);
    } else {
      /* sync headset mix ratio setting to dongle */
      memcpy(&(connection_info->mix_ratio), &(ctx->dl_mix_ratio), sizeof(bt_ull_mix_ratio_t));
      /* sync latency to dongle */
      connection_info->dl_latency = ctx->dl_latency;
      connection_info->ul_latency = ctx->ul_latency;
#ifdef MTK_AWS_MCE_ENABLE
      connection_info->client_type = BT_ULL_EARBUDS_CLIENT;
#else
      connection_info->client_type = BT_ULL_HEADSET_CLIENT;
#endif
      /* 1. set share buffer address to controller */
      ctx->am_handle = bt_ull_am_init();
      bt_utils_assert(NULL != ctx->am_handle && "fail create pseudo device!");

      bt_role_t role;
      bt_gap_connection_handle_t gap_hd = bt_gap_get_handle_by_address((const bt_bd_addr_t *) & (ctx->bt_addr));
      /* client should be slave role */
      if ((bt_gap_get_role_sync(gap_hd, &role) == BT_STATUS_SUCCESS) && (role == BT_ROLE_SLAVE)) {
          const audio_src_srv_handle_t *running = audio_src_srv_get_runing_pseudo_device();
          if (running && (running->priority > ctx->am_handle->priority)) {
              ull_report("[ULL] running priority: 0x%x ,am_handle priority: 0x%x", 
                         2, running->priority, ctx->am_handle->priority);
              ctx->allow_play = BT_ULL_PLAY_DISALLOW;
          } else {
              ctx->allow_play = BT_ULL_PLAY_ALLOW;
          }
      } else {
          ctx->allow_play = BT_ULL_PLAY_DISALLOW;
      }
      connection_info->allow_play = ctx->allow_play;
      ull_report("[ULL] sync headset/earbuds is allow_play: 0x%x, client type: 0x%x, dl_latency:%d, ul_latency:%d, role:0x%x", 5,
                 ctx->allow_play, connection_info->client_type, ctx->dl_latency, ctx->ul_latency, role);
    }
   bt_ull_send_data(handle, (uint8_t *)&request, sizeof(request));
   return status;
}

bt_status_t bt_ull_connection_handle(uint32_t handle, bt_ull_connection_state_t state)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_context_t *ctx = bt_ull_get_context();

    if (BT_ULL_DISCONNECTED == state) {
        /* enable page scan after ull disconnect */
        bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
        /* release auido source && close codec */
        if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
            /* ull profile disconnected we shuld enable page scan */
            ull_report("[ULL] client voice id: 0x%x, ul_microphone.is_transmitter_start: 0x%x", 2, ctx->ul_microphone.transmitter, ctx->ul_microphone.is_transmitter_start);
            /* close transmitter */
            ctx->ul_microphone.is_request_transmitter_start = false;
            if (ctx->ul_microphone.is_transmitter_start) {
                /* stop transmitter & disable sidetone */
                am_audio_side_tone_disable();
                bt_ull_stop_transmitter(&ctx->ul_microphone);
            }
            /* clear last connection info */
            bt_ull_clear_last_connected_device_info();
            bt_ull_am_deinit();
        } else if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
            /* fdcb notify controller enable, controller will be clear music downlink avm buffer */
            uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
            bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, false, ctx->dl_latency);
            /* restore latency to default */
            ctx->dl_latency = BT_ULL_DOWNLINK_LIMIATTION;
            ctx->allow_play = BT_ULL_PLAY_ALLOW;
            /* enable page scan after ull disconnect */
            bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);

            //ull_report("[ULL] server voice id: 0x%x, ul_microphone.is_transmitter_start: 0x%x", 2, ctx->ul_microphone.transmitter, ctx->ul_microphone.is_transmitter_start);
            bt_ull_stop_transmitter(&ctx->ul_microphone);

            /* unmix before speaker transmitter stop */
            bt_ull_unmix_handle(&ctx->dl_speaker);
            /* unmix before chat transmitter stop */
            bt_ull_unmix_handle(&ctx->dl_chat);

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
            /* unmix before  line-in transmitter stop */
            bt_ull_unmix_handle(&ctx->dl_linein);
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
            /* unmix before  i2s-in transmitter stop */
            bt_ull_unmix_handle(&ctx->dl_linei2s);
#endif
            if (ctx->is_disable_sniff) {
                ctx->is_disable_sniff = false;
                bt_gap_link_policy_setting_t link_policy;
                link_policy.sniff_mode = BT_GAP_LINK_POLICY_ENABLE;
                bt_gap_write_link_policy(gap_handle, &link_policy);
            }
        }
    } else if (BT_ULL_CONNECTED == state) {
        /* reset critical data seq */
        ctx->critical_data_tx_seq = 0x01;
        ctx->critical_data_rx_seq = 0x00;
        if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
#ifdef AIR_USB_ENABLE
            /* clear mic voice data info */
            g_ull_voice_data_offset = 0;
            voice_data = NULL;
            voice_data_length = 0;
            joint_buffer_offset = 0;
            is_debug_mode_enable = false;
#ifdef AIR_ECNR_POST_PART_ENABLE
            g_ull_wait_dsp_decode_len = 0;
            is_dsp_pre_decode_done = false;
#endif
#endif
            /* ull profile connected we shuld disable page scan && inquiry scan due to bandwidth limiation */
            bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_DISABLE);
            /* server disable sniff mode after ull link connected */
#ifndef AIR_BT_ULTRA_LOW_LATENCY_IDLE_SNIFF_ENABLE
            ctx->is_disable_sniff = true;
            bt_gap_link_policy_setting_t link_policy;
            link_policy.sniff_mode = BT_GAP_LINK_POLICY_DISABLE;
            uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
            bt_utils_assert(gap_handle && "gap handle is null");
            bt_gap_write_link_policy(gap_handle, &link_policy);
#endif
        } else {
            if (ctx->dl_latency <= BT_ULL_DOWNLINK_LIMIATTION_MULTILINK) {
                /* ull profile connected we shuld disable page scan && inquiry scan due to bandwidth limiation */
                bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_DISABLE);
            }
        }
        if (ctx->is_ull_version_ready) {
           bt_ull_sync_connection_handle(handle, ctx->ull_role);
        }
    } else {
        bt_utils_assert(0 && "unknown connection state");
    }
    return status;
}


static void bt_ull_reconfig_transmitter(bt_ull_transmitter_t transmitter_type)
{
   bt_ull_streaming_if_info_t *p_streaming = NULL;
   bt_ull_context_t *ctx = bt_ull_get_context();

   if (BT_ULL_GAMING_TRANSMITTER == transmitter_type) {
     p_streaming = &(ctx->dl_speaker);
   } else if (BT_ULL_CHAT_TRANSMITTER == transmitter_type) {
     p_streaming = &(ctx->dl_chat);
   } else if (BT_ULL_MIC_TRANSMITTER == transmitter_type) {
     p_streaming = &(ctx->ul_microphone);
   }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
   else if (BT_ULL_LINE_IN_TRANSMITTER == transmitter_type) {
     p_streaming = &(ctx->dl_linein);
   }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
   else if (BT_ULL_LINE_OUT_TRANSMITTER == transmitter_type) {
     p_streaming = &(ctx->ul_lineout);
   }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
   else if (BT_ULL_I2S_IN_TRANSMITTER == transmitter_type) {
     p_streaming = &(ctx->dl_linei2s);
  }
#endif
   else {
     bt_utils_assert(0 && "unsupport transmitter type");
   }
   if (p_streaming->is_streaming) {
        if (AUD_ID_INVALID == p_streaming->transmitter) {
            bt_ull_init_transimter(ctx->codec_type, transmitter_type);
        }
        if (AUD_ID_INVALID != p_streaming->transmitter
            && !p_streaming->is_transmitter_start
            && (BT_ULL_PLAY_ALLOW == ctx->allow_play)) {
            bt_ull_start_transmitter(p_streaming);
            uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
            bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, true, ctx->dl_latency);
        }
   }
}


#ifdef MTK_AWS_MCE_ENABLE
static void bt_ull_sync_agent_info_to_partner(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_aws_mce_eir_info_t eir_info = {0};

    eir_info.event = BT_ULL_AWS_MCE_EVT_CONNECTION;
    memcpy(&(eir_info.param.aws_connection.remode_device), &(ctx->remode_device), sizeof(ctx->remode_device));
    /* sync dl speaker info to partner */
    eir_info.param.aws_connection.dl_streaming.volume.vol_left = ctx->dl_speaker.original_volume.vol_left;
    eir_info.param.aws_connection.dl_streaming.volume.vol_right = ctx->dl_speaker.original_volume.vol_right;
    eir_info.param.aws_connection.dl_streaming.is_mute = ctx->dl_speaker.is_mute;
    eir_info.param.aws_connection.dl_streaming.is_streaming = ctx->dl_speaker.is_streaming;
    /* sync ul mic info to partner */
    eir_info.param.aws_connection.ul_streaming.volume.vol_left = ctx->ul_microphone.original_volume.vol_left;
    eir_info.param.aws_connection.ul_streaming.volume.vol_right = ctx->ul_microphone.original_volume.vol_right;
    eir_info.param.aws_connection.ul_streaming.is_mute = ctx->ul_microphone.is_mute;
    eir_info.param.aws_connection.ul_streaming.is_streaming = ctx->ul_microphone.is_streaming;
    /* common info */
    eir_info.param.aws_connection.is_ull_connected = ctx->is_ull_connected;
    eir_info.param.aws_connection.codec_type = ctx->codec_type;
    eir_info.param.aws_connection.dl_latency = ctx->dl_latency;
    eir_info.param.aws_connection.ul_latency = ctx->ul_latency;
    eir_info.param.aws_connection.ul_frame_size = bt_ull_get_mic_frame_size();

    bt_ull_aws_mce_send_eir(&eir_info, sizeof(eir_info), true);
    ull_report("[ULL][AWS] agent sync coinnection info, vol_lev: %d, is_mute:0x%x, is_streaming:0x%x, dl_latency:%d, ul_latency:%d", 5,
               ctx->dl_speaker.volume.vol_left, ctx->dl_speaker.is_mute, ctx->dl_speaker.is_streaming, ctx->dl_latency, ctx->ul_latency);
}
#endif

void bt_ull_handle_volume_add(bt_ull_streaming_if_info_t *ep, uint8_t volume)
{
    ep->volume.vol_left += volume;
    ep->volume.vol_left = (ep->volume.vol_left > AUD_VOL_OUT_LEVEL15) ? AUD_VOL_OUT_LEVEL15 : ep->volume.vol_left;
    ep->volume.vol_right += volume;
    ep->volume.vol_right = (ep->volume.vol_right > AUD_VOL_OUT_LEVEL15) ? AUD_VOL_OUT_LEVEL15 : ep->volume.vol_right;
}

void bt_ull_handle_volume_sub(bt_ull_streaming_if_info_t *ep, uint8_t volume)
{
    if (ep->volume.vol_left > AUD_VOL_OUT_LEVEL0) {
        if ((ep->volume.vol_left - AUD_VOL_OUT_LEVEL0) > volume) {
            ep->volume.vol_left -= volume;
        } else {
            ep->volume.vol_left = AUD_VOL_OUT_LEVEL0;
        }
    }
    if (ep->volume.vol_right > AUD_VOL_OUT_LEVEL0) {
        if ((ep->volume.vol_right - AUD_VOL_OUT_LEVEL0) > volume) {
            ep->volume.vol_right -= volume;
        } else {
            ep->volume.vol_right = AUD_VOL_OUT_LEVEL0;
        }
    }
}

#if 1	// richard for ab1571d command processing
extern void ab1571d_data_processing(uint8_t temp_command_no,uint8_t temp_command_data);
#endif
void bt_ull_rx_data_handle(uint8_t *data, uint16_t length)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (data && length) {
        bt_ull_req_t *req_data = (bt_ull_req_t *)data;
        ull_report("[ULL] rx recieved request event: 0x%x, role: 0x%x", 2, req_data->event, ctx->ull_role);
        switch (req_data->event) {
            case BT_ULL_EVENT_CONNECTION_INFO: {
                bt_ull_connection_info_t *connection_info = &(req_data->connection_info);
                if (NULL != connection_info) {
                    ctx->is_ull_connected = true;
                    ctx->codec_type = BT_ULL_OPUS_CODEC;  /* default always using OPUS codec */

                    if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
                        ull_report("[ULL] client type: 0x%x, allow_play: 0x%x, dl_latency: %d, ul_latency: %d", 4,
                                   connection_info->client_type, connection_info->allow_play, connection_info->dl_latency, connection_info->ul_latency);
                        /* notify controller media data share buffer info */
                        //bt_ull_update_audio_buffer(ctx->ull_role, connection_info->client_type);
                        ctx->dl_latency = connection_info->dl_latency;
                        ctx->ul_latency = connection_info->ul_latency;
                        ctx->allow_play = connection_info->allow_play;
                        /* store mix ratio setting from client */
                        memcpy(&(ctx->dl_mix_ratio), &(connection_info->mix_ratio), sizeof(bt_ull_mix_ratio_t));
                        uint8_t idx = 0;
                        for (idx = 0; idx < BT_ULL_MAX_STREAMING_NUM; idx++) {
                            ull_report("[ULL] Service rx recieved, streaming[%d] type:0x%x, port:0x%x, ratio: %d", 4,
                                       idx, ctx->dl_mix_ratio.streamings[idx].streaming.streaming_interface, ctx->dl_mix_ratio.streamings[idx].streaming.port, ctx->dl_mix_ratio.streamings[idx].ratio);
                        }
                        if (ctx->dl_speaker.is_streaming
                            || ctx->dl_chat.is_streaming
                            || ctx->ul_microphone.is_streaming
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                            || ctx->dl_linein.is_streaming
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                            || ctx->dl_linei2s.is_streaming
#endif
                        ){
#ifdef AIR_BT_ULTRA_LOW_LATENCY_IDLE_SNIFF_ENABLE
                            /* disable sniff mode during one of ep streaming start */
                            if (false == ctx->is_disable_sniff) {
                                ctx->is_disable_sniff = true;
                                uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
                                bt_utils_assert(gap_handle && "gap handle is null");
                                bt_gap_link_policy_setting_t link_policy;
                                link_policy.sniff_mode = BT_GAP_LINK_POLICY_DISABLE;
                                bt_gap_write_link_policy(gap_handle, &link_policy);
                            }
#endif
                        }
                    } else {
                        ull_report("[ULL] speaker is streaming: 0x%x, is_mute:0x%x, microphone is streaming:0x%x, is_mute: 0x%x", 4,
                                   connection_info->server_speaker_dl.is_streaming, connection_info->server_speaker_dl.is_mute, connection_info->server_mic_ul.is_streaming, connection_info->server_mic_ul.is_mute);
                        memset(&(ctx->codec_cap), 0x00, sizeof(bt_a2dp_codec_capability_t));
                        ctx->codec_cap.type = BT_ULL_CODEC_AIRO_CELT;
                        ctx->codec_cap.sep_type = BT_A2DP_SINK;
                        /* store usb Host speaker && micrphone info in client */
                        ctx->dl_speaker.sample_rate = connection_info->server_speaker_dl.sample_rate;
                        ctx->dl_speaker.is_mute = connection_info->server_speaker_dl.is_mute;
                        ctx->dl_speaker.is_streaming = connection_info->server_speaker_dl.is_streaming;
                        memcpy(&(ctx->dl_speaker.original_volume), &(connection_info->server_speaker_dl.vol), sizeof(bt_ull_original_duel_volume_t));

                        ctx->ul_microphone.sample_rate = connection_info->server_mic_ul.sample_rate;
                        ctx->ul_microphone.is_mute = connection_info->server_mic_ul.is_mute;
                        ctx->ul_microphone.is_streaming = connection_info->server_mic_ul.is_streaming;
                        memcpy(&(ctx->ul_microphone.original_volume), &(connection_info->server_mic_ul.vol), sizeof(bt_ull_original_duel_volume_t));
                        memcpy(&(ctx->dl_chat.original_volume), &(connection_info->server_chat_volume), sizeof(bt_ull_original_duel_volume_t));

                        bt_ull_set_mic_frame_size(connection_info->uplink_info.ul_frame_size);
#ifdef MTK_AWS_MCE_ENABLE
                        bt_ull_update_audio_buffer(ctx->ull_role, BT_ULL_EARBUDS_CLIENT);
#else
                        bt_ull_update_audio_buffer(ctx->ull_role, BT_ULL_HEADSET_CLIENT);
#endif
#ifdef MTK_AWS_MCE_ENABLE
                        if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()) {
                            bt_bd_addr_t aws_device;
                            if (bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), &aws_device, 1)) {
                                if (!memcmp(&aws_device, &(ctx->bt_addr), BT_BD_ADDR_LEN)) {
                                    ull_report("[ULL][CM_CB] partner attached in normal link! is_ull_connected:0x%x", 1, ctx->is_ull_connected);
                                    bt_ull_sync_agent_info_to_partner();
                                } else {
                                    ull_report("[ULL][CM_CB] partner attached not in ull link !!!", 0);
                                    ull_report("[ULL][CM_CB] aws_device->address:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", 6,
                                               aws_device[0], aws_device[1], aws_device[2], aws_device[3], aws_device[4], aws_device[5]);
                                    ull_report("[ULL][CM_CB] ctx->bt_addr:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", 6,
                                               ctx->bt_addr[0], ctx->bt_addr[1], ctx->bt_addr[2], ctx->bt_addr[3], ctx->bt_addr[4], ctx->bt_addr[5]);
                                }
                            }
                        }
#endif
                        /* for client only need init MIC transmitter with fix sample rate */
                        bt_ull_init_transimter(ctx->codec_type, BT_ULL_MIC_TRANSMITTER);
                        /* play if dongle is playing */
                        if (ctx->dl_speaker.is_streaming || ctx->ul_microphone.is_streaming) {
                            bt_ull_am_play();
                        }
                    }
                }
                break;
            }
            case BT_ULL_EVENT_STREAMING_START_IND: {
                bt_ull_streaming_t *steaming = (bt_ull_streaming_t *)(&(req_data->streaming_port));
                ull_report("[ULL] rx recieved BT_ULL_EVENT_STREAMING_START_IND, if_type: 0x%x, port:0x%x", 2, steaming->streaming_interface, steaming->port);
                if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
                    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == steaming->streaming_interface) {
                        ctx->dl_speaker.is_streaming = true;
                        bt_ull_am_play();
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                    } else if (BT_ULL_STREAMING_INTERFACE_LINE_IN == steaming->streaming_interface) {
                        ctx->dl_linein.is_streaming = true;
                        bt_ull_am_play();
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                    } else if (BT_ULL_STREAMING_INTERFACE_I2S_IN == steaming->streaming_interface) {
                        ctx->dl_linei2s.is_streaming = true;
                        bt_ull_am_play();
#endif
                    } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == steaming->streaming_interface 
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
                            || BT_ULL_STREAMING_INTERFACE_LINE_OUT == steaming->streaming_interface
#endif
                    ){
                       if(false == ctx->ul_microphone.is_streaming) {
                           ctx->ul_microphone.is_streaming = true;

#ifdef BT_SINK_DUAL_ANT_ENABLE
                           bt_sink_srv_dual_ant_context_t *dual_ant_ctx = bt_sink_srv_dual_ant_get_context();
                           if (dual_ant_ctx->call_info.esco_state) {
                               ull_report("[ULL] current esco is ongoing, ull mic should waiting! ", 0);
                               break;
                           }
#endif
                           /* enable mic if ull is playing */
                           if (BT_ULL_AM_PLAYING == ctx->am_state) {
                               /* start voice path */
                               if(ctx->ul_microphone.resource_handle != NULL) {
                                  audio_src_srv_resource_manager_take(ctx->ul_microphone.resource_handle);
                               }
                           } else {
                               bt_ull_am_play();
                           }
                       }
                    }
                }
                break;
            }
            case BT_ULL_EVENT_STREAMING_STOP_IND: {
                bt_ull_streaming_t *steaming = (bt_ull_streaming_t *)(&(req_data->streaming_port));
                ull_report("[ULL] rx recieved BT_ULL_EVENT_STREAMING_STOP_IND, if_type: 0x%x, port:0x%x", 2, steaming->streaming_interface, steaming->port);
                if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
                    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == steaming->streaming_interface) {
                        ctx->dl_speaker.is_streaming = false;
                        if ((false == ctx->ul_microphone.is_streaming) 
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                         && (false == ctx->dl_speaker.is_streaming)
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                         && (false == ctx->dl_linei2s.is_streaming)
#endif
                    ) {
                         bt_ull_am_stop();
                      }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                    } else if (BT_ULL_STREAMING_INTERFACE_LINE_IN == steaming->streaming_interface) {
                        ctx->dl_linein.is_streaming = false;
                        if ((false == ctx->ul_microphone.is_streaming) && (false == ctx->dl_speaker.is_streaming )
              #ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                         && (false == ctx->dl_linei2s.is_streaming)
              #endif
                    ) {
                         bt_ull_am_stop();
                      }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                    } else if (BT_ULL_STREAMING_INTERFACE_I2S_IN == steaming->streaming_interface) {
                        ctx->dl_linei2s.is_streaming = false;
                        if ((false == ctx->ul_microphone.is_streaming) && (false == ctx->dl_speaker.is_streaming )
              #ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                         && (false == ctx->dl_linein.is_streaming)
              #endif
                    ) {
                         bt_ull_am_stop();
                      }
#endif

                    } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == steaming->streaming_interface
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
                            || BT_ULL_STREAMING_INTERFACE_LINE_OUT == steaming->streaming_interface
#endif
                        ){
                        if(ctx->ul_microphone.is_streaming == true) {
                            ctx->ul_microphone.is_streaming = false;
                            if(false == ctx->ul_microphone.is_streaming) {
                                if (false == ctx->dl_speaker.is_streaming) {
                                    bt_ull_am_stop();
                                } else {
                                    /* stop voice audio transmitter */
                                    am_audio_side_tone_disable();
                                    bt_ull_stop_transmitter(&ctx->ul_microphone);
                                }
                            }
                        }
                    }
                }
                break;
            }
            case BT_ULL_EVENT_VOLUME_IND: {
                bt_ull_volume_ind_t *p_vol = (bt_ull_volume_ind_t *)(&(req_data->vol_ind));
                bt_ull_streaming_if_info_t *stream_prt = NULL;

                ull_report("[ULL] rx recieved BT_ULL_EVENT_VOLUME_IND, if_type: 0x%x, port: 0x%x, vol_left:%d, vol_right:%d", 4,
                           p_vol->streaming.streaming_interface, p_vol->streaming.port, p_vol->vol.vol_left, p_vol->vol.vol_right);
                if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
                    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == p_vol->streaming.streaming_interface) {
                        if (0x00 == p_vol->streaming.port) {
                        stream_prt = &ctx->dl_speaker;
                        } else if (0x01 == p_vol->streaming.port) {
                        stream_prt = &ctx->dl_chat;
                        }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                    } else if (BT_ULL_STREAMING_INTERFACE_LINE_IN == p_vol->streaming.streaming_interface) {
                        stream_prt = &ctx->dl_linein;
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                    } else if (BT_ULL_STREAMING_INTERFACE_I2S_IN == p_vol->streaming.streaming_interface) {
                        stream_prt = &ctx->dl_linei2s;
#endif
                    } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == p_vol->streaming.streaming_interface) {
                        stream_prt = &ctx->ul_microphone;
                    } else {
                        bt_utils_assert(0 && "unknown streaming interface");
                    }
                    if(stream_prt != NULL) {
                      stream_prt->original_volume.vol_left = p_vol->vol.vol_left;
                      stream_prt->original_volume.vol_right = p_vol->vol.vol_right;
                    }
                }
                break;
            }
            case BT_ULL_EVENT_VOLUME_MUTE: {
                bt_ull_streaming_t *steaming = (bt_ull_streaming_t *)(&(req_data->streaming_port));
                ull_report("[ULL] rx recieved BT_ULL_EVENT_VOLUME_MUTE, if_type: 0x%x, port:0x%x", 2, steaming->streaming_interface, steaming->port);
                if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
                    /* dongle mute */
                    bt_ull_handle_set_streaming_mute(steaming, true);
                }
#if 0
                bt_ull_am_set_mute(true);
#ifdef MTK_AWS_MCE_ENABLE
                if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()) {
                    eir_info.event = BT_ULL_AWS_MCE_EVT_VOL_SYNC;
                    eir_info.param.aws_vol_sync.vol_lev = ctx->dl_speaker.volume.vol_left;
                    eir_info.param.aws_vol_sync.is_mute = ctx->dl_speaker.is_mute;
                    bt_ull_aws_mce_send_eir(&eir_info, sizeof(eir_info), false);
                    ull_report("[ULL][AWS] agent sync streaming mute, vol_lev: %d, is_mute:0x%x", 2,
                               ctx->dl_speaker.volume.vol_left, ctx->dl_speaker.is_mute);
                }
#endif
#endif
                break;
            }
            case BT_ULL_EVENT_VOLUME_UNMUTE: {
                bt_ull_streaming_t *steaming = (bt_ull_streaming_t *)(&(req_data->streaming_port));
                ull_report("[ULL] rx recieved BT_ULL_EVENT_VOLUME_UNMUTE, if_type: 0x%x, port:0x%x", 2, steaming->streaming_interface, steaming->port);
                if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
                    /* dongle un-mute */
                    bt_ull_handle_set_streaming_mute(steaming, false);
                }
#if 0
                bt_ull_am_set_mute(false);
#ifdef MTK_AWS_MCE_ENABLE
                if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()) {
                    eir_info.event = BT_ULL_AWS_MCE_EVT_VOL_SYNC;
                    eir_info.param.aws_vol_sync.vol_lev = ctx->dl_speaker.volume.vol_left;
                    eir_info.param.aws_vol_sync.is_mute = ctx->dl_speaker.is_mute;
                    bt_ull_aws_mce_send_eir(&eir_info, sizeof(eir_info), false);
                    ull_report("[ULL][AWS] agent sync streaming un-mute, vol_lev: %d, is_mute:0x%x", 2,
                               ctx->dl_speaker.volume.vol_left, ctx->dl_speaker.is_mute);
                }
#endif
#endif
                break;
            }
            /* server reply switch latency, client restart codec */
            case BT_ULL_EVENT_LATENCY_SWITCH_COMPLETE: {
                bt_ull_latency_t *latency = (bt_ull_latency_t *)(&(req_data->latency));
                if (latency) {
                    ull_report("[ULL] rx recieved BT_ULL_EVENT_LATENCY_SWITCH_COMPLETE, latency: %d", 1, latency->latency);
                    ctx->dl_latency = latency->latency;
                    if (ctx->dl_latency > BT_ULL_DOWNLINK_SINGLE_LIMITATION) {
                        ctx->ul_latency = BT_ULL_UPLINK_MAX;
                    } else {
                        ctx->ul_latency = BT_ULL_UPLINK_DEFAULT;
                    }
                    if (ctx->is_ull_connected) {
                        if (BT_ULL_AM_PLAYING == ctx->am_state
                            || BT_ULL_SUB_STATE_PREPARE_CODEC == ctx->am_substate) {
                            /* restart ull codec, if we are playing ull to swith latency */
                            bt_ull_am_restart();
                        }
                    }
                }
                break;
            }

            case BT_ULL_EVENT_LATENCY_SWITCH: {
                bt_ull_latency_t *req_latency = (bt_ull_latency_t *)(&(req_data->latency));
                if (req_latency) {
                    bt_ull_handle_set_streaming_latency(req_latency);
                }
                break;
            }
            case BT_ULL_EVENT_ALLOW_PLAY_REPORT: {
                bt_ull_allow_play_report_t *report = (bt_ull_allow_play_report_t *)(&(req_data->allow_play_report));
                if (report) {
                    ull_report("[ULL] dongle rx recieved allow_play : 0x%x, current is_allow: 0x%x", 2, report->allow_play, ctx->allow_play);
                    if (report->allow_play != ctx->allow_play) {
                        ctx->allow_play = report->allow_play;
                        if (BT_ULL_PLAY_DISALLOW == ctx->allow_play) {
                            /* fdcb notify controller enable, controller will be clear music downlink avm buffer */
                            uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
                            bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, false, ctx->dl_latency);
                            
                            /* stop gaming transmitter */
                            bt_ull_unmix_handle(&ctx->dl_speaker);
                            /* stop chat transmitter */
                            bt_ull_unmix_handle(&ctx->dl_chat);
                            /* stop voice transmitter */
                            bt_ull_stop_transmitter(&ctx->ul_microphone);
                        } else {
                            /* start gaming transmitter */
                            if (ctx->dl_speaker.is_streaming) {
                                bt_ull_start_transmitter(&ctx->dl_speaker);
                            }
                            /* start chat transmitter */
                            if (ctx->dl_chat.is_streaming) {
                                bt_ull_start_transmitter(&ctx->dl_chat);
                            }
                            /* start voice transmitter */
                            if (ctx->ul_microphone.is_streaming) {
                                if (ctx->ul_microphone.resource_handle != NULL) {
                                    audio_src_srv_resource_manager_take(ctx->ul_microphone.resource_handle);
                                }
                            }
                            /* enable controller */
                            uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
                            bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, true, ctx->dl_latency);
                        }
                    }
                }
                break;
            }
            /* dongle receive headset volume operation */
            case BT_ULL_EVENT_VOLUME_ACTION: {
                bt_ull_volume_t *vol = (bt_ull_volume_t *)(&(req_data->vol_action));
                ull_report("[ULL][API] dongle rx recieved VOLUME_REPORT, type:0x%x, volume action: 0x%x, volume_level: %d,", 3,
                           vol->streaming.streaming_interface, vol->action, vol->volume);
#if defined(AIR_USB_ENABLE) && defined(AIR_USB_AUDIO_ENABLE) && defined(AIR_USB_HID_ENABLE)
                if (BT_ULL_STREAMING_INTERFACE_SPEAKER == vol->streaming.streaming_interface) {
                    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
                        ull_report("[ULL] usb_hid_control, current is XBOX mode, ignore the HID control event", 0);
                    } else {
                        if (BT_ULL_VOLUME_ACTION_SET_UP == vol->action) {
                            USB_HID_VolumeUp(vol->volume);
                        } else if (BT_ULL_VOLUME_ACTION_SET_DOWN == vol->action) {
                            USB_HID_VolumeDown(vol->volume);
                        }
                    }
                } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == vol->streaming.streaming_interface
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
                      || BT_ULL_STREAMING_INTERFACE_LINE_OUT == vol->streaming.streaming_interface
#endif
                ){
                    bt_ull_handle_set_volume(vol);
                }
#else
                bt_ull_streaming_if_info_t *ep = NULL;
                bt_ull_streaming_if_info_t *chat_ep = NULL;
                bt_sink_srv_am_volume_level_out_t chat_max_vol = AUD_VOL_OUT_LEVEL0;
                bt_sink_srv_am_volume_level_out_t max_vol = AUD_VOL_OUT_LEVEL0;
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                bt_ull_streaming_if_info_t *linein_ep = NULL;
                bt_sink_srv_am_volume_level_out_t linein_max_vol = AUD_VOL_OUT_LEVEL0;
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                bt_ull_streaming_if_info_t *linei2s_ep = NULL;
                bt_sink_srv_am_volume_level_out_t linei2s_max_vol = AUD_VOL_OUT_LEVEL0;
#endif
                if (BT_ULL_STREAMING_INTERFACE_SPEAKER == vol->streaming.streaming_interface) {
                    ep = &(ctx->dl_speaker);
                    chat_ep = &(ctx->dl_chat);
                    chat_max_vol = (chat_ep->volume.vol_left > chat_ep->volume.vol_right) ? chat_ep->volume.vol_left : chat_ep->volume.vol_right;
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                } else if (BT_ULL_STREAMING_INTERFACE_LINE_IN == vol->streaming.streaming_interface) {
                    linein_ep = &(ctx->dl_linein);
                    linein_max_vol = (linein_ep->volume.vol_left > linein_ep->volume.vol_right) ? linein_ep->volume.vol_left : linein_ep->volume.vol_right;
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                } else if (BT_ULL_STREAMING_INTERFACE_I2S_IN == vol->streaming.streaming_interface) {
                    linei2s_ep = &(ctx->dl_linei2s);
                    linei2s_max_vol = (linei2s_ep->volume.vol_left > linei2s_ep->volume.vol_right) ? linei2s_ep->volume.vol_left : linei2s_ep->volume.vol_right;
#endif
                } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == vol->streaming.streaming_interface) {
                    ep = &(ctx->ul_microphone);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
                } else if (BT_ULL_STREAMING_INTERFACE_LINE_OUT == vol->streaming.streaming_interface) {
                    ep = &(ctx->ul_lineout);
#endif
                } else {
                    bt_utils_assert(0 && "unknown usb end pointer type");
                    return;
                }
                max_vol = (ep->volume.vol_left > ep->volume.vol_right) ? ep->volume.vol_left : ep->volume.vol_right;
                ull_report("[ULL][API] dongle current vol_left: %d, vol_right: %d", 2, ep->volume.vol_left, ep->volume.vol_right);
                if (BT_ULL_VOLUME_ACTION_SET_UP == vol->action) {
                    if (chat_max_vol < AUD_VOL_OUT_LEVEL15) {
                        bt_ull_handle_volume_add(chat_ep, vol->volume);
                    }
                    if (max_vol < AUD_VOL_OUT_LEVEL15) {
                        bt_ull_handle_volume_add(ep, vol->volume);
                    }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                    if (linein_max_vol < AUD_VOL_OUT_LEVEL15) {
                        bt_ull_handle_volume_add(linein_ep, vol->volume);
                    }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                    if (linei2s_max_vol < AUD_VOL_OUT_LEVEL15) {
                        bt_ull_handle_volume_add(linei2s_ep, vol->volume);
                    }
#endif
                } else if (BT_ULL_VOLUME_ACTION_SET_DOWN == vol->action) {
                    if (chat_max_vol > AUD_VOL_OUT_LEVEL0) {
                        bt_ull_handle_volume_sub(chat_ep, vol->volume);
                    }
                    if (max_vol > AUD_VOL_OUT_LEVEL0) {
                        bt_ull_handle_volume_sub(ep, vol->volume);
                    }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                    if (linein_max_vol > AUD_VOL_OUT_LEVEL0) {
                        bt_ull_handle_volume_sub(linein_ep, vol->volume);
                    }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                    if (linei2s_max_vol > AUD_VOL_OUT_LEVEL0) {
                        bt_ull_handle_volume_sub(linei2s_ep, vol->volume);
                    }
#endif
                } else if (BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME == vol->action) {
                    if ((vol->volume >= AUD_VOL_OUT_LEVEL0) && (vol->volume <= AUD_VOL_OUT_LEVEL15)) {
                        ep->volume.vol_left = vol->volume;
                        ep->volume.vol_right = vol->volume;
                        chat_ep->volume.vol_right = vol->volume;
                        chat_ep->volume.vol_left = vol->volume;
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                        linein_ep->volume.vol_right = vol->volume;
                        linein_ep->volume.vol_left = vol->volume;
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                        linei2s_ep->volume.vol_right = vol->volume;
                        linei2s_ep->volume.vol_left = vol->volume;
#endif
                    }
                } else {
                    bt_utils_assert(0 && "unknown event type");
                }
                bt_ull_streaming_t streaming;
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                /* set gaming streaming volume */
                streaming.port = 0;
                bt_ull_set_transmitter_volume(&streaming);
                /* set chat streaming volume */
                streaming.port = 1;
                bt_ull_set_transmitter_volume(&streaming);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                /* set line_in streaming volume */
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
                streaming.port = 0;
                bt_ull_set_transmitter_volume(&streaming);
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                /* set line_in streaming volume */
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
                streaming.port = 0;
                bt_ull_set_transmitter_volume(&streaming);
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
                /* set line_out streaming volume */
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
                streaming.port = 0;
                bt_ull_set_transmitter_volume(&streaming);
#endif
#endif
                break;
            }
            case BT_ULL_EVENT_MIX_RATIO_ACTION: {
                bt_ull_mix_ratio_t *mix_ratio = (bt_ull_mix_ratio_t *)(&(req_data->mix_ratio));
                if (mix_ratio) {
                    ull_report("[ULL][API] Server rx recieved MIX_RATIO, streaming[0]: %d, streaming[1]: %d", 2,
                               mix_ratio->streamings[0].ratio, mix_ratio->streamings[1].ratio);
                    bt_ull_handle_set_streaming_mix_ratio(mix_ratio);
                }
                break;
            }

            case BT_ULL_EVENT_USER_DATA: {
                uint8_t *p_rx = &data[1];
                bt_ull_user_data_t user_data_cb;
                memcpy(user_data_cb.remote_address, ctx->bt_addr, sizeof(bt_bd_addr_t));
                memcpy(&(user_data_cb.user_data_length), p_rx, sizeof(user_data_cb.user_data_length));
                p_rx += sizeof(user_data_cb.user_data_length);
                user_data_cb.user_data = p_rx;
#if 1	// richard for ab1571d command processing
			if(data[3]==3)
			{
				ab1571d_data_processing(data[4], data[5]);
			}
#endif				
                bt_ull_event_callback(BT_ULL_EVENT_USER_DATA_IND, &user_data_cb, sizeof(user_data_cb));
                break;
            }

            case BT_ULL_EVENT_USB_HID_CONTROL_ACTION: {
                bt_ull_usb_hid_control_t control_id = req_data->hid_control;
                bt_ull_handle_usb_hid_control(control_id);
                break;
            }
            case BT_ULL_EVENT_UPLINK_INFO: {
               bt_ull_client_t client_type;
               audio_transmitter_status_t status;
               bt_ull_uplink_info_t *p_rx = (bt_ull_uplink_info_t *)(&(req_data->uplink_info));
               bt_ull_set_mic_frame_size(p_rx->ul_frame_size);
#ifdef MTK_AWS_MCE_ENABLE
               client_type = BT_ULL_EARBUDS_CLIENT;
#else
               client_type = BT_ULL_HEADSET_CLIENT;
#endif
               bt_ull_update_audio_buffer(ctx->ull_role, client_type);
               status = audio_transmitter_deinit(ctx->ul_microphone.transmitter);
               if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                   ctx->ul_microphone.transmitter = AUD_ID_INVALID;
                   bt_ull_init_transimter(ctx->codec_type, BT_ULL_MIC_TRANSMITTER);
               }
               break;
            }
            default:
                break;
        }
    } else {
        ull_report_error("[ULL] rx recieved data error!", 0);
    }
}

void bt_ull_event_callback(bt_ull_event_t event, void *param, uint32_t len)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL][CALLBACK][D] bt_ull_event_callback event:0x%x, param len:0x%x", 2, event, len);
    if (ctx && ctx->callback) {
        ctx->callback(event, param, len);
    }
}

bt_ull_context_t *bt_ull_get_context(void)
{
    return &g_bt_ull_context;
}

static bt_status_t bt_ull_cm_event_callback(bt_cm_event_t event_id, void *params, uint32_t params_len)
{
    bt_ull_context_t *ctx = bt_ull_get_context();

    ull_report("[ULL][CM_CB] callback event 0x:%x", 1, event_id);
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)params;
            if (NULL == remote_update) {
                break;
            }
            ull_report("[ULL][CM_CB] callback remote_info_update, connected_service = 0x%x, pre_connected_service = 0x%x, acl_state = 0x%x, pre_acl_state = 0x%x", 4,
                       remote_update->connected_service, remote_update->pre_connected_service, remote_update->acl_state, remote_update->pre_acl_state);
            ull_report("[ULL][CM_CB] remote_update->address:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", 6,
                       remote_update->address[0], remote_update->address[1], remote_update->address[2],
                       remote_update->address[3], remote_update->address[4], remote_update->address[5]);
            /* enable page scan after ull link ACL disconnected */
            if (!memcmp(remote_update->address, ctx->bt_addr, BT_BD_ADDR_LEN)
                && BT_CM_ACL_LINK_DISCONNECTED != remote_update->pre_acl_state
                && BT_CM_ACL_LINK_DISCONNECTED == remote_update->acl_state) {
                /* enable page scan after ull disconnect */
                bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
            }
#ifdef MTK_AWS_MCE_ENABLE
            /* AWS profile connected, Agent sync ull connection state to partner */
            bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
            if (!(remote_update->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))
                && (remote_update->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))) {
                if (BT_AWS_MCE_ROLE_AGENT == role) {
                    bt_bd_addr_t *local_addr = bt_device_manager_get_local_address();
                    if (memcmp(remote_update->address, local_addr, BT_BD_ADDR_LEN)) {
                        ull_report("[ULL][CM_CB] partner attached in normal link! is_ull_connected:0x%x", 1, ctx->is_ull_connected);
                        if (ctx->is_ull_connected && !memcmp(&(remote_update->address), &(ctx->bt_addr), BT_BD_ADDR_LEN)) {
                            bt_ull_sync_agent_info_to_partner();
                        } else {
                            ull_report("[ULL][AWS] partner attached not in ull link !!!", 0);
                            if (ctx->is_ull_connected) {
                                ull_report("[ULL][CM_CB] ctx->bt_addr:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", 6,
                                           ctx->bt_addr[0], ctx->bt_addr[1], ctx->bt_addr[2], ctx->bt_addr[3], ctx->bt_addr[4], ctx->bt_addr[5]);
                                if (BT_ULL_AM_PLAYING == ctx->am_state
                                    || (ctx->flag & BT_ULL_FLAG_WAIT_AM_OPEN_CODEC)) {
                                    bt_cm_connect_t conn_req;
                                    memcpy(&(conn_req.address), &(ctx->bt_addr), sizeof(bt_bd_addr_t));
                                    conn_req.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS);
                                    bt_cm_connect(&conn_req);
                                    ull_report("[ULL] switch aws link", 0);
                                }
                            }
                        }
                    } else {
                        ull_report("[ULL][CM_CB] partner attached in special link! is_ull_connected:0x%x", 1, ctx->is_ull_connected);
                    }
                } else if (BT_AWS_MCE_ROLE_CLINET == role
                           || BT_AWS_MCE_ROLE_PARTNER == role) {
                    /* save aws link address */
                    ull_report("[ULL][CM_CB] partner connected with agent", 0);
                    memcpy(ctx->bt_addr, remote_update->address, BT_BD_ADDR_LEN);
                }
            } else if (!(remote_update->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))
                       && (remote_update->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))) {
                /* aws profile disconnected, partner shuold clear previous context */
                if (BT_AWS_MCE_ROLE_PARTNER == role || BT_AWS_MCE_ROLE_CLINET == role) {
                    bt_ull_aws_mce_handle_connection(false);
                }
            }
#endif
            /* hfp/hsp connected */
            if ((!(remote_update->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP))
                 && (remote_update->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)))
                || (!(remote_update->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HSP))
                    && (remote_update->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HSP)))) {
                if (ctx->is_ull_connected) {
                    const bt_gap_default_sniff_params_t ull_multi_link_sniff = {
                        .max_sniff_interval = 400,
                        .min_sniff_interval = 400,
                        .sniff_attempt = 3,
                        .sniff_timeout = 0
                    };
                    bt_cm_set_sniff_parameters(&ull_multi_link_sniff);
                } else {
                    bt_cm_reset_sniff_parameters();
                }
            }
            break;
        }
        default:
            break;
    }

    return BT_STATUS_SUCCESS;
}

#ifdef AIR_USB_ENABLE
/* ccni notify DSP to prepare decode next frame */
static void bt_ull_ccni_dsp(void)
{
    hal_ccni_message_t ccni_msg = {{0}};
    //ull_report("[ULL][USB] bt_ull_ccni_dsp", 0);
    for (uint32_t i=0; (hal_ccni_set_event(CCNI_CM4_TO_DSP0_BT_COMMON, &ccni_msg)) != HAL_CCNI_STATUS_OK; i++) {
        if ((i % 1000) == 0) {
            ull_report_error("[ULL] Send ccni message waiting %d\r\n", 1, (int)i);
        }
    }
}

#if 0
/*========================================================================================*/
/*                          USB Audio evet callback                                                                                                                                      */
/*========================================================================================*/
/*
Parameter: bInterfaceNumber (default 1 for audio)
      bAlternateSetting (0: disable, 1: enable)
Stage: Task level
Period: trigger once
Description: PC enable/disable audio
*/
static void bt_ull_setinterface_cb(uint8_t interface_number, uint8_t alternate_set)
{
    bt_ull_streaming_t type;
    type.streaming_interface = BT_ULL_STREAMING_INTERFACE_UNKNOWN;
    ull_report("[ULL][USB] bt_ull_setinterface_cb, interfaceNumber:%d, setting: %d", 2,
               interface_number, alternate_set);

    if (0x01 == interface_number || 0x04 == interface_number) { /* SPEAKER */
        type.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        if (0x01 == interface_number) {
            type.port = 0; /* gaming streaming port */
        } else {
            type.port = 1; /* chat streaming port */
        }
    } else if (0x02 == interface_number) {  /* MIC */
        type.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        type.port = 0;
    }
    /* callback */
    if (BT_ULL_STREAMING_INTERFACE_UNKNOWN != type.streaming_interface) {
        if (0x01 == alternate_set) {
            bt_ull_event_callback(BT_ULL_EVENT_USB_PLAYING_IND, &type, sizeof(type));
        } else {
            bt_ull_event_callback(BT_ULL_EVENT_USB_STOP_IND, &type, sizeof(type));
        }
    }
}

/*
Parameter: NULL
Stage: Task level
Period: trigger once
Description: Notify that USB cable remove.
*/
static void bt_ull_unplug_cb(void)
{
    ull_report("[ULL][USB] bt_ull_unplug_cb usb unplugged.", 0);
    //bt_ull_event_callback(BT_ULL_EVENT_USB_STOP_IND, NULL, 0);
}

/*
Parameter: ep_number (1 for audio)
      sampling_rate (48000: default, 44100: optional)
Stage: Task level
Period: trigger once
Description: PC sample rate
*/
static void bt_ull_sample_rate(uint8_t ep_number, uint32_t sampling_rate)
{
    ull_report("[ULL][USB] bt_ull_sample_rate :0x%x, ep_number: 0x%x", 2, sampling_rate, ep_number);
    bt_ull_sample_rate_t sample;
    sample.sample_rate = sampling_rate;
    if (0x01 == ep_number || 0x02 == ep_number) {    /* SPEAKER */
        sample.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        if (0x01 == ep_number) {
            sample.streaming.port = 0; /* gaming streaming port */
        } else {
            sample.streaming.port = 1; /* chat streaming port */
        }
        bt_ull_event_callback(BT_ULL_EVENT_USB_SAMPLE_RATE_IND, &sample, sizeof(sample));
    } else if (0x81 == ep_number) {    /* MIC */
        sample.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        sample.streaming.port = 0;
        bt_ull_event_callback(BT_ULL_EVENT_USB_SAMPLE_RATE_IND, &sample, sizeof(sample));
    } else { }
}

/*
Parameter: ep_number (default: 1)
     channel (default 1&2 channel change concurrency)
     volume (1-100)
Stage: Task level
Period: trigger once
Description: Notify that PC volume change
*/
static void bt_ull_volumechange_cb(uint8_t ep_number, uint8_t channel, uint32_t volume)
{
    ull_report("[ULL][USB] bt_ull_volumechange_cb, ep_number:0x%x, channle :0x%x,. volume:0x%x", 3, ep_number, channel, volume);
    bt_ull_volume_t vol;

    if (0x01 == ep_number || 0x02 == ep_number) {    /* SPEAKER */
        if (0x01 == channel) {
            vol.channel = BT_ULL_AUDIO_CHANNEL_LEFT;
        } else if (0x02 == channel) {
            vol.channel = BT_ULL_AUDIO_CHANNEL_RIGHT;
        }
        vol.action = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
        vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        if (0x01 == ep_number) {
            vol.streaming.port = 0; /* gaming streaming port */
        } else {
            vol.streaming.port = 1; /* chat streaming port */
        }
        vol.volume = (volume > 100) ? 100 : volume;
        bt_ull_event_callback(BT_ULL_EVENT_USB_VOLUME_IND, &vol, sizeof(vol));
    } else if (0x81 == ep_number) {    /* MIC */
        if (0x01 == channel) {
            vol.channel = BT_ULL_AUDIO_CHANNEL_LEFT;
        } else if (0x02 == channel) {
            vol.channel = BT_ULL_AUDIO_CHANNEL_RIGHT;
        }
        vol.action = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
        vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        vol.streaming.port = 0; /* voice streaming port */
        vol.volume = (volume > 100) ? 100 : volume;
        bt_ull_event_callback(BT_ULL_EVENT_USB_VOLUME_IND, &vol, sizeof(vol));
    } else {
        return;
    }
}

/*
Parameter: ep_number (default: 1), usb_audio_mute_t (1: mute, 0: not mute)
Stage: Task level
Period: trigger once
Description: Notify that PC change mute or not
*/
static void bt_ull_mute_cb(uint8_t ep_number, usb_audio_mute_t mute)
{
    bt_ull_streaming_t streaming;
    streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_UNKNOWN;
    ull_report("[ULL][USB] bt_ull_mute_cb, mute :0x%x, ep_number: 0x%x", 2, mute, ep_number);

    if (0x01 == ep_number || 0x02 == ep_number) {    /* SPEAKER */
        streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        if (0x01 == ep_number) {
            streaming.port = 0; /* gaming streaming port */
        } else {
            streaming.port = 1; /* chat streaming port */
        }
    } else if (0x81 == ep_number) {    /* MIC */
        streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        streaming.port = 0;
    }

    if (BT_ULL_STREAMING_INTERFACE_UNKNOWN != streaming.streaming_interface) {
        if (USB_AUDIO_MUTE_ON == mute) {
            bt_ull_event_callback(BT_ULL_EVENT_USB_MUTE_IND, &streaming, sizeof(streaming));
        } else {
            bt_ull_event_callback(BT_ULL_EVENT_USB_UNMUTE_IND, &streaming, sizeof(streaming));
        }
    }
}

#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
/*
Parameter: NULL
Stage: Task level
Period: trigger once
Description: Notify that USB cable remove.
*/
static void bt_ull_2nd_unplug_cb(void)
{
    ull_report("[ULL][USB] bt_ull_2nd_unplug_cb usb unplugged.", 0);
    //bt_ull_event_callback(BT_ULL_EVENT_USB_STOP_IND, NULL, 0);
}
#endif

#endif



#ifdef AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE
static void bt_ull_uplink_latency_debug(uint16_t data_len, int16_t* data)
{
    static uint32_t last_level = 0;
    int16_t *start_address = NULL;
    int16_t sample_value = 0;
    uint32_t current_level = 0;
    uint32_t i;
    current_level = last_level;
    start_address = (int16_t *)data;
    for (i = 0; i < (data_len / 2); i++) {
        sample_value += (*(start_address + i) / (data_len / 2));
    }
    if (sample_value >= 5000) {
        current_level = 1;
    } else {
        current_level = 0;
    }
    if (current_level != last_level) {
        hal_gpio_set_output(HAL_GPIO_13, current_level);
        last_level = current_level;
    }
}
#endif

/*
Parameter: NULL
Stage: ISR level
Period: every 1ms
Description: USB sent USB audio data to PC (Phone/Linux/PS4).
*/
static void bt_ull_usb_tx_cb(void)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_streaming_t streaming;
    streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
    streaming.port = 0;
    /* debug usb tx irq */
    uint32_t gpt_count;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    if (g_ull_usb_tx_gpt) {
        uint32_t diff = gpt_count - g_ull_usb_tx_gpt;
        if ((diff > 1450) || (diff < 550)) {
            ull_report("[ULL] bt_ull_usb_tx_cb, last_gpt: 0x%x, current_gpt:0x%x, diff:%d", 3, g_ull_usb_tx_gpt, gpt_count, diff);
        }
    }
    g_ull_usb_tx_gpt = gpt_count;

    //uint32_t decode_len = 0;
    uint16_t ull_uac_data_len = 0;

    /* calculate mic data bitrate according to sample rate */
    if (0x01 == ctx->ul_microphone.sample_rate) {
        ull_uac_data_len = 96;    /* 96B/ms, 48KHZ/MONO */
        //decode_len = 240;             /* 2.5ms PCM data len */
    } else if (0x08 == ctx->ul_microphone.sample_rate) {
        ull_uac_data_len = 32;    /* 32B/ms, 16KHZ/MONO */
        //decode_len = 80;               /* 2.5ms PCM data len */
    } else {
        bt_utils_assert(0 && "error usb mic sample rate");
    }

    if (bt_ull_get_raw_pcm_data(&streaming, all_zero_buffer, BT_ULL_MIC_PCM_DATA_RATE)) {
        USB_Audio_TX_SendData(0, ull_uac_data_len, (uint8_t*)all_zero_buffer);
    } else {
        memset(all_zero_buffer, 0x00, BT_ULL_MIC_PCM_DATA_RATE);
        USB_Audio_TX_SendData(0, ull_uac_data_len, (uint8_t*)all_zero_buffer);
    }
}

bt_status_t bt_ull_srv_write_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *data, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_streaming_if_info_t *ep = NULL;
    uint8_t *wirte_address = NULL;
    uint32_t free_length = 0;
    bt_ull_context_t *ctx = bt_ull_get_context();

    if (streaming && BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface
        && 0x00 == streaming->port) {
        ep = &(ctx->dl_speaker);
    } else if (streaming && BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface
               && 0x01 == streaming->port) {
        ep = &(ctx->dl_chat);
    } else {
        bt_utils_assert(0 && "error parameter!");
    }
    bt_utils_assert(data);

    if (ep && AUD_ID_INVALID != ep->transmitter
        && !(ep->streaming_flag & BT_ULL_STREAMING_STOPPING)
        && ep->is_transmitter_start) {
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS == audio_transmitter_get_write_information(ep->transmitter, &wirte_address, &free_length)) {
            if (wirte_address && (free_length >= length)) {
                memcpy(wirte_address, data, length);
                audio_transmitter_write_done(ep->transmitter, length);
                status = BT_STATUS_SUCCESS;
            }
        } else {
            ull_report_error("[ULL] bt_ull_write_raw_pcm_data fail due to get write block fail, wirte_address:0x%x, len:0x%x", 2, wirte_address, free_length);
        }
    } else {
        ull_report_error("[ULL] bt_ull_write_raw_pcm_data fail due to transmitter not start", 0);
    }
    return status;
}

uint32_t bt_ull_srv_get_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *buffer, uint32_t buffer_length)
{
    bt_ull_streaming_if_info_t *ep = NULL;
    bt_ull_context_t *ctx = bt_ull_get_context();
    uint32_t read_length = 0;

    bt_utils_assert(streaming && buffer && "error parameter!");

    if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface) {
        ep = &(ctx->ul_microphone);
    } else {
        bt_utils_assert(0 && "only support get mic uplink pcm data!");
    }

    uint32_t decode_len = 0;
    uint16_t ull_uac_data_len = 0;
    /* calculate mic data bitrate according to sample rate */
    if (0x01 == ctx->ul_microphone.sample_rate) {
        ull_uac_data_len = 96;    /* 96B/ms, 48KHZ/MONO */
        decode_len = 240;         /* 2.5ms PCM data len */
    } else if (0x08 == ctx->ul_microphone.sample_rate) {
        ull_uac_data_len = 32;    /* 32B/ms, 16KHZ/MONO */
        decode_len = 80;          /* 2.5ms PCM data len */
    } else {
        bt_utils_assert(0 && "error usb mic sample rate");
    }

    if (ep && (AUD_ID_INVALID != ep->transmitter)
        && !(ep->streaming_flag & BT_ULL_STREAMING_STOPPING)
        && ep->is_transmitter_start) {
        if (NULL != voice_data && voice_data_length) {
            bt_utils_assert((decode_len == voice_data_length) && "error voice_data_length!!!");
            /* 1. Continue write 2.5ms pcm data to usb */
            if (g_ull_voice_data_offset < ull_uac_data_len * 2) {
                memcpy(buffer, (void *)(voice_data + g_ull_voice_data_offset), ull_uac_data_len);
                read_length = ull_uac_data_len;

#if defined(AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE)
                bt_ull_uplink_latency_debug(ull_uac_data_len, (int16_t *)(voice_data + g_ull_voice_data_offset));
#endif /* AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE */
                /* remain 2ms data, should trigger dsp to decode next 2.5ms data */
                if ((ull_uac_data_len / 2) == g_ull_voice_data_offset) {
                    bt_ull_ccni_dsp();
                }
                g_ull_voice_data_offset += ull_uac_data_len;
                /* write done */
                if (voice_data_length == g_ull_voice_data_offset) {
                    audio_transmitter_read_done(ctx->ul_microphone.transmitter, voice_data_length);
                    g_ull_voice_data_offset = 0;
                    voice_data = NULL;
                    voice_data_length = 0;
                }
            } else {
                bt_utils_assert(joint_buffer_offset < ull_uac_data_len && "error joint_buffer_offset!!!");
                /* copy last 0.5ms to joint buffer && notify transmitter read done, clear data pointer */
                if (0 == joint_buffer_offset) {
                    memcpy(joint_buffer + joint_buffer_offset, voice_data + g_ull_voice_data_offset, voice_data_length - g_ull_voice_data_offset);
                    joint_buffer_offset += (voice_data_length - g_ull_voice_data_offset);
                    audio_transmitter_read_done(ctx->ul_microphone.transmitter, voice_data_length);
                    voice_data = NULL;
                    voice_data_length = 0;
                    /* continue read next frame */
                    /* 2.5ms 16KHZ/16Bit/Mono data (32*2.5) */
                    if ((AUDIO_TRANSMITTER_STATUS_SUCCESS == audio_transmitter_get_read_information(ctx->ul_microphone.transmitter, &voice_data, &voice_data_length))
                        && voice_data_length) {
                        bt_utils_assert((decode_len == voice_data_length) && "error voice_data_length!!!");
                        memcpy(joint_buffer + joint_buffer_offset, voice_data, joint_buffer_offset);
                        memcpy(buffer, (void *)joint_buffer, ull_uac_data_len);
                        read_length = ull_uac_data_len;
                        g_ull_voice_data_offset = joint_buffer_offset;
                        joint_buffer_offset = 0;

#if defined(AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE)
                        bt_ull_uplink_latency_debug(ull_uac_data_len,(int16_t *)joint_buffer);
#endif /* AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE */
                    } else {
                        bt_utils_assert(0 && "can't read next packet!!!");
                    }
                } else {
                    /* write joint buffer to usb dongle, <1st packet 0.5ms + 2ns packet 0.5ms> && update g_ull_voice_data_offset to 0.5ms pos */
                    memcpy(joint_buffer + joint_buffer_offset, voice_data, joint_buffer_offset);
                    memcpy(buffer, (void *)joint_buffer, ull_uac_data_len);
                    read_length = ull_uac_data_len;

#if defined(AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE)
                    bt_ull_uplink_latency_debug(ull_uac_data_len,(int16_t *)joint_buffer);
#endif /* AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE */
                    g_ull_voice_data_offset = joint_buffer_offset;
                    joint_buffer_offset = 0;
                }
            }
        } else {
            if ((AUDIO_TRANSMITTER_STATUS_SUCCESS == audio_transmitter_get_read_information(ctx->ul_microphone.transmitter, &voice_data, &voice_data_length))
                && voice_data_length) {
#ifdef AIR_ECNR_POST_PART_ENABLE
                if (false == is_dsp_pre_decode_done) {
                    g_ull_wait_dsp_decode_len =  audio_transmitter_get_available_data_size(ctx->ul_microphone.transmitter);
                    ull_report("[ULL] 1-audio_transmitter_get_total_available_data_size:0x%x", 1, g_ull_wait_dsp_decode_len);
                    if (g_ull_wait_dsp_decode_len >= decode_len * 2) {
                        is_dsp_pre_decode_done = true;;
                        ull_report("[ULL] is_dsp_pre_decode_done:0x%x", 1, is_dsp_pre_decode_done);
                    }
                    voice_data = NULL;
                    voice_data_length = 0;
                    bt_ull_ccni_dsp();
                } else {
#endif
                    if (false == is_debug_mode_enable) {
                        ull_report("[ULL] audio_transmitter_get_read_information, first time  len: 0x%x", 1, voice_data_length);
                        is_debug_mode_enable = true;
                    }
                    bt_utils_assert((decode_len == voice_data_length) && "error voice_data_length!!!");
                    bt_ull_ccni_dsp();
                    memcpy(buffer, (void *)voice_data, ull_uac_data_len);
                    read_length = ull_uac_data_len;
                    g_ull_voice_data_offset += ull_uac_data_len;
#if defined(AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE)
                    bt_ull_uplink_latency_debug(ull_uac_data_len,(int16_t *)voice_data);
#endif /* AIR_GAMING_MODE_UPLINK_LANTENCY_DEBUG_ENABLE */

#ifdef AIR_ECNR_POST_PART_ENABLE
                }
#endif
            } else {
                bt_ull_ccni_dsp();
            }
        }
    } else {
#ifdef AIR_USB_ENABLE
        /* clear mic voice data info */
        g_ull_voice_data_offset = 0;
        voice_data = NULL;
        voice_data_length = 0;
        joint_buffer_offset = 0;
        is_debug_mode_enable = false;
        g_ull_usb_tx_gpt = 0;
#ifdef AIR_ECNR_POST_PART_ENABLE
        g_ull_wait_dsp_decode_len = 0;
        is_dsp_pre_decode_done = false;
#endif

#endif
    }
    return read_length;
}
#endif

static bt_status_t bt_ull_handle_start_streaming(bt_ull_streaming_t *streaming)
{
    uint32_t sp_swicth = 0;
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_streaming_if_info_t *ep = NULL;
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL] bt_ull_handle_start_streaming, streaming_if :0x%x, port: 0x%x", 2, streaming->streaming_interface, streaming->port);

    bt_ull_transmitter_t trans_type = BT_ULL_GAMING_TRANSMITTER;

    ep = bt_ull_get_streaming_interface(streaming);

    if(&(ctx->dl_speaker) == ep) {
      trans_type = BT_ULL_GAMING_TRANSMITTER;
    } else if (&(ctx->dl_chat) == ep) {
      trans_type = BT_ULL_CHAT_TRANSMITTER;
    } else if (&(ctx->ul_microphone) == ep) {
      trans_type = BT_ULL_MIC_TRANSMITTER;
    }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
      else if (&(ctx->dl_linein) == ep) {
      trans_type = BT_ULL_STREAMING_INTERFACE_LINE_IN;
    }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
      else if (&(ctx->ul_lineout) == ep) {
      trans_type = BT_ULL_LINE_OUT_TRANSMITTER;
    }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
      else if (&(ctx->dl_linei2s) == ep) {
      trans_type = BT_ULL_I2S_IN_TRANSMITTER;
    }
#endif
    else {
      bt_utils_assert(0 && "unknown streaming interface type");
      return BT_STATUS_FAIL;
    }

    if(BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface) {
       uint32_t sampe_rate = USB_Audio_Get_RX_Sample_Rate(!(streaming->port));
       ull_report("[ULL] get usb sampe_rate:%d ,current is %d", 2, sampe_rate, ep->sample_rate);
       if (sampe_rate != 0) {
            sp_swicth = bt_ull_handle_sample_rate_switch(sampe_rate);
         if (ep->sample_rate != sp_swicth) {
            ep->sample_rate = sp_swicth;
            ep->is_usb_param_update = true;
         }
      } else {
         ull_report("[ULL] cannot get sampe_rate,current is %d", 1, sampe_rate);
         return BT_STATUS_BUSY;
      }
    }

    if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
        ep->is_streaming = true;
        if (BT_SPP_INVALID_HANDLE != ctx->spp_handle && ctx->is_ull_connected && ctx->is_ull_version_ready) {
            /* 1. exist sniff mode && disable link sniff mode when this link is streaming on */
            uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
            bt_utils_assert(gap_handle && "gap handle is null");
            bt_cm_link_info_t *link_info = bt_cm_get_link_information(ctx->bt_addr);
            ull_report("[ULL] bt_ull_handle_start_streaming, is_disable_sniff: 0x%x", 1, ctx->is_disable_sniff);
            /* disable sniff mode during one of ep streaming start */
#ifdef AIR_BT_ULTRA_LOW_LATENCY_IDLE_SNIFF_ENABLE
            if (false == ctx->is_disable_sniff) {
                ctx->is_disable_sniff = true;
                bt_gap_link_policy_setting_t link_policy;
                link_policy.sniff_mode = BT_GAP_LINK_POLICY_DISABLE;
                bt_gap_write_link_policy(gap_handle, &link_policy);
            }
#endif
            if(BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface) {
                if (AUD_ID_INVALID != ep->transmitter) {
                    ull_report("[ULL] transmitter deinit :0x%x", 1, status);
                    audio_transmitter_status_t status = audio_transmitter_deinit(ep->transmitter);
                    if(AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                      ep->transmitter = AUD_ID_INVALID;
                      ep->is_usb_param_update = false;
                      bt_ull_init_transimter(ctx->codec_type, trans_type);
                   }
                } else {
                   if (ep->is_usb_param_update) {
                      ep->is_usb_param_update = false;
                      bt_ull_init_transimter(ctx->codec_type, trans_type);
                   }
                }
           } else {
            if (AUD_ID_INVALID == ep->transmitter) {
                bt_ull_init_transimter(ctx->codec_type, trans_type);
            }
           }
            /* we shuold 1st exit sniff mode before start DSP */
            if (link_info && (BT_GAP_LINK_SNIFF_TYPE_ACTIVE != link_info->sniff_state)) {
                bt_gap_exit_sniff_mode(gap_handle);
            } else {
               bt_ull_req_t request;
               if(&(ctx->ul_microphone) == ep) {
                  uint8_t  bt_mic_frame_size;
                  bt_mic_frame_size = bt_ull_get_mic_frame_size();
                  memset(&request, 0x00, sizeof(request));
                  request.event = BT_ULL_EVENT_UPLINK_INFO;
                  request.uplink_info.ul_frame_size = bt_mic_frame_size;
                  bt_ull_send_data(ctx->spp_handle, (uint8_t*)&request, sizeof(request));
                  ull_report("[ULL][API] dongle sync uplink_frame_size info: %d", 1, bt_mic_frame_size);
                }
                /* 2. sync ep streaming start to client */
                memset(&request, 0x00, sizeof(request));
                request.event = BT_ULL_EVENT_STREAMING_START_IND;
                memcpy(&(request.streaming_port), streaming, sizeof(bt_ull_streaming_t));
                bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
                ull_report("[ULL][API] current is allow play: 0x%x", 1, ctx->allow_play);
                /* if client is not allow, dongle server shuold stop due to client bandwidth limitation */
                if (BT_ULL_PLAY_ALLOW == ctx->allow_play) {
                    /* 3. send 0xFDCB vnd cmd to controller notify ull is playing */
                    bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, true, ctx->dl_latency);
                    /* 4. start audio transmitter to notify DSP starting */
                    bt_ull_start_transmitter(ep);
                }
            }
        }
    } else {
        status = BT_STATUS_UNSUPPORTED;
    }
    return status;
}
static bt_status_t bt_ull_handle_stop_streaming(bt_ull_streaming_t *streaming)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_streaming_if_info_t *ep = NULL;
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL] bt_ull_handle_stop_streaming, streaming_if :0x%x, port: 0x%x", 2, streaming->streaming_interface, streaming->port);

    ep = bt_ull_get_streaming_interface(streaming);

    if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
        ep->is_streaming = false;
        if (&(ctx->ul_microphone) == ep) {
#ifdef AIR_USB_ENABLE
            /* clear mic voice data info */
            g_ull_voice_data_offset = 0;
            voice_data = NULL;
            voice_data_length = 0;
            joint_buffer_offset = 0;
            is_debug_mode_enable = false;
            g_ull_usb_tx_gpt = 0;
#ifdef AIR_ECNR_POST_PART_ENABLE
            g_ull_wait_dsp_decode_len = 0;
            is_dsp_pre_decode_done = false;
#endif
#endif
        }
        if (BT_SPP_INVALID_HANDLE != ctx->spp_handle && ctx->is_ull_connected && ctx->is_ull_version_ready) {
            /* 1. sync ep streaming stop to client */
            bt_ull_req_t request;
            memset(&request, 0x00, sizeof(request));
            request.event = BT_ULL_EVENT_STREAMING_STOP_IND;
            memcpy(&(request.streaming_port), streaming, sizeof(bt_ull_streaming_t));
            if ((&(ctx->ul_microphone) == ep )
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
                || (&(ctx->ul_lineout) == ep)
#endif
            ){
            if (false == ctx->ul_microphone.is_streaming
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
             && false == ctx->ul_lineout.is_streaming
#endif
            ){
                bt_ull_send_data(ctx->spp_handle, (uint8_t*)&request, sizeof(request));
             }
            } else if (false == ctx->dl_speaker.is_streaming
                    && false == ctx->dl_chat.is_streaming
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                    && false == ctx->dl_linein.is_streaming
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                    && false == ctx->dl_linei2s.is_streaming
#endif
             ) {
                /* notify client rx stop after both 2-rx stop */
                bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
            }
            /* disable controller and enable sniff mode after all streaming stop */
            if (false == ctx->dl_speaker.is_streaming
                && false == ctx->dl_chat.is_streaming
                && false == ctx->ul_microphone.is_streaming
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
                && false == ctx->dl_linein.is_streaming
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
                && false == ctx->ul_lineout.is_streaming
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
                && false == ctx->dl_linei2s.is_streaming
#endif
               ) {
                ull_report("[ULL] bt_ull_handle_stop_streaming, is_disable_sniff: 0x%x", 1, ctx->is_disable_sniff);
                /* 2. enable link sniff policy */
                uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
                bt_utils_assert(gap_handle && "gap handle is null");
#ifdef AIR_BT_ULTRA_LOW_LATENCY_IDLE_SNIFF_ENABLE
                if (ctx->is_disable_sniff) {
                    ctx->is_disable_sniff = false;
                    bt_gap_link_policy_setting_t link_policy;
                    link_policy.sniff_mode = BT_GAP_LINK_POLICY_ENABLE;
                    bt_gap_write_link_policy(gap_handle, &link_policy);
                }
#endif
                /* 3. fdcb notify controller disable, controller will be clear music downlink avm buffer */
                bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, false, ctx->dl_latency);
            }

            /* 4. stop audio transmitter to notify DSP stop */
            bt_ull_stop_transmitter(ep);
        }

    } else {
        status = BT_STATUS_UNSUPPORTED;
    }
    return status;
}

static bt_status_t bt_ull_handle_set_volume(bt_ull_volume_t *vol)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bool is_vol_update = false;
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_streaming_if_info_t *ep = NULL;
    ull_report("[ULL][API] bt_ull_handle_set_volume, streaming_if:0x%x, port:0x%x, action:0x%x, audio_channel: 0x%x, volume:%d", 5,
               vol->streaming.streaming_interface, vol->streaming.port, vol->action, vol->channel, vol->volume);

    ep = bt_ull_get_streaming_interface(&vol->streaming);

    if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
#ifdef MTK_AWS_MCE_ENABLE
        bt_aws_mce_role_t cur_role = bt_device_manager_aws_local_info_get_role();
        if (BT_AWS_MCE_ROLE_PARTNER == cur_role || BT_AWS_MCE_ROLE_CLINET == cur_role) {
            /* partner send request to agent */
            bt_ull_aws_mce_eir_info_t report_info = {0};
            report_info.event = BT_ULL_AWS_MCE_REPORT_VOL_SYNC;
            memcpy(&(report_info.param.aws_vol_report), vol, sizeof(bt_ull_volume_t));
            bt_ull_aws_mce_send_eir(&report_info, sizeof(report_info), false);
        } else {
            /* sync volume operation to server */
            if (BT_SPP_INVALID_HANDLE != ctx->spp_handle && ctx->is_ull_connected) {
                bt_ull_req_t request;
                memset(&request, 0x00, sizeof(request));
                request.event = BT_ULL_EVENT_VOLUME_ACTION;
                bt_ull_volume_t *vol_action = (bt_ull_volume_t *) & (request.vol_action);
                memcpy(vol_action, vol, sizeof(bt_ull_volume_t));
                bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
            }
        }
#else
        /* sync volume operation to service */
        if (BT_SPP_INVALID_HANDLE != ctx->spp_handle && ctx->is_ull_connected) {
            bt_ull_req_t request;
            memset(&request, 0x00, sizeof(request));
            request.event = BT_ULL_EVENT_VOLUME_ACTION;
            bt_ull_volume_t *vol_action = (bt_ull_volume_t *) & (request.vol_action);
            memcpy(vol_action, vol, sizeof(bt_ull_volume_t));
            bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
        }
#endif
    } else if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
        if (BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME == vol->action) {
            bt_sink_srv_am_volume_level_out_t new_vol = bt_ull_get_volume_level(vol->volume);
            if (BT_ULL_AUDIO_CHANNEL_DUAL == vol->channel) {
                ep->original_volume.vol_left = vol->volume;
                ep->original_volume.vol_right = vol->volume;
                if (ep->volume.vol_left != new_vol
                    || ep->volume.vol_right != new_vol) {
                    ep->volume.vol_left = new_vol;
                    ep->volume.vol_right = new_vol;
                    is_vol_update = true;
                }
            } else if (BT_ULL_AUDIO_CHANNEL_LEFT == vol->channel) {
                ep->original_volume.vol_left = vol->volume;
                if (ep->volume.vol_left != new_vol) {
                    ep->volume.vol_left = new_vol;
                    is_vol_update = true;
                }
            } else if (BT_ULL_AUDIO_CHANNEL_RIGHT == vol->channel) {
                ep->original_volume.vol_right = vol->volume;
                if (ep->volume.vol_right != new_vol) {
                    ep->volume.vol_right = new_vol;
                    is_vol_update = true;
                }
            } else {
                bt_utils_assert(0 && "unknown audio channel");
            }
        } else {
            bt_utils_assert(0 && "unknown action on server!");
        }
#if 1
        /* sync Server volume to client */
        if (ctx->is_ull_connected
            && BT_SPP_INVALID_HANDLE != ctx->spp_handle) {
            /* sync PC volume to client */
            bt_ull_req_t request;
            memset(&request, 0x00, sizeof(request));
            request.event = BT_ULL_EVENT_VOLUME_IND;
            bt_ull_volume_ind_t *p_vol = (bt_ull_volume_ind_t *) & (request.vol_ind);
            memcpy(&(p_vol->streaming), &(vol->streaming), sizeof(p_vol->streaming));
            p_vol->vol.vol_left = ep->original_volume.vol_left;
            p_vol->vol.vol_right = ep->original_volume.vol_right;
            bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
        }
#endif
        /* set volume */
        if (is_vol_update) {
            bt_ull_set_transmitter_volume(&(vol->streaming));
        }
    }
    return status;
}

static bt_status_t bt_ull_handle_set_streaming_mute(bt_ull_streaming_t *streaming, bool is_mute)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_streaming_if_info_t *ep = NULL;

    ull_report("[ULL][API] bt_ull_handle_set_streaming_mute, type:0x%x, port:0x%x, is_mute: 0x%x,", 3,
               streaming->streaming_interface, streaming->port, is_mute);

    ep = bt_ull_get_streaming_interface(streaming);
    if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
        ep->is_mute = is_mute;
        /* sync mute operation to server */
        if (BT_SPP_INVALID_HANDLE != ctx->spp_handle && ctx->is_ull_connected) {
            bt_ull_req_t request;
            memset(&request, 0x00, sizeof(request));
            if (is_mute) {
                request.event = BT_ULL_EVENT_VOLUME_MUTE;
            } else {
                request.event = BT_ULL_EVENT_VOLUME_UNMUTE;
            }
            memcpy(&(request.streaming_port), streaming, sizeof(bt_ull_streaming_t));
            status = bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
        }
    } else if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
        ep->is_mute = is_mute;
        /* Server mute or unmute streaming */
        if (AUD_ID_INVALID != ep->transmitter
            && ep->is_transmitter_start) {
            audio_transmitter_runtime_config_t config;
            gaming_mode_runtime_config_operation_t vol_op;
            memset(&config, 0x00, sizeof(config));
            if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface) {
                vol_op = GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_DUL;
            } else {
                vol_op = GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL;
                if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface) {
                    uint8_t idx = 0;
                    for (idx = 0; idx < BT_ULL_MAX_STREAMING_NUM; idx++) {
                        if (!memcmp(streaming, &(ctx->dl_mix_ratio.streamings[idx].streaming), sizeof(bt_ull_streaming_t))) {
                            config.gaming_mode_runtime_config.vol_level.vol_ratio = ctx->dl_mix_ratio.streamings[idx].ratio;
                            ull_report("[ULL] streaming mix ratio: %d", 1, config.gaming_mode_runtime_config.vol_level.vol_ratio);
                            break;
                        }
                    }
                }
            }
            /* we set streaming both L and R channel volume to 0 when mute */
            if (is_mute) {
                config.gaming_mode_runtime_config.vol_level.vol_level_l = 0;
                config.gaming_mode_runtime_config.vol_level.vol_level_r = 0;
            } else {
                /* reset L and R channel volume to back when unmute */
                config.gaming_mode_runtime_config.vol_level.vol_level_l = ep->volume.vol_left;
                config.gaming_mode_runtime_config.vol_level.vol_level_r = ep->volume.vol_right;
                ull_report("[ULL][API] unmute, reset vol_left: %d, vol_right: %d", 2, ep->volume.vol_left, ep->volume.vol_right);
            }
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ep->transmitter, vol_op, &config)) {
                ull_report_error("[ULL][API] audio_transmitter_set_runtime_config fail, trans_id:0x%x, vol_op:0x%x, vol_left: %d, vol_right: %d", 4,
                                 ep->transmitter, vol_op, ep->volume.vol_left, ep->volume.vol_right);
            }
        }
    } else {
        bt_utils_assert(0 && "unknown role");
    }
    return status;
}


static uint32_t bt_ull_handle_sample_rate_switch(uint32_t sample_rate)
{
    uint32_t sr_switch = 0x00;
    /** sample rate switch */
    /**< b0: 48000, b1: 44100, b2: 32000, b3: 16000,  b4:96000*/
    if (48000 == sample_rate) {
        sr_switch = 0x01;
    } else if (44100 == sample_rate) {
        sr_switch = 0x02;
    } else if (32000 == sample_rate) {
        sr_switch = 0x04;
    } else if (16000 == sample_rate) {
        sr_switch = 0x08;
    } else if (96000 == sample_rate) {
        sr_switch = 0x10;
    } else {
        return BT_STATUS_FAIL;
        ull_report_error("[ULL] server receive unknown sample rate: %d", 1, sample_rate);
    }
    return sr_switch;
}


static bt_status_t bt_ull_handle_set_streaming_sample_rate(bt_ull_sample_rate_t *sample_rate)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_streaming_if_info_t *ep = NULL;
    uint32_t sr_switch = 0x00;
    uint32_t gap_handle = 0x00;

    ull_report("[ULL][API] bt_ull_handle_set_streaming_sample_rate, if_type:0x%x, port:0x%x, rate: %d,", 3,
               sample_rate->streaming.streaming_interface, sample_rate->streaming.port, sample_rate->sample_rate);

    if (BT_ULL_ROLE_SERVER != ctx->ull_role) {
        ull_report_error("[ULL] error role: 0x%x", 1, ctx->ull_role);
        return BT_STATUS_FAIL;
    }

    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == sample_rate->streaming.streaming_interface
        && 0x00 == sample_rate->streaming.port) {
        ep = &(ctx->dl_speaker);
    } else if (BT_ULL_STREAMING_INTERFACE_SPEAKER == sample_rate->streaming.streaming_interface
               && 0x01 == sample_rate->streaming.port) {
        ep = &(ctx->dl_chat);
    } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == sample_rate->streaming.streaming_interface) {
        ep = &(ctx->ul_microphone);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
    } else if (BT_ULL_STREAMING_INTERFACE_LINE_OUT == sample_rate->streaming.streaming_interface){
        ep = &(ctx->ul_lineout);
#endif
    } else {
        bt_utils_assert(0 && "unknown usb end pointer type");
        return BT_STATUS_FAIL;
    }

    sr_switch = bt_ull_handle_sample_rate_switch(sample_rate->sample_rate);

    ull_report("[ULL] ep->transmitter:0x%x, ep->is_transmitter_start: 0x%x, streaming_flag:0x%x, ep->sample_rate:%d, sr_switch:%d", 5,
               ep->transmitter, ep->is_transmitter_start, ep->streaming_flag, ep->sample_rate, sr_switch);

    if(sr_switch != ep->sample_rate){
        ep->sample_rate = sr_switch;
      ep->is_usb_param_update = true;
    }
    if (BT_SPP_INVALID_HANDLE != ctx->spp_handle && ctx->is_ull_connected && ep->is_usb_param_update) {
        ep->is_usb_param_update = false;
        gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
        bt_utils_assert(gap_handle && "gap handle is null");

            if (AUD_ID_INVALID != ep->transmitter) {
                if (false == ep->is_transmitter_start) {
                    /* transmitter is starting/stopping */
                    if (ep->streaming_flag & BT_ULL_STREAMING_STARTING
                        || ep->streaming_flag & BT_ULL_STREAMING_STOPPING) {
                        /* reconfig after start success */
                        BT_ULL_SET_FLAG(ep->streaming_flag, BT_ULL_STREAMING_RECONFIG);
                    } else {
                        /* transmitter is stopped */
                        audio_transmitter_status_t status = audio_transmitter_deinit(ep->transmitter);
                        ull_report("[ULL] transmitter deinit :0x%x", 1, status);
                        if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                            ep->transmitter = AUD_ID_INVALID;
                            if (ep == &(ctx->ul_microphone)) {
                                bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_MIC_TRANSMITTER);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
                            } else if (ep == &(ctx->ul_lineout)) {
                               bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_LINE_OUT_TRANSMITTER);
#endif
                            } else if (ep == &(ctx->dl_chat)) {
                                bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_CHAT_TRANSMITTER);
                            } else {
                                bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_GAMING_TRANSMITTER);
                            }
                        }
                    }
                } else {
                    if (ep->streaming_flag & BT_ULL_STREAMING_STARTING
                        || ep->streaming_flag & BT_ULL_STREAMING_STOPPING) {
                        /* transmitter is starting/stopping */
                        BT_ULL_SET_FLAG(ep->streaming_flag, BT_ULL_STREAMING_RECONFIG);
                    } else {
                        /* transmitter is started */
                        BT_ULL_SET_FLAG(ep->streaming_flag, BT_ULL_STREAMING_RECONFIG);
                        bt_ull_stop_transmitter(ep);
                    }
                }
            } else {
                /* init transmitter info according to usb sample rate */
                if (ep == &(ctx->ul_microphone)) {
                    bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_MIC_TRANSMITTER);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
                } else if (ep == &(ctx->ul_lineout)) {
                bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_LINE_OUT_TRANSMITTER);
#endif
                } else if (ep == &(ctx->dl_chat)) {
                    bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_CHAT_TRANSMITTER);
                } else {
                    bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_GAMING_TRANSMITTER);
                }
            }
        }
    return status;
}


static bt_status_t bt_ull_handle_set_streaming_sample_size(bt_ull_streaming_sample_size_t *sample_size)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_streaming_if_info_t *ep = NULL;

    if (BT_ULL_ROLE_SERVER != ctx->ull_role) {
        ull_report("[ULL] SAMPLE SIZE, error role: 0x%x", 1, ctx->ull_role);
        return BT_STATUS_FAIL;
    }

    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == sample_size->streaming.streaming_interface
        && 0x00 == sample_size->streaming.port) {
        ep = &(ctx->dl_speaker);
    } else if (BT_ULL_STREAMING_INTERFACE_SPEAKER == sample_size->streaming.streaming_interface
        && 0x01 == sample_size->streaming.port) {
        ep = &(ctx->dl_chat);
    } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == sample_size->streaming.streaming_interface) {
        ep = &(ctx->ul_microphone);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
    } else if (BT_ULL_STREAMING_INTERFACE_LINE_OUT == sample_size->streaming.streaming_interface){
        ep = &(ctx->ul_lineout);
#endif
    } else {
        bt_utils_assert(0 && "unknown usb end pointer type");
        return BT_STATUS_FAIL;
    }

    ull_report("[ULL]bt_ull_handle_set_streaming_sample_size, if_type:0x%x, port:0x%x, sample size: %d, usb sample size: %d", 4,
        sample_size->streaming.streaming_interface, sample_size->streaming.port, ep->sample_size, sample_size->sample_size);

    if (ep->sample_size != sample_size->sample_size) {
        ep->sample_size = sample_size->sample_size;
        ep->is_usb_param_update= true;
    } else {
        status = BT_STATUS_FAIL;
        ull_report("[ULL]bt_ull_srv_handle_set_streaming_sample_size, stream_info is null", 0);
    }
    return status;
}


static bt_status_t bt_ull_handle_set_streaming_sample_channel(bt_ull_streaming_sample_channel_t *sample_channel)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_streaming_if_info_t *ep = NULL;

    if (BT_ULL_ROLE_SERVER != ctx->ull_role) {
        ull_report("[ULL] SAMPLE CHANNEL, error role: 0x%x", 1, ctx->ull_role);
        return BT_STATUS_FAIL;
    }

    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == sample_channel->streaming.streaming_interface
        && 0x00 == sample_channel->streaming.port) {
        ep = &(ctx->dl_speaker);
    } else if (BT_ULL_STREAMING_INTERFACE_SPEAKER == sample_channel->streaming.streaming_interface
        && 0x01 == sample_channel->streaming.port) {
        ep = &(ctx->dl_chat);
    } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == sample_channel->streaming.streaming_interface) {
        ep = &(ctx->ul_microphone);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
    } else if (BT_ULL_STREAMING_INTERFACE_LINE_OUT == sample_channel->streaming.streaming_interface){
        ep = &(ctx->ul_lineout);
#endif
    } else {
        bt_utils_assert(0 && "unknown usb end pointer type");
        return BT_STATUS_FAIL;
    }

    ull_report("[ULL]bt_ull_handle_set_streaming_sample_channel, if_type:0x%x, port:0x%x, sample channel: %d, usb sample channel: %d", 4,
        sample_channel->streaming.streaming_interface, sample_channel->streaming.port, sample_channel->sample_channel, ep->sample_channel);

    if (ep->sample_channel != sample_channel->sample_channel) {
        ep->sample_channel = sample_channel->sample_channel;
        ep->is_usb_param_update = true;
    } else {
        status = BT_STATUS_FAIL;
        ull_report("[ULL]bt_ull_srv_handle_set_streaming_sample_channel, stream_info is null", 0);
    }
    return status;
}

static bt_status_t bt_ull_handle_set_streaming_latency(bt_ull_latency_t *req_latency)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL] set latency to: 0x%x, old latency: 0x%x, dl is streaming: 0x%x, dl transmitter state: 0x%x", 4,
               req_latency->latency, ctx->dl_latency, ctx->dl_speaker.is_streaming, ctx->dl_speaker.is_transmitter_start);
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t cur_role = bt_device_manager_aws_local_info_get_role();
    if (BT_AWS_MCE_ROLE_PARTNER == cur_role
        || BT_AWS_MCE_ROLE_CLINET == cur_role) {
        /* partner role cannot switch latency */
        return BT_STATUS_FAIL;
    }
#endif
    if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
#if 0
        if (false == bt_ull_get_ULL_mode()) {
            if (req_latency->latency < BT_ULL_DOWNLINK_LIMIATTION) {
                req_latency->latency = BT_ULL_DOWNLINK_LIMIATTION;
            }
        }
#endif
        /* notify dongle latency switch */
        if (req_latency->latency != ctx->dl_latency) {
            ctx->dl_latency = req_latency->latency;
            if (ctx->dl_latency > BT_ULL_DOWNLINK_SINGLE_LIMITATION) {
                ctx->ul_latency = BT_ULL_UPLINK_MAX;
            } else {
                ctx->ul_latency = BT_ULL_UPLINK_DEFAULT;
            }
            if (ctx->spp_handle) {
                bt_ull_req_t request;
                memset(&request, 0x00, sizeof(request));
                request.event = BT_ULL_EVENT_LATENCY_SWITCH;
                bt_ull_latency_t *latency = (bt_ull_latency_t *) & (request.latency);
                memcpy(&(latency->streaming), &(req_latency->streaming), sizeof(bt_ull_streaming_t));
                latency->latency = ctx->dl_latency;
                status = bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
            }
            /* should disable page scan if latency < 25ms */
            if (ctx->is_ull_connected) {
                if (req_latency->latency > BT_ULL_DOWNLINK_LIMIATTION_MULTILINK) {
                    bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
                } else {
                    bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_DISABLE);
                }
            }
        }
    } else if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
        if (req_latency->latency != ctx->dl_latency) {
            ctx->dl_latency = req_latency->latency;
                if (ctx->dl_latency > BT_ULL_DOWNLINK_SINGLE_LIMITATION) {
                    ctx->ul_latency = BT_ULL_UPLINK_MAX;
            } else {
                if(ctx->sdk_version < AIROHA_SDK_VERSION) {
                  ctx->ul_latency = BT_ULL_UPLINK_LEGENCY_DEFAULT;
                } else {
                  ctx->ul_latency = BT_ULL_UPLINK_DEFAULT;
                }
            }
            /* set microphone uplink transmitter latency */
            if (ctx->ul_microphone.transmitter
                && ctx->ul_microphone.is_transmitter_start) {
                audio_transmitter_runtime_config_t config;
                config.gaming_mode_runtime_config.latency_us = ctx->ul_latency * 1000;
                ull_report("[ULL] dongle latency switch,  set uplink latency:%d", 1, ctx->ul_latency);
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != audio_transmitter_set_runtime_config(ctx->ul_microphone.transmitter, GAMING_MODE_CONFIG_OP_LATENCY_SWITCH, &config)) {
                    ull_report_error("[ULL][API] audio_transmitter_set_runtime_config fail, trans_id:0x%x, latency: %d", 2,
                                     ctx->ul_microphone.transmitter, config.gaming_mode_runtime_config.latency_us);
                }
            }

            if (ctx->dl_speaker.is_streaming || ctx->dl_chat.is_streaming) {
                uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
                bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, false, ctx->dl_latency);
            }

            /* restart gaming transmitter */
            if (ctx->dl_speaker.is_streaming) {
                bt_ull_stop_transmitter(&ctx->dl_speaker);
                bt_ull_start_transmitter(&ctx->dl_speaker);
            }
            /* restart chat transmitter */
            if (ctx->dl_chat.is_streaming) {
                bt_ull_stop_transmitter(&ctx->dl_chat);
                bt_ull_start_transmitter(&ctx->dl_chat);
            }
            /* reply service to ensure dongle stop before headset */
            bt_ull_req_t reply;
            memset(&reply, 0x00, sizeof(reply));
            reply.event = BT_ULL_EVENT_LATENCY_SWITCH_COMPLETE;
            bt_ull_latency_t *latency = (bt_ull_latency_t *) & (reply.latency);
            memcpy(&(latency->streaming), &(req_latency->streaming), sizeof(bt_ull_streaming_t));
            latency->latency = ctx->dl_latency;
            bt_ull_send_data(ctx->spp_handle, (uint8_t *)&reply, sizeof(reply));
        }
    }
    return status;
}
static bt_status_t bt_ull_handle_set_streaming_mix_ratio(bt_ull_mix_ratio_t *ratio)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_context_t *ctx = bt_ull_get_context();

        uint8_t idx = 0;
        for (idx = 0; idx < BT_ULL_MAX_STREAMING_NUM; idx++) {
            ull_report("[ULL] bt_ull_handle_set_streaming_mix_ratio, streaming[%d] type:0x%x, port:0x%x, ratio: %d", 4,
                idx, ratio->streamings[idx].streaming.streaming_interface, ratio->streamings[idx].streaming.port, ratio->streamings[idx].ratio);
        }
        /* Now, only speaker streaming can mix */
        if (BT_ULL_STREAMING_INTERFACE_SPEAKER != ratio->streamings[0].streaming.streaming_interface 
           ||((BT_ULL_STREAMING_INTERFACE_SPEAKER != ratio->streamings[1].streaming.streaming_interface)
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
           ||(BT_ULL_STREAMING_INTERFACE_LINE_IN != ratio->streamings[1].streaming.streaming_interface)
#endif
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
           ||(BT_ULL_STREAMING_INTERFACE_I2S_IN != ratio->streamings[1].streaming.streaming_interface)
#endif
           )) {
           return BT_STATUS_FAIL;
        }
        memcpy(&(ctx->dl_mix_ratio), ratio, sizeof(bt_ull_mix_ratio_t));
        if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
            /* sync mix setting to server */
            if (BT_SPP_INVALID_HANDLE != ctx->spp_handle && ctx->is_ull_connected) {
                bt_ull_req_t request;
                memset(&request, 0x00, sizeof(request));
                request.event = BT_ULL_EVENT_MIX_RATIO_ACTION;
                memcpy(&(request.mix_ratio), ratio, sizeof(bt_ull_mix_ratio_t));
                status = bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
            }
        } else if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
            bt_ull_streaming_t streaming;
            /*  set gaming streaming mixing */
            if (ctx->dl_speaker.is_streaming) {
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                streaming.port = 0;
                bt_ull_set_transmitter_volume(&streaming);
            }
            /*  set chat streaming mixing */
            if (ctx->dl_chat.is_streaming) {
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                streaming.port = 1;
                bt_ull_set_transmitter_volume(&streaming);
            }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
            /*  set linein streaming mixing */
            if (ctx->dl_linein.is_streaming) {
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
                streaming.port = 0;
                bt_ull_set_transmitter_volume(&streaming);
            }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
            /*  set linei2s streaming mixing */
            if (ctx->dl_linei2s.is_streaming) {
                streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
                streaming.port = 0;
                bt_ull_set_transmitter_volume(&streaming);
            }
#endif
        } else {
            bt_utils_assert(0 && "error role");
    }
    return status;
}

static bt_status_t bt_ull_handle_usb_hid_control(bt_ull_usb_hid_control_t action)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL] usb_hid_control: 0x%x, role:0x%x", 2, action, ctx->ull_role);

#ifdef AIR_USB_ENABLE
    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
        ull_report("[ULL] usb_hid_control, current is XBOX mode, ignore the HID control event", 0);
        return BT_STATUS_FAIL;
    }
#endif /* AIR_USB_ENABLE */

    if (ctx->is_ull_connected) {
        if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
#ifdef MTK_AWS_MCE_ENABLE
            bt_aws_mce_role_t cur_role = bt_device_manager_aws_local_info_get_role();
            if (BT_AWS_MCE_ROLE_PARTNER == cur_role
                || BT_AWS_MCE_ROLE_CLINET == cur_role) {
                /* partner role cannot switch latency */
                return BT_STATUS_FAIL;
            }
#endif
            bt_ull_req_t request;
            memset(&request, 0x00, sizeof(request));
            request.event = BT_ULL_EVENT_USB_HID_CONTROL_ACTION;
            request.hid_control = action;
            bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
        } else if (BT_ULL_ROLE_SERVER == ctx->ull_role) {

#if defined(AIR_USB_ENABLE) && defined(AIR_USB_AUDIO_ENABLE) && defined(AIR_USB_HID_ENABLE)
            if (BT_ULL_USB_HID_PLAY_PAUSE_TOGGLE == action
                || BT_ULL_USB_HID_PAUSE == action
                || BT_ULL_USB_HID_PLAY == action) {
                USB_HID_PlayPause();
            } else if (BT_ULL_USB_HID_PREVIOUS_TRACK == action) {
                USB_HID_ScanPreviousTrack();
            } else if (BT_ULL_USB_HID_NEXT_TRACK == action) {
                USB_HID_ScanNextTrack();
            }
#endif
        }
    }
    return status;
}


#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
static bt_status_t bt_ull_handle_set_streaming_param(bt_ull_interface_config_t *param)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_context_t* ctx = bt_ull_get_context();
    bt_ull_streaming_if_info_t *ep = NULL;

    i2s_in_dongle_config_t *param_ptr = &param->i2s_in_dongle_config;
    i2s_in_dongle_config_t *i2s_ptr = &ctx->linei2s_param;

    if (BT_ULL_STREAMING_INTERFACE_I2S_IN == param->streaming.streaming_interface) {
      ep = &(ctx->dl_linei2s);
      i2s_ptr->audio_device = param_ptr->audio_device;
      i2s_ptr->audio_interface = param_ptr->audio_interface;
      i2s_ptr->codec_param.pcm.sample_rate = param_ptr->codec_param.pcm.sample_rate;
      i2s_ptr->i2s_fromat = param_ptr->i2s_fromat;
      i2s_ptr->i2s_word_length = param_ptr->i2s_word_length;

      ull_report("[ULL] ull set line_i2s param audio_device:%d, audio_interface:%d, sample_rate:%d, i2s_fromat:%d, i2s_word_length:%d", 5,
        i2s_ptr->audio_device,i2s_ptr->audio_interface,i2s_ptr->codec_param.pcm.sample_rate,i2s_ptr->i2s_fromat,i2s_ptr->i2s_word_length);

      if (BT_SPP_INVALID_HANDLE != ctx->spp_handle && ctx->is_ull_connected) {
          if (AUD_ID_INVALID != ep->transmitter) {
           if (false == ep->is_transmitter_start) {
               /* transmitter is starting/stopping */
               if (ep->streaming_flag & BT_ULL_STREAMING_STARTING
                   || ep->streaming_flag & BT_ULL_STREAMING_STOPPING) {
                   /* reconfig after start success */
                   BT_ULL_SET_FLAG(ep->streaming_flag, BT_ULL_STREAMING_RECONFIG);
               } else {
                   /* transmitter is stopped */
                   audio_transmitter_status_t status = audio_transmitter_deinit(ep->transmitter);
                   ull_report("[ULL] transmitter deinit :0x%x", 1, status);
                   if (AUDIO_TRANSMITTER_STATUS_SUCCESS == status) {
                      ep->transmitter = AUD_ID_INVALID;
                      bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_I2S_IN_TRANSMITTER);
                   }
               }
           } else {
               if (ep->streaming_flag & BT_ULL_STREAMING_STARTING
                   || ep->streaming_flag & BT_ULL_STREAMING_STOPPING) {
                   /* transmitter is starting/stopping */
                   BT_ULL_SET_FLAG(ep->streaming_flag, BT_ULL_STREAMING_RECONFIG);
               } else {
                   /* transmitter is started */
                   BT_ULL_SET_FLAG(ep->streaming_flag, BT_ULL_STREAMING_RECONFIG);
                   bt_ull_stop_transmitter(ep);
               }
          }
        } else {
        /* init transmitter info */
        bt_ull_init_transimter(BT_ULL_OPUS_CODEC, BT_ULL_I2S_IN_TRANSMITTER);
       }
     }
   }else {
     ull_report("[ULL] only line_i2s support set param", 0);
   }
   return status;
}
#endif


static bt_status_t bt_ull_handle_gap_event_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    bt_ull_streaming_if_info_t *ep = NULL;
    bt_ull_streaming_if_info_t *next_ep = NULL;
    /* only care sniff mode change */
    if (BT_GAP_SNIFF_MODE_CHANGE_IND == msg) {
        bt_gap_sniff_mode_changed_ind_t *ind = (bt_gap_sniff_mode_changed_ind_t *)buffer;
        if (ind) {
            ull_report("[ULL][GAP] BT_GAP_SNIFF_MODE_CHANGE_IND: 0x%x", 1, ind->sniff_status);
            if (BT_ULL_ROLE_SERVER == ctx->ull_role
                && BT_GAP_LINK_SNIFF_TYPE_ACTIVE == ind->sniff_status) {
                if (BT_SPP_INVALID_HANDLE != ctx->spp_handle && ctx->is_ull_connected) {
                    /* check all ep is streaming or not after sniff mode exit */
                    ep = &(ctx->dl_speaker);
                    next_ep = &(ctx->dl_speaker);
                    while (next_ep && ep) {
                        /* 2. sync ep streaming start to client */
                        bt_ull_req_t request;
                        memset(&request, 0x00, sizeof(request));
                        request.event = BT_ULL_EVENT_STREAMING_START_IND;
                        if (ep == &(ctx->dl_speaker)) {
                            if (ep->is_streaming) {
                                request.streaming_port.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                                request.streaming_port.port = 0;
                            }
                            next_ep = &(ctx->dl_chat);
                        } else if (ep == &(ctx->dl_chat)) {
                            if (ep->is_streaming) {
                                request.streaming_port.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                                request.streaming_port.port = 1;
                            }
                            next_ep = &(ctx->ul_microphone);
                        } else if (ep == &(ctx->ul_microphone)) {
                            if (ep->is_streaming) {
                                request.streaming_port.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                            }
                            next_ep = NULL;
                        }
                        if (BT_ULL_STREAMING_INTERFACE_UNKNOWN != request.streaming_port.streaming_interface) {
                            bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
                        }

                        ull_report("[ULL][API] current is allow: 0x%x, is ep streaming: 0x%x", 2, ctx->allow_play, ep->is_streaming);
                        /* if client is in call, dongle server shuold stop due to client bandwidth limitation */
                        if (BT_ULL_PLAY_ALLOW == ctx->allow_play && ep->is_streaming) {
                            /* 3. send 0xFDCB vnd cmd to controller notify ull is playing */
                            uint32_t gap_handle = bt_cm_get_gap_handle(ctx->bt_addr);
                            bt_ull_set_music_enable(gap_handle, BT_AVM_ROLE_NORMAL, true, ctx->dl_latency);
                            /* 4. start audio transmitter to notify DSP starting */
                            bt_ull_start_transmitter(ep);
                        }
                        ep = next_ep;
                    }
                }
            }
        }
    } else if (BT_GAP_ROLE_CHANGED_IND == msg) {
        bt_gap_role_changed_ind_t *role_change = (bt_gap_role_changed_ind_t *)buffer;
        if (role_change) {
            ull_report("[ULL][GAP] BT_GAP_ROLE_CHANGED_IND, local_role: 0x%x", 1, role_change->local_role);
            if (BT_ULL_ROLE_CLIENT == ctx->ull_role
                && BT_ROLE_SLAVE == role_change->local_role) {
                if (role_change->handle == bt_gap_get_handle_by_address((const bt_bd_addr_t *) & (ctx->bt_addr))) {
                    bt_ull_notify_server_play_is_allow(BT_ULL_PLAY_ALLOW);
                    if (ctx->dl_speaker.is_streaming || ctx->ul_microphone.is_streaming) {
                        bt_ull_am_play();
                    }
                }
            }
        }
    }
    return BT_STATUS_SUCCESS;
}


static void bt_ull_critial_data_timeout_callback(uint32_t timer_id, uint32_t data)
{
    //ull_report("[ULL][AVM] bt_ull_critial_data_timeout_callback, timer_id: 0x%x, data:0x%x", 2, timer_id, data);
    if (BT_ULL_CRITICAL_RX_IND_TIMER_ID == timer_id) {
        uint8_t *data_ind = (uint8_t *)data;
        bt_ull_rx_critical_user_data_t rx_data = {0};
        rx_data.user_data_length = data_ind[0];
        rx_data.user_data = &data_ind[1];
        bt_ull_event_callback(BT_ULL_EVENT_RX_CRITICAL_USER_DATA_IND, &rx_data, sizeof(rx_data));
        bt_ull_memory_free((void *)data);
    } else if (BT_ULL_CRITICAL_TX_RESULT_TIMER_ID == timer_id) {
        bt_ull_tx_critical_data_status_t tx_result;
        if (0 == data) {
            tx_result = BT_ULL_TX_CRITICAL_DATA_SUCCESS;
        } else if (1 == data) {
            tx_result = BT_ULL_TX_CRITICAL_DATA_TIMEOUT;
        } else {
            tx_result = BT_ULL_TX_CRITICAL_DATA_ABANDON;
        }
        bt_ull_event_callback(BT_ULL_EVENT_TX_CRITICAL_USER_DATA_RESULT, &tx_result, sizeof(tx_result));
    }
}

static void bt_ull_critial_data_controller_cb(bt_avm_critial_data_event_t event, void *data)
{
    uint32_t timer_data = 0;
    bt_ull_context_t *ctx = bt_ull_get_context();

    if (BT_AVM_CIRTICAL_DATA_RX_IND == event) {
        bt_avm_critial_data_t *rx_ind = (bt_avm_critial_data_t *)data;
        //ull_report("[ULL][AVM] bt_ull_critial_data rx ind, seq: 0x%x, len:0x%x, cur_seq:0x%x", 3, rx_ind->seq, rx_ind->length, ctx->critical_data_rx_seq);
        if (ctx->critical_data_rx_seq != rx_ind->seq) {  /* filter duplicate packet */
            uint16_t total_len = rx_ind->length + sizeof(rx_ind->length);
            uint8_t *data_ind = (uint8_t *)bt_ull_memory_alloc(total_len);
            data_ind[0] = rx_ind->length;
            memcpy(&data_ind[1], rx_ind->data, rx_ind->length);
            timer_data = (uint32_t)data_ind;
            bt_timer_ext_status_t status = bt_timer_ext_start(BT_ULL_CRITICAL_RX_IND_TIMER_ID, timer_data, 1, bt_ull_critial_data_timeout_callback);
            if (BT_TIMER_EXT_STATUS_SUCCESS != status) {
                ull_report_error("[ULL][AVM] bt_ull_critial_data rx ind, start timer fail !!!", 0);
                bt_ull_memory_free(data_ind);
            }
            /* for debug log */
            uint8_t temp_seq = ctx->critical_data_rx_seq;
            if (0xFF == temp_seq) {
                temp_seq = 0x01;
            } else {
                temp_seq++;
            }
            if (temp_seq != rx_ind->seq) {
                ull_report("[ULL][WARNING] bt_ull_critial_data rx lost!!!, seq: 0x%x, cur_seq:0x%x", 2, rx_ind->seq, ctx->critical_data_rx_seq);
            }
        } else {
            ull_report("[ULL][WARNING] bt_ull_critial_data rx same data, seq: 0x%x, len:0x%x, cur_seq:0x%x", 3, rx_ind->seq, rx_ind->length, ctx->critical_data_rx_seq);
        }
        ctx->critical_data_rx_seq = rx_ind->seq;
    } else if (BT_AVM_CIRTICAL_DATA_TX_RESULT == event) {
        bt_avm_critial_tx_result_t *result = (bt_avm_critial_tx_result_t *)data;
        timer_data = result->status;
        if (0 != result->status) {
            ull_report("[ULL][AVM] bt_ull_critial_data tx result, status: 0x%x, seq:0x%x", 2, result->status, result->seq);
        }
        bt_timer_ext_start(BT_ULL_CRITICAL_TX_RESULT_TIMER_ID, timer_data, 1, bt_ull_critial_data_timeout_callback);
    }
}

static bt_status_t bt_ull_critial_data_init(uint8_t max_len)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (0 == ctx->critical_data_max_len) {
        if (BT_STATUS_SUCCESS != bt_avm_critical_data_init(max_len, (void *)bt_ull_critial_data_controller_cb)) {
            ret = BT_STATUS_FAIL;
        } else {
            ctx->critical_data_max_len = max_len;
            ctx->critical_data_tx_seq = 0x01;  /* valid seq: 1 ~ 255 */
            ctx->critical_data_rx_seq = 0x00;  /* valid seq: 1 ~ 255 */
        }
        ull_report("[ULL][AVM] critical_data_init, max_len:0x%x,ret: 0x%x", 2, max_len, ret);
    } else {
        ret = BT_STATUS_FAIL;
        ull_report_error("[ULL][AVM] critical_data_init, fail due to was inited, max_len:0x%x", 1, ctx->critical_data_max_len);
    }
    return ret;
}

static bt_status_t bt_ull_critial_data_send(uint32_t gap_handle, uint16_t flush_timeout, bt_avm_critial_data_t *data)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (0 != ctx->critical_data_max_len) {
        bt_utils_assert(data && (data->length <= ctx->critical_data_max_len));
        ret = bt_avm_critical_data_send(gap_handle, flush_timeout, data);
        ull_report("[ULL][AVM] critical_data_send, flush_timeout:%d, seq:0x%x, gap_handle:0x%x, result:0x%x", 4, flush_timeout, data->seq, gap_handle, ret);
    } else {
        ret = BT_STATUS_FAIL;
        ull_report("[ULL][AVM] critical_data_send fail due to not inited", 0);
    }
    return ret;
}

void bt_ull_srv_lock_streaming(bool lock)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL] bt_ull_user_lock_play:0x%x role:0x%0x", 2, lock, ctx->ull_role);

    if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
        bt_ull_streaming_t stream;

        if (lock) {
            if (ctx->dl_speaker.is_streaming) {
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                stream.port = 0;
                bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
            }
            if (ctx->dl_chat.is_streaming) {
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                stream.port = 1;
                bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
            }
            if (ctx->ul_microphone.is_streaming) {
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                stream.port = 0;
                bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
            }
        } else {
            if (false == ctx->dl_speaker.is_streaming) {
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                stream.port = 0;
                bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
            if (false == ctx->dl_chat.is_streaming) {
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                stream.port = 1;
                bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
            if (false == ctx->ul_microphone.is_streaming) {
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                stream.port = 0;
                bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
        }
    } else if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
        if (lock) {
            /* stop current play & disallow dongle play */
            bt_ull_notify_server_play_is_allow(BT_ULL_PLAY_DISALLOW);
        } else {
            /* start current play & allow dongle play */
            bt_ull_notify_server_play_is_allow(BT_ULL_PLAY_ALLOW);
        }
    }
}

static bt_status_t bt_ull_set_version(uint32_t version)
{
    bt_ull_context_t *ctx = bt_ull_get_context();

    ull_report("[ULL] set_version: 0x%08x, is_connected:0x%0x", 2, version, ctx->is_ull_connected);
    if (version > 0 && ctx->is_ull_connected) {
       ctx->sdk_version = version;
       ctx->is_ull_version_ready = true;
       /* notify controller SDK Version info */
       BT_Set_Peer_Sdk_Version(version);
       /* notify controller media data share buffer info */
#ifdef MTK_AWS_MCE_ENABLE
       bt_ull_update_audio_buffer(BT_ULL_ROLE_SERVER, BT_ULL_EARBUDS_CLIENT);
#else
       bt_ull_update_audio_buffer(BT_ULL_ROLE_SERVER, BT_ULL_HEADSET_CLIENT);
#endif
        bt_ull_sync_connection_handle(ctx->spp_handle, BT_ULL_ROLE_SERVER);

        bt_ull_reconfig_transmitter(BT_ULL_GAMING_TRANSMITTER);
        bt_ull_reconfig_transmitter(BT_ULL_CHAT_TRANSMITTER);
        bt_ull_reconfig_transmitter(BT_ULL_MIC_TRANSMITTER);
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
        bt_ull_reconfig_transmitter(BT_ULL_LINE_IN_TRANSMITTER);
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
        bt_ull_reconfig_transmitter(BT_ULL_LINE_OUT_TRANSMITTER);
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
        bt_ull_reconfig_transmitter(BT_ULL_I2S_IN_TRANSMITTER);
#endif
        return  BT_STATUS_SUCCESS;
     }else {
        return  BT_STATUS_FAIL;
    }
}

