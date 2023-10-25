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
#include "timers.h"
#include "hal_trng.h"
#include "bt_callback_manager.h"
#include "bt_callback_manager_config.h"

#include "bt_gatts.h"
#include "ble_hrs.h"
#include "ble_app_hrs.h"
#include "ble_app_hrs_data.h"

static uint16_t g_hrs_app_data_energy_expended = 0;

void ble_hrs_app_data_reset_energy_expended(void)
{
    g_hrs_app_data_energy_expended = 0;
}

#if BLE_HRS_APP_TEST_ENABLE
/**
 * @brief   This function is used to get sensor data,and set sensor data to HRS app by API.
 */
void ble_hrs_app_data_get_sensor_data(void)
{
    /* When you detect Sensor data or the HRS APP proactively retrieves data, you can
     * use the following API to refresh data.
     */
    static uint8_t value = 0;
    g_hrs_app_data_energy_expended++;
    ble_hrs_app_set_sensor_contact_status(BLE_HRS_APP_SENSOR_STATUS_CONTACT);
    ble_hrs_app_set_heart_rate_measurement_value(0x3C);
    ble_hrs_app_set_energy_expended(g_hrs_app_data_energy_expended);
    ble_hrs_app_store_rr_interval_record(0x0250 + value);
    ble_hrs_app_store_rr_interval_record(0x0260 + value);
}
#endif

#endif  /* AIR_BLE_HRS_ENABLE */

