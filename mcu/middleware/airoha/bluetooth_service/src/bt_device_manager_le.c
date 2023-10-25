
/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include "bt_device_manager_le_config.h"
#include "bt_device_manager_le.h"
#include "bt_device_manager.h"
#include "bt_callback_manager.h"
#include "hal_trng.h"
#include "bt_system.h"
#include <string.h>
#include "syslog.h"
#include <stdlib.h>
#ifdef AIR_NVDM_ENABLE
#include "nvkey_id_list.h"
#include "nvdm.h"
#include "nvkey.h"
#endif
#include "FreeRTOS.h"
#ifdef MTK_BLE_GAP_SRV_ENABLE
#include "bt_gap_le_service.h"
#include "bt_gap_le_service_utils.h"
#endif
#include "bt_utils.h"
#include "bt_device_manager_nvkey_struct.h"
#include "bt_os_layer_api.h"
#include "bt_device_manager.h"

log_create_module(BT_DM, PRINT_LEVEL_INFO);

#define BT_ADDR_TYPE_UNKNOW         (0xFF)

#define BT_DM_LE_INVAILD_INDEX      0xFF

#define BT_DM_LE_RPA_HASH_LENGTH    3U

#define BT_DM_LE_INVALID_HANDLE     0x0000U

/**
 *  @brief Connection Structure, internal use.
 */
BT_PACKED(
typedef struct {
    bt_handle_t                                       connection_handle; /**< Connection handle. */
    bt_device_manager_le_connection_param_t           connection_params; /**< Connection parameters. */
    bt_addr_t                                         peer_address;
}) bt_device_manager_le_connection_struct_t;

typedef struct {
    bool in_use;
    bt_device_manager_le_bonded_event_callback callback;
} ble_dm_callback_node_t;

#define BT_DM_LE_FLAG_NONE             (0x0000)
#define BT_DM_LE_FLAG_NOT_SYNC         (0x0001)
#define BT_DM_LE_FLAG_BONDED_COMPLETE  (0x0002)
#define BT_DM_LE_FLAG_USING            (0x0004)
#define BT_DM_LE_FLAG_CTKD_CONVERT     (0x0008)
typedef uint16_t bt_dm_le_flag_t;

static ble_dm_callback_node_t ble_dm_cb_list[BT_DEVICE_MANAGER_LE_CB_MAX_NUM] = {{0}};

static bool g_nvram_read_flag = false;
static bool bt_dm_le_initialized = false;
static bool bt_dm_clear_flag = false;
static const bt_addr_t default_bt_addr = {
    .type = BT_ADDR_TYPE_UNKNOW,
    .addr = {0}
};

static bt_bd_addr_t bt_device_manager_le_local_public_addr = {0};
bt_device_manager_le_connection_struct_t dm_info[BT_DEVICE_MANAGER_LE_CONNECTION_MAX] = {{0}};

typedef struct {
    bt_device_manager_le_bonded_info_t bonded_info;
    bt_dm_le_flag_t                    flags;
    bool                               is_connecting;
    bt_hci_privacy_mode_t              privacy_mode;
    bt_gap_le_srv_link_t               link_type;
    bt_handle_t                        connection_handle;
} bt_dm_le_bond_info_context_t;

static bt_dm_le_bond_info_context_t bond_info_context[BT_DEVICE_MANAGER_LE_BONDED_MAX];

typedef uint8_t bt_dm_le_bond_info_order_t;
static bt_dm_le_bond_info_order_t bond_info_order[BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX];

#define BT_DM_LE_NVDM_FLAG_MASK                         (BT_DM_LE_FLAG_NOT_SYNC | BT_DM_LE_FLAG_BONDED_COMPLETE)
#define BT_DM_LE_SET_FLAG(context, flag)                (context->flags |= flag)
#define BT_DM_LE_REMOVE_FLAG(context, flag)             (context->flags &= ~flag)
#define BT_DM_LE_CLEAR_FLAG_BY_INDEX(index)             (bond_info_context[index].flags &= BT_DM_LE_FLAG_NONE)
#define BT_DM_LE_SET_FLAG_BY_INDEX(index, flag)         (bond_info_context[index].flags |= flag)
#define BT_DM_LE_REMOVE_FLAG_BY_INDEX(index, flag)      (bond_info_context[index].flags &= ~flag)
#define BT_DM_LE_GET_FLAG_BY_INDEX(index)               (bond_info_context[index].flags)


#ifdef MTK_BLE_GAP_SRV_ENABLE
#define BT_DM_LE_WL_ACTION_REMOVE        (0x01)
#define BT_DM_LE_WL_ACTION_ADD           (0x02)
typedef uint8_t bt_dm_le_wl_action_t;

typedef struct {
    bt_gap_le_identity_info_t            peer_identity_info;
    bt_gap_le_identity_info_t            local_identity_info;
    bt_gap_le_identity_address_info_t    identity_addr;
} bt_dm_le_wl_rsl_list_info_t;

typedef struct {
    bt_gap_le_srv_linknode_t           node;
    bt_dm_le_wl_action_t               action;
    bt_dm_le_wl_rsl_list_info_t        rsl_list_info;
    bt_hci_privacy_mode_t              privacy_mode;
} bt_dm_le_wl_context_t;

static bt_gap_le_srv_linknode_t bt_dm_wl = {
    .front = NULL,
};
#endif

/** default secure configuration. */
static bool g_sc_only_default = false;
static bt_gap_le_local_config_req_ind_t g_local_config_default;
static bt_gap_le_smp_pairing_config_t g_pairing_config_req_default = {
    .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_NO_BONDING,
    .maximum_encryption_key_size = 16,
};
static bt_gap_le_local_key_t g_local_key_req_default = {
    .encryption_info = {{0}},
    .master_id = {0},
    .identity_info = {{0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x19, 0x28, 0x55, 0x33, 0x68, 0x33, 0x56, 0xde}},
    .signing_info = {{0}}
};


static void bt_device_manager_le_reset_bonded_infos(void);
static void bt_device_manager_le_gen_public_address(void);
static void bt_device_manager_le_reset_connection_infos(void);
static void bt_device_manager_le_save_connection_params(void *buff);
static void bt_device_manager_le_delete_connection_params(void *buff);
static void bt_device_manager_le_event_callback_register(void);
static void bt_device_manager_le_store_bonded_info_to_nvdm(uint8_t index);
#ifdef AIR_NVDM_ENABLE
static bt_status_t bt_device_manager_le_write_nvdm(bt_device_manager_le_bonded_info_t *p_bond_info, uint8_t index);
#endif
static uint8_t bt_device_manager_le_get_index_by_address(bt_addr_t *address);
static void bt_device_manager_le_restore_bonded_info_from_nvdm(uint8_t index);
static void bt_device_manager_le_update_current_conn_interval(void *conn_params);
static bt_gap_le_local_config_req_ind_t *bt_device_manager_le_get_local_config(void);
static bt_gap_le_bonding_info_t *bt_device_manager_le_get_bonding_info(const bt_addr_t remote_addr);
static bt_status_t bt_device_manager_le_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff);
static bt_device_manager_le_bonded_info_t *bt_device_manager_le_get_or_new_bonded_info(const bt_addr_t *peer_addr, bool new_flag);
#ifdef MTK_BLE_GAP_SRV_ENABLE
extern bt_gap_le_srv_link_attribute_t bt_gap_le_srv_get_link_attribute_by_handle(bt_handle_t handle);
extern void bt_gap_le_srv_rsl_update_event_handler(void);
extern bt_status_t bt_gap_le_srv_prepare_set_rsl(void);
extern bt_gap_le_srv_link_attribute_t bt_gap_le_srv_get_link_attribute_by_handle(bt_handle_t handle);
extern bt_status_t bt_gap_le_redirect_bond_info(bt_handle_t connection_handle, bt_gap_le_bonding_info_t *bond_info);
static bt_status_t bt_dm_le_add_waiting_list(bt_dm_le_wl_action_t action, bt_dm_le_bond_info_context_t *context);
static bt_gap_le_srv_error_t bt_dm_le_remove_waiting_list_node(bt_dm_le_wl_context_t *context);
static void bt_dm_le_clear_waiting_list(void);
static bt_status_t bt_dm_le_run_waiting_list(void);
#endif
static bt_status_t bt_dm_le_analysis_rpa(bt_dm_le_bond_info_context_t *context, uint8_t *prand, uint8_t *hash);

#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
#include "bt_role_handover.h"
BT_PACKED(
typedef struct {
    uint8_t connected_dev_num;
}) ble_dm_rho_header_t;

ble_dm_rho_header_t rho_header = {0};

static bt_status_t bt_device_manager_le_rho_allowed_cb(const bt_bd_addr_t *addr)
{
    //Bonding info have sync by bt_app_common part when bonding info changed, so we only RHO connection info part here
    return BT_STATUS_SUCCESS;
}

static uint8_t bt_device_manager_le_rho_get_data_length_cb(const bt_bd_addr_t *addr)
{
    uint8_t i, counter = 0;
#ifdef AIR_MULTI_POINT_ENABLE
    if (addr != NULL) {
        LOG_MSGID_I(BT_DM, "[DM][LE][RHO] get data length addr != NULL for EMP", 0);
        return 0;
    }
#endif
    for (i = 0; i < BT_DEVICE_MANAGER_LE_CONNECTION_MAX; i++) {
        if (dm_info[i].connection_handle) {
            counter++;
        }
    }
    rho_header.connected_dev_num = counter;
    if (rho_header.connected_dev_num) {
        return (sizeof(ble_dm_rho_header_t) + (sizeof(bt_device_manager_le_connection_struct_t) * rho_header.connected_dev_num));
    }
    return 0;
}

static bt_status_t bt_device_manager_le_rho_get_data_cb(const bt_bd_addr_t *addr, void *data)
{
    uint8_t j;
#ifdef AIR_MULTI_POINT_ENABLE
    if (addr != NULL) {
        LOG_MSGID_I(BT_DM, "[DM][LE][RHO] get data addr != NULL for EMP", 0);
        return BT_STATUS_FAIL;
    }
#endif
    if (data && (rho_header.connected_dev_num)) {
        uint8_t index = 0;
        ble_dm_rho_header_t *rho_head = (ble_dm_rho_header_t *)data;
        rho_head->connected_dev_num = rho_header.connected_dev_num;
        bt_device_manager_le_connection_struct_t *rho_cntx = (bt_device_manager_le_connection_struct_t *)(rho_head + 1);
        for (j = 0; ((j < BT_DEVICE_MANAGER_LE_CONNECTION_MAX) && (index < rho_header.connected_dev_num)); j++) {
            if (dm_info[j].connection_handle) {
                bt_utils_memcpy((bt_device_manager_le_connection_struct_t *)(rho_cntx + index), &(dm_info[j]), sizeof(bt_device_manager_le_connection_struct_t));
                index++;
            }
        }
    }
    return BT_STATUS_SUCCESS;
}


static void bt_device_manager_le_rho_status_cb(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status)
{
    uint8_t j;
    switch (event) {
        case BT_ROLE_HANDOVER_COMPLETE_IND: {
            if ((BT_AWS_MCE_ROLE_AGENT == role) && (BT_STATUS_SUCCESS == status)) {
                for (j = 0; j < BT_DEVICE_MANAGER_LE_CONNECTION_MAX ; j++) {
                    bt_utils_memset(&(dm_info[j]), 0x00, sizeof(bt_device_manager_le_connection_struct_t));
                }
                for (j = 0; j < BT_DEVICE_MANAGER_LE_BONDED_MAX; j++) {
                    if (!(BT_DM_LE_GET_FLAG_BY_INDEX(j) & BT_DM_LE_FLAG_NOT_SYNC)) {
                        BT_DM_LE_REMOVE_FLAG_BY_INDEX(j, BT_DM_LE_FLAG_USING);
                    }
                }
                rho_header.connected_dev_num = 0;
            } else if ((BT_AWS_MCE_ROLE_PARTNER == role) && (BT_STATUS_SUCCESS == status)) {
            }
        }
        break;
        default:
            break;
    }
}

static bt_status_t bt_device_manager_le_rho_update_cb(bt_role_handover_update_info_t *info)
{
    uint8_t j;
    if (info && (BT_AWS_MCE_ROLE_PARTNER == info->role)) {
        if ((info->length > 0) && (info->data)) {//copy data to context
            ble_dm_rho_header_t *rho_head = (ble_dm_rho_header_t *)info->data;
            bt_device_manager_le_connection_struct_t *rho_cntx = (bt_device_manager_le_connection_struct_t *)(rho_head + 1);
            for (j = 0; j < rho_head->connected_dev_num; j++) {
                bt_utils_memcpy(&(dm_info[j]), (bt_device_manager_le_connection_struct_t *)(rho_cntx + j), sizeof(bt_device_manager_le_connection_struct_t));
            }
        } else {
            //error log
            return BT_STATUS_FAIL;
        }
    }
    return BT_STATUS_SUCCESS;
}

bt_role_handover_callbacks_t bt_device_manager_le_rho_callbacks = {
    .allowed_cb = bt_device_manager_le_rho_allowed_cb,/*optional if always allowed*/
    .get_len_cb = bt_device_manager_le_rho_get_data_length_cb,  /*optional if no RHO data to partner*/
    .get_data_cb = bt_device_manager_le_rho_get_data_cb,   /*optional if no RHO data to partner*/
    .update_cb = bt_device_manager_le_rho_update_cb,       /*optional if no RHO data to partner*/
    .status_cb = bt_device_manager_le_rho_status_cb /*Mandatory for all users.*/
};

#endif /*__MTK_AWS_MCE_ENABLE__ */

static void bt_device_manager_le_reset_bond_info_order(void)
{
    LOG_MSGID_I(BT_DM, "[DM][LE] reset device manager le bond info order", 0);
    bt_utils_memset(bond_info_order, 0xff, BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX); 
}

#ifdef AIR_NVDM_ENABLE
static void  bt_dm_le_bond_info_order_print(void)
{
    int i;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX; i++){
        LOG_MSGID_I(BT_DM, "[DM][LE] bond_info_order[%d] = bond_info_index_%d ", 2, i, bond_info_order[i]);
    }
}

void bt_device_manager_le_write_nvdm_order_callback(nvkey_status_t status, void *user_data)
{
    LOG_MSGID_I(BT_DM, "[DM][LE] write nvdm order status = %02x ", 1, status);
}

void bt_device_manager_le_write_nvdm_callback(nvkey_status_t status, void *user_data)
{
    LOG_MSGID_I(BT_DM, "[DM][LE] write nvdm bond info index = %02x status = %02x", 2, (uint32_t)user_data, status);
}
#endif

static bt_status_t bt_device_manager_le_write_bond_info_order_nvdm(void)
{
#ifdef AIR_NVDM_ENABLE
    nvkey_status_t write_status;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_write_bond_info_order_nvdm.", 0);
    bt_device_manager_le_db_bonded_info_order_t bond_info_order_db  = {0};
    bt_utils_memcpy(bond_info_order_db.bond_info_order, bond_info_order, BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX);
    
    bt_dm_le_bond_info_order_print();
    write_status = nvkey_write_data_non_blocking(NVID_BT_HOST_LE_BOND_INFO_ORDER, (const uint8_t *)&bond_info_order_db, 
                                                sizeof(bt_device_manager_le_db_bonded_info_order_t), bt_device_manager_le_write_nvdm_order_callback, NULL);
    if (write_status != NVKEY_STATUS_OK) {
        LOG_MSGID_I(BT_DM, "Write bond_info_order nvdm fail", 0);
        return BT_STATUS_FAIL;
    }
    LOG_MSGID_I(BT_DM, "Write bond_info_order nvdm success", 0);
#endif
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_device_manager_le_read_bond_info_order_nvdm(void)
{
#ifdef AIR_NVDM_ENABLE
    nvkey_status_t read_status;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_read_bond_info_order_nvdm.", 0);
    bt_device_manager_le_db_bonded_info_order_t bond_info_order_db;

    uint32_t size = sizeof(bt_device_manager_le_db_bonded_info_order_t);
    read_status = nvkey_read_data(NVID_BT_HOST_LE_BOND_INFO_ORDER, (uint8_t *)&bond_info_order_db, &size);
    if (read_status != NVKEY_STATUS_OK) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Read bond_info_order nvdm fail", 0);
        return BT_STATUS_FAIL;
    }

    bt_utils_memcpy(bond_info_order, bond_info_order_db.bond_info_order, BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX);
    bt_dm_le_bond_info_order_print();
    LOG_MSGID_I(BT_DM, "[DM][LE] Read bond_info_order nvdm success", 0);
#endif
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_device_manager_le_add_bond_info_order(uint8_t index)
{
    int i;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_add_bond_info_order, index = %d", 1, index);

    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX; i++){
        if (bond_info_order[i] == 0xff) {
            bond_info_order[i] = index;
            return BT_STATUS_SUCCESS;
        }
    }

    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_add_bond_info_order, bond_info_order full!", 0);
    return BT_STATUS_FAIL;
}

static bt_status_t bt_device_manager_le_remove_bond_info_order(uint8_t index)
{
    int i, j;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_remove_bond_info_order, index = %d", 1, index);

    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX; i++) {
        if ((bond_info_order[i] != 0xff) &&
            (bond_info_order[i] == index)) {
            
            for (j = i; j < BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX; j++) {
                if (j == BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX - 1) {
                    bond_info_order[j] = 0xff;
                    continue;
                }
                bond_info_order[j] = bond_info_order[j+1];
            }
            return BT_STATUS_SUCCESS;
        }
    }

    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_remove_bond_info_order, index = %d not found!", 1, index);
    return BT_STATUS_FAIL;
}

static bt_status_t bt_device_manager_le_update_bond_info_order(uint8_t index)
{
    int ret;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_update_bond_info_order, index = %d", 1, index);

    bt_device_manager_le_remove_bond_info_order(index);
    ret = bt_device_manager_le_add_bond_info_order(index);
    if (ret != BT_STATUS_SUCCESS) {
        LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_update_bond_info_order, index = %d update fail!", 1, index);
        return ret;
    }

    return ret;
}
#if 0
static uint8_t bt_device_manager_le_get_latest_bonded_info_index(void)
{
    int i;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX; i++){
        if (bond_info_order[i] == 0xff) {
            LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_latest_bonded_info_index, index = %d", 1, bond_info_order[i-1]);
            return bond_info_order[i-1];
        }
    }

    if (i == BT_DEVICE_MANAGER_LE_BONDED_ORDER_MAX && bond_info_order[i] != 0xff) {
        LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_latest_bonded_info_index, index = %d", 1, bond_info_order[i-1]);
        return bond_info_order[i];
    }

    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_latest_bonded_info_index fail", 0);
    return 0xff;
}
#endif
static uint8_t bt_device_manager_le_get_oldest_bonded_info_index(void)
{
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_oldest_bonded_info_index, index = %d", 1, bond_info_order[0]);
    return bond_info_order[0];
}

static void bt_device_manager_le_bonded_event_cb(bt_device_manager_le_bonded_event_t event, bt_addr_t *address)
{
    uint8_t i = 0;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_CB_MAX_NUM; i++) {
        if (ble_dm_cb_list[i].in_use && ble_dm_cb_list[i].callback != NULL) {
            ble_dm_cb_list[i].callback(event, address);
        }
    }
}

static bt_status_t bt_device_manager_le_cb_register_int(bt_device_manager_le_bonded_event_callback callback)
{
    uint8_t i = 0;
    bt_status_t status = 0;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_CB_MAX_NUM; i++) {
        if (!ble_dm_cb_list[i].in_use) {
            ble_dm_cb_list[i].callback = callback;
            ble_dm_cb_list[i].in_use = true;
            break;
        }
    }
    if (i == BT_DEVICE_MANAGER_LE_CB_MAX_NUM) {
        LOG_MSGID_I(BT_DM, "all are in use, please extend the value of BT_DEVICE_MANAGER_LE_CB_MAX_NUM!", 0);
        status = BT_STATUS_FAIL;
    }
    return status;
}

static bt_status_t bt_device_manager_le_cb_deregister_int(bt_device_manager_le_bonded_event_callback callback)
{
    uint8_t i = 0;
    bt_status_t status = 0;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_CB_MAX_NUM; i++) {
        if (ble_dm_cb_list[i].in_use && ble_dm_cb_list[i].callback == callback) {
            ble_dm_cb_list[i].callback = NULL;
            ble_dm_cb_list[i].in_use = false;
            break;
        }
    }
    if (i == BT_DEVICE_MANAGER_LE_CB_MAX_NUM) {
        LOG_MSGID_I(BT_DM, "deregister fail, because of not find the callback", 0);
        status = BT_STATUS_FAIL;
    }
    return status;
}

void bt_device_manager_le_init(void)
{
    if (bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "Others application had initialized BT DM LE module", 0);
        return;
    } else {
        bt_dm_le_initialized = true;
        bt_device_manager_le_reset_bonded_infos();
        bt_device_manager_le_reset_bond_info_order();
        bt_device_manager_le_reset_connection_infos();
        bt_device_manager_le_event_callback_register();
        if (!g_nvram_read_flag) {
            g_nvram_read_flag = true;
            bt_device_manager_le_restore_bonded_info_from_nvdm(0);
            bt_device_manager_le_read_bond_info_order_nvdm();
        }
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
        bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_BLE_DM, &bt_device_manager_le_rho_callbacks);
#endif
    }
}

static void bt_device_manager_le_event_callback_register(void)
{
    bt_callback_manager_register_callback(bt_callback_type_gap_le_get_bonding_info, 0, (void *)bt_device_manager_le_get_bonding_info);
    bt_callback_manager_register_callback(bt_callback_type_gap_le_get_local_cofig, 0, (void *)bt_device_manager_le_get_local_config);
    bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_SYSTEM | MODULE_MASK_GAP, (void *)bt_device_manager_le_event_callback);
}

static bt_status_t bt_dm_le_delete_device_from_resolving_list(bt_dm_le_bond_info_context_t *context)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_gap_le_bonding_info_t *bonded_info = &context->bonded_info.info;
    if ((BT_ADDR_TYPE_UNKNOW != bonded_info->identity_addr.address.type) &&
        (0 != bt_utils_memcmp(&(default_bt_addr.addr), &(bonded_info->identity_addr.address.addr), BT_BD_ADDR_LEN))) {
        if (bt_gap_le_srv_prepare_set_rsl() == BT_STATUS_BUSY) {
            bt_dm_le_add_waiting_list(BT_DM_LE_WL_ACTION_REMOVE, context);
            return BT_STATUS_SUCCESS;
        }
        bt_hci_cmd_le_remove_device_from_resolving_list_t dev;
        dev.peer_identity_address = bonded_info->identity_addr.address;
        status = bt_gap_le_set_resolving_list(BT_GAP_LE_REMOVE_FROM_RESOLVING_LIST, (void *)&dev);
        if ((BT_STATUS_SUCCESS != status) && (BT_STATUS_PENDING != status)) {
            LOG_MSGID_I(BT_DM, "[DM][LE] delete bond device:%02x from rsl fail status = %02x", 2, bonded_info, status);
            return status;
        }
    }
    LOG_MSGID_I(BT_DM, "[DM][LE] delete bond device:%02x from rsl list status = %02x", 2, bonded_info, status);
    return status;
}

static bt_status_t bt_dm_le_add_device_to_resolving_list(bt_dm_le_bond_info_context_t *context)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_gap_le_bonding_info_t *bonded_info = &context->bonded_info.info;
    if ((BT_ADDR_TYPE_UNKNOW != bonded_info->identity_addr.address.type) &&
        (0 != bt_utils_memcmp(&(default_bt_addr.addr), &(bonded_info->identity_addr.address.addr), BT_BD_ADDR_LEN))) {
        bt_hci_cmd_le_add_device_to_resolving_list_t dev;
        dev.peer_identity_address = bonded_info->identity_addr.address;
        LOG_MSGID_I(BT_DM, "[DM][LE] add rsl list peer identity address: %02X:%02X:%02X:%02X:%02X:%02X, type is %d", 7,
                    dev.peer_identity_address.addr[5],
                    dev.peer_identity_address.addr[4],
                    dev.peer_identity_address.addr[3],
                    dev.peer_identity_address.addr[2],
                    dev.peer_identity_address.addr[1],
                    dev.peer_identity_address.addr[0],
                    dev.peer_identity_address.type
                   );
        if (bt_gap_le_srv_prepare_set_rsl() == BT_STATUS_BUSY) {
            bt_dm_le_add_waiting_list(BT_DM_LE_WL_ACTION_ADD, context);
            return BT_STATUS_SUCCESS;
        }
        bt_utils_memcpy(dev.peer_irk, &(bonded_info->identity_info), sizeof(dev.peer_irk));
        bt_utils_memcpy(dev.local_irk, &(bonded_info->local_key.identity_info), sizeof(dev.local_irk));
        status = bt_gap_le_set_resolving_list(BT_GAP_LE_ADD_TO_RESOLVING_LIST, (void *)&dev);
        if ((BT_STATUS_SUCCESS != status) && (BT_STATUS_PENDING != status)) {
            LOG_MSGID_I(BT_DM, "[DM][LE] add bond device:%02x to rsl fail status = %02x", 2, bonded_info, status);
            return status;
        }
        status = bt_gap_le_set_address_resolution_enable(true);
        if ((BT_STATUS_SUCCESS != status) && (BT_STATUS_PENDING != status)) {
            LOG_MSGID_I(BT_DM, "[DM][LE] add bond device:%02x to enable resolution fail status = %02x", 2, bonded_info, status);
            return status;
        }
        bt_hci_cmd_le_set_privacy_mode_t parameter;
        parameter.privacy_mode = context->privacy_mode;
        bt_utils_memcpy(&parameter.peer_address, &bonded_info->identity_addr.address, sizeof(bt_addr_t));
        if (parameter.peer_address.type >= BT_ADDR_PUBLIC_IDENTITY) {
            parameter.peer_address.type = parameter.peer_address.type - BT_ADDR_PUBLIC_IDENTITY;
        }
        status = bt_gap_le_set_privacy_mode(&parameter);
        if ((BT_STATUS_SUCCESS != status) && (BT_STATUS_PENDING != status)) {
            LOG_MSGID_I(BT_DM, "[DM][LE] add bond device:%02x sert privacy mode fail status = %02x", 2, bonded_info, status);
            return status;
        }
    }
    LOG_MSGID_I(BT_DM, "[DM][LE] add bond device:%02x to rsl success, status = %02x", 2, bonded_info, status);
    return status;
}

static bt_status_t bt_dm_le_add_device_to_resolving_list_by_index(uint8_t index)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_device_manager_le_bonded_info_t *p_bond_info;
    p_bond_info = &(bond_info_context[index].bonded_info);
    if (0 == bt_utils_memcmp(&default_bt_addr, &(p_bond_info->bt_addr), sizeof(bt_addr_t))) {
        LOG_MSGID_I(BT_DM, "[DM][LE] add device to rsl list by index fail", 0);
    } else {
        if ((BT_ADDR_TYPE_UNKNOW != p_bond_info->info.identity_addr.address.type) &&
            (0 != bt_utils_memcmp(&(default_bt_addr.addr), &(p_bond_info->info.identity_addr.address.addr), BT_BD_ADDR_LEN))) {
            status = bt_dm_le_add_device_to_resolving_list(&(bond_info_context[index]));
            LOG_MSGID_I(BT_DM, "[DM][LE] add device to rsl list by index = %d status = %02x", 2, index, status);
            return status;
        } else {
            LOG_MSGID_I(BT_DM, "[DM][LE] add device to rsl list by index for identity addr error", 0);
        }
    }
    return status;
}

static void bt_dm_le_reset_context(bt_dm_le_bond_info_context_t *context)
{
    bt_utils_memset(context, 0x00, sizeof(bt_dm_le_bond_info_context_t));
    context->bonded_info.bt_addr.type = BT_ADDR_TYPE_UNKNOW;
    context->bonded_info.info.identity_addr.address.type = BT_ADDR_TYPE_UNKNOW;
    context->privacy_mode = BT_HCI_PRIVACY_MODE_DEVICE;
}

static bt_status_t bt_device_manager_le_bond_info_remove_by_index(uint8_t index)
{
    if (index >= BT_DEVICE_MANAGER_LE_BONDED_MAX) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Remove bonded info index over max = %d", 1, index);
        return BT_STATUS_FAIL;
    }
    bt_device_manager_le_bonded_info_t *p_bond_info = &bond_info_context[index].bonded_info;
#ifdef MTK_BLE_GAP_SRV_ENABLE
    bt_handle_t connection_handle = bt_gap_le_srv_get_conn_handle_by_addr(&(p_bond_info->bt_addr), false);
    LOG_MSGID_I(BT_DM, "[DM][LE] Remove bonded info connection handle = %02x", 1, connection_handle);
    if (!(bt_gap_le_srv_get_link_attribute_by_handle(connection_handle) & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NOT_NEED_RHO)) {
        bt_device_manager_le_bonded_event_cb(BT_DEVICE_MANAGER_LE_BONDED_REMOVE, &p_bond_info->bt_addr);
    } else {
        LOG_MSGID_I(BT_DM, "[DM][LE] not need sync bond info to agent", 0);
    }
#else
    bt_device_manager_le_bonded_event_cb(BT_DEVICE_MANAGER_LE_BONDED_REMOVE, &p_bond_info->bt_addr);
#endif
    bt_dm_le_delete_device_from_resolving_list(&bond_info_context[index]);
    bt_dm_le_reset_context(&bond_info_context[index]);
    LOG_MSGID_I(BT_DM, "[DM][LE] Remove bonded info by index  %d", 1, index);
    bt_device_manager_le_store_bonded_info_to_nvdm(index);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_device_manager_le_bond_info_duplicate_remove(uint8_t index)
{
    uint8_t i = 0;
    bt_device_manager_le_bonded_info_t *new_bond_info = NULL;
    bt_device_manager_le_bonded_info_t *old_bond_info = NULL;
    new_bond_info = &(bond_info_context[index].bonded_info);
    bt_bd_addr_t *old_remote_address = NULL;
    LOG_MSGID_I(BT_DM, "[DM][LE]bond info is duplicate index = %d", 1, index);

    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        old_bond_info = &(bond_info_context[i].bonded_info);
        old_remote_address = (bt_bd_addr_t *)&old_bond_info->info.identity_addr.address.addr;
        if (((0 == bt_utils_memcmp(&(new_bond_info->info.identity_addr.address.addr), old_remote_address, BT_BD_ADDR_LEN)) ||
             (0 == bt_utils_memcmp(&(new_bond_info->bt_addr.addr), &(old_bond_info->bt_addr.addr), BT_BD_ADDR_LEN))) &&
             (i != index) && (0 != bt_utils_memcmp(&(default_bt_addr.addr), old_remote_address, BT_BD_ADDR_LEN))) {
            LOG_MSGID_I(BT_DM, "[DM][LE]duplicate remove info index = %02x,address: %02X:%02X:%02X:%02X:%02X:%02X,type %d", 8,
                        i,
                        old_bond_info->bt_addr.addr[5],
                        old_bond_info->bt_addr.addr[4],
                        old_bond_info->bt_addr.addr[3],
                        old_bond_info->bt_addr.addr[2],
                        old_bond_info->bt_addr.addr[1],
                        old_bond_info->bt_addr.addr[0],
                        old_bond_info->bt_addr.type
                       );
            bt_device_manager_le_bond_info_remove_by_index(i);
            bt_device_manager_le_remove_bond_info_order(i);
            break;
        }
    }
    if (i == BT_DEVICE_MANAGER_LE_BONDED_MAX) {
        LOG_MSGID_I(BT_DM, "[DM][LE] bond info not have duplicate remove", 0);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_device_manager_le_update_resolving_list(void)
{
    bt_status_t wl_status = bt_dm_le_run_waiting_list();
    LOG_MSGID_I(BT_DM, "[DM][LE] start rsl status = %02x after le operation complete", 1, wl_status);
    return BT_STATUS_SUCCESS;
}

#ifdef AIR_NVDM_ENABLE

static void bt_dm_le_nvdm_info_print(bt_device_manager_le_bonded_info_t *p_bond_info)
{
    bt_addr_t *remote_address = &p_bond_info->bt_addr;
    LOG_MSGID_I(BT_DM, "[DM][LE] Read bond info peer address type = %02x, addr:%02x:%02x:%02x:%02x:%02x:%02x", 7, remote_address->type,
                remote_address->addr[0],
                remote_address->addr[1],
                remote_address->addr[2],
                remote_address->addr[3],
                remote_address->addr[4],
                remote_address->addr[5]);
    remote_address = &p_bond_info->info.identity_addr.address;
    LOG_MSGID_I(BT_DM, "[DM][LE] read bond info IDA address type = %02x, addr:%02x:%02x:%02x:%02x:%02x:%02x", 7, remote_address->type,
                remote_address->addr[0],
                remote_address->addr[1],
                remote_address->addr[2],
                remote_address->addr[3],
                remote_address->addr[4],
                remote_address->addr[5]);
    uint8_t *key_ptr = (uint8_t *)&p_bond_info->info.local_key.encryption_info.ltk;
    LOG_MSGID_I(BT_DM, "[DM][LE] local ltk:%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x", 16,
                key_ptr[0], key_ptr[1], key_ptr[2], key_ptr[3],
                key_ptr[4], key_ptr[5], key_ptr[6], key_ptr[7],
                key_ptr[8], key_ptr[9], key_ptr[10], key_ptr[11],
                key_ptr[12], key_ptr[13], key_ptr[14], key_ptr[15]);
    key_ptr = (uint8_t *)&p_bond_info->info.encryption_info.ltk;
    LOG_MSGID_I(BT_DM, "[DM][LE] remote ltk:%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x", 16,
                key_ptr[0], key_ptr[1], key_ptr[2], key_ptr[3],
                key_ptr[4], key_ptr[5], key_ptr[6], key_ptr[7],
                key_ptr[8], key_ptr[9], key_ptr[10], key_ptr[11],
                key_ptr[12], key_ptr[13], key_ptr[14], key_ptr[15]);
    key_ptr = (uint8_t *)&p_bond_info->info.identity_info.irk;
    LOG_MSGID_I(BT_DM, "[DM][LE] remote IRK:%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x", 16,
                key_ptr[0], key_ptr[1], key_ptr[2], key_ptr[3],
                key_ptr[4], key_ptr[5], key_ptr[6], key_ptr[7],
                key_ptr[8], key_ptr[9], key_ptr[10], key_ptr[11],
                key_ptr[12], key_ptr[13], key_ptr[14], key_ptr[15]);
    key_ptr = (uint8_t *)&p_bond_info->info.local_key.identity_info.irk;
    LOG_MSGID_I(BT_DM, "[DM][LE] local IRK:%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x", 16,
                key_ptr[0], key_ptr[1], key_ptr[2], key_ptr[3],
                key_ptr[4], key_ptr[5], key_ptr[6], key_ptr[7],
                key_ptr[8], key_ptr[9], key_ptr[10], key_ptr[11],
                key_ptr[12], key_ptr[13], key_ptr[14], key_ptr[15]);
}

static bt_status_t bt_device_manager_le_write_nvdm(bt_device_manager_le_bonded_info_t *p_bond_info, uint8_t index)
{
    nvkey_status_t write_status = NVKEY_STATUS_ERROR;
    uint16_t bond_info_nvkey_id = NVID_BT_HOST_LE_BOND_INFO_1 + index;
    bt_device_manager_le_db_bonded_info_t bond_info_nvdm;
    bt_utils_memset(&bond_info_nvdm, 0, sizeof(bt_device_manager_le_db_bonded_info_t));
    bt_utils_memcpy(&bond_info_nvdm.bond_info, p_bond_info, sizeof(bt_device_manager_le_bonded_info_t));
    bt_utils_memcpy(&bond_info_nvdm.link_type, &bond_info_context[index].link_type, sizeof(bt_gap_le_srv_link_t));
    bond_info_nvdm.flag = BT_DM_LE_GET_FLAG_BY_INDEX(index) & BT_DM_LE_NVDM_FLAG_MASK;
    LOG_MSGID_I(BT_DM, "[DM][LE] Write nvdm index = %d, flag = %02x.", 2, index, bond_info_nvdm.flag);

    bt_dm_le_nvdm_info_print(p_bond_info);
    LOG_MSGID_I(BT_DM, "[DM][LE] bond_info_context[%d].link_type = 0x%04x", 2, index, bond_info_context[index].link_type);

    uint32_t nvdm_index = (uint32_t)index;
    write_status = nvkey_write_data_non_blocking(bond_info_nvkey_id, (const uint8_t *)&bond_info_nvdm, 
                                                sizeof(bt_device_manager_le_db_bonded_info_t), bt_device_manager_le_write_nvdm_callback, (const void *)nvdm_index);
    if (write_status != NVKEY_STATUS_OK) {
        LOG_MSGID_I(BT_DM, "Write nvdm fail, bonded_info[%d], write nvdm status = %d.", 2, index, write_status);
    }
    LOG_MSGID_I(BT_DM, "Write nvdm success, bonded_info[%d].", 1, index);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_device_manager_le_read_nvdm(bt_device_manager_le_bonded_info_t *p_bond_info, uint8_t index)
{
    nvkey_status_t read_status = NVKEY_STATUS_ERROR;
    if (p_bond_info == NULL) {
        return BT_STATUS_FAIL;
    }
    uint32_t size = sizeof(bt_device_manager_le_db_bonded_info_t);
    bt_device_manager_le_db_bonded_info_t bond_info_nvdm;
    uint16_t bond_info_nvkey_id = NVID_BT_HOST_LE_BOND_INFO_1 + index;
    read_status = nvkey_read_data(bond_info_nvkey_id, (uint8_t *)&bond_info_nvdm, &size);
    if (read_status != NVKEY_STATUS_OK) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Read nvdm fail, bonded_info[%d], read nvdm status = %d.", 2, index, read_status);
        return BT_STATUS_FAIL;
    }
    bt_utils_memcpy(p_bond_info, &bond_info_nvdm.bond_info, sizeof(bt_device_manager_le_bonded_info_t));
    bt_utils_memcpy(&bond_info_context[index].link_type, &bond_info_nvdm.link_type, sizeof(bt_gap_le_srv_link_t));
    if (0 == bt_utils_memcmp(&default_bt_addr, &(p_bond_info->bt_addr), sizeof(default_bt_addr))) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Read all nvdm success, but bonded_info[%d] addr is NULL.", 1, index);
    }

    bt_dm_le_nvdm_info_print(p_bond_info);
    LOG_MSGID_I(BT_DM, "[DM][LE] bond_info_context[%d].link_type = 0x%04x", 2, index, bond_info_context[index].link_type);
    LOG_MSGID_I(BT_DM, "[DM][LE] Read nvdm success, index = %d, flag = %02x.", 2, index, bond_info_nvdm.flag);
    BT_DM_LE_SET_FLAG_BY_INDEX(index, bond_info_nvdm.flag);
    
    return BT_STATUS_SUCCESS;
}
#endif

static void bt_device_manager_le_set_link_type_by_handle(uint8_t bond_info_index, bt_handle_t conn_handle)
{
    if (conn_handle == BT_HANDLE_INVALID) {
        return;
    }
    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(conn_handle);
    if (conn_info == NULL) {
        LOG_MSGID_I(BT_DM, "[DM][LE] set link type conn is NULL", 0);
        return;
    }
    bond_info_context[bond_info_index].link_type |= conn_info->link_type;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_set_link_type_by_handle, bond_info_index:%d, conn_handle:%d, link_type:0x%4x", 3, bond_info_index, conn_handle, conn_info->link_type);
}

bt_status_t bt_device_manager_le_set_link_type_by_addr(bt_addr_t *address, bt_gap_le_srv_link_t link_type)
{
    uint8_t i;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_set_link_type_by_addr", 0);

    i = bt_device_manager_le_get_index_by_address(address);
    bond_info_context[i].link_type = link_type;
    LOG_MSGID_I(BT_DM, "[DM][LE] set link type in bonded_info[%d] success", 1, i);
#ifdef AIR_NVDM_ENABLE
    bt_device_manager_le_write_nvdm(&bond_info_context[i].bonded_info, i);
#endif
    return BT_STATUS_SUCCESS;
}

bt_gap_le_srv_link_t bt_device_manager_le_get_link_type_by_addr(bt_bd_addr_t *remote_addr)
{
    uint8_t i;
    bt_gap_le_srv_link_t link_type;
    bt_device_manager_le_bonded_info_t *p_bond_info = NULL;
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Device bonded status check fail, please init BT DM LE first!", 0);
        return BT_GAP_LE_SRV_LINK_TYPE_NONE;
    }
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        p_bond_info = &(bond_info_context[i].bonded_info);
        if (0 == bt_utils_memcmp(remote_addr, &(p_bond_info->bt_addr.addr), sizeof(bt_bd_addr_t))) {
            bt_utils_memcpy(&link_type, &bond_info_context[i].link_type, sizeof(bt_gap_le_srv_link_t));
            LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_link_type_by_addr, bond_info_context:%d, link_type:0x%04x", 2, i, link_type);
            return link_type;
        } else if (0 == bt_utils_memcmp(remote_addr, &(p_bond_info->info.identity_addr.address.addr), sizeof(bt_bd_addr_t))) {
            bt_utils_memcpy(&link_type, &bond_info_context[i].link_type, sizeof(bt_gap_le_srv_link_t));
            LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_link_type_by_addr, bond_info_context:%d, link_type:0x%04x", 2, i, link_type);
            return link_type;
        }
    }

    if (i == BT_DEVICE_MANAGER_LE_BONDED_MAX) {
        LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_link_type_by_addr, not found bond info", 0);
    }
    return BT_GAP_LE_SRV_LINK_TYPE_NONE;
}

static void bt_dm_le_add_all_device_to_rsl_list(void)
{
    uint32_t i = 0;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        bt_dm_le_add_device_to_resolving_list_by_index(i);
    }
}

static void bt_dm_le_redirect_link_bond_info(bt_dm_le_bond_info_context_t *context)
{
    uint8_t hash[BT_DM_LE_RPA_HASH_LENGTH] = {0};
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t *address_prand = NULL;
    uint8_t *address_hash = NULL;
    LOG_MSGID_I(BT_DM, "[DM][LE] redirect link bond info context = %02x", 1, context);
    /* check whether all links correspond to the bond info */
    for (uint32_t i = 0; i < BT_DEVICE_MANAGER_LE_CONNECTION_MAX; i++) {
        bt_device_manager_le_connection_struct_t *le_connection = &dm_info[i];
        if (le_connection->connection_handle != 0) {
            address_prand = (uint8_t *)((uint8_t *)&le_connection->peer_address.addr + 3);
            address_hash = (uint8_t *)&le_connection->peer_address.addr;
            status = bt_dm_le_analysis_rpa(context, address_prand, hash);
            LOG_MSGID_I(BT_DM, "[DM][LE] analysis hash:", 0);
            for (uint32_t i = 0; i < BT_DM_LE_RPA_HASH_LENGTH; i++) {
                LOG_MSGID_I(BT_DM, "[DM][LE] %02x", 1, hash[i]);
            }
            LOG_MSGID_I(BT_DM, "[DM][LE] hash address:", 0);
            for (uint32_t i = 0; i < BT_DM_LE_RPA_HASH_LENGTH; i++) {
                LOG_MSGID_I(BT_DM, "[DM][LE] %02x", 1, address_hash[i]);
            }
            if ((status == BT_STATUS_SUCCESS) && (bt_utils_memcmp(hash, address_hash, BT_DM_LE_RPA_HASH_LENGTH) == 0)) {
                /* redirection LE link bond info */
                LOG_MSGID_I(BT_DM, "[DM][LE] redirect link connection handle = %02x", 1, le_connection->connection_handle);
                bt_gap_le_redirect_bond_info(le_connection->connection_handle, &context->bonded_info.info);
                break;
            }
        }
    }
}

static bt_dm_le_bond_info_context_t *bt_dm_le_find_context_by_handle(bt_handle_t connection_handle)
{
    uint32_t i = 0;
    bt_dm_le_bond_info_context_t *context = NULL;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        if (bond_info_context[i].connection_handle == connection_handle) {
            context = &bond_info_context[i];
            break;
        }
    }
    LOG_MSGID_I(BT_DM, "[DM][LE] find context = %02x index = %02x by handle = %02x", 3, context, i, connection_handle);
    return context;
}

static uint8_t bt_dm_le_get_index_by_context(bt_dm_le_bond_info_context_t *context)
{
    uint32_t i = 0;
    if (context == NULL) {
        return BT_DM_LE_INVAILD_INDEX;
    }

    while (i < BT_DEVICE_MANAGER_LE_BONDED_MAX) {
        if (&bond_info_context[i] == context) {
            break;
        }
        i++;
    };

    if (i == BT_DEVICE_MANAGER_LE_BONDED_MAX) {
        i = BT_DM_LE_INVAILD_INDEX;
    }

    LOG_MSGID_I(BT_DM, "[DM][LE] find index = %02x by context = %02x", 2, i, context);
    return (uint8_t)i;
}

static bt_status_t bt_device_manager_le_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_POWER_ON_CNF: {
            bt_dm_le_add_all_device_to_rsl_list();
        }
        break;
        case BT_POWER_OFF_CNF: {
            bt_dm_le_clear_waiting_list();
        }
        break;
        case BT_GAP_LE_SET_RESOLVING_LIST_CNF: {
            LOG_MSGID_I(BT_DM, "[DM][LE] resolving list set complete status = %02x", 1, status);
            bt_status_t wl_status = bt_dm_le_run_waiting_list();
            if (BT_STATUS_FAIL == wl_status) {
#ifdef MTK_BLE_GAP_SRV_ENABLE
                bt_gap_le_srv_rsl_update_event_handler();
#endif
            }
        }
        break;
        case BT_GAP_LE_CONNECT_IND: {
            const bt_gap_le_connection_ind_t *connect_ind = (bt_gap_le_connection_ind_t *)buff;
            bt_device_manager_le_save_connection_params(buff);
            LOG_MSGID_I(BT_DM, "[DM][LE] connected, connection role = %d", 1, connect_ind->role);
        }
        break;
        case BT_GAP_LE_CONNECTION_UPDATE_IND:
            bt_device_manager_le_update_current_conn_interval(buff);
            break;
        case BT_GAP_LE_DISCONNECT_IND: {
            bt_device_manager_le_delete_connection_params(buff);
        }
        break;
        case BT_GAP_LE_BONDING_COMPLETE_IND: {
            bt_gap_le_bonding_complete_ind_t *ind = (bt_gap_le_bonding_complete_ind_t *)buff;
            LOG_MSGID_I(BT_DM, "[DM][LE] Bond complete, conn_handle(0x%04x)status is (0x%04x)", 2, ind->handle, status);
            if ((ind->handle == BT_HANDLE_INVALID) && (BT_STATUS_SUCCESS == status)) {
                /* Get new EDR connect address. */
                bt_bd_addr_t *bd_adddress = bt_device_manager_remote_get_dev_by_seq_num(1);
                if (bd_adddress == NULL) {
                    LOG_MSGID_I(BT_DM, "[DM][LE] get newest EDR connect address fail", 0);
                }
                bt_addr_t remote_address = {
                    .type = BT_ADDR_PUBLIC,
                };
                bt_utils_memcpy(&remote_address.addr, bd_adddress, sizeof(bt_bd_addr_t));
                uint8_t *p_remote_address = (uint8_t *)&remote_address.addr;
                LOG_MSGID_I(BT_DM, "[DM][LE] get public address:%02x:%02x:%02x:%02x:%02x:%02x", 6, p_remote_address[0], p_remote_address[1],
                                        p_remote_address[2], p_remote_address[3], p_remote_address[4], p_remote_address[5]);
                uint8_t i = bt_device_manager_le_get_index_by_address(&remote_address);
                if (i == BT_DM_LE_INVAILD_INDEX) {
                    break;
                }
#ifdef MTK_AWS_MCE_ENABLE
                if (bt_utils_memcmp(bt_device_manager_get_local_address(), bt_device_manager_aws_local_info_get_fixed_address(), sizeof(bt_bd_addr_t)) == 0) {
                    BT_DM_LE_SET_FLAG_BY_INDEX(i, BT_DM_LE_FLAG_NOT_SYNC);
                    bt_device_manager_le_bond_info_duplicate_remove(i);
                    bt_dm_le_add_device_to_resolving_list(&(bond_info_context[i]));
                } else {
                    BT_DM_LE_REMOVE_FLAG_BY_INDEX(i, BT_DM_LE_FLAG_NOT_SYNC);
                }
#else
                BT_DM_LE_SET_FLAG_BY_INDEX(i, BT_DM_LE_FLAG_NOT_SYNC);
                bt_device_manager_le_bond_info_duplicate_remove(i);
                bt_dm_le_add_device_to_resolving_list(&(bond_info_context[i]));
#endif
                /* link key->LTK, save nvdm. */
                BT_DM_LE_SET_FLAG_BY_INDEX(i, BT_DM_LE_FLAG_BONDED_COMPLETE);

                BT_DM_LE_SET_FLAG_BY_INDEX(i, BT_DM_LE_FLAG_CTKD_CONVERT);

                bt_dm_le_redirect_link_bond_info(&bond_info_context[i]);
                bt_device_manager_le_update_bond_info_order(i);
                bt_device_manager_le_write_bond_info_order_nvdm();
#ifdef AIR_NVDM_ENABLE
                bt_device_manager_le_write_nvdm(&bond_info_context[i].bonded_info, i);
#endif
                bt_dm_le_add_device_to_resolving_list(&(bond_info_context[i]));
                bt_device_manager_le_bonded_event_cb(BT_DEVICE_MANAGER_LE_BONDED_ADD, &bond_info_context[i].bonded_info.bt_addr);
                break;
            }
            bt_dm_le_bond_info_context_t *context = bt_dm_le_find_context_by_handle(ind->handle);
            if (NULL != context) {
                uint8_t i = 255;
                bt_device_manager_le_bonded_info_t *bond_info = &context->bonded_info;
                if (BT_STATUS_SUCCESS != status) {
                    if (0x06 == status) {// means key or pin missing
                        LOG_MSGID_I(BT_DM, "[DM][LE] Bond fail because remote device key missing, conn_handle(0x%04x)", 1, ind->handle);
                        bt_gap_le_bond(ind->handle, &g_pairing_config_req_default);
                        break;
                    }

                    i = bt_dm_le_get_index_by_context(context);
                    if (i >= BT_DEVICE_MANAGER_LE_BONDED_MAX) {
                        break;
                    }
                    bt_dm_le_reset_context(context);
                    bt_device_manager_le_write_nvdm(bond_info, i);
                    bt_device_manager_le_remove_bond_info_order(i);
                    bt_device_manager_le_write_bond_info_order_nvdm();
                    return BT_STATUS_SUCCESS;
                }
                i = bt_device_manager_le_get_index_by_address(&(bond_info->bt_addr));
                bt_device_manager_le_update_bond_info_order(i);
                bt_device_manager_le_bond_info_duplicate_remove(i);
                bt_device_manager_le_set_link_type_by_handle(i, ind->handle);
                /* If we got IRK/Identity address from peer, we have to change
                            * bonding info's bd address; bt_device_manager_le_bonded_info_t. */
                if (BT_ADDR_TYPE_UNKNOW != bond_info->info.identity_addr.address.type) {
                    /* Because value of bonded_info->info.identity_addr.address_type is 0[Public Identity] or 1[Random Identity],
                                   *but Identity address type were definied 2 or 3 in spec.
                                   *We have to "+2" for synchronization. */
                    //bond_info->bt_addr = bond_info->info.identity_addr.address;
                    //update resolving list
                    bt_dm_le_add_device_to_resolving_list(&(bond_info_context[i]));
                }
#ifdef MTK_BLE_GAP_SRV_ENABLE
                if (bt_gap_le_srv_get_link_attribute_by_handle(ind->handle) & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NOT_NEED_RHO) {
                    BT_DM_LE_SET_FLAG_BY_INDEX(i, BT_DM_LE_FLAG_NOT_SYNC);
                } else {
                    BT_DM_LE_REMOVE_FLAG_BY_INDEX(i, BT_DM_LE_FLAG_NOT_SYNC);
                }
#endif
                BT_DM_LE_SET_FLAG_BY_INDEX(i, BT_DM_LE_FLAG_BONDED_COMPLETE);
                if ((255 != i) && (bond_info->info.key_security_mode & BT_GAP_LE_SECURITY_BONDED_MASK)) {
#ifdef AIR_NVDM_ENABLE
                    bt_device_manager_le_write_nvdm(bond_info, i);
                    bt_device_manager_le_write_bond_info_order_nvdm();
#endif
                }
#ifdef MTK_BLE_GAP_SRV_ENABLE
                if (!(bt_gap_le_srv_get_link_attribute_by_handle(ind->handle) & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NOT_NEED_RHO)) {
                    bt_device_manager_le_bonded_event_cb(BT_DEVICE_MANAGER_LE_BONDED_ADD, &(bond_info->bt_addr));
                } else {
                    LOG_MSGID_I(BT_DM, "[DM][LE] not need sync bond info to agent", 0);
                }
#else
                bt_device_manager_le_bonded_event_cb(BT_DEVICE_MANAGER_LE_BONDED_ADD, &(bond_info->bt_addr));
#endif
            }
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_gap_le_local_config_req_ind_t *bt_device_manager_le_get_local_config(void)
{
    g_local_config_default.local_key_req = &g_local_key_req_default;
    g_local_config_default.sc_only_mode_req = g_sc_only_default;
    return &g_local_config_default;
}

static bt_gap_le_bonding_info_t *bt_device_manager_le_get_bonding_info(const bt_addr_t remote_addr)
{
    bt_device_manager_le_bonded_info_t *dm_bonded_info = bt_device_manager_le_get_or_new_bonded_info(&remote_addr, true);
    if (dm_bonded_info) {
        return &(dm_bonded_info->info);
    }
    return NULL;
}

bt_gap_le_bonding_info_t *bt_device_manager_le_get_bonding_info_by_addr(bt_bd_addr_t *remote_addr)
{
    uint8_t i;
    bt_addr_t addr = {0};
    bt_device_manager_le_bonded_info_t *p_bond_info = NULL;
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Device bonded status check fail, please init BT DM LE first!", 0);
        return NULL;
    }
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        p_bond_info = &(bond_info_context[i].bonded_info);
        if (0 == bt_utils_memcmp(remote_addr, &(p_bond_info->bt_addr.addr), sizeof(bt_bd_addr_t))) {
            bt_utils_memcpy(&addr, &(p_bond_info->bt_addr), sizeof(bt_addr_t));
            LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_bonding_info_by_addr use bt_addr.", 0);
            return bt_device_manager_le_get_bonding_info((const bt_addr_t)addr);
        } else if (0 == bt_utils_memcmp(remote_addr, &(p_bond_info->info.identity_addr.address.addr), sizeof(bt_bd_addr_t))) {
            bt_utils_memcpy(&addr, &(p_bond_info->info.identity_addr.address), sizeof(bt_addr_t));
            LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_bonding_info_by_addr use identity addr.", 0);
            return bt_device_manager_le_get_bonding_info((const bt_addr_t)addr);
        }
    }
    return NULL;
}

bt_status_t bt_device_manager_le_gap_set_local_configuration(bt_gap_le_local_key_t *local_key, bool sc_only_mode)
{
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Set Local config fail, please init BT DM LE first!", 0);
        return BT_STATUS_FAIL;
    }
    bt_utils_memset(&g_local_key_req_default, 0x00, sizeof(bt_gap_le_local_key_t));
    bt_utils_memcpy(&g_local_key_req_default, local_key, sizeof(bt_gap_le_local_key_t));
    g_sc_only_default = sc_only_mode;
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_device_manager_le_gap_set_pairing_configuration(bt_gap_le_smp_pairing_config_t *pairing_config)
{
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Set Pairing config fail, please init BT DM LE first!", 0);
        return BT_STATUS_FAIL;
    }
    bt_utils_memset(&g_pairing_config_req_default, 0x00, sizeof(bt_gap_le_smp_pairing_config_t));
    g_pairing_config_req_default.io_capability = pairing_config->io_capability;
    g_pairing_config_req_default.oob_data_flag = pairing_config->oob_data_flag;
    g_pairing_config_req_default.auth_req = pairing_config->auth_req;
    g_pairing_config_req_default.maximum_encryption_key_size = pairing_config->maximum_encryption_key_size;
    g_pairing_config_req_default.initiator_key_distribution = pairing_config->initiator_key_distribution;
    g_pairing_config_req_default.responder_key_distribution = pairing_config->responder_key_distribution;
    return BT_STATUS_SUCCESS;
}

static void bt_device_manager_le_store_bonded_info_to_nvdm(uint8_t index)
{
    uint8_t i;
    bt_device_manager_le_bonded_info_t *p_bond_info = NULL;
    for (i = index; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        p_bond_info = &(bond_info_context[i].bonded_info);
        /**< if it is the end unit.*/
        if (0 == bt_utils_memcmp(&default_bt_addr, &(p_bond_info->bt_addr), sizeof(default_bt_addr))) {
#ifdef AIR_NVDM_ENABLE
            bt_device_manager_le_write_nvdm(p_bond_info, i);
#endif
            if (bt_dm_clear_flag) {
                if (i == (BT_DEVICE_MANAGER_LE_BONDED_MAX - 1)) {
                    bt_dm_clear_flag = false;
                    LOG_MSGID_I(BT_DM, "[DM][LE] clear NVDM buffer success", 0);
                    return;
                }
                continue;
            }
        }
        if (0 != bt_utils_memcmp(&default_bt_addr, &(p_bond_info->bt_addr), sizeof(default_bt_addr))) {
#ifdef AIR_NVDM_ENABLE
            bt_device_manager_le_write_nvdm(p_bond_info, i);
#endif
        }
    }
}

static void bt_device_manager_le_restore_bonded_info_from_nvdm(uint8_t index)
{
#ifdef AIR_NVDM_ENABLE
    uint8_t i;
    bt_device_manager_le_bonded_info_t *bond_info;
    for (i = index; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        bond_info = &(bond_info_context[i].bonded_info);
        bt_device_manager_le_read_nvdm(bond_info, i);
    }
#endif
}

static bt_dm_le_bond_info_context_t *bt_device_manager_le_find_old_bond_info_context(void)
{
#if 0    
    uint32_t i = 0;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX ; i++) {
        if (!(bond_info_context[i].flags & BT_DM_LE_FLAG_USING)) {
            LOG_MSGID_I(BT_DM, "[DM][LE] find old bond info index = %d.", 1, i);
            return &bond_info_context[i];
        }
    }
    LOG_MSGID_I(BT_DM, "[DM][LE] not find old bond info, all bond info is connecting.", 0);
    return NULL;
#endif
    uint8_t oldest_index;
    oldest_index = bt_device_manager_le_get_oldest_bonded_info_index();
    return &bond_info_context[oldest_index];
}

static bt_dm_le_bond_info_context_t *bt_dm_le_find_free_context_by_full(const bt_addr_t *peer_addr)
{
    bt_dm_le_bond_info_context_t *context = NULL;
    context = bt_device_manager_le_find_old_bond_info_context();
    if (context != NULL) {
        bt_dm_le_delete_device_from_resolving_list(context);
        bt_dm_le_reset_context(context);
        return context;
    } else {
        LOG_MSGID_I(BT_DM, "[DM][LE] Not find free context.", 0);
    }
    return NULL;
}

static bt_device_manager_le_bonded_info_t *bt_device_manager_le_get_or_new_bonded_info(const bt_addr_t *peer_addr, bool new_flag)
{
    uint8_t i;
    bt_device_manager_le_bonded_info_t *p_bond_info = NULL;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_or_new_bonded_info,unbond flag is (%d)", 1, new_flag);
    LOG_MSGID_I(BT_DM, "[DM][LE] get bonding info address: %02X:%02X:%02X:%02X:%02X:%02X,type %d", 7,
                peer_addr->addr[5],
                peer_addr->addr[4],
                peer_addr->addr[3],
                peer_addr->addr[2],
                peer_addr->addr[1],
                peer_addr->addr[0],
                peer_addr->type
               );
    if (!g_nvram_read_flag) {
        g_nvram_read_flag = true;
        bt_device_manager_le_restore_bonded_info_from_nvdm(0);
        bt_device_manager_le_read_bond_info_order_nvdm();
    }
    /** Check whether bonded? */
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX ; i++) {
        p_bond_info = &(bond_info_context[i].bonded_info);
        if ((0 == bt_utils_memcmp(&peer_addr->addr, &(p_bond_info->bt_addr.addr), sizeof(bt_bd_addr_t))) ||
            (0 == bt_utils_memcmp(&peer_addr->addr, &(p_bond_info->info.identity_addr.address.addr), sizeof(bt_bd_addr_t)))) {
            LOG_MSGID_I(BT_DM, "[DM][LE] Have Bonded, return bonded_info[%d].", 1, i);
            return p_bond_info;
        }
    }
    /** unbonded, so need new a buffer. */
    if (new_flag) {
        for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX ; i++) {
            p_bond_info = &(bond_info_context[i].bonded_info);
            if (0 == bt_utils_memcmp(&default_bt_addr, &(p_bond_info->bt_addr), sizeof(bt_addr_t))) {
                p_bond_info->info.identity_addr.address.type = BT_ADDR_TYPE_UNKNOW;
                bt_utils_memcpy(&p_bond_info->bt_addr, peer_addr, sizeof(bt_addr_t));
                LOG_MSGID_I(BT_DM, "[DM][LE] Un-Bonded, return bonded_info[%d].", 1, i);
                return p_bond_info;
            }
        }
        /* Have no empty buffer, so delete the oldest one, and return the last one buffer. */
        if (i == BT_DEVICE_MANAGER_LE_BONDED_MAX) {
            /* Get the bond info that was originally stored. */
            bt_dm_le_bond_info_context_t *context = bt_dm_le_find_free_context_by_full(peer_addr);
            if (context != NULL) {
                bt_utils_memcpy(&context->bonded_info.bt_addr, peer_addr, sizeof(bt_addr_t));
                return &context->bonded_info;
            }
        }
    }
    return NULL;
}

bt_device_manager_le_bonded_info_t *bt_device_manager_le_get_bonded_info_by_handle(bt_handle_t connection_handle)
{
    bt_status_t status;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_bonded_info_by_handle,conn_handle(0x%04x)", 1, connection_handle);
    bt_gap_le_connection_information_t con;
    bt_utils_memset(&(con), 0x00, sizeof(bt_gap_le_connection_information_t));
    status = bt_gap_le_get_connection_information(connection_handle, &con);
    if (BT_STATUS_SUCCESS == status) {
        return bt_device_manager_le_get_or_new_bonded_info(&(con.peer_addr), false);
    }
    return NULL;
}

static uint8_t bt_device_manager_le_get_index_by_address(bt_addr_t *address)
{
    uint8_t i;
    if (0 == bt_utils_memcmp(&default_bt_addr, address, sizeof(default_bt_addr))) {
        LOG_MSGID_I(BT_DM, "[DM][LE] empty address for find!", 0);
        return 255;
    }
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX ; i++) {
        if ((0 == bt_utils_memcmp(&address->addr, &(bond_info_context[i].bonded_info.bt_addr.addr), sizeof(bt_bd_addr_t))) ||
            (0 == bt_utils_memcmp(&address->addr, &(bond_info_context[i].bonded_info.info.identity_addr.address.addr), sizeof(bt_bd_addr_t)))) {
            return i;
        }
    }
    if (i == BT_DEVICE_MANAGER_LE_BONDED_MAX) {
        LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_index_by_address, not find !", 0);
        return 255;
    }
    return 255;
}

void bt_device_manager_le_remove_bonded_device(bt_addr_t *peer_addr)
{
    uint8_t i;
    bt_device_manager_le_bonded_info_t *p_bond_info = NULL;
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Remove bonded device fail, please init BT DM LE first!", 0);
        return;
    }
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        p_bond_info = &(bond_info_context[i].bonded_info);
        if ((0 == bt_utils_memcmp(peer_addr->addr, &(p_bond_info->bt_addr.addr), sizeof(bt_bd_addr_t))) ||
            (0 == bt_utils_memcmp(peer_addr->addr, &(p_bond_info->info.identity_addr.address.addr), sizeof(bt_bd_addr_t)))) {

            bt_device_manager_le_bonded_event_cb(BT_DEVICE_MANAGER_LE_BONDED_REMOVE, peer_addr);
            bt_dm_le_delete_device_from_resolving_list(&bond_info_context[i]);
            bt_dm_le_reset_context(&bond_info_context[i]);
            LOG_MSGID_I(BT_DM, "[DM][LE] Remove bonded info for index  %d", 1, i);
            bt_device_manager_le_store_bonded_info_to_nvdm(i);
            bt_device_manager_le_remove_bond_info_order(i);
            bt_device_manager_le_write_bond_info_order_nvdm();
            break;
        }
    }
    if (i == BT_DEVICE_MANAGER_LE_BONDED_MAX) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Remove bonded info fail, because can not find it!", 0);
    }
}

static void bt_device_manager_le_reset_bonded_infos(void)
{
    uint8_t i;
    bt_device_manager_le_bonded_event_cb(BT_DEVICE_MANAGER_LE_BONDED_CLEAR, NULL);
    g_nvram_read_flag = false;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX ; i++) {
        bt_dm_le_reset_context(&bond_info_context[i]);
    }
    bt_status_t st = bt_gap_le_set_resolving_list(BT_GAP_LE_CLEAR_RESOLVING_LIST, NULL);
    if (BT_STATUS_SUCCESS != st) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Clear device from Resolving List Failed", 0);
    }
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_reset_bonded_infos done!", 0);
}

void bt_device_manager_le_clear_all_bonded_info(void)
{
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Clear bonded info fail, please init BT DM LE first!", 0);
        return;
    }
    LOG_MSGID_I(BT_DM, "[DM][LE] start clear bonded info", 0);
    bt_dm_clear_flag = true;
    bt_device_manager_le_reset_bonded_infos();
    bt_device_manager_le_store_bonded_info_to_nvdm(0);
    bt_device_manager_le_reset_bond_info_order();
    bt_device_manager_le_write_bond_info_order_nvdm();
}

bool bt_device_manager_le_is_bonded(bt_addr_t *address)
{
    uint8_t i;
    bt_device_manager_le_bonded_info_t *p_bond_info = NULL;
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Device bonded status check fail, please init BT DM LE first!", 0);
        return false;
    }
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        p_bond_info = &(bond_info_context[i].bonded_info);
        if ((0 == bt_utils_memcmp(address, &(p_bond_info->bt_addr), sizeof(bt_addr_t))) ||
            (0 == bt_utils_memcmp(address, &(p_bond_info->info.identity_addr.address), sizeof(bt_addr_t)))) {
            LOG_MSGID_I(BT_DM, "[DM][LE] device have bonded! index is %d", 1, i);
            return true;
        }
    }
    return false;
}

uint8_t bt_device_manager_le_get_bonded_number(void)
{
    uint8_t i;
    uint8_t count = 0;
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Get bonded device number fail, please init BT DM LE first!", 0);
        return 0;
    }
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        if ((0 != bt_utils_memcmp(&default_bt_addr, &(bond_info_context[i].bonded_info.bt_addr), sizeof(bt_addr_t))) &&
            (bond_info_context[i].flags & BT_DM_LE_FLAG_BONDED_COMPLETE)) {
            count++;
        }
    }
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_bonded_number, bonded number is %d", 1, count);
    return count;
}

void bt_device_manager_le_get_bonded_list(bt_bd_addr_t *list, uint8_t *count)
{
    uint8_t i;
    uint8_t buff_size = *count;
    bt_bd_addr_t *p = list;
    uint8_t bonded_num = 0;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_bonded_list, want_read_count is %d", 1, *count);
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Get bonded list fail, please init BT DM LE first!", 0);
        return;
    }
    if ((NULL == list) || (0 == buff_size)) {
        LOG_MSGID_I(BT_DM, "[DM][LE] buffer is empty!", 0);
        return;
    }
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        LOG_MSGID_I(BT_DM, "[DM][LE] %d,%d,%d,%d!,index = %d", 5, default_bt_addr.type, default_bt_addr.addr[0], bond_info_context[i].bonded_info.bt_addr.type, bond_info_context[i].bonded_info.bt_addr.addr[0], i);
        if ((0 != bt_utils_memcmp(&default_bt_addr, &(bond_info_context[i].bonded_info.bt_addr), sizeof(bt_addr_t))) &&
            (bond_info_context[i].flags & BT_DM_LE_FLAG_BONDED_COMPLETE)) {
            bt_utils_memcpy(p, bond_info_context[i].bonded_info.bt_addr.addr, sizeof(bt_bd_addr_t));
            bonded_num ++;
            if (buff_size == bonded_num) {
                break;
            }
            p++;
        }
    }
    *count = bonded_num;
    LOG_MSGID_I(BT_DM, "bt_device_manager_le_get_bonded_list, real_read_count is %d", 1, *count);
}

static void bt_dm_le_set_bond_context_using(bt_addr_t *peer_address, bt_handle_t connection_handle, bool is_using)
{
    uint32_t i = 0;
    bt_device_manager_le_bonded_info_t *p_bond_info = NULL;
    bt_dm_le_bond_info_context_t *context = NULL;
    uint8_t *p_address = (uint8_t *)&peer_address->addr;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        p_bond_info = &(bond_info_context[i].bonded_info);
        if ((0 == bt_utils_memcmp(peer_address, &(p_bond_info->bt_addr), sizeof(bt_bd_addr_t))) ||
            (0 == bt_utils_memcmp(peer_address, &(p_bond_info->info.identity_addr.address), sizeof(bt_bd_addr_t)))) {
            LOG_MSGID_I(BT_DM, "[DM][LE] set bond context connecting index = %d, address:%02x:%02x:%02x:%02x:%02x:%02x is using = %02x connection handle = %02x", 9, i,
                        p_address[0], p_address[1], p_address[2], p_address[3], p_address[4], p_address[5], is_using, connection_handle);
            context = &bond_info_context[i];
            if (is_using) {
                context->connection_handle = connection_handle;
                BT_DM_LE_SET_FLAG(context, BT_DM_LE_FLAG_USING);
            } else {
                context->connection_handle = BT_DM_LE_INVALID_HANDLE;
                BT_DM_LE_REMOVE_FLAG(context, BT_DM_LE_FLAG_USING);
            }
            return;
        }
    }
    LOG_MSGID_I(BT_DM, "[DM][LE] not find need set using bond info", 0);
}

static void bt_device_manager_le_save_connection_params(void *buff)
{
    uint8_t i;
    bt_gap_le_connection_ind_t *conn_ind = (bt_gap_le_connection_ind_t *)buff;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_CONNECTION_MAX; i++) {
        if (0 == dm_info[i].connection_handle) {
            dm_info[i].connection_handle = conn_ind->connection_handle;
            dm_info[i].connection_params.conn_interval = conn_ind->conn_interval;
            dm_info[i].connection_params.slave_latency = conn_ind->conn_latency;
            dm_info[i].connection_params.supervision_timeout = conn_ind->supervision_timeout;
            bt_utils_memcpy(&dm_info[i].peer_address, &conn_ind->peer_addr, sizeof(bt_addr_t));
            break;
        }
    }
    if (i == BT_DEVICE_MANAGER_LE_CONNECTION_MAX) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Reach maximum connection, no empty buffer to save conn_info!", 0);
    }
    bt_dm_le_set_bond_context_using(&conn_ind->peer_addr, conn_ind->connection_handle, true);
}

static void bt_device_manager_le_delete_connection_params(void *buff)
{
    uint8_t i;
    bt_hci_evt_disconnect_complete_t *disc_ind;
    disc_ind = (bt_hci_evt_disconnect_complete_t *) buff;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_CONNECTION_MAX ; i++) {
        if (disc_ind->connection_handle == dm_info[i].connection_handle) {
            bt_dm_le_set_bond_context_using(&dm_info[i].peer_address, dm_info[i].connection_handle, false);
            bt_utils_memset(&(dm_info[i]), 0x00, sizeof(bt_device_manager_le_connection_struct_t));
            bt_device_manager_le_bonded_info_t *bond_info = bt_device_manager_le_get_bonded_info_by_handle(disc_ind->connection_handle);
            if ((bond_info != NULL) && (!(bond_info->info.key_security_mode & BT_GAP_LE_SECURITY_ENCRYPTION_MASK))) {
                bt_utils_memset(bond_info, 0, sizeof(bt_device_manager_le_bonded_info_t));
            }
            break;
        }
    }
    if (i == BT_DEVICE_MANAGER_LE_CONNECTION_MAX) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Don't know connection info for deleting!", 0);
    }
}

static void bt_device_manager_le_reset_connection_infos(void)
{
    uint8_t i;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_reset_connection_infos", 0);
    for (i = 0; i < BT_DEVICE_MANAGER_LE_CONNECTION_MAX ; i++) {
        bt_utils_memset(&(dm_info[i]), 0x00, sizeof(bt_device_manager_le_connection_struct_t));
    }
}

static void bt_device_manager_le_update_current_conn_interval(void *conn_params)
{
    uint8_t i;
    bt_gap_le_connection_update_ind_t *ind = (bt_gap_le_connection_update_ind_t *)conn_params;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_update_current_conn_interval, conn_handle(0x%04x)", 1, ind->conn_handle);
    for (i = 0; i < BT_DEVICE_MANAGER_LE_CONNECTION_MAX; i++) {
        if (dm_info[i].connection_handle == ind->conn_handle) {
            dm_info[i].connection_params.conn_interval = ind->conn_interval;
            dm_info[i].connection_params.slave_latency = ind->conn_latency;
            dm_info[i].connection_params.supervision_timeout = ind->supervision_timeout;
            break;
        }
    }
    if (i == BT_DEVICE_MANAGER_LE_CONNECTION_MAX) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Reach maximum connection, update conn params fail!", 0);
    }
}

bt_device_manager_le_connection_param_t *bt_device_manager_le_get_current_connection_param(bt_handle_t connection_handle)
{
    uint8_t i;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_current_connection_param, conn_handle(0x%04x)", 1, connection_handle);
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Get Conn interval fail, please init BT DM LE first! ", 0);
        return NULL;
    }
    for (i = 0; i < BT_DEVICE_MANAGER_LE_CONNECTION_MAX; i++) {
        if (connection_handle == dm_info[i].connection_handle) {
            return &(dm_info[i].connection_params);
        }
    }
    return NULL;
}

bt_bd_addr_ptr_t bt_device_manager_le_get_public_address(void)
{
    bt_device_manager_le_gen_public_address();
    return (bt_bd_addr_ptr_t)(&bt_device_manager_le_local_public_addr);
}


static void bt_device_manager_le_gen_public_address(void)
{
    bt_bd_addr_t tempaddr = {0};
    if (bt_utils_memcmp(bt_device_manager_le_local_public_addr, &tempaddr, sizeof(bt_bd_addr_t)) == 0) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Try to read public address from NVDM! ", 0);
        bt_bd_addr_t *localaddr = bt_device_manager_get_local_address();
        if (bt_utils_memcmp(localaddr, &tempaddr, sizeof(tempaddr))) {
            bt_utils_memcpy(bt_device_manager_le_local_public_addr, localaddr, sizeof(bt_bd_addr_t));
            LOG_MSGID_I(BT_DM, "[DM][LE] Read address from NVDM [%02X:%02X:%02X:%02X:%02X:%02X]", 6,
                        bt_device_manager_le_local_public_addr[0],
                        bt_device_manager_le_local_public_addr[1],
                        bt_device_manager_le_local_public_addr[2],
                        bt_device_manager_le_local_public_addr[3],
                        bt_device_manager_le_local_public_addr[4],
                        bt_device_manager_le_local_public_addr[5]);
            return;
        } else {
#ifdef HAL_TRNG_MODULE_ENABLED
            int8_t i;
            uint32_t random_seed;
            hal_trng_status_t ret = HAL_TRNG_STATUS_OK;
            ret = hal_trng_init();
            if (HAL_TRNG_STATUS_OK != ret) {
                LOG_MSGID_I(BT_DM, "[DM][LE] generate_public_address--error 1!", 0);
            }
            for (i = 0; i < 30; ++i) {
                ret = hal_trng_get_generated_random_number(&random_seed);
                if (HAL_TRNG_STATUS_OK != ret) {
                    LOG_MSGID_I(BT_DM, "[DM][LE] generate_public_address--error 2!", 0);
                }
            }
            /* randomly generate address */
            ret = hal_trng_get_generated_random_number(&random_seed);
            if (HAL_TRNG_STATUS_OK != ret) {
                LOG_MSGID_I(BT_DM, "[DM][LE] generate_public_address--error 3!", 0);
            }
            bt_device_manager_le_local_public_addr[0] = random_seed & 0xFF;
            bt_device_manager_le_local_public_addr[1] = (random_seed >> 8) & 0xFF;
            bt_device_manager_le_local_public_addr[2] = (random_seed >> 16) & 0xFF;
            bt_device_manager_le_local_public_addr[3] = (random_seed >> 24) & 0xFF;
            ret = hal_trng_get_generated_random_number(&random_seed);
            if (HAL_TRNG_STATUS_OK != ret) {
                LOG_MSGID_I(BT_DM, "[DM][LE] generate_public_address--error 3!", 0);
            }
            bt_device_manager_le_local_public_addr[4] = random_seed & 0xFF;
            bt_device_manager_le_local_public_addr[5] = (random_seed >> 8) & 0xCF;
            hal_trng_deinit();
#else
#include "hal_gpt.h"
            uint32_t seed = 0;
            hal_gpt_status_t gpt_ret = HAL_GPT_STATUS_OK;
            gpt_ret = (int32_t)hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &seed);
            if (gpt_ret == HAL_GPT_STATUS_OK) {
                srand(seed);
                bt_device_manager_le_local_public_addr[0] = rand() & 0xFF;
                bt_device_manager_le_local_public_addr[1] = rand() & 0xFF;
                bt_device_manager_le_local_public_addr[2] = rand() & 0xFF;
                bt_device_manager_le_local_public_addr[3] = rand() & 0xFF;
                bt_device_manager_le_local_public_addr[4] = rand() & 0xFF;
                bt_device_manager_le_local_public_addr[5] = rand() & 0xFF;
            } else {
                bt_device_manager_le_local_public_addr[0] = 0x66;
                bt_device_manager_le_local_public_addr[1] = 0x77;
                bt_device_manager_le_local_public_addr[2] = 0xE2;
                bt_device_manager_le_local_public_addr[3] = 0xE1;
                bt_device_manager_le_local_public_addr[4] = 0x90;
                bt_device_manager_le_local_public_addr[5] = 0x87;
            }
#endif
        }
    }
    /* save address to NVDM */
    bt_device_manager_store_local_address(&bt_device_manager_le_local_public_addr);
    LOG_MSGID_I(BT_DM, "[DM][LE] Successfully store address to NVDM [%02X:%02X:%02X:%02X:%02X:%02X]", 6,
                bt_device_manager_le_local_public_addr[0],
                bt_device_manager_le_local_public_addr[1],
                bt_device_manager_le_local_public_addr[2],
                bt_device_manager_le_local_public_addr[3],
                bt_device_manager_le_local_public_addr[4],
                bt_device_manager_le_local_public_addr[5]);
}

bt_device_manager_le_bonded_info_t *bt_device_manager_le_get_bonding_info_by_addr_ext(bt_bd_addr_t *remote_addr)
{
    uint8_t i;
    //bt_addr_t addr = {0};
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Device bonded status check fail, please init BT DM LE first!", 0);
        return NULL;
    }
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        if ((0 != bt_utils_memcmp(&default_bt_addr.addr, remote_addr, BT_BD_ADDR_LEN)) &&
            ((0 == bt_utils_memcmp(remote_addr, &(bond_info_context[i].bonded_info.bt_addr.addr), sizeof(bt_bd_addr_t))) ||
             (0 == bt_utils_memcmp(remote_addr, &(bond_info_context[i].bonded_info.info.identity_addr.address.addr), sizeof(bt_bd_addr_t))))) {
            return &(bond_info_context[i].bonded_info);
        }
    }
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_bonding_info_by_addr_ext Fail!", 0);
    return NULL;
}

bt_status_t bt_device_manager_le_get_all_bonding_infos(bt_device_manager_le_bonded_info_t *infos, uint8_t *count)
{
    uint8_t i;
    uint8_t buff_size = *count;
    bt_device_manager_le_bonded_info_t *p = infos;
    uint8_t bonded_num = 0;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_bonded_list, want_read_count is %d", 1, *count);
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "Get bonded list fail, please init BT DM LE first!", 0);
        return BT_STATUS_FAIL;
    }
    if ((NULL == infos) || (0 == buff_size)) {
        LOG_MSGID_I(BT_DM, "[DM][LE] buffer is empty!", 0);
        return BT_STATUS_FAIL;
    }
    for (i = 0; ((i < BT_DEVICE_MANAGER_LE_BONDED_MAX) && (i < buff_size)); i++) {
        LOG_MSGID_I(BT_DM, "%d,%d,%d,%d!", 4, default_bt_addr.type, default_bt_addr.addr[0], bond_info_context[i].bonded_info.bt_addr.type, bond_info_context[i].bonded_info.bt_addr.addr[0]);
        if (0 != bt_utils_memcmp(&default_bt_addr, &(bond_info_context[i].bonded_info.bt_addr), sizeof(bt_addr_t))) {
            bt_utils_memcpy(&(p->bt_addr), &(bond_info_context[i].bonded_info.bt_addr), sizeof(bt_addr_t));
            bt_utils_memcpy(&(p->info), &(bond_info_context[i].bonded_info.info), sizeof(bt_gap_le_bonding_info_t));
            bonded_num++;
            p++;
        }
    }
    *count = bonded_num;
    LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_get_bonded_list, real_read_count is %d", 1, *count);
    return BT_STATUS_SUCCESS;
}

static void bt_dm_le_generate_random_key(uint8_t *key, uint8_t size)
{
    uint32_t random_seed;
    hal_trng_status_t ret = hal_trng_init();
    int8_t i;
    if (HAL_TRNG_STATUS_OK != ret) {
        LOG_MSGID_I(BT_DM, "[DM][LE] generate_random_key--error 1", 0);
    }
    for (i = 0; i < 30; ++i) {
        ret = hal_trng_get_generated_random_number(&random_seed);
        if (HAL_TRNG_STATUS_OK != ret) {
            LOG_MSGID_I(BT_DM, "DM][LE] generate_random_key--error 2", 0);
        }
    }
    for (i = 0; i < (size + 3) / 4; i++) {
        ret = hal_trng_get_generated_random_number(&random_seed);
        if (HAL_TRNG_STATUS_OK != ret) {
            LOG_MSGID_I(BT_DM, "DM][LE] generate_random_key--error 3", 0);
        }
        key[0 + 4 * i] = random_seed & 0xFF;
        key[1 + 4 * i] = (random_seed >> 8) & 0xFF;
        key[2 + 4 * i] = (random_seed >> 16) & 0xFF;
        key[3 + 4 * i] = (random_seed >> 24) & 0xFF;
    }

    hal_trng_deinit();
}

bt_status_t bt_device_manager_le_set_bonding_info_by_addr(bt_addr_t *remote_addr, bt_gap_le_bonding_info_t *info)
{
    uint8_t i;
    bt_utils_mutex_lock();
    uint8_t default_irk[BT_KEY_SIZE] = {0};
    bt_utils_memset(default_irk, 0xff, BT_KEY_SIZE);
    if (bt_utils_memcmp(&info->identity_info.irk, default_irk, BT_KEY_SIZE) == 0) {
        LOG_MSGID_I(BT_DM, "[DM][LE] set special bond info", 0);
        bt_dm_le_generate_random_key((uint8_t *)&info->local_key.identity_info.irk, BT_KEY_SIZE);
    }
    //bt_addr_t addr = {0};
    if (!bt_dm_le_initialized) {
        LOG_MSGID_I(BT_DM, "[DM][LE] Device bonded status check fail, please init BT DM LE first!", 0);
        bt_utils_mutex_unlock();
        return BT_STATUS_FAIL;
    }
    if ((NULL == info) || (!(info->key_security_mode & BT_GAP_LE_SECURITY_BONDED_MASK))) {
        LOG_MSGID_I(BT_DM, "[DM][LE] bt_device_manager_le_set_bonding_info_by_addr fail, empty info!", 0);
        bt_utils_mutex_unlock();
        return BT_STATUS_FAIL;
    }
    /** Check whether bonded? */
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX ; i++) {
        if ((0 == bt_utils_memcmp(remote_addr->addr, &(bond_info_context[i].bonded_info.bt_addr.addr), sizeof(bt_bd_addr_t))) ||
            (0 == bt_utils_memcmp(remote_addr->addr, &(bond_info_context[i].bonded_info.info.identity_addr.address.addr), sizeof(bt_bd_addr_t)))) {
            LOG_MSGID_I(BT_DM, "[DM][LE] Have Bonded, bonded_info[%d].", 1, i);
            //TODO, update anyway(compare all)
            //save the new keys
            if (0 != bt_utils_memcmp(info, &(bond_info_context[i].bonded_info.info), sizeof(bt_gap_le_bonding_info_t))) {
                bt_dm_le_delete_device_from_resolving_list((&bond_info_context[i]));
                bt_utils_memcpy(&(bond_info_context[i].bonded_info.info), info, sizeof(bt_gap_le_bonding_info_t));
                BT_DM_LE_SET_FLAG_BY_INDEX(i, BT_DM_LE_FLAG_NOT_SYNC | BT_DM_LE_FLAG_BONDED_COMPLETE);
                bt_device_manager_le_store_bonded_info_to_nvdm(i);
                bt_device_manager_le_update_bond_info_order(i);
                bt_device_manager_le_write_bond_info_order_nvdm();
                bt_dm_le_add_device_to_resolving_list_by_index(i);
                LOG_MSGID_I(BT_DM, "[DM][LE] re-save success, bonded_info[%d]", 1, i);
            }
            bt_utils_mutex_unlock();
            return BT_STATUS_SUCCESS;
        }
    }
    /** unbonded, so need new a buffer. */
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX ; i++) {
        if (0 == bt_utils_memcmp(&default_bt_addr, &(bond_info_context[i].bonded_info.bt_addr), sizeof(bt_addr_t))) {
            bond_info_context[i].bonded_info.info.identity_addr.address.type = BT_ADDR_TYPE_UNKNOW;
            bt_utils_memcpy(&(bond_info_context[i].bonded_info.bt_addr), remote_addr, sizeof(bt_addr_t));
            bt_utils_memcpy(&(bond_info_context[i].bonded_info.info), info, sizeof(bt_gap_le_bonding_info_t));
            LOG_MSGID_I(BT_DM, "[DM][LE] save success, bonded_info[%d]", 1, i);
            BT_DM_LE_SET_FLAG_BY_INDEX(i, BT_DM_LE_FLAG_NOT_SYNC | BT_DM_LE_FLAG_BONDED_COMPLETE);
            bt_device_manager_le_store_bonded_info_to_nvdm(i);
            bt_device_manager_le_update_bond_info_order(i);
            bt_device_manager_le_write_bond_info_order_nvdm();
            bt_dm_le_add_device_to_resolving_list_by_index(i);
            bt_utils_mutex_unlock();
            return BT_STATUS_SUCCESS;
        }
    }
    /**have no empty buffer, so delete the oldest one, and return the last one buffer. */
    if (i == BT_DEVICE_MANAGER_LE_BONDED_MAX) {
        LOG_MSGID_I(BT_DM, "[DM][LE] No empty buffer, Need to delete the oldest one!", 0);
        bt_dm_le_bond_info_context_t *context = bt_dm_le_find_free_context_by_full(remote_addr);
        if (context != NULL) {
            BT_DM_LE_SET_FLAG(context, BT_DM_LE_FLAG_NOT_SYNC | BT_DM_LE_FLAG_BONDED_COMPLETE);
            bt_utils_memcpy(&context->bonded_info.bt_addr, remote_addr, sizeof(bt_addr_t));
            bt_utils_memcpy(&context->bonded_info.info, info, sizeof(bt_gap_le_bonding_info_t));
            uint8_t index = bt_device_manager_le_get_index_by_address(&context->bonded_info.bt_addr);
            bt_device_manager_le_write_nvdm(&context->bonded_info, index);
            bt_dm_le_add_device_to_resolving_list(context);
            bt_utils_mutex_unlock();
            return BT_STATUS_SUCCESS;
        }
    }
    bt_utils_mutex_unlock();
    return BT_STATUS_SUCCESS;
}

/**
 * @brief Function for application main entry.
 */
bt_status_t bt_device_manager_le_register_callback(bt_device_manager_le_bonded_event_callback callback)
{
    if (NULL == callback) {
        return BT_STATUS_FAIL;
    } else {
        /**Initialize.*/
        LOG_MSGID_I(BT_DM, "[DM][LE] init app_callback=0x%04x", 1, callback);
        return bt_device_manager_le_cb_register_int(callback);
    }
}

bt_status_t bt_device_manager_le_deregister_callback(bt_device_manager_le_bonded_event_callback callback)
{
    if (NULL == callback) {
        return BT_STATUS_FAIL;
    } else {
        /**Deinit.*/
        LOG_MSGID_I(BT_DM, "[DM][LE] deinit app_callback = 0x%04x", 1, callback);
        return bt_device_manager_le_cb_deregister_int(callback);
    }
    return BT_STATUS_FAIL;
}

bool bt_device_manager_le_is_sync_bond_info(bt_addr_t *bt_addr)
{
    uint8_t i = 0;
    bt_device_manager_le_bonded_info_t *p_bond_info = NULL;
    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        p_bond_info = &bond_info_context[i].bonded_info;
        if ((0 == bt_utils_memcmp(&p_bond_info->bt_addr, bt_addr, sizeof(bt_addr_t))) &&
            (0 != bt_utils_memcmp(&default_bt_addr, bt_addr, sizeof(bt_addr_t)))) {
            if (BT_DM_LE_GET_FLAG_BY_INDEX(i) & BT_DM_LE_FLAG_NOT_SYNC) {
                LOG_MSGID_I(BT_DM, "[DM][LE] Not need sync bond info index = %d ", 1, i);
                return false;
            }
        }
    }
    return true;
}

static bt_dm_le_bond_info_context_t *bt_dm_le_get_bonded_info_context_by_handle(bt_handle_t connection_handle)
{
    bt_status_t status;
    uint32_t i = 0;
    LOG_MSGID_I(BT_DM, "[DM][LE] get bond info context by handle = %02x", 1, connection_handle);
    bt_gap_le_connection_information_t connection_info;
    bt_utils_memset(&(connection_info), 0x00, sizeof(bt_gap_le_connection_information_t));
    status = bt_gap_le_get_connection_information(connection_handle, &connection_info);
    bt_device_manager_le_bonded_info_t *p_bond_info = NULL;
    if (BT_STATUS_SUCCESS == status) {
        bt_addr_t *peer_addr = &connection_info.peer_addr;
        for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX ; i++) {
            p_bond_info = &(bond_info_context[i].bonded_info);
            if ((0 == bt_utils_memcmp(peer_addr, &(p_bond_info->bt_addr), sizeof(bt_addr_t))) ||
                (0 == bt_utils_memcmp(peer_addr, &(p_bond_info->info.identity_addr.address), sizeof(bt_addr_t)))) {
                LOG_MSGID_I(BT_DM, "[DM][LE] have Bonded, index = %d.", 1, i);
                return &(bond_info_context[i]);
            }
        }
    } else {
        LOG_MSGID_I(BT_DM, "[DM][LE] not find le link by handle", 0);
    }
    return NULL;
}

bt_status_t bt_device_manager_le_set_privacy_mode(bt_handle_t connection_handle, bt_hci_privacy_mode_t mode)
{
    LOG_MSGID_I(BT_DM, "[DM][LE] set privacy mode connection handle = %02x, mode = %02x", 2, connection_handle, mode);
    bt_dm_le_bond_info_context_t *context = bt_dm_le_get_bonded_info_context_by_handle(connection_handle);
    if (context != NULL) {
        context->privacy_mode = mode;
    } else {
        LOG_MSGID_I(BT_DM, "[DM][LE] not find bond info context by handle", 0);
    }
    return BT_STATUS_SUCCESS;
}

#ifdef MTK_BLE_GAP_SRV_ENABLE
static bt_dm_le_wl_context_t *bt_dm_le_find_free_wl_context(void)
{
    bt_dm_le_wl_context_t *wl_context = (bt_dm_le_wl_context_t *)pvPortMalloc(sizeof(bt_dm_le_wl_context_t));
    return wl_context;
}

static void bt_dm_le_push_rsl_list_info(bt_dm_le_wl_rsl_list_info_t *rsl_list_info, const bt_gap_le_bonding_info_t *bond_info)
{
    bt_utils_memcpy(&rsl_list_info->peer_identity_info, &bond_info->identity_info, sizeof(bt_gap_le_identity_info_t));
    bt_utils_memcpy(&rsl_list_info->local_identity_info, &bond_info->local_key.identity_info, sizeof(bt_gap_le_identity_info_t));
    bt_utils_memcpy(&rsl_list_info->identity_addr, &bond_info->identity_addr, sizeof(bt_gap_le_identity_address_info_t));
}

static void bt_dm_le_pop_rsl_list_info(const bt_dm_le_wl_rsl_list_info_t *rsl_list_info, bt_gap_le_bonding_info_t *bond_info)
{
    bt_utils_memcpy(&bond_info->identity_info, &rsl_list_info->peer_identity_info, sizeof(bt_gap_le_identity_info_t));
    bt_utils_memcpy(&bond_info->local_key.identity_info, &rsl_list_info->local_identity_info, sizeof(bt_gap_le_identity_info_t));
    bt_utils_memcpy(&bond_info->identity_addr, &rsl_list_info->identity_addr, sizeof(bt_gap_le_identity_address_info_t));
}

static bt_status_t bt_dm_le_add_waiting_list(bt_dm_le_wl_action_t action, bt_dm_le_bond_info_context_t *context)
{
    if (context == NULL) {
        LOG_MSGID_I(BT_DM, "[DM][LE][WL] add waiting list context is NULL", 0);
        return BT_STATUS_FAIL;
    }
    bt_dm_le_wl_context_t *wl_context = bt_dm_le_find_free_wl_context();
    if (wl_context == NULL) {
        LOG_MSGID_I(BT_DM, "[DM][LE][WL] waiting list context allocate fail", 0);
        return BT_STATUS_FAIL;
    }
    wl_context->action = action;
    wl_context->privacy_mode = context->privacy_mode;
    bt_dm_le_push_rsl_list_info(&wl_context->rsl_list_info, &context->bonded_info.info);
    LOG_MSGID_I(BT_DM, "[DM][LE][WL] add wl action = %02x, wl context = %02x", 2, action, wl_context);
    bt_gap_le_srv_linknode_insert_node(&bt_dm_wl, (bt_gap_le_srv_linknode_t *)wl_context, BT_GAP_LE_SRV_NODE_BACK);
    return BT_STATUS_SUCCESS;
}

static bt_gap_le_srv_error_t bt_dm_le_remove_waiting_list_node(bt_dm_le_wl_context_t *context)
{
    if (context == NULL) {
        LOG_MSGID_I(BT_DM, "[DM][LE][WL] remove waiting list node is NULL", 0);
        return BT_STATUS_FAIL;
    }
    LOG_MSGID_I(BT_DM, "[DM][LE][WL] remove adv waiting list node = %02x", 1, context);
    bt_gap_le_srv_linknode_remove_node(&bt_dm_wl, (bt_gap_le_srv_linknode_t *)context);
    /* free alloc buffer. */
    vPortFree((void *)context);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_dm_le_run_waiting_list(void)
{
    bt_dm_le_wl_context_t *wl_context = (bt_dm_le_wl_context_t *)bt_dm_wl.front;
    if (wl_context == NULL) {
        LOG_MSGID_I(BT_DM, "[DM][LE][WL] not find waiting action", 0);
        return BT_STATUS_FAIL;
    }
    LOG_MSGID_I(BT_DM, "[DM][LE][WL] find wl action = %02x, wl context = %02x", 2, wl_context->action, wl_context);
    bt_dm_le_bond_info_context_t context = {0};
    bt_dm_le_pop_rsl_list_info(&wl_context->rsl_list_info, &context.bonded_info.info);
    context.privacy_mode = wl_context->privacy_mode;
    switch (wl_context->action) {
        case BT_DM_LE_WL_ACTION_REMOVE: {
            bt_dm_le_delete_device_from_resolving_list(&context);
        }
        break;
        case BT_DM_LE_WL_ACTION_ADD: {
            bt_dm_le_add_device_to_resolving_list(&context);
        }
        break;
        default:
            break;
    }
    bt_dm_le_remove_waiting_list_node(wl_context);
    return BT_STATUS_SUCCESS;
}

static void bt_dm_le_clear_waiting_list(void)
{
    bt_gap_le_srv_linknode_t *temp_node = (bt_gap_le_srv_linknode_t *)bt_dm_wl.front;
    while (temp_node != NULL) {
        bt_gap_le_srv_linknode_remove_node((bt_gap_le_srv_linknode_t *)&bt_dm_wl, temp_node);
        /* Free node buffer. */
        vPortFree(temp_node);
        temp_node = (bt_gap_le_srv_linknode_t *)bt_dm_wl.front;
    }
    bt_dm_wl.front = NULL;
}
#endif

bt_status_t bt_dm_le_remove_bonded_device(bt_addr_t *peer_addr, bool is_clear)
{
    uint32_t i;
    bt_device_manager_le_bonded_info_t *p_bond_info = NULL;
    bt_status_t status = BT_STATUS_FAIL;

    for (i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        p_bond_info = &(bond_info_context[i].bonded_info);
        if ((0 == bt_utils_memcmp(peer_addr, &(p_bond_info->bt_addr), sizeof(bt_addr_t))) ||
            (0 == bt_utils_memcmp(peer_addr, &(p_bond_info->info.identity_addr.address), sizeof(bt_addr_t)))) {
            bt_device_manager_le_bonded_event_cb(BT_DEVICE_MANAGER_LE_BONDED_REMOVE, peer_addr);
            LOG_MSGID_I(BT_DM, "[DM][LE] Remove bonded info for index for re-paring", 1, i);
            status = BT_STATUS_SUCCESS;
            break;
        }
    }
    if (i == BT_DEVICE_MANAGER_LE_BONDED_MAX) {
        LOG_MSGID_I(BT_DM, "[DM][LE] remove bonded info fail for re-paring, because can not find it", 0);
    }
    return status;
}

void static bt_dm_le_reverse_key(uint8_t *dst, const uint8_t *src, uint32_t length)
{
    uint32_t i;
    for (i = 0; i < length; i++) {
        dst[i] = src[length - i - 1];
    }
}

static void bt_dm_le_sm_encrypt(bt_key_t output, bt_key_t *key, bt_key_t *data)
{
    bt_os_layer_aes_buffer_t encrypted_data, plain_text, key_struct;
    uint8_t buffer[32] = {0};
    uint8_t reverse_key[16] = {0};
    uint8_t reverse_data[16] = {0};

    bt_dm_le_reverse_key(reverse_key, (uint8_t *)key, BT_KEY_SIZE);
    bt_dm_le_reverse_key(reverse_data, (uint8_t *)data, BT_KEY_SIZE);

    key_struct.buffer = (void *)reverse_key;
    key_struct.length = sizeof(bt_key_t);

    plain_text.buffer = (void *)reverse_data;
    plain_text.length = sizeof(bt_key_t);

    encrypted_data.buffer = buffer;
    encrypted_data.length = 32;

    bt_os_layer_aes_encrypt(&encrypted_data, &plain_text, &key_struct);

    bt_utils_memcpy(output, buffer, sizeof(bt_key_t));

    bt_dm_le_reverse_key(output, buffer, BT_KEY_SIZE);
}

static bt_status_t bt_dm_le_analysis_rpa(bt_dm_le_bond_info_context_t *context, uint8_t *prand, uint8_t *hash)
{
    bt_utils_assert(context && "analysis rpa conetxt is NULL");
    uint8_t default_key[BT_KEY_SIZE] = {0};
    uint8_t output_key[BT_KEY_SIZE] = {0};
    uint8_t padding_data[BT_KEY_SIZE] = {0};

    if (hash == NULL) {
        return BT_STATUS_FAIL;
    }

    bt_gap_le_bonding_info_t *bond_info = (bt_gap_le_bonding_info_t *)&context->bonded_info.info;

    if (0 == bt_utils_memcmp(&bond_info->identity_info.irk, &default_key, sizeof(default_key))) {
        LOG_MSGID_W(BT_DM, "[DM][LE] analysis rpa remote IRK is 0", 0);
        return BT_STATUS_FAIL;
    }
    bt_utils_memcpy(padding_data, prand, BT_DM_LE_RPA_HASH_LENGTH);
    bt_dm_le_sm_encrypt(output_key, &bond_info->identity_info.irk, (bt_key_t *)padding_data);
    bt_utils_memcpy(hash, output_key, BT_DM_LE_RPA_HASH_LENGTH);
    LOG_MSGID_I(BT_DM, "[DM][LE] analysis rpa output", 0);
    for (uint32_t i = 0; i < 16; i++) {
        LOG_MSGID_I(BT_DM, "[DM][LE] %02x", 1, output_key[i]);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_device_manager_le_get_bonding_info_by_link_type(bt_gap_le_srv_link_t link_type, bt_device_manager_le_bonded_info_t *infos, uint8_t *count)
{
    uint32_t actual_count = *count;
    uint32_t bond_number = 0;
    bt_device_manager_le_bonded_info_t *p_bond_info = infos;

    if ((infos == NULL) || (*count == 0)) {
        LOG_MSGID_I(BT_DM, "[DM][LE] get bond info by link type fail infos = %02x count = %02x", 2, infos, *count);
        return BT_STATUS_FAIL;
    }

    for (uint32_t i = 0; i < BT_DEVICE_MANAGER_LE_BONDED_MAX; i++) {
        bt_dm_le_bond_info_context_t *context = &bond_info_context[i];
        if ((context->link_type & link_type) > 0) {
            bt_utils_memcpy(&(p_bond_info->bt_addr), &(context->bonded_info.bt_addr), sizeof(bt_addr_t));
            bt_utils_memcpy(&(p_bond_info->info), &(context->bonded_info.info), sizeof(bt_gap_le_bonding_info_t));
            LOG_MSGID_I(BT_DM, "[DM][LE] get bond info index = %02x link type = %02x", 2, i, link_type);
            bond_number++;
            if (bond_number >= actual_count) {
                break;
            }
            p_bond_info++;
        }
    }
    *count = bond_number;
    LOG_MSGID_I(BT_DM, "[DM][LE] get bond info number = %02x by link type = %02x", 2, *count, link_type);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_device_manager_le_generate_rpa(bt_device_manager_le_bonded_info_t *bond_info,
                                              uint8_t *prand, uint32_t prand_length, bt_bd_addr_t *rpa)
{
    uint8_t output_key[BT_KEY_SIZE] = {0};
    uint8_t padding_data[BT_KEY_SIZE] = {0};

    if ((NULL == bond_info) || (NULL == prand) || (prand_length != BT_DM_LE_RPA_HASH_LENGTH) || (rpa == NULL)) {
        LOG_MSGID_E(BT_DM, "[DM][LE] generate rpa fail bond info = %02x prand = %02x prand_length = %02x rpa = %02x", 4,
                    bond_info, prand, prand_length, rpa);
        return BT_STATUS_FAIL;
    }

    bt_utils_memcpy(padding_data, prand, BT_DM_LE_RPA_HASH_LENGTH);
    bt_dm_le_sm_encrypt(output_key, &bond_info->info.local_key.identity_info.irk, (bt_key_t *)padding_data);

    LOG_MSGID_I(BT_DM, "[DM][LE] generate rpa hash output = %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x", 16,
                output_key[0], output_key[1], output_key[2], output_key[3],
                output_key[4], output_key[5], output_key[6], output_key[7],
                output_key[8], output_key[9], output_key[10], output_key[11],
                output_key[12], output_key[13], output_key[14], output_key[15]);

    bt_utils_memcpy((uint8_t *)rpa, output_key, BT_DM_LE_RPA_HASH_LENGTH);
    bt_utils_memcpy(((uint8_t *)rpa + 3), prand, BT_DM_LE_RPA_HASH_LENGTH);
    return BT_STATUS_SUCCESS;
}
