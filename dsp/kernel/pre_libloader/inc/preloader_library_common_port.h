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

#ifndef __PRELOADER_LIBRARY_COMMON_PORT__
#define __PRELOADER_LIBRARY_COMMON_PORT__
#include <stdlib.h>
#include <stdio.h>
#include "semphr.h"
#include "preloader_pisplit.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup pic_library_load_information Struct
 *  @{
 */
/** @brief This struct defines preloader user customization configure structure */
typedef struct {
    void* code_address;    /**< Defines the code address of load code region*/
    void* data_address;    /**< Defines the data address of load code region */
    preloader_pisplit_customer_callback_t library_callback;  /**< Defines the callback after the library load done */
    xtlib_packaged_library *library;  /**< Defines the library address */
    uint32_t *dram_size;  /**< Defines the library data size*/
}pic_library_load_info_t;
/**
 * @}
 */

/**
 * @}
 */


/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * @brief     This function to load pisplit library internal protable layer.
 * @param[in] *load_lib_xSemaphore  specifies user pisplit library use semaphore global variable.
 * @param[in] *handle  specifies user pisplit library handle.
 * @param[in] *load_info  specifies library information of loading.
 * @return
 *                #PRELOADER_PISPLIT_XTLIB_NO_ERR if OK. \n
 *                other return value please reference #preloader_pisplit_error_handling_t. \n
 *
 * @note
 *            User must the pisplit library handle before calling this function.
 */
void preloader_pisplit_library_load_internal_port(SemaphoreHandle_t *load_lib_xSemaphore,preloader_pisplit_handle_t *handle, pic_library_load_info_t *load_info);

/**
 * @brief     This function to unload pisplit library internal protable layer.
 * @param[in] handle   specifies user pisplit library handle.
 * @param[in] **export_parameters_table  specifies the table address that docks the library function address.
 * @param[in] export_function_number  specifies numbers of export function in library.
 * @param[in] *default_function_address specifies default function address.
 * @return
 *                #PRELOADER_PISPLIT_XTLIB_NO_ERR if OK. \n
 *                other return value please reference #preloader_pisplit_error_handling_t. \n
 *
 * @note
 *            User must the pisplit library handle before calling this function.
 */
void preloader_pisplit_library_unload_internal_port(preloader_pisplit_handle_t handle, void** export_parameters_table, uint32_t export_function_number,void* default_function_address);


#ifdef __cplusplus
}
#endif

#endif /* __PRELOADER_PISPLIT_INTERNAL__ */
