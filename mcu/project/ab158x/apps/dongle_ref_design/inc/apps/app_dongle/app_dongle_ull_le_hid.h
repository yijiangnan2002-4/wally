/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

 #ifndef __APP_DONGLE_ULL_LE_HID_H__
#define __APP_DONGLE_ULL_LE_HID_H__

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE

#include "app_dongle_le_race.h"
#include "bt_ull_le_hid_service.h"

#define APP_DONGLE_ULL_LE_HID_SCAN_NONE                 0x00
#define APP_DONGLE_ULL_LE_HID_SCAN_AUDIO                0x01
#define APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD             0x02
#define APP_DONGLE_ULL_LE_HID_SCAN_MOUSE                0x04
typedef uint16_t app_dongle_ull_le_hid_scan_t;

/******************************************************************************
 * BT HID Report ID Definition
 *****************************************************************************/
#define BT_HID_REPORT_ID_KB         0x01
#define BT_HID_REPORT_ID_MS         0x02
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
#define BT_HID_REPORT_ID_NVMS       0x03
#endif
#define BT_HID_REPORT_ID_CONSUMER   0x05

#define BT_HID_REPORT_ID_KB_NKEY_GROUP1    0x06
#define BT_HID_REPORT_ID_KB_NKEY_GROUP2    0x07
#define BT_HID_REPORT_ID_KB_NKEY_GROUP3    0x08

void app_dongle_ull_le_hid_srv_event_callback(bt_ull_event_t event, void *param, uint32_t param_len);
void app_dongle_ull_le_hid_register_race_callback(app_dongle_le_race_event_callback_t callback);
bt_status_t app_dongle_ull_le_hid_connect_device(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr, bool is_fast_pair);
bt_status_t app_dongle_ull_le_hid_disconnect_all_device(uint8_t reason);
bt_status_t app_dongle_ull_le_hid_disconnect_device(bt_ull_le_hid_srv_device_t device_type, uint8_t reason, bt_addr_t *addr);
bt_status_t app_dongle_ull_le_hid_start_reconnect(void);
bt_status_t app_dongle_ull_le_hid_stop_scan(app_dongle_ull_le_hid_scan_t scan_type);
bt_status_t app_dongle_ull_le_hid_start_scan(app_dongle_ull_le_hid_scan_t scan_type);
bt_status_t app_dongle_ull_le_hid_start_up(void);
bt_status_t app_dongle_ull_le_hid_set_scenaraio(bt_ull_le_hid_srv_app_scenario_t scenario);
bt_status_t app_dongle_ull_le_hid_get_scenaraio(bt_ull_le_hid_srv_app_scenario_t *p_scenario);
bt_bd_addr_t *app_dongle_ull_le_hid_get_addr_by_device_type(uint8_t device_type);
void app_dongle_ull_le_hid_stop_create_cis_timer(void);
bt_status_t app_dongle_ull_le_hid_fota_lock(bool lock);
bt_ull_le_hid_srv_app_scenario_t app_dongle_ull_le_get_scenario_from_ctx();
#if defined(AIR_PURE_GAMING_MS_ENABLE) || defined(AIR_PURE_GAMING_KB_ENABLE) || defined(AIR_PURE_GAMING_MS_KB_ENABLE)
void app_dongle_ull_le_clear_bond_info_by_device_type(bt_ull_le_hid_srv_device_t cur_device_type);
#endif

#if (defined MTK_RACE_CMD_ENABLE)
void app_dongle_ull_le_hid_get_device_list_handler(uint8_t race_channel, app_dongle_le_race_sink_device_t device_type);
void app_dongle_ull_le_hid_get_paired_list_handler(uint8_t race_channel, app_dongle_le_race_sink_device_t device_type);
void app_dongle_ull_le_hid_delete_device(app_dongle_le_race_sink_device_t device_type, bt_addr_t *addr);
void app_dongle_ull_le_hid_get_device_status_handler(uint8_t race_channel, app_dongle_le_race_sink_device_t device_type,
    app_dongle_le_race_get_device_status_cmd_t *cmd);
#endif

#if defined (AIR_PURE_GAMING_MS_ENABLE) || defined (AIR_PURE_GAMING_KB_ENABLE)
void app_dongle_ull_le_hid_ep_tx_reg(void);
#endif

#endif

#endif


