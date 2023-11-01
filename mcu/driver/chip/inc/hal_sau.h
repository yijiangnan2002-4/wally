/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#ifndef __HAL_SAU_H__
#define __HAL_SAU_H__
#include "hal_platform.h"

#ifdef HAL_SAU_MODULE_ENABLED

/**
 * @addtogroup HAL
 * @{
 * @addtogroup SAU
 * @{
 * The SAU determines the security of an address.
 *
 * For instructions, the SAU returns the security attribute (Secure or Non-secure) and identifies whether the
 * instruction address is in a Non-secure callable region.
 *
 * For data, the SAU returns the security attribute (Secure or Non-secure).
 *
 * When a memory access is performed, the security of the address is verified by the SAU. Any address that
 * matches multiple SAU regions will be marked with the most secure attribute of the matching regions.
 * \n
 * .
 * SAU mismatches and permission violations invoke the programmable-priority Secure Fault handler.
 *
 * @section HAL_SAU_Terms_Chapter Terms and acronyms
 *
 * The following provides descriptions to the terms commonly used in the SAU driver and how to use its various functions.
 *
 * |Terms                   |Details                  |
 * |------------------------|-------------------------|
 * |\b SAU                  | Security Attribute Unit.|
 *
 *
 * @section HAL_SAU_Features_Chapter Supported features
 * The SAU HAL driver enables configuration of location, size and access permissions for each region. Call the SAU APIs to configure the SAU, as shown below:
 * - \b Configure \b each \b region \b for \b different \b access \b permissions. \n
 *   Call hal_sau_region_configure(), to configure the specified region. The first parameter is of #hal_sau_region_t type, to indicate which region is to be configured. 
 *   The second parameter is of #hal_sau_region_config_t type, and defines the start address, end address and non-secure callable.
 * - \b Enable \b the \b SAU \b feature \b inside \b the \b MCU. \n
 *   To enable the SAU settings for the configured region(s), user has to follow these two steps:
 *   - Step1: Call hal_sau_region_enable() to enable the SAU region(s).
 *   - Step2: Call hal_sau_enable() to enable the SAU function. \n
 *   .
 *   ----NOTE---
 *   -Only Privileged accesses to the SAU registers are permitted. Unprivileged accesses generate a fault.
 *   -The SAU registers are word accessible only. Halfword and byte accesses are UNPREDICTABLE.
 *   -The SAU registers are RAZ/WI when accessed from Non-secure state.
 *   -The SAU registers are not banked between Security states.
 *
 * @section HAL_SAU_Driver_Usage_Chapter How to use this driver
 *  The SAU HAL driver APIs enable the user to perform the following.
 * - \b Set \b location, \b size \b and \b access \b permissions \b for \b each \b region.
 *   - As described above in the section @ref HAL_SAU_Features_Chapter, user can accomplish this by calling hal_sau_region_configure(). The sample code is shown below:
 *   - Sample code:
 *   @code
 *   hal_sau_init();
 * 	 hal_sau_region_t region = HAL_SAU_REGION_1;
 * 	 hal_sau_region_config_t region_config;
 *
 * 	 memset(region_config, 0, sizeof(hal_sau_region_config_t));   //Note, call memset() to make sure all memory regions are set to 0.
 * 	 region_config.sau_region_start_address = 0x00000000;         //The start address of the region that is configured.
 * 	 region_config.sau_region_end_address   = 0x00001000;         //The end address of the region that is configured.
 * 	 region_config.sau_non_secure_callable  = true;               //Non_secure_callable.
 *
 * 	 hal_sau_region_configure(region, &region_config);
 *
 *   @endcode
 * - \b Enable \b SAU \b function \b inside \b the \b MCU. \n
*    First call hal_sau_region_enable() to enable the region(s), then hal_sau_enable(), to enable the SAU. The sample code is shown below:
*    - Sample code:
*    @code
*
* 	 hal_sau_region_t region = HAL_SAU_REGION_1;
*
* 	 hal_sau_region_enable(region);//Enables the region.
* 	 hal_sau_enable();//Enables the SAU.
*
*    @endcode
 */


#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * Enums
 *****************************************************************************/

/** @defgroup hal_sau_enum Enum
 *  @{
 */

/** @brief This enum defines the SAU API return status. */
typedef enum {
    HAL_SAU_STATUS_INVALID_PARAMETER = -6,	    /**< Invalid parameter */
    HAL_SAU_STATUS_ERROR_BUSY = -5,		        /**< SAU is busy */
    HAL_SAU_STATUS_ERROR_REGION = -4,           /**< SAU region number error, SAU region is not a value of type #hal_SAU_region_t */
    HAL_SAU_STATUS_ERROR_REGION_ADDRESS = -3,   /**< SAU region address error, SAU region address is not valid */
    HAL_SAU_STATUS_ERROR_REGION_SIZE = -2,      /**< SAU region size error, SAU region size is not a value of type #hal_sau_region_size_t */
    HAL_SAU_STATUS_ERROR = -1,                  /**< SAU error, errors other than reasons described above */
    HAL_SAU_STATUS_OK = 0                       /**< SAU ok */
} hal_sau_status_t;

/** @brief SAU region number. */
typedef enum {
    HAL_SAU_REGION_0 = 0,                       /**< SAU region 0 */
    HAL_SAU_REGION_1 = 1,                       /**< SAU region 1 */
    HAL_SAU_REGION_2 = 2,                       /**< SAU region 2 */
    HAL_SAU_REGION_3 = 3,                       /**< SAU region 3 */
    HAL_SAU_REGION_4 = 4,                       /**< SAU region 4 */
    HAL_SAU_REGION_5 = 5,                       /**< SAU region 5 */
    HAL_SAU_REGION_6 = 6,                       /**< SAU region 6 */
    HAL_SAU_REGION_7 = 7,                       /**< SAU region 7 */
    HAL_SAU_REGION_MAX                          /**< Max SAU region number (invalid) */
} hal_sau_region_t;

/**
 * @}
 */

/*****************************************************************************
 * Structures
 *****************************************************************************/

/** @defgroup hal_sau_struct Struct
 *  @{
 */

/** @brief SAU region config structure contains the start address, the size and access rights of the region. */
typedef struct {
    uint32_t sau_region_start_address;          /**< SAU region start address */
    uint32_t sau_region_end_address;            /**< SAU region end address */
    bool     sau_non_secure_callable;           /**< SAU region wheather Non-secure callable. Controls whether Non-secure state is permitted to execute an SG instruction from this region. */
} hal_sau_region_config_t;

/**
 * @}
 */

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * @brief 	SAU initialization function.
 * @param[in] sau_config is the configuration information for SAU,which indicates the number of regions implemented by the Security Attribution Unit.
 * @return
 * #HAL_SAU_STATUS_OK, SAU is successfully initialized. \n
 * #HAL_SAU_STATUS_INVALID_PARAMETER, sau_config is NULL. \n
 * #HAL_SAU_STATUS_ERROR_BUSY, SAU is busy.
 */
hal_sau_status_t hal_sau_init(void);

/**
 * @brief 	SAU deinitialization function. This function resets the SAU registers to their default values.
 * @return
 * #HAL_SAU_STATUS_OK, SAU is successfully de-initialized.
 */
hal_sau_status_t hal_sau_deinit(void);

/**
 * @brief 	SAU enable function. Enables the SAU settings during a memory access. @sa hal_sau_disable().
 * @return
 * #HAL_SAU_STATUS_OK, SAU is successfully enabled .
 */
hal_sau_status_t hal_sau_enable(void);

/**
 * @brief SAU disable function. Disables the SAU settings. @sa hal_sau_enable().
 * @return
 * #HAL_SAU_STATUS_OK, SAU is successfully disabled.
 */
hal_sau_status_t hal_sau_disable(void);

/**
 * @brief 	SAU region enable function. Enables the specified region, when the hal_sau_enable() is called, the settings of the corresponding region take effect. @sa hal_sau_enable().
 * @param[in] region is the region that is enabled, this parameter can only be a value of type #hal_sau_region_t.
 * @return
 * #HAL_SAU_STATUS_OK, SAU region is successfully enabled. \n
 * #HAL_SAU_STATUS_ERROR_REGION, the region is invalid.
 */
hal_sau_status_t hal_sau_region_enable(hal_sau_region_t region);

/**
 * @brief 	SAU region disable function. Disables the specified region, when this function is called, the settings of corresponding region are disabled even if the hal_sau_enable() function is called. @sa hal_sau_disable().
 * @param[in] region is the region that is disabled, this parameter can only be a value of type #hal_sau_region_t.
 * @return
 * #HAL_SAU_STATUS_OK, SAU region is successfully disabled. \n
 * #HAL_SAU_STATUS_ERROR_REGION, the region is invalid.
 */
hal_sau_status_t hal_sau_region_disable(hal_sau_region_t region);

/**
 * @brief 	SAU region configuration function.
 * @param[in] region is the region that is configured.
 * @param[in] region_config is the configuration information of the region.
 * @return
 * #HAL_SAU_STATUS_OK, SAU region is successfully configured. \n
 * #HAL_SAU_STATUS_INVALID_PARAMETER, region_config is NULL. \n
 * #HAL_SAU_STATUS_ERROR_REGION, the region is invalid. \n
 * #HAL_SAU_STATUS_ERROR_REGION_SIZE, the region size is invalid. \n
 * #HAL_SAU_STATUS_ERROR_REGION_ADDRESS, the region address is invalid.
 */
hal_sau_status_t hal_sau_region_configure(hal_sau_region_t region, const hal_sau_region_config_t *region_config);

#ifdef __cplusplus
}
#endif

/**
 * @}
 * @}
*/


#endif /* HAL_SAU_MODULE_ENABLED */
#endif /* __HAL_SAU_H__ */


