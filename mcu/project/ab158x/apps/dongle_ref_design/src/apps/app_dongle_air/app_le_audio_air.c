/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#include "app_le_audio_air.h"

#include "syslog.h"
#include <stdbool.h>
#include "bt_gattc.h"
#include "bt_gattc_discovery.h"
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_le_audio_msglog.h"
#endif
#include "apps_events_interaction_event.h"
#include "apps_events_event_group.h"
#include "ui_shell_manager.h"
#include "bt_callback_manager.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_LE_AUDIO_DISABLE_NOTIFICATION   0
#define APP_LE_AUDIO_ENABLE_NOTIFICATION    1
#define APP_LE_AUDIO_CCCD_VALUE_LEN         2

#define APP_LE_AIR_SRV_MAX_CHARC_NUMBER     3
#define APP_LE_AUDIO_AIR_SRV_MAX_CHARC_NUMBER   6   /* Must be the MAX charc num of the services to be discovered */

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
#define APP_LE_AUDIO_LINK_MAX_NUM    3
#elif defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#ifdef AIR_WIRELESS_MIC_ENABLE
#define APP_LE_AUDIO_LINK_MAX_NUM      4   /**< Wireless MIC support 4 LE link. */
#else
#define APP_LE_AUDIO_LINK_MAX_NUM      2   /**< Wireless MIC support 2 LE link. */
#endif
#else
#define APP_LE_AUDIO_LINK_MAX_NUM    2
#endif

log_create_module(DONGLE_AIR, PRINT_LEVEL_INFO);


typedef struct {
    bt_gattc_discovery_characteristic_t charc[APP_LE_AUDIO_AIR_SRV_MAX_CHARC_NUMBER];
    bt_gattc_discovery_descriptor_t descrp[APP_LE_AUDIO_AIR_SRV_MAX_CHARC_NUMBER];
} app_le_audio_air_discovery_charc_t;

/**************************************************************************************************
* Variable
**************************************************************************************************/
static bt_gattc_discovery_service_t g_le_audio_air_srv_discovery;
static app_le_audio_air_info_t g_le_audio_air_info[APP_LE_AUDIO_LINK_MAX_NUM];
static app_le_audio_air_callback_t g_le_audio_air_callback = NULL;
static const uint8_t g_le_audio_air_srv_charc_uuid_rx[16] = {0x45, 0x4C, 0x42, 0x61, 0x68, 0x6F, 0x72, 0x69, 0x41, 0x32, 0xAB, 0x2D, 0x52, 0x41, 0x48, 0x43};
static const uint8_t g_le_audio_air_srv_charc_uuid_tx[16] = {0x45, 0x4C, 0x42, 0x61, 0x68, 0x6F, 0x72, 0x69, 0x41, 0x31, 0xAB, 0x2D, 0x52, 0x41, 0x48, 0x43};
static const uint8_t g_le_audio_air_srv_charc_uuid_aws_mce_role[16] = {0x45, 0x4C, 0x42, 0x61, 0x68, 0x6F, 0x72, 0x69, 0x41, 0x30, 0xAB, 0x2D, 0x52, 0x41, 0x48, 0x43};
static app_le_audio_air_discovery_charc_t g_le_audio_air_charc_discovery;


/**************************************************************************************************
* Prototype
**************************************************************************************************/
#if (defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE))
//extern bt_status_t ble_air_srv_switch_link(bt_handle_t connection_handle);
#endif
/**************************************************************************************************
* Static function
**************************************************************************************************/
static bt_status_t app_le_audio_air_set_cccd(bt_handle_t handle, uint16_t att_handle, uint16_t cccd)
{
    bt_status_t ret;
    uint8_t p_buf[5];

    BT_GATTC_NEW_WRITE_CHARC_REQ(req, p_buf, att_handle, (uint8_t *)&cccd, APP_LE_AUDIO_CCCD_VALUE_LEN);

    if (BT_STATUS_SUCCESS != (ret = bt_gattc_write_charc(handle, &req))) {
        LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] set_cccd, fail! handle:%x att_handle:%x ret:%x", 3, handle, att_handle, ret);
    }

    return ret;
}

static bt_status_t app_le_audio_air_read(bt_handle_t handle, uint16_t att_handle)
{
    bt_status_t ret;

    BT_GATTC_NEW_READ_CHARC_REQ(req, att_handle);
    if (BT_STATUS_SUCCESS != (ret = bt_gattc_read_charc(handle, &req))) {
        LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] read, fail! handle:%x att_handle:%x status:%x", 3, handle, att_handle, ret);
    }

    return ret;
}

static void app_le_audio_air_handle_read_rsp(bt_gattc_read_rsp_t *rsp)
{
    app_le_audio_air_info_t *p_info;
    bt_att_read_rsp_t *att_rsp = rsp->att_rsp;

    if (NULL == (p_info = app_le_audio_air_get_info(rsp->connection_handle))) {
        //LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] read_rsp, link not exist", 0);
        LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] read_rsp, link not exist", 0);
        return;
    }

    if (APP_LE_AUDIO_AIR_STATE_READ_ROLE != p_info->state) {
        return;
    }

    LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] read_rsp, handle:%x rsp_length:%x rsp_data:%x", 3, rsp->connection_handle, rsp->length, att_rsp->attribute_value[0]);

    p_info->role = att_rsp->attribute_value[0];

    if (BT_AWS_MCE_ROLE_AGENT == p_info->role) {
        LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] Agent, Found!", 0);
#if (defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE))
        //ble_air_srv_switch_link(rsp->connection_handle);
#endif
    }

    app_le_audio_air_start_pre_action(rsp->connection_handle, BT_STATUS_SUCCESS);
}

static void app_le_audio_air_handle_write_rsp(bt_gattc_write_rsp_t *rsp)
{
    app_le_audio_air_start_pre_action(rsp->connection_handle, BT_STATUS_SUCCESS);
}

static void app_le_audio_air_handle_error_rsp(bt_status_t ret, bt_gattc_error_rsp_t *rsp)
{
    app_le_audio_air_start_pre_action(rsp->connection_handle, BT_STATUS_FAIL);
}

static void app_le_audio_air_handle_notification(bt_gatt_handle_value_notification_t *noti)
{
    app_le_audio_air_info_t *p_info;
    bt_att_handle_value_notification_t *att_rsp = noti->att_rsp;

    if (NULL == (p_info = app_le_audio_air_get_info(noti->connection_handle))) {
        LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] notification, link not exist", 0);
        return;
    }

    if (att_rsp->handle != p_info->att_handle_role) {
        LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] notification, handle:%x , att_handle_role:%x", 2 , att_rsp->handle , p_info->att_handle_role);
        return;
    }

    LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] noti, handle:%x rsp_length:%x rsp_data:%x", 3, noti->connection_handle, noti->length, att_rsp->attribute_value[0]);

    p_info->role = att_rsp->attribute_value[0];

    if (BT_AWS_MCE_ROLE_AGENT == p_info->role) {
#if (defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE))
        //ble_air_srv_switch_link(noti->connection_handle);
#endif
    }

    if (g_le_audio_air_callback) {
        app_le_audio_air_event_rho_t param = {0};
        param.handle = noti->connection_handle;
        param.role = p_info->role;
        g_le_audio_air_callback(APP_LE_AUDIO_AIR_EVENT_RHO, &param);
    }
}

/**************************************************************************************************
* Public function
**************************************************************************************************/
void app_le_audio_air_event_handler(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    if (NULL == buff) {
        return;
    }
    LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] air_event_handler,msg:%x,status:%x", 2, msg ,status);
    switch (msg) {
        case BT_GATTC_READ_CHARC: {
            if (status == BT_STATUS_SUCCESS) {
                app_le_audio_air_handle_read_rsp((bt_gattc_read_rsp_t *)buff);

            } else {
                app_le_audio_air_handle_error_rsp(status, (bt_gattc_error_rsp_t *)buff);
            }
            break;
        }

        case BT_GATTC_WRITE_LONG_CHARC:
        case BT_GATTC_WRITE_CHARC: {
            if (status == BT_STATUS_SUCCESS) {
                app_le_audio_air_handle_write_rsp((bt_gattc_write_rsp_t *)buff);

            } else {
                app_le_audio_air_handle_error_rsp(status, (bt_gattc_error_rsp_t *)buff);
            }
            break;
        }

        case BT_GATTC_CHARC_VALUE_NOTIFICATION: {
            app_le_audio_air_handle_notification((bt_gatt_handle_value_notification_t *)buff);
            break;
        }

        default:
            break;
    }
}

void app_le_audio_air_start_pre_action(bt_handle_t handle, bt_status_t status)
{
    app_le_audio_air_info_t *p_info;

    if (NULL == (p_info = app_le_audio_air_get_info(handle))) {
        LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] pre_action, link not exist", 0);
        return;
    }

    if ((APP_LE_AUDIO_AIR_STATE_SRV_DISCOVERY_COMPLETE > p_info->state) || (APP_LE_AUDIO_AIR_STATE_READY <= p_info->state)) {
        LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] pre_action,  p_info->state:%x", 1,  p_info->state);
        return;
    }

    LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] pre_action, handle:%x curr_state:%x", 2, handle, p_info->state);

    switch (p_info->state) {
        case APP_LE_AUDIO_AIR_STATE_SRV_DISCOVERY_COMPLETE: {
            if (BT_HANDLE_INVALID != p_info->att_handle_role) {
                p_info->state = APP_LE_AUDIO_AIR_STATE_READ_ROLE;
                if (BT_STATUS_SUCCESS == app_le_audio_air_read(handle, p_info->att_handle_role)) {
                    break;
                }
            }
        }
        /*Pass through*/
        case APP_LE_AUDIO_AIR_STATE_READ_ROLE: {
            if (BT_HANDLE_INVALID != p_info->att_handle_tx_cccd) {
                p_info->state = APP_LE_AUDIO_AIR_STATE_SET_CCCD_TX;
                if (BT_STATUS_SUCCESS == app_le_audio_air_set_cccd(handle, p_info->att_handle_tx_cccd, APP_LE_AUDIO_ENABLE_NOTIFICATION)) {
                    break;
                }
            }
            break;
        }
        /*Pass through*/
        case APP_LE_AUDIO_AIR_STATE_SET_CCCD_TX: {
            if (g_le_audio_air_callback) {
                app_le_audio_air_event_enable_service_complete_t param = {0};
                param.handle = handle;
                param.status = status;
                param.role = p_info->role;
                g_le_audio_air_callback(APP_LE_AUDIO_AIR_EVENT_ENABLE_SERVICE_COMPLETE, &param);
            }
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_AUDIO_RACE_READY,
                                &p_info->role, 0, NULL, 100);
            if (BT_HANDLE_INVALID != p_info->att_handle_role_cccd) {
                p_info->state = APP_LE_AUDIO_AIR_STATE_SET_CCCD_ROLE;
                if (BT_STATUS_SUCCESS == app_le_audio_air_set_cccd(handle, p_info->att_handle_role_cccd, APP_LE_AUDIO_ENABLE_NOTIFICATION)) {
                    break;
                }
            }
        }
        /*Pass through*/
        case APP_LE_AUDIO_AIR_STATE_SET_CCCD_ROLE: {
            //LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] set_attribute_cb, done!", 0);
            p_info->state = APP_LE_AUDIO_AIR_STATE_READY;
            bt_gattc_discovery_continue(handle);
        }
        default:
            break;
    }
    //LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] pre_action, handle:%x state:%x", 2, handle, p_info->state);
}

app_le_audio_air_info_t *app_le_audio_air_get_info(bt_handle_t handle)
{
    uint8_t i;

    for (i = 0; i < APP_LE_AUDIO_LINK_MAX_NUM; i++) {
        if (handle == g_le_audio_air_info[i].conn_handle) {
            return &g_le_audio_air_info[i];
        }
    }
    return NULL;
}

bt_aws_mce_role_t app_le_audio_air_get_role(bt_addr_t *addr)
{
    uint8_t i;
    if (NULL == addr) {
        return APP_LE_AUDIO_AIR_ROLE_INVALID;
    }
    //LOG_MSGID_I(DONGLE_AIR, "[APP][AIR]addr:%2x:%2x:%2x:%2x:%2x:%2x", 6, addr->addr[5], addr->addr[4], addr->addr[3], addr->addr[2], addr->addr[1], addr->addr[0]);

    for (i = 0; i < APP_LE_AUDIO_LINK_MAX_NUM; i++) {
        if (0 == memcmp((uint8_t *)&g_le_audio_air_info[i].addr.addr, addr->addr, sizeof(bt_bd_addr_t))) {
            LOG_MSGID_I(DONGLE_AIR, "[APP][AIR]addr:%2x:%2x:%2x:%2x:%2x:%2x,role:%x", 7, addr->addr[5], addr->addr[4], addr->addr[3], addr->addr[2], addr->addr[1], addr->addr[0], g_le_audio_air_info[i].role);
            return g_le_audio_air_info[i].role;
        }
    }
    return APP_LE_AUDIO_AIR_ROLE_INVALID;
}

void app_le_audio_air_reset_info(uint8_t link_idx)
{
    if (APP_LE_AUDIO_LINK_MAX_NUM <= link_idx) {
        return;
    }


    g_le_audio_air_info[link_idx].conn_handle = BT_HANDLE_INVALID;
    memset(&g_le_audio_air_info[link_idx].addr, 0, sizeof(g_le_audio_air_info[link_idx].addr));
    g_le_audio_air_info[link_idx].att_handle_tx = BT_HANDLE_INVALID;
    g_le_audio_air_info[link_idx].att_handle_rx = BT_HANDLE_INVALID;
    g_le_audio_air_info[link_idx].att_handle_tx = BT_HANDLE_INVALID;
    g_le_audio_air_info[link_idx].att_handle_tx_cccd = BT_HANDLE_INVALID;
    g_le_audio_air_info[link_idx].att_handle_role = BT_HANDLE_INVALID;
    g_le_audio_air_info[link_idx].att_handle_role_cccd = BT_HANDLE_INVALID;
    g_le_audio_air_info[link_idx].state = APP_LE_AUDIO_AIR_STATE_IDLE;
    g_le_audio_air_info[link_idx].role = APP_LE_AUDIO_AIR_ROLE_INVALID;
}

void app_le_audio_air_init(void)
{
    uint8_t i;

    for (i = 0; i < APP_LE_AUDIO_LINK_MAX_NUM; i++) {
        app_le_audio_air_reset_info(i);
    }
}

bt_status_t app_le_audio_air_register_callback(app_le_audio_air_callback_t callback)
{
    if (NULL != g_le_audio_air_callback) {
        return BT_STATUS_FAIL;
    }

    g_le_audio_air_callback = callback;
    return BT_STATUS_SUCCESS;
}
static void app_le_audio_air_srv_discovery_callback(bt_gattc_discovery_event_t *event)
{
    app_le_audio_air_info_t *p_air_info;
    bt_gattc_discovery_characteristic_t *p_charc = NULL;
    uint8_t i;

    if (NULL == event) {
        bt_gattc_discovery_continue(BT_HANDLE_INVALID);
        return;
    }

    if ((BT_GATTC_DISCOVERY_EVENT_FAIL == event->event_type) && (!event->last_instance)) {
        bt_gattc_discovery_continue(event->conn_handle);
        return;
    }

    if (NULL == (p_air_info = app_le_audio_air_get_info(event->conn_handle))) {
        bt_gattc_discovery_continue(event->conn_handle);
        return;
    }

    LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] air_srv_discovery_callback, handle:%x charc_num:%d", 2, event->conn_handle, g_le_audio_air_srv_discovery.char_count_found);

    if (BT_GATTC_DISCOVERY_EVENT_COMPLETE == event->event_type) {

        if (0 != (i = g_le_audio_air_srv_discovery.char_count_found)) {
            while (i > 0) {
                i--;
                p_charc = &g_le_audio_air_srv_discovery.charateristics[i];

#if 0
                LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] air_srv_discovery_callback, uuid:%x %x %x %x %x %x %x %x", 8, p_charc->char_uuid.uuid.uuid[0],
                                  p_charc->char_uuid.uuid.uuid[1],
                                  p_charc->char_uuid.uuid.uuid[2],
                                  p_charc->char_uuid.uuid.uuid[3],
                                  p_charc->char_uuid.uuid.uuid[4],
                                  p_charc->char_uuid.uuid.uuid[5],
                                  p_charc->char_uuid.uuid.uuid[6],
                                  p_charc->char_uuid.uuid.uuid[7]);

                LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] air_srv_discovery_callback, uuid:%x %x %x %x %x %x %x %x", 8, p_charc->char_uuid.uuid.uuid[8],
                                  p_charc->char_uuid.uuid.uuid[9],
                                  p_charc->char_uuid.uuid.uuid[10],
                                  p_charc->char_uuid.uuid.uuid[11],
                                  p_charc->char_uuid.uuid.uuid[12],
                                  p_charc->char_uuid.uuid.uuid[13],
                                  p_charc->char_uuid.uuid.uuid[14],
                                  p_charc->char_uuid.uuid.uuid[15]);
#endif

                if (0 == memcmp(&p_charc->char_uuid.uuid.uuid[0], &g_le_audio_air_srv_charc_uuid_rx[0], 16)) {
                    /* RX */
                    p_air_info->att_handle_rx = p_charc->value_handle;
                    //LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] air_srv_discovery_callback, att_handle_rx:%d", 1, p_air_info->att_handle_rx);

                } else if (0 == memcmp(&p_charc->char_uuid.uuid.uuid[0], &g_le_audio_air_srv_charc_uuid_tx[0], 16)) {
                    /* TX */
                    p_air_info->att_handle_tx = p_charc->value_handle;
                    p_air_info->att_handle_tx_cccd = p_charc->descriptor[0].handle;
                    //LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] air_srv_discovery_callback, att_handle_tx:%d att_handle_tx_cccd:%x", 2, p_air_info->att_handle_rx, p_air_info->att_handle_tx_cccd);

                } else if (0 == memcmp(&p_charc->char_uuid.uuid.uuid[0], &g_le_audio_air_srv_charc_uuid_aws_mce_role[0], 16)) {
                    /* role */
                    p_air_info->att_handle_role = p_charc->value_handle;
                    p_air_info->att_handle_role_cccd = p_charc->descriptor[0].handle;
                    //LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] air_srv_discovery_callback, att_handle_role:%d att_handle_role_cccd:%x", 2, p_air_info->att_handle_role, p_air_info->att_handle_role_cccd);
                }
            }

            p_air_info->state = APP_LE_AUDIO_AIR_STATE_SRV_DISCOVERY_COMPLETE;
            app_le_audio_air_start_pre_action(event->conn_handle, BT_STATUS_SUCCESS);

            return;
        }
    }
    bt_gattc_discovery_continue(event->conn_handle);
}

static bt_status_t app_le_audio_air_save_connection_info(void *buff)
{
    uint8_t i;
    bt_status_t status = BT_STATUS_SUCCESS;
    if (NULL == buff) {
        LOG_MSGID_E(DONGLE_AIR, "app_le_audio_air_save_connection_info null", 0);
        status = BT_STATUS_FAIL;
        return status;
    }
    bt_gap_le_connection_ind_t *conn_ind = (bt_gap_le_connection_ind_t *)buff;
    for (i = 0; i < APP_LE_AUDIO_LINK_MAX_NUM; i++) {
        LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] save handle:%x,%x", 2, conn_ind->connection_handle, g_le_audio_air_info[i].conn_handle);
        /**< first connect, to save connection info. */
        if (BT_HANDLE_INVALID == g_le_audio_air_info[i].conn_handle || 0 == g_le_audio_air_info[i].conn_handle) {
            g_le_audio_air_info[i].conn_handle = conn_ind->connection_handle;
            memcpy(&g_le_audio_air_info[i].addr.addr, conn_ind->peer_addr.addr, sizeof(conn_ind->peer_addr.addr));
            //LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] save_connection, connection_handle:%x", 1, conn_ind->connection_handle);
            break;
            /**< Reconnect. */
        } else if (conn_ind->connection_handle == g_le_audio_air_info[i].conn_handle) {
            break;
        }
    }
    if (i == APP_LE_AUDIO_LINK_MAX_NUM) {
        status = BT_STATUS_OUT_OF_MEMORY;
    }
    return status;
}

static bt_status_t app_le_audio_air_delete_connection_info(void *buff)
{
    uint8_t i;
    bt_status_t status = BT_STATUS_SUCCESS;
    if (NULL == buff) {
        status = BT_STATUS_FAIL;
        return status;
    }
    bt_hci_evt_disconnect_complete_t *disconn_ind;
    disconn_ind = (bt_hci_evt_disconnect_complete_t *) buff;
    for (i = 0; i < APP_LE_AUDIO_LINK_MAX_NUM ; i++) {
        if (disconn_ind->connection_handle == g_le_audio_air_info[i].conn_handle) {
            LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] delete_connection, disconn handle:%x", 1, disconn_ind->connection_handle);
            app_le_audio_air_reset_info(i);
            break;
        }
    }
    if (i == APP_LE_AUDIO_LINK_MAX_NUM) {
        status = BT_STATUS_FAIL;
    }
    return status;
}

#ifndef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
static void app_le_audio_air_discovery_start(void *buff)
{
    bt_gattc_discovery_status_t status = BT_GATTC_DISCOVERY_STATUS_FAIL;
    if (NULL == buff) {
        LOG_MSGID_E(DONGLE_AIR, "[APP][AIR] discovery_start status null", 0);
        return ;
    }
    bt_gatt_exchange_mtu_rsp_t *conn_ind = (bt_gatt_exchange_mtu_rsp_t *)buff;
    status = bt_gattc_discovery_start(BT_GATTC_DISCOVERY_USER_AIR_SERVICE, conn_ind->connection_handle, false);
    if (BT_GATTC_DISCOVERY_STATUS_SUCCESS != status) {
        LOG_MSGID_I(DONGLE_AIR, "[APP][AIR] discovery_start status:%x, connection_handle:%x", 2, status ,conn_ind->connection_handle);
    }
}
#endif
static bt_status_t app_le_audio_air_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    if (NULL == buff) {
        return BT_STATUS_FAIL;
    }
    switch (msg) {
        case BT_GAP_LE_CONNECT_IND: {
            app_le_audio_air_save_connection_info(buff);
            break;
        }
        case BT_GAP_LE_DISCONNECT_IND: {
            app_le_audio_air_delete_connection_info(buff);
            break;
        }
        case BT_GATTC_READ_CHARC:
        case BT_GATTC_WRITE_CHARC:
        case BT_GATTC_CHARC_VALUE_NOTIFICATION: {
            app_le_audio_air_event_handler(msg, status, buff);
            break;
        case BT_GATTC_EXCHANGE_MTU:
#ifndef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
            app_le_audio_air_discovery_start(buff);
#endif
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

void ble_audio_air_main(void)
{
    /* Register AIR service discovery */
    uint8_t i = APP_LE_AUDIO_AIR_SRV_MAX_CHARC_NUMBER;
    memset(&g_le_audio_air_charc_discovery, 0, sizeof(app_le_audio_air_discovery_charc_t));

    while (i > 0) {
        i--;
        g_le_audio_air_charc_discovery.charc[i].descriptor_count = 1;
        g_le_audio_air_charc_discovery.charc[i].descriptor = &g_le_audio_air_charc_discovery.descrp[i];
    }
    ble_uuid_t air_srv_uuid = {
        .type = BLE_UUID_TYPE_128BIT,
        .uuid.uuid = {0x45, 0x4C, 0x42, 0x61, 0x68, 0x6F, 0x72, 0x69, 0x41, 0x03, 0xAB, 0x2D, 0x4D, 0x49, 0x52, 0x50}
    };
    memset(&g_le_audio_air_srv_discovery, 0, sizeof(bt_gattc_discovery_service_t));
    g_le_audio_air_srv_discovery.charateristics = g_le_audio_air_charc_discovery.charc;
    g_le_audio_air_srv_discovery.characteristic_count = APP_LE_AIR_SRV_MAX_CHARC_NUMBER;
    app_le_audio_air_init();
    //bt_gattc_discovery_service_register(&air_srv_uuid, &g_le_audio_air_srv_discovery, app_le_audio_air_srv_discovery_callback);

    bt_gattc_discovery_user_data_t discovery_data = {
            .uuid = air_srv_uuid,
            .need_cache = true,
            .srv_info = &g_le_audio_air_srv_discovery,
            .handler = app_le_audio_air_srv_discovery_callback
    };
    bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_AIR_SERVICE, &discovery_data);
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_SYSTEM),
                                          (void *)app_le_audio_air_event_callback);
}



