/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO ObtAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */
#include "hal_platform.h"
#include "hal_nvic.h"
#include "mux.h"
#include "mux_port_device.h"
#include "assert.h"
#include "mux_port.h"
#ifdef MTK_GATT_OVER_BREDR_ENABLE
#include "gatt_over_bredr_air.h"
#endif
#ifdef MTK_AIRUPDATE_ENABLE
#include "airupdate_interface.h"
#endif
#ifdef MTK_MUX_BT_ENABLE
#include "bt_type.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "ble_air_interface.h"
#include "spp_air_interface.h"
#include "bt_utils.h"
#include "mux_bt.h"
log_create_module(MUX_BT, PRINT_LEVEL_INFO);
#define MUX_BT_INVAILD_HANDLE    0x00000000

#define MUX_BT_INDEX_TO_MUX_PORT(port_index) (port_index + MUX_BT_BEGIN)

typedef struct {
    mux_status_t        tx_status;
    uint16_t            tx_complete_size;
    SemaphoreHandle_t   mux_bt_semaphore;
} mux_bt_context_t;

typedef enum {
    MUX_BT_PORT_TYPE_SPP = 0,
    MUX_BT_PORT_TYPE_BLE,
#ifdef MTK_GATT_OVER_BREDR_ENABLE
    MUX_BT_PORT_TYPE_GATT_OVER_BREDR,
#endif
#ifdef MTK_AIRUPDATE_ENABLE
    MUX_BT_PORT_TYPE_AIRUPDATE,
#endif
    MUX_BT_PORT_TYPE_MAX,
} mux_bt_port_t;

typedef struct {
    uint32_t                         connection_handle;
    uint16_t                         mtu_size;
    bt_bd_addr_t                     peer_address;
    bool                             is_open;
    mux_bt_port_t                    type;
    mux_irq_handler_t                mux_callback;
    mux_port_t                       port_index;
    bool                             is_point_inited;
    virtual_read_write_point_t       rw_point;
    mux_port_config_t                *local_setting;
    uint8_t                          rx_not_enough_count;
} mux_bt_port_context_t;

typedef struct {
    bool                    is_air_inited;
    mux_bt_port_t           type;
    uint8_t                 index_start;
    uint8_t                 index_end;
} mux_bt_port_group_context_t;

static mux_bt_port_group_context_t mux_port_group[MUX_BT_PORT_TYPE_MAX] = {
    {false, MUX_BT_PORT_TYPE_SPP, MUX_BT_SPP - MUX_BT_BEGIN, MUX_BT_SPP - MUX_BT_BEGIN},
    {false, MUX_BT_PORT_TYPE_BLE, MUX_BT_BLE - MUX_BT_BEGIN, MUX_BT_BLE_2 - MUX_BT_BEGIN},
#ifdef MTK_GATT_OVER_BREDR_ENABLE
    {false, MUX_BT_PORT_TYPE_GATT_OVER_BREDR, MUX_BT_GATT_OVER_BREDR - MUX_BT_BEGIN, MUX_BT_GATT_OVER_BREDR - MUX_BT_BEGIN},
#endif
#ifdef MTK_AIRUPDATE_ENABLE
    {false, MUX_BT_PORT_TYPE_AIRUPDATE, MUX_BT_AIRUPATE - MUX_BT_BEGIN, MUX_BT_AIRUPATE - MUX_BT_BEGIN},
#endif
};

static mux_bt_port_context_t mux_port_context[MUX_BT_END - MUX_BT_BEGIN + 1] = {0};
static mux_bt_context_t mux_bt_context = {0};

static void mux_bt_set_rx_hw_wptr_internal_use(uint8_t port_index, uint32_t move_bytes);
static void mux_bt_set_tx_hw_rptr_internal_use(uint8_t port_index, uint32_t move_bytes);

static mux_bt_port_context_t *mux_bt_get_free_port_context(mux_bt_port_t type)
{
    uint32_t i = 0;
    mux_bt_port_group_context_t *port_group = &mux_port_group[type];
    for (i = port_group->index_start; i <= port_group->index_end; i++) {
        if (mux_port_context[i].connection_handle == MUX_BT_INVAILD_HANDLE) {
            LOG_MSGID_I(MUX_BT, "[MUX][BT] get free context index = %02x, type = %02x", 2, i, type);
            mux_port_context[i].port_index = i;
            return &mux_port_context[i];
        }
    }
    LOG_MSGID_I(MUX_BT, "[MUX][BT] cannot get free context by type = %02x", 1, type);
    return NULL;
}

static void mux_bt_reset_port_context_conn_info(mux_bt_port_context_t *context)
{
    context->connection_handle = MUX_BT_INVAILD_HANDLE;
    context->mtu_size = 0;
    bt_utils_memset(&context->peer_address, 0, sizeof(bt_bd_addr_t));
}

static mux_bt_port_context_t *mux_bt_find_port_context_by_handle(mux_bt_port_t type, uint32_t connection_handle)
{
    uint32_t i = 0;
    mux_bt_port_group_context_t *port_group = &mux_port_group[type];
    for (i = port_group->index_start; i <= port_group->index_end; i++) {
        if (mux_port_context[i].connection_handle == connection_handle) {
            LOG_MSGID_I(MUX_BT, "[MUX][BT] find context index:%02x by handle:%02x, type = %02x", 3, i, connection_handle, type);
            return &mux_port_context[i];
        }
    }
    LOG_MSGID_I(MUX_BT, "[MUX][BT] not find context by handle = %02x, type = %02x", 2, connection_handle, type);
    return NULL;
}

static void mux_bt_reset_connected_port_context_info(mux_bt_port_t type, uint32_t connection_handle, bt_bd_addr_t peer_address)
{
    if (NULL == peer_address) {
        LOG_MSGID_I(MUX_BT, "peer_address is null", 0);
        return;
    }
    uint32_t i = 0;
    mux_bt_port_group_context_t *port_group = &mux_port_group[type];
    for (i = port_group->index_start; i <= port_group->index_end; i++) {
        if (bt_utils_memcmp(&mux_port_context[i].peer_address, peer_address, sizeof(bt_bd_addr_t)) == 0) {
            if (mux_port_context[i].connection_handle != MUX_BT_INVAILD_HANDLE && connection_handle != mux_port_context[i].connection_handle\
            && connection_handle != MUX_BT_INVAILD_HANDLE) {
                LOG_MSGID_I(MUX_BT, "mux_bt_reset_port_context_conn_info = %02x,%02x", 2, type, connection_handle);
                mux_bt_reset_port_context_conn_info(&mux_port_context[i]);
            }
        }
    }
}

static void mux_bt_rx_packet_handle(mux_bt_port_context_t *context, uint8_t *data, uint32_t length)
{
    virtual_read_write_point_t *ring_point = &context->rw_point;
    uint32_t next_free_block_len = 0;
    uint32_t per_cpu_irq_mask = 0;
    if (length > (ring_point->rx_buff_end - ring_point->rx_buff_start - ring_point->rx_buff_available_len)) {
        context->rx_not_enough_count++;
        if (context->rx_not_enough_count >= 2) {
            ring_point->rx_buff_read_point =  ring_point->rx_buff_start;
            ring_point->rx_buff_write_point =  ring_point->rx_buff_start;
            ring_point->rx_buff_available_len = 0;
            ring_point->rx_receiving_write_point = 0xFFFFFFFF;
            LOG_MSGID_E(MUX_BT, "[MUX][BT] rx buffer not enough to save, rx_read_point = %02x, rx_write_point = %02x", 2, \
                        ring_point->rx_buff_read_point, ring_point->rx_buff_write_point);
        }
        LOG_MSGID_E(MUX_BT, "[MUX][BT] rx buffer not enough to save, length: %02x, rx_not_enough_count = %d", 2, length, context->rx_not_enough_count);
    }
    context->rx_not_enough_count = 0;
    next_free_block_len = mux_common_device_get_buf_next_free_block_len(ring_point->rx_buff_start, ring_point->rx_buff_read_point, \
                          ring_point->rx_buff_write_point, ring_point->rx_buff_end, ring_point->rx_buff_available_len);
    if (next_free_block_len >= length) {
        bt_utils_memcpy((void *)(ring_point->rx_buff_write_point), data, length);
    } else {
        bt_utils_memcpy((void *)(ring_point->rx_buff_write_point), data, next_free_block_len);
        bt_utils_memcpy((void *)(ring_point->rx_buff_start), data + next_free_block_len, length - next_free_block_len);
    }
    port_mux_local_cpu_enter_critical(&per_cpu_irq_mask);
    mux_bt_set_rx_hw_wptr_internal_use(context->port_index, length);
    mux_driver_debug_for_check(ring_point);
    port_mux_local_cpu_exit_critical(per_cpu_irq_mask);
    context->mux_callback(MUX_BT_INDEX_TO_MUX_PORT(context->port_index), MUX_EVENT_READY_TO_READ, NULL);
}

static void mux_bt_port_le_event_callback(ble_air_event_t event, void *parameter)
{
    switch (event) {
        case BLE_AIR_EVENT_CONNECT_IND: {
            ble_air_connect_t *conn_ind = (ble_air_connect_t *)parameter;
            mux_bt_reset_connected_port_context_info(MUX_BT_PORT_TYPE_BLE, conn_ind->conn_handle, conn_ind->bdaddr);
            if (NULL != mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_BLE, conn_ind->conn_handle)) {
                break;
            }
            LOG_MSGID_I(MUX_BT, "[MUX][BT] le_event_callback connect handle = %02x", 1, conn_ind->conn_handle);
            mux_bt_port_context_t *context = mux_bt_get_free_port_context(MUX_BT_PORT_TYPE_BLE);
            if (context != NULL) {
                context->type = MUX_BT_PORT_TYPE_BLE;
                context->connection_handle = conn_ind->conn_handle;
                bt_utils_memcpy(&context->peer_address, &conn_ind->bdaddr, sizeof(bt_bd_addr_t));
                if (context->is_open) {
                    mux_bt_connect_ind_t ind = {0};
                    ind.handle = conn_ind->conn_handle;
                    ind.max_packet_length = 0;
                    bt_utils_memcpy(&ind.address, &conn_ind->bdaddr, sizeof(bt_bd_addr_t));
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] le port connect handle = %02x ,port_index =%02x, addr: %x:%x:%x:%x:%x:%x", 8, conn_ind->conn_handle ,context->port_index,
                        ind.address[0], ind.address[1], ind.address[2], ind.address[3], ind.address[4], ind.address[5]);
                    context->mux_callback(MUX_BT_INDEX_TO_MUX_PORT(context->port_index), MUX_EVENT_CONNECTION, &ind);
                }
            }
        }
        break;
        case BLE_AIR_EVENT_DISCONNECT_IND: {
            ble_air_disconnect_t *dis_ind = (ble_air_disconnect_t *)parameter;
            mux_bt_port_context_t *context = mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_BLE, dis_ind->conn_handle);
            if (context != NULL) {
                mux_bt_reset_port_context_conn_info(context);
                if (context->is_open) {
                    mux_bt_disconnect_ind_t ind = {0};
                    ind.handle = dis_ind->conn_handle;
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] le port disconnect handle = %02x", 1, dis_ind->conn_handle);
                    context->mux_callback(MUX_BT_INDEX_TO_MUX_PORT(context->port_index), MUX_EVENT_DISCONNECTION, &ind);
                }
            }
        }
        break;
        case BLE_AIR_EVENT_READY_TO_READ_IND: {
            ble_air_ready_to_read_t *ready_read = (ble_air_ready_to_read_t *)parameter;
            mux_bt_port_context_t *context = mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_BLE, ready_read->conn_handle);
            if ((context != NULL) && (context->is_open)) {
                virtual_read_write_point_t *ring_point = &context->rw_point;
                uint8_t *read_data = (uint8_t *)port_mux_malloc(ring_point->rx_buff_len);
                if (read_data == NULL) {
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] read port index = %02x, handle = %02x malloc fail", 2, context->port_index, ready_read->conn_handle);
                    break;
                }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
                uint32_t ret_length = ble_air_srv_read_data(ready_read->conn_handle, read_data, ring_point->rx_buff_len);
#else
                uint32_t ret_length = ble_air_read_data(ready_read->conn_handle, read_data, ring_point->rx_buff_len);
#endif
                if (ret_length == 0) {
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] read port index = %02x, handle = %02x size is 0", 2, context->port_index, ready_read->conn_handle);
                    port_mux_free(read_data);
                    break;
                }
                mux_bt_rx_packet_handle(context, read_data, ret_length);
                port_mux_free(read_data);
            }
        }
        break;
        default:
            break;
    }
}

static void mux_bt_port_spp_event_callback(spp_air_event_t event_id, void *param)
{
    switch (event_id) {
        case SPP_AIR_CONNECT_IND: {
            spp_air_connect_ind_t *conn_ind = (spp_air_connect_ind_t *)param;
            mux_bt_reset_connected_port_context_info(MUX_BT_PORT_TYPE_SPP, conn_ind->handle, conn_ind->address);

            if (NULL != mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_SPP, conn_ind->handle)) {
                break;
            }
            mux_bt_port_context_t *context = mux_bt_get_free_port_context(MUX_BT_PORT_TYPE_SPP);
            if (context != NULL) {
                context->type = MUX_BT_PORT_TYPE_SPP;
                context->connection_handle = conn_ind->handle;
                context->mtu_size = conn_ind->max_packet_length;
                bt_utils_memcpy(&context->peer_address, &conn_ind->address, sizeof(bt_bd_addr_t));
                if (context->is_open) {
                    mux_bt_connect_ind_t ind = {0};
                    ind.handle = conn_ind->handle;;
                    ind.max_packet_length = conn_ind->max_packet_length;;
                    bt_utils_memcpy(&ind.address, &conn_ind->address, sizeof(bt_bd_addr_t));
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] spp port connect handle = %02x", 1, conn_ind->handle);
                    context->mux_callback(MUX_BT_INDEX_TO_MUX_PORT(context->port_index), MUX_EVENT_CONNECTION, &ind);
                }
            }
        }
        break;
        case SPP_AIR_DISCONNECT_IND: {
            spp_air_disconnect_ind_t *dis_ind = (spp_air_disconnect_ind_t *)param;
            mux_bt_port_context_t *context = mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_SPP, dis_ind->handle);
            if (context != NULL) {
                mux_bt_reset_port_context_conn_info(context);
                if (context->is_open) {
                    mux_bt_disconnect_ind_t ind = {0};
                    ind.handle = dis_ind->handle;
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] spp port disconnect handle = %02x", 1, dis_ind->handle);
                    context->mux_callback(MUX_BT_INDEX_TO_MUX_PORT(context->port_index), MUX_EVENT_DISCONNECTION, &ind);
                }
            }
        }
        break;
        case SPP_AIR_RECIEVED_DATA_IND: {
            spp_air_data_received_ind_t *ready_read = (spp_air_data_received_ind_t *)param;
            mux_bt_port_context_t *context = mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_SPP, ready_read->handle);
            if ((context != NULL) && (context->is_open)) {
                virtual_read_write_point_t *ring_point = &context->rw_point;
                uint8_t *read_data = (uint8_t *)port_mux_malloc(ring_point->rx_buff_len);
                if (read_data == NULL) {
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] read port index = %02x, handle = %02x malloc fail", 2, context->port_index, ready_read->handle);
                    break;
                }
                uint32_t ret_length = spp_air_read_data(ready_read->handle, read_data, ring_point->rx_buff_len);
                if (ret_length == 0) {
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] read port index = %02x, handle = %02x size is 0", 2, context->port_index, ready_read->handle);
                    port_mux_free(read_data);
                    break;
                }
                mux_bt_rx_packet_handle(context, read_data, ret_length);
                port_mux_free(read_data);
            }
        }
        break;
        default:
            break;
    }
}

#ifdef MTK_GATT_OVER_BREDR_ENABLE
static void mux_bt_port_gatt_over_bredr_event_callback(gatt_over_bredr_air_event_t event, void *callback_param)
{
    switch (event) {
        case GATT_OVER_BREDR_AIR_EVENT_CONNECT_IND: {
            gatt_over_bredr_air_connect_t *conn_ind = (gatt_over_bredr_air_connect_t *)callback_param;

            mux_bt_reset_connected_port_context_info(MUX_BT_PORT_TYPE_GATT_OVER_BREDR, conn_ind->conn_handle, conn_ind->bdaddr);

            if (NULL != mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_GATT_OVER_BREDR, conn_ind->conn_handle)) {
                break;
            }
            mux_bt_port_context_t *context = mux_bt_get_free_port_context(MUX_BT_PORT_TYPE_GATT_OVER_BREDR);
            if (context != NULL) {
                context->type = MUX_BT_PORT_TYPE_GATT_OVER_BREDR;
                context->connection_handle = conn_ind->conn_handle;
                context->mtu_size = conn_ind->max_packet_length;
                bt_utils_memcpy(&context->peer_address, &conn_ind->bdaddr, sizeof(bt_bd_addr_t));
                if (context->is_open) {
                    mux_bt_connect_ind_t ind = {0};
                    ind.handle = conn_ind->conn_handle;;
                    ind.max_packet_length = conn_ind->max_packet_length;
                    bt_utils_memcpy(&ind.address, &conn_ind->bdaddr, sizeof(bt_bd_addr_t));
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] gatt over bredr port connect handle = %02x", 1, conn_ind->conn_handle);
                    context->mux_callback(MUX_BT_INDEX_TO_MUX_PORT(context->port_index), MUX_EVENT_CONNECTION, &ind);
                }
            }
        }
        break;
        case GATT_OVER_BREDR_AIR_EVENT_DISCONNECT_IND: {
            gatt_over_bredr_air_disconnect_t *dis_ind = (gatt_over_bredr_air_disconnect_t *)callback_param;
            mux_bt_port_context_t *context = mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_GATT_OVER_BREDR, dis_ind->conn_handle);
            if (context != NULL) {
                mux_bt_reset_port_context_conn_info(context);
                if (context->is_open) {
                    mux_bt_disconnect_ind_t ind = {0};
                    ind.handle = dis_ind->conn_handle;
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] gatt over bredr port disconnect handle = %02x", 1, dis_ind->conn_handle);
                    context->mux_callback(MUX_BT_INDEX_TO_MUX_PORT(context->port_index), MUX_EVENT_DISCONNECTION, &ind);
                }
            }
        }
        break;
        case GATT_OVER_BREDR_AIR_EVENT_READY_TO_READ_IND: {
            gatt_over_bredr_air_ready_to_read_t *ready_read = (gatt_over_bredr_air_ready_to_read_t *)callback_param;
            mux_bt_port_context_t *context = mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_GATT_OVER_BREDR, ready_read->conn_handle);
            if ((context != NULL) && (context->is_open)) {
                virtual_read_write_point_t *ring_point = &context->rw_point;
                uint8_t *read_data = (uint8_t *)port_mux_malloc(ring_point->rx_buff_len);
                if (read_data == NULL) {
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] read port index = %02x, handle = %02x malloc fail", 2, context->port_index, ready_read->conn_handle);
                    break;
                }
                uint32_t ret_length = gatt_over_bredr_air_read_data(ready_read->conn_handle, read_data, ring_point->rx_buff_len);
                if (ret_length == 0) {
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] read port index = %02x, handle = %02x size is 0", 2, context->port_index, ready_read->conn_handle);
                    port_mux_free(read_data);
                    break;
                }
                mux_bt_rx_packet_handle(context, read_data, ret_length);
                port_mux_free(read_data);
            }
        }
        break;
        default:
            break;
    }
}
#endif

#ifdef MTK_AIRUPDATE_ENABLE
static void mux_bt_port_airupdate_event_callback(airupdate_event_t event_id, void *param)
{
    switch (event_id) {
        case AIRUPDATE_CONNECT_IND: {
            airupdate_connect_ind_t *conn_ind = (airupdate_connect_ind_t *)param;
            mux_bt_reset_connected_port_context_info(MUX_BT_PORT_TYPE_AIRUPDATE, conn_ind->handle, conn_ind->address);
            if (NULL != mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_AIRUPDATE, conn_ind->handle)) {
                break;
            }
            mux_bt_port_context_t *context = mux_bt_get_free_port_context(MUX_BT_PORT_TYPE_AIRUPDATE);
            if (context != NULL) {
                context->type = MUX_BT_PORT_TYPE_AIRUPDATE;
                context->connection_handle = conn_ind->handle;
                context->mtu_size = conn_ind->max_packet_length;
                bt_utils_memcpy(&context->peer_address, &conn_ind->bdaddr, sizeof(bt_bd_addr_t));
                if (context->is_open) {
                    mux_bt_connect_ind_t ind = {0};
                    ind.handle = conn_ind->handle;;
                    ind.max_packet_length = 0;
                    ind.max_packet_length = conn_ind->max_packet_length;
                    bt_utils_memcpy(&ind.address, &conn_ind->bdaddr, sizeof(bt_bd_addr_t));
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] airupdate port connect handle = %02x", 1, conn_ind->handle);
                    context->mux_callback(MUX_BT_INDEX_TO_MUX_PORT(context->port_index), MUX_EVENT_CONNECTION, &ind);
                }
            }
        }
        break;
        case AIRUPDATE_DISCONNECT_IND: {
            airupdate_disconnect_ind_t *dis_ind = (airupdate_disconnect_ind_t *)param;
            mux_bt_port_context_t *context = mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_AIRUPDATE, dis_ind->conn_handle);
            if (context != NULL) {
                mux_bt_reset_port_context_conn_info(context);
                if (context->is_open) {
                    mux_bt_disconnect_ind_t ind = {0};
                    ind.handle = dis_ind->conn_handle;
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] airupdate port disconnect handle = %02x", 1, dis_ind->handle);
                    context->mux_callback(MUX_BT_INDEX_TO_MUX_PORT(context->port_index), MUX_EVENT_DISCONNECTION, &ind);
                }
            }
        }
        break;
        case AIRUPDATE_RECIEVED_DATA_IND: {
            airupdate_ready_to_send_ind_t *ready_read = (airupdate_ready_to_send_ind_t *)param;
            mux_bt_port_context_t *context = mux_bt_find_port_context_by_handle(MUX_BT_PORT_TYPE_AIRUPDATE, ready_read->conn_handle);
            if ((context != NULL) && (context->is_open)) {
                virtual_read_write_point_t *ring_point = &context->rw_point;
                uint8_t *read_data = (uint8_t *)port_mux_malloc(ring_point->rx_buff_len);
                if (read_data == NULL) {
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] read port index = %02x, handle = %02x malloc fail", 2, context->port_index, ready_read->conn_handle);
                    break;
                }
                uint32_t ret_length = airupdate_read_data(ready_read->conn_handle, read_data, ring_point->rx_buff_len);
                if (ret_length == 0) {
                    LOG_MSGID_I(MUX_BT, "[MUX][BT] read port index = %02x, handle = %02x size is 0", 2, context->port_index, ready_read->conn_handle);
                    port_mux_free(read_data);
                    break;
                }
                mux_bt_rx_packet_handle(context, read_data, ret_length);
                port_mux_free(read_data);
            }
        }
        break;
        default:
            break;
    }
}
#endif

mux_status_t port_mux_bt_init(mux_port_t port)
{
    mux_status_t status;
    mux_port_setting_t setting;
    mux_protocol_t mux_bt_pro_callback = {
        .tx_protocol_callback = NULL,
        .rx_protocol_callback = NULL,
        .user_data = NULL
    };
    setting.tx_buffer_size = 512;
    setting.rx_buffer_size = 512;
    status = mux_init(port, &setting, &mux_bt_pro_callback);
    return status;
}

static mux_bt_port_group_context_t *mux_bt_find_group_context_by_index(uint8_t port_index)
{
    uint32_t i = 0;
    for (i = 0; i < MUX_BT_PORT_TYPE_MAX; i++) {
        if ((mux_port_group[i].index_start <= port_index) && (mux_port_group[i].index_end >= port_index)) {
            return &mux_port_group[i];
        }
    }
    return NULL;
}

static mux_status_t mux_bt_air_init(mux_bt_port_group_context_t *group_context)
{
    mux_status_t status = MUX_STATUS_OK;
    LOG_MSGID_I(MUX_BT, "[MUX][BT] air init group type = %02x", 1, group_context->type);
    switch (group_context->type) {
        case MUX_BT_PORT_TYPE_SPP: {
            if (0 != spp_air_init(mux_bt_port_spp_event_callback)) {
                status = MUX_STATUS_ERROR;
            }
        }
        break;
        case MUX_BT_PORT_TYPE_BLE: {
            if (0 != ble_air_init(mux_bt_port_le_event_callback)) {
                status = MUX_STATUS_ERROR;
            }
        }
        break;
#ifdef MTK_GATT_OVER_BREDR_ENABLE
        case MUX_BT_PORT_TYPE_GATT_OVER_BREDR: {
            if (0 != gatt_over_bredr_air_init(mux_bt_port_gatt_over_bredr_event_callback)) {
                status = MUX_STATUS_ERROR;
            }
        }
        break;
#endif
#ifdef MTK_AIRUPDATE_ENABLE
        case MUX_BT_PORT_TYPE_AIRUPDATE: {
            if (0 != airupdate_init(mux_bt_port_airupdate_event_callback)) {
                status = MUX_STATUS_ERROR;
            }
        }
        break;
#endif
        default:
            status = MUX_STATUS_ERROR;
            break;
    }
    if (status == MUX_STATUS_OK) {
        group_context->is_air_inited = true;
    }
    return status;
}

static mux_status_t mux_bt_air_deinit(mux_bt_port_group_context_t *group_context)
{
    mux_status_t status = MUX_STATUS_OK;
    LOG_MSGID_I(MUX_BT, "[MUX][BT] air deinit group type = %02x", 1, group_context->type);
    switch (group_context->type) {
        case MUX_BT_PORT_TYPE_SPP: {
            if (0 != spp_air_deinit(mux_bt_port_spp_event_callback)) {
                status = MUX_STATUS_ERROR;
            }
        }
        break;
        case MUX_BT_PORT_TYPE_BLE: {
            if (0 != ble_air_deinit(mux_bt_port_le_event_callback)) {
                status = MUX_STATUS_ERROR;
            }
        }
        break;
#ifdef MTK_GATT_OVER_BREDR_ENABLE
        case MUX_BT_PORT_TYPE_GATT_OVER_BREDR: {
            if (0 != gatt_over_bredr_air_deinit(mux_bt_port_gatt_over_bredr_event_callback)) {
                status = MUX_STATUS_ERROR;
            }
        }
        break;
#endif
#ifdef MTK_AIRUPDATE_ENABLE
        case MUX_BT_PORT_TYPE_AIRUPDATE: {
            if (0 != airupdate_deinit(mux_bt_port_airupdate_event_callback)) {
                status = MUX_STATUS_ERROR;
            }
        }
        break;
#endif
        default:
            status = MUX_STATUS_ERROR;
            break;
    }
    if (status == MUX_STATUS_OK) {
        group_context->is_air_inited = false;
    }
    return status;
}

static mux_status_t mux_bt_normal_init(uint8_t port_index, mux_port_config_t *p_setting, mux_irq_handler_t irq_handler)
{
    mux_bt_port_context_t *context = &mux_port_context[port_index];
    mux_status_t status = MUX_STATUS_OK;
    if (mux_bt_context.mux_bt_semaphore == NULL) {
        mux_bt_context.mux_bt_semaphore = xSemaphoreCreateMutex();
        configASSERT(mux_bt_context.mux_bt_semaphore != NULL);
    }
    context->mux_callback = irq_handler;
    context->is_open = true;
    mux_common_device_r_w_point_init(&context->rw_point, p_setting);
    context->is_point_inited = true;
    context->port_index = port_index;
    context->local_setting = p_setting;
    mux_driver_debug_for_check(&context->rw_point);
    mux_bt_port_group_context_t *group_context = mux_bt_find_group_context_by_index(port_index);
    if (group_context != NULL) {
        if (!group_context->is_air_inited) {
            status = mux_bt_air_init(group_context);
        } else if (context->connection_handle != MUX_BT_INVAILD_HANDLE){
            /* notify user for connected before port open. */
            mux_bt_connect_ind_t ind = {0};
            ind.handle = context->connection_handle;
            ind.max_packet_length = context->mtu_size;
            bt_utils_memcpy(&ind.address, &context->peer_address, sizeof(bt_bd_addr_t));
            LOG_MSGID_I(MUX_BT, "[MUX][BT] had inited air and connected, ", 0);
            context->mux_callback(MUX_BT_INDEX_TO_MUX_PORT(context->port_index), MUX_EVENT_CONNECTION, &ind);
        } else {
            LOG_MSGID_I(MUX_BT, "[MUX][BT] init group context = %02x had inited", 1, group_context);
        }
    } else {
        LOG_MSGID_I(MUX_BT, "[MUX][BT] init group context is NULL", 0);
        status = MUX_STATUS_ERROR;
    }
    LOG_MSGID_I(MUX_BT, "[MUX][BT] init port index = %02x status = %02x", 2, port_index, status);
    return status;
}

static bool mux_bt_group_link_is_exist(mux_bt_port_group_context_t *group_context)
{
    mux_bt_port_context_t *context = NULL;
    uint32_t i = 0;
    for (i = group_context->index_start; i < group_context->index_end; i++) {
        context = &mux_port_context[i];
        if (context->connection_handle != MUX_BT_INVAILD_HANDLE) {
            return true;
        }
    }
    return false;
}

mux_status_t mux_bt_deinit(uint8_t port_index)
{
    mux_status_t status = MUX_STATUS_OK;
    mux_bt_port_context_t *context = &mux_port_context[port_index];
    mux_driver_debug_for_check(&context->rw_point);
    mux_bt_port_group_context_t *group_context = mux_bt_find_group_context_by_index(port_index);
    if (group_context == NULL) {
        return MUX_STATUS_ERROR;
    }
    mux_bt_reset_port_context_conn_info(context);
    context->local_setting = NULL;
    if (!mux_bt_group_link_is_exist(group_context)) {
        /* group port have not link exist. */
        status = mux_bt_air_deinit(group_context);
    }

    LOG_MSGID_I(MUX_BT, "[MUX][BT] deinit port index = %02x status = %02x", 2, port_index, status);
    return status;
}

uint32_t mux_bt_air_send(uint8_t port_index, uint8_t *packet, uint32_t length)
{
    mux_bt_port_context_t *context = &mux_port_context[port_index];
    uint8_t *send_packet = NULL;
    uint32_t send_length = 0;
    uint32_t per_cpu_irq_mask = 0;
    uint32_t tx_complete_size = 0;
    uint32_t ret_send_length = 0;
    do {
        send_packet = (uint8_t *)(packet + tx_complete_size);
        send_length = length - tx_complete_size;
        switch (context->type) {
            case MUX_BT_PORT_TYPE_SPP: {
                ret_send_length = spp_air_write_data(context->connection_handle, send_packet, send_length);
            }
            break;
            case MUX_BT_PORT_TYPE_BLE: {
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
                ret_send_length = ble_air_srv_write_data(context->connection_handle, send_packet, send_length);
#else
                ret_send_length = ble_air_write_data(context->connection_handle, send_packet, send_length);
#endif
            }
            break;
#ifdef MTK_GATT_OVER_BREDR_ENABLE
            case MUX_BT_PORT_TYPE_GATT_OVER_BREDR: {
                ret_send_length = gatt_over_bredr_air_write_data(context->connection_handle, send_packet, send_length);
            }
            break;
#endif
#ifdef MTK_AIRUPDATE_ENABLE
            case MUX_BT_PORT_TYPE_AIRUPDATE: {
                ret_send_length = airupdate_write_data(context->connection_handle, send_packet, send_length);
            }
            break;
#endif
            default:
                break;
        }
        tx_complete_size += ret_send_length;
    } while ((ret_send_length != 0) && (tx_complete_size < length));
    port_mux_local_cpu_enter_critical(&per_cpu_irq_mask);
    if (!context->is_point_inited) {
        mux_bt_set_tx_hw_rptr_internal_use(port_index, length);
    }
    port_mux_local_cpu_exit_critical(per_cpu_irq_mask);
    return tx_complete_size;
}

mux_status_t mux_bt_phase2_send(uint8_t port_index)
{
    uint32_t per_cpu_irq_mask = 0;
    virtual_read_write_point_t *p = &mux_port_context[port_index].rw_point;
    uint32_t tx_complete_size = 0;
    uint32_t tx_prepare_size = 0;
    port_mux_local_cpu_enter_critical(&per_cpu_irq_mask);
    mux_driver_debug_for_check(p);
    port_mux_local_cpu_exit_critical(per_cpu_irq_mask);
    mux_port_context[port_index].is_point_inited = false;
    if (HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) {  /* xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED. */
        /* Task context */
        xSemaphoreTake(mux_bt_context.mux_bt_semaphore, portMAX_DELAY);
        mux_bt_context.tx_status = MUX_STATUS_OK;
        mux_bt_context.tx_complete_size = 0;
        p->tx_send_is_running = MUX_DEVICE_HW_RUNNING;
        LOG_MSGID_I(MUX_BT, "[MUX][BT] point:port_index = %d, r_point = %02x, w_point = %02x, s_point = %02x, e_point = %02x, tx_available_len = %02x", 6,
                    port_index, p->tx_buff_read_point, p->tx_buff_write_point, p->tx_buff_start, p->tx_buff_end, p->tx_buff_available_len);
        if (p->tx_buff_read_point <= p->tx_buff_write_point) {
            tx_prepare_size = p->tx_buff_write_point - p->tx_buff_read_point;
            uint8_t *data_point = (uint8_t *)(p->tx_buff_read_point);
            tx_complete_size = mux_bt_air_send(port_index, data_point, p->tx_buff_write_point - p->tx_buff_read_point);  /* user must update Read_point equle Write_point in this function. */
            LOG_MSGID_I(MUX_BT, "[MUX][BT] xSemaphoreGive OK tx complete size = %d,tx_prepare_size = %d", 2, tx_complete_size, tx_prepare_size);
        } else {
            uint8_t *first_data = (uint8_t *)(p->tx_buff_read_point);
            uint16_t first_data_length = p->tx_buff_end - p->tx_buff_read_point;
            uint8_t *second_data = (uint8_t *)(p->tx_buff_start);
            uint16_t second_data_length = p->tx_buff_write_point - p->tx_buff_start;
            uint8_t *full_buffer = (uint8_t *)pvPortMalloc(first_data_length + second_data_length);
            tx_prepare_size = first_data_length + second_data_length;
            if (full_buffer == NULL) {
                LOG_MSGID_I(MUX_BT, "[MUX][BT] alloc full buffer fail ", 0);
                p->tx_send_is_running = MUX_DEVICE_HW_IDLE;
                mux_bt_context.tx_status = MUX_STATUS_ERROR;
                xSemaphoreGive(mux_bt_context.mux_bt_semaphore);
                return MUX_STATUS_ERROR;
            }
            /* copy data from ring buffer. */
            bt_utils_memcpy(full_buffer, first_data, first_data_length);
            bt_utils_memcpy(full_buffer + first_data_length, second_data, second_data_length);
            tx_complete_size = mux_bt_air_send(port_index, full_buffer, first_data_length + second_data_length);
            vPortFree(full_buffer);
            LOG_MSGID_I(MUX_BT, "[MUX][BT] Joining together xSemaphoreGive OK tx complete size = %d,tx_prepare_size = %d\r\n", 2, tx_complete_size, tx_prepare_size);
        }
        p->tx_send_is_running = MUX_DEVICE_HW_IDLE;
        xSemaphoreGive(mux_bt_context.mux_bt_semaphore);
        if ((tx_complete_size != tx_prepare_size) || (tx_prepare_size == 0)) {
            mux_bt_context.tx_status = MUX_STATUS_ERROR;
        }
        mux_bt_context.tx_complete_size = tx_complete_size;
    } else {

    }
    return MUX_STATUS_OK;
}

void mux_bt_exception_init(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
}

void mux_bt_exception_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
}

bool mux_bt_buf_is_full(uint8_t port_index, bool is_rx)
{
    return mux_common_device_buf_is_full(&mux_port_context[port_index].rw_point, is_rx);
}

uint32_t mux_bt_get_hw_rptr(uint8_t port_index, bool is_rx)
{
    return mux_common_device_get_hw_rptr(&mux_port_context[port_index].rw_point, is_rx);
}

uint32_t mux_bt_get_hw_wptr(uint8_t port_index, bool is_rx)
{
    return mux_common_device_get_hw_wptr(&mux_port_context[port_index].rw_point, is_rx);
}

void mux_bt_set_rx_hw_rptr(uint8_t port_index, uint32_t move_bytes)
{
    mux_common_device_set_rx_hw_rptr(&mux_port_context[port_index].rw_point, move_bytes);
}

static void mux_bt_set_rx_hw_wptr_internal_use(uint8_t port_index, uint32_t move_bytes)
{
    mux_common_device_set_rx_hw_wptr_internal_use(&mux_port_context[port_index].rw_point, move_bytes);
}

static void mux_bt_set_tx_hw_rptr_internal_use(uint8_t port_index, uint32_t move_bytes)
{
    mux_common_device_set_tx_hw_rptr_internal_use(&mux_port_context[port_index].rw_point, move_bytes);
}

void mux_bt_set_tx_hw_wptr(uint8_t port_index, uint32_t move_bytes)
{
    virtual_read_write_point_t *p = &mux_port_context[port_index].rw_point;
    mux_common_device_set_tx_hw_wptr(p, move_bytes);
}

mux_status_t mux_bt_phase1_send(uint8_t port_index)
{
    PORT_MUX_UNUSED(port_index);
    return MUX_STATUS_OK;
}

mux_status_t mux_bt_control(uint8_t port_index, mux_ctrl_cmd_t command, mux_ctrl_para_t *para)
{
    //mux_get_trx_status_t *p_mux_trx_status = (mux_get_trx_status_t *)para;
    mux_bt_port_context_t *context = &mux_port_context[port_index];
    switch (command) {
        case MUX_CMD_GET_TX_SEND_STATUS: {
            mux_get_trx_status_t *p_mux_trx_status = (mux_get_trx_status_t *)para;
            p_mux_trx_status->tx_send_status = mux_bt_context.tx_status;
            p_mux_trx_status->transfer_completed_size = mux_bt_context.tx_complete_size;
            return MUX_STATUS_OK;
        }
        break;
        case MUX_CMD_GET_RX_RECEIVED_STATUS: {
            return MUX_STATUS_OK;
        }
        break;
        case MUX_CMD_CLEAN: {
            /* Clear mux bt point by index. */
            if (context->local_setting != NULL) {
                LOG_MSGID_I(MUX_BT, "[MUX][BT] init point with port_index = %02x", 1, port_index);
                mux_port_context[port_index].is_point_inited = true;
                mux_common_device_r_w_point_init(&context->rw_point, context->local_setting);
            }
        }
        break;
        case MUX_CMD_GET_CONNECTION_PARAM: {
            /* get remote connected device address*/
            if (MUX_BT_INVAILD_HANDLE == context->connection_handle || NULL == para) {
                return MUX_STATUS_ERROR;
            } else {
                bt_utils_memcpy(&para->mux_get_connection_param.remote_address, &context->peer_address, sizeof(bt_bd_addr_t));
                return MUX_STATUS_OK;
            }
        }
        break;
        default:
            break;
    }
    return MUX_STATUS_ERROR;
}

port_mux_device_ops_t g_port_mux_bt_ops = {
#ifdef MTK_CPU_NUMBER_0
    mux_bt_normal_init,
    mux_bt_deinit,
    mux_bt_exception_init,
    mux_bt_exception_send,
    mux_bt_buf_is_full,
#endif
    mux_bt_get_hw_rptr,
    mux_bt_set_rx_hw_rptr,
    mux_bt_get_hw_wptr,
    mux_bt_set_tx_hw_wptr,
    mux_bt_phase1_send,
    mux_bt_phase2_send,
    mux_bt_control,
    NULL,
    NULL,
};
#endif
