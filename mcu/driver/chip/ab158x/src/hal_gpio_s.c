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

#include "hal_gpio.h"


#ifdef HAL_GPIO_MODULE_ENABLED
#include "hal_gpio_internal.h"
#include "hal_nvic.h"
#include "memory_attribute.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef AIR_CPU_IN_SECURITY_MODE
ATTR_TEXT_IN_TCM hal_gpio_status_t hal_gpio_set_secure_state(hal_gpio_pin_t gpio_pin)
{
    uint32_t mask;
    uint32_t reg_index;
    uint32_t reg_shift;

    if (gpio_pin > HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }
    reg_index = gpio_pin / 32;
    reg_shift = 1 << (gpio_pin % 32);

    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (reg_index > 0) {
        GPIO_SECURE_B_0_SET = reg_shift;
    } else {
        GPIO_SECURE_B_1_SET = reg_shift;
    }
    hal_nvic_restore_interrupt_mask(mask);

    return HAL_GPIO_STATUS_OK;
}

ATTR_TEXT_IN_TCM hal_gpio_status_t hal_gpio_get_secure_state(hal_gpio_pin_t gpio_pin, bool *secure_state)
{
    uint32_t mask;
    uint32_t reg_index;
    uint32_t reg_shift;

    if (gpio_pin > HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (!secure_state) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    reg_index = gpio_pin / 32;
    reg_shift = 1 << (gpio_pin % 32);

    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (reg_index > 0) {
        *secure_state = (GPIO_SECURE_B_0 == reg_shift);
    } else {
        *secure_state = (GPIO_SECURE_B_1 == reg_shift);
    }
    hal_nvic_restore_interrupt_mask(mask);

    return HAL_GPIO_STATUS_OK;
}

ATTR_TEXT_IN_TCM hal_gpio_status_t hal_gpio_clear_secure_state(hal_gpio_pin_t gpio_pin)
{
    uint32_t mask;
    uint32_t reg_index;
    uint32_t reg_shift;

    if (gpio_pin > HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }
    reg_index = gpio_pin / 32;
    reg_shift = 1 << (gpio_pin % 32);

    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (reg_index > 0) {
        GPIO_SECURE_B_0_CLR = reg_shift;
    } else {
        GPIO_SECURE_B_1_CLR = reg_shift;
    }
    hal_nvic_restore_interrupt_mask(mask);

    return HAL_GPIO_STATUS_OK;
}
#endif
#ifdef __cplusplus
}
#endif

#endif  /* HAL_GPIO_MODULE_ENABLED */

