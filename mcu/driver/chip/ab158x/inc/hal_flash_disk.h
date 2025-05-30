/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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
 
#ifndef __FLASH_DISK_H__
#define __FLASH_DISK_H__

/*******************************************
*   Include File
********************************************/
#include "hal_flash_custom_memorydevice.h"
#include "hal_flash_opt.h"
#include "hal_flash_mtd.h"
#include "air_chip.h"
#include <stdio.h>
#include <stdlib.h>
#include "hal_flash_drvflash.h"



/*******************************************
*   Error Code Definition
********************************************/

#define RAW_DISK_ERR_NONE 0
#define RAW_DISK_ERR_WRONG_ADDRESS 2
#define RAW_DISK_ERR_NO_SPACE  3


#define RAW_DISK_ERR_PROG_FAIL 5
#define RAW_DISK_ERR_ERASE_FAIL 6
#define RAW_DISK_ERR_FDM_LOCKED 7
#define RAW_DISK_ERR_DRV_LOCKED 8
#define RAW_DISK_ERR_ERASE_OVER_TIME 9
#define RAW_DISK_ERR_PROG_USER_DATA_CHANGED_FAIL  (10)

#define ERROR_NOR_SUCCESS             (0)   //(ERROR_FOTA_SUCCESS)
#define ERROR_NOR_READ                (-13) //(ERROR_FOTA_READ)
#define ERROR_NOR_PROGRAM             (-14) //(ERROR_FOTA_PROGRAM)
#define ERROR_NOR_ERASE               (-15) //(ERROR_FOTA_ERASE)
#define ERROR_NOR_OVERRANGE           (-1)  //(ERROR_FOTA_OVERRANGE)

/*******************************************
*   Structure and Macro Definition
********************************************/

#define FLASH_DISK_DONE FS_NO_ERROR

#ifdef __SERIAL_FLASH_EN__
#define NOR_FLASH_PAGE_SIZE 0x200 //default 512B page
#define NOR_FLASH_PAGE_SHIFT 9
#else
#define NOR_FLASH_PAGE_SIZE 0x800 //default 2KB page
#define NOR_FLASH_PAGE_SHIFT 11
#endif


int32_t NOR_init (void);
//int32_t NOR_QueryInfo (Logical_Flash_info info);
int32_t NOR_ReadPhysicalPage (uint32_t block_idx, uint32_t page_idx, uint8_t *data_ptr);
#ifdef __NOR_FULL_DRIVER__
int32_t NOR_ProgramPhysicalPage (uint32_t block_idx, uint32_t page_idx, uint8_t *data_ptr);
int32_t NOR_ErasePhysicalBlock (uint32_t block_idx);
#endif //__NOR_FULL_DRIVER__
uint32_t NOR_BlockSize(uint32_t block_idx);
uint32_t NOR_BlockIndex(uint32_t block_addr);
int32_t NOR_Addr2BlockIndexOffset(uint32_t block_addr, uint32_t *block_index, uint32_t *offset);
int32_t NOR_BlockIndexOffset2Addr(uint32_t block_index, uint32_t offset, uint32_t *addr);

#ifdef HAL_FLASH_MODULE_ENABLED
hal_flash_block_t get_block_size_from_address(uint32_t address);
#endif

#endif //__FLASH_DISK_H__

