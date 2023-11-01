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

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "at_command.h"
#include "hal_gpt.h"
#include "syslog.h"
#include <stdlib.h>

#ifdef HAL_GPIO_MODULE_ENABLED

#include "hal_gpio.h"
#if (PRODUCT_VERSION != 7687 && PRODUCT_VERSION != 7697)
#include "hal_gpio_internal.h"
#endif
/*
 * sample code
 * AT+EGPIO=GPIO_GET
 * AT+EGPIO=GPIO_SET:<pin number>:<mode><dir><pull><output>
 * AT+EGPIO=GPIO_SET_MODE:<pin number>:<mode>
 * ... ...
*/

log_create_module(atci_gpio, PRINT_LEVEL_INFO);
#define LOGE(fmt,arg...)   LOG_E(atci_gpio, "[GPIO]"fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(atci_gpio, "[GPIO]"fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(atci_gpio ,"[GPIO]"fmt,##arg)

#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atci_gpio ,"[GPIO]"fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atci_gpio ,"[GPIO]"fmt,cnt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atci_gpio ,"[GPIO]"fmt,cnt,##arg)

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_gpio(atci_parse_cmd_param_t *parse_cmd);


/* AT command handler  */

void atci_gpio_send_response(int32_t *len1, atci_response_t *response)
{
    response->response_len = (uint16_t)(* len1);
    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    atci_send_response(response);
}

#ifdef AG3335
#include "hal_eint.h"
volatile uint32_t pps_output_plus_status = 0;
void pps_test_callback(void *user_data)
{
    pps_output_plus_status = 1;
}
#endif

atci_status_t atci_cmd_hdlr_gpio(atci_parse_cmd_param_t *parse_cmd)
{
#if (PRODUCT_VERSION == 7687 || PRODUCT_VERSION == 7697)
    //atci_response_t response = {{0}};
    atci_response_t *response = atci_mem_alloc(sizeof(atci_response_t));
#else
    //atci_response_t response = {{0}};
    atci_response_t *response = atci_mem_alloc(sizeof(atci_response_t));

    char *param1 = NULL;
    char *param2 = NULL;
    char *param3 = NULL;
    char *param4 = NULL;
    char *param5 = NULL;
    char *param6 = NULL;
    char *param7 = NULL;

    const char *direct[2] = {"input ", \
                             "output"
                            };
#if (PRODUCT_VERSION == 3335)
    const char current_state[8] = {1, 2, 4, 8, 12, 16, 18, 20};
    #define   CURRENT_STRING   "drv:0-1ma, 1-2ma, 2-4ma, 3-8ma, 4-12ma, 5-16ma, 6-18ma, 7-20ma\r\n"
#elif (AIR_BTA_IC_PREMIUM_G2)
    const char current_state[4] = {2, 4, 6, 8};
    #define   CURRENT_STRING   "drv:0-2ma, 1-4ma, 2-6ma, 3-8ma\r\n"
#elif defined(AIR_BTA_IC_PREMIUM_G3)
    const char current_state[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    #define   CURRENT_STRING   "drv:0-2ma, 1-4ma, 2-6ma, 3-8ma, 4-10ma, 5-12ma, 6-14ma, 7-16ma\r\n"
#else
    const char current_state[4] = {4, 8, 12, 16};
    #define   CURRENT_STRING   "drv:0-4ma, 1-8ma, 2-12ma, 3-16ma\r\n"
#endif
    const char *pull_state[10] = {"NO_PULL ", \
                                  "PU_R  ", \
                                  "PD_R  ", \
                                  "PU_R0  ", \
                                  "PD_R0  ", \
                                  "PU_R0_R1", \
                                  "PD_R0_R1", \
                                  "PUPD_ERR", \
                                  "PU_R1  ", \
                                  "PD_R1  "
                                 };

    uint8_t i, error_flag;
    int32_t ret_len1;
    uint32_t config_data[10];

    hal_gpio_pin_t pin_index[2];
    gpio_state_t gpio_state;
#ifdef HAL_GPIO_FEATURE_PUPD
    hal_gpio_status_t ret_state;
#endif
#endif
    response->response_flag = 0; /*    Command Execute Finish.  */
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response->cmd_id = parse_cmd->cmd_id;
#endif

    /*
        sprintf((char *)response->response_buf, "mode=%d\r\n",parse_cmd->mode);
        response->response_len = strlen((char *)response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_URC_FORMAT;
        atci_send_response(response);
        */

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:
#if (PRODUCT_VERSION == 7687 || PRODUCT_VERSION == 7697)
            strncpy((char *)response->response_buf, "Not support GPIO AT command", sizeof("Not support GPIO AT command"));
            response->response_len = strlen((char *)response->response_buf);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(response);
#else
            strncpy((char *)response->response_buf, "ATCI_CMD_MODE_TESTING", sizeof("ATCI_CMD_MODE_TESTING"));
            response->response_len = strlen((char *)response->response_buf);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(response);

            ret_len1 =  snprintf((char *)(response->response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "\r\nAT+EGPIO=GPIO_GET:m~n get all  GPIO information\r\n n must >= m, n<%d\r\n", HAL_GPIO_MAX);
            atci_gpio_send_response(&ret_len1, response);

            ret_len1 =  snprintf((char *)(response->response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "\r\nAT+EGPIO=GPIO_SET_MODE:g,m  set mode to one pin\r\n g:gpio pin number\r\n m:mode number\r\n");
            atci_gpio_send_response(&ret_len1, response);

            ret_len1 =  snprintf((char *)(response->response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "\r\nAT+EGPIO=GPIO_SET_DIR:g,d  set dir to one pin\r\n g:gpio pin number\r\n d:direction value, 1-ouput, 0-input\r\n");
            atci_gpio_send_response(&ret_len1, response);

            ret_len1 =  snprintf((char *)(response->response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "\r\nAT+EGPIO=GPIO_SET_PULL:g,p  set pull state to one pin\r\n g:gpio pin number\r\n p:pull state value, 0-pullup, 1-pulldown,2-disable pull\r\n");
            atci_gpio_send_response(&ret_len1, response);

            ret_len1 =  snprintf((char *)(response->response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "\r\nAT+EGPIO=GPIO_SET_PUPD:g,pupd,r0,r1  set resistor to one pin\r\n g:gpio pin number\r\n pupd:pull select, 0-pullup, 1-pulldown\r\n r0 or r1:1-select, 0-not select\r\n");
            atci_gpio_send_response(&ret_len1, response);

            ret_len1 =  snprintf((char *)(response->response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "\r\nAT+EGPIO=GPIO_SET_OD:g,d  set output data to one pin\r\n g:gpio pin number\r\n d:0-low, 1-high\r\n");
            atci_gpio_send_response(&ret_len1, response);

            ret_len1 =  snprintf((char *)(response->response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "\r\nAT+EGPIO=GPIO_SET_DRV:g,drv  set drving current to one pin\r\n g:gpio pin number\r\n" CURRENT_STRING
                        );

            atci_gpio_send_response(&ret_len1, response);

            ret_len1 =  snprintf((char *)(response->response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "\r\nAT+EGPIO=GPIO_SET_ALL:g,m,dir,do,p,drv set several configuration to one pin\r\n g:gpio pin number\r\n dir:direction value, 1-ouput, 0-input\r\n do:0-low, 1-high\r\n p:pull state value, 0-pullup, 1-pulldown,2-disable pull\r\n"
                                 CURRENT_STRING
                        );
            atci_gpio_send_response(&ret_len1, response);

            break;
#endif
        case ATCI_CMD_MODE_EXECUTION:
#if (PRODUCT_VERSION == 7687 || PRODUCT_VERSION == 7697)
            strncpy((char *)response->response_buf, "Not support GPIO AT command", sizeof("Not support GPIO AT command"));
            response->response_len = strlen((char *)response->response_buf);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(response);
            break;
#else
            strncpy((char *)response->response_buf, "ATCI_CMD_MODE_EXECUTION", sizeof("ATCI_CMD_MODE_EXECUTION"));
            response->response_len = strlen((char *)response->response_buf);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(response);

            /*get specify gpio number state*/
            if (strncmp(parse_cmd->string_ptr, "AT+EGPIO=GPIO_GET:", strlen("AT+EGPIO=GPIO_GET:")) == 0) {

                param1 = strtok(parse_cmd->string_ptr, ":");
                param2 = strtok(NULL, "~");
                param3 = strtok(NULL, "~");

                pin_index[0] = (hal_gpio_pin_t)strtoul(param2, NULL, 10);
                pin_index[1] = (hal_gpio_pin_t)strtoul(param3, NULL, 10);


                if ((pin_index[1] < pin_index[0]) || (pin_index[1] >= HAL_GPIO_MAX)) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "GPIO get sate input error parameter, your input pin is %d~%d\r\n", pin_index[0], pin_index[1]);
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    break;
                }

                ret_len1 = 0;
                response->response_len = 0;

                for (i = pin_index[0]; i <= pin_index[1]; i++) {
                    gpio_get_state((hal_gpio_pin_t)i, &gpio_state);
                    ret_len1 = snprintf((char *)&response->response_buf[response->response_len], ATCI_UART_TX_FIFO_BUFFER_SIZE - response->response_len, \
                                        "GPIO%-2d, md=%d, %s, di=%d, do=%d, %dma, %s\r\n", \
                                        (int)i, (int)gpio_state.mode, (char *)direct[gpio_state.dir], (int)gpio_state.din, \
                                        (int)gpio_state.dout, current_state[(uint8_t)gpio_state.current_type], (char *)(pull_state[gpio_state.pull_type]));

                    /*check buffer if enough, send it*/
                    if ((ATCI_UART_TX_FIFO_BUFFER_SIZE - (response->response_len + ret_len1 + 1)) <= 0) {
                        response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                        atci_send_response(response);
                        response->response_len = 0;
                    }

                    response->response_len += (uint16_t)(ret_len1);
                }

                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);

                break;
            }

            param1 = strtok(parse_cmd->string_ptr, ":");
            param2 = strtok(NULL, ",");
            param3 = strtok(NULL, ",");

            pin_index[0]  = (hal_gpio_pin_t)atoi(param2);
            config_data[0] = (hal_gpio_pin_t)atoi(param3);

            if (pin_index[0] >= HAL_GPIO_MAX) {
                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                         "input error parameter, your input pin is %d, GPIO must be 0~%d\r\n", pin_index[0], HAL_GPIO_MAX - 1);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);
                break;
            }


            /*  set GPIO mode. */
            if (strncmp(param1, "AT+EGPIO=GPIO_SET_MODE", strlen("AT+EGPIO=GPIO_SET_MODE")) == 0) {

                hal_pinmux_set_function(pin_index[0], config_data[0]);

                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                         "SET GPIO%d to mode%d done!\r\n", (int)pin_index[0], (int)config_data[0]);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);

                break;
            }
            /*  set GPIO direction. */
            else if (strncmp(param1, "AT+EGPIO=GPIO_SET_DIR", strlen("AT+EGPIO=GPIO_SET_DIR")) == 0) {

                if (config_data[0] > 1) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "[error] inupt diretion is errror!\r\n 0:intput\r\n 1:output\r\n");
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    break;
                }

                hal_gpio_set_direction(pin_index[0], (hal_gpio_direction_t)config_data[0]);

                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                         "SET GPIO%d to %s done!\r\n", pin_index[0], direct[config_data[0]]);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);
                break;
            }

            /*  set GPIO pull state. */
            else if (strncmp(param1, "AT+EGPIO=GPIO_SET_PULL", strlen("AT+EGPIO=GPIO_SET_PULL")) == 0) {
                if (config_data[0] > 2) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "[error] inupt pull state is errror!\r\n 0:pull up\r\n 1:pull down\r\n 2:disable pull\r\n");
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    break;
                }

                if (config_data[0] == 0) {
                    hal_gpio_pull_up((hal_gpio_pin_t)pin_index[0]);
                } else if (config_data[0] == 1) {
                    hal_gpio_pull_down((hal_gpio_pin_t)pin_index[0]);
                } else {
                    hal_gpio_disable_pull((hal_gpio_pin_t)pin_index[0]);
                }

                gpio_get_state(pin_index[0], &gpio_state);

                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                         "SET GPIO%d to %s done!\r\n", pin_index[0], pull_state[gpio_state.pull_type]);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);
                break;
            }
#ifdef HAL_GPIO_FEATURE_PUPD
            /*  set GPIO pupd r0 r1 state. */
            else if (strncmp(param1, "AT+EGPIO=GPIO_SET_PUPD", strlen("AT+EGPIO=GPIO_SET_PUPD")) == 0) {

                param4 = strtok(NULL, ",");
                param5 = strtok(NULL, ",");

                config_data[1] = (uint32_t)atoi(param4);
                config_data[2] = (uint32_t)atoi(param5);

                if ((config_data[0] > 1) || (config_data[1] > 1) || (config_data[2] > 1)) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "[error] inupt pupd-r0-r1 is errror!\r\n 0-0-0/1-0-0: Hi-Z\r\n 0-0-1/0-1-0:pull-up 47k\r\n 0-1-1:pull-up 23.5k\r\n 1-0-1/1-1-0:pull-down 47k\r\n 1-1-1:pull-down 23.5K\r\n");
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    break;
                }

                ret_state = hal_gpio_set_pupd_register(pin_index[0], config_data[0], config_data[1], config_data[2]);
                if (ret_state == HAL_GPIO_STATUS_ERROR_PIN) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "[error]you input pin %d is not the pupd-r0-r1 pin, please refer GPIO datasheet to check\r\n", pin_index[0]);
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    break;
                }


                gpio_get_state(pin_index[0], &gpio_state);

                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                         "SET GPIO%d to %s done!\r\n", pin_index[0], pull_state[gpio_state.pull_type]);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);
                break;
            }
#endif
            /*  set GPIO output data. */
            else if (strncmp(param1, "AT+EGPIO=GPIO_SET_OD", strlen("AT+EGPIO=GPIO_SET_OD")) == 0) {
                if (config_data[0] > 1) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "[error] inupt output data is errror!\r\n 0:low\r\n 1:high\r\n");
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    break;
                }
                hal_gpio_set_output(pin_index[0], (hal_gpio_data_t)config_data[0]);
                gpio_get_state(pin_index[0], &gpio_state);

                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                         "SET GPIO%d output %d done!\r\n", (int)pin_index[0], (int)config_data[0]);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);

                break;
            }
            /*  set GPIO current driving. */
            else if (strncmp(param1, "AT+EGPIO=GPIO_SET_DRV", strlen("AT+EGPIO=GPIO_SET_DRV")) == 0) {
                if (config_data[0] >= sizeof(current_state) / sizeof(current_state[0])) {
                    const char *str5 = "[error] inupt driving type is errror!\r\n";
                    ret_len1 = strlen(str5);
                    strncpy((char *)response->response_buf, str5, ret_len1);
                    for (uint32_t i = 0; i < sizeof(current_state) / sizeof(current_state[0]); i++) {
                        ret_len1 += snprintf((char *)(response->response_buf + ret_len1), 16, "%d : %dma\r\n", (int)i, (int)current_state[i]);
                    }
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    break;
                }
                hal_gpio_set_driving_current(pin_index[0], (hal_gpio_driving_current_t)config_data[0]);
                gpio_get_state(pin_index[0], &gpio_state);

                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                         "SET GPIO%d current driving %dma done!\r\n", pin_index[0], current_state[gpio_state.current_type]);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);

                break;
            }
            /*  set GPIO RG. */
            else if (strncmp(param1, "AT+EGPIO=GPIO_SET_RG", strlen("AT+EGPIO=GPIO_SET_RG")) == 0) {

                config_data[0] = (uint32_t)strtoul((char *)param2, NULL, 16);
                config_data[1] = (uint32_t)strtoul((char *)param3, NULL, 16);

                *((uint32_t *)(config_data[0])) = config_data[1];

                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                         "SET GPIO adress:0x%x = 0x%x done!\r\n", (int)config_data[0], (int)config_data[1]);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);

                break;
            }
#ifdef HAL_GPIO_FEATURE_CLOCKOUT
            else if (strncmp(param1, "AT+EGPIO=GPIO_SET_CLKO", strlen("AT+EGPIO=GPIO_SET_CLKO")) == 0) {
                if (HAL_GPIO_STATUS_OK != hal_gpio_set_clockout(pin_index[0], (hal_gpio_clock_mode_t)config_data[0])) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "[error] gpio_clock_num(%d) out of range\r\n", (int)pin_index[0]);

                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    break;
                }

                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                         "SET CLKO%d=%d done!\r\n", (int)pin_index[0], (int)config_data[0]);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);
                break;
            }
#endif
            /* Set serveral configurations to one GPIO. */
            else if (strncmp(param1, "AT+EGPIO=GPIO_SET_ALL", strlen("AT+EGPIO=GPIO_SET_ALL")) == 0) {
                param4 = strtok(NULL, ",");
                param5 = strtok(NULL, ",");
                param6 = strtok(NULL, ",");
                param7 = strtok(NULL, ",");

                config_data[1] = (uint32_t)atoi(param4);  //direction
                config_data[2] = (uint32_t)atoi(param5);  //output data
                config_data[3] = (uint32_t)atoi(param6);  //pull state
                config_data[4] = (uint32_t)atoi(param7);  //current driving

                error_flag = 0;
                if (config_data[1] > 1) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "[error] inupt diretion is errror!\r\n 0:intput\r\n 1:output\r\n");
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    error_flag++;
                }

                if (config_data[2] > 1) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "[error] inupt output data is errror!\r\n 0:low\r\n 1:high\r\n");
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    error_flag++;
                }

                if (config_data[3] > 2) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "[error] inupt pull state is errror!\r\n 0:pull up\r\n 1:pull down\r\n 2:disable pull\r\n");
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    error_flag++;
                }

                if (config_data[4] > sizeof(current_state)) {
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "[error] inupt driving type is errror!\r\n"
                            CURRENT_STRING
                        );
                    response->response_len = strlen((char *)response->response_buf);
                    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(response);
                    error_flag++;
                }
                if (error_flag > 0) {
                    break;
                }

                hal_pinmux_set_function(pin_index[0], config_data[0]);
                hal_gpio_set_direction(pin_index[0], (hal_gpio_direction_t)config_data[1]);
                hal_gpio_set_output(pin_index[0], (hal_gpio_data_t)config_data[2]);

                if (config_data[3] == 0) {
                    hal_gpio_pull_up(pin_index[0]);
                } else if (config_data[3] == 1) {
                    hal_gpio_pull_down(pin_index[0]);
                } else {
                    hal_gpio_disable_pull(pin_index[0]);
                }

                hal_gpio_set_driving_current(pin_index[0], (hal_gpio_driving_current_t)config_data[4]);

                gpio_get_state(pin_index[0], &gpio_state);                          //get specify gpio current configuration

                response->response_len = snprintf((char *)(response->response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                                "set GPIO%-2d, md=%d, %s, di=%d, do=%d, %dma, %s done!\r\n", \
                                                (int)pin_index[0], (int)gpio_state.mode, (char *)direct[gpio_state.dir], (int)gpio_state.din, (int)gpio_state.dout, \
                                                current_state[(uint8_t)gpio_state.current_type], (char *)(pull_state[gpio_state.pull_type]));
                response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);

                break;
            }
#endif
#ifdef AG3335
            else if (strncmp(param1, "AT+EGPIO=FT", strlen("AT+EGPIO=FT")) == 0) {
                hal_gpio_data_t gpio_in_data = HAL_GPIO_DATA_LOW;
                uint32_t test_fail = 0;
                /*GPIO0<-->GPIO5*/
                hal_pinmux_set_function(HAL_GPIO_0, HAL_GPIO_0_GPIO0);
                hal_gpio_set_direction(HAL_GPIO_0, HAL_GPIO_DIRECTION_OUTPUT);
                hal_pinmux_set_function(HAL_GPIO_5, HAL_GPIO_5_GPIO5);
                hal_gpio_set_direction(HAL_GPIO_5, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_set_output(HAL_GPIO_0, HAL_GPIO_DATA_HIGH);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_5, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_HIGH) {
                    strncpy((char *)response->response_buf, "GPIO_0 and GPIO5 Output High Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_0 and GPIO5 Output High Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_0 and GPIO5 Output High Fail, AT+GPIO_FT FAIL\r\n",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }
                hal_gpio_set_output(HAL_GPIO_0, HAL_GPIO_DATA_LOW);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_5, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_LOW) {
                    strncpy((char *)response->response_buf, "GPIO_0 and GPIO5 Output Low Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_0 and GPIO5 Output Low Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_0 and GPIO5 Output Low Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }

                /*GPIO2<-->GPIO3*/
                hal_pinmux_set_function(HAL_GPIO_2, HAL_GPIO_2_GPIO2);
                hal_gpio_set_direction(HAL_GPIO_2, HAL_GPIO_DIRECTION_OUTPUT);
                hal_pinmux_set_function(HAL_GPIO_3, HAL_GPIO_3_GPIO3);
                hal_gpio_set_direction(HAL_GPIO_3, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_set_output(HAL_GPIO_2, HAL_GPIO_DATA_HIGH);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_3, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_HIGH) {
                    strncpy((char *)response->response_buf, "GPIO_2 and GPIO3 Output High Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_2 and GPIO3 Output High Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_2 and GPIO3 Output High Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }
                hal_gpio_set_output(HAL_GPIO_2, HAL_GPIO_DATA_LOW);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_3, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_LOW) {
                    strncpy((char *)response->response_buf, "GPIO_2 and GPIO3 Output Low Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_2 and GPIO3 Output Low Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_2 and GPIO3 Output Low Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }

                /*GPIO20<-->GPIO21*/
                hal_pinmux_set_function(HAL_GPIO_20, HAL_GPIO_20_GPIO20);
                hal_gpio_set_direction(HAL_GPIO_20, HAL_GPIO_DIRECTION_OUTPUT);
                hal_pinmux_set_function(HAL_GPIO_21, HAL_GPIO_21_GPIO21);
                hal_gpio_set_direction(HAL_GPIO_21, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_set_output(HAL_GPIO_20, HAL_GPIO_DATA_HIGH);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_21, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_HIGH) {
                    strncpy((char *)response->response_buf, "GPIO_20 and GPIO21 Output High Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_20 and GPIO21 Output High Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_20 and GPIO21 Output High Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }
                hal_gpio_set_output(HAL_GPIO_20, HAL_GPIO_DATA_LOW);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_21, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_LOW) {
                    strncpy((char *)response->response_buf, "GPIO_20 and GPIO21 Output Low Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_20 and GPIO21 Output Low Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_20 and GPIO21 Output Low Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }

                /*GPIO13<-->GPIO14*/
                hal_pinmux_set_function(HAL_GPIO_13, HAL_GPIO_13_GPIO13);
                hal_gpio_set_direction(HAL_GPIO_13, HAL_GPIO_DIRECTION_OUTPUT);
                hal_pinmux_set_function(HAL_GPIO_14, HAL_GPIO_14_GPIO14);
                hal_gpio_set_direction(HAL_GPIO_14, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_set_output(HAL_GPIO_13, HAL_GPIO_DATA_HIGH);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_14, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_HIGH) {
                    strncpy((char *)response->response_buf, "GPIO_13 and GPIO14 Output High Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_13 and GPIO14 Output High Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_13 and GPIO14 Output High Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }
                hal_gpio_set_output(HAL_GPIO_13, HAL_GPIO_DATA_LOW);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_14, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_LOW) {
                    strncpy((char *)response->response_buf, "GPIO_13 and GPIO14 Output Low Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_13 and GPIO14 Output Low Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_13 and GPIO14 Output Low Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }

                /*GPIO10<-->GPIO4*/
                hal_pinmux_set_function(HAL_GPIO_10, HAL_GPIO_10_GPIO10);
                hal_gpio_set_direction(HAL_GPIO_10, HAL_GPIO_DIRECTION_OUTPUT);
                hal_pinmux_set_function(HAL_GPIO_4, HAL_GPIO_11_GPIO11);
                hal_gpio_set_direction(HAL_GPIO_4, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_set_output(HAL_GPIO_10, HAL_GPIO_DATA_HIGH);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_4, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_HIGH) {
                    strncpy((char *)response->response_buf, "GPIO_10 and GPIO4 Output High Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_10 and GPIO4 Output High Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_10 and GPIO4 Output High Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }
                hal_gpio_set_output(HAL_GPIO_10, HAL_GPIO_DATA_LOW);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_4, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_LOW) {
                    strncpy((char *)response->response_buf, "GPIO_10 and GPIO4 Output Low Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_10 and GPIO4 Output Low Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_10 and GPIO4 Output Low Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }

                /*GPIO1<-->GPIO11*/
                hal_pinmux_set_function(HAL_GPIO_1, HAL_GPIO_1_GPIO1);
                hal_gpio_set_direction(HAL_GPIO_1, HAL_GPIO_DIRECTION_OUTPUT);
                hal_pinmux_set_function(HAL_GPIO_11, HAL_GPIO_4_GPIO4);
                hal_gpio_set_direction(HAL_GPIO_11, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_set_output(HAL_GPIO_1, HAL_GPIO_DATA_HIGH);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_11, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_HIGH) {
                    strncpy((char *)response->response_buf, "GPIO_1 and GPIO11 Output High Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_1 and GPIO11 Output High Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_1 and GPIO11 Output High Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }
                hal_gpio_set_output(HAL_GPIO_1, HAL_GPIO_DATA_LOW);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_11, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_LOW) {
                    strncpy((char *)response->response_buf, "GPIO_1 and GPIO11 Output Low Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_1 and GPIO11 Output Low Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_1 and GPIO11 Output Low Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }

                /*GPIO22 check high*/
                hal_pinmux_set_function(HAL_GPIO_22, HAL_GPIO_22_GPIO22);
                hal_gpio_set_direction(HAL_GPIO_22, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_pull_down(HAL_GPIO_22);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_22, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_HIGH) {
                    strncpy((char *)response->response_buf, "GPIO_22 Read High Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_22 Read High Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_22 Read High Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }

                /*GPIO15 check high*/
                hal_pinmux_set_function(HAL_GPIO_15, HAL_GPIO_15_GPIO15);
                hal_gpio_set_direction(HAL_GPIO_15, HAL_GPIO_DIRECTION_INPUT);
                hal_gpio_pull_down(HAL_GPIO_15);
                hal_gpt_delay_ms(10);
                hal_gpio_get_input(HAL_GPIO_15, &gpio_in_data);
                if (gpio_in_data != HAL_GPIO_DATA_HIGH) {
                    strncpy((char *)response->response_buf, "GPIO_15 Read High Fail, AT+GPIO_FT FAIL\r\n", strlen("GPIO_15 Read High Fail, AT+GPIO_FT FAIL\r\n"));
                    LOGMSGIDE("GPIO_15 Read High Fail, AT+GPIO_FT FAIL",0);
                    test_fail++;
                    goto gpio_ft_fail;
                }

gpio_ft_fail:
                if (test_fail == 0) {
                    strncpy((char *)response->response_buf, "AT+GPIO_FT PASS\r\n", strlen("AT+GPIO_FT PASS\r\n\r\n"));
                    LOGMSGIDI("AT+GPIO_FT PASS",0);
                }
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = (test_fail == 0) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
                atci_send_response(response);
                break;

            }

            else if (strncmp(param1, "AT+EGPIO=PPS", strlen("AT+EGPIO=PPS")) == 0) {
                uint32_t wait_count = 10;
                uint32_t test_fail = 1;
                hal_eint_config_t eint_config;
                /*PPS0 output test*/

                hal_eint_mask(HAL_EINT_NUMBER_4);
                hal_pinmux_set_function(HAL_GPIO_4, HAL_GPIO_4_EINT4);
                hal_gpio_pull_down(HAL_GPIO_4);
                eint_config.trigger_mode = HAL_EINT_EDGE_RISING;
                eint_config.debounce_time = 1;
                if (HAL_EINT_STATUS_OK !=  hal_eint_init(HAL_EINT_NUMBER_4, &eint_config)) {
                    test_fail = 1;
                    strncpy((char *)response->response_buf, "AT+GPIO_PPS FAIL\r\n", strlen("AT+GPIO_PPS FAIL\r\n"));
                    LOGMSGIDE("AT+GPIO_PPS FAIL, hal_eint_init fail\r\n",0);
                    goto gpio_pps_test_fail;
                }
                if (HAL_EINT_STATUS_OK !=  hal_eint_register_callback(HAL_EINT_NUMBER_4, pps_test_callback, NULL)) {
                    test_fail = 1;
                    strncpy((char *)response->response_buf, "AT+GPIO_PPS FAIL\r\n", strlen("AT+GPIO_PPS FAIL\r\n"));
                    LOGMSGIDE("AT+GPIO_PPS FAIL, hal_eint_register_callback fail\r\n",0);
                    goto gpio_pps_test_fail;
                }
                hal_eint_unmask(HAL_EINT_NUMBER_4);

                hal_gpio_set_direction(HAL_GPIO_10, HAL_GPIO_10_PPS0);

                while (wait_count--) {
                    hal_gpt_delay_ms(100);
                    if (pps_output_plus_status) {
                        test_fail = 0;
                        pps_output_plus_status = 0;
                        break;
                    }
                }
gpio_pps_test_fail:
                if (test_fail == 0) {
                    strncpy((char *)response->response_buf, "AT+GPIO_PPS PASS\r\n", strlen("AT+GPIO_PPS PASS\r\n\r\n"));
                    LOGMSGIDI("AT+GPIO_PPS PASS",0);
                }
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = (test_fail == 0) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
                atci_send_response(response);
                break;
            }
#endif
        default :
            /* others are invalid command format */
            strncpy((char *)response->response_buf, "Wrong command ERROR\r\n", sizeof("Wrong command ERROR\r\n"));
            response->response_len = strlen((char *)response->response_buf);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(response);
            break;
    }

    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
             "/*****************next command line*********************\r\n");
    response->response_len = strlen((char *)response->response_buf);
    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    atci_send_response(response);
    atci_mem_free(response);
    return ATCI_STATUS_OK;
}

#endif

