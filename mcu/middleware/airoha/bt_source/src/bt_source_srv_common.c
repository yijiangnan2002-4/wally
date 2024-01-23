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

#include "bt_source_srv_common.h"
#include "bt_source_srv_utils.h"
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
#include "bt_source_srv_call.h"
#endif
#ifdef AIR_SOURCE_SRV_MUSIC_ENABLE
#include "bt_source_srv_avrcp.h"
#include "bt_source_srv_music_pseduo_dev.h"
#endif

#include "audio_src_srv.h"

#define BT_SOURCE_SRV_COMMON_PORT_MAX                          BT_SOURCE_SRV_PORT_I2S_IN_1

#define BT_SOURCE_SRV_COMMON_ACTION_MASK                      0x000000FF

#define BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_PORT            0x0001
#define BT_SOURCE_SRV_COMMON_AUDIO_PORT_RESET_PORT             0x0002
#define BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_SAMPLE_RATE     0x0003
#define BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_SAMPLE_SIZE     0x0004
#define BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_SAMPLE_CHANNEL  0x0005
typedef uint16_t bt_source_srv_common_audio_port_update_t;

typedef bt_status_t (*bt_source_srv_common_action_handler_t)(void *parameter, uint32_t length);

static bt_source_srv_common_audio_port_context_t g_common_port_context[BT_SOURCE_SRV_COMMON_PORT_MAX] = {0};

/* common action handle */
static bt_status_t bt_source_srv_common_handle_audio_port(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_common_handle_audio_sample_rate(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_common_handle_audio_sample_size(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_common_handle_audio_sample_channel(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_common_handle_mute(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_common_handle_unmute(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_common_handle_volume_up(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_common_handle_volume_down(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_common_handle_volume_change(void *parameter, uint32_t length);
static bt_status_t bt_source_srv_common_handle_switch_codec(void *parameter, uint32_t length);

static const bt_source_srv_common_action_handler_t g_handle_common_action_table[] = {
    NULL,
    bt_source_srv_common_handle_audio_port,
    bt_source_srv_common_handle_audio_sample_rate,
    bt_source_srv_common_handle_audio_sample_size,
    bt_source_srv_common_handle_audio_sample_channel,
    bt_source_srv_common_handle_mute,
    bt_source_srv_common_handle_unmute,
    bt_source_srv_common_handle_volume_up,
    bt_source_srv_common_handle_volume_down,
    bt_source_srv_common_handle_volume_change,
    bt_source_srv_common_handle_switch_codec
};

static void bt_source_srv_common_audio_port_reset(bt_source_srv_common_audio_port_context_t *port_context)
{
    bt_source_srv_memset(port_context, 0, sizeof(bt_source_srv_common_audio_port_context_t));
}

static bt_source_srv_common_audio_port_context_t *bt_source_srv_common_audio_get_port_context(bt_source_srv_port_t port)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_COMMON_PORT_MAX; i++) {
        if (g_common_port_context[i].port == port) {
            return &g_common_port_context[i];
        }
    }

    for (i = 0; i < BT_SOURCE_SRV_COMMON_PORT_MAX; i++) {
        if (g_common_port_context[i].port == BT_SOURCE_SRV_PORT_NONE) {
            return &g_common_port_context[i];
        }
    }

    LOG_MSGID_E(source_srv, "[SOURCE][COMMON] get audio port fail by port = %02x", 1, port);
    return NULL;
}

bt_status_t bt_source_srv_common_init(void)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_COMMON_PORT_MAX; i++) {
        bt_source_srv_common_audio_port_reset(&g_common_port_context[i]);
    }
    return BT_STATUS_SUCCESS;
}

bt_source_srv_t bt_source_srv_common_get_playing_device(void)
{
    audio_src_srv_resource_manager_handle_t *audio_src = audio_src_srv_resource_manager_get_current_running_handle(AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE);
    if ((audio_src != NULL) && (bt_source_srv_memcmp(audio_src->handle_name, AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE_USER_HFP, \
                                strlen(AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE_USER_HFP)) == 0)) {
        return BT_SOURCE_SRV_TYPE_HFP;
    }
    return BT_SOURCE_SRV_TYPE_NONE;
}

bool bt_source_srv_common_audio_port_is_valid(bt_source_srv_port_t port)
{
    bt_source_srv_common_audio_port_context_t *port_context = NULL;
    for (uint32_t i = 0; i < BT_SOURCE_SRV_COMMON_PORT_MAX; i++) {
        if (g_common_port_context[i].port == port) {
            port_context = &g_common_port_context[i];
            break;
        }
    }

    if (port_context == NULL) {
        LOG_MSGID_W(source_srv, "[SOURCE][COMMON] audio port = %02x is invalid not find", 1, port);
        return false;
    }

#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
    if ((port_context->port == BT_SOURCE_SRV_PORT_LINE_IN) || (port_context->port == BT_SOURCE_SRV_PORT_I2S_IN) || ((port_context->port == BT_SOURCE_SRV_PORT_I2S_IN_1))) {
        return true;
    }
#endif

    if ((port_context->sample_rate == 0) || (port_context->sample_channel == 0) ||
            (port_context->sample_size == 0)) {
        LOG_MSGID_W(source_srv, "[SOURCE][COMMON] audio port = %02x is invalid", 1, port_context->port);
        return false;
    }
    return true;
}

bt_status_t bt_source_srv_common_audio_find_port_context(bt_source_srv_port_t port, bt_source_srv_common_audio_port_context_t *context)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_COMMON_PORT_MAX; i++) {
        if ((g_common_port_context[i].port == port) && (bt_source_srv_common_audio_port_is_valid(port))) {
            bt_source_srv_memcpy(context, &g_common_port_context[i], sizeof(bt_source_srv_common_audio_port_context_t));
            return BT_STATUS_SUCCESS;
        }
    }

    LOG_MSGID_E(source_srv, "[SOURCE][COMMON] find audio port fail by port = %02x", 1, port);
    return BT_STATUS_FAIL;
}

bt_status_t bt_source_srv_common_audio_port_update(bt_source_srv_common_audio_port_context_t *context, bt_source_srv_common_audio_port_update_t type)
{
    bt_source_srv_common_audio_port_context_t *audio_port_context = NULL;
    bt_source_srv_common_port_action_t common_port_action = BT_SOURCE_SRV_COMMON_PORT_ACTION_NONE;
    bool is_port_parameter_update = false;
    bool is_previous_port_valid = false;

    audio_port_context = bt_source_srv_common_audio_get_port_context(context->port);
    if (audio_port_context == NULL) {
        return BT_STATUS_FAIL;
    }
    LOG_MSGID_I(source_srv, "[SOURCE][COMMON] update type = %02x, audio port = %02x", 2, type, context->port);

    if (bt_source_srv_common_audio_port_is_valid(context->port)) {
        is_previous_port_valid = true;
    }

    switch (type) {
        case BT_SOURCE_SRV_COMMON_AUDIO_PORT_RESET_PORT: {
            bt_source_srv_common_audio_port_reset(audio_port_context);
            common_port_action = BT_SOURCE_SRV_CALL_PORT_ACTION_CLOSE;
        }
        break;
        case BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_SAMPLE_RATE: {
            if (audio_port_context->sample_rate != context->sample_rate) {
                is_port_parameter_update = true;
            }
            audio_port_context->sample_rate = context->sample_rate;
        }
        break;
        case BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_SAMPLE_SIZE: {
            if (audio_port_context->sample_size != context->sample_size) {
                is_port_parameter_update = true;
            }
            audio_port_context->sample_size = context->sample_size;
        }
        break;
        case BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_SAMPLE_CHANNEL: {
            if (audio_port_context->sample_channel != context->sample_channel) {
                is_port_parameter_update = true;
            }
            audio_port_context->sample_channel = context->sample_channel;
        }
        break;
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_PORT: {
            if ((context->port == BT_SOURCE_SRV_PORT_LINE_IN) || (context->port == BT_SOURCE_SRV_PORT_I2S_IN) || (context->port == BT_SOURCE_SRV_PORT_I2S_IN_1)) {
                common_port_action = BT_SOURCE_SRV_COMMON_PORT_ACTION_OPEN;
            }
        }
        break;
#endif
        default:
            break;
    }

    if (type != BT_SOURCE_SRV_COMMON_AUDIO_PORT_RESET_PORT) {
        audio_port_context->port = context->port;
    }

    if (is_port_parameter_update && (bt_source_srv_common_audio_port_is_valid(context->port))) {
        common_port_action = is_previous_port_valid ? BT_SOURCE_SRV_COMMON_PORT_ACTION_UPDATE : BT_SOURCE_SRV_COMMON_PORT_ACTION_OPEN;
    }

    LOG_MSGID_I(source_srv, "[SOURCE][COMMON] audio port update sample rate = %02x, sample size = %02x, sample channel = %02x, common_port_action = %d", 4,
                audio_port_context->sample_rate, audio_port_context->sample_size, audio_port_context->sample_channel, common_port_action);

    if (common_port_action != BT_SOURCE_SRV_COMMON_PORT_ACTION_NONE) {
        /* port open complete and parameter update */
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
        bt_source_srv_call_audio_port_update(context->port, common_port_action);
#endif
#ifdef AIR_SOURCE_SRV_MUSIC_ENABLE
        bt_source_srv_music_audio_port_update(context->port, common_port_action);
#endif
    }

    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_common_handle_audio_port(void *parameter, uint32_t length)
{
    bt_source_srv_audio_port_t *audio_port = (bt_source_srv_audio_port_t *)parameter;

    bt_source_srv_common_audio_port_context_t context = {
        .port = audio_port->port,
    };

    if (audio_port->state == BT_SOURCE_SRV_AUDIO_PORT_STATE_DISABLE) {
        return bt_source_srv_common_audio_port_update(&context, BT_SOURCE_SRV_COMMON_AUDIO_PORT_RESET_PORT);
    }
    return bt_source_srv_common_audio_port_update(&context, BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_PORT);
}

static bt_status_t bt_source_srv_common_handle_audio_sample_rate(void *parameter, uint32_t length)
{
    bt_source_srv_audio_sample_rate_t *audio_sample_rate = (bt_source_srv_audio_sample_rate_t *)parameter;

    bt_source_srv_common_audio_port_context_t context = {
        .port = audio_sample_rate->port,
        .sample_rate = audio_sample_rate->sample_rate
    };
    return bt_source_srv_common_audio_port_update(&context, BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_SAMPLE_RATE);
}

static bt_status_t bt_source_srv_common_handle_audio_sample_size(void *parameter, uint32_t length)
{
    bt_source_srv_audio_sample_size_t *audio_sample_size = (bt_source_srv_audio_sample_size_t *)parameter;

    bt_source_srv_common_audio_port_context_t context = {
        .port = audio_sample_size->port,
        .sample_size = audio_sample_size->sample_size
    };
    return bt_source_srv_common_audio_port_update(&context, BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_SAMPLE_SIZE);
}

static bt_status_t bt_source_srv_common_handle_audio_sample_channel(void *parameter, uint32_t length)
{
    bt_source_srv_audio_sample_channel_t *audio_sample_channel = (bt_source_srv_audio_sample_channel_t *)parameter;

    bt_source_srv_common_audio_port_context_t context = {
        .port = audio_sample_channel->port,
        .sample_channel = audio_sample_channel->channel
    };
    return bt_source_srv_common_audio_port_update(&context, BT_SOURCE_SRV_COMMON_AUDIO_PORT_UPDATE_SAMPLE_CHANNEL);
}

static bt_status_t bt_source_srv_common_handle_mute(void *parameter, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
    status = bt_source_srv_call_send_action(BT_SOURCE_SRV_ACTION_MUTE, parameter, length);
#endif
#ifdef AIR_SOURCE_SRV_MUSIC_ENABLE
    status = bt_source_srv_a2dp_common_action_handler(BT_SOURCE_SRV_ACTION_MUTE, parameter, length);
#endif

    return status;
}

static bt_status_t bt_source_srv_common_handle_unmute(void *parameter, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
    status = bt_source_srv_call_send_action(BT_SOURCE_SRV_ACTION_UNMUTE, parameter, length);
#endif
#ifdef AIR_SOURCE_SRV_MUSIC_ENABLE
    status = bt_source_srv_a2dp_common_action_handler(BT_SOURCE_SRV_ACTION_UNMUTE, parameter, length);
#endif

    return status;
}

static bt_status_t bt_source_srv_common_handle_volume_up(void *parameter, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
    if (bt_source_srv_common_get_playing_device() == BT_SOURCE_SRV_TYPE_HFP) {
        status = bt_source_srv_call_send_action(BT_SOURCE_SRV_ACTION_VOLUME_UP, parameter, length);
        return status;
    }
#endif
    return status;
}

static bt_status_t bt_source_srv_common_handle_volume_down(void *parameter, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
    if (bt_source_srv_common_get_playing_device() == BT_SOURCE_SRV_TYPE_HFP) {
        status = bt_source_srv_call_send_action(BT_SOURCE_SRV_ACTION_VOLUME_DOWN, parameter, length);
        return status;
    }
#endif
    return status;
}

static bt_status_t bt_source_srv_common_handle_volume_change(void *parameter, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;

#ifdef AIR_SOURCE_SRV_HFP_ENABLE
    status = bt_source_srv_call_send_action(BT_SOURCE_SRV_ACTION_VOLUME_CHANGE, parameter, length);
#endif
    if (bt_source_srv_common_get_playing_device() != BT_SOURCE_SRV_TYPE_HFP) {
#ifdef AIR_SOURCE_SRV_MUSIC_ENABLE
        status = bt_source_srv_avrcp_common_action_handler(BT_SOURCE_SRV_ACTION_VOLUME_CHANGE, parameter, length);
#endif
    }
    return status;
}

static bt_status_t bt_source_srv_common_handle_switch_codec(void *parameter, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_source_srv_switch_codec_t *switch_codec = (bt_source_srv_switch_codec_t *)parameter;
        LOG_MSGID_I(source_srv, "[SOURCE][COMMON] common_handle_switch_codec = %02x", 1,switch_codec->type);
    if (switch_codec->type == BT_SOURCE_SRV_TYPE_HFP) {
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
        status = bt_source_srv_call_send_action(BT_SOURCE_SRV_ACTION_SWITCH_CODEC, parameter, length);
#endif
    } else if (switch_codec->type == BT_SOURCE_SRV_TYPE_A2DP) {
#ifdef AIR_SOURCE_SRV_MUSIC_ENABLE
        status = bt_source_srv_a2dp_common_action_handler(BT_SOURCE_SRV_ACTION_SWITCH_CODEC, parameter, length);
#endif
    }

    return status;
}

bt_status_t bt_source_srv_common_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length)
{
    uint8_t action_index = (uint8_t)(action & BT_SOURCE_SRV_COMMON_ACTION_MASK);
    return g_handle_common_action_table[action_index](parameter, length);
}

bt_status_t bt_source_srv_common_audio_get_default_port_parameter(bt_source_srv_port_t port, bt_source_srv_common_audio_port_context_t *context)
{
    if (port == BT_SOURCE_SRV_PORT_CHAT_SPEAKER) {
        context->port = BT_SOURCE_SRV_PORT_CHAT_SPEAKER;
        context->sample_rate = 0xbb80;
        context->sample_size = 0x02;
        context->sample_channel = 0x02;
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_FAIL;
}


bt_status_t bt_source_srv_common_switch_sniff_mode(bt_bd_addr_t *dev_addr, bool is_allow)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_gap_link_policy_setting_t setting = {0};
    bt_cm_link_info_t *link_info = bt_cm_get_link_information(*dev_addr);
    bt_gap_connection_handle_t gap_handle = bt_cm_get_gap_handle(*dev_addr);
    if (link_info == NULL || !gap_handle) {
        LOG_MSGID_I(source_srv, "[SOURCE][COMMON]switch_sniff_mode:error", 0);
        return status;
    }
    LOG_MSGID_I(source_srv, "[SOURCE][COMMON] switch_sniff_mode:gap_handle = %02x, is_allow = %02x, sniff_mode:%02x", 3,  gap_handle, is_allow, link_info->sniff_state);

    if (!is_allow && link_info->sniff_state == BT_GAP_LINK_SNIFF_TYPE_SNIFFED) {
        status = bt_gap_exit_sniff_mode(gap_handle);
    }

    setting.sniff_mode = is_allow ? BT_GAP_LINK_POLICY_ENABLE : BT_GAP_LINK_POLICY_DISABLE;
    status = bt_gap_write_link_policy(gap_handle, &setting);

    return status;
}

#ifdef AIR_FEATURE_SOURCE_MHDT_SUPPORT
typedef struct {
    bt_handle_t handle;
    uint8_t tx_rates;
    uint8_t rx_rates;
} bt_source_tci_mHDT_mode_t;

#define BT_SOURCE_mHDT_RATE_1M      (0x01)
#define BT_SOURCE_mHDT_RATE_2M      (0x02)
#define BT_SOURCE_mHDT_RATE_3M      (0x04)
#define BT_SOURCE_mHDT_RATE_EDR4    (0x08)
#define BT_SOURCE_mHDT_RATE_EDR6    (0x10)
#define BT_SOURCE_mHDT_RATE_EDR8    (0x20)

extern uint8_t bt_gap_get_local_mHDT_feature();
extern uint32_t bt_gap_le_get_local_mHDT_feature(void);
extern uint8_t bt_gap_get_remote_mHDT_feature_by_address(const bt_bd_addr_t *addr);
extern bt_status_t bt_gap_tci_mHDT_mode(bt_source_tci_mHDT_mode_t mHDT_param);
extern bt_status_t bt_gap_tci_exit_mHDT_mode(bt_handle_t handle);
extern bt_handle_t bt_gap_find_connection_handle(const bt_bd_addr_t *address);

static uint8_t bt_source_srv_common_get_mhdt_feature(const bt_bd_addr_t *remote_address)
{
    uint8_t local_mhdt_feature = bt_gap_get_local_mHDT_feature();
    uint8_t remote_mhdt_feature = bt_gap_get_remote_mHDT_feature_by_address(remote_address);

    return (local_mhdt_feature & remote_mhdt_feature);
}

static uint8_t bt_source_srv_common_get_mhdt_optimal_config(const bt_bd_addr_t *remote_address)
{
    uint8_t mdft_feature = bt_source_srv_common_get_mhdt_feature(remote_address);

    LOG_MSGID_I(source_srv, "[SOURCE][COMMON] switch mhdt get mhdt feature = %02x", 1, mdft_feature);
    for (int32_t i = 2; i >= 0; i--) {
        if ((mdft_feature & (1 << i)) > 0) {
            return (mdft_feature & (1 << i));
        }
    }
    return 0;
}

bt_status_t bt_source_srv_common_switch_mhdt(bt_bd_addr_t *remote_address, bool is_enable)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_handle_t gap_handle = bt_gap_find_connection_handle(remote_address);
    uint8_t mhdt_optimal_config = bt_source_srv_common_get_mhdt_optimal_config(remote_address);

    if (mhdt_optimal_config != 0) {
        if (is_enable) {
            bt_source_tci_mHDT_mode_t mhdt_mode = {
                .handle = gap_handle,
                .rx_rates = BT_SOURCE_mHDT_RATE_2M,
                .tx_rates = (mhdt_optimal_config << 3),
            };
            status = bt_gap_tci_mHDT_mode(mhdt_mode);
        } else {
            status = bt_gap_tci_exit_mHDT_mode(gap_handle);
        }
    }
    LOG_MSGID_I(source_srv, "[SOURCE][COMMON] switch mhdt address = %02x:%02x:%02x:%02x:%02x:%02x enable = %02x handle = %02x status = %02x", 9,
                (*remote_address)[0],  (*remote_address)[1], (*remote_address)[2],  (*remote_address)[3], (*remote_address)[4],  (*remote_address)[5],
                is_enable, gap_handle, status);
    return status;
}
#endif