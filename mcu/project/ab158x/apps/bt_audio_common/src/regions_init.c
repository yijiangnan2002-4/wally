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

#include "exception_handler.h"
#include "air_chip.h"
#include "hal_resource_assignment.h"
#include "hal_dump_peripherals_register.h"
/******************************************************************************/
/*            Memory Regions Definition                                       */
/******************************************************************************/
#if defined(__GNUC__)

#if !defined(AIR_BTA_IC_PREMIUM_G2) && !defined(AIR_BTA_IC_STEREO_HIGH_G3)

extern unsigned int _text_start[];
extern unsigned int _text_end[];
extern unsigned int _sysram_code_start[];
extern unsigned int _sysram_code_end[];
extern unsigned int _cached_sysram_data_start[];
extern unsigned int _bss_end[];
extern unsigned int _noncached_sysram_rw_start[];
extern unsigned int _noncached_sysram_zi_end[];
extern unsigned int _share_data_start[];
extern unsigned int _share_data_end[];
extern unsigned int _share_bss_start[];
extern unsigned int _share_bss_end[];
extern unsigned int _tcm_text_start[];
extern unsigned int _tcm_zi_end[];
extern unsigned int _image_stack_zi_base[];
extern unsigned int _image_stack_zi_limit[];
extern unsigned int _sysram_bt_start[];
extern unsigned int _sysram_bt_limit[];

#ifdef MTK_NVDM_ENABLE
extern unsigned int _nvdm_start[];
extern unsigned int _nvdm_end[];
extern unsigned int _nvdm_ou_start[];
extern unsigned int _nvdm_ou_end[];
#endif /* MTK_NVDM_ENABLE */
const memory_region_type memory_regions[] = {
#ifdef MTK_NVDM_ENABLE
    {"nvdm_ou", _nvdm_ou_start, _nvdm_ou_end, 1},
    {"nvdm", _nvdm_start, _nvdm_end, 1},
#endif

    {"sysram_bt", _sysram_bt_start, _sysram_bt_limit, 1},
    {"text", _text_start, _text_end, 0},
    {"cached_sysram_text", _sysram_code_start, _sysram_code_end, 1},
    {"cached_sysram_data", _cached_sysram_data_start, _bss_end, 1},
    {"noncached_sysram_data", _noncached_sysram_rw_start, _noncached_sysram_zi_end, 1},
    {"share_rwdata", _share_data_start, _share_data_end, 1},
    {"share_zidata", _share_bss_start, _share_bss_end, 1},
    {"dsp_swla", (unsigned int *)0x04580000, (unsigned int *)(0x04580000 + 1024 * 1024), 1},
    {"tcm", _tcm_text_start, _tcm_zi_end, 1},
    {"stack", _image_stack_zi_base, _image_stack_zi_limit, 1},
    {"scs", (unsigned int *)SCS_BASE, (unsigned int *)(SCS_BASE + 0x1000), 1},
    {"dwt", (unsigned int *)(0xE0001000), (unsigned int *)(0xE0002000), 1},
    {"private_memory", (unsigned int *)(HW_SYSRAM_PRIVATE_MEMORY_START), (unsigned int *)(HW_SYSRAM_PRIVATE_MEMORY_START + HW_SYSRAM_PRIVATE_MEMORY_LEN), 1},
#ifdef HAL_DUMP_MODULE_REGISTER_ENABLE
    HAL_DUMP_PERIPHERAL_REGISTER
#endif

    {0}
};

#ifdef MTK_SWLA_ENABLE
extern unsigned int _bss_end[];
extern unsigned int _image_swla_base[];
extern unsigned int _image_swla_limit[];
extern unsigned int _share_bss_end[];

void SLA_get_region(uint32_t *pxBase, uint32_t *pxLen)
{
    *pxBase = (((uint32_t)_share_bss_end & (~VRAM_BASE)) + 0x20) & ~(0x20 - 1); /* align up to 32Byte */
    if (((uint32_t)_image_swla_limit & ~(0x20 - 1)) - *pxBase <= 0) {
        *pxLen = 0x0;
    } else {
        *pxLen = (uint32_t)(((uint32_t)_image_swla_limit - *pxBase)) & ~(0x20 - 1);    /* swla buffer area is free ram + swla  reserve area */
    }
}
#endif /* MTK_SWLA_ENABLE */
#else /* defined(AIR_BTA_IC_PREMIUM_G2) */
extern unsigned int Image$$TEXT$$Base[];
extern unsigned int Image$$TEXT$$Limit[];
extern unsigned int Image$$CACHED_SYSRAM_TEXT$$Base[];
extern unsigned int Image$$CACHED_SYSRAM_TEXT$$Limit[];
extern unsigned int Image$$CACHED_SYSRAM_DATA$$RW$$Base[];
extern unsigned int Image$$CACHED_SYSRAM_DATA$$ZI$$Limit[];
extern unsigned int Image$$NONCACHED_SYSRAM_DATA$$Base[];
extern unsigned int Image$$NONCACHED_SYSRAM_ZI$$Limit[];
extern unsigned int Image$$SHARE_DATA$$Base[];
extern unsigned int Image$$SHARE_DATA$$Limit[];
extern unsigned int Image$$SHARE_ZI$$Base[];
extern unsigned int Image$$SHARE_ZI$$Limit[];
extern unsigned int Image$$TCM$$RO$$Base[];
extern unsigned int Image$$TCM$$ZI$$Limit[];
extern unsigned int Image$$STACK$$ZI$$Base[];
extern unsigned int Image$$STACK$$ZI$$Limit[];
extern unsigned int Image$$BT_SYSRAM_RW_DATA$$RW$$Base[];
extern unsigned int Image$$BT_SYSRAM_ZI_DATA$$RW$$Limit[];

#ifdef MTK_NVDM_ENABLE
extern unsigned int Image$$NVDM$$ZI$$Base[];
extern unsigned int Image$$NVDM$$ZI$$Limit[];
extern unsigned int _nvdm_ota_start[];
extern unsigned int _nvdm_ota_end[];
#endif /* MTK_NVDM_ENABLE */
const memory_region_type memory_regions[] = {
    {"text", Image$$TEXT$$Base, Image$$TEXT$$Limit, 0},
#ifdef MTK_NVDM_ENABLE
    {"nvdm_ota", _nvdm_ota_start, _nvdm_ota_end, 1},
    {"nvdm", Image$$NVDM$$ZI$$Base, Image$$NVDM$$ZI$$Limit, 1},
#endif
    {"cached_sysram_text", Image$$CACHED_SYSRAM_TEXT$$Base, Image$$CACHED_SYSRAM_TEXT$$Limit, 1},
    {"cached_sysram_data", Image$$CACHED_SYSRAM_DATA$$RW$$Base, Image$$CACHED_SYSRAM_DATA$$ZI$$Limit, 1},
    {"noncached_sysram_data", Image$$NONCACHED_SYSRAM_DATA$$Base, Image$$NONCACHED_SYSRAM_ZI$$Limit, 1},
    {"share_rwdata", Image$$SHARE_DATA$$Base, Image$$SHARE_DATA$$Limit, 1},
    {"share_zidata", Image$$SHARE_ZI$$Base, Image$$SHARE_ZI$$Limit, 1},
    {"tcm", Image$$TCM$$RO$$Base, Image$$TCM$$ZI$$Limit, 1},
    {"stack", Image$$STACK$$ZI$$Base, Image$$STACK$$ZI$$Limit, 1},
    {"scs", (unsigned int *)SCS_BASE, (unsigned int *)(SCS_BASE + 0x1000), 1},
    {"dwt", (unsigned int *)(0xE0001000), (unsigned int *)(0xE0002000), 1},
    {"private_memory", (unsigned int *)(HW_SYSRAM_PRIVATE_MEMORY_START), (unsigned int *)(HW_SYSRAM_PRIVATE_MEMORY_START + HW_SYSRAM_PRIVATE_MEMORY_LEN), 1},
    {"sysram_bt", Image$$BT_SYSRAM_RW_DATA$$RW$$Base, Image$$BT_SYSRAM_ZI_DATA$$RW$$Limit, 1},
#ifdef HAL_DUMP_MODULE_REGISTER_ENABLE
    HAL_DUMP_PERIPHERAL_REGISTER
#endif
    {0}
};

#ifdef MTK_SWLA_ENABLE
extern unsigned int Image$$CACHED_SYSRAM_DATA$$ZI$$Limit[];
extern unsigned int Image$$SWLA$$Base[];
extern unsigned int Image$$SWLA$$Limit[];
extern unsigned int Image$$SHARE_ZI$$Limit[];

void SLA_get_region(uint32_t *pxBase, uint32_t *pxLen)
{
    *pxBase = (((uint32_t)Image$$SHARE_ZI$$Limit | VRAM_BASE) + 0x20) & ~(0x20 - 1);  /* Align up to 32Byte. */
    if (((uint32_t)Image$$SWLA$$Limit & ~(0x20 - 1)) - *pxBase <= 0) {
        *pxLen = 0x0;
    } else {
        *pxLen = (uint32_t)(((uint32_t)Image$$SWLA$$Limit - *pxBase)) & ~(0x20 - 1);    /* SWLA buffer area is free ram + swla  reserve area. */
    }
}
#endif /* MTK_SWLA_ENABLE */
#endif

#endif /* __GNUC__ */


