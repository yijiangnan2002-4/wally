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
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "bt_type.h"
#include "bt_device_manager_le.h"
#include "bt_callback_manager.h"
#include "bt_l2cap_fix_channel.h"
#include "bt_ull_service.h"
#include "bt_ull_le_service.h"
#include "bt_ull_le_conn_service.h"
#include "bt_ull_utility.h"
#include "bt_ull_le_utility.h"
#include "bt_ull_le_audio_manager.h"
#include "bt_ull_le_audio_transmitter.h"
#include "bt_sink_srv_ami.h"
#include "bt_device_manager_power.h"
#include "bt_timer_external.h"
#include "bt_avm.h"
#include "bt_ull_le_call_service.h"
#include "bt_connection_manager.h"
#include "audio_src_srv.h"

// Audeara ULL patch
#include "bt_ull_audeara.h"
#include "race_core.h"
#include "race_cmd.h"
#include "race_xport.h"
#include "mux.h"
#include "nvdm.h"
#include "nvkey.h"
#include "bt_device_manager.h"
#include "apps_events_interaction_event.h"
#include "apps_events_event_group.h"
#include "ui_shell_manager.h"
#include "race.h"
#include "race_cmd_relay_cmd.h"

#include "app_hear_through_race_cmd_handler.h"
uint8_t reply_buf[1200] = {0}; // RX audeara message buffer

bool aua_notification_state = true; // placeholder this as true for

/**************************************************************************************************
* Define
**************************************************************************************************/

/**
 * @brief Define the macro of invalid value
 */
#define BT_ULL_LE_SRV_INVALID_VALUE            (0xFF)

/**
 * @brief Define the macro of waiting timer value.
 */
#define BT_ULL_LE_SRV_WAITING_TIMER            (1000)

/**
 * @brief Define the macro of L2CAP LE Fixed Channel.
 */
#define BT_ULL_LE_SRV_FIX_CHANNEL_CID          (0x0101)
#define BT_ULL_LE_SRV_FIX_CHANNEL_MTU          (0x3E8)

/**
 * @brief Define the value of Audio Channel Counts in Codec Specific Capabilities parameters.
 */
typedef uint8_t bt_ull_le_channel_counts_t;
#define BT_ULL_LE_EARBUDS_CHANNEL_COUNTS        0x01    /**< Channel count 1. */
#define BT_ULL_LE_HEADSET_CHANNEL_COUNTS        0x02    /**< Channel count 2. */
#define BT_ULL_LE_MIC_CHANNEL_COUNTS            BT_ULL_LE_EARBUDS_CHANNEL_COUNTS    /**< Channel count 1. */
#define BT_ULL_LE_SPK_CHANNEL_COUNTS            BT_ULL_LE_EARBUDS_CHANNEL_COUNTS    /**< Channel count 1. */

typedef uint8_t bt_ull_le_restart_streaming_reason_t;
#define BT_ULL_LE_RESTART_STREAMING_REASON_LATENCY_CHANGE       0x00
#define BT_ULL_LE_RESTART_STREAMING_REASON_ALLOW_PALY           0x01
#define BT_ULL_LE_RESTART_STREAMING_REASON_RESERVED             0x02
#define BT_ULL_LE_RESTART_STREAMING_REASON_AIRCIS_RECONNECT     0x03
#define BT_ULL_LE_RESTART_STREAMING_REASON_AUD_QUALITY_CHANGE   0x04

typedef uint32_t bt_ull_le_srv_feature_t;
#define BT_ULL_LE_SRV_FEATURE_MHDT_4M_PHY        0x1
#define BT_ULL_LE_SRV_FEATURE_MHDT_8M_PHY        0x2
#define BT_ULL_LE_SRV_IS_SUPPORT(IDX, _F)      (g_ull_le_link_info[IDX].feature_msk & _F)

/**************************************************************************************************
* Structure
**************************************************************************************************/

/**
 * @brief This structure defines the information of ULL Dongle AirCIS.
 */
typedef struct {
    bool        dl_enable;
    bool        ul_enable;
} bt_ull_le_srv_cis_info_t;

/**
 * @brief This structure defines the parameters of ULL Dongle Link.
 */
typedef struct {
    bt_ull_le_link_state_t curr_state;               /**< state machine. */
    bt_handle_t conn_handle;                         /**< connection handle. */
    bt_ull_le_srv_cis_info_t cis_info;
    bt_bd_addr_t addr;                               /**< peer device address. */
    uint32_t max_packet_size;                        /**< MTU. */
    bt_ull_le_srv_configuration_t sink_cfg;
    bool ul_active;
    bool is_cig_table_set;
    bt_ull_allow_play_t allow_play;
    uint8_t latency;
    bt_ull_le_srv_audio_quality_t aud_quality;
    bt_addr_t group_device_addr[BT_ULL_LE_MAX_LINK_NUM - 1];
    bt_ull_le_srv_capability_t       peer_capability_msk;
    bt_ull_le_srv_feature_t          feature_msk;
} bt_ull_le_srv_link_info_t; /** by link. */


typedef struct {
    uint8_t    expect_aud_quality;
} bt_ull_le_srv_aud_quality_info_t;

/**************************************************************************************************
* Variable
**************************************************************************************************/

bt_ull_le_srv_context_t g_ull_le_ctx = {0};
static bt_ull_le_srv_link_info_t g_ull_le_link_info[BT_ULL_LE_MAX_LINK_NUM];
static bt_ull_le_srv_aud_quality_info_t g_ull_le_aud_quality_info = {0};
static bt_ull_le_srv_capability_t g_bt_ull_le_capability = {0};
static bt_addr_t g_group_device_addr[BT_ULL_LE_MAX_LINK_NUM - 1] = {0}; /*for client use*/
static bt_ull_le_scenario_t g_ull_scenario_type = BT_ULL_LE_SCENARIO_INVALID;
static bool g_ull_le_wait_audio_quality = false;
/**************************************************************************************************
* Prototype
**************************************************************************************************/
static void bt_ull_le_srv_rx_data_handle(uint16_t handle, uint8_t *data ,uint16_t length);
static bt_status_t bt_ull_le_srv_connection_handle(uint16_t handle, bt_ull_connection_state_t state);
static bt_status_t bt_ull_le_srv_handle_start_streaming(bt_ull_streaming_t *streaming);
static bt_status_t bt_ull_le_srv_handle_stop_streaming(bt_ull_streaming_t *streaming);
static bool bt_ull_le_srv_check_all_downlink_stream_is_mute(bt_ull_transmitter_t trans_type);
static bt_status_t bt_ull_le_srv_handle_set_volume(bt_ull_volume_t *vol);
static bt_status_t bt_ull_le_srv_handle_set_streaming_mute(bt_ull_streaming_t *streaming, bool is_mute);
static bt_status_t bt_ull_le_srv_handle_set_streaming_sample_rate(bt_ull_sample_rate_t *sample_rate);
static bt_status_t bt_ull_le_srv_handle_set_streaming_sample_size(bt_ull_streaming_sample_size_t *sample_size);
static bt_status_t bt_ull_le_srv_handle_set_streaming_sample_channel(bt_ull_streaming_sample_channel_t *sample_channel);
static bt_status_t bt_ull_le_srv_handle_set_streaming_latency(bt_ull_latency_t *req_latency, bt_handle_t handle);
static bt_status_t bt_ull_le_srv_handle_set_streaming_mix_ratio(bt_ull_mix_ratio_t *ratio);
static bt_status_t bt_ull_le_srv_handle_usb_hid_control(bt_ull_usb_hid_control_t action);
static bt_status_t bt_ull_le_srv_handle_host_event_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);
static bt_status_t bt_ull_le_srv_critial_data_init(uint8_t max_len);
static bt_status_t bt_ull_le_srv_critial_data_send(bt_handle_t handle, uint16_t flush_timeout, bt_avm_critial_data_t* data);
static bt_status_t bt_ull_le_srv_handle_find_cs(bt_ull_le_find_cs_info_t *cs_data);
static void bt_ull_le_srv_start_audio_transmitter_callback(bt_ull_transmitter_t transmitter_type, bt_status_t status);
static void bt_ull_le_srv_stop_audio_transmitter_callback(bt_ull_transmitter_t transmitter_type, bt_status_t status);
static void bt_ull_le_srv_play_am_callback(bt_ull_le_am_mode_t mode, bt_status_t status);
static void bt_ull_le_srv_stop_am_callback(bt_ull_le_am_mode_t mode, bt_status_t status);
static bt_status_t bt_ull_le_srv_calculate_air_cis_count(void);
static bt_status_t bt_ull_le_srv_open_audio_stream(bt_ull_le_stream_port_mask_t stream_port);
static void bt_ull_le_srv_init_audio_location_by_client_type(bt_ull_client_t client_type);
static bt_status_t bt_ull_le_srv_send_configuration(bt_handle_t handle);
bt_status_t bt_ull_le_srv_activate_up_link_handler(bt_handle_t handle);
void bt_ull_le_srv_indicate_server_play_is_allow(bt_ull_allow_play_t is_allow, uint8_t reason);
bt_status_t bt_ull_le_srv_disable_streaming(void);
static bt_status_t bt_ull_le_srv_play_am(bt_ull_streaming_t *streaming, bt_ull_le_stream_mode_t stream_mode);
bt_status_t bt_ull_le_srv_notify_client_restart_streaming(bt_ull_le_restart_streaming_reason_t reason);
bool bt_ull_le_srv_is_connected(void);

static bool bt_ull_le_srv_is_any_aircis_connected(void);

bt_status_t bt_ull_le_srv_change_ull_scenario_handle(bt_ull_le_scenario_t scenario_type);
static void bt_ull_le_srv_set_scenario_type(bt_ull_le_scenario_t scenario_type);
static bt_ull_le_scenario_t bt_ull_le_srv_get_scenario_type(void);

bt_status_t bt_ull_le_srv_change_audio_quality_handle(bt_handle_t acl_handle, bt_ull_le_srv_audio_quality_t audio_quality);
static bt_status_t bt_ull_le_srv_enable_adaptive_bitrate_mode_internal(bt_handle_t handle, bt_ull_le_adaptive_bitrate_params_t *adaptive_bitrate_param);

void bt_ull_le_srv_qos_report_hdl(bt_status_t status, bt_ull_le_conn_srv_qos_report_t *report);
static void bt_ull_le_srv_reset_timer(uint32_t timer_id, uint32_t tm_ms);
static uint8_t bt_ull_le_srv_get_idx_by_handle(bt_handle_t acl_handle);

static bt_status_t bt_ull_le_srv_client_switch_uplink_handler(bool switch_ul);
static void bt_ull_le_srv_switch_uplink(bt_handle_t handle, bool switch_ul);
static bt_status_t bt_ull_le_srv_sync_streaming_status(bt_ull_le_stream_port_mask_t stream_port, uint8_t event);
static uint8_t bt_ull_le_srv_get_connected_link_num(void);

#ifdef AIR_WIRELESS_MIC_ENABLE
static void bt_ull_le_srv_set_client_preferred_channel_mode(bt_ull_le_channel_mode_t  channel_mode);
static bt_ull_le_channel_mode_t bt_ull_le_srv_get_client_preferred_channel_mode(void);
#endif
static bt_status_t bt_ull_le_srv_notify_client_auido_quality(bt_ull_le_srv_audio_quality_t aud_quality);
static void bt_ull_le_srv_aud_quality_change_retry_timeout_callback(uint32_t timer_id, uint32_t data);
static bt_status_t bt_ull_le_srv_disable_adaptive_bitrate_mode(void);
static bt_status_t bt_ull_le_srv_send_audio_quality_by_handle(bt_handle_t handle, bt_ull_le_srv_audio_quality_t aud_quality);

/**************************************************************************************************
* Functions
**************************************************************************************************/
#if defined(AIR_USB_AUDIO_ENABLE) && defined(AIR_USB_AUDIO_1_MIC_ENABLE)
extern uint32_t USB_Audio_Get_TX_Sample_Rate(uint32_t port);
#else
static uint32_t USB_Audio_Get_TX_Sample_Rate(uint32_t port)
{
    return 0;
}
#endif

#ifdef AIR_USB_AUDIO_ENABLE
extern uint32_t USB_Audio_Get_RX_Sample_Rate(uint32_t port);
#else
static uint32_t USB_Audio_Get_RX_Sample_Rate(uint32_t port)
{
    return 0;
}
#endif

#if 1
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
static void default_bt_ull_wireless_mic_stop_volume_monitor(void)
{
    return;
}
extern void bt_ull_wireless_mic_stop_volume_monitor(void);
#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_ull_wireless_mic_stop_volume_monitor = _default_bt_ull_wireless_mic_stop_volume_monitor")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak bt_ull_wireless_mic_stop_volume_monitor = default_bt_ull_wireless_mic_stop_volume_monitor
#else
#error "Unsupported Platform"
#endif
#endif
#endif

extern void bt_ull_le_set_audio_manager_priority(audio_src_srv_priority_t priority);

bt_ull_le_srv_context_t *bt_ull_le_srv_get_context(void)
{
    return &g_ull_le_ctx;
}

bt_ull_le_srv_aud_quality_info_t *bt_ull_le_srv_get_aud_quality_info(void)
{
    return &g_ull_le_aud_quality_info;
}

void bt_ull_le_srv_init_audio_location(uint32_t audio_location)
{
    g_ull_le_ctx.audio_location = audio_location;
}

void bt_ull_le_srv_set_client_type(bt_ull_client_t client_type)
{
    g_ull_le_ctx.client_type = client_type;
}

static void bt_ull_le_srv_init_audio_location_by_client_type(bt_ull_client_t client_type)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();

    if (BT_ULL_HEADSET_CLIENT == client_type || BT_ULL_SPEAKER_CLIENT == client_type) {
        ctx->audio_location = BT_ULL_LE_AUDIO_LOCATION_FRONT_LEFT | BT_ULL_LE_AUDIO_LOCATION_FRONT_RIGHT;
    } else if (BT_ULL_EARBUDS_CLIENT == client_type) {
        audio_channel_t channel = ami_get_audio_channel();
        ctx->audio_location = (channel == AUDIO_CHANNEL_NONE) ? BT_ULL_LE_AUDIO_LOCATION_NONE : (channel == AUDIO_CHANNEL_R) ? BT_ULL_LE_AUDIO_LOCATION_FRONT_RIGHT : BT_ULL_LE_AUDIO_LOCATION_FRONT_LEFT;
    } else {
        ctx->audio_location = BT_ULL_LE_AUDIO_LOCATION_NONE;
    }
    ull_report("[ULL][LE] bt_ull_le_srv_init_audio_location_by_client_type ct: %d, audio_location: %x", 2, client_type, ctx->audio_location);
}

bt_status_t bt_ull_le_srv_set_device_info(bt_ull_le_device_info_t *dev_info)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    if (dev_info && ctx) {
        if ((BT_ULL_UNKNOWN_CLIENT == dev_info->client_type) || 
            (BT_ULL_INVALID_CLIENT <= dev_info->client_type) ||
            (BT_STATUS_SUCCESS != bt_ull_le_srv_set_sirk(dev_info->sirk)) ||
            (BT_STATUS_SUCCESS != bt_ull_le_srv_set_coordinated_set_size(dev_info->size))) {
            ull_report_error("[ULL][LE] bt_ull_le_srv_set_device_info, fail, client_type: %d, size: %d", 3, dev_info->client_type, dev_info->size, bt_ull_le_srv_set_sirk(dev_info->sirk));
            return BT_STATUS_FAIL;
        }
        bt_ull_le_srv_set_client_type(dev_info->client_type);
        bt_ull_le_srv_set_group_device_addr(dev_info->group_device_addr);
    #ifdef AIR_WIRELESS_MIC_ENABLE
        if (BT_ULL_MIC_CLIENT == dev_info->client_type) {
        #ifdef AIR_AUDIO_ULD_CODEC_ENABLE
            bt_ull_le_srv_set_client_preferred_channel_mode(BT_ULL_LE_CHANNEL_MODE_MONO);
        #else
            bt_ull_le_srv_set_client_preferred_channel_mode(dev_info->client_preferred_channel_mode);
        #endif
            ull_report("[ULL][LE] Server(Wireless Mic TX) update codec param", 0);
            bt_ull_le_srv_set_wireless_mic_codec_param(BT_ULL_ROLE_CLIENT);
        }
    #else
        bt_ull_le_srv_set_client_preferred_codec_type(dev_info->client_preferred_codec_type);
        //default use Lc3pus codec, if use opus codec, update codec param.
        if (BT_ULL_LE_CODEC_OPUS == bt_ull_le_srv_get_client_preferred_codec_type()) {
            bt_ull_le_srv_set_opus_codec_param();
        }
    #endif
        bt_ull_le_srv_memcpy(&(ctx->local_random_addr), &(dev_info->local_random_addr), BT_BD_ADDR_LEN);
        bt_ull_le_srv_init_audio_location_by_client_type(dev_info->client_type);
        ull_report("[ULL][LE] bt_ull_le_srv_set_device_info success, client_type: %d, size: %d, addr: %x-%x-%x-%x-%x-%x", 8, \
            dev_info->client_type, dev_info->size, dev_info->local_random_addr[0], dev_info->local_random_addr[1], dev_info->local_random_addr[2], 
            dev_info->local_random_addr[3], dev_info->local_random_addr[4], dev_info->local_random_addr[5]);
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_FAIL;
}

bt_status_t bt_ull_le_srv_set_codec_info(bt_ull_le_set_codec_port_t port, bt_ull_le_codec_t codec_type, bt_ull_le_codec_param_t *codec_param)
{
    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if ((BT_ULL_ROLE_SERVER != ull_ctx->role) && (BT_ULL_ROLE_CLIENT != ull_ctx->role)) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_set_codec_info, fail, current role is %d, not support set codec", 1, ull_ctx->role);
        return BT_STATUS_FAIL;
    }
    stream_ctx->is_same_codec = (BT_ULL_LE_SET_CODEC_PORT_ALL_SAME == port) ? true : false;
    stream_ctx->codec_type = codec_type;

    if (BT_ULL_ROLE_SERVER == ull_ctx->role) {
        switch (port) {
            case BT_ULL_LE_SET_CODEC_PORT_ALL_SAME: {
                uint8_t i;
                for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
                    bt_ull_le_srv_memcpy(&(stream_ctx->server.stream[i]), codec_param, sizeof(bt_ull_le_codec_param_t));
                }
            } break;

            case BT_ULL_LE_SET_CODEC_GAMING_SPK_PORT:
            case BT_ULL_LE_SET_CODEC_CHAT_SPK_PORT:
            case BT_ULL_LE_SET_CODEC_MIC_PORT:
            case BT_ULL_LE_SET_CODEC_LINE_IN_PORT:
            case BT_ULL_LE_SET_CODEC_LINE_OUT_PORT:
            case BT_ULL_LE_SET_CODEC_I2S_IN_PORT:
            case BT_ULL_LE_SET_CODEC_I2S_OUT_PORT: {
                bt_ull_le_srv_memcpy(&(stream_ctx->server.stream[port - 1].codec_param), codec_param, sizeof(bt_ull_le_codec_param_t));
            } break;

            default:
                return BT_STATUS_FAIL;
        }
    } else if (BT_ULL_ROLE_CLIENT == ull_ctx->role) {
        switch (port) {
            case BT_ULL_LE_SET_CODEC_PORT_ALL_SAME: {
                bt_ull_le_srv_memcpy(&(stream_ctx->client.dl.codec_param), codec_param, sizeof(bt_ull_le_codec_param_t));
                bt_ull_le_srv_memcpy(&(stream_ctx->client.ul.codec_param), codec_param, sizeof(bt_ull_le_codec_param_t));
            } break;

            case BT_ULL_LE_SET_CODEC_GAMING_SPK_PORT:
            case BT_ULL_LE_SET_CODEC_CHAT_SPK_PORT:
            case BT_ULL_LE_SET_CODEC_LINE_IN_PORT:
            case BT_ULL_LE_SET_CODEC_I2S_IN_PORT: {
                bt_ull_le_srv_memcpy(&(stream_ctx->client.dl.codec_param), codec_param, sizeof(bt_ull_le_codec_param_t));
            } break;

            case BT_ULL_LE_SET_CODEC_MIC_PORT:
            case BT_ULL_LE_SET_CODEC_LINE_OUT_PORT:
            case BT_ULL_LE_SET_CODEC_I2S_OUT_PORT: {
                bt_ull_le_srv_memcpy(&(stream_ctx->client.ul.codec_param), codec_param, sizeof(bt_ull_le_codec_param_t));
            } break;

            default:
                return BT_STATUS_FAIL;
        }
    }
    ull_report("[ULL][LE] bt_ull_le_srv_set_codec_info,is_same_codec: %d, codec type: %d", 2, stream_ctx->is_same_codec, codec_type);
    return BT_STATUS_SUCCESS;
}

static bt_ull_le_srv_link_info_t *bt_ull_le_srv_get_link_info(bt_handle_t handle)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    uint8_t i = 0;
    uint8_t max_link_count = (BT_ULL_ROLE_SERVER == ctx->role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;
    while (i < max_link_count) {
        if (handle == g_ull_le_link_info[i].conn_handle) {
            return &g_ull_le_link_info[i];
        }
        i++;
    }
    ull_report_error("[ULL][LE] bt_ull_le_srv_get_link_info fail, conn handle:0x%x", 1, handle);
    return NULL;
}

bt_status_t bt_ull_le_srv_get_streaming_info(bt_ull_streaming_t streaming, bt_ull_streaming_info_t *info)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_STREAMING_INTERFACE_MAX_NUM <= streaming.streaming_interface) {
        return BT_STATUS_FAIL;
    }

    if (info) {
        bt_ull_transmitter_t trans = bt_ull_le_srv_get_transmitter_by_stream_interface(&streaming);
        if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming.streaming_interface) {
            if (1 < streaming.port) {
                return BT_STATUS_FAIL;
            }

            if (0 == streaming.port) {
                if (BT_ULL_ROLE_CLIENT == ctx->role) {
                    if (stream_ctx->client.dl.is_am_open) {
                        info->is_playing = true;
                    } else {
                        info->is_playing = false;
                    }
                } else {
                    info->is_playing = (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) ? true : false;
                }
            } else if (1 == streaming.port) {
                info->is_playing = (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) ? true : false;
            }
                info->latency = stream_ctx->dl_latency;
        } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming.streaming_interface) {
            info->is_playing = (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) ? true : false;
            if (stream_ctx->dl_latency > BT_ULL_DOWNLINK_SINGLE_LIMITATION) {
                info->latency = BT_ULL_UPLINK_MAX;
            } else {
                info->latency = BT_ULL_UPLINK_DEFAULT;
            }
        }
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        else if (BT_ULL_STREAMING_INTERFACE_LINE_IN == streaming.streaming_interface) {
                info->is_playing = (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_IN) ? true : false;
                info->latency = stream_ctx->dl_latency;
        }
#endif

#if (defined AIR_GAMING_MODE_DONGLE_V2_LINE_OUT_ENABLE) || (defined AIR_WIRELESS_MIC_ENABLE)
        else if (BT_ULL_STREAMING_INTERFACE_LINE_OUT == streaming.streaming_interface) {
                info->is_playing = (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT) ? true : false;
                info->latency = stream_ctx->dl_latency;
        }
#endif

#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
        else if (BT_ULL_STREAMING_INTERFACE_I2S_IN == streaming.streaming_interface) {
                info->is_playing = (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_IN) ? true : false;
                info->latency = stream_ctx->dl_latency;
        }
#endif
#if (defined AIR_DONGLE_I2S_SLV_OUT_ENABLE) || (defined AIR_DONGLE_I2S_MST_OUT_ENABLE) || (defined AIR_WIRELESS_MIC_ENABLE)
        else if (BT_ULL_STREAMING_INTERFACE_I2S_OUT == streaming.streaming_interface) {
                info->is_playing = (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT) ? true : false;
                info->latency = stream_ctx->dl_latency;
        }
#endif

        info->sample_rate = stream_ctx->server.stream[trans].usb_sample_rate;
        info->volume_left = stream_ctx->server.stream[trans].volume.vol_left;
        info->volume_right = stream_ctx->server.stream[trans].volume.vol_right;
    } else {
        status = BT_STATUS_FAIL;
    }

    if (BT_STATUS_SUCCESS == status) {
        ull_report("[ULL][LE] bt_ull_le_srv_get_streaming_info, stream_port: %d, volume_left: %d, latency: %d, is_playing:0x%x, sample_rate: %d", 5,
            stream_ctx->streaming_port, info->volume_left, info->latency, info->is_playing, info->sample_rate);
    }
    return status;
}

static void bt_ull_le_srv_conn_waiting_timeout_hdl(uint32_t timer, uint32_t data)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_srv_link_info_t *link_info = NULL;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context(); 

	if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
        /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server]: Timer timeout, check which link should establish aircis, for earbuds project*/
        uint8_t max_link_count = BT_ULL_LE_MAX_LINK_NUM;
        uint8_t i = 0;
        while (i < max_link_count) {
            if ((BT_ULL_LE_LINK_STATE_READY == g_ull_le_link_info[i].curr_state) && (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle)) {
                link_info = &g_ull_le_link_info[i];
        	    if (!stream_ctx->allow_play) {
        			if (!ctx->is_removing_cig_for_switch_latency || !ctx->is_removing_cig_for_change_aud_quality || !ctx->is_removing_cig_for_change_aud_codec) {
        				if (!ctx->is_cig_created) {
        					/*Set Air CIG parameters. */
        					if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_set_air_cig_params(ctx->cis_num)) {
        						bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
        					} else {
        						ull_report_error("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event, set cig error", 0);
        					}
        				} else {
        					if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER > ctx->curr_stream_state) {
        						bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
        					}
        					if (BT_ULL_LE_LINK_STATE_READY == link_info->curr_state) {
        						/* Create CIS */
        						if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(link_info->conn_handle)) {
        							link_info->curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
        						}
        					} else if (BT_ULL_LE_LINK_STATE_STREAMING == link_info->curr_state) {
        						/**<Air CIS has been created done and open audio transmitter successfully. */
        						if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER == ctx->curr_stream_state)
        							bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
        					}
        				}
        	        }
                }
            }
            i++;
        }
	}


    ull_report("[ULL][LE] bt_ull_le_srv_conn_waiting_timeout_hdl, straming_port mask:0x%x", 1, stream_ctx->streaming_port);
    if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING)) {
        if (!stream_ctx->is_silence) {
            bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_GAMING, BT_ULL_EVENT_STREAMING_START_IND);
        }
    }
    if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT)) {
        if (!stream_ctx->is_silence) {
            bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_CHAT, BT_ULL_EVENT_STREAMING_START_IND);
        }

    }
    if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC)) {
        bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_MIC, BT_ULL_EVENT_STREAMING_START_IND);
    }
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
    if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_IN) {
        bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_LINE_IN, BT_ULL_EVENT_STREAMING_START_IND);
    }
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_LINE_OUT_ENABLE) || (defined AIR_WIRELESS_MIC_ENABLE)
    if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT) {
        bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT, BT_ULL_EVENT_STREAMING_START_IND);
    }
#endif
#if (defined AIR_DONGLE_I2S_SLV_OUT_ENABLE) || (defined AIR_DONGLE_I2S_MST_OUT_ENABLE) || (defined AIR_WIRELESS_MIC_ENABLE)
    if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT) {
        bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT, BT_ULL_EVENT_STREAMING_START_IND);
    }
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
    if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_IN) {
        bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_I2S_IN, BT_ULL_EVENT_STREAMING_START_IND);
    }
#endif

}

static void bt_ull_le_srv_reset_link_info(void)
{
    uint8_t i = (BT_ULL_ROLE_SERVER == g_ull_le_ctx.role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        bt_ull_le_srv_memset(&(g_ull_le_link_info[i]), 0x00, sizeof(bt_ull_le_srv_link_info_t));
        g_ull_le_link_info[i].conn_handle = BT_HANDLE_INVALID;
    }
    ull_report("[ULL][LE] reset link info, YES!", 0);
}

bool bt_ull_le_srv_is_connected(void)
{
    uint8_t i = (BT_ULL_ROLE_SERVER == g_ull_le_ctx.role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        if (BT_HANDLE_INVALID == g_ull_le_link_info[i].conn_handle) {
            continue;
        }
        if (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
            return true;
        }
    }
    return false;
}

static bt_status_t bt_ull_le_srv_disconnect_aircis(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t i = (BT_ULL_ROLE_SERVER == g_ull_le_ctx.role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        if (BT_HANDLE_INVALID == g_ull_le_link_info[i].conn_handle) {
            continue;
        }
        if (BT_ULL_LE_LINK_STATE_READY < g_ull_le_link_info[i].curr_state) {
            status = bt_ull_le_conn_srv_destroy_air_cis(g_ull_le_link_info[i].conn_handle);
            ull_report("[ULL][LE] bt_ull_le_srv_disconnect_aircis, conn_handle: 0x%x, status: 0x%x", 2, g_ull_le_link_info[i].conn_handle, status);
        }
    }
    ull_report("[ULL][LE] bt_ull_le_srv_disconnect_aircis end, status: 0x%x", 1, status);
    return status;
}

static void bt_ull_le_srv_handle_audio_transmitter_event(bt_ull_le_at_event_t event, void *param, uint32_t param_len)
{
    switch (event) {
        case BT_ULL_LE_AT_EVENT_START_IND: {
            bt_ull_le_at_result_t *start_ind = (bt_ull_le_at_result_t *)param;
            bt_ull_le_srv_start_audio_transmitter_callback(start_ind->transmitter_type, start_ind->result);
        } break;

        case BT_ULL_LE_AT_EVENT_STOP_IND: {
            bt_ull_le_at_result_t *stop_ind = (bt_ull_le_at_result_t *)param;
            bt_ull_le_srv_stop_audio_transmitter_callback(stop_ind->transmitter_type, stop_ind->result);
        } break;

        default:
            break;
    }
}

static void bt_ull_le_srv_handle_audio_manager_event(bt_ull_le_am_event_t event, void *data, uint32_t data_len)
{
    switch (event) {
        case BT_ULL_LE_AM_PLAY_IND: {
            bt_ull_le_am_result_t *play_ind = (bt_ull_le_am_result_t *)data;
            bt_ull_le_srv_play_am_callback(play_ind->mode, play_ind->result);
        } break;

        case BT_ULL_LE_AM_STOP_IND: {
            bt_ull_le_am_result_t *stop_ind = (bt_ull_le_am_result_t *)data;
            bt_ull_le_srv_stop_am_callback(stop_ind->mode, stop_ind->result);
        } break;

        default:
            break;
    }
}

static void bt_ull_le_srv_handle_aircis_event(bt_ull_le_conn_srv_event_t event, void *data)
{
    uint8_t i;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_link_info_t *link_info;

    switch (event) {
        case BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_PARAMS_SET_DONE_IND: {
            bt_ull_le_conn_srv_air_cis_evt_ind_t *set_cig_ind = (bt_ull_le_conn_srv_air_cis_evt_ind_t *)data;
            ull_report("[ULL][LE] AirCIG PARAMS SET DONE IND, status: %x, state: %d", 2, set_cig_ind->status, ctx->curr_stream_state);
            if (BT_STATUS_SUCCESS == set_cig_ind->status) {
                ctx->is_cig_created = true;
                if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER == ctx->curr_stream_state) {
                    if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                        /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server] Keep cis always alive when cig set done*/
                        ull_report("[ULL][LE][BOB][DEBUG] AirCIG PARAMS SET DONE IND, not check port streaming and always establish aircis!!", 0);
                    } else {
                        if (!bt_ull_le_srv_is_any_streaming_started(ctx->role)) {
                            //ctx->curr_stream_state = BT_ULL_LE_SRV_STREAM_STATE_IDLE;
                            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_IDLE);
                            ull_report_error("[ULL][LE] AirCIG PARAMS SET DONE IND, No any streaming!!!", 0);
                            break;
                        }
                    }
                    for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
                        if (BT_ULL_LE_LINK_STATE_READY == g_ull_le_link_info[i].curr_state) {
                            /* Create CIS */
                            if (BT_ULL_SPEAKER_CLIENT == bt_ull_le_srv_get_client_type()) {
                                if (BT_ULL_PLAY_ALLOW == g_ull_le_link_info[i].allow_play) {
                                    if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(g_ull_le_link_info[i].conn_handle)) {
                                        g_ull_le_link_info[i].curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                                    }
                                }
                            } else {
                                if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(g_ull_le_link_info[i].conn_handle)) {
                                    g_ull_le_link_info[i].curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                                }
                            }
                        } else if (BT_ULL_LE_LINK_STATE_STREAMING == g_ull_le_link_info[i].curr_state) {
                            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                        }
                    }
                    ull_report("[ULL][LE] Set AirCIG Success, curr_stream_state is 0x%x", 1, ctx->curr_stream_state);
                }
            }
        } break;

        case BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND: {
            bt_ull_le_conn_srv_air_cis_evt_ind_t *cis_conn = (bt_ull_le_conn_srv_air_cis_evt_ind_t *)data;
            if (BT_HANDLE_INVALID == cis_conn->established.handle) {
                ull_report_error("[ULL][LE] AIRCIS ESTABLISHED IND, invlid handle", 0);
                break;
            }
            link_info = bt_ull_le_srv_get_link_info(cis_conn->established.handle);
            bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
            if (link_info) {
                if (BT_STATUS_SUCCESS != cis_conn->status) {
                    ull_report_error("[ULL][LE] BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND, %x, %d", 2, cis_conn->status, link_info->allow_play);
                    if (BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES == cis_conn->status
                        && (BT_ULL_PLAY_ALLOW == link_info->allow_play)) {
                        if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(link_info->conn_handle)) {
                            link_info->curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                        } else {
                            link_info->curr_state = BT_ULL_LE_LINK_STATE_READY;
                        }
                    } else {
                        link_info->curr_state = BT_ULL_LE_LINK_STATE_READY;
                    }
                    break;
                }
                if (bt_ull_le_srv_check_inactive_aircis_feature_on() && (BT_ULL_ROLE_SERVER == ctx->role)) {
                    /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server] Not disconnect CIS if no streaming*/
                    ull_report("[ULL][LE][BOB][DEBUG] AIRCIS ESTABLISHED IND, not disconnect aircis if no streaming!!", 0);
                } else {
                    if (!bt_ull_le_srv_is_any_streaming_started(ctx->role) && BT_ULL_ROLE_SERVER == ctx->role) {
    					bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_IDLE);
                        ull_report_error("[ULL][LE] [ULL][LE] AIRCIS ESTABLISHED IND, No any streaming!!!", 0);
                        bt_ull_le_srv_disconnect_aircis();
                        break;
                    }
                }
                link_info->cis_info.dl_enable = cis_conn->established.dl_enable;
                link_info->cis_info.ul_enable = cis_conn->established.ul_enable;
                /*update status to set data path*/
                if ((BT_ULL_ROLE_SERVER == ctx->role) && (BT_ULL_LE_LINK_STATE_CREATING_CIS == link_info->curr_state)) {
                    link_info->curr_state = BT_ULL_LE_LINK_STATE_SETTING_DATA_PATH;
                } else if ((BT_ULL_ROLE_CLIENT == ctx->role) && (BT_ULL_LE_LINK_STATE_READY == link_info->curr_state)) {
                    link_info->curr_state = BT_ULL_LE_LINK_STATE_SETTING_DATA_PATH;
                }
                if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                    /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server] Active ISO if PC UL or DL is streaming*/
                    ull_report("[ULL][LE][BOB][DEBUG] AIRCIS ESTABLISHED Success. Check port is in streaming, set Iso data path!", 0);
                    if (BT_ULL_ROLE_SERVER == ctx->role && bt_ull_le_srv_is_any_streaming_started(BT_ULL_ROLE_SERVER)) {
                        bt_ull_le_srv_active_streaming(); /*set iso data path*/
                    } else if (BT_ULL_ROLE_CLIENT == ctx->role && (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK) || bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) && bt_ull_le_srv_is_any_streaming_started(BT_ULL_ROLE_CLIENT)) {
                        if (bt_ull_le_srv_is_any_streaming_started(BT_ULL_ROLE_CLIENT)) {
                            bt_ull_le_srv_active_streaming(); /*set iso data path*/
                        } else {
                            bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
                        }
                    }                    
                }
                ull_report("[ULL][LE] AIRCIS ESTABLISHED Success, Link curr_state is 0x%x", 1, link_info->curr_state);
            } else {
                ull_report_error("[ULL][LE] AIRCIS ESTABLISHED Fail, can not find link info!", 0);
                break;
            }

            if (BT_ULL_ROLE_SERVER == ctx->role && BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
                if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC && \
                    cis_conn->established.ul_enable) {
                    bt_ull_le_at_set_ul_channel_locaton(link_info->sink_cfg.audio_location);
                }
            }
            if (BT_ULL_ROLE_CLIENT == ctx->role && BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
                link_info->ul_active = cis_conn->established.ul_enable;
                if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                    if (!(stream_ctx->client.dl.is_am_open)) {
                        ull_report("[ULL][LE] AIRCIS ESTABLISHED Success, find DL port is streaming, open DL AM!", 0);
                        bt_ull_streaming_t stream;
                        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                        stream.port = 0;
                        bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_DOWNLINK);
                    }
                } else if (bt_ull_le_srv_check_inactive_aircis_feature_on() && !bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK) && !(stream_ctx->client.dl.is_am_open)) {
                    ull_report("[ULL][LE][BOB][DEBUG] Client cis established, downlink port is not open and dsp is not open, need open dsp with priority 0", 0);
                    bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_LOW);
                    bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
                    if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
                    }                
                }

                if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                    if (cis_conn->established.ul_enable) {
                        if (!(stream_ctx->client.ul.is_am_open)) {
                            bt_ull_streaming_t stream;
                            stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                            stream.port = 0;
                            bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
                            //bt_ull_le_am_play(BT_ULL_LE_AM_UL_MODE, stream_ctx->codec_type, true);
                        }
                    } else {
                        bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
                    }
                }

            } else if (BT_ULL_ROLE_CLIENT == ctx->role && BT_ULL_HEADSET_CLIENT == ctx->client_type) {
                /* Enable DSP if dongle is playing */
                bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
                ull_report("[ULL][LE] AIRCIS ESTABLISHED IND, recheck am, %d, %d, %d, %d", 4, \
                    bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK), (stream_ctx->client.dl.is_am_open), \
                    bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK), (stream_ctx->client.ul.is_am_open));
                if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK) && !(stream_ctx->client.dl.is_am_open)) {
                    stream_ctx->client.dl.is_dl_dummy_mode = false;
                    bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_NORMAL);
                    bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
                    if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
                    }
                } else if (bt_ull_le_srv_check_inactive_aircis_feature_on() && !bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK) && !(stream_ctx->client.dl.is_am_open)) {
                    ull_report("[ULL][LE][BOB][DEBUG] Client cis established, downlink port is not open and dsp is not open, need open dsp with priority 0", 0);
                    bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_LOW);
                    bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
                    if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
                    }
                }
                if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK) && !(stream_ctx->client.ul.is_am_open)) {
                    bt_ull_streaming_t stream;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    stream.port = 0;
                    bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
                }

            }
            /*
            if (BT_ULL_ROLE_CLIENT == ctx->role) {
                stream_ctx->audio_quality = cis_conn->established.audio_quility;
                bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_CLIENT, stream_ctx->audio_quality);
            }
            */
        } break;

        case BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND: {
            bt_ull_le_conn_srv_air_cis_evt_ind_t *cis_disconn = (bt_ull_le_conn_srv_air_cis_evt_ind_t *)data;
            bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
            if (BT_HANDLE_INVALID == cis_disconn->destroied.handle) {
                ull_report_error("[ULL][LE] AIRCIS DISCONNECTED IND, invlid handle!", 0);
                break;
            }

            link_info = bt_ull_le_srv_get_link_info(cis_disconn->destroied.handle);
            if ((link_info) && (BT_STATUS_SUCCESS == cis_disconn->status)) {
                link_info->cis_info.dl_enable = false;
                link_info->cis_info.ul_enable = false;
                if (BT_ULL_LE_LINK_STATE_READY <= link_info->curr_state) {
                    link_info->curr_state = BT_ULL_LE_LINK_STATE_READY;
                }
            } else {
                ull_report_error("[ULL][LE] BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND, status is Fail!!!", 0);
                break;
            }
            bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

            if (BT_ULL_ROLE_CLIENT == ctx->role) {
                ull_report("[ULL][LE] AIRCIS DISCONNECTED IND (Client), ul open: %d, dl open: %d", 2, \
                    stream_ctx->client.ul.is_am_open, stream_ctx->client.dl.is_am_open);
                if (stream_ctx->client.ul.is_am_open) {
                #ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
                    bt_ull_wireless_mic_stop_volume_monitor();
                #endif
                }
                bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
                if (stream_ctx->client.dl.is_am_open) {
                    bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true);
                }
                break;
            }
            uint8_t power_state = bt_device_manager_power_get_power_state(BT_DEVICE_TYPE_LE);
            ull_report("[ULL][LE] AIRCIS DISCONNECTED IND (Server), streaming:%d, allow_play: %d, switch_latency: %d, aud_qul_change: %d, cig_created: %d!, reason: %x, power_state: %x", 7,
                bt_ull_le_srv_is_any_streaming_started(ctx->role),
                stream_ctx->allow_play,
                ctx->is_removing_cig_for_switch_latency,
                ctx->is_removing_cig_for_change_aud_quality,
                ctx->is_cig_created,
                cis_disconn->destroied.reason,
                power_state);

            switch (cis_disconn->destroied.reason) {
                case BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_POWER_OFF:
                case BT_HCI_STATUS_CONNECTION_TIMEOUT:
                case BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION:
                {
                    if (!bt_ull_le_srv_is_any_aircis_connected() && \
                        bt_ull_le_srv_is_connected() && \
                        (BT_ULL_MIC_CLIENT == ctx->client_type || BT_ULL_ULD_MIC_CLIENT == ctx->client_type)) {
                        //restart AT
                        ull_report("[ULL][LE] All aircis disconnect but AT is in streaming-1, restart!!", 0);
                        bt_ull_le_at_restart(BT_ULL_MIC_TRANSMITTER);
                    }
                    return;
                    break;
                }
                default:
                    break;
            }
            if (BT_DEVICE_MANAGER_POWER_STATE_STANDBY == power_state || BT_DEVICE_MANAGER_POWER_STATE_STANDBY_PENDING == power_state) {
                return;
            }
            if (bt_ull_le_srv_is_any_streaming_started(ctx->role) \
                && (BT_ULL_ROLE_SERVER == ctx->role) \
                && !stream_ctx->allow_play \
                && !ctx->is_removing_cig_for_switch_latency
                && !ctx->is_removing_cig_for_change_aud_quality
                && !ctx->is_removing_cig_for_change_aud_codec
                ) {
                ull_report("[ULL][LE] AIRCIS DISCONNECTED IND, but have port is open, recreate AirCIS!", 0);
                if (!ctx->is_cig_created) {
                    /*Set Air CIG parameters. */
                    if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_set_air_cig_params(ctx->cis_num)) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                    }
                } else {
                    if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER > ctx->curr_stream_state) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                    }
                    if (!bt_ull_le_srv_is_any_aircis_connected() && \
                        bt_ull_le_srv_is_connected() && \
                        (BT_ULL_MIC_CLIENT == ctx->client_type || BT_ULL_ULD_MIC_CLIENT == ctx->client_type)) {
                        //restart AT
                        ull_report("[ULL][LE] All aircis disconnect but AT is in streaming, restart!!", 0);
                        bt_ull_le_at_restart(BT_ULL_MIC_TRANSMITTER);

                } else {
                        for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
                            if (BT_ULL_LE_LINK_STATE_READY == g_ull_le_link_info[i].curr_state) {
                                /* Create CIS */
                                if (BT_ULL_SPEAKER_CLIENT == bt_ull_le_srv_get_client_type()) {
                                    if (BT_ULL_PLAY_ALLOW == g_ull_le_link_info[i].allow_play) {
                                        if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(g_ull_le_link_info[i].conn_handle)) {
                                            g_ull_le_link_info[i].curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                                        }
                                    }
                                } else {
                                    if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(g_ull_le_link_info[i].conn_handle)) {
                                        g_ull_le_link_info[i].curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                                    }
                                }
                            } else if (BT_ULL_LE_LINK_STATE_STREAMING == g_ull_le_link_info[i].curr_state) {
                                /**<Air CIS has been created done and open audio transmitter successfully. */
                                if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER == ctx->curr_stream_state)
                                    bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                            }
                        }
                    }
                }
                ull_report("[ULL][LE][DEBUG] AIRCIS RECONNECT, notify client restart!", 0);
                bt_ull_le_srv_notify_client_restart_streaming(BT_ULL_LE_RESTART_STREAMING_REASON_AIRCIS_RECONNECT);
            }
        } break;

        case BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ACTIVATED_IND: {
            bt_ull_le_conn_srv_air_cis_evt_ind_t *cis_activiated = (bt_ull_le_conn_srv_air_cis_evt_ind_t *)data;
            if (BT_HANDLE_INVALID == cis_activiated->cis_activiated.handle) {
                ull_report_error("[ULL][LE] AIRCIS STREAMING IND, invlid handle!", 0);
                break;
            }

            link_info = bt_ull_le_srv_get_link_info(cis_activiated->cis_activiated.handle);
            if (!link_info) {
                ull_report_error("[ULL][LE] BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ACTIVATED_IND, link info is null!", 0);
                break;
            }
            if (cis_activiated->cis_activiated.active_state) {
                if (link_info) {
                    if (BT_STATUS_SUCCESS != cis_activiated->status) {
                        ull_report_error("[ULL]AIRCIS STREAMING IND Fail, status is 0x%x!", 1, cis_activiated->status);
                        link_info->curr_state = BT_ULL_LE_LINK_STATE_READY;
                        break;
                    }
                    link_info->curr_state = BT_ULL_LE_LINK_STATE_STREAMING;
                    /**< CIS Prepare done, check if any audio transmitter or audio manager is open. */
                    if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER == ctx->curr_stream_state) {
                        if (bt_ull_le_srv_is_any_streaming_started(ctx->role)) {
                            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                        } else {
                            if (bt_ull_le_srv_check_inactive_aircis_feature_on() && (BT_ULL_ROLE_SERVER == ctx->role)) {
                                /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server] Not to disconnect CIS if no streaming now*/
                                /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client] Deactive ISO if no streaming now.*/
                                bt_ull_le_srv_deactive_streaming();
                            } else {
                                bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_IDLE);
                                bt_ull_le_srv_disconnect_aircis();
                                ull_report_error("[ULL]AIRCIS STREAMING IND, No any streaming, to disconnect AirCIS", 0);
                                break;
                            }
                        }
                    }
                #ifdef AIR_WIRELESS_MIC_RX_ENABLE
                    ull_report("[ULL][LE][Wireless_Mic][Debug], air cis established, update client address", 0);
                    if (BT_ULL_ROLE_SERVER == ctx->role && BT_ULL_MIC_CLIENT == ctx->client_type) {
                        uint8_t idx = bt_ull_conn_srv_get_data_path_id_by_acl_handle(cis_activiated->cis_activiated.handle);
                        bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
                        uint8_t i = 0;
                        bt_ull_le_streaming_start_ind_t start_stream;
                        if (stream_ctx) {
                            for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
                                if (stream_ctx->server.stream[i].is_transmitter_start) {
                                    if (BT_STATUS_SUCCESS == bt_ull_le_srv_get_stream_by_transmitter(i, &start_stream.stream) && ctx->curr_stream_state == BT_ULL_LE_SRV_STREAM_STATE_STREAMING) {
                                        bt_ull_le_at_set_wireless_mic_client_link_info(idx-1, &link_info->addr);//record wireless mic tx link index and tx address.
                                        bt_ull_le_at_update_client_addr(i, idx-1, &link_info->addr);
                                        /* notify app streaming stoped */
                                        bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_START_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t));
                                    }
                                }
                            }
                        }
                    }
               #endif

                    ull_report("[ULL][LE] AIRCIS STREAMING Success, curr_stream_state is 0x%x", 1, ctx->curr_stream_state);
                }

            }else {
                if (BT_STATUS_SUCCESS != cis_activiated->status) {
                    ull_report_error("[ULL]AIRCIS REMOVE STREAMING IND Fail, status is 0x%x!", 1, cis_activiated->status);
                    break;
                }
                link_info->curr_state = BT_ULL_LE_LINK_STATE_SETTING_DATA_PATH;
            }

        } break;

        case BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_PARAMS_REMOVED_IND: {
            bt_ull_le_conn_srv_air_cis_evt_ind_t *cis_remove_ind = (bt_ull_le_conn_srv_air_cis_evt_ind_t *)data;
            bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
            if (BT_STATUS_SUCCESS == cis_remove_ind->status) {
                for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
                    if (BT_ULL_LE_LINK_STATE_READY < g_ull_le_link_info[i].curr_state) {
                        g_ull_le_link_info[i].curr_state = BT_ULL_LE_LINK_STATE_READY;
                    }
                }
             #if 0
                if ((BT_ULL_ROLE_SERVER == ctx->role) && (ctx->is_share_buff_set) && (!bt_ull_le_srv_is_any_streaming_started(ctx->role))) {
                    /*AIRCIG Removed and no Streaming is ongoing, to deinit share buffer. */
                    bt_ull_le_srv_deinit_share_info();
                    ctx->is_share_buff_set = false;
                }
             #endif
                ctx->is_cig_created = false;
                bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_IDLE);
                ull_report("[ULL][LE] AIRCIG Removed Success, curr_stream_state is 0x%x, %d, %d, %d, %d, %d", 6, \
                    ctx->curr_stream_state,
                    bt_ull_le_srv_is_connected(),
                    stream_ctx->allow_play,
                    ctx->is_removing_cig_for_switch_latency,
                    ctx->is_removing_cig_for_change_aud_quality,
                    ctx->is_removing_cig_for_change_aud_codec);

                if (ctx->is_removing_cig_for_change_aud_quality || ctx->is_removing_cig_for_change_aud_codec) {
                    /*set air params table */
                    uint8_t i;
                    for (i = 0;i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
                        if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                            BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                            uint8_t temp_state = g_ull_le_link_info[i].is_cig_table_set;
                            g_ull_le_link_info[i].is_cig_table_set = false;
                            bt_status_t status = bt_ull_le_conn_srv_set_air_cig_params_table(g_ull_le_link_info[i].conn_handle, ctx->cis_num, stream_ctx->codec_type);
                            if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != status) {
                                ull_report_error("[ULL][LE] AIRCIG Removed, set cig table fail!!", 0);
                                g_ull_le_link_info[i].is_cig_table_set = temp_state;
                                break;
                            }
                        }
                    }
                }
                if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                    /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server] Not check current streaming is exist or not when set air CIG parameter .*/
                } else {
                    if (bt_ull_le_srv_is_any_streaming_started(ctx->role) && bt_ull_le_srv_is_connected() && !stream_ctx->allow_play \
                        && !ctx->is_removing_cig_for_switch_latency
                        && !ctx->is_removing_cig_for_change_aud_quality
                        && !ctx->is_removing_cig_for_change_aud_codec) {
                        if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_set_air_cig_params(ctx->cis_num)) {
                            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                        }
                        ull_report("[ULL][LE] AIRCIG Removed Success but have port is open, reset Aircig", 0);
                    }
                }
            }
            if (ctx->is_removing_cig_for_switch_latency) {
                ctx->is_removing_cig_for_switch_latency = false;
                //ctx->is_removing_cig_for_change_aud_quality = false;
                /* set microphone uplink transmitter latency, to stop/start transmitter.*/
                bt_ull_le_srv_notify_client_restart_streaming(BT_ULL_LE_RESTART_STREAMING_REASON_LATENCY_CHANGE);
                bt_ull_le_at_set_latency(stream_ctx->ul_latency, stream_ctx->dl_latency);
                if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                    /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server]:Always set AIR CIG*/
                    if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_set_air_cig_params(ctx->cis_num)) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                    } 
                }
            }

        } break;

        case BT_ULL_LE_CONN_SRV_EVENT_LABEL_CHANGED_IND: {
            bt_ull_le_conn_srv_air_cis_evt_ind_t *label_change = (bt_ull_le_conn_srv_air_cis_evt_ind_t *)data;
            bt_ull_le_srv_aud_quality_info_t *aud_quality_info = bt_ull_le_srv_get_aud_quality_info();
            ull_report("[ULL][LE] LABEL_CHANGED_IND, status is 0x%x, latency: %d, quality: %d, except: %d", 4, \
                label_change->status, label_change->label_changed.latency, label_change->label_changed.audio_quility, aud_quality_info->expect_aud_quality);
            bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
            if (BT_ULL_ROLE_SERVER == ctx->role) {
                stream_ctx->audio_quality = label_change->label_changed.audio_quility;
                stream_ctx->dl_latency = label_change->label_changed.latency;
                bt_ull_le_srv_change_audio_quality(ctx->role, stream_ctx->audio_quality);
                if (BT_STATUS_SUCCESS == label_change->status) {
                    bt_ull_le_srv_notify_client_auido_quality(stream_ctx->audio_quality);
                } else if (BT_HCI_STATUS_PARAMETER_OUT_OF_MANADATORY_RANGE == label_change->status) {
                    ull_report_error("[ULL][LE] LABEL CHANGE FAIL due to channel num!", 0);
                }
                if (aud_quality_info->expect_aud_quality != stream_ctx->audio_quality && BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER != aud_quality_info->expect_aud_quality) {
                    bt_timer_ext_start(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID, aud_quality_info->expect_aud_quality, 20000, bt_ull_le_srv_aud_quality_change_retry_timeout_callback);
                }
            }
/*
            if (BT_STATUS_SUCCESS == lt_change->status) {
                uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
                while (0 != i) {
                    i--;
                    if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                        (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                        uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_latency_t);
                        uint8_t *reply = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                        if (NULL != reply) {
                            reply[0] = BT_ULL_EVENT_LATENCY_SWITCH_COMPLETE;
                            bt_ull_latency_t *latency = (bt_ull_latency_t *)(reply + sizeof(bt_ull_req_event_t));
                            latency->latency = lt_change->latency_changed.latency;
                            bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)reply, len);
                            bt_ull_le_srv_memory_free(reply);
                        }
                    }
                }
            }
            */
        } break;

        case BT_ULL_LE_CONN_SRV_EVENT_UPLINK_ENABLED_IND: {
            bt_ull_le_conn_srv_air_cis_evt_ind_t *ul_enabled = (bt_ull_le_conn_srv_air_cis_evt_ind_t*)data;
            bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
            if (BT_STATUS_SUCCESS == ul_enabled->status) {
                link_info = bt_ull_le_srv_get_link_info(ul_enabled->ul_enabled.handle);
                if (!link_info) {
                    ull_report_error("[ULL][LE] BT_ULL_LE_CONN_SRV_EVENT_UPLINK_ENABLED_IND, link info is null!", 0);
                    break;
                }

                link_info->cis_info.ul_enable = ul_enabled->ul_enabled.ul_enable;
                /*notify app?*/
                /* notify dsp to switch avm buffer*/
                if (BT_ULL_ROLE_SERVER == ctx->role) {
                    if (BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
                        link_info->ul_active = ul_enabled->ul_enabled.ul_enable;
                        bt_ull_le_at_set_ul_channel_locaton(link_info->sink_cfg.audio_location);
                    }
                } else if (BT_ULL_ROLE_CLIENT == ctx->role && BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
                    link_info->ul_active = ul_enabled->ul_enabled.ul_enable;
                    if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                        if (ul_enabled->ul_enabled.ul_enable) {
                            if (!(stream_ctx->client.ul.is_am_open)) {
                                bt_ull_streaming_t stream;
                                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                                stream.port = 0;
                                bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
                                //bt_ull_le_am_play(BT_ULL_LE_AM_UL_MODE, stream_ctx->codec_type, true);
                            }
                        } else {
                            bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
                        }
                    }

                }
            }


        } break;

        case BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_TABLE_SET_IND: {
            bt_ull_le_conn_srv_air_cis_evt_ind_t *set_ind = (bt_ull_le_conn_srv_air_cis_evt_ind_t*)data;
            if (!set_ind) {
                ull_report_error("[ULL][LE] BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_TABLE_SET_IND, set_ind is null!", 0);
                break;
            }
            ull_report("[ULL][LE] Set AIR_CIG_TABLE_SET_IND, status: %d, aud_qua_chg: %d, codec_chg: %d", 3, \
                set_ind->status,
                ctx->is_removing_cig_for_change_aud_quality,
                ctx->is_removing_cig_for_change_aud_codec);

            if (BT_STATUS_SUCCESS == set_ind->status) {
                link_info = bt_ull_le_srv_get_link_info(set_ind->cig_set.handle);
                if (!link_info) {
                    ull_report_error("[ULL][LE] BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_TABLE_SET_IND, link info is null!", 0);
                    break;
                }
                link_info->is_cig_table_set = true;
            }
            bool reset_now = true;
            if (ctx->is_removing_cig_for_change_aud_quality) {
                uint8_t i = (BT_ULL_ROLE_SERVER == ctx->role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;
                while (i > 0) {
                    i--;
                    if (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                        if (!g_ull_le_link_info[i].is_cig_table_set) {
                            reset_now = false;
                            break;
                        }
                    }
                }
                ull_report("[ULL][LE] CIG_TABLE_SET_IND, reset_now: %d", 1, reset_now);
                /*
                if (reset_now) {
                    ctx->is_removing_cig_for_change_aud_quality = false;
                    bt_ull_le_srv_notify_client_restart_streaming(BT_ULL_LE_RESTART_STREAMING_REASON_AUD_QUALITY_CHANGE);
                }
                */
            }
            if(ctx->is_removing_cig_for_change_aud_codec || \
                (ctx->is_removing_cig_for_change_aud_quality && reset_now)) {
               uint8_t transmitter_type;
               int32_t status = BT_STATUS_FAIL;
               ctx->is_share_buff_set = false;
               if (!bt_ull_le_at_is_any_transmitter_start(ctx->role)) {
                    ull_report("[ULL][LE] no streaming is started, clear the flag!!!",0);
                    ctx->is_removing_cig_for_change_aud_codec = false;
                    ctx->is_removing_cig_for_change_aud_quality = false;
                    /* init controller media data share buffer info */
                    status = bt_ull_le_srv_init_share_info(ctx->client_type);
                    if ((BT_STATUS_SUCCESS == status) && (BT_STATUS_SUCCESS == bt_ull_le_srv_set_avm_share_buffer(ctx->role, ctx->client_type, ctx->cis_num))) {
                        ctx->is_share_buff_set = true;
                    } else {
                        ull_report_error("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event set AVM buffer error!!", 0);
                    }
                }
                for (transmitter_type = 0; transmitter_type < BT_ULL_TRANSMITTER_MAX_NUM; transmitter_type++) {
                  if (bt_ull_le_at_is_start(transmitter_type)) {
                     if (BT_STATUS_SUCCESS != bt_ull_le_at_stop(transmitter_type, true)) {
                        ull_report_error("[ULL][LE] SWITCH_AUDIO_CODEC, stop transmitter %d fail", 1, transmitter_type);
                     }
                   } else {
                       /*transmitter is stoped, deinit transmitter directly*/
                       ull_report("[ULL][LE] SWITCH_AUDIO_CODEC, init other transmitter type: %d", 1, transmitter_type);
                       bt_ull_le_at_restart(transmitter_type);
                   }
               }
            }
        } break;
        case BT_ULL_LE_CONN_SRV_EVENT_QOS_REPORT_IND: {
            bt_ull_le_conn_srv_air_cis_evt_ind_t *qos_report = (bt_ull_le_conn_srv_air_cis_evt_ind_t*)data;
            if (qos_report && (BT_STATUS_SUCCESS == qos_report->status)) {
                bt_ull_le_srv_qos_report_hdl(qos_report->status, &qos_report->qos_report);
            }
        } break;

        default:
            break;
    }
}

bt_ull_le_srv_audio_quality_t bt_ull_le_srv_infer_audio_quality(void)
{
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_srv_audio_quality_t aud_qos = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID;
    switch (stream_ctx->audio_quality) {
        case BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION: {
            aud_qos = BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY;
            break;
        }
        case BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY: 
        case BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT: {
            aud_qos = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT;
            break;
        }
        default: {
            break;
        }
    }
    return aud_qos;
}

static void bt_ull_le_srv_aud_quality_change_retry_timeout_callback(uint32_t timer_id, uint32_t data)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    status = bt_ull_le_conn_srv_change_audio_quality(data);
    ull_report("[ULL][LE] bt_ull_le_srv_aud_qos_change_timeout_callback, status: %x, aud_chg: %d", 2, \
        status, data);
}
static void bt_ull_le_srv_aud_qos_change_timeout_callback(uint32_t timer_id, uint32_t data)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_audio_quality_t aud_qos = data;
    bt_ull_le_srv_audio_quality_t resume_qos = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID;
    //status = bt_ull_le_conn_srv_change_audio_quality(aud_qos);
    status = bt_ull_le_srv_change_audio_quality_handle(BT_HANDLE_INVALID, aud_qos);
    bt_timer_ext_t *timer = bt_timer_ext_find(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID);
    if (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION != aud_qos) {
        if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == aud_qos) {
            resume_qos = BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY;
        } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY == aud_qos) {
            resume_qos = BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION;
        }
        if (timer) {
            if (timer->data != resume_qos) {
                timer->data = resume_qos;
            }
            bt_ull_le_srv_reset_timer(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID, 20000);
        } else {
            bt_timer_ext_start(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID, resume_qos, 20000, bt_ull_le_srv_aud_qos_change_timeout_callback);
        }
    }

    ull_report("[ULL][LE] bt_ull_le_srv_aud_qos_change_timeout_callback, status: %x, timert: %d, aud_qos: %d, resume_qos: %d", 4, \
        status, timer, aud_qos, resume_qos);
}

static void bt_ull_le_srv_aud_qos_report_enable_timeout_callback(uint32_t timer_id, uint32_t data)
{

    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    ull_report("[ULL][LE] bt_ull_le_srv_aud_qos_report_enable_timeout_callback, timer_id: %d, data: %x, enable: %d", 3, timer_id, data, ctx->adaptive_bitrate_param.enable);
    if (ctx->adaptive_bitrate_param.enable) {
        bt_ull_le_srv_enable_adaptive_bitrate_mode_internal((bt_handle_t)data, &ctx->adaptive_bitrate_param);
    }
}

static void bt_ull_le_srv_reset_timer(uint32_t timer_id, uint32_t tm_ms)
{
    bt_timer_ext_t *timer = bt_timer_ext_find(timer_id);
    ull_report("[ULL][LE] bt_ull_le_srv_reset_timer, timer_id: %d, timer: %x, tm_ms: %d", 3, timer_id, timer, tm_ms);
    if (timer) {
        uint32_t data = timer->data;
        bt_timer_ext_timeout_callback_t cb = timer->cb;
        bt_timer_ext_stop(timer_id);
        bt_timer_ext_start(timer_id, data, tm_ms, cb);
    }
}

void bt_ull_le_srv_qos_report_hdl(bt_status_t status, bt_ull_le_conn_srv_qos_report_t *report)
{
    if (BT_STATUS_SUCCESS != status || !report) {
        return;
    }
    //bt_timer_ext_status_t t_status ;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_srv_audio_quality_t aud_quality = bt_ull_le_srv_infer_audio_quality();
    bt_timer_ext_t *timer = bt_timer_ext_find(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID);
    bt_status_t ret_status = BT_STATUS_FAIL;
    ull_report("[ULL][LE] bt_ull_le_srv_qos_report_hdl, status: %x, role: %d, timer: %x, stre_aud_qos: %d, aud_qua: %d", 5, \
        status, ctx->role, timer, stream_ctx->audio_quality, aud_quality);
    if (timer) {
        ull_report("[ULL][LE] bt_ull_le_srv_qos_report_hdl, data: %d", 1, timer->data);
    }
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID == aud_quality) {
            if (timer) {
                bt_timer_ext_stop(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID);
            }
            return;
        } else {
            if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == stream_ctx->audio_quality) {
                if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == aud_quality) {
                    if (timer) {
                        if (timer->data != BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY) {
                            timer->data = BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY;
                        }
                        bt_ull_le_srv_reset_timer(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID, 20000);
                    } else {
                        bt_timer_ext_start(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID, BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY, 20000, bt_ull_le_srv_aud_qos_change_timeout_callback);
                    }
                } else {
                    assert(0);
                }
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY == stream_ctx->audio_quality) {
                if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == aud_quality) {
                    if (timer) {
                        if (timer->data != BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY) {
                            timer->data = BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY;
                        }
                        bt_ull_le_srv_reset_timer(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID, 20000);
                    } else {
                        bt_timer_ext_start(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID, BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY, 20000, bt_ull_le_srv_aud_qos_change_timeout_callback);
                    }
                    ret_status = bt_ull_le_srv_change_audio_quality_handle(BT_HANDLE_INVALID, aud_quality);
                    if (!ret_status) {
                        bt_ull_le_adaptive_bitrate_params_t enable = {0};
                        enable.enable = false;
                        bt_ull_le_srv_enable_adaptive_bitrate_mode_internal(report->handle, &enable);
                        bt_timer_ext_start(BT_ULL_LE_AUD_QOS_REPORT_ENABLE_TIMER_ID, (uint32_t)report->handle, 300, bt_ull_le_srv_aud_qos_report_enable_timeout_callback);
                    }
                }
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION == stream_ctx->audio_quality) {
                if (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY == aud_quality) {
                    if (timer) {
                        if (timer->data != BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION) {
                            timer->data = BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION;
                        }
                        bt_ull_le_srv_reset_timer(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID, 20000);
                    } else {
                        bt_timer_ext_start(BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID, BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION, 20000, bt_ull_le_srv_aud_qos_change_timeout_callback);
                    }
                    ret_status = bt_ull_le_srv_change_audio_quality_handle(BT_HANDLE_INVALID, aud_quality);
                    if (!ret_status) {
                        bt_ull_le_adaptive_bitrate_params_t enable;
                        enable.enable = false;
                        bt_ull_le_srv_enable_adaptive_bitrate_mode_internal(report->handle, &enable);
                        bt_timer_ext_start(BT_ULL_LE_AUD_QOS_REPORT_ENABLE_TIMER_ID, (uint32_t)report->handle, 300, bt_ull_le_srv_aud_qos_report_enable_timeout_callback);
                    }
                }
            }
        }
    }else {

    }
    //ull_report("[ULL][LE] bt_ull_le_srv_qos_report_hdl, status: %d", 1, ret_status);
}

static bt_status_t bt_ull_le_srv_handle_host_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    bt_ull_le_srv_link_info_t *link_info = NULL;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();

    switch (msg) {
        case BT_POWER_OFF_CNF: {
            uint8_t i = (BT_ULL_ROLE_SERVER == ctx->role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;

            while (i > 0) {
                i--;
                if (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                    ull_report("[ULL][LE] BT power off, disconnect all audio stream", 0);
                    bt_ull_le_srv_connection_handle(g_ull_le_link_info[i].conn_handle, BT_ULL_DISCONNECTED);
                }
            }
            bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
            bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
            bt_ull_le_srv_reset_link_info();
            bt_ull_le_conn_srv_deinit();
            ctx->is_cig_created = false;
            ctx->is_removing_cig_for_switch_latency = false;
            ctx->is_removing_cig_for_change_aud_quality = false;
            ctx->is_removing_cig_for_change_aud_codec = false;
            bt_ull_le_srv_deinit_share_info();
            ctx->is_share_buff_set = false;
            ctx->critical_data_max_len = 0x00;
            ctx->critical_data_tx_seq = 0x00;
            ctx->critical_data_rx_seq = 0x00;
            stream_ctx->dl_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
            stream_ctx->allow_play = BT_ULL_PLAY_ALLOW;
            if (BT_ULL_ROLE_CLIENT == ctx->role) {
                stream_ctx->streaming_port = BT_ULL_LE_STREAM_PORT_MASK_NONE;
            }

        } break;

        case BT_GAP_LE_CONNECT_IND: {/*LE ACL is established. */
            bt_gap_le_connection_ind_t *conn_ind_p = (bt_gap_le_connection_ind_t *)buff;
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
            bool ull2_link = bt_gap_le_check_remote_features(conn_ind_p->connection_handle, BT_GAP_LE_ULL2_0);
            if (!ull2_link) {
                ull_report_error("[ULL][LE] Not Support ULL V2,  handle: 0x%x", 1, conn_ind_p->connection_handle);
                break;
            }
#endif
            ull_report("[ULL][LE] BT_GAP_LE_CONNECT_IND, status: %x, handle: 0x%x, local addr: %x-%x-%x-%x-%x-%x", 8, \
                status, conn_ind_p->connection_handle, conn_ind_p->local_addr.addr[0], 
                conn_ind_p->local_addr.addr[1],
                conn_ind_p->local_addr.addr[2],
                conn_ind_p->local_addr.addr[3],
                conn_ind_p->local_addr.addr[4],
                conn_ind_p->local_addr.addr[5]);
            if ((BT_STATUS_SUCCESS != status) || (BT_HANDLE_INVALID == conn_ind_p->connection_handle)) {
                ull_report_error("[ULL][LE] BT_GAP_LE_CONNECT_IND, Error, status is 0x%x", 1, status);
            } else {
                if ((BT_ULL_ROLE_SERVER == ctx->role) ||((BT_ULL_ROLE_CLIENT == ctx->role)
#ifndef AIR_LE_AUDIO_DUALMODE_ENABLE
                    && (0 == bt_ull_le_srv_memcmp(&conn_ind_p->local_addr.addr, &(ctx->local_random_addr), sizeof(bt_bd_addr_t)))
#endif
                )) {
                    if (NULL != (link_info = bt_ull_le_srv_get_link_info(BT_HANDLE_INVALID))) {
                        /**< just save the link info, and waiting for find cs info. */
                        link_info->max_packet_size = BT_ULL_LE_SRV_FIX_CHANNEL_MTU; /**<Default Value, must < HCI MTU. */
                        link_info->conn_handle = conn_ind_p->connection_handle;
                        bt_ull_le_srv_memcpy(&(link_info->addr), &(conn_ind_p->peer_addr.addr), sizeof(bt_bd_addr_t));
                        ull_report("[ULL][LE] BT_GAP_LE_CONNECT_IND, handle: 0x%x", 1, conn_ind_p->connection_handle);
                    }
                }
            }
        } break;

        case BT_GAP_LE_DISCONNECT_IND: {
            bt_ull_le_disconnect_info_t dis_conn;
            bt_gap_le_disconnect_ind_t *disc_ind_p = (bt_gap_le_disconnect_ind_t *)buff;
            if ((BT_STATUS_SUCCESS == status) && (BT_HANDLE_INVALID != disc_ind_p->connection_handle)) {
                if (NULL != (link_info = bt_ull_le_srv_get_link_info(disc_ind_p->connection_handle))) {
                    /*switch ul when the link disconnect*/
                    if (BT_ULL_ROLE_SERVER == ctx->role && BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
                        ull_report("[ULL][LE] Need switch ul, conn handle:0x%x, ul:%d", 2, disc_ind_p->connection_handle, link_info->ul_active);
                        if (link_info->ul_active) {
                            uint8_t i = 0;
                            for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
                                if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                                    disc_ind_p->connection_handle != g_ull_le_link_info[i].conn_handle) {
                                    bt_ull_le_srv_activate_up_link_handler(g_ull_le_link_info[i].conn_handle);
                                }
                            }
                        }
                    }
                    bool is_ull_connected = false;
                    if (BT_ULL_LE_LINK_STATE_READY <= link_info->curr_state) {//ULL real connected, not only LE link connected
                        is_ull_connected = true;
                    }
                #ifdef AIR_WIRELESS_MIC_RX_ENABLE
                    ull_report("[ULL][LE][Wireless Mic][Debug], Ull disconnect, clear client link info", 0);
                    bt_ull_le_at_clear_wireless_mic_client_link_info_by_addr(&link_info->addr);
                #endif
                    bt_ull_le_srv_memset(link_info, 0, sizeof(bt_ull_le_srv_link_info_t));
                    link_info->conn_handle = BT_HANDLE_INVALID;
                    bt_ull_le_srv_connection_handle(disc_ind_p->connection_handle, BT_ULL_DISCONNECTED);

                    /* notify app disconencted*/
                    if (is_ull_connected) { 
                        bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
                        dis_conn.conn_handle = disc_ind_p->connection_handle;
                        bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_DISCONNECTED, (void *)&dis_conn, sizeof(bt_ull_le_disconnect_info_t));
                        is_ull_connected = false; 
                        if (BT_ULL_ROLE_CLIENT == ctx->role) {
                            stream_ctx->streaming_port = BT_ULL_LE_STREAM_PORT_MASK_NONE;
                        }
                    } else {
                        ull_report_error("[ULL][LE] BT_GAP_LE_DISCONNECT_IND, Error, ULL is not real connected, No need notify app", 0);
                        bt_ull_le_connected_info_t connection_info;
                        connection_info.status = BT_STATUS_FAIL;
                        connection_info.conn_handle = disc_ind_p->connection_handle;
                        bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CONNECTED, (void *)&connection_info, sizeof(bt_ull_le_connected_info_t));
                    }
                }
                ull_report("[ULL][LE] BT_GAP_LE_DISCONNECT_IND, conn handle:0x%x, role:0x%x", 2, disc_ind_p->connection_handle, ctx->role);
            } else {
                ull_report_error("[ULL][LE] BT_GAP_LE_DISCONNECT_IND, Error, status is 0x%x, handle is 0x%x", 2, status, disc_ind_p->connection_handle);
            }
        } break;

    default:
        break;
    }
    return BT_STATUS_SUCCESS;
}

static void bt_ull_le_srv_stop_audio_transmitter_callback(bt_ull_transmitter_t transmitter_type, bt_status_t status)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_status_t share_status = BT_STATUS_FAIL;
    uint8_t i;

    ull_report("[ULL][LE] bt_ull_le_srv_stop_audio_transmitter_callback, trans_type is %d, status is 0x%x, curr_stream_state is %d", 3, transmitter_type, status, ctx->curr_stream_state);
    if ((BT_STATUS_SUCCESS == status) && (BT_ULL_ROLE_SERVER == ctx->role)) {
    //BTA-37783,when stop normal mode and enter special mode, not set is_transmitter_start flag, please refer to JIRA.
        bt_ull_le_srv_set_transmitter_is_start(transmitter_type, false);
        //bt_ull_le_at_deinit(transmitter_type);
        if (!bt_ull_le_srv_is_any_streaming_started(ctx->role) && ctx->curr_stream_state >= BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER) {/*all streaming are disabled.*/
            /* disable controller and enable sniff mode after all streaming stop */
            if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
            } else {
                bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_IDLE);
            }
            #if 0
            if (ctx->is_share_buff_set && (!ctx->is_cig_created)) {
                /*AIRCIG Removed and no Streaming is ongoing, to deinit share buffer. */
                bt_ull_le_srv_deinit_share_info();
                ctx->is_share_buff_set = false;
            }
            #endif
            if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                 /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server]: Relace deavtiving CIS to disconnect CIS, always keep CIS is alive*/
                ull_report("[ULL][LE][BOB][DEBUG] bt_ull_le_srv_stop_audio_transmitter_callback, not disconnect aircis, just deactive aircis!!", 0);
                bt_ull_le_srv_deactive_streaming();
            } else {
                bt_ull_le_srv_disconnect_aircis();
            }
            /* TBD. enable link sniff policy */
        }
        if (!bt_ull_le_srv_is_any_streaming_started(ctx->role)) {
            ull_report("[ULL][LE] bt_ull_le_srv_stop_audio_transmitter_callback, audio_codec: %d,audio_quality: %d", 2, ctx->is_removing_cig_for_change_aud_codec, ctx->is_removing_cig_for_change_aud_quality);
            if (ctx->is_removing_cig_for_change_aud_codec || ctx->is_removing_cig_for_change_aud_quality) {
                ctx->is_removing_cig_for_change_aud_codec = false;
                ctx->is_removing_cig_for_change_aud_quality = false;

               if(!ctx->is_share_buff_set) {
                   bt_ull_le_srv_deinit_share_info();
                   /* init controller media data share buffer info */
                   share_status = bt_ull_le_srv_init_share_info(ctx->client_type);
                   if ((BT_STATUS_SUCCESS == share_status) && (BT_STATUS_SUCCESS == bt_ull_le_srv_set_avm_share_buffer(ctx->role, ctx->client_type, ctx->cis_num))) {
                        ctx->is_share_buff_set = true;
                   } else {
                       ull_report_error("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event set AVM buffer error!!", 0);
                   }
               }
               for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
                    bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(i);
                    if (stream_ctx->streaming_port & stream_port) {
                       ull_report("[ULL][LE] change_aud_codec,restart transmitter", 0);
                       if(BT_ULL_PLAY_ALLOW == stream_ctx->allow_play && bt_ull_le_srv_is_connected()) {
                           bt_ull_le_at_reinit(i);
                           if (BT_STATUS_SUCCESS != bt_ull_le_at_start(i, true)) {
                               ull_report_error("[ULL][LE] SWITCH_AUDIO_CODEC, start transmitter %d fail", 1, i);
                            } else {
                               bt_ull_le_srv_sync_streaming_status(stream_port, BT_ULL_EVENT_STREAMING_START_IND);
                            }
                        }
                    }
               }
            }
        }
        bt_ull_le_streaming_stop_ind_t stop_stream;
        if (BT_STATUS_SUCCESS == bt_ull_le_srv_get_stream_by_transmitter(transmitter_type, &stop_stream.stream)) {
            /* notify app streaming stoped */
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_STOP_IND, (void *)&stop_stream, sizeof(bt_ull_le_streaming_stop_ind_t));
        }
    }
}

static void bt_ull_le_srv_start_audio_transmitter_callback(bt_ull_transmitter_t transmitter_type, bt_status_t status)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE] bt_ull_le_srv_start_audio_transmitter_callback, trans_type is %d, status is 0x%x, switch_latency: %d, aud_change: %d, cig_created: %d, allow_play: %d, curr_stream_state: %d",
        7, transmitter_type, status, ctx->is_removing_cig_for_switch_latency, ctx->is_removing_cig_for_change_aud_quality, ctx->is_cig_created, stream_ctx->allow_play, ctx->curr_stream_state);
    if ((BT_STATUS_SUCCESS == status) && (BT_ULL_ROLE_SERVER == ctx->role)) {
        bt_ull_le_srv_set_transmitter_is_start(transmitter_type, true);
        uint8_t i;
        if (BT_ULL_MIC_TRANSMITTER == transmitter_type && BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
            for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
                if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                    BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state && \
                    g_ull_le_link_info[i].ul_active) {
                    bt_ull_le_at_set_ul_channel_locaton(g_ull_le_link_info[i].sink_cfg.audio_location);
                    break;
                }
            }
        }
        if (BT_ULL_PLAY_DISALLOW == stream_ctx->allow_play) {
            if (BT_STATUS_SUCCESS != bt_ull_le_at_stop(transmitter_type, true)) {
              ull_report_error("[ULL] bt_ull_le_srv_start_audio_transmitter_callback, bt_ull_le_at_stop error", 0);
            }
        }
        if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
            /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server]: Active ISO in audio transmitter callback*/
            ull_report("[ULL][LE][BOB][DEBUG] bt_ull_le_srv_start_audio_transmitter_callback, Active ISO in audio transmitter callback!!", 0);
            if (!ctx->is_removing_cig_for_switch_latency || !ctx->is_removing_cig_for_change_aud_quality || !ctx->is_removing_cig_for_change_aud_codec) {
                if (!ctx->is_cig_created) {
                    /*Set Air CIG parameters. */
                    if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_set_air_cig_params(ctx->cis_num)) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                    } else {
                        ull_report_error("[ULL][LE] bt_ull_le_srv_start_audio_transmitter_callback, set cig error", 0);
                    }
                } else {
                    if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER > ctx->curr_stream_state) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                    }
                    for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
                        if (BT_ULL_LE_LINK_STATE_READY == g_ull_le_link_info[i].curr_state) {
                            /* Create CIS */
                            if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(g_ull_le_link_info[i].conn_handle)) {
                                g_ull_le_link_info[i].curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                            }
                        } else if (BT_ULL_LE_LINK_STATE_SETTING_DATA_PATH == g_ull_le_link_info[i].curr_state) {
                            /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server]: Active ISO in audio transmitter callback*/
                            bt_ull_le_srv_active_streaming(); /*set iso data path*/
                        } else if (BT_ULL_LE_LINK_STATE_STREAMING == g_ull_le_link_info[i].curr_state) {
                            /**<Air CIS has been created done and open audio transmitter successfully. */
                            if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER == ctx->curr_stream_state) {
                                bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                            }
                        }
                    }
                }
            }
        } else {
            if (!ctx->is_removing_cig_for_switch_latency || !ctx->is_removing_cig_for_change_aud_quality || !ctx->is_removing_cig_for_change_aud_codec) {
                if (!ctx->is_cig_created) {
                    /*Set Air CIG parameters. */
                    uint8_t temp_state = ctx->curr_stream_state;
                    bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                    if (BT_STATUS_SUCCESS != bt_ull_le_conn_srv_set_air_cig_params(ctx->cis_num)) {
                        ull_report_error("[ULL][LE] bt_ull_le_srv_start_audio_transmitter_callback, set cig error", 0);
                        ctx->curr_stream_state = temp_state;
                    }
                } else {
                    if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER > ctx->curr_stream_state) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                    }
                    for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
                        if (BT_ULL_LE_LINK_STATE_READY == g_ull_le_link_info[i].curr_state) {
                            /* Create CIS */
                            if (BT_ULL_SPEAKER_CLIENT == bt_ull_le_srv_get_client_type()) {
                                if (BT_ULL_PLAY_ALLOW == g_ull_le_link_info[i].allow_play) {
                            if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(g_ull_le_link_info[i].conn_handle)) {
                                g_ull_le_link_info[i].curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                            }
                                }
                            } else {
                                if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(g_ull_le_link_info[i].conn_handle)) {
                                    g_ull_le_link_info[i].curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                                }
                            }
                        } else if (BT_ULL_LE_LINK_STATE_STREAMING == g_ull_le_link_info[i].curr_state) {
                            /**<Air CIS has been created done and open audio transmitter successfully. */
                            if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER == ctx->curr_stream_state)
                                bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                             if (BT_ULL_MIC_TRANSMITTER != transmitter_type) {
                             /** always need send unmute vendor cmd to controller, controller will send play_en to DSP. */
                                 if (BT_STATUS_SUCCESS != bt_ull_le_conn_srv_unmute_air_cis(g_ull_le_link_info[i].conn_handle)) {
                                    ull_report_error("[ULL][LE] bt_ull_le_srv_play_am_callback unmute air cis fail", 0);
                               }
                            }
                        }
                    }
                }
            }
        }
        bt_ull_le_streaming_start_ind_t start_stream;
        if (BT_STATUS_SUCCESS == bt_ull_le_srv_get_stream_by_transmitter(transmitter_type, &start_stream.stream) && ctx->curr_stream_state == BT_ULL_LE_SRV_STREAM_STATE_STREAMING) {
            /* notify app streaming stoped */
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_START_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t));
        }
        ull_report("[ULL][LE] bt_ull_le_srv_start_audio_transmitter_callback curr_stream_state: %d", 1, ctx->curr_stream_state);
    } else {
        bt_ull_le_srv_set_transmitter_is_start(transmitter_type, false);
    }
}

static bt_status_t bt_ull_le_srv_play_am(bt_ull_streaming_t *streaming, bt_ull_le_stream_mode_t stream_mode)
{
    bt_status_t status = BT_STATUS_SUCCESS;

    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
    bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_srv_client_stream_t *stream_info = (bt_ull_le_srv_client_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_CLIENT, trans_type);

    if (ctx->is_streaming_locked) {
        stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_MIC; 
        ull_report_error("[ULL][LE] bt_ull_le_srv_play_am, Fail, streaming is locked!", 0);
        return BT_STATUS_FAIL;
    }

    if (BT_ULL_LE_STREAM_MODE_DOWNLINK == stream_mode) {
        if (!stream_info) {
            return BT_STATUS_FAIL;
        }

        if (stream_info->is_dl_dummy_mode && stream_info->is_am_open) {
            /*BTA-50829: if dsp open but current is dummy mode, should notify APP start play*/
            bt_ull_le_streaming_start_ind_t start_stream;
            start_stream.stream_mode = BT_ULL_LE_STREAM_MODE_DOWNLINK;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_START_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t));            
        }
        stream_info->is_dl_dummy_mode = false;

        if (!(stream_info->is_am_open)) {
            status = bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
            if ((BT_STATUS_SUCCESS == status) && (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state)) {
                bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
            }
        } else {
            uint8_t i;
            for (i = 0; i < 1; i++) {
                if (BT_ULL_LE_LINK_STATE_STREAMING == g_ull_le_link_info[i].curr_state) {
                    if (BT_ULL_LE_SRV_STREAM_STATE_STREAMING != ctx->curr_stream_state)
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                }
            }
        }
    } else if (BT_ULL_LE_STREAM_MODE_UPLINK == stream_mode) {
        if (bt_ull_le_srv_is_connected()) {
            bt_ull_le_srv_link_info_t *link_info = &g_ull_le_link_info[0];
            /* Normal case, enable mic if ull is playing */
            ull_report("[ULL][LE] is_am_open: %d", 1, stream_ctx->client.ul.is_am_open);
            if (((BT_ULL_EARBUDS_CLIENT == ctx->client_type && link_info->ul_active) || BT_ULL_EARBUDS_CLIENT != ctx->client_type) && \
                (!(stream_ctx->client.ul.is_am_open)||(bt_ull_le_am_is_stop_ongoing(BT_ULL_LE_AM_UL_MODE)) || (false == bt_ull_le_am_check_request_start_flag(BT_ULL_LE_AM_UL_MODE)))) {
                /* Ensure DL has beed enabled before UL enable.*/
                /* 1. only Uplink come, enter DL dummy mode*/
#ifndef AIR_WIRELESS_MIC_ENABLE
                bt_ull_le_am_update_dl_priority(true);//need to raise proiryty for Uplink
                if (!(stream_ctx->client.dl.is_am_open) || bt_ull_le_am_is_stop_ongoing(BT_ULL_LE_AM_DL_MODE)) {
                    stream_ctx->client.dl.is_dl_dummy_mode = bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK) ? false : true;
                    if (BT_STATUS_SUCCESS == bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true)) {
                        ull_report("[ULL][LE] Raise Prority and Enable Downlink before Start Uplink", 0);
                    }
                } else
#endif
                {/* 2. DL has beed enabled before UL enable*/
                    bt_ull_le_am_play(BT_ULL_LE_AM_UL_MODE, stream_ctx->codec_type, true);
                    if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
                    }
                }
            } else {
                /* ULL earbuds only have 1 earbud active uplink, but it stiil need to update the downlink's audio priority to support Uplink can match the EMP media priority rule */
                if ((BT_ULL_EARBUDS_CLIENT == ctx->client_type) && !(link_info->ul_active)) {
                    bt_ull_le_am_update_dl_priority(true);//need to raise proiryty for Uplink

                    bt_ull_le_am_sync_sidetone_status(true);
                    if (!(stream_ctx->client.dl.is_am_open)) {
                        stream_ctx->client.dl.is_dl_dummy_mode = bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK) ? false : true;

                        if (BT_STATUS_SUCCESS == bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true)) {
                            ull_report("[ULL][LE] Raise Prority and Enable Downlink before Start Uplink", 0);
                        }
                        return status;
                    } else {/* 2. DL has beed enabled before UL enable*/
                        //stream_ctx->client.ul.is_am_open = true;
                    }
                }
                if (BT_ULL_LE_LINK_STATE_STREAMING == link_info->curr_state) {
                    if (BT_ULL_LE_SRV_STREAM_STATE_STREAMING != ctx->curr_stream_state)
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                }

                // notify app UL streaming started for sync other earbuds that open UL realy;
                bt_ull_le_streaming_start_ind_t start_stream;
                start_stream.stream_mode = BT_ULL_LE_STREAM_MODE_UPLINK;
                bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_START_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t));
            }
        }
    }
    ull_report("[ULL][LE] Client recieved enable stream_port:%d, status:0x%x, curr_stream_state:0x%x", 3, stream_port, status, ctx->curr_stream_state);
    return status;
}

static bt_status_t bt_ull_le_srv_stop_am(bt_ull_streaming_t *streaming)
{
    bt_status_t status = BT_STATUS_SUCCESS;

    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
    bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    stream_ctx->streaming_port &= ~stream_port;
    if (stream_ctx->locking_port & stream_port) {
        stream_ctx->locking_port &= ~stream_port;
    }
    //BTA-37571: remove the conditional jugement, is_am_on. for detail info, please refer to JIRA
    if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
        //BTA-38465: add condition judement, bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK),  for detail info, please refer to JIRA
        if (stream_ctx->client.ul.is_am_open || bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
        #ifndef AIR_WIRELESS_MIC_ENABLE
            stream_ctx->client.dl.is_dl_dummy_mode = true;
        #endif
        } else {
            bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true);
        }
    }
    ull_report("[ULL][LE] Client recieved disable stream_port:%d, status:0x%x, curr_stream_state:0x%x", 3, stream_port, status, ctx->curr_stream_state);
    return status;
}

static void bt_ull_le_srv_play_am_callback(bt_ull_le_am_mode_t mode, bt_status_t status)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE] bt_ull_le_srv_play_am_callback, mode is %d, status is 0x%x", 2, mode, status);

    if ((BT_STATUS_SUCCESS == status) && (BT_ULL_ROLE_CLIENT == ctx->role)) {
        uint8_t i;
        bt_ull_le_streaming_start_ind_t start_stream;
        bt_ull_le_srv_client_stream_t *stream_info = NULL;
        if (BT_ULL_LE_AM_DL_MODE == mode) {
            start_stream.stream_mode = BT_ULL_LE_STREAM_MODE_DOWNLINK;
            stream_info = (bt_ull_le_srv_client_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_CLIENT, BT_ULL_GAMING_TRANSMITTER);
            if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: Not active air cis when play am callback*/
                ull_report("[ULL][LE][BOB][DEBUG] bt_ull_le_srv_play_am_callback, not active aircis!!", 0);
            } else {
                for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
                    if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                        BT_ULL_LE_LINK_STATE_SETTING_DATA_PATH == g_ull_le_link_info[i].curr_state) {
                        bt_ull_le_conn_srv_activiate_air_cis(g_ull_le_link_info[i].conn_handle);
                    }
                }
            }

        #ifndef AIR_WIRELESS_MIC_ENABLE
            /**< 1. raise DL pseudo device priority; 2) Take Mic resource for ULL UL;*/
            if ((!(stream_ctx->client.ul.is_am_open)) && (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK))) {
                bt_ull_le_am_update_dl_priority(true);//need to raise proiryty for Uplink
                if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                    stream_ctx->client.dl.is_dl_dummy_mode = true;
                }
                
                bt_ull_le_srv_link_info_t *link_info = &g_ull_le_link_info[0];
                if ((BT_ULL_EARBUDS_CLIENT == ctx->client_type) && !(link_info->ul_active)) {
                    ull_report("[ULL][LE] Downlink Enable, incative UL earbuds no need ti start Uplink", 0);
                } else {
                    if (BT_STATUS_SUCCESS == bt_ull_le_am_play(BT_ULL_LE_AM_UL_MODE, stream_ctx->codec_type, true)) {
                        ull_report("[ULL][LE] Downlink Enable, then start Uplink", 0);
                    }
                }
            }
            ull_report("[ULL][LE]Downlink Enable, DL is Dummy mode: %d", 1, stream_ctx->client.dl.is_dl_dummy_mode);
        #endif
        } else if (BT_ULL_LE_AM_UL_MODE == mode) {
            start_stream.stream_mode = BT_ULL_LE_STREAM_MODE_UPLINK;
            stream_info = (bt_ull_le_srv_client_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_CLIENT, BT_ULL_MIC_TRANSMITTER);
        }
        if (NULL == stream_info) {
            ull_report_error("[ULL][LE] bt_ull_le_srv_play_am_callback can not get streaming info", 0);
            return;
        }
        stream_info->is_am_open = true;
        /* notify app streaming started */
        if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
            /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: when streaming port is real streaming, Notify up layer start streaming status when dsp playback*/
            if ((BT_ULL_LE_AM_UL_MODE == mode) && (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK))) {
                bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_START_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t));            
            } else if ((BT_ULL_LE_AM_DL_MODE == mode) && (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK))) {
                bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_START_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t));              
            }            
        } else {
            if ((BT_ULL_LE_AM_UL_MODE == mode) || ((BT_ULL_LE_AM_DL_MODE == mode) && (!(stream_ctx->client.dl.is_dl_dummy_mode)))) {
                bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_START_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t));
            }
        }

        if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER > ctx->curr_stream_state) {
            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
        }
        for (i = 0; i < BT_ULL_LE_CLIENT_LINK_MAX_NUM; i++) {
            if (BT_ULL_LE_LINK_STATE_STREAMING == g_ull_le_link_info[i].curr_state) {
                if (BT_ULL_LE_SRV_STREAM_STATE_STREAMING != ctx->curr_stream_state) {
                    bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                }
                 /** always need send unmute vendor cmd to controller, controller will send play_en to DSP. */
                if (BT_STATUS_SUCCESS != bt_ull_le_conn_srv_unmute_air_cis(g_ull_le_link_info[i].conn_handle)) {
                    ull_report_error("[ULL][LE] bt_ull_le_srv_play_am_callback unmute air cis fail", 0);
                }
            }
            else if (bt_ull_le_srv_check_inactive_aircis_feature_on() && \
                (BT_ULL_LE_LINK_STATE_SETTING_DATA_PATH == g_ull_le_link_info[i].curr_state) && \
                (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK) || bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK))) {
                /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: Active ISO in dsp playback if streaming exist*/
                ull_report("[ULL][LE][BOB][DEBUG] bt_ull_le_srv_play_am_callback, Active ISO in dsp playback if streaming exist!!", 0);
                if (BT_STATUS_SUCCESS == bt_ull_le_srv_active_streaming()) {
                    ull_report("[ULL][LE] bt_ull_le_srv_play_am_callback active air cis success", 0);
                }
            }
        }
    }
    ull_report("[ULL][LE] bt_ull_le_srv_play_am_callback, mode is %d, status is 0x%x, curr_stream_state is 0x%x", 3, mode, status, ctx->curr_stream_state);
}

static void bt_ull_le_srv_stop_am_callback(bt_ull_le_am_mode_t mode, bt_status_t status)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE] bt_ull_le_srv_stop_am_callback, mode is %d, status is 0x%x", 2, mode, status);

    if ((BT_STATUS_SUCCESS == status) && (BT_ULL_ROLE_CLIENT == ctx->role)) {
        bt_ull_le_streaming_stop_ind_t stop_stream;
        bt_ull_le_srv_client_stream_t *stream_info = NULL;
        if (BT_ULL_LE_AM_DL_MODE == mode) {
            //stream_ctx->client.dl.is_dl_dummy_mode = false;
            stop_stream.stream_mode = BT_ULL_LE_STREAM_MODE_DOWNLINK;
            stream_info = (bt_ull_le_srv_client_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_CLIENT, BT_ULL_GAMING_TRANSMITTER);
            if (!stream_info) {
                return;
            }
            stream_info->is_am_open = false;
    #ifndef AIR_WIRELESS_MIC_ENABLE
            if (stream_ctx->client.ul.is_am_open && !(ctx->restart_streaming_mask & (1 << BT_ULL_LE_STREAM_MODE_DOWNLINK))) {
                bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
                stream_ctx->client.dl.is_dl_dummy_mode = false;
                ull_report("[ULL][LE] Downlink Disabled but Uplink still exist, need to disable Uplink", 0);
            }
    #endif
            if (ctx->restart_streaming_mask & (1 << BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                ctx->restart_streaming_mask &= (~(1 << BT_ULL_LE_STREAM_MODE_DOWNLINK));
                ull_report("[ULL][LE] Need restart DL streming, %d", 1, bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK));
                if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                    bt_ull_streaming_t stream;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    stream.port = 0;
                    bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_DOWNLINK);
                }
            }
            if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                if (bt_ull_le_srv_is_connected()) {
                    /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: Always play am*/
                    ull_report("[ULL][LE][DEBUG] bt_ull_le_srv_stop_am_callback, play am again", 0);
                    bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
                }
            }
        } else if (BT_ULL_LE_AM_UL_MODE == mode) {
            stop_stream.stream_mode = BT_ULL_LE_STREAM_MODE_UPLINK;
            stream_info = (bt_ull_le_srv_client_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_CLIENT, BT_ULL_MIC_TRANSMITTER);
            if (!stream_info) {
                return;
            }
            stream_info->is_am_open = false;
    #ifndef AIR_WIRELESS_MIC_ENABLE
            /**< 1. check if the DL is needed to enabled(not dummy mode); 2. if 1 is yes, lower DL pseudo device priority; */
            if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
                    if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                        /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: Reduce audio prority to 0 when UL and DL is not streaming*/
                        ull_report("[ULL][LE] Uplink Disable, DL port is not open, not stop DL but set priority to 0", 0);
                        if (BT_STATUS_SUCCESS == bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true)) {
                            stream_ctx->client.dl.is_dl_dummy_mode = false;
                        }
                        bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_LOW);                        
                    } else {
                        if (BT_STATUS_SUCCESS == bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true)) {
                            stream_ctx->client.dl.is_dl_dummy_mode = false;
                            ull_report("[ULL][LE] Uplink Disable, then stop Downlink Dummy mode", 0);
                        }
                        bt_ull_le_am_update_dl_priority(false);
                    }
                }
                if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
                    stream_ctx->client.dl.is_dl_dummy_mode = true;
                }
            } else {/*dummy mode, need to lower proiryty because of Uplink had been disabled*/
                bt_ull_le_srv_link_info_t *link_info = &g_ull_le_link_info[0];
                if ((BT_ULL_EARBUDS_CLIENT == ctx->client_type) && (!(link_info->ul_active))
                    && (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK))) {
                    /*Uplink switch active to the other earbuds, do not lower the proiryty.*/
                    /*Do nothing*/
                } else {
                    stream_ctx->client.dl.is_dl_dummy_mode = false;
                    bt_ull_le_am_update_dl_priority(false);
                }
            }
            ull_report("[ULL][LE] Uplink Disable, DL is Dummy mode: %d", 1, stream_ctx->client.dl.is_dl_dummy_mode);
    #endif
            if (ctx->restart_streaming_mask & (1 << BT_ULL_LE_STREAM_MODE_UPLINK)) {
                ctx->restart_streaming_mask &= (~(1 << BT_ULL_LE_STREAM_MODE_UPLINK));
                ull_report("[ULL][LE] Need restart UL streming, %d", 1, bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK));
                if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
                    bt_ull_streaming_t stream;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    stream.port = 0;
                    bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
                }
            }
        }
        if (NULL == stream_info) {
            ull_report_error("[ULL][LE] bt_ull_le_srv_stop_am_callback can not get streaming info", 0);
            return;
        }
        /* notify app streaming stoped */
        if ((BT_ULL_LE_AM_UL_MODE == mode) || ((BT_ULL_LE_AM_DL_MODE == mode) && (!stream_ctx->client.dl.is_dl_dummy_mode))) {
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_STOP_IND, (void *)&stop_stream, sizeof(bt_ull_le_streaming_stop_ind_t));
        }

        if (!bt_ull_le_srv_is_any_streaming_started(ctx->role)) {/*all streaming are disabled.*/
            /* disable controller and enable sniff mode after all streaming stop */
            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_IDLE);
            /* TBD. enable link sniff policy */
        }
        ull_report("[ULL][LE] bt_ull_le_srv_stop_am_callback, curr_stream_state %d", 1, ctx->curr_stream_state);
    }
}

void bt_ull_le_srv_event_callback(bt_ull_event_t event, void *param, uint32_t len)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    //ull_report("[ULL][LE][D] bt_ull_le_srv_event_callback event:0x%x, param len:0x%x", 2, event, len);
    if (ctx && ctx->callback) {
        ctx->callback(event, param, len);
    }
}

static void bt_ull_le_srv_handle_pairing_event(uint16_t handle, bt_status_t status, bt_ull_client_t client_type)
{
    bt_key_t *sirk = NULL;
    bt_ull_le_connected_info_t conn_info;
    bt_ull_le_srv_link_info_t *link_info = bt_ull_le_srv_get_link_info(handle);

    if (link_info && (BT_HANDLE_INVALID != handle)) {
        sirk = bt_ull_le_srv_get_sirk();
        if (NULL != sirk) {
            bt_ull_le_srv_memcpy(&(conn_info.sirk), sirk, sizeof(bt_key_t));
        }
        conn_info.set_size = bt_ull_le_srv_get_coordinated_set_size();
        conn_info.client_type = client_type;
        conn_info.status = status;
        conn_info.conn_handle = handle;
        bt_ull_le_srv_memcpy((uint8_t *)&conn_info.group_device_addr, (uint8_t *)&link_info->group_device_addr, sizeof(bt_addr_t) * (BT_ULL_LE_MAX_LINK_NUM - 1));
        bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CONNECTED, &conn_info, sizeof(bt_ull_le_connected_info_t));
        ull_report("[ULL][LE] Notify App ULL Connect event, status is 0x%x, handle is 0x%x, client is %d, CS size is %d ", 4, status, handle, client_type, conn_info.set_size);
    } else {
        ull_report_error("[ULL][LE] bt_ull_le_srv_handle_pairing_event, invalid handle !", 0);
    }
}

static bt_status_t bt_ull_le_srv_handle_find_cs(bt_ull_le_find_cs_info_t *cs_data)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_srv_link_info_t *link_info = NULL;
    bt_ull_le_srv_context_t *ull_context = bt_ull_le_srv_get_context();
    if (!cs_data) {
        ull_report_error("[ULL][LE]cs_data is NULL!!", 0);
        return BT_STATUS_FAIL;
    }
    if (BT_HANDLE_INVALID == cs_data->conn_handle) {
        ull_report_error("[ULL][LE]bt_ull_le_srv_handle_find_cs, invlaid params, handle is 0x%x", 1, cs_data->conn_handle);
        return BT_STATUS_FAIL;
    }

    link_info = bt_ull_le_srv_get_link_info(cs_data->conn_handle);
    if ((link_info) && (BT_ULL_ROLE_SERVER == ull_context->role) &&
        (BT_ULL_LE_LINK_STATE_IDLE == link_info->curr_state)) {
        /**< REQ Client the SIRK and CS. */
        uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_cs_info_t);
        uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
        if (NULL != request) {
            request[0] = BT_ULL_EVENT_COORDINATED_SET_INFO;
            bt_ull_le_srv_cs_info_t *cs_req =(bt_ull_le_srv_cs_info_t *)(request + sizeof(bt_ull_req_event_t));
            cs_req->op = BT_ULL_LE_SRV_CS_REQ;

            //TBD: set a timer as duration
            if (BT_STATUS_SUCCESS == bt_ull_le_srv_send_data(cs_data->conn_handle, (uint8_t*)request, len)) {
                link_info->curr_state = BT_ULL_LE_LINK_FINDING_CS_INFO;
                status = BT_STATUS_SUCCESS;
            }
            bt_ull_le_srv_memory_free(request);
        }
    }

    ull_report("[ULL][LE] CS Request, conn handle is 0x%4x", 1, cs_data->conn_handle);
    return status;
}

static bt_status_t bt_ull_le_srv_handle_sync_codec(uint16_t handle)
{
    bt_status_t status = BT_STATUS_FAIL;

    bt_ull_le_srv_context_t *ull_context = bt_ull_le_srv_get_context();
    bt_ull_le_srv_link_info_t *link_info = bt_ull_le_srv_get_link_info(handle);
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if ((BT_HANDLE_INVALID == handle) || (!link_info)) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_handle_sync_codec, invlaid params, handle is 0x%x", 1, handle);
        return BT_STATUS_FAIL;
    }
    if (BT_ULL_LE_LINK_STATE_SYNC_CODEC_INFO != link_info->curr_state) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_handle_sync_codec, Error, curr state is %d", 1, link_info->curr_state);
        return BT_STATUS_FAIL;
    }

    bt_ull_le_srv_stream_context_t *stream_info = NULL;
    uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_stream_context_t);
    uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
    if (NULL != request) {
        request[0] = BT_ULL_EVENT_LE_STREAM_CODEC_INFO;
        stream_info = (bt_ull_le_srv_stream_context_t *)(request + sizeof(bt_ull_req_event_t));
    }
    if (NULL == stream_info) {
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE] codec sync: len: %d, request: %x", 2, len, request);
    /* exchange codec info */
    if (BT_ULL_ROLE_SERVER == ull_context->role) {
        /* set server speaker info */
        stream_info->codec_type = stream_ctx->codec_type;
        stream_info->streaming_port = stream_ctx->streaming_port;

        stream_info->client.dl.is_mute = stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].is_mute;
        stream_info->client.dl.usb_sample_rate = stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].usb_sample_rate;
        stream_info->is_silence = stream_ctx->is_silence;
        //bt_ull_le_srv_memcpy(&(stream_info->client.dl.volume), &(stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].volume), sizeof(bt_ull_original_duel_volume_t));
        bt_ull_le_srv_memcpy(&(stream_info->client.dl.volume), &(stream_ctx->client.dl.volume), sizeof(bt_ull_original_duel_volume_t));
        bt_ull_le_srv_memcpy(&(stream_info->server_chat_volume), &(stream_ctx->server.stream[BT_ULL_CHAT_TRANSMITTER].volume), sizeof(bt_ull_original_duel_volume_t));
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        bt_ull_le_srv_memcpy(&(stream_info->server_linein_volume), &(stream_ctx->server.stream[BT_ULL_LINE_IN_TRANSMITTER].volume), sizeof(bt_ull_original_duel_volume_t));
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
        bt_ull_le_srv_memcpy(&(stream_info->server_linei2s_volume), &(stream_ctx->server.stream[BT_ULL_I2S_IN_TRANSMITTER].volume), sizeof(bt_ull_original_duel_volume_t));
#endif

        /* set server microphone info */
        stream_info->client.ul.is_mute = stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].is_mute;
        stream_info->client.ul.usb_sample_rate = stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].usb_sample_rate;
        bt_ull_le_srv_memcpy(&(stream_info->client.ul.volume), &(stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].volume), sizeof(bt_ull_original_duel_volume_t));

        ull_report("[ULL][LE] server streaming port: 0x%x, is_mute:0x%x, microphone is_mute:0x%x, Dl_codec_samplerate: %d, is_silence: %d", 5,
            stream_info->streaming_port, stream_info->client.dl.is_mute, stream_info->client.ul.is_mute, stream_info->client.dl.codec_param.sample_rate, stream_info->is_silence);


    } else if (BT_ULL_ROLE_CLIENT == ull_context->role) {
        bt_ull_le_srv_aud_quality_info_t *sink_info = bt_ull_le_srv_get_aud_quality_info();
        /* sync headset mix ratio setting to dongle */
        bt_ull_le_srv_memcpy(&(stream_info->dl_mix_ratio), &(stream_ctx->dl_mix_ratio), sizeof(bt_ull_mix_ratio_t));
        /* sync latency to dongle */
        stream_info->dl_latency = stream_ctx->dl_latency;
        stream_info->ul_latency = stream_ctx->ul_latency;
        stream_info->audio_quality = sink_info->expect_aud_quality;
        stream_ctx->allow_play = bt_ull_le_am_is_allow_play()? BT_ULL_PLAY_ALLOW : BT_ULL_PLAY_DISALLOW;
        stream_info->allow_play = stream_ctx->allow_play;

        /* 1. set share buffer address to controller, maybe init in main.c, same as LE Audio */
        ull_report("[ULL][LE] sync headset/earbuds is allow_play: 0x%x, dl_latency:%d, ul_latency:%d, Mic_codec_sample_rate: 0x%x, aud_quality: %d", 5,
            stream_info->allow_play, stream_info->dl_latency, stream_info->ul_latency, stream_info->server.stream[BT_ULL_MIC_TRANSMITTER].codec_param.sample_rate, sink_info->expect_aud_quality);
    }

    if (NULL != request) {
        status = bt_ull_le_srv_send_data(handle, (uint8_t*)request, len);
        bt_ull_le_srv_memory_free(request);
    }
    return status;
}

static bt_status_t bt_ull_le_srv_sync_streaming_status(bt_ull_le_stream_port_mask_t stream_port, uint8_t event)
{
    bt_status_t result = BT_STATUS_FAIL;

    uint8_t i;
    bt_ull_streaming_t stream;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_port(stream_port);
    if ((BT_ULL_ROLE_SERVER == ctx->role) && (BT_STATUS_SUCCESS == bt_ull_le_srv_get_stream_by_transmitter(trans_type, &stream))) {
        if (BT_ULL_EVENT_STREAMING_STOP_IND == event) {
            for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {/*notify all connected device.*/
                if ((BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)
#ifndef AIR_SILENCE_DETECTION_ENABLE
                    && (!bt_ull_le_srv_is_streaming(bt_ull_le_srv_get_stream_mode(trans_type)))
#endif
                    ) {
                    /* notify client DL stop after both 2-rx stop */
                    uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_streaming_t);
                    uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                    if (NULL != request) {
                        request[0] = event;
                        bt_ull_streaming_t *streaming_port =(bt_ull_streaming_t *)(request + sizeof(bt_ull_req_event_t));
                        streaming_port->streaming_interface = stream.streaming_interface;
                        streaming_port->port = stream.port;
                        result = bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                        bt_ull_le_srv_memory_free(request);
                        ull_report("[ULL][LE] bt_ull_le_srv_sync_streaming_status result is 0x%x, conn_handle is 0x%x, trans_type:0x%x", 3, 
                            result, g_ull_le_link_info[i].conn_handle, trans_type);
                    }
                }
            }
        } else if (BT_ULL_EVENT_STREAMING_START_IND == event) {
            if (((!ctx->is_streaming_locked)) && (BT_STATUS_SUCCESS == bt_ull_le_srv_open_audio_stream(stream_port))) {
                for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {/*notify all connected device.*/
                    if (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                        uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_streaming_t);
                        uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                        if (NULL != request) {
                            request[0] = event;
                            bt_ull_streaming_t *streaming_port =(bt_ull_streaming_t *)(request + sizeof(bt_ull_req_event_t));
                            streaming_port->streaming_interface = stream.streaming_interface;
                            streaming_port->port = stream.port;
                            result = bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                            bt_ull_le_srv_memory_free(request);
                            ull_report("[ULL][LE] bt_ull_le_srv_sync_streaming_status result is 0x%x, conn_handle is 0x%x, trans_type:0x%x", 3, 
                                result, g_ull_le_link_info[i].conn_handle, trans_type);
                        }
                    }
                }
            }
        }
    }
    return result;
}

static bt_status_t bt_ull_le_srv_handle_codec_sync_event(uint16_t handle, bt_ull_le_srv_stream_context_t *stream_info)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_link_info_t *link_info = NULL;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    //uint32_t temp_usb_sample_rate = 0;
    if ((BT_HANDLE_INVALID == handle) || (!stream_info)) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event, Error,invlaid params", 0);
        return BT_STATUS_FAIL;
    }

    if (NULL == (link_info = bt_ull_le_srv_get_link_info(handle))) {
        return BT_STATUS_FAIL;
    }
    if ((!link_info) || (BT_ULL_LE_LINK_STATE_SYNC_CODEC_INFO != link_info->curr_state)) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event, Error, curr state is %d", 1, link_info->curr_state);
        return BT_STATUS_FAIL;
    }
    bool sup_mhdt_8m = bt_gap_le_check_remote_features(handle, BT_GAP_LE_EDRLE_8M_PHY);
    bool sup_mhdt_4m = bt_gap_le_check_remote_features(handle, BT_GAP_LE_EDRLE_4M_PHY);
    if (sup_mhdt_4m) {
        link_info->feature_msk |= BT_ULL_LE_SRV_FEATURE_MHDT_4M_PHY;
    }
    if (sup_mhdt_8m) {
        link_info->feature_msk |= BT_ULL_LE_SRV_FEATURE_MHDT_8M_PHY;
    }
    ull_report("[ULL][LE] Read mHDT feaeture mask: EDRLE4M support: %d, EDRLE8M support: %d, link feature: %x", 3, sup_mhdt_4m, sup_mhdt_8m, link_info->feature_msk);

    if (BT_ULL_ROLE_SERVER == ctx->role) {
        uint8_t idx = 0;
        ull_report("[ULL][LE] Server RECIVE CODEC from Client!! client type: 0x%x, allow_play: 0x%x, dl_latency: %d, ul_latency: %d, UL_Codec_samplerate: %d, is_silence: %d", 6,
            ctx->client_type, stream_info->allow_play, stream_info->dl_latency, stream_info->ul_latency, stream_info->server.stream[BT_ULL_MIC_TRANSMITTER].codec_param.sample_rate, stream_ctx->is_silence);
        link_info->curr_state = BT_ULL_LE_LINK_STATE_READY;
        /**< Notify app ULL is real connected, include sync codec info. */
        bt_ull_le_srv_handle_pairing_event(handle, BT_STATUS_SUCCESS, ctx->client_type);

        if (BT_ULL_SPEAKER_CLIENT == ctx->client_type) {
            uint8_t i = 0;
            uint8_t other_link_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
            uint8_t change_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
            link_info->latency = stream_info->dl_latency;
            link_info->allow_play = stream_info->allow_play;
            for (i = 0;i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
                if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                    handle != g_ull_le_link_info[i].conn_handle && \
                    BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                    other_link_latency = g_ull_le_link_info[i].latency;
                    break;
                    }
            }
            change_latency = (link_info->latency > other_link_latency) ? link_info->latency : other_link_latency;
            ull_report("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event(latency), %d, %d, %d, %d, %d", 5, \
                change_latency, stream_ctx->dl_latency, other_link_latency, ctx->cs_size, ctx->curr_stream_state);
            if (change_latency != stream_ctx->dl_latency && \
                BT_ULL_LE_SRV_LATENCY_DEFAULT != other_link_latency && \
                (1 < ctx->cs_size) && \
                (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER <= ctx->curr_stream_state)) {
                    stream_ctx->dl_latency = change_latency;
                    stream_ctx->ul_latency = (stream_ctx->dl_latency > BT_ULL_LE_SRV_LATENCY_SINGLE_LINK_MODE) ? BT_ULL_LE_SRV_LATENCY_SINGLE_LINK_MODE : BT_ULL_LE_SRV_LATENCY_DEFAULT;
                    /* set microphone uplink transmitter latency, to stop/start transmitter.*/
                    bt_ull_le_conn_srv_set_latency(change_latency);
                    if (ctx->is_cig_created && !ctx->is_removing_cig_for_switch_latency) {
                        /*not disconnect CIS, just change another params. */
                        //if (BT_STATUS_SUCCESS != bt_ull_le_conn_srv_switch_latency(stream_ctx->ul_latency)) {
                        status = bt_ull_le_conn_srv_remove_air_cig_params();
                        if (BT_STATUS_SUCCESS == status || BT_STATUS_PENDING == status) {
                            ctx->is_removing_cig_for_switch_latency = true;
                        } else {
                            ull_report_error("[ULL][LE] bt_ull_le_srv_handle_set_streaming_latency, controller switch latency fail", 0);
                        }
                    } else {
                    bt_ull_le_srv_notify_client_restart_streaming(BT_ULL_LE_RESTART_STREAMING_REASON_LATENCY_CHANGE);
                }
            } else {
                stream_ctx->dl_latency = change_latency;
                stream_ctx->ul_latency = stream_info->ul_latency;
                bt_ull_le_conn_srv_set_latency(stream_ctx->dl_latency);
            }
            stream_ctx->audio_quality = stream_info->audio_quality;
            bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_SERVER, stream_ctx->audio_quality);
            bt_ull_le_conn_srv_change_audio_quality(stream_ctx->audio_quality);
        } else {
            stream_ctx->dl_latency = stream_info->dl_latency;
            stream_ctx->ul_latency = stream_info->ul_latency;
            stream_ctx->allow_play = stream_info->allow_play;
            uint8_t i = 0;
            bt_ull_le_srv_audio_quality_t other_link_aud_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID;
            bt_ull_le_srv_audio_quality_t change_aud_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID;
            bt_ull_le_srv_aud_quality_info_t *sink_info = bt_ull_le_srv_get_aud_quality_info();
            //uint8_t idx = bt_ull_le_srv_get_idx_by_handle(handle);
            if (((BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY == stream_info->audio_quality && \
                sup_mhdt_4m) || \
                (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION == stream_info->audio_quality && \
                sup_mhdt_8m) || \
                BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == stream_info->audio_quality || \
                BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_A2DP == stream_info->audio_quality || \
                BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_HFP == stream_info->audio_quality || \
                BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == stream_info->audio_quality) && \
                (BT_ULL_LE_CODEC_LC3PLUS == stream_ctx->codec_type)) {
                link_info->aud_quality = stream_info->audio_quality;
            } else {
                link_info->aud_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT;
            }
            
            if (BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
                for (i = 0;i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
                    if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                        handle != g_ull_le_link_info[i].conn_handle && \
                        BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                        other_link_aud_quality = g_ull_le_link_info[i].aud_quality;
                        break;
                    }
                }
            }
            if (stream_ctx->audio_quality == BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER) {
                change_aud_quality = stream_ctx->audio_quality;
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == other_link_aud_quality) {
                change_aud_quality = other_link_aud_quality;
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID == other_link_aud_quality) {
                change_aud_quality = link_info->aud_quality;
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == link_info->aud_quality) {
                change_aud_quality = other_link_aud_quality;
            } else {
                change_aud_quality = ((BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID != other_link_aud_quality) && (link_info->aud_quality >= other_link_aud_quality)) ? other_link_aud_quality : link_info->aud_quality;
            }

            ull_report("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event(aud_quality), chg: %d, curr:%d, other: %d, link: %d, cs: %d, state: %d, excpet: %d, codec: %d", 8, \
                change_aud_quality, stream_ctx->audio_quality, other_link_aud_quality, link_info->aud_quality, ctx->cs_size, ctx->curr_stream_state, sink_info->expect_aud_quality, stream_ctx->codec_type);
/*
            if (change_aud_quality != stream_ctx->audio_quality && \
                1 < ctx->cs_size && \
                BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT != other_link_aud_quality && \
                BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER <= ctx->curr_stream_state
                ) {
                stream_ctx->audio_quality = change_aud_quality;
            } else {
                stream_ctx->audio_quality = change_aud_quality;
            }
*/
            stream_ctx->audio_quality = change_aud_quality;
            sink_info->expect_aud_quality = change_aud_quality;
            //For Auto test script check log
            if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == change_aud_quality) {
                ull_report("[ULL][LE] ULL2.0 Default Quality Enable.", 0);
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY == change_aud_quality) {
                ull_report("[ULL][LE] ULL2.1 High-Quality Enable.", 0);
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION == change_aud_quality) {
                ull_report("[ULL][LE] ULL2.1 High-Resolution Enable.", 0);
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == change_aud_quality) {
                ull_report("[ULL][LE] ULL2.0 Low Power Mode Enable.", 0);
            }
            bt_ull_le_srv_send_audio_quality_by_handle(handle, change_aud_quality);

            bt_ull_le_conn_srv_change_audio_quality(stream_ctx->audio_quality);
            bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_SERVER, stream_ctx->audio_quality);
            //bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
            bt_ull_le_conn_srv_set_air_cig_params_table(handle, ctx->cis_num, stream_ctx->codec_type);

        }
        //stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].codec_param.sample_rate = stream_info->server.stream[BT_ULL_MIC_TRANSMITTER].codec_param.sample_rate;
        /* init controller media data share buffer info */
        if (!ctx->is_share_buff_set) {
            status = bt_ull_le_srv_init_share_info(ctx->client_type);
            if ((BT_STATUS_SUCCESS == status) && (BT_STATUS_SUCCESS == bt_ull_le_srv_set_avm_share_buffer(ctx->role, ctx->client_type, ctx->cis_num))) {
                ctx->is_share_buff_set = true;
            } else {
                ull_report_error("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event set AVM buffer error!!", 0);
            }
        }
#if 0
        if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
            /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server]: Always create aircis when ull service is connected*/
            if (!stream_ctx->allow_play) {
                if (!ctx->is_removing_cig_for_switch_latency || !ctx->is_removing_cig_for_change_aud_quality || !ctx->is_removing_cig_for_change_aud_codec) {
                    if (!ctx->is_cig_created) {
                        /*Set Air CIG parameters. */
                        if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_set_air_cig_params(ctx->cis_num)) {
                            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                        } else {
                            ull_report_error("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event, set cig error", 0);
                        }
                    } else {
                        if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER > ctx->curr_stream_state) {
                            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                        }
                        if (BT_ULL_LE_LINK_STATE_READY == link_info->curr_state) {
                            /* Create CIS */
                            if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(link_info->conn_handle)) {
                                link_info->curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                            }
                        } else if (BT_ULL_LE_LINK_STATE_STREAMING == link_info->curr_state) {
                            /**<Air CIS has been created done and open audio transmitter successfully. */
                            if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER == ctx->curr_stream_state)
                                bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                        }
                    }
                }
            }
        }
#endif
        /* store mix ratio setting from client */
        bt_ull_le_srv_memcpy(&(stream_ctx->dl_mix_ratio), &(stream_info->dl_mix_ratio), sizeof(bt_ull_mix_ratio_t));

        for (idx = 0; idx < BT_ULL_MAX_STREAMING_NUM; idx++) {
            ull_report("[ULL][LE] Server rx recieved Mix ratio status, streaming[%d] type:0x%x, port:0x%x, ratio: %d", 4,
                idx, stream_ctx->dl_mix_ratio.streamings[idx].streaming.streaming_interface, stream_ctx->dl_mix_ratio.streamings[idx].streaming.port, stream_ctx->dl_mix_ratio.streamings[idx].ratio);
        }

        /* update downlink volume*/
    #if 1
        ull_report("[ULL][LE] dl volume, L:%d, R:%d", 2, stream_ctx->client.dl.volume.vol_left, stream_ctx->client.dl.volume.vol_right);
        bt_status_t ret = BT_STATUS_FAIL;
        uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_original_duel_volume_t);
        uint8_t *vol_update_request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
        if (NULL != vol_update_request) {
            vol_update_request[0] = BT_ULL_EVENT_UPDATE_VOLUME_IND;
            bt_ull_original_duel_volume_t *volume_update = (bt_ull_original_duel_volume_t*)(vol_update_request + sizeof(bt_ull_req_event_t));
            bt_ull_le_srv_memcpy(volume_update, &(stream_ctx->client.dl.volume), sizeof(bt_ull_original_duel_volume_t));
            ret = bt_ull_le_srv_send_data(handle, (uint8_t*)vol_update_request, len);
            bt_ull_le_srv_memory_free(vol_update_request); 
        }
        ull_report("[ULL][LE] server update volume to client, ret: %d", 1, ret);
    #endif
        ull_report("[ULL][LE] straming_port mask:0x%x", 1, stream_ctx->streaming_port);
        uint8_t link_num = bt_ull_le_srv_get_connected_link_num();
        ull_report("[ULL][LE]bt_ull_le_srv_handle_codec_sync_event: ct: %d, connected_num: %d, cs: %d, aws_conneted: %d", 4, \
            ctx->client_type, link_num, ctx->cs_size, link_info->sink_cfg.aws_connected);
        if (BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
            if (link_num == ctx->cs_size) {
                if (bt_timer_ext_find(BT_ULL_LE_CONN_WAITING_TIMER_ID)) {
                    bt_timer_ext_stop(BT_ULL_LE_CONN_WAITING_TIMER_ID);
                }
                ull_report("[ULL][LE] The second link is connected!!!!", 0);
                goto NEXT_ACTION;
            } else {
                if (link_info->sink_cfg.aws_connected) {
                    bt_timer_ext_start(BT_ULL_LE_CONN_WAITING_TIMER_ID, 0, BT_ULL_LE_SRV_WAITING_TIMER, bt_ull_le_srv_conn_waiting_timeout_hdl);
                    ull_report("[ULL][LE] Waite second link conneted!!!!", 0);
                    return status;
                } else {
                    ull_report("[ULL][LE] AWS is not connected, not waite!!!!", 0);
                    goto NEXT_ACTION;
                }
            }
        } 

NEXT_ACTION:

        if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
            /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server]: Always create aircis when ull service is connected*/
            if (!stream_ctx->allow_play) {
                if (!ctx->is_removing_cig_for_switch_latency || !ctx->is_removing_cig_for_change_aud_quality || !ctx->is_removing_cig_for_change_aud_codec) {
                    if (!ctx->is_cig_created) {
                        /*Set Air CIG parameters. */
                        if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_set_air_cig_params(ctx->cis_num)) {
                            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                        } else {
                            ull_report_error("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event, set cig error", 0);
                        }
                    } else {
                        if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER > ctx->curr_stream_state) {
                            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                        }
                        if (BT_ULL_LE_LINK_STATE_READY == link_info->curr_state) {
                            /* Create CIS */
                            if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(link_info->conn_handle)) {
                                link_info->curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                            }
                        } else if (BT_ULL_LE_LINK_STATE_STREAMING == link_info->curr_state) {
                            /**<Air CIS has been created done and open audio transmitter successfully. */
                            if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER == ctx->curr_stream_state)
                                bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                        }
                    }
                }
            }
        }

        if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) && stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].usb_sample_rate) {
            if (!stream_ctx->is_silence) {
                bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_GAMING, BT_ULL_EVENT_STREAMING_START_IND);
            }
        }
        if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) && stream_ctx->server.stream[BT_ULL_CHAT_TRANSMITTER].usb_sample_rate) {
            if (!stream_ctx->is_silence) {
                bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_CHAT, BT_ULL_EVENT_STREAMING_START_IND);
            }
        }
        if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) && stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].usb_sample_rate) {
            bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_MIC, BT_ULL_EVENT_STREAMING_START_IND);
        }
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_IN) {
            bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_LINE_IN, BT_ULL_EVENT_STREAMING_START_IND);
        }
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_LINE_OUT_ENABLE) || (defined AIR_WIRELESS_MIC_ENABLE)
        if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT) {
            bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT, BT_ULL_EVENT_STREAMING_START_IND);
        }
#endif
#if (defined AIR_DONGLE_I2S_SLV_OUT_ENABLE) || (defined AIR_DONGLE_I2S_MST_OUT_ENABLE) || (defined AIR_WIRELESS_MIC_ENABLE)
        if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT) {
            bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT, BT_ULL_EVENT_STREAMING_START_IND);
        }
#endif
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE)
        if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_IN) {
            bt_ull_le_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_I2S_IN, BT_ULL_EVENT_STREAMING_START_IND);
        }
#endif

        if (!bt_ull_le_srv_check_inactive_aircis_feature_on()) {
            if ((1 < ctx->cs_size) && (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER <= ctx->curr_stream_state) \
                && !stream_ctx->allow_play && !stream_ctx->is_silence) {
                /*means that the other link have been opening or opened the audio stream. */
                /* Create CIS */
                if (BT_ULL_SPEAKER_CLIENT == ctx->client_type) {
                    if (!link_info->allow_play) {
                if (BT_STATUS_SUCCESS == (status = bt_ull_le_conn_srv_establish_air_cis(handle))) {
                    link_info->curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                }
                    }
                } else {
                    if (BT_STATUS_SUCCESS == (status = bt_ull_le_conn_srv_establish_air_cis(handle))) {
                        link_info->curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                    }
                }
                ull_report("[ULL][LE] New link create AirCIS, ct, %d, Conn handle:0x%x, link_info->curr_state: %d", 3,
                            ctx->client_type, handle, link_info->curr_state);
                return status;
            }
        }

    } else {
        ull_report("[ULL][LE] Client RECIVE CODEC from Server!! Server streaming port: 0x%x, dl is_mute:0x%x, microphone is_mute: 0x%x, is_silence: %d", 4,
            stream_info->streaming_port, stream_info->client.dl.is_mute, stream_info->client.ul.is_mute, stream_info->is_silence);
        stream_ctx->codec_type = stream_info->codec_type;  /* default  using LC3pluscodec */
        stream_ctx->streaming_port = stream_info->streaming_port;
        stream_ctx->is_silence = stream_info->is_silence;
        if (stream_ctx->is_silence) {
            stream_ctx->streaming_port &= (~BT_ULL_LE_STREAM_PORT_MASK_GAMING);
            stream_ctx->streaming_port &= (~BT_ULL_LE_STREAM_PORT_MASK_CHAT);
            stream_ctx->is_silence = false;
        }

        if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
            /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: Runtime config priority by port streaming status*/
            if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
                bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_HIGH);
            } else if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_NORMAL);
            } else {
                bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_LOW);
            }            
        }
        /* exchange codec info */
        if ((BT_ULL_LE_LINK_STATE_SYNC_CODEC_INFO == link_info->curr_state) &&
            (BT_STATUS_SUCCESS == (status = bt_ull_le_srv_handle_sync_codec(handle)))) {
            link_info->curr_state = BT_ULL_LE_LINK_STATE_READY;
            ull_report("[ULL][LE] Client SYNC CODEC to server success!!", 0);
        } else {
            ull_report_error("[ULL][LE] Client send Sync info fail, role is: %d", 0);
            link_info->curr_state = BT_ULL_LE_LINK_STATE_IDLE;
            bt_ull_le_srv_handle_pairing_event(handle, BT_STATUS_FAIL, ctx->client_type);
            return BT_STATUS_FAIL;
        }

        /**< Notify app ULL is real connected, include sync codec info. */
        bt_ull_le_srv_handle_pairing_event(handle, BT_STATUS_SUCCESS, ctx->client_type);
        if (ctx->adaptive_bitrate_param.enable) {
            bt_ull_le_srv_enable_adaptive_bitrate_mode_internal(handle, &ctx->adaptive_bitrate_param);
        }
        /*stream_ctx->codec_type = stream_info->codec_type; 
        stream_ctx->streaming_port = stream_info->streaming_port;
        stream_ctx->is_silence = stream_info->is_silence;
        if (stream_ctx->is_silence) {
            stream_ctx->streaming_port &= (~BT_ULL_LE_STREAM_PORT_MASK_GAMING);
            stream_ctx->streaming_port &= (~BT_ULL_LE_STREAM_PORT_MASK_CHAT);
            stream_ctx->is_silence = false;
        }*/
        /*save the DL info*/
        bt_ull_le_srv_memcpy(&(stream_ctx->client.dl.volume), &(stream_info->client.dl.volume), sizeof(bt_ull_original_duel_volume_t));     
        bt_ull_le_am_sync_volume(&(stream_ctx->client.dl.volume));
        bt_ull_le_srv_memcpy(&(stream_ctx->server_chat_volume), &(stream_info->server_chat_volume), sizeof(bt_ull_original_duel_volume_t));
        bt_ull_le_srv_memcpy(&(stream_ctx->server_linein_volume), &(stream_info->server_linein_volume), sizeof(bt_ull_original_duel_volume_t));
        bt_ull_le_srv_memcpy(&(stream_ctx->server_linei2s_volume), &(stream_info->server_linei2s_volume), sizeof(bt_ull_original_duel_volume_t));
        stream_ctx->client.dl.is_mute = stream_info->client.dl.is_mute;
        stream_ctx->client.dl.usb_sample_rate = stream_info->client.dl.usb_sample_rate;

        /*save the UL info*/
        bt_ull_le_srv_memcpy(&(stream_ctx->client.ul.volume), &(stream_info->client.ul.volume), sizeof(bt_ull_original_duel_volume_t));
        stream_ctx->client.ul.is_mute = stream_info->client.ul.is_mute;
        stream_ctx->client.ul.usb_sample_rate = stream_info->client.ul.usb_sample_rate;

        ull_report("[ULL][LE] Client Codec, dl_codec_samplerate:%d, mic codec_samplerate: %d, 8M sup: %d, 4M sup: %d", 4,
            stream_ctx->client.dl.codec_param.sample_rate, stream_ctx->client.ul.codec_param.sample_rate, sup_mhdt_8m, sup_mhdt_4m);
        /* Enable DSP if dongle is playing */
        if (BT_ULL_LE_CODEC_LC3PLUS == stream_ctx->codec_type && \
            BT_ULL_EARBUDS_CLIENT == ctx->client_type && \
            BT_ULL_LE_SCENARIO_ULLV2_0 == ctx->client_preferred_scenario_type) {
            g_ull_le_wait_audio_quality = true;
            ull_report("[ULL][LE] Not open audio, wait the sync audio quality!!", 0);
        } else {
            if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                stream_ctx->client.dl.is_dl_dummy_mode = false;
                bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
                if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state) {
                    bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
                }
            } else {
                if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                    /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: PC is not streaming, but also open DSP with priority 0*/
                    bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
                    if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
                    }                      
                }
            }
            if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
                bt_ull_streaming_t stream;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                stream.port = 0;
                bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
            }
        }
    }
    return status;
}

static bt_status_t bt_ull_le_srv_handle_start_streaming(bt_ull_streaming_t *streaming)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE] bt_ull_le_srv_handle_start_streaming, streaming_if :0x%x, port: 0x%x", 2, streaming->streaming_interface, streaming->port);

    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if (bt_ull_le_srv_is_connected() && (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)) {
        #ifdef AIR_WIRELESS_MIC_ENABLE
            stream_ctx->streaming_port |= BT_ULL_LE_STREAM_PORT_MASK_MIC;
            if (ctx->is_streaming_locked) {
                stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_MIC; 
                ull_report_error("[ULL][LE] bt_ull_le_srv_handle_start_streaming, role is client, Fail, streaming is locked!", 0);
                return BT_STATUS_FAIL;
            } else {
                /* Enable UL directly*/
                status = bt_ull_le_am_play(BT_ULL_LE_AM_UL_MODE, stream_ctx->codec_type, true);
                if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state) {
                    bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
                }
            }
        #else
            stream_ctx->streaming_port |= BT_ULL_LE_STREAM_PORT_MASK_MIC;
            status = bt_ull_le_srv_play_am(streaming, BT_ULL_LE_STREAM_MODE_UPLINK);
        #endif
        }  else {
             ull_report_error("[ULL][LE] bt_ull_le_srv_handle_start_streaming, role is client, but port type is un-supported", 0);
        }
    } else if (BT_ULL_ROLE_SERVER == ctx->role) {
        uint8_t open_now = false;
        uint32_t sampe_rate = 0;
        bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
        bt_ull_le_stream_port_mask_t start_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
        stream_ctx->streaming_port |= start_port;
        if (ctx->is_streaming_locked) {
            stream_ctx->locking_port |= start_port;
        }
        bt_ull_le_srv_server_stream_t *stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
        if (!stream_info) {
            return BT_STATUS_FAIL;
        }
        if (bt_ull_le_srv_is_connected()) {
            /* 1. Initial transmitter after ull link connected & streaming on */
            if (BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface) {
                sampe_rate = USB_Audio_Get_RX_Sample_Rate(!(streaming->port));
                if (sampe_rate != 0) {
                    if (stream_info->usb_sample_rate != sampe_rate) {
                        stream_info->usb_sample_rate = sampe_rate;
                        stream_info->is_need_restart = true;
                    }
                    open_now = true;
                } else {
                    open_now = false;
                }
            } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface) {
                sampe_rate = USB_Audio_Get_TX_Sample_Rate(streaming->port);
                if (sampe_rate != 0) {
                    open_now = true;
                    if (stream_info->usb_sample_rate != sampe_rate) {
                        stream_info->usb_sample_rate = sampe_rate;
                        stream_info->is_need_restart = true;
                    }
                } else {
                    open_now = false;
                }
            } else {
                open_now = true;
            }
            ull_report("[ULL][LE] Open Audio streaming now ? open_now: %d, sample_rate: %x, need restart: %d", 3, open_now, sampe_rate, stream_info->is_need_restart);
            if (open_now) {
                if ((stream_info->is_need_restart) && (!ctx->is_streaming_locked)) {
                    bt_ull_le_at_restart(trans_type);
                    stream_info->is_need_restart = false;
                }
                status = bt_ull_le_srv_sync_streaming_status(start_port, BT_ULL_EVENT_STREAMING_START_IND);
            }
        } else {
            ull_report_error("[ULL][LE] bt_ull_le_srv_handle_start_streaming fail! have not any connected device", 0);
        }
        /* if USB Host MIC ep enable, we shuold trigger usb mic driver tx enable */
#if 0
        if (BT_ULL_LE_STREAM_PORT_MASK_MIC == start_port) {
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
            ull_report("[ULL][LE] bt_ull_le_srv_handle_start_streaming, hal_usb_set_endpoint_tx_ready", 0);
            hal_usb_set_endpoint_tx_ready(1);
#endif
        }
#endif 
    }
    return status;
}

static bt_status_t bt_ull_le_srv_handle_stop_streaming(bt_ull_streaming_t *streaming)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_server_stream_t *stream_info;
    bt_ull_le_stream_port_mask_t stop_port = 0;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE] bt_ull_le_srv_handle_stop_streaming, streaming_if :0x%x, port: 0x%x", 2, streaming->streaming_interface, streaming->port);

    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if (bt_ull_le_srv_is_connected() && (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)) {
            stream_ctx->streaming_port &= ~BT_ULL_LE_STREAM_PORT_MASK_MIC;
            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_MIC;
            }
            /* disable mic if ull is playing */
            bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);

            ull_report("[ULL][LE] client recieved disable mic, curr_stream_state:0x%x, ", 1, ctx->curr_stream_state);
        } else {
            ull_report_error("[ULL][LE] bt_ull_le_srv_handle_stop_streaming, role is client, but port type is un-supported", 0);
        }
        return status;
    } else if (BT_ULL_ROLE_SERVER == ctx->role) {
        bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
        stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
        stop_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
        stream_ctx->streaming_port &= ~stop_port;
        if (stream_ctx->locking_port & stop_port) {
            stream_ctx->locking_port &= ~stop_port;
        }

        if (bt_ull_le_srv_is_connected()) {
            bt_ull_le_srv_sync_streaming_status(stop_port, BT_ULL_EVENT_STREAMING_STOP_IND);
        } else {
            ull_report_error("[ULL][LE] bt_ull_le_srv_handle_stop_streaming fail! have not any connected device", 0);
        }

        if (stream_info) {
            if (BT_STATUS_SUCCESS != bt_ull_le_at_stop(trans_type, true)) {
                ull_report_error("[ULL][LE] [Error] bt_ull_le_srv_handle_stop_streaming, stop transmitter fail",0);
            }
        }
    }

    return status;
}

static bool bt_ull_le_srv_check_all_downlink_stream_is_mute(bt_ull_transmitter_t trans_type)
{
    bool is_all_stream_mute = true;
    bt_ull_le_srv_server_stream_t *stream_info;
    uint8_t i;
    for (i = BT_ULL_GAMING_TRANSMITTER; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
        if ((BT_ULL_MIC_TRANSMITTER == i) || (BT_ULL_LINE_OUT_TRANSMITTER == i) || (BT_ULL_I2S_OUT_TRANSMITTER == i)) {
            continue;
        }
        stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, i);
        if (bt_ull_le_at_is_start(i) && stream_info) {
            if (false == stream_info->is_mute) {
                //is_all_stream_mute = false;
                return false;
            }
        }
    }
    ull_report("[ULL][LE]bt_ull_le_srv_check_all_downlink_stream_is_mute, result: %d", 1, is_all_stream_mute);
    return is_all_stream_mute;
}
static bt_status_t bt_ull_le_srv_handle_set_volume(bt_ull_volume_t *vol)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE] bt_ull_handle_set_volume, streaming_if:0x%x, port:0x%x, action:0x%x, audio_channel: 0x%x, volume_level:%d", 5,
        vol->streaming.streaming_interface, vol->streaming.port, vol->action, vol->channel, vol->volume);

    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        /* check the all link*/
        uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM; /*Client Only Support 1 link*/
        while (0 != i) {
            i--;
            if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                /* sync volume operation to service */
                uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_volume_t);
                uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                if (NULL != request) {
                    request[0] = BT_ULL_EVENT_VOLUME_ACTION;
                    bt_ull_volume_t *vol_action = (bt_ull_volume_t *)(request + sizeof(bt_ull_req_event_t));
                    bt_ull_le_srv_memcpy(vol_action, vol, sizeof(bt_ull_volume_t));
                    bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                    bt_ull_le_srv_memory_free(request);
                }
            }
        }
    } else if (BT_ULL_ROLE_SERVER == ctx->role) {
        bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(&(vol->streaming));
        bt_ull_le_srv_server_stream_t *stream_info;
        stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
        if (!stream_info) {
            ull_report_error("[ULL][LE] bt_ull_le_srv_handle_set_volume, stream_info is NULL!!", 0);
            return BT_STATUS_FAIL;
        }
        if (BT_STATUS_SUCCESS == bt_ull_le_at_set_volume(trans_type, vol->action, vol->channel, vol->volume)) {
            ull_report("[ULL][LE] bt_ull_le_at_set_volume success", 0);
        }
        if (BT_ULL_STREAMING_INTERFACE_SPEAKER == vol->streaming.streaming_interface
            || BT_ULL_STREAMING_INTERFACE_LINE_IN == vol->streaming.streaming_interface
            || BT_ULL_STREAMING_INTERFACE_I2S_IN == vol->streaming.streaming_interface) {
            //stream_ctx->server_chat_volume.vol_left = vol->volume;
            if ((BT_ULL_AUDIO_CHANNEL_DUAL == vol->channel) ||
                (BT_ULL_AUDIO_CHANNEL_LEFT == vol->channel)) {
                if (BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME == vol->action) {
                    stream_ctx->client.dl.volume.vol_left = vol->volume;
                    stream_info->volume.vol_left = vol->volume;
                } else if (BT_ULL_VOLUME_ACTION_SET_UP == vol->action) {
                    uint8_t volume_temp = stream_ctx->client.dl.volume.vol_left;
                    volume_temp = (volume_temp+7 < 100)?(volume_temp+7):(100);
                    stream_ctx->client.dl.volume.vol_left = volume_temp;
                    stream_info->volume.vol_left = vol->volume;
                } else if (BT_ULL_VOLUME_ACTION_SET_DOWN== vol->action) {
                    //stream_ctx->client.dl.volume.vol_left = (vol->volume-7 < 100) ? (vol->volume+7) : (100);
                    uint8_t volume_temp = stream_ctx->client.dl.volume.vol_left;
                    if (volume_temp < 7) {
                        volume_temp = 0;
                    } else {
                        volume_temp = volume_temp - 7;
                    }
                    stream_ctx->client.dl.volume.vol_left = volume_temp;
                    stream_info->volume.vol_left = volume_temp;
                }
            }
            if ((BT_ULL_AUDIO_CHANNEL_DUAL == vol->channel) ||
                (BT_ULL_AUDIO_CHANNEL_RIGHT == vol->channel)) {
                if (BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME == vol->action) {
                    stream_ctx->client.dl.volume.vol_right= vol->volume;
                    stream_info->volume.vol_right = vol->volume;
                } else if (BT_ULL_VOLUME_ACTION_SET_UP == vol->action) {
                    uint8_t volume_temp = stream_ctx->client.dl.volume.vol_right;
                    volume_temp = (volume_temp+7 < 100)?(volume_temp+7):(100);
                    stream_ctx->client.dl.volume.vol_right = volume_temp;
                    stream_info->volume.vol_right = vol->volume;
                } else if (BT_ULL_VOLUME_ACTION_SET_DOWN== vol->action) {
                    //stream_ctx->client.dl.volume.vol_left = (vol->volume-7 < 100) ? (vol->volume+7) : (100);
                    uint8_t volume_temp = stream_ctx->client.dl.volume.vol_right;
                    if (volume_temp < 7) {
                        volume_temp = 0;
                    } else {
                        volume_temp = volume_temp - 7;
                    }
                    stream_ctx->client.dl.volume.vol_right = volume_temp;
                    stream_info->volume.vol_right = vol->volume;
                }             
            }
            if (0 == stream_info->volume.vol_left && 0 == stream_info->volume.vol_right) {
                stream_info->is_mute = true;
            } else {
                stream_info->is_mute = false;
            }
		}
        //workaround for BTA-44562
        if (stream_info->is_mute) {
            if (bt_ull_le_srv_check_all_downlink_stream_is_mute(trans_type)) {
                ull_report("[ULL][LE][DEBUG]bt_ull_le_srv_handle_set_volume, check all dl stream mute, notify client! ", 0);
            } else {
                return status;
            }
        }

#if 1
        /* check the all link */
        /* sync Server volume to client */
        uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
        while (0 != i) {
            i--;
            if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                /* Sync PC volume to client */
                uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_volume_ind_t);
                uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                if (NULL != request) {
                    request[0] = BT_ULL_EVENT_VOLUME_IND;
                    bt_ull_volume_ind_t *p_vol = (bt_ull_volume_ind_t *)(request + sizeof(bt_ull_req_event_t));
                    bt_ull_le_srv_memcpy(&(p_vol->streaming), &(vol->streaming), sizeof(p_vol->streaming));
                    p_vol->channel = vol->channel;

                    if ((BT_ULL_AUDIO_CHANNEL_DUAL == vol->channel) ||
                        (BT_ULL_AUDIO_CHANNEL_LEFT == vol->channel)) {
                        //p_vol->vol.vol_left = vol->volume;
                        p_vol->vol.vol_left = stream_ctx->client.dl.volume.vol_left;
                        //p_vol->vol_gain.vol_left = vol->gain;
                    }
                    if ((BT_ULL_AUDIO_CHANNEL_DUAL == vol->channel) ||
                        (BT_ULL_AUDIO_CHANNEL_RIGHT == vol->channel)) {
                        //p_vol->vol.vol_right = vol->volume;
                        p_vol->vol.vol_right = stream_ctx->client.dl.volume.vol_right;
                        //p_vol->vol_gain.vol_right = vol->gain;
                    }
                    bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                    bt_ull_le_srv_memory_free(request);
                }
            }
        }
#endif
    }
    return status;
}

static bt_status_t bt_ull_le_srv_handle_set_streaming_mute(bt_ull_streaming_t *streaming, bool is_mute)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();

    ull_report("[ULL][LE] bt_ull_le_srv_handle_set_streaming_mute, type:0x%x, port:0x%x, is_mute: 0x%x,", 3,
        streaming->streaming_interface, streaming->port, is_mute);

    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface) {
            stream_ctx->client.ul.is_mute = is_mute;
            bt_ull_le_am_set_mute(BT_ULL_LE_AM_UL_MODE, is_mute);
        } else {
            stream_ctx->client.dl.is_mute = is_mute;
            bt_ull_le_am_set_mute(BT_ULL_LE_AM_DL_MODE, is_mute);
        }
        /* sync mute operation to server */
        uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM;
        while (0 != i) {
            i--;
            if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_streaming_t);
                uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);

                if (NULL != request) {
                    request[0] = is_mute ? BT_ULL_EVENT_VOLUME_MUTE : BT_ULL_EVENT_VOLUME_UNMUTE;
                    bt_ull_le_srv_memcpy((bt_ull_streaming_t *)(request + sizeof(bt_ull_req_event_t)), streaming, sizeof(bt_ull_streaming_t));
                    status = bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                    bt_ull_le_srv_memory_free(request);
                }
            }
        }

    } else if (BT_ULL_ROLE_SERVER == ctx->role) {
        if (BT_ULL_MIC_CLIENT == ctx->client_type) {
            //TBD: need to notify the remote MIC device to mute
        } else {
            /* Server mute or unmute streaming */
            bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
            if ((BT_ULL_GAMING_TRANSMITTER <= trans_type) && (BT_ULL_TRANSMITTER_MAX_NUM > trans_type)) {
                stream_ctx->server.stream[trans_type].is_mute = is_mute;
            } else {
                ull_assert(0 && "unknown streaming interface type");
                return BT_STATUS_FAIL;
            }
            bt_ull_le_at_set_mute(trans_type, is_mute);
#if 1
            /* sync mute operation to server */
            uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
            while (0 != i) {
                i--;
                if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                    (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                    uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_streaming_t);
                    uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);

                    if (NULL != request) {
                        request[0] = is_mute ? BT_ULL_EVENT_VOLUME_MUTE : BT_ULL_EVENT_VOLUME_UNMUTE;
                        bt_ull_le_srv_memcpy((bt_ull_streaming_t *)(request + sizeof(bt_ull_req_event_t)), streaming, sizeof(bt_ull_streaming_t));
                        status = bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                        bt_ull_le_srv_memory_free(request);
                    }
                }
            }
#endif
        }
    } else {
        ull_assert(0 && "[ULL][LE] unknown role");
    }
    return status;
}

static bt_status_t bt_ull_le_srv_handle_set_streaming_sample_rate(bt_ull_sample_rate_t *sample_rate)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_server_stream_t *stream_info;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_ROLE_SERVER != ctx->role) {
        ull_report_error("[ULL][LE] error role: 0x%x", 1, ctx->role);
        return BT_STATUS_FAIL;
    }
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(&(sample_rate->streaming));
    stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
    bt_ull_le_stream_port_mask_t start_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
    if (NULL == stream_info) {
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE] bt_ull_le_srv_handle_set_streaming_sample_rate, if_type:0x%x, port:0x%x, rate: %d, usb sample rate: %d, need restart: %d, is_transmitter_start: %d, streaming_port: %x", 7,
        sample_rate->streaming.streaming_interface,
        sample_rate->streaming.port,
        sample_rate->sample_rate,
        stream_info->usb_sample_rate,
        stream_info->is_need_restart,
        stream_ctx->server.stream[trans_type].is_transmitter_start,
        stream_ctx->streaming_port
        );

    if ((stream_info) && (sample_rate->sample_rate != stream_info->usb_sample_rate)) {
        /** TBD:remove CIG and notify remote device to disable audio manager.*/
        stream_info->usb_sample_rate = sample_rate->sample_rate;
        stream_info->is_need_restart = true;
    }
    /* Audio transmitter will be restart.*/
    if (stream_info->is_need_restart && (!ctx->is_streaming_locked) && bt_ull_le_srv_is_connected()) {
        bt_ull_le_at_restart(trans_type);
        stream_info->is_need_restart = false;
    }

    if ( bt_ull_le_srv_is_connected() && (start_port & stream_ctx->streaming_port)) {
        status = bt_ull_le_srv_sync_streaming_status(start_port, BT_ULL_EVENT_STREAMING_START_IND);
    }
    return status;
}

static bt_status_t bt_ull_le_srv_handle_set_streaming_sample_size(bt_ull_streaming_sample_size_t *sample_size)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_server_stream_t *stream_info;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();

    if (BT_ULL_ROLE_SERVER != ctx->role) {
        ull_report_error("[ULL][LE] SAMPLE SIZE, error role: 0x%x", 1, ctx->role);
        return BT_STATUS_FAIL;
    }
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(&(sample_size->streaming));
    if (NULL == (stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type))){
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE] SAMPLE SIZE,, if_type:0x%x, port:0x%x, sample size: %d, usb sample size: %d", 4,
        sample_size->streaming.streaming_interface, sample_size->streaming.port, sample_size->sample_size, stream_info->usb_sample_size);
        if (sample_size->sample_size != stream_info->usb_sample_size) {
            stream_info->usb_sample_size = sample_size->sample_size;
            stream_info->is_need_restart = true;
/*
            if (stream_ctx->server.stream[trans_type].is_transmitter_start && bt_ull_le_srv_is_connected()) {
                ull_report("[ULL][LE] SAMPLE SIZE, port: %d streaming is runing", 1, trans_type);
                bt_ull_le_at_restart(trans_type);
            } else {
                ull_report("[ULL][LE] SAMPLE SIZE, no streaming is runing, record only!", 0);
            }
*/
        }
    return status;
}

static bt_status_t bt_ull_le_srv_handle_set_streaming_sample_channel(bt_ull_streaming_sample_channel_t *sample_channel)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_server_stream_t *stream_info;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();

    if (BT_ULL_ROLE_SERVER != ctx->role) {
        ull_report_error("[ULL][LE] SAMPLE CHANNEL, error role: 0x%x", 1, ctx->role);
        return BT_STATUS_FAIL;
    }
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(&(sample_channel->streaming));
    stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
    if (NULL == stream_info) {
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE] SAMPLE CHANNEL, if_type:0x%x, port:0x%x, sample channel: %d, usb sample channel: %d", 4,
        sample_channel->streaming.streaming_interface, sample_channel->streaming.port, sample_channel->sample_channel, stream_info->usb_sample_channel);
        if (sample_channel->sample_channel != stream_info->usb_sample_channel) {
            stream_info->usb_sample_channel = sample_channel->sample_channel;
            stream_info->is_need_restart = true;
/*
            if (stream_ctx->server.stream[trans_type].is_transmitter_start && bt_ull_le_srv_is_connected()) {
                ull_report_error("[ULL][LE] SAMPLE CHANNEL, port: %d streaming is runing", 1, trans_type);
                bt_ull_le_at_restart(trans_type)
            }
*/
        }
    return status;
}

static bt_status_t bt_ull_le_srv_handle_set_streaming_latency(bt_ull_latency_t *req_latency, bt_handle_t handle)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE] set latency to: 0x%x, old latency: 0x%x, handle: %x, removing_cig: %d", 4, \
        req_latency->latency, stream_ctx->dl_latency, handle, ctx->is_removing_cig_for_switch_latency);
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        /* notify dongle latency switch */
        if (req_latency->latency != stream_ctx->dl_latency) {
            stream_ctx->dl_latency = req_latency->latency;
            if (stream_ctx->dl_latency > BT_ULL_LE_SRV_LATENCY_SINGLE_LINK_MODE) {
                stream_ctx->ul_latency = BT_ULL_LE_SRV_LATENCY_MULTI_LINK_CONNECTING_MODE;
            } else {
                stream_ctx->ul_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
            }

            uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM;
            while (0 != i) {
                i--;
                if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                    (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                    uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_latency_t);
                    uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                    if (NULL != request) {
                        request[0] = BT_ULL_EVENT_LATENCY_SWITCH;
                        bt_ull_latency_t *latency = (bt_ull_latency_t *)(request + sizeof(bt_ull_req_event_t));
                        bt_ull_le_srv_memcpy(&(latency->streaming), &(req_latency->streaming), sizeof(bt_ull_streaming_t));
                        latency->latency = stream_ctx->dl_latency;
                        status = bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                        bt_ull_le_srv_memory_free(request);
                    }
                }
            }
        }
    } else if (BT_ULL_ROLE_SERVER == ctx->role) {

        if (BT_ULL_SPEAKER_CLIENT == ctx->client_type) {
            bt_ull_le_srv_link_info_t *link = bt_ull_le_srv_get_link_info(handle);
            if (NULL == link) {
                return BT_STATUS_FAIL;
            }
            ull_report("[ULL][LE] ull spk set latency-1: %d, %d", 2, req_latency->latency, link->latency);
            if (req_latency->latency != link->latency) {
                link->latency = req_latency->latency;
            } else {
                return status;
            }
            uint8_t i = 0;
            uint8_t other_link_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
            uint8_t change_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
            for (i = 0;i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
                if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                    handle != g_ull_le_link_info[i].conn_handle && \
                    BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                    other_link_latency = g_ull_le_link_info[i].latency;
                    break;
                }
            }
            change_latency = (link->latency > other_link_latency) ? link->latency : other_link_latency;
            ull_report("[ULL][LE] ull spk set latency-2: %d, %d", 2, change_latency, stream_ctx->dl_latency);
            if (change_latency != stream_ctx->dl_latency) {
                    stream_ctx->dl_latency = change_latency;
                    stream_ctx->ul_latency = (stream_ctx->dl_latency > BT_ULL_LE_SRV_LATENCY_SINGLE_LINK_MODE) ? BT_ULL_LE_SRV_LATENCY_SINGLE_LINK_MODE : BT_ULL_LE_SRV_LATENCY_DEFAULT;
                    /* set microphone uplink transmitter latency, to stop/start transmitter.*/
                    bt_ull_le_conn_srv_set_latency(req_latency->latency);
                    ull_report("[ULL][LE] ull spk set latency-3: %d, %d", 2, ctx->is_cig_created, ctx->is_removing_cig_for_switch_latency);
                    if (ctx->is_cig_created && !ctx->is_removing_cig_for_switch_latency) {
                        /*not disconnect CIS, just change another params. */
                        status = bt_ull_le_conn_srv_remove_air_cig_params();
                        if (BT_STATUS_SUCCESS == status || BT_STATUS_PENDING == status) {
                            ctx->is_removing_cig_for_switch_latency = true;
                        } else {
                            ull_report_error("[ULL][LE] bt_ull_le_srv_handle_set_streaming_latency, controller switch latency fail", 0);
                        }
                    } else {
                    bt_ull_le_srv_notify_client_restart_streaming(BT_ULL_LE_RESTART_STREAMING_REASON_LATENCY_CHANGE);
                }

            }
        } else {

                if (req_latency->latency != stream_ctx->dl_latency) {
                    stream_ctx->dl_latency = req_latency->latency;
                    stream_ctx->ul_latency = (stream_ctx->dl_latency > BT_ULL_LE_SRV_LATENCY_SINGLE_LINK_MODE) ? BT_ULL_LE_SRV_LATENCY_SINGLE_LINK_MODE : BT_ULL_LE_SRV_LATENCY_DEFAULT;
                    /* set microphone uplink transmitter latency, to stop/start transmitter.*/
                    bt_ull_le_conn_srv_set_latency(req_latency->latency);
                    if (ctx->is_cig_created && !ctx->is_removing_cig_for_switch_latency) {
                        /*not disconnect CIS, just change another params. */
                        //if (BT_STATUS_SUCCESS != bt_ull_le_conn_srv_switch_latency(stream_ctx->ul_latency)) {
                        status = bt_ull_le_conn_srv_remove_air_cig_params();
                        if (BT_STATUS_SUCCESS == status || BT_STATUS_PENDING == status) {
                            ctx->is_removing_cig_for_switch_latency = true;
                        } else {
                            ull_report_error("[ULL][LE] bt_ull_le_srv_handle_set_streaming_latency, controller switch latency fail", 0);
                        }
                    } else {
                    bt_ull_le_srv_notify_client_restart_streaming(BT_ULL_LE_RESTART_STREAMING_REASON_LATENCY_CHANGE);
                }
            }
        }
    }
    return status;
}

static bt_status_t bt_ull_le_srv_handle_set_streaming_mix_ratio(bt_ull_mix_ratio_t *ratio)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();

    if (ratio) {
        uint8_t idx = 0;
        for (idx = 0; idx < BT_ULL_MAX_STREAMING_NUM; idx++) {
            ull_report("[ULL][LE] bt_ull_handle_set_streaming_mix_ratio, streaming[%d] type:0x%x, port:0x%x, ratio: %d", 4,
                idx, ratio->streamings[idx].streaming.streaming_interface, ratio->streamings[idx].streaming.port, ratio->streamings[idx].ratio);
        }
        /* Now, only speaker streaming can mix */
        if (BT_ULL_STREAMING_INTERFACE_SPEAKER != ratio->streamings[0].streaming.streaming_interface
            || BT_ULL_STREAMING_INTERFACE_SPEAKER != ratio->streamings[1].streaming.streaming_interface) {
            return BT_STATUS_FAIL;
        }
        bt_ull_le_srv_memcpy(&(stream_ctx->dl_mix_ratio), ratio, sizeof(bt_ull_mix_ratio_t));

        if (BT_ULL_ROLE_CLIENT == ctx->role) {
            /* sync mix setting to server */
            uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
            while (0 != i) {
                i--;
                if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                    (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                    uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_mix_ratio_t);
                    uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                    if (NULL != request) {
                        request[0] = BT_ULL_EVENT_MIX_RATIO_ACTION;
                        bt_ull_mix_ratio_t *ratio_req = (bt_ull_mix_ratio_t *)(request + sizeof(bt_ull_req_event_t));
                        bt_ull_le_srv_memcpy(ratio_req, ratio, sizeof(bt_ull_mix_ratio_t));
                        bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                        bt_ull_le_srv_memory_free(request);
                    }
                }
            }
        } else if (BT_ULL_ROLE_SERVER == ctx->role) {
            /*  set gaming & chat streaming mixing */
            bt_ull_le_at_set_mix_ratio(ratio);
        } else {
            ull_assert(0 && "error role");
        }
    } else {
        status = BT_STATUS_FAIL;
        ull_assert(0 && "ratio is NULL!");
    }
    return status;
}

static bt_status_t bt_ull_le_srv_handle_usb_hid_control(bt_ull_usb_hid_control_t action)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    ull_report("[ULL][LE] usb_hid_control: 0x%x, role:0x%x", 2, action, ctx->role);

#ifdef AIR_USB_ENABLE
    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
        ull_report("[ULL][LE] usb_hid_control, current is XBOX mode, ignore the HID control event", 0);
        return BT_STATUS_FAIL;
    }
#endif /* MTK_USB_DEMO_ENABLED */

    if (bt_ull_le_srv_is_connected()) {
        if (BT_ULL_ROLE_CLIENT == ctx->role) {
            uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM; /*Client Only Support 1 link*/
            while (0 != i) {
                i--;
                if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                    (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                    /* sync play/pause/next/previous action to server */
                    uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_usb_hid_control_t);
                    uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                    if (NULL != request) {
                        request[0] = BT_ULL_EVENT_USB_HID_CONTROL_ACTION;
                        request[1] = action;
                        bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                        bt_ull_le_srv_memory_free(request);
                    }
                }
            }
        } else if (BT_ULL_ROLE_SERVER == ctx->role) {
#if defined(AIR_USB_ENABLE) && defined(AIR_USB_AUDIO_ENABLE) && defined(AIR_USB_HID_ENABLE)
            if (BT_ULL_USB_HID_PLAY_PAUSE_TOGGLE == action
                || BT_ULL_USB_HID_PAUSE == action
                || BT_ULL_USB_HID_PLAY == action) {
                if (USB_HID_STATUS_OK == USB_HID_PlayPause()) {
                    ull_report("[ULL][LE] usb_hid_control, PlayPause Success", 0);
                }
            } else if (BT_ULL_USB_HID_PREVIOUS_TRACK == action) {
                if (USB_HID_STATUS_OK == USB_HID_ScanPreviousTrack()) {
                    ull_report("[ULL][LE] usb_hid_control, Previous Track Success", 0);
                }
            } else if (BT_ULL_USB_HID_NEXT_TRACK == action) {
                if (USB_HID_STATUS_OK == USB_HID_ScanNextTrack()) {
                    ull_report("[ULL][LE] usb_hid_control, Next Track Success", 0);
                }
            }
#endif
        }
    } else {
        ull_report_error("[ULL][LE] bt_ull_le_srv_handle_usb_hid_control fail! have not any connected device", 0);
    }
    return status;
}

static bt_status_t bt_ull_le_srv_connection_handle(uint16_t handle, bt_ull_connection_state_t state)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_ULL_DISCONNECTED == state) {
        bt_ull_le_conn_srv_delete_all_cache_cmd_by_handle(handle);
        /* release auido source && close codec */
        if (BT_ULL_ROLE_CLIENT == ctx->role) {
            /* stop audio manager */
            bt_ull_le_am_sync_sidetone_status(false);//disconnect ULL, disable sidetone
            bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
            /* stop audio manager */
            bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true);
        } else if (BT_ULL_ROLE_SERVER == ctx->role) {
            /* close audio transmitter */
            uint8_t i;
            if (!bt_ull_le_srv_is_connected()) {
                for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
                    bt_ull_le_at_deinit(i, true);
                }

                /* restore latency to default */
                stream_ctx->dl_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
                stream_ctx->allow_play = BT_ULL_PLAY_ALLOW;
                stream_ctx->audio_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT;
                if (ctx->is_cig_created) {
                    bt_ull_le_conn_srv_remove_air_cig_params();
                    //ctx->is_cig_created = false;
                }

                if (ctx->is_share_buff_set) {
                    bt_ull_le_srv_deinit_share_info();
                    ctx->is_share_buff_set = false;
                }
            }
        }
        ull_report("[ULL][LE] BT_ULL_LE_DISCONNECTED", 0);
    } else if (BT_ULL_CONNECTED == state) {
        bt_ull_le_srv_link_info_t *link_info = bt_ull_le_srv_get_link_info(handle);

        if (!link_info) {
            ull_report_error("[ULL][LE] bt_ull_le_srv_connection_handle, can't find link info !", 0);
            return BT_STATUS_FAIL;
        }
        /* reset critical data seq */
        ctx->critical_data_tx_seq = 0x01;
        ctx->critical_data_rx_seq = 0x00;

        /* send configuration */
        if (BT_ULL_ROLE_CLIENT == ctx->role) {
            ull_report("[ULL][LE] bt_ull_le_srv_init, client_type: %d", 1, g_ull_le_ctx.client_type);
            if (BT_ULL_HEADSET_CLIENT == g_ull_le_ctx.client_type || BT_ULL_EARBUDS_CLIENT == g_ull_le_ctx.client_type) {
                bool aircis_inactive_enable = bt_ull_le_srv_read_aircis_inactive_mode_enable();
                ctx->aircis_inactive_mode_enable = aircis_inactive_enable;
            }

            if ((BT_STATUS_SUCCESS == (status = bt_ull_le_srv_send_configuration(handle))) &&
                (BT_ULL_LE_LINK_FINDING_CS_INFO == link_info->curr_state)) {
                link_info->curr_state = BT_ULL_LE_LINK_STATE_CONFIGURATION;
            } else {
                ull_report_error("[ULL][LE] send Sync info fail, role is: %d", 1, ctx->role);
                link_info->curr_state = BT_ULL_LE_LINK_STATE_IDLE;
                bt_ull_le_srv_handle_pairing_event(handle, BT_STATUS_FAIL, ctx->client_type);
            }
        } else {
            link_info->curr_state = BT_ULL_LE_LINK_STATE_CONFIGURATION;
        }
        ull_report("[ULL][LE] BT_ULL_LE_CONNECTED, curr_state: %d", 1, link_info->curr_state);
    } else {
        ull_assert(0 && "[LE] unknown connection state");
    }
    return status;
}

static bt_status_t bt_ull_le_srv_send_configuration(bt_handle_t handle)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    uint32_t aws_link = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), NULL, 0);
    uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_configuration_t);
    ull_report_error("[ULL][LE] bt_ull_le_srv_send_configuration, aud_loaction: %d, aws: %d", 2, ctx->audio_location, aws_link);
    uint8_t *send_cfg = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
    if (NULL != send_cfg) {
        send_cfg[0] = BT_ULL_EVENT_SEND_CONFIGURATION;
        bt_ull_le_srv_configuration_t cfg;
        bt_ull_le_srv_memset(&cfg, 0, sizeof(bt_ull_le_srv_configuration_t));
        cfg.cis_id = 0;
        cfg.audio_location = ctx->audio_location;
		cfg.aws_connected = (uint8_t)aws_link;
        bt_ull_le_srv_memcpy(&cfg.capability_msk, &g_bt_ull_le_capability, sizeof(bt_ull_le_srv_capability_t));
        cfg.aircis_inactive_enable = ctx->aircis_inactive_mode_enable;
        bt_ull_le_srv_memcpy((bt_ull_le_srv_configuration_t *)(send_cfg + sizeof(bt_ull_req_event_t)), &cfg, sizeof(bt_ull_le_srv_configuration_t));
        status = bt_ull_le_srv_send_data(handle, (uint8_t*)send_cfg, len);
        bt_ull_le_srv_memory_free(send_cfg);
    }
    return status;
}

#if 1	// richard for ab1571d command processing

extern void key_oceanvp_trige_proc(void);
extern void key_oceanvp_up_proc(void);
extern void key_ocean_vpdown_proc(void);
extern void key_volumeup_proc(uint8_t volume_up_mode);
extern void key_volumedown_proc(uint8_t volume_down_mode);
extern void key_avrcp_next_proc(uint8_t vol_m_flag);
extern void key_avrcp_prev_proc(void);
extern void key_fact_pairing_proc(void);
extern void key_switch_anc_and_passthrough_proc(void);
extern bool key_multifunction_short_click();
extern uint8_t Is_earbuds_agent_proc(void);
extern void app_set_ab1571d_version(uint8_t version_data);
extern void app_set_ab1571d_main_version(uint8_t version_data);
extern uint8_t ab1585h_command_no;
extern uint8_t ab1585h_command_data;
extern void BT_send_data_proc(void);
#if 0	// for production test
#include "voice_prompt_api.h"
#endif
void ab1571d_data_processing(uint8_t temp_command_no,uint8_t temp_command_data)
{
	ull_report("[ULL][LE] key from charging box: key=%d, event=%d ", 2, temp_command_no, temp_command_data);

#if 0	// for production test
	if(temp_command_no==0)
	{
		voice_prompt_play_sync_vp_ull_volume_up();
		return;
	}		
	else if(temp_command_no==1)
	{
		voice_prompt_play_sync_vp_ull_volume_down();
		return;
	}		
	else if(temp_command_no==2)
	{
		voice_prompt_play_sync_vp_ull_mbutton();
		return;
	}
#endif

//	if(Is_earbuds_agent_proc()==0)
//		return;
	switch(temp_command_no)
	{
		case 0:			// key0: volume up
			if(temp_command_data==1)			// SP
			{
				key_volumeup_proc(0);
			}
			else if(temp_command_data==2)		// DP
			{
			}
			else if(temp_command_data==0x22)	// LP2
			{
				//key_volumeup_proc(1);
			}
			break;
		case 1:			// key1: volume down
			if(temp_command_data==1)			// SP
			{
				key_volumedown_proc(0);
			}
			else if(temp_command_data==2)		// DP
			{
			}			
			else if(temp_command_data==0x22)	// LP2
			{
				//key_volumedown_proc(1);			
			}
			break;
		case 2:			// Multi-function
			if(temp_command_data==1)									// SP
			{
				key_multifunction_short_click();
			}
			else if(temp_command_data==2)		// DP
			{
				key_avrcp_next_proc(0);  // 0 next;1 ha MODE SWITCH;
			}			
			else if(temp_command_data==3)		// tripe P
			{
				key_avrcp_prev_proc();
			}			
			else if(temp_command_data==0x21)							// case hold 1s
			{
				key_switch_anc_and_passthrough_proc();
			}
			break;
		case 3:		// ab1571d version
			app_set_ab1571d_version(temp_command_data);
			ab1585h_command_no=2;	// 2: version feedback
			ab1585h_command_data=0;
			BT_send_data_proc();

			ab1585h_command_no=4;	// 4: bt address
			ab1585h_command_data=0;
			BT_send_data_proc();
			break;
		case 4:		// pairing
			key_fact_pairing_proc();
			break;
		case 5:		// OCEAN   ON/OFF 
			key_oceanvp_trige_proc();
			break;
			#if 0
			
		case 6:		// OCEAN  VP+ 
			key_oceanvp_up_proc();
			break;
			
		case 7:		// OCEAN  VP- 
			key_ocean_vpdown_proc();
			break;
			#endif
		default:
			break;
	}
}

// Audeara LE messaging patch
extern void Audeara_BT_send_data_proc(uint8_t frame, uint8_t * data, uint16_t length);

static nvdm_status_t AudearaWriteNVKey(uint8_t *data_buf, uint16_t data_len)
{
	nvdm_status_t nvstatus;
	uint16_t keyid = data_buf[0] | ((uint16_t)data_buf[1])<<8;
	//LOG_MSGID_I(AUCOMM_DEBUG, "Key write [%04x] length = %d data [%02x][%02x]", 4, keyid, data_len-2, data_buf[2],data_buf[3]);
	nvstatus = nvkey_write_data(keyid, &data_buf[2], data_len-2);
	return nvstatus;
}

static nvdm_status_t AudearaReadNVKey(uint8_t *data_buf, uint8_t *out_buf, uint16_t * data_len)
{
	uint32_t sz;
	nvdm_status_t nvstatus;
	uint16_t keyid = data_buf[0] | ((uint16_t)data_buf[1])<<8;
	sz = *data_len;
	nvstatus = nvkey_read_data(keyid, out_buf, &sz);

	//LOG_MSGID_I(AUCOMM_DEBUG, "Key read [%04x] length = %d data [%02x][%02x]", 4, keyid, sz, out_buf[0],out_buf[1]);
	*data_len = (uint16_t) sz;
	return nvstatus;
}

bool AudearaGetNotificationState(void)
{
    return aua_notification_state;
}

void AudearaSetNotificationState(bool audeara_notification_state)
{
    aua_notification_state = audeara_notification_state;
}

void audeara_ab1571d_data_processing(AUDEARA_BTULL_MESSAGE_FRAMES_T frame, uint8_t * data, uint16_t length)
 {
     bt_bd_addr_t *mac = {0}; 
     switch(frame)
     {
        case AUA_BUDSFRAME_READ_SYSTEM_STATUS:
            break;
        case AUA_BUDSFRAME_WRITE_NVKEY:
            reply_buf[0] =  AudearaWriteNVKey(data, length);
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_WRITE_NVKEY, reply_buf, 1);
            break;
        case AUA_BUDSFRAME_READ_NVKEY:
            AudearaReadNVKey(data, data, &length); // Just return the nvkey data in the input data
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_READ_NVKEY, data, length);
            break;
        case AUA_BUDSFRAME_SEND_RACE_CMD:
            if(data[0] == 0) // No realy, send to master bud    
            {
                //data[2] = 0x05, 3 = 0x5A, 4 = length 1, 5 = length 2, 6 = ID1, 7 = ID2
               // if(data[6] == 0x87 && data[7] == 0x2C) // OPCODE for Hearing aid commands
                //{
                 //       app_hear_through_race_cmd_handler((ptr_race_pkt_t)&data[2], (length-2), data[1]);
               // }
                //else
                {
                    audeara_race_cmd_local_handler(SERIAL_PORT_DEV_UNDEFINED, &data[2], AUA_BUDSFRAME_SEND_RACE_CMD, 0);
                }
            }

            else
            {
                reply_buf[0] = 0x05;
                reply_buf[1] = 0x5A;
                reply_buf[2] = (((length - 2) + 4) & 0xFF);
                reply_buf[3] = ((((length - 2) + 4) >> 8) & 0xFF);
                reply_buf[4] = 0x01;
                reply_buf[5] = 0x0D;
                reply_buf[6] = 0x05;
                reply_buf[7] = 0x06;

                memcpy(&reply_buf[8], &data[2], (length - 2));
                
                audeara_race_cmd_local_handler(data[1], reply_buf, AUA_BUDSFRAME_SEND_RACE_CMD, 1);
            }
                break;
        case AUA_BUDSFRAME_SEND_RACE_CMD_RESP:
            // do nothing, this is a one way response command
            break;
        case AUA_BUDSFRAME_POWER_ON:
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                    APPS_EVENTS_INTERACTION_REQUEST_REBOOT, NULL, 0,
                    NULL, 0);
            reply_buf[0] = 0;
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_POWER_ON, reply_buf, 1);
            break;
        case AUA_BUDSFRAME_POWER_OFF:
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF, NULL, 0,
                NULL, 0);
            reply_buf[0] = 0;
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_POWER_OFF, reply_buf, 1);
            break;
        case AUA_BUDSFRAME_CPU_RESET:
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                APPS_EVENTS_INTERACTION_REQUEST_REBOOT, NULL, 0,
                NULL, 0);
            reply_buf[0] = 0;
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_CPU_RESET, reply_buf, 1);
            break;
        case AUA_BUDSFRAME_FACTORY_RESET:
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                APPS_EVENTS_INTERACTION_FACTORY_RESET_REQUEST, NULL, 0,
                NULL, 0);
            reply_buf[0] = 0;
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_FACTORY_RESET, reply_buf, 1);
            break;
        case AUA_BUDSFRAME_WRITE_LOCAL_MAC_ADDR:
			memcpy(mac, data, 6);
			bt_device_manager_store_local_address(mac);
            reply_buf[0] = 0x00;
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_WRITE_LOCAL_MAC_ADDR, reply_buf, 1);
            break;
        case AUA_BUDSFRAME_READ_LOCAL_MAC_ADDR:
            mac = bt_device_manager_get_local_address();
            memcpy(reply_buf, mac, 6);
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_READ_LOCAL_MAC_ADDR, reply_buf, 6);
            break;
        case AUA_BUDSFRAME_PROGRAM_FW_UPDATE:
            // This is to trigger a FOTA update, not transfer the update across, that is in a different function

            break;
        case AUA_BUDSRAME_RACE_NOTI_ON_OFF:
            AudearaSetNotificationState((bool)data[0]);
            Audeara_BT_send_data_proc(AUA_BUDSRAME_RACE_NOTI_ON_OFF, data, 1); // Ping back notification state
            break;
        case AUA_BUDSFRAME_TEST_DATA:
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_TEST_DATA, data, length);
            break;
        case AUA_BUDSFRAME_VOL_UP:
            key_volumeup_proc(0);
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_VOL_UP, 0, 1);
            break;
        case AUA_BUDSFRAME_VOL_DOWN:
            key_volumedown_proc(0);
            Audeara_BT_send_data_proc(AUA_BUDSFRAME_VOL_DOWN, 0, 1);
            break;
     }
     
 }

 // End audeara LE messaging patch
#endif
static void bt_ull_le_srv_rx_data_handle(uint16_t handle, uint8_t *data ,uint16_t length)
{
    if (BT_HANDLE_INVALID == handle) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_rx_data_handle, invalid handle!", 0);
        return;
    }
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_link_info_t *link_info = bt_ull_le_srv_get_link_info(handle);
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (data && link_info && (0 != length)) {
        uint8_t *req_data = (uint8_t *)data;
        ull_report("[ULL][LE] rx recieved event: 0x%x, role: 0x%x, conn_handle: 0x%x", 3, req_data[0], ctx->role, handle);

        switch(req_data[0]) {
            case BT_ULL_EVENT_COORDINATED_SET_INFO: {
                bt_ull_le_srv_cs_info_t *cs_info = (bt_ull_le_srv_cs_info_t *)(req_data + sizeof(bt_ull_req_event_t));
                if (cs_info) {
                    if ((BT_ULL_ROLE_SERVER == ctx->role) && (BT_ULL_LE_SRV_CS_RSP == cs_info->op) &&
                        (BT_ULL_LE_LINK_FINDING_CS_INFO == link_info->curr_state)) {
                        ull_report("[ULL][LE] Server received Find CS RSP From Client, set_size: %d, client_type: %d", 2, cs_info->set_size, cs_info->client_type);
                        if ((0 != cs_info->set_size) && (!bt_ull_le_srv_sirk_is_null(cs_info->sirk))) {/*Valid CS info*/
                        #ifdef AIR_WIRELESS_MIC_ENABLE
                            if (BT_ULL_MIC_CLIENT != cs_info->client_type) {
                                ull_report("Not wireless mic, wrong client type!!", 0);
                                bt_ull_le_connected_info_t connection_info;
                                connection_info.status = BT_STATUS_FAIL;
                                connection_info.conn_handle = handle;
                                bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CONNECTED, (void *)&connection_info, sizeof(bt_ull_le_connected_info_t));
                                return;
                            }
                        #else
                            if (BT_ULL_MIC_CLIENT == cs_info->client_type) {
                                ull_report("Is wireless mic, wrong client type!!", 0);
                                bt_ull_le_connected_info_t connection_info;
                                connection_info.status = BT_STATUS_FAIL;
                                connection_info.conn_handle = handle;
                                bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CONNECTED, (void *)&connection_info, sizeof(bt_ull_le_connected_info_t));
                                return;
                            }
                        #endif
                            bt_ull_le_srv_set_client_type(cs_info->client_type);
                            bt_ull_le_srv_set_sirk(cs_info->sirk);
                            bt_ull_le_srv_set_coordinated_set_size(cs_info->set_size);
                            bt_ull_le_srv_memcpy((uint8_t *)&link_info->group_device_addr, (uint8_t *)&cs_info->group_device_addr, sizeof(bt_addr_t) * (BT_ULL_LE_MAX_LINK_NUM - 1));
                            ull_report("[ULL][LE]Group device addr:type: %d, addr: %x-%x-%x-%x-%x-%x", 7, link_info->group_device_addr[0].type,
                                link_info->group_device_addr[0].addr[0],
                                link_info->group_device_addr[0].addr[1],
                                link_info->group_device_addr[0].addr[2],
                                link_info->group_device_addr[0].addr[3],
                                link_info->group_device_addr[0].addr[4],
                                link_info->group_device_addr[0].addr[5]
                                );
                            /**< server update codec param  */
                            bt_ull_le_srv_memcpy(&(ctx->client_preferred_codec_param), &(cs_info->codec_param), sizeof(bt_ull_le_srv_client_preferred_codec_param));
                            stream_ctx->codec_type = ctx->client_preferred_codec_param.codec_type;
                            ull_report("[ULL][LE] Server update codec param, get client preffered codec type : %d", 1, stream_ctx->codec_type);
                            if (BT_ULL_LE_CODEC_LC3PLUS == stream_ctx->codec_type) {
                            #ifndef AIR_AUDIO_LC3PLUS_CODEC_ENABLE
                                ull_assert(0&&"server not support lc3plus codec!!");
                            #endif
                            } else if (BT_ULL_LE_CODEC_OPUS == stream_ctx->codec_type) {
                            #ifndef AIR_AUDIO_VEND_CODEC_ENABLE
                                ull_assert(0&&"server not support opus codec!!");
                            #endif
                            } else if (BT_ULL_LE_CODEC_ULD == stream_ctx->codec_type) {
                            #ifndef AIR_AUDIO_ULD_CODEC_ENABLE
                                ull_assert(0&&"server not support ULD codec!!");
                            #endif
                            } else if(BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS == stream_ctx->codec_type) {
                               #ifndef AIR_AUDIO_ULD_CODEC_ENABLE
                                ull_assert(0&&"server not support DL_ULD_UL_LC3PLUS codec!!");
                               #endif
                            }
#ifdef AIR_AUDIO_VEND_CODEC_ENABLE
                            else if(BT_ULL_LE_CODEC_DL_ULD_UL_OPUS == stream_ctx->codec_type) {
                               #ifndef AIR_AUDIO_ULD_CODEC_ENABLE
                                    ull_assert(0&&"server not support DL_ULD_UL_OPUS codec!!");
                               #endif
                            }
#endif
                            else {
                                ull_assert(0&&"server not support any codec!!");
                            }
                            bt_ull_le_srv_set_codec_param_by_sample_rate(BT_ULL_ROLE_SERVER, ctx->client_preferred_codec_param.dl_samplerate, ctx->client_preferred_codec_param.ul_samplerate);

                            #ifdef AIR_WIRELESS_MIC_ENABLE
                            bt_ull_le_srv_set_client_preferred_channel_mode(cs_info->client_preferred_channel_mode);
                            if (BT_ULL_LE_CODEC_ULD != stream_ctx->codec_type) {
                            ull_report("[ULL][LE] Server(Wireless Mic RX) update codec param", 0);
                            bt_ull_le_srv_set_wireless_mic_codec_param(BT_ULL_ROLE_SERVER);
                            }
                        #else
                            /*bt_ull_le_srv_set_client_preferred_codec_type(cs_info->client_preferred_codec_type);
                                                                                       if (BT_ULL_LE_CODEC_OPUS == bt_ull_le_srv_get_client_preferred_codec_type()) {
                                                                                      bt_ull_le_srv_set_opus_codec_param();
                                                                                    }*/
                        #endif
                            bt_ull_le_srv_connection_handle(handle, BT_ULL_CONNECTED);

                            /*Init CIS NUM*/
                            bt_ull_le_srv_calculate_air_cis_count();
                        } else {
                            /**< notify app, ULL connect fail  */
                            ull_report_error("[ULL][LE] Server received CS rsp, but content is error!", 0);
                            bt_ull_le_srv_handle_pairing_event(handle, BT_STATUS_FAIL, BT_ULL_UNKNOWN_CLIENT);
                            link_info->curr_state = BT_ULL_LE_LINK_STATE_IDLE;
                        }
                    } else if ((BT_ULL_ROLE_CLIENT == ctx->role) && (BT_ULL_LE_SRV_CS_REQ == cs_info->op) &&
                                (BT_ULL_LE_LINK_STATE_IDLE == link_info->curr_state)) {/*Headset received Find CS request*/
                        /**< RSP Server the SIRK and CS */
                        uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_cs_info_t);
                        uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                        if (NULL != request) {
                            bt_key_t *sirk;
                            request[0] = BT_ULL_EVENT_COORDINATED_SET_INFO;
                            bt_ull_le_srv_cs_info_t *cs_rsp = (bt_ull_le_srv_cs_info_t *)(request + sizeof(bt_ull_req_event_t));
                            cs_rsp->op = BT_ULL_LE_SRV_CS_RSP;
                            cs_rsp->client_type = bt_ull_le_srv_get_client_type();
                            cs_rsp->set_size = bt_ull_le_srv_get_coordinated_set_size();
                            bt_ull_le_srv_memcpy((uint8_t *)&cs_rsp->group_device_addr, (uint8_t *)&g_group_device_addr, sizeof(bt_addr_t) * (BT_ULL_LE_MAX_LINK_NUM - 1));
                            ull_report("[ULL][LE]Group device addr:type: %d, addr: %x-%x-%x-%x-%x-%x", 7, cs_rsp->group_device_addr[0].type,
                                cs_rsp->group_device_addr[0].addr[0],
                                cs_rsp->group_device_addr[0].addr[1],
                                cs_rsp->group_device_addr[0].addr[2],
                                cs_rsp->group_device_addr[0].addr[3],
                                cs_rsp->group_device_addr[0].addr[4],
                                cs_rsp->group_device_addr[0].addr[5]
                                );                            

                        #ifdef AIR_WIRELESS_MIC_ENABLE
                            cs_rsp->client_preferred_channel_mode = bt_ull_le_srv_get_client_preferred_channel_mode();
                            ull_report("[ULL][LE] Client send CS rsp to Server, preferred channel mode is 0x%x", 1, cs_rsp->client_preferred_channel_mode);
                        #else
                            //cs_rsp->client_preferred_codec_type = bt_ull_le_srv_get_client_preferred_codec_type();
                            //ull_report("[ULL][LE] Client send CS rsp to Server, preferred codec type is 0x%x", 1, cs_rsp->client_preferred_codec_type);
                        #endif
                            /**< Sync codec param from client to server */
                            bt_ull_le_srv_memcpy(&(cs_rsp->codec_param), &(ctx->client_preferred_codec_param), sizeof(bt_ull_le_srv_client_preferred_codec_param));

                            if ((NULL != (sirk = bt_ull_le_srv_get_sirk())) && (0 != cs_rsp->set_size)) {
                                bt_ull_le_srv_memcpy(&(cs_rsp->sirk), sirk, sizeof(bt_key_t));
                                /**< ULL pairing success. */
                                link_info->curr_state = BT_ULL_LE_LINK_FINDING_CS_INFO;
                                /**< Send FIND CS RSP to Server. */
                                if (BT_STATUS_SUCCESS == bt_ull_le_srv_send_data(handle, (uint8_t*)request, len)) {
                                    bt_ull_le_srv_connection_handle(handle, BT_ULL_CONNECTED);
                                } else {
                                    link_info->curr_state = BT_ULL_LE_LINK_STATE_IDLE;
                                }
                                ull_report("[ULL][LE] Client send CS rsp to Server, curr_state is 0x%x", 1, link_info->curr_state);
                            } else {
                                link_info->curr_state = BT_ULL_LE_LINK_STATE_IDLE;
                                /**< Still Send FIND CS RSP to Server. */
                                if (BT_STATUS_SUCCESS != bt_ull_le_srv_send_data(handle, (uint8_t*)request, len)) {
                                    ull_report_error("[ULL][LE] Client send CS rsp error!", 0);
                                }
                            }
                            bt_ull_le_srv_memory_free(request);
                        }
                    }
                } else {
                    ull_report("[ULL][LE] CS info is NULL!", 0);
                }
                break;
            }

            case BT_ULL_EVENT_LE_STREAM_CODEC_INFO: {
                bt_ull_le_srv_stream_context_t *stream_info = (bt_ull_le_srv_stream_context_t *)(req_data + sizeof(bt_ull_req_event_t));
                if (NULL != stream_info) {
                    bt_ull_le_srv_handle_codec_sync_event(handle, stream_info);
                }
                break;
            }
            /*Client Received the Play status from Server*/
            case BT_ULL_EVENT_STREAMING_START_IND: {
                bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)(req_data + sizeof(bt_ull_req_event_t));
                ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_STREAMING_START_IND, if_type: 0x%x, port:0x%x", 2, streaming->streaming_interface, streaming->port);
                bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
                bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
                ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_STREAMING_START_IND, streaming_port: 0x%x", 1, stream_ctx->streaming_port);
                if (BT_ULL_ROLE_CLIENT == ctx->role) {
                    /*< ULL Client use the same avm buffer with LE Audio, it is initialized in app common part. please check it firstly.*/
                    if ((BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface)
                            || (BT_ULL_STREAMING_INTERFACE_LINE_IN == streaming->streaming_interface)
                            || (BT_ULL_STREAMING_INTERFACE_I2S_IN == streaming->streaming_interface)) {

                        if ((stream_ctx->client.dl.is_am_open) && (stream_ctx->streaming_port & stream_port)) {
                            /**< means that client had recieved the same event. ignore it. */
                            ull_report("[ULL][LE] means that client had recieved the same event. ignore it.",0);
                            break;
                        }
                        stream_ctx->streaming_port |= stream_port;
                        bt_ull_le_srv_indicate_server_play_is_allow(BT_ULL_PLAY_ALLOW, BT_ULL_ALLOW_PLAY_REASON_AUDIO); /*BTA-37651*/

                        if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                            /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: received DL start ind, raise ULL DL priority to MID*/
                            bool ul_playing = bt_ull_le_am_is_playing(BT_ULL_LE_AM_UL_MODE);
                            bool ul_streaming = bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK);
                            bool dl_playing = bt_ull_le_am_is_playing(BT_ULL_LE_AM_DL_MODE);
                            if (dl_playing && (!stream_ctx->client.dl.is_am_open)) {
                                ull_report("[ULL][LE][DEBUG] Inactive aircis, dl is playing but is_am_on not set!",0);
                                stream_ctx->client.dl.is_am_open = true;
                            }
                            ull_report("[ULL][LE]BT_ULL_EVENT_STREAMING_START_IND, ul_playing: %d, ul_streaming:%d, dl_playing: %d", 3, ul_playing, ul_streaming, dl_playing);
                            if (!ul_playing && !ul_streaming) {
                                bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_NORMAL);
                            }
                            /*Enable Downlink, exit DL dummy mode.*/
                            if (dl_playing) {
                                bt_ull_le_streaming_start_ind_t start_stream;
                                start_stream.stream_mode = BT_ULL_LE_STREAM_MODE_DOWNLINK;
                                bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_START_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t)); 
                                if (BT_ULL_LE_LINK_STATE_SETTING_DATA_PATH == link_info->curr_state) {
                                    bt_ull_le_srv_active_streaming();
                                }
                            } else {
                                bt_ull_le_srv_play_am(streaming, BT_ULL_LE_STREAM_MODE_DOWNLINK);
                            }							
                        } else {
                            /*Enable Downlink, exit DL dummy mode.*/
                            bt_ull_le_srv_play_am(streaming, BT_ULL_LE_STREAM_MODE_DOWNLINK);
                        }
                    } else if ((BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)
                            || (BT_ULL_STREAMING_INTERFACE_LINE_OUT == streaming->streaming_interface)
                            || (BT_ULL_STREAMING_INTERFACE_I2S_OUT == streaming->streaming_interface)) {
                        if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                            /**< means that client had recieved the same event. ignore it. */
                            break;
                        }                            
                        stream_ctx->streaming_port |= BT_ULL_LE_STREAM_PORT_MASK_MIC;
                        bt_ull_le_srv_play_am(streaming, BT_ULL_LE_STREAM_MODE_UPLINK);
                    }
                }
                break;
            }
            /*Client Received the Stop status from Server*/
            case BT_ULL_EVENT_STREAMING_STOP_IND: {
                bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)(req_data + sizeof(bt_ull_req_event_t));
                ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_STREAMING_STOP_IND, if_type: 0x%x, port:0x%x, current port mask: %x", 3, \
                    streaming->streaming_interface, streaming->port, stream_ctx->streaming_port);
                bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
                bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
                ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_STREAMING_STOP_IND, streaming_port: 0x%x", 1, stream_ctx->streaming_port);
                if (BT_ULL_ROLE_CLIENT == ctx->role) {
                    if ((BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface)
                            || (BT_ULL_STREAMING_INTERFACE_LINE_IN == streaming->streaming_interface)
                            || (BT_ULL_STREAMING_INTERFACE_I2S_IN == streaming->streaming_interface)) {
                        /*Disable Downlink.*/
                        if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                            //bug fix
                            if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                                ull_report("[ULL][LE] means that client had recieved the same stop event. ignore it.",0);
                                break;
                            }
                        }
                        stream_ctx->streaming_port &= ~stream_port;
                        if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                            /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: received DL stop ind, reduce ULL DL priority to LOW*/
                            bool dl_stream = bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK);
                            bool ul_stream = bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK);
                            bool ul_is_playing = bt_ull_le_am_is_playing(BT_ULL_LE_AM_UL_MODE);
                            ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_STREAMING_STOP_IND, %d, %d, %d", 3, \
                                dl_stream, ul_stream, ul_is_playing);
                            if (!dl_stream \
                                && !ul_stream \
                                && !ul_is_playing) {
                                //bt_ull_le_am_restart(BT_ULL_LE_AM_DL_MODE);//1106: before decrease priority, should restart DSP.
                                bt_ull_le_srv_stop_am(streaming);
                                bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_LOW);
                            }
                            //bt_ull_le_srv_stop_am(streaming);
                            /*if (!dl_stream) {
                                //notify APP DL Stop actrually
                                bt_ull_le_streaming_stop_ind_t stop_stream;
                                stop_stream.stream_mode = BT_ULL_LE_STREAM_MODE_DOWNLINK;
                                bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_STOP_IND, (void *)&stop_stream, sizeof(bt_ull_le_streaming_stop_ind_t));
                            }*/                            
                        } else {
                            bt_ull_le_srv_stop_am(streaming);
                        }
                    } else if ((BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)
                            || (BT_ULL_STREAMING_INTERFACE_LINE_OUT == streaming->streaming_interface)
                            || (BT_ULL_STREAMING_INTERFACE_I2S_OUT == streaming->streaming_interface)) {
                        stream_ctx->streaming_port &= ~BT_ULL_LE_STREAM_PORT_MASK_MIC;
                        if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                            stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_MIC;
                        }
                        /* ULL earbuds only have 1 earbud active uplink, but it stiil need to update the downlink's audio priority to support Uplink can match the EMP media priority rule */
                        if ((BT_ULL_EARBUDS_CLIENT == ctx->client_type) && !(link_info->ul_active)) {
                            stream_ctx->client.ul.is_am_open = false;

                            // notify app UL streaming stop for sync other earbuds that open UL realy;
                            bt_ull_le_streaming_start_ind_t start_stream;
                            start_stream.stream_mode = BT_ULL_LE_STREAM_MODE_UPLINK;
                            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_STREAMING_STOP_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t));

                            bt_ull_le_am_sync_sidetone_status(false);
                            if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                                /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: For earbuds, inactive ul side not close DL, only adjust priority by DL port streaming*/
                                if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                                    bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_LOW);
                                } else {
                                    bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_NORMAL);
                                }
                            } else {
                                if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {//DL is dummy mode
                                    if (BT_STATUS_SUCCESS == bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true)) {
    									if (BT_STATUS_SUCCESS == bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true)) {
    										stream_ctx->client.dl.is_dl_dummy_mode = false;
    										ull_report("[ULL][LE] Uplink Disable, then stop Downlink Dummy mode", 0);
    									}
                                    }
                                    break;
                                } else {
                                    bt_ull_le_am_update_dl_priority(false);//need to lower proiryty because of Uplink had been disabled
                                }
                            }
                        }

                        /* Normal case, disble mic if ull is playing */
                        //if (stream_ctx->client.ul.is_am_open) {
                        #ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
                            bt_ull_wireless_mic_stop_volume_monitor();
                        #endif
                            ull_report("[ULL][LE] check ul playing: %d.", 1, stream_ctx->client.ul.is_am_open);
                            bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
                            stream_ctx->client.dl.is_dl_dummy_mode = false;
                            if (!stream_ctx->client.ul.is_am_open) {
                                if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                                    if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                                        bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_NORMAL); //if dl port is in streaming, set priority to 2
                                    } else {
                                        bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_LOW); // if dl port not streaming, set priority to 0
                                    }
                                } else {
                                    bt_ull_le_am_update_dl_priority(false);
                                }
                            }
                        //}
                        ull_report("[ULL][LE] Client recieved disable mic, curr_stream_state:0x%x, is_dl_dummy_mode:No", 1, ctx->curr_stream_state);
                    }
                    if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                        /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: Deactive ISO both UL and DL stop streaming*/
                        if ((!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) && (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK))) {
                            //bt_ull_le_am_restart(BT_ULL_LE_AM_DL_MODE);//1106: before deactive streaming, should restart DSP.
                            bt_ull_le_srv_deactive_streaming();
                        }                        
                    } else {
                        if (!stream_ctx->streaming_port) {
                            bt_ull_le_srv_disable_streaming();
                        }
                    }
                }
                break;
            }
            case BT_ULL_EVENT_VOLUME_IND: {
                bt_ull_volume_ind_t *p_vol = (bt_ull_volume_ind_t *)(req_data + sizeof(bt_ull_req_event_t));
                ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_VOLUME_IND, if_type: 0x%x, port: 0x%x, vol_left:%d, vol_right:%d", 4,
                    p_vol->streaming.streaming_interface, p_vol->streaming.port, p_vol->vol.vol_left, p_vol->vol.vol_right);
                if (BT_ULL_ROLE_CLIENT == ctx->role) {
                    if ((BT_ULL_STREAMING_INTERFACE_MICROPHONE == p_vol->streaming.streaming_interface)
                        || (BT_ULL_STREAMING_INTERFACE_LINE_OUT == p_vol->streaming.streaming_interface)
                        || (BT_ULL_STREAMING_INTERFACE_I2S_OUT == p_vol->streaming.streaming_interface)) {
                        stream_ctx->client.ul.volume.vol_left = p_vol->vol.vol_left;
                        stream_ctx->client.ul.volume.vol_right = p_vol->vol.vol_right;
                        bt_ull_le_am_set_volume(BT_ULL_LE_AM_UL_MODE, p_vol->vol.vol_left, BT_ULL_AUDIO_CHANNEL_DUAL);
                        break;
                    }

                    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == p_vol->streaming.streaming_interface) {
                       if (0x00 == p_vol->streaming.port) {
                            stream_ctx->client.dl.volume.vol_left = p_vol->vol.vol_left;
                            stream_ctx->client.dl.volume.vol_right = p_vol->vol.vol_right;
                        } else if (0x01 == p_vol->streaming.port) {
                            stream_ctx->server_chat_volume.vol_left = p_vol->vol.vol_left;
                            stream_ctx->server_chat_volume.vol_right = p_vol->vol.vol_right;
                        }
                    } else if (BT_ULL_STREAMING_INTERFACE_LINE_IN == p_vol->streaming.streaming_interface) {
                        stream_ctx->server_linein_volume.vol_left = p_vol->vol.vol_left;
                        stream_ctx->server_linein_volume.vol_right = p_vol->vol.vol_right;
                    } else if (BT_ULL_STREAMING_INTERFACE_LINE_OUT == p_vol->streaming.streaming_interface) {
                        stream_ctx->server_lineout_volume.vol_left = p_vol->vol.vol_left;
                        stream_ctx->server_lineout_volume.vol_right = p_vol->vol.vol_right;
                    } else if (BT_ULL_STREAMING_INTERFACE_I2S_IN == p_vol->streaming.streaming_interface) {
                        stream_ctx->server_linei2s_volume.vol_left = p_vol->vol.vol_left;
                        stream_ctx->server_linei2s_volume.vol_right = p_vol->vol.vol_right;
                    }
                #if 0
                        //BTA-33021
                    if (0 == p_vol->vol.vol_left && 0 != p_vol->vol.vol_right) {
                        bt_ull_le_am_set_volume(BT_ULL_LE_AM_DL_MODE, p_vol->vol.vol_right, p_vol->channel);
                    } else if (0 != p_vol->vol.vol_left && 0 == p_vol->vol.vol_right) {
                        bt_ull_le_am_set_volume(BT_ULL_LE_AM_DL_MODE, p_vol->vol.vol_left, p_vol->channel);
                    } else {
                        bt_ull_le_am_set_volume(BT_ULL_LE_AM_DL_MODE, p_vol->vol.vol_left, p_vol->channel);
                    }
                #endif
                }
                break;
            }
            case BT_ULL_EVENT_VOLUME_MUTE: {
                bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)(req_data + sizeof(bt_ull_req_event_t));
                ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_VOLUME_MUTE, if_type: 0x%x, port:0x%x", 2, streaming->streaming_interface, streaming->port);
                if (BT_ULL_ROLE_SERVER == ctx->role) {
                    if (BT_ULL_MIC_CLIENT == ctx->client_type) {
                        /* Wirless Mic RX not mute */
                        /* Notify APP to update the UI */
                    } else {
                        /* dongle mute */
                        bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
                        if ((BT_ULL_GAMING_TRANSMITTER <= trans_type) && (BT_ULL_TRANSMITTER_MAX_NUM > trans_type)) {
                            stream_ctx->server.stream[trans_type].is_mute = true;
                            bt_ull_le_at_set_mute(trans_type, true);
                        }
                    }
                } else if (BT_ULL_ROLE_CLIENT == ctx->role) {
                    if ((BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)
                        || (BT_ULL_STREAMING_INTERFACE_LINE_OUT == streaming->streaming_interface)
                        || (BT_ULL_STREAMING_INTERFACE_I2S_OUT == streaming->streaming_interface)) {
                        stream_ctx->client.ul.is_mute = true;
                        bt_ull_le_am_set_mute(BT_ULL_LE_AM_UL_MODE, true);
                    } else {
                        stream_ctx->client.dl.is_mute = true;
                        bt_ull_le_am_set_mute(BT_ULL_LE_AM_DL_MODE, true);
                    }
                }
                break;
            }
            case BT_ULL_EVENT_VOLUME_UNMUTE: {
                bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)(req_data + sizeof(bt_ull_req_event_t));
                ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_VOLUME_UNMUTE, if_type: 0x%x, port:0x%x", 2, streaming->streaming_interface, streaming->port);
                if (BT_ULL_ROLE_SERVER == ctx->role) {
                    /* dongle un-mute */
                    //bt_ull_le_srv_handle_set_streaming_mute(streaming, false);
                    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
                    if ((BT_ULL_GAMING_TRANSMITTER <= trans_type) && (BT_ULL_TRANSMITTER_MAX_NUM > trans_type)) {
                        stream_ctx->server.stream[trans_type].is_mute = false;
                        bt_ull_le_at_set_mute(trans_type, false);
                    }
                } else if (BT_ULL_ROLE_CLIENT == ctx->role) {
                    if ((BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)
                        || (BT_ULL_STREAMING_INTERFACE_LINE_OUT == streaming->streaming_interface)
                        || (BT_ULL_STREAMING_INTERFACE_I2S_OUT == streaming->streaming_interface)) {
                        stream_ctx->client.ul.is_mute = false;
                        bt_ull_le_am_set_mute(BT_ULL_LE_AM_UL_MODE, false);
                    } else {
                        stream_ctx->client.dl.is_mute = false;
                        bt_ull_le_am_set_mute(BT_ULL_LE_AM_DL_MODE, false);
                    }
                }
                break;
            }
            /* server reply switch latency, client restart codec */
            case BT_ULL_EVENT_LATENCY_SWITCH_COMPLETE: {
                bt_ull_latency_t *latency = (bt_ull_latency_t *)(req_data + sizeof(bt_ull_req_event_t));
                if (latency) {
                    ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_LATENCY_SWITCH_COMPLETE, latency: %d", 1, latency->latency);
                    stream_ctx->dl_latency = latency->latency;
                    if (stream_ctx->dl_latency > BT_ULL_LE_SRV_LATENCY_SINGLE_LINK_MODE) {
                        stream_ctx->ul_latency = BT_ULL_LE_SRV_LATENCY_MULTI_LINK_CONNECTING_MODE;
                    } else {
                        stream_ctx->ul_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
                    }

                    if ((bt_ull_le_srv_is_connected()) && bt_ull_le_srv_is_any_streaming_started(ctx->role)) {
                        /* restart ull codec, if we are playing ull to swith latency */
                        if (stream_ctx->client.dl.is_am_open) {
                            bt_ull_le_am_restart(BT_ULL_LE_AM_DL_MODE);
                        }
                        if (stream_ctx->client.ul.is_am_open) {
                            bt_ull_le_am_restart(BT_ULL_LE_AM_UL_MODE);
                        }
                    }
                }
                break;
            }
            /* dongle receive headset Latency request*/
            case BT_ULL_EVENT_LATENCY_SWITCH: {
                bt_ull_latency_t *req_latency = (bt_ull_latency_t*)(req_data + sizeof(bt_ull_req_event_t));
                if (req_latency) {
                    bt_ull_le_srv_handle_set_streaming_latency(req_latency, handle);
                }
                break;
            }
            /* dongle receive headset stream allow/disallow request*/
            case BT_ULL_EVENT_ALLOW_PLAY_REPORT: {
                bt_ull_allow_play_report_t *report = (bt_ull_allow_play_report_t*)(req_data + sizeof(bt_ull_req_event_t));
                bt_ull_client_t ct = bt_ull_le_srv_get_client_type();
                if (report) {
                    ull_report("[ULL][LE] dongle rx recieved allow_play : 0x%x, current is_allow: 0x%x, ct: %x, link state: %x, reason: %d", 5, \
                        report->allow_play, stream_ctx->allow_play, ct, link_info->curr_state, report->reason);
                    if (BT_ULL_SPEAKER_CLIENT == ct) {
                        if (BT_ULL_ALLOW_PLAY_REASON_AUDIO == report->reason || BT_ULL_ALLOW_PLAY_REASON_FOTA == report->reason) {
                            if (link_info->allow_play != report->allow_play) {
                                link_info->allow_play = report->allow_play;
                                if (BT_ULL_PLAY_DISALLOW == link_info->allow_play) {
                                    if (BT_ULL_LE_LINK_STATE_READY < link_info->curr_state) {
                                        status = bt_ull_le_conn_srv_destroy_air_cis(link_info->conn_handle);
                                        ull_report("[ULL][LE] Disc CIS, conn_handle: 0x%x, status: 0x%x", 2, link_info->conn_handle, status);
                                    }                                  
                                //} else if (BT_ULL_PLAY_ALLOW == link_info->allow_play && BT_ULL_PLAY_ALLOW == stream_ctx->allow_play) {
                                } else if (BT_ULL_PLAY_ALLOW == link_info->allow_play) {
                                    if (bt_ull_le_srv_is_any_streaming_started(ctx->role)) {
                                        ull_report("[ULL][LE] Create CIS, recreate air cis handle 0x%x, state: %x", 2, \
                                            link_info->conn_handle, link_info->curr_state);
                                        if (BT_ULL_LE_LINK_STATE_READY == link_info->curr_state) {
                                            /* Create CIS */
                                            if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(link_info->conn_handle)) {
                                                link_info->curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                                            }
                                        }
                                    }
                                }
                            }
                        } else {
                            if (report->allow_play != stream_ctx->allow_play) {
                                stream_ctx->allow_play = report->allow_play;
                                if (BT_ULL_PLAY_DISALLOW == stream_ctx->allow_play) {
                                    /* stop audio transmitter */
                                    uint8_t i;
                                    for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
                                        if (stream_ctx->server.stream[i].is_transmitter_start) {
                                            if (BT_STATUS_SUCCESS != bt_ull_le_at_stop(i, true)) {
                                                ull_report_error("[ULL][LE] BT_ULL_EVENT_ALLOW_PLAY_REPORT-FOTA, stop transmitter %d fail", 1, i);
                                            }
                                        }
                                    }
                                } else {
                                    /* start audio transmitter */
                                    uint8_t i;
                                    for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
                                        bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(i);
                                        if (stream_ctx->streaming_port & stream_port) {
                                            if (BT_STATUS_SUCCESS != bt_ull_le_at_start(i, true)) {
                                                ull_report_error("[ULL][LE] BT_ULL_EVENT_ALLOW_PLAY_REPORT-FOTA, start transmitter %d fail", 1, i);
                                            }
                                        }
                                    }
                                }
                            }

                        }
                    } else {
                    link_info->allow_play = report->allow_play;
                    if (report->allow_play != stream_ctx->allow_play) {
                        stream_ctx->allow_play = report->allow_play;
                        if (BT_ULL_PLAY_DISALLOW == stream_ctx->allow_play) {
                            if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                                /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server]: Disconnect CIS if ULL sink can not get audio resource*/
                                bt_ull_le_srv_disconnect_aircis();                                
                            } else {
                                /* stop audio transmitter */
                                uint8_t i;
                                for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
                                    if (stream_ctx->server.stream[i].is_transmitter_start || bt_ull_le_at_is_prepare_start(i)) {
                                        if (BT_STATUS_SUCCESS != bt_ull_le_at_stop(i, true)) {
                                            ull_report_error("[ULL][LE] BT_ULL_EVENT_ALLOW_PLAY_REPORT, stop transmitter %d fail", 1, i);
                                        }
                                    }
                                }
                            }
                            /*Remove CIG.*/
                            if (ctx->is_cig_created) {
                                //bt_status_t status = bt_ull_le_conn_srv_remove_air_cig_params();
                                ull_report("[ULL][LE] BT_ULL_EVENT_ALLOW_PLAY_REPORT, remove CIG, status is 0x%x", 1, status);
                            }
                        } else {
                            if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                                /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Server]: Establish CIS if ULL sink can get audio source*/
                                if (!ctx->is_removing_cig_for_switch_latency) {
                                    if (!ctx->is_cig_created) {
                                        /*Set Air CIG parameters. */
                                        if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_set_air_cig_params(ctx->cis_num)) {
                                            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                                        } else {
                                            ull_report_error("[ULL][LE] bt_ull_le_srv_start_audio_transmitter_callback, set cig error", 0);
                                        }
                                    } else {
                                        if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER > ctx->curr_stream_state) {
                                            bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER);
                                        }
                                        uint8_t i;
                                        for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {
                                            if (BT_ULL_LE_LINK_STATE_READY == g_ull_le_link_info[i].curr_state) {
                                                /* Create CIS */
                                                if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(g_ull_le_link_info[i].conn_handle)) {
                                                    g_ull_le_link_info[i].curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                                                }
                                            } else if (BT_ULL_LE_LINK_STATE_STREAMING == g_ull_le_link_info[i].curr_state) {
                                                /**<Air CIS has been created done and open audio transmitter successfully. */
                                                if (BT_ULL_LE_SRV_STREAM_STATE_SET_CIG_PARAMETER == ctx->curr_stream_state)
                                                    bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_STREAMING);
                                            }
                                        }
                                    }
                                }
                                /* restart audio transmitter */
                                uint8_t i;
                                for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
                                    bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(i);
                                    if (stream_ctx->streaming_port & stream_port) {
                                        if (stream_ctx->server.stream[i].is_transmitter_start) {
                                            if (BT_STATUS_SUCCESS != bt_ull_le_at_restart(i)) {
                                                ull_report_error("[ULL][LE] BT_ULL_EVENT_ALLOW_PLAY_REPORT, restart transmitter %d fail", 1, i);
                                            }
                                        } else {
                                            if (BT_STATUS_SUCCESS != bt_ull_le_at_start(i, true)) {
                                                ull_report_error("[ULL][LE] BT_ULL_EVENT_ALLOW_PLAY_REPORT, start transmitter %d fail", 1, i);
                                            }
                                        }
                                    }
                                }       
                            } else {
                                if (bt_ull_le_srv_is_any_streaming_started(ctx->role)) {
                                    /*audio transmitter is already start, recreate air cis*/
                                    ull_report("[ULL][LE] BT_ULL_EVENT_ALLOW_PLAY_REPORT, audio transmitter is already start, recreate air cis handle 0x%x, state: %x", 2, \
                                        link_info->conn_handle, link_info->curr_state);
                                    if (BT_ULL_LE_LINK_STATE_READY == link_info->curr_state) {
                                        /* Create CIS */
                                        if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(link_info->conn_handle)) {
                                            link_info->curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                                        }
                                    }
                                } else {
                                    /* start audio transmitter */
                                    uint8_t i;
                                    for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
                                        bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(i);
                                        if ((stream_ctx->streaming_port & stream_port) && (NULL == bt_timer_ext_find(BT_ULL_LE_CONN_WAITING_TIMER_ID))) {
                                            if (BT_STATUS_SUCCESS != bt_ull_le_at_start(i, true)) {
                                                ull_report_error("[ULL][LE] BT_ULL_EVENT_ALLOW_PLAY_REPORT, start transmitter %d fail", 1, i);
                                            }
                                        }
                                    }
                                    bt_ull_le_srv_notify_client_restart_streaming(BT_ULL_LE_RESTART_STREAMING_REASON_ALLOW_PALY);
                            	}
                            }
                        }
                    } else {
                        if (bt_ull_le_srv_is_any_streaming_started(ctx->role)) {
                            ull_report("[ULL][LE] BT_ULL_EVENT_ALLOW_PLAY_REPORT, recreate air cis handle 0x%x, state: %x", 2, \
                                link_info->conn_handle, link_info->curr_state);
                            if (BT_ULL_LE_LINK_STATE_READY == link_info->curr_state) {
                                /* Create CIS */
                                if (BT_STATUS_SUCCESS == bt_ull_le_conn_srv_establish_air_cis(link_info->conn_handle)) {
                                    link_info->curr_state = BT_ULL_LE_LINK_STATE_CREATING_CIS;
                                }
                            }
                        }
                    }
                    }

                }
                break;
            }
            /* dongle receive headset volume operation */
            case BT_ULL_EVENT_VOLUME_ACTION: {
                bt_ull_volume_t *vol = (bt_ull_volume_t *)(req_data + sizeof(bt_ull_req_event_t));
                ull_report("[ULL][LE] dongle rx recieved VOLUME_REPORT, type:0x%x, volume action: 0x%x, volume_level: %d,", 3,
                    vol->streaming.streaming_interface, vol->action, vol->volume);
#if defined(AIR_USB_ENABLE) && defined(AIR_USB_AUDIO_ENABLE) && defined(AIR_USB_HID_ENABLE)
                if (BT_ULL_STREAMING_INTERFACE_SPEAKER == vol->streaming.streaming_interface) {
                    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
                        ull_report("[ULL][LE] usb_hid_control, current is XBOX mode, ignore the HID control event", 0);
                    } else {
                        if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) || (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT)) {
                            bt_status_t usb_hid_volume_control_result = BT_STATUS_FAIL;
                            if (BT_ULL_VOLUME_ACTION_SET_UP == vol->action) {
                                if (USB_HID_STATUS_OK == USB_HID_VolumeUp(vol->volume)) {
                                    usb_hid_volume_control_result = BT_STATUS_SUCCESS;
                                    ull_report("[ULL][LE] usb_hid_control, volume up success", 0);
                                }
    
                            } else if (BT_ULL_VOLUME_ACTION_SET_DOWN == vol->action) {
                                if (USB_HID_STATUS_OK == USB_HID_VolumeDown(vol->volume)) {
                                    usb_hid_volume_control_result = BT_STATUS_SUCCESS;
                                    ull_report("[ULL][LE] usb_hid_control, volume down success", 0);
                                }
                            }
                            if (BT_STATUS_FAIL == usb_hid_volume_control_result) {
                                ull_report_error("[ULL][LE] usb_hid_control fail, local adjust volume", 0);
                                if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
                                    vol->streaming.port = 0;
                                    bt_ull_le_srv_handle_set_volume(vol);
                                }
                                if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
                                    vol->streaming.port = 1;
                                    bt_ull_le_srv_handle_set_volume(vol);
                                }                            
                            }
                        } else if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_IN) 
                            || (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_IN)) {
                            ull_report_error("[ULL][LE] streaming port is not USB_IN type, local adjust volume", 0);
                            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_IN) {
                                vol->streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
                                vol->streaming.port = 0;
                                bt_ull_le_srv_handle_set_volume(vol);
                            } else if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_IN) {
                                vol->streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
                                vol->streaming.port = 0;
                                bt_ull_le_srv_handle_set_volume(vol);                                
                            } 
                        }
                    }
                } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == vol->streaming.streaming_interface 
                            ||BT_ULL_STREAMING_INTERFACE_LINE_OUT == vol->streaming.streaming_interface) {
                    if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                        vol->streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                        bt_ull_le_srv_handle_set_volume(vol);
                    }
                    if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT) {
                        vol->streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
                        bt_ull_le_srv_handle_set_volume(vol);
                    }

                    //bt_ull_le_srv_handle_set_volume(vol);
                }
#else
                if ((BT_ULL_STREAMING_INTERFACE_UNKNOWN < vol->streaming.streaming_interface) &&
                    (BT_ULL_STREAMING_INTERFACE_MAX_NUM > vol->streaming.streaming_interface)) {
                    bt_ull_transmitter_t trans = bt_ull_le_srv_get_transmitter_by_stream_interface(&vol->streaming);
                    bt_ull_le_at_set_volume(trans, vol->action, vol->channel, vol->volume);
                } else {
                    ull_assert(0 && "unknown transmitter type");
                    return;
                }
#endif
                break;
            }
            case BT_ULL_EVENT_MIX_RATIO_ACTION: {
                bt_ull_mix_ratio_t *mix_ratio = (bt_ull_mix_ratio_t *)(req_data + sizeof(bt_ull_req_event_t));
                if (mix_ratio) {
                    ull_report("[ULL][LE] Server rx recieved MIX_RATIO, streaming[0]: %d, streaming[1]: %d", 2,
                        mix_ratio->streamings[0].ratio, mix_ratio->streamings[1].ratio);
                    bt_ull_le_srv_handle_set_streaming_mix_ratio(mix_ratio);
                }
                break;
            }
            case BT_ULL_EVENT_SAMPLE_RATE_CHANGE: {
                bt_ull_le_srv_sample_rate_change_t *sample_rate = (bt_ull_le_srv_sample_rate_change_t *)(req_data + sizeof(bt_ull_req_event_t));
                if (sample_rate) {
                    ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_SAMPLE_RATE_CHANGE, sample_rate: %d", 1, sample_rate->sample_rate);
                    bt_ull_le_srv_client_stream_t *stream_info;
                    stream_info = (bt_ull_le_srv_client_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_CLIENT, sample_rate->transmitter);

                    if ((stream_info) && (stream_info->usb_sample_rate != sample_rate->sample_rate)) {
                        stream_info->usb_sample_rate = sample_rate->sample_rate;
                        if (stream_info->is_am_open) {
                            /* restart ull codec, if we are playing ull to swith latency */
                            if ((BT_ULL_GAMING_TRANSMITTER == sample_rate->transmitter)
                             || (BT_ULL_CHAT_TRANSMITTER == sample_rate->transmitter)
                             || (BT_ULL_LINE_IN_TRANSMITTER == sample_rate->transmitter)
                             || (BT_ULL_I2S_IN_TRANSMITTER == sample_rate->transmitter)) {
                                bt_ull_le_am_restart(BT_ULL_LE_AM_DL_MODE);
                            } else if ((BT_ULL_MIC_TRANSMITTER == sample_rate->transmitter)
                                    || (BT_ULL_LINE_OUT_TRANSMITTER == sample_rate->transmitter)
                                    || (BT_ULL_I2S_OUT_TRANSMITTER == sample_rate->transmitter)) {
                                bt_ull_le_am_restart(BT_ULL_LE_AM_UL_MODE);
                            }
                        }
                    }
                }
                break;
            }
            case BT_ULL_EVENT_USER_DATA: {
                ull_report("[ULL][LE] rx recieved BT_ULL_EVENT_USER_DATA", 0);
                uint8_t *p_rx = &data[1];
                bt_ull_user_data_t user_data_cb;
                bt_ull_le_srv_memcpy(user_data_cb.remote_address, link_info->addr, sizeof(bt_bd_addr_t));
                bt_ull_le_srv_memcpy(&(user_data_cb.user_data_length), p_rx, sizeof(user_data_cb.user_data_length));
                p_rx += sizeof(user_data_cb.user_data_length);
                user_data_cb.user_data = p_rx;
#if 1	// richard for ab1571d command processing
			if(data[3]==3)
			{
				ab1571d_data_processing(data[4], data[5]);
			}
#endif		
#if 1  // Alex b for audeara ab1571d command processing		
            if (data[3] == 0x0B)
            {
                // Data[4] is the frame
                uint16_t aua_packet_length = (((data[5] & 0xFF) << 8) + (data[6] & 0xFF));
                audeara_ab1571d_data_processing(data[4], &data[7], aua_packet_length);
            }
#endif
                bt_ull_le_srv_event_callback(BT_ULL_EVENT_USER_DATA_IND, &user_data_cb, sizeof(user_data_cb));
                break;
            }

            case BT_ULL_EVENT_USB_HID_CONTROL_ACTION: {
                bt_ull_usb_hid_control_t control_id = req_data[1];
                ull_report("[ULL][LE] Server rx recieved USB_HID_CONTROL, action: 0x%x", 1, control_id);
                bt_ull_le_srv_handle_usb_hid_control(control_id);
                break;
            }

            case BT_ULL_EVENT_SEND_CONFIGURATION: {
                bt_ull_le_srv_configuration_t *cfg = (bt_ull_le_srv_configuration_t *)(req_data + sizeof(bt_ull_req_event_t));
                ull_report("[ULL][LE] CONFIGURATION , audio_location: %d, cis_id: %d, aws: %d, aircis_inactive: %d", 4, cfg->audio_location, bt_ull_le_conn_srv_get_cis_id_by_location(cfg->audio_location), cfg->aws_connected, cfg->aircis_inactive_enable);
                if (BT_ULL_ROLE_CLIENT == ctx->role) {
                    bt_ull_le_srv_memcpy(&link_info->sink_cfg, cfg, sizeof(bt_ull_le_srv_configuration_t));
                    link_info->curr_state = BT_ULL_LE_LINK_STATE_SYNC_CODEC_INFO;
                } else if (BT_ULL_ROLE_SERVER == ctx->role) {
                    link_info->sink_cfg.audio_location = cfg->audio_location;
                    link_info->sink_cfg.aws_connected= cfg->aws_connected;
                    ctx->aircis_inactive_mode_enable = cfg->aircis_inactive_enable;
                    if (BT_ULL_HEADSET_CLIENT == ctx->client_type) {
                        link_info->sink_cfg.cis_id = BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1;
                        link_info->ul_active = true;
                    } else if (BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
                        link_info->sink_cfg.cis_id = bt_ull_le_conn_srv_get_cis_id_by_location(cfg->audio_location);
                        link_info->ul_active = true;
                        uint8_t i = 0;
                        /*earbuds only have one LE link with uplink activated*/
                        for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
                            if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                                link_info->conn_handle != g_ull_le_link_info[i].conn_handle && \
                                g_ull_le_link_info[i].ul_active) { /*LE linnk connected && not this le link && ul has activated*/
                                link_info->ul_active = false;
                                break;
                            }
                        }
                    } else if (BT_ULL_SPEAKER_CLIENT == ctx->client_type) {
                        link_info->sink_cfg.cis_id = bt_ull_le_conn_srv_get_avaliable_cis_idx();
                        link_info->ul_active = false;
                    }
                    uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_configuration_t);
                    uint8_t *send_cfg = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                    bt_ull_le_srv_memcpy(&link_info->peer_capability_msk, &cfg->capability_msk, sizeof(bt_ull_le_srv_capability_t));
                    if (NULL != send_cfg) {
                        send_cfg[0] = BT_ULL_EVENT_SEND_CONFIGURATION;
                        bt_ull_le_srv_memcpy((bt_ull_le_srv_configuration_t *)(send_cfg + sizeof(bt_ull_req_event_t)), &link_info->sink_cfg, sizeof(bt_ull_le_srv_configuration_t));
                        status = bt_ull_le_srv_send_data(handle, (uint8_t*)send_cfg, len);
                        bt_ull_le_srv_memory_free(send_cfg);
                        if (!status) {
                             link_info->curr_state = BT_ULL_LE_LINK_STATE_CONFIGURATION;
                        }
                    }
                    if (BT_STATUS_SUCCESS == status) {
                        link_info->curr_state = BT_ULL_LE_LINK_STATE_SYNC_CODEC_INFO;
                        /* exchange codec info */
                        if ((BT_ULL_LE_LINK_STATE_SYNC_CODEC_INFO == link_info->curr_state) &&
                            (BT_STATUS_SUCCESS == (status = bt_ull_le_srv_handle_sync_codec(handle)))) {
                            ull_report("[ULL][LE] Server SYNC CODEC to client success!!", 0);
                        } else {
                            ull_report_error("[ULL][LE] Server send Sync info fail, role is: %d", 1, ctx->role);
                            link_info->curr_state = BT_ULL_LE_LINK_STATE_IDLE;
                            bt_ull_le_srv_handle_pairing_event(handle, BT_STATUS_FAIL, ctx->client_type);
                        }
                    }
                }
                break;
            }

            case BT_ULL_EVENT_ACTIVATE_UPLINK_IND : {
                bt_ull_le_srv_activate_ul_t *activate = (bt_ull_le_srv_activate_ul_t *)(req_data + sizeof(bt_ull_req_event_t));
                bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
                ull_report("[ULL][LE] Server rx recieved Active UL!, %d, %d, %d, %x, %d", 5,
                    ctx->role, link_info->ul_active, activate->enable, stream_ctx->streaming_port, stream_ctx->client.ul.is_am_open);
                if (BT_ULL_ROLE_CLIENT == ctx->role && BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
                    link_info->ul_active = activate->enable;
                    if ((activate->enable)
                    && (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC)
                    && (!(stream_ctx->client.ul.is_am_open))) {
                        bt_ull_streaming_t stream;
                        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                        stream.port = 0;
                        bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
                    }
                }
                break;
            }
            case BT_ULL_EVENT_RESTART_STREAMING_IND: {
                bt_ull_le_srv_notify_restart_streaming_t *data = \
                    (bt_ull_le_srv_notify_restart_streaming_t *)(req_data + sizeof(bt_ull_req_event_t));
                ull_report("[ULL][LE] BT_ULL_EVENT_RESTART_STREAMING_IND, reason: %d, mode: %x", 2, data->reason, data->mode_mask);
                bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
                if (BT_ULL_ROLE_CLIENT == ctx->role) {
                    if (bt_ull_le_srv_is_connected()) {
                        /* Enable DSP if dongle is playing */
                        if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                            bt_ull_streaming_t stream;
                            stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                            stream.port = 0;
                            bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_DOWNLINK);
                            if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state) {
                                bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
                            }
                        }
                        
                        if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
                            bt_ull_streaming_t stream;
                            stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                            stream.port = 0;
                            bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
                        }

                        /* restart ull codec, if we are playing ull to swith latency */
/*
                        if ((data->mode_mask & (1 << BT_ULL_LE_STREAM_MODE_UPLINK)) && \
                            stream_ctx->client.ul.is_am_open) {
                            ctx->restart_streaming_mask |= (1 << BT_ULL_LE_STREAM_MODE_UPLINK);
                            bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
                            //bt_ull_le_am_restart(BT_ULL_LE_AM_UL_MODE);
                        }
                        if ((data->mode_mask & (1 << BT_ULL_LE_STREAM_MODE_DOWNLINK)) && \
                            (stream_ctx->client.dl.is_am_open)) {
                            ctx->restart_streaming_mask |= (1 << BT_ULL_LE_STREAM_MODE_DOWNLINK);
                            bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true);
                            //bt_ull_le_am_restart(BT_ULL_LE_AM_DL_MODE);
                        }
*/
                    }
                }
                break;
            }
            case BT_ULL_EVENT_CALL_STATE: {
                bt_ull_le_srv_call_status_t call_state = req_data[1];
                void *call_data = req_data + sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_call_status_t);
                bt_ull_le_call_srv_client_handle_call_state(call_state, call_data);
                break;
            }
            case BT_ULL_EVENT_CALL_ACTION: {
                bt_ull_le_srv_call_action_t call_action = req_data[1];
                void *call_data = req_data + sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_call_action_t);
                bt_ull_le_call_srv_client_handle_call_action(call_action, call_data);
                break;
            }
            case BT_ULL_EVENT_AUDIO_QUALITY_CHANGE_IND: {
                bt_ull_le_srv_audio_quality_t *aud_qual = (bt_ull_le_srv_audio_quality_t *)(req_data + sizeof(bt_ull_req_event_t));
                bt_ull_le_srv_change_audio_quality_handle(handle, *aud_qual);
                break;
            }
            case BT_ULL_EVENT_SWITCH_UPLINK_IND: {
                bt_ull_le_client_switch_ul_ind_t *switch_ul_ind = (bt_ull_le_client_switch_ul_ind_t*)(req_data + sizeof(bt_ull_req_event_t));
                ull_report("[ULL][LE] Server rx recieved client enter smart charger, switch_ul: 0x%x", 1, switch_ul_ind->is_need_switch_ul);
                bt_ull_le_srv_switch_uplink(handle, switch_ul_ind->is_need_switch_ul);
                break;
            }
            case BT_ULL_EVENT_SCENARIO_IND: {
                bt_ull_le_srv_client_preferred_codec_param *codec_param = (bt_ull_le_srv_client_preferred_codec_param *)(req_data + sizeof(bt_ull_req_event_t));
                if(BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS == codec_param->codec_type) {
                    ull_report("[ULL][LE] switch ull scenario_type ullv3",0);
                }
#ifdef AIR_AUDIO_VEND_CODEC_ENABLE
                else if(BT_ULL_LE_CODEC_DL_ULD_UL_OPUS == codec_param->codec_type){
                    ull_report("[ULL][LE] switch ull scenario_type ullv3_o",0);
                }
#endif
                else if (BT_ULL_LE_CODEC_LC3PLUS == codec_param->codec_type){
                    ull_report("[ULL][LE] switch ull scenario_type ullv2",0);
                } 
#ifdef AIR_AUDIO_VEND_CODEC_ENABLE
                else if (BT_ULL_LE_CODEC_OPUS == codec_param->codec_type) {
                    ull_report("[ULL][LE] switch ull scenario_type ullv2",0);
                }
#endif
                else {
                    ull_report_error("[ULL][LE] unsupport ull codec_type %d!", 1, codec_param->codec_type);
                    return;
                }

                if (BT_ULL_ROLE_SERVER == ctx->role) {
                    ull_report("[ULL][LE] BT_ULL_EVENT_SCENARIO_IND, codec_type: %d, dl_samplerate: %d, ul_samplerate: %d", 3, codec_param->codec_type, codec_param->dl_samplerate, codec_param->ul_samplerate);
#if (defined AIR_ULL_AUDIO_V2_DONGLE_ENABLE) && (defined AIR_SILENCE_DETECTION_ENABLE)
                    bt_ull_le_at_stop_silence_detection();
#endif
                    stream_ctx->codec_type = codec_param->codec_type;
                    bt_ull_le_srv_memcpy(&(ctx->client_preferred_codec_param), codec_param, sizeof(bt_ull_le_srv_client_preferred_codec_param));
                    bt_ull_le_srv_set_codec_param_by_sample_rate(BT_ULL_ROLE_SERVER, ctx->client_preferred_codec_param.dl_samplerate, ctx->client_preferred_codec_param.ul_samplerate);
                    /*Remove CIG Param.*/
                    if (ctx->is_cig_created && !ctx->is_removing_cig_for_change_aud_codec) {
                        bt_status_t status = bt_ull_le_conn_srv_remove_air_cig_params();
                        if (BT_STATUS_SUCCESS == status || BT_STATUS_PENDING == status) {
                            ctx->is_removing_cig_for_change_aud_codec = true;
                        } else {
                            ull_report_error("[ULL][LE] bt_ull_le_srv_change_ull_scenario_handle, change aud codec", 0);
                        }
                        ull_report("[ULL][LE] SWITCH_ULL_SCENARIO, remove CIG, status is 0x%x", 1, status);
                    } else {
                         uint8_t i;
                        /* init controller media data share buffer info */
                        status = bt_ull_le_srv_init_share_info(ctx->client_type);
                        if ((BT_STATUS_SUCCESS == status) && (BT_STATUS_SUCCESS == bt_ull_le_srv_set_avm_share_buffer(ctx->role, ctx->client_type, ctx->cis_num))) {
                            ctx->is_share_buff_set = true;
                            for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
                            if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) {
                               bt_ull_le_conn_srv_set_air_cig_params_table(g_ull_le_link_info[i].conn_handle, ctx->cis_num, stream_ctx->codec_type);
                              }
                            }
                        } else {
                            ull_report_error("[ULL][LE] bt_ull_le_srv_handle_codec_sync_event set AVM buffer error!!", 0);
                        }
                    }
                }
                break;
            }
            case BT_ULL_EVENT_UPDATE_VOLUME_IND: {
                bt_ull_original_duel_volume_t *volume = (bt_ull_original_duel_volume_t*)(req_data + sizeof(bt_ull_req_event_t));
                ull_report("[ULL][LE] Client rx recieved volume update, L:%d, R:%d", 2, volume->vol_left, volume->vol_right);
                bt_ull_le_am_sync_volume(volume);
                break;
            }
            default: {
                ull_report_error("[ULL][LE] rx recieved continue data !", 0);
                break;
            }
         }
    } else {
        ull_report_error("[ULL][LE] rx recieved data error!", 0);
    }
}

static uint8_t bt_ull_le_srv_get_idx_by_handle(bt_handle_t acl_handle)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    uint8_t i = 0;
    uint8_t max_link_count = (BT_ULL_ROLE_SERVER == ctx->role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;
    while (i < max_link_count) {
        if (acl_handle == g_ull_le_link_info[i].conn_handle && BT_HANDLE_INVALID != acl_handle) {
            return i;
        }
        i++;
    }
    ull_report_error("[ULL][LE] bt_ull_le_srv_get_idx_by_handle fail, conn handle:0x%x", 1, acl_handle);
    return BT_ULL_LE_SRV_INVALID_VALUE;

}

bt_status_t bt_ull_le_srv_change_audio_quality_handle(bt_handle_t acl_handle, bt_ull_le_srv_audio_quality_t audio_quality)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_srv_aud_quality_info_t *aud_quality_info = bt_ull_le_srv_get_aud_quality_info();
    bool srv_connected = false;
    ull_report("[ULL][LE] bt_ull_le_srv_change_audio_quality_handle, handle: %x, chg: %d, cur: %d, aud_qua_flg: %d, ct: %d, expect: %d, codec: %d, sc: %d", 8, \
        acl_handle,
        audio_quality,
        stream_ctx->audio_quality,
        ctx->is_removing_cig_for_change_aud_quality,
        ctx->client_type,
        aud_quality_info->expect_aud_quality,
        stream_ctx->codec_type,
        ctx->client_preferred_scenario_type
        );
    if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID == audio_quality \
        || BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER < audio_quality \
        || BT_ULL_LE_CODEC_LC3PLUS != stream_ctx->codec_type) {
        return BT_STATUS_FAIL;
    }
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if (BT_ULL_LE_SCENARIO_ULLV2_0 != ctx->client_preferred_scenario_type) {
            return BT_STATUS_FAIL;
        }
        if (BT_HANDLE_INVALID == acl_handle) {
            if (audio_quality == aud_quality_info->expect_aud_quality) {
                return BT_STATUS_SUCCESS;
            }
            uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM;
            while (0 != i) {
                i--;
                if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                    (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                    if ((BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY == audio_quality && BT_ULL_LE_SRV_IS_SUPPORT(i, BT_ULL_LE_SRV_FEATURE_MHDT_4M_PHY)) \
                        || (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION == audio_quality && BT_ULL_LE_SRV_IS_SUPPORT(i, BT_ULL_LE_SRV_FEATURE_MHDT_8M_PHY)) \
                        || BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == audio_quality \
                        || BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_A2DP == audio_quality \
                        || BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_HFP == audio_quality \
                        || BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == audio_quality
                        ) {
                        if (BT_ULL_LE_LINK_STATE_SETTING_DATA_PATH <= g_ull_le_link_info[i].curr_state) {
                            srv_connected = true;
                        }
                        bt_ull_le_srv_send_audio_quality_by_handle(g_ull_le_link_info[i].conn_handle, audio_quality);
                    }
                }
            }
            aud_quality_info->expect_aud_quality = audio_quality;
            if (!srv_connected) {
                bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_CLIENT, audio_quality);
            }
        } else {
            bool dl_stream = bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK);
            bool dl_play = bt_ull_le_am_is_playing(BT_ULL_LE_AM_DL_MODE);
            bool ul_stream = bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK);
            bool ul_play = bt_ull_le_am_is_playing(BT_ULL_LE_AM_UL_MODE);
            ull_report("[ULL][LE] bt_ull_le_srv_change_audio_quality_handle, flg: %d, dl_stream: %d, dl paly: %d, ul stream: %d, ul play: %d", 5, \
                g_ull_le_wait_audio_quality, dl_stream, dl_play, ul_stream, ul_play);
            bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_CLIENT, audio_quality);
            if (g_ull_le_wait_audio_quality) {
                g_ull_le_wait_audio_quality = false;
                /* Enable DSP if dongle is playing */
                if (dl_stream && !dl_play) {
                    stream_ctx->client.dl.is_dl_dummy_mode = false;
                    bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
                    if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
                    }
                }
                if (ul_stream && !ul_play) {
                    bt_ull_streaming_t stream;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    stream.port = 0;
                    bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
                }
            }
        }
        //TODO: run configure audio manager
    } else if (BT_ULL_ROLE_SERVER == ctx->role) {
        uint8_t idx = bt_ull_le_srv_get_idx_by_handle(acl_handle);
        if (BT_ULL_LE_SRV_INVALID_VALUE == idx) {
            return status;
        }
        uint8_t state = bt_ull_le_srv_get_link_state_by_index(idx);
        bt_ull_le_srv_aud_quality_info_t *aud_quality_info = bt_ull_le_srv_get_aud_quality_info();
        ull_report("[ULL][LE] bt_ull_le_srv_change_audio_quality_handle, state: %d, idx: %d", 2, state, idx);
        if (BT_ULL_LE_LINK_STATE_READY > state) {
            return status;
        }
        if (g_ull_le_link_info[idx].aud_quality == audio_quality) {
            return BT_STATUS_SUCCESS;
        }
        uint8_t i = 0;
        bt_ull_le_srv_audio_quality_t other_link_aud_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID;
        bt_ull_le_srv_audio_quality_t change_aud_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID;
        g_ull_le_link_info[idx].aud_quality = audio_quality;
        if (BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
            for (i = 0;i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
                if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                    acl_handle != g_ull_le_link_info[i].conn_handle && \
                    BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                    other_link_aud_quality = g_ull_le_link_info[i].aud_quality;
                    break;
                }
            }
        }
        if (BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == other_link_aud_quality || BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID == other_link_aud_quality) {
            change_aud_quality = audio_quality;
        } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == audio_quality) {
            change_aud_quality = other_link_aud_quality;
        } else {
            change_aud_quality = ((BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID != other_link_aud_quality) && (audio_quality >= other_link_aud_quality)) ? other_link_aud_quality : audio_quality;
        }
        ull_report("[ULL][LE] bt_ull_le_srv_change_audio_quality_handle[server], final_chg: %d, curr: %d, other: %d, cs: %d, state: %d", 5, \
            change_aud_quality, stream_ctx->audio_quality, other_link_aud_quality, ctx->cs_size, ctx->curr_stream_state);
        aud_quality_info->expect_aud_quality = change_aud_quality;
        if (change_aud_quality == stream_ctx->audio_quality) {
            return BT_STATUS_SUCCESS;
        }
        if (((BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY == change_aud_quality && BT_ULL_LE_SRV_IS_SUPPORT(idx, BT_ULL_LE_SRV_FEATURE_MHDT_4M_PHY)) \
            || (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION == change_aud_quality && BT_ULL_LE_SRV_IS_SUPPORT(idx, BT_ULL_LE_SRV_FEATURE_MHDT_8M_PHY)) \
            || BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == change_aud_quality \
            || BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_A2DP == change_aud_quality \
            || BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_HFP == change_aud_quality) \
            && BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER != stream_ctx->audio_quality) {
            //For Auto test script check log
            if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == change_aud_quality) {
                ull_report("[ULL][LE] ULL2.0 Default Quality Enable.", 0);
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY == change_aud_quality) {
                ull_report("[ULL][LE] ULL2.1 High-Quality Enable.", 0);
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION == change_aud_quality) {
                ull_report("[ULL][LE] ULL2.1 High-Resolution Enable.", 0);
            }
            status = bt_ull_le_conn_srv_change_audio_quality(change_aud_quality);
            if (!status) {
                stream_ctx->audio_quality = change_aud_quality;
                bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_SERVER, change_aud_quality);
            }
        //TODO: run configure audio transmmiter
        } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == change_aud_quality \
            || BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == change_aud_quality) {
            //status = bt_ull_le_conn_srv_change_audio_quality(change_aud_quality);
            bt_ull_le_conn_srv_set_audio_quality(change_aud_quality);
            stream_ctx->audio_quality = change_aud_quality;
            bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_SERVER, change_aud_quality);
            if (BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == change_aud_quality) {
                ull_report("[ULL][LE] ULL2.0 Low Power Mode Enable.", 0);
            } else if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT == change_aud_quality) {
                ull_report("[ULL][LE] ULL2.0 Default Quality Enable.", 0);
            }
            if (ctx->is_cig_created && !ctx->is_removing_cig_for_change_aud_quality) {
                status = bt_ull_le_conn_srv_remove_air_cig_params();
                if (BT_STATUS_SUCCESS == status || BT_STATUS_PENDING == status) {
                    ctx->is_removing_cig_for_change_aud_quality = true;
                } else {
                    ull_report_error("[ULL][LE] bt_ull_le_srv_change_audio_quality_handle, change aud quality", 0);
                }
            } else {
                for (i = 0;i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
                    if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                        BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                        uint8_t temp_state = g_ull_le_link_info[i].is_cig_table_set;
                        g_ull_le_link_info[i].is_cig_table_set = false;
                        status = bt_ull_le_conn_srv_set_air_cig_params_table(g_ull_le_link_info[i].conn_handle, ctx->cis_num, stream_ctx->codec_type);
                        if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != status) {
                            ull_report_error("[ULL][LE] Enable low power mode fail due to set cig table", 0);
                            g_ull_le_link_info[i].is_cig_table_set = temp_state;
                            break;
                        } else {
                            ctx->is_removing_cig_for_change_aud_quality = true;
                        }
                    }
                }
            }
            bt_ull_le_srv_notify_client_auido_quality(change_aud_quality);
        }
    }
    return status;
}
static bt_status_t bt_ull_le_srv_send_audio_quality_by_handle(bt_handle_t handle, bt_ull_le_srv_audio_quality_t aud_quality)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (BT_HANDLE_INVALID != handle) {
        bt_ull_le_srv_link_info_t *link = bt_ull_le_srv_get_link_info(handle);
        if (link) {
            if (link->curr_state >= BT_ULL_LE_LINK_STATE_READY) {
                uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_audio_quality_t);
                uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                if (NULL != request) {
                    request[0] = BT_ULL_EVENT_AUDIO_QUALITY_CHANGE_IND;
                    bt_ull_le_srv_audio_quality_t *quality = (bt_ull_le_srv_audio_quality_t *)(request + sizeof(bt_ull_req_event_t));
                    *quality = aud_quality;
                    status = bt_ull_le_srv_send_data(handle, (uint8_t*)request, len);
                    bt_ull_le_srv_memory_free(request);
                }
            }
        }
    }
    return status;
}
static bt_status_t bt_ull_le_srv_notify_client_auido_quality(bt_ull_le_srv_audio_quality_t aud_quality)
{
    uint8_t i = 0;
    bt_status_t status = BT_STATUS_SUCCESS;
    ull_report("[ULL][LE] bt_ull_le_srv_notify_client_auido_quality: %d", 1, aud_quality);
    if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID != aud_quality) {
        for (i = 0;i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
            if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && \
                BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                status = bt_ull_le_srv_send_audio_quality_by_handle(g_ull_le_link_info[i].conn_handle, aud_quality);
            }
        }
    }
    return status;
}
bt_status_t bt_ull_le_srv_change_ull_scenario_handle(bt_ull_le_scenario_t scenario_type)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

#ifdef AIR_ULL_AUDIO_V3_ENABLE
    bt_ull_le_srv_set_scenario_type(scenario_type);
#else
    bt_ull_le_srv_set_scenario_type(BT_ULL_LE_SCENARIO_ULLV2_0);
    return status;
#endif

    if(BT_ULL_LE_SCENARIO_ULLV3_0 == g_ull_scenario_type) {
        ull_report("[ULL][LE] switch ull scenario_type ullv3",0);
        bt_ull_le_srv_disable_adaptive_bitrate_mode();
    } else if(BT_ULL_LE_SCENARIO_ULLV2_0 == g_ull_scenario_type) {
        bt_ull_le_srv_enable_adaptive_bitrate_mode(&ctx->adaptive_bitrate_param);
      ull_report("[ULL][LE] switch ull scenario_type ullv2",0);
    } else {
      ull_report_error("[ULL][LE] Unsupport ull scenario_type ullv2",0);
    }

    if(ctx->client_preferred_scenario_type == scenario_type) {
      ull_report("[ULL][LE] codec unchanged,do nothing!", 0);
      return status;
    }
   if(BT_ULL_ROLE_CLIENT == ctx->role) {
        ctx->client_preferred_scenario_type = scenario_type;
#if 1 //use common function
        stream_ctx->codec_type = bt_ull_le_srv_get_client_preffered_codec(scenario_type);
        ctx->client_preferred_codec_param.codec_type = stream_ctx->codec_type;
#else
        if (BT_ULL_LE_SCENARIO_ULLV2_0 == scenario_type) {
          stream_ctx->codec_type = BT_ULL_LE_CODEC_LC3PLUS;
          ctx->client_preferred_codec_param.codec_type = BT_ULL_LE_CODEC_LC3PLUS;
        } else if (BT_ULL_LE_SCENARIO_ULLV3_0 == scenario_type) {
          stream_ctx->codec_type = BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS;
          ctx->client_preferred_codec_param.codec_type = BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS;
        }
#endif
        ctx->client_preferred_codec_param.dl_samplerate = bt_ull_le_srv_get_client_preffered_dl_codec_samplerate(scenario_type);
        ctx->client_preferred_codec_param.ul_samplerate = bt_ull_le_srv_get_client_preffered_ul_codec_samplerate(scenario_type);
        bt_ull_le_srv_set_codec_param_by_sample_rate(BT_ULL_ROLE_CLIENT, ctx->client_preferred_codec_param.dl_samplerate, ctx->client_preferred_codec_param.ul_samplerate);
        if (bt_ull_le_srv_is_connected()) {
             /* sync change codec operation to service */
             uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM;
             while (0 != i) {
                i--;
                if(BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) {
                 ull_report("[ULL][LE] Send BT_ULL_EVENT_SCENARIO_IND", 0);
                 uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_client_preferred_codec_param);
                 uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                 if (NULL != request) {
                     request[0] = BT_ULL_EVENT_SCENARIO_IND;
                     bt_ull_le_srv_client_preferred_codec_param *prefer_param = (bt_ull_le_srv_client_preferred_codec_param *)(request + sizeof(bt_ull_req_event_t));
                     prefer_param->codec_type = ctx->client_preferred_codec_param.codec_type;
                     prefer_param->dl_samplerate = ctx->client_preferred_codec_param.dl_samplerate;
                     prefer_param->ul_samplerate = ctx->client_preferred_codec_param.ul_samplerate;
                     bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                     bt_ull_le_srv_memory_free(request);
                   }
                }
            }
       }
   }
   return status;
}

bool bt_ull_le_srv_check_ul_activate_state_by_handle(bt_handle_t acl_handle)
{
    bt_ull_le_srv_link_info_t *link_info = bt_ull_le_srv_get_link_info(acl_handle);
    if ((!link_info) || (BT_HANDLE_INVALID == link_info->conn_handle)) {
        return false;
    }
    return link_info->ul_active;
}

static void bt_ull_le_srv_critial_data_timeout_callback(uint32_t timer_id, uint32_t data)
{
    ull_report("[ULL][LE][AVM] bt_ull_le_srv_critial_data_timeout_callback, timer_id: 0x%x, data:0x%x", 2, timer_id, data);
    if (BT_ULL_CRITICAL_RX_IND_TIMER_ID == timer_id) {
        uint8_t* data_ind = (uint8_t*)data;
        bt_ull_rx_critical_user_data_t rx_data = {0};
        rx_data.user_data_length = data_ind[0];
        rx_data.user_data = &data_ind[1];
        bt_ull_le_srv_event_callback(BT_ULL_EVENT_RX_CRITICAL_USER_DATA_IND, &rx_data, sizeof(rx_data));
        bt_ull_le_srv_memory_free((void*)data);
    } else if (BT_ULL_CRITICAL_TX_RESULT_TIMER_ID == timer_id) {
        bt_ull_tx_critical_data_status_t tx_result;
        if (0 == data) {
            tx_result = BT_ULL_TX_CRITICAL_DATA_SUCCESS;
        } else if (1 == data) {
            tx_result = BT_ULL_TX_CRITICAL_DATA_TIMEOUT;
        } else {
            tx_result = BT_ULL_TX_CRITICAL_DATA_ABANDON;
        }
        if (tx_result > BT_ULL_TX_CRITICAL_DATA_SUCCESS) {
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_TX_CRITICAL_USER_DATA_RESULT, &tx_result, sizeof(tx_result));
        }
    }
}

static void bt_ull_le_srv_critial_data_controller_cb(bt_avm_critial_data_event_t event, void* data)
{
    uint32_t timer_data = 0;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();

    if (BT_AVM_CIRTICAL_DATA_RX_IND == event) {
        bt_avm_critial_data_t *rx_ind = (bt_avm_critial_data_t*)data;
        //ull_report("[ULL][AVM] bt_ull_critial_data rx ind, seq: 0x%x, len:0x%x, cur_seq:0x%x", 3, rx_ind->seq, rx_ind->length, ctx->critical_data_rx_seq);
        if (ctx->critical_data_rx_seq != rx_ind->seq) {  /* filter duplicate packet */
            uint16_t total_len = rx_ind->length+ sizeof(rx_ind->length);
            uint8_t *data_ind = (uint8_t*)bt_ull_le_srv_memory_alloc(total_len);
            if (NULL != data_ind) {
                data_ind[0] = rx_ind->length;
                bt_ull_le_srv_memcpy(&data_ind[1], rx_ind->data, rx_ind->length);
                timer_data = (uint32_t)data_ind;
                bt_timer_ext_status_t status = bt_timer_ext_start(BT_ULL_CRITICAL_RX_IND_TIMER_ID, timer_data, 1, bt_ull_le_srv_critial_data_timeout_callback);
                if (BT_TIMER_EXT_STATUS_SUCCESS != status) {
                    ull_report_error("[ULL][LE] bt_ull_le_srv_critial_data_controller_cb rx ind, start timer fail !!!", 0);
                    bt_ull_le_srv_memory_free(data_ind);
                }
            }
            /* for debug log */
            uint8_t temp_seq = ctx->critical_data_rx_seq;
            if (0xFF == temp_seq) {
                temp_seq = 0x01;
            } else {
                temp_seq++;
            }
            if (temp_seq != rx_ind->seq) {
                ull_report_error("[ULL][LE][WARNING] bt_ull_le_srv_critial_data_controller_cb rx lost!!!, seq: 0x%x, cur_seq:0x%x", 2, rx_ind->seq, ctx->critical_data_rx_seq);
            }
        } else {
            ull_report_error("[ULL][LE][WARNING] bt_ull_le_srv_critial_data_controller_cb rx same data, seq: 0x%x, len:0x%x, cur_seq:0x%x", 3, rx_ind->seq, rx_ind->length, ctx->critical_data_rx_seq);
        }
        ctx->critical_data_rx_seq = rx_ind->seq;
#if 0
        if (rx_ind->seq == 1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &g_gpt_clock_start);
        } else if (rx_ind->seq == g_max_frame_seq && g_max_frame_seq != 0) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &g_gpt_clock_end);
            uint32_t rx_ms = (g_gpt_clock_end - g_gpt_clock_start) * 1000 / 32768;
            ull_report("[ULL][LE] test ull 2.0 tx throughput time, time: %d", 1, rx_ms);
        }
        ull_report("[ULL][LE] test ull 2.0 tx throughput seq, seq: %d", 1, rx_ind->seq);
#endif
    } else if (BT_AVM_CIRTICAL_DATA_TX_RESULT == event) {
        bt_avm_critial_tx_result_t *result = (bt_avm_critial_tx_result_t*)data;
        timer_data = result->status;
        if (0 != result->status) {
            ull_report_error("[ULL][LE][AVM] bt_ull_le_srv_critial_data_controller_cb tx result, status: 0x%x, seq:0x%x", 2, result->status, result->seq);
        }
        bt_timer_ext_start(BT_ULL_CRITICAL_TX_RESULT_TIMER_ID, timer_data, 1, bt_ull_le_srv_critial_data_timeout_callback);
    }
}

static bt_status_t bt_ull_le_srv_critial_data_init(uint8_t max_len)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    if (0 == ctx->critical_data_max_len) {
        if (BT_STATUS_SUCCESS != bt_avm_critical_data_init(max_len, (void*)bt_ull_le_srv_critial_data_controller_cb)) {
            ret = BT_STATUS_FAIL;
        } else {
            ctx->critical_data_max_len = max_len;
            ctx->critical_data_tx_seq = 0x01;  /* valid seq: 1 ~ 255 */
            ctx->critical_data_rx_seq = 0x00;  /* valid seq: 1 ~ 255 */
        }
        ull_report("[ULL][LE][AVM] bt_ull_le_srv_critial_data_init, max_len:0x%x,ret: 0x%x", 2, max_len, ret);
    } else {
        ret = BT_STATUS_FAIL;
        ull_report_error("[ULL][LE][AVM] bt_ull_le_srv_critial_data_init, fail due to was inited, max_len:0x%x", 1, ctx->critical_data_max_len);
    }
    return ret;
}

static bt_status_t bt_ull_le_srv_critial_data_send(bt_handle_t handle, uint16_t flush_timeout, bt_avm_critial_data_t* data)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();

    if (0 != ctx->critical_data_max_len) {
        ull_assert(data && (data->length <= ctx->critical_data_max_len));
        ret = bt_avm_critical_data_send(handle, flush_timeout, data);
        ull_report("[ULL][LE][AVM] LE critical_data_send, flush_timeout:%d, seq:0x%x, gap_handle:0x%x, result:0x%x", 4 ,flush_timeout, data->seq, handle, ret);
    } else {
        ret = BT_STATUS_FAIL;
        ull_report_error("[ULL][LE][AVM] LE critical_data_send fail due to not inited", 0);
    }
    return ret;
}

void bt_ull_le_srv_lock_streaming(bool lock)
{
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE] bt_ull_le_srv_lock_streaming, old lock: %d, new lock: %d, role:0x%0x", 3, 
        ctx->is_streaming_locked, lock, ctx->role);

    if (ctx->is_streaming_locked != lock) {
        ctx->is_streaming_locked = lock;
    }
    if (BT_ULL_ROLE_SERVER == ctx->role) {
        bt_ull_streaming_t stream;

        if (lock) {
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
               stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
               stream.port = 0;
               bt_ull_le_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
               stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_GAMING;
             }
             if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
               stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
               stream.port = 1;
               bt_ull_le_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
               stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_CHAT;
             }
             if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
               stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
               stream.port = 0;
               bt_ull_le_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
               stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_MIC;
             }
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_IN) {
              stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
              stream.port = 0;
              bt_ull_le_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
              stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_LINE_IN;
            }
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT) {
              stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
              stream.port = 0;
              bt_ull_le_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
              stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT;
            }
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT) {
              stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
              stream.port = 0;
              bt_ull_le_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
              stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT;
            }
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_IN) {
              stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
              stream.port = 0;
              bt_ull_le_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
              stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_I2S_IN;
            }
             /*Remove CIG.*/
            if (ctx->is_cig_created) {
                bt_ull_le_conn_srv_remove_air_cig_params();
            }
        } else {
            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_GAMING;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                stream.port = 0;
                bt_ull_le_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_CHAT;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                stream.port = 1;
                bt_ull_le_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_MIC;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                stream.port = 0;
                bt_ull_le_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_IN) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_LINE_IN;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
                stream.port = 0;
                bt_ull_le_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }

            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
                stream.port = 0;
                bt_ull_le_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }

            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
                stream.port = 0;
                bt_ull_le_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }

            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_IN) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_I2S_IN;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
                stream.port = 0;
                bt_ull_le_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
        }
  } else if (BT_ULL_ROLE_CLIENT == ctx->role) {
     if (lock) {
        /* stop current play & disallow dongle play */
        bt_ull_le_srv_indicate_server_play_is_allow(BT_ULL_PLAY_DISALLOW, BT_ULL_ALLOW_PLAY_REASON_FOTA);
        stream_ctx->locking_port = stream_ctx->streaming_port;
        bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
        bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true);
    } else {
      /* start current play & allow dongle play */
        stream_ctx->locking_port = BT_ULL_LE_STREAM_PORT_MASK_NONE;
        if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
            bt_ull_streaming_t stream;
            stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
            stream.port = 0;
            bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
        } else if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
            bt_ull_streaming_t stream;
            stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
            stream.port = 0;
            bt_ull_le_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_DOWNLINK);
        }
        bt_ull_le_srv_indicate_server_play_is_allow(BT_ULL_PLAY_ALLOW, BT_ULL_ALLOW_PLAY_REASON_FOTA);
    }
  }
}

void bt_ull_le_srv_indicate_server_play_is_allow(bt_ull_allow_play_t is_allow, uint8_t reason)
{
    bt_ull_allow_play_t allow_play = BT_ULL_PLAY_DISALLOW;
    
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bool am_allow = bt_ull_le_am_is_allow_play();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE] bt_ull_le_srv_indicate_server_play_is_allow, %d, %d, %d, %x, %d", 5, \
        is_allow, stream_ctx->allow_play, am_allow, stream_ctx->locking_port, reason);

    if ((BT_ULL_PLAY_ALLOW == is_allow) && am_allow &&
        (!ctx->is_streaming_locked)) {//BTA-37318
        allow_play = BT_ULL_PLAY_ALLOW;
    }

    if (allow_play == stream_ctx->allow_play) {
        return;
    }
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        /* sync usb play state to client */
        stream_ctx->allow_play = allow_play;

        uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_allow_play_report_t);
        uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
        if (NULL != request) {
            request[0] = BT_ULL_EVENT_ALLOW_PLAY_REPORT;
            uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM;
            while (i > 0) {
                i--;
                if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                    (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                        bt_ull_allow_play_report_t *report = (bt_ull_allow_play_report_t *)(request + sizeof(bt_ull_req_event_t));
                        report->allow_play = allow_play;
                        report->reason = reason;
                        bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                }
            }
            bt_ull_le_srv_memory_free(request);
        }
    }
}

static bt_status_t bt_ull_le_srv_l2cap_event_callback(bt_l2cap_fix_channel_event_t event_id, bt_status_t status, void *buffer)
{
    bt_status_t ull_status = BT_STATUS_SUCCESS;
    switch (event_id) {
        case BT_L2CAP_FIX_CHANNEL_DATA_IND: {
            bt_l2cap_fix_channel_data_ind_t *packet = (bt_l2cap_fix_channel_data_ind_t *)buffer;

            ull_report("[ULL][LE] BT_L2CAP_FIX_CHANNEL_DATA_IND, fix cid is 0x%x, status is 0x%x, handle is 0x%x", 3, packet->cid, status, packet->connection_handle);
            if ((BT_ULL_LE_SRV_FIX_CHANNEL_CID == packet->cid) &&
                (BT_STATUS_SUCCESS == status) && (BT_HANDLE_INVALID != packet->connection_handle)) {
                bt_ull_le_srv_rx_data_handle(packet->connection_handle, packet->packet, packet->packet_length);
            } else {
                ull_status = BT_STATUS_FAIL;
            }
        } break;

        default:
            break;
    }
    return ull_status;
}

static bt_status_t bt_ull_le_srv_l2cap_fix_channel_init(void)
{
    bt_status_t status = bt_l2cap_fix_channel_register(BT_L2CAP_FIX_CHANNEL_BLE, BT_ULL_LE_SRV_FIX_CHANNEL_CID, bt_ull_le_srv_l2cap_event_callback);
    ull_report("[ULL][LE] bt_ull_le_srv_l2cap_fix_channel_init, result: 0x%x", 1, status);
    return status;
}

bt_status_t bt_ull_le_srv_init(bt_ull_role_t role, bt_ull_callback callback)
{
    bt_status_t result = BT_STATUS_SUCCESS;
	bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    /**< Init L2CAP  LE Fixed Channel.*/
    if (BT_STATUS_SUCCESS != bt_ull_le_srv_l2cap_fix_channel_init()) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_l2cap_fix_channel_init fail", 0);
        return BT_STATUS_FAIL;
    }
    /**< Reset ULL context.*/
    bt_ull_le_srv_memset(&g_ull_le_ctx, 0x00, sizeof(bt_ull_le_srv_context_t));
    g_ull_le_ctx.role = role;
    g_ull_le_ctx.callback = callback;
    g_ull_le_ctx.client_preferred_scenario_type = bt_ull_le_srv_get_scenario_type();
    /**< Init ULL Audio Transmitter & Audio Manager module & ULL Conn service.*/
    if (BT_ULL_ROLE_SERVER == role) {
        bt_ull_le_at_init_ctx((void *)bt_ull_le_srv_handle_audio_transmitter_event);
    } else if (BT_ULL_ROLE_CLIENT == role) {
        bt_ull_le_am_init((void *)bt_ull_le_srv_handle_audio_manager_event);
    }
    bt_ull_le_conn_srv_init(role, (void *)bt_ull_le_srv_handle_aircis_event);
    bt_ull_le_call_srv_init();
    /**< Reset Link info & Stream ctx.*/
    bt_ull_le_srv_reset_link_info();
    bt_ull_le_srv_reset_stream_ctx();
    /**update client codec param*/
    if (BT_ULL_ROLE_CLIENT == g_ull_le_ctx.role) {
        bt_ull_le_srv_client_preferred_codec_param *codec_param = &(g_ull_le_ctx.client_preferred_codec_param);
        codec_param->codec_type = bt_ull_le_srv_get_client_preffered_codec(g_ull_le_ctx.client_preferred_scenario_type);
        codec_param->dl_samplerate = bt_ull_le_srv_get_client_preffered_dl_codec_samplerate(g_ull_le_ctx.client_preferred_scenario_type);
        codec_param->ul_samplerate = bt_ull_le_srv_get_client_preffered_ul_codec_samplerate(g_ull_le_ctx.client_preferred_scenario_type);
        stream_ctx->codec_type = codec_param->codec_type;
#ifdef AIR_WIRELESS_MIC_ENABLE
        if (BT_ULL_LE_CODEC_ULD == stream_ctx->codec_type) {
            bt_ull_le_srv_set_client_preferred_channel_mode(BT_ULL_LE_CHANNEL_MODE_MONO);
        }
#endif
        bt_ull_le_srv_set_codec_param_by_sample_rate(BT_ULL_ROLE_CLIENT, codec_param->dl_samplerate, codec_param->ul_samplerate);
    }

    bt_ull_le_srv_audio_quality_t quality = bt_ull_le_srv_read_aud_quality_from_nvkey();
    if (BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_INVALID != quality && BT_ULL_ROLE_CLIENT == g_ull_le_ctx.role \
        && BT_ULL_LE_SCENARIO_ULLV2_0 == g_ull_le_ctx.client_preferred_scenario_type) {
        bt_ull_le_srv_change_audio_quality_handle(BT_HANDLE_INVALID, quality);
        //bt_ull_le_srv_change_audio_quality(BT_ULL_ROLE_CLIENT, quality);
    } else {
        stream_ctx->audio_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT;
    }
    /* Register Host callback to ull sniff mode change notification */
    result = bt_callback_manager_register_callback(bt_callback_type_app_event,
    (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_SYSTEM),
    (void *)bt_ull_le_srv_handle_host_event_callback);
#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
    g_bt_ull_le_capability.dchs_support = true;
#endif

#if defined (AIR_BTA_IC_STEREO_HIGH_G3)
    g_bt_ull_le_capability.high_res_support = true;
#endif
    ull_report("[ULL][LE] bt_ull_le_srv_init, result: 0x%x", 1, result);
    return result;
}

bt_status_t bt_ull_le_srv_action(bt_ull_action_t action, const void *param, uint32_t param_len)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    uint32_t data_tx_free_memory = 0;
    bt_ull_le_srv_context_t* ull_context = bt_ull_le_srv_get_context();

    ull_report("[ULL][LE] bt_ull_le_srv_action action: 0x%x, role: 0x%x", 2, action, ull_context->role);

    BT_ULL_MUTEX_LOCK();
    switch (action) {
        case BT_ULL_ACTION_FIND_CS_INFO: {
            ull_assert(param);
            bt_ull_le_find_cs_info_t *cs_data = (bt_ull_le_find_cs_info_t *)param;
            status = bt_ull_le_srv_handle_find_cs(cs_data);
        }
        break;
        case BT_ULL_ACTION_START_STREAMING: {
            ull_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t*)param;
            status = bt_ull_le_srv_handle_start_streaming(streaming);
            break;
        }
        case BT_ULL_ACTION_STOP_STREAMING: {
            ull_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t*)param;
            status = bt_ull_le_srv_handle_stop_streaming(streaming);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_VOLUME: {
            ull_assert(param);
            bt_ull_volume_t *volume = (bt_ull_volume_t*)param;
            status = bt_ull_le_srv_handle_set_volume(volume);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_MUTE: {
            ull_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t*)param;
            status = bt_ull_le_srv_handle_set_streaming_mute(streaming, true);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_UNMUTE: {
            ull_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t*)param;
            status = bt_ull_le_srv_handle_set_streaming_mute(streaming, false);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE: {
            ull_assert(param);
            bt_ull_sample_rate_t *sample_rate = (bt_ull_sample_rate_t*)param;
            status = bt_ull_le_srv_handle_set_streaming_sample_rate(sample_rate);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_SAMPLE_SIZE: {
            ull_assert(param);
            bt_ull_streaming_sample_size_t *sample_size = (bt_ull_streaming_sample_size_t*)param;
            status = bt_ull_le_srv_handle_set_streaming_sample_size(sample_size);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_SAMPLE_CHANNEL: {
            ull_assert(param);
            bt_ull_streaming_sample_channel_t *sample_channel = (bt_ull_streaming_sample_channel_t*)param;
            status = bt_ull_le_srv_handle_set_streaming_sample_channel(sample_channel);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_LATENCY: {
            ull_assert(param);
            bt_ull_latency_t *latency = (bt_ull_latency_t*)param;
            status = bt_ull_le_srv_handle_set_streaming_latency(latency, BT_HANDLE_INVALID);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_MIX_RATIO: {
            ull_assert(param);
            bt_ull_mix_ratio_t *ratio = (bt_ull_mix_ratio_t*)param;
            status = bt_ull_le_srv_handle_set_streaming_mix_ratio(ratio);
            break;
        }
        case BT_ULL_ACTION_TX_USER_DATA: {
            ull_assert(param);
            uint8_t i = (BT_ULL_ROLE_SERVER == ull_context->role) ? BT_ULL_LE_MAX_LINK_NUM : 1;
            bt_ull_user_data_t *tx = (bt_ull_user_data_t*)param;

            while (i > 0) {
                i--;
                if ((0 == memcmp(g_ull_le_link_info[i].addr, tx->remote_address, sizeof(bt_bd_addr_t))) &&
                    (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                    bt_ull_req_event_t req_action = BT_ULL_EVENT_USER_DATA;
                    uint16_t total_len = tx->user_data_length + sizeof(tx->user_data_length) + sizeof(bt_ull_req_event_t);
                    uint8_t *tx_buf = (uint8_t*)bt_ull_le_srv_memory_alloc(total_len);
                    if (NULL != tx_buf) {
			data_tx_free_memory=bt_memory_get_total_free_size(BT_MEMORY_TX_BUFFER);
            		ull_report("[ULL][LE] BT_ULL_ACTION_TX_USER_DATA data_tx_free_memory: 0x%x", 2, data_tx_free_memory);
			if(data_tx_free_memory>0xc0)  // harry for out memory 2024 1014
			{
                        tx_buf[0] = req_action;
                        ull_assert(tx->user_data_length && tx->user_data);
                        bt_ull_le_srv_memcpy(&tx_buf[1], &(tx->user_data_length), sizeof(tx->user_data_length));
                        bt_ull_le_srv_memcpy(&tx_buf[3], tx->user_data, tx->user_data_length);
                        status = bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, tx_buf, total_len);
			}
			else	{
            		ull_report_error("[ULL][LE] BT_ULL_ACTION_TX_USER_DATA data_tx_free_memory: 0x%x out memoryyyyyyyyyyyyyy", 2, data_tx_free_memory);
				}
                        bt_ull_le_srv_memory_free(tx_buf);
                    }
                }
            }

            ull_report("[ULL][LE] BT_ULL_ACTION_TX_USER_DATA astatus: 0x%x", 2, status);
            break;
        }
        case BT_ULL_ACTION_INIT_CRITICAL_USER_DATA: {
            ull_assert(param);
            bt_ull_init_critical_user_data_t *max_len = (bt_ull_init_critical_user_data_t*)param;
            status = bt_ull_le_srv_critial_data_init(max_len->max_user_data_length);
            break;
        }
        case BT_ULL_ACTION_TX_CRITICAL_USER_DATA: {
            ull_assert(param);
            uint8_t i = (BT_ULL_ROLE_SERVER == ull_context->role) ? BT_ULL_LE_MAX_LINK_NUM : 1;

            bt_ull_tx_critical_user_data_t *tx_data = (bt_ull_tx_critical_user_data_t*)param;
            while (i > 0) {
                i--;
                if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                    (BT_ULL_LE_LINK_STATE_STREAMING == g_ull_le_link_info[i].curr_state)) {
                    bt_avm_critial_data_t critical_data = {0};
                    critical_data.seq = ull_context->critical_data_tx_seq;
                    critical_data.length = tx_data->user_data_length;
                    critical_data.data = tx_data->user_data;
                    critical_data.is_le_data = true;
                    status = bt_ull_le_srv_critial_data_send(g_ull_le_link_info[i].conn_handle, tx_data->flush_timeout, &critical_data);
                    if (BT_STATUS_SUCCESS == status) {
                        if (0xFF == ull_context->critical_data_tx_seq) {
                            ull_context->critical_data_tx_seq = 0x01;
                        } else {
                            ull_context->critical_data_tx_seq++;
                        }
                    }
                }
            }

            ull_report("[ULL][LE] BT_ULL_ACTION_TX_CRITICAL_USER_DATA status: 0x%x", 2, status);
            break;
        }
        case BT_ULL_ACTION_USB_HID_CONTROL: {
            ull_assert(param);
            bt_ull_usb_hid_control_t control_id = *((bt_ull_usb_hid_control_t*)param);
            status = bt_ull_le_srv_handle_usb_hid_control(control_id);
            break;
        }
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE) || (defined AIR_DONGLE_I2S_SLV_OUT_ENABLE) || (defined AIR_DONGLE_I2S_MST_OUT_ENABLE)
        case BT_ULL_ACTION_SET_INTERFACE_PARAM: {
            ull_assert(param);
            bt_ull_interface_config_t *info = (bt_ull_interface_config_t*)param;
            bt_ull_le_at_set_streaming_param(info);
            break;
        }
#endif
        case BT_ULL_ACTION_SET_CLIENT_SIDETONE_SWITCH: {
            ull_assert(param);
            bt_ull_client_sidetone_switch_t *sidetone_switch = (bt_ull_client_sidetone_switch_t*)param;
            bt_ull_le_am_set_sidetone_switch(sidetone_switch->sidetone_enable);
            break;
        }
        case BT_ULL_ACTION_SET_AUDIO_QUALITY: {
            ull_assert(param);
            bt_ull_le_srv_audio_quality_t *audio_quality = (bt_ull_le_srv_audio_quality_t *)param;
            status = bt_ull_le_srv_change_audio_quality_handle(BT_HANDLE_INVALID, *audio_quality);
            break;
        }
        case BT_ULL_ACTION_SWITCH_UPLINK: {
            ull_assert(param);
            bt_ull_le_client_switch_ul_ind_t *switch_ul = (bt_ull_le_client_switch_ul_ind_t*)param;
            status = bt_ull_le_srv_client_switch_uplink_handler(switch_ul->is_need_switch_ul);
            break;
        }
        case BT_ULL_ACTION_SET_ULL_SCENARIO: {
            ull_assert(param);
            bt_ull_le_scenario_t *scenario_type = (bt_ull_le_scenario_t *)param;
            status = bt_ull_le_srv_change_ull_scenario_handle(*scenario_type);
            break;
        }
        case BT_ULL_ACTION_SWITCH_AIRCIS_INACTIVE_MODE: {
            ull_assert(param);
            bool enable = *(uint8_t*)param;
            status = bt_ull_le_srv_write_aircis_inactive_mode_enable(enable);
            break;
        }
        default:
            status = BT_STATUS_SUCCESS;
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
    return status;
}

static bt_status_t bt_ull_le_srv_calculate_air_cis_count(void)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();

    /* To do: get set size and AirCIS num */
    switch (ctx->client_type) {
        case BT_ULL_HEADSET_CLIENT: {
            ctx->cis_num = ctx->cs_size * BT_ULL_LE_HEADSET_CHANNEL_COUNTS;
        } break;

        case BT_ULL_EARBUDS_CLIENT: {
            ctx->cis_num = ctx->cs_size * BT_ULL_LE_EARBUDS_CHANNEL_COUNTS;
        } break;

        case BT_ULL_MIC_CLIENT: {
            ctx->cis_num = ctx->cs_size * BT_ULL_LE_MIC_CHANNEL_COUNTS;
        } break;

        case BT_ULL_SPEAKER_CLIENT: {
            ctx->cis_num = ctx->cs_size * BT_ULL_LE_SPK_CHANNEL_COUNTS;
        } break;

        default:
            return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE] bt_ull_le_srv_calculate_air_cis_count, client type is %d, cis_num is %d", 2, ctx->client_type, ctx->cis_num);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_ull_le_srv_open_audio_stream(bt_ull_le_stream_port_mask_t stream_port)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    if ((!bt_ull_le_srv_is_connected() || (0 == stream_ctx->streaming_port))) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_open_audio_strem fail, streaming_port mask is %d", 1, stream_ctx->streaming_port);
        return BT_STATUS_FAIL;
    }
    /* the audio tramsmitter has been open. */
    bt_ull_transmitter_t trans = bt_ull_le_srv_get_transmitter_by_stream_port(stream_port);

    /* init controller/DSP media data share buffer info */
    if (!ctx->is_share_buff_set) {
        status = bt_ull_le_srv_init_share_info(ctx->client_type);
        ull_report("[ULL][LE] bt_ull_le_srv_open_audio_strem, init share buffer: 0x%x, status: 0x%x", 2, ctx->is_disable_sniff, status);

        if ((BT_STATUS_SUCCESS == status) && (BT_STATUS_SUCCESS == bt_ull_le_srv_set_avm_share_buffer(ctx->role, ctx->client_type, ctx->cis_num))) {
            ctx->is_share_buff_set = true;
        } else {
            ull_report_error("[ULL][LE] bt_ull_le_srv_open_audio_strem fail: set AVM buffer error", 0);
            return BT_STATUS_FAIL;
        }
    }

    /* check gaming port state */
    bt_ull_le_srv_audio_out_t audio_out = bt_ull_le_srv_get_audio_out_by_stream_port(stream_port);

    /*set codec info to controller (for set CIG) & to Audio transmitter. */
    status = bt_ull_le_at_init(ctx->client_type, audio_out, trans, stream_ctx->codec_type);
    if ((BT_STATUS_SUCCESS == status) && ((BT_ULL_PLAY_ALLOW == stream_ctx->allow_play))) {
        if (BT_STATUS_SUCCESS == (status = bt_ull_le_at_start(trans, true))) {
            bt_ull_le_srv_set_transmitter_is_start(trans, true);
            if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
                /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: Change state machine, do nothing*/
            } else {
                if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->curr_stream_state) {
                        bt_ull_le_srv_set_curr_stream_state(BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM);
                }
            }
        }
    }
    ull_report("[ULL][LE] start tramitter is %d, status is 0x%x, curr_stream_state is %d, allow paly: %d", 4, \
        trans, status, ctx->curr_stream_state, stream_ctx->allow_play);
    return status;
}

static bt_status_t bt_ull_le_send_data_by_fix_channel(bt_handle_t handle, uint8_t *data, uint16_t length)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_l2cap_fix_channel_tx_data_t tx_data;
    tx_data.type = BT_L2CAP_FIX_CHANNEL_BLE;
    tx_data.connection_handle = handle;
    tx_data.cid = BT_ULL_LE_SRV_FIX_CHANNEL_CID;
    tx_data.data = data;
    tx_data.length = length;

    status = bt_l2cap_fix_channel_send(&tx_data);
    if (BT_STATUS_OUT_OF_MEMORY == status) {
        ull_assert(0 && "send data fail because of OOM!");
    }
    return status;
}

bt_status_t bt_ull_le_srv_send_data(uint16_t handle, uint8_t *packet, uint16_t packet_size)
{
    bt_status_t result = BT_STATUS_FAIL;
    uint16_t send_length = 0;
    uint32_t data_tx_free_memory = 0;
    uint32_t data_rx_free_memory = 0;
    bool need_resend = false;
    bt_ull_le_srv_link_info_t *link_info = bt_ull_le_srv_get_link_info(handle);

    if ((BT_HANDLE_INVALID == handle) || (!link_info)) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_send_data, invalid params", 0);
        return BT_STATUS_FAIL;
    }
	data_tx_free_memory=bt_memory_get_total_free_size(BT_MEMORY_TX_BUFFER);
	data_rx_free_memory=bt_memory_get_total_free_size(BT_MEMORY_RX_BUFFER);
    ull_report("[ULL][LE] bt_ull_le_srv_send_data, status: data_tx_free_memory:0x%x, data_rx_free_memory: 0x%x, packet_len: 0x%x,", 3, data_tx_free_memory, data_rx_free_memory, packet_size);
    if (link_info->max_packet_size < packet_size) {
        need_resend = true;
        send_length = link_info->max_packet_size;
        result = bt_ull_le_send_data_by_fix_channel(handle, packet, send_length);
        if ((BT_STATUS_SUCCESS == result) && (need_resend)) {
            need_resend = false;
            result = bt_ull_le_srv_send_data(handle, (uint8_t *)(packet + send_length), (packet_size - send_length));
        }
        ull_report("[ULL][LE] bt_ull_le_srv_send_data, packet_size: %d, send size: %d, status: %d, need_resend: %d", 4,
            packet_size, send_length, result, need_resend);
    } else {
        send_length = packet_size;
        result = bt_ull_le_send_data_by_fix_channel(handle, packet, packet_size);
    }
    ull_report("[ULL][LE] bt_ull_le_srv_send_data, status: 0x%x, handle: 0x%x, packet_len: 0x%x, send_len: 0x%x", 4, result, handle, packet_size, send_length);
    return result;
}

/*< Begin: the following fucntions is just for Xbox project. */
#define BT_ULL_LE_MIC_DATA_FRAME_SIZE (32)
uint32_t bt_ull_le_srv_get_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *buffer, uint32_t buffer_length)
{
    uint32_t read_length = 0;
    bt_ull_le_srv_server_stream_t *stream_info;

    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_assert(streaming && buffer && "error parameter!");

    if (BT_ULL_ROLE_SERVER != ull_ctx->role) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_get_raw_pcm_data, fail, current role is %d, not support this feature", 1, ull_ctx->role);
        return BT_STATUS_FAIL;
    }

    if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface) {
        stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, BT_ULL_MIC_TRANSMITTER);
        if ((stream_info) && (stream_info->is_transmitter_start) && (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC)) {
            /**<Mic is opened and running. */
            /**<Call DSP's API to get raw pcm data. */
            bt_status_t status = bt_ull_le_at_read_pcm_data(BT_ULL_MIC_TRANSMITTER, buffer, &buffer_length);
            read_length = (BT_STATUS_SUCCESS == status) ? BT_ULL_LE_MIC_DATA_FRAME_SIZE : 0;
        }
    } else {
        ull_assert(0 && "only support get mic uplink pcm data!");
    }
    return read_length;
}

bt_status_t bt_ull_le_srv_write_raw_pcm_data(bt_ull_streaming_t *streaming, uint8_t *data, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_srv_server_stream_t *stream_info = NULL;
    bt_ull_le_stream_port_mask_t write_port = 0;

    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    //ull_report("[ULL][LE] bt_ull_le_srv_write_raw_pcm_data, streaming_if:0x%x, port: 0x%x", 2, streaming->streaming_interface, streaming->port);

    ull_assert(data);
    if (BT_ULL_ROLE_SERVER != ull_ctx->role) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_write_raw_pcm_data, fail, current role is %d, not support this feature", 1, ull_ctx->role);
        return BT_STATUS_FAIL;
    }

    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
    if ((BT_ULL_GAMING_TRANSMITTER == trans_type) || (BT_ULL_CHAT_TRANSMITTER == trans_type)) {
        stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
    } else {
        ull_assert(0 && "error parameter!");
    }

    write_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
    if ((stream_info) && (stream_info->is_transmitter_start) && (stream_ctx->streaming_port & write_port)) {
        /**<DL is opened and running. */
        /**<Call DSP's API to write raw pcm data. */
        status = bt_ull_le_at_write_pcm_data(trans_type, data, &length);
    } else {
        ull_report_error("[ULL] bt_ull_le_srv_write_raw_pcm_data fail, transmitter %d is not start", 1, trans_type);
    }
    //ull_report("[ULL][LE] bt_ull_le_srv_write_raw_pcm_data, status:0x%x", 1, status);
    return status;
}

/*< end: the previous fucntions is just for Xbox project. */

bool bt_ull_le_srv_is_transmitter_start(bt_ull_streaming_t *streaming)
{
    bt_ull_le_srv_server_stream_t *stream_info;

    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    //ull_report("[ULL][LE] bt_ull_le_srv_is_transmitter_start, streaming_if:0x%x, port: 0x%x", 2, streaming->streaming_interface, streaming->port);

    if (BT_ULL_ROLE_SERVER != ull_ctx->role) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_is_transmitter_start, fail, current role is %d, not support this feature", 1, ull_ctx->role);
        return BT_STATUS_FAIL;
    }
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
    stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);

    if (stream_info) {
        return stream_info->is_transmitter_start;
    }
    return false;
}

bt_ull_le_audio_location_t bt_ull_le_srv_get_audio_location_by_handle(bt_handle_t handle)
{
    bt_ull_le_srv_link_info_t *link_info = bt_ull_le_srv_get_link_info(handle);
    if (!link_info) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_get_audio_location_by_handle, invalid link info!", 0);
        return BT_ULL_LE_AUDIO_LOCATION_NONE;
    }
    return link_info->sink_cfg.audio_location;
}

static bt_ull_le_srv_link_info_t* bt_ull_le_srv_get_other_connected_link_info(bt_handle_t handle)
{
    uint8_t i;
    for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
        if(BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle &&
            handle != g_ull_le_link_info[i].conn_handle &&
            g_ull_le_link_info[i].curr_state >= BT_ULL_LE_LINK_STATE_READY) {
            return &g_ull_le_link_info[i];
        }
	}
    return NULL;
}

static uint8_t bt_ull_le_srv_get_connected_link_num(void)
{
    uint8_t i = 0;
    uint8_t connected_num = 0;
    for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i ++) {
        if(BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle &&
            g_ull_le_link_info[i].curr_state >= BT_ULL_LE_LINK_STATE_READY) {
            connected_num++;
        }
    }
    return connected_num;
}

bt_status_t bt_ull_le_srv_activate_up_link_handler(bt_handle_t handle)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_link_info_t *link_info = bt_ull_le_srv_get_link_info(handle);
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    if (NULL == link_info) {
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE] bt_ull_le_srv_activate_up_link_handler, curr_state:%d client_type:%d ul_active:%d ul_enable:%d handle:%d!", 5,
        link_info->curr_state, ctx->client_type, link_info->ul_active, link_info->cis_info.ul_enable, handle);
    if (BT_ULL_EARBUDS_CLIENT != ctx->client_type) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_activate_up_link_handler, invalid ct!", 0);
        return BT_STATUS_FAIL;
    }
    if (link_info->ul_active) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_activate_up_link_handler, ul has activated!", 0);
        return status;
    }
    link_info->ul_active = true;

    bt_ull_le_srv_link_info_t* other_link = bt_ull_le_srv_get_other_connected_link_info(handle);
    if (other_link) {
        other_link->ul_active = false;
        link_info->cis_info.ul_enable = false;
    }
    if (BT_ULL_LE_LINK_STATE_READY <= link_info->curr_state) {
        uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_activate_ul_t);
        uint8_t *ul_activate = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
        if (NULL != ul_activate) {
            ull_report("[ULL][LE] bt_ull_le_srv_activate_up_link_handler, ######!", 0);
            ul_activate[0] = BT_ULL_EVENT_ACTIVATE_UPLINK_IND;
            bt_ull_le_srv_activate_ul_t *enable =(bt_ull_le_srv_activate_ul_t *)(ul_activate + sizeof(bt_ull_req_event_t));
            enable->enable = true;
            status = bt_ull_le_srv_send_data(handle, (uint8_t*)ul_activate, len);
            bt_ull_le_srv_memory_free(ul_activate);
        }
    }
    if (BT_STATUS_SUCCESS == status) {
        status = bt_ull_le_conn_srv_activate_uplink(handle);
    }
    return status;
}

bool bt_ull_le_srv_get_connected_addr_by_link_index(uint8_t index, bt_bd_addr_t *addr)
{
    bt_ull_le_srv_link_info_t *link_info = &g_ull_le_link_info[index];
    if (link_info->conn_handle != BT_HANDLE_INVALID && link_info->curr_state >= BT_ULL_LE_LINK_STATE_READY) {
        bt_ull_le_srv_memcpy(addr, &(link_info->addr), sizeof(bt_bd_addr_t));
        return true;
    } else {
        bt_ull_le_srv_memset(addr , 0, sizeof(bt_bd_addr_t));
    }
    return false;
}

uint8_t bt_ull_le_srv_get_connected_link_index_by_addr(bt_bd_addr_t *addr)
{
    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    uint8_t max_link_count = (BT_ULL_ROLE_SERVER == ull_ctx->role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;
    uint8_t i;
    uint8_t index = 0xFF;
    for (i = 0; i < max_link_count; i++) {
        if (0 == (bt_ull_le_srv_memcmp(&g_ull_le_link_info[i].addr, addr, sizeof(bt_bd_addr_t)))) {
            index = i;
            break;
        }
    }
    ull_report("[ULL][LE] bt_ull_le_srv_get_connected_link_index_by_addr, link index: %d", 1, index);
    return index;
}

#ifdef AIR_WIRELESS_MIC_ENABLE
static void bt_ull_le_srv_set_client_preferred_channel_mode(bt_ull_le_channel_mode_t  channel_mode)
{
    g_ull_le_ctx.client_preferred_channel_mode = channel_mode;
}

static bt_ull_le_channel_mode_t bt_ull_le_srv_get_client_preferred_channel_mode(void)
{
    return g_ull_le_ctx.client_preferred_channel_mode;
}
#endif

#ifdef AIR_WIRELESS_MIC_RX_ENABLE
bt_status_t bt_ull_le_srv_set_audio_connection_info(bt_ull_streaming_t *streaming, void *audio_connection_info, uint32_t size)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_stream_port_mask_t start_port = 0;
    ull_assert(streaming && audio_connection_info && (size) && "error parameter!");
    if (BT_ULL_ROLE_SERVER != ull_ctx->role) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_set_audio_connection_info, fail, current role is %d, not support this feature", 1, ull_ctx->role);
        return BT_STATUS_FAIL;
    }
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
    start_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
    bt_ull_le_srv_audio_out_t audio_out = bt_ull_le_srv_get_audio_out_by_stream_port(start_port);
    if (bt_ull_le_srv_is_connected() && (BT_STATUS_SUCCESS == bt_ull_le_at_init(ull_ctx->client_type, audio_out, trans_type, stream_ctx->codec_type))) {
        status = bt_ull_le_at_set_audio_connection_info(trans_type, audio_connection_info, size);
    } else {
        ull_report_error("[ULL][LE] bt_ull_le_srv_set_audio_connection_info, fail, transmitter type: %d", 1, trans_type);
    }
    return status;
}

bt_status_t bt_ull_le_srv_set_safety_mode_volume(bt_ull_streaming_t *streaming, S32 left_vol_diff, S32 right_vol_diff)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_stream_port_mask_t start_port = 0;
    ull_assert(streaming && "error parameter!");
    if (BT_ULL_ROLE_SERVER != ull_ctx->role) {
        ull_report_error("[ULL][LE] bt_ull_le_srv_set_safety_mode_volume, fail, current role is %d, not support this feature", 1, ull_ctx->role);
        return BT_STATUS_FAIL;
    }
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
    start_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
    bt_ull_le_srv_audio_out_t audio_out = bt_ull_le_srv_get_audio_out_by_stream_port(start_port);
    if (bt_ull_le_srv_is_connected() && (BT_STATUS_SUCCESS == bt_ull_le_at_init(ull_ctx->client_type, audio_out, trans_type, stream_ctx->codec_type))) {
        status = bt_ull_le_at_set_wireless_mic_safety_mode_volume_by_channel(trans_type, left_vol_diff, right_vol_diff);
    } else {
        ull_report_error("[ULL][LE] bt_ull_le_srv_set_safety_mode_volume, fail, transmitter type: %d", 1, trans_type);
    }
    return status;

}

uint8_t bt_ull_le_srv_get_streaming_volume(bt_ull_streaming_t *streaming)
{
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
    return bt_ull_le_at_get_volume_value(trans_type);
}

#endif

bt_status_t bt_ull_le_srv_enable_streaming(void)
{
    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t max_link_count = (BT_ULL_ROLE_SERVER == ull_ctx->role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;
    uint8_t i;
    for (i = 0; i < max_link_count; i++) {
        ull_report("[ULL][LE] bt_ull_le_srv_enable_streaming, link index: %d, handle: %x, state: %d", 3, i, g_ull_le_link_info[i].conn_handle, g_ull_le_link_info[i].curr_state);
        if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && g_ull_le_link_info[i].curr_state > BT_ULL_LE_LINK_STATE_CREATING_CIS) {
            status = bt_ull_le_conn_srv_activiate_air_cis(g_ull_le_link_info[i].conn_handle);
        }
    }
    return status;
}

bt_status_t bt_ull_le_srv_disable_streaming(void)
{
    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t max_link_count = (BT_ULL_ROLE_SERVER == ull_ctx->role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;
    uint8_t i;
    for (i = 0; i < max_link_count; i++) {
        ull_report("[ULL][LE] bt_ull_le_srv_disable_streaming, link index: %d, handle: %x, state: %d", 3, i, g_ull_le_link_info[i].conn_handle, g_ull_le_link_info[i].curr_state);
        if (BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle && g_ull_le_link_info[i].curr_state == BT_ULL_LE_LINK_STATE_STREAMING) {
            status = bt_ull_le_conn_srv_deactivate_air_cis(g_ull_le_link_info[i].conn_handle);
        }
    }
    return status;
}

bt_status_t bt_ull_le_srv_notify_client_restart_streaming(bt_ull_le_restart_streaming_reason_t reason)
{
    uint8_t i = 0;
    uint8_t mode_mask = 0;
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    if (BT_ULL_ROLE_SERVER == ull_ctx->role) {
        if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
            mode_mask |= (1 << BT_ULL_LE_STREAM_MODE_DOWNLINK);
        }
        if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
            mode_mask |= (1 << BT_ULL_LE_STREAM_MODE_UPLINK);
        }
        ull_report("[ULL][LE] bt_ull_le_srv_notify_client_restart_streaming, reason: %d, mode: %x", 2, reason, mode_mask);
        for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM; i++) {/*notify all connected device.*/
            if ((BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) && mode_mask) {
                uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_notify_restart_streaming_t);
                uint8_t *request = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                if (NULL != request) {
                    request[0] = BT_ULL_EVENT_RESTART_STREAMING_IND;
                    bt_ull_le_srv_notify_restart_streaming_t *data = \
                        (bt_ull_le_srv_notify_restart_streaming_t *)(request + sizeof(bt_ull_req_event_t));
                    data->reason = reason;
                    data->mode_mask = mode_mask;
                    status = bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                    bt_ull_le_srv_memory_free(request);
                }
            }
        }
    }
    return status;
}

bt_status_t bt_ull_le_srv_set_group_device_addr(bt_addr_t *addr)
{
    uint8_t i = 0;
    uint8_t *debug_addr = NULL;
    if (!addr) {
        return BT_STATUS_FAIL;
    }
    for (i = 0; i <BT_ULL_LE_MAX_LINK_NUM - 1; i ++) {
        debug_addr = addr[i].addr;
        ull_report("[ULL][LE] Set group type: %daddr: %x-%x-%x-%x-%x-%x", 7, \
        addr[i].type, debug_addr[0], debug_addr[1], debug_addr[2], debug_addr[3], debug_addr[4], debug_addr[5]);
        memcpy((uint8_t *)&g_group_device_addr[i], (uint8_t *)addr, sizeof(bt_addr_t));
    }
    return BT_STATUS_SUCCESS;
}

bt_handle_t bt_ull_le_srv_get_connection_handle_by_index(uint8_t idx)
{
    return g_ull_le_link_info[idx].conn_handle;    
}

bt_ull_le_link_state_t bt_ull_le_srv_get_link_state_by_index(uint8_t idx)
{
    return g_ull_le_link_info[idx].curr_state;
}

static bool bt_ull_le_srv_is_any_aircis_connected(void)
{
    bool is_any_aircis_connected = false;
    uint8_t i = (BT_ULL_ROLE_SERVER == g_ull_le_ctx.role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        if (BT_HANDLE_INVALID == g_ull_le_link_info[i].conn_handle) {
            continue;
        }
        if (BT_ULL_LE_LINK_STATE_STREAMING <= g_ull_le_link_info[i].curr_state) {
            is_any_aircis_connected = true;
        }
    }
    ull_report("[ULL][LE] bt_ull_le_srv_is_any_aircis_connected: %d", 1, is_any_aircis_connected);
    return is_any_aircis_connected;
}
#ifdef AIR_SILENCE_DETECTION_ENABLE
void bt_ull_le_srv_silence_detection_notify_client(bool is_silence, bt_ull_transmitter_t transmitter_type)
{
    ull_report("[ULL][LE][SD_DEBUG] bt_ull_le_srv_silence_detection_notify_client, is_silence: %d, transmitter_type: 0x%x", 2, is_silence, transmitter_type);
    if (bt_ull_le_srv_is_connected()) {
        bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(transmitter_type);
        /*send BT_ULL_EVENT_STREAMING_STOP_IND or BT_ULL_EVENT_STREAMING_START_IND to client and notify client stop am*/
        uint8_t event = is_silence ? BT_ULL_EVENT_STREAMING_STOP_IND : BT_ULL_EVENT_STREAMING_START_IND;
		bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
        stream_ctx->is_silence = is_silence;
        bt_ull_le_srv_sync_streaming_status(stream_port, event);
    }
}
#endif

bt_ull_le_srv_capability_t *bt_ull_le_srv_get_peer_capability(bt_handle_t acl_handle)
{
    bt_ull_le_srv_link_info_t *link = bt_ull_le_srv_get_link_info(acl_handle);
    if (!link) {
        return NULL;
    }
    return &link->peer_capability_msk;
}

static bt_status_t bt_ull_le_srv_enable_adaptive_bitrate_mode_internal(bt_handle_t handle, bt_ull_le_adaptive_bitrate_params_t *adaptive_bitrate_param)
{
    bt_ull_le_srv_link_info_t *link = bt_ull_le_srv_get_link_info(handle);
    uint8_t idx = bt_ull_le_srv_get_idx_by_handle(handle);
    bt_status_t status = BT_STATUS_FAIL;
    if (!link || BT_ULL_LE_SRV_INVALID_VALUE == idx) {
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE] bt_ull_le_srv_enable_adaptive_bitrate_mode_internal: handle: %d, state: %d, enable: %d, interval: %d, crc: %d, rx_to: %d, flush_to: %d", 7, \
         handle,
         link->curr_state,
         adaptive_bitrate_param->enable,
         adaptive_bitrate_param->report_interval,
         adaptive_bitrate_param->crc_threshold,
         adaptive_bitrate_param->flush_timeout_threshold);
    if (BT_ULL_LE_LINK_STATE_READY > link->curr_state) {
        return BT_STATUS_FAIL;
    }
    if (BT_ULL_LE_SRV_IS_SUPPORT(idx, BT_ULL_LE_SRV_FEATURE_MHDT_8M_PHY) || BT_ULL_LE_SRV_IS_SUPPORT(idx, BT_ULL_LE_SRV_FEATURE_MHDT_4M_PHY)) {
        bt_ull_le_enable_qos_report_t enable;
        enable.acl_connection_handle = handle;
        enable.enable = adaptive_bitrate_param->enable;
        enable.report_interval = adaptive_bitrate_param->report_interval;
        enable.crc_threshold = adaptive_bitrate_param->crc_threshold;
        enable.rx_timeout_threshold = adaptive_bitrate_param->rx_timeout_threshold;
        enable.flush_timeout_threshold = adaptive_bitrate_param->flush_timeout_threshold;
        status = bt_ull_le_enable_qos_report(&enable);
    }
     return status;
}

bt_status_t bt_ull_le_srv_enable_adaptive_bitrate_mode(bt_ull_le_adaptive_bitrate_params_t *adaptive_bitrate_param)
{
    bt_ull_le_srv_context_t *ull_ctx = bt_ull_le_srv_get_context();
    bt_status_t status = BT_STATUS_SUCCESS;
    ull_report("[ULL][LE] bt_ull_le_srv_enable_adaptive_bitrate_mode, scenario: %d", 1, ull_ctx->client_preferred_scenario_type);
    bt_ull_le_srv_memcpy(&ull_ctx->adaptive_bitrate_param, adaptive_bitrate_param, sizeof(bt_ull_le_adaptive_bitrate_params_t));
    if (BT_ULL_LE_SCENARIO_ULLV2_0 != ull_ctx->client_preferred_scenario_type) {
        return BT_STATUS_FAIL;
    }
    if (adaptive_bitrate_param->enable) {
        bt_ull_le_srv_change_audio_quality_handle(BT_HANDLE_INVALID, BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION);
    }
    uint8_t i = (BT_ULL_ROLE_SERVER == g_ull_le_ctx.role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;
    if (bt_ull_le_srv_is_connected()) {
        while (i > 0) {
            i--;
            if (BT_HANDLE_INVALID == g_ull_le_link_info[i].conn_handle) {
                continue;
            }
            if (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                status = bt_ull_le_srv_enable_adaptive_bitrate_mode_internal(g_ull_le_link_info[i].conn_handle, adaptive_bitrate_param);
            }
        }
    }
    return status;
}

static bt_status_t bt_ull_le_srv_disable_adaptive_bitrate_mode(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_adaptive_bitrate_params_t pram = {0};
    pram.enable = false;
    bt_ull_le_srv_change_audio_quality_handle(BT_HANDLE_INVALID, BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT);
    uint8_t i = (BT_ULL_ROLE_SERVER == g_ull_le_ctx.role) ? BT_ULL_LE_MAX_LINK_NUM : BT_ULL_LE_CLIENT_LINK_MAX_NUM;
    if (bt_ull_le_srv_is_connected()) {
        while (i > 0) {
            i--;
            if (BT_HANDLE_INVALID == g_ull_le_link_info[i].conn_handle) {
                continue;
            }
            if (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state) {
                status = bt_ull_le_srv_enable_adaptive_bitrate_mode_internal(g_ull_le_link_info[i].conn_handle, &pram);
            }
        }
    }
    return status;

}

static bt_status_t bt_ull_le_srv_client_switch_uplink_handler(bool switch_ul)
{
    ull_report("[ULL][LE]bt_ull_le_srv_client_switch_uplink_handler, switch_ul: 0x%x", 1, switch_ul);
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_status_t status = BT_STATUS_FAIL;

    if ((bt_ull_le_srv_is_connected()) && (BT_ULL_EARBUDS_CLIENT == ctx->client_type)  && switch_ul) {
        if (BT_ULL_ROLE_CLIENT == ctx->role) {
            /*check ul is open*/
            
            if (stream_ctx->client.ul.is_am_open) {
                ull_report("[ULL][LE][DEBUG]bt_ull_le_srv_client_switch_uplink_handler, uplink is on this link, not need switch uplink.", 0);
                return BT_STATUS_SUCCESS; 
            } else {
                uint8_t i;
                for (i = 0; i < BT_ULL_LE_CLIENT_LINK_MAX_NUM; i++) {
                    if ((BT_HANDLE_INVALID != g_ull_le_link_info[i].conn_handle) &&
                        (BT_ULL_LE_LINK_STATE_READY <= g_ull_le_link_info[i].curr_state)) {
                        /*sync enter smart charger status to dongle*/
                        uint8_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_client_switch_ul_ind_t);
                        uint8_t *request = (uint8_t*)bt_ull_le_srv_memory_alloc(len);
                        if (NULL != request) {
                            request[0] = BT_ULL_EVENT_SWITCH_UPLINK_IND;
                            bt_ull_le_client_switch_ul_ind_t *switch_ul_ind = (bt_ull_le_client_switch_ul_ind_t*)(request + sizeof(bt_ull_req_event_t));
                            switch_ul_ind->is_need_switch_ul = switch_ul;
                            status = bt_ull_le_srv_send_data(g_ull_le_link_info[i].conn_handle, (uint8_t*)request, len);
                            bt_ull_le_srv_memory_free(request);                        }
                    }
                }
            }
        }
        ull_report("[ULL][LE]sync dongle switch uplink status: 0x%x", 1, status);
    } else {
        ull_report_error("[ULL][LE] can not switch uplink!!", 0);
    }
    return status;
}

static void bt_ull_le_srv_switch_uplink(bt_handle_t handle, bool switch_ul)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_link_info_t *link_info = NULL;
    /*switch ul*/
    if (switch_ul) {
        if (NULL != (link_info = bt_ull_le_srv_get_link_info(handle))) {
            if (BT_ULL_ROLE_SERVER == ctx->role && BT_ULL_EARBUDS_CLIENT == ctx->client_type) {
                ull_report("[ULL][LE] Check need switch ul, conn handle:0x%x, ul:%d", 2, handle, link_info->ul_active);
                if (!link_info->ul_active) {
                    bt_ull_le_srv_activate_up_link_handler(handle);
                }
            }
        }
    }
}

static void bt_ull_le_srv_set_scenario_type(bt_ull_le_scenario_t scenario_type)
{
   g_ull_scenario_type = scenario_type;
   ull_report("[ULL][LE] bt_ull_le_srv_set_scenario_type, scenario_type: 0x%x", 1, g_ull_scenario_type);
}

static bt_ull_le_scenario_t bt_ull_le_srv_get_scenario_type(void)
{
   ull_report("[ULL][LE] bt_ull_le_srv_get_scenario_type, scenario_type: 0x%x", 1, g_ull_scenario_type);
   return g_ull_scenario_type;
}

/*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: [Client]: set current audio state*/
void bt_ull_le_srv_set_curr_stream_state(bt_ull_le_srv_stream_state_t curr_stream_state)
{
    bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    ull_report("[ULL][LE] set curr_stream_state, state change: 0x%x -> 0x%x", 2, ctx->curr_stream_state, curr_stream_state);
    ctx->curr_stream_state = curr_stream_state;
}

/*bool bt_ull_le_srv_check_inactive_aircis_feature_on(void)
{
#ifdef AIR_ULL_AUDIO_V3_ENABLE //ULL v3 not support this feature
    return false;
#else
	bt_ull_le_srv_context_t *ctx = bt_ull_le_srv_get_context();
    return ctx->aircis_inactive_mode_enable;
#endif
}*/

#endif

