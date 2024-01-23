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

#include "app_bolt_poc_report_merge.h"
#include "app_bolt_poc_report_usage.h"
#include "app_bolt_poc_data.h"
#include "app_bolt_poc_utility.h"
#include "apps_debug.h"

#define BIT_COUNT_PER_BYTE (8)
#define MAX_DEV_NUM (APP_WHITE_LIST_NUM)

static Device_Info_s g_device_info[MAX_DEV_NUM] = {0};


static int8_t app_bolt_poc_find_dev_idx_by_address(const bt_addr_t *bt_addr)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_DEV_NUM; i++)
    {
        const Device_Info_s *dev_info = &g_device_info[i];
        if (app_bolt_poc_addr_not_empty(&dev_info->addr_)) {
            if (app_bolt_poc_cmp_addr(bt_addr, &dev_info->addr_)) {
                return i;
            }
        }
    }
    return -1;
}

static int8_t app_bolt_poc_find_dev_idx_by_handle(bt_handle_t handle)
{
    const bt_addr_t *bt_addr = app_bolt_poc_find_address_by_handle(handle);
    if (bt_addr == NULL) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_bolt_poc_find_dev_idx_by_handle return null address", 0);
        return -1;
    }

    return app_bolt_poc_find_dev_idx_by_address(bt_addr);
}

static const Bit_Field_s *app_bolt_poc_find_bit_field(uint8_t idx, uint8_t dev_id)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        const Sub_Dev_Info_s *dev_info = &g_device_info[idx].dev_[i];
        if (dev_info->device_id_ == 0) {
            break;
        }

        if (dev_info->device_id_ == dev_id) {
            return dev_info->field_;
        }
    }

    return NULL;
}

static const uint8_t app_bolt_poc_find_field_count(uint8_t idx, uint8_t dev_id)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        const Sub_Dev_Info_s *dev_info = &g_device_info[idx].dev_[i];
        if (dev_info->device_id_ == 0) {
            break;
        }

        if (dev_info->device_id_ == dev_id) {
            return dev_info->field_unit_count_;
        }
    }

    return 0;
}

// USB notify convert flow, you can ask Liyu for details
static int8_t app_bolt_poc_keyid_to_idx(uint8_t key_id)
{
    int8_t idx = 0;
    if (key_id >= 0x90 && key_id <= 0x92) {
        idx = 0x75 + (key_id - 0x90);
    } else if (key_id >= 0x87 && key_id <= 0x8B) {
        idx = 0x70 + (key_id - 0x87);
    } else if (key_id >= 0x04 && key_id <= 0x73) {
        idx = 0 + (key_id - 0x04);
    } else {
        idx = -1;
    }
    return idx;
}

static void app_bolt_poc_set_bit_data(int8_t *buf, int8_t idx)
{
    buf[idx / BIT_COUNT_PER_BYTE] |= (1 << (idx % BIT_COUNT_PER_BYTE));
}

void app_bolt_poc_init_device_info()
{
    memset(g_device_info, 0, MAX_DEV_NUM * sizeof(Device_Info_s));
}

uint8_t app_bolt_poc_find_dev_id(bt_handle_t handle, uint8_t report_id, int8_t *p_idx)
{
    const int8_t idx = app_bolt_poc_find_dev_idx_by_handle(handle);
    if (idx < 0) {
        return 0;
    }

    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        const Sub_Dev_Info_s *dev_info = &g_device_info[idx].dev_[i];
        if (dev_info->device_id_ == 0) {
            break;
        }

        if (dev_info->report_id_ == report_id) {
            *p_idx = idx;
            return dev_info->device_id_;
        }
    }

    return 0;
}

bool app_bolt_poc_dev_info_full()
{
    uint8_t i = 0;
    for (i = 0; i < MAX_DEV_NUM; i++)
    {
        Device_Info_s *dev_info = &g_device_info[i];
        if (!app_bolt_poc_addr_not_empty(&dev_info->addr_)) {
            return false;
        }
    }
    return true;
}

bool app_bolt_poc_add_dev_info(const Device_Info_s *data)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_DEV_NUM; i++)
    {
        Device_Info_s *dev_info = &g_device_info[i];

        // If address is empty, need to add dev info
        if (!app_bolt_poc_addr_not_empty(&dev_info->addr_)) {
            memcpy(dev_info, data, sizeof(Device_Info_s));
            return true;
        }
    }

    return false;
}

void app_bolt_poc_del_dev_info(const bt_addr_t *bt_addr)
{
    const int8_t idx = app_bolt_poc_find_dev_idx_by_address(bt_addr);
    if (idx < 0) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_bolt_poc_del_dev_info return", 0);
        return;
    }

    Device_Info_s *dev_info = &g_device_info[idx];

    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        app_bolt_poc_memory_free(dev_info->dev_[i].field_);
    }

    memset(dev_info, 0, sizeof(Device_Info_s));
}

bool app_bolt_poc_get_mouse_info(LogiBolt_Mouse_t *data, bt_handle_t handle, const uint8_t *buffer, uint16_t length, int8_t idx)
{
    const uint8_t field_count = app_bolt_poc_find_field_count(idx, GD_Mouse);
    const Bit_Field_s *bit_field = app_bolt_poc_find_bit_field(idx, GD_Mouse);
    if (NULL == bit_field) {
        return false;
    }

    uint16_t total_bit_count = 0;

    uint8_t i = 0;
    for (i = 0; i < field_count; i++) {
        total_bit_count += bit_field[i].bit_count_;
    }

    if (length * BIT_COUNT_PER_BYTE != total_bit_count) {
        return false;
    }

    //APPS_LOG_DUMP_I(" [BOLT_POC] Mouse report data: ", buffer, length);
    for (i = 0; i < field_count; i++)
    {
        const uint8_t id = bit_field[i].id_;
        const uint16_t start_bit = bit_field[i].start_bit_;
        const uint16_t bit_count = bit_field[i].bit_count_;

        switch (id)
        {
        case MOUSE_BUTTON_EN:
            data->bitmap_Btn = get_int16(buffer, start_bit, bit_count);
            //APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_get_mouse_info Button(0x%X)", 1, data->bitmap_Btn);
            break;
        case MOUSE_X_EN:
            data->x = get_sign_int16(buffer, start_bit, bit_count);
            //APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_get_mouse_info X(0x%X)", 1, data->x);
            break;
        case MOUSE_Y_EN:
            data->y = get_sign_int16(buffer, start_bit, bit_count);
            //APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_get_mouse_info Y(0x%X)", 1, data->y);
            break;
        case MOUSE_WHEEL_EN:
            data->wheel = get_sign_int8(buffer, start_bit, bit_count);
            //APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_get_mouse_info Wheel(0x%X)", 1, data->wheel);
            break;
        case MOUSE_AC_PAN_EN:
            data->ac_pan = get_sign_int8(buffer, start_bit, bit_count);
            //APPS_LOG_MSGID_I(" [BOLT_POC] app_bolt_poc_get_mouse_info AC Pan(0x%X)", 1, data->ac_pan);
            break;
        }
    }
    return true;
}

bool app_bolt_poc_get_keyboard_info(LogiBolt_Keyboard_t *data, bt_handle_t handle, const uint8_t *buffer, uint16_t length, int8_t idx)
{
    const uint8_t field_count = app_bolt_poc_find_field_count(idx, GD_Keyboard);
    const Bit_Field_s *bit_field = app_bolt_poc_find_bit_field(idx, GD_Keyboard);
    if (NULL == bit_field) {
        return false;
    }

    uint16_t bit_count_sum = 0;

    uint8_t i = 0;
    for (i = 0; i < field_count; i++) {
        bit_count_sum += bit_field[i].bit_count_;
    }

    if (length * BIT_COUNT_PER_BYTE != bit_count_sum) {
        return false;
    }

    //APPS_LOG_DUMP_I(" [BOLT_POC] Keyboard report data: ", buffer, length);
    for (i = 0; i < field_count; i++)
    {
        if (bit_field[i].id_ == KEYBOARD_BUTTON_EN) {
            const uint16_t start_bit = bit_field[i].start_bit_;
            const uint16_t bit_count = bit_field[i].bit_count_;
            if (bit_count <= BIT_COUNT_PER_BYTE) {
                data->keymap[0] = get_int8(buffer, start_bit, bit_count);
            } else {
                const uint8_t byte_count = bit_count / BIT_COUNT_PER_BYTE;
                uint8_t j = 0;
                for (j = 0; j < byte_count; j++) {
                    const uint8_t key_id = get_int8(buffer, start_bit + BIT_COUNT_PER_BYTE * j, BIT_COUNT_PER_BYTE);
                    const int8_t idx = app_bolt_poc_keyid_to_idx(key_id);
                    if (idx >= 0 && idx < 120) {
                        app_bolt_poc_set_bit_data(&data->keymap[1], idx);
                    }
                }
            }
        }
    }
    //APPS_LOG_DUMP_I(" [BOLT_POC] Keyboard to usb data: ", data->keymap, sizeof(data->keymap));

    return true;
}

bool app_poc_bolt_find_keyboard_dev_id(const bt_addr_t *bt_addr)
{
    const int8_t idx = app_bolt_poc_find_dev_idx_by_address(bt_addr);
    if (idx < 0) {
        return false;
    }

    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        const Sub_Dev_Info_s *dev_info = &g_device_info[idx].dev_[i];
        if (dev_info->device_id_ == GD_Keyboard) {
            return true;
        }
    }
    return false;
}

