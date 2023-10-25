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

/* Includes ------------------------------------------------------------------*/
#include "hal_resource_assignment.h"
#include "hal_feature_config.h"
#include "hal_platform.h"
#include "hal_core_status.h"
#include "memory_attribute.h"

ATTR_TEXT_IN_IRAM hal_core_status_t hal_core_status_read(hal_core_id_t id)
{
    if (id >= HAL_CORE_MAX) {
        return HAL_CORE_ERROR;
    } else {
        return *(((hal_core_id_t *)HW_SYSRAM_PRIVATE_MEMORY_CORE_STATUS_START) + id);
    }
}


hal_core_status_t hal_core_status_write(hal_core_id_t id, hal_core_status_t status)
{
#if defined CORE_CM4
    if (id != HAL_CORE_MCU) {
        return HAL_CORE_ERROR;
    }
#elif defined CORE_DSP0
    if (id != HAL_CORE_DSP0) {
        return HAL_CORE_ERROR;
    }
#else
#error "Must be define this is which Core"
#endif
    *(((hal_core_id_t *)HW_SYSRAM_PRIVATE_MEMORY_CORE_STATUS_START) + id) = status;
    return status;

}


void hal_dsp_core_reset(hal_core_id_t id, uint32_t reset_vector)
{
    /* Set core status to init */
    *(((hal_core_id_t *)HW_SYSRAM_PRIVATE_MEMORY_CORE_STATUS_START) + id) =(hal_core_id_t)HAL_CORE_OFF;

    switch (id) {
        /*DSP0 set reset vector and reset */
        case HAL_CORE_DSP0:
            MCU_CFG_PRI->DSP0CFG_BOOT_VECTOR = reset_vector;
            MCU_CFG_PRI->DSP0CFG_BOOT_VECTOR_SELECT = 1;
            MCU_CFG_PRI->DSP0CFG_STALL = 1;
            WDT_REGISTER->WDT_RST0_UNION.WDT_RST0 = 0x011A0119;
            WDT_REGISTER->WDT_RST0_UNION.WDT_RST0 = 0x001A0019;
            MCU_CFG_PRI->DSP0CFG_STALL = 0;
            break;

        default:
            break;
    }
}


