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


#ifndef RACE_CMD_BL_DFU_H
#define RACE_CMD_BL_DFU_H


#ifdef __cplusplus
extern "C"
{
#endif

#if defined(AIR_BL_DFU_ENABLE)

#include <stdint.h>

#define RACE_ID_DA_WRITE_BYTES              (0x2100)
#define RACE_ID_DA_READ_BYTES               (0x2101)
#define RACE_ID_DA_ERASE_BYTES              (0x2104)
#define RACE_ID_DA_GET_FLASH_ADDRESS        (0x210E)
#define RACE_ID_DA_GET_FLASH_SIZE           (0x210F)
#define RACE_ID_DA_GET_FLASH_ID             (0x2110)
#define RACE_ID_DA_DATA_RANGE_CRC           (0x211A)
#define RACE_ID_DFU_START                   (0x211B)
#define RACE_ID_DFU_RESET                   (0x211C)

#define RACE_BL_DFU_RW_MAX_SIZE             (4096)


void race_cmd_da_write_bytes(uint8_t *cmd, uint32_t cmd_len, uint8_t port);
void race_cmd_da_read_bytes(uint8_t *cmd, uint32_t cmd_len, uint8_t port);
void race_cmd_da_erase_bytes(uint8_t *cmd, uint32_t cmd_len, uint8_t port);
void race_cmd_da_data_range_crc(uint8_t *cmd, uint32_t cmd_len, uint8_t port);
void race_cmd_dfu_start(uint8_t *cmd, uint32_t cmd_len, uint8_t port);
void race_cmd_dfu_reset(uint8_t *cmd, uint32_t cmd_len, uint8_t port);
void race_da_get_flash_addr(uint8_t *pcmd, uint32_t cmd_len, uint8_t port);
void race_da_get_flash_size(uint8_t *pcmd, uint32_t cmd_len, uint8_t port);
void race_da_get_flash_id(uint8_t *pcmd, uint32_t cmd_len, uint8_t port);


#endif

#ifdef __cplusplus
}
#endif

#endif

#endif

