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
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "at_command.h"
#include "hal_gpt.h"
#include "syslog.h"
#include "hal_pwm.h"


#if defined(HAL_PWM_MODULE_ENABLED) && defined(AIR_HQA_TEST_ENABLED)
/*--- Function ---*/
atci_status_t atci_cmd_hdlr_pwm(atci_parse_cmd_param_t *parse_cmd);

/*
AT+EPWM=<pwmnum,pwmclocksource,pwmoutfrequency,pwmduty>         |   "OK"
AT+EPWM=?                                                       |   "+EPWM="OK"

*/
// AT command handler
atci_status_t atci_cmd_hdlr_pwm(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = atci_mem_alloc(sizeof(atci_response_t));
    if (NULL == response) {
        return ATCI_STATUS_ERROR;
    }

    char            cmd[256] = {0};
    uint8_t         i        = 0;
    char           *ptr      = NULL;
    uint32_t        para[4]  = {0};
    uint32_t        temp     = 0;
    bool            error    = 1;
    char            *str_clk = NULL;
    hal_pwm_source_clock_t  clock;
    hal_pwm_status_t        status;

    bool            arg_valid  = true;
    bool            para_valid = true;

    //memset(response, 0, sizeof(atci_response_t));
    strncpy(cmd, (char *)parse_cmd->string_ptr, sizeof(cmd) - 1);
    for (i = 0; i < strlen((char *)parse_cmd->string_ptr); i++) {
        cmd[i] = (char)toupper((unsigned char)cmd[i]);
    }
    response->response_flag = 0; // Command Execute Finish.
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response->cmd_id = parse_cmd->cmd_id;
#endif

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    /* rec: AT+EPWM=?   */
            strncpy((char *)response->response_buf, "+EPWM=\"1,on\"\r\nOK\r\n", strlen("+EPWM=\"1,on\"\r\nOK\r\n")+1);
            error = 0;
            break;
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+EPWM=<op>  the handler need to parse the parameters
            if (strncmp(cmd, "AT+EPWM=SET,", strlen("AT+EPWM=SET,")) == 0) {
                /*command: AT+EPWM=SET,1,13000000,1000,50*/
                ptr = strtok(parse_cmd->string_ptr, ",");
                for (i = 0; i < 4; i++) {
                    ptr = strtok(NULL, ",");
                    if (ptr != NULL) {
                        para[i] = atoi(ptr);
                    } else {
                        arg_valid = false;
                        break;
                    }
                }
                /*para[0]-> channel;para[1]->source clock; para[2]->frequency; para[3]->duty*/
                if (arg_valid == true) {
                    if (para[3] > 100 && para[1] < para[2]) {
                        para_valid = 0;
                    }

                    switch (para[1]) {
#ifdef HAL_PWM_CLOCK_32K_SUPPORTED
                        case 32000:
                            clock = HAL_PWM_CLOCK_32KHZ;
                            str_clk = "32K";
                            break;
#endif

#ifdef HAL_PWM_CLOCK_13M_SUPPORTED
                        case 13000000:
                            clock = HAL_PWM_CLOCK_13MHZ;
                            str_clk =  "13M";
                            break;
#endif

#ifdef HAL_PWM_CLOCK_26M_SUPPORTED
                        case 26000000:
                            clock = HAL_PWM_CLOCK_26MHZ;
                            str_clk =  "26M";
                            break;
#endif

#ifdef HAL_PWM_CLOCK_39M_SUPPORTED
                        case 39000000:
                            clock = HAL_PWM_CLOCK_39MHZ;
                            str_clk =  "39M";
                            break;
#endif

#ifdef HAL_PWM_CLOCK_40M_SUPPORTED
                        case 40000000:
                            clock = HAL_PWM_CLOCK_40MHZ;
                            str_clk = "40M";
                            break;
#endif

#ifdef HAL_PWM_CLOCK_41M_SUPPORTED
                        case 41000000:
                            clock = HAL_PWM_CLOCK_41MHZ;
                            str_clk = "41M";
                            break;
#endif

#ifdef HAL_PWM_CLOCK_48M_SUPPORTED
                        case 48000000:
                            clock = HAL_PWM_CLOCK_48MHZ;
                            str_clk = "48M";
                            break;
#endif

#ifdef HAL_PWM_CLOCK_86M_SUPPORTED
                        case 86000000:
                            clock = HAL_PWM_CLOCK_86MHZ;
                            str_clk = "86M";
                            break;
#endif

                        default:
                            para_valid = 0;
                            break;
                    }
                    if (para_valid) {
                        hal_pwm_deinit((hal_pwm_channel_t)para[0]);
                        status = hal_pwm_init((hal_pwm_channel_t)para[0], clock);
                        status |= hal_pwm_set_frequency((hal_pwm_channel_t)para[0], para[2], &temp);
                        temp = temp * para[3] / 100;
                        status |= hal_pwm_set_duty_cycle((hal_pwm_channel_t)para[0], temp);
                        status |= hal_pwm_start((hal_pwm_channel_t)para[0]);
                        if (status == HAL_PWM_STATUS_OK) {
                            error = 0;
                            snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Ch:%d,Clk:%s,Freq:%d,Duty=%d \r\nOK\r\n", (int)para[0], str_clk, (int)para[2], (int)para[3]);
                        } else {
                            para_valid = 0;
                        }
                    }
                }
            }
            break;
        default:
            /* others are invalid command format */
            error  = 1;
            break;
    }
    if (arg_valid == 0) {
        strncpy((char *)response->response_buf, "argument error\r\n", strlen("argument error\r\n")+1);
    }
    if (para_valid == 0) {
        strncpy((char *)response->response_buf, "parameter invalid\r\n", strlen("parameter invalid\r\n")+1);
    }
    if (error == 1) {
        strncat((char *)response->response_buf, "ERROR\r\n", strlen("ERROR\r\n")+1);
    }
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    atci_mem_free(response);
    return ATCI_STATUS_OK;
}

#endif

