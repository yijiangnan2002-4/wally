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

#ifndef __MUX_LL_UART_CONFIG_H__
#define __MUX_LL_UART_CONFIG_H__
#include <stdint.h>
#include "hal_core_status.h"

#define MUX_LL_UART_RDWR_PTR_CHECK_ENABLE   /**< Only for debug */
#define MUX_LL_UART_SEQ_ENABLE              /**< To enable sequence number in every packet header. */
#define MUX_LL_UART_HEADER_CHECK_ENABLE     /**< To enable header check. */
// #define MUX_LL_UART_CRC_CHECK_ENABLE        /**< To enable data CRC check. */
#define MUX_LL_UART_TIMESTAMP_ENABLE        /**< To enable timestamp in every packet header. This is used to debug. */

#define _HAL_CORE_MCU_MEM_ATTR  ATTR_ZIDATA_IN_NONCACHED_SYSRAM
#define _HAL_CORE_DSP0_MEM_ATTR ATTR_SHARE_ZIDATA

#define MUX_TX_BUFFER(name)     mux_ll_##name##_txbuffer
#define MUX_RX_BUFFER(name)     mux_ll_##name##_rxbuffer
#define MUX_TX_BUFFER_LEN(name) mux_ll_##name##_tx_buf_len
#define MUX_RX_BUFFER_LEN(name) mux_ll_##name##_rx_buf_len
#define MUX_TX_FROM_CORE(name)  mux_ll_##name##_tx_from_core
#define MUX_RX_TO_CORE(name)    mux_ll_##name##_rx_to_core


/**
 * @brief MUX user attributes.
 * MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL and MUX_LL_UART_ATTR_PACKAGE_TYPE_DECODE are exclusive.
 */
typedef enum {
    MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL = 0x01, /**< Send full user data when this flag is set. */
    MUX_LL_UART_ATTR_PACKAGE_TYPE_DECODE = 0x02, /**< Choose high 3 bytes in one word user data to send
                                                           when this flag is set. */
    MUX_LL_UART_ATTR_USER_RX_POLLING     = 0x04, /**< User need to query FIFO when this flag is set, or,
                                                           user callback is called when user data arrived. */
    MUX_LL_UART_ATTR_USER_TRX_PROTOCOL   = 0x08, /**< TX/RX protocol is used when this flag is set. */
} mux_ll_uart_attr_t;

/**
 * @brief User priority
 */
typedef enum {
    MUX_LL_UART_PRIORITY_ABOVE_HIGH = 0, /**< Reserved for internal use. */
    MUX_LL_UART_PRIORITY_HIGH       = 1, /**< The HIGH priority. The highest priority can be used by user.*/
    MUX_LL_UART_PRIORITY_MIDDLE     = 2, /**< The MIDDLE priority.*/
    MUX_LL_UART_PRIORITY_LOW        = 3, /**< The LOW priority.*/
    MUX_LL_UART_PRIORITY_MAX        = 4, /**< The maximum priority number.*/
} mux_ll_uart_priority_t;

/**
 * @brief Define user TX and RX buffers. \n
 * name : user name \n
 * tx_data_from : If the data is send from DSP, the value should be HAL_CORE_DSP0, or, it should be HAL_CORE_MCU. \n
 * rx_data_to : If received data is beblong to DSP, the value should be HAL_CORE_DSP0, or, it should be HAL_CORE_MCU. \n
 */
#define MUX_DEF_USER_BUFFER(name, tx_data_from, rx_data_to, tx_buff_len, rx_buff_len)\
    const uint32_t MUX_TX_BUFFER_LEN(name) = tx_buff_len;\
    const uint32_t MUX_RX_BUFFER_LEN(name) = rx_buff_len;\
    const hal_core_id_t MUX_TX_FROM_CORE(name) = tx_data_from;\
    const hal_core_id_t MUX_RX_TO_CORE(name) = rx_data_to;\
    _##tx_data_from##_MEM_ATTR ATTR_ALIGN(4)  uint8_t MUX_TX_BUFFER(name)[tx_buff_len + 1];\
    _##rx_data_to##_MEM_ATTR ATTR_ALIGN(4)  uint8_t MUX_RX_BUFFER(name)[rx_buff_len + 1]

/**
 * @brief Define user attributes.
 *  nam : user name \n
 *  flag : user flags \n
 *  prio : the priority of sending data. \n
 *  pkt_len : The minimum packet size for sending data each time.\n
 */
#define MUX_DEF_USER_ATTR(nam, flag, prio, pkt_len)\
    {.name = #nam, \
    .tx_scid = MUX_TX_FROM_CORE(nam), \
    .rx_dcid = MUX_RX_TO_CORE(nam), \
    .flags = flag, \
    .tx_buf_len = MUX_TX_BUFFER_LEN(nam), \
    .rx_buf_len = MUX_RX_BUFFER_LEN(nam), \
    .tx_buf_addr = MUX_TX_BUFFER(nam), \
    .rx_buf_addr = MUX_RX_BUFFER(nam), \
    .tx_pkt_len = pkt_len, \
    .tx_priority = prio}


#define MUX_DEF_TX_FROM_DSP_RX_TO_DSP_USER_BUFFER(name, tx_buff_len, rx_buff_len) MUX_DEF_USER_BUFFER(name, HAL_CORE_DSP0, HAL_CORE_DSP0, tx_buff_len, rx_buff_len)
#define MUX_DEF_TX_FROM_MCU_RX_TO_MCU_USER_BUFFER(name, tx_buff_len, rx_buff_len) MUX_DEF_USER_BUFFER(name, HAL_CORE_MCU , HAL_CORE_MCU , tx_buff_len, rx_buff_len)

typedef struct {
    const char *name;
    hal_core_id_t tx_scid;
    hal_core_id_t rx_dcid;
    mux_ll_uart_priority_t tx_priority;

    uint32_t tx_buf_len;
    uint32_t rx_buf_len;
    uint8_t *tx_buf_addr;
    uint8_t *rx_buf_addr;

    uint32_t tx_pkt_len;
    uint8_t flags;
} mux_ll_uart_user_configure_t;

#ifdef AIR_LL_MUX_WAKEUP_ENABLE
#define MUX_LL_USER_WAKEUP_COUNT_MAX 1
#else
#define MUX_LL_USER_WAKEUP_COUNT_MAX 0
#endif
#define MUX_LL_INTERNAL_USER_COUNT_MAX      (2 + MUX_LL_USER_WAKEUP_COUNT_MAX)

#endif //__MUX_LL_UART_CONFIG_H__
