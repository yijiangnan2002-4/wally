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

#include "bt_type.h"
#include "bt_callback_manager.h"
#include "bt_utils.h"
#include "bt_gattc_discovery.h"
#include "bt_gatt_service_client.h"
#include "bt_timer_external.h"
#include "bt_gap_le_service.h"
#ifdef BT_ROLE_HANDOVER_WITH_SPP_BLE
#include "bt_role_handover.h"
#endif

log_create_module(gatt_client, PRINT_LEVEL_INFO);

#define BT_GATT_SRV_CLIENT_LINK_NUMBER            4U
#define BT_GATT_SRV_CLIENT_INVALID_HANDLE         0U

#define BT_GATT_SRV_CLIENT_GAP_CHARC_NUMBER       5U
#define BT_GATT_SRV_CLIENT_GATT_CHARC_NUMBER      4U
#define BT_GATT_SRV_CLIENT_GATT_DISCRIPTOR_NUMBER 1U

#define BT_GATT_SRV_CLIENT_CHARC_VALUE_LENGTH     10U

#define BT_GATT_SRV_CLIENT_ACTION_NUMBER          5U

#define BT_GATT_SRV_CLIENT_DEVICE_NAME_LENGTH     20U

#define BT_GATT_SRV_CLIENT_NONE                   0xFF         /**< The invalid characteristic. */

#define BT_GATT_SRV_CLIENT_FLAG_GATT_DISCORYED          0x0001
#define BT_GATT_SRV_CLIENT_FLAG_GAP_DISCORYED           0x0002
#define BT_GATT_SRV_CLIENT_FLAG_READING                 0x0004
#define BT_GATT_SRV_CLIENT_FLAG_WRITING                 0x0008
#define BT_GATT_SRV_CLIENT_FLAG_DISCOVERY_CONTINUE      0x0010
#define BT_GATT_SRV_CLIENT_FLAG_RHO_PENDING             0x0020
#define BT_GATT_SRV_CLIENT_FLAG_RHO_DISCOVERY_TRIGGER   0x0040
typedef uint16_t bt_gatt_srv_client_flag_t;

#define BT_GATT_SRV_CLIENT_SET_FLAG(context, flag)         (context->flags |= flag)
#define BT_GATT_SRV_CLIENT_REMOVE_FLAG(context, flag)      (context->flags &= ~flag)
#define BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, flag)      (context->flags & flag)

typedef struct {
    bt_gattc_discovery_service_t         gap_service;
    bt_gattc_discovery_service_t         gatt_service;
    bt_gattc_discovery_characteristic_t  gap_charc[BT_GATT_SRV_CLIENT_GAP_CHARC_NUMBER];
    bt_gattc_discovery_characteristic_t  gatt_charc[BT_GATT_SRV_CLIENT_GATT_CHARC_NUMBER];
    bt_gattc_discovery_descriptor_t      gatt_descriptor[BT_GATT_SRV_CLIENT_GATT_DISCRIPTOR_NUMBER];
} bt_gatt_srv_client_discovery_cache_t;

typedef struct {
    bt_gatt_srv_client_discovery_cache_t *discovery_cache;
} bt_gatt_srv_client_common_context_t;

typedef struct {
    uint16_t               value_handle;
    uint16_t               cccd_handle;
} bt_gatt_srv_client_charc_information_t;

typedef struct {
    uint32_t                                connection_handle;
    bt_gatt_srv_client_charc_information_t  *charc_info;
    bt_utils_linknode_t                     action_list;
    bt_gatt_srv_client_flag_t               flags;
    uint8_t                                 device_name_length;
    uint8_t                                 device_name[BT_GATT_SRV_CLIENT_DEVICE_NAME_LENGTH];
    uint8_t                                 central_address_resolution_support;
} bt_gatt_srv_client_context_t;

typedef struct {
    uint16_t               uuid;
    bt_gatt_srv_client_t   type;
} bt_gatt_srv_client_type_mapping_t;

typedef struct {
    bt_utils_linknode_t                node;
    bool                               is_used;
    bt_gatt_srv_client_action_t        action;
    bt_gatt_srv_client_t               type;
    bt_gatt_srv_client_enable_t        enable;
    bt_gatt_srv_client_event_callback  callback;
} bt_gatt_srv_client_action_context_t;

static bt_gatt_srv_client_context_t g_gatt_srv_client_context[BT_GATT_SRV_CLIENT_LINK_NUMBER] = {0};

static bt_gatt_srv_client_common_context_t g_common_context = {0};

static bt_gatt_srv_client_action_context_t g_action_context[BT_GATT_SRV_CLIENT_ACTION_NUMBER] = {0};

typedef bt_status_t (*bt_gatt_srv_client_common_event_callback_t)(bt_status_t status, void *buffer);

typedef struct {
    bt_msg_type_t                                    msg;
    bt_gatt_srv_client_common_event_callback_t       callback;
} bt_gatt_srv_client_handle_common_event_t;

const static bt_gatt_srv_client_type_mapping_t g_type_mapping_table[] = {
    {BT_SIG_UUID16_DEVICE_NAME, BT_GATT_SRV_CLIENT_DEVICE_NAME},
    {BT_SIG_UUID16_APPEARANCE,  BT_GATT_SRV_CLIENT_APPEARANCE},
    {BT_SIG_UUID16_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS, BT_GATT_SRV_CLIENT_PPCP},
    {BT_SIG_UUID16_CENTRAL_ADDRESS_RESOLUTION, BT_GATT_SRV_CLIENT_CENTRAL_ADDRESS_RESOLUTION},
    {BT_SIG_UUID16_RPA_ONLY, BT_GATT_SRV_CLIENT_RPA_ONLY},
    {BT_SIG_UUID16_SERVICE_CHANGED, BT_GATT_SRV_CLIENT_SERVICE_CHANGE},
    {BT_SIG_UUID16_CLIENT_SUPPORTED_FEATURES, BT_GATT_SRV_CLIENT_SUPPORTED_FEATURE},
    {BT_SIG_UUID16_DATABASE_HASH, BT_GATT_SRV_CLIENT_DATABASE_HASH},
    {BT_SIG_UUID16_SERVER_SUPPORTED_FEATURES, BT_GATT_SRV_CLIENT_SERVER_SUPPORTED_FEATURE}
};

const static bt_gatt_srv_client_event_t g_event_mapping_table[] = {
    BT_GATT_SRV_CLIENT_EVENT_READ_VALUE_COMPLTETE,
    BT_GATT_SRV_CLIENT_EVENT_WRITE_CCCD_COMPLTETE
};

static bt_gatt_srv_client_action_context_t *bt_gatt_srv_client_find_current_action(bt_gatt_srv_client_context_t *context);
static void bt_gatt_srv_client_remove_action(bt_gatt_srv_client_context_t *context, bt_gatt_srv_client_action_context_t *node);
static bt_status_t bt_gatt_srv_client_run_next_action(bt_gatt_srv_client_context_t *context);
static void bt_status_srv_client_deinit_charc_information(bt_gatt_srv_client_context_t *context);
static void bt_gatt_srv_client_clear_action(bt_gatt_srv_client_context_t *context);
static void bt_gatt_srv_client_discovery_cache_init(void);
static void bt_gatt_srv_client_discovery_cache_deinit(void);
static bt_status_t bt_gatt_srv_client_add_action(bt_gatt_srv_client_context_t *context,\
        bt_gatt_srv_client_action_t action, void *parameter);
static bool bt_gatt_srv_client_is_all_link_discovery_complete(void);
#ifdef BT_ROLE_HANDOVER_WITH_SPP_BLE
bt_status_t bt_gatt_srv_client_rho_init(void);
#endif

static bt_status_t bt_gatt_srv_client_handle_le_connected(bt_status_t status, void *buffer);
static bt_status_t bt_gatt_srv_client_handle_le_disconnected(bt_status_t status, void *buffer);
static bt_status_t bt_gatt_srv_client_handle_read_charc_rsp(bt_status_t status, void *buffer);
static bt_status_t bt_gatt_srv_client_handle_write_charc_rsp(bt_status_t status, void *buffer);
static bt_status_t bt_gatt_srv_client_handle_exchange_mtu_rsp(bt_status_t status, void *buffer);

const static bt_gatt_srv_client_handle_common_event_t g_handle_common_event_table[] = {
    {BT_GAP_LE_CONNECT_IND, bt_gatt_srv_client_handle_le_connected},
    {BT_GAP_LE_DISCONNECT_IND, bt_gatt_srv_client_handle_le_disconnected},
    {BT_GATTC_READ_CHARC, bt_gatt_srv_client_handle_read_charc_rsp},
    {BT_GATTC_WRITE_CHARC, bt_gatt_srv_client_handle_write_charc_rsp},
    {BT_GATTC_EXCHANGE_MTU, bt_gatt_srv_client_handle_exchange_mtu_rsp}
};

static bt_gatt_srv_client_context_t *bt_gatt_srv_client_get_free_context(void)
{
    for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_LINK_NUMBER; i++) {
        if (g_gatt_srv_client_context[i].connection_handle == BT_GATT_SRV_CLIENT_INVALID_HANDLE) {
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] get free context = %02x index = %d", 2 , &g_gatt_srv_client_context[i], i);
            return &g_gatt_srv_client_context[i];
        }
    }
    LOG_MSGID_E(gatt_client, "[GATT][CLIENT] get free context fail", 0);
    return NULL;
}

static bt_gatt_srv_client_context_t *bt_gatt_srv_client_find_context_by_handle(uint32_t connection_handle)
{
    for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_LINK_NUMBER; i++) {
        if (g_gatt_srv_client_context[i].connection_handle == connection_handle) {
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] find context = %02x by handle = %02x", 2 , &g_gatt_srv_client_context[i], connection_handle);
            return &g_gatt_srv_client_context[i];
        }
    }
    LOG_MSGID_E(gatt_client, "[GATT][CLIENT] find context fail by handle = %02x", 1, connection_handle);
    return NULL;
}

static void bt_gatt_srv_client_reset_context(bt_gatt_srv_client_context_t *context)
{
    bt_status_srv_client_deinit_charc_information(context);
    bt_gatt_srv_client_clear_action(context);
    bt_utils_memset(context, 0, sizeof(bt_gatt_srv_client_context_t));
}

static void bt_gatt_srv_client_init_service(void)
{
    bt_gatt_srv_client_discovery_cache_t *discovery_cache = g_common_context.discovery_cache;
    /* GAP sevice init discovery buffer*/
    discovery_cache->gap_service.charateristics = discovery_cache->gap_charc;
    discovery_cache->gap_service.characteristic_count = BT_GATT_SRV_CLIENT_GAP_CHARC_NUMBER;
    /* GATT service init discovery buffer */
    discovery_cache->gatt_service.charateristics = discovery_cache->gatt_charc;
    discovery_cache->gatt_service.characteristic_count = BT_GATT_SRV_CLIENT_GATT_CHARC_NUMBER;
    discovery_cache->gatt_charc[0].descriptor = discovery_cache->gatt_descriptor;
    discovery_cache->gatt_charc[0].descriptor_count = BT_GATT_SRV_CLIENT_GATT_DISCRIPTOR_NUMBER;
}

static void bt_gatt_srv_client_deinit_service(void)
{
    bt_gatt_srv_client_discovery_cache_t *discovery_cache = g_common_context.discovery_cache;
    bt_utils_memset(&discovery_cache->gap_service, 0, sizeof(bt_gattc_discovery_service_t));
    bt_utils_memset(&discovery_cache->gatt_service, 0, sizeof(bt_gattc_discovery_service_t));
    bt_utils_memset(&discovery_cache->gap_charc, 0, sizeof(bt_gattc_discovery_characteristic_t));
    bt_utils_memset(&discovery_cache->gatt_charc, 0, sizeof(bt_gattc_discovery_characteristic_t));
    bt_utils_memset(&discovery_cache->gatt_descriptor, 0, sizeof(bt_gattc_discovery_descriptor_t));
}

static void bt_status_srv_client_init_charc_information(bt_gatt_srv_client_context_t *context)
{
    context->charc_info = (bt_gatt_srv_client_charc_information_t *)bt_utils_memory_alloc(sizeof(bt_gatt_srv_client_charc_information_t) * BT_GATT_SRV_CLIENT_MAX_NUMBER);
    if (context->charc_info == NULL) {
        return;
    }
}

static void bt_status_srv_client_deinit_charc_information(bt_gatt_srv_client_context_t *context)
{
    if (context->charc_info != NULL) {
        bt_utils_memory_free((void *)context->charc_info);
        context->charc_info = NULL;
    } else {
        LOG_MSGID_E(gatt_client, "[GATT][CLIENT] charc informatio deinit point is NULL", 0);
    }
}

static bt_status_t bt_gatt_srv_client_handle_le_connected(bt_status_t status, void *buffer)
{
    bt_gap_le_connection_ind_t *connect_ind = (bt_gap_le_connection_ind_t *)buffer;
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] LE connect handle = %02x status = %02x", 2, connect_ind->connection_handle, status);

    if (status != BT_STATUS_SUCCESS) {
        return BT_STATUS_FAIL;
    }

    bt_gatt_srv_client_context_t *context = bt_gatt_srv_client_get_free_context();
    if (context != NULL) {
        context->connection_handle = connect_ind->connection_handle;
        bt_status_srv_client_init_charc_information(context);
        /**
         * Root cause:get device name block LEA flow
         * Solution:add get device name to discovery flow, the stored device name is returned directly when user get device name
         */
        bt_gatt_srv_client_action_read_value_t local_action = {
            .connection_handle = context->connection_handle,
            .type = BT_GATT_SRV_CLIENT_DEVICE_NAME
        };
        bt_gatt_srv_client_add_action(context, BT_GATT_SRV_CLIENT_ACTION_READ_VALUE, &local_action);

        local_action.type = BT_GATT_SRV_CLIENT_CENTRAL_ADDRESS_RESOLUTION;
        bt_gatt_srv_client_add_action(context, BT_GATT_SRV_CLIENT_ACTION_READ_VALUE, &local_action);

#ifdef AIR_SWIFT_PAIR_ENABLE
        if (connect_ind->role == BT_ROLE_SLAVE) {
            bt_gatt_srv_client_discovery_cache_init();
            bt_gattc_discovery_status_t discovery_status = bt_gattc_discovery_start(BT_GATTC_DISCOVERY_USER_COMMON_SERVICE,
                    context->connection_handle, false);
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] trigger discovery status = %02x", 1, discovery_status);
            if (discovery_status != BT_GATTC_DISCOVERY_STATUS_SUCCESS) {
                BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GATT_DISCORYED);
                BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GAP_DISCORYED);
                if (bt_gatt_srv_client_is_all_link_discovery_complete()) {
                    bt_gatt_srv_client_discovery_cache_deinit();
                }
            }
        }
#endif

    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_gatt_srv_client_handle_le_disconnected(bt_status_t status, void *buffer)
{
    bt_gap_le_disconnect_ind_t *disconnect_ind = (bt_gap_le_disconnect_ind_t *)buffer;
    bt_gatt_srv_client_context_t *context = bt_gatt_srv_client_find_context_by_handle(disconnect_ind->connection_handle);
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] LE disconnect handle = %02x", 1, disconnect_ind->connection_handle);
    if (context != NULL) {
        bt_gatt_srv_client_reset_context(context);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_gatt_srv_client_handle_read_charc_rsp(bt_status_t status, void *buffer)
{
    bt_gatt_srv_client_context_t *context = NULL;
    if (status == BT_STATUS_SUCCESS) {
        bt_gattc_read_rsp_t *read_rsp = (bt_gattc_read_rsp_t *)buffer;
        context = bt_gatt_srv_client_find_context_by_handle(read_rsp->connection_handle);
    } else {
        bt_gattc_error_rsp_t *error_rsp = (bt_gattc_error_rsp_t *)buffer;
        context = bt_gatt_srv_client_find_context_by_handle(error_rsp->connection_handle);
    }

    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    if (!BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_READING)) {
        bt_gatt_srv_client_run_next_action(context);
        return BT_STATUS_SUCCESS;
    }

    BT_GATT_SRV_CLIENT_REMOVE_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_READING);

    bt_gatt_srv_client_action_context_t *action_context = bt_gatt_srv_client_find_current_action(context);

    if (action_context != NULL) {
        if ((status == BT_STATUS_SUCCESS) && (action_context->type == BT_GATT_SRV_CLIENT_DEVICE_NAME)) {
            bt_gattc_read_rsp_t *read_rsp = (bt_gattc_read_rsp_t *)buffer;
            uint32_t device_name_length = ((read_rsp->length - 1) <= BT_GATT_SRV_CLIENT_DEVICE_NAME_LENGTH) ? (read_rsp->length - 1) : BT_GATT_SRV_CLIENT_DEVICE_NAME_LENGTH;
            bt_utils_memcpy(context->device_name, &read_rsp->att_rsp->attribute_value, device_name_length);
            context->device_name_length = device_name_length;
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] read device name length = %02x", 1, context->device_name_length);
        }

        if ((status == BT_STATUS_SUCCESS) && (action_context->type == BT_GATT_SRV_CLIENT_CENTRAL_ADDRESS_RESOLUTION)) {
            bt_gattc_read_rsp_t *read_rsp = (bt_gattc_read_rsp_t *)buffer;
            context->central_address_resolution_support = read_rsp->att_rsp->attribute_value[0];
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] read central address resolution support = %02x", 1, context->central_address_resolution_support);
        }
        bt_gatt_srv_client_remove_action(context, action_context);
    }
    
    if ((action_context != NULL) && (action_context->type != BT_GATT_SRV_CLIENT_DEVICE_NAME)) {
        if (status == BT_STATUS_SUCCESS) {
            bt_gattc_read_rsp_t *read_rsp = (bt_gattc_read_rsp_t *)buffer;
            bt_gatt_srv_client_read_value_complete_t read_value_rsp = {
                .type = action_context->type,
                .connection_handle = context->connection_handle,
                .data = (uint8_t *) &read_rsp->att_rsp->attribute_value,
                .length = read_rsp->length - 1,
            };
            action_context->callback(g_event_mapping_table[action_context->action], status, &read_value_rsp);
        } else {
            bt_gatt_srv_client_action_error_t error_event = {
                .type = action_context->type,
                .connection_handle = context->connection_handle,
            };
            action_context->callback(g_event_mapping_table[action_context->action], status, &error_event);
        }
        bt_gatt_srv_client_remove_action(context, action_context);
    }
    /* run next action */
    bt_gatt_srv_client_run_next_action(context);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_gatt_srv_client_handle_write_charc_rsp(bt_status_t status, void *buffer)
{
    bt_gatt_srv_client_context_t *context = NULL;
    if (status == BT_STATUS_SUCCESS) {
        bt_gattc_write_rsp_t *write_rsp = (bt_gattc_write_rsp_t *)buffer;
        context = bt_gatt_srv_client_find_context_by_handle(write_rsp->connection_handle);
    } else {
        bt_gattc_error_rsp_t *error_rsp = (bt_gattc_error_rsp_t *)buffer;
        context = bt_gatt_srv_client_find_context_by_handle(error_rsp->connection_handle);
    }

    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    if (!BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_WRITING)) {
        bt_gatt_srv_client_run_next_action(context);
        return BT_STATUS_SUCCESS;
    }

    BT_GATT_SRV_CLIENT_REMOVE_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_WRITING);

    /* remove action list and notify user */
    bt_gatt_srv_client_action_context_t *action_context = bt_gatt_srv_client_find_current_action(context);
    if (action_context != NULL) {
        if (status == BT_STATUS_SUCCESS) {
            bt_gatt_srv_client_write_cccd_complete_t write_cccd_rsp = {
                .type = action_context->type,
                .connection_handle = context->connection_handle,
            };
            action_context->callback(g_event_mapping_table[action_context->action], status, &write_cccd_rsp);
        } else {
            bt_gatt_srv_client_action_error_t error_event = {
                .type = action_context->type,
                .connection_handle = context->connection_handle,
            };
            action_context->callback(g_event_mapping_table[action_context->action], status, &error_event);
        }
        bt_gatt_srv_client_remove_action(context, action_context);
    }
    /* run next action */
    bt_gatt_srv_client_run_next_action(context);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_gatt_srv_client_handle_exchange_mtu_rsp(bt_status_t status, void *buffer)
{
    bt_gatt_exchange_mtu_rsp_t *mtu_rsp = (bt_gatt_exchange_mtu_rsp_t *)buffer;
    bt_gatt_srv_client_context_t *context = bt_gatt_srv_client_find_context_by_handle(mtu_rsp->connection_handle);
    if (context == NULL) {
        return BT_STATUS_FAIL;
    }

    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(mtu_rsp->connection_handle);

    if (conn_info == NULL) {
        return BT_STATUS_FAIL;
    }

    if (conn_info->role == BT_ROLE_MASTER) {
        bt_gatt_srv_client_discovery_cache_init();
        bt_gattc_discovery_status_t discovery_status = bt_gattc_discovery_start(BT_GATTC_DISCOVERY_USER_COMMON_SERVICE,
                context->connection_handle, false);
        LOG_MSGID_I(gatt_client, "[GATT][CLIENT] trigger discovery status = %02x", 1, discovery_status);
        if (discovery_status != BT_GATTC_DISCOVERY_STATUS_SUCCESS) {
            BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GATT_DISCORYED);
            BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GAP_DISCORYED);
            if (bt_gatt_srv_client_is_all_link_discovery_complete()) {
                bt_gatt_srv_client_discovery_cache_deinit();
            }
        }
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_gatt_srv_client_common_event_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_status_t result = BT_STATUS_FAIL;
    for (uint32_t i = 0; (i < sizeof(g_handle_common_event_table) >> 3); i++) {
        if (g_handle_common_event_table[i].msg == msg) {
            result = g_handle_common_event_table[i].callback(status, buffer);
        }
    }
    return result;
}

static bt_gatt_srv_client_t bt_gatt_srv_client_type_mapping_by_uuid(uint16_t uuid)
{
    for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_MAX_NUMBER; i++) {
        if (g_type_mapping_table[i].uuid == uuid) {
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] mapping type = %02x by uuid = %02x", 2,  g_type_mapping_table[i].type, uuid);
            return  g_type_mapping_table[i].type;
        }
    }
    return BT_GATT_SRV_CLIENT_NONE;
}

static bool bt_gatt_srv_client_is_all_link_discovery_complete(void)
{
    for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_LINK_NUMBER; i++) {
        bt_gatt_srv_client_context_t *context = &g_gatt_srv_client_context[i];
        LOG_MSGID_I(gatt_client, "[GATT][CLIENT] context index = %02x flag= %02x is discovering", 2, i, context->flags);
        if ((context->connection_handle != BT_GATT_SRV_CLIENT_INVALID_HANDLE) &&
                (!BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GAP_DISCORYED) || !BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GATT_DISCORYED))) {
            return false;
        }
    }
    return true;
}

static void bt_gatt_srv_client_discovery_gap_callback(bt_gattc_discovery_event_t *event)
{
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] discovery gap event type = %02x", 1, event->event_type);
    bt_gatt_srv_client_charc_information_t *charc_info = NULL;
    bt_gatt_srv_client_discovery_cache_t *discovery_cache = g_common_context.discovery_cache;

    bt_gatt_srv_client_context_t *context = bt_gatt_srv_client_find_context_by_handle(event->conn_handle);
    if (context == NULL) {
        bt_gattc_discovery_continue(BT_HANDLE_INVALID);
        return;
    }

    if (discovery_cache == NULL) {
        LOG_MSGID_I(gatt_client, "[GATT][CLIENT] discovery gap cache is NULL", 0);
        bt_gattc_discovery_continue(context->connection_handle);
        return;
    }

    if (event->event_type == BT_GATTC_DISCOVERY_EVENT_FAIL) {
        bt_gattc_discovery_continue(BT_HANDLE_INVALID);
        return;
    }

    BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GAP_DISCORYED);
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] discovery gap charc number = %02x", 1, discovery_cache->gap_service.char_count_found);
    for (uint32_t i = 0; i < discovery_cache->gap_service.char_count_found; i++) {
        bt_gattc_discovery_characteristic_t *charc = &discovery_cache->gap_service.charateristics[i];
        bt_gatt_srv_client_t type = bt_gatt_srv_client_type_mapping_by_uuid(charc->char_uuid.uuid.uuid16);
        if (type >= BT_GATT_SRV_CLIENT_MAX_NUMBER) {
            continue;
        }

        charc_info = &context->charc_info[type];
        charc_info->value_handle = charc->value_handle;
    }

    if (BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GATT_DISCORYED)) {
        BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_DISCOVERY_CONTINUE);
        bt_gatt_srv_client_run_next_action(context);
        if (bt_gatt_srv_client_is_all_link_discovery_complete()) {
            bt_gatt_srv_client_discovery_cache_deinit();
        }
    } else {
        bt_gattc_discovery_continue(context->connection_handle);
    }
}

static void bt_gatt_srv_client_discovery_gatt_callback(bt_gattc_discovery_event_t *event)
{
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] discovery gatt event type = %02x", 1, event->event_type);
    bt_gatt_srv_client_charc_information_t *charc_info = NULL;
    bt_gatt_srv_client_discovery_cache_t *discovery_cache = g_common_context.discovery_cache;

    bt_gatt_srv_client_context_t *context = bt_gatt_srv_client_find_context_by_handle(event->conn_handle);
    if (context == NULL) {
        bt_gattc_discovery_continue(BT_HANDLE_INVALID);
        return;
    }

    if (NULL == discovery_cache) {
        LOG_MSGID_E(gatt_client, "[GATT][CLIENT] discovery gatt cache is NULL", 0);
        bt_gattc_discovery_continue(context->connection_handle);
        return;
    }

    if (event->event_type == BT_GATTC_DISCOVERY_EVENT_FAIL) {
        bt_gattc_discovery_continue(BT_HANDLE_INVALID);
        return;
    }

    BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GATT_DISCORYED);
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] discovery gatt charc number = %02x", 1, discovery_cache->gatt_service.char_count_found);
    for (uint32_t i = 0; i < discovery_cache->gatt_service.char_count_found; i++) {
        bt_gattc_discovery_characteristic_t *charc = &discovery_cache->gatt_service.charateristics[i];
        bt_gatt_srv_client_t type = bt_gatt_srv_client_type_mapping_by_uuid(charc->char_uuid.uuid.uuid16);
        if (type >= BT_GATT_SRV_CLIENT_MAX_NUMBER) {
            continue;
        }

        charc_info = &context->charc_info[type];
        charc_info->value_handle = charc->value_handle;
        LOG_MSGID_I(gatt_client, "[GATT][CLIENT] gatt value handle = %02x", 1, charc_info->value_handle);
        if ((charc->descr_count_found != 0) && (charc->descriptor != NULL)) {
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] discovery gatt descriptor index = %02x handle = %02x", 2, i, charc->descriptor->handle);
            charc_info->cccd_handle = charc->descriptor->handle;
        }
    }

    if (BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GAP_DISCORYED)) {
        BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_DISCOVERY_CONTINUE);
        bt_gatt_srv_client_run_next_action(context);
        if (bt_gatt_srv_client_is_all_link_discovery_complete()) {
            bt_gatt_srv_client_discovery_cache_deinit();
        }
    } else {
        bt_gattc_discovery_continue(context->connection_handle);
    }
}

static bt_gatt_srv_client_action_context_t *bt_gatt_srv_client_alloc_action_context(void)
{
    for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_ACTION_NUMBER; i++) {
        if ((g_action_context[i].callback == NULL) && (!g_action_context[i].is_used)) {
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] alloc action context = %02x, index = %02x", 2, &g_action_context[i], i);
            g_action_context[i].is_used = true;
            return &g_action_context[i];
        }
    }
    LOG_MSGID_E(gatt_client, "[GATT][CLIENT] alloc action context fail", 0);
    return NULL;
}

static void  bt_gatt_srv_client_free_action_context(bt_gatt_srv_client_action_context_t *context)
{
    bt_utils_memset(context, 0, sizeof(bt_gatt_srv_client_action_context_t));
}

static bt_status_t bt_gatt_srv_client_add_action(bt_gatt_srv_client_context_t *context,
        bt_gatt_srv_client_action_t action, void *parameter)
{
    bt_gatt_srv_client_action_context_t *action_context = bt_gatt_srv_client_alloc_action_context();
    if (action_context == NULL) {
        return BT_STATUS_FAIL;
    }

    switch (action) {
        case BT_GATT_SRV_CLIENT_ACTION_READ_VALUE: {
            bt_gatt_srv_client_action_read_value_t *read_value = (bt_gatt_srv_client_action_read_value_t *)parameter;
            action_context->type = read_value->type;
            action_context->callback = read_value->callback;
        }
        break;
        case BT_GATT_SRV_CLIENT_ACTION_WRITE_CCCD: {
            bt_gatt_srv_client_action_write_cccd_t *write_cccd = (bt_gatt_srv_client_action_write_cccd_t *)parameter;
            action_context->type = write_cccd->type;
            action_context->callback = write_cccd->callback;
            action_context->enable = write_cccd->enable;
        }
        break;
        default:
            break;
    }
    action_context->action = action;
    bt_utils_srv_linknode_insert_node(&context->action_list, (bt_utils_linknode_t *)action_context, BT_UTILS_SRV_NODE_BACK);
    return BT_STATUS_SUCCESS;
}

static void bt_gatt_srv_client_remove_action(bt_gatt_srv_client_context_t *context, bt_gatt_srv_client_action_context_t *node)
{
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] remove context = %02x action context = %02x", 2, context, node);
    bt_utils_srv_linknode_remove_node(&context->action_list, (bt_utils_linknode_t *)node);
    bt_gatt_srv_client_free_action_context(node);
}

static void bt_gatt_srv_client_clear_action(bt_gatt_srv_client_context_t *context)
{
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] clear context = %02x action list", 1, context);
    bt_gatt_srv_client_action_context_t *action_context = (bt_gatt_srv_client_action_context_t *)context->action_list.front;
    while (action_context != NULL) {
        bt_utils_srv_linknode_remove_node(&context->action_list, (bt_utils_linknode_t *)action_context);
        bt_gatt_srv_client_free_action_context(action_context);
        action_context = (bt_gatt_srv_client_action_context_t *)action_context->node.front;
    }
}

static bt_gatt_srv_client_action_context_t *bt_gatt_srv_client_find_current_action(bt_gatt_srv_client_context_t *context)
{
    return (bt_gatt_srv_client_action_context_t *)context->action_list.front;
}

static bool bt_gatt_srv_client_handle_is_valid(bt_gatt_srv_client_context_t *context, bt_gatt_srv_client_action_context_t *action_context)
{
    bool is_valid = false;
    switch (action_context->action) {
        case BT_GATT_SRV_CLIENT_ACTION_READ_VALUE: {
            if (context->charc_info[action_context->type].value_handle != BT_GATT_SRV_CLIENT_INVALID_HANDLE) {
                is_valid = true;
            }
        }
        break;
        case BT_GATT_SRV_CLIENT_ACTION_WRITE_CCCD: {
            if (context->charc_info[action_context->type].cccd_handle != BT_GATT_SRV_CLIENT_INVALID_HANDLE) {
                is_valid = true;
            }
        }
        break;
        default:
            break;
    }
    return is_valid;
}

#ifdef BT_ROLE_HANDOVER_WITH_SPP_BLE
static bool bt_gatt_srv_client_is_rho_pending(void)
{
    for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_LINK_NUMBER; i++) {
        bt_gatt_srv_client_context_t *context = &g_gatt_srv_client_context[i];
        if (BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_RHO_PENDING)) {
            return true;
        }
    }
    return false;
}
#endif

static bt_status_t bt_gatt_srv_client_run_next_action(bt_gatt_srv_client_context_t *context)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_gatt_srv_client_action_context_t *action_context = (bt_gatt_srv_client_action_context_t *)context->action_list.front;

    do {
        if (action_context == NULL) {
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] not have action need to run", 0);
            if (BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_DISCOVERY_CONTINUE)) {
                BT_GATT_SRV_CLIENT_REMOVE_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_DISCOVERY_CONTINUE);
                bt_gattc_discovery_continue(context->connection_handle);
            }
#ifdef BT_ROLE_HANDOVER_WITH_SPP_BLE
            if (bt_gatt_srv_client_is_rho_pending()) {
                LOG_MSGID_I(gatt_client, "[GATT][CLIENT] reply rho prepare request", 0);
                bt_role_handover_reply_prepare_request(BT_ROLE_HANDOVER_MODULE_GATT_SRV_CLIENT);
            }
#endif
            return BT_STATUS_FAIL;
        }

        LOG_MSGID_I(gatt_client, "[GATT][CLIENT] run context = %02x action context = %02x action = %02x type = %02x", 4,
                    context, action_context, action_context->action, action_context->type);
        if (bt_gatt_srv_client_handle_is_valid(context, action_context)) {
            break;
        }

        LOG_MSGID_W(gatt_client, "[GATT][CLIENT] attribute handle is invalid", 0);
        bt_gatt_srv_client_action_error_t error_event = {
            .type = action_context->type,
            .connection_handle = context->connection_handle,
        };

        if (action_context->callback != NULL) {
            action_context->callback(g_event_mapping_table[action_context->action], BT_STATUS_FAIL, &error_event);
        }
        bt_gatt_srv_client_remove_action(context, action_context);
        action_context = (bt_gatt_srv_client_action_context_t *)context->action_list.front;
    } while (1);

    switch (action_context->action) {
        case BT_GATT_SRV_CLIENT_ACTION_READ_VALUE: {
            BT_GATTC_NEW_READ_CHARC_REQ(read_value, context->charc_info[action_context->type].value_handle);
            status = bt_gattc_read_charc(context->connection_handle, &read_value);
            if (status == BT_STATUS_SUCCESS) {
                BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_READING);
            }
        }
        break;
        case BT_GATT_SRV_CLIENT_ACTION_WRITE_CCCD: {
            uint8_t send_buffer[10] = {0};
            uint16_t enable_cccd_value = (action_context->enable == BT_GATT_SRV_CLIENT_ENABLE) ? 0x0002 : 0x0000;
            BT_GATTC_NEW_WRITE_CHARC_REQ(write_value, send_buffer, context->charc_info[action_context->type].cccd_handle, &enable_cccd_value, sizeof(uint16_t))
            status = bt_gattc_write_charc(context->connection_handle, &write_value);
            if (status == BT_STATUS_SUCCESS) {
                BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_WRITING);
            }
        }
        break;
        default:
            break;
    }

    if ((status != BT_STATUS_SUCCESS) && (BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_DISCOVERY_CONTINUE))) {
        BT_GATT_SRV_CLIENT_REMOVE_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_DISCOVERY_CONTINUE);
        bt_gattc_discovery_continue(context->connection_handle);
    }
    return status;
}

static bool bt_gatt_srv_client_is_busy(bt_gatt_srv_client_context_t *context)
{
    if ((!BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GATT_DISCORYED)) ||
            (!BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_GAP_DISCORYED)) ||
            BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_READING) ||
            BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_WRITING) || (context->action_list.front != NULL)) {
        LOG_MSGID_I(gatt_client, "[GATT][CLIENT] action is busy flag = %02x, action front = %02x", 2, context->flags, context->action_list.front);
        return true;
    }
    return false;
}

static void bt_gatt_srv_client_discovery_cache_init(void)
{
    bt_gattc_discovery_status_t discovery_status = BT_GATTC_DISCOVERY_STATUS_FAIL;
    bt_gattc_discovery_user_data_t user_data = {0};

    if (g_common_context.discovery_cache != NULL) {
        LOG_MSGID_W(gatt_client, "[GATT][CLIENT] discovery cache had init", 0);
        return;
    }

    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] discovery cache init", 0);

    g_common_context.discovery_cache  = (bt_gatt_srv_client_discovery_cache_t *)bt_utils_memory_alloc(sizeof(bt_gatt_srv_client_discovery_cache_t));
    if (g_common_context.discovery_cache == NULL) {
        return;
    }
    bt_gatt_srv_client_discovery_cache_t *discovery_cache = g_common_context.discovery_cache;
    user_data.uuid.type = BLE_UUID_TYPE_16BIT;
    user_data.uuid.uuid.uuid16 = BT_GATT_UUID16_GAP_SERVICE;
    user_data.srv_info = &discovery_cache->gap_service;
    user_data.need_cache = true;
    user_data.handler = bt_gatt_srv_client_discovery_gap_callback;
    discovery_status = bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_COMMON_SERVICE, &user_data);
    if (discovery_status != BT_GATTC_DISCOVERY_STATUS_SUCCESS) {
        LOG_MSGID_E(gatt_client, "[GATT][CLIENT] register gap dicovery fail status = %02x", 1, discovery_status);
        return;
    }

    user_data.uuid.type = BLE_UUID_TYPE_16BIT;
    user_data.uuid.uuid.uuid16 = BT_GATT_UUID16_GATT_SERVICE;
    user_data.srv_info = &discovery_cache->gatt_service;
    user_data.need_cache = true;
    user_data.handler = bt_gatt_srv_client_discovery_gatt_callback;
    discovery_status = bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_COMMON_SERVICE, &user_data);
    if (discovery_status != BT_GATTC_DISCOVERY_STATUS_SUCCESS) {
        LOG_MSGID_E(gatt_client, "[GATT][CLIENT] register gatt dicovery fail status = %02x", 1, discovery_status);
        return;
    }
    bt_gatt_srv_client_init_service();
}

static void bt_gatt_srv_client_discovery_cache_deinit(void)
{
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] discovery cache deinit", 0);
    if (g_common_context.discovery_cache != NULL) {
        bt_gatt_srv_client_deinit_service();
        bt_utils_memory_free(g_common_context.discovery_cache);
        g_common_context.discovery_cache = NULL;
    } else {
        LOG_MSGID_E(gatt_client, "[GATT][CLIENT] discovery cache deinit point is NULL", 0);
    }
}

bt_status_t bt_gatt_srv_client_init(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    status = bt_callback_manager_register_callback(bt_callback_type_app_event,
             MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_SYSTEM, bt_gatt_srv_client_common_event_callback);
#ifdef BT_ROLE_HANDOVER_WITH_SPP_BLE
    status = bt_gatt_srv_client_rho_init();
#endif
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT] init status %02x", 1, status);
    return status;
}

bt_status_t bt_gatt_srv_client_deinit(void)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_gatt_srv_client_send_action(bt_gatt_srv_client_action_t action, void *parameter, uint32_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (action) {
        case BT_GATT_SRV_CLIENT_ACTION_READ_VALUE: {
            bt_gatt_srv_client_action_read_value_t *read_value = (bt_gatt_srv_client_action_read_value_t *)parameter;
            bt_gatt_srv_client_context_t *context = bt_gatt_srv_client_find_context_by_handle(read_value->connection_handle);
            if (context == NULL) {
                break;
            }

            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] send context = %02x action = %02x type = %02x", 3, context, action, read_value->type);

            /**
             * Root cause:get device name block LEA flow
             * Solution:add get device name to discovery flow, the stored device name is returned directly when user get device name
             */
            if (read_value->type == BT_GATT_SRV_CLIENT_DEVICE_NAME) {
                if (context->device_name_length != 0) {
                    bt_gatt_srv_client_read_value_complete_t read_value_complete = {
                        .type = BT_GATT_SRV_CLIENT_DEVICE_NAME,
                        .connection_handle = context->connection_handle,
                        .data = context->device_name,
                        .length = context->device_name_length
                    };
                    read_value->callback(BT_GATT_SRV_CLIENT_EVENT_READ_VALUE_COMPLTETE, BT_STATUS_SUCCESS, &read_value_complete);
                    return BT_STATUS_SUCCESS;
                }
                LOG_MSGID_E(gatt_client, "[GATT][CLIENT] get device name fail", 0);
                return BT_STATUS_FAIL;
            }

            if (read_value->type == BT_GATT_SRV_CLIENT_CENTRAL_ADDRESS_RESOLUTION) {
                bt_gatt_srv_client_read_value_complete_t read_value_complete = {
                    .type = BT_GATT_SRV_CLIENT_CENTRAL_ADDRESS_RESOLUTION,
                    .connection_handle = context->connection_handle,
                    .data = &context->central_address_resolution_support,
                    .length = 1
                };
                read_value->callback(BT_GATT_SRV_CLIENT_EVENT_READ_VALUE_COMPLTETE, BT_STATUS_SUCCESS, &read_value_complete);
                return BT_STATUS_SUCCESS;
            }

            if (!bt_gatt_srv_client_is_busy(context)) {
                BT_GATTC_NEW_READ_CHARC_REQ(remote_read_value, context->charc_info[read_value->type].value_handle);
                status = bt_gattc_read_charc(context->connection_handle, &remote_read_value);

                if (status == BT_STATUS_SUCCESS) {
                    BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_READING);
                }
            }
            status = bt_gatt_srv_client_add_action(context, action, parameter);
        }
        break;
        case BT_GATT_SRV_CLIENT_ACTION_WRITE_CCCD: {
            bt_gatt_srv_client_action_write_cccd_t *write_cccd = (bt_gatt_srv_client_action_write_cccd_t *)parameter;
            bt_gatt_srv_client_context_t *context = bt_gatt_srv_client_find_context_by_handle(write_cccd->connection_handle);
            if (context == NULL) {
                break;
            }
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT] send context = %02x action = %02x type = %02x", 3, context, action, write_cccd->type);
            if (!bt_gatt_srv_client_is_busy(context)) {
                uint8_t send_buffer[10] = {0};
                uint16_t enable_cccd_value = (write_cccd->enable == BT_GATT_SRV_CLIENT_ENABLE) ? 0x0001 : 0x0000;
                BT_GATTC_NEW_WRITE_CHARC_REQ(write_value, send_buffer, context->charc_info[write_cccd->type].cccd_handle, &enable_cccd_value, sizeof(uint16_t))
                status = bt_gattc_write_charc(context->connection_handle, &write_value);
                if (status == BT_STATUS_SUCCESS) {
                    BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_WRITING);
                }
            }
            status = bt_gatt_srv_client_add_action(context, action, parameter);
        }
        break;
        default:
            LOG_MSGID_W(gatt_client, "[GATT][CLIENT] send action error = %02x", 1, action);
            break;
    }
    return status;
}

#ifdef BT_ROLE_HANDOVER_WITH_SPP_BLE
typedef struct {
    uint32_t                                connection_handle;
} bt_gatt_srv_client_rho_context_t;

static bt_status_t bt_gatt_srv_client_rho_allowed_cb(const bt_bd_addr_t *addr)
{
#ifdef AIR_MULTI_POINT_ENABLE
    if (addr != NULL) {
        LOG_MSGID_I(gatt_client, "[GATT][CLIENT][RHO] is allow addr != NULL for EMP", 0);
        return BT_STATUS_SUCCESS;
    }
#endif

    for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_LINK_NUMBER; i++) {
        bt_gatt_srv_client_context_t *context = &g_gatt_srv_client_context[i];
        bt_gatt_srv_client_action_context_t *action_context = (bt_gatt_srv_client_action_context_t *)context->action_list.front;
        bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(context->connection_handle);
        if ((conn_info != NULL) && (context->connection_handle != BT_GATT_SRV_CLIENT_INVALID_HANDLE) && (action_context != NULL)\
                && (!(conn_info->attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NOT_NEED_RHO))) {
            LOG_MSGID_W(gatt_client, "[GATT][CLIENT][RHO] pending rho by context = %02x, index = %02x", 2, context, i);
            BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_RHO_PENDING);
            return BT_STATUS_PENDING;
        }
    }
    return BT_STATUS_SUCCESS;
}

static uint8_t bt_gatt_srv_client_rho_get_data_length_cb(const bt_bd_addr_t *addr)
{
    uint32_t rho_length = 0;
#ifdef AIR_MULTI_POINT_ENABLE
    if (addr != NULL) {
        LOG_MSGID_I(gatt_client, "[GATT][CLIENT][RHO] get data length addr != NULL for EMP", 0);
        return 0;
    }
#endif
    for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_LINK_NUMBER; i++) {
        bt_gatt_srv_client_context_t *context = &g_gatt_srv_client_context[i];
        bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(context->connection_handle);
        if ((conn_info != NULL) && (context->connection_handle != BT_GATT_SRV_CLIENT_INVALID_HANDLE)\
                && (!(conn_info->attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NOT_NEED_RHO))) {
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT][RHO] get data length context = %02x, index = %02x", 2, context, i);
            rho_length += sizeof(bt_gatt_srv_client_rho_context_t);
        }
    }
    LOG_MSGID_I(gatt_client, "[GATT][CLIENT][RHO] get data length = %02x", 1, rho_length);
    return rho_length;
}

static bt_status_t bt_gatt_srv_client_rho_get_data_cb(const bt_bd_addr_t *addr, void *data)
{
    uint32_t rho_index = 0;
    if (data == NULL) {
        return BT_STATUS_FAIL;
    }

#ifdef AIR_MULTI_POINT_ENABLE
    if (addr != NULL) {
        LOG_MSGID_I(gatt_client, "[GATT][CLIENT][RHO] get data addr != NULL for EMP", 0);
        return BT_STATUS_FAIL;
    }
#endif
    for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_LINK_NUMBER; i++) {
        bt_gatt_srv_client_context_t *context = &g_gatt_srv_client_context[i];
        bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(context->connection_handle);
        if ((conn_info != NULL) && (context->connection_handle != BT_GATT_SRV_CLIENT_INVALID_HANDLE)\
                && (!(conn_info->attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NOT_NEED_RHO))) {
            bt_gatt_srv_client_rho_context_t *rho_context = (bt_gatt_srv_client_rho_context_t *)(data + sizeof(bt_gatt_srv_client_rho_context_t) * rho_index);
            rho_context->connection_handle = context->connection_handle;
            rho_index++;
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT][RHO] get data context = %02x, index = %02x connection handle = %02x", 3, context, i, rho_context->connection_handle);
        }
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_gatt_srv_client_rho_update_data_cb(bt_role_handover_update_info_t *info)
{
    if (info == NULL) {
        return BT_STATUS_FAIL;
    }

    if (BT_AWS_MCE_ROLE_PARTNER == info->role) {
        uint32_t data_num = info->length / sizeof(bt_gatt_srv_client_rho_context_t);
        for (uint32_t i = 0; i < data_num; i++) {
            bt_gatt_srv_client_rho_context_t *rho_context = (bt_gatt_srv_client_rho_context_t *)(info->data + sizeof (bt_gatt_srv_client_rho_context_t) * i);
            bt_gatt_srv_client_context_t *context = bt_gatt_srv_client_get_free_context();
            if (context != NULL) {
                context->connection_handle = bt_gap_le_srv_get_handle_by_old_handle(rho_context->connection_handle);
                LOG_MSGID_I(gatt_client, "[GATT][CLIENT][RHO] update old handle = %02x new handle", 2,
                            rho_context->connection_handle, context->connection_handle);
                BT_GATT_SRV_CLIENT_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_RHO_DISCOVERY_TRIGGER);
            }
        }
    } else if (BT_AWS_MCE_ROLE_AGENT == info->role) {
        for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_LINK_NUMBER; i++) {
            bt_gatt_srv_client_context_t *context = &g_gatt_srv_client_context[i];
            bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(context->connection_handle);
            if ((conn_info != NULL) && (context->connection_handle != BT_GATT_SRV_CLIENT_INVALID_HANDLE)\
                    && (!(conn_info->attribute & BT_GAP_LE_SRV_LINK_ATTRIBUTE_NOT_NEED_RHO))) {
                bt_gatt_srv_client_reset_context(context);
            }
        }
    }
    return BT_STATUS_SUCCESS;
}

static void bt_gatt_srv_client_rho_status_cb(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status)
{
    switch (event) {
        case BT_ROLE_HANDOVER_COMPLETE_IND: {
            LOG_MSGID_I(gatt_client, "[GATT][CLIENT][RHO] rho complete role = %02x status = %02x", 2, role, status);
            for (uint32_t i = 0; i < BT_GATT_SRV_CLIENT_LINK_NUMBER; i++) {
                bt_gatt_srv_client_context_t *context = &g_gatt_srv_client_context[i];
                if ((role == BT_AWS_MCE_ROLE_PARTNER) && (BT_GATT_SRV_CLIENT_IS_SET_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_RHO_DISCOVERY_TRIGGER))) {
                    BT_GATT_SRV_CLIENT_REMOVE_FLAG(context, BT_GATT_SRV_CLIENT_FLAG_RHO_DISCOVERY_TRIGGER);
                    /* re-trigger discovery GAP/GATT service. */
                    bt_status_srv_client_init_charc_information(context);
                    bt_gatt_srv_client_discovery_cache_init();
                    bt_gattc_discovery_status_t discovery_status = bt_gattc_discovery_start(BT_GATTC_DISCOVERY_USER_COMMON_SERVICE,
                            context->connection_handle, false);
                    LOG_MSGID_I(gatt_client, "[GATT][CLIENT][RHO] trigger discovery status = %02x", 1, discovery_status);
                }
            }
        }
        break;
        default:
            break;
    }
}

bt_role_handover_callbacks_t bt_gatt_srv_client_rho_callbacks = {
    .allowed_cb = bt_gatt_srv_client_rho_allowed_cb,             /*optional if always allowed. */
    .get_len_cb = bt_gatt_srv_client_rho_get_data_length_cb,     /* optional if no RHO data to partner. */
    .get_data_cb = bt_gatt_srv_client_rho_get_data_cb,           /* optional if no RHO data to partner. */
    .update_cb = bt_gatt_srv_client_rho_update_data_cb,          /* optional if no RHO data to partner. */
    .status_cb = bt_gatt_srv_client_rho_status_cb                /* Mandatory for all users. */
};

bt_status_t bt_gatt_srv_client_rho_init(void)
{
    return bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_GATT_SRV_CLIENT, &bt_gatt_srv_client_rho_callbacks);
}
#endif
