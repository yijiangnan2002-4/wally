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

#include "app_ccp.h"

#include "apps_debug.h"

#include "ble_ccp_discovery.h"
#include "bt_gattc_discovery.h"

#include "FreeRTOS.h"

/**************************************************************************************************
 * Define
**************************************************************************************************/
#define LOG_TAG     "[LEA][CCP]"

typedef struct {
    bt_gattc_discovery_characteristic_t charc[BLE_TBS_MAX_CHARC_NUMBER];
    bt_gattc_discovery_descriptor_t     descrp[BLE_TBS_MAX_CHARC_NUMBER];
} app_ccp_discovery_charc_t;

/**************************************************************************************************
 * Variable
**************************************************************************************************/
static bt_gattc_discovery_service_t      app_lea_ccp_service;
static app_ccp_discovery_charc_t         app_lea_ccp_charc;

/**************************************************************************************************
* Prototype
**************************************************************************************************/
extern bool bt_le_audio_sink_is_link_valid(bt_handle_t handle);

/**************************************************************************************************
 * Static function
**************************************************************************************************/
static void app_le_audio_ccp_set_attribute_callback(bt_handle_t conn_handle)
{
    APPS_LOG_MSGID_I(LOG_TAG" set_attribute_callback, conn_handle=0x%04X", 1, conn_handle);
    bt_gattc_discovery_continue(conn_handle);
}

static bool app_le_audio_ccp_discovery_callback(bt_gattc_discovery_event_t *event, bool is_gtbs)
{
    bt_gattc_discovery_characteristic_t *found = NULL;
    ble_ccp_characteristic_t *charc_list = NULL;
    ble_ccp_characteristic_t *charc = NULL;
    ble_ccp_set_service_attribute_parameter_t param = {0};
    bt_status_t bt_status = BT_STATUS_SUCCESS;
    uint8_t i = 0;

    if (NULL == event) {
        return FALSE;
    }

    if (!bt_le_audio_sink_is_link_valid(event->conn_handle)
        || (event->event_type != BT_GATTC_DISCOVERY_EVENT_COMPLETE && !event->last_instance)) {
        APPS_LOG_MSGID_E(LOG_TAG" discovery_callback, error", 0);
        return FALSE;
    }

    APPS_LOG_MSGID_I(LOG_TAG" discovery_callback, conn_handle=0x%04X last_instance=%d event_type=%d is_gtbs=%d charc_num=%d",
                     5, event->conn_handle, event->last_instance, event->event_type, is_gtbs, app_lea_ccp_service.char_count_found);

    memset(&param, 0, sizeof(ble_ccp_set_service_attribute_parameter_t));

    if (BT_GATTC_DISCOVERY_EVENT_COMPLETE == event->event_type) {
        uint8_t charc_num = 0;
        charc_num = app_lea_ccp_service.char_count_found;
        if (0 != charc_num) {
            /* 1. Malloc TBS charc list */
            if (NULL != (charc_list = (ble_ccp_characteristic_t *)pvPortMalloc(sizeof(ble_ccp_characteristic_t) * charc_num))) {
                /* 2. Fill TBS charc list */
                for (i = 0; i < charc_num; i++) {
                    found = &app_lea_ccp_service.charateristics[i];
                    charc = &charc_list[i];
                    charc->uuid = found->char_uuid.uuid.uuid16;
                    charc->value_handle = found->value_handle;
                    charc->desc_handle = 0;

                    if (found->descr_count_found) {
                        charc->desc_handle = found->descriptor[0].handle;
                    }
                }
                param.charc = charc_list;
            }
        }

        param.start_handle = app_lea_ccp_service.start_handle;
        param.end_handle = app_lea_ccp_service.end_handle;
        param.charc_num = charc_num;
    }

    extern void app_lea_conn_mgr_update_discovery_result(bt_handle_t conn_handle, bool tbs, bool general, uint8_t char_num);
    app_lea_conn_mgr_update_discovery_result(event->conn_handle, TRUE, is_gtbs, app_lea_ccp_service.char_count_found);

    param.is_gtbs = is_gtbs;
    param.is_complete = event->last_instance;
    param.callback = (ble_ccp_set_attribute_callback_t)app_le_audio_ccp_set_attribute_callback;
    bt_status = ble_ccp_set_service_attribute(event->conn_handle, &param);
    APPS_LOG_MSGID_I(LOG_TAG" discovery_callback, bt_status=0x%08X", 1, bt_status);

    if (charc_list != NULL) {
        vPortFree(charc_list);
    }

    memset(&app_lea_ccp_service, 0, sizeof(bt_gattc_discovery_service_t));
    app_lea_ccp_service.characteristic_count = BLE_TBS_MAX_CHARC_NUMBER;
    app_lea_ccp_service.charateristics = app_lea_ccp_charc.charc;

    return (bt_status == BT_STATUS_SUCCESS);
}

static void app_le_audio_ccp_discover_gtbs_callback(bt_gattc_discovery_event_t *event)
{
    if (!app_le_audio_ccp_discovery_callback(event, TRUE)) {
        bt_gattc_discovery_continue((event != NULL ? event->conn_handle : BT_HANDLE_INVALID));
    }
}

static void app_le_audio_ccp_discover_tbs_callback(bt_gattc_discovery_event_t *event)
{
    if (!app_le_audio_ccp_discovery_callback(event, FALSE)) {
        bt_gattc_discovery_continue((event != NULL ? event->conn_handle : BT_HANDLE_INVALID));
    }
}

/**************************************************************************************************
 * Public function
**************************************************************************************************/
void app_le_audio_ccp_init(void)
{
    memset(&app_lea_ccp_charc, 0, sizeof(app_ccp_discovery_charc_t));
    for (int i = 0; i < BLE_TBS_MAX_CHARC_NUMBER; i++) {
        app_lea_ccp_charc.charc[i].descriptor_count = 1;
        app_lea_ccp_charc.charc[i].descriptor = &app_lea_ccp_charc.descrp[i];
    }

    memset(&app_lea_ccp_service, 0, sizeof(bt_gattc_discovery_service_t));
    app_lea_ccp_service.characteristic_count = BLE_TBS_MAX_CHARC_NUMBER;
    app_lea_ccp_service.charateristics = app_lea_ccp_charc.charc;

    bt_gattc_discovery_user_data_t discovery_data = {
        .uuid.type = BLE_UUID_TYPE_16BIT,
        .uuid.uuid.uuid16 = BT_SIG_UUID16_GENERIC_TELEPHONE_BEARER_SERVICE,
        .need_cache = TRUE,
        .srv_info = &app_lea_ccp_service,
        .handler = app_le_audio_ccp_discover_gtbs_callback
    };
    bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_LE_AUDIO, &discovery_data);

    discovery_data.uuid.uuid.uuid16 = BT_SIG_UUID16_TELEPHONE_BEARER_SERVICE;
    discovery_data.srv_info = &app_lea_ccp_service;
    discovery_data.handler = app_le_audio_ccp_discover_tbs_callback;
    bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_LE_AUDIO, &discovery_data);
}

#endif
