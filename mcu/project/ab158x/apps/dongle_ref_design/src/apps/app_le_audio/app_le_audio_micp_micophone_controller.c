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
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_le_audio_msglog.h"
#include "apps_events_event_group.h"
#include "ui_shell_manager.h"
#include "ble_micp_enhance.h"
#include "app_le_audio.h"
#include "app_le_audio_utillity.h"
#include "app_le_audio_micp_micophone_controller.h"
#include "app_le_audio_csip_set_coordinator.h"

/**************************************************************************************************
* Define
**************************************************************************************************/



/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/

//app_le_audio_micp_info_t g_lea_micp_link_info[APP_LE_AUDIO_UCST_LINK_MAX_NUM];

/**************************************************************************************************
* Prototype
**************************************************************************************************/
extern void bt_app_common_at_cmd_print_report(char *string);


/**************************************************************************************************
* Static Functions
**************************************************************************************************/

static void app_le_audio_micp_handle_discover_service_complete(ble_micp_enhance_discover_service_complete_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][MICP] micp_handle_discover_service_complete, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][MICP] micp_handle_discover_service_complete, handle:%x state:%x->%x aics:%x", 4, p_info->handle,
                   p_info->curr_state, p_info->next_state, cnf->number_of_aics);

}

static void app_le_audio_micp_handle_read_mute_cnf(ble_micp_enhance_read_mute_cfm_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    char conn_string[50] = {0};

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][MICP] read_mute_cnf, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }
    snprintf((char *)conn_string, 50, "mic mute state:0x%02x, handle:0x%02x", cnf->mute, cnf->handle);
    bt_app_common_at_cmd_print_report(conn_string);

    p_info->micp_info.mics_info.mute = cnf->mute;

    LE_AUDIO_MSGLOG_I("[APP][MICP] read_mute_cnf, handle:%x, mute:0x%x", 2, p_info->handle, cnf->mute);
}

static void app_le_audio_micp_handle_mute_notify(ble_micp_enhance_mute_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    app_le_audio_micp_operate_t current_operate;
    char conn_string[50] = {0};

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    snprintf((char *)conn_string, 50, "mic mute state:0x%02x, handle:0x%02x", p_notify->mute, p_notify->handle);
    bt_app_common_at_cmd_print_report(conn_string);

    LE_AUDIO_MSGLOG_I("[APP][MICP] mute, handle:%x mute:0x%x->0x%x", 3, p_info->handle, p_info->micp_info.mics_info.mute, p_notify->mute);

    p_info->micp_info.mics_info.mute = p_notify->mute;
    current_operate = p_info->micp_info.mics_info.current_operate;
    p_info->micp_info.mics_info.current_operate = APP_LEA_MICP_OPERATE_IDLE;

    if (APP_LEA_MICP_OPERATE_IDLE == current_operate) {
        // Remote device's mic mute changed by other way, sync to other set member
        uint8_t link_idx;
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        uint8_t group_id = p_info->group_id;
#endif
        for (link_idx = 0; link_idx < APP_LE_AUDIO_UCST_LINK_MAX_NUM; link_idx++) {
            if (NULL == (p_info = app_le_audio_ucst_get_link_info_by_idx(link_idx))) {
                continue;
            }
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
            if (group_id != p_info->group_id) {
                continue;
            }
#endif
            if (p_notify->handle == p_info->handle) {
                continue;
            }

            if (BT_HANDLE_INVALID != p_info->handle) {
                if(BLE_MICS_MUTE_STATE_UNMUTE == p_notify->mute) {
                    app_le_audio_micp_set_mute_state(p_info->handle, false);
                }
                else if(BLE_MICS_MUTE_STATE_MUTE == p_notify->mute) {
                    app_le_audio_micp_set_mute_state(p_info->handle, true);
                }
            }
        }

    }
    else {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO,APP_LE_AUDIO_EVENT_MICP_RETRY);
        if (p_info->micp_info.mics_info.target_mute != p_info->micp_info.mics_info.mute) {
            if(BLE_MICS_MUTE_STATE_UNMUTE == p_info->micp_info.mics_info.target_mute) {
                app_le_audio_micp_set_mute_state(p_info->handle, false);
            }
            else if(BLE_MICS_MUTE_STATE_MUTE == p_info->micp_info.mics_info.target_mute) {
                app_le_audio_micp_set_mute_state(p_info->handle, true);
            }
        }
    }
}

static void app_le_audio_micp_handle_write_mute_response(ble_micp_enhance_write_cfm_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][MICP] write_mute_response, handle:%x service_index:0x%x status:0x%x", 3, p_info->handle,
                    cnf->service_index, cnf->status);
    //BLE_MICS_ATT_APP_ERROR_MUTE_DISABLED
}


static void app_le_audio_micp_retry_operate(uint32_t micp_retry)
{
    ui_shell_send_event(TRUE,
                    EVENT_PRIORITY_HIGH,
                    EVENT_GROUP_UI_SHELL_LE_AUDIO,
                    APP_LE_AUDIO_EVENT_MICP_RETRY,
                    (void *)micp_retry,
                    0,
                    NULL,
                    60);
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
bool app_le_audio_micp_is_enhanced(void)
{
    //MICP Enhance is not supportted before SDK V3.8.0
    //The way to use MICP Enhance:
    //Step 1: Please replace the following files with the ones in the SDK which is after V3.8.0.
    //        app_le_audio_micp_micophone_controller.c, app_le_audio_micp_micophone_controller.h
    //Step 2: Merge newest code which calls the new api in app_le_audio_micp_micophone_controller.h from the latest SDK.
    //        Note: Buid error will help you to find the code and files that you must merge.
    return true;
}

void app_le_audio_micp_handle_evt(ble_micp_enhance_event_t event, void *msg)
{
    if (NULL == msg) {
        return;
    }

    switch (event) {
        case BLE_MICP_ENHANCE_MICS_READ_MUTE_CFM: {
            app_le_audio_micp_handle_read_mute_cnf((ble_micp_enhance_read_mute_cfm_t *)msg);
            break;
        }
        case BLE_MICP_ENHANCE_MICS_MUTE_NOTIFY: {
            app_le_audio_micp_handle_mute_notify((ble_micp_enhance_mute_notify_t *)msg);
            break;
        }
        case BLE_MICP_ENHANCE_DISCOVER_SERVICE_COMPLETE_NOTIFY: {
            app_le_audio_micp_handle_discover_service_complete((ble_micp_enhance_discover_service_complete_t *)msg);
            break;
        }
        case BLE_MICP_ENHANCE_MICS_WRITE_MUTE_CFM: {
            app_le_audio_micp_handle_write_mute_response((ble_micp_enhance_write_cfm_t *)msg);
            break;
        }
        default:
            break;
    }
}


bt_status_t app_le_audio_micp_handle_retry_event(uint32_t micp_retry)
{
    bt_handle_t handle = (bt_handle_t)((micp_retry >> 16) & 0xFFFF);
    ble_mics_mute_state_t param;
    bt_status_t ret = BT_STATUS_FAIL;
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }

    param = (uint8_t)(micp_retry & 0xFF);
    if(BLE_MICS_MUTE_STATE_UNMUTE == param) {
        ret = app_le_audio_micp_set_mute_state(p_info->handle, false);
    }
    else if(BLE_MICS_MUTE_STATE_MUTE == param) {
        ret = app_le_audio_micp_set_mute_state(p_info->handle, true);
    }

    if (BT_STATUS_CONNECTION_IN_USE == ret) {
        //uint32_t micp_retry = param | (p_info->handle<<16);
        app_le_audio_micp_retry_operate(micp_retry);
    }

    LE_AUDIO_MSGLOG_I("[APP][MICP] app_le_audio_micp_handle_retry_event handle:%x mute_state:%x ret:%x", 3, p_info->handle, param, ret);
    return ret;
}

bt_status_t app_le_audio_micp_set_mute_state(bt_handle_t handle, bool mute)
{
    ble_mics_mute_state_t param = BLE_MICS_MUTE_STATE_UNMUTE;
    bt_status_t ret = BT_STATUS_FAIL;
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (!app_le_audio_ucst_is_active_group(p_info->group_id)) {
        return BT_STATUS_FAIL;
    }
#endif

    if (mute) {
        param = BLE_MICS_MUTE_STATE_MUTE;
    }
    p_info->micp_info.mics_info.current_operate = param + 1;
    p_info->micp_info.mics_info.target_mute = param;

    if ((param == p_info->micp_info.mics_info.mute) || (BLE_MICS_MUTE_STATE_DISABLE == p_info->micp_info.mics_info.mute)) {
        return BT_STATUS_FAIL;
    }

    if (BT_STATUS_SUCCESS != (ret = ble_micp_enhance_mics_write_mute(p_info->handle, param))) {
        if (BT_STATUS_CONNECTION_IN_USE == ret) {
            uint32_t micp_retry = param | (p_info->handle<<16);
            app_le_audio_micp_retry_operate(micp_retry);
        }
    }
    LE_AUDIO_MSGLOG_I("[APP][MICP] app_le_audio_micp_set_mute_state, handle:%x mute:ret:%x ret:%x", 3, p_info->handle, param, ret);
    return ret;
}

bt_status_t app_le_audio_micp_control_active_device(app_le_audio_micp_operate_t operate, void *parameter)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret = BT_STATUS_FAIL;
    uint8_t link_idx;

    for (link_idx = 0; link_idx < APP_LE_AUDIO_UCST_LINK_MAX_NUM; link_idx++) {
        if (NULL == (p_info = app_le_audio_ucst_get_link_info_by_idx(link_idx))) {
            continue;
        }
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (!app_le_audio_ucst_is_active_group(p_info->group_id)) {
            continue;
        }
#endif
        if (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC > p_info->curr_state) {
            //Prevent Service Discovery from being interrupted
            continue;
        }

        if ((BT_HANDLE_INVALID != p_info->handle)) {
            switch (operate) {
                case APP_LEA_MICP_OPERATE_UNMUTE: {
                    ret = app_le_audio_micp_set_mute_state(p_info->handle, false);
                    break;
                }
                case APP_LEA_MICP_OPERATE_MUTE: {
                    ret = app_le_audio_micp_set_mute_state(p_info->handle, true);
                    break;
                }
                default:
                    break;
            }
        }
    }
    return ret;
}

// for AT CMD (BQB)
bt_status_t app_le_audio_micp_control_device(app_le_audio_micp_operate_t operate, void *parameter)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret = BT_STATUS_FAIL;
    uint8_t link_idx;

    for (link_idx = 0; link_idx < APP_LE_AUDIO_UCST_LINK_MAX_NUM; link_idx++) {
        if (NULL == (p_info = app_le_audio_ucst_get_link_info_by_idx(link_idx))) {
            continue;
        }

        if ((BT_HANDLE_INVALID != p_info->handle)) {
            switch (operate) {
                case APP_LEA_MICP_OPERATE_UNMUTE: {
                    ret = app_le_audio_micp_set_mute_state(p_info->handle, false);
                    break;
                }
                case APP_LEA_MICP_OPERATE_MUTE: {
                    ret = app_le_audio_micp_set_mute_state(p_info->handle, true);
                    break;
                }
                default:
                    break;
            }
        }
    }
    return ret;
}

#endif


