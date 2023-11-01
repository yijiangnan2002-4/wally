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

#ifdef AIR_COMPONENT_CALIBRATION_ENABLE

#include "audio_calibration.h"
#include "bt_sink_srv_ami.h"
#include "race_cmd_dsprealtime.h"
#include "audio_nvdm_common.h"
#include "race_xport.h"
#include "hal_audio_cm4_dsp_message.h"
#include <math.h>
#include "mux_ll_uart_latch.h"

extern void *pvPortMallocNC(size_t xWantedSize);
extern void vPortFreeNC(void *pv);
#ifndef UNUSED
#define UNUSED(x)  ((void)(x))
#endif


#ifndef AUDIO_CALIBRATION_USE_MSGID_LOG
#define AUDIO_CALIBRATION_USE_MSGID_LOG
#endif

log_create_module(audio_calibration, PRINT_LEVEL_INFO);
#define AUDIO_CALIBRATION_LOG_E(fmt,arg...)   LOG_MSGID_E(audio_calibration, "[AUDIO CALIBRATION]"fmt,##arg)
#define AUDIO_CALIBRATION_LOG_W(fmt,arg...)   LOG_MSGID_W(audio_calibration, "[AUDIO CALIBRATION]"fmt,##arg)
#define AUDIO_CALIBRATION_LOG_I(fmt,arg...)   LOG_MSGID_I(audio_calibration, "[AUDIO CALIBRATION]"fmt,##arg)


/******************************************************************************
 * Type Definitions
 ******************************************************************************/
typedef bt_sink_srv_am_notify_callback audio_calibration_notify_cb;


/******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
static audio_calibration_aud_id_type_t g_audio_calibration;



/******************************************************************************
 * Functions Declaration
 ******************************************************************************/
#if 0
#ifdef FREERTOS_ENABLE
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static SemaphoreHandle_t g_audio_calibration_mutex = NULL;
void audio_calibration_mutex_creat(void)
{
    g_audio_calibration_mutex = xSemaphoreCreateMutex();
    if (g_audio_calibration_mutex == NULL) {
        AUDIO_CALIBRATION_LOG_E("audio_calibration_mutex_creat error\r\n", 0);
    }
}
void audio_calibration_mutex_take(void)
{
    if (g_audio_calibration_mutex != NULL) {
        if (xSemaphoreTake(g_audio_calibration_mutex, portMAX_DELAY) == pdFALSE) {
            AUDIO_CALIBRATION_LOG_E("audio_calibration_mutex_take error\r\n", 0);
        }
    } else {
        for (uint16_t i = 1; ; i++) {
            if (g_audio_calibration_mutex != NULL) {
                if (xSemaphoreTake(g_audio_calibration_mutex, portMAX_DELAY) == pdFALSE) {
                    AUDIO_CALIBRATION_LOG_E("audio_calibration_mutex_take error\r\n", 0);
                }
                return;
            }
            if (i == 1000) {
                AUDIO_CALIBRATION_LOG_E("Record_mutex NULL take time out.\r\n", 0);
                configASSERT(0);
            }
            vTaskDelay(2 / portTICK_RATE_MS);
        }
    }
}
void audio_calibration_mutex_give(void)
{
    if (g_audio_calibration_mutex != NULL) {
        if (xSemaphoreGive(g_audio_calibration_mutex) == pdFALSE) {
            AUDIO_CALIBRATION_LOG_E("audio_calibration_mutex_give error\r\n", 0);
        }
    }
}
#else
static int g_audio_calibration_mutex = 1;
void audio_calibration_mutex_creat(void)
{
}
void audio_calibration_mutex_take(void)
{
}
void audio_calibration_mutex_give(void)
{
}
#endif
#endif

#if defined(AIR_COMPONENT_CALIBRATION_SYNC_WITH_ANC_ENABLE)
const audio_calibration_device_matching_t audio_calibration_device_matching_table_for_anc[] = {
    {HAL_AUDIO_DEVICE_MAIN_MIC_L,       HAL_AUDIO_INTERFACE_1, AUDIO_CALIBRATION_COMPONENT_MIC0},
    {HAL_AUDIO_DEVICE_MAIN_MIC_L,       HAL_AUDIO_INTERFACE_2, AUDIO_CALIBRATION_COMPONENT_MIC2},
    {HAL_AUDIO_DEVICE_MAIN_MIC_L,       HAL_AUDIO_INTERFACE_3, AUDIO_CALIBRATION_COMPONENT_MIC4},

    {HAL_AUDIO_DEVICE_MAIN_MIC_R,       HAL_AUDIO_INTERFACE_1, AUDIO_CALIBRATION_COMPONENT_MIC1},
    {HAL_AUDIO_DEVICE_MAIN_MIC_R,       HAL_AUDIO_INTERFACE_2, AUDIO_CALIBRATION_COMPONENT_MIC3},
    {HAL_AUDIO_DEVICE_MAIN_MIC_R,       HAL_AUDIO_INTERFACE_3, AUDIO_CALIBRATION_COMPONENT_MIC5},

    {HAL_AUDIO_DEVICE_DIGITAL_MIC_L,    HAL_AUDIO_INTERFACE_1, AUDIO_CALIBRATION_COMPONENT_MIC0},
    {HAL_AUDIO_DEVICE_DIGITAL_MIC_L,    HAL_AUDIO_INTERFACE_2, AUDIO_CALIBRATION_COMPONENT_MIC2},
    {HAL_AUDIO_DEVICE_DIGITAL_MIC_L,    HAL_AUDIO_INTERFACE_3, AUDIO_CALIBRATION_COMPONENT_MIC4},

    {HAL_AUDIO_DEVICE_DIGITAL_MIC_R,    HAL_AUDIO_INTERFACE_1, AUDIO_CALIBRATION_COMPONENT_MIC1},
    {HAL_AUDIO_DEVICE_DIGITAL_MIC_R,    HAL_AUDIO_INTERFACE_2, AUDIO_CALIBRATION_COMPONENT_MIC3},
    {HAL_AUDIO_DEVICE_DIGITAL_MIC_R,    HAL_AUDIO_INTERFACE_3, AUDIO_CALIBRATION_COMPONENT_MIC5},
};
audio_calibration_component_t audio_calibration_get_anc_matching_component(hal_audio_device_t device, hal_audio_interface_t device_interface)
{
    uint32_t i, table_number;
    table_number = sizeof(audio_calibration_device_matching_table_for_anc)/sizeof(audio_calibration_component_t);
    for (i = 0 ; i < table_number ; i++) {
        if ((audio_calibration_device_matching_table_for_anc[i].device == device) &&
            (audio_calibration_device_matching_table_for_anc[i].device_interface == device_interface)) {
            return audio_calibration_device_matching_table_for_anc[i].component;
        }
    }
    return AUDIO_CALIBRATION_COMPONENT_NUMBER;
}

void audio_calibration_init_anc_matching(void)
{
    mcu2dsp_open_stream_in_param_t open_stream_in_param;
    hal_audio_get_stream_in_setting_config(AU_DSP_ANC, &open_stream_in_param);
    g_audio_calibration.anc_mic_matching_table[AUDIO_ANC_FF_LEFT_CH_MIC] = audio_calibration_get_anc_matching_component(open_stream_in_param.afe.audio_device, open_stream_in_param.afe.audio_interface);
    g_audio_calibration.anc_mic_matching_table[AUDIO_ANC_FB_LEFT_CH_MIC] = audio_calibration_get_anc_matching_component(open_stream_in_param.afe.audio_device1, open_stream_in_param.afe.audio_interface1);
    g_audio_calibration.anc_mic_matching_table[AUDIO_ANC_FF_RIGHT_CH_MIC] = audio_calibration_get_anc_matching_component(open_stream_in_param.afe.audio_device2, open_stream_in_param.afe.audio_interface2);
    g_audio_calibration.anc_mic_matching_table[AUDIO_ANC_FB_RIGHT_CH_MIC] = audio_calibration_get_anc_matching_component(open_stream_in_param.afe.audio_device3, open_stream_in_param.afe.audio_interface3);
}

void audio_calibration_update_anc_gain(audio_calibration_component_t component, int16_t gain_offset)
{
    uint32_t i;
    if ((component >= AUDIO_CALIBRATION_COMPONENT_INPUT_MIN) && (component <= AUDIO_CALIBRATION_COMPONENT_INPUT_MAX)) {
        for (i = 0 ; i < AUDIO_ANC_TOTAL_MIC_NUMBER ; i++) {
            if (component == g_audio_calibration.anc_mic_matching_table[i]) {
                g_audio_calibration.anc_gain.gain_index_mics[i] = gain_offset;
            }
        }
    }
}
#endif

audio_calibration_id_t audio_calibration_init(void *ccni_callback, void *user_data, void *cb_handler)
{
    UNUSED(ccni_callback);
    UNUSED(user_data);
    UNUSED(cb_handler);
    g_audio_calibration.is_init = true;
    audio_calibration_read_nvkey();
    audio_calibration_update_gain_offset_to_dsp();

#if defined(AIR_COMPONENT_CALIBRATION_SYNC_WITH_ANC_ENABLE)
    audio_anc_control_get_calibrate_gain_from_flash(&g_audio_calibration.anc_gain);
    audio_calibration_init_anc_matching();
#endif
    return 0;
}

audio_calibration_result_t audio_calibration_start(void)
{
    return AUDIO_CALIBRATION_EXECUTION_CANCELLED;
}

audio_calibration_result_t audio_calibration_stop(void)
{
    return AUDIO_CALIBRATION_EXECUTION_CANCELLED;
}






audio_calibration_result_t audio_calibration_read_nvkey(void)
{
    uint32_t nvkey_length = 0;
    sysram_status_t nvdm_status;
    uint16_t nvkey_id = NVID_DSP_FW_PARA_COMPONENT_GAIN_OFFSET;


    nvdm_status =  flash_memory_query_nvdm_data_length(nvkey_id, &nvkey_length);
    if (!nvdm_status) {
        nvdm_status = flash_memory_read_nvdm_data(nvkey_id, (uint8_t *)&g_audio_calibration.component, &nvkey_length);
    }

    if (nvdm_status || (nvkey_length != sizeof(audio_calibration_component_gain_offset_t))) {
        AUDIO_CALIBRATION_LOG_E("Read Nvkey data Fail status:%d length:%d ", 2, nvdm_status, nvkey_length);
        assert(0);
    }
    return AUDIO_CALIBRATION_EXECUTION_SUCCESS;
}


audio_calibration_result_t audio_calibration_write_nvkey(void)
{
    uint32_t nvkey_length = 0;
    sysram_status_t nvdm_status;
    uint16_t nvkey_id = NVID_DSP_FW_PARA_COMPONENT_GAIN_OFFSET;
    if (g_audio_calibration.is_init) {
        nvdm_status =  flash_memory_query_nvdm_data_length(nvkey_id, &nvkey_length);
        if (!nvdm_status) {
            nvdm_status = flash_memory_write_nvdm_data(nvkey_id, (uint8_t *)&g_audio_calibration.component, nvkey_length);
#if defined(AIR_COMPONENT_CALIBRATION_SYNC_WITH_ANC_ENABLE)
            //write component gain to ANC DG gain nvkey
            audio_anc_control_set_calibrate_gain_into_flash(g_audio_calibration.anc_gain);
#endif

        }

        if (nvdm_status || !nvkey_length) {
            AUDIO_CALIBRATION_LOG_E("Write Nvkey length Fail status:%d lemgth:%d", 2, nvdm_status, nvkey_length);
            assert(0);
        }
        return AUDIO_CALIBRATION_EXECUTION_SUCCESS;
    }
    return AUDIO_CALIBRATION_EXECUTION_CANCELLED;
}

audio_calibration_result_t audio_calibration_update_gain_offset_to_dsp(void)
{
    uint32_t update_length = 0;

    if (g_audio_calibration.is_init) {
        update_length = sizeof(audio_calibration_component_gain_offset_t);
        hal_audio_dsp_controller_put_paramter((uint8_t *)&g_audio_calibration.component, update_length, AUDIO_MESSAGE_TYPE_COMMON);

        int16_t *malloc_ptr;
        malloc_ptr = (int16_t *)pvPortMallocNC(update_length);
        if (malloc_ptr) {
            memcpy(malloc_ptr, g_audio_calibration.component.gain_offset, update_length);
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DRIVER_PARAM, AUDIO_DRIVER_SET_GAIN_OFFSET, ((uint32_t)malloc_ptr), true);
            vPortFreeNC((void *)malloc_ptr);
        } else {
            AUDIO_CALIBRATION_LOG_E("memory alloc fail", 0);
            assert(0);
        }
#if defined(AIR_COMPONENT_CALIBRATION_SYNC_WITH_ANC_ENABLE)
        //Update component gain to ANC DG gain
        audio_anc_control_set_calibrate_gain(g_audio_calibration.anc_gain);
#endif
    } else {
        //AUDIO_CALIBRATION_LOG_E("didn't initialize", 0);
        assert(0);
    }
    return AUDIO_CALIBRATION_EXECUTION_SUCCESS;
}


int16_t audio_calibration_get_component_gain_offset(audio_calibration_component_t component)
{
    if (g_audio_calibration.is_init && (component < AUDIO_CALIBRATION_COMPONENT_NUMBER)) {
        return g_audio_calibration.component.gain_offset[component];
    }
    AUDIO_CALIBRATION_LOG_W("Get gain offset Fail init:%d, component:%d ", 2, g_audio_calibration.is_init, component);
    return 0;
}

audio_calibration_result_t audio_calibration_set_component_gain_offset(audio_calibration_component_t component, int16_t gain_offset)
{
    if (g_audio_calibration.is_init && (component < AUDIO_CALIBRATION_COMPONENT_NUMBER)) {
        g_audio_calibration.component.gain_offset[component] = gain_offset;
#if defined(AIR_COMPONENT_CALIBRATION_SYNC_WITH_ANC_ENABLE)
        audio_calibration_update_anc_gain(component, gain_offset);
#endif
        return AUDIO_CALIBRATION_EXECUTION_SUCCESS;
    }
    AUDIO_CALIBRATION_LOG_W("Set gain offset Fail init:%d, component:%d ", 2, g_audio_calibration.is_init, component);
    return AUDIO_CALIBRATION_EXECUTION_FAIL;
}


#endif /* AIR_COMPONENT_CALIBRATION_ENABLE */

