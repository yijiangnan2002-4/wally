/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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
#include "dsp_temp.h"
#include "hal_trng.h"
#include "xt_library_loader.h"
#include "preloader_pisplit.h"
#include "voice_nr_portable.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "dsp_feature_interface.h"
#include "preloader_library_common_port.h"
#include "dsp_share_memory.h"

#if PIC_LOGPRINT
#define IGO_PRINTF_IMPORT_API printf
#else
#define IGO_PRINTF_IMPORT_API fake_printf
#endif

static void *g_exnr_import_parameters[1] = {IGO_PRINTF_IMPORT_API};

static void ecnr_default_function_parameter(void)
{
    DSP_MW_LOG_E("[DSP][VOICE_NR] ECNR export function is not initial", 0);
    AUDIO_ASSERT(0);
}

#if defined(AIR_3RD_PARTY_NR_ENABLE)

#ifndef AIR_BTA_IC_STEREO_HIGH_G3
#define TX_EQ_EXPORT_PARAM_NUM (4)

static SemaphoreHandle_t g_tx_eq_load_lib_xSemaphore = NULL;
static preloader_pisplit_handle_t g_tx_eq_pisplit_library_handle = NULL;
static uint32_t g_tx_eq_lib_loaded_counter = 0;

void *g_tx_eq_export_parameters[TX_EQ_EXPORT_PARAM_NUM] = {
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter
};

static void tx_eq_load_library_callback(preloader_pisplit_handle_t handle)
{
    uint32_t i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;

    p_export_parameters = preloader_pisplit_get_export_parameter(handle, g_exnr_import_parameters);
    if (p_export_parameters == NULL) {
        DSP_MW_LOG_E("[DSP][VOICE_NR] TXEQ lib export APIs is NULL", 0);
    } else {
        for (i = 0; i < TX_EQ_EXPORT_PARAM_NUM; i++) {
            g_tx_eq_export_parameters[i] = (void *)p_export_parameters[i];
        }
        xSemaphoreGiveFromISR(g_tx_eq_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

uint32_t tx_eq_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    pic_library_load_info_t load_info;

    g_tx_eq_lib_loaded_counter++;
    if (g_tx_eq_lib_loaded_counter == 1) {
        load_info.code_address = code_address;
        load_info.data_address = data_address;
        load_info.library_callback = tx_eq_load_library_callback;
        load_info.library = &pisplit_fir_eq;
        load_info.dram_size = dram_pic_usage;
        preloader_pisplit_library_load_internal_port(&g_tx_eq_load_lib_xSemaphore, &g_tx_eq_pisplit_library_handle, &load_info);
        }

    DSP_MW_LOG_I("[DSP][VOICE_NR] TXEQ library load ++ finish", 0);

    return 0;
}

uint32_t tx_eq_library_unload(void)
{
    g_tx_eq_lib_loaded_counter--;
    if (g_tx_eq_lib_loaded_counter == 0) {
        preloader_pisplit_library_unload_internal_port(g_tx_eq_pisplit_library_handle, g_tx_eq_export_parameters, TX_EQ_EXPORT_PARAM_NUM, ecnr_default_function_parameter);
        g_tx_eq_pisplit_library_handle = NULL;
    }

    DSP_MW_LOG_I("[DSP][VOICE_NR] TXEQ library unload -- finish", 0);

    return 0;
}
#endif // #ifndef AIR_BTA_IC_STEREO_HIGH_G3

#if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)

#if defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE)
#define IGO_TXNR_PIC_LIB pisplit_igo_aibf15
#else
#define IGO_TXNR_PIC_LIB pisplit_igo_txnr
#endif

#ifdef AIR_BTA_IC_PREMIUM_G2
#if defined(AIR_AI_NR_PREMIUM_ENABLE)
static void *g_igo_txnr_import_parameters[] = {IGO_PRINTF_IMPORT_API, hal_trng_init, hal_trng_get_generated_random_number, hal_trng_deinit};
#endif
#if defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
static void *g_igo_txnr_import_parameters[] = {IGO_PRINTF_IMPORT_API, hal_trng_init, hal_trng_get_generated_random_number, hal_trng_deinit};
#endif
#elif defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
static void *g_igo_txnr_import_parameters[] = {IGO_PRINTF_IMPORT_API};
#endif
static SemaphoreHandle_t g_igo_txnr_load_lib_xSemaphore = NULL;
static preloader_pisplit_handle_t g_igo_txnr_pisplit_library_handle = NULL;
static uint32_t g_igo_txnr_lib_loaded_counter = 0;

#ifdef AIR_BTA_IC_PREMIUM_G2
#define IGO_TXNR_EXPORT_PARAM_NUM (3)
void *g_igo_txnr_export_parameters[IGO_TXNR_EXPORT_PARAM_NUM] = {
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
};
#elif defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
#define IGO_TXNR_EXPORT_PARAM_NUM (3)
void *g_igo_txnr_export_parameters[IGO_TXNR_EXPORT_PARAM_NUM] = {
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter
};
#endif

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
static uint32_t g_igo_txnr_pic_code_size;
static uint32_t g_igo_txnr_pic_data_size;
#endif

static void igo_txnr_load_library_callback(preloader_pisplit_handle_t handle)
{
    uint32_t i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;
    p_export_parameters = preloader_pisplit_get_export_parameter(handle, g_igo_txnr_import_parameters);

    if (p_export_parameters == NULL) {
        DSP_MW_LOG_E("[DSP][VOICE_NR] IGO lib export APIs is NULL", 0);
    } else {
        for (i = 0; i < IGO_TXNR_EXPORT_PARAM_NUM; i++) {
            g_igo_txnr_export_parameters[i] = (void *)p_export_parameters[i];
        }

        xSemaphoreGiveFromISR(g_igo_txnr_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void igo_txnr_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    pic_library_load_info_t load_info;

    g_igo_txnr_lib_loaded_counter++;
    if (g_igo_txnr_lib_loaded_counter == 1) {
        load_info.code_address = code_address;
        load_info.data_address = data_address;
        load_info.library_callback = igo_txnr_load_library_callback;
        load_info.library = &IGO_TXNR_PIC_LIB;
        load_info.dram_size = dram_pic_usage;
        preloader_pisplit_library_load_internal_port(&g_igo_txnr_load_lib_xSemaphore, &g_igo_txnr_pisplit_library_handle, &load_info);

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    preloader_pisplit_get_library_code_and_data_size(g_igo_txnr_pisplit_library_handle, &g_igo_txnr_pic_code_size, &g_igo_txnr_pic_data_size);
#endif
    }

    DSP_MW_LOG_I("[DSP][VOICE_NR] IGO library load ++ finish", 0);
}

void igo_txnr_library_unload(void)
{
    g_igo_txnr_lib_loaded_counter--;
    if (g_igo_txnr_lib_loaded_counter == 0) {
        preloader_pisplit_library_unload_internal_port(g_igo_txnr_pisplit_library_handle, g_igo_txnr_export_parameters, IGO_TXNR_EXPORT_PARAM_NUM, ecnr_default_function_parameter);
        g_igo_txnr_pisplit_library_handle = NULL;
    }

    DSP_MW_LOG_I("[DSP][VOICE_NR] IGO library unload -- finish", 0);
}
#endif
#endif /* defined(AIR_3RD_PARTY_NR_ENABLE) && defined(AIR_AI_NR_PREMIUM_ENABLE) */

#ifdef AIR_BTA_IC_PREMIUM_G2
#if defined(AIR_ECNR_1MIC_INEAR_ENABLE)|| defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_ECNR_1_OR_2_MIC_ENABLE)
        #define ECNR_DECODER_EXPORT_PARAM_NUM (27)
        #define ECNR_PIC_LIB pisplit_ecnr_inear_v2_swb_prcs
    #elif defined(AIR_3RD_PARTY_NR_ENABLE)
        #define ECNR_DECODER_EXPORT_PARAM_NUM (29)
        #define ECNR_PIC_LIB pisplit_ec_rxnr_swb_prcs
#endif
#elif defined(AIR_BTA_IC_PREMIUM_G3)
    #if (defined(AIR_ECNR_1_OR_2_MIC_ENABLE) || defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE))
        #define ECNR_DECODER_EXPORT_PARAM_NUM (26)
        #define ECNR_PIC_LIB pisplit_ecnr_inear_v2_swb
    #elif defined(AIR_3RD_PARTY_NR_ENABLE)
        #define ECNR_DECODER_EXPORT_PARAM_NUM (29)
        #define ECNR_PIC_LIB pisplit_ec_rxnr_swb_prcs
    #endif
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
    #if (defined(AIR_ECNR_1_OR_2_MIC_ENABLE) || defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE))
        #define ECNR_DECODER_EXPORT_PARAM_NUM (27)
        #define ECNR_PIC_LIB pisplit_ecnr_inear_v2_swb_prcs
    #elif defined(AIR_3RD_PARTY_NR_ENABLE)
        #ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
        #define ECFB_DECODER_EXPORT_PARAM_NUM (7)
        #define ECFB_PIC_LIB pisplit_ec_fb
        #endif
        #define ECNR_DECODER_EXPORT_PARAM_NUM (29)
        #define ECNR_PIC_LIB pisplit_ec_rxnr_swb_prcs
    #endif
#endif

static SemaphoreHandle_t g_ecnr_load_lib_xSemaphore = NULL;
static preloader_pisplit_handle_t g_ecnr_pisplit_library_handle = NULL;
static uint32_t g_ecnr_lib_loaded_counter = 0;

#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
static SemaphoreHandle_t g_ecfb_load_lib_xSemaphore = NULL;
static preloader_pisplit_handle_t g_ecfb_pisplit_library_handle = NULL;
static uint32_t g_ecfb_lib_loaded_counter = 0;
#endif

#ifdef AIR_BTA_IC_PREMIUM_G2
void *g_ecnr_decoder_export_parameters[ECNR_DECODER_EXPORT_PARAM_NUM] = {
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
#if defined(AIR_3RD_PARTY_NR_ENABLE)
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
#endif

};
#elif defined(AIR_BTA_IC_PREMIUM_G3)
void *g_ecnr_decoder_export_parameters[ECNR_DECODER_EXPORT_PARAM_NUM] = {
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
#if (defined(AIR_ECNR_1_OR_2_MIC_ENABLE) || defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE))
    ecnr_default_function_parameter,
#elif defined(AIR_3RD_PARTY_NR_ENABLE)
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
#endif
};

#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
void *g_ecnr_decoder_export_parameters[ECNR_DECODER_EXPORT_PARAM_NUM] = {
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
#if defined(AIR_3RD_PARTY_NR_ENABLE)
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
#endif
};
#if defined (AIR_VOICE_BAND_CONFIG_TYPE_FB)
void *g_ecfb_decoder_export_parameters[ECFB_DECODER_EXPORT_PARAM_NUM] = {
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
};
#endif
#endif
#if defined (AIR_VOICE_BAND_CONFIG_TYPE_FB)
static void ecfb_load_library_callback(preloader_pisplit_handle_t handle)
{
    uint32_t i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;

    p_export_parameters = preloader_pisplit_get_export_parameter(handle, g_exnr_import_parameters);
    if (p_export_parameters == NULL) {
        DSP_MW_LOG_E("[DSP][VOICE_NR] ECFB lib export APIs is NULL", 0);
    } else {
        for (i = 0; i < ECFB_DECODER_EXPORT_PARAM_NUM; i++) {
            g_ecfb_decoder_export_parameters[i] = (void *)p_export_parameters[i];
        }
        xSemaphoreGiveFromISR(g_ecfb_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
#endif

static void ecnr_load_library_callback(preloader_pisplit_handle_t handle)
{
    uint32_t i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;

    p_export_parameters = preloader_pisplit_get_export_parameter(handle, g_exnr_import_parameters);
    if (p_export_parameters == NULL) {
        DSP_MW_LOG_E("[DSP][VOICE_NR] ECNR lib export APIs is NULL", 0);
    } else {
        for (i = 0; i < ECNR_DECODER_EXPORT_PARAM_NUM; i++) {
            g_ecnr_decoder_export_parameters[i] = (void *)p_export_parameters[i];
        }
        xSemaphoreGiveFromISR(g_ecnr_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

uint32_t ecnr_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    pic_library_load_info_t load_info;

    if (gDspAlgParameter.EscoMode.Tx != VOICE_FB) {
#if defined(AIR_3RD_PARTY_NR_ENABLE)
#if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
        igo_txnr_library_load(code_address, data_address, dram_pic_usage); /* Load IGO before EC since the memory size of IGO is larger */
#endif
#endif
        g_ecnr_lib_loaded_counter++;
        if (g_ecnr_lib_loaded_counter == 1) {
#if defined(AIR_DSP_MEMORY_REGION_ENABLE) && defined(AIR_3RD_PARTY_NR_ENABLE) && (defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE))
            load_info.code_address = ((uint32_t)code_address + g_igo_txnr_pic_code_size + 31) & (~31);
            load_info.data_address = ((uint32_t)data_address + g_igo_txnr_pic_data_size + 31) & (~31)
#else
            load_info.code_address = code_address;
            load_info.data_address = data_address;
#endif
            load_info.library_callback = ecnr_load_library_callback;
            load_info.library = &ECNR_PIC_LIB;
            load_info.dram_size = dram_pic_usage;
            preloader_pisplit_library_load_internal_port(&g_ecnr_load_lib_xSemaphore, &g_ecnr_pisplit_library_handle, &load_info);
#if defined(AIR_DSP_MEMORY_REGION_ENABLE) && defined(AIR_3RD_PARTY_NR_ENABLE) && (defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE))
        *dram_pic_usage += g_igo_txnr_pic_data_size;
#endif
        }
    }

#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
    if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
        g_ecfb_lib_loaded_counter++;
        if (g_ecfb_lib_loaded_counter == 1) {
            load_info.code_address = code_address;
            load_info.data_address = data_address;
            load_info.library_callback = ecfb_load_library_callback;
            load_info.library = &ECFB_PIC_LIB;
            load_info.dram_size = dram_pic_usage;
            preloader_pisplit_library_load_internal_port(&g_ecfb_load_lib_xSemaphore, &g_ecfb_pisplit_library_handle, &load_info);
            DSP_MW_LOG_I("[DSP][VOICE_NR] load Fullband", 0);
        }
    }
#endif

    DSP_MW_LOG_I("[DSP][VOICE_NR] ECNR library load ++ finish", 0);

    return 0;
}

uint32_t ecnr_library_unload(void)
{

    if (gDspAlgParameter.EscoMode.Tx != VOICE_FB) {
#if defined(AIR_3RD_PARTY_NR_ENABLE)
#if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
        igo_txnr_library_unload();
#endif
#endif

        g_ecnr_lib_loaded_counter--;
        if (g_ecnr_lib_loaded_counter == 0) {
            preloader_pisplit_library_unload_internal_port(g_ecnr_pisplit_library_handle, g_ecnr_decoder_export_parameters, ECNR_DECODER_EXPORT_PARAM_NUM, ecnr_default_function_parameter);
            g_ecnr_pisplit_library_handle = NULL;
        }
    }
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
    if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
        g_ecfb_lib_loaded_counter--;
        if (g_ecfb_lib_loaded_counter == 0) {
            preloader_pisplit_library_unload_internal_port(g_ecfb_pisplit_library_handle, g_ecfb_decoder_export_parameters, ECFB_DECODER_EXPORT_PARAM_NUM, ecnr_default_function_parameter);
            g_ecfb_pisplit_library_handle = NULL;
            DSP_MW_LOG_I("[DSP][VOICE_NR] unload Fullband", 0);
        }
    }
#endif

    DSP_MW_LOG_I("[DSP][VOICE_NR] ECNR library unload -- finish", 0);

    return 0;
}

#if defined(AIR_3RD_PARTY_NR_ENABLE)
#if defined(AIR_ECNR_PREV_PART_ENABLE)

#ifdef AIR_ECNR_SEPARATE_MODE_ENABLE

uint32_t ecnr_prev_part_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    return ecnr_library_load(code_address, data_address, dram_pic_usage);
}
uint32_t ecnr_prev_part_library_unload(void)
{
    return ecnr_library_unload();
}

#else

#define ECNR_PREV_DECODER_EXPORT_PARAM_NUM (15)
#ifdef AIR_BTA_IC_PREMIUM_G2
#define ECNR_PREV_PIC_LIB pisplit_ec120
#elif defined(AIR_BTA_IC_PREMIUM_G3)
#define ECNR_PREV_PIC_LIB pisplit_ec120_swb
#endif

static SemaphoreHandle_t g_ecnr_prev_load_lib_xSemaphore = NULL;
static preloader_pisplit_handle_t g_ecnr_prev_pisplit_library_handle = NULL;
static uint32_t g_ecnr_prev_lib_loaded_counter = 0;

void *g_ecnr_prev_decoder_export_parameters[ECNR_PREV_DECODER_EXPORT_PARAM_NUM] = {
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
};

static void ecnr_prev_load_library_callback(preloader_pisplit_handle_t handle)
{
    uint32_t i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;

    p_export_parameters = preloader_pisplit_get_export_parameter(handle, g_exnr_import_parameters);
    if (p_export_parameters == NULL) {
        DSP_MW_LOG_E("[DSP][VOICE_NR] ECNR lib export APIs is NULL", 0);
    } else {
        for (i = 0; i < ECNR_PREV_DECODER_EXPORT_PARAM_NUM; i++) {
            g_ecnr_prev_decoder_export_parameters[i] = (void *)p_export_parameters[i];
        }
        xSemaphoreGiveFromISR(g_ecnr_prev_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

uint32_t ecnr_prev_part_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    pic_library_load_info_t load_info;

        g_ecnr_prev_lib_loaded_counter++;
    if (g_ecnr_prev_lib_loaded_counter == 1) {
        load_info.code_address = code_address;
        load_info.data_address = data_address;
        load_info.library_callback = ecnr_prev_load_library_callback;
        load_info.library = &ECNR_PREV_PIC_LIB;
        load_info.dram_size = dram_pic_usage;
        preloader_pisplit_library_load_internal_port(&g_ecnr_prev_load_lib_xSemaphore, &g_ecnr_prev_pisplit_library_handle, &load_info);
    }

    DSP_MW_LOG_I("[DSP][AEC/NR] ec120_library_load ++ finish", 0);

    return 0;
}

uint32_t ecnr_prev_part_library_unload(void)
{
        g_ecnr_prev_lib_loaded_counter--;
    if (g_ecnr_prev_lib_loaded_counter == 0) {
        preloader_pisplit_library_unload_internal_port(g_ecnr_prev_pisplit_library_handle, g_ecnr_prev_decoder_export_parameters, ECNR_PREV_DECODER_EXPORT_PARAM_NUM, ecnr_default_function_parameter);
        g_ecnr_prev_pisplit_library_handle = NULL;
    }

    DSP_MW_LOG_I("[DSP][AEC/NR] ec120_library_unload -- finish", 0);

    return 0;
}
#endif
#endif
#endif

#if defined(AIR_3RD_PARTY_NR_ENABLE)
#if defined(AIR_ECNR_POST_PART_ENABLE)

#ifdef AIR_ECNR_SEPARATE_MODE_ENABLE

uint32_t ecnr_post_ec_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    return ecnr_library_load(code_address, data_address, dram_pic_usage);
}
uint32_t ecnr_post_ec_library_unload(void)
{
    return ecnr_library_unload();
}

#else

#define ECNR_POST_DECODER_EXPORT_PARAM_NUM (3)
#define ECNR_POST_PIC_LIB pisplit_post_ec

static SemaphoreHandle_t g_ecnr_post_load_lib_xSemaphore = NULL;
static preloader_pisplit_handle_t g_ecnr_post_pisplit_library_handle = NULL;
static uint32_t g_ecnr_post_lib_loaded_counter = 0;

void *g_ecnr_post_decoder_export_parameters[ECNR_POST_DECODER_EXPORT_PARAM_NUM] = {
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
    ecnr_default_function_parameter,
};

static void ecnr_post_load_library_callback(preloader_pisplit_handle_t handle)
{
    uint32_t i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t *p_export_parameters;

    p_export_parameters = preloader_pisplit_get_export_parameter(handle, g_exnr_import_parameters);
    if (p_export_parameters == NULL) {
        DSP_MW_LOG_E("[DSP][VOICE_NR] ECNR lib export APIs is NULL", 0);
    } else {
        for (i = 0; i < ECNR_POST_DECODER_EXPORT_PARAM_NUM; i++) {
            g_ecnr_post_decoder_export_parameters[i] = (void *)p_export_parameters[i];
        }
        xSemaphoreGiveFromISR(g_ecnr_post_load_lib_xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

uint32_t ecnr_post_ec_library_load(void *code_address, void *data_address, uint32_t *dram_pic_usage)
{
    pic_library_load_info_t load_info;

    if (gDspAlgParameter.EscoMode.Tx != VOICE_FB) {
#if defined(AIR_3RD_PARTY_NR_ENABLE)
#if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
        igo_txnr_library_load(code_address, data_address, dram_pic_usage); /* Load IGO before EC since the memory size of IGO is larger */
#endif
#endif

        g_ecnr_post_lib_loaded_counter++;
        if (g_ecnr_post_lib_loaded_counter == 1) {
#if defined(AIR_DSP_MEMORY_REGION_ENABLE) && defined(AIR_3RD_PARTY_NR_ENABLE) && (defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE))
            load_info.code_address = ((uint32_t)code_address + g_igo_txnr_pic_code_size + 31) & (~31);
            load_info.data_address = ((uint32_t)data_address + g_igo_txnr_pic_data_size + 31) & (~31)
#else
            load_info.code_address = code_address;
            load_info.data_address = data_address;
#endif
            load_info.library_callback = ecnr_post_load_library_callback;
            load_info.library = &ECNR_POST_PIC_LIB;
            load_info.dram_size = dram_pic_usage;
            preloader_pisplit_library_load_internal_port(&g_ecnr_post_load_lib_xSemaphore, &g_ecnr_post_pisplit_library_handle, &load_info);
#if defined(AIR_DSP_MEMORY_REGION_ENABLE) && defined(AIR_3RD_PARTY_NR_ENABLE) && (defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE))
            *dram_pic_usage += g_igo_txnr_pic_data_size;
#endif
        }
    }
#ifdef AIR_VOICE_BAND_CONFIG_TYPE_FB
    if (gDspAlgParameter.EscoMode.Tx == VOICE_FB) {
        g_ecfb_lib_loaded_counter++;
        if (g_ecfb_lib_loaded_counter == 1) {
            load_info.code_address = code_address;
            load_info.data_address = data_address;
            load_info.library_callback = ecfb_load_library_callback;
            load_info.library = &ECFB_PIC_LIB;
            load_info.dram_size = dram_pic_usage;
            preloader_pisplit_library_load_internal_port(&g_ecfb_load_lib_xSemaphore, &g_ecfb_pisplit_library_handle, &load_info);
            DSP_MW_LOG_I("[DSP][VOICE_NR] load Fullband", 0);
        }
    }
#endif
    DSP_MW_LOG_I("[DSP][VOICE_NR] PostEC library load ++ finish", 0);

    return 0;
}

uint32_t ecnr_post_ec_library_unload(void)
{
#if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
    igo_txnr_library_unload();
#endif

        g_ecnr_post_lib_loaded_counter--;
    if (g_ecnr_post_lib_loaded_counter == 0) {
        preloader_pisplit_library_unload_internal_port(g_ecnr_post_pisplit_library_handle, g_ecnr_post_decoder_export_parameters, ECNR_POST_DECODER_EXPORT_PARAM_NUM, ecnr_default_function_parameter);
        g_ecnr_post_pisplit_library_handle = NULL;
    }

    DSP_MW_LOG_I("[DSP][VOICE_NR] PostEC library unload -- finish", 0);

    return 0;
}
#endif
#endif
#endif

