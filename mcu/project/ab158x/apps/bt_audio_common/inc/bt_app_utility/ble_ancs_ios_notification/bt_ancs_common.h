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

#include "bt_type.h"
#include "bt_gattc_discovery.h"


/**
 *  @brief Define ancs event type.
 */
#define BLE_ANCS_EVENT_CONNECTED                0
#define BLE_ANCS_EVENT_IOS_NOTIFICATION         1
#define BLE_ANCS_EVENT_NOTIFICATION_ATTRIBUTE   2
#define BLE_ANCS_EVENT_APP_ATTRIBUTE            3
#define BLE_ANCS_EVENT_REQUEST_COMPLETED        4
#define BLE_ANCS_EVENT_DISCONNECTED             5
typedef uint8_t ble_ancs_event_type_t;


/**
 *  @brief Define bt_ancs_common status.
 */
#define ANCS_COMM_STATUS_READY              0
#define ANCS_COMM_STATUS_READ_NOTIF_SOURCE  1
#define ANCS_COMM_STATUS_READ_DATA_SOURCE   2
typedef uint8_t ble_ancs_comm_status_t;


#define BLE_ANCS_PROXY_CHARC_TYPE_NOTIFICATION_SOURCE   0   /**< The enum of the ANCS characteristic notification source.*/
#define BLE_ANCS_PROXY_CHARC_TYPE_CONTROL_POINT         1   /**< The enum of the ANCS characteristic control point.*/
#define BLE_ANCS_PROXY_CHARC_TYPE_DATA_SOURCE           2   /**< The enum of the ANCS characteristic data source.*/
typedef uint8_t ble_ancs_proxy_charc_type_t;             /**< The ANCS proxy characteristic type.*/

typedef struct {
    ble_ancs_event_type_t evt_type;
    bt_handle_t connection_handle;
    bt_status_t result;
    union {
        ble_ancs_event_notification_t notification;
        ble_ancs_event_attribute_t attribute;
    } data;
} ble_ancs_event_t;

typedef void(* ble_ancs_event_handler_t)(ble_ancs_event_t *p_evt);

typedef struct {
    ble_ancs_comm_status_t status;
    ble_ancs_event_handler_t evt_handler;
    bt_gattc_discovery_service_t service;
    bt_gattc_discovery_characteristic_t charc[BLE_ANCS_MAX_CHARC_NUMBER];
    bt_gattc_discovery_descriptor_t descrp[BLE_ANCS_MAX_CHARC_NUMBER];
} ble_ancs_srv_content_t;


void ble_ancs_start(ble_ancs_event_handler_t evt_handler);

bt_status_t ble_ancs_proxy_write_charc_cccd(bt_handle_t connection_handle, ble_ancs_proxy_charc_type_t type, uint16_t value);
bt_status_t ble_ancs_proxy_write_charc(bt_handle_t connection_handle, ble_ancs_proxy_charc_type_t type, uint8_t *value, uint16_t length);
bt_status_t ble_ancs_proxy_read_charc_cccd(bt_handle_t connection_handle, ble_ancs_proxy_charc_type_t type);
bt_gattc_discovery_characteristic_t *ble_ancs_proxy_find_charc_by_uuid(const uint8_t *uuid);
