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

#include <string.h>
#include "atci.h"
#include "bt_ull_utility.h"
#include "bt_ull_service.h"
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "bt_ull_le_utility.h"
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
#include "bt_ull_le_utility.h"
#include "bt_ull_le_hid_service.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_le_service.h"
#include "bt_ull_le_conn_service.h"
#include "bt_ull_le_audio_manager.h"
#endif
#include "bt_connection_manager.h"
#include "bt_timer_external.h"

#include "bt_ull_audeara.h"



//defined.
#define ULL_STRPARAM(s)              s, strlen(s)

#define ULL_AIR_PAIRING_INFO    { \
        0x41, 0x69, 0x72, 0x6f, 0x68, 0x61, \
        0x41, 0x42, 0x41, 0x45, 0x46, 0x78, \
        0x49, 0x4E, 0x46, 0x4F \
}   /* Earbuds air pairing faled if air pairing informatin is not matched, every manufacturer should define pairing information */

#define ULL_AIR_PAIRING_KEY     { \
        0x01, 0x05, 0x05, 0x00, 0x0A, 0x0B, 0x0C, 0x0D, \
        0x0E, 0x0F, 0x11, 0x22, 0xA2, 0xBC, 0x32, 0x49  \
}   /* Key for air pairing, every manufacturer should define a different key */

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
static uint8_t bt_ull_atci_test_tx_seq = 0x01;
static uint8_t bt_ull_atci_cirtical_max_len = 0x00;
static uint8_t bt_ull_atci_cirtical_buf[255] = {0x01};
#endif
//static functions.
static atci_status_t bt_ull_atci_it_handler(atci_parse_cmd_param_t *parse_cmd);
static int16_t bt_ull_cmd_entry(const char *string);

static atci_cmd_hdlr_item_t g_bt_ull_atci_cmd[] = {
    {
        .command_head = "AT+BTULL",    /* INTERNAL USE, IT TEST */
        .command_hdlr = bt_ull_atci_it_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

extern bt_status_t bt_ull_le_srv_activate_up_link_handler(bt_handle_t handle);
extern void bt_ull_le_srv_change_audio_quality(bt_ull_role_t role, bt_ull_le_srv_audio_quality_t quality);
#ifdef BT_ULL_LE_THROUGHPUT_DEBUG
    extern uint32_t g_max_frame_seq;
#endif

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE) || defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
static void bt_ull_atci_timer_callback(uint32_t timer_id, uint32_t data)
{
    memset(bt_ull_atci_cirtical_buf, bt_ull_atci_test_tx_seq, bt_ull_atci_cirtical_max_len);

    bt_ull_tx_critical_user_data_t tx_data = {0};
    tx_data.flush_timeout = data;
    tx_data.user_data_length = bt_ull_atci_cirtical_max_len;
    tx_data.user_data = bt_ull_atci_cirtical_buf;
    if (BT_STATUS_SUCCESS == bt_ull_action(BT_ULL_ACTION_TX_CRITICAL_USER_DATA, &tx_data, sizeof(tx_data))) {
        if (0xFF == bt_ull_atci_test_tx_seq) {
            bt_ull_atci_test_tx_seq = 0x01;
        } else {
            bt_ull_atci_test_tx_seq++;
        }
    }
    bt_timer_ext_start(BT_ULL_ATCI_TEST_TIMER_ID, data, data, bt_ull_atci_timer_callback);
}
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
static void bt_ull_atci_enable_high_resolution_audio_hdl(const char *string)
{
    if (0 == memcmp(string, ULL_STRPARAM("enable"))) {
        string = strchr(string, ',');
        string++;
        if (0 == memcmp(string, ULL_STRPARAM("EDRLE 8M"))) {
            bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_CLIENT, BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION);
            ull_report("[ULL][ATCI] bt_ull_atci_enable_high_resolution_audio_hdl enable: EDRLE 8M !", 0);
        } else if (0 == memcmp(string, ULL_STRPARAM("EDRLE 4M"))) {
            bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_CLIENT, BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY);
            ull_report("[ULL][ATCI] bt_ull_atci_enable_high_resolution_audio_hdl enable: EDRLE 4M !", 0);
        }
        return;
    } else if (0 == memcmp(string, ULL_STRPARAM("disable"))) {
        bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_CLIENT, BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT);
        ull_report("[ULL][ATCI] bt_ull_atci_enable_high_resolution_audio_hdl disable: 0 !", 0);
        return;
    }
    ull_report_error("[ULL][ATCI] bt_ull_atci_enable_high_resolution_audio_hdl!", 0);
}
#endif
static int16_t bt_ull_cmd_entry(const char *string)
{
    ull_report("[ULL][ATCI][INJECT] string:%s", 1, string);
    /* ACTION PART */
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    if (0 == memcmp(string, ULL_STRPARAM("start_ull_client_pair"))) {
        bt_ull_pairing_info_t air_pairing_data = {
            .duration = 30,
            .role = BT_ULL_ROLE_CLIENT,
            .key = ULL_AIR_PAIRING_KEY,
            .info = ULL_AIR_PAIRING_INFO,
            .rssi_threshold = -55,
        };
        bt_ull_action(BT_ULL_ACTION_START_PAIRING, &air_pairing_data, sizeof(air_pairing_data));
    } else if (0 == memcmp(string, ULL_STRPARAM("start_ull_server_pair"))) {
        bt_ull_pairing_info_t air_pairing_data = {
            .duration = 30,
            .role = BT_ULL_ROLE_SERVER,
            .key = ULL_AIR_PAIRING_KEY,
            .info = ULL_AIR_PAIRING_INFO,
            .rssi_threshold = -55,
        };
        bt_ull_action(BT_ULL_ACTION_START_PAIRING, &air_pairing_data, sizeof(air_pairing_data));
    } else if (0 == memcmp(string, ULL_STRPARAM("stop_ull_pair"))) {
        bt_ull_action(BT_ULL_ACTION_STOP_PAIRING, NULL, 0);
    } else if (0 == memcmp(string, ULL_STRPARAM("start_music_transmitter"))) {
        bt_ull_context_t *ctx = bt_ull_get_context();
        if (AUD_ID_INVALID != ctx->dl_speaker.transmitter
            && BT_ULL_ROLE_SERVER == ctx->ull_role) {
            bt_ull_start_transmitter(&ctx->dl_speaker);
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("start_chat_transmitter"))) {
        bt_ull_context_t *ctx = bt_ull_get_context();
        if (AUD_ID_INVALID != ctx->dl_chat.transmitter
            && BT_ULL_ROLE_SERVER == ctx->ull_role) {
            bt_ull_start_transmitter(&ctx->dl_chat);
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("stop_music_transmitter"))) {
        bt_ull_context_t *ctx = bt_ull_get_context();
        if (AUD_ID_INVALID != ctx->dl_speaker.transmitter
            && BT_ULL_ROLE_SERVER == ctx->ull_role) {
            bt_ull_stop_transmitter(&ctx->dl_speaker);
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("stop_chat_transmitter"))) {
        bt_ull_context_t *ctx = bt_ull_get_context();
        if (AUD_ID_INVALID != ctx->dl_chat.transmitter
            && BT_ULL_ROLE_SERVER == ctx->ull_role) {
            bt_ull_stop_transmitter(&ctx->dl_chat);
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("start_voice_transmitter"))) {
        bt_ull_context_t *ctx = bt_ull_get_context();
        bt_ull_start_transmitter(&ctx->ul_microphone);
    } else if (0 == memcmp(string, ULL_STRPARAM("stop_voice_transmitter"))) {
        bt_ull_context_t *ctx = bt_ull_get_context();
        if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
            am_audio_side_tone_disable();
        }
        bt_ull_stop_transmitter(&ctx->ul_microphone);
    } else if (0 == memcmp(string, ULL_STRPARAM("set_mix_ratio"))) {
        bt_ull_mix_ratio_t mix_ratio;
        sscanf(string + 14, "%d %d", (int *)(&(mix_ratio.streamings[0].ratio)), (int *)(&(mix_ratio.streamings[1].ratio)));
        ull_report("[ULL][ATCI] set_mix_ratio gaming:%d, chat: %d", 2, mix_ratio.streamings[0].ratio, mix_ratio.streamings[1].ratio);
        mix_ratio.num_streaming = 2;
        mix_ratio.streamings[0].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        mix_ratio.streamings[1].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        mix_ratio.streamings[0].streaming.port = 0;
        mix_ratio.streamings[1].streaming.port = 1;
        bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MIX_RATIO, &mix_ratio, sizeof(mix_ratio));
    } else if (0 == memcmp(string, ULL_STRPARAM("init_critical_data"))) {
        uint8_t max_len;
        sscanf(string + 19, "%d", (int *)(&max_len));
        bt_ull_init_critical_user_data_t tx_init = {0};
        tx_init.max_user_data_length = max_len;
        if (BT_STATUS_SUCCESS == bt_ull_action(BT_ULL_ACTION_INIT_CRITICAL_USER_DATA, &tx_init, sizeof(tx_init))) {
            bt_ull_atci_cirtical_max_len = max_len;
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("tx_critical_data_cycle_cancel"))) {
        bt_timer_ext_stop(BT_ULL_ATCI_TEST_TIMER_ID);
    } else if (0 == memcmp(string, ULL_STRPARAM("tx_critical_data_cycle"))) {
        uint16_t flush_timeout;
        sscanf(string + 23, "%d", (int *)(&flush_timeout));
        bt_timer_ext_start(BT_ULL_ATCI_TEST_TIMER_ID, flush_timeout, flush_timeout, bt_ull_atci_timer_callback);
    } else if (0 == memcmp(string, ULL_STRPARAM("tx_critical_data"))) {
        uint16_t flush_timeout;
        sscanf(string + 17, "%d", (int *)(&flush_timeout));
        memset(bt_ull_atci_cirtical_buf, bt_ull_atci_test_tx_seq, bt_ull_atci_cirtical_max_len);
        bt_ull_tx_critical_user_data_t tx_data = {0};
        tx_data.flush_timeout = flush_timeout;
        tx_data.user_data_length = bt_ull_atci_cirtical_max_len;
        tx_data.user_data = bt_ull_atci_cirtical_buf;
        bt_ull_action(BT_ULL_ACTION_TX_CRITICAL_USER_DATA, &tx_data, sizeof(tx_data));
        if (0xFF == bt_ull_atci_test_tx_seq) {
            bt_ull_atci_test_tx_seq = 0x01;
        } else {
            bt_ull_atci_test_tx_seq++;
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("set_ULL_mode"))) {
        uint16_t ull_mode;
        bt_ull_latency_t param;
        sscanf(string + 13, "%d", (int *)(&ull_mode));
        ull_report("[ULL][ATCI] set ULL mode :%d", 1, ull_mode);
        if (ull_mode) {
            bt_ull_set_ULL_mode(true);
            param.latency = 20;
        } else {
            bt_ull_set_ULL_mode(false);
            param.latency = 25;
        }
        param.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        param.streaming.port = 0;
        bt_ull_action(BT_ULL_ACTION_SET_STREAMING_LATENCY, &param, sizeof(param));
    } else if (0 == memcmp(string, ULL_STRPARAM("set_sniff"))) {
        bt_ull_context_t *ctx = bt_ull_get_context();
        uint8_t sniff_mode = 0;
        sscanf(string + 10, "%d", (int *)(&(sniff_mode)));
        ull_report("[ULL][ATCI] set sniff mode :%d, role: 0x%x", 2, sniff_mode, ctx->ull_role);
        if (BT_ULL_ROLE_SERVER == ctx->ull_role) {
            bt_gap_connection_handle_t handle = bt_cm_get_gap_handle(ctx->bt_addr);
            bt_gap_link_policy_setting_t setting;
            if (sniff_mode) {
                setting.sniff_mode = BT_GAP_LINK_POLICY_ENABLE;
            } else {
                setting.sniff_mode = BT_GAP_LINK_POLICY_DISABLE;
            }
            bt_gap_write_link_policy(handle, &setting);
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("connect_spp_air"))) {
        bt_cm_connect_t param;
        bt_ull_context_t *ctx = bt_ull_get_context();
        memcpy(&param.address, &ctx->bt_addr, sizeof(bt_bd_addr_t));
        param.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AIR);
        bt_cm_connect(&param);
    } else if (0 == memcmp(string, ULL_STRPARAM("set_volume"))) {
        bt_ull_volume_t vol;
        sscanf(string + 11, "%d", (int *)(&(vol.action)));
        bt_ull_context_t *ctx = bt_ull_get_context();
        if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
            vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
            vol.streaming.port = 0;
            vol.channel = BT_ULL_AUDIO_CHANNEL_DUAL;
            vol.volume = 0x01;
            bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("send_user_data"))) {
        ull_report("[ULL][ATCI] send_user_data AT Test", 0);
        bt_bd_addr_t addr_list = {0};
        if (bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL), &addr_list, 1)) {
            bt_ull_user_data_t tx_data;
            uint8_t buf[5] = {0x01, 0x02, 0x03, 0x04, 0x05}; //test data
            memcpy(&(tx_data.remote_address), addr_list, sizeof(bt_bd_addr_t));
            tx_data.user_data_length = 0x05; //test len
            tx_data.user_data = buf;
            bt_ull_action(BT_ULL_ACTION_TX_USER_DATA, &tx_data, sizeof(tx_data));
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("usb_hid_control"))) {
        bt_ull_context_t *ctx = bt_ull_get_context();
        bt_ull_usb_hid_control_t control_id = 0;
        sscanf(string + 16, "%d", (int *)(&(control_id)));
        if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
            bt_ull_action(BT_ULL_ACTION_USB_HID_CONTROL, &control_id, sizeof(control_id));
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("set_air_retry_count"))) {
        bt_ull_latency_t param;
        uint8_t retry = 2;
        uint32_t base_latency = 12500; /* uint: microsecond */
        sscanf(string + 20, "%d", (int *)(&(retry)));
        bt_ull_set_ULL_mode(true);
        base_latency += (retry + 1) * 2500;
        ull_report("[ULL][ATCI] set air retry count :%d, total latency: %d microsecond", 2, retry, base_latency);
        if (base_latency % 1000) {
            base_latency = base_latency / 1000 + 1;
        } else {
            base_latency = base_latency / 1000;
        }
        param.latency = base_latency;
        param.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        param.streaming.port = 0;
        bt_ull_action(BT_ULL_ACTION_SET_STREAMING_LATENCY, &param, sizeof(param));
    } else if (0 == memcmp(string, ULL_STRPARAM("get_ull_playing_state"))) {
        bt_ull_context_t *ctx = bt_ull_get_context();
        if (BT_ULL_ROLE_CLIENT == ctx->ull_role) {
            ull_report("[ULL][ATCI] ull am_state:0x%x", 1, ctx->am_state);
        }
    } else if (0 == memcmp(string, ULL_STRPARAM("set_mic_frame_size"))) {
        bt_ull_context_t *ctx = bt_ull_get_context();
        uint8_t block_size = 0;
        sscanf(string + 19, "%d", (int *)(&(block_size)));
        if (30 == block_size
            || 47 == block_size) {
            if (false == ctx->is_ull_connected) {
                /* set frame size */
                bt_ull_set_mic_frame_size(block_size);
            }
        }
#ifdef AIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
    } else if (0 == memcmp(string, ULL_STRPARAM("set_i2s_volume"))) {
        bt_ull_volume_t vol;
        sscanf(string + 15, "%d", (int*)(&(vol.volume)));
        ull_report("[ULL][ATCI] set i2s volume:%d", 1, vol.volume);
        bt_ull_context_t* ctx = bt_ull_get_context();
        if (BT_ULL_ROLE_SERVER== ctx->ull_role) {
            vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
            vol.streaming.port = 0;
            vol.action  = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
            vol.channel = BT_ULL_AUDIO_CHANNEL_DUAL;
            vol.volume = vol.volume;
            bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
        }
#endif
    }
#endif

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    if (0 == memcmp(string, ULL_STRPARAM("critical_data_tx_cycle_start"))) {
        uint16_t flush_timeout;
        sscanf(string + 29, "%d", (int *)(&flush_timeout));
/*
        bt_ull_atci_cirtical_timeout = timeout;
        char *str = strchr(string + 29, ',');
        str ++;
        sscanf(str, "%d", (int *)(&flush_timeout));
*/
        ull_report("[ULL][ATCI] critical_data_tx_cycle_start flush_timeout:%d", 1, flush_timeout);
        bt_timer_ext_start(BT_ULL_ATCI_TEST_TIMER_ID, flush_timeout, flush_timeout, bt_ull_atci_timer_callback);
    } else if (0 == memcmp(string, ULL_STRPARAM("critical_data_tx_cycle_stop"))) {
        ull_report("[ULL][ATCI] critical_data_tx_cycle_stop", 0);
        bt_timer_ext_stop(BT_ULL_ATCI_TEST_TIMER_ID);
    } else if (0 == memcmp(string, ULL_STRPARAM("critical_data_tx_start"))) {
#ifdef BT_ULL_LE_THROUGHPUT_DEBUG
        extern uint32_t g_max_frame_seq;
        uint16_t flush_timeout, i;
        sscanf(string + 23, "%d", (int *)(&flush_timeout));
        bt_ull_atci_test_tx_seq = 0xFF;
        memset(bt_ull_atci_cirtical_buf, 1, bt_ull_atci_cirtical_max_len);
        bt_ull_tx_critical_user_data_t tx_data = {0};
        tx_data.flush_timeout = flush_timeout;
        tx_data.user_data_length = bt_ull_atci_cirtical_max_len;
        tx_data.user_data = bt_ull_atci_cirtical_buf;

        ull_report("[ULL][ATCI] throughput_test_start flush_timeout:%d", 1, flush_timeout);

        for (i = 0; i < g_max_frame_seq; i ++) {
            bt_ull_action(BT_ULL_ACTION_TX_CRITICAL_USER_DATA, &tx_data, sizeof(tx_data));
            if (0xFF == bt_ull_atci_test_tx_seq) {
                bt_ull_atci_test_tx_seq = 0x01;
            } else {
                bt_ull_atci_test_tx_seq++;
            }
        }
#endif
    } else if (0 == memcmp(string, ULL_STRPARAM("critical_data_init"))) {
#ifdef BT_ULL_LE_THROUGHPUT_DEBUG
        uint8_t max_len;
        uint16_t max_frame_seq;
        extern uint32_t g_max_frame_seq;
        sscanf(string + 19, "%d", (int *)(&max_len));

        char *str = strchr(string + 19, ',');
        str ++;
        sscanf(str, "%d", (int *)(&max_frame_seq));
        g_max_frame_seq = max_frame_seq;
        bt_ull_init_critical_user_data_t tx_init = {1};
        tx_init.max_user_data_length = max_len;
        if (BT_STATUS_SUCCESS == bt_ull_action(BT_ULL_ACTION_INIT_CRITICAL_USER_DATA, &tx_init, sizeof(tx_init))) {
            bt_ull_atci_cirtical_max_len = max_len;
            ull_report("[ULL][ATCI] critical_data_init max_len:%d, g_max_frame_seq: %d", 2, max_len, max_frame_seq);
        } else {
            ull_report("[ULL][ATCI] critical_data_init fail.", 0);
        }
#endif
    }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    else if (0 == memcmp(string, ULL_STRPARAM("change_bitrate"))) {
        uint8_t codec_index;
        sscanf(string + 15, "%d", (int *)(&(codec_index)));
        bt_ull_le_srv_switch_codec_params(BT_ULL_ROLE_CLIENT, codec_index);
    } else if (0 == memcmp(string, ULL_STRPARAM("mHDT_High-Res"))) {
        string = strchr(string, ',');
        string++;
        bt_ull_atci_enable_high_resolution_audio_hdl(string);
    } else if (0 == memcmp(string, ULL_STRPARAM("Auto_Switch_Imitate"))) {
        uint16_t conn_handle = 0x0;
        sscanf(string + 20, "%x", (int *)(&(conn_handle)));
        bt_ull_le_conn_srv_qos_report_t report = {
            .handle = conn_handle,
            .crc = 10,
            .rx_to = 10,
            .flush_to = 4,
        };
        extern void bt_ull_le_srv_qos_report_hdl(bt_status_t status, bt_ull_le_conn_srv_qos_report_t *report);
        bt_ull_le_srv_qos_report_hdl(BT_STATUS_SUCCESS, &report);
    } else if (0 == memcmp(string, ULL_STRPARAM("set_air_cig"))) {
        bt_ull_le_conn_srv_set_air_cig_params(2);
    } else if (0 == memcmp(string, ULL_STRPARAM("remove_air_cig"))) {
        bt_ull_le_conn_srv_remove_air_cig_params();
    } else if (0 == memcmp(string, ULL_STRPARAM("establish_air_cis"))) {
        bt_handle_t acl_handle;
        sscanf(string + 18, "%x", (int *)(&(acl_handle)));
        bt_ull_le_conn_srv_establish_air_cis(acl_handle);
    } else if (0 == memcmp(string, ULL_STRPARAM("disconnect_air_cis"))) {
        bt_handle_t acl_handle;
        sscanf(string + 19, "%x", (int *)(&(acl_handle)));
        bt_ull_le_conn_srv_destroy_air_cis(acl_handle);
    } else if (0 == memcmp(string, ULL_STRPARAM("switch_latency"))) {
        uint8_t latency;
        sscanf(string + 15, "%d", (int *)(&(latency)));
        bt_ull_le_conn_srv_switch_latency(latency);
    } else if (0 == memcmp(string, ULL_STRPARAM("change_aud_qual"))) {
        extern bt_status_t bt_ull_le_srv_change_audio_quality_handle(bt_handle_t acl_handle, bt_ull_le_srv_audio_quality_t audio_quality);
        uint8_t aud_qual = 0x0;
        sscanf(string + 16, "%x", (int *)(&(aud_qual)));
        bt_ull_le_srv_change_audio_quality_handle(0xFFFF, aud_qual);
    } else if (0 == memcmp(string, ULL_STRPARAM("unmute_air_cis"))) {
        bt_handle_t acl_handle;
        sscanf(string + 15, "%x", (int *)(&(acl_handle)));
        bt_ull_le_conn_srv_unmute_air_cis(acl_handle);
    } else if (0 == memcmp(string, ULL_STRPARAM("activate_uplink"))) {
        bt_handle_t acl_handle;
        sscanf(string + 16, "%x", (int *)(&(acl_handle)));
        bt_ull_le_srv_activate_up_link_handler(acl_handle);
    } else if (0 == memcmp(string, ULL_STRPARAM("DL_AM_PLAY"))) {
        bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, BT_ULL_LE_CODEC_LC3PLUS, true);
    } else if (0 == memcmp(string, ULL_STRPARAM("UL_AM_PLAY"))) {
        bt_ull_le_am_play(BT_ULL_LE_AM_UL_MODE, BT_ULL_LE_CODEC_LC3PLUS, true);
    } else if (0 == memcmp(string, ULL_STRPARAM("DL_AM_STOP"))) {
        bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true);
    } else if (0 == memcmp(string, ULL_STRPARAM("UL_AM_STOP"))) {
        bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
    } else if (0 == memcmp(string, ULL_STRPARAM("DL_AM_RESTART"))) {
        bt_ull_le_am_restart(BT_ULL_LE_AM_DL_MODE);
    } else if (0 == memcmp(string, ULL_STRPARAM("UL_AM_RESTART"))) {
        bt_ull_le_am_restart(BT_ULL_LE_AM_UL_MODE);
    } else if (0 == memcmp(string, ULL_STRPARAM("DL_AM_MUTE"))) {
        bt_ull_le_am_set_mute(BT_ULL_LE_AM_DL_MODE, true);
    } else if (0 == memcmp(string, ULL_STRPARAM("DL_AM_UNMUTE"))) {
        bt_ull_le_am_set_mute(BT_ULL_LE_AM_DL_MODE, false);
    } else if (0 == memcmp(string, ULL_STRPARAM("UL_AM_MUTE"))) {
        bt_ull_le_am_set_mute(BT_ULL_LE_AM_UL_MODE, true);
    } else if (0 == memcmp(string, ULL_STRPARAM("UL_AM_UNMUTE"))) {
        bt_ull_le_am_set_mute(BT_ULL_LE_AM_UL_MODE, false);
    } else if (0 == memcmp(string, ULL_STRPARAM("DL_AM_SET_VOLUME"))) {
        bt_ull_le_am_set_volume(BT_ULL_LE_AM_DL_MODE, 120, BT_ULL_AUDIO_CHANNEL_DUAL);
    } else if (0 == memcmp(string, ULL_STRPARAM("UL_AM_SET_VOLUME"))) {
        bt_ull_le_am_set_volume(BT_ULL_LE_AM_UL_MODE, 120, BT_ULL_AUDIO_CHANNEL_DUAL);
    } else if (0 == memcmp(string, ULL_STRPARAM("SET_MIX_RATIO"))) {
        bt_ull_mix_ratio_t mix_ratio;
        sscanf(string + 14, "%d %d", (int *)(&(mix_ratio.streamings[0].ratio)), (int *)(&(mix_ratio.streamings[1].ratio)));
        ull_report("[ULL][LE][ATCI] set_mix_ratio gaming:%d, chat: %d", 2, mix_ratio.streamings[0].ratio, mix_ratio.streamings[1].ratio);
        mix_ratio.num_streaming = 2;
        mix_ratio.streamings[0].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        mix_ratio.streamings[1].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        mix_ratio.streamings[0].streaming.port = 0;
        mix_ratio.streamings[1].streaming.port = 1;
        bt_ull_le_srv_action(BT_ULL_ACTION_SET_STREAMING_MIX_RATIO, &mix_ratio, sizeof(mix_ratio));
    } else if (0 == memcmp(string, ULL_STRPARAM("ULLV3"))) {
       bt_ull_le_scenario_t scenario_type = BT_ULL_LE_SCENARIO_ULLV3_0;
       bt_ull_le_srv_action(BT_ULL_ACTION_SET_ULL_SCENARIO, &scenario_type, sizeof(bt_ull_le_scenario_t));
    } else if (0 == memcmp(string, ULL_STRPARAM("ULLV2"))) {
       bt_ull_le_scenario_t scenario_type = BT_ULL_LE_SCENARIO_ULLV2_0;
       bt_ull_le_srv_action(BT_ULL_ACTION_SET_ULL_SCENARIO, &scenario_type, sizeof(bt_ull_le_scenario_t));
    }
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    else if (0 == memcmp(string, ULL_STRPARAM("CLEAR_BONDING_LIST"))) {
        bt_ull_le_hid_srv_clear_bonded_list(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
        bt_ull_le_hid_srv_clear_bonded_list(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
        bt_ull_le_hid_srv_clear_bonded_list(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    }
#endif
    else {
           return -1;
    }

#endif
    return 0;
}

static atci_status_t bt_ull_atci_it_handler(atci_parse_cmd_param_t *parse_cmd)
{
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    atci_response_t *response = (atci_response_t *)bt_ull_le_srv_memory_alloc(sizeof(atci_response_t));
#else
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    atci_response_t *response = (atci_response_t *)bt_ull_memory_alloc(sizeof(atci_response_t));
#endif
#endif
    if (NULL == response) {
        ull_report_error("[ULL][ATCI] malloc heap memory fail.", 0);
        return ATCI_STATUS_ERROR;
    }

    memset(response, 0, sizeof(atci_response_t));
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+BTULL=<ACTION, PARAMS> */
            if (0 == memcmp(parse_cmd->string_ptr + 5, "ULL", 3)) {
                int16_t result;
                result = bt_ull_cmd_entry(parse_cmd->string_ptr + 9);
                if (result == 0) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else {
                    snprintf((char *)response->response_buf,
                             ATCI_UART_TX_FIFO_BUFFER_SIZE,
                             "command error:%d\r\n",
                             result);
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                }
            } else {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;

        default :
            /* others are invalid command format */
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;
    }
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE )|| defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    bt_ull_le_srv_memory_free(response);
#else
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    bt_ull_memory_free(response);
#endif
#endif
    return ATCI_STATUS_OK;
}

void bt_ull_atci_init(void)
{
    atci_status_t ret;
    ret = atci_register_handler(g_bt_ull_atci_cmd, sizeof(g_bt_ull_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));

    if (ret != ATCI_STATUS_OK) {
        ull_report_error("[ULL][ATCI]Register fail!", 0);
    }
}

#if 1	// richard for BT send data
//#include "bt_type.h"
//#include "bt_ull_service.h"
//#include "bt_ull_le_service.h"
//#include "bt_ull_le_utility.h"
uint8_t send_command_type=2;		// 2: command from ab1585h to ab1571d; 3: command from ab1571d to ab1585h
uint8_t ab1585h_command_no=10;		// 0: BT status; 1:OTA staus; 2: version feed back; 3: bt_connnected_num; 4:bt address; 5: anc ha mode
uint8_t ab1585h_command_data=0;
void BT_send_data_proc(void)
{
	bt_bd_addr_t addr_list = {0};
	if (bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL), &addr_list, 1))
	{
		bt_ull_user_data_t tx_data;
		uint8_t buf[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a}; //test data
		memcpy(&(tx_data.remote_address), addr_list, sizeof(bt_bd_addr_t));
		buf[0]=send_command_type;
		buf[1]=ab1585h_command_no;
		if(ab1585h_command_no==4)
		{
			bt_bd_addr_t *local_addr = bt_device_manager_get_local_address();
			buf[2]=(*local_addr)[5];
			buf[3]=(*local_addr)[4];
			buf[4]=(*local_addr)[3];
			buf[5]=(*local_addr)[2];
			buf[6]=(*local_addr)[1];
			buf[7]=(*local_addr)[0];			
		}
		else
		{
			buf[2]=ab1585h_command_data;
		}
		tx_data.user_data_length = 10; //test len
		tx_data.user_data = buf;
		bt_ull_action(BT_ULL_ACTION_TX_USER_DATA, &tx_data, sizeof(tx_data));
//		APPS_LOG_MSGID_I("app Send user data by ULL headset", 0);
	}
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
	else
	{
		bt_bd_addr_t address_list[BT_ULL_LE_MAX_LINK_NUM];
		uint8_t i;
		bt_ull_user_data_t tx_data;
		uint8_t buf[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a}; //test data
		buf[0]=send_command_type;
		buf[1]=ab1585h_command_no;
		if(ab1585h_command_no==4)
		{
			bt_bd_addr_t *local_addr = bt_device_manager_get_local_address();
			buf[2]=(*local_addr)[5];
			buf[3]=(*local_addr)[4];
			buf[4]=(*local_addr)[3];
			buf[5]=(*local_addr)[2];
			buf[6]=(*local_addr)[1];
			buf[7]=(*local_addr)[0];			
		}
		else
		{
			buf[2]=ab1585h_command_data;
		}
		tx_data.user_data_length = 10; // test len
		tx_data.user_data = buf;
		for(i=0; i<BT_ULL_LE_MAX_LINK_NUM; i++)
		{
			if(bt_ull_le_srv_get_connected_addr_by_link_index(i, &address_list[i]) == true)
			{
				memcpy(tx_data.remote_address, address_list[i], sizeof(bt_bd_addr_t));
				bt_ull_action(BT_ULL_ACTION_TX_USER_DATA, &tx_data, sizeof(tx_data));
			}
		}
//		APPS_LOG_MSGID_I("app Send user data by ULL headset LE", 0);
	}
#endif
}

void Audeara_BT_send_notify_proc(uint8_t * data, uint16_t length)
{
   // if(AudearaGetNotificationState())
   // {
         Audeara_BT_send_data_proc(AUA_BUDSFRAME_SEND_RACE_CMD_RESP, data, length);
   // }  
}

uint8_t buffer[1200] = {0};// Max msg size
void Audeara_BT_send_data_proc(uint8_t frame, uint8_t * data, uint16_t length)
{
    bt_bd_addr_t addr_list = {0};
    uint16_t tot_length = 0;
    buffer[0] = 0x0A; // Audeara return command type
    buffer[1] = frame;
    buffer[2] = (length >> 8) & 0xFF;
    buffer[3] = (length & 0xFF);
    memcpy(&buffer[4], data, length); // Copy payload into buffer
    tot_length = length + 4;

	if (bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL), &addr_list, 1))
	{
		bt_ull_user_data_t tx_data;
		tx_data.user_data_length = tot_length; //total len, not payload length
		tx_data.user_data = buffer;
		bt_ull_action(BT_ULL_ACTION_TX_USER_DATA, &tx_data, sizeof(tx_data));
//		APPS_LOG_MSGID_I("app Send user data by ULL headset", 0);
	}
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
	else
	{
        bt_bd_addr_t address_list[BT_ULL_LE_MAX_LINK_NUM]= {0};
        uint8_t i;
		bt_ull_user_data_t tx_data;
		tx_data.user_data_length = tot_length; //total len, not payload length
		tx_data.user_data = buffer;
		for(i=0; i<BT_ULL_LE_MAX_LINK_NUM; i++)
		{
			if(bt_ull_le_srv_get_connected_addr_by_link_index(i, &address_list[i]) == true)
			{
				memcpy(tx_data.remote_address, address_list[i], sizeof(bt_bd_addr_t));
				bt_ull_action(BT_ULL_ACTION_TX_USER_DATA, &tx_data, sizeof(tx_data));
			}
		}
//		APPS_LOG_MSGID_I("app Send user data by ULL headset LE", 0);
	}
#endif
}

#endif

