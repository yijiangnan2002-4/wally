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
#include "timers.h"
#include "hal_sleep_manager_platform.h"
int *voltage_table = NULL;
uint8_t gauge_timer_count = 0;                              /*Data : For gauge calculation*/
uint8_t gauge_regular_second_time;                          /*Data : For gauge calculation*/
extern battery_basic_data bm_cust;                          /*Data : restore battery basic data*/
extern battery_managerment_control_info_t bm_ctrl_info;     /*Data : restore battery basic data*/
int battery_linear_soc[LINEAR_GAUGE_SIZE];                  /*Data : For linear gauge soc smooth*/
int battery_linear_bat[LINEAR_GAUGE_SIZE];                  /*Data : For linear gauge soc smooth*/
uint8_t linear_gauge_array_flag = 0 ;                       /*flag : For linear gauge soc array init*/
extern uint8_t battery_VA18_flag ;

uint32_t smooth_soc = 0;
uint32_t smooth_vbat_voltage = 0;
int linear_array_index = 0;
uint8_t linear_dischargr_step_flag = BM_DISCHARGER_NOT_INIT ;                       /*flag : For linear gauge soc array init*/
uint8_t linear_charging_step_flag = BM_CHARGING_NOT_INIT ;                       /*flag : For linear gauge soc array init*/
uint32_t older_value = 0;
/*==========[Battery Management:linear gauge]==========*/
void battery_linear_gauge_task(void *pvParameters)
{
    TickType_t xLastWakeTime;
    uint32_t temp_batsns = 0, temp_soc = 0;
    uint32_t cc_index = 0;
    while (1) {
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, LINEAR_REGULAR_GAUGE_TIME / portTICK_RATE_MS);
        if (battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST)) {
#ifdef BATTERY_AVOID_EVBUS_RAISE_VBAT_VOL
            cc_index = pmu_get_charger_current_index();
            if (battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY) < BATTERY_STABLE_CHARGING_VOLTAGE) {
                pmu_set_charger_current(pmu_fastcc_chrcur_2mA);
                temp_batsns = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
                pmu_set_charger_current(cc_index);
                LOG_MSGID_I(battery_management, "[BM_GAUGE]BATTERY_DECREASE_RESISTANCE disable", 0);
                temp_soc = battery_core_gauge_function(get_smooth_vbat());
            } else {
                temp_batsns = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
                temp_soc = battery_core_gauge_function(get_smooth_vbat());
            }
#else
            temp_batsns = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
            temp_soc = battery_core_gauge_function(temp_batsns);
#endif
            battery_smooth_charging_soc(temp_soc, temp_batsns);
            LOG_MSGID_I(battery_management, "[BM_GAUGE]battery_linear_gauge_task  VBUS on smooth_SOC:%d SOC%d", 2, get_smooth_soc(), temp_soc);
        } else {
            temp_batsns = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
            temp_soc = battery_core_gauge_function(temp_batsns);
            battery_smooth_discharger_soc(temp_soc, temp_batsns);
            LOG_MSGID_I(battery_management, "[BM_GAUGE]battery_linear_gauge_task  VBUS off smooth_SOC:%d SOC%d", 2, get_smooth_soc(), temp_soc);
        }
    }
}
/*Linear gauge use smooth SOC and VBAT*/
void battery_smooth_charging_soc(uint32_t soc_value, uint32_t batsns)
{
    LOG_MSGID_I(battery_management, "[BM_GAUGE]charging [soc_value: %d][batsns :%d][step:%d]", 3, soc_value, batsns, linear_charging_step_flag);
    int i = 0;
    uint32_t temp_soc = 0;
    uint32_t temp_bat = 0;

    if (linear_charging_step_flag == BM_CHARGING_NOT_INIT) {
        linear_array_index = 0;
        for (i = 0; i < LINEAR_GAUGE_SIZE; i++) {
            battery_linear_soc[i] = soc_value;
            battery_linear_bat[i] = batsns;
        }
        linear_charging_step_flag = BM_CHARGING_START;
        linear_array_index++;
    } else if (linear_charging_step_flag == BM_CHARGING_START) {
        linear_array_index = 0;
        for (i = 0; i < LINEAR_GAUGE_SIZE; i++) {
            battery_linear_soc[i] = smooth_soc;
            battery_linear_bat[i] = smooth_vbat_voltage;
        }
        linear_charging_step_flag = BM_CHARGING_STAGE;
        linear_array_index++;
    } else if (linear_charging_step_flag == BM_CHARGING_STAGE) {
        battery_linear_soc[linear_array_index] = soc_value;
        battery_linear_bat[linear_array_index] = batsns;
        linear_array_index++;
        if (linear_array_index == LINEAR_GAUGE_SIZE) {
            linear_array_index = 0;
        }
        linear_charging_step_flag = BM_CHARGING_STAGE;
    } else {
        LOG_MSGID_I(battery_management, "[BM_GAUGE]Charging step error", 0);
    }

    for (i = 0; i < LINEAR_GAUGE_SIZE; i++) {
        temp_soc += battery_linear_soc[i];
        temp_bat += battery_linear_bat[i];
    }
    smooth_soc = (temp_soc / LINEAR_GAUGE_SIZE);
    smooth_vbat_voltage = (temp_bat / LINEAR_GAUGE_SIZE);
    linear_dischargr_step_flag = BM_DISCHARGER_START;
    LOG_MSGID_I(battery_management, "[BM_GAUGE]Charging smooth [soc: %d Vbat :%d]", 2, smooth_soc, smooth_vbat_voltage);
}
void battery_smooth_discharger_soc(uint32_t soc_value, uint32_t batsns)
{
    LOG_MSGID_I(battery_management, "[BM_GAUGE]Discharger [soc_value: %d][batsns :%d][step:%d]", 3, soc_value, batsns, linear_dischargr_step_flag);
    int i = 0;
    uint32_t temp_soc = 0;
    uint32_t temp_bat = 0;

    if (linear_dischargr_step_flag == BM_DISCHARGER_NOT_INIT) {
        linear_array_index = 0;
        for (i = 0; i < LINEAR_GAUGE_SIZE; i++) {
            battery_linear_soc[i] = soc_value;
            battery_linear_bat[i] = batsns;
        }
        linear_dischargr_step_flag = BM_DISCHARGER_START;
        linear_array_index++;
    } else if (linear_dischargr_step_flag == BM_DISCHARGER_START) {
        linear_array_index = 0;
        for (i = 0; i < LINEAR_GAUGE_SIZE; i++) {
            battery_linear_soc[i] = smooth_soc;
            battery_linear_bat[i] = smooth_vbat_voltage;
        }
        linear_dischargr_step_flag = BM_DISCHARGER_STAGE;
        linear_array_index++;
    } else if (linear_dischargr_step_flag == BM_DISCHARGER_STAGE) {
        battery_linear_soc[linear_array_index] = soc_value;
        battery_linear_bat[linear_array_index] = batsns;
        linear_array_index++;
        if (linear_array_index == LINEAR_GAUGE_SIZE) {
            linear_array_index = 0;
        }
        linear_dischargr_step_flag = BM_DISCHARGER_STAGE;
    } else {
        LOG_MSGID_I(battery_management, "[BM_GAUGE]Discharger step error", 0);
    }
    for (i = 0; i < LINEAR_GAUGE_SIZE; i++) {
        temp_soc += battery_linear_soc[i];
        temp_bat += battery_linear_bat[i];
    }
    smooth_soc = (temp_soc / LINEAR_GAUGE_SIZE);
    smooth_vbat_voltage = (temp_bat / LINEAR_GAUGE_SIZE);
    linear_charging_step_flag = BM_CHARGING_START;
    LOG_MSGID_I(battery_management, "[BM_GAUGE]Discharger smooth [soc: %d Vbat :%d]", 2, smooth_soc, smooth_vbat_voltage);
}

uint32_t get_smooth_soc(void)
{
    return smooth_soc;
}
uint32_t get_smooth_vbat(void)
{
    return smooth_vbat_voltage;
}
void battery_gauge_init(void)
{
    uint32_t temp_batsns = 0, temp_soc = 0;
    if (battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST)) {
#ifdef BATTERY_DECREASE_CC_OBTAIN_VABT
        uint32_t cc_index = 0;
        cc_index = pmu_get_charger_current_index();
        if (battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY) < BATTERY_STABLE_CHARGING_VOLTAGE) {
            pmu_set_charger_current(pmu_fastcc_chrcur_2mA);
            temp_batsns = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
            pmu_set_charger_current(cc_index);
            temp_soc = battery_core_gauge_function(temp_batsns);
        } else {
            temp_batsns = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
            temp_soc = battery_core_gauge_function(temp_batsns);
        }
#else
        temp_batsns = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
        temp_soc = battery_core_gauge_function(temp_batsns);
#endif
        battery_smooth_charging_soc(temp_soc, temp_batsns);
        LOG_MSGID_I(battery_management, "[BM_GAUGE]battery_linear_gauge_task  VBUS on smooth_SOC:%d SOC%d", 2, get_smooth_soc(), temp_soc);
    } else {
        temp_batsns = pmu_auxadc_get_channel_value(PMU_AUX_BATSNS);
        temp_soc = battery_core_gauge_function(temp_batsns);
        battery_smooth_discharger_soc(temp_soc, temp_batsns);
        LOG_MSGID_I(battery_management, "[BM_GAUGE]battery_linear_gauge_task  VBUS off smooth_SOC:%d SOC%d", 2, get_smooth_soc(), temp_soc);
    }
}
/*==========[Battery Management:Gague]==========*/
void battery_calibration_gauge_tolerance(void)
{
    if (pmu_get_charger_state() == 0x2) {
        gauge_timer_count++;
        if (gauge_timer_count == gauge_regular_second_time) {
            bm_ctrl_info.gauge_calibration += 1;
            gauge_timer_count = 0;
        }
        if (bm_ctrl_info.gauge_calibration >= 5) {
            bm_ctrl_info.gauge_calibration = 5;
        }
    } else if (pmu_get_charger_state() != 0x2 && bm_ctrl_info.gauge_calibration > 0) {
        gauge_timer_count++;
        if ((bm_ctrl_info.feature.charger_option == 1) && (bm_ctrl_info.isChargerExist == 1)) {
            bm_ctrl_info.gauge_calibration = 5;
        } else if (gauge_timer_count == gauge_regular_second_time) {
            bm_ctrl_info.gauge_calibration -= 1;
            gauge_timer_count = 0;
        }
        if (bm_ctrl_info.gauge_calibration <= 0) {
            bm_ctrl_info.gauge_calibration = 0;
        }
    }
}
/* ste 0:Vbus plug-out
 * ste 1:Vbus plug-in */
void battery_switch_calibration_state(uint8_t ste)
{
#ifndef BATTERY_CHARGER_CV_2STEP
    if (ste) {
        if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > (bm_cust.full_bat - bm_cust.full_bat_offset)) {
            bm_ctrl_info.gauge_calibration = ((pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) - (bm_cust.full_bat - bm_cust.full_bat_offset)) / 10) + 1;
        } else {
            bm_ctrl_info.gauge_calibration = 0;
        }
    } else {
        if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > (bm_cust.full_bat - bm_cust.full_bat_offset) && bm_ctrl_info.gauge_calibration == 0) {
            bm_ctrl_info.gauge_calibration = ((pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) - (bm_cust.full_bat - bm_cust.full_bat_offset)) / 10);
        }
        gauge_timer_count = 0;
    }
#else
    if (ste) {
        if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > (bm_cust.full_bat - bm_cust.full_bat_offset)) {
            bm_ctrl_info.gauge_calibration = ((pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) - (bm_cust.full_bat - bm_cust.full_bat_offset)) / 10) + 1;
        } else {
            bm_ctrl_info.gauge_calibration = 0;
        }
    } else {
        if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > (bm_cust.full_bat - bm_cust.full_bat_offset) && bm_ctrl_info.gauge_calibration == 0) {
            bm_ctrl_info.gauge_calibration = ((pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) - (bm_cust.full_bat - bm_cust.full_bat_offset)) / 10);
        }
        gauge_timer_count = 0;
    }
#endif

}
/*How many seconds will be divided into six times to reaching 100% */
void battery_set_calibration_time(void)
{
    if ((CHARGER_TOLERANCE_TIME * 60) <= CHARGER_REGULAR_TIME) {
        gauge_regular_second_time = 1;
    } else {
        gauge_regular_second_time = ((CHARGER_TOLERANCE_TIME * 60) / CHARGER_REGULAR_TIME) / 6;
    }
}

uint32_t battery_smooth_soc(uint32_t soc_value)
{
    int i = 0;
    uint32_t temp_soc = 0;
    if (linear_gauge_array_flag == 0) {
        for (i = 0; i < LINEAR_GAUGE_SIZE; i++) {
            battery_linear_soc[i] = soc_value;
        }
        linear_gauge_array_flag = 1;
        linear_array_index++;
    } else {
        battery_linear_soc[linear_array_index] = soc_value;
        linear_array_index++;
        if (linear_array_index == LINEAR_GAUGE_SIZE) {
            linear_array_index = 0;
        }
    }

    for (i = 0; i < LINEAR_GAUGE_SIZE; i++) {
        temp_soc += battery_linear_soc[i];
    }
    return (temp_soc / LINEAR_GAUGE_SIZE);
}
uint32_t battery_get_gauge_percent(void)
{
    uint32_t interpolationVoltage = 0;
#ifdef MTK_FUEL_GAUGE
    interpolationVoltage = get_fg_soc();
#else
#ifdef BATTERY_LINEAR_SMOOTH_SOC
    interpolationVoltage = get_smooth_soc();
#else
    interpolationVoltage = battery_core_gauge_function(pmu_auxadc_get_channel_value(PMU_AUX_VBAT));
#endif
#endif
    if (MTK_BATTERY_ULTRA_LOW_BAT >= interpolationVoltage) {
        interpolationVoltage = MTK_BATTERY_ULTRA_LOW_BAT;
    }
    return interpolationVoltage;
}
uint32_t battery_get_gauge_percent_level(void)
{
    return (int)(battery_get_gauge_percent() / 10);
}
unsigned char battery_gauge_get_refernece_index(signed short vBatSnsValue)
{
    unsigned char index;
    for (index = 0; index < BATTERY_VOLTAGE_REFERENCE_POINTS; index++) {
        if (vBatSnsValue < voltage_table[index]) {
            break;
        }
    }
    return index;
}

uint32_t battery_core_gauge_function(signed short vBatSnsValue)
{
    unsigned char index = 0;
    float slope;
    uint32_t interPolationValue = 0;
    short xAxisDiff = 0;
    voltage_table = bm_cust.check_point;
    index = battery_gauge_get_refernece_index(vBatSnsValue);
    slope = battery_core_gauge_indexCalc(index);
    if (index == 0) {
        xAxisDiff = (float)(vBatSnsValue - bm_cust.shutdown_bat);
        interPolationValue = 0 + slope * xAxisDiff;
    } else if (index == BATTERY_VOLTAGE_REFERENCE_POINTS) {
        xAxisDiff = (float)(vBatSnsValue - voltage_table[BATTERY_VOLTAGE_REFERENCE_POINTS - 1]);
        interPolationValue = 90 + slope * xAxisDiff;
        if (bm_cust.feature_2cv == BATTERY_OPERATE_OFF) {
            if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > (bm_cust.full_bat - bm_cust.full_bat_offset)) {
                interPolationValue = GAUGE_TOLERANCE_PERCENT + bm_ctrl_info.gauge_calibration;
            }
        } else {
            if (pmu_auxadc_get_channel_value(PMU_AUX_BATSNS) > (bm_cust.cv2 - bm_cust.full_bat_offset)) {
                interPolationValue = GAUGE_TOLERANCE_PERCENT + bm_ctrl_info.gauge_calibration;
            }
        }

        if (interPolationValue > 100) {
            interPolationValue = 100;
        }
    } else {
        xAxisDiff = (float)(vBatSnsValue - voltage_table[index - 1]);
        interPolationValue = (index * 10) + slope * xAxisDiff;
    }
    return interPolationValue;
}

float battery_core_gauge_indexCalc(unsigned char index)
{
    short dataX0, dataX1 = 0;
    short dataY0, dataY1 = 0;
    float slope;

    if (index == 0) {
        dataX0 = bm_cust.shutdown_bat;
        dataX1 = voltage_table[0];

        dataY0 = 0;
        dataY1 = 10;
    } else if (index == BATTERY_VOLTAGE_REFERENCE_POINTS) {
        dataX0 = voltage_table[BATTERY_VOLTAGE_REFERENCE_POINTS - 1];
        if (bm_cust.feature_2cv == BATTERY_OPERATE_OFF) {
            dataX1 = bm_cust.full_bat;
        } else {
            dataX1 = bm_cust.cv2;
        }
        dataY0 = BATTERY_VOLTAGE_REFERENCE_POINTS * 10;
        dataY1 = 100;
    } else {
        dataX0 = voltage_table[index - 1];
        dataX1 = voltage_table[index];
        dataY0 = index * 10;
        dataY1 = (index + 1) * 10;
    }

    if (dataX1 == dataX0) {
        return 0.0;
    }

    slope = ((float)(dataY1 - dataY0)) / ((float)(dataX1 - dataX0));
    return slope;
}

#ifdef MTK_FUEL_GAUGE
#include "battery_common.h"
#include "memory_attribute.h"

extern bool g_suspend_timeout;
PMU_ChargerStruct BMT_status;
extern uint32_t sleep_total_time;
extern uint32_t wake_up_smooth_time;
static bool battery_meter_initilized = false;
bool g_battery_soc_ready = false;
static bool g_battery_management_init_flag = false;
static TaskHandle_t battery_management_task_handle = NULL;
extern uint32_t battery_thread_time;
static bool g_bat_init_flag = false;
static QueueHandle_t bmt_queue_handle;
static TimerHandle_t xTimerofFuelGauge = NULL;
static bmt_cmd_event_t bmt_timer_event;
uint32_t battery_duration_time[DURATION_NUM] = {0}; /* sec */
static int32_t bmt_timer_id;
static int32_t bmt_timer_fg_id;


#define BMT_TIMER_ID0 0
#define BMT_TIMER_ID1 1
#define CONFIG_DIS_CHECK_BATTERY
#define BATTERY_AVERAGE_SIZE            20
#define BATTERY_AVERAGE_DATA_NUMBER     3
#define V_0PERCENT_TRACKING             3450  /* 3450mV */

//TODO move these defines to task_def.h
#define BMT_TASK_NAME "BMT"
#define BMT_TASK_STACKSIZE 2048
//#define BMT_TASK_PRIO TASK_PRIORITY_ABOVE_NORMAL
#define BMT_TASK_PRIO 3
#define BMT_QUEUE_LENGTH 1
extern void battery_management_fuel_gauge_deputy_reinit(void);
#ifdef FUEL_GAUGE_DEPUTY_TABLE_ENABLE
int fuel_gauge_deputy_temp_l = FUEL_GAUGE_DEPUTY_TEMP_L;
int fuel_gauge_deputy_temp_h = FUEL_GAUGE_DEPUTY_TEMP_H;
#endif
void battery_management_gpt_callback(TimerHandle_t xTimer)
{
    LOG_MSGID_I(battery_management, "[BM_FG]Regular FG callback", 0);
    BaseType_t xHigherPriorityTaskWoken;
    bmt_timer_event.event = BMT_EVENT_TIMEOUT;
    xQueueSendFromISR(bmt_queue_handle, &bmt_timer_event, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
#ifdef FUEL_GAUGE_DEPUTY_TABLE_ENABLE
    if ((battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE) <= fuel_gauge_deputy_temp_l) && (BMT_status.meter_table == 1)) {
        LOG_MSGID_I(battery_management, "[BM_FG]Tempature is lower specified temperature :%d", 1,fuel_gauge_deputy_temp_l);
        battery_management_fuel_gauge_deputy_reinit();
    }
    if ((battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE) >= fuel_gauge_deputy_temp_h) && (BMT_status.meter_table == 2)) {
        LOG_MSGID_I(battery_management, "[BM_FG]Tempature is higher specified temperature :%d", 1,fuel_gauge_deputy_temp_h);
        battery_management_fuel_gauge_init();
    }
#endif
}

uint32_t battery_core_get_duration_time(BATTERY_TIME_ENUM duration_type)
{
    return battery_duration_time[duration_type];
}

uint32_t battery_core_get_current_time_in_ms(void)
{
    uint32_t count = 0;
    uint64_t count64 = 0;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    count64 = ((uint64_t)count) * 1000 / 32768;

    return (uint32_t)(count64);
}

static TickType_t battery_core_get_wakeup_ticks(void)
{
    TickType_t xQueueTicksToWait;
    xQueueTicksToWait = portMAX_DELAY;
    return xQueueTicksToWait;
}

bool battery_core_get_charger_status(void)
{
    return bm_ctrl_info.isChargerExist;
}

static bool battery_core_is_charger_detect(void)
{
    return bm_ctrl_info.isChargerExist;
}

static void battery_core_check_charger_detect(void)
{
    if (battery_core_is_charger_detect() == true) {
        BMT_status.charger_exist = true;
        LOG_MSGID_I(battery_management, "[BM_FG][battery_core_thread]Cable in, CHR_Type_num=%d\r\n", 1, (int)BMT_status.charger_type);
    } else {
        BMT_status.charger_exist = false;;
        BMT_status.bat_full = false;
        LOG_MSGID_I(battery_management, "[BM_FG][battery_core_thread]Cable out\r\n", 0);
    }
}

static uint32_t battery_core_convert_duration_time_ms(uint32_t pre_time_ms, uint32_t this_time_ms)
{
    if (this_time_ms >= pre_time_ms) {
        return (this_time_ms - pre_time_ms);
    } else {
        /* Avoid overflow in ms 0xffffffff*1000/32768= 0x07cfffff */
        return (0x07cfffff - (pre_time_ms - this_time_ms) + 1);
    }
}

void battery_core_update_time(uint32_t *pre_time, BATTERY_TIME_ENUM duration_type)
{
    uint32_t time;
    uint32_t duration_time;
    static uint32_t pre_saved_time[DURATION_NUM] = {0};

    time = battery_core_get_current_time_in_ms();

    duration_time = battery_core_convert_duration_time_ms(*pre_time, time);

    battery_duration_time[duration_type] = (duration_time + pre_saved_time[duration_type]) / 1000;
    pre_saved_time[duration_type] = (duration_time + pre_saved_time[duration_type]) % 1000;

    *pre_time = time;
}

static void battery_core_average_method_init(BATTERY_AVG_ENUM type, int32_t *bufferdata, uint32_t data,
                                             int32_t *sum)
{
    uint32_t i;
    static uint8_t index;
    static bool batteryBufferFirst = true;
    /* reset charging current window while plug in/out { */
    if (type == BATTERY_AVG_CURRENT) {
#if 0

        static bool previous_charger_exist = false;
        static bool previous_in_recharge_state = false;
        if (BMT_status.charger_exist == true) {
            if (previous_charger_exist == false) {
                batteryBufferFirst = true;
                previous_charger_exist = true;
                if (BMT_status.charger_type == HAL_CHARGER_TYPE_STANDARD_CHARGER) {
                    data = AC_CHARGER_CURRENT;
                } else if (BMT_status.charger_type == HAL_CHARGER_TYPE_CHARGING_HOST) {
                    data = CHARGING_HOST_CHARGER_CURRENT;
                } else if (BMT_status.charger_type == HAL_CHARGER_TYPE_NONSTANDARD_CHARGER) {
                    data = NON_STD_AC_CHARGER_CURRENT;    /* mA */
                } else {    /* USB */
                    data = USB_CHARGER_CURRENT;    /* mA */
                }
            } else if ((previous_in_recharge_state == false)
                       && (BMT_status.bat_in_recharging_state == true)) {
                batteryBufferFirst = true;

                if (BMT_status.charger_type == HAL_CHARGER_TYPE_STANDARD_CHARGER) {
                    data = AC_CHARGER_CURRENT;
                } else if (BMT_status.charger_type == HAL_CHARGER_TYPE_CHARGING_HOST) {
                    data = CHARGING_HOST_CHARGER_CURRENT;
                } else if (BMT_status.charger_type == HAL_CHARGER_TYPE_NONSTANDARD_CHARGER) {
                    data = NON_STD_AC_CHARGER_CURRENT;    /* mA */
                } else {    /* USB */
                    data = USB_CHARGER_CURRENT;    /* mA */
                }
            }

            previous_in_recharge_state = BMT_status.bat_in_recharging_state;
        } else {
            if (previous_charger_exist == true) {
                batteryBufferFirst = true;
                previous_charger_exist = false;
                data = 0;
            }
        }
#endif
    }
    /* reset charging current window while plug in/out } */

    BMT_DBG("[BM_FG]batteryBufferFirst =%d, data= (%d)",
            (int) batteryBufferFirst, (int) data);

    if (batteryBufferFirst == true) {
        for (i = 0; i < BATTERY_AVERAGE_SIZE; i++) {
            bufferdata[i] = data;
        }

        *sum = data * BATTERY_AVERAGE_SIZE;
    }

    index++;
    if (index >= BATTERY_AVERAGE_DATA_NUMBER) {
        index = BATTERY_AVERAGE_DATA_NUMBER;
        batteryBufferFirst = false;
    }
}

static uint32_t battery_core_average_method(BATTERY_AVG_ENUM type, int32_t *bufferdata, uint32_t data,
                                            int32_t *sum, uint8_t batteryIndex)
{
    uint32_t avgdata;

    battery_core_average_method_init(type, bufferdata, data, sum);

    *sum -= bufferdata[batteryIndex];
    *sum += data;
    bufferdata[batteryIndex] = data;
    avgdata = (*sum) / BATTERY_AVERAGE_SIZE;

    BMT_DBG("[BM_FG]bufferdata[%d]= (%d)", (int)batteryIndex, (int)bufferdata[batteryIndex]);

    return avgdata;
}

static void battery_core_get_battery_data(void)
{
    uint32_t bat_vol = 0, ZCV = 0;
    int32_t temperature = 0;
    static int32_t bat_sum, temperature_sum;
    static int32_t batteryVoltageBuffer[BATTERY_AVERAGE_SIZE];
    static int32_t batteryTempBuffer[BATTERY_AVERAGE_SIZE];
    static uint8_t batteryIndex = 0;
    static int32_t previous_SOC = -1;

    bat_vol = battery_meter_get_battery_voltage(true);
    temperature = battery_meter_get_battery_temperature();
    ZCV = battery_meter_get_battery_zcv();

    if (previous_SOC == -1 && bat_vol <= V_0PERCENT_TRACKING) {
        previous_SOC = 0;
        if (ZCV != 0) {
            LOG_MSGID_I(battery_management, "[BM_FG]battery voltage too low, use ZCV to init average data.\r\n", 0);
            BMT_status.bat_vol =
                battery_core_average_method(BATTERY_AVG_VOLT, &batteryVoltageBuffer[0], ZCV, &bat_sum, batteryIndex);
        } else {
            LOG_MSGID_I(battery_management, "[BM_FG]battery voltage too low, use V_0PERCENT_TRACKING + 100 to init average data.\r\n", 0);
            BMT_status.bat_vol =
                battery_core_average_method(BATTERY_AVG_VOLT, &batteryVoltageBuffer[0], V_0PERCENT_TRACKING + 100, &bat_sum, batteryIndex);
        }
    } else {
        BMT_status.bat_vol =
            battery_core_average_method(BATTERY_AVG_VOLT, &batteryVoltageBuffer[0], bat_vol, &bat_sum, batteryIndex);
    }
    BMT_status.temperature =
        battery_core_average_method(BATTERY_AVG_TEMP, &batteryTempBuffer[0], temperature, &temperature_sum, batteryIndex);
    BMT_status.ZCV = ZCV;

    batteryIndex++;
    if (batteryIndex >= BATTERY_AVERAGE_SIZE) {
        batteryIndex = 0;
    }

    //BMT_INFO("[kernel]AvgVbat %d,bat_vol %d, AvgI %d, I %d, VChr %d, AvgT %d, T %d, ZCV %d, UI_SOC2 %d, LEVEL %d",
    //         (int)BMT_status.bat_vol, (int)bat_vol, (int)BMT_status.ICharging, (int)ICharging,
    //         (int)BMT_status.charger_vol, (int)BMT_status.temperature, (int)temperature,
    //         (int)BMT_status.ZCV, (int)BMT_status.UI_SOC2, (int)BMT_status.UI_SOC2_LEVEL);
}

void battery_core_check_battery_exist(void)
{
#if defined(CONFIG_DIS_CHECK_BATTERY)
    LOG_MSGID_I(battery_management, "[BM_FG]Disable check battery exist.\r\n", 0);
#else
    uint32_t baton_count = 0;
    bool charging_enable = false;
    uint32_t battery_status = 0;
    uint32_t i;

    for (i = 0; i < 3; i++) {
        //TODO
        //hal_charger_get_battery_status((bool *)&battery_status);
        if (battery_status) {
            baton_count += 0;
        } else {
            baton_count += 1;
        }
    }

    if (baton_count >= 3) {
        //hal_charger_enable(charging_enable);
        battery_set_enable_charger(charging_enable);
    }
#endif
}

void battery_core_thread(void)
{
    pmu_enable_power(PMU_LDO_VA18, PMU_ON);
    battery_reguest_va18(BATTERY_OPERATE_ON);
    if (battery_meter_initilized == false) {
        battery_core_get_battery_data();
        battery_meter_initial();
        BMT_status.nPercent_ZCV = battery_meter_get_battery_nPercent_zcv();
        battery_meter_initilized = true;
    }
    battery_core_update_time(&battery_thread_time, BATTERY_THREAD_TIME);
    sw_fg_main_flow();
    battery_core_check_charger_detect();
    battery_core_get_battery_data();
    battery_notification(BATTERY_MANAGEMENT_EVENT_BATTERY_UPDATE, pmu_get_chr_detect_value(), pmu_get_charger_state());
    battery_reguest_va18(BATTERY_OPERATE_OFF);
    pmu_enable_power(PMU_LDO_VA18, PMU_OFF);
}

void battery_management_task(void *pvParameters)
{
    TickType_t xQueueTicksToWait = portMAX_DELAY;

    bmt_cmd_event_t event_item;
    uint32_t xLastExecutionTime;

    LOG_MSGID_I(battery_management, "[BM_FG]batery_manager_task_init\r\n", 0);
    battery_thread_time = battery_core_get_current_time_in_ms();

    g_bat_init_flag = true;

    while (1) {

        xQueueTicksToWait = battery_core_get_wakeup_ticks();
        if (xQueueReceive(bmt_queue_handle, &event_item, xQueueTicksToWait)) {
            hal_sleep_manager_lock_sleep(SLEEP_LOCK_BATTERY_MANAGEMENT);
            xLastExecutionTime = xTaskGetTickCount();
            LOG_MSGID_I(battery_management, "[BM_FG]Queue Receive\r\n", 0);
            battery_core_thread();
            xLastExecutionTime = (xTaskGetTickCount() - xLastExecutionTime);
            hal_sleep_manager_unlock_sleep(SLEEP_LOCK_BATTERY_MANAGEMENT);
        } else {
            LOG_MSGID_I(battery_management, "[BM_FG]xQueueTicksToWait time out\r\n", 0);
            sleep_total_time = 0;
            wake_up_smooth_time = 0;
            bmt_timer_event.event = BMT_EVENT_TIMEOUT;
            xQueueSend(bmt_queue_handle, &bmt_timer_event, 0 / portTICK_PERIOD_MS);
        }
    }
}

void battery_management_fuel_gauge_init(void)
{
    LOG_MSGID_I(battery_management, "[BM_FG]battery_management_fuel_gauge_init", 0);
    LOG_MSGID_I(battery_management, "[BM_FG]BATTERY_CATEGORY %d\r\n", 1, bm_cust.battery_category);
    //Init custom data
    switch (bm_cust.battery_category) {
        case 1:
            battery_meter_init();
            break;
        case 2:
            battery_meter_init_custom_meter_data_bat2();
            break;
    }
    battery_meter_init();

    /* Initialization BMT Struct */
    BMT_status.bat_exist = true;    /* device must have battery */
    BMT_status.charger_exist = false;   /* for default, no charger */
    BMT_status.bat_vol = 0;
    BMT_status.temperature = 0;

    BMT_status.SOC = 0;
    BMT_status.UI_SOC = -100;
    BMT_status.UI_SOC2 = -100;
    BMT_status.UI_SOC2_LEVEL = -1;

    BMT_status.bat_full = false;
    BMT_status.nPercent_ZCV = 0;
    BMT_status.meter_table = 1;
    bmt_timer_id = BMT_TIMER_ID0;
    /* Queue creation */
    bmt_queue_handle = xQueueCreate(BMT_QUEUE_LENGTH, sizeof(bmt_cmd_event_t));

    if (battery_meter_initilized == false) {
        battery_core_get_battery_data();
        battery_meter_initial();
        BMT_status.nPercent_ZCV = battery_meter_get_battery_nPercent_zcv();
        battery_meter_initilized = true;
    }

    bmt_timer_fg_id = BMT_TIMER_ID1;
    xTimerofFuelGauge = xTimerCreate("TimerofFuelGauge",       /* Just a text name, not used by the kernel. */
                                     (BAT_SLEEP_WAKE_UP_PERIOD * 1000 / portTICK_PERIOD_MS),    /* The timer period in ticks. */
                                     pdTRUE,        /* The timers will auto-reload themselves when they expire. */
                                     (void *)bmt_timer_fg_id,   /* Assign each timer a unique id equal to its array index. */
                                     battery_management_gpt_callback /* Each timer calls the same callback when it expires. */
                                    );
    if (xTimerofFuelGauge == NULL) {
        BMT_ERR("[BM_FG]xTimerofFuelGauge create fail");
    }
    xTimerStart(xTimerofFuelGauge, 0);

    bmt_timer_event.event = BMT_EVENT_INIT;
    xQueueSend(bmt_queue_handle, &bmt_timer_event, 0 / portTICK_PERIOD_MS);

    xTaskCreate(battery_management_task, BMT_TASK_NAME, BMT_TASK_STACKSIZE / sizeof(StackType_t), NULL, BMT_TASK_PRIO, &battery_management_task_handle);
    g_battery_management_init_flag = true;
}

void battery_management_fuel_gauge_reinit(void)
{
    LOG_MSGID_I(battery_management, "[BM_FG]battery_management_fuel_gauge_reinit", 0);
    if (HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION) {
        xTimerStopFromISR(xTimerofFuelGauge, 0);
    } else {
        xTimerStop(xTimerofFuelGauge, 0);
    }
    /* Initialization BMT Struct */
    BMT_status.bat_exist = true;     /* device must have battery */
    BMT_status.charger_exist = false;    /* for default, no charger */
    BMT_status.bat_vol = 0;
    BMT_status.temperature = 0;

    BMT_status.SOC = 0;
    BMT_status.UI_SOC = -100;
    BMT_status.UI_SOC2 = -100;
    BMT_status.UI_SOC2_LEVEL = -1;

    BMT_status.bat_full = false;
    BMT_status.nPercent_ZCV = 0;
    BMT_status.meter_table = 1;
    battery_core_get_battery_data();
    //  battery_meter_initial();
    battery_meter_get_fuel_gauge_init_data();
    fgauge_reinit();
    BMT_status.nPercent_ZCV = battery_meter_get_battery_nPercent_zcv();

    xTimerofFuelGauge = xTimerCreate("TimerofFuelGauge", /* Just a text name, not used by the kernel. */
                                     (BAT_SLEEP_WAKE_UP_PERIOD * 1000 / portTICK_PERIOD_MS), /* The timer period in ticks. */
                                     pdTRUE, /* The timers will auto-reload themselves when they expire. */
                                     (void *)bmt_timer_fg_id, /* Assign each timer a unique id equal to its array index. */
                                     battery_management_gpt_callback /* Each timer calls the same callback when it expires. */
                                    );
    sw_fg_main_flow();
    if (HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION) {
        xTimerStartFromISR(xTimerofFuelGauge, 0);
    } else {
        xTimerStart(xTimerofFuelGauge, 0);
    }

    g_battery_management_init_flag = true;
}

void battery_management_fuel_gauge_deputy_reinit(void)
{
    LOG_MSGID_I(battery_management, "[BM_FG]battery_management_fuel_gauge_deputy_reinit", 0);
    if (HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION) {
        xTimerStopFromISR(xTimerofFuelGauge, 0);
    } else {
        xTimerStop(xTimerofFuelGauge, 0);
    }

    battery_meter_init_deputy_meter_data();

    /* Initialization BMT Struct */
    BMT_status.bat_exist = true;     /* device must have battery */
    BMT_status.charger_exist = false;    /* for default, no charger */
    BMT_status.bat_vol = 0;
    BMT_status.temperature = 0;

    BMT_status.SOC = 0;
    BMT_status.UI_SOC = -100;
    BMT_status.UI_SOC2 = -100;
    BMT_status.UI_SOC2_LEVEL = -1;

    BMT_status.bat_full = false;
    BMT_status.nPercent_ZCV = 0;
    BMT_status.meter_table = 2;
    battery_core_get_battery_data();
    //  battery_meter_initial();
    battery_meter_get_fuel_gauge_init_data();
    fgauge_reinit();
    BMT_status.nPercent_ZCV = battery_meter_get_battery_nPercent_zcv();

    xTimerofFuelGauge = xTimerCreate("TimerofFuelGauge", /* Just a text name, not used by the kernel. */
                                     (BAT_SLEEP_WAKE_UP_PERIOD * 1000 / portTICK_PERIOD_MS), /* The timer period in ticks. */
                                     pdTRUE, /* The timers will auto-reload themselves when they expire. */
                                     (void *)bmt_timer_fg_id, /* Assign each timer a unique id equal to its array index. */
                                     battery_management_gpt_callback /* Each timer calls the same callback when it expires. */
                                    );
    sw_fg_main_flow();
    if (HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION) {
        xTimerStartFromISR(xTimerofFuelGauge, 0);
    } else {
        xTimerStart(xTimerofFuelGauge, 0);
    }

    g_battery_management_init_flag = true;
}
void battery_set_deputy_temp(int temp_l, int temp_h){
#ifdef FUEL_GAUGE_DEPUTY_TABLE_ENABLE
    LOG_MSGID_I(battery_management, "[BM_FG]Change Fuel gauge deputy temp value[L:H][%d:%d]", 2,temp_l,temp_h);
    fuel_gauge_deputy_temp_l = temp_l;
    fuel_gauge_deputy_temp_h = temp_h;
#endif
}
#endif

