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

/* Includes ------------------------------------------------------------------*/
#include "dsp_feature_interface.h"
#include "dsp_audio_process.h"
#include "dsp_dump.h"
#include "memory_attribute.h"
#include "preloader_pisplit.h"
#include "memory_attribute.h"
#include "dsp_memory.h"
#include "dsp_scenario.h"

#include "wind_interface.h"
#include "dsp_rom_table.h"

#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
#include "full_adapt_anc_api.h"
#endif

/* Global variables ---------------------------------------------------------*/
#ifdef AIR_ANC_SCENARIO_CONTROL_GAIN_ENABLE
extern bool g_anc_ramp_by_scenario_set_wnd;
extern bool g_anc_ramp_by_scenario;
bool g_anc_wind_suspend_by_scenario = false;
#endif

extern uint8_t dsp_stream_algo_nvkey_loading_status;

/* Private define ------------------------------------------------------------*/
#define WIND_VALID_MEMORY_CHECK_VALUE   ((U32)0x00580058)
#define DSP_WIND_INSTANCE_MEMSIZE       (sizeof(WIND_INSTANCE))
#define WIND_DETECTION_PROCESS_FRAME    (240)//Unit: sample


/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
WIND_INSTANCE_PTR wind_instance;
void *wind_instance_nvkey_ptr = NULL;

WIND_INSTANCE_R_PTR wind_instance_r;


/* Public variables ----------------------------------------------------------*/
int16_t wind_nvkey_attenuation_gain = 0;
//bool wind_instance_nvkey_loading = false;    // This flag is used to protect race condition issue between anc_monitor task (pri:3) and non-realtime rx task (pri:2).

/* Private functions ---------------------------------------------------------*/
BOOL wind_check_memory(VOID)
{
    if (NULL != wind_instance) {
        if (WIND_VALID_MEMORY_CHECK_VALUE == wind_instance->memory_check) {
            return TRUE;
        }
    }
    return FALSE;
}

bool stream_function_wind_detection_reset_init(void)
{
    if (wind_check_memory()) {
        wind_instance->init_done = false;
    }
    return false;
}

bool stream_function_wind_detection_enable(bool enable)
{
    if (wind_check_memory()) {
        wind_instance->nvkey.ENABLE = enable;
        stream_function_wind_reset_status();
        stream_function_wind_detection_reset_init();
    }
    DSP_MW_LOG_I("[Wind Detection] Enable : %d", 1, enable);
    return false;
}

bool stream_function_wind_detection_pause_attenuation(bool is_pause)
{
    if (wind_check_memory()) {
        wind_instance->nvkey.pause_attenuation = is_pause;
        stream_function_wind_reset_status();
    }
    DSP_MW_LOG_I("[Wind Detection] Pause Procedure : %d", 1, is_pause);
    return false;
}

void stream_function_wind_detection_set_value(audio_anc_monitor_set_info_t set_type, uint32_t data)
{
    switch(set_type){
        case AUDIO_ANC_MONITOR_SET_WIND_DETECT_ENABLE:
            stream_function_wind_detection_enable((bool)data);
            break;
        case AUDIO_ANC_MONITOR_SET_WIND_DETECT_SUSPEND:
            stream_function_wind_detection_pause_attenuation((bool)data);
            break;
        default:
            DSP_MW_LOG_W("[Wind Detection] Unsupported Set type : %d", 1, set_type);
            break;
    }
}

bool wind_push_data(wind_buf_ctrl* buf_ctrl, int16_t *data, uint32_t size_of_sample)
{
    if ((size_of_sample + buf_ctrl->buffer_cnt) <= DSP_WIND_DETECTION_BUFFERINF_SIZE) {
        memcpy(&buf_ctrl->buffer[buf_ctrl->buffer_cnt], data, sizeof(int16_t)*size_of_sample);
        buf_ctrl->buffer_cnt += size_of_sample;
    } else {
        AUDIO_ASSERT(0);
    }
    return false;
}

bool wind_pop_data(wind_buf_ctrl* buf_ctrl, uint32_t size_of_sample)
{
    if (buf_ctrl->buffer_cnt >= size_of_sample) {
        memcpy(&buf_ctrl->buffer[0], &buf_ctrl->buffer[size_of_sample], sizeof(int16_t) * (buf_ctrl->buffer_cnt - size_of_sample));
        buf_ctrl->buffer_cnt -= size_of_sample;
    } else {
        AUDIO_ASSERT(0);
    }
    return false;
}

uint32_t wind_get_data_cnt(void)
{
    return wind_instance->buf_ctrl.buffer_cnt;
}

int16_t *wind_get_read_buffer_ptr(wind_buf_ctrl* buf_ctrl)
{
    return &buf_ctrl->buffer[0]; //&wind_instance->buf_ctrl.buffer[0];
}

int16_t wind_comprehensive_detect_result(int16_t detect_result1, int16_t detect_result2)
{
    if ((detect_result1 == 1)||(detect_result2 == 1)) {
        return 1; //windy
    } else {
        return 0; //silent
    }
}

int16_t wind_get_wind_status(void)
{
    if (wind_check_memory()) {
        return wind_instance->detect_wind;
    } else {
        DSP_MW_LOG_W("[Wind Detection] get_wind_status, wind_detection is not running.", 0);
        return 0;
    }
}

#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
extern int16_t g_FADP_detect_wind;
#endif

bool wind_change_status(int16_t detect_result)
{
    uint32_t interrupt_mask;
    audio_extend_gain_control_t gain_control;
    int16_t attenuation = 0;
    hal_nvic_save_and_set_interrupt_mask(&interrupt_mask);
    wind_instance->detect_wind = detect_result;
    wind_instance->consecutive_times = 0;
    if (wind_instance->detect_wind) {
        attenuation = wind_instance->nvkey.attenuation;
        if (wind_instance->nvkey.pause_attenuation) { //Suspend attenuation, Keep at disabled status.
            attenuation = 0;
        }
        #ifdef AIR_ANC_SCENARIO_CONTROL_GAIN_ENABLE
        if (g_anc_wind_suspend_by_scenario) { //Suspend attenuation, Keep at disabled status.
            attenuation = 0;
        }
        #endif
    }
    hal_nvic_restore_interrupt_mask(interrupt_mask);

#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    extern int16_t g_UU_detect_wind;
    g_UU_detect_wind = wind_instance->detect_wind;
#endif
#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
    // Store WND attenuation
    g_FADP_detect_wind = wind_instance->detect_wind;
#endif

    //Call CCNI
    gain_control.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_WIND_NOISE;
    gain_control.misc = 0;
    gain_control.gain[0] = attenuation;
    gain_control.gain[1] = 0;
    dsp_ccni_set_extend_gain(&gain_control);
    DSP_MW_LOG_I("[Wind Detection] Change Detect Result : %d, pause_attenuation: %d ", 2, wind_instance->detect_wind, wind_instance->nvkey.pause_attenuation);
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    dsp_adapt_anc_send_unstable_status(AUDIO_ADAPTIVE_ANC_CLK_CTRL_TYPE_WD, wind_instance->detect_wind);
#endif
    return false;
}


/* Public functions ----------------------------------------------------------*/
bool stream_function_wind_load_nvkey(void *nvkey)
{
    uint32_t ret;
    dsp_stream_algo_nvkey_loading_status |= WND_NVKEY_LOADING;
    if (wind_check_memory()) {
        memcpy(&wind_instance->nvkey, nvkey, sizeof(DSP_PARA_WIND_STRU));
        wind_instance->nvkey_ready = true;

        wind_nvkey_attenuation_gain = wind_instance->nvkey.attenuation;

        if (wind_instance_nvkey_ptr) {
            vPortFree(wind_instance_nvkey_ptr);
            DSP_MW_LOG_I("[Wind Detection] memory heap free", 0);
        }
        wind_instance_nvkey_ptr = NULL;
        stream_function_wind_detection_reset_init(); //reset for updating nvkey param
        ret = true;
    } else {
        if (!wind_instance_nvkey_ptr) {
            wind_instance_nvkey_ptr = pvPortMalloc(sizeof(DSP_PARA_WIND_STRU));
            DSP_MW_LOG_I("[Wind Detection] memory heap allocate 0x%x ", 1, wind_instance_nvkey_ptr);
        }

        if (wind_instance_nvkey_ptr) {
            memcpy(wind_instance_nvkey_ptr, nvkey, sizeof(DSP_PARA_WIND_STRU));
        }
        ret = false;
    }
    dsp_stream_algo_nvkey_loading_status &= ~(WND_NVKEY_LOADING);

    return ret;
}

bool stream_function_wind_get_status(void)
{
    if (wind_check_memory()) {
        return (bool) wind_instance->detect_wind;
    }
    return false;
}

bool stream_function_wind_reset_status(void)
{
    if (wind_check_memory()) {
        wind_instance->consecutive_times = 0;
        wind_change_status(false);
    }
    return false;
}

int16_t stream_function_wind_check_headset(void)
{
#ifdef AIR_DSP_PRODUCT_CATEGORY_HEADSET
    return 1;
#else
    return 0;
#endif
}

#ifdef AIR_BTA_IC_PREMIUM_G3
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ bool stream_function_wind_initialize(void *para)
#else
ATTR_TEXT_IN_IRAM_LEVEL_2 bool stream_function_wind_initialize(void *para)
#endif
{
    uint32_t scratch_memory_size, need_size;

    if (!wind_check_memory()) {
#ifdef BASE_STEREO_HIGH_G3_TYPE_77
        wind_set_rom_start(ROM_TABLE_ADDR_WIND);
#endif
        scratch_memory_size = WindDet_get_memsize();
        if (stream_function_wind_check_headset()) {
            need_size = DSP_WIND_INSTANCE_MEMSIZE + sizeof(WIND_INSTANCE_R) + scratch_memory_size*2;
        } else {
            need_size = DSP_WIND_INSTANCE_MEMSIZE + scratch_memory_size;
        }

        if (need_size > stream_function_get_working_buffer_length(para)) {
            DSP_MW_LOG_E("[Wind Detection]MEM SIZE is insufficient require:%d, allocated:%d, FW(L):%d, Lib:%d", 4, need_size, stream_function_get_working_buffer_length(para), DSP_WIND_INSTANCE_MEMSIZE, scratch_memory_size);
            assert(false);
        }
        wind_instance = (WIND_INSTANCE_PTR)stream_function_get_working_buffer(para);
        DSP_MW_LOG_I("[Wind Detection]MEM SIZE require:%d, allocated:%d", 2, need_size, stream_function_get_working_buffer_length(para));

        if (stream_function_wind_check_headset()) {
            wind_instance_r = (WIND_INSTANCE_R_PTR)((uint32_t*)wind_instance + WIND_DETECTION_MEMSIZE_L/sizeof(uint32_t));
            DSP_MW_LOG_W("[Wind Detection] L need %d at 0x%x, R need %d at 0x%x", 4, DSP_WIND_INSTANCE_MEMSIZE + scratch_memory_size, wind_instance, sizeof(WIND_INSTANCE_R) + scratch_memory_size, wind_instance_r);
            wind_instance_r->buf_ctrl.buffer_cnt = 0;
        }

        wind_instance->memory_check = WIND_VALID_MEMORY_CHECK_VALUE;
        wind_instance->nvkey_ready = false;
        wind_instance->init_done = false;
        wind_instance->buf_ctrl.buffer_cnt = 0;
    }

    //Re-read Nvkey when create stream late
    if (wind_instance_nvkey_ptr && !(dsp_stream_algo_nvkey_loading_status & WND_NVKEY_LOADING)) {
        stream_function_wind_load_nvkey(wind_instance_nvkey_ptr);
    }

    if (wind_instance->nvkey_ready) {
        WindDet_Init(&wind_instance->scratch_memory, &wind_instance->nvkey);
        if(stream_function_wind_check_headset()){
            WindDet_Init(&wind_instance_r->scratch_memory, &wind_instance->nvkey);
        }
        wind_instance->init_done = true;
        wind_instance->detection_period = (wind_instance->nvkey.option) ? 1 : wind_instance->nvkey.vad_leave;

        // reset wnd status every time init.
        wind_change_status(false);
    }
    DSP_MW_LOG_I("[Wind Detection] initialize:  init = %d, nvkey = %d, nvkey.enable = %d, Headset_project:%d", 4, wind_instance->init_done, wind_instance->nvkey_ready, wind_instance->nvkey.ENABLE, stream_function_wind_check_headset());

    return false;
}

#ifdef AIR_BTA_IC_PREMIUM_G3
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ bool stream_function_wind_process(void *para)
#else
ATTR_TEXT_IN_IRAM_LEVEL_2 bool stream_function_wind_process(void *para)
#endif
{
    S16 *input_buffer = stream_function_get_1st_inout_buffer(para);
    #ifndef AIR_ANC_USER_UNAWARE_ENABLE
    S16 *input_buffer2 = stream_function_get_2nd_inout_buffer(para); //FF_R
    #else
    S16 *input_buffer2 = stream_function_get_3rd_inout_buffer(para); //FF_R
    #endif

    uint32_t frame_size;
    int32_t samples;
    S16 detect_result, detect_result_r = -1;
    S16 mix_detect_result = -1;

    if (!wind_check_memory() || !wind_instance->init_done) {
        stream_function_wind_initialize(para);
    }

    if (!wind_instance->init_done) {
        DSP_MW_LOG_I("[Wind Detection] haven't init = %d, nvkey = %d", 2, wind_instance->init_done, wind_instance->nvkey_ready);
        return false;
    }

    #ifdef AIR_ANC_SCENARIO_CONTROL_GAIN_ENABLE
    if (g_anc_ramp_by_scenario_set_wnd && g_anc_ramp_by_scenario) { //first scenario up, reset wnd to 0
        stream_function_wind_reset_status();
        g_anc_ramp_by_scenario_set_wnd = false;
        g_anc_wind_suspend_by_scenario = true;
        DSP_MW_LOG_I("[Wind Detection] g_anc_ramp_by_scenario_suspend_wnd, reset&suspend wnd", 0);
    } else if (g_anc_ramp_by_scenario_set_wnd && (!g_anc_ramp_by_scenario)) { //resume wnd
        g_anc_ramp_by_scenario_set_wnd = false;
        g_anc_wind_suspend_by_scenario = false;
        DSP_MW_LOG_I("[Wind Detection] g_anc_ramp_by_scenario_suspend_wnd, resume wnd", 0);
    }
    #endif

    if (stream_function_get_output_resolution(para) == RESOLUTION_32BIT) {
    }

    frame_size = stream_function_get_output_size(para);
    samples = frame_size / sizeof(int16_t); //RESOLUTION_16BIT
    S16 wind_mix_buffer[samples];
    memset(wind_mix_buffer, 0, sizeof(wind_mix_buffer));

    if (stream_function_wind_check_headset() && (input_buffer != NULL) && (input_buffer2 != NULL)) { //Headset
        wind_push_data(&wind_instance->buf_ctrl, input_buffer, samples);
        wind_push_data(&wind_instance_r->buf_ctrl, input_buffer2, samples);
    } else { //Earbuds
        wind_push_data(&wind_instance->buf_ctrl, input_buffer, samples);
    }

    while (wind_get_data_cnt() >= WIND_DETECTION_PROCESS_FRAME) {

        LOG_AUDIO_DUMP((U8 *)wind_get_read_buffer_ptr(&wind_instance->buf_ctrl), (U32)WIND_DETECTION_PROCESS_FRAME * sizeof(int16_t), AUDIO_SOUNDBAR_INPUT);
        #ifndef AIR_ANC_USER_UNAWARE_ENABLE //borrow UU's audio dump ID
        if (stream_function_wind_check_headset()) { //Dump R channel
            LOG_AUDIO_DUMP((U8 *)wind_get_read_buffer_ptr(&wind_instance_r->buf_ctrl), (U32)WIND_DETECTION_PROCESS_FRAME * sizeof(int16_t), AUDIO_WOOFER_UPSAMPLE_16K);
        }
        #endif

        WindDet_Prcs(&wind_instance->scratch_memory, wind_get_read_buffer_ptr(&wind_instance->buf_ctrl), &detect_result);
        if (stream_function_wind_check_headset()) { //process R channel
            WindDet_Prcs(&wind_instance_r->scratch_memory, wind_get_read_buffer_ptr(&wind_instance_r->buf_ctrl), &detect_result_r);
        }

        memset((U8 *)wind_get_read_buffer_ptr(&wind_instance->buf_ctrl), detect_result * 100, (U32)WIND_DETECTION_PROCESS_FRAME * sizeof(int16_t));
        LOG_AUDIO_DUMP((U8 *)wind_get_read_buffer_ptr(&wind_instance->buf_ctrl), (U32)WIND_DETECTION_PROCESS_FRAME * sizeof(int16_t), AUDIO_SOUNDBAR_TX);
        #ifndef AIR_ANC_USER_UNAWARE_ENABLE //borrow UU's audio dump ID
        if (stream_function_wind_check_headset()) { //Dump R result
            memset((U8 *)wind_get_read_buffer_ptr(&wind_instance_r->buf_ctrl), detect_result_r * 100, (U32)WIND_DETECTION_PROCESS_FRAME * sizeof(int16_t));
            LOG_AUDIO_DUMP((U8 *)wind_get_read_buffer_ptr(&wind_instance_r->buf_ctrl), (U32)WIND_DETECTION_PROCESS_FRAME * sizeof(int16_t), AUDIO_WOOFER_CPD_OUT);
        }
        #endif

        wind_pop_data(&wind_instance->buf_ctrl, WIND_DETECTION_PROCESS_FRAME);
        if (stream_function_wind_check_headset()) {
            wind_pop_data(&wind_instance_r->buf_ctrl, WIND_DETECTION_PROCESS_FRAME);
        }

        if ((detect_result >= 0)||(detect_result_r >= 0)) {
            //Report detected result
            mix_detect_result = wind_comprehensive_detect_result(detect_result, detect_result_r);
            if ((wind_instance->nvkey._reserved) && (!(wind_instance->process_cnt % wind_instance->nvkey.vad_leave))) {
                DSP_MW_LOG_I("[Wind Detection] report status:(%d)|(%d)=(%d), cnt:%d ", 4, detect_result, detect_result_r, mix_detect_result, wind_instance->process_cnt);
            }
            if (mix_detect_result != wind_instance->detect_wind) {
                wind_instance->consecutive_times += wind_instance->detection_period;
                if (!mix_detect_result && (wind_instance->consecutive_times >= wind_instance->nvkey.release_count)) {
                    //Silent environment
                    wind_change_status(mix_detect_result);
                } else if (mix_detect_result && (wind_instance->consecutive_times >= wind_instance->nvkey.attack_count)) {
                    //Wind environment
                    wind_change_status(mix_detect_result);
                }
            } else {
                //Reset counter if result is unchanged
                wind_instance->consecutive_times = 0;
            }
        }
        wind_instance->process_cnt++;
    }

    return false;
}


