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


#ifndef __BLE_APP_HRS_H__
#define __BLE_APP_HRS_H__

#include "bt_type.h"
#include "bt_platform.h"

BT_EXTERN_C_BEGIN

#define BLE_HRS_APP_SENSOR_STATUS_NO_CONTACT         0    /**< Sensor is not contacted. */
#define BLE_HRS_APP_SENSOR_STATUS_CONTACT            1    /**< Sensor is contacted. */
typedef uint8_t ble_hrs_app_sensor_contact_status_t;    /**< Sensor contact status. */

/**
 * @brief   This function is used to set sensor contact status.
 * @param[in] contact_status    is whether or not skin contacted.
 */
void ble_hrs_app_set_sensor_contact_status(ble_hrs_app_sensor_contact_status_t contact_status);

/**
 * @brief   This function is used to set heart rate.
 * @param[in] beats_per_minute    is heart rate.
 */
void ble_hrs_app_set_heart_rate_measurement_value(uint16_t beats_per_minute);

/**
 * @brief   This function is used to set energy expended.
 * @param[in] energy_expended    is the accumulated energy expended in kilo Joules since the last time it was reset.
 */
void ble_hrs_app_set_energy_expended(uint16_t energy_expended);

/**
 * @brief   This function is used to set rr interval.
 * @param[in] rr_interval    is the RR-Interval,Unit:1/1024 second.
 */
void ble_hrs_app_store_rr_interval_record(uint16_t rr_interval);

BT_EXTERN_C_END

#endif /* __BLE_APP_HRS_H__ */
