/* Copyright Statement:
*
* (C) 2023  Airoha Technology Corp. All rights reserved.
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
/* Airoha restricted information */

#if defined(__EXT_BOOTLOADER__)

#if defined(AIR_BL_DFU_ENABLE)


#include "lw_mux.h"
#include "race_parser.h"
#include "race_handler.h"
#include "bl_common.h"
#include <string.h>


typedef struct {
    lw_mux_buffer_t *mux_buffer;
    uint32_t counter;               // the count of mux_buffer
    uint32_t total_length;          // total data length
} race_multi_buffer_t;


static uint8_t g_race_cmd_buf[RACE_PROTOCOL_TOTAL_DATA_MAX_LENGTH];


static void race_multi_buffer_init(race_multi_buffer_t *p_race_buf, lw_mux_buffer_t buffers[], uint32_t buffers_counter)
{
    uint32_t i;

    if (NULL == p_race_buf) {
        return;
    }
    memset(p_race_buf, 0, sizeof(race_multi_buffer_t));
    p_race_buf->mux_buffer = buffers;
    p_race_buf->counter = buffers_counter;
    for (i = 0; i < buffers_counter; i++) {
        p_race_buf->total_length += buffers[i].buf_size;
    }
}

static bool race_protocol_fetch(uint8_t *out_buf, uint32_t buf_len, race_multi_buffer_t *p_race_buf, uint32_t offset)
{
    uint32_t i;
    uint32_t buf_idx = 0;
    uint32_t byte_idx = 0;

    if (NULL == p_race_buf || NULL == out_buf || 0 >= buf_len) {
        return false;
    }
    for (buf_idx = 0; buf_idx < p_race_buf->counter; buf_idx++) {
        if (offset >= p_race_buf->mux_buffer[buf_idx].buf_size) {
            offset -= p_race_buf->mux_buffer[buf_idx].buf_size;
        } else {
            byte_idx = offset;
            offset = 0;
            break;
        }
    }
    if (buf_idx >= p_race_buf->counter || offset > 0) {
        return false;
    }

    for (i = 0; i < buf_len; i++) {
        if (byte_idx >= p_race_buf->mux_buffer[buf_idx].buf_size) {
            buf_idx++;
            byte_idx = 0;
            if (buf_idx >= p_race_buf->counter) {
                return false;
            }
        }
        *(out_buf + i) = *(p_race_buf->mux_buffer[buf_idx].p_buf + byte_idx);
        byte_idx++;
    }
    return true;
}

/*
* @p_race_header: output param
* @consume_len: output param
* @ p_race_buf: input param
*/
static bool race_protocol_fetch_header(race_common_hdr_t *p_race_header, uint32_t *consume_len, race_multi_buffer_t *p_race_buf)
{
    bool res = false;
    bool final_res = false;
    uint32_t drop_len = 0;
    uint32_t offset;

    if (NULL == p_race_buf || NULL == p_race_header || NULL == consume_len || NULL == p_race_buf->mux_buffer || 0 >= p_race_buf->counter) {
        return false;
    }

    memset(p_race_header, 0, sizeof(race_common_hdr_t));
    while (p_race_buf->total_length - drop_len >= RACE_PROTOCOL_MIN_SIZE) {
        offset = drop_len;
        res = race_protocol_fetch(&p_race_header->pktId, 1, p_race_buf, offset);
        if (false == res) {
            break;
        }
        if (RACE_PROTOCOL_CHANNEL != p_race_header->pktId) {
            drop_len += 1;
            continue;
        }

        offset += 1;
        res = race_protocol_fetch(&p_race_header->type, 1, p_race_buf, offset);
        if (false == res) {
            break;
        }
        if (false == (RACE_PROTOCOL_TYPE_CMD == p_race_header->type || RACE_PROTOCOL_TYPE_CMD_WITHOUT_RSP == p_race_header->type)) {
            drop_len += 1;
            continue;
        }

        offset += 1;
        res = race_protocol_fetch((uint8_t *)&p_race_header->length, 2, p_race_buf, offset);
        if (false == res) {
            break;
        }
        if (RACE_PROTOCOL_LENGTH_MIN > p_race_header->length || RACE_PROTOCOL_LENGTH_MAX < p_race_header->length) {
            drop_len += 1;
            continue;
        }
        offset += 2;
        res = race_protocol_fetch((uint8_t *)&p_race_header->id, 2, p_race_buf, offset);
        if (res) {
            final_res = true;
        }
        break;
    }
    *consume_len = drop_len;

    return final_res;
}

static void race_protocol_parser(lw_mux_rx_irq_event_msg_t *msg)
{
    race_multi_buffer_t multi_buf;
    race_common_hdr_t hdr;

    if (NULL == msg) {
        return ;
    }
    race_multi_buffer_init(&multi_buf, msg->buff, msg->buff_cnt);
    if (0 >= multi_buf.total_length) {
        return ;
    }
    if(true == race_protocol_fetch_header(&hdr, &msg->consume_len, &multi_buf)) {
        msg->pkt_size = hdr.length + RACE_PROTOCOL_HEADER_SIZE;
    }
    bl_print(LOG_DEBUG, "RACE:parser,in:%d,drop:%d,pkt:%d\r\n", multi_buf.total_length, msg->consume_len, msg->pkt_size);
}

static void race_data_receiver(lw_mux_pkt_ready_event_msg_t *msg)
{
    uint32_t rev_len;
    if (NULL == msg || 0 >= msg->pkt_len) {
        return ;
    }
    rev_len = lw_mux_rx(&g_race_cmd_buf[0], msg->pkt_len, msg->port);
    bl_print(LOG_INFO, "RACE:port:%d,rx:%d,pkt:%d\r\n", msg->port, rev_len, msg->pkt_len);
    race_cmd_handler(&g_race_cmd_buf[0], rev_len, msg->port);
}

void race_lw_mux_event_handler(uint8_t event, void *msg)
{
    bl_print(LOG_DEBUG, "RACE:mux_event:%d\r\n", event);
    switch (event) {
        case LW_MUX_EVENT_RX_IRQ:
            race_protocol_parser((lw_mux_rx_irq_event_msg_t *)msg);
            break;

        case LW_MUX_EVENT_PKT_READY:
            race_data_receiver((lw_mux_pkt_ready_event_msg_t *)msg);
            break;

        default:
            break;
    }
}

#endif

#endif

