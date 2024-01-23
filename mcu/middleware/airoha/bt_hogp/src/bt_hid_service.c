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
#include "bt_gap_le.h"
#include "bt_gatts.h"
#include "bt_gattc.h"
#include "bt_type.h"
#include "FreeRTOS.h"
#include "bt_hid_service.h"
#include "bt_hogp_internal.h"
#include "bt_device_manager_le.h"
#ifdef MTK_NVDM_ENABLE
#include "nvkey_id_list.h"
#include "nvdm.h"
#include "nvkey.h"
#endif
#include "bt_gap_le_service.h"
#include "bt_utils.h"

log_create_module(BT_HID_SRV, PRINT_LEVEL_INFO);

#define BT_HID_SRV_INVAILD_HANDLE 0x0000

typedef uint16_t bt_hid_service_cccd_t;

#define HID_SERVICE_UUID                       (0x1812)
#define HID_INFORMATION_UUID                   (0x2A4A)
#define HID_BK_INPUT_UUID                      (0x2A22)
#define HID_BK_OUTPUT_UUID                     (0x2A32)
#define HID_REPORT_MAP_UUID                    (0x2A4B)
#define HID_REPORT_INPUT_UUID                  (0x2A4D)
#define HID_REPORT_OUTPUT_UUID                 (0x2A4D)
#define HID_REPORT_FEATURE_UUID                (0x2A4D)
#define HID_CONTROL_POINT_UUID                 (0x2A4C)
#define HID_PROTOCOL_MODE_UUID                 (0x2A4E)

#define BT_HID_SERVICE_REPORT_REFERENCE             (0x2908)

static const bt_uuid_t HID_INFORMATION_MAP_UUID128 =
    BT_UUID_INIT_WITH_UUID16(HID_INFORMATION_UUID);

static const bt_uuid_t HID_BK_INPUT_MAP_UUID128 =
    BT_UUID_INIT_WITH_UUID16(HID_BK_INPUT_UUID);

static const bt_uuid_t HID_BK_OUTPUT_MAP_UUID128 =
    BT_UUID_INIT_WITH_UUID16(HID_BK_OUTPUT_UUID);

static const bt_uuid_t HID_REPORT_MAP_MAP_UUID128 =
    BT_UUID_INIT_WITH_UUID16(HID_REPORT_MAP_UUID);

static const bt_uuid_t HID_REPORT_INPUT_MAP_UUID128 =
    BT_UUID_INIT_WITH_UUID16(HID_REPORT_INPUT_UUID);

/*
static const bt_uuid_t HID_REPORT_OUTPUT_MAP_UUID128 =
    BT_UUID_INIT_WITH_UUID16(HID_REPORT_FEATURE_UUID);
*/

static const bt_uuid_t HID_REPORT_FEATURE_MAP_UUID128 =
    BT_UUID_INIT_WITH_UUID16(HID_REPORT_FEATURE_UUID);

static const bt_uuid_t HID_CONTROL_POINT_MAP_UUID128 =
    BT_UUID_INIT_WITH_UUID16(HID_CONTROL_POINT_UUID);

static const bt_uuid_t HID_PROTOCOL_MODE_MAP_UUID128 =
    BT_UUID_INIT_WITH_UUID16(HID_PROTOCOL_MODE_UUID);

static const bt_uuid_t BT_HID_SERVICE_REPORT_REFERENCE_MAP_UUID128 =
    BT_UUID_INIT_WITH_UUID16(BT_HID_SERVICE_REPORT_REFERENCE);


#define BT_GATTS_NEW_REPORT_REFERENCE(name, _perm, _callback)               \
    static const bt_gatts_client_characteristic_config_t name = {           \
        .rec_hdr.uuid_ptr = &BT_HID_SERVICE_REPORT_REFERENCE_MAP_UUID128,   \
        .rec_hdr.perm = _perm,                                              \
        .rec_hdr.value_len = 0,                                             \
        .callback = _callback                                               \
    }


#define HID_INFORMATION_VALUE_HANDLE           (0x0202)
#define HID_BK_INPUT_HANDLE                    (0x0204)
#define HID_BK_OUTPUT_HANDLE                   (0x0207)
#define HID_REPORT_MAP_HANDLE                  (0x0209)
#define HID_REPORT_INPUT_HANDLE                (0x020B)
#define HID_REPORT_OUTPUT_HANDLE               (0x020F)
#define HID_REPORT_FEATURE_HANDLE              (0x0212)
#define HID_CONTROL_POINT_HANDLE               (0x0215)
#define HID_PROTOCOL_MODE_HANDLE               (0x0217)


#define BT_HID_SERVICE_NOTIFICATION_ENABLE_MASK   0x0001
#define BT_HID_SERVICE_INDICATION_ENABLE_MASK     0x0002

#define BT_HID_SERVICE_FLAG_USING                 0x0001
#define BT_HID_SERVICE_FLAG_BONDED                0x0002
typedef uint16_t bt_hid_service_flag_t;

BT_PACKED(
typedef struct {
    bt_addr_t                remote_address;
    bt_hid_service_flag_t    flag;
    bt_hid_service_cccd_t    bk_input_cccd;
    bt_hid_service_cccd_t    report_input_cccd;
    uint8_t                  reserve[20];
}) bt_hid_service_nvdm_info_t;

typedef struct {
    bt_handle_t              connection_handle;
    bt_addr_t                remote_address;
    bt_hid_service_flag_t    flag;
    bt_hid_service_cccd_t    bk_input_cccd;
    bt_hid_service_cccd_t    report_input_cccd;
} bt_hid_service_context_t;

static bt_hid_service_context_t hid_context[BT_HID_SERVICE_CONTEXT_MAX] = {{0}};


static uint32_t bt_hid_get_report_handle(uint16_t handle, void *data, uint16_t size,
                                         uint16_t offset, bt_hogp_report_group_t *report_group);
static uint32_t bt_hid_get_report_id_handle(uint16_t handle, void *data, uint16_t size, bt_hogp_report_index_t index);
static uint32_t bt_hid_set_report_handle(uint16_t handle, void *data, uint16_t size, bt_hogp_report_group_t *report_group);

static bt_hid_service_context_t *bt_hid_get_free_context(void)
{
    uint32_t i = 0;
    for (i = 0; i < BT_HID_SERVICE_CONTEXT_MAX; i++) {
        if (!(hid_context[i].flag & BT_HID_SERVICE_FLAG_USING)) {
            LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] get free context index = %02x", 1, i);
            hid_context[i].flag |= BT_HID_SERVICE_FLAG_USING;
            return &hid_context[i];
        }
    }
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] cannot get free context", 0);
    return NULL;
}

static uint8_t bt_hid_find_context_index(bt_hid_service_context_t *context)
{
    uint32_t i = 0;
    for (i = 0; i < BT_HID_SERVICE_CONTEXT_MAX; i++) {
        if (&hid_context[i] == context) {
            return i;
        }
    }
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] not find context index by %02x", 1, context);
    return 0xFF;
}

static bt_hid_service_context_t *bt_hid_find_context_by_handle(bt_handle_t connection_handle)
{
    uint32_t i = 0;
    for (i = 0; i < BT_HID_SERVICE_CONTEXT_MAX; i++) {
        if (hid_context[i].connection_handle == connection_handle) {
            LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] find context index = %02x by handle = %02x", 2, i, connection_handle);
            return &hid_context[i];
        }
    }
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] not find context by handle = %02x", 1, connection_handle);
    return NULL;
}

static bt_hid_service_context_t *bt_hid_get_context_by_address(bt_addr_t *remote_address)
{
    uint32_t i = 0;
    bt_hid_service_context_t *context = NULL;
    for (i = 0; i < BT_HID_SERVICE_CONTEXT_MAX; i++) {
        if ((hid_context[i].flag & BT_HID_SERVICE_FLAG_USING) && (memcmp(&hid_context[i].remote_address, remote_address, sizeof(bt_addr_t)) == 0)) {
            LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] get context index = %02x", 1, i);
            return &hid_context[i];
        }
    }
    context = bt_hid_get_free_context();
    return context;
}

static bt_hid_service_context_t *bt_hid_find_context_by_address(bt_addr_t *remote_address)
{
    uint32_t i = 0;
    bt_hid_service_context_t *context = NULL;
    for (i = 0; i < BT_HID_SERVICE_CONTEXT_MAX; i++) {
        if ((hid_context[i].flag & BT_HID_SERVICE_FLAG_USING) && (memcmp(&hid_context[i].remote_address, remote_address, sizeof(bt_addr_t)) == 0)) {
            LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] find context index = %02x", 1, i);
            return &hid_context[i];
        }
    }
    return context;
}



static uint32_t bt_hid_service_information_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] information value callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    if (rw == BT_GATTS_CALLBACK_READ) {
        if (size == 0) {
            return sizeof(bt_hogp_information_t);
        } else {
            bt_utils_assert(data);
            bt_hogp_information_t *information = bt_hogp_get_hid_information();
            memcpy(data, information, sizeof(bt_hogp_information_t));
            return sizeof(bt_hogp_information_t);
        }
    }
    return 0;
}

static uint32_t bt_hid_service_bk_input_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] bk input value callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, data);
    return 0;
}

static uint32_t bt_hid_service_bk_input_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] bk input client config callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    bt_hid_service_context_t *context = bt_hid_find_context_by_handle(handle);
    if (context == NULL) {
        return 0;
    }

    if (rw == BT_GATTS_CALLBACK_WRITE) {
        if (size == sizeof(bt_hid_service_cccd_t)) {
            context->bk_input_cccd = *(bt_hid_service_cccd_t *)data;
        }
    } else if (rw == BT_GATTS_CALLBACK_READ) {
        if (size == sizeof(bt_hid_service_cccd_t)) {
            memcpy(data, &context->bk_input_cccd, sizeof(bt_hid_service_cccd_t));
        }
    }
    return sizeof(bt_hid_service_cccd_t);
}

static uint32_t bt_hid_service_bk_output_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] bk output value callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    return 0;
}

static uint32_t bt_hid_service_report_map_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] report map value callback, rw = %02x, handle = %02x, size = %02x, offset = %02x", 4,
                rw, handle, size, offset);
    bt_hogp_report_descriptor_t *report_descritor = bt_hogp_get_report_descriptor();
    if (rw == BT_GATTS_CALLBACK_READ) {
        if (size == 0) {
            return report_descritor->length;
        } else {
            memcpy(data, report_descritor->descriptor + offset, size);
            return size;
        }
    }
    return 0;
}

static uint32_t bt_hid_service_report_input_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] report input value callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    bt_hogp_report_group_t *report_group = bt_hogp_get_report_group_by_index(BT_HOGP_REPORT_1_INDEX);
    if (rw == BT_GATTS_CALLBACK_READ) {
        return bt_hid_get_report_handle(handle, data, size, offset, report_group);
    } else if (rw == BT_GATTS_CALLBACK_WRITE) {
        return bt_hid_set_report_handle(handle, data, size, report_group);
    }
    return 0;
}

static uint32_t bt_hid_service_report_input_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] input client config value callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    bt_hid_service_context_t *context = bt_hid_find_context_by_handle(handle);
    if (context == NULL) {
        return 0;
    }
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] input config client config value  = %02x", 1, context->report_input_cccd);
    if (rw == BT_GATTS_CALLBACK_WRITE) {
        if (size == sizeof(bt_hid_service_cccd_t)) {
            context->report_input_cccd = *(bt_hid_service_cccd_t *)data;
        }
    } else if (rw == BT_GATTS_CALLBACK_READ) {
        if (size == sizeof(bt_hid_service_cccd_t)) {
            memcpy(data, &context->report_input_cccd, sizeof(bt_hid_service_cccd_t));
        }
    }
    return sizeof(bt_hid_service_cccd_t);
}

static uint32_t bt_hid_service_report_input_reference_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] report input reference callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    if (rw == BT_GATTS_CALLBACK_READ) {
        return bt_hid_get_report_id_handle(handle, data, size, BT_HOGP_REPORT_1_INDEX);
    }
    return 0;
}

static uint32_t bt_hid_service_report_2_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] report 2 value callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    bt_hogp_report_group_t *report_group = bt_hogp_get_report_group_by_index(BT_HOGP_REPORT_2_INDEX);
    if (rw == BT_GATTS_CALLBACK_READ) {
        return bt_hid_get_report_handle(handle, data, size, offset, report_group);
    } else if (rw == BT_GATTS_CALLBACK_WRITE) {
        return bt_hid_set_report_handle(handle, data, size, report_group);
    }
    return 0;
}

static uint32_t bt_hid_service_report_2_reference_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] report 2 reference callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    if (rw == BT_GATTS_CALLBACK_READ) {
        return bt_hid_get_report_id_handle(handle, data, size, BT_HOGP_REPORT_2_INDEX);
    }
    return 0;
}

static uint32_t bt_hid_service_report_3_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] report 3 value callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    bt_hogp_report_group_t *report_group = bt_hogp_get_report_group_by_index(BT_HOGP_REPORT_3_INDEX);
    if (rw == BT_GATTS_CALLBACK_READ) {
        return bt_hid_get_report_handle(handle, data, size, offset, report_group);
    } else if (rw == BT_GATTS_CALLBACK_WRITE) {
        return bt_hid_set_report_handle(handle, data, size, report_group);
    }
    return 0;
}

static uint32_t bt_hid_service_report_3_reference_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] report 3 reference callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    if (rw == BT_GATTS_CALLBACK_READ) {
        return bt_hid_get_report_id_handle(handle, data, size, BT_HOGP_REPORT_3_INDEX);
    }
    return 0;
}

static uint32_t bt_hid_service_control_point_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] control point value callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    if (rw == BT_GATTS_CALLBACK_WRITE) {
        bt_hogp_control_ind_t ind;
        ind.connection_handle = handle;
        ind.command = *(bt_hogp_control_command_t *)data;
        bt_hogp_notify_user_by_callback(BT_HOGP_EVENT_CONTORL_IND, &ind, NULL);
    }
    return size;
}

static uint32_t bt_hid_service_protocol_mode_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] protocol mode value callback, rw = %02x, handle = %02x, size = %02x", 3,
                rw, handle, size);
    if (rw == BT_GATTS_CALLBACK_READ) {
        if (size == 0) {
            return sizeof(bt_hogp_protocol_mode_t);
        } else if (size == sizeof(bt_hogp_protocol_mode_t)) {
            bt_hogp_protocol_mode_t mode = bt_hogp_get_protocol_mode();
            *(uint8_t *)data = mode;
            return sizeof(bt_hogp_protocol_mode_t);
        }
    } else if (rw == BT_GATTS_CALLBACK_WRITE) {
    }
    return 0;
}

BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_hid_primary_service, HID_SERVICE_UUID);

BT_GATTS_NEW_CHARC_16(bt_hid_information_charc, BT_GATT_CHARC_PROP_READ, HID_INFORMATION_VALUE_HANDLE, HID_INFORMATION_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_hid_information_value, HID_INFORMATION_MAP_UUID128,
                                  BT_GATTS_REC_PERM_READABLE, bt_hid_service_information_value_callback);

BT_GATTS_NEW_CHARC_16(bt_hid_bk_input_charc, BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_WRITE, HID_BK_INPUT_HANDLE, HID_BK_INPUT_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_hid_bk_input_value, HID_BK_INPUT_MAP_UUID128,
                                  BT_GATTS_REC_PERM_WRITABLE | BT_GATTS_REC_PERM_READABLE, bt_hid_service_bk_input_value_callback);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(bt_hid_bk_input_client_config, BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, bt_hid_service_bk_input_client_config_callback);

BT_GATTS_NEW_CHARC_16(bt_hid_bk_output_charc, BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_WRITE | BT_GATT_CHARC_PROP_WRITE_WITHOUT_RSP,
                      HID_BK_OUTPUT_HANDLE, HID_BK_OUTPUT_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_hid_bk_output_value, HID_BK_OUTPUT_MAP_UUID128,
                                  BT_GATTS_REC_PERM_WRITABLE | BT_GATTS_REC_PERM_READABLE, bt_hid_service_bk_output_value_callback);

/* Report map characteristic. */
BT_GATTS_NEW_CHARC_16(bt_hid_report_map_charc, BT_GATT_CHARC_PROP_READ, HID_REPORT_MAP_HANDLE, HID_REPORT_MAP_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_hid_report_map_value, HID_REPORT_MAP_MAP_UUID128,
                                  BT_GATTS_REC_PERM_READABLE, bt_hid_service_report_map_value_callback);


BT_GATTS_NEW_CHARC_16(bt_hid_report_input_charc, BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_NOTIFY,
                      HID_REPORT_INPUT_HANDLE, HID_REPORT_INPUT_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_hid_report_input_value, HID_REPORT_INPUT_MAP_UUID128,
                                  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, bt_hid_service_report_input_value_callback);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(bt_hid_report_input_client_config,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, bt_hid_service_report_input_client_config_callback);
BT_GATTS_NEW_REPORT_REFERENCE(bt_hid_report_input_reference,
                              BT_GATTS_REC_PERM_READABLE, bt_hid_service_report_input_reference_callback);

BT_GATTS_NEW_CHARC_16(bt_hid_report_2_charc, BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_WRITE | BT_GATT_CHARC_PROP_WRITE_WITHOUT_RSP,
                      HID_REPORT_OUTPUT_HANDLE, HID_REPORT_FEATURE_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_hid_report_2_value, HID_REPORT_FEATURE_MAP_UUID128,
                                  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, bt_hid_service_report_2_value_callback);
BT_GATTS_NEW_REPORT_REFERENCE(bt_hid_report_2_reference,
                              BT_GATTS_REC_PERM_READABLE, bt_hid_service_report_2_reference_callback);

BT_GATTS_NEW_CHARC_16(bt_hid_report_3_charc, BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_WRITE,
                      HID_REPORT_FEATURE_HANDLE, HID_REPORT_FEATURE_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_hid_report_3_value, HID_REPORT_FEATURE_MAP_UUID128,
                                  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, bt_hid_service_report_3_value_callback);
BT_GATTS_NEW_REPORT_REFERENCE(bt_hid_report_3_reference,
                              BT_GATTS_REC_PERM_READABLE, bt_hid_service_report_3_reference_callback);

/* HID control point characteristic */
BT_GATTS_NEW_CHARC_16(bt_hid_control_point_charc, BT_GATT_CHARC_PROP_WRITE_WITHOUT_RSP,
                      HID_CONTROL_POINT_HANDLE, HID_CONTROL_POINT_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_hid_control_point_value, HID_CONTROL_POINT_MAP_UUID128,
                                  BT_GATTS_REC_PERM_WRITABLE, bt_hid_service_control_point_value_callback);


BT_GATTS_NEW_CHARC_16(bt_hid_protocol_mode_charc, BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_WRITE_WITHOUT_RSP,
                      HID_PROTOCOL_MODE_HANDLE, HID_PROTOCOL_MODE_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_hid_protocol_mode_value, HID_PROTOCOL_MODE_MAP_UUID128,
                                  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, bt_hid_service_protocol_mode_value_callback);

static const bt_gatts_service_rec_t *bt_hid_service_rec[] = {
    (const bt_gatts_service_rec_t *) &bt_hid_primary_service,           /* handle:0x0200. */

    (const bt_gatts_service_rec_t *) &bt_hid_information_charc,         /* handle:0x0201. */
    (const bt_gatts_service_rec_t *) &bt_hid_information_value,         /* handle:0x0202. */

    (const bt_gatts_service_rec_t *) &bt_hid_bk_input_charc,            /* handle:0x0203. */
    (const bt_gatts_service_rec_t *) &bt_hid_bk_input_value,            /* handle:0x0204. */
    (const bt_gatts_service_rec_t *) &bt_hid_bk_input_client_config,    /* handle:0x0205. */

    (const bt_gatts_service_rec_t *) &bt_hid_bk_output_charc,           /* handle:0x0206. */
    (const bt_gatts_service_rec_t *) &bt_hid_bk_output_value,           /* handle:0x0207. */

    (const bt_gatts_service_rec_t *) &bt_hid_report_map_charc,          /* handle:0x0208. */
    (const bt_gatts_service_rec_t *) &bt_hid_report_map_value,          /* handle:0x0209. */

    (const bt_gatts_service_rec_t *) &bt_hid_report_input_charc,        /* handle:0x020A. */
    (const bt_gatts_service_rec_t *) &bt_hid_report_input_value,        /* handle:0x020B. */
    (const bt_gatts_service_rec_t *) &bt_hid_report_input_client_config,/* handle:0x020C. */
    (const bt_gatts_service_rec_t *) &bt_hid_report_input_reference,    /* handle:0x020D. */

    (const bt_gatts_service_rec_t *) &bt_hid_report_2_charc,       /* handle:0x020E. */
    (const bt_gatts_service_rec_t *) &bt_hid_report_2_value,       /* handle:0x020F. */
    (const bt_gatts_service_rec_t *) &bt_hid_report_2_reference,   /* handle:0x0210. */

    (const bt_gatts_service_rec_t *) &bt_hid_report_3_charc,            /* handle:0x0211. */
    (const bt_gatts_service_rec_t *) &bt_hid_report_3_value,            /* handle:0x0212. */
    (const bt_gatts_service_rec_t *) &bt_hid_report_3_reference,        /* handle:0x0213. */

    (const bt_gatts_service_rec_t *) &bt_hid_control_point_charc,       /* handle:0x0214. */
    (const bt_gatts_service_rec_t *) &bt_hid_control_point_value,       /* handle:0x0215. */

    (const bt_gatts_service_rec_t *) &bt_hid_protocol_mode_charc,       /* handle:0x0216. */
    (const bt_gatts_service_rec_t *) &bt_hid_protocol_mode_value,       /* handle:0x0217. */
};

const bt_gatts_service_t bt_hid_service = {
    .starting_handle = 0x0200,
    .ending_handle = 0x0217,
    .required_encryption_key_size = 16,
    .records = bt_hid_service_rec
};

static uint32_t bt_hid_set_report_handle(uint16_t handle, void *data, uint16_t size, bt_hogp_report_group_t *report_group)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_hogp_set_report_ind_t ind;
    ind.connection_handle = handle;
    ind.report_id = report_group->report_id;
    ind.type = report_group->report_type;
    ind.report_length = size;
    ind.report_data = (uint8_t *)data;
    status = bt_hogp_notify_user_by_callback(BT_HOGP_EVENT_SET_REPORT_IND, &ind, NULL);
    if (status != BT_STATUS_SUCCESS) {
        return 0;
    }
    return size;
}

static uint32_t bt_hid_get_report_handle(uint16_t handle, void *data, uint16_t size,
                                         uint16_t offset, bt_hogp_report_group_t *report_group)
{
    static uint8_t cache_buffer[BT_HID_SERVICE_GET_REPORT_REPSONSE_MAX_LENGTH] = {0x00};
    if (report_group == NULL) {
        LOG_MSGID_E(BT_HID_SRV, "[HID][SRV] get report handle fail, report_group = NULL", 0);
        return 0;
    }
    if (size == 0) {
        memset(cache_buffer, 0, BT_HID_SERVICE_GET_REPORT_REPSONSE_MAX_LENGTH);
        bt_hogp_get_report_response_t response = {0};
        response.report_data = cache_buffer;
        bt_hogp_get_report_ind_t ind = {0};
        ind.connection_handle = handle;
        ind.type = report_group->report_type;
        ind.report_id = report_group->report_id;
        ind.offset = offset;
        response.report_length = BT_HID_SERVICE_GET_REPORT_REPSONSE_MAX_LENGTH;
        bt_hogp_notify_user_by_callback(BT_HOGP_EVENT_GET_REPORT_IND, &ind, &response);
        LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] get report data length = %d by report id = %02x", 2, response.report_length, report_group->report_id);
        return response.report_length;
    } else {
        memcpy(data, cache_buffer, size);
        return size;
    }
    LOG_MSGID_E(BT_HID_SRV, "[HID][SRV] get report handle fail, size error", 0);
    return 0;
}

static uint32_t bt_hid_get_report_id_handle(uint16_t handle, void *data, uint16_t size, bt_hogp_report_index_t index)
{
    bt_hogp_get_report_id_response_t response = {0};
    bt_hogp_report_group_t *report_group = bt_hogp_get_report_group_by_index(index);
    if (report_group == NULL) {
        LOG_MSGID_E(BT_HID_SRV, "[HID][SRV] get report id handle fail, report_group = NULL", 0);
        return 0;
    }
    if (size == 0) {
        if (BT_HOGP_INVALID_REPORT_ID == report_group->report_id) {
            bt_hogp_get_report_id_t ind = {0};
            ind.connection_handle = handle;
            ind.type = report_group->report_type;
            bt_hogp_notify_user_by_callback(BT_HOGP_EVENT_GET_REPORT_ID_IND, &ind, &response);
            report_group->report_id = response.report_id;
            LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] get report id = %02x by report type = %02x from user", 2, report_group->report_id, report_group->report_type);
        } else {
            LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] get report id = %02x by report type = %02x", 2, report_group->report_id, report_group->report_type);
        }
        return sizeof(bt_hogp_report_group_t);
    } else if (size == sizeof(bt_hogp_report_group_t)) {
        memcpy(data, report_group, sizeof(bt_hogp_report_group_t));
        return sizeof(bt_hogp_report_group_t);
    }
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] get report id handle fail, size error", 0);
    return 0;
}

bt_status_t bt_hid_service_report_input_notification(bt_handle_t connection_handle, uint8_t *payload, uint16_t length)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t send_buffer[100] = {0x00};
    bt_hid_service_context_t *context = bt_hid_find_context_by_handle(connection_handle);
    if (context == NULL) {
        LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] report input notification context is NULL", 0);
        return 0;
    }
    if (!(context->report_input_cccd & BT_HID_SERVICE_NOTIFICATION_ENABLE_MASK)) {
        LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] report notification fail,cccd disable", 0);
        return status;
    }
    bt_gattc_charc_value_notification_indication_t *notification = (bt_gattc_charc_value_notification_indication_t *)send_buffer;
    notification->attribute_value_length = 3 + length;
    notification->att_req.opcode = BT_ATT_OPCODE_HANDLE_VALUE_NOTIFICATION;
    notification->att_req.handle = HID_REPORT_INPUT_HANDLE;
    memcpy(notification->att_req.attribute_value, payload, length);
    status = bt_gatts_send_charc_value_notification_indication(connection_handle, notification);
    if (status != BT_STATUS_SUCCESS) {
        LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] report notification fail status = %02x", 1, status);
    }
    return status;
}


static void bt_hid_service_db_after_flash_complete_cb(nvkey_status_t status, void *user_data)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] Non-blocking flush db result:%d, index:%d", 2,
                status, (uint32_t)user_data);
}

static bt_status_t bt_hid_srv_db_storage_write(bt_hid_service_context_t *context)
{
#ifdef AIR_NVDM_ENABLE
    nvkey_status_t result = NVKEY_STATUS_ERROR;
    uint32_t index = (uint32_t)bt_hid_find_context_index(context);
    uint16_t hogp_nvkey_id = NVID_BT_HOST_HOGP_1 + index;
    bt_hid_service_nvdm_info_t nvdm_info = {{0}};
    memcpy(&nvdm_info.remote_address, &context->remote_address, sizeof(bt_addr_t));
    nvdm_info.bk_input_cccd = context->bk_input_cccd;
    nvdm_info.report_input_cccd = context->report_input_cccd;
    nvdm_info.flag = context->flag;
    result = nvkey_write_data_non_blocking(hogp_nvkey_id, (const uint8_t *)&nvdm_info, 
                                                sizeof(bt_hid_service_nvdm_info_t), bt_hid_service_db_after_flash_complete_cb, (const void *)index);
    if (result == NVKEY_STATUS_OK) {
        LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] db storage write index = %02x success", 1, index);
    } else {
        LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] db storage write index = %02x fail status = %02x", 2, index, result);
    }
#endif
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_hid_srv_db_storage_read(bt_hid_service_context_t *context)
{
#ifdef AIR_NVDM_ENABLE
    nvkey_status_t read_status = NVKEY_STATUS_ERROR;
    uint32_t nvdm_read_size = 0;
    bt_hid_service_nvdm_info_t nvdm_info = {{0}};
    uint8_t index = bt_hid_find_context_index(context);
    uint16_t hogp_nvkey_id = NVID_BT_HOST_HOGP_1 + index;
    nvdm_read_size = sizeof(bt_hid_service_nvdm_info_t);
    read_status = nvkey_read_data(hogp_nvkey_id, (uint8_t *)&nvdm_info, &nvdm_read_size);
    if (read_status == NVKEY_STATUS_OK) {
        LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] db storage read index = %02x success", 1, index);
        memcpy(&context->remote_address, &nvdm_info.remote_address, sizeof(bt_addr_t));
        context->bk_input_cccd = nvdm_info.bk_input_cccd;
        context->report_input_cccd = nvdm_info.report_input_cccd;
        context->flag = nvdm_info.flag;
        LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] db storage bk_input_cccd = %02x report_input_cccd = %02x, flag = %02x", 3,
                    context->bk_input_cccd, context->report_input_cccd, context->flag);
    } else {
        LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] db storage read index = %02x fail status = %02x", 2, index, read_status);
    }
#endif
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_hid_srv_db_storage_read_all(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint32_t i = 0;
    for (i = 0; i < BT_HID_SERVICE_CONTEXT_MAX; i++) {
        bt_hid_service_context_t *context = &hid_context[i];
        status = bt_hid_srv_db_storage_read(context);
    }
    return status;
}

static bt_status_t bt_hid_service_db_storage_all_context(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    for (uint32_t i = 0; i < BT_HID_SERVICE_CONTEXT_MAX; i++) {
        bt_hid_service_context_t *context = &hid_context[i];
        if (context->flag & BT_HID_SERVICE_FLAG_BONDED) {
            status = bt_hid_srv_db_storage_write(&hid_context[i]);
        }
    }
    return status;
}

static void bt_hid_service_get_report_id(bt_hid_service_context_t *context)
{
    for (uint32_t i = 0; i < BT_HOGP_REPORT_INDEX_MAX; i++) {
        bt_hogp_get_report_id_response_t response = {0};
        bt_hogp_report_group_t *report_group = bt_hogp_get_report_group_by_index(i);
        if (report_group == NULL) {
            LOG_MSGID_E(BT_HID_SRV, "[HID][SRV] get report id by context = %02x, report_group = NULL", 1, context);
            return;
        }

        if (BT_HOGP_INVALID_REPORT_ID == report_group->report_id) {
            bt_hogp_get_report_id_t ind = {0};
            ind.connection_handle = context->connection_handle;
            ind.type = report_group->report_type;
            bt_hogp_notify_user_by_callback(BT_HOGP_EVENT_GET_REPORT_ID_IND, &ind, &response);
            report_group->report_id = response.report_id;
            LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] get report id = %02x by report type = %02x with connected from user", 2, report_group->report_id, report_group->report_type);
        } else {
            LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] get report id = %02x by report type = %02x with connected", 2, report_group->report_id, report_group->report_type);
        }
    }
}

bt_status_t bt_hid_service_common_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_POWER_OFF_CNF: {
            bt_hid_service_db_storage_all_context();
        }
        break;
        case BT_GAP_LE_CONNECT_IND: {
            if (status == BT_STATUS_SUCCESS) {
                bt_addr_t *p_peer_address = NULL;
                bt_gap_le_connection_ind_t *ind = (bt_gap_le_connection_ind_t *)buff;
                LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] LE connect complete handle = %02x", 1, ind->connection_handle);
                p_peer_address = &ind->peer_addr;
                /* check whether the remote address is IDA */
                bt_device_manager_le_bonded_info_t *bond_info = bt_device_manager_le_get_bonding_info_by_addr_ext(&ind->peer_addr.addr);
                if (NULL != bond_info) {
                    if (bt_utils_memcmp(&ind->peer_addr, &bond_info->info.identity_addr.address, sizeof(bt_addr_t)) == 0) {
                        LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] LE connect address is IDA", 0);
                        p_peer_address = &bond_info->bt_addr;
                    }
                }

                bt_hid_service_context_t *context = bt_hid_get_context_by_address(p_peer_address);
                if (context != NULL) {
                    context->connection_handle = ind->connection_handle;
                    memcpy(&context->remote_address, p_peer_address, sizeof(bt_addr_t));
                    bt_hid_service_get_report_id(context);
                    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] LE connect context = %02x flag = %02x report_input_cccd = %02x bk_input_cccd = %02x", 4, context, context->flag, context->report_input_cccd, context->bk_input_cccd);
                }
            }
        }
        break;
        case BT_GAP_LE_DISCONNECT_IND: {
            if (status == BT_STATUS_SUCCESS) {
                bt_gap_le_disconnect_ind_t *ind = (bt_gap_le_disconnect_ind_t *)buff;
                LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] LE disconnect complete handle = %02x", 1, ind->connection_handle);
                bt_hid_service_context_t *context = bt_hid_find_context_by_handle(ind->connection_handle);
                if (context != NULL) {
                    context->connection_handle = BT_HID_SRV_INVAILD_HANDLE;
                }
            }
        }
        break;
        case BT_GAP_LE_BONDING_COMPLETE_IND: {
            bt_gap_le_bonding_complete_ind_t *ind = (bt_gap_le_bonding_complete_ind_t *)buff;
            bt_hid_service_context_t *context = bt_hid_find_context_by_handle(ind->handle);
            LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] bond complete handle = %02x", 1, ind->handle);
            if (NULL != context) {
                context->flag |= BT_HID_SERVICE_FLAG_BONDED;
                bt_hid_srv_db_storage_write(context);
            }
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static void bt_hid_service_bond_event_callback(bt_device_manager_le_bonded_event_t event, bt_addr_t *address)
{
    LOG_MSGID_I(BT_HID_SRV, "[HID][SRV] bond event = %02x", 1, event);
    switch (event) {
#if 0
        case BT_DEVICE_MANAGER_LE_BONDED_ADD: {
            bt_hid_service_context_t *context = bt_hid_get_context_by_address(address);
            if (context == NULL) {
                return;
            }
            context->flag |= BT_HID_SERVICE_FLAG_BONDED;
            bt_hid_srv_db_storage_write(context);
        }
        break;
#endif
        case BT_DEVICE_MANAGER_LE_BONDED_REMOVE: {
            bt_hid_service_context_t *context = bt_hid_find_context_by_address(address);
            if (context == NULL) {
                return;
            }

            context->flag &= ~BT_HID_SERVICE_FLAG_BONDED;
            bt_hid_service_context_t backup_context = {0};

            memcpy(&backup_context, context, sizeof(bt_hid_service_context_t));

            memset(context, 0, sizeof(bt_hid_service_context_t));
            bt_hid_srv_db_storage_write(context);

            memcpy(context, &backup_context, sizeof(bt_hid_service_context_t));
        }
        break;
        case BT_DEVICE_MANAGER_LE_BONDED_CLEAR: {
            memset(&hid_context, 0, sizeof(bt_hid_service_context_t) * BT_HID_SERVICE_CONTEXT_MAX);
            bt_hid_service_db_storage_all_context();
        }
        break;
        default:
            break;
    }
}

bt_status_t bt_hid_service_init(void)
{
    bt_hid_srv_db_storage_read_all();
    return bt_device_manager_le_register_callback(bt_hid_service_bond_event_callback);
}
