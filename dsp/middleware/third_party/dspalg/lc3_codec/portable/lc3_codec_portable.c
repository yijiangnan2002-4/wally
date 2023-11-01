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
#include <stdbool.h>
#include <xtensa/tie/xt_hifi2.h>
#include <xtensa/hal.h>
#include "assert.h"
#include "xt_library_loader.h"
#include "preloader_pisplit.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "dsp_feature_interface.h"

#ifdef MTK_BT_A2DP_LC3_USE_PIC

#include "lc3_codec_portable.h"
#include "lc3_codec_api.h"

static void lc3_codec_default_function_parameter(void);




void *g_lc3_codec_export_parameters[] = {
    lc3_codec_default_function_parameter, /* For LC3_Enc_Get_MemSize() */
    lc3_codec_default_function_parameter, /* For LC3_Enc_Init() */
    lc3_codec_default_function_parameter, /* For LC3_Enc_Prcs() */
    lc3_codec_default_function_parameter, /* For LC3_Enc_Set_BitRate() */
    lc3_codec_default_function_parameter, /* For LC3_Dec_Get_MemSize() */
    lc3_codec_default_function_parameter, /* For LC3_Dec_Get_Param() */
    lc3_codec_default_function_parameter, /* For LC3_Dec_Init() */
    lc3_codec_default_function_parameter, /* For LC3_Dec_Prcs() */
    lc3_codec_default_function_parameter, /* For LC3_Get_Version() */
};

/* Add for UT of PIC */
#if !PIC_LOGPRINT
#ifndef UNUSED
#define UNUSED(p) ((void)(p))
#endif
extern uint32_t fake_printf(const char *format, ...);
#endif

#if PIC_LOGPRINT
void *g_lc3_codec_import_parameters[3] = {
    printf,
    memcpy,
    memset
};
#else
void *g_lc3_codec_import_parameters[3] = {
    fake_printf,
    memcpy,
    memset
};
#endif

static void lc3_codec_default_function_parameter(void)
{
    DSP_MW_LOG_E("[lc3] codec library not load or had been unload!!!", 0);
    AUDIO_ASSERT(0);
}
#if defined(AIR_BT_CODEC_BLE_V2_ENABLED)
static uint32_t g_lc3_codec_lib_loaded_counter = 0;
static preloader_pisplit_handle_t g_lc3_codec_pisplit_library_handle = NULL;
static SemaphoreHandle_t g_lc3_codec_load_lib_xSemaphore = NULL;

static void lc3_codec_load_library_callback(preloader_pisplit_handle_t handle)
{
    uint32_t i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;

    p_export_parameters = preloader_pisplit_get_export_parameter(handle, g_lc3_codec_import_parameters);
    if (p_export_parameters == NULL) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_get_export_parameter() fail", 0);
        AUDIO_ASSERT(0);
        return;
    }

    for (i = 0; i < (sizeof(g_lc3_codec_export_parameters) / sizeof(void *)); i++) {
        g_lc3_codec_export_parameters[i] = (void *)p_export_parameters[i];
    }

    xSemaphoreGiveFromISR(g_lc3_codec_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


uint32_t lc3_codec_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    uint32_t i, irq_mask;
    preloader_pisplit_error_handling_t error_handling;

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    g_lc3_codec_lib_loaded_counter++;
    if (g_lc3_codec_lib_loaded_counter != 1) {
        hal_nvic_restore_interrupt_mask(irq_mask);
        DSP_MW_LOG_I("[lc3] lc3_codec_library_load() has been called", 0);
        return g_lc3_codec_lib_loaded_counter;
    }
    hal_nvic_restore_interrupt_mask(irq_mask);

    DSP_MW_LOG_I("[lc3] lc3_codec_library_load() begin", 0);

    if (g_lc3_codec_load_lib_xSemaphore == NULL) {
        g_lc3_codec_load_lib_xSemaphore = xSemaphoreCreateBinary();
        AUDIO_ASSERT(g_lc3_codec_load_lib_xSemaphore != NULL);
    }
    error_handling = preloader_pisplit_get_handle(&g_lc3_codec_pisplit_library_handle, &pisplit_lc3_codec, lc3_codec_load_library_callback);
    if (error_handling != PRELOADER_PISPLIT_XTLIB_NO_ERR) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_get_handle() fail %d", 1, error_handling);
        AUDIO_ASSERT(0);
    }
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    error_handling = preloader_pisplit_load_v1(g_lc3_codec_pisplit_library_handle, code_address, data_address);
#else
    error_handling = preloader_pisplit_load(g_lc3_codec_pisplit_library_handle, PRELOADER_I_HIGH_PERFORMANCE, PRELOADER_D_HIGH_PERFORMANCE);
#endif
    if (error_handling != PRELOADER_PISPLIT_XTLIB_NO_ERR) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_load() fail %d", 1, error_handling);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_I_HIGH_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_I_LOW_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_D_HIGH_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_D_LOW_PERFORMANCE);
        AUDIO_ASSERT(0);
        return g_lc3_codec_lib_loaded_counter;
    }
    if (xSemaphoreTake(g_lc3_codec_load_lib_xSemaphore, portMAX_DELAY) != pdTRUE) {
        AUDIO_ASSERT(0);
        return g_lc3_codec_lib_loaded_counter;
    }
    vSemaphoreDelete(g_lc3_codec_load_lib_xSemaphore);
    g_lc3_codec_load_lib_xSemaphore = NULL;
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    preloader_pisplit_get_library_code_and_data_size(g_lc3_codec_pisplit_library_handle, NULL, dram_pic_usage);
#else
    UNUSED(code_address);
    UNUSED(data_address);
    UNUSED(dram_pic_usage);
#endif
    for (i = 0; i < (sizeof(g_lc3_codec_export_parameters) / sizeof(void *)); i++) {
        DSP_MW_LOG_I("[lc3] g_lc3_codec_export_parameters[%d] = 0x%08x", 2, i, g_lc3_codec_export_parameters[i]);
    }

    DSP_MW_LOG_I("[lc3] lc3_codec_library_load() end", 0);
    return g_lc3_codec_lib_loaded_counter;
}

uint32_t lc3_codec_library_unload(void)
{
    uint32_t i;
    preloader_pisplit_error_handling_t error_handling;

    if (g_lc3_codec_lib_loaded_counter != 1) {
        DSP_MW_LOG_W("[lc3] lc3_codec_library_load() has been called before", 0);
        g_lc3_codec_lib_loaded_counter--;
        return g_lc3_codec_lib_loaded_counter;
    }

    DSP_MW_LOG_I("[lc3] lc3_codec_library_unload() begin", 0);
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    error_handling = preloader_pisplit_unload_v1(g_lc3_codec_pisplit_library_handle);
#else
    error_handling = preloader_pisplit_unload(g_lc3_codec_pisplit_library_handle);
#endif
    if (PRELOADER_PISPLIT_XTLIB_NO_ERR != error_handling) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_unload() fail %d", 1, error_handling);
        AUDIO_ASSERT(0);
        return g_lc3_codec_lib_loaded_counter;
    }
    error_handling = preloader_pisplit_release_handle(g_lc3_codec_pisplit_library_handle);
    if (PRELOADER_PISPLIT_XTLIB_NO_ERR != error_handling) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_release_handle() fail %d", 1, error_handling);
        AUDIO_ASSERT(0);
        return g_lc3_codec_lib_loaded_counter;
    }
    for (i = 0; i < (sizeof(g_lc3_codec_export_parameters) / sizeof(void *)); i++) {
        g_lc3_codec_export_parameters[i] = lc3_codec_default_function_parameter;
    }
    g_lc3_codec_pisplit_library_handle = NULL;

    g_lc3_codec_lib_loaded_counter = 0;

    DSP_MW_LOG_I("[lc3] lc3_codec_library_unload() end", 0);
    return g_lc3_codec_lib_loaded_counter;
}
#endif

/* Add for UT of PIC */
#if !PIC_LOGPRINT
#ifndef UNUSED
#define UNUSED(p) ((void)(p))
#endif
extern uint32_t fake_printf(const char *format, ...);
#endif

//////////////////////////////////////////////////////////////////////

static void lc3i_fft7_5ms_default_function_parameter(void);

static SemaphoreHandle_t g_lc3i_fft7_5ms_load_lib_xSemaphore = NULL;
static uint32_t g_lc3i_fft7_5ms_lib_loaded_counter = 0;
static preloader_pisplit_handle_t g_lc3i_fft7_5ms_pisplit_library_handle = NULL;

void *g_lc3i_fft7_5ms_export_parameters[] = {
    lc3i_fft7_5ms_default_function_parameter, /* For fix_fft_Init() */
    lc3i_fft7_5ms_default_function_parameter, /* For fix_fft15() */
    lc3i_fft7_5ms_default_function_parameter, /* For fix_fft30() */
    lc3i_fft7_5ms_default_function_parameter, /* For fix_fft40() */
    lc3i_fft7_5ms_default_function_parameter, /* For FFT4N() */
    lc3i_fft7_5ms_default_function_parameter, /* For FFT8N() */
    lc3i_fft7_5ms_default_function_parameter, /* For FFT12N() */
};

#if PIC_LOGPRINT
void *g_lc3i_fft7_5ms_import_parameters[3] = {
    printf,
    memcpy,
    memset
};
#else
void *g_lc3i_fft7_5ms_import_parameters[3] = {
    fake_printf,
    memcpy,
    memset
};
#endif

static void lc3i_fft7_5ms_default_function_parameter(void)
{
    DSP_MW_LOG_E("[lc3] fft7_5ms library not load or had been unload!!!", 0);
    assert(0);
}

static void lc3i_fft7_5ms_load_library_callback(preloader_pisplit_handle_t handle)
{
    uint32_t i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;

    p_export_parameters = preloader_pisplit_get_export_parameter(handle, g_lc3i_fft7_5ms_import_parameters);
    if (p_export_parameters == NULL) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_get_export_parameter() fail", 0);
        assert(0);
    }

    for (i = 0; i < (sizeof(g_lc3i_fft7_5ms_export_parameters) / sizeof(void *)); i++) {
        g_lc3i_fft7_5ms_export_parameters[i] = (void *)p_export_parameters[i];
    }

    xSemaphoreGiveFromISR(g_lc3i_fft7_5ms_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void lc3i_fft7_5ms_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    uint32_t i, irq_mask;
    preloader_pisplit_error_handling_t error_handling;
    DSP_MW_LOG_I("[lc3] lc3i_fft7_5ms_library_load() %d", 1,g_lc3i_fft7_5ms_lib_loaded_counter);

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    g_lc3i_fft7_5ms_lib_loaded_counter++;
    if (g_lc3i_fft7_5ms_lib_loaded_counter != 1) {
        hal_nvic_restore_interrupt_mask(irq_mask);
        DSP_MW_LOG_I("[lc3] lc3i_fft7_5ms_library_load() has been called %d", 1,g_lc3i_fft7_5ms_lib_loaded_counter);
        return;
    }
    hal_nvic_restore_interrupt_mask(irq_mask);

    DSP_MW_LOG_I("[lc3][DSP_RESOURCE] lc3i_fft7_5ms_library_load() begin", 0);

    if (g_lc3i_fft7_5ms_load_lib_xSemaphore == NULL) {
        g_lc3i_fft7_5ms_load_lib_xSemaphore = xSemaphoreCreateBinary();
        assert(g_lc3i_fft7_5ms_load_lib_xSemaphore != NULL);
    }
    error_handling = preloader_pisplit_get_handle(&g_lc3i_fft7_5ms_pisplit_library_handle, &pisplit_lc3i_fft7p5ms, lc3i_fft7_5ms_load_library_callback);
    if (error_handling != PRELOADER_PISPLIT_XTLIB_NO_ERR) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_get_handle() fail %d", 1, error_handling);
        assert(0);
        return;
    }
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    error_handling = preloader_pisplit_load_v1(g_lc3i_fft7_5ms_pisplit_library_handle, code_address, data_address);
#else
    error_handling = preloader_pisplit_load(g_lc3i_fft7_5ms_pisplit_library_handle, PRELOADER_I_HIGH_PERFORMANCE, PRELOADER_D_HIGH_PERFORMANCE);
#endif
    if (error_handling != PRELOADER_PISPLIT_XTLIB_NO_ERR) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_load() fail %d", 1, error_handling);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_I_HIGH_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_I_LOW_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_D_HIGH_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_D_LOW_PERFORMANCE);
        assert(0);
        return;
    }
    if (xSemaphoreTake(g_lc3i_fft7_5ms_load_lib_xSemaphore, portMAX_DELAY) != pdTRUE) {
        assert(0);
        return;
    }
    vSemaphoreDelete(g_lc3i_fft7_5ms_load_lib_xSemaphore);
    g_lc3i_fft7_5ms_load_lib_xSemaphore = NULL;
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    preloader_pisplit_get_library_code_and_data_size(g_lc3i_fft7_5ms_pisplit_library_handle, NULL, dram_pic_usage);
#else
    UNUSED(code_address);
    UNUSED(data_address);
    UNUSED(dram_pic_usage);
#endif

    for (i = 0; i < (sizeof(g_lc3i_fft7_5ms_export_parameters) / sizeof(void *)); i++) {
        DSP_MW_LOG_I("[lc3] g_lc3i_fft7_5ms_export_parameters[%d] = 0x%08x", 2, i, g_lc3i_fft7_5ms_export_parameters[i]);
    }

    DSP_MW_LOG_I("[lc3] lc3i_fft7_5ms_library_load() end", 0);
}

uint32_t lc3i_fft7_5ms_library_unload(void)
{
    uint32_t i;
    preloader_pisplit_error_handling_t error_handling;

    if (g_lc3i_fft7_5ms_lib_loaded_counter != 1) {
        DSP_MW_LOG_W("[lc3] lc3i_fft7_5ms_library_unload() has been called before %d", 1,g_lc3i_fft7_5ms_lib_loaded_counter);
        g_lc3i_fft7_5ms_lib_loaded_counter--;
        return g_lc3i_fft7_5ms_lib_loaded_counter;
    }

    DSP_MW_LOG_I("[lc3] lc3i_fft7_5ms_library_unload() begin", 0);

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    error_handling = preloader_pisplit_unload_v1(g_lc3i_fft7_5ms_pisplit_library_handle);
#else
    error_handling = preloader_pisplit_unload(g_lc3i_fft7_5ms_pisplit_library_handle);
#endif
    if (PRELOADER_PISPLIT_XTLIB_NO_ERR != error_handling) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_unload() fail %d", 1, error_handling);
        assert(0);
    }
    error_handling = preloader_pisplit_release_handle(g_lc3i_fft7_5ms_pisplit_library_handle);
    if (PRELOADER_PISPLIT_XTLIB_NO_ERR != error_handling) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_release_handle() fail %d", 1, error_handling);
        assert(0);
    }
    for (i = 0; i < (sizeof(g_lc3i_fft7_5ms_export_parameters) / sizeof(void *)); i++) {
        g_lc3i_fft7_5ms_export_parameters[i] = lc3i_fft7_5ms_default_function_parameter;
    }
    g_lc3i_fft7_5ms_pisplit_library_handle = NULL;

    g_lc3i_fft7_5ms_lib_loaded_counter = 0;
    DSP_MW_LOG_I("[lc3] lc3i_fft7_5ms_library_unload() end", 0);
    return g_lc3i_fft7_5ms_lib_loaded_counter;

}

//////////////////////////////////////////////////////////////////////

static void lc3i_fft10ms_default_function_parameter(void);

static SemaphoreHandle_t g_lc3i_fft10ms_load_lib_xSemaphore = NULL;
static uint32_t g_lc3i_fft10ms_lib_loaded_counter = 0;
static preloader_pisplit_handle_t g_lc3i_fft10ms_pisplit_library_handle = NULL;

void *g_lc3i_fft10ms_export_parameters[] = {
    lc3i_fft10ms_default_function_parameter, /* For fix_fft_Init() */
    lc3i_fft10ms_default_function_parameter, /* For fix_fft10() */
    lc3i_fft10ms_default_function_parameter, /* For fix_fft15() */
    lc3i_fft10ms_default_function_parameter, /* For fix_fft20() */
    lc3i_fft10ms_default_function_parameter, /* For fix_fft30() */
    lc3i_fft10ms_default_function_parameter, /* For fix_fft40() */
    lc3i_fft10ms_default_function_parameter, /* For FFT8N() */
};

#if PIC_LOGPRINT
void *g_lc3i_fft10ms_import_parameters[3] = {
    printf,
    memcpy,
    memset
};
#else
void *g_lc3i_fft10ms_import_parameters[3] = {
    fake_printf,
    memcpy,
    memset
};
#endif

static void lc3i_fft10ms_default_function_parameter(void)
{
    DSP_MW_LOG_E("[lc3] fft10ms library not load or had been unload!!!", 0);
    assert(0);
}

static void lc3i_fft10ms_load_library_callback(preloader_pisplit_handle_t handle)
{
    uint32_t i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;

    p_export_parameters = preloader_pisplit_get_export_parameter(handle, g_lc3i_fft10ms_import_parameters);
    if (p_export_parameters == NULL) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_get_export_parameter() fail", 0);
        assert(0);
        return;
    }

    for (i = 0; i < (sizeof(g_lc3i_fft10ms_export_parameters) / sizeof(void *)); i++) {
        g_lc3i_fft10ms_export_parameters[i] = (void *)p_export_parameters[i];
    }

    xSemaphoreGiveFromISR(g_lc3i_fft10ms_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void lc3i_fft10ms_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    uint32_t i, irq_mask;
    preloader_pisplit_error_handling_t error_handling;

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    g_lc3i_fft10ms_lib_loaded_counter++;
    if (g_lc3i_fft10ms_lib_loaded_counter != 1) {
        hal_nvic_restore_interrupt_mask(irq_mask);
        DSP_MW_LOG_I("[lc3] lc3i_fft10ms_library_load() has been called", 0);
        return;
    }
    hal_nvic_restore_interrupt_mask(irq_mask);

    DSP_MW_LOG_I("[lc3][DSP_RESOURCE] lc3i_fft10ms_library_load() begin", 0);

    if (g_lc3i_fft10ms_load_lib_xSemaphore == NULL) {
        g_lc3i_fft10ms_load_lib_xSemaphore = xSemaphoreCreateBinary();
        assert(g_lc3i_fft10ms_load_lib_xSemaphore != NULL);
    }
    error_handling = preloader_pisplit_get_handle(&g_lc3i_fft10ms_pisplit_library_handle, &pisplit_lc3i_fft10ms, lc3i_fft10ms_load_library_callback);
    if (error_handling != PRELOADER_PISPLIT_XTLIB_NO_ERR) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_get_handle() fail %d", 1, error_handling);
        assert(0);
    }
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    error_handling = preloader_pisplit_load_v1(g_lc3i_fft10ms_pisplit_library_handle, code_address, data_address);
#else
    error_handling = preloader_pisplit_load(g_lc3i_fft10ms_pisplit_library_handle, PRELOADER_I_HIGH_PERFORMANCE, PRELOADER_D_HIGH_PERFORMANCE);
#endif
    if (error_handling != PRELOADER_PISPLIT_XTLIB_NO_ERR) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_load() fail %d", 1, error_handling);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_I_HIGH_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_I_LOW_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_D_HIGH_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_D_LOW_PERFORMANCE);
        assert(0);
    }
    if (xSemaphoreTake(g_lc3i_fft10ms_load_lib_xSemaphore, portMAX_DELAY) != pdTRUE) {
        assert(0);
    }
    vSemaphoreDelete(g_lc3i_fft10ms_load_lib_xSemaphore);
    g_lc3i_fft10ms_load_lib_xSemaphore = NULL;
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    preloader_pisplit_get_library_code_and_data_size(g_lc3i_fft10ms_pisplit_library_handle, NULL, dram_pic_usage);
#else
    UNUSED(code_address);
    UNUSED(data_address);
    UNUSED(dram_pic_usage);
#endif

    for (i = 0; i < (sizeof(g_lc3i_fft10ms_export_parameters) / sizeof(void *)); i++) {
        DSP_MW_LOG_I("[lc3] g_lc3i_fft10ms_export_parameters[%d] = 0x%08x", 2, i, g_lc3i_fft10ms_export_parameters[i]);
    }

    DSP_MW_LOG_I("[lc3] lc3i_fft10ms_library_load() end", 0);
}

uint32_t lc3i_fft10ms_library_unload(void)
{
    uint32_t i;
    preloader_pisplit_error_handling_t error_handling;

    if (g_lc3i_fft10ms_lib_loaded_counter != 1) {
        DSP_MW_LOG_W("[lc3] lc3i_fft10ms_library_unload() has been called before %d", 1,g_lc3i_fft10ms_lib_loaded_counter);
        g_lc3i_fft10ms_lib_loaded_counter--;
        return g_lc3i_fft10ms_lib_loaded_counter;
    }

    DSP_MW_LOG_I("[lc3] lc3i_fft10ms_library_unload() begin", 0);
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    error_handling = preloader_pisplit_unload_v1(g_lc3i_fft10ms_pisplit_library_handle);
#else
    error_handling = preloader_pisplit_unload(g_lc3i_fft10ms_pisplit_library_handle);
#endif
    if (PRELOADER_PISPLIT_XTLIB_NO_ERR != error_handling) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_unload() fail %d", 1, error_handling);
        assert(0);
    }
    error_handling = preloader_pisplit_release_handle(g_lc3i_fft10ms_pisplit_library_handle);
    if (PRELOADER_PISPLIT_XTLIB_NO_ERR != error_handling) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_release_handle() fail %d", 1, error_handling);
        assert(0);
    }
    for (i = 0; i < (sizeof(g_lc3i_fft10ms_export_parameters) / sizeof(void *)); i++) {
        g_lc3i_fft10ms_export_parameters[i] = lc3i_fft10ms_default_function_parameter;
    }
    g_lc3i_fft10ms_pisplit_library_handle = NULL;

    g_lc3i_fft10ms_lib_loaded_counter = 0;
    DSP_MW_LOG_I("[lc3] lc3i_fft10ms_library_unload() end", 0);
    return g_lc3i_fft10ms_lib_loaded_counter;

}


//////////////////////////////////////////////////////////////////////

static void lc3i_codec_default_function_parameter(void);

static SemaphoreHandle_t g_lc3i_codec_load_lib_xSemaphore = NULL;
static uint32_t g_lc3i_codec_lib_loaded_counter = 0;
static preloader_pisplit_handle_t g_lc3i_codec_pisplit_library_handle = NULL;
SemaphoreHandle_t g_lc3i_codec_process_xSemaphore = NULL;

void *g_lc3i_codec_export_parameters[] = {
    lc3i_codec_default_function_parameter, /* For LC3_Enc_Prcs() */
    lc3i_codec_default_function_parameter, /* For LC3_Dec_Prcs() */
    lc3i_codec_default_function_parameter, /* For LC3_Dec_Get_Param() */
};

#if PIC_LOGPRINT
void *g_lc3i_codec_import_parameters[3] = {
    printf,
    memcpy,
    memset
};
#else
void *g_lc3i_codec_import_parameters[3] = {
    fake_printf,
    memcpy,
    memset
};
#endif

static void lc3i_codec_default_function_parameter(void)
{
    DSP_MW_LOG_E("[lc3] lc3i codec library not load or had been unload!!!", 0);
    assert(0);
}

static void lc3i_codec_load_library_callback(preloader_pisplit_handle_t handle)
{
    uint32_t i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;

    p_export_parameters = preloader_pisplit_get_export_parameter(handle, g_lc3i_codec_import_parameters);
    if (p_export_parameters == NULL) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_get_export_parameter() fail", 0);
        assert(0);
        return;
    }

    for (i = 0; i < (sizeof(g_lc3i_codec_export_parameters) / sizeof(void *)); i++) {
        g_lc3i_codec_export_parameters[i] = (void *)p_export_parameters[i];
    }

    xSemaphoreGiveFromISR(g_lc3i_codec_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

uint32_t lc3i_codec_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    uint32_t i, irq_mask;
    preloader_pisplit_error_handling_t error_handling;

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    g_lc3i_codec_lib_loaded_counter++;
    if (g_lc3i_codec_lib_loaded_counter != 1) {
        hal_nvic_restore_interrupt_mask(irq_mask);
        DSP_MW_LOG_I("[lc3] lc3i_codec_library_load() has been called", 0);
        #ifdef AIR_DSP_MEMORY_REGION_ENABLE
    preloader_pisplit_get_library_code_and_data_size(g_lc3i_codec_pisplit_library_handle, NULL, dram_pic_usage);
        #endif
        return g_lc3i_codec_lib_loaded_counter;
    }
    hal_nvic_restore_interrupt_mask(irq_mask);

    DSP_MW_LOG_I("[lc3] lc3i_codec_library_load() begin", 0);

    if (g_lc3i_codec_load_lib_xSemaphore == NULL) {
        g_lc3i_codec_load_lib_xSemaphore = xSemaphoreCreateBinary();
        assert(g_lc3i_codec_load_lib_xSemaphore != NULL);
    }
#ifdef AIR_BT_BLE_FIX_ADV_PLC
    error_handling = preloader_pisplit_get_handle(&g_lc3i_codec_pisplit_library_handle, &pisplit_lc3i_codec_APLC, lc3i_codec_load_library_callback);
    DSP_MW_LOG_I("[lc3] preloader load APLC", 0);
#else
    error_handling = preloader_pisplit_get_handle(&g_lc3i_codec_pisplit_library_handle, &pisplit_lc3i_codec, lc3i_codec_load_library_callback);
    DSP_MW_LOG_I("[lc3] preloader load SPLC/TPLC", 0);
#endif
    if (error_handling != PRELOADER_PISPLIT_XTLIB_NO_ERR) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_get_handle() fail %d", 1, error_handling);
        assert(0);
        return g_lc3i_codec_lib_loaded_counter;
    }
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    error_handling = preloader_pisplit_load_v1(g_lc3i_codec_pisplit_library_handle, code_address, data_address);
#else
    error_handling = preloader_pisplit_load(g_lc3i_codec_pisplit_library_handle, PRELOADER_I_HIGH_PERFORMANCE, PRELOADER_D_HIGH_PERFORMANCE);
#endif
    if (error_handling != PRELOADER_PISPLIT_XTLIB_NO_ERR) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_load() fail %d", 1, error_handling);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_I_HIGH_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_I_LOW_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_D_HIGH_PERFORMANCE);
        preloader_pisplit_dump_pic_memory_heap_info(PRELOADER_D_LOW_PERFORMANCE);
        assert(0);
        return g_lc3i_codec_lib_loaded_counter;
    }
    if (xSemaphoreTake(g_lc3i_codec_load_lib_xSemaphore, portMAX_DELAY) != pdTRUE) {
        assert(0);
        return g_lc3i_codec_lib_loaded_counter;
    }
    vSemaphoreDelete(g_lc3i_codec_load_lib_xSemaphore);
    g_lc3i_codec_load_lib_xSemaphore = NULL;
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    preloader_pisplit_get_library_code_and_data_size(g_lc3i_codec_pisplit_library_handle, NULL, dram_pic_usage);
#else
    UNUSED(code_address);
    UNUSED(data_address);
    UNUSED(dram_pic_usage);
#endif
    if (g_lc3i_codec_process_xSemaphore == NULL) {
        g_lc3i_codec_process_xSemaphore = xSemaphoreCreateBinary();
        assert(g_lc3i_codec_process_xSemaphore != NULL);
        xSemaphoreGive(g_lc3i_codec_process_xSemaphore);
    }
    for (i = 0; i < (sizeof(g_lc3i_codec_export_parameters) / sizeof(void *)); i++) {
        DSP_MW_LOG_I("[lc3] g_lc3i_codec_export_parameters[%d] = 0x%08x", 2, i, g_lc3i_codec_export_parameters[i]);
    }

    DSP_MW_LOG_I("[lc3] lc3i_codec_library_load() end", 0);
    return g_lc3i_codec_lib_loaded_counter;
}

uint32_t lc3i_codec_library_unload(void)
{
    uint32_t i;
    preloader_pisplit_error_handling_t error_handling;

    if (g_lc3i_codec_lib_loaded_counter != 1) {
        DSP_MW_LOG_W("[lc3] lc3i_codec_library_unload() has been called before %d",1, g_lc3i_codec_lib_loaded_counter);
        g_lc3i_codec_lib_loaded_counter--;
        return g_lc3i_codec_lib_loaded_counter;
    }

    DSP_MW_LOG_I("[lc3] lc3i_codec_library_unload() begin", 0);

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    error_handling = preloader_pisplit_unload_v1(g_lc3i_codec_pisplit_library_handle);
#else
    error_handling = preloader_pisplit_unload(g_lc3i_codec_pisplit_library_handle);
#endif
    if (PRELOADER_PISPLIT_XTLIB_NO_ERR != error_handling) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_unload() fail %d", 1, error_handling);
        assert(0);
        return g_lc3i_codec_lib_loaded_counter;
    }
    error_handling = preloader_pisplit_release_handle(g_lc3i_codec_pisplit_library_handle);
    if (PRELOADER_PISPLIT_XTLIB_NO_ERR != error_handling) {
        DSP_MW_LOG_E("[lc3] preloader_pisplit_release_handle() fail %d", 1, error_handling);
        assert(0);
        return g_lc3i_codec_lib_loaded_counter;
    }
    for (i = 0; i < (sizeof(g_lc3i_codec_export_parameters) / sizeof(void *)); i++) {
        g_lc3i_codec_export_parameters[i] = lc3i_codec_default_function_parameter;
    }
    g_lc3i_codec_pisplit_library_handle = NULL;

    g_lc3i_codec_lib_loaded_counter = 0;

    vSemaphoreDelete(g_lc3i_codec_process_xSemaphore);
    g_lc3i_codec_process_xSemaphore = NULL;

    DSP_MW_LOG_I("[lc3] lc3i_codec_library_unload() end", 0);
    return g_lc3i_codec_lib_loaded_counter;
}

#endif
