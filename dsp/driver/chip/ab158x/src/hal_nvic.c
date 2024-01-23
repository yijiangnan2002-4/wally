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

#include "hal_nvic.h"

#ifdef HAL_NVIC_MODULE_ENABLED
#include "hal_nvic_internal.h"
#include "hal_gpt_internal.h"
#include "assert.h"
//#include "hal_flash_disk_internal.h"
#include "memory_attribute.h"
// #include "hal_core_status.h"
#include "hal_resource_assignment.h"
#include "hal_log.h"
#include <xtensa/hal.h>
#include <xtensa/xtruntime.h>
#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif /* MTK_SWLA_ENABLE */
#ifdef HAL_TIME_CHECK_ENABLED
#include "hal_time_check.h"
#endif
#ifdef AIR_ICE_DEBUG_ENABLE
#include "hal_ice_debug.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    void (*nvic_callback)(hal_nvic_irq_t irq_number);
} nvic_function_t;

typedef struct{
    volatile uint32_t dsp_in_critical_flag;
    volatile uint32_t flash_busy_flag;
}hal_flash_irq_status_t;

ATTR_ZIDATA_IN_DRAM nvic_function_t nvic_function_table[IRQ_NUMBER_MAX];

ATTR_RWDATA_IN_DRAM hal_flash_irq_status_t * flash_irq_status = (hal_flash_irq_status_t*)HW_SYSRAM_PRIVATE_MEMORY_FLASH_IRQ_STATUS_START;

hal_nvic_status_t hal_nvic_init(void)
{
    static bool init = false;
    uint32_t intnum;

    if (init == false) {
        for (intnum = 0; intnum < IRQ_NUMBER_MAX; intnum++) {
            xthal_interrupt_clear(intnum);
        }
        init = true;
    }

    nvic_irq_execution_number = -1;

    return HAL_NVIC_STATUS_OK;
}

hal_nvic_status_t hal_nvic_enable_irq(hal_nvic_irq_t irq_number)
{
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;

    if (irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
        return status;
    } else {
        xthal_interrupt_enable(irq_number);
        status = HAL_NVIC_STATUS_OK;
    }

    return status;
}

hal_nvic_status_t hal_nvic_disable_irq(hal_nvic_irq_t irq_number)
{
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;

    if (irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
        return status;
    } else {
        xthal_interrupt_disable(irq_number);
        status = HAL_NVIC_STATUS_OK;
    }

    return status;
}


uint32_t hal_nvic_get_pending_irq(hal_nvic_irq_t irq_number)
{
    uint32_t ret = 0xFF;

    if (irq_number < IRQ_NUMBER_MAX) {
        return (xthal_get_interrupt() & (1 << irq_number)) ? 1 : 0;
    }

    return ret;
}

/*
hal_nvic_status_t hal_nvic_set_pending_irq(hal_nvic_irq_t irq_number)
{
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
        return status;
    } else {
        NVIC_SetPendingIRQ(irq_number);
        status = HAL_NVIC_STATUS_OK;
    }

    return status;
}

hal_nvic_status_t hal_nvic_clear_pending_irq(hal_nvic_irq_t irq_number)
{
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
        return status;
    } else {
        NVIC_ClearPendingIRQ(irq_number);
        status = HAL_NVIC_STATUS_OK;
    }

    return status;
}

hal_nvic_status_t hal_nvic_set_priority(hal_nvic_irq_t irq_number, uint32_t priority)
{
    hal_nvic_status_t status = HAL_NVIC_STATUS_ERROR;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        status = HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
        return status;
    } else {
        NVIC_SetPriority(irq_number, priority);
        status = HAL_NVIC_STATUS_OK;
    }

    return status;
}

uint32_t hal_nvic_get_priority(hal_nvic_irq_t irq_number)
{
    uint32_t ret = 0xff;

    if (irq_number < (hal_nvic_irq_t)0 || irq_number >= IRQ_NUMBER_MAX) {
        return ret;
    } else {
        ret = NVIC_GetPriority(irq_number);
    }

    return ret;
}

*/
#include "xtensa_context.h"
uint64_t back_and_restore_buffer[(XT_STK_FRMSZ+XT_CP_SIZE)/8+1];
uint32_t back_cpvalue;

ATTR_TEXT_IN_IRAM void backup_function_for_special_irq(void)
{

    __asm__ __volatile__(
        "movi a3 ,back_cpvalue\n"
        "rsr  a4, CPENABLE \n"
        "s32i a4, a3, 0 \n"
        "movi a3, 0x3 \n"
        "or  a4, a4, a3 \n"
        "wsr  a4, CPENABLE\n"
        "rsync \n"
        "movi   a2, back_and_restore_buffer \n"
        "ae_s64.i	aed0, a2, 0              \n "
        "addi	a2, a2, 8                    \n"
        "ae_s64.i	aed1, a2, 0 \n"
        "ae_s64.i	aed2, a2, 8 \n"
        "ae_s64.i	aed3, a2, 16 \n"
        "ae_s64.i	aed4, a2, 24 \n"
        "ae_s64.i	aed5, a2, 32 \n"
        "ae_s64.i	aed6, a2, 40 \n"
        "ae_s64.i	aed7, a2, 48 \n"
        "ae_s64.i	aed8, a2, 56 \n"
        "addi	a2, a2, 64 \n"
        "ae_s64.i	aed9, a2, 0 \n"
        "ae_s64.i	aed10, a2, 8 \n"
        "ae_s64.i	aed11, a2, 16 \n"
        "ae_s64.i	aed12, a2, 24 \n"
        "ae_s64.i	aed13, a2, 32 \n"
        "ae_s64.i	aed14, a2, 40 \n"
        "ae_s64.i	aed15, a2, 48 \n"
        "ae_s64.i	aed16, a2, 56 \n"
        " addi	a2, a2, 64\n"
        " ae_s64.i	aed17, a2, 0\n"
        " ae_s64.i	aed18, a2, 8\n"
        " ae_s64.i	aed19, a2, 16\n"
        " ae_s64.i	aed20, a2, 24\n"
        " ae_s64.i	aed21, a2, 32\n"
        " ae_s64.i	aed22, a2, 40\n"
        " ae_s64.i	aed23, a2, 48\n"
        " ae_s64.i	aed24, a2, 56\n"
        " addi	a2, a2, 64\n"
        " ae_s64.i	aed25, a2, 0; \n"
        " ae_s64.i	aed26, a2, 8; \n"
        " ae_s64.i	aed27, a2, 16\n"
        " ae_s64.i	aed28, a2, 24\n"
        " ae_s64.i	aed29, a2, 32\n"
        " ae_s64.i	aed30, a2, 40\n"
        " ae_s64.i	aed31, a2, 48\n"
        "ae_movae	a3, aep0 \n"
        " s8i	a3, a2, 56\n"
        " ae_movae	a3, aep1\n"
        " s8i	a3, a2, 57\n"
        " ae_movae	a3, aep2\n"
        " s8i	a3, a2, 58\n"
        " ae_movae	a3, aep3\n"
        " s8i	a3, a2, 59\n"
        " addi	a2, a2, 64\n"

        " addi	a2, a2, 8\n"

        " ae_salign128.i	u0, a2, 0\n"
        " ae_salign128.i	u1, a2, 16\n"
        " ae_salign128.i	u2, a2, 32\n"
        " ae_salign128.i	u3, a2, 48\n"
        "addi	a2, a2, 64\n"
        "ae_movdrzbvc	aed0\n"
        "ae_s64.i	aed0, a2, 0\n"
        "ae_movvfcrfsr	aed0\n"
        "ae_s64.i	aed0, a2, 8\n"
        "rur.ae_ovf_sar	a3\n"
        "s32i.n	a3, a2, 16\n"
        "rur.ae_bithead	a3\n"
        "s32i.n	a3, a2, 20\n"
        "rur.ae_ts_fts_bu_bp	a3\n"
        "s32i.n	a3, a2, 24\n"
        "rur.ae_cw_sd_no	a3\n"
        "s32i.n	a3, a2, 28\n"
        "rur.ae_cbegin0	a3\n"
        "s32i.n	a3, a2, 32\n"
        "rur.ae_cend0	a3\n"
        "s32i.n	a3, a2, 36\n"
        "rur.ae_cbegin1	a3\n"
        "s32i.n	a3, a2, 40\n"
        "rur.ae_cend1	a3\n"
        "s32i.n	a3, a2, 44\n"
        "rur.ae_cbegin2	a3\n"
        "s32i.n	a3, a2, 48\n"
        "rur.ae_cend2	a3\n"
        "s32i.n	a3, a2, 52\n"
        );

}

ATTR_TEXT_IN_IRAM void restore_function_for_special_irq(void)
{
       __asm__ __volatile__(
       "movi   a2, back_and_restore_buffer  \n"

       "addi	a2, a2, 8  \n"
       " ae_l64.i	aed1, a2, 0  \n"
       " ae_l64.i	aed2, a2, 8 \n"
       " ae_l64.i	aed3, a2, 16 \n"
       " ae_l64.i	aed4, a2, 24 \n"
       " ae_l64.i	aed5, a2, 32 \n"
       " ae_l64.i	aed6, a2, 40 \n"
       " ae_l64.i	aed7, a2, 48 \n"
       " ae_l64.i	aed8, a2, 56 \n"
       " addi	a2, a2, 64 \n"
       " ae_l64.i	aed9, a2, 0;  \n"
       " ae_l64.i	aed10, a2, 8 \n"
       " ae_l64.i	aed11, a2, 16 \n"
       " ae_l64.i	aed12, a2, 24 \n"
       " ae_l64.i	aed13, a2, 32 \n"
       " ae_l64.i	aed14, a2, 40 \n"
       " ae_l64.i	aed15, a2, 48 \n"
       " ae_l64.i	aed16, a2, 56 \n"
       " addi	a2, a2, 64 \n"
       " ae_l64.i	aed17, a2, 0; \n"
       " ae_l64.i	aed18, a2, 8; \n"
       " ae_l64.i	aed19, a2, 16 \n"
       " ae_l64.i	aed20, a2, 24 \n"
       " ae_l64.i	aed21, a2, 32 \n"
       " ae_l64.i	aed22, a2, 40 \n"
       " ae_l64.i	aed23, a2, 48 \n"
       " ae_l64.i	aed24, a2, 56 \n"
       " addi	a2, a2, 64 \n"
       " ae_l64.i	aed25, a2, 0; \n"
       " ae_l64.i	aed26, a2, 8; \n"
       " ae_l64.i	aed27, a2, 16 \n"
       " ae_l64.i	aed28, a2, 24 \n"
       " ae_l64.i	aed29, a2, 32 \n"
       " ae_l64.i	aed30, a2, 40 \n"
       " ae_l64.i	aed31, a2, 48 \n"
       " addi	a2, a2, 56 \n"
       " l8ui	a3, a2, 0 \n"
       " ae_movea	aep0, a3 \n"
       " l8ui	a3, a2, 1 \n"
       " ae_movea	aep1, a3 \n"
       " l8ui	a3, a2, 2 \n"
       " ae_movea	aep2, a3 \n"
       " l8ui	a3, a2, 3 \n"
       " ae_movea	aep3, a3 \n"

       " addi.n	a2, a2, 16 \n"

       " ae_lalign128.i	u0, a2, 0 \n"
       " ae_lalign128.i	u1, a2, 16 \n"
       " ae_lalign128.i	u2, a2, 32 \n"
       " ae_lalign128.i	u3, a2, 48 \n"
       " addi	a2, a2, 64 \n"

       " ae_l64.i	aed0, a2, 0 \n"
       "ae_movzbvcdr	aed0 \n"
       "ae_l64.i	aed0, a2, 8 \n"
       "ae_movfcrfsrv	aed0 \n"

       "movi   a4, back_and_restore_buffer  \n"
       "ae_l64.i	aed0, a4, 0  \n"

       " l32i.n	a3, a2, 16 \n"
       " wur.ae_ovf_sar	a3 \n"
       " l32i.n	a3, a2, 20 \n"
       " wur.ae_bithead	a3 \n"
       " l32i.n	a3, a2, 24 \n"
       " wur.ae_ts_fts_bu_bp	a3 \n"
       "l32i.n	a3, a2, 28 \n"
       "wur.ae_cw_sd_no	a3 \n"
       "l32i.n	a3, a2, 32 \n"
       "wur.ae_cbegin0	a3 \n"
       "l32i.n	a3, a2, 36 \n"
       "wur.ae_cend0	a3 \n"
       "l32i.n	a3, a2, 40 \n"
       "wur.ae_cbegin1	a3 \n"
       "l32i.n	a3, a2, 44 \n"
       "wur.ae_cend1	a3 \n"
       "l32i.n	a3, a2, 48 \n"
       "wur.ae_cbegin2	a3 \n"
       "l32i.n	a3, a2, 52 \n"
       "wur.ae_cend2	a3 \n"
       "movi a3 ,back_cpvalue \n"
       "l32i a3, a3, 0 \n"
       "wsr.cpenable a3 \n"
       "rsync \n"
       );

}




ATTR_TEXT_IN_IRAM void isrC_main()
{
    uint32_t irq_state;
    uint32_t i;
    uint32_t int_en;
    uint32_t shift;
    uint32_t int_level;
    uint32_t backup;
    uint32_t backup_int_level = 0;
    const  uint32_t sw_irq[4][2] = {{I2S_SLAVE_IRQn, RGU_IRQn},
        {MCU_DMA1_IRQn, BT_AUTX_IRQn},
        {ANC_IRQn, CM33_IRQn},
        {0, BT_PLAY_EN_IRQn}
    };

    backup    = nvic_irq_execution_number;
    irq_state = xthal_get_interrupt();
    int_en    = xthal_get_intenable();
    int_level = (uint32_t)XTOS_SET_INTLEVEL(4); // to get intlevel
    XTOS_RESTORE_INTLEVEL(int_level);           // restore intlevel

    int_level = int_level & 0xF;
    backup_int_level = int_level;  // backup current irq level

    for (i = sw_irq[int_level - 1][0]; i <= sw_irq[int_level - 1][1]; i++) {
        shift = 1 << i;
        nvic_irq_execution_number = i;
        if ((irq_state & shift) && (int_en & shift)) {
            if (nvic_function_table[i].nvic_callback != NULL) {
#ifdef MTK_SWLA_ENABLE
                SLA_RamLogging((uint32_t)(IRQ_START | i));
#endif /* MTK_SWLA_ENABLE */
                xthal_interrupt_clear(i);
#ifdef HAL_TIME_CHECK_ENABLED
                time_check_start(i);
#endif
                if(i == 8)
                {
                backup_function_for_special_irq();
                }
                nvic_function_table[i].nvic_callback((hal_nvic_irq_t)i);
                if(i == 8)
                {
                restore_function_for_special_irq();
                }
#ifdef HAL_TIME_CHECK_ENABLED
                time_check_end(i, TIME_CHECK_ISR_TIME, nvic_function_table[i].nvic_callback);
#endif
#ifdef MTK_SWLA_ENABLE
                SLA_RamLogging((uint32_t)IRQ_END);
#endif /* MTK_SWLA_ENABLE */
            } else {
                log_hal_msgid_info("irq[%d] has no callback!!!\r\n", 1, (int)i);
            }
        }
    }
    nvic_irq_execution_number = backup;

    /* Check if the off interrupt is paired  */
    int_level = (uint32_t)XTOS_SET_INTLEVEL(4); // to get intlevel
    XTOS_RESTORE_INTLEVEL(int_level);           // restore intlevel
    int_level = int_level & 0xF;
    if(int_level != backup_int_level) {
        log_hal_msgid_error("backup_int_level = %d, int_level = %d \r\n", 2, backup_int_level, int_level);
        hal_gpt_delay_ms(10);
        assert(0);
    }
}

hal_nvic_status_t hal_nvic_register_isr_handler(hal_nvic_irq_t irq_number, hal_nvic_isr_t callback)
{
    if (irq_number >= IRQ_NUMBER_MAX || callback == NULL) {
        return HAL_NVIC_STATUS_INVALID_PARAMETER;
    }

    xthal_interrupt_clear(irq_number);
    nvic_function_table[irq_number].nvic_callback = callback;

    return HAL_NVIC_STATUS_OK;
}


hal_nvic_status_t hal_nvic_irq_software_trigger(hal_nvic_irq_t irq_number)
{
    if (irq_number >= IRQ_NUMBER_MAX) {
        return HAL_NVIC_STATUS_ERROR_IRQ_NUMBER;
    }

    nvic_irq_software_trigger(irq_number);

    return HAL_NVIC_STATUS_OK;
}

#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
ATTR_ZIDATA_IN_DRAM uint32_t hal_time_check_disbale_irq_start;
ATTR_ZIDATA_IN_DRAM uint32_t hal_disable_irq_linkAddress;
ATTR_ZIDATA_IN_DRAM uint32_t hal_time_check_disbale_irq_start_special;
ATTR_ZIDATA_IN_DRAM uint32_t hal_disable_irq_linkAddress_special;
#endif

extern bool is_time_check_assert_enabled;

ATTR_TEXT_IN_IRAM hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask(uint32_t *mask)
{
    *mask = (uint32_t)XTOS_SET_INTLEVEL(4);

#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    if ((*mask & 0xf) == 0) {
        flash_irq_status->dsp_in_critical_flag = (0xDEAD0000 | 0x1);
        hal_time_check_disbale_irq_start = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
        hal_disable_irq_linkAddress = (uint32_t)__builtin_return_address(0);
    }
#endif

    return HAL_NVIC_STATUS_OK;
}

ATTR_TEXT_IN_IRAM hal_nvic_status_t hal_nvic_restore_interrupt_mask(uint32_t mask)
{
#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    uint32_t backup_time_start = 0, backup_time_end = 0, backup_duration_us = 0, backup_linkAddress = 0, backup_critical_flag;
    uint32_t check_assert_flag = TRUE;
    uint32_t int_level;

    int_level = (uint32_t)XTOS_SET_INTLEVEL(4);
    XTOS_RESTORE_INTLEVEL(int_level);           // restore intlevel

    if (((mask & 0xF) == 0) && ((int_level & 0xF) == 4)) {
        backup_time_end    = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
        backup_time_start = hal_time_check_disbale_irq_start;
        backup_duration_us = backup_time_end - backup_time_start;
        backup_linkAddress = hal_disable_irq_linkAddress;
        hal_disable_irq_linkAddress = 0x0;
        backup_critical_flag = flash_irq_status->dsp_in_critical_flag;
        flash_irq_status->dsp_in_critical_flag = (0xDEAD0000 | 0x0);
        if (flash_irq_status->flash_busy_flag == 0xDEAD0001){
            check_assert_flag = FALSE;
            flash_irq_status->flash_busy_flag = (0xDEAD0000 | 0x0);
        }
    }
#endif

    XTOS_RESTORE_INTLEVEL(mask);

#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    if((backup_duration_us > TIME_CHECK_DISABLE_IRQ_TIME) && (hal_core_status_read(HAL_CORE_DSP0) != HAL_CORE_EXCEPTION) && \
      (is_time_check_assert_enabled == true) && check_assert_flag && ((int_level & 0xF) == 4)) {
        log_hal_msgid_error("[NVIC][DSP HAL %d us]start:%d,end:%d,duartion:%d,lr:0x%x,flag:0x%x\r\n", 6,
                                TIME_CHECK_DISABLE_IRQ_TIME,
                                backup_time_start,
                                backup_time_end,
                                backup_duration_us,
                                backup_linkAddress,
                                backup_critical_flag);
        #ifdef AIR_ICE_DEBUG_ENABLE
        if (hal_ice_debug_is_enabled() == true) {
            return HAL_NVIC_STATUS_OK;
        }
        #endif

        assert(0);
    }
#endif

    return HAL_NVIC_STATUS_OK;
}

ATTR_TEXT_IN_IRAM hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask)
{
    *mask = (uint32_t)XTOS_SET_INTLEVEL(4);

#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    if ((*mask & 0xf) == 0) {
#ifdef MTK_SWLA_ENABLE
        SLA_RamLogging(0xAAAAFFFF);
#endif
        hal_time_check_disbale_irq_start_special = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
        hal_disable_irq_linkAddress_special = (uint32_t)__builtin_return_address(0);
    }
#endif

    return HAL_NVIC_STATUS_OK;
}

ATTR_TEXT_IN_IRAM hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask)
{
#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    uint32_t backup_time_start = 0, backup_time_end = 0, backup_duration_us = 0, backup_linkAddress = 0;
     uint32_t int_level;

    int_level = (uint32_t)XTOS_SET_INTLEVEL(4);
    XTOS_RESTORE_INTLEVEL(int_level);           // restore intlevel

    if (((mask & 0xF) == 0) && ((int_level & 0xF) == 4)) {
        backup_time_end    = GPT(HAL_GPT_US_PORT)->GPT_COUNT;
        backup_time_start = hal_time_check_disbale_irq_start_special;
        backup_duration_us = backup_time_end - backup_time_start;
        backup_linkAddress = hal_disable_irq_linkAddress_special;
        hal_disable_irq_linkAddress_special = 0x0;
#ifdef MTK_SWLA_ENABLE
        SLA_RamLogging(0xAAAAAAAA);
#endif
	}
#endif

    XTOS_RESTORE_INTLEVEL(mask);

#ifdef HAL_TIME_CHECK_DISABLE_IRQ_ENABLED
    if ((backup_duration_us > TIME_CHECK_DISABLE_IRQ_TIME_SPECIAL)  && (hal_core_status_read(HAL_CORE_DSP0) != HAL_CORE_EXCEPTION) && \
      (is_time_check_assert_enabled == true) && ((int_level & 0xF) == 4)) {
        log_hal_msgid_warning("[NVIC][DSP HAL SPECIAL %d us]start:%d,end:%d,duartion:%d,lr:0x%x \r\n", 5,
                                TIME_CHECK_DISABLE_IRQ_TIME_SPECIAL,
                                backup_time_start,
                                backup_time_end,
                                backup_duration_us,
                                backup_linkAddress);
        #ifdef AIR_ICE_DEBUG_ENABLE
        if (hal_ice_debug_is_enabled() == true) {
            return HAL_NVIC_STATUS_OK;
        }
        #endif

        assert(0);
    }
#endif

    return HAL_NVIC_STATUS_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* HAL_NVIC_MODULE_ENABLED */

