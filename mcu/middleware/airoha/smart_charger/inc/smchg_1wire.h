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

#ifndef _SMCHG_1WIRE_H_
#define _SMCHG_1WIRE_H_

#include "hal_platform.h"
#include "smartchargertypes.h"
#include "types.h"
#include "mux.h"
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#endif
////////////////////////////////////////////////////////////////////////////////
// Type Definitions ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
typedef enum {
    SMCHG_STATUS_REENTER_CHG_MODE                      = 1,
    SMCHG_STATUS_OK                                    = 0,
    SMCHG_STATUS_ERROR                                 = -1,
    SMCHG_STATUS_INVALID_PARAMETER                     = -2,
    SMCHG_STATUS_ERROR_TMR_OP_FAIL                     = -3,
    SMCHG_STATUS_ERROR_RX_HDL_TMR_RESTART              = -4,
    SMCHG_STATUS_ERROR_MUX_CTRL_FAIL                   = -5,
    SMCHG_STATUS_ERROR_MUX_TX_FAIL                     = -6,
    SMCHG_STATUS_ERROR_MUX_RX_FAIL                     = -7,
    SMCHG_STATUS_TX_POST_INVALID_CMD_ID                = -100,
    SMCHG_STATUS_TX_POST_INVALID_BAUDRATE              = -101,
    SMCHG_STATUS_TX_POST_INVALID_SMCHG_STATE           = -102,
    SMCHG_STATUS_TX_POST_INVALID_CMD_PTR               = -103
} smchg_status_t;

typedef enum {
    SMCHG_1WIRE_NORM,
    SMCHG_1WIRE_OUT,
    SMCHG_1WIRE_LOG,
    SMCHG_1WIRE_CHG,
    SMCHG_1WIRE_COM,
    SMCHG_1WIRE_RACE,
    SMCHG_1WIRE_ATCI,
    SMCHG_1WIRE_MAX,
} smchg_1wire_mode_t;

typedef struct {
    uint8_t tx_pin;
    uint8_t tx_gpio;
    uint8_t tx_uart;
    uint8_t rx_pin;
    uint8_t rx_gpio;
    uint8_t rx_uart;
    uint8_t trx_pin;
    uint8_t trx_gpio;
} smchg_1wire_gpio_t;

////////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern smchg_1wire_gpio_t smchg_1wire_gpio_cfg;
extern uint8_t race_mode_flag;


////////////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern void smchg_1wire_init(void);
extern void smchg_1wire_pre_handle(void);
extern int32_t smchg_1wire_chg_exist(void);
extern void smchg_1wire_com_mode_tx_done(void);
extern void smchg_1wire_init_to_log_mode(void);
extern void smchg_1wire_leave_race_mode(void);
extern void smchg_1wire_log_mode(void);
extern smchg_status_t smchg_1wire_enter_pwr_save_mode(void);
extern smchg_1wire_mode_t smchg_1wire_get_mode_status(void);
extern smchg_status_t smchg_1wire_set_mode_status(smchg_1wire_mode_t mode);
extern void smchg_1wire_gpio_init(void);
extern uint32_t smchg_1wire_baudrate_report(hal_uart_baudrate_t baudrate);
extern void smchg_1wire_mux_rx_protocol_cb(mux_handle_t *handle, mux_buffer_t buffers[], uint32_t buffers_counter, uint32_t *consume_len, uint32_t *package_len, void *user_data);
extern void smchg_1wire_mux_tx_protocol_cb(mux_handle_t handle, const mux_buffer_t payload[], uint32_t buffers_counter, mux_buffer_t *head, mux_buffer_t *tail, void *user_data);
extern void smchg_1wire_detect_callback(battery_management_event_t event, const void *data);

#endif /* _SMCHG_1WIRE_H_ */
