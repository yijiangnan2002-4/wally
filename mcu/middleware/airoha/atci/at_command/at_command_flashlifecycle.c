/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

// For Register AT command handler
// System head file
#include "hal_feature_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "at_command.h"
#include "syslog.h"
#include "hal_gpt.h"

#if !defined(MTK_REG_AT_COMMAND_DISABLE) && defined(HAL_FLASH_MODULE_ENABLED)

log_create_module(atci_sflash, PRINT_LEVEL_INFO);

#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atci_sflash ,"[SFLASH]"fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atci_sflash, "[SFLASH]"fmt,cnt,##arg)
#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atci_sflash ,"[SFLASH]"fmt,cnt,##arg)



// AT command handler
atci_status_t atci_cmd_hdlr_Sflash_lifecycle(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response;
    char cmd[256] = {0};
    uint8_t  i = 0;

    response = atci_mem_alloc(sizeof(atci_response_t));

    strncpy(cmd, (char *)parse_cmd->string_ptr, sizeof(cmd) - 1);
    for (i = 0; i < strlen((char *)parse_cmd->string_ptr); i++) {
        cmd[i] = (char)toupper((unsigned char)cmd[i]);
    }

    response->response_flag = 0; // Command Execute Finish.
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response->cmd_id = parse_cmd->cmd_id;
#endif

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+EREG=?
            strncpy((char *)response->response_buf, "+SFLASH=(0,1)\r\nOK\r\n", strlen("+SFLASH=(0,1)\r\nOK\r\n") + 1);
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+EREG=<op>  the handler need to parse the parameters
            if (strncmp(cmd, "AT+SFLASH=ID", strlen("AT+SFLASH=ID")) == 0) {
                /*command: AT+SFLASH=ID : end count */
                extern uint8_t nor_id[4];
                //LOGMSGIDI("Flash JEDID: 0x%x, 0x%x, 0x%x \r\n", 3, nor_id[0], nor_id[1], nor_id[2]);
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+SFLASH:0x%x,0x%x,0x%x\r\n", (unsigned int)nor_id[0], (unsigned int)nor_id[1], (unsigned int)nor_id[2]);
                response->response_len = strlen((char *)response->response_buf);
            } else if (strncmp(cmd, "AT+SFLASH=UID", strlen("AT+SFLASH=UID")) == 0) {
                /*command: AT+SFLASH=UID : end count */
                extern uint8_t nor_id[4];
                if (nor_id[0] == 0xC8) {
                    uint8_t nor_uid[16];
                    //GD Flash
                    extern void NOR_ReadUID(const uint16_t CS, uint8_t *flashuid, uint32_t uid_length);
                    NOR_ReadUID(0, nor_uid, 16);
                    //LOGMSGIDI("FAB information: 0x%x \r\n", 1, nor_uid[8]);
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+FAB:0x%x\r\n", nor_uid[8]);
                response->response_len = strlen((char *)response->response_buf);
                } else {
                    //LOGMSGIDI("NO FAB information for Vendor 0x%x \r\n", 1, nor_id[0]);
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Not support\r\n");
                    response->response_len = strlen((char *)response->response_buf);
                }
            }
            else {
                /*invalide parameter, return "ERROR"*/
                strncpy((char *)response->response_buf, "ERROR\r\n", strlen("ERROR\r\n") + 1);
                response->response_len = strlen((char *)response->response_buf);
            };
            atci_send_response(response);
            break;

        default :
            /* others are invalid command format */
            strncpy((char *)response->response_buf, "ERROR\r\n", strlen("ERROR\r\n") + 1);
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;
    }

    atci_mem_free(response);

    return ATCI_STATUS_OK;
}

#endif

