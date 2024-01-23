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

/* Hal includes. */
#include "hal.h"
#include "syslog.h"
#include "bsp_gpio_ept_config.h"
#include "memory_attribute.h"
#include "hal_resource_assignment.h"
#include "exception_handler.h"

#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#include "nvdm_config_factory_reset.h"
#endif
#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
#include "systemhang_tracer.h"
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */
#ifdef MTK_BOOTREASON_CHECK_ENABLE
#include "bootreason_check.h"
#endif /* MTK_BOOTREASON_CHECK_ENABLE */
#ifdef MTK_MINIDUMP_ENABLE
#include "offline_dump.h"
#endif /* MTK_MINIDUMP_ENABLE */
#include "serial_port_assignment.h"
#include "verno.h"
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#endif
#ifdef AIR_PMU_MANAGEMENT_ENABLE
#include "pmu_management.h"
#endif

#ifdef AIR_BTA_IC_PREMIUM_G3
__attribute__((__section__(".log_chip_option"))) __attribute__((used)) static const char chip_option[] = "AIR_BTA_IC_PREMIUM_G3";
#elif defined(AIR_BTA_IC_PREMIUM_G2)
__attribute__((__section__(".log_chip_option"))) __attribute__((used)) static const char chip_option[] = "AIR_BTA_IC_PREMIUM_G2";
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
__attribute__((__section__(".log_chip_option"))) __attribute__((used)) static const char chip_option[] = "AIR_BTA_IC_STEREO_HIGH_G3";
#endif

/**
* @brief       This function is to config system clock.
* @param[in]   None.
* @return      None.
*/
static void SystemClock_Config(void)
{
    hal_clock_init();
#ifdef HAL_DVFS_MODULE_ENABLED
    hal_dvfs_init();
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
    /* The requirement of gaming product is to run at the highest frequency
     * at the beginning of the boot, and to run at low frequency after USB suspend. */
    hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
#endif /* AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE */
#endif
}
#ifdef HAL_CACHE_MODULE_ENABLED
/**
* @brief       This function is to initialize cache controller.
* @param[in]   None.
* @return      None.
*/
static void cache_init(void)
{
    hal_cache_region_t region, region_number;

    /* Max region number is 16 */
    hal_cache_region_config_t region_cfg_tbl[] = {
        /* cacheable address, cacheable size(both MUST be 4k bytes aligned) */
        {CM4_BASE, CM4_LENGTH},
        /* virtual sysram */
        {VSYSRAM_BASE, VSYSRAM_LENGTH}
    };

    region_number = (hal_cache_region_t)(sizeof(region_cfg_tbl) / sizeof(region_cfg_tbl[0]));

    hal_cache_init();
    hal_cache_set_size(HAL_CACHE_SIZE_16KB);
    for (region = HAL_CACHE_REGION_0; region < region_number; region++) {
        hal_cache_region_config(region, &region_cfg_tbl[region]);
        hal_cache_region_enable(region);
    }
    for (; region < HAL_CACHE_REGION_MAX; region++) {
        hal_cache_region_disable(region);
    }
    hal_cache_enable();
}
#endif /* HAL_CACHE_MODULE_ENABLED */


#ifdef HAL_MPU_MODULE_ENABLED
//TCM: VECTOR TABLE + CODE+RO DATA
extern uint32_t _tcm_text_start;
extern uint32_t _tcm_text_end;
//SYSRAM: CODE+RO DATA
extern uint32_t _sysram_code_start;
extern uint32_t _sysram_code_end;
//STACK END
extern uint32_t _image_stack_zi_base;

/* ESC region address and size */
#if (defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3))
#define ESC_REGION_ADDR_START 0x0
#define ESC_REGION_SIZE 0x4000000
#endif


#if (defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3))
/**
 * @brief       Calculate actual bit value of region size.
 * @param[in]   region_size: actual region size.
 * @return      Corresponding bit value of region size for MPU setting.
 */
static uint32_t calculate_mpu_region_size(uint32_t region_size)
{
    uint32_t count;

    if (region_size < 32) {
        return 0;
    }
    for (count = 0; ((region_size  & 0x80000000) == 0); count++, region_size  <<= 1);
    return 30 - count;
}
#endif

/**
* @brief       This function is to initialize MPU.
* @param[in]   None.
* @return      None.
*/
static void mpu_init(void)
{
    hal_mpu_region_t region, region_number;

#if defined(AIR_BTA_IC_PREMIUM_G3)
    /* MAX region number is 16 */
    const hal_mpu_region_config_t region_information[] = {
        /* mpu_region_start_address, mpu_region_end_address, mpu_region_access_permission, mpu_subregion_mask, mpu_xn */
        {(uint32_t) ROM_BASE, (uint32_t)(ROM_BASE + ROM_LENGTH), HAL_MPU_RO_BY_ANY, FALSE}, //Set FLASH region as RO
        {(uint32_t) &_tcm_text_start, (uint32_t) &_tcm_text_end, HAL_MPU_RO_BY_ANY, FALSE},//Vector table+TCM code+TCM rodata
        {(uint32_t) &_sysram_code_start, (uint32_t) &_sysram_code_end, HAL_MPU_RO_BY_ANY, FALSE}, //SYSRAM code+SYSRAM rodata
        {(uint32_t) &_image_stack_zi_base, (uint32_t) &_image_stack_zi_base + 32, HAL_MPU_RO_BY_ANY, TRUE}, //Stack end check for stack overflow
#ifndef HAL_ESC_MODULE_ENABLED
        /* Use two same MPU channels with the same settings
         * to achieve the effect of simulation NO-ACCESS.
         */
        /* ESC cacheable region info */
        {ESC_REGION_ADDR_START, ESC_REGION_ADDR_START + ESC_REGION_SIZE, HAL_MPU_RO_BY_ANY, TRUE},
        {ESC_REGION_ADDR_START, ESC_REGION_ADDR_START + ESC_REGION_SIZE, HAL_MPU_RO_BY_ANY, TRUE},
        /* ESC non-cacheable region info */
        {ESC_REGION_ADDR_START | (1 << 29), (ESC_REGION_ADDR_START | (1 << 29)) + ESC_REGION_SIZE, HAL_MPU_RO_BY_ANY, TRUE},
        {ESC_REGION_ADDR_START | (1 << 29), (ESC_REGION_ADDR_START | (1 << 29)) + ESC_REGION_SIZE, HAL_MPU_RO_BY_ANY, TRUE},
#endif
    };
#elif defined (AIR_BTA_IC_STEREO_HIGH_G3)
    hal_mpu_region_config_t region_config;
    typedef struct {
        uint32_t mpu_region_base_address;/**< MPU region start address. */
        uint32_t mpu_region_end_address;/**< MPU region end address. */
        hal_mpu_access_permission_t mpu_region_access_permission;/**< MPU region access permission. */
        uint8_t mpu_subregion_mask;/**< MPU sub region mask. */
        bool mpu_xn;/**< XN attribute of MPU, if set TRUE, execution of an instruction fetched from the corresponding region is not permitted. */
    } mpu_region_information_t;

    /* MAX region number is 16 */
    mpu_region_information_t region_information[] = {
        /* mpu_region_start_address, mpu_region_end_address, mpu_region_access_permission, mpu_subregion_mask, mpu_xn */
        {(uint32_t) ROM_BASE, (uint32_t)(ROM_BASE + ROM_LENGTH), HAL_MPU_READONLY, 0x0, FALSE}, //Set FLASH region as RO
        {(uint32_t) &_tcm_text_start, (uint32_t) &_tcm_text_end, HAL_MPU_READONLY, 0x0, FALSE},//Vector table+TCM code+TCM rodata
        {(uint32_t) &_sysram_code_start, (uint32_t) &_sysram_code_end, HAL_MPU_READONLY, 0x0, FALSE}, //SYSRAM code+SYSRAM rodata
        {(uint32_t) &_sysram_code_start - VRAM_BASE, (uint32_t) &_sysram_code_end - VRAM_BASE, HAL_MPU_NO_ACCESS, 0x0, TRUE}, //Virtual memory
        {(uint32_t) &_image_stack_zi_base, (uint32_t) &_image_stack_zi_base + 32, HAL_MPU_READONLY, 0x0, TRUE}, //Stack end check for stack overflow
        {(uint32_t) 0, (uint32_t) 0x400000, HAL_MPU_NO_ACCESS, 0x0, TRUE}, //Set EMI address range as no access
    };
#elif defined (AIR_BTA_IC_PREMIUM_G2)
    hal_mpu_region_config_t region_config;
    typedef struct {
        uint32_t mpu_region_base_address;/**< MPU region start address. */
        uint32_t mpu_region_end_address;/**< MPU region end address. */
        hal_mpu_access_permission_t mpu_region_access_permission;/**< MPU region access permission. */
        uint8_t mpu_subregion_mask;/**< MPU sub region mask. */
        bool mpu_xn;/**< XN attribute of MPU, if set TRUE, execution of an instruction fetched from the corresponding region is not permitted. */
    } mpu_region_information_t;

    /* TCM: VECTOR TABLE + CODE + RO DATA. */
    extern uint32_t Image$$TCM$$RO$$Base;
    extern uint32_t Image$$TCM$$RO$$Limit;
    /* SYSRAM: CODE+RO DATA. */
    extern uint32_t Image$$CACHED_SYSRAM_TEXT$$Base;
    extern uint32_t Image$$CACHED_SYSRAM_TEXT$$Limit;
    /* STACK END. */
    extern unsigned int Image$$STACK$$ZI$$Base[];
    /* MAX region number is 8. */
    mpu_region_information_t region_information[] = {
        /* mpu_region_start_address, mpu_region_end_address, mpu_region_access_permission, mpu_subregion_mask, mpu_xn. */
        {(uint32_t) ROM_BASE, (uint32_t)(ROM_BASE + ROM_LENGTH), HAL_MPU_READONLY, 0x0, FALSE}, //Set FLASH region as RO
        {(uint32_t) &Image$$TCM$$RO$$Base, (uint32_t) &Image$$TCM$$RO$$Limit, HAL_MPU_READONLY, 0x0, FALSE},//Vector table+TCM code+TCM rodata
        {(uint32_t) &Image$$CACHED_SYSRAM_TEXT$$Base, (uint32_t) &Image$$CACHED_SYSRAM_TEXT$$Limit, HAL_MPU_READONLY, 0x0, FALSE}, //SYSRAM code+SYSRAM rodata
        {(uint32_t) &Image$$CACHED_SYSRAM_TEXT$$Base - VRAM_BASE, (uint32_t) &Image$$CACHED_SYSRAM_TEXT$$Limit - VRAM_BASE, HAL_MPU_NO_ACCESS, 0x0, TRUE}, //Virtual memory
        {(uint32_t) &Image$$STACK$$ZI$$Base, (uint32_t) &Image$$STACK$$ZI$$Base + 32, HAL_MPU_READONLY, 0x0, TRUE}, //Stack end check for stack overflow
        {(uint32_t) 0, (uint32_t) 0x400000, HAL_MPU_NO_ACCESS, 0x0, TRUE} //Set EMI address range as no access
    };
#endif
    hal_mpu_config_t mpu_config = {
        /* PRIVDEFENA, HFNMIENA */
        TRUE, TRUE
    };

    region_number = (hal_mpu_region_t)(sizeof(region_information) / sizeof(region_information[0]));

    hal_mpu_init(&mpu_config);
    for (region = HAL_MPU_REGION_0; region < region_number; region++) {
#if (defined (AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3))
        /* Updata region information to be configured. */
        region_config.mpu_region_address = region_information[region].mpu_region_base_address;
        region_config.mpu_region_size = (hal_mpu_region_size_t) calculate_mpu_region_size(region_information[region].mpu_region_end_address - region_information[region].mpu_region_base_address);
        region_config.mpu_region_access_permission = region_information[region].mpu_region_access_permission;
        region_config.mpu_subregion_mask = region_information[region].mpu_subregion_mask;
        region_config.mpu_xn = region_information[region].mpu_xn;
        hal_mpu_region_configure(region, &region_config);

#elif defined (AIR_BTA_IC_PREMIUM_G3)
        hal_mpu_region_configure(region, &region_information[region]);
#endif
        hal_mpu_region_enable(region);
    }
    /* make sure unused regions are disabled */
    for (; region < HAL_MPU_REGION_MAX; region++) {
        hal_mpu_region_disable(region);
    }
    hal_mpu_enable();
}
#endif /* HAL_MPU_MODULE_ENABLED */

extern  void SFI_Dev_Command(const uint16_t CS, const uint32_t cmd);

/**
* @brief       This function is to setup system hardware, such as cache init, uart init etc.
* @param[in]   None.
* @return      None.
*/
static void prvSetupHardware(void)
{
    /* log service pre-init */
    log_uart_init();
    LOG_MSGID_I(common, "log_uart_init done.", 0);

    /* TODO: sleep module init */
    hal_sleep_manager_init();

#ifdef HAL_CACHE_MODULE_ENABLED
    cache_init();
#endif

#ifdef HAL_MPU_MODULE_ENABLED
    mpu_init();
#endif


#ifdef FPGA_ENV
    /* TODO: exit 4byte address mode for FPGA, only FPGA enable */
    SFI_Dev_Command(0, 0xE9);
#endif

    /* TODO: flash module init */
    hal_flash_init();

    /* nvic module init */
    hal_nvic_init();

    /* TODO: ccni module init */
    hal_ccni_init();

    /* TODO: rtc module init */
    hal_rtc_init();

    /* ESC module initialize. */
#ifdef HAL_ESC_MODULE_ENABLED
    hal_esc_init();
#endif
}


#ifdef MTK_LAYOUT_PARTITION_ENABLE
const partition_t protect_partition[] = {
    PARTITION_BL,
    PARTITION_SECURITY_HEADER,
    PARTITION_SECURITY_HEADER2,
    PARTITION_MCU,
    PARTITION_DSP0,
    PARTITION_ROFS,
};
#endif

/**
* @brief       This function is to do system initialization, eg: system clock, hardware and logging port.
* @param[in]   None.
* @return      None.
*/
void system_init(void)
{
    syslog_port_type_t syslog_type;
    uint32_t           syslog_idx;

#ifdef AIR_BTA_IC_PREMIUM_G3
    /* Security Top Config,
        Enable reserved special 512k sysram for platform use */
    SPECIAL_SYSRAM_SEL |= (1 << 8);
#endif

    /* Init private memory at the beginning of the system initialization */
    private_memory_init();

#ifdef AIR_BTA_IC_PREMIUM_G3
#ifndef AIR_CPU_IN_SECURITY_MODE
    /* Exception share memory init */
    extern void exception_ns_share_memory_init();
    exception_ns_share_memory_init();
#endif
#endif

#if (defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3))
#ifdef MTK_LAYOUT_PARTITION_ENABLE
    /* layout partition parser initialize before cache_init. */
    lp_init();
    lp_register_readonly_partitions(protect_partition, sizeof(protect_partition) / sizeof(partition_t));
#endif
#endif

#ifndef FPGA_ENV
    /* Call this function to indicate the system initialize is ongoing. */
    hal_core_status_write(HAL_CORE_MCU, HAL_CORE_INIT);

    /* SystemClock Config */
    SystemClock_Config();
    SystemCoreClockUpdate();

    /* bsp_ept_gpio_setting_init() under driver/board/ab155x_evk/ept will initialize the GPIO settings
     * generated by easy pinmux tool (ept). ept_*.c and ept*.h are the ept files and will be used by
     * bsp_ept_gpio_setting_init() for GPIO pinumux setup.
     */
    bsp_ept_gpio_setting_init();
#endif /* FPGA_ENV */

    /* Configure the hardware ready to run the project. */
    prvSetupHardware();

#ifdef MTK_NVDM_ENABLE
    /* nvdm init */
    nvdm_init();
#endif

#ifdef HAL_PMU_MODULE_ENABLED
    pmu_init();
#endif

#ifndef FPGA_ENV
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
    /* Initialize the battery function */
    battery_management_init();
#endif

#if defined(HAL_DCXO_MODULE_ENABLED)
    hal_dcxo_init();
#endif

#endif /* FPGA_ENV */

#ifdef MTK_NVDM_ENABLE
    /* Check if there's a factory reset and do nvdm clear. */
    factory_rst_reserved_nvdm_item_list_check();
#endif

    /* log init */
    GET_SERIAL_PORT_TYPE_INDEX_FROM_PORT_DEV(CONFIG_SYSLOG_RUNNING_STAGE_PORT, syslog_type, syslog_idx);
    log_init(syslog_type, syslog_idx);
    log_set_filter();

    /* sys init done log */
    LOG_I(common, "[MCU]system initialize done[%s]", build_date_time_str);
    LOG_I(common, "FirmWare Version: %s", MTK_FW_VERSION);
#if defined (AIR_PURE_GAMING_ENABLE) || defined(AIR_HID_BT_HOGP_ENABLE)
    /* add workaround for MP tool, since MP tool must need this log, BTA DSP can output this log too */
    LOG_I(common, "[DSP0]system initialize done[%s]", build_date_time_str);
#endif

#ifdef MTK_NVDM_ENABLE
    exception_dump_config_init();
#endif

#if (defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3))
    /* config wdt reset as pmic mode */
    extern exception_config_mode_t exception_config_mode ;
    if (exception_config_mode.exception_mode_t.systemhang_pmic_mode) {
        pmu_select_wdt_mode(PMIC_WDT_REG);
    }
#endif
#ifndef FPGA_ENV
#ifdef MTK_MINIDUMP_ENABLE
    /* offline dump service init */
    offline_dump_region_init();
#endif /* MTK_MINIDUMP_ENABLE */

#ifdef MTK_BOOTREASON_CHECK_ENABLE
    /* bootreason service init */
    bootreason_init();
#endif /* MTK_BOOTREASON_CHECK_ENABLE */

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
    /* system hang tracer service init */
    systemhang_tracer_init(15);
#else
    /* Bootloader will config wdt reset mode for 9s,So,we need to disable wdt */
    hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */
#endif /* FPGA_ENV */
}


