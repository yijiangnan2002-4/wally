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

// For Register AT command handler
// System head file

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <ctype.h>
#include "toi.h"
#include "at_command.h"
#include "syslog.h"
#include "offline_dump.h"
#include "offline_dump_port.h"

#define RACE_MINIDUMP_MAX_PACK_SIZE 256

#define RACE_HEAD_SIZE              6
#define RACE_HEAD_CHANNEL           0x05
#define RACE_HEAD_TYPE              0x5D
#define RACE_EXCEPTION_STRING       0x0F12
#define RACE_MINIDUMP_PACK_OFFSET   4
#define RACE_MINIDUMP_BUFF_OFFSET   10

static int my_htoi(char s[])
{
    int i;
    int n = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        i = 2;
    } else {
        i = 0;
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i) {
        if (tolower((unsigned char)s[i]) > '9') {
            n = 16 * n + (10 + tolower((unsigned char)s[i]) - 'a');
        } else {
            n = 16 * n + (tolower((unsigned char)s[i]) - '0');
        }
    }
    return n;
}

static void offline_show_usage(uint8_t *buf)
{
    int pos = 0;

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+OFFLINE:\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "REGION TYPE: [0]EXCEPTION LOG [1]MINIDUMP [2]OFFLINE LOG \r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+OFFLINE=0,<region_type>, query region info)\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+OFFLINE=1,<region_type>,<cell_number(HEX)>, query detail info, address and length)\r\n");
}

atci_status_t atci_query_region_info_hdr(uint8_t *buf, offline_dump_region_type_t region_type)
{
    bool ret;
    uint32_t min_seq, max_seq;
    int pos = 0;

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+OFFLINE INFO:\r\n");

    /* region type check */
    if (region_type >= OFFLINE_REGION_MAX) {
        pos += snprintf((char *)(buf + pos),
                        ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                        "region[%d]: region parameter error\r\n",
                        region_type);
        return ATCI_STATUS_ERROR;
    }

    ret = offline_dump_region_query_seq_range(region_type, &min_seq, &max_seq);

    if (ret != OFFLINE_STATUS_OK) {
        pos += snprintf((char *)(buf + pos),
                        ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                        "region[%d]: no dump\r\n",
                        region_type);
        return ATCI_STATUS_ERROR;
    }

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "region[%d]: min=0x%08x max=0x%08x\r\n",
                    region_type,
                    (unsigned int)min_seq,
                    (unsigned int)max_seq);

    return ATCI_STATUS_OK;
}

atci_status_t atci_query_region_address_hdr(uint8_t *buf, offline_dump_region_type_t region_type, uint32_t cell_number)
{
    bool ret;
    int pos = 0;
    uint32_t min_seq, max_seq, start_addr, total_length;
    uint8_t  race_head[RACE_HEAD_SIZE];
    uint32_t race_address;
    uint16_t race_length, race_id, string_length;
    uint8_t  *p_race_buf;

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+OFFLINE ADDRESS:\r\n");

    /* region type check */
    if (region_type >= OFFLINE_REGION_MAX) {
        pos += snprintf((char *)(buf + pos),
                        ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                        "region[%d]: region parameter error\r\n",
                        region_type);
        return ATCI_STATUS_ERROR;
    }

    ret = offline_dump_region_query_seq_range(region_type, &min_seq, &max_seq);
    if (ret != OFFLINE_STATUS_OK) {
        pos += snprintf((char *)(buf + pos),
                        ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                        "region[%d]: query_seq_range fail\r\n",
                        region_type);
        return ATCI_STATUS_ERROR;
    }

    if ((cell_number > max_seq) || (cell_number < min_seq)) {
        pos += snprintf((char *)(buf + pos),
                        ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                        "region[%d]: input dump_index:0x%08x error\r\n",
                        region_type,
                        (unsigned int)cell_number);
        return ATCI_STATUS_ERROR;
    }

    ret = offline_dump_region_query_by_seq(region_type, cell_number, &start_addr, &total_length);
    if (ret == OFFLINE_STATUS_OK) {
        pos += snprintf((char *)(buf + pos),
                        ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                        "region[%d]: address:0x%08x length:0x%08x\r\n",
                        region_type,
                        (unsigned int)start_addr,
                        (unsigned int)total_length);
    } else {
        pos += snprintf((char *)(buf + pos),
                        ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                        "region[%d]: query_by_seq fail\r\n",
                        region_type);
        return ATCI_STATUS_ERROR;
    }

    p_race_buf = pvPortMalloc(RACE_MINIDUMP_MAX_PACK_SIZE);
    if (p_race_buf == NULL) {
        LOG_MSGID_E(offline_dump, "pvPortMalloc fail", 0);
        return ATCI_STATUS_ERROR;
    }

    if (region_type == OFFLINE_REGION_MINI_DUMP) {
        race_address = start_addr + sizeof(offline_dump_header_t);
        while (1) {
            PORT_FLASH_READ(race_address, race_head, sizeof(race_head));
            /* check race header */
            if ((race_head[0] != RACE_HEAD_CHANNEL) && (race_head[1] != RACE_HEAD_TYPE)) {
                pos += snprintf((char *)(buf + pos), ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "query crash info fail, race header error\r\n");
                break;
            }
            /* check race id */
            race_id = (race_head[5] << 8) | race_head[4];
            if (race_id != RACE_EXCEPTION_STRING) {
                pos += snprintf((char *)(buf + pos), ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "query crash info fail, race id error\r\n");
                break;
            }
            /* check race length */
            race_length = (race_head[3] << 8) | race_head[2];
            if ((race_length >= RACE_MINIDUMP_MAX_PACK_SIZE)  || (race_length <= RACE_HEAD_SIZE)) {
                pos += snprintf((char *)(buf + pos), ATCI_UART_TX_FIFO_BUFFER_SIZE - pos, "query crash info fail, race length error\r\n");
                break;
            }
            string_length = race_length - (RACE_MINIDUMP_BUFF_OFFSET - RACE_MINIDUMP_PACK_OFFSET);
            PORT_FLASH_READ(race_address + RACE_MINIDUMP_BUFF_OFFSET, p_race_buf, string_length);
            if (p_race_buf[0] == 'l' && (p_race_buf[1] == 'r')) {
                memcpy((char *)(buf + pos), p_race_buf, string_length);
                pos += string_length;
            } else if (p_race_buf[0] == 'p' && (p_race_buf[1] == 'c')) {
                memcpy((char *)(buf + pos), p_race_buf, string_length);
                pos += string_length;
                break;
            }
            race_address += (race_length + RACE_MINIDUMP_PACK_OFFSET);
        }
    }
    vPortFree(p_race_buf);

    return ATCI_STATUS_OK;
}


atci_status_t atci_cmd_hdlr_offline(atci_parse_cmd_param_t *parse_cmd)
{
    char *p1, *p2;
    atci_status_t status;
    atci_response_t *p_response = pvPortMalloc(sizeof(atci_response_t));
    offline_dump_region_type_t region_type;
    uint32_t cell_number;

    if (p_response == NULL) {
        LOG_MSGID_E(offline_dump, "atci_cmd_hdlr_offline malloc failed.\r\n", 0);
        return ATCI_STATUS_ERROR;
    }

    memset(p_response, 0, sizeof(atci_response_t));

    p_response->response_flag = 0; /* Command Execute Finish. */
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    p_response->cmd_id = parse_cmd->cmd_id;
#endif

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING: // rec: AT+OFFLINE=?
            offline_show_usage(p_response->response_buf);
            p_response->response_len = strlen((char *)p_response->response_buf);
            p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;

        case ATCI_CMD_MODE_READ:
                p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;

        case ATCI_CMD_MODE_EXECUTION:
            if (strstr(parse_cmd->string_ptr, "AT+OFFLINE=0,") != NULL) {
                p1 = parse_cmd->string_ptr + 13;
                region_type = my_htoi(p1);
                status = atci_query_region_info_hdr(p_response->response_buf, region_type);
                if (status != ATCI_STATUS_OK) {
                    p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                } else {
                    p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (strstr(parse_cmd->string_ptr, "AT+OFFLINE=1,") != NULL) {
                p1 = parse_cmd->string_ptr + 13;
                p2 = strchr(p1, ',');
                p2 += 1;
                region_type = my_htoi(p1);
                cell_number = my_htoi(p2);
                status = atci_query_region_address_hdr(p_response->response_buf, region_type, cell_number);
                if (status != ATCI_STATUS_OK) {
                    p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                } else {
                    p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else {
                p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            p_response->response_len = strlen((char *)p_response->response_buf);
            break;

        default:
            break;
    }

    atci_send_response(p_response);
    vPortFree(p_response);

    return ATCI_STATUS_OK;
}



