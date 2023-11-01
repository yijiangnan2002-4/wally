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




#include "nvkey.h"
#include "nvkey_id_list.h"
#include "smchg_1wire_config.h"
#include "smartchargertypes.h"
#include "hal_pmu.h"
#include "hal_log.h"
#include "assert.h"
#include "hal_uart.h"



smchg_cfg_t smchg_cfg;

const char __attribute__((weak)) HAL_UART0_RXD_PIN_M_UART0_TXD      = 0xFF;
const char __attribute__((weak)) HAL_UART1_RXD_PIN_M_UART1_TXD      = 0xFF;
const char __attribute__((weak)) HAL_UART1_TXD_PIN_M_GPIO           = 0xFF;
const char __attribute__((weak)) HAL_UART1_TXD_PIN_M_UART1_TXD      = 0xFF;

uint32_t smchg_1wire_baudrate_report(hal_uart_baudrate_t baudrate)
{
    uint32_t ret;

    switch (baudrate) {
        case HAL_UART_BAUDRATE_4800:
            ret = 4800;
            break;
        case HAL_UART_BAUDRATE_9600:
            ret = 9600;
            break;
        case HAL_UART_BAUDRATE_19200:
            ret = 19200;
            break;
        case HAL_UART_BAUDRATE_38400:
            ret = 38400;
            break;
        case HAL_UART_BAUDRATE_57600:
            ret = 57600;
            break;
        case HAL_UART_BAUDRATE_115200:
            ret = 115200;
            break;
        case HAL_UART_BAUDRATE_230400:
            ret = 230400;
            break;
        case HAL_UART_BAUDRATE_460800:
            ret = 460800;
            break;
        case HAL_UART_BAUDRATE_921600:
            ret = 921600;
            break;
        case HAL_UART_BAUDRATE_2000000:
            ret = 2000000;
            break;
        case HAL_UART_BAUDRATE_3000000:
            ret = 3000000;
            break;

        default:
            ret = 0;
            break;
    }

    return ret;
}



void smchg_nvkey_init(void)
{

    uint32_t size = sizeof(smchg_cfg_t);

    nvkey_status_t nvkey_status = nvkey_read_data(NVID_PERI_SMCHG_CFG, (uint8_t *)&smchg_cfg, &size);
    if (nvkey_status != NVKEY_STATUS_OK) {
        assert(0);
    }
    SMCHG_LOG_MSGID_I("smchg, 1wire, chg_type[%d], uart_sel[%d], com_br[%d], log_br[%d], out_case_mode[%d], out_case_timer[%d], com_timer[%d], race_timer[%d], vbus_thrd[%d], vbat_thrd[%d], 1wire_log[%d]", 11,
                       smchg_cfg.chg_type, smchg_cfg.uart_sel, smchg_cfg.com_mode_baud_rate,
                       smchg_cfg.log_mode_baud_rate, smchg_cfg.out_of_case_detect_mode, smchg_cfg.out_of_case_chk_timer, smchg_cfg.com_mode_chk_timer,
                       smchg_cfg.race_mode_chk_timer, smchg_cfg.out_of_case_vbus_uart_thrd, smchg_cfg.com_mode_vbat_thrd, smchg_cfg.one_wire_log);

}

