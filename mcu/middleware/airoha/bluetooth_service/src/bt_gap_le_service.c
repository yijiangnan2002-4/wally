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

#include <stdlib.h>

#include "bt_gap_le.h"
#include "bt_gap_le_service.h"
#include "bt_gap_le_service_utils.h"
#include "bt_device_manager.h"
#include "bt_device_manager_le.h"
#include "bt_callback_manager.h"

#include "hal_wdt.h"
#include "hal.h"
#ifdef AIR_REPLACE_NVDM_WITH_NVKEY
#include "nvdm.h"
#include "nvkey_id_list.h"
#include "nvkey.h"
#endif

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
#include "bt_role_handover.h"
#endif
#endif
#include "bt_utils.h"
#include "bt_os_layer_api.h"

#include "bt_os_layer_api.h"

#define BT_DM_POWER

/**********************************utils************************/
#define BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG(CHECK_CONDITION, RET_VALUE, LOG_STRING, ...) \
    if (CHECK_CONDITION) {  \
        bt_gap_le_srv_report_id(LOG_STRING, ##__VA_ARGS__); \
        return (RET_VALUE); \
    }

#define BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE(CHECK_CONDITION, RET_VALUE)   \
    if (CHECK_CONDITION) { \
        return (RET_VALUE); \
    }

#define BT_GAP_LE_SRV_CHECK_RET_WITH_LOG(CHECK_CONDITION, LOG_STRING, ...) \
    if (CHECK_CONDITION) {  \
        bt_gap_le_srv_report_id(LOG_STRING, ##__VA_ARGS__); \
        return; \
    }

#define BT_GAP_LE_SRV_CHECK_RET_NO_VALUE_LOG(CHECK_CONDITION)    \
    if (CHECK_CONDITION) {  \
        return; \
    }


#define BT_GAP_LE_SRV_EVENT_CALLBACK_MAX       8

/**********************************Defines************************/
/**
 * @brief Bluetooth GAP LE service connection info type.
 */
#define BT_GAP_LE_SRV_INFO_TYPE_CONNECTION_INFOS   0x00
#define BT_GAP_LE_SRV_INFO_TYPE_CONNECTION_PARAMS  0x01
typedef uint8_t bt_gap_le_srv_info_t;

/**
 * @brief Bluetooth GAP LE service sub state.
 */
#define BT_CM_LE_ADV_SUB_STATE_IDLE           0x00
#define BT_CM_LE_ADV_SUB_STATE_CONFIGURING    (0x01 << 0)
#define BT_CM_LE_ADV_SUB_STATE_UPDATING       (0x01 << 1)
#define BT_CM_LE_ADV_SUB_STATE_REMOVING       (0x01 << 2)
#define BT_CM_LE_ADV_SUB_STATE_FORCE_STOPPING (0x01 << 3)
typedef uint8_t bt_gap_le_srv_adv_sub_state_t;

#define BT_GAP_LE_SRV_DISABLE_BLE_REASON_NONE                     0x00
#define BT_GAP_LE_SRV_DISABLE_BLE_REASON_ROLE_RECOVERY            0x01
#define BT_GAP_LE_SRV_DISABLE_BLE_REASON_POWER_OFF                0x02
typedef uint8_t bt_gap_le_srv_disable_ble_reason_t;

/**
 *  @brief Advertising infomation structure.
 */
typedef struct {
    uint8_t       instance;
    bool          configured;
    int8_t        selected_tx_power;          /**< Current selected Tx power by controller. */
    bt_gap_le_srv_adv_state_t         state;
    bt_gap_le_srv_adv_sub_state_t     sub_state;
    uint8_t num_ext_adv_events;
    uint16_t      conn_handle;
    bt_gap_le_srv_event_cb_t evt_cb;
    bt_gap_le_srv_get_adv_data_cb_t data_cb;
    union {
        bt_hci_cmd_le_set_advertising_parameters_t param;
        struct {
            uint8_t random_addr[6];
            bt_hci_le_set_ext_advertising_parameters_t ext_param;
            bt_hci_le_set_ext_advertising_enable_t  ext_enable;
        };
    };
} bt_gap_le_srv_adv_info_t;

BT_PACKED(
typedef struct {
    bt_handle_t                 connection_handle;
    uint8_t                     adv_handle;
    bt_bd_addr_t                peer_rpa;
    bt_gap_le_srv_event_cb_t    update_cb;
    bt_gap_le_srv_conn_info_t   conn_info;
    bt_gap_le_srv_conn_params_t conn_params;
}) bt_gap_le_srv_conn_cntx_t;

typedef struct {
    bt_utils_linknode_t                    node;
    bt_gap_le_set_white_list_op_t          op;
    bt_addr_t                              address;
    bt_gap_le_srv_event_cb_t               callback;
} bt_gap_le_srv_white_list_node_t;

#define BT_GAP_LE_SRV_FLAG_DISABLE_BLE            (0x00001)
#define BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL         (0x00002)
#define BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST  (0x00004)
typedef uint16_t bt_gap_le_srv_flag_t;

#define BT_GAP_LE_SRV_CONN_FILTER_CMD_RSL           0x01
#define BT_GAP_LE_SRV_CONN_FILTER_CMD_WHITE_LIST    0x02          
typedef uint8_t bt_gap_le_srv_conn_filter_cmd_t;

typedef struct {
    bt_gap_le_srv_get_link_attribute_callback_t attribute_callback;
    bt_gap_le_srv_event_cb_t                    event_callback[BT_GAP_LE_SRV_EVENT_CALLBACK_MAX];
    bt_gap_le_srv_flag_t                        flag;
    bt_utils_linknode_t                         white_list_run;
} bt_gap_le_srv_context_t;

/**
 *  @brief Disable BLE info structure.
 */
typedef struct {
    bool disconnecting_all;
    uint8_t disconn_num;
    bt_gap_le_srv_disable_ble_reason_t reason;
    bt_gap_le_srv_event_cb_t app_cb;
} bt_gap_le_srv_disable_ble_info_t;

typedef struct {
    bool clearing;
    bt_gap_le_srv_event_cb_t app_cb;
} bt_gap_le_srv_adv_clear_info_t;

typedef struct {
    bool clearing;
    bt_gap_le_srv_event_cb_t app_cb;
} bt_gap_le_srv_conn_clear_info_t;

typedef struct {
    bt_handle_t new_conn_handle;
    bt_handle_t old_conn_handle;
} bt_gap_le_srv_handle_table_t;

static bt_gap_le_srv_context_t le_srv_context;

typedef uint8_t bt_gap_le_adv_wl_action_t;
#define BT_GAP_LE_ADV_WL_ACTION_START     0x01
#define BT_GAP_LE_ADV_WL_ACTION_UPDATE    0x02
#define BT_GAP_LE_ADV_WL_ACTION_STOP      0x03
#define BT_GAP_LE_ADV_WL_ACTION_REMOVE    0x04
#define BT_GAP_LE_ADV_WL_ACTION_STOP_ALL  0x05

typedef struct {
    bt_gap_le_srv_linknode_t        node;
    bt_gap_le_adv_wl_action_t       wl_action;
    uint8_t                         instance;
    bt_bd_addr_t                    random_addr;
    bt_gap_le_srv_adv_time_params_t time_param;
    void                            *adv_data_cb;
    void                            *adv_evt_cb;
} bt_gap_le_adv_waiting_list_context_t;

#define BT_GAP_LE_SRV_ADV_FLAG_STOPPING_ALL            (0x00001)
typedef uint16_t bt_gap_le_srv_adv_flag_t;
typedef struct {
    bt_gap_le_srv_adv_flag_t flag;
} bt_gap_le_adv_context_t;

static bt_gap_le_srv_linknode_t adv_wl_list;

#ifdef BT_DM_POWER
#include "bt_device_manager_power.h"
/**
 *  @brief Define for the bluetooth LE power state.
 */
#define BT_GAP_LE_SRV_POWER_STATE_OFF           (0x00)  /**< The bluetooth LE power off state. */
#define BT_GAP_LE_SRV_POWER_STATE_OFF_PENDING   (0x01)  /**< The bluetooth LE power off is ongoing. */
#define BT_GAP_LE_SRV_POWER_STATE_ON            (0x02)  /**< The bluetooth LE power on state. */
#define BT_GAP_LE_SRV_POWER_STATE_ON_PENDING    (0x03)  /**< The bluetooth LE power on is ongoing. */
#define BT_GAP_LE_SRV_POWER_STATE_RESETING      (0x04)  /**< The bluetooth LE power is reseting. */
typedef uint8_t bt_gap_le_srv_power_state_t;    /**< The bluetooth LE power state type. */

typedef struct {
    bt_gap_le_srv_power_state_t cur_state;
    bt_gap_le_srv_power_state_t target_state;
} bt_gap_le_srv_power_cntx_t;

static bt_gap_le_srv_power_cntx_t g_ble_srv_power_cntx;
#endif

static bt_gap_le_srv_disable_ble_info_t g_disable_ble_info;
static bt_gap_le_srv_adv_clear_info_t g_le_adv_clear;
static bt_gap_le_srv_conn_clear_info_t g_le_conn_clear;


static uint8_t g_le_conn_max;
static uint8_t g_le_adv_max;
static bt_gap_le_srv_conn_cntx_t *g_le_conn_cntx;
static bt_gap_le_srv_adv_info_t *g_le_adv_info;
static bt_gap_le_srv_handle_table_t *g_handle_table = NULL;

static bt_bd_addr_t g_ble_random_addr;

/* Set Scan API. */
#define BT_GAP_LE_SRV_SCAN_STATE_NONE                  (0x00)
#define BT_GAP_LE_SRV_SCAN_STATE_ENABLING              (0x01)
#define BT_GAP_LE_SRV_SCAN_STATE_ENABLED               (0x02)
#define BT_GAP_LE_SRV_SCAN_STATE_DISABLING             (0x03)
#define BT_GAP_LE_SRV_SCAN_STATE_DISABLED              (0x04)
typedef uint8_t bt_gap_le_srv_scan_state_t;


#define BT_GAP_LE_SRV_SCAN_SM_EVENT_NONE                  (0x00)
#define BT_GAP_LE_SRV_SCAN_SM_EVENT_SATRT                 (0x01)
#define BT_GAP_LE_SRV_SCAN_SM_EVENT_STOP                  (0x02)
#define BT_GAP_LE_SRV_SCAN_SM_EVENT_CMD_CONFIRM           (0x03)
#define BT_GAP_LE_SRV_SCAN_SM_EVENT_CMD_FAIL              (0x04)
#define BT_GAP_LE_SRV_SCAN_SM_EVENT_TIMEOUT               (0x05)
#define BT_GAP_LE_SRV_SCAN_SM_EVENT_CONN_FILTER_CMD_START (0x06)
#define BT_GAP_LE_SRV_SCAN_SM_EVENT_RSL_COMPLETE          (0x07)
#define BT_GAP_LE_SRV_SCAN_SM_EVENT_RHO_COMPLETE          (0x08)
typedef uint8_t bt_gap_le_srv_scan_sm_event_t;

#define BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_CONN_FILTER_CMD             (0x00001)
#define BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_USER            (0x00002)
#define BT_GAP_LE_SRV_SCAN_FLAG_START_AFTER_CONN_FILTER_CMD         (0x00004)
#define BT_GAP_LE_SRV_SCAN_FLAG_RESTART_OF_RHO_COMPLETE (0x00008)

typedef uint16_t bt_gap_le_srv_scan_flag_t;

typedef struct {
    bt_gap_le_srv_set_extended_scan_enable_t       enable;
    bt_gap_le_srv_set_extended_scan_parameters_t   parameter;
    le_ext_scan_item_t                             params_phy_1M;
    le_ext_scan_item_t                             params_phy_coded;
    uint32_t                                       duration_count;
} bt_gap_le_srv_scan_parameter_t;

typedef void (*bt_gap_le_srv_scan_sm_handle_t)(bt_gap_le_srv_scan_sm_event_t event, void *parameter);
static void bt_gap_le_scan_state_none(bt_gap_le_srv_scan_sm_event_t event, void *parameter);
static void bt_gap_le_scan_state_enabling(bt_gap_le_srv_scan_sm_event_t event, void *parameter);
static void bt_gap_le_scan_state_started(bt_gap_le_srv_scan_sm_event_t event, void *parameter);
static void bt_gap_le_scan_state_disabling(bt_gap_le_srv_scan_sm_event_t event, void *parameter);
static void bt_gap_le_scan_state_stopped(bt_gap_le_srv_scan_sm_event_t event, void *parameter);

static const bt_gap_le_srv_scan_sm_handle_t g_le_srv_scan_sm_handle[] = {
    bt_gap_le_scan_state_none,
    bt_gap_le_scan_state_enabling,
    bt_gap_le_scan_state_started,
    bt_gap_le_scan_state_disabling,
    bt_gap_le_scan_state_stopped
};


typedef struct {
    bt_gap_le_srv_event_cb_t         user_callback;
    bt_gap_le_srv_scan_state_t       state;
    bt_gap_le_srv_scan_flag_t        flag;
    bt_gap_le_srv_scan_parameter_t   param;
} bt_gap_le_srv_scan_context_t;

static bt_gap_le_srv_scan_context_t scan_context = {0};
static bt_gap_le_adv_context_t      adv_context = {0};

#define BT_GAP_LE_SRV_BOND_COMPLETE          (0x01)
#define BT_GAP_LE_SRV_BOND_LINK_DISCONNECT   (0x02)
typedef uint8_t bt_gap_le_srv_bond_event_t;
typedef struct {
    bt_gap_le_srv_linknode_t       node;
    bt_handle_t                    connection_handle;
    bt_gap_le_smp_pairing_config_t pairing_config;
} bt_gap_le_srv_bond_node_t;

typedef struct {
    bt_handle_t              bonding_handle;
    bt_gap_le_srv_linknode_t bond_wl_list;
} bt_gap_le_srv_bond_context_t;

static bt_gap_le_srv_bond_context_t le_bond_context = {0};

#define BT_GAP_LE_SRV_CONNECT_FLAG_RESTART            0x0001
typedef uint16_t bt_gap_le_srv_connect_flag_t;

#define BT_GAP_LE_SRV_CONNECT_EVENT_START             0x01
#define BT_GAP_LE_SRV_CONNECT_EVENT_COMPLETE          0x02
#define BT_GAP_LE_SRV_CONNECT_EVENT_CREATE_COMPLETE   0x03
#define BT_GAP_LE_SRV_CONNECT_EVENT_CREATE_FAIL       0x04
#define BT_GAP_LE_SRV_CONNECT_EVENT_CANCEL_REQUEST    0x05
#define BT_GAP_LE_SRV_CONNECT_EVENT_CANCELLED         0x06
#define BT_GAP_LE_SRV_CONNECT_EVENT_CONN_FILTER_CMD_START         0x07
#define BT_GAP_LE_SRV_CONNECT_EVENT_RSL_COMPLETE      0x08
typedef uint8_t bt_gap_le_srv_connect_event_t;

#define BT_GAP_LE_SRV_CONNECT_STATE_NONE               0x00
#define BT_GAP_LE_SRV_CONNECT_STATE_CREATING           0x01
#define BT_GAP_LE_SRV_CONNECT_STATE_CREATED            0x02
#define BT_GAP_LE_SRV_CONNECT_STATE_CANCELING          0x03
typedef uint8_t bt_gap_le_srv_connect_state_t;

typedef struct {
    bt_gap_le_srv_connect_state_t      state;
    bt_gap_le_srv_connect_flag_t       flags;
    bt_gap_le_srv_create_connection_t  wl_connect_parameter;
} bt_gap_le_srv_connect_context_t;

static bt_gap_le_srv_connect_context_t g_connect_context = {0};

static bt_status_t bt_gap_le_srv_connect_state_none_handle(bt_gap_le_srv_connect_context_t *context, bt_gap_le_srv_connect_event_t event, void *parameter);
static bt_status_t bt_gap_le_srv_connect_state_creating_handle(bt_gap_le_srv_connect_context_t *context, bt_gap_le_srv_connect_event_t event, void *parameter);
static bt_status_t bt_gap_le_srv_connect_state_created_handle(bt_gap_le_srv_connect_context_t *context, bt_gap_le_srv_connect_event_t event, void *parameter);
static bt_status_t bt_gap_le_srv_connect_state_canceling_handle(bt_gap_le_srv_connect_context_t *context, bt_gap_le_srv_connect_event_t event, void *parameter);
static bool bt_gap_le_srv_connect_is_busy(void);
static bt_status_t bt_gap_le_srv_connect_event_notify(bt_gap_le_srv_connect_event_t event, void *parameter);
typedef bt_status_t (*bt_gap_le_srv_connect_sm_handle_t)(bt_gap_le_srv_connect_context_t *context, bt_gap_le_srv_connect_event_t event, void *parameter);

static bt_gap_le_srv_connect_sm_handle_t g_connect_sm_handle[] = {
    bt_gap_le_srv_connect_state_none_handle,
    bt_gap_le_srv_connect_state_creating_handle,
    bt_gap_le_srv_connect_state_created_handle,
    bt_gap_le_srv_connect_state_canceling_handle,
};

#define BT_GAP_LE_SRV_FLAG_IS_SET(context, flag)     (context->flags & flag)
#define BT_GAP_LE_SRV_SET_FLAG(context, flag)        (context->flags |= flag)
#define BT_GAP_LE_SRV_REMOVE_FLAG(context, flag)     (context->flags &= ~flag)

/********************************Static Function*****************************/
static void bt_gap_le_srv_disable_ble_cnf(uint8_t result);
static void bt_gap_le_srv_conn_clear_cnf(void);
static void bt_gap_le_srv_adv_clear_cnf(uint8_t result);
static void bt_gap_le_srv_remove_all_stopped_adv(void);
static void bt_gap_le_srv_reset_all_context(void);
static bt_status_t bt_gap_le_srv_stop_all_adv(void);
static void bt_gap_le_srv_disconn_first_connection(bt_gap_le_srv_disable_ble_reason_t reason);
static uint8_t bt_gap_le_srv_get_connected_dev_num_int(void);
#if defined(MTK_AWS_MCE_ENABLE) && defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)
static uint8_t bt_gap_le_srv_get_connected_rho_dev_num_int(void);
static void bt_gap_le_srv_reset_all_context_with_rho(void);
#endif
static bt_status_t bt_gap_le_srv_rho_request_by_handle(bt_handle_t handle);
static bt_gap_le_srv_conn_cntx_t *bt_gap_le_srv_get_conn_cntx_by_handle(const bt_handle_t conn_handle);
static bt_gap_le_srv_error_t bt_gap_le_srv_disable_ble_by_internal(bt_gap_le_srv_disable_ble_reason_t reason, void *callback);
static void  bt_gap_le_srv_adv_run_wl(void);
static void bt_gap_le_srv_reset_adv_waiting_list(void);
static void bt_gap_le_srv_scan_update_state(bt_gap_le_srv_scan_state_t state);
static bt_status_t bt_gap_le_srv_set_scan_by_internel(bool is_enable);
static bt_status_t bt_gap_le_srv_notify_restart_conn_fliter_cmd(void);
static void bt_gap_le_srv_scan_event_notify(bt_gap_le_srv_scan_sm_event_t event, void *parameter);
static bool bt_gap_le_srv_scan_is_busy(void);
static void bt_gap_le_srv_scan_context_init(void);
static bt_gap_le_srv_error_t bt_gap_le_add_adv_waiting_list(bt_gap_le_adv_wl_action_t action, uint8_t instance, \
        bt_bd_addr_ptr_t random_addr, bt_gap_le_srv_adv_time_params_t *time_param, void *adv_data_cb, void *adv_evt_cb);
static bool bt_gap_le_srv_adv_is_busy(void);
static bool bt_gap_le_srv_all_adv_is_removed(void);
static bool bt_gap_le_srv_all_adv_is_stopped(void);
static void bt_gap_le_srv_bond_event_notify(bt_handle_t connection_handle, bt_gap_le_srv_bond_event_t event);
static void bt_gap_le_srv_event_notify_user(bt_gap_le_srv_event_t event, void *parameter);

static void bt_gap_le_srv_run_white_list(void);
static void bt_gap_le_srv_remove_current_white_list(bt_gap_le_srv_white_list_node_t *white_list);
static bt_gap_le_srv_white_list_node_t *bt_gap_le_srv_get_current_white_list(void);

/********************************RHO Function*****************************/
#if defined(MTK_AWS_MCE_ENABLE) && defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)

typedef bt_gap_le_srv_conn_cntx_t bt_gap_le_srv_rho_cntx_t;
BT_PACKED(
typedef struct {
    uint8_t connected_dev_num;
}) bt_gap_le_srv_rho_header_t;
bool g_rho_adv_prepare = false;
bt_gap_le_srv_rho_header_t g_rho_header = {0};

static void bt_gap_le_srv_rho_context_reset(void)
{
    bt_gap_le_srv_memset(&g_rho_header, 0, sizeof(bt_gap_le_srv_rho_header_t));
}

static void bt_gap_le_srv_rho_disable_ble_cnf(void)
{
    if ((false == g_rho_adv_prepare) &&
        (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state())) {
        //notify RHO service to do RHO
        bt_status_t sta = bt_role_handover_reply_prepare_request(BT_ROLE_HANDOVER_MODULE_BLE_SRV);
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] RHO Prepare, all advs stopped done status(0x%04x)", 1, sta);
    }
}

static bt_status_t bt_gap_le_srv_rho_allowed_cb(const bt_bd_addr_t *addr)
{
    uint8_t i;
    bool has_adv = false;
    bt_status_t status = BT_STATUS_SUCCESS;
    for (i = 0; i < g_le_adv_max; i++) {
        if ((g_le_adv_info[i].instance) &&
            (BT_CM_LE_ADV_STATE_TYPE_REMOVING <= g_le_adv_info[i].state)) {
            has_adv = true;
            break;
        }
    }
    /**<need to disconnect all connections and staop all adv.*/
    if (has_adv) {
        g_rho_adv_prepare = true;
        if (bt_gap_le_srv_stop_all_adv() == BT_STATUS_SUCCESS) {
            bt_gap_le_srv_remove_all_stopped_adv();
        }
        status = BT_STATUS_PENDING;
    }
    return status;
}


static uint8_t bt_gap_le_srv_rho_get_data_length_cb(const bt_bd_addr_t *addr)
{
#ifdef AIR_MULTI_POINT_ENABLE
    if (addr != NULL) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] get data length addr != NULL for EMP", 0);
        return 0;
    }
#endif
    if (g_rho_header.connected_dev_num == 0) {
        g_rho_header.connected_dev_num = bt_gap_le_srv_get_connected_rho_dev_num_int();
        if ((g_rho_header.connected_dev_num) && (addr == NULL)) {
            return (sizeof(bt_gap_le_srv_rho_header_t) + (sizeof(bt_gap_le_srv_rho_cntx_t) * g_rho_header.connected_dev_num));
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] RHO get data len error,rho_connected_num = %d", 1, g_rho_header.connected_dev_num);
    return 0;
}

static bt_status_t bt_gap_le_srv_rho_get_data_cb(const bt_bd_addr_t *addr, void *data)
{
#ifdef AIR_MULTI_POINT_ENABLE
    if (addr != NULL) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] get data addr != NULL for EMP", 0);
        return BT_STATUS_FAIL;
    }
#endif
    uint8_t i;
    if (data == NULL) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] RHO get data = NULL", 0);
        return BT_STATUS_FAIL;
    }
    if ((g_rho_header.connected_dev_num) && (addr == NULL)) {
        uint8_t index = 0;
        bt_gap_le_srv_rho_header_t *rho_head = (bt_gap_le_srv_rho_header_t *)data;
        bt_gap_le_srv_rho_cntx_t *rho_cntx = (bt_gap_le_srv_rho_cntx_t *)(rho_head + 1);
        rho_head->connected_dev_num = g_rho_header.connected_dev_num;
        for (i = 0; i < g_le_conn_max; i++) {
            if ((g_le_conn_cntx[i].connection_handle) && (g_le_conn_cntx[i].conn_info.attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NEED_RHO)) {
                bt_gap_le_srv_memcpy((bt_gap_le_srv_rho_cntx_t *)(rho_cntx + index),
                                     (bt_gap_le_srv_conn_cntx_t *)&g_le_conn_cntx[i], sizeof(bt_gap_le_srv_conn_cntx_t));
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] RHO get data,connection handle = %02x", 1, g_le_conn_cntx[i].connection_handle);
                index++;
            }
        }
    }
    return BT_STATUS_SUCCESS;
}

static void bt_gap_le_srv_rho_status_cb(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status)
{
    switch (event) {
        case BT_ROLE_HANDOVER_COMPLETE_IND: {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] RHO complete role = %02x, status = %02x", 2, role, status);
            if ((BT_AWS_MCE_ROLE_AGENT == role) && (BT_STATUS_SUCCESS == status)) {
                bt_gap_le_srv_reset_all_context_with_rho();
            }
            if (scan_context.flag & BT_GAP_LE_SRV_SCAN_FLAG_RESTART_OF_RHO_COMPLETE) {
                scan_context.flag &= ~BT_GAP_LE_SRV_SCAN_FLAG_RESTART_OF_RHO_COMPLETE;
                bt_gap_le_srv_scan_event_notify(BT_GAP_LE_SRV_SCAN_SM_EVENT_RHO_COMPLETE, NULL);
            }
            bt_gap_le_srv_rho_context_reset();
        }
        break;
        default:
            break;
    }
}

static bt_status_t bt_gap_le_srv_rho_update_cb(bt_role_handover_update_info_t *info)
{
    uint8_t j;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] RHO update data", 0);
    if (info && (BT_AWS_MCE_ROLE_PARTNER == info->role)) {
        if ((info->length > 0) && (info->data)) {//copy data to context
            bt_gap_le_srv_rho_header_t *rho_head = (bt_gap_le_srv_rho_header_t *)info->data;
            bt_gap_le_srv_rho_cntx_t *rho_cntx = (bt_gap_le_srv_rho_cntx_t *)(rho_head + 1);
            for (j = 0; j < rho_head->connected_dev_num; j++) {
                bt_gap_le_srv_conn_cntx_t *cntx = bt_gap_le_srv_get_conn_cntx_by_handle(0x00);
                if (cntx != NULL) {
                    bt_gap_le_srv_memcpy(cntx, (bt_gap_le_srv_rho_cntx_t *)(rho_cntx + j), sizeof(bt_gap_le_srv_rho_cntx_t));
                    bt_handle_t new_handle = bt_gap_le_srv_get_handle_by_old_handle(cntx->connection_handle);
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] RHO update old_handle = %02x, new_handle = %02x", 2, cntx->connection_handle, new_handle);
                    cntx->connection_handle = new_handle;
                } else {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] RHO update data,cntx == NULL", 0);
                }
            }
        } else {
            return BT_STATUS_FAIL;
        }
    }
    return BT_STATUS_SUCCESS;
}

bt_role_handover_callbacks_t bt_gap_le_srv_rho_callbacks = {
    .allowed_cb = bt_gap_le_srv_rho_allowed_cb,/*optional if always allowed*/
    .get_len_cb = bt_gap_le_srv_rho_get_data_length_cb,  /*optional if no RHO data to partner*/
    .get_data_cb = bt_gap_le_srv_rho_get_data_cb,   /*optional if no RHO data to partner*/
    .update_cb = bt_gap_le_srv_rho_update_cb,       /*optional if no RHO data to partner*/
    .status_cb = bt_gap_le_srv_rho_status_cb, /*Mandatory for all users.*/
};

#elif defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE) && !defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)
//need to disconnect all connections and stop all advs
bool g_rho_adv_prepare = false;
bool g_rho_conn_prepare = false;

static void bt_gap_le_srv_rho_disable_ble_cnf(void)
{
    if ((false == g_rho_conn_prepare) &&
        (false == g_rho_adv_prepare) &&
        (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state())) {
        //notify RHO service to do RHO
        bt_status_t sta = bt_role_handover_reply_prepare_request(BT_ROLE_HANDOVER_MODULE_BLE_SRV);
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] RHO Prepare, all connections diaconnect done status(0x%04x)", 1, sta);
    }
}

static bt_status_t bt_gap_le_srv_rho_allowed_cb(const bt_bd_addr_t *addr)
{
    uint8_t i;
    bool has_adv = false;
    bt_status_t status = BT_STATUS_SUCCESS;
    for (i = 0; i < g_le_adv_max; i++) {
        if ((g_le_adv_info[i].instance) &&
            (BT_CM_LE_ADV_STATE_TYPE_REMOVING <= g_le_adv_info[i].state)) {
            has_adv = true;
            break;
        }
    }
    /**<need to disconnect all connections and staop all adv.*/
    if (has_adv) {
        g_rho_adv_prepare = true;
        if (bt_gap_le_srv_stop_all_adv() == BT_STATUS_SUCCESS) {
            bt_gap_le_srv_remove_all_stopped_adv();
        }
        status = BT_STATUS_PENDING;
    }
    if (bt_gap_le_srv_get_connected_dev_num_int() > 0) {
        g_rho_conn_prepare = true;
        bt_gap_le_srv_disconn_first_connection(BT_GAP_LE_SRV_DISABLE_BLE_REASON_NONE);
        if (g_disable_ble_info.disconn_num) {
            status = BT_STATUS_PENDING;
        }
    }
    return status;
}
static void bt_gap_le_srv_rho_status_cb(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status)
{
    return;
}
bt_role_handover_callbacks_t bt_gap_le_srv_rho_callbacks = {
    .allowed_cb = bt_gap_le_srv_rho_allowed_cb,/*optional if always allowed*/
    .get_len_cb = NULL,  /*optional if no RHO data to partner*/
    .get_data_cb = NULL,   /*optional if no RHO data to partner*/
    .update_cb = NULL,       /*optional if no RHO data to partner*/
    .status_cb = bt_gap_le_srv_rho_status_cb, /*Mandatory for all users.*/
};
#endif /*__MTK_AWS_MCE_ENABLE__ */

/********************************Internal Function*****************************/
static bt_gap_le_srv_error_t bt_gap_le_srv_map_bt_status(bt_status_t status)
{
    bt_gap_le_srv_error_t err = BT_GAP_LE_SRV_ERROR_FAIL;
    switch (status) {
        case BT_STATUS_SUCCESS:
            err = BT_GAP_LE_SRV_SUCCESS;
            break;
        case BT_STATUS_UNSUPPORTED:
            err = BT_GAP_LE_SRV_ERROR_UNSUPPORTED;
            break;
        case BT_STATUS_FAIL:
            err = BT_GAP_LE_SRV_ERROR_FAIL;
            break;
        case BT_STATUS_OUT_OF_MEMORY:
            err = BT_GAP_LE_SRV_ERROR_NO_MEMORY;
            break;
        case BT_STATUS_BUSY:
            err = BT_GAP_LE_SRV_ERROR_BUSY;
            break;
        default :
            break;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Map err code is %d", 1, err);
    return err;
}

static bt_gap_le_srv_error_t bt_gap_le_srv_adv_set_header(uint8_t type, uint32_t data_len, uint32_t max_len,
        uint8_t *dst, uint32_t *dst_len)
{
    if (*dst_len + 2 + data_len > max_len) {// 2 = length of length + length of AD type
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] exceed the input buffer size", 0);
        return BT_GAP_LE_SRV_ERROR_INVALID_LENTH;//size wrong, exceed the adv max length
    }
    dst[*dst_len] = data_len + 1;
    dst[*dst_len + 1] = type;
    *dst_len += 2;
    return BT_GAP_LE_SRV_SUCCESS;
}

static bt_gap_le_srv_error_t bt_gap_le_srv_adv_set_flat(uint8_t type, uint32_t data_len, const void *data,
        uint8_t *dst, uint32_t *dst_len, uint32_t max_len)
{
    bt_gap_le_srv_error_t err = 0;
    bt_utils_assert(data_len > 0);
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]set flat, ad type is %d", 1, type);
    err = bt_gap_le_srv_adv_set_header(type, data_len, max_len, dst, dst_len);
    if (BT_GAP_LE_SRV_SUCCESS != err) {
        return err;
    }
    memcpy(dst + *dst_len, data, data_len);
    *dst_len += data_len;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]set ad type %d done, len is 0x%4x, len_index is 0x%4x", 3, type, data_len, *dst_len);
    return err;
}

static bt_gap_le_srv_error_t bt_gap_le_srv_adv_set_uuid16(uint8_t type, uint8_t num_elems,
        const bt_gap_le_srv_uuid_t *elems, uint8_t *dst,
        uint32_t *dst_len, uint32_t max_len)
{
    uint8_t i;
    bt_gap_le_srv_error_t err = 0;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]set uuid16, ad type is %d, num is %d", 2, type, num_elems);
    err = bt_gap_le_srv_adv_set_header(type, num_elems * 2, max_len, dst, dst_len);//length of uuid16 is 2
    if (BT_GAP_LE_SRV_SUCCESS != err) {
        return err;
    }
    for (i = 0; i < num_elems; i++) {
        bt_gap_le_srv_u16_to_u8(dst + *dst_len, elems[i].uuid16);
        *dst_len += 2;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]set uuid16 done, len_index is 0x%4x", 1, *dst_len);
    return err;
}

static bt_gap_le_srv_error_t bt_gap_le_srv_adv_set_uuid32(uint8_t type, uint8_t num_elems,
        const bt_gap_le_srv_uuid_t *elems, uint8_t *dst,
        uint32_t *dst_len, uint32_t max_len)
{
    uint8_t i;
    bt_gap_le_srv_error_t err = 0;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]set uuid32, ad type is %d, num is %d", 2, type, num_elems);
    err = bt_gap_le_srv_adv_set_header(type, num_elems * 4, max_len, dst, dst_len);//length of uuid32 is 4
    if (BT_GAP_LE_SRV_SUCCESS != err) {
        return err;
    }
    for (i = 0; i < num_elems; i++) {
        /* In AD, 32-bit UUID shall be written as an actual 32-bit value.*/
        bt_gap_le_srv_u32_to_u8(dst + *dst_len, elems[i].uuid32);
        *dst_len += 4;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]set uuid32 done, len_index is 0x%4x", 1, *dst_len);
    return err;
}

static bt_gap_le_srv_error_t bt_gap_le_srv_adv_set_uuid128(uint8_t type, uint8_t num_elems,
        const bt_gap_le_srv_uuid_t *elems, uint8_t *dst,
        uint32_t *dst_len, uint32_t max_len)
{
    uint8_t i;
    bt_gap_le_srv_error_t err = 0;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]set uuid128, ad type is %d, num is %d", 2, type, num_elems);
    err = bt_gap_le_srv_adv_set_header(type, num_elems * 16, max_len, dst, dst_len);//length of uuid128 is 16
    if (BT_GAP_LE_SRV_SUCCESS != err) {
        return err;
    }
    for (i = 0; i < num_elems; i++) {
        bt_gap_le_srv_memcpy(dst + *dst_len, (const void *)(&elems[i].uuid), 16);
        *dst_len += 16;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]set uuid32 done, len_index is 0x%4x", 1, *dst_len);
    return err;
}

static bt_gap_le_srv_error_t ble_gap_le_srv_generate_data_int(const bt_gap_le_srv_adv_fields_t *adv_fields,
        uint8_t *dst, uint32_t *dst_len, uint32_t max_len)
{
    uint8_t type;
    bt_gap_le_srv_error_t err;
    uint32_t dst_len_local = 0;
    /**< 0x01 - Flags. */
    /* The application has two options concerning the flags field:
        * 1. Don't include it in advertisements (flags == 0).
        * 2. Explicitly specify the value (flags != 0).
        */
    if (adv_fields->flags != 0) {
        err = bt_gap_le_srv_adv_set_flat(BT_GAP_LE_AD_TYPE_FLAG, 1,
                                         &adv_fields->flags, dst, &dst_len_local,
                                         max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /**< 0x02,0x03 - 16-bit service class UUIDs. */
    if ((NULL != adv_fields->uuids16) && adv_fields->num_uuids16) {
        if (adv_fields->uuids16_is_complete) {
            type = BT_GAP_LE_AD_TYPE_16_BIT_UUID_COMPLETE;
        } else {
            type = BT_GAP_LE_AD_TYPE_16_BIT_UUID_PART;
        }
        err = bt_gap_le_srv_adv_set_uuid16(type, adv_fields->num_uuids16,
                                           adv_fields->uuids16, dst, &dst_len_local,
                                           max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x04,0x05 - 32-bit service class UUIDs. */
    if ((NULL != adv_fields->uuids32) && adv_fields->num_uuids32) {
        if (adv_fields->uuids32_is_complete) {
            type = BT_GAP_LE_AD_TYPE_32_BIT_UUID_COMPLETE;
        } else {
            type = BT_GAP_LE_AD_TYPE_32_BIT_UUID_PART;
        }
        err = bt_gap_le_srv_adv_set_uuid32(type, adv_fields->num_uuids32,
                                           adv_fields->uuids32, dst, &dst_len_local,
                                           max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x06,0x07 - 128-bit service class UUIDs. */
    if ((NULL != adv_fields->uuids128) && (adv_fields->num_uuids128 > 0)) {
        if (adv_fields->uuids128_is_complete) {
            type = BT_GAP_LE_AD_TYPE_128_BIT_UUID_COMPLETE;
        } else {
            type = BT_GAP_LE_AD_TYPE_128_BIT_UUID_PART;
        }
        err = bt_gap_le_srv_adv_set_uuid128(type, adv_fields->num_uuids128,
                                            adv_fields->uuids128, dst, &dst_len_local,
                                            max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x08,0x09 - Local name. */
    if ((NULL != adv_fields->name) && (adv_fields->name_len > 0)) {
        if (adv_fields->name_is_complete) {
            type = BT_GAP_LE_AD_TYPE_NAME_COMPLETE;
        } else {
            type = BT_GAP_LE_AD_TYPE_NAME_SHORT;
        }
        err = bt_gap_le_srv_adv_set_flat(type, adv_fields->name_len,
                                         adv_fields->name, dst, &dst_len_local, max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x0a - Tx power level. */
    if (adv_fields->tx_power_level_is_present) {
        /* Read the power level from the controller if requested; otherwise use the explicitly specified value.*/
        err = bt_gap_le_srv_adv_set_flat(BT_GAP_LE_AD_TYPE_TX_POWER, 1,
                                         &adv_fields->tx_power_level, dst, &dst_len_local, max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x12 - Slave connection interval range. */
    if (NULL != adv_fields->slave_conn_interval) {
        err = bt_gap_le_srv_adv_set_flat(BT_GAP_LE_AD_TYPE_SLAVE_CONNECTION_INTERVAL_RANGE,
                                         4,
                                         adv_fields->slave_conn_interval, dst,
                                         &dst_len_local, max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x16 - Service data - 16-bit UUID. */
    if ((NULL != adv_fields->srv_data_uuid16) && adv_fields->srv_data_uuid16_len) {
        err = bt_gap_le_srv_adv_set_flat(BT_GAP_LE_AD_TYPE_16_BIT_UUID_DATA,
                                         adv_fields->srv_data_uuid16_len,
                                         adv_fields->srv_data_uuid16, dst, &dst_len_local,
                                         max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x17 - Public target address. */
    if ((NULL != adv_fields->public_target_addr) &&
        adv_fields->num_public_target_addrs) {
        err = bt_gap_le_srv_adv_set_flat(BT_GAP_LE_AD_TYPE_PUBLIC_TARGET_ADDRESS,
                                         6 * adv_fields->num_public_target_addrs,
                                         adv_fields->public_target_addr, dst, &dst_len_local,
                                         max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x19 - Appearance. */
    if (adv_fields->appearance_is_present) {
        err = bt_gap_le_srv_adv_set_flat(BT_GAP_LE_AD_TYPE_APPEARANCE,
                                         2,
                                         &adv_fields->appearance, dst, &dst_len_local,
                                         max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x1a - Advertising interval. */
    if (adv_fields->adv_interval_is_present) {
        err = bt_gap_le_srv_adv_set_flat(BT_GAP_LE_AD_TYPE_ADV_INTERVAL, 1,
                                         &adv_fields->adv_interval, dst, &dst_len_local,
                                         max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x20 - Service data - 32-bit UUID. */
    if (adv_fields->srv_data_uuid32 != NULL && adv_fields->srv_data_uuid32_len) {
        err = bt_gap_le_srv_adv_set_flat(BT_GAP_LE_AD_TYPE_32_BIT_UUID_DATA,
                                         adv_fields->srv_data_uuid32_len,
                                         adv_fields->srv_data_uuid32, dst, &dst_len_local,
                                         max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0x21 - Service data - 128-bit UUID. */
    if (adv_fields->srv_data_uuid128 != NULL && adv_fields->srv_data_uuid128_len) {
        err = bt_gap_le_srv_adv_set_flat(BT_GAP_LE_AD_TYPE_128_BIT_UUID_DATA,
                                         adv_fields->srv_data_uuid128_len,
                                         adv_fields->srv_data_uuid128, dst,
                                         &dst_len_local, max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    /*** 0xff - Manufacturer specific data. */
    if ((adv_fields->manufacturer_data != NULL) && (adv_fields->manufacturer_data_len >= 2)) {
        err = bt_gap_le_srv_adv_set_flat(BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC,
                                         adv_fields->manufacturer_data_len,
                                         adv_fields->manufacturer_data,
                                         dst, &dst_len_local, max_len);
        if (BT_GAP_LE_SRV_SUCCESS != err) {
            return err;
        }
    }
    if (dst_len) {
        *dst_len = dst_len_local;
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Gen data done, data_p is 0x%4x, data length is 0x%4x", 2, dst, *dst_len);
    }
    return BT_GAP_LE_SRV_SUCCESS;
}

static uint8_t bt_gap_le_srv_find_unused_instance(void)
{
    uint8_t i;
    for (i = 0; i < g_le_adv_max; i++) {
        if (0 == g_le_adv_info[i].instance) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] found unused instance %d ", 1, i + 1);
            return (i + 1);
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Can't find unused instance", 0);
    return 0;
}

static bt_gap_le_srv_adv_info_t *bt_gap_le_srv_find_adv_info_by_instance(const uint8_t instance)
{
    uint8_t i;
    for (i = 0; i < g_le_adv_max; i++) {
        if (instance == g_le_adv_info[i].instance) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Found instance %d adv info, index is %d", 2, instance, i);
            return (bt_gap_le_srv_adv_info_t *)(&g_le_adv_info[i]);
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Can't find instance %d adv info", 1, instance);
    return NULL;
}

static bt_gap_le_srv_conn_cntx_t *bt_gap_le_srv_get_conn_cntx_by_handle(const bt_handle_t conn_handle)
{
    uint8_t i;
    for (i = 0; i < g_le_conn_max; i++) {
        if (conn_handle == g_le_conn_cntx[i].connection_handle) {
            return (bt_gap_le_srv_conn_cntx_t *)(&(g_le_conn_cntx[i]));
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Get conn cntx, conn handle(0x%4x) not be found", 1, conn_handle);
    return NULL;
}

static bt_gap_le_srv_conn_cntx_t *bt_gap_le_srv_get_conn_cntx_by_local_addr(bt_addr_t *addr, bool is_local_addr)
{
    uint8_t i;
    if (is_local_addr) {
        for (i = 0; i < g_le_conn_max; i++) {
            if (0 == bt_gap_le_srv_memcmp(&(g_le_conn_cntx[i].conn_info.local_addr), addr, sizeof(bt_addr_t))) {
                return (bt_gap_le_srv_conn_cntx_t *)(&(g_le_conn_cntx[i]));
            }
        }
    } else {
        for (i = 0; i < g_le_conn_max; i++) {
            if (0 == bt_gap_le_srv_memcmp(&(g_le_conn_cntx[i].conn_info.peer_addr), addr, sizeof(bt_addr_t))) {
                return (bt_gap_le_srv_conn_cntx_t *)(&(g_le_conn_cntx[i]));
            }
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Get conn cntx by addr not be found, is_local_addr %d", 1, is_local_addr);
    return NULL;
}

static uint8_t bt_gap_le_srv_get_connected_dev_num_int(void)
{
    uint8_t i, dev_num = 0;
    for (i = 0; i < g_le_conn_max; i++) {
        if (g_le_conn_cntx[i].connection_handle) {
            dev_num++;
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Get connected num(%d)", 1, dev_num);
    return dev_num;
}

#if defined(MTK_AWS_MCE_ENABLE) && defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)
static uint8_t bt_gap_le_srv_get_connected_rho_dev_num_int(void)
{
    uint8_t i, dev_num = 0;
    for (i = 0; i < g_le_conn_max; i++) {
        if ((g_le_conn_cntx[i].connection_handle) && (g_le_conn_cntx[i].conn_info.attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NEED_RHO)) {
            dev_num++;
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Get connected rho num(%d)", 1, dev_num);
    return dev_num;
}
#endif

static bt_status_t bt_gap_le_srv_connection_duplicate_remove(bt_handle_t connection_handle, bt_addr_t *peer_address)
{
    uint32_t i = 0;
    bt_status_t status = BT_STATUS_FAIL;
    for (i = 0; i < g_le_conn_max; i++) {
        if (bt_gap_le_srv_memcmp(&g_le_conn_cntx[i].conn_info.peer_addr, peer_address, sizeof(bt_addr_t)) == 0) {
            /* Disconnect second LE link. */
            bt_hci_cmd_disconnect_t disconnection = {
                .connection_handle = g_le_conn_cntx[i].connection_handle,
                .reason = BT_HCI_STATUS_ACL_CONNECTION_ALREADY_EXISTS
            };
            status = bt_gap_le_disconnect(&disconnection);
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] connection is already exists, disconnect hanle = %02x, status = %02x", 2, connection_handle, status);
            return status;
        }
    }
    return status;
}

static void bt_gap_le_srv_save_conn_info(bt_gap_le_srv_info_t type, void *info)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_LOG((NULL == info), "[BLE_GAP_SRV][E] conn_ind info is null", 0);
    switch (type) {
        case BT_GAP_LE_SRV_INFO_TYPE_CONNECTION_INFOS: {
            bt_gap_le_connection_ind_t *connection_ind = (bt_gap_le_connection_ind_t *)info;
            bt_gap_le_srv_conn_cntx_t *conn_cntx = NULL;
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Conn_ind conn_handle(0x%04x), conn_interval(0x%x) role = %02x", 3,
                                    connection_ind->connection_handle, connection_ind->conn_interval, connection_ind->role);
            bt_gap_le_srv_connection_duplicate_remove(connection_ind->connection_handle, &connection_ind->peer_addr);
            if (BT_ROLE_MASTER == connection_ind->role) {
                conn_cntx = bt_gap_le_srv_get_conn_cntx_by_handle(0x00);
            } else {
                conn_cntx = bt_gap_le_srv_get_conn_cntx_by_handle(connection_ind->connection_handle);
            }
            if (conn_cntx) {
                conn_cntx->connection_handle = connection_ind->connection_handle;
                conn_cntx->conn_info.role = connection_ind->role;
                bt_gap_le_srv_memcpy(&(conn_cntx->conn_info.peer_addr), &(connection_ind->peer_addr), sizeof(bt_addr_t));
                bt_gap_le_srv_memcpy(&(conn_cntx->conn_info.local_addr), &(connection_ind->local_addr), sizeof(bt_addr_t));
                conn_cntx->conn_params.conn_interval = connection_ind->conn_interval;
                conn_cntx->conn_params.conn_latency = connection_ind->conn_latency;
                conn_cntx->conn_params.supervision_timeout = connection_ind->supervision_timeout;
                bt_gap_le_srv_memcpy(conn_cntx->peer_rpa, connection_ind->peer_resolvable_private_address, sizeof(bt_bd_addr_t));
                /* For legacy api to get atttibute. */
                if (le_srv_context.attribute_callback != NULL) {
                    conn_cntx->conn_info.attribute = le_srv_context.attribute_callback(&conn_cntx->conn_info.local_addr);
                } else {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] attribute callback is NULL", 0);
                }

                bt_gap_le_srv_link_info_t link_info = {
                    .lcoal_address = connection_ind->local_addr,
                    .remote_address = connection_ind->peer_addr,
                    .link_type = BT_GAP_LE_SRV_LINK_TYPE_NONE,
                    .instance = conn_cntx->adv_handle
                };
                bt_gap_le_srv_event_notify_user(BT_GAP_LE_SRV_GET_LINK_INFO, &link_info);

                conn_cntx->conn_info.attribute = link_info.attribute;
                conn_cntx->conn_info.link_type = link_info.link_type;

                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] get le link type = %02x, attribute = %02x by adv handle = %02x", 3, link_info.link_type, link_info.attribute, conn_cntx->adv_handle);

                bt_gap_le_srv_connect_ind_t conn_info = {0};
                conn_info.link_type = link_info.link_type;
                conn_info.connection_handle = connection_ind->connection_handle;
                bt_gap_le_srv_memcpy(&conn_info.peer_address, &connection_ind->peer_addr, sizeof(bt_addr_t));
                bt_gap_le_srv_event_notify_user(BT_GAP_LE_SRV_EVENT_CONNECT_IND, &conn_info);
            } else {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Get available conn info buff fail", 0);
            }
            if (connection_ind->role == BT_ROLE_MASTER) {
                bt_gap_le_srv_connect_event_notify(BT_GAP_LE_SRV_CONNECT_EVENT_COMPLETE, NULL);
            }
        }
        break;
        case BT_GAP_LE_SRV_INFO_TYPE_CONNECTION_PARAMS: {
            bt_gap_le_connection_update_ind_t *connection_param = (bt_gap_le_connection_update_ind_t *)info;
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Conn_update conn_handle(0x%04x), conn_interval(0x%x)", 2,
                                    connection_param->conn_handle, connection_param->conn_interval);
            bt_gap_le_srv_conn_cntx_t *conn_cntx = bt_gap_le_srv_get_conn_cntx_by_handle(connection_param->conn_handle);
            if (conn_cntx) {
                conn_cntx->conn_params.conn_interval = connection_param->conn_interval;
                conn_cntx->conn_params.conn_latency = connection_param->conn_latency;
                conn_cntx->conn_params.supervision_timeout = connection_param->supervision_timeout;
                //notify app update result
                if (conn_cntx->update_cb) {
                    bt_gap_le_srv_event_ind_t evt_ind;
                    evt_ind.conn_update.result = 0;
                    evt_ind.conn_update.conn_handle = connection_param->conn_handle;
                    bt_gap_le_srv_memcpy(&(evt_ind.conn_update.params), &(conn_cntx->conn_params), sizeof(bt_gap_le_srv_conn_params_t));
                    conn_cntx->update_cb(BT_GAP_LE_SRV_EVENT_CONN_UPDATED, &evt_ind);
                }
            } else {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Get available conn params buff fail", 0);
            }
        }
        break;
        default :
            break;
    }
}

static void bt_gap_le_srv_delete_conn_info(void *info)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_LOG((NULL == info), "[BLE_GAP_SRV][E] Disconn_ind info is null", 0);
    bt_gap_le_disconnect_ind_t *disc_ind = (bt_gap_le_disconnect_ind_t *)info;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Disconn_ind conn_handle(0x%04x), reason(0x%2x)", 2,
                            disc_ind->connection_handle, disc_ind->reason);
    bt_gap_le_srv_conn_cntx_t *conn_cntx = bt_gap_le_srv_get_conn_cntx_by_handle(disc_ind->connection_handle);
    bt_gap_le_srv_bond_event_notify(disc_ind->connection_handle, BT_GAP_LE_SRV_BOND_LINK_DISCONNECT);
    if (conn_cntx) {
        bt_gap_le_srv_bond_event_notify(disc_ind->connection_handle, BT_GAP_LE_SRV_BOND_LINK_DISCONNECT);
        bt_gap_le_srv_connect_ind_t disconn_info = {0};
        disconn_info.link_type = conn_cntx->conn_info.link_type;
        disconn_info.connection_handle = disc_ind->connection_handle;
        memcpy(&disconn_info.peer_address, &conn_cntx->conn_info.peer_addr, sizeof(bt_addr_t));
        bt_gap_le_srv_event_notify_user(BT_GAP_LE_SRV_EVENT_DISCONNECT_IND, &disconn_info);
        memset(conn_cntx, 0x0, sizeof(bt_gap_le_srv_conn_cntx_t));
        if (g_disable_ble_info.disconn_num > 0) {
            g_disable_ble_info.disconn_num--;
            bt_gap_le_srv_disconn_first_connection(g_disable_ble_info.reason);
            if (0 == g_disable_ble_info.disconn_num) {// all connections had been disconnected success
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE) && !defined(BT_ROLE_HANDOVER_WITH_SPP_BLE)
                if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
                    if (true == g_rho_conn_prepare) {
                        g_rho_conn_prepare = false;
                        bt_gap_le_srv_rho_disable_ble_cnf();
                    }
                }
#endif
                /**< call callback to notify app.*/
                if (g_disable_ble_info.disconnecting_all) {
                    g_disable_ble_info.disconnecting_all = false;
                    bt_gap_le_srv_disable_ble_cnf(0);
                } else if (g_le_conn_clear.clearing) {
                    g_le_conn_clear.clearing = false;
                    bt_gap_le_srv_conn_clear_cnf();
                }
            } else {
            }
        }
    }
}

static void bt_gap_le_srv_disable_ble_cnf(uint8_t result)
{
    if ((false == g_disable_ble_info.disconnecting_all) &&
        (bt_gap_le_srv_all_adv_is_stopped()) &&
        g_disable_ble_info.app_cb) {
        //notify app the result
        bt_gap_le_srv_event_ind_t evt_ind;
        bt_gap_le_srv_memset(&evt_ind, 0x0, sizeof(bt_gap_le_srv_event_ind_t));
        evt_ind.dis_complete.result = result;
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] disable ble done, result(%d)", 1, result);
        if (g_disable_ble_info.app_cb) {
            g_disable_ble_info.app_cb(BT_GAP_LE_SRV_EVENT_BLE_DISABLED, &evt_ind);
        }
        le_srv_context.flag &= ~BT_GAP_LE_SRV_FLAG_DISABLE_BLE;
        bt_gap_le_srv_memset(&g_disable_ble_info, 0x0, sizeof(bt_gap_le_srv_disable_ble_info_t));
    }
}

static void bt_gap_le_srv_adv_clear_cnf(uint8_t result)
{
    if ((false == g_le_adv_clear.clearing) && (g_le_adv_clear.app_cb)) {
        //notify app the result
        bt_gap_le_srv_event_ind_t evt_ind;
        bt_gap_le_srv_memset(&evt_ind, 0x0, sizeof(bt_gap_le_srv_event_ind_t));
        evt_ind.adv_clear.result = result;
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Clear adv done, result(%d)", 1, result);
        if (g_le_adv_clear.app_cb) {
            g_le_adv_clear.app_cb(BT_GAP_LE_SRV_EVENT_ADV_CLEARED, &evt_ind);
        }
        bt_gap_le_srv_memset(&g_le_adv_clear, 0x0, sizeof(bt_gap_le_srv_adv_clear_info_t));
    }
}

static void bt_gap_le_srv_conn_clear_cnf(void)
{
    if ((false == g_le_conn_clear.clearing) && (g_le_conn_clear.app_cb)) {
        /** notify app the result.*/
        bt_gap_le_srv_event_ind_t evt_ind;
        bt_gap_le_srv_memset(&evt_ind, 0x0, sizeof(bt_gap_le_srv_event_ind_t));
        evt_ind.conn_clear.result = 0;
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Clear all connections done", 0);
        if (g_le_conn_clear.app_cb) {
            g_le_conn_clear.app_cb(BT_GAP_LE_SRV_EVENT_CONN_CLEARED, &evt_ind);
        }
        bt_gap_le_srv_memset(&g_le_conn_clear, 0x0, sizeof(bt_gap_le_srv_conn_clear_info_t));
    }
}

static void bt_gap_le_srv_adv_event_handler(bt_gap_le_srv_adv_info_t *adv_info, uint8_t adv_event, uint8_t result)
{
    bt_gap_le_srv_event_ind_t evt_ind;
    bt_gap_le_srv_memset(&evt_ind, 0x0, sizeof(bt_gap_le_srv_event_ind_t));
    evt_ind.adv_complete.instance = adv_info->instance;
    evt_ind.adv_complete.result = result;
    evt_ind.adv_complete.adv_evt = adv_event;
    if ((adv_event == BT_GAP_LE_SRV_ADV_STARTED) && (0 == result)) {
        evt_ind.adv_complete.selected_tx_power = adv_info->selected_tx_power;
    } else if ((adv_event == BT_CM_LE_ADV_STATE_TYPE_STOPPED) && (0 == result)) {
        evt_ind.adv_complete.conn_handle = adv_info->conn_handle;
        evt_ind.adv_complete.num_ext_adv_events = adv_info->num_ext_adv_events;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Adv event handle, instance(%d), event(%d), result(%d)", 3, adv_info->instance, adv_event, result);
    if (adv_info->evt_cb) {
        adv_info->evt_cb(BT_GAP_LE_SRV_EVENT_ADV_COMPLETE, &evt_ind);
    } else {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Adv instance(%d) evt cb is null", 1, adv_info->instance);
    }
}

static void bt_gap_le_srv_conn_filter_cmd_complete(void)
{
    bt_gap_le_srv_event_ind_t evt_ind = {0};
        evt_ind.adv_complete.result = 0;
        evt_ind.adv_complete.adv_evt = BT_GAP_LE_SRV_ADV_FORCE_RESTART;
        evt_ind.adv_complete.conn_handle = 0;
    for (uint32_t i = 0; i < g_le_adv_max; i++) {
            if (((BT_CM_LE_ADV_STATE_TYPE_REMOVED == g_le_adv_info[i].state) ||
                 (BT_CM_LE_ADV_STATE_TYPE_STOPPED == g_le_adv_info[i].state)) &&
                (BT_CM_LE_ADV_SUB_STATE_FORCE_STOPPING & g_le_adv_info[i].sub_state)) {//need notify
                g_le_adv_info[i].sub_state &= ~BT_CM_LE_ADV_SUB_STATE_FORCE_STOPPING;
                evt_ind.adv_complete.instance = g_le_adv_info[i].instance;
                if (g_le_adv_info[i].evt_cb) {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] RSL updated, notify adv[%d] to force start", 1, g_le_adv_info[i].instance);
                    g_le_adv_info[i].evt_cb(BT_GAP_LE_SRV_EVENT_ADV_COMPLETE, &evt_ind);
                } else {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RSL Update, Adv instance(%d) evt cb is null", 1, g_le_adv_info[i].instance);
                }
            }
        }

        /* Notify scan restart. */
        bt_gap_le_srv_scan_event_notify(BT_GAP_LE_SRV_SCAN_SM_EVENT_RSL_COMPLETE, NULL);
        bt_gap_le_srv_connect_event_notify(BT_GAP_LE_SRV_CONNECT_EVENT_RSL_COMPLETE, NULL);
}

void bt_gap_le_srv_rsl_update_event_handler(void)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] rsl cmd set complete, context flag = %02x", 1, le_srv_context.flag);
    if (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL) {
        if (!(le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST)) {
            bt_gap_le_srv_conn_filter_cmd_complete();
        }
        le_srv_context.flag &= ~BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL;
    } else {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RSL Update done, but no user is listening the cb", 0);
    }
}

static void bt_gap_le_srv_set_white_list_complete(void)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] white list cmd set complete, context flag = %02x", 1, le_srv_context.flag);
    if (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST) {
        if (!(le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL)) {
            bt_gap_le_srv_conn_filter_cmd_complete();
        }
        le_srv_context.flag &= ~BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST;
    } else {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] set white list done, but no user is listening the cb", 0);
    }
}

static bt_status_t bt_gap_le_srv_gap_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_GAP_LE_CONNECTION_UPDATE_CNF: {
            if (0 != status) {
                //notify app update fail
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Update Cnf fail, status is 0x%4x", 1, status);
            }
        }
        break;
        case BT_GAP_LE_CONNECTION_UPDATE_IND: {
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_srv_save_conn_info(BT_GAP_LE_SRV_INFO_TYPE_CONNECTION_PARAMS, buff);
            } else {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] connection update fail status is 0x%4x", 1, status);
            }
        }
        break;
        case BT_GAP_LE_CONNECT_CANCEL_CNF: {
            bt_gap_le_srv_connect_event_notify(BT_GAP_LE_SRV_CONNECT_EVENT_CANCELLED, NULL);
        }
        break;
        case BT_GAP_LE_SET_RANDOM_ADDRESS_CNF:
            break;
        case BT_GAP_LE_SET_ADVERTISING_CNF:
            break;
        case BT_GAP_LE_ADVERTISING_REPORT_IND:
            break;
        case BT_GAP_LE_CONNECT_CNF: {
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_srv_connect_event_notify(BT_GAP_LE_SRV_CONNECT_EVENT_CREATE_COMPLETE, NULL);
            } else {
                bt_gap_le_srv_connect_event_notify(BT_GAP_LE_SRV_CONNECT_EVENT_CREATE_FAIL, NULL);
            }
        }
        break;
        case BT_GAP_LE_CONNECT_IND:
        case BT_GAP_LE_ULL_PSEUDO_LINK_CONNECT_IND:
            bt_gap_le_srv_save_conn_info(BT_GAP_LE_SRV_INFO_TYPE_CONNECTION_INFOS, buff);
            break;
        case BT_GAP_LE_DISCONNECT_IND:
            bt_gap_le_srv_delete_conn_info(buff);
            break;
        case BT_GAP_LE_CONFIG_EXTENDED_ADVERTISING_CNF: {
            bt_gap_le_config_extended_advertising_cnf_t *cnf = (bt_gap_le_config_extended_advertising_cnf_t *)buff;
            if (cnf) {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Config ADV CNF instance(0x%02x), selected_tx_power is 0x%02x", 2,
                                        cnf->handle, cnf->selected_tx_power);
                //find the cntx by instance
                bt_gap_le_srv_adv_info_t *adv_info = bt_gap_le_srv_find_adv_info_by_instance(cnf->handle);
                if (NULL == adv_info) {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Config extend adv cnf, but can't find the instance %d", 1, cnf->handle);
                    break;
                }
                //normal case
                if (BT_CM_LE_ADV_STATE_TYPE_STARTING == adv_info->state) {//waiting for be resue
                    if (BT_STATUS_SUCCESS != status) {
                        bt_gap_le_srv_adv_event_handler(adv_info, BT_GAP_LE_SRV_ADV_STARTED, BT_GAP_LE_SRV_ERROR_FAIL);
                        bt_gap_le_srv_adv_run_wl();
                        if (false == adv_info->configured) {//first config and start, clear the buffer
                            bt_gap_le_srv_memset(adv_info, 0x0, sizeof(bt_gap_le_srv_adv_info_t));
                        }
                        break;
                    } else {
                        adv_info->configured = true;
                        adv_info->selected_tx_power = cnf->selected_tx_power;
                        if (BT_CM_LE_ADV_SUB_STATE_CONFIGURING & adv_info->sub_state) {//start command have received
                            bt_status_t sta;
                            bt_hci_le_set_ext_advertising_enable_t ext_adv_enable;
                            ext_adv_enable.enable = adv_info->ext_enable.enable;
                            ext_adv_enable.duration = adv_info->ext_enable.duration;
                            ext_adv_enable.max_ext_advertising_evts = adv_info->ext_enable.max_ext_advertising_evts;
                            adv_info->sub_state &= ~BT_CM_LE_ADV_SUB_STATE_CONFIGURING;
                            sta = bt_gap_le_enable_extended_advertising(cnf->handle, &ext_adv_enable);
                            if (BT_STATUS_SUCCESS != sta) {//app can restart, now the adv is configured
                                adv_info->state = BT_CM_LE_ADV_STATE_TYPE_STOPPED;
                                bt_gap_le_srv_adv_run_wl();
                                bt_gap_le_srv_adv_event_handler(adv_info, BT_GAP_LE_SRV_ADV_STARTED, BT_GAP_LE_SRV_ERROR_FAIL);
                            } else if (ext_adv_enable.enable == BT_HCI_ENABLE) {
                            }
                        }
                    }
                } else if (((BT_CM_LE_ADV_STATE_TYPE_STOPPED == adv_info->state) ||
                            (BT_CM_LE_ADV_STATE_TYPE_STARTED == adv_info->state)) &&
                           (BT_CM_LE_ADV_SUB_STATE_UPDATING & adv_info->sub_state)) {//update when adv stopped/started
                    uint8_t result = (status == BT_STATUS_SUCCESS) ? BT_GAP_LE_SRV_SUCCESS : BT_GAP_LE_SRV_ERROR_FAIL;
                    adv_info->sub_state &= ~BT_CM_LE_ADV_SUB_STATE_UPDATING;
                    bt_gap_le_srv_adv_run_wl();
                    bt_gap_le_srv_adv_event_handler(adv_info, BT_GAP_LE_SRV_ADV_UPDATED, result);
                }
            } else {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Config extend advertising cnf is null", 0);
            }
        }
        break;
        case BT_GAP_LE_ENABLE_EXTENDED_ADVERTISING_CNF: {
            bt_gap_le_enable_extended_advertising_cnf_t *cnf = (bt_gap_le_enable_extended_advertising_cnf_t *)buff;
            if (cnf) {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] enalbe extend adv handle: 0x%02X, enable: %d,status: %02x", 3, cnf->handle, cnf->enable, status);
                //find the cntx by instance
                bt_gap_le_srv_adv_info_t *adv_info = bt_gap_le_srv_find_adv_info_by_instance(cnf->handle);
                if (NULL == adv_info) {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Enable extend adv cnf, but can't find the instance %d", 1, cnf->handle);
                    break;
                } else if (cnf->enable) {//waiting for be resue
                    //notify app start result
                    if (BT_CM_LE_ADV_STATE_TYPE_STARTING == adv_info->state) {
                        uint8_t result = (status == BT_STATUS_SUCCESS) ? BT_GAP_LE_SRV_SUCCESS : BT_GAP_LE_SRV_ERROR_FAIL;
                        if (status == BT_HCI_STATUS_CONNECTION_LIMIT_EXCEEDED) {
                            result = BT_GAP_LE_SRV_ERROR_CONN_LIMIT_EXCEEDED;
                        }
                        adv_info->state = (status == BT_STATUS_SUCCESS) ? BT_CM_LE_ADV_STATE_TYPE_STARTED : BT_CM_LE_ADV_STATE_TYPE_STOPPED;
                        bt_gap_le_srv_adv_event_handler(adv_info, BT_GAP_LE_SRV_ADV_STARTED, result);
                    } else {
                        adv_info->state = (status == BT_STATUS_SUCCESS) ? BT_CM_LE_ADV_STATE_TYPE_STARTED : BT_CM_LE_ADV_STATE_TYPE_STOPPED;
                    }
                    bt_gap_le_srv_adv_run_wl();
                } else if ((BT_HCI_DISABLE == cnf->enable) && (BT_CM_LE_ADV_STATE_TYPE_STOPPING == adv_info->state)) {
                    adv_info->state = (status == BT_STATUS_SUCCESS) ? BT_CM_LE_ADV_STATE_TYPE_STOPPED : BT_CM_LE_ADV_STATE_TYPE_STARTED;
                    if (adv_context.flag & BT_GAP_LE_SRV_ADV_FLAG_STOPPING_ALL) {
                        if (bt_gap_le_srv_stop_all_adv() == BT_STATUS_SUCCESS) {
                            if ((le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL) || (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST)) {
                                bt_status_t sta = bt_gap_le_srv_notify_restart_conn_fliter_cmd();
                                UNUSED(sta);
                                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] update RSL status 0x%04X", 1, sta);
                                if (!(le_srv_context.flag & BT_GAP_LE_SRV_FLAG_DISABLE_BLE)) {
                                    break;
                                } else {
                                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] disable ble for stopping adv", 0);
                                }
                            }
                            bt_gap_le_srv_remove_all_stopped_adv();
                            break;
                        }
                    } else {
                        /* Notify app stop result. */
                        uint8_t result = (status == BT_STATUS_SUCCESS) ? BT_GAP_LE_SRV_SUCCESS : BT_GAP_LE_SRV_ERROR_FAIL;
                        bt_gap_le_srv_adv_run_wl();
                        adv_info->conn_handle = 0;
                        bt_gap_le_srv_adv_event_handler(adv_info, BT_GAP_LE_SRV_ADV_STOPPED, result);
                    }
                } else if ((BT_HCI_DISABLE == cnf->enable) && (BT_CM_LE_ADV_STATE_TYPE_STARTED == adv_info->state)) {
                    if (BT_CM_LE_ADV_SUB_STATE_REMOVING == adv_info->sub_state) {
                        adv_info->state = (status == BT_STATUS_SUCCESS) ? BT_CM_LE_ADV_STATE_TYPE_STOPPED : BT_CM_LE_ADV_STATE_TYPE_STARTED;
                        adv_info->sub_state &= ~BT_CM_LE_ADV_SUB_STATE_REMOVING;
                        bt_gap_le_srv_adv_run_wl();
                        if (BT_CM_LE_ADV_STATE_TYPE_STOPPED == adv_info->state) {
                            //Next: remove the adv
                            bt_status_t sta = bt_gap_le_remove_extended_advertising(cnf->handle, BT_HCI_DISABLE);
                            if (BT_STATUS_SUCCESS != sta) {
                                bt_gap_le_srv_adv_event_handler(adv_info, BT_GAP_LE_SRV_ADV_REMOVED, BT_GAP_LE_SRV_ERROR_FAIL);
                            } else {
                                adv_info->state = BT_CM_LE_ADV_STATE_TYPE_REMOVING;
                            }
                        }
                    }
                }
            } else {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Enable extend advertising cnf is null", 0);
            }
        }
        break;
        case BT_GAP_LE_REMOVE_ADVERTISING_CNF: {
            bt_gap_le_remove_advertising_cnf_t *cnf = (bt_gap_le_remove_advertising_cnf_t *)buff;
            if (cnf) {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Remove ADV CNF instance(0x%02x), cleard All flag is %d", 2, cnf->handle, cnf->clear_all);
                //find the cntx by instance
                bt_gap_le_srv_adv_info_t *adv_info = bt_gap_le_srv_find_adv_info_by_instance(cnf->handle);
                if (BT_HCI_ENABLE == cnf->clear_all) {//clear all success
                    if (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_DISABLE_BLE) {
                        uint8_t result = (status == BT_STATUS_SUCCESS) ? BT_GAP_LE_SRV_SUCCESS : BT_GAP_LE_SRV_ERROR_FAIL;
                        //notify callback to app
                        bt_gap_le_srv_disable_ble_cnf(result);
                    } else if (g_le_adv_clear.clearing) {
                        uint8_t result = (status == BT_STATUS_SUCCESS) ? BT_GAP_LE_SRV_SUCCESS : BT_GAP_LE_SRV_ERROR_FAIL;
                        g_le_adv_clear.clearing = false;
                        bt_gap_le_srv_adv_clear_cnf(result);
                    }
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
                    else if (true == g_rho_adv_prepare) {
                        if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
                            g_rho_adv_prepare = false;
                            bt_gap_le_srv_rho_disable_ble_cnf();
                            break;
                        }
                    }
#endif
                    if (BT_STATUS_SUCCESS == status) {
                        uint8_t i;
                        //notify callback to all app
                        for (i = 0; i < g_le_adv_max; i++) {
                            if (g_le_adv_info[i].instance) {
                                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Cleared instance %d adv info", 1, g_le_adv_info[i].instance);
                                bt_gap_le_srv_adv_event_handler(&g_le_adv_info[i], BT_GAP_LE_SRV_ADV_REMOVED, BT_STATUS_SUCCESS);
                            }
                        }
                        bt_gap_le_srv_memset(g_le_adv_info, 0x0, (g_le_adv_max * sizeof(bt_gap_le_srv_adv_info_t)));
                    }
                    break;
                }
                if (NULL == adv_info) {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Remove extend adv cnf, but can't find the instance %d", 1, cnf->handle);
                    break;
                }
                if (BT_CM_LE_ADV_STATE_TYPE_REMOVING == adv_info->state) {
                    uint8_t result = (status == BT_STATUS_SUCCESS) ? BT_GAP_LE_SRV_SUCCESS : BT_GAP_LE_SRV_ERROR_FAIL;
                    adv_info->state = (status == BT_STATUS_SUCCESS) ? BT_CM_LE_ADV_STATE_TYPE_REMOVED : BT_CM_LE_ADV_STATE_TYPE_STOPPED;
                    bt_gap_le_srv_adv_event_handler(adv_info, BT_GAP_LE_SRV_ADV_REMOVED, BT_GAP_LE_SRV_ERROR_FAIL);
                    if (BT_GAP_LE_SRV_SUCCESS == result) {
                        bt_gap_le_srv_memset(adv_info, 0x0, sizeof(bt_gap_le_srv_adv_info_t));
                    }
                    bt_gap_le_srv_adv_run_wl();
                }
            } else {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Remove extend advertising cnf is null", 0);
            }
        }
        break;
        case BT_GAP_LE_ADVERTISING_SET_TERMINATED_IND: {
            bt_gap_le_advertising_set_terminated_ind_t *tm_ind = (bt_gap_le_advertising_set_terminated_ind_t *)buff;
            if (tm_ind) {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Terminated ADV instance(0x%02x), conn_handle(0x%2x), num_comp_adv_evt(%d)",
                                        3, tm_ind->handle, tm_ind->connection_handle, tm_ind->num_completed_extended_advertising_events);
                //find the cntx by instance
                bt_gap_le_srv_adv_info_t *adv_info = bt_gap_le_srv_find_adv_info_by_instance(tm_ind->handle);
                if (NULL == adv_info) {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Terminated extend adv ind, but can't find the instance %d", 1, tm_ind->handle);
                    break;
                }
                bt_gap_le_srv_conn_cntx_t *conn_context = bt_gap_le_srv_get_conn_cntx_by_handle(0x00);
                if (NULL != conn_context) {
                    conn_context->connection_handle = tm_ind->connection_handle;
                    conn_context->adv_handle = tm_ind->handle;
                }
                if ((BT_CM_LE_ADV_STATE_TYPE_STARTED == adv_info->state) && ((BT_STATUS_SUCCESS == status) || (BT_HCI_STATUS_CONNECTION_LIMIT_EXCEEDED == status))) {
                    uint8_t result = (status == BT_GAP_LE_SRV_SUCCESS) ? BT_GAP_LE_SRV_SUCCESS : BT_GAP_LE_SRV_ERROR_CONN_LIMIT_EXCEEDED;
                    adv_info->state = BT_CM_LE_ADV_STATE_TYPE_STOPPED;
                    adv_info->conn_handle = tm_ind->connection_handle;
                    adv_info->num_ext_adv_events = tm_ind->num_completed_extended_advertising_events;
                    bt_gap_le_srv_adv_event_handler(adv_info, BT_GAP_LE_SRV_ADV_STOPPED, result);
                    break;
                }
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Terminated extend adv state error, state is %d,status = %02x", 2, adv_info->state, status);
            } else {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Terminated extend advertising ind is null", 0);
            }
        }
        break;
        case BT_GAP_LE_HANDLE_UPDATE: {
            /* update connection handle after RHO. */
            bt_gap_le_handle_update_t *update = (bt_gap_le_handle_update_t *)buff;
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] GAP update LE connection handle,num = %d", 1, update->num);
            uint8_t i = 0;
            bt_gap_le_srv_memset(g_handle_table, 0, g_le_conn_max * sizeof(bt_gap_le_srv_handle_table_t));
            for (i = 0; i < update->num; i++) {
                g_handle_table[i].new_conn_handle = update->new_handles[i];
                g_handle_table[i].old_conn_handle = update->old_handles[i];
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] GAP update LE connection index = %d, old handle = %02x,new handle = %02x", 3, \
                                        i,  g_handle_table[i].old_conn_handle, g_handle_table[i].new_conn_handle);
            }
        }
        break;
        case BT_GAP_LE_SET_EXTENDED_SCAN_CNF: {
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_srv_scan_event_notify(BT_GAP_LE_SRV_SCAN_SM_EVENT_CMD_CONFIRM, NULL);
            } else {
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
                if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
                    /* RHO is ongoing, wait rho complete. */
                    scan_context.flag |= BT_GAP_LE_SRV_SCAN_FLAG_RESTART_OF_RHO_COMPLETE;
                }
#endif 
                bt_gap_le_srv_scan_event_notify(BT_GAP_LE_SRV_SCAN_SM_EVENT_CMD_FAIL, NULL);
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan confirmation status = %02x", 1, status);
            }
        }
        break;
        case BT_GAP_LE_SCAN_TIMEOUT_IND: {
            bt_gap_le_srv_scan_event_notify(BT_GAP_LE_SRV_SCAN_SM_EVENT_TIMEOUT, NULL);
        }
        break;
        case BT_GAP_LE_BONDING_COMPLETE_IND: {
            bt_gap_le_bonding_complete_ind_t *ind = (bt_gap_le_bonding_complete_ind_t *)buff;
            bt_gap_le_srv_bond_event_notify(ind->handle, BT_GAP_LE_SRV_BOND_COMPLETE);
        }
        break;
        case BT_GAP_LE_SET_WHITE_LIST_CNF: {
            bt_gap_le_srv_white_list_node_t *white_list = bt_gap_le_srv_get_current_white_list();
            if (white_list != NULL) {
                bt_gap_le_srv_operate_white_list_complete_t white_list_complete = {
                    .result = status,
                    .op = white_list->op,
                    .remote_addr = white_list->address
                };

                if (NULL != white_list->callback) {
                    white_list->callback(BT_GAP_LE_SRV_EVENT_OPERATE_WHITE_LIST_COMPLETE, (void *)&white_list_complete);
                } else {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][WHITE] confirm callback is NULL", 0);
                }
                bt_gap_le_srv_remove_current_white_list(white_list);
            }

            if (bt_gap_le_srv_get_current_white_list() == NULL) {
                bt_gap_le_srv_set_white_list_complete();
            }

        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static void bt_gap_le_srv_store_local_random_address(bt_bd_addr_t *addr)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_LOG((NULL == addr), "[BLE_GAP_SRV][E] Random address is NULL", 0);
    memcpy(&g_ble_random_addr, addr, sizeof(bt_bd_addr_t));
#ifdef AIR_REPLACE_NVDM_WITH_NVKEY
    int8_t i;
    uint8_t buffer[18] = {0};
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    /* save address to NVDM */
    for (i = 0; i < 6; ++i) {
        snprintf((char *)buffer + 2 * i, 3, "%02X", (*addr)[i]);
    }
    status = nvkey_write_data(NVID_BT_HOST_LE_SRV_RANDOM_ADDRESS, buffer, strlen((char *)buffer));
    if (NVKEY_STATUS_OK != status) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Write random Addr to NVDM fail", 0);
    } else {
        bt_gap_le_srv_report_id("[BT]Successfully store address to NVDM [%02X:%02X:%02X:%02X:%02X:%02X]\r\n", 6, addr[0],
                                (*addr)[1], (*addr)[2], (*addr)[3], (*addr)[4], (*addr)[5]);
    }
#endif
}

static void bt_gap_le_srv_generate_random_address(bt_bd_addr_t addr)
{
    int8_t i;
    bt_bd_addr_t tempaddr = {0};
    if (bt_gap_le_srv_memcmp(addr, tempaddr, sizeof(bt_bd_addr_t)) == 0) {
#ifdef AIR_REPLACE_NVDM_WITH_NVKEY
        uint32_t size = 12;
        uint8_t buffer[18] = {0};
        uint8_t tmp_buf[3] = {0};
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Try to read random addr from NVDM", 0);
        nvkey_status_t status = nvkey_read_data(NVID_BT_HOST_LE_SRV_RANDOM_ADDRESS, buffer, &size);
        if (NVKEY_STATUS_OK == status) {
            for (i = 0; i < 6; ++i) {
                tmp_buf[0] = buffer[2 * i];
                tmp_buf[1] = buffer[2 * i + 1];
                addr[i] = (uint8_t)strtoul((char *)tmp_buf, NULL, 16);
            }
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Read address from NVDM [%02X:%02X:%02X:%02X:%02X:%02X]", 6, addr[0],
                                    addr[1], addr[2], addr[3], addr[4], addr[5]);
            return;
        } else
#endif
        {
            uint32_t random_seed;
#ifdef HAL_TRNG_MODULE_ENABLED
            hal_trng_status_t ret;
            BT_GAP_LE_SRV_CHECK_RET_WITH_LOG((HAL_TRNG_STATUS_OK != hal_trng_init()), "[BLE_GAP_SRV][E] hal trng init fail", 0);
            for (i = 0; i < 30; ++i) {
                ret = hal_trng_get_generated_random_number(&random_seed);
                if (HAL_TRNG_STATUS_OK != ret) {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] generate random seed fail", 0);
                }
            }
            /* randomly generate address */
            ret = hal_trng_get_generated_random_number(&random_seed);
            if (HAL_TRNG_STATUS_OK != ret) {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] generate random seed fail", 0);
            }
            addr[0] = random_seed & 0xFF;
            addr[1] = (random_seed >> 8) & 0xFF;
            addr[2] = (random_seed >> 16) & 0xFF;
            addr[3] = (random_seed >> 24) & 0xFF;
            ret = hal_trng_get_generated_random_number(&random_seed);
            if (HAL_TRNG_STATUS_OK != ret) {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] generate random seed fail", 0);
            }
            addr[4] = random_seed & 0xFF;
            addr[5] = (random_seed >> 8) & 0xCF;
            addr[5] = addr[5] | 0xC0;
            hal_trng_deinit();
#else
#include "hal_gpt.h"
            uint32_t seed = 0;
            hal_gpt_status_t gpt_ret = hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &seed);
            if (gpt_ret == HAL_GPT_STATUS_OK) {
                srand(seed);
                addr[0] = rand() & 0xFF;
                addr[1] = rand() & 0xFF;
                addr[2] = rand() & 0xFF;
                addr[3] = rand() & 0xFF;
                addr[4] = rand() & 0xFF;
                addr[5] = rand() & 0xFF;
            } else {
                addr[0] = 0x66;
                addr[1] = 0x77;
                addr[2] = 0xE2;
                addr[3] = 0xE1;
                addr[4] = 0x90;
                addr[5] = 0x00;
            }
#endif
        }
    }
    bt_gap_le_srv_store_local_random_address((bt_bd_addr_t *)addr);
}

static bool bt_gap_le_srv_validate_conn_params(bt_gap_le_srv_conn_params_t *params)
{
    if ((NULL == params) || (params->conn_latency > 0x01F3)) {
        return false;
    }
    if (params->conn_interval < 0x0006 || params->conn_interval > 0x0C80) {
        return false;
    }
    /**< According to specification mentioned above we should make sure that:
        * supervision_timeout_ms > (1 + latency) * 2 * max_interval_ms
        * supervision_timeout * 10 ms > (1 + latency) * 2 * itvl_max * 1.25ms
        */
    if (params->supervision_timeout <=
        (((1 + params->conn_latency) * params->conn_interval) / 4)) {
        return false;
    }
    return true;
}

static void bt_gap_le_srv_disconn_first_connection(bt_gap_le_srv_disable_ble_reason_t reason)
{
    uint8_t i;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Try disconnect all connections, reason = %02x", 1, reason);
    for (i = 0; i < g_le_conn_max ; i++) {
        if ((g_le_conn_cntx[i].connection_handle) && ((((reason == BT_GAP_LE_SRV_DISABLE_BLE_REASON_ROLE_RECOVERY) &&
                                                        (g_le_conn_cntx[i].conn_info.attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NEED_RHO))) || (reason != BT_GAP_LE_SRV_DISABLE_BLE_REASON_ROLE_RECOVERY))) {
            bt_hci_cmd_disconnect_t disconnect_para = {
                .connection_handle = g_le_conn_cntx[i].connection_handle,
                .reason = BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION,
            };

            if (reason == BT_GAP_LE_SRV_DISABLE_BLE_REASON_POWER_OFF) {
                disconnect_para.reason = BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_POWER_OFF;
            }
            bt_status_t status = bt_gap_le_disconnect(&disconnect_para);
            if ((BT_STATUS_SUCCESS == status) || (BT_STATUS_PENDING == status)) {
                g_disable_ble_info.disconn_num++;
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Disconnecting connections(0x%4x), status = %02x", 2,
                                        g_le_conn_cntx[i].connection_handle, status);
                break;//disconnect one by one
            } else {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Disconnect connections(0x%4x), err status is (0x%4x)", 2,
                                        g_le_conn_cntx[i].connection_handle, status);
            }
        } else {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] not need disconnect ble connection handle = %02x,attribute = %02x", g_le_conn_cntx[i].connection_handle, g_le_conn_cntx[i].conn_info.attribute);
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] %d connections are disconnecting", 1, g_disable_ble_info.disconn_num);
}

static void bt_gap_le_srv_remove_all_stopped_adv(void)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Try to remove all adv", 0);
    bt_status_t sta = bt_gap_le_remove_extended_advertising(0, BT_HCI_ENABLE);//clear all
    BT_GAP_LE_SRV_CHECK_RET_WITH_LOG((0 != sta), "[BLE_GAP_SRV][E] Remove all adv error(0x%4x)", 1, sta);
}


static bt_status_t bt_gap_le_srv_stop_all_adv(void)
{
    uint32_t i;
    uint32_t stopping_adv_num = 0;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Try stop all adv", 0);
    if (bt_gap_le_srv_adv_is_busy()) {
        bt_gap_le_add_adv_waiting_list(BT_GAP_LE_ADV_WL_ACTION_STOP_ALL, 0, NULL, NULL, NULL, NULL);
        return BT_STATUS_PENDING;
    }
    for (i = 0; i < g_le_adv_max; i++) {
        if ((g_le_adv_info[i].instance) &&
            (BT_CM_LE_ADV_STATE_TYPE_STARTED == g_le_adv_info[i].state)) {
            bt_hci_le_set_ext_advertising_enable_t enable = {BT_HCI_DISABLE, 0, 0};
            bt_status_t sta = bt_gap_le_enable_extended_advertising(g_le_adv_info[i].instance,
                              (const bt_hci_le_set_ext_advertising_enable_t *)&enable);
            if (BT_STATUS_SUCCESS != sta) {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Stop adv instance(%d) fail, err status is (0x%4x)", 2,
                                        g_le_adv_info[i].instance, sta);
            } else {
                g_le_adv_info[i].state = BT_CM_LE_ADV_STATE_TYPE_STOPPING;
                stopping_adv_num++;
                adv_context.flag |= BT_GAP_LE_SRV_ADV_FLAG_STOPPING_ALL;
                break;
            }
        }
    }
    if (stopping_adv_num > 0) {
        if ((le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL) || (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST)) {
            g_le_adv_info[i].sub_state |= BT_CM_LE_ADV_SUB_STATE_FORCE_STOPPING;
        }
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] %d advs are stopping", 1, stopping_adv_num);
        return BT_STATUS_BUSY;
    } else {
        adv_context.flag &= ~BT_GAP_LE_SRV_ADV_FLAG_STOPPING_ALL;
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] ALL advs are stopped", 0);
    }
    return BT_STATUS_SUCCESS;
}

static void bt_gap_le_srv_reset_all_context_int(void)
{
    /**< Advertisng will stopped when power off happened.*/
    bt_gap_le_srv_memset(g_le_adv_info, 0x0, (g_le_adv_max * sizeof(bt_gap_le_srv_adv_info_t)));
    bt_gap_le_srv_memset(&g_disable_ble_info, 0x0, sizeof(bt_gap_le_srv_disable_ble_info_t));
    bt_gap_le_srv_memset(&g_le_adv_clear, 0x0, sizeof(bt_gap_le_srv_adv_clear_info_t));
    bt_gap_le_srv_memset(&g_le_conn_clear, 0x0, sizeof(bt_gap_le_srv_conn_clear_info_t));
    bt_gap_le_srv_memset(&g_connect_context, 0x0, sizeof(bt_gap_le_srv_connect_context_t));
    bt_gap_le_srv_scan_context_init();
#if defined(MTK_AWS_MCE_ENABLE) && defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)
    g_rho_header.connected_dev_num = 0;
    g_rho_adv_prepare = false;
#elif defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE) && !defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)
    g_rho_adv_prepare = false;
    g_rho_conn_prepare = false;
#endif
#ifdef BT_DM_POWER
    bt_gap_le_srv_memset(&g_ble_srv_power_cntx, 0x0, sizeof(bt_gap_le_srv_power_cntx_t));
#endif
}

static void bt_gap_le_srv_reset_all_context(void)
{
    /**< LE connection should be destroyed when BT power off.*/
    bt_gap_le_srv_memset(g_le_conn_cntx, 0x0, (g_le_conn_max * sizeof(bt_gap_le_srv_conn_cntx_t)));
    bt_gap_le_srv_reset_all_context_int();
    /* clear adv waiting list. */
    bt_gap_le_srv_reset_adv_waiting_list();
}

#if defined(MTK_AWS_MCE_ENABLE) && defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)
static void bt_gap_le_srv_reset_all_context_with_rho(void)
{
    uint8_t i = 0;
    for (i = 0; i < g_le_conn_max; i++) {
        if (g_le_conn_cntx[i].conn_info.attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NEED_RHO) {
            bt_gap_le_srv_memset(&g_le_conn_cntx[i], 0x0, sizeof(bt_gap_le_srv_conn_cntx_t));
        }
    }
    /**< Advertisng will stopped when power off happened.*/
    bt_gap_le_srv_memset(g_le_adv_info, 0x0, (g_le_adv_max * sizeof(bt_gap_le_srv_adv_info_t)));
    bt_gap_le_srv_memset(&g_disable_ble_info, 0x0, sizeof(bt_gap_le_srv_disable_ble_info_t));
    bt_gap_le_srv_memset(&g_le_adv_clear, 0x0, sizeof(bt_gap_le_srv_adv_clear_info_t));
    bt_gap_le_srv_memset(&g_le_conn_clear, 0x0, sizeof(bt_gap_le_srv_conn_clear_info_t));
#if defined(MTK_AWS_MCE_ENABLE) && defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)
    g_rho_header.connected_dev_num = 0;
    g_rho_adv_prepare = false;
#elif defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE) && !defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)
    g_rho_adv_prepare = false;
    g_rho_conn_prepare = false;
#endif
}
#endif

static bt_status_t  bt_gap_le_srv_system_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] System EVT(0x%4x), status is 0x%4x", 2, msg, status);
    switch (msg) {
        case BT_POWER_ON_CNF: {
#ifdef BT_DM_POWER
            //g_ble_srv_power_cntx.cur_state = BT_GAP_LE_SRV_POWER_STATE_ON;
#endif
            bt_gap_le_srv_generate_random_address(g_ble_random_addr);
            //bt_gap_le_set_random_address((bt_bd_addr_ptr_t)&g_ble_random_addr);
        }
        break;
        case BT_POWER_OFF_CNF: {
            /** <reset all infos
                      * stop all ble adv
                      * disconnect all BLE connection.*/
            bt_gap_le_srv_reset_all_context();
        }
        break;
        case BT_PANIC:
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][W] Warnning PANIC event", 0);
            break;
        default:
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Error SDK sys msg", 0);
            break;
    }
    return BT_STATUS_SUCCESS;
}

static void bt_gap_le_srv_adv_cntx_init(uint8_t adv_num)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Adv cntx Init, adv num is %d", 1, adv_num);
    uint8_t adv_max_num = adv_num ? adv_num : 1;
    if (NULL != g_le_adv_info) {
        bt_gap_le_srv_memory_free(g_le_adv_info);
        g_le_adv_info = NULL;
    }
    if (NULL == g_le_adv_info) {
        g_le_adv_info = (bt_gap_le_srv_adv_info_t *)bt_gap_le_srv_memory_alloc(adv_max_num * sizeof(bt_gap_le_srv_adv_info_t));
        BT_GAP_LE_SRV_CHECK_RET_WITH_LOG(NULL == g_le_adv_info, "[BLE_GAP_SRV][E] Adv Cntx memory allocate fail", 0);
        bt_gap_le_srv_memset(g_le_adv_info, 0x0, (adv_max_num * sizeof(bt_gap_le_srv_adv_info_t)));
        g_le_adv_max = adv_max_num;
    }
}

static void bt_gap_le_srv_adv_cntx_deinit(void)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Adv Cntx Deinit", 0);
    if (NULL != g_le_adv_info) {
        bt_gap_le_srv_memory_free(g_le_adv_info);
        g_le_adv_info = NULL;
        g_le_adv_max = 0;
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Adv Cntx memory Free done", 0);
    }
}

static void bt_gap_le_srv_conn_buf_init(uint8_t conn_num)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Conn buff init, conn num is %d", 1, conn_num);
    uint8_t device_max_num = conn_num ? conn_num : 1;
    if (NULL != g_le_conn_cntx) {
        bt_gap_le_srv_memory_free(g_le_conn_cntx);
        g_le_conn_cntx = NULL;
    }
    if (NULL == g_le_conn_cntx) {
        g_le_conn_cntx = (bt_gap_le_srv_conn_cntx_t *)bt_gap_le_srv_memory_alloc(device_max_num * sizeof(bt_gap_le_srv_conn_cntx_t));
        BT_GAP_LE_SRV_CHECK_RET_WITH_LOG(NULL == g_le_conn_cntx, "[BLE_GAP_SRV][E] Connection Context memory allocate fail", 0);
        bt_gap_le_srv_memset(g_le_conn_cntx, 0x0, (device_max_num * sizeof(bt_gap_le_srv_conn_cntx_t)));
        g_le_conn_max = device_max_num;
    }
    if (g_handle_table == NULL) {
        g_handle_table = (bt_gap_le_srv_handle_table_t *)bt_gap_le_srv_memory_alloc(device_max_num * sizeof(bt_gap_le_srv_handle_table_t));
        BT_GAP_LE_SRV_CHECK_RET_WITH_LOG(NULL == g_handle_table, "[BLE_GAP_SRV][E] Connection handle table memory allocate fail", 0);
        bt_gap_le_srv_memset(g_handle_table, 0x0, (device_max_num * sizeof(bt_gap_le_srv_handle_table_t)));
    }
}

static void bt_gap_le_srv_conn_buf_deinit(void)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Conn buff Deinit", 0);
    if (NULL != g_le_conn_cntx) {
        bt_gap_le_srv_memory_free(g_le_conn_cntx);
        g_le_conn_cntx = NULL;
        g_le_conn_max = 0;
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Context memory Free done", 0);
    }
}

#ifdef BT_DM_POWER
static void bt_gap_le_srv_power_off_cb_int(bt_gap_le_srv_event_t event, void *data)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]Srv event %d", 1, event);
    bt_gap_le_srv_event_ind_t *ind = (bt_gap_le_srv_event_ind_t *)data;
    switch (event) {
        case BT_GAP_LE_SRV_EVENT_BLE_DISABLED: {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]BLE disable result %d", 1, ind->dis_complete.result);
            if ((0 == ind->dis_complete.result) &&
                (BT_GAP_LE_SRV_POWER_STATE_OFF_PENDING == g_ble_srv_power_cntx.cur_state)) { //disable success
                g_ble_srv_power_cntx.cur_state = BT_GAP_LE_SRV_POWER_STATE_OFF;
                bt_device_manager_dev_set_power_state(BT_DEVICE_TYPE_LE, BT_DEVICE_MANAGER_POWER_STATE_STANDBY);
            } else {//disable failed
                g_ble_srv_power_cntx.cur_state = BT_GAP_LE_SRV_POWER_STATE_ON;
            }
        }
        break;
        default :
            break;
    }
}

static bt_status_t  bt_gap_le_srv_power_device_manager_callback(bt_device_manager_power_event_t evt, bt_device_manager_power_status_t status, void *data, uint32_t data_length)
{
    bt_status_t sta = BT_STATUS_SUCCESS;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] power device_manager callback event = %02x, status = %02x", 2, evt, status);
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_LE_AUDIO_DONGLE_ENABLE)
    if ((status == BT_DEVICE_MANAGER_POWER_STATUS_AIR_PAIRING_START) || (status == BT_DEVICE_MANAGER_POWER_STATUS_AIR_PAIRING_COMPLETE) ||
        (status == BT_DEVICE_MANAGER_POWER_STATUS_ROLE_RECOVERY)) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] because dongle, so not deal role recovery/air pairing", 0);
        return BT_STATUS_SUCCESS;
    }
#endif
    switch (evt) {
        case BT_DEVICE_MANAGER_POWER_EVT_PREPARE_ACTIVE: {
            g_ble_srv_power_cntx.cur_state = BT_GAP_LE_SRV_POWER_STATE_ON;
            break;
        }
        case BT_DEVICE_MANAGER_POWER_EVT_PREPARE_STANDBY: {
            if (BT_GAP_LE_SRV_POWER_STATE_ON == g_ble_srv_power_cntx.cur_state) {
                bt_gap_le_srv_error_t err;
                switch (status) {
                    case BT_DEVICE_MANAGER_POWER_STATUS_ROLE_RECOVERY: {
                        err = bt_gap_le_srv_disable_ble_by_internal(BT_GAP_LE_SRV_DISABLE_BLE_REASON_ROLE_RECOVERY, bt_gap_le_srv_power_off_cb_int);
                    }
                    break;
                    case BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS: {
                        err = bt_gap_le_srv_disable_ble_by_internal(BT_GAP_LE_SRV_DISABLE_BLE_REASON_POWER_OFF, bt_gap_le_srv_power_off_cb_int);
                    }
                    break;
                    default: {
                        err = bt_gap_le_srv_disable_ble_by_internal(BT_GAP_LE_SRV_DISABLE_BLE_REASON_NONE, bt_gap_le_srv_power_off_cb_int);
                    }
                    break;
                }
                if (err == BT_GAP_LE_SRV_ERROR_PENDING) {
                    sta = BT_STATUS_PENDING;
                    g_ble_srv_power_cntx.cur_state = BT_GAP_LE_SRV_POWER_STATE_OFF_PENDING;
                } else if (err == BT_GAP_LE_SRV_SUCCESS) {
                    sta = BT_STATUS_SUCCESS;
                    g_ble_srv_power_cntx.cur_state = BT_GAP_LE_SRV_POWER_STATE_OFF;
                }
            }
            break;
        }
        default:
            break;
    }
    return sta;
}

static void bt_gap_le_srv_power_init(void)
{
    bt_device_manager_dev_register_callback(BT_DEVICE_TYPE_LE, bt_gap_le_srv_power_device_manager_callback);
    bt_gap_le_srv_memset(&g_ble_srv_power_cntx, 0x0, sizeof(bt_gap_le_srv_power_cntx_t));
}

static void bt_gap_le_srv_power_deinit(void)
{
    bt_device_manager_dev_register_callback(BT_DEVICE_TYPE_LE, NULL);
}
#endif

/********************************External Function*****************************/

void bt_gap_le_srv_init(const bt_gap_le_srv_config_t *config)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Init", 0);
    BT_GAP_LE_SRV_CHECK_RET_WITH_LOG(NULL == config, "[BLE_GAP_SRV][E] Config param is null", 0);
    bt_gap_le_srv_conn_buf_init(config->max_connection_num);
    bt_gap_le_srv_adv_cntx_init(config->max_advertising_num);
    bt_callback_manager_register_callback(bt_callback_type_app_event, (uint32_t)MODULE_MASK_GAP, (void *)bt_gap_le_srv_gap_event_callback);
    bt_callback_manager_register_callback(bt_callback_type_app_event, (uint32_t)MODULE_MASK_SYSTEM, (void *)bt_gap_le_srv_system_callback);
#ifdef BT_DM_POWER
    bt_gap_le_srv_power_init();
#endif
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
    bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_BLE_SRV, &bt_gap_le_srv_rho_callbacks);
#endif
    bt_callback_manager_register_callback(bt_callback_type_gap_le_rho_request, 0, (void *)bt_gap_le_srv_rho_request_by_handle);
}

void bt_gap_le_srv_deinit(void)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Deinit", 0);
    bt_callback_manager_deregister_callback(bt_callback_type_app_event, (void *)bt_gap_le_srv_gap_event_callback);
    bt_callback_manager_deregister_callback(bt_callback_type_app_event, (void *)bt_gap_le_srv_system_callback);
#ifdef BT_DM_POWER
    bt_gap_le_srv_power_deinit();
#endif
    bt_callback_manager_deregister_callback(bt_callback_type_gap_le_rho_request, (void *)bt_gap_le_srv_rho_request_by_handle);
    bt_gap_le_srv_conn_buf_deinit();
    bt_gap_le_srv_adv_cntx_deinit();
    bt_gap_le_srv_scan_context_init();
}

//[in, out] data, data_len
bt_gap_le_srv_error_t bt_gap_le_srv_generate_adv_data(const bt_gap_le_srv_adv_fields_t *adv_fields, uint8_t *data, uint32_t *data_len)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Gen data", 0);
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((NULL == adv_fields) || (NULL == data) || (NULL == data_len),
                                           BT_GAP_LE_SRV_ERROR_INVALID_PARAM, "[BLE_GAP_SRV][E] Gen data error, params is null", 0);
    bt_gap_le_srv_mutex_lock();
    bt_gap_le_srv_error_t err;
    uint32_t adv_max_len;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] adv_max_len is 0x%4x, buff_len is 0x%4x", 2, bt_gap_le_get_max_adv_length(), *data_len);
    adv_max_len = (bt_gap_le_get_max_adv_length() > *data_len) ? *data_len : bt_gap_le_get_max_adv_length();
    err = ble_gap_le_srv_generate_data_int(adv_fields, data, data_len, adv_max_len);
    bt_gap_le_srv_mutex_unlock();
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Gen data, result is %d, data length is 0x%4x", 2, err, *data_len);
    return err;
}

//[in, out] length, field_data
bt_gap_le_srv_error_t bt_gap_le_srv_parse_field_data_by_ad_type(uint8_t type,
        uint8_t *adv_data, uint32_t *length, uint8_t **field_data)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((NULL == adv_data) || (NULL == length),
                                           BT_GAP_LE_SRV_ERROR_INVALID_PARAM, "[BLE_GAP_SRV][E] Parse data error, params is null", 0);
    bt_gap_le_srv_mutex_lock();
    uint32_t index = 0;
    uint32_t adv_max_len = bt_gap_le_get_max_adv_length();
    /* Error handling for data length overflow. */
    if (*length > adv_max_len) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] ADV Data Length Error, input len is 0x%4x", 1, *length);
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_ERROR_INVALID_LENTH;
    }
    while (index < *length) {
        uint8_t field_length = adv_data[index];
        uint8_t field_type   = adv_data[index + 1];
        if (field_type == type) {
            *field_data = &adv_data[index + 2];
            *length = (uint32_t)(field_length - 1);
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Parse AD type %d data success, index is %d, length is 0x%4x", 3, type, index, *length);
            bt_gap_le_srv_mutex_unlock();
            return BT_GAP_LE_SRV_SUCCESS;
        }
        index += field_length + 1;
    }
    bt_gap_le_srv_mutex_unlock();
    return BT_GAP_LE_SRV_ERROR_NOT_FOUND;
}

/**
 * @brief The type of advertising instance that Bluetooth GAP LE service supports, Range 1~4.
 */
bt_gap_le_srv_error_t bt_gap_le_srv_get_available_instance(uint8_t *instance)
{
    //first to get an empty instance buffer
    bt_gap_le_srv_mutex_lock();
    uint8_t free_int = bt_gap_le_srv_find_unused_instance();
    if (free_int) {//found free instance
        *instance = free_int;
        g_le_adv_info[free_int - 1].instance = free_int;
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_SUCCESS;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Get available instance fail", 0);
    bt_gap_le_srv_mutex_unlock();
    return BT_GAP_LE_SRV_ERROR_FAIL;
}

static void bt_gap_le_srv_clear_adv_waiting_list(void)
{
    bt_gap_le_srv_linknode_t *temp_node = (bt_gap_le_srv_linknode_t *)adv_wl_list.front;
    while (temp_node != NULL) {
        bt_gap_le_srv_linknode_remove_node((bt_gap_le_srv_linknode_t *)&adv_wl_list, temp_node);
        /* Free node buffer. */
        bt_gap_le_srv_memory_free(temp_node);
        temp_node = (bt_gap_le_srv_linknode_t *)adv_wl_list.front;
    }
    adv_wl_list.front = NULL;
}

static bt_gap_le_adv_waiting_list_context_t *bt_gap_le_adv_find_duplicate_wl_context(bt_gap_le_adv_wl_action_t action, uint8_t instance)
{
    bt_gap_le_srv_linknode_t *temp_node = (bt_gap_le_srv_linknode_t *)adv_wl_list.front;
    while (temp_node != NULL) {
        bt_gap_le_adv_waiting_list_context_t *wl_context = (bt_gap_le_adv_waiting_list_context_t *)temp_node;
        if ((wl_context->wl_action == action) && (wl_context->instance == instance)) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] find duplicate context address = %02x, action = %02x, instance = %02x", 3,
                                    wl_context, action, instance);
            return wl_context;
        }
        temp_node = temp_node->front;
    }
    return NULL;
}

static bt_gap_le_srv_error_t bt_gap_le_add_adv_waiting_list(bt_gap_le_adv_wl_action_t action, uint8_t instance,
        bt_bd_addr_ptr_t random_addr,
        bt_gap_le_srv_adv_time_params_t *time_param,
        void *adv_data_cb,
        void *adv_evt_cb)
{
    bool is_duplicate = false;
    bt_gap_le_adv_waiting_list_context_t *wl_ctx = NULL;
    wl_ctx = bt_gap_le_adv_find_duplicate_wl_context(action, instance);
    if (wl_ctx == NULL) {
        wl_ctx = (bt_gap_le_adv_waiting_list_context_t *)bt_gap_le_srv_memory_alloc(sizeof(bt_gap_le_adv_waiting_list_context_t));
        if (wl_ctx == NULL) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] add adv waiting list alloc buffer fail", 0);
            return BT_GAP_LE_SRV_ERROR_FAIL;
        }
    } else {
        is_duplicate = true;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] add adv waiting list head addr = %02x, node addr = %02x", 2, &adv_wl_list, wl_ctx);
    wl_ctx->wl_action = action;
    wl_ctx->instance = instance;
    if (random_addr != NULL) {
        bt_gap_le_srv_memcpy(&wl_ctx->random_addr, random_addr, sizeof(bt_bd_addr_t));
    }
    if (time_param != NULL) {
        bt_gap_le_srv_memcpy(&wl_ctx->time_param, time_param, sizeof(bt_gap_le_srv_adv_time_params_t));
    }
    wl_ctx->adv_data_cb = adv_data_cb;
    wl_ctx->adv_evt_cb = adv_evt_cb;
    if (!is_duplicate) {
        if (action == BT_GAP_LE_ADV_WL_ACTION_STOP_ALL) {
            /* Clear all action. */
            bt_gap_le_srv_clear_adv_waiting_list();
            bt_gap_le_srv_linknode_insert_node(&adv_wl_list, (bt_gap_le_srv_linknode_t *)wl_ctx, BT_GAP_LE_SRV_NODE_BACK);
        } else {
            bt_gap_le_srv_linknode_insert_node(&adv_wl_list, (bt_gap_le_srv_linknode_t *)wl_ctx, BT_GAP_LE_SRV_NODE_BACK);
        }
    }
    return BT_GAP_LE_SRV_SUCCESS;
}


static bt_gap_le_srv_error_t bt_gap_le_remove_adv_waiting_list_node(bt_gap_le_adv_waiting_list_context_t *context)
{
    if (context == NULL) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] remove adv waiting list node is NULL", 0);
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] remove adv waiting list node addr = %02x", 1, context);
    bt_gap_le_srv_linknode_remove_node(&adv_wl_list, (bt_gap_le_srv_linknode_t *)context);
    /* free alloc buffer. */
    bt_gap_le_srv_memory_free((void *)context);
    return BT_GAP_LE_SRV_SUCCESS;
}

static bool bt_gap_le_srv_adv_is_busy(void)
{
    uint32_t i = 0;
    for (i = 0; i < g_le_adv_max; i++) {
        if ((g_le_adv_info[i].instance != 0) && ((g_le_adv_info[i].state == BT_CM_LE_ADV_STATE_TYPE_STOPPING) ||
                (g_le_adv_info[i].state == BT_CM_LE_ADV_STATE_TYPE_STARTING) || (g_le_adv_info[i].state == BT_CM_LE_ADV_STATE_TYPE_REMOVING) ||
                (g_le_adv_info[i].sub_state == BT_CM_LE_ADV_SUB_STATE_UPDATING) || (g_le_adv_info[i].sub_state == BT_CM_LE_ADV_SUB_STATE_REMOVING))) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] adv is busy instance = %d, adv state = %02x, adv sub_state = %02x", 3,
                                    g_le_adv_info[i].instance, g_le_adv_info[i].state, g_le_adv_info[i].sub_state);
            return true;
        }
    }
    return false;
}

static bool bt_gap_le_srv_all_adv_is_stopped(void)
{
    uint32_t i = 0;
    for (i = 0; i < g_le_adv_max; i++) {
        if ((g_le_adv_info[i].state != BT_CM_LE_ADV_STATE_TYPE_STOPPED) && (g_le_adv_info[i].state != BT_CM_LE_ADV_STATE_TYPE_REMOVED)) {
            return false;
        }
    }
    return true;
}

static bool bt_gap_le_srv_all_adv_is_removed(void)
{
    uint32_t i = 0;
    for (i = 0; i < g_le_adv_max; i++) {
        if (g_le_adv_info[i].state != BT_CM_LE_ADV_STATE_TYPE_REMOVED) {
            return false;
        }
    }
    return true;
}

static void  bt_gap_le_srv_adv_run_wl(void)
{
    bt_gap_le_adv_waiting_list_context_t *wl_context = NULL;
    bt_gap_le_srv_error_t error_status = BT_GAP_LE_SRV_ERROR_FAIL;
    wl_context = (bt_gap_le_adv_waiting_list_context_t *)adv_wl_list.front;
    if (wl_context == NULL) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] There is no adv in waiting list", 0);
        return;
    }

    do {
        switch (wl_context->wl_action) {
            case BT_GAP_LE_ADV_WL_ACTION_START: {
                error_status = bt_gap_le_srv_start_adv(wl_context->instance, (bt_bd_addr_ptr_t)&wl_context->random_addr, &wl_context->time_param, wl_context->adv_data_cb, wl_context->adv_evt_cb);
            }
            break;
            case BT_GAP_LE_ADV_WL_ACTION_UPDATE: {
                error_status = bt_gap_le_srv_update_adv(wl_context->instance);
            }
            break;
            case BT_GAP_LE_ADV_WL_ACTION_STOP: {
                error_status = bt_gap_le_srv_stop_adv(wl_context->instance);
            }
            break;
            case BT_GAP_LE_ADV_WL_ACTION_REMOVE: {
                error_status = bt_gap_le_srv_remove_adv(wl_context->instance);
            }
            break;
            case BT_GAP_LE_ADV_WL_ACTION_STOP_ALL: {
                bt_status_t status = bt_gap_le_srv_stop_all_adv();
                if ((status == BT_STATUS_SUCCESS) && (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_DISABLE_BLE)) {
                    bt_gap_le_srv_remove_all_stopped_adv();
                    error_status = BT_GAP_LE_SRV_SUCCESS;
                } else {
                    error_status = BT_GAP_LE_SRV_ERROR_FAIL;
                }

                if ((status == BT_STATUS_SUCCESS) && ((le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL) || (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST))) {
                    bt_gap_le_srv_notify_restart_conn_fliter_cmd();
                }
            }
            break;
            default:
                break;
        }
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] run waiting list adv action = %02x instance = %d status = %02x", 3, wl_context->wl_action,
                            wl_context->instance, error_status);
        bt_gap_le_adv_waiting_list_context_t *free_wl_node = wl_context;
        wl_context = (bt_gap_le_adv_waiting_list_context_t *)wl_context->node.front;
        bt_gap_le_remove_adv_waiting_list_node(free_wl_node);
    } while ((wl_context != NULL) && (error_status != BT_GAP_LE_SRV_SUCCESS));
}

static void bt_gap_le_srv_reset_adv_waiting_list(void)
{
    bt_gap_le_adv_waiting_list_context_t *wl_context = (bt_gap_le_adv_waiting_list_context_t *)adv_wl_list.front;
    while (wl_context) {
        bt_gap_le_adv_waiting_list_context_t *free_wl_node = wl_context;
        wl_context = (bt_gap_le_adv_waiting_list_context_t *)wl_context->node.front;
        bt_gap_le_remove_adv_waiting_list_node(free_wl_node);
    }
}

/**
 * @brief The type of advertising instance that Bluetooth GAP LE service supports, Range 1~4.
 */
bt_gap_le_srv_error_t bt_gap_le_srv_start_adv(uint8_t instance,
        bt_bd_addr_ptr_t random_addr,
        bt_gap_le_srv_adv_time_params_t *time_param,
        void *adv_data_cb,
        void *adv_evt_cb)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((g_le_adv_max < instance) || (1 > instance),
                                           BT_GAP_LE_SRV_ERROR_INVALID_INSTANCE, "[BLE_GAP_SRV][E] Start adv error, Instance is not in range [1 ~ %d]", 1, g_le_adv_max);
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG(NULL == adv_evt_cb,
                                           BT_GAP_LE_SRV_ERROR_INVALID_PARAM, "[BLE_GAP_SRV][E] Event CB is null", 0);
    bt_gap_le_srv_mutex_lock();
    bt_gap_le_srv_error_t err;
    bt_status_t status;
    //bt_gap_le_srv_adv_info_t *adv_info = bt_gap_le_srv_find_adv_info_by_instance(instance);
    bt_gap_le_srv_adv_info_t *empty_adv_info = &g_le_adv_info[instance - 1];
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        bt_gap_le_srv_mutex_unlock();
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RHO ongoing, please don't start adv!", 0);
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
#endif
#ifdef BT_DM_POWER
    if (g_ble_srv_power_cntx.cur_state != BT_GAP_LE_SRV_POWER_STATE_ON) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] ADV start fail for power state = %02x", 1, g_ble_srv_power_cntx.cur_state);
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
#endif
    if ((le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL) || (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST)) {//updating the rsl, can't start any adv
        if ((BT_CM_LE_ADV_STATE_TYPE_REMOVED == empty_adv_info->state) ||
            (BT_CM_LE_ADV_STATE_TYPE_STOPPED == empty_adv_info->state)) {
            empty_adv_info->sub_state |= BT_CM_LE_ADV_SUB_STATE_FORCE_STOPPING;
            empty_adv_info->evt_cb = adv_evt_cb;
        }
        bt_gap_le_srv_mutex_unlock();
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Start Adv instance %d fail, Setting RSL now", 1, instance);
        return BT_GAP_LE_SRV_ERROR_INVALID_STATE;
    }
    if (bt_gap_le_srv_adv_is_busy()) {
        /* add adv waiting list */
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Start Adv instance %d fail, add adv waiting list", 1, instance);
        bt_gap_le_add_adv_waiting_list(BT_GAP_LE_ADV_WL_ACTION_START, instance, random_addr, time_param, adv_data_cb, adv_evt_cb);
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_SUCCESS;
    }
    if ((0 == empty_adv_info->instance) || ((instance == empty_adv_info->instance) &&
                                            (BT_CM_LE_ADV_STATE_TYPE_REMOVED == empty_adv_info->state)) ||
        ((empty_adv_info->configured) && (adv_data_cb) &&
         (BT_CM_LE_ADV_STATE_TYPE_STOPPED == empty_adv_info->state))) {//instance is not be used means first come
        uint8_t gen_ret;
#if defined(MTK_AWS_MCE_ENABLE)
        bool set_le_pub_addr = false;
#endif
        bt_gap_le_srv_adv_config_info_t config_info ;
        bt_hci_le_set_ext_advertising_parameters_t *adv_params = NULL;
        bt_gap_le_set_ext_advertising_data_t *adv_data = NULL;
        bt_gap_le_set_ext_scan_response_data_t *scan_rsp_data = NULL;
        bt_gap_le_srv_get_adv_data_cb_t data_cb = (bt_gap_le_srv_get_adv_data_cb_t)adv_data_cb;
        uint32_t adv_max_len = bt_gap_le_get_max_adv_length();
        bt_gap_le_srv_memset(&config_info, 0x0, sizeof(bt_gap_le_srv_adv_config_info_t));
        if (NULL == adv_data_cb) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Application not set adv data cb!", 0);
            bt_gap_le_srv_mutex_unlock();
            return BT_GAP_LE_SRV_ERROR_INVALID_PARAM;
        }
        config_info.adv_data.data = (uint8_t *)bt_gap_le_srv_memory_alloc(adv_max_len);
        config_info.scan_rsp.data = (uint8_t *)bt_gap_le_srv_memory_alloc(adv_max_len);
        if ((NULL == config_info.adv_data.data) || (NULL == config_info.scan_rsp.data)) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Can't alloc enough Heap memory", 0);
            bt_gap_le_srv_mutex_unlock();
            return BT_GAP_LE_SRV_ERROR_NO_MEMORY;
        }
        /**< allocate memory success.*/
        config_info.adv_data.data_length = adv_max_len;
        config_info.scan_rsp.data_length = adv_max_len;
        bt_gap_le_srv_memset(config_info.adv_data.data, 0x0, adv_max_len);
        bt_gap_le_srv_memset(config_info.scan_rsp.data, 0x0, adv_max_len);
        /**< begin to get data from upper user.*/
        gen_ret = data_cb(BT_GAP_LE_SRV_ADV_DATA_OP_CONFIG, (void *)&config_info);
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Gen result 0x%x", 1, gen_ret);
        if (!(gen_ret & BT_GAP_LE_ADV_PARAM_GEN))  {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Application not set adv parameters or adv data!", 0);
            bt_gap_le_srv_memory_free(config_info.adv_data.data);
            bt_gap_le_srv_memory_free(config_info.scan_rsp.data);
            bt_gap_le_srv_mutex_unlock();
            return BT_GAP_LE_SRV_ERROR_FAIL;
        } else {
            adv_params = &(config_info.adv_param);
        }
        if (gen_ret & BT_GAP_LE_ADV_DATA_GEN) {
            adv_data = &(config_info.adv_data);
        }
        if (gen_ret & BT_GAP_LE_ADV_SCAN_RSP_GEN)  {
            scan_rsp_data = &(config_info.scan_rsp);
        }

#if defined(MTK_AWS_MCE_ENABLE)
        if (adv_params->own_address_type == BT_ADDR_LE_PUBLIC) {
            adv_params->own_address_type = BT_ADDR_PUBLIC;
            set_le_pub_addr = true;
        }
#endif

        status = bt_gap_le_config_extended_advertising(instance, random_addr,
                 (const bt_hci_le_set_ext_advertising_parameters_t *)adv_params,
                 (const bt_gap_le_set_ext_advertising_data_t *)adv_data,
                 (const bt_gap_le_set_ext_scan_response_data_t *)scan_rsp_data);

#if defined(MTK_AWS_MCE_ENABLE)
        if (set_le_pub_addr) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] BT_ADDR_LE_PUBLIC, instance %d", 1, instance);
            bt_hci_cmd_vendor_le_set_adv_public_addr_t set_public_address = {0};
            set_public_address.enable = BT_HCI_ENABLE;
            set_public_address.advertising_handle = instance;
            bt_gap_le_srv_memcpy(&set_public_address.public_address, bt_device_manager_aws_local_info_get_fixed_address(), sizeof(bt_bd_addr_t));
            bt_gap_le_set_adv_public_address(&set_public_address);
        }
#endif

        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Config instace(%d) adv, status is 0x%4x", 2, instance, status);
        if (BT_STATUS_SUCCESS == status) {
            /**< save the instance adv info.*/
            bt_gap_le_srv_memset(empty_adv_info, 0x0, sizeof(bt_gap_le_srv_adv_info_t));
            empty_adv_info->instance = instance;
            empty_adv_info->data_cb = adv_data_cb;
            empty_adv_info->evt_cb = adv_evt_cb;
            bt_gap_le_srv_memcpy(empty_adv_info->random_addr, random_addr, BT_BD_ADDR_LEN);
            bt_gap_le_srv_memcpy(&(empty_adv_info->ext_param), &(config_info.adv_param), sizeof(bt_hci_le_set_ext_advertising_parameters_t));
            empty_adv_info->state = BT_CM_LE_ADV_STATE_TYPE_STARTING;
            //for enable adv later
            empty_adv_info->ext_enable.enable = BT_HCI_ENABLE;
            if (time_param) {
                empty_adv_info->ext_enable.duration = time_param->duration;
                empty_adv_info->ext_enable.max_ext_advertising_evts = time_param->max_ext_advertising_evts;
            }
            empty_adv_info->sub_state |= BT_CM_LE_ADV_SUB_STATE_CONFIGURING;
        } else {
            if (0 == empty_adv_info->instance) {
                bt_gap_le_srv_memset(empty_adv_info, 0x0, sizeof(bt_gap_le_srv_adv_info_t));
            }
        }
        bt_gap_le_srv_memory_free(config_info.adv_data.data);
        bt_gap_le_srv_memory_free(config_info.scan_rsp.data);
        //MAP error code
        err = bt_gap_le_srv_map_bt_status(status);
        bt_gap_le_srv_mutex_unlock();
        return err;
    }
    if (empty_adv_info->configured) {//not first come, restart an exist instance adv
        bt_status_t status;
        switch (empty_adv_info->state) {
            case BT_CM_LE_ADV_STATE_TYPE_STOPPED: {
                bt_hci_le_set_ext_advertising_enable_t enable;
                if (NULL != adv_data_cb) {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] please remove the adv with instance %d firstly!", 1, instance);
                    err = BT_GAP_LE_SRV_ERROR_INVALID_STATE;
                    break;
                }
                enable.enable = BT_HCI_ENABLE;
                if (time_param) {
                    enable.duration = time_param->duration;
                    enable.max_ext_advertising_evts = time_param->max_ext_advertising_evts;
                }
                status = bt_gap_le_enable_extended_advertising((bt_gap_le_advertising_handle_t)instance,
                         (const bt_hci_le_set_ext_advertising_enable_t *)&enable);
                if (BT_STATUS_SUCCESS != status) {
                    //Map the error code
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Normal Start Adv instance %d fail, status is 0x%4x", 2, instance, status);
                    err = bt_gap_le_srv_map_bt_status(status);
                    bt_gap_le_srv_mutex_unlock();
                    return err;
                } else {
                    //change the state;
                    bt_gap_le_srv_memcpy(&empty_adv_info->ext_enable, &enable, sizeof(bt_hci_le_set_ext_advertising_enable_t));
                    empty_adv_info->state = BT_CM_LE_ADV_STATE_TYPE_STARTING;
                    err = BT_GAP_LE_SRV_SUCCESS;
                }
            }
            break;
            case BT_CM_LE_ADV_STATE_TYPE_STARTING:
                err = BT_GAP_LE_SRV_ERROR_BUSY;
                break;
            case BT_CM_LE_ADV_STATE_TYPE_STARTED: {
                err = BT_GAP_LE_SRV_ERROR_INVALID_STATE;
            }
            break;
            default: {
                err = BT_GAP_LE_SRV_ERROR_INVALID_STATE;
            }
            break;
        }
    } else {
        err = BT_GAP_LE_SRV_ERROR_INSTANCE_USED;
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Start adv (instace:%d)fail, please get a free instance firstly!", 1, instance);
    }
    bt_gap_le_srv_mutex_unlock();
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Start Adv instance %d, err code is %d", 2, instance, err);
    return err;
}

bt_gap_le_srv_error_t bt_gap_le_srv_stop_adv(uint8_t instance)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((g_le_adv_max < instance) || (1 > instance),
                                           BT_GAP_LE_SRV_ERROR_INVALID_INSTANCE, "[BLE_GAP_SRV][E] Stop adv error, Instance is not in range [1 ~ %d]", 1, g_le_adv_max);
    bt_gap_le_srv_mutex_lock();
    bt_gap_le_srv_error_t err;
    bt_gap_le_srv_adv_info_t *adv_info = bt_gap_le_srv_find_adv_info_by_instance(instance);
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        bt_gap_le_srv_mutex_unlock();
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RHO ongoing, please don't stop adv!", 0);
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
#endif
    if (adv_info) {
        bt_status_t status;
        switch (adv_info->state) {
            case BT_CM_LE_ADV_STATE_TYPE_STARTED: {//normal state
                if (bt_gap_le_srv_adv_is_busy()) {
                    /* add adv waiting list */
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Stop Adv instance %d fail, add adv waiting list", 1, instance);
                    bt_gap_le_add_adv_waiting_list(BT_GAP_LE_ADV_WL_ACTION_STOP, instance, NULL, NULL, NULL, NULL);
                    bt_gap_le_srv_mutex_unlock();
                    return BT_GAP_LE_SRV_SUCCESS;
                }
                bt_hci_le_set_ext_advertising_enable_t enable = {BT_HCI_DISABLE, 0, 0};
                status = bt_gap_le_enable_extended_advertising((bt_gap_le_advertising_handle_t)instance,
                         (const bt_hci_le_set_ext_advertising_enable_t *)&enable);
                if (BT_STATUS_SUCCESS != status) {
                    //Map the error code
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Normal Stop Adv instance %d fail,status is 0x%4x", 1, instance, status);
                    err = bt_gap_le_srv_map_bt_status(status);
                    bt_gap_le_srv_mutex_unlock();
                    return err;
                } else {
                    //change the state;
                    adv_info->state = BT_CM_LE_ADV_STATE_TYPE_STOPPING;
                    err = BT_GAP_LE_SRV_SUCCESS;
                }
            }
            break;
            case BT_CM_LE_ADV_STATE_TYPE_STOPPING:
                err = BT_GAP_LE_SRV_ERROR_BUSY;
                break;
            case BT_CM_LE_ADV_STATE_TYPE_STOPPED: {
                err = BT_GAP_LE_SRV_ERROR_INVALID_STATE;
            }
            break;
            default: {
                err = BT_GAP_LE_SRV_ERROR_INVALID_STATE;
            }
            break;
        }
    } else {
        //config the instance firstly
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Stop fail. Adv instance not be found, please config it firstly", 0);
        err = BT_GAP_LE_SRV_ERROR_NOT_FOUND;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Stop instace(%d) adv,  err code is %d", 2, instance, err);
    bt_gap_le_srv_mutex_unlock();
    return err;
}

bt_gap_le_srv_error_t bt_gap_le_srv_update_adv(uint8_t instance)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((g_le_adv_max < instance) || (1 > instance),
                                           BT_GAP_LE_SRV_ERROR_INVALID_INSTANCE, "[BLE_GAP_SRV][E] Update adv error, Instance is not in range [1 ~ %d]", 1, g_le_adv_max);
    bt_gap_le_srv_mutex_lock();
    bt_gap_le_srv_error_t err;
    bt_gap_le_srv_adv_info_t *adv_info = bt_gap_le_srv_find_adv_info_by_instance(instance);
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        bt_gap_le_srv_mutex_unlock();
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RHO ongoing, please don't update adv!", 0);
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
#endif
    if (adv_info) {
        if (bt_gap_le_srv_adv_is_busy()) {
            /* add adv waiting list */
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Update Adv instance %d fail, add adv waiting list", 1, instance);
            bt_gap_le_add_adv_waiting_list(BT_GAP_LE_ADV_WL_ACTION_UPDATE, instance, NULL,
                                           NULL, NULL, NULL);
            bt_gap_le_srv_mutex_unlock();
            return BT_GAP_LE_SRV_SUCCESS;
        }
        bt_status_t status;
        switch (adv_info->state) {
            case BT_CM_LE_ADV_STATE_TYPE_STOPPED:
            case BT_CM_LE_ADV_STATE_TYPE_STARTED: {//instance is not be used and have empty buffer
                uint8_t gen_ret;
                bt_gap_le_srv_adv_update_info_t update_info ;
                bt_gap_le_set_ext_advertising_data_t *adv_data = NULL;
                bt_gap_le_set_ext_scan_response_data_t *scan_rsp_data = NULL;
                uint32_t adv_max_len = bt_gap_le_get_max_adv_length();
                bt_gap_le_srv_memset(&update_info, 0x0, sizeof(bt_gap_le_srv_adv_update_info_t));
                update_info.adv_data.data = (uint8_t *)bt_gap_le_srv_memory_alloc(adv_max_len);
                update_info.scan_rsp.data = (uint8_t *)bt_gap_le_srv_memory_alloc(adv_max_len);
                if ((NULL == update_info.adv_data.data) || (NULL == update_info.scan_rsp.data)) {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Can't alloc enough Heap memory", 0);
                    bt_gap_le_srv_mutex_unlock();
                    return BT_GAP_LE_SRV_ERROR_NO_MEMORY;
                }
                update_info.adv_data.data_length = adv_max_len;
                update_info.scan_rsp.data_length = adv_max_len;
                bt_gap_le_srv_memset(update_info.adv_data.data, 0x0, adv_max_len);
                bt_gap_le_srv_memset(update_info.scan_rsp.data, 0x0, adv_max_len);
                gen_ret = adv_info->data_cb(BT_GAP_LE_SRV_ADV_DATA_OP_UPDATE, &update_info);
                if ((!(gen_ret & BT_GAP_LE_ADV_SCAN_RSP_GEN)) && (!(gen_ret & BT_GAP_LE_ADV_DATA_GEN)))  {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] No Update data generated!", 0);
                    bt_gap_le_srv_memory_free(update_info.adv_data.data);
                    bt_gap_le_srv_memory_free(update_info.scan_rsp.data);
                    bt_gap_le_srv_mutex_unlock();
                    return BT_GAP_LE_SRV_ERROR_FAIL;
                }
                if (gen_ret & BT_GAP_LE_ADV_DATA_GEN) {
                    adv_data = &(update_info.adv_data);
                }
                if (gen_ret & BT_GAP_LE_ADV_SCAN_RSP_GEN) {
                    scan_rsp_data = &(update_info.scan_rsp);
                }
                status = bt_gap_le_config_extended_advertising(instance, NULL,
                         NULL,
                         (const bt_gap_le_set_ext_advertising_data_t *)adv_data,
                         (const bt_gap_le_set_ext_scan_response_data_t *)scan_rsp_data);
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Update instace(%d) adv, status is 0x%4x", 2, instance, status);
                if (BT_STATUS_SUCCESS == status) {
                    adv_info->sub_state |= BT_CM_LE_ADV_SUB_STATE_UPDATING;
                    err = BT_GAP_LE_SRV_SUCCESS;
                } else {
                    //MAP error code function
                    err = bt_gap_le_srv_map_bt_status(status);
                }
                bt_gap_le_srv_memory_free(update_info.adv_data.data);
                bt_gap_le_srv_memory_free(update_info.scan_rsp.data);
            }
            break;
            default: {
                err = BT_GAP_LE_SRV_ERROR_INVALID_STATE;
            }
            break;
        }
    } else {
        //config the instance firstly
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Update fail.Adv Instance not be found, please config it firstly", 0);
        err = BT_GAP_LE_SRV_ERROR_NOT_FOUND;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Update instace(%d) adv,  err code is %d", 2, instance, err);
    bt_gap_le_srv_mutex_unlock();
    return err;
}

bt_gap_le_srv_error_t bt_gap_le_srv_remove_adv(uint8_t instance)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((g_le_adv_max < instance) || (1 > instance),
                                           BT_GAP_LE_SRV_ERROR_INVALID_INSTANCE, "[BLE_GAP_SRV][E] Remove adv error, Instance is not in range [1 ~ %d]", 1, g_le_adv_max);
    bt_gap_le_srv_mutex_lock();
    bt_gap_le_srv_error_t err = 0;
    bool is_add_waiting = false;
    bt_gap_le_srv_adv_info_t *adv_info = bt_gap_le_srv_find_adv_info_by_instance(instance);
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        bt_gap_le_srv_mutex_unlock();
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RHO ongoing, please don't remove adv!", 0);
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
#endif
    if (adv_info) {
        bt_status_t status;
        if (false == adv_info->configured) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Adv not be configured, so no need to remove", 0);
            bt_gap_le_srv_mutex_unlock();
            return BT_GAP_LE_SRV_ERROR_FAIL;
        }
        switch (adv_info->state) {
            case BT_CM_LE_ADV_STATE_TYPE_STARTED: {//normal state
                if (bt_gap_le_srv_adv_is_busy()) {
                    /* add adv waiting list */
                    is_add_waiting = true;
                    break;
                }
                bt_hci_le_set_ext_advertising_enable_t enable = {BT_HCI_DISABLE, 0, 0};
                status = bt_gap_le_enable_extended_advertising((bt_gap_le_advertising_handle_t)instance,
                         (const bt_hci_le_set_ext_advertising_enable_t *)&enable);
                if (BT_STATUS_SUCCESS != status) {
                    //Map the error code
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Remove(first stop) Adv instance %d fail, status is 0x%4x", 2, instance, status);
                    bt_gap_le_srv_mutex_unlock();
                    return BT_GAP_LE_SRV_ERROR_FAIL;
                } else {
                    //change the state;
                    adv_info->sub_state = BT_CM_LE_ADV_SUB_STATE_REMOVING;
                }
            }
            break;
            case BT_CM_LE_ADV_STATE_TYPE_STOPPED: {
                if (bt_gap_le_srv_adv_is_busy()) {
                    /* add adv waiting list */
                    is_add_waiting = true;
                    break;
                }
                status = bt_gap_le_remove_extended_advertising((bt_gap_le_advertising_handle_t)instance, BT_HCI_DISABLE);
                if (0 == status) {
                    adv_info->state = BT_CM_LE_ADV_STATE_TYPE_REMOVING;
                } else {
                    //Map the error code
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Remove Adv instance %d fail, status is 0x%4x", 2, instance, status);
                    err = bt_gap_le_srv_map_bt_status(status);
                    bt_gap_le_srv_mutex_unlock();
                    return err;
                }
            }
            break;
            case BT_CM_LE_ADV_STATE_TYPE_REMOVED: {
                err = BT_GAP_LE_SRV_ERROR_INVALID_STATE;
            }
            break;
            case BT_CM_LE_ADV_STATE_TYPE_REMOVING: {
                err = BT_GAP_LE_SRV_ERROR_BUSY;
            }
            break;
            case BT_CM_LE_ADV_STATE_TYPE_STOPPING:
            {
                /* add adv waiting list */
                is_add_waiting = true;
            }
            break;
            default: {
                err = BT_GAP_LE_SRV_ERROR_INVALID_STATE;
            }
            break;
        }
    } else {
        //config the instance firstly
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Remove ADV fail. Adv Instance not be found, please config it firstly", 0);
        err = BT_GAP_LE_SRV_ERROR_NOT_FOUND;
    }
    if (is_add_waiting) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Remove started adv instance %d pending, add adv waiting list", 1, instance);
        bt_gap_le_add_adv_waiting_list(BT_GAP_LE_ADV_WL_ACTION_REMOVE, instance, NULL, NULL, NULL, NULL);
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Remove instace(%d) adv,  err code is %d", 2, instance, err);
    bt_gap_le_srv_mutex_unlock();
    return err;
}

static bool bt_gap_le_srv_is_all_adv_removed(void)
{
    uint32_t i = 0;
    for (i = 0; i < g_le_adv_max; i++) {
        if ((g_le_adv_info[i].instance != 0) && (g_le_adv_info[i].state != BT_CM_LE_ADV_STATE_TYPE_REMOVED)) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] adv is removed instance = %d, adv state = %02x, adv sub_state = %02x", 3,
                                    g_le_adv_info[i].instance, g_le_adv_info[i].state, g_le_adv_info[i].sub_state);
            return false;
        }
    }
    return true;
}

bt_gap_le_srv_error_t bt_gap_le_srv_clear_adv(void *callback)
{
    bt_gap_le_srv_mutex_lock();
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        bt_gap_le_srv_mutex_unlock();
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RHO ongoing, please don't clear adv!", 0);
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
#endif
    if (g_le_adv_clear.clearing) {//any user had called, return error
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_ERROR_BUSY;
    }
    bt_gap_le_srv_memset(&g_le_adv_clear, 0x0, sizeof(bt_gap_le_srv_adv_clear_info_t));
    g_le_adv_clear.app_cb = callback;
    g_le_adv_clear.clearing = true;
    if (bt_gap_le_srv_stop_all_adv() == BT_STATUS_SUCCESS) {
        if (bt_gap_le_srv_is_all_adv_removed()) {
            g_le_adv_clear.clearing = false;
        } else {
            bt_gap_le_srv_remove_all_stopped_adv();
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Clearing all instaces adv", 0);
    bt_gap_le_srv_mutex_unlock();
    return BT_GAP_LE_SRV_SUCCESS;
}

bt_gap_le_srv_adv_state_t bt_gap_le_srv_get_adv_state(uint8_t instance)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((g_le_adv_max < instance) || (1 > instance),
                                           BT_GAP_LE_SRV_ERROR_INVALID_INSTANCE, "[BLE_GAP_SRV][E] Get adv state error, Instance is not in range [1 ~ %d]", 1, g_le_adv_max);
    bt_gap_le_srv_mutex_lock();
    bt_gap_le_srv_adv_info_t *adv_info = bt_gap_le_srv_find_adv_info_by_instance(instance);
    if (adv_info) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Get instace(%d) adv state is %d", 2, instance, adv_info->state);
        bt_gap_le_srv_mutex_unlock();
        return adv_info->state;
    }
    bt_gap_le_srv_mutex_unlock();
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Get adv state error. Adv Instance not be found", 0);
    return 0;
}

bt_gap_le_srv_conn_info_t *bt_gap_le_srv_get_conn_info(bt_handle_t conn_handle)
{
    //find the connection info by conn -handle bt_gap_le_srv_conn_info_t
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((0x00 == conn_handle),
                                           NULL, "[BLE_GAP_SRV][E] Get conn info, conn handle is wrong ", 0);
    bt_gap_le_srv_mutex_lock();
    bt_gap_le_srv_conn_info_t *conn_info = NULL;
    bt_gap_le_srv_conn_cntx_t *conn_cntx = bt_gap_le_srv_get_conn_cntx_by_handle(conn_handle);
    if (conn_cntx) {
        conn_info = &(conn_cntx->conn_info);
    }
    bt_gap_le_srv_mutex_unlock();
    return conn_info;
}

uint16_t bt_gap_le_srv_get_conn_handle_by_addr(bt_addr_t *addr, bool is_local_addr)
{
    //find the connection info by conn -handle bt_gap_le_srv_conn_info_t
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((NULL == addr),
                                           0xFFFF, "[BLE_GAP_SRV][E] Get conn handle, addr is wrong ", 0);
    bt_gap_le_srv_mutex_lock();
    uint16_t conn_handle = 0xFFFF;
    bt_gap_le_srv_conn_cntx_t *conn_cntx = bt_gap_le_srv_get_conn_cntx_by_local_addr(addr, is_local_addr);
    if (conn_cntx != NULL) {
        conn_handle = conn_cntx->connection_handle;
    }
    bt_gap_le_srv_mutex_unlock();
    return conn_handle;
}

bt_gap_le_srv_conn_params_t *bt_gap_le_srv_get_current_conn_params(bt_handle_t conn_handle)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((0x00 == conn_handle),
                                           NULL, "[BLE_GAP_SRV][E] Get conn params, conn handle is wrong ", 0);
    bt_gap_le_srv_mutex_lock();
    bt_gap_le_srv_conn_params_t *conn_params = NULL;
    bt_gap_le_srv_conn_cntx_t *conn_cntx = bt_gap_le_srv_get_conn_cntx_by_handle(conn_handle);
    if (conn_cntx) {
        conn_params = &(conn_cntx->conn_params);
    }
    bt_gap_le_srv_mutex_unlock();
    return conn_params;
}

bt_gap_le_srv_error_t bt_gap_le_srv_update_conn_params(bt_handle_t conn_handle, bt_gap_le_srv_conn_params_t *param, void *callback)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((0x00 == conn_handle) || (NULL == param),
                                           BT_GAP_LE_SRV_ERROR_INVALID_PARAM, "[BLE_GAP_SRV][E] Get conn params, conn handle is wrong ", 0);
    bt_gap_le_srv_mutex_lock();
    bt_gap_le_srv_error_t err;
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE) && !defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        bt_gap_le_srv_mutex_unlock();
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RHO ongoing, please don't update conn parameters!", 0);
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
#endif
    if (bt_gap_le_srv_validate_conn_params(param)) {
        bt_gap_le_srv_conn_cntx_t *conn_cntx = bt_gap_le_srv_get_conn_cntx_by_handle(conn_handle);
        if (NULL == conn_cntx) {
            bt_gap_le_srv_mutex_unlock();
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Can't find the conn cntx by conn handle!", 0);
            return BT_GAP_LE_SRV_ERROR_FAIL;
        }
        if (0 == bt_gap_le_srv_memcmp(&(conn_cntx->conn_params), param, sizeof(bt_gap_le_srv_conn_params_t))) {//no re-update the same param
            bt_gap_le_srv_mutex_unlock();
            return BT_GAP_LE_SRV_SUCCESS;
        } else {//not the same param, to update
            bt_hci_cmd_le_connection_update_t new_param;
            bt_status_t sta;
            conn_cntx->update_cb = callback;
            new_param.connection_handle = conn_handle;
            new_param.conn_interval_min = param->conn_interval;
            new_param.conn_interval_max = param->conn_interval;
            new_param.conn_latency = param->conn_latency;
            new_param.supervision_timeout = param->supervision_timeout;
            sta = bt_gap_le_update_connection_parameter((const bt_hci_cmd_le_connection_update_t *)(&new_param));
            err = bt_gap_le_srv_map_bt_status(sta);
            bt_gap_le_srv_mutex_unlock();
            return err;
        }
    } else {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] Update conn params Fail, params is invalid", 0);
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_ERROR_INVALID_PARAM;
    }
}

const bt_bd_addr_t *bt_gap_le_srv_get_local_random_addr(void)
{
    bt_gap_le_srv_mutex_lock();
    bt_bd_addr_t *addr;
    addr = &g_ble_random_addr;
    bt_gap_le_srv_mutex_unlock();
    return (const bt_bd_addr_t *)addr;
}

bt_gap_le_srv_error_t bt_gap_le_srv_clear_connections(void *callback)
{
    bt_gap_le_srv_mutex_lock();
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE) && !defined (BT_ROLE_HANDOVER_WITH_SPP_BLE)
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        bt_gap_le_srv_mutex_unlock();
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RHO ongoing, please don't clear connections!", 0);
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
#endif
    if (g_le_conn_clear.clearing) { //any user had called, return error
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_ERROR_BUSY;
    }
    bt_gap_le_srv_memset(&g_le_conn_clear, 0x0, sizeof(bt_gap_le_srv_conn_clear_info_t));
    g_le_conn_clear.app_cb = callback;
    bt_gap_le_srv_disconn_first_connection(BT_GAP_LE_SRV_DISABLE_BLE_REASON_NONE);
    if (g_disable_ble_info.disconn_num > 0) {//need waiting the disconnect ind
        g_le_conn_clear.clearing = true;
    } else {//no connections
        bt_gap_le_srv_conn_clear_cnf();
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] Clear all connections", 0);
    bt_gap_le_srv_mutex_unlock();
    return BT_GAP_LE_SRV_SUCCESS;
}

bt_handle_t bt_gap_le_srv_get_conn_handle_by_address(const bt_bd_addr_t *address)
{
    //find the connection info by conn -handle bt_gap_le_srv_conn_info_t
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((NULL == address),
                                           0x0000, "[BLE_GAP_SRV][E] Get conn handle, addr is wrong ", 0);
    bt_gap_le_srv_mutex_lock();
    uint32_t i = 0;
    bt_bd_addr_t default_address = {0};
    bt_handle_t connection_handle = 0;
    for (i = 0; i < g_le_conn_max; i++) {
        if ((0 == bt_gap_le_srv_memcmp(&(g_le_conn_cntx[i].conn_info.peer_addr.addr), address, sizeof(bt_bd_addr_t))) ||\
            ((0 == bt_gap_le_srv_memcmp(&(g_le_conn_cntx[i].peer_rpa), address, sizeof(bt_bd_addr_t))) &&\
             (0 != bt_gap_le_srv_memcmp(&(g_le_conn_cntx[i].peer_rpa), default_address, sizeof(bt_bd_addr_t))))) {
            connection_handle = g_le_conn_cntx[i].connection_handle;
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] get connection handle:%02x by remote address %02x:%02x:%02x:%02x:%02x:%02x", 7, connection_handle,
                                                   (*address)[0], (*address)[1], (*address)[2], (*address)[3], (*address)[4], (*address)[5]);
    bt_gap_le_srv_mutex_unlock();
    return connection_handle;
}

bt_gap_le_srv_error_t bt_gap_le_srv_disable_ble(void *callback)
{
    /**< check link connection and disconnect all links
        * check all starting or started adv, then stop them
        * and after receive the confirm event, notify to BT CM. */
    bt_gap_le_srv_mutex_lock();
    bool all_adv_removed = false;
    bool all_link_disconnted = false;
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        bt_gap_le_srv_mutex_unlock();
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RHO ongoing, please don't disable ble!", 0);
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
#endif
    if (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_DISABLE_BLE) { //any user had called, return error
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_ERROR_BUSY;
    }
    bt_gap_le_srv_memset(&g_disable_ble_info, 0x0, sizeof(bt_gap_le_srv_disable_ble_info_t));
    g_disable_ble_info.app_cb = callback;
    /* stop and remove all adv. */
    if ((bt_gap_le_srv_stop_all_adv() == BT_STATUS_SUCCESS) && (!bt_gap_le_srv_all_adv_is_removed())) {
        bt_gap_le_srv_remove_all_stopped_adv();
    } else {
        if (!(adv_context.flag & BT_GAP_LE_SRV_ADV_FLAG_STOPPING_ALL)) {
            all_adv_removed = true;
        }
    }
    /**< disconnect all connected device. */
    bt_gap_le_srv_disconn_first_connection(BT_GAP_LE_SRV_DISABLE_BLE_REASON_NONE);
    if (g_disable_ble_info.disconn_num > 0) {//need waiting the disconnect ind
        g_disable_ble_info.disconnecting_all = true;
    } else {
        all_link_disconnted = true;
    }
    if (all_link_disconnted & all_adv_removed) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] disable ble complete", 0);
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_SUCCESS;
    }
    le_srv_context.flag |= BT_GAP_LE_SRV_FLAG_DISABLE_BLE;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] disable ble is ongoing", 0);
    bt_gap_le_srv_mutex_unlock();
    return BT_GAP_LE_SRV_ERROR_PENDING;
}

static bt_gap_le_srv_error_t bt_gap_le_srv_disable_ble_by_internal(bt_gap_le_srv_disable_ble_reason_t reason, void *callback)
{
    bt_gap_le_srv_mutex_lock();
    bool all_adv_removed = false;
    bool all_link_disconnted = false;
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RHO ongoing, please don't disable ble!", 0);
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_ERROR_FAIL;
    }
#endif
    if (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_DISABLE_BLE) { //any user had called, return error
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] disable le is ongoing", 0);
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_ERROR_BUSY;
    }
    bt_gap_le_srv_memset(&g_disable_ble_info, 0x0, sizeof(bt_gap_le_srv_disable_ble_info_t));
    g_disable_ble_info.app_cb = callback;
    g_disable_ble_info.reason = reason;
    /* stop and remove all adv. */
    if ((bt_gap_le_srv_stop_all_adv() == BT_STATUS_SUCCESS) && (!bt_gap_le_srv_all_adv_is_removed())) {
        bt_gap_le_srv_remove_all_stopped_adv();
    } else {
        if (!(adv_context.flag & BT_GAP_LE_SRV_ADV_FLAG_STOPPING_ALL)) {
            all_adv_removed = true;
        }
    }
    /* disconnect all connected device. */
    bt_gap_le_srv_disconn_first_connection(reason);
    if (g_disable_ble_info.disconn_num > 0) {//need waiting the disconnect ind
        g_disable_ble_info.disconnecting_all = true;
    } else {
        all_link_disconnted = true;
    }
    if (all_link_disconnted & all_adv_removed) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] disable ble complete", 0);
        bt_gap_le_srv_mutex_unlock();
        return BT_GAP_LE_SRV_SUCCESS;
    }
    le_srv_context.flag |= BT_GAP_LE_SRV_FLAG_DISABLE_BLE;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] disable ble is ongoing", 0);
    bt_gap_le_srv_mutex_unlock();
    return BT_GAP_LE_SRV_ERROR_PENDING;
}

uint8_t bt_gap_le_srv_get_connected_dev_num(void)
{
    bt_gap_le_srv_mutex_lock();
    uint8_t dev_num = bt_gap_le_srv_get_connected_dev_num_int();
    bt_gap_le_srv_mutex_unlock();
    return dev_num;
}

void bt_gap_le_srv_dump_conn_info_list(void)
{
    uint8_t i;
    for (i = 0; i < g_le_conn_max; i++) {
        if (g_le_conn_cntx[i].connection_handle) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]Conn Info[%d]: [Addr type: %x] %02x-%02x-%02x-%02x-%02x-%02x [0x%04x] [Slave? %d]", 10,
                                    i, g_le_conn_cntx[i].conn_info.peer_addr.type,
                                    g_le_conn_cntx[i].conn_info.peer_addr.addr[0], g_le_conn_cntx[i].conn_info.peer_addr.addr[1],
                                    g_le_conn_cntx[i].conn_info.peer_addr.addr[2], g_le_conn_cntx[i].conn_info.peer_addr.addr[3],
                                    g_le_conn_cntx[i].conn_info.peer_addr.addr[4], g_le_conn_cntx[i].conn_info.peer_addr.addr[5],
                                    g_le_conn_cntx[i].connection_handle, g_le_conn_cntx[i].conn_info.role);
        }
    }
}

void bt_gap_le_srv_dump_adv_info_list(void)
{
    uint8_t i;
    for (i = 0; i < g_le_adv_max; i++) {
        if (g_le_adv_info[i].instance) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I]adv Info[%d]: [instance: %d, configured: %d, state %d, sub_state: %d]", 5,
                                    i, g_le_adv_info[i].instance, g_le_adv_info[i].configured,
                                    g_le_adv_info[i].state, g_le_adv_info[i].sub_state);
        }
    }
}

bt_gap_le_srv_error_t bt_gap_le_srv_get_link_attribute_register(bt_gap_le_srv_get_link_attribute_callback_t callback)
{
    if (le_srv_context.attribute_callback != NULL) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] link attribute had register", 0);
        return BT_GAP_LE_SRV_ERROR_BUSY;
    }
    le_srv_context.attribute_callback = callback;
    return BT_GAP_LE_SRV_SUCCESS;
}

bt_handle_t bt_gap_le_srv_get_handle_by_old_handle(bt_handle_t old_handle)
{
    uint8_t i = 0;
    for (i = 0; i < g_le_conn_max; i++) {
        if (g_handle_table[i].old_conn_handle == old_handle) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] get new handle:%02x by old handle:%02x", 2, g_handle_table[i].new_conn_handle, old_handle);
            return g_handle_table[i].new_conn_handle;
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] not find mpping handle,old_handle = %02x", 1, old_handle);
    return 0;
}

bt_gap_le_srv_link_attribute_t bt_gap_le_srv_get_link_attribute_by_handle(bt_handle_t handle)
{
    uint8_t i = 0;
    for (i = 0; i < g_le_conn_max; i++) {
        if (g_le_conn_cntx[i].connection_handle == handle) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] connection link attributre= %02x by handle = %02x", 2, g_le_conn_cntx[i].conn_info.attribute, handle);
            return g_le_conn_cntx[i].conn_info.attribute;
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] not find connection link handle = %02x", 1, handle);
    return 0;
}
static bt_status_t bt_gap_le_srv_rho_request_by_handle(bt_handle_t handle)
{
    uint8_t i = 0;
    if ((handle == 0x0000) || (handle == BT_HANDLE_INVALID)) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] connection handle is 0x0000", 0);
        return BT_STATUS_SUCCESS;
    }
    for (i = 0; i < g_le_conn_max; i++) {
        if (g_le_conn_cntx[i].connection_handle == handle) {
            if (g_le_conn_cntx[i].conn_info.attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NOT_NEED_RHO) {
                return BT_STATUS_FAIL;
            } else {
                return BT_STATUS_SUCCESS;
            }
        }
    }
    return BT_STATUS_SUCCESS;
}

bt_gap_le_srv_error_t bt_gap_le_srv_get_conn_handle_by_addr_ext(bt_handle_t *handle_list, uint8_t *count, bt_addr_t *addr, bool is_local_addr)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((NULL == addr),
                                           0xFFFF, "[BLE_GAP_SRV][E] Get conn handle array, addr is wrong ", 0);
    bt_gap_le_srv_mutex_lock();
    uint8_t buffer_size = *count;
    uint8_t handle_cnt = 0;
    uint8_t i = 0;
    if (is_local_addr) {
        for (i = 0; i < g_le_conn_max; i++) {
            if (0 == bt_gap_le_srv_memcmp(&(g_le_conn_cntx[i].conn_info.local_addr), addr, sizeof(bt_addr_t))) {
                handle_list[handle_cnt] = g_le_conn_cntx[i].connection_handle;
                handle_cnt++;
                if (handle_cnt >= buffer_size) {
                    break;
                }
            }
        }
    } else {
        for (i = 0; i < g_le_conn_max; i++) {
            if (0 == bt_gap_le_srv_memcmp(&(g_le_conn_cntx[i].conn_info.peer_addr), addr, sizeof(bt_addr_t))) {
                handle_list[handle_cnt] = g_le_conn_cntx[i].connection_handle;
                handle_cnt++;
                if (handle_cnt >= buffer_size) {
                    break;
                }
            }
        }
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] find connection handle is_local_addr = %02x, count = %d", 2, is_local_addr, handle_cnt);
    *count = handle_cnt;
    bt_gap_le_srv_mutex_unlock();
    return BT_GAP_LE_SRV_SUCCESS;
}


static bt_status_t bt_gap_le_srv_notify_restart_conn_fliter_cmd(void)
{
    if ((bt_gap_le_srv_all_adv_is_stopped()) && (!bt_gap_le_srv_scan_is_busy()) && (!bt_gap_le_srv_connect_is_busy())) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] restart resolving list flag = %02x", 1, le_srv_context.flag);
        bt_gap_le_srv_event_notify_user(BT_GAP_LE_SRV_EVENT_RSL_SET_PREPARED_COMPLETE_IND, NULL);

        if (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL) {
        bt_device_manager_le_update_resolving_list();
        }
        if (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST) {
            bt_gap_le_srv_run_white_list();
        }
        return BT_STATUS_SUCCESS;
    } else {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] restart resolving list, scan state = %02x, connect state = %02x", 2, scan_context.state, g_connect_context.state);
    }
    return BT_STATUS_FAIL;
}

static void bt_gap_le_srv_scan_update_state(bt_gap_le_srv_scan_state_t state)
{
    scan_context.state = state;
}

static void bt_gap_le_srv_scan_notify_user(bt_gap_le_srv_event_t event)
{
    if (scan_context.user_callback != NULL) {
        scan_context.user_callback(event, NULL);
    }
}

static void bt_gap_le_srv_scan_context_init(void)
{
    bt_gap_le_srv_memset(&scan_context, 0, sizeof(bt_gap_le_srv_scan_context_t));
}

static void bt_gap_le_scan_state_none(bt_gap_le_srv_scan_sm_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (event) {
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_SATRT: {
            bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_ENABLING);
        }
        break;
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_RSL_COMPLETE: {
            if (scan_context.flag & BT_GAP_LE_SRV_SCAN_FLAG_START_AFTER_CONN_FILTER_CMD) {
                scan_context.flag &= ~BT_GAP_LE_SRV_SCAN_FLAG_START_AFTER_CONN_FILTER_CMD;
                scan_context.param.duration_count = (scan_context.param.enable.duration * 10) + bt_os_layer_get_system_tick();
                status = bt_gap_le_srv_set_scan_by_internel(true);
                if (status == BT_STATUS_SUCCESS) {
                    bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_ENABLING);
                } else {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] set scan by internal fail, start after rsl", 0);
                }
            }
        }
        break;
        default:
            break;
    }
}

static void bt_gap_le_scan_state_enabling(bt_gap_le_srv_scan_sm_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (event) {
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_CMD_CONFIRM: {
            bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_ENABLED);
            /* Notif user. */
            bt_gap_le_srv_scan_notify_user(BT_GAP_LE_SRV_EVENT_SCAN_STARTED);
            if (scan_context.flag & BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_CONN_FILTER_CMD) {
                status = bt_gap_le_srv_set_scan_by_internel(false);
                if (status == BT_STATUS_SUCCESS) {
                    scan_context.flag |= BT_GAP_LE_SRV_SCAN_FLAG_START_AFTER_CONN_FILTER_CMD;
                    bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_DISABLING);
                } else {
                    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] set scan by internal fail", 0);
                }
            }
        }
        break;
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_CMD_FAIL: {
            bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_DISABLED);
            /* Notif user. */
            bt_gap_le_srv_scan_notify_user(BT_GAP_LE_SRV_EVENT_SCAN_STOPPED);
            if (scan_context.flag & BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_CONN_FILTER_CMD) {
                bt_gap_le_srv_notify_restart_conn_fliter_cmd();
            }
        }
        break;
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_CONN_FILTER_CMD_START: {
            scan_context.flag |= BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_CONN_FILTER_CMD;
        }
        break;
        default:
            break;
    }
}

static void bt_gap_le_scan_state_started(bt_gap_le_srv_scan_sm_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (event) {
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_STOP: {
            bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_DISABLING);
        }
        break;
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_CONN_FILTER_CMD_START: {
            status = bt_gap_le_srv_set_scan_by_internel(false);
            if (status == BT_STATUS_SUCCESS) {
                scan_context.flag |= BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_CONN_FILTER_CMD;
                scan_context.flag |= BT_GAP_LE_SRV_SCAN_FLAG_START_AFTER_CONN_FILTER_CMD;
                bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_DISABLING);
            } else {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] set scan by internal fail", 0);
            }
        }
        break;
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_TIMEOUT: {
            bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_DISABLED);
            /* Notif user. */
            bt_gap_le_srv_scan_notify_user(BT_GAP_LE_SRV_EVENT_SCAN_STOPPED);
        }
        break;
        default:
            break;
    }
}

static void bt_gap_le_scan_state_disabling(bt_gap_le_srv_scan_sm_event_t event, void *parameter)
{
    switch (event) {
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_CMD_CONFIRM: {
            bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_DISABLED);
            if (scan_context.flag & BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_CONN_FILTER_CMD) {
                if (scan_context.flag & BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_USER) {
                    /* Notif user. */
                    bt_gap_le_srv_scan_notify_user(BT_GAP_LE_SRV_EVENT_SCAN_STOPPED);
                    scan_context.flag &= ~BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_USER;
                }
                bt_gap_le_srv_notify_restart_conn_fliter_cmd();
            }
        }
        break;
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_CONN_FILTER_CMD_START: {
            scan_context.flag |= BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_CONN_FILTER_CMD;
        }
        break;
        default:
            break;
    }
}

static void bt_gap_le_scan_state_stopped(bt_gap_le_srv_scan_sm_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (event) {
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_SATRT: {
            bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_ENABLING);
        }
        break;
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_RSL_COMPLETE: {
            if (scan_context.flag & BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_CONN_FILTER_CMD) {
                scan_context.flag &= ~BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_CONN_FILTER_CMD;
            }

            if (scan_context.flag & BT_GAP_LE_SRV_SCAN_FLAG_START_AFTER_CONN_FILTER_CMD) {
                scan_context.flag &= ~BT_GAP_LE_SRV_SCAN_FLAG_START_AFTER_CONN_FILTER_CMD;
                    status = bt_gap_le_srv_set_scan_by_internel(true);
                    if (status == BT_STATUS_SUCCESS) {
                        bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_ENABLING);
                    } else {
                        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] set scan by internal fail", 0);
                    }
                }
            }
        break;
        case BT_GAP_LE_SRV_SCAN_SM_EVENT_RHO_COMPLETE: {
            status = bt_gap_le_srv_set_scan_by_internel(true);
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_srv_scan_update_state(BT_GAP_LE_SRV_SCAN_STATE_ENABLING);
            } else {
                bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] set scan by internal fail, start after rho", 0);
            }
        }
        break;
        default:
            break;
    }
}

static bt_status_t bt_gap_le_srv_set_scan_by_internel(bool is_enable)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (is_enable) {
        if ((bt_os_layer_get_system_tick() < scan_context.param.duration_count) || (scan_context.param.duration_count == 0)) {
            if (scan_context.param.duration_count != 0) {
                scan_context.param.enable.duration = (scan_context.param.duration_count - bt_os_layer_get_system_tick()) / 10;
            }
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan set enable by interval duration = %02x", 1, scan_context.param.enable.duration);
            status = bt_gap_le_set_extended_scan(&scan_context.param.parameter, &scan_context.param.enable);
        } else {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] restart scan set by interval over duration, tick count = %02x, duration_count = %02x", 2,
                                    bt_os_layer_get_system_tick(), scan_context.param.duration_count);
        }
    } else {
        bt_gap_le_srv_set_extended_scan_enable_t enable;
        enable.enable = BT_HCI_DISABLE;
        status = bt_gap_le_set_extended_scan(NULL, &enable);
    }
    return status;
}

static void bt_gap_le_srv_scan_state_machine(bt_gap_le_srv_scan_sm_event_t event, void *parameter)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan state machine in state = %02x, event = %02x", 2, scan_context.state, event);
    g_le_srv_scan_sm_handle[scan_context.state](event, parameter);
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan state machine out state = %02x, event = %02x", 2, scan_context.state, event);
}

static void bt_gap_le_srv_scan_event_notify(bt_gap_le_srv_scan_sm_event_t event, void *parameter)
{
    bt_gap_le_srv_scan_state_machine(event, parameter);
}

static bool bt_gap_le_srv_scan_is_busy(void)
{
    if ((scan_context.state == BT_GAP_LE_SRV_SCAN_STATE_ENABLING) || (scan_context.state == BT_GAP_LE_SRV_SCAN_STATE_ENABLED) ||
        (scan_context.state == BT_GAP_LE_SRV_SCAN_STATE_DISABLING)) {
        return true;
    }
    return false;
}

static bool bt_gap_le_srv_scan_is_stopped(void)
{
    if ((scan_context.state == BT_GAP_LE_SRV_SCAN_STATE_NONE) || (scan_context.state == BT_GAP_LE_SRV_SCAN_STATE_DISABLED)) {
        return true;
    }
    return false;
}

static void bt_gap_le_srv_scan_parameter_save(bt_gap_le_srv_set_extended_scan_parameters_t *param, bt_gap_le_srv_set_extended_scan_enable_t *enable)
{
    bt_utils_memset(&scan_context.param, 0, sizeof(bt_gap_le_srv_scan_parameter_t));
    scan_context.param.parameter.params_phy_1M = NULL;
    scan_context.param.parameter.params_phy_coded = NULL;
    bt_gap_le_srv_memcpy(&scan_context.param.enable, enable, sizeof(bt_gap_le_srv_set_extended_scan_enable_t));
    bt_gap_le_srv_memcpy(&scan_context.param.parameter, param, sizeof(bt_gap_le_srv_set_extended_scan_parameters_t));
    if (enable->duration != 0) {
        scan_context.param.duration_count = (enable->duration * 10) + bt_os_layer_get_system_tick();
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan duration = %02x, duration_count = %02x", 2,
                                enable->duration * 10, scan_context.param.duration_count);
    }
    if (param->params_phy_1M != NULL) {
        bt_gap_le_srv_memcpy(&scan_context.param.params_phy_1M, param->params_phy_1M, sizeof(le_ext_scan_item_t));
        scan_context.param.parameter.params_phy_1M = &scan_context.param.params_phy_1M;
    }
    if (param->params_phy_coded != NULL) {
        bt_gap_le_srv_memcpy(&scan_context.param.params_phy_coded, param->params_phy_coded, sizeof(le_ext_scan_item_t));
        scan_context.param.parameter.params_phy_coded = &scan_context.param.params_phy_coded;
    }
}

bt_status_t bt_gap_le_srv_set_extended_scan(bt_gap_le_srv_set_extended_scan_parameters_t *param, bt_gap_le_srv_set_extended_scan_enable_t *enable, void *callback)
{
    bt_gap_le_srv_mutex_lock();
    bt_status_t status = BT_STATUS_FAIL;
    if (bt_gap_le_srv_scan_is_busy() && (enable->enable == BT_HCI_ENABLE)) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan is busy, state = %02x", 1, scan_context.state);
        bt_gap_le_srv_mutex_unlock();
        return BT_STATUS_BUSY;
    }

    if ((scan_context.flag & BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_CONN_FILTER_CMD) && (enable->enable == BT_HCI_DISABLE)) {
        scan_context.flag |= BT_GAP_LE_SRV_SCAN_FLAG_STOP_OF_USER;
        scan_context.flag &= ~BT_GAP_LE_SRV_SCAN_FLAG_START_AFTER_CONN_FILTER_CMD;
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan disable scan of user when rsl ongoing", 0);
        bt_gap_le_srv_mutex_unlock();
        return BT_STATUS_SUCCESS;
    }

    if ((bt_gap_le_srv_scan_is_stopped()) && (enable->enable == BT_HCI_DISABLE)) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan disable had complete", 0);
        bt_gap_le_srv_mutex_unlock();
        return BT_STATUS_FAIL;
    }

#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
    if (BT_ROLE_HANDOVER_STATE_ONGOING == bt_role_handover_get_state()) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][E] RHO ongoing, please don't start scan!", 0);
        bt_gap_le_srv_mutex_unlock();
        return BT_STATUS_FAIL;
    }
#endif

    if (((le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL) || (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST)) && (enable->enable == BT_HCI_ENABLE)) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan add waiting list, rsl is ongoing", 0);
        bt_gap_le_srv_scan_parameter_save(param, enable);
        scan_context.flag |= BT_GAP_LE_SRV_SCAN_FLAG_START_AFTER_CONN_FILTER_CMD;
        bt_gap_le_srv_mutex_unlock();
        return BT_STATUS_SUCCESS;
    }
    status = bt_gap_le_set_extended_scan(param, enable);
    if (status == BT_STATUS_SUCCESS) {
        if (enable->enable == BT_HCI_ENABLE) {
            bt_gap_le_srv_scan_parameter_save(param, enable);
            bt_gap_le_srv_scan_event_notify(BT_GAP_LE_SRV_SCAN_SM_EVENT_SATRT, NULL);
        } else if (enable->enable == BT_HCI_DISABLE) {
            bt_gap_le_srv_scan_event_notify(BT_GAP_LE_SRV_SCAN_SM_EVENT_STOP, NULL);
        } else {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan enable error", 0);
        }
        scan_context.user_callback = callback;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][I] scan set enable = %02x, status = %02x", 2, enable->enable, status);
    bt_gap_le_srv_mutex_unlock();
    return status;
}

static bt_gap_le_srv_bond_node_t *bt_gap_le_srv_find_bond_wl_node(void)
{
    if (le_bond_context.bond_wl_list.front != NULL) {
        return ((bt_gap_le_srv_bond_node_t *)le_bond_context.bond_wl_list.front);
    }
    return NULL;
}

static void bt_gap_le_srv_bond_wl_start(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_gap_le_srv_bond_node_t *wl_node = NULL;
    if ((wl_node = bt_gap_le_srv_find_bond_wl_node()) != NULL) {
        status = bt_gap_le_bond(wl_node->connection_handle, &wl_node->pairing_config);
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][BOND] find bond waiting list, start bond handle = %2x, status = %02x", 2,
                                wl_node->connection_handle, status);
        if (status == BT_STATUS_SUCCESS) {
            le_bond_context.bonding_handle = wl_node->connection_handle;
        } else {
            le_bond_context.bonding_handle = 0x0000;
        }
        bt_gap_le_srv_linknode_remove_node(&le_bond_context.bond_wl_list, (bt_gap_le_srv_linknode_t *)wl_node);
        bt_gap_le_srv_memory_free((void *)wl_node);
    } else {
        le_bond_context.bonding_handle = 0x0000;
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][BOND] not find bond waiting list", 0);
    }
}

static bt_status_t bt_gap_le_bond_remove_wl_by_handle(bt_handle_t connection_handle)
{
    bt_gap_le_srv_linknode_t *temp_node = (bt_gap_le_srv_linknode_t *)le_bond_context.bond_wl_list.front;
    while (temp_node != NULL) {
        bt_gap_le_srv_bond_node_t *bond_node = (bt_gap_le_srv_bond_node_t *)temp_node;
        if (bond_node->connection_handle == connection_handle) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][BOND] remove node by handle = %02x", 1, connection_handle);
            bt_gap_le_srv_linknode_remove_node((bt_gap_le_srv_linknode_t *)&le_bond_context.bond_wl_list, temp_node);
            /* Free node buffer. */
            bt_gap_le_srv_memory_free(temp_node);
            return BT_STATUS_SUCCESS;
        }
        temp_node = temp_node->front;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][BOND] remove node by handle = %02x not find in waiting list", 1, connection_handle);
    return BT_STATUS_FAIL;
}

static void bt_gap_le_srv_bond_event_notify(bt_handle_t connection_handle, bt_gap_le_srv_bond_event_t event)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][BOND] bond event notify connection_handle:%02x, bonding handle:%02x, event = %02x", 3,
                            connection_handle, le_bond_context.bonding_handle, event);
    switch (event) {
        case BT_GAP_LE_SRV_BOND_COMPLETE: {
            if (le_bond_context.bonding_handle == connection_handle) {
                bt_gap_le_srv_bond_wl_start();
            }
        }
        break;
        case BT_GAP_LE_SRV_BOND_LINK_DISCONNECT: {
            if (le_bond_context.bonding_handle == connection_handle) {
                bt_gap_le_srv_bond_wl_start();
            } else {
                bt_gap_le_bond_remove_wl_by_handle(connection_handle);
            }
        }
        break;
        default:
            break;
    }
}

bt_status_t bt_gap_le_srv_bond(bt_handle_t connection_handle, bt_gap_le_smp_pairing_config_t *pairing_config)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (le_bond_context.bonding_handle == connection_handle) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][BOND] le bonding had exist, bonding_handle = %02x", 1, le_bond_context.bonding_handle);
        return BT_STATUS_CONNECTION_IN_USE;
    }
    if ((le_bond_context.bonding_handle != connection_handle) && (le_bond_context.bonding_handle != 0x0000)) {
        /* Add bond waiting list. */
        bt_gap_le_srv_bond_node_t *wl_node = (bt_gap_le_srv_bond_node_t *)bt_gap_le_srv_memory_alloc(sizeof(bt_gap_le_srv_bond_node_t));
        if (wl_node == NULL) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][BOND] add waiting list alloc fail", 0);
            return BT_STATUS_FAIL;
        }
        wl_node->connection_handle = connection_handle;
        bt_gap_le_srv_memcpy(&wl_node->pairing_config, pairing_config, sizeof(bt_gap_le_smp_pairing_config_t));
        bt_gap_le_srv_linknode_insert_node(&le_bond_context.bond_wl_list, (bt_gap_le_srv_linknode_t *)wl_node, BT_GAP_LE_SRV_NODE_BACK);
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][BOND] add waiting list connection handle = %02x, bonding handle = %02x", 2,
                                connection_handle, le_bond_context.bonding_handle);
        return BT_STATUS_SUCCESS;
    }
    status = bt_gap_le_bond(connection_handle, pairing_config);
    if (status == BT_STATUS_SUCCESS) {
        le_bond_context.bonding_handle = connection_handle;
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][BOND] connection handle = %02x, bond status = %02x", 2, connection_handle, status);
    return status;
}

#ifdef AIR_LE_AUDIO_ENABLE
extern bool bt_le_audio_sink_is_link_valid(bt_handle_t handle);
#endif
uint32_t bt_gap_le_srv_get_address_by_link_type(bt_addr_t *address_list, uint32_t list_num, bool is_local_addr)
{
    BT_GAP_LE_SRV_CHECK_RET_WITH_VALUE_LOG((NULL == address_list),
                                           0, "[BLE_GAP_SRV][E] get address by tyor address_list is NULL", 0);
    bt_gap_le_srv_mutex_lock();
    uint32_t i;
    uint32_t link_count = 0;
    if (is_local_addr) {
        for (i = 0; i < g_le_conn_max; i++) {
            if ((g_le_conn_cntx[i].conn_info.attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NOT_NEED_RHO)
#ifdef AIR_LE_AUDIO_ENABLE
                 || (bt_le_audio_sink_is_link_valid(g_le_conn_cntx[i].connection_handle))
#endif
            ) {
                bt_gap_le_srv_memcpy(&address_list[link_count], &g_le_conn_cntx[i].conn_info.local_addr, sizeof(bt_addr_t));
                link_count++;
                if (link_count == list_num) {
                    break;
                }
            }
        }
    } else {
        for (i = 0; i < g_le_conn_max; i++) {
            if ((g_le_conn_cntx[i].conn_info.attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NOT_NEED_RHO)
#ifdef AIR_LE_AUDIO_ENABLE
                 || (bt_le_audio_sink_is_link_valid(g_le_conn_cntx[i].connection_handle))
#endif
            ) {
                bt_gap_le_srv_memcpy(&address_list[link_count], &g_le_conn_cntx[i].conn_info.peer_addr, sizeof(bt_addr_t));
                link_count++;
                if (link_count == list_num) {
                    break;
                }
            }
        }
    }
    bt_gap_le_srv_mutex_unlock();
    return link_count;
}

static bool bt_gap_le_service_is_allow_connection_filter_cmd(void)
{
    bool status = false;
    uint32_t i = 0;
    /* Check adv is ongoing. */
    for (i = 0; i < g_le_adv_max; i++) {
        if ((g_le_adv_info[i].instance != 0) && (g_le_adv_info[i].state > BT_CM_LE_ADV_STATE_TYPE_STOPPED)) {
            bt_gap_le_srv_report_id("[BLE_GAP_SRV][RSL] adv:%d is onging, state = %02x", 2, i, g_le_adv_info[i].state);
            return status;
        }
    }
    if ((scan_context.state != BT_GAP_LE_SRV_SCAN_STATE_DISABLED) && (scan_context.state != BT_GAP_LE_SRV_SCAN_STATE_NONE)) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][RSL] scan is onging, state = %02x", 1, scan_context.state);
        return status;
    }

    if (bt_gap_le_srv_connect_is_busy()) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][RSL] create connection is onging, state = %02x", 1, g_connect_context.state);
        return status;
    }
    status = true;
    return status;
}

static bt_status_t bt_gap_le_srv_prepare_set_conn_filter_cmd(bt_gap_le_srv_conn_filter_cmd_t cmd_type)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][RSL] prepare set conn filter cmd type = %02x flag = %02x", 2, cmd_type, le_srv_context.flag);
    if (bt_gap_le_service_is_allow_connection_filter_cmd()) {
        return BT_STATUS_SUCCESS;
    }

    switch (cmd_type) {
        case BT_GAP_LE_SRV_CONN_FILTER_CMD_RSL: {
            if ((le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST) || (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL)) {
                le_srv_context.flag |= BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL;
                return BT_STATUS_BUSY;
        }
            le_srv_context.flag |= BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL;
    }
        break;
        case BT_GAP_LE_SRV_CONN_FILTER_CMD_WHITE_LIST: {
            if ((le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST) || (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL)) {
                le_srv_context.flag |= BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST;
    return BT_STATUS_BUSY;
            }
            le_srv_context.flag |= BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST;
        }
        break;
    }

    bt_gap_le_srv_scan_event_notify(BT_GAP_LE_SRV_SCAN_SM_EVENT_CONN_FILTER_CMD_START, NULL);
    bt_gap_le_srv_connect_event_notify(BT_GAP_LE_SRV_CONNECT_EVENT_CONN_FILTER_CMD_START, NULL);
    if ((bt_gap_le_srv_stop_all_adv() == BT_STATUS_SUCCESS) && (!bt_gap_le_srv_scan_is_busy()) && (!bt_gap_le_srv_connect_is_busy())) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][RSL] all adv/scan/create connection is stopped", 0);
        le_srv_context.flag &= ~BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL;
        le_srv_context.flag &= ~BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST;
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_BUSY;
}

bt_status_t bt_gap_le_srv_prepare_set_rsl(void)
{
    bt_gap_le_srv_mutex_lock();
    bt_status_t status = bt_gap_le_srv_prepare_set_conn_filter_cmd(BT_GAP_LE_SRV_CONN_FILTER_CMD_RSL);
    bt_gap_le_srv_mutex_unlock();
    return status;
}

bt_gap_le_srv_error_t bt_gap_le_srv_register_event_callback(bt_gap_le_srv_event_cb_t callback)
{
    uint32_t i = 0;

    for (i = 0; i<BT_GAP_LE_SRV_EVENT_CALLBACK_MAX; i++) {
        if (le_srv_context.event_callback[i] == callback) {
            return BT_GAP_LE_SRV_ERROR_FAIL;
        }
    }

    for (i = 0; i<BT_GAP_LE_SRV_EVENT_CALLBACK_MAX; i++) {
        if (le_srv_context.event_callback[i] == NULL) {
            le_srv_context.event_callback[i] = callback;
            return BT_GAP_LE_SRV_SUCCESS;
        }
    }
    return BT_GAP_LE_SRV_ERROR_NO_MEMORY;
}


bt_gap_le_srv_error_t bt_gap_le_srv_deregister_event_callback(bt_gap_le_srv_event_cb_t callback)
{
    uint32_t i = 0;
    for (i = 0; i<BT_GAP_LE_SRV_EVENT_CALLBACK_MAX; i++) {
        if (le_srv_context.event_callback[i] == callback) {
            return BT_GAP_LE_SRV_SUCCESS;
        }
    }

    return BT_GAP_LE_SRV_ERROR_NOT_FOUND;
}

static void bt_gap_le_srv_event_notify_user(bt_gap_le_srv_event_t event, void *parameter)
{
    uint32_t i = 0;
    bt_gap_le_srv_event_cb_t callback = NULL;
    for (i = 0; i < BT_GAP_LE_SRV_EVENT_CALLBACK_MAX; i++) {
        callback = le_srv_context.event_callback[i];
        if (callback != NULL) {
            callback(event, parameter);
        }
    }
}


static bool bt_gap_le_srv_connect_is_busy(void)
{
    if (g_connect_context.state != BT_GAP_LE_SRV_CONNECT_STATE_NONE) {
        return true;
    }
    return false;
}

static void  bt_gap_le_srv_connect_update_state(bt_gap_le_srv_connect_context_t *context, bt_gap_le_srv_connect_state_t state)
{
    context->state = state;
}

static bt_status_t bt_gap_le_srv_connect_state_none_handle(bt_gap_le_srv_connect_context_t *context, bt_gap_le_srv_connect_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch(event) {
        case BT_GAP_LE_SRV_CONNECT_EVENT_START: {
            status = bt_gap_le_connect((bt_gap_le_srv_create_connection_t *)parameter);
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_srv_memcpy(&context->wl_connect_parameter, parameter, sizeof(bt_gap_le_srv_create_connection_t));
                bt_gap_le_srv_connect_update_state(context, BT_GAP_LE_SRV_CONNECT_STATE_CREATING);
            }
        }
        break;
        case BT_GAP_LE_SRV_CONNECT_EVENT_RSL_COMPLETE: {
            if (BT_GAP_LE_SRV_FLAG_IS_SET(context, BT_GAP_LE_SRV_CONNECT_FLAG_RESTART)) {
                BT_GAP_LE_SRV_REMOVE_FLAG(context, BT_GAP_LE_SRV_CONNECT_FLAG_RESTART);
                /* restart create connect */
                status = bt_gap_le_connect(&context->wl_connect_parameter);
                if (status == BT_STATUS_SUCCESS) {
                    bt_gap_le_srv_connect_update_state(context, BT_GAP_LE_SRV_CONNECT_STATE_CREATING);
                }
            } else {
                status = BT_STATUS_SUCCESS;
            }
        }
        break;
        default:
            break;
    }
    return status;
}

static bt_status_t bt_gap_le_srv_connect_state_creating_handle(bt_gap_le_srv_connect_context_t *context, bt_gap_le_srv_connect_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch(event) {
        case BT_GAP_LE_SRV_CONNECT_EVENT_CREATE_COMPLETE: {
            bt_gap_le_srv_connect_update_state(context, BT_GAP_LE_SRV_CONNECT_STATE_CREATED);
            status = BT_STATUS_SUCCESS;
            if (BT_GAP_LE_SRV_FLAG_IS_SET(context, BT_GAP_LE_SRV_CONNECT_FLAG_RESTART)) {
                /* cancel connection */
                status = bt_gap_le_srv_cancel_connection();
                if (status == BT_STATUS_SUCCESS) {
                    bt_gap_le_srv_connect_update_state(context, BT_GAP_LE_SRV_CONNECT_STATE_CANCELING);
                }
            }
        }
        break;
        case BT_GAP_LE_SRV_CONNECT_EVENT_CREATE_FAIL: {
            bt_gap_le_srv_connect_update_state(context, BT_GAP_LE_SRV_CONNECT_STATE_NONE);
            status = BT_STATUS_SUCCESS;
            if (BT_GAP_LE_SRV_FLAG_IS_SET(context, BT_GAP_LE_SRV_CONNECT_FLAG_RESTART)) {
                bt_gap_le_srv_notify_restart_conn_fliter_cmd();
            }
        }
        break;
        case BT_GAP_LE_SRV_CONNECT_EVENT_CONN_FILTER_CMD_START: {
            BT_GAP_LE_SRV_SET_FLAG(context, BT_GAP_LE_SRV_CONNECT_FLAG_RESTART);
            status = BT_STATUS_SUCCESS;
        }
        break;
        default:
            break;
    }
    return status;
}

static bt_status_t bt_gap_le_srv_connect_state_created_handle(bt_gap_le_srv_connect_context_t *context, bt_gap_le_srv_connect_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch(event) {
        case BT_GAP_LE_SRV_CONNECT_EVENT_COMPLETE: {
            bt_gap_le_srv_connect_update_state(context, BT_GAP_LE_SRV_CONNECT_STATE_NONE);
            status = BT_STATUS_SUCCESS;
        }
        break;
        case BT_GAP_LE_SRV_CONNECT_EVENT_CONN_FILTER_CMD_START: {
            status = bt_gap_le_cancel_connection();
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_srv_connect_update_state(context, BT_GAP_LE_SRV_CONNECT_STATE_CANCELING);
                BT_GAP_LE_SRV_SET_FLAG(context, BT_GAP_LE_SRV_CONNECT_FLAG_RESTART);
            }
        }
        break;
        case BT_GAP_LE_SRV_CONNECT_EVENT_CANCEL_REQUEST: {
            status = bt_gap_le_cancel_connection();
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_srv_connect_update_state(context, BT_GAP_LE_SRV_CONNECT_STATE_CANCELING);
            }
        }
        break;
        default:
            break;
    }
    return status;
}

static bt_status_t bt_gap_le_srv_connect_state_canceling_handle(bt_gap_le_srv_connect_context_t *context, bt_gap_le_srv_connect_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch(event) {
        case BT_GAP_LE_SRV_CONNECT_EVENT_CANCELLED: {
            bt_gap_le_srv_connect_update_state(context, BT_GAP_LE_SRV_CONNECT_STATE_NONE);
            if ((le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_RSL) || (le_srv_context.flag & BT_GAP_LE_SRV_FLAG_PREPARE_SET_WHITE_LIST)) {
                bt_gap_le_srv_notify_restart_conn_fliter_cmd();
            }
            status = BT_STATUS_SUCCESS;
        }
        break;
        case BT_GAP_LE_SRV_CONNECT_EVENT_CANCEL_REQUEST: {
            if (BT_GAP_LE_SRV_FLAG_IS_SET(context, BT_GAP_LE_SRV_CONNECT_FLAG_RESTART)) {
                BT_GAP_LE_SRV_REMOVE_FLAG(context, BT_GAP_LE_SRV_CONNECT_FLAG_RESTART);
            }
            status = BT_STATUS_SUCCESS;
        }
        break;
        default:
            break;
    }
    return status;
}

static bt_status_t bt_gap_le_srv_connect_state_machine(bt_gap_le_srv_connect_event_t event, void *parameter)
{
    bt_gap_le_srv_connect_context_t *context = &g_connect_context;
    bt_status_t status = BT_STATUS_FAIL;
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][CONN] connect state machine in state = %02x, event = %02x", 2, context->state, event);
    status = g_connect_sm_handle[context->state](context, event, parameter);
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][CONN] connect state machine out state = %02x, event = %02x, status = %02x", 3, context->state, event, status);
    return status;
}

static bt_status_t bt_gap_le_srv_connect_event_notify(bt_gap_le_srv_connect_event_t event, void *parameter)
{
    return bt_gap_le_srv_connect_state_machine(event, parameter);
}

bt_status_t bt_gap_le_srv_connect(bt_gap_le_srv_create_connection_t *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_gap_le_srv_mutex_lock();
    status = bt_gap_le_srv_connect_event_notify(BT_GAP_LE_SRV_CONNECT_EVENT_START, (void *)parameter);
    bt_gap_le_srv_mutex_unlock();
    return status;
}

bt_status_t bt_gap_le_srv_cancel_connection(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_gap_le_srv_mutex_lock();
    status = bt_gap_le_srv_connect_event_notify(BT_GAP_LE_SRV_CONNECT_EVENT_CANCEL_REQUEST, NULL);
    bt_gap_le_srv_mutex_unlock();
    return status;
}

static void bt_gap_le_srv_add_white_list(bt_gap_le_set_white_list_op_t op, const bt_addr_t *address, void *callback)
{
    bt_gap_le_srv_white_list_node_t *white_list = (bt_gap_le_srv_white_list_node_t *)bt_gap_le_srv_memory_alloc(sizeof(bt_gap_le_srv_white_list_node_t));

    if (NULL == white_list) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][WHITE] white_list node is NULL", 0);
        return;
    }
    white_list->op = op;
    white_list->callback = callback;
    if (op != BT_GAP_LE_CLEAR_WHITE_LIST) {
        bt_utils_memcpy(&white_list->address, address, sizeof(bt_addr_t));
    }
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][WHITE] add white list = %02x run list, callback = %02x", 2, white_list, callback);
    bt_utils_srv_linknode_insert_node(&le_srv_context.white_list_run, (bt_utils_linknode_t *)white_list, BT_UTILS_SRV_NODE_BACK);
}

static void bt_gap_le_srv_remove_current_white_list(bt_gap_le_srv_white_list_node_t *white_list)
{
    bt_gap_le_srv_report_id("[BLE_GAP_SRV][WHITE] remove white list = %02x", 1, white_list);
    bt_utils_srv_linknode_remove_node(&le_srv_context.white_list_run, (bt_utils_linknode_t *)white_list);
    bt_gap_le_srv_memory_free(white_list);
}

static bt_gap_le_srv_white_list_node_t *bt_gap_le_srv_get_current_white_list(void)
{
    return (bt_gap_le_srv_white_list_node_t*)le_srv_context.white_list_run.front;
}

static void bt_gap_le_srv_run_white_list(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_gap_le_srv_white_list_node_t *white_list = (bt_gap_le_srv_white_list_node_t*)le_srv_context.white_list_run.front;
    while(NULL != white_list) {
        status = bt_gap_le_set_white_list(white_list->op, &white_list->address);
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][WHITE] run white list wl = %02x op = %02x status = %02x", 3, white_list, white_list->op, status);
        white_list = (bt_gap_le_srv_white_list_node_t*)white_list->node.front;
    }
}

bt_status_t bt_gap_le_srv_operate_white_list(bt_gap_le_set_white_list_op_t op, const bt_addr_t *address,  void *callback)
{
    bt_gap_le_srv_mutex_lock();
    bt_status_t status = BT_STATUS_FAIL;
    if((address == NULL) && (BT_GAP_LE_CLEAR_WHITE_LIST != op)) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][WHITE] address is NULL", 0);
        bt_gap_le_srv_mutex_unlock();
        return BT_STATUS_FAIL;
    }
    if (bt_gap_le_srv_prepare_set_conn_filter_cmd(BT_GAP_LE_SRV_CONN_FILTER_CMD_WHITE_LIST) == BT_STATUS_BUSY) {
        bt_gap_le_srv_add_white_list(op, address, callback);
        bt_gap_le_srv_mutex_unlock();
        return BT_STATUS_SUCCESS;
    }
    status = bt_gap_le_set_white_list(op, address);
    if (op == BT_GAP_LE_CLEAR_WHITE_LIST) {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][WHITE] set white list op = %02x status = %02x", 2, op, status);
    } else {
        bt_gap_le_srv_report_id("[BLE_GAP_SRV][WHITE] set white list op = %02x address = %02x:%02x:%02x:%02x:%02x:%02x status = %02x", 8, op,
                                          address->addr[0], address->addr[1], address->addr[2], address->addr[3], address->addr[4],
                                          address->addr[5], status);
    }
    if ((BT_STATUS_SUCCESS == status) || (BT_STATUS_PENDING == status)) {
        bt_gap_le_srv_add_white_list(op, address, callback);
    }
    bt_gap_le_srv_mutex_unlock();
    return status;
}
