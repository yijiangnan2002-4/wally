
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
 * File: app_va_xiaoai_activity.c
 *
 * Description: This file is XiaoAI idle activity.
 *
 */

#ifdef AIR_XIAOAI_ENABLE

#include "app_va_xiaoai_activity.h"
#include "apps_config_event_list.h"
#include "apps_aws_sync_event.h"
#include "app_bt_takeover_service.h"
#include "app_bt_state_service.h"
#include "apps_events_bt_event.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_config_key_remapper.h"
#include "apps_config_vp_index_list.h"
#include "voice_prompt_api.h"
#include "app_rho_idle_activity.h"
#include "apps_debug.h"
#include "ui_shell_manager.h"

#include "bt_aws_mce_report.h"
#include "bt_customer_config.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_ami.h"
#include "bt_sink_srv_common.h"
#include "bt_sink_srv_music.h"
#include "bt_connection_manager.h"
#include "bt_device_manager_power.h"

#include "xiaoai.h"
#include "xiaoai_port_dvfs.h"

#include "multi_ble_adv_manager.h"
#include "app_va_xiaoai_ble_adv.h"
#include "app_va_xiaoai_bt_helper.h"
#include "app_va_xiaoai_config.h"
#include "app_va_xiaoai_device_config.h"
#include "app_va_xiaoai_hfp_at_cmd.h"
#include "app_va_xiaoai_ota.h"
#ifdef AIR_XIAOAI_PTT_ENABLE
#include "app_va_xiaoai_ptt_speech_activity.h"
#endif
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
#include "app_va_xiaoai_miui_fast_connect.h"
#endif
#ifdef AIR_SMART_CHARGER_ENABLE
#include "app_smcharger_utils.h"
#endif
#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#endif
#ifdef MTK_FOTA_ENABLE
#include "app_fota_idle_activity.h"
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_utils.h"
#endif
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_lea_service.h"
#endif
#include "app_fm_activity.h"

/* Default enable XiaoAI AT CMD for test. */
#define APP_XIAOAI_TEST

#ifdef APP_XIAOAI_TEST
#include "atci.h"

static atci_status_t app_xiaoai_atci_handler(atci_parse_cmd_param_t *parse_cmd);
static atci_cmd_hdlr_item_t app_xiaoai_atci_cmd[] = {
    {
        .command_head = "AT+XIAOAI",
        .command_hdlr = app_xiaoai_atci_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};
#endif

#define LOG_TAG           "[XIAOAI_VA]"

static bool g_xiaoai_init_done = FALSE;         /* XiaoAI init flag. */
static xiaoai_speech_state g_xiaoai_speech_state = XIAOAI_SPEECH_STATE_NONE;    /* XiaoAI Speech state. */
static xiaoai_session_state g_xiaoai_session_state = XIAOAI_SESSION_STATE_IDLE;    /* XiaoAI Session state - opcode 0x10 bit0. */
static bool g_xiaoai_aws_connected = FALSE;         /* AWS MCE connection state. */
static bool g_xiaoai_acl_connected = FALSE;         /* EDR(ACL) connection state. */
static bool g_xiaoai_edr_visible   = FALSE;         /* EDR discoverable mode. */

static bool g_xiaoai_mma_notify_pre_aws_state = FALSE;

extern void xiaoai_app_send_conn_state_to_peer();
extern void app_va_xiaoai_notify_leakage_detectable(bool detectable);

bool xiaoai_app_aws_is_connected()
{
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    return (aws_link_type != BT_AWS_MCE_SRV_LINK_NONE);
}

bool app_xiaoai_is_lea_mma_link(void)
{
    bool ret = FALSE;
#ifdef AIR_LE_AUDIO_ENABLE
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    if (conn_info.conn_state == XIAOAI_STATE_CONNECTED
        && conn_info.is_le_audio_mma) {
        ret = TRUE;
        //APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK]", 0);
    }
#endif
    return ret;
}

uint8_t g_xiaoai_ota_state = XIAOAI_OTA_STATE_STOP;

// workaround for iPhone XiaoAI APP 0x0F->0x06 when "remove device", avoid call BT_CM disconnect twice.
static bool g_xiaoai_unbind_flow_starting = FALSE;

#ifdef AIR_XIAOAI_WWE_ENABLE
#include "record_control.h"
#include "FreeRTOS.h"
#include "semphr.h"

#define XIAOAI_USE_ATCMD_TEST_WWE

static record_id_t g_xiaoai_wwe_record_id = -1;
static bool g_xiaoai_wwe_error_flag = FALSE;
static SemaphoreHandle_t g_xiaoai_wwe_semaphore;
static bool g_xiaoai_wwe_need_restart = FALSE;

#define XIAOAI_WWE_RECORDER_MAX_DELAY_TIME          (10 * 1000)

static void app_va_xiaoai_recorder_event_notify(xiaoai_va_recorder_event recorder_event);
static void app_va_xiaoai_wwe_update_state(xiaoai_wwe_state new_state);
static void xiaoai_wwe_recorder_init();
static bool xiaoai_wwe_recorder_start();
static bool xiaoai_wwe_recorder_stop();

#endif



/**
 *  @brief This enum defines the start reason of XiaoAI BLE ADV.
 */
typedef enum {
    XIAOAI_BLE_ADV_REASON_XIAOAI_CONN = 0,      /* Start XiaoAI BLE ADV due to XiaoAI connection changed. */
    XIAOAI_BLE_ADV_REASON_BOTH_LID_CHARGER_IN,  /* Start XiaoAI BLE ADV due to Agent and Partner charger in. */
    XIAOAI_BLE_ADV_REASON_LID_OPEN,             /* Start XiaoAI BLE ADV due to Agent and Partner open lid. */
    XIAOAI_BLE_ADV_REASON_LID_CLOSE,            /* Start XiaoAI BLE ADV due to Agent and Partner close lid. */
    XIAOAI_BLE_ADV_REASON_CHARGER_IN_OUT,
    XIAOAI_BLE_ADV_REASON_MIUI_FAST_CONNECT
} xiaoai_start_ble_adv_reason;

/**
 * @brief      This function is used to check and send event to start XiaoAI BLE ADV.
 * @param[in]  reason, the start reason of XiaoAI BLE ADV.
 * @return     TRUE, send XiaoAI event to start BLE ADV.
 */
static bool app_va_xiaoai_check_and_send_ble_adv_event(xiaoai_start_ble_adv_reason reason);

static bool app_va_xiaoai_is_call_ongoing(void)
{
    bool ret = FALSE;
    bt_sink_srv_device_state_t state_list[BT_SINK_SRV_MAX_DEVICE_NUM] = {0};
    bt_sink_srv_get_device_state(NULL, state_list, BT_SINK_SRV_MAX_DEVICE_NUM);
    for (int i = 0; i < BT_SINK_SRV_MAX_DEVICE_NUM; i++) {
        if (state_list[i].sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED
            || state_list[i].call_state >= BT_SINK_SRV_STATE_INCOMING) {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

#ifdef AIR_MULTI_POINT_ENABLE
bool app_xiaoai_emp_switch_allow_callback(bool need_enable, bt_bd_addr_t *keep_phone_addr)
{
    bool allow = TRUE;
    uint8_t conn_num = xiaoai_port_get_conn_num(FALSE);
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    xiaoai_connection_state xiaoai_state = conn_info.conn_state;
    bool is_le_audio_mma = conn_info.is_le_audio_mma;
    bool is_ble = conn_info.is_ble;
    if (!need_enable) {
        if (conn_num > 1 && is_ble && !is_le_audio_mma && xiaoai_state == XIAOAI_STATE_CONNECTED) {
            allow = FALSE;
        }
    }
    APPS_LOG_MSGID_I(LOG_TAG"[EMP] emp_switch_allow, need_enable=%d allow=%d conn_num=%d is_ble=%d is_le_audio_mma=%d",
                     5, need_enable, allow, conn_num, is_ble, is_le_audio_mma);
    return allow;
}
#endif

bool xiaoai_notify_sp_status(uint8_t notify_type, uint8_t data)
{
    bool success = FALSE;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    xiaoai_connection_state conn_state = xiaoai_get_connection_state();
    bool peer_lea_mma = xiaoai_peer_is_lea_mma_link();
    APPS_LOG_MSGID_I(LOG_TAG" notify_sp_status, [%02X] notify_type=%d data=%d conn_state=%d peer_lea_mma=%d",
                     5, role, notify_type, data, conn_state, peer_lea_mma);

    if (conn_state != XIAOAI_STATE_CONNECTED && peer_lea_mma) {
        xiaoai_notify_sp_param_t param = {0};
        param.notify_type = notify_type;
        param.data = data;
        bt_status_t bt_status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_XIAOAI,
                                                               XIAOAI_EVENT_LEA_SYNC_NOTIFY_SP_TO_PEER,
                                                               (uint8_t *)&param,
                                                               sizeof(xiaoai_notify_sp_param_t));
        APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK][%02X] aws_send notify_sp_param, bt_status=0x%08X",
                         2, role, bt_status);
        return (bt_status == BT_STATUS_SUCCESS);
    }

    if (notify_type == XIAOAI_APP_NOTIFY_AWS_STATUS) {
        success = xiaoai_notify_aws_status(data);
    } else if (notify_type == XIAOAI_APP_NOTIFY_EDR_STATUS) {
        success = xiaoai_notify_edr_status(data);
    } else if (notify_type == XIAOAI_APP_NOTIFY_ANC_STATUS) {
        success = xiaoai_notify_anc_status(data);
    } else if (notify_type == XIAOAI_APP_NOTIFY_BATTERY_STATUS) {
        success = xiaoai_notify_battery_report();
    }
    return success;
}

static void app_va_xiaoai_aws_action(bool aws_connected)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bool lea_mma = app_xiaoai_is_lea_mma_link();
    g_xiaoai_aws_connected = aws_connected;
    APPS_LOG_MSGID_I(LOG_TAG" AWS Action, [%02X] aws_connected=%d lea_mma=%d",
                     3, role, aws_connected, lea_mma);

    if (role == BT_AWS_MCE_ROLE_PARTNER && !lea_mma) {
        return;
    }

    xiaoai_notify_sp_status(XIAOAI_APP_NOTIFY_AWS_STATUS, aws_connected);

    /* Start XiaoAI BLE ADV when AWS connected (such as LID_OPEN case). */
    /* May stop XiaoAI BLE ADV when AWS disconnected. */
    //app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_XIAOAI_CONN);

    bool miui_ret = app_va_xiaoai_hfp_miui_basic_atcmd();
    bool mma_ret = xiaoai_notify_sp_status(XIAOAI_APP_NOTIFY_BATTERY_STATUS, 0);
    if (miui_ret || mma_ret) {
        g_xiaoai_mma_notify_pre_aws_state = aws_connected;
    }
}

/**
 * @brief      The XiaoAI APP callback function is used to notify XiaoAI speech timeout.
 */
static void app_va_xiaoai_speech_timeout()
{
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    xiaoai_connection_state xiaoai_state = conn_info.conn_state;
    bool is_ble = conn_info.is_ble;

    uint8_t phone_addr[6] = {0};
    bt_cm_profile_service_state_t hfp_state = 0;
    if (xiaoai_state == XIAOAI_STATE_CONNECTED && !is_ble) {
        memcpy(phone_addr, conn_info.phone_addr, 6);
        hfp_state = bt_cm_get_profile_service_state(phone_addr, BT_CM_PROFILE_SERVICE_HFP);
    } else {
        // don't care multi-point
        uint32_t result = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP), (bt_bd_addr_t *)phone_addr, 1);
        if (result == 1) {
            hfp_state = BT_CM_PROFILE_SERVICE_STATE_CONNECTED;
        } else {
            hfp_state = BT_CM_PROFILE_SERVICE_STATE_DISCONNECTED;
        }
    }

    /* Customer configure option: could ignore is_mi_phone, support iPhone siri or other Phone voice assistant. */
    bool is_mi_phone = TRUE; // xiaoai_is_mi_phone(phone_addr);
    APPS_LOG_MSGID_I(LOG_TAG" [WakeUp] speech_timeout xiaoai_state=%d hfp_state=%d phone_addr=%08X%04X (mi=%d)",
                     5, xiaoai_state, hfp_state, *((uint32_t *)(phone_addr + 2)), *((uint16_t *)phone_addr),
                     is_mi_phone);
    /* Activate HFP SCO VA if HFP connected and phone type is MI phone. */
    if (hfp_state == BT_CM_PROFILE_SERVICE_STATE_CONNECTED
        && is_mi_phone) {
        APPS_LOG_MSGID_I(LOG_TAG" [WakeUp] speech_timeout - AT+BVRA", 0);
        xiaoai_bt_hfp_va_activate(TRUE);
    } else {
        /* Customer need to add own VP: "Please open XiaoAI APP and connect me". */
    }
}

/**
 * @brief      The XiaoAI APP callback function is used to check and update BLE ADV.
 * @param[in]  charger_state, XiaoAI devices charger state.
 */
static void app_va_xiaoai_handle_battery_charger_change(xiaoai_bat_charger_state charger_state)
{
    xiaoai_connection_state conn_state = xiaoai_get_connection_state();
    bool lea_mma = app_xiaoai_is_lea_mma_link();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bool aws_connected = (bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                                                      NULL, 0) > 0);
    APPS_LOG_MSGID_I(LOG_TAG" handle_battery_charger_change [%02X] lea_mma=%d BAT_changed charger_state=%d conn_state=%d aws_connected=%d",
                     5, role, lea_mma, charger_state, conn_state, aws_connected);

    /* For slience OTA. */
    if (charger_state >= XIAOAI_BOTH_CHARGER_IN) {
        app_va_xiaoai_ota_check_both_close(charger_state == XIAOAI_BOTH_LID_CLOSE);
    }

    if ((!lea_mma && role != BT_AWS_MCE_ROLE_AGENT) || conn_state == XIAOAI_STATE_NONE) {
        APPS_LOG_MSGID_I(LOG_TAG" handle_battery_charger_change, ignore", 0);
        return;
    }

    /* Only one headset charger in or out. */
    if (charger_state == XIAOAI_CHARGER_STATE_CHANGED) {
        bool miui_ret = app_va_xiaoai_hfp_miui_basic_atcmd();
        bool mma_ret = xiaoai_notify_sp_status(XIAOAI_APP_NOTIFY_BATTERY_STATUS, 0);
        if (miui_ret || mma_ret) {
            g_xiaoai_mma_notify_pre_aws_state = aws_connected;
        }
        /* Update BLE ADV due to SmartCharger reason. */
        app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_CHARGER_IN_OUT);
    } else if (charger_state == XIAOAI_BATTERY_CHANGED) {
        bool miui_ret = app_va_xiaoai_hfp_miui_basic_atcmd();
        bool mma_ret = xiaoai_notify_sp_status(XIAOAI_APP_NOTIFY_BATTERY_STATUS, 0);
        if (miui_ret || mma_ret) {
            g_xiaoai_mma_notify_pre_aws_state = aws_connected;
        }
    } else {
        xiaoai_start_ble_adv_reason reason = 0;
        if (charger_state == XIAOAI_BOTH_CHARGER_IN) {
            reason = XIAOAI_BLE_ADV_REASON_BOTH_LID_CHARGER_IN;
            //xiaoai_update_ble_adv_broadcast_count();
        } else if (charger_state == XIAOAI_BOTH_LID_OPEN) {
            reason = XIAOAI_BLE_ADV_REASON_LID_OPEN;
            xiaoai_update_ble_adv_broadcast_count();
        } else if (charger_state == XIAOAI_BOTH_LID_CLOSE) {
            reason = XIAOAI_BLE_ADV_REASON_LID_CLOSE;
            //xiaoai_update_ble_adv_broadcast_count();
            bool miui_ret = app_va_xiaoai_hfp_miui_basic_atcmd();
            if (miui_ret) {
                g_xiaoai_mma_notify_pre_aws_state = aws_connected;
            }
        } else if (charger_state == XIAOAI_OWN_LID_OPEN) {

        } else if (charger_state == XIAOAI_OWN_LID_CLOSE) {
            reason = XIAOAI_BLE_ADV_REASON_LID_CLOSE;
            //xiaoai_update_ble_adv_broadcast_count();
        } else if (charger_state == XIAOAI_PEER_LID_OPEN) {

        } else if (charger_state == XIAOAI_PEER_LID_CLOSE) {
            reason = XIAOAI_BLE_ADV_REASON_LID_CLOSE;
            //xiaoai_update_ble_adv_broadcast_count();
            bool miui_ret = app_va_xiaoai_hfp_miui_basic_atcmd();
            if (miui_ret) {
                g_xiaoai_mma_notify_pre_aws_state = aws_connected;
            }
        }
        /* Update BLE ADV due to SmartCharger reason. */
        app_va_xiaoai_check_and_send_ble_adv_event(reason);
    }
}

/**
 * @brief      The XiaoAI APP callback function is used to handle vendor.
 * @param[in]  opcode, should be 0xF1 (default callback, MMA Vendor CMD) and 0x02,0x09 (if is_callback_mma_cmd_to_app = true).
 * @param[in]  app_type, Phone APP type - XiaoAI/MIUI etc.
 * @param[in]  opcode_sn, opcode SN.
 * @param[in]  No need to free
 *             For 0x02/0x09, data - Common response, from MMA Data (not include opcode_sn), length - data length
 *             For 0x0E, not support, 0x0E - device notify phone APP,
 *                       customer could call xiaoai_send_mma_cmd to send special 0x0E CMD to phone APP.
 * @param[in]  length, parameter length - 1 (opcode_sn)
 */
void app_va_xiaoai_handle_vendor_cmd(uint8_t opcode, uint8_t app_type, uint8_t opcode_sn,
                                     uint8_t *data, int length)
{
    // check opcode and handle data
    if (opcode == 0xF1) {
        // Customer feature
    } else if (opcode == 0x02
               || opcode == 0x09) {
        if (opcode == 0x02) {
            // handle 0x02 response data, add data for new feature
        }

        // rsp_data not include opcode_sn
        uint8_t *rsp_data = data;
        bool result = xiaoai_send_mma_rsp(opcode, app_type, XIAOAI_MMA_RSP_STATUS_SUCCESS, opcode_sn,
                                          rsp_data, length);
        APPS_LOG_MSGID_I(LOG_TAG" xiaoai_send_vendor_rsp, opCode=%02X length=%d result=%d",
                         3, opcode, length, result);
    }
}

bool app_va_xiaoai_check_ble_link(bool check_lea, uint8_t *le_conn_local_addr)
{
    bool ret = FALSE;
    bt_bd_addr_t adv_addr = {0};

    if (check_lea) {
#ifdef AIR_LE_AUDIO_ENABLE
        bool is_enable_dual_mode = app_lea_service_is_enable_dual_mode();
        if (is_enable_dual_mode) {
#ifdef MTK_AWS_MCE_ENABLE
            uint8_t *edr_addr = (uint8_t *)bt_device_manager_aws_local_info_get_fixed_address();
#else
            uint8_t *edr_addr = (uint8_t *)bt_device_manager_get_local_address();
#endif
            ret = (memcmp(le_conn_local_addr, edr_addr, BT_BD_ADDR_LEN) == 0);
            if (ret) {
                APPS_LOG_MSGID_I(LOG_TAG" check_ble_link, LEA public addr", 0);
            } else {
                APPS_LOG_MSGID_E(LOG_TAG" check_ble_link, not LEA public addr", 0);
            }
        } else {
            if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, &adv_addr, NULL)) {
                ret = (memcmp(&adv_addr, le_conn_local_addr, BT_BD_ADDR_LEN) == 0);
                APPS_LOG_MSGID_I(LOG_TAG" check_ble_link, LEA ret=%d", 1, ret);
            } else {
                APPS_LOG_MSGID_E(LOG_TAG" check_ble_link, get_lea_addr fail", 0);
            }
        }
#endif
    } else {
        if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_XIAOAI, &adv_addr, NULL)) {
            ret = (memcmp(&adv_addr, le_conn_local_addr, BT_BD_ADDR_LEN) == 0);
            APPS_LOG_MSGID_I(LOG_TAG" check_ble_link, RANDOM ret=%d", 1, ret);
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" check_ble_link, get_random_addr fail", 0);
        }
    }

    return ret;
}

void app_va_xiaoai_get_vendor_data(uint8_t **data, uint16_t *len, bool *need_free)
{
    if (data != NULL && len != NULL && need_free != NULL) {
        *len = 0;
//        *data = (uint8_t *) pvPortMalloc(100);
//        if (*data != NULL) {
//            *len = 100;
//            memset(*data, 0, *len);
//            /* Customer configure option: add your vendor data. */
//            *need_free = TRUE;
//        } else {
//            APPS_LOG_MSGID_E(LOG_TAG" get_vendor_data, malloc error", 0);
//            *len = 0;
//            *need_free = FALSE;
//        }
    } else {
        APPS_LOG_MSGID_E(LOG_TAG" get_vendor_data, parameter error", 0);
    }
}

/**
 * @brief      The function could check and wake up XiaoMI system XiaoAI APP (HFP SCO VA).
 */
static bool app_va_xiaoai_wake_up()
{
    bool hfp_va_activate = FALSE;
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    xiaoai_connection_state xiaoai_state = conn_info.conn_state;
    bool is_ble = conn_info.is_ble;

    uint8_t phone_addr[6] = {0};
    bt_cm_profile_service_state_t hfp_state = 0;
    if (xiaoai_state == XIAOAI_STATE_CONNECTED && !is_ble) {
        memcpy(phone_addr, conn_info.phone_addr, 6);
        hfp_state = bt_cm_get_profile_service_state(phone_addr, BT_CM_PROFILE_SERVICE_HFP);
    } else {
        // don't care multi-point
        uint32_t result = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP), (bt_bd_addr_t *)phone_addr, 1);
        if (result == 1) {
            hfp_state = BT_CM_PROFILE_SERVICE_STATE_CONNECTED;
        } else {
            hfp_state = BT_CM_PROFILE_SERVICE_STATE_DISCONNECTED;
        }
    }

    /* Customer configure option: could ignore is_mi_phone, support iPhone siri or other Phone voice assistant. */
    bool is_mi_phone = TRUE; // xiaoai_is_mi_phone(phone_addr);
    APPS_LOG_MSGID_I(LOG_TAG" [WakeUp] wake_up xiaoai_state=%d hfp_state=%02X phone_addr=%08X%04X (mi=%d)",
                     5, xiaoai_state, hfp_state, *((uint32_t *)(phone_addr + 2)), *((uint16_t *)phone_addr),
                     is_mi_phone);

    if (xiaoai_state == XIAOAI_STATE_CONNECTED) {
        if (hfp_state == BT_CM_PROFILE_SERVICE_STATE_CONNECTED) {
            /* Do nothing, continue to do XiaoAI VA flow. */
        } else if (hfp_state == BT_CM_PROFILE_SERVICE_STATE_CONNECTING) {
            /* Customer need to add own VP: "Awaking XiaoAI APP". */
        } else {
            /* Customer need to add own VP: "BT Disconnected, please connect me by SP BT Setting". */
        }
    } else {
        if (hfp_state == BT_CM_PROFILE_SERVICE_STATE_CONNECTED) {
            if (is_mi_phone) {
                APPS_LOG_MSGID_I(LOG_TAG" [WakeUp] wake_up - AT+BVRA", 0);
                /* Activate HFP SCO VA when XiaoAI disconnected and HFP connected in MI phone . */
                xiaoai_bt_hfp_va_activate(TRUE);
                hfp_va_activate = TRUE;
            } else {
                /* Customer need to add own VP: "Please open XiaoAI APP and connect me". */
            }
        } else {
            /* Customer need to add own VP: "BT Disconnected, please connect me by SP BT Setting". */
        }
    }
    return hfp_va_activate;
}

/**
 * @brief      The XiaoAI APP action function is callback from XiaoAI middleware, need to send event and run in UI Shell task.
 * @param[in]  action, XiaoAI APP action.
 * @param[in]  param, action parameter.
 */
static void app_va_xiaoai_action_fun(xiaoai_app_action action, void *param)
{
    ui_shell_status_t ret = UI_SHELL_STATUS_OK;
    switch (action) {
        case XIAOAI_ACTION_POWER_OFF: {
            APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai action - power_off", 0);
            ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                      EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF,
                                      NULL, 0, NULL, 0);
            break;
        }
        case XIAOAI_ACTION_REBOOT: {
            uint32_t delay = (uint32_t)param;
            APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai action - reboot", 0);
            ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                      EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_REQUEST_REBOOT,
                                      NULL, 0, NULL, (delay + 300));
            break;
        }
        case XIAOAI_ACTION_RECONNECT_EDR: {
            bool lea_mma = app_xiaoai_is_lea_mma_link();
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            bt_cm_power_state_t power_state = bt_cm_power_get_state();
            APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai action - Reconnect EDR [%02X] lea_mma=%d power_state=%d",
                             3, role, lea_mma, power_state);
            if (lea_mma) {
                APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai action - Reconnect EDR - ignore for LEA_MMA", 0);
                break;
            }
            if (role == BT_AWS_MCE_ROLE_AGENT
                && power_state == BT_CM_POWER_STATE_ON) {
                ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                          EVENT_GROUP_UI_SHELL_XIAOAI,
                                          XIAOAI_EVENT_RECONNECT_EDR_ACTION,
                                          NULL, 0, NULL, 0);
            }
            break;
        }
        case XIAOAI_ACTION_DISCONNECT_EDR: {
            bool lea_mma = app_xiaoai_is_lea_mma_link();
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            bt_cm_power_state_t power_state = bt_cm_power_get_state();
            APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai action - Disconnect EDR [%02X] lea_mma=%d power_state=%d unbind_flow_starting=%d",
                             4, role, lea_mma, power_state, g_xiaoai_unbind_flow_starting);
            if (lea_mma) {
                APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai action - Reconnect EDR - ignore for LEA_MMA", 0);
                break;
            }
            if (role == BT_AWS_MCE_ROLE_AGENT
                && power_state == BT_CM_POWER_STATE_ON
                && !g_xiaoai_unbind_flow_starting) {
                ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                          EVENT_GROUP_UI_SHELL_XIAOAI,
                                          XIAOAI_EVENT_DISCONNECT_EDR_ACTION,
                                          NULL, 0, NULL, 0);
            }
            break;
        }
        case XIAOAI_ACTION_UNBIND: {
            bool lea_mma = app_xiaoai_is_lea_mma_link();
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            bt_cm_power_state_t power_state = bt_cm_power_get_state();
            bool is_ble = (bool)param;
            APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai action - unbind [%02X] lea_mma=%d power_state=%d is_ble=%d",
                             4, role, lea_mma, power_state, (int)param);
            if (lea_mma) {
                APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai action - Reconnect EDR - ignore for LEA_MMA", 0);
                break;
            }
            if (role == BT_AWS_MCE_ROLE_AGENT
                && power_state == BT_CM_POWER_STATE_ON) {
                ret = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                          EVENT_GROUP_UI_SHELL_XIAOAI,
                                          XIAOAI_EVENT_UNBIND_ACTION,
                                          param, 0, NULL, (is_ble ? 500 : 0));
                if (ret == UI_SHELL_STATUS_OK) {
                    g_xiaoai_unbind_flow_starting = TRUE;
                }
            }
            break;
        }
        case XIAOAI_ACTION_SPEECH_VP: {
            APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai action - Speech VP", 0);
            voice_prompt_play_vp_press();
            break;
        }
    }
    APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai_action %d ret=%d", 2, action, ret);
}

static void app_va_xiaoai_device_config_fun(xiaoai_device_config_t *param)
{
    /* Switch to UI_Shell Task to device config, free param after dispatch UI_Shell event. */
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_XIAOAI,
                        XIAOAI_EVENT_DEVICE_CONFIG_ACTION,
                        (void *)param, sizeof(xiaoai_device_config_t), NULL, 0);
}

void app_va_xiaoai_stop_wwe()
{
#ifdef AIR_XIAOAI_WWE_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bool is_enable_wwe = app_va_xiaoai_is_enable_wwe();
    APPS_LOG_MSGID_I(LOG_TAG"[WWE], [%02X] stop_wwe %d", 2, role, is_enable_wwe);
    if (is_enable_wwe) {
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                            EVENT_GROUP_UI_SHELL_XIAOAI,
                            XIAOAI_EVENT_STOP_WWE_ACTION,
                            NULL, 0, NULL, 0);
    }
#endif
}

/**
 * @brief      The XiaoAI APP callback function is used to notify XiaoAI connection state.
 * @param[in]  state, XiaoAI connection state.
 */
static void app_va_xiaoai_conn_state_change(xiaoai_connection_state state)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bool lea_mma = app_xiaoai_is_lea_mma_link();
#ifdef AIR_XIAOAI_WWE_ENABLE
    bool wwe_feature = app_va_xiaoai_is_enable_wwe();
#else
    bool wwe_feature = FALSE;
#endif
    APPS_LOG_MSGID_I(LOG_TAG" app_va_xiaoai_state_change, [%02X] state=%d lea_mma=%d wwe_feature=%d",
                     4, role, state, lea_mma, wwe_feature);

    // Stop FM when XiaoAI Disconnected
    if (state == XIAOAI_STATE_DISCONNECTED) {
        app_find_me_do_find_me_action(false, FIND_ME_LEFT_RIGHT);
    }

    if (role == BT_AWS_MCE_ROLE_AGENT) {
        if (state == XIAOAI_STATE_PEER_CONNECTED || state == XIAOAI_STATE_PEER_DISCONNECTED) {
            /* Restart XiaoAI BLE ADV to update scan-response Byte13-bit7 when XiaoAI Connected/Disconnected. */
            app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_XIAOAI_CONN);
            return;
        }
    }

    if (state == XIAOAI_STATE_CONNECTED && lea_mma) {
        app_va_xiaoai_hfp_miui_more_atcmd_report_feature();
        uint8_t anti_lost_state = app_va_xiaoai_get_anti_lost_state();
        app_va_xiaoai_hfp_miui_more_atcmd_report_anti_lost(anti_lost_state);
        bool aws_connected = (bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), NULL, 0) > 0);
        bool miui_ret = app_va_xiaoai_hfp_miui_basic_atcmd();
        if (miui_ret) {
            g_xiaoai_mma_notify_pre_aws_state = aws_connected;
        }
    }

    if (state == XIAOAI_STATE_CONNECTED) {
#ifdef AIR_SMART_CHARGER_ENABLE
        app_smcharger_update_bat();
        uint8_t anti_lost = app_va_xiaoai_get_anti_lost_state();
        app_va_xiaoai_notify_device_config(XIAOAI_DEVICE_CONFIG_ANTI_LOST, &anti_lost, 1);
#endif

        /* Restart XiaoAI BLE ADV to update scan-response Byte13-bit7 when XiaoAI Connected. */
        app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_XIAOAI_CONN);
#ifdef AIR_XIAOAI_WWE_ENABLE
        if (wwe_feature) {
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_START_WWE_ACTION,
                                NULL, 0, NULL, 0);
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Conn)start", 0);
        }
#endif
    } else if (state == XIAOAI_STATE_DISCONNECTED) {
        /* Start XiaoAI BLE ADV when XiaoAI Disconnected. */
        app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_XIAOAI_CONN);
#if 0
        if (wwe_feature) {
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_STOP_WWE_ACTION,
                                NULL, 0, NULL, 0);
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Conn)stop", 0);
            // not stop, should restart when you support more hotword except "XiaoAI"
        }
#endif
    }
}

/**
 * @brief      The XiaoAI APP callback function is used to notify XiaoAI speech state.
 * @param[in]  state, XiaoAI speech state.
 */
static void app_va_xiaoai_speech_state_change(xiaoai_speech_state speech_state)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    xiaoai_connection_state conn_state = xiaoai_get_connection_state();
    APPS_LOG_MSGID_I(LOG_TAG" [%02X] speech_state_change %d->%d conn_state=%d",
                     4, role, g_xiaoai_speech_state, speech_state, conn_state);

#ifdef AIR_XIAOAI_PTT_ENABLE
    /* XiaoAI PTT - start PTT_SPEECH Activity.*/
    if (speech_state == XIAOAI_SPEECH_STATE_PTT_STARTING) {
        // PTT start (send D0 CMD)
        APPS_LOG_MSGID_I(LOG_TAG"[PTT] PTT_STARTING", 0);
    } else if (g_xiaoai_speech_state == XIAOAI_SPEECH_STATE_PTT_STARTING
               && speech_state == XIAOAI_SPEECH_STATE_STARTED) {
        // Recv D0 RSP
        bool ret = xiaoai_hold_ptt_speech();
        APPS_LOG_MSGID_I(LOG_TAG"[PTT] PTT_STARTED ret=%d", 1, ret);
    } else if (speech_state == XIAOAI_SPEECH_STATE_PTT_HOLD) {
        // Start D3 CMD and recv R3 RSP
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                            EVENT_GROUP_UI_SHELL_XIAOAI,
                            XIAOAI_EVENT_START_PTT_ACTIVITY,
                            NULL, 0, NULL, 0);
        APPS_LOG_MSGID_I(LOG_TAG"[PTT] PTT_HOLD", 0);
    } else if (g_xiaoai_speech_state == XIAOAI_SPEECH_STATE_PTT_HOLD
               && (speech_state == XIAOAI_SPEECH_STATE_STOPPING
                   || speech_state == XIAOAI_SPEECH_STATE_STOPPED)) {
        // PPT stop (after send D1 CMD)
        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                            EVENT_GROUP_UI_SHELL_XIAOAI,
                            XIAOAI_EVENT_FINISH_PTT_ACTIVITY,
                            NULL, 0, NULL, 0);
        APPS_LOG_MSGID_I(LOG_TAG"[PTT] PTT_STOP", 0);
    }
#endif

    if (g_xiaoai_speech_state != speech_state) {
#if defined(AIR_MULTI_POINT_ENABLE)
        /* For XiaoAI EMP Audio Active. */
        if (conn_state == XIAOAI_STATE_CONNECTED
            && speech_state == XIAOAI_SPEECH_STATE_STARTED) {
            g_xiaoai_session_state = XIAOAI_SESSION_STATE_RECORDING;
            xiaoai_bt_music_control_other_phone_music(TRUE);
        }
#endif

        g_xiaoai_speech_state = speech_state;
    }
}

static void app_va_xiaoai_session_state_change(xiaoai_session_state state)
{
    APPS_LOG_MSGID_I(LOG_TAG" session_state_change %d->%d",
                     2, g_xiaoai_session_state, state);
#if defined(AIR_MULTI_POINT_ENABLE)
    /* For XiaoAI EMP Audio Active. */
    if (g_xiaoai_session_state != XIAOAI_SESSION_STATE_SPEAKING
        && state == XIAOAI_SESSION_STATE_SPEAKING) {
        xiaoai_bt_music_control_other_phone_music(TRUE);
    } else if (g_xiaoai_session_state != XIAOAI_SESSION_STATE_IDLE
               && state == XIAOAI_SESSION_STATE_IDLE) {
        xiaoai_bt_music_control_other_phone_music(FALSE);
    }
#endif

    if (g_xiaoai_session_state != state) {
        g_xiaoai_session_state = state;
    }
}

/**
 * @brief      The XiaoAI APP callback function is used to notify XiaoAI OTA status.
 * @param[in]  ota_state
 */
static void app_va_xiaoai_ota_state_change(uint8_t ota_state)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    xiaoai_connection_state conn_state = xiaoai_get_connection_state();
    APPS_LOG_MSGID_I(LOG_TAG" [%02X] ota_state_change %d->%d conn_state=%d",
                     4, role, g_xiaoai_ota_state, ota_state, conn_state);

    if (g_xiaoai_ota_state != ota_state) {
        if (xiaoai_is_slience_ota()) {
            if (ota_state == XIAOAI_OTA_STATE_SLIENCE_OTA_DOWNLOAD_DONE) {
                APPS_LOG_MSGID_I(LOG_TAG" [%02X] SLIENCE_OTA_DOWNLOAD_DONE", 1, role);
                return;
            } else if (ota_state == XIAOAI_OTA_STATE_SLIENCE_OTA_DOWNLOAD_ERROR) {
                APPS_LOG_MSGID_I(LOG_TAG" [%02X] SLIENCE_OTA_DOWNLOAD_ERROR", 1, role);
                return;
            } else if (ota_state == XIAOAI_OTA_STATE_SLIENCE_OTA_ABORT_BY_FOREGROUND_OTA) {
                APPS_LOG_MSGID_I(LOG_TAG" [%02X] SLIENCE_OTA_ABORT_BY_FOREGROUND_OTA", 1, role);
                app_va_xiaoai_ota_slience_enable(FALSE, TRUE);
                return;
            }
        }

        if (ota_state == XIAOAI_OTA_STATE_ONGOING) {
            if (!xiaoai_is_slience_ota()) {
                app_va_xiaoai_send_ble_adv_event(FALSE, 0, 0);
            }

#ifdef AIR_XIAOAI_WWE_ENABLE
            if (conn_state == XIAOAI_STATE_CONNECTED && app_va_xiaoai_is_enable_wwe()) {
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                    EVENT_GROUP_UI_SHELL_XIAOAI,
                                    XIAOAI_EVENT_STOP_WWE_ACTION,
                                    NULL, 0, NULL, 0);
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] (OTA)stop", 0);
            }
#endif
            // exit and disable BT Sniff mode
            xiaoai_bt_exit_sniff_mode();
            // disable page_scan/LEA ADV temporarily
            xiaoai_bt_allow_new_conn(FALSE);

            g_xiaoai_ota_state = XIAOAI_OTA_STATE_ONGOING;
        } else if (ota_state == XIAOAI_OTA_STATE_STOP) {
#ifdef AIR_XIAOAI_WWE_ENABLE
            if (conn_state == XIAOAI_STATE_CONNECTED && app_va_xiaoai_is_enable_wwe()) {
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                    EVENT_GROUP_UI_SHELL_XIAOAI,
                                    XIAOAI_EVENT_START_WWE_ACTION,
                                    NULL, 0, NULL, 0);
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] (OTA)start", 0);
            }
#endif
            // enable BT Sniff mode
            xiaoai_bt_switch_sniff_mode(TRUE);
            // restore page_scan/LEA ADV temporarily
            xiaoai_bt_allow_new_conn(TRUE);

            g_xiaoai_ota_state = XIAOAI_OTA_STATE_STOP;
        }

#ifdef MTK_FOTA_ENABLE
        app_fota_set_ota_ongoing(ota_state == XIAOAI_OTA_STATE_ONGOING);
#endif
    }
}

/**
 * @brief      This function is used to check take over.
 * @param[in]  device_addr, the phone addr may be taken over.
 * @return     TRUE, allow.
 */
static bool app_va_xiaoai_takeover_allow(const bt_bd_addr_t phone_addr)
{
    bool allow = TRUE;
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    xiaoai_connection_state conn_state = conn_info.conn_state;
    bool is_ble = conn_info.is_ble;
    uint8_t *xiaoai_addr = conn_info.phone_addr;
    const uint8_t *takeover_addr = &phone_addr[0];
    APPS_LOG_MSGID_I(LOG_TAG" takeover_allow conn_state=%d is_ble=%d speech_state=%d addr=%08X%04X-%08X%04X",
                     7, conn_state, is_ble, g_xiaoai_speech_state,
                     *((uint32_t *)(xiaoai_addr + 2)), *((uint16_t *)xiaoai_addr),
                     *((uint32_t *)(takeover_addr + 2)), *((uint16_t *)takeover_addr));

    if (conn_state == XIAOAI_STATE_CONNECTED) {
#if 0
        // Note: VA via BLE cannot mapping phone EDR addr
        if (is_ble || memcmp(xiaoai_addr, takeover_addr, 6) == 0)
#endif
        {
            if ((g_xiaoai_speech_state == XIAOAI_SPEECH_STATE_NONE
                 || g_xiaoai_speech_state == XIAOAI_SPEECH_STATE_STOPPED)
                && g_xiaoai_ota_state == XIAOAI_OTA_STATE_STOP) {
                allow = TRUE;
            } else {
                // Note: We cannot get Thinking/Speaking state via XiaoAI APP/Spec.
                allow = FALSE;
            }
        }
    }

    APPS_LOG_MSGID_I(LOG_TAG" takeover_allow allow=%d", 1, allow);
    return allow;
}

/**
 * @brief      The XiaoAI APP init function, this function will be called when BT power on.
 */
static void app_va_xiaoai_init()
{
    APPS_LOG_MSGID_I(LOG_TAG" init xiaoai", 0);
    multi_ble_adv_manager_set_support_multi_adv_data(MULTI_ADV_INSTANCE_XIAOAI);
    app_bt_takeover_service_user_register(APP_BT_TAKEOVER_ID_XIAOAI, app_va_xiaoai_takeover_allow);
#ifdef AIR_MULTI_POINT_ENABLE
    app_bt_emp_srv_user_register(APP_BT_EMP_SRV_USER_ID_XIAOAI, app_xiaoai_emp_switch_allow_callback);
#endif
    app_va_xiaoai_hfp_at_cmd_register(TRUE);
    xiaoai_bt_helper_init(TRUE);

    xiaoai_init_para_t init_para = {0};
    /* Important reminder! Customer need to modify these parameters. */
    init_para.vid = APP_VA_XIAOAI_VID;             /* Customer configure option: vendor ID. */
    init_para.pid = APP_VA_XIAOAI_PID;             /* Customer configure option: product ID. */
    init_para.version = APP_VA_XIAOAI_VERSION;         /* Customer configure option: 1.0.0.1 (initial version). */
    init_para.device_name = (unsigned char *)bt_customer_config_get_gap_config()->device_name;
    init_para.app_action_fun = app_va_xiaoai_action_fun;
    init_para.device_config_fun = app_va_xiaoai_device_config_fun;
    init_para.handle_atcmd_fun = app_va_xiaoai_hfp_handle_mma_atcmd;
    init_para.conn_state_change_cb = app_va_xiaoai_conn_state_change;
    init_para.speech_state_change_cb = app_va_xiaoai_speech_state_change;
    init_para.session_state_change_cb = app_va_xiaoai_session_state_change;
    init_para.ota_state_cb = app_va_xiaoai_ota_state_change;
    init_para.speech_timeout_cb = app_va_xiaoai_speech_timeout;
    init_para.bat_charger_state_cb = app_va_xiaoai_handle_battery_charger_change;
    init_para.get_device_config_state_cb = app_va_xiaoai_get_device_config;
    init_para.get_bt_visible_cb = xiaoai_bt_is_visible;
    init_para.vendor_cmd_cb = app_va_xiaoai_handle_vendor_cmd;
    init_para.check_ble_link_cb = app_va_xiaoai_check_ble_link;
    init_para.get_vendor_data_cb = app_va_xiaoai_get_vendor_data;
    init_para.major_id = APP_VA_XIAOAI_MAJOR_ID;          /* Customer configure option: XiaoAI major_id. Airoha use test major_id from MI Air 2s. */
    init_para.minor_id = APP_VA_XIAOAI_MINOR_ID;          /* Customer configure option: XiaoAI minor_id. Airoha use test minor_id from MI Air 2s. */
    init_para.device_type = APP_VA_XIAOAI_DEVICE_TYPE;    /* Customer configure option: 0 - invalid, device_type 1~15. */
    init_para.is_callback_mma_cmd_to_app = TRUE; /* Customer configure option: TRUE - XiaoAI notify APP vendor_cmd_cb function to handle MMA 0x02,0x08,0x09,0x0E CMD. */
    init_para.is_reconnect_last_edr_addr = FALSE; /* Customer configure option: TRUE - XiaoAI reconnect last EDR addr when only BLE connected, not use virtual_addr mapping feature. */
    init_para.is_send_d0_when_speeching = FALSE;  /* Customer configure option: TRUE - XiaoAI still send 0xD0 CMD to phone when speech ongoing and click key. */
    init_para.is_foreground_ota = TRUE; /* Customer configure option: TRUE - must put earbuds into charger case, faster download speed; FALSE - background OTA, OTA while play music or call, download slowly, no need to put earbuds into charger case. */
    init_para.is_support_mma_in_leaudio = TRUE;   /* Customer configure option: whether support SP switch XIAOAI connection from EDR to LE Audio. */
#ifdef AIR_XIAOAI_WWE_ENABLE
    init_para.va_recorder_event_cb = app_va_xiaoai_recorder_event_notify;
#endif
    xiaoai_va_timer_t timer_param = {0};
    timer_param.lea_tx_cmd_timer = 9000;
    timer_param.wait_d0_rsp_timer_android = 500;
    timer_param.wait_d0_rsp_timer_iphone = 500;
    timer_param.wait_d0_rsp_timer_lea = 2000;
    timer_param.wait_d1_cmd_timer = 30 * 1000;
    timer_param.wait_d1_rsp_timer = 1000;
    init_para.timer_param = timer_param;
    xiaoai_app_init(init_para);
}

/**
 * @brief      The XiaoAI APP de-init function, this function will be called when BT power off.
 */
static void app_va_xiaoai_deinit()
{
    APPS_LOG_MSGID_I(LOG_TAG" deinit xiaoai", 0);
    app_bt_takeover_service_user_deregister(APP_BT_TAKEOVER_ID_XIAOAI);
    app_va_xiaoai_hfp_at_cmd_register(FALSE);
    xiaoai_bt_helper_init(FALSE);
    xiaoai_app_deinit();
}

#ifdef AIR_XIAOAI_PTT_ENABLE
static void app_va_xiaoai_ptt_stop()
{
    if (g_xiaoai_speech_state == XIAOAI_SPEECH_STATE_PTT_STARTING
        || g_xiaoai_speech_state == XIAOAI_SPEECH_STATE_STARTED
        || g_xiaoai_speech_state == XIAOAI_SPEECH_STATE_PTT_HOLD) {
        bool result = xiaoai_stop_recognizing();
        APPS_LOG_MSGID_I(LOG_TAG"[PTT] [AGENT] LONG_PRESS_TRIGGER_STOP speech=%d result=%d",
                         2, g_xiaoai_speech_state, result);
    } else {
        APPS_LOG_MSGID_I(LOG_TAG"[PTT] [AGENT] LONG_PRESS_TRIGGER_STOP speech=%d",
                         1, g_xiaoai_speech_state);
    }
}
#endif

bool app_va_xiaoai_activity_proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(LOG_TAG" CREATE", 0);
            xiaoai_app_preinit(app_va_xiaoai_handle_battery_charger_change);
            app_va_xiaoai_ota_init();
#ifdef AIR_XIAOAI_WWE_ENABLE
            xiaoai_app_enable_wwe();
            xiaoai_wwe_recorder_init();
#endif
            app_va_xiaoai_init_device_config();
#ifdef APP_XIAOAI_TEST
            atci_status_t ret = atci_register_handler(app_xiaoai_atci_cmd, sizeof(app_xiaoai_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
            APPS_LOG_MSGID_I(LOG_TAG" app_xiaoai init atci ret=%d", 1, ret);
#endif
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
            miui_fast_connect_init();
#endif
        }
        break;
    }
    return TRUE;
}

static bool app_va_xiaoai_interaction_event_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    switch (event_id) {
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
        /* The old Agent will switch to new Partner if RHO successfully. */
        case APPS_EVENTS_INTERACTION_RHO_END: {
            app_rho_result_t rho_result = (app_rho_result_t)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" [new Partner] RHO done - %d", 1, rho_result);
            if (APP_RHO_RESULT_SUCCESS == rho_result) {
                g_xiaoai_acl_connected = FALSE;
                /* Stop XiaoAI BLE ADV when RHO switch to Partner. */
                app_va_xiaoai_send_ble_adv_event(FALSE, 0, 0);
                xiaoai_app_send_conn_state_to_peer();
#ifdef AIR_XIAOAI_WWE_ENABLE
                if (app_va_xiaoai_is_enable_wwe()) {
                    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                        EVENT_GROUP_UI_SHELL_XIAOAI,
                                        XIAOAI_EVENT_STOP_WWE_ACTION,
                                        NULL, 0, NULL, 0);
                    APPS_LOG_MSGID_I(LOG_TAG"[WWE] (RHO)stop", 0);
                }
#endif
            }
            break;
        }
        /* The old Partner will switch to new Agent if RHO successfully. */
        case APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT: {
            app_rho_result_t rho_result = (app_rho_result_t)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" [new Agent] RHO done - %d", 1, rho_result);
            if (APP_RHO_RESULT_SUCCESS == rho_result) {
                xiaoai_app_send_conn_state_to_peer();
#ifdef AIR_SMART_CHARGER_ENABLE
                app_smcharger_update_bat();
#endif
                g_xiaoai_acl_connected = TRUE;
                app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_XIAOAI_CONN);
#ifdef AIR_XIAOAI_WWE_ENABLE
                if (app_va_xiaoai_is_enable_wwe()) {
                    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                        EVENT_GROUP_UI_SHELL_XIAOAI,
                                        XIAOAI_EVENT_START_WWE_ACTION,
                                        NULL, 0, NULL, 0);
                    APPS_LOG_MSGID_I(LOG_TAG"[WWE] (RHO)start", 0);
                }
#endif
            }
            break;
        }
#endif

#ifdef MTK_IN_EAR_FEATURE_ENABLE
        case APPS_EVENTS_INTERACTION_IN_EAR_UPDATE_STA: {
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            app_in_ear_sta_info_t *in_ear_event = (app_in_ear_sta_info_t *)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" [IN_EAR][%02X] in_ear=%d->%d", 3, role,
                             in_ear_event->previous, in_ear_event->current);
            // only agent need to handle the in_ear event for Anti-Lost feature
            if (role == BT_AWS_MCE_ROLE_AGENT && in_ear_event->previous != in_ear_event->current) {
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI, XIAOAI_EVENT_DEVICE_ANTI_LOST_ACTION);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                                    EVENT_GROUP_UI_SHELL_XIAOAI,
                                    XIAOAI_EVENT_DEVICE_ANTI_LOST_ACTION,
                                    NULL, 0, NULL, 500);
            }
            break;
        }
#endif
    }
    return ret;
}

bool app_va_xiaoai_activity_proc_key_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    uint16_t key_action = 0;
    if (extra_data != NULL) {
        key_action = *(uint16_t *)extra_data;
    }
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    xiaoai_connection_state conn_state = xiaoai_get_connection_state();

    if (key_action != KEY_VA_XIAOAI_START_NOTIFY
        && key_action != KEY_VA_XIAOAI_START
        && key_action != KEY_VA_XIAOAI_STOP_PLAY
#ifdef AIR_XIAOAI_PTT_ENABLE
        && key_action != KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_START
        && key_action != KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_STOP
#endif
       ) {
        return FALSE;
    }

    APPS_LOG_MSGID_I(LOG_TAG" key_event [%02X], key_id=0x%04X conn_state=%d",
                     3, role, key_action, conn_state);

    /* Cannot do XiaoAI VA flow when HFP/SCO_VA ongoing. */
    bt_sink_srv_state_t bt_sink_state = bt_sink_srv_get_state();
    bt_sink_srv_am_type_t scenario = bt_sink_srv_ami_get_current_scenario();
    if (bt_sink_state >= BT_SINK_SRV_STATE_INCOMING
        || scenario == HFP) {
        APPS_LOG_MSGID_I(LOG_TAG" key_event HFP state=%d scenario=%d",
                         2, bt_sink_state, scenario);
        return FALSE;
    } else if (app_va_xiaoai_is_call_ongoing()) {
        APPS_LOG_MSGID_I(LOG_TAG" key_event call ongoing", 0);
        return FALSE;
    }

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    /* Cannot do XiaoAI VA flow when leakage_detection ongoing. */
    if (app_va_xiaoai_is_ld_ongoing()) {
        APPS_LOG_MSGID_I(LOG_TAG" key_event leakage_detection ongoing", 0);
        return FALSE;
    }
#endif

    /* Check and do XiaoAI wake up flow. Only Agent for HFP-VA. */
    bool hfp_va_activate = FALSE;
    if (role == BT_AWS_MCE_ROLE_AGENT
        && (key_action == KEY_VA_XIAOAI_START || key_action == KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_START)) {
        hfp_va_activate = app_va_xiaoai_wake_up();
    }

    /* Always play press VP. */
    if (key_action == KEY_VA_XIAOAI_START_NOTIFY
        || key_action == KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_START) {
        APPS_LOG_MSGID_I(LOG_TAG" [%02X] key_event XIAOAI_START_NOTIFY VP", 1, role);
        voice_prompt_play_vp_press();
    }

    if (conn_state == XIAOAI_STATE_NONE) {
        APPS_LOG_MSGID_I(LOG_TAG" [%02X] key_event, ignore", 0);
        return FALSE;
    }

#ifdef MTK_AWS_MCE_ENABLE
    if (conn_state != XIAOAI_STATE_CONNECTED) {
        if (BT_STATUS_SUCCESS == apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, key_action)) {
            APPS_LOG_MSGID_I(LOG_TAG" key_event, send key_id=0x%04x succeed", 1, key_action);
        } else {
            APPS_LOG_MSGID_I(LOG_TAG" key_event, send key_id=0x%04x failed", 1, key_action);
        }
    }
#endif

    if (conn_state == XIAOAI_STATE_CONNECTED && !hfp_va_activate) {
        if (key_action == KEY_VA_XIAOAI_START
            || key_action == KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_START) {
            /* Start speech if Agent received KEY_VA_XIAOAI_START and XiaoAI connected. */
            xiaoai_va_trigger_type type = (key_action == KEY_VA_XIAOAI_START ? XIAOAI_VA_TRIGGER_TAP : XIAOAI_VA_TRIGGER_PRESS_HOLD);
#ifdef AIR_XIAOAI_WWE_ENABLE
            bool wwe_stop_ret = FALSE;
            if (app_va_xiaoai_is_enable_wwe()) {
                wwe_stop_ret = xiaoai_wwe_recorder_stop();
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] stop ret=%d", 1, wwe_stop_ret);
            } else {
                wwe_stop_ret = TRUE;
            }
            bool result = FALSE;
            if (wwe_stop_ret) {
                result = xiaoai_start_recognizing(type);
            }
#else
            bool result = xiaoai_start_recognizing(type);
#endif
            APPS_LOG_MSGID_I(LOG_TAG" [AGENT] key_event start_recognizing type=%d result=%d",
                             2, type, result);
        } else if (key_action == KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_STOP) {
#ifdef AIR_XIAOAI_PTT_ENABLE
            app_va_xiaoai_ptt_stop();
#endif
        }
    }
    return TRUE;
}

bool app_va_xiaoai_bt_sink_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    if (BT_SINK_SRV_EVENT_HF_SCO_STATE_UPDATE == event_id) {
        bt_sink_srv_sco_state_update_t *sco_state_update_ind = (bt_sink_srv_sco_state_update_t *) extra_data;
        bt_sink_srv_sco_connection_state_t state = sco_state_update_ind->state;
        APPS_LOG_MSGID_I(LOG_TAG" eSCO Connection state=%d", 1, state);

#ifdef AIR_XIAOAI_WWE_ENABLE
        bool enable_wwe = app_va_xiaoai_is_enable_wwe();
        APPS_LOG_MSGID_I(LOG_TAG" [WWE] eSCO wwe_state=%d enable_wwe=%d",
                         2, g_xiaoai_wwe_state, enable_wwe);
        if (state == BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED
            && g_xiaoai_wwe_state == XIAOAI_WWE_STATE_STOPPED
            && enable_wwe) {
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_START_WWE_ACTION,
                                NULL, 0, NULL, 0);
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (HFP)start", 0);
        }
#endif
    } else if (BT_SINK_SRV_EVENT_STATE_CHANGE == event_id) {
        bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *)extra_data;
        APPS_LOG_MSGID_I(LOG_TAG" bt_sink state %04X->%04X", 2, param->previous, param->current);

#ifdef AIR_XIAOAI_WWE_ENABLE
        bool enable_wwe = app_va_xiaoai_is_enable_wwe();
        if (param->previous >= BT_SINK_SRV_STATE_INCOMING
            && param->current < BT_SINK_SRV_STATE_INCOMING
            && g_xiaoai_wwe_state == XIAOAI_WWE_STATE_STOPPED
            && enable_wwe) {
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_START_WWE_ACTION,
                                NULL, 0, NULL, 0);
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (BT_SINK CALL)stop", 0);
        }
#endif
    } else if (BT_SINK_SRV_EVENT_LE_BIDIRECTION_LEA_UPDATE == event_id) {
        bt_sink_srv_bidirection_lea_state_update_t *param = (bt_sink_srv_bidirection_lea_state_update_t *)extra_data;
        APPS_LOG_MSGID_I(LOG_TAG" bidirection_lea handle=0x%04X state=%d", 2, param->le_handle, param->state);

#ifdef AIR_XIAOAI_WWE_ENABLE
        bool enable_wwe = app_va_xiaoai_is_enable_wwe();
        if (param->state == BT_SINK_SRV_BIDIRECTION_LEA_STATE_DISABLE
            && g_xiaoai_wwe_state == XIAOAI_WWE_STATE_STOPPED
            && enable_wwe) {
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_START_WWE_ACTION,
                                NULL, 0, NULL, 0);
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (LEA_CALL)start", 0);
        }
#endif
    }

    return FALSE;
}

bool app_va_xiaoai_bt_cm_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    /* Handle new BT_CM Event. */
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (remote_update != NULL) {
                bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
                bool lea_mma = app_xiaoai_is_lea_mma_link();
                bool peer_lea_mma = xiaoai_peer_is_lea_mma_link();

                uint8_t *addr = (uint8_t *)remote_update->address;
                APPS_LOG_MSGID_I(LOG_TAG" bt_cm_update [%02X] addr=%08X%04X acl=%d->%d srv=0x%04X->0x%04X",
                                 7, role, *((uint32_t *)(addr + 2)), *((uint16_t *)addr),
                                 remote_update->pre_acl_state, remote_update->acl_state,
                                 remote_update->pre_connected_service, remote_update->connected_service);

                // XiaoAI MMA 0x0F unbind Flow
                if (role == BT_AWS_MCE_ROLE_AGENT) {
                    xiaoai_bt_check_unbind(remote_update);
                }

                /* Check AWS connection and notify to phone. */
                if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                    && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK][%02X] AWS Attached, lea_mma=%d peer_lea_mma=%d",
                                     3, role, lea_mma, peer_lea_mma);
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI, XIAOAI_EVENT_AWS_ACTION);
                    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                        EVENT_GROUP_UI_SHELL_XIAOAI,
                                        XIAOAI_EVENT_AWS_ACTION,
                                        (void *)TRUE, 0, NULL, 1000);
                    if (role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_NONE) {
                        app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_XIAOAI_CONN);
                    }
                } else if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                           && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK][%02X] AWS Detached, lea_mma=%d peer_lea_mma=%d",
                                     3, role, lea_mma, peer_lea_mma);
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI, XIAOAI_EVENT_AWS_ACTION);
                    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                        EVENT_GROUP_UI_SHELL_XIAOAI,
                                        XIAOAI_EVENT_AWS_ACTION,
                                        FALSE, 0, NULL, 1000);
                    if (role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_NONE) {
                        app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_XIAOAI_CONN);
                    }
                }

                if (role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_NONE) {
                    /* Check Agent HFP connection and set HFP state. */
                    if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->pre_connected_service)
                        != (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->connected_service)) {
                        bool hfp_connected = FALSE;
                        if (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->connected_service) {
                            hfp_connected = TRUE;
                            app_va_xiaoai_hfp_miui_more_atcmd_report_feature();
                            uint8_t anti_lost_state = app_va_xiaoai_get_anti_lost_state();
                            app_va_xiaoai_hfp_miui_more_atcmd_report_anti_lost(anti_lost_state);
                            bool aws_connected = (bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                                                                              NULL, 0) > 0);
                            bool miui_ret = app_va_xiaoai_hfp_miui_basic_atcmd();
                            if (miui_ret) {
                                g_xiaoai_mma_notify_pre_aws_state = aws_connected;
                            }

                            app_va_xiaoai_report_miui_fast_connect_at_cmd();
                        } else {
                            hfp_connected = FALSE;
                        }
                        APPS_LOG_MSGID_I(LOG_TAG" [Agent] HFP Connection State %d", 1, hfp_connected);
                    }

                    /* Check Agent A2DP connection and notify. */
                    if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->pre_connected_service)
                        != (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->connected_service)) {
                        bool a2dp_connected = FALSE;
                        if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->connected_service) > 0) {
                            a2dp_connected = TRUE;
                            // Should notify EDR(A2DP) status after discuss with MI
                            xiaoai_notify_sp_status(XIAOAI_APP_NOTIFY_EDR_STATUS, TRUE);

#ifdef AIR_XIAOAI_WWE_ENABLE
                            APPS_LOG_MSGID_I(LOG_TAG"[WWE] [Agent] start WWE when A2DP ready", 0);
                            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI, XIAOAI_EVENT_START_WWE_ACTION);
                            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                                XIAOAI_EVENT_START_WWE_ACTION,
                                                NULL, 0, NULL, 0);
#endif
                        } else {
                            a2dp_connected = FALSE;
                            xiaoai_notify_sp_status(XIAOAI_APP_NOTIFY_EDR_STATUS, FALSE);
                        }
                        APPS_LOG_MSGID_I(LOG_TAG" [Agent] A2DP Connection State %d", 1, a2dp_connected);
                    }

                    /* ACL connection state. */
                    if (remote_update->pre_acl_state < BT_CM_ACL_LINK_CONNECTED
                        && remote_update->acl_state >= BT_CM_ACL_LINK_CONNECTED
                        && 0 != memcmp(remote_update->address, bt_device_manager_get_local_address(), sizeof(bt_bd_addr_t))) {
                        APPS_LOG_MSGID_I(LOG_TAG" phone EDR(ACL) Connected", 0);
                        g_xiaoai_acl_connected = TRUE;
                        /* Start XiaoAI BLE ADV if EDR(ACL) connected.*/
                        xiaoai_connection_state xiaoai_state = xiaoai_get_connection_state();
                        if (xiaoai_state == XIAOAI_STATE_CONNECTING
                            || xiaoai_state == XIAOAI_STATE_CONNECTED) {
                            APPS_LOG_MSGID_I(LOG_TAG" No need to start BLE ADV when XiaoAI/EDR Connected", 0);
                        } else {
                            app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_XIAOAI_CONN);
                        }
                    } else if (remote_update->pre_acl_state >= BT_CM_ACL_LINK_CONNECTED
                               && remote_update->acl_state < BT_CM_ACL_LINK_CONNECTED) {
                        if (bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), NULL, 0) == 0
                            && g_xiaoai_acl_connected) {
                            APPS_LOG_MSGID_I(LOG_TAG" phone EDR(ACL) Disconnected", 0);
                            g_xiaoai_acl_connected = FALSE;
                            /* May stop XiaoAI BLE ADV when EDR(ACL) disconnected. */
                            app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_XIAOAI_CONN);
                        }
                    }
                }
            }
            break;
        }

        case BT_CM_EVENT_VISIBILITY_STATE_UPDATE: {
            bt_cm_visibility_state_update_ind_t *visible_ind = (bt_cm_visibility_state_update_ind_t *)extra_data;
            if (visible_ind != NULL) {
                g_xiaoai_edr_visible = visible_ind->visibility_state;
                APPS_LOG_MSGID_I(LOG_TAG" EDR visible %d", 1, g_xiaoai_edr_visible);
#ifndef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
                /* May start/stop XiaoAI BLE ADV when visible state changed. */
                app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_XIAOAI_CONN);
#endif
            }
            break;
        }
        default:
            break;
    }
    return ret;
}

bool app_va_xiaoai_bt_dm_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bt_device_manager_power_event_t event = 0;
    bt_device_manager_power_status_t status = 0;
    bt_event_get_bt_dm_event_and_status(event_id, &event, &status);
    switch (event) {
        case BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_RESET_TYPE_NORMAL == status) {
                APPS_LOG_MSGID_I(LOG_TAG" BT power off", 0);
                if (g_xiaoai_init_done) {
                    app_va_xiaoai_deinit();
                    g_xiaoai_init_done = FALSE;
                }
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
                miui_fast_connect_ble_enable(FALSE);
#endif
            }
            break;
        }
        case BT_DEVICE_MANAGER_POWER_EVT_ACTIVE_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
                APPS_LOG_MSGID_I(LOG_TAG" BT power on", 0);
                if (!g_xiaoai_init_done) {
                    app_va_xiaoai_init();
                    g_xiaoai_init_done = TRUE;
                }
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
                miui_fast_connect_ble_enable(TRUE);
#endif
            }
            break;
        }
    }
    return FALSE;
}

#ifdef MTK_AWS_MCE_ENABLE
bool app_va_xiaoai_activity_proc_aws_data(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = FALSE;

    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    xiaoai_connection_state conn_state = xiaoai_get_connection_state();
    bool lea_mma = app_xiaoai_is_lea_mma_link();
    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group;
        uint32_t action;
        void *p_extra_data = NULL;
        uint32_t extra_data_len = 0;

        //apps_aws_sync_event_decode(aws_data_ind, &event_group, &action);
        apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &action, &p_extra_data, &extra_data_len);

        /* Agent should receive and handle XiaoAI key event from AWS MCE.*/
        if (event_group == EVENT_GROUP_UI_SHELL_KEY) {
            if (action == KEY_VA_XIAOAI_START_NOTIFY) {
                APPS_LOG_MSGID_I(LOG_TAG" [%02X] aws key_event -> XIAOAI_START_NOTIFY", 1, role);
            } else if (action == KEY_VA_XIAOAI_START
                       || action == KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_START) {
                /* Cannot do XiaoAI VA flow when HFP ongoing. */
                bt_sink_srv_state_t bt_sink_state = bt_sink_srv_get_state();
                bt_sink_srv_am_type_t scenario = bt_sink_srv_ami_get_current_scenario();
                if (bt_sink_state >= BT_SINK_SRV_STATE_INCOMING
                    || scenario == HFP) {
                    APPS_LOG_MSGID_I(LOG_TAG" [%02X] AWS key_event HFP state=%d",
                                     3, role, bt_sink_state, scenario);
                    return FALSE;
                }

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
                /* Cannot do XiaoAI VA flow when leakage_detection ongoing. */
                if (app_va_xiaoai_is_ld_ongoing()) {
                    APPS_LOG_MSGID_I(LOG_TAG" [%02X] AWS key_event leakage_detection ongoing", 1, role);
                    return FALSE;
                }
#endif

                /* Check and do XiaoAI wake up flow. */
                bool hfp_va_activate = FALSE;
                if (role == BT_AWS_MCE_ROLE_AGENT) {
                    hfp_va_activate = app_va_xiaoai_wake_up();
                }

                if (conn_state == XIAOAI_STATE_CONNECTED && !hfp_va_activate) {
                    /* Start speech if Agent received KEY_VA_XIAOAI_START and XiaoAI connected. */
                    xiaoai_va_trigger_type type = (action == KEY_VA_XIAOAI_START ? XIAOAI_VA_TRIGGER_TAP : XIAOAI_VA_TRIGGER_PRESS_HOLD);
#ifdef AIR_XIAOAI_WWE_ENABLE
                    bool wwe_stop_ret = FALSE;
                    if (app_va_xiaoai_is_enable_wwe()) {
                        wwe_stop_ret = xiaoai_wwe_recorder_stop();
                        APPS_LOG_MSGID_I(LOG_TAG"[WWE] stop ret=%d", 1, wwe_stop_ret);
                    } else {
                        wwe_stop_ret = TRUE;
                    }
                    bool result = FALSE;
                    if (wwe_stop_ret) {
                        result = xiaoai_start_recognizing(type);
                    }
#else
                    bool result = xiaoai_start_recognizing(type);
#endif
                    APPS_LOG_MSGID_I(LOG_TAG" [AGENT] AWS key_event start_recognizing type=%d result=%d",
                                     2, type, result);
                }
            } else if (action == KEY_VA_XIAOAI_LONG_PRESS_TRIGGER_STOP) {
#ifdef AIR_XIAOAI_PTT_ENABLE
                app_va_xiaoai_ptt_stop();
#endif
            }
        }

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
        if (role == BT_AWS_MCE_ROLE_AGENT
            && event_group == EVENT_GROUP_UI_SHELL_XIAOAI
            && action == XIAOAI_EVENT_SYNC_LEAKAGE_RESULT) {
            uint8_t leakage_result = *((uint8_t *)p_extra_data);
            APPS_LOG_MSGID_I(LOG_TAG" [Agent] AWS handle partner leakage=%d",
                             1, leakage_result);
            app_va_xiaoai_agent_handle_partner_leakage_result(leakage_result);
        }
#endif

#if defined(AIR_XIAOAI_WWE_ENABLE) || defined(MTK_ANC_ENABLE)
        if (event_group == EVENT_GROUP_UI_SHELL_XIAOAI
            && action == XIAOAI_EVENT_SYNC_SAVING_CONFIG) {
            APPS_LOG_MSGID_I(LOG_TAG" [%02X] AWS handle saving_config=0x%08X",
                             2, role, p_extra_data);
            app_va_xiaoai_partner_handle_saving_config((uint8_t *)p_extra_data);
        }
#endif

        if (event_group == EVENT_GROUP_UI_SHELL_XIAOAI
            && action == XIAOAI_EVENT_SYNC_DEVICE_NAME) {
            uint8_t *name = (uint8_t *)p_extra_data;
            uint8_t len = extra_data_len;
            APPS_LOG_MSGID_I(LOG_TAG" AWS handle name_len=%d", 1, len);
            app_va_xiaoai_peer_set_device_name(name, len);
        }

        // For LEA-Partner, need Agent to do device_config
        if (role == BT_AWS_MCE_ROLE_AGENT
            && event_group == EVENT_GROUP_UI_SHELL_XIAOAI
            && action == XIAOAI_EVENT_LEA_SYNC_DEVICE_CONFIG_TO_AGENT) {
            APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK][Agent] AWS handle Device_Config", 0);
            app_va_xiaoai_set_device_config(FALSE, (void *)p_extra_data);
        } else if (role == BT_AWS_MCE_ROLE_PARTNER
                   && event_group == EVENT_GROUP_UI_SHELL_XIAOAI
                   && action == XIAOAI_EVENT_LEA_SYNC_DEVICE_CONFIG_RESULT_TO_PARTNER) {
            APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK][Partner] AWS handle Device_Config", 0);
            app_va_xiaoai_reply_device_config_result((xiaoai_device_config_reply_result_t *)p_extra_data);
        }

        if (event_group == EVENT_GROUP_UI_SHELL_XIAOAI
            && action == XIAOAI_EVENT_LEA_SYNC_DEVICE_CONFIG_NOTIFY_TO_PEER) {
            xiaoai_device_config_notfiy_t *param = (xiaoai_device_config_notfiy_t *)p_extra_data;
            uint8_t type = param->type;
            uint8_t *data = param->data;
            uint8_t len = param->len;
            APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK][%02X] AWS handle DEVICE_CONFIG_NOTIFY lea_mma=%d type=%d len=%d",
                             4, role, lea_mma, type, len);
            if (lea_mma) {
                app_va_xiaoai_notify_device_config(type, data, len);
            }
        }

        if (event_group == EVENT_GROUP_UI_SHELL_XIAOAI
            && action == XIAOAI_EVENT_LEA_SYNC_AT_CMD_TO_PEER) {
            xiaoai_at_cmd_param_t *param = (xiaoai_at_cmd_param_t *)p_extra_data;
            uint16_t mma_type = param->mma_type;
            uint32_t atcmd_len = param->atcmd_len;
            uint8_t *atcmd = param->atcmd;
            APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK][%02X] AWS handle DEVICE_CONFIG_NOTIFY, lea_mma=%d mma_type=0x%04X atcmd=0x%08X atcmd_len=%d",
                             5, role, lea_mma, mma_type, atcmd, atcmd_len);
            if (lea_mma) {
                app_va_xiaoai_send_atcmd(mma_type, (const char *)atcmd, atcmd_len);
            }
        }

        if (event_group == EVENT_GROUP_UI_SHELL_XIAOAI
            && action == XIAOAI_EVENT_LEA_SYNC_NOTIFY_SP_TO_PEER) {
            xiaoai_notify_sp_param_t *param = (xiaoai_notify_sp_param_t *)p_extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK][%02X] AWS handle NOTIFY_SP lea_mma=%d notify_type=%d data=%d",
                             4, role, lea_mma, param->notify_type, param->data);
            if (lea_mma) {
                xiaoai_notify_sp_status(param->notify_type, param->data);
            }
        }

        if (event_group == EVENT_GROUP_UI_SHELL_XIAOAI
            && action == XIAOAI_EVENT_SYNC_LD_RESULT) {
            xiaoai_leakage_result_param_t *param = (xiaoai_leakage_result_param_t *)p_extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK][%02X] AWS handle NOTIFY_LD_RESULT left=%d right=%d",
                             3, role, param->left_result, param->right_result);
            xiaoai_set_ld_result(param->left_result, param->right_result);
        }

        if (event_group == EVENT_GROUP_UI_SHELL_XIAOAI
            && action == XIAOAI_EVENT_OTA_SLIENCE_OTA_SYNC) {
            uint8_t *enable = (uint8_t *)p_extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" [%02X] AWS handle SLIENCE_OTA_SYNC enable=%d",
                             2, role, *enable);
            app_va_xiaoai_ota_slience_enable(*enable, FALSE);
        }
#ifdef AIR_XIAOAI_WWE_ENABLE
        if (event_group == EVENT_GROUP_UI_SHELL_XIAOAI
            && action == XIAOAI_EVENT_RESTART_WWE_ACTION) {
            bool ret = xiaoai_wwe_recorder_stop();
            APPS_LOG_MSGID_I(LOG_TAG"[WWE][AWS] (Event)restart, stop ret=%d", 1, ret);
            if (ret) {
                ret = xiaoai_wwe_recorder_start();
                APPS_LOG_MSGID_I(LOG_TAG"[WWE][AWS] (Event)restart, ret=%d", 1, ret);
            }
        }
#endif
    }
    return ret;
}
#endif

/**
 * @brief      This function is used to check and send event to start XiaoAI BLE ADV.
 * @param[in]  reason, the start reason of XiaoAI BLE ADV.
 * @return     TRUE, send XiaoAI event to start BLE ADV.
 */
static bool app_va_xiaoai_check_and_send_ble_adv_event(xiaoai_start_ble_adv_reason reason)
{
#define MIUI_QUICK_CONNECT_BLE_ADV_TIME     (2 * 60 * 1000)     /* Only advertise 2 min for MIUI quick connection. */
#define XIAOAI_BLE_ADV_TIME                 (-1)                /* Always advertising for XiaoAI connection. */

    bool ret = FALSE;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    xiaoai_connection_state xiaoai_state = xiaoai_get_connection_state();
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    /* Should advertising when Agent and AWS connected or BT EDR connected. */
    if (role == BT_AWS_MCE_ROLE_AGENT
        && (g_xiaoai_acl_connected
            || g_xiaoai_edr_visible
            || aws_link_type != BT_AWS_MCE_SRV_LINK_NONE)) {
        ret = TRUE;
    }
    APPS_LOG_MSGID_I(LOG_TAG" [BLE_ADV] ret=%d role=0x%02X reason=%d bt_acl=%d visible=%d aws_connected=%d xiaoai=%d aws_link_type=%d",
                     8, ret, role, reason, g_xiaoai_acl_connected, g_xiaoai_edr_visible,
                     g_xiaoai_aws_connected, xiaoai_state, aws_link_type);
    if (ret) {
        /* For adv_interval_ms, need to fast advertising when both lid_open/lid_close. */
        bool fast_adv = (reason == XIAOAI_BLE_ADV_REASON_LID_OPEN
                         || reason == XIAOAI_BLE_ADV_REASON_LID_CLOSE
                         /*|| reason == XIAOAI_BLE_ADV_REASON_CHARGER_IN_OUT*/);
        uint16_t adv_interval_ms = (fast_adv ? 30 : 62);

        /* For adv_time_ms, Customer configure option: BLE_ADV duration time depend on MIUI_QUICK_CONNECT or XIAOAI Connection. */
        uint32_t adv_time_ms = 0;
        if (xiaoai_state == XIAOAI_STATE_CONNECTED) {
            adv_time_ms = MIUI_QUICK_CONNECT_BLE_ADV_TIME;
        } else {
            adv_time_ms = XIAOAI_BLE_ADV_TIME;
        }
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
        if (reason == XIAOAI_BLE_ADV_REASON_MIUI_FAST_CONNECT) {
            adv_interval_ms = 40;
            adv_time_ms = (3 * 60 * 1000);
        }
#endif
        app_va_xiaoai_report_miui_fast_connect_at_cmd();
        app_va_xiaoai_send_ble_adv_event(TRUE, adv_interval_ms, adv_time_ms);
    } else if (reason == XIAOAI_BLE_ADV_REASON_XIAOAI_CONN) {
        app_va_xiaoai_send_ble_adv_event(FALSE, 0, 0);
    }
    return ret;
}

#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
void app_va_xiaoai_miui_fc_start_adv()
{
    app_va_xiaoai_check_and_send_ble_adv_event(XIAOAI_BLE_ADV_REASON_MIUI_FAST_CONNECT);
}
#endif

bool app_va_xiaoai_smcharger_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = FALSE;
#ifdef AIR_SMART_CHARGER_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    switch (event_id) {
        case SMCHARGER_EVENT_NOTIFY_ACTION: {
            app_smcharger_public_event_para_t *event_para = (app_smcharger_public_event_para_t *)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" SMCharger group [%02X] NOTIFY_PUBLIC_EVENT %d", 2, role, event_para->action);
            /* set XiaoAI charger state according to SmartCharger public event. */
            if (event_para->action == SMCHARGER_CHARGER_IN_ACTION) {
                xiaoai_set_charger_state(XIAOAI_SMCHARGER_CHARGER_IN);
            } else if (event_para->action == SMCHARGER_OPEN_LID_ACTION) {
                xiaoai_set_charger_state(XIAOAI_SMCHARGER_OPEN_LID);
            } else if (event_para->action == SMCHARGER_CLOSE_LID_ACTION) {
                xiaoai_set_charger_state(XIAOAI_SMCHARGER_CLOSE_LID);
            } else if (event_para->action == SMCHARGER_CHARGER_OUT_ACTION) {
                xiaoai_set_charger_state(XIAOAI_SMCHARGER_OUT_OF_CASE);
            }
            break;
        }
        case SMCHARGER_EVENT_NOTIFY_BOTH_IN_OUT: {
            APPS_LOG_MSGID_I(LOG_TAG" SMCharger group, [%02X] NOTIFY_IN_OUT_EVENT", 1, role);
            if (role == BT_AWS_MCE_ROLE_AGENT) {
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI, XIAOAI_EVENT_DEVICE_ANTI_LOST_ACTION);
                ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                                    EVENT_GROUP_UI_SHELL_XIAOAI,
                                    XIAOAI_EVENT_DEVICE_ANTI_LOST_ACTION,
                                    NULL, 0, NULL, 500);
            }
            app_va_xiaoai_ota_check_both_close(FALSE);
            break;
        }
    }
#endif
    return ret;
}

void app_va_xiaoai_anti_lost_notify(void)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI, XIAOAI_EVENT_DEVICE_ANTI_LOST_ACTION);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                        EVENT_GROUP_UI_SHELL_XIAOAI,
                        XIAOAI_EVENT_DEVICE_ANTI_LOST_ACTION,
                        NULL, 0, NULL, 500);
}

#ifdef MTK_ANC_ENABLE
bool app_va_xiaoai_anc_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case AUDIO_ANC_CONTROL_EVENT_ON: {
            app_va_xiaoai_handle_anc_event(TRUE, extra_data);
            break;
        }
        case AUDIO_ANC_CONTROL_EVENT_OFF: {
            app_va_xiaoai_handle_anc_event(FALSE, NULL);
            break;
        }
    }
    return FALSE;
}
#endif

bool app_va_xiaoai_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    APPS_LOG_MSGID_I(LOG_TAG" XIAOAI group, event_id=0x%04X extra_data=0x%08X", 2, event_id, extra_data);
    /* Only XiaoAI activity should handle UI Shell XiaoAI event. */
    switch (event_id) {
        case XIAOAI_EVENT_UNBIND_ACTION: {
            g_xiaoai_unbind_flow_starting = FALSE;
            xiaoai_bt_unbind_start((bool)extra_data);
            break;
        }
        case XIAOAI_EVENT_RECONNECT_EDR_ACTION: {
            xiaoai_bt_reconnect_edr();
            break;
        }
        case XIAOAI_EVENT_DISCONNECT_EDR_ACTION: {
            // 0x06 only for iPhone BLE case
            // EDR could be disconnected by Android XiaoAI APP
            xiaoai_bt_disconnect_edr(TRUE, NULL, FALSE);
            break;
        }
        case XIAOAI_EVENT_START_BLE_ADV_ACTION: {
            /* Start XiaoAI BLE ADV: add XiaoAI from multi BLE ADV manager and notify. */
            uint16_t advertising_interval = (uint32_t)extra_data;
            app_va_xiaoai_start_ble_adv(TRUE, advertising_interval);
            break;
        }
        case XIAOAI_EVENT_STOP_BLE_ADV_ACTION: {
            /* Stop XiaoAI BLE ADV: remove XiaoAI from multi BLE ADV manager and notify. */
            app_va_xiaoai_start_ble_adv(FALSE, 0);
            break;
        }
        case XIAOAI_EVENT_DEVICE_CONFIG_ACTION: {
            if (extra_data != NULL) {
                bool lea_mma = app_xiaoai_is_lea_mma_link();
                bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
                xiaoai_device_config_t *param = (xiaoai_device_config_t *)(extra_data);
                if (lea_mma && role == BT_AWS_MCE_ROLE_PARTNER) {
                    uint8_t type = param->type;
                    // Need to run in Agent side
                    // Hotword/WWE or Not_support_feature should be run in XIAOAI Connection Side
                    if (app_va_xiaoai_need_run_in_agent(type)) {
                        bt_status_t status = BT_STATUS_SUCCESS;
                        if (param->data != NULL && param->length > 0) {
                            // ToDo, malloc buffer = sizeof(xiaoai_device_config_t) + length, then send
                        } else {
                            status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_XIAOAI,
                                                                    XIAOAI_EVENT_LEA_SYNC_DEVICE_CONFIG_TO_AGENT,
                                                                    (uint8_t *)param,
                                                                    sizeof(xiaoai_device_config_t));
                        }
                        APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK][Partner] aws_send, type=%d status=0x%08X",
                                         2, type, status);
                    } else {
                        app_va_xiaoai_set_device_config(TRUE, (void *)param);
                    }
                } else {
                    app_va_xiaoai_set_device_config(TRUE, (void *)param);
                }

                if (param->data != NULL && param->length > 0) {
                    vPortFree(param->data);
                }
            }
            break;
        }
        case XIAOAI_EVENT_AWS_ACTION: {
            bool aws_connected = (bool)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" XIAOAI group, AWS Action %d->%d mma_notify_pre=%d",
                             3, g_xiaoai_aws_connected, aws_connected, g_xiaoai_mma_notify_pre_aws_state);
            if (aws_connected != g_xiaoai_aws_connected
                || aws_connected != g_xiaoai_mma_notify_pre_aws_state) {
                app_va_xiaoai_aws_action(aws_connected);
            }
            break;
        }
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
        case XIAOAI_EVENT_DEVICE_LD_CHECK_IN_EAR: {
            bool ret = (bool)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" XIAOAI group, LD_CHECK_IN_EAR ret=%d", 1, ret);
            app_va_xiaoai_notify_leakage_detectable(ret);
            break;
        }
#endif
        case XIAOAI_EVENT_DEVICE_ANTI_LOST_ACTION: {
            uint8_t data = app_va_xiaoai_get_anti_lost_state();
            app_va_xiaoai_notify_device_config(XIAOAI_DEVICE_CONFIG_ANTI_LOST, &data, 1);
            app_va_xiaoai_hfp_miui_more_atcmd_report_anti_lost(data);
            break;
        }
#ifdef AIR_XIAOAI_PTT_ENABLE
        case XIAOAI_EVENT_START_PTT_ACTIVITY: {
            APPS_LOG_MSGID_I(LOG_TAG" XIAOAI group, START_PTT_ACTIVITY", 0);
            ui_shell_start_activity(self, app_va_xiaoai_ptt_sppech_activity_proc,
                                    ACTIVITY_PRIORITY_HIGH, NULL, 0);
            break;
        }
        case XIAOAI_EVENT_FINISH_PTT_ACTIVITY: {
            APPS_LOG_MSGID_I(LOG_TAG" XIAOAI group, FINISH_PTT_ACTIVITY", 0);
            break;
        }
#endif
#ifdef AIR_XIAOAI_WWE_ENABLE
        case XIAOAI_EVENT_START_WWE_ACTION: {
            bool ret = xiaoai_wwe_recorder_start();
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Event)start ret=%d", 1, ret);
            break;
        }
        case XIAOAI_EVENT_STOP_WWE_ACTION: {
            bool ret = xiaoai_wwe_recorder_stop();
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Event)stop ret=%d", 1, ret);
            break;
        }
        case XIAOAI_EVENT_RESTART_WWE_ACTION: {
            bool ret = xiaoai_wwe_recorder_stop();
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Event)restart, stop ret=%d", 1, ret);
            if (ret) {
                ret = xiaoai_wwe_recorder_start();
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Event)restart, ret=%d", 1, ret);
            }
            break;
        }
        case XIAOAI_EVENT_WWE_DETECTED_ACTION: {
            voice_prompt_play_vp_press();
            bool hfp_va_activate = app_va_xiaoai_wake_up();
            if (!hfp_va_activate) {
                app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_DETECTED);
                bool result = xiaoai_start_recognizing(XIAOAI_VA_TRIGGER_HOTWORD);
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Event)detected, start_recognizing ret=%d", 1, result);
            }
            break;
        }
        case XIAOAI_EVENT_WWE_DATA_ACTION: {
            uint32_t len = (uint32_t)extra_data;
            if (g_xiaoai_wwe_state == XIAOAI_WWE_STATE_DETECTED
                || g_xiaoai_wwe_state == XIAOAI_WWE_STATE_SENDING) {
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Event)data, len=%d", 1, len);
                xiaoai_app_handle_send_voice(len);
            } else {
                APPS_LOG_MSGID_E(LOG_TAG"[WWE] (Event)data, error_wwe_state=%d", 1, g_xiaoai_wwe_state);
#ifdef XIAOAI_USE_ATCMD_TEST_WWE
                // For Test when use WWE_MODE_NONE/AUDIO_DSP_CODEC_TYPE_PCM as WWE
                // Must consume PCM data when data notification
                uint8_t *pcm_buf = (uint8_t *)pvPortMalloc(len);
                if (pcm_buf == NULL) {
                    return TRUE;
                }
                record_control_result_t result = audio_record_control_read_data(pcm_buf, len);
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] control_event, consume PCM data result=%d",
                                 1, result);
                vPortFree(pcm_buf);
#endif
            }
            break;
        }
        case XIAOAI_EVENT_WWE_HFP_SUSPEND: {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Event)hfp_suspend", 0);
            app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_STOPPED);
            break;
        }
        case XIAOAI_EVENT_WWE_LE_CALL_SUSPEND: {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Event)le_call_suspend", 0);
            app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_STOPPED);
            break;
        }
        case XIAOAI_EVENT_WWE_LD_SUSPEND: {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Event)ld_suspend", 0);
            app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_STOPPED);
            g_xiaoai_wwe_need_restart = TRUE;
            break;
        }
        case XIAOAI_EVENT_WWE_AUDIO_DVFS_UNLOCK: {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] (Event)AUDIO_DVFS_UNLOCK", 0);
            xiaoai_port_dvfs_lock(FALSE, TRUE);
            break;
        }
#endif
        case XIAOAI_EVENT_OTA_SLIENCE_OTA_TIMEROUT: {
            app_va_xiaoai_ota_commit_reboot();
            break;
        }
    }
    return TRUE;
}

bool app_va_xiaoai_activity_proc(struct _ui_shell_activity *self,
                                 uint32_t event_group,
                                 uint32_t event_id,
                                 void *extra_data,
                                 size_t data_len)
{
    bool ret = FALSE;
    switch (event_group) {
        /* UI Shell internal events. */
        case EVENT_GROUP_UI_SHELL_SYSTEM:
            ret = app_va_xiaoai_activity_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        /* UI Shell APP_INTERACTION events - only for RHO. */
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            ret = app_va_xiaoai_interaction_event_group(self, event_id, extra_data, data_len);
            break;
        /* UI Shell key events - only support XiaoAI key event. */
        case EVENT_GROUP_UI_SHELL_KEY:
            ret = app_va_xiaoai_activity_proc_key_event(self, event_id, extra_data, data_len);
            break;
        /* UI Shell BT sink service events. */
        case EVENT_GROUP_UI_SHELL_BT_SINK:
            app_va_xiaoai_bt_sink_event_proc(self, event_id, extra_data, data_len);
            break;
        /* UI Shell BT Connection Manager events. */
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER:
            ret = app_va_xiaoai_bt_cm_event_proc(self, event_id, extra_data, data_len);
            break;
        /* UI Shell BT Device Manager events. */
        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER:
            ret = app_va_xiaoai_bt_dm_event_proc(self, event_id, extra_data, data_len);
            break;
#ifdef MTK_AWS_MCE_ENABLE
        /* UI Shell BT AWS_DATA events. */
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            ret = app_va_xiaoai_activity_proc_aws_data(self, event_id, extra_data, data_len);
            break;
#endif
        /* APP SmartCharger events. */
        case EVENT_GROUP_UI_SHELL_CHARGER_CASE:
            ret = app_va_xiaoai_smcharger_event_proc(self, event_id, extra_data, data_len);
            break;
            /* APP ANC events. */
#ifdef MTK_ANC_ENABLE
        case EVENT_GROUP_UI_SHELL_AUDIO_ANC:
            ret = app_va_xiaoai_anc_event_proc(self, event_id, extra_data, data_len);
            break;
#endif
        /* APP XiaoAI events. */
        case EVENT_GROUP_UI_SHELL_XIAOAI:
            ret = app_va_xiaoai_event_proc(self, event_id, extra_data, data_len);
            break;
    }
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
    miui_fast_connect_proc_ui_shell_event(event_group, event_id, extra_data, data_len);
#endif
    return ret;
}

/* XiaoAI UT code by using AT CMD. */
#ifdef APP_XIAOAI_TEST

static atci_status_t app_xiaoai_atci_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            char *atcmd = parse_cmd->string_ptr + parse_cmd->name_len + 1;
            char cmd[20] = {0};
            memcpy(cmd, atcmd, strlen(atcmd) - 2);
            APPS_LOG_I(LOG_TAG" XiaoAI AT CMD=%s", cmd);
            if (strstr(cmd, "XAVER") > 0) {
                memset(response.response_buf, 0, ATCI_UART_TX_FIFO_BUFFER_SIZE);
                snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                         "XiaoAI Version 0x%04X\r\n", APP_VA_XIAOAI_VERSION);
                break;
            } else if (strstr(cmd, "more_test") > 0) {
                xiaoai_update_vidpid_for_more_test(1);
                APPS_LOG_MSGID_I(LOG_TAG" XiaoAI more Test", 0);
            } else if (strstr(cmd, "chat") > 0) {
                int x = 0;
                sscanf(cmd, "chat,%d", &x);
                APPS_LOG_MSGID_I(LOG_TAG" XiaoAI Chat %d", 1, x);
                app_va_xiaoai_handle_chat_event(x == 1);
            } else if (strstr(cmd, "ble_conn") > 0) {
                static bool up_ble_conn = TRUE;
                if (up_ble_conn) {
                    xiaoai_update_ble_conn_interval(20);
                } else {
                    xiaoai_update_ble_conn_interval(30);
                }
                up_ble_conn = !up_ble_conn;
            } else if (strstr(cmd, "test") > 0) {
//                bt_hfp_custom_command_result_ind_t ind;
//                ind.result = "+XIAOMI: FF010201030901songnameFF"; // name
//                ind.result = "+XIAOMI: FF01020101020000FF"; // all
//                ind.result = "+XIAOMI: FF01020101020500FF"; // key
//                app_va_xiaoai_hfp_callback(BT_HFP_CUSTOM_COMMAND_RESULT_IND, BT_STATUS_SUCCESS, &ind);
            } else if (strstr(cmd, "slience_ota") > 0) {
                int x = 0;
                sscanf(cmd, "slience_ota,%d", &x);
                bool slience_ota = (x == 1);
                app_va_xiaoai_ota_slience_enable(slience_ota, TRUE);
#ifdef MTK_IN_EAR_FEATURE_ENABLE
            } else if (strstr(cmd, "in_ear") > 0) {
                int x = 0;
                sscanf(cmd, "in_ear,%d", &x);
                bool in_ear_flag = (x == 1);
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT);
                ui_shell_send_event(false, EVENT_PRIORITY_HIGNEST,
                                    EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_UPDATE_IN_EAR_STA_EFFECT,
                                    (void *)&in_ear_flag, 0,
                                    NULL, 500);
#endif
#if defined(AIR_XIAOAI_WWE_ENABLE) && defined(XIAOAI_USE_ATCMD_TEST_WWE)
            } else if (strstr(cmd, "wwd") > 0) {
                if (g_xiaoai_wwe_state == XIAOAI_WWE_STATE_STARTED) {
                    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                        EVENT_GROUP_UI_SHELL_XIAOAI,
                                        XIAOAI_EVENT_WWE_DETECTED_ACTION,
                                        NULL, 0, NULL, 0);
                }
#endif
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" invalid SMCharger AT-CMD", 0);
            }
            memset(response.response_buf, 0, ATCI_UART_TX_FIFO_BUFFER_SIZE);
            snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "OK - %s\r\n", atcmd);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}
#endif

#ifdef AIR_XIAOAI_WWE_ENABLE
static void app_va_xiaoai_recorder_event_notify(xiaoai_va_recorder_event recorder_event)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bool enable_wwe = app_va_xiaoai_is_enable_wwe();
    APPS_LOG_MSGID_I(LOG_TAG"[WWE][%02X] VA recorder_event=%d enable_wwe=%d wwe_need_restart=%d",
                     4, role, recorder_event, enable_wwe, g_xiaoai_wwe_need_restart);
    if (recorder_event == XIAOAI_VA_RECORDER_STARTED_EVENT) {

    } else if (recorder_event == XIAOAI_VA_RECORDER_STOPPED_EVENT) {
        // Need to start WWE when VA Normal recorder stopped
        if (enable_wwe) {
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_START_WWE_ACTION,
                                NULL, 0, NULL, 0);
        }
    } else if (recorder_event == XIAOAI_VA_NEED_STOP_WWE_EVENT) {
        // Must stop WWE for multi-turn (recv D0 CMD from phone)
        if (enable_wwe) {
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_STOP_WWE_ACTION,
                                NULL, 0, NULL, 0);
        }
    } else if (recorder_event == XIAOAI_VA_VENDOR_SE_LD_STOP_EVENT) {
        // LD Stop from audio vendor_se callback
        if (g_xiaoai_wwe_need_restart && enable_wwe) {
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_START_WWE_ACTION,
                                NULL, 0, NULL, 0);
        }
    } else if (recorder_event == XIAOAI_VA_WWE_START_SEND_EVENT) {
        // Update WWE State=Sending(WWD->Send D0->Recv D0 RSP->Start Send)
        app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_SENDING);
    } else if (recorder_event == XIAOAI_VA_NEED_RESTART_WWE_EVENT) {
        // WWE: stop WWE and restart when WWE sending state + recv D1 CMD
        if (enable_wwe) {
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_RESTART_WWE_ACTION,
                                NULL, 0, NULL, 0);
            bool lea_mma = app_xiaoai_is_lea_mma_link();
            if (lea_mma) {
                bt_status_t bt_status = apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_XIAOAI,
                                                                 XIAOAI_EVENT_RESTART_WWE_ACTION);
                APPS_LOG_MSGID_I(LOG_TAG"[WWE][LEA] sync restart_action to peer, bt_status=0x%08X",
                                 1, bt_status);
            }
        } else {
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_STOP_WWE_ACTION,
                                NULL, 0, NULL, 0);
        }
    }
}

static void app_va_xiaoai_wwe_update_state(xiaoai_wwe_state new_state)
{
    uint8_t old_state = g_xiaoai_wwe_state;
    APPS_LOG_MSGID_I(LOG_TAG"[WWE] state %d->%d", 2, old_state, new_state);
    if (old_state != new_state) {

        if (new_state == XIAOAI_WWE_STATE_STARTING || new_state == XIAOAI_WWE_STATE_DETECTED) {
            xiaoai_port_dvfs_lock(TRUE, TRUE);
        } else if ((old_state == XIAOAI_WWE_STATE_DETECTED || old_state == XIAOAI_WWE_STATE_SENDING)
                   && new_state == XIAOAI_WWE_STATE_STOPPING) {
            xiaoai_port_dvfs_lock(FALSE, TRUE);
        } else if (old_state == XIAOAI_WWE_STATE_STARTING
                   && new_state == XIAOAI_WWE_STATE_STOPPED) {
            xiaoai_port_dvfs_lock(FALSE, TRUE);
        } else if (old_state == XIAOAI_WWE_STATE_STARTING
                   && new_state == XIAOAI_WWE_STATE_STARTED) {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI, XIAOAI_EVENT_WWE_AUDIO_DVFS_UNLOCK);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_WWE_AUDIO_DVFS_UNLOCK,
                                NULL, 0, NULL, 3000);
        }

        g_xiaoai_wwe_state = new_state;
    }
}

void xiaoai_wwe_control_event_callback(hal_audio_event_t event, void *user_data)
{
    switch (event) {
        case HAL_AUDIO_EVENT_ERROR: {
            APPS_LOG_MSGID_E(LOG_TAG"[WWE] control_event, error", 0);
            break;
        }
        case HAL_AUDIO_EVENT_WWD_NOTIFICATION: {
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            xiaoai_connection_state conn_state = xiaoai_get_connection_state();
            bool peer_lea_mma = xiaoai_peer_is_lea_mma_link();
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] control_event, XIAOAI WWD_NOTIFICATION [%02X] conn_state=%d peer_lea_mma=%d",
                             3, role, conn_state, peer_lea_mma);
            // Only For XIAOAI Hotword
            if (conn_state == XIAOAI_STATE_CONNECTED || (!peer_lea_mma && role == BT_AWS_MCE_ROLE_AGENT)) {
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                    EVENT_GROUP_UI_SHELL_XIAOAI,
                                    XIAOAI_EVENT_WWE_DETECTED_ACTION,
                                    NULL, 0, NULL, 0);
            } else {
                APPS_LOG_MSGID_E(LOG_TAG"[WWE] control_event, XIAOAI WWD but NO XiaoAI Connection", 0);
            }
            break;
        }
        case HAL_AUDIO_EVENT_DATA_NOTIFICATION: {
            uint32_t len = audio_record_control_get_share_buf_data_byte_count();
#define XIAOAI_PCM_LEN_IN_ONE_OPUS_FRAME    640
            uint32_t opus_frame_num = len / XIAOAI_PCM_LEN_IN_ONE_OPUS_FRAME;
            if (opus_frame_num >= 1) {
                ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                    EVENT_GROUP_UI_SHELL_XIAOAI,
                                    XIAOAI_EVENT_WWE_DATA_ACTION,
                                    (void *)len, 0, NULL, 0);
            } else {
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] control_event, DATA error len=%d opus_frame_num=%d",
                                 2, len, opus_frame_num);
            }
            break;
        }
        default:
            break;
    }
}

static void xiaoai_wwe_am_callback(bt_sink_srv_am_id_t aud_id,
                                   bt_sink_srv_am_cb_msg_class_t msg_id,
                                   bt_sink_srv_am_cb_sub_msg_t sub_msg,
                                   void *parm)
{
    g_xiaoai_wwe_error_flag = FALSE;
    if (msg_id == AUD_SINK_OPEN_CODEC) {
        if (sub_msg == AUD_CMD_COMPLETE) {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] AM open complete", 0);
            xSemaphoreGive(g_xiaoai_wwe_semaphore);
        } else if (sub_msg == AUD_CMD_FAILURE) {
            APPS_LOG_MSGID_E(LOG_TAG"[WWE] AM open failure", 0);
            g_xiaoai_wwe_error_flag = TRUE;
            xSemaphoreGive(g_xiaoai_wwe_semaphore);
        }
    } else if (msg_id == AUD_SELF_CMD_REQ) {
        if (sub_msg == AUD_CMD_COMPLETE) {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] AM close complete", 0);
            xSemaphoreGive(g_xiaoai_wwe_semaphore);
        } else if (sub_msg == AUD_CMD_FAILURE) {
            APPS_LOG_MSGID_E(LOG_TAG"[WWE] AM close failure", 0);
            g_xiaoai_wwe_error_flag = TRUE;
            xSemaphoreGive(g_xiaoai_wwe_semaphore);
        }
    }

    if (msg_id == AUD_SUSPEND_BY_IND) {
        if (sub_msg == AUD_SUSPEND_BY_HFP) {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] AM SUSPEND_BY_HFP", 0);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_WWE_HFP_SUSPEND,
                                NULL, 0, NULL, 0);
        } else if (sub_msg == AUD_SUSPEND_BY_LE_CALL) {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] AM AUD_SUSPEND_BY_LE_CALL", 0);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_WWE_LE_CALL_SUSPEND,
                                NULL, 0, NULL, 0);
        } else if (sub_msg == AUD_SUSPEND_BY_LC) {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] AM SUSPEND_BY_LC", 0);
            xiaoai_set_leakage_detection_suspend(TRUE);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_WWE_LD_SUSPEND,
                                NULL, 0, NULL, 0);
        }
    }
}

static void xiaoai_wwe_recorder_init()
{
    if (g_xiaoai_wwe_record_id == -1) {
        record_encoder_cability_t encoder;
#ifdef XIAOAI_USE_ATCMD_TEST_WWE
        encoder.wwe_mode = WWE_MODE_NONE;
        encoder.codec_type = AUDIO_DSP_CODEC_TYPE_PCM;
        encoder.bit_rate = ENCODER_BITRATE_16KBPS;
#else
        encoder.wwe_mode = WWE_MODE_XXX;     // Customer configure option: WWE_MODE_XIAOAI
        encoder.bit_rate = ENCODER_BITRATE_16KBPS;
        encoder.codec_type = AUDIO_DSP_CODEC_TYPE_PCM_WWE;
        encoder.wwe_language_mode_address = NULL;
        encoder.wwe_language_mode_length = 0;
#endif
        g_xiaoai_wwe_record_id = audio_record_control_enabling_encoder_init(
                                     xiaoai_wwe_control_event_callback,
                                     NULL,
                                     xiaoai_wwe_am_callback,
                                     &encoder);

        APPS_LOG_MSGID_I(LOG_TAG"[WWE] wwe_recorder_init id=%d", 1, g_xiaoai_wwe_record_id);
        g_xiaoai_wwe_state = XIAOAI_WWE_STATE_STOPPED;

        g_xiaoai_wwe_semaphore = xSemaphoreCreateBinary();
        if (g_xiaoai_wwe_semaphore == NULL) {
            configASSERT(0);
        }
    }
}

static bool xiaoai_wwe_recorder_start()
{
    bool ret = FALSE;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bool lea_mma = app_xiaoai_is_lea_mma_link();
    if (!lea_mma && role != BT_AWS_MCE_ROLE_AGENT) {
        APPS_LOG_MSGID_I(LOG_TAG"[WWE] start fail, not Agent role & !lea_mma", 0);
        return ret;
    } else if (app_va_xiaoai_is_call_ongoing()) {
        APPS_LOG_MSGID_I(LOG_TAG"[WWE] start fail, call ongoing", 0);
        return ret;
    }

    if (g_xiaoai_wwe_record_id != -1) {
        if (g_xiaoai_wwe_state == XIAOAI_WWE_STATE_STOPPED) {
            app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_STARTING);
            record_control_result_t result = audio_record_control_start(g_xiaoai_wwe_record_id);
            if (result == RECORD_CONTROL_EXECUTION_SUCCESS) {
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] starting, state=%d", 1, g_xiaoai_wwe_state);
                if (xSemaphoreTake(g_xiaoai_wwe_semaphore, XIAOAI_WWE_RECORDER_MAX_DELAY_TIME) == pdTRUE) {
                    if (g_xiaoai_wwe_error_flag) {
                        g_xiaoai_wwe_error_flag = FALSE;
                        app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_STOPPED);
                    } else {
                        app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_STARTED);
                        APPS_LOG_MSGID_I(LOG_TAG"[WWE] start done, state=%d", 1, g_xiaoai_wwe_state);
                        ret = TRUE;
                    }
                } else {
                    APPS_LOG_MSGID_E(LOG_TAG"[WWE] start fail, take timeout", 0);
                    app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_STOPPED);
                }
            } else {
                APPS_LOG_MSGID_E(LOG_TAG"[WWE] start fail, result=%d", 1, result);
                app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_STOPPED);
            }
        } else if (g_xiaoai_wwe_state == XIAOAI_WWE_STATE_STARTED) {
            APPS_LOG_MSGID_E(LOG_TAG"[WWE] start, already start", 0);
            ret = TRUE;
        } else {
            APPS_LOG_MSGID_E(LOG_TAG"[WWE] start, error state=%d", 1, g_xiaoai_wwe_state);
        }
    } else {
        APPS_LOG_MSGID_E(LOG_TAG"[WWE] start fail, NULL wwe_record_id", 0);
    }

    if (ret) {
        g_xiaoai_wwe_need_restart = FALSE;
    }
    return ret;
}

static bool xiaoai_wwe_recorder_stop()
{
    bool ret = FALSE;
    if (g_xiaoai_wwe_record_id != -1) {
        if (g_xiaoai_wwe_state == XIAOAI_WWE_STATE_STARTED
            || g_xiaoai_wwe_state == XIAOAI_WWE_STATE_DETECTED
            || g_xiaoai_wwe_state == XIAOAI_WWE_STATE_SENDING) {
            record_control_result_t result = audio_record_control_stop(g_xiaoai_wwe_record_id);
            if (result == RECORD_CONTROL_EXECUTION_SUCCESS) {
                app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_STOPPING);
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] stopping, state=%d", 1, g_xiaoai_wwe_state);
                if (xSemaphoreTake(g_xiaoai_wwe_semaphore, XIAOAI_WWE_RECORDER_MAX_DELAY_TIME) == pdTRUE) {
                    if (g_xiaoai_wwe_error_flag) {
                        g_xiaoai_wwe_error_flag = FALSE;
                    } else {
                        app_va_xiaoai_wwe_update_state(XIAOAI_WWE_STATE_STOPPED);
                        APPS_LOG_MSGID_I(LOG_TAG"[WWE] stop done, state=%d", 1, g_xiaoai_wwe_state);
                        ret = TRUE;
                    }
                } else {
                    APPS_LOG_MSGID_E(LOG_TAG"[WWE] stop fail, take timeout", 0);
                }
            } else {
                APPS_LOG_MSGID_I(LOG_TAG"[WWE] stop fail, result=%d", 1, result);
            }
        } else if (g_xiaoai_wwe_state == XIAOAI_WWE_STATE_STOPPED) {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] stop, already stop", 0);
            ret = TRUE;
        } else {
            APPS_LOG_MSGID_I(LOG_TAG"[WWE] stop, error state=%d", 1, g_xiaoai_wwe_state);
        }
    } else {
        APPS_LOG_MSGID_I(LOG_TAG"[WWE] stop fail, NULL wwe_record_id", 0);
    }
    return ret;
}

#endif /* AIR_XIAOAI_WWE_ENABLE */



#endif /* AIR_XIAOAI_ENABLE */
