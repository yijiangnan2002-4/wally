/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#include "bt_ull_le_hid_service.h"
#include "bt_callback_manager.h"
#include "bt_l2cap_fix_channel.h"
#include "bt_ull_le_utility.h"
#include "bt_ull_utility.h"
#include "bt_avm.h"
#include "FreeRTOS.h"
//#include "timers.h"
#include "bt_timer_external.h"

#include "assert.h"
#include "apps_events_event_group.h"
#include "bt_ull_le_audio_transmitter.h"
#include "bt_ull_le_audio_manager.h"
#include "bt_timer_external.h"
#include "bt_ull_service.h"
#include "bt_ull_le_hid_conn_service.h"
#include "bt_gap_le.h"
#include "bt_ull_le_hid_utility.h"
#ifdef MTK_PORT_SERVICE_BT_ENABLE
#include "ble_air_interface.h"
#endif
#define BT_ULL_LE_HID_SRV_FIX_CHANNEL_CID          0x0102
#define BT_ULL_LE_HID_SRV_FIX_CHANNEL_MTU          1000

#define BT_ULL_LE_HID_SRV_TIMER_MAX                0x0A
#define BT_ULL_LE_HID_SRV_INVALID_VALUE            0xFF

/*
#define BT_ULL_LE_HID_SRV_MSG_SERVICE_CONNECT_REQ_TIMER    0x01
#define BT_ULL_LE_HID_SRV_MSG_SYNC_CONTEXT_INFO_TIMER      0x02
#define BT_ULL_LE_HID_SRV_MSG_BONDING_REQ_TIMER    0x03
#define BT_ULL_LE_HID_SRV_MSG_SYNC_BONDING_INFO_TIMER   0x04
*/

#define BT_ULL_LE_HID_SRV_TIMEOUT                  1500

#define BT_ULL_HID_LOG     "[ULL][LE][HID] "

#define MAKE_TIMER_ID(HANDLE, TIMER) (uint32_t)((((uint32_t) HANDLE) << 8) | TIMER)

typedef uint8_t bt_ull_le_hid_srv_link_state_t;
#define BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTED                 0x00
#define BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTING               0x01
#define BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTED                0x02
#define BT_ULL_LE_HID_SRV_LINK_STATE_WAITING_PSEUDO_ACL_CONNECTED   0x03
#define BT_ULL_LE_HID_SRV_LINK_STATE_ACL_CONNECTED                0x04
#define BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED        0x05
#define BT_ULL_LE_HID_SRV_LINK_STATE_STREAMING                    0x06
#define BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTING                0x07


/**
 *  @brief Define the state of ULL HID dongle audio stream.
 */
typedef uint8_t bt_ull_le_hid_srv_stream_state_t;
#define BT_ULL_LE_HID_SRV_STREAM_STATE_IDLE                            0x00
#define BT_ULL_LE_HID_SRV_STREAM_STATE_START_AUDIO_STREAM              0x01 /**< Audio Stream is Starting.*/
#define BT_ULL_LE_HID_SRV_STREAM_STATE_STREAMING                       0x02 /**< Sreaming. */
#define BT_ULL_LE_HID_SRV_STREAM_STATE_STOP_AUDIO_STREAM               0x03 /**< Audio Stream is Stoping.*/
#define BT_ULL_LE_HID_SRV_STREAM_STATE_MAX                             0x04



#define BT_ULL_LE_HID_SRV_STATE_NONE                              0x00
typedef uint8_t bt_ull_le_hid_srv_state_t;

typedef struct {
    bt_ull_le_hid_srv_module_t         module;
    bt_ull_callback                    callback;
} bt_ull_le_hid_srv_callback_t;


bt_ull_le_hid_srv_context_t g_bt_ull_hid_le_ctx;
static bt_ull_le_hid_srv_callback_t      g_bt_ull_le_hid_cb[BT_ULL_LE_HID_SRV_MODULE_MAX] = {0};
static uint8_t g_hs_uni_aa[BT_ULL_LE_HID_DM_UNI_AA_LEN] = {0x6D, 0xEB, 0x98, 0xE9};
static uint8_t g_kb_uni_aa[BT_ULL_LE_HID_DM_UNI_AA_LEN] = {0x6D, 0xEB, 0x98, 0xE9};
static uint8_t g_ms_uni_aa[BT_ULL_LE_HID_DM_UNI_AA_LEN] = {0x6D, 0xEB, 0x98, 0xE9};

/**************************************************************************************************
* Prototype
**************************************************************************************************/
static bt_status_t bt_ull_le_hid_srv_send_data_by_device_type(bt_ull_le_hid_srv_device_t device_type, uint8_t *packet, uint16_t packet_size);
static bool bt_ull_le_hid_srv_is_device_connected(bt_ull_le_hid_srv_device_t dt);
static bt_status_t bt_ull_le_hid_srv_handle_start_streaming(bt_ull_streaming_t *streaming);
static bt_status_t bt_ull_le_hid_srv_handle_stop_streaming(bt_ull_streaming_t *streaming);
static bt_status_t bt_ull_le_hid_srv_handle_set_volume(bt_ull_volume_t *vol);
static bt_status_t bt_ull_le_hid_srv_handle_set_streaming_mute(bt_ull_streaming_t *streaming, bool is_mute);
static bt_status_t bt_ull_le_hid_srv_handle_set_streaming_sample_rate(bt_ull_sample_rate_t *sample_rate);
static bt_status_t bt_ull_le_hid_srv_handle_set_streaming_sample_size(bt_ull_streaming_sample_size_t *sample_size);
static bt_status_t bt_ull_le_hid_srv_handle_set_streaming_sample_channel(bt_ull_streaming_sample_channel_t *sample_channel);
static void bt_ull_le_hid_srv_handle_audio_transmitter_event(bt_ull_le_at_event_t event, void *param, uint32_t param_len);
static void bt_ull_le_hid_srv_handle_audio_manager_event(bt_ull_le_am_event_t event, void *data, uint32_t data_len);
static void bt_ull_le_hid_srv_start_audio_transmitter_callback(bt_ull_transmitter_t transmitter_type, bt_status_t status);
static void bt_ull_le_hid_srv_stop_audio_transmitter_callback(bt_ull_transmitter_t transmitter_type, bt_status_t status);
static void bt_ull_le_hid_srv_play_am_callback(bt_ull_le_am_mode_t mode, bt_status_t status);
static void bt_ull_le_hid_srv_stop_am_callback(bt_ull_le_am_mode_t mode, bt_status_t status);
static bt_status_t bt_ull_le_hid_srv_open_audio_stream(bt_ull_le_stream_port_mask_t stream_port);
static bt_status_t bt_ull_le_hid_srv_sync_streaming_status(bt_ull_le_stream_port_mask_t stream_port, uint8_t event);
static bt_status_t bt_ull_le_hid_srv_play_am(bt_ull_streaming_t *streaming, bt_ull_le_stream_mode_t stream_mode);
static bt_status_t bt_ull_le_hid_srv_stop_am(bt_ull_streaming_t *streaming);
static bt_status_t bt_ull_le_hid_srv_tx_critical_user_data(bt_ull_tx_critical_user_data_t *tx_data);//only for headset or earbuds
static void bt_ull_le_hid_srv_notify_event(uint8_t module, bt_ull_event_t event, void *param, uint32_t len);
static bt_status_t bt_ull_le_hid_srv_l2cap_event_cb(bt_l2cap_fix_channel_event_t event_id, bt_status_t status, void *buffer);
static bt_status_t bt_ull_le_hid_srv_conn_event_cb(bt_ull_le_hid_conn_srv_msg_t msg, void *data);
static bt_status_t bt_ull_le_hid_srv_host_event_cb(bt_msg_type_t msg, bt_status_t status, void *buff);
static void bt_ull_le_hid_srv_l2cap_rx_hdl(bt_handle_t handle, uint8_t *data, uint16_t len) ;
static bt_status_t bt_ull_le_hid_srv_handle_set_streaming_mix_ratio_hdl(bt_ull_mix_ratio_t *ratio);
static bt_status_t bt_ull_le_hid_srv_handle_usb_audio_control(bt_ull_usb_hid_control_t action);
static bt_status_t bt_ull_le_hid_srv_led_control_hdl(bt_ull_le_hid_srv_control_indicater_led_t *led_control);
static bt_status_t bt_ull_le_hid_srv_create_connection_req(bt_ull_le_hid_srv_connect_t *create_conn);

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

void bt_ull_le_hid_srv_print_addr(bt_addr_t *addr)
{
    if (!addr) {
         return;
    }
    ull_report(BT_ULL_HID_LOG" PRINT BT ADDR: addr_type: %d, addr: %x-%x-%x-%x-%x-%x!", 7,
        addr->type,
        addr->addr[0],
        addr->addr[1],
        addr->addr[2],
        addr->addr[3],
        addr->addr[4],
        addr->addr[5]
        );
}

bt_status_t bt_ull_le_hid_srv_start_timer(uint32_t timer_id, uint32_t timer_period, bt_timer_ext_timeout_callback_t callback, uint32_t data)
{

    bt_timer_ext_status_t t_status = bt_timer_ext_start(timer_id, data, timer_period, callback);
    return (BT_TIMER_EXT_STATUS_SUCCESS == t_status) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

void bt_ull_le_hid_srv_reset_timer(uint32_t timer_id, uint32_t tm_ms)
{
    bt_timer_ext_t *timer = bt_timer_ext_find(timer_id);
    if (timer) {
        uint32_t data = timer->data;
        bt_timer_ext_timeout_callback_t cb = timer->cb;
        bt_timer_ext_stop(timer_id);
        bt_timer_ext_start(timer_id, data, tm_ms, cb);
    }
}

bt_status_t bt_ull_le_hid_srv_stop_timer(uint32_t timer_id)
{
    bt_timer_ext_status_t t_status = bt_timer_ext_stop(timer_id);
    return (BT_TIMER_EXT_STATUS_SUCCESS == t_status) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}
static void bt_ull_le_hid_srv_timer_timeout_handler(uint32_t timer_id, uint32_t data)
{
    //bt_status_t status = BT_STATUS_SUCCESS;
    ull_report_error(BT_ULL_HID_LOG"Timer Time out, timer id: %d", 1, timer_id);
    switch (timer_id) {
        case BT_ULL_LE_HID_SERVICE_CONNECT_TIMER_ID: {
            break;
        }
        case BT_ULL_LE_HID_SYNC_CONTEXT_INFO_TIMER_ID: {
            break;
        }
        case BT_ULL_LE_HID_BONDING_REQ_TIMER_ID: {
            break;
        }
        case BT_ULL_LE_HID_SYNC_BONDING_INFO_TIMER_ID: {
            break;
        }
        default:
            break;
    }
}

static bt_ull_le_hid_srv_context_t * bt_ull_le_hid_srv_get_ctx(void)
{
    return &g_bt_ull_hid_le_ctx;
}

bt_ull_role_t bt_ull_le_hid_srv_get_role(void)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    return ctx->role;
}
static uint8_t bt_ull_le_hid_srv_get_link_idx_by_handle(bt_handle_t handle)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    uint8_t i = 0;
    for (i =0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i ++) {
        if (ctx->link[i].acl_handle == handle) {
            return i;
        }
    }
    ull_report_error(BT_ULL_HID_LOG"not found handle, handle: %x!!", 1, handle);
    return BT_ULL_LE_HID_SRV_INVALID_VALUE;
}

static uint8_t bt_ull_le_hid_srv_get_link_idx_by_addr(bt_addr_t *addr)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    uint8_t i = 0;
    for (i =0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i ++) {
        if (!bt_ull_le_srv_memcmp(&ctx->link[i].peer_addr, addr, sizeof(bt_addr_t))) {
            return i;
        }
    }
    return BT_ULL_LE_HID_SRV_INVALID_VALUE;
}

static uint8_t bt_ull_le_hid_srv_get_empty_link(void)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    uint8_t i = 0;
    for (i =0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i ++) {
        if (ctx->link[i].state == BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTED) {
            return i;
        }
    }
    return BT_ULL_LE_HID_SRV_INVALID_VALUE;
}
static uint8_t  bt_ull_le_hid_srv_get_link_idx_by_dt(uint8_t device_type)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    uint8_t i = 0;
    for (i =0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i ++) {
        if (ctx->link[i].device_type == device_type) {
            return i;
        }
    }
    return BT_ULL_LE_HID_SRV_INVALID_VALUE;
}
/*
static bt_handle_t  bt_ull_le_hid_srv_get_acl_handle(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return BT_HANDLE_INVALID;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    return ctx->link[idx].acl_handle;
}
*/
static void bt_ull_le_hid_srv_clear_link(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_memset(&ctx->link[idx], 0, sizeof(bt_ull_le_hid_srv_link_info_t));
}

static void bt_ull_le_hid_srv_set_link_state(uint8_t idx, uint8_t state)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ull_report(BT_ULL_HID_LOG"Set link[%d] state: %d->%d!", 3, idx, ctx->link[idx].state, state);
    ctx->link[idx].state = state;
}

static uint8_t bt_ull_le_hid_srv_get_link_state(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return BT_ULL_LE_HID_SRV_INVALID_VALUE;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    return ctx->link[idx].state;
}

static void bt_ull_le_hid_srv_set_link_handle(uint8_t idx, bt_handle_t handle)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ctx->link[idx].acl_handle = handle;
}

static bt_handle_t bt_ull_le_hid_srv_get_link_handle(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return BT_HANDLE_INVALID;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    return ctx->link[idx].acl_handle;
}

static void bt_ull_le_hid_srv_set_device_type(uint8_t idx, bt_ull_le_hid_srv_device_t dt)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ctx->link[idx].device_type = dt;
}

static void bt_ull_le_hid_srv_set_link_mode(uint8_t idx, uint8_t mode)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ctx->link[idx].mode = mode;
}

static uint8_t bt_ull_le_hid_srv_get_link_mode(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return BT_ULL_LE_HID_SRV_INVALID_VALUE;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    return ctx->link[idx].mode;
}

static void bt_ull_le_hid_srv_set_rx_att_handle(uint8_t idx, uint16_t rx_att_handle)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ctx->link[idx].att_handle_rx = rx_att_handle;
}

static void bt_ull_le_hid_srv_set_tx_att_handle(uint8_t idx, uint16_t tx_att_handle)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ctx->link[idx].att_handle_tx = tx_att_handle;
}

static void bt_ull_le_hid_srv_set_cccd_att_handle(uint8_t idx, uint16_t cccd_att_handle)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ctx->link[idx].att_handle_cccd = cccd_att_handle;
}

static uint8_t bt_ull_le_hid_srv_get_device_type_by_idx(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM) {
        return BT_ULL_LE_HID_SRV_DEVICE_NONE;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    return ctx->link[idx].device_type;
}

static void bt_ull_le_hid_srv_set_link_addr(uint8_t idx, bt_addr_t *addr)
{
    if (idx >= BT_ULL_LE_HID_SRV_MAX_LINK_NUM || !addr) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_memcpy(&ctx->link[idx].peer_addr, addr, sizeof(bt_addr_t));
}

static bt_status_t bt_ull_le_hid_srv_l2cap_tx(bt_handle_t handle, uint8_t *data, uint16_t len)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_l2cap_fix_channel_tx_data_t tx_data;
    tx_data.type = BT_L2CAP_FIX_CHANNEL_BLE;
    tx_data.connection_handle = handle;
    tx_data.cid = BT_ULL_LE_HID_SRV_FIX_CHANNEL_CID;
    tx_data.data = data;
    tx_data.length = len;

    status = bt_l2cap_fix_channel_send(&tx_data);
    if (BT_STATUS_OUT_OF_MEMORY == status) {
        ull_assert(0 && "send data fail because of OOM!");
    }
    return status;
}

static void bt_ull_le_hid_srv_critial_data_timeout_callback(uint32_t timer_id, uint32_t data)
{
    ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_critial_data_timeout_callback, timer_id: 0x%x, data:0x%x", 2, timer_id, data);
    if (BT_ULL_CRITICAL_RX_IND_TIMER_ID == timer_id) {
        uint8_t* data_ind = (uint8_t*)data;
        bt_ull_rx_critical_user_data_t rx_data = {0};
        rx_data.user_data_length = data_ind[0];
        rx_data.user_data = &data_ind[1];
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_RX_CRITICAL_USER_DATA_IND, &rx_data, sizeof(rx_data));
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
            bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_TX_CRITICAL_USER_DATA_RESULT, &tx_result, sizeof(tx_result));
        }
    }
}

static void bt_ull_le_hid_srv_critial_data_controller_cb(bt_avm_critial_data_event_t event, void* data)
{
    uint32_t timer_data = 0;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();

    if (BT_AVM_CIRTICAL_DATA_RX_IND == event) {
        bt_avm_critial_data_t *rx_ind = (bt_avm_critial_data_t*)data;
        //ull_report("[ULL][AVM] bt_ull_critial_data rx ind, seq: 0x%x, len:0x%x, cur_seq:0x%x", 3, rx_ind->seq, rx_ind->length, ctx->critical_data_rx_seq);
        if (ctx->audio_sink.critical_data_rx_seq != rx_ind->seq) {  /* filter duplicate packet */
            uint16_t total_len = rx_ind->length+ sizeof(rx_ind->length);
            uint8_t *data_ind = (uint8_t*)bt_ull_le_srv_memory_alloc(total_len);
            if (NULL != data_ind) {
                data_ind[0] = rx_ind->length;
                bt_ull_le_srv_memcpy(&data_ind[1], rx_ind->data, rx_ind->length);
                timer_data = (uint32_t)data_ind;
                bt_timer_ext_status_t status = bt_timer_ext_start(BT_ULL_CRITICAL_RX_IND_TIMER_ID, timer_data, 1, bt_ull_le_hid_srv_critial_data_timeout_callback);
                if (BT_TIMER_EXT_STATUS_SUCCESS != status) {
                    ull_report_error(BT_ULL_HID_LOG"bt_ull_le_hid_srv_critial_data_controller_cb rx ind, start timer fail !!!", 0);
                    bt_ull_le_srv_memory_free(data_ind);
                }
            }
            /* for debug log */
            uint8_t temp_seq = ctx->audio_sink.critical_data_rx_seq;
            if (0xFF == temp_seq) {
                temp_seq = 0x01;
            } else {
                temp_seq++;
            }
            if (temp_seq != rx_ind->seq) {
                ull_report_error(BT_ULL_HID_LOG"bt_ull_le_hid_srv_critial_data_controller_cb rx lost!!!, seq: 0x%x, cur_seq:0x%x", 2, rx_ind->seq, ctx->audio_sink.critical_data_rx_seq);
            }
        } else {
            ull_report_error(BT_ULL_HID_LOG"bt_ull_le_hid_srv_critial_data_controller_cb rx same data, seq: 0x%x, len:0x%x, cur_seq:0x%x", 3, rx_ind->seq, rx_ind->length, ctx->audio_sink.critical_data_rx_seq);
        }
        ctx->audio_sink.critical_data_rx_seq = rx_ind->seq;
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
            ull_report_error(BT_ULL_HID_LOG"bt_ull_le_srv_critial_data_controller_cb tx result, status: 0x%x, seq:0x%x", 2, result->status, result->seq);
        }
        bt_timer_ext_start(BT_ULL_CRITICAL_TX_RESULT_TIMER_ID, timer_data, 1, bt_ull_le_hid_srv_critial_data_timeout_callback);
    }
}

static bt_status_t bt_ull_le_hid_srv_critial_data_init(bt_ull_init_critical_user_data_t *init)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    if (!init) {
        return BT_STATUS_FAIL;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    if (0 == ctx->audio_sink.critical_data_max_len) {
        if (BT_STATUS_SUCCESS != bt_avm_critical_data_init(init->max_user_data_length, (void*)bt_ull_le_hid_srv_critial_data_controller_cb)) {
            ret = BT_STATUS_FAIL;
        } else {
            ctx->audio_sink.critical_data_max_len = init->max_user_data_length;
            ctx->audio_sink.critical_data_tx_seq = 0x01;  /* valid seq: 1 ~ 255 */
            ctx->audio_sink.critical_data_rx_seq = 0x00;  /* valid seq: 1 ~ 255 */
        }
        ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_critial_data_init, max_len:0x%x,ret: 0x%x", 2, init->max_user_data_length, ret);
    } else {
        ret = BT_STATUS_FAIL;
        ull_report_error(BT_ULL_HID_LOG"bt_ull_le_hid_srv_critial_data_init, fail due to was inited, max_len:0x%x", 1, ctx->audio_sink.critical_data_max_len);
    }
    return ret;
}

static bt_status_t bt_ull_le_hid_srv_critial_data_send(bt_handle_t handle, uint16_t flush_timeout, bt_avm_critial_data_t* data)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();

    if (0 != ctx->audio_sink.critical_data_max_len) {
        ull_assert(data && (data->length <= ctx->audio_sink.critical_data_max_len));
        ret = bt_avm_critical_data_send(handle, flush_timeout, data);
        ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_critial_data_send, flush_timeout:%d, seq:0x%x, gap_handle:0x%x, result:0x%x", 4 ,flush_timeout, data->seq, handle, ret);
    } else {
        ret = BT_STATUS_FAIL;
        ull_report_error(BT_ULL_HID_LOG"bt_ull_le_hid_srv_critial_data_send fail due to not inited", 0);
    }
    return ret;
}

static void bt_ull_le_hid_srv_notify_event(uint8_t module, bt_ull_event_t event, void *param, uint32_t len)
{
    ull_report(BT_ULL_HID_LOG"notify_event, module: %d, event: %x", 2, module, event);
    if (BT_ULL_LE_HID_SRV_MODULE_MAX <= module) {
        return;
    }
    if (g_bt_ull_le_hid_cb[module].module == module && g_bt_ull_le_hid_cb[module].callback) {
        g_bt_ull_le_hid_cb[module].callback(event, param, len);
    }
}
bt_status_t bt_ull_le_hid_srv_set_device_info(bt_ull_le_device_info_t *dev_info)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    if (!dev_info) {
        return BT_STATUS_FAIL;
    }
    if (BT_ULL_UNKNOWN_CLIENT == dev_info->client_type) {
        ull_report_error(BT_ULL_HID_LOG"bt_ull_le_hid_srv_set_device_info fail!", 0);
        return BT_STATUS_FAIL;
    }
    ctx->audio_sink.client_type = dev_info->client_type;
    ctx->audio_sink.local_random_addr.type = BT_ADDR_RANDOM;
    bt_ull_le_srv_memcpy(&ctx->audio_sink.local_random_addr.addr, &dev_info->local_random_addr, sizeof(bt_bd_addr_t));
    ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_set_device_info success, client_type: %d, size: %d, addr: %x-%x-%x-%x-%x-%x", 8, \
        dev_info->client_type, dev_info->size, dev_info->local_random_addr[0], dev_info->local_random_addr[1], dev_info->local_random_addr[2], 
        dev_info->local_random_addr[3], dev_info->local_random_addr[4], dev_info->local_random_addr[5]);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_ull_le_hid_srv_l2cap_send_data(uint16_t handle, uint8_t *packet, uint16_t packet_size)
{
    bt_status_t result = BT_STATUS_FAIL;
    uint16_t send_length = 0;
    bool need_resend = false;
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(handle);

    if (BT_HANDLE_INVALID == handle || BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"bt_ull_le_hid_srv_l2cap_send_data, invalid params", 0);
        return BT_STATUS_FAIL;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    if (BT_ULL_LE_HID_SRV_LINK_STATE_ACL_CONNECTED > state) {
        ull_report_error(BT_ULL_HID_LOG"bt_ull_le_hid_srv_l2cap_send_data, error state: %d", 1, state);
        return BT_STATUS_FAIL;
    }
    if (BT_ULL_LE_HID_SRV_FIX_CHANNEL_MTU < packet_size) {
        need_resend = true;
        send_length = BT_ULL_LE_HID_SRV_FIX_CHANNEL_MTU;
        result = bt_ull_le_hid_srv_l2cap_tx(handle, packet, send_length);
        if ((BT_STATUS_SUCCESS == result) && (need_resend)) {
            need_resend = false;
            result = bt_ull_le_hid_srv_l2cap_send_data(handle, (uint8_t *)(packet + send_length), (packet_size - send_length));
        }
        ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_l2cap_send_data, packet_size: %d, send size: %d, status: %d, need_resend: %d", 4,
            packet_size, send_length, result, need_resend);
    } else {
        send_length = packet_size;
        result = bt_ull_le_hid_srv_l2cap_tx(handle, packet, packet_size);
    }
    ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_l2cap_send_data, status: 0x%x, handle: 0x%x, packet_len: 0x%x, send_len: 0x%x", 4, result, handle, packet_size, send_length);
    return result;

}

static bt_status_t bt_ull_le_hid_srv_l2cap_init(void)
{
    bt_status_t status = bt_l2cap_fix_channel_register(BT_L2CAP_FIX_CHANNEL_BLE, BT_ULL_LE_HID_SRV_FIX_CHANNEL_CID, bt_ull_le_hid_srv_l2cap_event_cb);
    ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_l2cap_init, result: 0x%x", 1, status);
    return status;

}
bt_status_t bt_ull_le_hid_srv_register_callback(bt_ull_le_hid_srv_module_t module, bt_ull_callback cb)
{
    ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_register_callback, module: 0x%x, 0x%x, 0x%x", 3, module, g_bt_ull_le_hid_cb[module].callback, cb);
    if (g_bt_ull_le_hid_cb[module].callback == NULL) {
        g_bt_ull_le_hid_cb[module].callback = cb;
        g_bt_ull_le_hid_cb[module].module = module;
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_FAIL;
}

bt_status_t bt_ull_le_hid_srv_init(bt_ull_role_t role, bt_ull_callback cb)
{
    bt_ull_le_srv_memset(&g_bt_ull_hid_le_ctx, 0, sizeof(bt_ull_le_hid_srv_context_t));
    //bt_ull_le_srv_memset(&g_bt_ull_le_hid_cb[0], 0, sizeof(bt_ull_le_hid_srv_callback_t) * BT_ULL_LE_HID_SRV_MODULE_MAX);
    if (!cb) {
        return BT_STATUS_FAIL;
    }
    if (BT_ULL_ROLE_SERVER == role) {
        bt_ull_le_at_init_ctx((void *)bt_ull_le_hid_srv_handle_audio_transmitter_event);
    } else if (BT_ULL_ROLE_CLIENT == role) {
        bt_ull_le_am_init((void *)bt_ull_le_hid_srv_handle_audio_manager_event);
    } else {
        return BT_STATUS_FAIL;
    }
    /**< Init L2CAP  LE Fixed Channel.*/
    if (BT_STATUS_SUCCESS != bt_ull_le_hid_srv_l2cap_init()) {
        ull_report_error(BT_ULL_HID_LOG"bt_ull_le_hid_srv_l2cap_init fail", 0);
        return BT_STATUS_FAIL;
    }
    if (BT_STATUS_SUCCESS != bt_ull_le_hid_dm_init()) {
        ull_report_error(BT_ULL_HID_LOG"bt_ull_le_hid_dm_init fail-2", 0);
        return BT_STATUS_FAIL;
    }
    g_bt_ull_hid_le_ctx.role = role;
    g_bt_ull_hid_le_ctx.scenario = BT_ULL_LE_HID_SRV_APP_SCENARIO_1;
    bt_ull_le_hid_conn_srv_init(role, (void *)bt_ull_le_hid_srv_conn_event_cb);

    /**update client codec param*/
    if (BT_ULL_ROLE_CLIENT == role) {
        bt_ull_le_srv_client_preferred_codec_param *codec_param = &(g_bt_ull_hid_le_ctx.audio_sink.client_preferred_codec_param);
        bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
        codec_param->codec_type = bt_ull_le_srv_get_client_preffered_codec(BT_ULL_LE_SCENARIO_ULLV2_0);
        bt_ull_le_srv_set_client_preferred_codec_type(codec_param->codec_type);
        codec_param->dl_samplerate = bt_ull_le_srv_get_client_preffered_dl_codec_samplerate(BT_ULL_LE_SCENARIO_ULLV2_0);
        codec_param->ul_samplerate = bt_ull_le_srv_get_client_preffered_ul_codec_samplerate(BT_ULL_LE_SCENARIO_ULLV2_0);
        stream_ctx->codec_type = codec_param->codec_type;
        bt_ull_le_srv_set_codec_param_by_sample_rate(role, codec_param->dl_samplerate, codec_param->ul_samplerate);
    } else {
        uint8_t num = bt_ull_le_hid_dm_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
        if (num) {
            bt_addr_t addr = {0};
            bt_ull_le_hid_dm_get_bonded_device_list(BT_ULL_LE_HID_SRV_DEVICE_HEADSET, 1, &addr);
            bt_ull_le_hid_dm_device_info_t *device = bt_ull_le_hid_dm_read_device_info(BT_ULL_LE_HID_SRV_DEVICE_HEADSET, &addr);
            if (device) {
                bt_ull_le_srv_set_codec_param_by_sample_rate(BT_ULL_ROLE_SERVER, device->codec_param.dl_samplerate, device->codec_param.ul_samplerate);
            }
        } else {
            /**< Reset Link info & Stream ctx.*/
            bt_ull_le_srv_reset_stream_ctx();
        }
    }
    bt_ull_le_hid_srv_register_callback(BT_ULL_LE_HID_SRV_MODULE_APP, cb);
    /* Register Host callback to ull sniff mode change notification */
    bt_status_t status = bt_callback_manager_register_callback(bt_callback_type_app_event,
    (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_SYSTEM),
    (void *)bt_ull_le_hid_srv_host_event_cb);
    ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_init, role: %d, status: 0x%x", 2, role, status);
    return status;
}

static void bt_ull_le_hid_srv_cis_connected_ind_hdl(bt_ull_le_hid_conn_srv_msg_ind_t *ind)
{
    //bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    //bt_status_t ret;
    if (!ind ) {
        return;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_dt(ind->cis_connected.device_type);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"cis_connected_ind_hdl, not found link!!", 0);
        return;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    ull_report(BT_ULL_HID_LOG"CIS CONNECTED, status: %d, link state: %x, dt: %d", 3, ind->status, state, ind->cis_connected.device_type);
    if (BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTING != state) {
        ull_report_error(BT_ULL_HID_LOG"cis_connected_ind_hdl, state error!!", 0);
        return;
    }
    if (BT_STATUS_SUCCESS != ind->status) {
        ull_report_error(BT_ULL_HID_LOG"cis_connected_ind_hdl, status error!!", 0);
        bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTED);
        bt_ull_le_hid_srv_set_device_type(idx, BT_ULL_LE_HID_SRV_DEVICE_NONE);
        bt_ull_le_hid_srv_connected_ind_t connection_complete;
        connection_complete.status = ind->status;
        connection_complete.handle = 0xFFFF;
        connection_complete.device_type = ind->cis_connected.device_type;
        bt_ull_le_srv_memcpy(&connection_complete.peer_addr, &ind->cis_connected.peer_addr, sizeof(bt_addr_t));
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_CONNECTED_IND, &connection_complete, sizeof(bt_ull_le_hid_srv_connected_ind_t));
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_AIR_SRV, BT_ULL_EVENT_LE_HID_CONNECTED_IND, &connection_complete, sizeof(bt_ull_le_hid_srv_connected_ind_t));
        return;
    }

    bt_ull_le_hid_srv_set_link_handle(idx, ind->cis_connected.acl_handle);
    bt_ull_le_hid_srv_set_link_addr(idx, &ind->cis_connected.peer_addr);
    bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTED);
    bt_ull_le_hid_srv_set_link_mode(idx, BT_ULL_LE_HID_SRV_LINK_MODE_NORMAL);
    /*waite for the pseudo le link is connected*/
    bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_WAITING_PSEUDO_ACL_CONNECTED);
/*
    if (ctx->role == BT_ULL_ROLE_CLIENT) {
        return;
    }

    bt_hci_cmd_le_create_connection_t param = {
        .le_scan_interval = 0x10,
        .le_scan_window = 0x10,
        .initiator_filter_policy = BT_HCI_CONN_FILTER_ASSIGNED_ADDRESS,
        .own_address_type = BT_ADDR_RANDOM,
        .conn_interval_min = 0x0018,
        .conn_interval_max = 0x0018,
        .conn_latency = 0x0000,
        .supervision_timeout = 0x01F4,
        .minimum_ce_length = 0x0002,
        .maximum_ce_length = 0x0002,
    };
    bt_ull_le_srv_memcpy(&(param.peer_address), &ind->cis_connected.peer_addr, sizeof(bt_addr_t));
    ull_report(BT_ULL_HID_LOG"ull_le_hid_connect, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      param.peer_address.type,
                      param.peer_address.addr[5],
                      param.peer_address.addr[4],
                      param.peer_address.addr[3],
                      param.peer_address.addr[2],
                      param.peer_address.addr[1],
                      param.peer_address.addr[0]);
    
    ret = bt_gap_le_connect(&param);
    ull_report(BT_ULL_HID_LOG"connect_device, status is 0x%4x", 1, ret);
    if (BT_STATUS_SUCCESS != ret) {
        bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTED);
    }
*/
}

static void bt_ull_le_hid_srv_pseudo_link_connected_ind_hdl(bt_status_t status, bt_gap_le_connection_ind_t *ind)
{
   if (!ind) {
       return;
   }
   if (BT_STATUS_SUCCESS != status || BT_HANDLE_INVALID == ind->connection_handle) {
       ull_report_error(BT_ULL_HID_LOG"pseudo_link_connected_ind_hdl, status fail!!", 0);
       return;
   }

   uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(ind->connection_handle);
   if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
       ull_report_error(BT_ULL_HID_LOG"pseudo_link_connected_ind_hdl, not found link!!", 0);
       return;
   }
   uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
   uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
   ull_report(BT_ULL_HID_LOG"PSEUDO ACL CONNECTED, status: %d, link state: %x, dt: %d, handle: %x", 4, status, state, device_type, ind->connection_handle);

   if (BT_STATUS_SUCCESS != status) {
       if (BT_ULL_LE_HID_SRV_LINK_STATE_WAITING_PSEUDO_ACL_CONNECTED == state) {
           bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTED);
       }
       return;
   }

   if (BT_ULL_LE_HID_SRV_LINK_STATE_WAITING_PSEUDO_ACL_CONNECTED != state ) {
       ull_report_error(BT_ULL_HID_LOG"pseudo_acl_connected_ind_hdl, state error!!", 0);
       return;
   }
   bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_ACL_CONNECTED);
   bt_ull_le_hid_srv_connected_ind_t connection_complete;
   connection_complete.status = status;
   connection_complete.handle = ind->connection_handle;
   connection_complete.device_type = device_type;
   bt_ull_le_srv_memcpy(&connection_complete.peer_addr, &ind->peer_addr, sizeof(bt_addr_t));
   bt_ull_le_hid_dm_shift_device_front(device_type, &ind->peer_addr);
   bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_CONNECTED_IND, &connection_complete, sizeof(bt_ull_le_hid_srv_connected_ind_t));
   bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_AIR_SRV, BT_ULL_EVENT_LE_HID_CONNECTED_IND, &connection_complete, sizeof(bt_ull_le_hid_srv_connected_ind_t));
}

static void bt_ull_le_hid_srv_cis_disconnected_ind_hdl(bt_ull_le_hid_conn_srv_msg_ind_t *ind)
{
    if (!ind ) {
        return;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(ind->cis_connected.acl_handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"cis_disconnected_ind_hdl, not found link!!", 0);
        return;
    }
    if (BT_STATUS_SUCCESS != ind->status) {
        ull_report_error(BT_ULL_HID_LOG"cis_disconnected_ind_hdl, status error!!", 0);
        return;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
    ull_report(BT_ULL_HID_LOG"CIS DISCONNECTED, status: %d, link state: %x, dt: %d", 3, ind->status, state, device_type);
/*
    bt_ull_le_hid_srv_clear_link(idx);
    bt_ull_le_hid_srv_disconnect_t dis;
    dis.reason = ind->cis_disconnected.reason;
    dis.handle = ind->cis_disconnected.acl_handle;

    bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_DISCONNECTED_IND, &dis, sizeof(bt_ull_le_hid_srv_disconnect_t));
    bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_AIR_SRV, BT_ULL_EVENT_LE_HID_DISCONNECTED_IND, &dis, sizeof(bt_ull_le_hid_srv_disconnect_t));
*/
}

static void bt_ull_le_hid_srv_cis_active_streaming_ind_hdl(bt_ull_le_hid_conn_srv_msg_ind_t *ind)
{
    if (!ind ) {
        return;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(ind->streaming_active.acl_handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"cis_active_streaming_ind_hdl, not found link!!", 0);
        return;
    }
    if (BT_STATUS_SUCCESS != ind->status) {
        ull_report_error(BT_ULL_HID_LOG"cis_active_streaming_ind_hdl, status error!!", 0);
        return;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
    ull_report(BT_ULL_HID_LOG"CIS START STREAMING, status: %d, link state: %x, dt: %d", 3, ind->status, state, device_type);
    if (BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED <= state) {
        bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_STREAMING);
    }

}

static void bt_ull_le_hid_srv_cis_deactive_streaming_ind_hdl(bt_ull_le_hid_conn_srv_msg_ind_t *ind)
{
    if (!ind ) {
        return;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(ind->streaming_deactive.acl_handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"cis_deactive_streaming_ind_hdl, not found link!!", 0);
        return;
    }
    if (BT_STATUS_SUCCESS != ind->status) {
        ull_report_error(BT_ULL_HID_LOG"cis_deactive_streaming_ind_hdl, status error!!", 0);
        return;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
    ull_report(BT_ULL_HID_LOG"CIS STOP STREAMING, status: %d, link state: %x, dt: %d", 3, ind->status, state, device_type);
    if (BT_ULL_LE_HID_SRV_LINK_STATE_STREAMING == state) {
        bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED);
    }

}

static void bt_ull_le_hid_srv_cis_cancel_create_ind_hdl(bt_ull_le_hid_conn_srv_msg_ind_t *ind)
{
    if (!ind ) {
        return;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_dt(ind->cancel_create.device_type);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"cis_deactive_streaming_ind_hdl, not found link!!", 0);
        return;
    }
    if (BT_STATUS_SUCCESS != ind->status) {
        ull_report_error(BT_ULL_HID_LOG"cis_deactive_streaming_ind_hdl, status error!!", 0);
        return;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
    ull_report(BT_ULL_HID_LOG"CANCEL CREATE CIS, status: %d, link state: %x, dt: %d", 3, ind->status, state, device_type);
    if (BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTING == state) {
        //bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTED);
        //bt_ull_le_hid_srv_set_device_type(idx, BT_ULL_LE_HID_SRV_DEVICE_NONE);
    }

}

static bt_status_t bt_ull_le_hid_srv_scenario_changed_ind_hdl(bt_ull_le_hid_conn_srv_msg_ind_t *ind)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (!ind ) {
        return status;
    }
    ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_scenario_changed_ind_hdl, status: %x, hs: %x, kb: %x, ms: %x", 4, \
        ind->status, ind->scenario_changed.hs, ind->scenario_changed.kb, ind->scenario_changed.ms);
    bt_ull_le_hid_srv_connect_t conn;
    conn.audio_sink = ind->scenario_changed.hs;
    conn.keyboard = ind->scenario_changed.kb;
    conn.mouse = ind->scenario_changed.ms;
    status = bt_ull_le_hid_srv_create_connection_req(&conn);
    return status;
}

static bt_status_t bt_ull_le_hid_srv_conn_event_cb(bt_ull_le_hid_conn_srv_msg_t msg, void *data)
{
    bt_ull_le_hid_conn_srv_msg_ind_t *ind = (bt_ull_le_hid_conn_srv_msg_ind_t *)data;
    switch (msg) {
        case BT_ULL_LE_HID_CONN_SRV_MSG_CIG_REMOVED_IND: {
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CONNECTED_IND: {
            bt_ull_le_hid_srv_cis_connected_ind_hdl(ind);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CANCEL_CREATE_IND: {
            bt_ull_le_hid_srv_cis_cancel_create_ind_hdl(ind);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_MSG_CIS_DISCONNECTED_IND: {
            bt_ull_le_hid_srv_cis_disconnected_ind_hdl(ind);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_MSG_CIS_ACTIVE_STREAMING_IND: {
            bt_ull_le_hid_srv_cis_active_streaming_ind_hdl(ind);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_MSG_CIS_INACTIVE_STREAMING_IND: {
            bt_ull_le_hid_srv_cis_deactive_streaming_ind_hdl(ind);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_MSG_SCENARIO_CHANGED_IND: {
            bt_ull_le_hid_srv_scenario_changed_ind_hdl(ind);
            break;
        }

    }
    return BT_STATUS_SUCCESS;
}

static void bt_ull_le_hid_srv_power_off_hdl(void)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    if (!ctx) {
        return;
    }
    bt_ull_le_hid_conn_srv_deinit();
    bt_ull_le_hid_srv_audio_contex_t *audio_ctx = &ctx->audio_sink;
    bt_ull_le_hid_srv_mouse_contex_t *ms_ctx = &ctx->mouse;
    bt_ull_le_hid_srv_keyboard_contex_t *kb_ctx = &ctx->keyboard;
    if (audio_ctx) {
        if (audio_ctx->is_share_buffer_set) {
            audio_ctx->is_share_buffer_set = false;
            bt_ull_le_srv_deinit_share_info();
        }
    }
    if (ms_ctx) {

    }
    if (kb_ctx) {

    }
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i ++) {
        bt_ull_le_hid_srv_clear_link(i);
    }
}
static void bt_ull_le_hid_srv_power_on(void)
{
    return;
}

static void bt_ull_le_hid_srv_acl_connect_ind_hdl(bt_status_t status, bt_gap_le_connection_ind_t *ind)
{
    if (!ind) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    if (BT_STATUS_SUCCESS != status || BT_HANDLE_INVALID == ind->connection_handle) {
        ull_report_error(BT_ULL_HID_LOG"BT_GAP_LE_CONNECT_IND, status fail!!", 0);
        return;
    } else {
        if (BT_ULL_ROLE_SERVER == ctx->role || ((BT_ULL_ROLE_CLIENT == ctx->role) && \
            !bt_ull_le_srv_memcmp(&ind->local_addr.addr, &ctx->audio_sink.local_random_addr.addr, sizeof(bt_bd_addr_t)))) {
            uint8_t idx = bt_ull_le_hid_srv_get_empty_link();
            if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
                ull_report_error(BT_ULL_HID_LOG"BT_GAP_LE_CONNECT_IND, not link resource!!", 0);
            }
            bt_ull_le_hid_srv_set_link_handle(idx, ind->connection_handle);
            bt_ull_le_hid_srv_set_link_addr(idx, &ind->peer_addr);
            bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_ACL_CONNECTED);
            bt_ull_le_hid_srv_set_link_mode(idx, BT_ULL_LE_HID_SRV_LINK_MODE_FOTA);
            ull_report(BT_ULL_HID_LOG"BT_GAP_LE_CONNECT_IND, addr: %2x-%2x-%2x-%2x-%2x-%2x!", 6, \
                ind->peer_addr.addr[0], ind->peer_addr.addr[1], ind->peer_addr.addr[2], \
                ind->peer_addr.addr[3], ind->peer_addr.addr[4], ind->peer_addr.addr[5]);
        } else {
            ull_report_error(BT_ULL_HID_LOG"BT_GAP_LE_CONNECT_IND, role or bt addr not match!!", 0);
        }

    }
}

static void bt_ull_le_hid_srv_acl_disconnect_ind_hdl(bt_status_t status, bt_gap_le_disconnect_ind_t *ind)
{
    if (!ind) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();

    if (BT_STATUS_SUCCESS != status || BT_HANDLE_INVALID == ind->connection_handle) {
        ull_report_error(BT_ULL_HID_LOG"BT_GAP_LE_DISCONNECT_IND, status fail!!", 0);
        return;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(ind->connection_handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"BT_GAP_LE_DISCONNECT_IND, invalid handle!!", 0);
    }
    uint8_t dt = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    ull_report(BT_ULL_HID_LOG"acl_disconnect_ind_hdl, dt: %d, state: %d, role: %d, idx: %d", 4, dt, state, ctx->role, idx);
    bt_ull_le_hid_srv_print_addr(&ctx->link[idx].peer_addr);
    /*For audio client, stop AT or AM*/
    if (BT_ULL_LE_HID_SRV_DEVICE_HEADSET == dt || BT_ULL_LE_HID_SRV_DEVICE_EARBUDS == dt) {
        if (BT_ULL_ROLE_CLIENT == ctx->role) {
            bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
            /* stop audio manager */
            bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true);            
        } else if (BT_ULL_ROLE_SERVER == ctx->role) {
            /* close audio transmitter */
            uint8_t i;
            for (i = 0; i < BT_ULL_TRANSMITTER_MAX_NUM; i++) {
                if (stream_ctx->server.stream[i].is_transmitter_start || bt_ull_le_at_is_start(i)) {
                    bt_ull_le_at_deinit(i, true);
                }
            }            
        }
    }
    if (BT_ULL_LE_HID_SRV_LINK_STATE_ACL_CONNECTED <= state) {
        bt_ull_le_hid_srv_disconnected_ind_t dis;
        dis.status = status;
        dis.device_type = dt;
        dis.handle = ind->connection_handle;
        dis.reason = ind->reason;
        bt_ull_le_srv_memcpy(&dis.peer_addr, &ctx->link[idx].peer_addr, sizeof(bt_addr_t));
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_DISCONNECTED_IND, &dis, sizeof(bt_ull_le_hid_srv_disconnected_ind_t));
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_AIR_SRV, BT_ULL_EVENT_LE_HID_DISCONNECTED_IND, &dis, sizeof(bt_ull_le_hid_srv_disconnect_t));
        bt_ull_le_hid_srv_clear_link(idx);
        
    } else {
        return;
    }
}

static bt_status_t bt_ull_le_hid_srv_host_event_cb(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_POWER_OFF_CNF: {
            bt_ull_le_hid_srv_power_off_hdl();
            break;
        }
        case BT_POWER_ON_CNF: {
            bt_ull_le_hid_srv_power_on();
            break;
        }
        case BT_GAP_LE_CONNECT_IND: {
            bt_gap_le_connection_ind_t *ind = (bt_gap_le_connection_ind_t *)buff;
            bt_ull_le_hid_srv_acl_connect_ind_hdl(status, ind);
            break;
        }
        case BT_GAP_LE_DISCONNECT_IND: {
            bt_gap_le_disconnect_ind_t *ind = (bt_gap_le_disconnect_ind_t *)buff;
            bt_ull_le_hid_srv_acl_disconnect_ind_hdl(status, ind);
            break;
        }
        
        case BT_GAP_LE_ULL_PSEUDO_LINK_CONNECT_IND: {
            bt_gap_le_connection_ind_t *ind = (bt_gap_le_connection_ind_t *)buff;
            bt_ull_le_hid_srv_pseudo_link_connected_ind_hdl(status, ind);
            break;
        } 
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_ull_le_hid_srv_l2cap_event_cb(bt_l2cap_fix_channel_event_t event_id, bt_status_t status, void *buffer)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    switch (event_id) {
        case BT_L2CAP_FIX_CHANNEL_DATA_IND: {
            bt_l2cap_fix_channel_data_ind_t *packet = (bt_l2cap_fix_channel_data_ind_t *)buffer;

            ull_report(BT_ULL_HID_LOG"BT_L2CAP_FIX_CHANNEL_DATA_IND, fix cid is 0x%x, status is 0x%x, handle is 0x%x", 3, packet->cid, status, packet->connection_handle);
            if ((BT_ULL_LE_HID_SRV_FIX_CHANNEL_CID == packet->cid) &&
                (BT_STATUS_SUCCESS == status) && (BT_HANDLE_INVALID != packet->connection_handle)) {
                bt_ull_le_hid_srv_l2cap_rx_hdl(packet->connection_handle, packet->packet, packet->packet_length);
            } else {
                //ret = BT_STATUS_FAIL;
            }
        } break;

        default:
            break;
    }
    return ret;

}

static bt_status_t bt_ull_le_hid_srv_create_connection_req(bt_ull_le_hid_srv_connect_t *create_conn)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (!create_conn) {
        return BT_STATUS_FAIL;
    }
    bt_ull_le_hid_srv_conn_params_t *hs = NULL;
    bt_ull_le_hid_srv_conn_params_t *kb = NULL;
    bt_ull_le_hid_srv_conn_params_t *ms = NULL;
    uint8_t idx1 = BT_ULL_LE_HID_SRV_INVALID_VALUE;
    uint8_t idx2 = BT_ULL_LE_HID_SRV_INVALID_VALUE;
    uint8_t idx3 = BT_ULL_LE_HID_SRV_INVALID_VALUE;
    if (create_conn->audio_sink) {
        idx1 = bt_ull_le_hid_srv_get_empty_link();
        if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx1) {
            ull_report_error(BT_ULL_HID_LOG"create_connection_req_hdl(Audio), no link resource!!", 0);
            return BT_STATUS_FAIL;
        }
        bt_ull_le_hid_srv_set_link_state(idx1, BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTING);
        bt_ull_le_hid_srv_set_device_type(idx1, create_conn->audio_sink->device_type);
        hs = create_conn->audio_sink;
        ull_report(BT_ULL_HID_LOG"create_connection_req_hdl(Audio), count: %d", 1, hs->list_num);
        bt_ull_le_hid_srv_print_addr((bt_addr_t *)&hs->peer_addr_list);
    }
    if (create_conn->keyboard) {
        idx2 = bt_ull_le_hid_srv_get_empty_link();
        if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx2) {
            ull_report_error(BT_ULL_HID_LOG"create_connection_req_hdl(KB), no link resource!!", 0);
            return BT_STATUS_FAIL;
        }
        bt_ull_le_hid_srv_set_link_state(idx2, BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTING);
        bt_ull_le_hid_srv_set_device_type(idx2, create_conn->keyboard->device_type);
        kb = create_conn->keyboard;
        ull_report(BT_ULL_HID_LOG"create_connection_req_hdl(KB), count: %d", 1, kb->list_num);
        bt_ull_le_hid_srv_print_addr((bt_addr_t *)&kb->peer_addr_list);
    }
    if (create_conn->mouse) {
        idx3 = bt_ull_le_hid_srv_get_empty_link();
        if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx3) {
            ull_report_error(BT_ULL_HID_LOG"create_connection_req_hdl(MS), no link resource!!", 0);
            return BT_STATUS_FAIL;
        }
        bt_ull_le_hid_srv_set_link_state(idx3, BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTING);
        bt_ull_le_hid_srv_set_device_type(idx3, create_conn->mouse->device_type);
        ms = create_conn->mouse;
        ull_report(BT_ULL_HID_LOG"create_connection_req_hdl(MS), count: %d", 1, ms->list_num);
        bt_ull_le_hid_srv_print_addr((bt_addr_t *)&ms->peer_addr_list);
    }
    //bt_ull_le_hid_conn_srv_change_scenario(BT_ULL_LE_HID_SRV_APP_SCENARIO_1);
    status = bt_ull_le_hid_conn_srv_create_air_cis(hs, kb, ms);
    if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != status) {
        if (BT_ULL_LE_HID_SRV_INVALID_VALUE != idx1) {
            bt_ull_le_hid_srv_set_link_state(idx1, BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTED);
            bt_ull_le_hid_srv_set_device_type(idx1, BT_ULL_LE_HID_SRV_DEVICE_NONE);
        }
        if (BT_ULL_LE_HID_SRV_INVALID_VALUE != idx2) {
            bt_ull_le_hid_srv_set_link_state(idx2, BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTED);
            bt_ull_le_hid_srv_set_device_type(idx2, BT_ULL_LE_HID_SRV_DEVICE_NONE);
        }
        if (BT_ULL_LE_HID_SRV_INVALID_VALUE != idx3) {
            bt_ull_le_hid_srv_set_link_state(idx3, BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTED);
            bt_ull_le_hid_srv_set_device_type(idx3, BT_ULL_LE_HID_SRV_DEVICE_NONE);
        }
        ull_report_error(BT_ULL_HID_LOG"create_connection_req_hdl fail, status: %d!!", 1, status);
    }
    return status;
}

static bt_status_t bt_ull_le_hid_srv_disconnect_req(bt_ull_le_hid_srv_disconnect_t *disconnect)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (!disconnect) {
        return status;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(disconnect->handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"disconnect_req_hdl, no link resource!!", 0);
        return status;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
    ull_report(BT_ULL_HID_LOG"DISCONNECT CIS, link state: %x, dt: %d", 2, state, device_type);

    if (BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTED > state) {
        return status;
    }
    if (BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTING == state) {
        return BT_STATUS_SUCCESS;
    }
    bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTING);
    status = bt_ull_le_hid_conn_srv_disconnect_air_cis(disconnect->handle, disconnect->reason);
    if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != state) {
        bt_ull_le_hid_srv_set_link_state(idx, state);
        ull_report_error(BT_ULL_HID_LOG"disconnect_req_hdl, status error!! status: %x", 1, status);
    }
    return status;
}

static bt_status_t bt_ull_le_hid_srv_start_bonding_req(bt_ull_le_hid_srv_bond_t *bonding)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (!bonding) {
        return status;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(bonding->handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"start_bonding_req_hdl, no link resource!!", 0);
        return status;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
    ull_report(BT_ULL_HID_LOG"SEND BONDING REQ [SERVER], link state: %x, dt: %d", 2, state, device_type);
    if (BT_ULL_LE_HID_SRV_LINK_STATE_ACL_CONNECTED != state) {
        return status;
    }
    uint8_t pairing_req = BT_ULL_LE_HID_MSG_BONDING_REQ;
    status = bt_ull_le_hid_srv_l2cap_send_data(bonding->handle, &pairing_req, sizeof(bt_ull_le_hid_srv_msg_t));
    if (BT_STATUS_SUCCESS == status) {
        bt_handle_t handle = bonding->handle;
        bt_ull_le_hid_srv_start_timer(BT_ULL_LE_HID_BONDING_REQ_TIMER_ID, BT_ULL_LE_HID_SRV_TIMEOUT, bt_ull_le_hid_srv_timer_timeout_handler, handle);
    } else {
        ull_report_error(BT_ULL_HID_LOG"SEND BONDING REQ FAIL[SERVER]!!", 0);
    }
    return status;
}

static bt_status_t bt_ull_le_hid_srv_create_sync_req(bt_ull_le_hid_srv_create_sync_t *req)
{
    bt_status_t status;
    if (!req) {
        return BT_STATUS_FAIL;
    }
    ull_report(BT_ULL_HID_LOG"create_sync_req, dt: %d, addr : %2x-%2x-%2x-%2x-%2x-%2x", 7, req->device_type, \
        req->peer_addr.addr[0], req->peer_addr.addr[1], req->peer_addr.addr[2], req->peer_addr.addr[3], req->peer_addr.addr[4], req->peer_addr.addr[5]);
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_addr(&req->peer_addr);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE != idx) {
        uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
        ull_report_error(BT_ULL_HID_LOG"create_sync_req(Audio), state: %d", 1, state);
        if (BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTED < state) {
            return BT_STATUS_SUCCESS;
        }
    } else {
        idx = bt_ull_le_hid_srv_get_empty_link();
        if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
            ull_report_error(BT_ULL_HID_LOG"create_sync_req(Audio), no link resource!!", 0);
            return BT_STATUS_FAIL;
        }
    }

    bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTING);
    bt_ull_le_hid_srv_set_device_type(idx, req->device_type);

    status = bt_ull_le_hid_conn_srv_sync_air_cis(req->device_type, &req->peer_addr);
    if (BT_STATUS_SUCCESS != status) {
        bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_DISCONNECTED);
        bt_ull_le_hid_srv_set_device_type(idx, BT_ULL_LE_HID_SRV_DEVICE_NONE);

    }
    return status;
}

static bt_status_t bt_ull_le_hid_srv_connect_service_req(bt_ull_le_hid_srv_connect_service_t *req)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (!req) {
        return status;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(req->handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"srv_connect_service_req, no link resource!!", 0);
        return status;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
    if (BT_ULL_LE_HID_SRV_LINK_STATE_ACL_CONNECTED != state) {
        return status;
    }
    ull_report(BT_ULL_HID_LOG"SEND CONNECT SERVICE REQ [SERVER], link state: %x, dt: %d", 2, state, device_type);
    bt_ull_le_hid_srv_msg_t connect_srv_req = BT_ULL_LE_HID_MSG_SERVICE_CONNECT_REQ;
    status = bt_ull_le_hid_srv_l2cap_send_data(req->handle, &connect_srv_req, sizeof(bt_ull_le_hid_srv_msg_t));
    if (BT_STATUS_SUCCESS == status) {
        bt_handle_t handle = req->handle;
        bt_ull_le_hid_srv_start_timer(BT_ULL_LE_HID_SERVICE_CONNECT_TIMER_ID, BT_ULL_LE_HID_SRV_TIMEOUT, bt_ull_le_hid_srv_timer_timeout_handler, handle);
    } else {
         ull_report_error(BT_ULL_HID_LOG"SEND CONNECT SERVICE REQ [SERVER] FAIL", 0);
    }
    return status;
}

static void bt_ull_le_hid_srv_l2cap_rx_sync_context_info_hdl(bt_handle_t handle, bt_ull_le_hid_srv_context_info_req_t *ctx_info)
{
    bt_status_t status;
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(handle);
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();

    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx || !ctx_info) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_sync_context_info_hdl, no link resource!!", 0);
        return;
    }
    bt_ull_role_t role = bt_ull_le_hid_srv_get_role();
    if (BT_ULL_ROLE_CLIENT != role) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_sync_context_info_hdl, error role!!", 0);
        return;
    }
    ull_report(BT_ULL_HID_LOG"RECEIVE SYNC CONTEXT INFO REQ [CLIENT]!! Server streaming port: 0x%x, dl is_mute:0x%x, microphone is_mute: 0x%x", 3,
        ctx_info->info.audio_info.streaming_port , ctx_info->info.audio_info.dl.is_mute, ctx_info->info.audio_info.ul.is_mute); 
    stream_ctx->streaming_port = ctx_info->info.audio_info.streaming_port;
    /*save the DL info*/
    bt_ull_le_srv_memcpy(&(stream_ctx->client.dl.volume), &(ctx_info->info.audio_info.dl.volume), sizeof(bt_ull_original_duel_volume_t));
    stream_ctx->client.dl.is_mute = ctx_info->info.audio_info.dl.is_mute;
    stream_ctx->client.dl.usb_sample_rate = ctx_info->info.audio_info.dl.usb_sample_rate;
    /*save the UL info*/
    bt_ull_le_srv_memcpy(&(stream_ctx->client.ul.volume), &(ctx_info->info.audio_info.ul.volume), sizeof(bt_ull_original_duel_volume_t));
    stream_ctx->client.ul.is_mute = ctx_info->info.audio_info.ul.is_mute;
    stream_ctx->client.ul.usb_sample_rate = ctx_info->info.audio_info.ul.usb_sample_rate;

    /*client sync codec rsp to server*/
    uint16_t len = sizeof(bt_ull_le_hid_srv_msg_t) + sizeof(bt_ull_le_hid_srv_rsp_t);
    uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
    ull_assert("out of memory" && (data != NULL));
    data[0] = BT_ULL_LE_HID_MSG_RESPONSE;
    bt_ull_le_hid_srv_rsp_t *response = (bt_ull_le_hid_srv_rsp_t *)(data + sizeof(bt_ull_le_hid_srv_msg_t));
    response->cmd_id = BT_ULL_LE_HID_MSG_SYNC_CONTEXT_INFO;
    response->status = BT_ULL_LE_HID_SRV_RSP_STATUS_SUCCESS;
    stream_ctx->allow_play = bt_ull_le_am_is_allow_play()? BT_ULL_PLAY_ALLOW : BT_ULL_PLAY_DISALLOW;
    response->param.audio_sink_context_info_rsp.allow_play = stream_ctx->allow_play;
    response->param.audio_sink_context_info_rsp.client_type = ctx->audio_sink.client_type;
#ifdef MTK_PORT_SERVICE_BT_ENABLE
    response->param.audio_sink_context_info_rsp.att_handle_rx = ble_air_get_rx_char_handle();
    response->param.audio_sink_context_info_rsp.att_handle_tx = ble_air_get_tx_char_handle();
    response->param.audio_sink_context_info_rsp.att_handle_cccd = ble_air_get_cccd_char_handle();
#endif
    ull_report(BT_ULL_HID_LOG"SEND SYNC CONTEXT INFO RESPONSE [CLIENT]!! %d, %d", 2, \
        stream_ctx->allow_play,
        ctx->audio_sink.client_type);
    status = bt_ull_le_hid_srv_l2cap_send_data(handle, data, len);
    bt_ull_le_srv_memory_free(data);
    if (BT_STATUS_SUCCESS == status) {
        ull_report(BT_ULL_HID_LOG"SERVICE CONNECTED [CLIENT]!!", 0);
        bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED);
        bt_ull_le_hid_srv_service_connected_ind_t srv_connect_complete;
        srv_connect_complete.handle = handle;
        srv_connect_complete.status = status;
        bt_ull_le_srv_memcpy(&srv_connect_complete.peer_addr, &ctx->link[idx].peer_addr, sizeof(bt_addr_t));
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND, &srv_connect_complete, sizeof(bt_ull_le_hid_srv_service_connected_ind_t));
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_AIR_SRV, BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND, &srv_connect_complete, sizeof(bt_ull_le_hid_srv_service_connected_ind_t));
    } else {
        ull_report_error(BT_ULL_HID_LOG"SEND SYNC CONTEXT INFO RESPONSE FAIL [CLIENT]!!", 0);
    }

    /* Enable DSP if dongle is playing */
    if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
        stream_ctx->client.dl.is_dl_dummy_mode = false;
        bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
        if (BT_ULL_LE_HID_SRV_STREAM_STATE_IDLE == ctx->audio_sink.curr_stream_state) {
            ctx->audio_sink.curr_stream_state = BT_ULL_LE_HID_SRV_STREAM_STATE_START_AUDIO_STREAM;
        }
    }

    if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        stream.port = 0;
        bt_ull_le_hid_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
    }
}

static void bt_ull_le_hid_srv_l2cap_rx_sync_bonding_info_hdl(bt_handle_t handle, bt_ull_le_hid_srv_bonding_info_t *bonding_info)
{
    bt_status_t status;
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx || !bonding_info) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_sync_bonding_info_hdl, no link resource!!", 0);
        return;
    }
    bt_ull_role_t role = bt_ull_le_hid_srv_get_role();
    if (BT_ULL_ROLE_CLIENT != role) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_sync_bonding_info_hdl, error role!!", 0);
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_hid_dm_device_info_t device_info;
    device_info.codec_param.codec_type = ctx->audio_sink.client_preferred_codec_param.codec_type;
    device_info.codec_param.dl_samplerate = ctx->audio_sink.client_preferred_codec_param.dl_samplerate;
    device_info.codec_param.ul_samplerate = ctx->audio_sink.client_preferred_codec_param.ul_samplerate;
    if (BT_ULL_HEADSET_CLIENT == ctx->audio_sink.client_type) {
        device_info.device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
    } else if (BT_ULL_EARBUDS_CLIENT == ctx->audio_sink.client_type) {
        device_info.device_type = BT_ULL_LE_HID_SRV_DEVICE_EARBUDS;
    }

    bt_ull_le_srv_memcpy(&device_info.addr, &ctx->link[idx].peer_addr, sizeof(bt_addr_t));
    bt_ull_le_srv_memcpy(&device_info.uni_aa, &bonding_info->uni_aa, BT_ULL_LE_HID_DM_UNI_AA_LEN);
    bt_ull_le_srv_memcpy(&device_info.ltk, &bonding_info->ltk, BT_ULL_LE_HID_DM_LTK_LEN);
    bt_ull_le_srv_memcpy(&device_info.skd, &bonding_info->skd, BT_ULL_LE_HID_DM_SKD_LEN);
    bt_ull_le_srv_memcpy(&device_info.iv, &bonding_info->iv, BT_ULL_LE_HID_DM_IV_LEN);
    ull_report(BT_ULL_HID_LOG"RECEIVE SYNC BONDING INFO [CLIENT], LTK: %x:%x...%x, UNI AA: %x:%x:%x:%x, SKD: %x:%x..%x, IV: %x:%x..%x", 13, bonding_info->ltk[0], bonding_info->ltk[1], bonding_info->ltk[15], \
        bonding_info->uni_aa[0], bonding_info->uni_aa[1], bonding_info->uni_aa[2], bonding_info->uni_aa[3], \
        bonding_info->skd[0], bonding_info->skd[1], bonding_info->skd[15], \
        bonding_info->iv[0], bonding_info->iv[1], bonding_info->iv[7]);
    status = bt_ull_le_hid_dm_write_device_info(&device_info);
    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_sync_bonding_info_hdl, save device info fail!!", 0);
    }
    
    uint16_t len = sizeof(bt_ull_le_hid_srv_msg_t) + sizeof(bt_ull_le_hid_srv_rsp_t);
    uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
    data[0] = BT_ULL_LE_HID_MSG_RESPONSE;
    bt_ull_le_hid_srv_rsp_t *response = (bt_ull_le_hid_srv_rsp_t *)(data + sizeof(bt_ull_le_hid_srv_msg_t));
    response->cmd_id = BT_ULL_LE_HID_MSG_SYNC_BONDING_INFO;
    response->status = (status == BT_STATUS_SUCCESS) ? BT_ULL_LE_HID_SRV_RSP_STATUS_SUCCESS : BT_ULL_LE_HID_SRV_RSP_STATUS_FAIL;
    if (BT_ULL_HEADSET_CLIENT == ctx->audio_sink.client_type) {
        response->param.audio_sink_bonding_info_rsp.device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
    } else if (BT_ULL_EARBUDS_CLIENT == ctx->audio_sink.client_type) {
        response->param.audio_sink_bonding_info_rsp.device_type = BT_ULL_LE_HID_SRV_DEVICE_EARBUDS;
    }
    response->param.audio_sink_bonding_info_rsp.codec_param.codec_type = ctx->audio_sink.client_preferred_codec_param.codec_type;
    response->param.audio_sink_bonding_info_rsp.codec_param.dl_samplerate = ctx->audio_sink.client_preferred_codec_param.dl_samplerate;
    response->param.audio_sink_bonding_info_rsp.codec_param.ul_samplerate = ctx->audio_sink.client_preferred_codec_param.ul_samplerate;
    ull_report(BT_ULL_HID_LOG"SEND SYNC BONDING INFO RESPONSE [CLIENT]!!, codec: %d, DL_SR: %d, UL_SR: %d, dt: %d", 4, \
       response->param.audio_sink_bonding_info_rsp.codec_param.codec_type,
       response->param.audio_sink_bonding_info_rsp.codec_param.dl_samplerate,
       response->param.audio_sink_bonding_info_rsp.codec_param.ul_samplerate,
       response->param.audio_sink_bonding_info_rsp.device_type);

    status = bt_ull_le_hid_srv_l2cap_send_data(handle, data, len);
    bt_ull_le_srv_memory_free(data);
    if (BT_STATUS_SUCCESS == status) {
        ull_report(BT_ULL_HID_LOG"BONDING COMPLETE!! [CLIENT]", 0);
        bt_ull_le_hid_srv_bonding_complete_ind_t bonding_complete;
        bonding_complete.handle = handle;
        bonding_complete.status = status;
        bt_ull_le_srv_memcpy(&bonding_complete.peer_addr, &ctx->link[idx].peer_addr, sizeof(bt_addr_t));
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_BONDING_COMPLETE_IND, &bonding_complete, sizeof(bt_ull_le_hid_srv_bonding_complete_ind_t));
    } else {
        ull_report_error(BT_ULL_HID_LOG"SEND SYNC BONDING INFO RESPONSE FAIL [CLIENT]!!", 0);
    }
}

static void bt_ull_le_hid_srv_l2cap_rx_bonding_req_hdl(bt_handle_t handle)
{
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_bonding_req_hdl, no link resource!!", 0);
        return;
    }
    bt_ull_role_t role = bt_ull_le_hid_srv_get_role();
    if (BT_ULL_ROLE_CLIENT != role) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_bonding_req_hdl, error role!!", 0);
        return;
    }
    ull_report(BT_ULL_HID_LOG"RECEIVE BONDING REQ [CLIENT]!!", 0);
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    uint16_t len = sizeof(bt_ull_le_hid_srv_msg_t) + sizeof(bt_ull_le_hid_srv_rsp_t);
    uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
    data[0] = BT_ULL_LE_HID_MSG_RESPONSE;
    bt_ull_le_hid_srv_rsp_t *response = (bt_ull_le_hid_srv_rsp_t *)(data + sizeof(bt_ull_le_hid_srv_msg_t));
    response->cmd_id = BT_ULL_LE_HID_MSG_BONDING_REQ;
    response->status = BT_ULL_LE_HID_SRV_RSP_STATUS_SUCCESS;
    if (BT_ULL_HEADSET_CLIENT == ctx->audio_sink.client_type) {
        response->param.bonding_rsp.device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
    } else if (BT_ULL_EARBUDS_CLIENT == ctx->audio_sink.client_type) {
        response->param.bonding_rsp.device_type = BT_ULL_LE_HID_SRV_DEVICE_EARBUDS;
    }
    ull_report(BT_ULL_HID_LOG"SEND BONDING REQ RESPONSE [CLIENT]!!", 0);
    bt_status_t status = bt_ull_le_hid_srv_l2cap_send_data(handle, data, len);
    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_HID_LOG"SEND BONDING REQ RESPONSE FAIL [CLIENT]!!", 0);
    }
    bt_ull_le_srv_memory_free(data);
}

static void bt_ull_le_hid_srv_l2cap_rx_response_hdl(bt_handle_t handle, bt_ull_le_hid_srv_rsp_t *rsp)
{
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(handle);
    bt_status_t status = BT_STATUS_SUCCESS;
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx || !rsp) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_response_hdl, no link resource!!", 0);
        return;
    }
    bt_ull_role_t role = bt_ull_le_hid_srv_get_role();
    if (BT_ULL_ROLE_SERVER != role) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_response_hdl, error role!!", 0);
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ull_report(BT_ULL_HID_LOG"rx_response_hdl, handle: %x, cmd id: %x, status: %x, ", 3, handle, rsp->cmd_id, rsp->status);
    switch (rsp->cmd_id) {
        case BT_ULL_LE_HID_MSG_SERVICE_CONNECT_REQ: {
            bt_ull_le_hid_srv_stop_timer(BT_ULL_LE_HID_SERVICE_CONNECT_TIMER_ID);
            ull_report(BT_ULL_HID_LOG"RECEIVE CONNECT RSP [SERVER]!![dt: %d]!!", 1, rsp->param.connect_rsp.device_type);
            if (BT_ULL_LE_HID_SRV_RSP_STATUS_SUCCESS == rsp->status) {
                bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
                bt_ull_le_hid_srv_set_device_type(idx, rsp->param.connect_rsp.device_type);
                uint16_t len = sizeof(bt_ull_le_hid_srv_msg_t) + sizeof(bt_ull_le_hid_srv_context_info_req_t);
                uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                ull_assert("out of memory" && (data != NULL));

                data[0] = BT_ULL_LE_HID_MSG_SYNC_CONTEXT_INFO;
                bt_ull_le_hid_srv_context_info_req_t *sync_data = (bt_ull_le_hid_srv_context_info_req_t *)(data + sizeof(bt_ull_le_hid_srv_msg_t));
                if (BT_ULL_LE_HID_SRV_DEVICE_HEADSET == rsp->param.connect_rsp.device_type \
                    || BT_ULL_LE_HID_SRV_DEVICE_EARBUDS == rsp->param.connect_rsp.device_type) {
                    /* exchange codec info */
                    if (BT_ULL_ROLE_SERVER == role) {
                        sync_data->info.audio_info.streaming_port = stream_ctx->streaming_port;
                        bt_ull_le_srv_memcpy(&sync_data->info.audio_info.dl_mix_ratio, &stream_ctx->dl_mix_ratio, sizeof(bt_ull_mix_ratio_t));
                        sync_data->info.audio_info.dl.is_mute = stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].is_mute;
                        sync_data->info.audio_info.dl.usb_sample_rate = stream_ctx->server.stream[BT_ULL_GAMING_TRANSMITTER].usb_sample_rate;
                        bt_ull_le_srv_memcpy(&(sync_data->info.audio_info.dl.volume), &(stream_ctx->client.dl.volume), sizeof(bt_ull_original_duel_volume_t));
                        sync_data->info.audio_info.ul.is_mute = stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].is_mute;
                        sync_data->info.audio_info.ul.usb_sample_rate = stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].usb_sample_rate;
                        bt_ull_le_srv_memcpy(&(sync_data->info.audio_info.ul.volume), &(stream_ctx->client.ul.volume), sizeof(bt_ull_original_duel_volume_t));
                        ull_report(BT_ULL_HID_LOG"server streaming port: 0x%x, is_mute:0x%x, microphone is_mute:0x%x", 3,
                            sync_data->info.audio_info.streaming_port, sync_data->info.audio_info.is_mute, sync_data->info.audio_info.ul.is_mute);

                    } 
                }
                bt_status_t status = bt_ull_le_hid_srv_l2cap_send_data(handle, data, len);
                if (BT_STATUS_SUCCESS == status) {
                    bt_handle_t save_handle = handle;
                    bt_ull_le_hid_srv_start_timer(BT_ULL_LE_HID_SYNC_CONTEXT_INFO_TIMER_ID, BT_ULL_LE_HID_SRV_TIMEOUT, bt_ull_le_hid_srv_timer_timeout_handler, save_handle);
                }
                ull_report(BT_ULL_HID_LOG"SEND SYNC CONTEXT INFO [SERVER]!! status: %x", 1, status);
                bt_ull_le_srv_memory_free(data);
            } else {
                ull_report_error(BT_ULL_HID_LOG"l2cap_rx_response_hdl, Connect srv fail!!", 0);
                bt_ull_le_hid_srv_service_connected_ind_t srv_complete;
                srv_complete.handle = handle;
                srv_complete.status = BT_STATUS_FAIL;
                bt_ull_le_srv_memcpy(&srv_complete.peer_addr, &ctx->link[idx].peer_addr, sizeof(bt_addr_t));
                bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND, &srv_complete, sizeof(bt_ull_le_hid_srv_service_connected_ind_t));
                bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_AIR_SRV, BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND, &srv_complete, sizeof(bt_ull_le_hid_srv_service_connected_ind_t));
            }
            break;
        }
        case BT_ULL_LE_HID_MSG_SYNC_CONTEXT_INFO: {
            bt_ull_le_hid_srv_stop_timer(BT_ULL_LE_HID_SYNC_CONTEXT_INFO_TIMER_ID);
            ull_report(BT_ULL_HID_LOG"RECEIVE SYNC CONTEXT INFO RSP [SERVER]!!", 0);
            uint8_t mode = bt_ull_le_hid_srv_get_link_mode(idx);
            if (BT_ULL_LE_HID_SRV_RSP_STATUS_SUCCESS == rsp->status) {
                uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
                if (BT_ULL_LE_HID_SRV_DEVICE_HEADSET == device_type || BT_ULL_LE_HID_SRV_DEVICE_EARBUDS == device_type) {
                    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
                    uint32_t usb_gaming_sample_rate = USB_Audio_Get_RX_Sample_Rate(1);
                    uint32_t usb_chat_sample_rate = USB_Audio_Get_RX_Sample_Rate(0);
                    uint32_t usb_mic_sample_rate = USB_Audio_Get_TX_Sample_Rate(0);
                    ctx->audio_sink.client_type = rsp->param.audio_sink_context_info_rsp.client_type;
                    ull_report(BT_ULL_HID_LOG"rx sync context info, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", 12, \
                        ctx->audio_sink.is_share_buffer_set,
                        rsp->param.audio_sink_context_info_rsp.allow_play,
                        ctx->audio_sink.client_type,
                        stream_ctx->is_silence,
                        stream_ctx->streaming_port,
                        usb_gaming_sample_rate,
                        usb_chat_sample_rate,
                        usb_mic_sample_rate,
                        mode,
                        rsp->param.audio_sink_context_info_rsp.att_handle_cccd,
                        rsp->param.audio_sink_context_info_rsp.att_handle_rx,
                        rsp->param.audio_sink_context_info_rsp.att_handle_tx);
                    bt_ull_le_hid_srv_set_cccd_att_handle(idx, rsp->param.audio_sink_context_info_rsp.att_handle_cccd);
                    bt_ull_le_hid_srv_set_rx_att_handle(idx, rsp->param.audio_sink_context_info_rsp.att_handle_rx);
                    bt_ull_le_hid_srv_set_tx_att_handle(idx, rsp->param.audio_sink_context_info_rsp.att_handle_tx);
                    bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED);
                    /* init controller media data share buffer info */
                    if (!ctx->audio_sink.is_share_buffer_set) {
                        status = bt_ull_le_srv_init_share_info(ctx->audio_sink.client_type);
                        if ((BT_STATUS_SUCCESS == status) && (BT_STATUS_SUCCESS == bt_ull_le_srv_set_avm_share_buffer(ctx->role, ctx->audio_sink.client_type, 0x02))) {
                            ctx->audio_sink.is_share_buffer_set = true;
                        } else {
                            ull_report_error(BT_ULL_HID_LOG"rx sync context info set AVM buffer error!!", 0);
                        }
                    }
                    stream_ctx->allow_play = rsp->param.audio_sink_context_info_rsp.allow_play;
                    if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) && (0 != usb_gaming_sample_rate)) {
                        bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_port(BT_ULL_LE_STREAM_PORT_MASK_GAMING);
                        bt_ull_le_srv_server_stream_t *stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
                        stream_info->usb_sample_rate = usb_gaming_sample_rate;
                        if (!stream_ctx->is_silence) {
                            bt_ull_le_hid_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_GAMING, BT_ULL_EVENT_STREAMING_START_IND);
                        }
                    }
                    if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) && (0 != usb_chat_sample_rate)) {
                        bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_port(BT_ULL_LE_STREAM_PORT_MASK_CHAT);
                        bt_ull_le_srv_server_stream_t *stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
                        stream_info->usb_sample_rate = usb_chat_sample_rate;
                        if (!stream_ctx->is_silence) {
                            bt_ull_le_hid_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_CHAT, BT_ULL_EVENT_STREAMING_START_IND);
                        }
            
                    }
                    if ((stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) && (0 != usb_mic_sample_rate)) {
                        bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_port(BT_ULL_LE_STREAM_PORT_MASK_MIC);
                        bt_ull_le_srv_server_stream_t *stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
                        stream_info->usb_sample_rate = usb_mic_sample_rate;
                        bt_ull_le_hid_srv_sync_streaming_status(BT_ULL_LE_STREAM_PORT_MASK_MIC, BT_ULL_EVENT_STREAMING_START_IND);
                    }                    
                    
                } else if (BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD == device_type) {
                    bt_ull_le_hid_srv_set_cccd_att_handle(idx, rsp->param.keboard_context_info_rsp.att_handle_cccd);
                    bt_ull_le_hid_srv_set_rx_att_handle(idx, rsp->param.keboard_context_info_rsp.att_handle_rx);
                    bt_ull_le_hid_srv_set_tx_att_handle(idx, rsp->param.keboard_context_info_rsp.att_handle_tx);
                    bt_ull_le_hid_srv_set_link_state(idx, BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED);
                } else if (BT_ULL_LE_HID_SRV_DEVICE_MOUSE == device_type) {
                    bt_ull_le_hid_srv_set_cccd_att_handle(idx, rsp->param.mouse_context_info_rsp.att_handle_cccd);
                    bt_ull_le_hid_srv_set_rx_att_handle(idx, rsp->param.mouse_context_info_rsp.att_handle_rx);
                    bt_ull_le_hid_srv_set_tx_att_handle(idx, rsp->param.mouse_context_info_rsp.att_handle_tx);
                } else {
                    assert(0);
                }

                ull_report(BT_ULL_HID_LOG"SERVICE CONNECTED [SERVER][dt: %d]!!" , 1, device_type);
            }
            bt_ull_le_hid_srv_service_connected_ind_t srv_conn_cmpl;
            srv_conn_cmpl.handle = handle;
            bt_ull_le_srv_memcpy(&srv_conn_cmpl.peer_addr, &ctx->link[idx].peer_addr, sizeof(bt_addr_t));
            srv_conn_cmpl.status = (BT_ULL_LE_HID_SRV_RSP_STATUS_SUCCESS == rsp->status) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
            bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND, &srv_conn_cmpl, sizeof(bt_ull_le_hid_srv_service_connected_ind_t));
            bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_AIR_SRV, BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND, &srv_conn_cmpl, sizeof(bt_ull_le_hid_srv_service_connected_ind_t));
            break;
        }
        case BT_ULL_LE_HID_MSG_BONDING_REQ: {
            bt_ull_le_hid_srv_stop_timer(BT_ULL_LE_HID_BONDING_REQ_TIMER_ID);
            ull_report(BT_ULL_HID_LOG"RECEIVE BONDING RSP [SERVER]!![dt: %d]!!", 1, rsp->param.bonding_rsp.device_type);
            if (BT_ULL_LE_HID_SRV_RSP_STATUS_SUCCESS == rsp->status) {
                bt_ull_le_hid_srv_set_device_type(idx, rsp->param.bonding_rsp.device_type);
                uint16_t len = sizeof(bt_ull_le_hid_srv_msg_t) + sizeof(bt_ull_le_hid_srv_bonding_info_t);
                uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                uint8_t *uni_aa = bt_ull_le_hid_dm_generate_uni_aa();
                data[0] = BT_ULL_LE_HID_MSG_SYNC_BONDING_INFO;
                bt_ull_le_hid_srv_bonding_info_t *bonding_info = (bt_ull_le_hid_srv_bonding_info_t *)(data + sizeof(bt_ull_le_hid_srv_msg_t));
                bt_ull_le_srv_memcpy(&bonding_info->uni_aa, uni_aa, BT_ULL_LE_HID_DM_UNI_AA_LEN);
                bt_ull_le_srv_memcpy(&bonding_info->ltk, bt_ull_le_hid_dm_get_ltk(), BT_ULL_LE_HID_DM_LTK_LEN);
                bt_ull_le_srv_memcpy(&bonding_info->skd, bt_ull_le_hid_dm_get_skd(), BT_ULL_LE_HID_DM_SKD_LEN);
                bt_ull_le_srv_memcpy(&bonding_info->iv, bt_ull_le_hid_dm_get_iv(), BT_ULL_LE_HID_DM_IV_LEN);
                bt_status_t status = bt_ull_le_hid_srv_l2cap_send_data(handle, data, len);
                bt_ull_le_srv_memory_free(data);
                if (BT_STATUS_SUCCESS == status) {
                    bt_handle_t save_handle = handle;
                    switch (rsp->param.bonding_rsp.device_type) {
                        case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                            bt_ull_le_srv_memcpy(&g_hs_uni_aa[0], uni_aa, BT_ULL_LE_HID_DM_UNI_AA_LEN);
                            break;
                        }
                        case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                            bt_ull_le_srv_memcpy(&g_kb_uni_aa[0], uni_aa, BT_ULL_LE_HID_DM_UNI_AA_LEN);
                            break;
                        }
                        case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
                            bt_ull_le_srv_memcpy(&g_ms_uni_aa[0], uni_aa, BT_ULL_LE_HID_DM_UNI_AA_LEN);
                            break;
                        }
                        default :
                            break;
                    }
                    bt_ull_le_hid_srv_start_timer(BT_ULL_LE_HID_SYNC_BONDING_INFO_TIMER_ID, BT_ULL_LE_HID_SRV_TIMEOUT, bt_ull_le_hid_srv_timer_timeout_handler, save_handle);
                }
                ull_report(BT_ULL_HID_LOG"SEND SYNC BONDING INFO [SERVER]!!, status: %x, Uni_AA: %x-%x-%x-%x", 5, status, \
                    uni_aa[0], uni_aa[1], uni_aa[2], uni_aa[3]);
            } else {
                bt_ull_le_hid_srv_bonding_complete_ind_t bonding_complete;
                bonding_complete.handle = handle;
                bt_ull_le_srv_memcpy(&bonding_complete.peer_addr, &ctx->link[idx].peer_addr, sizeof(bt_addr_t));
                bonding_complete.status = BT_STATUS_FAIL;
                bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_BONDING_COMPLETE_IND, &bonding_complete, sizeof(bt_ull_le_hid_srv_bonding_complete_ind_t));
            }

            break;
        }
        case BT_ULL_LE_HID_MSG_SYNC_BONDING_INFO: {
            bt_ull_le_hid_srv_stop_timer(BT_ULL_LE_HID_SYNC_BONDING_INFO_TIMER_ID);
            ull_report(BT_ULL_HID_LOG"RECEIVE SYNC BONDING INFO RSP [SERVER]!!", 0);
            if (BT_ULL_LE_HID_SRV_RSP_STATUS_SUCCESS == rsp->status) {
                bt_ull_le_hid_dm_device_info_t device_info;
                uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
                bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
                device_info.device_type = device_type;
                if (BT_ULL_LE_HID_SRV_DEVICE_HEADSET == device_type) {
                    device_info.codec_param.codec_type = rsp->param.audio_sink_bonding_info_rsp.codec_param.codec_type;
                    device_info.codec_param.dl_samplerate = rsp->param.audio_sink_bonding_info_rsp.codec_param.dl_samplerate;
                    device_info.codec_param.ul_samplerate = rsp->param.audio_sink_bonding_info_rsp.codec_param.ul_samplerate;
                    ull_report(BT_ULL_HID_LOG"AUDIO USE PARAM: CODEC: %d, DL_SR: %d, UL_SR: %d", 3, \
                        device_info.codec_param.codec_type,
                        device_info.codec_param.dl_samplerate,
                        device_info.codec_param.ul_samplerate);
                    bt_ull_le_srv_set_codec_param_by_sample_rate(BT_ULL_ROLE_SERVER, device_info.codec_param.dl_samplerate, device_info.codec_param.ul_samplerate);
                }
                switch (device_type) {
                    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                        bt_ull_le_srv_memcpy(&device_info.uni_aa, &g_hs_uni_aa[0], BT_ULL_LE_HID_DM_UNI_AA_LEN);
                        break;
                    }
                    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                        bt_ull_le_srv_memcpy(&device_info.uni_aa, &g_kb_uni_aa[0], BT_ULL_LE_HID_DM_UNI_AA_LEN);
                        break;
                    }
                    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
                        bt_ull_le_srv_memcpy(&device_info.uni_aa, &g_ms_uni_aa[0], BT_ULL_LE_HID_DM_UNI_AA_LEN);
                        break;
                    }
                    default :
                        break;
                }
                bt_ull_le_srv_memcpy(&device_info.addr, &ctx->link[idx].peer_addr, sizeof(bt_addr_t));
                bt_ull_le_srv_memcpy(&device_info.ltk, bt_ull_le_hid_dm_get_ltk(), BT_ULL_LE_HID_DM_LTK_LEN);
                bt_ull_le_srv_memcpy(&device_info.skd, bt_ull_le_hid_dm_get_skd(), BT_ULL_LE_HID_DM_SKD_LEN);
                bt_ull_le_srv_memcpy(&device_info.iv, bt_ull_le_hid_dm_get_iv(), BT_ULL_LE_HID_DM_IV_LEN);
                //bt_ull_le_srv_memcpy(&device_info.codec_param, rsp->param.audio_sink_bonding_info_rsp,uint32_t size);

                bt_status_t status = bt_ull_le_hid_dm_write_device_info(&device_info);
                if (BT_STATUS_SUCCESS != status) {
                    ull_report_error(BT_ULL_HID_LOG"l2cap_rx_response_hdl, save device info fail!!", 0);
                }
                bt_ull_le_hid_srv_bonding_complete_ind_t bonding_complete;
                bonding_complete.handle = handle;
                bonding_complete.status = status;
                bt_ull_le_srv_memcpy(&bonding_complete.peer_addr, &ctx->link[idx].peer_addr, sizeof(bt_addr_t));
                ull_report(BT_ULL_HID_LOG"BONDING COMPLETE!! [SERVER][dt: %d]!!" , 1, device_type);
                bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_BONDING_COMPLETE_IND, &bonding_complete, sizeof(bt_ull_le_hid_srv_bonding_complete_ind_t));

            } else {
                bt_ull_le_hid_srv_bonding_complete_ind_t bonding_complete;
                bonding_complete.handle = handle;
                bonding_complete.status = BT_STATUS_FAIL;
                bt_ull_le_srv_memcpy(&bonding_complete.peer_addr, &ctx->link[idx].peer_addr, sizeof(bt_addr_t));
                bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_BONDING_COMPLETE_IND, &bonding_complete, sizeof(bt_ull_le_hid_srv_bonding_complete_ind_t));
            }
            break;
        }
        default:
            break;

    }
}

static void bt_ull_le_hid_srv_l2cap_rx_service_connect_req_hdl(bt_handle_t handle)
{
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report_error(BT_ULL_HID_LOG"2cap_rx_service_connect_req_hdl, no link resource!!", 0);
        return;
    }
    bt_ull_role_t role = bt_ull_le_hid_srv_get_role();
    if (BT_ULL_ROLE_CLIENT != role) {
        ull_report_error(BT_ULL_HID_LOG"2cap_rx_service_connect_req_hdl, error role!!", 0);
        return;
    }
    ull_report(BT_ULL_HID_LOG"RECEIVE SERVICE_CONNECT REQ [CLIENT]!!", 0);
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    uint16_t len = sizeof(bt_ull_le_hid_srv_msg_t) + sizeof(bt_ull_le_hid_srv_rsp_t);
    uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
    data[0] = BT_ULL_LE_HID_MSG_RESPONSE;
    bt_ull_le_hid_srv_rsp_t *response = (bt_ull_le_hid_srv_rsp_t *)(data + sizeof(bt_ull_le_hid_srv_msg_t));
    response->cmd_id = BT_ULL_LE_HID_MSG_SERVICE_CONNECT_REQ;
    response->status = BT_ULL_LE_HID_SRV_RSP_STATUS_SUCCESS;
    if (BT_ULL_HEADSET_CLIENT == ctx->audio_sink.client_type) {
        response->param.connect_rsp.device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
    } else if (BT_ULL_EARBUDS_CLIENT == ctx->audio_sink.client_type) {
        response->param.connect_rsp.device_type = BT_ULL_LE_HID_SRV_DEVICE_EARBUDS;
    }
    bt_status_t status = bt_ull_le_hid_srv_l2cap_send_data(handle, data, len);
    bt_ull_le_srv_memory_free(data);
    if (BT_STATUS_SUCCESS == status) {
        ull_report(BT_ULL_HID_LOG"SEND SERVICE_CONNECT RESPONSE [CLIENT]!!", 0);
    } else {
        ull_report_error(BT_ULL_HID_LOG"SEND SERVICE_CONNECT RESPONSE FAIL [CLIENT]!!", 0);
    }

}

static void bt_ull_le_hid_srv_l2cap_rx_start_streaming_hdl(bt_ull_streaming_t *streaming)
{
    if (!streaming) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report(BT_ULL_HID_LOG"rx recieved BT_ULL_EVENT_STREAMING_START_IND, if_type: 0x%x, port:0x%x", 2, streaming->streaming_interface, streaming->port);
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
    bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
    ull_report(BT_ULL_HID_LOG"rx recieved BT_ULL_EVENT_STREAMING_START_IND, streaming_port: 0x%x", 1, stream_ctx->streaming_port);
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        /*< ULL Client use the same avm buffer with LE Audio, it is initialized in app common part. please check it firstly.*/
        if ((BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface)
            || (BT_ULL_STREAMING_INTERFACE_LINE_IN == streaming->streaming_interface)
            || (BT_ULL_STREAMING_INTERFACE_I2S_IN == streaming->streaming_interface)) {
            if (stream_ctx->streaming_port & stream_port) {
                /**< means that client had recieved the same event. ignore it. */
                return;
            }
            stream_ctx->streaming_port |= stream_port;
            //bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_ALLOW, BT_ULL_ALLOW_PLAY_REASON_AUDIO); /*BTA-37651*/
            /*Enable Downlink, exit DL dummy mode.*/
            bt_ull_le_hid_srv_play_am(streaming, BT_ULL_LE_STREAM_MODE_DOWNLINK);
        } else if ((BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)
                    || (BT_ULL_STREAMING_INTERFACE_LINE_OUT == streaming->streaming_interface)
                    || (BT_ULL_STREAMING_INTERFACE_I2S_OUT == streaming->streaming_interface)) {
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                /**< means that client had recieved the same event. ignore it. */
                return;
            }                            
            stream_ctx->streaming_port |= BT_ULL_LE_STREAM_PORT_MASK_MIC;
            bt_ull_le_hid_srv_play_am(streaming, BT_ULL_LE_STREAM_MODE_UPLINK);
        }
    }

}

static void bt_ull_le_hid_srv_l2cap_rx_stop_streaming_hdl(bt_ull_streaming_t *streaming)
{
    if (!streaming) {
        return;
    }

    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report(BT_ULL_HID_LOG"rx recieved BT_ULL_EVENT_STREAMING_STOP_IND, if_type: 0x%x, port:0x%x, current port mask: %x", 3, \
        streaming->streaming_interface, streaming->port, stream_ctx->streaming_port);
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
    bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
    ull_report(BT_ULL_HID_LOG"rx recieved BT_ULL_EVENT_STREAMING_STOP_IND, streaming_port: 0x%x", 1, stream_ctx->streaming_port);
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if ((BT_ULL_STREAMING_INTERFACE_SPEAKER == streaming->streaming_interface)
            || (BT_ULL_STREAMING_INTERFACE_LINE_IN == streaming->streaming_interface)
            || (BT_ULL_STREAMING_INTERFACE_I2S_IN == streaming->streaming_interface)) {
            /*Disable Downlink.*/
            stream_ctx->streaming_port &= ~stream_port;
            bt_ull_le_hid_srv_stop_am(streaming);
        } else if ((BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)
            || (BT_ULL_STREAMING_INTERFACE_LINE_OUT == streaming->streaming_interface)
            || (BT_ULL_STREAMING_INTERFACE_I2S_OUT == streaming->streaming_interface)) {
                stream_ctx->streaming_port &= ~BT_ULL_LE_STREAM_PORT_MASK_MIC;
                if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                    stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_MIC;
                }
                /* ULL earbuds only have 1 earbud active uplink, but it stiil need to update the downlink's audio priority to support Uplink can match the EMP media priority rule */
                if ((BT_ULL_EARBUDS_CLIENT == ctx->audio_sink.client_type) /*&& !(link_info->ul_active)*/) {
                    stream_ctx->client.ul.is_am_open = false;
                    bt_ull_le_am_sync_sidetone_status(false);
                    if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {//DL is dummy mode
                        if (BT_STATUS_SUCCESS == bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true)) {
                            stream_ctx->client.dl.is_dl_dummy_mode = false;
                            bt_ull_le_am_update_dl_priority(false);
                            ull_report(BT_ULL_HID_LOG"Uplink Disable, then stop Downlink Dummy mode", 0);
                        }
                        return;
                    } else {
                        bt_ull_le_am_update_dl_priority(false);//need to lower proiryty because of Uplink had been disabled
                    }
                }
    
                /* Normal case, disble mic if ull is playing */
                //if (stream_ctx->client.ul.is_am_open) {
                    bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
                    stream_ctx->client.dl.is_dl_dummy_mode = false;
                    bt_ull_le_am_update_dl_priority(false);
                //}
                ull_report(BT_ULL_HID_LOG"Client recieved disable mic, curr_stream_state:0x%x, is_dl_dummy_mode:No", 1, ctx->audio_sink.curr_stream_state);
        }

    }

}

static void bt_ull_le_hid_srv_l2cap_rx_volume_ind_hdl(bt_ull_volume_ind_t *p_vol)
{
    if (!p_vol) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report(BT_ULL_HID_LOG"rx recieved BT_ULL_EVENT_VOLUME_IND, if_type: 0x%x, port: 0x%x, vol_left:%d, vol_right:%d", 4,
        p_vol->streaming.streaming_interface, p_vol->streaming.port, p_vol->vol.vol_left, p_vol->vol.vol_right);
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if ((BT_ULL_STREAMING_INTERFACE_MICROPHONE == p_vol->streaming.streaming_interface)
            || (BT_ULL_STREAMING_INTERFACE_LINE_OUT == p_vol->streaming.streaming_interface)
            || (BT_ULL_STREAMING_INTERFACE_I2S_OUT == p_vol->streaming.streaming_interface)) {
            stream_ctx->client.ul.volume.vol_left = p_vol->vol.vol_left;
            stream_ctx->client.ul.volume.vol_right = p_vol->vol.vol_right;
            bt_ull_le_am_set_volume(BT_ULL_LE_AM_UL_MODE, p_vol->vol.vol_left, BT_ULL_AUDIO_CHANNEL_DUAL);
            return;
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
        //BTA-33021
        if (0 == p_vol->vol.vol_left && 0 != p_vol->vol.vol_right) {
            bt_ull_le_am_set_volume(BT_ULL_LE_AM_DL_MODE, p_vol->vol.vol_right, p_vol->channel);
        } else if (0 != p_vol->vol.vol_left && 0 == p_vol->vol.vol_right) {
            bt_ull_le_am_set_volume(BT_ULL_LE_AM_DL_MODE, p_vol->vol.vol_left, p_vol->channel);
        } else {
            bt_ull_le_am_set_volume(BT_ULL_LE_AM_DL_MODE, p_vol->vol.vol_left, p_vol->channel);
        }
    }
}

static void bt_ull_le_hid_srv_l2cap_rx_volume_hdl(bt_ull_volume_t *p_vol)
{
    if (!p_vol) {
        return;
    }

    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report(BT_ULL_HID_LOG"dongle rx recieved VOLUME_REPORT, type:0x%x, volume action: 0x%x, volume_level: %d,", 3,
        p_vol->streaming.streaming_interface, p_vol->action, p_vol->volume);
#if defined(AIR_USB_ENABLE) && defined(AIR_USB_AUDIO_ENABLE) && defined(AIR_USB_HID_ENABLE)
    if (BT_ULL_STREAMING_INTERFACE_SPEAKER == p_vol->streaming.streaming_interface) {
        if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
            ull_report(BT_ULL_HID_LOG"usb_hid_control, current is XBOX mode, ignore the HID control event", 0);
        } else {
            bt_status_t usb_hid_volume_control_result = BT_STATUS_FAIL;
            if (BT_ULL_VOLUME_ACTION_SET_UP == p_vol->action) {
                 if (USB_HID_STATUS_OK == USB_HID_VolumeUp(p_vol->volume)) {
                    usb_hid_volume_control_result = BT_STATUS_SUCCESS;
                    ull_report(BT_ULL_HID_LOG"usb_hid_control, volume up success", 0);
                }

            } else if (BT_ULL_VOLUME_ACTION_SET_DOWN == p_vol->action) {
                if (USB_HID_STATUS_OK == USB_HID_VolumeDown(p_vol->volume)) {
                    usb_hid_volume_control_result = BT_STATUS_SUCCESS;
                    ull_report(BT_ULL_HID_LOG"usb_hid_control, volume down success", 0);
                }
            }
            if (BT_STATUS_FAIL == usb_hid_volume_control_result) {
                ull_report_error(BT_ULL_HID_LOG"usb_hid_control fail, local adjust volume", 0);
                if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
                    p_vol->streaming.port = 0;
                    bt_ull_le_hid_srv_handle_set_volume(p_vol);
                }
                if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
                    p_vol->streaming.port = 1;
                    bt_ull_le_hid_srv_handle_set_volume(p_vol);
                }                            
            }
        }
    } else if (BT_ULL_STREAMING_INTERFACE_MICROPHONE == p_vol->streaming.streaming_interface 
            ||BT_ULL_STREAMING_INTERFACE_LINE_OUT == p_vol->streaming.streaming_interface) {
    if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
        p_vol->streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        bt_ull_le_hid_srv_handle_set_volume(p_vol);
    }
    if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT) {
        p_vol->streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
        bt_ull_le_hid_srv_handle_set_volume(p_vol);
    }

        //bt_ull_le_hid_srv_handle_set_volume(vol);
    }
#else
if ((BT_ULL_STREAMING_INTERFACE_UNKNOWN < p_vol->streaming.streaming_interface) &&
    (BT_ULL_STREAMING_INTERFACE_MAX_NUM > p_vol->streaming.streaming_interface)) {
    bt_ull_transmitter_t trans = bt_ull_le_srv_get_transmitter_by_stream_interface(&p_vol->streaming);
    bt_ull_le_at_set_volume(trans, p_vol->action, p_vol->channel, p_vol->volume);
} else {
    ull_assert(0 && "unknown transmitter type");
    return;
}
#endif

}

static void bt_ull_le_hid_srv_l2cap_rx_mute_hdl(bt_ull_streaming_t *streaming, bool is_mute)
{
    if (!streaming) {
        return;
    }

    ull_report(BT_ULL_HID_LOG"srv_l2cap_rx_mute_hdl, if_type: 0x%x, port:0x%x, mute: %d", 3, \
        streaming->streaming_interface, streaming->port, is_mute);
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    if (BT_ULL_ROLE_SERVER == ctx->role) {
        if (BT_ULL_MIC_CLIENT == ctx->audio_sink.client_type) {
            /* Wirless Mic RX not mute */
            /* Notify APP to update the UI */
        } else {
            /* dongle mute */
            bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
            if ((BT_ULL_GAMING_TRANSMITTER <= trans_type) && (BT_ULL_TRANSMITTER_MAX_NUM > trans_type)) {
                stream_ctx->server.stream[trans_type].is_mute = is_mute;
                bt_ull_le_at_set_mute(trans_type, is_mute);
            }
        }
    } else if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if ((BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)
            || (BT_ULL_STREAMING_INTERFACE_LINE_OUT == streaming->streaming_interface)
            || (BT_ULL_STREAMING_INTERFACE_I2S_OUT == streaming->streaming_interface)) {
            stream_ctx->client.ul.is_mute = is_mute;
            bt_ull_le_am_set_mute(BT_ULL_LE_AM_UL_MODE, is_mute);
        } else {
            stream_ctx->client.dl.is_mute = is_mute;
            bt_ull_le_am_set_mute(BT_ULL_LE_AM_DL_MODE, is_mute);
        }
    }

}

static void bt_ull_le_hid_srv_l2cap_rx_sample_rate_change_hdl(bt_ull_le_srv_sample_rate_change_t *sample_rate)
{
    if (sample_rate) {
        ull_report(BT_ULL_HID_LOG"rx recieved BT_ULL_EVENT_SAMPLE_RATE_CHANGE, sample_rate: %d", 1, sample_rate->sample_rate);
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

}

static void bt_ull_le_hid_srv_l2cap_rx_restart_streaming_hdl(bt_ull_le_srv_notify_restart_streaming_t *restart)
{
    if (!restart) {
        return;
    }
    ull_report(BT_ULL_HID_LOG"BT_ULL_EVENT_RESTART_STREAMING_IND, reason: %d, mode: %x", 2, restart->reason, restart->mode_mask);
     bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
     bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if ((bt_ull_le_hid_srv_is_device_connected(BT_ULL_LE_HID_SRV_DEVICE_HEADSET) || bt_ull_le_hid_srv_is_device_connected(BT_ULL_LE_HID_SRV_DEVICE_EARBUDS))) {
            /* restart ull codec, if we are playing ull to swith latency */
            if ((restart->mode_mask & (1 << BT_ULL_LE_STREAM_MODE_DOWNLINK)) && \
                (stream_ctx->client.dl.is_am_open)) {
                ctx->audio_sink.restart_streaming_mask |= (1 << BT_ULL_LE_STREAM_MODE_DOWNLINK);
                bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true);
                //bt_ull_le_am_restart(BT_ULL_LE_AM_DL_MODE);
            }
            if ((restart->mode_mask & (1 << BT_ULL_LE_STREAM_MODE_UPLINK)) && \
                stream_ctx->client.ul.is_am_open) {
                ctx->audio_sink.restart_streaming_mask |= (1 << BT_ULL_LE_STREAM_MODE_UPLINK);
                bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
                //bt_ull_le_am_restart(BT_ULL_LE_AM_UL_MODE);
            }
        }
    }

}

static void bt_ull_le_hid_srv_l2cap_rx_data_hdl(bt_handle_t handle, uint16_t len, uint8_t *rx_data, uint8_t data_type)
{
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx || !rx_data) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_user_data_hdl, invalid handle!", 0);
        return;
    }
    bt_ull_event_t evt = 0x00;
    uint8_t *data = rx_data;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_user_data_t user_data;
    bt_ull_le_srv_memcpy(user_data.remote_address, ctx->link[idx].peer_addr.addr, sizeof(bt_bd_addr_t));
    switch (data_type) {
        case BT_ULL_EVENT_USER_DATA: {
            evt = BT_ULL_EVENT_USER_DATA_IND;
            user_data.user_data_length = *(uint16_t *)(data);
            data += sizeof(user_data.user_data_length);
            user_data.user_data = data;
            bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, evt, &user_data, sizeof(bt_ull_user_data_t));
            break;
        }
        case BT_ULL_LE_HID_MSG_INPUT_REPORT_DATA: {
            evt = BT_ULL_EVENT_LE_HID_INPUT_REPORT_IND;
            user_data.user_data_length = len;
            user_data.user_data = data;
            bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, evt, &user_data, sizeof(bt_ull_user_data_t));
            break;
        }
        case BT_ULL_LE_HID_MSG_RACE_DATA: {
            evt = BT_ULL_EVENT_LE_HID_RACE_DATA_IND;
            user_data.user_data_length = len;
            user_data.user_data = data;
            bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_AIR_SRV, evt, &user_data, sizeof(bt_ull_user_data_t));
            break;
        }
        default: {
            //assert(0);
            return;
        }
    }

}

static void bt_ull_le_hid_srv_l2cap_rx_switch_link_hdl(bt_handle_t handle, bt_ull_le_hid_srv_change_link_req_t *change_link)
{
    if (!change_link) {
        return;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(handle);
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    ull_report(BT_ULL_HID_LOG"rx_switch_link_hdl! state: 0x%x, idx: 0x%x, dt: %d", 3, state, idx, bt_ull_le_hid_srv_get_device_type_by_idx(idx));
    if (state < BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    if (BT_ULL_ROLE_SERVER == ctx->role) {
        bt_ull_le_hid_srv_switch_link_mode_ind_t switch_link;
        switch_link.handle = handle;
        switch_link.link_mode = change_link->mode;
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_SWITCH_LINK_MODE_IND, &switch_link, sizeof(bt_ull_le_hid_srv_switch_link_mode_ind_t));
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_AIR_SRV, BT_ULL_EVENT_LE_HID_SWITCH_LINK_MODE_IND, &switch_link, sizeof(bt_ull_le_hid_srv_switch_link_mode_ind_t));
        return;
    }
}

static void bt_ull_le_hid_srv_l2cap_rx_hdl(bt_handle_t handle, uint8_t *data, uint16_t len) 
{
    if (BT_HANDLE_INVALID == handle) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_data_hdl, invalid handle!", 0);
        return;
    }
    //bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(handle);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx || !data || !len) {
        ull_report_error(BT_ULL_HID_LOG"l2cap_rx_data_hdl, no link resource!!", 0);
        return;
    }
    uint8_t device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
    uint8_t *msg = (uint8_t *)data;
    bt_ull_le_hid_srv_msg_t msg_id = msg[0];
    ull_report(BT_ULL_HID_LOG"rx data! msg id: 0x%x, conn_handle: 0x%x, dt: %d", 3, msg_id, handle, device_type);
    switch (msg_id) {
        case BT_ULL_LE_HID_MSG_RESPONSE: {
            bt_ull_le_hid_srv_rsp_t *rsp = (bt_ull_le_hid_srv_rsp_t *)(msg + sizeof(bt_ull_le_hid_srv_msg_t));
            bt_ull_le_hid_srv_l2cap_rx_response_hdl(handle, rsp);
            break;
        }
        case BT_ULL_LE_HID_MSG_SERVICE_CONNECT_REQ: {
            bt_ull_le_hid_srv_l2cap_rx_service_connect_req_hdl(handle);
            break;
        }
        case BT_ULL_LE_HID_MSG_SYNC_CONTEXT_INFO: {
            bt_ull_le_hid_srv_context_info_req_t *info = (bt_ull_le_hid_srv_context_info_req_t *)(msg + sizeof(bt_ull_le_hid_srv_msg_t));
            bt_ull_le_hid_srv_l2cap_rx_sync_context_info_hdl(handle, info);
            break;
        }
        case BT_ULL_LE_HID_MSG_BONDING_REQ: {
            bt_ull_le_hid_srv_l2cap_rx_bonding_req_hdl(handle);
            break;
        }
        case BT_ULL_LE_HID_MSG_SYNC_BONDING_INFO: {
            bt_ull_le_hid_srv_bonding_info_t *info = (bt_ull_le_hid_srv_bonding_info_t *)(msg + sizeof(bt_ull_le_hid_srv_msg_t));
            bt_ull_le_hid_srv_l2cap_rx_sync_bonding_info_hdl(handle, info);
            break;
        }
        case BT_ULL_EVENT_STREAMING_START_IND: {
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)(msg + sizeof(bt_ull_req_event_t));
            bt_ull_le_hid_srv_l2cap_rx_start_streaming_hdl(streaming);
            break;
        }
        case BT_ULL_EVENT_STREAMING_STOP_IND: {
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)(msg+ sizeof(bt_ull_req_event_t));
            bt_ull_le_hid_srv_l2cap_rx_stop_streaming_hdl(streaming);
            break;
        }
        case BT_ULL_EVENT_VOLUME_IND: {
            bt_ull_volume_ind_t *p_vol = (bt_ull_volume_ind_t *)(msg+ sizeof(bt_ull_req_event_t));
            bt_ull_le_hid_srv_l2cap_rx_volume_ind_hdl(p_vol);
            break;
        }
        case BT_ULL_EVENT_VOLUME_MUTE: {
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)(msg + sizeof(bt_ull_req_event_t));
            bt_ull_le_hid_srv_l2cap_rx_mute_hdl(streaming, true);
            break;
        }
        case BT_ULL_EVENT_VOLUME_UNMUTE: {
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t *)(msg + sizeof(bt_ull_req_event_t));
            bt_ull_le_hid_srv_l2cap_rx_mute_hdl(streaming, false);
            break;
        }
        case BT_ULL_EVENT_VOLUME_ACTION: {
            bt_ull_volume_t *vol = (bt_ull_volume_t *)(msg+ sizeof(bt_ull_req_event_t));
            bt_ull_le_hid_srv_l2cap_rx_volume_hdl(vol);
            break;
        }
        case BT_ULL_EVENT_MIX_RATIO_ACTION: {
            bt_ull_mix_ratio_t *mix_ratio = (bt_ull_mix_ratio_t *)(msg+ sizeof(bt_ull_req_event_t));
            bt_ull_le_hid_srv_handle_set_streaming_mix_ratio_hdl(mix_ratio);
            break;
        }
        case BT_ULL_EVENT_SAMPLE_RATE_CHANGE: {
            bt_ull_le_srv_sample_rate_change_t *sample_rate = (bt_ull_le_srv_sample_rate_change_t *)(msg+ sizeof(bt_ull_req_event_t));
            bt_ull_le_hid_srv_l2cap_rx_sample_rate_change_hdl(sample_rate);

            break;
        }
        case BT_ULL_EVENT_USB_HID_CONTROL_ACTION: {
            bt_ull_usb_hid_control_t control_id = msg[1];
            bt_ull_le_hid_srv_handle_usb_audio_control(control_id);
            break;
        }
        case BT_ULL_EVENT_RESTART_STREAMING_IND: {
            bt_ull_le_srv_notify_restart_streaming_t *restart = \
                (bt_ull_le_srv_notify_restart_streaming_t *)(msg + sizeof(bt_ull_req_event_t));
            bt_ull_le_hid_srv_l2cap_rx_restart_streaming_hdl(restart);
            break;
        }
        //case BT_ULL_LE_HID_MSG_INPUT_REPORT_DATA:
        case BT_ULL_LE_HID_MSG_RACE_DATA:
        case BT_ULL_EVENT_USER_DATA: {
            uint8_t *data = msg + sizeof(bt_ull_le_hid_srv_msg_t);
            bt_ull_le_hid_srv_l2cap_rx_data_hdl(handle, len - sizeof(bt_ull_le_hid_srv_msg_t), data, msg_id);
            break;
        }
        case BT_ULL_LE_HID_MSG_SWITCH_LINK_MODE: {
            bt_ull_le_hid_srv_change_link_req_t *switch_link = (bt_ull_le_hid_srv_change_link_req_t *)(msg + sizeof(bt_ull_le_hid_srv_msg_t));
            bt_ull_le_hid_srv_l2cap_rx_switch_link_hdl(handle, switch_link);
            break;
        }
        default:
            break;

    }

}

static bt_status_t bt_ull_le_hid_srv_tx_critical_user_data(bt_ull_tx_critical_user_data_t *tx_data)//only for headset or earbuds
{
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t i = 0;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_avm_critial_data_t critical_data = {0};
    critical_data.seq = ctx->audio_sink.critical_data_tx_seq;
    critical_data.length = tx_data->user_data_length;
    critical_data.data = tx_data->user_data;
    critical_data.is_le_data = true;

    for (i = 0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i++) {
        if ((BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED <= ctx->link[i].state) && \
            (BT_ULL_LE_HID_SRV_DEVICE_HEADSET == ctx->link[i].device_type || \
            BT_ULL_LE_HID_SRV_DEVICE_EARBUDS == ctx->link[i].device_type)) {
           status = bt_ull_le_hid_srv_critial_data_send(ctx->link[i].acl_handle, tx_data->flush_timeout, &critical_data);
           if (BT_STATUS_SUCCESS != status) {
               return status;
           }
        }
    }
    if (BT_STATUS_SUCCESS == status) {
        if (0xFF == ctx->audio_sink.critical_data_tx_seq) {
            ctx->audio_sink.critical_data_tx_seq = 0x01;
        } else {
            ctx->audio_sink.critical_data_tx_seq++;
        }
    }
    return status;
}

static bt_status_t bt_ull_le_hid_srv_tx_data_req(uint8_t *tx_data, uint8_t data_type)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (!tx_data) {
        return status;
    }
    bt_ull_user_data_t *user_data = NULL;
    //bt_ull_le_hid_srv_input_report_data_t *hogp_data = NULL;
    bt_ull_le_hid_srv_race_data_t *race_data = NULL;
    uint8_t idx = BT_ULL_LE_HID_SRV_INVALID_VALUE;
    uint8_t i = 0;
    uint8_t *data = NULL;
    uint16_t data_len = 0x0;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    switch (data_type) {
        case BT_ULL_ACTION_TX_USER_DATA: {
            user_data = (bt_ull_user_data_t *)tx_data;
            for (i =0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i ++) {
                if (!bt_ull_le_srv_memcmp(&ctx->link[i].peer_addr.addr, user_data->remote_address, sizeof(bt_bd_addr_t))) {
                    idx = i;
                }
            }
            data_len = user_data->user_data_length + sizeof(user_data->user_data_length)  + sizeof(bt_ull_le_hid_srv_msg_t);
            data = (uint8_t*)bt_ull_le_srv_memory_alloc(data_len);
            ull_assert("OOM" && (NULL != data));
            ull_assert(user_data->user_data_length && user_data->user_data);
            data[0] = BT_ULL_EVENT_USER_DATA;
            bt_ull_le_srv_memcpy(&data[1], &(user_data->user_data_length), sizeof(user_data->user_data_length));
            bt_ull_le_srv_memcpy(&data[3], user_data->user_data, user_data->user_data_length);
            break;
        }
/*
        case BT_ULL_ACTION_LE_HID_TX_HOGP_DATA_REQ: {
            hogp_data = (bt_ull_le_hid_srv_input_report_data_t *)tx_data;
            for (i =0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i ++) {
                if (!bt_ull_le_srv_memcmp(&ctx->link[i].peer_addr.addr, hogp_data->remote_address, sizeof(bt_bd_addr_t))) {
                    idx = i;
                }
            }
            data_len = hogp_data->user_data_length + sizeof(hogp_data->user_data_length)  + sizeof(bt_ull_le_hid_srv_msg_t);
            data = (uint8_t*)bt_ull_le_srv_memory_alloc(data_len);
            ull_assert("OOM" && (NULL != data));
            ull_assert(hogp_data->user_data_length && hogp_data->user_data);
            data[0] = BT_ULL_LE_HID_MSG_INPUT_REPORT_DATA;
            bt_ull_le_srv_memcpy(&data[1], &(hogp_data->user_data_length), sizeof(hogp_data->user_data_length));
            bt_ull_le_srv_memcpy(&data[3], hogp_data->user_data, hogp_data->user_data_length);
            break;
        }
*/
        case BT_ULL_ACTION_LE_HID_TX_RACE_DATA: {
            race_data = (bt_ull_le_hid_srv_input_report_data_t *)tx_data;
            for (i =0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i ++) {
                if (!bt_ull_le_srv_memcmp(&ctx->link[i].peer_addr.addr, race_data->remote_address, sizeof(bt_bd_addr_t))) {
                    idx = i;
                }
            }
            //data_len = race_data->user_data_length + sizeof(race_data->user_data_length)  + sizeof(bt_ull_le_hid_srv_msg_t);
            data_len = race_data->user_data_length + sizeof(bt_ull_le_hid_srv_msg_t);
            data = (uint8_t*)bt_ull_le_srv_memory_alloc(data_len);
            ull_assert("OOM" && (NULL != data));
            ull_assert(race_data->user_data_length && race_data->user_data);
            data[0] = BT_ULL_LE_HID_MSG_RACE_DATA;
            //bt_ull_le_srv_memcpy(&data[1], &(race_data->user_data_length), sizeof(race_data->user_data_length));
            bt_ull_le_srv_memcpy(&data[1], race_data->user_data, race_data->user_data_length);
            break;
        }
        default:
            break;
    }

    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx || !data) {
        if (data) {
            bt_ull_le_srv_memory_free(data);
        }
        ull_report_error(BT_ULL_HID_LOG"srv_tx_data_req, idx or data error", 0);
        return status;
    }
    bt_handle_t handle = bt_ull_le_hid_srv_get_link_handle(idx);
    status = bt_ull_le_hid_srv_l2cap_send_data(handle, data, data_len);
    ull_report(BT_ULL_HID_LOG"srv_tx_data_req, status: %d, handle: %d, data_type: %d", 3, status, handle, data_type);
    bt_ull_le_srv_memory_free(data);
    return status;
}

static bt_status_t bt_ull_le_hid_srv_switch_link_mode_req(bt_ull_le_hid_srv_switch_link_mode_t *req)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (!req) {
        return BT_STATUS_FAIL;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_handle(req->handle);
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
    ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_switch_link_mode_req, idx: %d, state: %d", 2, idx, state);
    if (state < BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED) {
        return BT_STATUS_FAIL;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    if (BT_ULL_ROLE_SERVER == ctx->role) {
        bt_ull_le_hid_srv_switch_link_mode_ind_t switch_link;
        switch_link.handle = req->handle;
        switch_link.link_mode = req->link_mode;
        bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_HID_SWITCH_LINK_MODE_IND, &switch_link, sizeof(bt_ull_le_hid_srv_switch_link_mode_ind_t));
        return BT_STATUS_SUCCESS;
    }
    uint16_t len = sizeof(bt_ull_le_hid_srv_msg_t) + sizeof(bt_ull_le_hid_srv_change_link_req_t);
    uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
    ull_assert("out of memory" && (data != NULL));
    
    data[0] = BT_ULL_LE_HID_MSG_SWITCH_LINK_MODE;
    bt_ull_le_hid_srv_change_link_req_t *switch_link = (bt_ull_le_hid_srv_change_link_req_t *)(data + sizeof(bt_ull_le_hid_srv_msg_t));
    switch_link->mode = req->link_mode;
    status = bt_ull_le_hid_srv_l2cap_send_data(req->handle, data, len);
    if (BT_STATUS_SUCCESS == status) {
        
    } else {
        ull_report_error(BT_ULL_HID_LOG"Switch link mode fail!!", 0);
    }

    bt_ull_le_srv_memory_free(data);
    return status;
}

static bt_status_t bt_ull_le_hid_srv_set_scenario(bt_ull_le_hid_srv_app_scenario_t scenario)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ull_report("[ULL][LE] bt_ull_le_hid_srv_set_scenario, set scenario :%d, curr scenario: %d", 2, scenario, ctx->scenario);
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        return BT_STATUS_FAIL;
    }
    if (BT_ULL_LE_HID_SRV_APP_SCENARIO_NONE == scenario) {
        return BT_STATUS_FAIL;
    }
    if (scenario == ctx->scenario) {
        return BT_STATUS_SUCCESS;
    }
    ctx->scenario = scenario;
    return bt_ull_le_hid_conn_srv_change_scenario(scenario);
}

static bt_status_t bt_ull_le_hid_srv_set_idle_time(bt_ull_le_hid_srv_idle_time_t idle_time)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ull_report("[ULL][LE] bt_ull_le_hid_srv_set_scenario, set idle_time :%d, curr idle_time: %d", 2, idle_time, ctx->idle_time);
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        return BT_STATUS_FAIL;
    }
    if (idle_time == ctx->idle_time) {
        return BT_STATUS_SUCCESS;
    }
    ctx->idle_time = idle_time;
    return bt_ull_le_hid_conn_srv_set_idle_time(idle_time);
}

bt_status_t bt_ull_le_hid_srv_cancel_connect_hdl(bt_ull_le_hid_srv_cancel_connect_t *cancel)
{
    if (!cancel) {
        return BT_STATUS_FAIL;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_dt(cancel->device_type);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_cancel_connect_hdl, not found dt: %x", 1, cancel->device_type);
        return BT_STATUS_FAIL;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
            ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_cancel_connect_hdl, dt: %x, state: %d", 2, cancel->device_type, state);
    if (BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTING != state) {
        return BT_STATUS_FAIL;
    }
    return bt_ull_le_hid_conn_srv_cancel_create_air_cis(cancel->device_type);
}

bt_status_t bt_ull_le_hid_srv_cancel_create_sync_hdl(bt_ull_le_hid_srv_cancel_create_sync_t *cancel)
{
    if (!cancel) {
        return BT_STATUS_FAIL;
    }
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_dt(cancel->device_type);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_cancel_create_sync_hdl, not found dt: %x", 1, cancel->device_type);
        return BT_STATUS_FAIL;
    }
    uint8_t state = bt_ull_le_hid_srv_get_link_state(idx);
            ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_cancel_create_sync_hdl, dt: %x, state: %d", 2, cancel->device_type, state);
    if (BT_ULL_LE_HID_SRV_LINK_STATE_CIS_CONNECTING != state) {
        return BT_STATUS_FAIL;
    }
    return bt_ull_le_hid_conn_srv_cancel_sync_air_cis(cancel->device_type);
}

bt_status_t bt_ull_le_hid_srv_action(bt_ull_action_t action, const void *param, uint32_t param_len)
{
    bt_status_t status;
    BT_ULL_MUTEX_LOCK();
    ull_report(BT_ULL_HID_LOG"bt_ull_le_hid_srv_action, action: %x", 1, action);
    switch (action) {
        case BT_ULL_ACTION_LE_HID_CONNECT: {
            bt_ull_le_hid_srv_connect_t *req = (bt_ull_le_hid_srv_connect_t *)param;
            status = bt_ull_le_hid_srv_create_connection_req(req);
            break;
        }
        case BT_ULL_ACTION_LE_HID_DISCONNECT: {
            bt_ull_le_hid_srv_disconnect_t *req = (bt_ull_le_hid_srv_disconnect_t *)param;
            status = bt_ull_le_hid_srv_disconnect_req(req);
            break;
        }
        case BT_ULL_ACTION_LE_HID_BOND: {
            bt_ull_le_hid_srv_bond_t *req = (bt_ull_le_hid_srv_bond_t *)param;
            status = bt_ull_le_hid_srv_start_bonding_req(req);
            break;
        }
        case BT_ULL_ACTION_LE_HID_CONNECT_SERVICE: {
            bt_ull_le_hid_srv_connect_service_t *req = (bt_ull_le_hid_srv_connect_service_t *)param;
            status = bt_ull_le_hid_srv_connect_service_req(req);
            break;
        }
        case BT_ULL_ACTION_LE_HID_SWITCH_LINK_MODE: {
            bt_ull_le_hid_srv_switch_link_mode_t *req = (bt_ull_le_hid_srv_switch_link_mode_t *)param;
            status = bt_ull_le_hid_srv_switch_link_mode_req(req);
            break;
        }
        case BT_ULL_ACTION_LE_HID_CREATE_SYNC: {
            bt_ull_le_hid_srv_create_sync_t *req = (bt_ull_le_hid_srv_create_sync_t *)param;
            status = bt_ull_le_hid_srv_create_sync_req(req);
            break;
        }
        case BT_ULL_ACTION_START_STREAMING: {
            ull_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t*)param;
            status = bt_ull_le_hid_srv_handle_start_streaming(streaming);
            break;
        }
        case BT_ULL_ACTION_STOP_STREAMING: {
            ull_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t*)param;
            status = bt_ull_le_hid_srv_handle_stop_streaming(streaming);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_VOLUME: {
            ull_assert(param);
            bt_ull_volume_t *volume = (bt_ull_volume_t*)param;
            status = bt_ull_le_hid_srv_handle_set_volume(volume);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_MUTE: {
            ull_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t*)param;
            status = bt_ull_le_hid_srv_handle_set_streaming_mute(streaming, true);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_UNMUTE: {
            ull_assert(param);
            bt_ull_streaming_t *streaming = (bt_ull_streaming_t*)param;
            status = bt_ull_le_hid_srv_handle_set_streaming_mute(streaming, false);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE: {
            ull_assert(param);
            bt_ull_sample_rate_t *sample_rate = (bt_ull_sample_rate_t*)param;
            status = bt_ull_le_hid_srv_handle_set_streaming_sample_rate(sample_rate);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_SAMPLE_SIZE: {
            ull_assert(param);
            bt_ull_streaming_sample_size_t *sample_size = (bt_ull_streaming_sample_size_t*)param;
            status = bt_ull_le_hid_srv_handle_set_streaming_sample_size(sample_size);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_SAMPLE_CHANNEL: {
            ull_assert(param);
            bt_ull_streaming_sample_channel_t *sample_channel = (bt_ull_streaming_sample_channel_t*)param;
            status = bt_ull_le_hid_srv_handle_set_streaming_sample_channel(sample_channel);
            break;
        }
        case BT_ULL_ACTION_SET_STREAMING_MIX_RATIO: {
            ull_assert(param);
            bt_ull_mix_ratio_t *ratio = (bt_ull_mix_ratio_t*)param;
            status = bt_ull_le_hid_srv_handle_set_streaming_mix_ratio_hdl(ratio);
            break;
        }
        case BT_ULL_ACTION_USB_HID_CONTROL: {
            ull_assert(param);
            bt_ull_usb_hid_control_t control_id = *((bt_ull_usb_hid_control_t*)param);
            status = bt_ull_le_hid_srv_handle_usb_audio_control(control_id);
            break;
        }
        case BT_ULL_ACTION_INIT_CRITICAL_USER_DATA: {
            if (!param) {
                return BT_STATUS_SUCCESS;
            }
            bt_ull_init_critical_user_data_t *max_len = (bt_ull_init_critical_user_data_t*)param;
            status = bt_ull_le_hid_srv_critial_data_init(max_len);
            break;
        }
        case BT_ULL_ACTION_TX_CRITICAL_USER_DATA: {
            bt_ull_tx_critical_user_data_t *tx_data = (bt_ull_tx_critical_user_data_t *)param;
            status = bt_ull_le_hid_srv_tx_critical_user_data(tx_data);
            break;
        }
        case BT_ULL_ACTION_TX_USER_DATA:
        //case BT_ULL_ACTION_LE_HID_TX_HOGP_DATA_REQ:
        case BT_ULL_ACTION_LE_HID_TX_RACE_DATA: {
            uint8_t *tx_data = (uint8_t *)param;
            status = bt_ull_le_hid_srv_tx_data_req(tx_data, action);
            break;
        }
        case BT_ULL_ACTION_LE_HID_CONTROL_RGB: {
            bt_ull_le_hid_srv_control_indicater_led_t *led_control = (bt_ull_le_hid_srv_control_indicater_led_t *)param;
            status = bt_ull_le_hid_srv_led_control_hdl(led_control);
            break;
        }
       case BT_ULL_ACTION_LE_HID_CHANGE_REPORT_RATE_LEVEL: {
           bt_ull_le_hid_srv_report_rate_level_t *rr_level = (bt_ull_le_hid_srv_report_rate_level_t *)param;
           status = bt_ull_le_hid_conn_srv_set_report_rate(*rr_level);
           break;
       }
       case BT_ULL_ACTION_LE_HID_SET_SCENARIO: {
           bt_ull_le_hid_srv_app_scenario_t *scenario = (bt_ull_le_hid_srv_app_scenario_t *)param;
           status = bt_ull_le_hid_srv_set_scenario(*scenario);
           break;
       }

       case BT_ULL_ACTION_LE_HID_CANCEL_CONNECT: {
           bt_ull_le_hid_srv_cancel_connect_t *cancel = (bt_ull_le_hid_srv_cancel_connect_t *)param;
           status = bt_ull_le_hid_srv_cancel_connect_hdl(cancel);
           break;
       }
       case BT_ULL_ACTION_LE_HID_CANCEL_CREATE_SYNC: {
           bt_ull_le_hid_srv_cancel_create_sync_t *cancel = (bt_ull_le_hid_srv_cancel_create_sync_t *)param;
           status = bt_ull_le_hid_srv_cancel_create_sync_hdl(cancel);
           break;
       }
       case BT_ULL_ACTION_LE_HID_SET_IDLE_TIME: {
           bt_ull_le_hid_srv_idle_time_t *idle_time = (bt_ull_le_hid_srv_idle_time_t *)param;
           status = bt_ull_le_hid_srv_set_idle_time(*idle_time);
           break;
       }
        default:
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
    return status;
}

static bt_status_t bt_ull_le_hid_srv_send_data_by_device_type(bt_ull_le_hid_srv_device_t device_type, uint8_t *packet, uint16_t packet_size)
{
    ull_report("[ULL][LE] bt_ull_le_hid_srv_send_data_by_device_type, device_type :%d, packet_size: %d", 2, device_type, packet_size);
    bt_status_t result = BT_STATUS_FAIL;
    uint8_t i;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    for (i = 0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i++) {
        if (BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED <= ctx->link[i].state && (device_type == ctx->link[i].device_type)) {
            result = bt_ull_le_hid_srv_l2cap_send_data(ctx->link[i].acl_handle, packet, packet_size);
        }
    }
    return result;
}

static bool bt_ull_le_hid_srv_is_device_connected(bt_ull_le_hid_srv_device_t dt)
{
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_dt(dt);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx) {
        return false;
    }
    bt_ull_le_hid_srv_link_state_t state = bt_ull_le_hid_srv_get_link_state(idx);
    if (BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED <= state) {
        return true;
    }
    return false;
}
static bt_status_t bt_ull_le_hid_srv_handle_start_streaming(bt_ull_streaming_t *streaming)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_client_t client_type = ctx->audio_sink.client_type;
    bt_ull_le_hid_srv_device_t device_type = (BT_ULL_HEADSET_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_HEADSET : ((BT_ULL_EARBUDS_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_EARBUDS : BT_ULL_LE_HID_SRV_DEVICE_NONE);
    //ull_assert(device_type);
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_handle_start_streaming, streaming_if :0x%x, port: 0x%x", 2, streaming->streaming_interface, streaming->port);

    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if (bt_ull_le_hid_srv_is_device_connected(device_type) && (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)) {
            stream_ctx->streaming_port |= BT_ULL_LE_STREAM_PORT_MASK_MIC;
            status = bt_ull_le_hid_srv_play_am(streaming, BT_ULL_LE_STREAM_MODE_UPLINK);
        }  else {
             ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_handle_start_streaming, role is client, but port type is un-supported", 0);
        }
    } else if (BT_ULL_ROLE_SERVER == ctx->role) {
        uint8_t open_now = false;
        uint32_t sampe_rate = 0;
        bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
        bt_ull_le_stream_port_mask_t start_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
        stream_ctx->streaming_port |= start_port;
        if (ctx->audio_sink.is_streaming_locked) {
            stream_ctx->locking_port |= start_port;
        }
        bt_ull_le_srv_server_stream_t *stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
        if (bt_ull_le_hid_srv_is_device_connected(device_type)) {
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
            ull_report(BT_ULL_HID_LOG" Open Audio streaming now ? open_now: %d, sample_rate: %d, need restart: %d", 3, open_now, sampe_rate, stream_info->is_need_restart);
            if (open_now) {
                if ((stream_info->is_need_restart) && (!ctx->audio_sink.is_streaming_locked)) {
                    bt_ull_le_at_restart(trans_type);
                    stream_info->is_need_restart = false;
                }
                status = bt_ull_le_hid_srv_sync_streaming_status(start_port, BT_ULL_EVENT_STREAMING_START_IND);
            }
        } else {
            ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_handle_start_streaming fail! have not any connected device", 0);
        }
    }
    return status;
}




static bt_status_t bt_ull_le_hid_srv_handle_stop_streaming(bt_ull_streaming_t *streaming)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_server_stream_t *stream_info;
    bt_ull_le_stream_port_mask_t stop_port = 0;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_client_t client_type = ctx->audio_sink.client_type;
    bt_ull_le_hid_srv_device_t device_type = (BT_ULL_HEADSET_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_HEADSET : ((BT_ULL_EARBUDS_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_EARBUDS : BT_ULL_LE_HID_SRV_DEVICE_NONE);
    //ull_assert(device_type);
    ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_handle_stop_streaming, streaming_if :0x%x, port: 0x%x", 2, streaming->streaming_interface, streaming->port);

    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        if (bt_ull_le_hid_srv_is_device_connected(device_type) && (BT_ULL_STREAMING_INTERFACE_MICROPHONE == streaming->streaming_interface)) {
            stream_ctx->streaming_port &= ~BT_ULL_LE_STREAM_PORT_MASK_MIC;
            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_MIC;
            }
            /* disable mic if ull is playing */
            bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);

            ull_report(BT_ULL_HID_LOG" client recieved disable mic, curr_stream_state:0x%x, ", 1, ctx->audio_sink.curr_stream_state);
        } else {
            ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_handle_stop_streaming, role is client, but port type is un-supported", 0);
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

        if (bt_ull_le_hid_srv_is_device_connected(device_type)) {
            bt_ull_le_hid_srv_sync_streaming_status(stop_port, BT_ULL_EVENT_STREAMING_STOP_IND);
        } else {
            ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_handle_stop_streaming fail! have not any connected device", 0);
        }

        if (stream_info) {
            if (BT_STATUS_SUCCESS != bt_ull_le_at_stop(trans_type, true)) {
                ull_report_error(BT_ULL_HID_LOG" [Error] bt_ull_le_hid_srv_handle_stop_streaming, stop transmitter fail",0);
            }
        }
    }

    return status;
}



static bt_status_t bt_ull_le_hid_srv_handle_set_volume(bt_ull_volume_t *vol)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_client_t client_type = ctx->audio_sink.client_type;
    bt_ull_le_hid_srv_device_t device_type = (BT_ULL_HEADSET_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_HEADSET : ((BT_ULL_EARBUDS_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_EARBUDS : BT_ULL_LE_HID_SRV_DEVICE_NONE);
    //ull_assert(device_type);
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_handle_set_volume, streaming_if:0x%x, port:0x%x, action:0x%x, audio_channel: 0x%x, volume_level:%d", 5,
        vol->streaming.streaming_interface, vol->streaming.port, vol->action, vol->channel, vol->volume);

    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        /* check the all link*/
        //TODO: 
        uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_volume_t);
        uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
        if (NULL != data) {
            data[0] = BT_ULL_EVENT_VOLUME_ACTION;
            bt_ull_volume_t *vol_action = (bt_ull_volume_t *)(data + sizeof(bt_ull_req_event_t));
            bt_ull_le_srv_memcpy(vol_action, vol, sizeof(bt_ull_volume_t));
            status = bt_ull_le_hid_srv_send_data_by_device_type(device_type, (uint8_t*)data, len);
            bt_ull_le_srv_memory_free(data);
        }
        
    } else if (BT_ULL_ROLE_SERVER == ctx->role) {
        bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(&(vol->streaming));
        if (BT_STATUS_SUCCESS == bt_ull_le_at_set_volume(trans_type, vol->action, vol->channel, vol->volume)) {
            ull_report(BT_ULL_HID_LOG" bt_ull_le_at_set_volume success", 0);
        }
        if (BT_ULL_STREAMING_INTERFACE_SPEAKER == vol->streaming.streaming_interface
            || BT_ULL_STREAMING_INTERFACE_LINE_IN == vol->streaming.streaming_interface
            || BT_ULL_STREAMING_INTERFACE_I2S_IN == vol->streaming.streaming_interface) {
            //stream_ctx->server_chat_volume.vol_left = vol->volume;
            if ((BT_ULL_AUDIO_CHANNEL_DUAL == vol->channel) ||
                (BT_ULL_AUDIO_CHANNEL_LEFT == vol->channel)) {
                stream_ctx->client.dl.volume.vol_left = vol->volume;              
            }
            if ((BT_ULL_AUDIO_CHANNEL_DUAL == vol->channel) ||
                (BT_ULL_AUDIO_CHANNEL_RIGHT == vol->channel)) {
                stream_ctx->client.dl.volume.vol_right= vol->volume;              
            }
        }

#if 1
        /* check the all link */
        /* sync Server volume to client */

        //TODO:
        /* Sync PC volume to client */
        uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_volume_ind_t);
        uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
        if (NULL != data) {
            data[0] = BT_ULL_EVENT_VOLUME_IND;
            bt_ull_volume_ind_t *p_vol = (bt_ull_volume_ind_t *)(data + sizeof(bt_ull_req_event_t));
            bt_ull_le_srv_memcpy(&(p_vol->streaming), &(vol->streaming), sizeof(p_vol->streaming));
            p_vol->channel = vol->channel;
            if ((BT_ULL_AUDIO_CHANNEL_DUAL == vol->channel) ||
                (BT_ULL_AUDIO_CHANNEL_LEFT == vol->channel)) {
                p_vol->vol.vol_left = vol->volume;
                //p_vol->vol_gain.vol_left = vol->gain;
            }
            if ((BT_ULL_AUDIO_CHANNEL_DUAL == vol->channel) ||
                (BT_ULL_AUDIO_CHANNEL_RIGHT == vol->channel)) {
                 p_vol->vol.vol_right = vol->volume;
                 //p_vol->vol_gain.vol_right = vol->gain;
            }
            status = bt_ull_le_hid_srv_send_data_by_device_type(device_type, (uint8_t*)data, len);
            bt_ull_le_srv_memory_free(data);
        }
#endif
    }
    return status;
}



static bt_status_t bt_ull_le_hid_srv_handle_set_streaming_mute(bt_ull_streaming_t *streaming, bool is_mute)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_client_t client_type = ctx->audio_sink.client_type;
    bt_ull_le_hid_srv_device_t device_type = (BT_ULL_HEADSET_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_HEADSET : ((BT_ULL_EARBUDS_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_EARBUDS : BT_ULL_LE_HID_SRV_DEVICE_NONE);
    //ull_assert(device_type);
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();

    ull_report(BT_ULL_HID_LOG"bt_ull_le_srv_handle_set_streaming_mute, type:0x%x, port:0x%x, is_mute: 0x%x,", 3,
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
        //TODO:
        uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_streaming_t);
        uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);

        if (NULL != data) {
            data[0] = is_mute ? BT_ULL_EVENT_VOLUME_MUTE : BT_ULL_EVENT_VOLUME_UNMUTE;
            bt_ull_le_srv_memcpy((bt_ull_streaming_t *)(data + sizeof(bt_ull_req_event_t)), streaming, sizeof(bt_ull_streaming_t));
            status = bt_ull_le_hid_srv_send_data_by_device_type(device_type, (uint8_t*)data, len);
            bt_ull_le_srv_memory_free(data);
        }

    } else if (BT_ULL_ROLE_SERVER == ctx->role) {
        if (BT_ULL_MIC_CLIENT == ctx->audio_sink.client_type) {
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
            //TODO:
            uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_streaming_t);
            uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);

            if (NULL != data) {
                data[0] = is_mute ? BT_ULL_EVENT_VOLUME_MUTE : BT_ULL_EVENT_VOLUME_UNMUTE;
                bt_ull_le_srv_memcpy((bt_ull_streaming_t *)(data + sizeof(bt_ull_req_event_t)), streaming, sizeof(bt_ull_streaming_t));
                status = bt_ull_le_hid_srv_send_data_by_device_type(device_type, (uint8_t*)data, len);
                bt_ull_le_srv_memory_free(data);
            }
#endif
        }
    } else {
        //ull_assert(0 && BT_ULL_HID_LOG" unknown role");
    }
    return status;
}

static bt_status_t bt_ull_le_hid_srv_sync_streaming_status(bt_ull_le_stream_port_mask_t stream_port, uint8_t event)
{
    bt_status_t result = BT_STATUS_FAIL;

    bt_ull_streaming_t stream;
    //bt_ull_le_hid_srv_audio_contex_t *audio_ctx = bt_ull_le_hid_srv_get_audio_context();
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_client_t client_type = ctx->audio_sink.client_type;
    bt_ull_le_hid_srv_device_t device_type = (BT_ULL_HEADSET_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_HEADSET : ((BT_ULL_EARBUDS_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_EARBUDS : BT_ULL_LE_HID_SRV_DEVICE_NONE);
    //ull_assert(device_type);
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_port(stream_port);
    if ((BT_ULL_ROLE_SERVER == ctx->role) && (BT_STATUS_SUCCESS == bt_ull_le_srv_get_stream_by_transmitter(trans_type, &stream))) {
        if (BT_ULL_EVENT_STREAMING_STOP_IND == event) {
        #ifndef AIR_SILENCE_DETECTION_ENABLE
            if (!bt_ull_le_srv_is_streaming(bt_ull_le_srv_get_stream_mode(trans_type)))
        #endif
            {
                /* notify client DL stop after both 2-rx stop */
                uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_streaming_t);
                uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                if (NULL != data) {
                    data[0] = event;
                    bt_ull_streaming_t *streaming_port =(bt_ull_streaming_t *)(data + sizeof(bt_ull_req_event_t));
                    streaming_port->streaming_interface = stream.streaming_interface;
                    streaming_port->port = stream.port;
                    result = bt_ull_le_hid_srv_send_data_by_device_type(device_type, (uint8_t*)data, len);
                    bt_ull_le_srv_memory_free(data);
                    ull_report(BT_ULL_HID_LOG" bt_ull_le_srv_sync_streaming_status result is 0x%x, trans_type:0x%x", 2, 
                        result, trans_type);
                }
            }
        } else if (BT_ULL_EVENT_STREAMING_START_IND == event) {
            if (((!ctx->audio_sink.is_streaming_locked)) && (BT_STATUS_SUCCESS == bt_ull_le_hid_srv_open_audio_stream(stream_port))) {
                /*notify all connected device.*/
                uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_streaming_t);
                uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
                if (NULL != data) {
                    data[0] = event;
                    bt_ull_streaming_t *streaming_port =(bt_ull_streaming_t *)(data + sizeof(bt_ull_req_event_t));
                    streaming_port->streaming_interface = stream.streaming_interface;
                    streaming_port->port = stream.port;
                    result = bt_ull_le_hid_srv_send_data_by_device_type(device_type, (uint8_t*)data, len);
                    bt_ull_le_srv_memory_free(data);
                    ull_report(BT_ULL_HID_LOG" bt_ull_le_srv_sync_streaming_status result is 0x%x, trans_type:0x%x", 2, 
                        result, trans_type);
                }

            }
        }
    }
    return result;
}



static bt_status_t bt_ull_le_hid_srv_handle_set_streaming_sample_rate(bt_ull_sample_rate_t *sample_rate)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_server_stream_t *stream_info;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_client_t client_type = ctx->audio_sink.client_type;
    bt_ull_le_hid_srv_device_t device_type = (BT_ULL_HEADSET_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_HEADSET : ((BT_ULL_EARBUDS_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_EARBUDS : BT_ULL_LE_HID_SRV_DEVICE_NONE);

    if (BT_ULL_ROLE_SERVER != ctx->role) {
        ull_report_error(BT_ULL_HID_LOG" error role: 0x%x", 1, ctx->role);
        return BT_STATUS_FAIL;
    }
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(&(sample_rate->streaming));
    stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
    bt_ull_le_stream_port_mask_t start_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);

    ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_handle_set_streaming_sample_rate, if_type:0x%x, port:0x%x, rate: %d, usb sample rate: %d, need restart: %d, is_transmitter_start: %d, streaming_port: %x", 7,
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
    if (stream_info->is_need_restart && (!ctx->audio_sink.is_streaming_locked) \
        && (bt_ull_le_hid_srv_is_device_connected(BT_ULL_LE_HID_SRV_DEVICE_HEADSET) || bt_ull_le_hid_srv_is_device_connected(BT_ULL_LE_HID_SRV_DEVICE_EARBUDS))) {
        bt_ull_le_at_restart(trans_type);
        stream_info->is_need_restart = false;
    }

    if (bt_ull_le_hid_srv_is_device_connected(device_type) && (start_port & stream_ctx->streaming_port)) {
        status = bt_ull_le_hid_srv_sync_streaming_status(start_port, BT_ULL_EVENT_STREAMING_START_IND);
    }
    return status;
}



static bt_status_t bt_ull_le_hid_srv_handle_set_streaming_sample_size(bt_ull_streaming_sample_size_t *sample_size)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_server_stream_t *stream_info;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();

    if (BT_ULL_ROLE_SERVER != ctx->role) {
        ull_report_error(BT_ULL_HID_LOG" SAMPLE SIZE, error role: 0x%x", 1, ctx->role);
        return BT_STATUS_FAIL;
    }
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(&(sample_size->streaming));
    stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
    ull_report(BT_ULL_HID_LOG" SAMPLE SIZE,, if_type:0x%x, port:0x%x, sample size: %d, usb sample size: %d", 4,
        sample_size->streaming.streaming_interface, sample_size->streaming.port, sample_size->sample_size, stream_info->usb_sample_size);
    if (stream_info) {
        if (sample_size->sample_size != stream_info->usb_sample_size) {
            stream_info->usb_sample_size = sample_size->sample_size;
            stream_info->is_need_restart = true;
        }
    } else {
        status = BT_STATUS_FAIL;
        ull_report_error(BT_ULL_HID_LOG" bt_ull_le_srv_handle_set_streaming_sample_size, stream_info is null", 0);
    }
    return status;
}

static bt_status_t bt_ull_le_hid_srv_handle_set_streaming_sample_channel(bt_ull_streaming_sample_channel_t *sample_channel)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_server_stream_t *stream_info;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();

    if (BT_ULL_ROLE_SERVER != ctx->role) {
        ull_report_error(BT_ULL_HID_LOG" SAMPLE CHANNEL, error role: 0x%x", 1, ctx->role);
        return BT_STATUS_FAIL;
    }
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(&(sample_channel->streaming));
    stream_info = (bt_ull_le_srv_server_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_SERVER, trans_type);
    ull_report(BT_ULL_HID_LOG" SAMPLE CHANNEL, if_type:0x%x, port:0x%x, sample channel: %d, usb sample channel: %d", 4,
        sample_channel->streaming.streaming_interface, sample_channel->streaming.port, sample_channel->sample_channel, stream_info->usb_sample_channel);
    if (stream_info) {
        if (sample_channel->sample_channel != stream_info->usb_sample_channel) {
            stream_info->usb_sample_channel = sample_channel->sample_channel;
            stream_info->is_need_restart = true;
        }
    } else {
        status = BT_STATUS_FAIL;
        ull_report_error(BT_ULL_HID_LOG" SAMPLE CHANNEL, stream_info is null", 0);
    }
    return status;
}



static bt_status_t bt_ull_le_hid_srv_handle_set_streaming_mix_ratio_hdl(bt_ull_mix_ratio_t *ratio)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_hid_srv_context_t* ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_client_t client_type = ctx->audio_sink.client_type;
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();

    if (ratio) {
        uint8_t idx = 0;
        for (idx = 0; idx < BT_ULL_MAX_STREAMING_NUM; idx++) {
            ull_report(BT_ULL_HID_LOG" bt_ull_handle_set_streaming_mix_ratio, streaming[%d] type:0x%x, port:0x%x, ratio: %d", 4,
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
            //TODO:
            uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_mix_ratio_t);
            uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
            if (NULL != data) {
                data[0] = BT_ULL_EVENT_MIX_RATIO_ACTION;
                bt_ull_mix_ratio_t *ratio_req = (bt_ull_mix_ratio_t *)(data + sizeof(bt_ull_req_event_t));
                bt_ull_le_srv_memcpy(ratio_req, ratio, sizeof(bt_ull_mix_ratio_t));
                if (BT_ULL_HEADSET_CLIENT == client_type) {
                    status = bt_ull_le_hid_srv_send_data_by_device_type(BT_ULL_LE_HID_SRV_DEVICE_HEADSET, (uint8_t*) data, len);
                } else if (BT_ULL_EARBUDS_CLIENT == client_type) {
                    status = bt_ull_le_hid_srv_send_data_by_device_type(BT_ULL_LE_HID_SRV_DEVICE_EARBUDS, (uint8_t*) data, len);
                } else {
                    ull_assert(0 && "invalid client type");
                }
                bt_ull_le_srv_memory_free(data);
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

static bt_status_t bt_ull_le_hid_srv_handle_usb_audio_control(bt_ull_usb_hid_control_t action)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_client_t client_type = ctx->audio_sink.client_type;
    bt_ull_le_hid_srv_device_t device_type = (BT_ULL_HEADSET_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_HEADSET : ((BT_ULL_EARBUDS_CLIENT == client_type) ? BT_ULL_LE_HID_SRV_DEVICE_EARBUDS : BT_ULL_LE_HID_SRV_DEVICE_NONE);

    ull_report(BT_ULL_HID_LOG" usb_hid_control: 0x%x, role:0x%x", 2, action, ctx->role);

#ifdef AIR_USB_ENABLE
    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
        ull_report(BT_ULL_HID_LOG" usb_hid_control, current is XBOX mode, ignore the HID control event", 0);
        return BT_STATUS_FAIL;
    }
#endif /* MTK_USB_DEMO_ENABLED */

    if (bt_ull_le_hid_srv_is_device_connected(device_type)) {
        if (BT_ULL_ROLE_CLIENT == ctx->role) {
            //TODO:
            uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_usb_hid_control_t);
            uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
            if (NULL != data) {
                data[0] = BT_ULL_EVENT_USB_HID_CONTROL_ACTION;
                data[1] = action;
                status = bt_ull_le_hid_srv_send_data_by_device_type(device_type, (uint8_t*) data, len);
                bt_ull_le_srv_memory_free(data);
            }
        } else if (BT_ULL_ROLE_SERVER == ctx->role) {
#if defined(AIR_USB_ENABLE) && defined(AIR_USB_AUDIO_ENABLE) && defined(AIR_USB_HID_ENABLE)
            if (BT_ULL_USB_HID_PLAY_PAUSE_TOGGLE == action
                || BT_ULL_USB_HID_PAUSE == action
                || BT_ULL_USB_HID_PLAY == action) {
                if (USB_HID_STATUS_OK == USB_HID_PlayPause()) {
                    ull_report(BT_ULL_HID_LOG" usb_hid_control, PlayPause Success", 0);
                }
            } else if (BT_ULL_USB_HID_PREVIOUS_TRACK == action) {
                if (USB_HID_STATUS_OK == USB_HID_ScanPreviousTrack()) {
                    ull_report(BT_ULL_HID_LOG" usb_hid_control, Previous Track Success", 0);
                }
            } else if (BT_ULL_USB_HID_NEXT_TRACK == action) {
                if (USB_HID_STATUS_OK == USB_HID_ScanNextTrack()) {
                    ull_report(BT_ULL_HID_LOG" usb_hid_control, Next Track Success", 0);
                }
            }
#endif
        }
    } else {
        ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_handle_usb_hid_control fail! have not any connected device", 0);
    }
    return status;
}

static bt_status_t bt_ull_le_hid_srv_led_control_hdl(bt_ull_le_hid_srv_control_indicater_led_t *led_control)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    uint16_t len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_hid_srv_control_indicater_led_t);
    uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(len);
    if (NULL != data) {
        data[0] = BT_ULL_LE_HID_MSG_OUT_PUT_DATA;
        bt_ull_le_hid_srv_control_indicater_led_t *led_control_req = (bt_ull_le_hid_srv_control_indicater_led_t *)(data + sizeof(bt_ull_req_event_t));
        bt_ull_le_srv_memcpy(led_control_req, led_control, sizeof(bt_ull_le_hid_srv_control_indicater_led_t));
        status = bt_ull_le_hid_srv_send_data_by_device_type(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD, (uint8_t *)data, len);
        bt_ull_le_srv_memory_free(data);
    }
    ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_led_control_hdl, status: 0x%x,led_control:%d", 2, status, led_control->indicater_led);
    return status;
}

static void bt_ull_le_hid_srv_handle_audio_transmitter_event(bt_ull_le_at_event_t event, void *param, uint32_t param_len)
{
    switch (event) {
        case BT_ULL_LE_AT_EVENT_START_IND: {
            bt_ull_le_at_result_t *start_ind = (bt_ull_le_at_result_t *)param;
            bt_ull_le_hid_srv_start_audio_transmitter_callback(start_ind->transmitter_type, start_ind->result);
        } break;

        case BT_ULL_LE_AT_EVENT_STOP_IND: {
            bt_ull_le_at_result_t *stop_ind = (bt_ull_le_at_result_t *)param;
            bt_ull_le_hid_srv_stop_audio_transmitter_callback(stop_ind->transmitter_type, stop_ind->result);
        } break;

        default:
            break;
    }
}

static void bt_ull_le_hid_srv_handle_audio_manager_event(bt_ull_le_am_event_t event, void *data, uint32_t data_len)
{
    switch (event) {
        case BT_ULL_LE_AM_PLAY_IND: {
            bt_ull_le_am_result_t *play_ind = (bt_ull_le_am_result_t *)data;
            bt_ull_le_hid_srv_play_am_callback(play_ind->mode, play_ind->result);
        } break;

        case BT_ULL_LE_AM_STOP_IND: {
            bt_ull_le_am_result_t *stop_ind = (bt_ull_le_am_result_t *)data;
            bt_ull_le_hid_srv_stop_am_callback(stop_ind->mode, stop_ind->result);
        } break;

        default:
            break;
    }
}


static void bt_ull_le_hid_srv_start_audio_transmitter_callback(bt_ull_transmitter_t transmitter_type, bt_status_t status)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    //bt_ull_client_t client_type = ctx->audio_sink.client_type;
    ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_start_audio_transmitter_callback, trans_type is %d, status is 0x%x", 2, transmitter_type, status);
    if ((BT_STATUS_SUCCESS == status) && (BT_ULL_ROLE_SERVER == ctx->role)) {
        bt_ull_le_srv_set_transmitter_is_start(transmitter_type, true);
        //Step1: set iso data path
        uint8_t i;
        for (i = 0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i++) {
            if ((ctx->link[i].device_type == BT_ULL_LE_HID_SRV_DEVICE_HEADSET || ctx->link[i].device_type == BT_ULL_LE_HID_SRV_DEVICE_EARBUDS) 
                && (BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED <= ctx->link[i].state) && (BT_HANDLE_INVALID != ctx->link[i].acl_handle)) {
                bt_ull_le_hid_conn_srv_active_streaming(ctx->link[i].acl_handle);
            }
        }

        // Need run state machine?
        //Step2: Notify APP streaming status
        bt_ull_le_streaming_start_ind_t start_stream;
        if (BT_STATUS_SUCCESS == bt_ull_le_srv_get_stream_by_transmitter(transmitter_type, &start_stream.stream) && ctx->audio_sink.curr_stream_state == BT_ULL_LE_SRV_STREAM_STATE_STREAMING) {
            /* notify app streaming stoped */
            bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_STREAMING_START_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t));
        }
        ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_start_audio_transmitter_callback curr_stream_state: %d", 1, ctx->audio_sink.curr_stream_state);
    } else {
        bt_ull_le_srv_set_transmitter_is_start(transmitter_type, false);
    }
}




static void bt_ull_le_hid_srv_stop_audio_transmitter_callback(bt_ull_transmitter_t transmitter_type, bt_status_t status)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_stop_audio_transmitter_callback, trans_type is %d, status is 0x%x", 2, transmitter_type, status);
    if ((BT_STATUS_SUCCESS == status) && (BT_ULL_ROLE_SERVER == ctx->role)) {
        bt_ull_le_srv_set_transmitter_is_start(transmitter_type, false);

        //Step1: remove iso data path
        if (!bt_ull_le_srv_is_any_streaming_started(ctx->role) && ctx->audio_sink.curr_stream_state >= BT_ULL_LE_HID_SRV_STREAM_STATE_START_AUDIO_STREAM) {
            uint8_t i;
            for (i = 0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i++) {
                if ((ctx->link[i].device_type == BT_ULL_LE_HID_SRV_DEVICE_HEADSET || ctx->link[i].device_type == BT_ULL_LE_HID_SRV_DEVICE_EARBUDS) 
                    && (BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED <= ctx->link[i].state) && (BT_HANDLE_INVALID != ctx->link[i].acl_handle)) {
                    bt_ull_le_hid_conn_srv_deactive_streaming(ctx->link[i].acl_handle);
                }
            }
            ctx->audio_sink.curr_stream_state = BT_ULL_LE_HID_SRV_STREAM_STATE_IDLE;
        }

        //Step2: Notify APP streaming status
        bt_ull_le_streaming_stop_ind_t stop_stream;
        if (BT_STATUS_SUCCESS == bt_ull_le_srv_get_stream_by_transmitter(transmitter_type, &stop_stream.stream)) {
            /* notify app streaming stoped */
            bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_STREAMING_STOP_IND, (void *)&stop_stream, sizeof(bt_ull_le_streaming_stop_ind_t));
        }
    }
}



static void bt_ull_le_hid_srv_play_am_callback(bt_ull_le_am_mode_t mode, bt_status_t status)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_play_am_callback, mode is %d, status is 0x%x", 2, mode, status);

    if ((BT_STATUS_SUCCESS == status) && (BT_ULL_ROLE_CLIENT == ctx->role)) {
        uint8_t i;
        bt_ull_le_streaming_start_ind_t start_stream;
        bt_ull_le_srv_client_stream_t *stream_info = NULL;
        if (BT_ULL_LE_AM_DL_MODE == mode) {
            start_stream.stream_mode = BT_ULL_LE_STREAM_MODE_DOWNLINK;
            stream_info = (bt_ull_le_srv_client_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_CLIENT, BT_ULL_GAMING_TRANSMITTER);
            //Step1: set iso data path
                     
            for (i = 0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i++) {
                ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_play_am_callback(active), %x, %x, %x", 3, \
                    ctx->link[i].device_type,
                    ctx->link[i].state,
                    ctx->link[i].acl_handle);
                if ((ctx->link[i].device_type == BT_ULL_LE_HID_SRV_DEVICE_HEADSET || ctx->link[i].device_type == BT_ULL_LE_HID_SRV_DEVICE_EARBUDS)
                    && (BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED <= ctx->link[i].state) && (BT_HANDLE_INVALID != ctx->link[i].acl_handle)) {
                    bt_ull_le_hid_conn_srv_active_streaming(ctx->link[i].acl_handle);
                }
            }

        #ifndef AIR_WIRELESS_MIC_ENABLE
            /**< 1. raise DL pseudo device priority; 2) Take Mic resource for ULL UL;*/
            if ((!(stream_ctx->client.ul.is_am_open)) && (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK))) {
                bt_ull_le_am_update_dl_priority(true);//need to raise proiryty for Uplink
                if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                    stream_ctx->client.dl.is_dl_dummy_mode = true;
                }
                
                if (BT_STATUS_SUCCESS == bt_ull_le_am_play(BT_ULL_LE_AM_UL_MODE, stream_ctx->codec_type, true)) {
                    ull_report(BT_ULL_HID_LOG" Downlink Enable, then start Uplink", 0);
                }
            }
            ull_report(BT_ULL_HID_LOG"Downlink Enable, DL is Dummy mode: %d", 1, stream_ctx->client.dl.is_dl_dummy_mode);
        #endif
        } else if (BT_ULL_LE_AM_UL_MODE == mode) {
            start_stream.stream_mode = BT_ULL_LE_STREAM_MODE_UPLINK;
            stream_info = (bt_ull_le_srv_client_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_CLIENT, BT_ULL_MIC_TRANSMITTER);
        }
        if (NULL == stream_info) {
            ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_play_am_callback can not get streaming info", 0);
            return;
        }
        stream_info->is_am_open = true;
        for (i = 0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i++) {
            ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_play_am_callback(unmute), %x, %x, %x", 3, \
                ctx->link[i].device_type,
                ctx->link[i].state,
                ctx->link[i].acl_handle);
            if ((ctx->link[i].device_type == BT_ULL_LE_HID_SRV_DEVICE_HEADSET || ctx->link[i].device_type == BT_ULL_LE_HID_SRV_DEVICE_EARBUDS)
                && (BT_ULL_LE_HID_SRV_LINK_STATE_STREAMING <= ctx->link[i].state) && (BT_HANDLE_INVALID != ctx->link[i].acl_handle)) {

                 /** always need send unmute vendor cmd to controller, controller will send play_en to DSP. */
                if (BT_STATUS_SUCCESS != bt_ull_le_hid_conn_srv_unmute_air_cis(ctx->link[i].acl_handle)) {
                    ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_play_am_callback unmute air cis fail", 0);
                }
            }
        }

        /* notify app streaming started */
        if ((BT_ULL_LE_AM_UL_MODE == mode) || ((BT_ULL_LE_AM_DL_MODE == mode) && (!(stream_ctx->client.dl.is_dl_dummy_mode)))) {
            bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_STREAMING_START_IND, (void *)&start_stream, sizeof(bt_ull_le_streaming_start_ind_t));
        }
    }
    ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_play_am_callback, mode is %d, status is 0x%x, curr_stream_state is 0x%x", 3, mode, status, ctx->audio_sink.curr_stream_state);
}




static void bt_ull_le_hid_srv_stop_am_callback(bt_ull_le_am_mode_t mode, bt_status_t status)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_stop_am_callback, mode is %d, status is 0x%x", 2, mode, status);
    uint8_t i = 0;
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
            if (stream_ctx->client.ul.is_am_open && !(ctx->audio_sink.restart_streaming_mask & (1 << BT_ULL_LE_STREAM_MODE_DOWNLINK))) {
                bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, true);
                stream_ctx->client.dl.is_dl_dummy_mode = false;
                ull_report(BT_ULL_HID_LOG" Downlink Disabled but Uplink still exist, need to disable Uplink", 0);
            }
    #endif
            if (ctx->audio_sink.restart_streaming_mask & (1 << BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                ctx->audio_sink.restart_streaming_mask &= (~(1 << BT_ULL_LE_STREAM_MODE_DOWNLINK));
                ull_report(BT_ULL_HID_LOG" Need restart DL streming, %d", 1, bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK));
                if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                    bt_ull_streaming_t stream;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                    stream.port = 0;
                    bt_ull_le_hid_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_DOWNLINK);
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
                if (BT_STATUS_SUCCESS == bt_ull_le_am_stop(BT_ULL_LE_AM_DL_MODE, true)) {
                    stream_ctx->client.dl.is_dl_dummy_mode = false;
                    ull_report(BT_ULL_HID_LOG" Uplink Disable, then stop Downlink Dummy mode", 0);
                }
                if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
                    stream_ctx->client.dl.is_dl_dummy_mode = true;
                }
            } else {/*dummy mode, need to lower proiryty because of Uplink had been disabled*/
                stream_ctx->client.dl.is_dl_dummy_mode = false;
                bt_ull_le_am_update_dl_priority(false);
            }
            ull_report(BT_ULL_HID_LOG" Uplink Disable, DL is Dummy mode: %d", 1, stream_ctx->client.dl.is_dl_dummy_mode);
    #endif
            if (ctx->audio_sink.restart_streaming_mask & (1 << BT_ULL_LE_STREAM_MODE_UPLINK)) {
                ctx->audio_sink.restart_streaming_mask &= (~(1 << BT_ULL_LE_STREAM_MODE_UPLINK));
                ull_report(BT_ULL_HID_LOG" Need restart UL streming, %d", 1, bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK));
                if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
                    bt_ull_streaming_t stream;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    stream.port = 0;
                    bt_ull_le_hid_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
                }
            }
        }
        if (NULL == stream_info) {
            ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_stop_am_callback can not get streaming info", 0);
            return;
        }
        /* notify app streaming stoped */
        if ((BT_ULL_LE_AM_UL_MODE == mode) || ((BT_ULL_LE_AM_DL_MODE == mode) && (!stream_ctx->client.dl.is_dl_dummy_mode))) {
            bt_ull_le_hid_srv_notify_event(BT_ULL_LE_HID_SRV_MODULE_APP, BT_ULL_EVENT_LE_STREAMING_STOP_IND, (void *)&stop_stream, sizeof(bt_ull_le_streaming_stop_ind_t));
        }

        if (!bt_ull_le_srv_is_any_streaming_started(ctx->role)) {/*all streaming are disabled.*/
            /* disable controller and enable sniff mode after all streaming stop */
            //ctx->curr_stream_state = BT_ULL_LE_SRV_STREAM_STATE_IDLE;
            for (i = 0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i++) {
                if ((ctx->link[i].device_type == BT_ULL_LE_HID_SRV_DEVICE_HEADSET || ctx->link[i].device_type == BT_ULL_LE_HID_SRV_DEVICE_EARBUDS) 
                    && (BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED <= ctx->link[i].state) && (BT_HANDLE_INVALID != ctx->link[i].acl_handle)) {
                    bt_ull_le_hid_conn_srv_deactive_streaming(ctx->link[i].acl_handle);
                }
            }
            ctx->audio_sink.curr_stream_state = BT_ULL_LE_HID_SRV_STREAM_STATE_IDLE;
            /* TBD. enable link sniff policy */
        }
        ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_stop_am_callback, curr_stream_state %d", 1, ctx->audio_sink.curr_stream_state);
    }
}



static bt_status_t bt_ull_le_hid_srv_open_audio_stream(bt_ull_le_stream_port_mask_t stream_port)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    if ((!((bt_ull_le_hid_srv_is_device_connected(BT_ULL_LE_HID_SRV_DEVICE_HEADSET) || bt_ull_le_hid_srv_is_device_connected(BT_ULL_LE_HID_SRV_DEVICE_EARBUDS))) \
        || (0 == stream_ctx->streaming_port))) {
        ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_open_audio_stream fail, streaming_port mask is %d", 1, stream_ctx->streaming_port);
        return BT_STATUS_FAIL;
    }
    /* the audio tramsmitter has been open. */
    bt_ull_transmitter_t trans = bt_ull_le_srv_get_transmitter_by_stream_port(stream_port);

    /* init controller/DSP media data share buffer info */
    if (!ctx->audio_sink.is_share_buffer_set) {
        status = bt_ull_le_srv_init_share_info(ctx->audio_sink.client_type);
        ull_report(BT_ULL_HID_LOG" bt_ull_le_hid_srv_open_audio_stream, init share buffer: 0x%x, status: 0x%x", 2, ctx->audio_sink.is_share_buffer_set, status);

        if ((BT_STATUS_SUCCESS == status) && (BT_STATUS_SUCCESS == bt_ull_le_srv_set_avm_share_buffer(ctx->role, ctx->audio_sink.client_type, (ctx->audio_sink.client_type == BT_ULL_HEADSET_CLIENT) ? 0x01 : 0x02))) {
            ctx->audio_sink.is_share_buffer_set = true;
        } else {
            ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_open_audio_stream fail: set AVM buffer error", 0);
            return BT_STATUS_FAIL;
        }
    }

    /* check gaming port state */
    bt_ull_le_srv_audio_out_t audio_out = bt_ull_le_srv_get_audio_out_by_stream_port(stream_port);

    /*set codec info to controller (for set CIG) & to Audio transmitter. */
    status = bt_ull_le_at_init(ctx->audio_sink.client_type, audio_out, trans, stream_ctx->codec_type);
    if ((BT_STATUS_SUCCESS == status) && ((BT_ULL_PLAY_ALLOW == stream_ctx->allow_play))) {
        if (BT_STATUS_SUCCESS == (status = bt_ull_le_at_start(trans, true))) {
            bt_ull_le_srv_set_transmitter_is_start(trans, true);
            if (BT_ULL_LE_SRV_STREAM_STATE_IDLE == ctx->audio_sink.curr_stream_state) {
                    ctx->audio_sink.curr_stream_state = BT_ULL_LE_SRV_STREAM_STATE_START_AUDIO_STREAM;
            }
        }
    }
    ull_report(BT_ULL_HID_LOG" start tramitter is %d, status is 0x%x, curr_stream_state is %d, allow paly: %d", 4, \
        trans, status, ctx->audio_sink.curr_stream_state, stream_ctx->allow_play);
    return status;
}



static bt_status_t bt_ull_le_hid_srv_play_am(bt_ull_streaming_t *streaming, bt_ull_le_stream_mode_t stream_mode)
{
    bt_status_t status = BT_STATUS_SUCCESS;

    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_transmitter_t trans_type = bt_ull_le_srv_get_transmitter_by_stream_interface(streaming);
    bt_ull_le_stream_port_mask_t stream_port = bt_ull_le_srv_get_stream_port_by_transmitter(trans_type);
    bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
    bt_ull_le_srv_client_stream_t *stream_info = (bt_ull_le_srv_client_stream_t *)bt_ull_le_srv_get_stream_info(BT_ULL_ROLE_CLIENT, trans_type);

    if (ctx->audio_sink.is_streaming_locked) {
        stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_MIC; 
        ull_report_error(BT_ULL_HID_LOG" bt_ull_le_hid_srv_play_am, Fail, streaming is locked!", 0);
        return BT_STATUS_FAIL;
    }

    if (BT_ULL_LE_STREAM_MODE_DOWNLINK == stream_mode) {
        
        stream_info->is_dl_dummy_mode = false;
        ull_report(BT_ULL_HID_LOG"Play DL, is_am_open: %d", 1, stream_ctx->client.dl.is_am_open);
        if (!(stream_info->is_am_open)) {
            status = bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true);
            if ((BT_STATUS_SUCCESS == status) && (BT_ULL_LE_HID_SRV_STREAM_STATE_IDLE == ctx->audio_sink.curr_stream_state)) {
                ctx->audio_sink.curr_stream_state = BT_ULL_LE_HID_SRV_STREAM_STATE_START_AUDIO_STREAM;
            }
        } else {
            uint8_t i;
            for (i = 0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i++) {
                if ((BT_ULL_LE_HID_SRV_DEVICE_HEADSET == ctx->link[i].device_type || BT_ULL_LE_HID_SRV_DEVICE_EARBUDS== ctx->link[i].device_type) && BT_ULL_LE_HID_SRV_LINK_STATE_STREAMING == ctx->link[i].state) {
                    if (BT_ULL_LE_HID_SRV_STREAM_STATE_STREAMING != ctx->audio_sink.curr_stream_state)
                        ctx->audio_sink.curr_stream_state = BT_ULL_LE_HID_SRV_STREAM_STATE_STREAMING;
                }
            }
        }
    } else if (BT_ULL_LE_STREAM_MODE_UPLINK == stream_mode) {
        if ((bt_ull_le_hid_srv_is_device_connected(BT_ULL_LE_HID_SRV_DEVICE_HEADSET) || bt_ull_le_hid_srv_is_device_connected(BT_ULL_LE_HID_SRV_DEVICE_EARBUDS))) {
            //bt_ull_le_hid_srv_link_info_t *link_info = &ctx->link[0];
            /* Normal case, enable mic if ull is playing */
            ull_report(BT_ULL_HID_LOG"Play UL, is_am_open: %d", 1, stream_ctx->client.ul.is_am_open);
            if (((BT_ULL_EARBUDS_CLIENT == ctx->audio_sink.client_type) || BT_ULL_EARBUDS_CLIENT != ctx->audio_sink.client_type) && \
                !(stream_ctx->client.ul.is_am_open)) {
                /* Ensure DL has beed enabled before UL enable.*/
                /* 1. only Uplink come, enter DL dummy mode*/
#ifndef AIR_WIRELESS_MIC_ENABLE
                bt_ull_le_am_update_dl_priority(true);//need to raise proiryty for Uplink
                if (!(stream_ctx->client.dl.is_am_open)) {
                    stream_ctx->client.dl.is_dl_dummy_mode = bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK) ? false : true;
                    if (BT_STATUS_SUCCESS == bt_ull_le_am_play(BT_ULL_LE_AM_DL_MODE, stream_ctx->codec_type, true)) {
                        ull_report(BT_ULL_HID_LOG" Raise Prority and Enable Downlink before Start Uplink", 0);
                    }
                } else
#endif
                {/* 2. DL has beed enabled before UL enable*/
                    bt_ull_le_am_play(BT_ULL_LE_AM_UL_MODE, stream_ctx->codec_type, true);
                    if (BT_ULL_LE_HID_SRV_STREAM_STATE_IDLE == ctx->audio_sink.curr_stream_state) {
                        ctx->audio_sink.curr_stream_state = BT_ULL_LE_HID_SRV_STREAM_STATE_STREAMING;
                    }
                }
            } else {
                bt_ull_le_hid_srv_link_info_t *link_info = NULL;
                uint8_t i;
                for (i = 0; i < BT_ULL_LE_HID_SRV_MAX_LINK_NUM; i++) {
                    if ((BT_ULL_LE_HID_SRV_DEVICE_HEADSET == ctx->link[i].device_type || BT_ULL_LE_HID_SRV_DEVICE_EARBUDS== ctx->link[i].device_type)
                        && (BT_ULL_LE_HID_SRV_LINK_STATE_ULL_HID_SRV_CONNECTED <= ctx->link[i].state)) {
                        link_info = &ctx->link[i];
                        break;
                    }
                }
               

                if (link_info && (BT_ULL_LE_HID_SRV_LINK_STATE_STREAMING == link_info->state)) {
                    if (BT_ULL_LE_HID_SRV_STREAM_STATE_STREAMING != ctx->audio_sink.curr_stream_state)
                        ctx->audio_sink.curr_stream_state = BT_ULL_LE_HID_SRV_STREAM_STATE_STREAMING;
                }
            }
        }
    }
    ull_report(BT_ULL_HID_LOG" Client recieved enable stream_port:%d, status:0x%x, curr_stream_state:0x%x", 3, stream_port, status, ctx->audio_sink.curr_stream_state);
    return status;
}




static bt_status_t bt_ull_le_hid_srv_stop_am(bt_ull_streaming_t *streaming)
{
    bt_status_t status = BT_STATUS_SUCCESS;

    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
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
    ull_report(BT_ULL_HID_LOG" Client recieved disable stream_port:%d, status:0x%x, curr_stream_state:0x%x", 3, \
        stream_port, status, ctx->audio_sink.curr_stream_state);
    return status;
}

void bt_ull_le_hid_srv_lock_streaming(bool lock)
{
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE] bt_ull_le_srv_lock_streaming, old lock: %d, new lock: %d, role:0x%0x", 3, 
        ctx->audio_sink.is_streaming_locked, lock, ctx->role);

    if (ctx->audio_sink.is_streaming_locked != lock) {
        ctx->audio_sink.is_streaming_locked = lock;
    }
    if (BT_ULL_ROLE_SERVER == ctx->role) {
        bt_ull_streaming_t stream;

        if (lock) {
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
               stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
               stream.port = 0;
               bt_ull_le_hid_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
               stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_GAMING;
             }
             if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
               stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
               stream.port = 1;
               bt_ull_le_hid_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
               stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_CHAT;
             }
             if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
               stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
               stream.port = 0;
               bt_ull_le_hid_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
               stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_MIC;
             }
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_IN) {
              stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
              stream.port = 0;
              bt_ull_le_hid_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
              stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_LINE_IN;
            }
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT) {
              stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
              stream.port = 0;
              bt_ull_le_hid_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
              stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT;
            }
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT) {
              stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
              stream.port = 0;
              bt_ull_le_hid_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
              stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT;
            }
            if (stream_ctx->streaming_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_IN) {
              stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
              stream.port = 0;
              bt_ull_le_hid_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
              stream_ctx->locking_port |= BT_ULL_LE_STREAM_PORT_MASK_I2S_IN;
            }
             /*Remove CIG.*/
            /*if (ctx->is_cig_created) {
                bt_ull_le_conn_srv_remove_air_cig_params();
            }*/
        } else {
            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_GAMING) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_GAMING;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                stream.port = 0;
                bt_ull_le_hid_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_CHAT) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_CHAT;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                stream.port = 1;
                bt_ull_le_hid_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_MIC) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_MIC;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                stream.port = 0;
                bt_ull_le_hid_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_IN) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_LINE_IN;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_IN;
                stream.port = 0;
                bt_ull_le_hid_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }

            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_LINE_OUT;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
                stream.port = 0;
                bt_ull_le_hid_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }

            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_I2S_OUT;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
                stream.port = 0;
                bt_ull_le_hid_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }

            if (stream_ctx->locking_port & BT_ULL_LE_STREAM_PORT_MASK_I2S_IN) {
                stream_ctx->locking_port &= ~BT_ULL_LE_STREAM_PORT_MASK_I2S_IN;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_IN;
                stream.port = 0;
                bt_ull_le_hid_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
            }
        }
     } else if (BT_ULL_ROLE_CLIENT == ctx->role) {
         if (lock) {
            /* stop current play & disallow dongle play */
            //bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_DISALLOW);
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
                bt_ull_le_hid_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_UPLINK);
            } else if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                bt_ull_streaming_t stream;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
                stream.port = 0;
                bt_ull_le_hid_srv_play_am(&stream, BT_ULL_LE_STREAM_MODE_DOWNLINK);
            }
            //bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_ALLOW);
        }
    }
}

void bt_ull_le_hid_srv_get_fota_info(bt_addr_t *addr, bt_ull_le_hid_srv_fota_info *info)
{
    uint8_t idx = bt_ull_le_hid_srv_get_link_idx_by_addr(addr);
    if (BT_ULL_LE_HID_SRV_INVALID_VALUE == idx || !info) {
        return;
    }
    bt_ull_le_hid_srv_context_t *ctx = bt_ull_le_hid_srv_get_ctx();
    info->mtu = BT_ULL_LE_HID_SRV_FIX_CHANNEL_MTU - sizeof(bt_ull_le_hid_srv_msg_t) - 2;
    info->att_handle_rx = ctx->link[idx].att_handle_rx;
    info->att_handle_tx = ctx->link[idx].att_handle_tx;
    info->att_handle_cccd = ctx->link[idx].att_handle_cccd;
    info->device_type = bt_ull_le_hid_srv_get_device_type_by_idx(idx);
}

uint8_t bt_ull_le_hid_srv_get_bonded_device_num(bt_ull_le_hid_srv_device_t device_type)
{
    return bt_ull_le_hid_dm_get_bonded_device_num(device_type);
}
void bt_ull_le_hid_srv_get_bonded_device_list(bt_ull_le_hid_srv_device_t device_type, uint8_t count, bt_addr_t *list)
{
    return bt_ull_le_hid_dm_get_bonded_device_list(device_type, count, list);
}
bool bt_ull_le_hid_srv_is_bonded_device(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr)
{
    return bt_ull_le_hid_dm_is_bonded_device(device_type, addr);
}

bt_status_t bt_ull_le_hid_srv_clear_bonded_list(bt_ull_le_hid_srv_device_t device_type)
{
    return bt_ull_le_hid_dm_clear_bonded_list(device_type);
}

void bt_ull_le_hid_srv_indicate_server_play_is_allow(bt_ull_allow_play_t is_allow, uint8_t reason)
{
    return;
}

bt_status_t bt_ull_le_hid_srv_disable_streaming(void)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_ull_le_hid_srv_enable_streaming(void)
{
    return BT_STATUS_SUCCESS;
}
bool bt_ull_le_hid_srv_is_connected(void)
{
    return bt_ull_le_hid_srv_is_device_connected(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
}

void bt_ull_le_hid_srv_silence_detection_notify_client(bool is_silence, bt_ull_transmitter_t transmitter_type)
{

}


