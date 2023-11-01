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
/**
 * File: memory_map.h
 *
 * Description: This file defines the struct or macro of system memory map.
 *
 */

#ifndef __MEMORY_MAP_H__
#define __MEMORY_MAP_H__
#ifdef AIR_BTA_IC_PREMIUM_G3
#include "layout_partition.h"


extern uint32_t _rom_base;
extern uint32_t _rom_length;

extern uint32_t lp_get_length(partition_t id);
extern uint32_t lp_get_begin_address(partition_t id);


#define ROM_BASE                       (uint32_t)(&_rom_base)
#define ROM_LENGTH                     (uint32_t)(&_rom_length)

#define SEC_HEADER1_BASE               lp_get_begin_address(PARTITION_SECURITY_HEADER)
#define SEC_HEADER1_LENGTH             lp_get_length(PARTITION_SECURITY_HEADER)

#define SEC_HEADER2_BASE               lp_get_begin_address(PARTITION_SECURITY_HEADER2)
#define SEC_HEADER2_LENGTH             lp_get_length(PARTITION_SECURITY_HEADER2)

#define BL_BASE                        lp_get_begin_address(PARTITION_BL)
#define BL_LENGTH                      lp_get_length(PARTITION_BL)

#define CM4_BASE                       lp_get_begin_address(PARTITION_MCU)
#define CM4_LENGTH                     lp_get_length(PARTITION_MCU)

#define DSP0_BASE                      lp_get_begin_address(PARTITION_DSP0)
#define DSP0_LENGTH                    lp_get_length(PARTITION_DSP0)

#define FOTA_RESERVED_BASE             lp_get_begin_address(PARTITION_FOTA)
#define FOTA_RESERVED_LENGTH           lp_get_length(PARTITION_FOTA)

#define ROM_NVDM_BASE                  lp_get_begin_address(PARTITION_NVDM)
#define ROM_NVDM_LENGTH                lp_get_length(PARTITION_NVDM)

#define ROM_NVDM_OU_BASE               lp_get_begin_address(PARTITION_NVDM_OU)
#define ROM_NVDM_OU_LENGTH             lp_get_length(PARTITION_NVDM_OU)

#define ROFS_BASE                      lp_get_begin_address(PARTITION_ROFS)
#define ROFS_LENGTH                    lp_get_length(PARTITION_ROFS)

#define LM_GVA_BASE                    lp_get_begin_address(PARTITION_LM_GVA)
#define LM_GVA_LENGTH                  lp_get_length(PARTITION_LM_GVA)

#define LM_AMA_BASE                    lp_get_begin_address(PARTITION_LM_AMA)
#define LM_AMA_LENGTH                  lp_get_length(PARTITION_LM_AMA)

#define ERASE_BACKUP_BASE              lp_get_begin_address(PARTITION_ERASE_BACKUP)
#define ERASE_BACKUP_LENGTH            lp_get_length(PARTITION_ERASE_BACKUP)

#define VRAM_BASE                      0x20000000

/* Variables obtained from the link script. */
extern uint32_t _sysram_start[];
extern uint32_t _sysram_length[];
extern uint32_t _vsysram_start[];
extern uint32_t _vsysram_length[];
extern uint32_t _tcm_start[];
extern uint32_t _tcm_length[];
extern uint32_t __tcm_free_begin[];
extern uint32_t __tcm_free_limit[];
extern uint32_t __sysram_free_begin[];
extern uint32_t __sysram_free_limit[];

#define TCM_BASE                       (uint32_t)_tcm_start
#define TCM_LENGTH                     (uint32_t)_tcm_length
#define TCM_FREE_BEGIN                 (uint32_t)__tcm_free_begin
#define TCM_FREE_LIMIT                 (uint32_t)__tcm_free_limit

#define SYSRAM_BASE                    (uint32_t)_sysram_start
#define SYSRAM_LENGTH                  (uint32_t)_sysram_length
#define SYSRAM_FREE_BEGIN              (uint32_t)__sysram_free_begin
#define SYSRAM_FREE_LIMIT              (uint32_t)__sysram_free_limit

#define VSYSRAM_BASE                   (uint32_t)_vsysram_start
#define VSYSRAM_LENGTH                 (uint32_t)_vsysram_length

#elif (defined (AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3))
extern uint32_t _sysram_start[];
extern uint32_t _sysram_length[];
extern uint32_t _vsysram_start[];
extern uint32_t _vsysram_length[];
extern uint32_t _tcm_start[];
extern uint32_t _tcm_length[];


extern uint32_t _rom_base;
extern uint32_t _rom_length;
#define ROM_BASE    (uint32_t)(&_rom_base)
#define ROM_LENGTH  (uint32_t)(&_rom_length)
#define PARTITION_TABLE  ((PartitionTable_T *)ROM_BASE)


#ifndef MTK_LAYOUT_PARTITION_ENABLE
typedef struct {
    uint32_t BinaryId;
    uint32_t PartitionId;
    uint32_t LoadAddressHigh;
    uint32_t LoadAddressLow;
    uint32_t BinaryLengthHigh;
    uint32_t BinaryLengthLow;
    uint32_t ExecutionAddress;
    uint32_t ReservedItem0;
    uint32_t ReservedItem1;
    uint32_t ReservedItem2;
    uint32_t ReservedItem3;
    uint32_t ReservedItem4;
} PartitionTableItem_T;

typedef struct {
    PartitionTableItem_T SEC_HEADER1;
    PartitionTableItem_T SEC_HEADER2;
    PartitionTableItem_T BL;
    PartitionTableItem_T CM4;
    PartitionTableItem_T DSP0;
    PartitionTableItem_T FOTA;
    PartitionTableItem_T NVDM;
    PartitionTableItem_T ROFS;
    PartitionTableItem_T LM;
    PartitionTableItem_T NVDM_OTA;
} PartitionTable_T;

#define SEC_HEADER1_BASE     PARTITION_TABLE->SEC_HEADER1.LoadAddressLow
#define SEC_HEADER1_LENGTH   PARTITION_TABLE->SEC_HEADER1.BinaryLengthLow

#define SEC_HEADER2_BASE     PARTITION_TABLE->SEC_HEADER2.LoadAddressLow
#define SEC_HEADER2_LENGTH   PARTITION_TABLE->SEC_HEADER2.BinaryLengthLow

#define BL_BASE     PARTITION_TABLE->BL.LoadAddressLow
#define BL_LENGTH   PARTITION_TABLE->BL.BinaryLengthLow

#define CM4_BASE    PARTITION_TABLE->CM4.LoadAddressLow
#define CM4_LENGTH  PARTITION_TABLE->CM4.BinaryLengthLow

#define DSP0_BASE   PARTITION_TABLE->DSP0.LoadAddressLow
#define DSP0_LENGTH PARTITION_TABLE->DSP0.BinaryLengthLow

#define FOTA_RESERVED_BASE    PARTITION_TABLE->FOTA.LoadAddressLow
#define FOTA_RESERVED_LENGTH  PARTITION_TABLE->FOTA.BinaryLengthLow

#define ROM_NVDM_BASE    PARTITION_TABLE->NVDM.LoadAddressLow
#define ROM_NVDM_LENGTH  PARTITION_TABLE->NVDM.BinaryLengthLow

#define ROFS_BASE   PARTITION_TABLE->ROFS.LoadAddressLow
#define ROFS_LENGTH PARTITION_TABLE->ROFS.BinaryLengthLow

#define LM_BASE   PARTITION_TABLE->LM.LoadAddressLow
#define LM_LENGTH PARTITION_TABLE->LM.BinaryLengthLow

#define ROM_NVDM_OU_BASE    PARTITION_TABLE->NVDM_OTA.LoadAddressLow
#define ROM_NVDM_OU_LENGTH    PARTITION_TABLE->NVDM_OTA.BinaryLengthLow

#else
#include "layout_partition.h"

#define SEC_HEADER1_BASE               lp_get_begin_address(PARTITION_SECURITY_HEADER)
#define SEC_HEADER1_LENGTH             lp_get_length(PARTITION_SECURITY_HEADER)

#define SEC_HEADER2_BASE               lp_get_begin_address(PARTITION_SECURITY_HEADER2)
#define SEC_HEADER2_LENGTH             lp_get_length(PARTITION_SECURITY_HEADER2)

#define BL_BASE                        lp_get_begin_address(PARTITION_BL)
#define BL_LENGTH                      lp_get_length(PARTITION_BL)

#define CM4_BASE                       lp_get_begin_address(PARTITION_MCU)
#define CM4_LENGTH                     lp_get_length(PARTITION_MCU)

#define DSP0_BASE                      lp_get_begin_address(PARTITION_DSP0)
#define DSP0_LENGTH                    lp_get_length(PARTITION_DSP0)

#define FOTA_RESERVED_BASE             lp_get_begin_address(PARTITION_FOTA)
#define FOTA_RESERVED_LENGTH           lp_get_length(PARTITION_FOTA)

#define ROM_NVDM_BASE                  lp_get_begin_address(PARTITION_NVDM)
#define ROM_NVDM_LENGTH                lp_get_length(PARTITION_NVDM)

#define ROFS_BASE                      lp_get_begin_address(PARTITION_ROFS)
#define ROFS_LENGTH                    lp_get_length(PARTITION_ROFS)

#define LM_BASE                        lp_get_begin_address(PARTITION_LM)
#define LM_LENGTH                      lp_get_length(PARTITION_LM)

#define ROM_NVDM_OU_BASE               lp_get_begin_address(PARTITION_NVDM_OU)
#define ROM_NVDM_OU_LENGTH             lp_get_length(PARTITION_NVDM_OU)

#endif

#define VRAM_BASE      0x10000000

#define TCM_BASE       (uint32_t)_tcm_start
#define TCM_LENGTH     (uint32_t)_tcm_length

#define SYSRAM_BASE    (uint32_t)_sysram_start
#define SYSRAM_LENGTH  (uint32_t)_sysram_length

#define VSYSRAM_BASE   (uint32_t)_vsysram_start
#define VSYSRAM_LENGTH (uint32_t)_vsysram_length

#endif


#endif

