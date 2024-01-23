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
 
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE

#include "app_dongle_ull_le_hid.h"
#include "app_dongle_connection_common.h"
#include "bt_ull_le_hid_device_manager.h"
#include "apps_debug.h"
#include "bt_type.h"
#include "bt_gap_le.h"
#include "bt_gap_le_service.h"
#include "bt_system.h"
#include "bt_callback_manager.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "bt_device_manager_power.h"

#if (defined MTK_RACE_CMD_ENABLE)
#include "apps_race_cmd_event.h"
#include "race_noti.h"
#include "race_xport.h"
#include "app_le_audio_air.h"
#endif

#include "bt_gattc.h"
#include "bt_avm.h"
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
#include "bt_timer_external.h"
#endif

#include "apps_usb_utils.h"
#ifdef AIR_PURE_GAMING_MS_ENABLE
#include "app_key_remap.h"
#endif

#define USB_SUSPEND_BT_PF_PO_SUPPORT

#define APP_DONGLE_HID_LOG_TAG     "[app_dongle_hid] "
#define APP_DONGLE_ULL_LE_HID_LINK_NUM_MAX                          0x03
#define APP_DONGLE_ULL_LE_HID_GATT_CLIENT_MTU                       512

//fast pair configuration
#define APP_DONGLE_ULL_LE_HID_FAST_PAIRING_TIMEOUT                  (100) //unit ms
#define APP_DONGLE_ULL_LE_HID_FAST_PAIRING_RSSI_THRESHOLD           (-60) //unit dbm
#define APP_DONGLE_ULL_LE_HID_FAST_PAIRING_RX_GAIN_VALUE            (BT_AVM_LOW_GAIN) //please refer to #bt_avm_fixed_rx_gain_t


#define APP_ULL_HID_UNIVERSAL_DONGLE_DISABLE                        0x00
#define APP_ULL_HID_UNIVERSAL_DONGLE_ENABLE                         0x01

#define APPEARANCE_SERVICE_MEDIA_PLAYER_HEADSET                     0x0281
#define APPEARANCE_SEERVICE_MEDIA_PLAYER_EARBUDS                    0x0282
#define APPEARANCE_SERVICE_HID_KEYBOARD                             0x0031  //0x03C1
#define APPEARANCE_SERVICE_HID_MOUSE                                0x0030  //0x03C2


#define APP_DONGLE_ULL_LE_HID_STATE_NONE                            0x00
#define APP_DONGLE_ULL_LE_HID_STATE_WAITING                          0x01
#define APP_DONGLE_ULL_LE_HID_STATE_START_SCAN                      0x02
#define APP_DONGLE_ULL_LE_HID_STATE_STOP_SCAN                       0x03
typedef uint8_t app_dongle_ull_le_hid_state_t;

#define APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTED               0x00
#define APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING                 0x01
#define APP_DONGLE_ULL_LE_HID_CONN_STATE_CANCEL_CONNECTING          0x02
#define APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED                  0x03
#define APP_DONGLE_ULL_LE_HID_CONN_STATE_BONDING                    0x04
#define APP_DONGLE_ULL_LE_HID_CONN_STATE_BOND_COMPLETE              0x05
#define APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING_SERVICE         0x06
#define APP_DONGLE_ULL_LE_HID_CONN_STATE_SERVICE_CONNECTED          0x07
#define APP_DONGLE_ULL_LE_HID_CONN_STATE_SWITCH_TO_CIS              0x08
#define APP_DONGLE_ULL_LE_HID_CONN_STATE_SWITCH_TO_ACL              0x09
#define APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTING              0x0A
typedef uint8_t app_dongle_ull_le_hid_conn_state_t;

#define APP_DONGLE_ULL_LE_HID_SET_WHITE_LIST_STATE_NONE             0x00
#define APP_DONGLE_ULL_LE_HID_SET_WHITE_LIST_STATE_ADD_ON_GOING     0x01
#define APP_DONGLE_ULL_LE_HID_SET_WHITE_LIST_STATE_REMOVE_ON_GOING  0x02
#define APP_DONGLE_ULL_LE_HID_SET_WHITE_LIST_STATE_CLEAR_ON_GOING   0x03
#define APP_DONGLE_ULL_LE_HID_SET_WHITE_LIST_STATE_COMPLETE         0x04
typedef uint8_t app_dongle_ull_le_hid_set_white_list_state_t;

#define APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLED                  (1 << 0x00)
#define APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLING                  (1 << 0x01)
#define APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLED                   (1 << 0x02)
#define APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLING                 (1 << 0x03)
typedef uint8_t app_dongle_ull_le_hid_scan_state_t;

#define APP_DONGLE_ULL_LE_HID_LINK_MODE_NONE                        0x00
#define APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_ACL                    0x01
#define APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_CIS                    0x02
typedef uint8_t app_dongle_ull_le_hid_link_mode_t;

#define APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE                        0x00
#define APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_RECONNECT                   0x01
#define APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FACTORY_TEST                0x02
#define APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FAST_PAIR                   0x03
typedef uint8_t app_dongle_ull_le_hid_connection_mode_t;

#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
#define APP_DONGLE_ULL_LE_HID_FAST_PAIRING_TIMER BT_ULL_LE_HID_FAST_PAIRING_MODE_TIMER_ID
#define APP_DONGLE_ULL_LE_HID_FACTORY_TEST_TIMER BT_ULL_LE_HID_FACTORY_TEST_MODE_TIMER_ID
#define APP_DONGLE_ULL_LE_HID_CREATE_CIS_TIMER    BT_ULL_LE_HID_CREAT_CIS_MODE_TIMER_ID
#else
#define APP_DONGLE_ULL_LE_HID_FAST_PAIRING_TIMER                          0x00
#define APP_DONGLE_ULL_LE_HID_FACTORY_TEST_TIMER                          0x01
#define APP_DONGLE_ULL_LE_HID_CREATE_CIS_TIMER                            0x02
#endif

/** AT CMD for test***/
uint16_t g_app_hid_create_cis_timeout = 10;
uint16_t g_app_hid_establish_wait = 2000;

/** AT CMD for test***/


typedef struct {
    bool                                   is_bond;
    bt_handle_t                            handle;
    bt_addr_t                              peer_addr;
    bt_ull_le_hid_srv_device_t             device_type;
    app_dongle_ull_le_hid_conn_state_t     link_state;
    app_dongle_ull_le_hid_link_mode_t      link_mode;
    app_dongle_ull_le_hid_link_mode_t      defualt_link_mode;
} app_dongle_ull_le_hid_link_t;

typedef struct {
    bt_ull_le_hid_srv_app_scenario_t                scenario;
    bt_ull_le_hid_srv_idle_time_t                   idle_time;
    app_dongle_ull_le_hid_connection_mode_t         connection_mode;
    struct {
        app_dongle_ull_le_hid_state_t               current_sate;
        app_dongle_ull_le_hid_state_t               next_state;
        bool                                        is_connecting;
    } state;
    struct {
        bool                                         wl_enable;
        app_dongle_ull_le_hid_set_white_list_state_t wl_state;
        app_dongle_ull_le_hid_scan_state_t           state_mask;
        app_dongle_ull_le_hid_scan_t                 scan_type_mask;
    } scan;
    app_dongle_ull_le_hid_link_t link_info[APP_DONGLE_ULL_LE_HID_LINK_NUM_MAX];
} app_dongle_ull_le_hid_contex_t;

#ifndef MTK_BT_TIMER_EXTERNAL_ENABLE
    typedef void (*app_dongle_ull_le_hid_timer_callback_t)(uint32_t timer_id, uint32_t data);
#else
    typedef bt_timer_ext_timeout_callback_t app_dongle_ull_le_hid_timer_callback_t;
#endif

#ifndef MTK_BT_TIMER_EXTERNAL_ENABLE
typedef struct {
    uint32_t                               timer_id;
    bool                                   is_start;
    TimerHandle_t                          timer_handle;
    app_dongle_ull_le_hid_timer_callback_t callback;
    uint32_t                               data;
} app_dongle_ull_le_hid_timer_info_t;

#define APP_DONGLE_ULL_LE_HID_TIMER_MAX     0x03
app_dongle_ull_le_hid_timer_info_t g_dongle_ull_le_hid_timer_info[APP_DONGLE_ULL_LE_HID_TIMER_MAX] = {0};
#endif

static app_dongle_ull_le_hid_contex_t g_app_dongle_ull_le_hid_ctx;
static app_dongle_le_race_event_callback_t g_ull_le_hid_callback = NULL;

static bt_status_t app_dongle_ull_le_hid_host_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff);
static void app_dongle_ull_le_hid_set_scan_state_mask(app_dongle_ull_le_hid_scan_state_t state);
static void app_dongle_ull_le_hid_clear_scan_state_mask(app_dongle_ull_le_hid_scan_state_t state);
static bt_status_t app_dongle_ull_le_hid_start_scan_device_internal(bool wl_enable);
static bt_status_t app_dongle_ull_le_hid_stop_scan_device_internal(void);
static bt_status_t app_dongle_ull_le_hid_start_connect_service(bt_handle_t handle);
static bt_status_t app_dongle_ull_le_hid_switch_to_cis(bt_handle_t handle);
static bt_status_t app_dongle_ull_le_hid_switch_to_acl(bt_handle_t handle);
static void app_dongle_ull_le_hid_encyption_complete_hdl(bt_status_t status, bt_gap_le_bonding_complete_ind_t *ind);
static void app_dongle_ull_le_hid_acl_disconnect_hdl(bt_status_t status, bt_gap_le_disconnect_ind_t *ind);
static void app_dongle_ull_le_hid_acl_connected_hdl(bt_status_t status, bt_gap_le_connection_ind_t *ind);


static void app_dongle_ull_le_hid_srv_bonding_complete_hdl(bt_ull_le_hid_srv_bonding_complete_ind_t *evt);
static void app_dongle_ull_le_hid_srv_switch_link_hdl(bt_ull_le_hid_srv_switch_link_mode_ind_t *evt);
static void app_dongle_ull_le_hid_srv_disconnected_hdl(bt_ull_le_hid_srv_disconnected_ind_t *evt);
static void app_dongle_ull_le_hid_srv_connection_complete_hdl(bt_ull_le_hid_srv_connected_ind_t *evt);
static void app_dongle_ull_le_hid_hid_srv_service_connected_hdl(bt_ull_le_hid_srv_service_connected_ind_t *evt);
static void app_dongle_ull_le_hid_print_addr(bt_addr_t *addr);
static bt_status_t app_dongle_ull_le_hid_cancel_connect(uint8_t device_type);
void app_dongle_ull_le_hid_factory_test_timeout_hdl(uint32_t timer_id, uint32_t data);
void app_dongle_ull_le_hid_fast_pairing_timeout_hdl(uint32_t timer_id, uint32_t data);
void app_dongle_ull_le_hid_create_cis_timeout_hdl(uint32_t timer_id, uint32_t data);
bt_status_t app_dongle_ull_le_hid_start_fast_pair(void);
bt_status_t app_dongle_ull_le_hid_start_factory_test(void);
void app_dongle_ull_le_hid_stop_fast_pair(void);
void app_dongle_ull_le_hid_stop_reconnect(void);
void app_dongle_ull_le_hid_stop_factory_test(void);
#if defined (AIR_PURE_GAMING_MS_ENABLE) || defined (AIR_PURE_GAMING_KB_ENABLE)
void app_dongle_ull_le_hid_ep_rx(bt_ull_user_data_t *evt);
#endif

#if defined (AIR_PURE_GAMING_ENABLE)
extern bt_status_t bt_ull_le_hid_dm_enter_test_mode(bt_ull_le_hid_srv_device_t device_type);
extern bt_status_t bt_ull_le_hid_dm_exit_test_mode(bt_ull_le_hid_srv_device_t device_type);
#endif

#ifdef USB_SUSPEND_BT_PF_PO_SUPPORT
extern void app_bt_state_service_set_bt_on_off(bool on, bool classic_off, bool need_do_rho, bool for_system_off);
#endif
extern void bt_ull_le_hid_conn_srv_set_cis_connection_timeout(uint16_t conn_timeout);
extern bt_status_t bt_gap_le_srv_set_extended_scan(bt_gap_le_srv_set_extended_scan_parameters_t *param, bt_gap_le_srv_set_extended_scan_enable_t *enable, void *callback);
extern void bt_ull_le_hid_conn_srv_set_create_cis_timeout(uint16_t create_cis_timeout);

static void *app_dongle_ull_le_hid_memcpy(void *dest, const void *src, uint32_t size)
{
    if ((NULL == dest) || (NULL == src)) {
        return NULL;
    }
    return memcpy(dest, src, size);
}

static void *app_dongle_ull_le_hid_memory_alloc(uint16_t size)
{
    void *memory = (void *)pvPortMalloc(size);
    if (NULL != memory) {
        memset(memory, 0, size);
    }
    return memory;
}

static void app_dongle_ull_le_hid_memory_free(void *point)
{
    if (point) {
        vPortFree(point);
    }
}

static void *app_dongle_ull_le_hid_memset(void *buf, uint8_t value, uint32_t size)
{
    return memset(buf, value, size);
}

static int32_t app_dongle_ull_le_hid_memcmp(const void *buf1, const void *buf2, uint32_t size)
{
    return memcmp(buf1, buf2, size);
}

#ifndef MTK_BT_TIMER_EXTERNAL_ENABLE
app_dongle_ull_le_hid_timer_info_t *app_dongle_ull_le_hid_get_timer_info_by_timer_id(uint32_t timer_id)
{
    uint32_t i = 0;

    for (i = 0; APP_DONGLE_ULL_LE_HID_TIMER_MAX > i; i++) {
        if (g_dongle_ull_le_hid_timer_info[i].timer_id == timer_id && g_dongle_ull_le_hid_timer_info[i].timer_handle != NULL) {
            return &g_dongle_ull_le_hid_timer_info[i];
        }
    }
    return NULL;
}

void app_dongle_ull_le_hid_reset_timer_info(app_dongle_ull_le_hid_timer_info_t *timer)
{
    if (timer) {
        app_dongle_ull_le_hid_memset(timer, 0, sizeof(app_dongle_ull_le_hid_timer_info_t));
    }
}

void app_dongle_ull_le_hid_timer_callback(TimerHandle_t timer_handle)
{
    uint8_t i = 0;
    if (timer_handle) {
        for (i = 0; i < APP_DONGLE_ULL_LE_HID_TIMER_MAX; i++) {
            if (g_dongle_ull_le_hid_timer_info[i].timer_handle == timer_handle && g_dongle_ull_le_hid_timer_info[i].callback) {
                g_dongle_ull_le_hid_timer_info[i].callback(g_dongle_ull_le_hid_timer_info[i].timer_id, g_dongle_ull_le_hid_timer_info[i].data);
                //xTimerDelete(g_dongle_ull_le_hid_timer_info[i].timer_handle, 0);
                //app_dongle_ull_le_hid_reset_timer_info(&g_dongle_ull_le_hid_timer_info[i]);
                g_dongle_ull_le_hid_timer_info[i].is_start = false;
                g_dongle_ull_le_hid_timer_info[i].data = 0;
                g_dongle_ull_le_hid_timer_info[i].callback = NULL;
                break;
            }
        }
    }
}

app_dongle_ull_le_hid_timer_info_t *app_dongle_ull_le_hid_get_avliable_timer_info(uint8_t timer_id)
{
    uint8_t i = 0;
    for (i = 0; APP_DONGLE_ULL_LE_HID_TIMER_MAX > i; i++) {
        if ((timer_id == g_dongle_ull_le_hid_timer_info[i].timer_id) && (NULL != g_dongle_ull_le_hid_timer_info[i].timer_handle)) {
             if (g_dongle_ull_le_hid_timer_info[i].is_start) {
                 return NULL;
             } else {
                 return &g_dongle_ull_le_hid_timer_info[i];
             }
        }
    }
    for (i = 0; APP_DONGLE_ULL_LE_HID_TIMER_MAX > i; i++) {
        if (NULL == g_dongle_ull_le_hid_timer_info[i].timer_handle) {
            app_dongle_ull_le_hid_memset(&g_dongle_ull_le_hid_timer_info[i], 0, sizeof(app_dongle_ull_le_hid_timer_info_t));
            g_dongle_ull_le_hid_timer_info[i].timer_id = timer_id;
            return &g_dongle_ull_le_hid_timer_info[i];
        }
    }
    return NULL;
}
#endif

void app_dongle_ull_le_hid_stop_timer(uint32_t timer_id)
{
#ifndef MTK_BT_TIMER_EXTERNAL_ENABLE
    app_dongle_ull_le_hid_timer_info_t *timer_info = app_dongle_ull_le_hid_get_timer_info_by_timer_id(timer_id);

    if (timer_info) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"stop timer. timer_id:%x", 1, timer_id);
        if (pdPASS == xTimerStop(timer_info->timer_handle, 0)) {
             timer_info->is_start = false;
             timer_info->data = 0;
             timer_info->callback = NULL;
        }
        //xTimerDelete(timer_info->timer_handle, 0);
        //app_dongle_ull_le_hid_reset_timer_info(timer_info);
    }
#else
    bt_timer_ext_status_t t_status = bt_timer_ext_stop(timer_id);
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"stop timer. timer_id:%x, status: %x", 2, timer_id, t_status);
#endif
}

bt_status_t app_dongle_ull_le_hid_start_timer(uint32_t timer_id, uint32_t timer_period, app_dongle_ull_le_hid_timer_callback_t callback, uint32_t data)
{
#ifndef MTK_BT_TIMER_EXTERNAL_ENABLE
    app_dongle_ull_le_hid_timer_info_t *timer_info = NULL;
    if (!timer_period || !callback) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"start timer fail-1. timer_id:%x period:%d", 2, timer_id, timer_period);
        return BT_STATUS_FAIL;
    }
    timer_info = app_dongle_ull_le_hid_get_avliable_timer_info(timer_id);
    if (!timer_info) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"start timer fail-2. timer_id:%x period:%d", 2, timer_id, timer_period);
        return BT_STATUS_FAIL;
    }
    if (timer_info->timer_handle) {
        if (timer_info->is_start) {
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"start timer fail-3. timer_id:%x period:%d", 2, timer_id, timer_period);
            return BT_STATUS_FAIL;
        }
        timer_info->callback = callback;
        timer_info->data = data;
        xTimerStart(timer_info->timer_handle, 0);
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"start timer-1. timer_id:%x period:%d", 2, timer_id, timer_period);
        return BT_STATUS_SUCCESS;
    }

    timer_info->timer_handle = xTimerCreate("app_dongle_ull_le_hid_timer",
                                            (timer_period / portTICK_PERIOD_MS),
                                            pdFALSE,
                                            NULL,
                                            app_dongle_ull_le_hid_timer_callback);

    if (timer_info->timer_handle) {
        timer_info->callback = callback;
        timer_info->data = data;
        xTimerStart(timer_info->timer_handle, 0);
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"start timer. timer_id:%x period:%d", 2, timer_id, timer_period);
        return BT_STATUS_SUCCESS;
    }
    APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"start timer fail. timer_id:%x period:%d", 2, timer_id, timer_period);
    return BT_STATUS_FAIL;
#else
    bt_timer_ext_status_t t_status = bt_timer_ext_start(timer_id, data, timer_period, callback);
    return (BT_TIMER_EXT_STATUS_SUCCESS == t_status) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
#endif
}

#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
void app_dongle_ull_le_hid_reset_timer(uint32_t timer_id, uint32_t tm_ms)
{
    bt_timer_ext_t *timer = bt_timer_ext_find(timer_id);
    if (timer) {
        uint32_t data = timer->data;
        bt_timer_ext_timeout_callback_t cb = timer->cb;
        bt_timer_ext_stop(timer_id);
        bt_timer_ext_start(timer_id, data, tm_ms, cb);
    }
}
#endif

app_dongle_ull_le_hid_contex_t *app_dongle_ull_le_hid_get_ctx(void)
{
    return &g_app_dongle_ull_le_hid_ctx;
}

static app_dongle_ull_le_hid_link_t *app_dongle_ull_le_hid_get_empty_link(void)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    uint8_t i = 0;
    for (i = 0; i < APP_DONGLE_ULL_LE_HID_LINK_NUM_MAX; i ++) {
        if (APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTED == ctx->link_info[i].link_state) {
            return &ctx->link_info[i];
        }
    }
    return NULL;
}

static app_dongle_ull_le_hid_link_t *app_dongle_ull_le_hid_get_link_by_handle(bt_handle_t handle)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    uint8_t i = 0;
    for (i = 0; i < APP_DONGLE_ULL_LE_HID_LINK_NUM_MAX; i ++) {
        if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED <= ctx->link_info[i].link_state \
            && ctx->link_info[i].handle == handle) {
            return &ctx->link_info[i];
        }
    }
    APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_get_link_info, no find link! handle: %x", 1, handle);
    return NULL;
}

static app_dongle_ull_le_hid_link_t *app_dongle_ull_le_hid_get_link_by_addr(bt_addr_t *addr)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    uint8_t i = 0;
    for (i = 0; i < APP_DONGLE_ULL_LE_HID_LINK_NUM_MAX; i ++) {
        if (!app_dongle_ull_le_hid_memcmp(&(ctx->link_info[i].peer_addr), addr, sizeof(bt_addr_t))) {
            return &ctx->link_info[i];
        }
    }
    APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_get_link_by_addr, no find link! addr: %2x-%2x...-%2x", 3, \
        addr->addr[0],addr->addr[1], addr->addr[5]);
    return NULL;
}

static void app_dongle_ull_le_hid_clear_link(bt_handle_t handle)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    uint8_t i = 0;
    for (i = 0; i < APP_DONGLE_ULL_LE_HID_LINK_NUM_MAX; i ++) {
        if (ctx->link_info[i].handle == handle) {
            app_dongle_ull_le_hid_memset(&ctx->link_info[i], 0, sizeof(app_dongle_ull_le_hid_link_t));
            break;
        }
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_clear_link, handle: %x", 1, handle);
}

static app_dongle_ull_le_hid_link_t *app_dongle_ull_le_hid_get_link_by_dt(bt_ull_le_hid_srv_device_t device_type)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    uint8_t i = 0;
    for (i = 0; i < APP_DONGLE_ULL_LE_HID_LINK_NUM_MAX; i ++) {
        if (ctx->link_info[i].device_type == device_type) {
            return &ctx->link_info[i];
        }
    }
    APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_get_link_info, no find link! device_type: %x", 1, device_type);
    return NULL;
}


static bool app_dongle_ull_le_hid_is_zero_addr(bt_addr_t *addr)
{
    bt_bd_addr_t zero_addr = {0};
    assert("Null addr!" && addr);
    if (!app_dongle_ull_le_hid_memcmp(&zero_addr, &addr->addr, sizeof(bt_bd_addr_t))) {
        return true;
    }
    return false;
}

static void app_dongle_ull_le_hid_set_link_state(app_dongle_ull_le_hid_link_t *link, uint8_t link_state)
{
    if (!link) {
        return;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Set Link(dt:%d) State: %d->%d", 3, link->device_type, link->link_state, link_state);
    link->link_state = link_state;

}
static void app_dongle_ull_le_hid_set_curr_state(app_dongle_ull_le_hid_state_t state)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Set Curr State: %d->%d", 2, ctx->state.current_sate, state);
    ctx->state.current_sate = state;
}

static uint8_t app_dongle_ull_le_hid_get_curr_state(void)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Get Curr State: %d", 1, ctx->state.current_sate);
    return ctx->state.current_sate;
}

static void app_dongle_ull_le_hid_set_next_state(app_dongle_ull_le_hid_state_t state)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Set Next State: %d->%d", 2, ctx->state.next_state, state);
    ctx->state.next_state = state;

}

static uint8_t app_dongle_ull_le_hid_get_next_state(void)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Get Next State:%d", 1, ctx->state.next_state);
    return ctx->state.next_state ;
}

static bt_status_t app_dongle_ull_le_hid_terminate(const bt_addr_t addr, app_dongle_cm_stop_source_param_t param)
{
    return BT_STATUS_SUCCESS;
}

static bt_status_t app_dongle_ull_le_hid_enable(const bt_addr_t addr, app_dongle_cm_start_source_param_t param)
{
    return BT_STATUS_SUCCESS;
}

static bool app_dongle_ull_le_hid_is_any_cancel_connecting_device(void)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    uint8_t i = 0;
    for (i = 0; i < APP_DONGLE_ULL_LE_HID_LINK_NUM_MAX; i ++) {
        if (ctx->link_info[i].link_state == APP_DONGLE_ULL_LE_HID_CONN_STATE_CANCEL_CONNECTING) {
                APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_is_any_cancel_connecting_device, dt: %x is connecting", 1, ctx->link_info[i].device_type);
            return true;
        }
    }
    return false;
}

void app_dongle_ull_le_hid_init(void)
{
    app_dongle_cm_handle_t app_dongle_ull_le_hid_handler = {
        app_dongle_ull_le_hid_enable,       /*Start connect function.*/
        app_dongle_ull_le_hid_terminate,     /*Stop all connect and scan function.*/
        NULL,                        /*Check adv data RSI info function.*/
    };
    app_dongle_cm_register_handle(APP_DONGLE_CM_SOURCE_HID, &app_dongle_ull_le_hid_handler);
	
    /* register host event callback */
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                        (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_SYSTEM),
                                        (void *)app_dongle_ull_le_hid_host_event_callback);
	
    uint32_t size = sizeof(bt_ull_le_hid_srv_app_scenario_t);
    nvkey_status_t status = nvkey_read_data(NVID_APP_DONGLE_ULL2_HID_SCENARIO, (uint8_t *)&g_app_dongle_ull_le_hid_ctx.scenario, &size);
    if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_init, SCENARIO error status:%d, size: %d", 2, status, size);
        return;
    }
    if (BT_ULL_LE_HID_SRV_APP_SCENARIO_NONE == g_app_dongle_ull_le_hid_ctx.scenario) {
        g_app_dongle_ull_le_hid_ctx.scenario = BT_ULL_LE_HID_SRV_APP_SCENARIO_1;
    }
	
    size = sizeof(bt_ull_le_hid_srv_idle_time_t);
    status = nvkey_read_data(NVID_APP_DONGLE_ULL2_HID_IDLE_TIME, (uint8_t *)&g_app_dongle_ull_le_hid_ctx.idle_time, &size);
    if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_init, IDLE_TIME error status:%d, size: %d", 2, status, size);
        return;
    }
}

static void app_dongle_ull_le_hid_print_addr(bt_addr_t *addr)
{
    if (!addr) {
         return;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_print_addr, addr_type: %d, addr: %x-%x-%x-%x-%x-%x!", 7,
        addr->type,
        addr->addr[0],
        addr->addr[1],
        addr->addr[2],
        addr->addr[3],
        addr->addr[4],
        addr->addr[5]
        );
}
bool app_dongle_ull_le_hid_any_bonded_device_exist(void)
{
    uint8_t audio_device_count = 0;
    uint8_t keyboard_count = 0;
    uint8_t mouse_count = 0;

#ifdef AIR_PURE_GAMING_ENABLE

#if defined (AIR_PURE_GAMING_MS_ENABLE)
    mouse_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    keyboard_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    mouse_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    keyboard_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#endif

#else
    audio_device_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
    keyboard_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    mouse_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#endif
    return ((mouse_count == 0) && (keyboard_count == 0) && (audio_device_count == 0)) ? false : true;
}

bt_status_t app_dongle_ull_le_hid_start_up(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;

    uint8_t audio_device_count = 0;
    uint8_t keyboard_count = 0;
    uint8_t mouse_count = 0;

#ifdef AIR_PURE_GAMING_ENABLE
#if defined (AIR_PURE_GAMING_MS_ENABLE)
    mouse_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    keyboard_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    mouse_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    keyboard_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#endif
#else
    audio_device_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
    keyboard_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    mouse_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#endif

    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_up, aud_count: %d, kb_count: %d, ms_count: %d", 3, \
        audio_device_count, keyboard_count, mouse_count);
    if (!audio_device_count && !keyboard_count && !mouse_count) {
        app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_WAITING);
        return status;
    }

    bt_ull_le_hid_srv_connect_t create_connection;
    bt_ull_le_hid_srv_conn_params_t *audio_sink = NULL;
    bt_ull_le_hid_srv_conn_params_t *key_board = NULL;
    bt_ull_le_hid_srv_conn_params_t *mouse = NULL;
    app_dongle_ull_le_hid_link_t *audio_info = NULL;
    app_dongle_ull_le_hid_link_t *kb_info = NULL;
    app_dongle_ull_le_hid_link_t *ms_info = NULL;

    if (audio_device_count) {
        audio_info = app_dongle_ull_le_hid_get_empty_link();
        if (audio_info) {
            APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_up, start reconnect", 0);
            app_dongle_ull_le_hid_memset(audio_info, 0 , sizeof(app_dongle_ull_le_hid_link_t));
            uint8_t len = sizeof(bt_ull_le_hid_srv_conn_params_t)+(sizeof(bt_addr_t) * audio_device_count) - 1;
            audio_sink = app_dongle_ull_le_hid_memory_alloc(len);
            assert( "out of memory!!" && (audio_sink != NULL));
            app_dongle_ull_le_hid_memset(audio_sink, 0 , len);
            audio_sink->list_num = audio_device_count;
            audio_sink->device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
            bt_ull_le_hid_srv_get_bonded_device_list(BT_ULL_LE_HID_SRV_DEVICE_HEADSET, audio_device_count, (bt_addr_t *)&audio_sink->peer_addr_list);
            audio_info->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING;
            audio_info->device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
            audio_info->is_bond = true;
            app_dongle_ull_le_hid_print_addr((bt_addr_t *)&audio_sink->peer_addr_list);

        } else {
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_up, HS no link source!", 0);
        }
    }
    if (keyboard_count) {
        kb_info = app_dongle_ull_le_hid_get_empty_link();
        if (kb_info) {
            app_dongle_ull_le_hid_memset(kb_info, 0 , sizeof(app_dongle_ull_le_hid_link_t));
            uint8_t len = sizeof(bt_ull_le_hid_srv_conn_params_t)+(sizeof(bt_addr_t) * keyboard_count) - 1;
            key_board = app_dongle_ull_le_hid_memory_alloc(len);
            assert( "out of memory!!" && (key_board != NULL));
            app_dongle_ull_le_hid_memset(key_board, 0 , len);
            key_board->list_num = keyboard_count;
            key_board->device_type = BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD;
            bt_ull_le_hid_srv_get_bonded_device_list(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD, keyboard_count, (bt_addr_t *)&key_board->peer_addr_list);
            kb_info->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING;
            kb_info->device_type = BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD;
            kb_info->is_bond = true;
        } else {
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_up, KB no link source!", 0);
        }
    }
    if (mouse_count) {
        ms_info = app_dongle_ull_le_hid_get_empty_link();
        if (ms_info) {
            app_dongle_ull_le_hid_memset(ms_info, 0 , sizeof(app_dongle_ull_le_hid_link_t));
            uint8_t len = sizeof(bt_ull_le_hid_srv_conn_params_t)+(sizeof(bt_addr_t) * mouse_count) - 1;
            mouse = app_dongle_ull_le_hid_memory_alloc(len);
            assert( "out of memory!!" && (mouse != NULL));
            app_dongle_ull_le_hid_memset(mouse, 0 , len);
            mouse->list_num = mouse_count;
            mouse->device_type = BT_ULL_LE_HID_SRV_DEVICE_MOUSE;
            bt_ull_le_hid_srv_get_bonded_device_list(BT_ULL_LE_HID_SRV_DEVICE_MOUSE, mouse_count, (bt_addr_t *)&mouse->peer_addr_list);
            ms_info->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING;
            ms_info->device_type = BT_ULL_LE_HID_SRV_DEVICE_MOUSE;
            ms_info->is_bond = true;
        } else {
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_up, MS no link source!", 0);
        }
    }
    if (!audio_sink && !key_board && !mouse) {
        return BT_STATUS_UNSUPPORTED;
    }
    create_connection.audio_sink = audio_sink;
    create_connection.keyboard = key_board;
    create_connection.mouse = mouse;
    status = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CONNECT, &create_connection, sizeof(bt_ull_le_hid_srv_connect_t));
    if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != status) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_up, Start connection fail status: 0x%X", 1, status);
        if (audio_sink) {
            app_dongle_ull_le_hid_memset(audio_info, 0, sizeof(app_dongle_ull_le_hid_link_t));
        }
        if (key_board) {
            app_dongle_ull_le_hid_memset(kb_info, 0, sizeof(app_dongle_ull_le_hid_link_t));
        }
        if (mouse) {
            app_dongle_ull_le_hid_memset(ms_info, 0, sizeof(app_dongle_ull_le_hid_link_t));
        }
    }
    if (audio_sink) {
        app_dongle_ull_le_hid_memory_free(audio_sink);
    }
    if (key_board) {
        app_dongle_ull_le_hid_memory_free(key_board);
    }
    if (mouse) {
        app_dongle_ull_le_hid_memory_free(mouse);
    }
    return status;
}

static void app_dongle_ull_le_hid_bt_power_on_hdl(void)
{
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_bt_power_on_hdl", 0);
#ifdef USB_SUSPEND_BT_PF_PO_SUPPORT
    static bool g_first_po = false;
    if (!g_first_po) {
        hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_UNLOCK);
        g_first_po = true;
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_bt_power_on_hdl unlock dvfs", 0);
    }
#else
    hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_UNLOCK);
#endif
    uint8_t mode = APP_ULL_HID_UNIVERSAL_DONGLE_ENABLE;//ull dongle mode,not headset; and ull feature is enable
    bt_hci_send_vendor_cmd(0xFDCC, (uint8_t *)&mode, sizeof(uint8_t));
    app_dongle_ull_le_hid_set_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLED);
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_SET_SCENARIO, &ctx->scenario, sizeof(bt_ull_le_hid_srv_app_scenario_t));
    bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_SET_IDLE_TIME, &ctx->idle_time, sizeof(bt_ull_le_hid_srv_idle_time_t));
#ifndef USB_SUSPEND_BT_PF_PO_SUPPORT
    bt_ull_le_hid_conn_srv_set_create_cis_timeout(0);
#endif

#ifdef AIR_PURE_GAMING_ENABLE
    bt_ull_le_hid_conn_srv_set_cis_connection_timeout(100);

#if defined (AIR_QC_DONGLE_MS_ENABLE)
    bt_ull_le_hid_dm_enter_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    app_dongle_ull_le_hid_start_up();
#elif defined (AIR_QC_DONGLE_KB_ENABLE)
    bt_ull_le_hid_dm_enter_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    app_dongle_ull_le_hid_start_up();
#else
    bool exist = app_dongle_ull_le_hid_any_bonded_device_exist();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_bt_power_on_hdl, exist: %d", 1, exist);
    if (exist) {
        app_dongle_ull_le_hid_start_fast_pair();
    } else {
        app_dongle_ull_le_hid_start_factory_test();
    }
#endif
#else
    app_dongle_ull_le_hid_start_up();
#endif
}

static void app_dongle_ull_le_hid_bt_power_off_hdl(void)
{
    //uint8_t mode = APP_ULL_DONGLE_ENABLE;//ull dongle mode,not headset; and ull feature is enable
    //bt_hci_send_vendor_cmd(0xFDCC, (uint8_t *)&mode, sizeof(uint8_t));
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_bt_power_off_hdl", 0);
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    uint8_t temp_scenario = ctx->scenario;
    app_dongle_ull_le_hid_memset(ctx, 0, sizeof(app_dongle_ull_le_hid_contex_t));
    ctx->scenario = temp_scenario;
}

static bt_status_t app_dongle_ull_le_hid_enable_scan(bool wl_enable)
{
    bt_status_t status;
    le_ext_scan_item_t ext_scan_1M_item = {
        .scan_type = BT_HCI_SCAN_TYPE_PASSIVE,
        .scan_interval = 0x30,
        .scan_window = 0x30,
    };
    bt_hci_cmd_le_set_extended_scan_enable_t enable = {
        .enable = BT_HCI_ENABLE,
        .filter_duplicates = BT_HCI_ENABLE,
        .duration = 0,
        .period = 0
    };

    bt_hci_le_set_ext_scan_parameters_t param = {
        .own_address_type = BT_HCI_SCAN_ADDR_RANDOM,
        .scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS,
        .scanning_phys_mask = BT_HCI_LE_PHY_MASK_1M,
        .params_phy_1M = &ext_scan_1M_item,
        .params_phy_coded = NULL,
    };
    param.scanning_filter_policy = wl_enable ? BT_HCI_SCAN_FILTER_ACCEPT_ONLY_ADVERTISING_PACKETS_IN_WHITE_LIST : BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS;
    if (BT_STATUS_SUCCESS != (status = bt_gap_le_srv_set_extended_scan(&param, &enable, NULL))) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"Enable scan failed, status:%x", 1, status);
        if (BT_STATUS_OUT_OF_MEMORY == status) {
            assert(0);
        }
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Enable ADV SCAN, status:%x", 1, status);
    return status;

}

static bt_status_t app_dongle_ull_le_hid_disable_scan(void)
{
    bt_status_t status;
    bt_hci_cmd_le_set_extended_scan_enable_t enable = {
        .enable = BT_HCI_DISABLE,
        .filter_duplicates = BT_HCI_DISABLE,
        .duration = 0,
        .period = 0
    };
    if (BT_STATUS_SUCCESS != (status = bt_gap_le_srv_set_extended_scan(NULL, &enable, NULL))) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"disable adv scan failed, status:%x", 1, status);
        if (BT_STATUS_OUT_OF_MEMORY == status) {
            assert(0);
        }
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"DISAVLE ADV SCAN, status:%x", 1, status);
    return status;
}

static void app_dongle_ull_le_hid_set_scan_state_mask(app_dongle_ull_le_hid_scan_state_t state)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    ctx->scan.state_mask |= state;
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Set scan state: %x, curr mask: 0x%x", 2, state, ctx->scan.state_mask);
}

static void app_dongle_ull_le_hid_clear_scan_state_mask(app_dongle_ull_le_hid_scan_state_t state)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    ctx->scan.state_mask &= (~ state);
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Clear scan state: %x, curr mask: 0x%x", 2, state, ctx->scan.state_mask);
}

static bt_status_t app_dongle_ull_le_hid_start_scan_device_internal(bool wl_enable)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    app_dongle_ull_le_hid_scan_state_t mask = ctx->scan.state_mask;
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_START_scan_device_internal, mask1: 0x%x, next_action: 0x%x, wl_enable: %d, wl_state: %d", 4, \
        mask, ctx->state.next_state, wl_enable, ctx->scan.wl_state);
    if (APP_DONGLE_ULL_LE_HID_SET_WHITE_LIST_STATE_ADD_ON_GOING == ctx->scan.wl_state || \
        APP_DONGLE_ULL_LE_HID_SET_WHITE_LIST_STATE_REMOVE_ON_GOING == ctx->scan.wl_state || \
        APP_DONGLE_ULL_LE_HID_SET_WHITE_LIST_STATE_CLEAR_ON_GOING == ctx->scan.wl_state) {
        return BT_STATUS_FAIL;
    }
    ctx->scan.wl_enable = wl_enable;
    if (APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLED == mask) {
        app_dongle_ull_le_hid_set_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLING);
        status = app_dongle_ull_le_hid_enable_scan(wl_enable);
        if (status) {
            app_dongle_ull_le_hid_clear_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLING);
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"Start Scan failed, status:%x", 1, status);
        }
    } else if (mask & APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLING) {
        app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_START_SCAN);
        status = BT_STATUS_PENDING;
    } else if (mask & APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLING || mask == APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLED) {
        status = BT_STATUS_SUCCESS;
    }

    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Start scan, mask2: 0x%x next_action: 0x%x", 2, ctx->scan.state_mask, ctx->state.next_state);
    return status;
}

static bt_status_t app_dongle_ull_le_hid_stop_scan_device_internal(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    app_dongle_ull_le_hid_scan_state_t mask = ctx->scan.state_mask;
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_STOP_scan_device_internal, mask1: 0x%x, next state: 0x%x", 2, \
        mask, ctx->state.next_state);

    if (APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLED == mask) {
        app_dongle_ull_le_hid_set_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLING);
        status = app_dongle_ull_le_hid_disable_scan();
        if (status) {
            app_dongle_ull_le_hid_clear_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLING);
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"Stop scan failed, status:%x", 1, status);
        }
    } else if (mask & APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLING) {
        app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_STOP_SCAN);
        status = BT_STATUS_PENDING;
    } else if (mask & APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLING || mask == APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLED) {
        status = BT_STATUS_SUCCESS;
    }

    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Stop Scan, mask1: 0x%x, mask2: 0x%x, next state: 0x%x", 3, mask, ctx->scan.state_mask, ctx->state.next_state);
    return status;

}

bt_status_t app_dongle_ull_le_hid_start_scan(app_dongle_ull_le_hid_scan_t scan_type)
{
    bt_status_t status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_scan, scan type:%x", 1, scan_type);
    if (APP_DONGLE_ULL_LE_HID_SCAN_NONE == scan_type) {
        return status;
    }

    ctx->scan.scan_type_mask |= scan_type;
    if (APP_DONGLE_ULL_LE_HID_SET_WHITE_LIST_STATE_COMPLETE != ctx->scan.wl_state && \
        APP_DONGLE_ULL_LE_HID_SET_WHITE_LIST_STATE_NONE != ctx->scan.wl_state) {
        app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_START_SCAN);
        APPS_LOG_MSGID_W(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_scan, scan Pending.", 0);
        return BT_STATUS_PENDING;
    }
    app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_START_SCAN);
    status = app_dongle_ull_le_hid_start_scan_device_internal(false);
    if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != status) {
        app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_scan, start scan fail!", 0);
    }
    return status;
}

#define RX_SETTING_DEBUG_ENABLE
#ifdef RX_SETTING_DEBUG_ENABLE
void app_dongle_ull_le_hid_print_rx_setting(void)
{
    // 2
    #define TOTAL_NUM (31)
    uint32_t read_value;
    int32_t j = 0;
    uint32_t addr_2wire[TOTAL_NUM] = {
        0x1ad,
        0x1ae,
        0x164,
        0x165,
        0x166,
        0x17e,
        0x17f,
        0x180,
        0x181,
        0x182,
        0x183,
        0x184,
        0x185,
        0x186,
        0x153  
    };
    
    for(j = 0; j < TOTAL_NUM; j++) {
        if(addr_2wire[j] != 0) {
            extern uint32_t DRV_2WIRE_Read(uint16_t Addr);
            read_value = DRV_2WIRE_Read(addr_2wire[j]); 
            printf("[HW RG]ADDR 0x%lx, value 0x%lx\n", addr_2wire[j], read_value);
        }
    }

    uint32_t addr_5[TOTAL_NUM] = {
        0xB1008008,
        0xB1008000,
        0xB1008004,
        0xB1008024,
        0xB1008028,
        0xB100802c,
        0xB1008030,
        0xB1008034,
        0xB1008038,
        0xB10080ec,
        0xB10080f0,
        0xB1000834,
        0xB1000838,
        0xB100083c,
        0xB1000864,
        0xB1000254
    };

    for(j = 0; j < TOTAL_NUM; j++) {
        if (addr_5[j] != 0) {
            read_value = *((volatile unsigned int *)addr_5[j]);
            printf("[HW RG]ADDR 0x%lx, value 0x%lx\n", addr_5[j], read_value);
        }
    }

}
#endif

bt_status_t app_dongle_ull_le_hid_stop_scan(app_dongle_ull_le_hid_scan_t scan_type)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_stop_scan, scan type:%x", 1, scan_type);
    ctx->scan.scan_type_mask &= (~scan_type);
#ifdef RX_SETTING_DEBUG_ENABLE
    app_dongle_ull_le_hid_print_rx_setting();
#endif
    if (APP_DONGLE_ULL_LE_HID_SCAN_NONE == ctx->scan.scan_type_mask) {
        app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_STOP_SCAN);
        status = app_dongle_ull_le_hid_stop_scan_device_internal();
        if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != status) {
            app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_scan, stop scan fail!", 0);
        }
    }
    return status;
}

static void app_dongle_ull_le_hid_scan_cnf_handler(bt_status_t status)
{
    bt_status_t scan_status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    app_dongle_ull_le_hid_scan_state_t mask = ctx->scan.state_mask;
    uint8_t next_state = app_dongle_ull_le_hid_get_next_state();
    uint8_t current_scan = app_dongle_ull_le_hid_get_curr_state();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_scan_cnf_handler, mask:%x, next_state: %x, current_scan: %d", 3, mask, next_state, current_scan);
    if (APP_DONGLE_ULL_LE_HID_STATE_START_SCAN == current_scan || APP_DONGLE_ULL_LE_HID_STATE_STOP_SCAN == current_scan) {
        app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
    }

    if (BT_STATUS_SUCCESS != status) {
        if (mask & APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLING) {
            app_dongle_ull_le_hid_clear_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLING);
            if (APP_DONGLE_ULL_LE_HID_STATE_STOP_SCAN == next_state) {
                app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
            } else if (APP_DONGLE_ULL_LE_HID_STATE_START_SCAN == next_state) {
                app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
                app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_START_SCAN);
                scan_status = app_dongle_ull_le_hid_start_scan_device_internal(ctx->scan.wl_enable);
                if (!scan_status) {
                    APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"start scan fail-1", 0);
                }
            }
        } else if (mask & APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLING) {
            app_dongle_ull_le_hid_clear_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLING);
            if (APP_DONGLE_ULL_LE_HID_STATE_START_SCAN == next_state) {
                app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
            } else if (APP_DONGLE_ULL_LE_HID_STATE_STOP_SCAN == next_state) {
                app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
                app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_STOP_SCAN);
                scan_status = app_dongle_ull_le_hid_stop_scan_device_internal();
                if (!scan_status) {
                    APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"stop scan fail", 0);
                }
            }
        }
    } else {
        if (mask & APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLING) {
            app_dongle_ull_le_hid_clear_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLING);
            app_dongle_ull_le_hid_clear_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLED);
            app_dongle_ull_le_hid_set_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLED);
            if (APP_DONGLE_ULL_LE_HID_STATE_STOP_SCAN == next_state) {
                app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
                app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_STOP_SCAN);
                scan_status = app_dongle_ull_le_hid_stop_scan_device_internal();
                if (!scan_status) {
                    app_dongle_ull_le_hid_clear_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLING);
                    APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"stop scan fail-1", 0);
                }
            } else if (APP_DONGLE_ULL_LE_HID_STATE_START_SCAN == next_state) {
                app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
            }
        } else if (mask & APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLING) {
            app_dongle_ull_le_hid_clear_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLING);
            app_dongle_ull_le_hid_clear_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLED);
            app_dongle_ull_le_hid_set_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLED);
            if (APP_DONGLE_ULL_LE_HID_STATE_START_SCAN == next_state) {
                app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
                app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_START_SCAN);
                scan_status = app_dongle_ull_le_hid_start_scan_device_internal(ctx->scan.wl_enable);
                if (!scan_status) {
                    app_dongle_ull_le_hid_clear_scan_state_mask(APP_DONGLE_ULL_LE_HID_SCAN_STATE_ENABLING);
                    APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"start scan fail-2", 0);
                }
            } else if (APP_DONGLE_ULL_LE_HID_STATE_STOP_SCAN == next_state) {
                app_dongle_ull_le_hid_set_next_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
            }
        }
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"scan cnf, mask1: 0x%x, mask2: 0x%x, next_action: 0x%x", 3, mask, ctx->scan.state_mask, next_state);
}

static bt_ull_le_hid_srv_device_t app_dongle_ull_le_hid_parse_adv_data(bt_gap_le_ext_advertising_report_ind_t *adv)
{
    uint16_t len_offset = 0;
    uint8_t device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
    if (!(adv->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_CONNECTABLE)) {
        return BT_ULL_LE_HID_SRV_DEVICE_NONE;
    }
    if ((!(adv->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_DIRECTED) &&(app_dongle_ull_le_hid_is_zero_addr(&adv->address))) \
        || ((adv->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_DIRECTED) && app_dongle_ull_le_hid_is_zero_addr(&adv->direct_address))) {
        return BT_ULL_LE_HID_SRV_DEVICE_NONE;
    }
    //APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"parse_adv_data, len: %d", 1, adv->data_length);
    while (len_offset < adv->data_length) {
        if (adv->data[len_offset] >= 3 && BT_GAP_LE_AD_TYPE_APPEARANCE == adv->data[len_offset + 1]) {
            if (((APPEARANCE_SERVICE_MEDIA_PLAYER_HEADSET & 0xFF) == adv->data[len_offset + 2]) &&
                (((APPEARANCE_SERVICE_MEDIA_PLAYER_HEADSET & 0xFF00) >> 8) == adv->data[len_offset + 3])) {
                device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
                APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Find Audio Headset!", 0);
            } else if (((APPEARANCE_SERVICE_HID_KEYBOARD & 0xFF) == adv->data[len_offset + 2]) &&
                (((APPEARANCE_SERVICE_HID_KEYBOARD & 0xFF00) >> 8) == adv->data[len_offset + 3])) {
                device_type = BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD;
                APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Find HID KeyBoard!", 0);
            } else if (((APPEARANCE_SERVICE_HID_MOUSE & 0xFF) == adv->data[len_offset + 2]) &&
                (((APPEARANCE_SERVICE_HID_MOUSE & 0xFF00) >> 8) == adv->data[len_offset + 3])) {
                device_type = BT_ULL_LE_HID_SRV_DEVICE_MOUSE;
                APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Find HID Mouse!", 0);
            }
        }
        len_offset += (adv->data[len_offset] + 1);
    }
    if (BT_ULL_LE_HID_SRV_DEVICE_HEADSET == device_type) {
        return device_type;
    } else if (BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD == device_type) {
        return device_type;
    } else if (BT_ULL_LE_HID_SRV_DEVICE_MOUSE == device_type) {
        return device_type;
    }
    return BT_ULL_LE_HID_SRV_DEVICE_NONE;
}

bt_status_t app_dongle_ull_le_hid_connect_internal(bt_addr_t *addr)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (!addr || app_dongle_ull_le_hid_is_zero_addr(addr)) {
        return status;
    }
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    if (ctx->state.is_connecting) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"connect_device, is connecting", 1, ctx->state.is_connecting);
        return status;
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
    app_dongle_ull_le_hid_memcpy(&(param.peer_address), addr, sizeof(bt_addr_t));

    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"connect_device, addrType:%x, addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      param.peer_address.type,
                      param.peer_address.addr[5],
                      param.peer_address.addr[4],
                      param.peer_address.addr[3],
                      param.peer_address.addr[2],
                      param.peer_address.addr[1],
                      param.peer_address.addr[0]);


    ctx->state.is_connecting = true;
    status = bt_gap_le_connect(&param);
    if (BT_STATUS_SUCCESS != status) {
        app_dongle_ull_le_hid_set_curr_state(APP_DONGLE_ULL_LE_HID_STATE_NONE);
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"connect_device, status is 0x%x", 1, status);
    return status;
}

static bt_status_t app_dongle_ull_le_disconnect_internal(bt_handle_t handle)
{
    bt_status_t status;
    bt_hci_cmd_disconnect_t param;

    if (BT_HANDLE_INVALID == handle) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"Disconnect HID device, invalid handle", 0);
        return BT_STATUS_FAIL;
    }

    param.connection_handle = handle;
    param.reason = BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST;
    status = bt_gap_le_disconnect(&param);
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Disconnect HID device, handle:%x ret:%x", 2, param.connection_handle, status);

    return status;
}

static bool app_dongle_ull_le_hid_need_fast_pair(bt_ull_le_hid_srv_device_t dev_type)
{
#if defined (AIR_PURE_GAMING_MS_ENABLE)
    if (dev_type == BT_ULL_LE_HID_SRV_DEVICE_MOUSE) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_need_fast_pair mouse", 0);
        return true;
    }
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    if (dev_type == BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_need_fast_pair keyboard", 0);
        return true;
    }
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    if (dev_type == BT_ULL_LE_HID_SRV_DEVICE_MOUSE || 
        dev_type == BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_need_fast_pair 2in1 dev_type(%d)", 1, dev_type);
        return true;
    }
#else
    if (dev_type == BT_ULL_LE_HID_SRV_DEVICE_MOUSE || 
        dev_type == BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD ||
        dev_type == BT_ULL_LE_HID_SRV_DEVICE_HEADSET) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_need_fast_pair 3in1 dev_type(%d)", 1, dev_type);
        return true;
    }
#endif
    return false;
}

static void app_dongle_ull_le_hid_adv_report_hdl(bt_status_t status, bt_gap_le_ext_advertising_report_ind_t *data)
{
    bt_ull_le_hid_srv_device_t device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
    bt_status_t bt_status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    if (BT_STATUS_SUCCESS != status || !data) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"adv report fail, status is 0x%x!", 1, status);
        return;
    }
    //APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"adv report, scan scan state: %x", 1, ctx->scan.state_mask);
    if (ctx->scan.state_mask & APP_DONGLE_ULL_LE_HID_SCAN_STATE_DISABLED) {
        return;
    }
    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FAST_PAIR == ctx->connection_mode) {
        if (data->rssi == 0x7F) {
            return;
        } else if (data->rssi >= APP_DONGLE_ULL_LE_HID_FAST_PAIRING_RSSI_THRESHOLD) {
            device_type = app_dongle_ull_le_hid_parse_adv_data(data);
            bool need_pair_flag = app_dongle_ull_le_hid_need_fast_pair(device_type);
            if (need_pair_flag) {
                bt_addr_t connect_addr = {0};
                app_dongle_ull_le_hid_memcpy(&connect_addr, &data->address, sizeof(bt_addr_t));
                APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Find fast pairing adv, dt: %d", 1, device_type);
                bt_status = app_dongle_ull_le_hid_connect_device(device_type, &connect_addr, true);
                if (BT_STATUS_SUCCESS == bt_status || BT_STATUS_PENDING == bt_status) {
                    app_dongle_ull_le_hid_stop_fast_pair();
                }
            }
        }
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"adv report addr: %x-%x-%x-%x-%x-%x, rssi %d", 7, 
            data->address.addr[0], 
            data->address.addr[1], 
            data->address.addr[2], 
            data->address.addr[3], 
            data->address.addr[4], 
            data->address.addr[5], 
            data->rssi);
    } else {
        device_type = app_dongle_ull_le_hid_parse_adv_data(data);
        switch (device_type) {
#ifdef AIR_PURE_GAMING_ENABLE

#if defined (AIR_PURE_GAMING_MS_ENABLE)
            case BT_ULL_LE_HID_SRV_DEVICE_MOUSE:{
                if (g_ull_le_hid_callback) {
                    g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_ADV_REPORT, APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS, data);
                }
                break;
            }
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
            case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                if (g_ull_le_hid_callback) {
                    g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_ADV_REPORT, APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB, data);
                }
                break;
            }
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
            case BT_ULL_LE_HID_SRV_DEVICE_MOUSE:{
                if (g_ull_le_hid_callback) {
                    g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_ADV_REPORT, APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS, data);
                }
                break;
            }
            case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                if (g_ull_le_hid_callback) {
                    g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_ADV_REPORT, APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB, data);
                }
                break;
            }
#endif

#else
            case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                if (g_ull_le_hid_callback) {
                    g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_ADV_REPORT, APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS, data);
                }
                break;
            }
            case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                if (g_ull_le_hid_callback) {
                    g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_ADV_REPORT, APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB, data);
                }
                break;
            }
            case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
                if (g_ull_le_hid_callback) {
                    g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_ADV_REPORT, APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS, data);
                }
                break;
            }
#endif
            default:
                break;
        }

    }

}

bt_status_t app_dongle_ull_le_hid_connect_device(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr, bool is_fast_pair)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_dt(device_type);
    if (NULL == link) {
        link = app_dongle_ull_le_hid_get_empty_link();
        if (!link) {
            assert(0); // for debug
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_connect_device, HS no link source!", 0);
        }
    } else {
        if (link->link_state != APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTED) {
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_connect_device, error state: %d!", 1, link->link_state);
            return BT_STATUS_FAIL;
        }
    }
    app_dongle_ull_le_hid_memset(link, 0 , sizeof(app_dongle_ull_le_hid_link_t));
    bool is_bond = bt_ull_le_hid_srv_is_bonded_device(device_type, addr);
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_connect_device, Start Connect dt: %d! bond: %d, fast_pair:%d", 3, device_type, is_bond, is_fast_pair);
    if (is_bond && !is_fast_pair) {
        uint8_t len = sizeof(bt_ull_le_hid_srv_conn_params_t) + sizeof(bt_addr_t) - 1;
        bt_ull_le_hid_srv_conn_params_t *audio_sink = app_dongle_ull_le_hid_memory_alloc(len);
        assert( "out of memory!!" && (audio_sink != NULL));
        app_dongle_ull_le_hid_memset(audio_sink, 0, len);
        audio_sink->list_num = 0x01;
        audio_sink->device_type = device_type;
        app_dongle_ull_le_hid_memcpy(&audio_sink->peer_addr_list, addr, sizeof(bt_addr_t));
        link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING;
        link->device_type = device_type;
        link->is_bond = true;
        bt_ull_le_hid_srv_connect_t create_connection;
        create_connection.audio_sink = audio_sink;
        create_connection.keyboard = NULL;
        create_connection.mouse = NULL;
        status = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CONNECT, &create_connection, sizeof(bt_ull_le_hid_srv_connect_t));
        if (BT_STATUS_SUCCESS != status) {
            link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTED;
            link->device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
            link->is_bond = false;
        }
        app_dongle_ull_le_hid_memory_free(audio_sink);
    } else {
        link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING;
        link->device_type = device_type;
        link->is_bond = false;
        status = app_dongle_ull_le_hid_connect_internal(addr);
        if (BT_STATUS_SUCCESS != status) {
            link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTED;
            link->device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
        } else {
            app_dongle_ull_le_hid_memcpy(&link->peer_addr, addr, sizeof(bt_addr_t));
        }
    }
    return status;
}

bt_status_t app_dongle_ull_le_hid_disconnect_all_device(bt_hci_disconnect_reason_t reason)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_disconnect_all_device, disconnect reason: %d!", 2, reason);

    app_dongle_ull_le_hid_link_t *ms = app_dongle_ull_le_hid_get_link_by_dt(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    app_dongle_ull_le_hid_link_t *hs = app_dongle_ull_le_hid_get_link_by_dt(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
    app_dongle_ull_le_hid_link_t *kb = app_dongle_ull_le_hid_get_link_by_dt(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    if (ms) {
        if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED <= ms->link_state) {
            status = app_dongle_ull_le_hid_disconnect_device(BT_ULL_LE_HID_SRV_DEVICE_MOUSE, reason, &ms->peer_addr);
        } else if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING == ms->link_state && reason != BT_HCI_STATUS_UNSPECIFIED_ERROR) {
            status = app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
        }
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_disconnect_all_device, ms status: %x!", 1, status);
    }
    if (hs) {
        if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED <= hs->link_state) {
            status = app_dongle_ull_le_hid_disconnect_device(BT_ULL_LE_HID_SRV_DEVICE_HEADSET, reason, &hs->peer_addr);
        } else if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING == hs->link_state && reason != BT_HCI_STATUS_UNSPECIFIED_ERROR) {
            status = app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
        }
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_disconnect_all_device, hs status: %x!", 1, status);
    }

    if (kb) {
        if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED <= kb->link_state) {
            status = app_dongle_ull_le_hid_disconnect_device(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD, reason, &kb->peer_addr);
        } else if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING == kb->link_state && reason != BT_HCI_STATUS_UNSPECIFIED_ERROR) {
            status = app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
        }
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_disconnect_all_device, kb status: %x!", 1, status);
    }

    return status;
}

bt_status_t app_dongle_ull_le_hid_disconnect_device(bt_ull_le_hid_srv_device_t device_type, uint8_t reason, bt_addr_t *addr)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    if (!addr) {
        return BT_STATUS_FAIL;
    }
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_addr(addr);
    if (!link) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_disconnect_device, Not found link!", 0);
        return BT_STATUS_FAIL;
    }

    uint8_t temp_state = link->link_state;
    link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTING;
    if (APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_ACL == link->link_mode) {
        status = app_dongle_ull_le_disconnect_internal(link->handle);
    } else if (APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_CIS == link->link_mode) {
        bt_ull_le_hid_srv_disconnect_t disconnect;
        disconnect.handle = link->handle;
        disconnect.reason = reason;
        status = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_DISCONNECT, &disconnect, sizeof(bt_ull_le_hid_srv_disconnect_t));
    } else {
        return BT_STATUS_FAIL;
    }

    if (BT_STATUS_SUCCESS != status) {
        link->link_state = temp_state;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_disconnect_device, disconnect dt: %d! status: %d, reason: %d", 3, link->device_type, status, reason);
    return status;
}

static void app_dongle_ull_le_hid_acl_connected_hdl(bt_status_t status, bt_gap_le_connection_ind_t *ind)
{
    uint8_t cur_state = app_dongle_ull_le_hid_get_curr_state();
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_addr(&ind->peer_addr);

    if (!link || !ind) {
        return;
    }
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();

    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"acl_connected_hdl, status: %x, cur_state: %x, dt: %d, handle: %d, is creating: %d", 5, \
         status, cur_state, link->device_type, ind->connection_handle, ctx->state.is_connecting);
    if (ctx->state.is_connecting) {
        ctx->state.is_connecting = false;
    } else {
        return;
    }

    if (BT_STATUS_SUCCESS != status) {
        app_dongle_ull_le_hid_memset(link, 0, sizeof(app_dongle_ull_le_hid_link_t));
        return;
    } else {
        if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING != link->link_state) {
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"acl_connected_hdl, state error: %x", 1, link->link_state);
            return;
        } else {
            BT_GATTC_NEW_EXCHANGE_MTU_REQ(req, APP_DONGLE_ULL_LE_HID_GATT_CLIENT_MTU);
            bt_gattc_exchange_mtu(ind->connection_handle, &req);
            link->handle = ind->connection_handle;
            link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED;
            link->link_mode = APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_ACL;
            app_dongle_ull_le_hid_memcpy(&link->peer_addr, &ind->peer_addr, sizeof(bt_addr_t));
            APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"HID LE CONNECTED!, status: %x, dt: %d", 2, status, link->device_type);
        }
    }
}

static void app_dongle_ull_le_hid_acl_disconnect_hdl(bt_status_t status, bt_gap_le_disconnect_ind_t *ind)
{
    bt_status_t ret;
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_handle(ind->connection_handle);
    if (!link || !ind) {
        return;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"HID LE ACL DISCONNECTED!, status: %x, dt: %d, mode: %d, state: %d, bond: %d", 5, \
         status, link->device_type, link->link_mode, link->link_state, link->is_bond);
    uint8_t temp_state = link->link_state;
    if (BT_STATUS_SUCCESS != status) {
        if (APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_ACL == link->link_mode && \
            APP_DONGLE_ULL_LE_HID_CONN_STATE_SWITCH_TO_CIS == link->link_state) {
            link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED;
        }
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"acl_disconnect_hdl, state error!", 0);
    } else {
        if (APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_ACL == link->link_mode && \
            APP_DONGLE_ULL_LE_HID_CONN_STATE_SWITCH_TO_CIS == link->link_state && link->is_bond
            ) { // to start cis streaming
            bt_ull_le_hid_srv_connect_t create_connection = {
                .audio_sink = NULL,
                .keyboard = NULL,
                .mouse = NULL,
            };
            uint16_t len = sizeof(bt_ull_le_hid_srv_conn_params_t)- 1 + sizeof(bt_addr_t);
            bt_ull_le_hid_srv_conn_params_t *connect = app_dongle_ull_le_hid_memory_alloc(len);
            assert("out of memory!" && (NULL != connect));
            connect->list_num = 0x01;
            connect->device_type = link->device_type;
            APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"acl_disconnect_hdl, length = %d", 1, len);
            app_dongle_ull_le_hid_memcpy(connect->peer_addr_list, &link->peer_addr, sizeof(bt_addr_t));
            switch (link->device_type) {
                case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                    create_connection.audio_sink = connect;
                    break;
                }
                case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                    create_connection.keyboard = connect;
                    break;
                }
                case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
                    create_connection.mouse = connect;
                    break;
                }
                default: {
                    app_dongle_ull_le_hid_memory_free(connect);
                    APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"acl_disconnect_hdl, error device type!", 0);
                    return;
                }
            }
            link->handle = BT_HANDLE_INVALID;
            link->link_mode = APP_DONGLE_ULL_LE_HID_LINK_MODE_NONE;
            link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING;
            ret = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CONNECT, &create_connection, sizeof(bt_ull_le_hid_srv_connect_t));
            if (BT_STATUS_SUCCESS == ret || BT_STATUS_PENDING == ret) {
                APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"acl_disconnect_hdl, Start Create CIS Success!", 0);
            } else {
                link->link_mode = APP_DONGLE_ULL_LE_HID_LINK_MODE_NONE;
                link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTED;
                link->device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
                APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"acl_disconnect_hdl, Start Create CIS Fail!!", 0);
            }
            app_dongle_ull_le_hid_memory_free(connect);
            return;
        }

        if (APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_ACL == link->link_mode) {
            bt_addr_t reconnect_addr;
            uint8_t dt = link->device_type;
            app_dongle_ull_le_hid_memcpy(&reconnect_addr, &link->peer_addr, sizeof(bt_addr_t));
            app_dongle_ull_le_hid_clear_link(ind->connection_handle);
            uint8_t power_state = bt_device_manager_power_get_power_state(BT_DEVICE_TYPE_LE);
            if (BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST != ind->reason \
                && (BT_DEVICE_MANAGER_POWER_STATE_STANDBY != power_state && BT_DEVICE_MANAGER_POWER_STATE_STANDBY_PENDING != power_state)) {
                APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"acl_disconnect_hdl, Reconnect ACL", 0);
                link = app_dongle_ull_le_hid_get_empty_link();
                if (!link) {
                    assert(0);
                }
                app_dongle_ull_le_hid_memset(link, 0 , sizeof(app_dongle_ull_le_hid_link_t));
                link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING;
                link->device_type = dt;
                status = app_dongle_ull_le_hid_connect_internal(&reconnect_addr);
                if (BT_STATUS_SUCCESS != status) {
                    link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTED;
                    link->device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
                    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"acl_disconnect_hdl, Reconnect ACL Fail!", 0);
                } else {
                    app_dongle_ull_le_hid_memcpy(&link->peer_addr, &reconnect_addr, sizeof(bt_addr_t));
                }
            }
            if (g_ull_le_hid_callback && temp_state >= APP_DONGLE_ULL_LE_HID_CONN_STATE_SERVICE_CONNECTED) {
                app_dongle_le_race_disconnect_ind_t dis_ind;
                dis_ind.ret = status;
                dis_ind.group_id = 0xFF;
                dis_ind.group_size = 0;
                app_dongle_ull_le_hid_memcpy(&dis_ind.peer_addr, &reconnect_addr, sizeof(bt_addr_t));
                app_dongle_le_race_sink_device_t type = APP_DONGLE_LE_RACE_SINK_DEVICE_NONE;
                switch (link->device_type) {
                    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                        type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS;
                        break;
                    }
                    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                        type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB;
                        break;
                    }
                    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
                        type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS;
                        break;
                    }
                    case BT_ULL_LE_HID_SRV_DEVICE_EARBUDS: {
                        type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB;
                        break;
                    }
                    default:
                        break;
                }
                g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, type, &dis_ind);
            }

        }
    }
}

#if defined (AIR_PURE_GAMING_MS_KB_ENABLE)
static void app_dongle_ull_le_hid_2in1_usb_suspend_handle(bt_ull_le_hid_srv_device_t device_type)
{
    if (device_type == BT_ULL_LE_HID_SRV_DEVICE_MOUSE) {
        app_dongle_ull_le_hid_link_t *kb_link_info = app_dongle_ull_le_hid_get_link_by_dt(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
        if (kb_link_info != NULL && APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED <= kb_link_info->link_state) {
            APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"srv_connection_complete_hdl, exist KB connection(%d)", 1, kb_link_info->link_state);
        } else {
            app_dongle_ull_le_hid_start_timer(APP_DONGLE_ULL_LE_HID_CREATE_CIS_TIMER, g_app_hid_establish_wait, app_dongle_ull_le_hid_create_cis_timeout_hdl, 0);
            app_bt_state_service_set_bt_on_off(false, false, true, false);
        }
    } else if (device_type == BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD) {
        app_dongle_ull_le_hid_link_t *ms_link_info = app_dongle_ull_le_hid_get_link_by_dt(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
        if (ms_link_info != NULL && APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED <= ms_link_info->link_state) {
            APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"srv_connection_complete_hdl, exist MS connection(%d)", 1, ms_link_info->link_state);
        } else {
            app_dongle_ull_le_hid_start_timer(APP_DONGLE_ULL_LE_HID_CREATE_CIS_TIMER, g_app_hid_establish_wait, app_dongle_ull_le_hid_create_cis_timeout_hdl, 0);
            app_bt_state_service_set_bt_on_off(false, false, true, false);
        }
    }
}
#endif

static void app_dongle_ull_le_hid_srv_connection_complete_hdl(bt_ull_le_hid_srv_connected_ind_t *evt)
{
    if (!evt) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"srv_connection_complete_hdl! evt is NULL!", 0);
        return;
    }

    if (BT_HANDLE_INVALID == evt->handle \
        || BT_ULL_LE_HID_SRV_DEVICE_NONE == evt->device_type \
        || app_dongle_ull_le_hid_is_zero_addr(&evt->peer_addr)) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"srv_connection_complete_hdl, handle: %x, dt: %x", 2, evt->handle, evt->device_type);
    }

    app_dongle_ull_le_hid_link_t *link_info = app_dongle_ull_le_hid_get_link_by_dt(evt->device_type);
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    if (NULL == link_info) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"srv_connection_complete_hdl, No find this type and alloc new link info!", 0);
        link_info = app_dongle_ull_le_hid_get_empty_link();
    }
    if (NULL == link_info) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"srv_connection_complete_hdl, No resource!", 0);
        return;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"srv_connection_complete_hdl, status: %d, dt: %d, link state: %d, conn_mode: %d, addr: %2x-%2x-%2x-%2x-%2x-%2x", 10, evt->status, evt->device_type, \
        link_info->link_state, ctx->connection_mode, evt->peer_addr.addr[0], evt->peer_addr.addr[1], evt->peer_addr.addr[2], evt->peer_addr.addr[3], evt->peer_addr.addr[4], evt->peer_addr.addr[5]);

    if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING == link_info->link_state \
        || APP_DONGLE_ULL_LE_HID_CONN_STATE_CANCEL_CONNECTING == link_info->link_state) {
        if (BT_STATUS_SUCCESS != evt->status) {
            app_dongle_ull_le_hid_memset(link_info, 0, sizeof(app_dongle_ull_le_hid_link_t));
            if (!app_dongle_ull_le_hid_is_any_cancel_connecting_device() && APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FACTORY_TEST == ctx->connection_mode) {
                ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;
                app_dongle_ull_le_hid_start_fast_pair();
                return;
            }
            if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_RECONNECT == ctx->connection_mode) {
                ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;
            }

            if (g_ull_le_hid_callback) {
                app_dongle_le_race_connect_ind_t connect_ind;
                connect_ind.ret = evt->status;
                connect_ind.group_id = 0xFF;
                connect_ind.group_size = 0;
                app_dongle_ull_le_hid_memcpy(&connect_ind.peer_addr, &evt->peer_addr, sizeof(bt_addr_t));
                app_dongle_le_race_sink_device_t type = APP_DONGLE_LE_RACE_SINK_DEVICE_NONE;
                switch (evt->device_type) {
                    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                        type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS;
                        break;
                    }
                    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                        type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB;
                        break;
                    }
                    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
                        type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS;
                        break;
                    }
                    case BT_ULL_LE_HID_SRV_DEVICE_EARBUDS: {
                        type = BT_ULL_LE_HID_SRV_DEVICE_EARBUDS;
                        break;
                    }
                    default:
                        break;
                }
                g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_CONNECT_IND, type, &connect_ind);
            }

            if(evt->status == BT_HCI_STATUS_DIRECTED_ADVERTISING_TIMEOUT) { // for usb suspend
#if defined (AIR_PURE_GAMING_MS_KB_ENABLE)
                app_dongle_ull_le_hid_2in1_usb_suspend_handle(evt->device_type);
#else
                app_dongle_ull_le_hid_start_timer(APP_DONGLE_ULL_LE_HID_CREATE_CIS_TIMER, g_app_hid_establish_wait, app_dongle_ull_le_hid_create_cis_timeout_hdl, 0);
#ifdef USB_SUSPEND_BT_PF_PO_SUPPORT
                app_bt_state_service_set_bt_on_off(false, false, true, false);
#endif
#endif
            }
            return;
        } else {
            link_info->handle = evt->handle;
            link_info->link_mode = APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_CIS;
            link_info->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED;
            app_dongle_ull_le_hid_memcpy(&link_info->peer_addr, &evt->peer_addr, sizeof(bt_addr_t));
        }
    } else if (APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTED == link_info->link_state) {
        if (BT_STATUS_SUCCESS != evt->status) {
            return;
        } else {
            link_info->handle = evt->handle;
            link_info->link_mode = APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_CIS;
            link_info->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED;
            app_dongle_ull_le_hid_memcpy(&link_info->peer_addr, &evt->peer_addr, sizeof(bt_addr_t));
            link_info->is_bond = true;
            link_info->device_type = evt->device_type;
        }
    }
    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FACTORY_TEST == ctx->connection_mode) {
        app_dongle_ull_le_hid_stop_timer(APP_DONGLE_ULL_LE_HID_FACTORY_TEST_TIMER);
#ifdef AIR_PURE_GAMING_ENABLE

#if defined (AIR_PURE_GAMING_MS_ENABLE)
        bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
        bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
        bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
        bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#endif

#else
        //bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
        //bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
        //bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#endif
        ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;
    } else if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE != ctx->connection_mode) {
        ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;
    }
    app_dongle_ull_le_hid_start_connect_service(evt->handle);

#if defined(AIR_PURE_GAMING_ENABLE) && defined(AIR_NVIDIA_REFLEX_ENABLE)
    if ((ctx->scenario == BT_ULL_LE_HID_SRV_APP_SCENARIO_4) || (ctx->scenario == BT_ULL_LE_HID_SRV_APP_SCENARIO_5))
    {
        hal_nvic_set_priority(USB_IRQn, USB_SW_IRQ_PRIORITY);
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"srv_connection_complete_hdl, lower USB priority!", 0);
    }
    else
    {
        hal_nvic_set_priority(USB_IRQn, USB_IRQ_PRIORITY);
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"srv_connection_complete_hdl, raise USB priority!", 0);
    }
#endif /* defined(AIR_PURE_GAMING_ENABLE) && defined(AIR_NVIDIA_REFLEX_ENABLE) */

#if defined(AIR_PURE_GAMING_ENABLE)
    // If usb suspend, call usb resume api
    usb_hid_chk_suspend_and_rmwk();
#endif
}

static void app_dongle_ull_le_hid_srv_disconnected_hdl(bt_ull_le_hid_srv_disconnected_ind_t *evt)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (!evt) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"HID service disconnect hdl error! evt is NULL!", 0);
        return;
    }
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_handle(evt->handle);
    if (NULL == link) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"HID service disconnect hdl fail! No find this link!", 0);
        return;
    }

    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"HID service disconnect hdl! status: 0x%X, handle: 0x%X, link_mode: 0x%X, link_state: 0x%X, dt: 0x%X, reason: 0x%X", 6, \
        evt->status, evt->handle, link->link_mode, link->link_state, evt->device_type, evt->reason);
    if (BT_STATUS_SUCCESS != evt->status || BT_HANDLE_INVALID == evt->handle || BT_ULL_LE_HID_SRV_DEVICE_NONE == evt->device_type) {
        return;
    }

    if (BT_HCI_STATUS_UNSPECIFIED_ERROR != evt->reason) {
        if (evt->device_type == BT_ULL_LE_HID_SRV_DEVICE_MOUSE 
            || evt->device_type == BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD) {
            app_usb_utils_hid_ms_release_pkt();
#if defined(AIR_PURE_GAMING_MS_ENABLE)
            app_usb_utils_hid_kb_release_pkt();
#else
            app_usb_utils_hid_nkey_release_pkt();
#endif
        }
    }

    bt_addr_t reconnect_addr = {0};
    app_dongle_ull_le_hid_memcpy(&reconnect_addr, &link->peer_addr, sizeof(bt_addr_t));
    if (APP_DONGLE_ULL_LE_HID_CONN_STATE_SWITCH_TO_ACL == link->link_state) {
        //to connect acl
        link->link_mode = APP_DONGLE_ULL_LE_HID_LINK_MODE_NONE;
        link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING;
        status = app_dongle_ull_le_hid_connect_internal(&link->peer_addr);
        if (BT_STATUS_SUCCESS != status) {
            link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTED;
            link->device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
            app_dongle_ull_le_hid_memset(&link->peer_addr, 0 , sizeof(bt_addr_t));
        }
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"srv_disconnected_hdl, Start Connect dt: %d! status: %d", 2, link->device_type, status);
    } else {
        app_dongle_ull_le_hid_clear_link(evt->handle);
        uint8_t power_state = bt_device_manager_power_get_power_state(BT_DEVICE_TYPE_LE);
        if (BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST != evt->reason \
            && (BT_DEVICE_MANAGER_POWER_STATE_STANDBY != power_state && BT_DEVICE_MANAGER_POWER_STATE_STANDBY_PENDING != power_state)) {
            bt_ull_le_hid_srv_connect_t create_connection = {NULL, NULL, NULL};
            uint16_t len = sizeof(bt_ull_le_hid_srv_conn_params_t)- 1 + sizeof(bt_addr_t);
            bt_ull_le_hid_srv_conn_params_t *reconnect = app_dongle_ull_le_hid_memory_alloc(len);
            assert("out of memory!" && (NULL != reconnect));
            reconnect->list_num = 0x01;
            reconnect->device_type = evt->device_type;
            if(BT_HCI_STATUS_UNSPECIFIED_ERROR == evt->reason){
                //for usb suspend
                bt_ull_le_hid_conn_srv_set_create_cis_timeout(g_app_hid_create_cis_timeout);
            } else{
                bt_ull_le_hid_conn_srv_set_create_cis_timeout(0);
            }
            app_dongle_ull_le_hid_memcpy(&reconnect->peer_addr_list, &reconnect_addr, sizeof(bt_addr_t));
            switch (evt->device_type) {
                case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                    create_connection.audio_sink = reconnect;
                    break;
                }
                case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                    create_connection.keyboard = reconnect;
                    break;
                }
                case BT_ULL_LE_HID_SRV_DEVICE_MOUSE:{
                    create_connection.mouse = reconnect;
                    break;
                }
                default: {
                    APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"HID service disconnect hdl fail! error device type!", 0);
                    return;
                }
            }
            app_dongle_ull_le_hid_link_t *reconn_link = app_dongle_ull_le_hid_get_empty_link();
            if (NULL == reconn_link) {
                APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"HID service disconnect hdl fail! No empty link!", 0);
                app_dongle_ull_le_hid_memory_free(reconnect);
                return;
            }
            app_dongle_ull_le_hid_memset(reconn_link, 0 , sizeof(app_dongle_ull_le_hid_link_t));
            reconn_link->device_type = evt->device_type;
            reconn_link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING;
            reconn_link->is_bond = true;
            APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"srv_disconnected_hdl, Start Reconnect! dt: %d! hs:%x, kb: %x, ms: %x", 4, \
                evt->device_type, create_connection.audio_sink, create_connection.keyboard, create_connection.mouse);
            status = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CONNECT, &create_connection, sizeof(bt_ull_le_hid_srv_connect_t));
            if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != status) {
                app_dongle_ull_le_hid_memset(reconn_link, 0 , sizeof(app_dongle_ull_le_hid_link_t));
            }
            app_dongle_ull_le_hid_memory_free(reconnect);
        } else if (BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST == evt->reason) {
            //TODO
        }
        /*notify pc tool*/
        if (g_ull_le_hid_callback) {
            app_dongle_le_race_disconnect_ind_t dis_ind;
            dis_ind.ret = status;
            dis_ind.group_id = 0xFF;
            dis_ind.group_size = 0;
            app_dongle_ull_le_hid_memcpy(&dis_ind.peer_addr, &reconnect_addr, sizeof(bt_addr_t));
            app_dongle_le_race_sink_device_t type = APP_DONGLE_LE_RACE_SINK_DEVICE_NONE;
            switch (evt->device_type) {
                case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                    type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS;
                    break;
                }
                case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                    type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB;
                    break;
                }
                case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
                    type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS;
                    break;
                }
                case BT_ULL_LE_HID_SRV_DEVICE_EARBUDS: {
                    type = BT_ULL_LE_HID_SRV_DEVICE_EARBUDS;
                    break;
                }
                default:
                    break;
            }
            g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, type, &dis_ind);
        }

    }
#ifdef AIR_PURE_GAMING_MS_ENABLE
    app_key_remap_disconnection_request();
#endif /* AIR_PURE_GAMING_MS_ENABLE */
}

static void app_dongle_ull_le_hid_encyption_complete_hdl(bt_status_t status, bt_gap_le_bonding_complete_ind_t *ind)
{
    bt_status_t ret;
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_handle(ind->handle);
    if (!link || !ind) {
        return;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Encryption Complete! status: %x, bond: %d!", 2, status, link->is_bond);
    if (BT_STATUS_SUCCESS != status) {
        app_dongle_ull_le_disconnect_internal(ind->handle);
    } else {
        if (!link->is_bond) {
            bt_ull_le_hid_srv_bond_t bonding;
            bonding.handle = ind->handle;
            APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"Encryption Complete! Start HID Bonding!", 0);
            ret = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_BOND, &bonding, sizeof(bt_ull_le_hid_srv_bond_t));
            if (BT_STATUS_SUCCESS == ret) {
                link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_BONDING;
            } else {
                APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"Encryption Complete! Start HID Bonding Fail!", 0);
            }
        } else {
            app_dongle_ull_le_hid_start_connect_service(ind->handle);
        }
    }
}

static bt_status_t app_dongle_ull_le_hid_host_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_POWER_ON_CNF: {
            app_dongle_ull_le_hid_bt_power_on_hdl();
            break;
        }
        case BT_POWER_OFF_CNF: {
            app_dongle_ull_le_hid_bt_power_off_hdl();
            break;

        }
        case BT_GAP_LE_SET_EXTENDED_SCAN_CNF: {
            app_dongle_ull_le_hid_scan_cnf_handler(status);
            break;
        }
        case BT_GAP_LE_EXT_ADVERTISING_REPORT_IND: {
            bt_gap_le_ext_advertising_report_ind_t *adv = (bt_gap_le_ext_advertising_report_ind_t *)buff;
            app_dongle_ull_le_hid_adv_report_hdl(status, adv);
            break;
        }
        case BT_GAP_LE_SET_WHITE_LIST_CNF: {
            break;
        }
        case BT_GAP_LE_CONNECT_IND: {
            bt_gap_le_connection_ind_t *ind = (bt_gap_le_connection_ind_t *)buff;
            app_dongle_ull_le_hid_acl_connected_hdl(status, ind);
            break;
        }
        case BT_GAP_LE_CONNECTION_UPDATE_IND: {
            break;
        }
        case BT_GAP_LE_BONDING_COMPLETE_IND: {
            bt_gap_le_bonding_complete_ind_t *ind = (bt_gap_le_bonding_complete_ind_t *)buff;
            app_dongle_ull_le_hid_encyption_complete_hdl(status, ind);
            break;
        }
        case BT_GAP_LE_DISCONNECT_IND: {
            bt_gap_le_disconnect_ind_t *ind = (bt_gap_le_disconnect_ind_t *)buff;
            app_dongle_ull_le_hid_acl_disconnect_hdl(status, ind);
            break;
        }
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t app_dongle_ull_le_hid_switch_to_cis(bt_handle_t handle)
{
    bt_status_t status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_handle(handle);
    if (!link) {
        return status;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"switch_to_CIS, state: %d, link mode: %d, handle: %d, dt: %d", 4, \
        link->link_state, link->link_mode, handle, link->device_type);
    if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED <= link->link_state &&
        APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTING >= link->link_state) {
        if (APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_ACL == link->link_mode) {
            link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_SWITCH_TO_CIS;
            status = app_dongle_ull_le_disconnect_internal(link->handle);
            if (BT_STATUS_SUCCESS != status) {
                APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"switch_to_CIS, disconnect acl fail!", 0);
            }
        } else if (APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_CIS == link->link_mode) {
           status = BT_STATUS_SUCCESS;
        } else {
            assert(0);
        }
    } else {
        status = BT_STATUS_FAIL;
    }
    return status;
}

static bt_status_t app_dongle_ull_le_hid_switch_to_acl(bt_handle_t handle)
{
    bt_status_t status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_handle(handle);
    if (!link) {
        return status;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"switch_to_ACL, state: %d, link mode: %d, handle: %d, dt: %d", 4, \
        link->link_state, link->link_mode, handle, link->device_type);
    if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING_SERVICE <= link->link_state) {
        if (APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_CIS == link->link_mode) {
            link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_SWITCH_TO_ACL;
            bt_ull_le_hid_srv_disconnect_t disconnect;

            disconnect.handle = link->handle;
            disconnect.reason = BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES;
            status = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_DISCONNECT, &disconnect, sizeof(bt_ull_le_hid_srv_disconnect_t));
            if (BT_STATUS_SUCCESS != status) {
                APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"switch_to_ACL, disconnect cis fail!", 0);
            }
        } else if (APP_DONGLE_ULL_LE_HID_LINK_MODE_OVER_CIS == link->link_mode) {
           status = BT_STATUS_SUCCESS;
        } else {
            assert(0);
        }
    } else {
        status = BT_STATUS_FAIL;
    }
    return status;

}

static void app_dongle_ull_le_hid_srv_switch_link_hdl(bt_ull_le_hid_srv_switch_link_mode_ind_t *evt)
{
    if (!evt) {
        return;
    }
    if (BT_ULL_LE_HID_SRV_LINK_MODE_NORMAL == evt->link_mode) {
        app_dongle_ull_le_hid_switch_to_cis(evt->handle);

    } else if (BT_ULL_LE_HID_SRV_LINK_MODE_FOTA == evt->link_mode) {
        app_dongle_ull_le_hid_switch_to_acl(evt->handle);
    }
    return;
}
static void app_dongle_ull_le_hid_srv_bonding_complete_hdl(bt_ull_le_hid_srv_bonding_complete_ind_t *evt)
{
    if (!evt) {
        return;
    }
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_handle(evt->handle);
    if (!link) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"srv_bonding_complete_hdl, bonding fail! %x", 1, evt->handle);
        return;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"srv_bonding_complete_hdl, dt: %d!, handle: %x, status: %x", 3, \
         link->device_type, evt->handle, evt->status);
    if (BT_STATUS_SUCCESS != evt->status) {
         link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_DISCONNECTING;
         app_dongle_ull_le_disconnect_internal(evt->handle);
         return;
    }
    link->link_state = APP_DONGLE_ULL_LE_HID_CONN_STATE_BOND_COMPLETE;
    link->is_bond = true;

#ifdef AIR_PURE_GAMING_ENABLE

#if defined (AIR_PURE_GAMING_MS_ENABLE)
    app_dongle_ull_le_hid_switch_to_cis(evt->handle);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    app_dongle_ull_le_hid_switch_to_cis(evt->handle);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    app_dongle_ull_le_hid_disconnect_device(link->device_type, BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST, &link->peer_addr);
#endif

#else
    app_dongle_ull_le_hid_disconnect_device(link->device_type, BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST, &link->peer_addr);
#endif

    //app_dongle_ull_le_hid_start_connect_service(evt->handle);
}

static bt_status_t app_dongle_ull_le_hid_start_connect_service(bt_handle_t handle)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_handle(handle);
    if (!link) {
        return BT_STATUS_FAIL;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"start_connect_service , dt: %d, state: %d", 2, link->device_type, link->link_state);
    if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED > link->link_state) {
        return BT_STATUS_FAIL;
    }
    bt_ull_le_hid_srv_connect_service_t connect;
    connect.handle = handle;
    status = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CONNECT_SERVICE, &connect, sizeof(bt_ull_le_hid_srv_connect_service_t));
    if (BT_STATUS_SUCCESS == status) {
        app_dongle_ull_le_hid_set_link_state(link, APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING_SERVICE);
    } else {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"start_connect_service fail!", 0);
    }
    return status;
}

static void app_dongle_ull_le_hid_hid_srv_service_connected_hdl(bt_ull_le_hid_srv_service_connected_ind_t *evt)
{
    if (!evt) {
        return;
    }
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_handle(evt->handle);
    if (!link) {
        return;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"srv_service_connected_hdl , dt: %d, state: %d, status: %x", 3, \
        link->device_type, link->link_state, evt->status);

    if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING_SERVICE == link->link_state) {
        if (BT_STATUS_SUCCESS != evt->status) {
            app_dongle_ull_le_hid_set_link_state(link, APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTED);
        } else {
            app_dongle_ull_le_hid_set_link_state(link, APP_DONGLE_ULL_LE_HID_CONN_STATE_SERVICE_CONNECTED);
        }
    } else {
        return;
    }
    if (g_ull_le_hid_callback) {
        app_dongle_le_race_connect_ind_t connect_ind;
        connect_ind.ret = evt->status;
        connect_ind.group_id = 0xFF;
        connect_ind.group_size = 0;
        app_dongle_ull_le_hid_memcpy(&connect_ind.peer_addr, &link->peer_addr, sizeof(bt_addr_t));
        app_dongle_le_race_sink_device_t type = APP_DONGLE_LE_RACE_SINK_DEVICE_NONE;
        switch (link->device_type) {
            case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS;
                break;
            }
            case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB;
                break;
            }
            case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
                type = APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS;
                break;
            }
            case BT_ULL_LE_HID_SRV_DEVICE_EARBUDS: {
                type = BT_ULL_LE_HID_SRV_DEVICE_EARBUDS;
                break;
            }
            default:
                break;
        }
        g_ull_le_hid_callback(APP_DONGLE_LE_RACE_EVT_CONNECT_IND, type, &connect_ind);
    }

#if defined(AIR_PURE_GAMING_KB_ENABLE)
    if (evt->status == BT_STATUS_SUCCESS) {
        app_usb_utils_hid_output_cache_send();
    }
#endif
}
void app_dongle_ull_le_hid_srv_event_callback(bt_ull_event_t event, void *param, uint32_t param_len)
{
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"hid_srv_event_callback event: 0x%X", 1, event);
    switch (event) {
        case BT_ULL_EVENT_LE_HID_CONNECTED_IND: {
            bt_ull_le_hid_srv_connected_ind_t *evt = (bt_ull_le_hid_srv_connected_ind_t *)param;
            app_dongle_ull_le_hid_srv_connection_complete_hdl(evt);
            break;
        }
        case BT_ULL_EVENT_LE_HID_DISCONNECTED_IND: {
            bt_ull_le_hid_srv_disconnected_ind_t *evt = (bt_ull_le_hid_srv_disconnected_ind_t *)param;
            app_dongle_ull_le_hid_srv_disconnected_hdl(evt);
            break;
        }
        case BT_ULL_EVENT_LE_HID_BONDING_COMPLETE_IND: {
            bt_ull_le_hid_srv_bonding_complete_ind_t *evt = (bt_ull_le_hid_srv_bonding_complete_ind_t *)param;
            app_dongle_ull_le_hid_srv_bonding_complete_hdl(evt);
            break;
        }
        case BT_ULL_EVENT_LE_STREAMING_START_IND: {
            break;
        }
        case BT_ULL_EVENT_LE_STREAMING_STOP_IND: {
            break;
        }
        case BT_ULL_EVENT_LE_HID_SWITCH_LINK_MODE_IND: {
            bt_ull_le_hid_srv_switch_link_mode_ind_t *evt = (bt_ull_le_hid_srv_switch_link_mode_ind_t *)param;
            app_dongle_ull_le_hid_srv_switch_link_hdl(evt);
            break;
        }
        case BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND: {
            bt_ull_le_hid_srv_service_connected_ind_t *evt = (bt_ull_le_hid_srv_service_connected_ind_t *)param;
            app_dongle_ull_le_hid_hid_srv_service_connected_hdl(evt);
            break;
        }
#if defined (AIR_PURE_GAMING_MS_ENABLE) || defined (AIR_PURE_GAMING_KB_ENABLE)
        case BT_ULL_EVENT_USER_DATA_IND: {
            bt_ull_user_data_t *evt = (bt_ull_user_data_t *)param;
            app_dongle_ull_le_hid_ep_rx(evt);
            break;
        }
#endif
        case BT_ULL_EVENT_LE_HID_INPUT_REPORT_IND: {
            break;
        }
        case BT_ULL_EVENT_LE_HID_RACE_DATA_IND: {
            break;
        }
    }
}

/*
bt_status_t app_dongle_ull_le_hid_send_race_data(bt_addr_t *addr, uint16_t len, uint8_t *race_data)
{
    bt_status_t status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_addr(addr);
    if (!link) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"send_race_data fail!", 0);
        return status;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"send_race_data, state: %d, dt: %d, addr: %2x-%2x-%2x-%2x-%2x-%2x", 8, link->link_state, \
        link->device_type, addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], addr->addr[4], addr->addr[5]);
    if (APP_DONGLE_ULL_LE_HID_CONN_STATE_SERVICE_CONNECTED > link->link_state) {
        return status;
    }
    bt_ull_le_hid_srv_race_data_t data;
    app_dongle_ull_le_hid_memcpy(&data.remote_address, &addr->addr, sizeof(bt_bd_addr_t));
    data.user_data_length = len;
    data.user_data = race_data;
    status = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_TX_RACE_DATA, &data, sizeof(bt_ull_le_hid_srv_race_data_t));
    return status;
}
*/

void app_dongle_ull_le_hid_register_race_callback(app_dongle_le_race_event_callback_t callback)
{
    if (callback) {
        g_ull_le_hid_callback = callback;
    }
}

bt_status_t app_dongle_ull_le_hid_cancel_create_connection(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG" app_dongle_ull_le_hid_cancel_create_connection, %x", 1, ctx->state.is_connecting);
    if (ctx->state.is_connecting) {
        status = bt_gap_le_cancel_connection();
    }
    return status;
}

static bt_status_t app_dongle_ull_le_hid_cancel_connect(bt_ull_le_hid_srv_device_t device_type) {
    app_dongle_ull_le_hid_link_t *link = app_dongle_ull_le_hid_get_link_by_dt(device_type);
    bt_status_t status = BT_STATUS_FAIL;
    if (link) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_cancel_connect dt: %d!", 1, device_type);
        if (APP_DONGLE_ULL_LE_HID_CONN_STATE_CONNECTING == link->link_state) {
            if (link->is_bond) {
                bt_ull_le_hid_srv_cancel_connect_t cancel;
                cancel.device_type = device_type;
                status = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CANCEL_CONNECT, &cancel, sizeof(bt_ull_le_hid_srv_cancel_connect_t));
            } else {
                status = app_dongle_ull_le_hid_cancel_create_connection();
            }
            if (BT_STATUS_SUCCESS == status || BT_STATUS_PENDING == status) {
                app_dongle_ull_le_hid_set_link_state(link, APP_DONGLE_ULL_LE_HID_CONN_STATE_CANCEL_CONNECTING);
            } else {
                APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_cancel_connect fail!", 0);
            }
            return status;
        }
    }
    return status;
}

void app_dongle_ull_le_hid_factory_test_timeout_hdl(uint32_t timer_id, uint32_t data)
{
    bt_status_t status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_factory_test_timeout_hdl conn_mode: %d.", 1, \
        ctx->connection_mode);
    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FACTORY_TEST != ctx->connection_mode) {
        return;
    }

#ifdef AIR_PURE_GAMING_ENABLE

#if defined (AIR_PURE_GAMING_MS_ENABLE)
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#endif

#else
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    //bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
    //bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    //bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#endif

    if (!app_dongle_ull_le_hid_is_any_cancel_connecting_device()) {
        ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;
        status = app_dongle_ull_le_hid_start_fast_pair();
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_factory_test_timeout_hdl status: %x!", 1, status);
}

void app_dongle_ull_le_hid_fast_pairing_timeout_hdl(uint32_t timer_id, uint32_t data)
{
    bt_status_t status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_fast_pairing_timeout_hdl conn mode: %x!", 1, ctx->connection_mode);
#ifdef AIR_PURE_GAMING_ENABLE
    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FAST_PAIR != ctx->connection_mode) {
        return;
    }
    bt_avm_set_rx_gain(BT_AVM_RX_GAIN_SCENARIO_SCAN, BT_AVM_NO_FIXED_RX_GAIN);

#if defined (AIR_PURE_GAMING_MS_ENABLE)
    status = app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    status = app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    status = app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD | APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
#endif

#else
    status = app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_AUDIO | APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD | APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
#endif

    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FAST_PAIR == ctx->connection_mode) {
        ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_fast_pairing_timeout_hdl status: %x!", 1, status);
    app_dongle_ull_le_hid_start_reconnect();
}

void app_dongle_ull_le_hid_create_cis_timeout_hdl(uint32_t timer_id, uint32_t data)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_hid_conn_srv_set_create_cis_timeout(g_app_hid_create_cis_timeout);
#ifdef USB_SUSPEND_BT_PF_PO_SUPPORT
    app_bt_state_service_set_bt_on_off(true, false, true, false);
#else
    status = app_dongle_ull_le_hid_start_reconnect();
#endif
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_create_cis_timeout_hdl status: %x!", 1, status);
}

bt_status_t app_dongle_ull_le_hid_start_fast_pair(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_fast_pair conn mode: %x!", 1, ctx->connection_mode);
    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE != ctx->connection_mode) {
        return status;
    }
    ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FAST_PAIR;
    bt_avm_set_rx_gain(BT_AVM_RX_GAIN_SCENARIO_SCAN, APP_DONGLE_ULL_LE_HID_FAST_PAIRING_RX_GAIN_VALUE);
#ifdef AIR_PURE_GAMING_ENABLE

#if defined (AIR_PURE_GAMING_MS_ENABLE)
    status = app_dongle_ull_le_hid_start_scan(APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    status = app_dongle_ull_le_hid_start_scan(APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    status = app_dongle_ull_le_hid_start_scan(APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD | APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
#endif

#else
    status = app_dongle_ull_le_hid_start_scan(APP_DONGLE_ULL_LE_HID_SCAN_AUDIO | APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD | APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
#endif
    if (BT_STATUS_PENDING == status || BT_STATUS_SUCCESS == status) {
        status = app_dongle_ull_le_hid_start_timer(APP_DONGLE_ULL_LE_HID_FAST_PAIRING_TIMER, APP_DONGLE_ULL_LE_HID_FAST_PAIRING_TIMEOUT, app_dongle_ull_le_hid_fast_pairing_timeout_hdl, 0);
    }
    return status;
}

bt_status_t app_dongle_ull_le_hid_start_reconnect(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_reconnect conn mode: %x!", 1, ctx->connection_mode);
    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE != ctx->connection_mode) {
        return status;
    }
    ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_RECONNECT;
    status = app_dongle_ull_le_hid_start_up();
    if (BT_STATUS_SUCCESS != status && BT_STATUS_PENDING != status) {
        ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;
    }
    return status;
}

bt_status_t app_dongle_ull_le_hid_start_factory_test(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_start_factory_test conn mode: %x!", 1, ctx->connection_mode);
    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE != ctx->connection_mode) {
        return status;
    }
    ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FACTORY_TEST;
#ifdef AIR_PURE_GAMING_ENABLE

#if defined (AIR_PURE_GAMING_MS_ENABLE)
    bt_ull_le_hid_dm_enter_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    bt_ull_le_hid_dm_enter_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    bt_ull_le_hid_dm_enter_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    bt_ull_le_hid_dm_enter_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#endif

#else
    //bt_ull_le_hid_dm_enter_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    //bt_ull_le_hid_dm_enter_test_mode(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
    //bt_ull_le_hid_dm_enter_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#endif
    status = app_dongle_ull_le_hid_start_up();
    if (BT_STATUS_PENDING == status || BT_STATUS_SUCCESS == status) {
        status = app_dongle_ull_le_hid_start_timer(APP_DONGLE_ULL_LE_HID_FACTORY_TEST_TIMER, 300, app_dongle_ull_le_hid_factory_test_timeout_hdl, 0);
    } else {
        ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;
    }
    return status;
}

void app_dongle_ull_le_hid_stop_fast_pair(void)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_stop_fast_pair conn mode: %x!", 1, ctx->connection_mode);
    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FAST_PAIR != ctx->connection_mode) {
        return;
    }
    ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;
    bt_avm_set_rx_gain(BT_AVM_RX_GAIN_SCENARIO_SCAN, BT_AVM_NO_FIXED_RX_GAIN);
    app_dongle_ull_le_hid_stop_timer(APP_DONGLE_ULL_LE_HID_FAST_PAIRING_TIMER);
#ifdef AIR_PURE_GAMING_ENABLE

#if defined (AIR_PURE_GAMING_MS_ENABLE)
    app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_MOUSE | APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD);
#endif

#else
    app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_AUDIO | APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD | APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
#endif
}

void app_dongle_ull_le_hid_stop_reconnect(void)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_stop_reconnect conn mode: %x!", 1, ctx->connection_mode);
    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_RECONNECT != ctx->connection_mode) {
        return;
    }
    ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;

#ifdef AIR_PURE_GAMING_ENABLE

#if defined (AIR_PURE_GAMING_MS_ENABLE)
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#endif

#else
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#endif
}

void app_dongle_ull_le_hid_stop_factory_test(void)
{
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_stop_factory_test conn mode: %x!", 1, ctx->connection_mode);
    if (APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_FACTORY_TEST != ctx->connection_mode) {
        return;
    }
    ctx->connection_mode = APP_DONGLE_ULL_LE_HID_CONNECTION_MODE_NONE;
    app_dongle_ull_le_hid_stop_timer(APP_DONGLE_ULL_LE_HID_FACTORY_TEST_TIMER);
#ifdef AIR_PURE_GAMING_ENABLE

#if defined (AIR_PURE_GAMING_MS_ENABLE)
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
#elif defined (AIR_PURE_GAMING_MS_KB_ENABLE)
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#endif

#else
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    app_dongle_ull_le_hid_cancel_connect(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
    //bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
    //bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
    //bt_ull_le_hid_dm_exit_test_mode(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
#endif
}

#if (defined MTK_RACE_CMD_ENABLE)
void app_dongle_ull_le_hid_get_device_list_handler(uint8_t race_channel, app_dongle_le_race_sink_device_t device_type)
{
    RACE_ERRCODE race_ret;
    uint8_t link_num = 0x00;
    uint8_t position = 0;
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG" get link_info type %d", 1, device_type);

    app_dongle_ull_le_hid_link_t *link = NULL;
    switch (device_type) {
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS: {
            link = app_dongle_ull_le_hid_get_link_by_dt(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
            break;
        }
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB: {
            link = app_dongle_ull_le_hid_get_link_by_dt(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
            break;
        }
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS: {
            link = app_dongle_ull_le_hid_get_link_by_dt(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
            break;
        }
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB: {
            break;
        }

    }
    if (link && APP_DONGLE_ULL_LE_HID_CONN_STATE_SERVICE_CONNECTED <= link->link_state) {
        link_num = 0x01;
    }
    app_dongle_le_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                         (uint16_t)RACE_DONGLE_LE_GET_DEVICE_LIST,
                                                                         (uint16_t)(sizeof(app_dongle_le_race_get_device_list_rsp_t) + link_num * sizeof(app_dongle_le_race_device_status_item_t)),
                                                                         race_channel);
    if (!response) {
        assert(0);
    }
    if (link_num > 0 && link) {
        app_dongle_ull_le_hid_memcpy(&response->devices_list[position].addr, &link->peer_addr, sizeof(bt_addr_t));
        response->devices_list[position].device_id = race_get_device_id_by_conn_address(&link->peer_addr.addr);
        response->devices_list[position].group_id = 0x01;
        response->devices_list[position].role = app_le_audio_air_get_role(&link->peer_addr);
    }

    response->status = 0;
    race_ret = race_noti_send(response, race_channel, false);
    if (race_ret != RACE_ERRCODE_SUCCESS) {
        RACE_FreePacket((void *)response);
    }
}

void app_dongle_ull_le_hid_get_paired_list_handler(uint8_t race_channel, app_dongle_le_race_sink_device_t device_type)
{
    RACE_ERRCODE race_ret;
    uint8_t link_num = 0x00;
    uint8_t i;
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG" get paired list, device_type %d", 1, device_type);

    bt_ull_le_hid_srv_conn_params_t *audio_sink = NULL;
    bt_ull_le_hid_srv_conn_params_t *key_board = NULL;
    bt_ull_le_hid_srv_conn_params_t *mouse = NULL;
    switch (device_type) {
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS: {
            uint8_t audio_device_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
            uint8_t len = sizeof(bt_ull_le_hid_srv_conn_params_t)+(sizeof(bt_addr_t) * audio_device_count) - 1;
            audio_sink = app_dongle_ull_le_hid_memory_alloc(len);
            assert( "out of memory!!" && (audio_sink != NULL));
            app_dongle_ull_le_hid_memset(audio_sink, 0 , len);
            audio_sink->list_num = audio_device_count;
            audio_sink->device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
            bt_ull_le_hid_srv_get_bonded_device_list(BT_ULL_LE_HID_SRV_DEVICE_HEADSET, audio_device_count, (bt_addr_t *)&audio_sink->peer_addr_list);
            link_num = audio_device_count;
            break;
        }
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB: {
            uint8_t keyboard_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
            uint8_t len = sizeof(bt_ull_le_hid_srv_conn_params_t)+(sizeof(bt_addr_t) * keyboard_count) - 1;
            key_board = app_dongle_ull_le_hid_memory_alloc(len);
            assert( "out of memory!!" && (key_board != NULL));
            app_dongle_ull_le_hid_memset(key_board, 0 , len);
            key_board->list_num = keyboard_count;
            key_board->device_type = BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD;
            bt_ull_le_hid_srv_get_bonded_device_list(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD, keyboard_count, (bt_addr_t *)&key_board->peer_addr_list);
            link_num = keyboard_count;
            break;
        }
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS: {
            uint8_t mouse_count = bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
            uint8_t len = sizeof(bt_ull_le_hid_srv_conn_params_t)+(sizeof(bt_addr_t) * mouse_count) - 1;
            mouse = app_dongle_ull_le_hid_memory_alloc(len);
            assert( "out of memory!!" && (mouse != NULL));
            app_dongle_ull_le_hid_memset(mouse, 0 , len);
            mouse->list_num = mouse_count;
            mouse->device_type = BT_ULL_LE_HID_SRV_DEVICE_MOUSE;
            bt_ull_le_hid_srv_get_bonded_device_list(BT_ULL_LE_HID_SRV_DEVICE_MOUSE, mouse_count, (bt_addr_t *)&mouse->peer_addr_list);
            link_num = mouse_count;
            break;
        }
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB: {
            break;
        }

    }
    app_dongle_le_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                         (uint16_t)RACE_DONGLE_LE_GET_PAIRED_LIST,
                                                                         (uint16_t)(sizeof(app_dongle_le_race_get_device_list_rsp_t) + link_num * sizeof(app_dongle_le_race_device_status_item_t)),
                                                                         race_channel);
    if (!response) {
        assert(0);
    }
    for(i = 0; i < link_num; i++){
        if(audio_sink){
            app_dongle_ull_le_hid_print_addr((bt_addr_t *)&audio_sink->peer_addr_list[i*sizeof(bt_addr_t)]);
            app_dongle_ull_le_hid_memcpy(&response->devices_list[i].addr, &audio_sink->peer_addr_list[i*sizeof(bt_addr_t)], sizeof(bt_addr_t));
            response->devices_list[i].device_id = race_get_device_id_by_conn_address(&((bt_addr_t *)&audio_sink->peer_addr_list[i*sizeof(bt_addr_t)])->addr);
            response->devices_list[i].group_id = 0x01;
            response->devices_list[i].role = app_le_audio_air_get_role((bt_addr_t *)&audio_sink->peer_addr_list[i*sizeof(bt_addr_t)]);
        }
        if(key_board){
            app_dongle_ull_le_hid_print_addr((bt_addr_t *)&key_board->peer_addr_list[i*sizeof(bt_addr_t)]);
            app_dongle_ull_le_hid_memcpy(&response->devices_list[i].addr, &key_board->peer_addr_list[i*sizeof(bt_addr_t)], sizeof(bt_addr_t));
            response->devices_list[i].device_id = race_get_device_id_by_conn_address(&((bt_addr_t *)&key_board->peer_addr_list[i*sizeof(bt_addr_t)])->addr);
            response->devices_list[i].group_id = 0x01;
            response->devices_list[i].role = app_le_audio_air_get_role((bt_addr_t *)&key_board->peer_addr_list[i*sizeof(bt_addr_t)]);
        }
        if(mouse){
            app_dongle_ull_le_hid_print_addr((bt_addr_t *)&mouse->peer_addr_list[i*sizeof(bt_addr_t)]);
            app_dongle_ull_le_hid_memcpy(&response->devices_list[i].addr, &mouse->peer_addr_list[i*sizeof(bt_addr_t)], sizeof(bt_addr_t));
            response->devices_list[i].device_id = race_get_device_id_by_conn_address(&((bt_addr_t *)&mouse->peer_addr_list[i*sizeof(bt_addr_t)])->addr);
            response->devices_list[i].group_id = 0x01;
            response->devices_list[i].role = app_le_audio_air_get_role((bt_addr_t *)&mouse->peer_addr_list[i*sizeof(bt_addr_t)]);
        }

    }

    response->status = 0;
    race_ret = race_noti_send(response, race_channel, false);
    if (audio_sink) {
        app_dongle_ull_le_hid_memory_free(audio_sink);
    }
    if (key_board) {
        app_dongle_ull_le_hid_memory_free(key_board);
    }
    if (mouse) {
        app_dongle_ull_le_hid_memory_free(mouse);
    }
    if (race_ret != RACE_ERRCODE_SUCCESS) {
        RACE_FreePacket((void *)response);
    }
}

void app_dongle_ull_le_hid_delete_device_info(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr)
{
    bt_ull_le_hid_dm_delete_device_info(device_type, addr);
}

void app_dongle_ull_le_hid_delete_device_by_dev_type(bt_ull_le_hid_srv_device_t cur_dev_type, bt_addr_t *addr)
{
    app_dongle_ull_le_hid_cancel_connect(cur_dev_type);
    app_dongle_ull_le_hid_delete_device_info(cur_dev_type, addr);
}

void app_dongle_ull_le_hid_delete_device(app_dongle_le_race_sink_device_t device_type, bt_addr_t *addr)
{
    bt_ull_le_hid_srv_device_t cur_dev_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG" app_dongle_ull_le_hid_delete_device, device_type 0x%x", 1, device_type);
    switch (device_type) {
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS: {
            cur_dev_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
            break;
        }
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB: {
            cur_dev_type = BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD;
            break;
        }
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS: {
            cur_dev_type = BT_ULL_LE_HID_SRV_DEVICE_MOUSE;
            break;
        }
        case APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB: {
            break;
        }
    }
    app_dongle_ull_le_hid_delete_device_by_dev_type(cur_dev_type, addr);
}

void app_dongle_ull_le_hid_get_device_status_handler(uint8_t race_channel, app_dongle_le_race_sink_device_t device_type,
    app_dongle_le_race_get_device_status_cmd_t *cmd)
{
    RACE_ERRCODE race_ret;
    app_dongle_ull_le_hid_link_t * link_info = app_dongle_ull_le_hid_get_link_by_addr(&cmd->addr);

    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG" get_device_status, handle 0x%x", 1, link_info->handle);
    app_dongle_le_race_get_device_status_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                           (uint16_t)RACE_DONGLE_LE_GET_DEVICE_STATUS,
                                                                           (uint16_t)(sizeof(app_dongle_le_race_get_device_status_rsp_t)),
                                                                           race_channel);

    if (response != NULL) {
        if (link_info) {
            response->status = 0;
            app_dongle_ull_le_hid_memcpy(&response->addr, &link_info->peer_addr, sizeof(bt_addr_t));
            response->device_id = race_get_device_id_by_conn_address(&response->addr.addr);
            response->group_id = 0x01;
            response->role = app_le_audio_air_get_role(&response->addr);
        } else {
            response->status = APP_DONGLE_LE_RACE_GET_STATUS_NOT_CONNECTED_STATUS;
            app_dongle_ull_le_hid_memcpy(&response->addr, &cmd->addr, sizeof(bt_addr_t));
            response->device_id = 0xFF;
            response->group_id = 0xFF;
            response->role = 0xFF;
        }
        race_ret = race_noti_send(response, race_channel, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }

}
#endif

void app_dongle_ull_le_hid_set_report_rate_cb(nvkey_status_t status, void *user_data)
{
    return;
}

bt_status_t app_dongle_ull_le_hid_set_scenaraio(bt_ull_le_hid_srv_app_scenario_t scenario)
{
    nvkey_status_t nv_status = NVKEY_STATUS_ERROR;
    bt_status_t bt_status = BT_STATUS_FAIL;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG" app_dongle_ull_le_hid_set_scenario, scenario: %d, curr_scenario: %d", 2, \
        scenario, ctx->scenario);
    if (BT_ULL_LE_HID_SRV_APP_SCENARIO_NONE == scenario) {
        return BT_STATUS_FAIL;
    }

    if (ctx->scenario == scenario) {
        return BT_STATUS_SUCCESS;
    }
    bt_ull_le_hid_srv_app_scenario_t temp_scenario = scenario;
    uint32_t size = sizeof(bt_ull_le_hid_srv_app_scenario_t);

    // In case of BT isr preempt, use non blocking api
    nv_status = nvkey_write_data_non_blocking(NVID_APP_DONGLE_ULL2_HID_SCENARIO, (uint8_t *)&temp_scenario, size, app_dongle_ull_le_hid_set_report_rate_cb, NULL);
    if (NVKEY_STATUS_OK == nv_status) {
        ctx->scenario = scenario;
        bt_status = bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_SET_SCENARIO, &temp_scenario, sizeof(bt_ull_le_hid_srv_app_scenario_t));
    }
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG" app_dongle_ull_le_hid_set_scenario, nv_status: %x, bt_status: %x", 2, \
        nv_status, bt_status);

    return bt_status;
}

bt_status_t app_dongle_ull_le_hid_get_scenaraio(bt_ull_le_hid_srv_app_scenario_t *p_scenario)
{
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    uint32_t size = sizeof(bt_ull_le_hid_srv_app_scenario_t);
    bt_ull_le_hid_srv_app_scenario_t scenario;
    app_dongle_ull_le_hid_contex_t *ctx = app_dongle_ull_le_hid_get_ctx();
    status = nvkey_read_data(NVID_APP_DONGLE_ULL2_HID_SCENARIO, (uint8_t *)&scenario, &size);
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG" app_dongle_ull_le_hid_get_scenaraio, scenario: %d, curr_scenario: %d, status: %x", 3, \
        scenario, ctx->scenario, status);

    if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_get_scenaraio, error status-1:%d, size: %d", 2, status, size);
        return BT_STATUS_FAIL;
    }
    if (BT_ULL_LE_HID_SRV_APP_SCENARIO_NONE == scenario) {
        *p_scenario = ctx->scenario;
        return BT_STATUS_SUCCESS;
    }
    *p_scenario = scenario;
    return BT_STATUS_SUCCESS;
}

bt_bd_addr_t *app_dongle_ull_le_hid_get_addr_by_device_type(uint8_t device_type)
{
    app_dongle_ull_le_hid_link_t *link_info = app_dongle_ull_le_hid_get_link_by_dt(device_type);
    return &link_info->peer_addr.addr;
}

bt_status_t app_dongle_ull_le_hid_fota_lock(bool lock)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_fota_lock, lock: %d", 1, lock);
    if (lock) {
        status = app_dongle_ull_le_hid_disconnect_all_device(BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION);
    } else {
        status = app_dongle_ull_le_hid_start_reconnect();
    }
    return status;
}

ATTR_TEXT_IN_TCM bt_ull_le_hid_srv_app_scenario_t app_dongle_ull_le_get_scenario_from_ctx()
{
    return g_app_dongle_ull_le_hid_ctx.scenario;
}

void app_dongle_ull_le_hid_stop_create_cis_timer(void)
{
    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_stop_create_cis_timer set cis timeout to 0", 0);
    bt_ull_le_hid_conn_srv_set_create_cis_timeout(0);
    app_dongle_ull_le_hid_stop_timer(APP_DONGLE_ULL_LE_HID_CREATE_CIS_TIMER);
}

void app_dongle_ull_le_clear_bond_info_by_device_type(bt_ull_le_hid_srv_device_t cur_device_type)
{
    extern bt_addr_t *bt_ull_le_hid_dm_addr_by_device_type(bt_ull_le_hid_srv_device_t cur_device_type);
    bt_addr_t *bt_addr = bt_ull_le_hid_dm_addr_by_device_type(cur_device_type);
    app_dongle_ull_le_hid_delete_device_by_dev_type(cur_device_type, bt_addr);
}

#if defined (AIR_PURE_GAMING_MS_ENABLE) || defined (AIR_PURE_GAMING_KB_ENABLE)
void app_dongle_ull_le_hid_ep_rx(bt_ull_user_data_t *evt)
{
    // data from DUT, need to send to PC by USB
    if (evt == NULL) {
        return;
    }

    uint16_t len = evt->user_data_length;
    uint8_t *data = evt->user_data;

    LOG_HEXDUMP_I(apps, "app_dongle_ull_le_hid_ep_rx data: ", data, len);

    if (data[0] != USB_HID_EPIO_TX_REPORT_ID || len != USB_HID_EPIO_TX_REPORT_LEN) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_ep_rx para error: id(0x%X), len(0x%X)", 2, data[0], len);
        return;
    }

    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_EPIO);
    if (port != USB_HID_MAX_DEVICE_NUM) {
        uint8_t tx_status = usb_hid_tx_non_blocking(port, data, len);
        if (tx_status != USB_HID_STATUS_OK) {
            APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_ep_rx fail: %d", 1, tx_status);
        }
    }
}

void app_dongle_ull_le_hid_ep_tx(uint8_t *data, uint32_t len)
{
    // data from PC, need to send to DUT by BT
    bt_ull_le_hid_srv_device_t device_type = 0;
#if defined (AIR_PURE_GAMING_MS_ENABLE)
    device_type = BT_ULL_LE_HID_SRV_DEVICE_MOUSE;
#elif defined (AIR_PURE_GAMING_KB_ENABLE)
    device_type = BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD;
#else
    return;
#endif
    
    app_dongle_ull_le_hid_link_t *link_info = app_dongle_ull_le_hid_get_link_by_dt(device_type);
    if (link_info == NULL) {
        return;
    }

    APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_ep_tx: DUT address(0x%X 0x%X 0x%X 0x%X 0x%X 0x%X)", 6, \
        link_info->peer_addr.addr[0], \
        link_info->peer_addr.addr[1], \
        link_info->peer_addr.addr[2], \
        link_info->peer_addr.addr[3], \
        link_info->peer_addr.addr[4], \
        link_info->peer_addr.addr[5]);

    bt_ull_user_data_t tx_data;
    memcpy(&tx_data.remote_address, &link_info->peer_addr.addr, sizeof(bt_bd_addr_t));
    tx_data.user_data = data;
    tx_data.user_data_length = len;
    bt_status_t ret = bt_ull_le_hid_srv_action(BT_ULL_ACTION_TX_USER_DATA, &tx_data, sizeof(tx_data));
    if (ret != BT_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_ep_tx fail: 0x%X", 1, ret);
    }
}

#include "apps_events_event_group.h"
#include "apps_events_usb_event.h"
void app_dongle_ull_le_hid_ep_cb(uint8_t *data, uint32_t len)
{
    // data from PC, need to send to DUT by BT
    if (data == NULL) {
        return;
    }

    if (data[0] != USB_HID_EPIO_RX_REPORT_ID || len != USB_HID_EPIO_RX_REPORT_LEN) {
        return;
    }

    LOG_HEXDUMP_I(apps, "app_dongle_ull_le_hid_ep_cb data: ", data, len);
    
    void *extra_data = pvPortMalloc(len);
    if (extra_data == NULL) {
        APPS_LOG_MSGID_I(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_ep_cb: malloc fail", 0);
        return;
    }
    memcpy(extra_data, data, len);
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_USB_AUDIO,
                    APPS_EVENTS_USB_HID_EP_TX, extra_data, len, NULL, 0);
}

void app_dongle_ull_le_hid_ep_tx_reg(void)
{
    USB_HID_HANDLER_t ret = usb_hid_handler_rx_register(USB_HID_EPIO_RX_REPORT_ID, USB_HID_EPIO_REPORT_LEN, app_dongle_ull_le_hid_ep_cb);
    if (ret != USB_HID_HANDLER_STATUS_OK) {
        APPS_LOG_MSGID_E(APP_DONGLE_HID_LOG_TAG"app_dongle_ull_le_hid_ep_tx_reg fail: %d", 1, ret);
    }
}
#endif

#endif
