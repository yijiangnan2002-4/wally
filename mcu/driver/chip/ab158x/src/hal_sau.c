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

#include "hal_sau.h"

#ifdef HAL_SAU_MODULE_ENABLED

#include "hal_sau_internal.h"
#include "hal_nvic.h"
#include "hal_nvic_internal.h"
#include "assert.h"
#include "core_cm33.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SAU_BUSY 1
#define SAU_IDLE 0

static volatile uint8_t g_sau_status = SAU_IDLE;

hal_sau_status_t hal_sau_init(void)
{
    uint32_t irq_flag;
    uint32_t region;

    /* In order to prevent race condition, interrupt should be disabled when query and update global variable which indicates the module status */
    hal_nvic_save_and_set_interrupt_mask(&irq_flag);

    /* Check module status */
    if (g_sau_status == SAU_BUSY) {
        /* Restore the previous status of interrupt */
        hal_nvic_restore_interrupt_mask(irq_flag);

        return HAL_SAU_STATUS_ERROR_BUSY;
    } else {
        /* Change status to busy */
        g_sau_status = SAU_BUSY;

        /* Restore the previous status of interrupt */
        hal_nvic_restore_interrupt_mask(irq_flag);
    }
    /* Set TYPE register to indicate implemented regions */
    //SAU->TYPE = HAL_SAU_REGION_MAX;
    /* Set CTRL register to default value */
    SAU->CTRL = 0;

    /* Update the global variable */
    g_sau_ctrl.w = SAU->CTRL;
    g_sau_region_en = 0;

    /* Init global variable for each sau region */
    for (region = HAL_SAU_REGION_0; region < HAL_SAU_REGION_MAX; region++) {
        /* Initialize rbar to 0 for each region*/
        g_sau_entry[region].sau_rbar.w = 0;
        /* Initialize rlar to 0 for each region*/
        g_sau_entry[region].sau_rlar.w = 0;
    }

    return HAL_SAU_STATUS_OK;
}

hal_sau_status_t hal_sau_deinit(void)
{
    hal_sau_region_t region;

    /* Set CTRL register to default value */
    SAU->CTRL = 0;

    /* Update the global variable */
    g_sau_ctrl.w = 0;
    g_sau_region_en = 0;

    /* Reset SAU setting as well as global variables to default value */
    for (region = HAL_SAU_REGION_0; region < HAL_SAU_REGION_MAX; region++) {
        SAU->RNR  = region;
        SAU->RBAR = 0;
        SAU->RLAR = 0;

        /* Update the global variable */
        g_sau_entry[region].sau_rbar.w = 0;
        g_sau_entry[region].sau_rlar.w = 0;
    }

    /* Change status to idle */
    g_sau_status = SAU_IDLE;

    return HAL_SAU_STATUS_OK;
}

hal_sau_status_t hal_sau_enable(void)
{
    /* Enable SAU */
    SAU->CTRL |= SAU_CTRL_ENABLE_Msk;

    /* Force memory writes before continuing */
    __DSB();
    /* Flush and refill pipeline with updated permissions */
    __ISB();

    /* Update the global variable */
    g_sau_ctrl.w = SAU->CTRL;

    return HAL_SAU_STATUS_OK;
}

hal_sau_status_t hal_sau_disable(void)
{
    /* Disable SAU */
    SAU->CTRL &= ~SAU_CTRL_ENABLE_Msk;

    /* Update the global variable */
    g_sau_ctrl.w = SAU->CTRL;

    return HAL_SAU_STATUS_OK;
}

hal_sau_status_t hal_sau_region_enable(hal_sau_region_t region)
{
    /* Region is invalid */
    if (region >= HAL_SAU_REGION_MAX) {
        return HAL_SAU_STATUS_ERROR_REGION;
    }

    /* Enable corresponding region */
    SAU->RNR = (region << SAU_RNR_REGION_Pos);
    SAU->RLAR |= SAU_RLAR_ENABLE_Msk;

    /* Update the global variable */
    g_sau_entry[region].sau_rlar.w = SAU->RLAR;
    g_sau_region_en |= (1 << region);  //TODO

    return HAL_SAU_STATUS_OK;
}

hal_sau_status_t hal_sau_region_disable(hal_sau_region_t region)
{
    /* Region is invalid */
    if (region >= HAL_SAU_REGION_MAX) {
        return HAL_SAU_STATUS_ERROR_REGION;
    }

    /* Disable corresponding region */
    SAU->RNR = (region << SAU_RNR_REGION_Pos);
    SAU->RLAR &= ~SAU_RLAR_ENABLE_Msk;

    /* Update the global variable */
    g_sau_entry[region].sau_rlar.w = SAU->RLAR;
    g_sau_region_en &= ~(1 << region);  //TODO

    return HAL_SAU_STATUS_OK;
}

hal_sau_status_t hal_sau_region_configure(hal_sau_region_t region, const hal_sau_region_config_t *region_config)
{
    /* Region is invalid */
    if (region >= HAL_SAU_REGION_MAX) {
        return HAL_SAU_STATUS_ERROR_REGION;
    }

    /* Parameter check */
    if (region_config == NULL) {
        return HAL_SAU_STATUS_INVALID_PARAMETER;
    }

    /* The SAU region size is invalid if region size is less than 32 byte*/
    if ((region_config->sau_region_end_address - region_config->sau_region_start_address) <= 32) {
        return HAL_SAU_STATUS_ERROR_REGION_SIZE;
    }

    /* select region */
    SAU->RNR  = region;
    /* Write the region setting to corresponding register */
    SAU->RBAR = ((region_config->sau_region_start_address & SAU_RBAR_BADDR_Msk));
    SAU->RLAR = (region_config->sau_region_end_address & SAU_RLAR_LADDR_Msk);

    if (region_config->sau_non_secure_callable == true) {
        SAU->RLAR |= ((1U << SAU_RLAR_NSC_Pos) & SAU_RLAR_NSC_Msk);
    }

    /* Update the global variable */
    g_sau_entry[region].sau_rbar.w = SAU->RBAR ;
    g_sau_entry[region].sau_rlar.w = SAU->RLAR;

    return HAL_SAU_STATUS_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* HAL_SAU_MODULE_ENABLED */

