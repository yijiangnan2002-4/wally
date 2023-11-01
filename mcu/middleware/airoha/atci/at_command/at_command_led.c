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
#include <string.h>
#include <stdio.h>
#include "at_command.h"
#include "atci_adapter.h"
#include "hal_gpio.h"
#include "hal_isink.h"
#include <stdlib.h>
#include "bsp_led.h"


#if defined(HAL_ISINK_MODULE_ENABLED) && defined(AIR_HQA_TEST_ENABLED)
/*
 * sample code
*/


/*--- Function ---*/
atci_status_t atci_cmd_hdlr_led(atci_parse_cmd_param_t *parse_cmd);


/*LED AT CMD Usage:
Format:
    //test hal isink interface
    AT+ELED=HAL,<channel>,<tr>,<ton>,<tf>,<toff>,<brightness>
    //test bsp led interface
    AT+ELED=BSP,<channel>,<mode>,<t0>,<t1>,<t2>,<t3>,<t1t2_rpt>,<brightness>
Example:
    //test hal isink interface
    AT+ELED=HAL,0,150,600,300,600,100
    //test bsp led interface
    AT+ELED=BSP,0,0,0,1000,1000,670,2,200
*/

#define     ELED_HELP       "AT+ELED=HAL,<channel>,<tr>,<ton>,<tf>,<toff>,<brightness>\r\n AT+ELED=BSP,<channel>,<mode>,<t0>,<t1>,<t2>,<t3>,<>t1t2_rpt,<brightness>\r\n"
#define     ELED_INV_PARA   "+ELED:Invalid Para\r\n"
#define     ELED_OK         "+ELED:Succ\r\n"



#if defined(HAL_ISINK_MODULE_ENABLED) && defined(HAL_ISINK_FEATURE_ADVANCED_CONFIG) && defined(HAL_ISINK_FEATURE_HW_PMIC2562)
void    atci_led_handle_isink_module(char *cmd_str, atci_response_t *response)
{
    char        *mid_pos = NULL;
    int channel, tr, tf, ton, toff, bright;
    hal_isink_config_ext_t isink_cfg;
    /*AT+ELED=HAL,<channel>,<tr>,<ton>,<tf>,<toff>,<brightness>*/
    if (response == NULL) {
        return;
    }
    mid_pos = strchr(cmd_str, ',');
    mid_pos++;
    channel = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    tr = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    ton = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    tf = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    toff = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    bright = atoi(mid_pos);

    hal_isink_stop(channel);
    hal_isink_deinit(channel);

    isink_cfg.timing.t0 = 0;
    isink_cfg.timing.t1.t_rising   = tr;
    isink_cfg.timing.t1.t_lightest = ton;
    isink_cfg.timing.t2.t_falling  = tf;
    isink_cfg.timing.t2.t_darkest  = toff;
    isink_cfg.timing.t3 = 0;
    isink_cfg.blink_nums = 0;
    isink_cfg.brightness = bright;
    hal_isink_init(channel);
    hal_isink_configure_ext(channel, &isink_cfg);
    hal_isink_start(channel);
    snprintf((char *)response->response_buf, 256, "AT+ISINK%d:TR(%d),TON(%d),TF(%d),TOFF(%d),BRIGHT(%d)\r\n+OK\r\n", channel, tr, ton, tf, toff, bright);
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
    //hal_isink_dump_register();
}
#endif


void    atci_led_handle_bsp_led_module(char *cmd_str, atci_response_t *response)
{
    char        *mid_pos = NULL;
    int mode, channel, t0, t1, t2, t3, t1t2_rpt, bright;
    bsp_led_config_t    led_cfg;
    /*AT+ELED=<channel>,<mode>,<t0>,<t1>,<t2>,<t3>,<>t1t2_rpt,<brightness>*/
    if (response == NULL) {
        return;
    }

    mid_pos = strchr(cmd_str, ',');
    mid_pos++;
    channel = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    mode = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    t0 = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    t1 = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    t2 = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    t3 = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    t1t2_rpt = atoi(mid_pos);

    mid_pos = strchr(mid_pos, ',');
    mid_pos++;
    bright = atoi(mid_pos);

    bsp_led_stop(channel);
    bsp_led_deinit(channel);

    memset(&led_cfg, 0, sizeof(led_cfg));
    led_cfg.onoff = 1;      //enable led on
    led_cfg.time_unit = 100; // time unit for t0/t1/t2/t3(unit:ms)
    led_cfg.t0 = t0 / 100;         //delay start time
    led_cfg.t1 = t1 / 100;        //led on time in one cycle
    led_cfg.t2 = t2 / 100;        //led off time in one cycle
    led_cfg.repeat_t1t2 = t1t2_rpt;//led repeat t0t1 times. if equal 0, led always repeat t1t2
    led_cfg.t3 = t3 / 100;       //led off time after finish t1t2 repeat
    led_cfg.repeat_ext  = 0;//ext loop times.if equal 0, led always repeat (t1t2_repeat + t3)
    led_cfg.sync_time   = 0;
    led_cfg.brightness  = bright;//brightness(0~255)

    bsp_led_init(channel, mode ? 1 : 0);
    bsp_led_config(channel, &led_cfg);
    bsp_led_start(channel);

    snprintf((char *)response->response_buf, 256, "+LED%d:T0(%d),T1(%d),T2(%d),T3(%d),RPT_T1T2(%d),BRIGHT(%d)\r\n+OK\r\n", channel, t0, t1, t2, t3, t1t2_rpt, bright);
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
    //hal_isink_dump_register();
}



inline void    _atci_led_handle_test(char *cmd_string, atci_response_t *response)
{
    uint32_t len;

    len = strlen(ELED_HELP);
    if (len > ATCI_UART_TX_FIFO_BUFFER_SIZE) {
        len = ATCI_UART_TX_FIFO_BUFFER_SIZE;
    }
    strncpy((char *)response->response_buf, ELED_HELP, len);
    response->response_len   = strlen((char *)response->response_buf);
    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
}


/* AT command handler  */
atci_status_t atci_cmd_hdlr_led(atci_parse_cmd_param_t *parse_cmd)
{
    atci_status_t   status = ATCI_STATUS_OK;
    atci_response_t *response;

    response = atci_mem_alloc(sizeof(atci_response_t));
    if(response == NULL) {
        return ATCI_STATUS_ERROR;
    }
    response->response_flag = 0; /*    Command Execute Finish.  */
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING: {   /* rec: AT+ELED=?   */
            _atci_led_handle_test(parse_cmd->string_ptr, response);
        }
        break;

        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+ELED=<op>  the handler need to parse the parameters  */
            if (strncmp(parse_cmd->string_ptr, "AT+ELED=HAL", strlen("AT+ELED=HAL")) == 0) {
#if defined(HAL_ISINK_MODULE_ENABLED) && defined(HAL_ISINK_FEATURE_ADVANCED_CONFIG) && defined(HAL_ISINK_FEATURE_HW_PMIC2562)
                atci_led_handle_isink_module(parse_cmd->string_ptr, response);
#else
                const char err_str[] = "error:none led hw";

                strncpy((char *)response->response_buf, err_str, sizeof(err_str));
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
            } else if (strncmp(parse_cmd->string_ptr, "AT+ELED=BSP", strlen("AT+ELED=BSP")) == 0) {
                atci_led_handle_bsp_led_module(parse_cmd->string_ptr, response);
            }
            break;
        default :
            /* others are invalid command format */
            response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    atci_send_response(response);
    atci_mem_free(response);
    return status;
}

#endif
