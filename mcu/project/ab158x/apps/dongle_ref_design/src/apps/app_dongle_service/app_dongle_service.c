
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

/**
 * File: app_dongle_service.c
 *
 * Description: This file is the activity to handle common event.
 *
 */
#include "apps_dongle_sync_event.h"

#include "app_dongle_service.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_config_event_list.h"
//#include "apps_dongle_sync_event.h"
#include "apps_debug.h"

#include "bt_connection_manager.h"
#include "usb_main.h"
#include "ui_shell_manager.h"
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_service.h"
#endif

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)

#define TAG "[APP SERVICE]"

#define APP_SERVICE_CALLBACK_TABLE_COUNT        3

typedef struct {
    bool is_used;
    app_dongle_service_callback_t callback_table;
} app_service_callback_table_t;

typedef struct {
    bool                            info_is_headset_charging;
    uint32_t                        info_headset_battery_level;
    bool                            info_is_power_off;
} app_service_battery_info_t;

typedef struct {
    bool                            ctx_is_headset_connected;
    bool                            ctx_is_handshake_done;
    bt_bd_addr_t                    ctx_headset_address;
    app_service_battery_info_t      ctx_battery_info;
    uint8_t                         ctx_headset_fw_version[APP_DONGLE_SERVICE_HEADSET_FW_VERSION_BUF_LEN];
    app_service_callback_table_t    ctx_callback[APP_SERVICE_CALLBACK_TABLE_COUNT];
    uint8_t                         ctx_headset_type;
} app_service_context_t;

static app_service_context_t s_srv_ctx;
static int32_t s_le_ull_conn_nums = 0;

static void app_srv_ctx_reset()
{
    s_srv_ctx.ctx_is_headset_connected = false;
    s_srv_ctx.ctx_is_handshake_done = false;
    memset(s_srv_ctx.ctx_headset_address, 0, BT_BD_ADDR_LEN);
    memset(&s_srv_ctx.ctx_battery_info, 0, sizeof(app_service_battery_info_t));
    memset(&s_srv_ctx.ctx_headset_fw_version, 0, APP_DONGLE_SERVICE_HEADSET_FW_VERSION_BUF_LEN);
    //memset(&s_srv_ctx, 0, sizeof(app_service_context_t));
}

static bool _proc_ui_shell_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(TAG" create", 0);
            app_srv_ctx_reset();
        }
        break;
        default:
            break;
    }
    return ret;
}

static void _headset_disconnected(void) {
    app_srv_ctx_reset();

    uint8_t notify_index = 0;
    for (notify_index = 0; notify_index < APP_SERVICE_CALLBACK_TABLE_COUNT; notify_index ++) {
        if ((s_srv_ctx.ctx_callback[notify_index].is_used == true)
            && (s_srv_ctx.ctx_callback[notify_index].callback_table.headset_disconnected != NULL)) {
            s_srv_ctx.ctx_callback[notify_index].callback_table.headset_disconnected();
        }
    }
}

static bool _proc_bt_cm_event(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* The follow logic just for ULL */
    if (event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE) {
        bt_cm_remote_info_update_ind_t *info = (bt_cm_remote_info_update_ind_t *)extra_data;
        if ((!(info->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL)))
            && (info->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL))) {

            APPS_LOG_MSGID_I(TAG" Connected Headset with address : 0x%02X:%02X:%02X:%02X:%02X:%02X. Waiting for Headset HELLO", 6,
                             info->address[0], info->address[1], info->address[2],
                             info->address[3], info->address[4], info->address[5]);

            memcpy(s_srv_ctx.ctx_headset_address, info->address, BT_BD_ADDR_LEN);
            s_srv_ctx.ctx_is_headset_connected = true;

        } else if ((info->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL))
                   && (!(info->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL)))) {
            APPS_LOG_MSGID_I(TAG" Disconnected with Headset, address : 0x%02X:%02X:%02X:%02X:%02X:%02X", 6,
                             info->address[0], info->address[1], info->address[2],
                             info->address[3], info->address[4], info->address[5]);

            _headset_disconnected();
        }
    }
    return false;
}

static bool _proc_bt_ull_data_event(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = true;
    uint8_t notify_index = 0;

    apps_dongle_event_sync_info_t *pkg = (apps_dongle_event_sync_info_t *)extra_data;
    if (pkg->event_group == EVENT_GROUP_UI_SHELL_APP_SERVICE) {

        if (s_srv_ctx.ctx_is_headset_connected == false) {
            APPS_LOG_MSGID_E(TAG" ULL Data event, but headset is not connected", 0);
            return false;
        }

        switch (pkg->event_id) {
            case APP_DONGLE_SERVICE_HEADSET_EVENT_HANDSHAKE_START: {
                #ifdef MTK_RACE_CMD_ENABLE
                uint8_t channel_id = apps_dongle_sync_event_get_channel_id(extra_data, data_len);
                #endif
                if (s_srv_ctx.ctx_is_handshake_done == true) {
                    APPS_LOG_MSGID_I(TAG" Headset connected. Already handshake done", 0);
                    //apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_APP_SERVICE, APP_DONGLE_SERVICE_DONGLE_EVENT_HANDSHAKE_START_ACK);

                    #ifdef MTK_RACE_CMD_ENABLE
                    apps_dongle_sync_event_send_extra_by_channel(EVENT_GROUP_UI_SHELL_APP_SERVICE,
                                                APP_DONGLE_SERVICE_DONGLE_EVENT_HANDSHAKE_START_ACK,
                                                NULL, 0, channel_id);
                    #endif
                    
                    break;
                }
                if (pkg->extra_data_len != 0) {
                    uint8_t* type = (uint8_t *)&pkg->data;
                    s_srv_ctx.ctx_headset_type = *type;
                } else {
                    s_srv_ctx.ctx_headset_type = APP_DONGLE_SERVICE_HEADSET_TYPE_6X;
                }
                APPS_LOG_MSGID_I(TAG" Headset connected. Start to handshake with Headset,type=%d, response start ACK",
                                1, s_srv_ctx.ctx_headset_type);
                //apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_APP_SERVICE, APP_DONGLE_SERVICE_DONGLE_EVENT_HANDSHAKE_START_ACK);
                #ifdef MTK_RACE_CMD_ENABLE
                apps_dongle_sync_event_send_extra_by_channel(EVENT_GROUP_UI_SHELL_APP_SERVICE,
                                                APP_DONGLE_SERVICE_DONGLE_EVENT_HANDSHAKE_START_ACK,
                                                NULL, 0, channel_id);
                #endif
            }
            break;

            case APP_DONGLE_SERVICE_HEADSET_EVENT_HANDSHAKE_DONE: {
                APPS_LOG_MSGID_I(TAG" Headset connected. Handshake done with Headset, response done ACK", 0);
                s_srv_ctx.ctx_is_handshake_done = true;

                #ifdef MTK_RACE_CMD_ENABLE
                //apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_APP_SERVICE, APP_DONGLE_SERVICE_DONGLE_EVENT_HANDSHAKE_DONE_ACK);
                uint8_t channel_id = apps_dongle_sync_event_get_channel_id(extra_data, data_len);

                apps_dongle_sync_event_send_extra_by_channel(EVENT_GROUP_UI_SHELL_APP_SERVICE,
                                                APP_DONGLE_SERVICE_DONGLE_EVENT_HANDSHAKE_DONE_ACK,
                                                NULL, 0, channel_id);
                #endif

                for (notify_index = 0; notify_index < APP_SERVICE_CALLBACK_TABLE_COUNT; notify_index ++) {
                    if ((s_srv_ctx.ctx_callback[notify_index].is_used == true)
                        && (s_srv_ctx.ctx_callback[notify_index].callback_table.headset_connected != NULL)) {
                        s_srv_ctx.ctx_callback[notify_index].callback_table.headset_connected();
                    }
                }
#ifdef AIR_USB_ENABLE
                if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
                    app_dongle_service_dongle_mode_t mode = APP_DONGLE_SERVICE_DONGLE_MODE_XBOX;
                    APPS_LOG_MSGID_I(TAG" app_dongle_service_update_dongle_mode, mode=%d", 1, mode);

                    #ifdef MTK_RACE_CMD_ENABLE
                    apps_dongle_sync_event_send_extra_by_channel(EVENT_GROUP_UI_SHELL_APP_SERVICE,
                                                      APP_DONGLE_SERVICE_DONGLE_EVENT_MODE_UPDATE,
                                                      (void *)(&mode),
                                                      sizeof(uint8_t), channel_id);
                    #endif
                    
                    //app_dongle_service_update_dongle_mode(APP_DONGLE_SERVICE_DONGLE_MODE_XBOX, channel_id);
                }
#endif /* AIR_USB_ENABLE */
            }
            break;

            case APP_DONGLE_SERVICE_HEADSET_EVENT_BATTERY_LEVEL: {
                if (pkg->extra_data_len != 0) {
                    if (s_srv_ctx.ctx_battery_info.info_headset_battery_level != (*((uint32_t *) & (pkg->data)))) {
                        s_srv_ctx.ctx_battery_info.info_headset_battery_level = *((uint32_t *) & (pkg->data));
                        for (notify_index = 0; notify_index < APP_SERVICE_CALLBACK_TABLE_COUNT; notify_index ++) {
                            if ((s_srv_ctx.ctx_callback[notify_index].is_used == true)
                                && (s_srv_ctx.ctx_callback[notify_index].callback_table.headset_battery_level_changed != NULL)) {
                                s_srv_ctx.ctx_callback[notify_index].callback_table.headset_battery_level_changed(s_srv_ctx.ctx_battery_info.info_headset_battery_level);
                            }
                        }
                    }

                    APPS_LOG_MSGID_I(TAG" Headset report battery level: %d.", 1, s_srv_ctx.ctx_battery_info.info_headset_battery_level);
                }
            }
            break;
            case APP_DONGLE_SERVICE_HEADSET_EVENT_CHARGING_STATE: {
                if (pkg->extra_data_len != 0) {
                    if (s_srv_ctx.ctx_battery_info.info_is_headset_charging != (*((bool *) & (pkg->data)))) {
                        s_srv_ctx.ctx_battery_info.info_is_headset_charging = *((bool *) & (pkg->data));
                        for (notify_index = 0; notify_index < APP_SERVICE_CALLBACK_TABLE_COUNT; notify_index ++) {
                            if ((s_srv_ctx.ctx_callback[notify_index].is_used == true)
                                && (s_srv_ctx.ctx_callback[notify_index].callback_table.headset_battery_charging_changed != NULL)) {
                                s_srv_ctx.ctx_callback[notify_index].callback_table.headset_battery_charging_changed(s_srv_ctx.ctx_battery_info.info_is_headset_charging);
                            }
                        }
                    }

                    APPS_LOG_MSGID_I(TAG" Headset report charging state: %d.", 1, s_srv_ctx.ctx_battery_info.info_is_headset_charging);
                }
            }
            break;
            case APP_DONGLE_SERVICE_HEADSET_EVENT_VERSION: {
                if (pkg->extra_data_len != 0) {
                    memcpy(s_srv_ctx.ctx_headset_fw_version, pkg->data,
                           pkg->extra_data_len > APP_DONGLE_SERVICE_HEADSET_FW_VERSION_BUF_LEN ? APP_DONGLE_SERVICE_HEADSET_FW_VERSION_BUF_LEN : pkg->extra_data_len);
                    APPS_LOG_MSGID_I(TAG" Headset report version: 0x%x, 0x%x, 0x%x, 0x%x.", 4,
                                     s_srv_ctx.ctx_headset_fw_version[0],
                                     s_srv_ctx.ctx_headset_fw_version[1],
                                     s_srv_ctx.ctx_headset_fw_version[2],
                                     s_srv_ctx.ctx_headset_fw_version[3]);
                }
            }
            break;
            case APP_DONGLE_SERVICE_HEADSET_EVENT_POWER_OFF: {
                APPS_LOG_MSGID_I(TAG" Headset report power off event, connection nums=%d", 1, s_le_ull_conn_nums);
                if (s_le_ull_conn_nums > 0) {
                    break;
                }
                for (notify_index = 0; notify_index < APP_SERVICE_CALLBACK_TABLE_COUNT; notify_index ++) {
                    if ((s_srv_ctx.ctx_callback[notify_index].is_used == true)
                        && (s_srv_ctx.ctx_callback[notify_index].callback_table.headset_headset_power_off != NULL)) {
                        s_srv_ctx.ctx_battery_info.info_is_power_off = true;
                        s_srv_ctx.ctx_callback[notify_index].callback_table.headset_headset_power_off();
                    }
                }
            }
            break;
        }
    } else {
        ret = false;
    }

    return ret;
}

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
static bool apps_dongle_proc_ull_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    APPS_LOG_MSGID_I(TAG"LE ULL event=%d, conn_nums=%d", 2, event_id, s_le_ull_conn_nums);
    switch (event_id) {
        case BT_ULL_EVENT_LE_CONNECTED: {
            s_srv_ctx.ctx_is_headset_connected = true;
            s_le_ull_conn_nums+=1;
            break;
        }
        case BT_ULL_EVENT_LE_DISCONNECTED: {
            s_le_ull_conn_nums--;
            if (s_le_ull_conn_nums == 0) {
                _headset_disconnected();
            }
            s_le_ull_conn_nums = s_le_ull_conn_nums >= 0 ? s_le_ull_conn_nums : 0;
            break;
        }
    }
    return false;
}
#endif

bool app_dongle_service_activity_proc(ui_shell_activity_t *self,
                                      uint32_t event_group,
                                      uint32_t event_id,
                                      void *extra_data,
                                      size_t data_len)
{
    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            ret = _proc_bt_cm_event(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_DONGLE_DATA: {
            ret = _proc_bt_ull_data_event(self, event_id, extra_data, data_len);
            break;
        }
        #ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY:
            ret = apps_dongle_proc_ull_event(self, event_id, extra_data, data_len);
            break;
        #endif
        case EVENT_GROUP_UI_SHELL_APP_SERVICE: {
            if (event_id == APP_DONGLE_SERVICE_MODE_SWITCH_EVENT) {
                if (s_srv_ctx.ctx_is_headset_connected == true) {
                    bt_cm_connect_t disconnect_param;
                    memcpy(disconnect_param.address, s_srv_ctx.ctx_headset_address, BT_BD_ADDR_LEN);
                    disconnect_param.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL);
                    bt_cm_disconnect(&disconnect_param);
                }
                s_srv_ctx.ctx_battery_info.info_is_power_off = true;
                ui_shell_send_event(TRUE,
                                    EVENT_PRIORITY_HIGHEST,
                                    EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_REQUEST_REBOOT,
                                    NULL, 0, NULL, 500);
            }
        }
        break;
        default: {
            break;
        }
    }

    return ret;
}

bool app_dongle_service_is_headset_connected()
{
    return s_srv_ctx.ctx_is_headset_connected && s_srv_ctx.ctx_is_handshake_done;
}

uint32_t app_dongle_service_get_headset_battery_level()
{
    return s_srv_ctx.ctx_battery_info.info_headset_battery_level;
}

bool app_dongle_service_is_headset_charging()
{
    return s_srv_ctx.ctx_battery_info.info_is_headset_charging;
}

bool app_dongle_service_is_power_off() {
     return s_srv_ctx.ctx_battery_info.info_is_power_off;
}

uint32_t app_dongle_service_get_headset_fw_version()
{
    uint32_t version = (s_srv_ctx.ctx_headset_fw_version[0])
                       + (s_srv_ctx.ctx_headset_fw_version[1] << 8)
                       + (s_srv_ctx.ctx_headset_fw_version[2] << 16)
                       + (s_srv_ctx.ctx_headset_fw_version[3] << 24);
    APPS_LOG_MSGID_I(TAG" app_dongle_service_get_headset_fw_version : 0x%x.", 1, version);
    return version;
}

void app_dongle_service_update_dongle_mode(app_dongle_service_dongle_mode_t mode)
{
    APPS_LOG_MSGID_I(TAG" app_dongle_service_update_dongle_mode, mode=%d", 1, mode);
    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_SERVICE,
                                      APP_DONGLE_SERVICE_DONGLE_EVENT_MODE_UPDATE,
                                      (void *)(&mode),
                                      sizeof(uint8_t));
}

void app_dongle_service_notify_off_state(void)
{
    APPS_LOG_MSGID_I(TAG" app_dongle_service_notify_off_state", 0);
    apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_APP_SERVICE,
                                APP_DONGLE_SERVICE_DONGLE_EVENT_STATE_OFF);
}

void app_dongle_service_notify_reset_state(void)
{
    APPS_LOG_MSGID_I(TAG" app_dongle_service_notify_reset_state", 0);
    apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_APP_SERVICE,
                                APP_DONGLE_SERVICE_DONGLE_EVENT_STATE_RESET);
}

void app_dongle_service_update_volume_status(uint32_t volume_status, uint8_t which)
{
    app_dongle_service_volume_change_t volume_changed = {
        .new_volume = volume_status,
        .which_changed = which
    };
    apps_dongle_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_SERVICE,
                                      APP_DONGLE_SERVICE_DONGLE_EVENT_VOLUME_STATE_UPDATE,
                                      &volume_changed,
                                      sizeof(app_dongle_service_volume_change_t));
}

void app_dongle_service_register_callback(app_dongle_service_callback_t callback_table)
{
    uint8_t index = 0;
    bool register_succeed = false;

    for (index = 0; index < APP_SERVICE_CALLBACK_TABLE_COUNT; index ++) {
        if (s_srv_ctx.ctx_callback[index].is_used == false) {
            memcpy(&(s_srv_ctx.ctx_callback[index].callback_table), &callback_table, sizeof(app_dongle_service_callback_t));
            s_srv_ctx.ctx_callback[index].is_used = true;

            register_succeed = true;
            break;
        }
    }

    if (register_succeed == false) {
        APPS_LOG_MSGID_I(TAG" app_dongle_service_register_callback, register failed, all callback table has been used", 0);
    }
}

uint8_t app_dongle_service_get_headset_type()
{
    return s_srv_ctx.ctx_headset_type;
}

#endif /* AIR_BT_ULTRA_LOW_LATENCY_ENABLE */
