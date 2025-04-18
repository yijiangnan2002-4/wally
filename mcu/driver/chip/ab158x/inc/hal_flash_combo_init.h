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
 
#ifndef __COMBO_FLASH_INIT_H__
#define __COMBO_FLASH_INIT_H__

#include "hal_flash_opt.h"
//#include "kal_release.h"
#include "hal_flash_combo_defs.h"

#if defined(__COMBO_MEMORY_SUPPORT__) || defined(__SERIAL_FLASH__) || defined(__SPI_NAND_SUPPORT__)

#include "hal_flash_custom_memorydevice.h"
//#include "fat_fs.h"
#include "hal_flash_drvflash.h"

// Naming Rule: Camo
typedef enum   {
    CMEM_NO_ERROR = 0,
    CMEM_ERR_ID_NOT_FOUND,
    CMEM_ERR_FDM_MISMATCH,
    CMEM_ERR_MTD_MISMATCH,
    CMEM_ERR_FDM_INIT_FAIL,
    CMEM_ERR_MTD_INIT_FAIL,
    CMEM_ERR_FDM_REGION_INFO_OVERFLOW
} Enum_CMEM_StatusCode;

typedef enum   {
    CMEM_TYPE_END_OF_LIST = 0,
    CMEM_TYPE_NAND,
    CMEM_TYPE_NOR,
    CMEM_TYPE_SERIAL_NOR_FLASH,
    CMEM_TYPE_EMMC,
    CMEM_TYPE_SERIAL_NAND_FLASH
} Enum_CMEM_Type;


//Sync with original hal_flash_opt.h and MemoryDeviceList
//Ransense series is phase out
typedef enum   {
    CMEM_FDM_NOR_DEFAULT = 0,	//for serial flash
    CMEM_NOR_CS_INTEL_SERIES,
    CMEM_NOR_CS_INTEL_SIBLEY,
    CMEM_NOR_CS_RAM_DISK,
    CMEM_NOR_CS_SST,
    CMEM_NOR_CS_AMD_SERIES,
    CMEM_NOR_CS_SPANSION_PL_J,
    CMEM_NOR_CS_SPANSION_PL_N,
    CMEM_NOR_CS_SPANSION_WS_N,
    CMEM_NOR_CS_SPANSION_WS_P,
    CMEM_NOR_CS_SPANSION_GL_A,
    CMEM_NOR_CS_SPANSION_GL_N,
    CMEM_NOR_CS_SPANSION_NS_N,
    CMEM_NOR_CS_SPANSION_NS_P,
    CMEM_NOR_CS_SPANSION_NS_J,
    CMEM_NOR_CS_TOSHIBA,
    CMEM_NOR_CS_TOSHIBA_TV,
    CMEM_NOR_CS_TOSHIBA_TY,
    CMEM_NOR_CS_SILICON7,
    CMEM_NOR_CS_SAMSUNG_SPANSION_NS_J_LIKE,
    CMEM_NOR_CS_SPANSION_VS_R,
    CMEM_NOR_CS_SPANSION_VS_R64,
    CMEM_NOR_CS_SPANSION_WS_R

} Enum_CMEM_CommSeries;

// Functions Exported to MAUI
extern int32_t ComboMem_Initialize(void);
extern int32_t CMEM_Init_FullDriver(void);
extern uint32_t CMEM_BlockSize(uint32_t address);
int32_t CMEM_Index(void);

const CMEMEntryID *CMEM_GetIDEntry(uint32_t index);

#endif // (defined(__COMBO_MEMORY_SUPPORT__) || defined(__SERIAL_FLASH__))

// Function exported to EMI/SFI
bool CMEM_CheckValidDeviceID(uint8_t *id);
bool CMEM_EMIINIT_CheckValidDeviceID(uint8_t *id);

int32_t CMEM_EMIINIT_Index(void);

#endif // __COMBO_FLASH_INIT_H__

