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

#include "hal_sleep_manager.h"
#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_rtc.h"
#include "hal_gpt.h"
#include "hal_gpt_internal.h"
#include "hal_sleep_manager_internal.h"
#include "hal_sleep_manager_platform.h"
#include "hal_pmu.h"
#include "hal_resource_assignment.h"
#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif /* MTK_SWLA_ENABLE */

#ifndef __UBL__
#include "assert.h"
#else
#define assert(expr) log_hal_msgid_error("assert\r\n", 0)
#endif

static uint32_t sleep_sw_gpt_handle;

void hal_sleep_manager_sw_gpt_callback()
{
    //log_hal_msgid_info("hal_sleep_manager_sw_gpt_callback\r\n", 0);
    hal_gpt_sw_stop_timer_ms(sleep_sw_gpt_handle); /* stop timer clear gpt irq */
}

uint8_t hal_sleep_manager_set_sleep_handle(const char *handle_name)
{
    uint8_t index;
    index = sleep_management_get_lock_handle(handle_name);
    return index;
}

ATTR_TEXT_IN_TCM hal_sleep_manager_status_t hal_sleep_manager_lock_sleep(uint8_t handle_index)
{
    sleep_management_lock_sleep(LOCK_SLEEP, handle_index);
    return HAL_SLEEP_MANAGER_OK;
}

hal_sleep_manager_status_t hal_sleep_manager_unlock_sleep(uint8_t handle_index)
{
    sleep_management_lock_sleep(UNLOCK_SLEEP, handle_index);
    return HAL_SLEEP_MANAGER_OK;
}

hal_sleep_manager_status_t hal_sleep_manager_release_sleep_handle(uint8_t handle_index)
{
    if (hal_sleep_manager_get_lock_status() & (1 << handle_index)) {
        log_hal_msgid_error("handle %d is currently holding a lock, cannot release\n", 1, handle_index);
        return HAL_SLEEP_MANAGER_ERROR;
    }
    sleep_management_release_lock_handle(handle_index);
    return HAL_SLEEP_MANAGER_OK;
}

uint32_t hal_sleep_manager_get_lock_status(void)
{
    return sleep_management_get_lock_sleep_request_info();
}

uint32_t hal_sleep_manager_sleep_driver_dump_handle_name(void)
{
    return sleep_management_get_lock_sleep_handle_list();
}

bool hal_sleep_manager_is_sleep_locked(void)
{
    return sleep_management_check_sleep_locks();
}

bool hal_sleep_manager_is_sleep_handle_alive(uint8_t handle_index)
{
    return sleep_management_check_handle_status(handle_index);
}

#ifdef HAL_SLEEP_MANAGER_SUPPORT_POWER_OFF
void hal_sleep_manager_enter_power_off_mode()
{

}
#endif

hal_sleep_manager_status_t hal_sleep_manager_set_sleep_time(uint32_t sleep_time_ms)
{
    hal_gpt_status_t    ret_status;

    if (sleep_time_ms > HAL_GPT_MAXIMUM_MS_TIMER_TIME) {
        sleep_time_ms = HAL_GPT_MAXIMUM_MS_TIMER_TIME;
    }

    hal_gpt_sw_stop_timer_ms(sleep_sw_gpt_handle);
    ret_status = hal_gpt_sw_start_timer_ms(sleep_sw_gpt_handle, sleep_time_ms, (hal_gpt_callback_t)hal_sleep_manager_sw_gpt_callback, NULL);
    if (ret_status != HAL_GPT_STATUS_OK) {
        //log_hal_error("ERROR : Deep Sleep GPT Start Fail %d", ret_status);
        assert(0);
        return HAL_SLEEP_MANAGER_ERROR;
    }

    return HAL_SLEEP_MANAGER_OK;
}

void hal_sleep_manager_enter_sleep_mode(hal_sleep_mode_t mode)
{
    if (mode == HAL_SLEEP_MODE_IDLE) {

#ifdef MTK_SWLA_ENABLE
        SLA_CustomLogging("wfi", SA_START);
#endif /* MTK_SWLA_ENABLE */
        __asm volatile("dsb");
        __asm volatile("wfi");
        __asm volatile("isb");



#ifdef MTK_SWLA_ENABLE
        SLA_CustomLogging("wfi", SA_STOP);
#endif /* MTK_SWLA_ENABLE */
    } else if (mode == HAL_SLEEP_MODE_SLEEP) {

#ifdef MTK_SWLA_ENABLE
        SLA_CustomLogging("dpm", SA_START);
#endif /* MTK_SWLA_ENABLE */
        hal_core_status_write(HAL_CORE_MCU, HAL_CORE_SLEEP);

        /* NS module backup in Limit TZ*/
#ifndef AIR_CPU_IN_SECURITY_MODE
        sleep_management_ns_suspend_callback();
#endif

#ifdef AIR_CPU_IN_SECURITY_MODE
        /* Sleep in All In Secure*/
        sleep_management_enter_deep_sleep(HAL_SLEEP_MODE_SLEEP);
#else
        /* Switch To S By NSC Fuction In Limit TZ or TCB In All In Secure*/
        sleep_management_enter_deep_sleep_in_secure(HAL_SLEEP_MODE_SLEEP);
#endif

        /* NS module restore in Limit TZ*/
#ifndef AIR_CPU_IN_SECURITY_MODE
        sleep_management_ns_resume_callback();
#endif

        hal_core_status_write(HAL_CORE_MCU, HAL_CORE_ACTIVE);
#ifdef MTK_SWLA_ENABLE
        SLA_CustomLogging("dpm", SA_STOP);
#endif /* MTK_SWLA_ENABLE */
    }
}

#ifdef HAL_SLEEP_MANAGER_SUPPORT_WAKEUP_SOURCE_CONFIG
hal_sleep_manager_status_t hal_sleep_manager_enable_wakeup_pin(hal_sleep_manager_wakeup_source_t pin)
{
    spm_unmask_wakeup_source(pin);
    return HAL_SLEEP_MANAGER_OK;
}

hal_sleep_manager_status_t hal_sleep_manager_disable_wakeup_pin(hal_sleep_manager_wakeup_source_t pin)
{
    spm_mask_wakeup_source(pin);
    return HAL_SLEEP_MANAGER_OK;
}
#endif
extern void spm_dvt_test_case_select();
uint8_t sleep_manager_handle;
hal_sleep_manager_status_t hal_sleep_manager_init()
{
    uint32_t freq_32K;

    hal_rtc_get_f32k_frequency(&freq_32K);
    //put rtc real frequency to share memory
    *((volatile uint32_t *)HW_SYSRAM_PRIVATE_MEMORY_RTC_FREQ_START) = freq_32K;

    //log_hal_msgid_info("hal_sleep_manager_init start\n", 0);

    /* SPM Clock Force to 26M bar, *SPM_CLK_SW_CON &= ~(1 << 0) */
    *SPM_CLK_SW_CON &= ~(1 << 0);

    spm_init();

    sleep_manager_handle = hal_sleep_manager_set_sleep_handle("slp");
    //hal_sleep_manager_lock_sleep(sleep_manager_handle);

#ifdef AIR_USB_DONGLE_PROJECT_ENABLE
    spm_control_mtcmos_internal(SPM_MTCMOS_AUDIO_HS, SPM_MTCMOS_PWR_DISABLE);
    spm_control_mtcmos_internal(SPM_MTCMOS_AUDIO_ANC, SPM_MTCMOS_PWR_DISABLE);
    spm_control_mtcmos_internal(SPM_MTCMOS_AUDIO_SYS, SPM_MTCMOS_PWR_DISABLE);
#endif
    log_hal_msgid_info("[SLP] SPM_PWR_STATUS is 0x%08x", 1, *SPM_PWR_STATUS);
    /*
    sleep_management_low_power_init_setting();
    */

    if (hal_gpt_sw_get_timer(&sleep_sw_gpt_handle) != HAL_GPT_STATUS_OK) {
        //log_hal_error("ERROR : Deep Sleep GPT Init Fail");
        assert(0);
        return HAL_SLEEP_MANAGER_ERROR;
    }

    // spm_dvt_test_case_select();

    return HAL_SLEEP_MANAGER_OK;
}

#endif /* HAL_SLEEP_MANAGER_ENABLED */
