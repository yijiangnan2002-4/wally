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
#include "hal_log.h"
#include "hal_gpt_internal.h"
#include "hal_clock_internal.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_TIME_CHECK_ENABLED

ATTR_ZIDATA_IN_TCM bool is_time_check_assert_enabled;

void hal_time_check_assert_enable(void)
{
    uint32_t temp_counter;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &temp_counter);

    if (is_time_check_assert_enabled == false) {
        is_time_check_assert_enabled = true;
    }
}

void hal_time_check_assert_disable(void)
{
    is_time_check_assert_enabled = false;
}

#endif  /* HAL_TIME_CHECK_ENABLED */

#ifdef AIR_CPU_MCPS_PRIORING_ENABLE
ATTR_ZIDATA_IN_TCM static uint32_t irq_timestamp_start = 0;
ATTR_ZIDATA_IN_TCM volatile static uint32_t irq_nest_flag = 0;
ATTR_ZIDATA_IN_TCM static uint32_t irq_timestamp_end = 0;
ATTR_ZIDATA_IN_TCM  uint32_t irq_total_time_us = 0;
#endif 

ATTR_TEXT_IN_TCM void irq_excute_time_start()
{
#ifdef AIR_CPU_MCPS_PRIORING_ENABLE
    uint32_t mask = __get_BASEPRI();
    /* disable all irq to protect irq_nest_flag */
    __set_BASEPRI(((1 << (8 - __NVIC_PRIO_BITS)) & 0xFF));
    __DMB();
    __ISB();

    /* >>case-1
     * t0
     * callback --> higher priority irq preempt
     * t1
     * >>case-2
     * t0
     * callback
     * t1
     * --> higher priority irq preempt
    */
    if(irq_nest_flag == FALSE){
        irq_timestamp_start = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
        irq_nest_flag = TRUE;
     }
    /* enable all IRQ */
    __set_BASEPRI(mask);
    __DMB();
    __ISB();
#endif
}

ATTR_TEXT_IN_TCM void irq_excute_time_end()
{
#ifdef AIR_CPU_MCPS_PRIORING_ENABLE
    uint32_t mask = __get_BASEPRI();
    /* disable all irq to make protect irq_nest_flag */
    __set_BASEPRI(((1 << (8 - __NVIC_PRIO_BITS)) & 0xFF));
    __DMB();
    __ISB();

	if (irq_nest_flag == TRUE){
		irq_timestamp_end = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
		irq_total_time_us += irq_timestamp_end - irq_timestamp_start;
		irq_nest_flag = FALSE;		
	}
    /* enable all IRQ */
    __set_BASEPRI(mask);
    __DMB();
    __ISB();
#endif
}




#ifdef __cplusplus
}
#endif




