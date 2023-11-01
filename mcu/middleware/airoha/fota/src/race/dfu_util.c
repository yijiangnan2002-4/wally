/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#include "bsp_flash.h"
#include <assert.h>
#include "memory_map.h"
#include "fota_platform.h"
#include "dfu_util.h"


bool dfu_flash_read(uint32_t start_address, uint8_t *data, uint32_t length)
{
    int32_t flash_ret = BSP_FLASH_STATUS_OK;
    flash_ret = bsp_flash_read(start_address, data, length);
    if (BSP_FLASH_STATUS_OK != flash_ret) {
        FOTA_LOG_MSGID_E("dfu_flash_read() failed err:%d, addr:%d, len:%d", 3, flash_ret, start_address, length);
        return false;
    }
    return true;
}

bool dfu_flash_erase(uint32_t start_address, bsp_block_size_type_t block_size)
{
    int32_t flash_ret = BSP_FLASH_STATUS_OK;
    flash_ret = bsp_flash_erase(start_address, block_size);
    if (BSP_FLASH_STATUS_OK != flash_ret) {
        FOTA_LOG_MSGID_E("dfu_flash_erase() failed err:%d, addr:%d, block_size_enum:%d", 3, flash_ret, start_address, block_size);
        return false;
    }
    return true;
}

bool dfu_flash_write(uint32_t start_address, uint8_t *data, uint32_t length)
{
    int32_t flash_ret = BSP_FLASH_STATUS_OK;
    flash_ret = bsp_flash_write(start_address, data, length);
    if (BSP_FLASH_STATUS_OK != flash_ret) {
        FOTA_LOG_MSGID_E("dfu_flash_write() failed err:%d, addr:%d, len:%d", 3, flash_ret, start_address, length);
        extern uint32_t error_addr;
        extern uint8_t org_data;
        extern uint8_t readback_data;
        extern uint32_t error_status;
        FOTA_LOG_MSGID_E("address:0x%x, write:0x%x, readback:0x%x, SR = 0x%x\r\n", 3, error_addr, org_data, readback_data, error_status);
        return false;
    }
    return true;
}

bool dfu_flag_is_set(void)
{
    uint8_t buffer[DFU_BL_ENABLE_FLAG_RECORD_LEN] = {0};
    bool flag;

    if (dfu_flash_read(ERASE_BACKUP_BASE, buffer, 1)) {
        if(buffer[0] == DFU_FLAG_DFU_ENABLE) {
            flag = true;
        } else {
            flag = false;
        }
    } else {
        flag = false;
        FOTA_LOG_E("dfu_flag_is_set() get flag failed");
    }

    return flag;
}

bool dfu_flag_clear(void)
{
    bool ret = false;

    FOTA_LOG_I("dfu_flag_clear start", 0);
    ret = dfu_flash_erase(ERASE_BACKUP_BASE, BSP_FLASH_BLOCK_4K);

    return ret;
}

bool dfu_flag_set(void)
{
    bool ret = false;
    uint8_t buffer[DFU_BL_ENABLE_FLAG_RECORD_LEN] = {0x00};

    FOTA_LOG_I("dfu_flag_set start", 0);
    ret =  dfu_flash_write(ERASE_BACKUP_BASE, buffer, DFU_BL_ENABLE_FLAG_RECORD_LEN);

    return ret;
}


