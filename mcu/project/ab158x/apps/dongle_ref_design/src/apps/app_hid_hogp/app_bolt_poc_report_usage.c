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


#include "app_bolt_poc_report_usage.h"
#include "app_bolt_poc_report_merge.h"
#include "app_bolt_poc_utility.h"
#include "app_bolt_poc_data.h"
#include "apps_debug.h"


#pragma pack(1)
typedef struct {
    uint16_t idx_;
} Idx_Field_s;
#pragma pack()

#pragma pack(1)
typedef struct {
    uint8_t data_;
    uint16_t idx_;
} Data_Field_s;
#pragma pack()

#pragma pack(1)
typedef struct {
    uint16_t id_;
    uint16_t idx_;
    uint16_t size_;
    uint16_t count_;
} Input_Info_s;
#pragma pack()

#pragma pack(1)
typedef struct {
    uint8_t dev_id_;
    uint16_t idx_;
    uint16_t start_;
    uint16_t end_;
} Usage_Range_s;;
#pragma pack()


#define MAX_ITEM_CNT (16)
static Idx_Field_s g_input_field[MAX_ITEM_CNT];
static Idx_Field_s g_output_field[MAX_ITEM_CNT];
static Idx_Field_s g_collection_field[MAX_ITEM_CNT];
static Idx_Field_s g_end_collection_field[MAX_ITEM_CNT];
static Data_Field_s g_name_id_field[MAX_ITEM_CNT];
static Data_Field_s g_count_field[MAX_ITEM_CNT];
static Data_Field_s g_size_field[MAX_ITEM_CNT];
static Data_Field_s g_report_id_field[MAX_ITEM_CNT];

static Input_Info_s g_input_info[MAX_ITEM_CNT];

static Usage_Range_s g_usage_range[MAX_SUB_DEV_NUM];

#define MOUSE_ITEM_CNT (8)
static Bit_Field_s g_mouse_field[MOUSE_ITEM_CNT];

#define KEYBOARD_ITEM_CNT (4)
static Bit_Field_s g_keyboard_field[KEYBOARD_ITEM_CNT];


void app_bolt_poc_field_buf_init()
{
    memset(g_input_field, 0, MAX_ITEM_CNT * sizeof(Idx_Field_s));
    memset(g_output_field, 0, MAX_ITEM_CNT * sizeof(Idx_Field_s));
    memset(g_collection_field, 0, MAX_ITEM_CNT * sizeof(Idx_Field_s));
    memset(g_end_collection_field, 0, MAX_ITEM_CNT * sizeof(Idx_Field_s));

    memset(g_name_id_field, 0, MAX_ITEM_CNT * sizeof(Data_Field_s));
    memset(g_count_field, 0, MAX_ITEM_CNT * sizeof(Data_Field_s));
    memset(g_size_field, 0, MAX_ITEM_CNT * sizeof(Data_Field_s));
    memset(g_report_id_field, 0, MAX_ITEM_CNT * sizeof(Data_Field_s));

    memset(g_input_info, 0, MAX_ITEM_CNT * sizeof(Input_Info_s));

    memset(g_usage_range, 0, MAX_SUB_DEV_NUM * sizeof(Usage_Range_s));

    memset(g_mouse_field, 0, MOUSE_ITEM_CNT * sizeof(Bit_Field_s));
    memset(g_keyboard_field, 0, KEYBOARD_ITEM_CNT * sizeof(Bit_Field_s));
}

bool app_bolt_poc_add_input_field(uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_input_field[i].idx_ == 0)
        {
            g_input_field[i].idx_ = idx;
            return true;
        }
    }
    return false;
}

bool app_bolt_poc_add_output_field(uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_output_field[i].idx_ == 0)
        {
            g_output_field[i].idx_ = idx;
            return true;
        }
    }
    return false;
}

bool app_bolt_poc_add_collection_field(uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_collection_field[i].idx_ == 0)
        {
            g_collection_field[i].idx_ = idx;
            return true;
        }
    }
    return false;
}

bool app_bolt_poc_add_end_collection_field(uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_end_collection_field[i].idx_ == 0)
        {
            g_end_collection_field[i].idx_ = idx;
            return true;
        }
    }
    return false;
}

bool app_bolt_poc_add_name_id_field(uint8_t name_id, uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_name_id_field[i].idx_ == 0)
        {
            g_name_id_field[i].idx_ = idx;
            g_name_id_field[i].data_ = name_id;
            return true;
        }
    }
    return false;
}

bool app_bolt_poc_add_count_field(int32_t count, uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_count_field[i].idx_ == 0)
        {
            g_count_field[i].idx_ = idx;
            g_count_field[i].data_ = count;
            return true;
        }
    }
    return false;
}

bool app_bolt_poc_add_size_field(int32_t size, uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_size_field[i].idx_ == 0)
        {
            g_size_field[i].idx_ = idx;
            g_size_field[i].data_ = size;
            return true;
        }
    }
    return false;
}

bool app_bolt_poc_add_report_id_field(int32_t report_id, uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_report_id_field[i].idx_ == 0)
        {
            g_report_id_field[i].idx_ = idx;
            g_report_id_field[i].data_ = report_id;
            return true;
        }
    }
    return false;
}

static bool app_bolt_poc_find_usage_dev_id(int32_t dev_id)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        if (g_usage_range[i].dev_id_ == dev_id) {
            return true;
        }
    }
    return false;
}

static bool app_bolt_poc_add_usage_dev_id(int32_t dev_id, uint16_t index)
{
    if (app_bolt_poc_find_usage_dev_id(dev_id)) {
        return false;
    }

    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        if (g_usage_range[i].dev_id_ == 0) {
            g_usage_range[i].dev_id_ = dev_id;
            g_usage_range[i].idx_ = index;
            return true;
        }
    }
    return false;
}

static bool app_bolt_poc_check_pair_range(uint16_t start, uint16_t end)
{
    if (start >= end) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_bolt_poc_check_pair_range start is not smaller than end", 0);
        return false;
    }

    uint8_t collection_cnt = 0;
    uint8_t end_collection_cnt = 0;

    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++) {
        const uint16_t collection_idx = g_collection_field[i].idx_;
        if (collection_idx == 0) {
            break;
        }

        if (collection_idx > start && collection_idx < end) {
            collection_cnt++;
        }
    }

    for (i = 0; i < MAX_ITEM_CNT; i++) {
        const uint16_t end_collection_idx = g_end_collection_field[i].idx_;
        if (end_collection_idx == 0) {
            break;
        }

        if (end_collection_idx > start && end_collection_idx < end) {
            end_collection_cnt++;
        }
    }

    return collection_cnt == end_collection_cnt;
}

static uint16_t app_bolt_poc_find_usage_range_start(uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++) {
        const uint16_t start = g_collection_field[i].idx_;
        if (start == 0) {
            break;
        }

        if (start > idx ) {
            return start;
        }
    }
    return 0;
}

static uint16_t app_bolt_poc_find_usage_range_end(uint16_t start)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++) {
        const uint16_t end = g_end_collection_field[i].idx_;
        if (end == 0) {
            break;
        }

        if (end > start && app_bolt_poc_check_pair_range(start, end)) {
            return end;
        }
    }
    return 0;
}

static bool app_bolt_poc_fill_usage_range_start()
{
    bool ret = false;

    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        const uint16_t idx = g_usage_range[i].idx_;
        const uint16_t dev_id = g_usage_range[i].dev_id_;
        if (idx == 0 || dev_id == 0) {
            break;
        }

        const uint16_t start = app_bolt_poc_find_usage_range_start(idx);
        //APPS_LOG_MSGID_I("[BOLT_POC] app_bolt_poc_fill_usage_range_start start(%d)", 1, start);
        if (start > 0) {
            g_usage_range[i].start_ = start;
            ret = true;
        }
    }
    return ret;
}

static bool app_bolt_poc_fill_usage_range_end()
{
    bool ret = false;

    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        const uint16_t start = g_usage_range[i].start_;
        const uint16_t dev_id = g_usage_range[i].dev_id_;
        if (start == 0 || dev_id == 0) {
            break;
        }

        const uint16_t end = app_bolt_poc_find_usage_range_end(start);
        //APPS_LOG_MSGID_I("[BOLT_POC] app_bolt_poc_fill_usage_range_end end(%d)", 1, end);
        if (end > 0) {
            g_usage_range[i].end_ = end;
            ret = true;
        }
    }
    return ret;
}

static bool app_bolt_poc_fill_usage_range()
{
    return app_bolt_poc_fill_usage_range_start() && app_bolt_poc_fill_usage_range_end();
}

static int8_t app_bolt_poc_find_id(uint16_t start, uint16_t end)
{
    if (start >= end) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_bolt_poc_find_id start is not smaller than end", 0);
        return 0;
    }

    uint16_t id_max_index = 0;

    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_name_id_field[i].idx_ == 0)
        {
            break;
        }
        id_max_index = g_name_id_field[i].idx_;
    }

    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_name_id_field[i].idx_ == 0)
        {
            break;
        }

        if (start > id_max_index)
        {
            return -1;
        }

        if (g_name_id_field[i].idx_ > start && g_name_id_field[i].idx_ < end)
        {
            return g_name_id_field[i].data_;
        }
    }

    return 0;
}

static int8_t app_bolt_poc_find_size(uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_size_field[i].idx_ == 0)
        {
            break;
        }

        if (i + 1 == MAX_ITEM_CNT)
        {
            break;
        }

        if (g_size_field[i + 1].idx_ == 0)
        {
            return g_size_field[i].data_;
        }

        if (g_size_field[i].idx_ < idx && g_size_field[i + 1].idx_ > idx)
        {
            return g_size_field[i].data_;
        }
    }

    return -1;
}

static int8_t app_bolt_poc_find_count(uint16_t idx)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        if (g_count_field[i].idx_ == 0)
        {
            break;
        }

        if (i + 1 == MAX_ITEM_CNT)
        {
            break;
        }

        if (g_count_field[i + 1].idx_ == 0)
        {
            return g_count_field[i].data_;
        }

        if (g_count_field[i].idx_ < idx && g_count_field[i + 1].idx_ > idx)
        {
            return g_count_field[i].data_;
        }
    }

    return -1;
}


static bool app_bolt_poc_fill_mouse_field(uint16_t start, uint16_t end)
{
    if (start >= end) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_bolt_poc_fill_mouse_field start is not smaller than end", 0);
        return false;
    }

    uint8_t count = 0;

    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        const uint8_t id = g_input_info[i].id_;
        const uint8_t bit_count = g_input_info[i].size_ * g_input_info[i].count_;
        if (bit_count == 0)
        {
            break;
        }

        if (g_input_info[i].idx_ > start && g_input_info[i].idx_ < end) {
            if (id == MOUSE_X_EN)
            {
                g_mouse_field[count].id_ = MOUSE_X_EN;
                g_mouse_field[count].bit_count_ = bit_count / 2;
                count++;
                g_mouse_field[count].id_ = MOUSE_Y_EN;
                g_mouse_field[count].bit_count_ = bit_count / 2;
                count++;
            }
            else if (id == MOUSE_Y_EN)
            {
                g_mouse_field[count].id_ = MOUSE_Y_EN;
                g_mouse_field[count].bit_count_ = bit_count / 2;
                count++;
                g_mouse_field[count].id_ = MOUSE_X_EN;
                g_mouse_field[count].bit_count_ = bit_count / 2;
                count++;
            }
            else
            {
                g_mouse_field[count].id_ = id;
                g_mouse_field[count].bit_count_ = bit_count;
                count++;
            }
        }
    }

    uint16_t sum = 0;
    for (i = 0; i < count; i++)
    {
        g_mouse_field[i].start_bit_ = sum;
        sum += g_mouse_field[i].bit_count_;
    }

    return count > 0;
}

static bool app_bolt_poc_fill_keyboard_field(uint16_t start, uint16_t end)
{
    if (start >= end) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_bolt_poc_fill_keyboard_field start is not smaller than end", 0);
        return false;
    }

    uint8_t count = 0;

    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        const uint8_t id = g_input_info[i].id_;
        const uint8_t size = g_input_info[i].size_ * g_input_info[i].count_;
        if (size == 0)
        {
            break;
        }

        if (g_input_info[i].idx_ > start && g_input_info[i].idx_ < end) {
            g_keyboard_field[count].id_ = id;
            g_keyboard_field[count].bit_count_ = size;
            count++;
        }
    }

    uint16_t sum = 0;
    for (i = 0; i < count; i++)
    {
        g_keyboard_field[i].start_bit_ = sum;
        sum += g_keyboard_field[i].bit_count_;
    }

    return count > 0;
}

static uint8_t app_bolt_poc_find_report_id_by_range(uint8_t start, uint8_t end)
{
    if (start >= end) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_bolt_poc_find_report_id_by_range start is not smaller than end", 0);
        return 0;
    }

    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        const uint8_t id = g_report_id_field[i].data_;
        if (id == 0)
        {
            break;
        }

        if (g_report_id_field[i].idx_ > start && g_report_id_field[i].idx_ < end) {
            return g_report_id_field[i].data_;
        }
    }
    return 0;
}

static uint8_t app_bolt_poc_get_report_id(uint8_t dev_id)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        if (g_usage_range[i].dev_id_ == 0) {
            break;
        }

        if (g_usage_range[i].dev_id_ == dev_id) {
            return app_bolt_poc_find_report_id_by_range(g_usage_range[i].start_, g_usage_range[i].end_);
        }
    }
    return 0;
}

static bool app_bolt_poc_fill_dev_field()
{
    bool ret_mouse = false;
    bool ret_kb = false;

    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        if (g_usage_range[i].dev_id_ == 0) {
            break;
        }

        if (g_usage_range[i].dev_id_ == GD_Mouse) {
            ret_mouse = app_bolt_poc_fill_mouse_field(g_usage_range[i].start_, g_usage_range[i].end_);
        } else if (g_usage_range[i].dev_id_ == GD_Keyboard) {
            ret_kb = app_bolt_poc_fill_keyboard_field(g_usage_range[i].start_, g_usage_range[i].end_);
        }
    }
    return ret_mouse || ret_kb;
}

static bool app_bolt_poc_fill_input_info()
{
    uint8_t cnt = 0;

    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++)
    {
        const uint16_t idx = g_input_field[i].idx_;
        if (idx == 0)
        {
            break;
        }

        const uint16_t start = (i > 0) ? g_input_field[i - 1].idx_ : 0;
        const int8_t id = app_bolt_poc_find_id(start, idx);
        if (id < 0)
        {
            break;
        }

        const int8_t size = app_bolt_poc_find_size(idx);
        if (size < 0)
        {
            break;
        }

        const int8_t count_value = app_bolt_poc_find_count(idx);
        if (count_value < 0)
        {
            break;
        }

        g_input_info[cnt].idx_ = idx;
        g_input_info[cnt].id_ = id;
        g_input_info[cnt].size_ = size;
        g_input_info[cnt].count_ = count_value;
        cnt++;
    }

    return cnt > 0;
}

static void app_poc_bolt_print_mouse(uint8_t id, uint16_t start_bit, uint16_t bit_count)
{
    switch (id)
    {
    case MOUSE_PADDING_EN:
        APPS_LOG_MSGID_I("[BOLT_POC] MOUSE(Padding): start(%d), bit(%d)", 2, start_bit, bit_count);
        break;
    case MOUSE_BUTTON_EN:
        APPS_LOG_MSGID_I("[BOLT_POC] MOUSE(Button): start(%d), bit(%d)", 2, start_bit, bit_count);
        break;
    case MOUSE_X_EN:
        APPS_LOG_MSGID_I("[BOLT_POC] MOUSE(X): start(%d), bit(%d)", 2, start_bit, bit_count);
        break;
    case MOUSE_Y_EN:
        APPS_LOG_MSGID_I("[BOLT_POC] MOUSE(Y): start(%d), bit(%d)", 2, start_bit, bit_count);
        break;
    case MOUSE_WHEEL_EN:
        APPS_LOG_MSGID_I("[BOLT_POC] MOUSE(Wheel): start(%d), bit(%d)", 2, start_bit, bit_count);
        break;
    case MOUSE_AC_PAN_EN:
        APPS_LOG_MSGID_I("[BOLT_POC] MOUSE(AC Pan): start(%d), bit(%d)", 2, start_bit, bit_count);
        break;
    }
}

static void app_poc_bolt_print_keyboard(uint8_t id, uint16_t start_bit, uint16_t bit_count)
{
    switch (id)
    {
    case KEYBOARD_PADDING_EN:
        APPS_LOG_MSGID_I("[BOLT_POC] KEYBOARD(Padding): start(%d), bit(%d)", 2, start_bit, bit_count);
        break;
    case KEYBOARD_BUTTON_EN:
        APPS_LOG_MSGID_I("[BOLT_POC] KEYBOARD(Button): start(%d), bit(%d)", 2, start_bit, bit_count);
        break;
    }
}

static void app_poc_bolt_print_dev_info(const Device_Info_s *data)
{
    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++)
    {
        const Sub_Dev_Info_s *dev_info = &data->dev_[i];
        if (dev_info->device_id_ == 0) {
            break;
        }

        uint8_t j = 0;
        for (j = 0; j < dev_info->field_unit_count_; j++) {
            const uint8_t id = dev_info->field_[j].id_;
            const uint16_t start_bit = dev_info->field_[j].start_bit_;
            const uint16_t bit_count = dev_info->field_[j].bit_count_;
            if (dev_info->device_id_ == GD_Mouse) {
                app_poc_bolt_print_mouse(id, start_bit, bit_count);
            } else if (dev_info->device_id_ == GD_Keyboard) {
                app_poc_bolt_print_keyboard(id, start_bit, bit_count);
            }
        }
    }
}

static bool app_poc_bolt_add_field_info(Device_Info_s *dst, const Bit_Field_s *src, uint8_t cnt, uint8_t dev_id, uint8_t report_id)
{
    const uint16_t len = cnt * sizeof(Bit_Field_s);
    Bit_Field_s *data = (Bit_Field_s*)app_bolt_poc_memory_alloc(len);
    if (data == NULL) {
        APPS_LOG_MSGID_E("[BOLT_POC] field malloc fail", 0);
        return false;
    }
    memcpy(data, src, len);

    uint8_t i = 0;
    for (i = 0; i < MAX_SUB_DEV_NUM; i++) {
        if (dst->dev_[i].device_id_ == 0) {
            dst->dev_[i].device_id_ = dev_id;
            dst->dev_[i].report_id_ = report_id;
            dst->dev_[i].field_ = data;
            dst->dev_[i].field_unit_count_ = cnt;
            return true;
        }
    }

    app_bolt_poc_memory_free(data);
    return false;
}

static uint8_t app_bolt_poc_get_bit_field_count(uint8_t dev_id)
{
    Bit_Field_s *bit_field = NULL;
    if (dev_id == GD_Mouse) {
        bit_field = g_mouse_field;
    } else if (dev_id == GD_Keyboard) {
        bit_field = g_keyboard_field;
    } else {
        return 0;
    }

    uint8_t count = 0;
    uint8_t i = 0;
    for (i = 0; i < MAX_ITEM_CNT; i++) {
        if (bit_field[i].bit_count_ == 0) {
            break;
        }
        count++;
    }
    return count;
}

static bool app_bolt_poc_add_field(Device_Info_s *dst, uint8_t dev_id)
{
    const uint8_t count = app_bolt_poc_get_bit_field_count(dev_id);
    if (count > 0) {
        const uint8_t report_id = app_bolt_poc_get_report_id(dev_id);
        if (dev_id == GD_Mouse) {
            return app_poc_bolt_add_field_info(dst, g_mouse_field, count, dev_id, report_id);
        } else if (dev_id == GD_Keyboard) {
            return app_poc_bolt_add_field_info(dst, g_keyboard_field, count, dev_id, report_id);
        }
    }
    return false;
}

static bool app_poc_bolt_save_dev_info(bt_handle_t handle)
{
    Device_Info_s dev_info;
    memset(&dev_info, 0, sizeof(Device_Info_s));

    const bt_addr_t *bt_addr = app_bolt_poc_find_address_by_handle(handle);
    if (bt_addr == NULL) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_poc_bolt_save_dev_info return", 0);
        return false;
    }
    memcpy(&dev_info.addr_, bt_addr, sizeof(bt_addr_t));

    const bool ret_mouse = app_bolt_poc_add_field(&dev_info, GD_Mouse);
    const bool ret_keyboard = app_bolt_poc_add_field(&dev_info, GD_Keyboard);
    if (ret_mouse || ret_keyboard) {
        if (app_bolt_poc_add_dev_info(&dev_info)) {
            app_poc_bolt_print_dev_info(&dev_info);
            return true;
        } else {
            APPS_LOG_MSGID_E("[BOLT_POC] app_poc_bolt_save_dev_info app_bolt_poc_add_dev_info fail", 0);
        }
    } else {
        APPS_LOG_MSGID_E("[BOLT_POC] app_poc_bolt_save_dev_info not find mouse or keyboard", 0);
    }

    return false;
}

void app_bolt_poc_process_dev_info(bt_handle_t handle)
{
    if (!app_bolt_poc_fill_usage_range()) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_bolt_poc_fill_usage_range fail", 0);
        return;
    }
    if (!app_bolt_poc_fill_input_info()) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_bolt_poc_fill_input_info fail", 0);
        return;
    }
    if (!app_bolt_poc_fill_dev_field()) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_bolt_poc_fill_dev_field fail", 0);
        return;
    }
    if (!app_poc_bolt_save_dev_info(handle)) {
        APPS_LOG_MSGID_E("[BOLT_POC] app_poc_bolt_save_dev_info fail", 0);
        return;
    }
}

void app_bolt_poc_ri_usage(int32_t usagePage, int32_t usage, uint16_t index)
{
    if (UP_Generic_Desktop == usagePage) {
        if (GD_Mouse == usage || GD_Keyboard == usage) {
            app_bolt_poc_add_usage_dev_id(usage, index);
        } else if (GD_X == usage) {
            app_bolt_poc_add_name_id_field(MOUSE_X_EN, index);
        } else if (GD_Y == usage) {
            app_bolt_poc_add_name_id_field(MOUSE_Y_EN, index);
        } else if (GD_Wheel == usage) {
            app_bolt_poc_add_name_id_field(MOUSE_WHEEL_EN, index);
        }
    } else if (UP_Consumer == usagePage) {
        if (UC_AC_Pan == usage) {
            app_bolt_poc_add_name_id_field(MOUSE_AC_PAN_EN, index);
        }
    }
}

