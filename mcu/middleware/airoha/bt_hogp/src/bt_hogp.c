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

#include "bt_hogp.h"
#include "FreeRTOS.h"
#include "bt_hid_service.h"
#include "bt_hogp_internal.h"
#include "bt_gap_le.h"
#include "bt_callback_manager.h"
#include "bt_utils.h"

log_create_module(BT_HOGP, PRINT_LEVEL_INFO);

typedef struct {
    bt_hogp_event_callback      event_callback;
    bt_hogp_information_t       hid_information;
    bt_hogp_protocol_mode_t     protocol_mode;
    bt_hogp_report_descriptor_t report_descriptor;
    uint16_t                    mtu_size;
} bt_hogp_context_t;

static bt_hogp_context_t hogp_context;

bt_hogp_report_group_t g_report_group[BT_HOGP_REPORT_INDEX_MAX] = {
    {BT_HOGP_INVALID_REPORT_ID, BT_HOGP_CONFIG_REPORT_1},  \
    {BT_HOGP_INVALID_REPORT_ID, BT_HOGP_CONFIG_REPORT_2},  \
    {BT_HOGP_INVALID_REPORT_ID, BT_HOGP_CONFIG_REPORT_3}
};


static bt_status_t bt_hogp_common_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    bt_hid_service_common_callback(msg, status, buff);
    switch (msg) {
        case BT_GAP_LE_CONNECT_IND: {
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_connection_ind_t *ind = (bt_gap_le_connection_ind_t *)buff;
                bt_hofp_connect_ind_t info;
                info.connection_handle = ind->connection_handle;
                info.addr = &ind->peer_addr;
                bt_hogp_notify_user_by_callback(BT_HOGP_EVENT_CONNECT_IND, &info, NULL);
            } else {
            }
        }
        break;

        case BT_GAP_LE_DISCONNECT_IND: {
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_disconnect_ind_t *ind = (bt_gap_le_disconnect_ind_t *)buff;
                bt_hofp_disconnect_ind_t info;
                info.connection_handle = ind->connection_handle;
                bt_hogp_notify_user_by_callback(BT_HOGP_EVENT_DISCONNECT_IND, &info, NULL);
            }
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

bt_hogp_report_group_t *bt_hogp_get_report_group_by_index(bt_hogp_report_index_t index)
{
    if (BT_HOGP_REPORT_INDEX_MAX <= index) {
        return NULL;
    }
    return &g_report_group[index];
}

bt_hogp_information_t *bt_hogp_get_hid_information(void)
{
    return &hogp_context.hid_information;
}

bt_hogp_report_descriptor_t *bt_hogp_get_report_descriptor(void)
{
    LOG_MSGID_I(BT_HOGP, "[HOGP]report descriptor length = %02x", 1, hogp_context.report_descriptor.length);
    return &hogp_context.report_descriptor;
}

bt_hogp_protocol_mode_t bt_hogp_get_protocol_mode(void)
{
    return hogp_context.protocol_mode;
}

static void bt_hogp_init_parameter_print(void)
{
    uint8_t i = 0;
    LOG_MSGID_I(BT_HOGP, "[HOGP] information:", 0);
    LOG_MSGID_I(BT_HOGP, "[HOGP] bcdhid:%02x", 1, hogp_context.hid_information.bcdhid);
    LOG_MSGID_I(BT_HOGP, "[HOGP] bcountrycode:%02x", 1, hogp_context.hid_information.bcountrycode);
    LOG_MSGID_I(BT_HOGP, "[HOGP] flags:%02x", 1, hogp_context.hid_information.flags);
    LOG_MSGID_I(BT_HOGP, "[HOGP] protocol mode:%02x", 1, hogp_context.protocol_mode);
    LOG_MSGID_I(BT_HOGP, "[HOGP] report descriptor length:%02x", 1, hogp_context.report_descriptor.length);
    LOG_MSGID_I(BT_HOGP, "[HOGP] report descriptor:", 0);
    for (i = 0; i < hogp_context.report_descriptor.length; i++) {
        LOG_MSGID_I(BT_HOGP, "%02x", 1,  hogp_context.report_descriptor.descriptor[i]);
    }
}

bt_status_t bt_hogp_init(bt_hogp_init_parameter_t *init_parameter, bt_hogp_event_callback callback)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (hogp_context.event_callback != NULL) {
        LOG_MSGID_I(BT_HOGP, "[HOGP] init fail, had init", 0);
        return BT_STATUS_FAIL;
    }
    memset(&hogp_context, 0, sizeof(bt_hogp_context_t));
    /* HOGP context init. */
    memcpy(&hogp_context.hid_information, &init_parameter->hid_information, sizeof(bt_hogp_information_t));
    hogp_context.report_descriptor.descriptor = bt_utils_memory_alloc(init_parameter->report_descriptor.length);
    if (hogp_context.report_descriptor.descriptor == NULL) {
        LOG_MSGID_I(BT_HOGP, "[HOGP] init fail, report descriptor alloc fail", 0);
        return BT_STATUS_FAIL;
    }
    hogp_context.report_descriptor.length = init_parameter->report_descriptor.length;
    if (init_parameter->report_descriptor.descriptor == NULL) {
        LOG_MSGID_I(BT_HOGP, "[HOGP] init fail, report descriptor is NULL", 0);
        return BT_STATUS_FAIL;
    }
    memcpy(hogp_context.report_descriptor.descriptor, init_parameter->report_descriptor.descriptor, hogp_context.report_descriptor.length);
    hogp_context.protocol_mode = init_parameter->mode;
    hogp_context.event_callback = callback;
    /* Register common callback. */
    status = bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_SYSTEM | MODULE_MASK_GAP, (void *)bt_hogp_common_event_callback);
    bt_hogp_init_parameter_print();
    bt_hid_service_init();
    LOG_MSGID_I(BT_HOGP, "[HOGP] init success", 0);
    return status;
}

bt_status_t bt_hogp_deinit(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    hogp_context.event_callback = NULL;
    if (hogp_context.report_descriptor.descriptor != NULL) {
        bt_utils_memory_free(hogp_context.report_descriptor.descriptor);
    }
    memset(&hogp_context, 0, sizeof(bt_hogp_context_t));
    bt_callback_manager_deregister_callback(bt_callback_type_app_event, (void *)bt_hogp_common_event_callback);
    return status;
}

bt_status_t bt_hogp_notify_user_by_callback(bt_hogp_event_t event, void *payload, void *output)
{
    if (hogp_context.event_callback == NULL) {
        LOG_MSGID_I(BT_HOGP, "[HOGP] event callback is NULL", 0);
        return BT_STATUS_FAIL;
    }
    return hogp_context.event_callback(event, payload, output);
}

bt_status_t bt_hogp_send_data(bt_handle_t connection_handle, bt_hogp_data_t *data)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (data->type) {
        case BT_HOGP_INPUT_REPORT: {
            status = bt_hid_service_report_input_notification(connection_handle, data->packet, data->packet_len);
        }
        break;
        default:
            break;
    }
    return status;
}
