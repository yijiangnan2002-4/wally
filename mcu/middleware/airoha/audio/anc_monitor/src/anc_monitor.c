/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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
#include <assert.h>

#include "anc_monitor.h"
#include "bt_sink_srv_ami.h"
#include "anc_control_api.h"
#include "hal_audio_cm4_dsp_message.h"
#include "hal_resource_assignment.h"
#include "semphr.h"

#ifdef AIR_ANC_WIND_DETECTION_ENABLE
#include "dsp_para_wind_nvkey_struct.h"
#endif
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
#include "dsp_para_user_unaware_nvkey_struct.h"
#endif
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
#include "dsp_para_environment_detection_nvkey_struct.h"
#endif
#ifdef AIR_ICE_DEBUG_ENABLE
#include "hal_ice_debug.h"
#endif


#if defined(MTK_ANC_SURROUND_MONITOR_ENABLE)
// ======== Definition ========

#ifdef ADAPIVE_ANC_STREAM_CONTROL
bool g_adaptive_ANC_stream_MP_control_flag = false;
bool g_adaptive_ANC_stream_suspend_control_flag = false;
static audio_anc_monitor_adaptive_anc_status_t g_adaptive_anc_status;
#endif
bool g_adaptive_ANC_suspend_by_ANC = false;
static SemaphoreHandle_t g_adaptive_ANC_semaphore = NULL;
bool g_adaptive_ANC_getinfo_DSP_write_done = false;
audio_anc_monitor_scenario_type_t g_ANC_monitor_get_ANC_type, g_ANC_monitor_get_ANC_type_pre = 0; //0 for ANC, 1 for PT, 2 for Sidetone
bool g_ANC_monitor_sidetone_exist, g_ANC_monitor_sidetone_exist_pre = false;

#ifdef AIR_ANC_ADAP_PT_ENABLE
extern bool g_anc_adaptive_pt_suspended;
extern bool g_anc_monitor_wind_status;
extern int16_t g_anc_monitor_wind_regular_extend_gain;
#endif

uint16_t g_anc_monitor_wind_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_WIND_DET; //NVID_DSP_ALG_ANC_WIND_DET_PT/ NVID_DSP_ALG_ANC_WIND_DET_SIDETONE
uint16_t g_anc_monitor_ed_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION; //NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_PT/ NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_SIDETONE

// ======== Static Structure ========
uint32_t anc_monitor_message_date;
audio_transmitter_monitor_config_t anc_monitor_transmitter;


// ======== Function Declaration ========



// ======== Private utility functions ========
#ifdef ADAPIVE_ANC_STREAM_CONTROL
void audio_anc_monitor_transmitter_print_status()
{
    TRANSMITTER_LOG_I("[ANC_Monitor]Enable/Suspend status: wnd %d/%d, uu %d/%d, ed %d/%d,", 6,
                      g_adaptive_anc_status.wnd_enable, g_adaptive_anc_status.wnd_suspend,
                      g_adaptive_anc_status.uu_enable, g_adaptive_anc_status.uu_suspend,
                      g_adaptive_anc_status.ed_enable, g_adaptive_anc_status.ed_suspend);
}
#endif

static void audio_anc_monitor_update_nvkey_id_by_anc_type(audio_anc_monitor_scenario_type_t anc_type)
{
    if (anc_type == AUDIO_ANC_MONI_SCENARIO_TYPE_ANC) { //ANC
        #ifdef AIR_ANC_WIND_DETECTION_ENABLE
        g_anc_monitor_wind_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_WIND_DET;
        #endif
        #ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
        g_anc_monitor_ed_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION;
        #endif
    } else if (anc_type == AUDIO_ANC_MONI_SCENARIO_TYPE_PT) { //PT
        #ifdef AIR_ANC_WIND_DETECTION_ENABLE
        g_anc_monitor_wind_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_WIND_DET_PT;
        #endif
        #ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
        g_anc_monitor_ed_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_PT;
        #endif
    } else if (anc_type == AUDIO_ANC_MONI_SCENARIO_TYPE_SIDETONE) { //SIDETONE
        #ifdef AIR_ANC_WIND_DETECTION_ENABLE
        g_anc_monitor_wind_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_WIND_DET_SIDETONE;
        #endif
        #ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
        g_anc_monitor_ed_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_SIDETONE;
        #endif
#ifdef AIR_HW_VIVID_PT_ENABLE
    } else if (anc_type == AUDIO_ANC_MONI_SCENARIO_TYPE_HW_VIVID_PT) { //HW VIVID PT
        #ifdef AIR_ANC_WIND_DETECTION_ENABLE
        g_anc_monitor_wind_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_WIND_DET_HW_VIVID_PT;
        #endif
        #ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
        g_anc_monitor_ed_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_HW_VIVID_PT;
        #endif
#endif
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    } else if (anc_type == AUDIO_ANC_MONI_SCENARIO_TYPE_HA_PSAP) { //HA / PSAP
        #ifdef AIR_ANC_WIND_DETECTION_ENABLE
        g_anc_monitor_wind_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_WIND_DET_HA_PSAP;
        #endif
        #ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
        g_anc_monitor_ed_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_HA_PSAP;
        #endif
#endif
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
    } else if (anc_type == AUDIO_ANC_MONI_SCENARIO_TYPE_SW_VIVID_PT) { //SW ViVID
        #ifdef AIR_ANC_WIND_DETECTION_ENABLE
        g_anc_monitor_wind_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_WIND_DET_SW_VIVID_PT;
        #endif
        #ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
        g_anc_monitor_ed_nvkey_id_by_anc_type = NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_SW_VIVID_PT;
        #endif
#endif
    } else {
        TRANSMITTER_LOG_E("[ANC_Monitor]audio_anc_monitor_update_nvkey_id_by_anc_type type:%d ", 1, anc_type);
        assert(0);
    }
}

void audio_anc_monitor_transmitter_send_ccni_event_to_am(uint16_t nvkey_id)
{
    //send to AM
    bt_sink_srv_am_feature_t feature_para;
    memset(&feature_para, 0, sizeof(bt_sink_srv_am_feature_t));
    feature_para.type_mask = AM_ANC_MONITOR;
    feature_para.feature_param.anc_monitor_param.event = AUDIO_ANC_MONITOR_SET_PARAMS;
    feature_para.feature_param.anc_monitor_param.event_type = nvkey_id;
    //feature_para.feature_param.anc_monitor_param.param = p_param_share; //share buffer addr
    am_audio_set_feature(FEATURE_NO_NEED_ID,&feature_para);
    TRANSMITTER_LOG_I("[ANC_Monitor]transmitter_send_ccni_to_am, id:0x%x", 1, nvkey_id);
}

void audio_anc_monitor_transmitter_send_ccni(uint16_t nvkey_id)
{
    uint32_t nvkey_length = 0;
    void *malloc_ptr = NULL;
    void *p_param_share;
    sysram_status_t nvdm_status;

    nvdm_status =  flash_memory_query_nvdm_data_length(nvkey_id, &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        TRANSMITTER_LOG_E("[ANC_Monitor]transmitter_send_ccni, Query Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
    }

    malloc_ptr = pvPortMalloc(nvkey_length);

    if (malloc_ptr) {
        nvdm_status = flash_memory_read_nvdm_data(nvkey_id, (uint8_t *)malloc_ptr, &nvkey_length);
        if (nvdm_status) {
            TRANSMITTER_LOG_E("[ANC_Monitor]transmitter_send_ccni, Read Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
            vPortFree(malloc_ptr);
            return;
        }
        p_param_share = hal_audio_dsp_controller_put_paramter(malloc_ptr, nvkey_length, AUDIO_MESSAGE_TYPE_COMMON);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_ALGORITHM_PARAM, nvkey_id, (uint32_t)p_param_share, true);
        vPortFree(malloc_ptr);
    } else {
        configASSERT(0 && "[ANC_Monitor]transmitter_send_ccni, malloc Fail");
    }
}


void audio_anc_monitor_transmitter_set_parameters(void)
{
    //Wind detect
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
    audio_anc_monitor_transmitter_send_ccni_event_to_am(g_anc_monitor_wind_nvkey_id_by_anc_type);
#endif
    //User unaware
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    audio_anc_monitor_transmitter_send_ccni_event_to_am(NVID_DSP_ALG_ANC_USR_UNAWARE);
#endif
    //Environment Detection
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
    audio_anc_monitor_transmitter_send_ccni_event_to_am(g_anc_monitor_ed_nvkey_id_by_anc_type);
#endif
}

static void audio_anc_monitor_transmitter_receive_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    (void)(event);
    (void)(data);
    (void)(user_data);
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS:
            //Load Nvkey
            audio_anc_monitor_transmitter_set_parameters();
            break;
        case AUDIO_TRANSMITTER_EVENT_START_FAIL:
            if (anc_monitor_transmitter.is_enable) {
                audio_transmitter_start(anc_monitor_transmitter.receive_id);
            }
            break;
        default:
            break;
    }
}

// ======== Public functions ========
void audio_anc_monitor_anc_init(void)
{
    audio_transmitter_config_t config = {};
    config.scenario_type = AUDIO_TRANSMITTER_ANC_MONITOR_STREAM;
    config.scenario_sub_id = 0;
    config.msg_handler = audio_anc_monitor_transmitter_receive_callback;
    config.user_data = NULL;
    anc_monitor_transmitter.receive_id = audio_transmitter_init(&config);

#ifdef ADAPIVE_ANC_STREAM_CONTROL
    audio_anc_monitor_init_adaptive_anc_status();
    audio_anc_monitor_transmitter_print_status();
#endif
    if (g_adaptive_ANC_semaphore == NULL) {
        g_adaptive_ANC_semaphore = xSemaphoreCreateBinary();
        if (!g_adaptive_ANC_semaphore) {
            configASSERT(0 && "[ANC_Monitor]audio_anc_monitor_anc_init create get_info_semaphore FAIL \n");
        } else {
            xSemaphoreGive(g_adaptive_ANC_semaphore);
        }
    }
#if defined(AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE)
    //Always enable ANC stream
    audio_anc_monitor_anc_callback(AUDIO_ANC_CONTROL_EVENT_ON, AUDIO_ANC_CONTROL_EXECUTION_SUCCESS);
#endif

}

static void audio_anc_monitor_get_anc_status(uint8_t *enable, audio_anc_monitor_scenario_type_t *anc_type, uint32_t *misc1, uint32_t *misc2)
{
    uint8_t                            anc_enable;
    audio_anc_control_filter_id_t      anc_current_filter_id;
    audio_anc_control_type_t           anc_current_type;
    int16_t                            anc_runtime_gain;
    uint8_t                            support_hybrid_enable;
    audio_anc_control_misc_t           local_misc;
    audio_anc_monitor_scenario_type_t  local_anc_type = 0; //0 for ANC, 1 for PT, 2 for Sidetone
    memset(&local_misc, 0, sizeof(audio_anc_control_misc_t));
    audio_anc_control_get_status(&anc_enable, &anc_current_filter_id, &anc_current_type, &anc_runtime_gain, &support_hybrid_enable, &local_misc);
    if (enable != NULL) {
        *enable = anc_enable;
    }

#ifdef AIR_ANC_V3
    if ((local_misc.extend_use_parameters != 0) && (anc_enable == 0)) {
        // Sidetone enable only
        local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_SIDETONE;
        g_ANC_monitor_sidetone_exist = true;
    } else if ((local_misc.extend_use_parameters != 0) && (anc_enable != 0)) {
        if (anc_current_filter_id <= 4/*AUDIO_ANC_CONTROL_FILTER_ID_ANC_END*/) {
            // Sidetone enable & ANC enable
            local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_ANC;
        } else {
            // Sidetone enable & PT enable
            local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_PT;
        }
        g_ANC_monitor_sidetone_exist = true;
    } else if ((local_misc.extend_use_parameters == 0) && (anc_enable != 0)) {
        if (anc_current_filter_id <= 4/*AUDIO_ANC_CONTROL_FILTER_ID_ANC_END*/) {
            // ANC enable only
            local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_ANC;
        } else {
            // PT enable only
            local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_PT;
        }
        g_ANC_monitor_sidetone_exist = false;
    } else if ((local_misc.extend_use_parameters == 0) && (anc_enable == 0)) {
        // all off
        g_ANC_monitor_sidetone_exist = false;
    }
#else
    if ((anc_current_filter_id == AUDIO_ANC_CONTROL_PASS_THRU_SIDETONE_FILTER_DEFAULT) && (anc_enable != 0)) {
        // Sidetone enable
        //V2 don't have pure sidetone
        if ((anc_current_type == AUDIO_ANC_CONTROL_TYPE_PASSTHRU_FF)||(anc_current_type == AUDIO_ANC_CONTROL_TYPE_PT_HYBRID)||(anc_current_type == AUDIO_ANC_CONTROL_TYPE_PT_FB)) {
            local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_PT;
        } else {
            local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_ANC;
        }
        g_ANC_monitor_sidetone_exist = true;
    } else if (anc_enable != 0) {
        if (anc_current_filter_id <= 4/*AUDIO_ANC_CONTROL_FILTER_ID_ANC_END*/) {
            // ANC enable only
            local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_ANC;
        } else {
            // PT enable only
            local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_PT;
        }
        g_ANC_monitor_sidetone_exist = false;
    } else if (anc_enable == 0) {
        // all off
        g_ANC_monitor_sidetone_exist = false;
    }
#endif
    #ifdef AIR_ANC_ADAP_PT_ENABLE //special case for ADAP_PT
    //anc_current_type = anc_get_official_type_from_internal_type(anc_current_type);
    if ((anc_enable != 0) && ((anc_current_type & 0xFFFF0000) == AUDIO_ANC_CONTROL_TYPE_PT_ADAP)) {
        local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_PT;
    }
    #endif

    #ifdef AIR_HW_VIVID_PT_ENABLE
    if ((anc_enable != 0) && ((anc_current_type & 0xFFFF0000) == AUDIO_ANC_CONTROL_TYPE_PT_HW_VIVID)) {
        //Enable HW vivid PT
        local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_HW_VIVID_PT;
    }
    #endif

    if ((anc_enable != 0) && ((anc_current_type & 0xFFFF0000) == AUDIO_ANC_CONTROL_TYPE_PT_HA_PSAP)) {
        //Enable PSAP/HA
        local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_HA_PSAP;
    }

    if ((anc_enable != 0) && ((anc_current_type & 0xFFFF0000) == AUDIO_ANC_CONTROL_TYPE_PT_SW_VIVID)) {
        //Enable SW vivid PT
        local_anc_type = AUDIO_ANC_MONI_SCENARIO_TYPE_SW_VIVID_PT;
    }

    if (anc_type != NULL) {
        *anc_type = local_anc_type;
    }
}

void audio_anc_monitor_anc_callback(audio_anc_control_event_t event_id, audio_anc_control_result_t result)
{
    audio_transmitter_status_t transmitter_receive_status;
    uint8_t start_stop_fail_reason = 0; //1: all features are disabled, 2: suspended or under MP mode, 3: AM start_stop fail, 4: anc_control_result error, 5: others, already enabled or unexpected event_id
    uint8_t anc_is_enabled;
    audio_anc_monitor_get_anc_status(&anc_is_enabled, &g_ANC_monitor_get_ANC_type, NULL, NULL);
    audio_anc_monitor_update_nvkey_id_by_anc_type(g_ANC_monitor_get_ANC_type);
    audio_anc_monitor_init_adaptive_anc_status();

    if ((g_ANC_monitor_get_ANC_type == AUDIO_ANC_MONI_SCENARIO_TYPE_SIDETONE) && (event_id == AUDIO_ANC_CONTROL_EVENT_OFF)) { //pure sidetone special regnization
        event_id = AUDIO_ANC_CONTROL_EVENT_ON;
    }

#if defined(AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE)
    event_id = AUDIO_ANC_CONTROL_EVENT_ON;
    result = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
#endif

    TRANSMITTER_LOG_I("[ANC_Monitor] ANC callback - event:%d, result:%d, dsp status:%d, ANC_suspend:%d, ANC_enable:%d, ANC_type: %d", 6, event_id, result, anc_monitor_transmitter.is_enable, g_adaptive_ANC_suspend_by_ANC, anc_is_enabled, g_ANC_monitor_get_ANC_type);

    if ((event_id == AUDIO_ANC_CONTROL_EVENT_RESUME_NOTIFY)) {
        g_adaptive_ANC_suspend_by_ANC = false;
        if (!anc_is_enabled) {
            return;
        }
    } else if ((event_id == AUDIO_ANC_CONTROL_EVENT_SUSPEND_NOTIFY)) {
        g_adaptive_ANC_suspend_by_ANC = true; //ANC is suspend, also suspend Adaptive ANC
    }

#if defined(ADAPIVE_ANC_STREAM_CONTROL) && !defined(AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE)
    if (((event_id == AUDIO_ANC_CONTROL_EVENT_ON) || (event_id == AUDIO_ANC_CONTROL_EVENT_RESUME_NOTIFY)) && (!g_adaptive_anc_status.uu_enable) && (!g_adaptive_anc_status.ed_enable) && (!g_adaptive_anc_status.wnd_enable)) {
        if(anc_monitor_transmitter.is_enable) {
            audio_anc_monitor_adaptive_anc_stream_enable(false);
        }
//            start_stop_fail_reason = 1; //1: all features are disabled
        TRANSMITTER_LOG_E("[ANC_Monitor]audio_anc_monitor_callback wnd,ed,uu don't need to enable",0);
        return;
    }
    if ((g_adaptive_ANC_stream_MP_control_flag == true) || (g_adaptive_ANC_stream_suspend_control_flag == true)) { //under MP mode or suspended
        start_stop_fail_reason = 2; //2: suspended or under MP mode
    } else { //
#endif

#ifdef MTK_ANC_V2
        if (result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) {
            if (((event_id == AUDIO_ANC_CONTROL_EVENT_ON) || (event_id == AUDIO_ANC_CONTROL_EVENT_RESUME_NOTIFY)) && (!anc_monitor_transmitter.is_enable) && (!g_adaptive_ANC_suspend_by_ANC)) {
                anc_monitor_transmitter.is_enable = true;
                g_ANC_monitor_get_ANC_type_pre = g_ANC_monitor_get_ANC_type;
                transmitter_receive_status = audio_transmitter_start(anc_monitor_transmitter.receive_id);
                if (transmitter_receive_status != AUDIO_TRANSMITTER_STATUS_SUCCESS) {
                    anc_monitor_transmitter.is_enable = false;
                    start_stop_fail_reason = 3; //3: AM start_stop fail
                }
            } else if (((event_id == AUDIO_ANC_CONTROL_EVENT_OFF) || (event_id == AUDIO_ANC_CONTROL_EVENT_FORCE_OFF) || (event_id == AUDIO_ANC_CONTROL_EVENT_SUSPEND_NOTIFY)) && (anc_monitor_transmitter.is_enable)) {
                transmitter_receive_status = audio_transmitter_stop(anc_monitor_transmitter.receive_id);
                if (transmitter_receive_status == AUDIO_TRANSMITTER_STATUS_SUCCESS) {
                    anc_monitor_transmitter.is_enable = false;
                } else {
                    start_stop_fail_reason = 3; //3: AM start_stop fail
                }
                g_ANC_monitor_get_ANC_type_pre = 0;
                g_ANC_monitor_get_ANC_type = 0;
            } else {
                if (anc_monitor_transmitter.is_enable) {
                    if (g_ANC_monitor_get_ANC_type != g_ANC_monitor_get_ANC_type_pre) { //anc_change type
                        TRANSMITTER_LOG_I("[ANC_Monitor]anc change type, %d -> %d", 2, g_ANC_monitor_get_ANC_type_pre, g_ANC_monitor_get_ANC_type);
                        g_ANC_monitor_get_ANC_type_pre = g_ANC_monitor_get_ANC_type;
                        //Reload Nvkey
                        //audio_anc_monitor_update_nvkey_id_by_anc_type(g_ANC_monitor_get_ANC_type);
                        audio_anc_monitor_transmitter_set_parameters();
                    }
                }
                start_stop_fail_reason = 5; //5: others, already enabled or unexpected event_id
            }
        } else {
            start_stop_fail_reason = 4; //4: anc_control_result error
        }
#endif

#if defined(ADAPIVE_ANC_STREAM_CONTROL) && !defined(AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE)
    }
#endif

    if (start_stop_fail_reason != 0) {
        TRANSMITTER_LOG_E("[ANC_Monitor]audio_anc_monitor_callback START/STOP FAIL, start_stop_fail_reason %d", 1, start_stop_fail_reason);
    }
    return;
}

void audio_anc_monitor_set_info(audio_anc_monitor_set_info_t set_type, uint32_t para)
{
    //send to AM
    bt_sink_srv_am_feature_t feature_para;
    memset(&feature_para, 0, sizeof(bt_sink_srv_am_feature_t));
    feature_para.type_mask = AM_ANC_MONITOR;
    feature_para.feature_param.anc_monitor_param.event = AUDIO_ANC_MONITOR_SET_INFO;
    feature_para.feature_param.anc_monitor_param.event_type = set_type;
    feature_para.feature_param.anc_monitor_param.param = para;
    am_audio_set_feature(FEATURE_NO_NEED_ID, &feature_para);
    TRANSMITTER_LOG_I("[ANC_Monitor]Set info, type:%d, para:%d", 2, set_type, para);
}

uint32_t audio_anc_monitor_get_info(audio_anc_monitor_get_info_t get_type)
{
    //send to AM
    uint32_t share_addr = (uint32_t)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_ANC_MONITOR_ADAPTIVE_ANC);
    bt_sink_srv_am_feature_t feature_para;
    memset(&feature_para, 0, sizeof(bt_sink_srv_am_feature_t));
    feature_para.type_mask = AM_ANC_MONITOR;
    feature_para.feature_param.anc_monitor_param.event = AUDIO_ANC_MONITOR_GET_INFO;
    feature_para.feature_param.anc_monitor_param.event_type = get_type;
    feature_para.feature_param.anc_monitor_param.param = share_addr; //addr
    am_audio_set_feature(FEATURE_NO_NEED_ID, &feature_para);
    TRANSMITTER_LOG_I("[ANC_Monitor]Get info, type:%d, share_addr 0x%x, start busy wating", 2, get_type, share_addr);

    return false;
}

#ifdef ADAPIVE_ANC_STREAM_CONTROL
void audio_anc_monitor_adaptive_anc_stream_get_status(uint8_t *enable)
{
    if (enable != NULL) {
        *enable = anc_monitor_transmitter.is_enable;
    }
}

//void audio_anc_monitor_adaptive_anc_stream_suspend_resume(bool enable)
void audio_anc_monitor_adaptive_anc_stream_enable(bool enable)
{
    uint8_t anc_enable;
    audio_anc_control_result_t anc_ret = AUDIO_ANC_CONTROL_EXECUTION_NONE;
    audio_anc_control_get_status(&anc_enable, NULL, NULL, NULL, NULL, NULL);
    TRANSMITTER_LOG_I("[ANC_Monitor]adaptive_anc_stream_enable, enable %d, anc_enable %d", 2, enable, anc_enable);
    if (enable) { //enable Adaptive ANC Stream
        if (anc_enable) {
            /* if ANC is ON, enable Adaptive ANC. */
            audio_anc_monitor_set_info(AUDIO_ANC_MONITOR_STREAM_CONTROL, TRUE);
            audio_anc_monitor_anc_callback(AUDIO_ANC_CONTROL_EVENT_ON, AUDIO_ANC_CONTROL_CB_LEVEL_ALL);
        }
    } else { //disable Adaptive ANC Stream
        if (anc_enable) {
            /* if ANC is ON, disable Adaptive ANC. */
            #ifdef AIR_ANC_USER_UNAWARE_ENABLE
            audio_anc_monitor_set_info(AUDIO_ANC_MONITOR_SET_USER_UNAWARE_ENABLE, FALSE);
            hal_gpt_delay_ms(1500); //delay 1.5s for UU ramping to tight
            #endif
            audio_anc_monitor_set_info(AUDIO_ANC_MONITOR_STREAM_CONTROL, FALSE);
            audio_anc_monitor_anc_callback(AUDIO_ANC_CONTROL_EVENT_OFF, AUDIO_ANC_CONTROL_CB_LEVEL_ALL);

            /* disable Adaptive ANC Stream , reset extend gain. */
            audio_anc_control_extend_ramp_cap_t init_ramp_cap;
            init_ramp_cap.extend_gain_1 = 0;
            init_ramp_cap.extend_gain_2 = 0;
            init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_WIND_NOISE;
            anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);
            init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_USER_UNAWARE;
            anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);
            init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_ENVIRONMENT_DETECTION;
            anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);
        }
        TRANSMITTER_LOG_I("[ANC_Monitor]adaptive_anc_stream_enable, disable done, reset ANC extend gain ret %d", 1, anc_ret);
    }
}
#endif

#if defined(AIR_ANC_USER_UNAWARE_ENABLE) || defined(AIR_ANC_WIND_DETECTION_ENABLE) || defined(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE)
void audio_anc_monitor_init_adaptive_anc_status(void)
{
    uint8_t enable;
    uint8_t suspend;

    //WND
#if defined(AIR_ANC_WIND_DETECTION_ENABLE) && !defined(AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE)
    audio_anc_monitor_adaptive_anc_get_status_from_nvdm(AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT, &enable, &suspend);
    g_adaptive_anc_status.wnd_enable = enable;
    g_adaptive_anc_status.wnd_suspend = suspend;
#else
    g_adaptive_anc_status.wnd_enable = false;
    g_adaptive_anc_status.wnd_suspend = false;
#endif

    //UU
#if defined(AIR_ANC_USER_UNAWARE_ENABLE) && !defined(AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE)
    audio_anc_monitor_adaptive_anc_get_status_from_nvdm(AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE, &enable, &suspend);
    g_adaptive_anc_status.uu_enable = enable;
    g_adaptive_anc_status.uu_suspend = suspend;
#else
    g_adaptive_anc_status.uu_enable = false;
    g_adaptive_anc_status.uu_suspend = false;
#endif

    //ED
#if defined(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE) && !defined(AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE)
    audio_anc_monitor_adaptive_anc_get_status_from_nvdm(AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT, &enable, &suspend);
    g_adaptive_anc_status.ed_enable = enable;
    g_adaptive_anc_status.ed_suspend = suspend;
#else
    g_adaptive_anc_status.ed_enable = false;
    g_adaptive_anc_status.ed_suspend = false;
#endif
}

void audio_anc_monitor_adaptive_anc_save_suspend_status_to_nvdm(audio_anc_monitor_feature_type_t feature, bool suspend)
{
    uint32_t nvkey_length = 0;
    void *malloc_ptr = NULL;
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
    wind_detection_para_t *wnd_ptr;
#endif
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    user_unaware_para_t *uu_ptr;
#endif
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
    DSP_PARA_ENVIRONMENT_DETECTION_STRU *ed_ptr;
#endif
    uint16_t nvkey_id;
    sysram_status_t nvdm_status;
    //TRANSMITTER_LOG_I("[ANC_Monitor]save_suspend_procedure_status, feature %d, suspend %d", 2, feature, suspend);
    switch (feature) {
        case AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT:
            g_adaptive_anc_status.wnd_suspend = suspend;
            nvkey_id = g_anc_monitor_wind_nvkey_id_by_anc_type;
            break;
        case AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE:
            g_adaptive_anc_status.uu_suspend = suspend;
            nvkey_id = NVID_DSP_ALG_ANC_USR_UNAWARE;
            break;
        case AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT:
            g_adaptive_anc_status.ed_suspend = suspend;
            nvkey_id = g_anc_monitor_ed_nvkey_id_by_anc_type;
            break;
        default:
            return;
            break;
    }

    nvdm_status =  flash_memory_query_nvdm_data_length(nvkey_id, &nvkey_length);

    malloc_ptr = pvPortMallocNC(nvkey_length);
    if (malloc_ptr) {
        nvdm_status = flash_memory_read_nvdm_data(nvkey_id, (uint8_t *)malloc_ptr, &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            TRANSMITTER_LOG_E("[ANC_Monitor]save_suspend_status, Read Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
            vPortFreeNC(malloc_ptr);
            return;
        }

        //malloc_ptr->pause_attenuation = (uint16_t)suspend;
        switch (feature) {
            case AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT:
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
                wnd_ptr = (wind_detection_para_t *)malloc_ptr;
                wnd_ptr->pause_attenuation = (uint16_t)suspend;
#endif
                break;
            case AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE:
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
                uu_ptr = (user_unaware_para_t *)malloc_ptr;
                uu_ptr->pause_attenuation = (uint16_t)suspend;
#endif
                break;
            case AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT:
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
                ed_ptr = (DSP_PARA_ENVIRONMENT_DETECTION_STRU *)malloc_ptr;
                ed_ptr->pause_attenuation = (uint16_t)suspend;
#endif
                break;
            default:
                break;
        }

        nvdm_status = flash_memory_write_nvdm_data(nvkey_id, (uint8_t *)malloc_ptr, nvkey_length);
        TRANSMITTER_LOG_I("[ANC_Monitor]save_suspend_status, feature: %d, nvkey_id:0x%x, suspend:%d, Write Nvkey data status:%d", 4, feature, nvkey_id, suspend, nvdm_status);
        vPortFreeNC(malloc_ptr);
    } else {
        configASSERT(0 && "[ANC_Monitor]save_suspend_status, malloc Fail");
    }
}

void audio_anc_monitor_adaptive_anc_save_enable_status_to_nvdm(audio_anc_monitor_feature_type_t feature, bool enable)
{
    uint32_t nvkey_length = 0;
    void *malloc_ptr = NULL;
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
    wind_detection_para_t *wnd_ptr;
#endif
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    user_unaware_para_t *uu_ptr;
#endif
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
    DSP_PARA_ENVIRONMENT_DETECTION_STRU *ed_ptr;
#endif

    uint16_t nvkey_id;
    sysram_status_t nvdm_status;

    switch (feature) {
        case AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT:
            g_adaptive_anc_status.wnd_enable = enable;
            nvkey_id = g_anc_monitor_wind_nvkey_id_by_anc_type;
            break;
        case AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE:
            g_adaptive_anc_status.uu_enable = enable;
            nvkey_id = NVID_DSP_ALG_ANC_USR_UNAWARE;
            break;
        case AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT:
            g_adaptive_anc_status.ed_enable = enable;
            nvkey_id = g_anc_monitor_ed_nvkey_id_by_anc_type;
            break;
        default:
            return;
            break;
    }

    nvdm_status =  flash_memory_query_nvdm_data_length(nvkey_id, &nvkey_length);

    malloc_ptr = pvPortMallocNC(nvkey_length);
    if (malloc_ptr) {
        nvdm_status = flash_memory_read_nvdm_data(nvkey_id, (uint8_t *)malloc_ptr, &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            TRANSMITTER_LOG_E("[ANC_Monitor]save_enable_status, Read Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
            vPortFreeNC(malloc_ptr);
            return;
        }

        //malloc_ptr->ENABLE = (uint8_t)enable;
        switch (feature) {
            case AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT:
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
                wnd_ptr = (wind_detection_para_t *)malloc_ptr;
                wnd_ptr->ENABLE = (uint8_t)enable;
#endif
                break;
            case AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE:
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
                uu_ptr = (user_unaware_para_t *)malloc_ptr;
                uu_ptr->ENABLE = (uint8_t)enable;
#endif
                break;
            case AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT:
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
                ed_ptr = (DSP_PARA_ENVIRONMENT_DETECTION_STRU *)malloc_ptr;
                ed_ptr->enable = (uint8_t)enable;
#endif
                break;
            default:
                break;
        }

        nvdm_status = flash_memory_write_nvdm_data(nvkey_id, (uint8_t *)malloc_ptr, nvkey_length);
        TRANSMITTER_LOG_I("[ANC_Monitor]save_enable_status, feature: %d, nvkey_id:0x%x, enable:%d, Write Nvkey data Fail status:%d", 4, feature, nvkey_id, enable, nvdm_status);
        vPortFreeNC(malloc_ptr);
    } else {
        configASSERT(0 && "[ANC_Monitor]save_enable_status, malloc Fail");
    }
}

void audio_anc_monitor_adaptive_anc_get_status_from_nvdm(audio_anc_monitor_feature_type_t feature, uint8_t *enable, uint8_t *suspend)
{
    uint32_t nvkey_length = 0;
    void *malloc_ptr = NULL;
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
    wind_detection_para_t *wnd_ptr;
#endif
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    user_unaware_para_t *uu_ptr;
#endif
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
    DSP_PARA_ENVIRONMENT_DETECTION_STRU *ed_ptr;
#endif

    uint16_t nvkey_id;
    sysram_status_t nvdm_status;

    //init to false
    if (enable != NULL) {
        *enable = false;
    }
    if (suspend != NULL) {
        *suspend = false;
    }

    switch (feature) {
        case AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT:
            nvkey_id = g_anc_monitor_wind_nvkey_id_by_anc_type;
            break;
        case AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE:
            nvkey_id = NVID_DSP_ALG_ANC_USR_UNAWARE;
            break;
        case AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT:
            nvkey_id = g_anc_monitor_ed_nvkey_id_by_anc_type;
            break;
        default:
            return;
            break;
    }

    nvdm_status =  flash_memory_query_nvdm_data_length(nvkey_id, &nvkey_length);

    malloc_ptr = pvPortMallocNC(nvkey_length);
    if (malloc_ptr) {
        nvdm_status = flash_memory_read_nvdm_data(nvkey_id, (uint8_t *)malloc_ptr, &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            TRANSMITTER_LOG_E("[ANC_Monitor]get_status_from_nvdm, Read Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
            vPortFreeNC(malloc_ptr);
            return;
        } else {
            if (enable != NULL) {
                //*enable = malloc_ptr->ENABLE;
                switch (feature) {
                    case AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT:
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
                        wnd_ptr = (wind_detection_para_t *)malloc_ptr;
                        *enable = wnd_ptr->ENABLE;
#endif
                        break;
                    case AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE:
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
                        uu_ptr = (user_unaware_para_t *)malloc_ptr;
                        *enable = uu_ptr->ENABLE;
#endif
                        break;
                    case AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT:
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
                        ed_ptr = (DSP_PARA_ENVIRONMENT_DETECTION_STRU *)malloc_ptr;
                        *enable = ed_ptr->enable;
#endif
                        break;
                    default:
                        break;
                }
            }
            if (suspend != NULL) {
                //*suspend = malloc_ptr->pause_attenuation;
                switch (feature) {
                    case AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT:
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
                        wnd_ptr = (wind_detection_para_t *)malloc_ptr;
                        *suspend = wnd_ptr->pause_attenuation;
#endif
                        break;
                    case AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE:
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
                        uu_ptr = (user_unaware_para_t *)malloc_ptr;
                        *suspend = uu_ptr->pause_attenuation;
#endif
                        break;
                    case AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT:
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
                        ed_ptr = (DSP_PARA_ENVIRONMENT_DETECTION_STRU *)malloc_ptr;
                        *suspend = ed_ptr->pause_attenuation;
#endif
                        break;
                    default:
                        break;
                }
            }
            TRANSMITTER_LOG_I("[ANC_Monitor]get_status_from_nvdm, feature: %d, nvkey_id:0x%x", 2, feature, nvkey_id);
        }
        vPortFreeNC(malloc_ptr);
    } else {
        configASSERT(0 && "[ANC_Monitor]get_status_from_nvdm, malloc Fail");
    }
}
#endif

#ifdef AIR_ANC_WIND_DETECTION_ENABLE
void audio_wind_detection_enable(bool enable)
{
    audio_anc_monitor_adaptive_anc_save_enable_status_to_nvdm(AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT, enable);
    audio_anc_monitor_transmitter_print_status();
    audio_anc_monitor_set_info(AUDIO_ANC_MONITOR_SET_WIND_DETECT_ENABLE, (uint32_t)enable);
    if(!enable) {
        audio_anc_control_result_t anc_ret = AUDIO_ANC_CONTROL_EXECUTION_NONE;
        audio_anc_control_extend_ramp_cap_t init_ramp_cap;
        init_ramp_cap.extend_gain_1 = 0;
        init_ramp_cap.extend_gain_2 = 0;
        init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_WIND_NOISE;
        anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);
        if(anc_ret != 0) {
            TRANSMITTER_LOG_W("[ANC_Monitor][Wind] disable reset extend gain fail %d", 1, anc_ret);
        }
    }
}

void audio_wind_detection_suspend_procedure(bool suspend)
{
    audio_anc_monitor_adaptive_anc_save_suspend_status_to_nvdm(AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT, suspend);
    audio_anc_monitor_transmitter_print_status();
    audio_anc_monitor_set_info(AUDIO_ANC_MONITOR_SET_WIND_DETECT_SUSPEND, (uint32_t)suspend);
}

#endif

#ifdef AIR_ANC_USER_UNAWARE_ENABLE
void audio_user_unaware_save_enable_status_to_nvdm(bool enable)
{
    //for app layer calling
    audio_anc_monitor_adaptive_anc_save_enable_status_to_nvdm(AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE, enable);
}

void audio_user_unaware_suspend_procedure(bool suspend)
{
    audio_anc_monitor_adaptive_anc_save_suspend_status_to_nvdm(AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE, suspend);
    audio_anc_monitor_transmitter_print_status();

    //audio_anc_monitor_set_info(AUDIO_ANC_MONITOR_SET_USER_UNAWARE_SUSPEND, (uint32_t)suspend); //Need?
}

void audio_user_unaware_enable(bool enable)
{
    //save status at app layer to seperate user-willing and feature-excluding.
    audio_anc_monitor_transmitter_print_status();
    audio_anc_monitor_set_info(AUDIO_ANC_MONITOR_SET_USER_UNAWARE_ENABLE, (uint32_t)enable);
}

U8 audio_user_unaware_get_enable_state(void)
{
#if 0
    U32 *enable = pvPortMallocNC(sizeof(U32));
    U8 res;
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AUDIO_ANC_ADAPTIVE_GET,
                                          AUDIO_USER_UNAWARE_GET_INFO_TYPE_ENABLE_STAT,
                                          hal_memview_cm4_to_infrasys((uint32_t)enable),
                                          true);
    res = (U8)(*enable);
    vPortFreeNC(enable);
#else
    return (U8)audio_anc_monitor_get_info(AUDIO_ANC_MONITOR_GET_USER_UNAWARE_ENABLE_STAT);
#endif
}

void audio_user_unaware_get_gain_info(uint8_t *info_ptr)
{
    if (xSemaphoreTake(g_adaptive_ANC_semaphore, portMAX_DELAY) == pdPASS) { //take semaphore
        uint16_t i;
        user_unaware_info *gain_ptr = (user_unaware_info *)info_ptr;
        uint32_t share_addr = (uint32_t)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_ANC_MONITOR_ADAPTIVE_ANC);
        memset(gain_ptr, 0, sizeof(user_unaware_info));

        g_adaptive_ANC_getinfo_DSP_write_done = false;
        audio_anc_monitor_get_info(AUDIO_ANC_MONITOR_GET_USER_UNAWARE_GAIN_INFO); //send to AM

        for (i = 0; ; i++) {
            if (g_adaptive_ANC_getinfo_DSP_write_done) {
                memcpy(gain_ptr, (user_unaware_info *)share_addr, sizeof(user_unaware_info));
                xSemaphoreGive(g_adaptive_ANC_semaphore);//give semaphore
                TRANSMITTER_LOG_I("[User Unaware]Get gain info, %d, %d, %d, %d, %d, %dms", 6, gain_ptr->log0[0], gain_ptr->log0[1], gain_ptr->log2, gain_ptr->log3, gain_ptr->log4, i * 2);
                break;
            }
            if ((i % 500) == 0) {
                if (i == 500) {
                    configASSERT(0 && "[User Unaware]audio_user_unaware_get_gain_info 1s timeout");
                }
            }
            hal_gpt_delay_ms(2);
        }
    }
}
#endif

#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
#if 0
void audio_environment_detection_get_suspend_procedure_status_from_nvdm(uint8_t *enable)
{
    uint32_t nvkey_length = 0;
    DSP_PARA_ENVIRONMENT_DETECTION_STRU *malloc_ptr = NULL;
    uint16_t nvkey_id;
    sysram_status_t nvdm_status;

    nvkey_id = NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION;
    nvdm_status =  flash_memory_query_nvdm_data_length(nvkey_id, &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        TRANSMITTER_LOG_E("[ANC_Monitor][Adapt_ANC][env_detect]Query Nvkey length Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
    }
    malloc_ptr = pvPortMallocNC(nvkey_length);
    if (malloc_ptr) {
        nvdm_status = flash_memory_read_nvdm_data(nvkey_id, (uint8_t *)malloc_ptr, &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            TRANSMITTER_LOG_E("[ANC_Monitor][Adapt_ANC][env_detect]Read Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
        }

        //malloc_ptr->pause_attenuation = (uint16_t)suspend;

        if (nvdm_status != NVDM_STATUS_NAT_OK) {
            TRANSMITTER_LOG_E("[ANC_Monitor][Adapt_ANC][env_detect]read_nvdm_data err, ID 0x%x, status %d", 2, nvkey_id, nvdm_status);
            if (enable != NULL) {
                *enable = false;
            }
        } else {
            if (enable != NULL) {
                if (malloc_ptr->pause_attenuation) { //true for suspend ED
                    *enable = false;
                } else {
                    *enable = true;
                }
            }
            TRANSMITTER_LOG_I("[ANC_Monitor][Adapt_ANC][env_detect]ed_get_enable_status_from_nvdm, enable %d", 1, *enable);
        }
        vPortFreeNC(malloc_ptr);
    } else {
        TRANSMITTER_LOG_E("[ANC_Monitor][Adapt_ANC][env_detect] malloc Fail", 0);
        assert(0);
    }
}
void audio_environment_detection_store_suspend_procedure(bool suspend)
{
    uint32_t nvkey_length = 0;
    DSP_PARA_ENVIRONMENT_DETECTION_STRU *malloc_ptr = NULL;
    uint16_t nvkey_id;
    sysram_status_t nvdm_status;

    nvkey_id = NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION;
    nvdm_status =  flash_memory_query_nvdm_data_length(nvkey_id, &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        TRANSMITTER_LOG_E("[ANC_Monitor][env_detect]Store Nvkey length Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
    }
    malloc_ptr = pvPortMallocNC(nvkey_length);
    if (malloc_ptr) {
        nvdm_status = flash_memory_read_nvdm_data(nvkey_id, (uint8_t *)malloc_ptr, &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            TRANSMITTER_LOG_E("[ANC_Monitor][env_detect]Store Read Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
        }

        malloc_ptr->pause_attenuation = (uint16_t)suspend;

        nvdm_status = flash_memory_write_nvdm_data(nvkey_id, (uint8_t *)malloc_ptr, nvkey_length);
        if (nvdm_status) {
            TRANSMITTER_LOG_E("[ANC_Monitor][env_detect]Store Write Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
        }
        vPortFreeNC(malloc_ptr);
    } else {
        TRANSMITTER_LOG_E("[ANC_Monitor][env_detect] malloc Fail", 0);
        AUDIO_ASSERT(0);
    }
}
#endif


void audio_environment_detection_suspend_procedure(bool suspend)
{
    audio_anc_monitor_adaptive_anc_save_suspend_status_to_nvdm(AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT, suspend);
    audio_anc_monitor_transmitter_print_status();
    audio_anc_monitor_set_info(AUDIO_ANC_MONITOR_SET_ENVIRONMENT_DETECTION_SUSPEND, (uint32_t)suspend);
}

int16_t audio_environment_detection_get_stationary_noise(void)
{
    //ED get info
    uint16_t i;
    int16_t info = 0;
    uint32_t share_addr = (uint32_t)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_ANC_MONITOR_ADAPTIVE_ANC);
    if (xSemaphoreTake(g_adaptive_ANC_semaphore, portMAX_DELAY) == pdPASS) { //take semaphore
        g_adaptive_ANC_getinfo_DSP_write_done = false;
        audio_anc_monitor_get_info(AUDIO_ANC_MONITOR_GET_ENVIRONMENT_DETECTION_STATIONARY_NOISE); //send to AM

        for (i = 0; ; i++) {
            if (g_adaptive_ANC_getinfo_DSP_write_done) {
                memcpy(&info, (int16_t *)share_addr, sizeof(info));
                xSemaphoreGive(g_adaptive_ANC_semaphore);//give semaphore
                TRANSMITTER_LOG_I("[ANC_Monitor][env_detect]Get info, %d, %dms", 2, info, i * 2);
                break;
            }
            if ((i % 1000) == 0) {
                if (i == 1000) {
#ifdef AIR_ICE_DEBUG_ENABLE
                    if (hal_ice_debug_is_enabled() == false) {
                        assert(0);
                    } else {
                        break;
                    }
#else
                    configASSERT(0 && "[ANC_Monitor][env_detect]audio_environment_detection_get_stationary_noise 1s timeout");
#endif
                }
            }
            hal_gpt_delay_ms(2);
        }
    }
    return info;
}

void audio_environment_detection_enable(bool enable)
{
    audio_anc_monitor_adaptive_anc_save_enable_status_to_nvdm(AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT, enable);
    audio_anc_monitor_transmitter_print_status();
    audio_anc_monitor_set_info(AUDIO_ANC_MONITOR_SET_ENVIRONMENT_DETECTION_ENABLE, (uint32_t)enable);
    if(!enable) {
        audio_anc_control_result_t anc_ret = AUDIO_ANC_CONTROL_EXECUTION_NONE;
        audio_anc_control_extend_ramp_cap_t init_ramp_cap;
        init_ramp_cap.extend_gain_1 = 0;
        init_ramp_cap.extend_gain_2 = 0;
        init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_ENVIRONMENT_DETECTION;
        anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);
        if(anc_ret != 0) {
            TRANSMITTER_LOG_W("[ANC_Monitor][env_detect] disable reset extend gain fail %d", 1, anc_ret);
        }
    }
}

#endif

#if 0
void audio_dsp_adaptive_message_callback(hal_audio_event_t event, void *user_data)
{
    bool is_sync_control = false;
    audio_anc_control_result_t anc_control_result = 0;
    audio_anc_control_extend_ramp_cap_t anc_extend_ramp_cap;

    anc_extend_ramp_cap.gain_type = ((uint8_t)event);
    anc_extend_ramp_cap.extend_gain_1 = (*(uint32_t *)user_data) & 0xFFFF;
    anc_extend_ramp_cap.extend_gain_2 = ((*(uint32_t *)user_data) >> 16) & 0xFFFF;
    if (anc_extend_ramp_cap.gain_type ==  AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_WIND_NOISE
        || anc_extend_ramp_cap.gain_type == AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_USER_UNAWARE
        || anc_extend_ramp_cap.gain_type == AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_ENVIRONMENT_DETECTION) {
        is_sync_control = true;
    } else if (anc_extend_ramp_cap.gain_type ==  AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_HOWLING_CONTROL) {
        is_sync_control = false;
    }
    if (anc_extend_ramp_cap.gain_type == AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_ENVIRONMENT_DETECTION) {
        uint8_t noise_level;
        noise_level = (uint8_t)((anc_monitor_temp >> 8) & 0xFF);
        TRANSMITTER_LOG_I(" gain_dsp_callback, Noise Gate Level %d @@@@", noise_level);
    }

    TRANSMITTER_LOG_E(" gain_dsp_callback, type=%d, is_sync=%d gain1=%d gain2=%d",
                      4, anc_extend_ramp_cap.gain_type, is_sync_control,
                      anc_extend_ramp_cap.extend_gain_1, anc_extend_ramp_cap.extend_gain_2);
#if 0
    if (is_sync_control) {
        /* Notify APP. */
        app_anc_extend_gain_handle_callback(anc_extend_ramp_cap.gain_type, anc_extend_ramp_cap.extend_gain_1, anc_extend_ramp_cap.extend_gain_2);
    } else {
        /* ANC control. */
        anc_control_result = audio_anc_control_set_extend_gain(anc_extend_ramp_cap.gain_type, &anc_extend_ramp_cap, NULL);
        APPS_LOG_MSGID_I(LOG_TAG" gain_dsp_callback, HOWLING_CONTROL result %d", 1, anc_control_result);
    }
#endif
}
#endif

#endif




