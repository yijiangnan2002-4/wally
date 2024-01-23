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
#include "ble_csis_def.h"
#include "ble_ascs_def.h"
#include "app_le_audio_nvkey_struct.h"
#include "bt_le_audio_util_nvkey_struct.h"

#include "app_le_audio.h"
#include "app_le_audio_ucst.h"
#include "app_le_audio_csip_set_coordinator.h"
#include "app_le_audio_ucst_utillity.h"
#include "apps_events_event_group.h"
#include "bt_le_audio_msglog.h"
#include "app_dongle_connection_common.h"
#include "bt_device_manager_le.h"
#include "bt_device_manager.h"

#include "app_le_audio_aird.h"
#include "app_dongle_session_manager.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_LE_AUDIO_ADV_TYPE_RSI 0x2E //Resolvable Set Identifier
#define APP_LE_AUDIO_ADV_TYPE_MSD 0xFF //Manufacturer Specific Data
#define APP_LE_AUDIO_GENERAL_ANNOUNCEMENT   0x00 //Unicast Server is connectable but is not requesting a connection.
#define APP_LE_AUDIO_TARGETED_ANNOUNCEMENT   0x01 //Unicast Server is connectable and is requesting a connection.

/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/
static bt_addr_t g_lea_ucst_waiting_conn_addr = {0};
static bt_addr_t g_lea_ucst_waiting_create_connection_addr = {0};
static bool g_lea_ucst_waiting_cancel_create_connection = false;
static uint8_t g_lea_ucst_waiting_conn_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
static bool g_lea_ucst_waiting_conn_flag = false;
static bool g_lea_ucst_waiting_disconn_flag = false;

static uint8_t g_lea_ucst_active_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
app_le_audio_ucst_group_info_t g_lea_ucst_group_info[APP_LE_AUDIO_UCST_GROUP_ID_MAX];
static app_le_audio_ucst_bonded_list_t g_lea_ucst_bonded_list;
static bt_key_t g_le_audio_sirk = {0};

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
static bt_lea_sink_info_nvkey_t g_lea_ucst_sirk_info[APP_LE_AUDIO_NVKEY_SIRK_INFO_MAX_NUM];
#endif
static app_le_audio_ucst_set_white_list_t g_lea_ucst_set_white_list;
static app_dongle_le_race_event_callback_t g_lea_ucst_callback = NULL;

uint8_t g_lea_ucst_pts_set_size = 0;

/**************************************************************************************************
* Prototype
**************************************************************************************************/
extern app_le_audio_ctrl_t g_lea_ctrl;
extern app_le_audio_ucst_ctrl_t g_lea_ucst_ctrl;
extern app_le_audio_ucst_link_info_t g_lea_ucst_link_info[APP_LE_AUDIO_UCST_LINK_MAX_NUM];
extern app_le_audio_ucst_cis_info_t g_lea_ucst_cis_info[APP_LE_AUDIO_UCST_CIS_MAX_NUM];
extern app_dongle_session_manager_lea_handle_t *g_lea_ucst_session_manager_cb;

extern bt_status_t ble_csip_discover_coordinated_set(bool enable);
extern bt_status_t ble_csip_set_by_sirk(bt_key_t sirk);
extern bt_status_t bt_gap_le_srv_set_extended_scan(bt_gap_le_srv_set_extended_scan_parameters_t *param, bt_gap_le_srv_set_extended_scan_enable_t *enable, void *callback);
extern void bt_app_common_at_cmd_print_report(char *string);
extern bt_status_t ble_tbs_switch_device_completed(void);
extern bool app_lea_dongle_customer_check_adv_data(bt_gap_le_ext_advertising_report_ind_t *ind);

static bt_status_t app_le_audio_ucst_update_connection_parameter(bt_handle_t handle, uint16_t interval, uint16_t ce_len);
static void app_le_audio_ucst_increase_connection_config_speed_timer_callback(TimerHandle_t timer_handle, void *user_data);
static bt_status_t app_le_audio_ucst_connect_bonded_device(bool search_cs);
static void app_le_audio_ucst_write_bonded_list_to_nvkey(void);
static bool app_le_audio_ucst_check_delete_group_device(void);
static void app_le_audio_ucst_remove_link_from_group(uint8_t group_id, uint8_t link_idx);
static bool app_le_audio_ucst_is_bonded_device(const bt_addr_t *addr, bool check_white_list);
static void app_le_audio_ucst_delete_oldest_group_from_bonded_list(void);
static void app_le_audio_ucst_add_link_to_group(uint8_t group_id, uint8_t link_idx);
static void app_le_audio_ucst_reset_all_group_info(void);
static uint8_t app_le_audio_ucst_get_group_link_num(uint8_t group_id);
static void app_le_audio_ucst_reset_group_info(uint8_t group_id);
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
static bool app_le_audio_ucst_is_group_full(void);
//static bool app_le_audio_ucst_is_bonded_device_all_connected(void);
static void app_le_audio_ucst_disconnect_other_group_device(uint8_t keep_group_id, uint8_t target_device_location);
static void app_le_audio_ucst_connect_group_device(uint8_t group_id);

#endif
static bool app_le_audio_ucst_check_connect_coordinated_set_ex(app_le_audio_ucst_link_info_t *p_info);
static void app_le_audio_ucst_check_connect_coordinated_set(app_le_audio_ucst_link_info_t *p_info);
static bool app_le_audio_ucst_check_bond_and_not_in_white_list(const bt_addr_t *addr);
static bool app_le_audio_ucst_disconnect_device_by_idx(uint8_t link_idx);
static bt_hci_disconnect_reason_t app_le_audio_ucst_get_disconnect_reason(const bt_addr_t *addr);

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:app_lea_dongle_customer_check_adv_data=default_app_lea_dongle_customer_check_adv_data")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak app_lea_dongle_customer_check_adv_data = default_app_lea_dongle_customer_check_adv_data
#else
#error "Unsupported Platform"
#endif

bool default_app_lea_dongle_customer_check_adv_data(bt_gap_le_ext_advertising_report_ind_t *ind)
{
    return true;
}

/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static void app_le_audio_ucst_print_lea_adv_addr(bt_addr_t *addr)
{
#if 0
    char conn_string[60] = {0};

    snprintf((char *)conn_string, 60, "LEA Device found! addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
             addr->type,
             addr->addr[5],
             addr->addr[4],
             addr->addr[3],
             addr->addr[2],
             addr->addr[1],
             addr->addr[0]);
    bt_app_common_at_cmd_print_report(conn_string);
#endif
#if 1
    LE_AUDIO_MSGLOG_I("[APP] LEA ADV found! addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      addr->type,
                      addr->addr[5],
                      addr->addr[4],
                      addr->addr[3],
                      addr->addr[2],
                      addr->addr[1],
                      addr->addr[0]);
#endif
}

static bool app_le_audio_ucst_is_lea_adv(bt_gap_le_ext_advertising_report_ind_t *adv_report)
{
    bt_hci_disconnect_reason_t disconnect_reason;
    uint16_t i = 0;
    uint8_t announcement_type;
    bool ret = false;

    while (i < adv_report->data_length) {
        if ((adv_report->data[i] >= 3) &&
            (adv_report->data[i + 1] == BT_GAP_LE_AD_TYPE_SERVICE_DATA) &&
            (adv_report->data[i + 2] == (BT_GATT_UUID16_ASCS_SERVICE & 0xFF)) &&
            (adv_report->data[i + 3] == ((BT_GATT_UUID16_ASCS_SERVICE & 0xFF00) >> 8))) {
            announcement_type = adv_report->data[i + 4];
            disconnect_reason = app_le_audio_ucst_get_disconnect_reason(&adv_report->address);
            if ((APP_LE_AUDIO_GENERAL_ANNOUNCEMENT == announcement_type)&&(true == app_le_audio_ucst_is_bonded_device(&adv_report->address, false))) {
                if (0 == disconnect_reason) {//dongle is plugged in or AT CMD to select device
                    ret = true;
                }
            }
            else {
                ret = true;
            }

            if (BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES == disconnect_reason) {
                ret = false;
            }

            LE_AUDIO_MSGLOG_I("[APP] is_lea_adv, YES! announcement:%d ret:%d", 2, announcement_type, ret);
            break;
        }

        i += (adv_report->data[i] + 1);
    }
    //LE_AUDIO_MSGLOG_I("[APP][U] is_lea_adv, NO!", 0);
    return ret;
}

static bt_status_t app_le_audio_ucst_add_white_list(bt_addr_t *addr)
{
    bt_addr_t device;
    bt_status_t ret;
    bt_device_manager_le_bonded_info_t * p_bonded_info = NULL;
    bt_bd_addr_t empty_addr = {0};
    p_bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)&addr->addr);

    memcpy(&device, addr, sizeof(bt_addr_t));
    //ret = bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &device);
    ret = bt_gap_le_srv_operate_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &device, NULL);
    LE_AUDIO_MSGLOG_I("[APP][U] add_white_list, ret:%x", 1, ret);

    if ((NULL != p_bonded_info) && (0 != memcmp(p_bonded_info->info.identity_addr.address.addr, empty_addr, sizeof(bt_bd_addr_t)))) {
        //ret = bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &p_bonded_info->info.identity_addr.address);
        ret = bt_gap_le_srv_operate_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &p_bonded_info->info.identity_addr.address, NULL);
        LE_AUDIO_MSGLOG_I("[APP][U] add_white_list(IDA), ret:%x addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 8, ret,
              p_bonded_info->info.identity_addr.address.type,
              p_bonded_info->info.identity_addr.address.addr[5],
              p_bonded_info->info.identity_addr.address.addr[4],
              p_bonded_info->info.identity_addr.address.addr[3],
              p_bonded_info->info.identity_addr.address.addr[2],
              p_bonded_info->info.identity_addr.address.addr[1],
              p_bonded_info->info.identity_addr.address.addr[0]);
    }
    return ret;
}

static bt_status_t app_le_audio_ucst_remove_white_list(bt_addr_t *addr)
{
    bt_addr_t device;
    bt_status_t ret;
    bt_device_manager_le_bonded_info_t * p_bonded_info = NULL;
    bt_bd_addr_t empty_addr = {0};
    p_bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)&addr->addr);

    memcpy(&device, addr, sizeof(bt_addr_t));
    //ret = bt_gap_le_set_white_list(BT_GAP_LE_REMOVE_FROM_WHITE_LIST, &device);
    ret = bt_gap_le_srv_operate_white_list(BT_GAP_LE_REMOVE_FROM_WHITE_LIST, &device, NULL);

    LE_AUDIO_MSGLOG_I("[APP][U] rm_white_list, ret:%x", 1, ret);

    if ((NULL != p_bonded_info) && (0 != memcmp(p_bonded_info->info.identity_addr.address.addr, empty_addr, sizeof(bt_bd_addr_t)))) {
        //ret = bt_gap_le_set_white_list(BT_GAP_LE_REMOVE_FROM_WHITE_LIST, &p_bonded_info->info.identity_addr.address);
        ret = bt_gap_le_srv_operate_white_list(BT_GAP_LE_REMOVE_FROM_WHITE_LIST, &p_bonded_info->info.identity_addr.address, NULL);
    }
    LE_AUDIO_MSGLOG_I("[APP][U] add_white_list, ret:%x", 1, ret);

    return ret;
}

static uint8_t app_le_audio_ucst_set_new_group_info(bt_key_t *sirk, uint8_t link_idx)
{
    uint8_t group_id = 0;

    if (g_lea_ucst_pts_set_size) {
        for (group_id = 0; group_id < APP_LE_AUDIO_UCST_GROUP_ID_MAX; group_id++) {
            if (g_lea_ucst_pts_set_size == g_lea_ucst_group_info[group_id].size) {
                break;
            }
        }
    }

    if ((!g_lea_ucst_pts_set_size) || (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= group_id)) {
        for (group_id = 0; group_id < APP_LE_AUDIO_UCST_GROUP_ID_MAX; group_id++) {
            if (0 == g_lea_ucst_group_info[group_id].size) {
                if (NULL != sirk) {
                    memcpy((uint8_t *)&g_lea_ucst_group_info[group_id].sirk, sirk, sizeof(bt_key_t));
                }

                g_lea_ucst_group_info[group_id].size = 1;    /* update later or no CSIS, 1 device */

                if (g_lea_ucst_pts_set_size) {
                    /* some PTS cases which need 2 or more PTS dongles may have no CSIS */
                    g_lea_ucst_group_info[group_id].size = g_lea_ucst_pts_set_size;
                }

                g_lea_ucst_group_info[group_id].bond_num = 1;
                g_lea_ucst_group_info[group_id].link_idx[0] = link_idx;
                break;
            }
        }
    }

    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX > group_id) {
        LE_AUDIO_MSGLOG_I("[APP][U] set_group_info, group:%x(size:%x) link_idx:(%x %x) bond_num:%x", 5,
                          group_id,
                          g_lea_ucst_group_info[group_id].size,
                          g_lea_ucst_group_info[group_id].link_idx[0],
                          g_lea_ucst_group_info[group_id].link_idx[1],
                          g_lea_ucst_group_info[group_id].bond_num);
    }

    return group_id;
}


/* stop scan LE AUDIO devices */
static bt_status_t app_le_audio_ucst_stop_scan(void)
{
    bt_status_t ret = BT_STATUS_FAIL;

    LE_AUDIO_MSGLOG_I("[APP][U] stop_scan, scan:%x->%x", 2,  g_lea_ucst_ctrl.curr_scan, g_lea_ucst_ctrl.next_scan);

    switch (g_lea_ucst_ctrl.curr_scan) {
        case APP_LE_AUDIO_UCST_SCAN_NONE:
        case APP_LE_AUDIO_UCST_SCAN_DISABLING:
        case APP_LE_AUDIO_UCST_SCAN_CS_DISABLING: {
            g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
            ret =  BT_STATUS_SUCCESS;
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_CS_ENABLING:
        case APP_LE_AUDIO_UCST_SCAN_CS_ENABLED: {
            ret = BT_STATUS_FAIL;
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_ENABLING: {
            g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_DISABLING;
            ret =  BT_STATUS_SUCCESS;
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_ENABLED: {
            bt_hci_cmd_le_set_extended_scan_enable_t enable = {
                .enable = BT_HCI_DISABLE,
                .filter_duplicates = BT_HCI_DISABLE,
                .duration = 0,
                .period = 0
            };
            // LE_SET_SCAN_CNF may callback before bt_gap_le_srv_set_extended_scan() return BT_STATUS_SUCCESS,
            // so need set status before call bt_gap_le_srv_set_extended_scan
            g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_DISABLING;
            ret = bt_gap_le_srv_set_extended_scan(NULL, &enable, NULL);
            if (BT_STATUS_SUCCESS != ret) {
                g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_ENABLED;
                LE_AUDIO_MSGLOG_I("[APP][U] stop_scan failed, ret:%x", 1,  ret);
            }
            break;
        }
        default:
            break;
    }

    return ret;
}

/* stop scan coordinated set devices */
static bt_status_t app_le_audio_ucst_stop_scan_cs(void)
{
    bt_status_t ret = BT_STATUS_FAIL;

    LE_AUDIO_MSGLOG_I("[APP][U] stop_scan_cs, scan:%x->%x", 2,  g_lea_ucst_ctrl.curr_scan, g_lea_ucst_ctrl.next_scan);

    switch (g_lea_ucst_ctrl.curr_scan) {
        case APP_LE_AUDIO_UCST_SCAN_NONE:
        case APP_LE_AUDIO_UCST_SCAN_DISABLING:
        case APP_LE_AUDIO_UCST_SCAN_CS_DISABLING: {
            g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
            ret = BT_STATUS_SUCCESS;
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_ENABLING:
        case APP_LE_AUDIO_UCST_SCAN_ENABLED: {
            ret = BT_STATUS_FAIL;
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_CS_ENABLING: {
            g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_CS_DISABLING;
            ret = BT_STATUS_SUCCESS;
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_CS_ENABLED: {
            ret = ble_csip_discover_coordinated_set(false);
            g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_CS_DISABLING;

            if (BT_STATUS_SUCCESS != ret) {
                g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_CS_ENABLED;
                LE_AUDIO_MSGLOG_I("[APP][U] stop_scan_cs failed, ret:%x", 1,  ret);
            } else {
                bt_app_common_at_cmd_print_report("Stop scan CS:\r\n");
            }
            break;
        }
        default:
            break;
    }

    return ret;
}

/* start scan LE AUDIO devices */
static bt_status_t app_le_audio_ucst_start_scan(bool use_white_list)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    LE_AUDIO_MSGLOG_I("[APP][U] start_scan, scan:%x->%x use_white_list:%x", 3,  g_lea_ucst_ctrl.curr_scan, g_lea_ucst_ctrl.next_scan, use_white_list);

    switch (g_lea_ucst_ctrl.curr_scan) {
        case APP_LE_AUDIO_UCST_SCAN_ENABLING:
        case APP_LE_AUDIO_UCST_SCAN_DISABLING:
        case APP_LE_AUDIO_UCST_SCAN_CS_ENABLING:
        case APP_LE_AUDIO_UCST_SCAN_CS_DISABLING: {
            g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_ENABLING;
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_ENABLED: {
            g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_ENABLING;
            if (BT_STATUS_SUCCESS != app_le_audio_ucst_stop_scan()) {
                g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_ENABLED;
            }
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_CS_ENABLED: {
            g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_ENABLING;
            if (BT_STATUS_SUCCESS != app_le_audio_ucst_stop_scan_cs()) {
                g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_CS_ENABLED;
            }
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_NONE: {
            le_ext_scan_item_t *ext_scan_1M_item;
            if (NULL == (ext_scan_1M_item = (le_ext_scan_item_t *)pvPortMalloc(sizeof(le_ext_scan_item_t)))) {
                return BT_STATUS_FAIL;
            }

            memset(ext_scan_1M_item, 0, sizeof(le_ext_scan_item_t));

            ext_scan_1M_item->scan_type = BT_HCI_SCAN_TYPE_PASSIVE;

            if (use_white_list) {
                ext_scan_1M_item->scan_interval = 0x90;
                ext_scan_1M_item->scan_window = 0x30;
            } else {
                ext_scan_1M_item->scan_interval = 0x90;
                ext_scan_1M_item->scan_window = 0x24;
            }

            bt_hci_le_set_ext_scan_parameters_t params = {
                .own_address_type = BT_HCI_SCAN_ADDR_RANDOM,
                .scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS,
                .scanning_phys_mask = BT_HCI_LE_PHY_MASK_1M,
                .params_phy_1M = ext_scan_1M_item,
                .params_phy_coded = NULL,
            };

            bt_hci_cmd_le_set_extended_scan_enable_t enable = {
                .enable = BT_HCI_ENABLE,
                .filter_duplicates = BT_HCI_ENABLE,
                .duration = 0,
                .period = 0
            };

            if (use_white_list) {
                params.scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ONLY_ADVERTISING_PACKETS_IN_WHITE_LIST;
            }
            g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_ENABLING;

            ret = bt_gap_le_srv_set_extended_scan(&params, &enable, NULL);
            if (BT_STATUS_SUCCESS != ret) {
                g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
                LE_AUDIO_MSGLOG_I("[APP][U] start_scan failed, ret:%x", 1,  ret);
            }
            vPortFree(ext_scan_1M_item);
         }
        default:
            break;
    }

    return ret;
}

/* start scan coordinated set devices */
static bt_status_t app_le_audio_ucst_start_scan_cs(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    LE_AUDIO_MSGLOG_I("[APP][U] start_scan_CS, scan:%x->%x", 2,  g_lea_ucst_ctrl.curr_scan, g_lea_ucst_ctrl.next_scan);

    switch (g_lea_ucst_ctrl.curr_scan) {
        case APP_LE_AUDIO_UCST_SCAN_ENABLING:
        case APP_LE_AUDIO_UCST_SCAN_DISABLING:
        case APP_LE_AUDIO_UCST_SCAN_CS_ENABLING:
        case APP_LE_AUDIO_UCST_SCAN_CS_DISABLING: {
            g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_CS_ENABLING;
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_ENABLED: {
            /* stop_scan--> start_scan_cs */
            g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_CS_ENABLING;
            if (BT_STATUS_SUCCESS != app_le_audio_ucst_stop_scan()) {
                g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_ENABLED;
            }
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_CS_ENABLED: {
            /* stop_scan_cs--> start_scan_cs */
            g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_CS_ENABLING;
            if (BT_STATUS_SUCCESS != app_le_audio_ucst_stop_scan_cs()) {
                g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_CS_ENABLED;
            }
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_NONE: {
            g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_CS_ENABLING;
            ret = ble_csip_discover_coordinated_set_by_sirk(true, g_le_audio_sirk);

            if (BT_STATUS_SUCCESS != ret) {
                g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
                LE_AUDIO_MSGLOG_I("[APP][U] start_scan_CS failed, ret:%x", 1,  ret);
            } else {
                bt_app_common_at_cmd_print_report("Scan CS:\r\n");
            }
            break;
        }
        default:
            break;
    }

    return ret;
}

static bool app_le_audio_ucst_scan_and_reconnect_device(uint8_t bond_idx)
{
    if (0 == g_lea_ucst_bonded_list.num) {
        app_le_audio_ucst_connect_coordinated_set(true);

    } else if ((APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID != bond_idx) && (g_lea_ucst_bonded_list.device[bond_idx].in_white_list)) {
        if (APP_LE_AUDIO_UCST_CONN_COORDINATED_SET_BY_SIRK == g_lea_ucst_ctrl.curr_conn) {
            app_le_audio_ucst_connect_bonded_device(true);
        } else {
            app_le_audio_ucst_connect_bonded_device(false);
        }
    } else {
        return false;
    }
    return true;
}

/* scan and connect bonded devices (also cs device) */
static bt_status_t app_le_audio_ucst_connect_bonded_device(bool search_cs)
{
    bt_status_t ret;

    if (0 == g_lea_ucst_bonded_list.num) {
        LE_AUDIO_MSGLOG_I("[APP] connect_bonded_device, bonded_list empty", 0);
        return BT_STATUS_CONNECTION_NOT_FOUND;
    }

    if (APP_LE_AUDIO_UCST_SET_WHITE_LIST_ADD_ON_GOING == g_lea_ucst_set_white_list.state) {
        LE_AUDIO_MSGLOG_I("[APP] connect_bonded_device, wait for set white list", 0);
        return BT_STATUS_BUSY;
    }

    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == app_le_audio_ucst_get_link_num()) {
        LE_AUDIO_MSGLOG_I("[APP][U] connect_bonded_device, link full", 0);
#ifndef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        return BT_STATUS_FAIL;
#endif
    }

    LE_AUDIO_MSGLOG_I("[APP][U] connect_bonded_device, scan:%x->%x conn:%x", 3,
                      g_lea_ucst_ctrl.curr_scan,
                      g_lea_ucst_ctrl.next_scan,
                      g_lea_ucst_ctrl.curr_conn);

    bt_app_common_at_cmd_print_report("Scan bonded device:\r\n");

    if (search_cs) {
        g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_CS_AND_BONDED_DEVICE;    /* scan bonded device and CS */
    } else {
        g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_BONDED_DEVICE;           /* scan bonded device only */
    }

    if (APP_LE_AUDIO_UCST_CONN_BONDED_DEVICE == g_lea_ucst_ctrl.curr_conn) {
        if (BT_STATUS_SUCCESS != (ret = app_le_audio_ucst_start_scan(true))) {      /* use white list */
            g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
        }
    } else {
        if (BT_STATUS_SUCCESS != (ret = app_le_audio_ucst_start_scan(false))) {     /* normal scan */
            g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][U] connect_bonded_device, curr_scan:%x conn:%x", 2, g_lea_ucst_ctrl.curr_scan, g_lea_ucst_ctrl.curr_conn);

    return ret;
}

static bool app_le_audio_ucst_check_connect_coordinated_set_ex(app_le_audio_ucst_link_info_t *p_info)
{
    app_le_audio_ucst_bonded_device_t *p_bond_device = NULL;
    uint8_t link_num, group_link_num, active_group;
    uint8_t i;
    bool scan_cs = false, scan_bond = false, is_active_device = false;
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    bool scan_active_group = false;
    bool scan_others = false;
#endif

    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= p_info->group_id) {
        return 0;
    }

    group_link_num = app_le_audio_ucst_get_group_link_num(p_info->group_id);

    if (p_info->group_id == (active_group = app_le_audio_ucst_get_active_group())) {
        is_active_device = true;
    }

    link_num = app_le_audio_ucst_get_link_num_ex();

    LE_AUDIO_MSGLOG_I("[APP][U] check_connect_cs, handle:%x group:%x(size:%x) bond_idx:%x group_link_num:%x bond_num:%x link_num:%x", 7,
                      p_info->handle,
                      p_info->group_id,
                      p_info->group_size,
                      p_info->bond_idx,
                      group_link_num,
                      g_lea_ucst_group_info[p_info->group_id].bond_num,
                      link_num);

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    /* [multi-device] check all bond list */
    for (i = 0; i < g_lea_ucst_bonded_list.num; i++) {
        if (i != p_info->bond_idx) {
            p_bond_device = &g_lea_ucst_bonded_list.device[i];
#if 1       /* for test */
            LE_AUDIO_MSGLOG_I("[APP] check_connect_cs, bond_idx:%x group:%x link_idx:%x in_white_list:%x", 4,
                              i, p_bond_device->group_id,
                              p_bond_device->link_idx, p_bond_device->in_white_list);
#endif
            if ((APP_LE_AUDIO_UCST_LINK_IDX_INVALID == p_bond_device->link_idx) &&
                (p_bond_device->in_white_list) &&
                (APP_LE_AUDIO_UCST_GROUP_ID_MAX > p_bond_device->group_id)) {
                if (p_info->group_id == p_bond_device->group_id) {
                    scan_bond = true;
                } else if (active_group == p_bond_device->group_id) {
                    scan_active_group = true;
                } else {
                    scan_others = true;
                }
            }
        }
    }
#else
    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == link_num) {
        return false;
    }
#endif

    /* check cs all connected */
    if (p_info->group_size != group_link_num) {
        /* not all connected, then start scan device */
        if (p_info->group_size != g_lea_ucst_group_info[p_info->group_id].bond_num) {
            /* non bond --> scan sc */
            scan_cs = true;
#ifndef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        } else {
            /* search all bond list */
            for (i = 0; i < g_lea_ucst_bonded_list.num; i++) {
                if (i != p_info->bond_idx) {
                    p_bond_device = &g_lea_ucst_bonded_list.device[i];
#if 1               /* for test */
                    LE_AUDIO_MSGLOG_I("[APP] check_connect_cs, bond_idx:%x group:%x link_idx:%x in_white_list:%x", 4,
                                      i, p_bond_device->group_id,
                                      p_bond_device->link_idx, p_bond_device->in_white_list);
#endif
                    if ((p_info->group_id == p_bond_device->group_id) &&
                        (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == p_bond_device->link_idx) &&
                        (p_bond_device->in_white_list)) {
                        scan_bond = true;
                        break;
                    }
                }
            }
#endif
        }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == link_num) {
            if ((!is_active_device) && (APP_LE_AUDIO_UCST_GROUP_ID_INVALID != active_group)) {
                scan_cs = false;
                scan_bond = false;
            }

            if ((true == scan_cs) || (true == scan_bond)) {
                g_lea_ucst_waiting_conn_group = p_info->group_id;
                LE_AUDIO_MSGLOG_I("[APP] check_connect_cs, link full scan, own group(%x) mate", 1, g_lea_ucst_waiting_conn_group);
            }
        }
#else
    } else {
        return false;
#endif
    }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == link_num) {
        scan_others = false;
        if (scan_active_group) {
            g_lea_ucst_waiting_conn_group = active_group;
            LE_AUDIO_MSGLOG_I("[APP] check_connect_cs, link full scan, other group(%x) mate", 1, g_lea_ucst_waiting_conn_group);
        }
    }
    if (scan_others || scan_active_group) {
        scan_bond = true;
    }

    LE_AUDIO_MSGLOG_I("[APP] check_connect_cs 1, scan_others:%x scan_active_group:%x", 2, scan_others, scan_active_group);
    scan_others = false;
    scan_active_group = false;

    /* [multi-device] check all cs connected */
    for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_ID_MAX; i++) {
        if ((i != p_info->group_id) && (0 != g_lea_ucst_group_info[i].size)) {
            group_link_num = app_le_audio_ucst_get_group_link_num(i);
            LE_AUDIO_MSGLOG_I("[APP][U] check_connect_cs, group:%x(size:%x) group_link_num:%x bond_num:%x", 4,
                              i,
                              g_lea_ucst_group_info[i].size,
                              group_link_num,
                              g_lea_ucst_group_info[i].bond_num);
            if ((g_lea_ucst_group_info[i].size != group_link_num) &&
                (g_lea_ucst_group_info[i].size != g_lea_ucst_group_info[i].bond_num)) {
                /* non bond --> scan sc */
                if (i == active_group) {
                    scan_active_group = true;
                    break;
                } else {
                    scan_others = true;
                }
            }
        }
    }
    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == link_num) {
        scan_others = false;
        if (scan_active_group) {
            g_lea_ucst_waiting_conn_group = active_group;
            LE_AUDIO_MSGLOG_I("[APP] check_connect_cs, link full scan, other group(%x) mate", 1, g_lea_ucst_waiting_conn_group);
        }
    }
    if (scan_others || scan_active_group) {
        scan_cs = true;
    }
    LE_AUDIO_MSGLOG_I("[APP] check_connect_cs 2, scan_others:%x scan_active_group:%x", 2, scan_others, scan_active_group);
#endif

    LE_AUDIO_MSGLOG_I("[APP] check_connect_cs, active_group:%x is_active_device:%x scan_cs:%x scan_bond:%x", 4,
                      active_group,
                      is_active_device,
                      scan_cs,
                      scan_bond);

    if ((APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode) ||
        ((false == scan_bond) && (false == scan_cs))) {
        return false;
    }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (scan_cs && scan_bond) {
        app_le_audio_ucst_connect_bonded_device(true);
    } else if (scan_cs) {
#else
    if (scan_cs) {
#endif
        app_le_audio_ucst_connect_coordinated_set(false);
    } else if (scan_bond) {
        app_le_audio_ucst_connect_bonded_device(false);
    }

    return true;
}

static void app_le_audio_ucst_check_connect_coordinated_set(app_le_audio_ucst_link_info_t *p_info)
{
    if (app_le_audio_ucst_check_connect_coordinated_set_ex(p_info)) {
        return;
    }

    g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                        APP_LE_AUDIO_EVENT_STOP_SCAN_NEW_DEVICE, NULL, 0,
                        NULL, 0);
}


static bt_status_t app_le_audio_ucst_update_connection_parameter(bt_handle_t handle, uint16_t interval, uint16_t ce_len)
{
    bt_hci_cmd_le_connection_update_t param;
    bt_status_t ret;

    param.connection_handle = handle;
    param.conn_interval_min = interval;
    param.conn_interval_max = interval;
    param.conn_latency = 0x0000;
    param.supervision_timeout = 0x01F4;
    param.minimum_ce_length = ce_len;
    param.maximum_ce_length = ce_len;

    ret = bt_gap_le_update_connection_parameter((const bt_hci_cmd_le_connection_update_t *)(&param));

    LE_AUDIO_MSGLOG_I("[APP][U] update_connection_param, handle:%x ret:%x", 2, handle, ret);
    return ret;
}

static void app_le_audio_ucst_increase_connection_config_speed_timer_callback(TimerHandle_t timer_handle, void *user_data)
{
    app_le_audio_ucst_link_info_t *link_info = (app_le_audio_ucst_link_info_t *)user_data;

    LE_AUDIO_MSGLOG_I("[APP][U] connection interval for config timer expired. ", 0);
    if (link_info) {
        /* Timer will be deleted after the callback returns. */
        link_info->conn_interval_timer_handle = NULL;
        app_le_audio_ucst_update_connection_interval(link_info, APP_LE_AUDIO_CONN_INTERVAL_STREAMING);
    }
}

static bool app_le_audio_ucst_disconnect_device_by_idx(uint8_t link_idx)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= link_idx) {
        return BT_STATUS_FAIL;
    }

    p_info = &g_lea_ucst_link_info[link_idx];

    if (BT_HANDLE_INVALID == p_info->handle) {
        return BT_STATUS_FAIL;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] disconnect_device, handle:%x link_idx:%x", 2, p_info->handle, link_idx);

    if (APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL == p_info->next_state) {
        return BT_STATUS_SUCCESS;
    }

    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL;

    if (APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID != p_info->bond_idx) {
        g_lea_ucst_bonded_list.device[p_info->bond_idx].in_white_list = false;
    }

    if (BT_STATUS_SUCCESS == app_le_audio_ucst_disconnect(p_info->handle)) {
        app_le_audio_ucst_remove_white_list(&(p_info->addr));
        return BT_STATUS_SUCCESS;
    }

    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
    app_le_audio_ucst_remove_white_list(&(p_info->addr));

    return BT_STATUS_FAIL;
}

static void app_le_audio_ucst_reset_link_info(uint8_t link_idx)
{
    app_le_audio_ucst_link_info_t *link_info = NULL;
    uint8_t i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;

    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= link_idx) {
        LE_AUDIO_MSGLOG_W("[APP] reset_link_info. out of range. link_idx:%d", 1, link_idx);
        return;
    }

    link_info = &g_lea_ucst_link_info[link_idx];

    while (i > 0) {
        i--;
        if (link_info->handle == g_lea_ucst_cis_info[i].acl_handle) {
            g_lea_ucst_cis_info[i].acl_handle = BT_HANDLE_INVALID;
            g_lea_ucst_cis_info[i].cis_status = APP_LE_AUDIO_UCST_CIS_IDLE;
            /* keep cis handle for device reconnect */
        }
    }

    if (link_info->le_connection_timer_handle) {
        LE_AUDIO_MSGLOG_I("[APP] reset_link_info. timer_handle:%x", 1, link_info->le_connection_timer_handle);
        app_le_audio_timer_stop(link_info->le_connection_timer_handle);
    }

    if (link_info->conn_interval_timer_handle) {
        LE_AUDIO_MSGLOG_I("[APP] reset_link_info. timer_handle:%x", 1, link_info->conn_interval_timer_handle);
        app_le_audio_timer_stop(link_info->conn_interval_timer_handle);
    }

    if (link_info->sink_pac.pac_record) {
        vPortFree(link_info->sink_pac.pac_record);
    }

    if (link_info->source_pac.pac_record) {
        vPortFree(link_info->source_pac.pac_record);
    }

    memset(link_info, 0, sizeof(app_le_audio_ucst_link_info_t));
    link_info->handle = BT_HANDLE_INVALID;
    link_info->group_id = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    link_info->bond_idx = APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID;
}

static app_le_audio_ucst_link_info_t *app_le_audio_ucst_get_available_link_info_for_new_le_connection(uint8_t *link_idx)
{
    if (link_idx) {
        *link_idx = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
    }

    for (uint32_t i = 0; APP_LE_AUDIO_UCST_LINK_MAX_NUM > i ; i++) {
        if (BT_HANDLE_INVALID == g_lea_ucst_link_info[i].handle &&
            NULL == g_lea_ucst_link_info[i].le_connection_timer_handle) {
            if (link_idx) {
                *link_idx = i;
            }
            return &g_lea_ucst_link_info[i];
        }
    }

    return NULL;
}

static app_le_audio_ucst_link_info_t *app_le_audio_ucst_find_connecting_link_info_by_timer_handle(TimerHandle_t timer_handle, uint8_t *link_idx)
{
    if (link_idx) {
        *link_idx = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
    }

    if (timer_handle) {
        for (uint32_t i = 0; APP_LE_AUDIO_UCST_LINK_MAX_NUM > i ; i++) {
            if (BT_HANDLE_INVALID == g_lea_ucst_link_info[i].handle &&
                timer_handle == g_lea_ucst_link_info[i].le_connection_timer_handle) {
                if (link_idx) {
                    *link_idx = i;
                }
                return &g_lea_ucst_link_info[i];
            }
        }
    }

    return NULL;
}


static app_le_audio_ucst_link_info_t *app_le_audio_ucst_find_connecting_link_info_by_peer_addr(const bt_addr_t *addr, uint8_t *link_idx)
{
    if (link_idx) {
        *link_idx = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
    }

    if (addr) {
        for (uint32_t i = 0; APP_LE_AUDIO_UCST_LINK_MAX_NUM > i ; i++) {
            if (BT_HANDLE_INVALID == g_lea_ucst_link_info[i].handle &&
                g_lea_ucst_link_info[i].le_connection_timer_handle &&
                (0 == memcmp(&(g_lea_ucst_link_info[i].addr), addr, sizeof(bt_addr_t)))) {
                if (link_idx) {
                    *link_idx = i;
                }
                return &g_lea_ucst_link_info[i];
            }
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][U] le_connection_timer not found link_idx:%x", 1, link_idx);
    return NULL;
}

static void app_le_audio_ucst_le_connection_timer_callback(TimerHandle_t timer_handle, void *user_data)
{
    uint8_t link_idx;
    app_le_audio_ucst_link_info_t *link_info = app_le_audio_ucst_find_connecting_link_info_by_timer_handle(timer_handle, &link_idx);

    LE_AUDIO_MSGLOG_I("[APP][U] LE connection timer expired. timer_handle:%x link_idx:%d user_data:%x conn:%x scan:%x->%x", 6,
                      timer_handle,
                      link_idx,
                      user_data,
                      g_lea_ucst_ctrl.curr_conn,
                      g_lea_ucst_ctrl.curr_scan,
                      g_lea_ucst_ctrl.next_scan);
    if (link_info) {
        /* LE connection is not finished. Otherwise, link_info will be NULL. */
        /* Timer will be deleted after the callback returns. */
        link_info->le_connection_timer_handle = NULL;
        app_le_audio_ucst_reset_link_info(link_idx);
        bt_gap_le_srv_cancel_connection();
        //if (APP_LE_AUDIO_UCST_SCAN_NONE == g_lea_ucst_ctrl.curr_scan) {
        //    app_le_audio_ucst_connect_coordinated_set(false);
        //}
    }
}
/*
static void app_le_audio_ucst_reset_all_link_info(void)
{
    for (int32_t i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
        app_le_audio_ucst_reset_link_info(i);
    }
}
*/
static uint8_t app_le_audio_ucst_get_bonded_list_idx(bt_addr_t *addr)
{
    uint8_t i;
    bt_device_manager_le_bonded_info_t * p_bonded_info = NULL;

    if (0 == g_lea_ucst_bonded_list.num) {
        return APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID;
    }
    p_bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)&addr->addr);

#if 0
    LE_AUDIO_MSGLOG_I("[APP] is_bonded_device, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      addr->type,
                      addr->addr[5],
                      addr->addr[4],
                      addr->addr[3],
                      addr->addr[2],
                      addr->addr[1],
#endif

    for (i = 0; i < g_lea_ucst_bonded_list.num; i++) {
    if ((0 != g_lea_ucst_bonded_list.device[i].group_size) &&
            ((0 == memcmp(&(g_lea_ucst_bonded_list.device[i].addr), addr, sizeof(bt_addr_t))) ||
            ((NULL != p_bonded_info) && (0 == memcmp(&(g_lea_ucst_bonded_list.device[i].addr), &p_bonded_info->bt_addr, sizeof(bt_addr_t)))))) {
            LE_AUDIO_MSGLOG_I("[APP][U] is_bonded_device, YES! i:%x group:%x(size:%x)", 3,
                              i,
                              g_lea_ucst_bonded_list.device[i].group_id,
                              g_lea_ucst_bonded_list.device[i].group_size);
            return i;
        }
    }

    return APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID;
}

static uint8_t app_le_audio_ucst_get_empty_bonded_list_idx(void)
{
    uint8_t i;

    if (APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM == g_lea_ucst_bonded_list.num) {
        LE_AUDIO_MSGLOG_I("[APP][U] bonded_list, num:%x", 1, g_lea_ucst_bonded_list.num);
        return APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID;
    }

#if 0
    LE_AUDIO_MSGLOG_I("[APP] is_bonded_device, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      addr->type,
                      addr->addr[5],
                      addr->addr[4],
                      addr->addr[3],
                      addr->addr[2],
                      addr->addr[1],
#endif

    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
        if ((APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_bonded_list.device[i].group_id) &&
            (0 == g_lea_ucst_bonded_list.device[i].group_size)) {
            LE_AUDIO_MSGLOG_I("[APP][U] get_empty_bonded_list, i:%x group:%x(size:%x)", 3, i, g_lea_ucst_bonded_list.device[i].group_id, g_lea_ucst_bonded_list.device[i].group_size);
            return i;
        }
    }

    return APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID;
}


static void app_le_audio_ucst_reset_bonded_list(uint8_t bond_idx)
{
    if (APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM <= bond_idx) {
        return;
    }

    memset(&(g_lea_ucst_bonded_list.device[bond_idx]), 0, sizeof(app_le_audio_ucst_bonded_device_t));
    g_lea_ucst_bonded_list.device[bond_idx].group_id = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    g_lea_ucst_bonded_list.device[bond_idx].link_idx = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
}

static void app_le_audio_ucst_reset_all_bonded_list(void)
{
    uint8_t i;

    memset(&g_lea_ucst_bonded_list, 0, sizeof(app_le_audio_ucst_bonded_list_t));

    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
        g_lea_ucst_bonded_list.device[i].group_id = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
        g_lea_ucst_bonded_list.device[i].link_idx = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
    }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    memset(&g_lea_ucst_sirk_info[0], 0, sizeof(bt_lea_sink_info_nvkey_t)*APP_LE_AUDIO_NVKEY_SIRK_INFO_MAX_NUM);

    for (i = 0; i < APP_LE_AUDIO_NVKEY_SIRK_INFO_MAX_NUM; i++) {
        g_lea_ucst_sirk_info[i].group_id = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    }
#endif
}

static void app_le_audio_ucst_refresh_bonded_list(uint8_t empty_idx)
{
    uint8_t i;

    if (0 == g_lea_ucst_bonded_list.num) {
        return;
    }

    if (empty_idx < (g_lea_ucst_bonded_list.num - 1)) {
        for (i = empty_idx; i < g_lea_ucst_bonded_list.num; i++) {
            //LE_AUDIO_MSGLOG_I("[APP][U] refresh_bonded_list, %x", 1, i);
            if (g_lea_ucst_bonded_list.num == (i + 1)) {
                break;
            }
            memcpy(&(g_lea_ucst_bonded_list.device[i]), &(g_lea_ucst_bonded_list.device[i + 1]), sizeof(app_le_audio_ucst_bonded_device_t));
            app_le_audio_ucst_reset_bonded_list(i + 1);
        }
    }

#if 1 /* for test */
    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
        bt_addr_t *addr = NULL;
        addr = &(g_lea_ucst_bonded_list.device[i].addr);
        LE_AUDIO_MSGLOG_I("[APP][U] refresh_bonded_list[%x], addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x group:%x(size:%x)", 10, i,
                          addr->type,
                          addr->addr[5],
                          addr->addr[4],
                          addr->addr[3],
                          addr->addr[2],
                          addr->addr[1],
                          addr->addr[0],
                          g_lea_ucst_bonded_list.device[i].group_id,
                          g_lea_ucst_bonded_list.device[i].group_size);
    }
#endif
}

static void app_le_audio_ucst_refresh_all_bonded_list(void)
{
    uint8_t i, num;
    bool write_nvkey = false;

    if (0 == g_lea_ucst_bonded_list.num) {
        return;
    }

    /* remove invalid bonded record */
    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
        if ((APP_LE_AUDIO_UCST_GROUP_ID_INVALID != g_lea_ucst_bonded_list.device[i].group_id) &&
            (APP_LE_AUDIO_UCST_BONDED_RECORD_INVALID & g_lea_ucst_bonded_list.device[i].group_id)) {
            app_le_audio_ucst_reset_bonded_list(i);
            app_le_audio_ucst_refresh_bonded_list(i);
            g_lea_ucst_bonded_list.num--;
            write_nvkey = true;
        }
    }

    num = 0;
    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
        if (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_bonded_list.device[i].group_id) {
            num++;
        }
    }

    g_lea_ucst_bonded_list.num = num;

    /* write to nvkey */
    if (write_nvkey) {
        app_le_audio_ucst_write_bonded_list_to_nvkey();
    }

    LE_AUDIO_MSGLOG_I("[APP] refresh_all_bonded_list, num:%x write_nvkey:%x", 2, g_lea_ucst_bonded_list.num, write_nvkey);
}



static void app_le_audio_ucst_add_bonded_device_to_white_list(void)
{
    if (0 == g_lea_ucst_bonded_list.num) {
        LE_AUDIO_MSGLOG_I("[APP][U] add_bonded_device_to_white_list, bonded_list empty", 0);
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP] add_bonded_device_to_white_list, total_num:%x idx:%x state:%x", 3,
                      g_lea_ucst_bonded_list.num,
                      g_lea_ucst_set_white_list.curr_idx,
                      g_lea_ucst_set_white_list.state);

    if (APP_LE_AUDIO_UCST_SET_WHITE_LIST_ADD_ON_GOING == g_lea_ucst_set_white_list.state) {
        g_lea_ucst_set_white_list.curr_idx++;

        if (g_lea_ucst_bonded_list.num <= g_lea_ucst_set_white_list.curr_idx) {
            memset(&g_lea_ucst_set_white_list, 0, sizeof(app_le_audio_ucst_set_white_list_t));
            g_lea_ucst_set_white_list.state = APP_LE_AUDIO_UCST_SET_WHITE_LIST_COMPLETE;
            app_le_audio_ucst_connect_bonded_device(false);
            return;
        }
    }

    bt_addr_t device;
    bt_status_t ret;
    uint8_t idx;

    idx = g_lea_ucst_set_white_list.curr_idx;
    memcpy(&device, &(g_lea_ucst_bonded_list.device[idx].addr), sizeof(bt_addr_t));

    LE_AUDIO_MSGLOG_I("[APP] white_list[%x], addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 8, idx, device.type,
                      device.addr[5], device.addr[4],
                      device.addr[3], device.addr[2],
                      device.addr[1], device.addr[0]);

    g_lea_ucst_bonded_list.device[idx].in_white_list = true;
    g_lea_ucst_set_white_list.state = APP_LE_AUDIO_UCST_SET_WHITE_LIST_ADD_ON_GOING;

    //ret = bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &device);
    ret = app_le_audio_ucst_add_white_list(&device);

    LE_AUDIO_MSGLOG_I("[APP][U] add_white_list, ret:%x", 1, ret);
}

void app_le_audio_ucst_get_bonded_list(app_le_audio_ucst_bonded_list_t *p_bonded_list)
{
    uint8_t i;
    p_bonded_list->num =  g_lea_ucst_bonded_list.num;
    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
        memcpy(&p_bonded_list->device[i], &g_lea_ucst_bonded_list.device[i], sizeof(app_le_audio_ucst_bonded_device_t));
    }

}

static void app_le_audio_ucst_read_bonded_list_from_nvkey(void)
{
    nvkey_status_t ret = NVKEY_STATUS_OK;
    uint32_t size = APP_LE_AUDIO_NVKEY_BONDED_LIST_LEN;
    bt_lea_ucst_bonded_list_nvkey_t buf_nvkey;
    uint8_t i, num = 0;

    /* read NVID_BT_LEA_BONDED_LIST: 36 byte */
    /* NVID_BT_LEA_BONDED_LIST: 36 = | bonded_device * APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM (4) | */
    /* bonded_device: | addr (7 bytes) + group_id (1 byte) + group_size(1 byte) | */
    ret = nvkey_read_data(NVID_BT_LEA_BONDED_LIST, (uint8_t*)&buf_nvkey, &size);
    LE_AUDIO_MSGLOG_I("[APP][U] read_bonded_list(nvkey), ret:%x size:%d", 2, ret, size);

    if (NVKEY_STATUS_OK == ret) {
        /* update bonded list */
        for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
            memcpy(&(g_lea_ucst_bonded_list.device[i].addr), &buf_nvkey.device[i].addr, sizeof(bt_addr_t));
            g_lea_ucst_bonded_list.device[i].group_id = buf_nvkey.device[i].group_id;
            g_lea_ucst_bonded_list.device[i].in_white_list = true;
            if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID != g_lea_ucst_bonded_list.device[i].group_id) {
                num++;
            }
            g_lea_ucst_bonded_list.device[i].group_size =  buf_nvkey.device[i].group_size;

        }
    }

#if 1   /* For test */
    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
        bt_addr_t *addr = NULL;
        addr = &(g_lea_ucst_bonded_list.device[i].addr);
        LE_AUDIO_MSGLOG_I("[APP] bonded_list[%x], addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x group:%x(size:%x)", 10, i,
                          addr->type,
                          addr->addr[5],
                          addr->addr[4],
                          addr->addr[3],
                          addr->addr[2],
                          addr->addr[1],
                          addr->addr[0],
                          g_lea_ucst_bonded_list.device[i].group_id,
                          g_lea_ucst_bonded_list.device[i].group_size);
    }
#endif
    g_lea_ucst_bonded_list.num = num;
    LE_AUDIO_MSGLOG_I("[APP] bonded_list, total_num:%x", 1, g_lea_ucst_bonded_list.num);
}


static void app_le_audio_ucst_write_bonded_list_to_nvkey(void)
{
    nvkey_status_t ret = NVKEY_STATUS_ERROR;
    uint32_t size = APP_LE_AUDIO_NVKEY_BONDED_LIST_LEN;
    bt_lea_ucst_bonded_list_nvkey_t buf_nvkey;
    uint8_t i = 0, j = 0;

    /* write NVID_BT_LEA_BONDED_LIST: 36 byte */
    /* NVID_BT_LEA_BONDED_LIST: 36 = | bonded_device * APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM (4) | */
    /* bonded_device: | addr (7 bytes) + group_id (1 byte) + group_size(1 byte) | */
    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
        for (; j < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; j++) {
            if (!g_lea_ucst_bonded_list.device[j].deleting) {
                break;
            }
        }
        if (APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM > j) {
            memcpy(&buf_nvkey.device[i].addr, &(g_lea_ucst_bonded_list.device[j].addr), sizeof(bt_addr_t));
            buf_nvkey.device[i].group_id = g_lea_ucst_bonded_list.device[j].group_id;
            buf_nvkey.device[i].group_size = g_lea_ucst_bonded_list.device[j].group_size;

        } else {
            memset(&buf_nvkey.device[i].addr, 0, sizeof(bt_addr_t));
            buf_nvkey.device[i].group_id = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
            buf_nvkey.device[i].group_size = 0;
        }
        LE_AUDIO_MSGLOG_I("[APP][U] write_bonded_list, [%x] %x, %02x %02x %02x %02x %02x %02x, %x %x", 10, i,
                          buf_nvkey.device[i].addr.type,
                          buf_nvkey.device[i].addr.addr[0],
                          buf_nvkey.device[i].addr.addr[1],
                          buf_nvkey.device[i].addr.addr[2],
                          buf_nvkey.device[i].addr.addr[3],
                          buf_nvkey.device[i].addr.addr[4],
                          buf_nvkey.device[i].addr.addr[5],
                          buf_nvkey.device[i].group_id,
                          buf_nvkey.device[i].group_size);
        j++;
    }

    ret = nvkey_write_data(NVID_BT_LEA_BONDED_LIST, (uint8_t*)&buf_nvkey, size);
    LE_AUDIO_MSGLOG_I("[APP][U] write_bonded_list(nvkey), ret:%x", 1, ret);
}

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
static void app_le_audio_ucst_read_sirk_list_from_nvkey(void)
{
    nvkey_status_t ret = NVKEY_STATUS_OK;
    uint32_t size = APP_LE_AUDIO_NVKEY_SIRK_LIST_LEN;

    /* read NVID_BT_LEA_SIRK_LIST: 34 byte */
    /* NVID_BT_LEA_SIRK_LIST: 34 = (| SIRK (16 bytes) + group_id (1 byte) |) * 2 */
    ret = nvkey_read_data(NVID_BT_LEA_SIRK_LIST, (uint8_t *)&g_lea_ucst_sirk_info, &size);
    LE_AUDIO_MSGLOG_I("[APP][U] read_sirk_list(nvkey), ret:%x size:%d", 2, ret, size);

    if (NVKEY_STATUS_OK == ret) {
        bt_lea_sink_info_nvkey_t *ptr = NULL;
        uint8_t i;

        for (i = 0; i < APP_LE_AUDIO_NVKEY_SIRK_INFO_MAX_NUM; i++) {
            ptr = &g_lea_ucst_sirk_info[i];
            LE_AUDIO_MSGLOG_I("[APP][U] read_sirk_list, [%d]group:%x sirk:%x %x %x %x %x %x %x %x", 10,
                              i, ptr->group_id,
                              ptr->sirk[0], ptr->sirk[1], ptr->sirk[2], ptr->sirk[3],
                              ptr->sirk[4], ptr->sirk[5], ptr->sirk[6], ptr->sirk[7]);
            LE_AUDIO_MSGLOG_I("[APP][U] read_sirk_list, sirk:%x %x %x %x %x %x %x %x", 8,
                              ptr->sirk[8], ptr->sirk[9], ptr->sirk[10], ptr->sirk[11],
                              ptr->sirk[12], ptr->sirk[13], ptr->sirk[14], ptr->sirk[15]);
        }
    }
}

static void app_le_audio_ucst_write_sirk_list_to_nvkey(void)
{
    nvkey_status_t ret = NVKEY_STATUS_OK;
    uint32_t size = APP_LE_AUDIO_NVKEY_SIRK_LIST_LEN;

    /* read NVID_BT_LEA_SIRK_LIST: 34 byte */
    /* NVID_BT_LEA_SIRK_LIST: 34 = (| SIRK (16 bytes) + group_id (1 byte) |) * 2 */
    ret = nvkey_write_data(NVID_BT_LEA_SIRK_LIST, (uint8_t *)&g_lea_ucst_sirk_info, size);
    LE_AUDIO_MSGLOG_I("[APP][U] write_sirk_list(nvkey), ret:%x size:%d", 2, ret, size);
}
#endif

app_le_audio_ucst_conn_t app_le_audio_ucst_get_curr_conn_type(void)
{
    return g_lea_ucst_ctrl.curr_conn;
}

bool app_le_audio_ucst_disconnect_all_device(void)
{
    uint8_t i;

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;
    while (0 != i) {
        i--;
        if ((BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) &&
            (APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL == g_lea_ucst_link_info[i].next_state)) {
            return true;
        }
    }

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;
    while (0 != i) {
        i--;
        if ((BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) &&
            (APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_CIS != g_lea_ucst_link_info[i].next_state)) {
            g_lea_ucst_link_info[i].next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL;
            if (BT_STATUS_SUCCESS == app_le_audio_ucst_disconnect(g_lea_ucst_link_info[i].handle)) {
                return true;
            }
            g_lea_ucst_link_info[i].next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
        }
    }

    return false;
}

void app_le_audio_ucst_handle_set_white_list_cnf(bt_status_t ret)
{
    uint8_t i;

    LE_AUDIO_MSGLOG_I("[APP][U] LE_SET_WHITE_LIST_CNF, ret:%x", 1, ret);

    if (APP_LE_AUDIO_UCST_SET_WHITE_LIST_ADD_ON_GOING == g_lea_ucst_set_white_list.state) {
        app_le_audio_ucst_add_bonded_device_to_white_list();
    }
    if (APP_LE_AUDIO_UCST_SET_WHITE_LIST_REMOVE_ON_GOING == g_lea_ucst_set_white_list.state) {
        g_lea_ucst_set_white_list.state = APP_LE_AUDIO_UCST_SET_WHITE_LIST_COMPLETE;
        if (g_lea_ucst_waiting_disconn_flag) {
            LE_AUDIO_MSGLOG_I("[APP][U] LE_SET_WHITE_LIST_CNF, wait disconn", 0);
            return;
        }
        /* check remove next white list */
        if (app_le_audio_ucst_check_delete_group_device()) {
            return;
        }

        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if ((BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) &&
                (g_lea_ucst_link_info[i].add_white_list)) {
                g_lea_ucst_link_info[i].add_white_list = false;
                LE_AUDIO_MSGLOG_I("[APP] set_white_list_cnf, set_white_list.state:%x", 1, g_lea_ucst_set_white_list.state);
                app_le_audio_ucst_add_white_list(&(g_lea_ucst_link_info[i].addr));
                break;
            }
        }
    }
}

void app_le_audio_ucst_handle_scan_cnf(bt_status_t ret)
{
    LE_AUDIO_MSGLOG_I("[APP][U] LE_SET_SCAN_CNF, ret:%x scan:%x->%x conn:%x", 4, ret,
                      g_lea_ucst_ctrl.curr_scan, g_lea_ucst_ctrl.next_scan,
                      g_lea_ucst_ctrl.curr_conn);

    if (BT_STATUS_SUCCESS != ret) {
        /* handle error case */
        switch (g_lea_ucst_ctrl.curr_scan) {
            case APP_LE_AUDIO_UCST_SCAN_ENABLING:
            case APP_LE_AUDIO_UCST_SCAN_CS_ENABLING: {
                g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
                bt_app_common_at_cmd_print_report("Stop scanning.\r\n");

                if (APP_LE_AUDIO_UCST_CONN_LEA_DEVICE == g_lea_ucst_ctrl.curr_conn) {
                    g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
                    if (g_lea_ucst_callback) {
                        app_dongle_le_race_scan_cnf_t cnf;
                        cnf.ret = ret;
                        cnf.start_scan = true;
                        g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_SCAN_CNF, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &cnf);
                    }
                }
                break;
            }
            case APP_LE_AUDIO_UCST_SCAN_DISABLING: {
                g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_ENABLED;
                if (APP_LE_AUDIO_UCST_CONN_LEA_DEVICE == g_lea_ucst_ctrl.curr_conn) {
                    if (g_lea_ucst_callback) {
                        app_dongle_le_race_scan_cnf_t cnf;
                        cnf.ret = ret;
                        cnf.start_scan = false;
                        g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_SCAN_CNF, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &cnf);
                    }
                }
                break;
            }
            case APP_LE_AUDIO_UCST_SCAN_CS_DISABLING: {
                g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_CS_ENABLED;
                break;
            }
            default:
                break;
        }
        g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
        return;
    }

    if (g_lea_ucst_ctrl.next_scan == g_lea_ucst_ctrl.curr_scan) {
        g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
    }

    /* update curr_scan */
    switch (g_lea_ucst_ctrl.curr_scan) {
        case APP_LE_AUDIO_UCST_SCAN_ENABLING: {
            g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_ENABLED;
            bt_app_common_at_cmd_print_report("Start scanning:\r\n");
            if (APP_LE_AUDIO_UCST_CONN_LEA_DEVICE == g_lea_ucst_ctrl.curr_conn) {
                if (g_lea_ucst_callback) {
                    app_dongle_le_race_scan_cnf_t cnf;
                    cnf.ret = ret;
                    cnf.start_scan = true;
                    g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_SCAN_CNF, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &cnf);
                }
            }
            if ((APP_LE_AUDIO_UCST_SCAN_CS_ENABLING == g_lea_ucst_ctrl.next_scan) ||
                (APP_LE_AUDIO_UCST_SCAN_DISABLING == g_lea_ucst_ctrl.next_scan)) {
                if (BT_STATUS_SUCCESS != app_le_audio_ucst_stop_scan()) {
                    g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
                }
                return;
            }
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_CS_ENABLING: {
            g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_CS_ENABLED;
            bt_app_common_at_cmd_print_report("Start scanning:\r\n");
            if ((APP_LE_AUDIO_UCST_SCAN_ENABLING == g_lea_ucst_ctrl.next_scan) ||
                (APP_LE_AUDIO_UCST_SCAN_CS_DISABLING == g_lea_ucst_ctrl.next_scan)) {
                if (BT_STATUS_SUCCESS != app_le_audio_ucst_stop_scan_cs()) {
                    g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
                }
                return;
            }
            break;
        }
        case APP_LE_AUDIO_UCST_SCAN_DISABLING:
        case APP_LE_AUDIO_UCST_SCAN_CS_DISABLING: {
            app_le_audio_ucst_scan_t curr_scan = g_lea_ucst_ctrl.curr_scan;
            g_lea_ucst_ctrl.curr_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
            bt_app_common_at_cmd_print_report("Stop scanning.\r\n");
            if ((APP_LE_AUDIO_UCST_SCAN_DISABLING == curr_scan) &&
                (APP_LE_AUDIO_UCST_CONN_LEA_DEVICE == g_lea_ucst_ctrl.curr_conn)) {
                if (APP_LE_AUDIO_UCST_SCAN_NONE == g_lea_ucst_ctrl.next_scan) {
                    g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
                }
                if (g_lea_ucst_callback) {
                    app_dongle_le_race_scan_cnf_t cnf;
                    cnf.ret = ret;
                    cnf.start_scan = false;
                    g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_SCAN_CNF, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &cnf);
                }
            }
            if ((APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode) ||
                (APP_LE_AUDIO_MODE_DISABLE == g_lea_ctrl.next_mode)) {
                /* [Switch streaming mode] UCST -> BCST */
                if (0 == app_le_audio_ucst_get_link_num_ex()) {
                    if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode) {
                        g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_BCST;
                        g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
                        app_le_audio_start_broadcast();
                    } else {
                        g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_NONE;
                        g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
                        app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_CIS;
                        app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, &lea_mode);
                    }
                }
                return;
            }
            if (APP_LE_AUDIO_UCST_SCAN_ENABLING == g_lea_ucst_ctrl.next_scan) {
                if (APP_LE_AUDIO_UCST_CONN_BONDED_DEVICE == g_lea_ucst_ctrl.curr_conn) {
                    if (BT_STATUS_SUCCESS != app_le_audio_ucst_start_scan(true)) {
                        g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
                    }
                } else {
                    if (BT_STATUS_SUCCESS != app_le_audio_ucst_start_scan(false)) {
                        g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
                    }
                }
                return;
            }
            if (APP_LE_AUDIO_UCST_SCAN_CS_ENABLING == g_lea_ucst_ctrl.next_scan) {
                if (BT_STATUS_SUCCESS != app_le_audio_ucst_start_scan_cs()) {
                    g_lea_ucst_ctrl.next_scan = APP_LE_AUDIO_UCST_SCAN_NONE;
                }
                return;
            }
            break;
        }
        default:
            break;
    }
}

void app_le_audio_ucst_handle_adv_report_ind(bt_status_t ret, bt_gap_le_ext_advertising_report_ind_t *ind)
{
    bt_bd_addr_t empty_addr = {0};
    uint8_t link_num;

    if (APP_LE_AUDIO_UCST_CONN_NONE == g_lea_ucst_ctrl.curr_conn) {
        //LE_AUDIO_MSGLOG_I("[APP][U] handle_adv_report, curr_conn:%x", 1, g_lea_ucst_ctrl.curr_conn);
        return;
    }

    if (BT_STATUS_SUCCESS != ret) {
        return;
    }

    /* check current link num */
    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == (link_num = app_le_audio_ucst_get_link_num_ex())) {
#ifndef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        LE_AUDIO_MSGLOG_I("[APP][U] handle_adv_report, link full", 0);
        g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
        app_le_audio_ucst_stop_scan();
        return;
#endif
    }


    if (!(ind->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_CONNECTABLE)) {
        /* ignore non-connectable adv */
        return;
    }

    /* check address */
    if ((0 == memcmp(ind->address.addr, empty_addr, sizeof(bt_bd_addr_t))) ||
        ((ind->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_DIRECTED) &&
         (0 == memcmp(ind->direct_address.addr, empty_addr, sizeof(bt_bd_addr_t))))) {
        LE_AUDIO_MSGLOG_I("[APP][U] handle_adv_report, event_type:%x invalid addr!", 1, ind->event_type);
        return;
    }

    if (app_le_audio_ucst_is_connected_device(&ind->address)) {
        /* ignore connected device */
        return;
    }

    if (!app_lea_dongle_customer_check_adv_data(ind)) {
        /* Not found the target customer adv data in ADV */
        return;
    }

    /* upper layer scan device */
    if (APP_LE_AUDIO_UCST_CONN_LEA_DEVICE == g_lea_ucst_ctrl.curr_conn) {
        if (ind->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_DIRECTED) {
            if (!app_le_audio_ucst_is_bonded_device(&ind->address, true)) {
                /* ignore unknown device */
                return;
            }
        } else if (!app_le_audio_ucst_is_lea_adv(ind)) {
            /* ignore not le audio device */
            return;
        }

        app_le_audio_ucst_print_lea_adv_addr(&ind->address);
        return;
    }

    /* connect bonded device */
    if ((APP_LE_AUDIO_UCST_CONN_BONDED_DEVICE == g_lea_ucst_ctrl.curr_conn) ||
        (APP_LE_AUDIO_UCST_CONN_CS_AND_BONDED_DEVICE == g_lea_ucst_ctrl.curr_conn)) {
        if (!app_le_audio_ucst_is_lea_adv(ind)) {
            /* ignore not le audio device */
            return;
        }
        if (app_le_audio_ucst_is_bonded_device(&ind->address, true)) {
            app_le_audio_ucst_print_lea_adv_addr(&ind->address);
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
            if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == link_num) {
                if (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_waiting_conn_group) {
                    LE_AUDIO_MSGLOG_I("[APP][CSIP] handle_adv_report, link full scan 1, found group:%x", 1, g_lea_ucst_waiting_conn_group);
                    memcpy(&g_lea_ucst_waiting_conn_addr, &ind->address, sizeof(bt_addr_t));
                    app_le_audio_ucst_stop_scan();
                    app_le_audio_ucst_disconnect_other_group_device(g_lea_ucst_waiting_conn_group, AUDIO_LOCATION_FRONT_LEFT);
                }
            } else
#endif
            {
                app_le_audio_ucst_connect_device(&ind->address);
            }
            return;
        }
    }

    /* connect cs device */
    if ((APP_LE_AUDIO_UCST_CONN_COORDINATED_SET_BY_SIRK == g_lea_ucst_ctrl.curr_conn) ||
        (APP_LE_AUDIO_UCST_CONN_CS_AND_BONDED_DEVICE == g_lea_ucst_ctrl.curr_conn)) {
        if (app_le_audio_ucst_is_lea_adv(ind) &&
            (!app_le_audio_ucst_check_bond_and_not_in_white_list(&ind->address))) {

            uint16_t length = 0, index = 0;

            while (index < ind->data_length) {
                length = ind->data[index];

                if (0 == length) {
                    break;
                }

                if ((length >= 7) && ((ind->data[index + 1] == 0xF0) || (ind->data[index + 1] == APP_LE_AUDIO_ADV_TYPE_RSI))) {
                    if (BT_STATUS_SUCCESS == ble_csip_verify_rsi(&ind->data[index + 2])) {
                        app_le_audio_ucst_print_lea_adv_addr(&ind->address);
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == link_num) {
                            if (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_waiting_conn_group) {
                                LE_AUDIO_MSGLOG_I("[APP][CSIP] handle_adv_report, link full scan 2, found group:%x", 1, g_lea_ucst_waiting_conn_group);
                                memcpy(&g_lea_ucst_waiting_conn_addr, &ind->address, sizeof(bt_addr_t));
                                app_le_audio_ucst_stop_scan();
                                app_le_audio_ucst_disconnect_other_group_device(g_lea_ucst_waiting_conn_group, AUDIO_LOCATION_FRONT_LEFT);
                            }
                        } else
#endif
                        {
                            app_le_audio_ucst_connect_device(&ind->address);
                        }
                    } else {
                        LE_AUDIO_MSGLOG_I("[APP] csip_verify_rsi fail 3", 0);
                    }
                    return;
                }

                if ((length >= 10) && ((ind->data[index + 1] == APP_LE_AUDIO_ADV_TYPE_MSD) && (ind->data[index + 2] == 0x94) && (ind->data[index + 5] == APP_LE_AUDIO_ADV_TYPE_RSI))) {
                    if (BT_STATUS_SUCCESS == ble_csip_verify_rsi(&ind->data[index + 6])) {
                        app_le_audio_ucst_print_lea_adv_addr(&ind->address);
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == link_num) {
                            if (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_waiting_conn_group) {
                                LE_AUDIO_MSGLOG_I("[APP][CSIP] handle_adv_report, link full scan 3, found group:%x", 1, g_lea_ucst_waiting_conn_group);
                                memcpy(&g_lea_ucst_waiting_conn_addr, &ind->address, sizeof(bt_addr_t));
                                app_le_audio_ucst_stop_scan();
                                app_le_audio_ucst_disconnect_other_group_device(g_lea_ucst_waiting_conn_group, AUDIO_LOCATION_FRONT_LEFT);
                            }
                        } else
#endif
                        {
                            app_le_audio_ucst_connect_device(&ind->address);
                        }
                    } else {
                        LE_AUDIO_MSGLOG_I("[APP] csip_verify_rsi fail 4", 0);
                    }
                    return;
                }
                index = index + length + 1;
            }
        }
    }
}

void app_le_audio_ucst_handle_connect_ind(bt_status_t ret, bt_gap_le_connection_ind_t *ind)
{
    app_le_audio_ucst_link_info_t *link_info = NULL;
    uint8_t link_idx = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
    app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_CIS;

    if (!((APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) ||
          (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) ||
          (APP_LE_AUDIO_MODE_ASIT == g_lea_ctrl.curr_mode))) {
        LE_AUDIO_MSGLOG_I("[APP][U] LE_CONNECT_IND failed, handle:%x mode:%x %x", 3, ind->connection_handle, g_lea_ctrl.curr_mode, g_lea_ctrl.next_mode);
        return;
    }

    if (NULL != (link_info = app_le_audio_ucst_find_connecting_link_info_by_peer_addr(&(ind->peer_addr), &link_idx))) {
        app_le_audio_timer_stop(link_info->le_connection_timer_handle);
        link_info->le_connection_timer_handle = NULL;
    }

    //g_lea_ucst_ctrl.is_creating_connection = false;

    if ((BT_STATUS_SUCCESS != ret) || (BT_HANDLE_INVALID == ind->connection_handle)) {
        LE_AUDIO_MSGLOG_I("[APP][U] LE_CONNECT_IND failed, handle:%x ret:%x curr_conn:%x", 3, ind->connection_handle, ret, g_lea_ucst_ctrl.curr_conn);
        app_le_audio_ucst_reset_link_info(link_idx);
        if ((0x02 == ret) && (0x0000 == ind->connection_handle)) {
            // cancel_create_connection maybe sent by GAP layer automatically.
            if (g_lea_ucst_waiting_cancel_create_connection) {
                app_le_audio_ucst_handle_cancel_create_connection_cnf(ret, ind);
            }
            return;
        }

        switch (g_lea_ucst_ctrl.curr_conn) {
            case APP_LE_AUDIO_UCST_CONN_LEA_DEVICE: {
                g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
                app_le_audio_ucst_find_device();
                break;
            }
            case APP_LE_AUDIO_UCST_CONN_BONDED_DEVICE: {
                app_le_audio_ucst_connect_bonded_device(false);
                break;
            }
            case APP_LE_AUDIO_UCST_CONN_COORDINATED_SET_BY_SIRK: {
                app_le_audio_ucst_connect_coordinated_set(false);
                break;
            }
            case APP_LE_AUDIO_UCST_CONN_CS_AND_BONDED_DEVICE: {
                app_le_audio_ucst_connect_bonded_device(true);
                break;
            }
            default:
                break;
        }

        if (g_lea_ucst_callback) {
            app_dongle_le_race_connect_ind_t evt;

            evt.ret = ret;
            memcpy(&(evt.peer_addr), &(ind->peer_addr), sizeof(bt_addr_t));
            evt.group_id = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
            evt.group_size = 0;
            g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_CONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
        }

        app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_STARTED, BT_STATUS_FAIL, &lea_mode);
        return;
    }

#if 0
    /* LE connection timer may re-start scan.
        * Scan will be re-started if the coordinated set size of CSIS is more than one.
        */
    app_le_audio_ucst_stop_scan_all();
#endif

    g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;

#if 0
    if (((APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) && (APP_LE_AUDIO_MODE_NONE == g_lea_ctrl.next_mode)) ||
        (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode)) {
        /* [Switch streaming mode] UCST -> BCST */
        app_le_audio_ucst_reset_link_info(link_idx);
        app_le_audio_ucst_disconnect(ind->connection_handle);
        return;
    }
#endif

    /* Timer may be expired. */
    if (NULL == link_info) {
        link_idx = app_le_audio_ucst_get_link_idx(ind->connection_handle);
        LE_AUDIO_MSGLOG_I("[APP][U] link_idx:%x, handle:%x", 2, link_idx, ind->connection_handle);
        if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == link_idx) {
            link_info = app_le_audio_ucst_get_available_link_info_for_new_le_connection(&link_idx);
            if (NULL == link_info) {
                /* Link full. */
                app_le_audio_ucst_disconnect(ind->connection_handle);
                return;
            }
        } else {
            /* Duplicate connect_ind? */
            return;
        }
    }

    uint8_t bond_idx;

    /* link_info: update content */
    memset(link_info, 0, sizeof(app_le_audio_ucst_link_info_t));
    link_info->handle = ind->connection_handle;
    memcpy(&(link_info->addr), &(ind->peer_addr), sizeof(bt_addr_t));
    link_info->curr_interval = ind->conn_interval;
    link_info->group_id = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    link_info->bond_idx = APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID;

    /* search bonding info */
    if (APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID == (bond_idx = app_le_audio_ucst_get_bonded_list_idx(&(link_info->addr)))) {
        /* new device */

        if (APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM == g_lea_ucst_bonded_list.num) {
            /* bonded list full, delete oldest group */
            app_le_audio_ucst_delete_oldest_group_from_bonded_list();
        }
        if (APP_LE_AUDIO_UCST_SET_WHITE_LIST_REMOVE_ON_GOING != g_lea_ucst_set_white_list.state) {
            /* add to white list */
            app_le_audio_ucst_add_white_list(&(link_info->addr));
        } else {
            /* wait remove white list finish */
            link_info->add_white_list = true;
        }

    } else {
        /* bonded device */

        /* link_info: update bonding info */
        link_info->bond_idx = bond_idx;
        link_info->group_size = g_lea_ucst_bonded_list.device[bond_idx].group_size;
        link_info->group_id = g_lea_ucst_bonded_list.device[bond_idx].group_id;
#ifndef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_ctrl.curr_group) {
            g_lea_ucst_ctrl.curr_group = link_info->group_id;
            LE_AUDIO_MSGLOG_I("[APP][U] LE_CONNECT_IND, active_group:%x", 1, g_lea_ucst_ctrl.curr_group);
        }
#endif
        g_lea_ucst_ctrl.latest_group = link_info->group_id;

        /* bonded_list: update link idx */
        g_lea_ucst_bonded_list.device[bond_idx].link_idx = link_idx;

        /* group_info: update link idx */
        app_le_audio_ucst_add_link_to_group(link_info->group_id, link_idx);

        if (!g_lea_ucst_bonded_list.device[bond_idx].in_white_list) {
            g_lea_ucst_bonded_list.device[bond_idx].in_white_list = true;
            if (APP_LE_AUDIO_UCST_SET_WHITE_LIST_REMOVE_ON_GOING != g_lea_ucst_set_white_list.state) {
                /* add to white list */
                app_le_audio_ucst_add_white_list(&(link_info->addr));
            } else {
                /* wait remove white list finish */
                link_info->add_white_list = true;
            }
        }
        app_le_audio_ucst_check_connect_coordinated_set(link_info);
    }

    LE_AUDIO_MSGLOG_I("[APP][U] LE_CONNECT_IND, handle[%x]:%x addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 9,
                      link_idx,
                      link_info->handle,
                      link_info->addr.type,
                      link_info->addr.addr[5],
                      link_info->addr.addr[4],
                      link_info->addr.addr[3],
                      link_info->addr.addr[2],
                      link_info->addr.addr[1],
                      link_info->addr.addr[0]);

    /* update connection parameter */
    if (APP_LE_AUDIO_UCST_PAUSE_STREAM_NONE == g_lea_ucst_ctrl.pause_stream) {
        LE_AUDIO_MSGLOG_I("[APP][U] LE_CONNECT_IND handle:%x interval:%x->%x set:8", 3,
                          link_info->handle,
                          link_info->curr_interval,
                          link_info->next_interval);
        app_le_audio_ucst_increase_connection_config_speed(link_info->handle);
    }
    app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_STARTED, BT_STATUS_SUCCESS, &lea_mode);
}


void app_le_audio_ucst_handle_disconnect_ind(bt_status_t ret, bt_gap_le_disconnect_ind_t *ind)
{
    app_dongle_le_race_disconnect_ind_t evt;
    uint8_t link_idx, i, group_id;

    LE_AUDIO_MSGLOG_I("[APP][U] LE_DISCONNECT_IND, handle:%x ret:%x reason:%x", 3, ind->connection_handle, ret, ind->reason);

    if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == (link_idx = app_le_audio_ucst_get_link_idx(ind->connection_handle))) {
        LE_AUDIO_MSGLOG_I("[APP][U] LE_DISCONNECT_IND, link not exist (hdl:%x)", 1, ind->connection_handle);
        return;
    }

    evt.ret = ret;
    memcpy(&(evt.peer_addr), &(g_lea_ucst_link_info[link_idx].addr), sizeof(bt_addr_t));
    evt.group_id = g_lea_ucst_link_info[link_idx].group_id;
    evt.group_size = g_lea_ucst_link_info[link_idx].group_size;

    if (BT_STATUS_SUCCESS != ret) {
        if (g_lea_ucst_callback) {
            g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
        }
        return;
    }
    app_le_audio_ucst_link_info_t *p_info = &g_lea_ucst_link_info[link_idx];
    if (NULL != p_info->conn_interval_timer_handle) {
        app_le_audio_timer_stop(p_info->conn_interval_timer_handle);
        p_info->conn_interval_timer_handle = NULL;
    }

    group_id = g_lea_ucst_link_info[link_idx].group_id;

    /* If all remaining LE link(s) did not receive any RELEASING notification, reset g_lea_ucst_ctrl.release to false. */
    g_lea_ucst_link_info[link_idx].release = FALSE;
    for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
        if (BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle &&
            g_lea_ucst_link_info[i].release) {
            break;
        }
    }

    // TODO: manage release by group?
    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= i) {
        g_lea_ucst_ctrl.release = false;
    }
    if (g_lea_ucst_session_manager_cb) {
        g_lea_ucst_session_manager_cb->lea_session_disconnected(ind->connection_handle);
    }

    if (g_lea_ucst_waiting_disconn_flag) {
        g_lea_ucst_waiting_disconn_flag = FALSE;
        LE_AUDIO_MSGLOG_I("[APP] DISCONNECT_IND, delete_device", 0);
        app_le_audio_ucst_reset_link_info(link_idx);
        app_le_audio_ucst_check_delete_group_device();
        app_le_audio_ucst_check_close_audio_stream();
        if (g_lea_ucst_callback) {
            g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
        }
        return;
    }

    /* bonded list: */
    uint8_t bond_idx;
    if (APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID != (bond_idx = app_le_audio_ucst_get_bonded_list_idx(&(g_lea_ucst_link_info[link_idx].addr)))) {
        LE_AUDIO_MSGLOG_I("[APP][U] disconnect_device, link_idx:%x in_white_list:%x", 2,
                          g_lea_ucst_bonded_list.device[bond_idx].link_idx,
                          g_lea_ucst_bonded_list.device[bond_idx].in_white_list);

        g_lea_ucst_bonded_list.device[bond_idx].link_idx = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
        g_lea_ucst_bonded_list.device[bond_idx].reason = ind->reason;   /**< Disconnect reason. */
#if 0
        if (BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES == ind->reason) {
            g_lea_ucst_bonded_list.device[bond_idx].in_white_list = false;
            app_le_audio_ucst_remove_white_list(&(g_lea_ucst_link_info[link_idx].addr));
        }
#endif
    }

    app_le_audio_ucst_remove_link_from_group(group_id, link_idx);
    app_le_audio_ucst_reset_link_info(link_idx);

    LE_AUDIO_MSGLOG_I("[APP] DISCONNECT_IND, target:%x->%x stream_state:%x->%x", 4,
                      g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target,
                      g_lea_ucst_ctrl.curr_stream_state, g_lea_ucst_ctrl.next_stream_state);


    if (g_lea_ucst_waiting_conn_flag) {
        g_lea_ucst_waiting_conn_flag = false;
        app_le_audio_ucst_check_close_audio_stream();
        LE_AUDIO_MSGLOG_I("[APP] DISCONNECT_IND, connect_device other device", 0);
        app_le_audio_ucst_connect_device(&g_lea_ucst_waiting_conn_addr);
        g_lea_ucst_waiting_conn_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
        memset(&g_lea_ucst_waiting_conn_addr, 0, sizeof(bt_addr_t));
        if (g_lea_ucst_callback) {
            g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
        }
        return;
    }

    uint8_t link_num = 0;
    link_num = app_le_audio_ucst_get_link_num_ex();

#ifndef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if (0 == link_num) {
        g_lea_ucst_ctrl.curr_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
        g_lea_ucst_ctrl.next_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
        g_lea_ucst_ctrl.latest_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    }
#endif

    LE_AUDIO_MSGLOG_I("[APP] DISCONNECT_IND, mode: %x %x", 2, g_lea_ctrl.curr_mode, g_lea_ctrl.next_mode);
    if (APP_LE_AUDIO_MODE_DISABLE == g_lea_ctrl.next_mode) {
        app_le_audio_ucst_check_close_audio_stream();
        if (0 != link_num) {
            app_le_audio_ucst_disconnect_all_device();

        } else if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
                   (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {
            g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_NONE;
            g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
            app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_CIS;
            app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, &lea_mode);
        }
        if (g_lea_ucst_callback) {
            g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
        }
        return;
    }

    if (((APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) && (APP_LE_AUDIO_MODE_NONE == g_lea_ctrl.next_mode)) ||
        (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.next_mode)) {
        if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
            (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {
            app_le_audio_start_broadcast();
            app_le_audio_ucst_scan_and_reconnect_device(bond_idx);
        }
        else {
            app_le_audio_ucst_check_close_audio_stream();
        }
        if (g_lea_ucst_callback) {
            g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
        }
        return;
    }

    if (((APP_LE_AUDIO_MODE_ASIT == g_lea_ctrl.curr_mode) && (APP_LE_AUDIO_MODE_NONE == g_lea_ctrl.next_mode)) ||
        (APP_LE_AUDIO_MODE_ASIT == g_lea_ctrl.next_mode)) {
        if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
            (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {
            app_le_audio_ucst_scan_and_reconnect_device(bond_idx);
        }
        else {
            app_le_audio_ucst_check_close_audio_stream();
        }
        if (g_lea_ucst_callback) {
            g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
        }
        return;
    }

    if (BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST == ind->reason) {
        if ((APP_LE_AUDIO_UCST_GROUP_ID_MAX > group_id) && (1 < g_lea_ucst_group_info[group_id].size)) {
            for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM; i++) {
                if (APP_LE_AUDIO_UCST_LINK_MAX_NUM > g_lea_ucst_group_info[group_id].link_idx[i]) {
                    app_le_audio_ucst_disconnect_device_by_idx(g_lea_ucst_group_info[group_id].link_idx[i]);
                    break;
                }
            }
        }
    }

    bool scan_enable = true;
    if (!app_le_audio_ucst_scan_and_reconnect_device(bond_idx)) {
        scan_enable = false;
        if ((APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID == bond_idx) && (BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST != ind->reason)) {
            if(BT_STATUS_SUCCESS == app_le_audio_ucst_connect_bonded_device(true)) {
                scan_enable = true;
            }
        }
    }

    uint8_t cis_num = 0;

    cis_num = app_le_audio_ucst_get_cis_num();

    LE_AUDIO_MSGLOG_I("[APP] DISCONNECT_IND, link_num:%x cis_num:%x cig_created:%x", 3, link_num, cis_num, g_lea_ucst_ctrl.is_cig_created);

    if (0 != link_num) {
        if (0 != cis_num) {
            app_le_audio_ucst_set_mic_channel();
            if (g_lea_ucst_callback) {
                g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
            }
            return;
        }

        if ((APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE == g_lea_ucst_ctrl.curr_target) ||
            (APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE == g_lea_ucst_ctrl.curr_target)) {
            uint8_t tmp;
            for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
                    continue;
                }
#else
                i = tmp;
#endif
                if (BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) {
                    LE_AUDIO_MSGLOG_I("[APP] DISCONNECT_IND, check handle:%x state:%x->%x", 3, g_lea_ucst_link_info[i].handle,
                                      g_lea_ucst_link_info[i].curr_state, g_lea_ucst_link_info[i].next_state);

                    if ((APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE == g_lea_ucst_link_info[i].curr_state) &&
                        (APP_LE_AUDIO_UCST_LINK_STATE_IDLE == g_lea_ucst_link_info[i].next_state) &&
                        (false == g_lea_ucst_link_info[i].ase_releasing)) {
                        g_lea_ucst_link_info[i].next_state = APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS;
                        app_le_audio_ucst_create_cis();
                        if (g_lea_ucst_callback) {
                            g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
                        }
                        return;
                    }
                    if ((APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS > g_lea_ucst_link_info[i].curr_state) &&
                        (APP_LE_AUDIO_UCST_LINK_STATE_IDLE != g_lea_ucst_link_info[i].next_state)) {
                        /* To do: check ase releasing */
                        if (g_lea_ucst_callback) {
                            g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
                        }
                        return;
                    }
                }
            }
        }
        else if ((APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.curr_target) &&
            (APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.curr_target)) {
            app_le_audio_ucst_start();
        }
    } else {
        if (APP_LE_AUDIO_UCST_PAUSE_STREAM_DONGLE_FOTA != g_lea_ucst_ctrl.pause_stream) {
            g_lea_ucst_ctrl.pause_stream = APP_LE_AUDIO_UCST_PAUSE_STREAM_NONE;
        }
    }

    if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE < g_lea_ucst_ctrl.curr_stream_state) &&
        (APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING >= g_lea_ucst_ctrl.curr_stream_state) &&
        (APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM != g_lea_ucst_ctrl.next_stream_state)) {

        /* stop audio stream */
        g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM;
        if (BT_STATUS_SUCCESS != app_le_audio_close_audio_transmitter()) {
            g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_IDLE;
        }
    } else if ((0 == link_num) &&
               (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
               (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {
        g_lea_ucst_ctrl.curr_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        g_lea_ucst_ctrl.next_target = APP_LE_AUDIO_UCST_TARGET_NONE;
        if (!scan_enable) {
            g_lea_ctrl.curr_mode = APP_LE_AUDIO_MODE_NONE;
            g_lea_ctrl.next_mode = APP_LE_AUDIO_MODE_NONE;
            app_dongle_cm_lea_mode_t lea_mode = APP_DONGLE_CM_LEA_MODE_CIS;
            app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_LEA, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, &lea_mode);
        }
    } else if ((0 == link_num) &&
               (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
               (APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM == g_lea_ucst_ctrl.next_stream_state)) {
        /* This may happen when restart unicast on receiving the terminated ind of the last CIS and after that the last LE link is disconnected just before
         * audio transmitter is opened which is triggered by restart unicast.
         */
        g_lea_ucst_ctrl.next_stream_state = APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM;
    }

    if (g_lea_ucst_callback) {
        g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_DISCONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
    }
}


void app_le_audio_ucst_handle_connection_update_ind(bt_status_t ret, bt_gap_le_connection_update_ind_t *ind)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(ind->conn_handle))) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] LE_CONNECTION_UPDATE_IND, handle:%x ret:%x interval:%x %x->%x", 5,
                      ind->conn_handle, ret,
                      ind->conn_interval,
                      p_info->curr_interval,
                      p_info->next_interval);
    if (BT_STATUS_SUCCESS == ret) {
        p_info->curr_interval = ind->conn_interval;
        if (p_info->next_interval == ind->conn_interval) {
            p_info->next_interval = 0;
        }
        app_le_audio_ucst_create_cis();
    }
    else {
        p_info->next_interval = 0;
    }
}

void app_le_audio_ucst_reset_all_bonded_info(void)
{
    bt_device_manager_le_clear_all_bonded_info();
    app_le_audio_ucst_reset_all_bonded_list();
    app_le_audio_ucst_write_bonded_list_to_nvkey();
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    app_le_audio_ucst_write_sirk_list_to_nvkey();
#endif
    app_le_audio_ucst_reset_all_group_info();
    g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
    app_le_audio_ucst_stop_scan_all();
}

bool app_le_audio_ucst_is_connected_device(const bt_addr_t *addr)
{
    uint8_t i;
    bt_device_manager_le_bonded_info_t * p_bonded_info = NULL;

#if 0
    /* For test */
    LE_AUDIO_MSGLOG_I("[APP] is_connected_device, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      addr->type,
                      addr->addr[5],
                      addr->addr[4],
                      addr->addr[3],
                      addr->addr[2],
                      addr->addr[1],
                      addr->addr[0]);
#endif

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        if (BT_HANDLE_INVALID == g_lea_ucst_link_info[i].handle) {
            continue;
        }

#if 0
        LE_AUDIO_MSGLOG_I("[APP] connected_list, handle[%x]:%x addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 9,
                          i,
                          g_lea_ucst_link_info[i].handle,
                          g_lea_ucst_link_info[i].addr.type,
                          g_lea_ucst_link_info[i].addr.addr[5],
                          g_lea_ucst_link_info[i].addr.addr[4],
                          g_lea_ucst_link_info[i].addr.addr[3],
                          g_lea_ucst_link_info[i].addr.addr[2],
                          g_lea_ucst_link_info[i].addr.addr[1],
                          g_lea_ucst_link_info[i].addr.addr[0]);
#endif

        if (0 == memcmp(&g_lea_ucst_link_info[i].addr, addr, sizeof(bt_addr_t))) {
            LE_AUDIO_MSGLOG_I("[APP] is_connected_device, YES!", 0);
            return true;
        }

        p_bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)&addr->addr);
        if ((NULL != p_bonded_info) && (0 == memcmp(p_bonded_info->bt_addr.addr, g_lea_ucst_link_info[i].addr.addr, sizeof(bt_bd_addr_t)))) {
            LE_AUDIO_MSGLOG_I("[APP] is_connected_device, YES!", 0);
            return true;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP] is_connected_device, NO!", 0);
    return false;
}

static bool app_le_audio_ucst_is_bonded_device(const bt_addr_t *addr, bool check_white_list)
{
    uint8_t i;
    bt_device_manager_le_bonded_info_t * p_bonded_info = NULL;

    if (0 == g_lea_ucst_bonded_list.num) {
        return false;
    }
    p_bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)&addr->addr);

#if 0
    LE_AUDIO_MSGLOG_I("[APP] is_bonded_device, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      addr->type,
                      addr->addr[5],
                      addr->addr[4],
                      addr->addr[3],
                      addr->addr[2],
                      addr->addr[1],
#endif

    for (i = 0; i < g_lea_ucst_bonded_list.num; i++) {
        /*LE_AUDIO_MSGLOG_I("[APP][U] is_bonded_device, i:%x group:%x(size:%x) in_white_list:%x", 4, i,
                          g_lea_ucst_bonded_list.device[i].group_id,
                          g_lea_ucst_bonded_list.device[i].group_size,
                          g_lea_ucst_bonded_list.device[i].in_white_list);*/
        if ((0 != g_lea_ucst_bonded_list.device[i].group_size) &&
            ((0 == memcmp(&(g_lea_ucst_bonded_list.device[i].addr), addr, sizeof(bt_addr_t))) ||
            ((NULL != p_bonded_info) && (0 == memcmp(&(g_lea_ucst_bonded_list.device[i].addr), &p_bonded_info->bt_addr, sizeof(bt_addr_t)))))){
            if (check_white_list && (!g_lea_ucst_bonded_list.device[i].in_white_list)) {
                return false;
            }

#ifndef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
            if ((APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.curr_group) &&
                (g_lea_ucst_bonded_list.device[i].group_id != g_lea_ucst_ctrl.curr_group)) {
                return false;
            }
#endif
            LE_AUDIO_MSGLOG_I("[APP] is_bonded_device, YES!", 0);
            return true;
        }
    }

    return false;
}

static bt_hci_disconnect_reason_t app_le_audio_ucst_get_disconnect_reason(const bt_addr_t *addr)
{
    uint8_t i;
    bt_device_manager_le_bonded_info_t * p_bonded_info = NULL;

    if (0 == g_lea_ucst_bonded_list.num) {
        return BT_HCI_STATUS_SUCCESS;
    }
    p_bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)&addr->addr);


    for (i = 0; i < g_lea_ucst_bonded_list.num; i++) {
        if ((0 != g_lea_ucst_bonded_list.device[i].group_size) &&
            ((0 == memcmp(&(g_lea_ucst_bonded_list.device[i].addr), addr, sizeof(bt_addr_t))) ||
            ((NULL != p_bonded_info) && (0 == memcmp(&(g_lea_ucst_bonded_list.device[i].addr), &p_bonded_info->bt_addr, sizeof(bt_addr_t)))))){

            LE_AUDIO_MSGLOG_I("[APP] disconnect_reason:0x%x!", 1, g_lea_ucst_bonded_list.device[i].reason);
            return g_lea_ucst_bonded_list.device[i].reason;
        }
    }

    return BT_HCI_STATUS_SUCCESS;
}

static bool app_le_audio_ucst_check_bond_and_not_in_white_list(const bt_addr_t *addr)
{
    uint8_t i;
    bt_device_manager_le_bonded_info_t * p_bonded_info = NULL;

    if (0 == g_lea_ucst_bonded_list.num) {
        return false;
    }
    p_bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)&addr->addr);

    for (i = 0; i < g_lea_ucst_bonded_list.num; i++) {
        if (((0 == memcmp(&(g_lea_ucst_bonded_list.device[i].addr), addr, sizeof(bt_addr_t))) ||
            ((NULL != p_bonded_info) && (0 == memcmp(&(g_lea_ucst_bonded_list.device[i].addr), &p_bonded_info->bt_addr, sizeof(bt_addr_t))))) &&
            (0 != g_lea_ucst_bonded_list.device[i].group_size)) {
            if (g_lea_ucst_bonded_list.device[i].in_white_list) {
                break;
            } else {
                /*LE_AUDIO_MSGLOG_I("[APP][U] is_bonded_and_not_in_white_list, i:%x group:%x(size:%x) in_white_list:%x", 4, i,
                                  g_lea_ucst_bonded_list.device[i].group_id,
                                  g_lea_ucst_bonded_list.device[i].group_size,
                                  g_lea_ucst_bonded_list.device[i].in_white_list);*/
                return true;
            }
        }
    }

    return false;
}

#if 0//def AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
static bool app_le_audio_ucst_is_bonded_device_all_connected(void)
{
    uint8_t i, count = 0;

    if (0 == g_lea_ucst_bonded_list.num) {
        return true;
    }

    for (i = 0; i < g_lea_ucst_bonded_list.num; i++) {
        if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID != g_lea_ucst_bonded_list.device[i].link_idx) {
            count++;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][U] is_bonded_device_all_connected, bond_num:%x count:%x", 2, g_lea_ucst_bonded_list.num, count);

    if (g_lea_ucst_bonded_list.num == count) {
        return true;
    }

    return false;
}
#endif

static void app_le_audio_ucst_read_active_group_from_nvkey(void)
{
    nvkey_status_t ret = NVKEY_STATUS_OK;
    uint32_t size = APP_LE_AUDIO_NVKEY_ACTIVE_GROUP_LEN;

    /* read NVID_BT_LEA_ACTIVE_GROUP: 1 byte */
    /* NVID_BT_LEA_ACTIVE_GROUP: 1 = | active_group (1 byte) | */
    ret = nvkey_read_data(NVID_BT_LEA_ACTIVE_GROUP, &g_lea_ucst_active_group, &size);
    LE_AUDIO_MSGLOG_I("[APP][U] read_active_group(nvkey), ret:%x size:%d active_group:%x", 3, ret, size, g_lea_ucst_active_group);

    if (NVKEY_STATUS_OK == ret) {
        /* update active group */
        if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID != g_lea_ucst_active_group) {
            g_lea_ucst_ctrl.curr_group = g_lea_ucst_active_group;
        }
    }
}

static void app_le_audio_ucst_write_active_group_to_nvkey(void)
{
    nvkey_status_t ret = NVKEY_STATUS_ERROR;
    uint32_t size = APP_LE_AUDIO_NVKEY_ACTIVE_GROUP_LEN;

    /* write NVID_BT_LEA_ACTIVE_GROUP: 1 byte */
    /* NVID_BT_LEA_ACTIVE_GROUP: 1 = | active_group (1 byte) | */
    ret = nvkey_write_data(NVID_BT_LEA_ACTIVE_GROUP, &g_lea_ucst_active_group, size);
    LE_AUDIO_MSGLOG_I("[APP][U] write_active_group(nvkey), ret:%x active_group:%x", 2, ret, g_lea_ucst_active_group);
}

static void app_le_audio_ucst_add_link_to_group(uint8_t group_id, uint8_t link_idx)
{
    uint8_t i;

    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= group_id) {
        return;
    }

    for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM; i++) {
        if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == g_lea_ucst_group_info[group_id].link_idx[i]) {
            g_lea_ucst_group_info[group_id].link_idx[i] = link_idx;
            break;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][U] add_link_to_group, group:%x(size:%x) link_idx:(%x %x) bond_num:%x", 5,
                      group_id,
                      g_lea_ucst_group_info[group_id].size,
                      g_lea_ucst_group_info[group_id].link_idx[0],
                      g_lea_ucst_group_info[group_id].link_idx[1],
                      g_lea_ucst_group_info[group_id].bond_num);
}


static void app_le_audio_ucst_remove_link_from_group(uint8_t group_id, uint8_t link_idx)
{
    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= group_id) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] rm_link_from_group, group:%x(size:%x) link_idx:(%x %x)", 4,
                      group_id,
                      g_lea_ucst_group_info[group_id].size,
                      g_lea_ucst_group_info[group_id].link_idx[0],
                      g_lea_ucst_group_info[group_id].link_idx[1]);

    uint8_t i;

    for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM; i++) {
        if (link_idx == g_lea_ucst_group_info[group_id].link_idx[i]) {
            g_lea_ucst_group_info[group_id].link_idx[i] = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
            break;
        }
    }
}


static void app_le_audio_ucst_delete_oldest_group_from_bonded_list(void)
{
    app_le_audio_ucst_bonded_device_t *p_device = NULL;
    uint8_t i;

    /* free oldest bonded group with no device connected, and also remove device from white list */
    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {

        p_device = &g_lea_ucst_bonded_list.device[i];

        LE_AUDIO_MSGLOG_I("[APP][U] del_oldest_group, [%x] link_idx:%x white_list:%x group:%x(size:%x)", 5, i,
                          p_device->link_idx,
                          p_device->in_white_list,
                          p_device->group_id,
                          p_device->group_size);

        if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= p_device->group_id) {
            LE_AUDIO_MSGLOG_I("[APP] del_oldest_group, [%x] invalid group:%x", 2, i, p_device->group_id);
            continue;
        }

        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM > p_device->link_idx) {
            continue;
        }

        if (0 != app_le_audio_ucst_get_group_link_num(p_device->group_id)) {
            /* any device in the group is connected, don't remove */
            continue;
        }

        bt_device_manager_le_remove_bonded_device(&(p_device->addr));

        if (1 < p_device->group_size) {
            app_le_audio_ucst_bonded_device_t *p_device_mate = NULL;
            uint8_t j = 0;

            /* search group mate */
            for (j = 0; j < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; j++) {
                if (i == j) {
                    continue;
                }

                p_device_mate = &g_lea_ucst_bonded_list.device[j];

                if (p_device->group_id != p_device_mate->group_id) {
                    continue;
                }

                LE_AUDIO_MSGLOG_I("[APP][U] del_oldest_group mate, [%x] link_idx:%x white_list:%x group:%x(size:%x)", 5, j,
                                  p_device_mate->link_idx,
                                  p_device_mate->in_white_list,
                                  p_device_mate->group_id,
                                  p_device_mate->group_size);

                p_device_mate->deleting = true;
                p_device_mate->group_id |= APP_LE_AUDIO_UCST_BONDED_RECORD_INVALID;

                bt_device_manager_le_remove_bonded_device(&(p_device_mate->addr));
            }
        }

        bt_addr_t addr = {0};
        bool rm_white_list = false;

        /* reset group info */
        app_le_audio_ucst_reset_group_info(p_device->group_id);

        if (p_device->in_white_list) {
            p_device->in_white_list = false;
            rm_white_list = true;
            memcpy((uint8_t *)&addr, (uint8_t *)&(p_device->addr), sizeof(bt_addr_t));
        }

        /* handle bonded list */
        LE_AUDIO_MSGLOG_I("[APP] del_oldest_group, i:%x", 1, i);
        app_le_audio_ucst_reset_bonded_list(i);
        app_le_audio_ucst_refresh_bonded_list(i);
        g_lea_ucst_bonded_list.num--;

        app_le_audio_ucst_write_bonded_list_to_nvkey();

        if (rm_white_list) {
            /* rm from white list */
            LE_AUDIO_MSGLOG_I("[APP] del_oldest_group, set_white_list.state:%x", 1, g_lea_ucst_set_white_list.state);
            g_lea_ucst_set_white_list.state = APP_LE_AUDIO_UCST_SET_WHITE_LIST_REMOVE_ON_GOING;
            app_le_audio_ucst_remove_white_list(&addr);
        }

        LE_AUDIO_MSGLOG_I("[APP] del_oldest_group, set_white_list.state:%x disconn:%x", 2,
                          g_lea_ucst_set_white_list.state,
                          g_lea_ucst_waiting_disconn_flag);

        if ((1 == p_device->group_size) ||
            (APP_LE_AUDIO_UCST_SET_WHITE_LIST_REMOVE_ON_GOING == g_lea_ucst_set_white_list.state) ||
            (true == g_lea_ucst_waiting_disconn_flag)) {
            return;
        }

        app_le_audio_ucst_check_delete_group_device();
    }
}

void app_le_audio_ucst_delete_group_from_bonded_list(bt_addr_t *addr)
{
    app_le_audio_ucst_bonded_device_t *p_device = NULL;
    uint8_t i = 0;

    /* search device */
    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
        p_device = &g_lea_ucst_bonded_list.device[i];
        if (0 == memcmp(&(p_device->addr), addr, sizeof(bt_addr_t))) {
            break;
        }
    }

    if (APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM == i) {
        LE_AUDIO_MSGLOG_I("[APP][U] del_group, device not found", 0);
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] del_group, [%x] link_idx:%x white_list:%x group:%x(size:%x)", 5, i,
                      p_device->link_idx,
                      p_device->in_white_list,
                      p_device->group_id,
                      p_device->group_size);

    bt_device_manager_le_remove_bonded_device(&(p_device->addr));

    if (1 < p_device->group_size) {
        app_le_audio_ucst_bonded_device_t *p_device_mate = NULL;
        uint8_t j = 0;

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        for (j = 0; j < APP_LE_AUDIO_NVKEY_SIRK_INFO_MAX_NUM; j ++) {
            if (p_device->group_id == g_lea_ucst_sirk_info[j].group_id) {
                g_lea_ucst_sirk_info[j].group_id = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
                memset((uint8_t *)&(g_lea_ucst_sirk_info[j].sirk), 0, sizeof(bt_key_t));
                app_le_audio_ucst_write_sirk_list_to_nvkey();
                break;
            }
        }
#endif
        if (APP_LE_AUDIO_UCST_CONN_COORDINATED_SET_BY_SIRK <= g_lea_ucst_ctrl.curr_conn) {
            app_le_audio_ucst_group_info_t *p_group = NULL;
            if (NULL != (p_group = app_le_audio_ucst_get_group_info(p_device->group_id))) {
                if (0 == memcmp((bt_key_t *)&g_le_audio_sirk, (bt_key_t *)&(p_group->sirk), sizeof(bt_key_t))) {
                    app_le_audio_ucst_stop_scan_all();
                }
            }
        }

        /* search group mate */
        for (j = 0; j < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; j++) {
            if (i == j) {
                continue;
            }

            p_device_mate = &g_lea_ucst_bonded_list.device[j];

            if (p_device->group_id != p_device_mate->group_id) {
                continue;
            }

            LE_AUDIO_MSGLOG_I("[APP][U] del_group mate, [%x] link_idx:%x white_list:%x group:%x(size:%x)", 5, j,
                              p_device_mate->link_idx,
                              p_device_mate->in_white_list,
                              p_device_mate->group_id,
                              p_device_mate->group_size);

            p_device_mate->deleting = true;
            p_device_mate->group_id |= APP_LE_AUDIO_UCST_BONDED_RECORD_INVALID;

            bt_device_manager_le_remove_bonded_device(&(p_device_mate->addr));
        }
    }

    uint8_t link_idx;
    bool rm_white_list = false;

    link_idx = p_device->link_idx;

    /* reset group info */
    app_le_audio_ucst_reset_group_info(p_device->group_id);

    if (p_device->in_white_list) {
        p_device->in_white_list = false;
        rm_white_list = true;
    }

    /* handle bonded list */
    LE_AUDIO_MSGLOG_I("[APP] del_group, i:%x", 1, i);
    app_le_audio_ucst_reset_bonded_list(i);
    app_le_audio_ucst_refresh_bonded_list(i);
    g_lea_ucst_bonded_list.num--;

    app_le_audio_ucst_write_bonded_list_to_nvkey();

    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM > link_idx) {
        app_le_audio_ucst_link_info_t *p_info = NULL;

        p_info = &g_lea_ucst_link_info[link_idx];
        if (APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL != p_info->next_state) {
            p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL;

            if (BT_STATUS_SUCCESS == app_le_audio_ucst_disconnect(p_info->handle)) {
                g_lea_ucst_waiting_disconn_flag = true;
            } else {
                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
            }
        }
    }

    if (rm_white_list) {
        /* rm from white list */
        LE_AUDIO_MSGLOG_I("[APP] del_group, set_white_list.state:%x", 1, g_lea_ucst_set_white_list.state);
        g_lea_ucst_set_white_list.state = APP_LE_AUDIO_UCST_SET_WHITE_LIST_REMOVE_ON_GOING;
        app_le_audio_ucst_remove_white_list(addr);
    }

    LE_AUDIO_MSGLOG_I("[APP] del_group, set_white_list.state:%x disconn:%x", 2,
                      g_lea_ucst_set_white_list.state,
                      g_lea_ucst_waiting_disconn_flag);

    if ((1 == p_device->group_size) ||
        (APP_LE_AUDIO_UCST_SET_WHITE_LIST_REMOVE_ON_GOING == g_lea_ucst_set_white_list.state) ||
        (true == g_lea_ucst_waiting_disconn_flag)) {
        return;
    }

    app_le_audio_ucst_check_delete_group_device();
}

void app_le_audio_ucst_delete_device(bt_addr_t *addr)
{
    app_le_audio_ucst_delete_group_from_bonded_list(addr);
}

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
void app_le_audio_ucst_set_active_group(uint8_t group)
{
    app_le_audio_ucst_group_info_t *p_group_info = NULL;

    if (NULL == (p_group_info = app_le_audio_ucst_get_group_info(group))) {
        LE_AUDIO_MSGLOG_I("[APP][U] set_active_group, invalid group:%x", 1, group);
        return;
    }

    if (0 == p_group_info->size) {
        LE_AUDIO_MSGLOG_I("[APP][U] set_active_group, not exist group:%x", 1, group);
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] set_active_group, stream_state:%x->%x group:%x active:%x->%x latest:%x", 6,
                      g_lea_ucst_ctrl.curr_stream_state,
                      g_lea_ucst_ctrl.next_stream_state,
                      group,
                      g_lea_ucst_ctrl.curr_group,
                      g_lea_ucst_ctrl.next_group,
                      g_lea_ucst_ctrl.latest_group);

    if (((group == g_lea_ucst_ctrl.curr_group) && (APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_ctrl.next_group)) ||
        (group == g_lea_ucst_ctrl.next_group)) {
        return;
    }


    if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_ctrl.curr_group) {
        g_lea_ucst_ctrl.curr_group = group;
        g_lea_ucst_active_group = group;
        app_le_audio_ucst_write_active_group_to_nvkey();
        if (!app_le_audio_ucst_is_group_device_all_connected(group)) {
            app_le_audio_ucst_connect_group_device(group);
        }
        ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                            APP_LE_AUDIO_EVENT_ACTIVE_DEVICE_CHANGED, NULL, 0,
                            NULL, 0);
        ble_tbs_switch_device_completed();
        app_le_audio_ucst_set_mic_channel();
        app_le_audio_ucst_start();
        return;
    }

    if ((group != g_lea_ucst_ctrl.curr_group) ||
        (group != g_lea_ucst_ctrl.next_group)) {
        g_lea_ucst_active_group = group;
        app_le_audio_ucst_write_active_group_to_nvkey();

        if (!app_le_audio_ucst_is_group_device_all_connected(group)) {
            app_le_audio_ucst_connect_group_device(group);
        }
        if ((APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.curr_stream_state) &&
            (APP_LE_AUDIO_UCST_STREAM_STATE_IDLE == g_lea_ucst_ctrl.next_stream_state)) {
            g_lea_ucst_ctrl.curr_group = group;
            g_lea_ucst_ctrl.next_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                APP_LE_AUDIO_EVENT_ACTIVE_DEVICE_CHANGED, NULL, 0,
                                NULL, 0);
            ble_tbs_switch_device_completed();
            app_le_audio_ucst_start();
        } else {
            g_lea_ucst_ctrl.next_group = group;
            app_le_audio_ucst_stop(true);
        }
    }
}
#endif

bool app_le_audio_ucst_is_active_group(uint8_t group_id)
{
    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.next_group) {
        return false;
    }

    if ((APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.curr_group) &&
        (group_id == g_lea_ucst_ctrl.curr_group)) {
        return true;
    }
    return false;
}

bool app_le_audio_ucst_is_active_group_by_handle(bt_handle_t handle)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;

    if (NULL == (p_info = app_le_audio_ucst_get_link_info(handle))) {
        return false;
    }

    return app_le_audio_ucst_is_active_group(p_info->group_id);
}

bool app_le_audio_ucst_is_group_device_all_connected(uint8_t group_id)
{
    uint8_t i, num = 0;

    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= group_id) {
        LE_AUDIO_MSGLOG_I("[APP][U] is_group_device_all_connected, ERROR! group:%x", 1, group_id);
        return true;
    }

    if (0 == g_lea_ucst_group_info[group_id].size) {
        LE_AUDIO_MSGLOG_I("[APP][U] is_group_device_all_connected, ERROR! group:%x size:%x", 2,
                          group_id,
                          g_lea_ucst_group_info[group_id].size);
        return true;
    }

    for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM; i++) {
        if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID != g_lea_ucst_group_info[group_id].link_idx[i]) {
            num++;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][U] is_group_device_all_connected, group:%x size:%x connected:%x", 3,
                      group_id,
                      g_lea_ucst_group_info[group_id].size,
                      num);

    if (g_lea_ucst_group_info[group_id].size == num) {
        return true;
    }

    return false;
}

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
static bool app_le_audio_ucst_is_group_full(void)
{
    uint8_t i, count = 0;

    for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_ID_MAX; i++) {
#if 0
        LE_AUDIO_MSGLOG_I("[APP][U] is_group_full, group:%x size:%x bond_num:%x", 3,
                          i,
                          g_lea_ucst_group_info[i].size,
                          g_lea_ucst_group_info[i].bond_num);
#endif
        if ((0 != g_lea_ucst_group_info[i].size) &&
            (g_lea_ucst_group_info[i].size == g_lea_ucst_group_info[i].bond_num)) {
            count++;
        }
    }

    if (count == APP_LE_AUDIO_UCST_GROUP_ID_MAX) {
        return true;
    }

    return false;
}
#endif
app_le_audio_ucst_group_info_t *app_le_audio_ucst_get_group_info(uint8_t group_id)
{
    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= group_id) {
        return NULL;
    }

    return &g_lea_ucst_group_info[group_id];
}

static uint8_t app_le_audio_ucst_get_group_id_by_sirk(bt_key_t *sirk)
{
    uint8_t i;

    i = APP_LE_AUDIO_UCST_GROUP_ID_MAX;
    while (i > 0) {
        i--;
        if ((0 != g_lea_ucst_group_info[i].size) &&
            (0 == memcmp((uint8_t *) & (g_lea_ucst_group_info[i].sirk), sirk, sizeof(bt_key_t)))) {
            return i;
        }
    }

    return APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
}



static uint8_t app_le_audio_ucst_get_group_link_num(uint8_t group_id)
{
    uint8_t i, link_num = 0;

    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= group_id) {
        return 0;
    }

    if (0 == g_lea_ucst_group_info[group_id].size) {
        return 0;
    }

    for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM; i++) {
        if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID != g_lea_ucst_group_info[group_id].link_idx[i]) {
            link_num++;
        }
    }

    LE_AUDIO_MSGLOG_I("[APP][U] get_group_link_num, group:%x size:%x link_num:%x", 3,
                      group_id,
                      g_lea_ucst_group_info[group_id].size,
                      link_num);

    return link_num;
}

uint8_t app_le_audio_ucst_get_group_link_num_ex(uint8_t group_id)
{
    uint8_t link_idx, i, num = 0;

    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= group_id) {
        return 0;
    }

    if (0 == g_lea_ucst_group_info[group_id].size) {
        return 0;
    }

    for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM; i++) {
        if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID != (link_idx = g_lea_ucst_group_info[group_id].link_idx[i])) {
            if (APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL != g_lea_ucst_link_info[link_idx].next_state) {
                num++;
            }
        }
    }

    return num;
}

uint8_t app_le_audio_ucst_get_active_group(void)
{
    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX > g_lea_ucst_ctrl.next_group) {
        return g_lea_ucst_ctrl.next_group;
    }

    return g_lea_ucst_ctrl.curr_group;
}

uint8_t app_le_audio_ucst_get_active_group_address(app_le_audio_ucst_active_group_addr_list_t *addr_list)
{
    app_le_audio_ucst_group_info_t *p_group = NULL;
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t group_id, num = 0;

    if (NULL == addr_list) {
        return num;
    }

    group_id = app_le_audio_ucst_get_active_group();

    if (NULL == (p_group = app_le_audio_ucst_get_group_info(group_id))) {
        return num;
    }


    if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID != p_group->link_idx[0]) {
        if (NULL != (p_info = app_le_audio_ucst_get_link_info_by_idx(p_group->link_idx[0]))) {
            LE_AUDIO_MSGLOG_I("[APP][U] get_active_group_addr num:%x link_idx[0]:%x handle:%x", 3, num, p_group->link_idx[0], p_info->handle);
            memcpy((uint8_t *)&(addr_list->addr[num]), (uint8_t *)&(p_info->addr), sizeof(bt_addr_t));
            num++;
        }
    }

    if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID != p_group->link_idx[1]) {
        if (NULL != (p_info = app_le_audio_ucst_get_link_info_by_idx(p_group->link_idx[1]))) {
            LE_AUDIO_MSGLOG_I("[APP][U] get_active_group_addr num:%x link_idx[1]:%x handle:%x", 3, num, p_group->link_idx[1], p_info->handle);
            memcpy((uint8_t *)&(addr_list->addr[num]), (uint8_t *)&(p_info->addr), sizeof(bt_addr_t));
            num++;
        }
    }
    addr_list->num = num;

    return num;
}

static void app_le_audio_ucst_reset_active_group(void)
{
    g_lea_ucst_active_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    g_lea_ucst_ctrl.curr_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    g_lea_ucst_ctrl.next_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
    g_lea_ucst_ctrl.latest_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
}

static void app_le_audio_ucst_reset_group_info(uint8_t group_id)
{
    uint8_t i;

    if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= group_id) {
        return;
    }

    memset(&g_lea_ucst_group_info[group_id], 0, sizeof(app_le_audio_ucst_group_info_t));

    for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM; i++) {
        g_lea_ucst_group_info[group_id].link_idx[i] = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
    }

    if (g_lea_ucst_ctrl.curr_group == group_id) {
        if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_ctrl.next_group) {
            g_lea_ucst_ctrl.curr_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
        }
    }

    if (g_lea_ucst_ctrl.latest_group == group_id) {
        g_lea_ucst_ctrl.latest_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
        for (i = 0; i < APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM; i++) {
            if ((i != group_id) && (0 != g_lea_ucst_group_info[i].size)) {
                g_lea_ucst_ctrl.latest_group = i;
            }
        }
    }

    if (g_lea_ucst_active_group == group_id) {
        g_lea_ucst_active_group = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
        app_le_audio_ucst_write_active_group_to_nvkey();
    }
}

static void app_le_audio_ucst_reset_all_group_info(void)
{
    uint8_t i;

    i = APP_LE_AUDIO_UCST_GROUP_ID_MAX;
    while (i > 0) {
        i--;
        app_le_audio_ucst_reset_group_info(i);
    }
}

static void app_le_audio_ucst_refresh_group_info(void)
{
    uint8_t i, group_id;

    if (0 == g_lea_ucst_bonded_list.num) {
        return;
    }

    for (i = 0; i < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; i++) {
        if ((APP_LE_AUDIO_UCST_GROUP_ID_MAX > (group_id = g_lea_ucst_bonded_list.device[i].group_id)) &&
            (0 != g_lea_ucst_bonded_list.device[i].group_size)) {
            g_lea_ucst_group_info[group_id].size = g_lea_ucst_bonded_list.device[i].group_size;



            g_lea_ucst_group_info[group_id].bond_num++;
        }
    }

#if 1   /* TEST */
    for (group_id = 0; group_id < APP_LE_AUDIO_UCST_GROUP_ID_MAX; group_id++) {
        LE_AUDIO_MSGLOG_I("[APP] refresh_group_info, group:%x(size:%x) link_idx:(%x %x) bond_num:%x", 5,
                          group_id,
                          g_lea_ucst_group_info[group_id].size,
                          g_lea_ucst_group_info[group_id].link_idx[0],
                          g_lea_ucst_group_info[group_id].link_idx[1],
                          g_lea_ucst_group_info[group_id].bond_num);
    }
#endif
}

static bool app_le_audio_ucst_check_delete_group_device(void)
{
    app_le_audio_ucst_bonded_device_t *p_device = NULL;
    app_le_audio_ucst_link_info_t *p_info = NULL;
    bt_addr_t addr = {0};
    uint8_t j, link_idx;
    bool rm_white_list = false;

    for (j = 0; j < APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM; j++) {
        p_device = &g_lea_ucst_bonded_list.device[j];

        if (!p_device->deleting) {
            continue;
        }

        link_idx = p_device->link_idx;

        if (p_device->in_white_list) {
            p_device->in_white_list = false;
            rm_white_list = true;
            memcpy((uint8_t *)&addr, (uint8_t *)&(p_device->addr), sizeof(bt_addr_t));
        }

        /* handle bonded list */
        app_le_audio_ucst_reset_bonded_list(j);
        app_le_audio_ucst_refresh_bonded_list(j);
        g_lea_ucst_bonded_list.num--;

        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM > link_idx) {
            p_info = &g_lea_ucst_link_info[link_idx];
            if (APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL != p_info->next_state) {
                p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL;
                if (BT_STATUS_SUCCESS == app_le_audio_ucst_disconnect(p_info->handle)) {
                    g_lea_ucst_waiting_disconn_flag = true;
                } else {
                    p_info->next_state = APP_LE_AUDIO_UCST_LINK_STATE_IDLE;
                }
            }
        }

        if (rm_white_list) {
            /* rm from white list */
            LE_AUDIO_MSGLOG_I("[APP] check_del_group, set_white_list.state:%x", 1, g_lea_ucst_set_white_list.state);
            g_lea_ucst_set_white_list.state = APP_LE_AUDIO_UCST_SET_WHITE_LIST_REMOVE_ON_GOING;
            app_le_audio_ucst_remove_white_list(&addr);
        }

        if ((APP_LE_AUDIO_UCST_SET_WHITE_LIST_REMOVE_ON_GOING == g_lea_ucst_set_white_list.state) ||
            (true == g_lea_ucst_waiting_disconn_flag)) {
            //app_le_audio_ucst_write_bonded_list_to_nvkey();
            return true;
        }
    }

    return false;
}

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
static void app_le_audio_ucst_connect_group_device(uint8_t group_id)
{
    g_lea_ucst_waiting_conn_group = group_id;
    if (g_lea_ucst_group_info[group_id].bond_num == g_lea_ucst_group_info[group_id].size) {
        uint8_t i;
        bool in_white_list = false, scan_cs = false, non_connected = false;
        for (i = 0; i < g_lea_ucst_bonded_list.num; i++) {
#if 0   /* for test */
            LE_AUDIO_MSGLOG_I("[APP] connect_group_device, [%x] group:%x link_idx:%x in_white_list:%x", 4,
                              i,
                              g_lea_ucst_bonded_list.device[i].group_id,
                              g_lea_ucst_bonded_list.device[i].link_idx,
                              g_lea_ucst_bonded_list.device[i].in_white_list);
#endif

            if (group_id == g_lea_ucst_bonded_list.device[i].group_id) {
                if (g_lea_ucst_bonded_list.device[i].in_white_list) {
                    in_white_list = true;
                } else {
                    scan_cs = true;
                }
                if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == g_lea_ucst_bonded_list.device[i].link_idx) {
                    non_connected = true;
                }
            }
        }

        if (in_white_list && non_connected) {
            app_le_audio_ucst_connect_bonded_device(scan_cs);
        }

    } else {
        uint8_t i = 0;
        for (i = 0; i < APP_LE_AUDIO_NVKEY_SIRK_INFO_MAX_NUM; i++) {
            if (g_lea_ucst_sirk_info[i].group_id == group_id) {
                app_le_audio_ucst_set_sirk(&(g_lea_ucst_sirk_info[i].sirk), false);
                app_le_audio_ucst_connect_coordinated_set(false);
                break;
            }
        }
    }
}
#endif

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
static void app_le_audio_ucst_disconnect_other_group_device(uint8_t keep_group_id, uint8_t target_device_location)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    app_le_audio_ucst_link_state_t next_state;
    bt_status_t ret;
    uint8_t i;

    for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
        p_info = &g_lea_ucst_link_info[i];

        LE_AUDIO_MSGLOG_I("[APP] disconnect_other_group_device, handle:%x group:%x(%x) sink_location:%x(%x)", 5,
                          p_info->handle,
                          p_info->group_id,
                          keep_group_id,
                          p_info->sink_location,
                          target_device_location);

        if ((BT_HANDLE_INVALID != p_info->handle) &&
            (APP_LE_AUDIO_UCST_GROUP_ID_MAX > p_info->group_id) &&
            (keep_group_id != p_info->group_id) &&
            ((target_device_location == p_info->sink_location) || (AUDIO_LOCATION_NONE == p_info->sink_location))) {
            /* if p_info->sink_location == AUDIO_LOCATION_NONE, service discovery not complete */
            next_state = g_lea_ucst_link_info[i].next_state;
            g_lea_ucst_link_info[i].next_state = APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL;
            if (BT_STATUS_SUCCESS == (ret = app_le_audio_ucst_disconnect(p_info->handle))) {
                g_lea_ucst_waiting_conn_flag = true;
            } else {
                g_lea_ucst_waiting_conn_flag = false;
                g_lea_ucst_link_info[i].next_state = next_state;
            }

            LE_AUDIO_MSGLOG_I("[APP] disconnect_other_group_device, ret:%x handle:%x group:%x(%x) sink_location:%x(%x)", 6,
                              ret,
                              p_info->handle,
                              p_info->group_id,
                              keep_group_id,
                              p_info->sink_location,
                              target_device_location);
            break;
        }
    }
}
#endif

static void app_le_audio_ucst_handle_csip_discover_cs_cnf(ble_csip_discover_coordinated_set_cnf_t *cnf)
{
    char conn_string[60] = {0};
    uint8_t link_num;

    if (app_le_audio_ucst_is_connected_device(&cnf->address)) {
        return;
    }

    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == (link_num = app_le_audio_ucst_get_link_num_ex())) {
        LE_AUDIO_MSGLOG_I("[APP][CSIP] DISCOVER_COORDINATED_SET_CNF, link full", 0);
        g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
        app_le_audio_ucst_stop_scan_all();
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][CSIP] DISCOVER_COORDINATED_SET_CNF, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      cnf->address.type,
                      cnf->address.addr[5],
                      cnf->address.addr[4],
                      cnf->address.addr[3],
                      cnf->address.addr[2],
                      cnf->address.addr[1],
                      cnf->address.addr[0]);

    snprintf((char *)conn_string, 60, "Coordinated set found! type:%d, %02x:%02x:%02x:%02x:%02x:%02x\r\n",
             cnf->address.type,
             cnf->address.addr[5],
             cnf->address.addr[4],
             cnf->address.addr[3],
             cnf->address.addr[2],
             cnf->address.addr[1],
             cnf->address.addr[0]);
    bt_app_common_at_cmd_print_report(conn_string);

    LE_AUDIO_MSGLOG_I("[APP] DISCOVER_COORDINATED_SET_CNF, curr_conn:%x link_num:%x", 2, g_lea_ucst_ctrl.curr_conn, link_num);

    switch (g_lea_ucst_ctrl.curr_conn) {
        case APP_LE_AUDIO_UCST_CONN_COORDINATED_SET_BY_SIRK: {
            app_le_audio_ucst_connect_device(&cnf->address);
            break;
        }
        default:
            break;
    }
}

static void app_le_audio_ucst_handle_csip_read_cs_size_cnf(ble_csip_read_set_size_cnf_t *cnf)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    char conn_string[60] = {0};
    uint8_t link_idx;
    uint8_t bond_idx = APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID;

    if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == (link_idx = app_le_audio_ucst_get_link_idx(cnf->handle))) {
        return;
    }

    snprintf((char *)conn_string, 60, "Coordinated set size[%d], handle:%x\r\n", cnf->size, cnf->handle);
    bt_app_common_at_cmd_print_report(conn_string);


    /* link_info */
    p_info = &g_lea_ucst_link_info[link_idx];
    p_info->group_size = cnf->size;

    LE_AUDIO_MSGLOG_I("[APP][U] READ_CS_SIZE_CNF, handle:%x size:%x bond_idx:%x group_id:%x", 4, p_info->handle, p_info->group_size, p_info->bond_idx, p_info->group_id);

    if (APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID == p_info->bond_idx) {
        /* new device */


        if (g_lea_ucst_pts_set_size) {
            p_info->group_size = g_lea_ucst_pts_set_size; /* some PTS cases which need 2 or more PTS dongles may have no CSIS */
        }

        /* group_info */
        if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= p_info->group_id) {
            return;
        }

        g_lea_ucst_group_info[p_info->group_id].size = p_info->group_size;

        if (APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID != (bond_idx = app_le_audio_ucst_get_empty_bonded_list_idx())) {
            /* link_info */
            p_info->bond_idx = bond_idx;

            /* bonded_list */
            g_lea_ucst_bonded_list.num++;
            memcpy(&(g_lea_ucst_bonded_list.device[bond_idx].addr), &(p_info->addr), sizeof(bt_addr_t));
            g_lea_ucst_bonded_list.device[bond_idx].group_id = p_info->group_id;
            g_lea_ucst_bonded_list.device[bond_idx].group_size = p_info->group_size;
            g_lea_ucst_bonded_list.device[bond_idx].link_idx = link_idx;
            g_lea_ucst_bonded_list.device[bond_idx].in_white_list = true;
            app_le_audio_ucst_write_bonded_list_to_nvkey();
        }
        app_le_audio_ucst_check_connect_coordinated_set(p_info);
    } else {
        /* Bonded device */
        if (g_lea_ucst_pts_set_size) {
            p_info->group_size = g_lea_ucst_pts_set_size; /* some PTS cases which need 2 or more PTS dongles may have no CSIS */
        }

        /* check group size changed */
        if (APP_LE_AUDIO_UCST_GROUP_ID_MAX > p_info->group_id) {
            if (g_lea_ucst_group_info[p_info->group_id].size != p_info->group_size) {
                g_lea_ucst_group_info[p_info->group_id].size = p_info->group_size;
                if (APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM > p_info->bond_idx) {
                    g_lea_ucst_bonded_list.device[p_info->bond_idx].group_size = p_info->group_size;
                    app_le_audio_ucst_write_bonded_list_to_nvkey();
                }
                if (p_info->group_size > 1) {
                    app_le_audio_ucst_check_connect_coordinated_set(p_info);
                }
            }
        }
    }

    if (g_lea_ucst_callback) {
        app_dongle_le_race_connect_ind_t evt;

        evt.ret = BT_STATUS_SUCCESS;
        memcpy(&(evt.peer_addr), &(p_info->addr), sizeof(bt_addr_t));
        evt.group_id = p_info->group_id;
        evt.group_size = p_info->group_size;
        g_lea_ucst_callback(APP_DONGLE_LE_RACE_EVT_CONNECT_IND, APP_DONGLE_LE_RACE_SINK_DEVICE_LEA, &evt);
    }
}


static void app_le_audio_ucst_handle_csip_sirk_cnf(ble_csip_read_sirk_cnf_t *cnf)
{
    uint8_t link_idx = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;

    if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == (link_idx = (app_le_audio_ucst_get_link_idx(cnf->handle)))) {
        return;
    }

    //LE_AUDIO_MSGLOG_I("[APP][CSIP] READ_SIRK_CNF, handle:%x", 1, cnf->handle);

    if (0 != memcmp((uint8_t *)&g_le_audio_sirk, &(cnf->sirk), sizeof(bt_key_t))) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        /* for scan cs */
        app_le_audio_ucst_set_sirk(&(cnf->sirk), false);
#else
        app_le_audio_ucst_set_sirk(&(cnf->sirk), true);
#endif
    }

    app_le_audio_ucst_link_info_t *p_info = NULL;

    p_info = &g_lea_ucst_link_info[link_idx];

    /* check group_info */
    if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID != p_info->group_id) {
        /* bonded device */

        memcpy((uint8_t *) & (g_lea_ucst_group_info[p_info->group_id].sirk), &(cnf->sirk), sizeof(bt_key_t));

    } else {
        /* new device */

        uint8_t group_id = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;

        if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID != (group_id = app_le_audio_ucst_get_group_id_by_sirk(&(cnf->sirk)))) {
            /* group exist */
            app_le_audio_ucst_add_link_to_group(group_id, link_idx);
            g_lea_ucst_group_info[group_id].bond_num++;

        } else {
            group_id = app_le_audio_ucst_set_new_group_info(&(cnf->sirk), link_idx);
        }

        p_info->group_id = group_id;
#ifndef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_ctrl.curr_group) {
            g_lea_ucst_ctrl.curr_group = group_id;
        }
#endif
        g_lea_ucst_ctrl.latest_group = group_id;

    }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    uint8_t i = 0;
    bool update_sirk = true;

    for (i = 0; i < APP_LE_AUDIO_NVKEY_SIRK_INFO_MAX_NUM; i++) {
        if (g_lea_ucst_sirk_info[i].group_id == p_info->group_id) {
            update_sirk = false;
            break;
        }
    }
    if (update_sirk) {
        for (i = 0; i < APP_LE_AUDIO_NVKEY_SIRK_INFO_MAX_NUM; i++) {
            if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_sirk_info[i].group_id) {
                g_lea_ucst_sirk_info[i].group_id = p_info->group_id;
                memcpy((uint8_t *)&(g_lea_ucst_sirk_info[i].sirk), &(cnf->sirk), sizeof(bt_key_t));
                app_le_audio_ucst_write_sirk_list_to_nvkey();
                break;
            }
        }
    }
#endif

    LE_AUDIO_MSGLOG_I("[APP][CSIP] READ_SIRK_CNF, handle:%x group:%x", 2,
                      p_info->handle,
                      p_info->group_id);

}


/**************************************************************************************************
* Public Functions
**************************************************************************************************/

void app_le_audio_ucst_register_callback(app_dongle_le_race_event_callback_t callback)
{
    g_lea_ucst_callback = callback;
}

bt_status_t app_le_audio_ucst_check_adv_data(bt_gap_le_ext_advertising_report_ind_t *ind)
{
    bt_bd_addr_t empty_addr = {0};
    uint16_t length = 0, index = 0;

    if (!(ind->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_CONNECTABLE)) {
        /* ignore non-connectable adv */
        //LE_AUDIO_MSGLOG_I("[APP][U] check_adv_data, ignore non-connectable adv", 0);
        return BT_STATUS_FAIL;
    }

    /* check address */
    if ((0 == memcmp(ind->address.addr, empty_addr, sizeof(bt_bd_addr_t))) ||
        ((ind->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_DIRECTED) &&
         (0 == memcmp(ind->direct_address.addr, empty_addr, sizeof(bt_bd_addr_t))))) {
        //LE_AUDIO_MSGLOG_I("[APP][U] check_adv_data, event_type:%x invalid addr!", 1, ind->event_type);
        return BT_STATUS_FAIL;
    }

    if (app_le_audio_ucst_is_connected_device(&ind->address)) {
        /* ignore connected device */
        //LE_AUDIO_MSGLOG_I("[APP][U] check_adv_data, ignore connected device", 0);
        return BT_STATUS_FAIL;
    }

    if (!app_lea_dongle_customer_check_adv_data(ind)) {
        /* Not found the target customer adv data in ADV */
        return BT_STATUS_FAIL;
    }

    if (ind->event_type & BT_GAP_LE_EXT_ADV_REPORT_EVT_MASK_DIRECTED) {
        if (!app_le_audio_ucst_is_bonded_device(&ind->address, false)) {
            /* ignore unknown device */
            //LE_AUDIO_MSGLOG_I("[APP][U] check_adv_data, ignore unknown device", 0);
            return BT_STATUS_FAIL;
        }

        return BT_STATUS_SUCCESS;
    }

    if (!app_le_audio_ucst_is_lea_adv(ind)) {
        return BT_STATUS_FAIL;
    }

    while (index < ind->data_length) {
        length = ind->data[index];

        if (0 == length) {
            break;
        }

        if ((length >= 7) && ((ind->data[index + 1] == 0xF0) || (ind->data[index + 1] == APP_LE_AUDIO_ADV_TYPE_RSI))) {
            if (BT_STATUS_SUCCESS != ble_csip_verify_rsi(&ind->data[index + 2])) {
                LE_AUDIO_MSGLOG_I("[APP] csip_verify_rsi fail 1", 0);
                return BT_STATUS_FAIL;
            }

            app_le_audio_ucst_print_lea_adv_addr(&ind->address);
            return BT_STATUS_SUCCESS;
        }

        if ((length >= 10) && ((ind->data[index + 1] == APP_LE_AUDIO_ADV_TYPE_MSD) && (ind->data[index + 2] == 0x94) && (ind->data[index + 5] == APP_LE_AUDIO_ADV_TYPE_RSI))) {
            if (BT_STATUS_SUCCESS != ble_csip_verify_rsi(&ind->data[index + 6])) {
                LE_AUDIO_MSGLOG_I("[APP] csip_verify_rsi fail 2", 0);
                return BT_STATUS_FAIL;
            }
            app_le_audio_ucst_print_lea_adv_addr(&ind->address);
            return BT_STATUS_SUCCESS;
        }
        index = index + length + 1;
    }

    return BT_STATUS_FAIL;
}

void app_le_audio_ucst_set_sirk(bt_key_t *sirk, bool update_nvkey)
{
    memcpy((uint8_t *)&g_le_audio_sirk, (uint8_t *)sirk, sizeof(bt_key_t));

    if (update_nvkey) {
        bt_lea_csis_data_nvkey_t data_nvkey;
        nvkey_status_t ret = NVKEY_STATUS_ERROR;
        uint32_t size = BLE_CSIS_NVKEY_DATA_LEN;

        /* NVID_BT_LEA_CSIS_DATA: | SIRK (16 bytes) | reserved (2 byte) | */
        if (NVKEY_STATUS_OK == nvkey_read_data(NVID_BT_LEA_CSIS_DATA, (uint8_t*)&data_nvkey, &size)) {
            memcpy(&data_nvkey, (uint8_t *)sirk, sizeof(bt_key_t));
            ret = nvkey_write_data(NVID_BT_LEA_CSIS_DATA, (uint8_t*)&data_nvkey, size);
        }

        LE_AUDIO_MSGLOG_I("[APP][U] set_sirk, nvkey write ret:%x", 1, ret);
    }
}

bt_key_t *app_le_audio_ucst_get_sirk(bool from_nvkey)
{
    if (from_nvkey) {
        bt_lea_csis_data_nvkey_t data_nvkey;
        uint32_t size = BLE_CSIS_NVKEY_DATA_LEN;

        /* NVID_BT_LEA_CSIS_DATA: | SIRK (16 bytes) | reserved (2 byte) | */
        if (NVKEY_STATUS_OK == nvkey_read_data(NVID_BT_LEA_CSIS_DATA, (uint8_t*)&data_nvkey, &size)) {
            memcpy((uint8_t *)&g_le_audio_sirk, (uint8_t*)&data_nvkey, sizeof(bt_key_t));
            LE_AUDIO_MSGLOG_I("[APP][U] get_sirk, nvkey read success", 0);
            LE_AUDIO_MSGLOG_I("[APP][U] get_sirk, sirk:%x %x %x %x %x %x %x %x", 8,
                              g_le_audio_sirk[0], g_le_audio_sirk[1], g_le_audio_sirk[2], g_le_audio_sirk[3],
                              g_le_audio_sirk[4], g_le_audio_sirk[5], g_le_audio_sirk[6], g_le_audio_sirk[7]);
            LE_AUDIO_MSGLOG_I("[APP][U] get_sirk, sirk:%x %x %x %x %x %x %x %x", 8,
                              g_le_audio_sirk[8], g_le_audio_sirk[9], g_le_audio_sirk[10], g_le_audio_sirk[11],
                              g_le_audio_sirk[12], g_le_audio_sirk[13], g_le_audio_sirk[14], g_le_audio_sirk[15]);
            return &g_le_audio_sirk;
        }

        LE_AUDIO_MSGLOG_I("[APP][U] get_sirk, nvkey read failed", 0);
        return NULL;

    }

    return &g_le_audio_sirk;
}

void app_le_audio_csip_reset(void)
{
    memset(&g_lea_ucst_waiting_conn_addr, 0, sizeof(bt_addr_t));

    memset(&g_lea_ucst_set_white_list, 0, sizeof(app_le_audio_ucst_set_white_list_t));
    /* reset link_info */
    for (int32_t i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
        app_le_audio_ucst_reset_link_info(i);
    }
}

bt_status_t app_le_audio_ucst_start_scan_device(void)
{
    bt_status_t ret;

    LE_AUDIO_MSGLOG_I("[APP][U] start_scan_device, scan:%x->%x conn:%x", 3, g_lea_ucst_ctrl.curr_scan, g_lea_ucst_ctrl.next_scan,
                      g_lea_ucst_ctrl.curr_conn);

    g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_LEA_DEVICE;

    bt_app_common_at_cmd_print_report("Scan LEA:\r\n");

    if (BT_STATUS_SUCCESS != (ret = app_le_audio_ucst_start_scan(false))) {
        g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
    }

    LE_AUDIO_MSGLOG_I("[APP][U] start_scan_device, curr_scan:%x conn:%x", 2, g_lea_ucst_ctrl.curr_scan, g_lea_ucst_ctrl.curr_conn);

    return ret;
}

bt_status_t app_le_audio_ucst_stop_scan_device(void)
{
    LE_AUDIO_MSGLOG_I("[APP][U] stop_scan_device, scan:%x->%x conn:%x", 3, g_lea_ucst_ctrl.curr_scan, g_lea_ucst_ctrl.next_scan,
                      g_lea_ucst_ctrl.curr_conn);

    //g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;

    bt_app_common_at_cmd_print_report("Stop scan LEA:\r\n");

    return app_le_audio_ucst_stop_scan();
}

/* stop current scan (scan LE AUDIO devices or scan coordinated set devices)*/
bt_status_t app_le_audio_ucst_stop_scan_all(void)
{
    if (APP_LE_AUDIO_UCST_SCAN_NONE == g_lea_ucst_ctrl.curr_scan) {
        return BT_STATUS_SUCCESS;
    }
    if ((APP_LE_AUDIO_UCST_SCAN_ENABLING <= g_lea_ucst_ctrl.curr_scan) &&
        (APP_LE_AUDIO_UCST_SCAN_DISABLING >= g_lea_ucst_ctrl.curr_scan)) {
        return app_le_audio_ucst_stop_scan();
    }
    if ((APP_LE_AUDIO_UCST_SCAN_CS_ENABLING <= g_lea_ucst_ctrl.curr_scan) &&
        (APP_LE_AUDIO_UCST_SCAN_CS_DISABLING >= g_lea_ucst_ctrl.curr_scan)) {
        return app_le_audio_ucst_stop_scan_cs();
    }
    return BT_STATUS_SUCCESS;
}

void app_le_audio_ucst_check_group_device_bond(const bt_addr_t *addr)
{
    app_le_audio_ucst_bonded_device_t *p_bond = NULL;
    uint8_t i, j;

    if (0 == g_lea_ucst_bonded_list.num) {
        return;
    }

    for (i = 0; i < g_lea_ucst_bonded_list.num; i++) {
        p_bond = &g_lea_ucst_bonded_list.device[i];
        p_bond->reason = BT_HCI_STATUS_SUCCESS;
        if (0 == memcmp(&(p_bond->addr), addr, sizeof(bt_addr_t))) {
            if (1 < p_bond->group_size) {
                for (j = 0; j < g_lea_ucst_bonded_list.num; j++) {
                    if ((i != j) && (p_bond->group_id == g_lea_ucst_bonded_list.device[j].group_id)) {
                        g_lea_ucst_bonded_list.device[j].in_white_list = true;
                        g_lea_ucst_bonded_list.device[j].reason = BT_HCI_STATUS_SUCCESS;
                        app_le_audio_ucst_add_white_list(&g_lea_ucst_bonded_list.device[j].addr);
                    }
                }
            }
            break;
        }
    }
}

/* scan and connect coordinated set devices */
bt_status_t app_le_audio_ucst_connect_coordinated_set(bool use_nvkey_sirk)
{
    uint8_t link_num = 0;
    bt_status_t ret;

    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == (link_num = app_le_audio_ucst_get_link_num())) {
        LE_AUDIO_MSGLOG_I("[APP][U] connect_cs, link full:%x", 1, link_num);
#ifndef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        return BT_STATUS_FAIL;
#endif
    }

    if (use_nvkey_sirk) {
        app_le_audio_ucst_get_sirk(true);
    }

#if 1   /* for test */
    {
        LE_AUDIO_MSGLOG_I("[APP][U] connect_cs, scan:%x->%x conn:%x", 3, g_lea_ucst_ctrl.curr_scan, g_lea_ucst_ctrl.next_scan,
                          g_lea_ucst_ctrl.curr_conn);

        LE_AUDIO_MSGLOG_I("[APP] connect_cs, sirk:%x-%x-%x-%x-%x-%x-%x-%x", 8,
                          g_le_audio_sirk[0],
                          g_le_audio_sirk[1],
                          g_le_audio_sirk[2],
                          g_le_audio_sirk[3],
                          g_le_audio_sirk[4],
                          g_le_audio_sirk[5],
                          g_le_audio_sirk[6],
                          g_le_audio_sirk[7]);
        LE_AUDIO_MSGLOG_I("[APP] connect_cs, sirk:%x-%x-%x-%x-%x-%x-%x-%x", 8,
                          g_le_audio_sirk[8],
                          g_le_audio_sirk[9],
                          g_le_audio_sirk[10],
                          g_le_audio_sirk[11],
                          g_le_audio_sirk[12],
                          g_le_audio_sirk[13],
                          g_le_audio_sirk[14],
                          g_le_audio_sirk[15]);
    }
#endif

    ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_LE_AUDIO,
                        APP_LE_AUDIO_EVENT_START_SCAN_NEW_DEVICE, NULL, 0,
                        NULL, 0);

    g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_COORDINATED_SET_BY_SIRK;

    ble_csip_set_by_sirk(g_le_audio_sirk);
    if (BT_STATUS_SUCCESS != (ret = app_le_audio_ucst_start_scan(false))) {
        g_lea_ucst_ctrl.curr_conn = APP_LE_AUDIO_UCST_CONN_NONE;
    }

    return ret;
}

bt_status_t app_le_audio_ucst_find_device(void)
{
    if (0 != g_lea_ucst_bonded_list.num) {
        return app_le_audio_ucst_connect_bonded_device(false);
    }

    return app_le_audio_ucst_connect_coordinated_set(true);
}

bt_status_t app_le_audio_ucst_connect_device(const bt_addr_t *addr)
{
    bt_bd_addr_t empty_addr = {0};
    bt_status_t ret = BT_STATUS_FAIL;
    uint8_t link_idx;
    uint8_t i;
    app_le_audio_ucst_link_info_t *link_info_connecting = app_le_audio_ucst_find_connecting_link_info_by_peer_addr(addr, NULL);
    app_le_audio_ucst_link_info_t *link_info = NULL;

    bt_hci_cmd_le_create_connection_t param = {
        .le_scan_interval = 0x10,
        .le_scan_window = 0x10,
        .initiator_filter_policy = BT_HCI_CONN_FILTER_ASSIGNED_ADDRESS,
        .own_address_type = APP_LE_AUDIO_DONGLE_ADDR_TYPE,
        .conn_interval_min = 0x0008,
        .conn_interval_max = 0x0018,
        .conn_latency = 0x0000,
        .supervision_timeout = 0x01F4,
        .minimum_ce_length = 0x0002,
        .maximum_ce_length = 0x0002,
    };

    if (0 == memcmp(addr->addr, empty_addr, sizeof(bt_bd_addr_t))) {
        return BT_STATUS_FAIL;
    }

    if (link_info_connecting) {
        LE_AUDIO_MSGLOG_I("[APP][U] device is connecting, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                          addr->type,
                          addr->addr[5],
                          addr->addr[4],
                          addr->addr[3],
                          addr->addr[2],
                          addr->addr[1],
                          addr->addr[0]);
        return BT_STATUS_SUCCESS;
    }

    /* check cancel connection */
    for (i = 0; APP_LE_AUDIO_UCST_LINK_MAX_NUM > i ; i++) {
        link_info = &g_lea_ucst_link_info[i];

        if ((BT_HANDLE_INVALID == link_info->handle) &&
            (NULL != link_info->le_connection_timer_handle)) {
            app_le_audio_timer_stop(link_info->le_connection_timer_handle);
            link_info->le_connection_timer_handle = NULL;
            app_le_audio_ucst_reset_link_info(i);
            g_lea_ucst_waiting_cancel_create_connection = true;
            memcpy(&g_lea_ucst_waiting_create_connection_addr, addr, sizeof(bt_addr_t));
            LE_AUDIO_MSGLOG_I("[APP][U] connect_device, busy", 0);
            return bt_gap_le_srv_cancel_connection();
        }
    }

    link_info = app_le_audio_ucst_get_available_link_info_for_new_le_connection(&link_idx);

    if (!link_info) {
        /* Link full. Or there is LE connection on-going and new connection will make link full. */
        return BT_STATUS_TIMER_FULL;
    }

    /*if (g_lea_ucst_ctrl.is_creating_connection) {
        LE_AUDIO_MSGLOG_I("[APP][U] connect_device, busy", 0);
        return BT_STATUS_BUSY;
    }

    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM == app_le_audio_ucst_get_link_num()) {
        LE_AUDIO_MSGLOG_I("[APP][U] connect_device, link full!", 0);
        return BT_STATUS_FAIL;
    }*/

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    if ((true == app_le_audio_ucst_is_group_full()) && (!app_le_audio_ucst_is_bonded_device(addr, false))) {
        LE_AUDIO_MSGLOG_I("[APP][U] connect_device, group full!", 0);
        return BT_STATUS_FAIL;
    }
#endif

    memcpy(&(param.peer_address), addr, sizeof(bt_addr_t));

#if 1
    LE_AUDIO_MSGLOG_I("[APP][U] connect_device, addrType:%x addr:%02x:%02x:%02x:%02x:%02x:%02x", 7,
                      param.peer_address.type,
                      param.peer_address.addr[5],
                      param.peer_address.addr[4],
                      param.peer_address.addr[3],
                      param.peer_address.addr[2],
                      param.peer_address.addr[1],
                      param.peer_address.addr[0]);

#endif

    memcpy(&(link_info->addr), addr, sizeof(bt_addr_t));
    ret = app_le_audio_timer_start(&(link_info->le_connection_timer_handle),
                                   APP_LE_AUDIO_TIMER_LE_CONNECTION_TIME_PERIOD,
                                   app_le_audio_ucst_le_connection_timer_callback,
                                   link_info);

    if (BT_STATUS_SUCCESS == ret) {
        app_le_audio_ucst_stop_scan_all();
        ret = bt_gap_le_srv_connect(&param);

        if (BT_STATUS_SUCCESS != ret) {
            app_le_audio_timer_stop(link_info->le_connection_timer_handle);
            link_info->le_connection_timer_handle = NULL;
            app_le_audio_ucst_reset_link_info(link_idx);
            app_le_audio_ucst_connect_coordinated_set(false);
        }
    }

    return ret;
}

void app_le_audio_ucst_cancel_create_connection(void)
{
    app_le_audio_ucst_link_info_t *link_info = NULL;

    /* check cancel connection */
    for (uint32_t i = 0; APP_LE_AUDIO_UCST_LINK_MAX_NUM > i ; i++) {
        link_info = &g_lea_ucst_link_info[i];

        if ((BT_HANDLE_INVALID == link_info->handle) &&
            (NULL != link_info->le_connection_timer_handle)) {
            app_le_audio_timer_stop(link_info->le_connection_timer_handle);
            link_info->le_connection_timer_handle = NULL;
            app_le_audio_ucst_reset_link_info(i);
            bt_gap_le_cancel_connection();
        }
    }
}

void app_le_audio_ucst_handle_cancel_create_connection_cnf(bt_status_t ret, void *ind)
{
    LE_AUDIO_MSGLOG_I("[APP][U] cancel_create_connection_cnf", 0);

    if (g_lea_ucst_waiting_cancel_create_connection) {
        g_lea_ucst_waiting_cancel_create_connection = false;
        app_le_audio_ucst_connect_device(&g_lea_ucst_waiting_create_connection_addr);
        memset(&g_lea_ucst_waiting_create_connection_addr, 0, sizeof(bt_addr_t));
    }
}

//Disconnect ACL or CIS
bt_status_t app_le_audio_ucst_disconnect(bt_handle_t handle)
{
    bt_status_t ret;
    bt_hci_cmd_disconnect_t param;

    if (BT_HANDLE_INVALID == handle) {
        LE_AUDIO_MSGLOG_I("[APP][U] disconnect, invalid handle", 0);
        return BT_STATUS_FAIL;
    }

    param.connection_handle = handle;
    param.reason = BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST;
    ret = bt_gap_le_disconnect(&param);
    LE_AUDIO_MSGLOG_I("[APP][U] disconnect, handle:%x ret:%x", 2, param.connection_handle, ret);

    return ret;
}

bt_status_t app_le_audio_ucst_disconnect_with_reason(bt_handle_t handle, bt_hci_disconnect_reason_t reason)
{
    bt_status_t ret;
    bt_hci_cmd_disconnect_t param;

    if (BT_HANDLE_INVALID == handle) {
        LE_AUDIO_MSGLOG_I("[APP][U] disconnect, invalid handle", 0);
        return BT_STATUS_FAIL;
    }

    param.connection_handle = handle;
    param.reason = reason;
    ret = bt_gap_le_disconnect(&param);
    LE_AUDIO_MSGLOG_I("[APP][U] disconnect, handle:%x reason:%x ret:%x", 3, param.connection_handle, reason, ret);

    return ret;
}

bt_status_t app_le_audio_ucst_disconnect_device(bt_addr_t *addr)
{
    uint8_t i;

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;
    while (0 != i) {
        i--;
        if (0 == memcmp(&(g_lea_ucst_link_info[i].addr), addr, sizeof(bt_addr_t))) {
            return app_le_audio_ucst_disconnect_device_by_idx(i);
        }
    }

    return BT_STATUS_FAIL;
}


bt_handle_t app_le_audio_ucst_get_handle(uint8_t link_idx)
{
    return g_lea_ucst_link_info[link_idx].handle;
}


uint8_t app_le_audio_ucst_get_link_num(void)
{
    uint8_t i = 0, count = 0;

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        if (BT_HANDLE_INVALID != g_lea_ucst_link_info[i].handle) {
            count++;
        }
    }

    return count;
}

uint8_t app_le_audio_ucst_get_link_idx(bt_handle_t handle)
{
    uint8_t i = 0;

    if (BT_HANDLE_INVALID == handle) {
        return APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
    }

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        if (handle == g_lea_ucst_link_info[i].handle) {
            return i;
        }
    }

    return APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
}

uint8_t app_le_audio_ucst_get_link_idx_by_cis(bt_handle_t cis_handle, uint8_t *cis_idx)
{
    uint8_t i, link_idx;

    i = APP_LE_AUDIO_UCST_CIS_MAX_NUM;

    while (i > 0) {
        i--;
        if (g_lea_ucst_cis_info[i].cis_handle == cis_handle) {
            if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID != (link_idx = app_le_audio_ucst_get_link_idx(g_lea_ucst_cis_info[i].acl_handle))) {
                *cis_idx = i;
                return link_idx;
            }
        }
    }

    *cis_idx = APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
    return APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
}

/*
uint8_t app_le_audio_ucst_get_link_idx_by_addr(bt_addr_t *addr)
{
    uint8_t i = 0;

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        if (0 == memcmp((uint8_t *)&g_lea_ucst_link_info[i].addr, addr, sizeof(bt_addr_t))) {
            return i;
        }
    }

    return APP_LE_AUDIO_UCST_LINK_IDX_INVALID;
}
*/
app_le_audio_ucst_link_info_t *app_le_audio_ucst_get_link_info(bt_handle_t handle)
{
    uint8_t i = 0;

    if (BT_HANDLE_INVALID == handle) {
        return NULL;
    }

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        if (handle == g_lea_ucst_link_info[i].handle) {
            return &g_lea_ucst_link_info[i];
        }
    }

    return NULL;
}

app_le_audio_ucst_link_info_t *app_le_audio_ucst_get_link_info_by_idx(uint8_t link_idx)
{
    if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= link_idx) {
        return NULL;
    }

    return &g_lea_ucst_link_info[link_idx];
}

app_le_audio_ucst_link_info_t *app_le_audio_ucst_get_link_info_by_addr(bt_addr_t *addr)
{
    uint8_t i = 0;

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        if (0 == memcmp((uint8_t *)&g_lea_ucst_link_info[i].addr, addr, sizeof(bt_addr_t))) {
            return &g_lea_ucst_link_info[i];
        }
    }

    return NULL;
}

const bt_bd_addr_t *app_le_audio_csip_get_dongle_addr(bt_addr_type_t *addr_type)
{
    if (addr_type) {
        *addr_type = APP_LE_AUDIO_DONGLE_ADDR_TYPE;
    }

#if (APP_LE_AUDIO_DONGLE_ADDR_TYPE == BT_ADDR_PUBLIC)
    return bt_device_manager_get_local_address();
#else
    return bt_app_common_get_local_random_addr();
#endif

}

void app_le_audio_csip_handle_power_on(void)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    /* reset bonded list, active group and group info */
    app_le_audio_ucst_reset_active_group();
    app_le_audio_ucst_reset_all_bonded_list();
    app_le_audio_ucst_reset_all_group_info();

    /* update bonded list */
    app_le_audio_ucst_read_bonded_list_from_nvkey();
    app_le_audio_ucst_refresh_all_bonded_list();
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    app_le_audio_ucst_read_sirk_list_from_nvkey();
#endif

    /* update group info */
    app_le_audio_ucst_refresh_group_info();

    /* update active group */
    app_le_audio_ucst_read_active_group_from_nvkey();

    /* add device to white list */
    //app_le_audio_ucst_add_bonded_device_to_white_list();

    //app_le_audio_start_unicast();
    app_le_audio_ucst_get_sirk(true);
    ble_csip_set_by_sirk(g_le_audio_sirk);
#endif

}

void app_le_audio_csip_handle_power_off(void)
{
    app_le_audio_csip_reset();
}


void app_le_audio_ucst_handle_csip_evt(ble_csip_event_t event, void *msg)
{
    if (NULL == msg) {
        return;
    }

    switch (event) {
        case BLE_CSIP_DISCOVER_COORDINATED_SET_CNF: {
            app_le_audio_ucst_handle_csip_discover_cs_cnf((ble_csip_discover_coordinated_set_cnf_t *)msg);
            break;
        }
        case BLE_CSIP_READ_COORDINATED_SET_SIZE_CNF: {
            app_le_audio_ucst_handle_csip_read_cs_size_cnf((ble_csip_read_set_size_cnf_t *)msg);
            break;
        }
        case BLE_CSIP_READ_SIRK_CNF: {
            app_le_audio_ucst_handle_csip_sirk_cnf((ble_csip_read_sirk_cnf_t *)msg);
            break;
        }
        case BLE_CSIP_READ_SET_MEMBER_LOCK_CNF: {
            ble_csip_read_set_lock_cnf_t *cnf = (ble_csip_read_set_lock_cnf_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][CSIP] BLE_CSIP_READ_SET_MEMBER_LOCK_CNF, handle:%x set_lock:%x", 2, cnf->handle, cnf->lock);
            break;
        }
        case BLE_CSIP_NOTIFY_SET_MEMBER_LOCK_NOTIFY: {
            ble_csip_notify_set_lock_ind_t *ind = (ble_csip_notify_set_lock_ind_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][CSIP] BLE_CSIP_NOTIFY_SET_MEMBER_LOCK_NOTIFY, handle:%x set_lock:%x", 2, ind->handle, ind->lock);
            break;
        }
        case BLE_CSIP_READ_SET_MEMBER_RANK_CNF: {
            ble_csip_read_rank_cnf_t *cnf = (ble_csip_read_rank_cnf_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][CSIP] BLE_CSIP_READ_SET_MEMBER_RANK_CNF, handle:%x set_rank:%x", 2, cnf->handle, cnf->rank);
            break;
        }
        case BLE_CSIP_DISCOVER_SERVICE_COMPLETE_NOTIFY: {
            ble_csip_discover_service_complete_t *ind = (ble_csip_discover_service_complete_t *)msg;

            LE_AUDIO_MSGLOG_I("[APP][CSIP] DISCOVER_SERVICE_COMPLETE, handle:%x ret:%x", 2, ind->handle, ind->status);

            if (BT_STATUS_SUCCESS == ind->status) {
                break;
            }

            app_le_audio_ucst_link_info_t *p_info = NULL;
            ble_csip_read_set_size_cnf_t cnf;
            uint8_t link_idx;

            if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == (link_idx = app_le_audio_ucst_get_link_idx(ind->handle))) {
                LE_AUDIO_MSGLOG_I("[APP][CSIP] DISCOVER_SERVICE_COMPLETE, link not exist (hdl:%x)", 1, ind->handle);
                break;
            }

            p_info = &g_lea_ucst_link_info[link_idx];

            /* check group_info */
            if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID == p_info->group_id) {
                /* new device */

                if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= (p_info->group_id = app_le_audio_ucst_set_new_group_info(NULL, link_idx))) {
                    LE_AUDIO_MSGLOG_I("[APP][CSIP] DISCOVER_SERVICE_COMPLETE, ERROR! handle:%x check group:%x)", 2,
                                      p_info->handle,
                                      p_info->group_id);
                    p_info->group_id = APP_LE_AUDIO_UCST_GROUP_ID_INVALID;
                    break;
                }

#ifndef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
                if (APP_LE_AUDIO_UCST_GROUP_ID_INVALID == g_lea_ucst_ctrl.curr_group) {
                    g_lea_ucst_ctrl.curr_group = p_info->group_id;
                }
#endif
                g_lea_ucst_ctrl.latest_group = p_info->group_id;

            }

            LE_AUDIO_MSGLOG_I("[APP][CSIP] DISCOVER_SERVICE_COMPLETE, handle:%x group:%x", 2,
                              p_info->handle,
                              p_info->group_id);


            cnf.handle = p_info->handle;
            cnf.size = 1;
            if (g_lea_ucst_pts_set_size) {
                /* some PTS cases which need 2 or more PTS dongles may have no CSIS */
               cnf.size = g_lea_ucst_pts_set_size;
            }
            app_le_audio_ucst_handle_csip_read_cs_size_cnf(&cnf);
            break;
        }
        default:
            break;
    }
}

void app_le_audio_handle_gap_le_srv_event_callback(bt_gap_le_srv_event_t event, void *data)
{
    switch (event) {
        case BT_GAP_LE_SRV_GET_LINK_INFO: {
            bt_gap_le_srv_link_info_t *link_info = (bt_gap_le_srv_link_info_t *)data;
            if (app_le_audio_ucst_find_connecting_link_info_by_peer_addr(&link_info->remote_address, NULL) ||
                app_le_audio_ucst_is_connected_device(&link_info->remote_address)
                ) {
                link_info->link_type = BT_GAP_LE_SRV_LINK_TYPE_LE_AUDIO;
                LE_AUDIO_MSGLOG_I("[APP] app_le_audio_handle_gap_le_srv_event_callback, set lea link type-1", 0);

            }
            break;
        }
        default:
            break;
    }
}

void app_le_audio_ucst_sync_lock_stream_flag(app_le_audio_ucst_link_info_t *p_info, app_le_audio_ucst_lock_stream_t lock, bool set)
{
    app_le_audio_ucst_link_info_t *p_info_tmp = NULL;
    uint8_t i = 0;

    if (NULL == p_info) {
        return;
    }

    if (p_info->group_size == 1) {
        return;
    }

    i = APP_LE_AUDIO_UCST_LINK_MAX_NUM;

    while (i > 0) {
        i--;
        p_info_tmp = &g_lea_ucst_link_info[i];
        if ((p_info->group_id == p_info_tmp->group_id) &&
            (p_info->handle != p_info_tmp->handle) &&
            (BT_HANDLE_INVALID != p_info_tmp->handle)) {
            if (set) {
                p_info_tmp->lock_stream |= lock;
            } else {
                p_info_tmp->lock_stream &= ~lock;
            }
            break;
        }
    }
}

bt_status_t app_le_audio_ucst_update_connection_interval(app_le_audio_ucst_link_info_t *p_info, uint16_t interval)
{
    bt_status_t ret = BT_STATUS_FAIL;
    uint16_t ce_len = 0x0002;//Connection Event length

    if ((p_info == NULL) || (BT_HANDLE_INVALID == p_info->handle)) {
        return ret;
    }
    // Delete for BTA-51532
    //if (APP_LE_AUDIO_CONN_INTERVAL_CONFIG == interval) {
    //    ce_len = 0x0003;
    //}

    if (((interval != p_info->curr_interval) &&
         (interval != p_info->next_interval)) ||
        ((interval == p_info->curr_interval) &&
         (interval != p_info->next_interval) &&
         (0 != p_info->next_interval))) {
        LE_AUDIO_MSGLOG_I("[APP][U] conn_interval, handle:%x interval:%x->%x set:%x", 4, p_info->handle,
                          p_info->curr_interval, p_info->next_interval, interval);
        p_info->next_interval = interval;
        if (BT_STATUS_SUCCESS != (ret = app_le_audio_ucst_update_connection_parameter(p_info->handle, interval, ce_len))) {
            p_info->next_interval = 0;
        }
    }
    return ret;
}



void app_le_audio_ucst_increase_connection_config_speed(bt_handle_t handle)
{
    uint8_t i;
    bt_status_t ret = BT_STATUS_SUCCESS;
    app_le_audio_ucst_link_info_t *p_link_info = NULL;

    for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
        p_link_info = &g_lea_ucst_link_info[i];

        if ((BT_HANDLE_INVALID != p_link_info->handle) && (handle == p_link_info->handle)) {
            if (NULL != p_link_info->conn_interval_timer_handle) { // Restart timer
                app_le_audio_timer_stop(p_link_info->conn_interval_timer_handle);
                p_link_info->conn_interval_timer_handle = NULL;
                app_le_audio_timer_start(&(p_link_info->conn_interval_timer_handle),
                                         APP_LE_AUDIO_UCST_TIMER_CONN_INTERVAL_CONFIG_TIME_PERIOD,
                                         app_le_audio_ucst_increase_connection_config_speed_timer_callback,
                                         p_link_info);
            } else if (BT_STATUS_SUCCESS == app_le_audio_ucst_update_connection_interval(p_link_info, APP_LE_AUDIO_CONN_INTERVAL_CONFIG)) {
                ret = app_le_audio_timer_start(&(p_link_info->conn_interval_timer_handle),
                                               APP_LE_AUDIO_UCST_TIMER_CONN_INTERVAL_CONFIG_TIME_PERIOD,
                                               app_le_audio_ucst_increase_connection_config_speed_timer_callback,
                                               p_link_info);
                if (BT_STATUS_SUCCESS != ret) {
                    app_le_audio_ucst_update_connection_interval(p_link_info, APP_LE_AUDIO_CONN_INTERVAL_STREAMING);
                }
            }
            break;
        }
    }

}

void app_le_audio_ucst_increase_active_device_config_speed(void)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t i, tmp;

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= g_lea_ucst_ctrl.curr_group) {
            break;
        }
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        p_info = &g_lea_ucst_link_info[i];
        app_le_audio_ucst_increase_connection_config_speed(p_info->handle);
    }
}

void app_le_audio_ucst_check_link_idle(bt_handle_t handle)
{
    uint8_t i;
    app_le_audio_ucst_link_info_t *p_link_info = NULL;

    for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
        p_link_info = &g_lea_ucst_link_info[i];

        if ((BT_HANDLE_INVALID != p_link_info->handle) && (handle == p_link_info->handle)) {
            LE_AUDIO_MSGLOG_I("[APP][U] link handle:%x curr_state:%x->%x  curr_target:%x->%x", 5, handle, p_link_info->curr_state, p_link_info->next_state, g_lea_ucst_ctrl.curr_target, g_lea_ucst_ctrl.next_target);
            if (((APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC == p_link_info->curr_state) || (APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS == p_link_info->curr_state)) &&
                (APP_LE_AUDIO_UCST_LINK_STATE_IDLE == p_link_info->next_state) &&
                (((APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.curr_target) &&
                  (APP_LE_AUDIO_UCST_TARGET_NONE == g_lea_ucst_ctrl.next_target))
#ifdef AIR_SILENCE_DETECTION_ENABLE
                 || (APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE == g_lea_ucst_ctrl.curr_target)
#endif
                )) {
                if (NULL != p_link_info->conn_interval_timer_handle) {
                    app_le_audio_timer_stop(p_link_info->conn_interval_timer_handle);
                    p_link_info->conn_interval_timer_handle = NULL;
                }
                LE_AUDIO_MSGLOG_I("[APP][U] link is idle, upate connection interval to 0x30, handle:%x ", 1, handle);
                app_le_audio_ucst_update_connection_interval(p_link_info, APP_LE_AUDIO_CONN_INTERVAL_STREAMING);
            }
            break;
        }
    }

}

void app_le_audio_ucst_check_active_device_idle(void)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t i, tmp;

    for (tmp = 0; tmp < app_le_audio_ucst_get_max_link_num(); tmp++) {
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (APP_LE_AUDIO_UCST_GROUP_ID_MAX <= g_lea_ucst_ctrl.curr_group) {
            break;
        }
        if (APP_LE_AUDIO_UCST_LINK_MAX_NUM <= (i = g_lea_ucst_group_info[g_lea_ucst_ctrl.curr_group].link_idx[tmp])) {
            continue;
        }
#else
        i = tmp;
#endif
        p_info = &g_lea_ucst_link_info[i];
        app_le_audio_ucst_check_link_idle(p_info->handle);
    }
}

bool app_le_audio_ucst_is_connection_update_request_accepted(bt_handle_t handle, bt_gap_le_connection_update_param_t *connection_parameter)
{
    /* Received the L2cap connection parameter update request from the remote device. */

    LE_AUDIO_MSGLOG_I("[APP][U] Reject update connection interval handle:%x interval:%x~%x, slave_latency:%x, timeout:%x", 5, handle,
    connection_parameter->interval_min,
    connection_parameter->interval_max,
    connection_parameter->slave_latency,
    connection_parameter->timeout_multiplier);
    if (app_le_audio_aird_is_connected(handle)) {
        return true;
    }

    return false;
}
#endif

