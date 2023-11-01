/* Copyright Statement:
 *
 * (C) 2022 Airoha Technology Corp. All rights reserved.
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
#include <malloc.h>

#include "FreeRTOS.h"
#include "task.h"

#include "at_command.h"
#include "syslog.h"


#if defined(HAL_UART_MODULE_ENABLED) && defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_HQA_TEST_ENABLED)

#include "hal_gpt.h"
#include "hal_uart.h"
#include "hal_uart_internal.h"
#include "hal_wdt.h"
#include "hal_platform.h"

#define UART_VFIFO_SIZE  4096
#define UART_USER_BUFFER_SIZE  2048
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t g_rx_vfifo_buffer[UART_VFIFO_SIZE];
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t g_tx_vfifo_buffer[UART_VFIFO_SIZE];

uint8_t rcv_buffer[UART_USER_BUFFER_SIZE] = {0x55};
uint8_t send_buffer[UART_USER_BUFFER_SIZE] = {0x55};

uint32_t uart_port;


/*--- Function ---*/
atci_status_t atci_cmd_hdlr_uart(atci_parse_cmd_param_t *parse_cmd);

void at_cmd_uart_dma_callback(hal_uart_callback_event_t status, void *user_data)
{

    if (status == HAL_UART_EVENT_READY_TO_WRITE) {
        LOG_MSGID_I(common, "irq occured: ready to write!!", 0);
    } else if (status == HAL_UART_EVENT_READY_TO_READ) {
        LOG_MSGID_I(common, "irq occured: ready to read!!", 0);
        hal_uart_receive_dma(uart_port, rcv_buffer, 512);
    } else if (status == HAL_UART_EVENT_TRANSMISSION_DONE) {
        LOG_MSGID_I(common, "irq occured: TRANSMISSION_DONE!!", 0);
        hal_uart_send_dma(uart_port, send_buffer, 512);
    }

}

/*
AT+EUART=<uart port>,<baudrate>
*/
extern UART_REGISTER_T *const       g_uart_regbase[HAL_UART_MAX];
atci_status_t atci_cmd_hdlr_uart(atci_parse_cmd_param_t *parse_cmd)
{
    char *param = NULL;
    uint32_t  param_val[10] = {0, 0, 0, 0, 0};
    uint32_t i = 0;
    uint32_t j = 0;

    atci_response_t *response  = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    memset(response, 0, sizeof(atci_response_t));

    hal_uart_config_t uart_config;
    hal_uart_dma_config_t dma_config;

    uint32_t actual_baudrate;
    UART_REGISTER_T *uartx;

    LOG_MSGID_I(common, "atci_cmd_hdlr_uart \r\n", 0);

#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response.cmd_id = parse_cmd->cmd_id;
#endif

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:   /* rec: AT+EUART=?   */
            strncpy((char *)response->response_buf, "+EUART:<p1>,<p2>", strlen("+EUART:<p1>,<p2>")+1);
            response->response_len = strlen((char *)response->response_buf);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(response);
            break;

        case ATCI_CMD_MODE_EXECUTION: // rec: AT+EUART=<op>  the handler need to parse the parameters

            param = strtok(parse_cmd->string_ptr, "=");
            while (NULL != (param = strtok(NULL, ","))) {
                param_val[i++] = atoi(param);
            }

            LOG_MSGID_I(common, "atci_cmd_hdlr_uart P0= %d, P1=%d \r\n", 2, param_val[0], param_val[1]);

            uart_port = param_val[0];
            actual_baudrate = param_val[1];
            uartx = g_uart_regbase[uart_port];

            for(j=0; j<UART_USER_BUFFER_SIZE; j++) {
                send_buffer[j] = 0x55;
            }

           // if((g_uart_para[uart_port].state && UART_INTERNAL_DMA_IRQ_MODE) != 0) {

                uart_config.baudrate    = HAL_UART_BAUDRATE_3000000;
                uart_config.parity      = HAL_UART_PARITY_NONE;
                uart_config.stop_bit    = HAL_UART_STOP_BIT_1;
                uart_config.word_length = HAL_UART_WORD_LENGTH_8;

                dma_config.receive_vfifo_alert_size     = 20;
                dma_config.receive_vfifo_buffer         = g_rx_vfifo_buffer;
                dma_config.receive_vfifo_buffer_size    = UART_VFIFO_SIZE;
                dma_config.receive_vfifo_threshold_size = (UART_VFIFO_SIZE*4)/5;
                dma_config.send_vfifo_buffer            = g_tx_vfifo_buffer;
                dma_config.send_vfifo_buffer_size       = UART_VFIFO_SIZE;
                dma_config.send_vfifo_threshold_size    = (UART_VFIFO_SIZE*1)/5;

                hal_uart_deinit(uart_port);
                hal_uart_init(uart_port, &uart_config);
                uart_set_baudrate(uartx, actual_baudrate);
                hal_uart_set_dma(uart_port, &dma_config);
                hal_uart_register_callback(uart_port, at_cmd_uart_dma_callback, NULL);

                hal_uart_send_dma(uart_port, send_buffer, 512);
           // }

    }
    vPortFree(response);

    return ATCI_STATUS_OK;
}
#endif
