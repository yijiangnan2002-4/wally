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


#ifndef __FLASH_CUSTOM_GEN__
#define __FLASH_CUSTOM_GEN__

#if 0
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

//#define ENTIRE_DISK_REGION_INFO_LAYOUT    {0x1000, 1024},

/*******************************************************************************
   NOTICE. Modify the value of TOTAL_BLOCKS, which is the sum of the number of
           sectors in all regions.
           Note : The Maximum value of TOTAL_BLOCKS is (127).
 *******************************************************************************/

//#define TOTAL_BLOCKS    64

/*******************************************************************************
   NOTICE. Modify the value of page buffer size in WORD for page buffer program
 *******************************************************************************/
//uint32_t PAGE_BUFFER_SIZE = 64;

//#define BANK_INFO_LAYOUT    {0x40000, 1}, 

/*******************************************************************************
   NOTICE. NOR FLASH BLOCKS SIZE LOOKUP TABLE
          Each entry element
          {Offset, Block_Size},
          Offset:     the offset address
          Block_Size: the size of block
 *******************************************************************************/
/*
FLASH_REGIONINFO_VAR_MODIFIER FlashBlockTBL NOTREADYYET[] =
{
    {0x0, 0x1000},
    EndBlockInfo
};
*/



//#define NOR_FLASH_SIZE  0x00400000

//const char FLASH_ID[] = "{0xC2, 0x28, 0x16}";

/****************************************************
 * This part is for auto-gen validity CHECK *
 * Don't modify any content in this comment section *


 * NOR_FLASH_DENSITY: 0x00400000
 * NOR_FLASH_SIZE(Mb): 32

FLASH_REGIONINFO_VAR_MODIFIER FlashRegionInfo oriRegionInfo[] =
{
   {0x1000, 64},
   EndoriRegionInfo
};


static NORBankInfo oriBankInfo[] =
{
   {0x400000, 1}, \
   EndBankInfo
};

 ****************************************************/
#endif

#endif  //__FLASH_CUSTOM_GEN__

