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

#include "task.h"
#include "task_def.h"
#include "hal_flash.h"
#include "hal_nvic_internal.h"
#include "hal_pmu_ddie.h"
#include "assert.h"

#ifndef __EXT_BOOTLOADER__
#include "FreeRTOS.h"
#include "portmacro.h"
#endif

#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_sleep_manager_platform.h"
#include "hal_sleep_manager_internal.h"
#include "hal_sleep_manager.h"
#endif

#ifdef HAL_USB_MODULE_ENABLED
#include "hal_usb.h"
#include "hal_usb_internal.h"
#endif
#ifndef MTK_BATTERY_MANAGEMENT_STATIC_INIT_ENABLE
#endif

#ifdef AIR_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif

#ifdef MTK_FUEL_GAUGE
#include "fuelgauge_interface.h"
#include "battery_common.h"
extern void battery_management_fuel_gauge_init(void);
#endif
#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_S
#include "hal_pmu_hp_platform.h"
#endif

uint8_t gauge_cardinal_number;                /*Data : For gauge calculation*/
uint8_t battery_align_flag = 0;               /*flag : Avoid VBUS shaking, ensure the Irq data is consistent this time */
uint8_t battery_init_setting_flag = 0;        /*flag : Check battery init setting is finish or not*/
static uint32_t battery_callback_index = 0;   /*Data : Restore register callback function number */
TimerHandle_t xbm_chr_detec_t;                /*Timer : When PMIC irq"CHRDET" is Triggered, this timer will receive from pmic irq regiseter callbcck for processing*/
TimerHandle_t xbm_jeita_timer;                /*Timer : When With NTC and HW-JEITA is enable ,This timer will check JEITA status*/
TimerHandle_t xbm_eoc_timer;                  /*Timer : 1st enter the EOC will implement , to ensure that EOC verification can be performed without battery*/
TimerHandle_t xbm_option_setting_timer;       /*Timer : EOC setting will used, divided into multiple segments increase system resource allocation */
TimerHandle_t xbm_chrdet_calibration_timer;   /*Timer : Avoid unexpected problems caused by irq shake*/
TimerHandle_t xbm_chager_detect_timer;        /*Timer : one shot for charger detect*/
extern uint8_t executing_status;
const char *bm_charger_type[10] = { "", "SDP", "CDP", "DCP", "SS", "IPAD2_IPAD4", "IPHONE_5V_1A", "NON-STD", "DP/DM_Floating", "UNABLE_TO_IDENTIFY"};    /*Data : For output log */
const char *bm_charger_state[8] = { "CHARGER OFF", "PRECC", "CC", "EXTENSION", "EOC", "THERMAL", "VBAT_OVP", "SAFETY TIMER"};                            /*Data : For output log */

const int battery_basic_checkpoint[BATTERY_BASIC_CHECKPOINT_TBL_NUM] = {3443, 3513, 3579, 3617, 3667, 3748, 3853, 3954, 4062}; /*Index and Value output: For linear gauge*/
const int precc_voltage[PRECC_VOLTAGE_TBL_NUM] = {2400, 2550, 2700, 2850, 3000, 3150, 3300, 3450}; /*Index and Value output: PRECC Voltage*/
const int bm_cc_cur[BM_CC_CUR_TBL_NUM] = {
    0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135,
    140, 145, 150, 155, 160, 165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235, 240, 245, 250, 255, 260, 265, 270, 275, 280, 285,
    290, 295, 300, 305, 310, 315, 320, 325, 330, 335, 340, 345, 350, 355, 360, 365, 370, 375, 380, 385, 390, 395, 400, 405, 410, 415, 420, 425, 430, 435,
    440, 445, 450, 455, 460, 465, 470, 475, 480, 485, 490, 495, 500, 505, 510, 515, 520, 525, 530, 535, 540, 545, 550, 555, 560, 565, 570, 575, 580, 585,
    590, 595, 600, 605, 610, 615, 620, 625, 630, 635, 640
}; /*Index and Value output: Charger current*/
const int bm_iterm_current[BM_ITERM_CURRENT_TBL_NUM] = {
    0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64,
}; /*Index and Value output: Iterm value*/
const int battery_jeita_percentage[BATTERY_JEITA_PERCENTAGE_TBL_NUM] = {20, 0, 0, 0, 40, 60, 80, 100 }; /*Index and Value output: Jeita percentage*/
const int battery_icl_value[BATTERY_ICL_VALUE_TBL_NUM] = {10, 75, 200, 300, 443, 500, 600, 700, 800, 900, 1000}; /*Index and Value output: ICL*/

battery_basic_data bm_cust;                                                                      /*Data : restore battery basic data*/
battery_managerment_control_info_t bm_ctrl_info;                                                 /*Data : restore battery info*/
static bmt_callback_context_t bmt_callback[BATTERY_MANAGER_CALLBACK_FUNCTION_NUMBER];            /*Data : callback function*/
const battery_managerment_control_info_t *bm_ctrl_info_p;                                        /*Data : used for battery monitor callback function */
TaskHandle_t battery_regular_task_t = NULL;                                                      /*Task : create regular task for charger behavior*/
TaskHandle_t battery_lineat_task_t = NULL;                                                       /*Task : create regular task for linear capacity*/
log_create_module(battery_management, PRINT_LEVEL_INFO);                                         /*Syslog create*/
extern hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);
extern hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask);

#ifdef BATTERY_DECREASE_RESISTANCE
int battery_resistance_offset[9] = {950,  850,  787,  775,  769,  825,  875,  863, 850};         /*Index : For linear gauge : resistance value*/
uint32_t remove_ir_soc = 0;                                                                      /*Data : For linear gauge smooth soc calculation*/
uint32_t remove_ir_vbat = 0;                                                                     /*Data : For linear gauge smooth soc calculation*/
extern int cc_value[67];                                                                        /*Data : For linear gauge smooth soc calculation*/
#else
int battery_resistance_offset[9] = {0,  0,  0,  0,  0,  0,  0,  0, 0};         /*Index : For linear gauge : resistance value*/
#endif
int bm_va18 = 0;
/*==========[Battery management API]=========*/


int32_t battery_management_get_battery_property_internal(battery_property_t property)
{
    int property_value = 0;
    switch (property) {
        case BATTERY_PROPERTY_CAPACITY:
#ifdef EXTERNAL_CHARGER
            /* add customize external charger api for calculating the
            percentage (0~100%) of the battery */
#else
            property_value = battery_get_gauge_percent();
#endif
            break;
        case BATTERY_PROPERTY_CAPACITY_LEVEL :
#ifdef EXTERNAL_CHARGER
            /*add customize external charger api for calculating the
            level of the battery*/
#else
            property_value = battery_get_gauge_percent_level();
#endif
            break;
        case BATTERY_PROPERTY_CHARGER_EXIST:
            if (battery_align_flag == 1) {
                property_value = bm_ctrl_info.isChargerExist;
            } else {
                property_value = pmu_get_chr_detect_value();
            }
            break;
        case BATTERY_PROPERTY_CHARGER_TYPE:
            property_value = bm_ctrl_info.chargerType;
            break;
        case BATTERY_PROPERTY_TEMPERATURE:
#ifdef EXTERNAL_CHARGER
            /*add customize external charger api for retrieving the charger's temperature*/
#else
            bm_ctrl_info.temperature =battery_auxadc_voltage_to_tempature(pmu_auxadc_get_channel_value(PMU_AUX_CHR_THM));
            property_value = bm_ctrl_info.temperature;
#endif
            break;
        case BATTERY_PROPERTY_VOLTAGE:
            property_value = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
            break;
        case BATTERY_PROPERTY_VOLTAGE_IN_PERCENT:
            property_value = battery_get_gauge_percent();
            break;
        case BATTERY_PROPERTY_PMIC_TEMPERATURE:
            property_value = battery_get_pmic_temp();
            break;
        case BATTERY_PROPERTY_CHARGER_STATE:
            /* 0: CHR_OFF ; 1: PRECC ; 2:FASTCC ;3: EXTENSION ; 4: EOC / 5: THR/ 6: VBAT_OVP  / 7: PRECC or CC  SAFETY timer time out*/
            property_value = pmu_get_charger_state();
            break;
#ifdef MTK_FUEL_GAUGE
        case BATTERY_PROPERTY_UI_SOC:
            property_value = get_ui_soc();
            break;
#endif
        default:
            property_value = BATTERY_INVALID_VALUE;
            break;
    }
    LOG_MSGID_I(battery_management, "[BM_BASIC]property_value : %d %d ", 2, property, property_value);
    return property_value;
}

void battery_switch_charger_option(int option)
{
    bm_ctrl_info.feature.charger_option = option;
}

void battery_enable_charger(battery_managerment_operate_t oper)
{
    battery_set_enable_charger(oper);
}

battery_basic_data battery_management_get_basic_data()
{
    return bm_cust;
}

/*==========[Battery Management Callback Function]==========*/
void battery_charger_state_change_callback(void)
{
    uint32_t newState = pmu_get_charger_state();
    switch (newState) {
        case CHARGER_STATE_CHR_OFF:
            LOG_W(MPLOG, "[BM_CHR]CHR OFF State "); /*Log output by BT*/
            if (bm_ctrl_info.feature.charger_option == 3 && ((bm_ctrl_info.chargerType != SDP_CHARGER) && (bm_ctrl_info.chargerType != CDP_CHARGER))) {
                battery_unlock_sleep();
                LOG_MSGID_I(battery_management, "[BM_CHR]PMU sleep handle %d\n", 1, sleep_management_check_handle_status(SLEEP_LOCK_BATTERY_MANAGEMENT));
            }
            break;

        case CHARGER_STATE_EOC:
            LOG_W(MPLOG, "[BM_CHR]EOC State  %d  ", bm_ctrl_info.feature.charger_option); /*Log output by BT*/
            if (bm_ctrl_info.feature.charger_option != 1 && (bm_ctrl_info.charger_eoc_state == 0 && EOC_CHECK_ON)) {
                LOG_MSGID_I(battery_management, "[BM_CHR]First times enter EOC state, start timer", 0);
                xTimerStartFromISR(xbm_eoc_timer, 0);
                break;
            }
            LOG_MSGID_I(battery_management, "[BM_CHR]Battery sleep handle %d\n", 1, sleep_management_check_handle_status(SLEEP_LOCK_BATTERY_MANAGEMENT));
            if (bm_ctrl_info.feature.charger_option == 1) {
                xTimerStartFromISR(xbm_option_setting_timer, 0);
            }
            battery_management_enter_eoc_rtc_mode();
            bm_ctrl_info.feature.charger_init = 1;
            bm_ctrl_info.gauge_calibration = 5;
            break;

        case CHARGER_STATE_SAFETY_TIMER_TIMEOUT:
            LOG_MSGID_I(battery_management, "[BM_CHR]Safety Timeout State ", 0);
            break;

        case CHARGER_STATE_FASTCC:
            LOG_W(MPLOG, "[BM_CHR]FASTCC State "); /*Log output by BT*/
            break;
        case CHARGER_STATE_THR:
            LOG_MSGID_I(battery_management, "[BM_CHR]THR State ", 0);
            break;
        case CHARGER_STATE_VBAT_OVP:
            LOG_MSGID_I(battery_management, "[BM_CHR]Vbat OVP State ", 0);
            pmu_ovp_debug();
            break;
        case CHARGER_STATE_PRECC:
            pmu_set_pre_charger_current(bm_cust.precc_cur);
            LOG_MSGID_I(battery_management, "[BM_CHR]Precc State ", 0);
            break;
        case CHARGER_STATE_EXTENSION:
            LOG_MSGID_I(battery_management, "[BM_CHR]Extension State ", 0);
            break;
        default:
            break;
    }
    bm_ctrl_info.chargerState = newState;
    battery_notification(BATTERY_MANAGEMENT_EVENT_CHARGER_STATE_UPDATE, pmu_get_chr_detect_value(), newState);
}
void battery_charger_setting(TimerHandle_t pxTimer)
{
    switch (bm_ctrl_info.charger_step) {
        case BM_CHARGER_IN_CHECK_POWER:
            if (!sleep_management_check_handle_status(SLEEP_LOCK_BATTERY_MANAGEMENT)) {
                hal_sleep_manager_lock_sleep(SLEEP_LOCK_BATTERY_MANAGEMENT);
            }
            battery_check_charger_power();
            battery_set_charger_step_timer(BM_CHARGER_IN_CHECK_POWER, BM_CHARGER_IN_JEITA_INIT);
            break;
        case BM_CHARGER_IN_JEITA_INIT:
            if (bm_cust.feature_jeita == BATTERY_OPERATE_ON) {
                battery_core_hw_jeita_init();
                pmu_set_hw_jeita_enable(PMU_ON);
                battery_set_charger_step_timer(BM_CHARGER_IN_JEITA_INIT, BM_CHARGER_IN_JEITA_WC);
            } else {
                pmu_set_hw_jeita_enable(PMU_OFF);
                battery_set_charger_step_timer(BM_CHARGER_IN_JEITA_INIT, BM_CHARGER_IN_USB);
            }
            break;
        case BM_CHARGER_IN_JEITA_WC:
            if(bm_cust.feature_warm_cool == BATTERY_OPERATE_ON){
                pmu_set_register_value(PMU_LCHR_DIG_CON0, RG_DISWARMCOOL_MASK, RG_DISWARMCOOL_SHIFT, 0);
                LOG_MSGID_I(battery_management, "[BM_CHR]Disable JEITA WARM COOL detect", 0);
            }else{
                pmu_enable_interrupt(RG_INT_JEITA_WARM, PMU_OFF);
                pmu_enable_interrupt(RG_INT_JEITA_COOL, PMU_OFF);
                pmu_set_register_value(PMU_LCHR_DIG_CON0, RG_DISWARMCOOL_MASK, RG_DISWARMCOOL_SHIFT, 1);
                LOG_MSGID_I(battery_management, "[BM_CHR]Enable JEITA WARM COOL detect", 0);
            }
            battery_set_charger_step_timer(BM_CHARGER_IN_JEITA_WC, BM_CHARGER_IN_USB);
            break;

        case BM_CHARGER_IN_USB:
#ifdef HAL_USB_MODULE_ENABLED
            hal_usb_phy_preinit();
#endif
            battery_set_charger_step_timer(BM_CHARGER_IN_USB, BM_CHARGER_IN_PLUGIN_INIT);
            break;
        case BM_CHARGER_IN_PLUGIN_INIT:
            pmu_set_charger_current_limit(ICL_ITH_75mA);
            battery_charger_plugin_initial_setting();
            //pmu_disable_vsys_discharge(0);
            battery_set_charger_step_timer(BM_CHARGER_IN_PLUGIN_INIT, BM_CHARGER_IN_BC12);
            break;
        case BM_CHARGER_IN_BC12:
            if (bm_cust.feature_bc12 == BATTERY_OPERATE_ON) {
#ifdef AIR_WIRELESS_MIC_ENABLE
                bm_ctrl_info.chargerType = pmu_get_bc12_charger_type();
                pmu_set_icl_by_type(bm_ctrl_info.chargerType);
#else
                pmu_set_charger_current_limit(bm_cust.icl);
#endif
            } else {
                pmu_set_charger_current_limit(bm_cust.icl);
            }
            battery_set_charger_step_timer(BM_CHARGER_IN_BC12, BM_CHARGER_IN_ENABLE);
            break;
        case BM_CHARGER_IN_ENABLE:
            pmu_charger_init(bm_cust.precc_cur, bm_cust.cv_termination); /*EOC option 3 will enter sleep , CO-domain will be reset*/
            battery_set_charger_step();
            battery_set_enable_charger(BATTERY_OPERATE_ON);
            if (bm_cust.feature_preecc_timer != BATTERY_OPERATE_ON) {
                LOG_MSGID_I(battery_management, "[BM_CHR]Disable precc safety timer", 0);
                pmu_enable_safety_timer_hp(PMU_PRECC, bm_cust.feature_preecc_timer);
            }
            if (bm_cust.feature_fastcc_timer != BATTERY_OPERATE_ON) {
                LOG_MSGID_I(battery_management, "[BM_CHR]Disable fastcc safety timer", 0);
                pmu_enable_safety_timer_hp(PMU_FASTCC, bm_cust.feature_fastcc_timer);
            }
            battery_set_charger_step_timer(BM_CHARGER_IN_ENABLE, BM_CHARGER_IN_GAUGE_CALI);
            break;
        case BM_CHARGER_IN_GAUGE_CALI:
            battery_switch_calibration_state(BATTERY_OPERATE_ON);
            battery_set_charger_step_timer(BM_CHARGER_IN_GAUGE_CALI, BM_CHARGER_NOTIFICATION);
            break;

        case BM_CHARGER_OUT_CHECK_POWER:
            //pmu_disable_vsys_discharge(0x2);
            bm_ctrl_info.feature.charger_init = 0;
            battery_set_charger_step_timer(BM_CHARGER_OUT_CHECK_POWER, BM_CHARGER_OUT_JEITA_OFF);
            break;

        case BM_CHARGER_OUT_JEITA_OFF:
            if (bm_cust.feature_jeita == BATTERY_OPERATE_ON) {
                pmu_set_hw_jeita_enable(PMU_OFF);
                xTimerStopFromISR(xbm_jeita_timer, 0);
            }
            battery_set_charger_step_timer(BM_CHARGER_OUT_JEITA_OFF, BM_CHARGER_OUT_EOC_EXIT);
            break;

        case BM_CHARGER_OUT_EOC_EXIT:
            battery_set_enable_charger(BATTERY_OPERATE_OFF);
            if (bm_ctrl_info.feature.charger_option == 1) {
                LOG_MSGID_I(battery_management, "[BM_CHR]EOC option 1 exit\n", 0);
            } else {
                LOG_MSGID_I(battery_management, "[BM_CHR]EOC option %d exit,error option\n", 1, bm_ctrl_info.feature.charger_option);
                assert(0);
            }
            battery_set_charger_step_timer(BM_CHARGER_OUT_EOC_EXIT, BM_CHARGER_OUT_GAUGE_CALI);
            break;

        case BM_CHARGER_OUT_GAUGE_CALI:
            battery_switch_calibration_state(BATTERY_OPERATE_OFF);
            battery_unlock_sleep();
            battery_reguest_va18(BATTERY_OPERATE_OFF);
            if (bm_va18 == 1) {
                pmu_enable_power(PMU_LDO_VA18, PMU_OFF);
                bm_va18 = 0;
            }
            LOG_MSGID_I(battery_management, "[BM_CHR]out bm_va18 %d\n", 1, bm_va18);
            LOG_MSGID_I(battery_management, "[BM_CHR]PMU sleep handle %d\n", 1, sleep_management_check_handle_status(SLEEP_LOCK_BATTERY_MANAGEMENT));
            battery_set_charger_step_timer(BM_CHARGER_OUT_GAUGE_CALI, BM_CHARGER_NOTIFICATION);
            break;

        case BM_CHARGER_NOTIFICATION:
#if 1
            xTimerStartFromISR(xbm_chrdet_calibration_timer, 0);
#endif
            bm_ctrl_info.charger_step = BM_CHARGER_DONE;
            xTimerStopFromISR(xbm_chr_detec_t, 0);
            if (battery_init_setting_flag == 0) {
                pmu_charger_check_faston(); /*for BAT_OVP when FAST_ON*/
            }
            /*Avoid plug in/out too fast and causing charger Confused. */
            if ((bm_ctrl_info.feature.charger_option == 3) && (pmu_get_charger_state() == CHARGER_STATE_CHR_OFF)
                && (sleep_management_check_handle_status(SLEEP_LOCK_BATTERY_MANAGEMENT) >= 1) && (bm_ctrl_info.isChargerExist == BATTERY_OPERATE_ON)) {
                battery_unlock_sleep();
            }
            LOG_MSGID_I(battery_management, "[BM_CHR]PMU sleep handle %d\n", 1, sleep_management_check_handle_status(SLEEP_LOCK_BATTERY_MANAGEMENT));
            LOG_MSGID_I(battery_management, "[BM_CHR][%d]notification update", 1, bm_ctrl_info.charger_step);
            LOG_MSGID_I(battery_management, "[BM_CHR]Battery capacity %d\n", 1, (int)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY));
            battery_notification(BATTERY_MANAGEMENT_EVENT_CHARGER_EXIST_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
            battery_charger_state_change_callback();
            battery_align_flag = 0;
            break;
    }
}

void battery_charger_detect_callback(void){
#ifndef __EXT_BOOTLOADER__
    int irq_table[1] = {PMU_IRQn};
    vPortDisableSchAndIrq(irq_table, 1);
#endif

#ifdef BATTERY_AVOID_SHAKING
    battery_align_flag = 1;
#endif
    bm_ctrl_info.isChargerExist = pmu_get_chr_detect_value();

    if(bm_ctrl_info.isChargerExist) {
        bm_ctrl_info.charger_step = BM_CHARGER_IN_CHECK_POWER;
#ifndef __EXT_BOOTLOADER__
        vPortEnableSchAndIrq(irq_table, 1);
#endif
        if (bm_va18 == 0) {
            pmu_enable_power(PMU_LDO_VA18, PMU_ON);
            bm_va18 = 1;
        }
        LOG_MSGID_I(battery_management, "[BM_CHR]In bm_va18 %d\n", 1, bm_va18);
        if (HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION) {
            if (xTimerStartFromISR(xbm_chr_detec_t, 0) != pdPASS) {
                LOG_MSGID_I(battery_management, "xbm_chr_detec_t xTimerStart fail\n", 0);
            }
        }else{
            if(xTimerStart(xbm_chr_detec_t, 0) != pdPASS) {
                LOG_MSGID_I(battery_management, "xbm_chr_detec_t xTimerStart fail\n", 0);
            }
        }
    }else{
        bm_ctrl_info.charger_step = BM_CHARGER_OUT_CHECK_POWER;
#ifndef __EXT_BOOTLOADER__
        vPortEnableSchAndIrq(irq_table, 1);
#endif
        if(HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION) {
            if(xTimerStartFromISR(xbm_chr_detec_t, 0) != pdPASS) {
                LOG_MSGID_I(battery_management, "xbm_chr_detec_t xTimerStart fail\n", 0);
            }
        }else{
            if(xTimerStart(xbm_chr_detec_t, 0) != pdPASS) {
                LOG_MSGID_I(battery_management, "xbm_chr_detec_t xTimerStart fail\n", 0);
            }
        }
        executing_status = HW_JEITA_NORMAL_STAGE;
    }
}

void battery_monitor(battery_management_event_t event, const void *data)
{
    bm_ctrl_info_p = data;
    if (event == BATTERY_MANAGEMENT_EVENT_CHARGER_EXIST_UPDATE) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]EVENT:CHARGER EXIST UPDATE:%d] [Charger Exist:%x][Chr_Ste:%d] ", 3, event, bm_ctrl_info_p->isChargerExist, bm_ctrl_info_p->chargerState);
        LOG_W(MPLOG, "[BM_BASIC]Battery State = %s\n", bm_charger_state[bm_ctrl_info_p->chargerState]);
    }
    if (event == BATTERY_MANAGEMENT_EVENT_CHARGER_STATE_UPDATE) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]EVENT:CHARGER STATE UPDATE:%d] [Charger Exist:%x][Chr_Ste:%d] ", 3, event, bm_ctrl_info_p->isChargerExist, bm_ctrl_info_p->chargerState);
        LOG_W(MPLOG, "[BM_BASIC]Battery State = %s\n", bm_charger_state[bm_ctrl_info_p->chargerState]);
    }
    if (event == BATTERY_MANAGEMENT_EVENT_CHARGER_ITERM) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]EVENT:CHARGER ITERM:%d index:%d] ", 2, event, bm_cust.cv_termination);
    }
    if (event == BATTERY_MANAGEMENT_EVENT_CHARGER_ITERM_CURRENT) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]EVENT:CHARGER ITERM CURRENT:%d index:%d] ", 2, event, bm_cust.iterm_irq);
        if (bm_cust.feature_2cv == BATTERY_OPERATE_ON) {
            battery_set_2cv_parameter(BM_2STEP_SET_CV2);
        }
    }
}

void battery_rechg_callback(void)
{
    LOG_MSGID_I(battery_management, "[BM_CHR]Recharge Interrupt Callback function ", 0);
    hal_sleep_manager_lock_sleep(SLEEP_LOCK_BATTERY_MANAGEMENT);
    if (bm_cust.feature_2cv == BATTERY_OPERATE_ON) {
        if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > bm_cust.cv1) {
            pmu_set_charger_current(bm_cust.s1_chr_cur);
        } else {
            pmu_set_charger_current(bm_cust.s2_chr_cur);
        }
    }
    if (bm_ctrl_info.feature.charger_option == 1) {
        LOG_MSGID_I(battery_management, "[BM_CHR]Option 1 recharger be trigged", 0);
    }  else {
        LOG_MSGID_I(battery_management, "[BM_CHR]recharger in empty option", 0);
    }

}

void  battery_eoc_timer_callback(TimerHandle_t pxTimer)
{
    LOG_MSGID_I(battery_management, "[BM_CHR]First times enter EOC state, stop timer", 0);
    bm_ctrl_info.charger_eoc_state = 1;
    battery_charger_state_change_callback();
}

void battery_thm_over110_callback(void)
{
    LOG_MSGID_I(battery_management, "[BM_CHR]THM_OVER 110 Trigger ", 0);
    LOG_MSGID_I(battery_management, "[BM_CHR]Set ICL 500mA ", 0);
    pmu_set_charger_current_limit(ICL_ITH_500mA);
}

void battery_one_shot_detect_callback(TimerHandle_t pxTimer)
{
    LOG_MSGID_I(battery_management, "[BM_CHR]battery_one_shot_detect_callback ", 0);
    battery_notification(BATTERY_MANAGEMENT_EVENT_CHARGER_EXIST_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
    battery_notification(BATTERY_MANAGEMENT_EVENT_BATTERY_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
}

/*==========[Battery Management init]==========*/
bmt_function_t battery_func = {
    battery_management_init_internal,
    battery_management_get_battery_property_internal,
    battery_management_register_callback_internal,
    NULL
};

void battery_timer_init(void)
{
    xTaskCreate(battery_charger_task, "charger_task", 384, NULL, TASK_PRIORITY_BELOW_NORMAL, &battery_regular_task_t);
#ifdef BATTERY_LINEAR_GAUGE
    xTaskCreate(battery_linear_gauge_task, "Linear_task", 384, NULL, TASK_PRIORITY_BELOW_NORMAL, &battery_lineat_task_t);
#endif
    xbm_option_setting_timer = xTimerCreate("xbm_option_setting_timer", (100 * TIMEOUT_PERIOD_1MS), pdFALSE, NULL, battery_eoc_option_setting);
    xbm_chr_detec_t = xTimerCreate("charger_regular_timer", 10 * TIMEOUT_PERIOD_1MS, pdFALSE, NULL, battery_charger_setting);
    if (bm_cust.feature_jeita == BATTERY_OPERATE_ON) {
        xbm_jeita_timer = xTimerCreate("jeita_Timer", (HW_JEITA_CHECK_INTERVAL_TIME * TIMEOUT_PERIOD_1S), pdTRUE, NULL, battery_jetia_timer_callback);
    }
#ifdef CHARGER_CALIBRATION
    xbm_chrdet_calibration_timer = xTimerCreate("charger_detect_Timer", (CALIBRATION_TIME * portTICK_PERIOD_MS), pdFALSE, NULL, battery_detect_calibration_timer_callback);
#endif
    xbm_eoc_timer = xTimerCreate("charger_eoc_timer", (BATTERY_EOC_CHECK_TIME * TIMEOUT_PERIOD_1S), pdFALSE, NULL, battery_eoc_timer_callback);
    xbm_chager_detect_timer = xTimerCreate("charger_detect", (1 * TIMEOUT_PERIOD_1S), pdFALSE, NULL, battery_one_shot_detect_callback);
    xTimerStart(xbm_chager_detect_timer, 0);
}

void battery_parameter_init(void)
{
    pmu_set_charger_current_limit(ICL_ITH_75mA);
    LOG_MSGID_I(battery_management, "[BM_BASIC]Change ICL Ith to 75mA ", 0);
    if (bm_cust.feature_jeita == BATTERY_OPERATE_ON) {
        LOG_MSGID_I(battery_management, "[BM_BASIC](Charger on)HW JEITA Init ", 0);
        bm_ctrl_info.feature.feature_jeita = 1;
        battery_core_hw_jeita_init();
    } else {
        bm_ctrl_info.feature.feature_jeita = 0;
    }
    bm_ctrl_info.feature.charger_option  = BATTERY_MANAGER_DEFAULT_CHARGER_OPTION;
    battery_set_calibration_time();
    if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > (bm_cust.full_bat - bm_cust.full_bat_offset) && pmu_get_charger_state() == 0x2) {
        battery_set_enable_charger(BATTERY_OPERATE_OFF);
        bm_ctrl_info.gauge_calibration = ((pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) - (bm_cust.full_bat - bm_cust.full_bat_offset)) / 10) + 1;
        battery_set_enable_charger(BATTERY_OPERATE_ON);
    } else if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > (bm_cust.full_bat - bm_cust.full_bat_offset)) {
        bm_ctrl_info.gauge_calibration = ((pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) - (bm_cust.full_bat - bm_cust.full_bat_offset)) / 10);
    } else {
        bm_ctrl_info.gauge_calibration = 0;
    }
    bm_ctrl_info.charger_step = BM_CHARGER_IN_CHECK_POWER;
    /**/
    battery_set_extend_charger_time(EXTEND_TIME);
    pmu_set_charger_current(bm_cust.s0_chr_cur); /*Set default charger current*/
    pmu_select_precc_voltage_hp(bm_cust.s0_voltage_index);//set precc voltage
    pmu_set_safety_timer_hp(PMU_PRECC, bm_cust.preecc_timer);
    pmu_set_safety_timer_hp(PMU_FASTCC, bm_cust.fastcc_timer);
    pmu_set_extend_charger_time_hp(bm_cust.extern_timer);
    pmu_charger_init(bm_cust.precc_cur, bm_cust.cv_termination);
}

battery_management_status_t battery_management_init_internal(void)
{
    LOG_MSGID_I(battery_management,"[BM_BASIC]Battery Management V4.0.5",0);
    if (pmu_get_power_on_reason() == 0x8) {
        pmu_select_eoc_option_operating(pmu_eoc_option4, option4_exit);
    }
    bm_cust.battery_category = 0;
#ifdef AIR_NVDM_ENABLE
    battery_management_set_battery_category();
#else
    bm_cust.battery_category = BATTERY_CATEGORY;
#endif
    LOG_MSGID_I(battery_management, "[BM_BASIC]Battery Categor[%d]", 1, bm_cust.battery_category);
  #if defined(AIR_BATTERY_MANAGEMENT_STATIC_INIT_ENABLE) || !defined(AIR_NVDM_ENABLE)
    battery_init_basic_data();
  #else
    battery_init_data_from_nvdm();
  #endif
    battery_timer_init();
    battery_parameter_init();
    battery_gauge_init();
    battery_core_hw_jeita_init_threshold();
#ifdef BATTERY_CUSTOMER_SETTING
    battery_manager_customer_setting();
#endif
    battery_management_interrupt_register();
#ifdef MTK_FUEL_GAUGE
    battery_management_fuel_gauge_init();
#endif
#ifdef MTK_FUEL_GAUGE
    LOG_MSGID_I(battery_management, "[BM_BASIC]VBat [%d][%d][%d]", 3, get_ui_soc(), battery_get_gauge_percent(), pmu_auxadc_get_channel_value(PMU_AUX_BATSNS));
#else
    LOG_MSGID_I(battery_management, "[BM_BASIC]VBat [%d][%d]", 2, battery_get_gauge_percent(), pmu_auxadc_get_channel_value(PMU_AUX_BATSNS));
#endif
    battery_init_check_charger_exist();
    /*When no charging, battery voltage is lower than initial battery or shutdown battery ,it will enter rtc mode*/
#ifdef BATTERY_CHECK_BATTERY_VOLTAGE
    if ((pmu_get_usb_input_status() != true) && (bm_cust.shutdown_bat > battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE))) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]Battery voltage low ", 0);
        hal_rtc_enter_rtc_mode();
    }
#endif
#ifdef BATTERY_DECREASE_RESISTANCE
    remove_ir_vbat = ((bm_cust.full_bat - bm_cust.full_bat_offset) - (cc_value[bm_cust.s2_chr_cur] * bm_cust.resist_offset[8] / 1000));
    remove_ir_soc = battery_core_gauge_function(remove_ir_vbat) - BATTERY_REMOVE_IR_SOC_MARGIN;
    LOG_MSGID_I(battery_management, "[BM_BASIC]remove_ir_vbat[%d]remove_ir_soc[%d] ", 2, remove_ir_vbat, remove_ir_soc);
#endif
    battery_init_setting_flag = 1;
    battery_unlock_sleep();
    LOG_W(MPLOG, "[BM_BASIC]Battery temperature = %d(Celsius degrees)", (int)battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE)); /*Log output by BT*/
    LOG_W(MPLOG, "[BM_BASIC]Charger status = %d ", (int)battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST)); /*Log output by BT*/
    LOG_W(MPLOG, "[BM_BASIC]Battery voltage = %d(mV)\n", (int) battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE)); /*Log output by BT*/
    LOG_W(MPLOG, "[BM_BASIC]Battery State = %s\n", bm_charger_state[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE)]); /*Log output by BT*/
#ifdef MTK_FUEL_GAUGE
    LOG_W(MPLOG, "[BM_BASIC]FG_SOC = %d(%%) ", (int)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY)); /*Log output by BT*/
    LOG_W(MPLOG, "[BM_BASIC]UI_SOC = %d(%%) ", (int)battery_management_get_battery_property(BATTERY_PROPERTY_UI_SOC)); /*Log output by BT*/
#else
    LOG_W(MPLOG, "[BM_BASIC]Battery capacity = %d(%%) ", (int)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY)); /*Log output by BT*/
#endif
    return BATTERY_MANAGEMENT_STATUS_OK;
}

void battery_iterm_callback(void)
{
    LOG_MSGID_I(battery_management, "[BM_BASIC]battery iterm callback", 0);
    battery_notification(BATTERY_MANAGEMENT_EVENT_CHARGER_ITERM, pmu_get_chr_detect_value(), pmu_get_charger_state());
}
void battery_iterm_current_callback(void)
{
    LOG_MSGID_I(battery_management, "[BM_BASIC]battery iterm current callback", 0);
    battery_notification(BATTERY_MANAGEMENT_EVENT_CHARGER_ITERM_CURRENT, pmu_get_chr_detect_value(), pmu_get_charger_state());
}
void battery_management_interrupt_register(void)
{
    pmu_register_callback(RG_INT_CHRDET, battery_charger_detect_callback, NULL);
    pmu_register_callback(RG_INT_ChgStatInt, battery_charger_state_change_callback, NULL);
    pmu_register_callback(RG_INT_VBAT_RECHG, battery_rechg_callback, NULL);
    pmu_register_callback(RG_INT_THM_OVER110, battery_thm_over110_callback, NULL);
    pmu_register_callback(RG_INT_ICHR_ITERM, battery_iterm_callback, NULL);
    pmu_register_callback(RG_INT_ICHR_CHG_CUR, battery_iterm_current_callback, NULL);
    if (battery_management_register_callback(battery_monitor) != BATTERY_MANAGEMENT_STATUS_OK) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]Cannot register battery callback", 0);
    }
}

void battery_init_check_charger_exist(void)
{
    bm_ctrl_info.isChargerExist = pmu_get_chr_detect_value();
    bm_ctrl_info.chargerState = pmu_get_charger_state();
    LOG_MSGID_I(battery_management, "[BM_BASIC]bm_ctrl_info.isChargerExist :%d", 1, bm_ctrl_info.isChargerExist);
    if (bm_ctrl_info.isChargerExist) {
        battery_charger_detect_callback();
        battery_charger_state_change_callback();
        battery_clear_charger_irq();
    } else {
        battery_notification(BATTERY_MANAGEMENT_EVENT_CHARGER_EXIST_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
        battery_notification(BATTERY_MANAGEMENT_EVENT_BATTERY_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
    }
}

/*==========[Other : Basic internal function]==========*/

#ifdef AIR_NVDM_ENABLE
uint8_t battery_management_set_battery_category(void)
{
    nvkey_status_t status = 0;
    uint32_t leng;
    nvid_cal_bat_mgmt_basic_t nvid_cal_bat_mgmt_basic;

    leng = sizeof(nvid_cal_bat_mgmt_basic);
    status = nvkey_read_data(NVID_CAL_BAT_MGMT_BASIC, (uint8_t *)&nvid_cal_bat_mgmt_basic, &leng);
    if (status != NVKEY_STATUS_OK) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]NVDM data error", 0);
        return BATTERY_MANAGEMENT_STATUS_ERROR;
    }
    bm_cust.battery_category = nvid_cal_bat_mgmt_basic.battery_category;
    return bm_cust.battery_category;
}

battery_management_status_t battery_init_data_from_nvdm()
{
    LOG_MSGID_I(battery_management, "[BM_BASIC]Get battery basic value from NVDM", 0);
    int i = 0;
    nvkey_status_t status = 0;
    uint32_t leng;
    //int temp_inde = 0;

    nvid_cal_general_cfg_t nvid_cal_general_cfg;
    nvid_cal_chg_ctl_cfg_t nvid_cal_chg_ctl_cfg;
    nvid_cal_bat_mgmt_basic_t nvid_cal_bat_mgmt_basic;
    nvid_cal_bat_mgmt_chr_t nvid_cal_bat_mgmt_chr;

    leng = sizeof(nvid_cal_bat_mgmt_basic);
    status = nvkey_read_data(NVID_CAL_BAT_MGMT_BASIC, (uint8_t *)&nvid_cal_bat_mgmt_basic, &leng);
    if (status != NVKEY_STATUS_OK) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]NVID_CAL_BAT_MGMT_BASIC data error", 0);
        battery_init_basic_data();
        return BATTERY_MANAGEMENT_STATUS_ERROR;
    }
    leng = sizeof(nvid_cal_bat_mgmt_chr);
    status = nvkey_read_data(NVID_CAL_BAT_MGMT_CHR, (uint8_t *)&nvid_cal_bat_mgmt_chr, &leng);
    if (status != NVKEY_STATUS_OK) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]NVID_CAL_BAT_MGMT_CHR data error", 0);
        battery_init_basic_data();
        return BATTERY_MANAGEMENT_STATUS_ERROR;
    }

    leng = sizeof(nvid_cal_chg_ctl_cfg);
    status = nvkey_read_data(NVID_CAL_CHG_CTL_CFG, (uint8_t *)&nvid_cal_chg_ctl_cfg, &leng);
    if (status != NVKEY_STATUS_OK) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]NVID_CAL_CHG_CTL_CFG data error", 0);
        battery_init_basic_data();
        return BATTERY_MANAGEMENT_STATUS_ERROR;
    }

    leng = sizeof(nvid_cal_general_cfg);
    status = nvkey_read_data(NVID_CAL_GENERAL_CFG, (uint8_t *)&nvid_cal_general_cfg, &leng);
    if (status != NVKEY_STATUS_OK) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]NVID_CAL_GENERAL_CFG data error", 0);
        battery_init_basic_data();
        return BATTERY_MANAGEMENT_STATUS_ERROR;
    }
    bm_cust.powerhold = nvid_cal_general_cfg.pwrhold_check;
    bm_cust.vbus_debounce = nvid_cal_general_cfg.vbus_debounce;
    pmu_set_vbus_debounce_time_hp(bm_cust.vbus_debounce);
    pmu_enable_powerhold_hp(bm_cust.powerhold);
    LOG_MSGID_I(battery_management, "[BM_BASIC]powerhold[%d]vbus_debounce[%d]", 2, bm_cust.powerhold, bm_cust.vbus_debounce);

    bm_cust.initial_bat = nvid_cal_bat_mgmt_chr.initial_bat;
    bm_cust.shutdown_bat = nvid_cal_bat_mgmt_chr.shutdown_bat;
    LOG_MSGID_I(battery_management, "[BM_BASIC]initial_bat[%dmV]shutdown_bat[%dmV]", 2, bm_cust.initial_bat, bm_cust.shutdown_bat);

    bm_cust.full_bat = nvid_cal_bat_mgmt_basic.full_bat_voltage;
    bm_cust.recharger_voltage = nvid_cal_bat_mgmt_basic.recharge_voltage;
    bm_cust.full_bat_offset = nvid_cal_bat_mgmt_basic.full_bat_voltage_offset;
    //temp_inde = nvid_cal_bat_mgmt_chr.rsv6;
    bm_cust.recharger_threshold = battery_get_recharger_index(bm_cust.full_bat_offset);
    LOG_MSGID_I(battery_management, "[BM_BASIC]Full_Vbat[%dmV]recharger_voltage[%dmV]index[%d]full_bat_offset[%dmV]", 4, bm_cust.full_bat, bm_cust.recharger_voltage, bm_cust.recharger_threshold, bm_cust.full_bat_offset);

    bm_cust.precc_cur_value = nvid_cal_bat_mgmt_basic.precc_current_setting / 10;
    bm_cust.precc_cur =  battery_get_charger_index(bm_cust.precc_cur_value);
    LOG_MSGID_I(battery_management, "[BM_BASIC]Precc_cur[%dmA]index[%d]", 2, bm_cust.precc_cur_value, bm_cust.precc_cur);

    bm_cust.feature_2cv = nvid_cal_bat_mgmt_basic._2step_cv_enable;
    bm_cust.feature_2cc = nvid_cal_bat_mgmt_basic._2step_cc_enable;
    bm_cust.feature_bc12 = nvid_cal_chg_ctl_cfg.bc12_support;
  #if 0  /* not efficient */
    bm_cust.icl = (buf_chg_ctl_cfg[NKCCCC_ICL_CONFIG_HB_IDX] << 8) + buf_chg_ctl_cfg[NKCCCC_ICL_CONFIG_LB_IDX];
    if (bm_cust.feature_bc12) {
        bm_cust.icl = ICL_ITH_443mA;
        bm_cust.icl_value = battery_icl_value[4];
    } else {
        bm_cust.icl = (buf_chg_ctl_cfg[NKCCCC_ICL_CONFIG_HB_IDX] << 8) + buf_chg_ctl_cfg[NKCCCC_ICL_CONFIG_LB_IDX];
        bm_cust.icl_value = battery_icl_value[bm_cust.icl];
    }
  #else
    if (bm_cust.feature_bc12) {
        bm_cust.icl = ICL_ITH_443mA;
    }
    else {
        bm_cust.icl = nvid_cal_chg_ctl_cfg.icl_config;
    }
    bm_cust.icl_value = battery_icl_value[bm_cust.icl];
  #endif
    bm_cust.feature_jeita = nvid_cal_chg_ctl_cfg.jeita_en;
    bm_cust.feature_warm_cool = nvid_cal_chg_ctl_cfg.warm_cool_en;
    bm_cust.feature_preecc_timer = nvid_cal_chg_ctl_cfg.precc_timer_en;
    bm_cust.preecc_timer = nvid_cal_chg_ctl_cfg.precc_safety_time;
    bm_cust.feature_fastcc_timer = nvid_cal_chg_ctl_cfg.fastcc_timer_en;
    bm_cust.fastcc_timer = nvid_cal_chg_ctl_cfg.fastcc_safety_time;
    bm_cust.extern_timer = nvid_cal_chg_ctl_cfg.extend_charging;

    LOG_MSGID_I(battery_management, "[BM_BASIC]Feature List", 0);
    LOG_MSGID_I(battery_management, "[BM_BASIC]2CC[%d]2CV[%d]Feature Preecc_timer[%d]Feature Fastcc_timer[%d]", 4, bm_cust.feature_2cc, bm_cust.feature_2cv, bm_cust.feature_preecc_timer, bm_cust.feature_fastcc_timer);
    LOG_MSGID_I(battery_management, "[BM_BASIC]fastcc_timer[%d]extern_timer[%d]", 2, bm_cust.fastcc_timer, bm_cust.extern_timer);
    LOG_MSGID_I(battery_management, "[BM_BASIC]BC1.2[%d]Jeita[%d]Warm_Cool[%d]", 3, bm_cust.feature_bc12, bm_cust.feature_jeita, bm_cust.feature_warm_cool);
    LOG_MSGID_I(battery_management, "[BM_BASIC]icl[%d]icl_value[%d]", 2, bm_cust.icl, bm_cust.icl_value);

    bm_cust.cool_cc_value = nvid_cal_bat_mgmt_basic.cool_charger_current;
    bm_cust.cool_cc = battery_get_jeita_percentage(bm_cust.cool_cc_value);
    bm_cust.cool_cv = nvid_cal_bat_mgmt_basic.cool_cv_flag;
    bm_cust.warm_cc_value = nvid_cal_bat_mgmt_basic.warm_charger_current;
    bm_cust.warm_cc = battery_get_jeita_percentage(bm_cust.warm_cc_value);
    bm_cust.warm_cv = nvid_cal_bat_mgmt_basic.warm_cv_flag;
    bm_cust.jeita.cold = nvid_cal_bat_mgmt_chr.cold;
    bm_cust.jeita.cool = nvid_cal_bat_mgmt_chr.cool;
    bm_cust.jeita.warm = nvid_cal_bat_mgmt_chr.warm;
    bm_cust.jeita.hot = nvid_cal_bat_mgmt_chr.hot;
    LOG_MSGID_I(battery_management, "[BM_BASIC]cool_cc[%d%]cool_index[%d]cool_cv[%dmV]", 3, bm_cust.cool_cc_value, bm_cust.cool_cc, bm_cust.cool_cv);
    LOG_MSGID_I(battery_management, "[BM_BASIC]warm_cc[%d%]warm_index[%d]warm_cv[%dmV]", 3, bm_cust.warm_cc_value, bm_cust.warm_cc, bm_cust.warm_cv);
    LOG_MSGID_I(battery_management, "[BM_BASIC]cold[%d]cool[%d]warm[%d]hot[%d]", 4, bm_cust.jeita.cold, bm_cust.jeita.cool, bm_cust.jeita.warm, bm_cust.jeita.hot);

    bm_cust.s0_voltage = nvid_cal_bat_mgmt_basic.s0_voltage;
    bm_cust.s0_voltage_index = battery_get_precc_index(bm_cust.s0_voltage);
    bm_cust.s0_chr_cur_value = nvid_cal_bat_mgmt_basic.s0_current / 10;
    bm_cust.s0_chr_cur = battery_get_charger_index(bm_cust.s0_chr_cur_value);
    LOG_MSGID_I(battery_management, "[BM_BASIC]s0_voltage[%dmV]index[%d]s0_cur[%dmA]index[%d]", 4, bm_cust.s0_voltage, bm_cust.s0_voltage_index, bm_cust.s0_chr_cur_value, bm_cust.s0_chr_cur);

    if (bm_cust.feature_2cc == BATTERY_OPERATE_ON) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]2CC feature on", 0);
        bm_cust.s1_voltage = nvid_cal_bat_mgmt_basic.s1_voltage;
        bm_cust.s1_chr_cur_value = nvid_cal_bat_mgmt_basic.s1_multi_level / 10;
        bm_cust.s1_chr_cur = battery_get_charger_index(bm_cust.s1_chr_cur_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]s1_voltage[%dmV]s1_cur[%dmA]index[%d]", 3, bm_cust.s1_voltage, bm_cust.s1_chr_cur_value, bm_cust.s1_chr_cur);

        bm_cust.s2_voltage = nvid_cal_bat_mgmt_basic.s2_voltage;
        bm_cust.s2_chr_cur_value = nvid_cal_bat_mgmt_basic.s2_multi_level / 10;
        bm_cust.s2_chr_cur = battery_get_charger_index(bm_cust.s2_chr_cur_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]s2_voltage[%dmV]s2_cur[%dmA]index[%d]", 3, bm_cust.s2_voltage, bm_cust.s2_chr_cur_value, bm_cust.s2_chr_cur);
    } else {
        LOG_MSGID_I(battery_management, "[BM_BASIC]2CC feature off", 0);
    }
    if (bm_cust.feature_2cv == BATTERY_OPERATE_OFF) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]2CV feature off", 0);
        bm_cust.cv_termination_value = nvid_cal_bat_mgmt_basic.cv_termination_current / 10;
        bm_cust.cv_termination = battery_get_iterm_index(bm_cust.cv_termination_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]iterm[%dmA]index[%d]", 2, bm_cust.cv_termination_value, bm_cust.cv_termination);
        bm_cust.iterm_irq_value = nvid_cal_bat_mgmt_basic.iterm_irq / 10;
        bm_cust.iterm_irq = battery_get_iterm_index(bm_cust.iterm_irq_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]iterm irq[%dmA]index[%d]", 2, bm_cust.iterm_irq_value, bm_cust.iterm_irq);
    } else {
        LOG_MSGID_I(battery_management, "[BM_BASIC]2CV feature on", 0);
        bm_cust.cv_termination_value = nvid_cal_bat_mgmt_basic.cv_termination_current / 10;
        bm_cust.cv_termination = battery_get_iterm_index(bm_cust.cv_termination_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]iterm[%dmA]index[%d]", 2, bm_cust.cv_termination_value, bm_cust.cv_termination);

        bm_cust.iterm_irq_cv1_value = nvid_cal_bat_mgmt_basic._2step_iterm_irq1 / 10;
        bm_cust.iterm_irq_cv1 = battery_get_iterm_index(bm_cust.iterm_irq_cv1_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]iterm_irq_cv1[%dmA]index[%d]", 2, bm_cust.iterm_irq_cv1_value, bm_cust.iterm_irq_cv1);

        bm_cust.cv1_value = nvid_cal_bat_mgmt_basic._2step_cv1;
        bm_cust.cv1 = battery_get_full_battery_index(bm_cust.iterm_irq_cv1_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]cv1_value[%dmV]index[%d]", 2, bm_cust.cv1_value, bm_cust.cv1);

        bm_cust.iterm_irq_cv2_value = nvid_cal_bat_mgmt_basic._2step_iterm_irq2 / 10;
        bm_cust.iterm_irq_cv2 = battery_get_iterm_index(bm_cust.iterm_irq_cv2_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]iterm_irq_cv2[%dmA]index[%d]", 2, bm_cust.iterm_irq_cv2_value, bm_cust.iterm_irq_cv2);

        bm_cust.cv2_value = nvid_cal_bat_mgmt_basic._2step_cv2;
        bm_cust.cv2 = battery_get_full_battery_index(bm_cust.iterm_irq_cv2_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]cv2_value[%dmV]index[%d]", 2, bm_cust.cv2_value, bm_cust.cv2);
    }

    for (i = 0; i < BATTERY_BASIC_CHECKPOINT_TBL_NUM; i++) {
        bm_cust.check_point[i] = nvid_cal_bat_mgmt_chr.check_point_arr[i].check_point;
        LOG_MSGID_I(battery_management, "[BM_BASIC]Check Point[%d][%dmV]  ", 2, i, bm_cust.check_point[i]);
    }
    LOG_MSGID_I(battery_management, "[BM_BASIC]BM Data setting", 0);
    if (bm_cust.feature_2cv == BATTERY_OPERATE_OFF) {
        pmu_select_cv_voltage_hp(battery_get_full_battery_index(bm_cust.full_bat)); //set VCV_VTH
        pmu_set_iterm_current_irq(bm_cust.iterm_irq);
    } else {
        bm_cust.full_bat = bm_cust.cv1_value;
        bm_cust.iterm_irq = bm_cust.iterm_irq_cv1;
        bm_cust.iterm_irq_value = bm_cust.iterm_irq_cv1_value;
        pmu_select_cv_voltage(battery_get_full_battery_index(bm_cust.cv1_value)); //set VCV_VTH
        pmu_set_iterm_current_irq(bm_cust.iterm_irq);
        LOG_MSGID_I(battery_management, "[BM_BASIC][BM_2CV]1step:[%dmV][%dmA] ", 2, bm_cust.full_bat, bm_cust.iterm_irq_value);
    }

    return BATTERY_MANAGEMENT_STATUS_OK;
}
#endif

/*2CV feature default off*/
void battery_set_2cv_parameter(battery_managerment_2step_cv_t step)
{
    int current_step = 0;
    switch (step) {
        case BM_2STEP_SET_PARA:
            if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > bm_cust.cv1) {
                current_step = 2;
                bm_cust.full_bat = bm_cust.cv2;
                bm_cust.iterm_irq = bm_cust.iterm_irq_cv2;
                pmu_set_charger_current(pmu_fastcc_chrcur_50mA);
            } else {
                current_step = 1;
                bm_cust.full_bat = bm_cust.cv1;
                bm_cust.iterm_irq = bm_cust.iterm_irq_cv1;
            }
            pmu_select_cv_voltage(battery_get_full_battery_index(bm_cust.full_bat)); //set VCV_VTH
            pmu_set_iterm_current_irq(bm_cust.iterm_irq);
            LOG_MSGID_I(battery_management, "[BM_CHR][2CV][%d step]SET_PARA:CV[%d]ITERM[%d] ", 3, current_step, bm_cust.full_bat, bm_cust.iterm_irq);
            break;
        case BM_2STEP_SET_CV2:
            bm_cust.full_bat = FULL_BATTERY_CV2;
            bm_cust.iterm_irq = CV2_ITERM_IRQ;
            pmu_select_cv_voltage(battery_get_full_battery_index(bm_cust.full_bat)); //set VCV_VTH
            pmu_set_iterm_current_irq(bm_cust.iterm_irq);
            pmu_set_charger_current(bm_cust.iterm_irq_cv1);
            LOG_MSGID_I(battery_management, "[BM_CHR][2CV]2 Step:CV[%d]ITERM[%d] ", 2, bm_cust.full_bat, bm_cust.iterm_irq);
            break;
        case BM_2STEP_SET_RECHARGER:
            bm_cust.iterm_irq = CV2_ITERM_IRQ;
            pmu_set_charger_current(bm_cust.iterm_irq_cv1);
            LOG_MSGID_I(battery_management, "[BM_CHR][2CV]Re-charger ", 0);
            break;
    }

}

battery_management_status_t battery_init_basic_data()
{
    LOG_MSGID_I(battery_management, "[BM_BASIC]Get battery default value from customer file", 0);
    int i = 0;

    bm_cust.powerhold = BATTERY_FEATURE_POWERHOLD;
    bm_cust.vbus_debounce = BATTERY_VBUS_DEBOUNCE_TIME;
    pmu_set_vbus_debounce_time_hp(bm_cust.vbus_debounce);
    pmu_enable_powerhold_hp(bm_cust.powerhold);
    LOG_MSGID_I(battery_management, "[BM_BASIC]powerhold[%d]vbus_debounce[%d]", 2, bm_cust.powerhold, bm_cust.vbus_debounce);
    bm_cust.initial_bat = INITIAL_BAT;
    bm_cust.shutdown_bat = SW_SHUT_DOWN;
    LOG_MSGID_I(battery_management, "[BM_BASIC]initial_bat[%dmV]shutdown_bat[%dmV]", 2, bm_cust.initial_bat, bm_cust.shutdown_bat);

    bm_cust.full_bat = FULL_BATTERY;
    bm_cust.recharger_voltage = EOC_RECHARGER_VOLTAGE;
    bm_cust.full_bat_offset = BATTERY_CAPACITY_OFFSET;
    bm_cust.recharger_threshold = RECHARGER_VOLTAGE;
    bm_cust.recharger_voltage = EOC_RECHARGER_VOLTAGE;

    LOG_MSGID_I(battery_management, "[BM_BASIC]Full_Vbat[%dmV]recharger_voltage[%dmV]index[%d]full_bat_offset[%dmV]", 4, bm_cust.full_bat, bm_cust.recharger_voltage, bm_cust.recharger_threshold, bm_cust.full_bat_offset);

    bm_cust.precc_cur = BATTERY_PRECC_CURRENT;
    bm_cust.precc_cur_value = bm_cc_cur[BATTERY_PRECC_CURRENT];
    LOG_MSGID_I(battery_management, "[BM_BASIC]Precc_cur[%dmA]index[%d]", 2, bm_cust.precc_cur_value, bm_cust.precc_cur);

    bm_cust.feature_2cc = BATTERY_FEATURE_2CC;
    bm_cust.feature_2cv = BATTERY_FEATURE_2CV;
    bm_cust.feature_bc12 = BATTERY_FEATURE_BC12;
    if (bm_cust.feature_bc12) {
        bm_cust.icl = ICL_ITH_443mA;
        bm_cust.icl_value = battery_icl_value[bm_cust.icl];
    } else {
        bm_cust.icl = BATTERY_WO_BC12_ICL;
        bm_cust.icl_value = battery_icl_value[bm_cust.icl];
    }
    bm_cust.feature_jeita = BATTERY_FEATURE_JEITA;
    bm_cust.feature_warm_cool = BATTERY_FEATURE_JEITA_WARM_COOL;
    bm_cust.feature_preecc_timer = BATTERY_FEATURE_PREECC_TIMER;
    bm_cust.preecc_timer = BATTERY_PRECC_TIMER;
    bm_cust.feature_fastcc_timer = BATTERY_FEATURE_FASTCC_TIMER;
    bm_cust.fastcc_timer = BATTERY_FASTCC_TIMER;
    bm_cust.extern_timer = BATTERY_EXTERNAL_TIMER;

    LOG_MSGID_I(battery_management, "[BM_BASIC]Feature List", 0);
    LOG_MSGID_I(battery_management, "[BM_BASIC]2CC[%d]2CV[%d]Feature Preecc_timer[%d]Feature Fastcc_timer[%d]", 4, bm_cust.feature_2cc, bm_cust.feature_2cv, bm_cust.feature_preecc_timer, bm_cust.feature_fastcc_timer);
    LOG_MSGID_I(battery_management, "[BM_BASIC]fastcc_timer[%d]extern_timer[%d]", 2, bm_cust.fastcc_timer, bm_cust.extern_timer);
    LOG_MSGID_I(battery_management, "[BM_BASIC]BC1.2[%d]Jeita[%d]Warm_Cool[%d]", 3, bm_cust.feature_bc12, bm_cust.feature_jeita, bm_cust.feature_warm_cool);
    LOG_MSGID_I(battery_management, "[BM_BASIC]icl[%d]icl_value[%d]", 2, bm_cust.icl, bm_cust.icl_value);

    bm_cust.cool_cc = BATTERY_COOL_CC;
    bm_cust.cool_cv = BATTERY_COOL_CV;
    bm_cust.cool_cc_value = battery_jeita_percentage[bm_cust.cool_cc];
    bm_cust.warm_cc = BATTERY_WARM_CC;
    bm_cust.warm_cv = BATTERY_WARM_CV;
    bm_cust.warm_cc_value = battery_jeita_percentage[bm_cust.warm_cc];
    bm_cust.jeita.cold = HW_JEITA_COLD_THRESHOLD;
    bm_cust.jeita.cool = HW_JEITA_COOL_THRESHOLD;
    bm_cust.jeita.warm = HW_JEITA_WARM_THRESHOLD;
    bm_cust.jeita.hot = HW_JEITA_HOT_THRESHOLD;
    LOG_MSGID_I(battery_management, "[BM_BASIC]cool_cc[%d%]cool_index[%d]cool_cv[%dmV]", 3, bm_cust.cool_cc_value, bm_cust.cool_cc, bm_cust.cool_cv);
    LOG_MSGID_I(battery_management, "[BM_BASIC]warm_cc[%d%]warm_index[%d]warm_cv[%dmV]", 3, bm_cust.warm_cc_value, bm_cust.warm_cc, bm_cust.warm_cv);
    LOG_MSGID_I(battery_management, "[BM_BASIC]cold[%d]cool[%d]warm[%d]hot[%d]", 4, bm_cust.jeita.cold, bm_cust.jeita.cool, bm_cust.jeita.warm, bm_cust.jeita.hot);
    bm_cust.s0_voltage_index = BATTERY_S0_VOLTAGE;
    bm_cust.s0_voltage = precc_voltage[bm_cust.s0_voltage_index];
    bm_cust.s0_chr_cur_value = bm_cc_cur[BATTERY_S0_CHR_CUR];
    bm_cust.s0_chr_cur = BATTERY_S0_CHR_CUR;
    LOG_MSGID_I(battery_management, "[BM_BASIC]s0_voltage[%dmV]index[%d]s0_cur[%dmA]index[%d]", 4, bm_cust.s0_voltage, bm_cust.s0_voltage_index, bm_cust.s0_chr_cur_value, bm_cust.s0_chr_cur);

    if (bm_cust.feature_2cc == BATTERY_OPERATE_ON) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]2CC feature on", 0);
        bm_cust.s1_voltage = BATTERY_S1_VOLTAGE;
        bm_cust.s1_chr_cur_value = bm_cc_cur[BATTERY_S1_CHR_CUR];;
        bm_cust.s1_chr_cur = BATTERY_S1_CHR_CUR;
        LOG_MSGID_I(battery_management, "[BM_BASIC]s1_voltage[%dmV]s1_cur[%dmA]index[%d]", 3, bm_cust.s1_voltage, bm_cust.s1_chr_cur_value, bm_cust.s1_chr_cur);

        bm_cust.s2_voltage = BATTERY_S2_VOLTAGE;
        bm_cust.s2_chr_cur_value = bm_cc_cur[BATTERY_S2_CHR_CUR];;
        bm_cust.s2_chr_cur = BATTERY_S2_CHR_CUR;
        LOG_MSGID_I(battery_management, "[BM_BASIC]s2_voltage[%dmV]s2_cur[%dmA]index[%d]", 3, bm_cust.s2_voltage, bm_cust.s2_chr_cur_value, bm_cust.s2_chr_cur);
    } else {
        LOG_MSGID_I(battery_management, "[BM_BASIC]2CC feature off", 0);
    }
    if (bm_cust.feature_2cv == BATTERY_OPERATE_OFF) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]2CV feature off", 0);
        bm_cust.cv_termination = BATTERY_ITERM_ITH;
        bm_cust.cv_termination_value = bm_iterm_current[BATTERY_ITERM_ITH];
        LOG_MSGID_I(battery_management, "[BM_BASIC]iterm[%dmA]index[%d]", 2, bm_cust.cv_termination_value, bm_cust.cv_termination);
        bm_cust.iterm_irq = BATTERY_ITERM_CURRENT_IRQ;
        bm_cust.iterm_irq_value = bm_iterm_current[bm_cust.iterm_irq];

        LOG_MSGID_I(battery_management, "[BM_BASIC]iterm irq[%dmA]index[%d]", 2, bm_cust.iterm_irq_value, bm_cust.iterm_irq);
    } else {
        bm_cust.cv_termination = BATTERY_ITERM_ITH;
        bm_cust.cv_termination_value = bm_iterm_current[BATTERY_ITERM_ITH];
        LOG_MSGID_I(battery_management, "[BM_BASIC]2CV feature on", 0);

        bm_cust.iterm_irq_cv1 = CV1_ITERM_IRQ;
        bm_cust.iterm_irq_cv1_value = bm_iterm_current[bm_cust.iterm_irq_cv1];
        LOG_MSGID_I(battery_management, "[BM_BASIC]iterm_irq_cv1[%dmA]index[%d]", 2, bm_cust.iterm_irq_cv1_value, bm_cust.iterm_irq_cv1);

        bm_cust.cv1_value = FULL_BATTERY_CV1;
        bm_cust.cv1 = battery_get_full_battery_index(bm_cust.cv1_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]cv1_value[%dmV]index[%d]", 2, bm_cust.cv1_value, bm_cust.cv1);

        bm_cust.iterm_irq_cv2 = CV2_ITERM_IRQ;
        bm_cust.iterm_irq_cv2_value = bm_iterm_current[bm_cust.iterm_irq_cv2];
        LOG_MSGID_I(battery_management, "[BM_BASIC]iterm_irq_cv2[%dmA]index[%d]", 2, bm_cust.iterm_irq_cv2_value, bm_cust.iterm_irq_cv2);

        bm_cust.cv2_value = FULL_BATTERY_CV2;
        bm_cust.cv2 = battery_get_full_battery_index(bm_cust.cv2_value);
        LOG_MSGID_I(battery_management, "[BM_BASIC]cv2_value[%dmV]index[%d]", 2, bm_cust.cv2_value, bm_cust.cv2);
    }

    for (i = 0; i < BATTERY_BASIC_CHECKPOINT_TBL_NUM; i++) {
        bm_cust.check_point[i] = battery_basic_checkpoint[i];
        LOG_MSGID_I(battery_management, "Check Point[%d][%d]  ", 2, i, bm_cust.check_point[i]);
    }
    LOG_MSGID_I(battery_management, "[BM_BASIC]BM Data setting", 0);
    if (bm_cust.feature_2cv == BATTERY_OPERATE_OFF) {
        pmu_select_cv_voltage_hp(battery_get_full_battery_index(bm_cust.full_bat)); //set VCV_VTH
        pmu_set_iterm_current_irq(bm_cust.iterm_irq);
    } else {
        bm_cust.full_bat = bm_cust.cv1_value;
        bm_cust.iterm_irq = bm_cust.iterm_irq_cv1;
        bm_cust.iterm_irq_value = bm_cust.iterm_irq_cv1_value;
        pmu_select_cv_voltage(battery_get_full_battery_index(bm_cust.cv1_value)); //set VCV_VTH
        pmu_set_iterm_current_irq(bm_cust.iterm_irq);
        LOG_MSGID_I(battery_management, "[BM_BASIC][BM_2CV]1step:[%dmV][%dmA] ", 2, bm_cust.full_bat, bm_cust.iterm_irq_value);
    }
    return BATTERY_MANAGEMENT_STATUS_OK;
}

void battery_notification(battery_management_event_t event, uint32_t chr_exist, uint32_t state)
{
    int i = 0;
    LOG_MSGID_I(battery_management, "[BM_BASIC][BM Notification : %d][Chr_exist:%d][Chr_state:%d] ", 3, event, (int)chr_exist, (int)state); /*Log output by BT*/
    for (i = 0; i < battery_callback_index; i++) {
        if ((bmt_callback[i].callback_init == true) && (bmt_callback[i].func != NULL)) {
            bmt_callback[i].func(event, &bm_ctrl_info);
        }
    }
}

uint8_t battery_get_full_battery_index(uint16_t vabt)
{
    int i = 0, index = 3; //Battery default value
    int battery_full_bat_voltage[BATTERY_FULLBAT_INDEX_MAX] = { 4050, 4075, 4100, 4125, 4150, 4175, 4200, 4225, 4250, 4275, 4300, 4325, 4350, 4375, 4400, 4425, 4450, 4475, 4500, 4525, 4550, 4575, 4600};
    for (i = 0; i < BATTERY_FULLBAT_INDEX_MAX; i++) {
        if (vabt == battery_full_bat_voltage[i]) {
            index = i;
        }
    }
    return index;
}

uint8_t battery_get_recharger_index(uint16_t vol)
{
    int i = 0, index = 1; //recharger default value
    int battery_recharger_threshold[BATTERY_RECHARGER_INDEX_MAX] = { 50, 100, 150, 200 };
    for (i = 0; i < BATTERY_RECHARGER_INDEX_MAX; i++) {
        if (vol == battery_recharger_threshold[i]) {
            index = i;
        }
    }
    return index;
}

void battery_charger_plugin_initial_setting(void)
{
    pmu_enable_recharger(BATTERY_OPERATE_ON);
    pmu_set_rechg_voltage(bm_cust.recharger_threshold);//Setting recharger voltage
}

battery_management_status_t battery_management_register_callback_internal(battery_management_callback_t callback)
{
    bmt_callback[battery_callback_index].func = callback;
    bmt_callback[battery_callback_index].callback_init = true;
    battery_callback_index++;
    return BATTERY_MANAGEMENT_STATUS_OK;
}

void battery_eoc_option_setting(TimerHandle_t pxTimer)
{
    if (bm_ctrl_info.feature.charger_option != 1) {
        pmu_select_eoc_option_operating(bm_ctrl_info.feature.charger_option, option_setting);
    }
}

void battery_unlock_sleep(void)
{
    uint8_t bm_lock = 0;
    do {
        bm_lock = sleep_management_check_handle_status(SLEEP_LOCK_BATTERY_MANAGEMENT);
        if (sleep_management_check_handle_status(SLEEP_LOCK_BATTERY_MANAGEMENT) >= 1) {
            hal_sleep_manager_unlock_sleep(SLEEP_LOCK_BATTERY_MANAGEMENT);
        }
    } while (bm_lock >= 1);
    LOG_MSGID_I(battery_management, "[BM_BASIC]battery unlock sleep: %d\n", 1, sleep_management_check_handle_status(SLEEP_LOCK_BATTERY_MANAGEMENT));
}

void battery_set_charger_step(void)
{
    uint32_t temp_batsns = 0;
    temp_batsns = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
    if (bm_cust.feature_2cc == BATTERY_OPERATE_ON) {
        if (temp_batsns > bm_cust.s2_voltage) {
            LOG_MSGID_I(battery_management, "[BM_CHR]Battery voltage detect step charger S2 %d", 1, bm_cust.s2_voltage);
            if (bm_cust.feature_2cv == BATTERY_OPERATE_OFF) {
                pmu_set_charger_current(bm_cust.s2_chr_cur);
            } else {
                if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > bm_cust.cv1) {
                    pmu_set_charger_current(bm_cust.s1_chr_cur);
                } else {
                    pmu_set_charger_current(bm_cust.s2_chr_cur);
                }
            }
        } else if ((temp_batsns >= bm_cust.s1_voltage) && (temp_batsns <= bm_cust.s2_voltage)) {
            LOG_MSGID_I(battery_management, "[BM_CHR]Battery voltage detect charger S1 %d", 1, bm_cust.s1_voltage);
            pmu_set_charger_current(bm_cust.s1_chr_cur);
        } else if ((temp_batsns <= bm_cust.s1_voltage) && (temp_batsns >= bm_cust.s0_voltage)) {
            LOG_MSGID_I(battery_management, "[BM_CHR]Battery voltage detect charger S0 %d", 1, bm_cust.s0_voltage);
            pmu_set_charger_current(bm_cust.s0_chr_cur);
        } else if (temp_batsns <= bm_cust.s0_voltage) { /*Note : batsns <S0 means  PRECC*/
            LOG_MSGID_I(battery_management, "[BM_CHR]Battery voltage enter precc ", 0);
            pmu_set_pre_charger_current(bm_cust.precc_cur);
        } else {
            LOG_MSGID_I(battery_management, "[BM_CHR]ERROR", 0);
        }
    }
}

void battery_set_charger_step_timer(uint8_t cur, uint8_t next)
{
    bm_ctrl_info.charger_step = next;
    LOG_MSGID_I(battery_management, "[BM_CHR]BM_CHARGER_STEP[%d]->[%d]", 2, cur, bm_ctrl_info.charger_step);
    xTimerStopFromISR(xbm_chr_detec_t, 0);
    if (xTimerStartFromISR(xbm_chr_detec_t, 0) != pdPASS) {
        LOG_MSGID_I(battery_management, "[BM_CHR]xbm_chr_detec_t xTimerStart fail\n", 0);
    }
}

void battery_charger_task(void *pvParameters)
{
    const TickType_t xDelay = (CHARGER_REGULAR_TIME * TIMEOUT_PERIOD_1S) / portTICK_PERIOD_MS;
    while (1) {
        vTaskDelay(xDelay);
        LOG_MSGID_I(battery_management, "[BM_BASIC]BM regular timer check ", 0);
        if (bm_cust.feature_2cv == BATTERY_OPERATE_OFF) {
            if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > (bm_cust.full_bat - bm_cust.full_bat_offset)) {
                battery_calibration_gauge_tolerance();
            }
        } else {
            if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > (bm_cust.cv2 - bm_cust.full_bat_offset)) {
                battery_calibration_gauge_tolerance();
            }
        }
        battery_set_charger_step();
        LOG_W(MPLOG, "[BM_BASIC]Battery temperature = %d(Celsius degrees)", (int)battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE)); /*Log output by BT*/
        LOG_W(MPLOG, "[BM_BASIC]Charger status = %d ", (int)battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST)); /*Log output by BT*/
        LOG_W(MPLOG, "[BM_BASIC]Battery voltage = %d(mV)\n", (int) battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE)); /*Log output by BT*/
        LOG_W(MPLOG, "[BM_BASIC]Battery State = %s\n", bm_charger_state[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE)]); /*Log output by BT*/
#ifdef MTK_FUEL_GAUGE
        LOG_W(MPLOG, "[BM_BASIC]FG_SOC = %d(%%) ", (int)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY)); /*Log output by BT*/
        LOG_W(MPLOG, "[BM_BASIC]UI_SOC = %d(%%) ", (int)battery_management_get_battery_property(BATTERY_PROPERTY_UI_SOC)); /*Log output by BT*/
#else
        LOG_W(MPLOG, "[BM_BASIC]Battery capacity = %d(%%) ", (int)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY)); /*Log output by BT*/
#endif
    }
}

/*==========[Other : Additional features]==========*/
void battery_management_enter_eoc_rtc_mode(void){
    LOG_MSGID_I(battery_management, "[BM_CHR]battery_enter_eoc_rtc_mode", 0);
    if (bm_ctrl_info.feature.charger_option == 4) {
        pmu_select_eoc_option_operating(pmu_eoc_option4, option4_init);
    }
}
#ifdef CHARGER_CALIBRATION
/*This API is for fixing human actions or environment exceptions in the charger plug-in/plug-out*/
void battery_detect_calibration_timer_callback(TimerHandle_t pxTimer)
{
    uint32_t mask_pri;
    hal_nvic_save_and_set_interrupt_mask_special(&mask_pri);
    if ((bm_ctrl_info.isChargerExist != pmu_get_chr_detect_value())) {
		hal_nvic_restore_interrupt_mask_special(mask_pri);
        LOG_MSGID_I(battery_management, "[BM_CHR]ERROR!!!!, Plug-in/out interval need 500ms ", 0);
        battery_charger_detect_callback();
        pmu_clear_interrupt(PMU_INT_MAX);
    }else{
		hal_nvic_restore_interrupt_mask_special(mask_pri);
	}
}
#endif
uint32_t battery_management_set_register_value(unsigned short int address, unsigned short int mask, unsigned short int shift, unsigned short int value)
{
    pmu_set_register_value(address, mask, shift, value);
    return pmu_get_register_value(address, mask, shift);
}
uint32_t battery_management_get_register_value(unsigned short int address, unsigned short int mask, unsigned short int shift)
{
    return pmu_get_register_value(address, mask, shift);
}

uint8_t battery_get_charger_index(int value)
{
    int index = 0;
    int i = 0;
    for (i = 0; i < BM_CC_CUR_TBL_NUM; i++) {
        if (value == bm_cc_cur[i]) {
            index = i;
        }
    }
    return index;
}
uint8_t battery_get_iterm_index(int value)
{
    int index = 0;
    int i = 0;
    for (i = 0; i < BM_ITERM_CURRENT_TBL_NUM; i++) {
        if (value == bm_iterm_current[i]) {
            index = i;
        }
    }
    return index;
}
uint8_t battery_get_jeita_percentage(uint8_t value)
{
    int index = 0;
    int i = 0;
    for (i = 0; i < BATTERY_JEITA_PERCENTAGE_TBL_NUM; i++) {
        if (value == battery_jeita_percentage[i]) {
            index = i;
        }
    }
    return index;
}
uint8_t battery_get_icl_value(uint8_t value)
{
    int index = 0;
    int i = 0;
    for (i = 0; i < BATTERY_ICL_VALUE_TBL_NUM; i++) {
        if (value == battery_icl_value[i]) {
            index = i;
        }
    }
    return index;
}
uint8_t battery_get_precc_index(uint32_t value)
{
    int index = 0;
    int i = 0;
    for (i = 0; i < PRECC_VOLTAGE_TBL_NUM; i++) {
        if (value == precc_voltage[i]) {
            index = i;
        }
    }
    return index;
}


#ifdef BATTERY_CUSTOMER_SETTING
battery_customer_setting_t bm_customer_setting[] = {
    { 0, 0 },
    { 0, 0 }
};
void battery_manager_customer_setting()
{
    int custom_setting, i;
    custom_setting = sizeof(bm_customer_setting) / sizeof(bm_customer_setting[0]);
    for (i = 0; i < custom_setting; i++) {
        pmu_set_register_value_2byte_mt6388(bm_customer_setting[i].addr, 0xFFFF, 0, bm_customer_setting[i].value);
    }
}
#endif
