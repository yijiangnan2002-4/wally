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

/**
 * File: app_preproc_sys_pwr.c
 *
 * Description: This activity is used to implement PMU OFF/RTC/Sleep function.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */


#include "app_preproc_sys_pwr.h"
#include "apps_events_bt_event.h"
#include "apps_debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hal_pmu.h"
#include "hal_gpt.h"

#ifdef HAL_SLEEP_MANAGER_ENABLED
#ifdef HAL_RTC_MODULE_ENABLED
#include "hal_sleep_manager_internal.h"
#include "hal_rtc.h"
#include "hal_spm.h"

static void app_rtc_alarm_setting()
{
    hal_rtc_time_t time, rtc_time;

    hal_rtc_get_time(&time);
    rtc_time.rtc_year = time.rtc_year;
    rtc_time.rtc_mon = time.rtc_mon;
    rtc_time.rtc_day = time.rtc_day;
    rtc_time.rtc_hour = time.rtc_hour;
    rtc_time.rtc_min = time.rtc_min;
    rtc_time.rtc_sec = time.rtc_sec;
    rtc_time.rtc_week = time.rtc_week;
    rtc_time.rtc_milli_sec = time.rtc_milli_sec;
    if (time.rtc_sec + 25 > 59) {
        rtc_time.rtc_sec = time.rtc_sec + 25 - 60;
        rtc_time.rtc_min = time.rtc_min + 1;
    } else {
        rtc_time.rtc_sec = time.rtc_sec + 25;
    }
    hal_rtc_set_alarm(&rtc_time);
}


extern bool    hal_rtc_is_back_from_rtcmode();
static void app_rtc_alarm_irq_test(void)
{
    if (hal_rtc_is_back_from_rtcmode() == false) {
        app_rtc_alarm_setting();
        hal_rtc_enable_alarm();
        hal_rtc_enter_rtc_mode();
    } else {
        APPS_LOG_MSGID_I("rtc mode wakeup by alarm test pass\r\n", 0);
        app_rtc_alarm_setting();
        hal_rtc_disable_alarm();
        hal_rtc_enter_rtc_mode();
    }
}

#endif
#endif

bool sys_pwr_component_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    vTaskDelay(500);

    /* Handle PMU Race CMD event in UI Shell task, see bt_race_system_power_event_callback in apps_events_bt_event.c. */
    switch (event_id) {
        /* Power off. */
        case APPS_EVENTS_SYSTEM_POWER_PMUOFF:
            /* AT+EPMUREG=PWR,0\0d\0a */
#ifdef HAL_PMU_MODULE_ENABLED
#ifdef AIR_BTA_IC_PREMIUM_G2
            pmu_power_off_sequence(0);
#else
            pmu_power_off_sequence(PMU_PWROFF);
#endif
#endif
            break;

#ifdef HAL_SLEEP_MANAGER_ENABLED
#ifdef HAL_RTC_MODULE_ENABLED
        /* Enter to RTC mode. */
        case APPS_EVENTS_SYSTEM_POWER_RTC:
            /* AT+SM=RTC,1 */
            hal_gpt_delay_ms(200);
            app_rtc_alarm_irq_test();
            break;

        /* Enter to sleep mode. */
        case APPS_EVENTS_SYSTEM_POWER_SLEEP:
            /* AT+SM=PWR,ALL,0, AT+SM=BASE */
#if 0
            spm_control_mtcmos(SPM_MTCMOS_DSP0, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_DSP1, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_CONN, SPM_MTCMOS_PWR_DISABLE);

            pmu_vcore_voltage_sel_6388(PMU_SLEEP, PMIC_VCORE_0P7_V);
            pmu_sw_enter_sleep(PMU_LDO_VA18);
            pmu_sw_enter_sleep(PMU_LDO_VLDO33);
            pmu_sw_enter_sleep(PMU_BUCK_VRF);
            pmu_sw_enter_sleep(PMU_BUCK_VAUD18);

            *((volatile uint32_t *)(SPM_BASE + 0x0250)) = 0xF;  /* Sysram1 enter power down in sleep. */
            pmu_lp_mode_6388(PMU_BUCK_VCORE, PMU_HW_MODE);

            __asm volatile("cpsid i");
            *SPM_CM4_WAKEUP_SOURCE_MASK = 0x3FFF;
            *SPM_DSP0_WAKEUP_SOURCE_MASK = 0x3FFF;
            *SPM_DSP1_WAKEUP_SOURCE_MASK = 0x3FFF;

            //bsp_ept_gpio_setting_init();
            while (1) {
                __asm volatile("cpsid i");
                sleep_management_enter_deep_sleep(0);
                __asm volatile("cpsie i");
            }
#endif
// 2833_FPGA_APP_WORKAROUND
#if 0
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_ON, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_OFF, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_DISABLE);
            //spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_VOW, SPM_MTCMOS_PWR_DISABLE);

            __asm volatile("cpsid i");
            uint32_t mask;

            //spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_DISABLE);

            int i;
            for (i = 0; i < 32; i++) {

                if (hal_nvic_get_pending_irq(i) == 1) {
                    hal_nvic_disable_irq(i);
                    log_hal_msgid_info("pending_irq:%d\r\n", 1, i);
                    hal_nvic_clear_pending_irq(i);
                    hal_nvic_disable_irq(i);
                }
            }
            for (i = 0; i < 32; i++) {

                if (hal_nvic_get_pending_irq(i) == 1) {
                    log_hal_msgid_info("pending_irq:%d\r\n", 1, i);
                    hal_nvic_clear_pending_irq(i);
                    hal_nvic_disable_irq(i);
                }
            }
            hal_nvic_disable_irq(CRYPTO_IRQn);
            hal_nvic_disable_irq(SPM_IRQn);

            *SPM_IGNORE_CPU_ACTIVE |= 0x100;    //IGNORE DSP ACTIVE
            *SPM_CM4_WAKEUP_SOURCE_MASK = 0xFFFFFFFF;
            *SPM_DSP_WAKEUP_SOURCE_MASK = 0xFFFFFFFF;

            while (1) {
                hal_nvic_clear_pending_irq(20);
                *SPM_CONN_AUDIO_ABB_XO_SIDEBAND = 0;        //for sleep test, need BT or Audio Control.
                hal_nvic_save_and_set_interrupt_mask(&mask);
#ifdef MTK_LOCK_S1
                *SPM_SPM_STATE_CONTROL_0 = 0;
#endif
                hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
                hal_nvic_restore_interrupt_mask(mask);
                *SPM_CONN_AUDIO_ABB_XO_SIDEBAND = 0x101;    //for sleep test, need BT or Audio Control.
            }
#endif
            break;
#endif
#endif
        default:
            break;
    }

    return TRUE;
}
