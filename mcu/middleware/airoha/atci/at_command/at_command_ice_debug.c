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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "task.h"

#include "at_command.h"
#include "syslog.h"
#include "task_def.h"
#include "os_port_callback.h"
#include "hal_wdt.h"
#include "hal_gpt.h"
#ifdef AIR_ICE_DEBUG_ENABLE
#include "hal_sleep_manager_platform.h"
#include "hal_ccni.h"
#include "hal_ccni_config.h"
#include "memory_attribute.h"
#include "hal_gpio_internal.h"
#include "hal_gpio.h"
#include "hal_platform.h"
#include "hal_spm.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_resource_assignment.h"
#include "hal_time_check.h"
#include "hal_trng.h"

#define LOGE(fmt,arg...)   LOG_E(atcmd, "ICE_DEUG: "fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(atcmd, "ICE_DEUG: "fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(atcmd ,"ICE_DEUG: "fmt,##arg)
#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atcmd, "ICE_DEUG: "fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atcmd ,"ICE_DEUG: "fmt,cnt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atcmd ,"ICE_DEUG: "fmt,cnt,##arg)

/*--- Function ---*/

extern atci_status_t atci_cmd_hdlr_debug(atci_parse_cmd_param_t *parse_cmd);


/*
AT+DEBUG=?
+DEBUG:
AT+DEBUG=<op>
<op> string type
enable: enable DEBUG
disable: disable DEBUG
*/

/* <pre>
* <b>[Category]</b>
*    System Service
* <b>[Description]</b>
*    Enable or disable ICE debug
* <b>[Command]</b>
*    AT+DEBUG=<op>
* <b>[Parameter]</b>
*    <op> string type, must be 'enable' or 'disable'
* <b>[Response]</b>
*    OK or ERROR;
* <b>[Example]</b>
* @code
*    Send:
*        AT+DEBUG= enable
*    Response:
*        OK
* @endcode
* <b>[Note]</b>
*    AIR_ICE_DEBUG_ENABLE should be defined in y in project's feature.mk
* </pre>
*/


/* ice debug configuration area */
typedef union {
    uint32_t ice_debug_mode;
    struct {
        uint32_t irq_time_check_disable  : 1;
        uint32_t system_hang_disable     : 1;
        uint32_t dsp_enter_idle_disable  : 1;
        uint32_t magic_number : 29;
    } ice_debug_mode_t;
} ice_debug_config_mode_t;

static volatile ice_debug_config_mode_t *ice_debug_para = (volatile ice_debug_config_mode_t *)HW_SYSRAM_PRIVATE_MEMORY_DSP_ICE_DEBUG_START;

static void stringToLower(char *pString)
{
    uint16_t index;
    uint16_t length = (uint16_t)strlen((char *)pString);

    for (index = 0; index <= length; index++) {
        pString[index] = (char)tolower((unsigned char)pString[index]);
    }
}

static void ice_debug_wdt_set(void)
{
    hal_wdt_config_t wdt_config;
    uint32_t random_seed = 0;
    hal_trng_status_t status;

    wdt_config.mode = HAL_WDT_MODE_INTERRUPT;
    status = hal_trng_get_generated_random_number(&random_seed);
    if (status != HAL_TRNG_STATUS_OK) {
        random_seed = 3;
    }
    wdt_config.seconds  = random_seed & 0x07;
    hal_wdt_init_ext(&wdt_config);
    hal_wdt_enable_ext(HAL_WDT_ENABLE_MAGIC);
}


static void debug_show_usage(uint8_t *buf)
{
    int32_t pos = 0;

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+DEBUG:\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+DEBUG=<op>)\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(<op>=[enable|disable|decouple_test_mcu|decouple_test_dsp|stop_sync_test_mcu|stop_sync_test_mcu])\r\n");
}


/* AT command handler  */
extern bool log_set_all_module_filter_off(log_switch_t log_switch);
atci_status_t atci_cmd_hdlr_debug(atci_parse_cmd_param_t *parse_cmd)
{
    hal_ccni_status_t ret;
    hal_ccni_message_t msg;
    char *param = NULL, *saveptr = NULL;

    atci_response_t *presponse = pvPortMalloc(sizeof(atci_response_t));
    if (presponse == NULL) {
        LOGMSGIDE("memory malloced failed.\r\n", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(presponse, 0, sizeof(atci_response_t));

#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    presponse.cmd_id = parse_cmd->cmd_id;
#endif /* ATCI_APB_PROXY_ADAPTER_ENABLE */

    /* To support both of uppercase and lowercase */
    stringToLower(parse_cmd->string_ptr);

    presponse->response_flag = 0; /* Command Execute Finish. */
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:/* rec: AT+DEBUG=? */
            debug_show_usage(presponse->response_buf);
            presponse->response_len = strlen((char *)(presponse->response_buf));
            presponse->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(presponse);
            break;
        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+DEBUG=<module> */
            param = parse_cmd->string_ptr + parse_cmd->parse_pos;
            param = strtok_r(param, "\n\r", &saveptr);

            if (strncmp(param, "enable", strlen("enable")) == 0) {
                hal_wdt_config_t wdt_config;
                msg.ccni_message[1] = 3;

                LOGMSGIDW("ice debug mode enabled.\r\n", 0);

                if (strncmp(param, "enable_decouple_mode", strlen("enable_decouple_mode")) == 0) {
                    ice_debug_para->ice_debug_mode = 0xAAff;
                } else if (strncmp(param, "enable_auto_hold_mode", strlen("enable_auto_hold_mode")) == 0) {
                    ice_debug_para->ice_debug_mode = 0xBBff;
                } else {
                    ice_debug_para->ice_debug_mode = 0xff;
                }

                wdt_config.mode = HAL_WDT_MODE_INTERRUPT;
                wdt_config.seconds = 30;
                hal_wdt_init_ext(&wdt_config);  /*WDT1 is default Reset mode, change to NMI mode*/

                ret = hal_ccni_set_event(CCNI_CM4_TO_DSP0_ICE_DEBUG, &msg);
                assert(HAL_CCNI_STATUS_OK == ret);

                hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);           /* Turn off NMI caused by 60S without context switch  */

                snprintf((char *)presponse->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "JTAG: %s\r\n", param);
                presponse->response_len = strlen((char *)presponse->response_buf);
                presponse->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(presponse);
                presponse->response_len = 0;
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (strncmp(param, "disable", strlen("disable")) == 0) {
                ice_debug_para->ice_debug_mode = 0x0;
                hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);  /* enable wdt, wdt count will be reset*/
                LOGMSGIDW("ice debug mode disabled.\r\n", 0);

                presponse->response_len = 0;
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (strncmp(param, "decouple_test_mcu", strlen("decouple_test_mcu")) == 0) {
                uint32_t stop_time;
                char *param1 = NULL;

                param1 = strtok(parse_cmd->string_ptr, ",");
                param1 = strtok(NULL, ",");
                stop_time  = atoi(param1);

                /*set flag*/
                ice_debug_para->ice_debug_mode = 0xA1FF | ((stop_time & 0xffff) << 16);

                ice_debug_wdt_set();

                presponse->response_len = 0;
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (strncmp(param, "decouple_test_dsp", strlen("decouple_test_dsp")) == 0) {
                char *param1 = NULL;
                uint32_t stop_time;

                param1 = strtok(parse_cmd->string_ptr, ",");
                param1 = strtok(NULL, ",");
                stop_time  = atoi(param1);

                /*set flag*/
                ice_debug_para->ice_debug_mode = 0xA2FF | ((stop_time & 0xffff) << 16);

                /* dsp test   */
                ice_debug_wdt_set();

                presponse->response_len = 0;
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (strncmp(param, "stop_sync_test_mcu", strlen("stop_sync_test_mcu")) == 0) {
                uint32_t stop_time;
                char *param1 = NULL;

                param1 = strtok(parse_cmd->string_ptr, ",");
                param1 = strtok(NULL, ",");
                stop_time  = atoi(param1);

                /*set flag*/
                ice_debug_para->ice_debug_mode = 0xA3FF | ((stop_time & 0xffff) << 16);
                ice_debug_wdt_set();

                presponse->response_len = 0;
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (strncmp(param, "stop_sync_test_dsp", strlen("stop_sync_test_dsp")) == 0) {
                char *param1 = NULL;
                uint32_t stop_time;

                param1 = strtok(parse_cmd->string_ptr, ",");
                param1 = strtok(NULL, ",");
                stop_time  = atoi(param1);

                /*set flag*/
                ice_debug_para->ice_debug_mode = 0xA4FF | ((stop_time & 0xffff) << 16);
                ice_debug_wdt_set();

                presponse->response_len = 0;
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                /* command syntax error */
                strncpy((char *)(presponse->response_buf),
                        "+SYSTEM:\r\ncommand syntax error\r\n",
                        (int32_t)ATCI_UART_TX_FIFO_BUFFER_SIZE);
                presponse->response_len = strlen((char *)(presponse->response_buf));
                presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                atci_send_response(presponse);
            }
            atci_send_response(presponse);
            break;
        default :
            /* others are invalid command format */
            presponse->response_len = strlen((char *)(presponse->response_buf));
            presponse->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(presponse);
            break;

    }

    vPortFree(presponse);
    return ATCI_STATUS_OK;

}
#endif /* AIR_ICE_DEBUG_ENABLE*/

