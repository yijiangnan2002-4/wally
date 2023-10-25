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

#include "hal_mpu.h"

#ifdef HAL_MPU_MODULE_ENABLED

#include "hal_mpu_internal.h"
#include "hal_log.h"
#include "hal_nvic.h"
#include "hal_nvic_internal.h"
#include "assert.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MPU_BUSY 1
#define MPU_IDLE 0

#define MPU_MAIR_NORMAL_MEMORY 0
#define MPU_MAIR_DEVICE_MEMORY 1

volatile uint8_t g_mpu_status = MPU_IDLE;

hal_mpu_status_t hal_mpu_init(const hal_mpu_config_t *mpu_config)
{
    hal_mpu_region_t region;
    uint32_t irq_flag;

    /* Parameter check */
    if (mpu_config == NULL) {
        return HAL_MPU_STATUS_INVALID_PARAMETER;
    }

    /* In order to prevent race condition, interrupt should be disabled when query and update global variable which indicates the module status */
    hal_nvic_save_and_set_interrupt_mask(&irq_flag);

    /* Check module status */
    if (g_mpu_status == MPU_BUSY) {
        /* Restore the previous status of interrupt */
        hal_nvic_restore_interrupt_mask(irq_flag);

        return HAL_MPU_STATUS_ERROR_BUSY;
    } else {
        /* Change status to busy */
        g_mpu_status = MPU_BUSY;

        /* Restore the previous status of interrupt */
        hal_nvic_restore_interrupt_mask(irq_flag);
    }

    /* Set CTRL register to default value */
    MPU->CTRL = 0;

    /* Update PRIVDEFENA and HFNMIENA bit of CTRL register */
    if (mpu_config->privdefena == TRUE) {
        MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk;
    }
    if (mpu_config->hfnmiena == TRUE) {
        MPU->CTRL |= MPU_CTRL_HFNMIENA_Msk;
    }

    /* Update the global variable */
    g_mpu_ctrl.w = MPU->CTRL;
    g_mpu_region_en = 0;

    /* Init global variable for each region */
    for (region = HAL_MPU_REGION_0; region < HAL_MPU_REGION_MAX; region++) {
        /* Initialize rbar to 0 for each region*/
        g_mpu_entry[region].mpu_rbar.w = 0;

        /* Initialize rlar to 0 for each region*/
        g_mpu_entry[region].mpu_rlar.w = 0;
    }

    /* 0x00 means Device-nGnRnE memory
     * 0x33 means
     *      [7:4] Outer Write-through transient, Read & Write Allocate
     *      [0:3] Inner Write-through transient, Read & Write Allocate
     * The attributes here are strongly related to Cache,
     * but Airoha has its own cache, so as long as it is not set to the default Device Memory.
     */
    g_mpu_mair0.w = (uint32_t)0x00000033;
    MPU->MAIR0 = g_mpu_mair0.w;

    return HAL_MPU_STATUS_OK;
}

hal_mpu_status_t hal_mpu_deinit(void)
{
    hal_mpu_region_t region;

    /* Set CTRL register to default value */
    MPU->CTRL = 0;

    /* Update the global variable */
    g_mpu_ctrl.w = 0;
    g_mpu_region_en = 0;

    /* Reset MPU setting as well as global variables to default value */
    for (region = HAL_MPU_REGION_0; region < HAL_MPU_REGION_MAX; region++) {
        MPU->RNR  = region;
        MPU->RBAR = 0;
        MPU->RLAR = 0;

        /* Update the global variable */
        g_mpu_entry[region].mpu_rbar.w = 0;
        g_mpu_entry[region].mpu_rlar.w = 0;
    }

    g_mpu_mair0.w = 0;
    g_mpu_mair1.w = 0;

    /* Change status to idle */
    g_mpu_status = MPU_IDLE;

    return HAL_MPU_STATUS_OK;
}

hal_mpu_status_t hal_mpu_enable(void)
{
    __DMB();
    /* Enable MPU */
    MPU->CTRL |= MPU_CTRL_ENABLE_Msk;

    __DSB();
    __ISB();

    /* Update the global variable */
    g_mpu_ctrl.w = MPU->CTRL;

    return HAL_MPU_STATUS_OK;
}

hal_mpu_status_t hal_mpu_disable(void)
{
    __DMB();
    /* Disable MPU */
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;
    __DSB();
    __ISB();

    /* Update the global variable */
    g_mpu_ctrl.w = MPU->CTRL;

    return HAL_MPU_STATUS_OK;
}

hal_mpu_status_t hal_mpu_region_enable(hal_mpu_region_t region)
{
    /* Region is invalid */
    if (region >= HAL_MPU_REGION_MAX) {
        return HAL_MPU_STATUS_ERROR_REGION;
    }

    /* Enable corresponding region */
    MPU->RNR = (region << MPU_RNR_REGION_Pos);
    MPU->RLAR |= MPU_RLAR_EN_Msk;

    /* Update the global variable */
    g_mpu_entry[region].mpu_rlar.w = MPU->RLAR;
    g_mpu_region_en |= (1 << region);

    return HAL_MPU_STATUS_OK;
}

hal_mpu_status_t hal_mpu_region_disable(hal_mpu_region_t region)
{
    /* Region is invalid */
    if (region >= HAL_MPU_REGION_MAX) {
        return HAL_MPU_STATUS_ERROR_REGION;
    }

    /* Disable corresponding region */
    MPU->RNR = (region << MPU_RNR_REGION_Pos);
    MPU->RLAR &= ~MPU_RLAR_EN_Msk;

    /* Update the global variable */
    g_mpu_entry[region].mpu_rlar.w = MPU->RLAR;
    g_mpu_region_en &= ~(1 << region);

    return HAL_MPU_STATUS_OK;
}


static uint8_t mpu_judge_memory_type(uint32_t start_addr, uint32_t end_addr)
{
    /* Follow the default memory map */
    static const uint32_t s_range_of_normal_memory[2][2] = {
        { 0x00000000, 0x3FFFFFFF },
        { 0x60000000, 0x9FFFFFFF }
    };
    uint32_t idx;
    for (idx = 0; idx < 2; idx++) {
        if ((start_addr >= s_range_of_normal_memory[idx][0]) &&
            (end_addr <= s_range_of_normal_memory[idx][1])) {
            return MPU_MAIR_NORMAL_MEMORY;
        }
    }
    return MPU_MAIR_DEVICE_MEMORY;
}

hal_mpu_status_t hal_mpu_region_configure(hal_mpu_region_t region, const hal_mpu_region_config_t *region_config)
{
    uint32_t reg;

    /* Region is invalid */
    if (region >= HAL_MPU_REGION_MAX) {
        assert(0);
        return HAL_MPU_STATUS_ERROR_REGION;
    }

    /* Parameter check */
    if (region_config == NULL) {
        assert(0);
        return HAL_MPU_STATUS_INVALID_PARAMETER;
    }

    /* The limit address must be greater than the base address. */
    if ((region_config->mpu_region_address & MPU_RBAR_BASE_Msk) >=
        (region_config->mpu_region_end_address & MPU_RLAR_LIMIT_Msk)) {
        assert(0);
        return HAL_MPU_STATUS_ERROR_REGION_ADDRESS;
    }

    /* select region */
    MPU->RNR  = region;
    /* Write the region setting to corresponding register */
    MPU->RBAR = ((region_config->mpu_region_address & MPU_RBAR_BASE_Msk) |
                 ((region_config->mpu_region_access_permission) << MPU_RBAR_AP_Pos));

    /* !!! NOTE !!!
     * The actual limit address is set to the value plus 32 in the Limit register.
     * !!! NOTE !!!
     */
    // MPU->RLAR = ((region_config->mpu_region_end_address & MPU_RLAR_LIMIT_Msk) - 1);
    reg = MPU->RLAR & (0x1F);
    reg |= ((region_config->mpu_region_end_address - 0x20) & MPU_RLAR_LIMIT_Msk);
    reg |= (mpu_judge_memory_type(region_config->mpu_region_address,
                                  region_config->mpu_region_end_address) << MPU_RLAR_AttrIndx_Pos);
    MPU->RLAR = reg;

    /* Set the XN(execution never) bit of RBAR if mpu_xn is true */
    if (region_config->mpu_xn == TRUE) {
        MPU->RBAR |= MPU_RBAR_XN_Msk;
    }

    /* Update the global variable */
    g_mpu_entry[region].mpu_rbar.w = MPU->RBAR;
    g_mpu_entry[region].mpu_rlar.w = MPU->RLAR;

    return HAL_MPU_STATUS_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* HAL_MPU_MODULE_ENABLED */

