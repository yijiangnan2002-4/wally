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
#include <xtensa/tie/xt_hifi2.h>
#include <xtensa/hal.h>
#include "assert.h"
#include "xt_library_loader.h"
#include "preloader_pisplit.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "dsp_feature_interface.h"
#include "wind_portable.h"

#if defined(AIR_ANC_WIND_DETECTION_ENABLE) && defined(AIR_WIND_DETECTION_USE_PIC)

/***********************************************************************/
static SemaphoreHandle_t wind_load_lib_xSemaphore = NULL;
#define WIND_EXPORT_PARA_NUM 3

/***********************************************************************/

extern void wind_default_function_parameter();


void *wind_export_parameters[WIND_EXPORT_PARA_NUM] = {wind_default_function_parameter,
                                                      wind_default_function_parameter,
                                                      wind_default_function_parameter
                                                      };
#if PIC_LOGPRINT
void *wind_import_parameters[1] = {printf};//
#else
void *wind_import_parameters[1] = {fake_printf};//
#endif


void wind_default_function_parameter()
{
    DSP_MW_LOG_I("function point is NULL!!! wind detect library not load or had been unload!!!", 0);
    AUDIO_ASSERT(0);
}


static preloader_pisplit_handle_t p_wind_pisplit_library_handle = NULL;
static uint32_t wind_lib_loaded_counter = 0;

void wind_load_library_callback(preloader_pisplit_handle_t handle)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;
    p_export_parameters = preloader_pisplit_get_export_parameter(handle, wind_import_parameters);

    if (p_export_parameters == NULL) {
       DSP_MW_LOG_I("p_export_parameters is NULL, please check!!!", 0);
    } else {
        wind_export_parameters[0] = (void *)p_export_parameters[0];
        wind_export_parameters[1] = (void *)p_export_parameters[1];
		wind_export_parameters[2] = (void *)p_export_parameters[2];
        xSemaphoreGiveFromISR(wind_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

uint32_t wind_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    pic_library_load_info_t temp_value;
    temp_value.code_address = code_address;
    temp_value.data_address = data_address;
    temp_value.library = &pisplit_wind_det;
    temp_value.library_callback = wind_load_library_callback;
    temp_value.dram_size = dram_pic_usage;

    if (wind_lib_loaded_counter == 0) {
        preloader_pisplit_library_load_internal_port(&wind_load_lib_xSemaphore, &p_wind_pisplit_library_handle, &temp_value);
        wind_lib_loaded_counter = 1;
    } else {
        wind_lib_loaded_counter++;
        DSP_MW_LOG_I("wind_library_load preloader_pisplit_load() had been loaded!!! wind_lib_loaded_counter: %d \r\n", 1, wind_lib_loaded_counter);
    }
    return wind_lib_loaded_counter;
}

uint32_t wind_library_unload()
{

    if (wind_lib_loaded_counter == 1) {
        preloader_pisplit_library_unload_internal_port(p_wind_pisplit_library_handle, wind_export_parameters, WIND_EXPORT_PARA_NUM, wind_default_function_parameter);

        //p_wind_pisplit_library_handle = NULL;
        wind_lib_loaded_counter = 0;
    } else {
        wind_lib_loaded_counter--;
    }
    return wind_lib_loaded_counter;
}

#endif

