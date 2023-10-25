/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#ifdef AIR_BLE_HRS_ENABLE

#include "FreeRTOS.h"
#include "hal_trng.h"
#include "bt_gap_le.h"

#include "bt_callback_manager.h"
#include "bt_callback_manager_config.h"

#include "bt_gatts.h"
#include "bt_timer_external.h"
#include "ble_hrs.h"
#include "ble_app_hrs.h"
#include "ble_app_hrs_data.h"

/* Create the log control block as user wishes. Here we use 'BLE_HRS' as module name.
 * User needs to define their own log control blocks as project needs.
 * Please refer to the log develop guide under /doc folder for more details.
 */
log_create_module(BLE_HRS_APP, PRINT_LEVEL_INFO);

#define BLE_HRS_APP_TIMER_HEART_RATE_MEASUREMENT_TIMEOUT_MS            (1000)  /**< 1s */

#define BLE_HRS_APP_NOTIFICATION_OPCODE_HANDLE_LENGTH            3 /**< Opcode lenth + Handle length. */

typedef struct {
    struct {
        uint8_t heart_rate_value_format: 1;          /**< Heart Rate Measurement Value field is in a format of UINT8 £¨0£©or UINT16£¨1£©. */
        uint8_t sensor_contact_detected: 1;          /**< If Sensor Contact is supported,indcates whether or not skin contact is detected . */
        uint8_t sensor_contact_support: 1;           /**< Sensor Contact Support bit. */
        uint8_t energy_expended_present: 1;          /**< whether or not, the Energy Expended field is present. */
        uint8_t rr_interval_present: 1;              /**< whether or not RR-Interval values are present. */
        uint8_t reserved: 3;                         /**< Reserved for Future Use. */
    } flags;
    uint16_t heart_rate_measurement_value;           /**< Heart Rate Measurement Value Field,Unit:org.bluetooth.unit.period.beats_per_minute. */
    uint16_t energy_expended;                        /**< Energy Expended Field,Unit: org.bluetooth.unit.energy.joule. */
    uint16_t rr_interval[BLE_HRS_APP_RR_INTERVAL_RECORDS_MAX];    /**< RR-Interval Field,Unit:1/1024 second. */
} ble_hrs_heart_rate_measurement_t;


typedef struct {
    bt_handle_t handle;
    uint16_t record_index;
    uint16_t records_num;
    uint16_t heart_rate_measurement_cccd;
    ble_hrs_heart_rate_measurement_t heart_rate_measurement;
    uint8_t body_sensor_location;
} ble_hrs_control_t;


static ble_hrs_control_t g_hrs_app_control = {
    .handle = BT_HANDLE_INVALID,
    .heart_rate_measurement.flags.heart_rate_value_format = 0,
    .heart_rate_measurement.flags.sensor_contact_support = BLE_HRS_FEATURE_SENSOR_CONTACT_ENABLE,
    .heart_rate_measurement.flags.sensor_contact_detected = 0,
    .heart_rate_measurement.flags.energy_expended_present = 0,
    .heart_rate_measurement.flags.rr_interval_present = 0,
    .body_sensor_location = BLE_HRS_APP_BODY_SENSOR_LOCATION,
};

static void ble_hrs_app_heart_rate_measurement_timeout_handler(uint32_t timer_id, uint32_t data);
static uint8_t ble_hrs_app_build_heart_rate_measurement_packet_available(void);
static uint16_t ble_hrs_app_get_oldest_rr_interval_record(void);
static uint8_t ble_hrs_app_build_heart_rate_measurement_packet(uint8_t *measurement, uint8_t len);
static bt_status_t ble_hrs_app_bt_msg_callback(bt_msg_type_t msg, bt_status_t status, void *buff);

static void ble_hrs_app_heart_rate_measurement_timeout_handler(uint32_t timer_id, uint32_t data)
{
    ble_hrs_control_t *hrs_app_control = (ble_hrs_control_t *)data;
    //LOG_MSGID_I(BLE_HRS_APP, "ble_hrs_app_heart_rate_measurement_timeout_handler,timer id = %d data = %d handle = %d,\r\n", 3, timer_id,data,hrs_app_control->handle);
    if (hrs_app_control->handle != BT_HANDLE_INVALID) {
        uint8_t notification_buf[BT_ATT_DEFAULT_MTU - BLE_HRS_APP_NOTIFICATION_OPCODE_HANDLE_LENGTH];//The actual MTU is notified by the BT_GATTS_EXCHANGE_MTU_INDICATION
        uint8_t len;
        if (1 == ble_hrs_app_build_heart_rate_measurement_packet_available()) {
            len = ble_hrs_app_build_heart_rate_measurement_packet(notification_buf, sizeof(notification_buf));
            ble_hrs_send_notification(hrs_app_control->handle, BT_SIG_UUID16_HEART_RATE_MEASUREMENT, notification_buf, len);
        } else {
            LOG_MSGID_I(BLE_HRS_APP, "ble_hrs_app_heart_rate_measurement_timeout_handler,Sensor does not contact skin.\r\n", 0);
        }
        bt_timer_ext_start(timer_id, data, BLE_HRS_APP_TIMER_HEART_RATE_MEASUREMENT_TIMEOUT_MS, ble_hrs_app_heart_rate_measurement_timeout_handler);
    }
}

static uint8_t ble_hrs_app_build_heart_rate_measurement_packet_available(void)
{
#if BLE_HRS_FEATURE_SENSOR_CONTACT_ENABLE
    if (g_hrs_app_control.heart_rate_measurement.flags.sensor_contact_support) {
        return g_hrs_app_control.heart_rate_measurement.flags.sensor_contact_detected;
    } else {
        return 1;
    }
#else
    return 1;
#endif
}

static uint16_t ble_hrs_app_get_oldest_rr_interval_record(void)
{
    if (g_hrs_app_control.records_num == 0) {
        return 0;
    } else {
        uint16_t index = (g_hrs_app_control.record_index + BLE_HRS_APP_RR_INTERVAL_RECORDS_MAX - g_hrs_app_control.records_num) % BLE_HRS_APP_RR_INTERVAL_RECORDS_MAX;
        g_hrs_app_control.records_num--;
        return g_hrs_app_control.heart_rate_measurement.rr_interval[index];
    }
}

static uint8_t ble_hrs_app_build_heart_rate_measurement_packet(uint8_t *measurement, uint8_t len)
{
    static uint8_t cnt = 0;
    uint8_t index = 1;
    uint16_t rr_interval;
#if BLE_HRS_APP_TEST_ENABLE
    extern void ble_hrs_app_data_get_sensor_data(void);
    ble_hrs_app_data_get_sensor_data();
#endif
    if (g_hrs_app_control.heart_rate_measurement.heart_rate_measurement_value > 0xFF) {
        g_hrs_app_control.heart_rate_measurement.flags.heart_rate_value_format = 1;
        memcpy(&measurement[index], &g_hrs_app_control.heart_rate_measurement.heart_rate_measurement_value, sizeof(uint16_t));
        index += sizeof(uint16_t);
    } else {
        g_hrs_app_control.heart_rate_measurement.flags.heart_rate_value_format = 0;
        measurement[index++] = (uint8_t)g_hrs_app_control.heart_rate_measurement.heart_rate_measurement_value;
    }

    cnt++;
#if BLE_HRS_FEATURE_ENERGY_EXPENDED_ENABLE
    if (cnt < 10) {
        g_hrs_app_control.heart_rate_measurement.flags.energy_expended_present = 0;
    } else {
        g_hrs_app_control.heart_rate_measurement.flags.energy_expended_present = 1;

        memcpy(&measurement[index], &g_hrs_app_control.heart_rate_measurement.energy_expended, sizeof(uint16_t));
        index += sizeof(uint16_t);
        cnt = 0;
    }
#endif
    g_hrs_app_control.heart_rate_measurement.flags.rr_interval_present = 0;
    while ((index + sizeof(uint16_t)) < len) {
        rr_interval = ble_hrs_app_get_oldest_rr_interval_record();
        if (rr_interval == 0) {
            break;
        }
        g_hrs_app_control.heart_rate_measurement.flags.rr_interval_present = 1;
        memcpy(&measurement[index], &rr_interval, sizeof(uint16_t));
        index += sizeof(uint16_t);
    }
    memcpy(&measurement[0], &g_hrs_app_control.heart_rate_measurement.flags, sizeof(uint8_t));
    return index;
}

static bt_status_t ble_hrs_app_bt_msg_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
#if 0
        case BT_GAP_LE_DISCONNECT_IND:
            //bt_hci_evt_disconnect_complete_t *ind = (bt_hci_evt_disconnect_complete_t *)buff;
            bt_gap_le_disconnect_ind_t *ind = (bt_gap_le_disconnect_ind_t *)buff;

            if (g_hrs_app_control.handle == ind->connection_handle) {
                g_hrs_app_control.handle = BT_HANDLE_INVALID;
                g_hrs_app_control.heart_rate_measurement_cccd = BT_ATT_CLIENT_CHARC_CONFIG_DEFAULT;
                bt_timer_ext_stop(BLE_SERVICE_HRS_APP_HEART_RATE_MEASUREMENT_TIMER_ID);
            } else {
            }
            break;
#endif
        default:
            break;
    }
    return 0;
}

void ble_hrs_app_init(void)
{
    bt_callback_manager_register_callback(bt_callback_type_app_event, (uint32_t)(MODULE_MASK_GAP), (void *)ble_hrs_app_bt_msg_callback);
}

void ble_hrs_app_set_sensor_contact_status(ble_hrs_app_sensor_contact_status_t contact_status)
{
#if BLE_HRS_FEATURE_SENSOR_CONTACT_ENABLE
    g_hrs_app_control.heart_rate_measurement.flags.sensor_contact_detected = contact_status;
    //LOG_MSGID_I(BLE_HRS_APP, "sensor contact status:%d \r\n", 1, contact_status);
#else
    LOG_MSGID_I(BLE_HRS_APP, "sensor contact feature is not support\r\n", 0);
#endif
}

void ble_hrs_app_set_heart_rate_measurement_value(uint16_t beats_per_minute)
{
    g_hrs_app_control.heart_rate_measurement.heart_rate_measurement_value = beats_per_minute;
}

void ble_hrs_app_set_energy_expended(uint16_t energy_expended)
{
    /**If the maximum value of 65535 kilo Joules is attained (0xFFFF), the field value should remain at 0xFFFF
     * so that the client can be made aware that a reset of the Energy Expended Field is required.
     **/
    g_hrs_app_control.heart_rate_measurement.energy_expended = energy_expended;
}

void ble_hrs_app_store_rr_interval_record(uint16_t rr_interval)
{
    /**
    *    Since this service does not provide for a time stamp to identify the measurement time (and age) of the data,
    *    the value of the Heart Rate Measurement characteristic shall be discarded if either the connection does not
    *    get established or if the notification is not successfully sent to the Client (e.g., link loss).
    **/
    if (g_hrs_app_control.handle == BT_HANDLE_INVALID) {
        return;
    }
    g_hrs_app_control.heart_rate_measurement.rr_interval[g_hrs_app_control.record_index] = rr_interval;//Can overwrite oldest
    g_hrs_app_control.record_index = (g_hrs_app_control.record_index + 1) % BLE_HRS_APP_RR_INTERVAL_RECORDS_MAX;
    if (g_hrs_app_control.records_num < BLE_HRS_APP_RR_INTERVAL_RECORDS_MAX) {
        g_hrs_app_control.records_num++;
    }
}

uint32_t ble_hrs_event_handler(ble_hrs_event_t event, bt_handle_t handle, void *data, uint16_t size, uint16_t offset)
{
    switch (event) {
        case BLE_HRS_EVENT_READ_BODY_SENSOR_LOCATION: {
            if (size != 0) {
                uint8_t *buff = (uint8_t *)data;
                *buff = g_hrs_app_control.body_sensor_location;
            }
            return sizeof(uint8_t);
        }

        case BLE_HRS_EVENT_READ_HEART_RATE_MEASUREMENT_CCCD: {
            if (size != 0) {
                uint16_t *buf = (uint16_t *)data;
                *buf = g_hrs_app_control.heart_rate_measurement_cccd;
            }
            return sizeof(uint16_t);
        }
        case BLE_HRS_EVENT_WRITE_HEART_RATE_MEASUREMENT_CCCD: {
            if (size != sizeof(uint16_t)) {
                return 0;
            }
            g_hrs_app_control.heart_rate_measurement_cccd = *(uint16_t *)data & BT_ATT_CLIENT_CHARC_CONFIG_NOTIFICATION;
            if (g_hrs_app_control.heart_rate_measurement_cccd == BT_ATT_CLIENT_CHARC_CONFIG_NOTIFICATION) {
                g_hrs_app_control.handle = handle;
#if BLE_HRS_APP_TEST_ENABLE
                ble_hrs_app_set_sensor_contact_status(BLE_HRS_APP_SENSOR_STATUS_CONTACT);
#endif
                bt_timer_ext_start(BLE_SERVICE_HRS_APP_HEART_RATE_MEASUREMENT_TIMER_ID, (uint32_t)&g_hrs_app_control, BLE_HRS_APP_TIMER_HEART_RATE_MEASUREMENT_TIMEOUT_MS, ble_hrs_app_heart_rate_measurement_timeout_handler);
                //LOG_MSGID_I(BLE_HRS_APP, "ble_hrs_event_handler,start timer id = %d \r\n", 1, BLE_SERVICE_HRS_APP_HEART_RATE_MEASUREMENT_TIMER_ID);
                //LOG_MSGID_I(BLE_HRS_APP, "g_hrs_app_control ADDR = %d handle = %d\r\n", 2, (uint32_t)&g_hrs_app_control,g_hrs_app_control.handle);
            } else {
                bt_timer_ext_stop(BLE_SERVICE_HRS_APP_HEART_RATE_MEASUREMENT_TIMER_ID);
            }
            return sizeof(uint16_t);
        }

        case BLE_HRS_EVENT_WRITE_HEART_RATE_CONTROL_POINT: {
            uint8_t opcode;
            if (size != 1) {
                return 0;
            }
            opcode = ((uint8_t *)data)[0];
            if (opcode != BLE_HRS_HRCP_OPCODE_RESET_ENERGY_EXPENDED) {
                ble_hrs_send_response(handle, BT_SIG_UUID16_HEART_RATE_CONTROL_POINT, BLE_HRS_ERRCODE_HRCP_VALUE_NOT_SUPPORTED);
            } else {
                ble_hrs_send_response(handle, BT_SIG_UUID16_HEART_RATE_CONTROL_POINT, BLE_HRS_ERRCODE_SUCCESS);
                ble_hrs_app_data_reset_energy_expended();
            }
            return BT_GATTS_ASYNC_RESPONSE;
        }
        default:
            break;

    }
    return 0;
}

#endif  /* AIR_BLE_HRS_ENABLE */

