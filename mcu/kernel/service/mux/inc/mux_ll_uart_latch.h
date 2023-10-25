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

#ifndef __MUX_LL_UART_LATCH_H__
#define __MUX_LL_UART_LATCH_H__

#include <stdint.h>
#include "mux_ringbuffer.h"
#include "mux_ll_uart_sync.h"

//user can send data to user fifo,but uart fifo data can not move to uart vfifo
#define DCHS_LOCK_UART_TX()            do {\
    RB_SET_FLAG(&g_port_config.txbuf, MUX_RB_FLAG_TX_BLOCKING);\
    LOG_MSGID_I(common, "[uart latch] lock uart tx", 0);\
} while (0)

#define DCHS_UNLOCK_UART_TX()          do {\
    RB_CLR_FLAG(&g_port_config.txbuf, MUX_RB_FLAG_TX_BLOCKING);\
    LOG_MSGID_I(common, "[uart latch] unlock uart tx", 0);\
} while (0)

#define DCHS_LOCK_UART_TX_SET_REQ()            do {\
    RB_SET_FLAG(&g_port_config.txbuf, MUX_RB_FLAG_TX_BLOCK_REQ);\
} while (0)

#define DCHS_LOCK_UART_TX_CLR_REQ()            do {\
    RB_CLR_FLAG(&g_port_config.txbuf, MUX_RB_FLAG_TX_BLOCK_REQ);\
} while (0)

#define DCHS_IS_UART_TX_LOCKED()        RB_CHK_FLAG(&g_port_config.txbuf, MUX_RB_FLAG_TX_BLOCKING)
#define DCHS_IS_UART_TX_BUFFER_EMPTY()  (mux_ringbuffer_data_length(&g_port_config.txbuf) < 50)
#define DCHS_IS_UART_TX_BLOCK_REQ()     RB_CHK_FLAG(&g_port_config.txbuf, MUX_RB_FLAG_TX_BLOCK_REQ)

extern void mux_ll_uart_latch_init(void);
extern void mux_ll_uart_start_latch_req(void);
extern void mux_ll_latch_timer_start(bool is_one_shot, uint32_t timeout);
extern void mux_ll_latch_timer_stop(void);
extern uint32_t mux_ll_uart_get_clock_offset_us(void);
extern uint32_t mux_ll_uart_is_bt_clock_synced(void);

#define MUX_LL_UART_PORT                MUX_LL_UART_1
#define MUX_LL_UART_SLEEP_LOCK_HANDLE   SLEEP_LOCK_UART1
#endif
