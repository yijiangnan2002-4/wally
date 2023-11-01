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
#include "race_cmd.h"
#include "race_cmd_nvdm.h"
#include "serial_port_assignment.h"

#include "race_port_cosys.h"

#ifdef AIR_RACE_CO_SYS_ENABLE

/**************************************************************************************************
* Define
**************************************************************************************************/

static mux_port_setting_t g_cosys_port_setting = {0};

#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
#if !defined (AIR_LOW_LATENCY_MUX_ENABLE)
static mux_port_setting_t g_emcu_port_setting = {0};
#endif
#endif

/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct {
    race_port_t cosys_port;
    race_port_t emcu_port;
    uint32_t cosys_handle;
    uint32_t emcu_handle;
} race_cosys_port_info_t;

/**************************************************************************************************
* Static Variable
**************************************************************************************************/
static race_cosys_port_info_t g_race_cosys_info = {RACE_INVALID_PORT, RACE_INVALID_PORT, 0, 0};

#ifndef AIR_LOW_LATENCY_MUX_ENABLE
static const char *g_race_cosys_user_name = "RACE_COSYS";
#endif

/**************************************************************************************************
* Prototype
**************************************************************************************************/
extern void race_cosys_tx_timer_init();
extern void race_cosys_tx_timer_start(void);
extern bool race_cosys_tx_timer_is_alive();
extern void race_cosys_rx_irq(mux_handle_t handle, uint32_t data_len);
extern bool race_relay_send_cosys(race_pkt_t *race_pkt, uint16_t length, uint8_t channel_id, uint8_t relay_type);

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
static race_tx_protocol_result_t race_port_cosys_tx_protocol_handler(
    mux_handle_t handle, const mux_buffer_t payload[], uint32_t buffers_counter, mux_buffer_t *head, mux_buffer_t *tail, void *user_data);
static void race_cosys_eint_callback(void *user_data);
#endif

#ifndef AIR_LOW_LATENCY_MUX_ENABLE
static void race_cosys_mux_event_handler(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data);
static bool race_check_cosys_user(race_port_t port, RACE_COMMON_HDR_STRU *pkt_hdr, race_mux_buffer_t *pkt_buf);
#endif
static void race_cosys_port_init_prepare(void);
static mux_port_setting_t *race_port_cosys_get_default_setting(void);

#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
#if !defined (AIR_LOW_LATENCY_MUX_ENABLE)
static mux_port_setting_t *race_port_emcu_get_default_setting(void);
static RACE_ERRCODE race_port_emcu_init(race_port_t port);
static bool race_port_emcu_deinit(race_port_t port);
#endif
#endif
static RACE_ERRCODE race_port_cosys_init(race_port_t port);
static bool race_port_cosys_deinit(race_port_t port);

#ifdef AIR_LOW_LATENCY_MUX_ENABLE
static uint32_t race_cosys_tx(uint32_t mux_handle, uint8_t *p_data, uint32_t len);
#endif

/**************************************************************************************************
* Static Functions
**************************************************************************************************/

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)

extern const unsigned char BSP_DUAL_CHIP_WAKEUP_EINT;
static void race_cosys_eint_callback(void *user_data)
{
    // User's handler
    race_start_sleep_lock_timer();

#if defined(AIR_BTA_IC_PREMIUM_G2)
    hal_eint_unmask(BSP_DUAL_CHIP_WAKEUP_EINT);
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
    hal_eint_unmask(HAL_EINT_RTC);
#endif
}

#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
static void race_cosys_port_rtc_gpio_init(void)
{
    hal_rtc_gpio_config_t gpio_config;
    gpio_config.is_analog = false;
    gpio_config.is_input = false;
    gpio_config.rtc_gpio = HAL_RTC_GPIO_1;
    gpio_config.is_pull_up = true;
    gpio_config.is_pull_down = false;
    hal_rtc_gpio_init(&gpio_config);
}

static void race_cosys_port_rtc_gpio_eint_init(void)
{
    hal_rtc_gpio_config_t gpio_config;
    gpio_config.is_analog = false;
    gpio_config.is_input = true;
    gpio_config.rtc_gpio = HAL_RTC_GPIO_1;
    gpio_config.is_pull_up = true;
    gpio_config.is_pull_down = false;
    hal_rtc_gpio_init(&gpio_config);

    hal_rtc_eint_config_t rtc_eint_config;
    rtc_eint_config.rtc_gpio = HAL_RTC_GPIO_1;
    rtc_eint_config.is_enable_rtc_eint = true;
    rtc_eint_config.is_enable_debounce = true;
    rtc_eint_config.is_falling_edge_active = true;
    hal_rtc_eint_init(&rtc_eint_config);
    hal_rtc_eint_register_callback(HAL_RTC_GPIO_1, race_cosys_eint_callback, NULL); /*Register RTC_GPIO EINT callback*/
    hal_eint_unmask(HAL_EINT_RTC);
}
#endif

static race_tx_protocol_result_t race_port_cosys_tx_protocol_handler(
    mux_handle_t handle, const mux_buffer_t payload[], uint32_t buffers_counter, mux_buffer_t *head, mux_buffer_t *tail, void *user_data)
{
    if (!race_cosys_tx_timer_is_alive()) {
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
#if defined(AIR_BTA_IC_PREMIUM_G2)
        extern const char BSP_DUAL_CHIP_TRIGGER_WAKEUP_PIN;
        hal_gpio_set_output(BSP_DUAL_CHIP_TRIGGER_WAKEUP_PIN, HAL_GPIO_DATA_LOW);
        hal_gpt_delay_ms(15);
        hal_gpio_set_output(BSP_DUAL_CHIP_TRIGGER_WAKEUP_PIN, HAL_GPIO_DATA_HIGH);
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
        race_cosys_port_rtc_gpio_init();

        hal_rtc_gpio_set_output(HAL_RTC_GPIO_1, false);
        hal_gpt_delay_ms(15);
        //hal_rtc_gpio_set_output(HAL_RTC_GPIO_1, true);

        race_cosys_port_rtc_gpio_eint_init();
#endif

#elif defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
#if defined(AIR_BTA_IC_PREMIUM_G2)
        extern const char BSP_DUAL_CHIP_TRIGGER_WAKEUP_PIN;
        hal_gpio_set_output(BSP_DUAL_CHIP_TRIGGER_WAKEUP_PIN, HAL_GPIO_DATA_LOW);
        hal_gpt_delay_ms(15);
        hal_gpio_set_output(BSP_DUAL_CHIP_TRIGGER_WAKEUP_PIN, HAL_GPIO_DATA_HIGH);
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
        race_cosys_port_rtc_gpio_init();

        hal_rtc_gpio_set_output(HAL_RTC_GPIO_1, false);
        hal_gpt_delay_ms(15);
       // hal_rtc_gpio_set_output(HAL_RTC_GPIO_1, true);

        race_cosys_port_rtc_gpio_eint_init();
#endif

#endif
    }
    race_cosys_tx_timer_start();
    return RACE_RX_PROTOCOL_RESULT_BYPASS;
}
#endif


#ifndef AIR_LOW_LATENCY_MUX_ENABLE
static void race_cosys_mux_event_handler(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data)
{
    RACE_LOG_MSGID_I("race_cosys_mux_event_handler, handle:0x%x, event:%d, data_len:%d", 3, handle, event, data_len);
    if (event == MUX_EVENT_READY_TO_READ) {
        race_cosys_rx_irq(handle, data_len);
    }
}

static bool race_check_cosys_user(race_port_t port, RACE_COMMON_HDR_STRU *pkt_hdr, race_mux_buffer_t *pkt_buf)
{
    uint8_t is_critical = 0;
    if (NULL == pkt_hdr || NULL == pkt_buf) {
        return false;
    }

    if ((race_cosys_get_port() == port || race_emcu_get_port() == port) == false) {
        return false;
    }

    if ((RACE_CHANNEL_RACE == pkt_hdr->pktId.value || RACE_CHANNEL_FOTA == pkt_hdr->pktId.value) &&
        (RACE_TYPE_COMMAND == pkt_hdr->type || RACE_TYPE_COMMAND_WITHOUT_RSP == pkt_hdr->type ||
        RACE_TYPE_COMMAND_WITHOUT_RSP == pkt_hdr->type || RACE_TYPE_NOTIFICATION == pkt_hdr->type) &&
        (0x110F == pkt_hdr->id)) {
        if (race_protocol_fetch(&is_critical, 1, pkt_buf) == true) {
            if (is_critical) {
                return true;
            }
        }
    }
    return false;
}
#endif

static void race_cosys_port_init_prepare(void)
{
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
#if defined(AIR_BTA_IC_PREMIUM_G2)
    extern const unsigned char BSP_DUAL_CHIP_WAKEUP_EINT;
    hal_eint_config_t config_eint;
    config_eint.trigger_mode = HAL_EINT_EDGE_FALLING;
    config_eint.debounce_time = 1;
    hal_eint_init(BSP_DUAL_CHIP_WAKEUP_EINT, &config_eint);
    hal_eint_register_callback(BSP_DUAL_CHIP_WAKEUP_EINT, race_cosys_eint_callback, NULL);
    //hal_eint_set_debounce_count(BSP_DUAL_CHIP_WAKEUP_EINT, 5); /* debounce time: (1/32k) * 5 = 165us*/
    hal_eint_unmask(BSP_DUAL_CHIP_WAKEUP_EINT);
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
    race_cosys_port_rtc_gpio_eint_init();
#endif

#elif defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
#if defined(AIR_BTA_IC_PREMIUM_G2)
    extern const unsigned char BSP_DUAL_CHIP_WAKEUP_EINT;
    hal_eint_config_t config_eint;
    config_eint.trigger_mode = HAL_EINT_EDGE_FALLING;
    config_eint.debounce_time = 1;
    hal_eint_init(BSP_DUAL_CHIP_WAKEUP_EINT, &config_eint);
    hal_eint_register_callback(BSP_DUAL_CHIP_WAKEUP_EINT, race_cosys_eint_callback, NULL);
    //hal_eint_set_debounce_count(BSP_DUAL_CHIP_WAKEUP_EINT, 5); /* debounce time: (1/32k) * 5 = 165us*/
    hal_eint_unmask(BSP_DUAL_CHIP_WAKEUP_EINT);
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
    race_cosys_port_rtc_gpio_eint_init();
#endif

#endif
}

static mux_port_setting_t *race_port_cosys_get_default_setting(void)
{
    g_cosys_port_setting.tx_buffer_size = RACE_COSYS_MUX_BUFF_SIZE;
    g_cosys_port_setting.rx_buffer_size = RACE_COSYS_MUX_BUFF_SIZE;
#if defined(AIR_LOW_LATENCY_MUX_ENABLE)
    g_cosys_port_setting.dev_setting.uart.uart_config.baudrate = HAL_UART_BAUDRATE_8666000;
#else
    g_cosys_port_setting.dev_setting.uart.uart_config.baudrate    = CONFIG_RACE_BAUDRATE;
#endif
    g_cosys_port_setting.dev_setting.uart.uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    g_cosys_port_setting.dev_setting.uart.uart_config.stop_bit    = HAL_UART_STOP_BIT_1;
    g_cosys_port_setting.dev_setting.uart.uart_config.parity      = HAL_UART_PARITY_NONE;
    g_cosys_port_setting.dev_setting.uart.flowcontrol_type        = MUX_UART_SW_FLOWCONTROL;
    return &g_cosys_port_setting;
}

static RACE_ERRCODE race_port_cosys_init(race_port_t port)
{
    RACE_ERRCODE res;
    race_port_init_t port_config = {0};
    race_user_config_t race_user_config = {0};

    race_cosys_port_init_prepare();

    memset(&port_config, 0, sizeof(race_port_init_t));
    port_config.port = port;
    port_config.port_type = RACE_PORT_TYPE_COSYS_UART;
    port_config.port_settings = race_port_cosys_get_default_setting();
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    port_config.tx_protocol_handler = race_port_cosys_tx_protocol_handler;
#endif
#ifdef AIR_LOW_LATENCY_MUX_ENABLE
    port_config.tx_function = race_cosys_tx;
#endif
    res = race_init_port(&port_config);
    if (RACE_ERRCODE_SUCCESS != res) {
        return res;
    }

    memset(&race_user_config, 0, sizeof(race_user_config_t));
    race_user_config.port = port;
    race_user_config.port_type = RACE_PORT_TYPE_COSYS_UART;
    race_user_config.user_name = NULL;
    res = race_open_port(&race_user_config);
    if (RACE_ERRCODE_SUCCESS != res) {
        return res;
    }

#ifndef AIR_LOW_LATENCY_MUX_ENABLE
    /*
    * Note: LL UART don't select mux_callback by use rx_protocol_callback.
    * The critical cmd is meaningless here.
    * So we don't need to open mux user "RACE_COSYS".
    */
    race_user_config_t cosys_user_config = {0};
    memset(&cosys_user_config, 0, sizeof(race_user_config_t));
    cosys_user_config.port = port;
    cosys_user_config.port_type = RACE_PORT_TYPE_COSYS_UART;
    cosys_user_config.user_name = g_race_cosys_user_name;
    cosys_user_config.mux_event_handler = race_cosys_mux_event_handler;
    cosys_user_config.check_function = race_check_cosys_user;
    cosys_user_config.priority = RACE_USER_PRIORITY_HIGHEST;
    res = race_open_port(&cosys_user_config);
#endif

    g_race_cosys_info.cosys_port = port;
    mux_query_user_handle(port, "RACE_CMD", &g_race_cosys_info.cosys_handle);
    return res;
}

static bool race_port_cosys_deinit(race_port_t port)
{
    race_close_port_for_all_user(port);
    return true;
}


#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
#if !defined (AIR_LOW_LATENCY_MUX_ENABLE)

static mux_port_setting_t *race_port_emcu_get_default_setting(void)
{
    g_emcu_port_setting.tx_buffer_size = RACE_EMCU_MUX_BUFF_SIZE;
    g_emcu_port_setting.rx_buffer_size = RACE_EMCU_MUX_BUFF_SIZE;
    return &g_emcu_port_setting;
}

static RACE_ERRCODE race_port_emcu_init(race_port_t port)
{
    RACE_ERRCODE res;
    //race_port_t port = RACE_DUAL_CHIP_EMCU_PORT;
    race_port_init_t port_config = {0};
    race_user_config_t race_user_config = {0};

    memset(&port_config, 0, sizeof(race_port_init_t));
    port_config.port = port;
    port_config.port_type = RACE_PORT_TYPE_EMCU_UART;
    port_config.port_settings = race_port_emcu_get_default_setting();

    res = race_init_port(&port_config);
    if (RACE_ERRCODE_SUCCESS != res) {
        return res;
    }

    memset(&race_user_config, 0, sizeof(race_user_config_t));
    race_user_config.port = port;
    race_user_config.port_type = RACE_PORT_TYPE_EMCU_UART;
    race_user_config.user_name = NULL;
    res = race_open_port(&race_user_config);

    g_race_cosys_info.emcu_port = port;
    mux_query_user_handle(port, "RACE_CMD", &g_race_cosys_info.emcu_handle);

    return res;
}

static bool race_port_emcu_deinit(race_port_t port)
{
    race_close_port_for_all_user(port);
    return true;
}

#endif
#endif



#ifdef AIR_LOW_LATENCY_MUX_ENABLE
static uint32_t race_cosys_tx(uint32_t mux_handle, uint8_t *p_data, uint32_t len)
{
    uint32_t sent_len;
    mux_buffer_t mux_buffer = {p_data, len};
    mux_tx(mux_handle, &mux_buffer, 1, &sent_len);

    race_dump_data(p_data, len, "race_tx_cosys");
    RACE_LOG_MSGID_I("race_tx_cosys, data_len:%d, tx_len:%d", 2, len, sent_len);
    return sent_len;
}
#endif

/**************************************************************************************************
* Public Functions
**************************************************************************************************/

void race_open_cosys_port(void)
{
    race_port_cosys_init(RACE_DUAL_CHIP_COSYS_PORT);
}

void race_close_cosys_port(void)
{
    race_port_cosys_deinit(RACE_DUAL_CHIP_COSYS_PORT);
}

#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
#if !defined (AIR_LOW_LATENCY_MUX_ENABLE)
void race_open_emcu_port(void)
{
    race_port_emcu_init(RACE_DUAL_CHIP_EMCU_PORT);
}

void race_close_emcu_port(void)
{
    race_port_emcu_deinit(RACE_DUAL_CHIP_EMCU_PORT);
}
#endif
#endif

void race_cosys_init(void)
{
    race_open_cosys_port();
#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
#if !defined (AIR_LOW_LATENCY_MUX_ENABLE)
    race_open_emcu_port();
#endif
#endif
    race_cosys_tx_timer_init();
    race_cosys_nvkey_init();
}

race_port_t race_cosys_get_port(void)
{
    RACE_LOG_MSGID_I("[race_cosys]active cosys port is %d.", 1, g_race_cosys_info.cosys_port);
    return g_race_cosys_info.cosys_port;
}

uint32_t race_cosys_get_handle(void)
{
    return g_race_cosys_info.cosys_handle;
}

race_port_t race_emcu_get_port(void)
{
    RACE_LOG_MSGID_I("[race_emcu]active emcu port is %d.", 1, g_race_cosys_info.emcu_port);
    return g_race_cosys_info.emcu_port;
}

uint32_t race_emcu_get_handle(void)
{
    return g_race_cosys_info.emcu_handle;
}


#endif


