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
#include "FreeRTOS.h"
#include "timers.h"
#include "portmacro.h"
#include "serial_port_assignment.h"
#include "usb_case.h"
#include "race_event.h"
#include "race_usb_relay.h"
#include "race_event_internal.h"

#include "atci_main.h"

#include "race_port_usb.h"

#if defined (MTK_USB_DEMO_ENABLED)

/**************************************************************************************************
* Define
**************************************************************************************************/



/**************************************************************************************************
* Structure
**************************************************************************************************/
/*
* used for usb plug in/out
*/
typedef struct {
    uint32_t user_mask; // record user in g_race_usb_user_list as bit mask
    race_port_t port;
} race_usb_port_cache_t;

/*
* used for usb port data monitor
* RX DATA IRQ: start timer
* received ready to read event: stop timer
* if timer expired, clear usb data
*/
typedef struct {
    const char *const timer_name;
    TimerHandle_t timer;
    bool timer_started;
} race_usb_port_monitor_t;

/**************************************************************************************************
* Static Variable
**************************************************************************************************/
static race_usb_port_cache_t g_race_usb_port_cache[RACE_USB_PORT_MAX];

static const char *g_race_usb_user_list[] = {
    "RACE_CMD",
    "ATCI"
};

static const mux_port_setting_t g_race_usb_default_setting = {
    .tx_buffer_size = RACE_MUX_USB_TX_BUFFER_SIZE,
    .rx_buffer_size = RACE_MUX_USB_RX_BUFFER_SIZE,
#if 0
    .dev_setting.uart.uart_config.baudrate    = CONFIG_RACE_BAUDRATE,
    .dev_setting.uart.uart_config.word_length = HAL_UART_WORD_LENGTH_8,
    .dev_setting.uart.uart_config.stop_bit    = HAL_UART_STOP_BIT_1,
    .dev_setting.uart.uart_config.parity      = HAL_UART_PARITY_NONE,
    .dev_setting.uart.flowcontrol_type        = MUX_UART_NONE_FLOWCONTROL,
#endif
};

#ifdef MTK_USB_AUDIO_HID_ENABLE
static race_usb_port_monitor_t g_race_usb_monitor[RACE_USB_PORT_MAX] = {
    {"race_usb_com_1_rx_timer", NULL, false},
    {"race_usb_com_2_rx_timer", NULL, false}
};
#endif

/**************************************************************************************************
* Prototype
**************************************************************************************************/

#if defined(MTK_USB_AUDIO_HID_ENABLE)
static void race_rx_clean_timer_callback(TimerHandle_t xtimer);
#endif

static void race_mux_rx_protocol_timer_check(race_port_t port);
static void race_mux_rx_protocol_timer_stop(race_port_t port);
static mux_port_setting_t *race_port_usb_get_default_setting(void);
static void race_usb_mux_event_handler(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data);
static void race_usb_cache_state(void);
static void race_mux_usb_callback(void *param);
static void race_usb_plug_event_handler(race_general_msg_t *pmsg);

/**************************************************************************************************
* Static Functions
**************************************************************************************************/

#ifdef MTK_USB_AUDIO_HID_ENABLE
static void race_rx_clean_timer_callback(TimerHandle_t xtimer)
{
    uint16_t i;
    if (NULL == xtimer) {
        return;
    }
    for (i = 0; i < RACE_USB_PORT_MAX; i++) {
        if (g_race_usb_monitor[i].timer == xtimer) {
            race_port_t port = MUX_USB_BEGIN + i;
            mux_control(port, MUX_CMD_CLEAN_RX_VIRUTUAL, NULL);
            RACE_LOG_MSGID_I("[race_usb] race_usb_rx_clean_timer timeout, reset usb rx buff, mux port:0x%02x", 1, port);
        }
    }
}
#endif

static void race_mux_rx_protocol_timer_check(race_port_t port)
{
#ifdef MTK_USB_AUDIO_HID_ENABLE
    uint16_t idx = 0;
    if (RACE_PORT_IS_USB(port) == false) {
        return;
    }
    idx = port - MUX_USB_BEGIN;
    if (g_race_usb_monitor[idx].timer == NULL) {
        return;
    }
    if (false == g_race_usb_monitor[idx].timer_started) {
        g_race_usb_monitor[idx].timer_started = TRUE;
        bool res = true;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (xTimerStartFromISR(g_race_usb_monitor[idx].timer, &xHigherPriorityTaskWoken) != pdPASS) {
            res = false;
        }
        if (xHigherPriorityTaskWoken != pdFALSE) {
            // Actual macro used here is port specific.
            portYIELD_FROM_ISR(pdTRUE);
        }
        RACE_LOG_MSGID_I("[race_usb] start race_usb_rx_clean_timer res:%d", 1, res);
    }
#endif
}

static void race_mux_rx_protocol_timer_stop(race_port_t port)
{
#ifdef MTK_USB_AUDIO_HID_ENABLE
    uint16_t idx = 0;
    if (RACE_PORT_IS_USB(port) == false) {
        return;
    }
    idx = port - MUX_USB_BEGIN;
    if (g_race_usb_monitor[idx].timer == NULL) {
        return;
    }

    if (g_race_usb_monitor[idx].timer_started) {
        bool res = true;
        g_race_usb_monitor[idx].timer_started = false;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (xTimerStopFromISR(g_race_usb_monitor[idx].timer, &xHigherPriorityTaskWoken) != pdPASS) {
            res = false;
        }
        if (xHigherPriorityTaskWoken != pdFALSE) {
            // Actual macro used here is port specific.
            portYIELD_FROM_ISR(pdTRUE);
        }
        RACE_LOG_MSGID_I("[race_usb] stop race_usb_rx_clean_timer, res:%d", 1, res);
    }
#endif
}

static mux_port_setting_t *race_port_usb_get_default_setting(void)
{
    return (mux_port_setting_t *)&g_race_usb_default_setting;
}

static void race_usb_mux_event_handler(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data)
{
    switch (event) {
        case MUX_EVENT_READY_TO_READ: {
            race_mux_rx_protocol_timer_stop(RACE_GET_PORT_BY_MUX_HANDLE(handle));
            break;
        }
        case MUX_EVENT_CONNECTION: {
            race_send_event_notify_msg(RACE_EVENT_TYPE_CONN_USB_CONNECT, NULL);
            break;
        }
        case MUX_EVENT_DISCONNECTION: {
            race_send_event_notify_msg(RACE_EVENT_TYPE_CONN_USB_DISCONNECT, NULL);
            break;
        }
        default:
            break;
    }
}

static void race_usb_cache_state(void)
{
    race_port_info_t *info[RACE_USB_PORT_MAX];
    uint16_t cnt, i, j;
    uint16_t user_num = sizeof(g_race_usb_user_list) / sizeof(g_race_usb_user_list[0]);
    race_port_user_t *user;

    cnt = race_search_port_type(info, RACE_USB_PORT_MAX, RACE_PORT_TYPE_USB);
    memset(&g_race_usb_port_cache[0], 0, sizeof(g_race_usb_port_cache));
    for (i = 0; i < cnt; i++) {
        g_race_usb_port_cache[i].port = info[i]->port;
        for (j = 0; j < user_num; j++) {
            user = race_search_user_by_name(info[i], g_race_usb_user_list[j]);
            if (user && user->status == RACE_USER_ACTIVE) {
                g_race_usb_port_cache[i].user_mask |= 1 << j;
            }
        }
    }
}

static bool race_usb_is_user_masked(const char *user_name, uint32_t mask)
{
    uint16_t user_num = sizeof(g_race_usb_user_list) / sizeof(g_race_usb_user_list[0]);
    uint16_t i;
    uint32_t temp;
    for (i = 0; i < user_num; i++) {
        if (strcmp(g_race_usb_user_list[i], user_name) == 0) {
            temp = 1 << i;
            return ((temp & mask) != 0);
        }
    }
    return false;
}

static void race_mux_usb_callback(void *param)
{
    /* move to race task, to avoid timing issue: race is sending data, but port is closed by irq or a higher priority task. */
    race_general_msg_t msg_item = {0};
    msg_item.msg_id = MSG_ID_RACE_USB_PLUG_IND;
    msg_item.msg_data = (uint8_t *)param;
    race_send_msg(&msg_item);
    RACE_LOG_MSGID_I("[race_usb] plug event:%d", 1, (uint32_t)param);
}

static void race_usb_plug_event_handler(race_general_msg_t *pmsg)
{
    bool plug_in;
    if (pmsg) {
        plug_in = (bool)pmsg->msg_data;
        if (plug_in) {
            race_usb_port_reinit();
        } else {
            race_usb_port_deinit();
        }
    }
}

static race_rx_protocol_result_t race_usb_rx_protocol_handler(
    mux_handle_t *handle, mux_buffer_t buffers[], uint32_t buffers_counter, uint32_t *consume_len, uint32_t *package_len, void *user_data)
{
    race_port_info_t *p_info = (race_port_info_t *)user_data;
    if (NULL != p_info) {
        race_mux_rx_protocol_timer_check(p_info->port);
    }
    return RACE_RX_PROTOCOL_RESULT_BYPASS;
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/

void race_usb_init(void)
{
    memset(&g_race_usb_port_cache[0], 0, sizeof(g_race_usb_port_cache));
    race_register_general_msg_hdl(MSG_ID_RACE_USB_PLUG_IND, race_usb_plug_event_handler);
    usb_case_register_race_callback(race_mux_usb_callback);
}

RACE_ERRCODE race_usb_port_init(race_port_t port)
{
    RACE_ERRCODE res;
    race_port_init_t port_config = {0};
    race_user_config_t user_config = {0};
    if (RACE_PORT_IS_USB(port) == false) {
        return RACE_ERRCODE_PARAMETER_ERROR;
    }
    memset(&port_config, 0, sizeof(race_port_init_t));
    port_config.port = port;
    port_config.port_type = RACE_PORT_TYPE_USB;
    port_config.port_settings = race_port_usb_get_default_setting();
    port_config.rx_protocol_handler = race_usb_rx_protocol_handler;

    res = race_init_port(&port_config);
    if (RACE_ERRCODE_SUCCESS != res) {
        return res;
    }
    memset(&user_config, 0, sizeof(race_user_config_t));
    user_config.port = port;
    user_config.port_type = RACE_PORT_TYPE_USB;
    user_config.user_name = NULL;
    user_config.mux_event_post_handler = race_usb_mux_event_handler;
    res = race_open_port(&user_config);
    if (RACE_ERRCODE_SUCCESS != res) {
        return res;
    }
#ifdef MTK_USB_AUDIO_HID_ENABLE
    uint16_t idx;
    idx = port - MUX_USB_BEGIN;
    if (g_race_usb_monitor[idx].timer == NULL) {
        g_race_usb_monitor[idx].timer = xTimerCreate(g_race_usb_monitor[idx].timer_name,
            (2 * 1000 / portTICK_PERIOD_MS), pdFALSE, NULL, race_rx_clean_timer_callback);
    }
#endif
    return res;
}

bool race_usb_port_deinit(void)
{
    uint16_t i;
    race_usb_cache_state();
    for (i = 0; i < RACE_USB_PORT_MAX; i++) {
        if (g_race_usb_port_cache[i].user_mask) {
            if (race_usb_is_user_masked("ATCI", g_race_usb_port_cache[i].user_mask) == true) {
                atci_deinit_port(g_race_usb_port_cache[i].port);
            }
            race_close_port_for_all_user(g_race_usb_port_cache[i].port);
#if defined RACE_USB_RELAY_ENABLE
            race_usb_relay_clear_flag(g_race_usb_port_cache[i].port);
#endif
        }
    }
    return true;
}

bool race_usb_port_reinit(void)
{
    uint16_t i;
    for (i = 0; i < RACE_USB_PORT_MAX; i++) {
        if (g_race_usb_port_cache[i].user_mask) {
            if (race_usb_is_user_masked("RACE_CMD", g_race_usb_port_cache[i].user_mask) == true) {
                race_usb_port_init(g_race_usb_port_cache[i].port);
            }
#if defined RACE_USB_RELAY_ENABLE
            race_usb_relay_clear_flag(g_race_usb_port_cache[i].port);
#endif
            if (race_usb_is_user_masked("ATCI", g_race_usb_port_cache[i].user_mask) == true) {
                atci_init_port(g_race_usb_port_cache[i].port, RACE_PORT_TYPE_USB);
            }
        }
    }
    return true;
}

#endif

race_port_t race_usb_get_active_port(void)
{
    race_port_info_t *buf[RACE_USB_PORT_MAX];
    uint16_t cnt = race_search_port_type(&buf[0], RACE_USB_PORT_MAX, RACE_PORT_TYPE_USB);
    uint16_t i;
    for (i = 0; i < cnt; i++) {
        if (NULL != buf[i] && RACE_PORT_INIT_SUC == buf[i]->port_status) {
            return buf[i]->port;
        }
    }
    RACE_LOG_MSGID_W("[race_usb]active usb port is NULL.", 0);
    return RACE_INVALID_PORT;
}


