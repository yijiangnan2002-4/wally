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
/* Airoha restricted information */

#include "mux.h"
#include "bt_spp.h"
#include "race_bt.h"
#include "race_noti.h"
#include "race_event.h"
#include "race_event_internal.h"
#include "race_timer.h"
#include "race_cmd.h"
#include "mux_bt.h"
#include "hal.h"

#include "bt_callback_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_type.h"
#include "bt_aws_mce.h"
#include "bt_connection_manager.h"

#include "race_lpcomm_util.h"
#include "race_lpcomm_retry.h"

#include "race_usb_relay_dongle.h"

#ifdef RACE_ROLE_HANDOVER_SERVICE_ENABLE
#include "bt_role_handover.h"
#endif

#ifdef RACE_FIND_ME_ENABLE
#include "race_cmd_find_me.h"
#endif

#include "race_port_bt.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define RACE_MUX_SPP_TX_BUFFER_SIZE                     (1024 + 128)
#define RACE_MUX_SPP_RX_BUFFER_SIZE                     ((891- 530) + 891) //891=895(SPP MTU)-4(BT header); 530 = tool max fota packet
#define RACE_MUX_BLE_TX_BUFFER_SIZE                     (1024)
#define RACE_MUX_BLE_RX_BUFFER_SIZE                     (1024+256)

#define RACE_MUX_AIRUPDATE_TX_BUFFER_SIZE               (1024)
#define RACE_MUX_AIRUPDATE_RX_BUFFER_SIZE               (1024)

#define RACE_MUX_IAP2_TX_BUFFER_SIZE                    (1024)
#define RACE_MUX_IAP2_RX_BUFFER_SIZE                    (1024 * 2)

#define RACE_MUX_GATT_OVER_BREDR_TX_BUFFER_SIZE         (1024)
#define RACE_MUX_GATT_OVER_BREDR_RX_BUFFER_SIZE         (1024 * 3)

#define RACE_SPP_TX_RETRY_TIME                          (100000) /* unit: us */

/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct {
    mux_port_t dev;
    bt_bd_addr_t addr;
} race_bt_conn_info_t;

typedef struct {
    race_port_t port;
    mux_event_t event;
    race_event_type_enum noti_msg;
} race_bt_event_map_t;

typedef struct {
    race_port_t port;
    race_port_type_t port_type;
    uint32_t tx_buf_size;
    uint32_t rx_buf_size;
} race_bt_port_buf_size_t;

/**************************************************************************************************
* Static Variable
**************************************************************************************************/
#ifdef MTK_RACE_EVENT_ID_ENABLE
int32_t g_race_event_register_id = RACE_EVENT_INVALID_REGISTER_ID;
#endif

static race_bt_conn_info_t g_race_bt_connection[] = {
    {MUX_BT_SPP,              {0}},
    {MUX_BT_BLE,              {0}},
    {MUX_BT_BLE_1,            {0}},
    {MUX_BT_BLE_2,            {0}},
#ifdef MTK_GATT_OVER_BREDR_ENABLE
    {MUX_BT_GATT_OVER_BREDR,  {0}},
#endif
    {MUX_BT_AIRUPATE,         {0}},
#ifdef MTK_IAP2_VIA_MUX_ENABLE
    {RACE_MUX_IAP2_PORT,      {0}},
#endif
};

#define RACE_BT_CONNECTION_NUM    (sizeof(g_race_bt_connection) / sizeof(race_bt_conn_info_t))

static const race_bt_event_map_t g_race_bt_event_map[] = {
#if defined(MTK_MUX_BT_ENABLE)
    {MUX_BT_SPP,                MUX_EVENT_CONNECTION,       RACE_EVENT_TYPE_CONN_SPP_CONNECT},
    {MUX_BT_SPP,                MUX_EVENT_DISCONNECTION,    RACE_EVENT_TYPE_CONN_SPP_DISCONNECT},
    {MUX_BT_BLE,                MUX_EVENT_CONNECTION,       RACE_EVENT_TYPE_CONN_BLE_CONNECT},
    {MUX_BT_BLE,                MUX_EVENT_DISCONNECTION,    RACE_EVENT_TYPE_CONN_BLE_DISCONNECT},
    {MUX_BT_BLE_1,              MUX_EVENT_CONNECTION,       RACE_EVENT_TYPE_CONN_BLE_1_CONNECT},
    {MUX_BT_BLE_1,              MUX_EVENT_DISCONNECTION,    RACE_EVENT_TYPE_CONN_BLE_1_DISCONNECT},
    {MUX_BT_BLE_2,              MUX_EVENT_CONNECTION,       RACE_EVENT_TYPE_CONN_BLE_2_CONNECT},
    {MUX_BT_BLE_2,              MUX_EVENT_DISCONNECTION,    RACE_EVENT_TYPE_CONN_BLE_2_DISCONNECT},
#if defined(MTK_GATT_OVER_BREDR_ENABLE)
    {MUX_BT_GATT_OVER_BREDR,    MUX_EVENT_CONNECTION,       RACE_EVENT_TYPE_CONN_GATT_OVER_BREDR_CONNECT},
    {MUX_BT_GATT_OVER_BREDR,    MUX_EVENT_DISCONNECTION,    RACE_EVENT_TYPE_CONN_GATT_OVER_BREDR_DISCONNECT},
#endif
    {MUX_BT_AIRUPATE,           MUX_EVENT_CONNECTION,       RACE_EVENT_TYPE_CONN_AIRUPDATE_CONNECT},
    {MUX_BT_AIRUPATE,           MUX_EVENT_DISCONNECTION,    RACE_EVENT_TYPE_CONN_AIRUPDATE_DISCONNECT},
#endif
#if defined(MTK_IAP2_VIA_MUX_ENABLE) && defined(MTK_IAP2_PROFILE_ENABLE)
    {RACE_MUX_IAP2_PORT,        MUX_EVENT_CONNECTION,       RACE_EVENT_TYPE_CONN_IAP2_CONNECT},
    {RACE_MUX_IAP2_PORT,        MUX_EVENT_DISCONNECTION,    RACE_EVENT_TYPE_CONN_IAP2_DISCONNECT},
#endif
#ifdef AIR_MUX_BT_HID_ENABLE
    {MUX_HID_CONTROL,           MUX_EVENT_CONNECTION,       RACE_EVENT_TYPE_CONN_HID_CONNECT},
    {MUX_HID_CONTROL,           MUX_EVENT_DISCONNECTION,    RACE_EVENT_TYPE_CONN_HID_DISCONNECT},
    {MUX_HID_INTERUPT,          MUX_EVENT_CONNECTION,       RACE_EVENT_TYPE_CONN_HID_CONNECT},
    {MUX_HID_INTERUPT,          MUX_EVENT_DISCONNECTION,    RACE_EVENT_TYPE_CONN_HID_DISCONNECT},
#endif
};

static const race_bt_port_buf_size_t g_race_bt_port_buf_size[] = {
#if defined(MTK_MUX_BT_ENABLE)
    {MUX_BT_SPP, RACE_PORT_TYPE_BT_BLE, RACE_MUX_SPP_TX_BUFFER_SIZE, RACE_MUX_SPP_RX_BUFFER_SIZE},
    {MUX_BT_BLE, RACE_PORT_TYPE_BT_BLE, RACE_MUX_BLE_TX_BUFFER_SIZE, RACE_MUX_BLE_RX_BUFFER_SIZE},
    {MUX_BT_BLE_1, RACE_PORT_TYPE_BT_BLE, RACE_MUX_BLE_TX_BUFFER_SIZE, RACE_MUX_BLE_RX_BUFFER_SIZE},
    {MUX_BT_BLE_2, RACE_PORT_TYPE_BT_BLE, RACE_MUX_BLE_TX_BUFFER_SIZE, RACE_MUX_BLE_RX_BUFFER_SIZE},
#if defined(MTK_GATT_OVER_BREDR_ENABLE)
    {MUX_BT_GATT_OVER_BREDR, RACE_PORT_TYPE_BT_BLE, RACE_MUX_GATT_OVER_BREDR_TX_BUFFER_SIZE, RACE_MUX_GATT_OVER_BREDR_RX_BUFFER_SIZE},
#endif
    {MUX_BT_AIRUPATE, RACE_PORT_TYPE_BT_BLE, RACE_MUX_AIRUPDATE_TX_BUFFER_SIZE, RACE_MUX_AIRUPDATE_RX_BUFFER_SIZE},
#endif

#if defined(MTK_IAP2_VIA_MUX_ENABLE) && defined(MTK_IAP2_PROFILE_ENABLE)
    {RACE_MUX_IAP2_PORT, RACE_PORT_TYPE_IAP2, RACE_MUX_IAP2_TX_BUFFER_SIZE, RACE_MUX_IAP2_RX_BUFFER_SIZE},
#endif

#ifdef AIR_MUX_BT_HID_ENABLE
    {MUX_HID_CONTROL, RACE_PORT_TYPE_BT_HID, RACE_MUX_SPP_TX_BUFFER_SIZE, RACE_MUX_SPP_RX_BUFFER_SIZE},
    {MUX_HID_INTERUPT, RACE_PORT_TYPE_BT_HID, RACE_MUX_SPP_TX_BUFFER_SIZE, RACE_MUX_SPP_RX_BUFFER_SIZE},
#endif
};

/**************************************************************************************************
* Prototype
**************************************************************************************************/
#if defined(RACE_ROLE_HANDOVER_SERVICE_ENABLE) && defined(RACE_AWS_ENABLE)
static bt_status_t race_rho_srv_allowed_callback(const bt_bd_addr_t *addr);
static uint8_t race_rho_get_data_len_callback(const bt_bd_addr_t *addr);
static bt_status_t race_rho_get_data_callback(const bt_bd_addr_t *addr, void *data);
static bt_status_t race_rho_update_data_callback(bt_role_handover_update_info_t *info);
static void race_rho_srv_status_callback(const bt_bd_addr_t *addr,
    bt_aws_mce_role_t role,
    bt_role_handover_event_t event,
    bt_status_t status);
#endif

static race_bt_conn_info_t *race_find_bt_conn_info_by_port(mux_port_t dev);
static void race_set_bt_connection_addr(mux_port_t dev, bt_bd_addr_t *addr);
static uint32_t race_iap2_tx(mux_handle_t mux_handle, uint8_t *buf, uint32_t buf_len);
static uint32_t race_spp_tx(mux_handle_t mux_handle, uint8_t *buf, uint32_t buf_len);
static race_event_type_enum race_bt_get_notify_event(race_port_t port, mux_event_t event);
static void race_mux_event_handler(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data);
static bool race_port_bt_prepare_open(race_port_t port, race_port_type_t port_type);

#ifdef MTK_RACE_EVENT_ID_ENABLE
static RACE_ERRCODE race_event_bt_cb(int32_t register_id, race_event_type_enum event_type, void *param, void *user_data);
#else
static RACE_ERRCODE race_event_bt_cb(race_event_type_enum event_type, void *param, void *user_data);
#endif

//static void race_port_bt_check_connected_before_open(race_port_t port);

/**************************************************************************************************
* Static Functions for RHO
**************************************************************************************************/

#if defined(RACE_ROLE_HANDOVER_SERVICE_ENABLE) && defined(RACE_AWS_ENABLE)
static bt_status_t race_rho_srv_allowed_callback(const bt_bd_addr_t *addr)
{
    /* Always PENDING for SPP/BLE/AIRUPDATE needs be disconnected. */
    return BT_STATUS_PENDING;
}

static uint8_t race_rho_get_data_len_callback(const bt_bd_addr_t *addr)
{
    uint8_t len = (uint8_t)sizeof(g_race_bt_connection);
#ifdef AIR_MULTI_POINT_ENABLE
    if (addr != NULL) {
        len = 0;
    }
#endif

    RACE_LOG_MSGID_I("RHO, data len %d", 1, len);
    return len;
}

static bt_status_t race_rho_get_data_callback(const bt_bd_addr_t *addr, void *data)
{
#ifdef AIR_MULTI_POINT_ENABLE
    if (addr != NULL) {
        RACE_LOG_MSGID_E("RHO, addr not null!", 0);
        return BT_STATUS_FAIL;
    }
#endif

    memcpy(data, &g_race_bt_connection, sizeof(g_race_bt_connection));
    return BT_STATUS_SUCCESS;
}

static bt_status_t race_rho_update_data_callback(bt_role_handover_update_info_t *info)
{
    if (info == NULL || info->data == NULL) {
        RACE_LOG_MSGID_E("RHO, update, data null", 0);
        return BT_STATUS_FAIL;
    }

    RACE_LOG_MSGID_I("RHO, update data, len %d", 1, info->length);
    if (info->length != sizeof(g_race_bt_connection)) {
        RACE_LOG_MSGID_E("RHO, update, length wrong", 0);
        return BT_STATUS_FAIL;
    }
    memcpy(&g_race_bt_connection, info->data, info->length);

    bt_bd_addr_t *addr = &(g_race_bt_connection[0].addr);
    RACE_LOG_MSGID_I("RHO, SPP addr %x:%x:%x:%x:%x:%x", 6,
                     ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5]);
    addr = &(g_race_bt_connection[1].addr);
    RACE_LOG_MSGID_I("RHO, BLE addr %x:%x:%x:%x:%x:%x", 6,
                     ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5]);

    return BT_STATUS_SUCCESS;
}

static void race_rho_srv_status_callback(
    const bt_bd_addr_t *addr,
    bt_aws_mce_role_t role,
    bt_role_handover_event_t event,
    bt_status_t status)
{
    if (BT_ROLE_HANDOVER_START_IND == event) {
        race_send_event_notify_msg(RACE_EVENT_TYPE_BT_RHO_START, NULL);
    } else if (BT_ROLE_HANDOVER_PREPARE_REQ_IND == event) {
        /* All allow RHO */
        race_send_event_notify_msg(RACE_EVENT_TYPE_BT_RHO_PREPARE, NULL);
    } else if (BT_ROLE_HANDOVER_COMPLETE_IND == event) {
        RACE_LOG_MSGID_I("RHO result:%d", 1, BT_STATUS_SUCCESS == status);
        race_event_send_bt_rho_result_event(BT_STATUS_SUCCESS == status);

        if (BT_STATUS_SUCCESS == status && role == BT_AWS_MCE_ROLE_AGENT) {
            RACE_LOG_MSGID_I("RHO complete, clear g_race_bt_connection", 0);
            memset(&(g_race_bt_connection[0].addr), 0, sizeof(bt_bd_addr_t));
            memset(&(g_race_bt_connection[1].addr), 0, sizeof(bt_bd_addr_t));
        }
    }
}
#endif

/**************************************************************************************************
* Static Functions
**************************************************************************************************/

static race_bt_conn_info_t *race_find_bt_conn_info_by_port(mux_port_t dev)
{
    uint8_t i = 0;

    for (i = 0; i < RACE_BT_CONNECTION_NUM; i ++) {
        if (g_race_bt_connection[i].dev == dev) {
            return &g_race_bt_connection[i];
        }
    }

    return NULL;
}

static void race_set_bt_connection_addr(mux_port_t dev, bt_bd_addr_t *addr)
{
    race_bt_conn_info_t *info = race_find_bt_conn_info_by_port(dev);

    if (info == NULL) {
        RACE_LOG_MSGID_E("race_set_bt_connection_addr, dev %d not support", 1, dev);
        return;
    }

    if (addr) {
        RACE_LOG_MSGID_I("race_set_bt_connection_addr, dev %d, addr %x:%x:%x:%x:%x:%x", 7, dev,
                         ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5]);
        memcpy(&(info->addr), addr, sizeof(bt_bd_addr_t));
    } else {
        RACE_LOG_MSGID_I("race_set_bt_connection_addr, dev %d, set 0", 1, dev);
        memset(&(info->addr), 0, sizeof(bt_bd_addr_t));
    }
}

static uint32_t race_iap2_tx(mux_handle_t mux_handle, uint8_t *buf, uint32_t buf_len)
{
    uint32_t data_len = 0;
    uint16_t iap2_session_id;
    mux_ctrl_para_t para;
    mux_control(RACE_GET_PORT_BY_MUX_HANDLE(mux_handle), MUX_CMD_GET_CONNECTION_PARAM, &para);
    RACE_LOG_MSGID_I("race_serial_port_send_data, current_sesion_num[%d], session_id0[%X], session_id1[%X], session_id2[%X]",
                     4, para.mux_get_connection_param.iap2_session_num, para.mux_get_connection_param.iap2_session_id[0],
                     para.mux_get_connection_param.iap2_session_id[1], para.mux_get_connection_param.iap2_session_id[2]);
    iap2_session_id = para.mux_get_connection_param.iap2_session_id[0];
    if (iap2_session_id != 0) {
        uint8_t *p_malloc_buff_addr;
        p_malloc_buff_addr = pvPortMalloc(buf_len + 2);
        if (p_malloc_buff_addr == NULL) {
            assert(0);
        }
        memcpy((void *)p_malloc_buff_addr, (void *)(&iap2_session_id), 2);
        memcpy((void *)p_malloc_buff_addr + 2, (void *)(buf), buf_len);

        mux_buffer_t mux_buffer_iap2[] = {
            {p_malloc_buff_addr, buf_len + 2}
        };
        mux_tx(mux_handle, (mux_buffer_t *)&mux_buffer_iap2, 1, &data_len);
        data_len -= 2;

        vPortFree(p_malloc_buff_addr);
        return data_len;
    }
    return 0;
}

static uint32_t race_spp_tx(mux_handle_t mux_handle, uint8_t *buf, uint32_t buf_len)
{
    mux_buffer_t mux_buffer = {buf, buf_len};
    mux_get_trx_status_t para;
    mux_status_t mux_status;
    uint32_t data_len = 0;
    uint32_t ts_start = 0;
    uint32_t ts_curr = 0;
    uint32_t retry_cnt = 0;
    enum {
        RACE_SPP_TX_SUCCESS = 0,
        RACE_SPP_TX_MUX_FAIL = 1,
        RACE_SPP_TX_GET_STATUS_FAIL = 2,
        RACE_SPP_TX_NOT_START = 3,
        RACE_SPP_TX_NOT_CONNECT = 0x7f,
        RACE_SPP_TX_TIMEOUT = 0xff,
    } tx_state;
    tx_state = RACE_SPP_TX_NOT_START;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &ts_start);
    while (1) {
        mux_status = mux_tx(mux_handle, &mux_buffer, 1, &data_len);

        if (MUX_STATUS_OK == mux_status) {
            mux_status = mux_control(RACE_GET_PORT_BY_MUX_HANDLE(mux_handle), MUX_CMD_GET_TX_SEND_STATUS, (mux_ctrl_para_t *)&para);
            if (MUX_STATUS_OK == mux_status) {
                if (MUX_STATUS_OK == para.tx_send_status) {
                    tx_state = RACE_SPP_TX_SUCCESS;
                    data_len = buf_len;
                    break;
                } /* else, need retry */
            } else {
                data_len = 0;
                tx_state = RACE_SPP_TX_MUX_FAIL;
                break;
            }
        } else {
            tx_state = RACE_SPP_TX_GET_STATUS_FAIL;
            break;
        }

        if (false == race_bt_is_connected(RACE_GET_PORT_BY_MUX_HANDLE(mux_handle))) {
            tx_state = RACE_SPP_TX_NOT_CONNECT;
            break;
        }

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &ts_curr);
        if (ts_curr - ts_start >= RACE_SPP_TX_RETRY_TIME) {
            tx_state = RACE_SPP_TX_TIMEOUT;
            break;
        }

        retry_cnt++;
    }
    RACE_LOG_MSGID_I("race_serial_port_send_data spp, state[%d], retry[%d]", 2, tx_state, retry_cnt);
    return data_len;
}

static race_event_type_enum race_bt_get_notify_event(race_port_t port, mux_event_t event)
{
    uint16_t i;
    uint16_t map_size = sizeof(g_race_bt_event_map) / sizeof(g_race_bt_event_map[0]);
    for (i = 0; i < map_size; i++) {
        if (g_race_bt_event_map[i].port == port && g_race_bt_event_map[i].event == event) {
            return g_race_bt_event_map[i].noti_msg;
        }
    }
    return RACE_EVENT_TYPE_NONE;
}

static void race_mux_event_handler(mux_handle_t handle, mux_event_t event, uint32_t data_info, void *user_data)
{
    race_event_type_enum noti_event = race_bt_get_notify_event(RACE_GET_PORT_BY_MUX_HANDLE(handle), event);
    if (RACE_EVENT_TYPE_NONE != noti_event) {
        race_send_event_notify_msg(noti_event, NULL);
    }
    switch(event) {
        case MUX_EVENT_CONNECTION: {
            if (0 != data_info) {
                mux_bt_connect_ind_t *ind = (mux_bt_connect_ind_t *)data_info;
                race_set_bt_connection_addr(RACE_GET_PORT_BY_MUX_HANDLE(handle), &(ind->address));
#ifdef RACE_DYNAMIC_MAPPING_DEVICE_ID
                race_dongle_relay_conn_callback(RACE_GET_PORT_BY_MUX_HANDLE(handle), &(ind->address));
#endif
            }
            break;
        }

        case MUX_EVENT_DISCONNECTION: {
            race_set_bt_connection_addr(RACE_GET_PORT_BY_MUX_HANDLE(handle), NULL);
            mux_control(RACE_GET_PORT_BY_MUX_HANDLE(handle), MUX_CMD_CLEAN, NULL);
#ifdef RACE_DYNAMIC_MAPPING_DEVICE_ID
            race_dongle_relay_disconn_callback(RACE_GET_PORT_BY_MUX_HANDLE(handle));
#endif
            break;
        }

        default:
            break;
    }
}

static bool race_port_bt_prepare_open(race_port_t port, race_port_type_t port_type)
{
#if defined(MTK_MUX_BT_ENABLE)
    if (MUX_BT_BEGIN <= port && MUX_BT_END >= port) {
        if (RACE_PORT_TYPE_BT_BLE == port_type) {
            return true;
        }
    }
#endif

#if defined(MTK_MUX_AWS_MCE_ENABLE)
    if (MUX_AWS_MCE_BEGIN <= port && MUX_AWS_MCE_END >= port) {
        if (RACE_PORT_TYPE_AWS_MCE == port_type) {
            return true;
        }
    }
#endif

#if defined(MTK_IAP2_VIA_MUX_ENABLE)
    if (MUX_IAP2_BEGIN <= port && MUX_IAP2_END >= port) {
        if (RACE_PORT_TYPE_IAP2 == port_type) {
            return true;
        }
    }
#endif

#if defined(AIR_LOW_LATENCY_MUX_ENABLE)
    if (MUX_LL_UART_BEGIN <= port && MUX_LL_UART_END >= port) {
        if (RACE_PORT_TYPE_LL_UART == port_type) {
            return true;
        }
    }
#endif

#if defined(AIR_MUX_BT_HID_ENABLE)
    if (MUX_HID_BEGIN <= port && MUX_HID_END >= port) {
        if (RACE_PORT_TYPE_BT_HID == port_type) {
            return true;
        }
    }
#endif

    return false;
}


#ifdef MTK_RACE_EVENT_ID_ENABLE
static RACE_ERRCODE race_event_bt_cb(int32_t register_id, race_event_type_enum event_type, void *param, void *user_data)
#else
static RACE_ERRCODE race_event_bt_cb(race_event_type_enum event_type, void *param, void *user_data)
#endif
{
#ifdef MTK_RACE_EVENT_ID_ENABLE
    RACE_LOG_MSGID_I("race_event_cb, register_id[%d] event_type[%d] param[0x%X] user_data[0x%X]", 4, register_id, event_type, param, user_data);

    if (g_race_event_register_id != register_id) {
        RACE_LOG_MSGID_E("race_event_cb, register_id does not match! g_register_id[%d]", 1, g_race_event_register_id);
        return RACE_ERRCODE_PARAMETER_ERROR;
    }
#else
    RACE_LOG_MSGID_I("race_event_cb, event_type[%d] param[0x%X] user_data[0x%X]", 3, event_type, param, user_data);
#endif

    switch (event_type) {
        case RACE_EVENT_TYPE_CONN_BLE_DISCONNECT:
        case RACE_EVENT_TYPE_CONN_BLE_1_DISCONNECT:
        case RACE_EVENT_TYPE_CONN_BLE_2_DISCONNECT:
        case RACE_EVENT_TYPE_CONN_SPP_DISCONNECT:
#ifdef MTK_AIRUPDATE_ENABLE
        case RACE_EVENT_TYPE_CONN_AIRUPDATE_DISCONNECT:
#endif
#ifdef MTK_IAP2_PROFILE_ENABLE
        case RACE_EVENT_TYPE_CONN_IAP2_DISCONNECT:
#endif
#ifdef MTK_GATT_OVER_BREDR_ENABLE
        case RACE_EVENT_TYPE_CONN_GATT_OVER_BREDR_DISCONNECT:
#endif
        case RACE_EVENT_TYPE_CONN_USB_DISCONNECT: {
            break;
        }

        case RACE_EVENT_TYPE_CONN_BLE_CONNECT:
        case RACE_EVENT_TYPE_CONN_SPP_CONNECT:
#ifdef MTK_AIRUPDATE_ENABLE
        case RACE_EVENT_TYPE_CONN_AIRUPDATE_CONNECT:
#endif
#ifdef MTK_IAP2_PROFILE_ENABLE
        case RACE_EVENT_TYPE_CONN_IAP2_CONNECT:
#endif
#ifdef MTK_GATT_OVER_BREDR_ENABLE
        case RACE_EVENT_TYPE_CONN_GATT_OVER_BREDR_CONNECT:
#endif
        {
#ifdef RACE_FIND_ME_ENABLE
#ifdef RACE_ROLE_HANDOVER_SERVICE_ENABLE
            race_cmd_set_find_me_trans_method(event_type);
#endif
#endif
            break;
        }

#ifdef RACE_AWS_ENABLE
#ifdef RACE_ROLE_HANDOVER_SERVICE_ENABLE
        case RACE_EVENT_TYPE_BT_RHO_START: {
            /* Warning: When this event is received by Partner before RHO, RHO may have already been done.
                        * This is because BT task's priority is much higher than race task's. Need add mutex protect for
                        * race timer and stop the timer in race_rho_srv_status_callback(). However, there's no problem
                        * currently because the timer's timeout value is long enough.
                        */
            /* Start RHO timer when processing Role_Switch cmd. Stop RHO timer on receiving RHO_SRV's START event,
                        * because if RHO START event is received, RHO END will be received for sure.
                        */
            race_timer_smart_stop(race_lpcomm_get_rho_timer_id(), NULL);
            race_lpcomm_set_rho_timer_id(RACE_TIMER_INVALID_TIMER_ID);
            break;
        }

        /* Only Agent will receive RHO_PREPARE Event. */
        case RACE_EVENT_TYPE_BT_RHO_PREPARE: {
            /* No need to receive the fake rsp because RHO may have already been done when receiving the fake rsp. */
#ifdef RACE_LPCOMM_RETRY_ENABLE
            race_lpcomm_retry_cancel(RACE_APP_ID_ALL, FALSE);
#endif
#ifndef RACE_RHO_WITHOUT_SMARTPHONE_DISCONNECT_ENABLE
            /* When the serial port is closed, DISCONNECT event triggered by BT_AIR module will not
                        * be received. Therefore, send DISCONNECT event here.
                        */
            race_port_bt_close_all();
#endif
            bt_role_handover_reply_prepare_request(BT_ROLE_HANDOVER_MODULE_RACE_CMD);
            break;
        }
#endif

        case RACE_EVENT_TYPE_BT_RHO_RESULT: {
            race_lpcomm_set_role_switch_enable(FALSE);
#ifndef RACE_RHO_WITHOUT_SMARTPHONE_DISCONNECT_ENABLE
            /* Only when RACE_AWS_ENABLE is defined, will this event be received. */
#ifdef RACE_ROLE_HANDOVER_SERVICE_ENABLE
            RACE_LOG_MSGID_I("open ports", 0);
            race_port_bt_open_all();
#endif
#endif
            break;
        }
#endif

        default:
            break;
    }

    return RACE_ERRCODE_SUCCESS;
}

static RACE_ERRCODE race_port_bt_open_internal(race_port_t port, race_port_type_t port_type, uint32_t tx_buf_size, uint32_t rx_buf_size)
{
    RACE_ERRCODE res;
    mux_port_setting_t setting = {
        .tx_buffer_size = tx_buf_size,
        .rx_buffer_size = rx_buf_size,
        };
    race_port_init_t port_config;
    race_user_config_t user_config;

    if (race_port_bt_prepare_open(port, port_type) == false) {
        return RACE_ERRCODE_PARAMETER_ERROR;
    }
    memset(&port_config, 0, sizeof(race_port_init_t));
    port_config.port = port;
    port_config.port_type = port_type;
    port_config.port_settings = &setting;
    if (MUX_BT_SPP == port) {
        port_config.tx_function = race_spp_tx;
    } else if (RACE_PORT_TYPE_IAP2 == port_type) {
        port_config.tx_function = race_iap2_tx;
    }

    res = race_init_port(&port_config);
    if (RACE_ERRCODE_SUCCESS != res) {
        return res;
    }
    memset(&user_config, 0, sizeof(race_user_config_t));
    user_config.port = port;
    user_config.port_type = port_type;
    user_config.user_name = NULL;
    user_config.mux_event_post_handler = race_mux_event_handler;
    res = race_open_port(&user_config);
    if (RACE_ERRCODE_SUCCESS != res) {
        return res;
    }

#if defined(MTK_IAP2_VIA_MUX_ENABLE) && defined(MTK_IAP2_PROFILE_ENABLE)
#ifdef AIR_BTA_IC_PREMIUM_G2
    if (RACE_PORT_TYPE_IAP2 == port_type) {
        mux_set_config_param_t config_param = {
            .is_rx_need_session_id = 0,
            .reserved = 0
        };
        mux_control(port, MUX_CMD_SET_RX_PARAM, (mux_ctrl_para_t *)&config_param);
    }
#endif
#endif

    //race_port_bt_check_connected_before_open(port);

    return RACE_ERRCODE_SUCCESS;
}
/* BTA-44623: For such corner case: BT/BLE link has connected before open this port. 
static void race_port_bt_check_connected_before_open(race_port_t port)
{
    mux_status_t mux_res;
    mux_ctrl_para_t bt_param;
    bt_bd_addr_t bdaddr;
    mux_res = mux_control(port, MUX_CMD_GET_CONNECTION_PARAM, &bt_param);
    if (MUX_STATUS_OK == mux_res) {
        race_event_type_enum noti_event = race_bt_get_notify_event(port, MUX_EVENT_CONNECTION);
        memcpy(&bdaddr[0], &bt_param.mux_get_connection_param.remote_address[0], sizeof(bt_bd_addr_t));
        race_set_bt_connection_addr(port, &bdaddr);
#ifdef RACE_DYNAMIC_MAPPING_DEVICE_ID
        race_dongle_relay_conn_callback(port, &bdaddr);
#endif
        if (RACE_EVENT_TYPE_NONE != noti_event) {
            race_send_event_notify_msg(noti_event, NULL);
        }
        RACE_LOG_MSGID_I("port[%d] has connected before mux_open", 1, port);
    }
}
*/
/**************************************************************************************************
* Public Functions
**************************************************************************************************/

RACE_ERRCODE race_port_bt_open(race_port_t port)
{
    uint16_t buf_size_cnt = sizeof (g_race_bt_port_buf_size) / sizeof (g_race_bt_port_buf_size[0]);
    uint16_t i;
    for (i = 0; i < buf_size_cnt; i++) {
        if (port == g_race_bt_port_buf_size[i].port) {
            return race_port_bt_open_internal(port,
                g_race_bt_port_buf_size[i].port_type,
                g_race_bt_port_buf_size[i].tx_buf_size,
                g_race_bt_port_buf_size[i].rx_buf_size);
        }
    }
    return RACE_ERRCODE_FAIL;
}

bool race_port_bt_close(race_port_t port)
{
    race_event_type_enum noti_event = race_bt_get_notify_event(port, MUX_EVENT_DISCONNECTION);
    race_close_port_for_all_user(port);
    if (RACE_EVENT_TYPE_NONE != noti_event) {
        race_send_event_notify_msg(noti_event, NULL);
    }
    return true;
}

bool race_port_bt_open_all(void)
{
#if 1
    RACE_LOG_MSGID_I("race_port_bt_open_all", 0);
#if defined(MTK_MUX_BT_ENABLE)
    race_port_bt_open(MUX_BT_SPP);
    race_port_bt_open(MUX_BT_BLE);
    race_port_bt_open(MUX_BT_BLE_1);
    race_port_bt_open(MUX_BT_BLE_2);
#if defined(MTK_GATT_OVER_BREDR_ENABLE)
    race_port_bt_open(MUX_BT_GATT_OVER_BREDR);
#endif
#ifdef MTK_AIRUPDATE_ENABLE
    race_port_bt_open(MUX_BT_AIRUPATE);
#endif
#endif
#if defined(MTK_IAP2_VIA_MUX_ENABLE) && defined(MTK_IAP2_PROFILE_ENABLE)
    race_port_bt_open(RACE_MUX_IAP2_PORT);
#endif
#ifdef AIR_MUX_BT_HID_ENABLE
    race_port_bt_open(RACE_MUX_HID_PORT);
#endif
#endif
    return true;
}

bool race_port_bt_close_all(void)
{
    RACE_LOG_MSGID_I("race_port_bt_close_all", 0);
#if defined(MTK_MUX_BT_ENABLE)
    race_port_bt_close(MUX_BT_SPP);
    race_port_bt_close(MUX_BT_BLE);
    race_port_bt_close(MUX_BT_BLE_1);
    race_port_bt_close(MUX_BT_BLE_2);
#if defined(MTK_GATT_OVER_BREDR_ENABLE)
    race_port_bt_close(MUX_BT_GATT_OVER_BREDR);
#endif
#ifdef MTK_AIRUPDATE_ENABLE
    race_port_bt_close(MUX_BT_AIRUPATE);
#endif
#endif
#if defined(MTK_IAP2_VIA_MUX_ENABLE) && defined(MTK_IAP2_PROFILE_ENABLE)
    race_port_bt_close(RACE_MUX_IAP2_PORT);
#endif
#ifdef AIR_MUX_BT_HID_ENABLE
    race_port_bt_close(MUX_HID_CONTROL);
#endif

    return true;
}

bt_bd_addr_t *race_get_bt_connection_addr(uint8_t channel_id)
{
    mux_port_t dev = channel_id;
    race_bt_conn_info_t *info = race_find_bt_conn_info_by_port(dev);

    if (info == NULL) {
        RACE_LOG_MSGID_E("race_get_bt_connection_addr, dev %d(channel %d) not support", 2, dev, channel_id);
        return NULL;
    }

    //RACE_LOG_MSGID_I("race_get_bt_connection_addr, dev %d(channel %d)", 2, dev, channel_id);
    return &(info->addr);
}

uint8_t race_get_channel_id_by_conn_address(bt_bd_addr_t *addr)
{
    uint8_t i = 0;
    if (!addr) {
        return RACE_INVALID_CHANNEL_ID;
    }
    for (i = 0; i < RACE_BT_CONNECTION_NUM; i ++) {
        if (memcmp(g_race_bt_connection[i].addr , addr , sizeof(bt_bd_addr_t)) == 0) {
            return g_race_bt_connection[i].dev;
        }
    }
    return RACE_INVALID_CHANNEL_ID;
}


void race_port_bt_init(void)
{
    int32_t ret = 0;
#ifdef RACE_ROLE_HANDOVER_SERVICE_ENABLE
#ifdef RACE_AWS_ENABLE
    bt_role_handover_callbacks_t rho_srv_callbacks = {
        race_rho_srv_allowed_callback,
        race_rho_get_data_len_callback,
        race_rho_get_data_callback,
        race_rho_update_data_callback,
        race_rho_srv_status_callback
    };
#endif
#endif

#ifdef RACE_BT_CMD_ENABLE
    /* Listen to BT Events */
    ret = bt_callback_manager_register_callback(bt_callback_type_app_event,
        MODULE_MASK_SYSTEM | MODULE_MASK_GAP | MODULE_MASK_HFP | MODULE_MASK_A2DP,
#ifdef RACE_BT_EVENT_MSG_HDL
        (void *)race_bt_app_event_handler
#else
        (void *)race_bt_event_process
#endif
        );


    if (BT_STATUS_SUCCESS != ret) {
        RACE_LOG_MSGID_E("race_init fail, ret[%d]", 1, ret);
        return;
    }

#ifdef RACE_ROLE_HANDOVER_SERVICE_ENABLE
#ifdef RACE_AWS_ENABLE
    bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_RACE_CMD, &rho_srv_callbacks);
#endif
#endif

    /* BT Init */
    race_bt_init();
#endif /* RACE_BT_CMD_ENABLE */

#ifdef MTK_RACE_EVENT_ID_ENABLE
    race_event_register(&g_race_event_register_id, race_event_bt_cb, NULL);
#else
    race_event_register(race_event_bt_cb, NULL);
#endif
}


bool race_bt_is_connected(mux_port_t dev)
{
    bt_bd_addr_t bdaddr = {0, 0, 0, 0, 0, 0};
    race_bt_conn_info_t *pinfo = race_find_bt_conn_info_by_port(dev);
    if (NULL != pinfo) {
        if (0 == memcmp(&bdaddr, &pinfo->addr, sizeof(bt_bd_addr_t))) {
            return false;
        } else {
            return true;
        }
    }
    return false;
}

