/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#ifndef _SMCHG_1WIRE_CONFIG_H_
#define _SMCHG_1WIRE_CONFIG_H_
#include "hal_uart.h"
#include "smchg_1wire_nvkey_struct.h"
////////////////////////////////////////////////////////////////////////////////
// Type Definitions ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#ifndef PACKED
#define PACKED __attribute__((packed))
#endif


extern const char HAL_UART0_RXD_PIN_M_UART0_TXD;
extern const char HAL_UART1_RXD_PIN_M_UART1_TXD;
extern const char HAL_UART1_TXD_PIN_M_GPIO;
extern const char HAL_UART1_TXD_PIN_M_UART1_TXD;

#define SMCHG_LOG_MSGID_D(fmt,cnt,arg...) LOG_MSGID_D(SM_CHG,fmt,cnt,##arg)
#define SMCHG_LOG_MSGID_I(fmt,cnt,arg...) LOG_MSGID_I(SM_CHG,fmt,cnt,##arg)
#define SMCHG_LOG_MSGID_W(fmt,cnt,arg...) LOG_MSGID_W(SM_CHG,fmt,cnt,##arg)
#define SMCHG_LOG_MSGID_E(fmt,cnt,arg...) LOG_MSGID_E(SM_CHG,fmt,cnt,##arg)


typedef enum {
    SMCHG_CFG_STATUS_INVALID_PARAMETER = -2,
    SMCHG_CFG_STATUS_ERROR = -1,
    SMCHG_CFG_STATUS_OK = 0
} smchg_cfg_status_t;


typedef enum {
    SMCHG_OUT_OF_CASE_DETECT_NORMAL_SW_POLLING_MODE = 0,
    SMCHG_OUT_OF_CASE_DETECT_FAST_DETECT_INTERRUPT_MODE = 1,
    SMCHG_OUT_OF_CASE_DETECT_MAX
} smchg_out_of_case_detect_mode_t;
////////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern smchg_cfg_t smchg_cfg;


////////////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern void smchg_nvkey_init(void);
extern void smchg_config_init(void);
extern uint32_t smchg_1wire_baudrate_report(hal_uart_baudrate_t baudrate);

#endif /* _SMCHG_1WIRE_CONFIG_H_ */
