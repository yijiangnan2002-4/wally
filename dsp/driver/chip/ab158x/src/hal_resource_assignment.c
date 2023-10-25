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

#include "hal_core_status.h"
#include "hal_platform.h"
#include "hal_resource_assignment.h"


#define        HAL_RESOURCE_ALLOW_CONVERT       0x1
#define        HAL_RESOURCE_NOT_ALLOW_CONVERT   0x0

#define        HAL_RESOURCE_ROM_BASE        (uint32_t)(0x08000000)
#define        HAL_RESOURCE_ROM_SIZE        (uint32_t)((uint32_t)128*1024*1024)
#define        HAL_RESOURCE_ROM_LIMIT       (HAL_RESOURCE_ROM_BASE+HAL_RESOURCE_ROM_SIZE)

/* Check Infrasys address */
#define        INFRA_TCM_BEGIN              (uint32_t)(0x04000000)
#define        INFRA_TCM_END                (uint32_t)(0x04040000)

#define        INFRA_SYSRAM_BEGIN           (uint32_t)(0x04200000)
#define        INFRA_SYSRAM_END             (uint32_t)(0x04300000)

#define        INFRA_ROM_BEGIN              (uint32_t)(0x08000000)
#define        INFRA_ROM_END                (uint32_t)(0x10000000)

#define        INFRA_IRAM_BEGIN             (uint32_t)(0x04700000)
#define        INFRA_IRAM_END               (uint32_t)(0x04740000)

#define        INFRA_DRAM_BEGIN             (uint32_t)(0x04580000)
#define        INFRA_DRAM_END               (uint32_t)(0x04680000)


#define        CROSS_CORE_MEM_BEGIN         (uint32_t)(HW_SYSRAM_PRIVATE_MEMORY_START)
#define        CROSS_CORE_MEM_END           (uint32_t)(HW_SYSRAM_PRIVATE_MEMORY_START+HW_SYSRAM_PRIVATE_MEMORY_LEN)


/* Obtain the return address of caller from link register */
#if defined(__GNUC__)
#define        GET_LR(x)                    x=(uint32_t)__builtin_return_address(0)
#elif defined(__CC_ARM)
#define        GET_LR(x)                    x=(uint32_t)__return_address(0)
#elif defined(__ICCARM__)
#define        GET_LR(x)                    x=(uint32_t)__get_LR(0)
#endif /* __GNUC__ */


/* Because on the DSP side, logging depends on the API of the memview class,
 * so this API cannot be logged, otherwise it will cause recursion,
 * so it is decided to use global variables to record the required lr,
 * input parameters and other information. */
#define        IDX_MCU2DSP0                 0
#define        IDX_MCU2INFRA                1
#define        IDX_DSP02MCU                 2
#define        IDX_DSP02INFRA               3
#define        IDX_INFRA2MCU                4
#define        IDX_INFRA2DSP0               5

typedef struct{
    uint32_t lr;        /* caller */
    uint32_t addr;      /* input address */
} hal_memview_rcd_t;

hal_memview_rcd_t g_memview_rcd[6];


extern void platform_assert(const char *expr, const char *file, int line);


ATTR_TEXT_IN_IRAM static uint32_t allow_convert_check(uint32_t addr)
{
    if (
        ((HAL_RESOURCE_ROM_BASE <= addr) && (addr < HAL_RESOURCE_ROM_LIMIT)) ||
        ((INFRA_IRAM_BEGIN <= addr) && (addr < INFRA_IRAM_END)) ||
        ((INFRA_DRAM_BEGIN <= addr) && (addr < INFRA_DRAM_END))
    ) {
        return HAL_RESOURCE_NOT_ALLOW_CONVERT;
    }
    return HAL_RESOURCE_ALLOW_CONVERT;
}


ATTR_TEXT_IN_IRAM static uint32_t check_infra_address(uint32_t addr)
{
    if( ((addr >= INFRA_TCM_BEGIN) && (addr < INFRA_TCM_END)) ||
        ((addr >= INFRA_SYSRAM_BEGIN) && (addr < INFRA_SYSRAM_END)) ||
        ((addr >= INFRA_ROM_BEGIN) && (addr < INFRA_ROM_END)) ||
        ((addr >= INFRA_IRAM_BEGIN) && (addr < INFRA_IRAM_END)) ||
        ((addr >= INFRA_DRAM_BEGIN) && (addr < INFRA_DRAM_END))
    ){
        return 0;
    }
    return 1;
}


ATTR_TEXT_IN_IRAM static uint32_t check_dsp_address(uint32_t addr)
{
    if( ((addr >= CROSS_CORE_MEM_BEGIN) && (addr < CROSS_CORE_MEM_END)) ||
        ((addr >= INFRA_ROM_BEGIN) && (addr < INFRA_ROM_END)) ||
        ((addr >= INFRA_IRAM_BEGIN) && (addr < INFRA_IRAM_END)) ||
        ((addr >= INFRA_DRAM_BEGIN) && (addr < INFRA_DRAM_END))
    ){
        return 0;
    }
    return 1;
}

#if 0

#define CHECK_INFRA_ADDR(x) { \
    uint32_t res = check_infra_address(x); \
    if(res != 0){ \
        platform_assert("[memview] Check infrasys address fail.", __FILE__, __LINE__); \
    }\
}

#define RCD_AND_ASSERT(IDX, ADDR) { \
    g_memview_rcd[IDX].lr = lr; \
    g_memview_rcd[IDX].addr = ADDR; \
    platform_assert("[memview] Wrong Input Address", __FILE__, __LINE__); \
}

#else

#define CHECK_INFRA_ADDR(x) { \
    uint32_t res = check_infra_address(x); \
    if(res != 0){ \
    }\
}

#define RCD_AND_ASSERT(IDX, ADDR) { \
    g_memview_rcd[IDX].lr = lr; \
    g_memview_rcd[IDX].addr = ADDR; \
}

#endif

/* Memory View Transform */

/* In the following nouns
    MCU  means: non-cacheable region of MCU.
    DSP0 means: non-cacheable region of DSP0.
    infrasys means: physical address. Could be non-cacheable in MCU view, cacheable in DSP0 view.
*/

/* MCU non-cacheable convert to DSP0 non-cacheable */
ATTR_TEXT_IN_IRAM uint32_t hal_memview_mcu_to_dsp0(uint32_t mcu_address)
{
    uint32_t dsp0_address = 0;
    uint32_t lr;
    GET_LR(lr);

    if ((mcu_address >= 0x20000000) && (mcu_address < 0x40000000)) {
        dsp0_address = mcu_address + 0x40000000;
    } else {
        dsp0_address = mcu_address;
        if(0 != check_dsp_address(dsp0_address)){
            RCD_AND_ASSERT(IDX_MCU2DSP0, mcu_address);
        }
    }

    return dsp0_address;
}

/* MCU non-cacheable convert to infrasys (physical) */
ATTR_TEXT_IN_IRAM uint32_t hal_memview_mcu_to_infrasys(uint32_t mcu_address)
{
    uint32_t infrasys_address = 0;
    uint32_t lr;
    GET_LR(lr);

    if ((mcu_address >= 0x20000000) && (mcu_address < 0x40000000)) {
        infrasys_address = mcu_address - 0x20000000;
    } else {
        RCD_AND_ASSERT(IDX_MCU2INFRA, mcu_address);
        infrasys_address = mcu_address;
    }

    CHECK_INFRA_ADDR(infrasys_address);

    return infrasys_address;
}

/* DSP0 non-cacheable convert to MCU non-cacheable */
ATTR_TEXT_IN_IRAM uint32_t hal_memview_dsp0_to_mcu(uint32_t dsp0_address)
{
    uint32_t mcu_address = 0;
    uint32_t lr;
    GET_LR(lr);

    if (allow_convert_check(dsp0_address) == HAL_RESOURCE_NOT_ALLOW_CONVERT) {
        return dsp0_address;
    }

    if ((dsp0_address >= 0x60000000) && (dsp0_address < 0x80000000)) {
        mcu_address = dsp0_address - 0x40000000;
    } else {
        RCD_AND_ASSERT(IDX_DSP02MCU, dsp0_address);
        mcu_address = dsp0_address;
    }

    return mcu_address;
}

/* DSP0 non-cacheable convert to infrasys(physical) */
ATTR_TEXT_IN_IRAM uint32_t hal_memview_dsp0_to_infrasys(uint32_t dsp0_address)
{
    uint32_t infrasys_address = 0;
    uint32_t lr;
    GET_LR(lr);

    if (allow_convert_check(dsp0_address) == HAL_RESOURCE_NOT_ALLOW_CONVERT) {
        return dsp0_address;
    }

    if ((dsp0_address >= 0x60000000) && (dsp0_address < 0x80000000)) {
        infrasys_address = dsp0_address - 0x60000000;
    } else {
        RCD_AND_ASSERT(IDX_DSP02INFRA, dsp0_address);
        infrasys_address = dsp0_address;
    }

    CHECK_INFRA_ADDR(infrasys_address);

    return infrasys_address;
}

/* infrasys(physical) convert to MCU non-cacheable */
uint32_t hal_memview_infrasys_to_mcu(uint32_t infrasys_address)
{
    uint32_t mcu_address = 0;
    uint32_t lr;
    GET_LR(lr);

    CHECK_INFRA_ADDR(infrasys_address);

    if ((infrasys_address < 0x20000000)) {
        mcu_address = infrasys_address + 0x20000000;
    } else {
        RCD_AND_ASSERT(IDX_INFRA2MCU, infrasys_address);
        mcu_address = infrasys_address;
    }

    return mcu_address;
}

/* infrasys(physical) convert to DSP0 non-cacheable */
ATTR_TEXT_IN_IRAM uint32_t hal_memview_infrasys_to_dsp0(uint32_t infrasys_address)
{
    uint32_t dsp0_address = 0;
    uint32_t lr;
    GET_LR(lr);

    CHECK_INFRA_ADDR(infrasys_address);

    if ((infrasys_address < 0x20000000)) {
        dsp0_address = infrasys_address + 0x60000000;
    } else {
        RCD_AND_ASSERT(IDX_INFRA2DSP0, infrasys_address);
        dsp0_address = infrasys_address;
    }

    return dsp0_address;
}
/* Memory View Transform end*/

