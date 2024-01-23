/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#include "app_le_audio.h"
#include "app_le_audio_utillity.h"

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
#include "bt_le_audio_source.h"
#include "app_le_audio_ucst.h"
#include "app_le_audio_ucst_utillity.h"
#include "usb_hid_srv.h"
#ifdef MTK_RACE_CMD_ENABLE
#include "app_le_audio_race.h"
#endif
#endif

#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_le_audio_bcst.h"
#endif

#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
#include "app_le_audio_line_in.h"
#endif

#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
#include "app_le_audio_i2s_in.h"
#endif

#ifdef AIR_LE_AUDIO_BA_ENABLE
#include "app_le_audio_ba.h"
#endif
#include "app_le_audio_air.h"
#include "app_dongle_connection_common.h"

#include "bt_system.h"
#include "apps_events_event_group.h"

#include "bt_callback_manager.h"

#include "bt_gap.h"

#include "bt_le_audio_msglog.h"
#ifdef MTK_BLE_GAP_SRV_ENABLE
#include "bt_gap_le_service.h"
#endif

#include "app_le_audio_ccp_call_control_server.h"

#include "app_dongle_session_manager.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define LOG_TAG                 "[le_audio_activity]"
#define APP_LE_AUDIO_DONGLE     1


/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/
extern app_le_audio_ctrl_t g_lea_ctrl;
//static bool g_lea_disable_page_scan = false;
static bool g_lea_test_mode = false;

/**************************************************************************************************
* Prototype
**************************************************************************************************/
#ifdef AIR_LE_AUDIO_BA_ENABLE
extern bt_status_t ble_bap_gap_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff);
#endif
extern void app_lea_dongle_customer_init(void);

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:app_lea_dongle_customer_init=default_app_lea_dongle_customer_init")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak app_lea_dongle_customer_init = default_app_lea_dongle_customer_init
#else
#error "Unsupported Platform"
#endif

void default_app_lea_dongle_customer_init(void)
{
}

/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static bool app_le_audio_handle_idle_system_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            LE_AUDIO_MSGLOG_I(LOG_TAG", create", 0);
            break;
        }
        default:
            break;
    }
    return ret;
}

static bool app_le_audio_handle_idle_interaction_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    return false;
}

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
static bool app_le_audio_handle_idle_session_manager_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    switch (event_id) {
        case APP_DONGLE_SESSION_MGR_RESTART: {
            if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
                app_le_audio_ucst_stop(true);
            }
            break;
        }
        default:
            break;
    }
    return ret;
}
#endif

static void app_le_audio_handle_bt_power_on_cnf(void)
{
    uint8_t mode = APP_LE_AUDIO_DONGLE;

    LE_AUDIO_MSGLOG_I("[APP] BT_POWER_ON_CNF", 0);

    bt_hci_send_vendor_cmd(0xFDCC, (uint8_t *)&mode, sizeof(uint8_t));

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    app_le_audio_csip_handle_power_on();
#endif
}

static void app_le_audio_handle_bt_power_off_cnf(void)
{
    LE_AUDIO_MSGLOG_I("[APP] BT_POWER_OFF_CNF", 0);

#ifdef AIR_SILENCE_DETECTION_ENABLE
    app_le_audio_silence_detection_handle_bt_off();
#endif
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    app_le_audio_csip_handle_power_off();

    app_le_audio_ucst_reset_param();

#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    app_le_audio_bcst_reset_info_ex();
#endif
    g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_NONE;
    g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;

    if (g_lea_ctrl.open_audio_transmitter) {
        app_le_audio_close_audio_transmitter();
    }
}

static bt_status_t app_le_audio_handle_event_callback(bt_msg_type_t msg, bt_status_t ret, void *buff)
{
    switch (msg) {
        case BT_POWER_ON_CNF: {
            app_le_audio_handle_bt_power_on_cnf();
            break;
        }
        case BT_POWER_OFF_CNF: {
            app_le_audio_handle_bt_power_off_cnf();
            break;
        }
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        case BT_GAP_LE_SET_SCAN_CNF:
        case BT_GAP_LE_SET_EXTENDED_SCAN_CNF: {
            app_le_audio_ucst_handle_scan_cnf(ret);
            break;
        }
        case BT_GAP_LE_EXT_ADVERTISING_REPORT_IND: {
            app_le_audio_ucst_handle_adv_report_ind(ret, (bt_gap_le_ext_advertising_report_ind_t *)buff);
            break;
        }
        case BT_GAP_LE_SET_WHITE_LIST_CNF: {
            app_le_audio_ucst_handle_set_white_list_cnf(ret);
            break;
        }
        /*case BT_GAP_LE_CONNECT_IND: {
            LE_AUDIO_MSGLOG_I("[APP] LE_CONNECT_IND, handle:0x%x, ret:0x%x", 2, ((bt_gap_le_connection_ind_t *)buff)->connection_handle, ret);
            break;
        }*/
        case BT_GAP_LE_CONNECTION_UPDATE_IND: {
            app_le_audio_ucst_handle_connection_update_ind(ret, (bt_gap_le_connection_update_ind_t *)buff);
            break;
        }
        case BT_GAP_LE_BONDING_COMPLETE_IND: {
            app_le_audio_ucst_handle_bonding_complete_ind(ret, (bt_gap_le_bonding_complete_ind_t *)buff);
            break;
        }
        case BT_GATTC_EXCHANGE_MTU: {
            app_le_audio_ucst_handle_exchange_mtu_rsp(ret, (bt_gatt_exchange_mtu_rsp_t *)buff);
            break;
        }
        case BT_GAP_LE_SET_CIG_PARAMETERS_TEST_CNF:
        case BT_GAP_LE_SET_CIG_PARAMETERS_CNF: {
            app_le_audio_ucst_handle_set_cig_parameter_cnf(ret, (bt_gap_le_set_cig_params_cnf_t *)buff);
            break;
        }
        case BT_GAP_LE_CIS_ESTABLISHED_IND: {
            app_le_audio_ucst_handle_cis_established_ind(ret, (bt_gap_le_cis_established_ind_t *)buff);
            break;
        }
        case BT_GAP_LE_CIS_TERMINATED_IND: {
            app_le_audio_ucst_handle_cis_terminated_ind(ret, (bt_gap_le_cis_terminated_ind_t *)buff);
            break;
        }
        case BT_GAP_LE_REMOVE_CIG_CNF: {
            app_le_audio_ucst_handle_remove_cig_cnf(ret, (bt_gap_le_remove_cig_cnf_t *)buff);
            app_le_audio_ucst_check_active_device_idle();
            break;
        }
        case BT_GAP_LE_DISCONNECT_IND: {
            app_le_audio_ucst_handle_disconnect_ind(ret, (bt_hci_evt_disconnect_complete_t *)buff);
#ifdef AIR_LE_AUDIO_BA_ENABLE
            app_le_audio_ba_handle_disconnect_ind(ret, (bt_hci_evt_disconnect_complete_t *)buff);
#endif
            break;
        }
#endif
        case BT_GAP_LE_SETUP_ISO_DATA_PATH_CNF: {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
            if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
                app_le_audio_ucst_handle_setup_iso_data_path_cnf(ret, (bt_gap_le_setup_iso_data_path_cnf_t *)buff);
                break;
            }
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
            if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) {
                app_le_audio_bcst_handle_setup_iso_data_path_cnf(ret, (bt_gap_le_setup_iso_data_path_cnf_t *)buff);
            }
#endif
            break;
        }
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        case BT_GAP_LE_REMOVE_ISO_DATA_PATH_CNF: {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
            if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
                break;
            }
#endif
            if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) {
                app_le_audio_bcst_handle_remove_iso_data_path_cnf(ret, (bt_gap_le_remove_iso_data_path_cnf_t *)buff);
            }
            break;
        }
        case BT_GAP_LE_CONFIG_EXTENDED_ADVERTISING_CNF: {
            app_le_audio_bcst_handle_config_extended_advertising_cnf(ret, (bt_gap_le_config_extended_advertising_cnf_t *)buff);
            break;
        }
        case BT_GAP_LE_ENABLE_EXTENDED_ADVERTISING_CNF: {
            app_le_audio_bcst_handle_enable_extended_advertising_cnf(ret, (bt_gap_le_enable_extended_advertising_cnf_t *)buff);
            break;
        }
        case BT_GAP_LE_CONFIG_PERIODIC_ADVERTISING_CNF: {
            app_le_audio_bcst_handle_config_periodic_advertising_cnf_t(ret, (bt_gap_le_config_periodic_advertising_cnf_t *)buff);
            break;
        }
        case BT_GAP_LE_ENABLE_PERIODIC_ADVERTISING_CNF: {
            app_le_audio_bcst_handle_enable_periodic_advertising_cnf(ret, (bt_gap_le_enable_periodic_advertising_cnf_t *)buff);
            break;
        }
        case BT_GAP_LE_CREATE_BIG_CNF: {
            LE_AUDIO_MSGLOG_I("[APP][B] LE_CREATE_BIG_CNF, ret:%x", 1, ret);
            break;
        }
        case BT_GAP_LE_TERMINATE_BIG_CNF: {
            LE_AUDIO_MSGLOG_I("[APP][B] LE_TERMINATE_BIG_CNF, ret:%x", 1, ret);
            break;
        }
        case BT_GAP_LE_BIG_ESTABLISHED_IND: {
            app_le_audio_bcst_handle_big_established_ind(ret, (bt_gap_le_big_established_ind_t *)buff);
            break;
        }
        case BT_GAP_LE_BIG_TERMINATED_IND: {
            app_le_audio_bcst_handle_big_terminated_ind(ret, (bt_gap_le_big_terminated_ind_t *)buff);
            break;
        }
        case BT_GAP_LE_BIG_SYNC_ESTABLISHED_IND: {
            LE_AUDIO_MSGLOG_I("[APP][B] LE_BIG_SYNC_ESTABLISHED_IND", 0);
            break;
        }
        case BT_GAP_LE_BIG_SYNC_LOST_IND: {
            LE_AUDIO_MSGLOG_I("[APP][B] LE_BIG_SYNC_LOST_IND", 0);
            break;
        }
#endif
        /*
        case BT_GAP_SET_SCAN_MODE_CNF: {
            LE_AUDIO_MSGLOG_I("[APP] BT_GAP_SET_SCAN_MODE_CNF, ret:%x", 1, ret);
            if (!g_lea_disable_page_scan) {
                g_lea_disable_page_scan = true;
                bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_NOT_ACCESSIBLE);
            }
            break;
        }
        */
#ifdef AIR_LE_AUDIO_BA_ENABLE
        case BT_GAP_LE_PERIODIC_ADVERTISING_SYNC_TRANSFER_CNF:
        case BT_GAP_LE_PERIODIC_ADVERTISING_SET_INFO_TRANSFER_CNF: {
            app_le_audio_ba_handle_sync_transfer_cnf(msg, buff);
            break;
        }
#endif
    }
#ifdef AIR_LE_AUDIO_BA_ENABLE
    if (APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE == app_le_audio_ba_get_mode()) {
        ble_bap_gap_event_callback(msg, ret, buff);
    }
#endif
    return BT_STATUS_SUCCESS;
}


static void app_le_audio_read_test_mode(void)
{
#if 0
    nvkey_status_t nvkey_status = NVKEY_STATUS_OK;
    uint32_t size = sizeof(g_lea_test_mode);

    /* NVKEYID_BT_LE_AUDIO_TEST_MODE: (1 byte) */
    nvkey_status = nvkey_read_data(NVKEYID_BT_LE_AUDIO_TEST_MODE, &g_lea_test_mode, &size);
    if (NVKEY_STATUS_OK != nvkey_status) {
        LE_AUDIO_MSGLOG_I("[APP][state] read NVKEYID_BT_LE_AUDIO_TEST_MODE, nvkey_status:%x", 1, nvkey_status);

        g_lea_test_mode = false;
        nvkey_status = nvkey_write_data(NVKEYID_BT_LE_AUDIO_TEST_MODE, &g_lea_test_mode, size);
        if (NVKEY_STATUS_OK != nvkey_status) {
            LE_AUDIO_MSGLOG_I("[APP][state] read_test_mode, write nvkey_status:%x", 1, nvkey_status);
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][state] app_le_audio_read_test_mode, enable:%x", 1, g_lea_test_mode);
#endif
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
bool app_le_audio_idle_activity_proc(ui_shell_activity_t *self,
                                     uint32_t event_group,
                                     uint32_t event_id,
                                     void *extra_data,
                                     size_t data_len)
{
    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            ret = app_le_audio_handle_idle_system_event_proc(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            app_le_audio_handle_idle_interaction_event(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_USB_AUDIO: {
            ret = app_le_audio_idle_usb_event_proc(self, event_id, extra_data, data_len);
            return false;
            break;
        }
        case EVENT_GROUP_UI_SHELL_LE_AUDIO: {
            ret = app_le_audio_handle_idle_le_audio_event(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANSMITTER: {
            ret = app_le_audio_handle_idle_transmitter_event(self, event_id, extra_data, data_len);
            break;
        }
#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
        case EVENT_GROUP_UI_SHELL_LINE_IN: {
            app_le_audio_idle_line_in_event_proc(self, event_id, extra_data, data_len);
            return false;
            break;
        }
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
        case EVENT_GROUP_UI_SHELL_I2S_IN: {
            app_le_audio_idle_i2s_in_event_proc(self, event_id, extra_data, data_len);
            return false;
            break;
        }
#endif
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        case EVENT_GROUP_UI_SHELL_USB_HID_CALL: {
            ret = app_le_audio_handle_idle_usb_hid_call_event(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_APP_SESSION_MANAGER: {
            app_le_audio_handle_idle_session_manager_event(self, event_id, extra_data, data_len);
            break;
        }
#endif
        default:
            break;
    }

    return ret;
}

bt_status_t app_le_audio_start_unicast_ex(const bt_addr_t *addr)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    LE_AUDIO_MSGLOG_I("[APP] start_unicast, curr_mode:%x next_mode:%x", 2, g_lea_ctrl.curr_mode, g_lea_ctrl.next_mode);

#ifdef AIR_LE_AUDIO_BA_ENABLE
    if (APP_LE_AUDIO_MODE_ASIT == g_lea_ctrl.next_mode) {
        app_le_audio_ba_stop_assistant();
    }
#endif

#ifdef AIR_LE_AUDIO_BIS_ENABLE
    if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) {
        LE_AUDIO_MSGLOG_I("[APP] start_unicast, need to stop broadcast, please wait!", 0);
        if (!app_le_audio_bcst_stop(false)) {
            g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_UCST;
            return BT_STATUS_SUCCESS;
        }
    }
#endif
    if (!((APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) && (APP_LE_AUDIO_MODE_NONE == g_lea_ctrl.next_mode))) {
        g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_UCST;
        g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
        app_le_audio_usb_refresh_volume();

        app_le_audio_ucst_resume_ex(APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL);

        if (NULL == addr) {
            if (0 == app_le_audio_ucst_get_link_num()) {
                app_le_audio_ucst_find_device();
            } else {
                //BA is always connected, Unicast must notify CM SOURCE_STARTED when switched to CIS from BIS.
                app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_CIS;
                app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_STARTED, BT_STATUS_SUCCESS, &lea_mode);
            }
            return BT_STATUS_SUCCESS;
        }
    }

    /* Create connection */
    if (NULL != addr) {
        bt_bd_addr_t empty_addr = {0};

        if (0 == memcmp(addr->addr, empty_addr, sizeof(bt_bd_addr_t))) {
            return BT_STATUS_SUCCESS;
        }
        if (!app_le_audio_ucst_is_connected_device(addr)) {
            LE_AUDIO_MSGLOG_I("[APP] start_unicast, connect device", 0);
            app_le_audio_ucst_check_group_device_bond(addr);
            app_le_audio_ucst_connect_device(addr);
        }
    }

    return BT_STATUS_SUCCESS;
#else
    return BT_STATUS_UNSUPPORTED;
#endif

}


bt_status_t app_le_audio_start_unicast(void)
{
    return app_le_audio_start_unicast_ex(NULL);
}

bt_status_t app_le_audio_start_broadcast(void)
{
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    LE_AUDIO_MSGLOG_I("[APP] start_broadcast, curr_mode:%x next_mode:%x", 2, g_lea_ctrl.curr_mode, g_lea_ctrl.next_mode);

#ifdef AIR_LE_AUDIO_BA_ENABLE
    if (APP_LE_AUDIO_MODE_ASIT == g_lea_ctrl.next_mode) {
        app_le_audio_ba_stop_assistant();
    }
#endif

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
        /* [Switch streaming mode] UCST -> BCST */
        if (0 != app_le_audio_ucst_get_link_num()) {
            g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_BCST;
#ifdef AIR_MS_TEAMS_ENABLE
            if (app_le_audio_usb_hid_call_existing()) {
                LE_AUDIO_MSGLOG_I("[APP] start_broadcast, please wait call ended!", 0);
                return BT_STATUS_SUCCESS;
            }
#endif
            LE_AUDIO_MSGLOG_I("[APP] start_broadcast, need to stop unicast streaming, please wait!", 0);

            if (app_le_audio_ucst_pause_ex(APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL)) {
                return BT_STATUS_SUCCESS;
            }
        }

        app_le_audio_ucst_stop_scan_all();
    }
#endif

    g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_BCST;
    g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
    app_le_audio_usb_refresh_volume();

    /* start bis */
    app_le_audio_bcst_start();
    return BT_STATUS_SUCCESS;
#else
    return BT_STATUS_UNSUPPORTED;
#endif
}


bt_status_t app_le_audio_stop_broadcast(void)
{
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    /* Check current streaming mode */
    if (APP_LE_AUDIO_MODE_BCST != g_lea_ctrl.curr_mode) {
        LE_AUDIO_MSGLOG_I("[APP] stop_broadcast, not streaming now!", 0);
        return BT_STATUS_FAIL;
    }

    if (!app_le_audio_bcst_stop(false)) {
        return BT_STATUS_PENDING;
    }

    return BT_STATUS_SUCCESS;
#else
    return BT_STATUS_UNSUPPORTED;
#endif
}

bt_status_t app_le_audio_start_broadcast_assistant(void)
{
#ifdef AIR_LE_AUDIO_BA_ENABLE
    LE_AUDIO_MSGLOG_I("[APP] start_broadcast_assistant, curr_mode:%x next_mode:%x", 2, g_lea_ctrl.curr_mode, g_lea_ctrl.next_mode);

    if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) {
        app_le_audio_bcst_stop(false);

    } else if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
        if (0 != app_le_audio_ucst_get_link_num()) {
            app_le_audio_ucst_pause_ex(APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL);
        } else {
            g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_BCST;
        }
        app_le_audio_ucst_stop_scan_all();
    }

    g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_ASIT;

    return BT_STATUS_SUCCESS;
#else
    return BT_STATUS_UNSUPPORTED;
#endif
}

uint8_t app_le_audio_get_test_mode(void)
{
    return g_lea_test_mode;
}

bt_status_t app_le_audio_start_source(const bt_addr_t addr, app_dongle_cm_start_source_param_t param)
{
    LE_AUDIO_MSGLOG_I("[APP] start_source, lea_mode:%x addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 8,
                      param.lea_mode,
                      addr.type,
                      addr.addr[5],
                      addr.addr[4],
                      addr.addr[3],
                      addr.addr[2],
                      addr.addr[1],
                      addr.addr[0]);

    if (APP_DONGLE_CM_LEA_MODE_CIS == param.lea_mode) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        return app_le_audio_start_unicast_ex(&addr);
#else
        return BT_STATUS_FAIL;
#endif
    }

    if (APP_DONGLE_CM_LEA_MODE_BIS == param.lea_mode) {
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        return app_le_audio_start_broadcast();
#else
        return BT_STATUS_FAIL;
#endif
    }

    return BT_STATUS_FAIL;
}

bt_status_t app_le_audio_stop_source(const bt_addr_t addr, app_dongle_cm_stop_source_param_t param)
{
    LE_AUDIO_MSGLOG_I("[APP] stop_source, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      addr.type,
                      addr.addr[5],
                      addr.addr[4],
                      addr.addr[3],
                      addr.addr[2],
                      addr.addr[1],
                      addr.addr[0]);


    if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_DISABLE;

        if (0 == app_le_audio_ucst_get_link_num()) {
            g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
            app_le_audio_ucst_cancel_create_connection();
            app_le_audio_ucst_pause_ex(APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL);
            g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_NONE;
            return BT_STATUS_SUCCESS;
        }

        if (app_le_audio_ucst_pause_ex(APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL)) {
            return BT_STATUS_SUCCESS;
        }

        app_le_audio_ucst_disconnect_all_device();
        return BT_STATUS_SUCCESS;
#else
        return BT_STATUS_FAIL;
#endif
    }

    if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) {
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_DISABLE;
        app_le_audio_stop_broadcast();
        return BT_STATUS_SUCCESS;
#else
        return BT_STATUS_FAIL;
#endif
    }

    return BT_STATUS_FAIL;
}

bt_status_t app_le_audio_pre_check(app_dongle_cm_precheck_data_t *check_data)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    return app_le_audio_ucst_check_adv_data((bt_gap_le_ext_advertising_report_ind_t *)check_data);
#else
    return BT_STATUS_FAIL;
#endif
}

void app_le_audio_init(void)
{
#if defined(AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE)
    uint8_t vol_level = APP_LE_AUDIO_VOL_LEVEL_DEFAULT;
#ifndef AIR_VOLUME_CONTROL_BY_DONGLE
    vol_level = APP_LE_AUDIO_VOL_LEVEL_MAX;
#endif
#endif
    app_dongle_cm_handle_t callback = {
        .start_source = app_le_audio_start_source,
        .stop_source = app_le_audio_stop_source,
        .precheck = app_le_audio_pre_check,
    };

    app_dongle_cm_register_handle(APP_DONGLE_CM_SOURCE_LEA, &callback);

    app_le_audio_read_test_mode();

    memset(&g_lea_ctrl, 0, sizeof(app_le_audio_ctrl_t));
#ifdef AIR_SILENCE_DETECTION_ENABLE
    app_le_audio_silence_detection_init();
#endif
    app_le_audio_usb_init();

    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_SYSTEM),
                                          (void *)app_le_audio_handle_event_callback);

#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
    app_le_audio_set_audio_transmitter_volume_level(APP_LE_AUDIO_STREAM_PORT_LINE_IN, vol_level, vol_level);
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
    app_le_audio_set_audio_transmitter_volume_level(APP_LE_AUDIO_STREAM_PORT_I2S_IN, vol_level, vol_level);
#endif

#ifdef AIR_LE_AUDIO_BIS_ENABLE
    app_le_audio_bcst_init();
#endif

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    bt_callback_manager_register_callback(bt_callback_type_gap_le_is_accept_connection_update_request,
                                          MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_SYSTEM,
                                          (void *)app_le_audio_ucst_is_connection_update_request_accepted);
    app_le_audio_ucst_init();
    bt_gap_le_srv_register_event_callback(&app_le_audio_handle_gap_le_srv_event_callback);

    if (g_lea_test_mode) {
        app_le_audio_ucst_set_create_cis_mode(APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL);
    }
#endif

#ifdef AIR_LE_AUDIO_BA_ENABLE
    app_le_audio_ba_init();
#endif

    app_lea_dongle_customer_init();
}
#endif  /* AIR_LE_AUDIO_ENABLE */

