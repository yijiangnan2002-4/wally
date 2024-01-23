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

 /**
  * File: app_bolt_poc_race.c
  *
  * Description: This file is the race flow for bolt poc.
  *
  * Note: todo.
  *
  */

#include "app_bolt_poc_race.h"
#include "app_bolt_poc_bt_common.h"
#include "app_bolt_poc_utility.h"
#include "app_bolt_poc_data.h"
#include "apps_race_cmd_event.h"
#include "race_noti.h"
#include "apps_debug.h"


#define RACE_BOLT_HOGP_SCAN                  0x3060
#define RACE_BOLT_HOGP_CONNECT               0x3061
#define RACE_BOLT_HOGP_GET_SCAN_LIST       0x3062
#define RACE_BOLT_HOGP_GET_CONN_LIST         0x3063


#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

typedef struct {
    uint8_t start_scan;
    uint16_t timeout_seconds;
} PACKED app_bolt_poc_race_scan_cmd_t;

typedef struct {
    uint8_t status;
} PACKED app_bolt_poc_race_scan_rsp_t;

typedef struct {
    uint8_t connect;
    bt_addr_t addr;
} PACKED app_bolt_poc_race_connect_cmd_t;

typedef struct {
    uint8_t status;
    uint8_t connect;
    bt_addr_t addr;
} PACKED app_bolt_poc_race_connect_rsp_t;

typedef struct {
    bt_addr_t addr;
} PACKED app_bolt_poc_race_get_device_status_cmd_t;

typedef struct {
    uint8_t status;
    bt_addr_t addr;
    uint8_t device_id;
    uint8_t group_id;
    uint8_t role;
} PACKED app_bolt_poc_race_get_device_status_rsp_t;

typedef struct {
    bt_addr_t addr;
    uint8_t conn_status;    // 0: dis, 1: conn
    uint8_t group_id;
    uint8_t role;
} PACKED app_bolt_poc_race_device_status_item_t;

typedef struct {
    uint8_t status;
    app_bolt_poc_race_device_status_item_t devices_list[0];
} PACKED app_bolt_poc_race_get_device_list_rsp_t;

static void app_bolt_poc_race_scan(const app_bolt_poc_race_scan_cmd_t *data, uint8_t channel_id)
{
    RACE_ERRCODE race_ret = 0;

    if (data->start_scan) {
        app_bolt_poc_scan_list_init();
        app_bolt_poc_start_scan();
    } else {
        app_bolt_poc_stop_scan();
    }

    app_bolt_poc_race_scan_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                              (uint16_t)RACE_BOLT_HOGP_SCAN,
                                                              (uint16_t)(sizeof(app_bolt_poc_race_scan_rsp_t)),
                                                              channel_id);
    if (response != NULL) {
        response->status = 0;
        race_ret = race_noti_send(response, channel_id, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            APPS_LOG_MSGID_E(" [BOLT_POC] send rsp of RACE_BOLT_HOGP_SCAN error: 0x%X.", 1, race_ret);
            RACE_FreePacket((void *)response);
        }
        //APPS_LOG_MSGID_I(" [BOLT_POC] send rsp of RACE_LE_AUDIO_SCAN result: 0x%X.", 1, race_ret);
    }
}

static void app_bolt_poc_race_conn(const app_bolt_poc_race_connect_cmd_t *data, uint8_t channel_id)
{
    RACE_ERRCODE race_ret = 0;
    bt_status_t bt_status = BT_STATUS_SUCCESS;

    const uint8_t conn_state = data->connect;

    bt_addr_t conn_addr;
    memcpy(&conn_addr, &data->addr, sizeof(bt_addr_t));

    if (conn_state) {
        app_bolt_poc_start_new_conn(&conn_addr);
    } else {
        // Todo: Unpair
    }

    app_bolt_poc_race_connect_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                 (uint16_t)RACE_BOLT_HOGP_CONNECT,
                                                                 (uint16_t)(sizeof(app_bolt_poc_race_connect_rsp_t)),
                                                                 channel_id);
    if (response != NULL) {
        response->connect = conn_state;
        memcpy(&response->addr, &conn_addr, sizeof(bt_addr_t));
        response->status = (bt_status == BT_STATUS_SUCCESS) ? 0 : 0xFF;
        race_ret = race_noti_send(response, channel_id, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            APPS_LOG_MSGID_E(" [BOLT_POC] send rsp of RACE_BOLT_HOGP_CONNECT error: 0x%X.", 1, race_ret);
            RACE_FreePacket((void *)response);
        }
        //APPS_LOG_MSGID_I(" [BOLT_POC] send rsp of RACE_LE_AUDIO_CONNECT resule: 0x%X.", 1, race_ret);
    }
}

static void app_bolt_poc_race_get_scan_dev_list(uint8_t channel_id)
{
    uint8_t i = 0;
    uint8_t count = 0;
    RACE_ERRCODE race_ret = 0;

    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        if (app_bolt_poc_get_scan_addr(i) != NULL) {
            count++;
        }
    }

    app_bolt_poc_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                         (uint16_t)RACE_BOLT_HOGP_GET_SCAN_LIST,
                                                                         (uint16_t)(sizeof(app_bolt_poc_race_get_device_list_rsp_t) + count * sizeof(app_bolt_poc_race_device_status_item_t)),
                                                                         channel_id);
    if (response != NULL) {
        count = 0;

        response->status = 0;
        for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
            const bt_addr_t *bt_addr = app_bolt_poc_get_scan_addr(i);
            if (bt_addr != NULL) {
                memcpy(&response->devices_list[count].addr, bt_addr, sizeof(bt_addr_t));
                count++;
            }
        }

        race_ret = race_noti_send(response, channel_id, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            APPS_LOG_MSGID_E(" [BOLT_POC] send rsp of RACE_BOLT_HOGP_GET_SCAN_LIST error: 0x%X.", 1, race_ret);
            RACE_FreePacket((void *)response);
        }
        //APPS_LOG_MSGID_I(" [BOLT_POC] send rsp of RACE_LE_AUDIO_GET_DEVICE_LIST resule: 0x%X.", 1, race_ret);
    }
}

static void app_bolt_poc_race_get_conn_dev_list(uint8_t channel_id)
{
    uint8_t i = 0;
    uint8_t count = 0;
    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        if (app_bolt_poc_get_white_list_addr(i) != NULL) {
            count++;
        }
    }
    APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_race_get_conn_dev_list: count(0x%X)", 1, count);

    app_bolt_poc_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                         (uint16_t)RACE_BOLT_HOGP_GET_CONN_LIST,
                                                                         (uint16_t)(sizeof(app_bolt_poc_race_get_device_list_rsp_t) + count * sizeof(app_bolt_poc_race_device_status_item_t)),
                                                                         channel_id);
    if (response != NULL) {
        count = 0;

        response->status = 0;
        for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
            const bt_addr_t *bt_addr = app_bolt_poc_get_white_list_addr(i);
            if (bt_addr != NULL) {
                memcpy(&response->devices_list[count].addr, bt_addr, sizeof(bt_addr_t));
                response->devices_list[count].conn_status = app_bolt_poc_get_dev_conn_status(i);
                count++;
            }
        }

        RACE_ERRCODE ret = race_noti_send(response, channel_id, false);
        if (ret != RACE_ERRCODE_SUCCESS) {
            APPS_LOG_MSGID_E(" [BOLT_POC] send res of RACE_BOLT_HOGP_GET_CONN_LIST error: 0x%X.", 1, ret);
            RACE_FreePacket((void *)response);
        }
        //APPS_LOG_MSGID_I(" [BOLT_POC] send res of RACE_BOLT_HOGP_GET_CONN_LIST ret(0x%X), cnt(%d).", 2, race_ret, cnt);
    }
}


static uint8_t g_channel_id = 0;
void *app_bolt_poc_race_cmd_handler(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id)
{
    g_channel_id = channel_id;
    
    switch (p_race_package->hdr.id) {
        case RACE_BOLT_HOGP_SCAN:  {
                const app_bolt_poc_race_scan_cmd_t *scan_cmd_data = (const app_bolt_poc_race_scan_cmd_t *)&(p_race_package->payload);
                app_bolt_poc_race_scan(scan_cmd_data, channel_id);
            }
            break;
        case RACE_BOLT_HOGP_CONNECT:  {
                const app_bolt_poc_race_connect_cmd_t *race_conn_data = (const app_bolt_poc_race_connect_cmd_t *)&(p_race_package->payload);
                app_bolt_poc_race_conn(race_conn_data, channel_id);
            }
            break;
        case RACE_BOLT_HOGP_GET_SCAN_LIST: {
                app_bolt_poc_race_get_scan_dev_list(channel_id);
            }
            break;
        case RACE_BOLT_HOGP_GET_CONN_LIST: {
                app_bolt_poc_race_get_conn_dev_list(channel_id);
            }
            break;
        default:
            break;
    }

    return NULL;
}

void app_bolt_poc_race_report_dev_list()
{
    uint8_t i = 0;
    uint8_t count = 0;
    for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
        if (app_bolt_poc_get_white_list_addr(i) != NULL) {
            count++;
        }
    }
    APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_race_report_dev_list: count(0x%X)", 1, count);

    app_bolt_poc_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                                                         (uint16_t)RACE_BOLT_HOGP_GET_CONN_LIST,
                                                                         (uint16_t)(sizeof(app_bolt_poc_race_get_device_list_rsp_t) + count * sizeof(app_bolt_poc_race_device_status_item_t)),
                                                                         g_channel_id);
    if (response != NULL) {
        count = 0;

        response->status = 0;
        for (i = 0; i < APP_WHITE_LIST_NUM; i++) {
            const bt_addr_t *bt_addr = app_bolt_poc_get_white_list_addr(i);
            if (bt_addr != NULL) {
                memcpy(&response->devices_list[count].addr, bt_addr, sizeof(bt_addr_t));
                bool status = app_bolt_poc_get_dev_conn_status(i);
                response->devices_list[count].conn_status = status;
                count++;
            }
        }

        RACE_ERRCODE ret = race_noti_send(response, g_channel_id, false);
        if (ret != RACE_ERRCODE_SUCCESS) {
            APPS_LOG_MSGID_E(" [BOLT_POC] send noti of RACE_BOLT_HOGP_GET_CONN_LIST error: 0x%X.", 1, ret);
            RACE_FreePacket((void *)response);
        }
        //APPS_LOG_MSGID_I(" [BOLT_POC] send noti of RACE_BOLT_HOGP_GET_CONN_LIST ret(0x%X), cnt(%d).", 2, race_ret, cnt);
    }
}

