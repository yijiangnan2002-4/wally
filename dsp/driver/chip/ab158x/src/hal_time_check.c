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

#include "hal_time_check.h"
#include "hal.h"
#include "assert.h"
#include "memory_attribute.h"
#include "hal_gpt_internal.h"
#include "hal_clock_internal.h"
#include <xtensa/xtruntime.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_TIME_CHECK_ENABLED

ATTR_ZIDATA_IN_DRAM bool is_time_check_assert_enabled;

#ifdef HAL_TIME_CHECK_ISR_ENABLED
#define MAX_INTERRUPT_LEVEL 4
#ifdef AIR_CPU_MCPS_PRIORING_ENABLE
ATTR_ZIDATA_IN_DRAM_4BYTE_ALIGN uint32_t irq_execution_time_us[MAX_INTERRUPT_LEVEL];
extern uint32_t irq_interrupt_time_us;
uint32_t total_irq_interrupt_time_us = 0;;
#endif

typedef struct {
    uint32_t irq_number;
    uint32_t execution_time;
    void    *callback;          //recording the callback for debugging
} time_check_t;

ATTR_ZIDATA_IN_DRAM_4BYTE_ALIGN time_check_t time_check_buffer[MAX_INTERRUPT_LEVEL];
ATTR_ZIDATA_IN_DRAM_4BYTE_ALIGN int32_t  time_check_buffer_index;
ATTR_ZIDATA_IN_DRAM_4BYTE_ALIGN uint32_t last_timestamp;
ATTR_ZIDATA_IN_DRAM_4BYTE_ALIGN uint32_t last_linkAddress;
ATTR_ZIDATA_IN_DRAM_4BYTE_ALIGN uint8_t  isTimerUsing;

#ifdef TIME_CHECK_ISR_EXCUTION_TIME_DEBUG_ENABLE
typedef struct {
    uint32_t callback_addr;
    uint32_t excution_time;
    uint32_t irq_num;
    uint32_t cur_time;
} hal_isr_excution_time_debug_t;

#define HAL_ISR_EXCUTION_TIME_DEBUG_NUM (20)
ATTR_ZIDATA_IN_DRAM hal_isr_excution_time_debug_t isr_excution_time_debug[HAL_ISR_EXCUTION_TIME_DEBUG_NUM];
ATTR_ZIDATA_IN_DRAM uint32_t isr_debug_cur_index;

ATTR_TEXT_IN_IRAM void hal_isr_excution_time_debug_log_save(uint32_t callback_addr, uint32_t time, uint32_t irq_num)
{
    uint32_t mask = 0;

    mask = (uint32_t)XTOS_SET_INTLEVEL(4);
    if (irq_num != OS_GPT_IRQn) {
        isr_excution_time_debug[isr_debug_cur_index].callback_addr = callback_addr;
        isr_excution_time_debug[isr_debug_cur_index].excution_time = time;
        isr_excution_time_debug[isr_debug_cur_index].irq_num = irq_num;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(isr_excution_time_debug[isr_debug_cur_index].cur_time));
        isr_debug_cur_index ++;
        isr_debug_cur_index = (isr_debug_cur_index < HAL_ISR_EXCUTION_TIME_DEBUG_NUM) ? isr_debug_cur_index : 0;
    }
    XTOS_RESTORE_INTLEVEL(mask);
}
#endif

ATTR_TEXT_IN_IRAM void time_check_start(uint32_t irq_number)
{
    uint32_t current_timestamp;
    uint32_t temp_duration_us;
    uint32_t mask;

    // if (is_time_check_assert_enabled == false) {
    //     return;
    // }

    mask = (uint32_t)XTOS_SET_INTLEVEL(4);

    current_timestamp = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
    if (time_check_buffer_index != -1) { // nested
        temp_duration_us = current_timestamp - last_timestamp;
        time_check_buffer[time_check_buffer_index].execution_time += temp_duration_us;
    }
    time_check_buffer_index++;
    if(time_check_buffer_index >= MAX_INTERRUPT_LEVEL) {
        assert(0);
    }
    time_check_buffer[time_check_buffer_index].execution_time = 0;
    time_check_buffer[time_check_buffer_index].irq_number = irq_number;
    last_timestamp = current_timestamp;

    XTOS_RESTORE_INTLEVEL(mask);

    return ;
}

ATTR_TEXT_IN_IRAM void time_check_end(uint32_t irq_number, uint32_t limter_us, void *callback)
{
    uint32_t current_timestamp;
    uint32_t temp_duration_us;
    uint32_t mask;

    // if (is_time_check_assert_enabled == false) {
    //     return;
    // }

    if ((time_check_buffer_index < 0) || (time_check_buffer_index >= MAX_INTERRUPT_LEVEL)) {
        log_hal_msgid_info("time_check_buffer_index=%d, irq_number = %d \r\n", 2, time_check_buffer_index, irq_number);
        assert(0);
        return;
    }

    if (time_check_buffer[time_check_buffer_index].irq_number != irq_number) {
        log_hal_msgid_info("time_check_buffer_index=%d, irq_number = %d not match %d\r\n", 3, time_check_buffer_index, time_check_buffer[time_check_buffer_index].irq_number, irq_number);
        assert(0);
        return;
    }

    mask = (uint32_t)XTOS_SET_INTLEVEL(4);

    current_timestamp = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
    temp_duration_us = current_timestamp - last_timestamp;
    temp_duration_us += time_check_buffer[time_check_buffer_index].execution_time ;

    last_timestamp    = current_timestamp;

#ifdef AIR_CPU_MCPS_PRIORING_ENABLE
    irq_execution_time_us[time_check_buffer_index] += temp_duration_us;
	/* irq_interrupt_time_us will be cleared every context switch */
    irq_interrupt_time_us += temp_duration_us;
	/* total_irq_interrupt_time_us will be cleared every task profile */
	total_irq_interrupt_time_us +=temp_duration_us;
#endif /* AIR_CPU_MCPS_PRIORING_ENABLE */

    time_check_buffer_index--;
    XTOS_RESTORE_INTLEVEL(mask);
#ifdef TIME_CHECK_ISR_EXCUTION_TIME_DEBUG_ENABLE
    hal_isr_excution_time_debug_log_save((uint32_t)callback, temp_duration_us, irq_number);
#endif
    if (temp_duration_us >= limter_us) {
        //if time over than limter_us, recording the callback user
        time_check_buffer[time_check_buffer_index + 1].callback = callback;
        log_hal_msgid_error("ERROR!!!!!!The IRQ%d execute time too long:%d us, callback addr:0x%x\r\n", 3, (int)irq_number, (int)temp_duration_us, (unsigned int)callback);

#ifdef ENABLE_IRQ_HANDLER_CHECK_ASSERT
#ifdef AIR_ICE_DEBUG_ENABLE
        if (hal_ice_debug_is_enabled() == false) {
            assert(0);
        }
#else
        assert(0);
#endif
#endif
    }
}

#endif

// #ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
// ATTR_ZIDATA_IN_DRAM uint32_t time_check_disbale_irq_start;

// typedef struct {
//     uint32_t callback_addr;
//     uint32_t mask_irq_time;
//     uint32_t cur_time;
// } hal_mask_irq_debug_t;

// #define HAL_MASK_IRQ_DEBUG_NUM (20)
// ATTR_ZIDATA_IN_DRAM hal_mask_irq_debug_t mask_irq_debug[HAL_MASK_IRQ_DEBUG_NUM];
// ATTR_ZIDATA_IN_DRAM uint32_t cur_index = 0;

// ATTR_TEXT_IN_IRAM void hal_mask_irq_debug_log_save(uint32_t callback_addr, uint32_t time)
// {
//     mask_irq_debug[cur_index].callback_addr = callback_addr;
//     mask_irq_debug[cur_index].mask_irq_time = time;
//     hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(mask_irq_debug[cur_index].cur_time));
//     cur_index ++;
//     cur_index = (cur_index < HAL_MASK_IRQ_DEBUG_NUM) ? cur_index : 0;
// }

// ATTR_TEXT_IN_IRAM void hal_time_check_disable_irq_start(uint32_t linkAddress)
// {
//     if (is_time_check_assert_enabled == false) {
//         return ;
//     }

//     //hal_dwt_request_watchpoint(HAL_DWT_0, 0x0, 0x2, WDE_DATA_WO);
//     time_check_disbale_irq_start = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
//    // hal_dwt_request_watchpoint(HAL_DWT_0, &time_check_disbale_irq_start, 0x2, WDE_DATA_WO);

//     isTimerUsing     = TRUE;
//     last_linkAddress = linkAddress;
//     return;
// }

// ATTR_TEXT_IN_IRAM bool hal_time_check_disable_irq_end(uint32_t limter_us, uint32_t linkAddress)
// {
//     uint32_t temp_time_end, temp_duration_us;
//     if (is_time_check_assert_enabled == false) {
//         return true ;
//     }

//     isTimerUsing = FALSE;

//     temp_time_end    = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
//     temp_duration_us = temp_time_end - time_check_disbale_irq_start;

//     hal_mask_irq_debug_log_save(linkAddress, temp_duration_us);
//     if ((limter_us != 0xffffffff) && (temp_duration_us >= limter_us)) {
//         log_hal_msgid_info("#####disable irq time = %d linkAddress = 0x%x \r\n", 2, (int)temp_duration_us, linkAddress);
// #ifdef ENABLE_REAL_TIME_CHECK_ASSERT
// #ifdef AIR_ICE_DEBUG_ENABLE
//         if (hal_ice_debug_is_enabled() == false) {
//             assert(0);
//         }
// #else
//         assert(0);
// #endif
// #endif
//         return false;
//     }

//     return true;
// }


// #endif

void hal_time_check_assert_enable(void)
{
    uint32_t temp_counter;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &temp_counter);

    if (is_time_check_assert_enabled == false) {
#ifdef HAL_TIME_CHECK_ISR_ENABLED
        time_check_buffer_index = -1;
        time_check_buffer[0].execution_time = 0;
#endif
        is_time_check_assert_enabled = true;
    }
}

void hal_time_check_assert_disable(void)
{
    is_time_check_assert_enabled = false;
}




#ifdef __cplusplus
}
#endif

#endif /* HAL_TIME_CHECK_ENABLED */


