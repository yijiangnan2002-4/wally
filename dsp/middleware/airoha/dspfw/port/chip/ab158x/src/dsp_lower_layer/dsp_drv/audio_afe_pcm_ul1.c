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
#include "source.h"
#include "stream_audio_setting.h"
#include "hal_nvic.h"
#include "FreeRTOS.h"
#include "stream_audio_driver.h"
#include "dsp_callback.h"
#include "dsp_audio_ctrl.h"
#include "hal_audio_afe_control.h"
#include "hal_audio_afe_define.h"
#include "audio_afe_common.h"
#include "hal_audio_clock.h"
#if defined (AIR_DCHS_MODE_ENABLE)
#include "stream_dchs.h"
#include "hal_gpt.h"
#include "dsp_mux_uart.h"
#define DCHS_USB_OUT_UL_SYNC_TIME  100
#endif

extern afe_stream_channel_t connect_type[2][2];
extern afe_t afe;
extern audio_source_pcm_ops_t afe_platform_awb_ops;
#if defined(AIR_AUDIO_PATH_CUSTOMIZE_ENABLE)
extern bool hal_audio_path_set_interconnection_state(hal_audio_path_interconnection_state_t connection_state, hal_audio_path_interconnection_input_t input, hal_audio_path_interconnection_output_t output);
#endif

// NOTE: 4096 for HWSRC i2s-tracking buffer's size at 48K side
// TODO: 8192 for HWSRC i2s-tracking buffer's size at 96K side
#ifdef AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ
#define AUDIO_AFE_SOURCE_ASRC_BUFFER_SIZE 8192
#else
#define AUDIO_AFE_SOURCE_ASRC_BUFFER_SIZE 4096
#endif

static void ul1_global_var_init(SOURCE source)
{
#if 0
    afe_block_t *afe_block = &source->param.audio.AfeBlkControl;
    memset(afe_block, 0, sizeof(afe_block_t));
#else
    UNUSED(source);
#endif
}

#if defined (AIR_DCHS_MODE_ENABLE)
uint32_t dchs_main_ul_handle;
uint32_t usb_line_out_gpt_count;
hal_audio_memory_selection_t dchs_main_ul_mem;
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void hal_audio_gpt_trigger_mem(void)
{
    uint32_t savedmask = 0;
    uint32_t curr_cnt  = 0;
    S32 cur_native_bt_clk = 0, cur_native_bt_phase = 0;

    hal_nvic_save_and_set_interrupt_mask_special(&savedmask); // enter cirtical code region

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
    if (usb_line_out_gpt_count > curr_cnt) { // gpt register does not overflow
        // DSP_MW_LOG_I("[DCHS UL][line_out/usb-out] trigger %d curr_cnt %u  tar %d", 3, __LINE__, curr_cnt, usb_line_out_gpt_count);
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            if (curr_cnt >= usb_line_out_gpt_count) { // expire at time
                break;
            }
        }
    } else if (curr_cnt - usb_line_out_gpt_count > 0x7fffffff) { // gpt register overflow
        // DSP_MW_LOG_I("[DCHS UL][line_out/usb-out] trigger %d curr_cnt %u  tar %d", 3, __LINE__, curr_cnt, usb_line_out_gpt_count);
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            if (curr_cnt >= usb_line_out_gpt_count) { // expire at time
                if ((curr_cnt & 0x80000000) == 0x0) {
                    break;
                }
            }
        }
    }
    hal_audio_trigger_start_parameter_t start_parameter;
    start_parameter.memory_select = dchs_main_ul_mem;
    start_parameter.enable = true;
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
    MCE_GetBtClk((BTCLK *)&cur_native_bt_clk, (BTPHASE *)&cur_native_bt_phase, DCHS_CLK_Offset);
    hal_gpt_sw_free_timer(dchs_main_ul_handle);
    hal_nvic_restore_interrupt_mask_special(savedmask);
    DSP_MW_LOG_I("[DCHS UL]line_out/usb-out Trigger Mem success cur_native_bt_clk:%u,cur_native_bt_phase:%u",2,cur_native_bt_clk,cur_native_bt_phase);
}
#endif
static int32_t pcm_ul1_probe(SOURCE source)
{
    ul1_global_var_init(source);
    return 0;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static int32_t pcm_ul1_start(SOURCE source)
{
    AUDIO_PARAMETER *runtime = &source->param.audio;
    afe_block_t *afe_block = &source->param.audio.AfeBlkControl;
    hal_audio_memory_parameter_t *mem_handle = &source->param.audio.mem_handle;//modify for ab1568
    hal_audio_memory_selection_t memory_select, fined_memory = 0, memory_search; //modify for ab1568
    uint32_t channel_index = 0;
    uint32_t update_offset = 0;
    mem_handle->audio_path_rate = runtime->rate;
    mem_handle->buffer_addr = afe_block->phys_buffer_addr;
    mem_handle->buffer_length = runtime->buffer_size;
    if(source->param.audio.mem_handle.pure_agent_with_src){
        mem_handle->buffer_length = AUDIO_AFE_SOURCE_ASRC_BUFFER_SIZE;
    }

    DSP_MW_LOG_I("[AFE UL] start: scenario type %d memory select:0x%x\r\n", 2, source->scenario_type, mem_handle->memory_select);//modify for ab1568
    mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_SW_TRIGGER;
    /* Set Memory */
    memory_select = mem_handle->memory_select;
    for (memory_search = HAL_AUDIO_MEMORY_UL_VUL1 ; memory_search <= HAL_AUDIO_MEMORY_UL_AWB2 ; memory_search <<= 1) {
        if (memory_search & memory_select) {
            uint32_t addr = 0;
            mem_handle->buffer_addr = 0; //Clear buffer ptr to allocate new SRAM
            mem_handle->memory_select = memory_search;
            hal_audio_set_memory(mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_ON);
            if (source->param.audio.mem_handle.pure_agent_with_src) {
                source->streamBuffer.BufferInfo.startaddr[channel_index++] = (void *)mem_handle->src_buffer_addr;
                addr = (uint32_t)mem_handle->src_buffer_addr;
            } else {
                source->streamBuffer.BufferInfo.startaddr[channel_index++] = (void *)mem_handle->buffer_addr;
                addr = (uint32_t)mem_handle->buffer_addr;
            }
            DSP_MW_LOG_I("[AFE UL] channel %d mem_search:0x%x, mem adr:0x%x\r\n", 3,
                channel_index,
                memory_search,
                addr);
            if (!fined_memory) {
                fined_memory = memory_search;
            }
        }
    }
    mem_handle->memory_select = memory_select;

#define ReadREG(_addr)          (*(volatile uint32_t *)(_addr))
    if ((runtime->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && (runtime->memory == HAL_AUDIO_MEM6)) {
        //I2S Slave Infra mode
        hal_audio_agent_t agent = HAL_AUDIO_AGENT_ERROR;
        if (runtime->audio_interface == HAL_AUDIO_INTERFACE_1) {
            #if defined(AIR_BTA_IC_PREMIUM_G2)
            agent = HAL_AUDIO_AGENT_DEVICE_I2S0_SLAVE;
            #else
            agent = HAL_AUDIO_AGENT_DEVICE_I2S0_SLAVE_TX;
            #endif
        } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_2) {
            #if defined(AIR_BTA_IC_PREMIUM_G2)
            agent = HAL_AUDIO_AGENT_DEVICE_I2S1_SLAVE;
            #else
            agent = HAL_AUDIO_AGENT_DEVICE_I2S1_SLAVE_TX;
            #endif
        } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_3) {
            #if defined(AIR_BTA_IC_PREMIUM_G2)
            agent = HAL_AUDIO_AGENT_DEVICE_I2S2_SLAVE;
            #else
            agent = HAL_AUDIO_AGENT_DEVICE_I2S2_SLAVE_TX;
            #endif
        }
        source->streamBuffer.BufferInfo.startaddr[0] = (void *)hal_memory_allocate_sram(mem_handle->scenario_type, agent, mem_handle->buffer_length);
        afe_block->phys_buffer_addr = (uint32_t)source->streamBuffer.BufferInfo.startaddr[0];

        hal_audio_slave_vdma_parameter_t vdma_setting;
        vdma_setting.base_address = afe_block->phys_buffer_addr;
        vdma_setting.size = mem_handle->buffer_length >> 2;
        vdma_setting.threshold = mem_handle->buffer_length >> 2;
        vdma_setting.audio_interface = runtime->audio_interface;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        vdma_setting.tdm_channel = HAL_AUDIO_I2S_TDM_DISABLE;
        vdma_setting.enable = true;
#endif
        vdma_setting.is_ul_mode = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&vdma_setting, HAL_AUDIO_SET_SLAVE_VDMA);
        if (runtime->audio_interface == HAL_AUDIO_INTERFACE_1) {
            Source_Audio_BufferInfo_Rst(source, ReadREG(0xC9000830) - ReadREG(0xC900082c));
        } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_2) {
            Source_Audio_BufferInfo_Rst(source, ReadREG(0xC9000230) - ReadREG(0xC900022c));
        } else if (runtime->audio_interface == HAL_AUDIO_INTERFACE_3) {
            Source_Audio_BufferInfo_Rst(source, ReadREG(0xC9000a30) - ReadREG(0xC9000a2c));
        }
    } else if ((runtime->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && (runtime->memory == HAL_AUDIO_MEM7)) {
        //I2S Slave Tdm mode
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        source->streamBuffer.BufferInfo.startaddr[0] = (void *)hal_memory_allocate_sram(mem_handle->scenario_type, HAL_AUDIO_AGENT_DEVICE_TDM_RX, mem_handle->buffer_length * runtime->device_handle.i2s_slave.tdm_channel);
        afe_block->phys_buffer_addr = (uint32_t)source->streamBuffer.BufferInfo.startaddr[0];
        for (channel_index = 1; channel_index < runtime->device_handle.i2s_slave.tdm_channel; channel_index++) {
            source->streamBuffer.BufferInfo.startaddr[channel_index] = source->streamBuffer.BufferInfo.startaddr[channel_index - 1] + mem_handle->buffer_length;
        }

        hal_audio_slave_vdma_parameter_t vdma_setting;
        vdma_setting.base_address = afe_block->phys_buffer_addr;
        vdma_setting.size = mem_handle->buffer_length >> 2;
        vdma_setting.threshold = mem_handle->buffer_length >> 2;
        vdma_setting.audio_interface = runtime->audio_interface;
        vdma_setting.tdm_channel = runtime->device_handle.i2s_slave.tdm_channel;
        vdma_setting.enable = true;
        vdma_setting.is_ul_mode = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&vdma_setting, HAL_AUDIO_SET_SLAVE_VDMA);
#endif
    } else if ((source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0) ||
        (source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
        (source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN)) {
        /* do nothing */
        // Trigger the memory agent by-self to ensure the low latency!
    }
#if defined(AIR_WIRED_AUDIO_ENABLE) && !defined(AIR_DCHS_MODE_ENABLE)
    else if (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN) {
        /* do nothing */
        // Trigger the memory agent by-self to ensure the low latency!
    }
#endif
    else {
        if (!source->param.audio.AfeBlkControl.u4awsflag) {
            /*trigger memory agent for no u4awsflag condition  */
            hal_audio_trigger_start_parameter_t start_parameter;
            start_parameter.memory_select = mem_handle->memory_select;
            start_parameter.enable = true;
#if defined (AIR_DCHS_MODE_ENABLE)
            if((source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT) || (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT)){
                dchs_main_ul_mem = mem_handle->memory_select;
                uint32_t count_1, hfp_delay_count,delay_time;
                S32 cur_native_bt_clk = 0,cur_native_bt_phase = 0;
                delay_time = DCHS_USB_OUT_UL_SYNC_TIME;
                hal_sw_gpt_absolute_parameter_t  dchs_hfp_absolute_parameter;

                hal_gpt_sw_get_timer(&dchs_main_ul_handle);

                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count_1);
                MCE_GetBtClk((BTCLK *)&cur_native_bt_clk, (BTPHASE *)&cur_native_bt_phase, DCHS_CLK_Offset);

                hfp_delay_count = ((uint32_t)(delay_time* 1000));
                usb_line_out_gpt_count = count_1 + hfp_delay_count;
                dchs_hfp_absolute_parameter.absolute_time_count = count_1 + hfp_delay_count - 600;
                dchs_hfp_absolute_parameter.callback = (void*)hal_audio_gpt_trigger_mem;
                dchs_hfp_absolute_parameter.maxdelay_time_count = hfp_delay_count;
                dps_uart_relay_ul_mem_sync_info(delay_time, cur_native_bt_clk, cur_native_bt_phase);

                hal_gpt_sw_start_timer_for_absolute_tick_1M(dchs_main_ul_handle,&dchs_hfp_absolute_parameter);

            }else{
                hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
            }
#else
            hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
#endif
            mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_NONE;
        }

        /* Update initial offset */
        hal_audio_current_offset_parameter_t get_current_offset;
        get_current_offset.memory_select = fined_memory;
        get_current_offset.pure_agent_with_src = false;
        hal_audio_get_value((hal_audio_get_value_parameter_t *)&get_current_offset, HAL_AUDIO_GET_MEMORY_INPUT_CURRENT_OFFSET);
        if ((get_current_offset.offset != 0) && (get_current_offset.base_address != 0)) {
            update_offset = get_current_offset.offset - get_current_offset.base_address;
        }
        if (mem_handle->sync_status == HAL_AUDIO_MEMORY_SYNC_SW_TRIGGER) {
            update_offset = 0;
        }
        Source_Audio_BufferInfo_Rst(source, update_offset);
    }


#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE)
#if !defined(AIR_DCHS_MODE_ENABLE)
    if (source->scenario_type != AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN) {
#endif
        if ((source->type >= SOURCE_TYPE_SUBAUDIO_MIN) && (source->type <= SOURCE_TYPE_SUBAUDIO_MAX)) {
            hal_audio_memory_irq_enable_parameter_t irq_enable;
            irq_enable.memory_select    = HAL_AUDIO_MEMORY_UL_MASK;
            irq_enable.irq_counter      = mem_handle->irq_counter;
            irq_enable.rate             = mem_handle->audio_path_rate;
            irq_enable.enable           = true;
            hal_audio_set_value((hal_audio_set_value_parameter_t *)&irq_enable, HAL_AUDIO_SET_MEMORY_IRQ_ENABLE);
        }
#if !defined(AIR_DCHS_MODE_ENABLE)
    }
#endif
#endif
    source->param.audio.is_memory_start = true;
    return 0;
}

static int32_t pcm_ul1_stop(SOURCE source)
{
    AUDIO_PARAMETER *runtime = &source->param.audio;
    UNUSED(runtime);
    //modify for ab1568 new driver
    hal_audio_memory_parameter_t *mem_handle = &source->param.audio.mem_handle;
    hal_audio_memory_selection_t memory_select, memory_search;
    DSP_MW_LOG_I("[AFE UL] stop: scenario type %d memory select:0x%x", 2, source->scenario_type, mem_handle->memory_select);
    source->param.audio.is_memory_start = false;
#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE)
    if ((source->type >= SOURCE_TYPE_SUBAUDIO_MIN) && (source->type <= SOURCE_TYPE_SUBAUDIO_MAX)) {
        hal_audio_memory_irq_enable_parameter_t irq_enable;
        irq_enable.memory_select    = HAL_AUDIO_MEMORY_UL_MASK;
        irq_enable.irq_counter      = mem_handle->irq_counter;
        irq_enable.rate             = mem_handle->audio_path_rate;
        irq_enable.enable           = false;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&irq_enable, HAL_AUDIO_SET_MEMORY_IRQ_ENABLE);
    }
#endif

    /* Set Memory */
    memory_select = mem_handle->memory_select;
    for (memory_search = HAL_AUDIO_MEMORY_UL_VUL1 ; memory_search <= HAL_AUDIO_MEMORY_UL_AWB2 ; memory_search <<= 1) {
        if (memory_search & memory_select) {
            mem_handle->memory_select = memory_search;
            hal_audio_set_memory(mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_OFF);
        }
    }
    mem_handle->memory_select = memory_select;

#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    if ((runtime->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && (runtime->memory == HAL_AUDIO_MEM7)) {
        //I2S Slave Tdm mode
        hal_audio_slave_vdma_parameter_t vdma_setting;
        vdma_setting.audio_interface = runtime->audio_interface;
        vdma_setting.tdm_channel = runtime->device_handle.i2s_slave.tdm_channel;
        vdma_setting.enable = false;
        vdma_setting.is_ul_mode = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&vdma_setting, HAL_AUDIO_SET_SLAVE_VDMA);
        hal_memory_free_sram(mem_handle->scenario_type, HAL_AUDIO_AGENT_DEVICE_TDM_RX);
    }
#endif
    return 0;
}

static int32_t pcm_ul1_hw_params(SOURCE source)
{
    UNUSED(source);
    return 0;
}

static int32_t pcm_ul1_open(SOURCE source)
{
    uint32_t source_ch;
    AUDIO_PARAMETER *runtime = &source->param.audio;
    DSP_MW_LOG_I("[AFE UL] open: scenario type %d already enabled %d", 2, source->scenario_type, runtime->is_pcm_ul1_open);
    if (runtime->is_pcm_ul1_open) {
        return 0;
    }
    runtime->is_pcm_ul1_open = true;
    hal_audio_device_t device = source->param.audio.audio_device;//hal_audio_get_stream_in_device();
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    hal_audio_device_t device1 = source->param.audio.audio_device1;
    hal_audio_device_t device2 = source->param.audio.audio_device2;
    hal_audio_device_t device3 = source->param.audio.audio_device3;
    hal_audio_device_t device4 = source->param.audio.audio_device4;
    hal_audio_device_t device5 = source->param.audio.audio_device5;
    hal_audio_device_t device6 = source->param.audio.audio_device6;
    hal_audio_device_t device7 = source->param.audio.audio_device7;
#endif
    int32_t ret = 0;
    hal_audio_path_parameter_t *path_handle = &source->param.audio.path_handle;//modify for ab1568
    hal_audio_device_parameter_t *device_handle = &source->param.audio.device_handle;//modify for ab1568
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    hal_audio_device_parameter_t *device_handle1 = &source->param.audio.device_handle1;//modify for ab1568
    hal_audio_device_parameter_t *device_handle2 = &source->param.audio.device_handle2;//modify for ab1568
    hal_audio_device_parameter_t *device_handle3 = &source->param.audio.device_handle3;//modify for ab1568
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    hal_audio_device_parameter_t *device_handle4 = &source->param.audio.device_handle4;//modify for ab1568
    hal_audio_device_parameter_t *device_handle5 = &source->param.audio.device_handle5;//modify for ab1568
    hal_audio_device_parameter_t *device_handle6 = &source->param.audio.device_handle6;//modify for ab1568
    hal_audio_device_parameter_t *device_handle7 = &source->param.audio.device_handle7;//modify for ab1568
#endif
#endif
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    if ((device == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER) || (device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) ||
        (device == HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL) || (device == HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL) ||
        (device == HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL)) {
         device1 = NULL;
    }
#endif
#ifdef AIR_DCHS_MODE_ENABLE
    dsp_uart_ul_open();
#endif
#if 0
    TRANSFORM transform = source->transform;
    DSP_CALLBACK_PTR callback_ptr = NULL;
    audio_afe_io_block_t audio_block_in, audio_block_out;
    uint32_t stream_ch, source_ch;
    callback_ptr = DSP_Callback_Get(source, transform->sink);
    stream_ch = (callback_ptr->EntryPara.in_channel_num > 2)
                ? 1
                : callback_ptr->EntryPara.in_channel_num - 1;
    source_ch = (runtime->channel_num > 2)
                ? 1
                : runtime->channel_num - 1;
    runtime->connect_channel_type = connect_type[stream_ch][source_ch];
#else
    source_ch = (runtime->channel_num > 2)
                ? 1
                : runtime->channel_num - 1;
    runtime->connect_channel_type = connect_type[source_ch][source_ch];
#endif
#if 0//modify for ab1568
    /*set interconnection*/
    DSP_MW_LOG_I("DSP audio pcm_ul1_open channel_type:%d \r\n", 1, runtime->connect_channel_type);
    hal_audio_afe_set_connection(runtime, true, true);

#if 0
    //UpLink use software gain control
    if (runtime->hw_gain) {
        afe_set_hardware_digital_gain_mode(AFE_HW_DIGITAL_GAIN2, runtime->rate, 20);
        afe_enable_hardware_digital_gain(AFE_HW_DIGITAL_GAIN2, true);
    }
#endif


    afe_audio_device_enable(true, device, runtime->audio_interface, runtime->format, runtime->rate, runtime->misc_parms);

#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    if (device1 != HAL_AUDIO_DEVICE_NONE) {
        afe_audio_device_enable(true, device1, runtime->audio_interface1, runtime->memory, runtime->format, runtime->rate, runtime->misc_parms);
    }
    if (device2 != HAL_AUDIO_DEVICE_NONE) {
        afe_audio_device_enable(true, device2, runtime->audio_interface2, runtime->memory, runtime->format, runtime->rate, runtime->misc_parms);
    }
    if (device3 != HAL_AUDIO_DEVICE_NONE) {
        afe_audio_device_enable(true, device3, runtime->audio_interface3, runtime->memory, runtime->format, runtime->rate, runtime->misc_parms);
    }
#endif
    if (source->param.audio.echo_reference == true) {
        ret = afe_platform_awb_ops.open(source);
    }
#else
    if ((runtime->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && ((runtime->memory == HAL_AUDIO_MEM6) || (runtime->memory == HAL_AUDIO_MEM7))) {
        //I2S Slave Infra mode or Tdm mode
    } else {
#ifdef AIR_AUDIO_PATH_CUSTOMIZE_ENABLE
        // ADDA UL to I2S0
        if ((gAudioCtrl.Afe.AfeULSetting.scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL) || (gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL)) {
            hal_audio_path_parameter_t path1_temp;
            path1_temp.input.interconn_sequence[0]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_UL2_CH1;
            path1_temp.input.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_UL2_CH2;
            path1_temp.output.interconn_sequence[0] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_MASTER_I2S0_CH1;
            path1_temp.output.interconn_sequence[1] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_MASTER_I2S0_CH2;
            path1_temp.connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
            path1_temp.connection_number = 2;
            path1_temp.audio_input_rate[0] = path_handle->audio_input_rate[0];
            path1_temp.audio_input_rate[1] = path_handle->audio_input_rate[1];
            path1_temp.audio_output_rate[0] = path_handle->audio_output_rate[0];
            path1_temp.audio_output_rate[1] = path_handle->audio_output_rate[1];
            path1_temp.with_hw_gain=false;
            path1_temp.with_updown_sampler[0]=false;
            path1_temp.with_updown_sampler[1]=false;
            path1_temp.with_dl_deq_mixer=0;
            path1_temp.out_device=4096;
            path1_temp.out_device_interface=2;
            path1_temp.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
            hal_audio_set_path(&path1_temp, HAL_AUDIO_CONTROL_ON);

            hal_i2s_master_set_loopback(AFE_I2S0, true); //DEBUG

            // I2S0 to UL
            hal_audio_path_parameter_t path3_temp;
            path3_temp.input.interconn_sequence[0]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_MASTER_I2S0_CH1;
            path3_temp.input.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_MASTER_I2S0_CH2;
            path3_temp.output.interconn_sequence[0] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_MEMORY_VUL1_CH1;
            path3_temp.output.interconn_sequence[1] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_MEMORY_VUL1_CH2;
            path3_temp.connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
            path3_temp.connection_number = 2;
            path3_temp.audio_input_rate[0] = path_handle->audio_output_rate[0];
            path3_temp.audio_input_rate[1] = path_handle->audio_output_rate[1];
            path3_temp.audio_output_rate[0] = path_handle->audio_output_rate[0];
            path3_temp.audio_output_rate[1] = path_handle->audio_output_rate[1];

            path3_temp.with_hw_gain=false;
            path3_temp.with_updown_sampler[0]=0;
            path3_temp.with_updown_sampler[1]=0;
            path3_temp.with_dl_deq_mixer=0;
            path3_temp.out_device=768;
            path3_temp.out_device_interface=1;
            path3_temp.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
            hal_audio_set_path(&path3_temp, HAL_AUDIO_CONTROL_ON);
        } else {
            hal_audio_set_path(path_handle, HAL_AUDIO_CONTROL_ON);
        }
#else
        hal_audio_set_path(path_handle, HAL_AUDIO_CONTROL_ON);
#endif
    }
#ifdef AIR_AUDIO_PATH_CUSTOMIZE_ENABLE
    if ((gAudioCtrl.Afe.AfeULSetting.scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL) || (gAudioCtrl.Afe.AfeDLSetting.scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL)) {
        hal_audio_device_parameter_t device_i2s_master_temp;
        device_i2s_master_temp.i2s_master.audio_device = HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER;
        device_i2s_master_temp.i2s_master.i2s_interface = HAL_AUDIO_INTERFACE_1;
        device_i2s_master_temp.i2s_master.rate = path_handle->audio_output_rate[0];
        device_i2s_master_temp.i2s_master.scenario_type = gAudioCtrl.Afe.AfeULSetting.scenario_type;
        device_i2s_master_temp.i2s_master.is_tx = false;
        device_i2s_master_temp.i2s_master.i2s_format = HAL_AUDIO_I2S_I2S;
        device_i2s_master_temp.i2s_master.word_length = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;
        device_i2s_master_temp.i2s_master.mclk_divider = 0;
        device_i2s_master_temp.i2s_master.with_mclk = false;
        device_i2s_master_temp.i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeULSetting.is_low_jitter[1];
        device_i2s_master_temp.i2s_master.is_rx_swap = false;
        device_i2s_master_temp.i2s_master.is_tx_swap = false;
        device_i2s_master_temp.i2s_master.is_internal_loopback = false;
        device_i2s_master_temp.i2s_master.is_recombinant = false;
        hal_audio_set_device(&device_i2s_master_temp, HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER, HAL_AUDIO_CONTROL_ON);
    }
#endif
    if (!(device & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R))) {
        hal_audio_set_device(device_handle, device, HAL_AUDIO_CONTROL_ON);
    }
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    if ((device1 != NULL) && (!(device1 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle1, device1, HAL_AUDIO_CONTROL_ON);
    }
    if ((device2 != NULL) && (!(device2 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle2, device2, HAL_AUDIO_CONTROL_ON);
    }
    if ((device3 != NULL) && (!(device3 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle3, device3, HAL_AUDIO_CONTROL_ON);
    }
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if ((device4 != NULL) && (!(device4 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle4, device4, HAL_AUDIO_CONTROL_ON);
    }
    if ((device5 != NULL) && (!(device5 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle5, device5, HAL_AUDIO_CONTROL_ON);
    }
    if ((device6 != NULL) && (!(device6 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle6, device6, HAL_AUDIO_CONTROL_ON);
    }
    if ((device7 != NULL) && (!(device7 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle7, device7, HAL_AUDIO_CONTROL_ON);
    }
#endif
#endif
#if 0
    //Sine generator for FGPA verification TEMP!!!
    hal_audio_sine_generator_parameter_t sine_generator;
    sine_generator.enable = true;

    sine_generator.rate = device_handle->common.rate;
    sine_generator.audio_control = device_handle->common.audio_device;
    sine_generator.port_parameter.device_interface = device_handle->common.device_interface;
    sine_generator.is_input_port = true;
    DSP_MW_LOG_I("audio_controol %d rate %d audio_interface %d sine_generator.rate %d\r\n", 4, sine_generator.audio_control, sine_generator.rate, sine_generator.port_parameter.device_interface, sine_generator.rate);
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&sine_generator, HAL_AUDIO_SET_SINE_GENERATOR);
#endif

#ifdef MTK_AUDIO_LOOPBACK_TEST_ENABLE
    //Audio loopback test choose pga 0/1/2/3 mux = null or normal
    if (Audio_setting->Audio_source.Pga_mux == HAL_AUDIO_LOOPBACK_TEST_PGA_NULL) {
        DSP_MW_LOG_I("[AUDIO LOOPBACK TEST] pga mux = null", 0);
        //Pga 0/1 mux = null
        ANA_SET_REG(AUDENC_ANA_CON0,  0, AUDENC_ANA_CON0_L_PREAMP_INPUT_SEL_MASK | AUDENC_ANA_CON0_L_PREAMP_GAIN_MASK);
        ANA_SET_REG(AUDENC_ANA_CON1,  0, AUDENC_ANA_CON1_R_PREAMP_INPUT_SEL_MASK | AUDENC_ANA_CON1_R_PREAMP_GAIN_MASK);
        //Pga 2/3 mux = null
        ANA_SET_REG(AUDENC_ANA_CON9,  0, AUDENC_ANA_CON9_RG_AUD23PREAMPLINPUTSEL_MASK | AUDENC_ANA_CON9_RG_AUD23PREAMPLGAIN_MASK);
        ANA_SET_REG(AUDENC_ANA_CON10, 0, AUDENC_ANA_CON10_RG_AUD23PREAMPRINPUTSEL_MASK | AUDENC_ANA_CON10_RG_AUD23PREAMPRGAIN_MASK);
        //Pga 4/5 mux = null
        ANA_SET_REG(AUDENC_ANA_CON18, 0, AUDENC_ANA_CON18_RG_AUD45PREAMPLINPUTSEL_MASK | AUDENC_ANA_CON18_RG_AUD45PREAMPLGAIN_MASK);
        ANA_SET_REG(AUDENC_ANA_CON19, 0, AUDENC_ANA_CON19_RG_AUD45PREAMPRINPUTSEL_MASK | AUDENC_ANA_CON19_RG_AUD45PREAMPRGAIN_MASK);
    } else {
        DSP_MW_LOG_I("[AUDIO LOOPBACK TEST] pga mux = normal", 0);
    }
#endif
#endif
    return ret;
}

static int32_t pcm_ul1_close(SOURCE source)
{
    int32_t ret = 0;
#if defined (AIR_DCHS_MODE_ENABLE)
    if((source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT) || (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT)){
        dchs_send_unlock_sleep_msg(false);
    }
#endif
    AUDIO_PARAMETER *runtime = &source->param.audio;
    hal_audio_device_t device = source->param.audio.audio_device;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    hal_audio_device_t device1 = source->param.audio.audio_device1;
    hal_audio_device_t device2 = source->param.audio.audio_device2;
    hal_audio_device_t device3 = source->param.audio.audio_device3;
    hal_audio_device_t device4 = source->param.audio.audio_device4;
    hal_audio_device_t device5 = source->param.audio.audio_device5;
    hal_audio_device_t device6 = source->param.audio.audio_device6;
    hal_audio_device_t device7 = source->param.audio.audio_device7;
#endif
    hal_audio_path_parameter_t *path_handle = &source->param.audio.path_handle;//modify for ab1568
    hal_audio_device_parameter_t *device_handle = &source->param.audio.device_handle;//modify for ab1568
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    hal_audio_device_parameter_t *device_handle1 = &source->param.audio.device_handle1;//modify for ab1568
    hal_audio_device_parameter_t *device_handle2 = &source->param.audio.device_handle2;//modify for ab1568
    hal_audio_device_parameter_t *device_handle3 = &source->param.audio.device_handle3;//modify for ab1568
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    hal_audio_device_parameter_t *device_handle4 = &source->param.audio.device_handle4;//modify for ab1568
    hal_audio_device_parameter_t *device_handle5 = &source->param.audio.device_handle5;//modify for ab1568
    hal_audio_device_parameter_t *device_handle6 = &source->param.audio.device_handle6;//modify for ab1568
    hal_audio_device_parameter_t *device_handle7 = &source->param.audio.device_handle7;//modify for ab1568
#endif
#endif
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        if ((device == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER) || (device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) ||
            (device == HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL) || (device == HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL) ||
            (device == HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL)) {
             device1 = NULL;
        }
#endif
    DSP_MW_LOG_I("[AFE UL] close: scenario type %d", 1, source->scenario_type);
    runtime->is_pcm_ul1_open = false;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if ((device7 != NULL) && (!(device7 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle7, device7, HAL_AUDIO_CONTROL_OFF);
    }
    if ((device6 != NULL) && (!(device6 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle6, device6, HAL_AUDIO_CONTROL_OFF);
    }
    if ((device5 != NULL) && (!(device5 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle5, device5, HAL_AUDIO_CONTROL_OFF);
    }
    if ((device4 != NULL) && (!(device4 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle4, device4, HAL_AUDIO_CONTROL_OFF);
    }
#endif
    if ((device3 != NULL) && (!(device3 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle3, device3, HAL_AUDIO_CONTROL_OFF);
    }
    if ((device2 != NULL) && (!(device3 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle2, device2, HAL_AUDIO_CONTROL_OFF);
    }
    if ((device1 != NULL) && (!(device1 & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R)))) {
        hal_audio_set_device(device_handle1, device1, HAL_AUDIO_CONTROL_OFF);
    }
#endif
    if (!(device & (HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_HW_GAIN_R|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_L|HAL_AUDIO_CONTROL_DEVICE_LOOPBACK_I2S_MASTER_R))) {
        hal_audio_set_device(device_handle, device, HAL_AUDIO_CONTROL_OFF);
    }
#ifdef AIR_AUDIO_PATH_CUSTOMIZE_ENABLE
    if ((source->scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL) || (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL)) {
        hal_audio_device_parameter_t device_i2s_master_temp;
        device_i2s_master_temp.i2s_master.audio_device = HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER;
        device_i2s_master_temp.i2s_master.i2s_interface = HAL_AUDIO_INTERFACE_1;
        device_i2s_master_temp.i2s_master.rate = path_handle->audio_output_rate[0];
        device_i2s_master_temp.i2s_master.scenario_type = source->scenario_type;
        device_i2s_master_temp.i2s_master.is_tx = false;
        device_i2s_master_temp.i2s_master.i2s_format = HAL_AUDIO_I2S_I2S;
        device_i2s_master_temp.i2s_master.word_length = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;
        device_i2s_master_temp.i2s_master.mclk_divider = 0;
        device_i2s_master_temp.i2s_master.with_mclk = false;
        device_i2s_master_temp.i2s_master.is_low_jitter = gAudioCtrl.Afe.AfeULSetting.is_low_jitter[1];
        device_i2s_master_temp.i2s_master.is_rx_swap = false;
        device_i2s_master_temp.i2s_master.is_tx_swap = false;
        device_i2s_master_temp.i2s_master.is_internal_loopback = false;
        device_i2s_master_temp.i2s_master.is_recombinant = false;
        hal_audio_set_device(&device_i2s_master_temp, HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER, HAL_AUDIO_CONTROL_OFF);
    }
#endif
#ifdef AIR_AUDIO_PATH_CUSTOMIZE_ENABLE
        // ADDA UL to I2S0
        if ((source->scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL) || (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL)) {
            hal_audio_path_parameter_t path1_temp;
            path1_temp.input.interconn_sequence[0]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_UL2_CH1;
            path1_temp.input.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_UL2_CH2;
            path1_temp.output.interconn_sequence[0] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_MASTER_I2S0_CH1;
            path1_temp.output.interconn_sequence[1] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_MASTER_I2S0_CH2;
            path1_temp.connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
            path1_temp.connection_number = 2;
            path1_temp.audio_input_rate[0] = path_handle->audio_input_rate[0];;
            path1_temp.audio_input_rate[1] = path_handle->audio_input_rate[1];;
            path1_temp.audio_output_rate[0] = path_handle->audio_output_rate[0];;
            path1_temp.audio_output_rate[1] = path_handle->audio_output_rate[1];;
            path1_temp.with_hw_gain=false;
            path1_temp.with_updown_sampler[0]=0;
            path1_temp.with_updown_sampler[1]=0;
            path1_temp.with_dl_deq_mixer=0;
            path1_temp.out_device=4096;
            path1_temp.out_device_interface=2;
            path1_temp.scenario_type = source->scenario_type;
            hal_audio_set_path(&path1_temp, HAL_AUDIO_CONTROL_OFF);

            // I2S0 to UL
            hal_audio_path_parameter_t path3_temp;
            path3_temp.input.interconn_sequence[0]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_MASTER_I2S0_CH1;
            path3_temp.input.interconn_sequence[1]  = HAL_AUDIO_INTERCONN_SELECT_INPUT_DEVICE_MASTER_I2S0_CH2;
            path3_temp.output.interconn_sequence[0] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_MEMORY_VUL1_CH1;
            path3_temp.output.interconn_sequence[1] = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_MEMORY_VUL1_CH2;
            path3_temp.connection_selection = HAL_AUDIO_INTERCONN_CH01CH02_to_CH01CH02;
            path3_temp.connection_number = 2;
            path3_temp.audio_input_rate[0] = path_handle->audio_output_rate[0];;
            path3_temp.audio_input_rate[1] = path_handle->audio_output_rate[1];;
            path3_temp.audio_output_rate[0] = path_handle->audio_output_rate[0];;
            path3_temp.audio_output_rate[1] = path_handle->audio_output_rate[1];;

            path3_temp.with_hw_gain=false;
            path3_temp.with_updown_sampler[0]=0;
            path3_temp.with_updown_sampler[1]=0;
            path3_temp.with_dl_deq_mixer=0;
            path3_temp.out_device=768;
            path3_temp.out_device_interface=1;
            path3_temp.scenario_type = source->scenario_type;
            hal_audio_set_path(&path3_temp, HAL_AUDIO_CONTROL_OFF);
        } else {
            hal_audio_set_path(path_handle, HAL_AUDIO_CONTROL_OFF);
        }
#else
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    if ((device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) && ((source->param.audio.memory == HAL_AUDIO_MEM6) || (source->param.audio.memory == HAL_AUDIO_MEM7))) {
        //I2S Slave Infra mode or Tdm mode
    } else {
        hal_audio_set_path(path_handle, HAL_AUDIO_CONTROL_OFF);
    }
#else
    hal_audio_set_path(path_handle, HAL_AUDIO_CONTROL_OFF);
#endif
#endif
    return ret;
}

static int32_t pcm_ul1_trigger(SOURCE source, int cmd)
{
    int32_t ret = 0;

    switch (cmd) {
        case AFE_PCM_TRIGGER_START:
            ret = pcm_ul1_start(source);
            if ((ret == 0) && (source->param.audio.echo_reference == true)) {
                ret = afe_platform_awb_ops.trigger(source, cmd);
            }
            break;
        case AFE_PCM_TRIGGER_STOP:
            if (source->param.audio.echo_reference == true) {
                ret = afe_platform_awb_ops.trigger(source, cmd);
            }
            if (ret == 0) {
                ret = pcm_ul1_stop(source);
            }
            break;
        case AFE_PCM_TRIGGER_RESUME:
            ret = pcm_ul1_open(source);
            if ((ret == 0) && (source->param.audio.echo_reference == true)) {
                ret = afe_platform_awb_ops.trigger(source, cmd);
            }
            break;
        case AFE_PCM_TRIGGER_SUSPEND:
            if (source->param.audio.echo_reference == true) {
                ret = afe_platform_awb_ops.trigger(source, cmd);
            }
            if (ret == 0) {
                ret = pcm_ul1_close(source);
            }
            break;
        default:
            ret = -1;
            break;
    }
    if (ret != 0) {
        DSP_MW_LOG_W("pcm_ul1_trigger %d, error:%d\r\n", 2, cmd, ret);
    }
    return ret;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 static int32_t pcm_ul1_copy(SOURCE source, void *dst, uint32_t count)
{
    //copy the AFE src streambuffer to sink streambuffer
    if (Source_Audio_ReadAudioBuffer(source, dst, count) == false) {
        return -1;
    }
    return 0;
}

audio_source_pcm_ops_t afe_platform_ul1_ops = {
    .probe      = pcm_ul1_probe,
    .open       = pcm_ul1_open,
    .close      = pcm_ul1_close,
    .hw_params  = pcm_ul1_hw_params,
    .trigger    = pcm_ul1_trigger,
    .copy       = pcm_ul1_copy,
};
