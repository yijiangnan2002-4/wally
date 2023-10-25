
/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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
 * File: app_le_audio_hfp_at_mcd.c
 *
 * Description: This file provides HFP AT CMD for LE Audio or other feature.
 *
 */

#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_LE_AUDIO_MTK_HFP_AT_CMD)

#include "app_le_audio_hfp_at_cmd.h"

#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "bt_app_common.h"
#include "bt_callback_manager.h"
#include "bt_connection_manager.h"
#include "bt_device_manager.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_ami.h"
#include "bt_sink_srv_hf.h"
#include "ui_shell_manager.h"
#include "FreeRTOS.h"

#include "app_le_audio.h"
#include "app_lea_service.h"
#include "app_lea_service_conn_mgr.h"
#include "app_lea_service_event.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#include "apps_aws_sync_event.h"
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
#include "app_rho_idle_activity.h"
#endif
#endif
#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#endif
#ifdef AIR_SMART_CHARGER_ENABLE
#include "app_smcharger.h"
#endif

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

#define LOG_TAG           "[LEA][HF]"

#define APP_LE_AUDIO_HFP_ATCMD            "+MTK="
#define APP_LE_AUDIO_HFP_ATCMD_HEADER     "FFFAFB"
#define APP_LE_AUDIO_HFP_ATCMD_LEN        (strlen(APP_LE_AUDIO_HFP_ATCMD))
#define APP_LE_AUDIO_HFP_HEADER_LEN       (strlen(APP_LE_AUDIO_HFP_ATCMD_HEADER)) // <FF><FA><FB>

#define APP_LE_AUDIO_BATTERY_TIME         (10 * 60 * 1000)

#define APP_LE_AUDIO_ATCMD_MAX_LEN        200

#define APP_LE_AUDIO_MAX_HFP_NUM          3

typedef enum {
    APP_LEA_HFP_STATE_DISCONNECTED        = 0,
    APP_LEA_HFP_STATE_CONNECTED,
    APP_LEA_HFP_STATE_ENABLE_MTK_BATTERY,
} app_lea_hfp_state_t;

typedef struct {
    bool                 used;
    uint8_t              addr[BT_BD_ADDR_LEN];
    uint8_t              state;
} PACKED app_lea_hfp_ctx_t;

static app_lea_hfp_ctx_t                  app_lea_hfp_ctx[APP_LE_AUDIO_MAX_HFP_NUM] = {0};

typedef enum {
    LE_AUDIO_HFP_ATCMD_TYPE_SWITCH = 0,
    LE_AUDIO_HFP_ATCMD_TYPE_BATTERY = 1,
    LE_AUDIO_HFP_ATCMD_TYPE_BATTERY_CONTROL = 2,
} le_audio_hfp_atcmd_type;



/**================================================================================*/
/**                                  Internal API                                  */
/**================================================================================*/
static void app_le_audio_hfp_printf_ctx(void)
{
    for (int i = 0; i < APP_LE_AUDIO_MAX_HFP_NUM; i++) {
        bool used = app_lea_hfp_ctx[i].used;
        uint8_t *addr = app_lea_hfp_ctx[i].addr;
        uint8_t state = app_lea_hfp_ctx[i].state;
        APPS_LOG_MSGID_I(LOG_TAG" printf_ctx, [%d] used=%d addr=%08X%04X state=%d",
                         5, i, used, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), state);
    }
}

static void app_lea_audio_hfp_sync_context(void)
{
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (bt_aws_mce_srv_get_link_type() == BT_AWS_MCE_SRV_LINK_NONE) {
        return;
    }

    bt_status_t bt_status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                                           EVENT_ID_LEA_SYNC_HFP_CONTEXT,
                                                           &app_lea_hfp_ctx,
                                                           sizeof(app_lea_hfp_ctx_t) * APP_LE_AUDIO_MAX_HFP_NUM);
    APPS_LOG_MSGID_I(LOG_TAG" sync_context, [%02X] bt_status=0x%08X", 2, role, bt_status);
#endif
}

#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
static void app_lea_audio_hfp_handle_aws_data(void *extra_data)
{
    uint32_t aws_event_group = 0;
    uint32_t aws_event_id = 0;
    void *p_extra_data = NULL;
    uint32_t extra_data_len = 0;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
    if (aws_data_ind == NULL || aws_data_ind->module_id != BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        return;
    }

    apps_aws_sync_event_decode_extra(aws_data_ind, &aws_event_group, &aws_event_id,
                                     &p_extra_data, &extra_data_len);
    if (aws_event_group == EVENT_GROUP_UI_SHELL_LE_AUDIO
        && aws_event_id == EVENT_ID_LEA_SYNC_HFP_CONTEXT) {
        app_lea_hfp_ctx_t *ctx = (app_lea_hfp_ctx_t *)p_extra_data;
        memcpy(&app_lea_hfp_ctx, ctx, sizeof(app_lea_hfp_ctx_t) * APP_LE_AUDIO_MAX_HFP_NUM);
        APPS_LOG_MSGID_I(LOG_TAG" AWS_DATA, [%02X] hfp_ctx", 1, role);
        app_le_audio_hfp_printf_ctx();
    }
}
#endif

static bool app_le_audio_hfp_is_enable_battery(void)
{
    bool is_enable = FALSE;
    for (int i = 0; i < APP_LE_AUDIO_MAX_HFP_NUM; i++) {
        uint8_t state = app_lea_hfp_ctx[i].state;
        if (state == APP_LEA_HFP_STATE_ENABLE_MTK_BATTERY) {
            is_enable = TRUE;
            break;
        }
    }
    return is_enable;
}

static void app_le_audio_hfp_update_battery_timer(void)
{
    bool enable_mtk_battery = app_le_audio_hfp_is_enable_battery();
    uint32_t hfp_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP), NULL, 0);
    APPS_LOG_MSGID_I(LOG_TAG" start_battery_timer, enable_mtk_battery=%d hfp_num=%d", 2, enable_mtk_battery, hfp_num);
    if (enable_mtk_battery && hfp_num > 0) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_NOTIFY_BATTERY);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                            EVENT_ID_LE_AUDIO_NOTIFY_BATTERY, NULL, 0, NULL, APP_LE_AUDIO_BATTERY_TIME);
    } else {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_NOTIFY_BATTERY);
    }
}

static void app_le_audio_hfp_connect_change(bool connect, uint8_t *addr)
{
    for (int i = 0; i < APP_LE_AUDIO_MAX_HFP_NUM; i++) {
        if (connect && memcmp(app_lea_hfp_ctx[i].addr, addr, BT_BD_ADDR_LEN) == 0
            && app_lea_hfp_ctx[i].state >= APP_LEA_HFP_STATE_CONNECTED) {
            APPS_LOG_MSGID_W(LOG_TAG" hfp_connect_change, duplicate", 0);
            break;
        }

        if (connect && !app_lea_hfp_ctx[i].used) {
            app_lea_hfp_ctx[i].used = TRUE;
            memcpy(app_lea_hfp_ctx[i].addr, addr, BT_BD_ADDR_LEN);
            app_lea_hfp_ctx[i].state = APP_LEA_HFP_STATE_CONNECTED;
            break;
        } else if (!connect && app_lea_hfp_ctx[i].used && memcmp(app_lea_hfp_ctx[i].addr, addr, BT_BD_ADDR_LEN) == 0) {
            memset(&app_lea_hfp_ctx[i], 0, sizeof(app_lea_hfp_ctx_t));
            break;
        }
    }

    app_le_audio_hfp_printf_ctx();
    app_le_audio_hfp_update_battery_timer();
    app_lea_audio_hfp_sync_context();
}

static void app_le_audio_hfp_handle_bt_cm_event(void *extra_data)
{
    bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
    if (remote_update == NULL) {
        return;
    }

    if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->pre_connected_service)
        && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->connected_service)) {
        app_le_audio_hfp_connect_change(TRUE, remote_update->address);
    } else if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->pre_connected_service)
               && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->connected_service)) {
        app_le_audio_hfp_connect_change(FALSE, remote_update->address);
    }

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
        && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)
        && role == BT_AWS_MCE_ROLE_AGENT) {
        app_lea_audio_hfp_sync_context();
    }
#endif
}

static bool app_le_audio_hfp_send_hfp_atcmd(uint8_t *addr, uint8_t type, uint8_t *data, uint8_t data_len)
{
    bool ret = FALSE;
    char *atcmd = NULL;
    char *param = NULL;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    APPS_LOG_MSGID_I(LOG_TAG" send_hfp_atcmd [%02X] addr=%08X%04X type=%d data=0x%08X data_len=%d",
                     6, role, *((uint32_t *)(addr + 2)), *((uint16_t *)addr), type, data, data_len);

    if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
        //APPS_LOG_MSGID_E(LOG_TAG" send_hfp_atcmd, role fail", 0);
        goto exit;
    }

    if ((type != LE_AUDIO_HFP_ATCMD_TYPE_SWITCH && type != LE_AUDIO_HFP_ATCMD_TYPE_BATTERY)
        || data == NULL || data_len == 0) {
        //APPS_LOG_MSGID_E(LOG_TAG" send_hfp_atcmd, parameter fail", 0);
        goto exit;
    }

    atcmd = (char *)pvPortMalloc(APP_LE_AUDIO_ATCMD_MAX_LEN);
    param = (char *)pvPortMalloc(data_len * 2 + 1);
    if (atcmd == NULL || param == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" send_hfp_atcmd, malloc fail", 0);
        goto exit;
    }
    memset(atcmd, 0, APP_LE_AUDIO_ATCMD_MAX_LEN);
    memset(param, 0, data_len * 2 + 1);

    for (int i = 0; i < data_len; i++) {
        char temp[3] = {0};
        snprintf(temp, 3, "%02X", data[i]);
        param[i * 2] = temp[0];
        param[i * 2 + 1] = temp[1];
    }

    snprintf(atcmd, APP_LE_AUDIO_ATCMD_MAX_LEN, "FFFAFB%02X%02X%sFF", type, data_len, param);

    int atcmd_len = strlen(atcmd);
    //APPS_LOG_I("[LEA][HF] HFP ATCMD=%s atcmd_len=%d\r\n", (char *)atcmd, atcmd_len);

    if (atcmd != NULL && atcmd_len > APP_LE_AUDIO_HFP_HEADER_LEN) {
        bt_status_t status = bt_sink_srv_hf_mtk_custom_ext((bt_bd_addr_t *)addr, atcmd, atcmd_len);
        ret = (status == BT_STATUS_SUCCESS);
    }

exit:
    if (atcmd != NULL) {
        vPortFree(atcmd);
    }
    if (param != NULL) {
        vPortFree(param);
    }
    return ret;
}

static void app_le_audio_hfp_switch(uint8_t *addr, const char *cmd)
{
    int len = 0;
    int type = 0;
    int value = 0;
    int n = sscanf(cmd, "%02x%02x%02x", &type, &len, &value);
    APPS_LOG_MSGID_I(LOG_TAG" switch, type=%d len=%d value=%d", 3, type, len, value);

    if (n == 3 && type == LE_AUDIO_HFP_ATCMD_TYPE_SWITCH && len == 1 && (value == 0 || value == 1)) {
        bool enable = (value == 1);
        if (enable && !app_lea_service_is_enable_dual_mode()) {
            // Note: For dual mode + earbuds DHSS, no need to start ADV with general mode since add bond info when EDR ACL connected
            app_lea_service_start_advertising(APP_LEA_ADV_MODE_GENERAL, TRUE, APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME);
        }
        app_le_audio_hfp_send_hfp_atcmd(addr, LE_AUDIO_HFP_ATCMD_TYPE_SWITCH, (uint8_t *)&value, 1);
    }
}

static void app_le_audio_hfp_enable_battery(uint8_t *addr, bool enable)
{
    bool match_update = FALSE;
    for (int i = 0; i < APP_LE_AUDIO_MAX_HFP_NUM; i++) {
        if (memcmp(app_lea_hfp_ctx[i].addr, addr, BT_BD_ADDR_LEN) == 0) {
            if (enable) {
                app_lea_hfp_ctx[i].state = APP_LEA_HFP_STATE_ENABLE_MTK_BATTERY;
                app_le_audio_hfp_send_battery_info();
            } else {
                app_lea_hfp_ctx[i].state = APP_LEA_HFP_STATE_CONNECTED;
            }
            match_update = TRUE;
            break;
        }
    }

    // HFP AT CMD first, then HFP profile connected on UI Shell task
    if (!match_update) {
        for (int i = 0; i < APP_LE_AUDIO_MAX_HFP_NUM; i++) {
            if (!app_lea_hfp_ctx[i].used) {
                app_lea_hfp_ctx[i].used = TRUE;
                memcpy(app_lea_hfp_ctx[i].addr, addr, BT_BD_ADDR_LEN);
                app_lea_hfp_ctx[i].state = APP_LEA_HFP_STATE_ENABLE_MTK_BATTERY;
                app_le_audio_hfp_send_battery_info();
                match_update = TRUE;
                break;
            }
        }
    }

    if (match_update) {
        app_lea_audio_hfp_sync_context();
        app_le_audio_hfp_printf_ctx();
        app_le_audio_hfp_update_battery_timer();
    }
}

static void app_le_audio_hfp_handle_enable_battery_cmd(uint8_t *addr, const char *cmd)
{
    int len = 0;
    int type = 0;
    int value = 0;
    int n = sscanf(cmd, "%02x%02x%02x", &type, &len, &value);
    APPS_LOG_MSGID_I(LOG_TAG" handle_enable_battery_cmd, type=%d len=%d value=%d", 3, type, len, value);

    if (n == 3 && type == LE_AUDIO_HFP_ATCMD_TYPE_BATTERY_CONTROL && len == 1 && (value == 0 || value == 1)) {
        app_le_audio_hfp_send_hfp_atcmd(addr, LE_AUDIO_HFP_ATCMD_TYPE_BATTERY_CONTROL, (uint8_t *)&value, 1);

        app_le_audio_hfp_enable_battery(addr, (value == 1));
    }
}

static bt_status_t app_le_audio_hfp_callback(bt_msg_type_t event, bt_status_t status, void *param)
{
    switch (event) {
        case BT_HFP_CUSTOM_COMMAND_RESULT_IND: {
            bt_hfp_custom_command_result_ind_t *ind = (bt_hfp_custom_command_result_ind_t *)param;
            const char *atcmd = ind->result;
            // +MTK:<FF><FA><FB><T><L>"V"<FF>
            if (status == BT_STATUS_SUCCESS && atcmd != NULL) {
                int atcmd_len = strlen(atcmd) - APP_LE_AUDIO_HFP_ATCMD_LEN - APP_LE_AUDIO_HFP_HEADER_LEN;
                if (atcmd_len > 0
                    && strstr(atcmd, APP_LE_AUDIO_HFP_ATCMD) > 0
                    && strstr(atcmd, APP_LE_AUDIO_HFP_ATCMD_HEADER) > 0) {
                    atcmd += APP_LE_AUDIO_HFP_ATCMD_LEN + APP_LE_AUDIO_HFP_HEADER_LEN;
                    int type = 0;
                    int len = 0;
                    sscanf(atcmd, "%02x%02x", &type, &len);
                    // Check suffix
                    const char *suffix = atcmd + 2 + 2 + len * 2;
                    // HFP handle -> addr
                    bt_bd_addr_t *bt_bd_addr = bt_hfp_get_bd_addr_by_handle(ind->handle);
                    uint8_t *addr = (uint8_t *)bt_bd_addr;
                    APPS_LOG_MSGID_I(LOG_TAG" hfp_callback, addr=%08X%04X type=%d len=%d suffix=%02X:%02X",
                                     6, *((uint32_t *)(addr + 2)), *((uint16_t *)addr),
                                     type, len, suffix[0], suffix[1]);
                    if (suffix[0] != 'F' || suffix[1] != 'F') {
                        //APPS_LOG_MSGID_E(LOG_TAG" hfp_callback, invalid suffix", 0);
                        break;
                    }

                    if (type == LE_AUDIO_HFP_ATCMD_TYPE_SWITCH) {
                        app_le_audio_hfp_switch(addr, atcmd);
                    } else if (type == LE_AUDIO_HFP_ATCMD_TYPE_BATTERY_CONTROL) {
                        app_le_audio_hfp_handle_enable_battery_cmd(addr, atcmd);
                    }
                }
            }
            break;
        }
    }
    return BT_STATUS_SUCCESS;
}



/**================================================================================*/
/**                                  PUBLIC API                                    */
/**================================================================================*/
void app_le_audio_hfp_at_cmd_register(bool enable)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (enable) {
        status = bt_callback_manager_register_callback(bt_callback_type_app_event,
                                                       MODULE_MASK_HFP, (void *)app_le_audio_hfp_callback);
    } else {
        status = bt_callback_manager_deregister_callback(bt_callback_type_app_event,
                                                         (void *)app_le_audio_hfp_callback);
    }
    memset(&app_lea_hfp_ctx[0], 0, sizeof(app_lea_hfp_ctx_t) * APP_LE_AUDIO_MAX_HFP_NUM);
    APPS_LOG_MSGID_I(LOG_TAG" register HFP enable=%d status=0x%08X", 2, enable, status);
}

void app_le_audio_hfp_handle_event(uint32_t event_group, uint32_t event_id,
                                   void *extra_data, size_t data_len)
{
    if (event_group == EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER && event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE) {
        app_le_audio_hfp_handle_bt_cm_event(extra_data);
    }
    if (event_group == EVENT_GROUP_UI_SHELL_LE_AUDIO && event_id == EVENT_ID_LE_AUDIO_NOTIFY_BATTERY) {
        app_le_audio_hfp_send_battery_info();
    }
#ifdef AIR_SMART_CHARGER_ENABLE
    if (event_group == EVENT_GROUP_UI_SHELL_CHARGER_CASE && event_id == SMCHARGER_EVENT_NOTIFY_BOTH_CHANGED) {
        app_le_audio_hfp_send_battery_info();
    }
#endif
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
    if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION && event_id == APPS_EVENTS_INTERACTION_RHO_END) {
        app_rho_result_t rho_ret = (app_rho_result_t)extra_data;
        if (APP_RHO_RESULT_SUCCESS == rho_ret) {
            // New partner sync to new Agent (old partner)
            app_lea_audio_hfp_sync_context();
        }
    }
    if (event_group == EVENT_GROUP_UI_SHELL_AWS_DATA) {
        app_lea_audio_hfp_handle_aws_data(extra_data);
    }
#endif
}

void app_le_audio_hfp_send_battery_info()
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
        return;
    }

    uint32_t hfp_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP), NULL, 0);
    if (hfp_num > 0 && app_le_audio_hfp_is_enable_battery()) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_NOTIFY_BATTERY);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                            EVENT_ID_LE_AUDIO_NOTIFY_BATTERY, NULL, 0, NULL, APP_LE_AUDIO_BATTERY_TIME);
    } else if (hfp_num == 0) {
        //APPS_LOG_MSGID_I(LOG_TAG" hfp_send_battery_info, stop timer", 0);
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_NOTIFY_BATTERY);
        return;
    }

    uint8_t report_data[3] = {0};
#ifdef AIR_SMART_CHARGER_ENABLE
    uint8_t own_battery = 0;
    uint8_t peer_battery = 0;
    uint8_t case_battery = 0;
    app_smcharger_get_battery_percent(&own_battery, &peer_battery, &case_battery);
    if (own_battery != 0xFF && app_smcharger_is_charging() == APP_SMCHARGER_IN) {
        own_battery |= 0x80;
    }
    if (peer_battery != 0xFF && app_smcharger_peer_is_charging() == APP_SMCHARGER_IN) {
        peer_battery |= 0x80;
    }

    audio_channel_t channel = ami_get_audio_channel();
    if (AUDIO_CHANNEL_L == channel) {
        report_data[0] = own_battery;
        report_data[1] = peer_battery;
    } else {
        report_data[0] = peer_battery;
        report_data[1] = own_battery;
    }
    report_data[2] = case_battery;
#endif

    for (int i = 0; i < APP_LE_AUDIO_MAX_HFP_NUM; i++) {
        bool used = app_lea_hfp_ctx[i].used;
        uint8_t *addr = app_lea_hfp_ctx[i].addr;
        uint8_t state = app_lea_hfp_ctx[i].state;
        if (used && state == APP_LEA_HFP_STATE_ENABLE_MTK_BATTERY) {
            app_le_audio_hfp_send_hfp_atcmd(addr, LE_AUDIO_HFP_ATCMD_TYPE_BATTERY, report_data, 3);
        }
    }
}

#endif /* AIR_LE_AUDIO_ENABLE */
