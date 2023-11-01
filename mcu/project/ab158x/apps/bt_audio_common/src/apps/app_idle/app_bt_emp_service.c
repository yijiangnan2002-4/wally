
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
 * File: app_bt_emp_service.c
 *
 * Description: This file provides BT EMP switch API.
 *
 */

#ifdef AIR_MULTI_POINT_ENABLE

#include "app_bt_emp_service.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#include "bt_aws_mce_report.h"
#endif
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_customer_config.h"
#include "bt_device_manager.h"
#include "bt_device_manager_link_record.h"
#include "FreeRTOS.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "ui_shell_manager.h"

#ifdef AIR_XIAOAI_ENABLE
#include "xiaoai.h"
#endif
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "app_lea_service.h"
#endif
#ifdef AIR_APP_A2DP_LBB_VENDOR_CODEC_LIMIT
#include "bt_sink_srv_a2dp.h"
#endif



/**================================================================================*/
/**                              Definition & Structure                            */
/**================================================================================*/
#define LOG_TAG     "[EMP]"

static void app_bt_emp_srv_init();
static void app_bt_emp_srv_update_nvkey(bool enable);
#ifdef MTK_AWS_MCE_ENABLE
static bool app_bt_emp_srv_sync_to_peer(uint8_t enable);
#endif

#define APP_BT_EMP_MAX_CONN_NUM     2
#define APP_BT_EMP_MIN_CONN_NUM     1

static app_bt_emp_switch_allow_cb_t app_bt_emp_srv_user_list[APP_BT_EMP_SRV_USER_ID_MAX] = {0};
static bool                         app_bt_emp_enable_flag = FALSE;



/**================================================================================*/
/**                                 Internal Function                              */
/**================================================================================*/
static uint8_t app_bt_emp_srv_get_bt_cm_num(bool enable)
{
    uint8_t bt_cm_num = 0;
#ifdef MTK_AWS_MCE_ENABLE
    bt_cm_num += 1 + 1; // one EDR + AWS
#else
    bt_cm_num += 1;     // only one EDR
#endif
    const bt_cm_config_t *bt_cm_config = bt_customer_config_get_cm_config();
    if (bt_cm_config->connection_takeover) {
        bt_cm_num += 1;
    }
    if (enable) {
        bt_cm_num += 1;
    }
    return bt_cm_num;
}

bool app_bt_emp_srv_activity_proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            //APPS_LOG_MSGID_I(LOG_TAG" CREATE", 0);
            app_bt_emp_srv_init();
        }
        break;
    }
    return TRUE;
}

#ifdef MTK_AWS_MCE_ENABLE
static bool app_bt_emp_srv_partner_enable(bool enable)
{
    if (app_bt_emp_enable_flag == enable) {
        //APPS_LOG_MSGID_I(LOG_TAG" partner_enable fail, already enable=%d", 1, enable);
        return TRUE;
    }

    uint8_t bt_cm_num = app_bt_emp_srv_get_bt_cm_num(enable);
    bt_bd_addr_t keep_phone_addr[1] = {{0}};
    bt_status_t status = bt_cm_set_max_connection_number(bt_cm_num, keep_phone_addr, 0, FALSE);
    APPS_LOG_MSGID_I(LOG_TAG" partner AWS Data, enable=%d bt_cm_num=%d status=0x%08X",
                     3, enable, bt_cm_num, status);
    if (status == BT_STATUS_SUCCESS) {
        app_bt_emp_srv_update_nvkey(enable);
    }
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    app_lea_service_enable_multi_conn(enable);
#endif
    bt_device_manager_link_record_set_max_num(enable ? APP_BT_EMP_MAX_CONN_NUM : APP_BT_EMP_MIN_CONN_NUM);
    return TRUE;
}

bool app_bt_emp_srv_activity_proc_aws_data(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (role == BT_AWS_MCE_ROLE_PARTNER || role == BT_AWS_MCE_ROLE_CLINET) {
        if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_EMP) {
            configASSERT(aws_data_ind->param_len == sizeof(uint8_t));
            uint8_t *enable = aws_data_ind->param;
            app_bt_emp_srv_partner_enable((bool)*enable);
        }
    }
    return FALSE;
}

static bool app_bt_emp_srv_sync_to_peer(uint8_t enable)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (bt_aws_mce_srv_get_link_type() != BT_AWS_MCE_SRV_LINK_NONE) {
        uint8_t info_array[sizeof(bt_aws_mce_report_info_t)] = {0};
        uint8_t data_array[sizeof(uint8_t)] = {0};
        bt_aws_mce_report_info_t *aws_data = (bt_aws_mce_report_info_t *)&info_array;
        aws_data->module_id = BT_AWS_MCE_REPORT_MODULE_APP_EMP;
        aws_data->is_sync = FALSE;
        aws_data->sync_time = 0;
        aws_data->param_len = sizeof(uint8_t);
        memcpy((uint8_t *)data_array, &enable, sizeof(uint8_t));
        aws_data->param = (void *)data_array;
        bt_status_t status = bt_aws_mce_report_send_event(aws_data);
        APPS_LOG_MSGID_I(LOG_TAG" [%02X] sync_to_peer, enable=%d status=0x%08X",
                         3, role, enable, status);
        return (status == BT_STATUS_SUCCESS);
    } else {
        APPS_LOG_MSGID_I(LOG_TAG" [%02X] sync_to_peer, fail", 1, role);
        return FALSE;
    }
}

#endif

static void app_bt_emp_srv_update_nvkey(bool enable)
{
    uint32_t size = sizeof(uint8_t);
    nvkey_status_t status = nvkey_write_data(NVID_APP_MULTI_POINT_ENABLE, (const uint8_t *)&enable, size);
    APPS_LOG_MSGID_I(LOG_TAG" app_bt_emp_srv update_nvkey, enable=%d status=%d",
                     2, enable, status);
    if (status == NVKEY_STATUS_OK) {
        app_bt_emp_enable_flag = enable;
    }
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_MULTI_POINT_STATE_CHANGED, NULL, 0, NULL, 0);
}

#ifdef AIR_XIAOAI_ENABLE
static bool app_bt_emp_disconnect_edr(const uint8_t *keep_addr)
{
    if (keep_addr == NULL) {
        APPS_LOG_MSGID_I(LOG_TAG" disconnect_edr, keep addr=NULL", 0);
    } else {
        APPS_LOG_MSGID_I(LOG_TAG" disconnect_edr, keep addr=%02X:%02X:%02X:%02X:%02X:%02X",
                         6, keep_addr[5], keep_addr[4], keep_addr[3], keep_addr[2], keep_addr[1], keep_addr[0]);
    }

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (role != BT_AWS_MCE_ROLE_AGENT) {
        APPS_LOG_MSGID_E(LOG_TAG" disconnect_edr, not agent %02X", 1, role);
        return FALSE;
    }
#endif

    bt_bd_addr_t device_list[3];
    uint32_t device_count = 3;
    device_count = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP),
                                               device_list, device_count);
    for (int i = 0; i < device_count; i++) {
        if (keep_addr == NULL || memcmp(keep_addr, device_list[i], BT_BD_ADDR_LEN) != 0) {
            bt_cm_connect_t connect_param = {{0}, BT_CM_PROFILE_SERVICE_MASK_ALL};
            memcpy(connect_param.address, device_list[i], sizeof(bt_bd_addr_t));
            bt_status_t bt_status = bt_cm_disconnect(&connect_param);

            uint8_t *addr = device_list[i];
            APPS_LOG_MSGID_I(LOG_TAG" disconnect_edr, disconnect addr=%02X:%02X:%02X:%02X:%02X:%02X bt_status=0x%08X",
                             7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], bt_status);
            break;
        }
    }

    return TRUE;
}
#endif

#if 0
static void app_bt_emp_disconnect_non_va_link()
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    xiaoai_connection_state xiaoai_state = conn_info.conn_state;
    bool is_le_audio_mma = conn_info.is_le_audio_mma;
    bool is_ble = conn_info.is_ble;
    uint8_t *addr = conn_info.phone_addr;
    APPS_LOG_MSGID_I(LOG_TAG" disconnect_non_va_link, xiaoai_state=%d is_ble=%d is_le_audio_mma=%d addr=%02X:%02X:%02X:%02X:%02X:XX",
                     8, xiaoai_state, is_ble, is_le_audio_mma, addr[5], addr[4], addr[3],
                     addr[2], addr[1]);
    if (xiaoai_state == XIAOAI_STATE_CONNECTED) {
        if (!is_ble) {
            // SPP - Disconnect All LEA
#ifdef AIR_LE_AUDIO_ENABLE
            app_lea_service_disconnect(TRUE, APP_LE_AUDIO_DISCONNECT_MODE_ALL, NULL, BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
#endif
        } else if (is_le_audio_mma) {
            // LEA - Disconnect other LEA, ALL EDR
            app_bt_emp_disconnect_edr(NULL);
#ifdef AIR_LE_AUDIO_ENABLE
            app_lea_service_disconnect(TRUE, APP_LE_AUDIO_DISCONNECT_MODE_KEEP, addr, BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
#endif
        }

        bt_bd_addr_t keep_phone_addr[1] = { { 0 } };
        memcpy(keep_phone_addr[0], addr, BT_BD_ADDR_LEN);
        uint8_t bt_cm_num = app_bt_emp_srv_get_bt_cm_num(FALSE);
        // Disconnect other EDR, set BT_CM 3
        bt_status_t bt_status = bt_cm_set_max_connection_number(bt_cm_num, keep_phone_addr, 0, FALSE);
        APPS_LOG_MSGID_I(LOG_TAG" disconnect_non_va_link, [%02X] bt_cm_num=%d bt_status=0x%08X",
                         3, role, bt_cm_num, bt_status);
    }
}
#endif

static void app_bt_emp_srv_init()
{
    uint32_t size = sizeof(uint8_t);
    nvkey_status_t status = nvkey_read_data(NVID_APP_MULTI_POINT_ENABLE, (uint8_t *)&app_bt_emp_enable_flag, &size);
    APPS_LOG_MSGID_I(LOG_TAG" init, read status=%d", 1, status);
    if (status == NVKEY_STATUS_ITEM_NOT_FOUND) {
        bool emp_enable = APP_BT_EMP_SWITCH_INIT_STATE;
        size = sizeof(uint8_t);
        status = nvkey_write_data(NVID_APP_MULTI_POINT_ENABLE, (const uint8_t *)&emp_enable, size);
        //APPS_LOG_MSGID_I(LOG_TAG" init, write status=%d", 1, status);
        if (status == NVKEY_STATUS_OK) {
            app_bt_emp_enable_flag = emp_enable;
        }
    }
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    app_lea_service_enable_multi_conn(app_bt_emp_enable_flag);
#endif
#ifdef AIR_APP_A2DP_LBB_VENDOR_CODEC_LIMIT
    bt_sink_srv_a2dp_enable_vendor_codec(!app_bt_emp_enable_flag);
#endif
    bt_device_manager_link_record_set_max_num(app_bt_emp_enable_flag ? APP_BT_EMP_MAX_CONN_NUM : APP_BT_EMP_MIN_CONN_NUM);
    APPS_LOG_MSGID_I(LOG_TAG" init %d", 1, app_bt_emp_enable_flag);
}



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
bool app_bt_emp_service_activity_proc(struct _ui_shell_activity *self,
                                      uint32_t event_group,
                                      uint32_t event_id,
                                      void *extra_data,
                                      size_t data_len)
{
    bool ret = FALSE;
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            ret = app_bt_emp_srv_activity_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
#ifdef MTK_AWS_MCE_ENABLE
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            ret = app_bt_emp_srv_activity_proc_aws_data(self, event_id, extra_data, data_len);
            break;
#endif
    }
    return ret;
}

bool app_bt_emp_is_enable()
{
    return app_bt_emp_enable_flag;
}

bool app_bt_emp_switch_enable(bool enable)
{
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    app_lea_service_enable_multi_conn(enable);
#endif
#ifdef AIR_APP_A2DP_LBB_VENDOR_CODEC_LIMIT
    bt_sink_srv_a2dp_enable_vendor_codec(!app_bt_emp_enable_flag);
#endif
    bt_device_manager_link_record_set_max_num(enable ? APP_BT_EMP_MAX_CONN_NUM : APP_BT_EMP_MIN_CONN_NUM);
    app_bt_emp_enable_flag = enable;
    /* Speaker notify MULTI_POINT_STATE_CHANGED here */
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_MULTI_POINT_STATE_CHANGED, NULL, 0, NULL, 0);
    return TRUE;
}

bool app_bt_emp_enable(bool enable)
{
    if (app_bt_emp_enable_flag == enable) {
        APPS_LOG_MSGID_I(LOG_TAG" enable, already enable=%d", 1, enable);
        return TRUE;
    }

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_link_type_t link_type = bt_aws_mce_srv_get_link_type();
    if (role != BT_AWS_MCE_ROLE_AGENT || link_type == BT_AWS_MCE_SRV_LINK_NONE) {
        APPS_LOG_MSGID_E(LOG_TAG" enable, [%02X] error link_type=%d", 2, role, link_type);
        return FALSE;
    }
#endif

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    app_lea_service_enable_multi_conn(enable);
#endif
    bt_device_manager_link_record_set_max_num(enable ? APP_BT_EMP_MAX_CONN_NUM : APP_BT_EMP_MIN_CONN_NUM);

    bool success = FALSE;
    //APPS_LOG_MSGID_I(LOG_TAG" enable, enable=%d", 1, enable);

    // Check whether allow
    bool allow = TRUE;
    for (uint8_t user_id = 0; user_id < APP_BT_EMP_SRV_USER_ID_MAX; user_id++) {
        app_bt_emp_switch_allow_cb_t allow_cb = app_bt_emp_srv_user_list[user_id];
        if (allow_cb != NULL && !allow_cb(enable, NULL)) {
            APPS_LOG_MSGID_I(LOG_TAG" enable, user_id=%d disallow", 1, user_id);
            allow = FALSE;
            break;
        }
    }
    if (!allow) {
        goto exit;
    }

    // Check keep_phone_addr (now only keep one SP connection)
    bt_bd_addr_t keep_phone_addr[1] = {{0}};
    uint8_t list_size = 0;
    if (!enable) {
#ifdef AIR_XIAOAI_ENABLE
        xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
        xiaoai_connection_state xiaoai_state = conn_info.conn_state;
        bool is_le_audio_mma = conn_info.is_le_audio_mma;
        bool is_ble = conn_info.is_ble;
        uint8_t *addr = conn_info.phone_addr;
        APPS_LOG_MSGID_I(LOG_TAG"[XIAOAI_EMP_OFF] enable, xiaoai_state=%d is_ble=%d is_le_audio_mma=%d addr=%02X:%02X:%02X:%02X:%02X:XX",
                         8, xiaoai_state, is_ble, is_le_audio_mma,
                         addr[5], addr[4], addr[3], addr[2], addr[1]);
        if (xiaoai_state == XIAOAI_STATE_CONNECTED) {
            if (!is_ble) {
                // XiaoAI Android SPP, disconnect all LEA and other EDR
                memcpy(keep_phone_addr[0], addr, 6);
                list_size = 1;
#ifdef AIR_LE_AUDIO_ENABLE
                app_lea_service_disconnect(TRUE, APP_LE_AUDIO_DISCONNECT_MODE_ALL, NULL, BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
#endif
                // Disconnect other EDR via BT CM set 3
            } else if (is_le_audio_mma) {
                // XiaoAI LE Audio, disconnect all EDR and other LEA
                app_bt_emp_disconnect_edr(NULL);
#ifdef AIR_LE_AUDIO_ENABLE
                app_lea_service_disconnect(TRUE, APP_LE_AUDIO_DISCONNECT_MODE_KEEP, addr, BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
#endif
            } else {
                // XiaoAI iPhone BLE(iPhone EDR), disallow when conn_num > 1, allow when conn_num==1
            }
        }
#endif
    }

    // Count bt_cm_number
    uint8_t bt_cm_num = app_bt_emp_srv_get_bt_cm_num(enable);
    // BT CM set max connection_number
    bt_status_t status = bt_cm_set_max_connection_number(bt_cm_num, keep_phone_addr, list_size, TRUE);
    uint8_t *addr = (uint8_t *)keep_phone_addr;
    APPS_LOG_MSGID_I(LOG_TAG" enable, bt_cm_num=%d %02X:%02X:%02X:%02X:%02X:XX list_size=%d status=0x%08X",
                     8, bt_cm_num, addr[5], addr[4], addr[3], addr[2], addr[1],
                     list_size, status);
    if (status != BT_STATUS_SUCCESS) {
        goto exit;
    }

    // Update NVKEY and send to partner
    app_bt_emp_srv_update_nvkey(enable);
#ifdef MTK_AWS_MCE_ENABLE
    app_bt_emp_srv_sync_to_peer(enable);
#endif
    success = TRUE;

exit:
    if (!success) {
        APPS_LOG_MSGID_E(LOG_TAG" enable, enable=%d fail", 1, enable);
    }
    return success;
}

void app_bt_emp_reset_max_conn_num()
{
    uint8_t bt_cm_num = app_bt_emp_srv_get_bt_cm_num(app_bt_emp_enable_flag);

    bt_bd_addr_t keep_phone_addr[1] = {{0}};
    bt_status_t status = bt_cm_set_max_connection_number(bt_cm_num, keep_phone_addr, 0, TRUE);
    APPS_LOG_MSGID_I(LOG_TAG" reinit, emp_enable_flag=%d bt_cm num=%d status=0x%08X",
                     3, app_bt_emp_enable_flag, bt_cm_num, status);
}

bool app_bt_emp_srv_user_register(app_bt_emp_srv_user_id user_id, app_bt_emp_switch_allow_cb_t func)
{
    bool ret = FALSE;
    if (user_id < APP_BT_EMP_SRV_USER_ID_MAX) {
        app_bt_emp_srv_user_list[user_id] = func;
        ret = TRUE;
    }
    //APPS_LOG_MSGID_I(LOG_TAG" user_register, user_id=%d ret=%d", 2, user_id, ret);
    return ret;
}

bool app_bt_emp_srv_user_deregister(app_bt_emp_srv_user_id user_id)
{
    bool ret = FALSE;
    if (user_id < APP_BT_EMP_SRV_USER_ID_MAX) {
        app_bt_emp_srv_user_list[user_id] = NULL;
        ret = TRUE;
    }
    //APPS_LOG_MSGID_I(LOG_TAG" user_deregister, user_id=%d ret=%d", 2, user_id, ret);
    return ret;
}

#endif  /* AIR_MULTI_POINT_ENABLE */
