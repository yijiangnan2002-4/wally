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

#include "hal_platform.h"

#ifndef __HAL_SLEEP_MANAGER_PLATFORM_H__
#define __HAL_SLEEP_MANAGER_PLATFORM_H__

#ifdef HAL_SLEEP_MANAGER_ENABLED

typedef enum {
    /* PERISYS */
    SLEEP_LOCK_ESC_AESOTF       = 0,
    SLEEP_LOCK_CRYPTO           = 1,
    SLEEP_LOCK_TRNG             = 2,
    SLEEP_LOCK_ESC              = 3,
    SLEEP_LOCK_SPI_MST0         = 4,
    SLEEP_LOCK_SPI_MST1         = 5,
    SLEEP_LOCK_SPI_MST2         = 6,
    SLEEP_LOCK_SPI_SLV          = 7,
    SLEEP_LOCK_I2C1             = 8,
    SLEEP_LOCK_I2C2             = 9,
    SLEEP_LOCK_I3C0             = 10,
    SLEEP_LOCK_I3C1             = 11,
    SLEEP_LOCK_IRRX             = 12,
    SLEEP_LOCK_DCXO_CFG         = 13,
    SLEEP_LOCK_PERISYS_MAX      = 14,
    /* INFRASYS */
    SLEEP_LOCK_AESOTF           = 15,
    SLEEP_LOCK_DMA              = 16, /*TODO check*/
    SLEEP_LOCK_UART0            = 17,
    SLEEP_LOCK_UART1            = 18,
    SLEEP_LOCK_UART2            = 19,
    SLEEP_LOCK_I2C0             = 20,
    SLEEP_LOCK_MSDC             = 21,
    SLEEP_LOCK_USB              = 22,
    SLEEP_LOCK_BT_CONTROLLER    = 23, /*TODO check*/
    SLEEP_LOCK_CHARGER_CASE     = 24, /*TODO check*/
    SLEEP_LOCK_BT_CONTROLLER_A2DP=25, /*TODO check*/
    SLEEP_LOCK_BATTERY_MANAGEMENT=26,
    SLEEP_LOCK_ICE_DEBUG        = 27,
    SLEEP_LOCK_USER_START_ID    = 28,
    SLEEP_LOCK_INVALID_ID       = 0xFF
} sleep_management_lock_request_t;

typedef enum {
    /* PERISYS */
    SLEEP_BACKUP_RESTORE_ESC            = 0,
	SLEEP_BACKUP_RESTORE_SPI_MST0       = 1,
    SLEEP_BACKUP_RESTORE_SPI_MST1       = 2,
    SLEEP_BACKUP_RESTORE_SPI_MST2       = 3,
    SLEEP_BACKUP_RESTORE_SPI_SLV        = 4,
    SLEEP_BACKUP_RESTORE_I2C1           = 5,
    SLEEP_BACKUP_RESTORE_I2C2           = 6,
	SLEEP_BACKUP_RESTORE_I3C0           = 7,
	SLEEP_BACKUP_RESTORE_I3C1           = 8,
	SLEEP_BACKUP_RESTORE_DCXO_CFG       = 9,
    SLEEP_BACKUP_RESTORE_AUXADC         = 10,
    SLEEP_BACKUP_RESTORE_PERISYS_MAX    = 11,
    /* INFRASYS */
    SLEEP_BACKUP_RESTORE_DMA            = 12, /* TODO check */
    SLEEP_BACKUP_RESTORE_FLASH          = 13,
    SLEEP_BACKUP_RESTORE_UART           = 14, /* uart0/1/2 */
    SLEEP_BACKUP_RESTORE_I2C0           = 15,
    SLEEP_BACKUP_RESTORE_MSDC           = 16,
    SLEEP_BACKUP_RESTORE_USB            = 17,
    SLEEP_BACKUP_RESTORE_PMU            = 18, /* I2C_ao backup/restore */
    SLEEP_BACKUP_RESTORE_PWM            = 19, /* AO domain */
    SLEEP_BACKUP_RESTORE_MODULE_MAX     = 20,
    SLEEP_BACKUP_RESTORE_USER           = 21,
} sleep_management_backup_restore_module_t;

#define SLEEP_BACKUP_RESTORE_USER_CALLBACK_FUNC_MAX 8
#define DEEP_SLEEP_GPT HAL_GPT_0

#endif /* HAL_SLEEP_MANAGER_ENABLED */
#endif

