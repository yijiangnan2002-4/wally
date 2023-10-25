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

#include <string.h>
#include "lw_mux.h"
#include "race_parser.h"
#include "lw_mux_uart.h"
#include "bl_common.h"
#include "hal_gpt.h"

#if defined(AIR_BL_USB_HID_DFU_ENABLE)
#include "lw_mux_usb.h"
#endif

/**************************************************************************************************
* Structure
**************************************************************************************************/

typedef enum {
    LW_MUX_GET_READ_BUF = 0,
    LW_MUX_GET_WRITE_BUF,
} lw_mux_get_buffer_type_t;

typedef struct {
    lw_mux_event_handler_t handler;
    lw_mux_tx_function_t tx;
    lw_mux_rx_function_t rx;
    uint8_t *rx_cache;
    uint32_t rx_cache_size;  /* size of rx_cache */
    uint32_t rx_cache_w_idx;
    uint32_t rx_cache_r_idx;
    uint32_t rx_data_len;    /* total data length in rx_cache buffer */
    uint32_t cur_pkt_len;    /* packet length for current packet */
    uint32_t pkt_ts;         /* timestamp for current packet, unit: us */
} lw_mux_port_info_t;


/**************************************************************************************************
* Static Variable
**************************************************************************************************/

#define LW_MUX_UART0_CACHE_SIZE         (RACE_PROTOCOL_TOTAL_DATA_MAX_LENGTH)

#if defined(AIR_BL_USB_HID_DFU_ENABLE)
#define LW_MUX_USBHID0_CACHE_SIZE       (RACE_PROTOCOL_TOTAL_DATA_MAX_LENGTH)
#endif

#define LW_MUX_RX_TIMEOUT               (800000)  /* uint: us */

static uint8_t g_lw_mux_uart0_cache[LW_MUX_UART0_CACHE_SIZE];

#if defined(AIR_BL_USB_HID_DFU_ENABLE)
static uint8_t g_lw_mux_usbhid0_cache[LW_MUX_USBHID0_CACHE_SIZE];
#endif

static lw_mux_port_info_t g_lw_mux_info[LW_MUX_PORT_MAX] = {
    {
        .handler = NULL,
        .tx = lw_mux_uart0_tx,
        .rx = lw_mux_uart0_rx,
        .rx_cache = &g_lw_mux_uart0_cache[0],
        .rx_cache_size = LW_MUX_UART0_CACHE_SIZE,
    },
#if defined(AIR_BL_USB_HID_DFU_ENABLE)
    {
        .handler = NULL,
        .tx = lw_mux_usb_tx,
        .rx = lw_mux_usb_rx,
        .rx_cache = &g_lw_mux_usbhid0_cache[0],
        .rx_cache_size = LW_MUX_USBHID0_CACHE_SIZE,
    },
#endif
};

/**************************************************************************************************
* Prototype
**************************************************************************************************/

static uint32_t lw_mux_get_buffer(lw_mux_buffer_t *buff, lw_mux_port_info_t *info, lw_mux_get_buffer_type_t opt_type);
static void lw_mux_receive_data(lw_mux_port_t port, lw_mux_port_info_t *info);
static void lw_mux_drop_rx_data(lw_mux_port_info_t *info, uint32_t drop_len);


/**************************************************************************************************
* Static Functions
**************************************************************************************************/

static uint32_t lw_mux_get_buffer(lw_mux_buffer_t *buff, lw_mux_port_info_t *info, lw_mux_get_buffer_type_t opt_type)
{
    if (NULL == buff || NULL == info) {
        return 0;
    }
    if (LW_MUX_GET_READ_BUF == opt_type) {
        if (0 >= info->rx_data_len) {
            /* buff empty, no data to read */
            return 0;
        }
        if (info->rx_cache_r_idx >= info->rx_cache_w_idx) {
            buff[0].p_buf = &info->rx_cache[info->rx_cache_r_idx];
            buff[0].buf_size = info->rx_cache_size - info->rx_cache_r_idx;
            if (0 < info->rx_cache_w_idx) {
                buff[1].p_buf = &info->rx_cache[0];
                buff[1].buf_size = info->rx_cache_w_idx;
                return 2;
            }
        } else {
            buff[0].p_buf = &info->rx_cache[info->rx_cache_r_idx];
            buff[0].buf_size = info->rx_cache_w_idx - info->rx_cache_r_idx;
        }
        return 1;
    } else {
        if (info->rx_data_len >= info->rx_cache_size) {
            /* buff full, cannot write */
            return 0;
        }
        if (info->rx_cache_r_idx <= info->rx_cache_w_idx) {
            buff[0].p_buf = &info->rx_cache[info->rx_cache_w_idx];
            buff[0].buf_size = info->rx_cache_size - info->rx_cache_w_idx;
            if (0 < info->rx_cache_r_idx) {
                buff[1].p_buf = &info->rx_cache[0];
                buff[1].buf_size = info->rx_cache_r_idx;
                return 2;
            }
        } else {
            buff[0].p_buf = &info->rx_cache[info->rx_cache_w_idx];
            buff[0].buf_size = info->rx_cache_r_idx - info->rx_cache_w_idx;
        }
        return 1;
    }
}

static void lw_mux_receive_data(lw_mux_port_t port, lw_mux_port_info_t *info)
{
    lw_mux_buffer_t buff[2] = {{NULL, 0}, {NULL, 0}};
    uint32_t i, rev_len, old_len;
    lw_mux_rx_irq_event_msg_t rx_msg = {0, 0, port, NULL, 0};
    lw_mux_pkt_ready_event_msg_t pkt_msg = {port, 0};
    bool rx_new_data = false;
    uint32_t cur_ts = 0;

    if (NULL == info || NULL == info->handler || NULL == info->rx) {
        return ;
    }
    old_len = info->rx_data_len;

    if (info->rx_cache_size > info->rx_data_len) {
        /* step1. prepare rx buffer */
        lw_mux_get_buffer(&buff[0], info, LW_MUX_GET_WRITE_BUF);
        //bl_print(LOG_DEBUG, "buff[0].buf_size = %d, buff[1].buf_size = %d\r\n", buff[0].buf_size, buff[1].buf_size);

        /* step2. recieve data from port */
        i = 0;
        while(i < 2) {
            if (buff[i].buf_size > 0) {
                rev_len = info->rx(buff[i].p_buf, buff[i].buf_size);
                if (0 >= rev_len) {
                    break;
                }
                info->rx_data_len += rev_len;
                info->rx_cache_w_idx += rev_len;
                if (info->rx_cache_w_idx >= info->rx_cache_size) {
                    info->rx_cache_w_idx = 0;
                }

                if (rev_len < buff[i].buf_size) {
                    /* current packet has rx done */
                    break;
                } else {
                    i++;
                }
            } else {
                i++;
            }
        }
    }

    if (old_len != info->rx_data_len) {
        rx_new_data = true;
        bl_print(LOG_DEBUG, "MUX:rx len:%d -> %d\r\n", old_len, info->rx_data_len);
    }
    /* step3. dispatch protocol */
    if (0 >= info->cur_pkt_len && 0 < info->rx_data_len) {
        memset(&buff[0], 0, sizeof(buff));
        rx_msg.buff_cnt = lw_mux_get_buffer(&buff[0], info, LW_MUX_GET_READ_BUF);
        rx_msg.buff = &buff[0];
        info->handler(LW_MUX_EVENT_RX_IRQ, &rx_msg);
        info->cur_pkt_len = rx_msg.pkt_size;
        /* drop consume_len byte data */
        if (rx_msg.consume_len) {
            lw_mux_drop_rx_data(info, rx_msg.consume_len);
        }
        if (true == rx_new_data && 0 < info->rx_data_len) {
            /* update timestamp */
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &info->pkt_ts);
        }
    }

    /* step4. deal pkt */
    if (0 < info->cur_pkt_len) {
        if (info->cur_pkt_len <= info->rx_data_len) {
            bl_print(LOG_DEBUG, "MUX:rx, cur_pkt:%d, rx_data:%d\r\n", info->cur_pkt_len, info->rx_data_len);
            pkt_msg.pkt_len = info->cur_pkt_len;
            info->handler(LW_MUX_EVENT_PKT_READY, &pkt_msg);
        } else {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &cur_ts);
            if (cur_ts > info->pkt_ts + LW_MUX_RX_TIMEOUT) {
                /* drop all rx cache data */
                bl_print(LOG_DEBUG, "MUX:rx timeout:%d~%d\r\n", cur_ts, info->pkt_ts);
                lw_mux_drop_rx_data(info, info->rx_data_len);
                info->cur_pkt_len = 0;
            }
        }
    }

}


static void lw_mux_drop_rx_data(lw_mux_port_info_t *info, uint32_t drop_len)
{
    uint32_t i, buf_cnt, temp_len;
    lw_mux_buffer_t buff[2] = {{NULL, 0}, {NULL, 0}};
    if (NULL == info) {
        return ;
    }
    drop_len = drop_len <= info->rx_data_len ? drop_len : info->rx_data_len;
    if (0 >= drop_len) {
        return ;
    }
    bl_print(LOG_DEBUG, "MUX:drop_rx:%d\r\n", drop_len);
    buf_cnt = lw_mux_get_buffer(&buff[0], info, LW_MUX_GET_READ_BUF);
    for (i = 0; i < buf_cnt; i++) {
        temp_len = drop_len <= buff[i].buf_size ? drop_len : buff[i].buf_size;
        drop_len -= temp_len;
        info->rx_data_len -= temp_len;
        info->rx_cache_r_idx += temp_len;
        if (info->rx_cache_r_idx >= info->rx_cache_size) {
            info->rx_cache_r_idx = 0;
        }
        if (0 >= drop_len) {
            break;
        }
    }
}


/**************************************************************************************************
* Public Functions
**************************************************************************************************/

void lw_mux_local_init(void)
{
    uint16_t i;
    for (i = 0; i < LW_MUX_PORT_MAX; i++) {
        g_lw_mux_info[i].rx_cache_w_idx = 0;
        g_lw_mux_info[i].rx_cache_r_idx = 0;
        g_lw_mux_info[i].rx_data_len = 0;
        g_lw_mux_info[i].cur_pkt_len = 0;
        g_lw_mux_info[i].pkt_ts = 0;
    }
}

bool lw_mux_open(lw_mux_port_t port, lw_mux_event_handler_t hdl)
{
    if (port < LW_MUX_PORT_MAX) {
        g_lw_mux_info[port].rx_cache_w_idx = 0;
        g_lw_mux_info[port].rx_cache_r_idx = 0;
        g_lw_mux_info[port].rx_data_len = 0;
        g_lw_mux_info[port].cur_pkt_len = 0;
        g_lw_mux_info[port].pkt_ts = 0;
        g_lw_mux_info[port].handler = hdl;
        return true;
    }
    return false;
}

bool lw_mux_close(lw_mux_port_t port)
{
    if (port < LW_MUX_PORT_MAX) {
        g_lw_mux_info[port].handler = NULL;
        g_lw_mux_info[port].rx_cache_w_idx = 0;
        g_lw_mux_info[port].rx_cache_r_idx = 0;
        g_lw_mux_info[port].rx_data_len = 0;
        g_lw_mux_info[port].cur_pkt_len = 0;
        g_lw_mux_info[port].pkt_ts = 0;
    }
    return true;
}

uint32_t lw_mux_rx(uint8_t *buf, uint32_t buf_size, lw_mux_port_t port)
{
    lw_mux_port_info_t *info = NULL;
    uint32_t i = 0, buf_cnt = 0;
    uint8_t *wp = buf;
    uint32_t length = buf_size;
    lw_mux_buffer_t buff[2] = {{NULL, 0}, {NULL, 0}};
    uint32_t temp_len = 0, cpy_len = 0;

    if (NULL == buf || 0 >= buf_size) {
        return 0;
    }
    if (port < LW_MUX_PORT_MAX) {
        info = &g_lw_mux_info[port];
    } else {
        return 0;
    }

    if (info->rx_data_len <= 0) {
        return 0;
    }

    buf_cnt = lw_mux_get_buffer(&buff[0], info, LW_MUX_GET_READ_BUF);
    for (i = 0; i < buf_cnt; i++) {
        temp_len = buff[i].buf_size <= length ? buff[i].buf_size : length;
        memcpy(wp, buff[i].p_buf, temp_len);

        length -= temp_len;
        wp = &wp[temp_len];
        cpy_len += temp_len;
        lw_mux_drop_rx_data(info, temp_len);
        if (0 >= length) {
            break;
        }
    }
    info->cur_pkt_len = 0;
    return cpy_len;
}

uint32_t lw_mux_tx(uint8_t *buf, uint32_t buf_size, lw_mux_port_t port)
{
    lw_mux_port_info_t *info = NULL;

    if (NULL == buf || 0 >= buf_size) {
        return 0;
    }
    if (port < LW_MUX_PORT_MAX) {
        info = &g_lw_mux_info[port];
    } else {
        return 0;
    }

    return info->tx(buf, buf_size);
}


void lw_mux_trigger_receiver(void)
{
    lw_mux_port_t port;
    for (port = LW_MUX_UART_0; port < LW_MUX_PORT_MAX; port++) {
        if (g_lw_mux_info[port].handler) {
            lw_mux_receive_data(port, &g_lw_mux_info[port]);
        }
    }
}

#endif

#endif

