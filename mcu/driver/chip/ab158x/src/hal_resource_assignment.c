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

#include <string.h>

#include "hal_core_status.h"
#include "hal_platform.h"
#include "hal_resource_assignment.h"

#define        HAL_RESOURCE_TCM_BASE        (uint32_t)(0x04000000)
#define        HAL_RESOURCE_TCM_SIZE        (uint32_t)(256*1024)
#define        HAL_RESOURCE_TCM_LIMIT       (HAL_RESOURCE_TCM_BASE+HAL_RESOURCE_TCM_SIZE)

#define        HAL_RESOURCE_ROM_BASE        (uint32_t)(0x08000000)
#define        HAL_RESOURCE_ROM_SIZE        (uint32_t)((uint32_t)128*1024*1024)
#define        HAL_RESOURCE_ROM_LIMIT       (HAL_RESOURCE_ROM_BASE+HAL_RESOURCE_ROM_SIZE)


#define        HAL_RESOURCE_ALLOW_CONVERT       0x1
#define        HAL_RESOURCE_NOT_ALLOW_CONVERT   0x0


#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
/* MCU => DSP, check source address when converting */
extern uint32_t _share_data_start[] ;
extern uint32_t _share_bss_end[] ;
#define        M2D_SHARE_MEM_BEGIN          (uint32_t)(_share_data_start)
#define        M2D_SHARE_MEM_END            (uint32_t)(_share_bss_end)

#define        M2D_CROSS_CORE_MEM_BEGIN     (uint32_t)(HW_SYSRAM_PRIVATE_MEMORY_START)
#define        M2D_CROSS_CORE_MEM_END       (uint32_t)(HW_SYSRAM_PRIVATE_MEMORY_START+HW_SYSRAM_PRIVATE_MEMORY_LEN)


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


/* Obtain the return address of caller from link register */
#if defined(__GNUC__)
#define        GET_LR(x)                    x=(uint32_t)__builtin_return_address(0);    (void)x
#elif defined(__CC_ARM)
#define        GET_LR(x)                    x=(uint32_t)__return_address(0);    (void)x
#elif defined(__ICCARM__)
#define        GET_LR(x)                    x=(uint32_t)__get_LR(0);    (void)x
#endif /* __GNUC__ */


static uint32_t check_infra_address(uint32_t addr)
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


#define CHECK_INFRA_ADDR(x) { \
    uint32_t res = check_infra_address(x); \
    if(res != 0){ \
        log_hal_msgid_error("[memview] Infra address range check fail 0x%08X! Caller is 0x%08X", 2, x, lr);\
    }\
}

#else /* defined(__EXT_BOOTLOADER__) || !defined(__EXT_DA__) */

#define        CHECK_INFRA_ADDR(x)        (void)x
#define        GET_LR(x)                  (void)x

#endif /* !defined(__EXT_BOOTLOADER__) &&  !defined(__EXT_DA__) */


static uint32_t allow_convert_check(uint32_t addr)
{
    if (
        ((HAL_RESOURCE_TCM_BASE <= addr) && (addr < HAL_RESOURCE_TCM_LIMIT)) ||
        ((HAL_RESOURCE_ROM_BASE <= addr) && (addr < HAL_RESOURCE_ROM_LIMIT))
    ) {
        return HAL_RESOURCE_NOT_ALLOW_CONVERT;
    }
    return HAL_RESOURCE_ALLOW_CONVERT;
}


/* Private memory init */
void private_memory_init(void)
{
    /* Clear private memory at the beginning of the system initialization */
    memset((void *)HW_SYSRAM_PRIVATE_MEMORY_START, 0, HW_SYSRAM_PRIVATE_MEMORY_LEN);
}

/* Memory View Transform */


/* MCU non-cacheable convert to DSP0 non-cacheable */
uint32_t hal_memview_mcu_to_dsp0(uint32_t mcu_address)
{
    uint32_t dsp0_address = 0;
    uint32_t lr;
    GET_LR(lr);

    if (allow_convert_check(mcu_address) == HAL_RESOURCE_NOT_ALLOW_CONVERT) {
        return mcu_address;
    }
#ifdef HAL_CACHE_MODULE_ENABLED
#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
    if ( ((mcu_address >= M2D_SHARE_MEM_BEGIN) && (mcu_address < M2D_SHARE_MEM_END)) ||
         ((mcu_address >= M2D_CROSS_CORE_MEM_BEGIN) && (mcu_address < M2D_CROSS_CORE_MEM_END))
    ) {
#else
    if ((mcu_address >= 0x20000000) && (mcu_address < 0x40000000)) {
#endif
        dsp0_address = mcu_address + 0x40000000;
    } else {
        log_hal_msgid_error("[memview][M=>D] Input wrong address 0x%08X! Caller is 0x%08X", 2, mcu_address, lr);
        dsp0_address = mcu_address;
    }
#else
    dsp0_address = mcu_address;
#endif

    return dsp0_address;
}

/* MCU non-cacheable convert to infrasys (physical) */
uint32_t hal_memview_mcu_to_infrasys(uint32_t mcu_address)
{
    uint32_t infrasys_address = 0;
    uint32_t lr;
    GET_LR(lr);

    if (allow_convert_check(mcu_address) == HAL_RESOURCE_NOT_ALLOW_CONVERT) {
        return mcu_address;
    }
#ifdef HAL_CACHE_MODULE_ENABLED
    if ((mcu_address >= 0x20000000) && (mcu_address < 0x40000000)) {
        infrasys_address = mcu_address - 0x20000000;
    } else {
        log_hal_msgid_error("[memview][M=>I] Input wrong address 0x%08X!  Caller is 0x%08X", 2, mcu_address, lr);
        infrasys_address = mcu_address;
    }
#else
    infrasys_address = mcu_address;
#endif

    CHECK_INFRA_ADDR(infrasys_address);

    return infrasys_address;
}

/* dsp0 non-cacheable convert to MCU non-cacheable */
uint32_t hal_memview_dsp0_to_mcu(uint32_t dsp0_address)
{
    uint32_t mcu_address = 0;
    uint32_t lr;
    GET_LR(lr);

#ifdef HAL_CACHE_MODULE_ENABLED
    if ((dsp0_address >= 0x60000000) && (dsp0_address < 0x80000000)) {
        mcu_address = dsp0_address - 0x40000000;
    } else {
        log_hal_msgid_error("[memview][D=>M] Input wrong address 0x%08X!  Caller is 0x%08X", 2, dsp0_address, lr);
        mcu_address = dsp0_address;
    }
#else
    mcu_address = dsp0_address;
#endif

    return mcu_address;
}

/* dsp0 non-cacheable convert to infrasys(physical) */
uint32_t hal_memview_dsp0_to_infrasys(uint32_t dsp0_address)
{
    uint32_t infrasys_address = 0;
    uint32_t lr;
    GET_LR(lr);

#ifdef HAL_CACHE_MODULE_ENABLED
    if ((dsp0_address >= 0x60000000) && (dsp0_address < 0x80000000)) {
        infrasys_address = dsp0_address - 0x60000000;
    } else {
        log_hal_msgid_error("[memview][D=>I] Input wrong address 0x%08X!  Caller is 0x%08X", 2, dsp0_address, lr);
        infrasys_address = dsp0_address;
    }
#else
    infrasys_address = dsp0_address;
#endif

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

#ifdef HAL_CACHE_MODULE_ENABLED
    if ((infrasys_address < 0x20000000)) {
        mcu_address = infrasys_address + 0x20000000;
    } else {
        log_hal_msgid_error("[memview][I=>M] Input wrong address 0x%08X!  Caller is 0x%08X", 2, infrasys_address, lr);
        mcu_address = infrasys_address;
    }
#else
    mcu_address = infrasys_address;
#endif

    return mcu_address;
}

/* infrasys(physical) convert to dsp0 non-cacheable */
uint32_t hal_memview_infrasys_to_dsp0(uint32_t infrasys_address)
{
    uint32_t dsp0_address = 0;
    uint32_t lr;
    GET_LR(lr);

    CHECK_INFRA_ADDR(infrasys_address);

#ifdef HAL_CACHE_MODULE_ENABLED
    if ((infrasys_address < 0x20000000)) {
        dsp0_address = infrasys_address + 0x60000000;
    } else {
        log_hal_msgid_error("[memview][I=>D] Input wrong address 0x%08X!  Caller is 0x%08X", 2, infrasys_address, lr);
        dsp0_address = infrasys_address;
    }
#else
    dsp0_address = infrasys_address;
#endif

    return dsp0_address;
}
/* Memory View Transform end*/