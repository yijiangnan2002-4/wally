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
/* Airoha restricted information */

#if defined(__EXT_BOOTLOADER__)

#if defined(AIR_BL_DFU_ENABLE)


#include "lw_mux.h"
#include "race_handler.h"
#include "race_parser.h"
#include "race_cmd_bl_dfu.h"
#include "bl_common.h"


typedef struct {
    uint16_t race_id;
    race_cmd_handler_t hdl;
} race_cmd_tbl_t;



static const race_cmd_tbl_t g_race_cmd_tbl[] = {
    {RACE_ID_DA_WRITE_BYTES,        race_cmd_da_write_bytes},
    {RACE_ID_DA_READ_BYTES,         race_cmd_da_read_bytes},
    {RACE_ID_DA_ERASE_BYTES,        race_cmd_da_erase_bytes},
    {RACE_ID_DA_DATA_RANGE_CRC,     race_cmd_da_data_range_crc},
    {RACE_ID_DFU_START,             race_cmd_dfu_start},
    {RACE_ID_DFU_RESET,             race_cmd_dfu_reset},
    {RACE_ID_DA_GET_FLASH_ADDRESS,  race_da_get_flash_addr},
    {RACE_ID_DA_GET_FLASH_SIZE,     race_da_get_flash_size},
    {RACE_ID_DA_GET_FLASH_ID,       race_da_get_flash_id},
};



void race_cmd_handler(uint8_t *cmd, uint32_t cmd_len, uint8_t port)
{
    race_common_hdr_t *hdr;
    uint32_t i;
    uint32_t tbl_size = sizeof(g_race_cmd_tbl) / sizeof(g_race_cmd_tbl[0]);

    if (NULL == cmd || RACE_PROTOCOL_MIN_SIZE > cmd_len || port >= LW_MUX_PORT_MAX) {
        return ;
    }
    hdr = (race_common_hdr_t *)cmd;
    for (i = 0; i < tbl_size; i++) {
        if (hdr->id == g_race_cmd_tbl[i].race_id) {
            g_race_cmd_tbl[i].hdl(cmd, cmd_len, port);
            break;
        }
    }
    bl_print(LOG_DEBUG, "RACE:complete.\r\n");
}

uint32_t race_combo_u32(uint8_t *buf)
{
    return (buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24));
}

uint16_t race_combo_u16(uint8_t *buf)
{
    return (buf[0] | ((uint16_t)buf[1] << 8));
}

void race_put_u32(uint8_t *buf, uint32_t data)
{
    uint16_t i;
    uint16_t offset = 0;
    for (i = 0; i < 4; i++) {
        buf[i] = (data >> offset) & 0xff;
        offset += 8;
    }
}

void race_put_u16(uint8_t *buf, uint16_t data)
{
    buf[0] = data & 0xff;
    buf[1] = (data >> 8) & 0xff;
}

#endif

#endif

