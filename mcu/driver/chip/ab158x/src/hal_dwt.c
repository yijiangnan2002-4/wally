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

#include "hal_dwt.h"

#ifdef HAL_DWT_MODULE_ENABLED
#include "hal_platform.h"
#include "core_cm33.h"
#include "hal_log.h"

//#define HAL_DWT_DEBUG

static int DWT_NUMCOMP;

/* reset all comparators' setting */
void hal_dwt_reset(void)
{
    DWT->COMP0 = 0;
    DWT->COMP1 = 0;
    DWT->COMP2 = 0;
    DWT->COMP3 = 0;
    DWT->FUNCTION0 = 0;
    DWT->FUNCTION1 = 0;
    DWT->FUNCTION2 = 0;
    DWT->FUNCTION3 = 0;
}
void hal_dwt_init(void)
{
    /* only enable hardware stack overflow check by the DWT when halting debug is disabled,
           because under halting-mode, the ICE will take over the DWT function.
           So the software stack overflow need to be checked by SW under halting-mode.
           The halting debug status can be checked by the bit(C_DEBUGEND), which is set when debugger is connected.
      */

    if (!(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)) {
        /* DWT reset*/
        hal_dwt_reset();

        /* enable debug monitor mode    */
        if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_MON_EN_Msk)) {
            CoreDebug->DEMCR |= (CoreDebug_DEMCR_MON_EN_Msk | CoreDebug_DEMCR_TRCENA_Msk) ;
        }

        DWT_NUMCOMP = DWT->CTRL >> DWT_CTRL_NUMCOMP_Pos;

#ifdef HAL_DWT_DEBUG
        log_hal_msgid_info(" DWT has %d comparators,\r\n ctrl register status 0x%lx,\r\n ", 2,
                           DWT_NUMCOMP, DWT->CTRL);
#endif /* HAL_DWT_DEBUG */
    }
}

#ifdef HAL_DWT_DEBUG
void hal_dwt_dump_status(void)
{
    log_hal_msgid_info("DHCSR:0x%lx, DEMCR:0x%lx \r\n", 2, CoreDebug->DHCSR, CoreDebug->DEMCR);
    log_hal_msgid_info("DWT_CTRL: 0x%lx \r\n", 1, DWT->CTRL);
    log_hal_msgid_info("COMP0: %8lx \t MASK0: %8lx \t FUNC0: %8lx \r\n", 3, DWT->COMP0,  DWT->FUNCTION0);
    log_hal_msgid_info("COMP1: %8lx \t MASK1: %8lx \t FUNC1: %8lx \r\n", 3, DWT->COMP1,  DWT->FUNCTION1);
    log_hal_msgid_info("COMP2: %8lx \t MASK2: %8lx \t FUNC2: %8lx \r\n", 3, DWT->COMP2,  DWT->FUNCTION2);
    log_hal_msgid_info("COMP3: %8lx \t MASK3: %8lx \t FUNC3: %8lx \r\n", 3, DWT->COMP3,  DWT->FUNCTION3);

}
#endif /* HAL_DWT_DEBUG */

/*
@param index:         comparator N, valid scope [0,DWT_NUMCOMP-1]
@param addr_base: address for data accesses or instruction fetches
@param addr_mask: the size of the ignore mask applied to address range matching
@param func:        which kind of compared accesses will generate watchpoint debug event
@return val:        status, -1: fail; 0:success
!! Note: the addr_base should be 2^(addr_mask) byte alignment, otherwise the behavior is UNPREDICTABLE !!
!! Note: only enable hardware stack overflow check by the DWT when halting debug is disabled, because under halting-mode, the ICE will take over the DWT function.
         The halting debug status can be checked by the bit(C_DEBUGEND), which is set when debugger is connected.
!! Note:
             Comparator 3 is used to check pxCurrentTCB stack
             Comparator 2/1/0 is reserved for future usage
*/
int32_t hal_dwt_request_watchpoint(uint32_t index, uint32_t addr_base, uint32_t addr_mask, DWT_FUNC_TYPE func)
{
	
    /* only enable hardware stack overflow check by the DWT when halting debug is disabled,
           because under halting-mode, the ICE will take over the DWT function.
           The SW will do stack overflow under halting-mode.
           The halting debug status can be checked by the bit(C_DEBUGEND), which is set when debugger is connected.
       */

	
    if (!(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)) {
        /* comparator N */
        if ((index >= DWT_NUMCOMP) || (addr_base & ((1 << addr_mask) - 1))) {

            return -1;
        }
		
		if (index >= HAL_DWT_MAX){
			return -1;
		}
		
		if (index == HAL_DWT_0){
			DWT->COMP0 = addr_base;
			DWT->FUNCTION0 = DWT->FUNCTION0 | (addr_mask << DWT_FUNCTION_DATAVSIZE_Pos) | (0x1 << DWT_FUNCTION_ACTION_Pos) | func ;
		}else if(index == HAL_DWT_1){
			DWT->COMP1 = addr_base;
			DWT->FUNCTION1 = DWT->FUNCTION1 | (addr_mask << DWT_FUNCTION_DATAVSIZE_Pos) | (0x1 << DWT_FUNCTION_ACTION_Pos) | func ;
		}else if (index == HAL_DWT_2){
		    DWT->COMP2 = addr_base;
			DWT->FUNCTION2 = DWT->FUNCTION2 | (addr_mask << DWT_FUNCTION_DATAVSIZE_Pos) | (0x1 << DWT_FUNCTION_ACTION_Pos) | func ;
		}else if (index == HAL_DWT_3){
			DWT->COMP3 = addr_base;
			DWT->FUNCTION3 = DWT->FUNCTION3 | (addr_mask << DWT_FUNCTION_DATAVSIZE_Pos) | (0x1 << DWT_FUNCTION_ACTION_Pos) | func ;
		}else{
			return -1;
		}
        return 0;
    } else {

        return -1;
    }


}
/*
@param index:       comparator N, valid scope [0,DWT_NUMCOMP-1]
@param is_enable:   0 means disable the watch point, and 1 means enable the watch point
@return val:        status, -1: fail; 0:success
*/
int32_t hal_dwt_control_watchpoint(uint32_t index, bool is_enable)
{

    if (!(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)) {
        /* comparator N */
        if (index >= DWT_NUMCOMP) {
            return -1;

        } else {
            //uint32_t offset;
            //offset = (0x10 * index) / 4;
            if (is_enable == 1) {
                //*(&DWT->FUNCTION0 + offset) |= DWT_FUNCTION_FUNCTION_Msk;
            } else {
                // *(&DWT->FUNCTION0 + offset) &= ~DWT_FUNCTION_FUNCTION_Msk;
            }

            return 0;
        }
    } else {
        return -1;
    }
}

#endif /* HAL_DWT_MODULE_ENABLED */
