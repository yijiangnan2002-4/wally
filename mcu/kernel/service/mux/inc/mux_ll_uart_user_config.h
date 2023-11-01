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

#ifndef __MUX_LL_UART_USER_CONFIG_H__
#define __MUX_LL_UART_USER_CONFIG_H__
#include <stdint.h>
#include "hal_core_status.h"
#include "mux_ll_uart_config.h"


// Example code for user to add channels.

#if 0
//The maximum customization user count
#define MUX_LL_UART_CUSTOMIZATION_USER_COUNT_MAX 4

#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
/**
 * @brief The macro MUX_DEF_TX_FROM_MCU_RX_TO_MCU_USER_BUFFER and MUX_DEF_TX_FROM_DSP_RX_TO_DSP_USER_BUFFER define user buffers.
 *        rx_buff_len and rx_buff_len can not be 0 at the same time.
 * name        : MUX user name, less than 16 bytes.
 * tx_buff_len : TX buffer length. It must align to power of 2. It also can be 0, if the user only act as receiver.
 * rx_buff_len : RX buffer length. It must align to power of 2. It also can be 0, if the user only act as sender.
 */
#define MUX_DEF_CUSTOMIZATION_USER_BUFFER\
                                            /*name   tx_buff_len     rx_buff_len*/\
    MUX_DEF_TX_FROM_MCU_RX_TO_MCU_USER_BUFFER(USR0,     1024,          1024);\
    MUX_DEF_TX_FROM_MCU_RX_TO_MCU_USER_BUFFER(USR1,     1024,          1024);\
    MUX_DEF_TX_FROM_DSP_RX_TO_DSP_USER_BUFFER(USR2,     1024,          1024);\
    MUX_DEF_TX_FROM_DSP_RX_TO_DSP_USER_BUFFER(USR3,     1024,          1024);

/**
 * @brief The macro MUX_DEF_USER_ATTR defines user attributes
 * name     : mux user name, less than 16 bytes. It must be the same with the user name in the user buffer deninition.
 * flag     : It must be MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL.
 * priority : It can be one of MUX_LL_UART_PRIORITY_HIGH, MUX_LL_UART_PRIORITY_MIDDLE and MUX_LL_UART_PRIORITY_LOW
 * pkt_len  : If priority is MUX_LL_UART_PRIORITY_HIGH, pkt_len must be 240
 *            If priority is MUX_LL_UART_PRIORITY_MIDDLE, pkt_len must be 80
 *            If priority is MUX_LL_UART_PRIORITY_LOW, pkt_len must be 30
 */
#define MUX_DEF_CUSTOMIZATION_USER_ATTR\
                    /*name  flag                                  priority                     pkt_len*/\
    MUX_DEF_USER_ATTR(USR0, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_HIGH  , 240),\
    MUX_DEF_USER_ATTR(USR1, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_LOW   , 30),\
    MUX_DEF_USER_ATTR(USR2, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_MIDDLE, 80),\
    MUX_DEF_USER_ATTR(USR3, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_MIDDLE, 80)

#else //AIR_DCHS_MODE_SLAVE_ENABLE

#define MUX_DEF_CUSTOMIZATION_USER_BUFFER\
                                             /*name   tx_buff_len     rx_buff_len*/\
    MUX_DEF_TX_FROM_DSP_RX_TO_DSP_USER_BUFFER(USR0,      1024,          1024);\
    MUX_DEF_TX_FROM_MCU_RX_TO_MCU_USER_BUFFER(USR1,      1024,          1024);\
    MUX_DEF_TX_FROM_MCU_RX_TO_MCU_USER_BUFFER(USR2,      1024,          1024);\
    MUX_DEF_TX_FROM_DSP_RX_TO_DSP_USER_BUFFER(USR3,      1024,          1024);

#define MUX_DEF_CUSTOMIZATION_USER_ATTR\
                    /*name  flag                                  priority                     pkt_len*/\
    MUX_DEF_USER_ATTR(USR0, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_HIGH  , 240),\
    MUX_DEF_USER_ATTR(USR1, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_LOW   , 30),\
    MUX_DEF_USER_ATTR(USR2, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_MIDDLE, 80),\
    MUX_DEF_USER_ATTR(USR3, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_MIDDLE, 80),
#endif //AIR_DCHS_MODE_SLAVE_ENABLE

#endif

#endif //__MUX_LL_UART_USER_CONFIG_H__
