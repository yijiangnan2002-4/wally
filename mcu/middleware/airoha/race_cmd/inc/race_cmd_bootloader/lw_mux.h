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

#ifndef LW_MUX_H
#define LW_MUX_H


#ifdef __cplusplus
extern "C"
{
#endif

#if defined(AIR_BL_DFU_ENABLE)

#include <stdint.h>
#include <stdbool.h>


#ifndef NULL
#define NULL                            (void*)0
#endif


typedef enum {
    LW_MUX_UART_0 = 0,

#if defined(AIR_BL_USB_HID_DFU_ENABLE)
    LW_MUX_USB_HID_0,
#endif

    LW_MUX_PORT_MAX
} lw_mux_port_t;

typedef enum {
    LW_MUX_EVENT_RX_IRQ = 1,
    LW_MUX_EVENT_PKT_READY,
    LW_MUX_EVENT_MAX
} lw_mux_event_t;

typedef struct {
    uint8_t *p_buf;
    uint32_t buf_size;
} lw_mux_buffer_t;

typedef struct {
    uint32_t consume_len; // out param
    uint32_t pkt_size;    // out param
    lw_mux_port_t port;
    lw_mux_buffer_t *buff;
    uint32_t buff_cnt;
} lw_mux_rx_irq_event_msg_t;

typedef struct {
    lw_mux_port_t port;
    uint32_t pkt_len;
} lw_mux_pkt_ready_event_msg_t;


typedef void (*lw_mux_event_handler_t)(uint8_t event, void *event_data);
typedef uint32_t (*lw_mux_tx_function_t)(uint8_t *p_data, uint32_t len);
typedef uint32_t (*lw_mux_rx_function_t)(uint8_t *p_data, uint32_t len);


void lw_mux_local_init(void);
bool lw_mux_open(lw_mux_port_t port, lw_mux_event_handler_t hdl);
bool lw_mux_close(lw_mux_port_t port);
uint32_t lw_mux_rx(uint8_t *buf, uint32_t buf_size, lw_mux_port_t port);
uint32_t lw_mux_tx(uint8_t *buf, uint32_t buf_size, lw_mux_port_t port);
void lw_mux_trigger_receiver(void);


#endif


#ifdef __cplusplus
}
#endif

#endif

#endif

