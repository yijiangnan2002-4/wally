/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#ifndef __APP_DONGLE_LE_RACE_H__
#define __APP_DONGLE_LE_RACE_H__
#include <stdint.h>
#include <stdbool.h>
#ifdef MTK_RACE_CMD_ENABLE
#include "race_cmd.h"
#endif

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

#include "ui_shell_manager.h"

#include "ble_air_interface.h"

#define APP_DONGLE_LE_RACE_SINK_DEVICE_NONE       0x00
#define APP_DONGLE_LE_RACE_SINK_DEVICE_LEA        0x01
#define APP_DONGLE_LE_RACE_SINK_DEVICE_ULL_V2     0x02
#define APP_DONGLE_LE_RACE_SINK_DEVICE_BT_SRC     0x03
#define APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_HS  0x04
#define APP_DONGLE_LE_RACE_SINK_DEVICE_HID_KB     0x05
#define APP_DONGLE_LE_RACE_SINK_DEVICE_HID_MS     0x06
#define APP_DONGLE_LE_RACE_SINK_DEVICE_HID_AUDIO_EB  0x07
typedef uint8_t app_dongle_le_race_sink_device_t;

#define APP_DONGLE_LE_RACE_EVT_SCAN_CNF          0   /**< The result of #app_dongle_le_race_start_scan_device() or #app_dongle_le_race_stop_scan_device with payload #app_dongle_le_race_scan_cnf_t. */
#define APP_DONGLE_LE_RACE_EVT_ADV_REPORT        1
#define APP_DONGLE_LE_RACE_EVT_CONNECT_IND       2   /**< The event of connection complete with payload #app_dongle_le_race_connect_ind_t. */
#define APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND    3   /**< The event of disconnection complete with payload #app_dongle_le_race_disconnect_ind_t. */
typedef uint8_t app_dongle_le_race_event_t;

#define APP_DONGLE_LE_RACE_CONNECT_EVENT_CONNECTED       (0x01)
#define APP_DONGLE_LE_RACE_CONNECT_EVENT_DISCONNECTED    (0x00)
#define APP_DONGLE_LE_RACE_GET_STATUS_NOT_CONNECTED_STATUS   (0xFF)

#define APP_DONGLE_DEVICE_ID_BASE                0x80

typedef void (*app_dongle_le_race_event_callback_t)(app_dongle_le_race_event_t event, app_dongle_le_race_sink_device_t sink_type, void *buff);

/**************************************************************************************************
* Structure
**************************************************************************************************/
/**
 *  @brief This structure defines the parameter data type for event #APP_DONGLE_LE_RACE_EVT_SCAN_CNF.
 */
typedef struct {
    bt_status_t ret;    /**< The resutle of the command. */
    bool start_scan;    /**< The scan command to start or stop scan. */
} app_dongle_le_race_scan_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #APP_DONGLE_LE_RACE_EVT_CONNECT_IND.
 */
typedef struct {
    bt_status_t ret;                /**< The result of the connection. */
    bt_addr_t peer_addr;            /**< The address information of the remote device. */
    uint8_t group_id;               /**< The id of the coordinated set group. */
    uint8_t group_size;             /**< The size of the coordinated set group. */
} app_dongle_le_race_connect_ind_t, app_dongle_le_race_disconnect_ind_t;

typedef struct {
    uint8_t status;
    app_dongle_le_race_sink_device_t sink_type;
    bt_addr_t addr;
    uint8_t rssi;
    uint8_t name[0];
} PACKED app_dongle_le_adv_report_notify_t;

typedef struct {
    uint8_t status;
    app_dongle_le_race_sink_device_t sink_type;
    uint8_t connected;
    bt_addr_t addr;
    ble_air_device_id_t device_id;
    uint8_t group_id;
} PACKED app_dongle_le_race_connect_notify_t;

typedef struct {
    uint8_t start_scan;
    uint16_t timeout_seconds;
} PACKED app_dongle_le_race_scan_cmd_t;

typedef struct {
    uint8_t status;
} PACKED app_dongle_le_race_scan_rsp_t;

typedef struct {
    uint8_t connect;
    bt_addr_t addr;
} PACKED app_dongle_le_race_connect_cmd_t;

typedef struct {
    uint8_t type;
} PACKED app_dongle_le_race_set_link_type_cmd_t;

typedef struct {
    uint8_t status;
    uint8_t type;
} PACKED app_dongle_le_race_set_link_type_rsp_t, app_dongle_le_race_get_link_type_rsp_t;

typedef struct {
    uint8_t status;
    uint8_t connect;
    bt_addr_t addr;
} PACKED app_dongle_le_race_connect_rsp_t;

typedef struct {
    bt_addr_t addr;
    app_dongle_le_race_sink_device_t sink_type;
} PACKED app_dongle_le_race_get_device_status_cmd_t;

typedef struct {
    uint8_t status;
    bt_addr_t addr;
    uint8_t device_id;
    uint8_t group_id;
    uint8_t role;
} PACKED app_dongle_le_race_get_device_status_rsp_t;

typedef struct {
    bt_addr_t addr;
    uint8_t device_id;
    uint8_t group_id;
    uint8_t role;
} PACKED app_dongle_le_race_device_status_item_t;

typedef struct {
    uint8_t status;
    app_dongle_le_race_device_status_item_t devices_list[0];
} PACKED app_dongle_le_race_get_device_list_rsp_t;

typedef struct {
    uint8_t set_or_get;
    uint8_t group_id;
} PACKED app_dongle_le_race_switch_active_audio_cmd_t;

typedef struct {
    uint8_t status;
    bt_addr_t addr;
} PACKED app_dongle_le_race_remove_paired_record_rsp_t;

typedef struct {
    uint8_t status;
    bt_addr_t addr;
    char name[0];
} PACKED app_dongle_le_race_get_device_name_rsp_t;

typedef struct {
    uint8_t status;
    uint8_t set_or_get;
    uint8_t group_id;
} PACKED app_dongle_le_race_switch_active_audio_rsp_t;

void app_dongle_le_race_init(void);

void app_dongle_le_race_reply_scan_report(uint8_t sink_type, uint8_t addr_type, uint8_t *addr, uint8_t rssi,
                                          const char *name, uint8_t name_len);
void app_dongle_le_race_notify_connect_event(uint8_t status, uint8_t sink_type, bool conn_or_disconn,
                                             uint8_t addr_type, uint8_t *addr, uint8_t device_id, uint8_t group_id);

bool app_dongle_le_race_cmd_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len);

bool app_dongle_le_race_interaction_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len);
void app_dongle_le_race_set_current_sink_device(app_dongle_le_race_sink_device_t sink_device);
#ifdef MTK_RACE_CMD_ENABLE
void *app_dongle_le_race_cmd_handler(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id);
#endif

#endif

