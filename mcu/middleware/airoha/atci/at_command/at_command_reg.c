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
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "at_command.h"
#include "syslog.h"
#include <stdlib.h>

#if !defined(MTK_REG_AT_COMMAND_DISABLE) && defined(HAL_REG_MODULE_ENABLED)

log_create_module(atci_reg, PRINT_LEVEL_INFO);

#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atci_reg ,"[REG]"fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atci_reg, "[REG]"fmt,cnt,##arg)
#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atci_reg ,"[REG]"fmt,cnt,##arg)


/*
 * sample code
*/

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_reg(atci_parse_cmd_param_t *parse_cmd);
/*
AT+EPMUREG=<op>                |   "OK"
AT+EPMUREG=?                |   "+EPMUREG=(0,1)","OK"
*/

void defualt_write_api(uint16_t Addr, uint8_t Data)
{
    (void)Addr;
    (void)Data;
}

uint32_t defualt_read_api(uint16_t Addr)
{
    (void)Addr;
    return 0xdeaddead;
}

#if defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak DRV_2WIRE_Write = defualt_write_api
#pragma weak DRV_2WIRE_Read = defualt_read_api
#endif

extern void DRV_2WIRE_Write(uint16_t Addr, uint8_t Data);
extern uint32_t DRV_2WIRE_Read(uint16_t Addr);

// AT command handler
atci_status_t atci_cmd_hdlr_reg(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response;
    int read_value = 0;
    int input_addr = 0;
    int input_value = 0;
    char *end_pos = NULL;
    char cmd[256] = {0};
    uint8_t  i = 0;
    uint8_t opration = 0;

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
            strncpy((char *)response->response_buf, "+EREG=(0,1)\r\nOK\r\n", strlen("+EREG=(0,1)\r\nOK\r\n") + 1);
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+EREG=<op>  the handler need to parse the parameters
            if (strncmp(cmd, "AT+EREG=0,", strlen("AT+EREG=0,")) == 0) {
                opration = 0;
            } else if (strncmp(cmd, "AT+EREG=1,", strlen("AT+EREG=1,")) == 0) {
                opration = 1;
            } else if (strncmp(cmd, "AT+EREG=2,", strlen("AT+EREG=2,")) == 0) {
                opration = 2;
            } else if (strncmp(cmd, "AT+EREG=3,", strlen("AT+EREG=3,")) == 0) {
                opration = 3;
            }  else if (strncmp(cmd, "AT+EREG=5,", strlen("AT+EREG=5,")) == 0) {
                opration = 5;
            } else if (strncmp(cmd, "AT+EREG=6,", strlen("AT+EREG=6,")) == 0) {
                opration = 6;
            } else {
                /*invalide parameter, return "ERROR"*/
                strncpy((char *)response->response_buf, "ERROR\r\n", strlen("ERROR\r\n") + 1);
                response->response_len = strlen((char *)response->response_buf);
            };

            if (opration == 0 || opration == 2 || opration == 5) {
                end_pos = strchr(cmd, ',');
                end_pos ++;

                input_addr = (int)strtoul(end_pos, NULL, 0);
                end_pos = NULL;
                //LOGMSGIDI("read register address:0x%x\r\n", 1, input_addr);

                if (opration == 0) {
                    /*command: AT+EREG=0,1234*/
                    /* read data of input register address */
                    //read_value = *((volatile unsigned int *)input_addr);
                    read_value |= ((uint32_t)(*((volatile unsigned char *)input_addr++)) << 0);
                    read_value |= ((uint32_t)(*((volatile unsigned char *)input_addr++)) << 8);
                    read_value |= ((uint32_t)(*((volatile unsigned char *)input_addr++)) << 16);
                    read_value |= ((uint32_t)(*((volatile unsigned char *)input_addr++)) << 24);
                    snprintf((char *)response->response_buf, sizeof(response->response_buf), "+EREG R0:0x%x,0x%x\r\n", (input_addr - 4), read_value);
                } else if (opration == 2) {
                    /*command: AT+EREG=2,1234*/
                    read_value = DRV_2WIRE_Read(input_addr);
                    snprintf((char *)response->response_buf, sizeof(response->response_buf), "+EREG R2:0x%x,0x%x\r\n", input_addr, read_value);
                } else if (opration == 5) {
                    /*command: AT+EREG=5,1234*/
                    read_value = *((volatile unsigned int *)input_addr);
                    snprintf((char *)response->response_buf, sizeof(response->response_buf), "+EREG R5:0x%x,0x%x\r\n", input_addr, read_value);
                }

                /* ATCI will help append "OK" at the end of resonse buffer */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((char *)response->response_buf);
            } else if (opration == 1 || opration == 3 || opration == 6) {
                char *mid_pos = NULL;
                char str[20] = {0};
                mid_pos = strchr(cmd, ',');
                mid_pos ++;
                end_pos = strchr(mid_pos, ',');

                if (strlen(mid_pos) - strlen(end_pos) < 20) {
                    memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
                    input_addr = (int)strtoul(mid_pos, NULL, 0);
                    end_pos ++;
                    input_value = (int)strtoul(end_pos, NULL, 0);

                    mid_pos = NULL;
                    end_pos = NULL;
                    //LOGMSGIDI("register address:0x%x, set register value:0x%x\r\n", 2, input_addr, input_value);

                    if (opration == 1) {
                        /*command: AT+EREG=1,1234,456*/
                        /* write input data to input register address*/
                        // *((volatile unsigned int *)input_addr) = input_value;
                        *((volatile unsigned char *)input_addr++) = (input_value >> 0) & 0xFF;
                        *((volatile unsigned char *)input_addr++) = (input_value >> 8) & 0xFF;
                        *((volatile unsigned char *)input_addr++) = (input_value >> 16) & 0xFF;
                        *((volatile unsigned char *)input_addr++) = (input_value >> 24) & 0xFF;
                        snprintf((char *)response->response_buf, sizeof(response->response_buf), "+EREG W1:0x%x,0x%x\r\n", input_addr, input_value);
                    } else if (opration == 3) {
                        /*command: AT+EREG=3,1234,456*/
                        /* write input data to input register address*/
                        DRV_2WIRE_Write((uint16_t)input_addr, (uint8_t)input_value);
                        snprintf((char *)response->response_buf, sizeof(response->response_buf), "+EREG W2:0x%x,0x%x\r\n", input_addr, input_value);
                    } else if (opration == 6) {
                        /*command: AT+EREG=6,1234,456*/
                        *((volatile unsigned int *)input_addr) = input_value;
                        snprintf((char *)response->response_buf, sizeof(response->response_buf), "+EREG W6:0x%x,0x%x\r\n", input_addr, input_value);
                    }

                    
                    /* ATCI will help append "OK" at the end of resonse buffer    */
                    response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                    response->response_len = strlen((char *)response->response_buf);
                } else {
                    //LOGMSGIDE("register address failed as error input\r\n", 0);
                    /*invalide parameter, return "ERROR"*/
                    strncpy((char *)response->response_buf, "ERROR\r\n", strlen("ERROR\r\n") + 1);
                    response->response_len = strlen((char *)response->response_buf);
                }
            }

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
