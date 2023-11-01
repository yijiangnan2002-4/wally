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
#include "battery_management_customer_file_lp.h"
#include "battery_management_sw_ntc.h"
#include "task.h"
#include "task_def.h"
#include "hal_flash.h"
#include "hal_nvic_internal.h"
#include "hal_sleep_manager_internal.h"
#include "hal_pmu_charger_lp.h"
#ifdef HAL_USB_MODULE_ENABLED
#include "hal_usb.h"
#include "hal_usb_internal.h"
#endif
#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#endif

#include "hal_pmu_cal_lp.h"
uint8_t battery_sleep_handle;                                       /*Data : Battery management handle for lock sleep */
uint8_t gauge_cardinal_number;                                      /*Data : For gauge calculation*/
uint8_t battery_align_flag = 0;                                     /*flag : Avoid VBUS shaking, ensure the Irq data is consistent this time */
uint8_t battery_init_setting_flag = 0;                              /*flag : Check battery init setting is finish or not*/
static uint32_t battery_callback_index = 0;                         /*Data : Restore register callback function number */
TimerHandle_t xbm_chr_detec_t;                                      /*Timer : When PMIC irq"CHRDET" is Triggered, this timer will receive from pmic irq regiseter callbcck for processing*/
TimerHandle_t xbm_eoc_timer;                                        /*Timer : 1st enter the EOC will implement , to ensure that EOC verification can be performed without battery*/

TimerHandle_t xbm_chrdet_calibration_timer;                         /*Timer : Avoid unexpected problems caused by irq shake*/
TimerHandle_t xbm_chager_detect_timer;                              /*Timer : one shot for charger detect*/

battery_basic_data bm_cust;                                         /*Data : restore battery basic data*/
battery_managerment_control_info_t bm_ctrl_info;                    /*Data : restore battery info*/

const battery_managerment_control_info_t *bm_ctrl_info_p;           /*Data : used for battery monitor callback function */
TaskHandle_t battery_regular_task_t = NULL;                         /*Task : create regular task for eco option 3 and gauge*/
TaskHandle_t battery_lineat_task_t = NULL;                          /*Task : create regular task for get_smooth_vbat*/
TaskHandle_t battery_jeita_task_t = NULL;                           /*Task : create regular task for jeita get temperature*/
log_create_module(battery_management, PRINT_LEVEL_INFO);            /*Syslog create*/
static bmt_callback_context_t bmt_callback[BATTERY_MANAGER_CALLBACK_FUNCTION_NUMBER]; /*Data : callback function*/

const char *bm_charger_type[10] = { "", "SDP", "CDP", "DCP", "SS", "IPAD2_IPAD4", "IPHONE_5V_1A", "NON-STD", "DP/DM_Floating", "UNABLE_TO_IDENTIFY"};    /*Data : For output log */
const char *bm_ntc_state[6] = { "NORM", "WARM", "COOL", "HOT", "COLD", "PWR_OFF"};    /*Data : For output log */
const char *bm_charger_state[9] = { "CHARGER OFF", "FASTCC_P", "FASTCC_B", "FASTCC", "FASTCC_V1", "FASTCC_V2", "EOC", "RECHG", "THR"};                            /*Data : For output log */
extern hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);
extern hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask);
extern pmu_status_t pmu_register_callback_lp(pmu_interrupt_index_lp_t pmu_int_ch, pmu_callback_t callback, void *user_data);
extern void battery_jeita_task(void *pvParameters);

// richard for customer UI spec
//extern int g_ntc_temp;
extern vbat_volt_cfg_t pmu_vbat_volt;
extern pmu_chg_info_t pmu_chg_info;
#include "hal_pmu.h"
/*==========[Battery management API]=========*/
int32_t battery_management_get_battery_property_internal(battery_property_t property) {
    uint32_t property_value = 0;
    switch (property) {
    case BATTERY_PROPERTY_CAPACITY:
#ifdef EXTERNAL_CHARGER
            /* add customize external charger api for calculating the
            percentage (0~100%) of the battery */
#else
        property_value = battery_get_gauge_percent();
#endif
        break;
    case BATTERY_PROPERTY_CAPACITY_LEVEL:
#ifdef EXTERNAL_CHARGER
            /* add customize external charger api for calculating the
            level of the battery */
#else
        property_value = (battery_get_gauge_percent()/10);
#endif
        break;
    case BATTERY_PROPERTY_CHARGER_EXIST:
        property_value = pmu_get_chr_detect_value();
        break;
    case BATTERY_PROPERTY_CHARGER_TYPE:
        property_value = bm_ctrl_info.chargerType;
        break;
    case BATTERY_PROPERTY_TEMPERATURE:
#ifdef EXTERNAL_CHARGER
            /*add customize external charger api for retrieving the charger's temperature*/
#else
        if (bm_cust.feature_jeita == BATTERY_OPERATE_ON) {
		// richard for customer UI spec
//            property_value = g_ntc_temp;
            property_value = bm_ctrl_info.temperature;
        } else {
            property_value = 25;
        }
#endif
        break;
    case BATTERY_PROPERTY_VOLTAGE:
#ifdef BATTERY_LINEAR_GAUGE
        property_value = get_smooth_vbat();
#else
        property_value = pmu_auxadc_get_channel_value(PMU_AUX_VBAT);
#endif
        break;
    case BATTERY_PROPERTY_VOLTAGE_IN_PERCENT:
        property_value = battery_get_gauge_percent();
        break;
    case BATTERY_PROPERTY_PMIC_TEMPERATURE:
        property_value = -1;
        break;
    case BATTERY_PROPERTY_CHARGER_STATE:
        property_value = pmu_get_charger_state();
        break;
    default:
        break;
    } LOG_MSGID_I(battery_management, "[BM_BASIC]property_value : %d %d\r\n", 2,property,property_value);
    return property_value;
}

/*==========[Battery Management Callback Function]==========*/
void battery_eoc_timer_callback(TimerHandle_t pxTimer) {
    LOG_MSGID_I(battery_management,"[BM_CHR]First times enter EOC state, stop timer",0);
    bm_ctrl_info.charger_eoc_state = 1;
    battery_charger_state_change_callback();
}

void battery_charger_state_change_callback(void) {
    uint32_t newState = pmu_get_charger_state();
    switch (newState) {
    case CHARGER_STATE_CHR_OFF:
        break;
    case CHARGER_STATE_FASTCC_P:
    case CHARGER_STATE_FASTCC:
        LOG_MSGID_I(battery_management, "[BM_CHR]FASTCC State\r\n", 0);
        break;

    case CHARGER_STATE_EOC:
        LOG_MSGID_I(battery_management, "[BM_CHR]EOC\r\n", 0);
        break;

    case CHARGER_STATE_RECHG:
        LOG_MSGID_I(battery_management, "[BM_CHR]RECHG State\r\n", 0);
        break;
    default:
        break;
    }
    bm_ctrl_info.chargerState = newState;
    battery_notification(BATTERY_MANAGEMENT_EVENT_CHARGER_STATE_UPDATE, pmu_get_chr_detect_value(), newState);
}

battery_basic_data battery_management_get_basic_data() {
    return bm_cust;
}
void battery_set_charger_step_timer(uint8_t cur, uint8_t next) {
    bm_ctrl_info.charger_step = next;
    LOG_MSGID_I(battery_management, "[BM_CHR]BM_CHARGER_STEP[%d]->[%d]",2,cur,bm_ctrl_info.charger_step);
    xTimerStopFromISR(xbm_chr_detec_t, 0);
    if (xTimerStartFromISR(xbm_chr_detec_t, 0) != pdPASS) {
        LOG_MSGID_I(battery_management, "[BM_CHR]xbm_chr_detec_t xTimerStart fail\n", 0);
    }
}
void battery_charger_setting(TimerHandle_t pxTimer) {
    switch (bm_ctrl_info.charger_step) {
    case BM_CHARGER_IN_USB:
#ifdef HAL_USB_MODULE_ENABLED
         hal_usb_phy_preinit();
#endif
         battery_set_charger_step_timer(BM_CHARGER_IN_USB, BM_CHARGER_IN_BC12);
    break;
    case BM_CHARGER_IN_BC12:
#ifdef BATTERY_FEATURE_BC12
#ifndef AIR_WIRELESS_MIC_ENABLE
        bm_ctrl_info.chargerType = pmu_get_bc12_charger_type();
#else
        bm_ctrl_info.chargerType = NON_STD_CHARGER;
#endif
#endif
        battery_set_charger_step_timer(BM_CHARGER_IN_BC12, BM_CHARGER_NOTIFICATION);
        break;
    case BM_CHARGER_NOTIFICATION:
#ifdef CHARGER_CALIBRATION
        xTimerStartFromISR(xbm_chrdet_calibration_timer, 0);
#endif
        bm_ctrl_info.charger_step = BM_CHARGER_DONE;
        xTimerStopFromISR(xbm_chr_detec_t, 0);
        LOG_MSGID_I(battery_management, "[BM_CHR]PMU sleep handle %d\n", 1,sleep_management_check_handle_status(battery_sleep_handle));
        battery_notification(BATTERY_MANAGEMENT_EVENT_CHARGER_EXIST_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
        battery_charger_state_change_callback();
        break;
    default:
        LOG_MSGID_I(battery_management, "[BM_CHR]AB1565 charger step fail\n", 0);
        break;
    }
}

void battery_one_shot_detect_callback(TimerHandle_t pxTimer) {
    LOG_MSGID_I(battery_management, "[BM_CHR]battery_one_shot_detect_callback\r\n", 0);
    battery_notification(BATTERY_MANAGEMENT_EVENT_CHARGER_EXIST_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
    battery_notification(BATTERY_MANAGEMENT_EVENT_BATTERY_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
}

/*==========[Battery Management init]==========*/
bmt_function_t battery_func = {
        battery_management_init_internal, battery_management_get_battery_property_internal, battery_management_register_callback_internal,
        NULL };

void battery_charger_in_callback() {
    battery_set_enable_charger(BATTERY_OPERATE_ON);
    bm_ctrl_info.isChargerExist = pmu_get_chr_detect_value();
    bm_ctrl_info.charger_step = BM_CHARGER_IN_USB;
    if(xTimerStartFromISR(xbm_chr_detec_t, 0) != pdPASS) {
        LOG_MSGID_I(battery_management, "xbm_chr_detec_t xTimerStart fail\n", 0);
    }
}

void battery_charger_out_callback() {
    battery_set_enable_charger(BATTERY_OPERATE_OFF);
    bm_ctrl_info.isChargerExist = pmu_get_chr_detect_value();
    xTimerStopFromISR(xbm_chr_detec_t, 0);
    bm_ctrl_info.charger_step = BM_CHARGER_NOTIFICATION;
    if(xTimerStartFromISR(xbm_chr_detec_t, 0) != pdPASS) {
        LOG_MSGID_I(battery_management, "[BM_CHR]xbm_chr_detec_t xTimerStart fail\n", 0);
    }
}

void battery_charger_compl_callback() {
    battery_charger_state_change_callback();
}
void battery_charger_rechg_callback() {
    battery_charger_state_change_callback();
}
void battery_monitor(battery_management_event_t event, const void *data) {
    bm_ctrl_info_p = data;
    if (event == BATTERY_MANAGEMENT_EVENT_CHARGER_EXIST_UPDATE) {
        LOG_MSGID_I(battery_management,"[BM_BASIC]EVENT:[CHARGER EXIST UPDATE:%d] [Charger Exist:%x] ",2,event,bm_ctrl_info_p->isChargerExist);
    }
    if (event == BATTERY_MANAGEMENT_EVENT_CHARGER_STATE_UPDATE) {
        LOG_MSGID_I(battery_management,"[BM_BASIC]EVENT:[CHARGER STATE UPDATE:%d] [Charger State:%x]",2,event,pmu_get_charger_state());
    }
}

void battery_management_interrupt_register(void) {
    pmu_register_callback_lp(RG_CHG_IN_INT_FLAG, (pmu_callback_t) battery_charger_in_callback, NULL);
    pmu_register_callback_lp(RG_CHG_OUT_INT_FLAG, (pmu_callback_t) battery_charger_out_callback, NULL);
    pmu_register_callback_lp(RG_CHG_COMPLETE_INT_FLAG, (pmu_callback_t) battery_charger_compl_callback, NULL);
    pmu_register_callback_lp(RG_CHG_RECHG_INT_FLAG, (pmu_callback_t) battery_charger_rechg_callback, NULL);

    if (battery_management_register_callback(battery_monitor) != BATTERY_MANAGEMENT_STATUS_OK) {
        LOG_MSGID_I(battery_management,"[BM_BASIC]Cannot register battery callback",0);
    }
}

void battery_charger_task(void *pvParameters) {

    const TickType_t xDelay = (CHARGER_REGULAR_TIME * TIMEOUT_PERIOD_1S) / portTICK_PERIOD_MS;
    while (1) {
        vTaskDelay(xDelay);
        LOG_MSGID_I(battery_management, "[BM_BASIC]BM regular timer check\r\n", 0);
        LOG_W(MPLOG,"[BM_BASIC]Battery temperature = %d(Celsius degrees)" ,(int)battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE));/*Log output by BT*/
        LOG_W(MPLOG,"[BM_BASIC]Battery capacity = %d(%%) ",(int)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY));/*Log output by BT*/
        LOG_W(MPLOG,"[BM_BASIC]Charger status = %d ",(int)battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST));/*Log output by BT*/
        LOG_W(MPLOG,"[BM_BASIC]Battery voltage = %d(mV)\n",(int) battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE));/*Log output by BT*/
        LOG_W(MPLOG,"[BM_BASIC]Battery State = %s\n",bm_charger_state[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE)]);/*Log output by BT*/
    }
}

void battery_timer_init(void) {
    xTaskCreate(battery_charger_task, "charger_task", 384, NULL, TASK_PRIORITY_NORMAL, &battery_regular_task_t);
#ifdef BATTERY_LINEAR_GAUGE
    xTaskCreate(battery_linear_gauge_task, "Linear_task", 384, NULL, TASK_PRIORITY_NORMAL, &battery_lineat_task_t);
#endif
    xbm_chr_detec_t = xTimerCreate("charger_regular_timer", 10 * TIMEOUT_PERIOD_1MS, pdFALSE, NULL, battery_charger_setting);
    bm_cust.feature_jeita = pmu_ntc_get_enable_status();
    if (bm_cust.feature_jeita == BATTERY_OPERATE_ON) {
        xTaskCreate(battery_jeita_task, "jeita_task", 384, NULL, TASK_PRIORITY_NORMAL, &battery_jeita_task_t);
        LOG_MSGID_I(battery_management, "[BM_BASIC]battery_sw_ntc_feature enable:%d \n", 1,bm_cust.feature_jeita);
    } else {
        LOG_MSGID_I(battery_management, "[BM_BASIC]battery_sw_ntc_feature disable:%d \n", 1,bm_cust.feature_jeita);
    }
#ifdef CHARGER_CALIBRATION
    xbm_chrdet_calibration_timer = xTimerCreate("charger_detect_Timer", (CALIBRATION_TIME * portTICK_PERIOD_MS), pdFALSE, NULL,
            battery_detect_calibration_timer_callback);
#endif
    xbm_chager_detect_timer = xTimerCreate("charger_detect", (1 * TIMEOUT_PERIOD_1S), pdFALSE, NULL, battery_one_shot_detect_callback);
    xTimerStart(xbm_chager_detect_timer, 0);
    xbm_eoc_timer = xTimerCreate("charger_eoc_timer", (BATTERY_EOC_CHECK_TIME * TIMEOUT_PERIOD_1S), pdFALSE, NULL, battery_eoc_timer_callback);
}

battery_management_status_t battery_management_init_internal(void) {
    LOG_MSGID_I(battery_management,"[BM_BASIC]Battery Management V4.0.5",0);
    battery_init_basic_data();
    battery_timer_init();
    battery_gauge_init();
    LOG_W(MPLOG,"[BM_BASIC]Battery temperature = %d(Celsius degrees)" ,(int)battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE));/*Log output by BT*/
    LOG_W(MPLOG,"[BM_BASIC]Charger status = %d ",(int)battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST));/*Log output by BT*/
    LOG_W(MPLOG,"[BM_BASIC]Battery voltage = %d(mV)\n",(int) battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE));/*Log output by BT*/
    LOG_W(MPLOG,"[BM_BASIC]Battery State = %s\n",bm_charger_state[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE)]);/*Log output by BT*/
    LOG_W(MPLOG,"[BM_BASIC]Battery capacity = %d(%%) ",(int)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY));/*Log output by BT*/
#if 0
    if ((pmu_get_usb_input_status() != true)
            && (bm_cust.initial_bat > battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE)
                    || bm_cust.shutdown_bat > battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE))) {
        LOG_MSGID_I(battery_management, "[BM_BASIC]Battery voltage low\r\n", 0);
        hal_rtc_enter_rtc_mode();
    }
#endif
    battery_management_interrupt_register();
    battery_init_check_charger_exist();
    LOG_MSGID_I(battery_management,"[BM_BASIC]Battery Management init done",0);
    return BATTERY_MANAGEMENT_STATUS_OK;
}

battery_management_status_t battery_management_register_callback_internal(battery_management_callback_t callback) {
    bmt_callback[battery_callback_index].func = callback;
    bmt_callback[battery_callback_index].callback_init = true;
    battery_callback_index++;
    return BATTERY_MANAGEMENT_STATUS_OK;
}

void battery_init_check_charger_exist(void) {
    bm_ctrl_info.isChargerExist = pmu_get_chr_detect_value();
    bm_ctrl_info.chargerState = pmu_get_charger_state();
    if (bm_ctrl_info.isChargerExist) {
        battery_charger_in_callback();
        battery_charger_state_change_callback();
    } else {
        battery_notification(BATTERY_MANAGEMENT_EVENT_CHARGER_EXIST_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
        battery_notification(BATTERY_MANAGEMENT_EVENT_BATTERY_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
    }
}

/*==========[Other : Basic internal function]==========*/
battery_management_status_t battery_init_basic_data() {
    bm_cust.s1_voltage = pmu_chg_info.cc1_thrd_volt;
    bm_cust.s1_chr_cur = pmu_chg_info.cc1_curr;
    bm_cust.s2_voltage = pmu_chg_info.cc2_thrd_volt;
    bm_cust.s2_chr_cur = pmu_chg_info.cc2_curr;
    bm_cust.full_bat = pmu_chg_info.full_bat_volt;
    bm_cust.recharger_voltage = pmu_chg_info.rechg_volt;
    bm_cust.initial_bat = pmu_vbat_volt.init_bat.volt;
    bm_cust.shutdown_bat = pmu_vbat_volt.sd_bat.volt;
    LOG_MSGID_I(battery_management, "[BM_BASIC]full_bat[%d]recharger_voltage[%d]init_bat[%d]shutdown_bat[%d]s1_voltage[%d]s1_chr_cur[%d]s2_voltage[%d]s2_chr_cur[%d]", 8,
           bm_cust.full_bat,bm_cust.recharger_voltage,bm_cust.initial_bat,bm_cust.shutdown_bat,bm_cust.s1_voltage,bm_cust.s1_chr_cur,bm_cust.s2_voltage,bm_cust.s2_chr_cur);
    return BATTERY_MANAGEMENT_STATUS_OK;
}

void battery_notification(battery_management_event_t event, uint32_t chr_exist, uint32_t state) {
    int i = 0;
    LOG_MSGID_I(battery_management, "[BM_BASIC][BM Notification : %d][Chr_exist:%d][Chr_state:%d]\r\n",3,event,(int)chr_exist,(int)state);/*Log output by BT*/
    for (i = 0; i < battery_callback_index; i++) {
        if ((bmt_callback[i].callback_init == true) && (bmt_callback[i].func != NULL)) {
            bmt_callback[i].func(event, &bm_ctrl_info);
        }
    }
}
void battery_unlock_sleep(void) {
    uint8_t bm_lock = 0;
    do {
        bm_lock = sleep_management_check_handle_status(battery_sleep_handle);
        if (sleep_management_check_handle_status(battery_sleep_handle) >= 1) {
            hal_sleep_manager_unlock_sleep(battery_sleep_handle);
        }
    } while (bm_lock >= 1);
    LOG_MSGID_I(battery_management, "[BM_BASIC]battery unlock sleep: %d\n", 1,sleep_management_check_handle_status(battery_sleep_handle));
}

/*==========[Other : Additional features]==========*/
#ifdef CHARGER_CALIBRATION
/*This API is for fixing human actions or environment exceptions in the charger plug-in/plug-out*/
void battery_detect_calibration_timer_callback(TimerHandle_t pxTimer) {
    uint32_t mask_pri;
    uint8_t temp_state = 0;
    hal_nvic_save_and_set_interrupt_mask_special(&mask_pri);
    temp_state = pmu_get_chr_detect_value();
    if ((bm_ctrl_info.isChargerExist != temp_state)) {
        hal_nvic_restore_interrupt_mask_special(mask_pri);
        LOG_MSGID_I(battery_management, "[BM_BASIC]ERROR!!!!, Plug-in/out is not staggered\r\n", 0);
        if (temp_state) {
            battery_charger_in_callback();
        } else {
            battery_charger_out_callback();
        }
    } else {
        hal_nvic_restore_interrupt_mask_special(mask_pri);
    }

}
#endif
