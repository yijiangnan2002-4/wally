/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
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

#include "nvkey.h"
#include "nvdm_internal.h"
#include <stdio.h>

#if ((PRODUCT_VERSION == 1552) || defined(AM255X) || \
    defined(AIR_BTA_IC_PREMIUM_G2) || \
    defined(AIR_BTA_IC_PREMIUM_G3) || \
    defined(AIR_BTA_IC_STEREO_HIGH_G3))

#ifndef __EXT_BOOTLOADER__
#define NVKEY_GROUP_NAME    ("AB15")

nvkey_status_t nvkey_read_data(uint16_t id, uint8_t *buffer, uint32_t *size)
{
    char hex[5];
    snprintf(hex, sizeof(hex), "%X", id);
    return nvdm_read_data_item(NVKEY_GROUP_NAME, hex, buffer, size);
}

nvkey_status_t nvkey_write_data(uint16_t id, const uint8_t *buffer, uint32_t size)
{
    char hex[5];
    snprintf(hex, sizeof(hex), "%X", id);
    return nvdm_write_data_item(NVKEY_GROUP_NAME, hex, NVDM_DATA_ITEM_TYPE_RAW_DATA, buffer, size);
}

nvkey_status_t nvkey_write_data_non_blocking(uint16_t id, const uint8_t *buffer, uint32_t size, const nvkey_user_callback_t callback, const void *user_data)
{
#ifdef SYSTEM_DAEMON_TASK_ENABLE
    char hex[5];
    snprintf(hex, sizeof(hex), "%X", id);
    return nvdm_write_data_item_non_blocking(NVKEY_GROUP_NAME, hex, NVDM_DATA_ITEM_TYPE_RAW_DATA, buffer, size, (const nvdm_user_callback_t)callback, user_data);
#else
    /* not support action for writting NVDM */
    return NVKEY_STATUS_ERROR;
#endif
}

nvkey_status_t nvkey_data_item_length(uint16_t id, uint32_t *size)
{
    char hex[5];
    snprintf(hex, sizeof(hex), "%X", id);
    return nvdm_query_data_item_length(NVKEY_GROUP_NAME, hex, size);
}

nvkey_status_t nvkey_delete_data_item(uint16_t id)
{
    char hex[5];
    snprintf(hex, sizeof(hex), "%X", id);
    return nvdm_delete_data_item(NVKEY_GROUP_NAME, hex);
}

#endif /* __EXT_BOOTLOADER__ */

#endif
