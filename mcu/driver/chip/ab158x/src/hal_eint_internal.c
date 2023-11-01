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

/*************************************************************************
 * Included header files
 *************************************************************************/

#include "hal_eint.h"

#ifdef HAL_EINT_MODULE_ENABLED
#include "hal_eint_internal.h"
#include "hal_nvic_internal.h"
#include "hal_log.h"
#include "hal_gpt.h"
#include "memory_attribute.h"

#ifdef HAL_TIME_CHECK_ENABLED
#include "hal_time_check.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

ATTR_RWDATA_IN_TCM EINT_REGISTER_T *EINT_REGISTER = (EINT_REGISTER_T *)EINT_BASE;

eint_function_t eint_function_table[HAL_EINT_NUMBER_MAX];

ATTR_TEXT_IN_TCM uint32_t eint_get_status(uint32_t index)
{
    if (index > (HAL_EINT_NUMBER_MAX >> 5)) {
        return 0xFFFFFFFF;
    }
    return (EINT_REGISTER->EINT_STA[index]);
}

ATTR_TEXT_IN_TCM uint32_t eint_get_event(uint32_t index)
{
    if (index > (HAL_EINT_NUMBER_MAX >> 5)) {
        return 0xFFFFFFFF;
    }
    return (EINT_REGISTER->EINT_EEVT[index]);
}

ATTR_TEXT_IN_TCM void eint_ack_interrupt(uint32_t eint_number)
{
    uint32_t reg_index;
    uint32_t reg_shift;

    reg_index = eint_number / EINT_GROUP_MAX_NUMBER;
    reg_shift = 1 << (eint_number % EINT_GROUP_MAX_NUMBER);

    EINT_REGISTER->EINT_INTACK[reg_index] = reg_shift;
}

ATTR_TEXT_IN_TCM void eint_ack_wakeup_event(uint32_t eint_number)
{
    uint32_t reg_index;
    uint32_t reg_shift;

    reg_index = eint_number / EINT_GROUP_MAX_NUMBER;
    reg_shift = 1 << (eint_number % EINT_GROUP_MAX_NUMBER);

    EINT_REGISTER->EINT_EEVTACK[reg_index] = reg_shift;
}

uint32_t eint_caculate_debounce_time(uint32_t ms)
{
    uint32_t prescaler;
    uint32_t count;

    if (ms == 0) {
        /* set to one 32KHz clock cycle */
        prescaler = EINT_CON_PRESCALER_32KHZ;
        count = 0;
    } else if (ms <= 62) {
        prescaler = EINT_CON_PRESCALER_32KHZ;
        count = EINT_TIME_MS_TO_COUNT(ms, 32768);
    } else if (ms <= 125) {
        prescaler = EINT_CON_PRESCALER_16KHZ;
        count = EINT_TIME_MS_TO_COUNT(ms, 16384);
    } else if (ms <= 250) {
        prescaler = EINT_CON_PRESCALER_8KHZ;
        count = EINT_TIME_MS_TO_COUNT(ms, 8192);
    } else if (ms <= 500) {
        prescaler = EINT_CON_PRESCALER_4KHZ;
        count = EINT_TIME_MS_TO_COUNT(ms, 4096);
    } else if (ms <= 1000) {
        prescaler = EINT_CON_PRESCALER_2KHZ;
        count = EINT_TIME_MS_TO_COUNT(ms, 2048);
    } else if (ms <= 2000) {
        prescaler = EINT_CON_PRESCALER_1KHZ;
        count = EINT_TIME_MS_TO_COUNT(ms, 1024);
    } else if (ms <= 4000) {
        prescaler = EINT_CON_PRESCALER_512HZ;
        count = EINT_TIME_MS_TO_COUNT(ms, 512);
    } else if (ms <= 8000) {
        prescaler = EINT_CON_PRESCALER_256HZ;
        count = EINT_TIME_MS_TO_COUNT(ms, 256);
    } else {
        /* set to maximum prescaler/count */
        prescaler = EINT_CON_PRESCALER_256HZ;
        count = EINT_CON_DBC_CNT_MASK;
    }

    if (count > EINT_CON_DBC_CNT_MASK) {
        count = EINT_CON_DBC_CNT_MASK;
    }

    count = (count | EINT_CON_DBC_EN_MASK |
             (EINT_CON_PRESCALER_MASK & (prescaler << EINT_CON_PRESCALER_OFFSET)));
    return count;
}

ATTR_TEXT_IN_TCM void hal_eint_isr(hal_nvic_irq_t index)
{
    uint32_t i;
    uint32_t reg_index;
    uint32_t eint_index;
    uint32_t status;

    for (reg_index = 0; reg_index < 3; reg_index++) {
        status = EINT_REGISTER->EINT_STA[reg_index];
        for (i = 0; status; i++) {
            if (status & (1 << i)) {
                eint_index = (reg_index << 5)  + i;
                if (eint_function_table[eint_index].eint_callback) {
                    hal_eint_mask((hal_eint_number_t)eint_index);
                    eint_ack_interrupt(eint_index);
                    eint_ack_wakeup_event(eint_index);

                    eint_function_table[eint_index].eint_callback(eint_function_table[eint_index].user_data);
                } else {
                    //log_hal_msgid_error("ERROR: no EINT interrupt handler!\n", 0);
                }
                status &= ~(1 << i);
            }
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif /* HAL_EINT_MODULE_ENABLED */

