/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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
 * File: app_bt_state_service.c
 *
 * Description: This file provides many utility function to control BT switch and visibility.
 *
 */

#include "app_bt_takeover_service.h"

#include "app_bt_conn_manager.h"
#include "apps_debug.h"
#include "apps_events_interaction_event.h"
#include "apps_events_event_group.h"
#include "app_music_utils.h"
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "app_lea_service.h"
#include "app_lea_service_conn_mgr.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "app_ull_idle_activity.h"
#endif

#include "bt_connection_manager.h"
#include "bt_device_manager.h"
#include "bt_device_manager_link_record.h"
#include "bt_sink_srv.h"
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
#include "bt_sink_srv_state_manager.h"
#endif

#ifdef MTK_AWS_MCE_ENABLE
#include "apps_aws_sync_event.h"
#include "app_rho_idle_activity.h"
#include "bt_aws_mce_srv.h"
#endif
#if defined(AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE) && defined(AIR_XIAOAI_AUDIO_SWITCH_ENABLE)
#include "app_va_xiaoai_miui_fast_connect.h"
#include "app_va_xiaoai_hfp_at_cmd.h"
#endif
#include "FreeRTOS.h"
#include "ui_shell_manager.h"



/**================================================================================*/
/**                              Definition & Structure                            */
/**================================================================================*/
#define LOG_TAG     "[APP_TAKEOVER]"

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

typedef enum {
    APP_BT_TAKEOVER_PRIORITY_NONE       = 0,
    APP_BT_TAKEOVER_PRIORITY_LOWEST,
    APP_BT_TAKEOVER_PRIORITY_LOW,
    APP_BT_TAKEOVER_PRIORITY_MID,                       // Last played device
    APP_BT_TAKEOVER_PRIORITY_HIGH,                      // Music/Call streaming, VA or OTA ongoing
    APP_BT_TAKEOVER_PRIORITY_HIGHEST,                   // Such as Dongle
} app_bt_takeover_priority_t;

typedef struct {
    uint8_t                             addr_type;
    uint8_t                             link_type;
    uint8_t                             addr[BT_BD_ADDR_LEN];
    uint8_t                             is_dongle;
    uint8_t                             is_ull2;
    app_bt_takeover_priority_t          priority;
} PACKED app_bt_takeover_item_t;

#define APP_BT_TAKEOVER_INVALID_INDEX                   (-1)
#define APP_BT_TAKEOVER_MAX_ITEM_SIZE                   (3 + 1)

static app_bt_takeover_service_allow_func_t app_bt_takeover_user[APP_BT_TAKEOVER_ID_MAX] = {0};

static bt_device_manager_link_record_item_t app_bt_takeover_disconnect_item = {0};

static app_bt_takeover_device_t             app_bt_takeover_last_device = {0};

#if defined(AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE) && defined(AIR_XIAOAI_AUDIO_SWITCH_ENABLE)
#define APP_BT_TAKEOVER_RETRIGGER_TIME1                 (3 * 1000)
#define APP_BT_TAKEOVER_RETRIGGER_TIME2                 (12 * 1000)

static uint8_t                              app_bt_takeover_miui_sass_addr[BT_BD_ADDR_LEN] = {0};
#endif



/**================================================================================*/
/**                                 Internal Function                              */
/**================================================================================*/
#ifdef AIR_BT_TAKEOVER_ENABLE
void app_bt_takeover_service_disconnect_edr(const uint8_t *addr)
{
    bt_cm_connect_t disconn_param = {0};
    disconn_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
    memcpy(&(disconn_param.address), addr, sizeof(bt_bd_addr_t));
    bt_status_t bt_status = bt_cm_disconnect(&disconn_param);

    APPS_LOG_MSGID_I(LOG_TAG" disconnect_edr, addr=%02X:%02X:%02X:%02X:%02X:%02X bt_status=0x%08X",
                     7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status);
}

static void app_bt_takeover_service_set_last_device(const uint8_t *addr, uint8_t addr_type, bool is_edr)
{
    memset(&app_bt_takeover_last_device, 0, sizeof(app_bt_takeover_device_t));
    memcpy(app_bt_takeover_last_device.addr, addr, BT_BD_ADDR_LEN);
    app_bt_takeover_last_device.addr_type = addr_type;
    app_bt_takeover_last_device.is_edr = is_edr;
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    if (!is_edr) {
        bool is_ull2 = app_lea_conn_mgr_is_ull_link(addr);
        if (is_ull2) {
            app_bt_takeover_last_device.is_ull2 = TRUE;
            app_bt_takeover_last_device.is_dongle = TRUE;
        } else if (app_lea_conn_mgr_is_lea_dongle_link(addr)) {
            app_bt_takeover_last_device.is_ull2 = FALSE;
            app_bt_takeover_last_device.is_dongle = TRUE;
        }
    }
#endif
}

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
static uint8_t app_bt_takeover_service_get_link_record_type(uint8_t link_type)
{
    if (link_type == BT_SINK_SRV_DEVICE_EDR) {
        return BT_DEVICE_MANAGER_LINK_TYPE_EDR;
    } else if (link_type == BT_SINK_SRV_DEVICE_LE) {
        return BT_DEVICE_MANAGER_LINK_TYPE_LE;
    } else {
        return 0xFF;
    }
}
#endif

static void app_bt_takeover_service_print(app_bt_takeover_item_t *list, uint8_t conn_num)
{
    for (int i = 0; i < conn_num; i++) {
        uint8_t *addr = list[i].addr;
        APPS_LOG_MSGID_I(LOG_TAG" run_takeover, [%d] link_type=%d addr=%08X%04X priority=%d addr_type=%d is_dongle=%d is_ull2=%d",
                         8, i, list[i].link_type, *((uint32_t *)(addr + 2)), *((uint16_t *)addr),
                         list[i].priority, list[i].addr_type, list[i].is_dongle, list[i].is_ull2);
    }
}
#endif

static bool app_bt_takeover_service_bt_sink_allow_cb(const bt_bd_addr_t remote_addr)
{
    bool ret = TRUE;
    bt_sink_srv_device_state_t state = {{0}, 0, 0, 0};
    if (bt_sink_srv_get_device_state((const bt_bd_addr_t *)remote_addr, &state, 1) == 1) {
        if (state.music_state == BT_SINK_SRV_STATE_STREAMING
            || state.call_state > BT_SINK_SRV_STATE_STREAMING
            || state.sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED) {
            const uint8_t *addr = (const uint8_t *)remote_addr;
            APPS_LOG_MSGID_I(LOG_TAG" bt_sink_allow_cb, disallow addr=%08X%04X music=%04X call=%04X eSCO=%d",
                             5, *((uint32_t *)(addr + 2)), *((uint16_t *)addr),
                             state.music_state, state.call_state, state.sco_state);
            ret = FALSE;
        }
    }

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
    if (ret && app_lea_conn_mgr_is_ull_link(remote_addr)) {
        const uint8_t *addr = (const uint8_t *)remote_addr;
        bool ull_streaming = app_music_get_ull_is_streaming();
        APPS_LOG_MSGID_I(LOG_TAG" bt_sink_allow_cb, disallow addr=%08X%04X ull_streaming=%d",
                         3, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), ull_streaming);
        ret = (!ull_streaming);
    }
#endif

    return ret;
}

#ifdef AIR_3_LINK_MULTI_POINT_ENABLE
static void app_bt_takeover_service_check_3edr_lea(void)
{
    const bt_device_manager_link_record_t *link_record = bt_device_manager_link_record_get_connected_link();
    uint8_t conn_num = link_record->connected_num;
    if (conn_num == 3) {
        return;
    }

    bt_device_manager_link_record_item_t *link_record_list = (bt_device_manager_link_record_item_t *)&link_record->connected_device[0];
    for (int i = 0; i < conn_num; i++) {
        uint8_t link_type = link_record_list[i].link_type;
        uint8_t addr_type = link_record_list[i].remote_bd_type;
        uint8_t *addr = (uint8_t *)link_record_list[i].remote_addr;

        if (link_type == BT_DEVICE_MANAGER_LINK_TYPE_LE) {
            app_bt_takeover_service_set_last_device(addr, addr_type, FALSE);
            app_bt_takeover_service_disconnect_edr(addr);

            APPS_LOG_MSGID_I(LOG_TAG"[3_LINK] check_3edr_lea, [%d] LE addr=%08X%04X",
                             3, i, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
            if (i == 0) {
                app_bt_takeover_disconnect_item.remote_bd_type = addr_type;
                memcpy(app_bt_takeover_disconnect_item.remote_addr, addr, BT_BD_ADDR_LEN);
                app_bt_takeover_disconnect_item.link_type = link_type;

                bt_device_manager_link_record_item_t *data = (bt_device_manager_link_record_item_t *)pvPortMalloc(sizeof(bt_device_manager_link_record_item_t));
                if (data == NULL) {
                    return;
                }
                memcpy(data, &app_bt_takeover_disconnect_item, sizeof(bt_device_manager_link_record_item_t));
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_TAKEOVER_ADDR,
                                    data, sizeof(bt_device_manager_link_record_item_t), NULL, 0);
#ifdef MTK_AWS_MCE_ENABLE
                apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_TAKEOVER_ADDR,
                                               &link_record_list[i], sizeof(bt_device_manager_link_record_item_t));
#endif
            } else {
                app_lea_service_disconnect(TRUE, APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT, addr,
                                           BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
            }
            break;
        }
    }
}
#endif

#ifdef MTK_AWS_MCE_ENABLE
static bool app_bt_takeover_service_proc_aws_data(void *extra_data, size_t data_len)
{
    bool ret = FALSE;

    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group;
        uint32_t event_id;
        void *p_extra_data = NULL;
        uint32_t extra_data_len = 0;

        apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &event_id, &p_extra_data, &extra_data_len);

        if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION && event_id == APPS_EVENTS_INTERACTION_LE_TAKEOVER_ADDR) {
            if (p_extra_data && (extra_data_len == (sizeof(bt_device_manager_link_record_item_t)))) {
                bt_device_manager_link_record_item_t *item = (bt_device_manager_link_record_item_t *)p_extra_data;
                const uint8_t *addr = (const uint8_t *)item->remote_addr;
                APPS_LOG_MSGID_I(LOG_TAG" proc_aws_data, addr=%08X%04X type=%d",
                                 3, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), item->link_type);

                memcpy(&app_bt_takeover_disconnect_item, item, sizeof(bt_device_manager_link_record_item_t));
                bt_device_manager_link_record_item_t *data = (bt_device_manager_link_record_item_t *)pvPortMalloc(sizeof(bt_device_manager_link_record_item_t));
                if (data == NULL) {
                    //APPS_LOG_MSGID_E(LOG_TAG" proc_aws_data, disconnect_item malloc fail", 0);
                    return ret;
                }
                memcpy(data, p_extra_data, sizeof(bt_device_manager_link_record_item_t));
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                    EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_TAKEOVER_ADDR,
                                    data, sizeof(bt_device_manager_link_record_item_t), NULL, 0);
            }
            ret = TRUE;
        }
    }
    return ret;
}
#endif



/**================================================================================*/
/**                                BT Callback Function                            */
/**================================================================================*/
void app_bt_takeover_service_run_takeover(bool force_for_sass, bool force_for_3_link)
{
#ifdef AIR_BT_TAKEOVER_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    const bt_device_manager_link_record_t *link_record = bt_device_manager_link_record_get_connected_link();
    uint8_t conn_num = link_record->connected_num;
    bool support_emp = app_bt_conn_mgr_is_support_emp();
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    bt_sink_srv_state_manager_played_device_t *last_played_device = NULL;
#endif

    if ((role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) || conn_num < APP_BT_CONN_MAX_CONN_NUM) {
        APPS_LOG_MSGID_E(LOG_TAG" run_takeover, [%02X] error item=0x%08X connected_num=%d",
                         2, role, conn_num);
        return;
    } else if ((support_emp && conn_num < (APP_BT_CONN_MAX_CONN_NUM + 1)) || (!support_emp && conn_num < APP_BT_CONN_MAX_CONN_NUM)) {
        APPS_LOG_MSGID_E(LOG_TAG" run_takeover, [%02X] error support_emp=%d conn_num=%d",
                         3, role, support_emp, conn_num);
        return;
    }

    bt_device_manager_link_record_item_t *link_record_list = (bt_device_manager_link_record_item_t *)&link_record->connected_device[0];
    app_bt_takeover_item_t link_list[APP_BT_TAKEOVER_MAX_ITEM_SIZE] = {0};
    app_bt_takeover_item_t *link_select = NULL;
    int disconnect_index = APP_BT_TAKEOVER_INVALID_INDEX;
    uint8_t edr_link_num = 0;
#if defined(AIR_BT_TAKEOVER_DONGLE_MUST_TAKEOVER) || defined(AIR_BT_TAKEOVER_EMP_OFF_ALWAYS_TAKEOVER)
    int first_index = conn_num - 1;
#endif

    for (int i = 0; i < conn_num; i++) {
        uint8_t link_type = link_record_list[i].link_type;
        uint8_t addr_type = link_record_list[i].remote_bd_type;
        uint8_t *addr = (uint8_t *)link_record_list[i].remote_addr;
        bool is_dongle = app_bt_conn_mgr_is_dongle(addr);

        link_list[i].link_type = link_type;
        link_list[i].addr_type = addr_type;
        memcpy(link_list[i].addr, addr, BT_BD_ADDR_LEN);
        link_list[i].is_dongle = is_dongle;
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        link_list[i].is_ull2 = app_lea_conn_mgr_is_ull_link(addr);
#endif
        link_list[i].priority = APP_BT_TAKEOVER_PRIORITY_LOW;

        if (link_type == BT_DEVICE_MANAGER_LINK_TYPE_EDR) {
            edr_link_num++;
        }
    }

#ifdef AIR_3_LINK_MULTI_POINT_ENABLE
    if (!force_for_3_link && edr_link_num == APP_BT_CONN_MAX_CONN_NUM + 1) {
        APPS_LOG_MSGID_E(LOG_TAG"[3_LINK] run_takeover, [%02X] ignore 3 LINK EDR case", 1, role);
        app_bt_takeover_service_check_3edr_lea();
        return;
    }

    if (edr_link_num == APP_BT_CONN_MAX_CONN_NUM + 1 + 1) {
        APPS_LOG_MSGID_W(LOG_TAG"[3_LINK] run_takeover, [%02X] 3 LINK EDR takeover %d", 2, role, edr_link_num);
    }
#endif

    // Set new addr and priority
    uint8_t *new_addr = link_list[0].addr;
    link_list[0].priority = APP_BT_TAKEOVER_PRIORITY_MID;

    // Set last_played device as middle priority
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    bt_sink_srv_state_manager_played_device_t played_list[3] = {0};
    uint8_t last_played_link_type = 0xFF;
    uint32_t srv_num = bt_sink_srv_state_manager_get_played_device_list(played_list, 3);
    if (srv_num != 0) {
        uint8_t *addr = (uint8_t *)played_list[0].address;
        last_played_link_type = app_bt_takeover_service_get_link_record_type(played_list[0].type);
        APPS_LOG_MSGID_I(LOG_TAG" run_takeover, last_played=%08X%04X link_type=%d mask=%02X",
                         4, *((uint32_t *)(addr + 2)), *((uint16_t *)addr),
                         last_played_link_type, played_list[0].mask);
        last_played_device = &(played_list[0]);
    }

    if (last_played_device == NULL) {
        // Set 1st/2nd connected device as low priority
        if (conn_num == APP_BT_CONN_MAX_CONN_NUM) {
            link_list[conn_num - 1].priority = APP_BT_TAKEOVER_PRIORITY_LOW;
        } else if (conn_num == APP_BT_CONN_MAX_CONN_NUM + 1) {
#ifdef AIR_BT_TAKEOVER_FIRST_DEVICE
            link_list[1].priority = APP_BT_TAKEOVER_PRIORITY_MID;
            link_list[2].priority = APP_BT_TAKEOVER_PRIORITY_LOW;
#else
            link_list[1].priority = APP_BT_TAKEOVER_PRIORITY_LOW;
            link_list[2].priority = APP_BT_TAKEOVER_PRIORITY_MID;
#endif
        }
    }
#endif

#ifdef AIR_3_LINK_MULTI_POINT_ENABLE
    if (edr_link_num == APP_BT_CONN_MAX_CONN_NUM + 1 + 1) {
        // 3 EDR + 1 takeover
#ifdef AIR_BT_TAKEOVER_FIRST_DEVICE
        link_list[1].priority = APP_BT_TAKEOVER_PRIORITY_MID;
        link_list[2].priority = APP_BT_TAKEOVER_PRIORITY_LOW;
        link_list[3].priority = APP_BT_TAKEOVER_PRIORITY_LOWEST;
#else
        link_list[1].priority = APP_BT_TAKEOVER_PRIORITY_LOWEST;
        link_list[2].priority = APP_BT_TAKEOVER_PRIORITY_LOW;
        link_list[3].priority = APP_BT_TAKEOVER_PRIORITY_MID;
#endif
    } else if (edr_link_num == APP_BT_CONN_MAX_CONN_NUM + 1 && force_for_3_link) {
#ifdef AIR_BT_TAKEOVER_FIRST_DEVICE
        link_list[0].priority = APP_BT_TAKEOVER_PRIORITY_MID;
        link_list[1].priority = APP_BT_TAKEOVER_PRIORITY_LOW;
        link_list[2].priority = APP_BT_TAKEOVER_PRIORITY_LOWEST;
#else
        link_list[0].priority = APP_BT_TAKEOVER_PRIORITY_LOWEST;
        link_list[1].priority = APP_BT_TAKEOVER_PRIORITY_LOW;
        link_list[2].priority = APP_BT_TAKEOVER_PRIORITY_MID;
#endif
    }
#endif

    for (int i = 1; i < conn_num; i++) {
        uint8_t *addr = (uint8_t *)link_list[i].addr;
        if (memcmp(addr, new_addr, sizeof(bt_bd_addr_t)) == 0) {
            APPS_LOG_MSGID_W(LOG_TAG" run_takeover, same address for mode switch - do nothing", 0);
            return;
        }

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
        if (conn_num >= (APP_BT_CONN_MAX_CONN_NUM + 1) && last_played_device != NULL && memcmp(addr, last_played_device->address, sizeof(bt_bd_addr_t)) == 0) {
            link_list[i].priority = APP_BT_TAKEOVER_PRIORITY_MID;
        }
#endif

#ifdef AIR_BT_TAKEOVER_KEEP_DONGLE
        if (link_list[i].is_dongle) {
            link_list[i].priority = APP_BT_TAKEOVER_PRIORITY_HIGHEST;
            APPS_LOG_MSGID_I(LOG_TAG" run_takeover, [%d] addr=%08X%04X set HIGHEST due to Dongle",
                             3, i, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
            continue;
        }
#endif

        for (int user_index = 0; user_index < APP_BT_TAKEOVER_ID_MAX; user_index++) {
            app_bt_takeover_service_allow_func_t user_func = app_bt_takeover_user[user_index];
            if (user_func != NULL && !user_func(link_list[i].addr)) {
                APPS_LOG_MSGID_I(LOG_TAG" run_takeover, [%d] addr=%08X%04X set HIGH by user[%d]",
                                 4, i, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), user_index);
                link_list[i].priority = APP_BT_TAKEOVER_PRIORITY_HIGH;
                break;
            }
        }
    }

    app_bt_takeover_service_print(link_list, conn_num);

#if defined(AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE) && defined(AIR_XIAOAI_AUDIO_SWITCH_ENABLE)
    // Note: check new addr whether MI phone to distinguish normal takeover or MIUI_SASS takeover (EMP OFF)
    // If it is not MIUI Audio Switch case, continue to run normal takeover flow
    // Audio Switch takeover must be two EDR
    if (app_va_xiaoai_is_support_auto_audio_switch() && miui_fc_is_account_key_pairing()
        && conn_num == APP_BT_CONN_MAX_CONN_NUM
        && link_list[0].link_type == BT_DEVICE_MANAGER_LINK_TYPE_EDR
        && link_list[1].link_type == BT_DEVICE_MANAGER_LINK_TYPE_EDR
        && !force_for_sass) {
        APPS_LOG_MSGID_E(LOG_TAG"[MIUI_SASS] takeover_callback, disallow takeover addr=%08X%04X",
                         2, *((uint32_t *)(new_addr + 2)), *((uint16_t *)new_addr));
        memcpy(app_bt_takeover_miui_sass_addr, new_addr, BT_BD_ADDR_LEN);
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_TRIGGER_TAKEOVER);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_TRIGGER_TAKEOVER, NULL, 0, NULL,
                            APP_BT_TAKEOVER_RETRIGGER_TIME2);
        return;
    }
#endif

#ifdef AIR_BT_TAKEOVER_DONGLE_MUST_TAKEOVER
    if (link_list[0].is_dongle) {
        link_list[0].priority = APP_BT_TAKEOVER_PRIORITY_HIGHEST;
        APPS_LOG_MSGID_W(LOG_TAG" run_takeover, DONGLE_MUST_TAKEOVER", 0);
        if (conn_num == APP_BT_CONN_MAX_CONN_NUM + 1) {
            if (link_list[1].priority < link_list[2].priority) {
                disconnect_index = 1;
            } else if (link_list[1].priority > link_list[2].priority) {
                disconnect_index = first_index;
            } else {
                disconnect_index = first_index;   // Takeover first
            }
        } else if (conn_num == APP_BT_CONN_MAX_CONN_NUM) {
            disconnect_index = first_index;
        }
        goto exit;
    }
#endif

#if defined(AIR_BT_TAKEOVER_EMP_OFF_ALWAYS_TAKEOVER) || defined(AIR_BT_TAKEOVER_EMP_OFF_ALWAYS_REJECT_NEW)
    if (!support_emp) {
#ifdef AIR_BT_TAKEOVER_EMP_OFF_ALWAYS_TAKEOVER
        APPS_LOG_MSGID_W(LOG_TAG" run_takeover, EMP off - always disconnect the connected one", 0);
        disconnect_index = first_index;
        goto exit;
#elif defined(AIR_BT_TAKEOVER_EMP_OFF_ALWAYS_REJECT_NEW)
        APPS_LOG_MSGID_W(LOG_TAG" run_takeover, EMP off - always disconnect new link", 0);
        disconnect_index = 0;
        goto exit;
#endif
    }
#endif

    // Select lowest priority to disconnect
    disconnect_index = 0;
    app_bt_takeover_priority_t lowest_priority = link_list[0].priority;
    for (int i = 1; i < conn_num; i++) {
        if (link_list[i].priority < lowest_priority) {
            disconnect_index = i;
            lowest_priority = link_list[i].priority;
        } else if (link_list[0].priority == APP_BT_TAKEOVER_PRIORITY_MID
                   && lowest_priority == APP_BT_TAKEOVER_PRIORITY_MID
                   && link_list[0].priority == link_list[i].priority) {
            // New link and old link are MID priority, disconnect old
            disconnect_index = i;
            lowest_priority = link_list[i].priority;
        }
    }

#if defined(AIR_BT_TAKEOVER_DONGLE_MUST_TAKEOVER) || defined(AIR_BT_TAKEOVER_EMP_OFF_ALWAYS_TAKEOVER) || defined(AIR_BT_TAKEOVER_EMP_OFF_ALWAYS_REJECT_NEW)
exit:
#endif
    if (disconnect_index == APP_BT_TAKEOVER_INVALID_INDEX) {
        APPS_LOG_MSGID_E(LOG_TAG" run_takeover, error link_select", 0);
        return;
    }

    link_select = &link_list[disconnect_index];
    uint8_t *disc_addr = link_list[disconnect_index].addr;
    uint8_t disc_addr_type = link_list[disconnect_index].addr_type;
    APPS_LOG_MSGID_I(LOG_TAG" run_takeover, disconnect [%d] addr=%08X%04X priority=%d",
                     4, disconnect_index, *((uint32_t *)(disc_addr + 2)), *((uint16_t *)disc_addr),
                     link_list[disconnect_index].priority);

    if (link_select->link_type == BT_DEVICE_MANAGER_LINK_TYPE_EDR) {
        app_bt_takeover_service_set_last_device(disc_addr, disc_addr_type, TRUE);
#ifdef AIR_3_LINK_MULTI_POINT_ENABLE
        if (force_for_3_link) {
            APPS_LOG_MSGID_E(LOG_TAG" run_takeover, no disconnect EDR when force_for_3_link", 0);
            return;
        }
#endif
        app_bt_takeover_service_disconnect_edr(disc_addr);
    } else if (link_select->link_type == BT_DEVICE_MANAGER_LINK_TYPE_LE) {
        app_bt_takeover_service_set_last_device(disc_addr, disc_addr_type, FALSE);
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
        if (disconnect_index == 0) {
            APPS_LOG_MSGID_W(LOG_TAG" run_takeover, disconnect new_incoming LE", 0);
            app_bt_takeover_disconnect_item.remote_bd_type = disc_addr_type;
            memcpy(app_bt_takeover_disconnect_item.remote_addr, disc_addr, BT_BD_ADDR_LEN);
            app_bt_takeover_disconnect_item.link_type = link_list[disconnect_index].link_type;

            bt_device_manager_link_record_item_t *data = (bt_device_manager_link_record_item_t *)pvPortMalloc(sizeof(bt_device_manager_link_record_item_t));
            if (data == NULL) {
                APPS_LOG_MSGID_E(LOG_TAG" run_takeover, disconnect_item malloc fail", 0);
                return;
            }
            memcpy(data, &app_bt_takeover_disconnect_item, sizeof(bt_device_manager_link_record_item_t));
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_TAKEOVER_ADDR,
                                data, sizeof(bt_device_manager_link_record_item_t), NULL, 0);
#ifdef MTK_AWS_MCE_ENABLE
            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_TAKEOVER_ADDR,
                                           link_select, sizeof(bt_device_manager_link_record_item_t));
#endif
        } else {
            app_lea_service_disconnect(TRUE, APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT, disc_addr,
                                       BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
        }
#endif
    }

#else
    // No AIR_BT_TAKEOVER_ENABLE
    APPS_LOG_MSGID_E(LOG_TAG" run_takeover, No AIR_BT_TAKEOVER_ENABLE", 0);
#endif
}

void bt_device_manager_link_record_takeover_callback(const bt_device_manager_link_record_item_t *item)
{
    app_bt_takeover_service_run_takeover(FALSE, FALSE);
}

bt_bd_addr_t *bt_cm_get_disconnect_addr_before_rho(void)
{
    bool is_support_emp = app_bt_conn_mgr_is_support_emp();
    uint8_t edr_num = app_bt_conn_mgr_get_edr_num();
    if (is_support_emp && edr_num == APP_BT_CONN_MAX_CONN_NUM + 1) {
        app_bt_takeover_service_run_takeover(FALSE, TRUE);
        return (bt_bd_addr_t *)app_bt_takeover_last_device.addr;
    }
    return NULL;
}

#if defined(AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE) && defined(AIR_XIAOAI_AUDIO_SWITCH_ENABLE)
static void app_bt_takeover_sass_force_trigger(void)
{
    const bt_device_manager_link_record_t *link_record = bt_device_manager_link_record_get_connected_link();
    uint8_t conn_num = link_record->connected_num;
    bool is_support_emp = app_bt_conn_mgr_is_support_emp();

    if (!is_support_emp && conn_num >= APP_BT_CONN_MAX_CONN_NUM) {
        APPS_LOG_MSGID_W(LOG_TAG"[MIUI_SASS] sass_force_trigger, emp=%d conn_num=%d", 2, is_support_emp, conn_num);
        app_bt_takeover_service_run_takeover(TRUE, FALSE);
    }
}
#endif



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
bool app_bt_takeover_service_user_register(app_bt_takeover_user_id_t user_id, app_bt_takeover_service_allow_func_t func)
{
    bool ret = FALSE;
    if (user_id < APP_BT_TAKEOVER_ID_MAX) {
        app_bt_takeover_user[user_id] = func;
        ret = TRUE;
    }
    APPS_LOG_MSGID_I(LOG_TAG" register, user_id=%d ret=%d", 2, user_id, ret);
    return ret;
}

bool app_bt_takeover_service_user_deregister(app_bt_takeover_user_id_t user_id)
{
    bool ret = FALSE;
    if (user_id < APP_BT_TAKEOVER_ID_MAX) {
        app_bt_takeover_user[user_id] = NULL;
        ret = TRUE;
    }
    APPS_LOG_MSGID_I(LOG_TAG" deregister, user_id=%d ret=%d", 2, user_id, ret);
    return ret;
}

void app_bt_takeover_service_init()
{
    app_bt_takeover_service_user_register(APP_BT_TAKEOVER_ID_BTSINK, app_bt_takeover_service_bt_sink_allow_cb);
}

bool app_bt_takeover_service_get_last_takeover_device(app_bt_takeover_device_t *device)
{
    bool success = FALSE;
    uint8_t *addr = app_bt_takeover_last_device.addr;
    uint8_t empty_addr[BT_BD_ADDR_LEN] = {0};
    if (device != NULL && memcmp(addr, empty_addr, BT_BD_ADDR_LEN) != 0) {
        memcpy(device, &app_bt_takeover_last_device, sizeof(app_bt_takeover_device_t));
        success = TRUE;
    }
    return success;
}

void app_bt_takeover_service_disconnect_one(void)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bool is_support_emp = app_bt_conn_mgr_is_support_emp();
    uint8_t edr_num = app_bt_conn_mgr_get_edr_num();
    APPS_LOG_MSGID_W(LOG_TAG" disconnect_one, [%02X] is_support_emp=%d edr_num=%d",
                     3, role, is_support_emp, edr_num);

#ifdef AIR_3_LINK_MULTI_POINT_ENABLE
    if (is_support_emp && edr_num == APP_BT_CONN_MAX_CONN_NUM + 1) {
        app_bt_takeover_service_run_takeover(FALSE, TRUE);
    } else {
        app_bt_takeover_service_run_takeover(FALSE, FALSE);
    }
#else
    app_bt_takeover_service_run_takeover(FALSE, FALSE);
#endif
}

uint8_t *app_bt_takeover_get_disconnect_le_addr()
{
    bt_bd_addr_t zero_addr = {0};
    if (app_bt_takeover_disconnect_item.link_type != BT_DEVICE_MANAGER_LINK_TYPE_LE
        || memcmp(&(app_bt_takeover_disconnect_item.remote_addr), zero_addr, sizeof(bt_bd_addr_t)) == 0) {
        return NULL;
    }
    uint8_t *addr = (uint8_t *)(app_bt_takeover_disconnect_item.remote_addr);
    APPS_LOG_MSGID_I(LOG_TAG" get_disconnect_le_addr, addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     6, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    return addr;
}

void app_bt_takeover_clear_disconnect_le_addr()
{
    APPS_LOG_MSGID_I(LOG_TAG" clear_disconnect_le_addr", 0);
    memset(&app_bt_takeover_disconnect_item, 0, sizeof(bt_device_manager_link_record_item_t));
}

void app_bt_takeover_clear_miui_sass_ctx(void)
{
#if defined(AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE) && defined(AIR_XIAOAI_AUDIO_SWITCH_ENABLE)
    APPS_LOG_MSGID_I(LOG_TAG"[MIUI_SASS] clear_miui_sass_ctx", 0);
    memset(app_bt_takeover_miui_sass_addr, 0, BT_BD_ADDR_LEN);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_TRIGGER_TAKEOVER);
#endif
}

void app_bt_takeover_handle_pc_edr(void)
{
#if defined(AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE) && defined(AIR_XIAOAI_AUDIO_SWITCH_ENABLE)
    bt_device_manager_link_record_t *link_record = (bt_device_manager_link_record_t *)bt_device_manager_link_record_get_connected_link();
    uint8_t conn_num = link_record->connected_num;
    bool is_account_key_pairing = miui_fc_is_account_key_pairing();

    if (app_va_xiaoai_is_support_auto_audio_switch() && is_account_key_pairing
        && conn_num == 2 && link_record->connected_device[1].link_type == BT_DEVICE_MANAGER_LINK_TYPE_EDR) {
        uint8_t *addr = link_record->connected_device[1].remote_addr;
        APPS_LOG_MSGID_E(LOG_TAG"[MIUI_SASS] takeover_callback, PC disconnect addr=%08X%04X",
                         2, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));

        app_bt_takeover_clear_miui_sass_ctx();
        app_bt_takeover_service_disconnect_edr((const uint8_t *)addr);
    }
#endif
}

void app_bt_takeover_proc_ui_shell_event(uint32_t event_group, uint32_t event_id, void *extra_data, size_t data_len)
{
#ifdef MTK_AWS_MCE_ENABLE
    if (event_group == EVENT_GROUP_UI_SHELL_AWS_DATA) {
        app_bt_takeover_service_proc_aws_data(extra_data, data_len);
    }
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
    if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
        && event_id == APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT) {
        app_rho_result_t rho_result = (app_rho_result_t)extra_data;
        //APPS_LOG_MSGID_I(LOG_TAG" [new Agent] RHO done - %d", 1, rho_result);
        if (APP_RHO_RESULT_SUCCESS == rho_result) {
            app_bt_takeover_service_run_takeover(FALSE, FALSE);
        }
    }
#endif
#endif

#if defined(AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE) && defined(AIR_XIAOAI_AUDIO_SWITCH_ENABLE)
    if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
        && event_id == APPS_EVENTS_INTERACTION_TRIGGER_TAKEOVER) {
        APPS_LOG_MSGID_W(LOG_TAG"[MIUI_SASS] trigger takeover", 0);
        memset(app_bt_takeover_miui_sass_addr, 0, BT_BD_ADDR_LEN);
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_TRIGGER_TAKEOVER);
        app_bt_takeover_sass_force_trigger();
    }

    if (event_group == EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER
        && event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE) {
        bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
        if (remote_update != NULL) {
            uint8_t *addr = (uint8_t *)remote_update->address;
            bool hfp_connected = (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->pre_connected_service)
                                  && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->connected_service));
            if (hfp_connected && memcmp(app_bt_takeover_miui_sass_addr, addr, BT_BD_ADDR_LEN) == 0) {
                APPS_LOG_MSGID_I(LOG_TAG"[MIUI_SASS] new addr connected HFP addr=%08X%04X",
                                 2, *((uint32_t *)(addr + 2)), *((uint16_t *)addr));
                memset(app_bt_takeover_miui_sass_addr, 0, BT_BD_ADDR_LEN);
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_TRIGGER_TAKEOVER);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_TRIGGER_TAKEOVER, NULL, 0, NULL,
                                    APP_BT_TAKEOVER_RETRIGGER_TIME1);
            }
        }
    }
#endif
}
