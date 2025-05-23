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

#include "hal_flash.h"
#ifdef HAL_FLASH_MODULE_ENABLED

#define FLASHCONF_C

#include "hal_flash_general_types.h"
#include "hal_flash_fs_type.h"
#include "hal_flash_custom_memorydevice.h"
#include "hal_flash_sf.h"
#include "string.h"
#include "hal_flash_opt.h"
#include "hal_flash_drvflash.h"
#include "hal_flash_disk.h"
#include "hal_flash_disk_internal.h"
#include "hal_flash_combo_init.h"
#include "hal_flash_disk_internal.h"

#include "hal_flash_custom.h"

#define MS_TABLE_ENTRY_NUM    (4 * 2 + 1)//(NVRAM_CUSTOM_CFG_MAX_RECORD_SECTOR_NUM * 2 + 1)

/*
 ****************************************************************************
 PART 2:
 Essential Information of NOR Flash Geometry Layout Information
 ****************************************************************************
*/
/*******************************************************************************
   NOTICE: Fill the flash region information table, a region is the memory space
           that contains continuous sectors of equal size. Each region element
           in the table is the format as below:
           {S_sector, N_sector},
               S_sector: the size of sector in the region
               N_sector: the number of sectors in the region
 *******************************************************************************/

#define _NOR_FLASH_BOOTING_
#ifdef _NOR_FLASH_BOOTING_
#define FLASH_REGIONINFO_VAR_MODIFIER  static

#if (!defined(__COMBO_MEMORY_SUPPORT__) && !defined(__SERIAL_FLASH__))
#if 0
#ifdef __NOR_SUPPORT_RAW_DISK__

#ifndef NOR_BOOTING_NOR_DISK_NUM
#error "custom\system\{project}\hal_flash_custom_memorydevice.h: Error! NOR_BOOTING_NOR_DISK_NUM be defined when __NOR_SUPPORT_RAW_DISK__ is defined."
#endif //NOR_BOOTING_NOR_DISK_NUM

#if (NOR_BOOTING_NOR_DISK_NUM>0)
FLASH_REGIONINFO_VAR_MODIFIER FlashRegionInfo Disk0RegionInfo[] = /* Don't modify this line */
{
    DISK0_REGION_INFO_LAYOUT
    EndRegionInfo /* Don't modify this line */
};
#endif /* NOR_BOOTING_NOR_DISK_NUM>0 */

#if (NOR_BOOTING_NOR_DISK_NUM>1)
FLASH_REGIONINFO_VAR_MODIFIER FlashRegionInfo Disk1RegionInfo[] = /* Don't modify this line */
{
    DISK1_REGION_INFO_LAYOUT
    EndRegionInfo /* Don't modify this line */
};
#endif /* NOR_BOOTING_NOR_DISK_NUM>1 */
#endif // __NOR_SUPPORT_RAW_DISK__
#endif

#if 0
FLASH_REGIONINFO_VAR_MODIFIER FlashRegionInfo EntireDiskRegionInfo[] = /* Don't modify this line */
{
    ENTIRE_DISK_REGION_INFO_LAYOUT
    EndRegionInfo /* Don't modify this line */
};
#endif

NOR_FLASH_DISK_Data EntireDiskDriveData;
//static NOR_Flash_MTD_Data EntireDiskMtdData;

/*******************************************************************************
   NOTICE. Modify the value of page buffer size in WORD for page buffer program
 *******************************************************************************/
uint32_t PAGE_BUFFER_SIZE = BUFFER_PROGRAM_ITERATION_LENGTH;

/*******************************************************************************
   NOTICE. This is for the Enhanced Signle Bank Support, when this feature is
           turned on and still use multi-bank device, this table should be filled
           with correct value.

           This Table define the flash bank information which starts from
           FLASH_BASE_ADDRESS, please fill the flash bank information table, every
           entry defines the memory space that contains continuous banks of equal size.
           Each entry element in the table is the format as below:
           {S_Bank, N_Bank},
               S_Bank: the size of bank in the entry
               N_Bank: the number of banks in the entry
 *******************************************************************************/
#endif /* !__COMBO_MEMORY_SUPPORT__ && !__SERIAL_FLASH__ */

#endif // _NOR_FLASH_BOOTING_

/*
 ****************************************************************************
 PART 3:
 Essential Declarations for NOR-Flash Disk
 ****************************************************************************
*/


/*
 ****************************************************************************
 PART 4:
 Public Functions For NOR Flash Information Retrieve, Initial routine, and
 other misc routines.
 ****************************************************************************
*/
uint32_t custom_get_NORFLASH_Base(void);

/*
 ****************************************************************************
 PART 5:
 Essential Declarations for NAND-Flash Disk
 ****************************************************************************
*/

/*************************************************************************
* FUNCTION
*  custom_get_NORFLASH_ROMSpace()
*
* DESCRIPTION
*  Query the of space configured for NORFLASH ROM
*
* PARAMETERS
*
* RETURNS
*  BASE ADDRESS
*
* GLOBALS AFFECTED
*
*************************************************************************/
uint32_t
custom_get_NORFLASH_ROMSpace(void)
{
    return NOR_FLASH_BASE_ADDRESS;
}


/*************************************************************************
* FUNCTION
*  custom_get_NORFLASH_Size()
*
* DESCRIPTION
*  Query the size of NORFLASH ROM
*
* PARAMETERS
*
* RETURNS
*  SIZE
*
* GLOBALS AFFECTED
*
*************************************************************************/
uint32_t custom_get_NORFLASH_Size(void)
{
    uint32_t flash_size = 0;

    if (HAL_FLASH_STATUS_OK != hal_flash_get_disk_size(&flash_size)) {
        return 0;
    } else {
        return flash_size;
    }
}

/*************************************************************************
* FUNCTION
*  Initialize_FDD_tables
*
* DESCRIPTION
*  Initialize important information for NOR-flash disk
*
* PARAMETERS
*
* RETURNS
*  None
*
* GLOBALS AFFECTED
*
*************************************************************************/
#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH

#if ((defined(__COMBO_MEMORY_SUPPORT__) || defined(__SERIAL_FLASH__)) && defined(_NOR_FLASH_BOOTING_))
int32_t Initialize_FDD_tables(void)
{
    return ComboMem_Initialize();
}
#endif /* !(__COMBO_MEMORY_SUPPORT__) && !(__SERIAL_FLASH__) */

void nor_sweep_device(void)
{
//#ifdef __NOR_FDM5__
//    kal_set_eg_events(nor_egid, NOR_DMAN_EVENT, KAL_OR);
//#endif
    return;
}

void nor_manual_reclaim(void)
{
#ifdef __NOR_FDM5__
    kal_set_eg_events(nor_egid, NOR_BRECL_EVENT, KAL_OR);
#endif
    return;
}


/*************************************************************************
* FUNCTION
*  custom_get_fat_addr()
*
* DESCRIPTION
*  This function gets the start address of FAT.
*
* PARAMETERS
*  none
*
* RETURNS
*  FAT start address
*
*************************************************************************/
uint32_t custom_get_fat_addr()
{
    return NOR_FLASH_BASE_ADDRESS;
}

/*************************************************************************
* FUNCTION
*  custom_get_fat_len()
*
* DESCRIPTION
*  This function gets the len of FAT.
*
* PARAMETERS
*  none
*
* RETURNS
*  FAT length
*
*************************************************************************/
uint32_t custom_get_fat_len()
{
    return NOR_ALLOCATED_FAT_SPACE;
}
#endif // __MAUI_LOAD__

/*************************************************************************
* FUNCTION
*  Custom_NOR_Init
*
* DESCRIPTION
*  Initialize important information for NOR-flash disk
*
* PARAMETERS
*
* RETURNS
*  None
*
* GLOBALS AFFECTED
*
*************************************************************************/
int32_t Custom_NOR_Init(void)
{
    return CMEM_Init_FullDriver();
}


/*************************************************************************
* FUNCTION
*  custom_get_NORFLASH_Base()
*
* DESCRIPTION
*  Query the of space configured for NORFLASH ROM
*
* PARAMETERS
*
* RETURNS
*  BASE ADDRESS
*
* GLOBALS AFFECTED
*
*************************************************************************/
uint32_t custom_get_NORFLASH_Base(void)
{
    return HAL_FLASH_BASE_ADDRESS;
}

#endif //#ifdef HAL_FLASH_MODULE_ENABLED

