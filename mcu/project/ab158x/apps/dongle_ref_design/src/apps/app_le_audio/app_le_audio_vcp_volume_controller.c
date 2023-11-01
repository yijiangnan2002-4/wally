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
#include "ble_vcp_enhance.h"
#include "app_le_audio.h"
#include "app_le_audio_utillity.h"
#include "app_le_audio_vcp_volume_controller.h"
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

//app_le_audio_vcp_info_t g_lea_vcp_link_info[APP_LE_AUDIO_UCST_LINK_MAX_NUM];

/**************************************************************************************************
* Prototype
**************************************************************************************************/
extern void bt_app_common_at_cmd_print_report(char *string);

static void app_le_audio_vcp_handle_discover_service_complete(ble_vcp_enhance_discover_service_complete_t *cnf);
static void app_le_audio_vcp_handle_read_volume_flages_cnf(ble_vcp_enhance_read_volume_flags_cfm_t *cnf);
static void app_le_audio_vcp_handle_read_volume_state_cnf(ble_vcp_enhance_read_volume_state_cfm_t *cnf);
static void app_le_audio_vcp_handle_volume_flags_notify(ble_vcp_enhance_volume_flags_notify_t *p_notify);
static void app_le_audio_vcp_handle_volume_state_notify(ble_vcp_enhance_volume_state_notify_t *p_notify);
static void app_le_audio_vcp_handle_write_volume_control_response(ble_vcp_enhance_write_cfm_t *cnf);
static void app_le_audio_vcp_retry_operate(uint32_t vcp_retry);


/**************************************************************************************************
* Static Functions
**************************************************************************************************/

static void app_le_audio_vcp_handle_discover_service_complete(ble_vcp_enhance_discover_service_complete_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][VCP] vcp_handle_discover_service_complete, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][VCP] vcp_handle_discover_service_complete, handle:%x state:%x->%x vocs:%x aics:%x", 5, p_info->handle,
                   p_info->curr_state, p_info->next_state, cnf->number_of_vocs, cnf->number_of_aics);

}

static void app_le_audio_vcp_handle_read_volume_flages_cnf(ble_vcp_enhance_read_volume_flags_cfm_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][VCP] read_volume_flages_cnf, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }

    p_info->vcp_info.vcs_info.volume_flags = cnf->volume_flags;

    LE_AUDIO_MSGLOG_I("[APP][VCP] read_volume_flages_cnf, handle:%x volume_flags:0x%x", 2, p_info->handle, cnf->volume_flags);
}

static void app_le_audio_vcp_handle_read_volume_state_cnf(ble_vcp_enhance_read_volume_state_cfm_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    char conn_string[50] = {0};

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        LE_AUDIO_MSGLOG_I("[APP][VCP] read_volume_state_cnf, link not exist (hdl:%x)", 1, cnf->handle);
        return;
    }
    snprintf((char *)conn_string, 50, "Volume state:0x%02x, mute:0x%02x, handle:0x%02x", cnf->volume_setting, cnf->mute, cnf->handle);
    bt_app_common_at_cmd_print_report(conn_string);

    p_info->vcp_info.vcs_info.volume_setting = cnf->volume_setting;
    p_info->vcp_info.vcs_info.mute = cnf->mute;
    p_info->vcp_info.vcs_info.change_counter = cnf->change_counter;

    LE_AUDIO_MSGLOG_I("[APP][VCP] read_volume_state, handle:%x volume_setting:0x%x, mute:0x%x, change_counter:0x%x", 4, p_info->handle,
                    cnf->volume_setting, cnf->mute, cnf->change_counter);
    //BLE_VCS_ATT_APP_ERROR_INVALID_CHANGE_COUNTER retry current_operate
    if (APP_LEA_VCP_OPERATE_RELATIVE_VOLUME_DOWN == p_info->vcp_info.vcs_info.current_operate) {
        app_le_audio_vcp_relative_volume_down(cnf->handle, false);
    }
    else if (APP_LEA_VCP_OPERATE_RELATIVE_VOLUME_UP == p_info->vcp_info.vcs_info.current_operate) {
        app_le_audio_vcp_relative_volume_up(cnf->handle, false);
    }
    else if (APP_LEA_VCP_OPERATE_UNMUTE_RELATIVE_VOLUME_DOWN == p_info->vcp_info.vcs_info.current_operate) {
        app_le_audio_vcp_relative_volume_down(cnf->handle, true);
    }
    else if (APP_LEA_VCP_OPERATE_UNMUTE_RELATIVE_VOLUME_UP == p_info->vcp_info.vcs_info.current_operate) {
        app_le_audio_vcp_relative_volume_up(cnf->handle, true);
    }
    else if (APP_LEA_VCP_OPERATE_SET_ABSOLUTE_VOLUME == p_info->vcp_info.vcs_info.current_operate) {
        app_le_audio_vcp_set_absolute_volume(cnf->handle, p_info->vcp_info.vcs_info.target_volume_setting);
    }
    else if (APP_LEA_VCP_OPERATE_UNMUTE == p_info->vcp_info.vcs_info.current_operate) {
        app_le_audio_vcp_unmute(cnf->handle);
    }
    else if (APP_LEA_VCP_OPERATE_MUTE == p_info->vcp_info.vcs_info.current_operate) {
        app_le_audio_vcp_mute(cnf->handle);
    }
}

static void app_le_audio_vcp_handle_volume_flags_notify(ble_vcp_enhance_volume_flags_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][VCP] volume_flages, handle:%x volume_flags:0x%x", 2, p_info->handle, p_notify->volume_flags);

    p_info->vcp_info.vcs_info.volume_flags = p_notify->volume_flags;

}

static void app_le_audio_vcp_handle_volume_state_notify(ble_vcp_enhance_volume_state_notify_t *p_notify)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    app_le_audio_vcp_operate_t current_operate;
    char conn_string[50] = {0};

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(p_notify->handle))) {
        return;
    }

    snprintf((char *)conn_string, 50, "Volume state:0x%02x, mute:0x%02x, handle:0x%02x", p_notify->volume_setting, p_notify->mute, p_notify->handle);
    bt_app_common_at_cmd_print_report(conn_string);

    LE_AUDIO_MSGLOG_I("[APP][VCP] volume_state_notify, handle:%x operate:%d->%d volume_setting:0x%x->0x%x, mute:0x%x->0x%x, change_counter:0x%x->0x%x", 9, p_info->handle,
                    p_info->vcp_info.vcs_info.current_operate, p_info->vcp_info.vcs_info.next_operate,
                    p_info->vcp_info.vcs_info.volume_setting, p_notify->volume_setting,
                    p_info->vcp_info.vcs_info.mute, p_notify->mute,
                    p_info->vcp_info.vcs_info.change_counter, p_notify->change_counter);

    p_info->vcp_info.vcs_info.volume_setting = p_notify->volume_setting;
    p_info->vcp_info.vcs_info.mute = p_notify->mute;
    p_info->vcp_info.vcs_info.change_counter = p_notify->change_counter;
    current_operate = p_info->vcp_info.vcs_info.current_operate;
    p_info->vcp_info.vcs_info.current_operate = APP_LEA_VCP_OPERATE_IDLE;
#if 0
    if (APP_LEA_VCP_OPERATE_UNMUTE == p_info->vcp_info.vcs_info.next_operate) {
        app_le_audio_vcp_unmute(p_notify->handle);
    }
    else if (APP_LEA_VCP_OPERATE_MUTE == p_info->vcp_info.vcs_info.next_operate) {
        app_le_audio_vcp_mute(p_notify->handle);
    }
    p_info->vcp_info.vcs_info.next_operate = APP_LEA_VCP_OPERATE_IDLE;
#endif
    if (APP_LEA_VCP_OPERATE_IDLE == current_operate) {
        // Remote device's volume changed by other way, sync to other set member
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
                app_le_audio_vcp_set_volume_state(p_info->handle, p_notify->volume_setting, p_notify->mute);
            }
        }

    }
    else {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO,APP_LE_AUDIO_EVENT_VCP_RETRY);
        if (p_info->vcp_info.vcs_info.target_mute != p_info->vcp_info.vcs_info.mute) {
            if (BLE_VCS_MUTE == p_info->vcp_info.vcs_info.target_mute) {
                app_le_audio_vcp_mute(p_info->handle);
            }
            else {
                app_le_audio_vcp_unmute(p_info->handle);
            }
        }
        if (APP_LEA_VCP_OPERATE_SET_ABSOLUTE_VOLUME <= current_operate) {
            if (p_notify->volume_setting != p_info->vcp_info.vcs_info.target_volume_setting) {
                app_le_audio_vcp_set_absolute_volume(p_info->handle, p_info->vcp_info.vcs_info.target_volume_setting);
            }
        }
    }

}

static void app_le_audio_vcp_handle_write_volume_control_response(ble_vcp_enhance_write_cfm_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_status_t ret;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(cnf->handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][VCP] write_volume_control_response, handle:%x service_index:0x%x status:0x%x", 3, p_info->handle,
                    cnf->service_index, cnf->status);
    //if (BT_STATUS_SUCCESS != cnf->status) {
    //    p_info->vcp_info.vcs_info.current_operate = APP_LEA_VCP_OPERATE_IDLE;
    //}

    if (BLE_VCS_ATT_APP_ERROR_INVALID_CHANGE_COUNTER == cnf->status) {
        if (BT_STATUS_SUCCESS != (ret = ble_vcp_enhance_vcs_read_volume_state(p_info->handle))) {
            LE_AUDIO_MSGLOG_I("[APP] ble_vcp_enhance_vcs_read_volume_state failed, handle:%x ret:%x", 2, p_info->handle, ret);
        }
    }
}

static void app_le_audio_vcp_retry_operate(uint32_t vcp_retry)
{
    ui_shell_send_event(TRUE,
                    EVENT_PRIORITY_HIGH,
                    EVENT_GROUP_UI_SHELL_LE_AUDIO,
                    APP_LE_AUDIO_EVENT_VCP_RETRY,
                    (void *)vcp_retry,
                    0,
                    NULL,
                    60);
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
bool app_le_audio_vcp_is_enhanced(void)
{
    //VCP Enhance is not supportted before SDK V3.8.0
    //The way to use VCP Enhance:
    //Step 1: Please replace the following files with the ones in the SDK which is after V3.8.0.
    //        app_le_audio_vcp_volume_controller.c, app_le_audio_vcp_volume_controller.h
    //Step 2: Merge newest code which calls the new api in app_le_audio_vcp_volume_controller.h from the latest SDK.
    //        Note: Buid error will help you to find the code and files that you must merge.
    return true;
}

void app_le_audio_vcp_handle_evt(ble_vcp_enhance_event_t event, void *msg)
{
    if (NULL == msg) {
        return;
    }

    switch (event) {
        case BLE_VCP_ENHANCE_VCS_READ_VOLUME_FLAGS_CFM: {
            app_le_audio_vcp_handle_read_volume_flages_cnf((ble_vcp_enhance_read_volume_flags_cfm_t *)msg);
            break;
        }
        case BLE_VCP_ENHANCE_VCS_VOLUME_FLAGS_NOTIFY: {
            app_le_audio_vcp_handle_volume_flags_notify((ble_vcp_enhance_volume_flags_notify_t *)msg);
            break;
        }
        case BLE_VCP_ENHANCE_VCS_READ_VOLUME_STATE_CFM: {
            app_le_audio_vcp_handle_read_volume_state_cnf((ble_vcp_enhance_read_volume_state_cfm_t *)msg);
            break;
        }
        case BLE_VCP_ENHANCE_VCS_VOLUME_STATE_NOTIFY: {
            app_le_audio_vcp_handle_volume_state_notify((ble_vcp_enhance_volume_state_notify_t *)msg);
            break;
        }
        case BLE_VCP_ENHANCE_DISCOVER_SERVICE_COMPLETE_NOTIFY: {
            app_le_audio_vcp_handle_discover_service_complete((ble_vcp_enhance_discover_service_complete_t *)msg);
            break;
        }
        case BLE_VCP_ENHANCE_VCS_WRITE_VOLUME_CONTROL_POINT_CFM: {
            app_le_audio_vcp_handle_write_volume_control_response((ble_vcp_enhance_write_cfm_t *)msg);
            break;
        }
        default:
            break;
    }
}

bt_status_t app_le_audio_vcp_handle_retry_event(uint32_t vcp_retry)
{
    bt_handle_t handle = (bt_handle_t)((vcp_retry >> 16) & 0xFFFF);
    ble_vcp_enhance_volume_control_param_t param;
    bt_status_t ret = BT_STATUS_FAIL;
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }

    param.opcode = (uint8_t)(vcp_retry & 0xFF);
    param.change_counter = p_info->vcp_info.vcs_info.change_counter;
    if (BLE_VCS_OPCODE_SET_ABSOLUTE_VOLUME == param.opcode) {
        param.volume_setting = p_info->vcp_info.vcs_info.target_volume_setting;
    }

    if (BT_STATUS_SUCCESS != (ret = ble_vcp_enhance_vcs_write_volume_control_point(p_info->handle, &param))) {
        if (BT_STATUS_CONNECTION_IN_USE == ret) {
            //uint32_t vcp_retry = param.opcode | (p_info->handle<<16);
            app_le_audio_vcp_retry_operate(vcp_retry);
        }
    }
    else {
        p_info->vcp_info.vcs_info.current_operate = param.opcode + 1;
    }

    LE_AUDIO_MSGLOG_I("[APP][VCP] app_le_audio_vcp_handle_retry_event handle:%x opcode:%x ret:%x", 3, p_info->handle, param.opcode, ret);
    return ret;
}

bt_status_t app_le_audio_vcp_relative_volume_down(bt_handle_t handle, bool unmute)
{
    ble_vcp_enhance_volume_control_param_t param;
    bt_status_t ret = BT_STATUS_FAIL;
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }

    if ((false == unmute) && (BLE_VCS_VOLUME_MIN == p_info->vcp_info.vcs_info.volume_setting)) {
        return BT_STATUS_FAIL;
    }

    if (true == unmute) {
        p_info->vcp_info.vcs_info.target_mute = BLE_VCS_UNMUTE;
    }

    if ((true == unmute) && (BLE_VCS_VOLUME_MIN == p_info->vcp_info.vcs_info.volume_setting) && (BLE_VCS_UNMUTE == p_info->vcp_info.vcs_info.mute)) {
        return BT_STATUS_FAIL;
    }

    param.opcode = unmute ? BLE_VCS_OPCODE_UNMUTE_RELATIVE_VOLUME_DOWN : BLE_VCS_OPCODE_RELATIVE_VOLUME_DOWN;
    param.change_counter = p_info->vcp_info.vcs_info.change_counter;

    if (BT_STATUS_SUCCESS != (ret = ble_vcp_enhance_vcs_write_volume_control_point(p_info->handle, &param))) {
        if (BT_STATUS_CONNECTION_IN_USE == ret) {
            uint32_t vcp_retry = param.opcode | (p_info->handle<<16);
            app_le_audio_vcp_retry_operate(vcp_retry);
        }
    }
    else {
        p_info->vcp_info.vcs_info.current_operate = param.opcode + 1;
    }

    LE_AUDIO_MSGLOG_I("[APP][VCP] app_le_audio_vcp_relative_volume_down, handle:%x unmute:%d ret:%x", 3, p_info->handle, unmute, ret);
    return ret;
}

bt_status_t app_le_audio_vcp_relative_volume_up(bt_handle_t handle, bool unmute)
{
    ble_vcp_enhance_volume_control_param_t param;
    bt_status_t ret = BT_STATUS_FAIL;
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }

    if ((false == unmute) && (BLE_VCS_VOLUME_MAX == p_info->vcp_info.vcs_info.volume_setting)) {
        return BT_STATUS_FAIL;
    }

    if (true == unmute) {
        p_info->vcp_info.vcs_info.target_mute = BLE_VCS_UNMUTE;
    }

    if ((true == unmute) && (BLE_VCS_VOLUME_MAX == p_info->vcp_info.vcs_info.volume_setting) && (BLE_VCS_UNMUTE == p_info->vcp_info.vcs_info.mute)) {
        return BT_STATUS_FAIL;
    }

    param.opcode = unmute ? BLE_VCS_OPCODE_UNMUTE_RELATIVE_VOLUME_UP : BLE_VCS_OPCODE_RELATIVE_VOLUME_UP;
    param.change_counter = p_info->vcp_info.vcs_info.change_counter;

    if (BT_STATUS_SUCCESS != (ret = ble_vcp_enhance_vcs_write_volume_control_point(p_info->handle, &param))) {
        if (BT_STATUS_CONNECTION_IN_USE == ret) {
            uint32_t vcp_retry = param.opcode | (p_info->handle<<16);
            app_le_audio_vcp_retry_operate(vcp_retry);
        }
    }
    else {
        p_info->vcp_info.vcs_info.current_operate = param.opcode + 1;
    }

    LE_AUDIO_MSGLOG_I("[APP][VCP] app_le_audio_vcp_relative_volume_up, handle:%x unmute:%d ret:%x", 3, p_info->handle, unmute, ret);
    return ret;
}

bt_status_t app_le_audio_vcp_set_absolute_volume(bt_handle_t handle, ble_vcs_volume_t volume_setting)
{
    ble_vcp_enhance_volume_control_param_t param;
    bt_status_t ret = BT_STATUS_FAIL;
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }
    if (volume_setting == p_info->vcp_info.vcs_info.volume_setting) {
        return BT_STATUS_FAIL;
    }
    p_info->vcp_info.vcs_info.target_volume_setting = volume_setting;

    param.opcode = BLE_VCS_OPCODE_SET_ABSOLUTE_VOLUME;
    param.change_counter = p_info->vcp_info.vcs_info.change_counter;
    param.volume_setting = volume_setting;
    if (BT_STATUS_SUCCESS != (ret = ble_vcp_enhance_vcs_write_volume_control_point(p_info->handle, &param))) {
        if (BT_STATUS_CONNECTION_IN_USE == ret) {
            uint32_t vcp_retry = param.opcode | (p_info->handle<<16);
            app_le_audio_vcp_retry_operate(vcp_retry);
        }
    }
    else {
        p_info->vcp_info.vcs_info.current_operate = param.opcode + 1;
    }

    LE_AUDIO_MSGLOG_I("[APP][VCP] app_le_audio_vcp_set_absolute_volume handle:%x volume:%x ret:%x", 3, p_info->handle, volume_setting, ret);
    return ret;
}

bt_status_t app_le_audio_vcp_unmute(bt_handle_t handle)
{
    ble_vcp_enhance_volume_control_param_t param;
    bt_status_t ret = BT_STATUS_FAIL;
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }

    p_info->vcp_info.vcs_info.target_mute = BLE_VCS_UNMUTE;
    if (BLE_VCS_UNMUTE == p_info->vcp_info.vcs_info.mute) {
        return BT_STATUS_FAIL;
    }

    param.opcode = BLE_VCS_OPCODE_UNMUTE;
    param.change_counter = p_info->vcp_info.vcs_info.change_counter;
    if (BT_STATUS_SUCCESS != (ret = ble_vcp_enhance_vcs_write_volume_control_point(p_info->handle, &param))) {
        if (BT_STATUS_CONNECTION_IN_USE == ret) {
            uint32_t vcp_retry = param.opcode | (p_info->handle<<16);
            app_le_audio_vcp_retry_operate(vcp_retry);
        }
    }
    else {
        p_info->vcp_info.vcs_info.current_operate = param.opcode + 1;
    }

    LE_AUDIO_MSGLOG_I("[APP][VCP] app_le_audio_vcp_unmute, handle:%x ret:%x", 2, p_info->handle, ret);
    return ret;
}

bt_status_t app_le_audio_vcp_mute(bt_handle_t handle)
{
    ble_vcp_enhance_volume_control_param_t param;
    bt_status_t ret = BT_STATUS_FAIL;
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }
    p_info->vcp_info.vcs_info.target_mute = BLE_VCS_MUTE;
    if (BLE_VCS_MUTE == p_info->vcp_info.vcs_info.mute) {
        return BT_STATUS_FAIL;
    }

    param.opcode = BLE_VCS_OPCODE_MUTE;
    param.change_counter = p_info->vcp_info.vcs_info.change_counter;
    if (BT_STATUS_SUCCESS != (ret = ble_vcp_enhance_vcs_write_volume_control_point(p_info->handle, &param))) {
        if (BT_STATUS_CONNECTION_IN_USE == ret) {
            uint32_t vcp_retry = param.opcode | (p_info->handle<<16);
            app_le_audio_vcp_retry_operate(vcp_retry);
        }
    }
    else {
        p_info->vcp_info.vcs_info.current_operate = param.opcode + 1;
    }
    LE_AUDIO_MSGLOG_I("[APP][VCP] app_le_audio_vcp_mute, handle:%x ret:%x", 2, p_info->handle, ret);
    return ret;
}

bt_status_t app_le_audio_vcp_set_volume_state(bt_handle_t handle, ble_vcs_volume_t volume_setting, ble_vcs_mute_t mute)
{
    bt_status_t ret;
    app_le_audio_ucst_link_info_t *p_info = app_le_audio_ucst_get_link_info(handle);

    if (NULL == p_info) {
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (!app_le_audio_ucst_is_active_group(p_info->group_id)) {
        return BT_STATUS_FAIL;
    }
#endif

    if (volume_setting != p_info->vcp_info.vcs_info.volume_setting) {
        ret = app_le_audio_vcp_set_absolute_volume(handle, volume_setting);
        p_info->vcp_info.vcs_info.target_mute = mute;
        return ret;
    }

    if (mute != p_info->vcp_info.vcs_info.mute) {
        if (BLE_VCS_MUTE == mute) {
            return app_le_audio_vcp_mute(handle);
        }
        else {
            return app_le_audio_vcp_unmute(handle);
        }
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t app_le_audio_vcp_control_active_device_volume(app_le_audio_vcp_operate_t operate, void *parameter)
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
                case APP_LEA_VCP_OPERATE_RELATIVE_VOLUME_DOWN: {
                    ret = app_le_audio_vcp_relative_volume_down(p_info->handle, false);
                    break;
                }
                case APP_LEA_VCP_OPERATE_RELATIVE_VOLUME_UP: {
                    ret = app_le_audio_vcp_relative_volume_up(p_info->handle, false);
                    break;
                }
                case APP_LEA_VCP_OPERATE_UNMUTE_RELATIVE_VOLUME_DOWN: {
                    ret = app_le_audio_vcp_relative_volume_down(p_info->handle, true);
                    break;
                }
                case APP_LEA_VCP_OPERATE_UNMUTE_RELATIVE_VOLUME_UP: {
                    ret = app_le_audio_vcp_relative_volume_up(p_info->handle, true);
                    break;
                }
                case APP_LEA_VCP_OPERATE_SET_ABSOLUTE_VOLUME: {
                    ble_vcs_volume_t volume = *((ble_vcs_volume_t *)parameter);
                    ret = app_le_audio_vcp_set_absolute_volume(p_info->handle, volume);
                    break;
                }
                case APP_LEA_VCP_OPERATE_UNMUTE: {
                    ret = app_le_audio_vcp_unmute(p_info->handle);
                    break;
                }
                case APP_LEA_VCP_OPERATE_MUTE: {
                    ret = app_le_audio_vcp_mute(p_info->handle);
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
bt_status_t app_le_audio_vcp_control_device_volume(app_le_audio_vcp_operate_t operate, void *parameter)
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
        if ((BT_HANDLE_INVALID != p_info->handle)) {
            switch (operate) {
                case APP_LEA_VCP_OPERATE_RELATIVE_VOLUME_DOWN: {
                    ret = app_le_audio_vcp_relative_volume_down(p_info->handle, false);
                    break;
                }
                case APP_LEA_VCP_OPERATE_RELATIVE_VOLUME_UP: {
                    ret = app_le_audio_vcp_relative_volume_up(p_info->handle, false);
                    break;
                }
                case APP_LEA_VCP_OPERATE_UNMUTE_RELATIVE_VOLUME_DOWN: {
                    ret = app_le_audio_vcp_relative_volume_down(p_info->handle, true);
                    break;
                }
                case APP_LEA_VCP_OPERATE_UNMUTE_RELATIVE_VOLUME_UP: {
                    ret = app_le_audio_vcp_relative_volume_up(p_info->handle, true);
                    break;
                }
                case APP_LEA_VCP_OPERATE_SET_ABSOLUTE_VOLUME: {
                    ble_vcs_volume_t volume = *((ble_vcs_volume_t *)parameter);
                    ret = app_le_audio_vcp_set_absolute_volume(p_info->handle, volume);
                    break;
                }
                case APP_LEA_VCP_OPERATE_UNMUTE: {
                    ret = app_le_audio_vcp_unmute(p_info->handle);
                    break;
                }
                case APP_LEA_VCP_OPERATE_MUTE: {
                    ret = app_le_audio_vcp_mute(p_info->handle);
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

