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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "hal.h"
#include "hal_platform.h"
#include "hal_log.h"
#include "hal_i2c_master.h"
#include "at_command.h"
#include "FreeRTOS.h"
#include "task.h"
#include "syslog.h"

#if defined(HAL_I2C_MASTER_MODULE_ENABLED) && defined(AIR_HQA_TEST_ENABLED)

enum {
    ID_PORT = 0,
    ID_SLV_ADDR,
    ID_FREQ,
    ID_PARTTERN,
    ID_SND_SZ,
    ID_RECV_SZ,
    ID_MAX,
};

#define MAX_I2C_TX_BUFF_SZ          60000
#define MAX_I2C_RX_BUFF_SZ          128

static uint32_t  cmd[ID_MAX];
static hal_i2c_status_t    status;

const char *help_str_i2c_mst    = "AT+EI2C=HQA,port,slv_addr,freq,pattern,snd_sz,recv_sz\r\nOK\r\n";
const char *cmd_str_hqa_i2c_mst = "AT+EI2C=HQA,";

ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t s_i2c_txbuf[MAX_I2C_TX_BUFF_SZ];
ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t s_i2c_rxbuf[MAX_I2C_RX_BUFF_SZ];


void    atci_cmd_i2c_master_callback(uint8_t slave_address, hal_i2c_callback_event_t event, void *user_data)
{
    if(event != HAL_I2C_EVENT_SUCCESS) {
        log_hal_msgid_error("+EI2C: i2c transfer failed, event %d", 1, event);
    }

    hal_i2c_send_to_receive_config_ex_no_busy_t  snd_recv_config;

    snd_recv_config.i2c_config.frequency = cmd[ID_FREQ];
    snd_recv_config.i2c_callback = atci_cmd_i2c_master_callback;
    snd_recv_config.user_data    = NULL;
    snd_recv_config.i2c_send_to_receive_config_ex.receive_buffer = s_i2c_rxbuf;
    snd_recv_config.i2c_send_to_receive_config_ex.receive_bytes_in_one_packet = (cmd[ID_RECV_SZ] & 0xFFFF);
    snd_recv_config.i2c_send_to_receive_config_ex.receive_packet_length = 1;
    snd_recv_config.i2c_send_to_receive_config_ex.send_bytes_in_one_packet = (cmd[ID_SND_SZ] & 0xFFFF);
    snd_recv_config.i2c_send_to_receive_config_ex.send_data = s_i2c_txbuf;
    snd_recv_config.i2c_send_to_receive_config_ex.send_packet_length = 1;
    snd_recv_config.i2c_send_to_receive_config_ex.slave_address = (cmd[ID_SLV_ADDR] & 0xFF);

    status = hal_i2c_master_send_to_receive_dma_ex_none_blocking(cmd[ID_PORT], &snd_recv_config);
}

int  atci_cmd_handle_i2c_mst_snd_recv(char *cmd_string, atci_response_t *response)
{
    char  *ptr = 0;
    int    i = 0;

    ptr = cmd_string;
    for(i = 0;i < ID_MAX; i++){
        ptr = strchr(ptr, ',');
        if(ptr != NULL) {
            ptr++;
            cmd[i] = atoi(ptr);
        } else {
            break;
        }
    }

#if defined(HAL_I2C_MASTER_FEATURE_EXTENDED_DMA) && defined(HAL_I2C_MASTER_FRATURE_NONE_BLOCKING)
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+EI2C:port(%d),slv_addr(0x%x),freq(%d),pattern(0x%x),snd_sz(%d),recv_sz(%d)\r\n",
             (int) cmd[ID_PORT], (unsigned int) cmd[ID_SLV_ADDR], (int) cmd[ID_FREQ], (unsigned int) cmd[ID_PARTTERN], (int) cmd[ID_SND_SZ], (int) cmd[ID_RECV_SZ]);
    memset(s_i2c_txbuf, (uint8_t)cmd[ID_PARTTERN], MAX_I2C_TX_BUFF_SZ);
    
    hal_i2c_send_to_receive_config_ex_no_busy_t  snd_recv_config;

    snd_recv_config.i2c_config.frequency = cmd[ID_FREQ];
    snd_recv_config.i2c_callback = atci_cmd_i2c_master_callback;
    snd_recv_config.user_data    = NULL;
    snd_recv_config.i2c_send_to_receive_config_ex.receive_buffer = s_i2c_rxbuf;
    snd_recv_config.i2c_send_to_receive_config_ex.receive_bytes_in_one_packet = (cmd[ID_RECV_SZ] & 0xFFFF);
    snd_recv_config.i2c_send_to_receive_config_ex.receive_packet_length = 1;
    snd_recv_config.i2c_send_to_receive_config_ex.send_bytes_in_one_packet = (cmd[ID_SND_SZ] & 0xFFFF);
    snd_recv_config.i2c_send_to_receive_config_ex.send_data = s_i2c_txbuf;
    snd_recv_config.i2c_send_to_receive_config_ex.send_packet_length = 1;
    snd_recv_config.i2c_send_to_receive_config_ex.slave_address = (cmd[ID_SLV_ADDR] & 0xFF);

    status = hal_i2c_master_send_to_receive_dma_ex_none_blocking(cmd[ID_PORT], &snd_recv_config);
#else
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+EI2C:Not Enable I2C DMA\r\n");
    status = HAL_I2C_STATUS_ERROR;
#endif
    if(status != HAL_I2C_STATUS_OK) {
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
    } else {
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
    }
    return 0;
}

atci_status_t atci_cmd_hdlr_i2c(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = atci_mem_alloc(sizeof(atci_response_t));
    if (NULL == response) {
        return ATCI_STATUS_ERROR;
    }

    char   *cmd_string = NULL;

    cmd_string = parse_cmd->string_ptr;
    response->response_flag = 0; // Command Execute Finish.
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    /* rec: AT+EI2C=?   */
            strncpy((char *)response->response_buf, help_str_i2c_mst, strlen(help_str_i2c_mst));
            break;
        case ATCI_CMD_MODE_EXECUTION:{
            if (strncmp(cmd_string, "AT+EI2C=HQA,", strlen("AT+EI2C=HQA,") - 1) == 0) {
                atci_cmd_handle_i2c_mst_snd_recv(cmd_string, response);
            } else {

            }
        }
    }
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    atci_mem_free(response);
    return ATCI_STATUS_OK;
}

#endif


