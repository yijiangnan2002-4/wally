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

#include "race_port_1wire.h"
#include "atci_main.h"

#if defined (AIR_1WIRE_ENABLE)

#define RACE_1WIRE_INIT_CMD         (1)
#define RACE_1WIRE_DEINIT_CMD       (2)


/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static void race_1wire_mux_callback(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data)
{
    bool res;
    if (MUX_EVENT_TRANSMISSION_DONE == event) {
        if(mux_control(RACE_GET_PORT_BY_MUX_HANDLE(handle), MUX_CMD_UART_RX_ENABLE, NULL) != MUX_STATUS_OK) {
            res = false;
        }
        RACE_LOG_MSGID_I("race, 1wire, set rx res:%d", 1, res);
    }
}

static bool race_1wire_prepare_open(race_port_t port)
{
    if (MUX_UART_BEGIN > port || MUX_UART_END < port) {
        return false;
    }
    race_port_info_t *port_info = race_search_port(port);
    if (NULL != port_info) {
        if (port_info->port_type != RACE_PORT_TYPE_1WIRE_UART) {
            race_close_port_for_all_user(port);
            return true;
        }
    } else {
        return true;
    }
    return false;
}

static void race_1wire_init_handler(race_general_msg_t *pmsg)
{
    race_port_t port;
    RACE_ERRCODE res = RACE_ERRCODE_MAX;
    uint32_t cmd;

    if (NULL == pmsg) {
        return ;
    }

    cmd = (uint32_t)pmsg->msg_data;
    if (RACE_1WIRE_INIT_CMD == cmd) {
        port = (race_port_t)pmsg->dev_t;
        res = race_init_1wire_port(port);
#if defined(MTK_ATCI_VIA_MUX) && defined(MTK_MUX_ENABLE)
        atci_mux_port_reinit(port, true, true);
#endif
        RACE_LOG_MSGID_I("race_1wire_init, port[%d], res[%d]", 2, port, res);
    } else if (RACE_1WIRE_DEINIT_CMD == cmd) {
        port = race_1wire_get_active_port();
        if (RACE_INVALID_PORT != port) {
            race_close_port_for_all_user(port);
        }
#if defined(MTK_ATCI_VIA_MUX) && defined(MTK_MUX_ENABLE)
        atci_mux_port_resume(true, true);
#endif
        RACE_LOG_MSGID_I("race_1wire_deinit", 0);
    }
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/

uint32_t race_1wire_tx(uint32_t mux_handle, uint8_t *p_data, uint32_t len)
{
    mux_status_t mux_status = MUX_STATUS_ERROR;
    uint32_t data_len = 0;
    mux_buffer_t mux_buffer = {p_data, len};

    mux_control(RACE_GET_PORT_BY_MUX_HANDLE(mux_handle), MUX_CMD_UART_TX_ENABLE, NULL);
    mux_status = mux_tx(mux_handle, &mux_buffer, 1, &data_len);
    RACE_LOG_MSGID_I("race, 1wire, send_data res:%d", 1, mux_status);
    if (MUX_STATUS_OK == mux_status) {
        return data_len;
    } else {
        mux_control(RACE_GET_PORT_BY_MUX_HANDLE(mux_handle), MUX_CMD_UART_RX_ENABLE, NULL);
        return 0;
    }
}

RACE_ERRCODE race_init_1wire_port(race_port_t port)
{
    RACE_ERRCODE res;
    race_port_init_t port_config = {0};
    race_user_config_t user_config = {0};
    uint8_t step = 0;

    if (race_1wire_prepare_open(port) == false) {
        res = RACE_ERRCODE_PARAMETER_ERROR;
        goto INIT_OVER;
    }
    step = 1;
    memset(&port_config, 0, sizeof(race_port_init_t));
    port_config.port = port;
    port_config.port_type = RACE_PORT_TYPE_1WIRE_UART;
    port_config.tx_function = race_1wire_tx;

    res = race_init_port(&port_config);
    if (res != RACE_ERRCODE_SUCCESS) {
        goto INIT_OVER;
    }
    step = 2;
    memset(&user_config, 0, sizeof(race_user_config_t));
    user_config.port = port;
    user_config.port_type = RACE_PORT_TYPE_1WIRE_UART;
    user_config.user_name = NULL;
    user_config.mux_event_post_handler = race_1wire_mux_callback;
    res = race_open_port(&user_config);

INIT_OVER:
    RACE_LOG_MSGID_I("race_init_1wire_port, step:%d, port:%d, res:%d", 3, step, port, res);
    return res;
}

void race_1wire_init(race_port_t port, uint32_t baudrate)
{
    race_general_msg_t msg_item = {0};
    msg_item.msg_id = MSG_ID_RACE_1WIRE_INIT_IND;
    msg_item.dev_t = (serial_port_dev_t)port;
    msg_item.msg_data = (uint8_t *)RACE_1WIRE_INIT_CMD;
    race_send_msg(&msg_item);
}

void race_1wire_deinit(void)
{
    race_general_msg_t msg_item = {0};
    msg_item.msg_id = MSG_ID_RACE_1WIRE_INIT_IND;
    msg_item.msg_data = (uint8_t *)RACE_1WIRE_DEINIT_CMD;
    race_send_msg(&msg_item);
}

void race_1wire_local_init(void)
{
    race_register_general_msg_hdl(MSG_ID_RACE_1WIRE_INIT_IND, race_1wire_init_handler);
}

#endif

bool race_port_is_1wire(race_port_t port)
{
    race_port_info_t *p_info = race_search_port(port);
    if (NULL != p_info && RACE_PORT_INIT_SUC == p_info->port_status) {
        if (RACE_PORT_TYPE_1WIRE_UART == p_info->port_type) {
            return true;
        }
    }
    return false;
}

race_port_t race_1wire_get_active_port(void)
{
    race_port_info_t *p_info = NULL;
    race_search_port_type(&p_info, 1, RACE_PORT_TYPE_1WIRE_UART);
    if (NULL != p_info && RACE_PORT_INIT_SUC == p_info->port_status) {
        return p_info->port;
    }
    RACE_LOG_MSGID_W("[race_1wire]active 1wire port is NULL.", 0);
    return RACE_INVALID_PORT;
}


