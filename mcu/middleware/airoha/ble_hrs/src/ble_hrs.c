/*
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

#include <stdint.h>
#include "syslog.h"
#include "ble_hrs.h"

/* Create the log control block as user wishes. Here we use 'BLE_HRS' as module name.
 * User needs to define their own log control blocks as project needs.
 * Please refer to the log develop guide under /doc folder for more details.
 */
log_create_module(BLE_HRS, PRINT_LEVEL_INFO);

/************************************************
*   Macro and Structure
*************************************************/
#define HRS_START_HANDLE                                0x0541      /**< Attribute Start Hanlde . */
#define HRS_VALUE_HANDLE_HEART_RATE_MEASUREMENT         0x0543      /**< Attribute Vlaue Hanlde of PLX Continuous Measurement. */
#define HRS_VALUE_HANDLE_BODY_SENSOR_LOCATION           0x0546      /**< Attribute Vlaue Hanlde of PLX Features. */
#define HRS_VALUE_HANDLE_HEART_RATE_CONTROL_POINT       0x0548      /**< Attribute Vlaue Hanlde of Record Access Control Point. */
#define HRS_END_HANDLE                                  0x0548      /**< Attribute End Hanlde . */

/**
 *  @brief This structure defines HRS attribute handle detail.
 */
typedef struct {
    uint16_t uuid_type;           /**< UUID type */
    uint16_t att_handle;                /**< Attribute handle */
} ble_hrs_attribute_handle_t;


/************************************************
*   Global
*************************************************/
const bt_uuid_t BT_SIG_UUID_BODY_SENSOR_LOCATION                = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_BODY_SENSOR_LOCATION);
const bt_uuid_t BT_SIG_UUID_HRS_HEART_RATE_MEASUREMENT          = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_HEART_RATE_MEASUREMENT);
const bt_uuid_t BT_SIG_UUID_HEART_RATE_CONTROL_POINT            = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_HEART_RATE_CONTROL_POINT);

static const ble_hrs_attribute_handle_t g_hrs_att_handle_tbl[] = {
    {BT_SIG_UUID16_HEART_RATE_MEASUREMENT,      HRS_VALUE_HANDLE_HEART_RATE_MEASUREMENT},
    {BT_SIG_UUID16_BODY_SENSOR_LOCATION,        HRS_VALUE_HANDLE_BODY_SENSOR_LOCATION},
    {BT_SIG_UUID16_HEART_RATE_CONTROL_POINT,    HRS_VALUE_HANDLE_HEART_RATE_CONTROL_POINT},
};

/************************************************
* Prototype
*************************************************/
#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:ble_hrs_event_handler=default_ble_hrs_event_handler")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak ble_hrs_event_handler = default_ble_hrs_event_handler
#else
#error "Unsupported Platform"
#endif

/************************************************
* Static functions
*************************************************/
static uint32_t ble_hrs_body_sensor_location_callback(const uint8_t rw, bt_handle_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_hrs_heart_rate_measurement_client_config_callback(const uint8_t rw, bt_handle_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_hrs_heart_rate_control_point_callback(const uint8_t rw, bt_handle_t handle, void *data, uint16_t size, uint16_t offset);
static uint16_t ble_hrs_get_value_handle(uint16_t bt_sig_uuid16);

BT_GATTS_NEW_PRIMARY_SERVICE_16(ble_hrs_primary_service, BT_SIG_UUID16_HEART_RATE_SERVICE);


/** Heart Rate Measurement. */
BT_GATTS_NEW_CHARC_16(ble_hrs_char4_heart_rate_measurement,
                      BT_GATT_CHARC_PROP_NOTIFY, HRS_VALUE_HANDLE_HEART_RATE_MEASUREMENT, BT_SIG_UUID16_HEART_RATE_MEASUREMENT);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_hrs_heart_rate_measurement, BT_SIG_UUID_HRS_HEART_RATE_MEASUREMENT,
                                  0,
                                  NULL);

BT_GATTS_NEW_CLIENT_CHARC_CONFIG(ble_hrs_heart_rate_measurement_client_config,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 ble_hrs_heart_rate_measurement_client_config_callback);

/** Body Sensor Location */
#if BLE_HRS_FEATURE_BODY_SENSOR_LOCATION_ENABLE
BT_GATTS_NEW_CHARC_16(ble_hrs_char4_body_sensor_location,
                      BT_GATT_CHARC_PROP_READ,
                      HRS_VALUE_HANDLE_BODY_SENSOR_LOCATION,
                      BT_SIG_UUID16_BODY_SENSOR_LOCATION);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_hrs_body_sensor_location,
                                  BT_SIG_UUID_BODY_SENSOR_LOCATION,
                                  BT_GATTS_REC_PERM_READABLE,
                                  ble_hrs_body_sensor_location_callback);

#endif

/** Heart Rate Control Point. */
#if BLE_HRS_FEATURE_ENERGY_EXPENDED_ENABLE
BT_GATTS_NEW_CHARC_16(ble_hrs_char4_heart_rate_control_point,
                      BT_GATT_CHARC_PROP_WRITE, HRS_VALUE_HANDLE_HEART_RATE_CONTROL_POINT, BT_SIG_UUID16_HEART_RATE_CONTROL_POINT);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_hrs_heart_rate_control_point, BT_SIG_UUID_HEART_RATE_CONTROL_POINT,
                                  BT_GATTS_REC_PERM_WRITABLE,
                                  ble_hrs_heart_rate_control_point_callback);
#endif

static const bt_gatts_service_rec_t *ble_hrs_service_rec[] = {
    (const bt_gatts_service_rec_t *) &ble_hrs_primary_service,
    (const bt_gatts_service_rec_t *) &ble_hrs_char4_heart_rate_measurement,
    (const bt_gatts_service_rec_t *) &ble_hrs_heart_rate_measurement,
    (const bt_gatts_service_rec_t *) &ble_hrs_heart_rate_measurement_client_config,
#if BLE_HRS_FEATURE_BODY_SENSOR_LOCATION_ENABLE
    (const bt_gatts_service_rec_t *) &ble_hrs_char4_body_sensor_location,
    (const bt_gatts_service_rec_t *) &ble_hrs_body_sensor_location,
#endif
#if BLE_HRS_FEATURE_ENERGY_EXPENDED_ENABLE
    (const bt_gatts_service_rec_t *) &ble_hrs_char4_heart_rate_control_point,
    (const bt_gatts_service_rec_t *) &ble_hrs_heart_rate_control_point,
#endif
};

const bt_gatts_service_t ble_hrs_service = {
    .starting_handle = HRS_START_HANDLE,
    .ending_handle = HRS_END_HANDLE,
    .required_encryption_key_size = 0,
    .records = ble_hrs_service_rec
};

#if BLE_HRS_FEATURE_BODY_SENSOR_LOCATION_ENABLE
static uint32_t ble_hrs_body_sensor_location_callback(const uint8_t rw, bt_handle_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BLE_HRS, "ble_hrs_body_sensor_location_callback, handle:%d, opcode:%d, size:%d \r\n", 3, handle, rw, size);

    if ((handle != BT_HANDLE_INVALID) && (rw == BT_GATTS_CALLBACK_READ)) {
        return ble_hrs_event_handler(BLE_HRS_EVENT_READ_BODY_SENSOR_LOCATION, handle, data, size, offset);
    }
    return 0;
}
#endif


/** Client Characteristic Configuration Descriptor. */
static uint32_t ble_hrs_heart_rate_measurement_client_config_callback(const uint8_t rw, bt_handle_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BLE_HRS, "ble_hrs_heart_rate_measurement_client_config_callback, handle:%d, opcode:%d, size:%d \r\n", 3, handle, rw, size);

    if (BT_HANDLE_INVALID == handle) {
        return 0;
    }

    if (rw == BT_GATTS_CALLBACK_WRITE) {
        return ble_hrs_event_handler(BLE_HRS_EVENT_WRITE_HEART_RATE_MEASUREMENT_CCCD, handle, data, size, offset);
    } else if (rw == BT_GATTS_CALLBACK_READ) {
        return ble_hrs_event_handler(BLE_HRS_EVENT_READ_HEART_RATE_MEASUREMENT_CCCD, handle, data, size, offset);
    }
    return 0;
}

#if BLE_HRS_FEATURE_ENERGY_EXPENDED_ENABLE
static uint32_t ble_hrs_heart_rate_control_point_callback(const uint8_t rw, bt_handle_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BLE_HRS, "ble_hrs_heart_rate_control_point_callback, handle:%d, opcode:%d, size:%d \r\n", 3, handle, rw, size);

    if ((handle != BT_HANDLE_INVALID) && (rw == BT_GATTS_CALLBACK_WRITE)) {
        return ble_hrs_event_handler(BLE_HRS_EVENT_WRITE_HEART_RATE_CONTROL_POINT, handle, data, size, offset);
    }

    return 0;
}
#endif
static uint16_t ble_hrs_get_value_handle(uint16_t bt_sig_uuid16)
{
    uint16_t att_handle = 0;
    uint8_t i = 0;

    for (i = 0; i < sizeof(g_hrs_att_handle_tbl) / sizeof(ble_hrs_attribute_handle_t); i++) {
        if (g_hrs_att_handle_tbl[i].uuid_type == bt_sig_uuid16) {
            att_handle = g_hrs_att_handle_tbl[i].att_handle;
            break;
        }
    }

    return att_handle;
}

/************************************************
* Public functions
*************************************************/

uint32_t default_ble_hrs_event_handler(ble_hrs_event_t event, bt_handle_t handle, void *data, uint16_t size, uint16_t offset)
{
    return 0;
}

bt_status_t ble_hrs_send_response(bt_handle_t handle, uint16_t bt_sig_uuid16, uint8_t error_code)
{
    bt_gatts_error_rsp_t att_rsp;

    att_rsp.attribute_handle = ble_hrs_get_value_handle(bt_sig_uuid16);

    return bt_gatts_send_response(handle, error_code, BT_GATTS_CALLBACK_WRITE, (void *)&att_rsp);
}

bt_status_t ble_hrs_send_notification(bt_handle_t handle, uint16_t bt_sig_uuid16, uint8_t *data, uint8_t length)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    uint16_t att_handle = ble_hrs_get_value_handle(bt_sig_uuid16);
    uint8_t buf[100] = {0};
    bt_gattc_charc_value_notification_indication_t *pams_noti_rsp;
    pams_noti_rsp = (bt_gattc_charc_value_notification_indication_t *) buf;

    if ((handle == BT_HANDLE_INVALID) || (data == NULL) || (att_handle == 0)) {
        return BT_STATUS_FAIL;
    }

    pams_noti_rsp->att_req.opcode = BT_ATT_OPCODE_HANDLE_VALUE_NOTIFICATION;
    pams_noti_rsp->att_req.handle = att_handle;
    pams_noti_rsp->attribute_value_length = length + 3;//opcode + handle = 3

    memcpy((void *)(pams_noti_rsp->att_req.attribute_value), (void *)(data), length);

    ret = bt_gatts_send_charc_value_notification_indication(handle, pams_noti_rsp);
    return ret;
}

