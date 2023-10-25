/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#include "battery_management_core.h"
#include "hal_pmu_charger_hp.h"
#include "hal_pmu.h"
extern TimerHandle_t xbm_jeita_timer;
uint8_t executing_status = HW_JEITA_NORMAL_STAGE;
extern battery_managerment_control_info_t bm_ctrl_info;
extern battery_basic_data bm_cust;

void battery_jetia_create_check_timer(void)
{
#ifndef MTK_BATTERY_MANAGEMENT_NTC_LESS
    if (xTimerStartFromISR(xbm_jeita_timer, 0) != pdPASS) {
        LOG_MSGID_I(battery_management, "[BM_JEITA]xbm_jeita_Timer xTimerStart fail\n", 0);
    }
#endif
}

void battery_jetia_timer_callback(TimerHandle_t pxTimer)
{
    uint8_t jeita_status ;
#if 0 //For Debug
    uint32_t hwJeitaValue;
    hwJeitaValue = pmu_auxadc_get_channel_value(PMU_AUX_HW_JEITA);
    LOG_MSGID_I(battery_management, "[BM_JEITA]Jeita auxadc:%d jeita_status:%x\r\n", 2, hwJeitaValue, jeita_status);
#endif
    jeita_status = pmu_get_hw_jeita_status();
    if (jeita_status == HW_JEITA_NORMAL_STAGE) {
        if (executing_status != HW_JEITA_NORMAL_STAGE) {
            pmu_set_jeita_state_setting(HW_JEITA_NORMAL_STAGE, RG_ICC_JC_100, RG_VCV_VOLTAGE_4P20V); /*Normal State will not be change*/
            executing_status = HW_JEITA_NORMAL_STAGE;
            bm_ctrl_info.jeita_state = BM_TEMP_NORMAL;
            LOG_MSGID_I(battery_management, "[BM_JEITA]Normal Stage", 0);
        }
    } else if (jeita_status == HW_JEITA_HOT_STAGE) {
        if (executing_status != HW_JEITA_HOT_STAGE) {
            executing_status = HW_JEITA_HOT_STAGE;
            bm_ctrl_info.jeita_state = BM_TEMP_HOT;
            LOG_MSGID_I(battery_management, "[BM_JEITA]Hot Stage,Disable Charger", 0);
        }
    } else if (jeita_status == HW_JEITA_COLD_STAGE) {
        if (executing_status != 0) {
            executing_status = HW_JEITA_COLD_STAGE;
            bm_ctrl_info.jeita_state = BM_TEMP_COLD;
            LOG_MSGID_I(battery_management, "[BM_JEITA]Cold Stage,Disable Charger", 0);
        }
    } else if (jeita_status == HW_JEITA_WARM_STAGE) {
        if (executing_status != HW_JEITA_WARM_STAGE) {
            pmu_set_jeita_state_setting(HW_JEITA_WARM_STAGE, bm_cust.warm_cc, battery_get_full_battery_index(bm_cust.warm_cv));
            executing_status = HW_JEITA_WARM_STAGE;
            bm_ctrl_info.jeita_state = BM_TEMP_WARM;
            LOG_MSGID_I(battery_management, "[BM_JEITA]Warm Stage", 0);
        }
    } else if (jeita_status == HW_JEITA_COOL_STAGE) {
        if (executing_status != HW_JEITA_COOL_STAGE) {
            pmu_set_jeita_state_setting(HW_JEITA_COOL_STAGE, bm_cust.cool_cc, battery_get_full_battery_index(bm_cust.cool_cv));
            executing_status = HW_JEITA_COOL_STAGE;
            bm_ctrl_info.jeita_state = BM_TEMP_COOL;
            LOG_MSGID_I(battery_management, "[BM_JEITA]Cool Stage", 0);
        }
    } else {
        LOG_MSGID_I(battery_management, "[BM_JEITA]Error assert\r\n", 0);
    }
}

void battery_core_hw_jeita_set_threshold(uint32_t auxadcVolt, uint8_t JeitaThreshold)
{
    uint32_t jeitaVoltage = 0;
    jeitaVoltage = (uint32_t)((auxadcVolt * 4096) / 1800);
    switch (JeitaThreshold) {
        case HW_JEITA_HOT_STAGE:
            LOG_MSGID_I(battery_management, "[BM_JEITA]Set JEITA HOT:%d\r\n", 1, jeitaVoltage);
            pmu_set_jeita_voltage(jeitaVoltage, HW_JEITA_HOT_STAGE);
            pmu_register_callback(RG_INT_JEITA_HOT, battery_jetia_create_check_timer, NULL);
            break;

        case HW_JEITA_WARM_STAGE:
            LOG_MSGID_I(battery_management, "[BM_JEITA]Set JEITA WARM:%d\r\n", 1, jeitaVoltage);
            pmu_set_jeita_voltage(jeitaVoltage, HW_JEITA_WARM_STAGE);
            pmu_register_callback(RG_INT_JEITA_WARM, battery_jetia_create_check_timer, NULL);
            break;

        case HW_JEITA_COOL_STAGE:
            LOG_MSGID_I(battery_management, "[BM_JEITA]Set JEITA COOL:%d\r\n", 1, jeitaVoltage);
            pmu_set_jeita_voltage(jeitaVoltage, HW_JEITA_COOL_STAGE);
            pmu_register_callback(RG_INT_JEITA_COOL, battery_jetia_create_check_timer, NULL);
            break;

        case HW_JEITA_COLD_STAGE:
            LOG_MSGID_I(battery_management, "[BM_JEITA]Set JEITA COLD:%d\r\n", 1, jeitaVoltage);
            pmu_set_jeita_voltage(jeitaVoltage, HW_JEITA_COLD_STAGE);
            pmu_register_callback(RG_INT_JEITA_COLD, battery_jetia_create_check_timer, NULL);
            break;
    }
}

void battery_core_hw_jeita_init_threshold(void)
{
    battery_core_hw_jeita_set_threshold(bm_cust.jeita.hot, HW_JEITA_HOT_STAGE);
    battery_core_hw_jeita_set_threshold(bm_cust.jeita.warm, HW_JEITA_WARM_STAGE);
    battery_core_hw_jeita_set_threshold(bm_cust.jeita.cool, HW_JEITA_COOL_STAGE);
    battery_core_hw_jeita_set_threshold(bm_cust.jeita.cold, HW_JEITA_COLD_STAGE);
}

void battery_core_hw_jeita_init(void)
{
    LOG_MSGID_I(battery_management, "[BM_JEITA]Enable HW JEITA function\r\n", 0);
    pmu_hw_jeita_init();

}

