/* Copyright Statement:
 *
 * (C) 2019 Airoha Technology Corp. All rights reserved.
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
 * the License Agreement ("Permitted User"). If you are not a Permitted User,
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
#include "app_mcp.h"

#include "apps_debug.h"
#include "app_lea_service_conn_mgr.h"

#include "ble_mcp_discovery.h"
#include "bt_gattc_discovery.h"

#ifdef AIR_LE_OTP_ENABLE
#include "ble_otp.h"
#include "ble_otp_discovery.h"
#include "ble_mcp.h"
#endif

/**************************************************************************************************
 * Define
**************************************************************************************************/
#define LOG_TAG     "[LEA][MCP]"

typedef struct {
    bt_gattc_discovery_characteristic_t  charc[BLE_MCS_MAX_CHARC_NUMBER];
    bt_gattc_discovery_descriptor_t      descrp[BLE_MCS_MAX_CHARC_NUMBER];
} app_mcp_discovery_charc_t;

#ifdef AIR_LE_OTP_ENABLE
typedef struct {
    bt_gattc_discovery_characteristic_t  charc[BLE_OTS_MAX_CHARC_NUMBER];
    bt_gattc_discovery_descriptor_t      descrp[BLE_OTS_MAX_CHARC_NUMBER];
} app_otp_discovery_charc_t;
#endif

/**************************************************************************************************
 * Variable
**************************************************************************************************/
static bt_gattc_discovery_service_t      app_lea_mcp_service;
static app_mcp_discovery_charc_t         app_lea_mcp_charc;

#ifdef AIR_LE_OTP_ENABLE
static bt_gattc_discovery_service_t      app_lea_otp_included_service;
static app_otp_discovery_charc_t         app_lea_otp_charc;
static uint8_t                           app_lea_mcs_index = 0;
static bool                              app_lea_otp_is_last = FALSE;
#endif

/**************************************************************************************************
 * Prototype
**************************************************************************************************/
extern bool bt_le_audio_sink_is_link_valid(bt_handle_t handle);

/**************************************************************************************************
 * Static function
**************************************************************************************************/
#ifdef AIR_LE_OTP_ENABLE
bt_status_t app_mcp_set_discovery_included_service_range(uint16_t start_handle, uint16_t end_handle)
{
    APPS_LOG_MSGID_I(LOG_TAG"[OTP] app_mcp_set_discovery_included_service_range: 0x%x 0x%x 0x%x 0x%x",
                     4, app_lea_otp_included_service.start_handle,
                     app_lea_otp_included_service.end_handle,
                     start_handle, end_handle);
    app_lea_otp_included_service.start_handle = start_handle;
    app_lea_otp_included_service.end_handle = end_handle;
    return BT_STATUS_SUCCESS;
}
#endif

static void app_le_audio_mcp_set_attribute_callback(bt_handle_t conn_handle)
{
#ifdef AIR_LE_OTP_ENABLE
    APPS_LOG_MSGID_I(LOG_TAG"[OTP] app_mcp_set_attribute_callback, conn_handle=0x%04X is_last=%d mcs_index=%d",
                     3, conn_handle, app_lea_otp_is_last, app_lea_mcs_index);

    if (app_lea_otp_is_last && BLE_MCP_SERVICE_INDEX_GMCS == app_lea_mcs_index) {
        ble_mcp_attribute_handle_range_t attr_handle_range;
        app_mcp_set_discovery_included_service_range(0, 0);

        if (conn_handle != BT_HANDLE_INVALID) {
            if (BT_STATUS_SUCCESS != ble_mcp_get_service_attribute_range(conn_handle, BLE_MCP_SERVICE_INDEX_GMCS, &attr_handle_range)) {
                APPS_LOG_MSGID_I(LOG_TAG"[OTP] app_mcp_set_attribute_callback, attribute_range fail", 0);
            } else {
                app_mcp_set_discovery_included_service_range(attr_handle_range.start_handle, attr_handle_range.end_handle);
            }
        }
    }
#else
    APPS_LOG_MSGID_I(LOG_TAG" set_attribute_callback, conn_handle=0x%04X", 1, conn_handle);
#endif

    bt_gattc_discovery_continue(conn_handle);
}

static bool app_le_audio_mcp_discovery_callback(bt_gattc_discovery_event_t *event, bool is_gmcs)
{
    ble_mcp_characteristic_t charc[BLE_MCS_MAX_CHARC_NUMBER];
    ble_mcp_set_service_attribute_parameter_t param;
    bt_status_t bt_status;
    uint32_t i = 0;

    if (NULL == event) {
        return FALSE;
    }

    if (!bt_le_audio_sink_is_link_valid(event->conn_handle)
        || (event->event_type != BT_GATTC_DISCOVERY_EVENT_COMPLETE && !event->last_instance)) {
        return FALSE;
    }

    APPS_LOG_MSGID_I(LOG_TAG" discovery_callback, conn_handle=0x%04X last_instance=%d event_type=%d is_gmcs=%d charc_num=%d",
                     5, event->conn_handle, event->last_instance, event->event_type,
                     is_gmcs, app_lea_mcp_service.char_count_found);

    memset(&param, 0, sizeof(ble_mcp_set_service_attribute_parameter_t));

    if (BT_GATTC_DISCOVERY_EVENT_COMPLETE == event->event_type) {
        if (0 != app_lea_mcp_service.char_count_found) {
            /* Fill MCP charc table */
            i = app_lea_mcp_service.char_count_found;
            while (i > 0) {
                i--;
                (charc + i)->uuid = app_lea_mcp_service.charateristics[i].char_uuid.uuid.uuid16;
                (charc + i)->value_handle = app_lea_mcp_service.charateristics[i].value_handle;
                (charc + i)->desc_handle = app_lea_mcp_service.charateristics[i].descriptor[0].handle;
            }
            param.charc = charc;
        }
        param.start_handle = app_lea_mcp_service.start_handle;
        param.end_handle = app_lea_mcp_service.end_handle;
        param.charc_num = app_lea_mcp_service.char_count_found;
    }

    extern void app_lea_conn_mgr_update_discovery_result(bt_handle_t conn_handle, bool tbs, bool general, uint8_t char_num);
    app_lea_conn_mgr_update_discovery_result(event->conn_handle, FALSE, is_gmcs, app_lea_mcp_service.char_count_found);

    param.is_gmcs = is_gmcs;
    param.is_complete = event->last_instance;
    param.callback = (ble_mcp_set_attribute_callback_t)app_le_audio_mcp_set_attribute_callback;

    bt_status = ble_mcp_set_service_attribute(event->conn_handle, &param);
    APPS_LOG_MSGID_I(LOG_TAG" discovery_callback, bt_status=0x%08X", 1, bt_status);

    memset(&app_lea_mcp_service, 0, sizeof(bt_gattc_discovery_service_t));
    app_lea_mcp_service.characteristic_count = BLE_MCS_MAX_CHARC_NUMBER;
    app_lea_mcp_service.charateristics = app_lea_mcp_charc.charc;

    return (bt_status == BT_STATUS_SUCCESS);
}

#ifdef AIR_LE_OTP_ENABLE
static void app_otp_set_attribute_callback_included_service(bt_handle_t conn_handle, uint8_t ots_index)
{
    APPS_LOG_MSGID_I(LOG_TAG"[OTP] set_attribute_callback, conn_handle=0x%04X ots_index=%d",
                     2, conn_handle, ots_index);
    ble_mcp_attribute_handle_range_t attr_handle_range;
    uint32_t num_mcs = 0;

    app_mcp_set_discovery_included_service_range(0, 0);
    if (conn_handle != BT_HANDLE_INVALID
        && BT_STATUS_SUCCESS == ble_mcp_get_service_number(conn_handle, &num_mcs)) {
        APPS_LOG_MSGID_I(LOG_TAG"[OTP] set_attribute_callback, num_mcs=%d app_lea_mcs_index=%d",
                         2, num_mcs, app_lea_mcs_index);
        ble_mcp_set_ots_index(conn_handle, app_lea_mcs_index, ots_index);
        if (num_mcs > 0) {
            if (BLE_MCP_SERVICE_INDEX_GMCS == app_lea_mcs_index) {
                num_mcs--;
            } else {
                if (num_mcs > 2 + app_lea_mcs_index) {
                    num_mcs -= (app_lea_mcs_index + 2);
                } else {
                    num_mcs = 0;
                }
            }
        }
        while (num_mcs > 0) {
            if (BLE_MCP_SERVICE_INDEX_GMCS == app_lea_mcs_index) {
                /* GMCS is already handled in app_mcp_discovery_gmcs_callback() */
                app_lea_mcs_index = 0;
                continue;
            } else {
                app_lea_mcs_index++;
            }
            if (BT_STATUS_SUCCESS == ble_mcp_get_service_attribute_range(conn_handle, app_lea_mcs_index, &attr_handle_range)) {
                app_mcp_set_discovery_included_service_range(attr_handle_range.start_handle, attr_handle_range.end_handle);
                break;
            }
            num_mcs--;
        }
    }

    bt_gattc_discovery_continue(conn_handle);
}

static bool app_otp_discovery_callback_included_service(bt_gattc_discovery_event_t *event)
{
    ble_otp_characteristic_t charc[BLE_OTS_MAX_CHARC_NUMBER];
    ble_otp_set_service_attribute_parameter_t param;
    bt_gattc_discovery_service_t *discovery_service = NULL;

    if (NULL == event || event->event_type != BT_GATTC_DISCOVERY_EVENT_COMPLETE) {
        APPS_LOG_MSGID_I(LOG_TAG"[OTP] callback_included_service, error", 0);
        return FALSE;
    }

    if (BT_GATTC_DISCOVERY_ERROR_SERVICE_NOT_FOUND == event->params.error_code) {
        app_otp_set_attribute_callback_included_service(event->conn_handle, 0xFF);
        return TRUE;
    }

    discovery_service = &app_lea_otp_included_service;
    APPS_LOG_MSGID_I(LOG_TAG"[OTP] callback_included_service, charc_num=%d", 1, discovery_service->char_count_found);
    memset(&param, 0, sizeof(ble_otp_set_service_attribute_parameter_t));
    if (0 != discovery_service->char_count_found) {
        /* Fill OTP charc table */
        int i = discovery_service->char_count_found;
        while (i > 0) {
            i--;
            (charc + i)->uuid = discovery_service->charateristics[i].char_uuid.uuid.uuid16;
            (charc + i)->value_handle = discovery_service->charateristics[i].value_handle;
            (charc + i)->desc_handle = discovery_service->charateristics[i].descriptor[0].handle;
        }
        param.charc = charc;
    }
    param.start_handle = discovery_service->start_handle;
    param.end_handle = discovery_service->end_handle;
    param.charc_num = discovery_service->char_count_found;

    param.is_complete = TRUE;
    param.callback = app_otp_set_attribute_callback_included_service;
    bt_status_t bt_status = ble_otp_set_service_attribute(event->conn_handle, &param);
    APPS_LOG_MSGID_I(LOG_TAG"[OTP] callback_included_service, status=0x%08X", 1, bt_status);
    return (bt_status == BT_STATUS_SUCCESS);
}

#endif

static void app_le_audio_mcp_discovery_gmcs_callback(bt_gattc_discovery_event_t *event)
{
#ifdef AIR_LE_OTP_ENABLE
    app_lea_otp_is_last = event->last_instance;
    app_lea_mcs_index = BLE_MCP_SERVICE_INDEX_GMCS;
#endif

    if (!app_le_audio_mcp_discovery_callback(event, TRUE)) {
        bt_gattc_discovery_continue((event != NULL ? event->conn_handle : BT_HANDLE_INVALID));
    }
}

static void app_le_audio_mcp_discovery_mcs_callback(bt_gattc_discovery_event_t *event)
{
#ifdef AIR_LE_OTP_ENABLE
    app_lea_otp_is_last = event->last_instance;
#endif

    if (!app_le_audio_mcp_discovery_callback(event, FALSE)) {
        bt_gattc_discovery_continue((event != NULL ? event->conn_handle : BT_HANDLE_INVALID));
    }
}

#ifdef AIR_LE_OTP_ENABLE
static void app_otp_discovery_ots_included_service_callback(bt_gattc_discovery_event_t *event)
{
    APPS_LOG_MSGID_I(LOG_TAG"[OTP] ots_included_service_callback, last_instance=%d", 1, event->last_instance);
    app_lea_otp_is_last = event->last_instance;
    if (!app_otp_discovery_callback_included_service(event)) {
        bt_gattc_discovery_continue((event != NULL ? event->conn_handle : BT_HANDLE_INVALID));
    }
}
#endif

/**************************************************************************************************
 * Public function
**************************************************************************************************/
void app_le_audio_mcp_init(void)
{
    memset(&app_lea_mcp_charc, 0, sizeof(app_mcp_discovery_charc_t));
    int i = BLE_MCS_MAX_CHARC_NUMBER;
    while (i > 0) {
        i--;
        app_lea_mcp_charc.charc[i].descriptor_count = 1;
        app_lea_mcp_charc.charc[i].descriptor = &app_lea_mcp_charc.descrp[i];
    }

    memset(&app_lea_mcp_service, 0, sizeof(bt_gattc_discovery_service_t));
    app_lea_mcp_service.characteristic_count = BLE_MCS_MAX_CHARC_NUMBER;
    app_lea_mcp_service.charateristics = app_lea_mcp_charc.charc;

    bt_gattc_discovery_user_data_t discovery_data = {
        .uuid.type = BLE_UUID_TYPE_16BIT,
        .uuid.uuid.uuid16 = BT_GATT_UUID16_GENERIC_MEDIA_CONTROL_SERVICE,
        .need_cache = TRUE,
        .srv_info = &app_lea_mcp_service,
        .handler = app_le_audio_mcp_discovery_gmcs_callback
    };
    bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_LE_AUDIO, &discovery_data);

    discovery_data.uuid.uuid.uuid16 = BT_GATT_UUID16_MEDIA_CONTROL_SERVICE;
    discovery_data.srv_info = &app_lea_mcp_service;
    discovery_data.handler = app_le_audio_mcp_discovery_mcs_callback;
    bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_LE_AUDIO, &discovery_data);

#ifdef AIR_LE_OTP_ENABLE
    if (BT_STATUS_SUCCESS != ble_otp_init(APP_LEA_MAX_CONN_NUM)) {
        APPS_LOG_MSGID_E(LOG_TAG"[OTP] ble_otp_init, error", 0);
        return;
    }

    memset(&app_lea_otp_charc, 0, sizeof(app_otp_discovery_charc_t));

    i = BLE_OTS_MAX_CHARC_NUMBER;
    while (i > 0) {
        i--;
        app_lea_otp_charc.charc[i].descriptor_count = 1;
        app_lea_otp_charc.charc[i].descriptor = &app_lea_otp_charc.descrp[i];
    }

    /* register for included service */
    memset(&app_lea_otp_included_service, 0, sizeof(bt_gattc_discovery_service_t));
    app_lea_otp_included_service.characteristic_count = BLE_OTS_MAX_CHARC_NUMBER;
    app_lea_otp_included_service.charateristics = app_lea_otp_charc.charc;

    discovery_data.uuid.uuid.uuid16 = BLE_UUID16_OBJECT_TRANSFER_SERVICE;
    discovery_data.srv_info = &app_lea_otp_included_service;
    discovery_data.handler = app_otp_discovery_ots_included_service_callback;
    bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_LE_AUDIO, &discovery_data);
#endif
}

#endif

