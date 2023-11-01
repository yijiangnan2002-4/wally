
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
 * File: app_va_xiaoai_hfp_at_cmd.c
 *
 * Description: This file provides XiaoAI HFP AT CMD for XiaoAI activity.
 *
 */

#ifdef AIR_XIAOAI_ENABLE

#include "app_va_xiaoai_bt_helper.h"

#include "app_bt_state_service.h"
#include "apps_config_key_remapper.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "app_hfp_utils.h"
#include "bt_callback_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_connection_manager.h"
#include "bt_device_manager.h"
#include "bt_gap.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_music.h"
#include "FreeRTOS.h"

#include "xiaoai.h"

#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#endif
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_lea_service.h"
#endif

#define LOG_TAG           "[XIAOAI_BT]"

static bt_gap_connection_handle_t g_xiaoai_exiting_sniff_gap_handle = 0;
static uint8_t *g_xiaoai_unbind_addr1 = NULL;
static uint8_t *g_xiaoai_unbind_addr2 = NULL;

#if defined(AIR_MULTI_POINT_ENABLE)
/* For XiaoAI EMP Audio Active. */
extern bt_status_t bt_sink_srv_set_must_play_tone_flag(bt_bd_addr_t *sp_addr, bt_sink_srv_notification_voice_t type, bool is_open);
#endif

static bt_status_t xiaoai_bt_gap_evt_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    switch (msg) {
        case BT_GAP_SNIFF_MODE_CHANGE_IND: {
            bt_gap_sniff_mode_changed_ind_t *ind = (bt_gap_sniff_mode_changed_ind_t *)buffer;
            APPS_LOG_MSGID_I(LOG_TAG" SNIFF_MODE_CHANGE handle=0x%08X->0x%08X sniff_status=%d",
                             3, g_xiaoai_exiting_sniff_gap_handle, ind->handle, ind->sniff_status);
            if (g_xiaoai_exiting_sniff_gap_handle == ind->handle
                && ind->sniff_status == BT_GAP_LINK_SNIFF_TYPE_ACTIVE) {
                xiaoai_bt_switch_sniff_mode(FALSE);
            }
            break;
        }
        case BT_GAP_WRITE_LINK_POLICY_CNF: {
            bt_gap_write_link_policy_cnf_t *cnf = (bt_gap_write_link_policy_cnf_t *)buffer;
            APPS_LOG_MSGID_I(LOG_TAG" LINK_POLICY_CNF status=0x%08X sniff_mode=%d",
                             2, status, cnf->sniff_mode);
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static void xiaoai_bt_unbind_action(uint8_t *addr)
{
    if (addr != NULL) {
        bt_status_t bt_status = bt_device_manager_delete_paired_device((bt_bd_addr_ptr_t)addr);
        APPS_LOG_MSGID_I(LOG_TAG" unbind_action bt_status=0x%08X %02X:%02X:%02X:%02X:%02X:%02X",
                         7, bt_status, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    }
}

static void xiaoai_bt_discoverable_action()
{
    bt_status_t bt_status = bt_cm_discoverable(TRUE);
    APPS_LOG_MSGID_I(LOG_TAG" discoverable_action bt_status=0x%08X",
                     1, bt_status);
}

#if defined(AIR_MULTI_POINT_ENABLE)
/* TRUE - Pause other phone, FALSE - Resume other phone. */
void xiaoai_bt_music_control_other_phone_music(bool flag)
{
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    xiaoai_connection_state xiaoai_state = conn_info.conn_state;
    bool is_ble = conn_info.is_ble;
    APPS_LOG_MSGID_I(LOG_TAG"[EMP_Audio] control_other_phone_music xiaoai_state=%d is_ble=%d flag=%d",
                     3, xiaoai_state, is_ble, flag);

    /* XiaoAI cannot get phone EDR addr if XiaoAI connected via BLE. */
    if (xiaoai_state == XIAOAI_STATE_CONNECTED) {
        bt_status_t status = BT_STATUS_SUCCESS;
        if (is_ble) {
            status = bt_sink_srv_set_must_play_tone_flag(NULL,
                                                         BT_SINK_SRV_XIAOAI_NOTIFICATION_VOICE, flag);
        } else {
            status = bt_sink_srv_set_must_play_tone_flag(&(conn_info.phone_addr),
                                                         BT_SINK_SRV_XIAOAI_NOTIFICATION_VOICE, flag);
        }
        APPS_LOG_MSGID_I(LOG_TAG"[EMP_Audio] control_other_phone_music status=0x%08X",
                         1, status);
    } else {
        APPS_LOG_MSGID_I(LOG_TAG"[EMP_Audio] control_other_phone_music fail", 0);
    }
}
#endif

void xiaoai_bt_hfp_va_activate(bool enable)
{
    bool ret = app_hfp_set_va_enable(enable);
    APPS_LOG_MSGID_I(LOG_TAG" [WakeUp] hf_va_activate enable=%d ret=%d",
                     2, enable, ret);
}

void xiaoai_bt_reconnect_edr()
{
    uint8_t *reconnect_addr = NULL;
    bool is_emp_enable = FALSE;
#ifdef AIR_MULTI_POINT_ENABLE
    is_emp_enable = app_bt_emp_is_enable();
#endif
    bt_device_manager_paired_infomation_t paired_info[2];
    uint32_t paired_info_count = 2;
    bt_device_manager_get_paired_list(paired_info, &paired_info_count);
    APPS_LOG_MSGID_I(LOG_TAG" action_in_app -  Reconnect EDR is_emp_enable=%d paired_info_count=%d ",
                     2, is_emp_enable, paired_info_count);
    if (is_emp_enable) {
        for (int i = 0; i < paired_info_count; i++) {
            bt_cm_profile_service_mask_t mask = bt_cm_get_connected_profile_services(paired_info[i].address);
            uint8_t *addr = (uint8_t *)paired_info[i].address;
            APPS_LOG_MSGID_I(LOG_TAG" action_in_app -  Reconnect EDR %02X:%02X:%02X:%02X:%02X:%02X mask=0x%08X ",
                             7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], mask);
            if (mask == BT_CM_PROFILE_SERVICE_MASK_NONE) {
                reconnect_addr = (uint8_t *)paired_info[i].address;
                // only reconnect one unconnected device
                break;
            }
        }
    } else {
        if (paired_info_count > 0) {
#if 1
            bt_cm_profile_service_mask_t mask = bt_cm_get_connected_profile_services(paired_info[0].address);
            uint8_t *addr = (uint8_t *)paired_info[0].address;
            APPS_LOG_MSGID_I(LOG_TAG" action_in_app -  Reconnect EDR %02X:%02X:%02X:%02X:%02X:%02X mask=0x%08X ",
                             7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], mask);
            if (mask == BT_CM_PROFILE_SERVICE_MASK_NONE) {
                reconnect_addr = (uint8_t *)paired_info[0].address;
            }
#else
            reconnect_addr = xiaoai_get_virtual_addr_mapping_result();
            uint8_t *addr = (uint8_t *)reconnect_addr;
            APPS_LOG_MSGID_I(LOG_TAG" action_in_app -  Reconnect EDR (virtual_addr_mapping) %02X:%02X:%02X:%02X:%02X:%02X mask=0x%08X ",
                             6, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
#endif
        }
    }

    if (reconnect_addr != NULL) {
        bt_cm_connect_t connect_param = { {0},
            BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)
            | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK)
            | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP)
        };
        memcpy(connect_param.address, reconnect_addr, sizeof(bt_bd_addr_t));
        bt_status_t bt_status = bt_cm_connect(&connect_param);
        APPS_LOG_MSGID_I(LOG_TAG" action_in_app -  Reconnect EDR status=0x%08X",
                         1, bt_status);
    }
}

void xiaoai_bt_disconnect_edr(bool is_ble, uint8_t *xiaoai_addr, bool need_unbind)
{
    if (need_unbind) {
        if (g_xiaoai_unbind_addr1 != NULL) {
            vPortFree(g_xiaoai_unbind_addr1);
            g_xiaoai_unbind_addr1 = NULL;
        }
        if (g_xiaoai_unbind_addr2 != NULL) {
            vPortFree(g_xiaoai_unbind_addr2);
            g_xiaoai_unbind_addr2 = NULL;
        }
    }

#if defined(AIR_MULTI_POINT_ENABLE)
    APPS_LOG_MSGID_I(LOG_TAG" disconnect_action is_ble=%d need_unbind=%d",
                     2, is_ble, need_unbind);
    if (is_ble) {
        // Disconnect all phones for BLE & Multi-point
        bt_device_manager_paired_infomation_t paired_info[2];
        uint32_t paired_info_count = 2;
        bt_device_manager_get_paired_list(paired_info, &paired_info_count);
        if (paired_info_count > 0) {
            for (int i = 0; i < paired_info_count; i++) {
                bt_cm_connect_t connect_param;
                memcpy(&connect_param.address, &paired_info[i].address, sizeof(bt_bd_addr_t));
                connect_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
                bt_status_t bt_status = bt_cm_disconnect(&connect_param);
                APPS_LOG_MSGID_I(LOG_TAG" disconnect_action status=0x%08X",
                                 1, bt_status);
                if (need_unbind && bt_status == BT_STATUS_SUCCESS) {
                    if (i == 0) {
                        g_xiaoai_unbind_addr1 = (uint8_t *)pvPortMalloc(BT_BD_ADDR_LEN);
                        if (g_xiaoai_unbind_addr1 != NULL) {
                            memcpy(g_xiaoai_unbind_addr1, &paired_info[i].address, BT_BD_ADDR_LEN);
                            uint8_t *addr = g_xiaoai_unbind_addr1;
                            APPS_LOG_MSGID_I(LOG_TAG" disconnect_action addr1=%02X:%02X:%02X:%02X:%02X:%02X",
                                             6, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
                        }
                    } else if (i == 1) {
                        g_xiaoai_unbind_addr2 = (uint8_t *)pvPortMalloc(BT_BD_ADDR_LEN);
                        if (g_xiaoai_unbind_addr2 != NULL) {
                            memcpy(g_xiaoai_unbind_addr2, &paired_info[i].address, BT_BD_ADDR_LEN);
                            uint8_t *addr = g_xiaoai_unbind_addr2;
                            APPS_LOG_MSGID_I(LOG_TAG" disconnect_action addr2=%02X:%02X:%02X:%02X:%02X:%02X",
                                             6, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
                        }
                    }
                }
            }
        }
    } else if (xiaoai_addr != NULL) {
        bt_cm_connect_t connect_param;
        memcpy(&connect_param.address, xiaoai_addr, sizeof(bt_bd_addr_t));
        connect_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
        bt_status_t bt_status = bt_cm_disconnect(&connect_param);
        APPS_LOG_MSGID_I(LOG_TAG" disconnect_action status=0x%08X",
                         1, bt_status);
        if (need_unbind && bt_status == BT_STATUS_SUCCESS) {
            g_xiaoai_unbind_addr1 = (uint8_t *)pvPortMalloc(BT_BD_ADDR_LEN);
            g_xiaoai_unbind_addr2 = NULL;
            if (g_xiaoai_unbind_addr1 != NULL) {
                memcpy(g_xiaoai_unbind_addr1, xiaoai_addr, BT_BD_ADDR_LEN);
                uint8_t *addr = g_xiaoai_unbind_addr1;
                APPS_LOG_MSGID_I(LOG_TAG" disconnect_action addr1=%02X:%02X:%02X:%02X:%02X:%02X",
                                 6, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
            }
        }
    }
#else
    extern bt_bd_addr_t *bt_sink_srv_cm_last_connected_device(void);
    bt_bd_addr_t *bt_bd_addr = bt_sink_srv_cm_last_connected_device();
    if (bt_bd_addr != NULL) {
        bt_cm_connect_t connect_param;
        memcpy(&connect_param.address, *bt_bd_addr, sizeof(bt_bd_addr_t));
        connect_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
        bt_status_t bt_status = bt_cm_disconnect(&connect_param);
        APPS_LOG_MSGID_I(LOG_TAG" disconnect_action status=0x%08X",
                         1, bt_status);
        if (need_unbind && bt_status == BT_STATUS_SUCCESS) {
            g_xiaoai_unbind_addr1 = (uint8_t *)pvPortMalloc(BT_BD_ADDR_LEN);
            g_xiaoai_unbind_addr2 = NULL;
            if (g_xiaoai_unbind_addr1 != NULL) {
                memcpy(g_xiaoai_unbind_addr1, *bt_bd_addr, BT_BD_ADDR_LEN);
            }
        }
    }
#endif
}

void xiaoai_bt_unbind_start(bool is_ble)
{
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    xiaoai_connection_state conn_state = conn_info.conn_state;
    uint8_t *addr = conn_info.phone_addr;
    APPS_LOG_MSGID_I(LOG_TAG" action_in_app - unbind is_ble=%d conn_state=%d %02X:%02X:%02X:%02X:%02X:%02X",
                     8, is_ble, conn_state, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    if (is_ble) {
        xiaoai_bt_disconnect_edr(TRUE, NULL, TRUE);
    } else {
#if 0
        // EDR will be disconnected by Android XiaoAI APP
        xiaoai_bt_disconnect_edr(FALSE, addr, TRUE);
#else
        g_xiaoai_unbind_addr1 = (uint8_t *)pvPortMalloc(BT_BD_ADDR_LEN);
        g_xiaoai_unbind_addr2 = NULL;
        if (g_xiaoai_unbind_addr1 != NULL) {
            memcpy(g_xiaoai_unbind_addr1, addr, BT_BD_ADDR_LEN);
        }
#endif
    }
}

void xiaoai_bt_check_unbind(void *bt_cm_remote_info_update_ind)
{
    bt_cm_remote_info_update_ind_t *ind = (bt_cm_remote_info_update_ind_t *) bt_cm_remote_info_update_ind;
    if (ind != NULL && ind->acl_state == 0) {
        uint8_t *addr = (uint8_t *) ind->address;
        if (g_xiaoai_unbind_addr1 != NULL
            && memcmp(g_xiaoai_unbind_addr1, addr, BT_BD_ADDR_LEN) == 0) {
            xiaoai_bt_unbind_action(g_xiaoai_unbind_addr1);
            vPortFree(g_xiaoai_unbind_addr1);
            g_xiaoai_unbind_addr1 = NULL;
            if (g_xiaoai_unbind_addr2 == NULL) {
                xiaoai_disconnect();
                xiaoai_bt_discoverable_action();
            }
        }
        if (g_xiaoai_unbind_addr2 != NULL
            && memcmp(g_xiaoai_unbind_addr2, addr, BT_BD_ADDR_LEN) == 0) {
            xiaoai_bt_unbind_action(g_xiaoai_unbind_addr2);
            vPortFree(g_xiaoai_unbind_addr2);
            g_xiaoai_unbind_addr2 = NULL;
            if (g_xiaoai_unbind_addr1 == NULL) {
                xiaoai_disconnect();
                xiaoai_bt_discoverable_action();
            }
        }
    }
}

bool xiaoai_bt_is_visible(void)
{
    bt_bd_addr_t *last_addr = bt_device_manager_remote_get_dev_by_seq_num(1);
    bool is_visible = app_bt_service_is_visible();
    APPS_LOG_MSGID_I(LOG_TAG" is_visible, last_addr=0x%08X is_visible=%d",
                     2, last_addr, is_visible);
    return (last_addr == NULL || is_visible);
}

void xiaoai_bt_allow_new_conn(bool allow)
{
    APPS_LOG_MSGID_I(LOG_TAG" allow_new_conn, allow=%d", 1, allow);
    if (allow) {
        bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
        app_lea_service_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, TRUE, 0);
    } else {
        bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_DISABLE);
        app_lea_service_stop_advertising(TRUE);
    }
}

void xiaoai_bt_exit_sniff_mode()
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_status_t status = BT_STATUS_FAIL;
    if (role == BT_AWS_MCE_ROLE_AGENT && g_xiaoai_exiting_sniff_gap_handle == 0) {
        bt_bd_addr_t bd_addr = {0};
        uint32_t num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), &bd_addr, 1);
        uint8_t *addr = (uint8_t *)&bd_addr;
        APPS_LOG_MSGID_I(LOG_TAG" exit_sniff aws_num=%d %02X:%02X:%02X:%02X:%02X:%02X",
                         7, num, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
        if (num == 1) {
            bt_gap_connection_handle_t handle = bt_cm_get_gap_handle(bd_addr);
            status = bt_gap_exit_sniff_mode(handle);
            APPS_LOG_MSGID_I(LOG_TAG" exit_sniff handle=0x%08X status=0x%08X",
                             2, handle, status);
            if (status == BT_STATUS_SUCCESS || status == BT_STATUS_PENDING) {
                g_xiaoai_exiting_sniff_gap_handle = handle;
            } else {
                xiaoai_bt_switch_sniff_mode(FALSE);
            }
        }
    } else {
        APPS_LOG_MSGID_E(LOG_TAG" exit_sniff fail, role=%02X handle=0x%08X",
                         2, role, g_xiaoai_exiting_sniff_gap_handle);
    }
}

void xiaoai_bt_switch_sniff_mode(bool enable)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (role == BT_AWS_MCE_ROLE_AGENT) {
        bt_bd_addr_t bd_addr = {0};
        uint32_t num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), &bd_addr, 1);
        uint8_t *addr = (uint8_t *)&bd_addr;
        APPS_LOG_MSGID_I(LOG_TAG" switch_sniff aws_num=%d %02X:%02X:%02X:%02X:%02X:%02X",
                         7, num, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
        if (num == 1) {
            bt_gap_connection_handle_t handle = bt_cm_get_gap_handle(bd_addr);
            bt_gap_link_policy_setting_t setting;
            if (enable) {
                setting.sniff_mode = BT_GAP_LINK_POLICY_ENABLE;
            } else {
                setting.sniff_mode = BT_GAP_LINK_POLICY_DISABLE;
            }
            bt_status_t status = bt_gap_write_link_policy(handle, &setting);
            APPS_LOG_MSGID_I(LOG_TAG" switch_sniff, enable=%d status=0x%08X",
                             2, enable, status);
        }
    } else {
        APPS_LOG_MSGID_E(LOG_TAG" switch_sniff fail, role=%02X", 1, role);
    }
    g_xiaoai_exiting_sniff_gap_handle = 0;
}

void xiaoai_bt_helper_init(bool enable)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (enable) {
        status = bt_callback_manager_register_callback(bt_callback_type_app_event,
                                                       MODULE_MASK_GAP, (void *)xiaoai_bt_gap_evt_callback);
    } else {
        status = bt_callback_manager_deregister_callback(bt_callback_type_app_event,
                                                         (void *)xiaoai_bt_gap_evt_callback);
    }
    APPS_LOG_MSGID_I(LOG_TAG" init enable=%d status=0x%08X", 2, enable, status);
}


#endif /* AIR_XIAOAI_ENABLE */
