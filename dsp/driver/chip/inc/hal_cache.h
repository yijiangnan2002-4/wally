/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#ifndef __HAL_CACHE_H__
#define __HAL_CACHE_H__
#include "hal_platform.h"

#ifdef HAL_CACHE_MODULE_ENABLED

/**
 * @addtogroup HAL
 * @{
 * @addtogroup CACHE
 * @{
 * This section describes the programming interfaces of the CACHE controller.
 * There are two CACHE controllers (I-CACHE and D-CACHE) that allow the user to set the CACHE size, configure the cacheable region(s), enable or disable the CACHE and to perform CACHE maintenance operations, such as invalidate and flush. These maintenance operations may be performed on the entire CACHE or based on the given address and length.\n
 * Most of CACHE APIs will effect I-CACHE and D-CACHE at the same time.\n
 * When a memory is set as cacheable, the CPU exchanges data with the CACHE memory during the CACHE hit instead of an external memory, so it can accelerate the access performance. But a data coherency
 * issue is possible when the memory is shared between the Central Processing Unit (CPU) and hardware co-processors, such as Direct Memory Access (DMA). Peripherals (Inter-Integrated Circuit (I2C), Universal Asynchronous Receiver/Transmitter (UART), Serial Peripheral Interface (SPI)) or hardware modules (Graphics 2D (G2D), CAMERA) that operate
 * in DMA mode also face the same issue.\n
 * These peripherals or hardware modules must work with non-cacheable memory.
 * @section HAL_CACHE_Terms_Chapter Terms and acronyms
 *
 * The following provides descriptions to the terms commonly used in the CACHE controller and how to use its various functions.
 *
 * |Terms                   |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b AHB                        | Advanced High-performance Bus (AHB) is a bus protocol introduced in the <a href="http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ihi0011a/index.html">Advanced Microcontroller Bus Architecture version 2</a> published by ARM Ltd.|
 * |\b CACHE                      | A collection of data duplicating original values stored in external memory, to accelerate the data access and enhance the MCU performance.|
 * |\b CACHE line                 | The smallest unit of memory that can be transferred between the main memory and the CACHE. Unless otherwise indicated, the CACHE line size is 8 words (64 bytes).|
 * |\b IRAM                       | Local Instruction Memory (TCM), memory which resides directly in the processor.|
 * |\b DRAM                       | Local Data Memory (TCM), memory which resides directly in the processor.|
 * |\b flush                      | Data in invalid CACHE line with dirty bit set is evicted to write-back buffer, then dirty bits are cleared. If the entry is invalid or the dirty flag is not set, leave as it is. The action does not clear the valid bit(s).|
 * |\b invalidate                 | Unconditionally clear valid and dirty bits of the corresponding CACHE line(s).|
 *
 * @section HAL_CACHE_Features_Chapter Supported features
 * Features supported by this module are listed below:
 *
 * - \b Configure \b the \b overall \b CACHE \b size.\n
 *   To reach the optimum system performance, the CACHE size can be set with flexibility. If the CACHE size is not set to the maximum (32kB), the memory of the unused CACHE is reused by the TCM.
 *   Users can call the function #hal_cache_set_size(), to set the CACHE size. The input parameter of this function can be any value of type #hal_cache_size_t.
 *   Apply different parameters for the function #hal_cache_set_size() to set the cache size to one of the following four configurations:
 *      |Parameter             | CACHE size     |
 *      |----------------------|----------------|
 *      | #HAL_CACHE_SIZE_16KB | 16kB           |
 *
 * - \b Using \b CACHE \b region \b to \b set \b a \b cacheable \b attribute \b for \b a \b memory \b block.\n
 *   The CACHE controller has 8 regions for cacheable attribute settings. Note that each region can be continuous or non-continuous to each other. For those address ranges not covered by any region in the CACHE, the cacheable settings are automatically set as uncacheable. There is one restriction: different regions must not overlap.
 *   Call the function #hal_cache_region_config(), to set the cacheable memory blocks for each region. The parameter should be of type #hal_cache_region_config_t and the field in the #hal_cache_region_config_t includes the cache_region_address (must be 512MB aligned) and the cache_region_size (must be 512MB aligned).\n
 *
 * - \b CACHE \b flush \b or \b invalidate \b operation. \n
 *   The CACHE maintenance operation includes flush and invalidate. The two operations may be performed on the entire CACHE or on a given address-length. The CACHE driver API names distinguish one from the other by the word "all". For example, to invalidate all CACHE lines, call the API #hal_cache_invalidate_all_cache_lines(). To invalidate the CACHE lines for a particular memory block, simply call #hal_cache_invalidate_multiple_cache_lines(). Note that the parameters "address" and "length" must be supplied to specify the physical address range to perform the CACHE maintenance operation on. The usage of flush operation is the same as the invalidate operation.
 *
 *
 * @section HAL_CACHE_Driver_Usage_Chapter How to use this driver
 *
 * - \b Use \b CACHE \b driver \b to \b enable \b or \b disable \b CACHE.
 *  - #hal_cache_region_enable() or #hal_cache_region_disable() is used to enable or disable cacheable attribute for a specified region.
 *  - #hal_cache_enable() or #hal_cache_disable() is used to enable or disable the CACHE controller. If disabled, the MCU memory accesses are all non-cacheable. If enabled, the settings in CACHE controller will take effect.
 *
 * - \b Use \b CACHE \b driver \b to \b set \b a \b cacheable \b region.
 *  - Step 1. Call #hal_cache_set_size() to set the total CACHE size.
 *  - Step 2. Call #hal_cache_region_config() to set a CACHE region with specified address and length (should call more than once if there are multiple regions to be configured as cacheable).
 *  - Step 3. Call #hal_cache_region_enable() to enable the CACHE function of the corresponding region configured in step 2.
 *  - Step 4. Call #hal_cache_enable() to enable the CACHE controller.
 *  - Sample code:
 *  @code
 *
 *  hal_cache_region_t region = HAL_CACHE_REGION_0;
 *  hal_cache_region_config_t region_config;
 *
 *  hal_cache_set_size(HAL_CACHE_SIZE_16K); // Set the total CACHE size.Instruction cache size and Data cache size in dsp are fixed during hardware design.
 *
 *  region_config.cache_region_address = 0x00000000; // The start address of the region that is cacheable, which must be 512MB aligned.
 *  region_config.cache_region_size = 0x20000000; // The size of the cacheable region, which must be 512MB.
 *
 *  hal_cache_region_config(region, &region_config);
 *  hal_cache_region_enable(region);
 *
 *  hal_cache_enable();
 *  @endcode
 *
 * - \b Use \b CACHE \b driver \b to \b flush \b or \b invalidate \b a \b cacheable \b region.\n
 *   - Step 1.\n
 *        Call #hal_cache_flush_multiple_cache_lines() to flush a cacheable memory specified by address and length.
 *  \n    Call #hal_cache_flush_one_cache_line() instead, if just one line needs to be flushed.
 *   - Step 2.\n
 *        Call #hal_cache_invalidate_multiple_cache_lines() to invalidate a cacheable memory specified by address and length.
 *  \n    Call #hal_cache_invalidate_one_cache_line() instead, if just one line needs to be invalidated.
 *   - Sample code:
 *   @code
 *
 *  hal_cache_region_t region = HAL_CACHE_REGION_0;
 *  hal_cache_region_config_t region_config;
 *
 *  hal_cache_set_size(HAL_CACHE_SIZE_16K); // Set the total CACHE size.Instruction cache size and Data cache size in dsp are fixed during hardware design.
 *
 *  region_config.cache_region_address = 0x00000000; // The start address of the region that is cacheable, that must be 512MB aligned.
 *  region_config.cache_region_size = 0x20000000; // The size of the cacheable region, which must be 512MB.
 *
 *  hal_cache_region_config(region, &region_config);
 *  hal_cache_region_enable(region);
 *
 *  hal_cache_enable();
 *
 *  hal_cache_flush_multiple_cache_lines(region_config.cache_region_address, region_config.cache_size);
 *  hal_cache_invalidate_multiple_cache_lines(region_config.cache_region_address, region_config.cache_size);
 *  @endcode
 *
 * - \b Use \b CACHE \b driver \b to \b flush \b or \b invalidate \b all \b cacheable \b regions. \n
 *   - Step 1. Call #hal_cache_flush_all_cache_lines().
 *   - Step 2. Call #hal_cache_invalidate_all_cache_lines().
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * Enums
 *****************************************************************************/

/** @defgroup hal_cache_enum Enum
 *  @{
 */

/** @brief This enum defines the CACHE API return status. */
typedef enum {
    HAL_CACHE_STATUS_INVALID_PARAMETER = -7,    /**< Invalid parameter. */
    HAL_CACHE_STATUS_ERROR_BUSY = -6,           /**< CACHE is busy. */
    HAL_CACHE_STATUS_ERROR_CACHE_SIZE = -5,     /**< CACHE size is invalid, total CACHE size is not a value of type #hal_cache_size_t. */
    HAL_CACHE_STATUS_ERROR_REGION = -4,         /**< CACHE region error, CACHE region is not a value of type #hal_cache_region_t. */
    HAL_CACHE_STATUS_ERROR_REGION_ADDRESS = -3, /**< CACHE region address error, CACHE region address is not 512MB aligned. */
    HAL_CACHE_STATUS_ERROR_REGION_SIZE = -2,    /**< CACHE region size error, CACHE region size is not a multiple of 512MB. */
    HAL_CACHE_STATUS_ERROR = -1,                /**< CACHE region enable error, CACHE region is not set at the time the region is being enabled. */
    HAL_CACHE_STATUS_OK = 0                     /**< CACHE API call returns successfully. */
} hal_cache_status_t;


/** @brief This enum defines the CACHE size. */
typedef enum {
    HAL_CACHE_SIZE_16KB = 2,                    /**< CACHE size is 16kB. */
    HAL_CACHE_SIZE_32KB = 3,                    /**< CACHE size is 32kB. */
    HAL_CACHE_SIZE_MAX                          /**< The total number of CACHE sizes (invalid size). */
} hal_cache_size_t;


/** @brief This enum defines the CACHE region number. */
typedef enum {
    HAL_CACHE_REGION_0 = 0,                     /**< CACHE region 0. */
    HAL_CACHE_REGION_1 = 1,                     /**< CACHE region 1. */
    HAL_CACHE_REGION_2 = 2,                     /**< CACHE region 2. */
    HAL_CACHE_REGION_3 = 3,                     /**< CACHE region 3. */
    HAL_CACHE_REGION_4 = 4,                     /**< CACHE region 4. */
    HAL_CACHE_REGION_5 = 5,                     /**< CACHE region 5. */
    HAL_CACHE_REGION_6 = 6,                     /**< CACHE region 6. */
    HAL_CACHE_REGION_7 = 7,                     /**< CACHE region 7. */
    HAL_CACHE_REGION_MAX                        /**< The total number of CACHE regions (invalid region). */
} hal_cache_region_t;

/**
 * @}
 */

/*****************************************************************************
 * Structures
 *****************************************************************************/

/** @defgroup hal_cache_struct Struct
 *  @{
 */

/** @brief The CACHE region configuration structure that contains the start address and the size of the region, both must be 512MB aligned. */
typedef struct {
    uint32_t cache_region_address;              /**< Starting address of the CACHE region. */
    uint32_t cache_region_size;                 /**< The CACHE region size. */
} hal_cache_region_config_t;

/**
 * @}
 */

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * @brief  This function initializes the I-CACHE and the D-CACHE, sets the default cacheable attribute for the project memory layout.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully initialized. \n
 * #HAL_CACHE_STATUS_ERROR_BUSY, the CACHE is busy.
 */
hal_cache_status_t hal_cache_init(void);


/**
 * @brief  This function deinitializes the I-CACHE and the D-CACHE, returning the CACHE module to its default state.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully deinitialized.
 */
hal_cache_status_t hal_cache_deinit(void);


/**
 * @brief  This function enables the I-CACHE and the D-CACHE controllers. @sa hal_cache_region_enable().
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully enabled. \n
 * #HAL_CACHE_STATUS_ERROR_CACHE_SIZE, the CACHE cannot be enabled when the CACHE size is 0kB.
 */
hal_cache_status_t hal_cache_enable(void);


/**
 * @brief  This function disables the I-CACHE and the D-CACHE controllers. @sa hal_cache_region_disable().
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully disabled.
 */
hal_cache_status_t hal_cache_disable(void);


/**
 * @brief  This function enables a single I-CACHE and D-CACHE region. Once hal_cache_enable() is called, the settings of the specified region take effect. @sa hal_cache_enable().
 * @param[in] region is the enabled region, this parameter can only be a value of type #hal_cache_region_t.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE region is successfully enabled. \n
 * #HAL_CACHE_STATUS_ERROR_REGION, the region is invalid. \n
 * #HAL_CACHE_STATUS_ERROR, the region is not configured before enable.
 */
hal_cache_status_t hal_cache_region_enable(hal_cache_region_t region);


/**
 * @brief  This function disables a single I-CACHE and D-CACHE region. When this function is called, the CACHE settings of corresponding region are disabled, even if the hal_cache_enable() function is called. @sa hal_cache_disable().
 * @param[in] region is the disabled region, this parameter is any value of type #hal_cache_region_t.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE region is successfully disabled. \n
 * #HAL_CACHE_STATUS_ERROR_REGION, the region is invalid.
 */
hal_cache_status_t hal_cache_region_disable(hal_cache_region_t region);


/**
 * @brief  This function configures the I-CACHE and the D-CACHE.
 * @param[in] region is the region to configure.
 * @param[in] region_config is the configuration information of the region.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE region is successfully configured. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, region_config is NULL. \n
 * #HAL_CACHE_STATUS_ERROR_REGION, the region is invalid. \n
 * #HAL_CACHE_STATUS_ERROR_REGION_SIZE, the region size is invalid. \n
 * #HAL_CACHE_STATUS_ERROR_REGION_ADDRESS, the region address is invalid.
 */
hal_cache_status_t hal_cache_region_config(hal_cache_region_t region, const hal_cache_region_config_t *region_config);

/**
 * @brief  This function sets the total size of the I-CACHE and the total size of the D-CACHE.
 * @param[in] cache_size should only be any value of type #hal_cache_size_t.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE size setting is successful. \n
 * #HAL_CACHE_STATUS_ERROR_CACHE_SIZE, the CACHE size is invalid.
 */
hal_cache_status_t hal_cache_set_size(hal_cache_size_t cache_size);

/**
 * @brief  This function invalidates a I-CACHE line and a D-CACHE line at a given address.
 *         We do not recommend using this API due to the different I_CACHE and D_CACHE line size may cause the alignment issues of address or length.
 *         This API might be phased out in the future.
 * @param[in] address is the start address of the memory that is invalidated.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully invalidated. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, address is not CACHE line size aligned.
 *      For example: If I_CACHE line size is equal to 64, D_CACHE line size is equal to 32.
 *                   When HAL_CACHE_LINE_SIZE is defined as 32, this function will only operate on D_CACHE and ignore I_CACHE.
 *                   The alignment requires that the address of a CACHE line must be a multiple of #HAL_CACHE_LINE_SIZE.
 */
hal_cache_status_t hal_cache_invalidate_one_cache_line(uint32_t address);

/**
 * @brief  This function invalidates a I-CACHE line at a given address.
 * @param[in] address is the start address of the memory that is invalidated.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully invalidated. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, address is not CACHE line size aligned. The alignment requires that the address of a I_CACHE line must be a multiple of #HAL_I_CACHE_LINE_SIZE.
 */
hal_cache_status_t hal_cache_invalidate_one_i_cache_line(uint32_t address);

/**
 * @brief  This function invalidates a D-CACHE line at a given address.
 * @param[in] address is the start address of the memory that is invalidated.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully invalidated. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, address is not CACHE line size aligned. The alignment requires that the address of a D_CACHE line must be a multiple of #HAL_D_CACHE_LINE_SIZE.
 */
hal_cache_status_t hal_cache_invalidate_one_d_cache_line(uint32_t address);

/**
 * @brief  This function invalidates the I-CACHE lines and D-CACHE lines by address and length.
 *         We do not recommend using this API due to the different I_CACHE and D_CACHE line size may cause the alignment issues of address or length.
 *         This API might be phased out in the future.
 * @param[in] address is the start address of the memory that is invalidated.
 * @param[in] length is the length of memory that to be invalidated.The unit of the memory length is in bytes and both address and length must be CACHE line size aligned when the API is called.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully invalidated. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, either address or length is not CACHE line size aligned.
 *      For example: If I_CACHE line size is equal to 64, D_CACHE line size is equal to 32.
 *                   When HAL_CACHE_LINE_SIZE is defined as 32, this function will only operate on D_CACHE and ignore I_CACHE.
 *                   The alignment requires that the address of a CACHE line must be a multiple of #HAL_CACHE_LINE_SIZE.
 */
hal_cache_status_t hal_cache_invalidate_multiple_cache_lines(uint32_t address, uint32_t length);

/**
 * @brief  This function invalidates the I-CACHE lines by address and length.
 * @param[in] address is the start address of the memory that is invalidated.
 * @param[in] length is the length of memory that to be invalidated.The unit of the memory length is in bytes and both address and length must be CACHE line size aligned when the API is called.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully invalidated. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, either address or length is not CACHE line size aligned. The alignment requires that the address and the length of a I_CACHE line must be a multiple of #HAL_I_CACHE_LINE_SIZE.
 */
hal_cache_status_t hal_cache_invalidate_multiple_i_cache_lines(uint32_t address, uint32_t length);

/**
 * @brief  This function invalidates the D-CACHE lines by address and length.
 * @param[in] address is the start address of the memory that is invalidated.
 * @param[in] length is the length of memory that to be invalidated.The unit of the memory length is in bytes and both address and length must be CACHE line size aligned when the API is called.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully invalidated. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, either address or length is not CACHE line size aligned. The alignment requires that the address and the length of a D_CACHE line must be a multiple of #HAL_D_CACHE_LINE_SIZE.
 */
hal_cache_status_t hal_cache_invalidate_multiple_d_cache_lines(uint32_t address, uint32_t length);

/**
 * @brief  This function invalidates the whole I-CACHE and D-CACHE.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully invalidated.
 */
hal_cache_status_t hal_cache_invalidate_all_cache_lines(void);

/**
 * @brief  This function flushes one D-CACHE line by address.
 * @param[in] address is the start address of the memory that is flushed.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully flushed. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, address is not CACHE line size aligned. The alignment requires that the address of a CACHE line must be a multiple of #HAL_CACHE_LINE_SIZE.
 */
hal_cache_status_t hal_cache_flush_one_cache_line(uint32_t address);

/**
 * @brief  This function flushes the D-CACHE lines by address and length.
 * @param[in] address is the start address of the memory to flush.
 * @param[in] length is the length of the memory to be flushed. The unit of the memory length is in bytes and both address and length must be CACHE line size aligned when the API is called.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully flushed. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, either address or length is not CACHE line size aligned. The alignment requires that the address and the length of a CACHE line must be a multiple of #HAL_CACHE_LINE_SIZE.
 */
hal_cache_status_t hal_cache_flush_multiple_cache_lines(uint32_t address, uint32_t length);


/**
 * @brief  This function flushes the whole D-CACHE.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE is successfully flushed.
 * @note Flush all CACHE lines.
 */
hal_cache_status_t hal_cache_flush_all_cache_lines(void);


/**
 * @brief  This function checks whether the memory is cacheable.
 * @param[in] address is the memory address to check.
 * @return
 * return true, the memory is cacheable. \n
 * return false, the memory is non-cacheable.
 */
bool hal_cache_is_cacheable(uint32_t address);

/*convert cacheable region address to non-cacheable region address*/
#ifdef HAL_CACHE_REGION_CONVERT
/**
 * @brief  This function convert a cacheable address to non-cacheable address.
 * @param[in] address is cacheable.
 * @return
 * return non-cacheable address.
 */
uint32_t hal_cache_cacheable_to_noncacheable(uint32_t address);

/**
 * @brief  This function convert a non-cacheable address to cacheable address.
 * @param[in] address is non-cacheable.
 * @return
 * return cacheable address.
 */
uint32_t hal_cache_noncacheable_to_cacheable(uint32_t address);

#endif /* HAL_CACHE_REGION_CONVERT */

/** @defgroup hal_cache_define Define
  * @{
  */

/*convert cacheable region address to non-cacheable region address*/
#ifdef HAL_CACHE_REGION_CONVERT
/**
 * @brief  This function convert a cacheable address to non-cacheable address.
 * @param[in] address is cacheable.
 * @return
 * return non-cacheable address.
 */
uint32_t hal_cache_cacheable_to_noncacheable(uint32_t cacheable_address);

/**
 * @brief  This function convert a non-cacheable address to cacheable address.
 * @param[in] address is non-cacheable.
 * @return
 * return cacheable address.
 */
uint32_t hal_cache_noncacheable_to_cacheable(uint32_t noncacheable_address);

#endif /* HAL_CACHE_REGION_CONVERT */

/**
  * @}
  */


#ifdef __cplusplus
}
#endif

/**
 * @}
 * @}
*/
#endif /*HAL_CACHE_MODULE_ENABLED*/
#endif /* __HAL_CACHE_H__ */


