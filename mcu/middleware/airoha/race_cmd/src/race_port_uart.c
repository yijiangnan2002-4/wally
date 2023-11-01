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


#include "hal.h"
#include "hal_uart_internal.h"
#include "serial_port_assignment.h"

#include "race_port_uart.h"


static const mux_port_setting_t g_race_uart_default_setting = {
    .tx_buffer_size = RACE_MUX_UART_TX_BUFFER_SIZE,
    .rx_buffer_size = RACE_MUX_UART_RX_BUFFER_SIZE,
    .dev_setting.uart.uart_config.baudrate = CONFIG_RACE_BAUDRATE,
    .dev_setting.uart.uart_config.word_length = HAL_UART_WORD_LENGTH_8,
    .dev_setting.uart.uart_config.stop_bit    = HAL_UART_STOP_BIT_1,
    .dev_setting.uart.uart_config.parity      = HAL_UART_PARITY_NONE,
    .dev_setting.uart.flowcontrol_type        = MUX_UART_NONE_FLOWCONTROL,
};

bool race_port_uart_prepare_open(race_port_t port)
{
    if (RACE_PORT_IS_UART(port)) {
        //uart_disable_irq(port); /* syslog has already disabled irq */
        return true;
    } else {
        return false;
    }
}

static mux_port_setting_t *race_port_uart_get_default_setting(void)
{
    return (mux_port_setting_t *)&g_race_uart_default_setting;
}

RACE_ERRCODE race_port_normal_uart_init(race_port_t port)
{
    RACE_ERRCODE res;
    race_port_init_t port_config = {0};
    race_user_config_t user_config = {0};
    if (race_port_uart_prepare_open(port) == false) {
        return RACE_ERRCODE_PARAMETER_ERROR;
    }
    memset(&port_config, 0, sizeof(race_port_init_t));
    port_config.port = port;
    port_config.port_type = RACE_PORT_TYPE_NORMAL_UART;
    port_config.port_settings = race_port_uart_get_default_setting();

    res = race_init_port(&port_config);
    if (RACE_ERRCODE_SUCCESS != res) {
        return res;
    }
    memset(&user_config, 0, sizeof(race_user_config_t));
    user_config.port = port;
    user_config.port_type = RACE_PORT_TYPE_NORMAL_UART;
    user_config.user_name = NULL;
    return race_open_port(&user_config);
}

race_port_t race_uart_get_active_port(void)
{
    race_port_info_t *buf[RACE_UART_PORT_MAX];
    uint16_t cnt = race_search_port_type(&buf[0], RACE_UART_PORT_MAX, RACE_PORT_TYPE_NORMAL_UART);
    uint16_t i;
    for (i = 0; i < cnt; i++) {
        if (NULL != buf[i] && RACE_PORT_INIT_SUC == buf[i]->port_status) {
            if (race_search_user_by_name(buf[i], "RACE_CMD") != NULL) {
                return buf[i]->port;
            }
        }
    }
    RACE_LOG_MSGID_W("[race_uart]active uart port is NULL.", 0);
    return RACE_INVALID_PORT;
}


