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
 
#ifndef __HAL_SYS_H__
#define __HAL_SYS_H__

#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @file hal_sys.h
 *
 * @brief see hal_WDT.c in DVT tree.
 */
#define WHOLE_SYSTEM_REBOOT_COMMAND 0

#define HAL_SYS_REBOOT_MAGIC (538314278)


typedef enum {
    HAL_SYS_OK,
    HAL_SYS_INVALID_COMMAND,
    HAL_SYS_INVALID_MAGIC
} hal_sys_status_t;


typedef void (*hal_sys_reboot_callback_t)(uint32_t command);


/**
 * When a reboot (hal_sys_reboot) is requested, the callback is called.
 *
 * A reboot callback callback can be used to provide dying gasp function.
 *
 * @return Original callback;
 */
hal_sys_reboot_callback_t hal_sys_reboot_register_callback(
    hal_sys_reboot_callback_t    callback);


/**
 * Reboots the system.
 *
 * @param magic must be HAL_SYS_REBOOT_MAGIC to trigger reboot, this avoids
 *        any accidential calls to hal_sys_reboot().
 *
 * @param command 0 to reboot the whole system. Other comamnds may be used to
 *        reboot a part of the system on other chipsets. If the command is 0,
 *        this API will not return.
 *
 * @retval HAL_SYS_OK if the command executes okay.
 */
hal_sys_status_t hal_sys_reboot(uint32_t magic, uint32_t command);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_SYS_H__ */

