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


#include "FreeRTOS.h"
#include "task.h"
#include "syslog.h"
#include "mux.h"
#include "serial_port_assignment.h"

#include "atci_adapter.h"
#include "atci_main.h"
#include "at_command.h"

#include "race_xport.h"
#include "race_port_1wire.h"

#include "hal_dwt.h"

log_create_module(atci, PRINT_LEVEL_INFO);

/**************************************************************************************************
* Define
**************************************************************************************************/

#define ATCI_IS_HEADER_A(c)         ((c) == 'A' || (c) == 'a')
#define ATCI_IS_HEADER_T(c)         ((c) == 'T' || (c) == 't')
#define ATCI_IS_TAIL_LF(c)          ((c) == '\n')
#define ATCI_IS_VALID_CHAR(c)       ((c) > 0 && (c) <= 0x7F)

#define ATCI_MIN_SIZE               (4) /* "AT\r\n" is 4 byte */

#define ATCI_SUPPORT_PORT_MAX       (2)

#define ATCI_MSG_QUEUE_LENGTH       (15)

#define ATCI_RACE_HEADER            (0x0F92)
#define ATCI_PORT_USER_PRIORITY     (RACE_USER_PRIORITY_LOWEST + 2)
#define ATCI_DROP_RACE_HEADER_SIZE  (6)  /* 05 + 5A + length(2byte) + 0F92 */

#define ATCI_TASK_MAX_DELAY         (portMAX_DELAY)

#define ATCI_TX_BUFFER_SIZE         (256)
#define ATCI_RX_BUFFER_SIZE         (256)

/**************************************************************************************************
* Structure
**************************************************************************************************/

typedef void (*atci_general_msg_handler)(atci_general_msg_t *msg);


typedef enum {
    ATCI_PORT_CLOSED = 0,
    ATCI_SHARE_PORT_READY,
    ATCI_MONO_PORT_READY,
} atci_port_state_t;

enum {
    ATCI_SEARCH_A = 1,
    ATCI_SEARCH_T,
    ATCI_SEARCH_LF,
    ATCI_SEARCH_SUC,
};

enum {
    ATCI_SERVICE_INVALID = 0,
    ATCI_SERVICE_READY,
};

typedef struct {
    mux_port_t port;
    race_port_type_t port_type;
    mux_handle_t handle;
    atci_port_state_t state;
} atci_port_info_t;

typedef struct {
    uint32_t msg_queue;
    atci_port_info_t port_container[ATCI_SUPPORT_PORT_MAX];
    atci_port_info_t *focus_port;
    uint8_t state_machine;
    atci_general_msg_handler msg_handler[ATCI_MSG_ID_MAX];
} atci_ctrl_t;

/**************************************************************************************************
* Static Variable
**************************************************************************************************/
static atci_ctrl_t g_atci_ctrl;

/**************************************************************************************************
* Prototype
**************************************************************************************************/
static atci_status_t atci_local_init(void);
static bool atci_register_msg_handler(atci_msg_id_t msg_id, atci_general_msg_handler hdl);
static atci_port_info_t *atci_get_empty_port_info(void);
static atci_port_info_t *atci_get_port_info(mux_port_t port);
static void atci_update_focus_port(mux_port_t port);
static atci_port_info_t *atci_get_focus_port_info(void);
static uint32_t atci_get_multi_buf_size(const mux_buffer_t *buff, uint32_t buff_cnt);
static uint32_t atci_get_byte_from_multi_buf(const mux_buffer_t *buff, uint32_t buff_cnt, uint32_t index);
static race_tx_protocol_result_t atci_tx_protocol_callback(
    mux_handle_t handle, const mux_buffer_t payload[], uint32_t buffers_counter,
    mux_buffer_t *head, mux_buffer_t *tail, void *user_data);
static race_rx_protocol_result_t atci_rx_protocol_callback(
    mux_handle_t *handle, mux_buffer_t buffers[], uint32_t buffers_counter,
    uint32_t *consume_len, uint32_t *package_len, void *user_data);
static void atci_mux_callback(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data);
static uint32_t atci_get_drop_size(race_port_t port, RACE_COMMON_HDR_STRU *pkt_hdr, race_mux_buffer_t *pkt_buf);
static bool atci_check_race_at(race_port_t port, RACE_COMMON_HDR_STRU *pkt_hdr, race_mux_buffer_t *pkt_buf);
static void atci_config_port(race_port_init_t *port_config, race_port_t port, race_port_type_t type);
static void atci_config_user(race_user_config_t *user_config, race_port_t port, race_port_type_t type);
static atci_status_t atci_init_port_common(mux_port_t port, race_port_type_t port_type);
static atci_status_t atci_init_default_port(void);
static void atci_handle_input_data(atci_general_msg_t *msg);
static void atci_handle_output_data(atci_general_msg_t *msg);
static void atci_check_port(void);


/**************************************************************************************************
* Static Functions
**************************************************************************************************/

static atci_status_t atci_local_init(void)
{
    memset(&g_atci_ctrl, 0, sizeof(g_atci_ctrl));
    g_atci_ctrl.msg_queue = atci_queue_create(ATCI_MSG_QUEUE_LENGTH, sizeof(atci_general_msg_t));
    assert("ERROR: atci_local_init, create queue fail!" && g_atci_ctrl.msg_queue);

    atci_register_msg_handler(ATCI_MSG_ID_MUX_DATA_READY, atci_handle_input_data);
    atci_register_msg_handler(ATCI_MSG_ID_SEND_RSP_NOTIFY, atci_handle_output_data);
    atci_register_msg_handler(ATCI_MSG_ID_SEND_URC_NOTIFY, atci_handle_output_data);

    return ATCI_STATUS_OK;
}

static bool atci_register_msg_handler(atci_msg_id_t msg_id, atci_general_msg_handler hdl)
{
    if (msg_id < ATCI_MSG_ID_MAX) {
        g_atci_ctrl.msg_handler[msg_id] = hdl;
        return true;
    }
    return false;
}

static atci_port_info_t *atci_get_empty_port_info(void)
{
    uint16_t i;
    for (i = 0; i < ATCI_SUPPORT_PORT_MAX; i++) {
        if (ATCI_PORT_CLOSED == g_atci_ctrl.port_container[i].state) {
            return &g_atci_ctrl.port_container[i];
        }
    }
    return NULL;
}

static atci_port_info_t *atci_get_port_info(mux_port_t port)
{
    uint16_t i;
    for (i = 0; i < ATCI_SUPPORT_PORT_MAX; i++) {
        if (port == g_atci_ctrl.port_container[i].port && ATCI_PORT_CLOSED != g_atci_ctrl.port_container[i].state) {
            return &g_atci_ctrl.port_container[i];
        }
    }
    return NULL;
}

static void atci_update_focus_port(mux_port_t port)
{
    atci_port_info_t *info = atci_get_port_info(port);
    if (info) {
        g_atci_ctrl.focus_port = info;
        ATCI_LOG_I("[ATCI_MAIN] Update focus port:%d", 1, port);
    }
}

static atci_port_info_t *atci_get_focus_port_info(void)
{
    return g_atci_ctrl.focus_port;
}

static uint32_t atci_get_multi_buf_size(const mux_buffer_t *buff, uint32_t buff_cnt)
{
    uint32_t i;
    uint32_t res = 0;
    if (NULL == buff) {
        return 0;
    }
    for (i = 0; i < buff_cnt; i++) {
        res += buff[i].buf_size;
    }
    return res;
}

static uint32_t atci_get_byte_from_multi_buf(const mux_buffer_t *buff, uint32_t buff_cnt, uint32_t index)
{
    uint32_t buf_idx = 0;
    uint32_t offset = 0;
    uint32_t res = 0xffff;
    for (buf_idx = 0; buf_idx < buff_cnt; buf_idx++) {
        if (index >= offset + buff[buf_idx].buf_size) {
            offset += buff[buf_idx].buf_size;
        } else {
            if (buff[buf_idx].p_buf) {
                res = buff[buf_idx].p_buf[index - offset];
            }
            break;
        }
    }
    return res;
}

static race_tx_protocol_result_t atci_tx_protocol_callback(
    mux_handle_t handle, const mux_buffer_t payload[], uint32_t buffers_counter,
    mux_buffer_t *head, mux_buffer_t *tail, void *user_data)
{
    atci_port_info_t *pinfo = atci_get_port_info(RACE_GET_PORT_BY_MUX_HANDLE(handle));
    uint16_t len = atci_get_multi_buf_size(payload, buffers_counter);

    if (pinfo && ATCI_SHARE_PORT_READY == pinfo->state) {
        /* Add race header for share port */
        len += 2;  /* race id 2byte */
        head->p_buf[0] = RACE_CHANNEL_RACE;
        head->p_buf[1] = RACE_TYPE_NOTIFICATION;
        head->p_buf[2] = len & 0xff;
        head->p_buf[3] = (len >> 8) & 0xff;
        head->p_buf[4] = ATCI_RACE_HEADER & 0xff;
        head->p_buf[5] = (ATCI_RACE_HEADER >> 8) & 0xff;
        head->buf_size = 6;
    } else {
        head->buf_size = 0;
    }
    tail->buf_size = 0;
    return RACE_TX_PROTOCOL_RESULT_SUCCESS;
}


static race_rx_protocol_result_t atci_rx_protocol_callback(
    mux_handle_t *handle, mux_buffer_t buffers[], uint32_t buffers_counter,
    uint32_t *consume_len, uint32_t *package_len, void *user_data)
{
    race_port_info_t *race_info = (race_port_info_t *)user_data;
    atci_port_info_t *atci_info = NULL;
    uint32_t rx_len = atci_get_multi_buf_size(buffers, buffers_counter);
    uint8_t state = ATCI_SEARCH_A;
    uint32_t idx = 0;
    uint32_t byte = 0;
    uint32_t drop_len = 0;
    uint32_t pkt_len = 0;
    assert(race_info && "ERROR: atci_rx_protocol_callback, user_data is NULL !");

    while (idx < rx_len && state != ATCI_SEARCH_SUC) {
        byte = atci_get_byte_from_multi_buf(buffers, buffers_counter, idx);
        if (ATCI_IS_VALID_CHAR(byte) == false) {
            idx++;
            drop_len = idx;
            state = ATCI_SEARCH_A;
            continue;
        }

        switch (state) {
            case ATCI_SEARCH_A:
                if (ATCI_IS_HEADER_A(byte)) {
                    state = ATCI_SEARCH_T;
                    drop_len = idx;
                } else {
                    drop_len = idx + 1;
                }
                break;

            case ATCI_SEARCH_T:
                if (ATCI_IS_HEADER_T(byte)) {
                    state = ATCI_SEARCH_LF;
                } else {
                    state = ATCI_SEARCH_A;
                    drop_len = idx + 1;
                }
                break;

            case ATCI_SEARCH_LF:
                if (ATCI_IS_TAIL_LF(byte)) {
                    pkt_len = idx + 1 - drop_len;
                    state = ATCI_SEARCH_SUC;
                }
                break;

            default:
                break;
        }
        idx++;
    }

    *consume_len = drop_len;
    if (ATCI_SEARCH_SUC == state) {
        atci_info = atci_get_port_info(race_info->port);
        assert(atci_info && "ERROR: atci_rx_protocol_callback, atci info is NULL !");
        *handle = atci_info->handle;
        *package_len = pkt_len;
    }

    ATCI_LOG_I("rx_protocol_callback, rx_len:%d, drop_len:%d, pkt_len:%d, state", 4, rx_len, drop_len, pkt_len, state);
    return RACE_RX_PROTOCOL_RESULT_SUCCESS;
}

static void atci_mux_callback(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data)
{
#ifdef MTK_RACE_CMD_ENABLE
    atci_port_info_t *p_info = atci_get_port_info(RACE_GET_PORT_BY_MUX_HANDLE(handle));
    atci_general_msg_t msg_item;
    ATCI_LOG_I("atci_mux_callback, event:%d, p_info:0x%x", 2, event, (uint32_t)p_info);
    if (NULL == p_info) {
        return;
    }
    switch (event) {
        case MUX_EVENT_READY_TO_READ: {
            race_start_sleep_lock_timer();
            msg_item.msg_id = ATCI_MSG_ID_MUX_DATA_READY;
            msg_item.msg_data = (void *)data_len;
            msg_item.mux_index = handle;
            atci_send_msg(&msg_item);
            break;
        }

        case MUX_EVENT_WAKEUP_FROM_SLEEP:
            race_start_sleep_lock_timer();
            break;

        case MUX_EVENT_TRANSMISSION_DONE:
            if (p_info->port_type == RACE_PORT_TYPE_1WIRE_UART) {
                mux_control(p_info->port, MUX_CMD_UART_RX_ENABLE, NULL);
            }
            break;

        default:
            break;
    }
#else
    ATCI_LOG_E("ATCI NOT ENABLED:atci_mux_callback, event:%d, p_info:0x%x", 1, event);
#endif
}

static uint32_t atci_get_drop_size(race_port_t port, RACE_COMMON_HDR_STRU *pkt_hdr, race_mux_buffer_t *pkt_buf)
{
    /* drop race header(6byte) for race at cmd */
    return ATCI_DROP_RACE_HEADER_SIZE;
}

static bool atci_check_race_at(race_port_t port, RACE_COMMON_HDR_STRU *pkt_hdr, race_mux_buffer_t *pkt_buf)
{
    atci_port_info_t *pinfo = atci_get_port_info(port);
    if (NULL == pinfo || NULL == pkt_hdr) {
        return false;
    }
    if ((RACE_TYPE_COMMAND == pkt_hdr->type || RACE_TYPE_COMMAND_WITHOUT_RSP == pkt_hdr->type) && ATCI_RACE_HEADER == pkt_hdr->id) {
        if (ATCI_RX_BUFFER_SIZE <= pkt_hdr->length) {
            ATCI_LOG_E("ATCI pkt data too long:%d, length limitation:%d", 2, pkt_hdr->length, ATCI_RX_BUFFER_SIZE);
            return false;
        }
        return true;
    }
    return false;
}

static void atci_config_port(race_port_init_t *port_config, race_port_t port, race_port_type_t type)
{
    if (NULL == port_config || NULL == port_config->port_settings) {
        ATCI_LOG_E("atci_config_port, invalid parameter !", 0);
        return;
    }
    port_config->port = port;
    port_config->port_type = type;
    port_config->port_settings->tx_buffer_size = ATCI_TX_BUFFER_SIZE;
    port_config->port_settings->rx_buffer_size = ATCI_RX_BUFFER_SIZE;

    if (RACE_PORT_TYPE_NORMAL_UART == port_config->port_type || RACE_PORT_TYPE_1WIRE_UART == port_config->port_type) {
        port_config->port_settings->dev_setting.uart.uart_config.baudrate    = CONFIG_ATCI_BAUDRATE;
        port_config->port_settings->dev_setting.uart.uart_config.word_length = HAL_UART_WORD_LENGTH_8;
        port_config->port_settings->dev_setting.uart.uart_config.stop_bit    = HAL_UART_STOP_BIT_1;
        port_config->port_settings->dev_setting.uart.uart_config.parity      = HAL_UART_PARITY_NONE;
        port_config->port_settings->dev_setting.uart.flowcontrol_type        = MUX_UART_NONE_FLOWCONTROL;
    }
#if defined (AIR_1WIRE_ENABLE)
    if (RACE_PORT_TYPE_1WIRE_UART == type) {
        port_config->tx_function = race_1wire_tx;
    }
#endif
    port_config->tx_protocol_handler = atci_tx_protocol_callback;
    port_config->rx_protocol_handler = atci_rx_protocol_callback;
}

static void atci_config_user(race_user_config_t *user_config, race_port_t port, race_port_type_t type)
{
    if (NULL == user_config) {
        ATCI_LOG_E("atci_config_user, invalid parameter !", 0);
        return;
    }
    user_config->port = port;
    user_config->port_type = type;
    user_config->user_name = "ATCI";
    user_config->priority = ATCI_PORT_USER_PRIORITY;
    user_config->check_function = atci_check_race_at;
    user_config->get_drop_size = atci_get_drop_size;
    user_config->mux_event_handler = atci_mux_callback;
    user_config->tx_protocol_handler = atci_tx_protocol_callback;
}

static atci_status_t atci_init_port_common(mux_port_t port, race_port_type_t port_type)
{
    mux_port_setting_t atci_setting = {0};
    RACE_ERRCODE res = RACE_ERRCODE_FAIL;
    race_port_init_t port_config = {0};
    race_user_config_t user_config = {0};

    memset(&port_config, 0, sizeof(race_port_init_t));
    port_config.port_settings = &atci_setting;
    atci_config_port(&port_config, port, port_type);

#ifdef MTK_RACE_CMD_ENABLE
    res = race_init_port(&port_config);
    if (RACE_ERRCODE_SUCCESS != res) {
        return ATCI_STATUS_ERROR;
    }
#endif

    memset(&user_config, 0, sizeof(race_user_config_t));
    atci_config_user(&user_config, port, port_type);

#ifdef MTK_RACE_CMD_ENABLE
    res = race_open_port(&user_config);
#endif

    return res == RACE_ERRCODE_SUCCESS ? ATCI_STATUS_OK : ATCI_STATUS_ERROR;
}

static atci_status_t atci_init_default_port(void)
{
#ifdef MTK_RACE_CMD_ENABLE
    atci_status_t res1 = ATCI_STATUS_ERROR;
    atci_status_t res2 = ATCI_STATUS_ERROR;
    mux_status_t mux_status;
    mux_port_t atci_port1 = 0xff;
    mux_port_t atci_port2 = 0xff;
    mux_port_buffer_t query_port_buffer;

    /* step1. init share port */
    mux_status = mux_query_port_numbers_from_nvdm("SYSLOG", (mux_port_buffer_t *)&query_port_buffer);
    if (1 <= query_port_buffer.count && MUX_STATUS_OK == mux_status) {
        if (query_port_buffer.buf[0] != MUX_AIRAPP_0) { /* syslog via bt, not for atci*/
            atci_port1 = query_port_buffer.buf[0];
            res1 = atci_init_port(atci_port1, race_get_port_type(atci_port1));
        }
    }

    /* step2. init mono port */
    memset(&query_port_buffer, 0, sizeof(query_port_buffer));
    mux_status = mux_query_port_numbers_from_nvdm("ATCI", (mux_port_buffer_t *)&query_port_buffer);
    if (1 <= query_port_buffer.count && MUX_STATUS_OK == mux_status) {
        atci_port2 = query_port_buffer.buf[0];
    } else {
        atci_port2 = CONFIG_ATCI_PORT;
    }

    if (atci_port1 != atci_port2) {
        res2 = atci_init_port(atci_port2, race_get_port_type(atci_port2));
    }

    return (ATCI_STATUS_OK == res1 || ATCI_STATUS_OK == res2) ? ATCI_STATUS_OK : ATCI_STATUS_ERROR;
#else
    return ATCI_STATUS_REGISTRATION_FAILURE;
#endif
}


static void atci_handle_input_data(atci_general_msg_t *msg)
{
#ifdef MTK_RACE_CMD_ENABLE
    uint32_t handle;
    uint32_t data_len;
    uint32_t rev_len = 0;
    uint8_t *at_cmd = NULL;
    mux_buffer_t mux_buff;
    if (NULL == msg) {
        return;
    }
    ATCI_LOG_I("atci rev data", 1);
    data_len = (uint32_t)msg->msg_data;
    handle = msg->mux_index;
    at_cmd = (uint8_t *)atci_mem_alloc(data_len + 1);  /* +1 is for '\0' */
    mux_buff.p_buf = at_cmd;
    mux_buff.buf_size = data_len;
    if (race_receive_data(&rev_len, handle, &mux_buff) == false) {
        race_mem_free(at_cmd);
        return;
    }
    if (data_len != rev_len) {
        ATCI_LOG_E("atci_handle_input_data, receive data length error!", 0);
    }
    at_cmd[rev_len] = '\0';
    atci_update_focus_port(RACE_GET_PORT_BY_MUX_HANDLE(handle));
    atci_input_command_handler(at_cmd);
    atci_mem_free(at_cmd);
#endif
}

static void atci_handle_output_data(atci_general_msg_t *msg)
{
    uint8_t *buff = NULL;
    uint16_t buff_len = 0;
    if (NULL == msg || NULL == msg->msg_data) {
        return;
    }

    memcpy(&buff_len, msg->msg_data, sizeof(uint16_t));
    buff = msg->msg_data + sizeof(uint16_t);
    atci_send_data(buff, buff_len);
    atci_mem_free(msg->msg_data);
}

/* Check atci ports are closed by other module or not */
static void atci_check_port(void)
{
    uint16_t i;
    mux_status_t mux_state;
    mux_handle_t handle;
    for (i = 0; i < ATCI_SUPPORT_PORT_MAX; i++) {
        if (g_atci_ctrl.port_container[i].state != ATCI_PORT_CLOSED) {
            mux_state = mux_query_user_handle(g_atci_ctrl.port_container[i].port, "ATCI", &handle);
            if (MUX_STATUS_OK != mux_state) {
                ATCI_LOG_W("atci_check_port, port:%d, index:%d, is closed by other module", 2, g_atci_ctrl.port_container[i].port, i);
                if (g_atci_ctrl.focus_port == &g_atci_ctrl.port_container[i]) {
                    g_atci_ctrl.focus_port = NULL;
                }
                memset(&g_atci_ctrl.port_container[i], 0, sizeof(atci_port_info_t));
            }
        }
    }
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/

bool atci_is_service_ready(void)
{
    return (g_atci_ctrl.state_machine >= ATCI_SERVICE_READY);
}

void atci_send_msg(atci_general_msg_t *msg)
{
    if (atci_get_msg_queue_valid_size()) {
        atci_queue_send(g_atci_ctrl.msg_queue, (void *)msg);
    } else {
        ATCI_LOG_E("atci_send_msg, msg queue full, drop msg:%d", 1, msg->msg_id);
    }
}

atci_status_t atci_init_port(mux_port_t port, race_port_type_t type)
{
    uint32_t num;
    atci_port_info_t *p_info = NULL;
    bool is_share = false;
    atci_status_t res;

    atci_check_port();
    p_info = atci_get_port_info(port);
    if (p_info && ATCI_PORT_CLOSED != p_info->state) {
        return ATCI_STATUS_OK;
    }

    p_info = atci_get_empty_port_info();
    if (NULL == p_info) {
        return ATCI_STATUS_ERROR;
    }

    p_info->port = port;
    p_info->port_type = type;
    p_info->handle = 0;
    if (MUX_STATUS_OK == mux_query_port_user_number(port, &num)) {
        is_share = true;
    } else {
        is_share = false;
    }

    res = atci_init_port_common(port, type);
    if (ATCI_STATUS_OK == res) {
        mux_query_user_handle(port, "ATCI", &p_info->handle);
        p_info->state = is_share ? ATCI_SHARE_PORT_READY : ATCI_MONO_PORT_READY;
        atci_update_focus_port(port);
    }

    ATCI_LOG_I("atci_init_port, port:%d, type:%d, state:%d", 3, port, type, p_info->state);
    return res;
}

atci_status_t atci_deinit_port(mux_port_t port)
{
#ifdef MTK_RACE_CMD_ENABLE
    atci_port_info_t *pinfo = atci_get_port_info(port);
    race_close_port(port, "ATCI");
    if (pinfo) {
        if (g_atci_ctrl.focus_port == pinfo) {
            g_atci_ctrl.focus_port = NULL;
        }
        memset(pinfo, 0, sizeof(atci_port_info_t));
    }
#endif
    return ATCI_STATUS_OK;
}

/* Note: The parameter 'port' is not uesed, but this is SDK API, we cannot delete it */
atci_status_t atci_init(uint16_t port)
{
    atci_status_t res;

    atci_local_init();
    res = atci_init_default_port();

    if (ATCI_STATUS_OK == res) {
        res = at_command_init();
        if (ATCI_STATUS_OK == res) {
            g_atci_ctrl.state_machine = ATCI_SERVICE_READY;
        }
    }

    ATCI_LOG_I("atci_init res:%d!", 1, res);
    return res;
}

atci_status_t atci_deinit(hal_uart_port_t port)
{
    uint16_t i;

    g_atci_ctrl.state_machine = ATCI_SERVICE_INVALID;
    for (i = 0; i < ATCI_SUPPORT_PORT_MAX; i++) {
        if (ATCI_PORT_CLOSED != g_atci_ctrl.port_container[i].state) {
            atci_deinit_port(g_atci_ctrl.port_container[i].port);
        }
    }
    return ATCI_STATUS_OK;
}

atci_status_t atci_deinit_keep_table(hal_uart_port_t port)
{
    return atci_deinit(port);
}

atci_status_t atci_mux_port_reinit(mux_port_t port, bool is_share, bool is_1wire)
{
#ifdef MTK_RACE_CMD_ENABLE
    race_port_type_t port_type = is_1wire ? RACE_PORT_TYPE_1WIRE_UART : race_get_port_type(port);
    atci_port_info_t *pinfo = atci_get_port_info(port);
    uint8_t old_port = 0x7F;
    if (pinfo) {
        /* if this port exist, close this port and init it */
        atci_deinit_port(port);
        old_port = port;
    } else {
        /* if this port not exist, always close the first port */
        pinfo = &g_atci_ctrl.port_container[0];
        if (pinfo->state != ATCI_PORT_CLOSED) {
            old_port = pinfo->port | 0x80;
            atci_deinit_port(pinfo->port);
        }
    }

    ATCI_LOG_I("atci_mux_port_reinit, old port:0x%x, new port:0x%x", 2, old_port, port);
    return atci_init_port(port, port_type);
#else
    return ATCI_STATUS_ERROR;
#endif
}

atci_status_t atci_mux_port_resume(bool is_share, bool is_1wire)
{
    return atci_init_default_port();
}

atci_status_t atci_send_data(uint8_t *data, uint32_t data_len)
{
    atci_status_t res = ATCI_STATUS_ERROR;
    uint32_t sent_len = 0;
    atci_port_info_t *pinfo = atci_get_focus_port_info();
#ifdef MTK_RACE_CMD_ENABLE
    atci_dump_data(data, data_len, "atci_send_data");
    if (pinfo) {
        sent_len = race_send_data(pinfo->handle, data, data_len);
        res = (sent_len == data_len ? ATCI_STATUS_OK : ATCI_STATUS_ERROR);
    }
#endif
    ATCI_LOG_I("atci_send_data, port:0x%x, data_len:%d, sent_len%d", 3, (uint32_t)pinfo, data_len, sent_len);
    return res;
}

uint16_t atci_get_msg_queue_valid_size(void)
{
    return (ATCI_MSG_QUEUE_LENGTH - atci_queue_get_item_num(g_atci_ctrl.msg_queue));
}

void atci_processing(void)
{
    atci_general_msg_t msg_data;

    memset(&msg_data, 0, sizeof(atci_general_msg_t));
    ATCI_LOG_I("atci task running...", 0);

    while(1) {
        atci_queue_receive_wait(g_atci_ctrl.msg_queue, &msg_data, ATCI_TASK_MAX_DELAY);
        ATCI_LOG_I("atci task msg:%d", 1, msg_data.msg_id);
        if (ATCI_MSG_ID_MAX > msg_data.msg_id && NULL != g_atci_ctrl.msg_handler[msg_data.msg_id]) {
            g_atci_ctrl.msg_handler[msg_data.msg_id](&msg_data);
        }
        memset(&msg_data, 0, sizeof(atci_general_msg_t));
    }
}

void atci_task(void)
{
    atci_processing();
}


