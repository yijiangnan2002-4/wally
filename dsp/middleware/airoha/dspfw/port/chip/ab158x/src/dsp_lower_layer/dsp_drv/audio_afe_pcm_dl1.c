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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sink.h"
#include "transform.h"
#include "stream_audio_setting.h"
#include "hal_nvic.h"
#include "FreeRTOS.h"
#include "stream_audio_driver.h"
#include "dsp_callback.h"
#include "hal_audio_afe_control.h"
#include "hal_audio_afe_define.h"
#include "audio_afe_common.h"
#include "dsp_audio_ctrl.h"

extern afe_stream_channel_t connect_type[2][2];
extern afe_t afe;
#define AIR_DL_HW_DLOOPBACK_DEBUD_ENABLE (0)
#if defined(AIR_AUDIO_PATH_CUSTOMIZE_ENABLE) || defined(AIR_DL_HW_DLOOPBACK_DEBUD_ENABLE)
extern bool hal_audio_path_set_interconnection_state(hal_audio_path_interconnection_state_t connection_state, hal_audio_path_interconnection_input_t input, hal_audio_path_interconnection_output_t output);
extern bool hal_audio_status_get_agent_status(hal_audio_agent_t agent);
extern bool hal_audio_status_get_agent_of_type_status(hal_audio_agent_t agent, audio_scenario_type_t type);
#endif
#ifdef AIR_DCHS_MODE_ENABLE
#include "stream_dchs.h"
hal_audio_memory_selection_t dchs_main_dl_mem;
#define DCHS_LINE_OUT_DL_SYNC_TIME  108
#endif
#if AIR_DL_HW_DLOOPBACK_DEBUD_ENABLE
#include "hal_audio_path.h"
#include "hal_audio_driver.h"
#include "hal_audio_control.h"
#include "dsp_dump.h"

#define DSP_LOOPBACK_DUMP_SIZE (480)
#define DSP_LOOPBACK_DUMP_FRAME_NUMBER (4)


typedef struct {
    SOURCE      source;
    SINK        sink;
    TRANSFORM   transform;
    stream_feature_list_ptr_t pfeature_table;
} CONNECTION_IF;


U32 dsp_loopbackdump_control_cnt;
U32 loopbackdump_memory_dump_process;

hal_audio_memory_parameter_t dsp_loopbackdump_memory = {
    HAL_AUDIO_MEMORY_UL_AWB,
    HAL_AUDIO_PCM_FORMAT_S16_LE,
    HAL_AUDIO_MEMORY_SYNC_NONE,
    96000,
    0,
    DSP_LOOPBACK_DUMP_SIZE*DSP_LOOPBACK_DUMP_FRAME_NUMBER,
    0,
    0,
    0,
    0,
    DSP_LOOPBACK_DUMP_SIZE/2,
    HAL_AUDIO_SRC_TRACKING_DISABLE,
    true,
    false,
    #ifdef ENABLE_HWSRC_CLKSKEW
    0,
    #endif
    AUDIO_SCENARIO_TYPE_HFP_DL,
};
ATTR_TEXT_IN_IRAM void afe_loopback_interrupt_handler(void)
{
    LOG_AUDIO_DUMP((U8*)dsp_loopbackdump_memory.buffer_addr + (DSP_LOOPBACK_DUMP_SIZE*loopbackdump_memory_dump_process), (U32)DSP_LOOPBACK_DUMP_SIZE, AUDIO_SOUNDBAR_INPUT);
    loopbackdump_memory_dump_process++;
    if (loopbackdump_memory_dump_process >=DSP_LOOPBACK_DUMP_FRAME_NUMBER) {
        loopbackdump_memory_dump_process = 0;
    }
}

#endif

static void dl1_global_var_init(SINK sink)
{
#if 0
    afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;
    memset(afe_block, 0, sizeof(afe_block_t));
#else
    UNUSED(sink);
#endif
}

static int32_t pcm_dl1_probe(SINK sink)
{
    dl1_global_var_init(sink);
    return 0;
}

#if defined (AIR_DCHS_MODE_ENABLE)
uint32_t dchs_main_dl_handle;
static void hal_audio_gpt_trigger_mem(void)
{
    hal_audio_trigger_start_parameter_t start_parameter;
    start_parameter.memory_select = dchs_main_dl_mem;
    start_parameter.enable = true;
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
    DSP_MW_LOG_I("[DCHS UL]line_out Trigger DL Mem success",0);
    hal_gpt_sw_free_timer(dchs_main_dl_handle);
}
#endif

static int32_t pcm_dl1_start(SINK sink)
{
    AUDIO_PARAMETER *runtime = &sink->param.audio;
    afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;
    uint32_t dl_samplerate = runtime->rate;
    UNUSED(dl_samplerate);
    hal_audio_memory_parameter_t *mem_handle = &sink->param.audio.mem_handle;
    if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL1 || mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_SRC1) {
        mem_handle->audio_path_rate = runtime->rate;
        mem_handle->src_rate = runtime->src_rate;
        afe.dl1_enable = true;//workaround for 1552
#ifdef ENABLE_HWSRC_CLKSKEW
        if (mem_handle->asrc_clkskew_mode == HAL_AUDIO_SRC_CLK_SKEW_V2) {
            if (afe_block->u4asrcflag) {
                afe_block->u4asrcIrqParaCleanDone = false;
                afe_block->u4asrcSetCompensatedSamples = COMPENSATING_ADD_0_XPPM;
            }
        }
#endif
    } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL2 || mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_SRC2) {
        hal_audio_get_value_parameter_t get_value_handle;
        get_value_handle.get_device_rate.device_control = runtime->audio_device;
        get_value_handle.get_device_rate.device_interface = (hal_audio_interface_t)runtime->audio_interface;
        get_value_handle.get_device_rate.is_tx = false;
        mem_handle->audio_path_rate = hal_audio_get_value(&get_value_handle, HAL_AUDIO_GET_DEVICE_SAMPLE_RATE);//runtime->rate;
        if (runtime->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
            mem_handle->audio_path_rate = runtime->rate;
        }
        mem_handle->src_rate = runtime->src_rate;//hal_audio_get_value(&get_value_handle, HAL_AUDIO_GET_DEVICE_SAMPLE_RATE);
        afe.dl2_enable = true;//workaround for 1552
    } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL3) {
        mem_handle->audio_path_rate = runtime->rate;
        mem_handle->src_rate = runtime->src_rate;
    } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL12) {
        mem_handle->audio_path_rate = runtime->rate;
        mem_handle->src_rate = runtime->src_rate;
    }
    if (mem_handle->pure_agent_with_src) {
        //asrc out
        mem_handle->buffer_addr = afe_block->phys_buffer_addr;
        mem_handle->buffer_length = afe_block->u4asrc_buffer_size;
        //asrc in
        mem_handle->src_buffer_addr = afe_block->phys_buffer_addr + afe_block->u4asrc_buffer_size;
        mem_handle->src_buffer_length = runtime->buffer_size;
        afe_block->u4asrcrate = dl_samplerate;

        //issue BTA-9231
        if(mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL2) {
            // fix bug: vp use wrong src out rate in irq handler to do sram empty check
            afe_block->u4asrcrate = mem_handle->audio_path_rate;
            runtime->count = (mem_handle->audio_path_rate * ((runtime->count * 1000) / (runtime->rate / 1000))) / (1000 * 1000); //runtime->count = (mem_handle->audio_path_rate * runtime->period) / 1000;
        }
        mem_handle->irq_counter = runtime->count;
    } else if (mem_handle->memory_select & (HAL_AUDIO_MEMORY_DL_SRC1 | HAL_AUDIO_MEMORY_DL_SRC2)) {
        //mem_handle->src_buffer_addr = afe_block->phys_buffer_addr;
        //mem_handle->src_buffer_length = runtime->buffer_size;
        //asrc out
        mem_handle->buffer_addr = afe_block->phys_buffer_addr;
        mem_handle->buffer_length = runtime->buffer_size;
        //asrc in
        mem_handle->src_buffer_addr = afe_block->phys_buffer_addr;
        mem_handle->src_buffer_length = runtime->buffer_size;
    } else {
        mem_handle->buffer_addr = afe_block->phys_buffer_addr;
        mem_handle->buffer_length = runtime->buffer_size;
    }
    /* set irq start */
    if (!sink->param.audio.AfeBlkControl.u4awsflag) {
        if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL2 || mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_SRC2) {
            if (sink->param.audio.aws_sync_request != true) {
                //DSP_MW_LOG_I("[Rdebug2]VP irq enable", 0);
                //nosnyc
                mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_NONE;
            } else {
                // sw triger
                mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_SW_TRIGGER;
            }
        } else {
            mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_NONE;
        }
    } else {
        //plat_en
        mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_PLAY_EN;
    }
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
    if (sink->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
        mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_SW_TRIGGER; // LINE OUT No need Playen
    }
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
    if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_LINE_OUT) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0)) {
        mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_PLAY_EN;
    }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0)) {
        mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_PLAY_EN;
    }
#endif /* defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) */
#ifdef AIR_DCHS_MODE_ENABLE
    if (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT) {
        mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_SW_TRIGGER; // LINE OUT No need Playen
        dchs_main_dl_mem = mem_handle->memory_select;
        uint32_t count_1, hfp_delay_count,delay_time;
        delay_time = DCHS_LINE_OUT_DL_SYNC_TIME;
        hal_sw_gpt_absolute_parameter_t  dchs_hfp_absolute_parameter;
        hal_gpt_sw_get_timer(&dchs_main_dl_handle);
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count_1);
        hfp_delay_count = ((uint32_t)(delay_time* 1000));
        dchs_hfp_absolute_parameter.absolute_time_count = count_1 + hfp_delay_count;
        dchs_hfp_absolute_parameter.callback = (void*)hal_audio_gpt_trigger_mem;
        dchs_hfp_absolute_parameter.maxdelay_time_count = hfp_delay_count;
        hal_gpt_sw_start_timer_for_absolute_tick_1M(dchs_main_dl_handle,&dchs_hfp_absolute_parameter);
    }
#endif /* AIR_DCHS_MODE_ENABLE */
#if defined(AIR_WIRED_AUDIO_ENABLE) && !defined(AIR_DCHS_MODE_ENABLE)
    if ((sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN) ||
        (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM) ||
        (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT)) {
        mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_SW_TRIGGER;
    }
#endif
    mem_handle->initial_buffer_offset = sink->streamBuffer.BufferInfo.WriteOffset;
    DSP_MW_LOG_I("[AFE DL] start: scenario type %d memory%d rate:%d src_rate:%d src_buffer_addr:%x src_buffer_size:%d buffer_addr:%x buffer_size:%d count:%d period:%d sync_status:%d u4awsflag:%d pure_agent_with_src:%d aws_sync_request:%d initial_buffer_offset:%d", 15,
            sink->scenario_type,
            mem_handle->memory_select,
            mem_handle->audio_path_rate,
            mem_handle->src_rate,
            mem_handle->src_buffer_addr,
            mem_handle->src_buffer_length,
            mem_handle->buffer_addr,
            mem_handle->buffer_length,
            mem_handle->irq_counter,
            runtime->period,
            mem_handle->sync_status,
            sink->param.audio.AfeBlkControl.u4awsflag,
            mem_handle->pure_agent_with_src,
            sink->param.audio.aws_sync_request,
            mem_handle->initial_buffer_offset
            );
    if ((runtime->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && (runtime->memory == HAL_AUDIO_MEM6)) {
        //I2S Slave Infra mode
        hal_audio_slave_vdma_parameter_t vdma_setting;
        vdma_setting.base_address = afe_block->phys_buffer_addr;
        vdma_setting.size = mem_handle->buffer_length >> 2;
        vdma_setting.threshold = mem_handle->buffer_length >> 2;
        vdma_setting.audio_interface = runtime->audio_interface;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        vdma_setting.tdm_channel = HAL_AUDIO_I2S_TDM_DISABLE;
        vdma_setting.enable = true;
#endif
        vdma_setting.is_ul_mode = false;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&vdma_setting, HAL_AUDIO_SET_SLAVE_VDMA);
    } else if ((runtime->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && (runtime->memory == HAL_AUDIO_MEM7)) {
        //I2S Slave Tdm mode
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        hal_audio_slave_vdma_parameter_t vdma_setting;
        vdma_setting.base_address = afe_block->phys_buffer_addr;
        vdma_setting.size = mem_handle->buffer_length >> 2;
        vdma_setting.threshold = mem_handle->buffer_length >> 2;
        vdma_setting.audio_interface = runtime->audio_interface;
        vdma_setting.tdm_channel = gAudioCtrl.Afe.AfeULSetting.tdm_channel;
        vdma_setting.enable = true;
        vdma_setting.is_ul_mode = false;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&vdma_setting, HAL_AUDIO_SET_SLAVE_VDMA);
#endif
    } else {
        hal_audio_set_memory(mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_ON);
    }
#if AIR_DL_HW_DLOOPBACK_DEBUD_ENABLE
    if ((!dsp_loopbackdump_control_cnt)) {
        DSP_MW_LOG_I("DSP audio Debug bump Enable \r\n", 0);
        hal_i2s_master_set_loopback(AFE_I2S0, true);
        loopbackdump_memory_dump_process = DSP_LOOPBACK_DUMP_FRAME_NUMBER-1;
        hal_audio_set_value_parameter_t register_handler;
        register_handler.register_irq_handler.audio_irq = HAL_AUDIO_IRQ_AUDIOSYS;
        register_handler.register_irq_handler.memory_select = dsp_loopbackdump_memory.memory_select;
        register_handler.register_irq_handler.entry = afe_loopback_interrupt_handler;
        hal_audio_set_value(&register_handler, HAL_AUDIO_SET_IRQ_HANDLER);

        hal_audio_set_memory(&dsp_loopbackdump_memory, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_ON);

        //Connect I2S1 loopback to AWB
        hal_audio_path_set_interconnection_state(AUDIO_INTERCONNECTION_CONNECT, AUDIO_INTERCONNECTION_INPUT_I18, AUDIO_INTERCONNECTION_OUTPUT_O20);
        hal_audio_path_set_interconnection_state(AUDIO_INTERCONNECTION_CONNECT, AUDIO_INTERCONNECTION_INPUT_I19, AUDIO_INTERCONNECTION_OUTPUT_O21);
    }
    dsp_loopbackdump_control_cnt++;
#endif
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
        hal_audio_memory_parameter_t *mem_handle1;
        hal_audio_memory_parameter_t *mem_handle2;
        hal_audio_set_value_parameter_t set_handle;
        if (sink->param.audio.channel_num >= 4) {
            memcpy(&sink->param.audio.mem_handle1, mem_handle, sizeof(hal_audio_memory_parameter_t));
            mem_handle1 = &sink->param.audio.mem_handle1;
            if (mem_handle->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM) {
                mem_handle->memory_select = hal_memory_convert_dl(sink->param.audio.memory & HAL_AUDIO_MEM3);
                mem_handle1->memory_select = hal_memory_convert_dl(sink->param.audio.memory & HAL_AUDIO_MEM1);
            } else {
                mem_handle->memory_select = hal_memory_convert_dl(sink->param.audio.memory & HAL_AUDIO_MEM1);
                mem_handle1->memory_select = hal_memory_convert_dl(sink->param.audio.memory & HAL_AUDIO_MEM3);
            }
            mem_handle1->buffer_addr += mem_handle1->buffer_length;
            mem_handle1->src_buffer_addr += mem_handle1->src_buffer_length;
            hal_audio_set_memory(mem_handle1, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_ON);
            /* Workaorund to disable redundant IRQ counter */
            set_handle.irq_enable.memory_select = mem_handle1->memory_select;
            set_handle.irq_enable.enable = false;
            hal_audio_set_value(&set_handle, HAL_AUDIO_SET_MEMORY_IRQ_ENABLE);
            if (sink->param.audio.channel_num >= 6) {
                memcpy(&sink->param.audio.mem_handle2, mem_handle1, sizeof(hal_audio_memory_parameter_t));
                mem_handle2 = &sink->param.audio.mem_handle2;
                mem_handle2->memory_select = hal_memory_convert_dl(sink->param.audio.memory & HAL_AUDIO_MEM4);
                mem_handle2->buffer_addr += mem_handle2->buffer_length;
                mem_handle2->src_buffer_addr += mem_handle2->src_buffer_length;
                hal_audio_set_memory(mem_handle2, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_ON);
                /* Workaorund to disable redundant IRQ counter */
                set_handle.irq_enable.memory_select = mem_handle2->memory_select;
                set_handle.irq_enable.enable = false;
                hal_audio_set_value(&set_handle, HAL_AUDIO_SET_MEMORY_IRQ_ENABLE);
            }
        }
#endif

    return 0;
}

static int32_t pcm_dl1_stop(SINK sink)
{
#if AIR_DL_HW_DLOOPBACK_DEBUD_ENABLE
    dsp_loopbackdump_control_cnt--;
    if (!dsp_loopbackdump_control_cnt) {
    DSP_MW_LOG_I("DSP audio Debug bump disable \r\n", 0);
    hal_audio_path_set_interconnection_state(AUDIO_INTERCONNECTION_DISCONNECT, AUDIO_INTERCONNECTION_INPUT_I18, AUDIO_INTERCONNECTION_OUTPUT_O20);
    hal_audio_path_set_interconnection_state(AUDIO_INTERCONNECTION_DISCONNECT, AUDIO_INTERCONNECTION_INPUT_I19, AUDIO_INTERCONNECTION_OUTPUT_O21);

    hal_audio_set_memory(&dsp_loopbackdump_memory, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_OFF);
    dsp_loopbackdump_memory.buffer_addr = 0;
    hal_i2s_master_set_loopback(AFE_I2S0, false);
    }
#endif
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    if ((sink->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && (sink->param.audio.memory == HAL_AUDIO_MEM6)) {
        //I2S Slave Infra mode
    } else if ((sink->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && (sink->param.audio.memory == HAL_AUDIO_MEM7)) {
        //I2S Slave Tdm mode
        hal_audio_slave_vdma_parameter_t vdma_setting;
        vdma_setting.audio_interface = sink->param.audio.audio_interface;
        vdma_setting.tdm_channel = gAudioCtrl.Afe.AfeULSetting.tdm_channel;
        vdma_setting.enable = false;
        vdma_setting.is_ul_mode = false;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&vdma_setting, HAL_AUDIO_SET_SLAVE_VDMA);
    } else {
        hal_audio_memory_parameter_t *mem_handle = &sink->param.audio.mem_handle;
        if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL1 || mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_SRC1) {
            afe.dl1_enable = false;//workaround for 1552
        } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL2 || mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_SRC2) {
            afe.dl2_enable = false;//workaround for 1552
        }
        hal_audio_set_memory(mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_OFF);
    }
#else
    hal_audio_memory_parameter_t *mem_handle = &sink->param.audio.mem_handle;
    if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL1 || mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_SRC1) {
        afe.dl1_enable = false;//workaround for 1552
    } else if (mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL2 || mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_SRC2) {
        afe.dl2_enable = false;//workaround for 1552
    }
    hal_audio_set_memory(mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_OFF);
#endif

#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    if (sink->param.audio.channel_num >= 4) {
        mem_handle = &sink->param.audio.mem_handle1;
        hal_audio_set_memory(mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_OFF);
        if (sink->param.audio.channel_num >= 6) {
            mem_handle = &sink->param.audio.mem_handle2;
            hal_audio_set_memory(mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_OFF);
        }
    }
#endif
    return 0;
}

static int32_t pcm_dl1_hw_params(SINK sink)
{
    UNUSED(sink);
    return 0;
}

extern DSP_AUDIO_CTRL_t gAudioCtrl;
static int32_t pcm_dl1_open(SINK sink)
{
    AUDIO_PARAMETER *runtime = &sink->param.audio;
    hal_audio_device_t device = sink->param.audio.audio_device;//hal_audio_get_stream_out_device();
    hal_audio_memory_parameter_t *mem_handle = &sink->param.audio.mem_handle;
    uint32_t sink_ch;
    hal_audio_path_parameter_t *handle = &sink->param.audio.path_handle;//modify for ab1568
    hal_audio_device_parameter_t *device_handle = &sink->param.audio.device_handle;//modify for ab1568
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    hal_audio_device_t device1 = sink->param.audio.audio_device1;//hal_audio_get_stream_out_device();
    hal_audio_device_t device2 = sink->param.audio.audio_device2;//hal_audio_get_stream_out_device();
    hal_audio_device_parameter_t *device_handle1 = &sink->param.audio.device_handle1;
    hal_audio_device_parameter_t *device_handle2 = &sink->param.audio.device_handle2;
#endif
    //uint32_t i;
    DSP_MW_LOG_I("[AFE DL] open: scenario type %d", 1, sink->scenario_type);
    sink_ch = (runtime->channel_num >= 2)
              ? 1
              : 0;

    runtime->connect_channel_type = connect_type[sink_ch][sink_ch];
#if 0//modify for ab1568
    /*set interconnection*/
    DSP_MW_LOG_I("DSP audio pcm_dl1_open channel_type:%d \r\n", 1, runtime->connect_channel_type);
    hal_audio_afe_set_connection(runtime, false, true);

    afe_audio_device_enable(true, device, runtime->audio_interface, runtime->format, runtime->rate, runtime->misc_parms);

    if (runtime->hw_gain) {
        if (runtime->memory == HAL_AUDIO_MEM1) {
            afe_set_hardware_digital_gain_mode(AFE_HW_DIGITAL_GAIN1, afe_get_audio_device_samplerate(device, runtime->audio_interface));
            afe_enable_hardware_digital_gain(AFE_HW_DIGITAL_GAIN1, true);
        } else {
            afe_set_hardware_digital_gain_mode(AFE_HW_DIGITAL_GAIN2, afe_get_audio_device_samplerate(device, runtime->audio_interface));
            afe_enable_hardware_digital_gain(AFE_HW_DIGITAL_GAIN2, true);
        }
    }
#else
    #ifdef AIR_AUDIO_PATH_CUSTOMIZE_ENABLE
    if ((gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL) || (gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL)
        || (hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_DEVICE_I2S1_MASTER_DUAL_TX) && (gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_VP))) {
        hal_audio_device_parameter_t device_i2s_master_temp;
        device_i2s_master_temp.i2s_master.audio_device = HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER;
        device_i2s_master_temp.i2s_master.i2s_interface = HAL_AUDIO_INTERFACE_2;
        device_i2s_master_temp.i2s_master.rate = handle->audio_output_rate[0];
        device_i2s_master_temp.i2s_master.scenario_type = gAudioCtrl.Afe.AfeDLSetting.scenario_type;
        device_i2s_master_temp.i2s_master.is_tx = true;
        device_i2s_master_temp.i2s_master.i2s_format = HAL_AUDIO_I2S_I2S;
        device_i2s_master_temp.i2s_master.word_length = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;
        device_i2s_master_temp.i2s_master.mclk_divider = 0;
        device_i2s_master_temp.i2s_master.with_mclk = false;
        device_i2s_master_temp.i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeDLSetting.is_low_jitter[1];
        device_i2s_master_temp.i2s_master.is_rx_swap = false;
        device_i2s_master_temp.i2s_master.is_tx_swap = false;
        device_i2s_master_temp.i2s_master.is_internal_loopback = false;
        device_i2s_master_temp.i2s_master.is_recombinant = false;
        hal_audio_set_device(&device_i2s_master_temp, HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER, HAL_AUDIO_CONTROL_ON);
    }
    #endif

    #if defined(AIR_DCHS_MODE_ENABLE) || defined(AIR_MIXER_STREAM_ENABLE) //DCHS project device ON/OFF by dchs dl agent only
    #ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
        hal_audio_set_device(device_handle, device, HAL_AUDIO_CONTROL_ON);
    }else
    #endif
    {
        if(sink->param.audio.mem_handle.scenario_type == AUDIO_SCENARIO_TYPE_MIXER_STREAM){
            hal_audio_set_device(device_handle, device, HAL_AUDIO_CONTROL_ON);
        }
    }
    #else
    hal_audio_set_device(device_handle, device, HAL_AUDIO_CONTROL_ON);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    if (sink->param.audio.channel_num >= 4) {
        hal_audio_set_device(device_handle1, device1, HAL_AUDIO_CONTROL_ON);
        if (sink->param.audio.channel_num >= 6) {
            hal_audio_set_device(device_handle2, device2, HAL_AUDIO_CONTROL_ON);
        }
    }
#endif
    #ifdef AIR_WIRED_AUDIO_SUB_STREAM_ENABLE
    if(sink->param.audio.mem_handle.scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM){
        hal_audio_device_parameter_t *device_handle1 = &sink->param.audio.device_handle1;
        device_handle1->common.scenario_type = AUDIO_SCENARIO_TYPE_WIRED_AUDIO_SUBSTREAM;
        DSP_MW_LOG_I("SUBSTREAM on device_handle=0x%x,audio_device=0x%x audio_interface=0x%x, device_handle1=0x%x,audio_device1=0x%x audio_interface1=0x%x",6,
                device_handle, device, sink->param.audio.audio_interface, device_handle1, sink->param.audio.audio_device1, sink->param.audio.audio_interface1);
        if((device == sink->param.audio.audio_device1) && (sink->param.audio.audio_interface == sink->param.audio.audio_interface1)){
            assert(0 && "SUBSTREAM == MAINSTREAM");
        }
        hal_audio_set_device(device_handle1, sink->param.audio.audio_device1, HAL_AUDIO_CONTROL_ON);
        if(mem_handle->memory_select == HAL_AUDIO_MEMORY_DL_DL3){
            if((device == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER)&&(sink->param.audio.audio_device1 == HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL)){
                hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_CONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I06, AUDIO_INTERCONNECTION_OUTPUT_O08);
                hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_CONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I07, AUDIO_INTERCONNECTION_OUTPUT_O09);
            } else if ((device == HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL)&&(sink->param.audio.audio_device1 == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER)){
                    if(sink->param.audio.audio_interface1 == HAL_AUDIO_INTERFACE_1){
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_CONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I06, AUDIO_INTERCONNECTION_OUTPUT_O00);
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_CONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I07, AUDIO_INTERCONNECTION_OUTPUT_O01);
                    }else if(sink->param.audio.audio_interface1 == HAL_AUDIO_INTERFACE_2){
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_CONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I06, AUDIO_INTERCONNECTION_OUTPUT_O02);
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_CONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I07, AUDIO_INTERCONNECTION_OUTPUT_O03);
                    }else if(sink->param.audio.audio_interface1 == HAL_AUDIO_INTERFACE_3){
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_CONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I06, AUDIO_INTERCONNECTION_OUTPUT_O04);
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_CONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I07, AUDIO_INTERCONNECTION_OUTPUT_O05);
                    }else if(sink->param.audio.audio_interface1 == HAL_AUDIO_INTERFACE_4){
                #ifdef AIR_BTA_IC_PREMIUM_G3
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_CONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I06, AUDIO_INTERCONNECTION_OUTPUT_O42);
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_CONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I07, AUDIO_INTERCONNECTION_OUTPUT_O43);
                #endif
                    }
            } else {
                DSP_MW_LOG_E("SUBSTREAM not support",0);
            }
        } else {
            DSP_MW_LOG_E("SUBSTREAM not support",0);
        }
    }
    #endif
    #endif //AIR_DCHS_MODE_ENABLE

    if (mem_handle->pure_agent_with_src) {
        hal_audio_get_value_parameter_t get_value_handle;
        get_value_handle.get_device_rate.device_control = runtime->audio_device;
        get_value_handle.get_device_rate.device_interface = (hal_audio_interface_t)runtime->audio_interface;
        handle->audio_output_rate[0] =  hal_audio_get_value(&get_value_handle, HAL_AUDIO_GET_DEVICE_SAMPLE_RATE);//runtime->rate;
        if(runtime->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
            handle->audio_output_rate[0] = runtime->rate;
        }
        if (sink->type == SINK_TYPE_VP_AUDIO) {
            device_handle->common.rate = handle->audio_output_rate[0];
        }
    }
    if ((runtime->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && ((runtime->memory == HAL_AUDIO_MEM6) || (runtime->memory == HAL_AUDIO_MEM7))) {
        //I2S Slave Infra mode or Tdm mode
    } else {
        if (handle->audio_output_rate[0] == 0) {
            assert(0 && "[AFE DL]ERROR: DSP audio pcm_dl_open: output sample rate error = 0!!!!");
        } else {
#ifdef AIR_AUDIO_PATH_CUSTOMIZE_ENABLE
            DSP_MW_LOG_I("MITO type: %d",1,gAudioCtrl.Afe.AfeDLSetting.scenario_type);
            if ((gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL) || (gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL)
                || (hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_DEVICE_I2S1_MASTER_DUAL_TX) && (gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_VP))) {
                // DL to I2S0
                hal_audio_path_parameter_t path1_temp;
                if (gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_VP) {
                    path1_temp.input.interconn_sequence[0]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_MEMORY_DL2_CH1;
                    path1_temp.input.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_MEMORY_DL2_CH2;
                } else {
                    path1_temp.input.interconn_sequence[0]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_MEMORY_DL1_CH1;
                    path1_temp.input.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_MEMORY_DL1_CH2;
                }
                path1_temp.output.interconn_sequence[0] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_MASTER_I2S1_CH1;
                path1_temp.output.interconn_sequence[1] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_MASTER_I2S1_CH2;
                path1_temp.connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
                path1_temp.connection_number = 2;
                path1_temp.audio_input_rate[0] = handle->audio_input_rate[0];
                path1_temp.audio_input_rate[1] = handle->audio_input_rate[1];
                path1_temp.audio_output_rate[0] = handle->audio_output_rate[0];
                path1_temp.audio_output_rate[1] = handle->audio_output_rate[1];
                path1_temp.with_hw_gain=true;
                path1_temp.with_updown_sampler[0]=0;
                path1_temp.with_updown_sampler[1]=0;
                path1_temp.with_dl_deq_mixer=0;
                path1_temp.out_device=4096;
                path1_temp.out_device_interface=2;
                path1_temp.scenario_type = gAudioCtrl.Afe.AfeDLSetting.scenario_type;
                hal_audio_set_path(&path1_temp, HAL_AUDIO_CONTROL_ON);

                hal_i2s_master_set_loopback(AFE_I2S1, true); //DEBUG

                // I2S0 to DAC
                hal_audio_path_parameter_t path3_temp;
                path3_temp.input.interconn_sequence[0]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_MASTER_I2S1_CH1;
                path3_temp.input.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_MASTER_I2S1_CH2;
                path3_temp.output.interconn_sequence[0] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_DAC_CH1;
                path3_temp.output.interconn_sequence[1] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_DAC_CH2;
                path3_temp.connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
                path3_temp.connection_number = 2;
                path3_temp.audio_input_rate[0] = handle->audio_output_rate[0];
                path3_temp.audio_input_rate[1] = handle->audio_output_rate[1];
                path3_temp.audio_output_rate[0] = handle->audio_output_rate[0];
                path3_temp.audio_output_rate[1] = handle->audio_output_rate[1];

                path3_temp.with_hw_gain=false;
                path3_temp.with_updown_sampler[0]=0;
                path3_temp.with_updown_sampler[1]=0;
                path3_temp.with_dl_deq_mixer=0;
                path3_temp.out_device=768;
                path3_temp.out_device_interface=1;
                path3_temp.scenario_type = gAudioCtrl.Afe.AfeDLSetting.scenario_type;
                hal_audio_set_path(&path3_temp, HAL_AUDIO_CONTROL_ON);
            }else {
                hal_audio_set_path(handle, HAL_AUDIO_CONTROL_ON);
            }
#else
            hal_audio_set_path(handle, HAL_AUDIO_CONTROL_ON);
#endif
        }
    }

#if 0//interconn in
    //Sine generator for FGPA verification TEMP!!!
    hal_audio_sine_generator_parameter_t sine_generator;
    sine_generator.enable = true;

    sine_generator.rate = device_handle->common.rate;
    //sine_generator.audio_control = HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL;
    sine_generator.audio_control = HAL_AUDIO_CONTROL_MEMORY_INTERFACE;
    //sine_generator.port_parameter.device_interface = HAL_AUDIO_INTERFACE_1;
    sine_generator.port_parameter.memory_select = mem_handle->memory_select;
    sine_generator.is_input_port = true;
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&sine_generator, HAL_AUDIO_SET_SINE_GENERATOR);
#endif
#if 0//interconn out
    //Sine generator for FGPA verification TEMP!!!
    hal_audio_sine_generator_parameter_t sine_generator;
    sine_generator.enable = true;

    sine_generator.rate = device_handle->common.rate;
    sine_generator.audio_control = HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL;
    //sine_generator.audio_control = HAL_AUDIO_CONTROL_MEMORY_INTERFACE;
    sine_generator.port_parameter.device_interface = HAL_AUDIO_INTERFACE_1;
    //sine_generator.port_parameter.memory_select = HAL_AUDIO_MEMORY_DL_DL1;
    sine_generator.is_input_port = false;
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&sine_generator, HAL_AUDIO_SET_SINE_GENERATOR);
#endif
#endif
    return 0;
}

static int32_t pcm_dl1_close(SINK sink)
{
    hal_audio_device_t device = sink->param.audio.audio_device;
    hal_audio_path_parameter_t *handle = &sink->param.audio.path_handle;//modify for ab1568
    hal_audio_device_parameter_t *device_handle = &sink->param.audio.device_handle;//modify for ab1568
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    hal_audio_device_t device1 = sink->param.audio.audio_device1;//hal_audio_get_stream_out_device();
    hal_audio_device_t device2 = sink->param.audio.audio_device2;//hal_audio_get_stream_out_device();
    hal_audio_device_parameter_t *device_handle1 = &sink->param.audio.device_handle1;
    hal_audio_device_parameter_t *device_handle2 = &sink->param.audio.device_handle2;
#endif
    DSP_MW_LOG_I("[AFE DL] close: scenario type %d\r\n", 1, sink->scenario_type);


#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    if ((device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && ((sink->param.audio.memory == HAL_AUDIO_MEM6) || (sink->param.audio.memory == HAL_AUDIO_MEM7))) {
        //I2S Slave Infra mode or Tdm mode
    } else {
        hal_audio_set_path(handle, HAL_AUDIO_CONTROL_OFF);
    }
#else
#ifdef AIR_AUDIO_PATH_CUSTOMIZE_ENABLE
    if ((sink->scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL) || (sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL)
        || (hal_audio_status_get_agent_of_type_status(HAL_AUDIO_AGENT_DEVICE_I2S1_MASTER_DUAL_TX, AUDIO_SCENARIO_TYPE_VP) && (sink->scenario_type == AUDIO_SCENARIO_TYPE_VP))) {

        hal_audio_path_parameter_t path1_temp;
        if (sink->scenario_type == AUDIO_SCENARIO_TYPE_VP) {
            path1_temp.input.interconn_sequence[0]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_MEMORY_DL2_CH1;
            path1_temp.input.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_MEMORY_DL2_CH2;
        } else {
            path1_temp.input.interconn_sequence[0]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_MEMORY_DL1_CH1;
            path1_temp.input.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_MEMORY_DL1_CH2;
        }
        path1_temp.output.interconn_sequence[0] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_MASTER_I2S1_CH1;
        path1_temp.output.interconn_sequence[1] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_MASTER_I2S1_CH2;
        path1_temp.connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
        path1_temp.connection_number = 2;
        path1_temp.audio_input_rate[0] = handle->audio_input_rate[0];
        path1_temp.audio_input_rate[1] = handle->audio_input_rate[1];
        path1_temp.audio_output_rate[0] = handle->audio_output_rate[0];
        path1_temp.audio_output_rate[1] = handle->audio_output_rate[1];
        path1_temp.with_hw_gain=true;
        path1_temp.with_updown_sampler[0]=0;
        path1_temp.with_updown_sampler[1]=0;
        path1_temp.with_dl_deq_mixer=0;
        path1_temp.out_device=4096;
        path1_temp.out_device_interface=2;
        path1_temp.scenario_type = sink->scenario_type;
        hal_audio_set_path(&path1_temp, HAL_AUDIO_CONTROL_OFF);

        hal_audio_path_parameter_t path3_temp;
        path3_temp.input.interconn_sequence[0]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_MASTER_I2S0_CH1;
        path3_temp.input.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_MASTER_I2S0_CH2;
        path3_temp.output.interconn_sequence[0] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_DAC_CH1;
        path3_temp.output.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_DAC_CH2;
        path3_temp.connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
        path3_temp.connection_number = 2;
        path3_temp.audio_input_rate[0] = handle->audio_output_rate[0];
        path3_temp.audio_input_rate[1] = handle->audio_output_rate[1];
        path3_temp.audio_output_rate[0] = handle->audio_output_rate[0];
        path3_temp.audio_output_rate[1] = handle->audio_output_rate[1];
        path3_temp.with_hw_gain=false;
        path3_temp.with_updown_sampler[0]=0;
        path3_temp.with_updown_sampler[1]=0;
        path3_temp.with_dl_deq_mixer=0;
        path3_temp.out_device=768;
        path3_temp.out_device_interface=1;
        path3_temp.scenario_type = sink->scenario_type;
        hal_audio_set_path(&path3_temp, HAL_AUDIO_CONTROL_OFF);
    } else {
        hal_audio_set_path(handle, HAL_AUDIO_CONTROL_OFF);
    }
#else

        hal_audio_set_path(handle, HAL_AUDIO_CONTROL_OFF);
#endif
    #if defined(AIR_DCHS_MODE_ENABLE) || defined(AIR_MIXER_STREAM_ENABLE) //DCHS project device ON/OFF by dchs dl agent only
    #ifdef AIR_DCHS_MODE_ENABLE
    if (dchs_get_device_mode() == DCHS_MODE_SINGLE) {
        hal_audio_set_device(device_handle, device, HAL_AUDIO_CONTROL_OFF);
    } else
    #endif
    {
        if(sink->scenario_type == AUDIO_SCENARIO_TYPE_MIXER_STREAM){
            hal_audio_set_device(device_handle, device, HAL_AUDIO_CONTROL_OFF);
        }
    }
    #else
    #ifdef AIR_WIRED_AUDIO_SUB_STREAM_ENABLE
    if(sink->param.audio.mem_handle.scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM){
        hal_audio_device_parameter_t *device_handle1 = &sink->param.audio.device_handle1;
        device_handle1->common.scenario_type = AUDIO_SCENARIO_TYPE_WIRED_AUDIO_SUBSTREAM;
        DSP_MW_LOG_I("SUBSTREAM off  device_handle=0x%x,audio_device=0x%x audio_interface=0x%x, device_handle1=0x%x,audio_device1=0x%x audio_interface1=0x%x",6,
                device_handle, device, sink->param.audio.audio_interface, device_handle1, sink->param.audio.audio_device1, sink->param.audio.audio_interface1);
        hal_audio_set_device(device_handle1, sink->param.audio.audio_device1, HAL_AUDIO_CONTROL_OFF);
        if(sink->param.audio.mem_handle.memory_select == HAL_AUDIO_MEMORY_DL_DL3){
            if((device == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER)&&(sink->param.audio.audio_device1 == HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL)){
                hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I06, AUDIO_INTERCONNECTION_OUTPUT_O08);
                hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I07, AUDIO_INTERCONNECTION_OUTPUT_O09);
            } else if ((device == HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL)&&(sink->param.audio.audio_device1 == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER)){
                    if(sink->param.audio.audio_interface1 == HAL_AUDIO_INTERFACE_1){
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I06, AUDIO_INTERCONNECTION_OUTPUT_O00);
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I07, AUDIO_INTERCONNECTION_OUTPUT_O01);
                    }else if(sink->param.audio.audio_interface1 == HAL_AUDIO_INTERFACE_2){
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I06, AUDIO_INTERCONNECTION_OUTPUT_O02);
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I07, AUDIO_INTERCONNECTION_OUTPUT_O03);
                    }else if(sink->param.audio.audio_interface1 == HAL_AUDIO_INTERFACE_3){
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I06, AUDIO_INTERCONNECTION_OUTPUT_O04);
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I07, AUDIO_INTERCONNECTION_OUTPUT_O05);
                #ifdef AIR_BTA_IC_PREMIUM_G3
                    }else if(sink->param.audio.audio_interface1 == HAL_AUDIO_INTERFACE_4){
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I06, AUDIO_INTERCONNECTION_OUTPUT_O42);
                        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I07, AUDIO_INTERCONNECTION_OUTPUT_O43);
                #endif
                    }
            } else {
                DSP_MW_LOG_E("SUBSTREAM not support",0);
            }
        } else {
            DSP_MW_LOG_E("SUBSTREAM not support",0);
        }
    }
    #endif
    hal_audio_set_device(device_handle, device, HAL_AUDIO_CONTROL_OFF);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    if (sink->param.audio.channel_num >= 4) {
        hal_audio_set_device(device_handle1, device1, HAL_AUDIO_CONTROL_OFF);
        if (sink->param.audio.channel_num >= 6) {
            hal_audio_set_device(device_handle2, device2, HAL_AUDIO_CONTROL_OFF);
        }
    }
#endif
    #endif //AIR_DCHS_MODE_ENABLE

    #ifdef AIR_AUDIO_PATH_CUSTOMIZE_ENABLE
    DSP_MW_LOG_I("MITO type: %d",1,sink->scenario_type);
    if ((sink->scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL) || (sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL)
        || (hal_audio_status_get_agent_of_type_status(HAL_AUDIO_AGENT_DEVICE_I2S1_MASTER_DUAL_TX, AUDIO_SCENARIO_TYPE_VP) && (sink->scenario_type == AUDIO_SCENARIO_TYPE_VP))) {
        hal_audio_device_parameter_t device_i2s_master_temp;
        device_i2s_master_temp.i2s_master.audio_device = HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER;
        device_i2s_master_temp.i2s_master.i2s_interface = HAL_AUDIO_INTERFACE_2;
        device_i2s_master_temp.i2s_master.rate = handle->audio_output_rate[0];
        device_i2s_master_temp.i2s_master.scenario_type = sink->scenario_type;
        device_i2s_master_temp.i2s_master.is_tx = true;
        device_i2s_master_temp.i2s_master.i2s_format = HAL_AUDIO_I2S_I2S;
        device_i2s_master_temp.i2s_master.word_length = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;
        device_i2s_master_temp.i2s_master.mclk_divider = 0;
        device_i2s_master_temp.i2s_master.with_mclk = false;
        device_i2s_master_temp.i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeDLSetting.is_low_jitter[1];
        device_i2s_master_temp.i2s_master.is_rx_swap = false;
        device_i2s_master_temp.i2s_master.is_tx_swap = false;
        device_i2s_master_temp.i2s_master.is_internal_loopback = false;
        device_i2s_master_temp.i2s_master.is_recombinant = false;
        hal_audio_set_device(&device_i2s_master_temp, HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER, HAL_AUDIO_CONTROL_OFF);
    }
    #endif
#endif
    return 0;
}

static int32_t pcm_dl1_trigger(SINK sink, int cmd)
{
    switch (cmd) {
        case AFE_PCM_TRIGGER_START:
            return pcm_dl1_start(sink);
            break;
        case AFE_PCM_TRIGGER_STOP:
            return pcm_dl1_stop(sink);
            break;
        case AFE_PCM_TRIGGER_RESUME:
            return pcm_dl1_open(sink);
            break;
        case AFE_PCM_TRIGGER_SUSPEND:
            return pcm_dl1_close(sink);
            break;
        default:
            break;
    }
    return -1;
}

// src: Source Streambuffer not Sink Streambuffer
ATTR_TEXT_IN_IRAM_LEVEL_2 static int32_t pcm_dl1_copy(SINK sink, void *src, uint32_t count)
{
    // count: w/o channl, unit: bytes
    // copy the src's streambuffer to sink's streambuffer
    if (Sink_Audio_WriteBuffer(sink, src, count) == false) {
        return -1;
    }
    return 0;
}

audio_sink_pcm_ops_t afe_platform_dl1_ops = {
    .probe      = pcm_dl1_probe,
    .open       = pcm_dl1_open,
    .close      = pcm_dl1_close,
    .hw_params  = pcm_dl1_hw_params,
    .trigger    = pcm_dl1_trigger,
    .copy       = pcm_dl1_copy,
};
