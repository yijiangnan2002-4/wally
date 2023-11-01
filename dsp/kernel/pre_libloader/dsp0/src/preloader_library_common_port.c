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

#include <string.h>
//#include <xtensa/tie/xt_hifi2.h>
#include <xtensa/hal.h>
#include "assert.h"


#include "xt_library_loader.h"
#include "preloader_pisplit.h"
#include "preloader_pisplit_internal.h"
#include "preloader_pisplit_log.h"
#include "preloader_library_common_port.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "syslog.h"



void preloader_pisplit_library_load_internal_port(SemaphoreHandle_t *load_lib_xSemaphore,preloader_pisplit_handle_t *handle, pic_library_load_info_t *load_info)
{
    
    pic_library_load_info_t *ptemp = load_info;

    if (*load_lib_xSemaphore == NULL) {
        *load_lib_xSemaphore = xSemaphoreCreateBinary();
        //DSP_MW_LOG_E("pic_library_load_internal_port load_lib_xSemaphore :%x !!!!", 2,load_lib_xSemaphore,*load_lib_xSemaphore);
        if ( *load_lib_xSemaphore == NULL) {
            assert(0);
        }
    }

    if (PRELOADER_PISPLIT_XTLIB_NO_ERR != preloader_pisplit_get_handle(handle, ptemp->library, ptemp->library_callback)) {
        PRELOADER_LOG_W(preloader,"library[0x%x] preloader_pisplit_get_handle() error!!!!", 1,ptemp->library);
        assert(0);
    }
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    if(PRELOADER_PISPLIT_XTLIB_NO_ERR != preloader_pisplit_load_v1(*handle, ptemp->code_address, ptemp->data_address))
#else
    if(PRELOADER_PISPLIT_XTLIB_NO_ERR != preloader_pisplit_load(*handle,PRELOADER_I_HIGH_PERFORMANCE,PRELOADER_D_HIGH_PERFORMANCE) )
#endif
    {
        PRELOADER_LOG_W(preloader,"library[0x%x] preloader_pisplit_load() error!!!!", 1,ptemp->library);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_I_HIGH_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_I_LOW_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_D_HIGH_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_D_LOW_PERFORMANCE);
        assert(0);
    }

    if (xSemaphoreTake(*load_lib_xSemaphore, portMAX_DELAY) == pdTRUE) {
    } else {
        assert(0);
    }

    vSemaphoreDelete(*load_lib_xSemaphore);
    *load_lib_xSemaphore = NULL;

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    preloader_pisplit_get_library_code_and_data_size(*handle, NULL, ptemp->dram_size);
#endif

}

void preloader_pisplit_library_unload_internal_port(preloader_pisplit_handle_t handle, void** export_parameters_table, uint32_t export_function_number,void* default_function_address)
{
    pisplit_library_info_t *p_handle = (pisplit_library_info_t *)handle;

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    if(PRELOADER_PISPLIT_XTLIB_NO_ERR != preloader_pisplit_unload_v1(handle))
#else
    if(PRELOADER_PISPLIT_XTLIB_NO_ERR != preloader_pisplit_unload(handle))
#endif
    {
        PRELOADER_LOG_W(preloader,"library[0x%x] preloader_pisplit_unload() error!!!!", 1,p_handle->p_pi_library);
        assert(0);
    }
    if (PRELOADER_PISPLIT_XTLIB_NO_ERR != preloader_pisplit_release_handle(handle)) {
        PRELOADER_LOG_W(preloader,"library[0x%x] preloader_pisplit_release_handle() error!!!!", 1,p_handle->p_pi_library);
        assert(0);
    }

    for (uint32_t i = 0; i < export_function_number; i++) {
        export_parameters_table[i] = default_function_address;
    }

}
