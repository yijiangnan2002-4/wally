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

// For Register AT command handler
// System head file
#include "hal_feature_config.h"
#include "atci_adapter.h"
#ifdef HAL_RTC_MODULE_ENABLED
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "at_command.h"
#include "hal_rtc.h"
#include "syslog.h"
#include <ctype.h>
#include "hal_rtc_internal.h"

#define RTC_AT_COMMAND_ENABLED


#ifdef RTC_AT_COMMAND_ENABLED

#ifdef HAL_RTC_FEATURE_POWER_REASON
extern int hal_rtc_get_power_on_reason(void);

static const char  *at_cmd_rtc_reason[] = {
    "Unknow",
    "Fisrt Power On",
    "RTC ALARM",
    "RTC TICK",
    "RTC EINT0",
    "RTC EINT0",
    "RTC EINT1",
    "RTC EINT2",
    "RTC CAPTOUCH",
    "RTC POWERKEY",
    "NULL"
};

#endif /* End HAL_RTC_FEATURE_POWER_REASON */

static const char *at_cmd_rtc_usage = {
    "AT+ERTC=REG,<OP>,<Address>,<Value>\r\n"
    "AT+ERTC=TEST,<OP>,[Para]\r\n"
};


static int mt_htoi(char s[])
{
    int n = 0;
    int i = 0;
    while (s[i] != '\0' && s[i] != '\n') {
        if (s[i] == '0') {
            if (s[i + 1] == 'x' || s[i + 1] == 'X') {
                i += 2;
            }
        }
        if (s[i] >= '0' && s[i] <= '9') {
            n = n * 16 + (s[i] - '0');
        } else if (s[i] >= 'a' && s[i] <= 'f') {
            n = n * 16 + (s[i] - 'a') + 10;
        } else if (s[i] >= 'A' && s[i] <= 'F') {
            n = n * 16 + (s[i] - 'A') + 10;
        } else {
            return n;
        }
        ++i;
    }
    return n;
}



static uint32_t gpt_time_handle = 0;

/**************************RTC CMD Usage********************************

AT+ERTC=REG,<OP>,<Address>,<Value>
    OP:W for write, R for read
    Address: Hex format
    Value: Hex format
AT+ERTC=TEST,<Mode>,[Para]
    <mode>\r\n"
        <0>: for get rtc time.
        <1>: enter rtc mode.
        <2>,<second>: set alarm time.
        <3>: get alarm time.
        <4>,<date>,<time> for set rtc time.
Example:
    AT+ERTC=REG,W,0xA2080000,0x103
    AT+ERTC=TEST,2,5 //set 5 second later alarm
    AT+ERTC=TEST,3,21-1-13,12:00:00

************************************************************************/

void    _atci_rtc_handle_rw_register(char *para_list[], int para_cnt,atci_response_t *response)
{
    uint8_t  opt;
    volatile uint32_t *addr;
    uint32_t val;

    /*command: AT+ERTC=REG,W,0,1234*/
    opt = *para_list[1];
    addr = (uint32_t *)mt_htoi(para_list[2]);
    if (opt == 'W') {
        val  = mt_htoi(para_list[3]);
        *addr = val;
    } else {
        val  = *addr;
    }
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC:%c,0x%x,0x%x\r\n", (unsigned int)opt, (unsigned int)addr, (unsigned int)val);
    /* ATCI will help append "OK" at the end of response buffer  */
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
}


void    _atci_rtc_handle_get_time(char *para_list[], uint16_t para_cnt, atci_response_t *response)
{
    hal_rtc_time_t time;

    hal_rtc_get_time(&time);
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: get time %d-%d-%d %d:%d:%d\r\n",  \
             time.rtc_year, time.rtc_mon, time.rtc_day,  \
             time.rtc_hour, time.rtc_min, time.rtc_sec);
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
}

/* AT+ERTC=TEST,2,<second> for set rtc alarm*/
void    _atci_rtc_handle_set_alarm(char *para_list[], uint16_t para_cnt, atci_response_t *response)
{
    uint32_t second;

    second = atoi(para_list[2]);
#ifdef HAL_RTC_FEATURE_ALARM_BY_SECOND
    hal_rtc_set_alarm_by_second(second);
#else
    hal_rtc_time_t  time;

    hal_rtc_get_time(&time);
    time.rtc_sec += second;
    if (time.rtc_sec > 59) {
        time.rtc_min++;
    }
    if (time.rtc_min > 59) {
        time.rtc_hour++;
    }
#endif
    hal_rtc_enable_alarm();
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: set alarm %d second\r\n", (unsigned int) second);
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
}

/* AT+ERTC=TEST,4,21-1-13,12:00:00 for get rtc date time*/
void    _atci_rtc_handle_set_time(char *para_list[], uint16_t para_cnt, atci_response_t *response)
{
    hal_rtc_time_t  time;
    char    *end_pos = NULL;

    printf("date:%s, time:%s", para_list[2], para_list[3]);
    memset(&time, 0, sizeof(time));
    /*date*/
    end_pos = para_list[2];
    time.rtc_year = atoi(end_pos);
    end_pos = strchr(end_pos, '-');
    end_pos++;
    time.rtc_mon = atoi(end_pos);
    end_pos = strchr(end_pos, '-');
    end_pos++;
    time.rtc_day = atoi(end_pos);

    /* time*/
    end_pos = para_list[3];
    time.rtc_hour = atoi(end_pos);
    end_pos = strchr(end_pos, ':');
    end_pos++;
    time.rtc_min = atoi(end_pos);
    end_pos = strchr(end_pos, ':');
    end_pos++;
    time.rtc_sec = atoi(end_pos);

    hal_rtc_set_time(&time);
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: set time %d-%d-%d %d:%d:%d\r\n",  \
             time.rtc_year, time.rtc_mon, time.rtc_day,  \
             time.rtc_hour, time.rtc_min, time.rtc_sec);
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
}

/* AT+ERTC=TEST,6,<32k mode>,<threshold>,<test tick> for measure 32k ppm */
void    _atci_rtc_handle_eosc_test(char *para_list[], uint16_t para_cnt, atci_response_t *response)
{
    uint32_t i = 0, times = 0, err_cnt = 0, threshold = 0;
    hal_rtc_osc32k_mode_t mode = 0;
    double  central_frequency, frequency;
    double  ppm = 0.0, min_ppm = 0.0, max_ppm = 0.0;

    /* 32k mode */
    mode    = (hal_rtc_osc32k_mode_t) atoi(para_list[2]);
    if (mode > 2) {
        goto error;
    }

    /* threshold value */
    threshold = atoi(para_list[3]);

    /* test times */
    times   = atoi(para_list[4]);
    if (times == 0) {
        times = 64;
    }
    //log_hal_msgid_info("+ERTC: mode(%d), threshold(%d), test_times(%d)", 3, (int)mode, (int)threshold, (int)times);
    for (i = 0; i < times; i++) {
        central_frequency = (double) hal_rtc_measure_32k_with_windows(mode, 99);
        frequency         = (double) hal_rtc_measure_32k_with_windows(mode, 654);
        ppm = ((frequency - central_frequency) * 1000000.0) / central_frequency;
        if (ppm < min_ppm) {
            min_ppm = ppm;
        }
        if (ppm > max_ppm) {
            max_ppm = ppm;
        }
        if (abs(ppm) >= threshold) {
            log_hal_msgid_info("[atci][rtc] IC eosc32k fail, ppm(%d) is bigger than threshold(%d)", 2, (int) abs(ppm), (int)threshold);
            err_cnt++;
        }
        log_hal_msgid_info("[atci][rtc] center_freq(%d), measure_freq(%d), ppm(%d)", 3, (int)central_frequency, (int)frequency, (int)ppm);
    }
    if (err_cnt > 0) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: FAIL (ErrTms %d,Min %f, Max %f)!!!!!\r\n", (int)err_cnt, (double)min_ppm, (double)max_ppm);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
    } else {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: PASS (Min %f, Max %f)!!!!!\r\n", (double)min_ppm, (double)max_ppm);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
    }
    return;
error:
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: para err(mode %d, threshold %d, times %d)\r\n", (int)mode, (int)threshold, (int)times);
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
    return;
}

void    gpt_timer_callback(void *args)
{
    hal_rtc_enter_rtc_mode();
}
/* AT+ERTC=TEST,1,<sec> for enter rtc mode*/
void    _atci_rtc_handle_rtc_mode(char *para_list[], uint32_t para_cnt,atci_response_t *response)
{
    uint32_t second = 0;
    hal_gpt_status_t gpt_sta;

    if (para_cnt > 2) {
        second = atoi(para_list[2]);
#ifdef HAL_RTC_FEATURE_ALARM_BY_SECOND
        hal_rtc_set_alarm_by_second(second);
#else
        hal_rtc_time_t  time;

        hal_rtc_get_time(&time);
        time.rtc_sec += second;
        if (time.rtc_sec > 59) {
            time.rtc_min++;
        }
        if (time.rtc_min > 59) {
            time.rtc_hour++;
        }
#endif
        hal_rtc_enable_alarm();
    }
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: RTC Mode For %ds\r\n", (int) second);

    gpt_sta = hal_gpt_sw_get_timer(&gpt_time_handle);
    if (gpt_sta != HAL_GPT_STATUS_OK) {
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        return;
    }
    gpt_sta = hal_gpt_sw_start_timer_ms(gpt_time_handle, 100, gpt_timer_callback, NULL);
    if (gpt_sta != HAL_GPT_STATUS_OK) {
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        return;
    }

    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
}

/* AT+ERTC=TEST,7,<pin>,<Afunc>,<Ctrl> for set rtc alarm*/
void    _atci_rtc_handle_32k_to_gpio(char *para_list[], uint16_t para_cnt, atci_response_t *response)
{
    uint32_t pin, pinfunc, ctrl_val;

    /* Pin Number */
    pin     = atoi(para_list[2]);

    /* Pin Afunction */
    pinfunc = atoi(para_list[3]);

    /* ctrl value */
    ctrl_val = mt_htoi(para_list[4]);
#ifdef HAL_RTC_FEATURE_32K_TO_GPIO
    hal_rtc_output_32k_to_gpio(pin, pinfunc, ctrl_val);
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: output 32k to gpio%d (ctrl 0x%x)\r\n", (int) pin, (unsigned int)ctrl_val);
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
#else
    (void) pin;
    (void) pinfunc;
    (void) ctrl_val;
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: No This Function\r\n");
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
}


void    _atci_rtc_handle_execution(char *param_list[], uint16_t param_cnt, atci_response_t *response)
{
    uint32_t mode;

    /*command: AT+ERTC=TEST,<mode>,[para]*/
    mode = atoi(param_list[1]);

    switch (mode) {
        case 0: /* AT+ERTC=TEST,0 for get rtc date time*/
            _atci_rtc_handle_get_time(param_list, param_cnt, response);
        break;

        case 1: /* AT+ERTC=TEST,1,<sec> for enter rtc mode*/
            _atci_rtc_handle_rtc_mode(param_list, param_cnt, response);
        break;

        case 2: /* AT+ERTC=TEST,2,<second> for set rtc alarm*/
            _atci_rtc_handle_set_alarm(param_list, param_cnt, response);
        break;

        case 3: {   /* AT+ERTC=TEST,3 for get rtc alarm time*/
            hal_rtc_time_t  time;
            hal_rtc_get_alarm(&time);
            snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: get alarm %d-%d-%d %d:%d:%d\r\n",  \
                     time.rtc_year, time.rtc_mon, time.rtc_day,  \
                     time.rtc_hour, time.rtc_min, time.rtc_sec);
            response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        } break;

        case 4: /* AT+ERTC=TEST,4,21-1-13,12:00:00 for get rtc date time*/
            _atci_rtc_handle_set_time(param_list, param_cnt, response);
        break;

        case 5: {
#ifdef HAL_RTC_FEATURE_POWER_REASON
            int reason;
            reason = hal_rtc_get_power_on_reason();
            snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC: %s\r\n", at_cmd_rtc_reason[reason]);
            response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
#else
            response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
        } break;
        case 6: /* AT+ERTC=TEST,6,<32k mode>,<threshold>,<test tick> for measure 32k ppm */
            _atci_rtc_handle_eosc_test(param_list, param_cnt, response);
        break;

        case 7: /* AT+ERTC=TEST,7,<pin>,<Afunc>,<Ctrl> for output 32k to gpio*/
            _atci_rtc_handle_32k_to_gpio(param_list, param_cnt, response);
        break;

        default:
            response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        break;
    }
}

void    _atci_rtc_handle_test(char *cmd_string, atci_response_t *response)
{
    uint32_t len;

    if (response == NULL) {
        return;
    }
    len = strlen(at_cmd_rtc_usage);
    if (len >=  ATCI_UART_TX_FIFO_BUFFER_SIZE) {
        len = (ATCI_UART_TX_FIFO_BUFFER_SIZE - 1);
    }
    strncpy((char *)response->response_buf, at_cmd_rtc_usage, len);
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
}


void    _atci_rtc_handle_rtc_gpio(char *para_list[], uint16_t para_cnt, atci_response_t *response)
{
    uint8_t  mode, pin;
    char    *pull_str[3] = {"PD", "PU", "No-Pull"};
    /*command:
        AT+ERTC=GPIO,GET,<IO>\r\n
        AT+ERTC=GPIO,SET,<IO>,<Level>\r\n
        AT+ERTC=GPIO,CFG,<IO>,<Dir>,<Pull>\r\n
    */
    mode = *para_list[1];
    pin  = atoi(para_list[2]);

    switch (mode) {
        case 'G': {
            bool level = 0;
            if (hal_rtc_gpio_get_input(pin, &level) == HAL_RTC_STATUS_OK) {
                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC:Get RTC_GPIO%d-> %d\r\n", (int) pin, level);
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
        }
        break;
        case 'S': {
            bool level = 0;
            level   = atoi(para_list[3]);
            if (hal_rtc_gpio_set_output(pin, level) == HAL_RTC_STATUS_OK) {
                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC:Set RTC_GPIO%d-> %d\r\n", (int) pin, level);
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
        }
        break;
        case 'C': {
            uint8_t dir, pull;
            hal_rtc_gpio_config_t config;

            dir = atoi(para_list[3]);
            pull = atoi(para_list[4]);

            config.rtc_gpio  = pin;
            config.is_analog = false;

            if (dir == 0) {
                config.is_input = true;
            } else {
                config.is_input = false;
            }

            if (pull == 0) {
                config.is_pull_down = true;
                config.is_pull_up   = false;
            } else if (pull == 1) {
                config.is_pull_down = false;
                config.is_pull_up   = true;
            } else {
                config.is_pull_down = false;
                config.is_pull_up   = false;
            }
            pull = (pull > 2) ? 2 : pull;

            if (hal_rtc_gpio_init(&config) == HAL_RTC_STATUS_OK) {
                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ERTC:Cfg RTC_GPIO%d,%s,%s\r\n", (int) pin,
                         dir ? "Output" : "Input", pull_str[pull]);
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
        }
        break;
        default:
            response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
}



#endif /* RTC_AT_COMMAND_ENABLED */



atci_status_t atci_cmd_hdlr_rtc(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *rsp;
    char  *param_list[8];
    uint16_t  param_cnt = 0;

    rsp = (atci_response_t *)atci_mem_alloc(sizeof(atci_response_t));
    if (!rsp) {
        LOG_MSGID_E(common, "[atci][rtc] malloc rsp fail\r\n", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(rsp, 0, sizeof(atci_response_t));

    param_cnt = atci_get_parameter_list(parse_cmd, (uint8_t **)&param_list[0], 8);
    rsp->response_flag = 0; // Command Execute Finish.
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    rsp->cmd_id = parse_cmd->cmd_id;
#endif

    LOG_I(common, "[atci][rtc] cmd string 1:%s", parse_cmd->string_ptr);

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING: {   // rec: AT+ERTCREG=?
#ifdef RTC_AT_COMMAND_ENABLED
            _atci_rtc_handle_test(parse_cmd->string_ptr, rsp);
#else
            rsp->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
        }
        break;
#ifdef RTC_AT_COMMAND_ENABLED
        case ATCI_CMD_MODE_EXECUTION: {
            if (strncmp(parse_cmd->string_ptr, "AT+ERTC=REG", strlen("AT+ERTC=REG")) == 0) {
                _atci_rtc_handle_rw_register(param_list, param_cnt, rsp);
            } else if (strncmp(parse_cmd->string_ptr, "AT+ERTC=TEST", strlen("AT+ERTC=TEST")) == 0) {
                _atci_rtc_handle_execution(param_list, param_cnt, rsp);
            } else if (strncmp(parse_cmd->string_ptr, "AT+ERTC=GPIO", strlen("AT+ERTC=GPIO")) == 0) {
                _atci_rtc_handle_rtc_gpio(param_list, param_cnt, rsp);
            } else {
                rsp->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
        }
        break;
#endif
        default:
            rsp->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    rsp->response_len = strlen((char *)rsp->response_buf);
    atci_send_response(rsp);
    atci_mem_free(rsp);
    return ATCI_STATUS_OK;
}

#endif /* HAL_RTC_MODULE_ENABLED */


