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


#include "app_bolt_poc_report_item.h"
#include "app_bolt_poc_report_usage.h"
#include "app_bolt_poc_report_merge.h"
#include "apps_debug.h"


#define ri_ItemSize(sizeMask)      ((uint8_t)(sizeMask) == Size_4B?4:(uint8_t)(sizeMask))

static int32_t ri_GetItemData(uint8_t *itemData, uint8_t size)
{
    if(size == 1)
        return *itemData;
    else if(size == 2)
        return *((int16_t *)itemData);
    else if(size == 4)
        return *((int32_t *)itemData);

    return 0;
}

static void  ri_MainItem_(uint8_t itemTag, int32_t itemData, uint16_t index)
{
    switch (itemTag)
    {
    case Input(0):
        app_bolt_poc_add_input_field(index);
        break;
    case Output(0):
        break;
    case Feature(0):
        break;
    case Collection(0):
        app_bolt_poc_add_collection_field(index);
        break;
    case End_Colletion(0):
        app_bolt_poc_add_end_collection_field(index);
        break;
    default:
        break;
    }
}

static void ri_GlobalItem_(uint8_t itemTag, int32_t itemData, int32_t* pUsagePage, uint16_t index)
{
    switch (itemTag)
    {
    case Usage_Page(0):
        *pUsagePage = itemData;
        if (UP_Button == *pUsagePage) {
            app_bolt_poc_add_name_id_field(MOUSE_BUTTON_EN, index);
        } else if (UP_Keyboard_or_Keypad == *pUsagePage) {
            app_bolt_poc_add_name_id_field(KEYBOARD_BUTTON_EN, index);
        }
        break;
    case Logical_Minimum(0):
        break;
    case Logical_Maximum(0):
        break;
    case Physical_Minimum(0):
        break;
    case Physical_Maximum(0):
        break;
    case Unit_Exponent(0):
        break;
    case Unit(0):
        break;
    case Report_Size(0):
        app_bolt_poc_add_size_field(itemData, index);
        break;
    case Report_ID(0):
        app_bolt_poc_add_report_id_field(itemData, index);
        break;
    case Report_Count(0):
        app_bolt_poc_add_count_field(itemData, index);
        break;
    case Push(0):
        break;
    case Pop(0):
        break;
    default:
        break;
    }
}

static void ri_LocalItem_(uint8_t itemTag, int32_t itemData, int32_t usagePage, uint16_t index)
{
    switch (itemTag)
    {
    case Usage(0):
        app_bolt_poc_ri_usage(usagePage, itemData, index);
        break;
    case Usage_Minimum(0):
        break;
    case Usage_Maximum(0):
        break;
    case Designator_Index(0):
        break;
    case Designator_Minimum(0):
        break;
    case Designator_Maximum(0):
        break;
    case String_Index(0):
        break;
    case String_Minimum(0):
        break;
    case String_Maximum(0):
        break;
    case Delimiter(0):
        break;
    default:
        break;
    }
}

void app_bolt_poc_ri_parse(bt_handle_t handle, uint8_t *buffer, uint16_t len)
{
    if (app_bolt_poc_dev_info_full()) {
        APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_ri_parse: dev info full", 0);
        return;
    }

    app_bolt_poc_field_buf_init();

    uint16_t index = 0;
    while (index < len)
    {
        static int32_t sUsagePage = -1;
        const uint8_t itemSize = ri_ItemSize(buffer[index] & SIZE_MASK);
        if (index + itemSize >= len)
        {
            APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_ri_parse: out of buffer", 0);
            return;
        }

        const uint8_t itemTag = buffer[index] & TAG_MASK;
        const int32_t itemData = ri_GetItemData(&buffer[index + 1], itemSize);
        switch (itemTag & TYPE_MASK)
        {
        case MAIN_ITEM:
            ri_MainItem_(itemTag, itemData, index);
            break;
        case GLOBAL_ITEM:
            ri_GlobalItem_(itemTag, itemData, &sUsagePage, index);
            break;
        case LOCAL_ITEM:
            ri_LocalItem_(itemTag, itemData, sUsagePage, index);
            break;
        default:
            APPS_LOG_MSGID_E(" [BOLT_POC] app_bolt_poc_ri_parse: unknown type(0x%X), index(%d)", 2, itemTag, index);
            break;
        }
        index += (itemSize + 1);
    }

    app_bolt_poc_process_dev_info(handle);
}

