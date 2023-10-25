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


#include "app_hear_through_adv.h"
#include "app_hear_through_storage.h"
#include "app_hear_through_activity.h"
#include "apps_events_event_group.h"
#include "multi_ble_adv_manager.h"
#include "ui_shell_manager.h"
#include "apps_debug.h"
#include "bt_device_manager.h"
#include "bt_callback_manager.h"
#include "bt_app_common.h"
#include "bt_type.h"
#include "bt_gap.h"
#include "string.h"

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE

#define APP_HEAR_THROUGH_ADV_TAG "[HearThrough][ADV]"

const uint16_t hear_through_crc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

#define APP_HEAR_THROUGH_ADV_MAX_NAME_LEN        249

/**
 * @brief TODO how to handle EMP case
 */

typedef struct {
    uint8_t             connected_remote_address[BT_BD_ADDR_LEN];
    uint16_t            connected_name_crc16_value;
    char                connected_remote_name[APP_HEAR_THROUGH_ADV_MAX_NAME_LEN];
    bool                need_start_after_get_name;
    bool                adv_started;
    bool                adv_timeout_running;
} app_hear_through_adv_context_t;

app_hear_through_adv_context_t app_hear_through_adv_context;


uint16_t app_hear_through_adv_crc16_generate(uint8_t *ptr, uint32_t length, uint16_t crc_init)
{
    const uint8_t *p;

    p = ptr;
    crc_init = crc_init ^ ~0U;

    while (length--) {
        crc_init = hear_through_crc16_table[(crc_init ^ *p++) & 0xFF] ^ (crc_init >> 8);
    }

    return crc_init ^ ~0U;
}

static bt_status_t app_hear_through_adv_gap_callback_handler(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    if ((status != BT_STATUS_SUCCESS) || (buffer == NULL)) {
        return status;
    }

    if (msg == BT_GAP_READ_REMOTE_NAME_COMPLETE_IND) {
        bt_gap_read_remote_name_complete_ind_t *name_complete_ind = (bt_gap_read_remote_name_complete_ind_t *)buffer;
        if (memcmp(name_complete_ind->address, &(app_hear_through_adv_context.connected_remote_address), BT_BD_ADDR_LEN) == 0) {
            memcpy(app_hear_through_adv_context.connected_remote_name, name_complete_ind->name, strlen(name_complete_ind->name));
            printf("HearingAid, connected remote name : %s", app_hear_through_adv_context.connected_remote_name);

            app_hear_through_adv_context.connected_name_crc16_value = app_hear_through_adv_crc16_generate((uint8_t *)(app_hear_through_adv_context.connected_remote_name),
                                                                                                          strlen(app_hear_through_adv_context.connected_remote_name),
                                                                                                          0xFFFF);

            if (app_hear_through_adv_context.need_start_after_get_name == true) {
                app_hear_through_adv_context.need_start_after_get_name = false;
                app_hear_through_adv_start();
            }
        }
    }

    return BT_STATUS_SUCCESS;
}

static uint32_t app_hear_through_adv_get_ble_adv_data_handler(multi_ble_adv_info_t *adv_info)
{
    if (adv_info == NULL) {
        return MULTI_BLE_ADV_NEED_GEN_ADV_PARAM | MULTI_BLE_ADV_NEED_GEN_ADV_DATA | MULTI_BLE_ADV_NEED_GEN_SCAN_RSP;
    }

    if (adv_info) {
        bt_app_common_generate_default_adv_data(adv_info->adv_param,
                                                adv_info->adv_data,
                                                adv_info->scan_rsp,
                                                NULL,
                                                0);
    }
    return 0;

#if 0
    bt_bd_addr_t *edr_addr = bt_device_manager_get_local_address();
#if 1
    bt_gap_le_set_ext_advertising_data_t *adv_data = (bt_gap_le_set_ext_advertising_data_t *)adv_info->adv_data;
    if ((adv_data != NULL) && (edr_addr != NULL)) {
        adv_data->data_length = 10;
        adv_data->data[0] = 9;
        adv_data->data[1] = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
        memcpy(adv_data->data + 2, edr_addr, BT_BD_ADDR_LEN);
        memcpy(adv_data->data + 2 + BT_BD_ADDR_LEN, &app_hear_through_adv_context.connected_name_crc16_value, sizeof(uint16_t));
    }
    return MULTI_BLE_ADV_NEED_GEN_ADV_PARAM | MULTI_BLE_ADV_NEED_GEN_SCAN_RSP;
#else
    bt_gap_le_set_ext_scan_response_data_t *scan_rsp = (bt_gap_le_set_ext_scan_response_data_t *)adv_info->scan_rsp;
    if ((scan_rsp != NULL) && (edr_addr != NULL)) {
        adv_data->data_length = 10;
        scan_rsp->data[0] = 9;
        scan_rsp->data[1] = BT_GAP_LE_AD_TYPE_PUBLIC_TARGET_ADDRESS;
        memcpy(scan_rsp->data + 2, edr_addr, BT_BD_ADDR_LEN);
        memcpy(scan_rsp->data + 2 + BT_BD_ADDR_LEN, &app_hear_through_adv_context.connected_name_crc16_value, sizeof(uint16_t));
    }
    return MULTI_BLE_ADV_NEED_GEN_ADV_PARAM | MULTI_BLE_ADV_NEED_GEN_ADV_DATA;
#endif
#endif
}

void app_hear_through_adv_init()
{
    memset(&app_hear_through_adv_context, 0, sizeof(app_hear_through_adv_context_t));
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          MODULE_MASK_GAP,
                                          (void *)app_hear_through_adv_gap_callback_handler);
}

void app_hear_through_adv_set_connected_remote_address(bt_bd_addr_t *remote_address)
{
    static uint8_t zero_addr[BT_BD_ADDR_LEN] = {0};
    if (remote_address == NULL) {
        return;
    }

    if (memcmp(remote_address, zero_addr, BT_BD_ADDR_LEN) == 0) {
        return;
    }

    memcpy(app_hear_through_adv_context.connected_remote_address, remote_address, BT_BD_ADDR_LEN);
    bt_gap_read_remote_name((const bt_bd_addr_t *)(app_hear_through_adv_context.connected_remote_address));
    app_hear_through_adv_context.adv_started = false;
    app_hear_through_adv_context.adv_timeout_running = false;
}

void app_hear_through_adv_clear_connected_remote_address()
{
    memset(app_hear_through_adv_context.connected_remote_address, 0, BT_BD_ADDR_LEN);
    memset(app_hear_through_adv_context.connected_remote_name, 0, APP_HEAR_THROUGH_ADV_MAX_NAME_LEN);
    app_hear_through_adv_context.need_start_after_get_name = false;
    app_hear_through_adv_context.connected_name_crc16_value = 0x00;
    app_hear_through_adv_context.adv_started = false;
    app_hear_through_adv_context.adv_timeout_running = false;
}

void app_hear_through_adv_get_crc16_value(uint16_t *crc16_value)
{
    if (crc16_value != NULL) {
        *crc16_value = app_hear_through_adv_context.connected_name_crc16_value;
    }
}

void app_hear_through_adv_start()
{
    uint8_t remote_name_len = strlen(app_hear_through_adv_context.connected_remote_name);
    uint32_t timeout = app_hear_through_storage_get_ha_ble_advertising_timeout();

    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ADV_TAG"[app_hear_through_adv_start], adv started : %d, remote name length : %d, timeout : %d",
                     3,
                     app_hear_through_adv_context.adv_started,
                     remote_name_len,
                     timeout);

    if (app_hear_through_adv_context.adv_started == true) {
        return;
    }

    if (remote_name_len == 0) {
        app_hear_through_adv_context.need_start_after_get_name = true;
        return;
    }

    multi_ble_adv_manager_add_ble_adv(MULTI_ADV_INSTANCE_DEFAULT, app_hear_through_adv_get_ble_adv_data_handler, 1);
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_DEFAULT);

    if (timeout > 0) {
        ui_shell_send_event(false,
                            EVENT_PRIORITY_MIDDLE,
                            EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                            APP_HEAR_THROUGH_EVENT_ID_BLE_ADV_TIMEOUT,
                            NULL,
                            0,
                            NULL,
                            timeout);
        app_hear_through_adv_context.adv_timeout_running = true;
    }

    app_hear_through_adv_context.adv_started = true;
}

void app_hear_through_adv_stop()
{
    APPS_LOG_MSGID_I(APP_HEAR_THROUGH_ADV_TAG"[app_hear_through_adv_stop], Stop ADV, started : %d, timeout_running : %d",
                     2,
                     app_hear_through_adv_context.adv_started,
                     app_hear_through_adv_context.adv_timeout_running);

    if (app_hear_through_adv_context.adv_started == false) {
        return;
    }

    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_DEFAULT, app_hear_through_adv_get_ble_adv_data_handler);
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_DEFAULT);

    app_hear_through_adv_context.adv_started = false;

    if (app_hear_through_adv_context.adv_timeout_running == true) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_HEAR_THROUGH, APP_HEAR_THROUGH_EVENT_ID_BLE_ADV_TIMEOUT);
        app_hear_through_adv_context.adv_timeout_running = false;
    }
}

#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */


