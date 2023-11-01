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

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "hal_uart.h"

// #define AIR_SYSLOG_PORT_DEBUG_ENABLE


#define MAX_DEBUG_LOG_SIZE      160
#define RACE_LOG_HEARDER_SIZE   16

static bool is_io_ready = false;
static uint32_t debug_uart_port;

void syslog_debug_uart_init(uint32_t port)
{
#ifdef AIR_SYSLOG_PORT_DEBUG_ENABLE
    hal_uart_config_t uart_config;
    uart_config.baudrate    = HAL_UART_BAUDRATE_3000000;
    uart_config.parity      = HAL_UART_PARITY_NONE;
    uart_config.stop_bit    = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;

    if (port >= HAL_UART_MAX) {
        return ;
    }

    debug_uart_port = port;
    hal_uart_deinit(port);
    hal_uart_init(port, &uart_config);

#ifndef MTK_DEBUG_PLAIN_LOG_ENABLE
    hal_uart_set_software_flowcontrol(port, 0x11, 0x13, 0x77);
#endif

    is_io_ready = true;
#else
    (void)debug_uart_port;
    is_io_ready = false;
#endif
}

int debug_printf(const char *message, ...)
{
#ifdef AIR_SYSLOG_PORT_DEBUG_ENABLE
    va_list args;
    int32_t log_size;
    char frame_header[MAX_DEBUG_LOG_SIZE + RACE_LOG_HEARDER_SIZE];

    if(is_io_ready == false) {
        return 0;
    }

    va_start(args, message);

    log_size = vsnprintf(&frame_header[RACE_LOG_HEARDER_SIZE], MAX_DEBUG_LOG_SIZE, message, args);

    if (log_size < 0) {
        va_end(args);
        return 0;
    }

    if ((uint32_t)log_size >= MAX_DEBUG_LOG_SIZE) {
        log_size = MAX_DEBUG_LOG_SIZE - 1;
    }
    va_end(args);

    frame_header[0] = 0x05;
    frame_header[1] = 0x5D;
    frame_header[2] = (uint8_t)((log_size + 2 + 10) & 0xFF);
    frame_header[3] = (uint8_t)(((log_size + 2 + 10) >> 8) & 0xFF);
    frame_header[4] = (uint8_t)(0x0F40 & 0xFF);
    frame_header[5] = (uint8_t)((0x0F40 >> 8) & 0xFF);

    frame_header[6]  = 0; // CPU ID = 0
    frame_header[7]  = 0;

    frame_header[8]  = 0; // timestamp
    frame_header[9]  = 0;
    frame_header[10] = 0;
    frame_header[11] = 0;

    frame_header[12] = 0; // task info
    frame_header[13] = 0;
    frame_header[14] = 0;
    frame_header[15] = 0;

    hal_uart_send_polling(debug_uart_port, (uint8_t *)frame_header, log_size + RACE_LOG_HEARDER_SIZE);

    return log_size;
#else
    return 0;
#endif
}







