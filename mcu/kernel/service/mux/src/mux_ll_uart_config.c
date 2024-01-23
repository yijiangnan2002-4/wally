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

#include "mux_ll_uart_config.h"
#include "mux_ll_uart.h"
#include "memory_attribute.h"

/* LATCH, WAKEUP, SYNC for internal use. */
MUX_DEF_USER_BUFFER(LATCH , HAL_CORE_MCU , HAL_CORE_MCU , 128, 128);
MUX_DEF_USER_BUFFER(SYNC  , HAL_CORE_MCU , HAL_CORE_MCU , 128, 128);
#ifdef AIR_LL_MUX_WAKEUP_ENABLE
MUX_DEF_USER_BUFFER(WAKEUP, HAL_CORE_MCU , HAL_CORE_MCU , 64 , 64 );
#define MUX_DEF_WAKEUP_ATTR MUX_DEF_USER_ATTR(WAKEUP, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_ABOVE_HIGH,30),
#else
#define MUX_DEF_WAKEUP_ATTR
#endif

#define MUX_DEF_INTERNAL_USER_ATTR\
    MUX_DEF_USER_ATTR(LATCH, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_ABOVE_HIGH, 30),\
    MUX_DEF_WAKEUP_ATTR                                                                                 \
    MUX_DEF_USER_ATTR(SYNC, MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL, MUX_LL_UART_PRIORITY_ABOVE_HIGH, 30),


#define MUX_USER_TABLE_START(user_count)\
    mux_ll_uart_user_configure_t g_mux_ll_uart_user_configure_table[user_count] = {

#define MUX_USER_TABLE_END \
    MUX_DEF_INTERNAL_USER_ATTR \
    MUX_DEF_CUSTOMIZATION_USER_ATTR };

/**
* limitation:
*   1. buffer length must be aligned to power of two!!
*   2. user name can not exceed 15 bytes.
*/
                 /* name            tx_data_from   rx_data_to     tx_buff_len   rx_buff_len*/
MUX_DEF_USER_BUFFER(RACE_CMD      , HAL_CORE_MCU , HAL_CORE_MCU , 4096 ,        4096 );
MUX_DEF_USER_BUFFER(RACE_COSYS    , HAL_CORE_MCU , HAL_CORE_MCU , 2    ,        2    );
MUX_DEF_USER_BUFFER(AUDIO_CMD     , HAL_CORE_MCU , HAL_CORE_MCU , 512  ,        512  );
MUX_DEF_USER_BUFFER(AUDIO_CMD_DSP , HAL_CORE_DSP0, HAL_CORE_DSP0, 512  ,        512  );
#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
MUX_DEF_USER_BUFFER(R2L_DL        , HAL_CORE_DSP0, HAL_CORE_DSP0, 32768,        4096 );
MUX_DEF_USER_BUFFER(UL            , HAL_CORE_DSP0, HAL_CORE_DSP0, 2048 ,        8192 );
#else
MUX_DEF_USER_BUFFER(R2L_DL        , HAL_CORE_DSP0, HAL_CORE_DSP0, 4096 ,        16384);
MUX_DEF_USER_BUFFER(UL            , HAL_CORE_DSP0, HAL_CORE_DSP0, 2048 ,        8192 );
#endif

MUX_DEF_CUSTOMIZATION_USER_BUFFER;

MUX_USER_TABLE_START(MAX_MUX_LL_USER_COUNT)
                 /*name            attribute                                                                priority                     tx_pkt_len */
MUX_DEF_USER_ATTR(RACE_CMD       , MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL|MUX_LL_UART_ATTR_USER_TRX_PROTOCOL, MUX_LL_UART_PRIORITY_LOW   , 30 ),
MUX_DEF_USER_ATTR(RACE_COSYS     , MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL                                   , MUX_LL_UART_PRIORITY_LOW   , 30 ),
MUX_DEF_USER_ATTR(AUDIO_CMD      , MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL                                   , MUX_LL_UART_PRIORITY_HIGH  , 30 ),
MUX_DEF_USER_ATTR(AUDIO_CMD_DSP  , MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL                                   , MUX_LL_UART_PRIORITY_HIGH  , 30 ),
#ifdef AIR_DCHS_MODE_SLAVE_ENABLE
MUX_DEF_USER_ATTR(R2L_DL         , MUX_LL_UART_ATTR_PACKAGE_TYPE_DECODE                                   , MUX_LL_UART_PRIORITY_HIGH  , 1920),
#else
MUX_DEF_USER_ATTR(R2L_DL         , MUX_LL_UART_ATTR_PACKAGE_TYPE_DECODE                                   , MUX_LL_UART_PRIORITY_HIGH  , 240),
#endif
MUX_DEF_USER_ATTR(UL             , MUX_LL_UART_ATTR_PACKAGE_TYPE_NORMAL                                   , MUX_LL_UART_PRIORITY_HIGH  , 80 ),
MUX_USER_TABLE_END
